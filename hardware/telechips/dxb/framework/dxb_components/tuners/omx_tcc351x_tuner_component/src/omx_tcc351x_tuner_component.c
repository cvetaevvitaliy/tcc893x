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

#define LOG_TAG	"OMX_TCC351X"
#include <utils/Log.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <omxcore.h>
#include <omx_tcc351x_tuner_component.h>
#include "tcc_dxb_interface_tuner.h"
#include <OMX_TCC_Index.h>
#include "tcc_tsif.h"
#include "tcc_dxb_control.h"
#include "omx_tcc_thread.h" 

#include "TC3X_Common.h"

#include "TCC351X_dvbt.h"
#include "TCC351X_isdbt.h"
#include "TCC351X_tdmb.h"

#include "fig.h"

#define TSIF_PACKETSIZE			(188)
#define TSIF_MAX_PACKETCOUNT	(8190)
//#define TSIF_MAX_PACKETCOUNT	(5577)
#define TSIF_INT_PACKETCOUNT	(39)
#define EWS_MESSAGE_LENGTH      416

static pthread_t TCC351XThreadID;
static int giTSIFHandle = 0;
static int giCmdInterface = 0;
static int giDataInterface = 0;
static int giNumberOfBaseBand = 1; //default 1
static OMX_COMPONENTTYPE *gTunerOpenmaxStandComp = NULL;

extern long long g_FIC_Tick_ms;
extern FIC_USER_INFO_T gFicParserInfo[MAX_DAB_BB_NUM];

#define MAX_MESSAGE_CNT	256
st_EWS_MESSAGE *g_EWS_Message;//[MAX_MESSAGE_CNT] = {0};

pthread_mutex_t		lockEWSHandle;

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

void omx_tcc351x_tuner_component_searched_notify(void *pChannelInfo, int searchedCount)
{
	omx_tcc351x_tuner_component_PrivateType* omx_tcc351x_tuner_component_Private = (omx_tcc351x_tuner_component_PrivateType*)gTunerOpenmaxStandComp->pComponentPrivate;
	DEBUG(DEFAULT_MESSAGES,"%s %d \n",__func__, searchedCount);	
	(*(omx_tcc351x_tuner_component_Private->callbacks->EventHandler))
			(gTunerOpenmaxStandComp,
			omx_tcc351x_tuner_component_Private->callbackData,
			OMX_EventVendorSpecificEvent, /* The command was completed */
			OMX_VENDOR_EVENTDATA_TUNER, 
			DxB_TUNER_EVENT_NOTIFY_SEARCHED_CHANNEL,
			pChannelInfo);
}

void omx_tcc351x_tuner_component_set_channel_notify(int setResult)
{
	omx_tcc351x_tuner_component_PrivateType* omx_tcc351x_tuner_component_Private = (omx_tcc351x_tuner_component_PrivateType*)gTunerOpenmaxStandComp->pComponentPrivate;

	DEBUG(DEFAULT_MESSAGES,"%s %d \n", __func__, setResult);
		
	(*(omx_tcc351x_tuner_component_Private->callbacks->EventHandler))
		(gTunerOpenmaxStandComp,
		omx_tcc351x_tuner_component_Private->callbackData,
		OMX_EventVendorSpecificEvent, /* The command was completed */
		OMX_VENDOR_EVENTDATA_TUNER, 
		DxB_TUNER_EVENT_NOTIFY_SET_CHANNEL,
		setResult);
}

void omx_tcc351x_tuner_component_ews_notify(void *pst_EWS)
{
	int Current_SegNum;
	int Total_SegNum;
	int MessageID;
	TMC_EWSInfo *psTMC_EWS = (TMC_EWSInfo *)pst_EWS;
	st_EWS_MESSAGE *pMessage;

	omx_tcc351x_tuner_component_PrivateType* omx_tcc351x_tuner_component_Private = (omx_tcc351x_tuner_component_PrivateType*)gTunerOpenmaxStandComp->pComponentPrivate;

	//ALOGW("todo. 3. Please insert EWS Processing \n");

	//return;

	if(psTMC_EWS->NumMsg<=2)
	{
		usleep(5 * 1000);
		ALOGW("[%s:%d]\n",__func__, __LINE__);
		return;
	}

	Current_SegNum = psTMC_EWS->Message[0]>>4;     // 현재 세그먼트 번호
	Total_SegNum = (psTMC_EWS->Message[0]&0x0F);   // 전체세그먼트 수
	MessageID = psTMC_EWS->Message[1];             // 재난 메시지 ID
#if 0
	ALOGW("Current_SegNum = %d", Current_SegNum);
	ALOGW("Total_SegNum = %d", Total_SegNum);
	ALOGW("MessageID = %d", MessageID);
#endif
	if(MessageID >= MAX_MESSAGE_CNT)
	{
		usleep(5 * 1000);
		ALOGW("[%s:%d]\n",__func__, __LINE__);
		return;
	}

	pMessage = omx_tcc351x_tuner_component_Private->pEWS_Message + MessageID;

	omx_ews_lock();

	//ALOGW("[%s:%d] Current_SegNum = %d, Total_SegNum = %d, EWSMessageLen = %d\n",__func__, __LINE__, Current_SegNum, Total_SegNum, omx_tcc351x_tuner_component_Private->pEWS_Message[MessageID].EWSMessageLen);

	if(pMessage)
	{
		if(pMessage->EWSMessage == NULL)
		{
			pMessage->EWSMessage = TCC_malloc(EWS_MESSAGE_LENGTH);
			memset(pMessage->EWSMessage, 0x00, EWS_MESSAGE_LENGTH);
		}

		if(Current_SegNum == Total_SegNum && Total_SegNum==0)
		{
			// complete !!!
		#if 0			
			if(pMessage->EWSMessageID >= 0)
			{
				ALOGW("[%s:%d]\n",__func__, __LINE__);
				return;
			}
		#endif
			pMessage->EWSMessageLen = 0;
			pMessage->EWSMessageID = MessageID;

			if(pMessage->EWSMessage)
			{
				int iEventData[3];
				memset(pMessage->EWSMessage, 0x00, EWS_MESSAGE_LENGTH);
				memcpy(pMessage->EWSMessage, &psTMC_EWS->Message[2], psTMC_EWS->NumMsg-2);
				pMessage->EWSMessageLen = psTMC_EWS->NumMsg-2;

				iEventData[0] = pMessage->EWSMessageID;
				iEventData[1] = pMessage->EWSMessageLen;
				iEventData[2] = (int)(pMessage->EWSMessage);

//				DEBUG(DEFAULT_MESSAGES,"[%s] %s, %d \n",__func__, pMessage->EWSMessage, pMessage->EWSMessageLen);
				(*(omx_tcc351x_tuner_component_Private->callbacks->EventHandler))
						(gTunerOpenmaxStandComp,
						omx_tcc351x_tuner_component_Private->callbackData,
						OMX_EventVendorSpecificEvent, /* The command was completed */
						OMX_VENDOR_EVENTDATA_TUNER, 
						DxB_TUNER_EVENT_NOTIFY_UPDATE_EWS,
						iEventData);
//				TCC_free(pMessage->EWSMessage);
//				pMessage->EWSMessage = NULL;
				pMessage->EWSMessageLen = 0;
				pMessage->EWSMessageID = 0;
			}
		}
		else if(Current_SegNum == Total_SegNum && Total_SegNum != 0 && pMessage->EWSMessageLen!=0)
		{
			// complete !!!
		#if 0
			if(pMessage->EWSMessageID >= 0)
			{
				ALOGW("[%s:%d]\n",__func__, __LINE__);
				return;
			}
		#endif

			pMessage->EWSMessageID = MessageID;

			if(pMessage->EWSMessage)
			{
				int iEventData[3];
				//ALOGW("[%s:%d]\n",__func__, __LINE__);
				memcpy(&pMessage->EWSMessage[pMessage->EWSMessageLen], &psTMC_EWS->Message[2], psTMC_EWS->NumMsg-2);
				pMessage->EWSMessageLen += psTMC_EWS->NumMsg-2;
				
				iEventData[0] = pMessage->EWSMessageID;
				iEventData[1] = pMessage->EWSMessageLen;
				iEventData[2] = (int)pMessage->EWSMessage;

//				DEBUG(DEFAULT_MESSAGES,"[%s] %s, %d \n",__func__, pMessage->EWSMessage, pMessage->EWSMessageLen); 
				(*(omx_tcc351x_tuner_component_Private->callbacks->EventHandler))
						(gTunerOpenmaxStandComp,
						omx_tcc351x_tuner_component_Private->callbackData,
						OMX_EventVendorSpecificEvent, /* The command was completed */
						OMX_VENDOR_EVENTDATA_TUNER, 
						DxB_TUNER_EVENT_NOTIFY_UPDATE_EWS,
						iEventData);
//				TCC_free(pMessage->EWSMessage);
//				pMessage->EWSMessage = NULL;
				pMessage->EWSMessageLen = 0;
				pMessage->EWSMessageID = 0;
			}
		}
		else if(Current_SegNum==0)
		{
			//ALOGW("[%s:%d]\n",__func__, __LINE__);
			memset(pMessage->EWSMessage, 0x00, EWS_MESSAGE_LENGTH);
			memcpy(pMessage->EWSMessage, &psTMC_EWS->Message[2], psTMC_EWS->NumMsg-2);
			pMessage->EWSMessageLen = (psTMC_EWS->NumMsg-2);
		}
		else if(pMessage->EWSMessageLen != 0)
		{
			//ALOGW("[%s:%d]\n",__func__, __LINE__);
			memcpy(&pMessage->EWSMessage[pMessage->EWSMessageLen], &psTMC_EWS->Message[2], psTMC_EWS->NumMsg-2);
			pMessage->EWSMessageLen += (psTMC_EWS->NumMsg-2);
		}
		else
		{
			// none
		}
	}

	omx_ews_unlock();
}

void omx_tdmbtuner_channel_search_thread(void *param)
{
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE*)param;
	omx_tcc351x_tuner_component_PrivateType* omx_tcc351x_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	//Only TDMB is valid
	TCC351XTDMB_SearchChannels();				
	pthread_exit(omx_tcc351x_tuner_component_Private->ChannelSearchThread);
	omx_tcc351x_tuner_component_Private->ChannelSearchThread = NULL;
}

void tcc351x_tdmb_thread(void *arg)
{
	int ret;
	unsigned char *pucMSCDMABuf;
	pucMSCDMABuf = malloc(TSIF_PACKETSIZE);
	while(giTSIFHandle>=0)
	{
		ret = TCC_TSIF_Read(giTSIFHandle, pucMSCDMABuf, TSIF_PACKETSIZE);
		if( ret == TSIF_PACKETSIZE )
		{
			TCC351XTDMB_ParseData(pucMSCDMABuf, TSIF_PACKETSIZE);
		}
		else
		{
		    long long Gap;
		    Gap = TCC351XTDMB_Util_GetTimeInterval(g_FIC_Tick_ms);
            if(Gap>1000000)
            {
                // If FIC Data doesn't come within a 1sec, BB status is resynced status.
                TC3X_API_Reset_MonitorValue (0, 0);
                g_FIC_Tick_ms = TC3X_IO_GET_TIMECNT_ms();
            }
            
		}
	}
	free(pucMSCDMABuf);
}

int DXBBOARD_SelRfPath(int dxb_standard)
{
    int handle, ret = FALSE;
    unsigned int flag = DXB_RF_PATH_UHF;
    static unsigned int prev_flag = 0;
    handle = open("/dev/tcc_dxb_ctrl", O_RDWR|O_NDELAY);
    if(handle < 0)
    {
        ALOGD("[%s:%d] error:%d\n", __func__, __LINE__, errno);
        return ret;
    }

    switch(dxb_standard)
    {
        case TCC351X_TDMB:
            flag = DXB_RF_PATH_VHF;
            break;
        case TCC351X_DVBT:
        case TCC351X_ISDBT:
            flag = DXB_RF_PATH_UHF;
            break;
    }

    if( 0 != ioctl(handle, IOCTL_DXB_CTRL_RF_PATH, &flag))
    {
        ALOGD("[%s:%d] error:%d\n", __func__, __LINE__, errno);
        goto exit_sel_rf_path;
    }
    prev_flag = flag;
    ret = TRUE;

exit_sel_rf_path:
    close(handle);
    return ret;
}


OMX_ERRORTYPE omx_tcc351x_tuner_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,OMX_STRING cComponentName) 
{
	int err;
	int omxErr;
	omx_base_audio_PortType *pPort;
	omx_tcc351x_tuner_component_PrivateType* omx_tcc351x_tuner_component_Private;
	OMX_U32 i;
	char *pcm_name;

	DEBUG(DEFAULT_MESSAGES, "omx_tcc351x_tuner_component_Constructor  \n");

	if (!openmaxStandComp->pComponentPrivate)
	{
		openmaxStandComp->pComponentPrivate = TCC_calloc(1, sizeof(omx_tcc351x_tuner_component_PrivateType));
		if(openmaxStandComp->pComponentPrivate==NULL) 
		{
			return OMX_ErrorInsufficientResources;
		}
	}

	omx_tcc351x_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	omx_tcc351x_tuner_component_Private->ports = NULL;

	omxErr = omx_base_source_Constructor(openmaxStandComp,cComponentName);
	if (omxErr != OMX_ErrorNone)
	{
		return OMX_ErrorInsufficientResources;
	}

	omx_tcc351x_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nStartPortNumber = 0;
	omx_tcc351x_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts = 1;

	/** Allocate Ports and call port constructor. */  
	if (omx_tcc351x_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts && !omx_tcc351x_tuner_component_Private->ports) 
	{
		omx_tcc351x_tuner_component_Private->ports = TCC_calloc(omx_tcc351x_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts, sizeof(omx_base_audio_PortType *));
		if (!omx_tcc351x_tuner_component_Private->ports) 
		{
			return OMX_ErrorInsufficientResources;
		}
		for (i=0; i < omx_tcc351x_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) 
		{
			omx_tcc351x_tuner_component_Private->ports[i] = TCC_calloc(1, sizeof(omx_base_audio_PortType));
			if (!omx_tcc351x_tuner_component_Private->ports[i]) 
			{
				return OMX_ErrorInsufficientResources;
			}
		}
	}
	base_audio_port_Constructor(openmaxStandComp, &omx_tcc351x_tuner_component_Private->ports[0], 0, OMX_FALSE);
	pPort = (omx_base_audio_PortType *) omx_tcc351x_tuner_component_Private->ports[OMX_BASE_SOURCE_OUTPUTPORT_INDEX];
	
	omx_tcc351x_tuner_component_Private->BufferMgmtCallback = omx_tcc351x_tuner_component_BufferMgmtCallback;
	omx_tcc351x_tuner_component_Private->messageHandler = omx_tcc351x_tuner_component_MessageHandler;	
	omx_tcc351x_tuner_component_Private->destructor = omx_tcc351x_tuner_component_Destructor;

	openmaxStandComp->SetParameter  = omx_tcc351x_tuner_component_SetParameter;
	openmaxStandComp->GetParameter  = omx_tcc351x_tuner_component_GetParameter;

	openmaxStandComp->SetConfig = omx_tcc351x_tuner_component_SetConfig;
	openmaxStandComp->GetConfig = omx_tcc351x_tuner_component_GetConfig;

	openmaxStandComp->GetExtensionIndex = omx_tcc351x_tuner_component_GetExtensionIndex;
	gTunerOpenmaxStandComp = openmaxStandComp;
	giTSIFHandle = -1;

	giCmdInterface = 0; //CSPI
	giDataInterface = 0;
    giNumberOfBaseBand = 1;

	if((strcmp(cComponentName, BROADCAST_TCC351X_CSPI_STS_NAME) == 0 ) || (strcmp(cComponentName, BROADCAST_TCC351X_CSPI_TSIFMOD_NAME) == 0 ))
	{
		giCmdInterface = TC3X_IF_CSPI; 
		giDataInterface = TC3X_STREAM_IO_STS;
	} 
	else if(strcmp(cComponentName, BROADCAST_TCC351X_I2C_STS_NAME) == 0 )
	{
		giCmdInterface = TC3X_IF_I2C;
		giDataInterface = TC3X_STREAM_IO_STS;
	}
	else if(strcmp(cComponentName, BROADCAST_TCC351X_I2C_SPIMS_NAME) == 0 )
	{
		giCmdInterface = TC3X_IF_I2C;
		giDataInterface = TC3X_STREAM_IO_SPIMS;
	}
	else if(strcmp(cComponentName, BROADCAST_TCC351X_CSPI_SPIMS_NAME) == 0 )
	{
		giCmdInterface = TC3X_IF_CSPI;
		giDataInterface = TC3X_STREAM_IO_SPIMS;
	}

	omx_tcc351x_tuner_component_Private->ChannelSearchThread = NULL;

	omx_tcc351x_tuner_component_Private->pEWS_Message = (st_EWS_MESSAGE *)TCC_malloc(sizeof(st_EWS_MESSAGE) * MAX_MESSAGE_CNT);
	memset(omx_tcc351x_tuner_component_Private->pEWS_Message, 0x00, sizeof(st_EWS_MESSAGE) * MAX_MESSAGE_CNT);
	omx_ews_lock_init();

	return OMX_ErrorNone;
}

/** The Destructor 
 */
OMX_ERRORTYPE omx_tcc351x_tuner_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp)
{
	omx_tcc351x_tuner_component_PrivateType* omx_tcc351x_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	OMX_U32 i;

	/* frees port/s */
	if (omx_tcc351x_tuner_component_Private->ports) 
	{
		for (i=0; i < omx_tcc351x_tuner_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) 
		{
			if(omx_tcc351x_tuner_component_Private->ports[i])
				omx_tcc351x_tuner_component_Private->ports[i]->PortDestructor(omx_tcc351x_tuner_component_Private->ports[i]);
		}
		TCC_free(omx_tcc351x_tuner_component_Private->ports);
		omx_tcc351x_tuner_component_Private->ports=NULL;
	}

	if(omx_tcc351x_tuner_component_Private->ChannelSearchThread)
	{
		pthread_join(omx_tcc351x_tuner_component_Private->ChannelSearchThread,NULL);
		omx_tcc351x_tuner_component_Private->ChannelSearchThread = NULL;
	}


	if(omx_tcc351x_tuner_component_Private->pEWS_Message)
	{
	for(i=0; i<MAX_MESSAGE_CNT; i++)
	{
			st_EWS_MESSAGE *pMessage = omx_tcc351x_tuner_component_Private->pEWS_Message + i;
		
		if(pMessage)
		{
			if(pMessage->EWSMessage)
			{
				TCC_free(pMessage->EWSMessage);
				pMessage->EWSMessage = NULL;
			}
		}
	}

		TCC_free(omx_tcc351x_tuner_component_Private->pEWS_Message);
	}
	omx_tcc351x_tuner_component_Private->pEWS_Message = NULL;

	omx_ews_lock_deinit();

	return omx_base_source_Destructor(openmaxStandComp);

}

/** 
 * This function plays the input buffer. When fully consumed it returns.
 */

void omx_tcc351x_tuner_component_BufferMgmtCallback(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* outputbuffer) 
{
	OMX_S32 data_read;
	omx_tcc351x_tuner_component_PrivateType* omx_tcc351x_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	int cnt;
	int remain_cnt;
	outputbuffer->nFilledLen = 0;
	outputbuffer->nFilledLen = 0;
	
	
}

OMX_ERRORTYPE omx_tcc351x_tuner_component_GetExtensionIndex(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_STRING cParameterName,
  OMX_OUT OMX_INDEXTYPE* pIndexType)
{    
	
	DEBUG(DEB_LEV_FUNCTION_NAME,"In  %s \n",__func__);
	return OMX_ErrorNone;  
}


/** setting configurations */
OMX_ERRORTYPE omx_tcc351x_tuner_component_SetConfig(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nIndex,
  OMX_IN  OMX_PTR pComponentConfigStructure)
{
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_tcc351x_tuner_component_PrivateType* omx_tcc351x_tuner_component_Private = openmaxStandComp->pComponentPrivate;
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
OMX_ERRORTYPE omx_tcc351x_tuner_component_GetConfig(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nIndex,
  OMX_IN  OMX_PTR pComponentConfigStructure)
{
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_tcc351x_tuner_component_PrivateType* omx_tcc351x_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	
	switch (nIndex)
	{
		default: // delegate to superclass
			return omx_base_component_GetConfig(hComponent, nIndex, pComponentConfigStructure);
	}
	return OMX_ErrorNone;
}



OMX_ERRORTYPE omx_tcc351x_tuner_component_SetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_IN  OMX_PTR ComponentParameterStructure)
{
	int err;
	int omxErr = OMX_ErrorNone;
	OMX_OTHER_PARAM_PORTFORMATTYPE *pOtherPortFormat;
	OMX_U32 portIndex;
	int channel, freq, moduleidx;
	int cnt;
	int *piArg;

	/* Check which structure we are being fed and make control its header */
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE*)hComponent;
	omx_tcc351x_tuner_component_PrivateType* omx_tcc351x_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	omx_base_audio_PortType* pPort = (omx_base_audio_PortType *) omx_tcc351x_tuner_component_Private->ports[OMX_BASE_SOURCE_OUTPUTPORT_INDEX];

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
			if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_TDMB)
			{
				piArg = (int *)ComponentParameterStructure;
				channel = (int)piArg[0];
				moduleidx = (int)piArg[1];
				err = TCC351XTDMB_ChannelIndexScan(channel);
				if(err != 0)
				{
					DEBUG(DEB_LEV_ERR, "Fail to Change channel %d\n", channel);
					omxErr = OMX_ErrorBadParameter;
				}
			}	
			else
				omxErr = OMX_ErrorBadParameter;
			break;

		case OMX_IndexVendorParamTunerFreqScan:
			piArg = (int *)ComponentParameterStructure;
			freq = (int)piArg[0];
			moduleidx = (int)piArg[1];		
			err = TCC351XTDMB_FrequencyScan(freq);
			DEBUG(DEB_LEV_ERR, "[## OMX SetParameter ##] OMX_IndexVendorParamTunerFreqScan --> SCAN ret (%d)\n", err);
			break;

		case OMX_IndexVendorParamTunerChannelSet:		
			if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_DVBT)
			{
			    //Not Support!!!, USE OMX_IndexVendorParamTunerFrequencySet
				omxErr = OMX_ErrorBadParameter;
			}
			else if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_ISDBT)
			{
				piArg = (int *)ComponentParameterStructure;
				err = TCC351XISDBT_ChannelSet(piArg[0], piArg[1]); // piArg[0]: channel, piArg[1]: lockOn
				if(err != 0)	
				{
					DEBUG(DEB_LEV_ERR, "Fail to Change channel %d\n", piArg[0]);
					omxErr = OMX_ErrorBadParameter;
				}
			}
			else if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_TDMB)
			{
				piArg = (int *)ComponentParameterStructure;
				unsigned int * uiARG = (unsigned int *)piArg[0];
				int ContentsType, status = 0;
				err = TCC351XTDMB_FrequencySet(uiARG[2], piArg[1]);
				if(err == 0)
				{
					unsigned char ucService[7];
					ucService[0] = uiARG[4];
					ucService[1] = uiARG[5];
					ucService[2] = uiARG[6];
					ucService[3] = uiARG[7];
					ucService[4] = uiARG[8];
					ucService[5] = uiARG[9];
					ucService[6] = uiARG[10];

					err = TCC351XTDMB_ServiceSelect(uiARG[11], ucService);
					if(err != 0)
					{
						omxErr = OMX_ErrorBadParameter;
					}
					omx_tcc351x_tuner_component_set_channel_notify(err == 0? 1:0);
				}
				else
				{
					omxErr = OMX_ErrorBadParameter;
				}
			}	
			else
				omxErr = OMX_ErrorBadParameter;		
			break;
        case OMX_IndexVendorParamTunerGetChannelIndexByFrequency:
            break;
		case OMX_IndexVendorParamTunerFrequencySet:
			if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_DVBT)
			{				
				piArg = (int *)ComponentParameterStructure;
				err = TCC351XDVBT_FrequencySet(piArg[0], piArg[1], 1); //piArg[0] : Freq, piArg[1]: BW, piArg[2]: LockOn
				if(err != 0)
				{
					DEBUG(DEB_LEV_ERR, "Fail to Change Frequency %dkhz, %dkhz\n", piArg[0], piArg[1]);
					omxErr = OMX_ErrorBadParameter;
				}
			}
			else if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_ISDBT)
			{
				piArg = (int *)ComponentParameterStructure;
				err = TCC351XISDBT_FrequencySet(piArg[0], piArg[1], 1); //piArg[0] : Freq, piArg[1]: BW, piArg[2]: LockOn
				if(err != 0)
				{
					DEBUG(DEB_LEV_ERR, "Fail to Change Frequency %dkhz, %dkhz\n", piArg[0], piArg[1]);
					omxErr = OMX_ErrorBadParameter;
				}

			}
			else if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_TDMB)
			{
				piArg = (int *)ComponentParameterStructure;
				err = TCC351XTDMB_FrequencySet(piArg[0], 1);
				if(err != 0)
				{
					omxErr = OMX_ErrorBadParameter;
				}
			}	
			else
				omxErr = OMX_ErrorBadParameter;
			break;	

		case OMX_IndexVendorParamTunerCountryCodeSet:
			break;

		case OMX_IndexVendorParamTunerOpen:			
			piArg = (int *)ComponentParameterStructure;
			omx_tcc351x_tuner_component_Private->countrycode = piArg[0];
			omx_tcc351x_tuner_component_Private->dxb_standard = piArg[1];
            DXBBOARD_SelRfPath(omx_tcc351x_tuner_component_Private->dxb_standard);
			if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_DVBT)
			{
				err = TCC351XDVBT_Open(omx_tcc351x_tuner_component_Private->countrycode, giCmdInterface, giDataInterface, giNumberOfBaseBand);
				if(err != 0)
				{
					DEBUG(DEB_LEV_ERR, "Fail to Open tuner[%d]\n", omx_tcc351x_tuner_component_Private->countrycode);
					omxErr = OMX_ErrorBadParameter;
				}

			}
			else if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_ISDBT)
			{
				err = TCC351XISDBT_Open(omx_tcc351x_tuner_component_Private->countrycode, giCmdInterface, giDataInterface);
				if(err != 0)
				{
					DEBUG(DEB_LEV_ERR, "Fail to Open tuner[%d]\n", omx_tcc351x_tuner_component_Private->countrycode);
					omxErr = OMX_ErrorBadParameter;
				}
			}
			else if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_TDMB)
			{
				err = TCC351XTDMB_Open(omx_tcc351x_tuner_component_Private->countrycode, giCmdInterface, giDataInterface);
				if(err != 0)
				{					
					omxErr = OMX_ErrorBadParameter;
				}
				else
				{
					//if(piArg[2])
					{		
						//run thread
						if(giDataInterface == TC3X_STREAM_IO_STS)
							giTSIFHandle = TCC_TSIF_Open(1, TSIF_MAX_PACKETCOUNT, 1, 1);
						else
							giTSIFHandle = TCC_TSIF_Open(1, TSIF_MAX_PACKETCOUNT, 3, 0);
						if( giTSIFHandle < 0 )
						{
							DEBUG(DEB_LEV_ERR,"error TCCTSIF_Open\n");
							omxErr = OMX_ErrorBadParameter;
						}
						else
						{						
							err = TCCTHREAD_Create(&TCC351XThreadID, tcc351x_tdmb_thread, "tcc351x_tdmb_thread", LOW_PRIORITY_10, NULL);
							if(err)
							{
								DEBUG(DEB_LEV_ERR,"In %s CAN NOT make tcc351x_tdmb_thread !!!!\n", __func__);
								omxErr = OMX_ErrorBadParameter;
							}
						}	
					}	
				}
			}	
			else
				omxErr = OMX_ErrorBadParameter;
			break;

		case OMX_IndexVendorParamTunerClose:
			if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_DVBT)
			{
				TCC351XDVBT_Close();
			}
			else if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_ISDBT)
			{
				TCC351XISDBT_Close();
			}
			else if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_TDMB)
			{
				if(giTSIFHandle>=0)
				{
					TCC_TSIF_Close(giTSIFHandle);
					giTSIFHandle = -1;
					TCCTHREAD_Join(TCC351XThreadID,NULL);
				}
				TCC351XTDMB_Close();
			}
			break;

		case OMX_IndexVendorParamSetResync:				
		    break;

        case OMX_IndexVendorParamRegisterPID:
            {
               	int pid = (int)ComponentParameterStructure;
               	ALOGD("Register PID : [0x%X]\n", pid);
                //TC3X_API_REG_PID (0, pid);    
            }
			break;	

        case OMX_IndexVendorParamUnRegisterPID:		    
            {
               	int pid = (int)ComponentParameterStructure;
               	ALOGD("UnRegister PID : [0x%X]\n", pid);
                //TC3X_API_UnREG_PID (0, pid);    
            }
			break;	
			
        case OMX_IndexVendorParamTunerSetNumberOfBB:		    
            {
               	int number_of_bb = (int)ComponentParameterStructure;
               	giNumberOfBaseBand = number_of_bb; 
               	ALOGD("Setting number of baseband : [%d]\n", number_of_bb);     
            }
			break;				

		case OMX_IndexVendorConfigSetAntennaPower:
			{
				int arg = (int)ComponentParameterStructure;
				//TCC351XDVBT_SetAntennaPower(arg);
			}
			break;

		case OMX_IndexVendorParamTunerPlayStop:
			moduleidx = (int)ComponentParameterStructure;
			DEBUG(DEB_LEV_ERR, "[## OMX SetParameter ##] OMX_IndexVendorParamTunerPlayStop:%d\n", moduleidx);
			break;

		default: /*Call the base component function*/
			return omx_base_component_SetParameter(hComponent, nParamIndex, ComponentParameterStructure);
	}

	return omxErr;

}

OMX_ERRORTYPE omx_tcc351x_tuner_component_GetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_INOUT OMX_PTR ComponentParameterStructure)
{
	OMX_OTHER_PARAM_PORTFORMATTYPE *pOtherPortFormat;  
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE*)hComponent;
	omx_tcc351x_tuner_component_PrivateType* omx_tcc351x_tuner_component_Private = openmaxStandComp->pComponentPrivate;
	omx_base_audio_PortType *pPort = (omx_base_audio_PortType *) omx_tcc351x_tuner_component_Private->ports[OMX_BASE_SOURCE_OUTPUTPORT_INDEX];  
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
				TUNER351x_SEARCH_INFO *pSearchInfo;
				pSearchInfo =(TUNER351x_SEARCH_INFO *)ComponentParameterStructure;
				pSearchInfo->uiCountryCode = omx_tcc351x_tuner_component_Private->countrycode;				
				if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_DVBT)
				{
				}
				else if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_ISDBT)
				{
					pSearchInfo->uiMinChannel = TCC351XISDBT_GetMinChannel();
					pSearchInfo->uiMaxChannel =TCC351XISDBT_GetMaxChannel();
				}
				else if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_TDMB)
				{
					pSearchInfo->uiMinChannel = TCC351XTDMB_GetMinChannel();
					pSearchInfo->uiMaxChannel =TCC351XTDMB_GetMaxChannel();
					
					TCC351XTDMB_SetState(STATE_IDLE);	
				}	
			}	
			break;

		case OMX_IndexVendorParamTunerChannelSearchStart:				
			if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_TDMB)
			{
				err = pthread_create(&omx_tcc351x_tuner_component_Private->ChannelSearchThread,
					NULL,
					omx_tdmbtuner_channel_search_thread,
					openmaxStandComp);
							
				if(err) {
					DEBUG(DEB_LEV_ERR, "[TCC351XTUNER]Starting channel search thread failed\n");
					return OMX_ErrorUndefined;
				}
			}
			break;

		case OMX_IndexVendorParamTunerChannelSearchStop:
			if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_TDMB)
			{
				//Only TDMB is valid
				TCC351XTDMB_SearchStopChannels();				
				if(omx_tcc351x_tuner_component_Private->ChannelSearchThread)
				{
					pthread_join(omx_tcc351x_tuner_component_Private->ChannelSearchThread,NULL);
					omx_tcc351x_tuner_component_Private->ChannelSearchThread = NULL;
				}
			}
			break;

		case OMX_IndexVendorParamSetGetPacketDataEntry:
			if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_TDMB)
			{
				//Only TDMB is valid			
				*(int *)ComponentParameterStructure = (int)TCC351XTDMB_ReadFIFO;				
			}
			break;

		case OMX_IndexVendorConfigGetSignalStrength:
			{
				if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_TDMB)
				{
					int *piStrength = (int *)ComponentParameterStructure;
					memset(piStrength, 0x00, sizeof(int)*14);
					unsigned int SNR, PCBER;
					double RSSI;

					piStrength[0] = (int)TCC351XTDMB_GetSignalStrength(&SNR, &PCBER, &RSSI);
					piStrength[1] = (int)SNR;
					piStrength[2] = (int)PCBER;
					piStrength[3] = (int)RSSI;
				}
				else if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_DVBT)
				{
					unsigned int *puiStr = (unsigned int *)ComponentParameterStructure;
					char *msg = puiStr[4];

					puiStr[0] = TCC351XDVBT_GetSignalStrength();  // signal strength (0 ~ 100)
					puiStr[1] = TCC351XDVBT_GetSignalQuality();   // signal quality (0 ~ 100)
					puiStr[2] = 0;//TCC351XDVBT_GetSNR();         // snr (0 ~ 100)
					puiStr[3] = 0;//TCC351XDVBT_GetBER();         // ber (0 ~ 100)

					//if (msg) sprintf(msg, "STR: %d, QUA: %d, SNR: %d, BER: %d\n", puiStr[0], puiStr[1], puiStr[2], puiStr[3]);
				}
				else if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_ISDBT)
				{
					unsigned int *puiStr = (unsigned int *)ComponentParameterStructure;
					int	antena_bar;

					antena_bar = TCC351XISDBT_GetSignalStrength();
					puiStr[0] = sizeof(int)*2;
					puiStr[1] = antena_bar*20;
					puiStr[2] = antena_bar*20;
				}
			}	
			break;	

		case OMX_IndexVendorConfigGetSignalStrengthIndex:
			{
				if(omx_tcc351x_tuner_component_Private->dxb_standard == TCC351X_ISDBT) {
					int *puiStr = (int *)ComponentParameterStructure;
					int antena_bar;

					antena_bar = TCC351XISDBT_GetSignalStrength();
					*puiStr = antena_bar;
				}
			}
			break;
		case OMX_IndexVendorParamTunerGetEWSFlag:
			{
				int *puiStr = (unsigned int *)ComponentParameterStructure;
				*puiStr = 0;
			}
			break;
		case OMX_IndexVendorParamTunerGetChannelValidity:
			{
				int channel = (int)ComponentParameterStructure;
				//none
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

OMX_ERRORTYPE omx_tcc351x_tuner_component_MessageHandler(OMX_COMPONENTTYPE* openmaxStandComp, internalRequestMessageType *message)
{
	omx_tcc351x_tuner_component_PrivateType* omx_tcc351x_tuner_component_Private = (omx_tcc351x_tuner_component_PrivateType*)openmaxStandComp->pComponentPrivate;  
	OMX_ERRORTYPE err;
	OMX_STATETYPE eCurrentState = omx_tcc351x_tuner_component_Private->state;
	DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s\n", __func__);
	
	return err;  
}

OMX_ERRORTYPE omx_tcc351x_CSPI_STS_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp) 
{
	return (omx_tcc351x_tuner_component_Constructor(openmaxStandComp, BROADCAST_TCC351X_CSPI_STS_NAME));
}

OMX_ERRORTYPE omx_tcc351x_CSPI_SPIMS_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp) 
{
	return (omx_tcc351x_tuner_component_Constructor(openmaxStandComp, BROADCAST_TCC351X_CSPI_SPIMS_NAME));
}

OMX_ERRORTYPE omx_tcc351x_I2C_STS_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp) 
{
	return (omx_tcc351x_tuner_component_Constructor(openmaxStandComp, BROADCAST_TCC351X_I2C_STS_NAME));
}

OMX_ERRORTYPE omx_tcc351x_I2C_SPIMS_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp) 
{
	return (omx_tcc351x_tuner_component_Constructor(openmaxStandComp, BROADCAST_TCC351X_I2C_SPIMS_NAME));
}

OMX_ERRORTYPE omx_tcc351x_CSPI_TSIFMOD_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp) 
{
	return (omx_tcc351x_tuner_component_Constructor(openmaxStandComp, BROADCAST_TCC351X_CSPI_TSIFMOD_NAME));
}

OMX_ERRORTYPE omx_tcc351x_tuner_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp)
{
	return omx_tcc351x_tuner_component_Destructor(openmaxStandComp);
}

