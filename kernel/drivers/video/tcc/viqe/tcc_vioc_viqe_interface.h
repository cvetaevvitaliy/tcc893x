/****************************************************************************
linux/drivers/video/tcc/viqe/tcc_vioc_viqe_interface.h
Description: TCC VIOC h/w block 

Copyright (C) 2013 Telechips Inc.

This program is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA 02111-1307 USA
****************************************************************************/

#ifndef TCC_VIOC_VIQE_INTERFACE_H_INCLUDED
#define TCC_VIOC_VIQE_INTERFACE_H_INCLUDED
#include <mach/tca_fb_output.h>

 void TCC_VIQE_DI_PlugInOut_forAlphablending(int plugIn);
 void TCC_VIQE_DI_Init(int scalerCh, int useWMIXER, unsigned int FrmWidth, unsigned int FrmHeight, int crop_top, int crop_bottom, int crop_left, int crop_right, int OddFirst);
 void TCC_VIQE_DI_Run(int DI_use);
 void TCC_VIQE_DI_DeInit(void);
 void TCC_VIQE_DI_Init60Hz(TCC_OUTPUT_TYPE outputMode, int lcdCtrlNum, int Lcdc_layer, int useSCALER, unsigned int img_fmt, 
						unsigned int srcWidth, unsigned int srcHeight,
						unsigned int destWidth, unsigned int destHeight, unsigned int offset_x, unsigned int offset_y, int OddFirst);
void TCC_VIQE_DI_Swap60Hz(int mode);
void TCC_VIQE_DI_SetFMT60Hz(int enable);
void TCC_VIQE_DI_Run60Hz(int useSCALER, unsigned int addr0, unsigned int addr1, unsigned int addr2,
						unsigned int srcWidth, unsigned int srcHeight,	
						int crop_top, int crop_bottom, int crop_left, int crop_right,
						unsigned int destWidth, unsigned int destHeight,
						unsigned int offset_x, unsigned int offset_y, int OddFirst, int FrameInfo_Interlace, int reset_frmCnt);
 void TCC_VIQE_DI_DeInit60Hz(void);


#endif
