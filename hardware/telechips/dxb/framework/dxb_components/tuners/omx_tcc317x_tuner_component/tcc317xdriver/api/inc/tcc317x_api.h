/****************************************************************************
 *    FileName    : tcc317x_api.h
 *   Description : TCC317X API functions
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

#ifndef __TCC317X_API_H__
#define __TCC317X_API_H__

#include "tcc317x_common.h"

#define TDMB_OFDMDETECT_LOCK    (100)
#define TDMB_OFDMDETECT_RETRY   (1)
#define TDMB_CTO_LOCK           (100)
#define TDMB_CTO_RETRY          (3)
#define TDMB_CFO_LOCK           (20)
#define TDMB_CFO_RETRY          (3)
/* invalid min : 100 */
/* valid min : 220 */
/* worst : 760 */

typedef enum
{
    TCC317X_STATUS_NONE = 0,
    TCC317X_STATUS_OPEND,
    TCC317X_STATUS_INITED,
    TCC317X_STATUS_CLOSED
} EnumTcc317xStatus;

typedef struct
{
    I32U status;
    I32U frequency;
} Tcc317xApiControl_t;

I32S Tcc317xApiOpen (I32S _moduleIndex, Tcc317xOption_t *_Tcc317xOption, I32S _optionSize);
I32S Tcc317xApiInit (I32S _moduleIndex, I08U *_coldbootData, I32S _codeSize);
I32S Tcc317xApiInit_DiversityNormalMode (I32S _moduleIndex, I32S _diversityIndex, I08U *_coldbootData, I32S _codeSize);
I32S Tcc317xApiClose(I32S _moduleIndex);
I32S Tcc317xApiChannelSearch(I32S _moduleIndex, I32S _frequency, I32U *_reservedOption);
I32S Tcc317xApiChannelSelect(I32S _moduleIndex, I32S _frequency, I32U *_reservedOption);
I32S Tcc317xApiRegisterRead(I32S _moduleIndex, I32S _diversityIndex, I08U _address, I08U *_data, I32U _size);
I32S Tcc317xApiRegisterWrite(I32S _moduleIndex, I32S _diversityIndex, I08U _address, I08U *_data, I32U _size);
I32S Tcc317xApiRfRegisterRead(I32S _moduleIndex, I32S _diversityIndex, I08U _address, I32U *_data);
I32S Tcc317xApiRfRegisterWrite(I32S _moduleIndex, I32S _diversityIndex, I08U _address, I32U _data);
I32S Tcc317xApiAddService(I32S _moduleIndex, Tcc317xService * _service);
I32S Tcc317xApiRemoveService(I32S _moduleIndex, I32S _identifier);
I32S Tcc317xApiStreamStop(I32S _moduleIndex);
I32S Tcc317xApiStreamStart(I32S _moduleIndex);
I32S Tcc317xApiStreamRead(I32S _moduleIndex, I08U *_data, I32S _size);
I32S Tcc317xApiMemoryWrite(I32S _moduleIndex, I32S _diversityIndex, I32U _address, I08U *_data, I32U _size);
I32S Tcc317xApiMemoryRead(I32S _moduleIndex, I32S _diversityIndex, I32U _address, I08U *_data, I32U _size);
I32S Tcc317xApiGetIrqError(I32S _moduleIndex, I08U *_data);
I32S Tcc317xApiIrqClear(I32S _moduleIndex);
I32S Tcc317xApiMailboxWrite (I32S _moduleIndex, I32S _diversityIndex, I32U _command, I32U *dataArray, I32S wordSize);
I32S Tcc317xApiMailboxRead (I32S _moduleIndex, I32S _diversityIndex, I32U _command, mailbox_t * _mailbox);
I32S Tcc317xApiWarmBoot (I32S _moduleIndex, I32S _diversityIndex);
I32S Tcc317xApiUserCommand(I32S _moduleIndex, I32S _diversityIndex, I32S _command, void *_param1, void *_param2, void *_param3, void *_param4);
I32S Tcc317xApiLockStatus(I32S _moduleIndex, I32S _diversityIndex, DmbLock_t *_tdmbLock);
I32S Tcc317xApiWaitLock(I32S _moduleIndex);
I32U Tcc317xApiGetCurrentFrequency(I32S _moduleIndex) ;
I32S Tcc317xApiGetDriverVersion(I32U *_version);
I32S Tcc317xApiGetDSPCodeVersion(I32S _moduleIndex, I32S _diversityIndex, I32U *_version);

#endif
