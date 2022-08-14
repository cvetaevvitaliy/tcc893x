/*
 *           Copyright 2012 Availink, Inc.
 *
 *  This software contains Availink proprietary information and
 *  its use and disclosure are restricted solely to the terms in
 *  the corresponding written license agreement. It shall not be 
 *  disclosed to anyone other than valid licensees without
 *  written permission of Availink, Inc.
 *
 */


///$Date: 2012-3-8 21:47 $
///
///
/// @file
/// @brief Implements the functions declared in IBSP.h. 
/// 
#include "../include/IBSP.h"

#ifdef  TELECHIPS_CODE
#include <linux/delay.h>
#include <linux/i2c.h>
#endif//TELECHIPS_CODE


/// The following table illustrates a set of PLL configuration values to operate AVL6211 in two modes:
// Standard performance mode.
// High performance mode

/// Please refer to the AVL6211 channel receiver datasheet for detailed information on highest symbol rate 
/// supported by the demod in both these modes.

///For more information on other supported clock frequencies and PLL settings for higher symbol rates, please 
///contact Availink.

/// Users can remove unused elements from the following array to reduce the SDK footprint size.
const struct AVL_DVBSx_PllConf pll_conf[] =
{
	// The following set of PLL configuration at different reference clock frequencies refer to demod operation
	// in standard performance mode.
	 {503,  1, 7, 4, 2,  4000, 11200, 16800, 25200} ///< Reference clock 4 MHz,   Demod clock 112 MHz, FEC clock 168 MHz, MPEG clock 252 MHz
	,{447,  1, 7, 4, 2,  4500, 11200, 16800, 25200} ///< Reference clock 4.5 MHz, Demod clock 112 MHz, FEC clock 168 MHz, MPEG clock 252 MHz
	,{503,  4, 7, 4, 2, 10000, 11200, 16800, 25200} ///< Reference clock 10 MHz,  Demod clock 112 MHz, FEC clock 168 MHz, MPEG clock 252 MHz
	,{503,  7, 7, 4, 2, 16000, 11200, 16800, 25200} ///< Reference clock 16 MHz,  Demod clock 112 MHz, FEC clock 168 MHz, MPEG clock 252 MHz
	,{111,  2, 7, 4, 2, 27000, 11200, 16800, 25200} ///< Reference clock 27 MHz,  Demod clock 112 MHz, FEC clock 168 MHz, MPEG clock 252 MHz
	
	// The following set of PLL configuration at different reference clock frequencies refer to demod operation
	// in high performance mode. 
	,{566,  1, 7, 4, 2,  4000, 12600, 18900, 28350} /// < Reference clock 4 MHz,   Demod clock 126 MHz, FEC clock 189 MHz, MPEG clock 283.5 MHz
	,{503,  1, 7, 4, 2,  4500, 12600, 18900, 28350} ///< Reference clock 4.5 MHz, Demod clock 126 MHz, FEC clock 189 MHz, MPEG clock 283.5 MHz
	,{566,  4, 7, 4, 2, 10000, 12600, 18900, 28350} ///< Reference clock 10 MHz,  Demod clock 126 MHz, FEC clock 189 MHz, MPEG clock 283.5 MHz
	,{566,  7, 7, 4, 2, 16000, 12600, 18900, 28350} ///< Reference clock 16 MHz,  Demod clock 126 MHz, FEC clock 189 MHz, MPEG clock 283.5 MHz
	,{377,  8, 7, 4, 2, 27000, 12600, 18900, 28350} ///< Reference clock 27 MHz,  Demod clock 126 MHz, FEC clock 189 MHz, MPEG clock 283.5 MHz

};

const AVL_uint16 pll_array_size = sizeof(pll_conf)/sizeof(struct AVL_DVBSx_PllConf);

AVL_DVBSx_ErrorCode AVL_DVBSx_IBSP_Initialize(void)
{
	return(AVL_DVBSx_EC_OK);
}

AVL_DVBSx_ErrorCode AVL_DVBSx_IBSP_Dispose(void)
{
	return(AVL_DVBSx_EC_OK);
}

AVL_DVBSx_ErrorCode AVL_DVBSx_IBSP_Delay( AVL_uint32 uiMS )
{
#ifdef  TELECHIPS_CODE
	msleep(uiMS);
#endif//TELECHIPS_CODE
	return(AVL_DVBSx_EC_OK);
}

AVL_DVBSx_ErrorCode AVL_DVBSx_IBSP_I2CRead( const struct AVL_DVBSx_Chip * pAVLChip, AVL_puchar pucBuff, AVL_puint16 puiSize )
{
#ifdef  TELECHIPS_CODE
	AVL_uint32 nRetCode = 0;
	struct i2c_msg msg[1];
	struct i2c_adapter *i2c_handle;

	if(pucBuff == 0 || *puiSize == 0)
	{
		printk("avl6211 read register parameter error !!\n");
		return(AVL_DVBSx_EC_I2CFail);
	}

	//read real data 
	memset(msg, 0, sizeof(msg));
	msg[0].addr = pAVLChip->m_SlaveAddr;
	msg[0].flags |= I2C_M_RD;  //write  I2C_M_RD=0x01
	msg[0].len = *puiSize;
	msg[0].buf = pucBuff;

	i2c_handle = i2c_get_adapter(1);
	if (!i2c_handle) {
		printk("cannot get i2c adaptor\n");
		return(AVL_DVBSx_EC_I2CFail);
	}

	nRetCode = i2c_transfer(i2c_handle, msg, 1);
	if(nRetCode != 1)
	{
		printk("avl6211_readregister reg failure!\n");
		return(AVL_DVBSx_EC_I2CFail);
	}
#endif//TELECHIPS_CODE
	return(AVL_DVBSx_EC_OK);
}
AVL_DVBSx_ErrorCode AVL_DVBSx_IBSP_I2CWrite( const struct AVL_DVBSx_Chip * pAVLChip, AVL_puchar pucBuff, AVL_puint16  puiSize )
{
#ifdef  TELECHIPS_CODE
	AVL_int32 nRetCode = 0;
	struct i2c_msg msg; //construct 2 msgs, 1 for reg addr, 1 for reg value, send together
	struct i2c_adapter *i2c_handle;

	if(pucBuff == 0 || *puiSize == 0)
	{
		printk("avl6211 write register parameter error !!\n");
		return(AVL_DVBSx_EC_I2CFail);
	}

	//write value
	memset(&msg, 0, sizeof(msg));
	msg.addr = pAVLChip->m_SlaveAddr;
	msg.flags = 0;	//I2C_M_NOSTART;
	msg.buf = pucBuff;
	msg.len = *puiSize;

	i2c_handle = i2c_get_adapter(1);
	if (!i2c_handle) {
		printk("cannot get i2c adaptor\n");
		return(AVL_DVBSx_EC_I2CFail);
	}

	nRetCode = i2c_transfer(i2c_handle, &msg, 1);
	if(nRetCode < 0)
	{
		printk(" %s: writereg error, errno is %d \n", __FUNCTION__, nRetCode);
		return(AVL_DVBSx_EC_I2CFail);
	}
#endif//TELECHIPS_CODE
	return(AVL_DVBSx_EC_OK);
}

AVL_DVBSx_ErrorCode AVL_DVBSx_IBSP_InitSemaphore( AVL_psemaphore pSemaphore )
{
#ifdef  TELECHIPS_CODE
	sema_init(pSemaphore, 1);
#endif//TELECHIPS_CODE
	return(AVL_DVBSx_EC_OK);
}

AVL_DVBSx_ErrorCode AVL_DVBSx_IBSP_WaitSemaphore( AVL_psemaphore pSemaphore )
{
#ifdef  TELECHIPS_CODE
	if(down_interruptible(pSemaphore))
		return -AVL_DVBSx_EC_GeneralFail;
#endif//TELECHIPS_CODE
	return(AVL_DVBSx_EC_OK);
}

AVL_DVBSx_ErrorCode AVL_DVBSx_IBSP_ReleaseSemaphore( AVL_psemaphore pSemaphore )
{
#ifdef  TELECHIPS_CODE
	up(pSemaphore);
#endif//TELECHIPS_CODE
	return(AVL_DVBSx_EC_OK);
}
