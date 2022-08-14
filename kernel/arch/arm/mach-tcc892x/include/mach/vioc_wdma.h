/*
 * linux/arch/arm/mach-tcc892x/include/mach/vioc_wdma.h
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
#ifndef __VIOC_WDMA_H__
#define	__VIOC_WDMA_H__
#include <mach/reg_physical.h>

#define VIOC_WDMA_IREQ_UPD_MASK 		0x00000001UL
#define VIOC_WDMA_IREQ_SREQ_MASK 		0x00000002UL
#define VIOC_WDMA_IREQ_ROLL_MASK 		0x00000004UL
#define VIOC_WDMA_IREQ_ENR_MASK 		0x00000008UL
#define VIOC_WDMA_IREQ_ENF_MASK 		0x00000010UL
#define VIOC_WDMA_IREQ_EOFR_MASK 		0x00000020UL
#define VIOC_WDMA_IREQ_EOFF_MASK 		0x00000040UL
#define VIOC_WDMA_IREQ_SEOFR_MASK 		0x00000080UL
#define VIOC_WDMA_IREQ_SEOFF_MASK 		0x00000100UL
#define VIOC_WDMA_IREQ_STSEN_MASK 		0x20000000UL
#define VIOC_WDMA_IREQ_STBF_MASK 		0x40000000UL
#define VIOC_WDMA_IREQ_STEOF_MASK 		0x80000000UL
#define VIOC_WDMA_IREQ_ALL_MASK 		0xFFFFFFFFUL

typedef struct
{
	unsigned int ImgSizeWidth;
	unsigned int ImgSizeHeight;
	unsigned int TargetWidth;
	unsigned int TargetHeight;
	unsigned int ImgFormat;
	unsigned int BaseAddress;
	unsigned int BaseAddress1;
	unsigned int BaseAddress2;
	unsigned int Interlaced;
	unsigned int ContinuousMode;
	unsigned int SyncMode;
	unsigned int AlphaValue;
	unsigned int Hue;
	unsigned int Bright;
	unsigned int Contrast;
} VIOC_WDMA_IMAGE_INFO_Type;


/* Interface APIs. */
extern void VIOC_WDMA_SetDefaultImageConfig(VIOC_WDMA * pWDMA, VIOC_WDMA_IMAGE_INFO_Type * ImageCfg, unsigned int r2yEn);
extern void VIOC_WDMA_SetWifiDisplayImageConfig(VIOC_WDMA * pWDMA, VIOC_WDMA_IMAGE_INFO_Type * ImageCfg, unsigned int r2yEn);
extern void VIOC_WDMA_SetImageEnable(VIOC_WDMA * pWDMA, unsigned int nContinuous);
extern void VIOC_WDMA_SetImageDisable(VIOC_WDMA * pWDMA);
extern void VIOC_WDMA_SetImageUpdate(VIOC_WDMA * pWDMA);
extern void VIOC_WDMA_SetImageFormat(VIOC_WDMA *pWDMA, unsigned int nFormat);
extern void VIOC_WDMA_SetImageInterlaced(VIOC_WDMA *pWDMA, unsigned int intl);
extern void VIOC_WDMA_SetImageR2YMode(VIOC_WDMA *pWDMA, unsigned int r2y_mode);
extern void VIOC_WDMA_SetImageR2YEnable(VIOC_WDMA *pWDMA, unsigned int enable);
extern void VIOC_WDMA_SetImageRateControl(VIOC_WDMA *pWDMA, unsigned int enable, unsigned int rate);
extern void VIOC_WDMA_SetImageSyncControl(VIOC_WDMA *pWDMA, unsigned int enable, unsigned int RDMA);
extern void VIOC_WDMA_SetImageDither(VIOC_WDMA *pWDMA, unsigned int nDithEn, unsigned int nDithSel, unsigned char mat[4][4]);
extern void VIOC_WDMA_SetImageEnhancer(VIOC_WDMA *pWDMA, unsigned int nContrast, unsigned int nBright, unsigned int nHue);
extern void VIOC_WDMA_SetImageSize(VIOC_WDMA *pWDMA, unsigned int sw, unsigned int sh);
extern void VIOC_WDMA_SetImageBase(VIOC_WDMA *pWDMA, unsigned int nBase0, unsigned int nBase1, unsigned int nBase2);
extern void VIOC_WDMA_SetImageOffset(VIOC_WDMA *pWDMA, unsigned int imgFmt, unsigned int imgWidth);
extern void VIOC_WDMA_SetIreqMask(VIOC_WDMA * pWDMA, unsigned int mask, unsigned int set);
extern void VIOC_WDMA_IreqHandler(unsigned int vectorID);
extern void VIOC_WDMA_SWReset(PVIOC_IREQ_CONFIG pIrgConfig, unsigned int WDMA);

#endif
