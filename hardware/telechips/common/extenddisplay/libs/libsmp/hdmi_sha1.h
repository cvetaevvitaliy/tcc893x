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

#ifndef __HDMI_SHA1_H__
#define __HDMI_SHA1_H__

#ifdef  __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------
// Preprocessor
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// Compile option
//-------------------------------------------------------------------

//#define    HDMI_BIGEND    //!< Compile with big endian

//-------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------
#define WORD_BIT_LEN    32
//! Rotate left using shift operation
#define ROTL_WORD(x,n) ((unsigned int)((x) << (n)) | (unsigned int)((x) >> (WORD_BIT_LEN-(n))))
//! Rotate right using shift operation
#define ROTR_WORD(x,n) ((unsigned int)((x) >> (n)) | (unsigned int)((x) << (WORD_BIT_LEN-(n))))

//! Rotate left using shift operation
#define ROTL2_WORD(x0,x1,n) ((unsigned int)((x0) << (n)) | (unsigned int)((x1) >> (WORD_BIT_LEN-(n))))
//! Rotate right using shift operation
#define ROTR2_WORD(x0,x1,n) ((unsigned int)((x0) >> (n)) | (unsigned int)((x1) << (WORD_BIT_LEN-(n))))

//! Reverse byte order of WORD
#define ENDIAN_REVERSE_WORD(wS) ((ROTL_WORD((wS),8) & (unsigned int)0x00ff00ff) \
    | (ROTR_WORD((wS),8) & (unsigned int)0xff00ff00))

//! Convert between WORD type and BYTE type
#if defined(HDMI_BIGEND)
    #define BIG_B2W(B,W) W = *(unsigned int*)(B)
    #define BIG_W2B(W,B) *(unsigned int*)(B) = (unsigned int)(W)
    #define LITTLE_B2W(B,W)    W = ENDIAN_REVERSE_WORD(*(unsigned int*)(B))
    #define LITTLE_W2B(W,B)    *(unsigned int*)(B) = ENDIAN_REVERSE_WORD(W)
#else
    #define BIG_B2W(B,W) W = ENDIAN_REVERSE_WORD(*(unsigned int*)(B))
    #define BIG_W2B(W,B) *(unsigned int*)(B) = ENDIAN_REVERSE_WORD(W)
    #define LITTLE_B2W(B,W)    W = *(unsigned int*)(B)
    #define LITTLE_D2B(W,B)    *(unsigned int*)(B) = (unsigned int)(W)
#endif

//! Length of digest input block in bytes
#define DIGEST_IN_LEN  64
//! Length of digest output block in bytes
#define DIGEST_OUT_LEN 20

//-------------------------------------------------------------------
// Data Types
//-------------------------------------------------------------------
//! HDMI hash function(SHA-1) structure
typedef struct _hdmi_sha1_st_
{
    unsigned int* pwChainVar;  //!< Chaining variable of SHA-1
    unsigned int* pwCount;     //!< Length of input block in bits
    unsigned char* pbBuffer;    //!< Buffer for unfilled block
} HDMI_SHA1;
//! HDMI hash function input message structure
typedef struct _hdmi_sha1_msg_
{
    unsigned char* pbMsg;      //!< Buffer for unfilled block
    unsigned int pwLength;   //!< Length of input block in bits
} SHA1_MSG;
//-------------------------------------------------------------------
// Function Prototypes
//-------------------------------------------------------------------

// HDMI hash(SHA-1) functions
void SHA1(unsigned char* msg, unsigned int msg_len, unsigned char* digest);
void HDMI_SHA1_init(HDMI_SHA1* sha1);
void HDMI_SHA1_update(HDMI_SHA1* sha1, unsigned char* buf, unsigned int len);
void HDMI_SHA1_final(HDMI_SHA1* sha1, unsigned char* digest);

// HDMI debugging functions

#ifdef  __cplusplus
}
#endif
#endif
