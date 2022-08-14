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

#ifndef _TCC_DXB_INTERFACE_VIDEO_H_
#define _TCC_DXB_INTERFACE_VIDEO_H_
#include "tcc_dxb_interface_type.h"

/**
 * Define video's notifying event
 */
typedef	enum
{
    DxB_VIDEO_NOTIFY_FIRSTFRAME_DISPLAYED,
    DxB_VIDEO_NOTIFY_CAPTURE_DONE,
    DxB_VIDEO_NOTIFY_USER_DATA_AVAILABLE,
    DxB_VIDEO_NOTIFY_VIDEO_DEFINITION_UPDATE,
	DxB_VIDEO_NOTIFY_EVT_END
} DxB_VIDEO_NOTIFY_EVT;

typedef void (*pfnDxB_VIDEO_EVENT_Notify)(DxB_VIDEO_NOTIFY_EVT nEvent, void *pCallbackData);


/********************************************************************************************/
/********************************************************************************************
						FOR MW LAYER FUNCTION
********************************************************************************************/
/********************************************************************************************/

DxB_ERR_CODE TCC_DxB_VIDEO_Init(void);
DxB_ERR_CODE TCC_DxB_VIDEO_Start(UINT32 ulDevId, UINT32 ulVideoFormat);
DxB_ERR_CODE TCC_DxB_VIDEO_Stop(UINT32 ulDevId);
DxB_ERR_CODE TCC_DxB_VIDEO_Pause(UINT32 ulDevId, BOOLEAN bOn );
DxB_ERR_CODE TCC_DxB_VIDEO_StartCapture(UINT32 ulDevId, UINT16 usTargetWidth, UINT16 usTargetHeight, UINT8 *pucFileName);

DxB_ERR_CODE TCC_DxB_VIDEO_Select_Display_Output(UINT32 ulDevId);
DxB_ERR_CODE TCC_DxB_VIDEO_RefreshDisplay(UINT32 ulDevId);
DxB_ERR_CODE TCC_DxB_VIDEO_IsSupportCountry(UINT32 ulDevId, UINT32 ulCountry);
DxB_ERR_CODE TCC_DxB_VIDEO_EnableDisplay(UINT32 ulDevId);
DxB_ERR_CODE TCC_DxB_VIDEO_DisableDisplay(UINT32 ulDevId);
DxB_ERR_CODE TCC_DxB_VIDEO_LCD_Update(UINT32 ulContentsType);

DxB_ERR_CODE TCC_DxB_VIDEO_GetVideoInfo(UINT32 ulDevId, UINT32 *videoWidth, UINT32 *videoHeight);
DxB_ERR_CODE TCC_DxB_VIDEO_RegisterEventCallback(UINT32 ulDevId, pfnDxB_VIDEO_EVENT_Notify pfnEventCallback);
DxB_ERR_CODE TCC_DxB_VIDEO_SetPause(UINT32 ulDemuxId, UINT32 pause);
DxB_ERR_CODE TCC_DxB_VIDEO_IFrameSearchEnable (UINT32 ulDemuxId);
DxB_ERR_CODE TCC_DxB_VIDEO_SetSinkByPass(UINT32 ulDemuxId, UINT32 sink);
DxB_ERR_CODE TCC_DxB_VIDEO_SendEvent(DxB_VIDEO_NOTIFY_EVT nEvent, void *pCallbackData);
DxB_ERR_CODE TCC_DxB_VIDEO_InitSurface(int arg);
DxB_ERR_CODE TCC_DxB_VIDEO_DeinitSurface();
DxB_ERR_CODE TCC_DxB_VIDEO_SetSurface(void *nativeWidow);
DxB_ERR_CODE TCC_DxB_VIDEO_Use(INT32 arg);
DxB_ERR_CODE TCC_DxB_VIDEO_Release(void);
DxB_ERR_CODE TCC_DxB_VIDEO_Subtitle(void *arg);
DxB_ERR_CODE TCC_DxB_VIDEO_SupportFieldDecoding(UINT32 OnOff);
DxB_ERR_CODE TCC_DxB_VIDEO_SupportIFrameSearch(UINT32 OnOff);
DxB_ERR_CODE TCC_DxB_VIDEO_SupportUsingErrorMB(UINT32 OnOff);
DxB_ERR_CODE TCC_DxB_VIDEO_SupportDirectDisplay(UINT32 OnOff);
unsigned int TCC_DxB_VIDEO_GetDisplayFlag(void);
DxB_ERR_CODE TCC_DxB_VIDEO_SetProprietaryData (UINT32 channel_index);

#endif

