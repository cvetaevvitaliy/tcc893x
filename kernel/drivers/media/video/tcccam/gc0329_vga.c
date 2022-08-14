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

#include <linux/delay.h>
#include <asm/system.h>
#include <mach/hardware.h>
#include <asm/io.h>
#include "sensor_if.h"
#include "cam.h"
#include "tcc_cam_i2c.h"

#if defined(CONFIG_ARCH_TCC92XX) || defined(CONFIG_ARCH_TCC93XX)  || defined(CONFIG_ARCH_TCC88XX) || defined(CONFIG_ARCH_TCC893X) || defined(CONFIG_ARCH_TCC892X) 
#include <mach/bsp.h>
#elif defined(CONFIG_ARCH_TCC79X)
#include <mach/tcc79x.h>
#endif

#ifdef CONFIG_VIDEO_CAMERA_SENSOR_GC0329

/* Array of image sizes supported by SP0A18.  These must be ordered from 
* smallest image size to largest.
*/
#if defined(CONFIG_VIDEO_DUAL_CAMERA_SUPPORT)
struct capture_size sensor_sizes_sp0a18[] = {
	{  640,  480 },	/* VGA */
	{  320,  240 },	/* QVGA */
};
#else
struct capture_size sensor_sizes[] = {
	{  640,  480 },	/* VGA */
	{  320,  240 },	/* QVGA */
};
#endif


/* register initialization tables for sensor */
/* common sensor register initialization for all image sizes, pixel formats, 
 * and frame rates
 */
static struct sensor_reg sensor_initialize[] = {
        {0xfe, 0x80},
        {0xfe, 0x80},

	//{REG_TERM, 0x00},
	
        {0xfc, 0x16},
        {0xfc, 0x16},
        {0xfc, 0x16},
        {0xfc, 0x16},
        {0xfe, 0x00},

        {0x73, 0x90},
        {0x74, 0x80}, 
        {0x75, 0x80}, 
        {0x76, 0x94}, 
        {0x42, 0x00}, 
        {0x77, 0x57}, 
        {0x78, 0x4d}, 
        {0x79, 0x45}, 
        //{0x42, 0xfc},
        ////////////////////analog////////////////////
        {0x0a, 0x02}, 
        {0x0c, 0x02}, 
        {0x17, 0x14},  // h_v switch
        {0x19, 0x05}, 
        {0x1b, 0x24}, 
        {0x1c, 0x04}, 
        {0x1e, 0x08}, 
        {0x1f, 0xc8},// c0 20120720 
        {0x20, 0x00}, 
        {0x21, 0x48}, 
        {0x22, 0xba}, // ba or 8a 20120720
        {0x23, 0x22},
        {0x24, 0x16},      
        ////////////////////blk////////////////////
        {0x26, 0xf7}, 
        {0x29, 0x80}, 
        {0x32, 0x04},
        {0x33, 0x20},
        {0x34, 0x20},
        {0x35, 0x20},
        {0x36, 0x20},
        ////////////////////ISP BLOCK ENABL////////////////////
        {0x40, 0xff},
        {0x41, 0x44}, 
        {0x42, 0x7e}, 
        {0x44, 0xa2}, 
#ifdef CONFIG_ARCH_MESON3
	{0x26,0x01}, // vsync mode 
#else
	{0x26,0x03}, //03
#endif
        {0x4b, 0xca},
        {0x4d, 0x01}, 
        {0x4f, 0x01},
        {0x70, 0x48}, 
        //{0xb0, 0x00},
        //{0xbc, 0x00},
        //{0xbd, 0x00},
        //{0xbe, 0x00},
        ////////////////////DNDD////////////////////
        {0x80, 0xe7},
        {0x82, 0x55},
        {0x83, 0x03}, // 20120423
        {0x87, 0x4a},
        ////////////////////INTPEE////////////////////
        {0x95, 0x45},
        ////////////////////ASDE////////////////////
        //{0xfe, 0x01},
        //{0x18, 0x22}, 
        //{0xfe, 0x00},
        //{0x9c, 0x0a}, 
        //{0xa0, 0xaf}, 
        //{0xa2, 0xff},
        //{0xa4, 0x50},
        //{0xa5, 0x21},
        //{0xa7, 0x35},
        ////////////////////RGB gamma////////////////////
        //RGB gamma m4'
        {0xbf, 0x06},
        {0xc0, 0x14},
        {0xc1, 0x27},
        {0xc2, 0x3b},
        {0xc3, 0x4f},
        {0xc4, 0x62},
        {0xc5, 0x72},
        {0xc6, 0x8d},
        {0xc7, 0xa4},
        {0xc8, 0xb8},
        {0xc9, 0xc9},
        {0xcA, 0xd6},
        {0xcB, 0xe0},
        {0xcC, 0xe8},
        {0xcD, 0xf4},
        {0xcE, 0xFc},
        {0xcF, 0xFF},
        //////////////////CC///////////////////
        {0xfe, 0x00},
        {0xb3, 0x44},
        {0xb4, 0xfd},
        {0xb5, 0x02},
        {0xb6, 0xfa},
        {0xb7, 0x48},
        {0xb8, 0xf0},
        // crop                                                    
        {0x50, 0x01},
        ////////////////////YCP////////////////////
        {0xfe, 0x00},
        {0xd1, 0x38},
        {0xd2, 0x38},
        {0xdd, 0x54},
        ////////////////////AEC////////////////////
        {0xfe, 0x01},
        {0x10, 0x40},
        {0x11, 0x21}, 
        {0x12, 0x07}, 
        {0x13, 0x50},
        {0x17, 0x88}, 
        {0x21, 0xb0},
        {0x22, 0x48},
        {0x3c, 0x95},
        {0x3d, 0x50},
        {0x3e, 0x48}, 
        ////////////////////AWB////////////////////
        {0xfe, 0x01},
        {0x06, 0x16},
        {0x07, 0x06},
        {0x08, 0x98},
        {0x09, 0xee},
        {0x50, 0xfc}, 
        {0x51, 0x20},
        {0x52, 0x0b},
        {0x53, 0x20}, 
        {0x54, 0x40}, 
        {0x55, 0x10}, 
        {0x56, 0x20}, 
        //{0x57, 0x40},
        {0x58, 0xa0}, 
        {0x59, 0x28}, 
        {0x5a, 0x02}, 
        {0x5b, 0x63}, 
        {0x5c, 0x34},
        {0x5d, 0x73}, 
        {0x5e, 0x11}, 
        {0x5f, 0x40}, 
        {0x60, 0x40}, 
        {0x61, 0xc8}, 
        {0x62, 0xa0}, 
        {0x63, 0x40}, 
        {0x64, 0x50}, 
        {0x65, 0x98}, 
        {0x66, 0xfa}, 
        {0x67, 0x70}, 
        {0x68, 0x58}, 
        {0x69, 0x85}, 
        {0x6a, 0x40},
        {0x6b, 0x39},
        {0x6c, 0x18},
        {0x6d, 0x28},
        {0x6e, 0x41}, 
        {0x70, 0x02}, 
        {0x71, 0x00}, 
        {0x72, 0x10},
        {0x73, 0x40}, 
        {0x74, 0x40},
        {0x75, 0x58},
        {0x76, 0x24},
        {0x77, 0x40},
        {0x78, 0x20},
        {0x79, 0x60},
        {0x7a, 0x58},
        {0x7b, 0x20},
        {0x7c, 0x30},
        {0x7d, 0x35},
        {0x7e, 0x10},
        {0x7f, 0x08},
        {0x81, 0x50}, 
        {0x82, 0x42}, 
        {0x83, 0x40}, 
        {0x84, 0x40}, 
        {0x85, 0x40}, 
        
        ////////////////////ABS////////////////////
        {0x9c, 0x02}, 
        {0x9d, 0x20},
        //{0x9f, 0x40}, 
        ////////////////////CC-AWB////////////////////
        {0xd0, 0x00},
        {0xd2, 0x2c},
        {0xd3, 0x80}, 
        ////////////////////LSC///////////////////
        {0xfe, 0x01},
        {0xa0, 0x00},
        {0xa1, 0x3c},
        {0xa2, 0x50},
        {0xa3, 0x00},
        {0xa8, 0x0f},
        {0xa9, 0x08},
        {0xaa, 0x00},
        {0xab, 0x04},
        {0xac, 0x00},
        {0xad, 0x07},
        {0xae, 0x0e},
        {0xaf, 0x00},
        {0xb0, 0x00},
        {0xb1, 0x09},
        {0xb2, 0x00},
        {0xb3, 0x00},
        {0xb4, 0x31},
        {0xb5, 0x19},
        {0xb6, 0x24},
        {0xba, 0x3a},
        {0xbb, 0x24},
        {0xbc, 0x2a},
        {0xc0, 0x17},
        {0xc1, 0x13},
        {0xc2, 0x17},
        {0xc6, 0x21},
        {0xc7, 0x1c},
        {0xc8, 0x1c},
        {0xb7, 0x00},
        {0xb8, 0x00},
        {0xb9, 0x00},
        {0xbd, 0x00},
        {0xbe, 0x00},
        {0xbf, 0x00},
        {0xc3, 0x00},
        {0xc4, 0x00},
        {0xc5, 0x00},
        {0xc9, 0x00},
        {0xca, 0x00},
        {0xcb, 0x00},
        {0xa4, 0x00},
        {0xa5, 0x00},
        {0xa6, 0x00},
        {0xa7, 0x00},
        /////20120502////
        {0xfe,0x01},
        {0x18,0x22},
        {0x21,0xc0},
        {0x06,0x12},
        {0x08,0x9c},
        {0x51,0x28},
        {0x52,0x10},
        {0x53,0x20},
        {0x54,0x40},
        {0x55,0x16},
        {0x56,0x30},
        {0x58,0x60},
        {0x59,0x08},
        {0x5c,0x35},
        {0x5d,0x72},
        {0x67,0x80},
        {0x68,0x60},
        {0x69,0x90},
        {0x6c,0x30},
        {0x6d,0x60},
        {0x70,0x10},
        {0xfe,0x00},
        {0x9c,0x0a},
        {0xa0,0xaf},
        {0xa2,0xff},
        {0xa4,0x60},
        {0xa5,0x31},
        {0xa7,0x35},
        {0x42,0xfe},
        {0xd1,0x34},
        {0xd2,0x34},
        {0xfe,0x00},
        ////////////////////asde ///////////////////
        //{0xa0, 0xaf},
        //{0xa2, 0xff},
       // {0x44, 0xa1},//0xa3}, // yuv order
        {0x44, 0xa0}, // yuv order -> MID 7048
        {0x46, 0x33},
        {0xf0, 0x07},
        {0xf1, 0x01},

    {REG_TERM, VAL_TERM}
};


static struct sensor_reg sensor_set_preview[] = {
	
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_set_capture[] = {
	{REG_TERM, VAL_TERM}
};

 struct sensor_reg* sensor_reg_common[3] =
{
	sensor_initialize,
	sensor_set_preview,
	sensor_set_capture
};

static struct sensor_reg sensor_brightness_0[] = {
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_brightness_1[] = {
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_brightness_2[] = {
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_brightness_3[] = {
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_brightness_4[] = {
    {REG_TERM, VAL_TERM}
};

struct sensor_reg* sensor_reg_brightness[5] =
{
	sensor_brightness_0,
	sensor_brightness_1,
	sensor_brightness_2,
	sensor_brightness_3,
	sensor_brightness_4
};


static struct sensor_reg sensor_awb_auto[] = {
	{0xfe, 0x00},
        {0x44, 0xa0},//0xa3}, // yuv order
        {0xf0, 0x07},
        {0xf1, 0x01},	
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_awb_daylight[] = {
	{0xfe, 0x00},
        {0x44, 0xa1},//0xa3}, // yuv order
        {0xf0, 0x07},
        {0xf1, 0x01},
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_awb_incandescent[] = {
	{0xfe, 0x00},
        {0x44, 0xa2},//0xa3}, // yuv order
        {0xf0, 0x07},
        {0xf1, 0x01},
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_awb_fluorescent[] = {
	{0xfe, 0x00},
        {0x44, 0xa3},//0xa3}, // yuv order
        {0xf0, 0x07},
        {0xf1, 0x01},
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_awb_cloudy[] = {
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_awb_sunset[] = {
    {REG_TERM, VAL_TERM}
};

struct sensor_reg* sensor_reg_awb[6] =
{
	sensor_awb_auto,
	sensor_awb_daylight,
	sensor_awb_incandescent,
	sensor_awb_fluorescent,
	sensor_awb_cloudy,
	sensor_awb_sunset
	
};


static struct sensor_reg sensor_iso_auto[] = {
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_iso_100[] = {
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_iso_200[] = {
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_iso_400[] = {
	{REG_TERM, VAL_TERM}
};

struct sensor_reg* sensor_reg_iso[4] =
{
	sensor_iso_auto,
	sensor_iso_100,
	sensor_iso_200,
	sensor_iso_400
};


static struct sensor_reg sensor_effect_normal[] = {
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_effect_gray[] = {
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_effect_negative[] = {
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_effect_sepia[] = {
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_effect_sharpness[] = {
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_effect_sketch[] = {
    {REG_TERM, VAL_TERM}
};

struct sensor_reg* sensor_reg_effect[6] =
{
	sensor_effect_normal,
	sensor_effect_gray,
	sensor_effect_negative,
	sensor_effect_sepia,
	sensor_effect_sharpness,
	sensor_effect_sketch,
};


static struct sensor_reg sensor_reg_flipnone[] = {
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_reg_hflip[] = {
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_reg_vflip[] = {
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_reg_hvflip[] = {
    {REG_TERM, VAL_TERM}
};

struct sensor_reg* sensor_reg_flip[4] =
{
	sensor_reg_flipnone,
	sensor_reg_hflip,
	sensor_reg_vflip,
	sensor_reg_hvflip,
};


static struct sensor_reg sensor_secne_auto[] = {
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_secne_night[] = {
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_secne_landscape[] = {
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_secne_portrait[] = {
	{REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_secne_sport[] = {
	{REG_TERM, VAL_TERM}
};

struct sensor_reg* sensor_reg_scene[5] =
{
	sensor_secne_auto,
	sensor_secne_night,
	sensor_secne_landscape,
	sensor_secne_portrait,
	sensor_secne_sport
};

static struct sensor_reg sensor_me_mtrix[] = {
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_me_center_weighted[] = {
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_me_spot[] = {
    {REG_TERM, VAL_TERM}
};

struct sensor_reg* sensor_reg_metering_exposure[3] =
{
	sensor_me_mtrix,
	sensor_me_center_weighted,
	sensor_me_spot,
};

static struct sensor_reg sensor_af_single[] = {
    {REG_TERM, VAL_TERM}
};

static struct sensor_reg sensor_af_manual[] = {
    {REG_TERM, VAL_TERM}
};

struct sensor_reg* sensor_reg_af[2] =
{
	sensor_af_single,
	sensor_af_manual,
};

static int write_regs(const struct sensor_reg reglist[])
{
	int err;
	int err_cnt = 0;	
	int sleep_cnt = 100;
	unsigned char data[132];
	unsigned char bytes;
	const struct sensor_reg *next = reglist;
	
	while (!((next->reg == REG_TERM) && (next->val == VAL_TERM)))
	{
#if 0		
		if(next->reg == REG_TERM && next->val != VAL_TERM)
		{
			//mdelay(next->val);
			msleep(next->val);
			sleep_cnt = 100;
			printk("Sensor init Delay[%d]!!!! \n", next->val);
			next++;
		}
		else
		{
			sleep_cnt--;
			if(sleep_cnt == 0)
			{
				msleep(10);
				sleep_cnt = 100;
			}
			
			bytes = 0;
			data[bytes]= next->reg>>8;		bytes++;		
			data[bytes]= (u8)next->reg&0xff; 	bytes++;

			data[bytes]= next->val>>8;		bytes++;		
			data[bytes]= (u8)next->val&0xff; 	bytes++;
			
	        err = DDI_I2C_Write(data, 2, bytes-2);
	        
	        if (err)
	        {
				err_cnt++;
				if(err_cnt >= 3)
				{
					printk("ERROR: Sensor I2C !!!! \n"); 
					return err;
				}
	        }
	        else
	        {
	        	err_cnt = 0;
	            next++;
	        }
		}
#endif
		bytes = 0;
		data[bytes]= next->reg&0xff; 	bytes++;
		data[bytes]= next->val&0xff;
		
        err = DDI_I2C_Write(data, 1, 1);

#if 0
		if(!err)//read test
		{			
			char data_r[2];
			unsigned char data_read = 0;
			
			DDI_I2C_Read(next->reg, 1, data_r, 1);
			data_read = data_r[0];

			if(next->val != data_read)
			{
				err = 1;
				printk("Err: 0x%x-(0x%x -> 0x%x) !!!! \n", next->reg, next->val, data_read);
			}
		}
#endif		
        if (err)
        {
			err_cnt++;

			if(err_cnt >= 3)
			{
				printk("ERROR: Sensor I2C !!!! \n"); 
				#if 0
				return err;
				#else
				printk("Sam: Ok,Although I2C has error,we need go ahead\n");
				return 0;
				#endif
			}
        }
        else
        {
#ifdef CONFIG_COBY_MID7025
			//printk("DDI_I2C_Write Ok!!! \n"); 
#endif		
        	err_cnt = 0;
            next++;
        }

	}

	return 0;
}



static int sensor_open(bool bChangeCamera)
{

	unsigned int id = 0;
	int i;
	int ret = 0;
	unsigned char reg[2] = {0}, val = 0, data[3]={0};
	unsigned short cmd = 0;

	CIF_Open();
        sensor_delay(40);
	
	sensor_power_disable();
	sensor_delay(10);
	
	sensor_power_enable();
	sensor_delay(10);

	sensor_powerdown_enable();
	sensor_delay(10);
	
	sensor_powerdown_disable();
	sensor_delay(10);
	
	sensor_reset_low();
	sensor_delay(10);
	/*
	CIF_Open();
	sensor_delay(40);
	*/	
	sensor_reset_high();
	sensor_delay(15);

	printk("init sensor GC0329 !!!! \n");
/*
	id = 0x6a;
	DDI_I2C_Write(&id,0x06,1);
	id = 0x70;
	DDI_I2C_Write(&id,0x08,1);
	id = 0xe8;
	DDI_I2C_Write(&id,0x0e,1);
	id = 0x88;
	DDI_I2C_Write(&id,0x10,1);
	for(i = 0;i<256;i++){
		DDI_I2C_Read(i, 1, &id, 1);
		printk("read reg[0x%x] = 0x%x\n",i,id);
	}
*/
	DDI_I2C_Read(0xfe, 1, &id, 1);	
	printk("read 0xfe : 0x%x\n", id);
	DDI_I2C_Read(0x00, 1, &id, 1);
	printk("read sensor ID0 : 0x%x\n", id); 
	DDI_I2C_Read(0xfb, 1, &id, 1);
        printk("read sensor ID1 : 0x%x\n", id);
	
	
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	ret = write_regs(sensor_reg_common[0]);

	return ret;
}

static int sensor_close(void)
{
	CIF_ONOFF(OFF);

	sensor_reset_low();
	sensor_power_disable();
	sensor_powerdown_enable();

	CIF_Close();   
	msleep(5);    

	return 0;
}

static int sensor_preview(void)
{
	//GT2005_Preview();
	//return 0;
	return write_regs(sensor_reg_common[1]);

}

static int sensor_capture(void)
{
	//GT2005_Capture();
	//return 0;
	return write_regs(sensor_reg_common[2]);
}

static int sensor_capturecfg(int width, int height)
{
	return 0;
}

static int sensor_zoom(int val)
{
	//GT2005_YUV_SensorSetting(CAM_PARAM_ZOOM_FACTOR, val);
	return 0;
}

static int sensor_autofocus(int val)
{	
	return 0;
}

static int sensor_effect(int val)
{
	//GT2005_YUV_SensorSetting(CAM_PARAM_EFFECT, val);
	return 0;
}

static int sensor_flip(int val)
{
	return 0;
}

static int sensor_iso(int val)
{
	return 0;
}

static int sensor_me(int val)
{
	return 0;
}

static int sensor_wb(int val)
{
	//GT2005_YUV_SensorSetting(CAM_PARAM_WB, val);
	return 0;
}

static int sensor_bright(int val)
{
	//GT2005_YUV_SensorSetting(CAM_PARAM_BRIGHTNESS, val);
	return 0;
}

static int sensor_scene(int val)
{
	return 0;
}

static int sensor_check_esd(int val)
{
	return 0;
}

static int sensor_check_luma(int val)
{
	return 0;
}

/**********************************************************
*    Function  
**********************************************************/
void sensor_init_fnc(SENSOR_FUNC_TYPE *sensor_func)
{
	sensor_func->Open 			= sensor_open;
	sensor_func->Close 			= sensor_close;

	sensor_func->Set_Preview 	= sensor_preview;
	sensor_func->Set_Capture 	= sensor_capture;
	sensor_func->Set_CaptureCfg = sensor_capturecfg;

	sensor_func->Set_Zoom		= sensor_zoom;
	sensor_func->Set_AF 		= sensor_autofocus;
	sensor_func->Set_Effect 	= sensor_effect;
	sensor_func->Set_Flip 		= sensor_flip;
	sensor_func->Set_ISO 		= sensor_iso;
	sensor_func->Set_ME 		= sensor_me;
	sensor_func->Set_WB 		= sensor_wb;
	sensor_func->Set_Bright 	= sensor_bright;
	sensor_func->Set_Scene 		= sensor_scene;

	sensor_func->Check_ESD 		= sensor_check_esd;
	sensor_func->Check_Luma 	= sensor_check_luma;
}

#endif

