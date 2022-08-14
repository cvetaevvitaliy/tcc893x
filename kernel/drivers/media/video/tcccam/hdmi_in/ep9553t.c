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
#include <mach/tcc_hdmi_in_parameters.h>
#include <asm/io.h>
#include "sensor_if.h"
#include "cam.h"
#include "tcc_cam_i2c.h"
#include "ep9553t.h"

#if defined(CONFIG_VIDEO_HDMI_IN_SENSOR_EP9553T)


unsigned short PRV_W = 0;
unsigned short PRV_H = 0;
unsigned short CAP_W = 0;
unsigned short CAP_H = 0;

unsigned char hdmi_in_Interlaced;
//unsigned char hdmi_in_vin_fmt;
//unsigned char hdmi_in_vin_yuvmode;

static int debug_video	   = 0;
static int debug_sound	   = 0;
#define vprintk(msg...)	if(debug_video) { printk( "ep9553t(Video): " msg); }
#define sprintk(msg...)	if(debug_sound) { printk( "ep9553t(Sound): " msg); }

struct capture_size sensor_sizes[] =
{
	{1280, 720},
	{1980, 1080}
};

unsigned char tcc_get_ep9553e_vin_size(void)
{
	

	unsigned char  V_TIME   =	0x00;
	unsigned short reg      =	0x0301; 	// V-TIME : Decoded Video Timing Register

	hdmi_in_Interlaced = false; //default : Progressive, true = Interlaced, false = Progressive

	PRV_W = CAP_W = 0;
	PRV_H = CAP_H = 0;

	DDI_I2C_Read_HDMI_IN(reg, 2, &V_TIME, 1);
	vprintk("read register 0x%04x, Value is 0x%02x \n",reg,V_TIME);
	
	if(V_TIME == 0x01){

		reg      =	0x0600;
		DDI_I2C_Read_HDMI_IN(reg, 2, &V_TIME, 1);
		vprintk("read register 0x%04x, Value is 0x%02x \n",reg,V_TIME);
		
		if(V_TIME == 0x02){

			reg      =	0x0601;
			DDI_I2C_Read_HDMI_IN(reg, 2, &V_TIME, 1);
			vprintk("read register 0x%04x, Value is 0x%02x \n",reg,V_TIME);
			
			if(V_TIME == 0x80)	V_TIME = 0x01;
			else				V_TIME = 0x00;
			
		}
		else{
			V_TIME = 0x00;
		}
	}

	//CEA-861D
	switch(V_TIME){

		// 640 x 480
		case 0x01:
			PRV_W = CAP_W = 640;
			PRV_H = CAP_H = 480;
			V_TIME = SIZE_640_480P;
			break;

		// 720 x 480
		case 0x30:	case 0x31:	case 0x02:
		case 0x03:	case 0x38:	case 0x39:
			PRV_W = CAP_W = 720;
			PRV_H = CAP_H = 480;
			V_TIME = SIZE_720_480P;
			break;
		case 0x06:	case 0x07:
			hdmi_in_Interlaced = true;
			PRV_W = CAP_W = 720;
			PRV_H = CAP_H = 240;
			V_TIME = SIZE_720_480I;
			break;

		// 720 x 576
		case 0x11:	case 0x12:	case 0x2A:
		case 0x2B:	case 0x34:	case 0x35:
			PRV_W = CAP_W = 720;
			PRV_H = CAP_H = 576;
			V_TIME = SIZE_720_576P;
			break;

		//1440 x 480
		case 0x32:	case 0x33:
		case 0x3A:	case 0x3B: 
			hdmi_in_Interlaced = true;
			PRV_W = CAP_W = 1440;
			PRV_H = CAP_H = 240;
			V_TIME = SIZE_1440_480I;
			break;
		case 0x0E:	case 0x0F:	
			PRV_W = CAP_W = 1440;
			PRV_H = CAP_H = 480;
			V_TIME = SIZE_1440_480P;
			break;

		// 1440 x 240
		case 0x08:			
		case 0x09:		
			PRV_W = CAP_W = 1440;
			PRV_H = CAP_H = 240;
			V_TIME = SIZE_1440_240P;
			break;

		// 1440 x 576
		case 0x15:	case 0x16:	case 0x2C:
		case 0x2D:	case 0x36:	case 0x37:
			hdmi_in_Interlaced = true;
			PRV_W = CAP_W = 1440;
			PRV_H = CAP_H = 288;
			V_TIME = SIZE_1440_576I;
			break;
		case 0x1D:
		case 0x1E:
			PRV_W = CAP_W = 1440;
			PRV_H = CAP_H = 576;
			V_TIME = SIZE_1440_576P;			
			break;

		// 1440 x 288
		case 0x17:	case 0x18:		
		case 0x1B:	case 0x1C:	
			PRV_W = CAP_W = 1440;
			PRV_H = CAP_H = 288;
			V_TIME = SIZE_1440_288P;			
			break;

		// 1280 x 720
		case 0x04: 	case 0x13: 		
		case 0x29:	case 0x2F:
			PRV_W = CAP_W = 1280;
			PRV_H = CAP_H = 720;
			V_TIME = SIZE_1280_720P;					
			break;

		// 1920 x 1080
		case 0x05: 	case 0x14: 	case 0x27: 
		case 0x28: 	case 0x2E:
			hdmi_in_Interlaced = true;
			PRV_W = CAP_W = 1920;
			PRV_H = CAP_H = 540;
			V_TIME = SIZE_1920_1080I;					
			break;
			
		case 0x10: 	case 0x1F: 	case 0x20: 
		case 0x21: 	case 0x22: 
			PRV_W = CAP_W = 1920;
			PRV_H = CAP_H = 1080;
			V_TIME = SIZE_1920_1080P;					
			break;

		// 2880 x 480
		case 0x0A: 
		case 0x0B: 
			hdmi_in_Interlaced = true;
			PRV_W = CAP_W = 2880;
			PRV_H = CAP_H = 240;
			V_TIME = SIZE_2880_480I;								
			break;			
		case 0x23:
		case 0x24:
			PRV_W = CAP_W = 2880;
			PRV_H = CAP_H = 480;
			V_TIME = SIZE_2880_480P;				
			break;

		// 2880 x 240
		case 0x0C: 
		case 0x0D: 			
			PRV_W = CAP_W = 2880;
			PRV_H = CAP_H = 240;
			V_TIME = SIZE_2880_240P;				
			break;

		// 2880 x 576
		case 0x19:			
		case 0x1A:	
			hdmi_in_Interlaced = true;
			PRV_W = CAP_W = 2880;
			PRV_H = CAP_H = 288;
			V_TIME = SIZE_2880_576I;				
			break;
		case 0x25:
		case 0x26:
			PRV_W = CAP_W = 2880;
			PRV_H = CAP_H = 576;
			V_TIME = SIZE_2880_576P;					
			break;


	}
	
	vprintk("Preview Size is %d x %d \n",PRV_W,PRV_H);
	return V_TIME;
}


#if 0 //use static color format "RGB 444"
/*** EP9553E Input Video Format Indicator

4¡¯b_0000	RGB444_LR (Limited Range, ITU601 detected)		
4¡¯b_0100	RGB444_LR (Limited Range, ITU709 detected)		
4¡¯b_0x10	RGB444_FR (Full Range)		
4¡¯b_1000	YUV444 with Color Space ITU601		
4¡¯b_1001	YUV422 with Color Space ITU601		
4¡¯b_1010	YUV444 with Color Space xvYCC601		
4¡¯b_1011	YUV422 with Color Space xvYCC601		
4¡¯b_1100	YUV444 with Color Space ITU709		
4¡¯b_1101	YUV422 with Color Space ITU709		
4¡¯b_1110	YUV444 with Color Space xvYCC709		
4¡¯b_1111	YUV422 with Color Space xvYCC709		

***/
void tcc_get_ep9553e_vin_fmt()
{
	
	unsigned char  VIN_FMT   =	0x00;
	unsigned short reg      =	0x0300; 	// V-TIME : Decoded Video Timing Register
	unsigned char  prv_VIN_FMT = 0x00;
	unsigned int   cnt		=	0;
	
	hdmi_in_vin_yuvmode = false; //default : RGB mode, true : yuv mode, false : RGB mode
	

	
	for(cnt = 0;cnt<10;cnt++){
		DDI_I2C_Read_HDMI_IN(reg, 2, &VIN_FMT, 1);
		vprintk("HDMI IN Video IN register 0x%04x, Value is 0x%02x \n",reg,VIN_FMT);
		
		if(VIN_FMT != prv_VIN_FMT){
			if(VIN_FMT != 0x00)	prv_VIN_FMT = VIN_FMT;
			sensor_delay(500);
		} else {
			break;
		}
	}

	
	switch(VIN_FMT){

		case 0x08: // YUV444 with Color Space ITU601	
		case 0x0A: // YUV444 with Color Space xvYCC601
		case 0x0C: // YUV444 with Color Space ITU709	
		case 0x0E: // YUV444 with Color Space xvYCC709
			hdmi_in_vin_yuvmode = true;
		case 0x00:  // RGB444_LR (Limited Range, ITU601 detected)	
		case 0x04:	// RGB444_LR (Limited Range, ITU709 detected)		
		case 0x02: // RGB444_FR (Full Range)	
		case 0x06:
		
			hdmi_in_vin_fmt = FMT_RGB444_24BIT;
			break;
			
			
		case 0x0D: // YUV422 with Color Space ITU709
		case 0x0B: // YUV422 with Color Space xvYCC601
		case 0x09: // YUV422 with Color Space ITU601
		case 0x0F: // YUV422 with Color Space xvYCC709	
			hdmi_in_vin_yuvmode = true;		
			hdmi_in_vin_fmt = FMT_YUV422_16BIT;
			break;
	
		default:
			hdmi_in_vin_fmt = FMT_RGB444_24BIT;
			
	}

}
#endif

unsigned char tcc_get_ep9553e_audio_samplerate(void)
{
	unsigned char  value	=	0x00;
	unsigned short reg      =	0x0400; 	

	do{
		DDI_I2C_Read_HDMI_IN(reg, 2, &value, 1);
	}while(value == 0xb7);

	sprintk("register 0x%04x, Value is 0x%02x \n",reg,value);
	
	return value;
}

static int sensor_close(void)
{
	unsigned char close_value = 0x12;
	unsigned short reg       = 0x2100;
	DDI_I2C_Write_HDMI_IN(reg, &close_value, 2, 1);

	return 0;
}

static int sensor_open(void)
{
	

	unsigned char init_value = 0x00;
	unsigned short reg       = 0x00;
	
	// KeyScan(Mode Change) Register Control(to HDMI mode)
	reg = 0x2100;
	init_value = 0x10; // the register 0x2100 bit1, PD_HDMI, to "0" through IIC.
	DDI_I2C_Write_HDMI_IN(reg, &init_value, 2, 1);
	sensor_delay(5000); // waiting for C BUS connect

	// the register 0x41 bit5 swing for support "MHL Mode"  2013 09 12 swhwang 
	reg = 0x41;
	init_value = 0x2C; 
	DDI_I2C_Write_HDMI_IN(reg, &init_value, 1, 1);
	
	init_value = 0x0C; 
	DDI_I2C_Write_HDMI_IN(reg, &init_value, 1, 1);

	return 0;
}


static int sensor_preview(void)
{
	return 0;
}

static int sensor_capture(void)
{
	return 0;
}

static int sensor_capturecfg(int width, int height)
{
	return 0;
}

static int sensor_zoom(int val)
{
	return 0;
}

static int sensor_autofocus(int val)
{
	return 0;
}

static int sensor_effect(int val)
{
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
	return 0;
}

static int sensor_bright(int val)
{
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

static int sensor_get_VideoSize(void)
{
	return tcc_get_ep9553e_vin_size();
}

static int sensor_get_AudioSR(void)
{
	return tcc_get_ep9553e_audio_samplerate();
}


/**********************************************************
*    Function  
**********************************************************/
void sensor_init_fnc(SENSOR_FUNC_TYPE *sensor_func)
{
	sensor_func->Open 			= sensor_open;
	sensor_func->Close 		= sensor_close;

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
	sensor_func->Set_Scene 	= sensor_scene;
	sensor_func->GetVideoSize	= sensor_get_VideoSize;
	sensor_func->GetAudioSR	= sensor_get_AudioSR;
	sensor_func->Check_ESD 	= sensor_check_esd;
	sensor_func->Check_Luma 	= sensor_check_luma;

}

#endif //CONFIG_VIDEO_HDMI_IN_SENSOR_EP9553T
