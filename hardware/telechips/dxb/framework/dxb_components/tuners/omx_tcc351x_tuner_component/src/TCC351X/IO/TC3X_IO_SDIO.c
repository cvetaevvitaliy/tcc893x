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

#include "TC3X_IO_SDIO.h"
#if defined (USE_IF_SDIO)
#include "TC3X_IO.h"
#include "mmc_basic.h"
#ifdef TCC78X
#include "TCC78x.h"
#else
#include "TCC79x.h"

#include "sd.h"
#include "sdio_func.h"
#endif

#include "IO_TCC7XX.h"

#ifdef	TCC78X
#if (defined(MMC_INCLUDE) || defined(TRIFLASH_INCLUDE))
#define	TIMEOUT_VALUE	0.5     // about 0.5 Second
#else
#define	TIMEOUT_VALUE	3       // about 3 Second
#endif
#else
#ifdef MMC_INCLUDE
#define	TIMEOUT_VALUE	200000  // about 0.5 Second
#else
#define	TIMEOUT_VALUE	1000000 // about 2.66 Second
#endif
#endif

#define MODE_1BIT	1
#define MODE_4BIT	4

#ifdef TCC78X
#define		TCC78X_SDMMC_CH1_USE
#endif

typedef struct
{
    struct sdio_driver driver;
    struct sdio_func *pFunc;
} SDIO_CLIENT_FUNC_T;

SDIORESPONSE g_SDIOResponse[MAX_TCC3X];
SDIO_CLIENT_FUNC_T gsdio_client_TC3X[MAX_TCC3X];

extern int SDMMC_DRV_Initialize (void);

//=======================================================================================
//
//  Common
//
//=======================================================================================

static void HandleSdio0 (struct sdio_func *pFunc)
{
    TC3X_IO_SDIO_INT_Handler0 ();
}

static void HandleSdio1 (struct sdio_func *pFunc)
{
    TC3X_IO_SDIO_INT_Handler1 ();
}

void TC3X_IO_SDIO_INT_Handler0 ()
{
    TC3X_IO_LISR_BB_INT_Handler0 ();
}

void TC3X_IO_SDIO_INT_Handler1 ()
{
    TC3X_IO_LISR_BB_INT_Handler1 ();
}

static int _Client_Probe0 (struct sdio_func *pFunc, const struct sdio_device_id *pDeviceId)
{
    gsdio_client_TC3X[0].pFunc = pFunc;

    sdio_enable_func (&gsdio_client_TC3X[0].pFunc);

    if (sdio_claim_irq (pFunc, HandleSdio0))
        return -1;      //failed

    PRINTF_LV_2 ("# SDIO Client Probe...\n");
    return 0;
}

static int _Client_Probe1 (struct sdio_func *pFunc, const struct sdio_device_id *pDeviceId)
{
    gsdio_client_TC3X[1].pFunc = pFunc;

    sdio_enable_func (&gsdio_client_TC3X[1].pFunc);

    if (sdio_claim_irq (pFunc, HandleSdio1))
        return -1;      //failed

    PRINTF_LV_2 ("# SDIO Client Probe...\n");
    return 0;
}

void TC3X_IO_SDIO_VariablesInit (int moduleidx)
{
#if defined TCC78X
    TC3X_IO_memset (&g_SDIOResponse[moduleidx], 0x00, 1 * sizeof (SDIORESPONSE));
    g_SDIOResponse[moduleidx].bus = SDIO_BUS_FREE;
    g_SDIOResponse[moduleidx].bsm = SDIO_BSM_INI;

#elif defined TCC79X
    const struct sdio_device_id sdio_client_device_id_table[] = {
        {(unsigned char) 0x00, 0x00FE, 0x0305},
        //{ (unsigned char)SDIO_ANY_ID, 0x0000, 0x0001 },
        //{ (unsigned char)SDIO_ANY_ID, 0x0000, 0x0002 },
        {0, 0, 0}       // for null termination
    };

    gsdio_client_TC3X[moduleidx].driver.name = "TC3X sdio clint";
    gsdio_client_TC3X[moduleidx].driver.id_table = sdio_client_device_id_table;

    switch(moduleidx)
    {
    case 0:
        gsdio_client_TC3X[moduleidx].driver.probe = _Client_Probe0;
    break;
    case 1:
        gsdio_client_TC3X[moduleidx].driver.probe = _Client_Probe1;
    break;
    default:
    break;
    }

    gsdio_client_TC3X[moduleidx].driver.remove = NULL;
    gsdio_client_TC3X[moduleidx].pFunc = 0;

    if (sdio_register_driver (&gsdio_client_TC3X[moduleidx].driver) == 0)
    {
        // success!
        SD_CardDetectEnable (SDHC_SLOT_1);
        while (gsdio_client_TC3X[moduleidx].pFunc == 0)
            Sleep (10);
    }
    else
    {
        PRINTF_LV_1 ("# sdio_register_driver failure!\n");
    }

#endif
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_SDIO_Init
//    Description : Initialize SDIO
//    Input : 
//      moduleidx : module index
//    Output : 
//      TC3X_IO_SUCCESS
//    Remark : 
//--------------------------------------------------------------------------
TC3X_IO_Err TC3X_IO_SDIO_Init (int moduleidx, unsigned int BWMODE)        /* 79x setting 추가할 것 */
{
//      unsigned int iArg;
//      unsigned int data;

//      char iResBuf[12];
//      char cPNMBuf[12];
//      int  iRCABuf[12];

    switch(moduleidx)
    {
    case 0:

        TC3X_IO_SDIO_VariablesInit (moduleidx);

        // SDIO 1bit : 1, 4bit : 4
        if (BWMODE == 1)
            g_SDIOResponse[moduleidx].BWMODE = MODE_1BIT;
        else
            g_SDIOResponse[moduleidx].BWMODE = MODE_4BIT;

        // SDMMC_SD_ResetCMD와 같음
#ifdef TCC78X
        TC3X_IO_CKC_SetSDMMCClock (270000);
        if (!ISSET (HwBCLKCTR, HwBCLKCTR_SD_ON))
        {
            MMC_AllClockOnOff (ON);
        }
        HwSDICLK = 0x00003000;
        HwSDIDITIMER = 0x40FFFFFF;
        HwSDIDCTRL = 0x0EFF1490;    // multi block transfer사용 안하므로 pass해도 됨
        //HwSDIIENABLE = 0x00;
        HwSDIIENABLE = 0x1FF;

#ifdef TCC78X_SDMMC_CH0_USE
        // SD/MMC Channel 0
        BITCSET (HwPORTCFG0, HwPORTCFG0_EINT_3, HwPORTCFG0_EINT_1);
        BITCSET (HwPORTCFG2, (HwPORTCFG2_UART1_3 | HwPORTCFG2_UARTC1_3), (HwPORTCFG2_UART1_1 | HwPORTCFG2_UARTC1_1));
        BITCSET (HwPORTCFG2, HwPORTCFG2_SDISEL_3, HwPORTCFG2_SDISEL_0);
#endif

#ifdef TCC78X_SDMMC_CH1_USE
        // SD/MMC Channel 1
        //BITSET(HwPORTCFG0, HwPORTCFG0_EINT_3);//SD0를 GPIO로 전환
        //HwPORTCFG2|=0xff;//SD0를 GPIO로 전환
        BITCSET (HwPORTCFG1, HwPORTCFG1_HP0_3, HwPORTCFG1_HP0_1);   //SD1 enable
        BITCSET (HwPORTCFG2, HwPORTCFG2_SDISEL_3, HwPORTCFG2_SDISEL_1);
#endif

#ifdef TCC78X_SDMMC_CH2_USE
        //SD/MMC Channel 2
        BITSET (HwPORTCFG0, HwPORTCFG0_EINT_3);
        HwPORTCFG2 |= 0xff;
        BITCSET (HwPORTCFG3, HwPORTCFG3_HDD1_3, HwPORTCFG3_HDD1_2);
        BITCSET (HwPORTCFG2, HwPORTCFG2_SDISEL_3, HwPORTCFG2_SDISEL_2);
#endif

        BITCLR (HwSDIDCTRL, (HwSDIDCTRL_WB_4 | HwSDIDCTRL_WB_8));   // 1bit

        HwSDIDCTRL |= (4 << 8);     //FCNT(1word=4bytes);

        HwSWRESET |= HwSWRESET_SD_ON;
        TC_TimeDly (1);
        HwSWRESET &= ~HwSWRESET_SD_ON;

        HwSDIIENABLE = 0x1FF;

        if (g_SDIOResponse[moduleidx].BWMODE == MODE_4BIT)
        {
            //HwSDICLK = 0x00001002;
            //HwPCK_SDMMC = 0x1100001f; // 4bit test    // good 8.4m
            //HwPCK_SDMMC = 0x11000012; // 4bit test    // good 14.2m
            //HwPCK_SDMMC = 0x11000013; // 4bit test    // good 13.5m
            HwPCK_SDMMC = 0x11000015;
        }
        else
        {
            //HwPCK_SDMMC = 0x11000008; // 30Mh - DVBH OK
            HwPCK_SDMMC = 0x11000009;
        }

        iArg = 0;
        iArg = 0xFF8000;
        TC3X_IO_SDIO_SENDCMD5 (moduleidx, iArg);       //in order to get IO OCR
        iArg = 0x00;
        TC3X_IO_SDIO_SENDCMD3 (moduleidx, iArg);       //in order to get IO RCA
        iArg = iArg | (g_SDIOResponse[moduleidx].resp6.RCA << 16);
        iArg = 0x10000;
        TC3X_IO_SDIO_SENDCMD7 (iArg);       //set new RCA value

        if (g_SDIOResponse[moduleidx].BWMODE == MODE_4BIT)
        {
            //CCCR func0 : 0x07 - bus width(0x07)-0,1bit : 0 :1bit, 2:4bit
            TC3X_IO_SDIO_SENDCMD52 (moduleidx, SDIO_FUNC0, SDIO_READ, 0x07, 0x00);
            data = g_SDIOResponse[moduleidx].resp5.data;
            BITCSET (data, 0x03, 0x02);
            TC3X_IO_SDIO_SENDCMD52 (moduleidx, SDIO_FUNC0, SDIO_WRITE, 0x07, data);
            SetBW_4BIT ();
        }

#elif defined TCC79X
        BITCSET (HwCPDRV0, HwCPDRV0_HPCTRL (3), HwCPDRV0_HPCTRL (3));       // strength 3
#endif
    break;
    
    case 1:
    	PRINTF_LV_0("Please insert your code\n");
    break;
    
    default:
    break;
    }



    return TC3X_IO_SUCCESS;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_SDIO_Close
//    Description : Close SDIO
//    Input : 
//      moduleidx : module index
//    Output : 
//      TC3X_IO_SUCCESS
//    Remark : 
//--------------------------------------------------------------------------
TC3X_IO_Err TC3X_IO_SDIO_Close(int moduleidx)
{
#if defined TCC79X
        sdio_unregister_driver (&gsdio_client_TC3X[moduleidx].driver);
#endif
    return TC3X_IO_SUCCESS;
}

//=======================================================================================
//
//  Read / Write API
//
//=======================================================================================

void TC3X_IO_SDIO_Func0_Write (int moduleidx, unsigned int RegAddr, unsigned char Data, int InnerSem)
{
    int       err_ret;
    int       semret;


    switch(moduleidx)
    {
    case 0:
    break;
    case 1:
    	PRINTF_LV_0("Please insert your code\n");
    break;
    default:
    break;
    }

    if (!InnerSem)
    {
        semret = TC3X_IO_IF_LOCK (moduleidx);
    }

#if defined TCC78X
#elif defined TCC79X
    sdio_f0_writeb (gsdio_client_TC3X[moduleidx].pFunc, Data, RegAddr, &err_ret);  // byte write

    if (err_ret)
    {
        PRINTF_LV_0 ("# IO_SDIO_Func0_Write error[%x]\n", err_ret);
    }

    if (!InnerSem)
    {
        TC3X_IO_IF_UnLOCK (moduleidx);
    }

#endif
}

unsigned char TC3X_IO_SDIO_Func0_Read (int moduleidx, unsigned int RegAddr, int InnerSem)
{
    int       err_ret;
    unsigned char iData;

    int       semret;

    if (!InnerSem)
    {
        semret = TC3X_IO_IF_LOCK (moduleidx);
    }

#if defined TCC78X
#elif defined TCC79X
    iData = sdio_f0_readb (gsdio_client_TC3X[moduleidx].pFunc, RegAddr, &err_ret); // byte read

    if (err_ret)
    {
        PRINTF_LV_0 ("# IO_SDIO_Func0_Read error[%x]\n", err_ret);
        if (!InnerSem)
        {
            TC3X_IO_IF_UnLOCK (moduleidx);
        }
        return 0;
    }
    else
    {
        if (!InnerSem)
        {
            TC3X_IO_IF_UnLOCK (moduleidx);
        }
        return iData;
    }

    if (!InnerSem)
    {
        TC3X_IO_IF_UnLOCK (moduleidx);
    }
#endif
}

// use semaphore
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_SDIO_Reg_Read
//    Description : Reg read
//    Input : 
//      moduleidx : module index
//      ChipAddr : chip address
//      RegAddr : Register Address
//      pError : Error flag
//    Output : 
//      TC3X_IO_SUCCESS
//    Remark : 
//--------------------------------------------------------------------------
unsigned int TC3X_IO_SDIO_Reg_Read (int moduleidx, int ChipAddr, int RegAddr, TC3X_IO_Err * pError)    // use 52cmd
{
    unsigned int retv;
    int       semret;
    semret = TC3X_IO_IF_LOCK (moduleidx);

#if defined TCC78X
    TC3X_IO_SDIO_SENDCMD52 (moduleidx, SDIO_FUNC1, SDIO_READ, (unsigned int) RegAddr, 0x00);
    retv = g_SDIOResponse[moduleidx].resp5.data;
#elif defined TCC79X
    retv = TC3X_IO_SDIO_SENDCMD52 (moduleidx, SDIO_FUNC1, SDIO_READ, (unsigned int) RegAddr, 0x00);
#endif

    TC3X_IO_IF_UnLOCK (moduleidx);
    return retv;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_SDIO_Reg_Write
//    Description : Reg write
//    Input : 
//      moduleidx : module index
//      ChipAddr : chip address
//      RegAddr : Register Address
//      data : data
//    Output : 
//      TC3X_IO_SUCCESS
//    Remark : 
//--------------------------------------------------------------------------
TC3X_IO_Err TC3X_IO_SDIO_Reg_Write (int moduleidx, int ChipAddr, int RegAddr, unsigned int data)       // use 52cmd
{
    unsigned char inputdata;
    int       semret;
    semret = TC3X_IO_IF_LOCK (moduleidx);

    inputdata = (data & 0xff);
    TC3X_IO_SDIO_SENDCMD52 (moduleidx, SDIO_FUNC1, SDIO_WRITE, RegAddr, inputdata);
    TC3X_IO_IF_UnLOCK (moduleidx);

    return TC3X_IO_SUCCESS;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_SDIO_Reg_ReadEx
//    Description : Reg multi-read
//    Input : 
//      moduleidx : module index
//      ChipAddr : chip address
//      RegAddr : Register Address
//      data : data pointer
//      iSize : size
//    Output : 
//      TC3X_IO_SUCCESS
//    Remark : 
//--------------------------------------------------------------------------
TC3X_IO_Err TC3X_IO_SDIO_Reg_ReadEx (int moduleidx, int ChipAddr, int RegAddr, unsigned char *data, int iSize) // use 53cmd
{
    unsigned int c512;
    unsigned int cremain;
    unsigned int i;
    int       semret;
    c512 = (iSize / 512);
    cremain = (iSize % 512);

    semret = TC3X_IO_IF_LOCK (moduleidx);

    for (i = 0; i < c512; i++)
    {
        TC3X_IO_SDIO_READDATA (moduleidx, SDIO_FUNC1, &data[i * 512], (unsigned int) RegAddr, SDIO_FIXEDADDR, 512);
    }
    if (cremain)
    {
        TC3X_IO_SDIO_READDATA (moduleidx, SDIO_FUNC1, &data[i * 512], (unsigned int) RegAddr, SDIO_FIXEDADDR, cremain);
    }
    TC3X_IO_IF_UnLOCK (moduleidx);

    return TC3X_IO_SUCCESS;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_SDIO_Reg_WriteEx
//    Description : Register multi-write
//    Input : 
//      moduleidx : module index
//      ChipAddr : chip address
//      RegAddr : Register Address
//      pCh : data pointer
//      iSize : size
//    Output : 
//      TC3X_IO_SUCCESS
//    Remark : 
//--------------------------------------------------------------------------
TC3X_IO_Err TC3X_IO_SDIO_Reg_WriteEx (int moduleidx, int ChipAddr, int RegAddr, unsigned char *pCh, int iSize) // use 53cmd
{
    unsigned int c512;
    unsigned int cremain;
    unsigned int i;
    int       semret;

    c512 = (iSize / 512);
    cremain = (iSize % 512);

    semret = TC3X_IO_IF_LOCK (moduleidx);

    for (i = 0; i < c512; i++)
    {
        TC3X_IO_SDIO_WRITEDATA (moduleidx, SDIO_FUNC1, &pCh[i * 512], (unsigned int) RegAddr, SDIO_FIXEDADDR, 512);
    }
    if (cremain)
    {
        TC3X_IO_SDIO_WRITEDATA (moduleidx, SDIO_FUNC1, &pCh[i * 512], (unsigned int) RegAddr, SDIO_FIXEDADDR, cremain);
    }
    TC3X_IO_IF_UnLOCK (moduleidx);
    return TC3X_IO_SUCCESS;
}

// not use semaphore

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_SDIO_Reg_Read_InnerSem
//    Description : Reg read (inside semaphore)
//    Input : 
//      moduleidx : module index
//      ChipAddr : chip address
//      RegAddr : Register Address
//      pError : Error flag
//    Output : 
//      TC3X_IO_SUCCESS
//    Remark : 
//--------------------------------------------------------------------------
unsigned int TC3X_IO_SDIO_Reg_Read_InnerSem (int moduleidx, int ChipAddr, int RegAddr, TC3X_IO_Err * pError)
{
    unsigned int retv;

#if defined TCC78X
    TC3X_IO_SDIO_SENDCMD52 (moduleidx, SDIO_FUNC1, SDIO_READ, (unsigned int) RegAddr, 0x00);
    retv = g_SDIOResponse[moduleidx].resp5.data;
#elif defined TCC79X
    retv = TC3X_IO_SDIO_SENDCMD52 (moduleidx, SDIO_FUNC1, SDIO_READ, (unsigned int) RegAddr, 0x00);
#endif

    return retv;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_SDIO_Reg_Write_InnerSem
//    Description : Reg write (inside semaphore)
//    Input : 
//      moduleidx : module index
//      ChipAddr : chip address
//      RegAddr : Register Address
//      pCh : data
//      iSize : size
//    Output : 
//      TC3X_IO_SUCCESS
//    Remark : 
//--------------------------------------------------------------------------
TC3X_IO_Err TC3X_IO_SDIO_Reg_Write_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned int data)
{
    unsigned char inputdata;

    inputdata = (data & 0xff);
    TC3X_IO_SDIO_SENDCMD52 (moduleidx, SDIO_FUNC1, SDIO_WRITE, RegAddr, inputdata);

    return TC3X_IO_SUCCESS;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_SDIO_Reg_ReadEx_InnerSem
//    Description : Reg multi-read (inside semaphore)
//    Input : 
//      moduleidx : module index
//      ChipAddr : chip address
//      RegAddr : Register Address
//      data : data pointer
//      iSize : size
//    Output : 
//      TC3X_IO_SUCCESS
//    Remark : 
//--------------------------------------------------------------------------
TC3X_IO_Err TC3X_IO_SDIO_Reg_ReadEx_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned char *data, int iSize)
{
    unsigned int c512;
    unsigned int cremain;
    unsigned int i;

    c512 = (iSize / 512);
    cremain = (iSize % 512);
    for (i = 0; i < c512; i++)
    {
        TC3X_IO_SDIO_READDATA (moduleidx, SDIO_FUNC1, &data[i * 512], (unsigned int) RegAddr, SDIO_FIXEDADDR, 512);
    }
    if (cremain)
    {
        TC3X_IO_SDIO_READDATA (moduleidx, SDIO_FUNC1, &data[i * 512], (unsigned int) RegAddr, SDIO_FIXEDADDR, cremain);
    }
    return TC3X_IO_SUCCESS;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_SDIO_Reg_WriteEx_InnerSem
//    Description : Reg write (inside semaphore)
//    Input : 
//      moduleidx : module index
//      ChipAddr : chip address
//      RegAddr : Register Address
//      pCh : data
//      iSize : size
//    Output : 
//      TC3X_IO_SUCCESS
//    Remark : 
//--------------------------------------------------------------------------
TC3X_IO_Err TC3X_IO_SDIO_Reg_WriteEx_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned char *pCh, int iSize)
{
    unsigned int c512;
    unsigned int cremain;
    unsigned int i;

    c512 = (iSize / 512);
    cremain = (iSize % 512);
    for (i = 0; i < c512; i++)
    {
        TC3X_IO_SDIO_WRITEDATA (moduleidx, SDIO_FUNC1, &pCh[i * 512], (unsigned int) RegAddr, SDIO_FIXEDADDR, 512);
    }
    if (cremain)
    {
        TC3X_IO_SDIO_WRITEDATA (moduleidx, SDIO_FUNC1, &pCh[i * 512], (unsigned int) RegAddr, SDIO_FIXEDADDR, cremain);
    }
    return TC3X_IO_SUCCESS;
}

//=======================================================================================
//
//  SDIO - SD
//
//=======================================================================================

void SetBW_1BIT ()
{
#ifdef TCC78X
#if 0
#ifdef _BOARD_VERSION_TCC7801_010_
    BITCSET (HwPORTCFG1, HwPORTCFG1_HP1_1 * 3, HwPORTCFG1_HP1_2);
    BITCSET (HwPORTCFG2, HwPORTCFG2_SDISEL_3, HwPORTCFG2_SDISEL_1);
#else
    BITCSET (HwPORTCFG0, HwPORTCFG0_EINT_3, HwPORTCFG0_EINT_1);
    BITCSET (HwPORTCFG2,
             (HwPORTCFG2_UART1_3 | HwPORTCFG2_UARTC1_3) | HwPORTCFG2_SDISEL_3,
             (HwPORTCFG2_UART1_1 | HwPORTCFG2_UARTC1_1) | HwPORTCFG2_SDISEL_0);
#endif
#endif

#ifdef TCC78X_SDMMC_CH0_USE
    // SD/MMC Channel 0
    BITCSET (HwPORTCFG0, HwPORTCFG0_EINT_3, HwPORTCFG0_EINT_1);
    BITCSET (HwPORTCFG2, (HwPORTCFG2_UART1_3 | HwPORTCFG2_UARTC1_3), (HwPORTCFG2_UART1_1 | HwPORTCFG2_UARTC1_1));
    BITCSET (HwPORTCFG2, HwPORTCFG2_SDISEL_3, HwPORTCFG2_SDISEL_0);
#endif

#ifdef TCC78X_SDMMC_CH1_USE
    // SD/MMC Channel 1
    BITSET (HwPORTCFG0, HwPORTCFG0_EINT_3);     //SD0를 GPIO로 전환
    HwPORTCFG2 |= 0xff; //SD0를 GPIO로 전환
    BITCSET (HwPORTCFG1, HwPORTCFG1_HP0_3, HwPORTCFG1_HP0_1);   //SD1 enable
    BITCSET (HwPORTCFG2, HwPORTCFG2_SDISEL_3, HwPORTCFG2_SDISEL_1);
#endif

#ifdef TCC78X_SDMMC_CH2_USE
    //SD/MMC Channel 2
    BITSET (HwPORTCFG0, HwPORTCFG0_EINT_3);
    HwPORTCFG2 |= 0xff;
    BITCSET (HwPORTCFG3, HwPORTCFG3_HDD1_3, HwPORTCFG3_HDD1_2);
    BITCSET (HwPORTCFG2, HwPORTCFG2_SDISEL_3, HwPORTCFG2_SDISEL_2);
#endif
    BITCLR (HwSDIDCTRL, (HwSDIDCTRL_WB_4 | HwSDIDCTRL_WB_8));
#else
    PRINTF_LV_0 ("# Insert Code SetBW_1BIT\n");
#endif
}

void SetBW_4BIT ()                                         /* 79x 부분 추가할 것 */
{
#ifdef TCC78X
    BITCSET (HwSDIDCTRL, HwSDIDCTRL_WB_4 | HwSDIDCTRL_WB_8, HwSDIDCTRL_WB_4);
#else
    PRINTF_LV_0 ("# Insert Code SetBW_4BIT\n");
#endif
}

int TC3X_IO_SDIO_ChkResponse (tSDIO_RESPONSE_TYPE RspType) /* 79x 부분 추가할 것 */
{
    int       rcv;
#ifdef	TCC78X
    rcv = HwSDIRSPARGU0;        //RspType1,RspType1b,RspType3,RspType4,RspType5,RspType6
#endif
    return rcv;
}

int TC3X_IO_SDIO_ParseResponse (int moduleidx, int RespData, tSDIO_RESPONSE_TYPE type)
{
    switch (type)
    {
    case SDIO_RESPONSE_R1:
        break;

    case SDIO_RESPONSE_R4:
        g_SDIOResponse[moduleidx].resp4.C = (RespData & 0x80000000) ? 1 : 0;
        g_SDIOResponse[moduleidx].resp4.Num_Func = (RespData & 0x70000000) >> 28;
        g_SDIOResponse[moduleidx].resp4.MP = (RespData & 0x08000000) ? 1 : 0;
        g_SDIOResponse[moduleidx].resp4.OCR = (RespData & 0x00ffffff);
        break;

    case SDIO_RESPONSE_R5:
        g_SDIOResponse[moduleidx].resp5.RespFlag = (RespData & 0x0000ff00) >> 8;
        g_SDIOResponse[moduleidx].resp5.cState = (RespData & 0x00003000) >> 12;
        g_SDIOResponse[moduleidx].resp5.data = (RespData & 0xff);
        break;

    case SDIO_RESPONSE_R6:
        g_SDIOResponse[moduleidx].resp6.RCA = (RespData & 0xFFFF0000) >> 16;
        g_SDIOResponse[moduleidx].resp6.CardStatus = (RespData & 0x0000ffff);
        break;

    default:
        return 0;
    }
    return 1;
}

void SD_SendCommand (int moduleidx, int iRspType, int iIndex, int iArgument)
{
//      int       iTempCmd = 0x00000000;
//      int       iWR;
//      int       iCD;

#ifdef TCC78X
    if (iRspType == SDIO_RESPONSE_NO)
        iWR = 0;
    else
        iWR = 1;

    if (iIndex == SDIO_COMMAND_CMD53)
        iCD = 1;
    else
        iCD = 0;

    // Make Command Register
    iTempCmd = iTempCmd | (127 << 12) | Hw11 | (iRspType << 7) | (iWR << 6) | iCD << 10 | iIndex;
    SDMMC_WaitCmdReady ();
    HwSDIARGU = iArgument;
    HwSDICMD = iTempCmd;
    SDMMC_WaitCmdReady ();

    if (iRspType != SDIO_RESPONSE_NO)
    {
        TC3X_IO_SDIO_ParseResponse (moduleidx, TC3X_IO_SDIO_ChkResponse (iRspType), iRspType);
    }
#endif
}

void TC3X_IO_SDIO_SENDCMD0 (int moduleidx, int iArg)
{
#ifdef TCC78X
    HwSDIDCTRL2 = 0x00;
    SD_SendCommand (moduleidx, SDIO_RESPONSE_NO, SDIO_COMMAND_CMD0, iArg);
#else

#endif
}

void TC3X_IO_SDIO_SENDCMD3 (int moduleidx, int iArg)
{
#ifdef TCC78X
    HwSDIDCTRL2 = 0x00;
    SD_SendCommand (moduleidx, SDIO_RESPONSE_R6, SDIO_COMMAND_CMD3, iArg);
#endif
}

void TC3X_IO_SDIO_SENDCMD5 (int moduleidx, int iArg)
{
#ifdef TCC78X
    HwSDIDCTRL2 = 0x00;
    SD_SendCommand (moduleidx, SDIO_RESPONSE_R4, SDIO_COMMAND_CMD5, iArg);
#endif
}

void TC3X_IO_SDIO_SENDCMD7 (int moduleidx, int iArg)
{
#ifdef	TCC78X
    HwSDIDCTRL2 = 0x00;
    SD_SendCommand (moduleidx, SDIO_RESPONSE_R1, SDIO_COMMAND_CMD7, iArg);
#endif
}

//void TC3X_IO_SDIO_SENDCMD52 (unsigned int FuncNum, unsigned int RWFlag, unsigned int RegAddr, unsigned char Data)
unsigned char TC3X_IO_SDIO_SENDCMD52 (int moduleidx, unsigned int FuncNum, unsigned int RWFlag, unsigned int RegAddr, unsigned char Data)
{
    //int       iArg;
    int       err_ret;
    unsigned char iData;

#if defined TCC78X
    HwSDIDCTRL2 = 0x00;
    iArg = (RWFlag << 31 | FuncNum << 28 | RegAddr << 9 | Data);
    SD_SendCommand (SDIO_RESPONSE_R5, SDIO_COMMAND_CMD52, iArg);
#elif defined TCC79X

    if (RWFlag == SDIO_READ)
    {
        iData = sdio_readb (gsdio_client_TC3X[moduleidx].pFunc, RegAddr, &err_ret);        // byte read

        if (err_ret)
        {
            PRINTF_LV_0 ("# CMD52 read error[%x]\n", err_ret);
            return 0;   // failed
        }
        else
        {
            return iData;
        }
    }
    else
    {
        sdio_writeb (gsdio_client_TC3X[moduleidx].pFunc, Data, RegAddr, &err_ret); // byte write

        if (err_ret)
        {
            PRINTF_LV_0 ("# CMD52 write error[%x]\n", err_ret);
            return 0;   // failed
        }
    }
#endif
    return 0;
}

void TC3X_IO_SDIO_SENDCMD53 (int moduleidx, unsigned int FuncNum, unsigned int RWFlag, unsigned int RegAddr, unsigned int IncAddr, unsigned int DataLen)
{
#ifdef TCC78X
    int       iArg;
    unsigned int bcnt;
    unsigned int st;
    unsigned int mb;
    unsigned int dd;
    unsigned int mode;

    if (DataLen == 512)
        DataLen = 0;

    bcnt = DataLen;
    st = 0;
    mb = 0;
    if (RWFlag == SDIO_READ)
        dd = 0;
    else
        dd = 1;
    mode = 1;

    HwSDIDCTRL2 = (bcnt << 4 | st << 3 | mb << 2 | dd << 1 | mode);
    if (g_SDIOResponse[moduleidx].mode == MODE_1BIT)
        HwSDIDCTRL = (Hw12 | Hw8);
    else
        HwSDIDCTRL = (Hw12 | Hw8 | Hw2);

    iArg = (RWFlag << 31 | FuncNum << 28 | IncAddr << 26 | RegAddr << 9 | DataLen);
    SD_SendCommand (SDIO_RESPONSE_R5, SDIO_COMMAND_CMD53, iArg);
#endif
}

int TC3X_IO_SDIO_WaitDataReady (void)
{
#ifdef	TCC78X
    return TC3X_IO_SDIO_WaitCondition (1, HwSDISTATUS_DPR, 0);
#endif
    return 0;
}

int TC3X_IO_SDIO_WaitRspReceived (void)
{
#ifdef	TCC78X
    return TC3X_IO_SDIO_WaitCondition (1, HwSDISTATUS_CRE, HwSDISTATUS_CTO | HwSDISTATUS_CCF);
#else
#endif
    return 0;
}

void TC3X_IO_SDIO_ResetDataPath (void)
{
#ifdef TCC78X
    HwSDIDCTRL &= 0xFF001FFF;
#endif
}

int TC3X_IO_SDIO_READDATA (int moduleidx, unsigned int FuncNum, unsigned char *pData, unsigned int RegAddr, unsigned int IncAddr, unsigned int DataLen)
{
#if defined TCC78X

    TC3X_IO_SDIO_ResetDataPath ();
    TC3X_IO_SDIO_SENDCMD53 (FuncNum, SDIO_READ, RegAddr, IncAddr, DataLen);
    if (TC3X_IO_SDIO_WaitRspReceived () < 0)
    {
        PRINTF_LV_0 ("# [R]Cann't Receive Read-Resp.\n");
        return -1;
    }
    TC3X_IO_SDIO_Basic_Read (pData, DataLen);
    TC3X_IO_SDIO_WaitDataReady ();
    return 0;

#elif defined TCC79X

    if (IncAddr == SDIO_FIXEDADDR)      // fixed mode
    {
        sdio_readsb (gsdio_client_TC3X[moduleidx].pFunc, pData, RegAddr, DataLen);
    }
    else        // increment mode
    {
        sdio_memcpy_fromio (gsdio_client_TC3X[moduleidx].pFunc, pData, RegAddr, DataLen);
    }

#endif
    return 0;
}

int TC3X_IO_SDIO_WRITEDATA (int moduleidx, unsigned int FuncNum, unsigned char *pData, unsigned int RegAddr, unsigned int IncAddr, unsigned int DataLen)
{
#if defined TCC78X

    TC3X_IO_SDIO_ResetDataPath ();
    TC3X_IO_SDIO_SENDCMD53 (FuncNum, SDIO_WRITE, RegAddr, IncAddr, DataLen);
    if (TC3X_IO_SDIO_WaitRspReceived () < 0)
    {
        PRINTF_LV_0 ("# [W]Cann't Receive Read-Resp.\n");
        return -1;
    }
    TC3X_IO_SDIO_Basic_Write (pData, DataLen);
    TC3X_IO_SDIO_WaitDataReady ();
    return 0;

#elif defined TCC79X

    if (IncAddr == SDIO_FIXEDADDR)      // fixed mode
    {
        sdio_writesb (gsdio_client_TC3X[moduleidx].pFunc, RegAddr, pData, DataLen);
    }
    else        // increment mode
    {
        sdio_memcpy_toio (gsdio_client_TC3X[moduleidx].pFunc, RegAddr, pData, DataLen);
    }

#endif
    return 0;
}

int TC3X_IO_SDIO_WaitCondition (unsigned uCond, unsigned uWaitBitMask, unsigned uErrorBitMask)
{
#ifdef	TCC78X
    unsigned  uStatus;
    unsigned long long iTimeOut;
    unsigned long long ref1, ref2, diff;

    iTimeOut = TIMEOUT_VALUE * 1000000;
    ref1 = HwTC32MCNT;

    while (1)
    {
        uStatus = HwSDISTATUS;
        if (uCond == 1)
        {
            // Break if One of uWaitBitMask is set to 1.
            if (ISONE (uStatus, uWaitBitMask))
                return 1;
        }
        else
        {
            // Break if One of uWaitBitMask is set to 0.
            if ((uStatus & uWaitBitMask) != uWaitBitMask)
                return 1;
        }

        ref2 = HwTC32MCNT;
        if (ref2 < ref1)
            diff = (ref2 + 0x100000000LL) - ref1;
        else
            diff = ref2 - ref1;

        if (diff >= iTimeOut)
        {
            PRINTF_LV_0 ("# Error TimeOut (%x,%x)\n", uStatus, uWaitBitMask);
            return -1;
        }
    }
#else

#endif
    return 1;
}

int TC3X_IO_SDIO_WaitFifoFetchRqt (void)
{
    // Wait until FIFO Fetch Request is generated.
#ifdef	TCC78X
    return TC3X_IO_SDIO_WaitCondition (1, HwSDISTATUS_FFR, HwSDISTATUS_DTO | HwSDISTATUS_CTO | HwSDISTATUS_CCF);
#else
#endif
    return 0;
}

int TC3X_IO_SDIO_WaitFifoLoadRqt (void)
{
    // Wait until FIFO Load Request is generated.
#ifdef TCC78X
    return TC3X_IO_SDIO_WaitCondition (1, HwSDISTATUS_FE, HwSDISTATUS_CTO | HwSDISTATUS_CCF);
#endif
    return 0;
}

void TC3X_IO_SDIO_Basic_Write (unsigned char *cBuf, int iSize)
{

#ifdef TCC78X
    unsigned int hwStatus;
    int      *tmp = (int *) cBuf;
    int       res;

    while (iSize)
    {
        res = TC3X_IO_SDIO_WaitFifoLoadRqt ();
        if (res < 0)
        {
            PRINTF_LV_0 ("# Err_FIFOLoadRqt\n");
            break;
        }
        iSize -= 4;
        HwSDIWDATA = *tmp++;

        /*
         * hwStatus = HwSDISTATUS;
         * if(hwStatus&HwSDISTATUS_FE) {
         * iSize-=4;
         * HwSDIWDATA = *tmp++;
         * } */
    }

    hwStatus = HwSDISTATUS;
    if (hwStatus & Hw1)
    {
        PRINTF_LV_0 ("# WCRCErr[%08x]\n", hwStatus);
    }

    hwStatus = HwSDIIFLAG;
    if (hwStatus & Hw1)
    {
        PRINTF_LV_0 ("# WErr[%08x]\n", hwStatus);
    }
    HwSDIIFLAG = 0x1FF;
#endif
}

void TC3X_IO_SDIO_Basic_Read (unsigned char *cBuf, int iSize)
{

#ifdef TCC78X
    unsigned int hwStatus;
    int      *tmp = (int *) cBuf;
    int       res;

    while (iSize)
    {
        res = TC3X_IO_SDIO_WaitFifoFetchRqt ();
        if (res < 0)
        {
            PRINTF_LV_0 ("# Err_FIFOFetchRqt\n");
            break;
        }
        *tmp++ = HwSDIRDATA;
        iSize -= 4;

        /*
         * hwStatus = HwSDISTATUS;
         * if(hwStatus&HwSDISTATUS_FFR) {
         * *tmp++ = HwSDIRDATA;
         * iSize -= 4;
         * }
         */
    }

    hwStatus = HwSDISTATUS;
    if (hwStatus & Hw2)
    {
        PRINTF_LV_0 ("# RCRCErr[%08x]\n", hwStatus);
    }

    hwStatus = HwSDIIFLAG;
    if (hwStatus & Hw2)
    {
        PRINTF_LV_0 ("# RErr[%08x]\n", hwStatus);
    }
    HwSDIIFLAG = 0x1FF;
#endif
}
#endif // USE_IF_SDIO
