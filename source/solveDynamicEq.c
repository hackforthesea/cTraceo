/****************************************************************************************
 * solveDynamicEq.c                                                                     *
 * (formerly "sdyneq.for")                                                              *
 * A raytracing subroutine for cylindrical simmetry. Calculates complex amplitude along *
 * a ray's path. Note that the ray's path must have been previously calculated by       *
 * calling solveEikonal.c                                                               *
 *                                                                                      *
 * ------------------------------------------------------------------------------------ *
 * Website:                                                                             *
 *          https://github.com/EyNuel/cTraceo/wiki                                      *
 *                                                                                      *
 * License: This file is part of the cTraceo Raytracing Model and is released under the *
 *          Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License  *
 *          http://creativecommons.org/licenses/by-nc-sa/3.0/                           *
 *                                                                                      *
 * NOTE:    cTraceo is research code under active development.                          *
 *          The code may contain bugs and updates are possible in the future.           *
 *                                                                                      *
 * Written for project SENSOCEAN by:                                                    *
 *          Emanuel Ey                                                                  *
 *          emanuel.ey@gmail.com                                                        *
 *          Copyright (C) 2011 - 2013                                                   *
 *          Signal Processing Laboratory                                                *
 *          Universidade do Algarve                                                     *
 *                                                                                      *
 * cTraceo is the C port of the FORTRAN 77 TRACEO code written by:                      *
 *          Orlando Camargo Rodriguez:                                                  *
 *          Copyright (C) 2010                                                          *
 *          Orlando Camargo Rodriguez                                                   *
 *          orodrig@ualg.pt                                                             *
 *          Universidade do Algarve                                                     *
 *          Physics Department                                                          *
 *          Signal Processing Laboratory                                                *
 *                                                                                      *
 * ------------------------------------------------------------------------------------ *
 *  Inputs:                                                                             *
 *          settings:   Pointer to structure containing all input info.                 *
 *                                                                                      *
 *  Outputs:                                                                            *
 *          ray:        Pointer to structure containing all ray information.            *
 *                      Note that the ray's launching angle (ray->theta) must be        *
 *                      previously defined as it is an input value.                     *
 *                      See also: "globals.h" for ray structure definition.             *
 *                                                                                      *
 * Return Value:                                                                        *
 *          None                                                                        *
 *                                                                                      *
 ****************************************************************************************/

#pragma  once
#include <math.h>       //for NAN
#include "csValues.c"
#include "thorpe.c"
#include "globals.h"
#include "tools.h"
#include "dotProduct.c"
#include <complex.h>
#include "reflectionCorr.c"

void    solveDynamicEq(settings_t*, ray_t*);

void    solveDynamicEq(settings_t* settings, ray_t* ray){
    DEBUG(3,"in\n");
    int32_t         ibdry;
    double          alpha;
    double          ri, zi, ci, cii, cxc, sigmaI, crri, czzi, crzi;
    double          crriNext, czziNext, crziNext;   //
    vector_t        slowness     = {0,0};
    vector_t        gradC   = {0,0};    //gradient of sound speed (c) at current coords
    vector_t        nGradC  = {0,0};    //gradient of sound speed (c) at next coords
    vector_t        dGradC  = {0,0};
    double          dr, dz, dsi;
    vector_t        es, sigma;
    double          drdn, dzdn;
    double          cnn;
    vector_t        sigmaN;
    double          cnj, csj, rm, rn;
    vector_t        tauB;
    double          prod;
    complex double  ap_aq;
    uintptr_t       i;

    //Define initial conditions:
    ray->p[0]       = 1;
    ray->q[0]       = 0;
    ray->caustc[0]  = 0;
    ibdry           = 1;

    //Get Thorpe attenuation in dB/m:
    thorpe(settings->source.freqx, &alpha);

    //get gradient of the sound speed at the first set of coordinates
    //NOTE: these values are saves as "next" so that they can be used correctlyin the first iteration of the loop.
    ri = ray->r[0];
    zi = ray->z[0];
    csValues(settings, ri, zi, &cii, &cxc, &sigmaI, &gradC.r, &gradC.z, &slowness, &crri, &czzi, &crzi);
    (void)slowness;     //TODO: slowness is not used -it should not be calculated

    //Solve the Dynamic Equations:
    for(i=0; i<ray->nCoords -2; i++){

        //NOTE: in subsequent iterations, the "current" is the former "next", so we'll get the former "next" and use it as "current"
        gradC.r = nGradC.r;
        gradC.z = nGradC.z;
        crri = crriNext;
        czzi = czziNext;
        crzi = crziNext;
        
        //determine the gradient of the sound speed at the next set of coordinates
        //TODO call csvalues directly with ray->xx (i.e.: skip the intermediate variable ri,zi
        ri = ray->r[i+1];
        zi = ray->z[i+1];
        csValues(settings, ri, zi, &cii, &cxc, &sigmaI, &nGradC.r, &nGradC.z, &slowness, &crriNext, &czziNext, &crziNext);
        (void)slowness;     //TODO: slowness is not used -it should not be calculated
        
        dGradC.r = nGradC.r - gradC.r;
        dGradC.z = nGradC.z - gradC.z;
        dr = ray->r[i+1] - ray->r[i];
        dz = ray->z[i+1] - ray->z[i];

        dsi = sqrt( dr*dr + dz*dz );
        es.r = dr/dsi;
        #if VERBOSE == 1
            if(isnan(es.r)){
                DEBUG(1,"i: %u\n", (uint32_t)i);
                fatal("Found NaN!");
            }
        #endif
        es.z = dz/dsi;

        sigma.r = sigmaI * es.r;
        sigma.z = sigmaI * es.z;

        drdn = -es.z;
        dzdn =  es.r;
        cnn = ( drdn*drdn )*crri + 2 * drdn * dzdn * crzi + (dzdn*dzdn )*czzi;
        DEBUG(8,"drdn:%e, dzdn:%e, crri:%e, crzi:%e, czzi:%e, cnn:%e\n", drdn, dzdn, crri, crzi, czzi, cnn);

        ci = ray->c[i];
        cxc = ci*ci;

        if ( ray->iRefl[i+1] == false){
            DEBUG(9,"Case 1\n");
            DEBUG(10,"p[0]:%e, p:%e\n", ray->p[0], ray->p[i]);
            ray->p[i+1] = ray->p[i] - ray->q[i] * (cnn / cxc) * dsi;
            ray->q[i+1] = ray->q[i] + ray->p[i] * ci * dsi;
            DEBUG(8,"p:%e, q:%e, ci:%e, dsi:%e\n",ray->p[i], ray->q[i], ci, dsi);
            //Bellhop's refraction correction:
            sigmaN.r = -sigma.z;
            sigmaN.z =  sigma.r;
            cnj = dotProduct( &dGradC, &sigmaN);
            csj = dotProduct( &dGradC, &sigma);
            
            if (sigma.z != 0){
                rm      = sigma.r/sigma.z;
                rn      = -( rm * ( 2 * cnj - rm * csj )/cii );
                ray->p[i+1] = ray->p[i] + ray->q[i] * rn;
            }
        }else if (ray->iRefl[i+1] == true){
            DEBUG(9,"Case 2\n");
            ibdry   = ray->boundaryJ[i+1];
            tauB.r  = ray->boundaryTg[i+1].r;
            tauB.z  = ray->boundaryTg[i+1].z;

            reflectionCorr(ibdry, sigma, tauB, gradC, ci, &rn);
            ray->p[i+1] = ray->p[i] + ray->q[i] * rn;
            ray->q[i+1] = ray->q[i];
        }else{
            fatal("Solving dynamic equations: iRefl neither 1 nor 0!\nAborting...");
        }
        
        //Q: is this completely redundant?! can't seem to find anywhere where "caustc" is actually used..
        //A: actually, the value of caustc is used when calculating acoustic pressure at a hydrophone in "getRayPressure()".
        prod = ray->q[i] * ray->q[i+1];
        if ( (prod <= 0) && (ray->q[i] != 0)){
            ray->caustc[i+1] = ray->caustc[i] + M_PI/2.0;
        }else{
            ray->caustc[i+1] = ray->caustc[i];
        }
    }   //for(i=0; i<ray->nCoords -2; i++)

    //Amplitude calculation:
    DEBUG(10, "amp[10]:%lf, cxc:%lf, cnn:%e\n", cabs(ray->amp[10]), cxc, cnn);
    for(i=1; i<ray->nCoords; i++){
        ap_aq       = (complex double)( ray->c[0] * cos(ray->theta) * ray->c[i] / ( ray->ic[i] * ray->q[i] ));
        DEBUG(7, "i:%u, ap_aq:%e, c: %lf, ic:%lf, q:%e\n", (uint32_t)i, (double)cabs(ap_aq), ray->c[i], ray->ic[i], ray->q[i]);
        ray->amp[i] = csqrt( ap_aq ) * ray->decay[i] * exp( -alpha * ray->s[i] );
        /*
        if(isnan((float)cabs(ray->amp[i]))){
            //fatal("its a NAN!\n");
        }
        */
    }
    ray->amp[0] = NAN;
    DEBUG(4, "decay[n-3]: %e +i*%e, s[n-3]: %e, amp[n-3]: %e +j*%e, p[n-3]: %e +j*%e, q[n-3]: %e +j*%e, c[n-3]: %e\n", creal(ray->decay[ray->nCoords-3]), cimag(ray->decay[ray->nCoords-3]), ray->s[ray->nCoords-3], creal(ray->amp[ray->nCoords-3]), cimag(ray->amp[ray->nCoords-3]), creal(ray->p[ray->nCoords-3]), cimag(ray->p[ray->nCoords-3]), creal(ray->q[ray->nCoords-3]), cimag(ray->q[ray->nCoords-3]), ray->c[ray->nCoords-3]);
    DEBUG(4, "decay[n-2]: %e +i*%e, s[n-2]: %e, amp[n-2]: %e +j*%e, p[n-2]: %e +j*%e, q[n-2]: %e +j*%e, c[n-2]: %e\n", creal(ray->decay[ray->nCoords-2]), cimag(ray->decay[ray->nCoords-2]), ray->s[ray->nCoords-2], creal(ray->amp[ray->nCoords-2]), cimag(ray->amp[ray->nCoords-2]), creal(ray->p[ray->nCoords-2]), cimag(ray->p[ray->nCoords-2]), creal(ray->q[ray->nCoords-2]), cimag(ray->q[ray->nCoords-2]), ray->c[ray->nCoords-2]);
    DEBUG(4, "decay[n-1]: %e +i*%e, s[n-1]: %e, amp[n-1]: %e +j*%e, p[n-1]: %e +j*%e, q[n-1]: %e +j*%e, c[n-1]: %e\n", creal(ray->decay[ray->nCoords-1]), cimag(ray->decay[ray->nCoords-1]), ray->s[ray->nCoords-1], creal(ray->amp[ray->nCoords-1]), cimag(ray->amp[ray->nCoords-1]), creal(ray->p[ray->nCoords-1]), cimag(ray->p[ray->nCoords-1]), creal(ray->q[ray->nCoords-1]), cimag(ray->q[ray->nCoords-1]), ray->c[ray->nCoords-1]);
    DEBUG(3, "out\n");
}
