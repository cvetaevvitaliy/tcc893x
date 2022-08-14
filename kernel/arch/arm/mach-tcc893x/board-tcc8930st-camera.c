/****************************************************************************
FileName    : kernel/arch/arm//mach-tcc893x/board-tcc8930st-camera.c
Description : 

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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <asm/mach-types.h>
#include "devices.h"
#include "board-tcc8930st.h"
#include <asm/system.h>
#include <linux/gpio.h>



#if defined(CONFIG_VIDEO_TCCXX_CAMERA) //20100720 ysseung   add to sensor slave id.
#include <media/cam_i2c.h>
#if defined(CONFIG_VIDEO_CAMERA_SENSOR_AIT848_ISP)
#define SENSOR_I2C_SLAVE_ID 		(0x06>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9P111)
#define SENSOR_I2C_SLAVE_ID 		(0x7A>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9T111)
#define SENSOR_I2C_SLAVE_ID 		(0x7A>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9T113)
#define SENSOR_I2C_SLAVE_ID 		(0x7A>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_MV9317)
#define SENSOR_I2C_SLAVE_ID 		(0x50>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9D112)
#define SENSOR_I2C_SLAVE_ID 		(0x7A>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_OV3640)
#define SENSOR_I2C_SLAVE_ID 		(0x78>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_S5K4BAFB)
#define SENSOR_I2C_SLAVE_ID 		(0x52>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_ISX006)
#define SENSOR_I2C_SLAVE_ID 		(0x34>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_OV7690)
#define SENSOR_I2C_SLAVE_ID 		(0x42>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_SIV100B)
#define SENSOR_I2C_SLAVE_ID 		(0x66>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9M113)
#define SENSOR_I2C_SLAVE_ID 		(0x78>>1)
#elif defined(CONFIG_VIDEO_ATV_SENSOR_TVP5150)
#define SENSOR_I2C_SLAVE_ID 		(0xB8>>1)
#elif defined(CONFIG_VIDEO_ATV_SENSOR_RDA5888)
#define SENSOR_I2C_SLAVE_ID 		(0xC4>>1)
#elif defined(CONFIG_VIDEO_CAMERA_NEXTCHIP_TEST)
#define SENSOR_I2C_SLAVE_ID 		(0x50>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_S5K5CAGA)
#define SENSOR_I2C_SLAVE_ID 		(0x5A>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_SR130PC10)
#define SENSOR_I2C_SLAVE_ID 		(0x40>>1)
#elif defined(CONFIG_VIDEO_HDMI_IN_SENSOR_EP9351)
#define SENSOR_I2C_SLAVE_ID 		(0x78>>1)
#elif defined(CONFIG_VIDEO_HDMI_IN_SENSOR_EP9553T)
#define SENSOR_I2C_SLAVE_ID 		(0x78>>1)
#endif
#endif // defined(CONFIG_VIDEO_TCCXX_CAMERA)





#if defined(CONFIG_VIDEO_TCCXX_CAMERA)
static struct cam_i2c_platform_data cam_i2c_data1 = {
};


static struct i2c_board_info __initdata i2c_camera_devices[] = {
	#if defined(CONFIG_VIDEO_DUAL_CAMERA_SUPPORT)
	{
		#if defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9P111) || defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9T111)
			I2C_BOARD_INFO("tcc-cam-sensor-0", (0x7A>>1)), //20100716 ysseung   sign-up to sensor slave-id.
		#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_S5K5CAGA)
			I2C_BOARD_INFO("tcc-cam-sensor-0", (0x5A>>1)),  //20121114 swhwang add
		#endif
		.platform_data = &cam_i2c_data1,
	},
	{
		#if defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9M113)
			I2C_BOARD_INFO("tcc-cam-sensor-1", (0x78>>1)), //20100716 ysseung   sign-up to sensor slave-id.
		#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_SR130PC10) 
			I2C_BOARD_INFO("tcc-cam-sensor-1", (0x40>>1)), //20121114 swhwang add 
		#endif
		.platform_data = &cam_i2c_data1,
	},
	#else // CONFIG_VIDEO_DUAL_CAMERA_SUPPORT
	{
		I2C_BOARD_INFO("tcc-cam-sensor", SENSOR_I2C_SLAVE_ID), 
		.platform_data = &cam_i2c_data1,
	},
	#endif // CONFIG_VIDEO_DUAL_CAMERA_SUPPORT
};
#endif // CONFIG_VIDEO_TCCXX_CAMERA	
 
//void __init tcc8930st_init_camera(void)
void __init tcc8930st_init_camera(void)
{
	if (!machine_is_tcc8930st())
		return;
	#if defined(CONFIG_VIDEO_TCCXX_CAMERA)
		#if defined(CONFIG_MACH_TCC8930ST)
			i2c_register_board_info(2, i2c_camera_devices, ARRAY_SIZE(i2c_camera_devices));
		#endif
	#endif

	

}

//device_initcall(tcc8930_init_camera);
