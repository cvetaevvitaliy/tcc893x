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
#ifndef _MxL101SF_DVBT_H_
#define _MxL101SF_DVBT_H_

#ifdef __cplusplus
extern "C" {
#endif

int SetAntennaPower(int arg);
int MxL101SFDVBT_Open(int iCountryCode, int iCmdInterface, int iDataInterface, int iNumberOfBB);
int MxL101SFDVBT_Close(void);
int MxL101SFDVBT_ChannelSet(int iChannel, int bLockOn);
int MxL101SFDVBT_FrequencySet(int iFreqKhz, int iBWKhz, int bLockOn, int *iSNR);
int MxL101SFDVBT_GetSignalQuality(void);
int MxL101SFDVBT_GetSignalStrength(void);
int MxL101SFDVBT_GetSNR(void);
int MxL101SFDVBT_GetBER(void);

#ifdef __cplusplus
}
#endif
#endif


