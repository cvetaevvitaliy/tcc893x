/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions andlimitations under the License.

****************************************************************************/
   																				
#ifndef _VENC_H_
#define _VENC_H_

#include "../cdk/cdk_core.h"
#include "../cdk/cdk_port.h"
#ifdef INCLUDE_VPU_ENC
	#include "vpu/TCC_VPU_CODEC_EX.h" // VPU video codec
#endif
#define K_VA 2

#define VENC_PIC_TYPE_I	0
#define VENC_PIC_TYPE_P	1

typedef struct venc_init_t
{
	unsigned int m_RegBaseVirtualAddr;	//!< virtual address BIT_BASE

	// Encoding Info
	int m_iBitstreamFormat;				
	int m_iPicWidth;					//!< Width  : multiple of 16
	int m_iPicHeight;					//!< Height : multiple of 16
	int m_iFrameRate;					//!< Frame rate
	int m_iTargetKbps;					//!< Target bit rate in Kbps. if 0, there will be no rate control, 
										//!< and pictures will be encoded with a quantization parameter equal to quantParam

	int m_iKeyInterval;					//!< Key interval : max 32767
	int m_iAvcFastEncoding;				//!< fast encoding for AVC( 0: default, 1: encode intra 16x16 only )

	unsigned int m_BitstreamBufferAddr; //!< physical address : multiple of 4
	unsigned int m_BitstreamBufferAddr_VA;
	int m_iBitstreamBufferSize;			//!< bitstream buffer size : multiple of 1024

	//! Callback Func
	void* (*m_pfMemcpy ) ( void*, const void*, unsigned int );	//!< memcpy
	void  (*m_pfMemset ) ( void*, int, unsigned int );			//!< memset
	int   (*m_pfInterrupt ) ( void );								//!< hw interrupt
} venc_init_t;

typedef struct venc_seq_header_t
{
	codec_addr_t m_SeqHeaderBuffer[2];	//!< [in]  Seqence header buffer
	int m_iSeqHeaderBufferSize;			//!< [in]  Seqence header buffer size
	unsigned char* m_pSeqHeaderOut;		//!< [out] Seqence header pointer
	int m_iSeqHeaderOutSize;			//!< [out] Seqence header size
} venc_seq_header_t;

typedef struct venc_input_t
{
	int m_bCbCrInterleaved;
	unsigned char* m_pInputY;
	unsigned char* m_pInputCbCr[2];

	codec_addr_t m_BitstreamBufferPA;	//!< [in] physical address
	int m_iBitstreamBufferSize;
} venc_input_t;

typedef struct venc_output_t 
{
	unsigned char* m_pBitstreamOut;
	int m_iBitstreamOutSize;
	int m_iPicType;
} venc_output_t;

#define VENC_INIT		0
#define VENC_CLOSE		1
#define VENC_SEQ_HEADER	2
#define VENC_ENCODE		3


int
venc_vpu( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

#endif //_VENC_H_ 
