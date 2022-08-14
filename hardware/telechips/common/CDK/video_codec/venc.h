/****************************************************************************
 *   FileName    : venc.h
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided ¡°AS IS¡± and nothing contained in this source code shall constitute any express or implied warranty of any kind, including without limitation, any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright or other third party intellectual property right. No warranty is made, express or implied, regarding the information¡¯s accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between Telechips and Company.
*
****************************************************************************/
#ifndef _VENC_H_
#define _VENC_H_

#include "../cdk/cdk_core.h"
#include "../cdk/cdk_port.h"
#ifdef INCLUDE_VPU_ENC
	#include "vpu/TCC_VPU_CODEC.h" // VPU video codec
#endif

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
	int m_iWFDTranscoding;				//!< Transcoding for WFD Source Device
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
