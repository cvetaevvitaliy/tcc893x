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

#include <libhpd/libhpd.h>
#include <libedid/libedid.h>
#include <libhdmi/libhdmi.h>

#include <cutils/properties.h>
#include <mach/tccfb_ioctrl.h>
#include <mach/tcc_component_ioctl.h>

#define FB_DEVICE				"/dev/graphics/fb0"
#define COMPONENT_DEVICE 		"/dev/component"

#define LOG_NDEBUG				0
#define LOG_TAG					"COMPONENT_APP"

#define COMPONENT_APP_DEBUG      0
#if COMPONENT_APP_DEBUG
#define DPRINTF(args...)    ALOGD(args)
#else
#define DPRINTF(args...)
#endif

static int component_fd = -1;
static int fb_fd = -1;

int component_fb_open(void)
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

int component_fb_close(void)
{
	DPRINTF("%s", __func__);
	
	// Close FB Device
	close(fb_fd);

	fb_fd = -1;

	return 1;	
}


unsigned int component_outputmode_check(void)
{
	unsigned int OutputMode  = 0;
	
	//DPRINTF("%s", __func__);
	
	if (ioctl(fb_fd, TCC_LCDC_OUTPUT_MODE_CHECK, &OutputMode) ) {
		DPRINTF("%s failed!\n",__func__);
		return 0;
	}
	
	return OutputMode;
}


int component_output_check(int *output_check)
{
	unsigned int OutputSelMode;
	OutputSelMode = component_outputmode_check();

	//DPRINTF("%s", __func__);
	
	if( OutputSelMode == OUTPUT_SELECT_NONE)
		*output_check = 1;
	else
		*output_check = 0;

	return 1;
}

int component_fb_set_mode(int mode)
{
	DPRINTF("%s, mode=%d", __func__, mode);
	
	if(ioctl(fb_fd, TCC_LCDC_COMPONENT_MODE_SET, &mode) != 0)
	{
		ALOGE("can't set component mode");
		return -1;
	}

	return 1;	
}

int component_open(void)
{
	DPRINTF("%s", __func__);
	
	if(component_fd > 0)
		return 1;

	// Open Component Device
	component_fd = open(COMPONENT_DEVICE, O_RDWR);
	if (component_fd <= 0) 
	{
		ALOGE("can't open component device '%s'", COMPONENT_DEVICE);
		return -1;
	}
	
	return 1;	
}

int component_close(void)
{
	DPRINTF("%s", __func__);
	
	// Close Component Device
	close(component_fd);

	component_fd = -1;

	return 1;	
}

int component_start(char lcdc)
{
	TCC_COMPONENT_START_TYPE component_start;
	unsigned int component_mode;
    char value[PROPERTY_VALUE_MAX];

	DPRINTF("%s", __func__);

	component_start.lcdc = lcdc;
	
	#if defined(COMPONENT_CHIP_CS4954)
		component_start.mode = COMPONENT_NTST_M;
	#else
		memset(value, NULL, PROPERTY_VALUE_MAX);
		property_get("persist.sys.component_mode", value, "");
		component_mode = atoi(value);

		if(component_mode == OUTPUT_COMPONENT_1080I)		
			component_start.mode = COMPONENT_1080I;
		else
			component_start.mode = COMPONENT_720P;
	#endif
	
	if(ioctl(component_fd, TCC_COMPONENT_IOCTL_START, &component_start) != 0)
	{
		ALOGE("can't start component mode");
		return -1;
	}

	return 1;
}

int component_end(void)
{
	DPRINTF("%s", __func__);
	
	if(ioctl(component_fd, TCC_COMPONENT_IOCTL_END, NULL) != 0)
	{
		ALOGE("can't end component mode");
		return -1;
	}

	return 1;
}

/********************************************************
TCC Component Control function 
 Component sequence
1. init
2. suspend_check
3. detect_onoff(1)
4. output_set(1)
 Component display state
5. output_set(0)
6. dectect_onfoff(0)
7. deinit
**********************************************************/

int component_display_init(void)
{
	if (component_fb_open() < 0) 
	{
		DPRINTF("fail to open FB device\n");
		return NULL;
	}
	return NULL;
}

int component_display_deinit()
{
	DPRINTF("%s", __func__);
	component_fb_close();
    return 0;
}

unsigned int component_lcdc_check(void)
{
    int component_check  = 0;

	//DPRINTF("%s", __func__);
    if (ioctl(fb_fd, TCC_LCDC_HDMI_CHECK, &component_check) ) {
        DPRINTF("%s TCC_LCDC_HDMI_CHECK failed!\n",__func__);
        return 0;
    }

    return component_check;
}

unsigned int component_suspend_check(void)
{
    int component_suspend  = 0;

	//DPRINTF("%s", __func__);
    if (ioctl(component_fd, TCC_COPONENT_IOCTL_GET_SUSPEND_STATUS, &component_suspend) ) {
        DPRINTF("%s TCC_COPONENT_IOCTL_GET_SUSPEND_STATUS failed!\n",__func__);
        return 0;
    }
    return component_suspend;
}

int component_display_detect_onoff(char onoff)
{
	int ret;
	if(onoff)
	{
		ret = component_open();
	}
	else
	{
		ret = component_close();
	}

	DPRINTF("%s ret[%d] \n",__func__, ret);
	return ret;
}

int component_display_output_set(char onoff, char lcdc)
{
	char value[PROPERTY_VALUE_MAX];
	memset(value, NULL, PROPERTY_VALUE_MAX);

	if(onoff)
	{
		if (component_start(lcdc) < 0) 		{
			DPRINTF("fail to start component output\n");
			return NULL;
		}

		if (component_fb_set_mode(LCDC_COMPONENT_UI_MODE) < 0)		{
			DPRINTF("fail to set FB mode\n");
			return NULL;
		}

		value[0] = '1';
		property_set("persist.sys.component_detected", value);
	}
	else
	{
		if (component_fb_set_mode(LCDC_COMPONENT_NONE_MODE) < 0)		{
			DPRINTF("fail to set FB mode\n");
			return NULL;
		}

		if (component_end() < 0)		{
			DPRINTF("fail to end component output\n");
			return NULL;
		}

		value[0] = '0';
		property_set("persist.sys.component_detected", value);
	}
	return NULL;

}

int component_display_output_modeset(char enable)
{
	unsigned int OutputMode  = 0;

	if(enable)
		OutputMode = OUTPUT_COMPONENT;
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

int component_display_cabledetect(void)
{
	int component_detect = 0;

    if(ioctl(fb_fd, TCC_LCDC_COMPONENT_CHECK, &component_detect))
	{
        DPRINTF("%s failed!\n",__func__);
        return 0;
    }

	DPRINTF("%s, component_detect=%d", __func__, component_detect);
	
	return component_detect;
}

