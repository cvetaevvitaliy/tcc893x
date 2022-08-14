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

#ifndef GC0329_H
#define GC0329_H

/* The GC0329 I2C sensor chip has a fixed slave address of 0x78. */
#define SENSOR_I2C_ADDR		0x62

#define REG_TERM 0xff	/* terminating list entry for reg */
#define VAL_TERM 0xff	/* terminating list entry for val */

#define SENSOR_FRAMERATE	15 /* For SDK2308 */

#if defined(CONFIG_USE_ISP)
#define CAM_POLARITY_VSYNC	0   // 1: low active,	0: high active	
#define CAM_POLARITY_HSYNC	0   // 1: low active,	0: high active	
#define CAM_POLARITY_PCLK	1   // 1: positive edge	0: negative edge
#endif //CONFIG_USE_ISP

//CLOCK
#define CKC_CAMERA_MCLK				240000
#define CKC_CAMERA_SCLK				480000

#define FRAMESKIP_COUNT_FOR_CAPTURE 1

// ZOOM Setting!!
#define PRV_W			640
#define PRV_H			480
#define PRV_ZOFFX		8
#define PRV_ZOFFY		6

#define CAP_W			640
#define CAP_H			480
#define CAP_ZOFFX		8
#define CAP_ZOFFY		6

//#define CAM_2XMAX_ZOOM_STEP 	24//31
//#define CAM_CAPCHG_WIDTH  		800
#define CAM_CAPCHG_WIDTH  		640


#define CAM_2XMAX_ZOOM_STEP 	24
#define CAM_CAPCHG_WIDTH  		640

#ifdef CONFIG_USE_ISP
#define CAM_MAX_ZOOM_STEP    CAM_2XMAX_ZOOM_STEP+1
#endif


struct sensor_reg {
	unsigned char reg;
	unsigned char val;
};

struct capture_size {
	unsigned long width;
	unsigned long height;
};

extern struct capture_size sensor_sizes[];
extern void sensor_init_fnc(SENSOR_FUNC_TYPE *sensor_func);

#endif /* GC0329_H */

