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

#ifndef	_TCC_DVB_MANAGER_DEMUX_H_
#define	_TCC_DVB_MANAGER_DEMUX_H_
#include "tcc_dxb_interface_demux.h"

#include <tcc_dxb_memory.h>
#include <tcc_dxb_queue.h>
#include <tcc_dxb_semaphore.h>
#include <tcc_dxb_thread.h>

#define PID_PAT				0x0000
#define PID_PMT				0x0002
#define	PID_PMT_1SEG		0x1FC8
#define PID_NIT 			0x0010
#define PID_SDT				0x0011
#define PID_EIT				0x0012
#define PID_TOT				0x0014

//#define ALTI_CAS

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
   PAT_ID,                    /* 0x00   program_association_section                             */
   CAT_ID,                    /* 0x01   conditional access_section                              */
   PMT_ID,                    /* 0x02   program_map_section                                     */

   NIT_A_ID          = 0x40,  /* 0x40  network_information_section - actual_network             */
   NIT_O_ID          = 0x41,  /* 0x41  network_information_section - other_network              */
   SDT_A_ID          = 0x42,  /* 0x42  service_description_section - actual_ts                  */
   SDT_O_ID          = 0x46,  /* 0x46  service_description_section - other_ts                   */
   BAT_ID            = 0x4A,  /* 0x4A  bouquet_association_section                              */
   EIT_PF_A_ID       = 0x4E,  /* 0x4E  event_information_section - actual_ts, present/following */
   EIT_PF_O_ID       = 0x4F,  /* 0x4F  event_information_section - other_ts, present/following  */
   EIT_SA_0_ID       = 0x50,  /* 0x50-0x5F   event_information_section - actual_ts, schedule    */
   EIT_SA_1_ID,
   EIT_SA_2_ID,
   EIT_SA_3_ID,
   EIT_SA_4_ID,
   EIT_SA_5_ID,
   EIT_SA_6_ID,
   EIT_SA_7_ID,
   EIT_SA_8_ID,
   EIT_SA_9_ID,
   EIT_SA_A_ID,
   EIT_SA_B_ID,
   EIT_SA_C_ID,
   EIT_SA_D_ID,
   EIT_SA_E_ID,
   EIT_SA_F_ID,
   EIT_SO_0_ID       = 0x60,  /* 0x60-0x6F   event_information_section - other_ts, schedule     */
   EIT_SO_1_ID,
   EIT_SO_2_ID,
   EIT_SO_3_ID,
   EIT_SO_4_ID,
   EIT_SO_5_ID,
   EIT_SO_6_ID,
   EIT_SO_7_ID,
   EIT_SO_8_ID,
   EIT_SO_9_ID,
   EIT_SO_A_ID,
   EIT_SO_B_ID,
   EIT_SO_C_ID,
   EIT_SO_D_ID,
   EIT_SO_E_ID,
   EIT_SO_F_ID,
   TDT_ID            = 0x70,  /* 0x70  time_date_section                */
   RST_ID            = 0x71,  /* 0x71  running_status_section           */
   ST_ID             = 0x72,  /* 0x72  stuffing_table                   */
   TOT_ID            = 0x73  /* 0x73  time_offset_section              */
}  MPEG_TABLE_IDS;

typedef struct
{
	unsigned int		service_id;
	void *			pmtdata;
}TCC_CA_INFO;

typedef enum
{
	DEMUX_REQUEST_PAT = 1000,
	DEMUX_REQUEST_PMT = 2000,
	DEMUX_REQUEST_SDT = 3000,
	DEMUX_REQUEST_EIT = 4000,
	DEMUX_REQUEST_TTX = 5000,
	DEMUX_REQUEST_SUBT = 6000,
	DEMUX_REQUEST_NIT = 7000
}E_REQUEST_IDS;

typedef enum
{
	DEC_CMDTYPE_DATA,
	DEC_CMDTYPE_CONTROL
}E_DECTHREAD_COMMANDTYPE;

typedef enum
{
	DEC_CTRLCMD_START,
	DEC_CTRLCMD_STOP,
	DEC_CTRLCMD_CLEAR,	
}E_DECTHREAD_CTRLCOMMAND;

typedef enum
{
	DEC_THREAD_RUNNING,
	DEC_THREAD_STOP
}E_DECTHREAD_STATUS;


typedef struct
{	
	E_DECTHREAD_COMMANDTYPE eCommandType;
	E_DECTHREAD_CTRLCOMMAND	eCtrlCmd;
	int 		 iCommandSync;
	void *pArg;
	int iRequestID;
	unsigned int uiDataSize;
	unsigned int uiPTS;
	char *pData;
}DEMUX_DEC_COMMAND_TYPE;

extern void tcc_demux_init_searchlist(void);
extern int tcc_demux_add_searchlist(int iReqID);
extern int tcc_demux_delete_searchlist(int iReqID);
extern int tcc_demux_get_valid_searchlist(void);
extern int tcc_demux_load_pmts(void);
extern int tcc_demux_load_sdt(void);
extern int tcc_demux_parse_epg(void);
extern int tcc_demux_stop_epg(void);
extern void tcc_demux_teletext_decoder(void *arg);
extern void tcc_demux_subtitle_decoder(void *arg);
extern void tcc_demux_section_decoder(void *arg);
extern DxB_ERR_CODE tcc_demux_section_notify(UINT32 ulDemuxId, UINT8 *pucBuf,  UINT32 ulRequestId);
extern DxB_ERR_CODE tcc_demux_sectionEx_notify(UINT32 ulDemuxId, UINT32 SectionHandle, UINT8 *pucBuf,  UINT32 length, UINT32 ulPid, UINT32 ulRequestId);
extern DxB_ERR_CODE tcc_demux_teletext_notify(UINT32 ulDemuxId, UINT8 *pucBuf, UINT32 uiBufSize, UINT64 ullPTS, UINT32 ulRequestId);
extern DxB_ERR_CODE tcc_demux_subtitle_notify(UINT32 ulDemuxId, UINT8 *pucBuf, UINT32 uiBufSize, UINT64 ullPTS, UINT32 ulRequestId);
extern int tcc_demux_send_dec_ctrl_cmd(tcc_dxb_queue_t* pDecoderQueue, E_DECTHREAD_CTRLCOMMAND eESCmd, int iSync, void *pArg);
extern DxB_ERR_CODE tcc_demux_alloc_buffer(UINT32 usBufLen, UINT8 **ppucBuf);
extern DxB_ERR_CODE tcc_demux_free_buffer( UINT8 *pucBuf);
extern void tcc_demux_event(DxB_DEMUX_NOTIFY_EVT nEvent, void  *pEventData);
extern int tcc_manager_demux_init(void);
extern int tcc_manager_demux_deinit(void);
extern int tcc_manager_demux_set_tunerinfo(int iChannel, int iFrequency, int iCountrycode);
extern int tcc_manager_demux_delete_db(void);
extern int tcc_manager_demux_scan(void);
extern int tcc_manager_demux_avplay(int AudioPID, int VideoPID, int PcrPID);
extern int tcc_manager_demux_subtitlestop(void);
extern int tcc_manager_demux_subtitleplay(int iPID, int iCompositionID, int uiAncillaryID);
extern int tcc_manager_demux_avstop(void);
extern int tcc_manager_demux_reset_av(int Audio, int Video);
extern int tcc_manager_demux_rec_start(unsigned char *pucFileName);
extern int tcc_manager_demux_rec_stop(void);
extern int tcc_manager_demux_startdemuxing(void);
extern int tcc_manager_demux_ip_avstop(void);
extern int tcc_manager_demux_setactivemode(int activemode);


#ifdef __cplusplus
}
#endif

#endif


