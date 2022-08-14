/**

  @file omx_tp_audiodec_component.c

  This component implement 3rd-party audio decoder.

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

#include <omx_tp_audiodec_component.h>

#include "../lib/third_party_audio_dec_api.h"
#include "TCCMemory.h"

#ifdef HAVE_ANDROID_OS
#define USE_EXTERNAL_BUFFER 0
#define LOG_TAG	"OMX_3rdParty_DEC"
#include <utils/Log.h>

static int DEBUG_ON	= 0;
#define DBUG_MSG(msg...)	if (DEBUG_ON) { ALOGD( ": " msg);}
#endif /* HAVE_ANDROID_OS */

#ifdef HAVE_ANDROID_OS
OMX_ERRORTYPE OMX_ComponentInit(OMX_HANDLETYPE openmaxStandComp, OMX_STRING cCompontName, OMX_HANDLE_FUNC pHandleFunc)
{
  	OMX_ERRORTYPE err = OMX_ErrorNone;
  	
  	DBUG_MSG("In %s \n",__func__);

	err = component_Constructor(openmaxStandComp,cCompontName);
	omx_audiodec_component_Init(openmaxStandComp);	// Pass-Through 2ch Audio Playback Planet 20121204

	return err;
}
#endif

OMX_ERRORTYPE component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp, OMX_STRING cComponentName) 
{

	OMX_ERRORTYPE err = OMX_ErrorNone;  
	component_PrivateType* pPrivate;
	omx_base_audio_PortType *inPort,*outPort;
	OMX_U32 i;

	DBUG_MSG("In %s\n", __func__);
#ifdef HAVE_ANDROID_OS
	if (1)
#else
	if (!openmaxStandComp->pComponentPrivate) 
#endif
	{
		openmaxStandComp->pComponentPrivate = TCC_calloc(1, sizeof(component_PrivateType));

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

	pPrivate = openmaxStandComp->pComponentPrivate;
	pPrivate->ports = NULL;

	/** we could create our own port structures here
	* fixme maybe the base class could use a "port factory" function pointer?  
	*/
	err = omx_base_filter_Constructor(openmaxStandComp, cComponentName);

	DBUG_MSG("constructor of ThirdParty decoder component is called\n");

	/* Domain specific section for the ports. */  
	/* first we set the parameter common to both formats */
	/* parameters related to input port which does not depend upon input audio format    */
	/* Allocate Ports and call port constructor. */  

	pPrivate->sPortTypesParam[OMX_PortDomainAudio].nStartPortNumber = 0;
  	pPrivate->sPortTypesParam[OMX_PortDomainAudio].nPorts = 2;

	if (pPrivate->sPortTypesParam[OMX_PortDomainAudio].nPorts && !pPrivate->ports) 
	{
	    pPrivate->ports = TCC_calloc(pPrivate->sPortTypesParam[OMX_PortDomainAudio].nPorts, sizeof(omx_base_PortType *));
	    if (!pPrivate->ports) 
	    {
	  	    return OMX_ErrorInsufficientResources;
	    }
	    for (i=0; i < pPrivate->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) 
	    {
		      pPrivate->ports[i] = TCC_calloc(1, sizeof(omx_base_audio_PortType));
		      if (!pPrivate->ports[i]) 
		      {
		        	return OMX_ErrorInsufficientResources;
		      }
	    }
	}

	base_audio_port_Constructor(openmaxStandComp, &pPrivate->ports[0], 0, OMX_TRUE); // input
	base_audio_port_Constructor(openmaxStandComp, &pPrivate->ports[1], 1, OMX_FALSE); // output

	/*======================================================================================================*/
	/*	3rd-Praty Lib Port setting     													 					*/
	/*======================================================================================================*/	
	Port_Setting(pPrivate);
	
	/** now it's time to know the audio coding type of the component */
	if(!strcmp(cComponentName, audio_dec_name))  
	{   
		 pPrivate->audio_coding_type = audio_coding_type;
	} 
	else if (!strcmp(cComponentName, AUDIO_DEC_BASE_NAME)) 
	{
		pPrivate->audio_coding_type = OMX_AUDIO_CodingUnused;
	}
	else  
	{
	    // IL client specified an invalid component name
	    
		ALOGE("OMX_ErrorInvalidComponentName %s", cComponentName);
	    return OMX_ErrorInvalidComponentName;
	}

	/** general configuration irrespective of any audio formats */
	/**  setting values of other fields of omx_maddec_component_Private structure */	
	pPrivate->BufferMgmtCallback =omx_audiodec_component_BufferMgmtCallback;
	pPrivate->destructor = omx_audiodec_component_Destructor;
	pPrivate->messageHandler = omx_audiodec_component_MessageHandler;
	openmaxStandComp->SetParameter = omx_audiodec_component_SetParameter;
	openmaxStandComp->GetParameter = omx_audiodec_component_GetParameter;
	openmaxStandComp->GetExtensionIndex = omx_audiodec_component_GetExtensionIndex;

	pPrivate->OpenCodec = OpenDec;
	pPrivate->DecodeCodec = Decode;
	pPrivate->FlushCodec = FlushDec;
	pPrivate->CloseCodec = CloseDec;

	pPrivate->decode_ready = OMX_FALSE;	
	
	memset(&pPrivate->cdmx_info, 0x00, sizeof(cdmx_info_t));	
	memset(&pPrivate->stStreamInfo, 0x00, sizeof(audio_streaminfo_t));
	
	DBUG_MSG("constructor of ThirdParty decoder component is completed %d \n", err);
	
	return err;

}

#define OpenDecoder		ThirdPartyAudio_OpenDecoder 
#define InitDecoder 	ThirdPartyAudio_InitDecoder 
#define DecodeFrame		ThirdPartyAudio_DecodeFrame 
#define FlushDecoder	ThirdPartyAudio_FlushDecoder
#define CloseDecoder	ThirdPartyAudio_CloseDecoder

static OMX_S32 OpenDec(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer)
{
	component_PrivateType* pPrivate = openmaxStandComp->pComponentPrivate;
	OMX_S32 iDecodedSampleRate, iDecodedChannels;
	OMX_S32 ret	= 0;


	if(pPrivate->iExtractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_TCC)
	{
		iDecodedSampleRate = pPrivate->cdmx_info.m_sAudioInfo.m_iSamplePerSec;
		iDecodedChannels = pPrivate->cdmx_info.m_sAudioInfo.m_iChannels;
	}

	/*======================================================================================================*/
	/*	3rd-Praty Lib Open             													 					*/
	/*======================================================================================================*/	
	pPrivate->pDecoder = OpenDecoder(pPrivate);
	if(pPrivate->pDecoder == NULL)
	{
		return OMX_ErrorInsufficientResources;
	}
	
    /*======================================================================================================*/
    /*	3rd-Praty Lib Init																 					*/
    /*======================================================================================================*/
	ret = InitDecoder(pPrivate, 
					  inputbuffer,
					  NULL,
					  &iDecodedSampleRate,
					  &iDecodedChannels);	
	if(ret == 0)
	{
		if(iDecodedChannels != pPrivate->pAudioPcmMode.nChannels )
		{
			pPrivate->pAudioPcmMode.nChannels = iDecodedChannels;
		}
		if(iDecodedSampleRate != pPrivate->pAudioPcmMode.nSamplingRate )
		{
			pPrivate->pAudioPcmMode.nSamplingRate = iDecodedSampleRate;
		}
	}
	
	pPrivate->iMinStreamSize = MIN_STREM_SIZE;
	
	DBUG_MSG("%s result %d\n",__func__, ret);

	return ret;

}

static OMX_S32 Decode(OMX_COMPONENTTYPE* openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer, OMX_BUFFERHEADERTYPE* outputbuffer)
{
	OMX_S32 iOutFrameSize, iRet = TCAS_SUCCESS;
	component_PrivateType* pPrivate = openmaxStandComp->pComponentPrivate;
	OMX_S32 iUsedBytes, iDecodedSampleRate, iDecodedChannels;
	OMX_U8* pOutPtr;
	OMX_S32 iDecodedSamples = 0;

	iOutFrameSize = 0;

	DBUG_MSG("3rd-party %s: input length %d", __func__, pPrivate->stStreamInfo.m_iStreamLength);

	iDecodedChannels = pPrivate->pAudioPcmMode.nChannels;
	iDecodedSampleRate = pPrivate->pAudioPcmMode.nSamplingRate;
	
	pOutPtr = outputbuffer->pBuffer;

	while(pPrivate->stStreamInfo.m_iStreamLength > pPrivate->iMinStreamSize)
	{
		/*======================================================================================================*/
		/* 3rd-Praty Lib Decode														 							*/
		/*======================================================================================================*/		
		// The following information is provided as input through the variable on the left::
        //    Variable       Meaning
		// 1. pInputPtr    = input bit-stream buffer pointer
		// 2. iInputLength = amount of input bit-stream in bytes
		// 3. pOutPtr      = output pcm buffer pointer
				
		iRet = DecodeFrame( pPrivate, 
						    pPrivate->stStreamInfo.m_pcStream, 
							pPrivate->stStreamInfo.m_iStreamLength, 
							(OMX_PTR *)pOutPtr, 
							&iUsedBytes, 
							&iOutFrameSize,
							&iDecodedSampleRate,
							&iDecodedChannels);
						  				  
		// The following information must be provided through the variable on the left::
        //    Variable                Meaning
		// 1. iUsedBytes      		= How many bytes the decoder has read (used bytes of input buffer)
		// 2. iDecodedSamples 		= How many samples were decoded (number of total samples, not bytes)
		// 3. iDecodedSampleRate	= Samplerate of Decoded PCM
		// 4. iDecodedChannels		= Number of channels of Decoded PCM
		// 5. iRet             		= Decoding result
		//    iRet == 0  	: Decoding success
		//    iRet == 1		: Need more bitstream data
		//    iRet == 2 	: This is recoverable, just ignore the current frame
		//    else			: Unrecoverable error, the remaining data in the input bit-stream buffer will be discarded
				
		/* input buffer update */
		pPrivate->stStreamInfo.m_pcStream += iUsedBytes;
		pPrivate->stStreamInfo.m_iStreamLength -= iUsedBytes;
		
		/* output buffer update */
		if(iRet == 0)
		{
			if(iOutFrameSize)
			{
				iDecodedSamples += iOutFrameSize;
				pOutPtr = outputbuffer->pBuffer + iDecodedSamples * sizeof(short);
				if(iUsedBytes)
				{
					break;
				}
			}
		}
		else
		{
			if(iRet != 2) // need more (ret == 1) or unrecoverable error!
			{
				if(iRet < 0 || iRet > 2)
				{
					pPrivate->stStreamInfo.m_iStreamLength = 0;	// Unrecoverable, Discarding the remaining data
				}
				break;
			}
		}
	}

	if(iDecodedChannels != pPrivate->pAudioPcmMode.nChannels )
	{
		pPrivate->pAudioPcmMode.nChannels = iDecodedChannels;
	}
	if(iDecodedSampleRate != pPrivate->pAudioPcmMode.nSamplingRate )
	{
		pPrivate->pAudioPcmMode.nSamplingRate = iDecodedSampleRate;
	}
	
	pPrivate->iDecodedSamplePerCh = iDecodedSamples/pPrivate->pAudioPcmMode.nChannels;
		
	switch(iRet)
	{
	case 0:
		iRet = TCAS_SUCCESS;
		break;
	case 1:
		iRet = TCAS_ERROR_MORE_DATA;
		break;		
	case 2:
		iRet = TCAS_ERROR_SKIP_FRAME;
		break;		
	default:
		iRet = TCAS_ERROR_DECODE;
		break;
	}
	
	return iRet;
}

static OMX_S32 FlushDec(OMX_COMPONENTTYPE* openmaxStandComp)
{
	component_PrivateType* pPrivate = openmaxStandComp->pComponentPrivate;

	DBUG_MSG("%s\n",__func__);
	
	FlushDecoder(pPrivate);

	return 0;
}

static OMX_S32 CloseDec(OMX_COMPONENTTYPE* openmaxStandComp)
{
	component_PrivateType* pPrivate = openmaxStandComp->pComponentPrivate;

	DBUG_MSG("%s\n",__func__);

	CloseDecoder(pPrivate);

	pPrivate->pDecoder = NULL;

	return 0;
}

