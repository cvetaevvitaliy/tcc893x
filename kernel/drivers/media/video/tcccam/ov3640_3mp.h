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

#ifndef OV3640_H
#define OV3640_H

/* The OV3640 I2C sensor chip has a fixed slave address of 0x78. */
#define SENSOR_I2C_ADDR		0x78

#define REG_TERM 0xffff	/* terminating list entry for reg */
#define VAL_TERM 0xff	/* terminating list entry for val */

//CLOCK
#define CKC_CAMERA_MCLK				240000
#define CKC_CAMERA_SCLK				800000

#define FRAMESKIP_COUNT_FOR_CAPTURE 1

// ZOOM Setting!!
#define PRV_W			1024
#define PRV_H			768
#define PRV_ZOFFX		8
#define PRV_ZOFFY		6

#define CAP_W			2048
#define CAP_H			1536
#define CAP_ZOFFX		16
#define CAP_ZOFFY		12

#define CAM_2XMAX_ZOOM_STEP 	31
#define CAM_CAPCHG_WIDTH  		1024


struct sensor_reg {
	unsigned short reg;
	unsigned char val;
};

struct capture_size {
	unsigned long width;
	unsigned long height;
};

extern struct capture_size sensor_sizes[];
extern void sensor_init_fnc(SENSOR_FUNC_TYPE *sensor_func);

#endif /* MT9D112_H */

