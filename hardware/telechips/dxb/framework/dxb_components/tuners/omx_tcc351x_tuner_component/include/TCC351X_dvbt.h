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
#ifndef _TCC351X_DVBT_H_
#define _TCC351X_DVBT_H_

int TCC351XDVBT_Open(int iCountryCode, int iCmdInterface, int iDataInterface, int iNumberOfBB);
int TCC351XDVBT_Close(void);
int TCC351XDVBT_ChannelSet(int iChannel, int bLockOn);
int TCC351XDVBT_FrequencySet(int iFreqKhz, int iBWKhz, int bLockOn);
int TCC351XDVBT_GetSignalStrength(void);
int TCC351XDVBT_GetSignalQuality(void);
int TCC351XDVBT_GetSNR(void);
int TCC351XDVBT_GetBER(void);
int TCC351XDVBT_SetAntennaPower(int arg);

#endif


