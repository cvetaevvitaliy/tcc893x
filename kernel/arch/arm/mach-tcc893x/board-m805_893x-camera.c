/* linux/arch/arm/mach-tcc893x/board-m805_893x-camera.c
 *
 * Copyright (C) 2012 Telechips, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <asm/mach-types.h>
#include "devices.h"
#include "board-tcc8930.h"
#include <asm/system.h>

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
#define SENSOR_I2C_SLAVE_ID 		(0x78>>1)
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
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_HI704)
#define SENSOR_I2C_SLAVE_ID 		(0x60>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_GC0329) // 2012.12.04 swhwang sign-up to sensor slave-id
#define SENSOR_I2C_SLAVE_ID 		(0x62>>1)
#endif
#endif // defined(CONFIG_VIDEO_TCCXX_CAMERA)


#if defined(CONFIG_VIDEO_TCCXX_CAMERA)
static struct cam_i2c_platform_data cam_i2c_data1 = {
};

static struct i2c_board_info __initdata i2c_camera_devices[] = {
	#if defined(CONFIG_VIDEO_DUAL_CAMERA_SUPPORT)
	{
		#if defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9P111) || defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9T111)
			I2C_BOARD_INFO("tcc-cam-sensor-0", (0x7A>>1)), 
		#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_OV3660) || defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9D112)
			I2C_BOARD_INFO("tcc-cam-sensor-0", (0x78>>1)), 
		#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_S5K5CAGA)
			I2C_BOARD_INFO("tcc-cam-sensor-0", (0x5A>>1)),
		#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_HI253) // add by ccx on 20120725
			I2C_BOARD_INFO("tcc-cam-sensor-0", (0x40>>1)),
		#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_GC2035) // add by ccx on 20120725
			I2C_BOARD_INFO("tcc-cam-sensor-0", (0x78>>1)),
		#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_SP2518)
			I2C_BOARD_INFO("tcc-cam-sensor-0", (0x60>>1)),
		#endif
		.platform_data = &cam_i2c_data1,
	},
	{
		#if defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9M113)
			I2C_BOARD_INFO("tcc-cam-sensor-1", (0x78>>1)), 
		#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_HM1355)
			I2C_BOARD_INFO("tcc-cam-sensor-1", (0x48>>1)), //20100716 ysseung   sign-up to sensor slave-id.
		#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_SIV100B)
			I2C_BOARD_INFO("tcc-cam-sensor-1", (0x66>>1)), 
		#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_SR130PC10)
			I2C_BOARD_INFO("tcc-cam-sensor-1", (0x40>>1)), 
		#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_HI704)
			I2C_BOARD_INFO("tcc-cam-sensor-1", (0x60>>1)), //add by ccx on 20120725
		#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_GC0308)
			I2C_BOARD_INFO("tcc-cam-sensor-1", (0x42>>1)), //add by ccx on 20120725
		#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_SP0A18)
			I2C_BOARD_INFO("tcc-cam-sensor-1", (0x42>>1)),
		#endif
		.platform_data = &cam_i2c_data1,
	},
	#else // CONFIG_VIDEO_DUAL_CAMERA_SUPPORT
	{
		I2C_BOARD_INFO("tcc-cam-sensor", SENSOR_I2C_SLAVE_ID), //20100716 ysseung   sign-up to sensor slave-id.
		.platform_data = &cam_i2c_data1,
	},
	#endif // CONFIG_VIDEO_DUAL_CAMERA_SUPPORT
};
#endif // CONFIG_VIDEO_TCCXX_CAMERA	

void __init m805_893x_init_camera(void)
{
	if (!machine_is_m805_893x())
		return;

	#if defined(CONFIG_VIDEO_TCCXX_CAMERA)
		i2c_register_board_info(2, i2c_camera_devices, ARRAY_SIZE(i2c_camera_devices));
	#endif
}

//device_initcall(m805_893x_init_camera);
