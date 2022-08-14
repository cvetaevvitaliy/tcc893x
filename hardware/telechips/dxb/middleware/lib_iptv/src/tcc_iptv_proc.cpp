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
#define LOG_TAG	"IPTV_Proc"

#include <utils/Log.h>
#include "tcc_iptv_proc.h"
#include "tcc_alticas.h"
#include "tcc_iptv_manager_demux.h" 
#include "tcc_iptv_manager_video.h"
#include "tcc_iptv_manager_audio.h"
#include "tcc_iptv_manager_socket.h"

#define DEBUG_MSG	ALOGE

typedef enum
{
	TCCDXBPROC_IDLE,
	TCCDXBPROC_LOAD,
	TCCDXBPROC_PLAY,
	TCCDXBPROC_ERROR
}TCCDXBPROC_STATE;


typedef enum
{
	TCCDXBPROC_STANDBYMODE=0,
	TCCDXBPROC_PLAYMODE,
	TCCDXBPROC_TRICK_PLAYMODE,
	TCCDXBPROC_PAUSEMODE,
	TCCDXBPROC_SEEK
}TCCDXBPROC_IPTVMODESTATE;

static TCCDXBPROC_STATE geTCCDXBProcState = TCCDXBPROC_IDLE;
static unsigned int	iptv_activemode = TCCDXBPROC_STANDBYMODE;

int TCCDxBProc_Init(TCCDXBPROC_INIT *pArg)
{
	DEBUG_MSG("%s::%d", __func__,__LINE__);	
	geTCCDXBProcState = TCCDXBPROC_IDLE;
	tcc_manager_demux_init();
	tcc_manager_audio_init();
	tcc_manager_video_init();
	geTCCDXBProcState = TCCDXBPROC_LOAD;
	
	return 0;
}

int TCCDxBProc_DeInit(void)
{
	DEBUG_MSG("%s", __func__);	
	tcc_manager_video_stop(1);
	tcc_manager_video_stop(0);
	tcc_manager_audio_stop(0);
	tcc_manager_audio_stop(1);        
	tcc_manager_demux_deinit();
	geTCCDXBProcState = TCCDXBPROC_IDLE;
	return 0;
}

int TCCDxBProc_Stop(void)
{
	DEBUG_MSG("%s geTCCDXBProcState = %d \n", __func__, geTCCDXBProcState);	
	if(geTCCDXBProcState == TCCDXBPROC_PLAY)
	{
		DEBUG_MSG("%s:: Stop current playing!!!\n", __func__);

		if(iptv_activemode  == TCCDXBPROC_PAUSEMODE)
			TCCDxBProc_SetActiveMode(TCCDXBPROC_PLAYMODE);

		TCCDxBProc_SocketStop();
		tcc_manager_video_stop(1);
		tcc_manager_video_stop(0);
		tcc_manager_audio_stop(0);
		tcc_manager_audio_stop(1);        
		tcc_manager_demux_ip_avstop();
		geTCCDXBProcState = TCCDXBPROC_IDLE;
		DEBUG_MSG("%s:: Current State TCCDXBPROC_IDLE!!!\n", __func__);
		geTCCDXBProcState = TCCDXBPROC_LOAD;	
		DEBUG_MSG("%s:: Current State TCCDXBPROC_LOAD!!!\n", __func__);
	}
	return 0;
}

int TCCDxBProc_Socketinit(void)
{
	DEBUG_MSG("%s", __func__);	
	tcc_manager_socket_init();
 	return 0;
}

int TCCDxBProc_SocketIpsetting(TCCDXBPROC_SET_IP *pArg)
{
	DEBUG_MSG("%s", __func__);	
	tcc_manager_socket_ipsetting((unsigned char *)pArg->aucIPStr, pArg->uiPort, pArg->uiProtocol);
 	return 0;
}

int TCCDxBProc_SocketStart(void)
{
	DEBUG_MSG("%s", __func__);	
	tcc_manager_socket_start();
 	return 0;
}

int TCCDxBProc_SocketStop(void)
{
	DEBUG_MSG("%s", __func__);	
	tcc_manager_socket_stop();
 	return 0;
}

int TCCDxBProc_SocketCommand(unsigned int cmd)
{
	DEBUG_MSG("%s", __func__);	
	tcc_manager_socket_command(cmd);
 	return 0;
}

int TCCDxBProc_SetActiveMode(unsigned int activemode)
{
	DEBUG_MSG("%s activemode = %d ", __func__, activemode);	
	tcc_manager_demux_setactivemode(activemode);
	tcc_manager_video_setactivemode(activemode);

	if(activemode == TCCDXBPROC_STANDBYMODE)
	{
		tcc_manager_video_setPause(TRUE);
		tcc_manager_audio_setPause(TRUE);
		tcc_manager_video_setSinkBypass(FALSE);
		tcc_manager_audio_setSinkBypass(FALSE);
	}
	else if(activemode == TCCDXBPROC_PLAYMODE)
	{
		tcc_manager_video_setPause(TRUE);
		tcc_manager_audio_setPause(TRUE);
		tcc_manager_video_RefreshDisplay();

		tcc_manager_video_setPause(FALSE);
		tcc_manager_audio_setPause(FALSE);
		tcc_manager_video_setSinkBypass(FALSE);
		tcc_manager_audio_setSinkBypass(FALSE);
	}
	else  if(activemode == TCCDXBPROC_TRICK_PLAYMODE)
	{
		tcc_manager_video_setPause(TRUE);
		tcc_manager_audio_setPause(TRUE);
		tcc_manager_video_RefreshDisplay();

		tcc_manager_video_setPause(FALSE);
		tcc_manager_audio_setPause(FALSE);
		tcc_manager_video_setSinkBypass(FALSE);
		tcc_manager_audio_setSinkBypass(FALSE);
	}
	else  if(activemode == TCCDXBPROC_SEEK)
	{
		tcc_manager_video_iFrameSearchEnable();
	}	

	iptv_activemode = activemode;
 	return 0;
}

int TCCDxBProc_GetVideoInfo(int *width, int *height)
{
	DEBUG_MSG("%s %d ", __func__, __LINE__);	
	tcc_manager_video_GetVideoInfo(width, height);
 	return 0;
}

int TCCDxBProc_MultiCastingSet(TCCDXBPROC_SET_IP *pArg, TCCDXBPROC_SET *pStream_Arg)
{
	DEBUG_MSG("%s::%d", __func__,__LINE__);	
	if(geTCCDXBProcState == TCCDXBPROC_IDLE)
	{
		DEBUG_MSG("%s:: NOT Ready !!!\n", __func__);
		return 1;
	}

	if(geTCCDXBProcState == TCCDXBPROC_PLAY)
	{
		DEBUG_MSG("%s:: Stop current playing!!!\n", __func__);

		if(iptv_activemode  == TCCDXBPROC_PAUSEMODE)
			TCCDxBProc_SetActiveMode(TCCDXBPROC_PLAYMODE);

		TCCDxBProc_SocketStop();
		tcc_manager_video_stop(1);
		tcc_manager_video_stop(0);
		tcc_manager_audio_stop(0);
		tcc_manager_audio_stop(1);        
		tcc_manager_demux_ip_avstop();

		geTCCDXBProcState = TCCDXBPROC_IDLE;
		DEBUG_MSG("%s:: Current State TCCDXBPROC_IDLE!!!\n", __func__);
		geTCCDXBProcState = TCCDXBPROC_LOAD;	
		DEBUG_MSG("%s:: Current State TCCDXBPROC_LOAD!!!\n", __func__);
	}
	
	ALOGE("%s %d audio_pid = %x  video_pid = %x \n", __func__, __LINE__, pStream_Arg->iAudioPID, pStream_Arg->iVideoPID);

	tcc_manager_video_SupportFieldDecoding(TRUE);
	tcc_manager_video_SupportIFrameSearch(TRUE);
	tcc_manager_video_SupportUsingErrorMB(TRUE);
	tcc_manager_video_SupportDirectDisplay(TRUE);

	if(pStream_Arg->iAudioPID != NULL &&  pStream_Arg->iVideoPID != NULL)	
	{
		ALOGE("%s %d audio_pid = %x	video_pid = %x \n", __func__, __LINE__, pStream_Arg->iAudioStreamType, pStream_Arg->iVideoStreamType);

		if( tcc_manager_audio_start(pStream_Arg->iAudioStreamType ) < 0)
		{
			pStream_Arg->iAudioStreamType = 0xFFFF;
			pStream_Arg->iAudioPID = 0xFFFF;		
		}
		
		if( tcc_manager_video_start(pStream_Arg->iVideoStreamType) < 0)
		{
			pStream_Arg->iVideoStreamType = 0xFFFF;
			pStream_Arg->iVideoPID = 0xFFFF;		
		}
		tcc_manager_demux_avplay(pStream_Arg->iAudioPID, pStream_Arg->iVideoPID, pStream_Arg->iPcrPID);
	}
	else
		tcc_manager_demux_startdemuxing();

	tcc_manager_audio_select_output(0, true);

	geTCCDXBProcState = TCCDXBPROC_PLAY;
	return 0;
}

int TCCDxBProc_InitVideoSurface(int arg)
{
    int error=0;
    error = tcc_manager_video_init_surface(arg);
    return error;
}

int TCCDxBProc_DeinitVideoSurface(void)
{
    int error=0;
    error = tcc_manager_video_deinit_surface();
    return error;
}


int TCCDxBProc_SetVideoSurface(void *nativeWidow)
{
    int error=0;
    error = tcc_manager_video_set_surface(nativeWidow);
    return error;
}

int TCCDxBProc_VideoUseSurface(void)
{
    int error=0;
    error = tcc_manager_video_UseSurface(); 
    return error;
}

int TCCDxBProc_VideoReleaseSurface(void)
{
    int error=0;
    error = tcc_manager_video_ReleaseSurface();
    return error;
}
