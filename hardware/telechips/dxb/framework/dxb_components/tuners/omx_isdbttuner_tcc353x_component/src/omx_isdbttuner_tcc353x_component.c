/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions andlimitations under the License.

****************************************************************************/
#define LOG_TAG	"OMX_TCC353X_TUNER"
#include <utils/Log.h>

#include <omxcore.h>
#include <OMX_TCC_Index.h>
#include <omx_isdbttuner_tcc353x_component.h>
#include "isdbt_tcc353x_tuner.h"
#include "isdbt_tuner_space.h"
#include "tcc353x_common.h"

static int giCmdInterface = TCC353X_IF_I2C;
static int giDataInterface = TCC353X_STREAM_IO_STS;

extern I32S Tcc353xApiCasOpen  (I32S _moduleIndex, I32U _casRound, I08U * _systemKey);
extern I32S Tcc353xApiCasSetPid  (I32S _moduleIndex, I32U *_pids, I32U _numberOfPids);
extern I32S Tcc353xApiCasSetKeyMulti2 (I32S _moduleIndex, I32S _parity, I08U *_key,I32S _keyLength, I08U *_initVector,I32S _initVectorLength);
extern unsigned int FrequencyInfo[4];

typedef struct {
	unsigned int tmccLock;
	unsigned int mer[3];
	unsigned int vBer[3];
	unsigned int pcber[3];
	unsigned int tsper[3];
	int rssi;
	unsigned int bbLoopGain;
	unsigned int rfLoopGain;
	unsigned int snr;
	unsigned int frequency;
} _Tcc353xStatusToUi;

#define _USE_4STEP_
static int ISDBT_CalcSignalStrengthTcc353x(unsigned int antennaBarPercent)
{
	int ret = 0;

#ifdef _USE_4STEP_
	if(antennaBarPercent>75)
		return	4;
	else if(antennaBarPercent>50)
		return	3;
	else if(antennaBarPercent>25)
		return	2;
	else if(antennaBarPercent>0)
		return	1;
	else
		return	0;
#else	/* 5step */
	if(antennaBarPercent>80)
		return	5;
	else if(antennaBarPercent>60)
		return	4;
	else if(antennaBarPercent>40)
		return	3;
	else if(antennaBarPercent>20)
		return	2;
	else if(antennaBarPercent>0)
		return	1;
	else
		return	0;
#endif
	return ret;
}

OMX_ERRORTYPE omx_isdbt_tuner_tcc353x_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,OMX_STRING cComponentName) 
{
	int err;
	int omxErr;
	omx_base_audio_PortType *pPort;
	omx_isdbt_tuner_tcc353x_component_PrivateType* omx_isdbt_tuner_tcc353x_component_Private;
	OMX_U32 i;
	char *pcm_name;

	DEBUG(DEFAULT_MESSAGES, "omx_isdbt_tuner_tcc353x_component_Constructor  \n");

	if (!openmaxStandComp->pComponentPrivate)
	{
		openmaxStandComp->pComponentPrivate = TCC_calloc(1, sizeof(omx_isdbt_tuner_tcc353x_component_PrivateType));
		if(openmaxStandComp->pComponentPrivate==NULL) 
		{
			return OMX_ErrorInsufficientResources;
		}
	}

	omx_isdbt_tuner_tcc353x_component_Private = openmaxStandComp->pComponentPrivate;
	omx_isdbt_tuner_tcc353x_component_Private->ports = NULL;

	omxErr = omx_base_source_Constructor(openmaxStandComp,cComponentName);
	if (omxErr != OMX_ErrorNone)
	{
		return OMX_ErrorInsufficientResources;
	}

	omx_isdbt_tuner_tcc353x_component_Private->sPortTypesParam[OMX_PortDomainAudio].nStartPortNumber = 0;
	omx_isdbt_tuner_tcc353x_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts = 1;

	/** Allocate Ports and call port constructor. */  
	if (omx_isdbt_tuner_tcc353x_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts && !omx_isdbt_tuner_tcc353x_component_Private->ports) 
	{
		omx_isdbt_tuner_tcc353x_component_Private->ports = TCC_calloc(omx_isdbt_tuner_tcc353x_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts, sizeof(omx_base_audio_PortType *));
		if (!omx_isdbt_tuner_tcc353x_component_Private->ports) 
		{
			return OMX_ErrorInsufficientResources;
		}
		for (i=0; i < omx_isdbt_tuner_tcc353x_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) 
		{
			omx_isdbt_tuner_tcc353x_component_Private->ports[i] = TCC_calloc(1, sizeof(omx_base_audio_PortType));
			if (!omx_isdbt_tuner_tcc353x_component_Private->ports[i]) 
			{
				return OMX_ErrorInsufficientResources;
			}
		}
	}

	base_audio_port_Constructor(openmaxStandComp, &omx_isdbt_tuner_tcc353x_component_Private->ports[0], 0, OMX_FALSE);

	pPort = (omx_base_audio_PortType *) omx_isdbt_tuner_tcc353x_component_Private->ports[OMX_BASE_SOURCE_OUTPUTPORT_INDEX];
	omx_isdbt_tuner_tcc353x_component_Private->BufferMgmtCallback = omx_isdbt_tuner_tcc353x_component_BufferMgmtCallback;
	omx_isdbt_tuner_tcc353x_component_Private->messageHandler = omx_isdbt_tuner_tcc353x_component_MessageHandler;	
	omx_isdbt_tuner_tcc353x_component_Private->destructor = omx_isdbt_tuner_tcc353x_component_Destructor;

	openmaxStandComp->SetParameter  = omx_isdbt_tuner_tcc353x_component_SetParameter;
	openmaxStandComp->GetParameter  = omx_isdbt_tuner_tcc353x_component_GetParameter;

	openmaxStandComp->SetConfig = omx_isdbt_tuner_tcc353x_component_SetConfig;
	openmaxStandComp->GetConfig = omx_isdbt_tuner_tcc353x_component_GetConfig;

	openmaxStandComp->GetExtensionIndex = omx_isdbt_tuner_tcc353x_component_GetExtensionIndex;

	if(strcmp(cComponentName, BROADCAST_TCC353X_CSPI_STS_TUNER_NAME) == 0 )
	{
		giCmdInterface = TCC353X_IF_TCCSPI; 
		giDataInterface = TCC353X_STREAM_IO_STS;
	} 
	else if(strcmp(cComponentName, BROADCAST_TCC353X_I2C_STS_TUNER_NAME) == 0 )
	{
		giCmdInterface = TCC353X_IF_I2C;
		giDataInterface = TCC353X_STREAM_IO_STS;
	}
    else
    {
		giCmdInterface = TCC353X_IF_TCCSPI; 
		giDataInterface = TCC353X_STREAM_IO_STS;
    }

	return OMX_ErrorNone;

}

/** The Destructor 
 */
OMX_ERRORTYPE omx_isdbt_tuner_tcc353x_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp)
{
	omx_isdbt_tuner_tcc353x_component_PrivateType* omx_isdbt_tuner_tcc353x_component_Private = openmaxStandComp->pComponentPrivate;
	OMX_U32 i;

	/* frees port/s */
	if (omx_isdbt_tuner_tcc353x_component_Private->ports) 
	{
		for (i=0; i < omx_isdbt_tuner_tcc353x_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) 
		{
			if(omx_isdbt_tuner_tcc353x_component_Private->ports[i])
				omx_isdbt_tuner_tcc353x_component_Private->ports[i]->PortDestructor(omx_isdbt_tuner_tcc353x_component_Private->ports[i]);
		}
		TCC_free(omx_isdbt_tuner_tcc353x_component_Private->ports);
		omx_isdbt_tuner_tcc353x_component_Private->ports=NULL;
	}

	return omx_base_source_Destructor(openmaxStandComp);

}

/** 
 * This function plays the input buffer. When fully consumed it returns.
 */

void omx_isdbt_tuner_tcc353x_component_BufferMgmtCallback(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* outputbuffer) 
{
	OMX_S32 data_read;
	omx_isdbt_tuner_tcc353x_component_PrivateType* omx_isdbt_tuner_tcc353x_component_Private = openmaxStandComp->pComponentPrivate;
	int cnt;
	int remain_cnt;
	outputbuffer->nFilledLen = 0;
	outputbuffer->nFilledLen = 0;
	

}

OMX_ERRORTYPE omx_isdbt_tuner_tcc353x_component_GetExtensionIndex(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_STRING cParameterName,
  OMX_OUT OMX_INDEXTYPE* pIndexType)
{    
	
	DEBUG(DEB_LEV_FUNCTION_NAME,"In  %s \n",__func__);
	return OMX_ErrorNone;  
}


/** setting configurations */
OMX_ERRORTYPE omx_isdbt_tuner_tcc353x_component_SetConfig(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nIndex,
  OMX_IN  OMX_PTR pComponentConfigStructure)
{
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_isdbt_tuner_tcc353x_component_PrivateType* omx_isdbt_tuner_tcc353x_component_Private = openmaxStandComp->pComponentPrivate;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	int	ret;
	int	channel;
	
	switch (nIndex)
	{		
		default: // delegate to superclass
			return omx_base_component_SetConfig(hComponent, nIndex, pComponentConfigStructure);
  }
  return OMX_ErrorNone;
}

/** setting configurations */
OMX_ERRORTYPE omx_isdbt_tuner_tcc353x_component_GetConfig(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nIndex,
  OMX_IN  OMX_PTR pComponentConfigStructure)
{
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_isdbt_tuner_tcc353x_component_PrivateType* omx_isdbt_tuner_tcc353x_component_Private = openmaxStandComp->pComponentPrivate;
	
	switch (nIndex)
	{
		default: // delegate to superclass
			return omx_base_component_GetConfig(hComponent, nIndex, pComponentConfigStructure);
	}
	return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_isdbt_tuner_tcc353x_component_SetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_IN  OMX_PTR ComponentParameterStructure)
{
	int err;
	int omxErr = OMX_ErrorNone;
	OMX_OTHER_PARAM_PORTFORMATTYPE *pOtherPortFormat;
	OMX_U32 portIndex;
	int freq;
	int channel;
	int cnt;
	int *piArg;	
	unsigned int *puiArg;	
	unsigned char *pcArg;	

	/* Check which structure we are being fed and make control its header */
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE*)hComponent;
	omx_isdbt_tuner_tcc353x_component_PrivateType* omx_isdbt_tuner_tcc353x_component_Private = openmaxStandComp->pComponentPrivate;
	omx_base_audio_PortType* pPort = (omx_base_audio_PortType *) omx_isdbt_tuner_tcc353x_component_Private->ports[OMX_BASE_SOURCE_OUTPUTPORT_INDEX];

	DEBUG(DEB_LEV_SIMPLE_SEQ, "   Setting parameter %i\n", nParamIndex);

	/** Each time we are (re)configuring the hw_params thing
	* we need to reinitialize it, otherwise previous changes will not take effect.
	* e.g.: changing a previously configured sampling rate does not have
	* any effect if we are not calling this each time.
	*/

	switch(nParamIndex) 
	{		
		case OMX_IndexVendorParamTunerDeviceSet:		
			break;

		case OMX_IndexVendorParamTunerChannelSet:		
			piArg = (int *)ComponentParameterStructure;
			channel = piArg[0];
			err = tcc353x_frequency_get (channel, &freq);
			if (err == 0) {
				DEBUG(DEB_LEV_ERR, "Channel_index %d\n", channel);
				err = tcc353x_channel_set(channel);
				if(err < 0)
				{
					omxErr = OMX_ErrorBadParameter;
				}
			} else {
				omxErr = OMX_ErrorBadParameter;
			}
			break;
		case OMX_IndexVendorParamTunerFrequencySet:
			piArg = (int *)ComponentParameterStructure;
			err = tcc353x_frequency_set(piArg[0], piArg[1]); //piArg[0] : Freq, piArg[1]: BW
			if(err == 0)
			{
				omxErr = OMX_ErrorBadParameter;
			}
			break;	
		case OMX_IndexVendorParamTunerCountryCodeSet:
			omx_isdbt_tuner_tcc353x_component_Private->countrycode = (int)ComponentParameterStructure;
			break;

		case OMX_IndexVendorParamTunerOpen:
			piArg = (int *)ComponentParameterStructure;
			omx_isdbt_tuner_tcc353x_component_Private->countrycode = piArg[0];
			SetCountryCode(omx_isdbt_tuner_tcc353x_component_Private->countrycode);
			err = tcc353x_init(giCmdInterface, giDataInterface);
			if(err < 0)
			{
				DEBUG(DEB_LEV_ERR, "tcc353x_tuner_init Error\n");
				omxErr = OMX_ErrorBadParameter;
			}
			break;
			
		case OMX_IndexVendorParamTunerSetCasOpen:
			pcArg = (unsigned char *)ComponentParameterStructure;
			Tcc353xApiCasOpen  (0, pcArg[0], &pcArg[1]);
		break;
            
		case OMX_IndexVendorParamTunerSetCasSetmulti2:
		    //ALOGD("OMX_IndexVendorParamTunerSetCasSetmulti2\n");
			pcArg = (unsigned char *)ComponentParameterStructure;
			Tcc353xApiCasSetKeyMulti2 (0, pcArg[0], &pcArg[1], 32, &pcArg[17], pcArg[16]);
		break;
            
		case OMX_IndexVendorParamTunerSetCasSetPid:
			puiArg = (unsigned int *)ComponentParameterStructure;
			Tcc353xApiCasSetPid  (0, &puiArg[1], puiArg[0]);
		break;
            
		case OMX_IndexVendorParamTunerClose:
			tcc353x_deinit(giCmdInterface, giDataInterface);
			break;		
		case OMX_IndexVendorParamTunerSetFreqBand:
			freq = (int)ComponentParameterStructure;
			SetFreqBand (freq);
			break;
		case OMX_IndexVendorParamRegisterPID:
			//empty in intention.
			break;
		default: /*Call the base component function*/
			return omx_base_component_SetParameter(hComponent, nParamIndex, ComponentParameterStructure);
	}

	return omxErr;

}

OMX_ERRORTYPE omx_isdbt_tuner_tcc353x_component_GetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_INOUT OMX_PTR ComponentParameterStructure)
{
	OMX_OTHER_PARAM_PORTFORMATTYPE *pOtherPortFormat;  
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE*)hComponent;
	omx_isdbt_tuner_tcc353x_component_PrivateType* omx_isdbt_tuner_tcc353x_component_Private = openmaxStandComp->pComponentPrivate;
	omx_base_audio_PortType *pPort = (omx_base_audio_PortType *) omx_isdbt_tuner_tcc353x_component_Private->ports[OMX_BASE_SOURCE_OUTPUTPORT_INDEX];  
	int	countrycode;
	int	channel_cnt;
	int	ret;
		
	if (ComponentParameterStructure == NULL) 
	{
		return OMX_ErrorBadParameter;
	}
	DEBUG(DEB_LEV_SIMPLE_SEQ, "   Getting parameter %i\n", nParamIndex);
	/* Check which structure we are being fed and fill its header */

	switch(nParamIndex) 
	{		
		case OMX_IndexVendorParamTunerChannelSet:						
			{
				TUNER_SEARCH_INFO *pSearchInfo;
				pSearchInfo =(TUNER_SEARCH_INFO *)ComponentParameterStructure;
				pSearchInfo->uiCountryCode = omx_isdbt_tuner_tcc353x_component_Private->countrycode;
				pSearchInfo->uiMinChannel = GetMinChannel();
				pSearchInfo->uiMaxChannel = GetMaxChannel();
			}
			break;
		case OMX_IndexVendorParamTunerChannelSearchStart:
			{
				int     min, max, i, ret;
				int iEventData[3]; //0:Channel 1:Frequency 2:Contrycode
				DEBUG(DEFAULT_MESSAGES, "[ISDBTTUNER]SCAN START!!!\n");

				min = GetMinChannel();
				max =GetMaxChannel();
				for(i=min; i<=max; i++)
				{
					DEBUG(DEB_LEV_ERR, "Channel : %d\n", i);
					ret= tcc353x_channel_set(i);
					if(ret != FALSE)
					{
						iEventData[0] = i; //Channel
						iEventData[1] = ret; //Frequency
						iEventData[2] = omx_isdbt_tuner_tcc353x_component_Private->countrycode; //Contrycode


						(*(omx_isdbt_tuner_tcc353x_component_Private->callbacks->EventHandler))
						(openmaxStandComp,
						omx_isdbt_tuner_tcc353x_component_Private->callbackData,
						OMX_EventDynamicResourcesAvailable, /* The command was completed */
						0,
						0, /* This is the output port index */
						iEventData);
					}

				}

				DEBUG(DEFAULT_MESSAGES, "[ISDBTTUNER]SCAN END\n");

				break;
			}
			break;
		case OMX_IndexVendorConfigGetSignalStrength:
			{
				unsigned int *puiStr;
				Tcc353xStatus_t monitorValues;
				puiStr = (unsigned int *)ComponentParameterStructure;
				_Tcc353xStatusToUi *pStatToUi;
				tmccInfo_t TmccInfo;
				int tmcc_transparam_layerA;
				int tmcc_transparam_layerB;
				int tmcc_transparam_layerC;
				int res;
				int ret;

				/* 
				  * puiStr		0 = size of sqinfo in bytes
				  *			1 = percentage of 1seg signal quality
				  *			2 = percentage of fullseg signal quality
				  *			3 ~ other informations about signal quality
				  * total size including the size of sqinfo should be less than 256*sizeof(int)
				  */
				ret = tcc353x_get_signal_strength(&monitorValues);
				if(ret==-1) {
					err = OMX_ErrorHardware;
				} else {
					err = OMX_ErrorNone;
					puiStr[0] = sizeof(_Tcc353xStatusToUi) + 6*sizeof(int);
					if (monitorValues.status.isdbLock.TMCC) {
						res = tcc353x_get_tmcc_information(&TmccInfo);
						tmcc_transparam_layerA = TmccInfo.currentInfo.transParammLayerA;
						tmcc_transparam_layerB = TmccInfo.currentInfo.transParammLayerB;
						tmcc_transparam_layerC = TmccInfo.currentInfo.transParammLayerC;
					} else {
						tmcc_transparam_layerA = 0;
						tmcc_transparam_layerB = 0;
						tmcc_transparam_layerC = 0;
					}

					puiStr[1] = monitorValues.antennaPercent[0];
					puiStr[2] = monitorValues.antennaPercent[1];
					puiStr[3] = monitorValues.antennaPercent[2];
					puiStr[4] = tmcc_transparam_layerA;
					puiStr[5] = tmcc_transparam_layerB;
					puiStr[6] = tmcc_transparam_layerC;
		
					pStatToUi = (_Tcc353xStatusToUi *) &puiStr[7];

					pStatToUi->tmccLock = monitorValues.status.isdbLock.TMCC;

					pStatToUi->mer[0] = monitorValues.status.mer[0].currentValue;
					pStatToUi->mer[1] = monitorValues.status.mer[1].currentValue;
					pStatToUi->mer[2] = monitorValues.status.mer[2].currentValue;

					pStatToUi->snr = monitorValues.status.snr.currentValue;
					
					pStatToUi->vBer[0] = monitorValues.status.viterbiber[0].currentValue;
					pStatToUi->vBer[1] = monitorValues.status.viterbiber[1].currentValue;
					pStatToUi->vBer[2] = monitorValues.status.viterbiber[2].currentValue;

					pStatToUi->pcber[0] = monitorValues.status.pcber[0].currentValue;
					pStatToUi->pcber[1] = monitorValues.status.pcber[1].currentValue;
					pStatToUi->pcber[2] = monitorValues.status.pcber[2].currentValue;

					pStatToUi->tsper[0] = monitorValues.status.tsper[0].currentValue;
					pStatToUi->tsper[1] = monitorValues.status.tsper[1].currentValue;
					pStatToUi->tsper[2] = monitorValues.status.tsper[2].currentValue;

					pStatToUi->rssi = monitorValues.status.rssi.currentValue;
					pStatToUi->bbLoopGain = monitorValues.bbLoopGain;
					pStatToUi->rfLoopGain = monitorValues.rfLoopGain;

					pStatToUi->frequency = FrequencyInfo[0];
				}
			}
			break;

		case OMX_IndexVendorConfigGetSignalStrengthIndex:
			{
				int *puiStr;
				Tcc353xStatus_t monitorValues;
				puiStr = (int *)ComponentParameterStructure;
				int ret;

				ret = tcc353x_get_signal_strength(&monitorValues);
				if(ret==-1) {
					err = OMX_ErrorHardware;
				} else {
					err = OMX_ErrorNone;
					*puiStr = monitorValues.status.rssi.currentValue;
				}
			}
			break;

		case OMX_IndexVendorParamTunerGetEWSFlag:
			{
				int *puiStr = (unsigned int *)ComponentParameterStructure;
				#if 1
				int flag;
				flag = tcc353x_get_ews_flag ();
				*puiStr = flag;
				/*
				TcpalPrintStatus((I08S *)"[TCC353X] EWS [%d]\n", flag);
				*/
				#else
				*puiStr = 0;
				#endif
			}
			break;
		case OMX_IndexVendorParamTunerGetChannelValidity:
			{
				int *pChannel = (int *)ComponentParameterStructure;
				int validity;
				validity = GetFrequencyValidity (*pChannel);
				if (validity) err = OMX_ErrorNone;
				else err = OMX_ErrorHardware;
				break;
			}
		case OMX_IndexVendorConfigGetSignalStrengthIndexTime:
			{
				int *tuner_str_idx_time;
				tuner_str_idx_time = (int *)ComponentParameterStructure;
				*tuner_str_idx_time = 1000;	//mili-sec
				break;
			}
		default: /*Call the base component function*/
			return omx_base_component_GetParameter(hComponent, nParamIndex, ComponentParameterStructure);

	}

	return err;

}

OMX_ERRORTYPE omx_isdbt_tuner_tcc353x_component_MessageHandler(OMX_COMPONENTTYPE* openmaxStandComp, internalRequestMessageType *message)
{
	omx_isdbt_tuner_tcc353x_component_PrivateType* omx_isdbt_tuner_tcc353x_component_Private = (omx_isdbt_tuner_tcc353x_component_PrivateType*)openmaxStandComp->pComponentPrivate;  
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_STATETYPE eCurrentState = omx_isdbt_tuner_tcc353x_component_Private->state;
	DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s\n", __func__);
	
	return err;  
}

OMX_ERRORTYPE omx_tcc353x_I2C_STS_tuner_component_Init (OMX_COMPONENTTYPE *openmaxStandComp) 
{
	return (omx_isdbt_tuner_tcc353x_component_Constructor(openmaxStandComp, BROADCAST_TCC353X_I2C_STS_TUNER_NAME));
}

OMX_ERRORTYPE omx_tcc353x_CSPI_STS_tuner_component_Init (OMX_COMPONENTTYPE *openmaxStandComp) 
{
	return (omx_isdbt_tuner_tcc353x_component_Constructor(openmaxStandComp, BROADCAST_TCC353X_CSPI_STS_TUNER_NAME));
}

OMX_ERRORTYPE omx_tcc353x_I2C_STS_tuner_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp)
{
	return omx_isdbt_tuner_tcc353x_component_Destructor(openmaxStandComp);
}

OMX_ERRORTYPE omx_tcc353x_CSPI_STS_tuner_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp)
{
	return omx_isdbt_tuner_tcc353x_component_Destructor(openmaxStandComp);
}

