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

#ifndef __SMP_MONT_H__
#define __SMP_MONT_H__

#ifdef  __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// Data Types
//-------------------------------------------------------------------

//! Montgomery data structure
typedef struct _smp_mont_st_
{
    int ri;        //!< number of bits in R
    SMP RR;        //!< used to convert to montgomery form
    SMP N;         //!< The modulus
    SMP Ni;        //!< R*(1/R mod N) - N*Ni = 1
    SMP_WORD n0;   //!< least significant word of Ni
    int flags;     //!< Control flags
} SMP_MONT;

//-------------------------------------------------------------------
// Function Prototypes
//-------------------------------------------------------------------

SMP_MONT* SMP_MONT_new(void);
void SMP_MONT_init(SMP_MONT *mont);
void SMP_MONT_free(SMP_MONT *mont);
SMP_MONT* SMP_MONT_copy(SMP_MONT *to, SMP_MONT *from);
int SMP_MONT_set(SMP_MONT *mont, const SMP *mod);
int SMP_MONT_red(SMP *r, const SMP *a, SMP_MONT *mont);
int SMP_MONT_mul(SMP *r, const SMP *a, const SMP *b, SMP_MONT *mont);
int SMP_MONT_mod_exp(SMP *r, const SMP *a, const SMP *p, const SMP *m, SMP_MONT *mont);

#ifdef  __cplusplus
}
#endif
#endif
