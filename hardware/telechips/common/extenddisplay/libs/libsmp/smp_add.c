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
//! Add a and b, and save result in r
//!
//! @param r a + b result
//! @param a SMP data structure a
//! @param b SMP data structure b
//! @return 0 - Fail 1 - Success
//!
int
SMP_add(SMP *r, const SMP *a, const SMP *b)
{
    const SMP *tmp;
    int a_neg = a->neg;

    smp_check_top(a);
    smp_check_top(b);

    //  a +  b    a+b
    //  a + -b    a-b
    // -a +  b    b-a
    // -a + -b    -(a+b)
    if (a_neg ^ b->neg)
    {
        // only one is negative
        if (a_neg)
        {
            tmp = a;
            a = b;
            b = tmp;
        }

        // we are now a - b
        if (SMP_ucmp(a,b) < 0)
        {
            if (!SMP_usub(r,b,a))
                return 0;
            r->neg=1;
        }
        else
        {
            if (!SMP_usub(r,a,b))
                return 0;
            r->neg=0;
        }
        return 1;
    }

    if (!SMP_uadd(r,a,b))
        return 0;

    if (a_neg) // both are neg
        r->neg=1;
    else
        r->neg=0;

    return 1;
}

//!
//! Add absolute values of a and b, and save result in r
//!
//! @param r |a| + |b| result
//! @param a SMP data structure a
//! @param b SMP data structure b
//! @return 0 - Fail 1 - Success
//!
int
SMP_uadd(SMP *r, const SMP *a, const SMP *b)
{
    register int i;
    int max, min;
    SMP_WORD *ap, *bp, *rp, carry, t1;
    const SMP *tmp;

    smp_check_top(a);
    smp_check_top(b);

    if (a->top < b->top)
    {
        tmp = a;
        a = b;
        b = tmp;
    }
    max = a->top;
    min = b->top;

    if (SMP_expand_w(r,max+1) == NULL)
        return 0;

    r->top = max;

    ap = a->d;
    bp = b->d;
    rp = r->d;
    carry = 0;

    carry = smp_add_words(rp,ap,bp,min);
    rp += min;
    ap += min;
    bp += min;
    i = min;

    if (carry)
    {
        while (i < max)
        {
            i++;
            t1= *(ap++);
            if ((*(rp++)=(t1+1)&SMP_MASK) >= t1)
            {
                carry=0;
                break;
            }
        }
        if ((i >= max) && carry)
        {
            *(rp++)=1;
            r->top++;
        }
    }
    if (rp != ap)
    {
        for (; i<max; i++)
            *(rp++)= *(ap++);
    }
    // memcpy(rp,ap,sizeof(*ap)*(max-i));
    r->neg = 0;
    return 1;
}

//!
//! Add word w to SMP data a
//!
//! @param a SMP data structure
//! @param w Positive word
//! @return 0 - Fail 1 - Success
//!
int
SMP_add_word(SMP *a, SMP_WORD w)
{
    SMP_WORD l;
    int i;

    if (a->neg)
    {
        a->neg = 0;
        i = SMP_sub_word(a,w);
        if (!SMP_is_zero(a))
            a->neg = !(a->neg);
        return i;
    }

    w &= SMP_MASK;
    if (SMP_expand_w(a,a->top+1) == NULL)
        return 0;

    i = 0;
    for (;;)
    {
        if (i >= a->top)
            l = w;
        else
            l = (a->d[i]+(SMP_WORD)w)&SMP_MASK;

        a->d[i] = l;
        if (w > l)
            w = 1;
        else
            break;
        i++;
    }
    if (i >= a->top)
        a->top++;

    return 1;
}

//!
//! Subtract b from a, and save result in r
//!
//! @param r a - b result
//! @param a SMP data structure a
//! @param b SMP data structure b
//! @return 0 - Fail 1 - Success
//!
int
SMP_sub(SMP *r, const SMP *a, const SMP *b)
    {
    int max;
    int add=0,neg=0;
    const SMP *tmp;

    smp_check_top(a);
    smp_check_top(b);

    //  a -  b    a-b
    //  a - -b    a+b
    // -a -  b    -(a+b)
    // -a - -b    b-a
    if (a->neg)
    {
        if (b->neg)
        {
            tmp = a;
            a = b;
            b = tmp;
        }
        else
        {
            add=1;
            neg=1;
        }
    }
    else
    {
        if (b->neg)
        {
            add=1;
            neg=0;
        }
    }

    if (add)
    {
        if (!SMP_uadd(r,a,b))
            return 0;
        r->neg=neg;
        return 1;
    }

    // We are actually doing a - b :-)

    max = (a->top > b->top) ? a->top : b->top;
    if (SMP_expand_w(r,max) == NULL)
        return 0;
    if (SMP_ucmp(a,b) < 0)
    {
        if (!SMP_usub(r,b,a))
            return 0;
        r->neg=1;
    }
    else
    {
        if (!SMP_usub(r,a,b))
            return 0;
        r->neg=0;
    }

    return 1;
}

//!
//! Subtract absolute value of b from a, and save result in r
//! Absolute value of a must be bigger than b (|a| > |b|)
//! Result is always positive
//!
//! @param r |a| - |b| result
//! @param a SMP data structure a
//! @param b SMP data structure b
//! @return 0 - Fail 1 - Success
//!
int
SMP_usub(SMP *r, const SMP *a, const SMP *b)
{
    int max,min;
    register SMP_WORD t1,t2,*ap,*bp,*rp;
    int i,carry;

    smp_check_top(a);
    smp_check_top(b);

    if (a->top < b->top) // hmm... should not be happening
        return 0;

    max=a->top;
    min=b->top;
    if (SMP_expand_w(r,max) == NULL)
        return 0;

    ap=a->d;
    bp=b->d;
    rp=r->d;

    carry=0;
    for (i=0; i<min; i++)
    {
        t1= *(ap++);
        t2= *(bp++);
        if (carry)
        {
            carry=(t1 <= t2);
            t1=(t1-t2-1)&SMP_MASK;
        }
        else
        {
            carry=(t1 < t2);
            t1=(t1-t2)&SMP_MASK;
        }
        *(rp++)=t1&SMP_MASK;
    }

    if (carry) // subtracted
    {
        while (i < max)
        {
            i++;
            t1= *(ap++);
            t2=(t1-1)&SMP_MASK;
            *(rp++)=t2;
            if (t1 > t2) break;
        }
    }
    if (rp != ap)
    {
        for (;;)
        {
            if (i++ >= max) break;
            rp[0]=ap[0];
            if (i++ >= max) break;
            rp[1]=ap[1];
            if (i++ >= max) break;
            rp[2]=ap[2];
            if (i++ >= max) break;
            rp[3]=ap[3];
            rp+=4;
            ap+=4;
        }
    }

    r->top=max;
    r->neg=0;
    SMP_fix_top(r);
    return 1;
}

//!
//! Subtract word w from SMP data a
//!
//! @param a SMP data structure
//! @param w Positive word
//! @return 0 - Fail 1 - Success
//!
int
SMP_sub_word(SMP *a, SMP_WORD w)
{
    int i;

    if (SMP_is_zero(a) || a->neg)
    {
        a->neg = 0;
        i = SMP_add_word(a,w);
        a->neg = 1;
        return i;
    }

    w &= SMP_MASK;
    if ((a->top == 1) && (a->d[0] < w))
    {
        a->d[0] = w-a->d[0];
        a->neg = 1;
        return 1;
    }

    i = 0;
    for (;;)
    {
        if (a->d[i] >= w)
        {
            a->d[i] -= w;
            break;
        }
        else
        {
            a->d[i] = (a->d[i]-w)&SMP_MASK;
            i++;
            w = 1;
        }
    }

    if ((a->d[i] == 0) && (i == (a->top-1)))
        a->top--;

    return 1;
}
