/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions 
andlimitations under the License.

****************************************************************************/

/****************************************************************************

Revision History

****************************************************************************

****************************************************************************/

#ifndef _TCC_DXB_INTERFACE_DEMUX_H_
#define _TCC_DXB_INTERFACE_DEMUX_H_
#include "tcc_dxb_interface_type.h"

#define	PID_BITMASK_GEN(X) (0x00000001 << X)
#define PID_BITMASK_PCR  PID_BITMASK_GEN(0)
#define PID_BITMASK_VIDEO_MAIN PID_BITMASK_GEN(1)
#define PID_BITMASK_VIDEO_SUB PID_BITMASK_GEN(2)
#define PID_BITMASK_AUDIO_MAIN PID_BITMASK_GEN(3)
#define PID_BITMASK_AUDIO_SUB PID_BITMASK_GEN(4)
#define PID_BITMASK_AUDIO_SPDIF PID_BITMASK_GEN(5)
#define	PID_BITMASK_PCR_SUB	PID_BITMASK_GEN(6)

typedef	enum DxB_ELEMENTARYSTREAM
{
    STREAMTYPE_NONE=0x00, 
    STREAMTYPE_MPEG1_VIDEO=0x01, 
    STREAMTYPE_VIDEO=0x02, 
    STREAMTYPE_AUDIO1=0x03, // MPEG-1 Audio
    STREAMTYPE_AUDIO2=0x04, // MPEG-2 Audio
    STREAMTYPE_PRIVATE=0x06, 
    STREAMTYPE_AUDIO_AAC=0x0f, 
    STREAMTYPE_AUDIO_AAC_LATM = 0x11,				 /*0x11 ISO/IEC 14496-3(MPEG-4 AAC)*/
    STREAMTYPE_AUDIO_AAC_PLUS = 0x12,
    STREAMTYPE_VIDEO_AVCH264=0x1b, 
    STREAMTYPE_AUDIO_BSAC = 0x22,					 /*0x22 ISO/IEC 14496-3 ER BASC Audio Object*/
    STREAMTYPE_AUDIO_AC3_1=0x80, 
    STREAMTYPE_AUDIO_AC3_2=0x81, // AC3
}DxB_ELEMENTARYSTREAM;

typedef enum DxB_DEMUX_PESTYPE
{
	DxB_DEMUX_PESTYPE_SUBTITLE = 0,	
	DxB_DEMUX_PESTYPE_TELETEXT,	
	DxB_DEMUX_PESTYPE_USERDEFINE,
	DxB_DEMUX_PESTYPE_SUBTITLE_ES, //It exclude PES Header, Only send playload data
	DxB_DEMUX_PESTYPE_SUBTITLE_1SEG_ES, //It exclude PES Header, Only send playload data
	DxB_DEMUX_PESTYPE_TELETEXT_ES, //It exclude PES Header, Only send playload data
	DxB_DEMUX_PESTYPE_TELETEXT_1SEG_ES, //It exclude PES Header, Only send playload data
	DxB_DEMUX_PESTYPE_USERDEFINE_ES, //It exclude PES Header, Only send playload data
	DxB_DEMUX_PESTYPE_MAX
} DxB_DEMUX_PESTYPE;

typedef enum DxB_DEMUX_TSSTATE
{
	DxB_DMX_TS_Scramble,
	DxB_DMX_TS_Clean,
	DxB_DMX_TS_UnKnown
}DxB_DEMUX_TSSTATE;

typedef struct DxB_PID_INFO
{
	UINT32 pidBitmask;
	UINT16 usPCRPid;
	UINT16 usPCRSubPid;
	UINT16 usVideoMainPid;
	UINT16 usVideoSubPid;
	UINT16 usVideoMainType;
	UINT16 usVideoSubType;
	UINT16 usAudioMainPid;
	UINT16 usAudioSubPid;
	UINT16 usAudioMainType;
	UINT16 usAudioSubType;
	UINT16 usAudioSpdifPid;
} DxB_Pid_Info;

typedef enum
{
	DxB_DEMUX_NOTIFY_FIRSTFRAME_DISPLAYED,
	DxB_DEMUX_EVENT_RECORDING_STOP,
	DxB_DEMUX_EVENT_CAS_CHANNELCHANGE,
	DxB_DEMUX_EVENT_REGISTER_PID,
	DxB_DEMUX_EVENT_UNREGISTER_PID,
	DxB_DEMUX_EVENT_SCRAMBLED_STATUS,
	DxB_DEMUX_EVENT_AUDIO_CODEC_INFORMATION,
	DxB_DEMUX_EVENT_DLS_UPDATE,
	DxB_DEMUX_EVENT_NOTIFY_END
} DxB_DEMUX_NOTIFY_EVT;

/* For A/V sync parameters and returns
 */
enum
{
    DxB_SYNC_AUDIO,
    DxB_SYNC_VIDEO,
};

enum
{
    DxB_SYNC_DROP = -1,
    DxB_SYNC_OK = 0,
    DxB_SYNC_WAIT = 1,
    DxB_SYNC_BYPASS = 2,
    DxB_SYNC_LONG_WAIT = 3,
};
/////////////////////////////////////
//
typedef DxB_ERR_CODE	(*pfnDxB_DEMUX_PES_Notify)(UINT32 ulDemuxId, UINT8 *pucBuf, UINT32 ulBufSize, UINT64 ullPTS, UINT32 ulRequestID);
typedef DxB_ERR_CODE	(*pfnDxB_DEMUX_Notify)(UINT32 ulDemuxId, UINT8 *pucBuf,  UINT32 ulRequestId);
typedef DxB_ERR_CODE	(*pfnDxB_DEMUX_AllocBuffer)(UINT32 usBufLen, UINT8 **ppucBuf);
typedef DxB_ERR_CODE	(*pfnDxB_DEMUX_FreeBufferForError)( UINT8 *ppucBuf);
typedef INT64			(*pfnDxB_DEMUX_GetSTC)(INT32 itype, INT64 lpts, unsigned int index, int log);
typedef int				(*pfnDxB_DEMUX_UpdatePCR)(INT32 itype, int index, INT64 lpts);

typedef DxB_ERR_CODE	(*pfnDxB_DEMUX_NotifyEx)(UINT32 ulDemuxId, UINT32 SectionHandle, UINT8 *pucBuf, UINT32 size, UINT32 ulPid, UINT32 ulRequestId);
typedef void (*pfnDxB_DEMUX_EVENT_Notify)(DxB_DEMUX_NOTIFY_EVT nEvent, void *pCallbackData);

typedef int (*pfnDxb_DEMUX_CasDecrypt)(unsigned int type, unsigned char *pBuf, unsigned int uiSize);


/*CAUTION!!!
* Below pfnDxB_DEMUX_ES_Notify use at only DxB_DEMUX_PESTYPE_SUBTITLE_ES, DxB_DEMUX_PESTYPE_TELETEXT_ES, DxB_DEMUX_PESTYPE_USERDEFINE_ES.
* We make that DxB_DEMUX_PESTYPE(XXXX_ES), in order to reducing parsing load of PES.It strip PES Header and Send only Payload.
* You must be careful using that.
*/
typedef DxB_ERR_CODE	(*pfnDxB_DEMUX_ES_Notify)(UINT32 ulDemuxId, UINT8 *pucBuf, UINT32 uiBufSize, UINT64 ullPTS, UINT32 ulRequestID);
typedef DxB_ERR_CODE	(*pfnDxB_DEMUX_ES_Notify2)(UINT32 ulDemuxId, UINT8 *pucBuf, UINT32 uiBufSize, UINT64 ullPTS, UINT32 ulRequestID, INT32 ulType);


/********************************************************************************************/
/********************************************************************************************
						FOR MW LAYER FUNCTION
********************************************************************************************/
/********************************************************************************************/
DxB_ERR_CODE TCC_DxB_DEMUX_Init(DxB_STANDARDS Standard, UINT32 ulBaseBandType);
DxB_ERR_CODE TCC_DxB_DEMUX_DeInit(void);
DxB_ERR_CODE TCC_DxB_DEMUX_GetNumOfDemux(UINT32 * ulNumOfDemux);
DxB_ERR_CODE TCC_DxB_DEMUX_StartAVPID(UINT32 ulDemuxId, DxB_Pid_Info *ppidInfo);
DxB_ERR_CODE TCC_DxB_DEMUX_StopAVPID(UINT32 ulDemuxId, UINT32 pidBitmask);
DxB_ERR_CODE TCC_DxB_DEMUX_ChangeAVPID(UINT32 ulDemuxId, UINT32 pidBitmask, DxB_Pid_Info *ppidInfo);
DxB_ERR_CODE TCC_DxB_DEMUX_RegisterPESCallback (UINT32 ulDemuxId,
											DxB_DEMUX_PESTYPE etPesType,
											pfnDxB_DEMUX_PES_Notify pfnNotify,
											pfnDxB_DEMUX_AllocBuffer pfnAllocBuffer,
											pfnDxB_DEMUX_FreeBufferForError pfnErrorFreeBuffer);

DxB_ERR_CODE TCC_DxB_DEMUX_StartPESFilter (UINT32 ulDemuxId,UINT16 ulPid, DxB_DEMUX_PESTYPE etPESType, UINT32 ulRequestID);
DxB_ERR_CODE TCC_DxB_DEMUX_StopPESFilter (UINT32 ulDemuxId, UINT32 ulRequestID);
DxB_ERR_CODE TCC_DxB_DEMUX_RegisterSectionCallback (UINT32 ulDemuxId,pfnDxB_DEMUX_Notify	pfnNotify,pfnDxB_DEMUX_AllocBuffer	pfnAllocBuffer);
DxB_ERR_CODE TCC_DxB_DEMUX_StartSectionFilter (UINT32 ulDemuxId,UINT16 usPid,UINT32 ulRequestID, BOOLEAN bIsOnce,UINT32 ulFilterLen,UINT8 * pucValue,UINT8 * pucMask, UINT8 * pucExclusion,BOOLEAN bCheckCRC);
DxB_ERR_CODE TCC_DxB_DEMUX_StopSectionFilter (UINT32 ulDemuxId, UINT32	ulRequestID);
DxB_ERR_CODE TCC_DxB_DEMUX_StartSectionFilterEx(UINT32 ulDemuxId, UINT16 usPid, UINT32 ulRequestID, BOOLEAN bIsOnce,
											 UINT32 ulFilterLen, UINT8 * pucValue, UINT8 * pucMask,
											 UINT8 * pucExclusion, BOOLEAN bCheckCRC,
											 pfnDxB_DEMUX_NotifyEx	pfnNotify, 
											 pfnDxB_DEMUX_AllocBuffer pfnAllocBuffer,
											 UINT32 *SectionHandle);
DxB_ERR_CODE TCC_DxB_DEMUX_StopSectionFilterEx (UINT32 ulDemuxId, UINT32 SectionHandle);

DxB_ERR_CODE TCC_DxB_DEMUX_GetSTC (UINT32 ulDemuxId,UINT32 *pulHighBitSTC, UINT32 *pulLowBitSTC, unsigned int index);

DxB_ERR_CODE TCC_DxB_DEMUX_ResetVideo (UINT32 ulDemuxId);
DxB_ERR_CODE TCC_DxB_DEMUX_ResetAudio (UINT32 ulDemuxId);

DxB_ERR_CODE TCC_DxB_DEMUX_RecordStart (UINT32 ulDemuxId, UINT8 * pucFileName);
DxB_ERR_CODE TCC_DxB_DEMUX_RecordStop (UINT32 ulDemuxId);

DxB_ERR_CODE TCC_DxB_DEMUX_StartTSCMonitor(UINT32 ulDemuxId, UINT16 usPid, UINT32 *pulHandle); //TSC(Transport Scrambling Control bit)
DxB_ERR_CODE TCC_DxB_DEMUX_StopTSCMonitor(UINT32 ulHandle);
DxB_ERR_CODE TCC_DxB_DEMUX_GetTSCStatus(UINT32 ulHandle, DxB_DEMUX_TSSTATE *peTsState);
DxB_ERR_CODE TCC_DxB_DEMUX_StartDemuxing (UINT32 ulDemuxId);
DxB_ERR_CODE TCC_DxB_DEMUX_SetActiveMode (UINT32 ulDemuxId, UINT32 activemode);
DxB_ERR_CODE TCC_DxB_DEMUX_Send_data (UINT32 ulDemuxId, UINT8 *data, UINT32 datasize);
DxB_ERR_CODE TCC_DxB_DEMUX_SetAudioCodecInformation(UINT32 ulDemuxId, void *pAudioInfo);
DxB_ERR_CODE TCC_DxB_DEMUX_TDMB_SetContentsType(UINT32 ulDemuxId, UINT32 ContentsType);
DxB_ERR_CODE TCC_DxB_DEMUX_RegisterEventCallback(UINT32 ulDemuxId, pfnDxB_DEMUX_EVENT_Notify pfnEventCallback);
DxB_ERR_CODE TCC_DxB_DEMUX_SetFirstDsiplaySet (UINT32 ulDemuxId, UINT32 displayflag);


DxB_ERR_CODE TCC_DxB_DEMUX_SetParentLock (UINT32 ulDemuxId, UINT32 uiParentLock);
DxB_ERR_CODE TCC_DxB_DEMUX_RegisterCasDecryptCallback(UINT32 ulDemuxId, pfnDxb_DEMUX_CasDecrypt pfnCasCallback);

DxB_STANDARDS TCC_DxB_DEMUX_GetStandard(void);
DxB_ERR_CODE TCC_DxB_DEMUX_SendEvent(DxB_DEMUX_NOTIFY_EVT nEvent, void *pCallbackData);
DxB_ERR_CODE TCC_DxB_DEMUX_EnableAudioDescription(BOOLEAN isEnableAudioDescription);

#endif

