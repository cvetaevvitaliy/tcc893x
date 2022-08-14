/**

  @file omx_audiodec_component.h

  This file is header of audio decoder component.

  Copyright (C) 2007-2008  STMicroelectronics
  Copyright (C) 2007-2008 Nokia Corporation and/or its subsidiary(-ies).
  Copyright (C) 2009-2010 Telechips Inc.

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA
  02110-1301  USA

*/

#ifndef _OMX_AUDIODEC_COMPONENT_H_
#define _OMX_AUDIODEC_COMPONENT_H_

#include <OMX_Types.h>
#include <OMX_Component.h>
#include <OMX_Core.h>
#ifdef OPENMAX1_2
#include <OMX_AudioExt.h>
#endif
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <omx_base_filter.h>
#include "TCCMemory.h"

#include "OMX_TCC_Index.h"

#include "tcc_video_common.h"

#include "cdmx.h"
#include "cmux.h"
#include "cdk.h"
#include "cdk_audio.h"

// audio decoder base class
DERIVEDCLASS(omx_audiodec_component_PrivateType, omx_base_filter_PrivateType)
#define omx_audiodec_component_PrivateType_FIELDS omx_base_filter_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_PCMMODETYPE pAudioPcmMode; \
	OMX_U32 decode_ready; \
	int isNewBuffer; \
  	OMX_U32 audio_coding_type; \
	cdmx_info_t cdmx_info; \
	int iAdecType; \
	int iCtype; \
	OMX_TICKS iMuteSamples; \
	OMX_TICKS iGuardSamples; \
	OMX_U32 iNumOfSeek; \
	OMX_TICKS iStartTS; \
	OMX_TICKS iPrevTS; \
	OMX_TICKS iPrevOriginalTS; \
	OMX_TICKS iDuration; \
	OMX_BOOL bPrevDecFail; \
	OMX_U32	uiInputBufferCount; \
	/** for output buffer split*/ \
	OMX_U8* pRest; \
	OMX_U32 iRestSamples; \
	OMX_U32 iSplitLength; \
	OMX_U32 iSplitPosition; \
	OMX_BOOL bBitstreamOut; \
	OMX_U32 uiSplitBufferSampleCapacity; \
	/** for audio decoder*/ \	
	audio_pcminfo_t stPcmInfo; \
	audio_streaminfo_t stStreamInfo; \	
	OMX_U8*	pucStreamBuffer; \
	OMX_S32	iStreamBufferSize; \
	OMX_S32	iMinStreamSize; \
	OMX_S32 iDecodedSamplePerCh; \
	OMX_S32 iAudioProcessMode; \
	OMX_S32 iEndOfFile; \
	OMX_PTR pvCodecSpecific; \
	OMX_S32 iUseCodecSpecificTS; \
	OMX_S32	iExtractorType; \
	OMX_S32 iEmptyBufferDone; \
	OMX_TICKS uiNumSamplesOutput; \
	OMX_BOOL iUsingMergeBuffer; \
	OMX_HANDLE_FUNC pExternalDec; \
	OMX_S32 (*OpenCodec)(OMX_COMPONENTTYPE* openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer); \
	OMX_S32 (*DecodeCodec)(OMX_COMPONENTTYPE* openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer, OMX_BUFFERHEADERTYPE* outputbuffer); \
	OMX_S32 (*FlushCodec)(OMX_COMPONENTTYPE* openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer); \
	OMX_S32 (*CloseCodec)(OMX_COMPONENTTYPE* openmaxStandComp);
ENDCLASS(omx_audiodec_component_PrivateType)

// aac decoder class
DERIVEDCLASS(omx_aacdec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_aacdec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_AACPROFILETYPE pAudioAac; \
	adec_init_t stADecInit; \
	OMX_S32 iDecoderHandle; \
	cdk_callback_func_t stCallbackFunc; \
	OMX_U8* pAACRawData; \
	OMX_S32 iAACRawDataSize; \
	OMX_S32 iCurrInputBufferOrgSize; \
	OMX_PTR pvSubParser; \
	OMX_PTR pfLatmDLLModule; \
	OMX_PTR (*pfLatmInit)(unsigned char*, unsigned int, int*, int*, void*, int); \
	OMX_S32 (*pfLatmClose)(void*); \
	OMX_S32 (*pfLatmGetFrame)(void*, unsigned char*, int, unsigned char**, int*, unsigned int); \
	OMX_S32 (*pfLatmGetHeaderType)(void*);
ENDCLASS(omx_aacdec_component_PrivateType)

// ac3 decoder class
DERIVEDCLASS(omx_ac3dec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_ac3dec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_AC3TYPE pAudioAc3; \
	adec_init_t stADecInit; \
	OMX_S32 iDecoderHandle;
ENDCLASS(omx_ac3dec_component_PrivateType)

// ape decoder class
DERIVEDCLASS(omx_apedec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_apedec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_APETYPE pAudioApe; \
	adec_init_t stADecInit; \
	OMX_TICKS iSamples; \
	OMX_S32 iDecoderHandle;	
ENDCLASS(omx_apedec_component_PrivateType)

// ddp decoder class
DERIVEDCLASS(omx_ddpdec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_ddpdec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_AC3TYPE pAudioAc3; \
	adec_init_t stADecInit; \
	OMX_S32 iDecoderHandle;	
ENDCLASS(omx_ddpdec_component_PrivateType)

// dts decoder class
DERIVEDCLASS(omx_dtsdec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_dtsdec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_DTSTYPE pAudioDts; \
	adec_init_t stADecInit; \
	OMX_S32 iDecoderHandle;	
ENDCLASS(omx_dtsdec_component_PrivateType)

// flac decoder class
DERIVEDCLASS(omx_flacdec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_flacdec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_FLACTYPE pAudioFlac; \
	adec_init_t stADecInit; \
	OMX_S32 iDecoderHandle;	
ENDCLASS(omx_flacdec_component_PrivateType)

// mp2 decoder class
DERIVEDCLASS(omx_mp2dec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_mp2dec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_MP2TYPE pAudioMp2; \
	adec_init_t stADecInit; \
	OMX_S32 iDecoderHandle;	
ENDCLASS(omx_mp2dec_component_PrivateType)

// mp3 decoder class
DERIVEDCLASS(omx_mp3dec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_mp3dec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_MP3TYPE pAudioMp3; \
	adec_init_t stADecInit; \
	OMX_S32 iDecoderHandle;	
ENDCLASS(omx_mp3dec_component_PrivateType)

// real audio decoder class
DERIVEDCLASS(omx_radec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_radec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_RATYPE pAudioRa; \
	adec_init_t stADecInit; \
	OMX_S32 iDecoderHandle;	
ENDCLASS(omx_radec_component_PrivateType)

// vorbis decoder class
DERIVEDCLASS(omx_vorbisdec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_vorbisdec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_VORBISTYPE pAudioVorbis; \
	adec_init_t stADecInit; \
	OMX_S32 iDecoderHandle;	
ENDCLASS(omx_vorbisdec_component_PrivateType)

// wav decoder class
DERIVEDCLASS(omx_wavdec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_wavdec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_PCMMODETYPE pAudioPcm; \
	adec_init_t stADecInit; \
	OMX_S32 iDecoderHandle;	
ENDCLASS(omx_wavdec_component_PrivateType)

// wma decoder class
DERIVEDCLASS(omx_wmadec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_wmadec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_WMATYPE pAudioWma; \
	adec_init_t stADecInit; \
	OMX_S32 iDecoderHandle;	
ENDCLASS(omx_wmadec_component_PrivateType)



// common functions
void AudioInfo_print(cdmx_info_t *info);
OMX_ERRORTYPE omx_audiodec_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
void omx_audiodec_component_BufferMgmtCallback(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer, OMX_BUFFERHEADERTYPE* outputbuffer);
OMX_ERRORTYPE omx_audiodec_component_LibInit(omx_audiodec_component_PrivateType* omx_audiodec_component_Private);
OMX_ERRORTYPE omx_audiodec_component_LibDeinit(omx_audiodec_component_PrivateType* omx_audiodec_component_Private);
OMX_ERRORTYPE omx_audiodec_component_MessageHandler(OMX_COMPONENTTYPE* openmaxStandComp, internalRequestMessageType *message);
OMX_ERRORTYPE omx_audiodec_component_GetExtensionIndex(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_STRING cParameterName,
  OMX_OUT OMX_INDEXTYPE* pIndexType);
OMX_ERRORTYPE omx_audiodec_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp);

OMX_ERRORTYPE omx_audiodec_component_GetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_INOUT OMX_PTR ComponentParameterStructure);

OMX_ERRORTYPE omx_audiodec_component_SetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_IN  OMX_PTR ComponentParameterStructure);


void * WRAPPER_malloc(unsigned int size);
void * WRAPPER_calloc(unsigned int size, unsigned int count);
void WRAPPER_free(void * ptr);
void * WRAPPER_realloc(void * ptr, unsigned int size);
void * WRAPPER_memcpy(void* dest, const void* src, unsigned int size);
void WRAPPER_memset(void* ptr, int val, unsigned int size);
void * WRAPPER_memmove(void* dest, const void* src, unsigned int size);

#endif /* _OMX_AUDIODEC_COMPONENT_H_ */

