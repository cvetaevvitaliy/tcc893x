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

#ifndef _TCC_DXB_INTERFACE_SC_H_
#define _TCC_DXB_INTERFACE_SC_H_
#include "tcc_dxb_interface_type.h"

typedef enum 
{
	DxB_SC_STD_ISO,			//ISO7816 -> T0 or T1
	DxB_SC_STD_CONAX,
	DxB_SC_STD_IRDETO,		// T14
	DxB_SC_STD_NDS, 		// NDS
	DxB_SC_STD_EMV2000
} DxB_SC_STD;

typedef enum 
{
	DxB_T_0 = 0,	//T0 default is IR
	DxB_T_1,		//T1 default is NA

	DxB_T_0_IR,
	DxB_T_0_CX,
	DxB_T_1_NA,
	DxB_T_1_ARIB,

	DxB_T_14 = 14
} DxB_SC_PROTOCOL;

typedef enum 
{
	DxB_SC_NOT_PRESENT = 0,
	DxB_SC_PRESENT
} DxB_SC_STATUS;

typedef enum 
{
	DxB_SC_REMOVED = 0, 
	DxB_SC_INSERTED,
	DxB_SC_EVENT_MAX
} DxB_SC_EVENT;

typedef enum DxB_SC_VccLevel {
	DxB_SC_VccLevel_e5V = 0,   /* 5v is default value */
	DxB_SC_VccLevel_e3V = 1    /* 3v  */  
} DxB_SC_VccLevel;

typedef void (*pfnDxB_SC_EVT_CALLBACK)(UINT32 unDeviceId, DxB_SC_STATUS nStatus);


/********************************************************************************************/
/********************************************************************************************
						FOR MW LAYER FUNCTION
********************************************************************************************/
/********************************************************************************************/

DxB_ERR_CODE TCC_DxB_SC_GetCapability(UINT32*pNumDevice);
DxB_ERR_CODE TCC_DxB_SC_NegotiatePTS(UINT32 unDeviceId, unsigned char *pucWriteBuf, int iNumToWrite, unsigned char *pucReadBuf, int *piNumRead);
DxB_ERR_CODE TCC_DxB_SC_SetParams(UINT32 unDeviceId, DxB_SC_PROTOCOL nProtocol, unsigned long ulMinClock, unsigned long ulSrcBaudrate, unsigned char ucFI, unsigned char ucDI, unsigned char ucN, unsigned char ucCWI, unsigned char ucBWI);
DxB_ERR_CODE TCC_DxB_SC_GetParams(UINT32 unDeviceId, DxB_SC_PROTOCOL *pnProtocol, unsigned long *pulClock, unsigned long *pulBaudrate, unsigned char *pucFI, unsigned char *pucDI, unsigned char *pucN, unsigned char *pucCWI, unsigned char *pucBWI);
DxB_ERR_CODE TCC_DxB_SC_GetCardStatus(UINT32 unDeviceId, DxB_SC_STATUS *pnStatus);
DxB_ERR_CODE TCC_DxB_SC_Reset (UINT32 unDeviceId, UINT8 *pucAtr, UINT32 *pucAtrlen);
DxB_ERR_CODE TCC_DxB_SC_TransferData(UINT32 unDeviceId, unsigned char *pucWriteBuf, int iNumToWrite, unsigned char *pucReadBuf, int *piNumRead);
DxB_ERR_CODE TCC_DxB_SC_ReadData(UINT32 unDeviceId, unsigned char *pucWriteBuf, int iNumToWrite, unsigned char *pucReadBuf, int *piNumRead);
DxB_ERR_CODE TCC_DxB_SC_WriteData(UINT32 unDeviceId, unsigned char *pucWriteBuf, int iNumToWrite, unsigned char *pucReadBuf, int *piNumRead);
DxB_ERR_CODE TCC_DxB_SC_RegisterCallback(UINT32 unDeviceId, pfnDxB_SC_EVT_CALLBACK pfnSCCallback);
DxB_ERR_CODE TCC_DxB_SC_SetVccLevel(UINT32 unDeviceId, DxB_SC_VccLevel in_vccLevel);
DxB_ERR_CODE TCC_DxB_SC_DownUpVCC(UINT32 unDeviceId, unsigned int unDownTime /*ms*/);
DxB_ERR_CODE TCC_DxB_SC_Activate(UINT32 unDeviceId);
DxB_ERR_CODE TCC_DxB_SC_Deactivate(UINT32 unDeviceId);
DxB_ERR_CODE TCC_DxB_SC_CardDetect(void);
DxB_ERR_CODE TCC_DxB_SC_Open(void);
DxB_ERR_CODE TCC_DxB_SC_Close(void);

#endif

