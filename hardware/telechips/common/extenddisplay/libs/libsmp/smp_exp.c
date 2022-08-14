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
/*
//!
//! Calculate a ^ p, and save result in r
//! This may take VERY long! :(
//!
//! @param r a ^ p result
//! @param a Base
//! @param p Exponent
//! @return 0 - Fail 1 - Success
//!
int
SMP_exp(SMP *r, const SMP *a, const SMP *p)
{
    int i,bits;

    SMP smpv, smprr;
    SMP *v = &smpv;
    SMP *rr = &smprr;

    SMP_init(v);
    SMP_init(rr);

    if (SMP_copy(v,a) == NULL)
        return (0);
    bits=SMP_num_bits(p);

    if (SMP_is_odd(p))
    {
        if (SMP_copy(rr,a) == NULL)
            return (0);
    }
    else
    {
        if (!SMP_set_word(rr,1))
            return (0);
    }

    for (i=1; i<bits; i++)
    {
        if (!SMP_sqr(v,v))
            return (0);
        if (SMP_is_bit_set(p,i))
        {
            if (!SMP_mul(rr,rr,v))
                return (0);
        }
    }

    SMP_copy(r,rr);
    return (1);
}
*/
//!
//! Calculate (a ^ p) mod m, and save result in r
//!
//! @param r (a ^ p) mod m result
//! @param a Base
//! @param p Exponent
//! @param m Modulus
//! @return 0 - Fail 1 - Success
//!
int
SMP_mod_exp(SMP *r, const SMP *a, const SMP *p, const SMP *m)
{
    int ret;

    smp_check_top(a);
    smp_check_top(p);
    smp_check_top(m);

    // For even modulus  m = 2^k*m_odd,  it might make sense to compute
    // a^p mod m_odd  and  a^p mod 2^k  separately (with Montgomery
    // exponentiation for the odd part), using appropriate exponent
    // reductions, and combine the results using the CRT.
    //
    // For now, we use Montgomery only if the modulus is odd; otherwise,
    // exponentiation using the simple algorithm is used.

    if (SMP_is_odd(m))
    {
/*
        if (a->top == 1 && !a->neg)
        {
            SMP_WORD A = a->d[0];
            ret = SMP_mod_exp_mont_word(r,A,p,m);
        }
        else
*/
        ret = SMP_MONT_mod_exp(r,a,p,m,NULL);
    }
    else
    {
        ret = SMP_mod_exp_simple(r,a,p,m);
    }

    return (ret);
}

//!
//! Simple algorithm of modular exponentiation
//! Used when modulus is even
//!
//! @param r (a ^ p) mod m result
//! @param a Base
//! @param p Exponent
//! @param m Modulus
//! @return 0 - Fail 1 - Success
//!
int
SMP_mod_exp_simple(SMP *r, const SMP *a, const SMP *p, const SMP *m)
{
    int i,j,bits,ret=0,wstart,wend,window,wvalue,ts=0;
    int start=1;

    SMP smpd;
    SMP *d = &smpd;
    SMP val[SMP_TABLE_SIZE];

    bits = SMP_num_bits(p);

    if (bits == 0)
    {
        ret = SMP_set_word(r,1);
        return (ret);
    }

    SMP_init(d);
    SMP_init(&(val[0]));

    ts=1;
    if (!SMP_nnmod(&(val[0]),a,m)) // 1
        goto err;
    if (SMP_is_zero(&(val[0])))
    {
        ret = SMP_set_word(r,0);
        goto err;
    }

    window = SMP_window_bits_for_exponent_size(bits);
    if (window > 1)
    {
        if (!SMP_mod_mul(d,&(val[0]),&(val[0]),m)) // 2
            goto err;
        j=1<<(window-1);
        for (i=1; i<j; i++)
        {
            SMP_init(&(val[i]));
            if (!SMP_mod_mul(&(val[i]),&(val[i-1]),d,m))
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

    if (!SMP_set_word(r,1)) goto err;

    for (;;)
    {
        if (SMP_is_bit_set(p,wstart) == 0)
        {
            if (!start)
                if (!SMP_mod_mul(r,r,r,m))
                    goto err;
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
                if (!SMP_mod_mul(r,r,r,m))
                    goto err;
            }

        // wvalue will be an odd number < 2^window
        if (!SMP_mod_mul(r,r,&(val[wvalue>>1]),m))
            goto err;

        // move the 'window' down further
        wstart-=wend+1;
        wvalue=0;
        start=0;
        if (wstart < 0) break;
    }

    ret = 1;

err:

    for (i=0; i<ts; i++)
        SMP_free(&(val[i]));
    SMP_free(d);

    return (ret);
}

