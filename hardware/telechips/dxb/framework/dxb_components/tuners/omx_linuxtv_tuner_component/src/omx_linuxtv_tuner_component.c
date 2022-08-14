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

#define LOG_TAG	"OMX_LINUXTV_TUNER"
#include <utils/Log.h>

#include <OMX_TCC_Index.h>
#include <omxcore.h>
#include <omx_linuxtv_tuner_component.h>

#include <LinuxTV_Frontend.h>

OMX_ERRORTYPE omx_linuxtv_tuner_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,OMX_STRING cComponentName) 
{
	int err;
	int omxErr;
	omx_base_audio_PortType *pPort;
	omx_linuxtv_tuner_component_PrivateType* omx_linuxtv_tuner_component_Private;
	OMX_U32 i;
	char *pcm_name;

	DEBUG(DEFAULT_MESSAGES, "omx_linuxtv_tuner_component_Constructor  \n");

	if (!openmaxStandComp->pComponentPrivate)
	{
		openmaxStandComp->pComponentPrivate = TCC_calloc(1, sizeof(omx_linuxtv_tuner_component_PrivateType));
		if(openmaxStandComp->pComponentPrivate==NULL) 
		{
			return OMX_ErrorInsufficientResources;
		}
	}

	omx_linuxtv_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	omx_linuxtv_tuner_component_Private->ports = NULL;

	omxErr = omx_base_source_Constructor(openmaxStandComp,cComponentName);
	if (omxErr != OMX_ErrorNone)
	{
		return OMX_ErrorInsufficientResources;
	}

	omx_linuxtv_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nStartPortNumber = 0;
	omx_linuxtv_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts = 1;

	/** Allocate Ports and call port constructor. */  
	if (omx_linuxtv_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts && !omx_linuxtv_tuner_component_Private->ports) 
	{
		omx_linuxtv_tuner_component_Private->ports = TCC_calloc(omx_linuxtv_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts, sizeof(omx_base_audio_PortType *));
		if (!omx_linuxtv_tuner_component_Private->ports) 
		{
			return OMX_ErrorInsufficientResources;
		}
		for (i=0; i < omx_linuxtv_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) 
		{
			omx_linuxtv_tuner_component_Private->ports[i] = TCC_calloc(1, sizeof(omx_base_audio_PortType));
			if (!omx_linuxtv_tuner_component_Private->ports[i]) 
			{
				return OMX_ErrorInsufficientResources;
			}
		}
	}
	base_audio_port_Constructor(openmaxStandComp, &omx_linuxtv_tuner_component_Private->ports[0], 0, OMX_FALSE);
	pPort = (omx_base_audio_PortType *) omx_linuxtv_tuner_component_Private->ports[OMX_BASE_SOURCE_OUTPUTPORT_INDEX];	
	omx_linuxtv_tuner_component_Private->BufferMgmtCallback = omx_linuxtv_tuner_component_BufferMgmtCallback;
	omx_linuxtv_tuner_component_Private->messageHandler = omx_linuxtv_tuner_component_MessageHandler;	
	omx_linuxtv_tuner_component_Private->destructor = omx_linuxtv_tuner_component_Destructor;

	openmaxStandComp->SetParameter  = omx_linuxtv_tuner_component_SetParameter;
	openmaxStandComp->GetParameter  = omx_linuxtv_tuner_component_GetParameter;

	openmaxStandComp->SetConfig = omx_linuxtv_tuner_component_SetConfig;
	openmaxStandComp->GetConfig = omx_linuxtv_tuner_component_GetConfig;

	openmaxStandComp->GetExtensionIndex = omx_linuxtv_tuner_component_GetExtensionIndex;	

	return OMX_ErrorNone;
}

/**
 * The Destructor 
 */
OMX_ERRORTYPE omx_linuxtv_tuner_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp)
{
	omx_linuxtv_tuner_component_PrivateType* omx_linuxtv_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	OMX_U32 i;

	/* frees port/s */
	if (omx_linuxtv_tuner_component_Private->ports) 
	{
		for (i=0; i < omx_linuxtv_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) 
		{
			if(omx_linuxtv_tuner_component_Private->ports[i])
				omx_linuxtv_tuner_component_Private->ports[i]->PortDestructor(omx_linuxtv_tuner_component_Private->ports[i]);
		}
		TCC_free(omx_linuxtv_tuner_component_Private->ports);
		omx_linuxtv_tuner_component_Private->ports=NULL;
	}		
	return omx_base_source_Destructor(openmaxStandComp);

}

/** 
 * This function plays the input buffer. When fully consumed it returns.
 */
void omx_linuxtv_tuner_component_BufferMgmtCallback(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* outputbuffer) 
{
	omx_linuxtv_tuner_component_PrivateType* omx_linuxtv_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	outputbuffer->nFilledLen = 0;
	outputbuffer->nFilledLen = 0;
}

OMX_ERRORTYPE omx_linuxtv_tuner_component_GetExtensionIndex(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_STRING cParameterName,
  OMX_OUT OMX_INDEXTYPE* pIndexType)
{    
	DEBUG(DEB_LEV_FUNCTION_NAME,"In  %s \n",__func__);
	return OMX_ErrorNone;  
}

OMX_ERRORTYPE omx_linuxtv_tuner_component_SetConfig(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nIndex,
  OMX_IN  OMX_PTR pComponentConfigStructure)
{
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_linuxtv_tuner_component_PrivateType* omx_linuxtv_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	OMX_ERRORTYPE omxErr = OMX_ErrorNone;
	
	switch (nIndex)
	{		
		default: // delegate to superclass
			return omx_base_component_SetConfig(hComponent, nIndex, pComponentConfigStructure);
  }
  return omxErr;
}

OMX_ERRORTYPE omx_linuxtv_tuner_component_GetConfig(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nIndex,
  OMX_IN  OMX_PTR pComponentConfigStructure)
{
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_linuxtv_tuner_component_PrivateType* omx_linuxtv_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	OMX_ERRORTYPE omxErr = OMX_ErrorNone;

	switch (nIndex)
	{
		default: // delegate to superclass
			return omx_base_component_GetConfig(hComponent, nIndex, pComponentConfigStructure);
	}
	return omxErr;
}

OMX_ERRORTYPE omx_linuxtv_tuner_component_SetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_IN  OMX_PTR ComponentParameterStructure)
{
	OMX_ERRORTYPE omxErr = OMX_ErrorNone;
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE*)hComponent;
	omx_linuxtv_tuner_component_PrivateType* omx_linuxtv_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	int *piArg;

	DEBUG(DEB_LEV_SIMPLE_SEQ, "   Setting parameter %i\n", nParamIndex);

	switch(nParamIndex) 
	{		
		case OMX_IndexVendorParamTunerDeviceSet:			
			break;

		case OMX_IndexVendorParamTunerChannelScan:			
			break;
		case OMX_IndexVendorParamTunerChannelSet:
			break;
		case OMX_IndexVendorParamTunerGetChannelIndexByFrequency:
			break;
		case OMX_IndexVendorParamTunerFrequencySet:
    		{
				piArg = (int *)ComponentParameterStructure; //piArg[0] : Freq, piArg[1]: BW, piArg[2]: LockOn, piArg[3] : option
				if (LinuxTV_Frontend_SetFrontend(piArg[0], piArg[1], piArg[2]) < 0)
				{
					DEBUG(DEB_LEV_ERR, "Fail to Change Frequency %dkhz, %dkhz\n", piArg[0], piArg[1]);
					omxErr = OMX_ErrorBadParameter;
				}
			}		   
			break;	

		case OMX_IndexVendorParamTunerCountryCodeSet:
			break;

		case OMX_IndexVendorParamTunerOpen:
			{
				piArg = (int *)ComponentParameterStructure;
				LinuxTV_Frontend_Open();
				LinuxTV_Frontend_SetTuneMode(FE_TUNE_MODE_ONESHOT);
            }
			break;

		case OMX_IndexVendorParamTunerClose:
			{
				LinuxTV_Frontend_Close();
			}
			break;
		case OMX_IndexVendorParamSetResync:
			break;
        case OMX_IndexVendorParamRegisterPID:			
            break;
        case OMX_IndexVendorParamUnRegisterPID:		    
            break;
        case OMX_IndexVendorParamTunerSetNumberOfBB:		    
            break;
        case OMX_IndexVendorParamTunerSetDataPLP:
            break;
        case OMX_IndexVendorConfigSetAntennaPower:
            {
                int iArg = (int)ComponentParameterStructure;
                LinuxTV_Frontend_SetAntennaPower(iArg);
            }
            break;
        case OMX_IndexVendorParamSetVoltage:
            {
                int iArg = (int)ComponentParameterStructure;
                if (LinuxTV_Frontend_SetVoltage(iArg) != 0)
                    omxErr = OMX_ErrorBadParameter;
            }
            break;
        case OMX_IndexVendorParamSetTone:
            {
                int iArg = (int)ComponentParameterStructure;
                if (LinuxTV_Frontend_SetTone(iArg) != 0)
                    omxErr = OMX_ErrorBadParameter;
            }
            break;
        case OMX_IndexVendorParamDiSEqCSendBurst:
            {
                int iArg = (int)ComponentParameterStructure;
                if (LinuxTV_Frontend_DiSEqC_SendBurst(iArg) != 0)
                    omxErr = OMX_ErrorBadParameter;
            }
            break;
        case OMX_IndexVendorParamDiSEqCSendCMD:
            {
                piArg = (int *)ComponentParameterStructure; // piArg[0]: message, piArg[1]: length of message
                if (LinuxTV_Frontend_DiSEqC_SendCMD(piArg[0], piArg[1]) != 0)
                    omxErr = OMX_ErrorBadParameter;
            }
            break;

        case OMX_IndexVendorParamBlindScanReset:
            {
                if (LinuxTV_Frontend_BlindScan_Reset() != 0)
                    omxErr = OMX_ErrorBadParameter;
            }
            break;

        case OMX_IndexVendorParamBlindScanStart:
            {
                if (LinuxTV_Frontend_BlindScan_Start() != 0)
                    omxErr = OMX_ErrorBadParameter;
            }
            break;

        case OMX_IndexVendorParamBlindScanCancel:
            {
                if (LinuxTV_Frontend_BlindScan_Cancel() != 0)
                    omxErr = OMX_ErrorBadParameter;
            }
            break;

		default: /*Call the base component function*/
			return omx_base_component_SetParameter(hComponent, nParamIndex, ComponentParameterStructure);
	}

	return omxErr;
}

OMX_ERRORTYPE omx_linuxtv_tuner_component_GetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_INOUT OMX_PTR ComponentParameterStructure)
{
	OMX_ERRORTYPE omxErr = OMX_ErrorNone;
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE*)hComponent;
	omx_linuxtv_tuner_component_PrivateType* omx_linuxtv_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	int *piArg;
		
	if (ComponentParameterStructure == NULL) 
	{
		return OMX_ErrorBadParameter;
	}
	DEBUG(DEB_LEV_SIMPLE_SEQ, "   Getting parameter %i\n", nParamIndex);

	switch(nParamIndex) 
	{		
		case OMX_IndexVendorParamTunerChannelSet:
			break;

		case OMX_IndexVendorParamTunerChannelSearchStart:
			break;

		case OMX_IndexVendorParamTunerChannelSearchStop:
			break;

		case OMX_IndexVendorParamSetGetPacketDataEntry:
			break;

		case OMX_IndexVendorConfigGetSignalStrength:
			{
				piArg = (unsigned int *)ComponentParameterStructure;

				piArg[0] = LinuxTV_Frontend_GetSignalStrength(); // signal strength (0 ~ 100)
				piArg[1] = LinuxTV_Frontend_GetSignalQuality();  // signal quality (0 ~ 100)
				piArg[2] = 0;//LinuxTV_Frontend_GetSNR();            // snr
				piArg[3] = 0;//LinuxTV_Frontend_GetBER();            // ber

				//if (piArg[4]) sprintf((char *)piArg[4], "STR: %d, QUA: %d, SNR: %d, BER: %d\n", piArg[0], piArg[1], piArg[2], piArg[3]);
			}
			break;

        case OMX_IndexVendorParamBlindScanState:
            {
                piArg = (unsigned int *)ComponentParameterStructure;
                if (LinuxTV_Frontend_BlindScan_GetState(piArg) != 0)
                    omxErr = OMX_ErrorBadParameter;
            }
            break;

        case OMX_IndexVendorParamBlindScanInfo:
            {
                piArg = (unsigned int *)ComponentParameterStructure;

                if (LinuxTV_Frontend_BlindScan_GetInfo(&piArg[0], &piArg[1], &piArg[2], &piArg[3]) != 0)
                    omxErr = OMX_ErrorBadParameter;
            }
            break;			

        case OMX_IndexVendorParamTunerGetDataPLPs:
            break;			
		case OMX_IndexVendorParamTunerGetEWSFlag:
			break;

		default: /*Call the base component function*/
			return omx_base_component_GetParameter(hComponent, nParamIndex, ComponentParameterStructure);
	}

	return omxErr;
}

OMX_ERRORTYPE omx_linuxtv_tuner_component_MessageHandler(OMX_COMPONENTTYPE* openmaxStandComp, internalRequestMessageType *message)
{
	OMX_ERRORTYPE omxErr = OMX_ErrorNone;
	omx_linuxtv_tuner_component_PrivateType* omx_linuxtv_tuner_component_Private = (omx_linuxtv_tuner_component_PrivateType*)openmaxStandComp->pComponentPrivate;  	
	OMX_STATETYPE eCurrentState = omx_linuxtv_tuner_component_Private->state;

	DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s\n", __func__);

	return omxErr;
}

OMX_ERRORTYPE omx_linuxtv_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp)
{
	return (omx_linuxtv_tuner_component_Constructor(openmaxStandComp, "OMX.tcc.broadcast.dxb_tuner"));
}

OMX_ERRORTYPE omx_linuxtv_tuner_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp)
{
	return omx_linuxtv_tuner_component_Destructor(openmaxStandComp);
}
