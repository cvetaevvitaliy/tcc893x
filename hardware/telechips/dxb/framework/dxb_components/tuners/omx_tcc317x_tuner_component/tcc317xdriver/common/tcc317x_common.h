/****************************************************************************
 *   FileName    : tcc317x_common.h
 *   Description : common defines
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 

This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-
distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall 
constitute any express or implied warranty of any kind, including without limitation, any warranty 
of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright 
or other third party intellectual property right. No warranty is made, express or implied, 
regarding the information's accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of 
or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement 
between Telechips and Company.
*
****************************************************************************/

#ifndef __TCC317X_COMMON_H__
#define __TCC317X_COMMON_H__

#include "tcpal_types.h"

/* Caution : Do not modify this File (Share with Driver Core Library) */

/* max 4diversity or max dual baseband */
#define TCC317X_MAX             4
#define TCC317X_DIVERSITY_MAX   4

/* set interrupt threshold */
#define TCC317X_STREAM_THRESHOLD                2048

typedef enum
{
    TCC317X_TCC3170 = 0,
    TCC317X_TCC3171,
    TCC317X_CHIP_SELECTION_MAX
} EnumTcc317xChipSelection;

typedef enum
{
    /* if use non-chain mode, please select TCC317X_BOARD_SINGLE */
    TCC317X_BOARD_SINGLE = 0,

    TCC317X_BOARD_DUAL_CHAINMODE,
    TCC317X_BOARD_TRIPLE_CHAINMODE,

    TCC317X_BOARD_2DIVERSITY,
    TCC317X_BOARD_3DIVERSITY,
    TCC317X_BOARD_4DIVERSITY,
    TCC317X_BOARD_MAX
} EnumTcc317xBoardType;

typedef enum
{
    TCC317X_DIVERSITY_NONE = 0,
    TCC317X_DIVERSITY_MASTER,
    TCC317X_DIVERSITY_MID,
    TCC317X_DIVERSITY_LAST
} EnumTcc317xDiversityPosition;

typedef struct
{
    I08U      streamDataConfig0;
    I08U      streamDataConfig1;
    I08U      streamDataConfig2;
    I08U      streamDataConfig3;
    I08U      gpioAltH;
    I08U      gpioAltL;
    I08U      gpioDrH;
    I08U      gpioDrL;
    I08U      gpioLrH;
    I08U      gpioLrL;
    I08U      gpioDrvH;
    I08U      gpioDrvL;
    I08U      gpiopeH;
    I08U      gpiopeL;
    I08U      divio;
    I08U      irqMode;
    I08U      periConfig0;
    I08U      periConfig1;
    I08U      periConfig2;
    I08U      periConfig3;
    I08U      periConfig4;
    I08U      smxConfig0;
    I08U      bufferConfig0;
    I08U      bufferConfig1;
    I08U      bufferConfig30;
    I08U      bufferConfig31;
} Tcc317xRegisterConfig_t;

typedef struct
{
    /* Chip Selection */
    I08S      chipSelection;
    /* board type */
    I08S      boardType;
    /* select command interface */
    I08S      commandInterface;
    /* select stream interface */
    I08S      streamInterface;
    /* changed i2c/tccspi address 0xa0-> 0xXX */
    I08U      address;

    /* pll, oscKhz 
    OSC : 19200 , PLL : 0x0F06
    OSC : 24576 , PLL : 0x180E
    OSC : 38400 , PLL : 0x0F0E
    */

    /* pll option - 0x0F06 */
    I16U      pll;
    /* osc clk*/
    I32U      oscKhz;

    I08S      useInterrupt;

    I16S      powersave;
    I16S      diversityPosition;

    I16S      useLBAND;
    Tcc317xRegisterConfig_t config;
} Tcc317xOption_t;

typedef enum
{
    TCC317X_NOT_USE_SRAMLIKE = 0,       /* can't support yet */
    TCC317X_IF_I2C,
    TCC317X_IF_TCCSPI,
    TCC317X_NOT_USE_SDIO1BIT,           /* can't support yet */
    TCC317X_NOT_USE_SDIO4BIT            /* can't support yet */
} EnumCommandIo;

typedef enum
{
    TCC317X_STREAM_IO_MAINIO = 0,
    TCC317X_STREAM_IO_PTS,              /* can't support yet */
    TCC317X_STREAM_IO_STS,
    TCC317X_STREAM_IO_SPIMS,
    TCC317X_STREAM_IO_SPISLV,           /* can't support yet */
    TCC317X_STREAM_IO_HPI_HEADERON,     /* can't support yet */
    TCC317X_STREAM_IO_HPI_HEADEROFF,    /* can't support yet */
    TCC317X_STREAM_IO_MAX
} EnumStreamIo;

typedef enum
{
    TCC317X_RETURN_FAIL_NULL_ACCESS = -4,
    TCC317X_RETURN_FAIL_UNKNOWN = -3,
    TCC317X_RETURN_FAIL_TIMEOUT = -2,
    TCC317X_RETURN_FAIL = -1,
    TCC317X_RETURN_SUCCESS = 0,
    TCC317X_RETURN_FIRST
} EnumReturn;

typedef enum
{
    TCC317X_EVENT_COMMAND_READ_FAIL = 0,
    TCC317X_EVENT_COMMAND_WRITE_FAIL,
    TCC317X_EVENT_MAILBOX_FAIL,
    TCC317X_EVENT_UNKNOWN
} EnumCallBackEvent;

typedef struct
{
    I32S      serviceType;          /*2 bit*/
    I32S      pType;                /*2 bit*/
    I32S      identifier;           /*6bit identifier - ex)subchannel id */
    I32S      cuSize;               /*10 bit*/
    I32S      startCu;              /*10 bit*/
    I32S      reConfig;             /*2 bit*/
    I32S      rsOn;
    I32S      pLevel;               /*3 bit*/
    I32S      bitrate;              /*8 bit*/
} Tcc317xService;

typedef struct
{
    I08U      AGC;
    I08U      CTO;
    I08U      CFO;
    I08U      FTO;
    I08U      SYNC_BYTE;
    I08U      OFDM_DETECT;
} DmbLock_t;

typedef enum
{
    DATAMODE_DAB = 0,
    DATAMODE_DMB,
    DATAMODE_EPM,
    DATAMODE_DABPLUS
} TDMBDataMode;

typedef struct
{
    I32U cmd;
    I32U word_cnt;
    I32U status;
    I32U data_array[7];
} mailbox_t;

/*
#define _TCC317X_SUPPORT_AUTO_SEARCHING_	// disabled
*/
#if defined (_TCC317X_SUPPORT_AUTO_SEARCHING_)
#define MAX_AUTOSEARCH_BAND3_IDX		41
#define MAX_AUTOSEARCH_LBAND_IDX		23
#define MAX_AUTOSEARCH_KBAND_IDX		21
																			
#define AUTOSEARCH_TDMB_OFDMDETECT_LOCK		(10000)
#define AUTOSEARCH_TDMB_OFDMDETECT_RETRY		(1)
#define BANDIII_MAXDATA0				32
#define BANDIII_MAXDATA1				9
#define LBAND_MAXDATA0					23
#define KOREA_MAXDATA0					21

#define SELECT_BAND3					1
#define SELECT_LBAND					2
#define SELECT_KOREA					3

typedef struct
{	
	I32U Lockfrequency_Band3[MAX_AUTOSEARCH_BAND3_IDX];
	I32U Lockfrequency_Lband[MAX_AUTOSEARCH_LBAND_IDX];
	I32U Lockfrequency_Kband[MAX_AUTOSEARCH_KBAND_IDX];
	I08U LockSucessCount;	
}Tcc317xAutoSearch_t;
#endif

/*--------------------------------------------------------------------------*/
/* TDMB Service Types-------------------------------------------------------*/

#define SRVTYPE_NONE                0x00
#define SRVTYPE_DAB                 0x01
#define SRVTYPE_DABPLUS             0x02
#define SRVTYPE_DATA                0x03
#define SRVTYPE_DMB                 0x04
#define SRVTYPE_FIDC                0x05
#define SRVTYPE_FIC                 0x06
#define SRVTYPE_FIC_WITH_ERR        0x07
#define EWS_ANNOUNCE_FLAG                   0x08
#define RECONFIGURATION_FLAG                0x09
#define EWS_ANNOUNCE_RECONFIGURATION_FLAG   0x0a
#define SRVTYPE_STATUS                 0x0b

#endif
