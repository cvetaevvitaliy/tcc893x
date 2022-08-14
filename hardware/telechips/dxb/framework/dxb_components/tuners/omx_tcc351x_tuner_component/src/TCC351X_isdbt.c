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
#include "TC3X_Common.h"
#include "TCC351X_BOOT_ISDBT1SEG.h"
int TCC351XISDBT_Open(int iCountryCode, int iCmdInterface, int iDataInterface)
{
	TC3X_API_Init(0, 1);
	TC3X_API_IOOpen(0, iCmdInterface, iDataInterface);
    TC3X_API_BBOpen(0);
    TC3X_API_BBPreInit(0, TC3X_STD_ISDBT1SEG);
#if 0
    TC3X_API_PreSet_PLLData(0, 0x60,0x05,0x7d,0x82,19200); //13 seg 
#else
    TC3X_API_PreSet_PLLData(0, 0x60,0x05,0x5a,0x02,19200); // 1seg
#endif

	if( TC3X_API_ColdBootTable(0, &TCC351X_BOOT_DATA_ISDBT1SEG[0],TCC351X_BOOT_SIZE_ISDBT1SEG, CMD_BB_COLDBOOT_NORMAL) != 0x1ACCE551 )
	{
		TC3X_API_Close(0);
		return 1;
	}
	TC3X_API_BBInit(0);
	ISDBTSPACE_CTuneSpace(iCountryCode);
	return 0;
}

int TCC351XISDBT_Close(void)
{
	TC3X_API_Close(0);
	return 0;
}

int TCC351XISDBT_ChannelSet(int iChannel, int bLockOn)
{
	int iFreq;
	int iBandWidth;
	int iFinetune;
	
	ISDBT_TCC35XX_Antenna_Init(0);
	
	iFreq=ISDBTSPACE_GetFrequency(iChannel);
	iBandWidth=ISDBTSPACE_GetBandwidth(iChannel);
	iFinetune=ISDBTSPACE_GetFinetune(iChannel);
	iFreq+=iFinetune;
	return TCC351XISDBT_FrequencySet(iFreq, iBandWidth*1000, bLockOn);
}

int TCC351XISDBT_FrequencySet(int iFreqKhz, int iBWKhz, int bLockOn)
{	
	tFreqSet_Option Option;	
	int LockOK = 1;	
	Option.EnablePIDFiltering = 0;
	Option.Hierarchy = 0;	
	TC3X_API_SetFreq(0, iBWKhz, iFreqKhz, &Option);
	if (bLockOn)
		LockOK = TC3X_API_GetISDBT_Lock_Wait(0, 0);
	
	if(LockOK) 
	{
		PRINTF_LV_2("Lock Success !!!\n");
		return 0;
	}
	PRINTF_LV_2("Lock Fail !!!\n");
	return 1;
}

int TCC351XISDBT_GetMinChannel(void)
{
	return ISDBTSPACE_GetMinChannel();
}

int TCC351XISDBT_GetMaxChannel(void)
{
	return ISDBTSPACE_GetMaxChannel();
}

int TCC351XISDBT_GetSignalStrength(void)
{
	double MER=0, TSPER=0, Temp1, Temp2;
	t_ISDBTLock Lock;

	TC3X_API_UpdateCMD_ISDBT_BERSET(0, 0);
	TC3X_API_GetISDBT_Lock(0, 0, &Lock);
	TC3X_API_GetISDBT_MER(0, 0, &MER, &Temp1);
	TC3X_API_GetISDBT_TSPER(0, 0, &TSPER, &Temp1);

	return ISDBT_Get_TCC35XX_Antenna(0, MER, TSPER, Lock.TMCC);
}

