/****************************************************************************
 *   FileName    : tcc317x_command_control.c
 *   Description : control command Function
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

#include "tcpal_os.h"
#include "tcc317x_command_control.h"
#include "tcc317x_register_control.h"

#define PHY_BASE_ADDR						(0x80000000)

extern Tcc317xHandle_t Tcc317xHandle[TCC317X_MAX][TCC317X_DIVERSITY_MAX];
extern TcpalSemaphore_t Tcc317xInterfaceSema;

/* baseband memory control*/
I32S Tcc317xMemoryRead(I32S _moduleIndex, I32S _diversityIndex, I32U _address, I08U *_data, I32U _size)
{
    I08U inputValue[4];
    Tcc317xHandle_t *h;

    if(!Tcc317xHandle[_moduleIndex][_diversityIndex].opened)
        return TCC317X_RETURN_FAIL_NULL_ACCESS;
        
    h = &Tcc317xHandle[_moduleIndex][_diversityIndex];
    if (!(_size > 0))
	return TCC317X_RETURN_FAIL;

    _address |= PHY_BASE_ADDR;
    TcpalSemaphoreLock(&Tcc317xInterfaceSema);
    Tcc317xSetRegDmaControl (h, TC3XREG_CMDDMA_DMAEN | TC3XREG_CMDDMA_READMODE , _UNLOCK_);
    inputValue[0] = (I08U)(((_address>>24)&0xFF));
    inputValue[1] = (I08U)(((_address>>16)&0xFF));
    inputValue[2] = (I08U)(((_address>>8)&0xFF));
    inputValue[3] = (I08U)((_address&0xFF));
    Tcc317xSetRegDmaSourceAddress (h, &inputValue[0], _UNLOCK_);
    inputValue[0] = (I08U)((((_size >> 2)>>8)&0xFF));
    inputValue[1] = (I08U)(((_size >> 2)&0xFF));
    Tcc317xSetRegDmaSize (h, &inputValue[0], _UNLOCK_);
    Tcc317xSetRegDmaStartControl (h, TC3XREG_CMDDMA_START_AUTOCLR | TC3XREG_CMDDMA_INIT_AUTOCLR | TC3XREG_CMDDMA_CRC32INIT_AUTOCLR, _UNLOCK_);
    Tcc317xGetRegDataWindow (h, _data, _size, _UNLOCK_);
    TcpalSemaphoreUnLock(&Tcc317xInterfaceSema);
    return TCC317X_RETURN_SUCCESS;
}

I32S Tcc317xMemoryWrite(I32S _moduleIndex, I32S _diversityIndex, I32U _address, I08U *_data, I32U _size)
{
    I08U inputValue[4];
    Tcc317xHandle_t *h;

    if(!Tcc317xHandle[_moduleIndex][_diversityIndex].opened)
        return TCC317X_RETURN_FAIL_NULL_ACCESS;

    h = &Tcc317xHandle[_moduleIndex][_diversityIndex];
	if (!(_size > 0))
		return TCC317X_RETURN_FAIL;

    _address |= PHY_BASE_ADDR;
    TcpalSemaphoreLock(&Tcc317xInterfaceSema);
	Tcc317xSetRegDmaControl (h, TC3XREG_CMDDMA_DMAEN | TC3XREG_CMDDMA_WRITEMODE , _UNLOCK_);
    inputValue[0] = (I08U)(((_address>>24)&0xFF));
    inputValue[1] = (I08U)(((_address>>16)&0xFF));
    inputValue[2] = (I08U)(((_address>>8)&0xFF));
    inputValue[3] = (I08U)((_address&0xFF));
	Tcc317xSetRegDmaSourceAddress (h, &inputValue[0], _UNLOCK_);
    inputValue[0] = (I08U)((((_size >> 2)>>8)&0xFF));
    inputValue[1] = (I08U)(((_size >> 2)&0xFF));
	Tcc317xSetRegDmaSize (h, &inputValue[0], _UNLOCK_);
	Tcc317xSetRegDmaStartControl (h, TC3XREG_CMDDMA_START_AUTOCLR | TC3XREG_CMDDMA_INIT_AUTOCLR | TC3XREG_CMDDMA_CRC32INIT_AUTOCLR, _UNLOCK_);
    Tcc317xSetRegDataWindow (h, _data, _size, _UNLOCK_);
    TcpalSemaphoreUnLock(&Tcc317xInterfaceSema);
    return TCC317X_RETURN_SUCCESS;
}

I32S Tcc317xStreamRead(I32S _moduleIndex, I08U *_data, I32S _size)
{
    Tcc317xHandle_t *h;

    if(!Tcc317xHandle[_moduleIndex][0].opened)
        return TCC317X_RETURN_FAIL_NULL_ACCESS;

    h = &Tcc317xHandle[_moduleIndex][0];

	if (_size <= 0)
		return TCC317X_RETURN_FAIL;

    return (Tcc317xGetRegStreamData (h, _data, _size));
}

I32S Tcc317xDspTableWrite (Tcc317xHandle_t * _handle, I32U _addr, I08U *_data, I32S _size)
{
    I08U inputValue[4];
	if (_size <= 0)
		return TCC317X_RETURN_FAIL;
    TcpalSemaphoreLock(&Tcc317xInterfaceSema);
	Tcc317xSetRegDmaControl (_handle, TC3XREG_CMDDMA_DMAEN | TC3XREG_CMDDMA_WRITEMODE, _UNLOCK_);
    inputValue[0] = (I08U)(((_addr>>24)&0xFF));
    inputValue[1] = (I08U)(((_addr>>16)&0xFF));
    inputValue[2] = (I08U)(((_addr>>8)&0xFF));
    inputValue[3] = (I08U)((_addr&0xFF));
	Tcc317xSetRegDmaSourceAddress (_handle, &inputValue[0], _UNLOCK_);
    inputValue[0] = (I08U)((((_size >> 2)>>8)&0xFF));
    inputValue[1] = (I08U)(((_size >> 2)&0xFF));
	Tcc317xSetRegDmaSize (_handle, &inputValue[0], _UNLOCK_);
	Tcc317xSetRegDmaStartControl (_handle, TC3XREG_CMDDMA_START_AUTOCLR | TC3XREG_CMDDMA_INIT_AUTOCLR | TC3XREG_CMDDMA_CRC32INIT_AUTOCLR, _UNLOCK_);
    Tcc317xSetRegDataWindow (_handle, _data, _size, _UNLOCK_);
    TcpalSemaphoreUnLock(&Tcc317xInterfaceSema);
    return TCC317X_RETURN_SUCCESS;
}

I32S Tcc317xDspAsmWrite (Tcc317xHandle_t * _handle, I08U *_data, I32S _size)
{
    I08U inputValue[4];

	if (_size <= 0)
		return TCC317X_RETURN_FAIL;

    TcpalSemaphoreLock(&Tcc317xInterfaceSema);
	Tcc317xSetRegDmaControl (_handle, TC3XREG_CMDDMA_DMAEN | TC3XREG_CMDDMA_WRITEMODE | TC3XREG_CMDDMA_CRC32EN, _UNLOCK_);
    inputValue[0] = ((TC317X_CODEMEMBASE_8000>>24)&0xFF);
    inputValue[1] = ((TC317X_CODEMEMBASE_8000>>16)&0xFF);
    inputValue[2] = ((TC317X_CODEMEMBASE_8000>>8)&0xFF);
    inputValue[3] = (TC317X_CODEMEMBASE_8000&0xFF);
	Tcc317xSetRegDmaSourceAddress (_handle, &inputValue[0], _UNLOCK_);
	inputValue[0] = (I08U)((((_size >> 2)>>8)&0xFF));
    inputValue[1] = (I08U)(((_size >> 2)&0xFF));
	Tcc317xSetRegDmaSize (_handle, &inputValue[0], _UNLOCK_);
	Tcc317xSetRegDmaStartControl (_handle, TC3XREG_CMDDMA_START_AUTOCLR | TC3XREG_CMDDMA_INIT_AUTOCLR | TC3XREG_CMDDMA_CRC32INIT_AUTOCLR, _UNLOCK_);
    Tcc317xSetRegDataWindow (_handle, _data, _size, _UNLOCK_);
    TcpalSemaphoreUnLock(&Tcc317xInterfaceSema);
    return TCC317X_RETURN_SUCCESS;
}

I32S ReadProcess(Tcc317xHandle_t *_handle, I08U _registerAddr, I32S _size, I08U *_outData, I08U _unlock)
{
    I32S ret;
    if(_unlock == _LOCK_)
        TcpalSemaphoreLock(&Tcc317xInterfaceSema);

    ret = _handle->Read(_handle->moduleIndex, _handle->currentAddress, _registerAddr, _outData, _size);

    if(_unlock == _LOCK_)
        TcpalSemaphoreUnLock(&Tcc317xInterfaceSema);

    if(ret != TCC317X_RETURN_SUCCESS)
        _handle->EventCallBack(_handle->moduleIndex, _handle->diversityIndex, 
                               TCC317X_EVENT_COMMAND_READ_FAIL);

    return ret;
}

I32S WriteProcess(Tcc317xHandle_t *_handle, I08U _registerAddr, I08U *_inputData, I32S _size, I08U _unlock)
{
    I32S ret;
    if(_unlock == _LOCK_)
        TcpalSemaphoreLock(&Tcc317xInterfaceSema);

    ret = _handle->Write(_handle->moduleIndex, _handle->currentAddress, _registerAddr, _inputData, _size);

    if(_unlock == _LOCK_)
        TcpalSemaphoreUnLock(&Tcc317xInterfaceSema);

    if(ret != TCC317X_RETURN_SUCCESS)
        _handle->EventCallBack(_handle->moduleIndex, _handle->diversityIndex, 
                               TCC317X_EVENT_COMMAND_WRITE_FAIL);

    return ret;
}

/* communication with API Layer !!!! */

/* baseband register*/
I32S Tcc317xRead(I32S _moduleIndex, I32S _diversityIndex, I08U _address, I08U *_data, I32U _size)
{
    Tcc317xHandle_t *h;

    if(!Tcc317xHandle[_moduleIndex][_diversityIndex].opened)
        return TCC317X_RETURN_FAIL_NULL_ACCESS;

    h = &Tcc317xHandle[_moduleIndex][_diversityIndex];
    return (Tcc317xGetRegManual (h, _address, _size, &_data[0]));
}

I32S Tcc317xWrite(I32S _moduleIndex, I32S _diversityIndex, I08U _address, I08U *_data, I32U _size)
{
    Tcc317xHandle_t *h;

    if(!Tcc317xHandle[_moduleIndex][_diversityIndex].opened)
        return TCC317X_RETURN_FAIL_NULL_ACCESS;

    h = &Tcc317xHandle[_moduleIndex][_diversityIndex];
    return (Tcc317xSetRegManual (h, _address, &_data[0], _size));
}

I32S Tcc317xReadIrqError(I32S _moduleIndex, I08U *_data)
{
    Tcc317xHandle_t *h;

    if(!Tcc317xHandle[_moduleIndex][0].opened)
        return TCC317X_RETURN_FAIL_NULL_ACCESS;

    h = &Tcc317xHandle[_moduleIndex][0];
    return (Tcc317xGetRegIrqError (h, _data));
}

I32S Tcc317xIrqClear(I32S _moduleIndex)
{
    Tcc317xHandle_t *h;

    if(!Tcc317xHandle[_moduleIndex][0].opened)
        return TCC317X_RETURN_FAIL_NULL_ACCESS;

    h = &Tcc317xHandle[_moduleIndex][0];
    return (Tcc317xSetRegIrqClear (h, TC3XREG_IRQ_STAT_DATAINT));
}

#define SWAP16(x) \
    ((I16U)( \
    (((I16U)(x) & (I16U)0x00ffU) << 8) | \
    (((I16U)(x) & (I16U)0xff00U) >> 8) ))

#define SWAP32(x) \
    ((I32U)( \
    (((I32U)(x) & (I32U)0x000000ffUL) << 24) | \
    (((I32U)(x) & (I32U)0x0000ff00UL) <<  8) | \
    (((I32U)(x) & (I32U)0x00ff0000UL) >>  8) | \
    (((I32U)(x) & (I32U)0xff000000UL) >> 24) ))

/* rf register */
I32S Tcc317xRfRegisterRead(I32S _moduleIndex, I32S _diversityIndex, I08U _address, I32U *_data)
{
    Tcc317xHandle_t *h;

    if(!Tcc317xHandle[_moduleIndex][_diversityIndex].opened)
        return TCC317X_RETURN_FAIL_NULL_ACCESS;

    h = &Tcc317xHandle[_moduleIndex][_diversityIndex];

    TcpalSemaphoreLock(&Tcc317xInterfaceSema);

    Tcc317xSetRegRfConfig(h, TC3XREG_RF_MANAGE_ENABLE|TC3XREG_RF_READ, _UNLOCK_);
    Tcc317xSetRegRfAddress(h, _address, _UNLOCK_);
    Tcc317xSetRegRfAction(h, TC3XREG_RF_ACTION, _UNLOCK_);
    Tcc317xGetRegRfData(h, (I08U*)_data, _UNLOCK_);
    _data[0] = SWAP32(_data[0]);
    Tcc317xSetRegRfConfig(h, TC3XREG_RF_MANAGE_DISABLE, _UNLOCK_);
    
    TcpalSemaphoreUnLock(&Tcc317xInterfaceSema);
    return TCC317X_RETURN_SUCCESS;
}

I32S Tcc317xRfRegisterWrite(I32S _moduleIndex, I32S _diversityIndex, I08U _address, I32U _data)
{
    Tcc317xHandle_t *h;

    if(!Tcc317xHandle[_moduleIndex][_diversityIndex].opened)
        return TCC317X_RETURN_FAIL_NULL_ACCESS;

    h = &Tcc317xHandle[_moduleIndex][_diversityIndex];

    TcpalSemaphoreLock(&Tcc317xInterfaceSema);

    Tcc317xSetRegRfConfig(h, TC3XREG_RF_MANAGE_ENABLE|TC3XREG_RF_WRITE, _UNLOCK_);
    Tcc317xSetRegRfAddress(h, _address, _UNLOCK_);
    _data = SWAP32(_data);
    Tcc317xSetRegRfData(h, (I08U*)&_data, _UNLOCK_);
    Tcc317xSetRegRfAction(h, TC3XREG_RF_ACTION, _UNLOCK_);
    Tcc317xSetRegRfConfig(h, TC3XREG_RF_MANAGE_DISABLE, _UNLOCK_);

    TcpalSemaphoreUnLock(&Tcc317xInterfaceSema);
    return TCC317X_RETURN_SUCCESS;
}
