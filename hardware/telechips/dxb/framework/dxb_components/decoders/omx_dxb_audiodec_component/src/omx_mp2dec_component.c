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

#include <omxcore.h>
#include <omx_base_audio_port.h>

#include <omx_mp2dec_component.h>
#include <tccaudio.h>
#include <OMX_TCC_Index.h>
#include "TCCFile.h"
#include "TCCMemory.h"

#ifdef HAVE_ANDROID_OS
#define USE_EXTERNAL_BUFFER 0
#define LOG_TAG	"OMX_TCC_MP2DEC"
#include <utils/Log.h>

static int DEBUG_ON	= 0;
#define DBUG_MSG(msg...)	if (DEBUG_ON) { ALOGD( ": " msg);}
#endif /* HAVE_ANDROID_OS */

#ifdef HAVE_ANDROID_OS
OMX_ERRORTYPE OMX_DXB_AudioDec_MP2ComponentInit(OMX_COMPONENTTYPE *openmaxStandComp)
{
  OMX_ERRORTYPE err = OMX_ErrorNone;

	err = omx_mp2dec_component_Constructor(openmaxStandComp,AUDIO_DEC_MP2_NAME);

	return err;
}

#if 0
OMX_ERRORTYPE OMX_ComponentInit(OMX_HANDLETYPE openmaxStandComp, OMX_STRING cCompontName)
{
  OMX_ERRORTYPE err = OMX_ErrorNone;

	err = omx_mp2dec_component_Constructor(openmaxStandComp,cCompontName);

	return err;
}
#endif
#endif

OMX_ERRORTYPE omx_mp2dec_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp, OMX_STRING cComponentName) 
{

	OMX_ERRORTYPE err = OMX_ErrorNone;  
	omx_mp2dec_component_PrivateType* omx_mp2dec_component_Private;
	omx_base_audio_PortType *inPort,*outPort;
	OMX_U32 i;

	if (!openmaxStandComp->pComponentPrivate) 
	{
		openmaxStandComp->pComponentPrivate = TCC_calloc(1, sizeof(omx_mp2dec_component_PrivateType));

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

	omx_mp2dec_component_Private = openmaxStandComp->pComponentPrivate;
	omx_mp2dec_component_Private->ports = NULL;

	/** we could create our own port structures here
	* fixme maybe the base class could use a "port factory" function pointer?  
	*/
	err = omx_base_filter_Constructor(openmaxStandComp, cComponentName);

	DBUG_MSG("constructor of MP2 decoder component is called\n");

	/* Domain specific section for the ports. */  
	/* first we set the parameter common to both formats */
	/* parameters related to input port which does not depend upon input audio format    */
	/* Allocate Ports and call port constructor. */  

	omx_mp2dec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nStartPortNumber = 0;
  	omx_mp2dec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts = 3;

	if (omx_mp2dec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts && !omx_mp2dec_component_Private->ports) 
	{
	    omx_mp2dec_component_Private->ports = TCC_calloc(omx_mp2dec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts, sizeof(omx_base_PortType *));
	    if (!omx_mp2dec_component_Private->ports) 
	    {
	  	    return OMX_ErrorInsufficientResources;
	    }
	    for (i=0; i < omx_mp2dec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) 
	    {
		      omx_mp2dec_component_Private->ports[i] = TCC_calloc(1, sizeof(omx_base_audio_PortType));
		      if (!omx_mp2dec_component_Private->ports[i]) 
		      {
		        	return OMX_ErrorInsufficientResources;
		      }
	    }
	}

	base_audio_port_Constructor(openmaxStandComp, &omx_mp2dec_component_Private->ports[0], 0, OMX_TRUE); // input
	base_audio_port_Constructor(openmaxStandComp, &omx_mp2dec_component_Private->ports[1], 1, OMX_TRUE); // input
	base_audio_port_Constructor(openmaxStandComp, &omx_mp2dec_component_Private->ports[2], 2, OMX_FALSE); // output

	/** parameters related to input port */
	inPort = (omx_base_audio_PortType *) omx_mp2dec_component_Private->ports[0];
	inPort->sPortParam.nBufferSize = AUDIO_DXB_IN_BUFFER_SIZE;	
	strcpy(inPort->sPortParam.format.audio.cMIMEType, "audio/mpeg2");
	inPort->sPortParam.format.audio.eEncoding = OMX_AUDIO_CodingMP2;
	inPort->sAudioParam.eEncoding = OMX_AUDIO_CodingMP2;
	inPort->sPortParam.nBufferCountMin = 4;
	inPort->sPortParam.nBufferCountActual = 4;

	inPort = (omx_base_audio_PortType *) omx_mp2dec_component_Private->ports[1];
	inPort->sPortParam.nBufferSize = AUDIO_DXB_IN_BUFFER_SIZE;	
	strcpy(inPort->sPortParam.format.audio.cMIMEType, "audio/mpeg2");
	inPort->sPortParam.format.audio.eEncoding = OMX_AUDIO_CodingMP2;
	inPort->sAudioParam.eEncoding = OMX_AUDIO_CodingMP2;
	inPort->sPortParam.nBufferCountMin = 4;
	inPort->sPortParam.nBufferCountActual = 4;
	
	/** parameters related to output port */
	outPort = (omx_base_audio_PortType *) omx_mp2dec_component_Private->ports[2];
	outPort->sPortParam.nBufferSize = AUDIO_DXB_OUT_BUFFER_SIZE;
	outPort->sPortParam.format.audio.eEncoding = OMX_AUDIO_CodingPCM;
	outPort->sAudioParam.eEncoding = OMX_AUDIO_CodingPCM;
	outPort->sPortParam.nBufferCountMin = 8;
	outPort->sPortParam.nBufferCountActual = 8;

    //Default values for MP2 audio param port
	setHeader(&omx_mp2dec_component_Private->pAudioMp2, sizeof(OMX_AUDIO_PARAM_MP2TYPE));
    omx_mp2dec_component_Private->pAudioMp2.nPortIndex = 0;
    omx_mp2dec_component_Private->pAudioMp2.nChannels = 2;
    omx_mp2dec_component_Private->pAudioMp2.nBitRate = 0;
    omx_mp2dec_component_Private->pAudioMp2.nSampleRate = 48000;
    omx_mp2dec_component_Private->pAudioMp2.eChannelMode = OMX_AUDIO_ChannelModeStereo;
	omx_mp2dec_component_Private->pAudioMp2.eFormat = OMX_AUDIO_MP3StreamFormatMP2Layer3;

	/** settings of output port audio format - pcm */
	for(i=0; i<iTotalHandle; i++)
	{
		setHeader(&omx_mp2dec_component_Private->pAudioPcmMode[i], sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
		omx_mp2dec_component_Private->pAudioPcmMode[i].nPortIndex = 2;
		omx_mp2dec_component_Private->pAudioPcmMode[i].nChannels = 2;
		omx_mp2dec_component_Private->pAudioPcmMode[i].eNumData = OMX_NumericalDataSigned;
		omx_mp2dec_component_Private->pAudioPcmMode[i].eEndian = OMX_EndianLittle;
		omx_mp2dec_component_Private->pAudioPcmMode[i].bInterleaved = OMX_TRUE;
		omx_mp2dec_component_Private->pAudioPcmMode[i].nBitPerSample = 16;
		omx_mp2dec_component_Private->pAudioPcmMode[i].nSamplingRate = 48000;
		omx_mp2dec_component_Private->pAudioPcmMode[i].ePCMMode = OMX_AUDIO_PCMModeLinear;
		omx_mp2dec_component_Private->pAudioPcmMode[i].eChannelMapping[0] = OMX_AUDIO_ChannelLF;
		omx_mp2dec_component_Private->pAudioPcmMode[i].eChannelMapping[1] = OMX_AUDIO_ChannelRF;
	}

	/** now it's time to know the audio coding type of the component */
	OMX_U32 coding_type;
	if(!strcmp(cComponentName, AUDIO_DEC_MP2_NAME))  
	{   
		coding_type = OMX_AUDIO_CodingMP2;
	} 
	else if (!strcmp(cComponentName, AUDIO_DEC_BASE_NAME)) 
	{
		coding_type = OMX_AUDIO_CodingUnused;
	}
	else  
	{
	    // IL client specified an invalid component name
	    
		ALOGE("OMX_ErrorInvalidComponentName %s", cComponentName);
	    return OMX_ErrorInvalidComponentName;
	}


	/** general configuration irrespective of any audio formats */
	/**  setting values of other fields of omx_maddec_component_Private structure */
	omx_mp2dec_component_Private->BufferMgmtFunction = omx_twoport_filter_component_BufferMgmtFunction;
	omx_mp2dec_component_Private->BufferMgmtCallback = omx_audiodec_component_BufferMgmtCallback;
	omx_mp2dec_component_Private->messageHandler = omx_audiodec_component_MessageHandler;
	omx_mp2dec_component_Private->destructor = omx_audiodec_component_Destructor;
	openmaxStandComp->SetParameter = omx_audiodec_component_SetParameter;
	openmaxStandComp->GetParameter = omx_audiodec_component_GetParameter;
	openmaxStandComp->GetExtensionIndex = omx_audiodec_component_GetExtensionIndex;

	OMX_S16 nDevID;
	omx_mp2dec_component_Private->pAudioDecPrivate = TCC_calloc(iTotalHandle, sizeof(OMX_AUDIO_DEC_PRIVATE_DATA));
	for(nDevID=0 ; nDevID<iTotalHandle ; nDevID++) {
		omx_mp2dec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type = coding_type;

		omx_mp2dec_component_Private->pAudioDecPrivate[nDevID].decode_ready = OMX_FALSE;
	
		omx_mp2dec_component_Private->pAudioDecPrivate[nDevID].cdk_core.m_iAudioProcessMode = AUDIO_BROADCAST_MODE; /* decoded pcm mode */
	
		omx_mp2dec_component_Private->pAudioDecPrivate[nDevID].cdk_core.m_psCallback = &(omx_mp2dec_component_Private->callback_func);
		omx_mp2dec_component_Private->pAudioDecPrivate[nDevID].cdk_core.m_psCallback->m_pfMalloc   = (void* (*) ( unsigned int ))malloc;
		omx_mp2dec_component_Private->pAudioDecPrivate[nDevID].cdk_core.m_psCallback->m_pfRealloc  = (void* (*) ( void*, unsigned int ))realloc;
		omx_mp2dec_component_Private->pAudioDecPrivate[nDevID].cdk_core.m_psCallback->m_pfFree	  = (void  (*) ( void* ))free;
		omx_mp2dec_component_Private->pAudioDecPrivate[nDevID].cdk_core.m_psCallback->m_pfMemcpy   = (void* (*) ( void*, const void*, unsigned int ))memcpy;
		omx_mp2dec_component_Private->pAudioDecPrivate[nDevID].cdk_core.m_psCallback->m_pfMemmove  = (void* (*) ( void*, const void*, unsigned int ))memmove;
		omx_mp2dec_component_Private->pAudioDecPrivate[nDevID].cdk_core.m_psCallback->m_pfMemset   = (void  (*) ( void*, int, unsigned int ))memset;

		omx_mp2dec_component_Private->pAudioDecPrivate[nDevID].iAdecType = AUDIO_ID_MP2;
		omx_mp2dec_component_Private->pAudioDecPrivate[nDevID].iCtype = CONTAINER_TYPE_MPG;
		omx_mp2dec_component_Private->pAudioDecPrivate[nDevID].cb_function = NULL; // TCC_MP2_DEC

		omx_mp2dec_component_Private->pAudioDecPrivate[nDevID].bAudioStarted = OMX_TRUE;
		omx_mp2dec_component_Private->pAudioDecPrivate[nDevID].bAudioPaused = OMX_FALSE;
	}

	DBUG_MSG("constructor of MP2 decoder component is completed ret = %d \n", err);

	return err;

}


OMX_ERRORTYPE omx_audiodec_component_Change_MP2dec(OMX_S16 nDevID, OMX_COMPONENTTYPE *openmaxStandComp, OMX_STRING cComponentName) 
{

	OMX_ERRORTYPE err = OMX_ErrorNone;  
	omx_mp2dec_component_PrivateType* omx_mp2dec_component_Private;
	omx_base_audio_PortType *inPort,*outPort;

	if (!openmaxStandComp->pComponentPrivate)
	{
		DBUG_MSG("In %s, Error Insufficient Resources\n", __func__);
		return OMX_ErrorInsufficientResources;
	}

	omx_mp2dec_component_Private = openmaxStandComp->pComponentPrivate;

	/** parameters related to input port */
	inPort = (omx_base_audio_PortType *) omx_mp2dec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];

	strcpy(inPort->sPortParam.format.audio.cMIMEType, "audio/mpeg2");
	inPort->sPortParam.format.audio.eEncoding = OMX_AUDIO_CodingMP2;
	inPort->sAudioParam.eEncoding = OMX_AUDIO_CodingMP2;

    //Default values for MP2 audio param port
	setHeader(&omx_mp2dec_component_Private->pAudioMp2, sizeof(OMX_AUDIO_PARAM_MP2TYPE));
    omx_mp2dec_component_Private->pAudioMp2.nPortIndex = 0;
    omx_mp2dec_component_Private->pAudioMp2.nChannels = 2;
    omx_mp2dec_component_Private->pAudioMp2.nBitRate = 0;
    omx_mp2dec_component_Private->pAudioMp2.nSampleRate = 48000;
    omx_mp2dec_component_Private->pAudioMp2.eChannelMode = OMX_AUDIO_ChannelModeStereo;
	omx_mp2dec_component_Private->pAudioMp2.eFormat = OMX_AUDIO_MP3StreamFormatMP2Layer3;

	/** settings of output port audio format - pcm */
	setHeader(&omx_mp2dec_component_Private->pAudioPcmMode[nDevID], sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
	omx_mp2dec_component_Private->pAudioPcmMode[nDevID].nPortIndex = 2;
	omx_mp2dec_component_Private->pAudioPcmMode[nDevID].nChannels = 2;
	omx_mp2dec_component_Private->pAudioPcmMode[nDevID].eNumData = OMX_NumericalDataSigned;
	omx_mp2dec_component_Private->pAudioPcmMode[nDevID].eEndian = OMX_EndianLittle;
	omx_mp2dec_component_Private->pAudioPcmMode[nDevID].bInterleaved = OMX_TRUE;
	omx_mp2dec_component_Private->pAudioPcmMode[nDevID].nBitPerSample = 16;
	omx_mp2dec_component_Private->pAudioPcmMode[nDevID].nSamplingRate = 48000;
	omx_mp2dec_component_Private->pAudioPcmMode[nDevID].ePCMMode = OMX_AUDIO_PCMModeLinear;
	omx_mp2dec_component_Private->pAudioPcmMode[nDevID].eChannelMapping[0] = OMX_AUDIO_ChannelLF;
	omx_mp2dec_component_Private->pAudioPcmMode[nDevID].eChannelMapping[1] = OMX_AUDIO_ChannelRF;

	/** now it's time to know the audio coding type of the component */
	if(!strcmp(cComponentName, AUDIO_DEC_MP2_NAME))  
	{   
		 omx_mp2dec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type = OMX_AUDIO_CodingMP2;
	} 
	else if (!strcmp(cComponentName, AUDIO_DEC_BASE_NAME)) 
	{
		omx_mp2dec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type = OMX_AUDIO_CodingUnused;
	}
	else  
	{
	    // IL client specified an invalid component name
	    
		ALOGE("OMX_ErrorInvalidComponentName %s", cComponentName);
	    return OMX_ErrorInvalidComponentName;
	}

	omx_mp2dec_component_Private->pAudioDecPrivate[nDevID].iAdecType = AUDIO_ID_MP2;
	omx_mp2dec_component_Private->pAudioDecPrivate[nDevID].iCtype = CONTAINER_TYPE_MPG;
	omx_mp2dec_component_Private->pAudioDecPrivate[nDevID].cb_function = NULL; // TCC_MP2_DEC

	DBUG_MSG("change of MP2 decoder component is completed ret = %d \n", err);

	return err;
}

#ifndef      SUPPORT_AC3    
int TCC_AC3_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 )
{
    return 0;
}
#endif

OMX_ERRORTYPE omx_audiodec_component_Change_Default_AC3dec(OMX_S16 nDevID, OMX_COMPONENTTYPE *openmaxStandComp, OMX_STRING cComponentName) 
{
 OMX_ERRORTYPE err = OMX_ErrorInsufficientResources;
#ifdef SUPPORT_DDP_CODEC
    err = omx_audiodec_component_Change_DDPdec(nDevID, openmaxStandComp, AUDIO_DEC_AC3_NAME);
#else
    #ifdef      SUPPORT_AC3    
    err = omx_audiodec_component_Change_AC3dec(nDevID, openmaxStandComp, AUDIO_DEC_AC3_NAME);
    #endif
#endif
   	return err;
}

OMX_ERRORTYPE OMX_DXB_AudioDec_AC3Common_ComponentInit(OMX_COMPONENTTYPE *openmaxStandComp)
{
#ifdef SUPPORT_DDP_CODEC
    return OMX_DXB_AudioDec_DDPComponentInit(openmaxStandComp);
#else
#ifdef      SUPPORT_AC3    
    return OMX_DXB_AudioDec_AC3ComponentInit(openmaxStandComp);
#else
    return OMX_ErrorInsufficientResources;
#endif    
#endif

}

