/****************************************************************************
 *   FileName    : tcc317x_monitoring_calculate.h
 *   Description : Sample source for monitoring
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

#ifndef __TCC317X_MONITORING_CALCULATE_H__
#define __TCC317X_MONITORING_CALCULATE_H__

#include "tcc317x_common.h"
#include "tcc317x_monitoring.h"

void Tcc317xCalculateInitValues(Tcc317xStatus_t * _dMBStatData);
I32U Tcc317xCalculateSnr (Tcc317xStatus_t * _dMBStatData);
I32U Tcc317xCalculatePcber (I32U _inPcber);
I32U Tcc317xCalculateViterbiber (Tcc317xStatus_t * _dMBStatData, I32U oldViterbiber);
I32U Tcc317xCalculateTsper (Tcc317xStatus_t * _dMBStatData, I32U oldTsper);
I32S Tcc317xCalculateRssi (Tcc317xStatus_t * _dmbStatusData, I32U _frequency);
I32U Tcc317xGetUnsignedMovingAvg (I32U *_Array, I32U _input, I32U _count, I32S _maxArray);
I32S Tcc317xGetSignedMovingAvg (I32S *_Array, I32S _input, I32S _count, I32S _maxArray);

#endif
