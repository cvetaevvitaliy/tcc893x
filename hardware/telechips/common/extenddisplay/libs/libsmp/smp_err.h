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

#ifndef __SMP_ERR_H__
#define __SMP_ERR_H__

#ifdef  __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------

// All error handling in main program source must only use these MACROs
#ifdef SMP_DEBUG
#include <assert.h>
#define smp_check_top(a) assert((a)->top >= 0 && (a)->top <= (a)->dmax);
#define SMP_ERR_INIT() smp_init_err()
#define SMP_ERR(a) smp_put_err(a, __FILE__, __LINE__)
#else
#define smp_check_top(a)
#define SMP_ERR_INIT()
#define SMP_ERR(a)
#endif

//! Number of maximum error to be stored
#define SMP_ERR_MAX   8

//-------------------------------------------------------------------
// Data Types
//-------------------------------------------------------------------

//! Error stack data structure
typedef struct _smp_err_st_
{
    unsigned long err_code[SMP_ERR_MAX];   //!< Error code array
    const char *err_file[SMP_ERR_MAX];     //!< Error occurrence file
    int err_line[SMP_ERR_MAX];             //!< Error occurrence line
    int top;                               //!< Error top index counter
    int bottom;                            //!< Error bottom index counter
} SMP_ERR;

//! Error string structure
typedef struct _smp_err_str_st_
{
    unsigned long error;                   //!< Error code
    const char *string;                    //!< Error string
} SMP_ERR_STR;

//-------------------------------------------------------------------
// Error Codes
//-------------------------------------------------------------------

//! Error codes
enum
{
    SMP_ERR_NONE = 0x0,                    //!< No error
    SMP_ERR_UNKNOWN,                       //!< Unknown error
    SMP_ERR_MALLOC,                        //!< Memory allocation error
    SMP_ERR_STATIC,                        //!< Static memory allocation error
    SMP_ERR_NULL,                          //!< NULL memory access error
    SMP_ERR_DIVBYZERO,                     //!< Divide by zero error
    SMP_ERR_NOINVERSE,                     //!< Inverse non-existance error
    SMP_ERR_MONTEVENMOD,                   //!< Even modulus for Montgomery error
    SMP_ERR_INVALIDRANGE                   //!< Invalid range error
};

//-------------------------------------------------------------------
// Function Prototypes
//-------------------------------------------------------------------

SMP_ERR* smp_load_err(void);

void smp_init_err(void);
void smp_put_err(unsigned long errcode, const char *errfile, int errline);
void smp_print_err(void);

#ifdef  __cplusplus
}
#endif
#endif












