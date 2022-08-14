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
#include "sensor_if.h"

#ifndef MT9M113_H
#define MT9M113_H

#define MT9M113_REG_TERM 				0x0000	/* terminating list entry for reg */
#define MT9M113_VAL_TERM 				0x0000	/* terminating list entry for val */

#if defined(CONFIG_VIDEO_DUAL_CAMERA_SUPPORT)
extern void sensor_info_init_mt9m113(TCC_SENSOR_INFO_TYPE *sensor_info);
extern void sensor_init_fnc_mt9m113(SENSOR_FUNC_TYPE *sensor_func);
#else // CONFIG_VIDEO_DUAL_CAMERA_SUPPORT
/* The MT9M113 I2C sensor chip has a fixed slave address of 0x7A. */
#define SENSOR_I2C_ADDR		0x78
#define SENSOR_FRAMERATE	15

//CLOCK
#if defined(CONFIG_ARCH_TCC88XX) || defined(CONFIG_ARCH_TCC892X) || defined(CONFIG_ARCH_TCC893X)
#if defined(CONFIG_USE_ISP)
#define CAM_POLARITY_VSYNC	0   // 1: low active,	0: high active	
#define CAM_POLARITY_HSYNC	0   // 1: low active,	0: high active	
#define CAM_POLARITY_PCLK	1   // 1: positive edge	0: negative edge
#endif //CONFIG_USE_ISP
#define CKC_CAMERA_MCLK				240000
#define CKC_CAMERA_SCLK				720000			//max 20 fps : 36 Mhz(pclk)/72 Mhz(PLL2), max 27 fps : 60 Mhz(pclk)/125 Mhz(PLL1)
#elif defined(CONFIG_ARCH_TCC92XX)
#define CKC_CAMERA_MCLK				240000
#define CKC_CAMERA_SCLK				720000
#elif defined(CONFIG_ARCH_TCC93XX)
#if defined(CONFIG_USE_ISP)
#define CAM_POLARITY_VSYNC	0   // 1: low active,	0: high active	
#define CAM_POLARITY_HSYNC	0   // 1: low active,	0: high active	
#define CAM_POLARITY_PCLK	1   // 1: positive edge	0: negative edge
#endif //CONFIG_USE_ISP
#define CKC_CAMERA_MCLK				240000
#define CKC_CAMERA_SCLK				660000			// max 20 fps : 36 Mhz(pclk)/66 Mhz(/PLL2), 27 fps : 60 Mhz(pclk)/112 Mhz(PLL1)
#else
// todo : 
#endif

#define FRAMESKIP_COUNT_FOR_CAPTURE 1

// ZOOM Setting!!
#define PRV_W			640
#define PRV_H			512
#define PRV_ZOFFX		10
#define PRV_ZOFFY		8

#define CAP_W			1280
#define CAP_H			1024
#define CAP_ZOFFX		20
#define CAP_ZOFFY		16

#define CAM_2XMAX_ZOOM_STEP 	15
#define CAM_CAPCHG_WIDTH  		640

#ifdef CONFIG_USE_ISP
#define CAM_MAX_ZOOM_STEP    CAM_2XMAX_ZOOM_STEP+1
#endif

struct sensor_reg {
	unsigned short reg;
	unsigned short val;
};

struct capture_size {
	unsigned long width;
	unsigned long height;
};

extern struct capture_size sensor_sizes[];

extern void sensor_init_fnc(SENSOR_FUNC_TYPE *sensor_func);
#endif // CONFIG_VIDEO_DUAL_CAMERA_SUPPORT
#endif /* MT9M113_H */

