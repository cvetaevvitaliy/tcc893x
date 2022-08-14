/****************************************************************************
Copyright (C) 2013 Telechips Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
****************************************************************************/

#include <utils/Log.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <cutils/properties.h>
#include <mach/tccfb_ioctrl.h>
#include <mach/tcc_composite_ioctl.h>

#define FB_DEVICE				"/dev/graphics/fb0"
#define COMPOSITE_DEVICE 		"/dev/composite"

#define LOG_TAG					"COMPOSITE_APP"
#define COMPOSITE_APP_DEBUG		0
#if COMPOSITE_APP_DEBUG
#define DPRINTF(args...)    ALOGD(args)
#else
#define DPRINTF(args...)
#endif

static int composite_fd = -1;
static int fb_fd = -1;

int composite_fb_open(void)
{
	DPRINTF("%s", __func__);
	
	if(fb_fd > 0)
		return 1;

	// Open FB Device
	fb_fd = open(FB_DEVICE, O_RDWR);
	if(fb_fd <= 0) 
	{
		ALOGE("can't open tcc fb device '%s'", FB_DEVICE);
		return -1;
	}

	return 1;	
}

int composite_fb_close(void)
{
	DPRINTF("%s", __func__);
	
	// Close FB Device
	close(fb_fd);

	fb_fd = -1;

	return 1;	
}


unsigned int composite_outputmode_check(void)
{
	unsigned int OutputMode  = 0;
	
	//DPRINTF("%s", __func__);
	
	if (ioctl(fb_fd, TCC_LCDC_OUTPUT_MODE_CHECK, &OutputMode) ) {
		DPRINTF("%s failed!\n",__func__);
		return 0;
	}
	
	return OutputMode;
}


int composite_output_check(int *output_check)
{
	unsigned int OutputSelMode;
	OutputSelMode = composite_outputmode_check();

	if( OutputSelMode == OUTPUT_SELECT_NONE)
		*output_check = 1;
	else
		*output_check = 0;

	DPRINTF("%s output check:%d ", __func__, OutputSelMode);
	return 1;
}

int composite_fb_set_mode(int mode)
{
	DPRINTF("%s, mode=%d", __func__, mode);
	
	if(ioctl(fb_fd, TCC_LCDC_COMPOSITE_MODE_SET, &mode) != 0)
	{
		ALOGE("can't set composite mode");
		return -1;
	}

	return 1;	
}

int composite_open(void)
{
	DPRINTF("%s", __func__);
	
	if(composite_fd > 0)
		return 1;

	// Open Composite Device
	composite_fd = open(COMPOSITE_DEVICE, O_RDWR);
	if (composite_fd <= 0) 
	{
		ALOGE("can't open composite device '%s'", COMPOSITE_DEVICE);
		return -1;
	}
	
	return 1;	
}

int composite_close(void)
{
	DPRINTF("%s", __func__);
	
	// Close Composite Device
	close(composite_fd);

	composite_fd = -1;

	return 1;	
}

int composite_start(char lcdc)
{
	TCC_COMPOSITE_START_TYPE composite_start;
    char value[PROPERTY_VALUE_MAX];

	DPRINTF("%s", __func__);
	
    property_get("persist.sys.composite_mode", value, "");

	if(atoi(value) == 0)	{
		composite_start.mode = COMPOSITE_NTST_M;
		DPRINTF("composite_start() : NTSC");
	}
	else	{
		composite_start.mode = COMPOSITE_PAL_M;
		DPRINTF("composite_start() : PAL");
	}
	
	composite_start.lcdc = lcdc;

	if(ioctl(composite_fd, TCC_COMPOSITE_IOCTL_START, &composite_start) != 0)
	{
		ALOGE("can't start composite mode");
		return -1;
	}

	return 1;
}

int composite_end(void)
{
	DPRINTF("%s", __func__);
	
	if(ioctl(composite_fd, TCC_COMPOSITE_IOCTL_END, NULL) != 0)
	{
		ALOGE("can't end composite mode");
		return -1;
	}

	return 1;
}


/********************************************************
TCC Composite Control function 
	Composite sequence
1. init
2. suspend_check
3. detect_onoff(1)
4. output_set(1)
 composite display state
5. output_set(0)
6. dectect_onfoff(0)
7. deinit

**********************************************************/

int composite_display_init(void)
{
	if (composite_fb_open() < 0) 
	{
		DPRINTF("fail to open FB device\n");
		return NULL;
	}
	return NULL;
}

int composite_display_deinit()
{
	DPRINTF("%s", __func__);
	composite_fb_close();
    return 0;
}

unsigned int composite_lcdc_check(void)
{
    int composite_check  = 0;

	//DPRINTF("%s", __func__);
    if (ioctl(fb_fd, TCC_LCDC_HDMI_CHECK, &composite_check) ) {
        DPRINTF("%s TCC_LCDC_HDMI_CHECK failed!\n",__func__);
        return 0;
    }

    return composite_check;
}

unsigned int composite_suspend_check(void)
{
    int composite_suspend  = 0;

	//DPRINTF("%s", __func__);
    if (ioctl(composite_fd, TCC_COPOSITE_IOCTL_GET_SUSPEND_STATUS, &composite_suspend) ) {
        DPRINTF("%s TCC_COPOSITE_IOCTL_GET_SUSPEND_STATUS failed!\n",__func__);
        return 0;
    }

    return composite_suspend;
}

int composite_display_detect_onoff(char onoff)
{
	int ret;
	if(onoff)
	{
		ret = composite_open();
	}
	else
	{
		ret = composite_close();
	}

	DPRINTF("%s ret[%d] \n",__func__, ret);
	return ret;
}

int composite_display_output_set(char onoff, char lcdc)
{
	char value[PROPERTY_VALUE_MAX];
	memset(value, NULL, PROPERTY_VALUE_MAX);
	
	if(onoff)
	{
		if (composite_start(lcdc) < 0) 		{
			ALOGE("fail to start composite output\n");
			return NULL;
		}

		if (composite_fb_set_mode(LCDC_COMPOSITE_UI_MODE) < 0)		{
			ALOGE("fail to set FB mode\n");
			return NULL;
		}

		value[0] = '1';
		property_set("persist.sys.composite_detected", value);
	}
	else
	{
		if (composite_fb_set_mode(LCDC_COMPOSITE_NONE_MODE) < 0)		{
			DPRINTF("fail to set FB mode\n");
			return NULL;
		}

		if (composite_end() < 0)		{
			DPRINTF("fail to end composite output\n");
			return NULL;
		}

		value[0] = '0';
 		property_set("persist.sys.composite_detected", value);
	}

	return NULL;
}

int composite_display_output_modeset(char enable)
{
	unsigned int OutputMode  = 0;

	if(enable)
		OutputMode = OUTPUT_COMPOSITE;
	else
		OutputMode = OUTPUT_NONE;
	
	DPRINTF("%s, OutputMode=%d", __func__, OutputMode);
	
	if(ioctl(fb_fd, TCC_LCDC_OUTPUT_MODE_SET, &OutputMode) != 0)
	{
		ALOGE("%s, can't set output mode", __func__);
		return -1;
	}

	return 1;	
}

int composite_display_output_attach(char onoff, char lcdc)
{
	TCC_COMPOSITE_START_TYPE composite_start;
    char value[PROPERTY_VALUE_MAX];

	DPRINTF("%s", __func__);
	
    property_get("persist.sys.composite_mode", value, "");

	if(atoi(value) == 0)	{
		composite_start.mode = COMPOSITE_NTST_M;
		DPRINTF("composite_start() : NTSC");
	}
	else	{
		composite_start.mode = COMPOSITE_PAL_M;
		DPRINTF("composite_start() : PAL");
	}
	
	composite_start.lcdc = lcdc;

	if(onoff)
	{
		if(ioctl(composite_fd, TCC_COMPOSITE_IOCTL_ATTACH, &composite_start) != 0)
		{
			ALOGE("can't attach composite mode");
			return -1;
		}
	}
	else
	{
		if(ioctl(composite_fd, TCC_COMPOSITE_IOCTL_DETACH, NULL) != 0)
		{
			ALOGE("can't detach composite mode");
			return -1;
		}
	}

	return 1;
}

int composite_display_cabledetect(void)
{
	int composite_detect = 0;

    if(ioctl(fb_fd, TCC_LCDC_COMPOSITE_CHECK, &composite_detect))
	{
        DPRINTF("%s failed!\n",__func__);
        return 0;
    }

	DPRINTF("%s, composite_detect=%d", __func__, composite_detect);
	
	return composite_detect;
}

