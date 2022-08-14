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

#ifndef _TCC_DXB_INTERFACE_AUDIO_H_
#define _TCC_DXB_INTERFACE_AUDIO_H_
#include "tcc_dxb_interface_type.h"

typedef enum
{
	DxB_AUDIO_DUAL_Stereo,
	DxB_AUDIO_DUAL_LeftOnly,
	DxB_AUDIO_DUAL_RightOnly,
	DxB_AUDIO_DUAL_Mix
} DxB_AUDIO_STEREO_MODE;

typedef enum
{
	DxB_AUDIO_NOTIFY_FIRSTFRAME_SOUNDED,
	DxB_AUDIO_EVENT_NOTIFY_END
} DxB_AUDIO_NOTIFY_EVT;


typedef void (*pfnDxB_AUDIO_EVENT_Notify )(DxB_AUDIO_NOTIFY_EVT nEvent, void  *pEventData); // pEvent = DxB_AUDIO_CALLBACK_DATA *


/********************************************************************************************/
/********************************************************************************************
						FOR MW LAYER FUNCTION
********************************************************************************************/
/********************************************************************************************/

DxB_ERR_CODE TCC_DxB_AUDIO_Init(void);
DxB_ERR_CODE TCC_DxB_AUDIO_Start(UINT32 ulDevId, UINT32 ulAudioFormat);
DxB_ERR_CODE TCC_DxB_AUDIO_Stop(UINT32 ulDevId);
DxB_ERR_CODE TCC_DxB_AUDIO_SetStereo(UINT32 ulDevId, DxB_AUDIO_STEREO_MODE eMode);
DxB_ERR_CODE TCC_DxB_AUDIO_SetVolume(UINT32 ulDevId, UINT32 iVolume);
DxB_ERR_CODE TCC_DxB_AUDIO_SetVolumeF(UINT32 ulDevId, REAL32 leftVolume, REAL32 rightVolume);
DxB_ERR_CODE TCC_DxB_AUDIO_SetMute(UINT32 ulDevId, BOOLEAN bMute);
DxB_ERR_CODE TCC_DxB_AUDIO_SetAudioStartSyncWithVideo( BOOLEAN bEnable );
DxB_ERR_CODE TCC_DxB_AUDIO_SetAudioPatternToCheckPTSnSTC( int pattern, int waittime_ms, int droptime_ms, int jumptime_ms );
DxB_ERR_CODE TCC_DxB_AUDIO_Flush(void);
DxB_ERR_CODE TCC_DxB_AUDIO_DelayOutput(UINT32 ulDevId, INT32 delayMs);
DxB_ERR_CODE TCC_DxB_AUDIO_RegisterEventCallback(UINT32 ulDevId, pfnDxB_AUDIO_EVENT_Notify pfnEventCallback);
DxB_ERR_CODE TCC_DxB_Audio_SetPause(UINT32 ulDevId, UINT32 pause);
DxB_ERR_CODE TCC_DxB_Audio_SetSinkByPass(UINT32 ulDevId, UINT32 sink);
DxB_ERR_CODE TCC_DxB_AUDIO_SetAudioInfomation(UINT32 ulDevId, void* pAudioInfo);
DxB_ERR_CODE TCC_DxB_AUDIO_RecordStart(UINT32 ulDevId, UINT8 * pucFileName);
DxB_ERR_CODE TCC_DxB_AUDIO_RecordStop (UINT32 ulDevId);
DxB_ERR_CODE TCC_DxB_AUDIO_Select_Output (UINT32 ulDevId, UINT32 isEnableAudioOutput);
DxB_ERR_CODE TCC_DxB_AUDIO_IsSupportCountry(UINT32 uiDevId, UINT32 uiCountry);
DxB_ERR_CODE TCC_DxB_AUDIO_STREAM_RollBack(UINT32 ulDevId);
DxB_ERR_CODE TCC_DxB_AUDIO_SetDualMono (UINT32 ulDevId, int audio_mode);
DxB_ERR_CODE TCC_DxB_AUDIO_GetAudioType (int iDeviceID, int *piNumCh, int *piAudioMode);
DxB_ERR_CODE TCC_DxB_AUDIO_SetAudioStartNotify(UINT32 uiDevId);
#endif

