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

#ifndef __SMP_H__
#define __SMP_H__

#ifdef  __cplusplus
extern "C" {
#endif

#define FILEOUT "fileout.txt"

//-------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------

#define SMP_WORD       unsigned long   //!< Basic word length (32-bit)
#define SMP_BYTES      4               //!< Number of bytes in SMP_WORD
#define SMP_BITS       32              //!< Number of bits in SMP_WORD
#define SMP_BITS2      16              //!< Number of bits in SMP_WORD/2
#define SMP_MASK       0xffffffffL     //!< SMP_WORD mask
#define SMP_MASKh      0xffff0000L     //!< Higher half mask
#define SMP_MASKl      0xffff          //!< Lower half mask
#define SMP_MASK15     0x00007fffL     //!< 15-bit mask
#define SMP_MASK17     0xffff8000L     //!< 17-bit mask
#define SMP_TBIT       0x80000000L     //!< SMP_WORD MSB-bit mask
#define SMP_DEC_CONV   1000000000L     //!< Decimal division factor
#define SMP_DEC_FMT1   "%lu"           //!< Decimal representation format
#define SMP_DEC_FMT2   "%09lu"         //!< Decimal representation format with 0 padding
#define SMP_DEC_NUM    9               //!< Number of decimal digits representable

#define SMP_FLG_MALLOCED     0x01  //!< Memory allocated flag for SMP
#define SMP_FLG_STATIC_DATA  0x02  //!< Static memory flag for SMP.d

//! Set flag macro
#define SMP_set_flags(a,n)    ((a)->flags|=(n))
//! Get flag macro
#define SMP_get_flags(a,n)    ((a)->flags&(n))

//! Expand internal memory by bit size
#define SMP_expand_b(a,bits)  ((((((bits+SMP_BITS-1))/SMP_BITS)) <= (a)->dmax) \
                              ? (a) : smp_expand_free((a),(bits)/SMP_BITS+1))
//! Expand internal memory by word size
#define SMP_expand_w(a,words) (((words) <= (a)->dmax) \
                              ? (a) : smp_expand_free((a),(words)))

//! Number of effective bytes in SMP data structure
#define SMP_num_bytes(a)      ((SMP_num_bits(a)+7)/8)

//! Remove leading zeros to fix top
#define SMP_fix_top(a) { \
    SMP_WORD *ftl; \
    if ((a)->top > 0) \
    { \
        for (ftl = &((a)->d[(a)->top-1]); (a)->top > 0; (a)->top--) \
        if (*(ftl--)) break; \
    } \
}

//! Absolute value of SMP data a is equal to w
#define SMP_abs_is_word(a,w) (((a)->top == 1) && ((a)->d[0] == (SMP_WORD)(w)))

//! SMP data a is equal to zero
#define SMP_is_zero(a)       (((a)->top == 0) || SMP_abs_is_word(a,0))

//! SMP data a is equal to one
#define SMP_is_one(a)        (SMP_abs_is_word((a),1) && !(a)->neg)

//! Value of SMP data a is equal to w
#define SMP_is_word(a,w)     ((w) ? SMP_abs_is_word((a),(w)) && !(a)->neg : SMP_is_zero((a)))

//! Value of SMP data a is odd
#define SMP_is_odd(a)         (((a)->top > 0) && ((a)->d[0] & 1))

//-------------------------------------------------------------------
// Macros for exponentiation
//-------------------------------------------------------------------

//! Number of maximum tables needed for sliding window
#define SMP_TABLE_SIZE 32

//! Size of window for sliding window mod_exp functions
#define SMP_window_bits_for_exponent_size(b) \
        ((b) > 671 ? 6 : \
         (b) > 239 ? 5 : \
         (b) >  79 ? 4 : \
         (b) >  23 ? 3 : 1)

//-------------------------------------------------------------------
// Macros for arithmetics
//-------------------------------------------------------------------

//! Lower 16-bit of SMP_WORD
#define LBITS(a)    ((a) & SMP_MASKl)

//! Higher 16-bit of SMP_WORD
#define HBITS(a)    (((a)>>SMP_BITS2) & SMP_MASKl)

//! Shift lower 16-bit to higher 16-bit
#define    L2HBITS(a)    (((a)<<SMP_BITS2) & SMP_MASK)

//! Multiply two 32-bit data to get 64-bit result
#define mul64(l,h,bl,bh) { \
    SMP_WORD m,m1,lt,ht; \
    lt = l; \
    ht = h; \
    m  = (bh)*(lt); \
    lt = (bl)*(lt); \
    m1 = (bl)*(ht); \
    ht = (bh)*(ht); \
    m  = (m+m1)&SMP_MASK; if (m < m1) ht+=L2HBITS((SMP_WORD)1); \
    ht += HBITS(m); \
    m1 = L2HBITS(m); \
    lt = (lt+m1)&SMP_MASK; if (lt < m1) ht++; \
    (l) = lt; \
    (h) = ht; \
}

//! Square 32-bit data to get 64-bit result
#define sqr64(lo,ho,in) { \
    SMP_WORD l,h,m; \
    h = (in); \
    l = LBITS(h); \
    h = HBITS(h); \
    m = (l)*(h); \
    l *= l; \
    h *= h; \
    h += (m&SMP_MASK17)>>(SMP_BITS2-1); \
    m = (m&SMP_MASK15)<<(SMP_BITS2+1); \
    l = (l+m)&SMP_MASK; if (l < m) h++; \
    (lo) = l; \
    (ho) = h; \
}

//! Multiply two 32-bit data with carry-in
#define mul(r,a,bl,bh,c) { \
    SMP_WORD l,h; \
    h = (a); \
    l = LBITS(h); \
    h = HBITS(h); \
    mul64(l,h,(bl),(bh)); \
    l += (c); if ((l&SMP_MASK) < (c)) h++; \
    (c) = h&SMP_MASK; \
    (r) = l&SMP_MASK; \
}

//! Multiply two 32-bit data with carry-in and accumulate result
#define mul_add(r,a,bl,bh,c) { \
    SMP_WORD l,h; \
    h = (a); \
    l = LBITS(h); \
    h = HBITS(h); \
    mul64(l,h,(bl),(bh)); \
    l = (l+(c))&SMP_MASK; if (l < (c)) h++; \
    (c) = (r); \
    l = (l+(c))&SMP_MASK; if (l < (c)) h++; \
    (c) = h&SMP_MASK; \
    (r) = l; \
}

//-------------------------------------------------------------------
// Data Types
//-------------------------------------------------------------------

//! Multi precision data structure
typedef struct _smp_st_
{
    SMP_WORD *d;   //!< Pointer to an array of 32-bit chunks.
    int top;       //!< Index of last used d +1.
    int dmax;      //!< Size of the d array.
    int neg;       //!< One if the number is negative
    int flags;     //!< Control flags
} SMP;

//-------------------------------------------------------------------
// Function Prototypes
//-------------------------------------------------------------------

void *SMP_mem_malloc(unsigned int size);
void SMP_mem_free(void *ptr);

SMP* SMP_new(void);
void SMP_init(SMP *smp);
void SMP_clear(SMP *smp);
void SMP_free(SMP *smp);

SMP_WORD* smp_expand_internal(const SMP *smp, int size);
SMP* smp_expand_free(SMP *smp, int size);

////SMP* SMP_dup(SMP *a);
SMP* SMP_copy(SMP *a, const SMP *b);
////void SMP_swap(SMP *a, SMP *b);
int SMP_ucmp(const SMP *a, const SMP *b);
int SMP_cmp(const SMP *a, const SMP *b);

int SMP_num_bits_word(SMP_WORD l);
int SMP_num_bits(const SMP *a);
int SMP_is_bit_set(const SMP *a, int n);
int SMP_set_bit(SMP *a, int n);
int SMP_mask_bits(SMP *a, int n);
const SMP* SMP_value_one(void);

////SMP_WORD SMP_get_word(const SMP *a);
int SMP_set_word(SMP *a, SMP_WORD w);

SMP* SMP_bin2mp(const unsigned char *s, int len, SMP *ret);
////int SMP_mp2bin(const SMP *a, unsigned char *to);
////int SMP_hex2mp(SMP **smp, const char *a);

int SMP_lshift1(SMP *r, const SMP *a);
int SMP_rshift1(SMP *r, const SMP *a);
int SMP_lshift(SMP *r, const SMP *a, int n);
int SMP_rshift(SMP *r, const SMP *a, int n);

SMP_WORD smp_add_words(SMP_WORD *r, const SMP_WORD *a, const SMP_WORD *b, int n);
int SMP_add(SMP *r, const SMP *a, const SMP *b);
int SMP_uadd(SMP *r, const SMP *a, const SMP *b);
int SMP_add_word(SMP *a, SMP_WORD w);

SMP_WORD smp_sub_words(SMP_WORD *r, const SMP_WORD *a, const SMP_WORD *b, int n);
int SMP_sub(SMP *r, const SMP *a, const SMP *b);
int SMP_usub(SMP *r, const SMP *a, const SMP *b);
int SMP_sub_word(SMP *a, SMP_WORD w);

SMP_WORD smp_mul_words(SMP_WORD *rp, const SMP_WORD *ap, int num, SMP_WORD w);
SMP_WORD smp_mul_add_words(SMP_WORD *rp, const SMP_WORD *ap, int num, SMP_WORD w);
void smp_mul_normal(SMP_WORD *r, SMP_WORD *a, int na, SMP_WORD *b, int nb);
int SMP_mul(SMP *r, const SMP *a, const SMP *b);
int SMP_mul_word(SMP *a, SMP_WORD w);

void smp_sqr_words(SMP_WORD *r, const SMP_WORD *a, int n);
void smp_sqr_normal(SMP_WORD *r, const SMP_WORD *a, int n, SMP_WORD *tmp);
int SMP_sqr(SMP *r, const SMP *a);

////int SMP_exp(SMP *r, const SMP *a, const SMP *p);

SMP_WORD smp_div_words(SMP_WORD h, SMP_WORD l, SMP_WORD d);
int SMP_div(SMP *dv, SMP *rm, const SMP *num, const SMP *divisor);
////SMP_WORD SMP_div_word(SMP *a, SMP_WORD w);
////SMP_WORD SMP_mod_word(const SMP *a, SMP_WORD w);

////SMP* smp_euclid(SMP *a, SMP *b);
////int SMP_gcd(SMP *r, const SMP *a, const SMP *b);

int SMP_mod(SMP *r, const SMP *a, const SMP *m);
int SMP_nnmod(SMP *r, const SMP *a, const SMP *m);
////int SMP_mod_add(SMP *r, const SMP *a, const SMP *b, const SMP *m);
////int SMP_mod_add_quick(SMP *r, const SMP *a, const SMP *b, const SMP *m);
////int SMP_mod_sub(SMP *r, const SMP *a, const SMP *b, const SMP *m);
////int SMP_mod_sub_quick(SMP *r, const SMP *a, const SMP *b, const SMP *m);
int SMP_mod_mul(SMP *r, const SMP *a, const SMP *b, const SMP *m);
////int SMP_mod_sqr(SMP *r, const SMP *a, const SMP *m);
int SMP_mod_exp(SMP *r, const SMP *a, const SMP *p, const SMP *m);
int SMP_mod_exp_simple(SMP *r, const SMP *a, const SMP *p, const SMP *m);
////int SMP_mod_lshift1(SMP *r, const SMP *a, const SMP *m);
////int SMP_mod_lshift1_quick(SMP *r, const SMP *a, const SMP *m);
////int SMP_mod_lshift(SMP *r, const SMP *a, int n, const SMP *m);
////int SMP_mod_lshift_quick(SMP *r, const SMP *a, int n, const SMP *m);
SMP* SMP_mod_inverse(SMP *in, const SMP *a, const SMP *n);



#ifdef  __cplusplus
}
#endif
#endif
