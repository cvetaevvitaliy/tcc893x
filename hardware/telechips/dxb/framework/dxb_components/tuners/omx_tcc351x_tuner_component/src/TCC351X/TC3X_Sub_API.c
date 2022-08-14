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

#include "TC3X_Sub_API.h"
#include "IO/TC3X_IO.h"

#if defined (USE_IF_SRAMLIKE)
#include "IO/TC3X_IO_SRAMLIKE.h"
#endif // USE_IF_SRAMLIKE
#if defined (USE_IF_I2C)
#include "IO/TC3X_IO_I2C.h"
#endif // USE_IF_I2C
#if defined (USE_IF_CSPI)
#include "IO/TC3X_IO_CSPI.h"
#endif // USE_IF_CSPI
#if defined (USE_IF_SDIO)
#include "IO/TC3X_IO_SDIO.h"
#endif // USE_IF_SDIO
#if defined (USE_IF_HPI)
#include "IO/TC3X_IO_HPI.h"
#endif // USE_IF_HPI

#include "IO/TC3X_IO_UTIL.h"

// extern
extern TC3X_DNUM TC3X_Entry (int moduleidx,
                      TC3X_DNUM hSTD,
                      FN_v_ihv * BB_PreInit,
                      FN_i_ivi * BB_ColdBoot,
                      FN_v_ihi * BB_Init,
                      FN_v_ih * BB_Close,
                      FN_i_ihiiv * BB_SetFrequency,
                      FN_v_ihiPci * BB_StartService,
                      FN_v_ihi * BB_StopService, FN_v_ihi * BB_CtrlPower, FN_i_ihivvvv * BB_UserFunction, FN_v_ihivvvv eventFn, int option);
extern void      TC3X_AttachDevice (int moduleidx, int num, void *pSetOption);

tSTDInfo  g_tSTDInfo[MAX_TCC3X];
void *pg_tSTDInfo0 = NULL;
void *pg_tSTDInfo1 = NULL;
tSTDCtrl **g_pSTDCtrl = NULL;

typedef struct
{
    unsigned int open_count;
    unsigned int Standard;
} tTCC3X_Driver;

tTCC3X_Driver TCC3X_Driver[MAX_TCC3X];
void *pTCC3X_Driver0 = NULL;
void *pTCC3X_Driver1 = NULL;

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_SubAPI_Init
//    Description : Application initialize code
//    Input : 
//      moduleidx : module index 
//      NumOfDemodule : Number of Demodulator
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_SubAPI_Init (int moduleidx, int NumDemodule)
{
    int       maxbb;
    maxbb = TC3X_IO_Util_GET_MAXBB();

    if(moduleidx>=maxbb)
    {
        PRINTF_LV_0("[Error] TC3X to much devices\n");
        return;
    }

    switch(moduleidx)
    {
    case 0:
        if(pTCC3X_Driver0)
        {
            PRINTF_LV_0("[Error] TC3X aleady opend\n");
            return;
        }
        else
        {
            pTCC3X_Driver0 = &TCC3X_Driver[0];
        }
        break;
    case 1:
        if(pTCC3X_Driver1)
        {
            PRINTF_LV_0("[Error] TC3X aleady opend\n");
            return;
        }
        else
        {
            pTCC3X_Driver1 = &TCC3X_Driver[1];
        }
        break;
    default:
        break;
    }
    

    TCC3X_Driver[moduleidx].open_count++;
    
    TC3X_IO_memset (&g_tSTDInfo[moduleidx], 0, sizeof (tSTDInfo));
    g_tSTDInfo[moduleidx].NumOfDemodule = NumDemodule;

    if (!g_pSTDCtrl)
    {
        g_pSTDCtrl = (tSTDCtrl **) TC3X_IO_malloc (sizeof (tSTDCtrl *) * maxbb);
        TC3X_IO_memset (g_pSTDCtrl, 0, sizeof (tSTDCtrl *) * maxbb);
    }

    if (g_pSTDCtrl[moduleidx])
    {
        TC3X_IO_free (g_pSTDCtrl[moduleidx]);
        g_pSTDCtrl[moduleidx] = NULL;
    }

    g_pSTDCtrl[moduleidx] = (tSTDCtrl *) TC3X_IO_malloc (sizeof (tSTDCtrl) * g_tSTDInfo[moduleidx].NumOfDemodule);
    TC3X_IO_memset (g_pSTDCtrl[moduleidx], 0, sizeof (tSTDCtrl) * g_tSTDInfo[moduleidx].NumOfDemodule);

    TC3X_IO_Host_Preset (moduleidx);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_SubAPI_IOOpen
//    Description : IO Open API
//    Input : 
//      moduleidx : module index
//      MainIO : main IO
//      StreamIO : stream IO
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_SubAPI_IOOpen (int moduleidx, int MainIO, int StreamIO)
{
    g_tSTDInfo[moduleidx].MainIO = MainIO;
    g_tSTDInfo[moduleidx].StreamIO = StreamIO;

    // BB & RF PWON
    TC3X_SubAPI_PWON (moduleidx);

    // io select & setting
    TC3X_SubAPI_MainIOSel (moduleidx, MainIO);
    TC3X_SubAPI_StreamIOSel (moduleidx, StreamIO);

    switch(moduleidx)
    {
    case 0:
        pg_tSTDInfo0 = &g_tSTDInfo[moduleidx];
    break;
    case 1:
        pg_tSTDInfo1 = &g_tSTDInfo[moduleidx];
    break;
    default :
    break;
    }
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_SubAPI_BBOpen
//    Description : Find BaseBand & Attach
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_SubAPI_BBOpen (int moduleidx, tTC3X_SettingOption *pSetOption)
{
    g_tSTDInfo[moduleidx].BBType = TC3X_SubAPI_BBDetect (moduleidx, pSetOption);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_SubAPI_GetPacketInfo
//    Description : Get packet information
//    Input : 
//      moduleidx : module index
//      Standard : Standard
//      pPktSize : packet size
//      pPktThrNum : packet threshold number
//    Output : 
//      0
//    Remark : 
//--------------------------------------------------------------------------
static int TC3X_SubAPI_GetPacketInfo (int moduleidx, int Standard, int *pPktSize, int *pPktThrNum)
{
    switch (Standard)
    {
#if defined (STDDEF_DVBT) || defined (STDDEF_DVBH)
    case TC3X_STD_DVBT:
        {
            unsigned int table_pktThrNum[TC3X_STREAM_IO_MAX + 1] = {
                TC3X_DVBT_PKTNUM_CMDIO, TC3X_DVBT_PKTNUM_SPI,
                TC3X_DVBT_PKTNUM_STS, TC3X_DVBT_PKTNUM_SPI,
                TC3X_DVBT_PKTNUM_SPI, TC3X_DVBT_PKTNUM_CMDIO,
                TC3X_DVBT_PKTNUM_CMDIO, TC3X_DVBT_PKTNUM_SPI
            };
            *pPktSize = TC3X_DVBT_PKTSIZE;
            if (g_tSTDInfo[moduleidx].StreamIO < TC3X_STREAM_IO_MAX)
                *pPktThrNum = table_pktThrNum[g_tSTDInfo[moduleidx].StreamIO];
            else
                *pPktThrNum = table_pktThrNum[TC3X_STREAM_IO_MAX];
        }
        break;
#endif // defined (STDDEF_DVBT) || defined (STDDEF_DVBH)
#if defined (STDDEF_DVBH)
    case TC3X_STD_DVBH:
        {
            unsigned int table_pktThrNum[TC3X_STREAM_IO_MAX + 1] = {
                TC3X_DVBH_PKTNUM_CMDIO, TC3X_DVBH_PKTNUM_SPI,
                TC3X_DVBH_PKTNUM_STS, TC3X_DVBH_PKTNUM_SPI,
                TC3X_DVBH_PKTNUM_SPI, TC3X_DVBH_PKTNUM_CMDIO,
                TC3X_DVBH_PKTNUM_CMDIO, TC3X_DVBH_PKTNUM_SPI
            };
            *pPktSize = TC3X_DVBH_PKTSIZE;
            if (g_tSTDInfo[moduleidx].StreamIO < TC3X_STREAM_IO_MAX)
                *pPktThrNum = table_pktThrNum[g_tSTDInfo[moduleidx].StreamIO];
            else
                *pPktThrNum = table_pktThrNum[TC3X_STREAM_IO_MAX];

        }
        break;
#endif // STDDEF_DVBH
#if defined (STDDEF_CMMB)
    case TC3X_STD_CMMB:
        {
            unsigned int table_pktThrNum[TC3X_STREAM_IO_MAX + 1] = {
                TC3X_CMMB_PKTNUM, TC3X_CMMB_PKTNUM_TS,
                TC3X_CMMB_PKTNUM_TS, TC3X_CMMB_PKTNUM,
                TC3X_CMMB_PKTNUM, TC3X_CMMB_PKTNUM,
                TC3X_CMMB_PKTNUM, TC3X_CMMB_PKTNUM
            };
            unsigned int table_pktSize[TC3X_STREAM_IO_MAX + 1] = {
                TC3X_CMMB_PKTSIZE, TC3X_CMMB_PKTSIZE_TS,
                TC3X_CMMB_PKTSIZE_TS, TC3X_CMMB_PKTSIZE,
                TC3X_CMMB_PKTSIZE, TC3X_CMMB_PKTSIZE,
                TC3X_CMMB_PKTSIZE, TC3X_CMMB_PKTSIZE
            };
            
            
            if (g_tSTDInfo[moduleidx].StreamIO < TC3X_STREAM_IO_MAX)
            {
                *pPktThrNum = table_pktThrNum[g_tSTDInfo[moduleidx].StreamIO];
                *pPktSize = table_pktSize[g_tSTDInfo[moduleidx].StreamIO];
            }
            else
            {
                *pPktThrNum = table_pktThrNum[TC3X_STREAM_IO_MAX];
                *pPktSize = table_pktSize[TC3X_STREAM_IO_MAX];
            }
        }
        break;
#endif // STDDEF_CMMB
#if defined (STDDEF_DMB)
    case TC3X_STD_DMB:
        {
            unsigned int table_pktThrNum[TC3X_STREAM_IO_MAX + 1] = {
                TC3X_TDMB_PKTNUM_CMDIO, TC3X_TDMB_PKTNUM,
                TC3X_TDMB_PKTNUM, TC3X_TDMB_PKTNUM,
                TC3X_TDMB_PKTNUM, TC3X_TDMB_PKTNUM_CMDIO,
                TC3X_TDMB_PKTNUM_CMDIO, TC3X_TDMB_PKTNUM
            };
            *pPktSize = TC3X_TDMB_PKTSIZE;
            if (g_tSTDInfo[moduleidx].StreamIO < TC3X_STREAM_IO_MAX)
                *pPktThrNum = table_pktThrNum[g_tSTDInfo[moduleidx].StreamIO];
            else
                *pPktThrNum = table_pktThrNum[TC3X_STREAM_IO_MAX];

        }
        break;
#endif // STDDEF_DMB
#if defined(STDDEF_ISDBT1SEG)
    case TC3X_STD_ISDBT1SEG:
        {
            unsigned int table_pktThrNum[TC3X_STREAM_IO_MAX + 1] = {
                TC3X_ISDBT1SEG_PKTNUM_CMDIO, TC3X_ISDBT1SEG_PKTNUM_SPI,
                TC3X_ISDBT1SEG_PKTNUM_STS, TC3X_ISDBT1SEG_PKTNUM_SPI,
                TC3X_ISDBT1SEG_PKTNUM_SPI, TC3X_ISDBT1SEG_PKTNUM_CMDIO,
                TC3X_ISDBT1SEG_PKTNUM_CMDIO, TC3X_ISDBT1SEG_PKTNUM_SPI
            };
            *pPktSize = TC3X_ISDBT1SEG_PKTSIZE;
            if (g_tSTDInfo[moduleidx].StreamIO < TC3X_STREAM_IO_MAX)
                *pPktThrNum = table_pktThrNum[g_tSTDInfo[moduleidx].StreamIO];
            else
                *pPktThrNum = table_pktThrNum[TC3X_STREAM_IO_MAX];

        }
        break;
#endif // defined(STDDEF_ISDBT1SEG)

#if defined(STDDEF_ISDBT13SEG)
    case TC3X_STD_ISDBT13SEG:
        {
            unsigned int table_pktThrNum[TC3X_STREAM_IO_MAX + 1] = {
                TC3X_ISDBT13SEG_PKTNUM_CMDIO, TC3X_ISDBT13SEG_PKTNUM_SPI,
                TC3X_ISDBT13SEG_PKTNUM_STS, TC3X_ISDBT13SEG_PKTNUM_SPI,
                TC3X_ISDBT13SEG_PKTNUM_SPI, TC3X_ISDBT13SEG_PKTNUM_CMDIO,
                TC3X_ISDBT13SEG_PKTNUM_CMDIO, TC3X_ISDBT13SEG_PKTNUM_SPI
            };
            *pPktSize = TC3X_ISDBT13SEG_PKTSIZE;
            if (g_tSTDInfo[moduleidx].StreamIO < TC3X_STREAM_IO_MAX)
                *pPktThrNum = table_pktThrNum[g_tSTDInfo[moduleidx].StreamIO];
            else
                *pPktThrNum = table_pktThrNum[TC3X_STREAM_IO_MAX];

        }
        break;
#endif // defined(STDDEF_ISDBT13SEG)

#if defined (STDDEF_RESERVED1)
    case TC3X_STD_OTHERS:
        {
            unsigned int table_pktThrNum[TC3X_STREAM_IO_MAX + 1] = {
                TC3X_CMMB_PKTNUM, TC3X_CMMB_PKTNUM,
                TC3X_CMMB_PKTNUM, TC3X_CMMB_PKTNUM,
                TC3X_CMMB_PKTNUM, TC3X_CMMB_PKTNUM,
                TC3X_CMMB_PKTNUM, TC3X_CMMB_PKTNUM
            };
            *pPktSize = TC3X_CMMB_PKTSIZE;
            if (g_tSTDInfo[moduleidx].StreamIO < TC3X_STREAM_IO_MAX)
                *pPktThrNum = table_pktThrNum[g_tSTDInfo[moduleidx].StreamIO];
            else
                *pPktThrNum = table_pktThrNum[TC3X_STREAM_IO_MAX];
        }
        break;
#endif // STDDEF_RESERVED1

    }

    return 0;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_SubAPI_BBPreInit
//    Description : BaseBand Pre-Initialize & Register Callback Function
//    Input : 
//      moduleidx : module index
//      Standard : Standard
//      EVENTFunction : Event function (call func)
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_SubAPI_BBPreInit (int moduleidx, int Standard, FN_v_ihivvvv EVENTFunction)
{
    int       i;
    tSTDIOSet STDIOSet;
    int       pktSize = 0, pktThrNum = 0;

    TCC3X_Driver[moduleidx].Standard = Standard;

    g_tSTDInfo[moduleidx].Standard = Standard;
    TC3X_SubAPI_GetPacketInfo (moduleidx, Standard, &pktSize, &pktThrNum);

    for (i = 0; i < g_tSTDInfo[moduleidx].NumOfDemodule; i++)
    {
        switch (g_tSTDInfo[moduleidx].BBType)
        {
        case DEVICE_BB_TC3X:
            g_pSTDCtrl[moduleidx][i].bbType = DEVICE_BB_TC3X;

            g_pSTDCtrl[moduleidx][i].hBB = TC3X_Entry (moduleidx, i,
                                                       &g_pSTDCtrl[moduleidx][i].BB_PreInit, &g_pSTDCtrl[moduleidx][i].BB_ColdBoot,
                                                       &g_pSTDCtrl[moduleidx][i].BB_Init, &g_pSTDCtrl[moduleidx][i].BB_Close,
                                                       &g_pSTDCtrl[moduleidx][i].BB_SetFrequency, &g_pSTDCtrl[moduleidx][i].BB_StartService,
                                                       &g_pSTDCtrl[moduleidx][i].BB_StopService, &g_pSTDCtrl[moduleidx][i].BB_CtrlPower,
                                                       &g_pSTDCtrl[moduleidx][i].BB_UserFunction, EVENTFunction, 0);

            STDIOSet.Standard = g_tSTDInfo[moduleidx].Standard;
            STDIOSet.MainIO = g_tSTDInfo[moduleidx].MainIO;
            STDIOSet.StreamIO = g_tSTDInfo[moduleidx].StreamIO;
            STDIOSet.PktSize = pktSize;
            STDIOSet.PktThrNum = pktThrNum;

            g_pSTDCtrl[moduleidx][i].BB_PreInit (moduleidx, g_pSTDCtrl[moduleidx][i].hBB, (void *) &STDIOSet);
            g_pSTDCtrl[moduleidx][i].BSetted = 1;       // Setted
            break;
        }
    }
}

// -------------------------------------------------------
// BB ColdBoot
//
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_SubAPI_BBColdBoot_NONWrite
//    Description : Cold boot without write to memory
//    Input : 
//      moduleidx : module index
//    Output : 
//      Success: 0x1ACCE551 Fail : others
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_SubAPI_BBColdBoot_NONWrite(int moduleidx)
{
    int       ret;
    ret = TC3X_SubAPI_COLDBOOT (moduleidx, NULL, CMD_BB_COLDBOOT_NON_WRITE);
    return ret;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_SubAPI_BBColdBoot_FILE
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
int TC3X_SubAPI_BBColdBoot_FILE (int moduleidx, unsigned char *pData, unsigned int Size, int option)
{
    int       ret;
    TC3X_BOOTBIN BootBin;
    ret = TC3X_IO_UTIL_ColdBootParser (pData, Size, &BootBin);
    if (!ret)
    {
        PRINTF_LV_0 ("# coldboot parser error\n");
    }
    ret = TC3X_SubAPI_COLDBOOT (moduleidx, &BootBin, option);
    return ret;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_SubAPI_BBColdBoot_MEM
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
int TC3X_SubAPI_BBColdBoot_MEM (int moduleidx, unsigned char *pData, unsigned int Size, int option)
{
    int       ret;
    TC3X_BOOTBIN BootBin;
    ret = TC3X_IO_UTIL_ColdBootParser (pData, Size, &BootBin);
    if (!ret)
    {
        PRINTF_LV_0 ("# coldboot parser error\n");
    }
    ret = TC3X_SubAPI_COLDBOOT (moduleidx, &BootBin, option);
    return ret;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_SubAPI_BBColdBoot_NANDFLASH
//    Description : Write Boot Code Binary File to BB from nandflash
//    Input : 
//      moduleidx : module index
//      pData : Boot Binary Data Pointer
//      size : size of boot binary
//      option : broadcast / normal / non write
//    Output : 
//      Success: 0x1ACCE551 Fail : others
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_SubAPI_BBColdBoot_NANDFLASH (int moduleidx, char *FileName, int option)
{
    return 0;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_SubAPI_COLDBOOT
//    Description : cold boot
//    Input : 
//      moduleidx : module index
//      BootBin : boot binary
//      option : option
//    Output : 
//      Success: 0x1ACCE551 Fail : others
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_SubAPI_COLDBOOT (int moduleidx, TC3X_BOOTBIN * BootBin, int option)
{       // access at once
    int       ret;
    ret = g_pSTDCtrl[moduleidx][0].BB_ColdBoot (moduleidx, (void *) BootBin, option);
    return ret;
}

// First Setup Slave and then Setup Master.
// TC3X GPIO Setup & RF Search & RF Init
//void TC3X_SubAPI_BBInit (TC3X_DNUM handle, int UseI2CRepeater)
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_SubAPI_BBInit
//    Description : BB Initialization (if use i2c, i2c repeater mode will be enable)
//    Input : 
//      moduleidx : module index
//      UseI2CRepeater : 1 : use i2c repeater
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_SubAPI_BBInit (int moduleidx, int UseI2CRepeater)
{
    g_pSTDCtrl[moduleidx][0].BB_Init (moduleidx, g_pSTDCtrl[moduleidx][0].hBB, UseI2CRepeater);
    g_pSTDCtrl[moduleidx][0].BInited = 1;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_SubAPI_Close
//    Description : close sub api
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_SubAPI_Close (int moduleidx)
{
    int       i;
    int       exist;
    int       maxbb;
    maxbb = TC3X_IO_Util_GET_MAXBB();
    exist = 0;
    
    // Detach
    TC3X_SubAPI_BBDetach (moduleidx);   // include rfdetach

    // BB & RF PWDN
    TC3X_SubAPI_PWDN (moduleidx);

    // IO Close

    switch (g_tSTDInfo[moduleidx].MainIO)
    {
    case TC3X_IF_SRAMLIKE:
#if defined (USE_IF_SRAMLIKE)
        TC3X_IO_SRAMLIKE_Close (moduleidx);
#endif // USE_IF_SRAMLIKE
        break;

    case TC3X_IF_I2C:
#if defined (USE_IF_I2C)
        TC3X_IO_I2C_Close (moduleidx);
#endif // USE_IF_I2C
        break;

    case TC3X_IF_CSPI:
#if defined (USE_IF_CSPI)
        TC3X_IO_CSPI_Close (moduleidx);
#endif // USE_IF_CSPI
        break;

    case TC3X_IF_SDIO1BIT:
#if defined (USE_IF_SDIO)
        TC3X_IO_SDIO_Close(moduleidx);
#endif // USE_IF_SDIO
        break;

    case TC3X_IF_SDIO4BIT:
#if defined (USE_IF_SDIO)
        TC3X_IO_SDIO_Close(moduleidx);
#endif // USE_IF_SDIO
        break;
    }
    
    
    if (g_pSTDCtrl[moduleidx])
    {
        TC3X_IO_free (g_pSTDCtrl[moduleidx]);
        g_pSTDCtrl[moduleidx] = NULL;
    }

    exist = 0;
    for (i = 0; i < maxbb; i++)
    {
        if (g_pSTDCtrl[i])
        {
            exist = 1;
            break;
        }
    }

    if (!exist)
    {
        TC3X_IO_free (g_pSTDCtrl);
        g_pSTDCtrl = NULL;
    }

    TCC3X_Driver[moduleidx].open_count = 0;
    
    switch(moduleidx)
    {
    case 0:
        pTCC3X_Driver0 = NULL;
        break;
    case 1:
        pTCC3X_Driver1 = NULL;
        break;
    default:
        break;
    }

}

// -------------------------------------------------------
// PW Ctrl
//
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_SubAPI_PWON
//    Description : power on
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_SubAPI_PWON (int moduleidx)
{
    TC3X_IO_PW_ON (moduleidx);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_SubAPI_PWDN
//    Description : power down
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_SubAPI_PWDN (int moduleidx)
{
    TC3X_IO_PW_DN (moduleidx);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_SubAPI_PWRESET
//    Description : power reset
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_SubAPI_PWRESET (int moduleidx)
{
    TC3X_IO_PW_RESET (moduleidx);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_SubAPI_PWRESET
//    Description : power reset
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_SubAPI_SetAntennaPower(int moduleidx, int arg)
{
    TC3X_IO_Set_AntennaPower(arg);
}

// -------------------------------------------------------
// Select IO
//
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_SubAPI_MainIOSel
//    Description : select main io and initialize
//    Input : 
//      moduleidx : module index
//      IOSeries : io series
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_SubAPI_MainIOSel (int moduleidx, int IOSeries)
{
    switch (IOSeries)
    {
    case TC3X_IF_SRAMLIKE:
#if defined (USE_IF_SRAMLIKE)
        TC3X_IO_SRAMLIKE_Init (moduleidx);
#endif // USE_IF_SRAMLIKE
        break;

    case TC3X_IF_I2C:
#if defined (USE_IF_I2C)
        TC3X_IO_I2C_Init (moduleidx);
#endif // USE_IF_I2C
        break;

    case TC3X_IF_CSPI:
#if defined (USE_IF_CSPI)
        TC3X_IO_CSPI_Init (moduleidx);
#endif // USE_IF_CSPI
        break;

    case TC3X_IF_SDIO1BIT:
#if defined (USE_IF_SDIO)
        TC3X_IO_SDIO_Init (moduleidx, 1);
#endif // USE_IF_SDIO
        break;

    case TC3X_IF_SDIO4BIT:
#if defined (USE_IF_SDIO)
        TC3X_IO_SDIO_Init (moduleidx, 4);
#endif // USE_IF_SDIO
        break;
    }
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_SubAPI_StreamIOSel
//    Description : select stream io
//    Input : 
//      moduleidx : module index
//      IOSeries : io series
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_SubAPI_StreamIOSel (int moduleidx, int IOSeries)
{
    switch (IOSeries)
    {
    case TC3X_STREAM_IO_MAINIO:
        break;

    case TC3X_STREAM_IO_PTS:
    case TC3X_STREAM_IO_STS:
    case TC3X_STREAM_IO_SPIMS:
    case TC3X_STREAM_IO_SPISLV:
        break;
#ifdef USE_IF_HPI
    case TC3X_STREAM_IO_HPI_HEADERON:  // use MainIO = I2C
        TC3X_IO_HPI_Init (moduleidx);
        break;

    case TC3X_STREAM_IO_HPI_HEADEROFF: // use MainIO = I2C
        TC3X_IO_HPI_Init (moduleidx);
        break;
#endif
    }
}

// -------------------------------------------------------
// BB Detect
//
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_SubAPI_BBDetect
//    Description : detect baseband
//    Input : 
//      moduleidx : module index
//    Output : 
//      baseband
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_SubAPI_BBDetect (int moduleidx, tTC3X_SettingOption *pSetOption)
{
    TC3X_SubAPI_BBAttach (moduleidx, DEVICE_BB_TC3X, pSetOption);
    return DEVICE_BB_TC3X;
}

// -------------------------------------------------------
// BB Attach
//
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_SubAPI_BBAttach
//    Description : attach baseband
//    Input : 
//      moduleidx : module index
//      DeviceBB : device baseband
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_SubAPI_BBAttach (int moduleidx, int DeviceBB, tTC3X_SettingOption *pSetOption)
{
    switch (DeviceBB)
    {
    case DEVICE_BB_TC3X:
        TC3X_AttachDevice (moduleidx, g_tSTDInfo[moduleidx].NumOfDemodule, pSetOption);
        break;
    }
}

// =======================================================
// Service Control
//

//  SetFreq Sequence : Stream Stop - Stream Start - SetFreq
//  Standard
//  Change Freq (same standard) : Same as SetFreq Sequence
//  Change Freq (diff standard) : BB PreInit(standard) - ColdBoot(standard) - BBInit(standard) - SetFreq Sequence

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_SubAPI_SetFrequency
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
int TC3X_SubAPI_SetFrequency (int moduleidx, int freq_khz, int bw_khz, tFreqSet_Option * pOption)
{
    return (g_pSTDCtrl[moduleidx][0].BB_SetFrequency (moduleidx, g_pSTDCtrl[moduleidx][0].hBB, freq_khz, bw_khz, pOption));
}

// =======================================================
// Driver Detach
//
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_SubAPI_BBDetach
//    Description : detach baseband
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_SubAPI_BBDetach (int moduleidx)
{
    if (!g_pSTDCtrl)
        return;
        
    if (!g_pSTDCtrl[moduleidx])
        return;

    if (!g_pSTDCtrl[moduleidx][0].BB_Close)
        return;

    g_pSTDCtrl[moduleidx][0].BB_Close (moduleidx, g_pSTDCtrl[moduleidx][0].hBB);
}
