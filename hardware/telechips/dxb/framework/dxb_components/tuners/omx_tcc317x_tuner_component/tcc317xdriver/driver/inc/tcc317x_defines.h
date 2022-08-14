/****************************************************************************
 *   FileName    : tcc317x_defines.h
 *   Description : defines 
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

#ifndef __TCC317X_DEFINES_H__
#define __TCC317X_DEFINES_H__

#include "tcc317x_common.h"

/* single run version (Default) */
#define ENABLE_GARBAGE_STREAM

/* CODE Memory Setting */
#define	TCC3170_START_PC		(0x0000)
#define	TCC3170_START_PC_opset		(0x8000)
#define	TC317X_CODEMEMBASE		(0x80000000+TCC3170_START_PC)
#define	TC317X_CODEMEMBASE_8000 	(0x80000000+TCC3170_START_PC_opset)
#define	TC317X_CODE_TABLEBASE_RAND 	(0xF0020000)
#define	TC317X_CODE_TABLEBASE_DINT 	(0xF0024000)
#define	TC317X_CODE_TABLEBASE_DAGU 	(0xF0028000)
#define	TC317X_CODE_TABLEBASE_COL_ORDER (0xF002C000)

#define PHY_BASE_ADDR               	(0x80000000)

/* total 16kbyte */
#define PHY_MEM_ADDR_A_START		(PHY_BASE_ADDR + 0x0000c000)    /* over size of ASM 8K */
#define PHY_MEM_ADDR_A_END		(PHY_BASE_ADDR + 0x0000cfff)
#define PHY_MEM_ADDR_B_START		(PHY_BASE_ADDR + 0x0000d000)
#define PHY_MEM_ADDR_B_END		(PHY_BASE_ADDR + 0x0000cfff)
#define PHY_MEM_ADDR_C_START		(PHY_BASE_ADDR + 0x0000d000)
#define PHY_MEM_ADDR_C_END		(PHY_BASE_ADDR + 0x0000cfff)
#define PHY_MEM_ADDR_D_START		(PHY_BASE_ADDR + 0x0000d000)
#define PHY_MEM_ADDR_D_END		(PHY_BASE_ADDR + 0x0000dfff)

typedef struct Tcc317xHandle_t
{
    I08U      moduleIndex;
    I08U      diversityIndex;
    I08U      currentAddress;
    I08U      sysEnValue;

    I08U      asmDownloaded;

    I08U      mailboxLog;
    I08U      mailboxErrorCounter;
    I08U      reserved2;

    I32U      dspCodeVersion;
    I32U      mainClkKhz;
    Tcc317xOption_t options;

    I32U      currentBand; /* 0 : VHF, 1 :UHF */
    I32U      opened;
    I32S (*Read) (I32S _moduleIndex, I32S _chipAddress, I08U  _registerAddr, I08U * _outData, I32S _size);
    I32S (*Write) (I32S _moduleIndex, I32S _chipAddress, I08U  _registerAddr, I08U * _outData, I32S _size);
    void (*EventCallBack) (I32S _moduleIndex, I32S _chipAddress, I32S  _callBackEvent);
} Tcc317xHandle_t;

typedef enum
{
    MAILBOX_LOG_NONE = 0,
    MAILBOX_LOG_FREQSET_PREPARE,
    MAILBOX_LOG_FREQSET_START,
    MAILBOX_LOG_FREQSET_START_STEP1,
    MAILBOX_LOG_FREQSET_START_STEP2,
    MAILBOX_LOG_FREQSET_STOP,
    MAILBOX_LOG_SERVICE
} ENUM_MAILBOX_LOG;

#endif
