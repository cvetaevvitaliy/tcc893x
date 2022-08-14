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
//! Divide num by divisor and save quotient result in dv
//! and remainder result in rm
//!
//! @param dv num / divisor result
//! @param rm num % divisor result
//! @param num SMP operand
//! @param divisor SMP divisor
//! @return 0 - Fail 1 - Success
//!
int
SMP_div(SMP *dv, SMP *rm, const SMP *num, const SMP *divisor)
{
    int norm_shift,i,j,loop,ret=0;
    SMP_WORD *resp,*wnump;
    SMP_WORD d0,d1;
    int num_n,div_n;

    SMP smptmp, smpsnum, smpsdiv, smpres, wnum;
    SMP *tmp  = &smptmp;
    SMP *snum = &smpsnum;
    SMP *sdiv = &smpsdiv;
    SMP *res  = &smpres;

    smp_check_top(num);
    smp_check_top(divisor);

    if (SMP_is_zero(divisor))
    {
        SMP_ERR(SMP_ERR_DIVBYZERO);
        return (0);
    }

    if (SMP_ucmp(num,divisor) < 0)
    {
        if (rm != NULL)
        {
            if (SMP_copy(rm,num) == NULL)
                return (0);
        }
        if (dv != NULL)
        {
            if (!SMP_set_word(dv,0))
                return (0);
        }
        return(1);
    }

    SMP_init(tmp);
    SMP_init(snum);
    SMP_init(sdiv);
    SMP_init(res);
    if (dv != NULL)
        res = dv;

    tmp->neg = 0;

    // First we normalise the numbers
    norm_shift = SMP_BITS-((SMP_num_bits(divisor))%SMP_BITS);
    if (!(SMP_lshift(sdiv,divisor,norm_shift)))
        goto err;
    sdiv->neg=0;
    norm_shift+=SMP_BITS;
    if (!(SMP_lshift(snum,num,norm_shift)))
        goto err;
    snum->neg=0;
    div_n=sdiv->top;
    num_n=snum->top;
    loop=num_n-div_n;

    // Lets setup a 'window' into snum
    // This is the part that corresponds to the current
    // 'area' being divided
    SMP_init(&wnum);
    wnum.d=     &(snum->d[loop]);
    wnum.top= div_n;
    wnum.dmax= snum->dmax+1; /* a bit of a lie */

    // Get the top 2 words of sdiv
    // i=sdiv->top;
    d0=sdiv->d[div_n-1];
    d1=(div_n == 1)?0:sdiv->d[div_n-2];

    // pointer to the 'top' of snum
    wnump= &(snum->d[num_n-1]);

    // Setup to 'res'
    res->neg= (num->neg^divisor->neg);
    if (!SMP_expand_w(res,(loop+1)))
        goto err;
    res->top=loop;
    resp= &(res->d[loop-1]);

    // space for temp
    if (!SMP_expand_w(tmp,(div_n+1)))
        goto err;

    if (SMP_ucmp(&wnum,sdiv) >= 0)
    {
        if (!SMP_usub(&wnum,&wnum,sdiv))
            goto err;
        *resp=1;
        res->d[res->top-1]=1;
    }
    else
        res->top--;

    if (res->top == 0)
        res->neg = 0;
    resp--;

    for (i=0; i<loop-1; i++)
    {
        SMP_WORD q,l0;
        SMP_WORD n0,n1,rem=0;

        n0=wnump[0];
        n1=wnump[-1];
        if (n0 == d0)
            q=SMP_MASK;
        else // n0 < d0
        {
            SMP_WORD t2l,t2h,ql,qh;

            q=smp_div_words(n0,n1,d0);
            rem=(n1-q*d0)&SMP_MASK;

            t2l=LBITS(d1); t2h=HBITS(d1);
            ql =LBITS(q);  qh =HBITS(q);
            mul64(t2l,t2h,ql,qh); // t2=d1*q;

            for (;;)
            {
                if ((t2h < rem) ||
                    ((t2h == rem) && (t2l <= wnump[-2])))
                    break;
                q--;
                rem += d0;
                if (rem < d0) break; // don't let rem overflow
                if (t2l < d1) t2h--; t2l -= d1;
            }
        }

        l0=smp_mul_words(tmp->d,sdiv->d,div_n,q);
        wnum.d--; wnum.top++;
        tmp->d[div_n]=l0;
        for (j=div_n+1; j>0; j--)
            if (tmp->d[j-1]) break;
        tmp->top=j;

        j=wnum.top;
        if (!SMP_sub(&wnum,&wnum,tmp))
            goto err;

        snum->top=snum->top+wnum.top-j;

        if (wnum.neg)
        {
            q--;
            j=wnum.top;
            if (!SMP_add(&wnum,&wnum,sdiv))
                goto err;
            snum->top+=wnum.top-j;
        }
        *(resp--)=q;
        wnump--;
    }

    if (rm != NULL)
    {
        // Keep a copy of the neg flag in num because if rm==num
        // SMP_rshift() will overwrite it.
        //
        int neg = num->neg;
        SMP_rshift(rm,snum,norm_shift);
        if (!SMP_is_zero(rm))
            rm->neg = neg;
    }

    ret = 1;

err:

    if (dv == NULL) SMP_free(res);
    SMP_free(sdiv);
    SMP_free(snum);
    SMP_free(tmp);

    return(ret);
}
/*
//!
//! Divide SMP data a by word w
//!
//! @param a SMP data structure
//! @param w Positive word
//! @return Remainder result
//!
SMP_WORD
SMP_div_word(SMP *a, SMP_WORD w)
{
    SMP_WORD ret;
    int i;

    if (a->top == 0)
        return (0);

    ret = 0;
    w &= SMP_MASK;
    for (i=a->top-1; i>=0; i--)
    {
        SMP_WORD l,d;

        l=a->d[i];

        // smp_div_words assertion occurs if divisor is bigger --a
        // Added by Juno 2004/5/29
        if ((ret == 0) && (l < w))
            d = 0;
        else
            d = smp_div_words(ret,l,w);

        ret=(l-((d*w)&SMP_MASK))&SMP_MASK;
        a->d[i]=d;
    }
    if ((a->top > 0) && (a->d[a->top-1] == 0))
        a->top--;

    return (ret);
}

//!
//! SMP data a % word w
//!
//! @param a SMP data structure
//! @param w Positive word
//! @return Modulus result
//!
SMP_WORD
SMP_mod_word(const SMP *a, SMP_WORD w)
{
    SMP_WORD ret=0;
    int i;

    w &= SMP_MASK;
    for (i=a->top-1; i>=0; i--)
    {
        ret=((ret<<SMP_BITS2)|((a->d[i]>>SMP_BITS2)&SMP_MASKl))%w;
        ret=((ret<<SMP_BITS2)|(a->d[i]&SMP_MASKl))%w;
    }

    return((SMP_WORD)ret);
}
*/
//!
//! Divide (h,l) by d and return quotient result
//!
//! @param h Higher part of operand
//! @param l Lower part of operand
//! @param d Divisor
//! @return Quotient result
//!
SMP_WORD
smp_div_words(SMP_WORD h, SMP_WORD l, SMP_WORD d)
{
    SMP_WORD dh,dl,q,ret=0,th,tl,t;
    int i,count=2;

    if (d == 0)
        return(SMP_MASK);

    i=SMP_num_bits_word(d);

    // I don't understand how this works :-( Juno
    // assert((i == SMP_BITS) || (h > (SMP_WORD)1<<i));

    i=SMP_BITS-i;
    if (h >= d)
        h-=d;

    if (i)
    {
        d<<=i;
        h=(h<<i)|(l>>(SMP_BITS-i));
        l<<=i;
    }
    dh=(d&SMP_MASKh)>>SMP_BITS2;
    dl=(d&SMP_MASKl);
    for (;;)
    {
        if ((h>>SMP_BITS2) == dh)
            q=SMP_MASKl;
        else
            q=h/dh;

        th=q*dh;
        tl=dl*q;
        for (;;)
        {
            t=h-th;
            if ((t&SMP_MASKh) || ((tl) <= ((t<<SMP_BITS2)|((l&SMP_MASKh)>>SMP_BITS2))))
                break;
            q--;
            th-=dh;
            tl-=dl;
        }
        t=(tl>>SMP_BITS2);
        tl=(tl<<SMP_BITS2)&SMP_MASKh;
        th+=t;

        if (l < tl) th++;
        l-=tl;
        if (h < th)
        {
            h+=d;
            q--;
        }
        h-=th;

        if (--count == 0)
            break;

        ret=q<<SMP_BITS2;
        h=((h<<SMP_BITS2)|(l>>SMP_BITS2))&SMP_MASK;
        l=(l&SMP_MASKl)<<SMP_BITS2;
    }
    ret|=q;

    return(ret);
}
