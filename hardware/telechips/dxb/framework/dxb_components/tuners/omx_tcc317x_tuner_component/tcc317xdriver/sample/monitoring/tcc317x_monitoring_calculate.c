/****************************************************************************
 *   FileName    : tcc317x_monitoring_calculate.c
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

#include "tcc317x_monitoring_calculate.h"
#include "tcpal_os.h"

static I32U Tcc317xLog10 (I32U _val);

#define SCALE_FACTOR 100000

#define _TDMB_MIN_SNR_  0
#define _TDMB_MAX_SNR_  25

#define _TDMB_MIN_PCBER_  0
#define _TDMB_MAX_PCBER_  0.2

#define _TDMB_MIN_VITERBIBER_  0
#define _TDMB_MAX_VITERBIBER_  1

#define _TDMB_MIN_TSPER_  0
#define _TDMB_MAX_TSPER_  1

#define _TDMB_MIN_RSSI_  (-105)
#define _TDMB_MAX_RSSI_  3

void Tcc317xCalculateInitValues(Tcc317xStatus_t * _dmbStatusData)
{
    /* FIC PCBER */
    _dmbStatusData->status.ficPcber.count = 1;
    TcpalMemset (&_dmbStatusData->status.ficPcber.array[0], 0x00, sizeof(I32U)*TDMB_MAX_MOV_AVG);

    _dmbStatusData->status.ficPcber.currentValue = (I32U)(_TDMB_MAX_PCBER_ * SCALE_FACTOR);
    _dmbStatusData->status.ficPcber.avgValue =
    Tcc317xGetUnsignedMovingAvg (&_dmbStatusData->status.ficPcber.array[0], _dmbStatusData->status.ficPcber.currentValue,
    _dmbStatusData->status.ficPcber.count, TDMB_MAX_MOV_AVG);

    /* MSC PCBER */
    _dmbStatusData->status.mscPcber.count = 1;
    TcpalMemset (&_dmbStatusData->status.mscPcber.array[0], 0x00, sizeof(I32U)*TDMB_MAX_MOV_AVG);

    _dmbStatusData->status.mscPcber.currentValue = (I32U)(_TDMB_MAX_PCBER_ * SCALE_FACTOR);
    _dmbStatusData->status.mscPcber.avgValue =
    Tcc317xGetUnsignedMovingAvg (&_dmbStatusData->status.mscPcber.array[0], _dmbStatusData->status.mscPcber.currentValue,
    _dmbStatusData->status.mscPcber.count, TDMB_MAX_MOV_AVG);

    /* PCBER */
    _dmbStatusData->status.pcber.count = 1;
    TcpalMemset (&_dmbStatusData->status.pcber.array[0], 0x00, sizeof(I32U)*TDMB_MAX_MOV_AVG);

    _dmbStatusData->status.pcber.currentValue = (I32U)(_TDMB_MAX_PCBER_ * SCALE_FACTOR);
    _dmbStatusData->status.pcber.avgValue =
    Tcc317xGetUnsignedMovingAvg (&_dmbStatusData->status.pcber.array[0], _dmbStatusData->status.pcber.currentValue,
    _dmbStatusData->status.pcber.count, TDMB_MAX_MOV_AVG);

    /* SNR */
    _dmbStatusData->status.snrDb.count = 1;
    TcpalMemset(&_dmbStatusData->status.snrDb.array[0], 0x00, sizeof(I32U)*TDMB_MAX_MOV_AVG);

    _dmbStatusData->status.snrDb.currentValue = _TDMB_MIN_SNR_;
    _dmbStatusData->status.snrDb.avgValue =
    Tcc317xGetUnsignedMovingAvg (&_dmbStatusData->status.snrDb.array[0], _dmbStatusData->status.snrDb.currentValue,
    _dmbStatusData->status.snrDb.count, TDMB_MAX_MOV_AVG);

    /* TSPER */
    _dmbStatusData->status.tsper.count = 1;
    TcpalMemset(&_dmbStatusData->status.tsper.array[0], 0x00, sizeof(I32U)*TDMB_MAX_MOV_AVG);

    _dmbStatusData->status.tsper.currentValue = _TDMB_MAX_TSPER_*SCALE_FACTOR;
    _dmbStatusData->status.tsper.avgValue =
    Tcc317xGetUnsignedMovingAvg (&_dmbStatusData->status.tsper.array[0], _dmbStatusData->status.tsper.currentValue,
    _dmbStatusData->status.tsper.count, TDMB_MAX_MOV_AVG);

    /* VITERBI */
    _dmbStatusData->status.viterbiber.count = 1;
    TcpalMemset(&_dmbStatusData->status.viterbiber.array[0], 0x00, sizeof(I32U)*TDMB_MAX_MOV_AVG);

    _dmbStatusData->status.viterbiber.currentValue = _TDMB_MAX_VITERBIBER_*SCALE_FACTOR;
    _dmbStatusData->status.viterbiber.avgValue =
    Tcc317xGetUnsignedMovingAvg (&_dmbStatusData->status.viterbiber.array[0], _dmbStatusData->status.viterbiber.currentValue,
    _dmbStatusData->status.viterbiber.count, TDMB_MAX_MOV_AVG);

    /* RSSI */
    _dmbStatusData->status.rssi.count = 1;
    TcpalMemset(&_dmbStatusData->status.rssi.array[0], 0x00, sizeof(I32S)*TDMB_MAX_MOV_AVG);

    _dmbStatusData->status.rssi.currentValue = _TDMB_MIN_RSSI_;
    _dmbStatusData->status.rssi.avgValue =
    Tcc317xGetSignedMovingAvg (&_dmbStatusData->status.rssi.array[0], _dmbStatusData->status.rssi.currentValue,
    _dmbStatusData->status.rssi.count, TDMB_MAX_MOV_AVG);

}

I32U Tcc317xCalculateSnr (Tcc317xStatus_t * _dmbStatusData)
{
    I32U      logi;
    I32U      SNR;
    I32U      prsSnr_mod_2_10;

    prsSnr_mod_2_10 = _dmbStatusData->prsSnr;

    logi = prsSnr_mod_2_10;

    if (logi == 0)
    {
        SNR = _TDMB_MIN_SNR_;
    }
    else
    {
        SNR = Tcc317xLog10 (logi); /* TC3X_log10_ return *10 */
    }

    if (SNR > _TDMB_MAX_SNR_)
        SNR = _TDMB_MAX_SNR_;

    return SNR;
}

I32U Tcc317xCalculatePcber (I32U _inPcber)
{
    I32U      pcber;
    I64U      over;

    over = (I64U) (_inPcber) * SCALE_FACTOR;
    pcber = (I32U) (over >> 16);

    if (pcber > _TDMB_MAX_PCBER_ * SCALE_FACTOR)
        pcber = (I32U)(_TDMB_MAX_PCBER_ * SCALE_FACTOR);

    return pcber;
}

I32U Tcc317xCalculateViterbiber (Tcc317xStatus_t * _dmbStatusData, I32U oldViterbiber)
{
    I32U      VITERBIBER;
    I32U      overCnt;
    I32U      errorCnt;
    I32U      over;
    I32U      under;
    I32U      temp;

    if (_dmbStatusData->packetResynced)
    {
        VITERBIBER = _TDMB_MAX_VITERBIBER_ * SCALE_FACTOR;
        return VITERBIBER;
    }

    if (!(_dmbStatusData->packetResynced) && _dmbStatusData->rsPacketCount == 0)
    {
        VITERBIBER = _TDMB_MIN_VITERBIBER_;
        return VITERBIBER;
    }

    if (!_dmbStatusData->packetResynced && (_dmbStatusData->rsPacketCount == _dmbStatusData->oldRsPacketCount))
    {
        VITERBIBER = oldViterbiber;
        return VITERBIBER;
    }

    overCnt = (_dmbStatusData->rsOverCount - _dmbStatusData->oldRsOverCount);
    errorCnt = (I32U)(_dmbStatusData->rsErrorCount - _dmbStatusData->oldRsErrorCount);
    temp = _dmbStatusData->rsPacketCount - _dmbStatusData->oldRsPacketCount;
    under = temp * 204 * 8;

    over = (overCnt*(8*8) + errorCnt) * SCALE_FACTOR;

    VITERBIBER = (over / under);

    if (VITERBIBER > _TDMB_MAX_VITERBIBER_ * SCALE_FACTOR)
        VITERBIBER = _TDMB_MAX_VITERBIBER_ * SCALE_FACTOR;

    return VITERBIBER;
}

I32U Tcc317xCalculateTsper (Tcc317xStatus_t * _dmbStatusData, I32U oldTsper)
{
    I32U      TSPER;
    I32U      over;
    I32U      under;

    if (_dmbStatusData->packetResynced)
    {
        TSPER = _TDMB_MAX_TSPER_ * SCALE_FACTOR;
        return TSPER;
    }

    if (!(_dmbStatusData->packetResynced) && _dmbStatusData->rsPacketCount == 0)
    {
        TSPER = _TDMB_MIN_TSPER_;
        return TSPER;
    }

    if (!(_dmbStatusData->packetResynced) && _dmbStatusData->rsPacketCount == _dmbStatusData->oldRsPacketCount)
    {
        TSPER = oldTsper;
        return TSPER;
    }

    over = (_dmbStatusData->rsOverCount - _dmbStatusData->oldRsOverCount);
    under = (_dmbStatusData->rsPacketCount - _dmbStatusData->oldRsPacketCount);

    over *= SCALE_FACTOR;

    TSPER = (over / under);

    if (TSPER > _TDMB_MAX_TSPER_ * SCALE_FACTOR)
        TSPER = _TDMB_MAX_TSPER_ * SCALE_FACTOR;

    return TSPER;
}

I32S Tcc317xCalculateRssi (Tcc317xStatus_t * _dmbStatusData, I32U _frequency)
{
    I32S      RSSI;

    if (_frequency > 1000000) { /* L-Band */
	    if (_dmbStatusData->rfLoopGain <= 90)
		RSSI = 1000 - ((I32S) _dmbStatusData->bbLoopGain) * 33 - 20 * ((I32S) _dmbStatusData->rfLoopGain);
	    else
		RSSI = 1150 - ((I32S) _dmbStatusData->bbLoopGain) * 33 - 20 * ((I32S) _dmbStatusData->rfLoopGain);
    } else { /* VHF */

#if defined (_RSSI_EXT_LNA_CUSTOM1_)
    if (_dmbStatusData->rfLoopGain == 0)
		RSSI = 1100 - _dmbStatusData->bbLoopGain*50;
    else if (_dmbStatusData->rfLoopGain <= 90)
		RSSI = 300 - _dmbStatusData->bbLoopGain*30 - 28*_dmbStatusData->rfLoopGain;
    else
		RSSI = -500 - _dmbStatusData->bbLoopGain*32 - 19*_dmbStatusData->rfLoopGain;
#else
	    if (_dmbStatusData->rfLoopGain <= 90)
		RSSI = 1300 - ((I32S) _dmbStatusData->bbLoopGain) * 30 - 28 * ((I32S) _dmbStatusData->rfLoopGain);
	    else
		RSSI = 400 - ((I32S) _dmbStatusData->bbLoopGain) * 32 - 19 * ((I32S) _dmbStatusData->rfLoopGain);
#endif

    }

    RSSI /= 100;
    if (RSSI < _TDMB_MIN_RSSI_)
        RSSI = _TDMB_MIN_RSSI_;
    else if (RSSI > _TDMB_MAX_RSSI_)
        RSSI = _TDMB_MAX_RSSI_;

    return RSSI;
}

static I32U Tcc317xLog10 (I32U _val)
{
    I32U      i, SNR = 0;
    I32U      snr_table[28] = {
        1024,
        1289,
        1622,
        2043,
        2572,
        3238,
        4076,
        5132,
        6461,
        8133,
        10240,
        12891,
        16229,
        20431,
        25721,
        32381,
        40766,
        51321,
        64610,
        81339,
        102400,
        128913,
        162293,
        204314,
        257217,
        323817,
        407661,
        513215
    };

    if (_val < snr_table[0])
    {
        SNR = 0;
    }
    else if (_val >= snr_table[27])
    {
        SNR = 27;
    }
    else
    {
        for (i = 0; i < 27; i++)
        {
            if (_val >= snr_table[i] && _val < snr_table[i + 1])
            {
                SNR = i;
                break;
            }
        }
    }

    return SNR;
}

I32U Tcc317xGetUnsignedMovingAvg (I32U *_Array, I32U _input, I32U _count, I32S _maxArray)
{
    I32S      j;
    I32S      maxroop = 0;
    I32U      sum = 0;

    maxroop = _count;
    
    TcpalMemcpy (_Array, &_Array[1], (_maxArray - 1) * sizeof (I32U));
    _Array[_maxArray - 1] = _input;

    for (j = 0; j < maxroop; j++)
    {
        if (_maxArray - j - 1 < 0)
        {
            break;
        }

        sum += _Array[_maxArray - j - 1];
    }

    return (sum / _count);
}

I32S Tcc317xGetSignedMovingAvg (I32S *_Array, I32S _input, I32S _count, I32S _maxArray)
{
    I32S      j;
    I32S      maxroop = 0;
    I32S      sum = 0;

    maxroop = _count;
    
    TcpalMemcpy (_Array, &_Array[1], (_maxArray - 1) * sizeof (I32S));
    _Array[_maxArray - 1] = _input;

    for (j = 0; j < maxroop; j++)
    {
        if (_maxArray - j - 1 < 0)
        {
            break;
        }

        sum += _Array[_maxArray - j - 1];
    }

    return (sum / _count);
}

