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

#include <stdio.h>
#include <stdlib.h>

#include "libsrm.h"

#define SRM_DEBUG   0
#if SRM_DEBUG
#define DPRINTF(args...)    printf(args)
#else
#define DPRINTF(args...)
#endif

/**
 * This function checks SRM integrity.\n
 *
 * @param    message    [in]    Pointer to the input SRM\n
 *
 * @return  1=Input SRM's integrity is not correct or not satisfy the term (0<r<q && 0<s<q)\n
 *          0=Input SRM's integrity is correct.\n
 *
 *   The DSA makes use of the following parameters:\n
 * p = a prime modulus, where 2^(L-1) < p < 2^L for 512 <= L <= 1024 and L a multiple of 64\n
 * q = a prime divisor of p - 1, where 2^159 < q < 2^160\n
 * g = h^((p-1)/q) mod p, where h is any integer with 1 < h < p - 1 such that h^((p-1)/q) mod p > 1
 * (g has order q mod p)\n
 * x = a randomly or pseudorandomly generated integer with 0 < x < q\n
 * y = g^x mod p\n
 * k = a randomly or pseudorandomly generated integer with 0 < k < q\n
*/
unsigned int verify_SRM(unsigned char* message)
{
    // Cryptographic parameters for verifying SRM
    unsigned char Prime_Modulus[128]=
    {
    0xd3,0xc3,0xf5,0xb2,0xfd,0x17,0x61,0xb7,0x01,0x8d,
    0x75,0xf7,0x93,0x43,0x78,0x6b,0x17,0x39,0x5b,0x35,
    0x5a,0x52,0xc7,0xb8,0xa1,0xa2,0x4f,0xc3,0x6a,0x70,
    0x58,0xff,0x8e,0x7f,0xa1,0x64,0xf5,0x00,0xe0,0xdc,
    0xa0,0xd2,0x84,0x82,0x1d,0x96,0x9e,0x4b,0x4f,0x34,
    0xdc,0x0c,0xae,0x7c,0x76,0x67,0xb8,0x44,0xc7,0x47,
    0xd4,0xc6,0xb9,0x83,0xe5,0x2b,0xa7,0x0e,0x54,0x47,
    0xcf,0x35,0xf4,0x04,0xa0,0xbc,0xd1,0x97,0x4c,0x3a,
    0x10,0x71,0x55,0x09,0xb3,0x72,0x15,0x30,0xa7,0x3f,
    0x32,0x07,0xb9,0x98,0x20,0x49,0x5c,0x7b,0x9c,0x14,
    0x32,0x75,0x73,0x3b,0x02,0x8a,0x49,0xfd,0x96,0x89,
    0x19,0x54,0x2a,0x39,0x95,0x1c,0x46,0xed,0xc2,0x11,
    0x8c,0x59,0x80,0x2b,0xf3,0x28,0x75,0x27
    };

    unsigned char Prime_Divisor[20]=
    {
    0xee,0x8a,0xf2,0xce,0x5e,0x6d,0xb5,0x6a,0xcd,0x6d,
    0x14,0xe2,0x97,0xef,0x3f,0x4d,0xf9,0xc7,0x08,0xe7
    };

    unsigned char Generator[128]=
    {
    0x92,0xf8,0x5d,0x1b,0x6a,0x4d,0x52,0x13,0x1a,0xe4,
    0x3e,0x24,0x45,0xde,0x1a,0xb5,0x02,0xaf,0xde,0xac,
    0xa9,0xbe,0xd7,0x31,0x5d,0x56,0xd7,0x66,0xcd,0x27,
    0x86,0x11,0x8f,0x5d,0xb1,0x4a,0xbd,0xec,0xa9,0xd2,
    0x51,0x62,0x97,0x7d,0xa8,0x3e,0xff,0xa8,0x8e,0xed,
    0xc6,0xbf,0xeb,0x37,0xe1,0xa9,0x0e,0x29,0xcd,0x0c,
    0xa0,0x3d,0x79,0x9e,0x92,0xdd,0x29,0x45,0xf7,0x78,
    0x58,0x5f,0xf7,0xc8,0x35,0x64,0x2c,0x21,0xba,0x7f,
    0xb1,0xa0,0xb6,0xbe,0x81,0xc8,0xa5,0xe3,0xc8,0xab,
    0x69,0xb2,0x1d,0xa5,0x42,0x42,0xc9,0x8e,0x9b,0x8a,
    0xab,0x4a,0x9d,0xc2,0x51,0xfa,0x7d,0xac,0x29,0x21,
    0x6f,0xe8,0xb9,0x3f,0x18,0x5b,0x2f,0x67,0x40,0x5b,
    0x69,0x46,0x24,0x42,0xc2,0xba,0x0b,0xd9
    };

    unsigned char Public_Key[128]=
    {
    0xc7,0x06,0x00,0x52,0x6b,0xa0,0xb0,0x86,0x3a,0x80,
    0xfb,0xe0,0xa3,0xac,0xff,0x0d,0x4f,0x0d,0x76,0x65,
    0x8a,0x17,0x54,0xa8,0xe7,0x65,0x47,0x55,0xf1,0x5b,
    0xa7,0x8d,0x56,0x95,0x0e,0x48,0x65,0x4f,0x0b,0xbd,
    0xe1,0x68,0x04,0xde,0x1b,0x54,0x18,0x74,0xdb,0x22,
    0xe1,0x4f,0x03,0x17,0x04,0xdb,0x8d,0x5c,0xb2,0xa4,
    0x17,0xc4,0x56,0x6c,0x27,0xba,0x97,0x3c,0x43,0xd8,
    0x4e,0x0d,0xa2,0xa7,0x08,0x56,0xfe,0x9e,0xa4,0x8d,
    0x87,0x25,0x90,0x38,0xb1,0x65,0x53,0xe6,0x62,0x43,
    0x5f,0xf7,0xfd,0x52,0x06,0xe2,0x7b,0xb7,0xff,0xbd,
    0x88,0x6c,0x24,0x10,0x95,0xc8,0xdc,0x8d,0x66,0xf6,
    0x62,0xcb,0xd8,0x8f,0x9d,0xf7,0xe9,0xb3,0xfb,0x83,
    0x62,0xa9,0xf7,0xfa,0x36,0xe5,0x37,0x99
    };

    SRM_DATA SRM_in;
    SRM_KEY SRM_key;

    SMP w;
    SMP u1;
    SMP u2;
    SMP v;
    SMP p;
    SMP q;
    SMP s;
    SMP SHA_M;
    SMP r;
    SMP g;
    SMP y;

    int i,chk=0,generation=0;
    unsigned int Message_len;
    unsigned char *pSHA=NULL;

    //Load SRM key
    SRM_key.pbP=Prime_Modulus;
    SRM_key.pbQ=Prime_Divisor;
    SRM_key.pbGen=Generator;
    SRM_key.pbPublic=Public_Key;
    // pbRcvr  = 20 byte
    // pbRcvs  = 20 byte

    // Init SMP variables
    //SMP_init(pW);
    SMP_init(&w);
    SMP_init(&u1);
    SMP_init(&u2);
    SMP_init(&v);
    SMP_init(&p);
    SMP_init(&q);
    SMP_init(&s);
    SMP_init(&SHA_M);
    SMP_init(&r);
    SMP_init(&g);
    SMP_init(&y);

    // Read SRM generation number
    generation=message[4];

    // Alloc memory for SHA
    pSHA=(unsigned char *)malloc(sizeof(unsigned char)*20);
    if (pSHA == NULL) DPRINTF("MEM ALLOC ERROR!!\n");

    for(i=generation; i>0; i--)
    {
        unsigned int msg_cnt;
        // Calculate r,s starting point
        if( i == generation )
        {
            msg_cnt=5;
            Message_len=((message[msg_cnt]<<16)|(message[msg_cnt+1]<<8)|message[msg_cnt+2]);
            msg_cnt=8;

            //If Empty SRM (SRM for testing)
            if(Message_len <= 43)
            {
                // Calculate the starting point of r,s
                msg_cnt=8;
            }
            else
            {
                //2009.01.28 hskim
                // Fix pbRcvr and pbRsvs start point calculation error.
                msg_cnt=Message_len+5-40;          
            }
                SRM_in.pbRcvr=&message[msg_cnt];
                SRM_in.pbRcvs=&message[msg_cnt+20];
        }
        else
        {
            msg_cnt=0;
            Message_len=((message[msg_cnt]<<8)|message[msg_cnt+1]);

            // Calculate the starting point of r,s
            //2009.01.28 hskim
            // Fix pbRcvr and pbRsvs start point calculation error.
            msg_cnt=Message_len-40;

            SRM_in.pbRcvr=&message[msg_cnt];
            SRM_in.pbRcvs=&message[msg_cnt+20];
        }
            //SMP* SMP_bin2mp(const unsigned char *s, int len, SMP *ret)
            // Load received SRM (r', s')
            SMP_bin2mp(SRM_in.pbRcvr, 20, &r);
            SMP_bin2mp(SRM_in.pbRcvs, 20, &s);

            // Load common parameters (p,q,g)
            SMP_bin2mp(SRM_key.pbP, 128, &p);
            SMP_bin2mp(SRM_key.pbQ, 20, &q);
            SMP_bin2mp(SRM_key.pbGen, 128, &g);
            SMP_bin2mp(SRM_key.pbPublic, 128, &y);

            // Check 0<r<q && 0<s<q
            if(!((SMP_cmp(&r, &q)<0)&&(SMP_cmp(&s,&q)<0)))
            {
                chk=1;
                DPRINTF("0<r<q && 0<s<q ERROR!!! \n");      
                break;
            }

            // Start Calculation
            // w = (s')^-1 mod q

            //SMP_mod_inverse(&w, &s, &q);
            SMP_mod_inverse(&w, &s, &q);

            // u1 = ((SHA-1(M'))w) mod q
            SHA1(message,Message_len-40,pSHA);
            SMP_bin2mp(pSHA, 20, &SHA_M);

            //param r (a * b) % m result
            SMP_mod_mul(&u1, &SHA_M, &w, &q);

            //u2 = ((r')w) mod q
            SMP_mod_mul(&u2, &r, &w, &q);

            //v = (((g)^u1 (y)^u2) mod p) mod q.
            SMP_clear(&s);// store (g^u1)mod p
            SMP_clear(&w);// store (y^u2)mod p

            SMP_mod_exp(&s, &g, &u1, &p);
            SMP_mod_exp(&w, &y, &u2, &p);
            SMP_mod_mul(&v, &w, &s, &p);
            SMP_mod(&v, &v, &q);

            // Go to the next generation SRM
            if(i==generation)
            {
                message+=Message_len+5;
            }
            else
            {
                message+=Message_len;
            }

            // Check v = r'
            chk=SMP_cmp(&v, &r);
            if(chk != 0)
            {
                DPRINTF("Generation %d is ERROR!!!\n",generation-i+1);
                break;
            }
    }// End for()

    free(pSHA);
    SMP_free(&y);
    SMP_free(&g);
    SMP_free(&r);
    SMP_free(&SHA_M);
    SMP_free(&s);
    SMP_free(&q);
    SMP_free(&p);
    SMP_free(&v);
    SMP_free(&u2);
    SMP_free(&u1);
    SMP_free(&w);

    // Check v = r'
    if (chk)
    {
        DPRINTF("Verify ERROR!!\n");
        return(1);
    }
    else
    {
        DPRINTF("### Verify OK ###\n");
        return(0);
    }
}// End verify_SRM

