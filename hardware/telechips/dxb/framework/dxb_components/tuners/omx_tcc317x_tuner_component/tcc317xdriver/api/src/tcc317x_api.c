/****************************************************************************
 *   FileName    : tcc317xapi.cpp
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

#include "tcc317x_common.h"
#include "tcpal_os.h"
#include "tcc317x_api.h"

/* extern functions */
extern I32S Tcc317xOpen(I32S _moduleIndex, Tcc317xOption_t *_Tcc317xOption, func_v_iii _eventCallBackFunc);
extern I32S Tcc317xClose(I32S _moduleIndex);
extern I32S Tcc317xInit (I32S _moduleIndex, I08U *_coldbootData, I32S _codeSize);
extern I32S Tcc317xInit_DiversityNormalMode(I32S _moduleIndex, I32S _diversityIndex, I08U * _coldbootData, I32S _codeSize);
extern I32S Tcc317xTune (I32S _moduleIndex, I32S _frequency, I32U *_reservedOption);
extern I32S Tcc317xStreamStop(I32S _moduleIndex);
extern I32S Tcc317xStreamStart(I32S _moduleIndex);
extern I32S Tcc317xMemoryRead(I32S _moduleIndex, I32S _diversityIndex, I32U _address, I08U *_data, I32U _size);
extern I32S Tcc317xMemoryWrite(I32S _moduleIndex, I32S _diversityIndex, I32U _address, I08U *_data, I32U _size);
extern I32S Tcc317xStreamRead(I32S _moduleIndex, I08U *_data, I32S _size);
extern I32S Tcc317xRead(I32S _moduleIndex, I32S _diversityIndex, I08U _address, I08U *_data, I32U _size);
extern I32S Tcc317xWrite(I32S _moduleIndex, I32S _diversityIndex, I08U _address, I08U *_data, I32U _size);
extern I32S Tcc317xRfRegisterRead(I32S _moduleIndex, I32S _diversityIndex, I08U _address, I32U *_data);
extern I32S Tcc317xRfRegisterWrite(I32S _moduleIndex, I32S _diversityIndex, I08U _address, I32U _data);
extern I32U Tcc317xGetCoreVersion(void);
extern I32U Tcc317xGetDspCodeVersion(I32S _moduleIndex, I32S _diversityIndex);
extern I32S Tcc317xAddService(I32S _moduleIndex, Tcc317xService * _DmbService);
extern I32S Tcc317xRemoveService(I32S _moduleIndex, I32S _identifier);
extern I32S Tcc317xReadIrqError(I32S _moduleIndex, I08U *_data);
extern I32S Tcc317xApiLockStatus(I32S _moduleIndex, I32S _diversityIndex, DmbLock_t *_tdmbLock);
extern I32S Tcc317xApiWaitLock(I32S _moduleIndex);
#if defined (_TCC317X_SUPPORT_AUTO_SEARCHING_)
extern I32S Tcc317xAutoSearch(I32S _moduleIndex, I32S _diversityIndex, I32U _BandSelect, Tcc317xAutoSearch_t *_Lockstate);
#endif
extern I32S Tcc317xIrqClear(I32S _moduleIndex);
extern I32S Tcc317xMailboxRead (I32S _moduleIndex, I32S _diversityIndex, I32U _command, mailbox_t * _mailbox);
extern I32S Tcc317xMailboxWrite (I32S _moduleIndex, I32S _diversityIndex, I32U _command, I32U * dataArray, I32S wordSize);
extern I32U Tcc317xWarmBoot (I32S _moduleIndex, I32S _diversityIndex,  I32S _sendStartMail);

static I32S Tcc317xApiCheckConditions (I32S _moduleIndex);
static I32S Tcc317xGetLockRegister (I08U _data, DmbLock_t * _tdmbLock);
void Tcc317xEventCallBackFunction (I32S _moduleIndex, I32S _diversityIndex, I32S _callBackEvent);

/* global values */
Tcc317xApiControl_t Tcc317xApiControl[TCC317X_MAX][TCC317X_DIVERSITY_MAX];
Tcc317xOption_t Tcc317xOption[TCC317X_MAX];
extern I08S Tcc317xCurrDiversityCount[TCC317X_MAX];

/* number of devices with EnumTcc317xBoardType */
I08S DevicesCount[7] = {1, 2, 3, 2, 3, 4, 1};
I08S DiversityCount[7] = {1, 1, 1, 2, 3, 4, 1};
/* service types matching with sub channel id */
I08U ServiceTypes[TCC317X_MAX][0x40];

/* please complete i2c setting, power control before api open */
I32S Tcc317xApiOpen (I32S _moduleIndex, Tcc317xOption_t *_Tcc317xOption, I32S _optionSize)
{
    I32U driverCoreVersion = 0x00;
    I32S ret = TCC317X_RETURN_FAIL;

    if(_optionSize > TCC317X_MAX*sizeof(Tcc317xOption_t))
    {
        TcpalPrintErr((I08S*)"#[E] Tcc317xApiOpen - Too much optionsize\n");
    }
    else
    {
        if((I32U)(_optionSize) < (I32U)(DiversityCount[_Tcc317xOption[0].boardType]) * sizeof(Tcc317xOption_t))
        {
            TcpalPrintErr((I08S*)"#[E] Tcc317xApiOpen - optionsize mismatch\n");
        }
        else
        {
            TcpalMemset (&Tcc317xApiControl[_moduleIndex][0], 0, sizeof (Tcc317xApiControl_t));
            driverCoreVersion = Tcc317xGetCoreVersion();
            TcpalPrintLog ((I08S*)"# TCC317X Driver Core Version [0x%02x. 0x%02x. 0x%02x]\n",
                ((driverCoreVersion>>16) & 0xFF),((driverCoreVersion>>8) & 0xFF),(driverCoreVersion & 0xFF));
            ret = Tcc317xOpen(_moduleIndex, _Tcc317xOption, Tcc317xEventCallBackFunction);
            Tcc317xApiControl[_moduleIndex][0].status = TCC317X_STATUS_OPEND;
        }
    }

    return ret;
}

I32S Tcc317xApiInit (I32S _moduleIndex, I08U *_coldbootData, I32S _codeSize)
{
    I32S ret;
    if(Tcc317xApiControl[_moduleIndex][0].status==TCC317X_STATUS_INITED)
    {
        TcpalPrintErr((I08S*)"# TCC317X already initialized\n");
        return TCC317X_RETURN_FAIL;
    }
    
    ret = Tcc317xInit(_moduleIndex, _coldbootData, _codeSize);
    Tcc317xApiControl[_moduleIndex][0].status = TCC317X_STATUS_INITED;
    return ret;
}

I32S Tcc317xApiInit_DiversityNormalMode (I32S _moduleIndex, I32S _diversityIndex, I08U *_coldbootData, I32S _codeSize)
{
    I32S ret;
    if(Tcc317xApiControl[_moduleIndex][_diversityIndex].status==TCC317X_STATUS_INITED)
    {
        TcpalPrintErr((I08S*)"# TCC317X already initialized\n");
        return TCC317X_RETURN_FAIL;
    }
    
    ret = Tcc317xInit_DiversityNormalMode(_moduleIndex, _diversityIndex, _coldbootData, _codeSize);
    Tcc317xApiControl[_moduleIndex][_diversityIndex].status = TCC317X_STATUS_INITED;
    return ret;
}

I32S Tcc317xApiClose(I32S _moduleIndex)
{
    I32S ret,i;

    if(Tcc317xApiControl[_moduleIndex][0].status == TCC317X_STATUS_NONE)
	return TCC317X_RETURN_FAIL;
     
    for(i=0;i<Tcc317xCurrDiversityCount[_moduleIndex];i++)
	 Tcc317xApiControl[_moduleIndex][i].status = TCC317X_STATUS_CLOSED;
    
    ret = Tcc317xClose(_moduleIndex);    
    return ret;
}

static I32S Tcc317xApiCheckConditions (I32S _moduleIndex)
{
    I32S ret = TCC317X_RETURN_SUCCESS;

    if(Tcc317xApiControl[_moduleIndex][0].status == TCC317X_STATUS_NONE ||
    	Tcc317xApiControl[_moduleIndex][0].status == TCC317X_STATUS_CLOSED)
    {
        ret = TCC317X_RETURN_FAIL;
    }
    else if(Tcc317xApiControl[_moduleIndex][0].status != TCC317X_STATUS_INITED)
    {
        ret = TCC317X_RETURN_FAIL;
    }

    return ret;
}

I32S Tcc317xApiLockStatus(I32S _moduleIndex, I32S _diversityIndex, DmbLock_t *_tdmbLock)
{
    I08U        lockRegister;

    Tcc317xApiRegisterRead(_moduleIndex, _diversityIndex, 0x0B, &lockRegister, 1);
    Tcc317xGetLockRegister(lockRegister, _tdmbLock);
    return TCC317X_RETURN_SUCCESS;
}

I32S Tcc317xApiWaitLock(I32S _moduleIndex)
{
    I32S        i, j;
    I08U        lockRegister;
    DmbLock_t   lock;
    DmbLock_t   lockSub;
    I32S        tickMs;
    I32S        tickCount;
    I32S        subLockOk;
    TcpalTime_t startTime;
    TcpalTime_t timeOut;

    /*
    * Fast Scan : find exist channel in 100ms!!
    * -------------------------------------------
    * 1. OFDM Detect : if not fail : (TDMB_OFDMDETECT_lock * TDMB_OFDMDETECT_RETRY) ms : 
    * 2. CTO & CFO Detect : if not fail :(TDMB_CTO_lock * TDMB_CTO_RETRY) + ((TDMB_CTO_lock+TDMB_CFO_lock) * TDMB_CFO_RETRY) ms;
    */

    tickMs = 10;

    /* OFDM Detect */
    subLockOk = 0;
    tickCount = (TDMB_OFDMDETECT_LOCK * TDMB_OFDMDETECT_RETRY) / tickMs;

    timeOut = (TDMB_OFDMDETECT_LOCK * TDMB_OFDMDETECT_RETRY);
    startTime = TcpalGetCurrentTimeCount_ms();

    for (i = 0; i < tickCount; i++)
    {
        TcpalMemset (&lock, 0x00, sizeof (DmbLock_t));
        for (j = 0; j < DiversityCount[Tcc317xOption[0].boardType]; j++)
        {
            Tcc317xApiRegisterRead(_moduleIndex, j, 0x0B, &lockRegister, 1);
            Tcc317xGetLockRegister(lockRegister, &lockSub);
            lock.AGC |= lockSub.AGC;
            lock.CTO |= lockSub.CTO;
            lock.CFO |= lockSub.CFO;
            lock.FTO |= lockSub.FTO;
            lock.SYNC_BYTE |= lockSub.SYNC_BYTE;
            lock.OFDM_DETECT |= lockSub.OFDM_DETECT;
        }
	
        if (lock.CTO && lock.CFO)
        {            
            return TCC317X_RETURN_SUCCESS;   /* lock Success */
        }

        if (lock.OFDM_DETECT)
        {
            subLockOk = 1;
            break;
        }
        else
        {
            if (TcpalGetTimeIntervalCount_ms (startTime) > timeOut)
            {
                return TCC317X_RETURN_FAIL;
            }
        }
        TcpalSleep (tickMs);
    }

    if (subLockOk == 0) /* lock Fail */
        return TCC317X_RETURN_FAIL;

    /* CTO & CFO Detect */
    subLockOk = 0;
    tickCount = ((TDMB_CTO_LOCK * TDMB_CTO_RETRY) + ((TDMB_CTO_LOCK + TDMB_CFO_LOCK) * TDMB_CFO_RETRY)) / tickMs;

    timeOut = ((TDMB_CTO_LOCK * TDMB_CTO_RETRY) + ((TDMB_CTO_LOCK + TDMB_CFO_LOCK) * TDMB_CFO_RETRY));
    startTime = TcpalGetCurrentTimeCount_ms ();

    for (i = 0; i < tickCount; i++)
    {
        TcpalMemset (&lock, 0x00, sizeof (DmbLock_t));
        for (j = 0; j < DiversityCount[Tcc317xOption[0].boardType]; j++)
        {
            Tcc317xApiRegisterRead(_moduleIndex, j, 0x0B, &lockRegister, 1);
            Tcc317xGetLockRegister(lockRegister, &lockSub);
            lock.AGC |= lockSub.AGC;
            lock.CTO |= lockSub.CTO;
            lock.CFO |= lockSub.CFO;
            lock.FTO |= lockSub.FTO;
            lock.SYNC_BYTE |= lockSub.SYNC_BYTE;
            lock.OFDM_DETECT |= lockSub.OFDM_DETECT;
        }
        if (lock.CTO && lock.CFO)
        {
            subLockOk = 1;
            return TCC317X_RETURN_SUCCESS;   /* lock Success */
        }
        else
        {
            if (TcpalGetTimeIntervalCount_ms (startTime) > timeOut)
            {
                return TCC317X_RETURN_FAIL;
            }
        }
        TcpalSleep(tickMs);
    }

    return TCC317X_RETURN_FAIL;
}

/* channel search - checking lock status. */
I32S Tcc317xApiChannelSearch(I32S _moduleIndex, I32S _frequency, I32U *_reservedOption)
{
    I32S subret;
    I32S ret;

    if(Tcc317xApiCheckConditions (_moduleIndex) != TCC317X_RETURN_SUCCESS)
    {
        ret = TCC317X_RETURN_FAIL;
    }
    else
    {
        subret = Tcc317xApiChannelSelect(_moduleIndex, _frequency, NULL);
        if(subret == TCC317X_RETURN_SUCCESS)
        {
            /* lock check */
            if(Tcc317xApiWaitLock(_moduleIndex)==TCC317X_RETURN_SUCCESS)
                ret = TCC317X_RETURN_SUCCESS;
            else
                ret = TCC317X_RETURN_FAIL;
        }
        else
        {
            ret = TCC317X_RETURN_FAIL;
        }
    }

    return ret;
}

/* channel select - no checking lock status. */
I32S Tcc317xApiChannelSelect(I32S _moduleIndex, I32S _frequency, I32U *_reservedOption)
{
    I32S subret;
    I32S ret;

    if(Tcc317xApiCheckConditions (_moduleIndex) != TCC317X_RETURN_SUCCESS)
    { 
        ret = TCC317X_RETURN_FAIL;
    }
    else
    {
        subret = Tcc317xTune(_moduleIndex, _frequency, _reservedOption);
        if(subret == TCC317X_RETURN_SUCCESS)
        {
            Tcc317xApiControl[_moduleIndex][0].frequency = (I32U)(_frequency);
            ret = TCC317X_RETURN_SUCCESS;
        }
        else
        {
            ret = TCC317X_RETURN_FAIL;
        }
    }
    
    return ret;
}

/* register read/write */
I32S Tcc317xApiRegisterRead(I32S _moduleIndex, I32S _diversityIndex, I08U _address, I08U *_data, I32U _size)
{
    I32S ret = TCC317X_RETURN_SUCCESS;

    if(Tcc317xApiControl[_moduleIndex][0].status == TCC317X_STATUS_NONE ||
    	Tcc317xApiControl[_moduleIndex][0].status == TCC317X_STATUS_CLOSED)
    {
        ret = TCC317X_RETURN_FAIL;
    }
    else
    {
        ret = Tcc317xRead(_moduleIndex, _diversityIndex, _address, _data, _size);
    }

    return ret;
}

I32S Tcc317xApiRegisterWrite(I32S _moduleIndex, I32S _diversityIndex, I08U _address, I08U *_data, I32U _size)
{
    I32S ret = TCC317X_RETURN_SUCCESS;

    if(Tcc317xApiControl[_moduleIndex][0].status == TCC317X_STATUS_NONE ||
    	Tcc317xApiControl[_moduleIndex][0].status == TCC317X_STATUS_CLOSED)
    {
        ret = TCC317X_RETURN_FAIL;
    }
    else
    {
        ret = Tcc317xWrite(_moduleIndex, _diversityIndex, _address, _data, _size);
    }

    return ret;

}

/* single read */
I32S Tcc317xApiRfRegisterRead(I32S _moduleIndex, I32S _diversityIndex, I08U _address, I32U *_data)
{
    I32S ret = TCC317X_RETURN_SUCCESS;

    if(Tcc317xApiCheckConditions (_moduleIndex) != TCC317X_RETURN_SUCCESS)
    {
        ret = TCC317X_RETURN_FAIL;
    }
    else
    {
        ret = Tcc317xRfRegisterRead(_moduleIndex, _diversityIndex, _address, _data);
    }

    return ret;

}

/* single write */
I32S Tcc317xApiRfRegisterWrite(I32S _moduleIndex, I32S _diversityIndex, I08U _address, I32U _data)
{
    I32S ret = TCC317X_RETURN_SUCCESS;

    if(Tcc317xApiCheckConditions (_moduleIndex) != TCC317X_RETURN_SUCCESS)
    {
        ret = TCC317X_RETURN_FAIL;
    }
    else
    {
        ret = Tcc317xRfRegisterWrite(_moduleIndex, _diversityIndex, _address, _data);
    }
    return ret;
}


/* service control */
I32S Tcc317xApiAddService(I32S _moduleIndex, Tcc317xService * _service)
{
    I32S ret = TCC317X_RETURN_SUCCESS;

    if(Tcc317xApiCheckConditions (_moduleIndex) != TCC317X_RETURN_SUCCESS)
    {
        ret = TCC317X_RETURN_FAIL;
    }
    else
    {
        ServiceTypes[_moduleIndex][_service->identifier] = (I08U)(_service->serviceType);
        ret = Tcc317xAddService(_moduleIndex, _service);
    }

    return ret;
}

I32S Tcc317xApiRemoveService(I32S _moduleIndex, I32S _identifier)
{
    I32S ret = TCC317X_RETURN_SUCCESS;

    if(Tcc317xApiCheckConditions (_moduleIndex) != TCC317X_RETURN_SUCCESS)
    {
        ret = TCC317X_RETURN_FAIL;
    }
    else
    {
        ret = Tcc317xRemoveService(_moduleIndex, _identifier);
    }

    return ret;
}

/* stream control */
I32S Tcc317xApiStreamStop(I32S _moduleIndex)
{
    I32S ret = TCC317X_RETURN_SUCCESS;

    if(Tcc317xApiCheckConditions (_moduleIndex) != TCC317X_RETURN_SUCCESS)
    {
        ret = TCC317X_RETURN_FAIL;
    }
    else
    {
        ret = Tcc317xStreamStop(_moduleIndex);
    }

    return ret;
}

I32S Tcc317xApiStreamStart(I32S _moduleIndex)
{
    I32S ret = TCC317X_RETURN_SUCCESS;

    if(Tcc317xApiCheckConditions (_moduleIndex) != TCC317X_RETURN_SUCCESS)
    {
        ret = TCC317X_RETURN_FAIL;
    }
    else
    {
        ret = Tcc317xStreamStart(_moduleIndex);
    }

    return ret;
}

/* for interrupt handler */
I32S Tcc317xApiStreamRead(I32S _moduleIndex, I08U *_data, I32S _size)
{
    I32S ret = TCC317X_RETURN_SUCCESS;

    if(Tcc317xApiCheckConditions (_moduleIndex) != TCC317X_RETURN_SUCCESS)
    {
        ret = TCC317X_RETURN_FAIL;
    }
    else
    {
        ret = Tcc317xStreamRead(_moduleIndex, _data, _size);
    }
    return ret;
}

I32S Tcc317xApiMemoryWrite(I32S _moduleIndex, I32S _diversityIndex, I32U _address, I08U *_data, I32U _size)
{
    I32S ret = TCC317X_RETURN_SUCCESS;

    if(Tcc317xApiCheckConditions (_moduleIndex) != TCC317X_RETURN_SUCCESS)
    {
        ret = TCC317X_RETURN_FAIL;
    }
    else
    {
        ret = Tcc317xMemoryWrite(_moduleIndex, _diversityIndex, _address, _data, _size);
    }
    return ret;
}


I32S Tcc317xApiMemoryRead(I32S _moduleIndex, I32S _diversityIndex, I32U _address, I08U *_data, I32U _size)
{
    I32S ret = TCC317X_RETURN_SUCCESS;

    if(Tcc317xApiCheckConditions (_moduleIndex) != TCC317X_RETURN_SUCCESS)
    {
        ret = TCC317X_RETURN_FAIL;
    }
    else
    {
        ret = Tcc317xMemoryRead(_moduleIndex, _diversityIndex, _address, _data, _size);
    }
    return ret;
}

I32S Tcc317xApiGetIrqError(I32S _moduleIndex, I08U *_data)
{
    I32S ret = TCC317X_RETURN_SUCCESS;

    if(Tcc317xApiCheckConditions (_moduleIndex) != TCC317X_RETURN_SUCCESS)
    {
        ret = TCC317X_RETURN_FAIL;
    }
    else
    {
        ret = Tcc317xReadIrqError(_moduleIndex, _data);
    }
    return ret;
}

I32S Tcc317xApiIrqClear(I32S _moduleIndex)
{
    I32S ret = TCC317X_RETURN_SUCCESS;

    if(Tcc317xApiCheckConditions (_moduleIndex) != TCC317X_RETURN_SUCCESS)
    {
        ret = TCC317X_RETURN_FAIL;
    }
    else
    {
        ret = Tcc317xIrqClear(_moduleIndex);
    }
    return ret;
}

I32S Tcc317xApiMailboxWrite (I32S _moduleIndex, I32S _diversityIndex, I32U _command, I32U *dataArray, I32S wordSize)
{
    I32S ret = TCC317X_RETURN_SUCCESS;

    if(Tcc317xApiCheckConditions (_moduleIndex) != TCC317X_RETURN_SUCCESS)
    {
        ret = TCC317X_RETURN_FAIL;
    }
    else
    {
        ret = Tcc317xMailboxWrite(_moduleIndex, _diversityIndex, _command, dataArray, wordSize);
    }
    return ret;
}

I32S Tcc317xApiMailboxRead (I32S _moduleIndex, I32S _diversityIndex, I32U _command, mailbox_t * _mailbox)
{
    I32S ret = TCC317X_RETURN_SUCCESS;

    if(Tcc317xApiCheckConditions (_moduleIndex) != TCC317X_RETURN_SUCCESS)
    {
        ret = TCC317X_RETURN_FAIL;
    }
    else
    {
        ret = Tcc317xMailboxRead(_moduleIndex, _diversityIndex, _command, _mailbox);
    }
    return ret;
}

I32S Tcc317xApiWarmBoot (I32S _moduleIndex, I32S _diversityIndex)
{
    I32S ret = TCC317X_RETURN_SUCCESS;

    if(Tcc317xApiCheckConditions (_moduleIndex) != TCC317X_RETURN_SUCCESS)
    {
        ret = TCC317X_RETURN_FAIL;
    }
    else
    {
	ret = Tcc317xWarmBoot (_moduleIndex, _diversityIndex, 1);

    }
    return ret;
}

/* other user command for special usages */
I32S Tcc317xApiUserCommand(I32S _moduleIndex, I32S _diversityIndex, I32S _command, void *_param1, void *_param2, void *_param3, void *_param4)
{
    I32S ret = TCC317X_RETURN_SUCCESS;

    if(Tcc317xApiCheckConditions (_moduleIndex) != TCC317X_RETURN_SUCCESS)
    {
        ret = TCC317X_RETURN_FAIL;
    }
    else
    {
        /* reserved to debugging or patch */        
    }

    return ret;
}

#if defined (_TCC317X_SUPPORT_AUTO_SEARCHING_)
I32S Tcc317xApiAutoSearch(I32S _moduleIndex, I32S _diversityIndex, I32U _BandSelect, Tcc317xAutoSearch_t *_Lockstate)
{
	I32S ret = TCC317X_RETURN_SUCCESS;

	if(Tcc317xAutoSearch(_moduleIndex, _diversityIndex, _BandSelect, _Lockstate) != TCC317X_RETURN_SUCCESS)
	{
		ret  = TCC317X_RETURN_FAIL;
	}
    else{ /* reserved */ }

    return ret;
}
#endif

static void Parse_TDMBSyncStat (DmbLock_t * pTDMBLock, I08U _input)
{
    pTDMBLock->AGC = (_input) & 0x01;
    pTDMBLock->CTO = (_input >> 1) & 0x01;
    pTDMBLock->CFO = (_input >> 2) & 0x01;
    pTDMBLock->FTO = (_input >> 3) & 0x01;
    pTDMBLock->SYNC_BYTE = (_input >> 4) & 0x01;
    pTDMBLock->OFDM_DETECT = (_input >> 5) & 0x01;
}

static I32S Tcc317xGetLockRegister (I08U _data, DmbLock_t * _tdmbLock)
{
    Parse_TDMBSyncStat (_tdmbLock, _data);
    return TCC317X_RETURN_SUCCESS;
}

I32U Tcc317xApiGetCurrentFrequency(I32S _moduleIndex) 
{
    return (Tcc317xApiControl[_moduleIndex][0].frequency);
}

void Tcc317xEventCallBackFunction (I32S _moduleIndex, I32S _diversityIndex, I32S _callBackEvent) 
{
    TcpalPrintErr ((I08S*)"#[E] Event CallBack Function [%d][%d] event[%d]\n",
                   _moduleIndex, _diversityIndex, _callBackEvent);

    switch(_callBackEvent) {
    case TCC317X_EVENT_COMMAND_READ_FAIL:
        break;
    case TCC317X_EVENT_COMMAND_WRITE_FAIL:
        break;
    case TCC317X_EVENT_MAILBOX_FAIL:
        break;
    case TCC317X_EVENT_UNKNOWN:
        break;
    default:
        break;
    }
}

/*---------------------------------------------------------------------
 * Function name
 * 	Tcc317xApiGetDriverVersion
 * Description
 * 	Get driver core version
 * Parameters
 * 	_version : version data
 * Return value
 *
 * Remark
 *	[23:16] High version information	(Major version)
 * 	[15: 8] Middle version information	(Major version)
 *	[ 7: 0] Low version information		(Minor version)
 ---------------------------------------------------------------------*/
I32S Tcc317xApiGetDriverVersion(I32U *_version)
{
	_version[0] =Tcc317xGetCoreVersion();
	return TCC317X_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------
 * Function name
 * 	Tcc317xApiGetDSPCodeVersion
 * Description
 * 	Get dsp code version
 * Parameters
 * 	
 * Return value
 * 	
 * Remark
 *	[32:24] High version information	(Major version)
 * 	[23:16] Middle version information	(Major version)
 *	[15: 8] Low version information		(Minor version)
 *	[ 7: 0] Type information
 ---------------------------------------------------------------------*/
I32S Tcc317xApiGetDSPCodeVersion(I32S _moduleIndex, I32S _diversityIndex, I32U *_version)
{
	if(Tcc317xApiCheckConditions (_moduleIndex) != TCC317X_RETURN_SUCCESS)
	    return TCC317X_RETURN_FAIL;

	_version[0] = Tcc317xGetDspCodeVersion(_moduleIndex,_diversityIndex);
	return TCC317X_RETURN_SUCCESS;
}

#if 0 /* example source */
/*---------------------------------------------------------------------
 * Function name
 * 	Tcc317xApiGetCurrentTxMode
 * Description
 * 	Getting current Tx mode
 * Parameters
 * 	_moduleIndex
 *		0 : BaseBand#0
 * 		1 : BaseBand#1
 * Return value
 * 	TCC317X_RETURN_FAIL : fail
 * 	Others : Tx mode values
 * Remark
 * 	
 ---------------------------------------------------------------------*/
I32S Tcc317xApiGetCurrentTxMode (I32S _moduleIndex) 
{
    I32S ret;
    I32S txmode;
    mailbox_t mail;
    I08U lockRegister;
    DmbLock_t lock;

#ifndef MBCMD_CTO_DAB_RESULT
#define MBCMD_CTO_DAB_RESULT	((0x04<<11) | 0x01)
#endif

    ret = Tcc317xApiMailboxRead (_moduleIndex, 0, MBCMD_CTO_DAB_RESULT, &mail);
    if(ret==TCC317X_RETURN_FAIL)
    	return TCC317X_RETURN_FAIL;

    Tcc317xApiRegisterRead(_moduleIndex, 0, 0x0B, &lockRegister, 1);
    Tcc317xGetLockRegister(lockRegister, &lock);

    if(!lock.CTO)
	return TCC317X_RETURN_FAIL;
    else
    	txmode = (mail.data_array[1]&0x0F);

    if(txmode>4 || txmode<1)
        return TCC317X_RETURN_FAIL;

    return txmode;
}

/*---------------------------------------------------------------------
 * Function name
 * 	Tcc317xApiSetTIICfg
 * Description
 * 	Set TII enable or disable
 * Parameters
 * 	_moduleIndex
 *		0 : BaseBand#0
 * 		1 : BaseBand#1
 * 	_enableFlag
 *		0 : TII disable
 * 		1 : TII enable
 * Return value
 * 	TCC317X_RETURN_SUCCESS : success
 * 	Others : fail
 * Remark
 * 	For enable, call this function after every tunning function.
 ---------------------------------------------------------------------*/
I32S Tcc317xApiSetTIICfg(I32S _moduleIndex, I32U _enableFlag)
{
    I32S ret;
    I32U data = 0;

#ifndef MBCMD_TII_CFG
#define MBCMD_TII_CFG				((0x03 << 11) | 0x01)
#endif

    if(_enableFlag)
        data = 1;
    else
	data = 0;

    ret = Tcc317xApiMailboxWrite (_moduleIndex, 0, MBCMD_TII_CFG, &data, 1);
    if(ret==TCC317X_RETURN_FAIL)
	return TCC317X_RETURN_FAIL;

    return TCC317X_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------
 * Function name
 * 	Tcc317xApiGetTIIResult
 * Description
 * 	Get TII values
 * Parameters
 * 	_moduleIndex
 *		0 : BaseBand#0
 * 		1 : BaseBand#1
 * 	_tiiResult : unsigned int * 8 array
 *		0 : TII disable
 * 		1 : TII enable
 * Return value
 * 	TCC317X_RETURN_SUCCESS : success
 * 	Others : fail
 * Remark
 *	It return valid value after cfo lock + 300 ms.
 * 	_tiiResult format :
 *		Main-ID[27:21] Sub-ID[20:16] Energy[15:0]
 ---------------------------------------------------------------------*/
I32S Tcc317xApiGetTIIResult(I32S _moduleIndex, I32U *_tiiResult)
{
    I32S ret;
    mailbox_t mail;

#ifndef MBCMD_TII_RESULT_1
#define MBCMD_TII_RESULT_1			((0x03 << 11) | 0x02)
#endif

#ifndef MBCMD_TII_RESULT_2
#define MBCMD_TII_RESULT_2			((0x03 << 11) | 0x03)
#endif

    ret = Tcc317xApiMailboxRead (_moduleIndex, 0, MBCMD_TII_RESULT_1, &mail);
    if(ret==TCC317X_RETURN_FAIL)
    	return TCC317X_RETURN_FAIL;
    TcpalMemcpy(&_tiiResult[0], &mail.data_array[0], sizeof(I32U)*4);

    ret = Tcc317xApiMailboxRead (_moduleIndex, 0, MBCMD_TII_RESULT_2, &mail);
    if(ret==TCC317X_RETURN_FAIL)
    	return TCC317X_RETURN_FAIL;
    TcpalMemcpy(&_tiiResult[4], &mail.data_array[0], sizeof(I32U)*4);

    return TCC317X_RETURN_SUCCESS;
}

#endif

#if 0 /* not support yet */
/*---------------------------------------------------------------------
 * Function name
 * 	Tcc317xApiChangeToDiversityMode
 * Description
 * 	Change dual mode to diversity mode
 * Parameters
 * 	_mergeIndex : merge index
 *		0 : BaseBand#0 (Single : default, Dual : BaseBand#0)
 * 		1 : BaseBand#1 (Single : Not use, Dual : BaseBand#1)
 * 	_Tcc317xOption : Options for driver setting
 * 	_optionSize : size of option data
 * Return value
 * 	Refer EnumReturn
 * Remark
 * 	Especially when using 2chips
 ---------------------------------------------------------------------*/
I32S Tcc317xApiChangeToDiversityMode (I32S _mergeIndex, 
				      Tcc317xOption_t * _Tcc317xOption,
				      I32S _optionSize)
{
	I32S i;
	I32S ret;
	I32S numberOfBaseband;

	numberOfBaseband = Tcc317xApiControl[0][0].NumberofBaseband;
	numberOfBaseband += Tcc317xApiControl[1][0].NumberofBaseband;

	if(_mergeIndex>numberOfBaseband)
		return TCC317X_RETURN_FAIL;

	if(_optionSize != sizeof(Tcc317xOption_t)*numberOfBaseband)
		return TCC317X_RETURN_FAIL;

	ret = Tcc317xChangeToDiversityMode(_mergeIndex, _Tcc317xOption);

	for(i = 0; i<numberOfBaseband-_mergeIndex; i++) {
		Tcc317xApiControl[0][_mergeIndex+i].status = 
						TCC317X_STATUS_INITED;
		Tcc317xApiControl[0][_mergeIndex+i].NumberofBaseband = 
						numberOfBaseband;
	}

	for(i = 0; i<_mergeIndex; i++)
		Tcc317xApiControl[0][i].NumberofBaseband = 0;
	
	Tcc317xApiControl[1][0].status = TCC317X_STATUS_CLOSED;

	return ret;
}

/*---------------------------------------------------------------------
 * Function name
 * 	Tcc317xApiChangeToDualMode
 * Description
 * 	Change diversity mode to dual mode
 * Parameters
 * 	_devideIndex : devide index
 *		0 : BaseBand#0 (Single : default, Dual : BaseBand#0)
 * 		1 : BaseBand#1 (Single : Not use, Dual : BaseBand#1)
 * 	_Tcc317xOption : Options for driver setting
 * 	_optionSize : size of option data
 * Return value
 * 	Refer EnumReturn
 * Remark
 * 	Especially when using 2chips
 ---------------------------------------------------------------------*/
I32S Tcc317xApiChangeToDualMode (I32S _devideIndex, 
				 Tcc317xOption_t * _Tcc317xOption,
				 I32S _optionSize)
{
	I32S i;
	I32S ret;
	I32S numberOfBaseband;

	numberOfBaseband = Tcc317xApiControl[0][0].NumberofBaseband;

	if(_devideIndex>numberOfBaseband)
		return TCC317X_RETURN_FAIL;
	
	if(_optionSize != sizeof(Tcc317xOption_t)*numberOfBaseband)
		return TCC317X_RETURN_FAIL;

	ret = Tcc317xChangeToDualMode(_devideIndex, _Tcc317xOption);

	/* to disable mailbox access */
	for(i = _devideIndex; i<numberOfBaseband; i++)
		Tcc317xApiControl[0][i].status = TCC317X_STATUS_CLOSED;
	for(i = 0; i<_devideIndex; i++)
		Tcc317xApiControl[0][i].NumberofBaseband = _devideIndex;

	for(i = 0; i<numberOfBaseband-_devideIndex; i++) {
		Tcc317xApiControl[1][i].status = TCC317X_STATUS_INITED;
		Tcc317xApiControl[1][i].NumberofBaseband = 
					numberOfBaseband-_devideIndex;
	}

	return ret;
}
#endif /* not support yet */

