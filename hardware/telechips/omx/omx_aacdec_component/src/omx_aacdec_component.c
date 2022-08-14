/**

  @file omx_aacdec_component.c

  This component implement AAC decoder.

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

#include <omx_aacdec_component.h>
#include <tccaudio.h>
#include <OMX_TCC_Index.h>
#include "TCCFile.h"
#include "TCCMemory.h"
#include <dlfcn.h>

#ifdef HAVE_ANDROID_OS
#define USE_EXTERNAL_BUFFER 0
#define LOG_TAG	"OMX_TCC_AACDEC"
#include <utils/Log.h>

//#define DEBUG_ON
#ifdef DEBUG_ON
#define DBUG_MSG(msg...)	ALOGD(msg)
#else
#define DBUG_MSG(msg...)
#endif
#endif /* HAVE_ANDROID_OS */

#ifdef HAVE_ANDROID_OS
OMX_ERRORTYPE OMX_ComponentInit(OMX_HANDLETYPE openmaxStandComp, OMX_STRING cCompontName, OMX_HANDLE_FUNC pHandleFunc)
{
  OMX_ERRORTYPE err = OMX_ErrorNone;

	err = omx_aacdec_component_Constructor(openmaxStandComp,cCompontName,pHandleFunc);
	omx_audiodec_component_Init(openmaxStandComp);	// Pass-Through 2ch Audio Playback Planet 20121204

	return err;
}
#endif

OMX_ERRORTYPE omx_aacdec_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp, OMX_STRING cComponentName, OMX_HANDLE_FUNC pHandleFunc) 
{

	OMX_ERRORTYPE err = OMX_ErrorNone;  
	omx_aacdec_component_PrivateType* omx_aacdec_component_Private;
	omx_base_audio_PortType *inPort,*outPort;
	OMX_U32 i;

#ifdef HAVE_ANDROID_OS
	if (1)
#else
	if (!openmaxStandComp->pComponentPrivate) 
#endif
	{
		openmaxStandComp->pComponentPrivate = TCC_calloc(1, sizeof(omx_aacdec_component_PrivateType));

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

	omx_aacdec_component_Private = openmaxStandComp->pComponentPrivate;
	omx_aacdec_component_Private->ports = NULL;

	/** we could create our own port structures here
	* fixme maybe the base class could use a "port factory" function pointer?  
	*/
	err = omx_base_filter_Constructor(openmaxStandComp, cComponentName);

	DBUG_MSG("constructor of AAC decoder component is called\n");

	/* Domain specific section for the ports. */  
	/* first we set the parameter common to both formats */
	/* parameters related to input port which does not depend upon input audio format    */
	/* Allocate Ports and call port constructor. */  

	omx_aacdec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nStartPortNumber = 0;
  	omx_aacdec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts = 2;
	
	if (omx_aacdec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts && !omx_aacdec_component_Private->ports) 
	{
	    omx_aacdec_component_Private->ports = TCC_calloc(omx_aacdec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts, sizeof(omx_base_PortType *));
	    if (!omx_aacdec_component_Private->ports) 
	    {
	  	    return OMX_ErrorInsufficientResources;
	    }
	    for (i=0; i < omx_aacdec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) 
	    {
		      omx_aacdec_component_Private->ports[i] = TCC_calloc(1, sizeof(omx_base_audio_PortType));
		      if (!omx_aacdec_component_Private->ports[i]) 
		      {
		        	return OMX_ErrorInsufficientResources;
		      }
	    }
	}

	base_audio_port_Constructor(openmaxStandComp, &omx_aacdec_component_Private->ports[0], 0, OMX_TRUE); // input
	base_audio_port_Constructor(openmaxStandComp, &omx_aacdec_component_Private->ports[1], 1, OMX_FALSE); // output

	/** parameters related to input port */
	inPort = (omx_base_audio_PortType *) omx_aacdec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
	  
	inPort->sPortParam.nBufferSize = DEFAULT_IN_BUFFER_SIZE*2;
#ifndef OPENMAX1_2
	strcpy(inPort->sPortParam.format.audio.cMIMEType, "audio/aac");
#endif
	inPort->sPortParam.format.audio.eEncoding = OMX_AUDIO_CodingAAC;
	inPort->sAudioParam.eEncoding = OMX_AUDIO_CodingAAC;
	
	/** parameters related to output port */
	outPort = (omx_base_audio_PortType *) omx_aacdec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
	outPort->sPortParam.format.audio.eEncoding = OMX_AUDIO_CodingPCM;
	outPort->sPortParam.nBufferSize = AUDIO_DEC_OUT_BUFFER_SIZE;
	outPort->sAudioParam.eEncoding = OMX_AUDIO_CodingPCM;

    //Default values for AAC audio param port
	setHeader(&omx_aacdec_component_Private->pAudioAac, sizeof(OMX_AUDIO_PARAM_AACPROFILETYPE));
    omx_aacdec_component_Private->pAudioAac.nPortIndex = 0;
    omx_aacdec_component_Private->pAudioAac.nChannels = 2;
    omx_aacdec_component_Private->pAudioAac.nBitRate = 0;
    omx_aacdec_component_Private->pAudioAac.nSampleRate = 44100;
    omx_aacdec_component_Private->pAudioAac.nAudioBandWidth = 0;
    omx_aacdec_component_Private->pAudioAac.nFrameLength = 2048; // use HE_PS frame size as default
    omx_aacdec_component_Private->pAudioAac.eChannelMode = OMX_AUDIO_ChannelModeStereo;
    omx_aacdec_component_Private->pAudioAac.eAACProfile = OMX_AUDIO_AACObjectHE_PS;    //OMX_AUDIO_AACObjectLC;
    omx_aacdec_component_Private->pAudioAac.eAACStreamFormat = OMX_AUDIO_AACStreamFormatRAW;

	/** settings of output port audio format - pcm */
	setHeader(&omx_aacdec_component_Private->pAudioPcmMode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
	omx_aacdec_component_Private->pAudioPcmMode.nPortIndex = 1;
	omx_aacdec_component_Private->pAudioPcmMode.nChannels = 2;
	omx_aacdec_component_Private->pAudioPcmMode.eNumData = OMX_NumericalDataSigned;
	omx_aacdec_component_Private->pAudioPcmMode.eEndian = OMX_EndianLittle;
	omx_aacdec_component_Private->pAudioPcmMode.bInterleaved = OMX_TRUE;
	omx_aacdec_component_Private->pAudioPcmMode.nBitPerSample = 16;
	omx_aacdec_component_Private->pAudioPcmMode.nSamplingRate = 44100;
	omx_aacdec_component_Private->pAudioPcmMode.ePCMMode = OMX_AUDIO_PCMModeLinear;
	omx_aacdec_component_Private->pAudioPcmMode.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
	omx_aacdec_component_Private->pAudioPcmMode.eChannelMapping[1] = OMX_AUDIO_ChannelRF;

	/** now it's time to know the audio coding type of the component */
	if(!strcmp(cComponentName, AUDIO_DEC_AAC_NAME))  
	{   
		 omx_aacdec_component_Private->audio_coding_type = OMX_AUDIO_CodingAAC;
	} 
	else if (!strcmp(cComponentName, AUDIO_DEC_BASE_NAME)) 
	{
		omx_aacdec_component_Private->audio_coding_type = OMX_AUDIO_CodingUnused;
	}
	else  
	{
	    // IL client specified an invalid component name
	    
		ALOGE("OMX_ErrorInvalidComponentName %s", cComponentName);
	    return OMX_ErrorInvalidComponentName;
	}

	/** general configuration irrespective of any audio formats */
	/**  setting values of other fields of omx_maddec_component_Private structure */
	
	omx_aacdec_component_Private->BufferMgmtCallback = omx_audiodec_component_BufferMgmtCallback;
	omx_aacdec_component_Private->messageHandler = omx_audiodec_component_MessageHandler;
	omx_aacdec_component_Private->destructor = omx_audiodec_component_Destructor;
	
	openmaxStandComp->SetParameter = omx_aacdec_component_SetParameter;
	openmaxStandComp->GetParameter = omx_aacdec_component_GetParameter;
	openmaxStandComp->GetExtensionIndex = omx_audiodec_component_GetExtensionIndex;

	omx_aacdec_component_Private->pExternalDec = pHandleFunc;
	omx_aacdec_component_Private->OpenCodec = OpenAACDec;
	omx_aacdec_component_Private->DecodeCodec = DecodeAAC;
	omx_aacdec_component_Private->FlushCodec = FlushAACDec;
	omx_aacdec_component_Private->CloseCodec = CloseAACDec;

	omx_aacdec_component_Private->decode_ready = OMX_FALSE;	
	
	memset(&omx_aacdec_component_Private->cdmx_info, 0x00, sizeof(cdmx_info_t));
	memset(&omx_aacdec_component_Private->stStreamInfo, 0x00, sizeof(audio_streaminfo_t));
	memset(&omx_aacdec_component_Private->stPcmInfo, 0x00, sizeof(audio_pcminfo_t));
	memset(&omx_aacdec_component_Private->stADecInit, 0x00, sizeof(adec_init_t));
	
	omx_aacdec_component_Private->iAudioProcessMode = AUDIO_NORMAL_MODE; /* decoded pcm mode */
	omx_aacdec_component_Private->iAdecType = AUDIO_ID_AAC;

	DBUG_MSG("constructor of AAC decoder component is completed ret = %d \n", err);

	return err;

}

OMX_ERRORTYPE omx_aacdec_component_SetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_IN  OMX_PTR ComponentParameterStructure)
{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_aacdec_component_PrivateType* AACPrivate = openmaxStandComp->pComponentPrivate;
	
	if (ComponentParameterStructure == NULL) 
	{
		return OMX_ErrorBadParameter;
	}

	DBUG_MSG("AAC Dec Setting parameter 0x%x\n", nParamIndex);

	switch(nParamIndex)
	{
		case OMX_IndexParamStandardComponentRole:
		{
			OMX_PARAM_COMPONENTROLETYPE *roleParams = (OMX_PARAM_COMPONENTROLETYPE*)ComponentParameterStructure;
			if(strcmp((char*)roleParams->cRole, AUDIO_DEC_TCC_AAC_ROLE)) 
			{
				return OMX_ErrorBadParameter;			
			}  
			else
			{
				AACPrivate->audio_coding_type = OMX_AUDIO_CodingAAC;
			}
			return OMX_ErrorNone;
		}

		case OMX_IndexParamAudioAac:
		{
			OMX_AUDIO_PARAM_AACPROFILETYPE* aacParams = (OMX_AUDIO_PARAM_AACPROFILETYPE*) ComponentParameterStructure;
			err = omx_base_component_ParameterSanityCheck(hComponent, aacParams->nPortIndex, aacParams, sizeof(OMX_AUDIO_PARAM_AACPROFILETYPE));
			
			if(err!=OMX_ErrorNone)
			{ 
				DBUG_MSG("In %s Parameter Check Error=%x\n",__func__,err); 
				return err;
			}
			if (aacParams->nPortIndex == 0) 
			{
				OMX_AUDIO_PARAM_PCMMODETYPE* pcmParams = &AACPrivate->pAudioPcmMode;

				memcpy(&AACPrivate->pAudioAac, aacParams, sizeof(OMX_AUDIO_PARAM_AACPROFILETYPE));
				
				if (aacParams->nSampleRate < 32000) // SBR signaling
				{
					pcmParams->nSamplingRate = aacParams->nSampleRate * 2;
				} 
				else 
				{
					pcmParams->nSamplingRate = aacParams->nSampleRate;
				}
				
				if(aacParams->nChannels == 1) // PS signaling
				{
					pcmParams->nChannels = 2;
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

OMX_ERRORTYPE omx_aacdec_component_GetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_INOUT OMX_PTR ComponentParameterStructure)
{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_aacdec_component_PrivateType* AACPrivate = openmaxStandComp->pComponentPrivate;;

	if (ComponentParameterStructure == NULL)
	{
		return OMX_ErrorBadParameter;
	}
	
	DBUG_MSG("AAC Dec Getting parameter 0x%x\n", nParamIndex);

	switch(nParamIndex) 
	{
		case OMX_IndexParamAudioPcm:
		{
			OMX_AUDIO_PARAM_PCMMODETYPE* pcmParams = (OMX_AUDIO_PARAM_PCMMODETYPE*)ComponentParameterStructure;
			if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE))) != OMX_ErrorNone) 
			{ 
				return err;
			}
			if (pcmParams->nPortIndex != 1) 
			{
				return OMX_ErrorBadPortIndex;
			}
			
			memcpy(pcmParams, &AACPrivate->pAudioPcmMode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
			
			return OMX_ErrorNone;
		}
		
		case OMX_IndexParamAudioAac:
		{
			OMX_AUDIO_PARAM_AACPROFILETYPE* aacParams = (OMX_AUDIO_PARAM_AACPROFILETYPE*)ComponentParameterStructure;
			if (aacParams->nPortIndex != 0) 
			{
				return OMX_ErrorBadPortIndex;
			}
			if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_AACPROFILETYPE))) != OMX_ErrorNone) 
			{ 
				return err;
			}
			memcpy(aacParams, &AACPrivate->pAudioAac, sizeof(OMX_AUDIO_PARAM_AACPROFILETYPE));
			return OMX_ErrorNone;
		}

		case OMX_IndexParamStandardComponentRole:
		{
			OMX_PARAM_COMPONENTROLETYPE * roleParams = (OMX_PARAM_COMPONENTROLETYPE*)ComponentParameterStructure;
			if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_PARAM_COMPONENTROLETYPE))) != OMX_ErrorNone) 
			{ 
				return err;
			}
			
			strcpy((char*)roleParams->cRole, AUDIO_DEC_TCC_AAC_ROLE);
			return OMX_ErrorNone;
		}
			
		default: /*Call the common GetParameter function*/
			return omx_audiodec_component_GetParameter(hComponent, nParamIndex, ComponentParameterStructure);
	}
}

#define LATM_PARSER_LIB_NAME "libTCC.latmdmx.so"

static OMX_S32 OpenLatmDemuxer(omx_aacdec_component_PrivateType* pPrivate, unsigned char* pucLatmstream, int iStreamSize, int iNeedStreamInfo)
{
	// setting callback functions for latm demuxer
	pPrivate->stCallbackFunc.m_pfMalloc = (void* (*) ( unsigned int ))malloc;
	pPrivate->stCallbackFunc.m_pfRealloc  = (void* (*) ( void*, unsigned int ))realloc;
	pPrivate->stCallbackFunc.m_pfFree= (void  (*) ( void* ))free;
	pPrivate->stCallbackFunc.m_pfMemcpy= (void* (*) ( void*, const void*, unsigned int ))memcpy;
	pPrivate->stCallbackFunc.m_pfMemmove= (void* (*) ( void*, const void*, unsigned int ))memmove;
	pPrivate->stCallbackFunc.m_pfMemset= (void  (*) ( void*, int, unsigned int ))memset;

	pPrivate->pfLatmDLLModule = dlopen(LATM_PARSER_LIB_NAME, RTLD_LAZY | RTLD_GLOBAL);
    if( pPrivate->pfLatmDLLModule == NULL ) 
	{
        ALOGE("Load library '%s' failed: %s", LATM_PARSER_LIB_NAME, dlerror());
        return -1;
    } 
	else 
    {
		ALOGI("Library '%s' Loaded", LATM_PARSER_LIB_NAME);
    }

	pPrivate->pfLatmInit = dlsym(pPrivate->pfLatmDLLModule, "Latm_Init");
    if( pPrivate->pfLatmInit == NULL ) 
	{
		ALOGE("Load symbol Latm_Init failed");
        return -1;
    }

	pPrivate->pfLatmClose = dlsym(pPrivate->pfLatmDLLModule, "Latm_Close");
    if( pPrivate->pfLatmClose == NULL ) 
	{
		ALOGE("Load symbol Latm_Close failed");
        return -1;
    }

	pPrivate->pfLatmGetFrame = dlsym(pPrivate->pfLatmDLLModule, "Latm_GetFrame");
    if( pPrivate->pfLatmGetFrame == NULL ) 
	{
		ALOGE("Load symbol Latm_GetFrame failed");
        return -1;
    }

	//pPrivate->pfLatmGetHeaderType = dlsym(pPrivate->pfLatmDLLModule, "Latm_GetHeaderType");
    //if( pPrivate->pfLatmGetHeaderType == NULL ) {
	//	ALOGE("Load symbol Latm_GetHeaderType failed");
    //   return -1;
    //}

	if(iNeedStreamInfo && pucLatmstream && iStreamSize )
	{
		pPrivate->pvSubParser = pPrivate->pfLatmInit( pucLatmstream, iStreamSize, (int *)&pPrivate->stStreamInfo.m_eSampleRate, (int *)&pPrivate->stStreamInfo.m_uiNumberOfChannel, (void*)&pPrivate->stCallbackFunc, TF_AAC_LOAS);
	}
	else
	{
		pPrivate->pvSubParser = pPrivate->pfLatmInit( NULL, 0, NULL, NULL, (void*)&pPrivate->stCallbackFunc, TF_AAC_LOAS);	
	}

	if( pPrivate->pvSubParser == NULL )
	{
		if( pPrivate->pfLatmDLLModule != NULL )
		{
			dlclose(pPrivate->pfLatmDLLModule);
			pPrivate->pfLatmDLLModule = NULL;
		}
		return -1;				
	}

	return 0;
}

#if 0	
OMX_S32 SetMultiChannelOrder(OMX_COMPONENTTYPE *openmaxStandComp)
{
	omx_audiodec_component_PrivateType* pPrivate = openmaxStandComp->pComponentPrivate;  
	OMX_U32 i;

	for(i = 0; i < OMX_AUDIO_MAXCHANNELS; i++)
	{
		ALOGD("ch %d, type %d :: ", i, pPrivate->pAudioPcmMode.eChannelMapping[i]);
		switch(pPrivate->pAudioPcmMode.eChannelMapping[i])
		{
		case OMX_AUDIO_ChannelNone:	/**< Unused or empty */
			ALOGD("%dth Channel is Unused!\n", i);
			break;
		case OMX_AUDIO_ChannelLF:	/**< Left front */
			ALOGD("%dth Channel is Left front!\n", i);
			pPrivate->stPcmInfo.m_iNchannelOrder[CH_LEFT_FRONT] = i+1;
			break;
	    case OMX_AUDIO_ChannelRF:	/**< Right front */
			ALOGD("%dth Channel is Right front!\n", i);
			pPrivate->stPcmInfo.m_iNchannelOrder[CH_RIGHT_FRONT] = i+1;
			break;
	    case OMX_AUDIO_ChannelCF:	/**< Center front */
			ALOGD("%dth Channel is Center front!\n", i);
			pPrivate->stPcmInfo.m_iNchannelOrder[CH_CENTER] = i+1;
			break;
	    case OMX_AUDIO_ChannelLS:	/**< Left surround */
			ALOGD("%dth Channel is Left surround!\n", i);
			pPrivate->stPcmInfo.m_iNchannelOrder[CH_LEFT_SIDE] = i+1;
			break;
	    case OMX_AUDIO_ChannelRS:	/**< Right surround */
			ALOGD("%dth Channel is Right surround!\n", i);
			pPrivate->stPcmInfo.m_iNchannelOrder[CH_RIGHT_SIDE] = i+1;
			break;			
	    case OMX_AUDIO_ChannelLFE:	/**< Low frequency effects */
			ALOGD("%dth Channel is Low frequency effects!\n", i);
			pPrivate->stPcmInfo.m_iNchannelOrder[CH_LFE] = i+1;
			break;			
	    case OMX_AUDIO_ChannelCS:	/**< Back surround */
			ALOGD("%dth Channel is Back surround -> not support!\n", i);
			break;			
	    case OMX_AUDIO_ChannelLR:	/**< Left rear. */
			ALOGD("%dth Channel is Left rear!\n", i);
			pPrivate->stPcmInfo.m_iNchannelOrder[CH_LEFT_REAR] = i+1;
			break;			
	    case OMX_AUDIO_ChannelRR:	/**< Right rear. */
			ALOGD("%dth Channel is Right rear!\n", i);
			pPrivate->stPcmInfo.m_iNchannelOrder[CH_RIGHT_REAR] = i+1;
			break;			
		default:
			ALOGD("not support!!\n", i);
			break;
		}
	}
}
#endif

static OMX_S32 OpenAACDec(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer)
{
	omx_aacdec_component_PrivateType* pPrivate = openmaxStandComp->pComponentPrivate;
	OMX_S32 i, ret	= 0;
	
	// setting callback functions
	pPrivate->stADecInit.m_pfMalloc = (void* (*) ( unsigned int ))malloc;
	pPrivate->stADecInit.m_pfFree = (void  (*) ( void* ))free;
	pPrivate->stADecInit.m_pfMemcpy = (void* (*) ( void*, const void*, unsigned int ))memcpy;
	pPrivate->stADecInit.m_pfMemmove = (void* (*) ( void*, const void*, unsigned int ))memmove;
	pPrivate->stADecInit.m_pfMemset = (void  (*) ( void*, int, unsigned int ))memset;
#if 0	
	ALOGD("AAC: ch %d, sf %d, br %d, bw %d, len %d, tool %d, ertool %d, profile %d, format %d, mode %d", 
		pPrivate->pAudioAac.nChannels, 
		pPrivate->pAudioAac.nSampleRate, 
		pPrivate->pAudioAac.nBitRate, 
		pPrivate->pAudioAac.nAudioBandWidth, 
		pPrivate->pAudioAac.nFrameLength, 
		pPrivate->pAudioAac.nAACtools, 
		pPrivate->pAudioAac.nAACERtools, 
		pPrivate->pAudioAac.eAACProfile, 
		pPrivate->pAudioAac.eAACStreamFormat, 
		pPrivate->pAudioAac.eChannelMode);
	
	ALOGD("PCM: ch %d, sign %d, endian %d, interleave %d, bps %d, sf %d, mode %d", 
		pPrivate->pAudioPcmMode.nChannels, 
		pPrivate->pAudioPcmMode.eNumData, 
		pPrivate->pAudioPcmMode.eEndian, 
		pPrivate->pAudioPcmMode.bInterleaved, 
		pPrivate->pAudioPcmMode.nBitPerSample, 
		pPrivate->pAudioPcmMode.nSamplingRate, 
		pPrivate->pAudioPcmMode.ePCMMode);
#endif		

	// setting pcm info 
	pPrivate->stPcmInfo.m_uiBitsPerSample = pPrivate->pAudioPcmMode.nBitPerSample;
	pPrivate->stPcmInfo.m_ePcmInterLeaved = pPrivate->pAudioPcmMode.bInterleaved;
	
#if 0	
	if(pPrivate->pAudioAac.nChannels > 2 && pPrivate->iMultiChannelAudioOut)	// multichannel output
	{
		pPrivate->stADecInit.m_iDownMixMode = 0;
		SetMultiChannelOrder(openmaxStandComp);
		
	}
	else
#endif		
	{
		pPrivate->stADecInit.m_iDownMixMode = 1;
		pPrivate->stPcmInfo.m_iNchannelOrder[CH_LEFT_FRONT] = 1;	//first channel
		pPrivate->stPcmInfo.m_iNchannelOrder[CH_RIGHT_FRONT] = 2;	//second channel
	}

	pPrivate->stADecInit.m_psAudiodecInput = &pPrivate->stStreamInfo;
	pPrivate->stADecInit.m_psAudiodecOutput = &pPrivate->stPcmInfo;

	pPrivate->stADecInit.m_unAudioCodecParams.m_unAAC.m_iAACForceUpmix = 1;
	pPrivate->stADecInit.m_unAudioCodecParams.m_unAAC.m_iAACForceUpsampling = 1;
	pPrivate->iMinStreamSize = 4;

	pPrivate->stStreamInfo.m_eSampleRate = pPrivate->pAudioAac.nSampleRate;
	pPrivate->stStreamInfo.m_uiNumberOfChannel = pPrivate->pAudioAac.nChannels;

	if(pPrivate->iExtractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_TCC)
	{
		// setting extra info
		pPrivate->stADecInit.m_pucExtraData = pPrivate->cdmx_info.m_sAudioInfo.m_pExtraData;
		pPrivate->stADecInit.m_iExtraDataLen = pPrivate->cdmx_info.m_sAudioInfo.m_iExtraDataLen;

		/* START : latm demuxer init -----------------------------------------*/
		if((pPrivate->cdmx_info.m_sAudioInfo.m_ulSubType == AUDIO_SUBTYPE_AAC_ADTS) || 
		   (pPrivate->cdmx_info.m_sAudioInfo.m_ulSubType == AUDIO_SUBTYPE_AAC_LATM))
		{
			unsigned char* pucLatmstream;
			int iStreamSize;
			int iNeedStreamInfo;

			pucLatmstream = NULL;
			iStreamSize = 0;
			iNeedStreamInfo = 0;

			if(pPrivate->stStreamInfo.m_eSampleRate == 0)
			{
				iNeedStreamInfo = 1;
				if(pPrivate->cdmx_info.m_sAudioInfo.m_pExtraData && pPrivate->cdmx_info.m_sAudioInfo.m_iExtraDataLen)
				{
					pucLatmstream = pPrivate->cdmx_info.m_sAudioInfo.m_pExtraData;
					iStreamSize = pPrivate->cdmx_info.m_sAudioInfo.m_iExtraDataLen;
				}
				else	
				{
					DBUG_MSG( "[AAC DEC] need latm stream for codec-init\n" );
					return -1;		
				}
			}

			if(OpenLatmDemuxer(pPrivate, pucLatmstream, iStreamSize, iNeedStreamInfo) != 0)
			{
				DBUG_MSG( "[AAC DEC] latm demuxer init failed\n" );
				return -1;	
			}
			
			DBUG_MSG( "[AAC DEC] latm demuxer init ok\n" );

			pPrivate->stADecInit.m_pucExtraData = NULL;	// No GASpecificConfig
			pPrivate->stADecInit.m_iExtraDataLen = 0;
			pPrivate->iMinStreamSize = 0;
			pPrivate->iCurrInputBufferOrgSize = 0;
		}
		/* End : latm demuxer init -----------------------------------------*/

		if( ( pPrivate->cdmx_info.m_sAudioInfo.m_iMinDataSize != 0 ) && ( pPrivate->iCtype == CONTAINER_TYPE_AUDIO) )
		{
			pPrivate->iMinStreamSize = pPrivate->cdmx_info.m_sAudioInfo.m_iMinDataSize;
		}
		
		if( pPrivate->iCtype == CONTAINER_TYPE_MP4 )
		{
			pPrivate->iMinStreamSize = 0;
		}
	}
	else
	{
		// AudioSpecificConfig data for OMX_AUDIO_CodingAAC
		pPrivate->stADecInit.m_pucExtraData = inputbuffer->pBuffer;
		pPrivate->stADecInit.m_iExtraDataLen = inputbuffer->nFilledLen;

		switch(pPrivate->pAudioAac.eAACProfile)
		{
		case OMX_AUDIO_AACObjectMain:
		case OMX_AUDIO_AACObjectLC:
		case OMX_AUDIO_AACObjectLTP:
		case OMX_AUDIO_AACObjectHE:
			pPrivate->stADecInit.m_unAudioCodecParams.m_unAAC.m_iAACObjectType = pPrivate->pAudioAac.eAACProfile;
			break;			
		case OMX_AUDIO_AACObjectHE_PS:		
			pPrivate->stADecInit.m_unAudioCodecParams.m_unAAC.m_iAACObjectType = OMX_AUDIO_AACObjectHE;
			break;						
			
		default:
			ALOGD("not support --> set default\n");
			pPrivate->stADecInit.m_unAudioCodecParams.m_unAAC.m_iAACObjectType = OMX_AUDIO_AACObjectLC;
		}		

		switch(pPrivate->pAudioAac.eAACStreamFormat)
		{
		case OMX_AUDIO_AACStreamFormatMP2ADTS:
		case OMX_AUDIO_AACStreamFormatMP4ADTS:
			pPrivate->stADecInit.m_unAudioCodecParams.m_unAAC.m_iAACHeaderType = 1; // ADTS
			pPrivate->iMinStreamSize = 0;
			break;
		case OMX_AUDIO_AACStreamFormatMP4LATM:
			if(OpenLatmDemuxer(pPrivate, NULL, 0, 0) != 0)
			{
				DBUG_MSG( "[AAC DEC] latm demuxer init failed\n" );
				return -1;	
			}
			
			DBUG_MSG( "[AAC DEC] latm demuxer init ok\n" );
			pPrivate->iMinStreamSize = 0;
			
		case OMX_AUDIO_AACStreamFormatRAW:
			pPrivate->stADecInit.m_unAudioCodecParams.m_unAAC.m_iAACHeaderType = 0; // RAW
			break;			
		case OMX_AUDIO_AACStreamFormatADIF: 
			pPrivate->stADecInit.m_unAudioCodecParams.m_unAAC.m_iAACHeaderType = 2; // ADIF
			break;			
		default:
			pPrivate->stADecInit.m_unAudioCodecParams.m_unAAC.m_iAACHeaderType = 0; // RAW
			break;			
		}
	}

	ret = pPrivate->pExternalDec(AUDIO_INIT, &pPrivate->iDecoderHandle, &pPrivate->stADecInit, NULL);

	DBUG_MSG("%s result %d\n",__func__, ret);

	/* reset PCM Info */
	pPrivate->pAudioPcmMode.nChannels = pPrivate->stPcmInfo.m_uiNumberOfChannel;
	pPrivate->pAudioPcmMode.nSamplingRate = pPrivate->stPcmInfo.m_eSampleRate;

	/* reset AAC Info */
	pPrivate->pAudioAac.nSampleRate = pPrivate->stPcmInfo.m_eSampleRate;
	pPrivate->pAudioAac.nChannels = pPrivate->stPcmInfo.m_uiNumberOfChannel;
	
	return ret;
}

static OMX_S32 DecodeAAC(OMX_COMPONENTTYPE* openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer, OMX_BUFFERHEADERTYPE* outputbuffer)
{
	OMX_S32 iOutFrameSize, iRet = TCAS_SUCCESS;
	omx_aacdec_component_PrivateType* pPrivate = openmaxStandComp->pComponentPrivate;

	iOutFrameSize = 0;
	pPrivate->iDecodedSamplePerCh = 0;

	if(pPrivate->pvSubParser != NULL)
	{	
		if(inputbuffer->nOffset == 0) // new buffer ?
		{
			pPrivate->iCurrInputBufferOrgSize = pPrivate->stStreamInfo.m_iStreamLength;
			iRet = pPrivate->pfLatmGetFrame( pPrivate->pvSubParser, pPrivate->stStreamInfo.m_pcStream, pPrivate->stStreamInfo.m_iStreamLength, &pPrivate->pAACRawData, &pPrivate->iAACRawDataSize, 0 );
			if( iRet < 0 )
			{
				DBUG_MSG("[AAC DEC] latm_parser_get_frame: Fatal error %d!\n", iRet);
				pPrivate->stStreamInfo.m_iStreamLength = 0;
				return iRet;
			}

			if(pPrivate->iAACRawDataSize <= 0)
			{
				DBUG_MSG("[AAC DEC] latm_parser_get_frame: Need more data!\n");
				pPrivate->stStreamInfo.m_iStreamLength = 0;
				return TCAS_ERROR_MORE_DATA;
			}

			pPrivate->stStreamInfo.m_pcStream = pPrivate->pAACRawData;
			pPrivate->stStreamInfo.m_iStreamLength = pPrivate->iAACRawDataSize;
		}
	}

	DBUG_MSG("%s: input length %d", __func__, pPrivate->stStreamInfo.m_iStreamLength);

	while((pPrivate->stStreamInfo.m_iStreamLength > pPrivate->iMinStreamSize ) || ((pPrivate->stStreamInfo.m_iStreamLength > 0) && (pPrivate->iEndOfFile == 1)))
	{
		pPrivate->stPcmInfo.m_pvChannel[0]  = (void *)(outputbuffer->pBuffer + iOutFrameSize);

		iRet = pPrivate->pExternalDec(AUDIO_DECODE, &pPrivate->iDecoderHandle, &pPrivate->stStreamInfo, &pPrivate->stPcmInfo);
		
		if(iRet == TCAS_SUCCESS || iRet == TCAS_ERROR_CONCEALMENT_APPLIED)
		{
			iOutFrameSize += pPrivate->stPcmInfo.m_uiNumberOfChannel * pPrivate->stPcmInfo.m_uiSamplesPerChannel * sizeof(short);
			pPrivate->iDecodedSamplePerCh += pPrivate->stPcmInfo.m_uiSamplesPerChannel;
			break;
		}
		else
		{
			if(iRet != TCAS_ERROR_SKIP_FRAME)
			{
				break;
			}
		}
	}

	if(pPrivate->stPcmInfo.m_uiNumberOfChannel != pPrivate->pAudioPcmMode.nChannels)
	{
		pPrivate->pAudioPcmMode.nChannels = pPrivate->stPcmInfo.m_uiNumberOfChannel;
	}
	if(pPrivate->stPcmInfo.m_eSampleRate != pPrivate->pAudioPcmMode.nSamplingRate)
	{
		pPrivate->pAudioPcmMode.nSamplingRate = pPrivate->stPcmInfo.m_eSampleRate;
	}
	
	return iRet;
}

static OMX_S32 FlushAACDec(OMX_COMPONENTTYPE* openmaxStandComp)
{
	omx_aacdec_component_PrivateType* pPrivate = openmaxStandComp->pComponentPrivate;

	DBUG_MSG("%s\n",__func__);
	
	pPrivate->pExternalDec(AUDIO_FLUSH, &pPrivate->iDecoderHandle, NULL, NULL);

	return 0;
}

static OMX_S32 CloseAACDec(OMX_COMPONENTTYPE* openmaxStandComp)
{
	omx_aacdec_component_PrivateType* pPrivate = openmaxStandComp->pComponentPrivate;

	DBUG_MSG("%s\n",__func__);

	if(pPrivate->pvSubParser != NULL)
	{
		pPrivate->pfLatmClose(pPrivate->pvSubParser);
		if( pPrivate->pfLatmDLLModule != NULL)
		{
			dlclose(pPrivate->pfLatmDLLModule);
			pPrivate->pfLatmDLLModule = NULL;
		}
	}

	pPrivate->pExternalDec(AUDIO_CLOSE, &pPrivate->iDecoderHandle, NULL, NULL);

	pPrivate->iDecoderHandle = 0;

	return 0;
}



