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
#ifndef __LINUXTV_FRONTEND_H__
#define __LINUXTV_FRONTEND_H__

#include <frontend.h>

int LinuxTV_Frontend_Open();
int LinuxTV_Frontend_Close();
int LinuxTV_Frontend_DiSEqC_ResetOverload();
int LinuxTV_Frontend_DiSEqC_SendCMD(unsigned char *msg, unsigned int len);
int LinuxTV_Frontend_DiSEqC_GetReply(unsigned char *msg);
int LinuxTV_Frontend_DiSEqC_SendBurst(int arg);
int LinuxTV_Frontend_SetTone(int arg);
int LinuxTV_Frontend_SetVoltage(int arg);
int LinuxTV_Frontend_EnableHighLNBVoltage(int arg);
int LinuxTV_Frontend_GetStatus();
int LinuxTV_Frontend_GetBER();
int LinuxTV_Frontend_GetSignalQuality();
int LinuxTV_Frontend_GetSignalStrength();
int LinuxTV_Frontend_GetSNR();
int LinuxTV_Frontend_GetUncorrectedBlocks();
int LinuxTV_Frontend_SetFrontend(unsigned int uiFreqKHz, unsigned int uiBWKHz, unsigned int uiLock);
int LinuxTV_Frontend_GetFrontend();
int LinuxTV_Frontend_SetTuneMode(int arg);
int LinuxTV_Frontend_GetEvent();
int LinuxTV_Frontend_DishNetwork_SendLegacyCMD(int arg);
int LinuxTV_Frontend_SetAntennaPower(int arg);
int LinuxTV_Frontend_BlindScan_Start();
int LinuxTV_Frontend_BlindScan_Cancel();
int LinuxTV_Frontend_BlindScan_Reset();
int LinuxTV_Frontend_BlindScan_GetState(int *state);
int LinuxTV_Frontend_BlindScan_GetInfo(int *percent, int *index, int *freqMHz, int *symKHz);

#endif //__LINUXTV_FRONTEND_H__
