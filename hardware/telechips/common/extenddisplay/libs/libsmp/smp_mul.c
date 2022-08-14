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
//! Multiply a and b, and save result in r
//!
//! @param r a * b result
//! @param a SMP data structure a
//! @param b SMP data structure b
//! @return 0 - Fail 1 - Success
//!
int
SMP_mul(SMP *r, const SMP *a, const SMP *b)
{
    int top,al,bl;
    int ret = 0;
    SMP *rr = NULL;

    smp_check_top(a);
    smp_check_top(b);
    smp_check_top(r);

    al=a->top;
    bl=b->top;

    if ((al == 0) || (bl == 0))
    {
        if (!SMP_set_word(r,0))
            return (0);

        return (1);
    }
    top=al+bl;

    rr = SMP_new();
    if (rr == NULL)
        return (0);

    rr->neg = a->neg ^ b->neg;

    if (SMP_expand_w(rr,top) == NULL)
        goto err;

    rr->top=top;
    smp_mul_normal(rr->d, a->d, al, b->d, bl);

    SMP_fix_top(rr);
    SMP_copy(r,rr);

    ret = 1;

err:
    SMP_free(rr);
    return (ret);
}

//!
//! Multiply SMP_WORD array a with SMP_WORD array b
//!
//! @param r SMP_WORD array to save result
//! @param a SMP_WORD array a
//! @param na Length of SMP_WORD array a
//! @param b SMP_WORD array b
//! @param nb Length of SMP_WORD array b
//!
void
smp_mul_normal(SMP_WORD *r, SMP_WORD *a, int na, SMP_WORD *b, int nb)
{
    SMP_WORD *rr;

    if (na < nb)
    {
        int itmp;
        SMP_WORD *ltmp;

        itmp=na; na=nb; nb=itmp;
        ltmp=a;   a=b;   b=ltmp;

    }
    rr = &(r[na]);
    rr[0] = smp_mul_words(r,a,na,b[0]);

    for (;;)
    {
        if (--nb <= 0) return;
        rr[1]=smp_mul_add_words(&(r[1]),a,na,b[1]);
        if (--nb <= 0) return;
        rr[2]=smp_mul_add_words(&(r[2]),a,na,b[2]);
        if (--nb <= 0) return;
        rr[3]=smp_mul_add_words(&(r[3]),a,na,b[3]);
        if (--nb <= 0) return;
        rr[4]=smp_mul_add_words(&(r[4]),a,na,b[4]);

        rr += 4;
        r  += 4;
        b  += 4;
    }
}

//!
//! Multiply word w to SMP data a
//!
//! @param a SMP data structure
//! @param w Positive word
//! @return 0 - Fail 1 - Success
//!
int
SMP_mul_word(SMP *a, SMP_WORD w)
{
    SMP_WORD ll;

    w &= SMP_MASK;
    if (a->top)
    {
        if (w == 0)
            SMP_set_word(a,0);
        else
        {
            ll=smp_mul_words(a->d,a->d,a->top,w);
            if (ll)
            {
                if (SMP_expand_w(a,a->top+1) == NULL)
                    return (0);
                a->d[a->top++]=ll;
            }
        }
    }

    return (1);
}
