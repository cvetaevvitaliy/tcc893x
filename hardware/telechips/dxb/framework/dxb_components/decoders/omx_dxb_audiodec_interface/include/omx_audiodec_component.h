/**

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


#include "omx_base_filter.h"
#include "tcc_video_common.h"
#include "cmux.h"

#include "cdmx.h"
#include "cdk.h"
#include "cdk_audio.h"
#include <tccaudio.h>

typedef enum
{
	OMX_IndexParamAudioSetStart = 0x7E000001,  			/**< reference: OMX_AUDIO_PARAM_STARTTYPE */
	OMX_IndexParamAudioSetDABMode,
	OMX_IndexParamAudioSetGetSignalStrength,
	OMX_IndexParamAudioChangeCodec
}TCC_DXBAUDIO_OMX_INDEXVENDORTYPE;

typedef enum
{
	TCC_DXBAUDIO_OMX_Codec_AC3 = 0x100,
	TCC_DXBAUDIO_OMX_Codec_AAC_ADTS,
	TCC_DXBAUDIO_OMX_Codec_AAC_LATM,
	TCC_DXBAUDIO_OMX_Codec_AAC_PLUS,
	TCC_DXBAUDIO_OMX_Codec_MP2,
	TCC_DXBAUDIO_OMX_Codec_DDP,
	TCC_DXBAUDIO_OMX_Codec_MAX
}TCC_DXBAUDIO_OMX_CODECTYPE;

typedef enum
{
	TCC_DXBAUDIO_OMX_Dec_None = 0,
	TCC_DXBAUDIO_OMX_Dec_Stop,
	TCC_DXBAUDIO_OMX_Dec_Start,
	TCC_DXBAUDIO_OMX_Dec_MAX
}TCC_DXBAUDIO_OMX_DECTYPE;

typedef enum
{
	TCC_DXBAUDIO_DUALMONO_Left = 0,
	TCC_DXBAUDIO_DUALMONO_Right,
	TCC_DXBAUDIO_DUALMONO_LeftNRight,
	TCC_DXBAUDIO_DUALMONO_Mix,
} TCC_DXBAUDIO_DUALMONO_TYPE;


typedef struct OMX_AUDIO_PARAM_STARTTYPE {
	OMX_U32 nDevID;
    OMX_U32 nAudioFormat;              
    OMX_BOOL bStartFlag;              
} OMX_AUDIO_PARAM_STARTTYPE;

#define	AUDIO_INIT_BUFFER_SIZE  (32*1024)
typedef struct AudioStartInfo
{
	OMX_U32		nState;
	OMX_U32		nFormat;
	pthread_mutex_t mutex;
} AudioStartInfo;

typedef struct OMX_AUDIO_DEC_PRIVATE_DATA {
	OMX_BOOL bAudioStarted;
	OMX_BOOL bAudioPaused;
	OMX_BOOL bAudioInDec;
	OMX_TICKS iStartTS;
	OMX_TICKS iPrevTS;
	OMX_BOOL bPrevDecFail;
	OMX_TICKS iSamples;
	OMX_U32 iNumOfSeek;
	OMX_U32 decode_ready;
	cdk_core_t cdk_core;
	cdmx_info_t cdmx_info;
	cdmx_output_t cdmx_out;
	ADEC_VARS gsADec;
	int iAdecType;
	int iCtype;
	cdk_audio_func_t* cb_function;
  	OMX_U32 audio_coding_type;
	AudioStartInfo	stAudioStart;
	unsigned char	gucAudioInitBuffer[AUDIO_INIT_BUFFER_SIZE];
	unsigned int	guiAudioInitBufferIndex;
	OMX_U32 nPVRFlags;     // for pvr
} OMX_AUDIO_DEC_PRIVATE_DATA;

extern OMX_S16 iTotalHandle;

// audio decoder base class
DERIVEDCLASS(omx_audiodec_component_PrivateType, omx_base_filter_PrivateType)
#define omx_audiodec_component_PrivateType_FIELDS omx_base_filter_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_PCMMODETYPE pAudioPcmMode[TOTAL_AUDIO_TRACK]; \
	OMX_BUFFERHEADERTYPE *temporary_buffer; \
	OMX_AUDIO_DEC_PRIVATE_DATA *pAudioDecPrivate; \
	OMX_U32 outDevID; \
	OMX_U8* temp_buffer; \
	int isNewBuffer; \
	int audio_decode_state; \
  	int *Power_Spectrum_info; \
	int *Energy_Volume_level; \
	cdk_callback_func_t callback_func; \
	OMX_TICKS iGuardSamples; \
	OMX_U32 iSPDIFMode; \
	OMX_U8* spdif_pBuffer; \
	OMX_S32 spdif_nFilledLength; \
	OMX_S32 spdif_nConsumedLength; \
	OMX_BOOL bDABMode; \
	OMX_U32 uiCountry; \
	TCC_DXBAUDIO_DUALMONO_TYPE eStereoMode; \
	int (*callbackfunction)(void*);
ENDCLASS(omx_audiodec_component_PrivateType)

// aac decoder class
DERIVEDCLASS(omx_aacdec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_aacdec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_AACPROFILETYPE pAudioAac; \
	TCC_DXBAUDIO_DUALMONO_TYPE	eAudioDualMonoSel;
ENDCLASS(omx_aacdec_component_PrivateType)

// ac3 decoder class
DERIVEDCLASS(omx_ac3dec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_ac3dec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_AC3TYPE pAudioAc3;
ENDCLASS(omx_ac3dec_component_PrivateType)

// ape decoder class
DERIVEDCLASS(omx_apedec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_apedec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_APETYPE pAudioApe;
ENDCLASS(omx_apedec_component_PrivateType)

// ddp decoder class
DERIVEDCLASS(omx_ddpdec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_ddpdec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_AC3TYPE pAudioAc3;
ENDCLASS(omx_ddpdec_component_PrivateType)

// dts decoder class
DERIVEDCLASS(omx_dtsdec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_dtsdec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_DTSTYPE pAudioDts;
ENDCLASS(omx_dtsdec_component_PrivateType)

// flac decoder class
DERIVEDCLASS(omx_flacdec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_flacdec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_FLACTYPE pAudioFlac;
ENDCLASS(omx_flacdec_component_PrivateType)

// mp2 decoder class
DERIVEDCLASS(omx_mp2dec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_mp2dec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_MP2TYPE pAudioMp2;
ENDCLASS(omx_mp2dec_component_PrivateType)

// mp3 decoder class
DERIVEDCLASS(omx_mp3dec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_mp3dec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_MP3TYPE pAudioMp3; 
ENDCLASS(omx_mp3dec_component_PrivateType)

// real audio decoder class
DERIVEDCLASS(omx_radec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_radec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_RATYPE pAudioRa;
ENDCLASS(omx_radec_component_PrivateType)

// vorbis decoder class
DERIVEDCLASS(omx_vorbisdec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_vorbisdec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_VORBISTYPE pAudioVorbis;
ENDCLASS(omx_vorbisdec_component_PrivateType)

// wav decoder class
DERIVEDCLASS(omx_wavdec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_wavdec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_PCMMODETYPE pAudioPcm;
ENDCLASS(omx_wavdec_component_PrivateType)

// wma decoder class
DERIVEDCLASS(omx_wmadec_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_wmadec_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_AUDIO_PARAM_WMATYPE pAudioWma;
ENDCLASS(omx_wmadec_component_PrivateType)

// spdif passthrough class
DERIVEDCLASS(omx_spdif_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_spdif_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_U32 pSPDIFMode;
ENDCLASS(omx_spdif_component_PrivateType)

DERIVEDCLASS(omx_dxb_audio_default_component_PrivateType, omx_audiodec_component_PrivateType)
#define omx_dxb_audio_default_component_PrivateType_FIELDS omx_audiodec_component_PrivateType_FIELDS \
	OMX_U32 dummy[1024];
ENDCLASS(omx_dxb_audio_default_component_PrivateType)


// common functions
void omx_audiodec_component_BufferMgmtCallback(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer, OMX_BUFFERHEADERTYPE* outputbuffer);
OMX_ERRORTYPE omx_audiodec_component_LibInit(OMX_S16 nDevID, omx_audiodec_component_PrivateType* omx_audiodec_component_Private);
OMX_ERRORTYPE omx_audiodec_component_LibDeinit(OMX_S16 nDevID, omx_audiodec_component_PrivateType* omx_audiodec_component_Private);
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

OMX_ERRORTYPE OMX_DXB_AudioDec_Default_ComponentInit(OMX_COMPONENTTYPE *openmaxStandComp);

#endif /* _OMX_AUDIODEC_COMPONENT_H_ */

