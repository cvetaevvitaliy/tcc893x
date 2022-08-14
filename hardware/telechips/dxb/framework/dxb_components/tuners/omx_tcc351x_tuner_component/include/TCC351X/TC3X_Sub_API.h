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

#ifndef __TC3X_Sub_API_H__
#define __TC3X_Sub_API_H__

#include "IO/TC3X_IO_UTIL.h"
#include "TC3X_Common.h"

void      TC3X_SubAPI_Init (int moduleidx, int NumDemodule);
void      TC3X_SubAPI_IOOpen (int moduleidx, int MainIO, int StreamIO);
void      TC3X_SubAPI_BBOpen (int moduleidx, tTC3X_SettingOption *pSetOption);
void      TC3X_SubAPI_BBPreInit (int moduleidx, int Standard, FN_v_ihivvvv EVENTFunction);
#if 0
void      TC3X_SubAPI_ColdBoot_FILE (int Standard, unsigned char *pData, unsigned int Size);
void      TC3X_SubAPI_ColdBoot_NANDFLASH (int Standard, unsigned char *FileName);
#endif
int       TC3X_SubAPI_BBColdBoot_FILE (int moduleidx, unsigned char *pData, unsigned int Size, int option);
int       TC3X_SubAPI_BBColdBoot_MEM (int moduleidx, unsigned char *pData, unsigned int Size, int option);
int       TC3X_SubAPI_BBColdBoot_NANDFLASH (int moduleidx, char *FileName, int option);
int       TC3X_SubAPI_BBColdBoot_NONWrite(int moduleidx);
int       TC3X_SubAPI_COLDBOOT (int moduleidx, TC3X_BOOTBIN * BootBin, int option);
void      TC3X_SubAPI_BBInit (int moduleidx, int UseI2CRepeater);
void      TC3X_SubAPI_Close (int moduleidx);

void      TC3X_SubAPI_PWON (int moduleidx);
void      TC3X_SubAPI_PWDN (int moduleidx);
void      TC3X_SubAPI_PWRESET (int moduleidx);
void      TC3X_SubAPI_SetAntennaPower(int moduleidx, int arg);
void      TC3X_SubAPI_MainIOSel (int moduleidx, int IOSeries);
void      TC3X_SubAPI_StreamIOSel (int moduleidx, int IOSeries);
int       TC3X_SubAPI_BBDetect (int moduleidx, tTC3X_SettingOption *pSetOption);
void      TC3X_SubAPI_BBAttach (int moduleidx, int DeviceBB, tTC3X_SettingOption *pSetOption);
int       TC3X_SubAPI_SetFrequency (int moduleidx, int freq_khz, int bw_khz, tFreqSet_Option * pOption);
void      TC3X_SubAPI_BBStreamOutStart (int moduleidx);
void      TC3X_SubAPI_BBStreamOutStop (int moduleidx);
void      TC3X_SubAPI_BBPwCtrl (int moduleidx);
void      TC3X_SubAPI_RFPwCtrl (int moduleidx);
void      TC3X_SubAPI_BBDetach (int moduleidx);
void      TC3X_SubAPI_RFDetach (int moduleidx);

extern tSTDInfo g_tSTDInfo[MAX_TCC3X];
extern tSTDCtrl **g_pSTDCtrl;

#endif
