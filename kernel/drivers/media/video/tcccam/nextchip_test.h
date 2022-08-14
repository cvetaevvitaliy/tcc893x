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

#ifndef NEXTCHIP_TEST_H
#define NEXTCHIP_TEST_H

#define _NEXTCHIP_

/* The NEXTCHIP I2C sensor chip has a fixed slave address of 0xXX. */
#define SENSOR_I2C_ADDR		0x7A	// todo!!

#define REG_TERM 0x0000	/* terminating list entry for reg */
#define VAL_TERM 0x0000	/* terminating list entry for val */

//original v-sync:0, h-sync:0, pclk:1
#define CAM_POLARITY_VSYNC	0   // 1: low active,	0: high active	
#define CAM_POLARITY_HSYNC	0   // 1: low active,	0: high active	
#define CAM_POLARITY_PCLK	1   // 1: positive edge	0: negative edge

#define CKC_CAMERA_MCLK				960000		//(1-e)96M-25.8fps,  (2-o)86.4M - 23.26fps,  (3-e)82.5M - 22.2fps, (1-o)76.8M-20.6fps, 64Mhz-17fps
#define CKC_CAMERA_SCLK				960000

#define CAMDLY 0xFFFF

// ZOOM Setting!!
#define PRV_W			1920
#define PRV_H			1080
#define PRV_ZOFFX		8
#define PRV_ZOFFY		6

#define CAP_W			1600
#define CAP_H			1200
#define CAP_ZOFFX		16
#define CAP_ZOFFY		12

#define CAM_2XMAX_ZOOM_STEP 	25
#define CAM_CAPCHG_WIDTH  		800

#ifdef CONFIG_USE_ISP
#define CAM_MAX_ZOOM_STEP    25+1
#endif


struct sensor_reg {
	unsigned short reg;
	unsigned short val[1];
};

struct capture_size {
	unsigned long width;
	unsigned long height;
};
#if 0
extern struct capture_size sensor_sizes[];

extern struct sensor_reg* sensor_reg_common[];
extern struct sensor_reg* sensor_reg_brightness[];
extern struct sensor_reg* sensor_reg_awb[];
extern struct sensor_reg* sensor_reg_iso[];
extern struct sensor_reg* sensor_reg_effect[];
extern struct sensor_reg* sensor_reg_flip[];
extern struct sensor_reg* sensor_reg_scene[];
extern struct sensor_reg* sensor_reg_metering_exposure[];
extern struct sensor_reg* sensor_reg_af[];

struct sensor_reg {
	unsigned char reg;
	unsigned char val;
};

struct capture_size {
	unsigned long width;
	unsigned long height;
};
#endif
extern struct capture_size sensor_sizes[];
extern void sensor_init_fnc(SENSOR_FUNC_TYPE *sensor_func);


#endif /* NEXTCHIP_TEST_H */

