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

/****************************************************************************

Revision History

****************************************************************************

****************************************************************************/
#ifndef _TCC351X_TDMB_H_
#define _TCC351X_TDMB_H_
typedef enum
{
	STATE_IDLE,
	STATE_SEARCHING,	
	STATE_SEARCHED,	
	STATE_TDMB_PLAY,
	STATE_DAB_PLAY,
	STATE_MAX
}E_TDMB_STATE;

extern E_TDMB_STATE TCC351XTDMB_SetState(E_TDMB_STATE eState);
extern E_TDMB_STATE TCC351XTDMB_GetState(void);
extern int TCC351XTDMB_Open(int iCountryCode, int iCmdInterface,int iDataInterface);
extern int TCC351XTDMB_Close(void);
extern int TCC351XTDMB_ServiceControl(unsigned int uiServiceType, unsigned char *pucService, int icontrol);
extern int TCC351XTDMB_ServiceSelect(unsigned int uiServiceType, unsigned char *pucService);
extern int TCC351XTDMB_ServiceRelease(unsigned int uiServiceType, unsigned char *pucService);
extern int TCC351XTDMB_ChannelSet(int iChannel);
extern int TCC351XTDMB_FrequencySet(int iFreqKhz, int bLockOn);
extern int TCC351XTDMB_ParseData(unsigned char *pucData, unsigned int uiSize);
extern int TCC351XTDMB_AVServiceProcess(int moduleidx, unsigned char *pucData, unsigned int uiSize, int uiType);
extern int TCC351XTDMB_DataServiceProcess(int moduleidx, unsigned char *pucData, unsigned int uiSize, int uiType);
extern int TCC351XTDMB_ReadFIFO(int iHandle, unsigned char *pucData, unsigned int uiSize);
extern int TCC351XTDMB_FICProcess(int moduleidx, unsigned char *pucData, unsigned int uiSize, unsigned int uiCRC, int uiType);
extern int TCC351XTDMB_GetMinChannel(void);
extern int TCC351XTDMB_GetMaxChannel(void);
extern int TCC351XTDMB_SearchChannels(void);
extern int TCC351XTDMB_ChannelIndexScan(unsigned int uiFreqIndex);
extern int TCC351XTDMB_FrequencyScan(unsigned int uiFreq);
extern int TCC351XTDMB_SearchStopChannels(void);
extern int TCC351XTDMB_GetSignalStrength(unsigned int *pSNR, unsigned int *pPCBER, double *pdRSSI);
extern long long TCC351XTDMB_Util_GetTimeInterval(long long startTick);
#endif



