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
#ifndef _TCC_DXB_INTERFACE_TUNER_H_
#define _TCC_DXB_INTERFACE_TUNER_H_

#include "tcc_dxb_interface_type.h"

/*******************************************************************/
/****************************** typedef ****************************/
/*******************************************************************/

typedef enum
{
	DxB_TUNER_EVENT_NOTIFY_LOCK_TIMEOUT, 
	DxB_TUNER_EVENT_NOTIFY_SIGNAL_STATUS, 
	DxB_TUNER_EVENT_NOTIFY_NO_SIGNAL, 
	DxB_TUNER_EVENT_NOTIFY_SIGNAL, 
	DxB_TUNER_EVENT_NOTIFY_SEARCHED_CHANNEL,
	DxB_TUNER_EVENT_NOTIFY_SET_CHANNEL,
	DxB_TUNER_EVENT_NOTIFY_UPDATE_EWS,
	DxB_TUNER_EVENT_NOTIFY_UPDATE_DATA,
	DxB_TUNER_EVENT_NOTIFY_END
} DxB_TUNER_NOTIFY_EVT;

typedef void (*pfnDxB_TUNER_EVENT_Notify)(DxB_TUNER_NOTIFY_EVT nEvent, void *pCallbackData);
/*******************************************************************/
/****************************** functions ****************************/
/*******************************************************************/

DxB_ERR_CODE TCC_DxB_TUNER_Init (UINT32 ulStandard, UINT32 ulTunerType);
DxB_ERR_CODE TCC_DxB_TUNER_Init_Ex (UINT32 ulStandard, UINT32 ulTunerType, UINT32 ulTunerID);
DxB_ERR_CODE TCC_DxB_TUNER_SetNumberOfBB(UINT32 ulTunerID, UINT32 ulNumberOfBB);
DxB_ERR_CODE TCC_DxB_TUNER_SetCountrycode (UINT32 ulTunerID, UINT32 ulCountrycode);
DxB_ERR_CODE TCC_DxB_TUNER_SetModulation (UINT32 ulTunerID, UINT32 ulModulation);
DxB_ERR_CODE TCC_DxB_TUNER_Open(UINT32 ulTunerID, UINT32 *pulArg);
DxB_ERR_CODE TCC_DxB_TUNER_Close(UINT32 ulTunerID);
DxB_ERR_CODE TCC_DxB_TUNER_SearchChannel(UINT32 ulTunerID, UINT32 ulChannel);
/* If ulBandWidthKhz is zero, tuner selects bandwidth automatically.
 */
DxB_ERR_CODE TCC_DxB_TUNER_SearchFrequencyWithBW(UINT32 ulTunerID, UINT32 ulFrequencyKhz, UINT32 ulBandWidthKhz, UINT32 ulLockWait, UINT32 *pOptions,  UINT32 *pSnr);
DxB_ERR_CODE TCC_DxB_TUNER_GetChannelIndexByFrequency(UINT32 ulTunerID, UINT32 ulFrequencyKhz, UINT32 *pulChannelIndex);
DxB_ERR_CODE TCC_DxB_TUNER_SearchAnalogChannel(UINT32 ulTunerID, UINT32 ulChannel);
DxB_ERR_CODE TCC_DxB_TUNER_ScanChannel(UINT32 ulTunerID, UINT32 ulChannel);
DxB_ERR_CODE TCC_DxB_TUNER_ScanFreqChannel(UINT32 ulTunerID, UINT32 ulChannel);
DxB_ERR_CODE TCC_DxB_TUNER_SetChannel(UINT32 ulTunerID, UINT32 ulChannel, UINT32 lockOn);
DxB_ERR_CODE TCC_DxB_TUNER_GetChannelInfo(UINT32 ulTunerID, void *pChannelInfo);
DxB_ERR_CODE TCC_DxB_TUNER_GetSignalStrength(UINT32 ulTunerID, void *pStrength);
DxB_ERR_CODE TCC_DxB_TUNER_GetSignalStrengthIndex (UINT32 ulTunerID, int *strength_index);
DxB_ERR_CODE TCC_DxB_TUNER_GetSignalStrengthIndexTime (UINT32 ulTumerID, int *tuner_str_idx_time);
DxB_ERR_CODE TCC_DxB_TUNER_GetRFInformation(UINT8 **ppucData, UINT32 *puiSize);
DxB_ERR_CODE TCC_DxB_TUNER_SetAntennaPower(UINT32 ulTunerID, UINT32 ulArg);

DxB_ERR_CODE TCC_DxB_TUNER_Data_Stream_Stop(UINT32 ulTunerID);

DxB_ERR_CODE TCC_DxB_TUNER_RegisterPID(UINT32 ulTunerID, UINT32 ulPID);
DxB_ERR_CODE TCC_DxB_TUNER_UnRegisterPID(UINT32 ulTunerID, UINT32 ulPID);
DxB_ERR_CODE TCC_DxB_TUNER_RegisterEventCallback(UINT32 ulTunerID, pfnDxB_TUNER_EVENT_Notify pfnEventCallback);
DxB_ERR_CODE TCC_DxB_TUNER_SendEvent(DxB_TUNER_NOTIFY_EVT nEvent, void *pCallbackData);
DxB_ERR_CODE TCC_DxB_TUNER_Get_EWS_Flag(UINT32 ulTunerID, UINT32 *ulStartFlag);

DxB_ERR_CODE TCC_DxB_TUNER_cas_open(unsigned char _casRound, unsigned char * _systemKey);
DxB_ERR_CODE TCC_DxB_TUNER_cas_key_multi2(unsigned char _parity, unsigned char *_key, unsigned char _keyLength, unsigned char *_initVector, unsigned char _initVectorLength);
DxB_ERR_CODE TCC_DxB_TUNER_cas_set_pid(unsigned int *_pids, unsigned int _numberOfPids);
DxB_ERR_CODE TCC_DxB_TUNER_setFreqBand (UINT32 ulTunerID, int freq_band);
DxB_ERR_CODE TCC_DxB_TUNER_GetDataPLPs(UINT32 ulTunerID, UINT32 *pulPLPId, UINT32 *pulPLPNum);
DxB_ERR_CODE TCC_DxB_TUNER_SetDataPLP(UINT32 ulTunerID, UINT32 ulPLPId);

DxB_ERR_CODE TCC_DxB_TUNER_SetLNBVoltage(UINT32 ulTunerID, UINT32 ulArg);
DxB_ERR_CODE TCC_DxB_TUNER_SetTone(UINT32 ulTunerID, UINT32 ulArg);
DxB_ERR_CODE TCC_DxB_TUNER_DiseqcSendBurst(UINT32 ulTunerID, UINT32 ulCMD);
DxB_ERR_CODE TCC_DxB_TUNER_DiseqcSendCMD(UINT32 ulTunerID, unsigned char *pucCMD, UINT32 ulLen);
DxB_ERR_CODE TCC_DxB_TUNER_GetChannelValidity (UINT32 ulTunerID, int *pChannel);
DxB_ERR_CODE TCC_DxB_TUNER_BlindScan_Reset(UINT32 ulTunerID);
DxB_ERR_CODE TCC_DxB_TUNER_BlindScan_Start(UINT32 ulTunerID);
DxB_ERR_CODE TCC_DxB_TUNER_BlindScan_Cancel(UINT32 ulTunerID);
DxB_ERR_CODE TCC_DxB_TUNER_BlindScan_GetState(UINT32 ulTunerID, unsigned int *puiState);
DxB_ERR_CODE TCC_DxB_TUNER_BlindScan_GetInfo(UINT32 ulTunerID, unsigned int *puiInfo);

#endif
