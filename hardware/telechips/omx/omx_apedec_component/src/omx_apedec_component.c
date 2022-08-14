/**

  @file omx_apedec_component.c

  This component implement APE decoder.

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

#include <omx_apedec_component.h>
#include <tccaudio.h>
#include <OMX_TCC_Index.h>
#include "TCCFile.h"
#include "TCCMemory.h"

#ifdef HAVE_ANDROID_OS
#define USE_EXTERNAL_BUFFER 0
#define LOG_TAG	"OMX_TCC_APEDEC"
#include <utils/Log.h>

static int DEBUG_ON	= 0;
#define DBUG_MSG(msg...)	if (DEBUG_ON) { ALOGD( ": " msg);}
#endif /* HAVE_ANDROID_OS */

#ifdef HAVE_ANDROID_OS
OMX_ERRORTYPE OMX_ComponentInit(OMX_HANDLETYPE openmaxStandComp, OMX_STRING cCompontName, OMX_HANDLE_FUNC pHandleFunc)
{
  OMX_ERRORTYPE err = OMX_ErrorNone;

	err = omx_apedec_component_Constructor(openmaxStandComp,cCompontName,pHandleFunc);
	omx_audiodec_component_Init(openmaxStandComp);	// Pass-Through 2ch Audio Playback Planet 20121204

	return err;
}
#endif

OMX_ERRORTYPE omx_apedec_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp, OMX_STRING cComponentName, OMX_HANDLE_FUNC pHandleFunc) 
{

	OMX_ERRORTYPE err = OMX_ErrorNone;  
	omx_apedec_component_PrivateType* omx_apedec_component_Private;
	omx_base_audio_PortType *inPort,*outPort;
	OMX_U32 i;

#ifdef HAVE_ANDROID_OS
	if (1)
#else
	if (!openmaxStandComp->pComponentPrivate) 
#endif
	{
		openmaxStandComp->pComponentPrivate = TCC_calloc(1, sizeof(omx_apedec_component_PrivateType));

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

	omx_apedec_component_Private = openmaxStandComp->pComponentPrivate;
	omx_apedec_component_Private->ports = NULL;

	/** we could create our own port structures here
	* fixme maybe the base class could use a "port factory" function pointer?  
	*/
	err = omx_base_filter_Constructor(openmaxStandComp, cComponentName);

	DBUG_MSG("constructor of APE decoder component is called\n");

	/* Domain specific section for the ports. */  
	/* first we set the parameter common to both formats */
	/* parameters related to input port which does not depend upon input audio format    */
	/* Allocate Ports and call port constructor. */  

	omx_apedec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nStartPortNumber = 0;
  	omx_apedec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts = 2;

	if (omx_apedec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts && !omx_apedec_component_Private->ports) 
	{
	    omx_apedec_component_Private->ports = TCC_calloc(omx_apedec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts, sizeof(omx_base_PortType *));
	    if (!omx_apedec_component_Private->ports) 
	    {
	  	    return OMX_ErrorInsufficientResources;
	    }
	    for (i=0; i < omx_apedec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) 
	    {
		      omx_apedec_component_Private->ports[i] = TCC_calloc(1, sizeof(omx_base_audio_PortType));
		      if (!omx_apedec_component_Private->ports[i]) 
		      {
		        	return OMX_ErrorInsufficientResources;
		      }
	    }
	}

	base_audio_port_Constructor(openmaxStandComp, &omx_apedec_component_Private->ports[0], 0, OMX_TRUE); // input
	base_audio_port_Constructor(openmaxStandComp, &omx_apedec_component_Private->ports[1], 1, OMX_FALSE); // output

	/** parameters related to input port */
	inPort = (omx_base_audio_PortType *) omx_apedec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
	  
	inPort->sPortParam.nBufferSize = DEFAULT_IN_BUFFER_SIZE*2;
#ifndef OPENMAX1_2
	strcpy(inPort->sPortParam.format.audio.cMIMEType, "audio/x-ape");
#endif
	inPort->sPortParam.format.audio.eEncoding = OMX_AUDIO_CodingAPE;
	inPort->sAudioParam.eEncoding = OMX_AUDIO_CodingAPE;
	
	/** parameters related to output port */
	outPort = (omx_base_audio_PortType *) omx_apedec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
	outPort->sPortParam.format.audio.eEncoding = OMX_AUDIO_CodingPCM;
	outPort->sPortParam.nBufferSize = AUDIO_DEC_OUT_BUFFER_SIZE;
	outPort->sAudioParam.eEncoding = OMX_AUDIO_CodingPCM;

    //Default values for AAC audio param port
	setHeader(&omx_apedec_component_Private->pAudioApe, sizeof(OMX_AUDIO_PARAM_APETYPE));
    omx_apedec_component_Private->pAudioApe.nPortIndex = 0;
    omx_apedec_component_Private->pAudioApe.nChannels = 2;
    omx_apedec_component_Private->pAudioApe.nBitRate = 0;
    omx_apedec_component_Private->pAudioApe.nSampleRate = 44100;
    omx_apedec_component_Private->pAudioApe.eChannelMode = OMX_AUDIO_ChannelModeStereo;

	/** settings of output port audio format - pcm */
	setHeader(&omx_apedec_component_Private->pAudioPcmMode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
	omx_apedec_component_Private->pAudioPcmMode.nPortIndex = 1;
	omx_apedec_component_Private->pAudioPcmMode.nChannels = 2;
	omx_apedec_component_Private->pAudioPcmMode.eNumData = OMX_NumericalDataSigned;
	omx_apedec_component_Private->pAudioPcmMode.eEndian = OMX_EndianLittle;
	omx_apedec_component_Private->pAudioPcmMode.bInterleaved = OMX_TRUE;
	omx_apedec_component_Private->pAudioPcmMode.nBitPerSample = 16;
	omx_apedec_component_Private->pAudioPcmMode.nSamplingRate = 44100;
	omx_apedec_component_Private->pAudioPcmMode.ePCMMode = OMX_AUDIO_PCMModeLinear;
	omx_apedec_component_Private->pAudioPcmMode.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
	omx_apedec_component_Private->pAudioPcmMode.eChannelMapping[1] = OMX_AUDIO_ChannelRF;

	/** now it's time to know the audio coding type of the component */
	if(!strcmp(cComponentName, AUDIO_DEC_APE_NAME))  
	{   
		 omx_apedec_component_Private->audio_coding_type = OMX_AUDIO_CodingAPE;
	} 
	else if (!strcmp(cComponentName, AUDIO_DEC_BASE_NAME)) 
	{
		omx_apedec_component_Private->audio_coding_type = OMX_AUDIO_CodingUnused;
	}
	else  
	{
	    // IL client specified an invalid component name
	    
		ALOGE("OMX_ErrorInvalidComponentName %s", cComponentName);
	    return OMX_ErrorInvalidComponentName;
	}


	/** general configuration irrespective of any audio formats */
	/**  setting values of other fields of omx_maddec_component_Private structure */
	
	omx_apedec_component_Private->BufferMgmtCallback = omx_audiodec_component_BufferMgmtCallback;
	omx_apedec_component_Private->messageHandler = omx_audiodec_component_MessageHandler;
	omx_apedec_component_Private->destructor = omx_audiodec_component_Destructor;
	openmaxStandComp->SetParameter = omx_audiodec_component_SetParameter;
	openmaxStandComp->GetParameter = omx_audiodec_component_GetParameter;
	openmaxStandComp->GetExtensionIndex = omx_audiodec_component_GetExtensionIndex;

	omx_apedec_component_Private->pExternalDec = pHandleFunc;
	omx_apedec_component_Private->OpenCodec = OpenAPEDec;
	omx_apedec_component_Private->DecodeCodec = DecodeAPE;
	omx_apedec_component_Private->FlushCodec = FlushAPEDec;
	omx_apedec_component_Private->CloseCodec = CloseAPEDec;

	omx_apedec_component_Private->decode_ready = OMX_FALSE;	

	//memset(&omx_apedec_component_Private->cdk_core, 0x00, sizeof(cdk_core_t));
	memset(&omx_apedec_component_Private->cdmx_info, 0x00, sizeof(cdmx_info_t));
//	memset(&omx_apedec_component_Private->cdmx_out, 0x00, sizeof(cdmx_output_t));

	memset(&omx_apedec_component_Private->stStreamInfo, 0x00, sizeof(audio_streaminfo_t));
	memset(&omx_apedec_component_Private->stPcmInfo, 0x00, sizeof(audio_pcminfo_t));
	memset(&omx_apedec_component_Private->stADecInit, 0x00, sizeof(adec_init_t));	

	omx_apedec_component_Private->iAudioProcessMode = 2; /* decoded pcm mode */

	omx_apedec_component_Private->iAdecType = AUDIO_ID_APE;
	omx_apedec_component_Private->iCtype = CONTAINER_TYPE_AUDIO;

	DBUG_MSG("constructor of APE decoder component is completed ret = %d \n", err);

	return err;

}

static OMX_S32 OpenAPEDec(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer)
{
	omx_apedec_component_PrivateType* pPrivate = openmaxStandComp->pComponentPrivate;
	OMX_S32 ret	= 0;

	// setting callback functions
	pPrivate->stADecInit.m_pfMalloc = (void* (*) ( unsigned int ))malloc;
	pPrivate->stADecInit.m_pfRealloc = (void* (*) ( void*, unsigned int ))realloc;
	pPrivate->stADecInit.m_pfFree = (void  (*) ( void* ))free;
	pPrivate->stADecInit.m_pfMemcpy = (void* (*) ( void*, const void*, unsigned int ))memcpy;
	pPrivate->stADecInit.m_pfMemmove = (void* (*) ( void*, const void*, unsigned int ))memmove;
	pPrivate->stADecInit.m_pfMemset = (void  (*) ( void*, int, unsigned int ))memset;
	
	// setting pcm info 
	pPrivate->stPcmInfo.m_uiBitsPerSample = pPrivate->pAudioPcmMode.nBitPerSample;
	pPrivate->stPcmInfo.m_ePcmInterLeaved = pPrivate->pAudioPcmMode.bInterleaved;	// 0 or 1
	pPrivate->stPcmInfo.m_iNchannelOrder[CH_LEFT_FRONT] = 1;	//first channel
	pPrivate->stPcmInfo.m_iNchannelOrder[CH_RIGHT_FRONT] = 2;	//second channel

	pPrivate->iUseCodecSpecificTS = 0;
	pPrivate->iSamples = 0;

	if(pPrivate->iExtractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_TCC)
	{
		// setting stream info
		pPrivate->stStreamInfo.m_eSampleRate = pPrivate->cdmx_info.m_sAudioInfo.m_iSamplePerSec;
		pPrivate->stStreamInfo.m_uiNumberOfChannel = pPrivate->cdmx_info.m_sAudioInfo.m_iChannels;
		pPrivate->stStreamInfo.m_uiBitsPerSample = pPrivate->cdmx_info.m_sAudioInfo.m_iBitsPerSample;

		// setting extra info
		pPrivate->stADecInit.m_pucExtraData = pPrivate->cdmx_info.m_sAudioInfo.m_pExtraData;
		pPrivate->stADecInit.m_iExtraDataLen = pPrivate->cdmx_info.m_sAudioInfo.m_iExtraDataLen;

		if(pPrivate->iCtype == CONTAINER_TYPE_AUDIO)
			pPrivate->iUseCodecSpecificTS = 1;
			
		pPrivate->iMinStreamSize = AUDIO_MAX_INPUT_SIZE;
		if((pPrivate->cdmx_info.m_sAudioInfo.m_iMinDataSize != 0) && (pPrivate->iCtype == CONTAINER_TYPE_AUDIO))
		{
			pPrivate->iMinStreamSize = pPrivate->cdmx_info.m_sAudioInfo.m_iMinDataSize;
		}
	}
	else
	{
		// setting stream info
		pPrivate->stStreamInfo.m_eSampleRate = pPrivate->pAudioApe.nSampleRate;
		pPrivate->stStreamInfo.m_uiNumberOfChannel = pPrivate->pAudioApe.nChannels;

		// setting extra info
		pPrivate->stADecInit.m_pucExtraData = inputbuffer->pBuffer;
		pPrivate->stADecInit.m_iExtraDataLen = inputbuffer->nFilledLen;
	}

	pPrivate->stADecInit.m_psAudiodecInput = &pPrivate->stStreamInfo;
	pPrivate->stADecInit.m_psAudiodecOutput = &pPrivate->stPcmInfo;

	ret = pPrivate->pExternalDec(AUDIO_INIT, &pPrivate->iDecoderHandle, &pPrivate->stADecInit, NULL);
	
	DBUG_MSG("%s result %d\n",__func__, ret);

	return ret;

}

static OMX_S32 DecodeAPE(OMX_COMPONENTTYPE* openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer, OMX_BUFFERHEADERTYPE* outputbuffer)
{
	OMX_S32 iOutFrameSize, iRet = TCAS_SUCCESS;
	omx_apedec_component_PrivateType* pPrivate = openmaxStandComp->pComponentPrivate;

	iOutFrameSize = 0;
	pPrivate->iDecodedSamplePerCh = 0;

	//pPrivate->stStreamInfo.m_pvExtraInfo = pPrivate->pvCodecSpecific;

	DBUG_MSG("%s: input length %d", __func__, pPrivate->stStreamInfo.m_iStreamLength);

	while(pPrivate->stStreamInfo.m_iStreamLength > pPrivate->iMinStreamSize)
	{
		pPrivate->stPcmInfo.m_pvChannel[0]  = (void *)(outputbuffer->pBuffer + iOutFrameSize);

		iRet = pPrivate->pExternalDec(AUDIO_DECODE, &pPrivate->iDecoderHandle, &pPrivate->stStreamInfo, &pPrivate->stPcmInfo);
		
		if(iRet == TCAS_SUCCESS)
		{
			iOutFrameSize += pPrivate->stPcmInfo.m_uiNumberOfChannel * pPrivate->stPcmInfo.m_uiSamplesPerChannel * sizeof(short);
			pPrivate->iDecodedSamplePerCh += pPrivate->stPcmInfo.m_uiSamplesPerChannel;
			break;
		}
		else
		{
			if(iRet != TCAS_ERROR_SKIP_FRAME)
				break;
		}
	}

	if(pPrivate->iCtype == CONTAINER_TYPE_AUDIO)
	{
		outputbuffer->nTimeStamp = pPrivate->iStartTS + ((OMX_TICKS)pPrivate->iSamples * 1000000 + (pPrivate->pAudioPcmMode.nSamplingRate >> 1)) / pPrivate->pAudioPcmMode.nSamplingRate;
		pPrivate->iSamples += (OMX_TICKS)pPrivate->iDecodedSamplePerCh;
	}
	
	return iRet;
}

#define CODEC_SPECIFIC_MARKER 0x12345678
#define CODEC_SPECIFIC_MARKER_OFFSET 8
#define CODEC_SPECIFIC_INFO_OFFSET 4

typedef struct ape_dmx_exinput_t
{
	unsigned int m_uiInputBufferByteOffset;
	unsigned int m_uiCurrentFrames;
} ape_dmx_exinput_t;

static OMX_S32 FlushAPEDec(OMX_COMPONENTTYPE* openmaxStandComp,  OMX_BUFFERHEADERTYPE* inputbuffer)
{
	omx_apedec_component_PrivateType* pPrivate = openmaxStandComp->pComponentPrivate;
	ape_dmx_exinput_t *codecSpecific = 0;

	DBUG_MSG("%s\n",__func__);			

	if(inputbuffer)
	{
		OMX_U8* p = inputbuffer->pBuffer;

		if(*((OMX_U32*)(p+inputbuffer->nFilledLen-CODEC_SPECIFIC_MARKER_OFFSET)) == CODEC_SPECIFIC_MARKER)
		{
			ALOGE("codec specific data!");
			codecSpecific = (ape_dmx_exinput_t *)(*((OMX_U32*)(p+inputbuffer->nFilledLen-CODEC_SPECIFIC_INFO_OFFSET)));
			inputbuffer->nFilledLen -= CODEC_SPECIFIC_MARKER_OFFSET;
		}
		else
		{
			ALOGE("No codec specific data");
		}

		pPrivate->pvCodecSpecific = codecSpecific;
		pPrivate->pExternalDec(AUDIO_FLUSH, &pPrivate->iDecoderHandle, NULL, NULL);
	}
	pPrivate->iSamples = 0;

	return 0;
}

static OMX_S32 CloseAPEDec(OMX_COMPONENTTYPE* openmaxStandComp)
{
	omx_apedec_component_PrivateType* pPrivate = openmaxStandComp->pComponentPrivate;

	DBUG_MSG("%s\n",__func__);

	pPrivate->pExternalDec(AUDIO_CLOSE, &pPrivate->iDecoderHandle, NULL, NULL);

	pPrivate->iDecoderHandle = 0;

	return 0;
}

