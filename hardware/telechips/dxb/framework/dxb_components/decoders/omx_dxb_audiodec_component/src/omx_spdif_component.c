/**

  @file omx_spdif_component.c

  This component implement generating spdif frame.

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

#include <omxcore.h>
#include <omx_base_audio_port.h>

#include <omx_spdif_component.h>
#include <tccaudio.h>
#include "ac3_header.h"
#include "dts_header.h"
#include <OMX_TCC_Index.h>
#include "TCCFile.h"
#include "TCCMemory.h"

#ifdef HAVE_ANDROID_OS
#define USE_EXTERNAL_BUFFER 0
#define LOG_TAG	"OMX_TCC_SPDIF"
#include <utils/Log.h>
#include <cutils/properties.h>

#define TCC_DDP_TRANSCODEING_TO_DD
//#define DEBUG_ON
#ifdef DEBUG_ON
#define DBUG_MSG(x...)	ALOGD(x)
#else
#define DBUG_MSG(x...)
#endif

//#define MEM_DEBUG
#ifdef MEM_DEBUG
#define LOGMSG(x...) ALOGD(x)
#else
#define LOGMSG(x...)
#endif

#endif /* HAVE_ANDROID_OS */

#ifdef TCC_DDP_TRANSCODEING_TO_DD
#include "cdmx.h"
#include "cmux.h"
#include "cdk.h"
#include "cdk_audio.h"
#endif

#define SPDIF_BUFFER_SIZE	(1024 * 1024)

#define		BUFFER_INDICATOR	   "TDXB"
#define		BUFFER_INDICATOR_SIZE  4

spdif_header_info_s mSpdif_info;
unsigned char TRUEHD_MAT_Buffer[61440];
int TRUEHD_Count;

int DDP_remaining_length=-1;
int DDP_Count;
int DDP_buffer_filled;

#ifdef TCC_DDP_TRANSCODEING_TO_DD
enum
{
	DDP_TO_DD_NONE = 0,
	DDP_TO_DD_OK,
	DDP_TO_DD_NEED_MORE_DATA,
	DDP_TO_DD_ERROR
};

static cdk_core_t gsCdkCore;
static cdmx_info_t gsCdmxInfo;
static cdmx_output_t gsCdmxOut;
static cdk_callback_func_t gsCallbackFunc;
//static cdk_audio_func_t* gsCbFunction;
static ADEC_VARS gsADec;
static int giAdecType;
unsigned char * gptempPCMbuffer =NULL;

static void CloseDCV( omx_spdif_component_PrivateType* pPrivate, cdk_core_t* pCdk , ADEC_VARS* pgsADec);
#endif

enum
{
	SPDIF_MODE_NORMAL = 0,
	SPDIF_MODE_HD,
	SPDIF_DDP_TO_DD
};
// ---------------------------------------------------------
// ---------------------------------------------------------

void PrintHexData(OMX_U8* p)
{
	ALOGD("------------------------------");
	ALOGD("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
	ALOGD("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", p[0+16], p[1+16], p[2+16], p[3+16], p[4+16], p[5+16], p[6+16], p[7+16], p[8+16], p[9+16], p[10+16], p[11+16], p[12+16], p[13+16], p[14+16], p[15+16]);
	ALOGD("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", p[0+32], p[1+32], p[2+32], p[3+32], p[4+32], p[5+32], p[6+32], p[7+32], p[8+32], p[9+32], p[10+32], p[11+32], p[12+32], p[13+32], p[14+32], p[15+32]);
	ALOGD("------------------------------");
}

#if 0
OMX_ERRORTYPE OMX_ComponentInit(OMX_HANDLETYPE openmaxStandComp, OMX_STRING cCompontName)
{
  OMX_ERRORTYPE err = OMX_ErrorNone;

	//err = omx_spdif_component_Constructor(openmaxStandComp,cCompontName);

	return err;
}
#endif

// library init function
OMX_ERRORTYPE omx_spdif_component_LibInit(OMX_S16 nDevID, omx_spdif_component_PrivateType* omx_spdif_component_Private) 
{
	DBUG_MSG("In %s\n", __func__);

	if (omx_spdif_component_Private->spdif_pBuffer == NULL)
	{
		omx_spdif_component_Private->spdif_pBuffer = (OMX_U8*)malloc(SPDIF_BUFFER_SIZE);
		spdif_parse_init();
	}

	return OMX_ErrorNone;;
}

// library de-init function
OMX_ERRORTYPE omx_spdif_component_LibDeinit(OMX_S16 nDevID, omx_spdif_component_PrivateType* omx_spdif_component_Private) 
{
	DBUG_MSG("In %s\n", __func__);

	if (omx_spdif_component_Private->spdif_pBuffer)
	{
		spdif_parse_deinit();
		free(omx_spdif_component_Private->spdif_pBuffer);
		omx_spdif_component_Private->spdif_pBuffer = NULL;
	}

	if(omx_spdif_component_Private->pSPDIFMode == SPDIF_DDP_TO_DD)
	{
		CloseDCV(omx_spdif_component_Private,&gsCdkCore, &gsADec);
	}
	return OMX_ErrorNone;
}

#ifdef TCC_DDP_TRANSCODEING_TO_DD
static void CloseDCV(omx_spdif_component_PrivateType* pPrivate, cdk_core_t* pCdk , ADEC_VARS* pgsADec)
{
	if(pCdk->m_iAudioHandle != 0)// && pPrivate->pExternalDec)
	{
		cdk_adec_close(pCdk, pgsADec);
		pCdk->m_iAudioHandle = 0;
	}

	if(pgsADec->gpucAudioBuffer)
	{
		pCdk->m_psCallback->m_pfFree(pgsADec->gpucAudioBuffer);
		pgsADec->gpucAudioBuffer = NULL;
	}
	return;
}

static OMX_S32 InitDCV( omx_spdif_component_PrivateType* pPrivate, cdk_core_t* pCdk, cdmx_info_t *pCdmxInfo, ADEC_VARS* pgsADec)
{
#ifdef SUPPORT_DDP_CODEC
	int ret = CDK_ERR_NONE;

	/* audio common */
	pgsADec->gsADecInput.m_eSampleRate = pCdmxInfo->m_sAudioInfo.m_iSamplePerSec;
	pgsADec->gsADecInput.m_uiNumberOfChannel = pCdmxInfo->m_sAudioInfo.m_iChannels;

	/* audio common extra */
	pgsADec->gsADecInit.m_pucExtraData = pCdmxInfo->m_sAudioInfo.m_pExtraData;
	pgsADec->gsADecInit.m_iExtraDataLen = pCdmxInfo->m_sAudioInfo.m_iExtraDataLen;

	/* out-pcm setting */
	pgsADec->gsADecOutput.m_ePcmInterLeaved = 1;	// 0 or 1
	pgsADec->gsADecOutput.m_iNchannelOrder[CH_LEFT_FRONT] = 1;	//first channel
	pgsADec->gsADecOutput.m_iNchannelOrder[CH_RIGHT_FRONT] = 2;	//second channel

	/* audio callback */
	pgsADec->gsADecInit.m_pfMalloc = pCdk->m_psCallback->m_pfMalloc;
	pgsADec->gsADecInit.m_pfRealloc = pCdk->m_psCallback->m_pfRealloc;
	pgsADec->gsADecInit.m_pfFree = pCdk->m_psCallback->m_pfFree;
	pgsADec->gsADecInit.m_pfMemcpy = pCdk->m_psCallback->m_pfMemcpy;
	pgsADec->gsADecInit.m_pfMemmove = pCdk->m_psCallback->m_pfMemmove;
	pgsADec->gsADecInit.m_pfMemset = pCdk->m_psCallback->m_pfMemset;

	pgsADec->gsADecInit.m_psAudiodecInput = &pgsADec->gsADecInput;
	pgsADec->gsADecInit.m_psAudiodecOutput = &pgsADec->gsADecOutput;

	pgsADec->giAudioErrorCount = 0;
	pgsADec->guiAudioMinDataSize = 0;
	if(pgsADec->gpucAudioBuffer == NULL)
	{
		pgsADec->guiAudioInBufferSize = AUDIO_MAX_INPUT_SIZE;
		pgsADec->gpucAudioBuffer = pgsADec->gsADecInit.m_pfMalloc(pgsADec->guiAudioInBufferSize);
		if(pgsADec->gpucAudioBuffer == NULL)
		{
			return -1;
		}
	}

	pgsADec->gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_iDDPStreamBufNwords = pgsADec->guiAudioInBufferSize/2;
	pgsADec->gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_pcMixdataTextbuf = NULL;
	pgsADec->gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_iMixdataTextNsize = 0;
	pgsADec->gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_iCompMode = 2;
	pgsADec->gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_iOutLfe = 0;
	pgsADec->gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_iOutChanConfig = 2;
	pgsADec->gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_iPcmScale = 100;
	pgsADec->gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_iStereMode = 0;
	pgsADec->gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_iDualMode = 0;
	//pgsADec->gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_iDynScaleHigh = 100;
	//pgsADec->gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_iDynScaleLow = 100;
	pgsADec->gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_iKMode = 3;
	pgsADec->gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_iFrmDebugFlags = 0;
	pgsADec->gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_iDecDebugFlags = 0;
	pgsADec->gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_iNumOutChannels = 2;
	pgsADec->gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_imixdataout = 0;
	pgsADec->gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_iQuitOnErr = 0;
	pgsADec->gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_iSRCMode = 0;
	//if(pCdk->m_iAudioProcessMode == AUDIO_DDP_TO_DD_MODE) // when DDP->DD passthru
		pgsADec->gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_iOutPutMode = 2;// DD only
	
	ret = cdk_adec_init(pCdk, pCdmxInfo, AUDIO_ID_DDP, CONTAINER_TYPE_AVI, TCC_DDP_DEC, pgsADec);

	if(ret != TCAS_SUCCESS)
	{
		CloseDCV(pPrivate, pCdk, pgsADec);
		return ret;
	}

	return ret;
#else
	return -1;
#endif
}

static OMX_S32 ProcessDCV(omx_spdif_component_PrivateType* pPrivate, cdk_core_t* pCdk, cdmx_info_t *pCdmxInfo, cdmx_output_t *pCdmxOut, int iSeekFlag, ADEC_VARS* pgsADec)
{
	int ret = CDK_ERR_NONE;

	pCdmxOut->m_iDecodedSamples = 0;

	if(pgsADec->gsADecInput.m_iStreamLength < 0)
		pgsADec->gsADecInput.m_iStreamLength = 0;

	if((pgsADec->gsADecInput.m_iStreamLength + pCdmxOut->m_iPacketSize) > (int)pgsADec->guiAudioInBufferSize)
	{
		pgsADec->guiAudioInBufferSize = pgsADec->gsADecInput.m_iStreamLength + pCdmxOut->m_iPacketSize;
		pgsADec->gpucAudioBuffer = pgsADec->gsADecInit.m_pfRealloc(pgsADec->gpucAudioBuffer, pgsADec->guiAudioInBufferSize);

		if(pgsADec->gpucAudioBuffer == NULL)
		{
			return -1;
		}
	}

	pgsADec->gsADecInit.m_pfMemcpy(pgsADec->gpucAudioBuffer + pgsADec->gsADecInput.m_iStreamLength, pCdmxOut->m_pPacketData, pCdmxOut->m_iPacketSize);
	pgsADec->gsADecInput.m_pcStream = (TCAS_S8*)pgsADec->gpucAudioBuffer;
	pgsADec->gsADecInput.m_iStreamLength += pCdmxOut->m_iPacketSize;

	while(pgsADec->gsADecInput.m_iStreamLength > (int)pgsADec->guiAudioMinDataSize)
	{
		// cdk decoder do not move buffer start position to the end of the last decoded data
		pgsADec->gsADecOutput.m_pvChannel[0] = (void *)(pCdk->m_pOutWav + pgsADec->gsADecOutput.m_uiNumberOfChannel * pCdmxOut->m_iDecodedSamples * sizeof(short) );

		ret = cdk_adec_decode(pCdk, pCdmxInfo, pCdmxOut, AUDIO_ID_DDP, iSeekFlag, 0, pgsADec);

		if(ret != TCAS_SUCCESS)
		{
			pgsADec->giAudioErrorCount++;
			if(pgsADec->giAudioErrorCount > AUDIO_ERROR_THRESHOLD)
			{
				return ret;
			}

			switch(ret)
			{
			case TCAS_ERROR_SKIP_FRAME:
				//ALOGD("TCAS_ERROR_SKIP_FRAME");

			case TCAS_ERROR_DDP_NOT_SUPPORT_FRAME:
				//ALOGD("TCAS_ERROR_DDP_NOT_SUPPORT_FRAME");
				pgsADec->giAudioErrorCount = 0;
				break;

			case TCAS_ERROR_MORE_DATA:
				//ALOGD("TCAS_ERROR_MORE_DATA");
				if(pgsADec->gsADecInput.m_iStreamLength > 0)
					pgsADec->gsADecInit.m_pfMemmove(pgsADec->gpucAudioBuffer, pgsADec->gsADecInput.m_pcStream, pgsADec->gsADecInput.m_iStreamLength);
				return 1;	//need more data

			default:
				//ALOGD("default");
				if(pgsADec->gsADecInput.m_iStreamLength > 0)
				{
					pgsADec->gsADecInit.m_pfMemmove(pgsADec->gpucAudioBuffer, pgsADec->gsADecInput.m_pcStream, pgsADec->gsADecInput.m_iStreamLength);
				}
				return 2;	// keep going
			}
		}
		else
		{
			if(pgsADec->giAudioErrorCount != 0)
			{
				pgsADec->giAudioErrorCount = 0;	//reset error count
			}
			pCdmxOut->m_iDecodedSamples += pgsADec->gsADecOutput.m_uiSamplesPerChannel;
		}
	}

	if(pgsADec->gsADecInput.m_iStreamLength > 0)
	{
		pgsADec->gsADecInit.m_pfMemmove(pgsADec->gpucAudioBuffer, pgsADec->gsADecInput.m_pcStream, pgsADec->gsADecInput.m_iStreamLength);
	}

	if(ret == TCAS_ERROR_DDP_NOT_SUPPORT_FRAME)
	{
		return 1;	/* check ddp dependent frame */
	}

	if(ret != TCAS_SUCCESS)
	{
		return 2;
	}

	return ret;
}

static int init_ddp_to_dd(omx_spdif_component_PrivateType* pPrivate, OMX_BUFFERHEADERTYPE* inputbuffer, int uiDecoderID)
{
	int lCtype;

	pPrivate->pAudioDecPrivate[uiDecoderID].decode_ready = OMX_FALSE;
	memset(&gsCdkCore, 0x00, sizeof(cdk_core_t));
	memset(&gsCdmxInfo, 0x00, sizeof(cdmx_info_t));
	memset(&gsCdmxOut, 0x00, sizeof(cdmx_output_t));

	gsCdkCore.m_psCallback = &(gsCallbackFunc);
	gsCdkCore.m_psCallback->m_pfMalloc   = (void* (*) ( unsigned int ))malloc;
	gsCdkCore.m_psCallback->m_pfRealloc  = (void* (*) ( void*, unsigned int ))realloc;
	gsCdkCore.m_psCallback->m_pfFree	  = (void  (*) ( void* ))free;
	gsCdkCore.m_psCallback->m_pfMemcpy   = (void* (*) ( void*, const void*, unsigned int ))memcpy;
	gsCdkCore.m_psCallback->m_pfMemmove  = (void* (*) ( void*, const void*, unsigned int ))memmove;
	gsCdkCore.m_psCallback->m_pfMemset   = (void  (*) ( void*, int, unsigned int ))memset;

	memcpy(&(gsCdmxInfo.m_sAudioInfo), (void*)inputbuffer->pBuffer, sizeof(gsCdmxInfo.m_sAudioInfo));
	//AudioInfo_print(&gsCdmxInfo);

	memset(&gsADec, 0, sizeof(ADEC_VARS));
	memset(TRUEHD_MAT_Buffer, 0,61440);

	if (gptempPCMbuffer == NULL)
	{
		gptempPCMbuffer = (unsigned char*)malloc(16*1024);
		memset(gptempPCMbuffer, 0,16*1024);
	}

//	giAdecType 	= AUDIO_ID_DDP;
//	gsCbFunction	= TCC_DDP_DEC;
//	gsCdkCore.m_iAudioProcessMode = AUDIO_DDP_TO_DD_MODE;

	char value[PROPERTY_VALUE_MAX];
	property_get("tcc.audio.ddp.drc_high", value, "100");
	int drc_value;
	drc_value = atoi(value);

	if(drc_value >=0 && drc_value <=100 ){
		gsADec.gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_iDynScaleHigh = drc_value;
	}
	else{
		gsADec.gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_iDynScaleHigh  = 100;
	}

	property_get("tcc.audio.ddp.drc_low", value, "100");
	drc_value = atoi(value);
	if(drc_value >=0 && drc_value <=100 ){
		gsADec.gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_iDynScaleLow = drc_value;
	}
	else{
		gsADec.gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_iDynScaleLow = 100;
	}

	lCtype = (int)(*(inputbuffer->pBuffer + sizeof(gsCdmxInfo.m_sAudioInfo)));

	if( InitDCV(pPrivate,&gsCdkCore,
				&gsCdmxInfo,
//				gsCbFunction,
				&gsADec) < 0 )	// Audio decoder function
	{
		ALOGE("Audio DEC init error.");
		inputbuffer->nFlags &= ~OMX_BUFFERFLAG_CODECCONFIG;
		// to skip all audio data
		inputbuffer->nFilledLen = 0;
		return 0;
	}

	pPrivate->pAudioDecPrivate[uiDecoderID].decode_ready  = OMX_TRUE;
	pPrivate->isNewBuffer = 1;

	return 1;
}

static int convert_ddp_to_dd(omx_spdif_component_PrivateType* pPrivate, OMX_BUFFERHEADERTYPE* inputbuffer ,OMX_S16 seekFlag)
{
	OMX_S32 ret	= 0;
	OMX_S16 iSeekFlag = seekFlag;

	gsCdmxOut.m_uiUseCodecSpecific = 0;
	TRUEHD_Count =0;

	/* Decode the block */
	if(gptempPCMbuffer)
		gsCdkCore.m_pOutWav = gptempPCMbuffer;
	else
		gsCdkCore.m_pOutWav = (OMX_U8*)TRUEHD_MAT_Buffer;

	gsCdmxOut.m_pPacketData = inputbuffer->pBuffer;
	gsCdmxOut.m_iPacketSize = inputbuffer->nFilledLen;

	ret = ProcessDCV(pPrivate,&gsCdkCore,
			&gsCdmxInfo,
			&gsCdmxOut,
			iSeekFlag,
			&gsADec);

	if (ret >= 0)
	{
		if(gsADec.gsADecOutput.m_pvExtraInfo)
		{
			int *extradata = (int*)(gsADec.gsADecOutput.m_pvExtraInfo);
			int ddstream_length;
			unsigned char *p_ddstream;

			p_ddstream = *extradata;
			TRUEHD_Count = ddstream_length = *(extradata + 1);

			memcpy(TRUEHD_MAT_Buffer, p_ddstream, ddstream_length);
		}

		if(TRUEHD_Count == 0)
		{
			return DDP_TO_DD_NEED_MORE_DATA;
		}

		return DDP_TO_DD_OK;
	}
	else
	{
		ALOGE( "cdk_audio_dec_run error: %ld", ret );
		return DDP_TO_DD_ERROR;
	}
}
#endif

#define DDP_STRMTYP_MASK 0xC0
#define DDP_SUBSTREAMID_MASK 0x38
#define DDP_BIT_STREAM_ID_MASK  0x80

static int parse_ddp_frame(unsigned char* pbuffer ,unsigned int nFilledLen)
{
	int foundPayload = 0;
	int count = 0;
	unsigned short length=0;
	unsigned char * pucInputBuf = pbuffer;
	unsigned char checkValue;
	int foundDefendent =0;

	for (count = 0 ; count < nFilledLen-5; count++)
	{
		//To find sync info
		if (pucInputBuf[count] == 0x0B && pucInputBuf[count+1] == 0x77)
		{
			foundDefendent = 0;
			//To find first frame of data-burst payload
			if((((pucInputBuf[count+2] & DDP_STRMTYP_MASK) >>6)== 0 ||((pucInputBuf[count+2] & DDP_STRMTYP_MASK)>>6)== 2)
			&& ((pucInputBuf[count+2] & DDP_SUBSTREAMID_MASK) >>3)==0 )
			{
				//This frame will begin data-burst payload,i.e Indefendant substream
				foundPayload ++;
				checkValue = (pucInputBuf[count+4] & 0xf0);
			}
			else if(foundPayload && (((pucInputBuf[count+2] & DDP_STRMTYP_MASK)>>6)==1))
			{
				//compare with Indefendent's numblkscod, and bsid
				if((checkValue == (pucInputBuf[count+4] & 0xf0)) /*&& ( pucInputBuf[count+5] & DDP_BIT_STREAM_ID_MASK )*/)
				{
					//defendent substream
					foundDefendent =1;
				}
			}

			if(foundPayload > 1 ){
				// Another substream detect, previous stream have to be send to external decoder
				return count;
			}

			// check the frame size to check improper sysn info
			length = ((pucInputBuf[count+2] & 0x03) << 8) |(pucInputBuf[count+3] & 0xff);

			count += (length *2);

			if(foundDefendent && count > nFilledLen)
			{
				DDP_remaining_length = count-nFilledLen+2;
			}
		}
	}

	// One frame is exist, this frame will be sent to external decoder
	return count +5;
}
void omx_spdif_component_BufferMgmtCallback(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer, OMX_BUFFERHEADERTYPE* outputbuffer)
{
	omx_spdif_component_PrivateType* pPrivate = openmaxStandComp->pComponentPrivate;  

	OMX_S32 ret	= 0;
	OMX_U8* input_ptr = inputbuffer->pBuffer;
	OMX_S16 iSeekFlag = 0;
	unsigned int uiDemuxID = 0;
	unsigned int uiDecoderID = 0;
	unsigned int enStreamType;
	int isDDPConverted = 0;

	if( memcmp(inputbuffer->pBuffer,BUFFER_INDICATOR,BUFFER_INDICATOR_SIZE)==0)
    {
		memcpy(&input_ptr, inputbuffer->pBuffer + BUFFER_INDICATOR_SIZE, 4);		
		if(inputbuffer->pBuffer[BUFFER_INDICATOR_SIZE+4] == 0xAA)
        {
            uiDemuxID = inputbuffer->pBuffer[BUFFER_INDICATOR_SIZE+5];
            uiDecoderID = inputbuffer->pBuffer[BUFFER_INDICATOR_SIZE+6];
            //ALOGD("%s:Demuxer[%d]Decoder[%d]",__func__, uiDemuxID, uiDecoderID);
            if(uiDecoderID == 1)
            {
                inputbuffer->nFilledLen = 0;
        		outputbuffer->nFilledLen = 0;
		        return;
            }
        }
    }	    

	outputbuffer->nFilledLen = 0;
	outputbuffer->nOffset = 0;

	DBUG_MSG("BufferMgmtCallback IN inLen = %u, Flags = 0x%x, Timestamp = %lld", inputbuffer->nFilledLen, inputbuffer->nFlags, inputbuffer->nTimeStamp);
	//if((inputbuffer->nFlags & OMX_BUFFERFLAG_CODECCONFIG) && pPrivate->pAudioDecPrivate[uiDecoderID].decode_ready == OMX_FALSE)
	if( pPrivate->pAudioDecPrivate[uiDecoderID].decode_ready == OMX_FALSE )
	{
		if (pPrivate->pAudioDecPrivate[uiDecoderID].audio_coding_type == OMX_AUDIO_CodingAC3) 
			omx_audiodec_component_Change_SPDIF(openmaxStandComp, AUDIO_DEC_AC3_NAME, uiDecoderID);

		pPrivate->spdif_nFilledLength = 0;
		pPrivate->spdif_nConsumedLength = 0;

		pPrivate->pAudioDecPrivate[uiDecoderID].decode_ready  = OMX_TRUE;
		pPrivate->isNewBuffer = 1;
		outputbuffer->nFilledLen = 0;
		inputbuffer->nFilledLen = 0;

		char value[PROPERTY_VALUE_MAX];
		property_get("persist.sys.spdif_setting", value, "");

		if (!strcmp(value, "4"))
		{
			ALOGD("SPDIF HDMI mode");
			pPrivate->pSPDIFMode = SPDIF_MODE_HD;
		
			if((pPrivate->pAudioDecPrivate[uiDecoderID].audio_coding_type == OMX_AUDIO_CodingAC3
				&& pPrivate->pAudioDecPrivate[uiDecoderID].iAdecType == AUDIO_ID_AC3)
				|| (pPrivate->pAudioDecPrivate[uiDecoderID].audio_coding_type == OMX_AUDIO_CodingDTS))
			{
				pPrivate->pSPDIFMode = SPDIF_MODE_NORMAL;
				property_set("tcc.hdmi.audio_type", "2");
			}
			else if(pPrivate->pAudioDecPrivate[uiDecoderID].audio_coding_type == OMX_AUDIO_CodingAC3
					&& pPrivate->pAudioDecPrivate[uiDecoderID].iAdecType == AUDIO_ID_DDP)
			{
				ALOGD("HDMI Passthru ");
				property_set("tcc.hdmi.audio_type", "1");
				DDP_remaining_length = -1;
				DDP_buffer_filled =0;
				DDP_Count =0;
			}
			else
			{
				property_set("tcc.hdmi.audio_type", "0");
			}
		}
		
#ifdef TCC_DDP_TRANSCODEING_TO_DD		
		else if (!strcmp(value, "2"))
		{
			ALOGD("SPDIF normal mode");
			pPrivate->pSPDIFMode = SPDIF_MODE_NORMAL;

			char checkDDP[PROPERTY_VALUE_MAX];
			property_get("tcc.audio.support.ddp", checkDDP, "0");

			if(pPrivate->pAudioDecPrivate[uiDecoderID].audio_coding_type == OMX_AUDIO_CodingAC3
				&& pPrivate->pAudioDecPrivate[uiDecoderID].iAdecType == AUDIO_ID_DDP)
			{
				if(init_ddp_to_dd(pPrivate, inputbuffer, uiDecoderID)){
					ALOGD("SPDIF DDP to DD mode");
					pPrivate->pSPDIFMode = SPDIF_DDP_TO_DD;
				}
			}
		}
#endif		
		else
		{
			ALOGD("SPDIF normal mode");
			pPrivate->pSPDIFMode = SPDIF_MODE_NORMAL;
		}

		DBUG_MSG("Audio DEC initialized.");
	

#if defined(DUMP_INPUT_TO_FILE) || defined(DUMP_OUTPUT_TO_FILE)
IODumpFileInit();
#endif
		return;
	}
	
	if(inputbuffer->nFlags & OMX_BUFFERFLAG_SYNCFRAME)
	{
		iSeekFlag = 1;
		pPrivate->pAudioDecPrivate[uiDecoderID].iStartTS = inputbuffer->nTimeStamp;
		pPrivate->pAudioDecPrivate[uiDecoderID].iSamples = 0;
	}

#if defined(DUMP_INPUT_TO_FILE)
IODumpFileWriteInputData(input_ptr, inputbuffer->nFilledLen);
#endif
	if(pPrivate->pSPDIFMode == SPDIF_DDP_TO_DD)
		isDDPConverted = convert_ddp_to_dd(pPrivate, inputbuffer, iSeekFlag);

	if (pPrivate->pAudioDecPrivate[uiDecoderID].audio_coding_type == OMX_AUDIO_CodingAC3 || pPrivate->pAudioDecPrivate[uiDecoderID].audio_coding_type == OMX_AUDIO_CodingDTS) 
	{
		int	iDataOffset;
		unsigned char *pucInputBuf = NULL;
		int nInputSize = 0;
		int	iNextHeaderOffset = 0;
		int	nFlags, nSampleRate, nBitRate;
		int found_header;

		if (pPrivate->isNewBuffer)
		{
			if (pPrivate->spdif_nConsumedLength < pPrivate->spdif_nFilledLength)
			{
				memcpy(&pPrivate->spdif_pBuffer[0], &pPrivate->spdif_pBuffer[pPrivate->spdif_nConsumedLength], (pPrivate->spdif_nFilledLength - pPrivate->spdif_nConsumedLength));
				pPrivate->spdif_nFilledLength -= pPrivate->spdif_nConsumedLength;
				pPrivate->spdif_nConsumedLength = 0;
			}
			else
			{
				memset(&mSpdif_info, 0, sizeof(spdif_header_info_s));
				pPrivate->spdif_nFilledLength = 0;
				pPrivate->spdif_nConsumedLength = 0;
			}

			#ifdef TCC_DDP_TRANSCODEING_TO_DD
			if(pPrivate->pSPDIFMode == SPDIF_DDP_TO_DD)
			{
				switch(isDDPConverted)
				{
				case DDP_TO_DD_NONE:
					if ((pPrivate->spdif_nFilledLength + inputbuffer->nFilledLen) <= SPDIF_BUFFER_SIZE)
					{
						memcpy(&pPrivate->spdif_pBuffer[pPrivate->spdif_nFilledLength], input_ptr, inputbuffer->nFilledLen);
						pPrivate->spdif_nFilledLength += inputbuffer->nFilledLen;
					}
					else
					{
						ALOGE("DDP-DD :: input data length is too many to copy !!");
					}
					break;

				case DDP_TO_DD_OK:
					if ((pPrivate->spdif_nFilledLength + TRUEHD_Count) <= SPDIF_BUFFER_SIZE)
					{
						memcpy(&pPrivate->spdif_pBuffer[pPrivate->spdif_nFilledLength], TRUEHD_MAT_Buffer, TRUEHD_Count);
						pPrivate->spdif_nFilledLength += TRUEHD_Count;
					}
					else
					{
						ALOGE("SPDIF :: input data length is too many to copy !!");
					}
					break;
				case DDP_TO_DD_NEED_MORE_DATA:
					inputbuffer->nFilledLen = 0;
					pPrivate->isNewBuffer = 1;
					return;
					break;
				case DDP_TO_DD_ERROR:
					inputbuffer->nFilledLen = 0;
					pPrivate->isNewBuffer = 1;
					ALOGE("DDP-DD :: Error !!");
					break;
				default:
					ALOGD("Default");
					break;
				}
			}
			else
			{
				if ((pPrivate->spdif_nFilledLength + inputbuffer->nFilledLen) <= SPDIF_BUFFER_SIZE)
				{
					//ALOGD("not ddp to dd mode");
					memcpy(&pPrivate->spdif_pBuffer[pPrivate->spdif_nFilledLength], input_ptr, inputbuffer->nFilledLen);
					pPrivate->spdif_nFilledLength += inputbuffer->nFilledLen;
				}
				else
				{
					ALOGE("DDP-DD :: input data length is too many to copy !!");
				}
			}
			#else
			if ((pPrivate->spdif_nFilledLength + inputbuffer->nFilledLen) <= SPDIF_BUFFER_SIZE)
			{
				memcpy(&pPrivate->spdif_pBuffer[pPrivate->spdif_nFilledLength], input_ptr, inputbuffer->nFilledLen);
				pPrivate->spdif_nFilledLength += inputbuffer->nFilledLen;
			}
			else
			{
				ALOGE("SPDIF :: input data length is too many to copy !!");
			}
			#endif	
			
			pPrivate->isNewBuffer = 0;
		}

		pucInputBuf = &pPrivate->spdif_pBuffer[pPrivate->spdif_nConsumedLength];
		nInputSize = pPrivate->spdif_nFilledLength - pPrivate->spdif_nConsumedLength;
		iDataOffset = 0;

		while (iDataOffset < nInputSize)
		{
			if (pPrivate->pAudioDecPrivate[uiDecoderID].audio_coding_type == OMX_AUDIO_CodingAC3)
			{
				if (pPrivate->pAudioDecPrivate[uiDecoderID].iAdecType == AUDIO_ID_AC3 && pPrivate->pSPDIFMode == SPDIF_MODE_HD)
				{
					found_header = 1;
				}
				else if (pPrivate->pAudioDecPrivate[uiDecoderID].iAdecType == AUDIO_ID_DDP && pPrivate->pSPDIFMode == SPDIF_MODE_HD)
				{
					found_header = 1;
				}
				else
					found_header = ac3_header_parse(pucInputBuf, &mSpdif_info);
			}
			else if (pPrivate->pAudioDecPrivate[uiDecoderID].audio_coding_type == OMX_AUDIO_CodingDTS)
			{
				if(pPrivate->pSPDIFMode == SPDIF_MODE_HD)
				{
					found_header = 1;
				}
				else
					found_header = dts_header_parse(pucInputBuf, &mSpdif_info, 0, nInputSize - iDataOffset);
			}

			if (found_header == 1)
			{
				DBUG_MSG("found frame size in header = %d", mSpdif_info.frame_size);
				iNextHeaderOffset = mSpdif_info.frame_size;
				break;
			}
			else if (found_header == 4)
			{
				// need extend data
				DBUG_MSG("SPDIF :: need extend data !!");
				inputbuffer->nFilledLen = 0;
				pPrivate->isNewBuffer = 1;
				return;
			}
			else
			{
				pucInputBuf++;
				iDataOffset++;
			}
		}

		pPrivate->spdif_nConsumedLength += iDataOffset;

		// DDP HD passthru Start
		if((pPrivate->pAudioDecPrivate[uiDecoderID].audio_coding_type == OMX_AUDIO_CodingAC3)
			&& (pPrivate->pSPDIFMode == SPDIF_MODE_HD)
			&& (pPrivate->pAudioDecPrivate[uiDecoderID].iAdecType == AUDIO_ID_DDP))
		{
			static const uint8_t eac3_repeat[4] = {6, 3, 2, 1};
			int DDP_repeat = 0;
			int checkedFrameLen= 0;
			int checkLen = 0;
			int count=0;

			for(count = 0; count<inputbuffer->nFilledLen-4; count++)
			{
				if(input_ptr[count] == 0x0B && input_ptr[count+1] == 0x77)
				{
					if((input_ptr[count+4] & 0xc0) != 0xc0)
						DDP_repeat = eac3_repeat[(input_ptr[count+4] & 0x30) >> 4];
					break;
				}
			}

			DBUG_MSG("Start DDP_buffer_filled = %d repeat %d input len %d nInputSize %d count %d inputbuffer[] %x pucInputBuf[] %x",
			DDP_buffer_filled,DDP_repeat,inputbuffer->nFilledLen,nInputSize,count,input_ptr[count+4],pucInputBuf[count+4]);

			if(count ==0 && DDP_remaining_length > 0)
			{
				DBUG_MSG("DDP_remaining_length >0 but we found syncword , so DDP_remaining_length is not needed %d ",DDP_remaining_length);
				DDP_remaining_length = -1;
			}

			if(DDP_remaining_length == count)
			{
				DBUG_MSG("fill previous frame with DDP_remaining_length %d ",DDP_remaining_length);
				memcpy(&TRUEHD_MAT_Buffer[DDP_buffer_filled], pucInputBuf,count);
				pPrivate->spdif_nConsumedLength += count;
				DDP_buffer_filled += count;
				DDP_remaining_length =-1;
			}
			else
			{
				checkLen = nInputSize;

				if(DDP_repeat > 0){
					//checkedFrameLen = parse_ddp_frame(pucInputBuf,inputbuffer->nFilledLen);
					checkedFrameLen = parse_ddp_frame(pucInputBuf,nInputSize);

					// (checkedFrameLen < nInputSize) means that parse_ddp_frame() found next payload
					if(checkedFrameLen < nInputSize){
						checkLen = checkedFrameLen;
					}
				}

				if((DDP_buffer_filled+checkLen) > 61440)
				{
					ALOGE("Check DDP size DDP_buffer_filledl %d checkLen %d ",DDP_buffer_filled,checkLen);
					iNextHeaderOffset = 0;
					DDP_buffer_filled = 0;
					inputbuffer->nFilledLen = 0;
					pPrivate->isNewBuffer = 1;
					return ;
				}
				else
				{
					memcpy(&TRUEHD_MAT_Buffer[DDP_buffer_filled],pucInputBuf,checkLen);
				}

				pPrivate->spdif_nConsumedLength += checkLen;
				DDP_buffer_filled += checkLen;

				if (pPrivate->spdif_nConsumedLength >= pPrivate->spdif_nFilledLength)
				{
					DBUG_MSG("isNewBuffer @@ ");
					inputbuffer->nFilledLen = 0;
					pPrivate->isNewBuffer = 1;
				}

				if(++DDP_Count < DDP_repeat)
				{
 					iNextHeaderOffset = 0;
					DBUG_MSG("More Data DDP_Count = %d repeat %d DDP_buffer_filled %d ", DDP_Count,DDP_repeat,DDP_buffer_filled);
 					return ;
				}
			}

			if(DDP_remaining_length>0)
			{
				iNextHeaderOffset = 0;
				DBUG_MSG("More Data DDP_remaining_length = %d DDP_buffer_filled=%d", DDP_remaining_length,DDP_buffer_filled);
				return ;
			}

			iNextHeaderOffset = 24576;
			DDP_Count  = 0;

			DBUG_MSG("End DDP_buffer_filled = %d spdif_nFilledLength %d consumed %d ", DDP_buffer_filled, pPrivate->spdif_nFilledLength,pPrivate->spdif_nConsumedLength);
		}
		// DDP HD passthru End

		if (iNextHeaderOffset)
		{

			#if 0	// Test code	SPDIF_AC3_TRUEHD
			// Test Code Start
			FILE *dump_fp = NULL;

			dump_fp = fopen("/storage/usb0/dump_spdif_out_dvbt.data", "a+");
			if(dump_fp != NULL){
				fwrite(TRUEHD_MAT_Buffer, 61440, 1, dump_fp);
				fclose(dump_fp);
			}
			else {
				ALOGW("[Error] Don't write to /data/dump_out.pcm");
			}
			//memset(TRUEHD_MAT_Buffer,0,61440);
			//ALOGE("~~~~~~~~~!!!!!!!!!!!! Code End & SAVE & TRUEHD count %d !!!!!!!!!!!~~~~~~~~~~~~~~~",TRUEHD_Count);
			//return;
			#endif	// Test Code End

			if (((pPrivate->spdif_nConsumedLength + iNextHeaderOffset) <= pPrivate->spdif_nFilledLength)
				|| (pPrivate->pAudioDecPrivate[uiDecoderID].iAdecType == AUDIO_ID_DDP)
				&& (pPrivate->pSPDIFMode == SPDIF_MODE_HD))
			{
				SPDIF_CODEC_TYPE type;

				if(pPrivate->pAudioDecPrivate[uiDecoderID].audio_coding_type == OMX_AUDIO_CodingAC3)
				{
					if(pPrivate->pAudioDecPrivate[uiDecoderID].iAdecType == AUDIO_ID_DDP && pPrivate->pSPDIFMode == SPDIF_MODE_HD)
					{
						type = FORMAT_AC3_DDP;
					}					
					else
					{
						type = FORMAT_AC3;
					}
				}		
				else if (pPrivate->pAudioDecPrivate[uiDecoderID].audio_coding_type == OMX_AUDIO_CodingDTS)
				{
					type = FORMAT_DTS;
				}

				if(pPrivate->pSPDIFMode == SPDIF_MODE_HD && pPrivate->pAudioDecPrivate[uiDecoderID].iAdecType == AUDIO_ID_DDP) //DDP to DD
				{				
					if (spdif_parser_frame(TRUEHD_MAT_Buffer,DDP_buffer_filled, type, 0, 1))
					{
						int spdif_frame_size = (int)spdif_parse_get_frame_size();
						DBUG_MSG("spdif_frame_size %d", spdif_frame_size);

						if (spdif_frame_size > 0)
						{
							memcpy(outputbuffer->pBuffer, spdif_parse_get_buf(), spdif_frame_size);
							outputbuffer->nFilledLen = spdif_frame_size;

							if (pPrivate->pAudioDecPrivate[uiDecoderID].iSamples == 0) // audio data is not started with the first chunk
							{
								pPrivate->pAudioDecPrivate[uiDecoderID].iStartTS = inputbuffer->nTimeStamp;
							}
							outputbuffer->nTimeStamp = pPrivate->pAudioDecPrivate[uiDecoderID].iStartTS + pPrivate->pAudioDecPrivate[uiDecoderID].iSamples;
						}
						else
						{
							ALOGE("SPDIF :: spdif frame size is 0 !!");
						}
					}

					memset(TRUEHD_MAT_Buffer,0,iNextHeaderOffset);
					iNextHeaderOffset = DDP_buffer_filled;
					DDP_buffer_filled=0;
				}
				else
				{
					if (spdif_parser_frame(&pPrivate->spdif_pBuffer[pPrivate->spdif_nConsumedLength], iNextHeaderOffset, type, 0, 0)) //AC3
					{
						int spdif_frame_size = (int)spdif_parse_get_frame_size();
						DBUG_MSG("spdif_frame_size %d", spdif_frame_size);

						if (spdif_frame_size > 0)
						{
							memcpy(outputbuffer->pBuffer, spdif_parse_get_buf(), spdif_frame_size);
							outputbuffer->nFilledLen = spdif_frame_size;

							if (pPrivate->pAudioDecPrivate[uiDecoderID].iSamples == 0) // audio data is not started with the first chunk
							{
								pPrivate->pAudioDecPrivate[uiDecoderID].iStartTS = inputbuffer->nTimeStamp;
							}

							//outputbuffer->nTimeStamp = pPrivate->pAudioDecPrivate[uiDecoderID].iStartTS + pPrivate->pAudioDecPrivate[uiDecoderID].iSamples;
							outputbuffer->nTimeStamp = inputbuffer->nTimeStamp;

							// in case of SPDIF passthrough mode, iSamples equals duration of generated frames 
							if (pPrivate->pAudioDecPrivate[uiDecoderID].audio_coding_type == OMX_AUDIO_CodingAC3)
							{
								switch (pPrivate->pAudioPcmMode[uiDecoderID].nSamplingRate)
								{
									case 32000:
										pPrivate->pAudioDecPrivate[uiDecoderID].iSamples += 48000; // 1536/32 = 48
										break;
									case 44100:
										pPrivate->pAudioDecPrivate[uiDecoderID].iSamples += 34830; // 1536/44.1
										break;
									case 48000:
										pPrivate->pAudioDecPrivate[uiDecoderID].iSamples += 32000; // 1536/48 = 32
										break;
									default:
										break;
								}
							}
							else if (pPrivate->pAudioDecPrivate[uiDecoderID].audio_coding_type == OMX_AUDIO_CodingDTS)
							{
								// duration = total bits in a frame / bitrate
								// for 768kbps, total bits = 1006 * 8
								// so duration is 1006 * 8 / 754500 = 10.666666... ms
								pPrivate->pAudioDecPrivate[uiDecoderID].iSamples += 10667; // 960 ticks / 90 = 10.666666... ms for 768kbps 
							}
						}
						else
						{
							ALOGE("SPDIF :: spdif frame size is 0 !!");
						}
					}
					else
					{
						ALOGE("SPDIF :: spdif parse error !!");
					}
				}

				pPrivate->spdif_nConsumedLength += iNextHeaderOffset;
				if (pPrivate->spdif_nConsumedLength >= pPrivate->spdif_nFilledLength)
				{
					// need more data
					inputbuffer->nFilledLen = 0;
					pPrivate->isNewBuffer = 1;
				}
			}
			else
			{
				// need more data
				inputbuffer->nFilledLen = 0;
				pPrivate->isNewBuffer = 1;
			}
		}
		else
		{
			DBUG_MSG("SPDIF :: sync header search failed !!");

			inputbuffer->nFilledLen = 0;
			pPrivate->isNewBuffer = 1;
		}

		//PrintHexData(outputbuffer->pBuffer);
	}

#ifdef PCM_INFO_SIZE
	if(outputbuffer->nFilledLen > 0) {
		memcpy(outputbuffer->pBuffer+outputbuffer->nFilledLen, &pPrivate->pAudioPcmMode[uiDecoderID].nSamplingRate, 4);
		outputbuffer->nFilledLen += 4;
		memcpy(outputbuffer->pBuffer+outputbuffer->nFilledLen, &pPrivate->pAudioPcmMode[uiDecoderID].nChannels, 4);
		outputbuffer->nFilledLen += 4;
		memcpy(outputbuffer->pBuffer+outputbuffer->nFilledLen, &pPrivate->iSPDIFMode, 4);
		outputbuffer->nFilledLen += 4;
	}
#endif
}

OMX_ERRORTYPE omx_audiodec_component_Change_SPDIF(OMX_COMPONENTTYPE *openmaxStandComp, OMX_STRING cComponentName, OMX_U32 uiDecoderID)
{
	if(!strcmp(cComponentName, AUDIO_DEC_AC3_NAME)) {
		((omx_ac3dec_component_PrivateType*)(openmaxStandComp->pComponentPrivate))->pAudioAc3.nSamplingRate = 48000;
		((omx_ac3dec_component_PrivateType*)(openmaxStandComp->pComponentPrivate))->pAudioPcmMode[uiDecoderID].nSamplingRate = 48000;
	}
	return OMX_ErrorNone;
}
