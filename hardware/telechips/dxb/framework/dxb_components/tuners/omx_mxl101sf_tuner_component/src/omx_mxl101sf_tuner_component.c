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

#define LOG_TAG	"OMX_MXL101SF"
#include <utils/Log.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <omxcore.h>
#include <omx_mxl101sf_tuner_component.h>
#include "tcc_dxb_interface_tuner.h"
#include <OMX_TCC_Index.h>
#include "MxL101SF_dvbt.h"

typedef struct
{
	unsigned int uiMinChannel;
	unsigned int uiMaxChannel;
	unsigned int uiCountryCode;
}TUNER_SEARCH_INFO;

OMX_ERRORTYPE omx_mxl101sf_tuner_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,OMX_STRING cComponentName) 
{
	int err;
	int omxErr;
	omx_base_audio_PortType *pPort;
	omx_mxl101sf_tuner_component_PrivateType* omx_mxl101sf_tuner_component_Private;
	OMX_U32 i;
	char *pcm_name;

	DEBUG(DEFAULT_MESSAGES, "omx_mxl101sf_tuner_component_Constructor  \n");

	if (!openmaxStandComp->pComponentPrivate)
	{
		openmaxStandComp->pComponentPrivate = TCC_calloc(1, sizeof(omx_mxl101sf_tuner_component_PrivateType));
		if(openmaxStandComp->pComponentPrivate==NULL) 
		{
			return OMX_ErrorInsufficientResources;
		}
	}

	omx_mxl101sf_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	omx_mxl101sf_tuner_component_Private->ports = NULL;

	omxErr = omx_base_source_Constructor(openmaxStandComp,cComponentName);
	if (omxErr != OMX_ErrorNone)
	{
		return OMX_ErrorInsufficientResources;
	}

	omx_mxl101sf_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nStartPortNumber = 0;
	omx_mxl101sf_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts = 1;

	/** Allocate Ports and call port constructor. */  
	if (omx_mxl101sf_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts && !omx_mxl101sf_tuner_component_Private->ports) 
	{
		omx_mxl101sf_tuner_component_Private->ports = TCC_calloc(omx_mxl101sf_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts, sizeof(omx_base_audio_PortType *));
		if (!omx_mxl101sf_tuner_component_Private->ports) 
		{
			return OMX_ErrorInsufficientResources;
		}
		for (i=0; i < omx_mxl101sf_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) 
		{
			omx_mxl101sf_tuner_component_Private->ports[i] = TCC_calloc(1, sizeof(omx_base_audio_PortType));
			if (!omx_mxl101sf_tuner_component_Private->ports[i]) 
			{
				return OMX_ErrorInsufficientResources;
			}
		}
	}
	base_audio_port_Constructor(openmaxStandComp, &omx_mxl101sf_tuner_component_Private->ports[0], 0, OMX_FALSE);
	pPort = (omx_base_audio_PortType *) omx_mxl101sf_tuner_component_Private->ports[OMX_BASE_SOURCE_OUTPUTPORT_INDEX];	
	omx_mxl101sf_tuner_component_Private->BufferMgmtCallback = omx_mxl101sf_tuner_component_BufferMgmtCallback;
	omx_mxl101sf_tuner_component_Private->messageHandler = omx_mxl101sf_tuner_component_MessageHandler;	
	omx_mxl101sf_tuner_component_Private->destructor = omx_mxl101sf_tuner_component_Destructor;

	openmaxStandComp->SetParameter  = omx_mxl101sf_tuner_component_SetParameter;
	openmaxStandComp->GetParameter  = omx_mxl101sf_tuner_component_GetParameter;

	openmaxStandComp->SetConfig = omx_mxl101sf_tuner_component_SetConfig;
	openmaxStandComp->GetConfig = omx_mxl101sf_tuner_component_GetConfig;

	openmaxStandComp->GetExtensionIndex = omx_mxl101sf_tuner_component_GetExtensionIndex;	
	return OMX_ErrorNone;
}

/** The Destructor 
 */
OMX_ERRORTYPE omx_mxl101sf_tuner_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp)
{
	omx_mxl101sf_tuner_component_PrivateType* omx_mxl101sf_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	OMX_U32 i;

	/* frees port/s */
	if (omx_mxl101sf_tuner_component_Private->ports) 
	{
		for (i=0; i < omx_mxl101sf_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) 
		{
			if(omx_mxl101sf_tuner_component_Private->ports[i])
				omx_mxl101sf_tuner_component_Private->ports[i]->PortDestructor(omx_mxl101sf_tuner_component_Private->ports[i]);
		}
		TCC_free(omx_mxl101sf_tuner_component_Private->ports);
		omx_mxl101sf_tuner_component_Private->ports=NULL;
	}		
	return omx_base_source_Destructor(openmaxStandComp);

}

/** 
 * This function plays the input buffer. When fully consumed it returns.
 */

void omx_mxl101sf_tuner_component_BufferMgmtCallback(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* outputbuffer) 
{
	OMX_S32 data_read;
	omx_mxl101sf_tuner_component_PrivateType* omx_mxl101sf_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	int cnt;
	int remain_cnt;
	outputbuffer->nFilledLen = 0;
	outputbuffer->nFilledLen = 0;
	
	
}

OMX_ERRORTYPE omx_mxl101sf_tuner_component_GetExtensionIndex(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_STRING cParameterName,
  OMX_OUT OMX_INDEXTYPE* pIndexType)
{    
	
	DEBUG(DEB_LEV_FUNCTION_NAME,"In  %s \n",__func__);
	return OMX_ErrorNone;  
}


/** setting configurations */
OMX_ERRORTYPE omx_mxl101sf_tuner_component_SetConfig(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nIndex,
  OMX_IN  OMX_PTR pComponentConfigStructure)
{
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_mxl101sf_tuner_component_PrivateType* omx_mxl101sf_tuner_component_Private = openmaxStandComp->pComponentPrivate;
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
OMX_ERRORTYPE omx_mxl101sf_tuner_component_GetConfig(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nIndex,
  OMX_IN  OMX_PTR pComponentConfigStructure)
{
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_mxl101sf_tuner_component_PrivateType* omx_mxl101sf_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	
	switch (nIndex)
	{
		default: // delegate to superclass
			return omx_base_component_GetConfig(hComponent, nIndex, pComponentConfigStructure);
	}
	return OMX_ErrorNone;
}



OMX_ERRORTYPE omx_mxl101sf_tuner_component_SetParameter(
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

	/* Check which structure we are being fed and make control its header */
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE*)hComponent;
	omx_mxl101sf_tuner_component_PrivateType* omx_mxl101sf_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	omx_base_audio_PortType* pPort = (omx_base_audio_PortType *) omx_mxl101sf_tuner_component_Private->ports[OMX_BASE_SOURCE_OUTPUTPORT_INDEX];

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
        case OMX_IndexVendorParamTunerChannelScan:
            break;
        case OMX_IndexVendorParamTunerChannelSet:
            piArg = (int *)ComponentParameterStructure;
            err = MxL101SFDVBT_ChannelSet(piArg[0], piArg[1]); // piArg[0]: channel, piArg[1]: lockOn
            if(err != 0)
            {
                DEBUG(DEB_LEV_ERR, "Fail to Change channel %d\n", piArg[0]);
                omxErr = OMX_ErrorBadParameter;
            }
            break;
        case OMX_IndexVendorParamTunerGetChannelIndexByFrequency:
            break;
        case OMX_IndexVendorParamTunerFrequencySet:
            piArg = (int *)ComponentParameterStructure;
            err = MxL101SFDVBT_FrequencySet(piArg[0], piArg[1], 1, &piArg[4]); //piArg[0] : Freq, piArg[1]: BW, piArg[2]: LockOn, piArg[3]: option, piArg[4]: iSnr
            if(err != 0)
            {
                DEBUG(DEB_LEV_ERR, "Fail to Change Frequency %dkhz, %dkhz\n", piArg[0], piArg[1]);
                omxErr = OMX_ErrorBadParameter;
            }
            break;
        case OMX_IndexVendorParamTunerCountryCodeSet:
            break;
        case OMX_IndexVendorParamTunerOpen:
            piArg = (int *)ComponentParameterStructure;
            omx_mxl101sf_tuner_component_Private->countrycode = piArg[0];
            omx_mxl101sf_tuner_component_Private->dxb_standard = piArg[1];
            err = MxL101SFDVBT_Open(omx_mxl101sf_tuner_component_Private->countrycode, 0, 0, 1);
            if(err != 0)
            {
                DEBUG(DEB_LEV_ERR, "Fail to Open tuner[%d]\n", omx_mxl101sf_tuner_component_Private->countrycode);
                omxErr = OMX_ErrorBadParameter;
            }
            break;
        case OMX_IndexVendorParamTunerClose:
            MxL101SFDVBT_Close();
            break;
        case OMX_IndexVendorParamSetResync:
            break;
        case OMX_IndexVendorParamRegisterPID:
            break;
        case OMX_IndexVendorParamUnRegisterPID:
            break;
        case OMX_IndexVendorParamTunerSetNumberOfBB:
            break;
        case OMX_IndexVendorConfigSetAntennaPower:
            {
                int arg = (int)ComponentParameterStructure;
                SetAntennaPower(arg);
            }
            break;
        default: /*Call the base component function*/
            return omx_base_component_SetParameter(hComponent, nParamIndex, ComponentParameterStructure);
    }

    return omxErr;
}

OMX_ERRORTYPE omx_mxl101sf_tuner_component_GetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_INOUT OMX_PTR ComponentParameterStructure)
{
	OMX_OTHER_PARAM_PORTFORMATTYPE *pOtherPortFormat;  
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE*)hComponent;
	omx_mxl101sf_tuner_component_PrivateType* omx_mxl101sf_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	omx_base_audio_PortType *pPort = (omx_base_audio_PortType *) omx_mxl101sf_tuner_component_Private->ports[OMX_BASE_SOURCE_OUTPUTPORT_INDEX];  
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
			}	
			break;

		case OMX_IndexVendorParamTunerChannelSearchStart:
			break;

		case OMX_IndexVendorParamTunerChannelSearchStop:
			break;

		case OMX_IndexVendorParamSetGetPacketDataEntry:
			break;

		case OMX_IndexVendorConfigGetSignalStrength:
            {
                unsigned int *puiStr = (unsigned int *)ComponentParameterStructure;
                char *msg = puiStr[4];

                puiStr[0] = MxL101SFDVBT_GetSignalStrength();  // signal level (100 ~ 4)
                puiStr[1] = MxL101SFDVBT_GetSignalQuality();   // signal quality (0 ~ 100)
                puiStr[2] = 0;//MxL101SFDVBT_GetSNR();         // snr (0 ~ 100)
                puiStr[3] = 0;//MxL101SFDVBT_GetBER();         // ber (0 ~ 100)

                //if (msg) sprintf(msg, "STR: %d, QUA: %d, SNR: %d, BER: %d\n", puiStr[0], puiStr[1], puiStr[2], puiStr[3]);
            }
            break;

		case OMX_IndexVendorParamTunerGetEWSFlag:
			break;

		default: /*Call the base component function*/
			return omx_base_component_GetParameter(hComponent, nParamIndex, ComponentParameterStructure);

	}

	return err;

}

OMX_ERRORTYPE omx_mxl101sf_tuner_component_MessageHandler(OMX_COMPONENTTYPE* openmaxStandComp, internalRequestMessageType *message)
{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	omx_mxl101sf_tuner_component_PrivateType* omx_mxl101sf_tuner_component_Private = (omx_mxl101sf_tuner_component_PrivateType*)openmaxStandComp->pComponentPrivate;  	
	OMX_STATETYPE eCurrentState = omx_mxl101sf_tuner_component_Private->state;
	DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s\n", __func__);
	
	return err;  
}

OMX_ERRORTYPE omx_mxl101sf_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp) 
{
	return (omx_mxl101sf_tuner_component_Constructor(openmaxStandComp, "OMX.tcc.broadcast.mxl101sf"));
}

OMX_ERRORTYPE omx_mxl101sf_tuner_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp)
{
	return omx_mxl101sf_tuner_component_Destructor(openmaxStandComp);
}
