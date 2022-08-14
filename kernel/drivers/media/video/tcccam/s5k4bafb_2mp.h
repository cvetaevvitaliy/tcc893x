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

#ifndef S5K4BAFB_H
#define S5K4BAFB_H

/* The S5K4BAFB I2C sensor chip has a fixed slave address of 0x5D. */
#define SENSOR_I2C_ADDR		0x52

#define REG_TERM 0xFF	/* terminating list entry for reg */
#define VAL_TERM 0xFF	/* terminating list entry for val */

//CLOCK
#define CKC_CAMERA_MCLK				250000
#define CKC_CAMERA_SCLK				640000

#define FRAMESKIP_COUNT_FOR_CAPTURE 3

// ZOOM Setting!!
#define PRV_W			800
#define PRV_H			600
#define PRV_ZOFFX		8
#define PRV_ZOFFY		6

#define CAP_W			1600
#define CAP_H			1200
#define CAP_ZOFFX		16
#define CAP_ZOFFY		12

#define CAM_2XMAX_ZOOM_STEP 	25
#define CAM_CAPCHG_WIDTH  		800


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

#endif /* S5K4BAFB_H */

