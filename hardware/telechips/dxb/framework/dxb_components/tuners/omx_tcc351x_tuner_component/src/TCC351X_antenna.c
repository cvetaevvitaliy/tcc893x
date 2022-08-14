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

#define MAXMOVINGAVG 4
static double g_dSNR_MER_Array[4][MAXMOVINGAVG];
static int g_iSNR_MERCnt[4] = {0,0,0,0};

void DVB_TCC35XX_Antenna_Init(int DeviceIdx)
{
    g_iSNR_MERCnt[DeviceIdx]= 0;
    memset(&g_dSNR_MER_Array[DeviceIdx][0], 0, sizeof(double)*MAXMOVINGAVG);
}

void ISDBT_TCC35XX_Antenna_Init(int DeviceIdx)
{
    g_iSNR_MERCnt[DeviceIdx] = 0;
    memset(&g_dSNR_MER_Array[DeviceIdx][0], 0, sizeof(double)*MAXMOVINGAVG);
}

unsigned char DVB_Get_TCC35XX_Antenna(int DeviceIdx, double snr_mer, double tsper, int bLock)
{
	unsigned char retLevel;
    int i;
    double sum;
    double SNR_MER_MVAVG;
    memmove(&g_dSNR_MER_Array[DeviceIdx][0], &g_dSNR_MER_Array[DeviceIdx][1], sizeof(double)*(MAXMOVINGAVG-1));

    if(tsper>0.3)
        g_dSNR_MER_Array[DeviceIdx][MAXMOVINGAVG-1] = 0;
    else 
        g_dSNR_MER_Array[DeviceIdx][MAXMOVINGAVG-1] = snr_mer;

    sum = 0;
    for(i=0; i<MAXMOVINGAVG; i++)
    {
        sum+=g_dSNR_MER_Array[DeviceIdx][i];
    }
    if(g_iSNR_MERCnt[DeviceIdx]<MAXMOVINGAVG)
        g_iSNR_MERCnt[DeviceIdx]++;

    if(g_iSNR_MERCnt[DeviceIdx])
        SNR_MER_MVAVG = sum/g_iSNR_MERCnt[DeviceIdx];
	
	if(bLock==1)
	{
		if (SNR_MER_MVAVG > 20)
			SNR_MER_MVAVG = 20;

		retLevel = SNR_MER_MVAVG * 5;

		PRINTF_LV_2("[%d] Lock  Success(SNR Avg:%lf, Level:%d)\n", DeviceIdx, SNR_MER_MVAVG, retLevel);
	}
    else
	{
		// none

		PRINTF_LV_2("[%d] Lock  Fail(Level:0)\n", DeviceIdx);
        retLevel = 0;
	}

	return(retLevel);
}

unsigned char ISDBT_Get_TCC35XX_Antenna(int DeviceIdx, double snr_mer, double tsper, int bLock)
{
	unsigned char retLevel;
    int i;
    double sum;
    double SNR_MER_MVAVG;
    memmove(&g_dSNR_MER_Array[DeviceIdx][0], &g_dSNR_MER_Array[DeviceIdx][1], sizeof(double)*(MAXMOVINGAVG-1));

    if(tsper>0.3)
        g_dSNR_MER_Array[DeviceIdx][MAXMOVINGAVG-1] = 0;
    else 
        g_dSNR_MER_Array[DeviceIdx][MAXMOVINGAVG-1] = snr_mer;

    sum = 0;
    for(i=0; i<MAXMOVINGAVG; i++)
    {
        sum+=g_dSNR_MER_Array[DeviceIdx][i];
    }
    if(g_iSNR_MERCnt[DeviceIdx]<MAXMOVINGAVG)
        g_iSNR_MERCnt[DeviceIdx]++;

    if(g_iSNR_MERCnt[DeviceIdx])
        SNR_MER_MVAVG = sum/g_iSNR_MERCnt[DeviceIdx];
	
	if(bLock==1)
	{
		if(SNR_MER_MVAVG<4)
		{
            retLevel = 0;
		}
		else if(SNR_MER_MVAVG<8)
		{
            retLevel = 1;
		}
		else if(SNR_MER_MVAVG<12)
		{
            retLevel = 2;
		}
		else if(SNR_MER_MVAVG<15)
		{
            retLevel = 3;
		}
		else if(SNR_MER_MVAVG<20)
		{
            retLevel = 4;
		}
		else
		{
            retLevel = 5;
		}

		PRINTF_LV_2("Lock Success(SNR Avg:%lf, Level:%d)\n", SNR_MER_MVAVG, retLevel);
	}
    else
	{
		// none

		PRINTF_LV_2("Lock  Fail(Level:0)\n");
        retLevel = 0;
	}

	return(retLevel);
}

unsigned char TDMB_Get_TCC35XX_Antenna(double mvavg, int Lock_CTO, int Lock_CFO)
{
	unsigned char retLevel = 0;
	unsigned int uiPCBER;

	if(Lock_CTO == 0 || Lock_CFO == 0)	
	{
		PRINTF_LV_2("Lock  Fail(Level:0)\n");
		return 0; //unlock status
	}

	uiPCBER = (unsigned int)(mvavg*100000);

	if(uiPCBER <= 6000)
	{
        retLevel = 5;   // viterbi 10-6
	}   
	else if(uiPCBER <= 8000)
	{
        retLevel = 4;   // viterbi 10-5
	}
	else if(uiPCBER <= 10000)
	{
        retLevel = 3;   // viterbi 10-4
	}
	else if(uiPCBER <= 11000)
	{
        retLevel = 2;   // viterbi 10-3
	}
	else if(uiPCBER <= 13000)
	{
        retLevel = 1;   // viterbi 10-3
	}
	else if(uiPCBER > 13000)
	{
        retLevel = 0;   // viterbi 10-2
	}

	return(retLevel);
}

