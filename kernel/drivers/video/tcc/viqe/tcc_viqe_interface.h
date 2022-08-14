/****************************************************************************
linux/drivers/video/tcc/viqe/tcc_viqe_interface.h
Description: TCC VIQE h/w block 

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

#ifndef TCC_VIQE_INTERFACE_H_INCLUDED
#define TCC_VIQE_INTERFACE_H_INCLUDED

void* VIQE_PA_malloc(int uiSize);
void VIQE_PA_free(void* pPtr, int uiSize);
void TCC_DeInitializeVIQE(int lcdCtrlNo, int scalerCh, int DI_use);
void TCC_InitalizeVIQE(int lcdCtrlNo, int scalerCh, int OnTheFly, int DI_use, unsigned int FrmWidth, unsigned int FrmHeight,
					int crop_top, int crop_bottom, int crop_left, int crop_right);

int TCC_RunVIQE( unsigned int CurYBase, unsigned int CurUBase, unsigned int CurVBase, 
		unsigned int PrevYBase, unsigned int PrevUBase, unsigned int PrevVBase, 
		unsigned int OutputMEMBase, unsigned int M2MMode, int InputFormat, int OutputFormat,
		int OddFirst, int DI_use);

void TCC_ExcuteVIQE(int scalerCh, unsigned int *address,		// address[0:3] : previous, address[4:6] : current
						int srcWidth, int srcHeight,			// srcWidth, srcHeight
						int crop_top, int crop_bottom, int crop_left, int crop_right,	// offset top, offset bottom, offset left, offset right
						int lcdCtrlNo, unsigned int dstAddr,
						int deinterlaceM2M, int OddFirst, int DI_use);

int TCC_ConfigVIQEParamSet(int mode, int region, int strength1, int strength2, int modeparam);

int TCC_RunVIQE60( unsigned int CurYBase, unsigned int CurUBase, unsigned int CurVBase, 
		unsigned int PrevYBase, unsigned int PrevUBase, unsigned int PrevVBase, 
		int OddFirst, int DI_use);

void TCC_ExcuteVIQE_60Hz(int scalerCh, unsigned int *address,	// address[0:3] : previous, address[4:6] : current
						int srcWidth, int srcHeight,			// srcWidth, srcHeight
						int crop_top, int crop_bottom, int crop_left, int crop_right,	// offset top, offset bottom, offset left, offset right
						int dstWidth, int dstHeight, 			// dstWidth, dstHeight
						int offset_x, int offset_y,
						int lcdCtrlNo, int deinterlaceM2M, int renderCnt,
						int OddFirst, int DI_use);
void TCC_InitalizeVIQE_60Hz(int scalerCh, unsigned int *address,			// address[0:3] : previous, address[4:6] : current
						int srcWidth, int srcHeight,			// srcWidth, srcHeight
						int crop_top, int crop_bottom, int crop_left, int crop_right,	// offset top, offset bottom, offset left, offset right
						int dstWidth, int dstHeight, 			// dstWidth, dstHeight
						int offset_x, int offset_y,
						int lcdCtrlNo, int deinterlaceM2M, int renderCnt,
						int OddFirst, int DI_use);
void ShowVIQE(void);
#endif
