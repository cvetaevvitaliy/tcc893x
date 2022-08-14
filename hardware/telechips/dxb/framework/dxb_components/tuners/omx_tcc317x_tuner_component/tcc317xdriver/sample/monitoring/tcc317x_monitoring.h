/****************************************************************************
 *   FileName    : tcc317x_monitoring.h
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

#ifndef __TCC317X_MONITORING_H__
#define __TCC317X_MONITORING_H__

#include "tcc317x_common.h"

#define TDMB_MAX_MOV_AVG     32

typedef struct
{
    /* common */
    I32U currentValue;
    I32U avgValue;
    I32U count;
    I32U array[TDMB_MAX_MOV_AVG];
    I32U oldValue;
} Tcc317xStatusUnsignedValueSub_t;

typedef struct
{
    /* common */
    I32S currentValue;
    I32S avgValue;
    I32S count;
    I32S array[TDMB_MAX_MOV_AVG];
    I32S oldValue;
} Tcc317xStatusSignedValueSub_t;


typedef struct
{
    /* status value */
    /* RSSI */
    Tcc317xStatusSignedValueSub_t rssi;
    /* PCBER */
    Tcc317xStatusUnsignedValueSub_t pcber;
    /* FIC PCBER */
    Tcc317xStatusUnsignedValueSub_t ficPcber;
    /* MSC PCBER */
    Tcc317xStatusUnsignedValueSub_t mscPcber;
    /* SNR */
    Tcc317xStatusUnsignedValueSub_t snrDb;
    /* VITERBI BER */
    Tcc317xStatusUnsignedValueSub_t viterbiber;
    /* TSPER */
    Tcc317xStatusUnsignedValueSub_t tsper;
} Tcc317xStatusValue_t;

typedef struct
{
    /* received status */
    I32U syncStatus;
    I08U rfLoopGain;
    I08U bbLoopGain;
    I32U prsSnr;
    I32U fic_pcber;
    I32U msc_pcber;
    I32U pcber;		/* msc ber==0 -> pcber is fic_ber else, pcber is msc ber */
    I32U rsOverCount;
    I32U rsPacketCount;
    I64U rsErrorCount;

    /* status value */
    Tcc317xStatusValue_t status;

    /* calculate status */
    I64U oldRsErrorCount;
    I32U oldRsOverCount;
    I32U oldRsPacketCount;
    I32U packetResynced;
    I16U reserved0;
} Tcc317xStatus_t;

typedef struct
{
	/* TII value */
	/* Main ID */
	I08U MainID[8];
	/* Sub ID */
	I08U SubID[8];
	/* TII Max Energy */
	I16U MaxEnergy[8];
} Tcc317xTiiValue_t;

I32S Tcc317xMonitoringInit (I32S _moduleIndex, I32S _diversityIndex);
I32S Update_TDMB_Status (I32S _moduleIndex, I32S _diversityIndex, Tcc317xStatus_t * pDMBStatData);
I32S Tcc317xParseStatusData (I32S _moduleIndex, I08U * stat, I32S _size);
void Tcc317xResetMonitoringValues (I32S _moduleIndex, I32S _diversityIndex);
void Tcc317xGetSignalState(I32S _moduleIndex, I32S _diversityIndex, Tcc317xStatus_t *pStatus);

#endif
