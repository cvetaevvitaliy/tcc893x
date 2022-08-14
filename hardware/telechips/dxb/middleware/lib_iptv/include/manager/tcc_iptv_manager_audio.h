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

#ifndef	_TCC_IPTV_MANAGER_AUDIO_H_
#define	_TCC_IPTV_MANAGER_AUDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int tcc_manager_audio_init(void);
extern int tcc_manager_audio_start(unsigned int ulAudioFormat);
extern int tcc_manager_audio_stop(unsigned int uidevid);
extern int tcc_manager_audio_stereo(unsigned int ulMode);
extern int tcc_manager_audio_volume(unsigned int ulVolume);
extern int tcc_manager_audio_mute(unsigned int bOnOff);
extern int tcc_manager_audio_setPause(int pause);
extern int tcc_manager_audio_setSinkBypass(int sink);
extern int tcc_manager_audio_select_output(unsigned int uidevid, unsigned int isEnableAudioOutput);

#ifdef __cplusplus
}
#endif

#endif


