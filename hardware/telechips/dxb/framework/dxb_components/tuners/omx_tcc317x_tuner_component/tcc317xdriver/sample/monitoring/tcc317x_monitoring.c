/****************************************************************************
 *   FileName    : tcc317x_monitoring.c
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

#include "tcc317x_api.h"
#include "tcc317x_monitoring.h"
#include "tcc317x_monitoring_calculate.h"
#include "tcc317x_stream_process.h"
#include "tcpal_os.h"

Tcc317xStatus_t Tcc317xStatus[TCC317X_MAX][TCC317X_MAX];
Tcc317xTiiValue_t Tcc317xTiiValues[TCC317X_MAX][TCC317X_MAX];

/* can read only master */
I32S Tcc317xMonitoringInit (I32S _moduleIndex, I32S _diversityIndex)
{
    TcpalMemset(&Tcc317xStatus[_moduleIndex][_diversityIndex], 0x00, sizeof(Tcc317xStatus_t));
    TcpalMemset(&Tcc317xTiiValues[_moduleIndex][_diversityIndex], 0x00, sizeof(Tcc317xTiiValue_t));
    Tcc317xResetMonitoringValues (_moduleIndex, _diversityIndex);
    return TCC317X_RETURN_SUCCESS;
}

void Tcc317xResetMonitoringValues (I32S _moduleIndex, I32S _diversityIndex)
{
    Tcc317xCalculateInitValues(&Tcc317xStatus[_moduleIndex][_diversityIndex]);
}

I32S Update_TDMB_Status (I32S _moduleIndex, I32S _diversityIndex, Tcc317xStatus_t * pDMBStatData)
{
    I32U frequency;
    pDMBStatData->oldRsPacketCount = Tcc317xStatus[_moduleIndex][_diversityIndex].oldRsPacketCount;
    pDMBStatData->oldRsErrorCount = Tcc317xStatus[_moduleIndex][_diversityIndex].oldRsErrorCount;
    pDMBStatData->oldRsOverCount = Tcc317xStatus[_moduleIndex][_diversityIndex].oldRsOverCount;
    pDMBStatData->packetResynced = Tcc317xStatus[_moduleIndex][_diversityIndex].packetResynced;

    if (pDMBStatData->rsPacketCount == pDMBStatData->oldRsPacketCount)
    {
        /* same as previouse */
    }
    else
    {
        pDMBStatData->packetResynced = 0;
        if (pDMBStatData->rsPacketCount == 0)
        {
            if (pDMBStatData->oldRsPacketCount)
            {
                pDMBStatData->packetResynced = 1;
            }
        }
        else if (pDMBStatData->rsPacketCount != 0 && pDMBStatData->rsPacketCount < pDMBStatData->oldRsPacketCount)
        {
            if (pDMBStatData->oldRsPacketCount < 0x80000000)
            {
                /* auto resync situation */
                pDMBStatData->packetResynced = 1;
            }
            else
            {
                /* rotate */
                pDMBStatData->oldRsPacketCount = 0;
                pDMBStatData->oldRsErrorCount = 0;
                pDMBStatData->oldRsOverCount = 0;
            }
        }
    }

    /* Update SNR Set */
    if (Tcc317xStatus[_moduleIndex][_diversityIndex].status.snrDb.count < TDMB_MAX_MOV_AVG)
        Tcc317xStatus[_moduleIndex][_diversityIndex].status.snrDb.count++;

    Tcc317xStatus[_moduleIndex][_diversityIndex].status.snrDb.currentValue = Tcc317xCalculateSnr (pDMBStatData);
    Tcc317xStatus[_moduleIndex][_diversityIndex].status.snrDb.avgValue =
        Tcc317xGetUnsignedMovingAvg (Tcc317xStatus[_moduleIndex][_diversityIndex].status.snrDb.array,
                             Tcc317xStatus[_moduleIndex][_diversityIndex].status.snrDb.currentValue,
                             Tcc317xStatus[_moduleIndex][_diversityIndex].status.snrDb.count, TDMB_MAX_MOV_AVG);
   
    /* Update fic pcber Set */
    if (Tcc317xStatus[_moduleIndex][_diversityIndex].status.ficPcber.count < TDMB_MAX_MOV_AVG)
        Tcc317xStatus[_moduleIndex][_diversityIndex].status.ficPcber.count++;

    Tcc317xStatus[_moduleIndex][_diversityIndex].status.ficPcber.currentValue = Tcc317xCalculatePcber (pDMBStatData->fic_pcber);
    Tcc317xStatus[_moduleIndex][_diversityIndex].status.ficPcber.avgValue =
        Tcc317xGetUnsignedMovingAvg (Tcc317xStatus[_moduleIndex][_diversityIndex].status.ficPcber.array,
                             Tcc317xStatus[_moduleIndex][_diversityIndex].status.ficPcber.currentValue,
                             Tcc317xStatus[_moduleIndex][_diversityIndex].status.ficPcber.count, TDMB_MAX_MOV_AVG);

    /* Update msc pcber Set */
    if (Tcc317xStatus[_moduleIndex][_diversityIndex].status.mscPcber.count < TDMB_MAX_MOV_AVG)
        Tcc317xStatus[_moduleIndex][_diversityIndex].status.mscPcber.count++;

    Tcc317xStatus[_moduleIndex][_diversityIndex].status.mscPcber.currentValue = Tcc317xCalculatePcber (pDMBStatData->msc_pcber);
    Tcc317xStatus[_moduleIndex][_diversityIndex].status.mscPcber.avgValue =
        Tcc317xGetUnsignedMovingAvg (Tcc317xStatus[_moduleIndex][_diversityIndex].status.mscPcber.array,
                             Tcc317xStatus[_moduleIndex][_diversityIndex].status.mscPcber.currentValue,
                             Tcc317xStatus[_moduleIndex][_diversityIndex].status.mscPcber.count, TDMB_MAX_MOV_AVG);

    /* Update pcber Set */
    if (Tcc317xStatus[_moduleIndex][_diversityIndex].status.pcber.count < TDMB_MAX_MOV_AVG)
        Tcc317xStatus[_moduleIndex][_diversityIndex].status.pcber.count++;
    Tcc317xStatus[_moduleIndex][_diversityIndex].status.pcber.currentValue = Tcc317xCalculatePcber (pDMBStatData->pcber);
    Tcc317xStatus[_moduleIndex][_diversityIndex].status.pcber.avgValue =
        Tcc317xGetUnsignedMovingAvg (Tcc317xStatus[_moduleIndex][_diversityIndex].status.pcber.array,
                             Tcc317xStatus[_moduleIndex][_diversityIndex].status.pcber.currentValue,
                             Tcc317xStatus[_moduleIndex][_diversityIndex].status.pcber.count, TDMB_MAX_MOV_AVG);

    /* Update VITERBIBER Set */
    if (Tcc317xStatus[_moduleIndex][_diversityIndex].status.viterbiber.count < TDMB_MAX_MOV_AVG)
        Tcc317xStatus[_moduleIndex][_diversityIndex].status.viterbiber.count++;

    Tcc317xStatus[_moduleIndex][_diversityIndex].status.viterbiber.currentValue = 
        Tcc317xCalculateViterbiber (pDMBStatData, Tcc317xStatus[_moduleIndex][_diversityIndex].status.viterbiber.oldValue);
    Tcc317xStatus[_moduleIndex][_diversityIndex].status.viterbiber.avgValue =
        Tcc317xGetUnsignedMovingAvg (Tcc317xStatus[_moduleIndex][_diversityIndex].status.viterbiber.array,
                             Tcc317xStatus[_moduleIndex][_diversityIndex].status.viterbiber.currentValue,
                             Tcc317xStatus[_moduleIndex][_diversityIndex].status.viterbiber.count, TDMB_MAX_MOV_AVG);

    /* Update TSPER Set */
    if (Tcc317xStatus[_moduleIndex][_diversityIndex].status.tsper.count < TDMB_MAX_MOV_AVG)
        Tcc317xStatus[_moduleIndex][_diversityIndex].status.tsper.count++;

    Tcc317xStatus[_moduleIndex][_diversityIndex].status.tsper.currentValue = 
        Tcc317xCalculateTsper (pDMBStatData, Tcc317xStatus[_moduleIndex][_diversityIndex].status.tsper.oldValue);
    Tcc317xStatus[_moduleIndex][_diversityIndex].status.tsper.avgValue =
        Tcc317xGetUnsignedMovingAvg (Tcc317xStatus[_moduleIndex][_diversityIndex].status.tsper.array,
                             Tcc317xStatus[_moduleIndex][_diversityIndex].status.tsper.currentValue,
                             Tcc317xStatus[_moduleIndex][_diversityIndex].status.tsper.count, TDMB_MAX_MOV_AVG);
    /* Update RSSI Set */
    if (Tcc317xStatus[_moduleIndex][_diversityIndex].status.rssi.count < TDMB_MAX_MOV_AVG)
        Tcc317xStatus[_moduleIndex][_diversityIndex].status.rssi.count++;

    frequency = Tcc317xApiGetCurrentFrequency(_moduleIndex);

    Tcc317xStatus[_moduleIndex][_diversityIndex].status.rssi.currentValue = Tcc317xCalculateRssi (pDMBStatData, frequency);
    Tcc317xStatus[_moduleIndex][_diversityIndex].status.rssi.avgValue =
        Tcc317xGetSignedMovingAvg (Tcc317xStatus[_moduleIndex][_diversityIndex].status.rssi.array,
                             Tcc317xStatus[_moduleIndex][_diversityIndex].status.rssi.currentValue,
                             Tcc317xStatus[_moduleIndex][_diversityIndex].status.rssi.count, TDMB_MAX_MOV_AVG);

    pDMBStatData->oldRsPacketCount = pDMBStatData->rsPacketCount;
    pDMBStatData->oldRsErrorCount = pDMBStatData->rsErrorCount;
    pDMBStatData->oldRsOverCount = pDMBStatData->rsOverCount;

    TcpalMemcpy (&pDMBStatData->status, &Tcc317xStatus[_moduleIndex][_diversityIndex].status, sizeof (Tcc317xStatusValue_t));
    TcpalMemcpy (&Tcc317xStatus[_moduleIndex][_diversityIndex], pDMBStatData, sizeof (Tcc317xStatus_t));

    return 0;
}

I32S Tcc317xParseStatusData (I32S _moduleIndex, I08U * stat, I32S _size)
{
#define OFFSET_RF_LOOP_GAIN         4
#define OFFSET_BB_LOOP_GAIN         5
#define OFFSET_PRS_SNR              8
#define OFFSET_PCBER                12
#define OFFSET_RS_PACKET_COUNT      16
#define OFFSET_RS_OVER_COUNT        20
#define OFFSET_RS_ERROR_COUNT_LO    24
#define OFFSET_RS_ERROR_COUNT_HI    28

    Tcc317xStatus_t tmpDMBStatData;
    I64U temp;
    I32U temp32;
    I16U high16;
    I16U low16;
    I32U i;

    if (_size < _SIZE_STATDATA_)
    {
        TcpalPrintErr ((I08S*)"# [E]InputSizeErr[%d]\n", _size);
        return TCC317X_RETURN_FAIL;
    }

    tmpDMBStatData.syncStatus = *((I32U *) stat);
    tmpDMBStatData.rfLoopGain = stat[OFFSET_RF_LOOP_GAIN];
    tmpDMBStatData.bbLoopGain = stat[OFFSET_BB_LOOP_GAIN];

    tmpDMBStatData.prsSnr = *((I32U *) (stat + OFFSET_PRS_SNR));

    temp32 = *((I32U *) (stat + OFFSET_PCBER));
    high16 = (I16U)((temp32>>16) & 0xFFFF);
    low16 = (I16U)(temp32 & 0xFFFF);

    tmpDMBStatData.msc_pcber = (I32U)(low16);
    tmpDMBStatData.fic_pcber = (I32U)(high16);

    if(tmpDMBStatData.msc_pcber > 0xFFFD)
        tmpDMBStatData.pcber = tmpDMBStatData.fic_pcber;
    else 
        tmpDMBStatData.pcber = tmpDMBStatData.msc_pcber;

    tmpDMBStatData.rsOverCount = *((I32U *) (stat + OFFSET_RS_OVER_COUNT));
    tmpDMBStatData.rsPacketCount = *((I32U *) (stat + OFFSET_RS_PACKET_COUNT));
    tmpDMBStatData.rsErrorCount = *((I32U *) (stat + OFFSET_RS_ERROR_COUNT_LO));
    temp = *((I32U *) (stat + OFFSET_RS_ERROR_COUNT_HI));
    tmpDMBStatData.rsErrorCount |= ((I64U)(temp)<<32);

    if(tmpDMBStatData.pcber > 0xFFFD )
    {
        Tcc317xResetMonitoringValues (_moduleIndex, 0);
    } 
    else
    {
        Update_TDMB_Status (_moduleIndex, 0, &tmpDMBStatData);
    }


    if(_size>=_SIZE_STATDATA_WITH_TII_) {
        Tcc317xTiiValue_t *tii;
	tii = (Tcc317xTiiValue_t *)(&Tcc317xTiiValues[_moduleIndex][0]);

        for (i=0; i<8 ; i++) {
            temp32 = *((I32U *) (stat + _SIZE_STATDATA_ + (i<<2)));
            tii->MainID[i] = (I08U)((temp32>>21)&0x7F);
            tii->SubID[i] = (I08U)((temp32>>16)&0x1F);
            tii->MaxEnergy[i] = (I16U)((temp32)&0xFFFF);
        }
    }

    return TCC317X_RETURN_SUCCESS;
}

void Tcc317xGetSignalState(I32S _moduleIndex, I32S _diversityIndex, Tcc317xStatus_t *pStatus)
{
	DmbLock_t lock;

	if(_diversityIndex) {
		mailbox_t mailbox;
		I32U frequency;

		Tcc317xApiMailboxRead (_moduleIndex, _diversityIndex, ((0x00<<11) | 0x02), &mailbox);
		Tcc317xStatus[_moduleIndex][_diversityIndex].syncStatus = mailbox.data_array[0];

		Tcc317xApiMailboxRead (_moduleIndex, _diversityIndex, ((0x01<<11) | 0x01), &mailbox);
		Tcc317xStatus[_moduleIndex][_diversityIndex].rfLoopGain = (mailbox.data_array[0] & 0xFF);
		Tcc317xStatus[_moduleIndex][_diversityIndex].bbLoopGain = ((mailbox.data_array[0]>>8) & 0xFF);

		Tcc317xApiMailboxRead (_moduleIndex, _diversityIndex, ((0x0A<<11) | 0x00), &mailbox);
		Tcc317xStatus[_moduleIndex][_diversityIndex].prsSnr = mailbox.data_array[2];

		/* Update RSSI Set */
		if (Tcc317xStatus[_moduleIndex][_diversityIndex].status.rssi.count < TDMB_MAX_MOV_AVG)
		    Tcc317xStatus[_moduleIndex][_diversityIndex].status.rssi.count++;
		
		frequency = Tcc317xApiGetCurrentFrequency(_moduleIndex);
		
		Tcc317xStatus[_moduleIndex][_diversityIndex].status.rssi.currentValue = 
		    Tcc317xCalculateRssi (&Tcc317xStatus[_moduleIndex][_diversityIndex], frequency);
		Tcc317xStatus[_moduleIndex][_diversityIndex].status.rssi.avgValue =
		    Tcc317xGetSignedMovingAvg (Tcc317xStatus[_moduleIndex][_diversityIndex].status.rssi.array,
					 Tcc317xStatus[_moduleIndex][_diversityIndex].status.rssi.currentValue,
					 Tcc317xStatus[_moduleIndex][_diversityIndex].status.rssi.count, TDMB_MAX_MOV_AVG);

		/* Update SNR Set */
		if (Tcc317xStatus[_moduleIndex][_diversityIndex].status.snrDb.count < TDMB_MAX_MOV_AVG)
		    Tcc317xStatus[_moduleIndex][_diversityIndex].status.snrDb.count++;
		
		Tcc317xStatus[_moduleIndex][_diversityIndex].status.snrDb.currentValue = 
		    Tcc317xCalculateSnr (&Tcc317xStatus[_moduleIndex][_diversityIndex]);
		Tcc317xStatus[_moduleIndex][_diversityIndex].status.snrDb.avgValue =
		    Tcc317xGetUnsignedMovingAvg (Tcc317xStatus[_moduleIndex][_diversityIndex].status.snrDb.array,
					 Tcc317xStatus[_moduleIndex][_diversityIndex].status.snrDb.currentValue,
					 Tcc317xStatus[_moduleIndex][_diversityIndex].status.snrDb.count, TDMB_MAX_MOV_AVG);
	}

	Tcc317xApiLockStatus(_moduleIndex, _diversityIndex, &lock);
        if (!(lock.CTO && lock.CFO))
		Tcc317xResetMonitoringValues (_moduleIndex, _diversityIndex);

	pStatus->status.rssi.currentValue = Tcc317xStatus[_moduleIndex][_diversityIndex].status.rssi.currentValue;
	pStatus->status.rssi.avgValue = Tcc317xStatus[_moduleIndex][_diversityIndex].status.rssi.avgValue;
	pStatus->status.snrDb.currentValue = Tcc317xStatus[_moduleIndex][_diversityIndex].status.snrDb.currentValue;
	pStatus->status.snrDb.avgValue = Tcc317xStatus[_moduleIndex][_diversityIndex].status.snrDb.avgValue;
	pStatus->status.viterbiber.currentValue = Tcc317xStatus[_moduleIndex][_diversityIndex].status.viterbiber.currentValue;
	pStatus->status.viterbiber.avgValue = Tcc317xStatus[_moduleIndex][_diversityIndex].status.viterbiber.avgValue;
	pStatus->status.ficPcber.currentValue = Tcc317xStatus[_moduleIndex][_diversityIndex].status.ficPcber.currentValue;
	pStatus->status.ficPcber.avgValue = Tcc317xStatus[_moduleIndex][_diversityIndex].status.ficPcber.avgValue;
	pStatus->status.mscPcber.currentValue = Tcc317xStatus[_moduleIndex][_diversityIndex].status.mscPcber.currentValue;
	pStatus->status.mscPcber.avgValue = Tcc317xStatus[_moduleIndex][_diversityIndex].status.mscPcber.avgValue;
	pStatus->status.pcber.currentValue = Tcc317xStatus[_moduleIndex][_diversityIndex].status.pcber.currentValue;
	pStatus->status.pcber.avgValue = Tcc317xStatus[_moduleIndex][_diversityIndex].status.pcber.avgValue;
	pStatus->status.tsper.currentValue = Tcc317xStatus[_moduleIndex][_diversityIndex].status.tsper.currentValue;
	pStatus->status.tsper.avgValue = Tcc317xStatus[_moduleIndex][_diversityIndex].status.tsper.avgValue;
	pStatus->rfLoopGain = Tcc317xStatus[_moduleIndex][_diversityIndex].rfLoopGain;
	pStatus->bbLoopGain = Tcc317xStatus[_moduleIndex][_diversityIndex].bbLoopGain;
	pStatus->fic_pcber = Tcc317xStatus[_moduleIndex][_diversityIndex].status.ficPcber.currentValue;
	pStatus->msc_pcber = Tcc317xStatus[_moduleIndex][_diversityIndex].status.mscPcber.currentValue;
}

I32S Tcc317xGetTiiValues(I32S _moduleIndex, Tcc317xTiiValue_t *_tii)
{
	if(Tcc317xTiiValues[_moduleIndex][0].MaxEnergy[0] == 0 &&
	    Tcc317xTiiValues[_moduleIndex][0].MainID[0] == 0 &&
	    Tcc317xTiiValues[_moduleIndex][0].SubID[0] == 0)
        	return TCC317X_RETURN_FAIL;

	TcpalMemcpy (_tii, &Tcc317xTiiValues[_moduleIndex][0], sizeof(Tcc317xTiiValue_t));
	return TCC317X_RETURN_SUCCESS;
}
