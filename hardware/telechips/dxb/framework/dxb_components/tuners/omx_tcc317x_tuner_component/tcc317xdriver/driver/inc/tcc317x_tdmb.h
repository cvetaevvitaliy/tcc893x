/****************************************************************************
 *   FileName    : tcc317x_tdmb.h
 *   Description : tdmb Function
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

#ifndef __TCC317X_TDMB_H__
#define __TCC317X_TDMB_H__

#include "tcc317x_defines.h"

#define MAX_TDMBMSC_NUM     (6)
#define MAX_DMB_NUM         (3)

typedef struct
{
    I32S numberOfService;                           /* except FIC */
    Tcc317xService DmbService[MAX_TDMBMSC_NUM];
    I32U protocolData[MAX_TDMBMSC_NUM * 2];
    I08U onAirState[MAX_TDMBMSC_NUM];

    I08U OnAirServiceType[MAX_TDMBMSC_NUM];
    I08U onAirDMBChannelFlag[MAX_DMB_NUM];

    I32U dmbChannelId[MAX_TDMBMSC_NUM];
    I32U mciCount;
} TdmbControl_t;


void InitTdmbProcess (Tcc317xHandle_t *_handle);
I32S Tcc317xAddService(I32S _moduleIndex, Tcc317xService * _DmbService);
I32S Tcc317xRemoveService(I32S _moduleIndex, I32S _identifier);

#endif
