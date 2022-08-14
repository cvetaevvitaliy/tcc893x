/****************************************************************************
 *   FileName    : tcc317x_core.h
 *   Description : core Function
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

#ifndef __TCC317X_CORE_H__
#define __TCC317X_CORE_H__

#include "tcc317x_defines.h"
#include "tcc317x_common.h"

#define TCC317X_DEFAULT_ADDRESS     0xA0

#define DP_CFG_OPSET1				(1144)
#define DP_CFG_OPSET2				(204)

#define DP_CFG_1_DATA0				(0x8000d000)
#define DP_CFG_2_DATA0				(DP_CFG_1_DATA0 + DP_CFG_OPSET1)
#define DP_CFG_1_DATA1				(DP_CFG_2_DATA0 + DP_CFG_OPSET2)
#define DP_CFG_2_DATA1				(DP_CFG_1_DATA1 + DP_CFG_OPSET1)
#define DP_CFG_1_DATA2				(DP_CFG_2_DATA1 + DP_CFG_OPSET2)
#define DP_CFG_2_DATA2				(DP_CFG_1_DATA2 + DP_CFG_OPSET1)
#define DP_CFG_1_DATA3				(DP_CFG_2_DATA2 + DP_CFG_OPSET2)
#define DP_CFG_2_DATA3				(DP_CFG_1_DATA3 + DP_CFG_OPSET1)

typedef struct
{
    I08U     *coldbootDataPtr;
    I32U      coldbootDataSize;
    I08U     *daguDataPtr;
    I32U      daguDataSize;
    I08U     *dintDataPtr;
    I32U      dintDataSize;
    I08U     *randDataPtr;
    I32U      randDataSize;
    I08U     *colOrderDataPtr;
    I32U      colOrderDataSize;
} Tcc317xBoot_t;

I32S      Tcc317xOpen (I32S _moduleIndex, Tcc317xOption_t * _Tcc317xOption, func_v_iii _eventCallBackFunc);
I32S      Tcc317xClose (I32S _moduleIndex);
I32S      Tcc317xInit (I32S _moduleIndex, I08U * _coldbootData, I32S _codeSize);
I32S      Tcc317xTune (I32S _moduleIndex, I32S _frequency, I32U * _reservedOption);
I32S      Tcc317xMailboxWrite (I32S _moduleIndex, I32S _diversityIndex, I32U _command, I32U *dataArray, I32S wordSize);
I32S      Tcc317xMailboxRead (I32S _moduleIndex, I32S _diversityIndex, I32U _command, mailbox_t * _mailbox);
I32S      Tcc317xStreamStop (I32S _moduleIndex);
I32S      Tcc317xStreamStart (I32S _moduleIndex);
I32S      Tcc317xAttach (I32S _moduleIndex, Tcc317xOption_t * _Tcc317xOption, func_v_iii _eventCallBackFunc);
I32S      Tcc317xDetach (I32S _moduleIndex);
I32S      Tcc317xInitBroadcasting (I32S _moduleIndex, I08U * _coldbootData, I32S _codeSize);
I32S      Tcc317xInit_DiversityNormalMode(I32S _moduleIndex, I32S _diversityIndex, I08U * _coldbootData, I32S _codeSize);
I32S      Tcc317xInitBroadcasting_DiversityNormalMode (I32S _moduleIndex, 
			     I32S _diversityIndex, I08U * _coldbootData,  I32S _codeSize);
I32S      DummyFunction0 (I32S _moduleIndex, I32S _chipAddress, I08U _inputData, I08U * _outData, I32S _size);
I32S      DummyFunction1 (I32S _moduleIndex, I32S _chipAddress, I08U _address, I08U * _inputData, I32S _size);
I32U      Tcc317xWarmBoot (I32S _moduleIndex, I32S _diversityIndex,  I32S _sendStartMail);
I32U      Tcc317xSendStartMail (Tcc317xHandle_t * _handle);
I32U      Tcc317xGetCoreVersion (void);
I32U      Tcc317xGetDspCodeVersion(I32S _moduleIndex, I32S _diversityIndex);
void      Tcc317xSetAgcTableVhf (Tcc317xHandle_t * _handle);
void      Tcc317xSetAgcTableLband (Tcc317xHandle_t * _handle);
void      Tcc317xSetFpConfig (Tcc317xHandle_t * _handle);
void      Tcc317xSetFpIirCoefficient (Tcc317xHandle_t * _handle, I32S freq_khz);
void      Tcc317xPeripheralOnOff (Tcc317xHandle_t * _handle, I32S _onoff);

/* AutoSearch */
#if defined (_TCC317X_SUPPORT_AUTO_SEARCHING_)
I32S      Tcc317xAutoSearch(I32S _moduleIndex, I32S _diversityIndex, I32U _BandSelect, Tcc317xAutoSearch_t *_Lockstate);
#endif

extern    I32S Tcc317xApiChannelSelect(I32S _moduleIndex, I32S _frequency, I32U *_reservedOption);
extern    I32S Tcc317xApiRegisterWrite(I32S _moduleIndex, I32S _diversityIndex, I08U _address, I08U *_data, I32U _size);
extern    I32S Tcc317xApiRegisterRead(I32S _moduleIndex, I32S _diversityIndex, I08U _address, I08U *_data, I32U _size);
#endif
