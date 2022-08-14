/*
 * Copyright (C) 2013 Telechips, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_NDEBUG 0
#define LOG_TAG	"DVB_MANAGER_VIDEO"
#include <utils/Log.h>

#include <fcntl.h>

#include "tcc_dxb_interface_video.h" 
#include "tcc_iptv_manager_video.h" 

void tcc_video_event(DxB_VIDEO_NOTIFY_EVT nEvent, void  *pEventData)
{
    ALOGD("%s:[%d][%p]", __func__, nEvent, pEventData); 
    switch(nEvent)
    {
        case DxB_VIDEO_NOTIFY_FIRSTFRAME_DISPLAYED:
		{
			TCCDxBEvent_FirstFrameDisplayed(NULL);
        	}
            break;
        case DxB_VIDEO_NOTIFY_CAPTURE_DONE:
            break;
        default:
            break;
    }    
}

int tcc_manager_video_init(void)
{
    TCC_DxB_VIDEO_Init();
    TCC_DxB_VIDEO_RegisterEventCallback(0, tcc_video_event);
	return 0;
}

int tcc_manager_video_start(unsigned int ulVideoFormat)
{
	return TCC_DxB_VIDEO_Start(0, ulVideoFormat);
}

int tcc_manager_video_stop(unsigned int uidevid)
{
	return TCC_DxB_VIDEO_Stop(uidevid);
}

int tcc_manager_video_pause(unsigned int uOnOff)
{
	return 0;
}
int tcc_manager_video_capture(char *pucFileName)
{
    TCC_DxB_VIDEO_StartCapture(0, 0, 0, pucFileName);
    return 0;
}

int tcc_manager_video_GetVideoInfo(int *video_width, int *video_height)
{
	TCC_DxB_VIDEO_GetVideoInfo(0, video_width, video_height);
	return 0;
}

int tcc_manager_video_setVpuReset(void)
{	
	if( TCC_DxB_VIDEO_SetVpuReset(0) != DxB_ERR_OK)
		return 1;
	return 0;
}

int tcc_manager_video_setactivemode(int activemode)
{	
	if( TCC_DxB_VIDEO_SetActiveMode(0, activemode) != DxB_ERR_OK)
		return 1;
	return 0;
}

int tcc_manager_video_setPause(int pause)
{	
	if( TCC_DxB_VIDEO_SetPause(0, pause) != DxB_ERR_OK)
		return 1;
	return 0;
}

int tcc_manager_video_iFrameSearchEnable(void)
{	
	if( TCC_DxB_VIDEO_IFrameSearchEnable(0) != DxB_ERR_OK)
		return 1;
	return 0;
}

int tcc_manager_video_RefreshDisplay(void)
{	
	if( TCC_DxB_VIDEO_RefreshDisplay(0) != DxB_ERR_OK)
		return 1;
	return 0;
}


int tcc_manager_video_setSinkBypass(int sink)
{	
	if( TCC_DxB_VIDEO_SetSinkByPass(0, sink) != DxB_ERR_OK)
		return 1;
	return 0;
}

int tcc_manager_video_SupportFieldDecoding(unsigned int OnOff)
{
	return TCC_DxB_VIDEO_SupportFieldDecoding(OnOff);
}

int tcc_manager_video_SupportIFrameSearch(unsigned int OnOff)
{
	return TCC_DxB_VIDEO_SupportIFrameSearch(OnOff);
}

int tcc_manager_video_SupportUsingErrorMB(unsigned int OnOff)
{
	return TCC_DxB_VIDEO_SupportUsingErrorMB(OnOff);
}

int tcc_manager_video_SupportDirectDisplay(unsigned int OnOff)
{
	return TCC_DxB_VIDEO_SupportDirectDisplay(OnOff);
}

int tcc_manager_video_init_surface(int arg)
{
    return TCC_DxB_VIDEO_InitSurface(arg);
}

int tcc_manager_video_deinit_surface()
{
    return TCC_DxB_VIDEO_DeinitSurface();
}

int tcc_manager_video_set_surface(void *nativeWidow)
{
    return TCC_DxB_VIDEO_SetSurface(nativeWidow);
}

int tcc_manager_video_UseSurface(void)
{
    return TCC_DxB_VIDEO_UseSurface();
}

int tcc_manager_video_ReleaseSurface(void)
{
    return TCC_DxB_VIDEO_ReleaseSurface();
}
