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

#ifndef _TCC_DXB_INTERFACE_PVR_H_
#define _TCC_DXB_INTERFACE_PVR_H_
#include "tcc_dxb_interface_type.h"

typedef enum
{
	DxB_PVR_EVENT_RECORDING_STOP,
	DxB_PVR_EVENT_PLAYING_STOP,
	DxB_PVR_EVENT_PLAYING_CURRENT_TIME,
	DxB_PVR_EVENT_NOTIFY_END
} DxB_PVR_NOTIFY_EVT;

typedef enum {
	LOCAL_PLAY_STOP = 0,
	FAST_PLAY_STOP,
	SEEK_TO_STOP,
} DxB_PVR_STOP_EVENT;

typedef int		(*pfnPVRUpdatePCR)(int itype, int index, long long lpts);
typedef int		(*pfnPVRBufferCallback)(unsigned char *pucBuff, int iSize);
typedef void (*pfnDxB_PVR_EVENT_Notify)(DxB_PVR_NOTIFY_EVT nEvent, void *pCallbackData);

DxB_ERR_CODE TCC_DxB_PVR_Init(DxB_STANDARDS Standard, UINT32 ulBaseBandType);
DxB_ERR_CODE TCC_DxB_PVR_DeInit(void);
DxB_ERR_CODE TCC_DxB_PVR_RecordStart (UINT32 ulPvrId, UINT8 * pucFileName, void *pstDB);
DxB_ERR_CODE TCC_DxB_PVR_RecordStop (UINT32 ulPvrId);
DxB_ERR_CODE TCC_DxB_PVR_SetDualMode(UINT32 ulPvrId, int dual_mode);
DxB_ERR_CODE TCC_DxB_PVR_PlayStart (UINT32 ulPvrId, UINT8 * pucFileName);
DxB_ERR_CODE TCC_DxB_PVR_PlayStop (UINT32 ulPvrId);
DxB_ERR_CODE TCC_DxB_PVR_SeekTo(UINT32 ulPvrId, int iSeekTime);
DxB_ERR_CODE TCC_DxB_PVR_SeekComplete(UINT32 ulPvrId);
DxB_ERR_CODE TCC_DxB_PVR_SetPlaySpeed(UINT32 ulPvrId, int iSpeed);
DxB_ERR_CODE TCC_DxB_PVR_SetPlayPause(UINT32 ulPvrId, BOOLEAN bPlayPause);
DxB_ERR_CODE TCC_DxB_PVR_GetDuration(UINT32 ulPvrId, int *pDuration);
DxB_ERR_CODE TCC_DxB_PVR_GetCurrentPosition(UINT32 ulPvrId, int *pPosition);
DxB_ERR_CODE TCC_DxB_PVR_GetReadPosition(UINT32 ulPvrId, int *pBlockSize);
DxB_ERR_CODE TCC_DxB_PVR_GetWritePosition(UINT32 ulPvrId, int *pBlockSize);
DxB_ERR_CODE TCC_DxB_PVR_GetTotalSize(UINT32 ulPvrId, int *pBlockSize);
DxB_ERR_CODE TCC_DxB_PVR_GetMaxSize(UINT32 ulPvrId, int *pMaxSize);
DxB_ERR_CODE TCC_DxB_PVR_GetFreeSize(UINT32 ulPvrId, int *pBlockSize);
DxB_ERR_CODE TCC_DxB_PVR_GetBlockSize(UINT32 ulPvrId, int *pBlockSize);
DxB_ERR_CODE TCC_DxB_PVR_GetFSType(UINT32 ulPvrId, char *pFSType);
DxB_ERR_CODE TCC_DxB_PVR_RegisterEventCallback(UINT32 ulPvrId, pfnDxB_PVR_EVENT_Notify pfnEventCallback);
DxB_ERR_CODE TCC_DxB_PVR_SendEvent(DxB_PVR_NOTIFY_EVT nEvent, void *pCallbackData);

#endif

