/****************************************************************************
 *   FileName    : tcc317x_register_control.c
 *   Description : Register control functions
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

#include "tcc317x_register_control.h"
#include "tcc317x_command_control.h"
#include "tcpal_os.h"

extern TcpalSemaphore_t Tcc317xOpMailboxSema[TCC317X_MAX][TCC317X_MAX];

I32S Tcc317xSetRegManual (Tcc317xHandle_t * _handle, I08U _addr, I08U *_data, I32S _size)
{
	if (_addr == TC3XREG_SYS_EN)
	{
		TcpalPrintErr((I08S*)"# Can't control System Register!!\n");
		return TCC317X_RETURN_FAIL;
	}
    return (WriteProcess (_handle, _addr, _data, _size, _LOCK_));
}

I32S Tcc317xGetRegManual (Tcc317xHandle_t * _handle, I08U _addr, I32S _size, I08U *_data)
{
    if(_addr == TC3XREG_STREAM_CFG2 ||_addr == TC3XREG_STREAM_CFG1 )
    {
        I08U latch = 0x20;
        if(_handle->options.useInterrupt)
            latch = 0x21;
        else
            latch = 0x20;
        WriteProcess (_handle, TC3XREG_STREAM_CFG3, &latch, 1, _LOCK_);
    }
	return (ReadProcess (_handle, _addr, _size, _data, _LOCK_));
}

/* System Control Register */

I32S Tcc317xSetRegSysEnable (Tcc317xHandle_t * _handle, I08U _value)
{
    I32S ret;
    TcpalSemaphoreLock(&Tcc317xOpMailboxSema[_handle->moduleIndex][_handle->diversityIndex]);
    ret = WriteProcess (_handle, TC3XREG_SYS_EN, &_value, 1, _LOCK_);
	_handle->sysEnValue = _value;
    TcpalSemaphoreUnLock(&Tcc317xOpMailboxSema[_handle->moduleIndex][_handle->diversityIndex]);
    return ret;
}

I32S Tcc317xSetRegSysReset (Tcc317xHandle_t * _handle, I08U _value)
{
    I32S ret;
    TcpalSemaphoreLock(&Tcc317xOpMailboxSema[_handle->moduleIndex][_handle->diversityIndex]);
    ret = WriteProcess (_handle, TC3XREG_SYS_RESET, &_value, 1, _LOCK_);
    TcpalSemaphoreUnLock(&Tcc317xOpMailboxSema[_handle->moduleIndex][_handle->diversityIndex]);
    return ret;
}

I32S Tcc317xSetRegIrqMode (Tcc317xHandle_t * _handle, I08U _value)
{
    return (WriteProcess (_handle, TC3XREG_IRQ_MODE, &_value, 1, _LOCK_));
}

I32S Tcc317xSetRegIrqEnable (Tcc317xHandle_t * _handle, I08U _value)
{
    return (WriteProcess (_handle, TC3XREG_IRQ_EN, &_value, 1, _LOCK_));
}

I32S Tcc317xSetRegIrqClear (Tcc317xHandle_t * _handle, I08U _value)
{
    return (WriteProcess (_handle, TC3XREG_IRQ_STAT_CLR, &_value, 1, _LOCK_));
}

I32S Tcc317xGetRegIrqError (Tcc317xHandle_t * _handle, I08U *_data)
{
	return (ReadProcess (_handle, TC3XREG_IRQ_ERROR, 1, _data, _LOCK_));
}

I32S Tcc317xSetRegPll6 (Tcc317xHandle_t * _handle, I08U _value)
{
	return (WriteProcess (_handle, TC3XREG_PLL_6, &_value, 1, _LOCK_));
}

I32S Tcc317xSetRegPll7 (Tcc317xHandle_t * _handle, I08U _value)
{
	return (WriteProcess (_handle, TC3XREG_PLL_7, &_value, 1, _LOCK_));
}

I32S Tcc317xSetRegRemap (Tcc317xHandle_t * _handle, I08U _value)
{
    return (WriteProcess (_handle, TC3XREG_INIT_REMAP, &_value, 1, _LOCK_));
}

I32S Tcc317xSetRegRemapPc (Tcc317xHandle_t * _handle, I08U *_data, I32S _size)
{
    /* TC3XREG_INIT_REMAP/TC3XREG_INIT_PC8/TC3XREG_INIT_PC0 */
    return (WriteProcess (_handle, TC3XREG_INIT_REMAP,  _data, _size, _LOCK_));
}

I32S Tcc317xSetRegChipAddress (Tcc317xHandle_t * _handle, I08U _value)
{
    return (WriteProcess (_handle, TC3XREG_CHIPADDR, &_value, 1, _LOCK_));
}

I32S Tcc317xGetRegProgramId (Tcc317xHandle_t * _handle, I08U *_data)
{
	return (ReadProcess (_handle, TC3XREG_PROGRAMID, 1, _data, _LOCK_));
}

I32S Tcc317xGetRegChipId (Tcc317xHandle_t * _handle, I08U *_data)
{
	return (ReadProcess (_handle, TC3XREG_CHIPID, 1, _data, _LOCK_));
}

I32S Tcc317xSetRegGpioAlt (Tcc317xHandle_t * _handle, I08U *_data)
{
    return (WriteProcess (_handle, TC3XREG_GPIO_ALT8, _data, 2, _LOCK_));
}

I32S Tcc317xSetRegGpioDirection (Tcc317xHandle_t * _handle, I08U *_data)
{
    return (WriteProcess (_handle, TC3XREG_GPIO_DR8, _data, 2, _LOCK_));
}

I32S Tcc317xSetRegGpioOutput (Tcc317xHandle_t * _handle, I08U *_data)
{
    return (WriteProcess (_handle, TC3XREG_GPIO_LR8, _data, 2, _LOCK_));
}

I32S Tcc317xSetRegGpioStrength (Tcc317xHandle_t * _handle, I08U *_data)
{
    return (WriteProcess (_handle, TC3XREG_GPIO_DRV8, _data, 2, _LOCK_));
}

I32S Tcc317xSetRegGpioPull (Tcc317xHandle_t * _handle, I08U *_data)
{
    return (WriteProcess (_handle, TC3XREG_GPIO_PE8, _data, 2, _LOCK_));
}

I32S Tcc317xSetRegDiversityIo (Tcc317xHandle_t * _handle, I08U _value)
{
    return (WriteProcess (_handle, TC3XREG_DIVIO, &_value, 1, _LOCK_));
}

I32S Tcc317xSetRegStreamMix (Tcc317xHandle_t * _handle, I08U _value)
{
    return (WriteProcess (_handle, TC3XREG_STREAMMIX_CFG0, &_value, 1, _LOCK_));
}

I32S Tcc317xSetRegStreamConfig0 (Tcc317xHandle_t * _handle, I08U _value)
{
    return (WriteProcess (_handle, TC3XREG_STREAM_CFG0, &_value, 1, _LOCK_));
}

I32S Tcc317xSetRegStreamConfig1 (Tcc317xHandle_t * _handle, I08U _value)
{
    return (WriteProcess (_handle, TC3XREG_STREAM_CFG1, &_value, 1, _LOCK_));
}

I32S Tcc317xSetRegStreamConfig2 (Tcc317xHandle_t * _handle, I08U _value)
{
    return (WriteProcess (_handle, TC3XREG_STREAM_CFG2, &_value, 1, _LOCK_));
}

I32S Tcc317xSetRegStreamConfig3 (Tcc317xHandle_t * _handle, I08U _value)
{
    return (WriteProcess (_handle, TC3XREG_STREAM_CFG3, &_value, 1, _LOCK_));
}

I32S Tcc317xSetRegStreamConfig (Tcc317xHandle_t * _handle, I08U *_data)
{
    return (WriteProcess (_handle, TC3XREG_STREAM_CFG0, _data, 4, _LOCK_));
}

I32S Tcc317xGetRegStreamData (Tcc317xHandle_t * _handle, I08U *_data, I32S _size)
{
	return (ReadProcess (_handle, TC3XREG_STREAM_CFG4|Bit7, _size, _data, _LOCK_));
}

I32S Tcc317xSetRegDataWindow (Tcc317xHandle_t * _handle, I08U *_data, I32S _size, I08U _unlock)
{
	return (WriteProcess (_handle, TC3XREG_CMDDMA_DATA_WIND|Bit7, _data, _size, _unlock));
}

I32S Tcc317xGetRegDataWindow (Tcc317xHandle_t * _handle, I08U *_data, I32S _size, I08U _unlock)
{
	return (ReadProcess (_handle, TC3XREG_CMDDMA_DATA_WIND|Bit7, _size, _data, _unlock));
}

/* Command DMA Register */

I32S Tcc317xSetRegDmaControl (Tcc317xHandle_t * _handle, I08U _value, I08U _unlock)
{
    return (WriteProcess (_handle, TC3XREG_CMDDMA_CTRL, &_value, 1, _unlock));
}

I32S Tcc317xSetRegDmaSourceAddress (Tcc317xHandle_t * _handle, I08U *_data, I08U _unlock )
{
    return (WriteProcess (_handle, TC3XREG_CMDDMA_SADDR_24, _data, 4, _unlock));
}

I32S Tcc317xSetRegDmaSize (Tcc317xHandle_t * _handle, I08U *_data, I08U _unlock )
{
    return (WriteProcess (_handle, TC3XREG_CMDDMA_SIZE8, _data, 2, _unlock));
}

I32S Tcc317xSetRegDmaStartControl (Tcc317xHandle_t * _handle, I08U _value, I08U _unlock )
{
    return (WriteProcess (_handle, TC3XREG_CMDDMA_STARTCTRL, &_value, 1, _unlock));
}

I32S Tcc317xGetRegDmaCrc32 (Tcc317xHandle_t * _handle, I08U *_data)
{
	return (ReadProcess (_handle, TC3XREG_CMDDMA_CRC24, 4, _data, _LOCK_));
}

/* PERIperal for stream data Register */

I32S Tcc317xSetRegPeripheralConfig0 (Tcc317xHandle_t * _handle, I08U _value)
{
    return (WriteProcess (_handle, TC3XREG_PERI_CTRL, &_value, 1, _LOCK_));
}

I32S Tcc317xSetRegPeripheralConfig (Tcc317xHandle_t * _handle, I08U *_data)
{
    return (WriteProcess (_handle, TC3XREG_PERI_CTRL, _data, 5, _LOCK_));
}

/* MAILBOX Register */

I32S Tcc317xSetRegMailboxControl (Tcc317xHandle_t * _handle, I08U _value)
{
    return (WriteProcess (_handle, TC3XREG_MAIL_CTRL, &_value, 1, _LOCK_));
}

I32S Tcc317xGetRegMailboxFifoReadStatus (Tcc317xHandle_t * _handle, I08U *_data)
{
    I08U latchData = 0x5E;
    WriteProcess (_handle, TC3XREG_MAIL_FIFO_R, &latchData, 1, _LOCK_);
	return (ReadProcess (_handle, TC3XREG_MAIL_FIFO_R, 1, _data, _LOCK_));
}

I32S Tcc317xGetRegMailboxFifoWriteStatus (Tcc317xHandle_t * _handle, I08U *_data)
{
    I08U latchData = 0x5E;

    WriteProcess (_handle, TC3XREG_MAIL_FIFO_W, &latchData, 1, _LOCK_);
	return (ReadProcess (_handle, TC3XREG_MAIL_FIFO_W, 1, _data, _LOCK_));
}

I32S Tcc317xSetRegMailboxFifoWindow (Tcc317xHandle_t * _handle, I08U *_data, I32S _size)
{
    return (WriteProcess (_handle, TC3XREG_MAIL_FIFO_WIND | Bit7, _data, _size, _LOCK_));
}

I32S Tcc317xGetRegMailboxFifoWindow (Tcc317xHandle_t * _handle, I08U *_data, I32S _size)
{
	return (ReadProcess (_handle, TC3XREG_MAIL_FIFO_WIND | Bit7, _size, _data, _LOCK_));
}

/* OUTPUT Buffer Management Register */

I32S Tcc317xSetRegOutBufferConfig (Tcc317xHandle_t * _handle, I08U _value)
{
    return (WriteProcess (_handle, TC3XREG_OBUFF_CONFIG, &_value, 1, _LOCK_));
}

I32S Tcc317xSetRegOutBufferInit (Tcc317xHandle_t * _handle, I08U _value)
{
    return (WriteProcess (_handle, TC3XREG_OBUFF_INIT, &_value, 1, _LOCK_));
}

I32S Tcc317xSetRegOutBufferStartAddressA (Tcc317xHandle_t * _handle, I08U *_data)
{
    return (WriteProcess (_handle, TC3XREG_OBUFF_A_SADDR0, _data, 2, _LOCK_));
}

I32S Tcc317xSetRegOutBufferEndAddressA (Tcc317xHandle_t * _handle, I08U *_data)
{
    return (WriteProcess (_handle, TC3XREG_OBUFF_A_EADDR0, _data, 2, _LOCK_));
}

I32S Tcc317xSetRegOutBufferStartAddressB (Tcc317xHandle_t * _handle, I08U *_data)
{
    return (WriteProcess (_handle, TC3XREG_OBUFF_B_SADDR0, _data, 2, _LOCK_));
}

I32S Tcc317xSetRegOutBufferEndAddressB (Tcc317xHandle_t * _handle, I08U *_data)
{
    return (WriteProcess (_handle, TC3XREG_OBUFF_B_EADDR0, _data, 2, _LOCK_));
}

I32S Tcc317xSetRegOutBufferStartAddressC (Tcc317xHandle_t * _handle, I08U *_data)
{
    return (WriteProcess (_handle, TC3XREG_OBUFF_C_SADDR0, _data, 2, _LOCK_));
}

I32S Tcc317xSetRegOutBufferEndAddressC (Tcc317xHandle_t * _handle, I08U *_data)
{
    return (WriteProcess (_handle, TC3XREG_OBUFF_C_EADDR0, _data, 2, _LOCK_));
}

I32S Tcc317xSetRegOutBufferStartAddressD (Tcc317xHandle_t * _handle, I08U *_data)
{
    return (WriteProcess (_handle, TC3XREG_OBUFF_D_SADDR0, _data, 2, _LOCK_));
}

I32S Tcc317xSetRegOutBufferEndAddressD (Tcc317xHandle_t * _handle, I08U *_data)
{
    return (WriteProcess (_handle, TC3XREG_OBUFF_D_EADDR0, _data, 2, _LOCK_));
}

I32S Tcc317xSetRegOutBufferDFifoThr (Tcc317xHandle_t * _handle, I08U *_data)
{
    return (WriteProcess (_handle, TC3XREG_OBUFF_D_FIFO_THR0, _data, 2, _LOCK_));
}

I32S Tcc317xSetRegXtalBias(Tcc317xHandle_t * _handle, I08U _value)
{
    return (WriteProcess (_handle, TC3XREG_OP_XTAL_BIAS, &_value, 1, _LOCK_));
}

I32S Tcc317xSetRegXtalBiasKey(Tcc317xHandle_t * _handle, I08U _value)
{
    return (WriteProcess (_handle, TC3XREG_OP_XTAL_BIAS_KEY, &_value, 1, _LOCK_));
}

I32S Tcc317xSetRegRfConfig(Tcc317xHandle_t * _handle, I08U _value, I08U _unlock)
{
    return (WriteProcess (_handle, TC3XREG_RF_CFG0, &_value, 1, _unlock ));
}

I32S Tcc317xSetRegRfAction(Tcc317xHandle_t * _handle, I08U _value, I08U _unlock)
{
    return (WriteProcess (_handle, TC3XREG_RF_CFG1, &_value, 1, _unlock ));
}

I32S Tcc317xSetRegRfAddress(Tcc317xHandle_t * _handle, I08U _value, I08U _unlock)
{
    return (WriteProcess (_handle, TC3XREG_RF_CFG2, &_value, 1, _unlock ));
}

I32S Tcc317xSetRegRfData(Tcc317xHandle_t * _handle, I08U *_data, I08U _unlock)
{
    return (WriteProcess (_handle, TC3XREG_RF_CFG3, _data, 4, _unlock ));
}

I32S Tcc317xGetRegRfData(Tcc317xHandle_t * _handle, I08U *_data, I08U _unlock)
{
	return (ReadProcess (_handle, TC3XREG_RF_CFG3, 4, _data, _unlock ));
}

I32S Tcc317xGetRegOpDebug (Tcc317xHandle_t * _handle, I08U *_data, I32S _size)
{
	I08U temp = 0x01;
	WriteProcess (_handle, TC3XREG_OP_DEBUG0, &temp, 1, _LOCK_);
	return (ReadProcess (_handle, TC3XREG_OP_DEBUG0, _size, _data, _LOCK_));
}

