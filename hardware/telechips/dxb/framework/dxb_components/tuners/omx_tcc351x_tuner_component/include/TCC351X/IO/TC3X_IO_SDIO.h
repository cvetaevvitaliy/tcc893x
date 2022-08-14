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

#ifndef __TC3X_IO_SDIO_H__
#define __TC3X_IO_SDIO_H__

#include "../TC3X_Common.h"

#if defined (USE_IF_SDIO)
typedef enum
{
    SDIO_READ = 0,
    SDIO_WRITE
} tSDIO_RWFlag;

typedef enum
{
    SDIO_FUNC0 = 0,
    SDIO_FUNC1
} tSDIO_FUNCNUM;

typedef enum
{
    SDIO_FIXEDADDR = 0,
    SDIO_INCADDR
} tSDIO_ADDRMODE;

typedef enum
{
    SDIO_COMMAND_CMD0 = 0,
    SDIO_COMMAND_CMD3 = 3,
    SDIO_COMMAND_CMD5 = 5,
    SDIO_COMMAND_CMD7 = 7,
    SDIO_COMMAND_CMD15 = 15,
    SDIO_COMMAND_CMD52 = 52,
    SDIO_COMMAND_CMD53 = 53,
    SDIO_COMMAND_CMD59 = 59
} tSDIO_COMMAND_INDEX;

typedef enum
{
    SDIO_RESPONSE_NO = 0,
    SDIO_RESPONSE_R1 = 1,
    SDIO_RESPONSE_R4 = 5,
    SDIO_RESPONSE_R5 = 6,
    SDIO_RESPONSE_R6 = 7
} tSDIO_RESPONSE_TYPE;

typedef enum
{
    SDIO_MODE_SD1,                                         //SD-1bit mode
    SDIO_MODE_SD4,                                         //SD-4bit mode
    SDIO_MODE_SPI_USE_GPSB,
    SDIO_MODE_SPI_USE_GSIO
} tSDIO_MODE;

typedef enum
{
    TC3X_SDIO_FAIL = 0,
    TC3X_SDIO_SUCCESS,
    TC3X_SDIO_OUTOFRANGE
} TC3X_SDIOErr;

typedef enum
{
    SDIO_CURRENT_DIS,                                      //initialize, standby and inactive state(card not selected)
    SDIO_CURRENT_CMD,                                      // DAT lines free
    SDIO_CURRENT_TRN,                                      //Transfer
    SDIO_CURRENT_RFU                                       //reserved for future
} tSDIO_CURRENT_STATE;

typedef enum
{
    SDIO_BSM_INI,                                          //initialization state
    SDIO_BSM_STB,                                          //standby state
    SDIO_BSM_CMD,                                          //command state
    SDIO_BSM_TRN,                                          //transfer state
    SDIO_BSM_IACT                                          //inactive state
} tSDIO_BSM;                                               //Bus State Machine

typedef enum
{
    SDIO_BUS_FREE,
    SDIO_BUS_ACTIVE
} tSDIO_BUS;

typedef struct _tagSDIORESPRCV
{
    int       respdata;
    int       cmdidx;
} SDIORESPRCV;

typedef struct _tagSDIORESPONSE4
{
    char      C;
    char      Num_Func;
    char      MP;
    int       OCR;
} SDIORESPONSE4, *PSDIORESPONSE4;

typedef struct _tagSDIORESPONSE5
{
    char      RespFlag;
    char      data;
    tSDIO_CURRENT_STATE cState;
} SDIORESPONSE5, *PSDIORESPONSE5;

typedef struct _tagSDIORESPONSE6
{
    short     RCA;
    short     CardStatus;
} SDIORESPONSE6, *PSDIORESPONSE6;

typedef struct _tagSDIORESPONSE
{
    int       BWMODE;
    SDIORESPONSE4 resp4;
    SDIORESPONSE5 resp5;
    SDIORESPONSE6 resp6;
    tSDIO_MODE mode;
    tSDIO_BUS bus;
    tSDIO_BSM bsm;
    char      spi_status;
    char      spi_data;
} SDIORESPONSE, *PSDIORESPONSE;

typedef struct _tagCMD53FORMAT
{
    int       fn;
    int       bm;
    int       op;
    int       addr;
    int       count;
} CMD53FORMAT, *PCMD53FORMAT;

typedef struct _tagSDIOCCCR
{
    char      sdio_rev;
    char      cccr_rev;
    char      sd_spec;
} SDIOCCCRINFO, *PSDIOCCCRINFO;

void      TC3X_IO_SDIO_VariablesInit (int moduleidx);
TC3X_IO_Err TC3X_IO_SDIO_Init (int moduleidx, unsigned int BWMODE);
TC3X_IO_Err TC3X_IO_SDIO_Close(int moduleidx);
unsigned int TC3X_IO_SDIO_Reg_Read (int moduleidx, int ChipAddr, int RegAddr, TC3X_IO_Err * pError);   // use 52cmd
TC3X_IO_Err TC3X_IO_SDIO_Reg_Write (int moduleidx, int ChipAddr, int RegAddr, unsigned int data);      // use 52cmd
TC3X_IO_Err TC3X_IO_SDIO_Reg_ReadEx (int moduleidx, int ChipAddr, int RegAddr, unsigned char *data, int iSize);        // use 53cmd
TC3X_IO_Err TC3X_IO_SDIO_Reg_WriteEx (int moduleidx, int ChipAddr, int RegAddr, unsigned char *pCh, int iSize);        // use 53cmd
unsigned int TC3X_IO_SDIO_Reg_Read_InnerSem (int moduleidx, int ChipAddr, int RegAddr, TC3X_IO_Err * pError);
TC3X_IO_Err TC3X_IO_SDIO_Reg_Write_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned int data);
TC3X_IO_Err TC3X_IO_SDIO_Reg_ReadEx_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned char *data, int iSize);
TC3X_IO_Err TC3X_IO_SDIO_Reg_WriteEx_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned char *pCh, int iSize);
void      SetBW_1BIT (void);

void      SetBW_4BIT (void);
int       TC3X_IO_SDIO_ChkResponse (tSDIO_RESPONSE_TYPE RspType);
int       TC3X_IO_SDIO_ParseResponse (int moduleidx, int RespData, tSDIO_RESPONSE_TYPE type);
void      SD_SendCommand (int moduleidx, int iRspType, int iIndex, int iArgument);
void      TC3X_IO_SDIO_SENDCMD0 (int moduleidx, int iArg);
void      TC3X_IO_SDIO_SENDCMD3 (int moduleidx, int iArg);
void      TC3X_IO_SDIO_SENDCMD5 (int moduleidx, int iArg);
void      TC3X_IO_SDIO_SENDCMD7 (int moduleidx, int iArg);
//void TC3X_IO_SDIO_SENDCMD52 (unsigned int FuncNum, unsigned int RWFlag, unsigned int RegAddr, unsigned char Data);
unsigned char TC3X_IO_SDIO_SENDCMD52 (int moduleidx, unsigned int FuncNum, unsigned int RWFlag, unsigned int RegAddr, unsigned char Data);
void      TC3X_IO_SDIO_SENDCMD53 (int moduleidx, unsigned int FuncNum, unsigned int RWFlag, unsigned int RegAddr, unsigned int IncAddr,
                                  unsigned int DataLen);
int       TC3X_IO_SDIO_WaitDataReady (void);
int       TC3X_IO_SDIO_WaitRspReceived (void);
void      TC3X_IO_SDIO_ResetDataPath (void);
int       TC3X_IO_SDIO_READDATA (int moduleidx, unsigned int FuncNum, unsigned char *pData, unsigned int RegAddr, unsigned int IncAddr,
                                 unsigned int DataLen);
int       TC3X_IO_SDIO_WRITEDATA (int moduleidx, unsigned int FuncNum, unsigned char *pData, unsigned int RegAddr, unsigned int IncAddr,
                                  unsigned int DataLen);
int       TC3X_IO_SDIO_WaitCondition (unsigned uCond, unsigned uWaitBitMask, unsigned uErrorBitMask);
int       TC3X_IO_SDIO_WaitFifoFetchRqt (void);
int       TC3X_IO_SDIO_WaitFifoLoadRqt (void);
void      TC3X_IO_SDIO_Basic_Write (unsigned char *cBuf, int iSize);
void      TC3X_IO_SDIO_Basic_Read (unsigned char *cBuf, int iSize);

void      TC3X_IO_SDIO_Func0_Write (int moduleidx, unsigned int RegAddr, unsigned char Data, int InnerSem);
unsigned char TC3X_IO_SDIO_Func0_Read (int moduleidx, unsigned int RegAddr, int InnerSem);
void      TC3X_IO_SDIO_INT_Handler0 (void);
void      TC3X_IO_SDIO_INT_Handler1 (void);
#endif // USE_IF_SDIO
#endif
