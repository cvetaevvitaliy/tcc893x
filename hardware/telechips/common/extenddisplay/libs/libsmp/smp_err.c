/****************************************************************************
Copyright (C) 2013 Telechips Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
****************************************************************************/

//-------------------------------------------------------------------
// Preprocessor
//-------------------------------------------------------------------

// Standard header
#include <stdio.h>
#include <stdlib.h>

// SMP header
#include "smp_err.h"

//-------------------------------------------------------------------
// Global Variables
//-------------------------------------------------------------------

//! Global error data structure instance
SMP_ERR smp_err_glb;

//! Error string list
SMP_ERR_STR SMP_err_reason[]=
{
    {SMP_ERR_NONE, "No error"},
    {SMP_ERR_UNKNOWN, "Unknown error"},
    {SMP_ERR_MALLOC, "Memory allocation error"},
    {SMP_ERR_STATIC, "Trying to reallocate static memory error"},
    {SMP_ERR_NULL, "Trying to access NULL memory error"},
    {SMP_ERR_DIVBYZERO, "Trying to divide by zero error"},
    {SMP_ERR_NOINVERSE, "Inverse non-existance error"},
    {SMP_ERR_MONTEVENMOD, "Even modulus used for Montgomery algorithm error"},
    {SMP_ERR_INVALIDRANGE, "Invalid range specified error"},
    {0,NULL}
};

//-------------------------------------------------------------------
// Function Implementation
//-------------------------------------------------------------------

//!
//! Get SMP error data structure address
//!
//! @return SMP error data structure pointer
//!
SMP_ERR*
smp_load_err(void)
{
    return (&smp_err_glb);
}

//!
//! Initialize SMP error data structure
//!
void
smp_init_err(void)
{
    int iCnt;
    SMP_ERR *se = NULL;

    se = smp_load_err();
    if (se)
    {
        for (iCnt = 0 ; iCnt < SMP_ERR_MAX ; iCnt++)
        {
            se->err_code[iCnt] = 0x0;
            se->err_file[iCnt] = NULL;
            se->err_line[iCnt] = 0;
        }
        se->top = 0;
        se->bottom = 0;
    }
}

//!
//! Add an error to SMP error data structure
//!
//! @param errcode Error code to add
//! @param errfile Error occurrence file
//! @param errline Error occurrence line
//!
void
smp_put_err(unsigned long errcode, const char *errfile, int errline)
{
    SMP_ERR *se = NULL;

    se = smp_load_err();
    if (se)
    {
        se->err_code[se->top] = errcode;
        se->err_file[se->top] = errfile;
        se->err_line[se->top] = errline;
        se->top = (se->top+1)%SMP_ERR_MAX;
        if (se->top == se->bottom)
            se->bottom = (se->bottom+1)%SMP_ERR_MAX;
    }
}

//!
//! Print SMP error
//!
void
smp_print_err(void)
{
    int iTop, iBot;
    SMP_ERR *se = NULL;

    se = smp_load_err();
    if (se)
    {
        iTop = se->top;
        iBot = se->bottom;
        while (iTop != iBot)
        {
            printf("Error:%08lx:%s:%d\n", se->err_code[iBot], se->err_file[iBot], se->err_line[iBot]);
            printf("\t%s\n", (SMP_err_reason+(se->err_code[iBot]))->string);
            iBot = (iBot+1)%SMP_ERR_MAX;
        }
    }
}
