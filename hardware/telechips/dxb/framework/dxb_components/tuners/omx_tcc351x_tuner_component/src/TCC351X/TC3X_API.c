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

#include "TC3X_API.h"
#include "TC3X_Sub_API.h"
#include "TC3X_Common.h"
#include "IO/TC3X_IO_UTIL.h"
#include "IO/TC3X_IO.h"
#if defined(USE_ANDROID) || defined(USE_LINUX)
#if defined (USE_BAND_SWITCH_GPIOC_06)
#include "IO/TC3X_GPIO.h"
#endif // USE_BAND_SWITCH_GPIOC_06
#endif

#if defined (USE_BAND_SWITCH_GPIOC_06)
#ifndef BITSET
#define	BITSET(X, MASK)				( (X) |= (unsigned int)(MASK) )
#endif
#ifndef BITCLR
#define	BITCLR(X, MASK)				( (X) &= ~((unsigned int)(MASK)) )
#endif
#ifndef Hw6
#define	Hw6		0x00000040
#endif
#endif // USE_BAND_SWITCH_GPIOC_06

int       TCC351XTDMB_FICProcess (int moduleidx, unsigned char *pucData, unsigned int uiSize, unsigned int uiCRC,
                                  int uiType);
int       TCC351XTDMB_AVServiceProcess (int moduleidx, unsigned char *pucData, unsigned int uiSize, int uiType);
int       TCC351XTDMB_DataServiceProcess(int moduleidx, unsigned char *pucData, unsigned int uiSize, int uiType);

//  Sequence : IO Open - BB Open - BB PreInit(standard) - ColdBoot(standard) - BBInit(standard) - RF Open - RF Init

//  SetFreq Sequence : Stream Stop - Stream Start - SetFreq
//  Change Freq (same standard) : Same as SetFreq Sequence
//  Change Freq (diff standard) : CMD_BB_STREAMOUT_STOP - BB PreInit(standard) - ColdBoot(standard) - BBInit(standard) - SetFreq Sequence

tTC3X_SettingOption gt_TC3X_SetOption = {
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF
};

// usage with option    :   void TC3X_API_BBOpen_Opt (int moduleidx, &gt_TC3X_SetOption);
// usage without option :   void TC3X_API_BBOpen (int moduleidx, NULL);

//--------------------------------------------------------------------------
// common
//

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_Init
//    Description : Application initialize code
//    Input : 
//      moduleidx : module index 
//      NumOfDemodule : Number of Demodulator
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_Init (int moduleidx, int NumOfDemodule)
{
    PRINTF_LV_2 ("[TC3X]-----------------\n");
    PRINTF_LV_2 ("# APIInit \n");
    PRINTF_LV_2 ("# NumberOfDemodule [%d]\n", NumOfDemodule);
    TC3X_SubAPI_Init (moduleidx, NumOfDemodule);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_IOOpen
//    Description : IO Open API
//    Input : 
//      moduleidx : module index
//      MainIO : main IO
//      StreamIO : stream IO
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_IOOpen (int moduleidx, int MainIO, int StreamIO)
{
    char     *ifLabel[5] = { "SRAMLIKE", "I2C", "CSPI", "SDIO1BIT", "SDIO4BIT" };
    char     *streamioLabel[7] = { "MAINIO", "PTS", "STS", "SPIMS", "SPISLV", "HPI_HEADERON", "HPI_HEADEROFF" };

    PRINTF_LV_2 ("# CommandIO : %d\n", MainIO);

    TC3X_SubAPI_IOOpen (moduleidx, MainIO, StreamIO);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_BBOpen
//    Description : Find BaseBand & Attach
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_BBOpen (int moduleidx)
{
    PRINTF_LV_2 ("# BB Open\n");
    TC3X_SubAPI_BBOpen (moduleidx, NULL);       // find TC3X
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_BBOpen_Opt
//    Description : Find BaseBand & Attach
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_BBOpen_Opt (int moduleidx, tTC3X_SettingOption * pSetOption)
{
    PRINTF_LV_2 ("# BB Open\n");
    TC3X_SubAPI_BBOpen (moduleidx, pSetOption); // find TC3X
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_Close
//    Description : Close BaseBand
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_Close (int moduleidx)
{
    TC3X_SubAPI_Close (moduleidx);
    PRINTF_LV_2 ("# BB Close\n");
    PRINTF_LV_2 ("[TC3X End]------------\n");
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_SetAntennaPower
//    Description : Set the mode for antenna power control
//    Input : 
//      moduleidx : module index
//      arg       : 0 - off, 1 - on, 2 - auto
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_SetAntennaPower(int moduleidx, int arg)
{
	TC3X_SubAPI_SetAntennaPower(moduleidx, arg);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_BBPreInit
//    Description : BaseBand Pre-Initialize & Register Callback Function
//    Input : 
//      moduleidx : module index
//      Standard : Standard
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_BBPreInit (int moduleidx, int Standard)
{
    if (Standard >= TC3X_STD_MAX)
    {
        PRINTF_LV_0 ("BBPreInit Standard error\n");
        return;
    }

    PRINTF_LV_2 ("# BBPreInit\n");

    switch (Standard)
    {
#if defined (STDDEF_DVBT) || defined (STDDEF_DVBH)
    case TC3X_STD_DVBT:
        TC3X_SubAPI_BBPreInit (moduleidx, Standard, TC3X_API_DVBT_OccurEvent);
        break;
#endif // STDDEF_DVBT
#if defined (STDDEF_DVBH)
    case TC3X_STD_DVBH:
        TC3X_SubAPI_BBPreInit (moduleidx, Standard, TC3X_API_DVBH_OccurEvent);
        break;
#endif // STDDEF_DVBH
#if defined (STDDEF_CMMB)
    case TC3X_STD_CMMB:
        TC3X_SubAPI_BBPreInit (moduleidx, Standard, TC3X_API_CMMB_OccurEvent);
        break;
#endif // STDDEF_CMMB
#if defined (STDDEF_DMB)
    case TC3X_STD_DMB:
        TC3X_SubAPI_BBPreInit (moduleidx, Standard, TC3X_API_TDMB_OccurEvent);
        break;
#endif // STDDEF_DMB

#if defined(STDDEF_ISDBT1SEG)
    case TC3X_STD_ISDBT1SEG:
        TC3X_SubAPI_BBPreInit (moduleidx, Standard, TC3X_API_ISDBT1SEG_OccurEvent);
        break;
#endif

#if defined(STDDEF_ISDBT13SEG)
    case TC3X_STD_ISDBT13SEG:
        TC3X_SubAPI_BBPreInit (moduleidx, Standard, TC3X_API_ISDBT13SEG_OccurEvent);
        break;
#endif
    default:
        PRINTF_LV_0 ("Unknown Standard\n");
        return;
        break;
    }
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_PreSet_PLLData
//    Description : Optional Function. PLL Control Value Setting. Apply before ColdBoot.
//    Input : 
//      moduleidx : module index
//      PLL_WaitTime
//      PLL_P
//      PLL_M
//      PLL_S
//      OSC_Khz : Oscillator clk
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_PreSet_PLLData (int moduleidx, unsigned char PLL_WaitTime, unsigned char PLL_P, unsigned char PLL_M,
                              unsigned char PLL_S, unsigned int OSC_Khz)
{
    unsigned char PLL_WPMS[4];

    if (!g_pSTDCtrl)
        return;
    if (!g_pSTDCtrl[moduleidx])
        return;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return;

    PLL_WPMS[0] = PLL_WaitTime;
    PLL_WPMS[1] = PLL_P;
    PLL_WPMS[2] = PLL_M;
    PLL_WPMS[3] = PLL_S;

    g_pSTDCtrl[moduleidx][0].BB_UserFunction (moduleidx, 0, CMD_BB_PLL_SET, PLL_WPMS, &OSC_Khz, NULL, NULL);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_ColdBootTable
//    Description : Write Boot Code Binary File to BB
//    Input : 
//      moduleidx : module index
//      pData : Boot Binary Data Pointer
//      size : size of boot binary
//      option : broadcast / normal / non write
//    Output : 
//      Success: 0x1ACCE551 Fail : others
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_ColdBootTable (int moduleidx, unsigned char *pData, int size, int option)
{
    int       ret;
    PRINTF_LV_2 ("# ColdBootTable [%d]\n", size);

    if (option == CMD_BB_COLDBOOT_BROADCAST)
        ret = TC3X_SubAPI_BBColdBoot_FILE (moduleidx, pData, size, CMD_BB_COLDBOOT_BROADCAST);
    else if (option == CMD_BB_COLDBOOT_NON_WRITE)
        ret = TC3X_SubAPI_BBColdBoot_NONWrite (moduleidx);
    else        // (option==CMD_BB_COLDBOOT_NORMAL)
        ret = TC3X_SubAPI_BBColdBoot_FILE (moduleidx, pData, size, CMD_BB_COLDBOOT_NORMAL);

    return ret;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_BBInit
//    Description : BB Initialization (if use i2c, i2c repeater mode will be enable)
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_BBInit (int moduleidx)
{
    if (g_tSTDInfo[moduleidx].MainIO == TC3X_IF_I2C)
    {
        TC3X_SubAPI_BBInit (moduleidx, 0);
    }
    else
    {
        TC3X_SubAPI_BBInit (moduleidx, 0);
    }
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_REG_Write
//    Description : Register write to BB
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      addr : Address
//      data : data
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_REG_Write (int moduleidx, int DeviceIdx, int addr, int data)
{
    if (!g_pSTDCtrl)
        return;
    if (!g_pSTDCtrl[moduleidx])
        return;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return;

    g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB, CMD_BB_REG_W,
                                                      &addr, &data, NULL, NULL);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_REG_Read
//    Description : Register read from BB
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      addr : Address
//    Output : 
//      Read Value
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_REG_Read (int moduleidx, int DeviceIdx, int addr)
{
    int       ret;

    if (!g_pSTDCtrl)
        return 0;
    if (!g_pSTDCtrl[moduleidx])
        return 0;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return 0;

    ret =
        g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB, CMD_BB_REG_R,
                                                          &addr, NULL, NULL, NULL);
    return ret;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_SetFreq
//    Description : BB/RF frequency setting
//    Input : 
//      moduleidx : module index
//      bw_khz : bandwidth
//      freq_khz : frequency
//      pOption : option
//    Output : 
//      int (0:Fail, 1:Success)
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_SetFreq (int moduleidx, unsigned int bw_khz, unsigned int freq_khz, tFreqSet_Option * pOption)
{
#if defined (USE_BAND_SWITCH_GPIOC_06)
    int       value;
    int       i;
#endif // USE_BAND_SWITCH_GPIOC_06

    if (!g_pSTDCtrl)
        return 0;
    if (!g_pSTDCtrl[moduleidx])
        return 0;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return 0;

#if defined (USE_BAND_SWITCH_GPIOC_06)
    for (i = 0; i < g_tSTDInfo[moduleidx].NumOfDemodule; i++)
    {
        // GPIO C6 Direction Setting
        value = TC3X_API_REG_Read (moduleidx, i, 0x11);
        BITSET (value, Hw6);
        TC3X_API_REG_Write (moduleidx, i, 0x11, value);
    
        // set output value
        value = TC3X_API_REG_Read (moduleidx, i, 0x13);
        if (freq_khz > 400000)
	    BITSET (value, Hw6);	// UHF - high
        else
	    BITCLR (value, Hw6);	// VHF - low
        TC3X_API_REG_Write (moduleidx, i, 0x13, value);
    }
#endif // USE_BAND_SWITCH_GPIOC_06

    return (TC3X_SubAPI_SetFrequency (moduleidx, freq_khz, bw_khz, pOption));
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_RF_REG_R
//    Description : Read RF Register 
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      addr : Address
//    Output : 
//      read value
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_RF_REG_R (int moduleidx, unsigned int DeviceIdx, unsigned int addr)
{
    int       ret = 0;

    if (!g_pSTDCtrl)
        return 0;
    if (!g_pSTDCtrl[moduleidx])
        return 0;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return 0;

    ret =
        g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB,
                                                          CMD_RF_READ_REG_SINGLE, &addr, NULL, NULL, NULL);
    return ret;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_RF_REG_W
//    Description : Write to RF
//    Input : 
//      moduleidx : module index
//      moduleidx : module index
//      DeviceIdx : device index
//      addr : Address
//      data : Data
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_RF_REG_W (int moduleidx, unsigned int DeviceIdx, unsigned int addr, unsigned int data)
{
    if (!g_pSTDCtrl)
        return;
    if (!g_pSTDCtrl[moduleidx])
        return;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return;

    g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB,
                                                      CMD_RF_WRITE_REG_SINGLE, &addr, &data, NULL, NULL);
}

// stream pause
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_StreamPause
//    Description : Stream Pause
//    Input : 
//      moduleidx : module index
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_StreamPause (int moduleidx)
{
    if (!g_pSTDCtrl)
        return;
    if (!g_pSTDCtrl[moduleidx])
        return;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return;

    g_pSTDCtrl[moduleidx][0].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][0].hBB, CMD_BB_STREAMOUT_PAUSE, NULL,
                                              NULL, NULL, NULL);
}

// stream restart
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_StreamRestart
//    Description : Stream Restart
//    Input : 
//      moduleidx : module index
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_StreamRestart (int moduleidx)
{
    if (!g_pSTDCtrl)
        return;
    if (!g_pSTDCtrl[moduleidx])
        return;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return;

    g_pSTDCtrl[moduleidx][0].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][0].hBB, CMD_BB_STREAMOUT_RESTART, NULL,
                                              NULL, NULL, NULL);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetBootCodeVersion
//    Description : Get Boot Code (ASM) Version
//    Input : 
//      moduleidx : module index
//    
//    Output : 
//      version data
//    Remark : 
//--------------------------------------------------------------------------
unsigned int TC3X_API_GetBootCodeVersion (int moduleidx, unsigned int *pVersion)
{
    unsigned int ret = 0;

    if (!g_pSTDCtrl)
        return 0x00;
    if (!g_pSTDCtrl[moduleidx])
        return 0x00;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return 0x00;

    pVersion[0] = 0x00;
    ret =
        g_pSTDCtrl[moduleidx][0].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][0].hBB, CMD_BB_GET_BOOTCODE_VERSION,
                                                  pVersion, NULL, NULL, NULL);
    return ret;
}

// Stream Will be Come Into This Function

#if defined (STDDEF_DVBT) || defined (STDDEF_DVBH)
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_DVBT_OccurEvent
//    Description : Event fuction (call fuction)
//    Input : 
//      moduleidx : module index
//      handle : device index
//      message
//      param1, param2, param3, param4 : parameters
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_DVBT_OccurEvent (int moduleidx, TC3X_DNUM handle, int message, void *param1, void *param2, void *param3,
                               void *param4)
{
    unsigned char *src;
    unsigned int StreamSize;

    switch (message)
    {
    case CMD_STREAM_TS:
        src = (unsigned char *) param1;
        StreamSize = *(unsigned int *) param2;
        break;
    case CMD_STREAM_STOPPED:
        break;
    case EVENT_QUIET_DATALINE:
        break;
    case TC3X_ERROR_UNKNOWN:
        PRINTF_LV_0 ("# CRITICAL ERROR [UNKNOWN] \n");
        PRINTF_LV_0 ("# Please close TC3X Driver \n");
        break;
    case TC3X_ERROR_CRITICAL_MAILBOX:
        PRINTF_LV_0 ("# CRITICAL ERROR [MAILBOX] \n");
        PRINTF_LV_0 ("# Please close TC3X Driver \n");
        break;
    }
}

void TC3X_API_Enable_PIDFiltering (int moduleidx)
{
    if (!g_pSTDCtrl)
        return;
    if (!g_pSTDCtrl[moduleidx])
        return;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return;

    g_pSTDCtrl[moduleidx][0].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][0].hBB, CMD_BB_EN_PID, NULL, NULL, NULL,
                                              NULL);
}

void TC3X_API_Disable_PIDFiltering (int moduleidx)
{
    if (!g_pSTDCtrl)
        return;
    if (!g_pSTDCtrl[moduleidx])
        return;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return;

    g_pSTDCtrl[moduleidx][0].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][0].hBB, CMD_BB_DIS_PID, NULL, NULL, NULL,
                                              NULL);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_REG_PID
//    Description : Set PID Filter
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_REG_PID (int moduleidx, int pid)
{
    if (!g_pSTDCtrl)
        return;
    if (!g_pSTDCtrl[moduleidx])
        return;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return;

    g_pSTDCtrl[moduleidx][0].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][0].hBB, CMD_BB_PID_REG, &pid, NULL, NULL,
                                              NULL);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_UnREG_PID
//    Description : 
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_UnREG_PID (int moduleidx, int pid)
{
    if (!g_pSTDCtrl)
        return;
    if (!g_pSTDCtrl[moduleidx])
        return;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return;

    g_pSTDCtrl[moduleidx][0].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][0].hBB, CMD_BB_PID_UNREG, &pid, NULL,
                                              NULL, NULL);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetDVB_Lock
//    Description : Get Lock Status
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      pLock : Lock Status
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetDVB_Lock (int moduleidx, unsigned int DeviceIdx, t_DVBLock * pLock)
{
    int       ret = 0;

    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    ret =
        g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB,
                                                          CMD_GET_DVB_LOCK, pLock, NULL, NULL, NULL);
    return ret;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetDVB_Lock_Wait
//    Description : Get Lock Status until Thr Time.
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//    Output : 
//      1 : success, others : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetDVB_Lock_Wait (int moduleidx, unsigned int DeviceIdx)
{
    int       i, j;
    t_DVBLock Lock;
    t_DVBLock Lock_Sub;
    int       Tickms;
    int       TickCnt;
    int       SubLockOK;
#if defined(USE_ANDROID) || defined(USE_LINUX)
    long long starttime;
    long long timeout;
#else
    unsigned int starttime;
    unsigned int timeout;
#endif
    //-------------------------------------------
    // 0. AGC Lock : wait
    // 1. CTO & CFO Detect : if not fail :(DVB_CTO_LOCK * DVB_CTO_RETRY) + ((DVB_CTO_LOCK+DVB_CFO_LOCK) * DVB_CFO_RETRY) ms;
    // 2. TPS Detect : if not fail : (DVB_TPS_LOCK + (DVB_TPS_LOCK_FAIL* DVB_TPS_RETRY)) ms;
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    Tickms = 10;

    // CTO & CFO Detect
    SubLockOK = 0;
    TickCnt =
        ((DVB_CTO_LOCK * DVB_CTO_RETRY) + ((DVB_CTO_LOCK + DVB_CFO_LOCK) * DVB_CFO_RETRY) + DVB_AGC_LOCK) / Tickms;

    timeout = ((DVB_CTO_LOCK * DVB_CTO_RETRY) + ((DVB_CTO_LOCK + DVB_CFO_LOCK) * DVB_CFO_RETRY) + DVB_AGC_LOCK) * 1000;
    starttime = TC3X_IO_Util_GetTickCnt ();

    for (i = 0; i < TickCnt; i++)
    {
        TC3X_IO_memset (&Lock, 0x00, sizeof (t_DVBLock));
        for (j = 0; j < g_tSTDInfo[moduleidx].NumOfDemodule; j++)
        {
            TC3X_API_GetDVB_Lock (moduleidx, j, &Lock_Sub);
            Lock.AGC |= Lock_Sub.AGC;
            Lock.DCEC |= Lock_Sub.DCEC;
            Lock.CTO |= Lock_Sub.CTO;
            Lock.CFO |= Lock_Sub.CFO;
            Lock.SPISCH |= Lock_Sub.SPISCH;
            Lock.TPS |= Lock_Sub.TPS;
            Lock.TSPER_UNDER_THR |= Lock_Sub.TSPER_UNDER_THR;
            Lock.INV_SYNC_BYTE |= Lock_Sub.INV_SYNC_BYTE;
        }

        if (Lock.CTO && Lock.CFO && Lock.TPS && Lock.TSPER_UNDER_THR && Lock.INV_SYNC_BYTE)
        {
            return 1;   // Lock Success
        }

        if (Lock.CTO && Lock.CFO)
        {
            SubLockOK = 1;
            break;
        }
        else
        {
            if (TC3X_IO_Util_GetInterval (starttime) > timeout)
            {
                return 0;
            }
        }
        TC3X_IO_Sleep (Tickms);
    }

    if (SubLockOK == 0) // Lock Fail
        return 0;

    // TPS Detect
    SubLockOK = 0;
    TickCnt = (DVB_TPS_LOCK + (DVB_TPS_LOCK_FAIL * DVB_TPS_RETRY)) / Tickms;

    timeout = (DVB_TPS_LOCK + (DVB_TPS_LOCK_FAIL * DVB_TPS_RETRY)) * 1000;
    starttime = TC3X_IO_Util_GetTickCnt ();

    for (i = 0; i < TickCnt; i++)
    {
        TC3X_IO_memset (&Lock, 0x00, sizeof (t_DVBLock));
        for (j = 0; j < g_tSTDInfo[moduleidx].NumOfDemodule; j++)
        {
            TC3X_API_GetDVB_Lock (moduleidx, j, &Lock_Sub);
            Lock.AGC |= Lock_Sub.AGC;
            Lock.DCEC |= Lock_Sub.DCEC;
            Lock.CTO |= Lock_Sub.CTO;
            Lock.CFO |= Lock_Sub.CFO;
            Lock.SPISCH |= Lock_Sub.SPISCH;
            Lock.TPS |= Lock_Sub.TPS;
            Lock.TSPER_UNDER_THR |= Lock_Sub.TSPER_UNDER_THR;
            Lock.INV_SYNC_BYTE |= Lock_Sub.INV_SYNC_BYTE;
        }

        if (Lock.CTO && Lock.CFO && Lock.TPS && Lock.TSPER_UNDER_THR && Lock.INV_SYNC_BYTE)
        {
            SubLockOK = 1;
            return 1;   // Lock Success
        }

        if (Lock.CTO && Lock.CFO && Lock.TPS)
        {
            SubLockOK = 1;
            break;
        }
        else
        {
            if (TC3X_IO_Util_GetInterval (starttime) > timeout)
            {
                return 0;
            }
        }
        TC3X_IO_Sleep (Tickms);
    }

    if (SubLockOK == 0) // Lock Fail
        return 0;

    // SyncByte (InverseSyncByte) Detect
    SubLockOK = 0;
    TickCnt = (DVB_SYNC_INV_LOCK + (DVB_SYNC_TSPER_UNDER_THRP_LOCK * DVB_SYNC_TSPER_UNDER_THRP_RETRY)) / Tickms;

    timeout = (DVB_SYNC_INV_LOCK + (DVB_SYNC_TSPER_UNDER_THRP_LOCK * DVB_SYNC_TSPER_UNDER_THRP_RETRY)) * 1000;
    starttime = TC3X_IO_Util_GetTickCnt ();

    for (i = 0; i < TickCnt; i++)
    {
        TC3X_IO_memset (&Lock, 0x00, sizeof (t_DVBLock));
        for (j = 0; j < g_tSTDInfo[moduleidx].NumOfDemodule; j++)
        {
            TC3X_API_GetDVB_Lock (moduleidx, j, &Lock_Sub);
            Lock.AGC |= Lock_Sub.AGC;
            Lock.DCEC |= Lock_Sub.DCEC;
            Lock.CTO |= Lock_Sub.CTO;
            Lock.CFO |= Lock_Sub.CFO;
            Lock.SPISCH |= Lock_Sub.SPISCH;
            Lock.TPS |= Lock_Sub.TPS;
            Lock.TSPER_UNDER_THR |= Lock_Sub.TSPER_UNDER_THR;
            Lock.INV_SYNC_BYTE |= Lock_Sub.INV_SYNC_BYTE;
        }

        if (Lock.CTO && Lock.CFO && Lock.TPS && Lock.TSPER_UNDER_THR && Lock.INV_SYNC_BYTE)
        {
            SubLockOK = 1;
            PRINTF_LV_2 ("[TCC35XX] SYNC LOCK!!! \n");
            return 1;   // Lock Success
        }
        else
        {
            if (TC3X_IO_Util_GetInterval (starttime) > timeout)
            {
                return 0;
            }
        }
        TC3X_IO_Sleep (Tickms);
    }

    return 0;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetDVB_SNR
//    Description : Get snr
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      psnr : snr
//      pSNRdb : snr db
//      pMovingAVGSNRdb : Average snr db
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetDVB_SNR (int moduleidx, unsigned int DeviceIdx, double *pSNR, double *pSNRdb, double *pMovingAVGSNRdb)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    return (g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction
            (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB, CMD_GET_DVB_SNR_SET, pSNR, pSNRdb, pMovingAVGSNRdb,
             NULL));
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_UpdateCMD_DVB_BERSET
//    Description : update command ber set
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_UpdateCMD_DVB_BERSET (int moduleidx, unsigned int DeviceIdx)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    return (g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction
            (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB, CMD_UPDATECMD_DVB_BERSET, NULL, NULL, NULL, NULL));
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetDVB_PCBER
//    Description : Get pcber
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      pPCBER : pcber
//      pMovingAVGPCBER : Average pcber
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetDVB_PCBER (int moduleidx, unsigned int DeviceIdx, double *pPCBER, double *pMovingAVGPCBER)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    return (g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction
            (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB, CMD_GET_DVB_PCBER_SET, pPCBER, pMovingAVGPCBER, NULL,
             NULL));
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetDVB_VITERBIBER
//    Description : Get VITERBI Ber
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      pVITERBIBER : Viterbi ber
//      pMovingAVGVITERBIBER : Average Viterbi ber
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetDVB_VITERBIBER (int moduleidx, unsigned int DeviceIdx, double *pVITERBIBER,
                                double *pMovingAVGVITERBIBER)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    return (g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction
            (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB, CMD_GET_DVB_VITERBIBER_SET, pVITERBIBER,
             pMovingAVGVITERBIBER, NULL, NULL));
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetDVB_TSPER
//    Description : Get TSPER
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      pTSPER : TSPER
//      pMovingAVGTSPER : Average TSPER
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetDVB_TSPER (int moduleidx, unsigned int DeviceIdx, double *pTSPER, double *pMovingAVGTSPER)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    return (g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction
            (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB, CMD_GET_DVB_TSPER_SET, pTSPER, pMovingAVGTSPER, NULL,
             NULL));
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetDVB_RSSI
//    Description : Get RSSI
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      pRSSI : RSSI
//      pMovingAVGRSSI : Average RSSI
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetDVB_RSSI (int moduleidx, unsigned int DeviceIdx, double *pRSSI, double *pMovingAVGRSSI)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    return (g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction
            (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB, CMD_GET_DVB_RSSI_SET, pRSSI, pMovingAVGRSSI, NULL, NULL));
}
#endif // STDDEF_DVBT

#if defined (STDDEF_DVBH)
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_DVBH_OccurEvent
//    Description : Event fuction (call fuction)
//    Input : 
//      moduleidx : module index
//      handle : device index
//      message
//      param1, param2, param3, param4 : parameters
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_DVBH_OccurEvent (int moduleidx, TC3X_DNUM handle, int message, void *param1, void *param2, void *param3,
                               void *param4)
{
    unsigned char *src;
    unsigned int StreamSize, PID;
    //unsigned char TempDVBHIP[191*1024+8];

    switch (message)
    {
    case CMD_STREAM_TS:
        src = (unsigned char *) param1;
        StreamSize = *(unsigned int *) param2;
        break;

    case CMD_STREAM_IP:
        src = (unsigned char *) param1;
        StreamSize = *(unsigned int *) param2;
        PID = *(unsigned int *) param3;
        break;

    case CMD_STREAM_STOPPED:
        break;
    case EVENT_QUIET_DATALINE:
        break;
    case TC3X_ERROR_UNKNOWN:
        PRINTF_LV_0 ("# CRITICAL ERROR [UNKNOWN] \n");
        PRINTF_LV_0 ("# Please close TC3X Driver \n");
        break;
    case TC3X_ERROR_CRITICAL_MAILBOX:
        PRINTF_LV_0 ("# CRITICAL ERROR [MAILBOX] \n");
        PRINTF_LV_0 ("# Please close TC3X Driver \n");
        break;
    }
}

//-----------------------------------------------------------------------------------------------
// DVB-H
//
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_SetMPEFECParam
//    Description : Set MPEFEC Informations
//    Input : 
//      moduleidx : module index
//      pMPEFEC_info : mpefec informations
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_SetMPEFECParam (int moduleidx, tMPEFEC_INFO * pMPEFEC_info)
{
    if (!g_pSTDCtrl)
        return;
    if (!g_pSTDCtrl[moduleidx])
        return;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return;

    g_pSTDCtrl[moduleidx][0].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][0].hBB, CMD_BB_SETMPEFECPARAM,
                                              pMPEFEC_info, NULL, NULL, NULL);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_SET_MPEFEC_STAT_CLR
//    Description : clear mpefec status
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_SET_MPEFEC_STAT_CLR (int moduleidx)
{
    if (!g_pSTDCtrl)
        return;
    if (!g_pSTDCtrl[moduleidx])
        return;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return;

    g_pSTDCtrl[moduleidx][0].BB_UserFunction (moduleidx, 0, SET_MPEFEC_STAT_CLR, 0, 0, 0, 0);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GET_MPEFEC_STAT
//    Description : Get mpefec status
//    Input : 
//      moduleidx : module index
//      psucc : success
//      pfatil : failure
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_GET_MPEFEC_STAT (int moduleidx, unsigned int *psucc, unsigned int *pfail)
{
    if (!g_pSTDCtrl)
        return;
    if (!g_pSTDCtrl[moduleidx])
        return;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return;

    g_pSTDCtrl[moduleidx][0].BB_UserFunction (moduleidx, 0, GET_MPEFEC_STAT, psucc, pfail, 0, 0);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GET_MPEFEC_CR
//    Description : get mpefec cr
//    Input : 
//      moduleidx : module index
//      pover : over
//      pUnder : under
//      pMPECOL : column of mpe
//      pFECCol : column of fec
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_GET_MPEFEC_CR (int moduleidx, unsigned int *pOver, unsigned int *pUnder, unsigned int *pMPECOL,
                             unsigned int *pFECCol)
{
    if (!g_pSTDCtrl)
        return;
    if (!g_pSTDCtrl[moduleidx])
        return;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return;

    g_pSTDCtrl[moduleidx][0].BB_UserFunction (moduleidx, 0, GET_MPEFEC_CRSTAT, pOver, pUnder, pMPECOL, pFECCol);
}

#endif // STDDEF_DVBH

#if defined (STDDEF_CMMB)
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_CMMB_OccurEvent
//    Description : Event fuction (call fuction)
//    Input : 
//      moduleidx : module index
//      handle : device index
//      message
//      param1, param2, param3, param4 : parameters
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_CMMB_OccurEvent (int moduleidx, TC3X_DNUM handle, int message, void *param1, void *param2, void *param3,
                               void *param4)
{
    unsigned char *src;
    unsigned int StreamSize, MFID, TotalSize;

    switch (message)
    {
    case CMD_STREAM_CMMB:
        src = (unsigned char *) param1;
        StreamSize = *(unsigned int *) param2;
        MFID = *(unsigned int *) param3;
        break;

    case CMD_STREAM_CMMB_PIECE:
    case CMD_STREAM_CMMB_PIECE_START:
    case CMD_STREAM_CMMB_PIECE_END:
        src = (unsigned char *) param1;
        StreamSize = *(unsigned int *) param2;
        MFID = *(unsigned int *) param3;
        TotalSize = *(unsigned int *) param4;
        break;

    case CMD_STREAM_STOPPED:
        break;
    case TC3X_ERROR_UNKNOWN:
        PRINTF_LV_0 ("# CRITICAL ERROR [UNKNOWN] \n");
        PRINTF_LV_0 ("# Please close TC3X Driver \n");
        break;
    case TC3X_ERROR_CRITICAL_MAILBOX:
        PRINTF_LV_0 ("# CRITICAL ERROR [MAILBOX] \n");
        PRINTF_LV_0 ("# Please close TC3X Driver \n");
        break;
    }
}

//-----------------------------------------------------------------------------------------------
// CMMB
//
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_CMMBService_InfoSet
//    Description : push CMMB Service informations
//    Input : 
//      moduleidx : module index
//      pCMMBServiceInfo : CMMB Service informations
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_CMMBService_InfoSet (int moduleidx, tCMMBServicesInfo * pCMMBServiceInfo)
{
    if (!g_pSTDCtrl)
        return;
    if (!g_pSTDCtrl[moduleidx])
        return;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return;

    g_pSTDCtrl[moduleidx][0].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][0].hBB, CMD_BB_SET_CMMB_SERVICE_INFO,
                                              pCMMBServiceInfo, NULL, NULL, NULL);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_CMMBService_REG
//    Description : Register CMMB Service
//    Input : 
//      moduleidx : module index
//      MFID : MFID
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_CMMBService_REG (int moduleidx, unsigned int MFID)
{
    if (!g_pSTDCtrl)
        return;
    if (!g_pSTDCtrl[moduleidx])
        return;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return;

    g_pSTDCtrl[moduleidx][0].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][0].hBB, CMD_BB_CMMB_REG, &MFID, NULL,
                                              NULL, NULL);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_CMMBService_UnREG
//    Description : Unregister CMMB Service
//    Input : 
//      moduleidx : module index
//      MFID : MFID
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_CMMBService_UnREG (int moduleidx, unsigned int MFID)
{
    if (!g_pSTDCtrl)
        return;
    if (!g_pSTDCtrl[moduleidx])
        return;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return;

    g_pSTDCtrl[moduleidx][0].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][0].hBB, CMD_BB_CMMB_UNREG, &MFID, NULL,
                                              NULL, NULL);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetCMMB_Lock
//    Description : Get Lock Status
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      pLock : Lock Status
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetCMMB_Lock (int moduleidx, unsigned int DeviceIdx, t_CMMBLock * pLock)
{
    int       ret = 0;

    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    ret =
        g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB,
                                                          CMD_GET_CMMB_LOCK, pLock, NULL, NULL, NULL);
    return ret;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetCMMB_Lock_Wait
//    Description : Get Lock Status until Thr Time.
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//    Output : 
//      1 : success, others : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetCMMB_Lock_Wait (int moduleidx, unsigned int DeviceIdx)
{
    int       i;
    t_CMMBLock Lock;
    int       Tickms;
    int       TickCnt;
    int       SubLockOK;
#if defined(USE_ANDROID) || defined(USE_LINUX)
    long long starttime;
    long long timeout;
#else
    unsigned int starttime;
    unsigned int timeout;
#endif

    //-------------------------------------------
    // 0. AGC Lock : wait
    // 1. CTO & CFO Detect : if not fail :(CMMB_CTO_BEACON_LOCK * CMMB_CTO_BEACON_RETRY) + ((CMMB_CTO_BEACON_LOCK+CMMB_CFO_LOCK) * CMMB_CFO_RETRY) ms;
    // 2. CLCH Detect : if not fail : (CMMB_CLCH_LOCK * CMMB_CLCH_RETRY) ms;

    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    Tickms = 10;

    // CTO & CFO Detect
    SubLockOK = 0;
    TickCnt =
        ((CMMB_CTO_BEACON_LOCK * CMMB_CTO_BEACON_RETRY) + ((CMMB_CTO_BEACON_LOCK + CMMB_CFO_LOCK) * CMMB_CFO_RETRY) +
         CMMB_AGC_LOCK) / Tickms;

    timeout =
        ((CMMB_CTO_BEACON_LOCK * CMMB_CTO_BEACON_RETRY) + ((CMMB_CTO_BEACON_LOCK + CMMB_CFO_LOCK) * CMMB_CFO_RETRY) +
         CMMB_AGC_LOCK) * 1000;
    starttime = TC3X_IO_Util_GetTickCnt ();

    for (i = 0; i < TickCnt; i++)
    {
        TC3X_API_GetCMMB_Lock (moduleidx, DeviceIdx, &Lock);

        if (Lock.CTO && Lock.CFO && Lock.FTO && Lock.CLCH)
        {
            return 1;   // Lock Success
        }

        if (Lock.CTO && Lock.CFO && Lock.FTO)
        {
            SubLockOK = 1;
            break;
        }
        else
        {
            if (TC3X_IO_Util_GetInterval (starttime) > timeout)
            {
                return 0;
            }
        }
        TC3X_IO_Sleep (Tickms);
    }

    if (SubLockOK == 0) // Lock Fail
        return 0;

    // CLCH Detect
    SubLockOK = 0;
    TickCnt = (CMMB_CLCH_LOCK * CMMB_CLCH_RETRY) / Tickms;

    timeout = (CMMB_CLCH_LOCK * CMMB_CLCH_RETRY) * 1000;
    starttime = TC3X_IO_Util_GetTickCnt ();

    for (i = 0; i < TickCnt; i++)
    {
        TC3X_API_GetCMMB_Lock (moduleidx, DeviceIdx, &Lock);
        if (Lock.CTO && Lock.CFO && Lock.FTO && Lock.CLCH)
        {
            SubLockOK = 1;
            return 1;   // Lock Success
        }
        else
        {
            if (TC3X_IO_Util_GetInterval (starttime) > timeout)
            {
                return 0;
            }
        }
        TC3X_IO_Sleep (Tickms);
    }

    return 0;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetCMMB_MER
//    Description : Get MER
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      pMER : MER
//      pMovingAVGMER : Average MER
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetCMMB_MER (int moduleidx, unsigned int DeviceIdx, double *pMER, double *pMovingAVGMER)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    return (g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction
            (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB, CMD_GET_CMMB_MER_SET, pMER, pMovingAVGMER, NULL, NULL));
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_UpdateCMD_CMMB_BERSET
//    Description : update command ber set
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_UpdateCMD_CMMB_BERSET (int moduleidx, unsigned int DeviceIdx)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    return (g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction
            (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB, CMD_UPDATECMD_CMMB_BERSET, NULL, NULL, NULL, NULL));
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetCMMB_LDPC_BLER
//    Description : Get LDPC_BLER
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      pLDPC_BLER : LDPC_BLER
//      pMovingAVGLDPC_BLER : Average LDPC_BLER
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetCMMB_LDPC_BLER (int moduleidx, unsigned int DeviceIdx, double *pLDPC_BLER, double *pMovingAVGLDPC_BLER)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    return (g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction
            (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB, CMD_GET_CMMB_LDPC_BLER_SET, pLDPC_BLER,
             pMovingAVGLDPC_BLER, NULL, NULL));
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetCMMB_VITERBIBER
//    Description : Get VITERBI Ber
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      pVITERBIBER : Viterbi ber
//      pMovingAVGVITERBIBER : Average Viterbi ber
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetCMMB_VITERBIBER (int moduleidx, unsigned int DeviceIdx, double *pVITERBIBER,
                                 double *pMovingAVGVITERBIBER)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    return (g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction
            (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB, CMD_GET_CMMB_VITERBIBER_SET, pVITERBIBER,
             pMovingAVGVITERBIBER, NULL, NULL));
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetCMMB_FER
//    Description : Get FER
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      pFER : FER
//      pMovingAVGFER : Average FER
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetCMMB_FER (int moduleidx, unsigned int DeviceIdx, double *pFER, double *pMovingAVGFER)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    return (g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction
            (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB, CMD_GET_CMMB_FER_SET, pFER, pMovingAVGFER, NULL, NULL));
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetCMMB_RSSI
//    Description : Get RSSI
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      pRSSI : RSSI
//      pMovingAVGRSSI : Average RSSI
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetCMMB_RSSI (int moduleidx, unsigned int DeviceIdx, double *pRSSI, double *pMovingAVGRSSI)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    return (g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction
            (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB, CMD_GET_CMMB_RSSI_SET, pRSSI, pMovingAVGRSSI, NULL,
             NULL));
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_CMMB_AskSize
//    Description : 
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      mfid : MFID
//      pSize : CMMB MF Size    (output)
//    Output : 
//      -1 or 0 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_CMMB_AskSize (int moduleidx, unsigned int DeviceIdx, unsigned int mfid, unsigned int *pSize)
{
    int       ret;

    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    ret =
        g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB,
                                                          CMD_REQ_CMMBSIZE, pSize, NULL, NULL, NULL);
    return ret;
}
#endif // STDDEF_CMMB

#if defined (STDDEF_ISDBT1SEG)
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_ISDBT1SEG_OccurEvent
//    Description : Event fuction (call fuction)
//    Input : 
//      moduleidx : module index
//      handle : device index
//      message
//      param1, param2, param3, param4 : parameters
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_ISDBT1SEG_OccurEvent (int moduleidx, TC3X_DNUM handle, int message, void *param1, void *param2,
                                    void *param3, void *param4)
{
    unsigned char *src;
    unsigned int StreamSize;

    switch (message)
    {
    case CMD_STREAM_ISDBT1SEG:
        src = (unsigned char *) param1;
        StreamSize = *(unsigned int *) param2;
        break;
    case CMD_STREAM_STOPPED:
        break;
    case EVENT_QUIET_DATALINE:
        break;
    case TC3X_ERROR_UNKNOWN:
        PRINTF_LV_0 ("# CRITICAL ERROR [UNKNOWN] \n");
        PRINTF_LV_0 ("# Please close TC3X Driver \n");
        break;
    case TC3X_ERROR_CRITICAL_MAILBOX:
        PRINTF_LV_0 ("# CRITICAL ERROR [MAILBOX] \n");
        PRINTF_LV_0 ("# Please close TC3X Driver \n");
        break;
    }
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetISDBT_Lock
//    Description : Get Lock Status
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      pLock : Lock Status
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetISDBT_Lock (int moduleidx, unsigned int DeviceIdx, t_ISDBTLock * pLock)
{
    int       ret = 0;

    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    ret =
        g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB,
                                                          CMD_GET_ISDBT_LOCK, pLock, NULL, NULL, NULL);
    return ret;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetISDBT_Lock_Wait
//    Description : Get Lock Status until Thr Time.
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//    Output : 
//      1 : success, others : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetISDBT_Lock_Wait (int moduleidx, unsigned int DeviceIdx)
{
    int       i;
    t_ISDBTLock Lock;
    int       Tickms;
    int       TickCnt;
    int       SubLockOK;
#if defined(USE_ANDROID) || defined(USE_LINUX)
    long long starttime;
    long long timeout;
#else
    unsigned int starttime;
    unsigned int timeout;
#endif

    //-------------------------------------------
    // 0. AGC Lock : wait
    // 1. CTO & CFO Detect : if not fail :(ISDBT_CTO_LOCK * ISDBT_CTO_RETRY) + ((ISDBT_CTO_LOCK+ISDBT_CFO_LOCK) * ISDBT_CFO_RETRY) ms;
    // 2. TMCC Detect : if not fail : (ISDBT_TMCC_LOCK + (ISDBT_TMCC_LOCK_FAIL * ISDBT_TMCC_RETRY)) ms;

    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    Tickms = 10;

    // CTO & CFO Detect
    SubLockOK = 0;
    TickCnt =
        ((ISDBT_CTO_LOCK * ISDBT_CTO_RETRY) + ((ISDBT_CTO_LOCK + ISDBT_CFO_LOCK) * ISDBT_CFO_RETRY) +
         ISDBT_AGC_LOCK) / Tickms;

    timeout = ((ISDBT_CTO_LOCK * ISDBT_CTO_RETRY) + ((ISDBT_CTO_LOCK + ISDBT_CFO_LOCK) * ISDBT_CFO_RETRY) +
               ISDBT_AGC_LOCK) * 1000;
    starttime = TC3X_IO_Util_GetTickCnt ();

    for (i = 0; i < TickCnt; i++)
    {
        TC3X_API_GetISDBT_Lock (moduleidx, DeviceIdx, &Lock);

        if (Lock.CTO && Lock.CFO && Lock.TMCC)
        {
            return 1;   // Lock Success
        }

        if (Lock.CTO && Lock.CFO)
        {
            SubLockOK = 1;
            break;
        }
        else
        {
            if (TC3X_IO_Util_GetInterval (starttime) > timeout)
            {
                return 0;
            }
        }
        TC3X_IO_Sleep (Tickms);
    }

    if (SubLockOK == 0) // Lock Fail
        return 0;

    // TMCC Detect
    SubLockOK = 0;
    TickCnt = (ISDBT_TMCC_LOCK + (ISDBT_TMCC_LOCK_FAIL * ISDBT_TMCC_RETRY)) / Tickms;

    timeout = (ISDBT_TMCC_LOCK + (ISDBT_TMCC_LOCK_FAIL * ISDBT_TMCC_RETRY)) * 1000;
    starttime = TC3X_IO_Util_GetTickCnt ();

    for (i = 0; i < TickCnt; i++)
    {
        TC3X_API_GetISDBT_Lock (moduleidx, DeviceIdx, &Lock);
        if (Lock.CTO && Lock.CFO && Lock.TMCC)
        {
            SubLockOK = 1;
            return 1;   // Lock Success
        }
        else
        {
            if (TC3X_IO_Util_GetInterval (starttime) > timeout)
            {
                return 0;
            }
        }
        TC3X_IO_Sleep (Tickms);
    }

    return 0;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetISDBT_MER
//    Description : Get MER
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      pMER : MER
//      pMovingAVGMER : Average MER
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetISDBT_MER (int moduleidx, unsigned int DeviceIdx, double *pMER, double *pMovingAVGMER)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    return (g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction
            (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB, CMD_GET_ISDBT_MER_SET, pMER, pMovingAVGMER, NULL, NULL));
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_UpdateCMD_ISDBT_BERSET
//    Description : update command ber set
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_UpdateCMD_ISDBT_BERSET (int moduleidx, unsigned int DeviceIdx)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    return (g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction
            (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB, CMD_UPDATECMD_ISDBT_BERSET, NULL, NULL, NULL, NULL));
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetISDBT_PCBER
//    Description : Get pcber
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      pPCBER : pcber
//      pMovingAVGPCBER : Average pcber
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetISDBT_PCBER (int moduleidx, unsigned int DeviceIdx, double *pPCBER, double *pMovingAVGPCBER)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    return (g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction
            (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB, CMD_GET_ISDBT_PCBER_SET, pPCBER, pMovingAVGPCBER, NULL,
             NULL));
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetISDBT_VITERBIBER
//    Description : Get VITERBI Ber
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      pVITERBIBER : Viterbi ber
//      pMovingAVGVITERBIBER : Average Viterbi ber
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetISDBT_VITERBIBER (int moduleidx, unsigned int DeviceIdx, double *pVITERBIBER,
                                  double *pMovingAVGVITERBIBER)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    return (g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction
            (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB, CMD_GET_ISDBT_VITERBIBER_SET, pVITERBIBER,
             pMovingAVGVITERBIBER, NULL, NULL));
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetISDBT_TSPER
//    Description : Get TSPER
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      pTSPER : TSPER
//      pMovingAVGTSPER : Average TSPER
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetISDBT_TSPER (int moduleidx, unsigned int DeviceIdx, double *pTSPER, double *pMovingAVGTSPER)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    return (g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction
            (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB, CMD_GET_ISDBT_TSPER_SET, pTSPER, pMovingAVGTSPER, NULL,
             NULL));
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetISDBT_RSSI
//    Description : Get RSSI
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      pRSSI : RSSI
//      pMovingAVGRSSI : Average RSSI
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetISDBT_RSSI (int moduleidx, unsigned int DeviceIdx, double *pRSSI, double *pMovingAVGRSSI)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    return (g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction
            (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB, CMD_GET_ISDBT_RSSI_SET, pRSSI, pMovingAVGRSSI, NULL,
             NULL));
}
#endif // STDDEF_ISDBT1SEG

#if defined (STDDEF_ISDBT13SEG)
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_ISDBT13SEG_OccurEvent
//    Description : Event fuction (call fuction)
//    Input : 
//      moduleidx : module index
//      handle : device index
//      message
//      param1, param2, param3, param4 : parameters
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_ISDBT13SEG_OccurEvent (int moduleidx, TC3X_DNUM handle, int message, void *param1, void *param2,
                                     void *param3, void *param4)
{
    unsigned char *src;
    unsigned int StreamSize;
    //unsigned char TempDVBHIP[191*1024+8];

    switch (message)
    {
    case CMD_STREAM_ISDBT13SEG:
        src = (unsigned char *) param1;
        StreamSize = *(unsigned int *) param2;
        break;
    case CMD_STREAM_STOPPED:
        break;
    case EVENT_QUIET_DATALINE:
        break;
    case TC3X_ERROR_UNKNOWN:
        PRINTF_LV_0 ("# CRITICAL ERROR [UNKNOWN] \n");
        PRINTF_LV_0 ("# Please close TC3X Driver \n");
        break;
    case TC3X_ERROR_CRITICAL_MAILBOX:
        PRINTF_LV_0 ("# CRITICAL ERROR [MAILBOX] \n");
        PRINTF_LV_0 ("# Please close TC3X Driver \n");
        break;
    }
}
#endif // STDDEF_ISDBT13SEG

#if defined (STDDEF_DMB)

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_TDMB_OccurEvent
//    Description : Event fuction (call fuction)
//    Input : 
//      moduleidx : module index
//      handle : device index
//      message
//      param1, param2, param3, param4 : parameters
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_API_TDMB_OccurEvent (int moduleidx, TC3X_DNUM handle, int message, void *param1, void *param2, void *param3,
                               void *param4)
{
    switch (message)
    {
    case CMD_STREAM_TDMB:
        {
            unsigned char *src;
            unsigned int StreamSize;
            int       Temp;
            int       Type;
            int       SubChID;
            int       ServiceMailBoxIdx;
            unsigned int CRCErrorValue;

            src = (unsigned char *) param1;
            StreamSize = *(unsigned int *) param2;
            Temp = *(int *) param3;
            Type = ((Temp >> 8) & 0xFF);
            SubChID = (Temp & 0xFF);
            ServiceMailBoxIdx = *(int *) param4;
            CRCErrorValue = *(unsigned int *) param4;
            switch (Type)
            {
            case SRVTYPE_FIC:
            case SRVTYPE_FIC_WITH_ERR:
            case EWS_ANNOUNCE_FLAG:
            case RECONFIGURATION_FLAG:
            case EWS_ANNOUNCE_RECONFIGURATION_FLAG:
                TCC351XTDMB_FICProcess (moduleidx, src, StreamSize, CRCErrorValue, Type);
                break;
            case SRVTYPE_DMB:
                TCC351XTDMB_AVServiceProcess (moduleidx, src, StreamSize, Type);
                break;

            case SRVTYPE_DAB:
            case SRVTYPE_DABPLUS:
                TCC351XTDMB_AVServiceProcess (moduleidx, src, StreamSize, Type);
                break;
            case SRVTYPE_DATA:
                TCC351XTDMB_DataServiceProcess(moduleidx, src, StreamSize, Type);
                break;
            default:
                break;
            }
        }
        break;

    case CMD_STREAM_STOPPED:
        break;
    case TC3X_ERROR_UNKNOWN:
        PRINTF_LV_0 ("# CRITICAL ERROR [UNKNOWN] \n");
        PRINTF_LV_0 ("# Please close TC3X Driver \n");
        break;
    case TC3X_ERROR_CRITICAL_MAILBOX:
        PRINTF_LV_0 ("# CRITICAL ERROR [MAILBOX] \n");
        PRINTF_LV_0 ("# Please close TC3X Driver \n");
        break;
    }
}

//-----------------------------------------------------------------------------------------------
// TDMB
//
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_SET_TDMB_Service
//    Description : Set TDMB Service information
//    Input : 
//      moduleidx : module index
//      RegFlag : 1 Reg / 0 UnReg
//      pTDMBService : Service information
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_SET_TDMB_Service (int moduleidx, int RegFlag, TDMBService_t * pTDMBService)
{
    int       retv;
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;

    if (RegFlag == 0)
    {
        retv =
            g_pSTDCtrl[moduleidx][0].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][0].hBB, CMD_BB_TDMB_UNREG,
                                                      pTDMBService, NULL, NULL, NULL);
    }
    else
    {
        retv =
            g_pSTDCtrl[moduleidx][0].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][0].hBB, CMD_BB_TDMB_REG,
                                                      pTDMBService, NULL, NULL, NULL);
    }
    return retv;
}

//-----------------------------------------------------------------------------------------------
// TDMB Status Monitoring
//
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetTDMB_Lock
//    Description : Get Lock Status
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      pLock : Lock Status
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetTDMB_Lock (int moduleidx, unsigned int DeviceIdx, t_TDMBLock * pLock)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB,
                                                      CMD_GET_TDMB_LOCK, pLock, NULL, NULL, NULL);
    return 0;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetTDMB_Lock_Wait
//    Description : Get Lock Status until Thr Time.
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//    Output : 
//      1 : success, others : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetTDMB_Lock_Wait (int moduleidx, unsigned int DeviceIdx)
{
    int       i, j;
    t_TDMBLock Lock;
    t_TDMBLock Lock_Sub;
    int       Tickms;
    int       TickCnt;
    int       SubLockOK;
#if defined(USE_ANDROID) || defined(USE_LINUX)
    long long starttime;
    long long timeout;
#else
    unsigned int starttime;
    unsigned int timeout;
#endif

    // Fast Scan : find exist channel in 100ms!!
    //-------------------------------------------
    // 1. OFDM Detect : if not fail : (TDMB_OFDMDETECT_LOCK * TDMB_OFDMDETECT_RETRY) ms : 
    // 2. CTO & CFO Detect : if not fail :(TDMB_CTO_LOCK * TDMB_CTO_RETRY) + ((TDMB_CTO_LOCK+TDMB_CFO_LOCK) * TDMB_CFO_RETRY) ms;

    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    Tickms = 10;

    // OFDM Detect
    SubLockOK = 0;
    TickCnt = (TDMB_OFDMDETECT_LOCK * TDMB_OFDMDETECT_RETRY) / Tickms;

    timeout = (TDMB_OFDMDETECT_LOCK * TDMB_OFDMDETECT_RETRY) * 1000;
    starttime = TC3X_IO_Util_GetTickCnt ();

    for (i = 0; i < TickCnt; i++)
    {
        TC3X_IO_memset (&Lock, 0x00, sizeof (t_TDMBLock));
        for (j = 0; j < g_tSTDInfo[moduleidx].NumOfDemodule; j++)
        {
            TC3X_API_GetTDMB_Lock (moduleidx, j, &Lock_Sub);
            Lock.AGC |= Lock_Sub.AGC;
            Lock.DC |= Lock_Sub.DC;
            Lock.CTO |= Lock_Sub.CTO;
            Lock.CFO |= Lock_Sub.CFO;
            Lock.FFO |= Lock_Sub.FFO;
            Lock.FTO |= Lock_Sub.FTO;
            Lock.SYNC_BYTE |= Lock_Sub.SYNC_BYTE;
            Lock.OFDM_DETECT |= Lock_Sub.OFDM_DETECT;
        }

        if (Lock.CTO && Lock.CFO)
        {
            return 1;   // Lock Success
        }

        if (Lock.OFDM_DETECT)
        {
            SubLockOK = 1;
            break;
        }
        else
        {
            if (TC3X_IO_Util_GetInterval (starttime) > timeout)
            {
                return 0;
            }
        }
        TC3X_IO_Sleep (Tickms);
    }

    if (SubLockOK == 0) // Lock Fail
        return 0;

    // CTO & CFO Detect
    SubLockOK = 0;
    TickCnt = ((TDMB_CTO_LOCK * TDMB_CTO_RETRY) + ((TDMB_CTO_LOCK + TDMB_CFO_LOCK) * TDMB_CFO_RETRY)) / Tickms;

    timeout = ((TDMB_CTO_LOCK * TDMB_CTO_RETRY) + ((TDMB_CTO_LOCK + TDMB_CFO_LOCK) * TDMB_CFO_RETRY)) * 1000;
    starttime = TC3X_IO_Util_GetTickCnt ();

    for (i = 0; i < TickCnt; i++)
    {
        TC3X_IO_memset (&Lock, 0x00, sizeof (t_TDMBLock));
        for (j = 0; j < g_tSTDInfo[moduleidx].NumOfDemodule; j++)
        {
            TC3X_API_GetTDMB_Lock (moduleidx, j, &Lock_Sub);
            Lock.AGC |= Lock_Sub.AGC;
            Lock.DC |= Lock_Sub.DC;
            Lock.CTO |= Lock_Sub.CTO;
            Lock.CFO |= Lock_Sub.CFO;
            Lock.FFO |= Lock_Sub.FFO;
            Lock.FTO |= Lock_Sub.FTO;
            Lock.SYNC_BYTE |= Lock_Sub.SYNC_BYTE;
            Lock.OFDM_DETECT |= Lock_Sub.OFDM_DETECT;
        }
        if (Lock.CTO && Lock.CFO)
        {
            SubLockOK = 1;
            return 1;   // Lock Success
        }
        else
        {
            if (TC3X_IO_Util_GetInterval (starttime) > timeout)
            {
                return 0;
            }
        }
        TC3X_IO_Sleep (Tickms);
    }

    return 0;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetTDMB_SNR
//    Description : Get snr
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      psnr : snr
//      pSNRdb : snr db
//      pMovingAVGSNRdb : Average snr db
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetTDMB_SNR (int moduleidx, unsigned int DeviceIdx, double *pSNR, double *pSNRdb, double *pMovingAVGSNRdb)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB,
                                                      CMD_GET_TDMB_SNR_SET, pSNR, pSNRdb, pMovingAVGSNRdb, NULL);
    return 0;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetTDMB_PCBER
//    Description : Get pcber
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      pPCBER : pcber
//      pMovingAVGPCBER : Average pcber
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetTDMB_PCBER (int moduleidx, unsigned int DeviceIdx, double *pPCBER, double *pMovingAVGPCBER)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB,
                                                      CMD_GET_TDMB_PCBER_SET, pPCBER, pMovingAVGPCBER, NULL, NULL);
    return 0;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetTDMB_VITERBIBER
//    Description : Get VITERBI Ber
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      pVITERBIBER : Viterbi ber
//      pMovingAVGVITERBIBER : Average Viterbi ber
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetTDMB_VITERBIBER (int moduleidx, unsigned int DeviceIdx, double *pVITERBIBER,
                                 double *pMovingAVGVITERBIBER)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB,
                                                      CMD_GET_TDMB_VITERBIBER_SET, pVITERBIBER, pMovingAVGVITERBIBER,
                                                      NULL, NULL);
    return 0;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetTDMB_TSPER
//    Description : Get TSPER
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      pTSPER : TSPER
//      pMovingAVGTSPER : Average TSPER
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetTDMB_TSPER (int moduleidx, unsigned int DeviceIdx, double *pTSPER, double *pMovingAVGTSPER)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB,
                                                      CMD_GET_TDMB_TSPER_SET, pTSPER, pMovingAVGTSPER, NULL, NULL);
    return 0;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_GetTDMB_RSSI
//    Description : Get RSSI
//    Input : 
//      moduleidx : module index
//      DeviceIdx : device index
//      pRSSI : RSSI
//      pMovingAVGRSSI : Average RSSI
//    Output : 
//      -1 : fail
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_GetTDMB_RSSI (int moduleidx, unsigned int DeviceIdx, double *pRSSI, double *pMovingAVGRSSI)
{
    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB,
                                                      CMD_GET_TDMB_RSSI_SET, pRSSI, pMovingAVGRSSI, NULL, NULL);
    return 0;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_PushStatusData
//    Description : Manual Push Status Data
//    Input : 
//      moduleidx : module index
//      DeviceIdx : Device index
//      pData : Data pointer
//      Size : Size of Data
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_API_PushStatusData (int moduleidx, unsigned int DeviceIdx, unsigned char *pData, unsigned int Size)
{
    int       ret = 0;

    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    ret =
        g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB,
                                                          CMD_PUSH_TDMB_STAT_DATA, pData, &Size, NULL, NULL);
    return ret;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_ParseData
//    Description : Manual Push Status Data
//    Input : 
//      moduleidx : module index
//      DeviceIdx : Device index
//      pData : Data pointer
//      Size : Size of Data
//    Output : 
//    
//    Remark : It will be call event function
//--------------------------------------------------------------------------
int TC3X_API_ParseData (int moduleidx, unsigned int DeviceIdx, unsigned char *pData, unsigned int Size)
{
    int       ret = 0;

    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    ret =
        g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB,
                                                          CMD_REQUEST_PARSE_TDMB_DATA, pData, &Size, NULL, NULL);
    return ret;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_API_Reset_MonitorValue
//    Description : Reset PCBER
//    Input : 
//      moduleidx : module index
//      DeviceIdx : Device index
//    Output : 
//    
//--------------------------------------------------------------------------
int TC3X_API_Reset_MonitorValue (int moduleidx, unsigned int DeviceIdx)
{
    int       ret = 0;

    if (!g_pSTDCtrl)
        return -1;
    if (!g_pSTDCtrl[moduleidx])
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BSetted)
        return -1;
    if (!g_pSTDCtrl[moduleidx][0].BInited)
        return -1;

    ret =
        g_pSTDCtrl[moduleidx][DeviceIdx].BB_UserFunction (moduleidx, g_pSTDCtrl[moduleidx][DeviceIdx].hBB,
                                                          CMD_REQUEST_TDMB_RESET_MONITOR_VALUE, NULL, NULL, NULL, NULL);
    return ret;
}

#endif // STDDEF_DMB
