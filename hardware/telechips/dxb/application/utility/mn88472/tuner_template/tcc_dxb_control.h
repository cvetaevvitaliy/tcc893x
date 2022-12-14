/* 
 * linux/drivers/char/tcc_dxb/ctrl/tcc_dxb_control.h
 *
 * Author:  <linux@telechips.com>
 * Created: 10th Jun, 2008 
 * Description: Telechips Linux BACK-LIGHT DRIVER
 *
 * Copyright (c) Telechips, Inc.
 */

#ifndef     _TCC_DXB_CONTROL_H_
#define     _TCC_DXB_CONTROL_H_
#include    <linux/types.h>
typedef enum
{
	BOARD_TDMB_TCC3150, //TCCXXXX_DMB_TCC3150_SV1.2
	BOARD_TDMB_TCC3161, //TCCXXXX_DMB_TCC3161_SV1.0
	BOARD_DVBT_DIB7070,
	BOARD_DVBT_DIB9090, //TCCXXXX_DVBT&H_9080&9090_SV1.0
	BOARD_ISDBT_DIB10096, //tcc89&93XX_ISDBT_DIB1009X_SV0.2
	BOARD_DXB_TCC3510, //TCCXXXX_DXB_TCC3510_SV5.0, OnBoard Module in TCC93XX STB
	BOARD_DVBT_DIB9090M_PA, //TCC89&91&92XX_DVB_DIB90X0_SV0.1 //PARALLEL Interface
	BOARD_ISDBT_MTV818,	//TCCXXXX_ISDB-T_MTV818_SV0.1	//usable only for TCC89xx and TCC92xx	
	BOARD_DXB_NMI32X, //ISDBT_NMI325_SV0.1
	BOARD_ISDBT_TOSHIBA,
	BOARD_ISDBT_TCC353X,
	BOARD_ISDBT_TCC353X_FSMA,
	BOARD_MAX
}DXB_BOARD_TYPE;

typedef struct
{
	unsigned int uiI2C; //control channel of i2c
	unsigned int uiSPI; //control channel of spi 
}ST_CTRLINFO_ARG;

#define DXB_RF_PATH_UHF 1
#define DXB_RF_PATH_VHF 2

#define DXB_CTRL_DEV_FILE		"/dev/tcc_dxb_ctrl"
#define DXB_CTRL_DEV_NAME		"tcc_dxb_ctrl"
#define DXB_CTRL_DEV_MAJOR		251
#define DXB_CTRL_DEV_MINOR		0

#define IOCTL_DXB_CTRL_OFF		    _IO(DXB_CTRL_DEV_MAJOR, 1)
#define IOCTL_DXB_CTRL_ON			_IO(DXB_CTRL_DEV_MAJOR, 2)
#define IOCTL_DXB_CTRL_RESET    	_IO(DXB_CTRL_DEV_MAJOR, 3)
#define IOCTL_DXB_CTRL_SET_BOARD    _IO(DXB_CTRL_DEV_MAJOR, 4)
#define IOCTL_DXB_CTRL_GET_CTLINFO  _IO(DXB_CTRL_DEV_MAJOR, 5)
#define IOCTL_DXB_CTRL_RF_PATH      _IO(DXB_CTRL_DEV_MAJOR, 6)
#define IOCTL_DXB_CTRL_SET_CTRLMODE _IO(DXB_CTRL_DEV_MAJOR, 7)

#endif
