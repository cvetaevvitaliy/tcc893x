/*
 * Copyright (C) 2013 Telechips, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_NDEBUG 0
#define LOG_TAG	"DVB_MANAGER_DEMUX"
#include <utils/Log.h>

#include <fcntl.h>
#include <pthread.h>

#include "tcc_iptv_event.h"
#include "tcc_iptv_manager_demux.h" 
#include "tcc_dxb_manager_dsc.h"

#define	 _SUPPORT_DVB_SUBTITLE_
#define	 _SUPPORT_DVB_EIT_	
#define  FAST_CHSCAN_ENABLE

//#define _SUPPORT_DVB_SDT_
//#define _SUPPORT_DVB_NIT_

#define	SECTION_QUEUESIZE		1000
#define	TTX_QUEUESIZE			1000
#define	SUBT_QUEUESIZE			1000

static pthread_t SectionDecoderThreadID;
//static  void SectionDecoderThreadID;
static tcc_dxb_queue_t* SectionDecoderQueue;

static pthread_t TTXDecoderThreadID;
//static void TTXDecoderThreadID;
static tcc_dxb_queue_t* TTXDecoderQueue;

static pthread_t SUBTDecoderThreadID;
//static void SUBTDecoderThreadID;
static tcc_dxb_queue_t* SUBTDecoderQueue;


static int giSectionThreadRunning = 0, giTTXThreadRunning = 0, giSUBTThreadRunning = 0;
static int giIsplayingSubtitle = 0;

int gCurrentChannel = -1, gCurrentFrequency = -1, gCurrentCountryCode = -1;

#define	MAXSEARCH_LIST	64
static int giSearchList[MAXSEARCH_LIST];
tcc_dxb_sem_t *gpDemuxScanWait;	

void tcc_demux_init_searchlist(void)
{
	int i;
	for(i=0; i<MAXSEARCH_LIST; i++)
	{
		giSearchList[i] = -1;
	}	
}	

int tcc_demux_add_searchlist(int iReqID)
{
	int i;
	for(i=0; i<MAXSEARCH_LIST; i++)
	{
		if( giSearchList[i] == -1)
		{
			giSearchList[i] = iReqID;
			break;
		}
	}
	if( i==MAXSEARCH_LIST )
		return 1;	
	return 0;
}

int tcc_demux_delete_searchlist(int iReqID)
{
	int i;
	for(i=0; i<MAXSEARCH_LIST; i++)
	{
		if( giSearchList[i] == iReqID )
		{
			giSearchList[i] = -1;
			break;
		}
	}
	if( i == MAXSEARCH_LIST )
		return 1;
	
	return 0;
}

int tcc_demux_get_valid_searchlist(void)
{
	int ivalidnum = 0, i;
	for(i=0; i<MAXSEARCH_LIST; i++)
	{
		if( giSearchList[i] != -1 )
		{
			ivalidnum++;
		}
	}
	return ivalidnum;
}
int tcc_demux_load_pmts(void)
{
	return 0;
}

int tcc_demux_load_sdt(void)
{
	int ret = 0;
	char      ucValue[16];
	char      ucMask[16];
	char      ucExclusion[16];
	int iRequestID = DEMUX_REQUEST_SDT;
	ALOGD("In %s\n", __func__);
	ucValue[0] = 0x42;
	ucMask[0] = 0xff;
	ucExclusion[0] = 0x00;
	tcc_demux_add_searchlist(iRequestID);
	TCC_DxB_DEMUX_StartSectionFilter(0, 0x11, iRequestID, TRUE, 1, ucValue, ucMask, ucExclusion, TRUE);		
	return 0;
}
int tcc_demux_parse_epg(void)
{
#ifdef		_SUPPORT_DVB_EIT_	
	int ret = 0;
	char      ucValue[16];
	char      ucMask[16];
	char      ucExclusion[16];

	ALOGD("In %s\n", __func__);
#if 1
	ucValue[0] = 0x4E; //actual PF
	ucMask[0] = 0xff;
	ucExclusion[0] = 0x00;
	TCC_DxB_DEMUX_StartSectionFilter(0, 0x12, DEMUX_REQUEST_EIT, FALSE, 1, ucValue, ucMask, ucExclusion, FALSE);		
#endif

#if 1
	ucValue[0] = 0x50; //actual schedule
	ucMask[0] = 0xf0;
	ucExclusion[0] = 0x00;
	TCC_DxB_DEMUX_StartSectionFilter(0, 0x12, DEMUX_REQUEST_EIT+1, FALSE, 1, ucValue, ucMask, ucExclusion, FALSE);	
#endif	
#endif	
	return 0;
}	

int tcc_demux_stop_epg(void)
{
#ifdef		_SUPPORT_DVB_EIT_		
	ALOGD("In %s\n", __func__);
	TCC_DxB_DEMUX_StopSectionFilter (0,4000); //Actual PF
	TCC_DxB_DEMUX_StopSectionFilter (0,4001); //Actual Schedule	
#endif	
	return 0;
}

int tcc_demux_parse_nit(void)
{
	int ret = 0;
	unsigned char      ucValue[16];
	unsigned char      ucMask[16];
	unsigned char      ucExclusion[16];
	ALOGD("In %s\n", __func__);
	ucValue[0] = NIT_A_ID; //actual network information
	ucMask[0] = 0xff;
	ucExclusion[0] = 0x00;
	tcc_demux_add_searchlist(DEMUX_REQUEST_NIT);
	TCC_DxB_DEMUX_StartSectionFilter(0, PID_NIT, DEMUX_REQUEST_NIT, FALSE, 1, ucValue, ucMask, ucExclusion, FALSE);	
	return 0;
}	

int tcc_demux_stop_nit(void)
{
	ALOGD("In %s\n", __func__);
	tcc_demux_delete_searchlist(DEMUX_REQUEST_NIT);
	TCC_DxB_DEMUX_StopSectionFilter (0,DEMUX_REQUEST_NIT); //Actual network information
	return 0;
}

int tcc_demux_parse_sdt(void)
{
	int ret = 0;
	unsigned char      ucValue[16];
	unsigned char      ucMask[16];
	unsigned char      ucExclusion[16];
	ALOGD("In %s\n", __func__);
	ucValue[0] = SDT_A_ID; 
	ucMask[0] = 0xff;
	ucExclusion[0] = 0x00;
	tcc_demux_add_searchlist(DEMUX_REQUEST_SDT);
	TCC_DxB_DEMUX_StartSectionFilter(0, PID_SDT, DEMUX_REQUEST_SDT, FALSE, 1, ucValue, ucMask, ucExclusion, FALSE);	
	return 0;
}	

int tcc_demux_stop_sdt(void)
{
	ALOGD("In %s\n", __func__);
	tcc_demux_delete_searchlist(DEMUX_REQUEST_SDT);
	TCC_DxB_DEMUX_StopSectionFilter (0,DEMUX_REQUEST_SDT); 
	return 0;
}

void tcc_demux_teletext_decoder(void *arg)
{
	DEMUX_DEC_COMMAND_TYPE *pSectionCmd;

	while(giTTXThreadRunning)
	{
		if(tcc_dxb_getquenelem(TTXDecoderQueue))
		{
			pSectionCmd = tcc_dxb_dequeue(TTXDecoderQueue);
			if (pSectionCmd)
			{				
				//Parsing SI Infromation
				tcc_dxb_free(pSectionCmd->pData);
				tcc_dxb_free(pSectionCmd);
			}
		}	

		usleep(5000);			
	}
	giTTXThreadRunning = -1;
}

void tcc_demux_subtitle_decoder(void *arg)
{
	DEMUX_DEC_COMMAND_TYPE *pSectionCmd;

	while(giSUBTThreadRunning)
	{
		if(tcc_dxb_getquenelem(SUBTDecoderQueue))
		{
			pSectionCmd = tcc_dxb_dequeue(SUBTDecoderQueue);
			if (pSectionCmd)
			{				
				//Parsing SI Infromation
				tcc_dxb_free(pSectionCmd->pData);
				tcc_dxb_free(pSectionCmd);
			}
		}	

		usleep(5000);			
	}
	giSUBTThreadRunning = -1;
}


void tcc_demux_section_decoder(void *arg)
{
	int Result, iPATReuestID = 0;
	void 	*pSectionBitPars = NULL;
	DEMUX_DEC_COMMAND_TYPE *pSectionCmd;
	int sdt_handel, nit_handel;

	while(giSectionThreadRunning)
	{
		if(tcc_dxb_getquenelem(SectionDecoderQueue))
		{
			pSectionCmd = tcc_dxb_dequeue(SectionDecoderQueue);
			if (pSectionCmd)
			{				
				switch(pSectionCmd->pData[0])
				{
					case SDT_A_ID:
						ALOGE("FInd SDT %x \n", pSectionCmd->pData[0]);
						{
			//				sdt_handel = fopen("/mnt/sdcard/tflash/sdt_data.data", "wb+");
			//				fwrite(pSectionCmd->pData, 1, pSectionCmd->uiDataSize, sdt_handel);
			//				fclose(sdt_handel);

						}
						break;
					case NIT_A_ID:
						ALOGE("FInd NIT %d \n", pSectionCmd->pData[0]);
						{
							TCCDxBEvent_SendSIData(pSectionCmd->pData, NIT_A_ID); // only sample code 
			//				nit_handel = fopen("/mnt/sdcard/tflash/nit_data.data", "wb+");
			//				fwrite(pSectionCmd->pData, 1, pSectionCmd->uiDataSize, nit_handel);
			//				fclose(nit_handel);

						}
						break;
					default:
						if( (Result >= EIT_SA_0_ID) && (Result <= EIT_SA_F_ID) )
						{
							//DEBUG(DEFAULT_MESSAGES, "[TCCDEMUX]EPG Actual Schedule(0x%X)\n", Result);
						}
						else if(Result == EIT_PF_A_ID)
						{
							//DEBUG(DEFAULT_MESSAGES, "[TCCDEMUX]EPG Actual P/F(0x%X)\n", Result);
						}
						break;
				}				
				tcc_dxb_free(pSectionCmd->pData);
				tcc_dxb_free(pSectionCmd);
				usleep(5000);			
			}
			else
		usleep(5000);			
	}
		else
		{
			usleep(5000);			
		}
	}

	giSectionThreadRunning = -1;
}

DxB_ERR_CODE tcc_demux_section_notify(UINT32 ulDemuxId, UINT8 *pucBuf,  UINT32 ulRequestId)
{
	DEMUX_DEC_COMMAND_TYPE *pSectionCmd;
	//ALOGD("In %s\n", __func__);
	pSectionCmd = tcc_dxb_malloc(sizeof(DEMUX_DEC_COMMAND_TYPE));
	pSectionCmd->pData = pucBuf;	
	pSectionCmd->iRequestID = ulRequestId;

	if( tcc_dxb_queue_ex(SectionDecoderQueue, pSectionCmd) == 0 )
	{
		tcc_dxb_free(pSectionCmd->pData);
		tcc_dxb_free(pSectionCmd);
		//ALOGE("In %s : ERROR CAN NOT PUSH QUEUE\n", __func__);
	}
	return DxB_ERR_OK;
}

DxB_ERR_CODE tcc_demux_sectionEx_notify(UINT32 ulDemuxId, UINT32 SectionHandle, UINT8 *pucBuf,  UINT32 length, UINT32 ulPid, UINT32 ulRequestId)
{
	DEMUX_DEC_COMMAND_TYPE *pSectionCmd;
	//ALOGD("In %s\n", __func__);
	pSectionCmd = tcc_dxb_malloc(sizeof(DEMUX_DEC_COMMAND_TYPE));
	pSectionCmd->pData = pucBuf;	
	pSectionCmd->uiDataSize = length;
	pSectionCmd->iRequestID = ulRequestId;

	if( tcc_dxb_queue_ex(SectionDecoderQueue, pSectionCmd) == 0 )
	{
		tcc_dxb_free(pSectionCmd->pData);
		tcc_dxb_free(pSectionCmd);
		ALOGE("In %s : ERROR CAN NOT PUSH QUEUE\n", __func__);
	}
	return DxB_ERR_OK;
}

DxB_ERR_CODE tcc_demux_teletext_notify(UINT32 ulDemuxId, UINT8 *pucBuf, UINT32 uiBufSize, UINT64 ullPTS, UINT32 ulRequestId)
{	
	DEMUX_DEC_COMMAND_TYPE *pTTXCmd;
	//ALOGD("In %s\n", __func__);
	pTTXCmd = tcc_dxb_malloc(sizeof(DEMUX_DEC_COMMAND_TYPE));
	pTTXCmd->pData = pucBuf;	
	pTTXCmd->uiDataSize = uiBufSize;
	pTTXCmd->uiPTS = ullPTS/90;
	pTTXCmd->iRequestID = ulRequestId;
	if( tcc_dxb_queue_ex(TTXDecoderQueue, pTTXCmd) == 0 )
	{
		tcc_dxb_free(pTTXCmd->pData);
		tcc_dxb_free(pTTXCmd);
		ALOGE("In %s : ERROR CAN NOT PUSH QUEUE\n", __func__);
	}
	
	return DxB_ERR_OK;
}

DxB_ERR_CODE tcc_demux_subtitle_notify(UINT32 ulDemuxId, UINT8 *pucBuf, UINT32 uiBufSize, UINT64 ullPTS, UINT32 ulRequestId)
{	
	DEMUX_DEC_COMMAND_TYPE *pSubtCmd;
	//ALOGD("In %s\n", __func__);
	pSubtCmd = tcc_dxb_malloc(sizeof(DEMUX_DEC_COMMAND_TYPE));
	pSubtCmd->eCommandType = DEC_CMDTYPE_DATA;
	pSubtCmd->pData = pucBuf;	
	pSubtCmd->uiDataSize = uiBufSize;
	pSubtCmd->uiPTS = ullPTS/90;
	pSubtCmd->iRequestID = ulRequestId;
	if( tcc_dxb_queue_ex(SUBTDecoderQueue, pSubtCmd) == 0 )
	{
		tcc_dxb_free(pSubtCmd->pData);
		tcc_dxb_free(pSubtCmd);
		ALOGE("In %s : ERROR CAN NOT PUSH QUEUE\n", __func__);
	}	
	return DxB_ERR_OK;
}


int tcc_demux_send_dec_ctrl_cmd(tcc_dxb_queue_t* pDecoderQueue, E_DECTHREAD_CTRLCOMMAND eESCmd, int iSync, void *pArg)
{	int ret = 0;
	DEMUX_DEC_COMMAND_TYPE *pDecCmd;
	tcc_dxb_sem_t *pCmdSync;	
	int err;

	pDecCmd = tcc_dxb_malloc(sizeof(DEMUX_DEC_COMMAND_TYPE));
	if( pDecCmd )
	{
		pDecCmd->eCommandType = DEC_CMDTYPE_CONTROL;
		pDecCmd->eCtrlCmd = eESCmd;
		pDecCmd->iCommandSync = iSync;
		pDecCmd->pArg = pArg;
		if( pDecCmd->iCommandSync )
		{			
			pCmdSync = (tcc_dxb_sem_t *)tcc_dxb_calloc(1,sizeof(tcc_dxb_sem_t));
			tcc_dxb_sem_init(pCmdSync, 0);
			pDecCmd->pData = (unsigned char *)pCmdSync;
			
		}
		//if( queuefirst(pDecoderQueue, pDecCmd) == 0 )		
		if( tcc_dxb_queue_ex(pDecoderQueue, pDecCmd) == 0 )		
		{
			ALOGE("%s CMD FAIL !!! (%d)(%d)\n", __func__,eESCmd,iSync);
			ret = 1;
		}	
		else
		{
			ALOGD("%s CMD SUCCESS !!! (%d)(%d)\n", __func__,eESCmd,iSync);
			if( iSync )
			{
				if( tcc_dxb_sem_down_timewait(pCmdSync, 5) == 0 )
				{
					ALOGE("%s CMD TIMEOUT !!! (%d)(%d)\n", __func__,eESCmd,iSync);
					ret = 1;
				}	
			}		
		}
		
		if(ret || iSync)
		{
			if(iSync)
			{
				tcc_dxb_sem_deinit(pCmdSync);
				tcc_dxb_free(pCmdSync);
			}
			tcc_dxb_free(pDecCmd);
		}		
		
	}
	else
		ret = 1;

	return ret;
}

DxB_ERR_CODE tcc_demux_alloc_buffer(UINT32 usBufLen, UINT8 **ppucBuf)
{
	//ALOGD("In %s\n", __func__);
	*ppucBuf = tcc_dxb_malloc(usBufLen);
	return DxB_ERR_OK;
}

DxB_ERR_CODE tcc_demux_free_buffer( UINT8 *pucBuf)
{
	ALOGD("In %s\n", __func__);
	tcc_dxb_free(pucBuf);
	return DxB_ERR_OK;
}

void tcc_demux_event(DxB_DEMUX_NOTIFY_EVT nEvent, void  *pEventData)
{
    ALOGD("%s:[%d][%p]", __func__, nEvent, pEventData); 
    switch(nEvent)
    {
        case DxB_DEMUX_EVENT_CAS_CHANNELCHANGE:
		{
			int err; 
			TCC_CA_INFO 			*CA_PMT_info;
			CA_PMT_info = (TCC_CA_INFO *)pEventData;
#ifdef ALTI_CAS
			err = CA_channel_change(CA_PMT_info->service_id, CA_PMT_info->pmtdata);
			if(!err)
				ALOGE("%s %d CA_channel_change error\n", __func__, __LINE__);
#endif
        	}
            break;
        default:
            break;
    }    
}


int tcc_manager_demux_init(void)
{	
	int status;
	ALOGD("In %s\n", __func__);
	TCC_DxB_DEMUX_Init(DxB_STANDARD_IPTV, 0);
	TCC_DxB_DEMUX_RegisterEventCallback(0, tcc_demux_event);


#ifdef ALTI_CAS
	ALOGE("%s  call CA_init_cas", __func__); 
	CA_init_cas();	
	TSDEMUXDSCApi_ModeSet(DSC_MODE_HW);
	TCC_DxB_DEMUX_RegisterCasDecryptCallback(0, TSDEMUXDSCApi_Decrypt);	
#endif
	
	giSectionThreadRunning = 1;
	giTTXThreadRunning = 1;
	giSUBTThreadRunning = 1;
	
	SectionDecoderQueue = calloc(1,sizeof(tcc_dxb_queue_t));
	tcc_dxb_queue_init_ex(SectionDecoderQueue, SECTION_QUEUESIZE);	

	status = tcc_dxb_thread_create(&SectionDecoderThreadID, tcc_demux_section_decoder, "tcc_demux_section_decoder", LOW_PRIORITY_10, NULL);
	if(status)
	{
		ALOGE("In %s CAN NOT make Section decoder !!!!\n", __func__);
	}	
	TCC_DxB_DEMUX_RegisterSectionCallback(0, tcc_demux_section_notify, tcc_demux_alloc_buffer);
#if 0
	TTXDecoderQueue = calloc(1,sizeof(tcc_dxb_queue_t));
	tcc_dxb_queue_init_ex(TTXDecoderQueue, TTX_QUEUESIZE);	
	status = tcc_dxb_thread_create(&TTXDecoderThreadID, tcc_demux_teletext_decoder, "tcc_demux_teletext_decoder", LOW_PRIORITY_10, NULL);	
	if(status)
	{
		ALOGE("In %s CAN NOT make Section decoder !!!!\n", __func__);
	}	

	SUBTDecoderQueue = calloc(1,sizeof(tcc_dxb_queue_t));
	tcc_dxb_queue_init_ex(SUBTDecoderQueue, SUBT_QUEUESIZE);	
	status = tcc_dxb_thread_create(&SUBTDecoderThreadID, tcc_demux_subtitle_decoder, "tcc_demux_subtitle_decoder", LOW_PRIORITY_10, NULL);	
	if(status)
	{
		ALOGE("In %s CAN NOT make Section decoder !!!!\n", __func__);
	}	
	
	TCC_DxB_DEMUX_RegisterPESCallback(0, DxB_DEMUX_PESTYPE_TELETEXT_ES, tcc_demux_teletext_notify, tcc_demux_alloc_buffer, tcc_demux_free_buffer);
	TCC_DxB_DEMUX_RegisterPESCallback(0, DxB_DEMUX_PESTYPE_SUBTITLE_ES, tcc_demux_subtitle_notify, tcc_demux_alloc_buffer, tcc_demux_free_buffer);	
#endif

	gpDemuxScanWait = (tcc_dxb_sem_t *)tcc_dxb_calloc(1,sizeof(tcc_dxb_sem_t));
	giIsplayingSubtitle = 0;
	return 0;
}


int tcc_manager_demux_deinit(void)
{
	int err;
	giSectionThreadRunning = 0;
	giTTXThreadRunning = 0;
	giSUBTThreadRunning = 0;

	while(1)
	{
		if( giSectionThreadRunning == -1)
			break;
		usleep(5000);
	}
	err = tcc_dxb_thread_join(SectionDecoderThreadID,NULL);	
	tcc_dxb_queue_deinit(SectionDecoderQueue);
	tcc_dxb_free(SectionDecoderQueue);

#if 0
	while(1)
	{
		if( giTTXThreadRunning == -1)
			break;
		usleep(5000);
	}
	
	while(1)
	{
		if( giSUBTThreadRunning == -1)
			break;
		usleep(5000);
	}


	err = tcc_dxb_thread_join(TTXDecoderThreadID,NULL);	
	tcc_dxb_queue_deinit(TTXDecoderQueue);
	tcc_dxb_free(TTXDecoderQueue);

	err = tcc_dxb_thread_join(SUBTDecoderThreadID,NULL);	
	tcc_dxb_queue_deinit(SUBTDecoderQueue);
	tcc_dxb_free(SUBTDecoderQueue);
#endif

	tcc_dxb_free(gpDemuxScanWait);
	
	TCC_DxB_DEMUX_DeInit();
	ALOGD("In %s %d \n", __func__, __LINE__);

	return 0;
}

int tcc_manager_demux_set_tunerinfo(int iChannel, int iFrequency, int iCountrycode)
{
	ALOGD("In %s ch(%d)freq(%d)contry(%d)\n", __func__, iChannel, iFrequency, iCountrycode);
	gCurrentChannel = iChannel;
	gCurrentFrequency = iFrequency;
	gCurrentCountryCode = iCountrycode;	
	return 0;
}

int tcc_manager_demux_delete_db(void)
{
	return 0;
}

int tcc_manager_demux_scan(void)
{	
	return 0;
}

int tcc_manager_demux_avplay(int AudioPID, int VideoPID, int PcrPID)
{
	/*
	#define PID_BITMASK_PCR  0x00000001
	#define PID_BITMASK_VIDEO_MAIN 0x00000002
	#define PID_BITMASK_AUDIO_MAIN 0x00000004
	#define PID_BITMASK_AUDIO_SUB 0x00000008
	#define PID_BITMASK_AUDIO_SPDIF 0x00000010

	*/
	DxB_Pid_Info AVPidInfo;
	ALOGD("In %s\n", __func__);
	AVPidInfo.usPCRPid = PcrPID;
	AVPidInfo.usVideoMainPid = VideoPID;
	AVPidInfo.usAudioMainPid = AudioPID;
	AVPidInfo.pidBitmask = (PID_BITMASK_PCR | PID_BITMASK_VIDEO_MAIN | PID_BITMASK_AUDIO_MAIN);	
	//AVPidInfo.pidBitmask = (PID_BITMASK_VIDEO_MAIN | PID_BITMASK_AUDIO_MAIN);	
	TCC_DxB_DEMUX_StartAVPID(0, &AVPidInfo);

#ifdef 	_SUPPORT_DVB_NIT_
	tcc_demux_parse_nit();
#endif
#ifdef 	_SUPPORT_DVB_SDT_
	tcc_demux_parse_sdt();
#endif	

	return 0;
}
int tcc_manager_demux_subtitlestop(void)
{
#ifdef _SUPPORT_DVB_SUBTITLE_		
	if(giIsplayingSubtitle == 1)
	{
		//TCCDVBSUBT_DeInit();
		tcc_demux_send_dec_ctrl_cmd(SUBTDecoderQueue,DEC_CTRLCMD_STOP, 1, NULL);
		TCC_DxB_DEMUX_StopPESFilter(0, DEMUX_REQUEST_SUBT); //STOP for subtitle
	}	
	giIsplayingSubtitle = 0;
#endif	
	return 0;
}

int tcc_manager_demux_subtitleplay(int iPID, int iCompositionID, int uiAncillaryID)
{
#ifdef _SUPPORT_DVB_SUBTITLE_	
	int iArg[3];
	ALOGD("In %s\n", __func__);
	if(giIsplayingSubtitle == 1)
	{	
		tcc_manager_demux_subtitlestop();
	}
	//TCCDVBSUBT_Init(iCompositionID, uiAncillaryID);	
	iArg[0] = iPID;
	iArg[1] = iCompositionID;
	iArg[2] = uiAncillaryID;
	tcc_demux_send_dec_ctrl_cmd(SUBTDecoderQueue, DEC_CTRLCMD_START, 1, (void *)iArg);
	TCC_DxB_DEMUX_StartPESFilter(0, iPID, DxB_DEMUX_PESTYPE_SUBTITLE_ES, DEMUX_REQUEST_SUBT);
	giIsplayingSubtitle = 1;
#endif
	return 0;
}

int tcc_manager_demux_avstop(void)
{
	TCC_DxB_DEMUX_StopAVPID(0,0xFFFFFFFF);
	return 0;
}

int tcc_manager_demux_reset_av(int Audio, int Video)
{
	if(Audio)
		TCC_DxB_DEMUX_ResetAudio(0);
	
	if(Video)
		TCC_DxB_DEMUX_ResetVideo(0);		
	return 0;
}

int tcc_manager_demux_rec_start(unsigned char *pucFileName)
{
	TCC_DxB_DEMUX_RecordStart (0, pucFileName);
	return 0;
}


int tcc_manager_demux_rec_stop(void)
{
	TCC_DxB_DEMUX_RecordStop(0);
	return 0;
}

int tcc_manager_demux_startdemuxing(void)
{	
	if( TCC_DxB_DEMUX_StartDemuxing (0) != DxB_ERR_OK)
		return 1;
#ifdef 	_SUPPORT_DVB_NIT_
	tcc_demux_parse_nit();
#endif
#ifdef 	_SUPPORT_DVB_SDT_
	tcc_demux_parse_sdt();
#endif	
	return 0;
}

int tcc_manager_demux_ip_avstop(void)
{
#ifdef 	_SUPPORT_DVB_NIT_
	tcc_demux_stop_nit();
#endif
#ifdef 	_SUPPORT_DVB_SDT_
	tcc_demux_stop_sdt();
#endif
		
	TCC_DxB_DEMUX_StopAVPID(0,0xFFFFFFFF);
	return 0;
}

int tcc_manager_demux_setactivemode(int activemode)
{	
	if( TCC_DxB_DEMUX_SetActiveMode(0, activemode) != DxB_ERR_OK)
		return 1;
	return 0;
}

