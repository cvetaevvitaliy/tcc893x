/****************************************************************************
 *   FileName    : tcc317x_tdmb.c
 *   Description : TDMB Functions
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

#include "tcc317x_tdmb.h"
#include "tcc317x_mailbox.h"
#include "tcpal_os.h"

extern Tcc317xHandle_t Tcc317xHandle[TCC317X_MAX][TCC317X_DIVERSITY_MAX];
extern I08S Tcc317xCurrDiversityCount[4];

static void SetTDMBMSCFormat (Tcc317xHandle_t *_handle);
static I32S Tcc317xCheckMaxService(I32S _moduleIndex);
static I32S Tcc317xCheckDuplicatedService(I32S _moduleIndex, Tcc317xService * _DmbService);
static I32S Tcc317xCheckExistService(I32S _moduleIndex, I32S _identifier, I32S *_index);
static I32S Tcc317xRegisterService (I32S _moduleIndex, Tcc317xService * _service);
static I32S Tcc317xUnRegisterService (I32S _moduleIndex, I32S _index);
static void PushMCI_SortStartCU(I32S _moduleIndex, I32U *_pushData);


TdmbControl_t TdmbControl[TCC317X_MAX];

void InitTdmbProcess (Tcc317xHandle_t *_handle)
{
    TcpalMemset(&TdmbControl[_handle->moduleIndex], 0x00, sizeof(TdmbControl_t));
    SetTDMBMSCFormat (_handle);
}

void SetTDMBMSCFormat (Tcc317xHandle_t *_handle)
{
    I32U data;
    data = 0x7F;

#if defined (ENABLE_GARBAGE_STREAM)
    if(_handle->options.streamInterface == TCC317X_STREAM_IO_MAINIO)
        data = 0xFF;    /* Garbage Data on */
#endif

    Tcc317xMailboxTxOnly (_handle, MBPARA_SYS_DAB_STREAM_SET, &data, 1);
}

I32S Tcc317xCheckMaxService(I32S _moduleIndex)
{
    if (TdmbControl[_moduleIndex].numberOfService > (MAX_TDMBMSC_NUM - 1))
    {
        TcpalPrintErr ((I08S*)"# MAX Service Over\n");
        return TCC317X_RETURN_FAIL;
    }

    return TCC317X_RETURN_SUCCESS;
}

I32S Tcc317xCheckDuplicatedService(I32S _moduleIndex, Tcc317xService * _DmbService)
{
    I32S i;

    for (i = 0; i < MAX_TDMBMSC_NUM; i++)
    {       
        if ((_DmbService->identifier == TdmbControl[_moduleIndex].DmbService[i].identifier) && TdmbControl[_moduleIndex].onAirState[i] == 1)
        {
            TcpalPrintErr ((I08S*)"# Already Exist Service[%d]\n", _DmbService->identifier);
            return TCC317X_RETURN_FAIL;
        }
    }

    return TCC317X_RETURN_SUCCESS;
}

I32S Tcc317xCheckExistService(I32S _moduleIndex, I32S _identifier, I32S *_index)
{
    I32S i;
    I32S findid = 0;

    _index[0] = 0;
    if (TdmbControl[_moduleIndex].numberOfService < (1))
    {
//        TcpalPrintErr ((I08S*)"# Current, No Service.\n");
        return TCC317X_RETURN_FAIL;
    }

    /* no service, return */
    for (i = 0; i < MAX_TDMBMSC_NUM; i++)
    {
        if ((_identifier == TdmbControl[_moduleIndex].DmbService[i].identifier) && TdmbControl[_moduleIndex].onAirState[i] == 1)
        {
            findid = 1;
            _index[0] = i;
            break;
        }
    }

    if (!findid)
    {
        TcpalPrintErr ((I08S*)"# Can't Find old Serivce\n");
        return TCC317X_RETURN_FAIL;
    }
    
    return TCC317X_RETURN_SUCCESS;
}

I32S Tcc317xAddService(I32S _moduleIndex, Tcc317xService * _DmbService)
{
    /* Add Service */
    I32S ret = TCC317X_RETURN_SUCCESS;

    if(!Tcc317xHandle[_moduleIndex][0].opened)
        return TCC317X_RETURN_FAIL_NULL_ACCESS;

    TcpalPrintLog ((I08S*)"# [Command] Add Service  [%d]\n", _DmbService->identifier);

    if(Tcc317xCheckMaxService(_moduleIndex) != TCC317X_RETURN_SUCCESS)
        return TCC317X_RETURN_FAIL;

    if(Tcc317xCheckDuplicatedService(_moduleIndex, _DmbService) != TCC317X_RETURN_SUCCESS)
        return TCC317X_RETURN_FAIL;

    ret = Tcc317xRegisterService(_moduleIndex, _DmbService);
    return ret;
}

I32S Tcc317xRemoveService(I32S _moduleIndex, I32S _identifier)
{
    I32S searchIndex;

    if(!Tcc317xHandle[_moduleIndex][0].opened)
        return TCC317X_RETURN_FAIL_NULL_ACCESS;

    /* Remove Service */
    TcpalPrintLog ((I08S*)"# [Command] Remove Service  [%d]\n", _identifier);
    if(Tcc317xCheckExistService(_moduleIndex, _identifier, &searchIndex) != TCC317X_RETURN_SUCCESS)
        return TCC317X_RETURN_FAIL;

    Tcc317xUnRegisterService (_moduleIndex, searchIndex);
    return TCC317X_RETURN_SUCCESS;
}

I32S Tcc317xRegisterService (I32S _moduleIndex, Tcc317xService * _service)
{
    I32S i;
    I32S idx = 0;
    I32S sindx= 0;
    I32U data = 0;
    I32U protocolData[12];
    I32S DataMode, Soft_bit;
    I32S dmbChIdx = -1;

    switch(_service->serviceType)
    {
        case SRVTYPE_DAB:
            DataMode = 0;
            break;

        case SRVTYPE_DABPLUS:
            /* RS Decocding - not send rs parity    */
            /* DataMode = 3;                        */

            /* RS not Decocde   */
            DataMode = 0; 
            break;

        case SRVTYPE_DATA:
            DataMode = 0;
            break;

        case SRVTYPE_DMB:
            DataMode = 1;
            break;

        default:
            DataMode = 2;
            break;
    }

    TcpalMemset (protocolData, 0x00, 4 * 12);

    /* find reg idx */
    for (i = 0; i < MAX_TDMBMSC_NUM; i++)
    {
        if (TdmbControl[_moduleIndex].onAirState[i] == 0)
        {
            idx = i * 2;
            sindx = i;
            break;
        }
    }

    if (_service->serviceType == SRVTYPE_DMB) {
        for (i = 0; i < MAX_DMB_NUM; i++)
        {
            if (TdmbControl[_moduleIndex].onAirDMBChannelFlag[i] == 0)
            {
                dmbChIdx = i;
                TdmbControl[_moduleIndex].onAirDMBChannelFlag[i] = 1;
                break;
            }
        }

        if(dmbChIdx<0) {
            TcpalPrintLog ((I08S*)"# Can't Assign DmbChannel Id\n");
            return TCC317X_RETURN_FAIL;
        }
    }

    Soft_bit = 0;
    TcpalMemcpy (&TdmbControl[_moduleIndex].DmbService[sindx], _service, sizeof (Tcc317xService));

    TdmbControl[_moduleIndex].OnAirServiceType[sindx] = (I08U)(_service->serviceType);
    TdmbControl[_moduleIndex].onAirState[sindx] = 1;
    TdmbControl[_moduleIndex].mciCount++;
    
    if (_service->serviceType == SRVTYPE_DMB)
        TdmbControl[_moduleIndex].dmbChannelId[sindx] = dmbChIdx;
    else
        TdmbControl[_moduleIndex].dmbChannelId[sindx] = 0;

    TdmbControl[_moduleIndex].protocolData[idx] = _service->reConfig << 26 
                                                | _service->identifier << 20 
                                                | _service->cuSize << 10 
                                                | _service->startCu;
    TdmbControl[_moduleIndex].protocolData[idx + 1] = TdmbControl[_moduleIndex].dmbChannelId[sindx] << 20 
                                                | Soft_bit << 18
                                                | DataMode << 16 
                                                | _service->pType << 11 
                                                | _service->pLevel << 8 
                                                | _service->bitrate;
    TdmbControl[_moduleIndex].numberOfService++;

    if(Soft_bit == 1 || Soft_bit == 2)
        data = TdmbControl[_moduleIndex].mciCount+(1<<16);   /* in case soft bit = 4, 5*/
    else
        data = TdmbControl[_moduleIndex].mciCount;

    PushMCI_SortStartCU(_moduleIndex, &TdmbControl[_moduleIndex].protocolData[0]);
    
    for(i=0; i<Tcc317xCurrDiversityCount[_moduleIndex]; i++)
    {
        Tcc317xMailboxTxOnly (&Tcc317xHandle[_moduleIndex][i], MBPARA_SYS_DAB_MCI_UPDATE, &data, 1);
    }
    
    TdmbControl[_moduleIndex].protocolData[idx] &= 0xf3ffffff;

    TcpalPrintLog ((I08S*)"# identifier(subchannel id):%d, cusize:%d, startcu:%d, ptype:%d, plevel:%d, bitrate:%d, rs=%d, soft_bit=%d, reconfg=%d, DataMode=%d\n",
        _service->identifier, _service->cuSize, _service->startCu, _service->pType, _service->pLevel, 
        _service->bitrate, _service->rsOn, Soft_bit, _service->reConfig, DataMode);

    return TCC317X_RETURN_SUCCESS;
}

I32S Tcc317xUnRegisterService (I32S _moduleIndex, I32S _index)
{
    I32S i = 0;
    I32U data = 0;
    I32S dmbChIdx = 0;

    TcpalMemset (&TdmbControl[_moduleIndex].DmbService[_index], 0x00, sizeof (Tcc317xService));
    TdmbControl[_moduleIndex].protocolData[_index * 2] = 0x00;
    TdmbControl[_moduleIndex].protocolData[_index * 2 + 1] = 0x00;
    TdmbControl[_moduleIndex].numberOfService--;
    TdmbControl[_moduleIndex].onAirState[_index] = 0;

    if(TdmbControl[_moduleIndex].OnAirServiceType[_index]==SRVTYPE_DMB) {
	    dmbChIdx = TdmbControl[_moduleIndex].dmbChannelId[_index];
	    if(dmbChIdx>=0 && dmbChIdx<MAX_DMB_NUM)
		    TdmbControl[_moduleIndex].onAirDMBChannelFlag[dmbChIdx] = 0;
    }
    TdmbControl[_moduleIndex].dmbChannelId[_index] = 0;
    TdmbControl[_moduleIndex].OnAirServiceType[_index] = SRVTYPE_NONE;

    TdmbControl[_moduleIndex].mciCount--;
    data = TdmbControl[_moduleIndex].mciCount;

    PushMCI_SortStartCU(_moduleIndex, &TdmbControl[_moduleIndex].protocolData[0]);

    for(i=0; i<Tcc317xCurrDiversityCount[_moduleIndex]; i++)
    {
        Tcc317xMailboxTxOnly (&Tcc317xHandle[_moduleIndex][i], MBPARA_SYS_DAB_MCI_UPDATE, &data, 1);
    }

    TcpalPrintLog ((I08S*)"# removed index [%d]\n", _index);
    return TCC317X_RETURN_SUCCESS;
}

void PushMCI_SortStartCU(I32S _moduleIndex, I32U *_pushData)
{
    I32S i;
    I32S j;
    I32S StartCU[MAX_TDMBMSC_NUM];
    I32U RemapData[MAX_TDMBMSC_NUM*2];

    TcpalMemcpy(&RemapData[0], _pushData, sizeof(I32U)*MAX_TDMBMSC_NUM*2);

    for(i=0; i<MAX_TDMBMSC_NUM; i++)
    {
        if(_pushData[i*2]!=0)
            StartCU[i] = (_pushData[i*2] & 0x3FF);
        else
            StartCU[i] = 0x3FF;
    }

    for(i=0; i<MAX_TDMBMSC_NUM; i++)
    {
        for(j=i+1; j<MAX_TDMBMSC_NUM; j++)
        {
            if(StartCU[i]>StartCU[j])
            {
                I32U Data[2];
                I32S Temp;

                Temp = StartCU[i];
                Data[0] = RemapData[i*2];
                Data[1] = RemapData[i*2+1];

                StartCU[i] = StartCU[j];
                RemapData[i*2] = RemapData[j*2];
                RemapData[i*2+1] = RemapData[j*2+1];

                StartCU[j] = Temp;
                RemapData[j*2] = Data[0];
                RemapData[j*2+1] = Data[1];
            }
        }
    }

    for(i=0; i<Tcc317xCurrDiversityCount[_moduleIndex]; i++)
    {
        Tcc317xMailboxTxOnly (&Tcc317xHandle[_moduleIndex][i], MBPARA_SEL_CH_INFO_PRAM, &RemapData[0], 6);
        Tcc317xMailboxTxOnly (&Tcc317xHandle[_moduleIndex][i], MBPARA_SEL_CH_INFO_PRAM+1, &RemapData[6], 6);
    }

#if 0
    TcpalPrintLog((I08S*)"# 0: 0x%08x\n", RemapData[0]);
    TcpalPrintLog((I08S*)"# 1: 0x%08x\n", RemapData[1]);
    TcpalPrintLog((I08S*)"# 2: 0x%08x\n", RemapData[2]);
    TcpalPrintLog((I08S*)"# 3: 0x%08x\n", RemapData[3]);
    TcpalPrintLog((I08S*)"# 4: 0x%08x\n", RemapData[4]);
    TcpalPrintLog((I08S*)"# 5: 0x%08x\n", RemapData[5]);
    TcpalPrintLog((I08S*)"# 6: 0x%08x\n", RemapData[6]);
    TcpalPrintLog((I08S*)"# 7: 0x%08x\n", RemapData[7]);
    TcpalPrintLog((I08S*)"# 8: 0x%08x\n", RemapData[8]);
    TcpalPrintLog((I08S*)"# 9: 0x%08x\n", RemapData[9]);
    TcpalPrintLog((I08S*)"# A: 0x%08x\n", RemapData[10]);
    TcpalPrintLog((I08S*)"# B: 0x%08x\n", RemapData[11]);
#endif
}

