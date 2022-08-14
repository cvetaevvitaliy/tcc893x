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

#ifndef _LIBHDMI_H_
#define _LIBHDMI_H_


#if defined(HDMI_V1_3)
#include <mach/hdmi_1_3_video.h>
#include <mach/hdmi_1_3_audio.h>
#include <mach/hdmi_1_3_hdmi.h>
#else
#include <mach/hdmi_1_4_video.h>
#include <mach/hdmi_1_4_audio.h>
#include <mach/hdmi_1_4_hdmi.h>
#endif//

#ifdef __cplusplus
extern "C" {
#endif

// basic
int HDMIOpen(void);
int HDMIClose(void);

// video & audio
int HDMIPhyConfig(const struct HDMIVideoParameter * const hdmi_video_mode);
int HDMIDefaultPhyConfig(void);
int HDMISetHDMIMode(const enum HDMIMode mode);
int HDMISetVideoMode(const struct HDMIVideoParameter * const hdmi_video_mode);
int HDMISetAudioMode(const struct HDMIAudioParameter * const hdmi_audio_mode);
int HDMISetBlueScreen(const unsigned char enable);
int HDMISetPixelLimit(const enum PixelLimit mode);
int HDMISetSpeakerAllocation(const unsigned int speaker);
int HDMIGetPowerStatus(unsigned int *pwr_status);
int HDMISetPowerControl(unsigned int pwr_cmd);
int HDMISet5vPowerControl(unsigned int pwr_cmd);
int HDMIGetVideoMode(struct HDMIVideoParameter *hdmi_video_mode);
int HDMIGetAudioMode(struct HDMIAudioParameter *hdmi_audio_mode);
int HDMIGetRunStatus(void);
int HDMISetVideoFormatCtrl(unsigned int HDMIVideoFormat, unsigned int Structure_3D);

// control
int HDMIStart(void);
int HDMIStop(void);
int HDMIAudioStart(void);
int HDMIAudioStop(void);
int HDMISetAudioEnable(void);
int HDMISetAVMute(const unsigned char enable);
//pjj

//TCC8925S port change function 
int HDMIPortChangeOn(void);
int HDMIPortChangeGetState(unsigned int *port_change_state);
//MKC



/** framebuffer device file descriptor */
;
int HDMIFBOpen(void);

int HDMIFBClose(void);

int HDMILcdcConfig(enum VideoFormat hdmi_video_format);
unsigned int HDMI_LcdcCheck(void);
unsigned int HDMI_SuspendCheck(void);
unsigned int HDMI_OutputModeCheck(void);

int HDMI_lcdc_stop(void);

void HDMIGetLCDSize(enum VideoFormat videofmt ,unsigned int *width, unsigned int *height);


#ifdef __cplusplus
}
#endif

#endif
