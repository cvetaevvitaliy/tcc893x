/****************************************************************************
 *   FileName    : tcc317x_stream_process.h
 *   Description : Sample source for stream process
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

#ifndef __TCC317X_STREAM_PROCESS_H__
#define __TCC317X_STREAM_PROCESS_H__

#include "tcc317x_common.h"

#define _SIZE_STATDATA_			32
#define _SIZE_STATDATA_WITH_TII_	64
#define _SIZE_FICHEADER_		4

typedef enum
{
	ENUM_FIND_EP_SYNC = 0,
	ENUM_FIND_OTHER_CHIP,
	ENUM_STATUS_GET,
	ENUM_FIC_GET,
	ENUM_MSC_GET,
	ENUM_DATA_GET_S
} enum_DABParsingStat;

typedef struct
{
    I08U epSyncByte;
    I08U epHeaderData[4];
    I08U ficHeader[_SIZE_FICHEADER_];
    /*Stream Type 0xFD :other chip / 0xBF : Status  / 0x40 : FIC / 0x44 : dummy data for special uses / 0x00~3F : MSC Data */
    I08U streamType; 
    I32U identifier; /* 6bit identifier - ex) subchannel id */
    I32S streamSize;
    I08U parity;
} Tcc317xStreamHeader;

typedef struct
{
    I32S parserState;
    I32S dataSize;
    I32S currDataSize;
    Tcc317xStreamHeader streamHeader;

    I08U flagFig5_2;
    I08U flagFig0_19;
    I08U flagFig0_0;
    I08U flagFig0_0Change;

    I08U flagFig0_0Alarm;
    I08U reserved0;
    I08U reserved1;
    I08U reserved2;

    I08U statusData[_SIZE_STATDATA_WITH_TII_];

    I32U ficCrcResult;

    I32U OtherChipModuleSize;
} Tcc317xStreamCtrl_t;

void Tcc317xStreamParserInit(I32S _moduleIndex);
void Tcc317xStreamParsing(I32S _moduleIndex, I08U *_data, I32S _size);
void Tcc317xStreamResult(I32S _moduleIndex, I08U *_data, I32S _size, I32S _types, I32U _reservedParams1, I32U _reservedParams2);
void Tcc317xAndroidStreamResult (I32S _moduleIndex, I08U *_data, I32S _size, I32S _types, I32U _reservedParams1, I32U _reservedParams2);

#endif
