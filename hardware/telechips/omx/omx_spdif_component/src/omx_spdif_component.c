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
#ifdef OPENMAX1_2
#include <OMX_IndexExt.h>
#endif
#include <OMX_TCC_Index.h>
#include "TCCFile.h"
#include "TCCMemory.h"

#ifdef HAVE_ANDROID_OS
#define USE_EXTERNAL_BUFFER 0
#define LOG_TAG	"OMX_TCC_SPDIF"
#include <utils/Log.h>
#include <cutils/properties.h>

#define TCC_DDP_TRANSCODEING_TO_DD

#ifdef TCC_DDP_TRANSCODEING_TO_DD
#include "cdmx.h"
#include "cmux.h"
#include "cdk.h"
#include "cdk_audio.h"
#endif

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

#include "ac3_header.h"
#include "dts_header.h"

#define SPDIF_BUFFER_SIZE	(1024 * 1024)

// SPDIF_AC3_TRUEHD Start
#define MAT_FRAME_SIZE			61424
#define TRUEHD_FRAME_OFFSET		2560
#define MAT_MIDDLE_CODE_OFFSET	-4
#define BURST_HEADER_SIZE		0x8

unsigned char TRUEHD_MAT_Buffer[61440];
int TRUEHD_Count;
// SPDIF_AC3_TRUEHD End

int DDP_remaining_length=-1;
int DDP_Count;
int DDP_buffer_filled;
#undef DDP_DEBUG_ON
#ifdef DDP_DEBUG_ON
#define DDP_DEBUG_MSG(x...)	ALOGD(x)
#else
#define DDP_DEBUG_MSG(x...)
#endif

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
//static int giAdecType;
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

OMX_ERRORTYPE OMX_ComponentInit(OMX_HANDLETYPE openmaxStandComp, OMX_STRING cCompontName, OMX_HANDLE_FUNC pHandleFunc)
{
  OMX_ERRORTYPE err = OMX_ErrorNone;

	err = omx_spdif_component_Constructor(openmaxStandComp,cCompontName,pHandleFunc);

	return err;
}

OMX_ERRORTYPE omx_spdif_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp, OMX_STRING cComponentName, OMX_HANDLE_FUNC pHandleFunc) 
{
	OMX_ERRORTYPE err = OMX_ErrorNone;  
	omx_spdif_component_PrivateType* omx_spdif_component_Private;
	omx_base_audio_PortType *inPort,*outPort;
	OMX_U32 i;

#ifdef HAVE_ANDROID_OS
	if (1)
#else
	if (!openmaxStandComp->pComponentPrivate) 
#endif
	{
		openmaxStandComp->pComponentPrivate = TCC_calloc(1, sizeof(omx_spdif_component_PrivateType));

		if(openmaxStandComp->pComponentPrivate==NULL)  
		{
			return OMX_ErrorInsufficientResources;
		}
	} 
	else 
	{
	}

	omx_spdif_component_Private = openmaxStandComp->pComponentPrivate;
	omx_spdif_component_Private->ports = NULL;

	/** we could create our own port structures here
	 * fixme maybe the base class could use a "port factory" function pointer?  
	 */
	err = omx_base_filter_Constructor(openmaxStandComp, cComponentName);

	DBUG_MSG("[DBG]  ==> constructor of spdif component is called\n");

	/* Domain specific section for the ports. */  
	/* first we set the parameter common to both formats */
	/* parameters related to input port which does not depend upon input audio format    */
	/* Allocate Ports and call port constructor. */  

	omx_spdif_component_Private->sPortTypesParam[OMX_PortDomainAudio].nStartPortNumber = 0;
	omx_spdif_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts = 2;

	if (omx_spdif_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts && !omx_spdif_component_Private->ports) 
	{
		omx_spdif_component_Private->ports = TCC_calloc(omx_spdif_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts, sizeof(omx_base_PortType *));
		if (!omx_spdif_component_Private->ports) 
		{
			return OMX_ErrorInsufficientResources;
		}
		for (i=0; i < omx_spdif_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) 
		{
			omx_spdif_component_Private->ports[i] = TCC_calloc(1, sizeof(omx_base_audio_PortType));
			if (!omx_spdif_component_Private->ports[i]) 
			{
				return OMX_ErrorInsufficientResources;
			}
		}
	}

	base_audio_port_Constructor(openmaxStandComp, &omx_spdif_component_Private->ports[0], 0, OMX_TRUE); // input
	base_audio_port_Constructor(openmaxStandComp, &omx_spdif_component_Private->ports[1], 1, OMX_FALSE); // output

	/** now it's time to know the audio coding type of the component */
	if(!strcmp(cComponentName, AUDIO_DEC_AC3_NAME))  
	{   
		omx_spdif_component_Private->audio_coding_type = OMX_AUDIO_CodingAC3;
	} 
	else if(!strcmp(cComponentName, AUDIO_DEC_DTS_NAME))  
	{   
		omx_spdif_component_Private->audio_coding_type = OMX_AUDIO_CodingDTS;
	} 
	else if (!strcmp(cComponentName, AUDIO_DEC_BASE_NAME)) 
	{
		omx_spdif_component_Private->audio_coding_type = OMX_AUDIO_CodingUnused;
	}
	else  
	{
		// IL client specified an invalid component name
		ALOGE("OMX_ErrorInvalidComponentName %s", cComponentName);
		return OMX_ErrorInvalidComponentName;
	}

	/** parameters related to input port */
	inPort = (omx_base_audio_PortType *) omx_spdif_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];

	inPort->sPortParam.nBufferSize = DEFAULT_IN_BUFFER_SIZE*2;

	if(omx_spdif_component_Private->audio_coding_type == OMX_AUDIO_CodingAC3)  
	{   
#ifndef OPENMAX1_2
		strcpy(inPort->sPortParam.format.audio.cMIMEType, "audio/ac3");
#endif
		inPort->sPortParam.format.audio.eEncoding = OMX_AUDIO_CodingAC3;
		inPort->sAudioParam.eEncoding = OMX_AUDIO_CodingAC3;

		setHeader(&omx_spdif_component_Private->pAudioAc3, sizeof(OMX_AUDIO_PARAM_AC3TYPE));

		omx_spdif_component_Private->pAudioAc3.nPortIndex = 0;
		omx_spdif_component_Private->pAudioAc3.nChannels = 2;
		omx_spdif_component_Private->pAudioAc3.nBitRate = 28000;
#ifdef OPENMAX1_2
		omx_spdif_component_Private->pAudioAc3.nSampleRate = 44100;
#else
		omx_spdif_component_Private->pAudioAc3.nSamplingRate = 44100;
#endif
	} 
	else if(omx_spdif_component_Private->audio_coding_type == OMX_AUDIO_CodingDTS)  
	{   
#ifndef OPENMAX1_2
		strcpy(inPort->sPortParam.format.audio.cMIMEType, "audio/dts");
#endif
		inPort->sPortParam.format.audio.eEncoding = OMX_AUDIO_CodingDTS;
		inPort->sAudioParam.eEncoding = OMX_AUDIO_CodingDTS;

		setHeader(&omx_spdif_component_Private->pAudioDts, sizeof(OMX_AUDIO_PARAM_DTSTYPE));

		omx_spdif_component_Private->pAudioDts.nPortIndex = 0;
		omx_spdif_component_Private->pAudioDts.nChannels = 2;
		omx_spdif_component_Private->pAudioDts.nBitRate = 28000;
#ifdef OPENMAX1_2
		omx_spdif_component_Private->pAudioDts.nSampleRate = 44100;
#else
		omx_spdif_component_Private->pAudioDts.nSamplingRate = 44100;
		omx_spdif_component_Private->pAudioDts.eFormat = OMX_AUDIO_DTSFormatDTS;
#endif
	} 

	/** parameters related to output port */
	outPort = (omx_base_audio_PortType *) omx_spdif_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
	outPort->sPortParam.format.audio.eEncoding = OMX_AUDIO_CodingPCM;
	outPort->sPortParam.nBufferSize = AUDIO_DEC_OUT_BUFFER_SIZE;
	outPort->sAudioParam.eEncoding = OMX_AUDIO_CodingPCM;

	/** settings of output port audio format - pcm */
	setHeader(&omx_spdif_component_Private->pAudioPcmMode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
	omx_spdif_component_Private->pAudioPcmMode.nPortIndex = 1;
	omx_spdif_component_Private->pAudioPcmMode.nChannels = 2; //fixed tcc chanel count
	omx_spdif_component_Private->pAudioPcmMode.eNumData = OMX_NumericalDataSigned;
	omx_spdif_component_Private->pAudioPcmMode.eEndian = OMX_EndianLittle;
	omx_spdif_component_Private->pAudioPcmMode.bInterleaved = OMX_TRUE;
	omx_spdif_component_Private->pAudioPcmMode.nBitPerSample = 16;
	omx_spdif_component_Private->pAudioPcmMode.nSamplingRate = 44100; //48000;
	omx_spdif_component_Private->pAudioPcmMode.ePCMMode = OMX_AUDIO_PCMModeLinear;
	omx_spdif_component_Private->pAudioPcmMode.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
	omx_spdif_component_Private->pAudioPcmMode.eChannelMapping[1] = OMX_AUDIO_ChannelRF;

	/** general configuration irrespective of any audio formats */
	/**  setting values of other fields of omx_maddec_component_Private structure */

	omx_spdif_component_Private->BufferMgmtCallback = omx_spdif_component_BufferMgmtCallback;
	omx_spdif_component_Private->messageHandler = omx_spdif_component_MessageHandler;
	omx_spdif_component_Private->destructor = omx_spdif_component_Destructor;
	openmaxStandComp->SetParameter = omx_spdif_component_SetParameter;
	openmaxStandComp->GetParameter = omx_spdif_component_GetParameter;
	openmaxStandComp->GetExtensionIndex = omx_spdif_component_GetExtensionIndex;
	omx_spdif_component_Private->pExternalDec = pHandleFunc;

	omx_spdif_component_Private->decode_ready = OMX_FALSE;	

	if (omx_spdif_component_Private->spdif_pBuffer == NULL)
	{
		omx_spdif_component_Private->spdif_pBuffer = (OMX_U8*)malloc(SPDIF_BUFFER_SIZE);
	}

	DBUG_MSG("[DBG]  ==> constructor of SPDIF component is completed ret = %d \n", err);
	return err;
}

// library init function
OMX_ERRORTYPE omx_spdif_component_LibInit(omx_spdif_component_PrivateType* omx_spdif_component_Private) 
{
	DBUG_MSG("In %s\n", __func__);
	omx_spdif_component_Private->isNewBuffer = 1;

	return OMX_ErrorNone;;
}

// library de-init function
OMX_ERRORTYPE omx_spdif_component_LibDeinit(omx_spdif_component_PrivateType* omx_spdif_component_Private) 
{
	DBUG_MSG("In %s\n", __func__);
	omx_spdif_component_Private->decode_ready = OMX_FALSE;

	spdif_parse_deinit();

#ifdef TCC_DDP_TRANSCODEING_TO_DD	
	if(omx_spdif_component_Private->iSPDIFMode == SPDIF_DDP_TO_DD)
	CloseDCV(omx_spdif_component_Private,&gsCdkCore, &gsADec);
#endif	

	return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_spdif_component_MessageHandler(OMX_COMPONENTTYPE* openmaxStandComp, internalRequestMessageType *message)  
{
	omx_spdif_component_PrivateType* omx_spdif_component_Private = (omx_spdif_component_PrivateType*)openmaxStandComp->pComponentPrivate;  
	OMX_ERRORTYPE err;
	OMX_STATETYPE eCurrentState = omx_spdif_component_Private->state;
	DBUG_MSG("In %s\n", __func__);

	/** Execute the base message handling */
	err = omx_base_component_MessageHandler(openmaxStandComp, message);

	if (message->messageType == OMX_CommandStateSet){ 
		if ((message->messageParam == OMX_StateExecuting) && (eCurrentState == OMX_StateIdle)) {
			err = omx_spdif_component_LibInit(omx_spdif_component_Private);
			if(err!=OMX_ErrorNone) { 
				ALOGE("In %s SPDIF component library Init Failed Error=%x\n",__func__,err); 
				return err;
			}
		} else if ((message->messageParam == OMX_StateIdle) && (eCurrentState == OMX_StateExecuting || eCurrentState == OMX_StatePause)) {
			err = omx_spdif_component_LibDeinit(omx_spdif_component_Private);
			if(err!=OMX_ErrorNone) { 
				ALOGE("In %s SPDIF component library Deinit Failed Error=%x\n",__func__,err); 
				return err;
			}
		}
	}
	return err;  

}

OMX_ERRORTYPE omx_spdif_component_GetExtensionIndex(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_STRING cParameterName,
  OMX_OUT OMX_INDEXTYPE* pIndexType)
{    

	DBUG_MSG("In  %s \n",__func__);
	if(strcmp(cParameterName,TCC_AUDIO_FILE_OPEN_STRING) == 0) {
		*pIndexType = OMX_IndexVendorParamFileOpen;  
	} else if(strcmp(cParameterName,TCC_AUDIO_POWERSPECTUM_STRING) == 0){
		*pIndexType = OMX_IndexVendorConfigPowerSpectrum;  
	} else if(strcmp(cParameterName,TCC_AUDIO_MEDIA_INFO_STRING) == 0){
		*pIndexType = OMX_IndexVendorParamMediaInfo;
	}else if(strcmp(cParameterName,TCC_AUDIO_ENERGYVOLUME_STRING) == 0){
		*pIndexType = OMX_IndexVendorConfigEnergyVolume;
	}else{
		return OMX_ErrorBadParameter;
	}

  	return OMX_ErrorNone;  
}

/** The destructor */
OMX_ERRORTYPE omx_spdif_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp) 
{
	omx_spdif_component_PrivateType* omx_spdif_component_Private = openmaxStandComp->pComponentPrivate;
	OMX_U32 i;

	#ifdef TCC_DDP_TRANSCODEING_TO_DD
	if (gptempPCMbuffer )
	{
		free(gptempPCMbuffer);
		gptempPCMbuffer = NULL;
	}
	#endif

	if (omx_spdif_component_Private->spdif_pBuffer)
	{
		free(omx_spdif_component_Private->spdif_pBuffer);
		omx_spdif_component_Private->spdif_pBuffer = NULL;
	}

	/* frees port/s */
	if (omx_spdif_component_Private->ports) 
	{
		for (i=0; i < omx_spdif_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) 
		{
			if(omx_spdif_component_Private->ports[i])
				omx_spdif_component_Private->ports[i]->PortDestructor(omx_spdif_component_Private->ports[i]);
		}

		TCC_free(omx_spdif_component_Private->ports);
		omx_spdif_component_Private->ports=NULL;
	}

	DBUG_MSG("Destructor of the SPDIF component is called\n");

	omx_base_filter_Destructor(openmaxStandComp);

	return OMX_ErrorNone;

}

/** this function sets the parameter values regarding audio format & index */
OMX_ERRORTYPE omx_spdif_component_SetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_IN  OMX_PTR ComponentParameterStructure)
{

	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_AUDIO_PARAM_PORTFORMATTYPE *pAudioPortFormat;
	OMX_AUDIO_PARAM_PCMMODETYPE* pAudioPcmMode;
	OMX_PARAM_COMPONENTROLETYPE * pComponentRole;
#ifndef OPENMAX1_2
	OMX_AUDIO_CONFIG_INFOTYPE *info;
#endif

	omx_base_audio_PortType *port;
	OMX_U32 portIndex;

	/* Check which structure we are being fed and make control its header */
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_spdif_component_PrivateType* omx_spdif_component_Private = openmaxStandComp->pComponentPrivate;

	OMX_AUDIO_PARAM_AC3TYPE * pAudioAc3;
	OMX_AUDIO_PARAM_DTSTYPE * pAudioDts;
	
	if (ComponentParameterStructure == NULL) 
	{
		return OMX_ErrorBadParameter;
	}

	DBUG_MSG("   Setting parameter 0x%x\n", nParamIndex);

	switch(nParamIndex) 
	{
		case OMX_IndexParamAudioPortFormat:
			pAudioPortFormat = (OMX_AUDIO_PARAM_PORTFORMATTYPE*)ComponentParameterStructure;
			portIndex = pAudioPortFormat->nPortIndex;
			/*Check Structure Header and verify component state*/
			err = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pAudioPortFormat, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
			if(err!=OMX_ErrorNone)
			{ 
				DBUG_MSG("In %s Parameter Check Error=%x\n",__func__,err); 
				break;
			}
			if (portIndex <= 1) 
			{
				port = (omx_base_audio_PortType *) omx_spdif_component_Private->ports[portIndex];
				memcpy(&port->sAudioParam, pAudioPortFormat, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
			} 
			else
			{
				err = OMX_ErrorBadPortIndex;
			}
		break;

		case OMX_IndexParamAudioPcm:
			pAudioPcmMode = (OMX_AUDIO_PARAM_PCMMODETYPE*)ComponentParameterStructure;
			portIndex = pAudioPcmMode->nPortIndex;
			
			pComponentRole = (OMX_PARAM_COMPONENTROLETYPE*)ComponentParameterStructure;
		
			/*Check Structure Header and verify component state*/
			err = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pAudioPcmMode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
			if(err!=OMX_ErrorNone)
			{ 
				DBUG_MSG("In %s Parameter Check Error=%x\n",__func__,err); 
				break;
			}

			ALOGI("@@@@ :: nChannels = %d, nBitPerSample = %d, nSamplingRate = %d", pAudioPcmMode->nChannels, pAudioPcmMode->nBitPerSample, pAudioPcmMode->nSamplingRate);
			memcpy(&omx_spdif_component_Private->pAudioPcmMode, pAudioPcmMode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));          
		break;

		case OMX_IndexParamStandardComponentRole:
			pComponentRole = (OMX_PARAM_COMPONENTROLETYPE*)ComponentParameterStructure;
			if (!strcmp( (char*) pComponentRole->cRole, AUDIO_DEC_AC3_ROLE)) 
			{
				omx_spdif_component_Private->audio_coding_type = OMX_AUDIO_CodingAC3;
			}  
			else if (!strcmp( (char*) pComponentRole->cRole, AUDIO_DEC_DTS_ROLE)) 
			{
				omx_spdif_component_Private->audio_coding_type = OMX_AUDIO_CodingDTS;
			}  
			else
			{
				err = OMX_ErrorBadParameter;
			}
		break;
		
		// --------------------------------------------------------------------------------
		// codec specific parameters -------------------------------------------------------

#ifdef OPENMAX1_2
		case OMX_IndexParamAudioAc3:
#else
		case OMX_IndexParamAudioAC3:
#endif
			pAudioAc3 = (OMX_AUDIO_PARAM_AC3TYPE*) ComponentParameterStructure;
			portIndex = pAudioAc3->nPortIndex;
			err = omx_base_component_ParameterSanityCheck(hComponent,portIndex,pAudioAc3,sizeof(OMX_AUDIO_PARAM_AC3TYPE));
	
			if(err!=OMX_ErrorNone)
			{ 
				DBUG_MSG("[DBG_AC3]  ==> In %s Parameter Check Error=%x\n",__func__,err); 
				break;
			}
			if (pAudioAc3->nPortIndex == 0) 
			{
				memcpy(&omx_spdif_component_Private->pAudioAc3, pAudioAc3, sizeof(OMX_AUDIO_PARAM_AC3TYPE));
				OMX_AUDIO_PARAM_PCMMODETYPE *mode = &omx_spdif_component_Private->pAudioPcmMode;
#ifdef OPENMAX1_2
				mode->nSamplingRate = pAudioAc3->nSampleRate;
#else
				mode->nSamplingRate = pAudioAc3->nSamplingRate;
#endif
			}  
			else
			{
				err = OMX_ErrorBadPortIndex;
			}			
		break;
		
#ifdef OPENMAX1_2
		case OMX_IndexParamAudioDts:
#else
		case OMX_IndexParamAudioDTS:
#endif
			pAudioDts = (OMX_AUDIO_PARAM_DTSTYPE*) ComponentParameterStructure;
			portIndex = pAudioDts->nPortIndex;
			err = omx_base_component_ParameterSanityCheck(hComponent,portIndex,pAudioDts,sizeof(OMX_AUDIO_PARAM_DTSTYPE));
			
			if(err!=OMX_ErrorNone)
			{ 
				DBUG_MSG("In %s Parameter Check Error=%x\n",__func__,err); 
				break;
			}
			if (pAudioDts->nPortIndex == 0) 
			{
				memcpy(&omx_spdif_component_Private->pAudioDts, pAudioDts, sizeof(OMX_AUDIO_PARAM_DTSTYPE));
				OMX_AUDIO_PARAM_PCMMODETYPE *mode = &omx_spdif_component_Private->pAudioPcmMode;
#ifdef OPENMAX1_2
				mode->nSamplingRate = pAudioDts->nSampleRate;
#else
				mode->nSamplingRate = pAudioDts->nSamplingRate;
#endif
				if (pAudioDts->nChannels == 1) {
					mode->nChannels = 1;
				}
			}  
			else
			{
				err = OMX_ErrorBadPortIndex;
			}			
		break;

		// codec specific ---------------------------------------------------------------
		// ------------------------------------------------------------------------------

		case OMX_IndexVendorAudioExtraData:
		{
			OMX_VENDOR_EXTRADATATYPE* pExtradata;	
			pExtradata = (OMX_VENDOR_EXTRADATATYPE*)ComponentParameterStructure;
			
			if (pExtradata->nPortIndex <= 1) {
				/** copy the extradata in the codec context private structure */
////				memcpy(&omx_mp3dec_component_Private->audioinfo, pExtradata->pData, sizeof(rm_audio_info));
			} else {
				err = OMX_ErrorBadPortIndex;
			}
		}
		break;

#ifndef OPENMAX1_2
		case OMX_IndexVendorParamMediaInfo:
			info = (OMX_AUDIO_CONFIG_INFOTYPE*) ComponentParameterStructure;
			//omx_mp3dec_component_Private->pAudioMp3.nChannels = info->nChannels;
			//omx_mp3dec_component_Private->pAudioMp3.nBitRate = info->nBitPerSample;
			//omx_mp3dec_component_Private->pAudioMp3.nSampleRate = info->nSamplingRate;
			break;
#endif
		

		default: /*Call the base component function*/
			return omx_base_component_SetParameter(hComponent, nParamIndex, ComponentParameterStructure);

	}

	if(err != OMX_ErrorNone)
		ALOGE("ERROR %s :: nParamIndex = 0x%x, error(0x%x)", __func__, nParamIndex, err);

	return err;
}  

/** this function gets the parameters regarding audio formats and index */
OMX_ERRORTYPE omx_spdif_component_GetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_INOUT OMX_PTR ComponentParameterStructure)
{

	OMX_AUDIO_PARAM_PORTFORMATTYPE *pAudioPortFormat;  
	OMX_AUDIO_PARAM_PCMMODETYPE *pAudioPcmMode;
	OMX_PARAM_COMPONENTROLETYPE * pComponentRole;
#ifndef OPENMAX1_2
    OMX_AUDIO_CONFIG_INFOTYPE *info;
#endif
	omx_base_audio_PortType *port;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	int i;

	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_spdif_component_PrivateType* omx_spdif_component_Private = openmaxStandComp->pComponentPrivate;

	OMX_AUDIO_PARAM_AC3TYPE * pAudioAc3;
	OMX_AUDIO_PARAM_DTSTYPE * pAudioDts;

	if (ComponentParameterStructure == NULL)
	{
		return OMX_ErrorBadParameter;
	}
	DBUG_MSG("   Getting parameter 0x%x\n", nParamIndex);
	/* Check which structure we are being fed and fill its header */

	switch(nParamIndex) 
	{
		case OMX_IndexParamAudioInit:
			if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_PORT_PARAM_TYPE))) != OMX_ErrorNone) 
			{ 
			  break;
			}
			memcpy(ComponentParameterStructure, &omx_spdif_component_Private->sPortTypesParam, sizeof(OMX_PORT_PARAM_TYPE));
			break;    

		case OMX_IndexParamAudioPortFormat:
			pAudioPortFormat = (OMX_AUDIO_PARAM_PORTFORMATTYPE*)ComponentParameterStructure;
			if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE))) != OMX_ErrorNone) 
			{ 
				break;
			}
			if (pAudioPortFormat->nPortIndex <= 1) 
			{
				port = (omx_base_audio_PortType *)omx_spdif_component_Private->ports[pAudioPortFormat->nPortIndex];
				memcpy(pAudioPortFormat, &port->sAudioParam, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
			} 
			else 
			{
				return OMX_ErrorBadPortIndex;
			}
			break;    

		case OMX_IndexParamAudioPcm:
			pAudioPcmMode = (OMX_AUDIO_PARAM_PCMMODETYPE*)ComponentParameterStructure;
			if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE))) != OMX_ErrorNone) 
			{ 
				break;
			}
			if (pAudioPcmMode->nPortIndex > 1) 
			{
				return OMX_ErrorBadPortIndex;
			}
			
			memcpy(pAudioPcmMode, &omx_spdif_component_Private->pAudioPcmMode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
			break;
		
		// --------------------------------------------------------------------------------
		// codec specific parameters -------------------------------------------------------

#ifdef OPENMAX1_2
		case OMX_IndexParamAudioAc3: // parameter read(channel,samplerate etc...)
#else
		case OMX_IndexParamAudioAC3: // parameter read(channel,samplerate etc...)
#endif
			pAudioAc3 = (OMX_AUDIO_PARAM_AC3TYPE*)ComponentParameterStructure;
			if (pAudioAc3->nPortIndex != 0) 
			{
				return OMX_ErrorBadPortIndex;
			}
			if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_AC3TYPE))) != OMX_ErrorNone) 
			{ 
				break;
			}
			memcpy(pAudioAc3, &omx_spdif_component_Private->pAudioAc3, sizeof(OMX_AUDIO_PARAM_AC3TYPE));
			break;
		
#ifdef OPENMAX1_2
		case OMX_IndexParamAudioDts:
#else
		case OMX_IndexParamAudioDTS:
#endif
			pAudioDts = (OMX_AUDIO_PARAM_DTSTYPE*)ComponentParameterStructure;
			if (pAudioDts->nPortIndex != 0) 
			{
				return OMX_ErrorBadPortIndex;
			}
			if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_DTSTYPE))) != OMX_ErrorNone) 
			{ 
				break;
			}
			memcpy(pAudioDts, &omx_spdif_component_Private->pAudioDts, sizeof(OMX_AUDIO_PARAM_DTSTYPE));
			break;
		
		// codec specific ---------------------------------------------------------------
		// ------------------------------------------------------------------------------

		case OMX_IndexParamStandardComponentRole:
			pComponentRole = (OMX_PARAM_COMPONENTROLETYPE*)ComponentParameterStructure;
			if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_PARAM_COMPONENTROLETYPE))) != OMX_ErrorNone) 
			{ 
				break;
			}
			
			if (omx_spdif_component_Private->audio_coding_type == OMX_AUDIO_CodingAC3) 
			{
				strcpy( (char*) pComponentRole->cRole, AUDIO_DEC_AC3_ROLE);
			}
			else if (omx_spdif_component_Private->audio_coding_type == OMX_AUDIO_CodingDTS) 
			{
				strcpy( (char*) pComponentRole->cRole, AUDIO_DEC_DTS_ROLE);
			}
			else
			{
				strcpy( (char*) pComponentRole->cRole,"\0");;
			}
			break;

#ifdef HAVE_ANDROID_OS
			case PV_OMX_COMPONENT_CAPABILITY_TYPE_INDEX:
			{
				PV_OMXComponentCapabilityFlagsType *pCap_flags =
					(PV_OMXComponentCapabilityFlagsType *) ComponentParameterStructure;
				if (NULL == pCap_flags) {
					return OMX_ErrorBadParameter;
				}
		
				memset(pCap_flags, 0x00, sizeof(PV_OMXComponentCapabilityFlagsType));
				pCap_flags->iIsOMXComponentMultiThreaded = OMX_TRUE;
				pCap_flags->iOMXComponentSupportsExternalInputBufferAlloc 	= OMX_TRUE; //ZzaU::to use MovableInputBuffers
				pCap_flags->iOMXComponentSupportsExternalOutputBufferAlloc 	= OMX_FALSE;
				pCap_flags->iOMXComponentSupportsMovableInputBuffers		= OMX_TRUE;
				pCap_flags->iOMXComponentSupportsPartialFrames				= OMX_TRUE; //ZzaU::PartialFrames must enable to use MovableInputBuffers.
//				pCap_flags->iOMXComponentUsesNALStartCodes					= OMX_FALSE;
//				pCap_flags->iOMXComponentUsesFullAVCFrames					= OMX_FALSE;			
//				pCap_flags->iOMXComponentCanHandleIncompleteFrames			= OMX_FALSE;
			}
			break;
#endif

		default: /*Call the base component function*/
			return omx_base_component_GetParameter(hComponent, nParamIndex, ComponentParameterStructure);

	}

	return OMX_ErrorNone;

}

#ifdef TCC_DDP_TRANSCODEING_TO_DD
static void CloseDCV( omx_spdif_component_PrivateType* pPrivate, cdk_core_t* pCdk , ADEC_VARS* pgsADec)
{
	if(pCdk->m_iAudioHandle != 0 && pPrivate->pExternalDec)
	{
		pPrivate->pExternalDec(AUDIO_CLOSE, &pCdk->m_iAudioHandle, NULL, NULL);
		pCdk->m_iAudioHandle = 0;
	}
	
	if(pgsADec->gpucAudioBuffer)
	{
		pCdk->m_psCallback->m_pfFree(pgsADec->gpucAudioBuffer);
		pgsADec->gpucAudioBuffer = NULL;
	}
	
	DDP_DEBUG_MSG("CloseDCV: success");
	return;
}

static OMX_S32 InitDCV( omx_spdif_component_PrivateType* pPrivate, cdk_core_t* pCdk, cdmx_info_t *pCdmxInfo, ADEC_VARS* pgsADec)
{
	int ret = CDK_ERR_NONE;
	
	/* audio common */
	pgsADec->gsADecInput.m_eSampleRate = pCdmxInfo->m_sAudioInfo.m_iSamplePerSec;
	pgsADec->gsADecInput.m_uiNumberOfChannel = pCdmxInfo->m_sAudioInfo.m_iChannels;
	pgsADec->gsADecInput.m_uiBitsPerSample = pCdmxInfo->m_sAudioInfo.m_iBitsPerSample;
	pgsADec->gsADecOutput.m_uiBitsPerSample = 16;
	
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
			DDP_DEBUG_MSG("InitDCV: pgsADec->gpucAudioBuffer allocation fail\n");
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

	DDP_DEBUG_MSG("InitDCV: AUDIO_ID_DDP m_iOutPutMode %d", pgsADec->gsADecInit.m_unAudioCodecParams.m_unDDPDec.m_iOutPutMode);

	if(pPrivate->pExternalDec)
	ret = pPrivate->pExternalDec(AUDIO_INIT, &pCdk->m_iAudioHandle, &pgsADec->gsADecInit, NULL);
	
	if(ret != TCAS_SUCCESS)
	{
		DDP_DEBUG_MSG("InitDCV: init failed %d\n", ret);
		CloseDCV(pPrivate,pCdk, pgsADec);
		return ret;
	}
	DDP_DEBUG_MSG("InitDCV: init success");
	return ret;
}

static OMX_S32 ProcessDCV(omx_spdif_component_PrivateType* pPrivate, cdk_core_t* pCdk, cdmx_info_t *pCdmxInfo, cdmx_output_t *pCdmxOut, int iSeekFlag, ADEC_VARS* pgsADec)
{
	int ret = CDK_ERR_NONE;

	pCdmxOut->m_iDecodedSamples = 0;
			
	if(iSeekFlag != 0)
	{
		pgsADec->gsADecInput.m_iStreamLength = 0;
		if(pPrivate->pExternalDec)
		pPrivate->pExternalDec(AUDIO_FLUSH, &pCdk->m_iAudioHandle, NULL, NULL);
	}
	
	if(pgsADec->gsADecInput.m_iStreamLength < 0)
		pgsADec->gsADecInput.m_iStreamLength = 0;

	if((pgsADec->gsADecInput.m_iStreamLength + pCdmxOut->m_iPacketSize) > (int)pgsADec->guiAudioInBufferSize)
	{
		DDP_DEBUG_MSG("ProcessDCV: Realloc pgsADec->gpucAudioBuffer\n");
		pgsADec->guiAudioInBufferSize = pgsADec->gsADecInput.m_iStreamLength + pCdmxOut->m_iPacketSize;
		pgsADec->gpucAudioBuffer = pgsADec->gsADecInit.m_pfRealloc(pgsADec->gpucAudioBuffer, pgsADec->guiAudioInBufferSize);
		if(pgsADec->gpucAudioBuffer == NULL)
		{
			DDP_DEBUG_MSG("ProcessDCV: pgsADec->gpucAudioBuffer re-allocation fail\n");
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

		if(pPrivate->pExternalDec)
		ret = pPrivate->pExternalDec(AUDIO_DECODE, &pCdk->m_iAudioHandle, &pgsADec->gsADecInput, &pgsADec->gsADecOutput);

		if(ret != TCAS_SUCCESS)
		{
			pgsADec->giAudioErrorCount++;
			if(pgsADec->giAudioErrorCount > AUDIO_ERROR_THRESHOLD)
			{
				DDP_DEBUG_MSG("ProcessDCV: too many errors!\n");
				return ret;	
			}
				
			switch(ret)
			{
			case TCAS_ERROR_SKIP_FRAME:
			case TCAS_ERROR_DDP_NOT_SUPPORT_FRAME:	
				pgsADec->giAudioErrorCount = 0;
				break;

			case TCAS_ERROR_MORE_DATA:			// need more data		
				DDP_DEBUG_MSG("ProcessDCV: need more data, bytes left: %d\n", pgsADec->gsADecInput.m_iStreamLength);
				if(pgsADec->gsADecInput.m_iStreamLength > 0)
				{
					pgsADec->gsADecInit.m_pfMemmove(pgsADec->gpucAudioBuffer, pgsADec->gsADecInput.m_pcStream, pgsADec->gsADecInput.m_iStreamLength);	
				}	
				return 1;	//need more data
							
			default:
				DDP_DEBUG_MSG("ProcessDCV: error %d\n", ret);	
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
			DDP_DEBUG_MSG("ProcessDCV: m_iDecodedSamples %d", pCdmxOut->m_iDecodedSamples);
		}
	}

	if(pgsADec->gsADecInput.m_iStreamLength > 0)
		pgsADec->gsADecInit.m_pfMemmove(pgsADec->gpucAudioBuffer, pgsADec->gsADecInput.m_pcStream, pgsADec->gsADecInput.m_iStreamLength);
	
	if(ret == TCAS_ERROR_DDP_NOT_SUPPORT_FRAME)
		return 1;	/* check ddp dependent frame */

	if(ret != TCAS_SUCCESS)
		return 2;
		
	return ret;
}

static int init_ddp_to_dd(omx_spdif_component_PrivateType* pPrivate, OMX_BUFFERHEADERTYPE* inputbuffer )
{
	int lCtype;
	
	pPrivate->decode_ready = OMX_FALSE;
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
	
	DDP_DEBUG_MSG("init_ddp_to_dd. AudioInfo Size = %d", sizeof(gsCdmxInfo.m_sAudioInfo));
	
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
	pPrivate->decode_ready  = OMX_TRUE;
	pPrivate->isNewBuffer = 1;

	return 1;
}


static int convert_ddp_to_dd(omx_spdif_component_PrivateType* pPrivate, OMX_BUFFERHEADERTYPE* inputbuffer ,OMX_S16 seekFlag )
{
	OMX_S32 ret	= 0;
	OMX_S16 iSeekFlag = seekFlag;
	
	if(pPrivate->iSPDIFMode == SPDIF_DDP_TO_DD)
	{
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
			if(gsADec.gsADecOutput.m_pvExtraInfo )
			{
				int *extradata = (int*)(gsADec.gsADecOutput.m_pvExtraInfo);
				int ddstream_length;
				unsigned char *p_ddstream;
				
				DDP_DEBUG_MSG(" m_pvExtraInfo ret %d converted length %d ",ret,*(extradata + 1));
				p_ddstream = *extradata;
				TRUEHD_Count = ddstream_length = *(extradata + 1);

				memcpy(TRUEHD_MAT_Buffer, p_ddstream, ddstream_length);
			}
			
			if(TRUEHD_Count == 0)
				return DDP_TO_DD_NEED_MORE_DATA;
			
			DDP_DEBUG_MSG("DEC Success. ret = %d, output size = %d", ret,TRUEHD_Count);
			return DDP_TO_DD_OK;
		}
		else
		{
			ALOGE( "cdk_audio_dec_run error: %ld", ret );
			return DDP_TO_DD_ERROR;
		}
	}
	else{
		return DDP_TO_DD_NONE;
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
				
				DDP_DEBUG_MSG("found  Indefendent count %d foundPayload %d nFilledLen %d ",count,foundPayload,nFilledLen);
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
				DDP_DEBUG_MSG("found another payload count(%d) foundPayload (%d)  ",count,foundPayload);
				return count;
			}
			
			// check the frame size to check improper sysn info
			length = ((pucInputBuf[count+2] & 0x03) << 8) |(pucInputBuf[count+3] & 0xff);
			DDP_DEBUG_MSG("found  Sync Info count %d foundPayload %d length %d  ",count,foundPayload,length);
			
			count += (length *2);

			if(foundDefendent && count > nFilledLen)
			{
				DDP_remaining_length = count-nFilledLen+2;
				DDP_DEBUG_MSG("remaining part of this frame will be in next demuxer's packet (%d)>(%d) remaing (%d) ",count,nFilledLen,DDP_remaining_length);
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
	int isDDPConverted = 0;
	int mat_code_length = 0;	// SPDIF_AC3_TRUEHD
	
	outputbuffer->nFilledLen = 0;
	outputbuffer->nOffset = 0;

	DBUG_MSG("BufferMgmtCallback IN inLen = %u, Flags = 0x%x, Timestamp = %lld", inputbuffer->nFilledLen, inputbuffer->nFlags, inputbuffer->nTimeStamp);
	if((inputbuffer->nFlags & OMX_BUFFERFLAG_CODECCONFIG) && pPrivate->decode_ready == OMX_FALSE)
	{
		spdif_parse_init();

		pPrivate->spdif_nFilledLength = 0;
		pPrivate->spdif_nConsumedLength = 0;

		pPrivate->decode_ready  = OMX_TRUE;
		pPrivate->isNewBuffer = 1;
		outputbuffer->nFilledLen = 0;
		inputbuffer->nFilledLen = 0;

		char value[PROPERTY_VALUE_MAX];
		property_get("persist.sys.spdif_setting", value, "");
		if (!strcmp(value, "4"))
		{
			ALOGD("SPDIF HD mode");
			pPrivate->iSPDIFMode = SPDIF_MODE_HD;

			
			if( (pPrivate->audio_coding_type == OMX_AUDIO_CodingAC3 && pPrivate->pAudioAc3.eFormat == OMX_AUDIO_AC3FormatAC3)
				||(pPrivate->audio_coding_type == OMX_AUDIO_CodingDTS && pPrivate->pAudioDts.eFormat == OMX_AUDIO_DTSFormatDTS)){
				pPrivate->iSPDIFMode = SPDIF_MODE_NORMAL;
				property_set("tcc.hdmi.audio_type", "2");	// AudioOutput update Planet 20121120
				ALOGI("HDMI HBR Audio Type 2");
				//ALOGD("AC3 and DTS can't pass through HD mode");
			}else if(pPrivate->audio_coding_type == OMX_AUDIO_CodingAC3 && pPrivate->pAudioAc3.eFormat == OMX_AUDIO_AC3FormatDDP){
				ALOGD("DDP HD mode passthru ");
				property_set("tcc.hdmi.audio_type", "1");
				DDP_remaining_length = -1;
				DDP_buffer_filled =0;
				DDP_Count =0;
			}
			else{
				property_set("tcc.hdmi.audio_type", "0");
			}
		}
#ifdef TCC_DDP_TRANSCODEING_TO_DD
		else if (!strcmp(value, "2")){
			ALOGD("SPDIF normal mode");
			pPrivate->iSPDIFMode = SPDIF_MODE_NORMAL;
			
			char checkDDP[PROPERTY_VALUE_MAX];
			property_get("tcc.audio.support.ddp", checkDDP, "0");
			
			if(pPrivate->audio_coding_type == OMX_AUDIO_CodingAC3 && pPrivate->pAudioAc3.eFormat == OMX_AUDIO_AC3FormatDDP
				&& atoi(checkDDP) && pPrivate->pExternalDec)
			{
				if(init_ddp_to_dd(pPrivate,inputbuffer)){
					ALOGD("SPDIF DDP to DD mode");
					pPrivate->iSPDIFMode = SPDIF_DDP_TO_DD;
				}
			}
		}
#endif
		else
		{
			ALOGD("SPDIF normal mode");
			pPrivate->iSPDIFMode = SPDIF_MODE_NORMAL;
		}

		DBUG_MSG("Audio DEC initialized.");
		return;
	}

	if(pPrivate->decode_ready == OMX_FALSE)
	{
		DBUG_MSG(" Audio Decoder not Initialized!!");
		// to skip all audio data
		inputbuffer->nFilledLen = 0;
		pPrivate->isNewBuffer = 1;
		return;
	}

	if(inputbuffer->nFlags & OMX_BUFFERFLAG_SYNCFRAME)
	{
		iSeekFlag = 1;
		pPrivate->iStartTS = inputbuffer->nTimeStamp;
		pPrivate->iSamples = 0;
	}

	#ifdef TCC_DDP_TRANSCODEING_TO_DD
	isDDPConverted = convert_ddp_to_dd(pPrivate,inputbuffer,iSeekFlag);
	#endif

	if (pPrivate->audio_coding_type == OMX_AUDIO_CodingAC3 || pPrivate->audio_coding_type == OMX_AUDIO_CodingDTS) 
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
				memset(&pPrivate->spdif_info, 0, sizeof(spdif_header_info_s));
				pPrivate->spdif_nFilledLength = 0;
				pPrivate->spdif_nConsumedLength = 0;
			}

			#ifdef TCC_DDP_TRANSCODEING_TO_DD
			if(pPrivate->iSPDIFMode == SPDIF_DDP_TO_DD){
				DDP_DEBUG_MSG("isDDPConverted isDDPConverted = %d TRUEHD_Count %d spdif_nFilledLength %d ", isDDPConverted,TRUEHD_Count,pPrivate->spdif_nFilledLength);
				switch (isDDPConverted)
				{
				case DDP_TO_DD_NONE:
					if ((pPrivate->spdif_nFilledLength + inputbuffer->nFilledLen) <= SPDIF_BUFFER_SIZE)
					{
						memcpy(&pPrivate->spdif_pBuffer[pPrivate->spdif_nFilledLength], inputbuffer->pBuffer, inputbuffer->nFilledLen);
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
						ALOGE("DDP-DD :: input data length is too many to copy !!");
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
					break;
				}
			}
			else{// not ddp to dd mode
				if ((pPrivate->spdif_nFilledLength + inputbuffer->nFilledLen) <= SPDIF_BUFFER_SIZE)
				{
					memcpy(&pPrivate->spdif_pBuffer[pPrivate->spdif_nFilledLength], inputbuffer->pBuffer, inputbuffer->nFilledLen);
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
				memcpy(&pPrivate->spdif_pBuffer[pPrivate->spdif_nFilledLength], inputbuffer->pBuffer, inputbuffer->nFilledLen);
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
			if (pPrivate->audio_coding_type == OMX_AUDIO_CodingAC3)
			{
				// SPDIF_AC3_TRUEHD Start
				if(pPrivate->pAudioAc3.eFormat == OMX_AUDIO_AC3FormatTRUEHD && pPrivate->iSPDIFMode == SPDIF_MODE_HD )
				{
					found_header = 1;
				}
				// SPDIF_AC3_TRUEHD End
				else if(pPrivate->pAudioAc3.eFormat == OMX_AUDIO_AC3FormatDDP && pPrivate->iSPDIFMode == SPDIF_MODE_HD )
				{
					found_header = 1;
				}
				else
				{
					found_header = ac3_header_parse(pucInputBuf, &pPrivate->spdif_info);
				}
			}
			else if (pPrivate->audio_coding_type == OMX_AUDIO_CodingDTS)
			{
				if (pPrivate->pAudioDts.eFormat == OMX_AUDIO_DTSFormatDTSHD && pPrivate->iSPDIFMode == SPDIF_MODE_HD)
				{
					found_header = dts_header_parse_iv(pucInputBuf, &pPrivate->spdif_info, pPrivate->iSPDIFMode, nInputSize - iDataOffset);
				}
				else
				{
					found_header = dts_header_parse(pucInputBuf, &pPrivate->spdif_info, pPrivate->iSPDIFMode, nInputSize - iDataOffset);
				}
			}

			if (found_header == 1)
			{
				DBUG_MSG("found frame size in header = %d", pPrivate->spdif_info.frame_size);
				iNextHeaderOffset = pPrivate->spdif_info.frame_size;
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
		if((pPrivate->audio_coding_type == OMX_AUDIO_CodingAC3) && (pPrivate->pAudioAc3.eFormat == OMX_AUDIO_AC3FormatDDP && pPrivate->iSPDIFMode == SPDIF_MODE_HD) )
		{
			static const uint8_t eac3_repeat[4] = {6, 3, 2, 1};
			int DDP_repeat = 0;
			int checkedFrameLen= 0;
			int checkLen = 0;
			int count=0;
			
			for (count = 0 ; count < inputbuffer->nFilledLen-4; count++)
			{
				//To find sync info
				if (inputbuffer->pBuffer[count] == 0x0B && inputbuffer->pBuffer[count+1] == 0x77){
				    if ((inputbuffer->pBuffer[count+4] & 0xc0) != 0xc0) /* fscod */
		    			DDP_repeat = eac3_repeat[(inputbuffer->pBuffer[count+4] & 0x30) >> 4]; /* numblkscod */
					
					break;
				}
			}
			
			DDP_DEBUG_MSG("Start DDP_buffer_filled = %d repeat %d input len %d nInputSize %d count %d inputbuffer[] %x pucInputBuf[] %x",
				DDP_buffer_filled,DDP_repeat,inputbuffer->nFilledLen,nInputSize,count,inputbuffer->pBuffer[count+4],pucInputBuf[count+4]);
	
			if(count ==0 && DDP_remaining_length > 0)
			{
				DDP_DEBUG_MSG("DDP_remaining_length >0 but we found syncword , so DDP_remaining_length is not needed %d ",DDP_remaining_length);
				DDP_remaining_length = -1;
			}

			if(DDP_remaining_length == count)
			{
				DDP_DEBUG_MSG("fill previous frame with DDP_remaining_length %d ",DDP_remaining_length);
				memcpy(&TRUEHD_MAT_Buffer[DDP_buffer_filled], pucInputBuf,count);
				pPrivate->spdif_nConsumedLength += count;
				DDP_buffer_filled += count;
				DDP_remaining_length =-1;
			}
			else{
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
					ALOGE("Check DDP size DDP_buffer_filled %d checkLen %d ",DDP_buffer_filled,checkLen);
				iNextHeaderOffset = 0;
					DDP_buffer_filled = 0;
					inputbuffer->nFilledLen = 0;
					pPrivate->isNewBuffer = 1;
				return ;
			}
			else{
				memcpy(&TRUEHD_MAT_Buffer[DDP_buffer_filled],pucInputBuf,checkLen);
			}
				
			pPrivate->spdif_nConsumedLength += checkLen;
			DDP_buffer_filled += checkLen;

			DDP_DEBUG_MSG("DDP_buffer_filled = %d ConsumedLength %d checkLen %d illedLength %d ", DDP_buffer_filled,pPrivate->spdif_nConsumedLength,checkLen,pPrivate->spdif_nFilledLength);

			if (pPrivate->spdif_nConsumedLength >= pPrivate->spdif_nFilledLength)
			{
				DDP_DEBUG_MSG("isNewBuffer @@ ");
				inputbuffer->nFilledLen = 0;
				pPrivate->isNewBuffer = 1;
			}
			
			if(++DDP_Count < DDP_repeat)
			{
				iNextHeaderOffset = 0;
					DDP_DEBUG_MSG("More Data DDP_Count = %d repeat %d DDP_buffer_filled %d ", DDP_Count,DDP_repeat,DDP_buffer_filled);
					return ;
				}
			
			}
			
			if(DDP_remaining_length>0)
			{
				iNextHeaderOffset = 0;
				DDP_DEBUG_MSG("More Data DDP_remaining_length = %d DDP_buffer_filled=%d", DDP_remaining_length,DDP_buffer_filled);
				return ;
			}
			
			iNextHeaderOffset  = 24576;
			DDP_Count  = 0;
			
			DDP_DEBUG_MSG("End DDP_buffer_filled = %d spdif_nFilledLength %d consumed %d ", DDP_buffer_filled, pPrivate->spdif_nFilledLength,pPrivate->spdif_nConsumedLength);
		}
		// DDP HD passthru End

		// SPDIF_AC3_TRUEHD Start
		if((pPrivate->pAudioAc3.eFormat == OMX_AUDIO_AC3FormatTRUEHD) && (pPrivate->iSPDIFMode == SPDIF_MODE_HD))
		{
			// TODO : Create MAT File.
		}
		// SPDIF_AC3_TRUEHD End

		if (iNextHeaderOffset)
		{
			#if 0	// Test code	SPDIF_AC3_TRUEHD
			// Test Code Start
			FILE *dump_fp = NULL;

			dump_fp = fopen("/data/dump_spdif_out.data", "a+");
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
				|| (pPrivate->pAudioAc3.eFormat == OMX_AUDIO_AC3FormatTRUEHD) || (pPrivate->pAudioAc3.eFormat == OMX_AUDIO_AC3FormatDDP && pPrivate->iSPDIFMode == SPDIF_MODE_HD))	// SPDIF_AC3_TRUEHD
			{
				SPDIF_CODEC_TYPE type;

				if (pPrivate->audio_coding_type == OMX_AUDIO_CodingAC3)
				{
					// SPDIF_AC3_TRUEHD Start
					if(pPrivate->pAudioAc3.eFormat == OMX_AUDIO_AC3FormatTRUEHD)
					{
						type = FORMAT_AC3_TRUE_HD;
					}
					// SPDIF_AC3_TRUEHD End
					else if(pPrivate->pAudioAc3.eFormat == OMX_AUDIO_AC3FormatDDP && pPrivate->iSPDIFMode == SPDIF_MODE_HD)
					{
						type = FORMAT_AC3_DDP;
					}
					else
					{
						type = FORMAT_AC3;
					}
				}
				else if (pPrivate->audio_coding_type == OMX_AUDIO_CodingDTS)
				{
					if (pPrivate->pAudioDts.eFormat == OMX_AUDIO_DTSFormatDTSHD)
					{
						type = FORMAT_DTS_HD;
					}
					else
					{
						type = FORMAT_DTS;
					}
				}
				// SPDIF_AC3_TRUEHD Start
				if((pPrivate->pAudioAc3.eFormat == OMX_AUDIO_AC3FormatTRUEHD) && (pPrivate->iSPDIFMode == SPDIF_MODE_HD))
				{
					if (spdif_parser_frame(TRUEHD_MAT_Buffer, iNextHeaderOffset, type, 0, pPrivate->iSPDIFMode))
					{
						int spdif_frame_size = (int)spdif_parse_get_frame_size();
						//ALOGE("spdif_frame_size %d", spdif_frame_size);
						if (spdif_frame_size > 0)
						{
							memcpy(outputbuffer->pBuffer, spdif_parse_get_buf(), spdif_frame_size);
							outputbuffer->nFilledLen = spdif_frame_size;

							#if 0	// Test code
							// Test Code Start
							FILE *dump_fp = NULL;

							dump_fp = fopen("/data/dump_out002.data", "a+");
							if(dump_fp != NULL){
								fwrite(outputbuffer->pBuffer, spdif_frame_size, 1, dump_fp);
								fclose(dump_fp);
							}
							else {
								ALOGW("[Error] Don't write to /data/dump_out.pcm");
							}
							// Test Code End
							#endif

							if (pPrivate->iSamples == 0) // audio data is not started with the first chunk
							{
								pPrivate->iStartTS = inputbuffer->nTimeStamp;
							}
							outputbuffer->nTimeStamp = pPrivate->iStartTS + pPrivate->iSamples;
						}
						else
						{
							ALOGE("SPDIF :: spdif frame size is 0 !!");
						}
					}
					memset(TRUEHD_MAT_Buffer,0,61440);
				}
				// SPDIF_AC3_TRUEHD End
				else if(pPrivate->pAudioAc3.eFormat == OMX_AUDIO_AC3FormatDDP && pPrivate->iSPDIFMode == SPDIF_MODE_HD){
					if (spdif_parser_frame(TRUEHD_MAT_Buffer,DDP_buffer_filled , type, 0, 1))
					{
						int spdif_frame_size = (int)spdif_parse_get_frame_size();
						//ALOGD("OMX_AUDIO_AC3FormatDDP spdif_frame_size %d nSamplingRate %d iSamples %d ", 
						//	spdif_frame_size,pPrivate->pAudioPcmMode.nSamplingRate,pPrivate->iSamples);
						if (spdif_frame_size > 0)
						{
							memcpy(outputbuffer->pBuffer, spdif_parse_get_buf(), spdif_frame_size);
							outputbuffer->nFilledLen = spdif_frame_size;

							if (pPrivate->iSamples == 0) // audio data is not started with the first chunk
							{
								pPrivate->iStartTS = inputbuffer->nTimeStamp;
							}
							outputbuffer->nTimeStamp = pPrivate->iStartTS + pPrivate->iSamples;
						}
						else
						{
							ALOGE("SPDIF :: spdif frame size is 0 !!");
						}
					}
					
					memset(TRUEHD_MAT_Buffer,0,iNextHeaderOffset);
					iNextHeaderOffset = DDP_buffer_filled;
					DDP_buffer_filled=0;
					return;
				}
				else
				{
					if (spdif_parser_frame(&pPrivate->spdif_pBuffer[pPrivate->spdif_nConsumedLength], iNextHeaderOffset, type, 0, pPrivate->iSPDIFMode))
					{
						int spdif_frame_size = (int)spdif_parse_get_frame_size();
						DBUG_MSG("spdif_frame_size %d", spdif_frame_size);
						if (spdif_frame_size > 0)
						{
							memcpy(outputbuffer->pBuffer, spdif_parse_get_buf(), spdif_frame_size);
							outputbuffer->nFilledLen = spdif_frame_size;

							if (pPrivate->iSamples == 0) // audio data is not started with the first chunk
							{
								pPrivate->iStartTS = inputbuffer->nTimeStamp;
							}
							outputbuffer->nTimeStamp = pPrivate->iStartTS + pPrivate->iSamples;

							// in case of SPDIF passthrough mode, iSamples equals duration of generated frames 
							if (pPrivate->audio_coding_type == OMX_AUDIO_CodingAC3)
							{
								switch (pPrivate->pAudioPcmMode.nSamplingRate)
								{
									case 32000:
										pPrivate->iSamples += 48000; // 1536/32 = 48
										break;
									case 44100:
										pPrivate->iSamples += 34830; // 1536/44.1
										break;
									case 48000:
										pPrivate->iSamples += 32000; // 1536/48 = 32
										break;
									default:
										break;
								}
							}
							else if (pPrivate->audio_coding_type == OMX_AUDIO_CodingDTS)
							{
								// duration = total bits in a frame / bitrate
								// for 768kbps, total bits = 1006 * 8
								// so duration is 1006 * 8 / 754500 = 10.666666... ms
								pPrivate->iSamples += 10667; // 960 ticks / 90 = 10.666666... ms for 768kbps 
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
}

