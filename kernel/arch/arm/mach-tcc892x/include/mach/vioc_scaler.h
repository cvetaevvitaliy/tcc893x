/*
 * linux/arch/arm/mach-tcc892x/include/mach/vioc_scaler.h
 * Author:  <linux@telechips.com>
 * Created: June 10, 2008
 * Description: TCC VIOC h/w block 
 *
 * Copyright (C) 2008-2009 Telechips
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see the file COPYING, or write
 * to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __VIOC_SCALER_H__
#define	__VIOC_SCALER_H__

#include  <mach/reg_physical.h>


typedef enum {
	SC_IMG_FMT_1PPP = 0,
	SC_IMG_FMT_2PPP,
	SC_IMG_FMT_4PPP,
	SC_IMG_FMT_8PPP,
	SC_IMG_FMT_RGB332 = 8, 				// 1 bytes aligned : R[7:5],G[4:2],B[1:0]
	SC_IMG_FMT_RGB444, 					// 2 bytes aligned : A[15:12],R[11:8],G[7:4],B[3:0]
	SC_IMG_FMT_RGB565, 					// 2 bytes aligned : R[15:11],G[10:5],B[4:0]
	SC_IMG_FMT_RGB555, 					// 2 bytes aligned : A[15],R[14:10],G[9:5],B[4:0]
	SC_IMG_FMT_ARGB8888, 				// 4 bytes aligned : A[31:24],R[23:16],G[15:8],B[7:0]
	SC_IMG_FMT_RGB666, 					// 4 bytes aligned : A[23:18],R[17:12],G[11:6],B[5:0]
	SC_IMG_FMT_RGB888, 					// 3 bytes aligned : B1[31:24],R0[23:16],G0[15:8],B0[7:0]
	SC_IMG_FMT_ARGB6666, 				// 3 bytes aligned : A[23:18],R[17:12],G[11:6],B[5:0]
	SC_IMG_FMT_FCDEC, 					// Frame Decompression Mode
	SC_IMG_FMT_YCbCr422_SEQ_UYVY = 22, 	// sequential format : LSB [U/Y/V/Y] MSB
	SC_IMG_FMT_YCbCr422_SEQ_VYUY,		// sequential format : LSB [V/Y/U/Y] MSB
	SC_IMG_FMT_YCbCr420_SEP,
	SC_IMG_FMT_YCbCr422_SEP,
	SC_IMG_FMT_YCbCr422_SEQ_YUYV, 		// sequential format : LSB [Y/U/Y/V] MSB
	SC_IMG_FMT_YCbCr422_SEQ_YVYU, 		// sequential format : LSB [Y/V/Y/U] MSB
	SC_IMG_FMT_YCbCr420_INT_TYPE0, 		// YCbCr 4:2:0 interleaved type 0 format
	SC_IMG_FMT_YCbCr420_INT_TYPE1, 		// YCbCr 4:2:0 interleaved type 1 format
	SC_IMG_FMT_YCbCr422_INT_TYPE0,		// YCbCr 4:2:2 interleaved type 0 format
	SC_IMG_FMT_YCbCr422_INT_TYPE1,		// YCbCr 4:2:2 interleaved type 1 format
} VIOC_SCALER_IMAGE_FORMAT_TYPE;


/* Interface APIs */
extern void VIOC_SC_SetBypass(PVIOC_SC pSCALER, unsigned int nOnOff);
extern void VIOC_SC_SetUpdate(PVIOC_SC pSCALER);
extern void VIOC_SC_SetSrcSize(PVIOC_SC pSCALER, unsigned int nWidth, unsigned int nHeight);
extern void VIOC_SC_SetDstSize(PVIOC_SC pSCALER, unsigned int nWidth, unsigned int nHeight);
extern void VIOC_SC_SetOutSize(PVIOC_SC pSCALER, unsigned int nWidth, unsigned int nHeight);
extern void VIOC_SC_SetOutPosition(PVIOC_SC pSCALER, unsigned int nXpos, unsigned int nYpos);
extern void VIOC_SC_SetInterruptMask(PVIOC_SC pSCALER, unsigned int nMask, unsigned int set);
extern void VIOC_SC_IreqHandler(int index, int irq, void *client_data);
extern void VIOC_SC_SetSWReset(unsigned int SC, unsigned int RDMA, unsigned int WDMA);
#endif /*__VIOC_SCALER_H__*/



