/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions 
andlimitations under the License.

****************************************************************************/
/******************************************************************************
 file name : TCCXXX_IMAGE_CUSTOM_OUTPUT_SET.h (V1.55)
******************************************************************************/

#ifndef __TCCXXX_IMAGE_CUSTOM_OUTPUT_SET_H__
#define __TCCXXX_IMAGE_CUSTOM_OUTPUT_SET_H__

typedef struct {
	int Comp_1;
	int Comp_2;
	int Comp_3;
	int Comp_4;		//V.1.52~
	int Offset;
	int x;
	int y;
	int Src_Fmt;
}IM_PIX_INFO;

#define IM_SRC_YUV		0
#define IM_SRC_RGB		1
#define IM_SRC_OTHER	2

extern unsigned int	IM_2nd_Offset;
extern unsigned int	IM_3rd_Offset;
extern unsigned int	IM_LCD_Half_Stride;
extern unsigned int	IM_LCD_Addr;

void TCC_CustomOutput_RGB565(IM_PIX_INFO out_info);
void TCC_CustomOutput_RGB888_With_Alpha(IM_PIX_INFO out_info);
void TCC_CustomOutput_RGB888(IM_PIX_INFO out_info);
void TCC_CustomOutput_YUV420(IM_PIX_INFO out_info);
void TCC_CustomOutput_YUV444(IM_PIX_INFO out_info);

void TCC_CustomOutput_RGB888_With_Alpha_ISDBT(IM_PIX_INFO out_info); //PNGDEC_SUPPORT_ISDBT_PNG

#endif //__TCCXXX_IMAGE_CUSTOM_OUTPUT_SET_H__
