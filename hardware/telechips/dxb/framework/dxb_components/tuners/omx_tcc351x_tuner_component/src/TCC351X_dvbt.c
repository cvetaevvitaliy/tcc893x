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
#include "TCC351X_BOOT_DVBT.h"
static unsigned int guiNumberOfBB = 1, guiPrevFreq = -1;

int TCC351XDVBT_Open(int iCountryCode, int iCmdInterface, int iDataInterface, int iNumberOfBB)
{
    guiNumberOfBB = iNumberOfBB;
	TC3X_API_Init(0, guiNumberOfBB);
	TC3X_API_IOOpen(0, iCmdInterface, iDataInterface);

    #if 0
    {
    tTC3X_SettingOption SetOption;
    memset(&SetOption, 0xFF, sizeof(tTC3X_SettingOption));
    SetOption.STS_SPI_ClkSpeed_DLR = 0x03;
	// Standard             | DLR 0 | DLR 1 | DLR 2 | DLR 3 | DLR 4 | DLR 5 | DLR 6 | DLR 7
    // DVBT  (KHz)          | 67200 | 33600 | 22400 | 16800 | 13440 | 11200 | 9600  | 8400
    // TDMB  (KHz)          | 19200 | 9600  | 6400  | 4800  | 3840  | 3200  | 2743  | 2400
    // CMMB  (KHz)          | 55200 | 27600 | 18400 | 13800 | 11040 | 9200  | 7886  | 6900
    // ISDB-T 1SEG (KHz)    | 43200 | 21600 | 14400 | 10800 | 8640  | 7200  | 6172  | 5400
    // ISDB-T FULLSEG(KHz)  | 60000 | 30000 | 20000 | 15000 | 12000 | 10000 | 8572  | 7500
    TC3X_API_BBOpen_Opt (0, &SetOption);
  	}
    #else
    TC3X_API_BBOpen(0);
    #endif

    TC3X_API_BBPreInit(0, TC3X_STD_DVBT);
    TC3X_API_PreSet_PLLData(0, 0x60,0x01,0x1c,0x82,19200); //120M
	if( TC3X_API_ColdBootTable(0, &TCC351X_BOOT_DATA_DVBT[0],TCC351X_BOOT_SIZE_DVBT, CMD_BB_COLDBOOT_NORMAL) != 0x1ACCE551 )
	{
		TC3X_API_Close(0);
		return 1;
	}
	TC3X_API_BBInit(0);
	guiPrevFreq = -1; 
	return 0;
}

int TCC351XDVBT_Close(void)
{
	TC3X_API_Close(0);
	return 0;
}

int TCC351XDVBT_ChannelSet(int iChannel, int bLockOn)
{       
    //Don't Support this function(logical channel setting), Please use 
    //TCC351XDVBT_FrequencySet
   	ALOGE("%s:%d", __func__, __LINE__);
	return 1;
}

int TCC351XDVBT_FrequencySet(int iFreqKhz, int iBWKhz, int bLockOn)
{	
    int iChannel;
	tFreqSet_Option Option;	
	int LockOK = 1;	
	int i;
	
	for(i=0; i<guiNumberOfBB; i++)
		DVB_TCC35XX_Antenna_Init(i);

    Option.EnablePIDFiltering = 0;
	Option.Hierarchy = 0;	
    if( guiPrevFreq == iFreqKhz)
    {
        ALOGD("Same Frequency, Skip Setting !!!\n");
        return 0;
    }

	TC3X_API_SetFreq(0, iBWKhz, iFreqKhz, &Option);
	if (bLockOn)
		LockOK = TC3X_API_GetDVB_Lock_Wait(0, 0);
	
	if(LockOK) 
	{
		PRINTF_LV_2("Lock Success !!!\n");
		guiPrevFreq = iFreqKhz;
		return 0;
	}
	guiPrevFreq = -1; 
	PRINTF_LV_2("Lock Fail !!!\n");
	return 1;
}

int TCC351XDVBT_GetSignalStrength(void)
{
    int i, max_ant_level, ant_level[4]; //We support max 4 baseband
    double SNR=0, TSPER=0, Temp1, Temp2, vber, vber2;
    t_DVBLock Lock;
    unsigned int uivber, uitsper;
    double rssi, rssiavg;

    max_ant_level = -1;
    for(i=0; i<guiNumberOfBB; i++)
    {
    	TC3X_API_UpdateCMD_DVB_BERSET(0, i);
        TC3X_API_GetDVB_VITERBIBER (0, i, &vber, &vber2);
        TC3X_API_GetDVB_RSSI (0, i, &rssi, &rssiavg);
        TC3X_API_GetDVB_SNR(0, i, &Temp1, &SNR, &Temp2);
    	TC3X_API_GetDVB_Lock(0, i, &Lock);
        TC3X_API_GetDVB_TSPER(0, i, &TSPER, &Temp1);
  #if 0
	uivber = (unsigned int)(100000*vber2);
	uitsper = (unsigned int)(100000*TSPER);
	PRINTF_LV_0("\n [%d] monitoring\n", i);
	PRINTF_LV_0("SYNC[%d]\n", Lock.INV_SYNC_BYTE);
	PRINTF_LV_0("RSSI[%d] SNR[%d]\n", (int)(rssi), (unsigned int)(SNR));
	PRINTF_LV_0("VBER[%d] TSPER[%d]\n", uivber, uitsper);
#endif
	if(i==0)
        	ant_level[i] = DVB_Get_TCC35XX_Antenna(i, SNR, TSPER, Lock.INV_SYNC_BYTE);
        else 
	        ant_level[i] = DVB_Get_TCC35XX_Antenna(i, SNR, TSPER, Lock.TPS);

        if(ant_level[i] > max_ant_level)
            max_ant_level =  ant_level[i];
    }
	return max_ant_level;
}

int TCC351XDVBT_GetSignalQuality(void)
{
    return 100 - TCC351XDVBT_GetBER();
}

int TCC351XDVBT_GetSNR(void)
{
    int i, iSNR;
    double SNR, Temp1, Temp2, snrmax;

    snrmax = 0;
    for(i=0; i<guiNumberOfBB; i++)
    {	
        TC3X_API_GetDVB_SNR(0, i, &Temp1, &SNR, &Temp2);
        if(SNR>snrmax)
                snrmax=SNR;
    }

    SNR = snrmax;
    iSNR = SNR * 4;
    if (iSNR > 100) iSNR = 100;

	return iSNR;
}

int TCC351XDVBT_GetBER(void)
{
    int iBER;
    double BER, temp;
    TC3X_API_GetDVB_VITERBIBER (0, 0, &BER, &temp);
    iBER = BER*2500;
    if (iBER > 100) iBER = 100;

	return iBER;

}

int TCC351XDVBT_SetAntennaPower(int arg)
{
    TC3X_API_SetAntennaPower(0, arg);
    return 0;
}

