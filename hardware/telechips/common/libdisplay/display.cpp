//-------------------------------------------------------------------
// Copyright Telechips Co., Ltd
// All right reserved.
//
// This software is the confidential and proprietary information
// of Samsung Electronics, Inc. ("Confidential Information").  You
// shall not disclose such Confidential Information and shall use
// it only in accordance with the terms of the license agreement
// you entered into with Samsung Electronics.
//-------------------------------------------------------------------
#include <fcntl.h>
#include <errno.h>
#include <cutils/log.h>
#include <cutils/atomic.h>
#include <stdlib.h>
#include <mach/tccfb_ioctrl.h>

#define LOG_TAG "DisplayDevice"

#define LOG_NDEBUG			0

#define FB0_DEVICE	"/dev/graphics/fb0"

/* ----------------------------------------------------
Function : DisplayDevice_Set_CalibrationSet 
Saturation, Contrast, Hue Calibration Setting Value  : -128 ~ +127 
Saturation default value : 0
Contrast default value : 0x20
HUE default value : 0
------------------------------------------------------*/
int DisplayDevice_Set_CalibrationSet(int Saturation, int Contrast , int Hue)
{
	int Brightness = 0;
	int fd = -1;
	unsigned int Lcd_Lcdc_N;
	lcdc_colorenhance_params lcdc_colorenhance;

        fd = open(FB0_DEVICE, O_RDWR);
        if (fd < 0) {
            ALOGE("can't open file '%s' (%s)", FB0_DEVICE, strerror(errno));
            return -EINVAL;
        }

	if (ioctl(fd,TCC_LCDC_GET_NUM, &Lcd_Lcdc_N) ) {
		ALOGE("ioctl(TCC_LCDC_GET_NUM) failed!\n");
		close(fd);
		return -EINVAL;
	}

	lcdc_colorenhance.brightness = 0;
	lcdc_colorenhance.contrast = 0x20;
	lcdc_colorenhance.hue = 0;

	lcdc_colorenhance.lcdc_num = Lcd_Lcdc_N;

	// change to byte 2's complement
	if((Brightness <= 127) && (Brightness >= -128))
		(Brightness >= 0) ? lcdc_colorenhance.brightness = Brightness : lcdc_colorenhance.brightness = 256 -Brightness;
	
	if((Contrast <= 127) && (Contrast >= -128))
		(Contrast >= 0) ? lcdc_colorenhance.contrast = Contrast : lcdc_colorenhance.contrast = 256 -Contrast;

	if((Hue <= 127) && (Hue >= -128))
		(Hue >= 0) ? lcdc_colorenhance.hue = Hue : lcdc_colorenhance.hue = 256 -Hue;

	ALOGI("Display color enhnace , Brightness:%d [0x%x], Contrast:%d [0x%x], Hue :%d [0x%x] \n", 
		Brightness, lcdc_colorenhance.brightness, Contrast, lcdc_colorenhance.contrast, Hue, lcdc_colorenhance.hue);

	if (ioctl(fd,TCC_LCDC_SET_COLORENHANCE, &lcdc_colorenhance) ) {
	    ALOGE("ioctl(TCC_LCDC_SET_COLORENHANCE) failed!\n");
	}

	close(fd);

	return 0;
}


