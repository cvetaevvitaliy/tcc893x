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
//! Bitwise left shift of SMP data by 1 and save result in r
//!
//! @param r a << 1 result
//! @param a SMP data structure a
//! @return 0 - Fail 1 - Success
//!
int
SMP_lshift1(SMP *r, const SMP *a)
{
    register SMP_WORD *ap,*rp,t,c;
    int i;

    if (r != a)
    {
        r->neg=a->neg;
        if (SMP_expand_w(r,a->top+1) == NULL)
            return (0);
        r->top=a->top;
    }
    else
    {
        if (SMP_expand_w(r,a->top+1) == NULL)
            return (0);
    }

    ap=a->d;
    rp=r->d;
    c=0;
    for (i=0; i<a->top; i++)
    {
        t= *(ap++);
        *(rp++)=((t<<1)|c)&SMP_MASK;
        c=(t & SMP_TBIT)?1:0;
    }
    if (c)
    {
        *rp=1;
        r->top++;
    }

    return (1);
}

//!
//! Bitwise right shift of SMP data by 1 and save result in r
//!
//! @param r a >> 1 result
//! @param a SMP data structure a
//! @return 0 - Fail 1 - Success
//!
int
SMP_rshift1(SMP *r, const SMP *a)
{
    SMP_WORD *ap,*rp,t,c;
    int i;

    if (SMP_is_zero(a))
    {
        if (!SMP_set_word(r,0))
            return (0);

        return (1);
    }

    if (a != r)
    {
        if (SMP_expand_w(r,a->top) == NULL)
            return(0);
        r->top=a->top;
        r->neg=a->neg;
    }

    ap=a->d;
    rp=r->d;
    c=0;
    for (i=a->top-1; i>=0; i--)
    {
        t=ap[i];
        rp[i]=((t>>1)&SMP_MASK)|c;
        c=(t&1)?SMP_TBIT:0;
    }
    SMP_fix_top(r);

    return (1);
}

//!
//! Bitwise left shift of SMP data by n and save result in r
//!
//! @param r a << n result
//! @param a SMP data structure a
//! @param n Number of left shifts
//! @return 0 - Fail 1 - Success
//!
int
SMP_lshift(SMP *r, const SMP *a, int n)
{
    int i,nw,lb,rb;
    SMP_WORD *t,*f;
    SMP_WORD l;

    r->neg = a->neg;
    nw = n/SMP_BITS;
    if (SMP_expand_w(r,a->top+nw+1) == NULL)
        return (0);

    lb = n%SMP_BITS;
    rb = SMP_BITS-lb;
    f = a->d;
    t = r->d;
    t[a->top+nw] = 0;
    if (lb == 0)
        for (i=a->top-1 ; i>=0 ; i--)
            t[nw+i] = f[i];
    else
        for (i=a->top-1 ; i>=0 ; i--)
        {
            l = f[i];
            t[nw+i+1] |= (l>>rb)&SMP_MASK;
            t[nw+i] = (l<<lb)&SMP_MASK;
        }

    memset(t,0,nw*sizeof(t[0]));
    r->top=a->top+nw+1;
    SMP_fix_top(r);

    return (1);
}

//!
//! Bitwise right shift of SMP data by n and save result in r
//!
//! @param r a >> n result
//! @param a SMP data structure a
//! @param n Number of right shifts
//! @return 0 - Fail 1 - Success
//!
int
SMP_rshift(SMP *r, const SMP *a, int n)
    {
    int i,j,nw,lb,rb;
    SMP_WORD *t,*f;
    SMP_WORD l,tmp;

    nw = n/SMP_BITS;
    rb = n%SMP_BITS;
    lb = SMP_BITS-rb;
    if (nw > a->top || a->top == 0)
    {
        if (!SMP_set_word(r,0))
            return (0);

        return (1);
    }

    if (r != a)
    {
        r->neg = a->neg;
        if (SMP_expand_w(r,a->top-nw+1) == NULL)
            return(0);
    }
    else
    {
        if (n == 0)
            return 1; // or the copying loop will go berserk
    }

    f = &(a->d[nw]);
    t = r->d;
    j = a->top-nw;
    r->top = j;

    if (rb == 0)
    {
        for (i=j+1 ; i > 0 ; i--)
            *(t++)= *(f++);
    }
    else
    {
        l= *(f++);
        for (i=1; i<j; i++)
        {
            tmp = (l>>rb)&SMP_MASK;
            l = *(f++);
            *(t++) = (tmp|(l<<lb))&SMP_MASK;
        }
        *(t++) = (l>>rb)&SMP_MASK;
    }
    *t=0;
    SMP_fix_top(r);

    return (1);
}
