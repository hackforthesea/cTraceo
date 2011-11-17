/********************************************************************************
 *	calcCohTransLoss.c	 														*
 * 	(formerly "calctl.for")														*
 *	Calculates Coherent Transmission Loss.										*
 *																				*
 *	originally written in FORTRAN by:											*
 *  					Orlando Camargo Rodriguez:								*
 *						Copyright (C) 2010										*
 * 						Orlando Camargo Rodriguez								*
 *						orodrig@ualg.pt											*
 *						Universidade do Algarve									*
 *						Physics Department										*
 *						Signal Processing Laboratory							*
 *																				*
 *	Ported to C for project SENSOCEAN by:										*
 * 						Emanuel Ey												*
 *						emanuel.ey@gmail.com									*
 *						Signal Processing Laboratory							*
 *						Universidade do Algarve									*
 *																				*
 *	Inputs:																		*
 * 				settings:	Pointer to structure containing all input info.		*
 * 	Outputs:																	*
 * 				none:		Writes to file "ctl.mat".							*
 * 	Note:																		*
 * 		This function requires that the coherent acoustic pressure has been		*
 * 		calculated (by calcCohAcoustPress) and will be of no use otherwise.		*
 * 																				*
 *******************************************************************************/

#include "globals.h"
#if USE_MATLAB == 1
	#include <mat.h>
	#include "matrix.h"
#else
	#include	"matlabOut/matlabOut.h"
#endif
#include <math.h>
#include <complex.h>

void calcCohTransLoss(settings_t*);

void calcCohTransLoss(settings_t* settings){
	uint32_t	i, j, dim;
	double*		tl		= NULL;
	double**	tl2D	= NULL;
	MATFile*	matfile	= NULL;
	mxArray*	ptl		= NULL;
	mxArray*	ptl2D		= NULL;

	matfile		= matOpen("ctl.mat", "u");
	if(matfile == NULL){
		fatal("Memory alocation error.");
	}
	
	switch(settings->output.arrayType){
		case ARRAY_TYPE__RECTANGULAR:
		case ARRAY_TYPE__HORIZONTAL:
		case ARRAY_TYPE__VERTICAL:
			/*
			 * horizontal and vertical arrays are special cases of Rectangular hydrophone arrays,
			 * so all of them can be handled by the same code.
			 */
			tl2D = mallocDouble2D(settings->output.nArrayR, settings->output.nArrayZ);
			
			for(i=0; i<settings->output.nArrayR; i++){
				for(j=0; j<settings->output.nArrayZ; j++){
					tl2D[i][j] = -20.0*log10( cabs( settings->output.pressure2D[i][j] ) );
				}
			}
			
			ptl2D = mxCreateDoubleMatrix((MWSIZE)settings->output.nArrayZ, (MWSIZE)settings->output.nArrayR, mxREAL);
			if(ptl2D == NULL){
				fatal("Memory alocation error.");
			}
			copyDoubleToPtr2D_transposed(tl2D, ptl2D, settings->output.nArrayZ, settings->output.nArrayR);
			matPutVariable(matfile,"tl",ptl2D);
			mxDestroyArray(ptl2D);
			
			freeDouble2D(tl2D, settings->output.nArrayR);
			break;
			
		case ARRAY_TYPE__LINEAR:
			dim = (uint32_t)max((double)settings->output.nArrayR, (double)settings->output.nArrayZ);
			tl = mallocDouble(dim);
			
			for(j=0; j<dim; j++){
				tl[j] = -20.0*log10( cabs( settings->output.pressure2D[0][j] ) );
				DEBUG(8, "|p|: %lf, tl: %lf\n", cabs( settings->output.pressure2D[0][j] ), tl[j]);
			}
			
			ptl = mxCreateDoubleMatrix((MWSIZE)1, (MWSIZE)dim, mxREAL);
			if(ptl == NULL){
				fatal("Memory alocation error.");
			}
			copyDoubleToPtr(tl, mxGetPr(ptl), dim);
			matPutVariable(matfile,"tl",ptl);
			mxDestroyArray(ptl);
			
			free(tl);
			break;
	}
}
