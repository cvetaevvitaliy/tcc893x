/****************************************************************************
One line to give the program's name and a brief idea of what it does.
Copyright (C) 2013 Telechips Inc.

This program is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA 02111-1307 USA
****************************************************************************/

/****************************************************************************

drivers/media/video/tcccam/hdmi_in/ep9553e.h

Register definitions for the ep9553E HDMI Receiver Chip.

Author: swhwang (swhwang@telechips.com)

Copyright (C) 2013 Telechips, Inc.

****************************************************************************/



 #include <mach/globals.h>
#include "../sensor_if.h"

#ifndef __EP9553T_H__
#define __EP9553T_H__


#define SENSOR_I2C_ADDR 0x78

#define REG_TERM 0xFF
#define VAL_TERM 0xFF

//CLOCK
#define CKC_CAMERA_MCLK				245000
#define CKC_CAMERA_MCLK_SRC			DIRECTPLL1
#define CKC_CAMERA_SCLK				2000000
#define CKC_CAMERA_SCLK_SRC			DIRECTPLL2

#define FRAMESKIP_COUNT_FOR_CAPTURE 1
#define SENSOR_FRAMERATE	30

#define PRV_ZOFFX 0
#define PRV_ZOFFY 0
#define CAP_ZOFFX 0
#define CAP_ZOFFY 0

#define CAM_2XMAX_ZOOM_STEP 25
#define CAM_CAPCHG_WIDTH 720

#define true 1
#define false 0

extern  unsigned short PRV_W;
extern  unsigned short PRV_H; 
extern  unsigned short CAP_W;
extern  unsigned short CAP_H;

extern unsigned char hdmi_in_Interlaced;
//extern unsigned char hdmi_in_vin_fmt;
//extern unsigned char hdmi_in_vin_yuvmode;

struct sensor_reg
{
	unsigned char reg;
//	unsigned char val[1];
	unsigned char val[20];
};

struct capture_size
{
	unsigned long width;
	unsigned long height;
};


extern struct capture_size sensor_sizes[];
extern struct sensor_reg *sensor_reg_common[];
extern struct sensor_reg *sensor_reg_brightness[];
extern struct sensor_reg *sensor_reg_awb[];
extern struct sensor_reg *sensor_reg_iso[];
extern struct sensor_reg *sensor_reg_effect[];
extern struct sensor_reg *sensor_reg_flip[];
extern struct sensor_reg *sensor_reg_scene[];
extern struct sensor_reg *sensor_reg_metering_exposure[];
extern struct sensor_reg *sensor_reg_af[];

extern void sensor_init_fnc(SENSOR_FUNC_TYPE *sensor_func);

#endif /*__EP9553T_H__*/

