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
#ifndef _MN88472_DVBT_H_
#define _MN88472_DVBT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    unsigned int uiDVBSystem;       //0:DVBT, 1:DVBT2
    unsigned int uiFrequency;       //Unit Khz
    unsigned int uiBandwidth;       //Unit Khz
    unsigned int uiRotation;        //@DVBT -1, @DVBT2 - 1:Yes, 0:No
    unsigned int uiConstellation;   //@DVBT{ "QPSK","16QAM","64QAM"}, @DVBT2{"QPSK",	"16QAM", "64QAM", "256QAM"}
    unsigned int uiFECLength;       //@DVBT -1, @DVBT2{"16k LDPC", "64k LDPC"}
    unsigned int uiGuardInterval;   //@DVBT{"1/32","1/16",	"1/8",	"1/4"}, @DVBT2{	"1/32","1/16","1/8","1/4","1/128","19/128","19/256"}
    unsigned int uiCodeRate;        //@DVBT {"1/2",	"2/3",	"3/4",	"5/6",	"7/8"}, @DVBT2{"1/2",	"3/5",	"2/3",	"3/4",	"4/5",	"5/6"}
    unsigned int uiFFT;             //@DVBT{"2k","8k","4k"}, @DVBT2{"1k","2k","4k","8k","16k","32k"}
    unsigned int uiPLPNum;          //@DVBT-1, @DVBT2 - Number of PLP
    unsigned int uiPilotPP;         //@DVBT -1, @DVBT2{	"PP1",	"PP2",	"PP3",	"PP4",	"PP5",	"PP6",	"PP7",	"PP8"}
}DVBT_TUNE_STATUS;

int SetAntennaPower(int arg);
int MN88472DVBT_GetStatus(DVBT_TUNE_STATUS *pStatus);
int MN88472DVBT_Open(int iCountryCode, int iSystem); //iSystem : 0:DVBT, 1:DVBT2
int MN88472DVBT_Close(void);
int MN88472DVBT_ChannelSet(int iChannel, int bLockOn);
int MN88472DVBT_FrequencySet(int iFreqKhz, int iBWKhz, int iSystem, int bLockOn);
int MN88472DVBT_GetSignalStrength(void);
int MN88472DVBT_GetSignalQuality(void);
int MN88472DVBT_GetSNR(void);
int MN88472DVBT_GetBER(void);
int MN88472DVBT_GetDataPLPs(int *piPLPIds, int *piPLPNum);
int MN88472DVBT_SetDataPLP(int iPLPId);

#ifdef __cplusplus
}
#endif
#endif


