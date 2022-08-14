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


#ifndef __SUBTITLE_DISPLAY_H__
#define __SUBTITLE_DISPLAY_H__

#ifdef __cplusplus
	extern "C" {
#endif


#include <ISDBT_Caption.h>

typedef struct{
	int (*pInit)(int);
	int (*pClose)(void);
}SUB_OP_TYPE;

typedef struct{
	int handle;
	int flash_handle;
	int x;
	int y;
	int w;
	int h;
	unsigned long long pts;
}DISP_CTX_TYPE;

extern int subtitle_display_get_ccfb_fd(void);
extern void subtitle_display_set_ccfb_fd(int fd);
extern int subtitle_display_get_cls(int type);
extern void subtitle_display_set_cls(int type, int set);
extern void subtitle_display_set_skip_pts(int skip);
extern int subtitle_display_open_thread(void);
extern int subtitle_display_close_thread(void);

extern int subtitle_display_pre_init(SUB_ARCH_TYPE arch_type);
extern int subtitle_display_init(int data_type, void *p);
extern int subtitle_display_exit(void);
extern int subtitle_display_close(void);

//Subtitle
extern void subtitle_data_enable(int enable);
extern void subtitle_data_update(int handle);
extern void superimpose_data_update(int handle);
extern void subtitle_update(int update_flag);
extern void superimpose_update(int update_flag);
extern void subtitle_data_memcpy(void);
extern void subtitle_update(int update_flag);
//Superimpose
extern void superimpose_data_enable(int enable);
extern void superimpose_data_update(int handle);
extern void superimpose_data_memcpy(void);
extern void superimpose_update(int update_flag);
//PNG
extern void png_data_enable(int enable);
extern void png_data_update(int handle);
extern void png_data_memcpy(void);
extern void png_update(int update_flag);

extern void subtitle_display_clear_screen(int type, DISP_CTX_TYPE *pSt, DISP_CTX_TYPE *pSi);

extern int subtitle_display_init_ccfb(int arch_type);
extern int subtitle_display_close_ccfb(void);

extern void subtitle_display_memcpy_ex(unsigned int *pSrc, int src_x, int src_y, int src_w, int src_h, int src_pitch_w, unsigned int *pDst, int dst_x, int dst_y, int dst_pitch_w);

#ifdef __cplusplus
}
#endif

#endif	/* __SUBTITLE_DISPLAY_H__ */

