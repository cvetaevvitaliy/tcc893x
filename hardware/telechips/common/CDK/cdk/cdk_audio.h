/****************************************************************************
 *   FileName    : cdk_audio.h
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
#ifndef _CDK_AUDIO_H_
#define _CDK_AUDIO_H_
#include "adec_vars.h"
#include "aenc_vars.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************/
/*						                                                */
/* CDK Audio Funcs		                                                */
/*						                                                */
/************************************************************************/
cdk_result_t 
cdk_adec_init( cdk_core_t* pCdk, cdmx_info_t *pCdmxInfo, int iAdecType, int iCtype, cdk_audio_func_t* callback, ADEC_VARS* gsADec);
cdk_result_t 
cdk_adec_decode( cdk_core_t* pCdk, cdmx_info_t *pCdmxInfo, cdmx_output_t *pCdmxOut, int iAdecType, int iSeekFlag, int * alsa_status, ADEC_VARS* gsADec);
cdk_result_t 
cdk_adec_close( cdk_core_t* pCdk , ADEC_VARS* gsADec);

#if 0
cdk_result_t 
cdk_aenc_init( cdk_core_t* pCdk, cmux_info_t *pCmuxInfo, int iAencType, int iCtype);
cdk_result_t 
cdk_aenc_encode( cdk_core_t* pCdk, cmux_info_t *pCmuxInfo, cmux_input_t *pCdmxInput, int iAencType );
cdk_result_t 
cdk_aenc_close( cdk_core_t* pCdk );
#else
cdk_result_t
cdk_aenc_init( cdk_core_t* pCdk, cmux_info_t *pCmuxInfo, int iAdecType, int iCtype, cdk_func_t* callback, AENC_VARS* gsAEnc);
cdk_result_t
cdk_aenc_encode( cdk_core_t* pCdk, cmux_info_t *pCmuxInfo, cmux_input_t *pCmuxInput, int iAencType, AENC_VARS* gsAEnc );
cdk_result_t
cdk_aenc_close( cdk_core_t* pCdk, AENC_VARS* gsAEnc );
#endif

/************************************************************************/
/*						                                                */
/* CDK Audio Defines	                                                */
/*						                                                */
/************************************************************************/

#define AUDIO_ID_AAC 0
#define AUDIO_ID_MP3 1
#define AUDIO_ID_AMR 2
#define AUDIO_ID_AC3 3

//#define AUDIO_ID_PCM 4	//--> WAV
#define AUDIO_ID_MP3HD 4
#define AUDIO_ID_MP2 5
#define AUDIO_ID_DTS 6
#define AUDIO_ID_QCELP 7
#define AUDIO_ID_AMRWBP 8
#define AUDIO_ID_WMA 9
#define AUDIO_ID_EVRC 10
#define AUDIO_ID_FLAC 11
#define AUDIO_ID_COOK 12
#define AUDIO_ID_G722 13
#define AUDIO_ID_G729 14
#define AUDIO_ID_APE 15
//#define AUDIO_ID_MSADPCM 16	//--> WAV
#define AUDIO_ID_WAV 16
//#define AUDIO_ID_IMAADPCM 17	//--> WAV
#define AUDIO_ID_DRA 17
#define AUDIO_ID_VORBIS 18
#define AUDIO_ID_BSAC 19

#define AUDIO_ID_MP1  20
#define AUDIO_ID_DDP  21
#define AUDIO_ID_TRUEHD  23
#define AUDIO_ID_DTSHD   1006 // SSG
#define AUDIO_ID_AAC_GOOGLE	24


#define AUDIO_SIZE_MAX ( (4608 * 2) *8*2)	
#define AC3_BUFFER_SIZE		(1024*50)
#define AUDIO_MAX_INPUT_SIZE	(1024*96)

#define AUDIO_ERROR_THRESHOLD	1000

#define AUDIO_DECODING	0x10
#define AUDIO_ENCODING	0x20


#define CDK_AUDIO_RESERVED	0

typedef enum
{
	AUDIO_NORMAL_MODE		= 0x0002,	// normal playback mode (ex: file-play)
	AUDIO_BROADCAST_MODE	= 0x0004,	// broadcasting mode (ex: dab, dmb)
	AUDIO_DDP_TO_DD_MODE		= 0x0008,	// ddp to dd converting mode
}TCC_AUDIO_PROCESS_MODE;

/************************************************************************

				Audio Decoder List
				
*************************************************************************/
#ifdef INCLUDE_AAC_DEC
#define AAC_DECODER	TCC_AAC_DEC
#else
#define AAC_DECODER	0
#endif

#ifdef INCLUDE_MP3_DEC
#define MP3_DECODER	TCC_MP3_DEC
#else
#define MP3_DECODER	0
#endif

#ifdef INCLUDE_ARMNB_DEC
#define AMRNB_DECODER	TCC_AMRNB_DEC
#else
#define AMRNB_DECODER	0
#endif

#ifdef INCLUDE_DOLBY_DEC
#define AC3_DECODER	TCC_AC3_DEC
#define DDP_DECODER	TCC_DDP_DEC
#else
#define AC3_DECODER	0
#define DDP_DECODER	0
#endif

#ifdef INCLUDE_MP3HD_DEC
#define MP3HD_DECODER	TCC_MP3HD_DEC
#else
#define MP3HD_DECODER	0
#endif

#ifdef INCLUDE_MP2_DEC
#define MP2_DECODER	TCC_MP2_DEC
#else
#define MP2_DECODER	0
#endif

#ifdef INCLUDE_DTS_DEC
#define DTS_DECODER	TCC_DTS_DEC
#else
#define DTS_DECODER	0
#endif

#ifdef INCLUDE_QCELP_DEC
#define QCELP_DECODER	TCC_QCELP_DEC
#else
#define QCELP_DECODER	0
#endif

#ifdef INCLUDE_ARMWBPLUS_DEC
#define AMRWBPLUS_DECODER	TCC_AWP_DEC
#else
#define AMRWBPLUS_DECODER	0
#endif

#ifdef INCLUDE_WMA_DEC
#define WMA_DECODER	TCC_WMA_DEC
#else
#define WMA_DECODER	0
#endif	

#ifdef INCLUDE_EVRC_DEC
#define EVRC_DECODER	TCC_EVRC_DEC
#else
#define EVRC_DECODER	0
#endif	

#ifdef INCLUDE_FLAC_DEC
#define FLAC_DECODER	TCC_FLAC_DEC
#else
#define FLAC_DECODER	0
#endif	

#ifdef INCLUDE_RAG2_DEC
#define RAG2_DECODER	TCC_RAG2_DEC
#else
#define RAG2_DECODER	0
#endif	

#ifdef INCLUDE_G722_DEC
#define G722_DECODER	TCC_G722_DEC
#else
#define G722_DECODER	0
#endif	

#ifdef INCLUDE_G729AB_DEC
#define G729AB_DECODER	TCC_G729AB_DEC
#else
#define G729AB_DECODER	0
#endif	

#ifdef INCLUDE_APE_DEC
#define APE_DECODER	TCC_APE_DEC
#else
#define APE_DECODER	0
#endif	

#ifdef INCLUDE_WAV_DEC
#define WAV_DECODER	TCC_WAV_DEC
#else
#define WAV_DECODER	0
#endif

#ifdef INCLUDE_DRA_DEC
#define DRA_DECODER	TCC_DRA_DEC
#else
#define DRA_DECODER	0
#endif

#ifdef INCLUDE_VORBIS_DEC
#define VORBIS_DECODER	TCC_VORBIS_DEC
#else
#define VORBIS_DECODER	0
#endif	

#ifdef INCLUDE_BSAC_DEC
#define BSAC_DECODER	TCC_BSAC_DEC
#else
#define BSAC_DECODER	0
#endif

/************************************************************************

				Audio Encoder List
				
*************************************************************************/


#ifdef __cplusplus
}
#endif
#endif //_CDK_AUDIO_H_
