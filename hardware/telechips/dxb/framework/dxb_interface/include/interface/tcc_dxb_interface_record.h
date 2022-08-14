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

#ifndef _TCC_DXB_INTERFACE_RECORD_H_
#define _TCC_DXB_INTERFACE_RECORD_H_
#include "tcc_dxb_interface_type.h"

typedef enum
{
	DxB_RECORD_EVENT_RECORDING_STOP,
	DxB_RECORD_EVENT_NOTIFY_END
} DxB_RECORD_NOTIFY_EVT;


typedef void (*pfnDxB_RECORD_EVENT_Notify )(DxB_RECORD_NOTIFY_EVT nEvent, void  *pEventData);


/********************************************************************************************/
/********************************************************************************************
						FOR MW LAYER FUNCTION
********************************************************************************************/
/********************************************************************************************/

DxB_ERR_CODE TCC_DxB_RECORD_Init(void);
DxB_ERR_CODE TCC_DxB_RECORD_Start(UINT32 ulDevId, UINT8 * pucFileName);
DxB_ERR_CODE TCC_DxB_RECORD_Stop(UINT32 ulDevId);
DxB_ERR_CODE TCC_DxB_RECORD_RegisterEventCallback(UINT32 ulDevId, pfnDxB_RECORD_EVENT_Notify pfnEventCallback);
DxB_ERR_CODE TCC_DxB_RECORD_SendEvent(DxB_RECORD_NOTIFY_EVT nEvent, void *pCallbackData);

#endif

