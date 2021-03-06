/****************************************************************************************
 * toolsMisc.c                                                                          *
 * Collection of miscelaneous utility functions.                                        *
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
 ****************************************************************************************/

#pragma once
#include    <string.h>
#include    <ctype.h>   //for tolower()
#ifndef WINDOWS
    #include    <sys/time.h>        //for struct time_t
    #include    <sys/resource.h>    //for getrusage()
#endif
#include    "globals.h"
#include    <stdbool.h>
#include    <stdlib.h>


///Prototypes:

uint32_t    isnan_d(double);
double      min(double, double);
double      max(double, double);
void        fatal(const char*);
void        printCpuTime(FILE*);
char*       stringToLower(char* str);


///Functions:

uint32_t isnan_d(double x){
    //Note that isnan() is only defined for the float data type, and not for doubles
    //NANs are never equal to anything -even themselves:
    if (x!=x){
        return true;
    }else{
        return false;
    }
}

double      min(double a, double b){
    if( a <= b){
        return a;
    }else{
        return b;
    }
}

double      max(double a, double b){
    if( a > b){
        return a;
    }else{
        return b;
    }
}

void        fatal(const char* message){
    /*
        Prints a message and terminates the program with an error.
        Closes all open i/o streams before exiting.
    */
    printf("%s\n", message);
    fflush(NULL);               //flushes all i/o streams.
    exit(EXIT_FAILURE);
}

#if 0
void        printCpuTime(FILE* stream){
    /*
     * prints total cpu time used by process.
     * NOTE: because getrusage is Posix (ie, Unix/Linux) only, to make stuff easy, we're simply stripping this function from the windows version
     */
    
    #ifdef WINDOWS
        (void)stream;
    #else
        struct rusage   usage;
        
        getrusage(RUSAGE_SELF, &usage);
        fprintf(stream, "%ld.%06ld seconds user CPU time,\n", usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
        fprintf(stream, "%ld.%06ld seconds system CPU time used.\n", usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);
    #endif
}
#endif

char*   stringToLower(char* str){
    /*
     * Converts a string to lowercase. assumes string is properly NULL-terminated.
     */
    for(uintptr_t i = 0; str[i]; i++){
      str[i] = tolower(str[i]);
    }
    
    return str;
}
