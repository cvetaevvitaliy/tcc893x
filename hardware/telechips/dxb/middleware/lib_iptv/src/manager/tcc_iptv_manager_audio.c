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
#define LOG_TAG	"DVB_MANAGER_AUDIO"
#include <utils/Log.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "tcc_dxb_interface_audio.h" 
#include "tcc_iptv_manager_audio.h"

void tcc_audio_event(DxB_AUDIO_NOTIFY_EVT nEvent, void  *pEventData)
{
    ALOGD("%s:[%d][%p]", __func__, nEvent, pEventData);
}

int tcc_manager_audio_init(void)
{
    TCC_DxB_AUDIO_Init();
    TCC_DxB_AUDIO_RegisterEventCallback(0, tcc_audio_event);
	return 0;
}

int tcc_manager_audio_start(unsigned int ulAudioFormat)
{
	return TCC_DxB_AUDIO_Start(0, ulAudioFormat);
}

int tcc_manager_audio_stop(unsigned int uidevid)
{
	return TCC_DxB_AUDIO_Stop(uidevid);
}

int tcc_manager_audio_stereo(unsigned int ulMode)
{
	return TCC_DxB_AUDIO_SetStereo(0, ulMode);
}

int tcc_manager_audio_volume(unsigned int ulVolume)
{
	return TCC_DxB_AUDIO_SetVolume(0, ulVolume);
}

int tcc_manager_audio_mute(unsigned int bOnOff)
{
	return TCC_DxB_AUDIO_SetMute(0, bOnOff);
}

int tcc_manager_audio_setPause(int pause)
{	
	if( TCC_DxB_Audio_SetPause(0, pause) != DxB_ERR_OK)
		return 1;
	return 0;
}

int tcc_manager_audio_setSinkBypass(int sink)
{	
	if( TCC_DxB_Audio_SetSinkByPass(0, sink) != DxB_ERR_OK)
		return 1;
	return 0;
}

int tcc_manager_audio_select_output(unsigned int uidevid, unsigned int isEnableAudioOutput)
{
	return TCC_DxB_AUDIO_Select_Output(uidevid, isEnableAudioOutput);
}
