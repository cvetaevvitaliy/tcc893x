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
//! Calculates a mod m and save result in r
//!
//! @param r a % m result
//! @param a Operand
//! @param m Modulus
//! @return 0 - Fail 1 - Success
//!
int
SMP_mod(SMP *r, const SMP *a, const SMP *m)
{
    return (SMP_div(NULL,r,a,m));
    // note that  r->neg == a->neg  (unless the remainder is zero)
}

//!
//! Like SMP_mod, but returns non-negative remainder
//! (i.e.,  0 <= r < |m|  always holds)
//!
//! @param r a % m result
//! @param a Operand
//! @param m Modulus
//! @return 0 - Fail 1 - Success
//!
int
SMP_nnmod(SMP *r, const SMP *a, const SMP *m)
{

    if (!(SMP_mod(r,a,m)))
        return (0);

    if (!r->neg)
        return (1);

    // now   -|m| < r < 0,  so we have to set  r := r + |m|
    return (m->neg ? SMP_sub : SMP_add)(r, r, m);
}
/*
//!
//! Modular addition
//! Calculates (a + b) mod m and save result in r
//!
//! @param r (a + b) % m result
//! @param a Operand
//! @param b Operand
//! @param m Modulus
//! @return 0 - Fail 1 - Success
//!
int
SMP_mod_add(SMP *r, const SMP *a, const SMP *b, const SMP *m)
{
    if (!SMP_add(r, a, b))
        return (0);

    return SMP_nnmod(r, r, m);
}

//!
//! SMP_mod_add variant that may be used if both
//! a and b are non-negative and less than  m
//!
//! @param r (a + b) % m result
//! @param a Operand
//! @param b Operand
//! @param m Modulus
//! @return 0 - Fail 1 - Success
//!
int
SMP_mod_add_quick(SMP *r, const SMP *a, const SMP *b, const SMP *m)
{
    if (!SMP_add(r, a, b))
        return (0);

    if (SMP_ucmp(r, m) >= 0)
        return SMP_usub(r, r, m);

    return (1);
}

//!
//! Modular subtraction
//! Calculates (a - b) mod m and save result in r
//!
//! @param r (a - b) % m result
//! @param a Operand
//! @param b Operand
//! @param m Modulus
//! @return 0 - Fail 1 - Success
//!
int
SMP_mod_sub(SMP *r, const SMP *a, const SMP *b, const SMP *m)
{
    if (!SMP_sub(r, a, b))
        return (0);

    return SMP_nnmod(r, r, m);
}

//!
//! SMP_mod_sub variant that may be used if both
//! a and b are non-negative and less than m
//!
//! @param r (a - b) % m result
//! @param a Operand
//! @param b Operand
//! @param m Modulus
//! @return 0 - Fail 1 - Success
//!
int
SMP_mod_sub_quick(SMP *r, const SMP *a, const SMP *b, const SMP *m)
{
    if (!SMP_sub(r, a, b))
        return (0);

    if (r->neg)
        return SMP_add(r, r, m);

    return (1);
}
*/
//!
//! Modular multiplication
//! Calculates (a * b) mod m and save result in r
//!
//! @param r (a * b) % m result
//! @param a Operand
//! @param b Operand
//! @param m Modulus
//! @return 0 - Fail 1 - Success
//!
int
SMP_mod_mul(SMP *r, const SMP *a, const SMP *b, const SMP *m)
{
    int ret = 0;
    SMP smpt;
    SMP *t = &smpt;

    smp_check_top(a);
    smp_check_top(b);
    smp_check_top(m);

    SMP_init(t);
    if (a == b)
    {
        if (!SMP_sqr(t,a))
        goto err;
    }
    else
    {
        if (!SMP_mul(t,a,b))
        goto err;
    }

    if (!SMP_nnmod(r,t,m))
        goto err;

    ret = 1;

err:
    SMP_free(t);
    return (ret);
}
/*
//!
//! Modular Square
//! Calculates (a ^ 2) mod m and save result in r
//!
//! @param r (a ^ 2) % m result
//! @param a Operand
//! @param m Modulus
//! @return 0 - Fail 1 - Success
//!
int
SMP_mod_sqr(SMP *r, const SMP *a, const SMP *m)
{
    if (!SMP_sqr(r, a))
        return (0);

    // r->neg == 0,  thus we don't need SMP_nnmod
    return SMP_mod(r, r, m);
}

//!
//! Modular left shift by 1
//! Calculates (a << 1) mod m and save result in r
//!
//! @param r (a << 1) % m result
//! @param a Operand
//! @param m Modulus
//! @return 0 - Fail 1 - Success
//!
int
SMP_mod_lshift1(SMP *r, const SMP *a, const SMP *m)
{
    if (!SMP_lshift1(r, a))
        return (0);

    return SMP_nnmod(r, r, m);
}

//!
//! SMP_mod_lshift1 variant that may be used if
//! a is non-negative and less than  m
//!
//! @param r (a << 1) % m result
//! @param a Operand
//! @param m Modulus
//! @return 0 - Fail 1 - Success
//!
int
SMP_mod_lshift1_quick(SMP *r, const SMP *a, const SMP *m)
{
    if (!SMP_lshift1(r, a))
        return (0);

    if (SMP_cmp(r, m) >= 0)
        return SMP_sub(r, r, m);

    return (1);
}

//!
//! Modular left shift by n
//! Calculates (a << n) mod m and save result in r
//!
//! @param r (a << n) % m result
//! @param a Operand
//! @param n Shift amount
//! @param m Modulus
//! @return 0 - Fail 1 - Success
//!
int
SMP_mod_lshift(SMP *r, const SMP *a, int n, const SMP *m)
{
    SMP *abs_m = NULL;
    int ret;

    if (!SMP_nnmod(r, a, m))
        return 0;

    if (m->neg)
    {
        abs_m = SMP_dup((SMP*)m);
        if (abs_m == NULL)
            return 0;
        abs_m->neg = 0;
    }

    ret = SMP_mod_lshift_quick(r, r, n, (abs_m ? abs_m : m));

    if (abs_m)
        SMP_free(abs_m);

    return ret;
}

//!
//! SMP_mod_lshift variant that may be used if
//! a is non-negative and less than m
//!
//! @param r (a << n) % m result
//! @param a Operand
//! @param n Shift amount
//! @param m Modulus
//! @return 0 - Fail 1 - Success
//!
int
SMP_mod_lshift_quick(SMP *r, const SMP *a, int n, const SMP *m)
{
    if (r != a)
    {
        if (SMP_copy(r, a) == NULL)
            return (0);
    }

    while (n > 0)
    {
        int max_shift;

        // 0 < r < m
        max_shift = SMP_num_bits(m) - SMP_num_bits(r);
        // max_shift >= 0

        if (max_shift < 0)
            return (0);

        if (max_shift > n)
            max_shift = n;

        if (max_shift)
        {
            if (!SMP_lshift(r, r, max_shift))
                return (0);
            n -= max_shift;
        }
        else
        {
            if (!SMP_lshift1(r, r))
                return 0;
            --n;
        }

        // SMP_num_bits(r) <= SMP_num_bits(m)

        if (SMP_cmp(r, m) >= 0)
        {
            if (!SMP_sub(r, r, m))
                return (0);
        }
    }

    return (1);
}
*/
//!
//! Computes inverse of a mod n
//! Allocates new SMP data structure memory if in is NULL
//!
//! @param in (a ^ -1) % m result
//! @param a Operand
//! @param n Modulus
//! @return Pointer to inverse result SMP data structure
//!
SMP*
SMP_mod_inverse(SMP *in, const SMP *a, const SMP *n)
{
    int sign;
    SMP *ret = NULL;

    SMP smpA, smpB, smpX, smpY, smpM, smpD, smpT;
    SMP *A = &smpA;
    SMP *B = &smpB;
    SMP *X = &smpX;
    SMP *Y = &smpY;
    SMP *M = &smpM;
    SMP *D = &smpD;
    SMP *T = &smpT;
    SMP *R = NULL;

    smp_check_top(a);
    smp_check_top(n);

    SMP_init(A);
    SMP_init(B);
    SMP_init(X);
    SMP_init(Y);
    SMP_init(M);
    SMP_init(D);
    SMP_init(T);

    if (in == NULL)
        R = SMP_new();
    else
        R = in;
    if (R == NULL)
        return ret;

    SMP_set_word((X),1);
    SMP_set_word((Y),0);
    if (SMP_copy(B,a) == NULL) goto err;
    if (SMP_copy(A,n) == NULL) goto err;
    A->neg = 0;
    if (B->neg || (SMP_ucmp(B, A) >= 0))
    {
        if (!SMP_nnmod(B, B, A))
            goto err;
    }

    sign = -1;
    // From  B = a mod |n|,  A = |n|  it follows that
    //
    //      0 <= B < A,
    //     -sign*X*a  ==  B   (mod |n|),
    //      sign*Y*a  ==  A   (mod |n|).

    if (SMP_is_odd(n) && (SMP_num_bits(n) <= 450))
    {
        // Binary inversion algorithm; requires odd modulus.
        // This is faster than the general algorithm if the modulus
        // is sufficiently small (about 400 .. 500 bits on 32-bit
        // sytems, but much more on 64-bit systems)
        int shift;

        while (!SMP_is_zero(B))
        {
            //      0 < B < |n|,
            //      0 < A <= |n|,
            // (1) -sign*X*a  ==  B   (mod |n|),
            // (2)  sign*Y*a  ==  A   (mod |n|)
            //
            // Now divide  B  by the maximum possible power of two in the integers,
            // and divide  X  by the same value mod |n|.
            // When we're done, (1) still holds.
            shift = 0;
            while (!SMP_is_bit_set(B, shift)) // note that 0 < B
            {
                shift++;

                if (SMP_is_odd(X))
                {
                    if (!SMP_uadd(X, X, n)) goto err;
                }
                // now X is even, so we can easily divide it by two
                if (!SMP_rshift1(X, X)) goto err;
            }
            if (shift > 0)
            {
                if (!SMP_rshift(B, B, shift)) goto err;
            }


            // Same for  A  and  Y.  Afterwards, (2) still holds.
            shift = 0;
            while (!SMP_is_bit_set(A, shift)) // note that 0 < A
            {
                shift++;

                if (SMP_is_odd(Y))
                {
                    if (!SMP_uadd(Y, Y, n)) goto err;
                }
                // now Y is even
                if (!SMP_rshift1(Y, Y)) goto err;
            }
            if (shift > 0)
            {
                if (!SMP_rshift(A, A, shift)) goto err;
            }


            // We still have (1) and (2).
            // Both  A  and  B  are odd.
            // The following computations ensure that
            //
            //     0 <= B < |n|,
            //      0 < A < |n|,
            // (1) -sign*X*a  ==  B   (mod |n|),
            // (2)  sign*Y*a  ==  A   (mod |n|),
            //
            // and that either  A  or  B  is even in the next iteration.

            if (SMP_ucmp(B, A) >= 0)
            {
                // -sign*(X + Y)*a == B - A  (mod |n|)
                if (!SMP_uadd(X, X, Y)) goto err;
                // NB: we could use SMP_mod_add_quick(X, X, Y, n), but that
                // actually makes the algorithm slower
                if (!SMP_usub(B, B, A)) goto err;
            }
            else
            {
                //  sign*(X + Y)*a == A - B  (mod |n|)
                if (!SMP_uadd(Y, Y, X)) goto err;
                // as above, SMP_mod_add_quick(Y, Y, X, n) would slow things down
                if (!SMP_usub(A, A, B)) goto err;
            }
        }
    }
    else
    {
        // general inversion algorithm

        while (!SMP_is_zero(B))
        {
            SMP *tmp;

            //
            //      0 < B < A,
            // (*) -sign*X*a  ==  B   (mod |n|),
            //      sign*Y*a  ==  A   (mod |n|)

            // (D, M) := (A/B, A%B) ...
            if (SMP_num_bits(A) == SMP_num_bits(B))
            {
                if (!SMP_set_word(D,1)) goto err;
                if (!SMP_sub(M,A,B)) goto err;
            }
            else if (SMP_num_bits(A) == SMP_num_bits(B) + 1)
            {
                // A/B is 1, 2, or 3
                if (!SMP_lshift1(T,B)) goto err;
                if (SMP_ucmp(A,T) < 0)
                {
                    // A < 2*B, so D=1
                    if (!SMP_set_word(D,1)) goto err;
                    if (!SMP_sub(M,A,B)) goto err;
                }
                else
                {
                    // A >= 2*B, so D=2 or D=3
                    if (!SMP_sub(M,A,T)) goto err;
                    if (!SMP_add(D,T,B)) goto err; // use D (:= 3*B) as temp
                    if (SMP_ucmp(A,D) < 0)
                    {
                        // A < 3*B, so D=2
                        if (!SMP_set_word(D,2)) goto err;
                        // M (= A - 2*B) already has the correct value
                    }
                    else
                    {
                        // only D=3 remains
                        if (!SMP_set_word(D,3)) goto err;
                        // currently  M = A - 2*B,  but we need  M = A - 3*B
                        if (!SMP_sub(M,M,B)) goto err;
                    }
                }
            }
            else
            {
                if (!SMP_div(D,M,A,B)) goto err;
            }

            // Now
            //      A = D*B + M;
            // thus we have
            // (**)  sign*Y*a  ==  D*B + M   (mod |n|).

            tmp = A; // keep the SMP object, the value does not matter

            // (A, B) := (B, A mod B) ...
            A = B;
            B = M;
            // ... so we have  0 <= B < A  again

            // Since the former  M  is now  B  and the former  B  is now  A,
            // (**) translates into
            //       sign*Y*a  ==  D*A + B    (mod |n|),
            // i.e.
            //       sign*Y*a - D*A  ==  B    (mod |n|).
            // Similarly, (*) translates into
            //      -sign*X*a  ==  A          (mod |n|).
            //
            // Thus,
            //   sign*Y*a + D*sign*X*a  ==  B  (mod |n|),
            // i.e.
            //        sign*(Y + D*X)*a  ==  B  (mod |n|).
            //
            // So if we set  (X, Y, sign) := (Y + D*X, X, -sign),  we arrive back at
            //      -sign*X*a  ==  B   (mod |n|),
            //       sign*Y*a  ==  A   (mod |n|).
            // Note that  X  and  Y  stay non-negative all the time.

            // most of the time D is very small, so we can optimize tmp := D*X+Y
            if (SMP_is_one(D))
            {
                if (!SMP_add(tmp,X,Y)) goto err;
            }
            else
            {
                if (SMP_is_word(D,2))
                {
                    if (!SMP_lshift1(tmp,X)) goto err;
                }
                else if (SMP_is_word(D,4))
                {
                    if (!SMP_lshift(tmp,X,2)) goto err;
                }
                else if (D->top == 1)
                {
                    if (!SMP_copy(tmp,X)) goto err;
                    if (!SMP_mul_word(tmp,D->d[0])) goto err;
                }
                else
                {
                    if (!SMP_mul(tmp,D,X)) goto err;
                }
                if (!SMP_add(tmp,tmp,Y)) goto err;
            }

            M = Y; // keep the SMP object, the value does not matter
            Y = X;
            X = tmp;
            sign = -sign;
        }
    }

    //
    // The while loop (Euclid's algorithm) ends when
    //      A == gcd(a,n);
    // we have
    //       sign*Y*a  ==  A  (mod |n|),
    // where  Y  is non-negative.

    if (sign < 0)
    {
        if (!SMP_sub(Y,n,Y)) goto err;
    }
    // Now  Y*a  ==  A  (mod |n|).

    if (SMP_is_one(A))
    {
        // Y*a == 1  (mod |n|)
        if (!Y->neg && SMP_ucmp(Y,n) < 0)
        {
            if (!SMP_copy(R,Y)) goto err;
        }
        else
        {
            if (!SMP_nnmod(R,Y,n)) goto err;
        }
    }
    else
    {
        SMP_ERR(SMP_ERR_NOINVERSE);
        goto err;
    }

    ret = R;

err:

    if ((ret == NULL) && (in == NULL))
        SMP_free(R);
    SMP_free(T);
    SMP_free(D);
    SMP_free(M);
    SMP_free(Y);
    SMP_free(X);
    SMP_free(B);
    SMP_free(A);

    return(ret);
}
