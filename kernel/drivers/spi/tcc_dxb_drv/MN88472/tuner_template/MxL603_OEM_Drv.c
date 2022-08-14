/*****************************************************************************************
 *
 * FILE NAME          : MxL603_OEM_Drv.c
 * 
 * AUTHOR             : Mahendra Kondur
 *
 * DATE CREATED       : 12/23/2011  
 *
 * DESCRIPTION        : This file contains I2C driver and Sleep functins that 
 *                      OEM should implement for MxL603 APIs
 *                             
 *****************************************************************************************
 *                Copyright (c) 2011, MaxLinear, Inc.
 ****************************************************************************************/
#include <linux/delay.h>
#include <linux/i2c.h>

#include "MxL603_OEM_Drv.h"

#define     DEBUG_PRITF     printf

extern int Ctrl_SetI2CAddr(char I2CSlaveAddr);
extern int mn88472_get_fe_config(struct i2c_adapter **i2c_handle);

MXL_STATUS Ctrl_I2cConnect(UINT8 devId)
{
	return MXL_TRUE;
}

MXL_STATUS Ctrl_I2cDisConnect()
{
	return MXL_TRUE;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_OEM_ReadRegister
--| 
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 7/30/2009
--|
--| DESCRIPTION   : This function does I2C read operation.
--|
--| RETURN VALUE  : True or False
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_OEM_ReadRegister(UINT8 devId, UINT8 RegAddr, UINT8 *DataPtr)
{
	MXL_STATUS status = MXL_TRUE;
	UINT8 i2cSlaveAddr = 0x60;

	int ret;
	struct i2c_msg msg;
	struct i2c_adapter *i2c_handle;
	char buf[2];

	buf[0] = 0xFB;
    buf[1] = RegAddr;

	// write value
	memset(&msg, 0, sizeof(msg));
	msg.addr = i2cSlaveAddr;
	msg.flags = 0;
	msg.buf = buf;
	msg.len = 2;

	mn88472_get_fe_config(&i2c_handle);
	ret = i2c_transfer(i2c_handle, &msg, 1);
	if(ret<0){
		printk(" %s: writereg error, errno is %d \n", __FUNCTION__, ret);
		return MXL_FALSE;
	}

	// read value
	memset(&msg, 0, sizeof(msg));
	msg.addr = i2cSlaveAddr;
	msg.flags |=  I2C_M_RD;  //write  I2C_M_RD=0x01
	msg.buf = DataPtr;
	msg.len = 1;

	mn88472_get_fe_config(&i2c_handle);
	ret = i2c_transfer(i2c_handle, &msg, 1);
	if(ret != 1)
	{
		printk("mxl101_readregister reg failure!\n");
		return MXL_FALSE;
	}

	return status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_OEM_WriteRegister
--| 
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 7/30/2009
--|
--| DESCRIPTION   : This function does I2C write operation.
--|
--| RETURN VALUE  : True or False
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_OEM_WriteRegister(UINT8 devId, UINT8 RegAddr, UINT8 RegData)
{
	MXL_STATUS status = MXL_TRUE;
	UINT8 i2cSlaveAddr = 0x60;

	int ret;
	struct i2c_msg msg;
	struct i2c_adapter *i2c_handle;
	char buf[2];

	buf[0] = RegAddr;
	buf[1] = RegData;

	// write value
	memset(&msg, 0, sizeof(msg));
	msg.addr = i2cSlaveAddr;
	msg.flags = 0;
	msg.buf = buf;
	msg.len = 2;

	mn88472_get_fe_config(&i2c_handle);
	ret = i2c_transfer(i2c_handle, &msg, 1);
	if(ret<0){
		printk(" %s: writereg error, errno is %d \n", __FUNCTION__, ret);
		status = MXL_FALSE;
	}

    return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_OEM_Sleep
--| 
--| AUTHOR        : Dong Liu
--|
--| DATE CREATED  : 01/10/2010
--|
--| DESCRIPTION   : This function complete sleep operation. WaitTime is in ms unit
--|
--| RETURN VALUE  : None
--|
--|-------------------------------------------------------------------------------------*/

void MxLWare603_OEM_Sleep(UINT16 DelayTimeInMs)
{
  // OEM should implement sleep operation 
  udelay(DelayTimeInMs*1000);
}
