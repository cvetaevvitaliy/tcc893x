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


#ifndef __SUBTITLE_MAIN_H__
#define __SUBTITLE_MAIN_H__

#include <ISDBT_Caption.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
	/* 0:ccfb, 1:surface */
	unsigned int fb_type;

	/* video view size in java application */
	unsigned int raw_w;
	unsigned int raw_h;
	unsigned int view_w;
	unsigned int view_h;
	
	/* max resolution is 1920x1080 for display : lcdc */
	unsigned int disp_x;
	unsigned int disp_y;
	unsigned int disp_w;
	unsigned int disp_h;

	/* For LCDC magnification
	  * TCC92xx 
	  * 0:no scale, 1:x2, 2:x3, 3:x4, 4:x8 - Only Up-scale supported 
	  *
	  * TCC93/88xx
	  * 0:no scale, 1:/2, 2:/3, 3:/4, 4-6:rsvd, 7:/8 - Down-scale
	  * 8:rsvd, 9:x2, 10:x3, 11:x4, 12-14:rsvd, 15:x8 - Up-scale
	  */
	unsigned int disp_m;

	/* max resolution is 960x540 for subtitle display : memory */
	unsigned int sub_buf_w;
	unsigned int sub_buf_h;

	double disp_ratio_x;
	double disp_ratio_y;
}SUB_SYS_RES_TYPE;

typedef struct{
	unsigned int 			act_lcdc_num;
	E_DTV_MODE_TYPE	isdbt_type;
	SUB_ARCH_TYPE 		arch_type;
	int 					stb_mode;
	int 					output_mode;
	int 					hdmi_mode;
	int 					hdmi_res;
	int					hdmi_cable_mode;
	int 					composite_mode;
	int 					component_mode;
	int					disp_mode;
	int					disp_open;
	int					disp_condition;
	int					disp_sub_condition;
	int					disp_super_condition;	
	int					subtitle_size;
	int					superimpose_size;
	int					png_size;	
	void*				subtitle_pixels;
	void*				superimpose_pixels;
	void*				png_pixels;	
	unsigned int*		subtitle_addr;	
	unsigned int*		superimpose_addr;
	unsigned int*		png_addr;	
	SUB_SYS_RES_TYPE	res;
}SUB_SYS_INFO_TYPE;

typedef struct{
	SUB_SYS_INFO_TYPE sys_info;
	T_SUB_CONTEXT	sub_ctx[2];
}SUBTITLE_CONTEXT_TYPE;

extern SUB_SYS_INFO_TYPE* get_sub_context(void);

extern int subtitle_demux_check_statement_in(void);
extern void subtitle_demux_set_subtitle_lang(int cnt);
extern void subtitle_demux_set_superimpose_lang(int cnt);
extern void subtitle_demux_select_subtitle_lang(int index, int sel_cnt);
extern void subtitle_demux_select_superimpose_lang(int index, int sel_cnt);

extern int subtitle_demux_init(int seg_type, int country, int raw_w, int raw_h, int view_w, int view_h);
extern int subtitle_demux_exit(void);
extern int subtitle_demux_dec(int data_type, unsigned char *pData, unsigned int uiDataSize, unsigned long long iPts);
extern int subtitle_demux_process(void *p_disp_info, int disp_handle, unsigned long long  pts, int flash_flag, int time_flag);
extern int subtitle_get_num_languages(int data_type);

#ifdef __cplusplus
}
#endif

#endif	/* __SUBTITLE_MAIN_H__ */

