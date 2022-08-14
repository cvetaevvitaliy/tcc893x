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
#include <assert.h>

// SMP header
#include "smp.h"
#include "smp_err.h"

//-------------------------------------------------------------------
// Function Implementation
//-------------------------------------------------------------------

//!
//! Add SMP_WORD arrays with length n and save result in r
//! Memory should be allocated for result
//!
//! @param r Pointer to result
//! @param a Pointer to operand a
//! @param b Pointer to operand b
//! @param n Length of SMP_WORD arrays
//! @return Carry generated from operation
//!
SMP_WORD
smp_add_words(SMP_WORD *r, const SMP_WORD *a, const SMP_WORD *b, int n)
{
    SMP_WORD c,l,t;

    if (n <= 0)
        return((SMP_WORD)0);

    c=0;
    for (;;)
    {
        t=a[0];
        t=(t+c)&SMP_MASK;
        c=(t < c);
        l=(t+b[0])&SMP_MASK;
        c+=(l < t);
        r[0]=l;
        if (--n <= 0) break;

        t=a[1];
        t=(t+c)&SMP_MASK;
        c=(t < c);
        l=(t+b[1])&SMP_MASK;
        c+=(l < t);
        r[1]=l;
        if (--n <= 0) break;

        t=a[2];
        t=(t+c)&SMP_MASK;
        c=(t < c);
        l=(t+b[2])&SMP_MASK;
        c+=(l < t);
        r[2]=l;
        if (--n <= 0) break;

        t=a[3];
        t=(t+c)&SMP_MASK;
        c=(t < c);
        l=(t+b[3])&SMP_MASK;
        c+=(l < t);
        r[3]=l;
        if (--n <= 0) break;

        a+=4;
        b+=4;
        r+=4;
    }
    return((SMP_WORD)c);
}

//!
//! Subtract SMP_WORD arrays with length n and save result in r
//! Memory should be allocated for result
//!
//! @param r Pointer to result
//! @param a Pointer to operand a
//! @param b Pointer to operand b
//! @param n Length of SMP_WORD arrays
//! @return Borrow generated from operation
//!
SMP_WORD
smp_sub_words(SMP_WORD *r, const SMP_WORD *a, const SMP_WORD *b, int n)
{
    SMP_WORD t1,t2;
    int c=0;

    if (n <= 0)
        return((SMP_WORD)0);

    for (;;)
    {
        t1=a[0]; t2=b[0];
        r[0]=(t1-t2-c)&SMP_MASK;
        if (t1 != t2) c=(t1 < t2);
        if (--n <= 0) break;

        t1=a[1]; t2=b[1];
        r[1]=(t1-t2-c)&SMP_MASK;
        if (t1 != t2) c=(t1 < t2);
        if (--n <= 0) break;

        t1=a[2]; t2=b[2];
        r[2]=(t1-t2-c)&SMP_MASK;
        if (t1 != t2) c=(t1 < t2);
        if (--n <= 0) break;

        t1=a[3]; t2=b[3];
        r[3]=(t1-t2-c)&SMP_MASK;
        if (t1 != t2) c=(t1 < t2);
        if (--n <= 0) break;

        a+=4;
        b+=4;
        r+=4;
    }
    return((SMP_WORD)c);
}

//!
//! Multiply SMP_WORD array of length num with single SMP_WORD
//! Memory should be allocated for result
//!
//! @param rp Pointer to result
//! @param ap Pointer to SMP_WORD array operand
//! @param num Length of SMP_WORD array
//! @param w Single SMP_WORD operand
//! @return Carry generated from multiplication
//!
SMP_WORD
smp_mul_words(SMP_WORD *rp, const SMP_WORD *ap, int num, SMP_WORD w)
{
    SMP_WORD c=0;
    SMP_WORD bl,bh;

    if (num <= 0)
        return ((SMP_WORD)0);

    bl=LBITS(w);
    bh=HBITS(w);

    for (;;)
    {
        mul(rp[0],ap[0],bl,bh,c);
        if (--num == 0) break;
        mul(rp[1],ap[1],bl,bh,c);
        if (--num == 0) break;
        mul(rp[2],ap[2],bl,bh,c);
        if (--num == 0) break;
        mul(rp[3],ap[3],bl,bh,c);
        if (--num == 0) break;
        ap+=4;
        rp+=4;
    }
    return (c);
}

//!
//! Multiply SMP_WORD array of length num with single SMP_WORD and accumulate result
//! Memory should be allocated for result
//!
//! @param rp Pointer to accumulated result
//! @param ap Pointer to SMP_WORD array operand
//! @param num Length of SMP_WORD array
//! @param w Single SMP_WORD operand
//! @return Carry generated from multiplication
//!
SMP_WORD
smp_mul_add_words(SMP_WORD *rp, const SMP_WORD *ap, int num, SMP_WORD w)
{
    SMP_WORD c=0;
    SMP_WORD bl,bh;

    if (num <= 0)
        return ((SMP_WORD)0);

    bl=LBITS(w);
    bh=HBITS(w);

    for (;;)
    {
        mul_add(rp[0],ap[0],bl,bh,c);
        if (--num == 0) break;
        mul_add(rp[1],ap[1],bl,bh,c);
        if (--num == 0) break;
        mul_add(rp[2],ap[2],bl,bh,c);
        if (--num == 0) break;
        mul_add(rp[3],ap[3],bl,bh,c);
        if (--num == 0) break;
        ap+=4;
        rp+=4;
    }
    return (c);
}

//!
//! Square SMP_WORD array of length n
//! Memory should be allocated for result
//!
//! @param r Pointer to result
//! @param a Pointer to SMP_WORD array operand
//! @param n Length of SMP_WORD array
//!
void
smp_sqr_words(SMP_WORD *r, const SMP_WORD *a, int n)
{
    if (n <= 0)
        return;

    for (;;)
    {
        sqr64(r[0],r[1],a[0]);
        if (--n == 0) break;

        sqr64(r[2],r[3],a[1]);
        if (--n == 0) break;

        sqr64(r[4],r[5],a[2]);
        if (--n == 0) break;

        sqr64(r[6],r[7],a[3]);
        if (--n == 0) break;

        a+=4;
        r+=8;
    }
}
