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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <utils/Log.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <linux/fb.h>

#include <cutils/properties.h>
#include <libhdmi/libhdmi.h>
#include <mach/tccfb_ioctrl.h>
#include <mach/tcc_composite_ioctl.h>
#include <mach/tcc_component_ioctl.h>
#include <mach/tcc_fts_ioctl.h>

#define FB0_DEVICE	"/dev/graphics/fb0"
#define FTS_DEVICE	"/dev/fts"

#define LOG_NDEBUG			0
#define EXTENDDISPLAY_DEBUG     	0
#define EXTENDDISPLAY_THREAD_DEBUG	0

#define LOG_TAG				"-EX_DISP-"
#if EXTENDDISPLAY_DEBUG
#define DPRINTF(args...)    ALOGD(args)
#else
#define DPRINTF(args...)
#endif

#if EXTENDDISPLAY_THREAD_DEBUG
#define TPRINTF(args...)    ALOGD(args)
#else
#define TPRINTF(args...)
#endif

static unsigned int output_auto_detection_flag = 0;
static unsigned int output_first_detection_flag = 0;
static unsigned int uiOutputResizeMode_up = 0, uiOutputResizeMode_down = 0, uiOutputResizeMode_left = 0, uiOutputResizeMode_right = 0;

static char output_resolution[16];
static unsigned int output_display_type;
static unsigned int output_display_mode;

#ifdef HAVE_HDMI_OUTPUT
extern int hdmi_display_init(char hdmi_reset);
extern int hdmi_display_deinit(void);
extern unsigned int hdmi_lcdc_check(void);
extern unsigned int hdmi_suspend_check(void);
extern int hdmi_display_detect_onoff(char onoff);
extern HDMIAudioPort hdmi_get_AudioInputPort(void);
ColorDepth hdmi_get_ColorDepth(void);
ColorSpace hdmi_get_ColorSpace(void);
PixelAspectRatio hdmi_get_PixelAspectRatio(void);
extern int hdmi_AudioInputPortDetect(void);
extern int hdmi_compare_resolution(int width, int height);
extern void hdmi_set_output_detected(unsigned int detected);

extern int hdmi_display_output_set(char onoff);
extern int hdmi_display_output_modeset(char enable);
extern int hdmi_display_cabledetect(void);
extern int hdmi_AudioOnOffChk(void);
extern int HDMI_GetVideoResolution(void);
extern int CECCmdProcess(void);
extern int hdmi_set_video_format_ctrl(unsigned int HdmiVideoFormat, unsigned int Structure_3D);
#endif//

#ifdef TCC_OUTPUT_COMPOSITE
extern int composite_display_init(void);
extern int composite_display_deinit(void);
extern unsigned int composite_lcdc_check(void);
extern unsigned int composite_suspend_check(void);
extern int composite_display_detect_onoff(char onoff);
extern int composite_display_output_set(char onoff, char lcdc);
extern int composite_display_output_modeset(char enable);
extern int composite_display_output_attach(char onoff, char lcdc);
extern int composite_display_cabledetect(void);
#endif//

#ifdef TCC_OUTPUT_COMPONENT
extern int component_display_init(void);
extern int component_display_deinit(void);
extern unsigned int component_lcdc_check(void);
extern unsigned int component_suspend_check(void);
extern int component_display_detect_onoff(char onoff);
extern int component_display_output_set(char onoff, char lcdc);
extern int component_display_output_modeset(char enable);
extern int component_display_cabledetect(void);
#endif//

static pthread_t extend_display_thread_id;

static int running = 0;
static int cable_detect = 0;

static int extendthreadready = 0;

struct FB {
    unsigned short *bits;
    unsigned size;
    int fd;
    struct fb_fix_screeninfo fi;
    struct fb_var_screeninfo vi;
};

struct FB fb;

static void sighandler(int sig) {
    running = 0;
}

static void BlockSignals(void)
{
    sigset_t signal_set;
    /* add all signals */
    sigfillset(&signal_set);
    /* set signal mask */
    pthread_sigmask(SIG_BLOCK, &signal_set, 0);
}

typedef struct{
	unsigned int OutMode;
	unsigned int SetMode; // 1080p / 720p : ntsc/pal
	unsigned int SubMode; // HDMI or DVI
	unsigned int CECMode;
	unsigned int AspectRatio;
	unsigned int ColorDepth;
	unsigned int ColorSpace;
}extend_display_mode;
extend_display_mode CurrentMode;

static extend_display_mode OutputAttachMode;
static unsigned int output_auto_detection = 0;
static unsigned int output_attach_dual = 0;
static unsigned int output_attach_dual_auto = 0;
static unsigned int output_attach_hdmi_cvbs = 0;

static int fb_open(struct FB *fb)
{
	fb->fd = open(FB0_DEVICE, O_RDWR);

    if (ioctl(fb->fd, FBIOGET_FSCREENINFO, &fb->fi) < 0)
        DPRINTF("%s: FBIOGET_FSCREENINFO failed!\n",__func__);
    if (ioctl(fb->fd, FBIOGET_VSCREENINFO, &fb->vi) < 0)
        DPRINTF("%s: FBIOGET_VSCREENINFO failed!\n",__func__);

	return 0;

}

static void fb_update(struct FB *fb)
{
	// Get FB information
	if (ioctl(fb->fd, FBIOGET_VSCREENINFO, &fb->vi) < 0)
	{
		DPRINTF("%s: FBIOGET_VSCREENINFO failed!\n",__func__);
	}

	fb->vi.activate = FB_ACTIVATE_FORCE;

    if (ioctl(fb->fd, FBIOPUT_VSCREENINFO, &fb->vi) < 0)
    {
		DPRINTF("%s: FBIOPUT_VSCREENINFO failed!\n",__func__);
    }
}

int extend_display_SuspendCheck(unsigned int OutMode)
{
	switch(OutMode)
	{
		#ifdef HAVE_HDMI_OUTPUT
		case OUTPUT_HDMI:
			return hdmi_suspend_check();
		#endif//
		
		#ifdef TCC_OUTPUT_COMPOSITE
		case OUTPUT_COMPOSITE:			
			return composite_suspend_check();
		#endif//

		#ifdef TCC_OUTPUT_COMPONENT
		case OUTPUT_COMPONENT:
			return component_suspend_check();

		#endif//
		default:
			break;
	}
	return 0;
}



int extend_display_check(extend_display_mode *OutMode)
{
	char value[PROPERTY_VALUE_MAX];
	unsigned int OutputMode = 0, uiLcdcCheck = 0, ifixed = 0, uiSuspendCheck = 0;
	int run_check = 1;
	tcc_display_resize OutputResizeMode;
	unsigned int cec_mode;
	extend_display_mode check_output;

	check_output.OutMode = 0;
	check_output.SetMode = 0;	
	check_output.SubMode = 0;
	check_output.CECMode = 0;
	check_output.AspectRatio = 0;
	check_output.ColorSpace = 0;
	check_output.ColorDepth = 0;
	// Read UI setting value
	memset(value, NULL, PROPERTY_VALUE_MAX);

	OutputResizeMode.resize_up = 0;
	OutputResizeMode.resize_down = 0;
	OutputResizeMode.resize_left = 0;
	OutputResizeMode.resize_right = 0;

	property_get("persist.sys.extenddisplay_reset", value, "");
	if(atoi(value))
	{
		property_set("persist.sys.extenddisplay_reset", "0");
		return 0;
	}
	property_get("persist.sys.output_mode", value, "");
	OutputMode = atoi(value);

	#ifdef HAVE_HDMI_OUTPUT
	#if defined(DISPLAY_OUTPUT_STB)
	if((OutputMode == OUTPUT_HDMI) || (OutputMode == OUTPUT_NONE))
	#else
	if(OutputMode == OUTPUT_HDMI)
	#endif
	{
		memset(value, NULL, PROPERTY_VALUE_MAX);

		check_output.SetMode = HDMI_GetVideoResolution();

		#if 0
			property_get("persist.sys.hdmi_auto_select", value, "");
		#else
			property_get("persist.sys.hdmi_mode", value, "");
		#endif
		check_output.SubMode = atoi(value);

		uiLcdcCheck = hdmi_lcdc_check();

		property_get("persist.sys.hdmi_resize_up", value, "");
		OutputResizeMode.resize_up = atoi(value);
		property_get("persist.sys.hdmi_resize_dn", value, "");
		OutputResizeMode.resize_down = atoi(value);
		property_get("persist.sys.hdmi_resize_lt", value, "");
		OutputResizeMode.resize_left = atoi(value);
		property_get("persist.sys.hdmi_resize_rt", value, "");
		OutputResizeMode.resize_right = atoi(value);

		#if defined(DISPLAY_OUTPUT_STB)
		property_get("persist.sys.hdmi_cec", value, "");
                cec_mode = atoi(value);
		value[0] = '0';
		check_output.CECMode = cec_mode;
		if( (CurrentMode.OutMode != OutputMode) || (CurrentMode.CECMode != cec_mode))
		{
			memset(value, NULL, PROPERTY_VALUE_MAX);
			value[0] = '1'; // HDMI
			property_set("persist.sys.output_mode", value);

            if(cec_mode == 0) {
                    check_output.CECMode = 0;
            }else if(cec_mode == 1){
                    check_output.CECMode = 1;
            }
		}
		#endif

		{
			#if defined(DISPLAY_OUTPUT_STB)
				check_output.AspectRatio = hdmi_get_PixelAspectRatio();
				check_output.ColorDepth = hdmi_get_ColorDepth();
				check_output.ColorSpace = hdmi_get_ColorSpace();
			#else
				check_output.AspectRatio = 0;
				check_output.ColorDepth = 0;
				check_output.ColorSpace = 0;
			#endif
		}

		check_output.OutMode = OUTPUT_HDMI;
		
	}
	else 
	#endif//
	#ifdef TCC_OUTPUT_COMPOSITE
	if(OutputMode == OUTPUT_COMPOSITE)
	{
		check_output.OutMode = OUTPUT_COMPOSITE;

		property_get("persist.sys.composite_mode", value, "");
		check_output.SetMode = atoi(value);
		check_output.SubMode = 0;

		uiLcdcCheck = composite_lcdc_check();	

		property_get("persist.sys.composite_resize_up", value, "");
		OutputResizeMode.resize_up = atoi(value);
		property_get("persist.sys.composite_resize_dn", value, "");
		OutputResizeMode.resize_down = atoi(value);
		property_get("persist.sys.composite_resize_lt", value, "");
		OutputResizeMode.resize_left = atoi(value);
		property_get("persist.sys.composite_resize_rt", value, "");
		OutputResizeMode.resize_right = atoi(value);
	}
	else
	#endif//
	#ifdef TCC_OUTPUT_COMPONENT
	if(OutputMode == OUTPUT_COMPONENT)
	{
		check_output.OutMode = OUTPUT_COMPONENT;

		property_get("persist.sys.component_mode", value, "");
		check_output.SetMode = atoi(value);		
		check_output.SubMode = 0;

		uiLcdcCheck = component_lcdc_check ();

		property_get("persist.sys.component_resize_up", value, "");
		OutputResizeMode.resize_up = atoi(value);
		property_get("persist.sys.component_resize_dn", value, "");
		OutputResizeMode.resize_down = atoi(value);
		property_get("persist.sys.component_resize_lt", value, "");
		OutputResizeMode.resize_left = atoi(value);
		property_get("persist.sys.component_resize_rt", value, "");
		OutputResizeMode.resize_right = atoi(value);
	}
	else
	#endif//
	{
		check_output.OutMode = 0;
		check_output.SetMode = 0;	
		check_output.SubMode = 0;
		check_output.CECMode = 0;
		check_output.AspectRatio = 0;
		check_output.ColorDepth = 0;
		check_output.ColorSpace = 0;
	}

	if( (OutputMode != OUTPUT_NONE) && 
		(uiOutputResizeMode_up != OutputResizeMode.resize_up || uiOutputResizeMode_down != OutputResizeMode.resize_down || 
		uiOutputResizeMode_left != OutputResizeMode.resize_left || uiOutputResizeMode_right != OutputResizeMode.resize_right) )
	{
		DPRINTF("[Previous] OutputResizeMode_up = %d, OutputResizeMode_down = %d, OutputResizeMode_left = %d, OutputResizeMode_right = %d [ N e w  ] OutputResizeMode_up = %d, OutputResizeMode_down = %d, OutputResizeMode_left = %d, OutputResizeMode_right = %d",
				uiOutputResizeMode_up, uiOutputResizeMode_down, uiOutputResizeMode_left, uiOutputResizeMode_right, OutputResizeMode.resize_up, OutputResizeMode.resize_down, OutputResizeMode.resize_left, OutputResizeMode.resize_right);

		if(ioctl(fb.fd, TCC_LCDC_SET_OUTPUT_RESIZE_MODE, &OutputResizeMode)) 
		{
			DPRINTF("%s: TCC_LCDC_SET_OUTPUT_OFFSET failed!\n",__func__);
			return 0;
		}

		//if(output_first_detection_flag)
		//	fb_update(&fb);

		uiOutputResizeMode_up = OutputResizeMode.resize_up;
		uiOutputResizeMode_down = OutputResizeMode.resize_down;
		uiOutputResizeMode_left = OutputResizeMode.resize_left;
		uiOutputResizeMode_right = OutputResizeMode.resize_right;
	}

	//DPRINTF("~ %s %d Output:OUT:%d Set:%d suspend:%d ~ \n",__func__, run_check, check_output.OutMode, check_output.SetMode, uiSuspend);

	OutMode->OutMode = check_output.OutMode;
	OutMode->SetMode = check_output.SetMode;
	OutMode->SubMode = check_output.SubMode;
	OutMode->CECMode = check_output.CECMode;
	OutMode->AspectRatio = check_output.AspectRatio;
	OutMode->ColorDepth = check_output.ColorDepth;
	OutMode->ColorSpace = check_output.ColorSpace;

	uiSuspendCheck = extend_display_SuspendCheck(OutMode->OutMode);

	if(check_output.OutMode && (!uiLcdcCheck) && (!uiSuspendCheck))
		run_check = 1;
	else
		run_check = 0;
	

	return run_check;
}

void extend_display_device_init()
{
	#ifdef HAVE_HDMI_OUTPUT
		hdmi_display_init(1);
	#endif//

	#ifdef TCC_OUTPUT_COMPOSITE
	composite_display_init();
	#endif//
	
	#ifdef TCC_OUTPUT_COMPONENT
	component_display_init();
	#endif//

	fb_open(&fb);
	
}

void extend_display_device_deinit()
{
	#ifdef HAVE_HDMI_OUTPUT
	hdmi_display_deinit();
	#endif//

	#ifdef TCC_OUTPUT_COMPOSITE
	composite_display_deinit();
	#endif//
	
	#ifdef TCC_OUTPUT_COMPONENT
	component_display_deinit();
	#endif//

	close(fb.fd);
}

int extend_display_detect_OnOff(char onoff, unsigned int OutMode)
{
	switch(OutMode)
	{
		#ifdef HAVE_HDMI_OUTPUT
		case OUTPUT_HDMI:
			return hdmi_display_detect_onoff(onoff);
		#endif//
		
		#ifdef TCC_OUTPUT_COMPOSITE
		case OUTPUT_COMPOSITE:			
			return composite_display_detect_onoff(onoff);
		#endif//
		
		#ifdef TCC_OUTPUT_COMPONENT
		case OUTPUT_COMPONENT:
			return component_display_detect_onoff(onoff);
		#endif//

		default:
			break;
	}
	return 0;
}

int extend_display_output_set(unsigned int OutMode, char onoff)
{
	char lcdc_composite;
	char lcdc_component;

	if(onoff)
		property_set("tcc.sys.output_mode_detected", "1");
	else
		property_set("tcc.sys.output_mode_detected", "0");
			
	#if defined(DISPLAY_OUTPUT_STB)
		if(output_auto_detection || output_attach_dual_auto || output_attach_hdmi_cvbs)
		{
			if(onoff == 0)
			{
				property_set("tcc.sys.output_mode_plugout", "1");
				usleep(100000);
			}

		}
	#endif

#if defined(_TCC8800_) || defined(_TCC9300_) || defined(TCC892X)
	#if defined(DISPLAY_OUTPUT_STB)
		lcdc_composite = COMPOSITE_LCDC_1;
		lcdc_component = COMPONENT_LCDC_1;
	#else
		lcdc_composite = COMPOSITE_LCDC_0;
		lcdc_component = COMPONENT_LCDC_0;
	#endif
#elif defined(TCC893X)
	lcdc_composite = COMPOSITE_LCDC_0;
	lcdc_component = COMPONENT_LCDC_0;
#else
	lcdc_composite = COMPOSITE_LCDC_1;
	lcdc_component = COMPONENT_LCDC_1;
#endif

	#if defined(DISPLAY_OUTPUT_STB)
		if((output_attach_dual || output_attach_dual_auto) && onoff)
		{
			char value[PROPERTY_VALUE_MAX];

			memset(value, NULL, PROPERTY_VALUE_MAX);
			
			if(OutMode == OUTPUT_COMPOSITE)
			{
				value[0] = '0' + OUTPUT_COMPOSITE; 
				property_set("persist.sys.output_attach_sub", value);
			}
			else
			{
				value[0] = '0' + OutMode; 
				property_set("persist.sys.output_attach_main", value);
				value[0] = '0' + OUTPUT_COMPOSITE; 
				property_set("persist.sys.output_attach_sub", value);
			}
		}
	#endif
	
	switch(OutMode)
	{
		#ifdef HAVE_HDMI_OUTPUT
		case OUTPUT_HDMI:
			#if defined(DISPLAY_OUTPUT_STB)
				if(output_attach_dual || output_attach_dual_auto || output_attach_hdmi_cvbs)
				{
					hdmi_display_output_set(onoff);

					composite_display_detect_onoff(1);
					composite_display_output_attach(onoff, lcdc_composite);
					composite_display_detect_onoff(0);
					return 0;
				}
 				else
			#endif
					return hdmi_display_output_set(onoff);
		#endif
		
		#ifdef TCC_OUTPUT_COMPOSITE
		case OUTPUT_COMPOSITE:			
			#if defined(DISPLAY_OUTPUT_STB)
				if(output_attach_dual_auto)
					return composite_display_output_attach(onoff, lcdc_composite);
				else
			#endif
					return composite_display_output_set(onoff, lcdc_composite);
		#endif
		
		#ifdef TCC_OUTPUT_COMPONENT
		case OUTPUT_COMPONENT:
			#if defined(DISPLAY_OUTPUT_STB)
				if(output_attach_dual || output_attach_dual_auto)
				{
 					component_display_output_set(onoff, lcdc_component);

					composite_display_detect_onoff(1);
					composite_display_output_attach(onoff, lcdc_composite);
					composite_display_detect_onoff(0);
					return 0;
				}
 				else
			#endif
					return component_display_output_set(onoff, lcdc_component);
		#endif

		default:
			break;
	}

	return 0;
}

int extend_display_AudioInputPortDetect(unsigned int OutMode)
{
	switch(OutMode)
	{
		#ifdef HAVE_HDMI_OUTPUT
		case OUTPUT_HDMI:
			return hdmi_AudioInputPortDetect();
		#endif//
		
		#ifdef TCC_OUTPUT_COMPOSITE
		case OUTPUT_COMPOSITE:			
			return 1;
		#endif//

		#ifdef TCC_OUTPUT_COMPONENT
		case OUTPUT_COMPONENT:
			return 1;


		#endif//
		default:
			break;
	}
	return 0;
}

int extend_display_VideoFormatCtrl(void)
{
	unsigned int HdmiVideoFormat, Structure_3D, hdmi_mode;
	char value[PROPERTY_VALUE_MAX];

	memset(value, NULL, PROPERTY_VALUE_MAX);

	if(CurrentMode.OutMode == OUTPUT_HDMI)
	{
	    property_get("persist.sys.hdmi_mode", value, "");
	    hdmi_mode = atoi(value);

	    if(hdmi_mode == 0)
		{
			property_get("tcc.output.hdmi.video.format", value, "");
			HdmiVideoFormat = atoi(value);
			property_get("tcc.output.hdmi.structure.3d", value, "");
			Structure_3D = atoi(value);

			return hdmi_set_video_format_ctrl(HdmiVideoFormat, Structure_3D);
	    }
	}
					
	return 0;
}

int extend_display_CableDetect(unsigned int OutMode)
{
	switch(OutMode)
	{
		#ifdef HAVE_HDMI_OUTPUT
		case OUTPUT_HDMI:
			return hdmi_display_cabledetect();
		#endif//
		
		#ifdef TCC_OUTPUT_COMPOSITE
		case OUTPUT_COMPOSITE:			
			return composite_display_cabledetect();
		#endif//

		#ifdef TCC_OUTPUT_COMPONENT
		case OUTPUT_COMPONENT:
			return component_display_cabledetect();
		#endif//
		default:
			break;
	}
	return 0;
}

int extend_display_AutoDetect(void)
{
	extend_display_mode OutputMode;
	char value[PROPERTY_VALUE_MAX];
	int iOutput;

	/* check the cable detection of current output mode */
	if(extend_display_CableDetect(CurrentMode.OutMode))
		return 1;

	/* release the cable detection of currnet output mode */
	extend_display_detect_OnOff(0, CurrentMode.OutMode);

	/* check the cable detection */
	for(iOutput=OUTPUT_HDMI; iOutput<OUTPUT_MAX; iOutput++)
	{
		#if defined(DISPLAY_OUTPUT_STB)
			if(output_attach_dual_auto && (iOutput == OUTPUT_COMPOSITE))
				continue;
		#endif

		extend_display_detect_OnOff(1, iOutput);
		
		if(extend_display_CableDetect(iOutput))
		{
			TPRINTF("Old OUTPUT:%d, New OUTPUT:%d Auto-Detection!!\n", CurrentMode.OutMode, iOutput);

			if(iOutput != CurrentMode.OutMode)
			{
				memset(value, NULL, PROPERTY_VALUE_MAX);
				value[0] = '0' + iOutput;
				property_set("persist.sys.output_mode", value);
			}

			break;
		}

		extend_display_detect_OnOff(0, iOutput);
	}
		
	if(iOutput == OUTPUT_MAX)
	{
		/* open the cable detection of currnet output mode */
		extend_display_detect_OnOff(1, CurrentMode.OutMode);
	}
	else
	{
		/* set the new output mode */
		extend_display_check(&OutputMode);
		CurrentMode = OutputMode;
		return 1;
	}

	return 0;
}

int extend_display_CheckDetect(extend_display_mode *OutputMode)
{
	int cable_detect = 0;

	if(output_auto_detection || output_attach_dual_auto || output_attach_hdmi_cvbs)
	{
		if(output_auto_detection_flag)
		{
			cable_detect = extend_display_AutoDetect();

			if(cable_detect)
				extend_display_check(OutputMode);
		}
		else
		{
			cable_detect = extend_display_CableDetect(CurrentMode.OutMode);
		}
	}
	else
	{
		cable_detect = extend_display_CableDetect(CurrentMode.OutMode);
	}

	return cable_detect;
}

int extend_display_InitFirstOutput(void)
{
	int cable_detect = 0;

	/* reset component output after displaying dual output in bootloader/kernel */
	if(output_first_detection_flag == 0)
	{
		if(CurrentMode.OutMode == OUTPUT_HDMI)
		{
			cable_detect = extend_display_CableDetect(CurrentMode.OutMode);
			
			if( (cable_detect == 0) && (output_display_type == 0) && (output_auto_detection == 0) && 
				(output_attach_dual == 0) && (output_attach_dual_auto == 0) && (output_attach_hdmi_cvbs == 0) )
			{
				extend_display_detect_OnOff(1, OUTPUT_COMPONENT);
				extend_display_output_set(OUTPUT_COMPONENT, 0);
				extend_display_detect_OnOff(0, OUTPUT_COMPONENT);
			}

			return 1;
		}
	}

	return 0;
}

int extend_display_SetAttachOutput(void)
{
	char value[PROPERTY_VALUE_MAX];

	memset(value, NULL, PROPERTY_VALUE_MAX);

	property_get("persist.sys.composite_mode", value, "");

	if( (output_attach_hdmi_cvbs && OutputAttachMode.SetMode != atoi(value) && CurrentMode.OutMode == OUTPUT_HDMI) || 
		(output_attach_dual_auto && OutputAttachMode.SetMode != atoi(value)) )
	{
		extend_display_detect_OnOff(1, OUTPUT_COMPOSITE);
		extend_display_output_set(OUTPUT_COMPOSITE, 1);
		extend_display_detect_OnOff(0, OUTPUT_COMPOSITE);

		OutputAttachMode.SetMode = atoi(value);
		running = 0;

		return 1;
	}

	return 0;
}

int extend_display_SetDisplayMode(void)
{
	char value[PROPERTY_VALUE_MAX];

	memset(value, NULL, PROPERTY_VALUE_MAX);
	
	/* get display type - 0: HDMI/CVBS/COMPONENT, 1: HDMI/CVBS, 2: HDMI */
	if(ioctl(fb.fd, TCC_LCDC_GET_DISPLAY_TYPE, &output_display_type) >= 0) 
	{
		/* set property for display type */
		value[0] = '0' + output_display_type;
		property_set("tcc.output.display.type", value);
	}

	output_attach_dual = 0;
	output_attach_dual_auto = 0;
	output_attach_hdmi_cvbs = 0;

	property_get("persist.sys.display.mode", value, "");
	output_display_mode = atoi(value);

	if(output_display_mode == 1)
		output_auto_detection = 1;
	else if(output_display_mode == 2)
		output_attach_hdmi_cvbs = 1;
	else if(output_display_mode == 3)
		output_attach_dual_auto = 1;

	if(output_attach_dual || output_attach_dual_auto)
	{
		OutputAttachMode.OutMode = OUTPUT_COMPOSITE;
		OutputAttachMode.SetMode = 0;
		OutputAttachMode.SubMode = 0;
		OutputAttachMode.CECMode = 0;
		OutputAttachMode.AspectRatio = 0;
		OutputAttachMode.ColorDepth = 0;
		OutputAttachMode.ColorSpace = 0;
		
		output_auto_detection_flag = 1;
	}
	else if(output_attach_hdmi_cvbs)
	{
		property_get("persist.sys.composite_mode", value, "");

		OutputAttachMode.OutMode = OUTPUT_COMPOSITE;
		OutputAttachMode.SetMode = atoi(value);
		OutputAttachMode.SubMode = 0;
		OutputAttachMode.CECMode = 0;
		OutputAttachMode.AspectRatio = 0;
		OutputAttachMode.ColorDepth = 0;
		OutputAttachMode.ColorSpace = 0;
	}
		
	return 0;
}

int extend_display_SaveData(char *output_data, char data_size)
{
	int fts_fd, res;
	char data[512];
	char value[PROPERTY_VALUE_MAX];

	memset(value, NULL, PROPERTY_VALUE_MAX);

	output_resolution[0] = 'T';
	output_resolution[1] = 'C';
	output_resolution[2] = 'C';
	output_resolution[3] = CurrentMode.OutMode;
	output_resolution[4] = CurrentMode.SetMode;
	property_get("persist.sys.hdmi_resolution", value, "");
	output_resolution[5] = atoi(value);
	if(output_resolution[5] == 125)
	{
		property_get("persist.sys.hdmi_detected_res", value, "");
		output_resolution[5] = atoi(value);
	}
	property_get("persist.sys.composite_mode", value, "");
	output_resolution[6] = atoi(value);
	property_get("persist.sys.component_mode", value, "");
	output_resolution[7] = atoi(value);
	output_resolution[8] = 0;//outputmode_1st;
	output_resolution[9] = 0;//outputmode_2nd;
	property_get("persist.sys.hdmi_mode", value, "");
	output_resolution[10] = atoi(value);
					
	memset(data, 0x00, sizeof(data));
	memcpy(data, output_resolution, sizeof(output_resolution));

	fts_fd = open(FTS_DEVICE, O_RDWR);
	if (fts_fd < 0)
		return -1;

	if (ioctl(fts_fd, OUTPUT_SETTING_SET, data))
		res = -1;
	else
		res = 0;

	close(fts_fd);

	return 0;
}

char extend_display_runing(extend_display_mode OutPutMode)
{
	#if defined(DISPLAY_OUTPUT_STB)
		if(output_attach_dual_auto && (OutPutMode.OutMode == OUTPUT_COMPOSITE))
			return 1;	
		else
		{
			if((OutPutMode.OutMode == CurrentMode.OutMode))
				return 1;	
			else
				return 0;
		}
	#else
		if((OutPutMode.OutMode == CurrentMode.OutMode))
			return 1;	
		else
			return 0;
	#endif
}

static void* extend_display_thread(void* arg)
{
	int cable_detect = 0;
	extend_display_mode OutputMode;

#if defined(DISPLAY_OUTPUT_STB)
	char value[PROPERTY_VALUE_MAX];

	memset(value, NULL, PROPERTY_VALUE_MAX);
#endif

	hdmi_set_output_detected(false);

	running = extend_display_check(&OutputMode);
	CurrentMode = OutputMode;

	TPRINTF("%s Set_Mode:%d Cur_Mode:%d cable:%d running:%d ~~ \n",
				__func__, CurrentMode.OutMode, OutputMode.OutMode, cable_detect, running);

	#if defined(DISPLAY_OUTPUT_STB)
		extend_display_SetDisplayMode();
	#endif

	while(1)
	{
		// open to cable detect
		extend_display_detect_OnOff(1, OutputMode.OutMode);

		while(!running) {
			usleep(100000);
			running = extend_display_check(&OutputMode);
		}

		cable_detect = 0;
		CurrentMode = OutputMode;

		TPRINTF("open to cable detect CurrentMode:%d OutputMode:%d cable:%d \n", CurrentMode.OutMode, OutputMode.OutMode, cable_detect);

		#if defined(DISPLAY_OUTPUT_STB)
			extend_display_InitFirstOutput();
		#endif

		#if defined(DISPLAY_OUTPUT_STB)
			extendthreadready = 1;
		#endif

		while(running && extend_display_runing(OutputMode))
		{
			TPRINTF("extend display start Mode:%d Mode:%d cable:%d \n", CurrentMode.OutMode, OutputMode.OutMode, cable_detect);

			while((!cable_detect) && extend_display_runing(OutputMode) && running)
			{
				usleep(100000);
				running = extend_display_check(&OutputMode);	

			#if defined(DISPLAY_OUTPUT_STB)
				cable_detect = extend_display_CheckDetect(&OutputMode);
			#else
				cable_detect = extend_display_CableDetect(CurrentMode.OutMode);
			#endif
			}

			CurrentMode.SetMode = OutputMode.SetMode;
			CurrentMode.SubMode = OutputMode.SubMode;
			//CurrentMode.CECMode = OutputMode.CECMode;

			if(output_first_detection_flag == 0)
				output_first_detection_flag = 1;
					
			TPRINTF("run start Mode:%d Mode:%d cable:%d\n", CurrentMode.OutMode, OutputMode.OutMode, cable_detect);

			if(running && extend_display_runing(OutputMode))
			{
				DPRINTF("output_setting Mode:%d Mode:%d cable:%d\n", CurrentMode.OutMode, OutputMode.OutMode, cable_detect);

				extend_display_output_set(CurrentMode.OutMode, 1);

				#if defined(DISPLAY_OUTPUT_STB)
					//extend_display_SaveData(output_resolution, 11);
				#endif
			}

			TPRINTF("ready plug out Mode:%d Mode:%d cable:%d\n", CurrentMode.OutMode, OutputMode.OutMode, cable_detect);

			while(running && (cable_detect) && extend_display_runing(OutputMode))
			{
				usleep(100000);
				running = extend_display_check(&OutputMode);	
				cable_detect = extend_display_CableDetect(CurrentMode.OutMode);
				extend_display_AudioInputPortDetect(CurrentMode.OutMode);
				
			#if defined(DISPLAY_OUTPUT_STB)
				if(!extend_display_SetAttachOutput())
			#endif
				{
					if( (CurrentMode.OutMode != OutputMode.OutMode) ||
					    (CurrentMode.SubMode != OutputMode.SubMode) ||
					    (CurrentMode.SetMode != OutputMode.SetMode) ||
					    (CurrentMode.CECMode != OutputMode.CECMode) || 
					    (CurrentMode.AspectRatio != OutputMode.AspectRatio) || 
					    (CurrentMode.ColorDepth != OutputMode.ColorDepth) || 
					    (CurrentMode.ColorSpace != OutputMode.ColorSpace) )
					{
						running = 0;
					}

					if( !cable_detect )
						running = 0;
				}
				
			#if defined(DISPLAY_OUTPUT_STB)
				extend_display_VideoFormatCtrl();
			#endif
				
				//hdmi_AudioOnOffChk();
				CECCmdProcess();
			}

			TPRINTF("FINISH option run:%d cable:%d output%d~~~~~~~~~~\n", running , cable_detect , extend_display_runing(OutputMode));

			extend_display_output_set(CurrentMode.OutMode, 0);

			#if defined(DISPLAY_OUTPUT_STB)
				if(output_auto_detection_flag == 0)
					output_auto_detection_flag = 1;
			#endif /* DISPLAY_OUTPUT_STB */
		}

		// release to cable detect		
		extend_display_detect_OnOff(0, CurrentMode.OutMode);
		
		usleep(500000);
	}

	TPRINTF("~~~~~~ %s Mode:%d Mode:%d cable:%d FINISH~~~~~~~~~~\n", __func__, CurrentMode.OutMode, OutputMode.OutMode, cable_detect);
	return 0;
}

int extend_display_init()
{
	TPRINTF("~~~~~~~ extend_display_init");
#if defined(HAVE_HDMI_OUTPUT) || defined(TCC_OUTPUT_COMPONENT) || defined(TCC_OUTPUT_COMPOSITE)
//	signal(SIGINT, sighandler);

	extend_display_device_init();
	
#if defined(DISPLAY_OUTPUT_STB)
	extendthreadready = 0;
#endif /* DISPLAY_OUTPUT_STB */
	property_set("tcc.sys.output_mode_detected", "0");
	
	if (pthread_create(&extend_display_thread_id, NULL, &extend_display_thread, NULL))	{
		return -1;
	}

#if defined(DISPLAY_OUTPUT_STB)
	while(!extendthreadready)
		usleep(1000);
#endif /* DISPLAY_OUTPUT_STB */

	property_set("persist.sys.extenddisplay_reset", "0");

#endif//
    return 0;
}

int extend_display_deinit()
{
	running = 0;
	#if defined(HAVE_HDMI_OUTPUT) || defined(TCC_OUTPUT_COMPONENT) || defined(TCC_OUTPUT_COMPOSITE)
	extend_display_device_deinit();
	#endif//
	TPRINTF("~~~~~~~ %s finish", __func__);
    return 0;
}
