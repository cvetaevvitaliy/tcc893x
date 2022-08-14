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
#include <ctype.h>

/* Memory allocation check variables
#include <malloc.h>
*/

// SMP header
#include "smp.h"
#include "smp_err.h"

//-------------------------------------------------------------------
// Function Implementation
//-------------------------------------------------------------------

/* Memory allocation check variables
FILE *fp = NULL;
static int nalloc=0, max=0;
*/

//!
//! Allocate memory space
//!
//! @param size Size of memory to allocate
//! @return Address pointer to allocated memory
//!
void*
SMP_mem_malloc(unsigned int size)
{
    void* ptr = NULL;

    ptr = malloc(size);
    if (ptr == NULL)
        SMP_ERR(SMP_ERR_MALLOC);

/* Memory allocation check
nalloc += size;
if (nalloc > max) max = nalloc;
fp = fopen(FILEOUT, "a");
fprintf(fp, "Alloc = %d (Total = %d , Max = %d)\n", size, nalloc, max);
fclose(fp);
*/

    return ptr;
}

//!
//! Free memory space
//!
//! @param ptr memory pointer to free
//!
void
SMP_mem_free(void *ptr)
{
    if (ptr == NULL)
        return;

/* Memory allocation check
nalloc -= _msize(ptr);
fp = fopen(FILEOUT, "a");
fprintf(fp, "Free = %d (Total = %d , Max = %d)\n", _msize(ptr), nalloc, max);
fclose(fp);
*/

    free(ptr);
}

//!
//! SMP data structure memory allocation
//!
//! @return SMP data structure pointer
//!
SMP*
SMP_new(void)
{
    SMP *ptr = NULL;

    ptr = (SMP*)SMP_mem_malloc(sizeof(SMP));
    if (ptr)
    {
        ptr->d = NULL;
        ptr->top = 0;
        ptr->neg = 0;
        ptr->dmax = 0;
        ptr->flags = SMP_FLG_MALLOCED;
    }

    return ptr;
}

//!
//! SMP data structure initialization
//! Do NOT use this function after SMP_new()
//! because this will clear SMP_FLG_MALLOCED
//! and leak memory when freed
//!
//! @param smp SMP data structure
//!
void
SMP_init(SMP *smp)
{
    memset(smp, 0, sizeof(SMP));
}

//!
//! Clear SMP.d and reset SMP.top and SMP.neg
//!
//! @param smp SMP data structure
//!
void
SMP_clear(SMP *smp)
{
    if (smp->d != NULL)
        memset(smp->d,0,smp->dmax*sizeof(smp->d[0]));
    smp->top=0;
    smp->neg=0;
}

//!
//! Free SMP data structure
//!
//! @param smp SMP data structure pointer to free
//!
void
SMP_free(SMP *smp)
{
    if (smp == NULL)
        return;

    SMP_clear(smp);

    // (smp->dmax > 0) added by Juno 20040614
    if ((smp->d != NULL) && (smp->dmax > 0))
    {
        SMP_mem_free(smp->d);
        smp->dmax=0;
    }

    if (SMP_get_flags(smp, SMP_FLG_MALLOCED))
        SMP_mem_free(smp);
}

//!
//! Reallocates memory for internal SMP data structure
//! This is used when SMP.dmax is smaller than required size
//!
//! @param smp SMP data structure to reallocate memory
//! @param size Size of internal memory to reallocate in number of words
//! @return Reallocated memory address
//!
SMP_WORD*
smp_expand_internal(const SMP *smp, int size)
{
    SMP_WORD *newd = NULL;

    smp_check_top(smp);
    if (SMP_get_flags(smp, SMP_FLG_STATIC_DATA))
    {
        SMP_ERR(SMP_ERR_STATIC);
        return NULL;
    }

    newd = (SMP_WORD*)SMP_mem_malloc(sizeof(SMP_WORD)*(size+1));
    if (newd == NULL)
        return NULL;

    memset(newd, 0, sizeof(SMP_WORD)*(size+1));
    memcpy(newd, smp->d, sizeof(smp->d[0])*smp->top);

    return newd;
}

//!
//! Reallocates memory for SMP data structure
//! This function calls smp_expand_internal()
//! It frees the old memory allocated
//!
//! @param smp SMP data structure to reallocate memory
//! @param size Size of internal memory to reallocate in number of words
//! @return Reallocated SMP data structure
//!
SMP*
smp_expand_free(SMP *smp, int size)
{
    SMP_WORD *newd = NULL;

    if (size > smp->dmax)
    {
        newd = smp_expand_internal(smp, size);

        if (newd)
        {
            // (smp->dmax > 0) added by Juno 20040614
            if (smp->d && (smp->dmax > 0))
                SMP_mem_free(smp->d);

            smp->d = newd;
            smp->dmax = size;
        }
        else
            smp = NULL;
    }

    return smp;
}

/*
//!
//! Duplicate a SMP data structure
//! Allocates new internal and SMP data structure memory
//!
//! @param a Source SMP data structure
//! @return Address of destination SMP data structure
//!
SMP*
SMP_dup(SMP *a)
{
    SMP *r, *t;

    if (a == NULL)
        return NULL;

    smp_check_top(a);

    t = SMP_new();
    if (t == NULL)
        return NULL;

    r = SMP_copy(t, a);
    // now  r == t || r == NULL
    if (r == NULL)
        SMP_mem_free(t);

    return r;
}
*/

//!
//! Copy a SMP data structure
//! Allocates new internal memory if necessary
//! SMP data structure itself must be allocated
//!
//! @param a Destination SMP data structure
//! @param b Source SMP data structure
//! @return Address of destination SMP data structure
//!
SMP*
SMP_copy(SMP *a, const SMP *b)
{
    smp_check_top(b);

    if (a == NULL)
    {
        SMP_ERR(SMP_ERR_NULL);
        return NULL;
    }
    if (a == b) return(a);

    if (SMP_expand_w(a,b->top) == NULL) return(NULL);
    memcpy(a->d, b->d, sizeof(b->d[0])*b->top);

    a->top = b->top;
    if ((a->top == 0) && (a->d != NULL))
        a->d[0]=0;
    a->neg = b->neg;

    return a;
}

//!
//! Swap SMP data structure
//! No memory reallocation
//!
//! @param a SMP data structure to be swapped
//! @param b SMP data structure to be swapped
//!
void
SMP_swap(SMP *a, SMP *b)
{
    SMP_WORD *tmp_d;
    int flags_old_a, flags_old_b;
    int tmp_top, tmp_dmax, tmp_neg;

    flags_old_a = a->flags;
    flags_old_b = b->flags;

    tmp_d = a->d;
    tmp_top = a->top;
    tmp_dmax = a->dmax;
    tmp_neg = a->neg;

    a->d = b->d;
    a->top = b->top;
    a->dmax = b->dmax;
    a->neg = b->neg;

    b->d = tmp_d;
    b->top = tmp_top;
    b->dmax = tmp_dmax;
    b->neg = tmp_neg;

    a->flags = (flags_old_a & SMP_FLG_MALLOCED) | (flags_old_b & SMP_FLG_STATIC_DATA);
    b->flags = (flags_old_b & SMP_FLG_MALLOCED) | (flags_old_a & SMP_FLG_STATIC_DATA);
}

//!
//! Compares the absolute value of a and b
//!
//! @param a SMP data structure a
//! @param b SMP data structure b
//! @return 0 if |a| = |b|, positive if |a| > |b|, negative if |a| < |b|
//!
int
SMP_ucmp(const SMP *a, const SMP *b)
{
    int i;
    SMP_WORD t1,t2,*ap,*bp;

    smp_check_top(a);
    smp_check_top(b);

    i = a->top - b->top;
    if (i != 0)
        return i;

    ap=a->d;
    bp=b->d;
    for ( i = a->top-1 ; i >= 0 ; i-- )
    {
        t1= ap[i];
        t2= bp[i];
        if (t1 != t2)
            return (t1 > t2 ? 1 : -1);
    }

    return 0;
}

//!
//! Compares the value of a and b
//!
//! @param a SMP data structure a
//! @param b SMP data structure b
//! @return 0 if a = b, positive if a > b, negative if a < b
//!
int
SMP_cmp(const SMP *a, const SMP *b)
{
    int i;
    int gt,lt;
    SMP_WORD t1,t2;

    if ((a == NULL) || (b == NULL))
    {
        if (a != NULL)
            return -1;
        else if (b != NULL)
            return 1;
        else
            return 0;
    }

    smp_check_top(a);
    smp_check_top(b);

    if (a->neg != b->neg)
    {
        if (a->neg)
            return -1;
        else
            return 1;
    }

    if (a->neg == 0)
    {
        gt =  1;
        lt = -1;
    }
    else
    {
        gt = -1;
        lt =  1;
    }

    if (a->top > b->top)
        return gt;
    if (a->top < b->top)
        return lt;

    for (i=a->top-1; i>=0; i--)
    {
        t1=a->d[i];
        t2=b->d[i];
        if (t1 > t2)
            return gt;
        if (t1 < t2)
            return lt;
    }

    return 0;
}

//!
//! Calculates number of effective bits in a word
//!
//! @param l Word to calculate effective bits
//! @return Number of effective bits
//!
int
SMP_num_bits_word(SMP_WORD l)
{
    static const char bits[256]={
        0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,
        5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        };

        if (l & 0xffff0000L)
        {
            if (l & 0xff000000L)
                return(bits[(int)(l>>24L)]+24);
            else
                return(bits[(int)(l>>16L)]+16);
        }
        else
        {
            if (l & 0x0000ff00L)
                return (bits[(int)(l>>8)]+8);
            else
                return (bits[(int)(l   )]  );
        }
}

//!
//! Calculates number of effective bits in SMP data structure
//!
//! @param a SMP data structure to calculate effective bits
//! @return Number of effective bits
//!
int
SMP_num_bits(const SMP *a)
{
    SMP_WORD l;
    int i;

    smp_check_top(a);

    if (a->top == 0)
        return 0;

    l = a->d[a->top-1];
    assert(l != 0);
    i = (a->top-1)*SMP_BITS;

    return (i+SMP_num_bits_word(l));
}

//!
//! Check if nth bit of SMP data structure is set
//!
//! @param a SMP data structure to check
//! @param n Bit index to check
//! @return 0 - Unset 1 - Set
//!
int
SMP_is_bit_set(const SMP *a, int n)
{
    int i,j;

    if (n < 0)
        return 0;

    i=n/SMP_BITS;
    j=n%SMP_BITS;

    if (a->top <= i)
        return 0;

    return((a->d[i]&(((SMP_WORD)1)<<j))?1:0);
}

//!
//! Set nth bit of SMP data structure
//!
//! @param a SMP data structure to set
//! @param n Bit index to set
//! @return 0 - Fail 1 - Success
//!
int
SMP_set_bit(SMP *a, int n)
{
    int i,j,k;

    i=n/SMP_BITS;
    j=n%SMP_BITS;
    if (a->top <= i)
    {
        if (SMP_expand_w(a,i+1) == NULL)
            return (0);
        for(k=a->top; k<i+1; k++)
            a->d[k]=0;
        a->top=i+1;
    }

    a->d[i]|=(((SMP_WORD)1)<<j);

    return (1);
}

//!
//! Mask n-bits of SMP data structure
//!
//! @param a SMP data structure to mask
//! @param n Number of bits to mask
//! @return 0 - Fail 1 - Success
//!
int
SMP_mask_bits(SMP *a, int n)
{
    int b,w;

    w=n/SMP_BITS;
    b=n%SMP_BITS;
    if (w >= a->top)
        return(0);
    if (b == 0)
        a->top=w;
    else
    {
        a->top=w+1;
        a->d[w]&= ~(SMP_MASK<<b);
    }

    SMP_fix_top(a);

    return (1);
}

//!
//! Value one in SMP data structure
//!
//! @return Address of static SMP data structure with value one
//!
const SMP*
SMP_value_one(void)
{
    static SMP_WORD data_one = 1L;
    static SMP const_one;

    SMP_init(&const_one);
    const_one.d = &data_one;
    const_one.top = 1;
    const_one.dmax = 1;
    const_one.neg = 0;
    const_one.flags = SMP_FLG_STATIC_DATA;

    return (&const_one);
}
/*
//!
//! Get single word from SMP data structure
//! If SMP data structure contains more data
//! than a single word SMP_MASK is returned
//!
//! @param a SMP data structure
//! @return SMP_MASK - Failure Word value - Success
//!
SMP_WORD
SMP_get_word(const SMP *a)
{
    if (SMP_num_bytes(a) > sizeof(SMP_WORD))
        return (SMP_MASK);

    return (a->d[0]);
}
*/
//!
//! Set single word to SMP data structure
//!
//! @param a SMP data structure
//! @param w Word to set
//! @return 0 - Failure 1 - Success
//!
int
SMP_set_word(SMP *a, SMP_WORD w)
{
    if (SMP_expand_b(a,sizeof(SMP_WORD)*8) == NULL)
        return 0;

    a->neg = 0;
    a->top = 0;
    a->d[0] = (SMP_WORD)w&SMP_MASK;
    if (a->d[0] != 0)
        a->top=1;

    return 1;
}

//!
//! Converts the positive integer in big-endian form of
//! length len at s into a SMP and places it in ret.
//! If ret is NULL, a new SMP is created.
//!
//! @param s Address of source memory
//! @param len Length of source memory
//! @param ret Address of destination SMP data structure (New data structure allocated if NULL)
//! @return Address of destination SMP data structure
//!
SMP*
SMP_bin2mp(const unsigned char *s, int len, SMP *ret)
{
    unsigned int i,m;
    unsigned int n;
    SMP_WORD l;

    if (ret == NULL)
        ret = SMP_new();
    if (ret == NULL)
        return NULL ;

    l = 0;
    n = len;
    if (n == 0)
    {
        ret->top=0;
        return(ret);
    }

    if (SMP_expand_b(ret,(int)(n+2)*8) == NULL)
        return NULL;

    i = ((n-1)/SMP_BYTES)+1;
    m = ((n-1)%(SMP_BYTES));

    ret->top = i;
    ret->neg = 0;

    while (n-- > 0)
    {
        l = (l<<8L) | *(s++);
        if (m-- == 0)
        {
            ret->d[--i] = l;
            l = 0;
            m = SMP_BYTES-1;
        }
    }

    // need to call this due to clear byte at top if avoiding
    // having the top bit set (-ve number)
    SMP_fix_top(ret);

    return ret;
}
/*
//!
//! Converts the absolute value of a into big-endian form
//! and stores it at to. to must point to SMP_num_bytes(a)
//! bytes of memory.
//!
//! @param a Address of source SMP data structure
//! @param to Address of destination memory
//! @return Number of bytes copied to destination memory
//!
int
SMP_mp2bin(const SMP *a, unsigned char *to)
{
    int n,i;
    SMP_WORD l;

    if (to == NULL)
        return 0;

    n = i = SMP_num_bytes(a);
    while (i-- > 0)
    {
        l = a->d[i/SMP_BYTES];
        *(to++) = (unsigned char)(l>>(8*(i%SMP_BYTES)))&0xff;
    }

    return n;
}

//!
//! Converts the string a containing a hexadecimal number
//! to a SMP data structure and stores it in **smp.
//! If *smp is NULL, a new SMP is created.
//! If smp is NULL, it only computes the number's length in hexadecimal digits.
//! If the string starts with '-', the number is negative.
//!
//! @param smp Address reference of destination SMP data structure
//! @param a Address of source memory
//! @return Number of bytes effective in source memory
//!
int
SMP_hex2mp(SMP **smp, const char *a)
{
    SMP *ret = NULL;
    SMP_WORD l = 0;
    int neg = 0, h, m, i, j, k, c;
    int num;

    if ((a == NULL) || (*a == '\0'))
        return 0;

    if (*a == '-')
    {
        neg = 1;
        a++;
    }

    for (i = 0 ; isxdigit((unsigned char) a[i]) ; i++)
        ;

    num = i + neg;
    if (smp == NULL)
        return num;

    // a is the start of the hex digits, and it is 'i' long
    if (*smp == NULL)
    {
        if ((ret = SMP_new()) == NULL)
            return 0;
    }
    else
    {
        ret = *smp;

        // Dunno why we need this - Juno
        // SMP_zero(ret);
    }

    // i is the number of hex digests;
    if (SMP_expand_b(ret,i*4) == NULL)
        goto err;

    j = i; // least significant 'hex'
    m = 0;
    h = 0;
    while (j > 0)
    {
        m = ((SMP_BYTES*2) <= j) ? (SMP_BYTES*2) : j ;
        l = 0;

        for (;;)
        {
            c = a[j-m];

            if ((c >= '0') && (c <= '9'))
                k = c - '0';
            else if ((c >= 'a') && (c <= 'f'))
                k = c - 'a' + 10;
            else if ((c >= 'A') && (c <= 'F'))
                k = c - 'A' + 10;
            else
                k = 0;

            l = (l<<4) | k;

            if (--m <= 0)
            {
                ret->d[h++] = l;
                break;
            }
        }
        j -= (SMP_BYTES*2);
    }

    ret->top = h;
    SMP_fix_top(ret);
    ret->neg = neg;

    *smp = ret;
    return num;

err:
    if (*smp == NULL)
        SMP_free(ret);
    return 0;
}
*/

