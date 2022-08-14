/****************************************************************************
 *   FileName    : tcc353x_monitoring.c
 *   Description : sample source for monitoring
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

#include "tcc353x_monitoring.h"
#include "tcc353x_monitoring_calculate.h"
#include "tcpal_os.h"

extern I32S Tcc353xApiRegisterRead(I32S _moduleIndex, I32S _diversityIndex,
				   I08U _address, I08U * _data,
				   I32U _size);
extern void Tcc353xApiParseIsdbSyncStat(IsdbLock_t * _isdbLock,
					I08U _input);
extern I32S Tcc353xApiMailboxRead(I32S _moduleIndex, I32S _diversityIndex,
				  I32U _command, mailbox_t * _mailbox);
static void Tcc353xUpdateMonitoringStatusBer(I32S _moduleIndex,
					     I32S _diversityIndex,
					     Tcc353xStatus_t *
					     pISDBStatData);
static void Tcc353xUpdateMonitoringStatus(I32S _moduleIndex,
					  I32S _diversityIndex,
					  Tcc353xStatus_t * pISDBStatData);
extern I32S Tcc353xApiOpStatusRead(I32S _moduleIndex, I32S _diversityIndex,
			   I32U _dataSize, I32U * _datas);
extern I32S Tcc353xApiDiversityCombineBlock(I32S _moduleIndex, 
					    I32S _diversityIndex, 
					    I32S _diversityUnlockEnforcement);

Tcc353xStatus_t Tcc353xStatus[TCC353X_MAX][TCC353X_DIVERSITY_MAX];
Tcc353xDivProc_Params Tcc353xDivParams[TCC353X_MAX];

/*---------------------------------------------------------------------
 * Function name
 * 	Tcc353xMonitoringApiInit
 * Description
 * 	Initializing monitoring status
 * Parameters
 * 	_moduleIndex : Index of module
 *		0 : BaseBand#0 (Single : default, Dual : BaseBand#0)
 * 		1 : BaseBand#1 (Single : Not use, Dual : BaseBand#1)
 *	_diversityIndex : Index of diversity
 *		0 : Diversity master
 *		1 : Diversity slave#1
 *		2 : Diversity slave#2
 *		3 : Diversity slave#3
 * Return value
 * 	Refer EnumReturn
 * Remark
 * 	please apply this function when channel tuned.
 ---------------------------------------------------------------------*/
I32S Tcc353xMonitoringApiInit(I32S _moduleIndex, I32S _diversityIndex)
{
	TcpalMemset(&Tcc353xStatus[_moduleIndex][_diversityIndex], 0x00,
		    sizeof(Tcc353xStatus_t));
	TcpalMemset(&Tcc353xDivParams[_moduleIndex], 0x00,
		    sizeof(Tcc353xDivProc_Params));
	return TCC353X_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------
 * Function name
 * 	Tcc353xMonitoringApiAntennaPercentage
 * Description
 * 	Get Antenna Bar Percentages
 * Parameters
 * 	_moduleIndex : Index of module
 *		0 : BaseBand#0 (Single : default, Dual : BaseBand#0)
 * 		1 : BaseBand#1 (Single : Not use, Dual : BaseBand#1)
 *      pISDBStatData : all bb's Status data pointer
 *      _InputSize : sizeof input status data
 * Return value
 * 	Refer EnumReturn
 * Remark
 * 	
 ---------------------------------------------------------------------*/
I32S Tcc353xMonitoringApiAntennaPercentage (I32S _moduleIndex, 
				Tcc353xStatus_t * pISDBStatData,
				I32U _InputSize)
{
	I32U i, j;
	I32U max;
	I32U lock = 0;
	max = (_InputSize/sizeof(Tcc353xStatus_t));

	for(i=0; i<max; i++)
		lock |= (pISDBStatData[i].status.isdbLock.TMCC & 0x01);

	if(lock == 0) {
		for(i=0; i<max; i++) {
			pISDBStatData[i].antennaPercent[0] = 0;
			pISDBStatData[i].antennaPercent[1] = 0;
			pISDBStatData[i].antennaPercent[2] = 0;
		}
		return TCC353X_RETURN_SUCCESS;
	}

	for(i=0; i<max; i++) {
		for (j = 0; j < 3; j++) {
			I32U vBer;
			I32U vPCBER;
			/* vber master only */
			vBer = pISDBStatData[0].status.viterbiber[j].avgValue;
			if(vBer>=VBER_ANTENNA_0)
				pISDBStatData[i].antennaPercent[j] = 0;
			else if(vBer>=VBER_ANTENNA_5)
				pISDBStatData[i].antennaPercent[j] = 5;
			else if(vBer>=VBER_ANTENNA_10)
				pISDBStatData[i].antennaPercent[j] = 10;
			else if(vBer>=VBER_ANTENNA_15)
				pISDBStatData[i].antennaPercent[j] = 15;
			else if(vBer>=VBER_ANTENNA_20)
				pISDBStatData[i].antennaPercent[j] = 20;
			else if(vBer>=VBER_ANTENNA_25)
				pISDBStatData[i].antennaPercent[j] = 25;
			else if(vBer>=VBER_ANTENNA_30)
				pISDBStatData[i].antennaPercent[j] = 30;
			else if(vBer>=VBER_ANTENNA_35)
				pISDBStatData[i].antennaPercent[j] = 35;
			else if(vBer>=VBER_ANTENNA_40)
				pISDBStatData[i].antennaPercent[j] = 40;
			else if(vBer>=VBER_ANTENNA_45)
				pISDBStatData[i].antennaPercent[j] = 45;
			else if(vBer>=VBER_ANTENNA_50)
				pISDBStatData[i].antennaPercent[j] = 50;
			else if(vBer>=VBER_ANTENNA_55)
				pISDBStatData[i].antennaPercent[j] = 55;
			else if(vBer>=VBER_ANTENNA_60)
				pISDBStatData[i].antennaPercent[j] = 60;
			else if(vBer>=VBER_ANTENNA_65)
				pISDBStatData[i].antennaPercent[j] = 65;
			else {
				vPCBER = pISDBStatData[0].status.pcber[j].avgValue;
				if(vPCBER>PCBER_ANTENNA_75)
					pISDBStatData[i].antennaPercent[j] = 70;
				else if(vPCBER>PCBER_ANTENNA_80)
					pISDBStatData[i].antennaPercent[j] = 75;	
				else if(vPCBER>PCBER_ANTENNA_85)
					pISDBStatData[i].antennaPercent[j] = 80;	
				else if(vPCBER>PCBER_ANTENNA_90)
					pISDBStatData[i].antennaPercent[j] = 85;	
				else if(vPCBER>PCBER_ANTENNA_95)
					pISDBStatData[i].antennaPercent[j] = 90;	
				else if(vPCBER>PCBER_ANTENNA_100)
					pISDBStatData[i].antennaPercent[j] = 95;	
				else 
					pISDBStatData[i].antennaPercent[j] = 100; 
			}
			
		}
	}
	return TCC353X_RETURN_SUCCESS;
}


/*---------------------------------------------------------------------
 * Function name
 * 	Tcc353xMonitoringApiGetStatus
 * Description
 * 	Get tcc353x monitoring status
 * Parameters
 * 	_moduleIndex : Index of module
 *		0 : BaseBand#0 (Single : default, Dual : BaseBand#0)
 * 		1 : BaseBand#1 (Single : Not use, Dual : BaseBand#1)
 *	_diversityIndex : Index of diversity
 *		0 : Diversity master
 *		1 : Diversity slave#1
 *		2 : Diversity slave#2
 *		3 : Diversity slave#3
 *      pISDBStatData : status data structure
 * Return value
 * 	Refer EnumReturn
 * Remark
 * 	example for antenna bar
 *           VBER (scale factor : 100000)
 *      0 :   320
 *      1 :   200
 *      2 :   150
 *      3 :   100
 *      4 :    50
 ---------------------------------------------------------------------*/
I32S Tcc353xMonitoringApiGetStatus(I32S _moduleIndex, I32S _diversityIndex,
				   Tcc353xStatus_t * pISDBStatData)
{
#ifdef _READ_OPSTATUS_
	I32S ret;
	I32U opStatusData[8];

	ret =Tcc353xApiOpStatusRead(_moduleIndex, _diversityIndex,
				   32, &opStatusData[0]);
	if(ret != TCC353X_RETURN_SUCCESS)
		return TCC353X_RETURN_FAIL;

	/* get sync status from opstatus */
	pISDBStatData->opstat.syncStatus = (opStatusData[0]&0xFFF);

	if((pISDBStatData->opstat.syncStatus & 0x0C00)==0x0C00)
		pISDBStatData->opstat.dataState = 1;
	else
		pISDBStatData->opstat.dataState = 0;

	if(pISDBStatData->opstat.syncStatus & 0x04)
		pISDBStatData->opstat.cfoLock = 1;
	else
		pISDBStatData->opstat.cfoLock = 0;

	/* get MER */
	pISDBStatData->lxMer[0] = 0;
	pISDBStatData->lxMer[1] = 0;
	pISDBStatData->lxMer[2] = 0;

	/* get rssi */
	pISDBStatData->bbLoopGain = (I08U)((opStatusData[0] >> 24)&0xFF);
	pISDBStatData->rfLoopGain = (I08U)((opStatusData[0] >> 16)&0xFF);

	/* for opstatus */
	if(pISDBStatData->opstat.syncStatus >= 0x09)
		pISDBStatData->opstat.gi = (I08U)((opStatusData[0]>>14)&0x03);
	else
		pISDBStatData->opstat.gi = GI_UNKNOWN;

	if(pISDBStatData->opstat.syncStatus >= 0x09)
		pISDBStatData->opstat.mode = 
					(I08U)((opStatusData[0]>>12)&0x03);
	else
		pISDBStatData->opstat.mode = MODE_RESERVED;

	pISDBStatData->opstat.ResyncCnt = (I16U)((opStatusData[1]>>18)&0x1FFF);
	pISDBStatData->opstat.EEW = (I08U)((opStatusData[1]>>31)&0x01);

	/* get SNR */
	if(pISDBStatData->opstat.dataState)
		pISDBStatData->snrMer = (opStatusData[1] & 0x3FFFF);
	else
		pISDBStatData->snrMer = 0;

	if(_diversityIndex==0) {
		pISDBStatData->opstat.sysId = 
					(I08U)((opStatusData[2]>>30)&0x03);
		pISDBStatData->opstat.tmccSwitchCnt = 
					(I08U)((opStatusData[2]>>26)&0x0F);
		pISDBStatData->opstat.af = 
					(I08U)((opStatusData[2]>>25)&0x01);
		pISDBStatData->opstat.pr = 
					(I08U)((opStatusData[2]>>24)&0x01);
		pISDBStatData->opstat.AMod = 
					(I08U)((opStatusData[2]>>22)&0x03);
		pISDBStatData->opstat.ACr = 
					(I08U)((opStatusData[2]>>19)&0x07);
		pISDBStatData->opstat.AIntLen = 
					(I08U)((opStatusData[2]>>16)&0x07);
		pISDBStatData->opstat.ASegNo = 
					(I08U)((opStatusData[2]>>12)&0x0F);
		pISDBStatData->opstat.BMod = 
					(I08U)((opStatusData[2]>>10)&0x03);
		pISDBStatData->opstat.BCr = 
					(I08U)((opStatusData[2]>>7)&0x07);
		pISDBStatData->opstat.BIntLen = 
					(I08U)((opStatusData[2]>>4)&0x07);
		pISDBStatData->opstat.BSegNo = 
					(I08U)((opStatusData[2])&0x0F);

		pISDBStatData->opstat.APcber = 
					(I16U)((opStatusData[3]>>16)&0xFFFF);
		pISDBStatData->opstat.BPcber = 
					(I16U)((opStatusData[3])&0xFFFF);

		pISDBStatData->opstat.ARsErrorCnt = 
					(I32U)(opStatusData[4]);
		pISDBStatData->opstat.ARsOverCnt = 
					(I32U)((opStatusData[5]>>16)&0xFFFF);
		pISDBStatData->opstat.ARsCnt = 
					(I32U)((opStatusData[5])&0xFFFF);

		pISDBStatData->opstat.BRsErrorCnt = 
					(I32U)(opStatusData[6]);
		pISDBStatData->opstat.BRsOverCnt = 
					(I32U)((opStatusData[7]>>16)&0xFFFF);
		pISDBStatData->opstat.BRsCnt = 
					(I32U)((opStatusData[7])&0xFFFF);

		if(pISDBStatData->opstat.dataState) {
			I32U tempA,tempB;
			tempA = (I32U)(pISDBStatData->opstat.APcber);
			tempB = (I32U)(pISDBStatData->opstat.BPcber);
			
			pISDBStatData->pcber[0] = (tempA<<pISDBStatData->opstat.ACr);
			pISDBStatData->pcber[1] = (tempB<<pISDBStatData->opstat.BCr);
			pISDBStatData->pcber[2] = 0xFFFF000;	/* MAX */
		} else {
			pISDBStatData->pcber[0] = 0xFFFF000;
			pISDBStatData->pcber[1] = 0xFFFF000;
			pISDBStatData->pcber[2] = 0xFFFF000;	/* MAX */
		}
	}

	/* calculate all values */
	Tcc353xUpdateMonitoringStatus(_moduleIndex, _diversityIndex,
				      pISDBStatData);
#else
	I32S ret;
	mailbox_t mailbox;

#ifdef _TCC353X_FIELD_TEST_LOG
	/* for field test */
	I32U EST_DOP_FREQ;
	I32U DIV_NOF_OP;
	I32U DIV_NOF_COMP_OP;

	ret = Tcc353xApiMailboxRead(_moduleIndex, _diversityIndex,
			      ((0x00 << 11) | 0x01), &mailbox);
	if(ret != TCC353X_RETURN_SUCCESS)
		return TCC353X_RETURN_FAIL;
	pISDBStatData->syncStatus = mailbox.data_array[0];

	ret = Tcc353xApiMailboxRead(_moduleIndex, _diversityIndex,
			      ((0x00 << 11) | 0x03), &mailbox);
	if(ret != TCC353X_RETURN_SUCCESS)
		return TCC353X_RETURN_FAIL;
	TcpalMemcpy(&pISDBStatData->RESYNC_RESULT[0],
		    &mailbox.data_array[0], sizeof(I32U)*7);

	ret = Tcc353xApiMailboxRead(_moduleIndex, _diversityIndex,
			      ((0x0F << 11) | 0x00), &mailbox);
	if(ret != TCC353X_RETURN_SUCCESS)
		return TCC353X_RETURN_FAIL;
	pISDBStatData->EST_DOP_FREQ = (mailbox.data_array[2]&0xFFFF);

	ret = Tcc353xApiMailboxRead(_moduleIndex, _diversityIndex,
			      ((0x03 << 11) | 0x00), &mailbox);
	if(ret != TCC353X_RETURN_SUCCESS)
		return TCC353X_RETURN_FAIL;
	pISDBStatData->DIV_NOF_OP = (mailbox.data_array[3]&0xFFFF);
	pISDBStatData->DIV_NOF_COMP_OP = ((mailbox.data_array[3]>>16)&0xFFFF);
#endif

#if 0
	{
		/* test */
		I64S samplingfreqerr = 0;
		double dsample = 0;
		I64U L,H,temp, temp2;
		ret = Tcc353xApiMailboxRead(_moduleIndex, _diversityIndex,
				      ((0x0A << 11) | 0x01), &mailbox);
		if(ret != TCC353X_RETURN_SUCCESS)
			return TCC353X_RETURN_FAIL;
		H = (I64U)(mailbox.data_array[1]&0xFF);
		L = (I64U)(mailbox.data_array[2]);

		temp = (H<<32);
		if(H&0x80)
			temp2 = (temp|0xFFFFFF0000000000);

		samplingfreqerr = (I64S)(L+temp2);
		dsample = ((double)(samplingfreqerr)/1000000.0);

		if(_diversityIndex==0)
			TcpalPrintStatus((I08S *)"[Master] SFE [%lf]\n",dsample);
		else
			TcpalPrintStatus((I08S *)"[Slave] SFE [%lf]\n", dsample);
	}
#endif

	/* get SNR */
	ret = Tcc353xApiMailboxRead(_moduleIndex, _diversityIndex,
			      ((0x11 << 11) | 0x00), &mailbox);
	if(ret != TCC353X_RETURN_SUCCESS)
		return TCC353X_RETURN_FAIL;

	pISDBStatData->snrMer = mailbox.data_array[0];

	/* get MER */
	ret = Tcc353xApiMailboxRead(_moduleIndex, _diversityIndex,
			      ((0x12 << 11) | 0x01), &mailbox);
	if(ret != TCC353X_RETURN_SUCCESS)
		return TCC353X_RETURN_FAIL;

	pISDBStatData->lxMer[0] = (mailbox.data_array[0] & 0xFFFF);
	pISDBStatData->lxMer[1] = (mailbox.data_array[1] & 0xFFFF);
	pISDBStatData->lxMer[2] = (mailbox.data_array[2] & 0xFFFF);

	/* get rssi */
	ret = Tcc353xApiMailboxRead(_moduleIndex, _diversityIndex,
			      ((0x1 << 11) | 0x06), &mailbox);
	if(ret != TCC353X_RETURN_SUCCESS)
		return TCC353X_RETURN_FAIL;

	pISDBStatData->bbLoopGain = (I08U)(((mailbox.data_array[0] >> 8) & 0xFF));
	pISDBStatData->rfLoopGain = (I08U)((mailbox.data_array[0] & 0xFF));

	if(_diversityIndex==0) {
		/* get PCBER Layer1/2/3 */
		ret = Tcc353xApiMailboxRead(_moduleIndex, _diversityIndex,
				      ((0x2 << 11) | 0x00), &mailbox);
		if(ret != TCC353X_RETURN_SUCCESS)
			return TCC353X_RETURN_FAIL;
		
		pISDBStatData->pcber[0] = ((mailbox.data_array[0]) & 0xFFFF);
		pISDBStatData->rsErrorCount[0] =
		    (mailbox.data_array[1] |
		     ((I64U) (mailbox.data_array[2]) << 32));
		pISDBStatData->rsOverCount[0] = mailbox.data_array[3];
		pISDBStatData->rsPacketCount[0] = mailbox.data_array[4];

		ret = Tcc353xApiMailboxRead(_moduleIndex, _diversityIndex,
				      ((0x2 << 11) | 0x01), &mailbox);
		if(ret != TCC353X_RETURN_SUCCESS)
			return TCC353X_RETURN_FAIL;
		
		pISDBStatData->pcber[1] = ((mailbox.data_array[0]) & 0xFFFF);
		pISDBStatData->rsErrorCount[1] =
		    (mailbox.data_array[1] |
		     ((I64U) (mailbox.data_array[2]) << 32));
		pISDBStatData->rsOverCount[1] = mailbox.data_array[3];
		pISDBStatData->rsPacketCount[1] = mailbox.data_array[4];

		ret = Tcc353xApiMailboxRead(_moduleIndex, _diversityIndex,
				      ((0x2 << 11) | 0x02), &mailbox);
		if(ret != TCC353X_RETURN_SUCCESS)
			return TCC353X_RETURN_FAIL;
		
		pISDBStatData->pcber[2] = ((mailbox.data_array[0]) & 0xFFFF);
		pISDBStatData->rsErrorCount[2] =
		    (mailbox.data_array[1] |
		     ((I64U) (mailbox.data_array[2]) << 32));
		pISDBStatData->rsOverCount[2] = mailbox.data_array[3];
		pISDBStatData->rsPacketCount[2] = mailbox.data_array[4];
	}
	/* calculate all values */
	Tcc353xUpdateMonitoringStatus(_moduleIndex, _diversityIndex,
				      pISDBStatData);
#endif
	return TCC353X_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------
 * Function name
 * 	Tcc353xMonitoringApiGetDbgStatus
 * Description
 * 	Get Tcc353x debug status
 * Parameters
 * 	_moduleIndex : Index of module
 *		0 : BaseBand#0 (Single : default, Dual : BaseBand#0)
 * 		1 : BaseBand#1 (Single : Not use, Dual : BaseBand#1)
 *	_diversityIndex : Index of diversity
 *		0 : Diversity master
 *		1 : Diversity slave#1
 *		2 : Diversity slave#2
 *		3 : Diversity slave#3
 *      _mailbox : mailbox structure
 *      count : mailbox count
 * Return value
 * 	Refer EnumReturn
 * Remark
 * 	Do not use this function except for special purpose
 ---------------------------------------------------------------------*/
I32S Tcc353xMonitoringApiGetDbgStatus(I32S _moduleIndex,
				      I32S _diversityIndex,
				      mailbox_t * _mailbox, I32S _count)
{
	I32S i;
	I32S ret = TCC353X_RETURN_SUCCESS;

	for (i = 0; i < _count; i++) {
		ret = Tcc353xApiMailboxRead(_moduleIndex, _diversityIndex,
					    _mailbox[i].cmd, &_mailbox[i]);
		if (ret != TCC353X_RETURN_SUCCESS)
			break;
	}

	return ret;
}

/*---------------------------------------------------------------------
 * Function name
 * 	Tcc353xMonitoringApiAntennaPercentage
 * Description
 * 	Special tuning routine for diversity process
 *      - combining blocking
 * Parameters
 * 	_moduleIndex : Index of module
 *		0 : BaseBand#0 (Single : default, Dual : BaseBand#0)
 * 		1 : BaseBand#1 (Single : Not use, Dual : BaseBand#1)
 *      pISDBStatData : all bb's Status data pointer
 *      _InputSize : sizeof input status data
 * Return value
 * 	Refer EnumReturn
 * Remark
 * 
 ---------------------------------------------------------------------*/
I32S Tcc353xMonitoringDiversityProc (I32S _moduleIndex, 
				    Tcc353xStatus_t * pISDBStatData,
				    I32U _InputSize)
{
	I32S i;
	I32U max;
	Tcc353xDivProc_Params *divParam;
	I32U structSize = 0;
	TcpalTime_t Monitor_time;
	I32U TimeGap;

	divParam = (Tcc353xDivProc_Params *)(&Tcc353xDivParams[_moduleIndex]);
	max = (_InputSize/sizeof(Tcc353xStatus_t));
	if(max<2 || max>TCC353X_DIVERSITY_MAX)
		return TCC353X_RETURN_FAIL;

	/* diversity special process routine */
	if (divParam->rssiCount < MAX_DIV_PARAM_RSSI)
		divParam->rssiCount++;

	for(i=0; i<max; i++)	{
		divParam->rssi[i] = 
		    Tcc353xGetSignedMovingAvg(&divParam->rssi_avgArray[i][0], 
			pISDBStatData[i].status.rssi.currentValue, 
			divParam->rssiCount, MAX_DIV_PARAM_RSSI);
	}

	if(divParam->oldAccessTime==0)	/* already structrue inited */
		divParam->oldAccessTime = TcpalGetCurrentTimeCount_ms();

	TimeGap = (I32U)(TcpalGetTimeIntervalCount_ms(divParam->oldAccessTime));
	if(TimeGap > 1500) {
		/*over 1.5sec*/
		I32S rssimax = -105;

		divParam->oldAccessTime = TcpalGetCurrentTimeCount_ms();
		divParam->rssiCount = 1;

		for(i=0; i<max; i++)	{
			if(divParam->rssi[i]>rssimax) {
				rssimax = divParam->rssi[i];
			}
		}

		for(i=0; i<max; i++) {
			if(rssimax-divParam->rssi[i]>=40) {
				Tcc353xApiDiversityCombineBlock(0, i, 1);
				/*
				TcpalPrintStatus((I08S *)"[TCC353X] [%d][%ddBm][Block]\n", i, divParam->rssi[i]);
				*/
			} else {
				Tcc353xApiDiversityCombineBlock(0, i, 0);
				/*
				TcpalPrintStatus((I08S *)"[TCC353X] [%d][%ddBm][unBlock]\n", i, divParam->rssi[i]);
				*/
			}
		}
	}
	return TCC353X_RETURN_SUCCESS;
}

static void Tcc353xUpdateMonitoringStatusBer(I32S _moduleIndex,
					     I32S _diversityIndex,
					     Tcc353xStatus_t *
					     pISDBStatData)
{
#ifdef _READ_OPSTATUS_
	/* A,B,C Layer */
	I32S layerIndex;

	if(Tcc353xStatus[_moduleIndex][_diversityIndex].opstat.oldResyncCnt != 
					pISDBStatData->opstat.ResyncCnt)
		pISDBStatData->opstat.resynced = 1;
	else
		pISDBStatData->opstat.resynced = 0;

	pISDBStatData->opstat.oldResyncCnt = 
		pISDBStatData->opstat.ResyncCnt;

	for (layerIndex = 0; layerIndex < 3; layerIndex++) {
		/* Update MER Set */
		if (Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    mer[layerIndex].count < ISDB_MAX_MOV_AVG)
			Tcc353xStatus[_moduleIndex]
			    [_diversityIndex].status.
			    mer[layerIndex].count++;

		Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    mer[layerIndex].currentValue =
		    Tcc353xCalculateMer(pISDBStatData, layerIndex);
		Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    mer[layerIndex].avgValue =
		    Tcc353xGetUnsignedMovingAvg(Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						mer[layerIndex].array,
						Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						mer[layerIndex].
						currentValue,
						Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						mer[layerIndex].count,
						ISDB_MAX_MOV_AVG);

		/* Update pcber Set */
		if (Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    pcber[layerIndex].count < ISDB_MAX_MOV_AVG)
			Tcc353xStatus[_moduleIndex]
			    [_diversityIndex].status.
			    pcber[layerIndex].count++;

		Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    pcber[layerIndex].currentValue =
		    Tcc353xCalculatePcber(pISDBStatData, layerIndex);
		Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    pcber[layerIndex].avgValue =
		    Tcc353xGetUnsignedMovingAvg(Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						pcber[layerIndex].array,
						Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						pcber[layerIndex].
						currentValue,
						Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						pcber[layerIndex].count,
						ISDB_MAX_MOV_AVG);

		/* Update VITERBIBER Set */
		if (Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    viterbiber[layerIndex].count < ISDB_MAX_MOV_AVG)
			Tcc353xStatus[_moduleIndex]
			    [_diversityIndex].
			    status.viterbiber[layerIndex].count++;

		Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    viterbiber[layerIndex].currentValue =
		    Tcc353xCalculateViterbiber(pISDBStatData,
					       Tcc353xStatus[_moduleIndex]
					       [_diversityIndex].status.
					       viterbiber[layerIndex].
					       oldValue, layerIndex);
		Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    viterbiber[layerIndex].avgValue =
		    Tcc353xGetUnsignedMovingAvg(Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						viterbiber[layerIndex].
						array,
						Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						viterbiber[layerIndex].
						currentValue,
						Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						viterbiber[layerIndex].
						count, ISDB_MAX_MOV_AVG);

		/* Update TSPER Set */
		if (Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    tsper[layerIndex].count < ISDB_MAX_MOV_AVG)
			Tcc353xStatus[_moduleIndex]
			    [_diversityIndex].status.
			    tsper[layerIndex].count++;

		Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    tsper[layerIndex].currentValue =
		    Tcc353xCalculateTsper(pISDBStatData,
					  Tcc353xStatus[_moduleIndex]
					  [_diversityIndex].status.
					  tsper[layerIndex].oldValue,
					  layerIndex);
		Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    tsper[layerIndex].avgValue =
		    Tcc353xGetUnsignedMovingAvg(Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						tsper[layerIndex].array,
						Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						tsper[layerIndex].
						currentValue,
						Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						tsper[layerIndex].count,
						ISDB_MAX_MOV_AVG);
	}
#else
	/* A,B,C Layer */

	I32S layerIndex;

	for (layerIndex = 0; layerIndex < 3; layerIndex++) {
		pISDBStatData->oldRsPacketCount[layerIndex] =
		    Tcc353xStatus[_moduleIndex]
		    [_diversityIndex].oldRsPacketCount[layerIndex];
		pISDBStatData->oldRsErrorCount[layerIndex] =
		    Tcc353xStatus[_moduleIndex]
		    [_diversityIndex].oldRsErrorCount[layerIndex];
		pISDBStatData->oldRsOverCount[layerIndex] =
		    Tcc353xStatus[_moduleIndex]
		    [_diversityIndex].oldRsOverCount[layerIndex];
		pISDBStatData->packetResynced[layerIndex] =
		    Tcc353xStatus[_moduleIndex]
		    [_diversityIndex].packetResynced[layerIndex];

		if (pISDBStatData->rsPacketCount[layerIndex] ==
		    pISDBStatData->oldRsPacketCount[layerIndex]) {
			/* same as previouse */
		} else {
			pISDBStatData->packetResynced[layerIndex] = 0;
			if (pISDBStatData->rsPacketCount[layerIndex] == 0) {
				if (pISDBStatData->oldRsPacketCount
				    [layerIndex]) {
					pISDBStatData->packetResynced
					    [layerIndex] = 1;
				}
			} else if (pISDBStatData->rsPacketCount[layerIndex]
				   != 0
				   &&
				   pISDBStatData->rsPacketCount[layerIndex]
				   <
				   pISDBStatData->oldRsPacketCount
				   [layerIndex]) {
				if (pISDBStatData->oldRsPacketCount
				    [layerIndex] < 0x80000000) {
					/* auto resync situation */
					pISDBStatData->packetResynced
					    [layerIndex] = 1;
				} else {
					/* rotate */
					pISDBStatData->oldRsPacketCount
					    [layerIndex] = 0;
					pISDBStatData->oldRsErrorCount
					    [layerIndex] = 0;
					pISDBStatData->oldRsOverCount
					    [layerIndex] = 0;
				}
			}
		}

		/* Update MER Set */
		if (Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    mer[layerIndex].count < ISDB_MAX_MOV_AVG)
			Tcc353xStatus[_moduleIndex]
			    [_diversityIndex].status.
			    mer[layerIndex].count++;

		Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    mer[layerIndex].currentValue =
		    Tcc353xCalculateMer(pISDBStatData, layerIndex);
		Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    mer[layerIndex].avgValue =
		    Tcc353xGetUnsignedMovingAvg(Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						mer[layerIndex].array,
						Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						mer[layerIndex].
						currentValue,
						Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						mer[layerIndex].count,
						ISDB_MAX_MOV_AVG);

		/* Update pcber Set */
		if (Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    pcber[layerIndex].count < ISDB_MAX_MOV_AVG)
			Tcc353xStatus[_moduleIndex]
			    [_diversityIndex].status.
			    pcber[layerIndex].count++;

		Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    pcber[layerIndex].currentValue =
		    Tcc353xCalculatePcber(pISDBStatData, layerIndex);
		Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    pcber[layerIndex].avgValue =
		    Tcc353xGetUnsignedMovingAvg(Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						pcber[layerIndex].array,
						Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						pcber[layerIndex].
						currentValue,
						Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						pcber[layerIndex].count,
						ISDB_MAX_MOV_AVG);

		/* Update VITERBIBER Set */
		if (Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    viterbiber[layerIndex].count < ISDB_MAX_MOV_AVG)
			Tcc353xStatus[_moduleIndex]
			    [_diversityIndex].
			    status.viterbiber[layerIndex].count++;

		Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    viterbiber[layerIndex].currentValue =
		    Tcc353xCalculateViterbiber(pISDBStatData,
					       Tcc353xStatus[_moduleIndex]
					       [_diversityIndex].status.
					       viterbiber[layerIndex].
					       oldValue, layerIndex);
		Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    viterbiber[layerIndex].avgValue =
		    Tcc353xGetUnsignedMovingAvg(Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						viterbiber[layerIndex].
						array,
						Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						viterbiber[layerIndex].
						currentValue,
						Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						viterbiber[layerIndex].
						count, ISDB_MAX_MOV_AVG);

		/* Update TSPER Set */
		if (Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    tsper[layerIndex].count < ISDB_MAX_MOV_AVG)
			Tcc353xStatus[_moduleIndex]
			    [_diversityIndex].status.
			    tsper[layerIndex].count++;

		Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    tsper[layerIndex].currentValue =
		    Tcc353xCalculateTsper(pISDBStatData,
					  Tcc353xStatus[_moduleIndex]
					  [_diversityIndex].status.
					  tsper[layerIndex].oldValue,
					  layerIndex);
		Tcc353xStatus[_moduleIndex][_diversityIndex].status.
		    tsper[layerIndex].avgValue =
		    Tcc353xGetUnsignedMovingAvg(Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						tsper[layerIndex].array,
						Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						tsper[layerIndex].
						currentValue,
						Tcc353xStatus[_moduleIndex]
						[_diversityIndex].status.
						tsper[layerIndex].count,
						ISDB_MAX_MOV_AVG);

		pISDBStatData->oldRsPacketCount[layerIndex] =
		    pISDBStatData->rsPacketCount[layerIndex];
		pISDBStatData->oldRsErrorCount[layerIndex] =
		    pISDBStatData->rsErrorCount[layerIndex];
		pISDBStatData->oldRsOverCount[layerIndex] =
		    pISDBStatData->rsOverCount[layerIndex];
	}
#endif
}

static void Tcc353xUpdateMonitoringStatus(I32S _moduleIndex,
					  I32S _diversityIndex,
					  Tcc353xStatus_t * pISDBStatData)
{
	I08U lockRegister;

	if(_diversityIndex==0)
		/* Update MER, PCBER, Viterbi BER, TSPER */
		Tcc353xUpdateMonitoringStatusBer(_moduleIndex, 
						_diversityIndex,
						 pISDBStatData);

	/* get lock status */
	Tcc353xApiRegisterRead(_moduleIndex, _diversityIndex, 0x0B,
			       &lockRegister, 1);
	Tcc353xApiParseIsdbSyncStat(&Tcc353xStatus[_moduleIndex]
				    [_diversityIndex].status.isdbLock,
				    lockRegister);

	/* Update SNR Set */
	if (Tcc353xStatus[_moduleIndex][_diversityIndex].status.
	    snr.count < ISDB_MAX_MOV_AVG)
		Tcc353xStatus[_moduleIndex]
		    [_diversityIndex].status.
		    snr.count++;
	
	Tcc353xStatus[_moduleIndex][_diversityIndex].status.
	    snr.currentValue =
	    Tcc353xCalculateSnr(pISDBStatData);
	Tcc353xStatus[_moduleIndex][_diversityIndex].status.
	    snr.avgValue =
	    Tcc353xGetUnsignedMovingAvg(Tcc353xStatus[_moduleIndex]
					[_diversityIndex].status.
					snr.array,
					Tcc353xStatus[_moduleIndex]
					[_diversityIndex].status.
					snr.
					currentValue,
					Tcc353xStatus[_moduleIndex]
					[_diversityIndex].status.
					snr.count,
					ISDB_MAX_MOV_AVG);

	/* Update RSSI Set */
	if (Tcc353xStatus[_moduleIndex][_diversityIndex].status.rssi.
	    count < ISDB_MAX_MOV_AVG)
		Tcc353xStatus[_moduleIndex][_diversityIndex].status.rssi.
		    count++;

	Tcc353xStatus[_moduleIndex][_diversityIndex].status.rssi.
	    currentValue = Tcc353xCalculateRssi(_moduleIndex, _diversityIndex,
	    					pISDBStatData);
	Tcc353xStatus[_moduleIndex][_diversityIndex].status.rssi.avgValue =
	    Tcc353xGetSignedMovingAvg(Tcc353xStatus[_moduleIndex]
				      [_diversityIndex].status.rssi.array,
				      Tcc353xStatus[_moduleIndex]
				      [_diversityIndex].status.rssi.
				      currentValue,
				      Tcc353xStatus[_moduleIndex]
				      [_diversityIndex].status.rssi.count,
				      ISDB_MAX_MOV_AVG);
	TcpalMemcpy(&pISDBStatData->status,
		    &Tcc353xStatus[_moduleIndex][_diversityIndex].status,
		    sizeof(Tcc353xStatusValue_t));
	TcpalMemcpy(&Tcc353xStatus[_moduleIndex][_diversityIndex],
		    pISDBStatData, sizeof(Tcc353xStatus_t));
}
