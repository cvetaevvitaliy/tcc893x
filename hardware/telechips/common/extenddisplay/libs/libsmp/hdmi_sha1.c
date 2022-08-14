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
#include <stdio.h>
// HDMI SHA1 header
#include "hdmi_sha1.h"

//-------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------

//! 1st round Boolean operation
#define F(x,y,z) ((z) ^ ((x) & ((y) ^ (z))))
//! 2nd/4th round Boolean operation
#define G(x,y,z) ((x) ^ (y) ^ (z))
//! 3rd round Boolean operation
#define H(x,y,z) (((x)&(y)) | ((z) & ((x) | (y))))

//! 1st round step operation
#define FF(a,b,c,d,e,X) {                              \
    (e) += ROTL_WORD((a), 5) + F((b),(c),(d)) + X + K; \
    (b)  = ROTL_WORD((b),30);                          \
}
//! 2nd round step operation
#define GG(a,b,c,d,e,X) {                              \
    (e) += ROTL_WORD((a), 5) + G((b),(c),(d)) + X + K; \
    (b)  = ROTL_WORD((b),30);                          \
}
//! 3rd round step operation
#define HH(a,b,c,d,e,X) {                              \
    (e) += ROTL_WORD((a), 5) + H((b),(c),(d)) + X + K; \
    (b)  = ROTL_WORD((b),30);                          \
}
//! 4th round step operation
#define II(a,b,c,d,e,X) {                              \
    (e) += ROTL_WORD((a), 5) + G((b),(c),(d)) + X + K; \
    (b)  = ROTL_WORD((b),30);                          \
}

//! Message expansion
#define TT(t) X[t] = ROTL_WORD(X[t-16]^X[t-14]^X[t-8]^X[t-3],1)

//! Get data according to endianess
#if defined(HDMI_BIGEND)
    #define GetData(x) x
#else
    #define GetData(x) ENDIAN_REVERSE_WORD(x)
#endif

//-------------------------------------------------------------------
// Function Implementation
//-------------------------------------------------------------------

/***********************
// SHA1 test examples
***********************/
/*
unsigned char digest[20]={0,};
    //TABLE A-24V
       unsigned char msg[25]={
    0x2e,0x17,0x6a,0x79,0x35,0x0f,0xe2,0x71,0x8e,0x47,0xa6,0x97,0x53,0xe8,0x74,0x03,
    0x02,0x8f,0xe7,0xbb,0x38,0xce,0x3d,0x2d,0x37
    };
    unsigned char correct[20]={
    0x0f,0xcb,0xd5,0x86,
    0xef,0xc1,0x07,0xef,
    0xcc,0xd7,0x0a,0x1d,
    0xb1,0x18,0x6d,0xda,
    0x1f,0xb3,0xff,0x5e
    };

    //TABLE A-25V
    unsigned char msg[85]={
    0x4d,0xbe,0x9c,0xa1,0x23,0x70,0x35,0x99,0x7e,0x0d,0x09,0x7d,0x8d,0x45,0xd3,0x46,
    0xe9,0xdc,0xa2,0xe2,0x9d,0x49,0x8e,0x14,0xf3,0xa3,0x5c,0xe9,0x45,0x93,0xc5,0x07,
    0xb3,0x8c,0xda,0xac,0x75,0xfa,0x01,0x99,0x20,0x3c,0x3a,0x7f,0x69,0x19,0xed,0x58,
    0x97,0xc8,0x69,0xe8,0xa8,0xe3,0x2d,0xf2,0x5a,0x29,0xd9,0xe0,0xb3,0xa8,0x88,0xde,
    0x6c,0xf5,0x99,0x94,0x21,0x6e,0x72,0xa5,0xe1,0xe3,0x31,0x0f,0x03,0x8f,0xe7,0xbb,
    0x38,0xce,0x3d,0x2d,0x37
    };



    //unsigned char* input="abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    //Message digest = 84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
    unsigned char* input="abc";
    //Message digest = A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D

    SHA1(input,strlen(input),digest);

    SHA1(msg,64,digest);
*/
void SHA1(unsigned char* msg, unsigned int msg_len, unsigned char* digest)
{
    HDMI_SHA1 sha1;
    // pwChainVar=  5word
    // pwCount   =  2word
    // pwpbBuffer= 16word
    // Total     = 23word

    unsigned int SHA1_buf[23];
    sha1.pwChainVar= SHA1_buf;
    sha1.pwCount   = &SHA1_buf[5];
    sha1.pbBuffer  = (unsigned char*)&SHA1_buf[7];

    // Hash input message
    HDMI_SHA1_init(&sha1);
    HDMI_SHA1_update(&sha1,msg, msg_len);
    HDMI_SHA1_final(&sha1,digest);

}



static void
HDMI_SHA1_transform(unsigned int* Y, unsigned int* DigestValue)
{
    unsigned int a, b, c, d, e, K, *X;

    // Allocate memory for X
    unsigned int X_temp[DIGEST_OUT_LEN+DIGEST_IN_LEN+8];
    X=(unsigned int*)X_temp;
    // Unroll loops
    X[ 0] = GetData(Y[ 0]);
    X[ 1] = GetData(Y[ 1]);
    X[ 2] = GetData(Y[ 2]);
    X[ 3] = GetData(Y[ 3]);
    X[ 4] = GetData(Y[ 4]);
    X[ 5] = GetData(Y[ 5]);
    X[ 6] = GetData(Y[ 6]);
    X[ 7] = GetData(Y[ 7]);
    X[ 8] = GetData(Y[ 8]);
    X[ 9] = GetData(Y[ 9]);
    X[10] = GetData(Y[10]);
    X[11] = GetData(Y[11]);
    X[12] = GetData(Y[12]);
    X[13] = GetData(Y[13]);
    X[14] = GetData(Y[14]);
    X[15] = GetData(Y[15]);

    // Expand Message
    TT(16);    TT(17);    TT(18);    TT(19);    TT(20);    TT(21);    TT(22);    TT(23);
    TT(24);    TT(25);    TT(26);    TT(27);    TT(28);    TT(29);    TT(30);    TT(31);
    TT(32);    TT(33);    TT(34);    TT(35);    TT(36);    TT(37);    TT(38);    TT(39);
    TT(40);    TT(41);    TT(42);    TT(43);    TT(44);    TT(45);    TT(46);    TT(47);
    TT(48);    TT(49);    TT(50);    TT(51);    TT(52);    TT(53);    TT(54);    TT(55);
    TT(56);    TT(57);    TT(58);    TT(59);    TT(60);    TT(61);    TT(62);    TT(63);
    TT(64);    TT(65);    TT(66);    TT(67);    TT(68);    TT(69);    TT(70);    TT(71);
    TT(72);    TT(73);    TT(74);    TT(75);    TT(76);    TT(77);    TT(78);    TT(79);

    a=DigestValue[0];
    b=DigestValue[1];
    c=DigestValue[2];
    d=DigestValue[3];
    e=DigestValue[4];

    // 1st round
    K = 0x5A827999;
    FF(a,b,c,d,e,X[ 0]); FF(e,a,b,c,d,X[ 1]);
    FF(d,e,a,b,c,X[ 2]); FF(c,d,e,a,b,X[ 3]);
    FF(b,c,d,e,a,X[ 4]); FF(a,b,c,d,e,X[ 5]);
    FF(e,a,b,c,d,X[ 6]); FF(d,e,a,b,c,X[ 7]);
    FF(c,d,e,a,b,X[ 8]); FF(b,c,d,e,a,X[ 9]);
    FF(a,b,c,d,e,X[10]); FF(e,a,b,c,d,X[11]);
    FF(d,e,a,b,c,X[12]); FF(c,d,e,a,b,X[13]);
    FF(b,c,d,e,a,X[14]); FF(a,b,c,d,e,X[15]);
    FF(e,a,b,c,d,X[16]); FF(d,e,a,b,c,X[17]);
    FF(c,d,e,a,b,X[18]); FF(b,c,d,e,a,X[19]);

    // 2nd round
    K = 0x6ED9EBA1;
    GG(a,b,c,d,e,X[20]); GG(e,a,b,c,d,X[21]);
    GG(d,e,a,b,c,X[22]); GG(c,d,e,a,b,X[23]);
    GG(b,c,d,e,a,X[24]); GG(a,b,c,d,e,X[25]);
    GG(e,a,b,c,d,X[26]); GG(d,e,a,b,c,X[27]);
    GG(c,d,e,a,b,X[28]); GG(b,c,d,e,a,X[29]);
    GG(a,b,c,d,e,X[30]); GG(e,a,b,c,d,X[31]);
    GG(d,e,a,b,c,X[32]); GG(c,d,e,a,b,X[33]);
    GG(b,c,d,e,a,X[34]); GG(a,b,c,d,e,X[35]);
    GG(e,a,b,c,d,X[36]); GG(d,e,a,b,c,X[37]);
    GG(c,d,e,a,b,X[38]); GG(b,c,d,e,a,X[39]);

    // 3rd round
    K = 0x8F1BBCDC;
    HH(a,b,c,d,e,X[40]); HH(e,a,b,c,d,X[41]);
    HH(d,e,a,b,c,X[42]); HH(c,d,e,a,b,X[43]);
    HH(b,c,d,e,a,X[44]); HH(a,b,c,d,e,X[45]);
    HH(e,a,b,c,d,X[46]); HH(d,e,a,b,c,X[47]);
    HH(c,d,e,a,b,X[48]); HH(b,c,d,e,a,X[49]);
    HH(a,b,c,d,e,X[50]); HH(e,a,b,c,d,X[51]);
    HH(d,e,a,b,c,X[52]); HH(c,d,e,a,b,X[53]);
    HH(b,c,d,e,a,X[54]); HH(a,b,c,d,e,X[55]);
    HH(e,a,b,c,d,X[56]); HH(d,e,a,b,c,X[57]);
    HH(c,d,e,a,b,X[58]); HH(b,c,d,e,a,X[59]);

    // 4th round
    K = 0xCA62C1D6;
    II(a,b,c,d,e,X[60]); II(e,a,b,c,d,X[61]);
    II(d,e,a,b,c,X[62]); II(c,d,e,a,b,X[63]);
    II(b,c,d,e,a,X[64]); II(a,b,c,d,e,X[65]);
    II(e,a,b,c,d,X[66]); II(d,e,a,b,c,X[67]);
    II(c,d,e,a,b,X[68]); II(b,c,d,e,a,X[69]);
    II(a,b,c,d,e,X[70]); II(e,a,b,c,d,X[71]);
    II(d,e,a,b,c,X[72]); II(c,d,e,a,b,X[73]);
    II(b,c,d,e,a,X[74]); II(a,b,c,d,e,X[75]);
    II(e,a,b,c,d,X[76]); II(d,e,a,b,c,X[77]);
    II(c,d,e,a,b,X[78]); II(b,c,d,e,a,X[79]);

    // Chaining variables update
    DigestValue[0] += a;
    DigestValue[1] += b;
    DigestValue[2] += c;
    DigestValue[3] += d;
    DigestValue[4] += e;
}

//!
//! Initialize HDMI_SHA1 data structure
//!
//! [IMPORTANT] Do not initialize more than one instance of
//! HDMI_SHA1 data structure.
//!
//! @param sha1 HDMI_SHA1 data structure to initialize
//!
void
HDMI_SHA1_init(HDMI_SHA1* sha1)
{

    // Initialize chianing variables
    sha1->pwChainVar[0] = 0x67452301;
    sha1->pwChainVar[1] = 0xefcdab89;
    sha1->pwChainVar[2] = 0x98badcfe;
    sha1->pwChainVar[3] = 0x10325476;
    sha1->pwChainVar[4] = 0xC3D2E1F0;

    // Initialize length
    sha1->pwCount[0] = sha1->pwCount[1] = 0;

    // Initialize Message buffer
    *(((unsigned int*)(sha1->pbBuffer))+ 0) = 0;
    *(((unsigned int*)(sha1->pbBuffer))+ 1) = 0;
    *(((unsigned int*)(sha1->pbBuffer))+ 2) = 0;
    *(((unsigned int*)(sha1->pbBuffer))+ 3) = 0;
    *(((unsigned int*)(sha1->pbBuffer))+ 4) = 0;
    *(((unsigned int*)(sha1->pbBuffer))+ 5) = 0;
    *(((unsigned int*)(sha1->pbBuffer))+ 6) = 0;
    *(((unsigned int*)(sha1->pbBuffer))+ 7) = 0;
    *(((unsigned int*)(sha1->pbBuffer))+ 8) = 0;
    *(((unsigned int*)(sha1->pbBuffer))+ 9) = 0;
    *(((unsigned int*)(sha1->pbBuffer))+10) = 0;
    *(((unsigned int*)(sha1->pbBuffer))+11) = 0;
    *(((unsigned int*)(sha1->pbBuffer))+12) = 0;
    *(((unsigned int*)(sha1->pbBuffer))+13) = 0;
    *(((unsigned int*)(sha1->pbBuffer))+14) = 0;
    *(((unsigned int*)(sha1->pbBuffer))+15) = 0;
}

//!
//! Update chaining variable with message provided.
//! If input block is not enough to run digest function,
//! then just fill the buffer in the HDMI_SHA1 data structure
//! with message until enough message is filled.
//!
//! @param sha1 HDMI_SHA1 data structure
//! @param buf Input message pointer
//! @param len Length of input message in bytes
//!
void
HDMI_SHA1_update(HDMI_SHA1* sha1, unsigned char* buf, unsigned int len)
{

    unsigned int i, RemainedLen, PartLen;

    // Compute the number of hashed bytes mod DIGEST_IN_LEN
    RemainedLen = (sha1->pwCount[0] >> 3) % DIGEST_IN_LEN;
    // Compute the number of bytes that can be filled up
    PartLen = DIGEST_IN_LEN - RemainedLen;

    // Update Count (number of total data bits)
    // TODO: ???
#if 0
    if ((sha1->pwCount[0] += (len << 3)) < sha1->pwCount[0])
        sha1->pwCount[1]++;
#endif
    sha1->pwCount[1] += (len >> 29);

    // Core update routine
    if (len >= PartLen)
    {
        // Copy message into buffer
        for (i = 0 ; i < PartLen ; i++)
            sha1->pbBuffer[RemainedLen+i] = buf[i];
        HDMI_SHA1_transform((unsigned int*)sha1->pbBuffer, sha1->pwChainVar);

        buf += PartLen;
        len -= PartLen;
        RemainedLen = 0;

        while (len >= DIGEST_IN_LEN)
        {
            if ((((int)buf)%4) == 0)
                // Speed up technique
                HDMI_SHA1_transform((unsigned int*)buf, sha1->pwChainVar);
            else
            {
                for (i = 0 ; i < DIGEST_IN_LEN ; i++)
                    sha1->pbBuffer[i] = buf[i];
                HDMI_SHA1_transform((unsigned int*)sha1->pbBuffer, sha1->pwChainVar);
            }
            buf += DIGEST_IN_LEN;
            len -= DIGEST_IN_LEN;
        }
    }

    // Buffer remaining input
    for (i = 0 ; i < len ; i++)
        sha1->pbBuffer[RemainedLen+i] = buf[i];
}

//!
//! Finalize chaining variable and output digest value.
//! Padding and length of input are appended as required.
//! Length of output digest is given by DIGEST_OUT_LEN.
//!
//! @param sha1 HDMI_SHA1 data structure
//! @param digest Memory pointer to save output digest value.
//!
void
HDMI_SHA1_final(HDMI_SHA1* sha1, unsigned char* digest)
{
    unsigned int i, wIndex, CountL, CountH;

    // Digest final block
    CountL = sha1->pwCount[0];
    CountH = sha1->pwCount[1];
    wIndex = (CountL >> 3) % DIGEST_IN_LEN;
    sha1->pbBuffer[wIndex++] = 0x80;

    // Padding with zero
    if (wIndex > (DIGEST_IN_LEN-8))
    {
        for (i = 0 ; i < (DIGEST_IN_LEN-wIndex) ; i++)
            sha1->pbBuffer[wIndex+i] = 0;
        HDMI_SHA1_transform((unsigned int*)sha1->pbBuffer, sha1->pwChainVar);
        for (i = 0 ; i < (DIGEST_IN_LEN-8) ; i++)
            sha1->pbBuffer[i] = 0;
    }
    else
    {
        for (i = 0 ; i < (DIGEST_IN_LEN-wIndex-8) ; i++)
            sha1->pbBuffer[wIndex+i] = 0;
    }

#if !defined(HDMI_BIGEND)
    CountL = ENDIAN_REVERSE_WORD(CountL);
    CountH = ENDIAN_REVERSE_WORD(CountH);
#endif

    // Padding with length
    ((unsigned int*)sha1->pbBuffer)[DIGEST_IN_LEN/4-2] = CountH;
    ((unsigned int*)sha1->pbBuffer)[DIGEST_IN_LEN/4-1] = CountL;

    HDMI_SHA1_transform((unsigned int*)sha1->pbBuffer, sha1->pwChainVar);

    for (i=0 ; i < DIGEST_OUT_LEN ; i += 4)
        BIG_W2B((sha1->pwChainVar)[i/4], &(digest[i]));
}
