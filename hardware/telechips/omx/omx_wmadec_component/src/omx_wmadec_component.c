/**

  @file omx_wmadec_component.c

  This component implement WMA decoder.

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

#include <omx_wmadec_component.h>
#include <tccaudio.h>
#include <OMX_TCC_Index.h>
#include "TCCFile.h"
#include "TCCMemory.h"

#ifdef HAVE_ANDROID_OS
#define USE_EXTERNAL_BUFFER 0
#define LOG_TAG	"OMX_TCC_WMADEC"
#include <utils/Log.h>

static int DEBUG_ON	= 0;
#define DBUG_MSG(msg...)	if (DEBUG_ON) { ALOGD( ": " msg);}
#endif /* HAVE_ANDROID_OS */

#ifdef HAVE_ANDROID_OS
OMX_ERRORTYPE OMX_ComponentInit(OMX_HANDLETYPE openmaxStandComp, OMX_STRING cCompontName, OMX_HANDLE_FUNC pHandleFunc)
{
  OMX_ERRORTYPE err = OMX_ErrorNone;

	err = omx_wmadec_component_Constructor(openmaxStandComp,cCompontName,pHandleFunc);
	omx_audiodec_component_Init(openmaxStandComp);	// Pass-Through 2ch Audio Playback Planet 20121204

	return err;
}
#endif

OMX_ERRORTYPE omx_wmadec_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp, OMX_STRING cComponentName, OMX_HANDLE_FUNC pHandleFunc) 
{

	OMX_ERRORTYPE err = OMX_ErrorNone;  
	omx_wmadec_component_PrivateType* omx_wmadec_component_Private;
	omx_base_audio_PortType *inPort,*outPort;
	OMX_U32 i;

#ifdef HAVE_ANDROID_OS
	if (1)
#else
	if (!openmaxStandComp->pComponentPrivate) 
#endif
	{
		openmaxStandComp->pComponentPrivate = TCC_calloc(1, sizeof(omx_wmadec_component_PrivateType));

		if(openmaxStandComp->pComponentPrivate==NULL)  
		{
			return OMX_ErrorInsufficientResources;
		}
	} 
	else 
	{
	    DBUG_MSG("In %s, Error Component %x Already Allocated\n", 
	              __func__, (int)openmaxStandComp->pComponentPrivate);
	}

	if(pHandleFunc == NULL)
	{
		return OMX_ErrorBadParameter;
	}

	omx_wmadec_component_Private = openmaxStandComp->pComponentPrivate;
	omx_wmadec_component_Private->ports = NULL;

	/** we could create our own port structures here
	* fixme maybe the base class could use a "port factory" function pointer?  
	*/
	err = omx_base_filter_Constructor(openmaxStandComp, cComponentName);

	DBUG_MSG("constructor of WMA decoder component is called\n");

	/* Domain specific section for the ports. */  
	/* first we set the parameter common to both formats */
	/* parameters related to input port which does not depend upon input audio format    */
	/* Allocate Ports and call port constructor. */  

	omx_wmadec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nStartPortNumber = 0;
  	omx_wmadec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts = 2;

	if (omx_wmadec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts && !omx_wmadec_component_Private->ports) 
	{
	    omx_wmadec_component_Private->ports = TCC_calloc(omx_wmadec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts, sizeof(omx_base_PortType *));
	    if (!omx_wmadec_component_Private->ports) 
	    {
	  	    return OMX_ErrorInsufficientResources;
	    }
	    for (i=0; i < omx_wmadec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) 
	    {
		      omx_wmadec_component_Private->ports[i] = TCC_calloc(1, sizeof(omx_base_audio_PortType));
		      if (!omx_wmadec_component_Private->ports[i]) 
		      {
		        	return OMX_ErrorInsufficientResources;
		      }
	    }
	}

	base_audio_port_Constructor(openmaxStandComp, &omx_wmadec_component_Private->ports[0], 0, OMX_TRUE); // input
	base_audio_port_Constructor(openmaxStandComp, &omx_wmadec_component_Private->ports[1], 1, OMX_FALSE); // output

	/** parameters related to input port */
	inPort = (omx_base_audio_PortType *) omx_wmadec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
	  
	inPort->sPortParam.nBufferSize = DEFAULT_IN_BUFFER_SIZE*2;
#ifndef OPENMAX1_2
	strcpy(inPort->sPortParam.format.audio.cMIMEType, "audio/wma");
#endif
	inPort->sPortParam.format.audio.eEncoding = OMX_AUDIO_CodingWMA;
	inPort->sAudioParam.eEncoding = OMX_AUDIO_CodingWMA;

	/** parameters related to output port */
	outPort = (omx_base_audio_PortType *) omx_wmadec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
	outPort->sPortParam.format.audio.eEncoding = OMX_AUDIO_CodingPCM;
	outPort->sPortParam.nBufferSize = AUDIO_DEC_OUT_BUFFER_SIZE;
	outPort->sAudioParam.eEncoding = OMX_AUDIO_CodingPCM;

    //Default values for WMA audio param port
	setHeader(&omx_wmadec_component_Private->pAudioWma, sizeof(OMX_AUDIO_PARAM_WMATYPE));
    omx_wmadec_component_Private->pAudioWma.nPortIndex = 0;
    omx_wmadec_component_Private->pAudioWma.nChannels = 2;
    omx_wmadec_component_Private->pAudioWma.nBitRate = 0;
    omx_wmadec_component_Private->pAudioWma.nSamplingRate = 44100;
    omx_wmadec_component_Private->pAudioWma.eFormat = OMX_AUDIO_WMAFormat7;
    omx_wmadec_component_Private->pAudioWma.eProfile = OMX_AUDIO_WMAProfileUnused;
    omx_wmadec_component_Private->pAudioWma.nBlockAlign = 0;
    omx_wmadec_component_Private->pAudioWma.nEncodeOptions = 0;
    omx_wmadec_component_Private->pAudioWma.nSuperBlockAlign = 0;

	/** settings of output port audio format - pcm */
	setHeader(&omx_wmadec_component_Private->pAudioPcmMode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
	omx_wmadec_component_Private->pAudioPcmMode.nPortIndex = 1;
	omx_wmadec_component_Private->pAudioPcmMode.nChannels = 2;
	omx_wmadec_component_Private->pAudioPcmMode.eNumData = OMX_NumericalDataSigned;
	omx_wmadec_component_Private->pAudioPcmMode.eEndian = OMX_EndianLittle;
	omx_wmadec_component_Private->pAudioPcmMode.bInterleaved = OMX_TRUE;
	omx_wmadec_component_Private->pAudioPcmMode.nBitPerSample = 16;
	omx_wmadec_component_Private->pAudioPcmMode.nSamplingRate = 44100;
	omx_wmadec_component_Private->pAudioPcmMode.ePCMMode = OMX_AUDIO_PCMModeLinear;
	omx_wmadec_component_Private->pAudioPcmMode.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
	omx_wmadec_component_Private->pAudioPcmMode.eChannelMapping[1] = OMX_AUDIO_ChannelRF;

	/** now it's time to know the audio coding type of the component */
	if(!strcmp(cComponentName, AUDIO_DEC_WMA_NAME))  
	{   
		 omx_wmadec_component_Private->audio_coding_type = OMX_AUDIO_CodingWMA;
	} 
	else if (!strcmp(cComponentName, AUDIO_DEC_BASE_NAME)) 
	{
		omx_wmadec_component_Private->audio_coding_type = OMX_AUDIO_CodingUnused;
	}
	else  
	{
	    // IL client specified an invalid component name
	    
		ALOGE("OMX_ErrorInvalidComponentName %s", cComponentName);
	    return OMX_ErrorInvalidComponentName;
	}


	/** general configuration irrespective of any audio formats */
	/**  setting values of other fields of omx_maddec_component_Private structure */
	
	omx_wmadec_component_Private->BufferMgmtCallback = omx_audiodec_component_BufferMgmtCallback;
	omx_wmadec_component_Private->messageHandler = omx_audiodec_component_MessageHandler;
	omx_wmadec_component_Private->destructor = omx_audiodec_component_Destructor;
	openmaxStandComp->SetParameter = omx_wmadec_component_SetParameter;
	openmaxStandComp->GetParameter = omx_wmadec_component_GetParameter;
	openmaxStandComp->GetExtensionIndex = omx_audiodec_component_GetExtensionIndex;

	omx_wmadec_component_Private->pExternalDec = pHandleFunc;
	omx_wmadec_component_Private->OpenCodec = OpenWMADec;
	omx_wmadec_component_Private->DecodeCodec = DecodeWMA;
	omx_wmadec_component_Private->FlushCodec = FlushWMADec;
	omx_wmadec_component_Private->CloseCodec = CloseWMADec;
	
	omx_wmadec_component_Private->decode_ready = OMX_FALSE;	
	
//	memset(&omx_wmadec_component_Private->cdk_core, 0x00, sizeof(cdk_core_t));
	memset(&omx_wmadec_component_Private->cdmx_info, 0x00, sizeof(cdmx_info_t));
//	memset(&omx_wmadec_component_Private->cdmx_out, 0x00, sizeof(cdmx_output_t));

	memset(&omx_wmadec_component_Private->stStreamInfo, 0x00, sizeof(audio_streaminfo_t));
	memset(&omx_wmadec_component_Private->stPcmInfo, 0x00, sizeof(audio_pcminfo_t));
	memset(&omx_wmadec_component_Private->stADecInit, 0x00, sizeof(adec_init_t));
	
	omx_wmadec_component_Private->iAudioProcessMode = 2; /* decoded pcm mode */
  
	omx_wmadec_component_Private->iAdecType = AUDIO_ID_WMA;
	omx_wmadec_component_Private->iCtype = CONTAINER_TYPE_ASF;

	DBUG_MSG("constructor of WMA decoder component is completed ret = %d \n", err);

	return err;

}

/** this function sets the parameter values regarding audio format & index */
OMX_ERRORTYPE omx_wmadec_component_SetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_IN  OMX_PTR ComponentParameterStructure)
{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	/* Check which structure we are being fed and make control its header */
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_wmadec_component_PrivateType* WMAPrivate = openmaxStandComp->pComponentPrivate;
	
	if (ComponentParameterStructure == NULL) 
	{
		return OMX_ErrorBadParameter;
	}

	DBUG_MSG("WMA Dec Setting parameter 0x%x\n", nParamIndex);

	switch(nParamIndex) 
	{
		case OMX_IndexParamStandardComponentRole:
		{
			OMX_PARAM_COMPONENTROLETYPE * pComponentRole = (OMX_PARAM_COMPONENTROLETYPE*)ComponentParameterStructure;			
			if (strcmp( (char*) pComponentRole->cRole, AUDIO_DEC_TCC_WMA_ROLE)) 
			{
				return OMX_ErrorBadParameter;
			}  
			else
			{
				WMAPrivate->audio_coding_type = OMX_AUDIO_CodingWMA;
			}
			return OMX_ErrorNone;
		}
		
		case OMX_IndexParamAudioWma:
		{
			OMX_AUDIO_PARAM_WMATYPE * pAudioWma = (OMX_AUDIO_PARAM_WMATYPE*) ComponentParameterStructure;
			err = omx_base_component_ParameterSanityCheck(hComponent,pAudioWma->nPortIndex,pAudioWma,sizeof(OMX_AUDIO_PARAM_WMATYPE));
			
			if(err!=OMX_ErrorNone)
			{ 
				DBUG_MSG("In %s Parameter Check Error=%x\n",__func__,err); 
				return err;
			}
			if (pAudioWma->nPortIndex == 0) 
			{
				OMX_AUDIO_PARAM_PCMMODETYPE *pcmParams = &WMAPrivate->pAudioPcmMode;

				memcpy(&WMAPrivate->pAudioWma, pAudioWma, sizeof(OMX_AUDIO_PARAM_WMATYPE));

				if(pAudioWma->nSamplingRate > 48000)	//Professional or Lossless 
				{
					pcmParams->nSamplingRate = pAudioWma->nSamplingRate/2;
				}
				else
				{
					pcmParams->nSamplingRate = pAudioWma->nSamplingRate;
				}

				if (pAudioWma->nChannels == 1) 
				{
					pcmParams->nChannels = 1;
				}				
			}  
			else
			{
				return OMX_ErrorBadPortIndex;
			}			
		}
		
		default: /*Call the common setparam function*/
			return omx_audiodec_component_SetParameter(hComponent, nParamIndex, ComponentParameterStructure);

	}

}  

/** this function gets the parameters regarding audio formats and index */
OMX_ERRORTYPE omx_wmadec_component_GetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_INOUT OMX_PTR ComponentParameterStructure)
{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_wmadec_component_PrivateType* WMAPrivate = openmaxStandComp->pComponentPrivate;

	if (ComponentParameterStructure == NULL)
	{
		return OMX_ErrorBadParameter;
	}
	
	DBUG_MSG("WMA Dec Getting parameter 0x%x\n", nParamIndex);

	switch(nParamIndex) 
	{
		case OMX_IndexParamAudioPcm:
		{
			OMX_AUDIO_PARAM_PCMMODETYPE *pAudioPcmMode = (OMX_AUDIO_PARAM_PCMMODETYPE*)ComponentParameterStructure;
			if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE))) != OMX_ErrorNone) 
			{ 
				return err;
			}
			if (pAudioPcmMode->nPortIndex != 1) 
			{
				return OMX_ErrorBadPortIndex;
			}

			memcpy(pAudioPcmMode, &WMAPrivate->pAudioPcmMode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
			
			return OMX_ErrorNone;
		}
		
		case OMX_IndexParamAudioWma:
		{
			OMX_AUDIO_PARAM_WMATYPE *pAudioWma = (OMX_AUDIO_PARAM_WMATYPE*)ComponentParameterStructure;
			if (pAudioWma->nPortIndex != 0) 
			{
				return OMX_ErrorBadPortIndex;
			}
			if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_WMATYPE))) != OMX_ErrorNone) 
			{ 
				return err;
			}
			memcpy(pAudioWma, &WMAPrivate->pAudioWma, sizeof(OMX_AUDIO_PARAM_WMATYPE));
			return OMX_ErrorNone;
		}

		case OMX_IndexParamStandardComponentRole:
		{
			OMX_PARAM_COMPONENTROLETYPE *pComponentRole = (OMX_PARAM_COMPONENTROLETYPE*)ComponentParameterStructure;
			if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_PARAM_COMPONENTROLETYPE))) != OMX_ErrorNone) 
			{ 
				return err;
			}
			
			strcpy( (char*) pComponentRole->cRole, AUDIO_DEC_TCC_WMA_ROLE);
			return OMX_ErrorNone;
		}
		
		default: /*Call the common GetParameter component function*/
			return omx_audiodec_component_GetParameter(hComponent, nParamIndex, ComponentParameterStructure);

	}

}

static OMX_S32 OpenWMADec(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer)
{
	omx_wmadec_component_PrivateType* pPrivate = openmaxStandComp->pComponentPrivate;
	OMX_S32 ret	= 0;

	// setting callback functions
	pPrivate->stADecInit.m_pfMalloc = (void* (*) ( unsigned int ))malloc;
	pPrivate->stADecInit.m_pfRealloc = (void* (*) ( void*, unsigned int ))realloc;
	pPrivate->stADecInit.m_pfFree = (void  (*) ( void* ))free;
	pPrivate->stADecInit.m_pfMemcpy = (void* (*) ( void*, const void*, unsigned int ))memcpy;
	pPrivate->stADecInit.m_pfMemmove = (void* (*) ( void*, const void*, unsigned int ))memmove;
	pPrivate->stADecInit.m_pfMemset = (void  (*) ( void*, int, unsigned int ))memset;

	// setting output pcm info 
	pPrivate->stPcmInfo.m_uiBitsPerSample = pPrivate->pAudioPcmMode.nBitPerSample;
	pPrivate->stPcmInfo.m_ePcmInterLeaved = pPrivate->pAudioPcmMode.bInterleaved;
	pPrivate->stPcmInfo.m_iNchannelOrder[CH_LEFT_FRONT] = 1;	//first channel
	pPrivate->stPcmInfo.m_iNchannelOrder[CH_RIGHT_FRONT] = 2;	//second channel
	pPrivate->stADecInit.m_iDownMixMode = 1; // if(ch > 2) downmix to stereo

	if(pPrivate->iExtractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_TCC)
	{
		// setting stream info
		pPrivate->stStreamInfo.m_eSampleRate = pPrivate->cdmx_info.m_sAudioInfo.m_iSamplePerSec;
		pPrivate->stStreamInfo.m_uiNumberOfChannel = pPrivate->cdmx_info.m_sAudioInfo.m_iChannels;
		pPrivate->stStreamInfo.m_uiBitsPerSample = pPrivate->cdmx_info.m_sAudioInfo.m_iBitsPerSample;
		// setting extra info
		pPrivate->stADecInit.m_pucExtraData = pPrivate->cdmx_info.m_sAudioInfo.m_pExtraData;
		pPrivate->stADecInit.m_iExtraDataLen = pPrivate->cdmx_info.m_sAudioInfo.m_iExtraDataLen;

		/* WMA Decoder */
		pPrivate->stStreamInfo.m_uiBitRates = pPrivate->cdmx_info.m_sAudioInfo.m_iAvgBytesPerSec<<3;
		pPrivate->stADecInit.m_unAudioCodecParams.m_unWMADec.m_uiNBlockAlign = pPrivate->cdmx_info.m_sAudioInfo.m_iBlockAlign;
		pPrivate->stADecInit.m_unAudioCodecParams.m_unWMADec.m_uiWFormatTag = pPrivate->cdmx_info.m_sAudioInfo.m_iFormatId;
	}
	else
	{
		ALOGE("Not Support!!");
#if 0		
		// setting stream info
		pPrivate->stStreamInfo.m_eSampleRate = pPrivate->pAudioWma.nSamplingRate;
		pPrivate->stStreamInfo.m_uiNumberOfChannel = pPrivate->pAudioWma.nChannels;
		// setting extra info
		pPrivate->stADecInit.m_pucExtraData = inputbuffer->pBuffer;
		pPrivate->stADecInit.m_iExtraDataLen = inputbuffer->nFilledLen;	
#endif		
	}

	pPrivate->stADecInit.m_psAudiodecInput = &pPrivate->stStreamInfo;
	pPrivate->stADecInit.m_psAudiodecOutput = &pPrivate->stPcmInfo;
	pPrivate->iMinStreamSize = 0;
	
	ret = pPrivate->pExternalDec(AUDIO_INIT, &pPrivate->iDecoderHandle, &pPrivate->stADecInit, NULL);

	DBUG_MSG("%s result %d\n",__func__, ret);

	return ret;

}

// decode one frame of wma data
static OMX_S32 DecodeWMA(OMX_COMPONENTTYPE* openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer, OMX_BUFFERHEADERTYPE* outputbuffer)
{
	OMX_S32 iOutFrameSize, iStartBytes, iUsedBytes, iRet = TCAS_SUCCESS;
	omx_wmadec_component_PrivateType* pPrivate = openmaxStandComp->pComponentPrivate;

	iOutFrameSize = 0;
	pPrivate->iDecodedSamplePerCh = 0;

	iStartBytes = pPrivate->stStreamInfo.m_iStreamLength;
	
	DBUG_MSG("%s: input length %d", __func__, pPrivate->stStreamInfo.m_iStreamLength);

	while(pPrivate->stStreamInfo.m_iStreamLength > pPrivate->iMinStreamSize)
	{
		pPrivate->stPcmInfo.m_pvChannel[0]  = (void *)(outputbuffer->pBuffer + iOutFrameSize);
		
		iRet = pPrivate->pExternalDec(AUDIO_DECODE, &pPrivate->iDecoderHandle, &pPrivate->stStreamInfo, &pPrivate->stPcmInfo);

		DBUG_MSG("iRet %d, input length %d", iRet, pPrivate->stStreamInfo.m_iStreamLength);
		
		if(iRet == TCAS_SUCCESS)
		{
			if(pPrivate->stPcmInfo.m_uiSamplesPerChannel)
			{
				DBUG_MSG("ch %d, samples %d", pPrivate->stPcmInfo.m_uiNumberOfChannel, pPrivate->stPcmInfo.m_uiSamplesPerChannel);
				iOutFrameSize += pPrivate->stPcmInfo.m_uiNumberOfChannel * pPrivate->stPcmInfo.m_uiSamplesPerChannel * sizeof(short);
				pPrivate->iDecodedSamplePerCh += pPrivate->stPcmInfo.m_uiSamplesPerChannel;
				iUsedBytes = iStartBytes - pPrivate->stStreamInfo.m_iStreamLength;
				if(iUsedBytes)
				{
					break;
				}
			}
		}
		else
		{
			if(iRet != TCAS_ERROR_SKIP_FRAME)
			{
				if(iRet != TCAS_ERROR_MORE_DATA /*&& iRet != TCAS_ERROR_INVALID_BUFSTATE*/)
				{
					pPrivate->stStreamInfo.m_iStreamLength = 0;
				}
				break;
			}
		}
	}

	//iUsedBytes = iStartBytes - pPrivate->stStreamInfo.m_iStreamLength;
	
	return iRet;
}

static OMX_S32 FlushWMADec(OMX_COMPONENTTYPE* openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer)
{
	omx_wmadec_component_PrivateType* pPrivate = openmaxStandComp->pComponentPrivate;

	DBUG_MSG("%s\n",__func__);
	
	//pPrivate->pExternalDec(AUDIO_FLUSH, &pPrivate->iDecoderHandle, NULL, NULL);

	return 0;
}

static OMX_S32 CloseWMADec(OMX_COMPONENTTYPE* openmaxStandComp)
{
	omx_wmadec_component_PrivateType* pPrivate = openmaxStandComp->pComponentPrivate;

	DBUG_MSG("%s\n",__func__);

	pPrivate->pExternalDec(AUDIO_CLOSE, &pPrivate->iDecoderHandle, NULL, NULL);

	pPrivate->iDecoderHandle = 0;

	return 0;
}


