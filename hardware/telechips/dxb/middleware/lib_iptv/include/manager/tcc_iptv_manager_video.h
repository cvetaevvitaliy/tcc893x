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

#ifndef	_TCC_DVB_MANAGER_VIDEO_H_
#define	_TCC_DVB_MANAGER_VIDEO_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int tcc_manager_video_init(void);
extern int tcc_manager_video_start(unsigned int ulVideoFormat);
extern int tcc_manager_video_stop(unsigned int uidevid);
extern int tcc_manager_video_pause(unsigned int uOnOff);
extern int tcc_manager_video_capture(char *pucFileName);
extern int tcc_manager_video_GetVideoInfo( int *video_width, int *video_height);
extern int tcc_manager_video_setVpuReset(void);
extern int tcc_manager_video_setactivemode(int activemode);
extern int tcc_manager_video_setPause(int pause);
extern int tcc_manager_video_iFrameSearchEnable(void);
extern int tcc_manager_video_RefreshDisplay(void);
extern int tcc_manager_video_setSinkBypass(int sink);
extern int tcc_manager_video_SupportFieldDecoding(unsigned int OnOff);
extern int tcc_manager_video_SupportIFrameSearch(unsigned int OnOff);
extern int tcc_manager_video_SupportUsingErrorMB(unsigned int OnOff);
extern int tcc_manager_video_SupportDirectDisplay(unsigned int OnOff);
extern int tcc_manager_video_init_surface(int arg);
extern int tcc_manager_video_deinit_surface();
extern int tcc_manager_video_set_surface(void *nativeWidow);
extern int tcc_manager_video_UseSurface(void);
extern int tcc_manager_video_ReleaseSurface(void);

#ifdef __cplusplus
}
#endif

#endif
