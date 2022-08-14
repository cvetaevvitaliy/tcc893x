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


 #include <mach/globals.h>
#include "../sensor_if.h"

#ifndef __TVP5150_H__
#define __TVP5150_H__

//#define	CCR_VIDEODECODER_AK8856	1

#if	CCR_VIDEODECODER_AK8856
#define SENSOR_I2C_ADDR 0x88
#else
#define SENSOR_I2C_ADDR 0xB8
//#define SENSOR_I2C_ADDR 0xBA
#endif

#define REG_TERM 0xFF
#define VAL_TERM 0xFF

//CLOCK
#define CKC_CAMERA_MCLK				245000
#define CKC_CAMERA_SCLK				2000000

#define FRAMESKIP_COUNT_FOR_CAPTURE 1
#define SENSOR_FRAMERATE	15

//#define FEATURE_TV_STANDARD_NTSC
#if defined(FEATURE_TV_STANDARD_NTSC) 	// support to NTSC
#define PRV_W 720
#define PRV_H 240 // 480
#define CAP_W 720
#define CAP_H 240 // 480
#else 									// support to PAL
#define PRV_W 720
#define PRV_H 288 // 576
#define CAP_W 720
#define CAP_H 288 // 576
#endif
#define PRV_ZOFFX 0
#define PRV_ZOFFY 0
#define CAP_ZOFFX 0
#define CAP_ZOFFY 0

#define CAM_2XMAX_ZOOM_STEP 25
#define CAM_CAPCHG_WIDTH 720

struct sensor_reg
{
	unsigned char reg;
	unsigned char val[1];
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

#endif /*__TVP5150_H__*/

