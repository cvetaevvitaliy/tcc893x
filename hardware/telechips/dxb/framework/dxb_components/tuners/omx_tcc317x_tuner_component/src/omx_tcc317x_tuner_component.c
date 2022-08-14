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

#define LOG_TAG	"OMX_TCC317X_TUNER"
#include <utils/Log.h>

#include <omxcore.h>
#include <OMX_TCC_Index.h>
#include <omx_tcc317x_tuner_component.h>
#include "tcc_dxb_interface_tuner.h"
#include "tdmb_tcc317x_tuner.h"
#include "tdmb_tuner_space.h"
#include "tcc317x_common.h"
#include "tcc_tsif.h"
#include "SPI.h"
#include "fig.h"
#include "tcpal_types.h"

static int giCmdInterface = TCC317X_IF_I2C;
static int giDataInterface = TCC317X_STREAM_IO_STS;

extern TdmbTunerSpace gstTunerSpace;
extern int giTSIFHandle;

#if defined (_STATE_GET_TRANSMISSIONMODE_)
extern unsigned short Transmission_mode;
#endif

static OMX_COMPONENTTYPE *gTunerOpenmaxStandComp = NULL;

/* EWS */
pthread_mutex_t lockEWSHandle;
#define EWS_MESSAGE_LENGTH      416

void omx_ews_lock_init(void)
{
	pthread_mutex_init(&lockEWSHandle, NULL);
}

void omx_ews_lock_deinit(void)
{
	pthread_mutex_destroy(&lockEWSHandle);
}

void omx_ews_lock(void)
{
	pthread_mutex_lock (&lockEWSHandle);
}

void omx_ews_unlock(void)
{
	pthread_mutex_unlock (&lockEWSHandle);
}

/* Driver Open */
pthread_mutex_t lockTcc317xOpenHandle;

void omx_tcc317xopen_lock_init(void)
{
	pthread_mutex_init(&lockTcc317xOpenHandle, NULL);
}

void omx_tcc317xopen_lock_deinit(void)
{
	pthread_mutex_destroy(&lockTcc317xOpenHandle);
}

void omx_tcc317xopen_lock(void)
{
	pthread_mutex_lock (&lockTcc317xOpenHandle);
}

void omx_tcc317xopen_unlock(void)
{
	pthread_mutex_unlock (&lockTcc317xOpenHandle);
}

void omx_tcc317x_tuner_component_BufferMgmtCallback(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* outputbuffer) {
	DEBUG(DEB_LEV_ERR, "[## OMX ##] omx_isdbt_tuner_tcc317x_component_BufferMgmtCallback  \n");
	OMX_S32 data_read;
	omx_tcc317x_tuner_component_PrivateType* omx_tcc317x_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	int cnt;
	int remain_cnt;
	outputbuffer->nFilledLen = 0;
}

void omx_tcc317x_tuner_component_set_channel_notify(void *pResult)
{
	omx_tcc317x_tuner_component_PrivateType* omx_tcc317x_tuner_component_Private = (omx_tcc317x_tuner_component_PrivateType*)gTunerOpenmaxStandComp->pComponentPrivate;
	DEBUG(DEB_LEV_ERR, "[## OMX ##] omx_tcc317x_tuner_component_set_channel_notify  %d\n",pResult);		
	(*(omx_tcc317x_tuner_component_Private->callbacks->EventHandler))
		(gTunerOpenmaxStandComp,
		omx_tcc317x_tuner_component_Private->callbackData,
		OMX_EventVendorSpecificEvent, /* The command was completed */
		OMX_VENDOR_EVENTDATA_TUNER, 
		DxB_TUNER_EVENT_NOTIFY_SET_CHANNEL,
		pResult);
}

void omx_tcc317x_tuner_component_searched_notify(void *pChannelInfo, int searchedCount)
{
	omx_tcc317x_tuner_component_PrivateType* omx_tcc317x_tuner_component_Private =	(omx_tcc317x_tuner_component_PrivateType*)gTunerOpenmaxStandComp->pComponentPrivate;

	DEBUG(DEB_LEV_ERR, "[## OMX ##] omx_tcc317x_tuner_component_searched_notify  \n");
	
	(*(omx_tcc317x_tuner_component_Private->callbacks->EventHandler))
			(gTunerOpenmaxStandComp,
			omx_tcc317x_tuner_component_Private->callbackData,
			OMX_EventVendorSpecificEvent, /* The command was completed */
			OMX_VENDOR_EVENTDATA_TUNER, 
			DxB_TUNER_EVENT_NOTIFY_SEARCHED_CHANNEL,
			pChannelInfo);
}

OMX_ERRORTYPE omx_tcc317x_tuner_component_GetExtensionIndex(  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_STRING cParameterName,  OMX_OUT OMX_INDEXTYPE* pIndexType)
{    	
	DEBUG(DEB_LEV_ERR, "[## OMX ##] omx_tcc317x_tuner_component_GetExtensionIndex  \n");
	return OMX_ErrorNone;  
}

OMX_ERRORTYPE omx_tcc317x_tuner_component_MessageHandler(OMX_COMPONENTTYPE* openmaxStandComp, internalRequestMessageType *message)
{
	omx_tcc317x_tuner_component_PrivateType* omx_tcc317x_tuner_component_Private = (omx_tcc317x_tuner_component_PrivateType*)openmaxStandComp->pComponentPrivate;  
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_STATETYPE eCurrentState = omx_tcc317x_tuner_component_Private->state;
	DEBUG(DEB_LEV_ERR, "[## OMX ##] omx_tcc317x_tuner_component_MessageHandler  \n");
	
	return err;  
}

OMX_ERRORTYPE omx_tcc317x_tuner_component_SetConfig(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nIndex,
  OMX_IN  OMX_PTR pComponentConfigStructure)
{
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_tcc317x_tuner_component_PrivateType* omx_tcc317x_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	int	ret;
	int	channel;

	DEBUG(DEB_LEV_ERR, "[## OMX ##] omx_tcc317x_tuner_component_SetConfig  \n");
	switch (nIndex)
	{		
		default: 
			return omx_base_component_SetConfig(hComponent, nIndex, pComponentConfigStructure);
  	}
  	return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_tcc317x_tuner_component_GetConfig(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nIndex,
  OMX_IN  OMX_PTR pComponentConfigStructure)
{
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_tcc317x_tuner_component_PrivateType* omx_tcc317x_tuner_component_Private = openmaxStandComp->pComponentPrivate;

	DEBUG(DEB_LEV_ERR, "[## OMX ##] omx_tcc317x_tuner_component_GetConfig  \n");
	switch (nIndex)
	{
		default: 
			return omx_base_component_GetConfig(hComponent, nIndex, pComponentConfigStructure);
	}
	return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_tcc317x_tuner_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp){
	DEBUG(DEB_LEV_ERR, "[## OMX ##] omx_tuner_tcc317x_component_Destructor  \n");
	
	omx_tcc317x_tuner_component_PrivateType* omx_tcc317x_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	OMX_U32 i;

	/* frees port/s */
	if (omx_tcc317x_tuner_component_Private->ports) 
	{
		for (i=0; i < omx_tcc317x_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) 
		{
			if(omx_tcc317x_tuner_component_Private->ports[i])
				omx_tcc317x_tuner_component_Private->ports[i]->PortDestructor(omx_tcc317x_tuner_component_Private->ports[i]);
		}
		TCC_free(omx_tcc317x_tuner_component_Private->ports);
		omx_tcc317x_tuner_component_Private->ports=NULL;
	}

	if(omx_tcc317x_tuner_component_Private->ChannelSearchThread)
	{
		pthread_join(omx_tcc317x_tuner_component_Private->ChannelSearchThread,NULL);
		omx_tcc317x_tuner_component_Private->ChannelSearchThread = NULL;
	}


	if(omx_tcc317x_tuner_component_Private->pEWS_Message)
	{
	for(i=0; i<MAX_MESSAGE_CNT; i++)
	{
			st_EWS_MESSAGE *pMessage = omx_tcc317x_tuner_component_Private->pEWS_Message + i;
		
		if(pMessage)
		{
			if(pMessage->EWSMessage)
			{
				TCC_free(pMessage->EWSMessage);
				pMessage->EWSMessage = NULL;
			}
		}
	}

		TCC_free(omx_tcc317x_tuner_component_Private->pEWS_Message);
	}
	omx_tcc317x_tuner_component_Private->pEWS_Message = NULL;
	omx_ews_lock_deinit();
	omx_tcc317xopen_lock_deinit();

	return omx_base_source_Destructor(openmaxStandComp);
}

void omx_tcc317x_tuner_component_ews_notify(OMX_S32 moduleidx, void *pst_EWS)
{
	int Current_SegNum; /* Segment number */
	int Total_SegNum; /* Totol Segment Count */
	int MessageID; /* EWS Message ID */
	TMC_EWSInfo *psTMC_EWS = (TMC_EWSInfo *)pst_EWS;
	st_EWS_MESSAGE *pMessage;

	//DEBUG(DEB_LEV_ERR, "[## OMX ##] omx_tcc317x_tuner_component_ews_notify  \n");
	
	omx_tcc317x_tuner_component_PrivateType* omx_tcc317x_tuner_component_Private = 
		(omx_tcc317x_tuner_component_PrivateType*)gTunerOpenmaxStandComp->pComponentPrivate;

	if(psTMC_EWS->NumMsg<=2)
	{
		usleep(5 * 1000);
		ALOGW("[%s:%d]\n",__func__, __LINE__);
		return;
	}

	Current_SegNum = psTMC_EWS->Message[0]>>4;
	Total_SegNum = (psTMC_EWS->Message[0]&0x0F);
	MessageID = psTMC_EWS->Message[1];

	if(MessageID >= MAX_MESSAGE_CNT)
	{
		usleep(5 * 1000);
		ALOGW("[%s:%d]\n",__func__, __LINE__);
		return;
	}

	pMessage = omx_tcc317x_tuner_component_Private->pEWS_Message + MessageID;
	omx_ews_lock();	

	if(pMessage)
	{
		if(pMessage->EWSMessage == NULL)
		{
			pMessage->EWSMessage = TCC_malloc(EWS_MESSAGE_LENGTH);
			memset(pMessage->EWSMessage, 0x00, EWS_MESSAGE_LENGTH);
		}

		if(Current_SegNum == Total_SegNum && Total_SegNum==0)
		{
			/* complete !*/
			pMessage->EWSMessageLen = 0;
			pMessage->EWSMessageID = MessageID;

			if(pMessage->EWSMessage)
			{
				int iEventData[4];
				memset(pMessage->EWSMessage, 0x00, EWS_MESSAGE_LENGTH);
				memcpy(pMessage->EWSMessage, &psTMC_EWS->Message[2], psTMC_EWS->NumMsg-2);
				pMessage->EWSMessageLen = psTMC_EWS->NumMsg-2;

				iEventData[0] = moduleidx;
				iEventData[1] = pMessage->EWSMessageID;
				iEventData[2] = pMessage->EWSMessageLen;
				iEventData[3] = (int)(pMessage->EWSMessage);
				
				(*(omx_tcc317x_tuner_component_Private->callbacks->EventHandler))
						(gTunerOpenmaxStandComp,
						omx_tcc317x_tuner_component_Private->callbackData,
						OMX_EventVendorSpecificEvent, 
						OMX_VENDOR_EVENTDATA_TUNER, 
						DxB_TUNER_EVENT_NOTIFY_UPDATE_EWS,
						iEventData);
				//TCC_free(pMessage->EWSMessage);
				//pMessage->EWSMessage = NULL;
				pMessage->EWSMessageLen = 0;
				pMessage->EWSMessageID = 0;
			}
		}
		else if(Current_SegNum == Total_SegNum && Total_SegNum != 0 && pMessage->EWSMessageLen!=0)
		{
			/* complete !*/
			pMessage->EWSMessageID = MessageID;
			if(pMessage->EWSMessage)
			{
				int iEventData[4];

				memcpy(&pMessage->EWSMessage[pMessage->EWSMessageLen], &psTMC_EWS->Message[2], psTMC_EWS->NumMsg-2);
				pMessage->EWSMessageLen += psTMC_EWS->NumMsg-2;
				
				iEventData[0] = moduleidx;
				iEventData[1] = pMessage->EWSMessageID;
				iEventData[2] = pMessage->EWSMessageLen;
				iEventData[3] = (int)pMessage->EWSMessage;

				(*(omx_tcc317x_tuner_component_Private->callbacks->EventHandler))
						(gTunerOpenmaxStandComp,
						omx_tcc317x_tuner_component_Private->callbackData,
						OMX_EventVendorSpecificEvent, 
						OMX_VENDOR_EVENTDATA_TUNER, 
						DxB_TUNER_EVENT_NOTIFY_UPDATE_EWS,
						iEventData);
				//TCC_free(pMessage->EWSMessage);
				//pMessage->EWSMessage = NULL;
				pMessage->EWSMessageLen = 0;
				pMessage->EWSMessageID = 0;
			}
		}
		else if(Current_SegNum==0)
		{
			memset(pMessage->EWSMessage, 0x00, EWS_MESSAGE_LENGTH);
			memcpy(pMessage->EWSMessage, &psTMC_EWS->Message[2], psTMC_EWS->NumMsg-2);
			pMessage->EWSMessageLen = (psTMC_EWS->NumMsg-2);
		}
		else if(pMessage->EWSMessageLen != 0)
		{
			memcpy(&pMessage->EWSMessage[pMessage->EWSMessageLen], &psTMC_EWS->Message[2], psTMC_EWS->NumMsg-2);
			pMessage->EWSMessageLen += (psTMC_EWS->NumMsg-2);
		}
		else{/* none */}
	}
	omx_ews_unlock();
}

void omx_tcc317x_tuner_component_data_ch_notify(OMX_S32 moduleidx, OMX_PTR pucData, OMX_U32 uiSize)
{
//	ALOGI("[## OMX ##] omx_tcc317x_tuner_component_data_ch_notify : %dbytes\n", uiSize);

	omx_tcc317x_tuner_component_PrivateType* omx_tcc317x_tuner_component_Private = 
		(omx_tcc317x_tuner_component_PrivateType*)gTunerOpenmaxStandComp->pComponentPrivate;

	int iEventData[3];
	iEventData[0] = moduleidx;
	iEventData[1] = pucData;
	iEventData[2] = uiSize;

	(*(omx_tcc317x_tuner_component_Private->callbacks->EventHandler))
			(gTunerOpenmaxStandComp,
			omx_tcc317x_tuner_component_Private->callbackData,
			OMX_EventVendorSpecificEvent, 
			OMX_VENDOR_EVENTDATA_TUNER, 
			DxB_TUNER_EVENT_NOTIFY_UPDATE_DATA,
			iEventData);
}


OMX_ERRORTYPE omx_tcc317x_tuner_component_GetParameter(
	OMX_IN  OMX_HANDLETYPE hComponent,
	OMX_IN  OMX_INDEXTYPE nParamIndex,
	OMX_INOUT OMX_PTR ComponentParameterStructure){

	OMX_ERRORTYPE err = OMX_ErrorNone;
	
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE*)hComponent;
	omx_tcc317x_tuner_component_PrivateType* omx_tuner_tcc317x_component_Private = openmaxStandComp->pComponentPrivate;
	omx_base_audio_PortType *pPort = (omx_base_audio_PortType *) omx_tuner_tcc317x_component_Private->ports[OMX_BASE_SOURCE_OUTPUTPORT_INDEX]; 
		
	if (ComponentParameterStructure == NULL) 
		return OMX_ErrorBadParameter;

	switch(nParamIndex) 
	{	
		case OMX_IndexVendorParamTunerChannelSet:
			{
				DEBUG(DEB_LEV_ERR, "[## OMX GetParameter ##] OMX_IndexVendorParamTunerChannelSet  \n");
				TUNER_SEARCH_INFO *pSearchInfo;
				pSearchInfo =(TUNER_SEARCH_INFO *)ComponentParameterStructure;
				pSearchInfo->uiCountryCode = omx_tuner_tcc317x_component_Private->countrycode;
				pSearchInfo->uiMinChannel = TDMBSPACE_GetMinChannel();
				pSearchInfo->uiMaxChannel = TDMBSPACE_GetMaxChannel();
				TCC317XTDMB_SetState(0,STATE_IDLE);
			}
			break;

		case OMX_IndexVendorParamTunerChannelSearchStart:				
			DEBUG(DEB_LEV_ERR, "[## OMX GetParameter ##] OMX_IndexVendorParamTunerChannelSearchStart  \n");
			break;

		case OMX_IndexVendorParamTunerChannelSearchStop:
			DEBUG(DEB_LEV_ERR, "[## OMX GetParameter ##] OMX_IndexVendorParamTunerChannelSearchStop  \n");
			break;

		case OMX_IndexVendorParamSetGetPacketDataEntry:
			DEBUG(DEB_LEV_ERR, "[## OMX GetParameter ##] OMX_IndexVendorParamSetGetPacketDataEntry  \n");			
			*(int *)ComponentParameterStructure = (int)TCC317XTDMB_ReadFIFO;			
			break;

		case OMX_IndexVendorConfigGetSignalStrength:
			{
				int *piStrength = (int *)ComponentParameterStructure;

				TCC317X_INFO Tcc317x_info;
				piStrength[0] = TCC317XTDMB_GetSignalStrength(piStrength[3], 0 /*diversityindex*/, &Tcc317x_info);
				piStrength[1] = Tcc317x_info.SNR;
				piStrength[2] = Tcc317x_info.PCBer;
				piStrength[3] = Tcc317x_info.RSSI;
				//*
				piStrength[4] = Tcc317x_info.ViterBer;
				piStrength[5] = Tcc317x_info.FICBer;
				piStrength[6] = Tcc317x_info.MSCBer;
				piStrength[7] = Tcc317x_info.TSPer;
				piStrength[8] = Tcc317x_info.RFLoopGain;
				piStrength[9] = Tcc317x_info.BBLoopGain;
				piStrength[10] = Tcc317x_info.OFDM_Lock;
				piStrength[11] = Tcc317x_info.DSPVer;
				piStrength[12] = Tcc317x_info.DSPBuildDate;
				piStrength[13] = Tcc317x_info.DDVer;
				//*/

				#if 0 /*BaseBand Info*/
				TcpalPrintLog((I08S *) " * DSP Ver (%d.%d.%d), BuildDate: %dy-%dm-%dd %dh, DD Ver (%d.%d.%d)\n",
			                    (Tcc317x_info.DSPVer>>24)&0xFF,
			                    (Tcc317x_info.DSPVer>>16)&0xFF,
			                    (Tcc317x_info.DSPVer>>8)&0xFF,
			                    (Tcc317x_info.DSPBuildDate>>20)&0xFFF,
			                    (Tcc317x_info.DSPBuildDate>>16)&0xF,
			                    (Tcc317x_info.DSPBuildDate>>8)&0xFF,
			                    (Tcc317x_info.DSPBuildDate)&0xFF,
			                    (Tcc317x_info.DDVer>>16)&0xFF,
			                    (Tcc317x_info.DDVer>>8)&0xFF,
			                    (Tcc317x_info.DDVer)&0xFF);
				#endif
			}
			break;	

		case OMX_IndexVendorParamTunerGetEWSFlag:
			DEBUG(DEB_LEV_ERR, "[## OMX GetParameter ##] OMX_IndexVendorParamTunerGetEWSFlag  \n");
			{
				int *puiStr = (unsigned int *)ComponentParameterStructure;
				*puiStr = 0;
			}
			break;

		default: /*Call the base component function*/
			DEBUG(DEB_LEV_ERR, "[## OMX GetParameter ##] Default  \n");
			return omx_base_component_GetParameter(hComponent, nParamIndex, ComponentParameterStructure);
			break;
	}
	return err;

}

OMX_ERRORTYPE omx_tcc317x_tuner_component_SetParameter(
	OMX_IN  OMX_HANDLETYPE hComponent,
	OMX_IN  OMX_INDEXTYPE nParamIndex,
	OMX_IN  OMX_PTR ComponentParameterStructure){

	int err = OMX_ErrorNone;
	int channel, freq, moduleidx;
	int *piArg, *piChInfo;	
	
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE*)hComponent;
	omx_tcc317x_tuner_component_PrivateType* omx_tcc317x_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	omx_base_audio_PortType* pPort = (omx_base_audio_PortType *) omx_tcc317x_tuner_component_Private->ports[OMX_BASE_SOURCE_OUTPUTPORT_INDEX];

	switch(nParamIndex) 
	{
	case OMX_IndexVendorParamTunerDeviceSet:
		DEBUG(DEB_LEV_ERR, "[## OMX SetParameter ##] OMX_IndexVendorParamTunerDeviceSet\n");
		break;

	case OMX_IndexVendorParamTunerChannelScan:
		piArg = (int *)ComponentParameterStructure;
		channel = (int)piArg[0];
		moduleidx = (int)piArg[1];
		err = TCC317XTDMB_ChannelScan(moduleidx, channel);
		DEBUG(DEB_LEV_ERR, "[## OMX SetParameter ##] OMX_IndexVendorParamTunerChannelScan --> SCAN ret (%d)\n", err);	
		break;

	case OMX_IndexVendorParamTunerFreqScan:
		piArg = (int *)ComponentParameterStructure;
		freq = (int)piArg[0];
		moduleidx = (int)piArg[1];		
		err = TCC317XTDMB_ChannelFreqScan(moduleidx, freq);
		DEBUG(DEB_LEV_ERR, "[## OMX SetParameter ##] OMX_IndexVendorParamTunerFreqScan --> SCAN ret (%d)\n", err);
		break;

	case OMX_IndexVendorParamTunerChannelSet:
		piArg = (int *)ComponentParameterStructure;
		piChInfo = (int *)piArg[0];
#ifdef _MULTI_SERVICE_
		err = TCC317XTDMB_ChannelSet(piChInfo[12], (unsigned int *)piArg[0], piArg[1], piArg[2]/*Reg(1), Un-Reg(0), Change Frquency (2) */);
#else
		err = TCC317XTDMB_ChannelSet(piChInfo[12], (unsigned int *)piArg[0], piArg[1]);
#endif
		DEBUG(DEB_LEV_ERR, "[## OMX SetParameter ##] OMX_IndexVendorParamTunerChannelSet::%x\n", err);
		break;
		
	case OMX_IndexVendorParamTunerGetChannelIndexByFrequency:
		DEBUG(DEB_LEV_ERR, "[## OMX SetParameter ##] OMX_IndexVendorParamTunerGetChannelIndexByFrequency\n");
		break;

	case OMX_IndexVendorParamTunerFrequencySet:
		DEBUG(DEB_LEV_ERR, "[## OMX SetParameter ##] OMX_IndexVendorParamTunerFrequencySet\n");
		break;	

	case OMX_IndexVendorParamTunerCountryCodeSet:
		DEBUG(DEB_LEV_ERR, "[## OMX SetParameter ##] OMX_IndexVendorParamTunerCountryCodeSet\n");
		omx_tcc317x_tuner_component_Private->countrycode = (int)ComponentParameterStructure;				
		break;
 
	case OMX_IndexVendorParamTunerOpen:
		omx_tcc317xopen_lock();
		piArg = (int *)ComponentParameterStructure;
		omx_tcc317x_tuner_component_Private->countrycode = piArg[0]; // TEST: 0x02 BAND3
		moduleidx = piArg[2];
		TDMBSPACE_CTuneSpace(omx_tcc317x_tuner_component_Private->countrycode);
		DEBUG(DEB_LEV_ERR, "[## OMX SetParameter ##] OMX_IndexVendorParamTunerOpen --> (Driver Init:%d)\n", moduleidx);

		err = TCC317XTDMB_Init(moduleidx, omx_tcc317x_tuner_component_Private->countrycode, giCmdInterface, giDataInterface);
		if(err == 0){			
			if(TCC317XTDMB_OpenFlagGet(0) == 0 && moduleidx == 0 || TCC317XTDMB_OpenFlagGet(1) == 0 && moduleidx == 1)
				err = TCC317XTDMB_StreamInterface(moduleidx, &piArg[2], giDataInterface);
			
			TCC317XTDMB_OpenFlagSet(moduleidx, 1);
		}
		
		if(err == OMX_ErrorBadParameter)
			TCC317XTDMB_Close(moduleidx, giCmdInterface);

		omx_tcc317xopen_unlock();
		break;
		
	case OMX_IndexVendorParamTunerClose:
		moduleidx = (int)ComponentParameterStructure;
		DEBUG(DEB_LEV_ERR, "[## OMX SetParameter ##] OMX_IndexVendorParamTunerClose : ( %d )\n", moduleidx);
		if(TCC317XTDMB_OpenFlagGet(moduleidx) == 1)
			TCC317XTDMB_Close(moduleidx, giCmdInterface);
		break;

	case OMX_IndexVendorParamSetResync:
		DEBUG(DEB_LEV_ERR, "[## OMX SetParameter ##] OMX_IndexVendorParamSetResync\n");
		break;

	case OMX_IndexVendorParamRegisterPID:
		{
			int pid = (int)ComponentParameterStructure;			
			DEBUG(DEB_LEV_ERR, "[## OMX SetParameter ##] OMX_IndexVendorParamRegisterPID --> PID: (0x%x)\n",pid);
		}
		break;	

	case OMX_IndexVendorParamUnRegisterPID:
		{
			int pid = (int)ComponentParameterStructure;
			DEBUG(DEB_LEV_ERR, "[## OMX SetParameter ##] OMX_IndexVendorParamUnRegisterPID --> PID: (0x%x)\n",pid);
		}
		break;
				
	case OMX_IndexVendorParamTunerSetNumberOfBB:       	
		DEBUG(DEB_LEV_ERR, "[## OMX SetParameter ##] OMX_IndexVendorParamTunerSetNumberOfBB \n");
		break;				

	case OMX_IndexVendorConfigSetAntennaPower:
		DEBUG(DEB_LEV_ERR, "[## OMX SetParameter ##] OMX_IndexVendorConfigSetAntennaPower\n");
		break;

	case OMX_IndexVendorParamTunerPlayStop:
		moduleidx = (int)ComponentParameterStructure;
		DEBUG(DEB_LEV_ERR, "[## OMX SetParameter ##] OMX_IndexVendorParamTunerPlayStop:%d\n", moduleidx);
		TCC317XTDMB_StreamThreadHold(moduleidx);
		break;

	default: /*Call the base component function*/
		DEBUG(DEB_LEV_ERR, "[## OMX GetParameter ##] Default  \n");
		return omx_base_component_SetParameter(hComponent, nParamIndex, ComponentParameterStructure);
	}
	return err;
}


OMX_ERRORTYPE omx_tcc317x_tuner_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,OMX_STRING cComponentName) 
{	
	int omxErr;
	omx_base_audio_PortType *pPort;
	omx_tcc317x_tuner_component_PrivateType* omx_tcc317x_tuner_component_Private;
	OMX_U32 i;
	char *pcm_name;

	DEBUG(DEB_LEV_ERR, "[## OMX ##] omx_tcc317x_tuner_component_Constructor  \n");

	if (!openmaxStandComp->pComponentPrivate)
	{
		openmaxStandComp->pComponentPrivate = TCC_calloc(1, sizeof(omx_tcc317x_tuner_component_PrivateType));
		if(openmaxStandComp->pComponentPrivate==NULL) 
		{
			return OMX_ErrorInsufficientResources;
		}
	}

	omx_tcc317x_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	omx_tcc317x_tuner_component_Private->ports = NULL;

	omxErr = omx_base_source_Constructor(openmaxStandComp,cComponentName);
	if (omxErr != OMX_ErrorNone)
	{
		return OMX_ErrorInsufficientResources;
	}

	omx_tcc317x_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nStartPortNumber = 0;
	omx_tcc317x_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts = 1;

	/** Allocate Ports and call port constructor. */  
	if (omx_tcc317x_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts && !omx_tcc317x_tuner_component_Private->ports) 
	{
		omx_tcc317x_tuner_component_Private->ports = TCC_calloc(omx_tcc317x_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts, sizeof(omx_base_audio_PortType *));
		if (!omx_tcc317x_tuner_component_Private->ports) 
		{
			return OMX_ErrorInsufficientResources;
		}
		for (i=0; i < omx_tcc317x_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) 
		{
			omx_tcc317x_tuner_component_Private->ports[i] = TCC_calloc(1, sizeof(omx_base_audio_PortType));
			if (!omx_tcc317x_tuner_component_Private->ports[i]) 
			{
				return OMX_ErrorInsufficientResources;
			}
		}
	}

	base_audio_port_Constructor(openmaxStandComp, &omx_tcc317x_tuner_component_Private->ports[0], 0, OMX_FALSE);
	pPort = (omx_base_audio_PortType *) omx_tcc317x_tuner_component_Private->ports[OMX_BASE_SOURCE_OUTPUTPORT_INDEX];
	
	omx_tcc317x_tuner_component_Private->BufferMgmtCallback = omx_tcc317x_tuner_component_BufferMgmtCallback;
	omx_tcc317x_tuner_component_Private->messageHandler = omx_tcc317x_tuner_component_MessageHandler;	
	omx_tcc317x_tuner_component_Private->destructor = omx_tcc317x_tuner_component_Destructor;

	openmaxStandComp->SetParameter  = omx_tcc317x_tuner_component_SetParameter;
	openmaxStandComp->GetParameter  = omx_tcc317x_tuner_component_GetParameter;
	openmaxStandComp->SetConfig = omx_tcc317x_tuner_component_SetConfig;
	openmaxStandComp->GetConfig = omx_tcc317x_tuner_component_GetConfig;
	openmaxStandComp->GetExtensionIndex = omx_tcc317x_tuner_component_GetExtensionIndex;

	gTunerOpenmaxStandComp = openmaxStandComp;
	giTSIFHandle = -1;

	if(strcmp(cComponentName, BROADCAST_TCC317X_CSPI_STS_TUNER_NAME) == 0 ){
		giCmdInterface = TCC317X_IF_TCCSPI; 
		giDataInterface = TCC317X_STREAM_IO_STS;
	} 
	else if(strcmp(cComponentName, BROADCAST_TCC317X_I2C_STS_TUNER_NAME) == 0 ){
		giCmdInterface = TCC317X_IF_I2C;
		giDataInterface = TCC317X_STREAM_IO_STS;
	}else{
		giCmdInterface = TCC317X_IF_TCCSPI; 
		giDataInterface = TCC317X_STREAM_IO_STS;
    	}

	omx_tcc317x_tuner_component_Private->pEWS_Message = (st_EWS_MESSAGE *)TCC_malloc(sizeof(st_EWS_MESSAGE) * MAX_MESSAGE_CNT);
	memset(omx_tcc317x_tuner_component_Private->pEWS_Message, 0x00, sizeof(st_EWS_MESSAGE) * MAX_MESSAGE_CNT);
	omx_ews_lock_init();
	omx_tcc317xopen_lock_init();
 	
	return OMX_ErrorNone;
}


OMX_ERRORTYPE omx_tcc317x_I2C_STS_tuner_component_Init (OMX_COMPONENTTYPE *openmaxStandComp) 
{	
	return (omx_tcc317x_tuner_component_Constructor(openmaxStandComp, BROADCAST_TCC317X_I2C_STS_TUNER_NAME));
}

OMX_ERRORTYPE omx_tcc317x_CSPI_STS_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp) 
{
	return (omx_tcc317x_tuner_component_Constructor(openmaxStandComp, BROADCAST_TCC317X_CSPI_STS_TUNER_NAME));
}

OMX_ERRORTYPE omx_tcc317x_CSPI_SPIMS_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp) 
{
	return (omx_tcc317x_tuner_component_Constructor(openmaxStandComp, BROADCAST_TCC317X_CSPI_SPIMS_NAME));
}

OMX_ERRORTYPE omx_tcc317x_I2C_SPIMS_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp) 
{
	return (omx_tcc317x_tuner_component_Constructor(openmaxStandComp, BROADCAST_TCC317X_I2C_SPIMS_NAME));
}

OMX_ERRORTYPE omx_tcc317x_tuner_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp)
{
	return omx_tcc317x_tuner_component_Destructor(openmaxStandComp);
}

