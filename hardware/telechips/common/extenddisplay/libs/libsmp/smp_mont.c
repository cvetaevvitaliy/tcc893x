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
#include "smp_mont.h"

//-------------------------------------------------------------------
// Function Implementation
//-------------------------------------------------------------------

//!
//! Allocate new memory for Montgomery data structure
//!
//! @return Allocated Montgomery data structure pointer
//!
SMP_MONT*
SMP_MONT_new(void)
{
    SMP_MONT *ret;

    if ((ret=(SMP_MONT *)SMP_mem_malloc(sizeof(SMP_MONT))) == NULL)
        return (NULL);

    SMP_MONT_init(ret);
    ret->flags = SMP_FLG_MALLOCED;
    return (ret);
}

//!
//! Initialize Montgomery data structure
//!
//! @param mont Montgomery data structure pointer to initialize
//!
void
SMP_MONT_init(SMP_MONT *mont)
{
    mont->ri=0;
    SMP_init(&(mont->RR));
    SMP_init(&(mont->N));
    SMP_init(&(mont->Ni));
    mont->flags=0;
}

//!
//! Free Montgomery data structure
//!
//! @param mont Montgomery data structure pointer to free
//!
void
SMP_MONT_free(SMP_MONT *mont)
{
    if(mont == NULL)
        return;

    SMP_free(&(mont->RR));
    SMP_free(&(mont->N));
    SMP_free(&(mont->Ni));
    if (mont->flags & SMP_FLG_MALLOCED)
        SMP_mem_free(mont);
}

//!
//! Copy Montgomery data structure
//!
//! @param to Destination pointer
//! @param from Source pointer
//! @return NULL - Fail Destination pointer - Success
//!
SMP_MONT*
SMP_MONT_copy(SMP_MONT *to, SMP_MONT *from)
{
    if (to == from) return(to);

    if (!SMP_copy(&(to->RR),&(from->RR))) return NULL;
    if (!SMP_copy(&(to->N),&(from->N))) return NULL;
    if (!SMP_copy(&(to->Ni),&(from->Ni))) return NULL;
    to->ri=from->ri;
    to->n0=from->n0;
    return(to);
}

//!
//! Calculate and set modulus related information for Montgomery algorithm
//!
//! @param mont Data structre to store modulus related information
//! @param mod SMP data structure for modulus
//! @return 0 - Fail 1 - Success
//!
int
SMP_MONT_set(SMP_MONT *mont, const SMP *mod)
{
    SMP Ri,*R;

    SMP_init(&Ri);
    R = &(mont->RR);                    // grab RR as a temp
    SMP_copy(&(mont->N),mod);            // Set N
    mont->N.neg = 0;

    mont->ri=SMP_num_bits(&mont->N);
    if (!SMP_set_word(R,0))
        return (0);
    if (!SMP_set_bit(R,mont->ri))       // R = 2^ri
        return (0);                     // Ri = R^-1 mod N
    if ((SMP_mod_inverse(&Ri,R,&mont->N)) == NULL)
        return (0);
    if (!SMP_lshift(&Ri,&Ri,mont->ri))  // R*Ri
        return (0);
    if (!SMP_sub_word(&Ri,1))            // Ni = (R*Ri-1) / N
        return (0);

    if (!SMP_div(&(mont->Ni),NULL,&Ri,&mont->N))
        return (0);

    SMP_free(&Ri);

    // setup RR for conversions
    if (!SMP_set_word(&(mont->RR),0))
        return (0);
    if (!SMP_set_bit(&(mont->RR),mont->ri*2))
        return (0);
    if (!SMP_mod(&(mont->RR),&(mont->RR),&(mont->N)))
        return (0);

    return(1);
}

//!
//! Montgomery reduction
//!
//! @param r (a * R^-1) mod mont.N result
//! @param a SMP data structure a
//! @param mont Modulus related information for Montgomery algorithm
//! @return 0 - Fail 1 - Success
//!
int
SMP_MONT_red(SMP *r, const SMP *a, SMP_MONT *mont)
{
    int ret = 0;
    SMP smpt1,smpt2;
    SMP *t1 = &smpt1;
    SMP *t2 = &smpt2;

    SMP_init(t1);
    SMP_init(t2);

    if (!SMP_copy(t1,a))
        goto err;
    SMP_mask_bits(t1,mont->ri);

    if (!SMP_mul(t2,t1,&mont->Ni))
        goto err;
    SMP_mask_bits(t2,mont->ri);

    if (!SMP_mul(t1,t2,&mont->N))
        goto err;
    if (!SMP_add(t2,a,t1))
        goto err;
    if (!SMP_rshift(r,t2,mont->ri))
        goto err;

    if (SMP_ucmp(r, &(mont->N)) >= 0)
    {
        if (!SMP_usub(r,r,&(mont->N)))
            goto err;
    }

    ret = 1;

err:

    SMP_free(t1);
    SMP_free(t2);
    return (ret);
}


//!
//! Multiply and reduce by Montgomery alogorithm
//!
//! @param r (a * b * R^-1) mod mont.N result
//! @param a SMP data structure a
//! @param b SMP data structure b
//! @param mont Modulus related information for Montgomery algorithm
//! @return 0 - Fail 1 - Success
//!
int
SMP_MONT_mul(SMP *r, const SMP *a, const SMP *b, SMP_MONT *mont)
{
    int ret = 0;
    SMP smptmp;
    SMP *tmp = &smptmp;

    SMP_init(tmp);

    if (a == b)
    {
        if (!SMP_sqr(tmp,a))
            goto err;
    }
    else
    {
        if (!SMP_mul(tmp,a,b))
            goto err;
    }
    // reduce from aRR to aR
    if (!SMP_MONT_red(r,tmp,mont))
        goto err;

    ret = 1;

err:

    SMP_free(tmp);
    return (ret);
}

//!
//! Montgomery algorithm of modular exponentiation
//! Used when modulus is odd
//!
//! @param rr (a ^ p) mod m result
//! @param a Base
//! @param p Exponent
//! @param m Modulus
//! @param mont Montgomery parameters (Recalculated if NULL)
//! @return 0 - Fail 1 - Success
//!
int
SMP_MONT_mod_exp(SMP *rr, const SMP *a, const SMP *p, const SMP *m, SMP_MONT *mont)
{
    int i,j,bits,ret=0,wstart,wend,window,wvalue;
    int start=1,ts=0;

    SMP smpd, smpr;
    SMP *d = &smpd;
    SMP *r = &smpr;
    const SMP *aa;
    SMP val[SMP_TABLE_SIZE];
    SMP_MONT *lmont=NULL;

    smp_check_top(a);
    smp_check_top(p);
    smp_check_top(m);

    if (!(m->d[0] & 1))
    {
        SMP_ERR(SMP_ERR_MONTEVENMOD);
        return (0);
    }
    bits=SMP_num_bits(p);
    if (bits == 0)
    {
        ret = SMP_set_word(rr,1);
        return (ret);
    }

    SMP_init(d);
    SMP_init(r);

    if (mont != NULL)
    {
        lmont = mont;
    }
    else
    {
        if ((lmont=SMP_MONT_new()) == NULL)
            goto err;
        if (!SMP_MONT_set(lmont,m))
            goto err;
    }

    SMP_init(&val[0]);
    ts=1;
    if (a->neg || SMP_ucmp(a,m) >= 0)
    {
        if (!SMP_nnmod(&(val[0]),a,m))
            goto err;
        aa= &(val[0]);
    }
    else
        aa=a;

    if (SMP_is_zero(aa))
    {
        ret = SMP_set_word(rr,0);
        goto err;
    }
    if (!SMP_MONT_mul(&(val[0]),aa,&(lmont->RR),lmont))
        goto err;

    window = SMP_window_bits_for_exponent_size(bits);
    if (window > 1)
    {
        if (!SMP_MONT_mul(d,&(val[0]),&(val[0]),lmont))
            goto err;
        j=1<<(window-1);
        for (i=1; i<j; i++)
        {
            SMP_init(&(val[i]));
            if (!SMP_MONT_mul(&(val[i]),&(val[i-1]),d,lmont))
                goto err;
        }
        ts=i;
    }

    start=1;        // This is used to avoid multiplication etc
                    // when there is only the value '1' in the
                    // buffer.
    wvalue=0;        // The 'value' of the window
    wstart=bits-1;  // The top bit of the window
    wend=0;         // The bottom bit of the window

    if (!SMP_MONT_mul(r,SMP_value_one(),&(lmont->RR),lmont))
        goto err;

    for (;;)
    {
        if (SMP_is_bit_set(p,wstart) == 0)
        {
            if (!start)
            {
                if (!SMP_MONT_mul(r,r,r,lmont))
                goto err;
            }
            if (wstart == 0) break;
            wstart--;
            continue;
        }
        // We now have wstart on a 'set' bit, we now need to work out
        // how bit a window to do.  To do this we need to scan
        // forward until the last set bit before the end of the
        // window
        j=wstart;
        wvalue=1;
        wend=0;
        for (i=1; i<window; i++)
        {
            if (wstart-i < 0) break;
            if (SMP_is_bit_set(p,wstart-i))
            {
                wvalue<<=(i-wend);
                wvalue|=1;
                wend=i;
            }
        }

        // wend is the size of the current window
        j=wend+1;
        // add the 'bytes above'
        if (!start)
            for (i=0; i<j; i++)
            {
                if (!SMP_MONT_mul(r,r,r,lmont))
                    goto err;
            }

        // wvalue will be an odd number < 2^window
        if (!SMP_MONT_mul(r,r,&(val[wvalue>>1]),lmont))
            goto err;

        // move the 'window' down further
        wstart-=wend+1;
        wvalue=0;
        start=0;
        if (wstart < 0) break;
    }

    if (!SMP_MONT_red(rr,r,lmont))
        goto err;

    ret=1;

err:

    if ((mont == NULL) && lmont) SMP_MONT_free(lmont);
    for (i=0; i<ts; i++)
        SMP_free(&(val[i]));
    SMP_free(d);
    SMP_free(r);

    return (ret);
}
