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
#include <string.h>

// SMP header
#include "smp.h"
#include "smp_err.h"

//-------------------------------------------------------------------
// Function Implementation
//-------------------------------------------------------------------

//!
//! Square a and save result in r
//!
//! @param r a ^ 2 result
//! @param a SMP data structure a
//! @return 0 - Fail 1 - Success
//!
int
SMP_sqr(SMP *r, const SMP *a)
{
    int max,al;
    int ret  = 0;
    SMP *rr  = NULL;
    SMP *tmp = NULL;

    smp_check_top(a);

    al = a->top;
    if (al <= 0)
    {
        if (!SMP_set_word(r,0))
            return (0);

        return (1);
    }

    rr = SMP_new();
    if (rr == NULL)
        goto err;

    tmp = SMP_new();
    if (tmp == NULL)
        goto err;

    max = (al+al);
    if ((SMP_expand_w(rr,max+1) == NULL) || (SMP_expand_w(tmp,max) == NULL))
        goto err;

    smp_sqr_normal(rr->d,a->d,al,tmp->d);
    rr->top = max;
    rr->neg = 0;

    SMP_fix_top(rr);
    SMP_copy(r,rr);

    ret = 1;

err:
    if (tmp) SMP_free(tmp);
    if (rr) SMP_free(rr);
    return (ret);
}

//!
//! Square SMP_WORD array a with length n
//! Temporary variable tmp must have 2*n words
//!
//! @param r SMP_WORD array to save result
//! @param a SMP_WORD array a
//! @param n Length of SMP_WORD array a
//! @param tmp Temporary SMP_WORD array
//!
void
smp_sqr_normal(SMP_WORD *r, const SMP_WORD *a, int n, SMP_WORD *tmp)
{
    int i,j,max;
    const SMP_WORD *ap;
    SMP_WORD *rp;

    max = n*2;
    ap = a;
    rp = r;
    rp[0] = rp[max-1] = 0;
    rp++;
    j = n;

    if (--j > 0)
    {
        ap++;
        rp[j] = smp_mul_words(rp,ap,j,ap[-1]);
        rp+=2;
    }

    for (i = n-2 ; i > 0 ; i--)
    {
        j--;
        ap++;
        rp[j] = smp_mul_add_words(rp,ap,j,ap[-1]);
        rp+=2;
    }

    smp_add_words(r,r,r,max);

    // There will not be a carry

    smp_sqr_words(tmp,a,n);

    smp_add_words(r,r,tmp,max);
}
