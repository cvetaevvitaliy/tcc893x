/*
 * tcc_fontDraw.c
 *
 *  Copyright (C) 2011, Telechips, Inc.
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */
#include <utils/Log.h>

#include <stdio.h>
#include <stdlib.h>

#include "tcc_font.h"
#include "tcc_fontDraw.h"

/**************************************************************************
							DEFINE
**************************************************************************/
#if 0 
	#define ERR_PRINT			ALOGE
	#define DBG_PRINT			ALOGD
#else
	#define ERR_PRINT			
	#define DBG_PRINT
#endif

/**************************************************************************
							GLOBAL VARIABLE
**************************************************************************/
static unsigned int* g_puiDestBuf;
static unsigned int g_uiDestWidth;
static unsigned int g_uiDestHeight;
static unsigned int g_uiDestPitch;
static unsigned int g_uiDestForeColor;
static unsigned int g_uiDestHalfForeColor;
static unsigned int g_uiDestBackColor;
static unsigned int g_uiDestHalfBackColor;
static unsigned int g_uiDirection;
static unsigned int g_uiDestLineEffectColor;

extern void tcc_memset16 (void *s,int c, int n);

/**************************************************************************
							FUNCTION
**************************************************************************/
int tcc_fontDraw_init(void)
{
	g_puiDestBuf = NULL;
	g_uiDestWidth = 0;
	g_uiDestHeight = 0;
	g_uiDestPitch = 0;
	g_uiDestForeColor = 0;
	g_uiDestHalfForeColor = 0;
	g_uiDestBackColor = 0;
	g_uiDestHalfBackColor = 0;
	g_uiDestLineEffectColor = 0;

	return TCC_FONT_SUCCESS;
}

int tcc_fontDraw_close(void)
{
	return TCC_FONT_SUCCESS;
}

int tcc_fontDraw_setDestBuf(unsigned char *pBuf)
{
	if( NULL == pBuf ) {
		ERR_PRINT("Error Invalid Param(pBuf:%d)\n", pBuf);
		return TCC_FONT_ERROR_INVALID_PARAM;
	}

	g_puiDestBuf = (unsigned int*)pBuf;
	
	return TCC_FONT_SUCCESS;
}

int tcc_fontDraw_setDestRegion(unsigned int uiWidth, unsigned int uiHeight, unsigned int uiPitch)
{
	if( 0 == uiWidth || 0 == uiHeight || 0 == uiPitch ) {
		ERR_PRINT("Error Invalid Param(uiWidth:%d uiHeight:%d uiPitch:%d)\n", uiWidth, uiHeight, uiPitch);
		return TCC_FONT_ERROR_INVALID_PARAM;		
	}

	g_uiDestWidth = uiWidth;
	g_uiDestHeight = uiHeight;
	g_uiDestPitch = uiPitch;

	return TCC_FONT_SUCCESS;
}

unsigned int* tcc_fontDraw_getDestBuf(void)
{
	return g_puiDestBuf;
}

int tcc_fontDraw_getDestRegion(unsigned int* puiWidth, unsigned int* puiHeight, unsigned int *puiPitch)
{
	*puiWidth = g_uiDestWidth;
	*puiHeight = g_uiDestHeight;
	*puiPitch = g_uiDestPitch;

	return TCC_FONT_SUCCESS;	
}

int tcc_fontDraw_setFontColor(unsigned int uiForeColor, unsigned int uiHalfForeColor, unsigned int uiBackColor, unsigned int uiHalfBackColor)
{
	g_uiDestForeColor = uiForeColor;
	g_uiDestHalfForeColor = uiHalfForeColor;
	g_uiDestBackColor = uiBackColor;
	g_uiDestHalfBackColor = uiHalfBackColor;

	return TCC_FONT_SUCCESS;
}

int tcc_fontDraw_setFontLineEffectColor (unsigned int uiLineEffectColor)
{
	g_uiDestLineEffectColor = uiLineEffectColor;
	return TCC_FONT_SUCCESS;
}

int tcc_fontDraw_clearDestBuf(unsigned int uiColor)
{
	if( NULL == g_puiDestBuf ) {
		ERR_PRINT("Error g_puiDestBuf is NULL\n");
		return TCC_FONT_ERROR;
	}

	memset(g_puiDestBuf, uiColor, g_uiDestPitch*g_uiDestHeight);

	return TCC_FONT_SUCCESS;
}

void tcc_fontDraw_setDirection(unsigned int uiDirection)
{
	g_uiDirection = uiDirection;
}

int tcc_fontDraw_clearFontBG(unsigned int uiX, unsigned int uiY, unsigned int uiWidth, unsigned int uiHeight)
{
	unsigned int* puiDestBuf = g_puiDestBuf;
	unsigned int uiDestWidth = g_uiDestWidth;
	unsigned int uiDestHeight = g_uiDestHeight;
	unsigned int uiDestPitch = g_uiDestPitch;
	unsigned int uiDestBGColor = g_uiDestBackColor;

	unsigned int* puiDst;

	int i, j;

	if( 0 == uiWidth || 0 == uiHeight)
	{
		ERR_PRINT("Error Invalid Param(W:%d, H:%d)\n", uiWidth, uiHeight);
		return TCC_FONT_ERROR_INVALID_PARAM;
	}

	//Rotate 0
	for(i=0; i<uiHeight; i++)
	{				
		puiDst = puiDestBuf + ((i + uiY) * uiDestPitch) + uiX;	

		for(j=0; j<uiWidth; j++)
		{
			*puiDst++ = uiDestBGColor;	
		}
	}

	return TCC_FONT_SUCCESS;
}

int tcc_fontDraw_drawHighlighting(unsigned int uiX, unsigned int uiY, unsigned int uiWidth, unsigned int uiHeight, unsigned int Type)
{
	unsigned int* puiDestBuf = g_puiDestBuf;
	unsigned int uiDestWidth = g_uiDestWidth;
	unsigned int uiDestHeight = g_uiDestHeight;
	unsigned int uiDestPitch = g_uiDestPitch;
	unsigned int uiDestFTColor = g_uiDestLineEffectColor;
	unsigned int uiDestBGColor = g_uiDestBackColor;

	int i;

	if( 0 == uiWidth || 0 == uiHeight ){
		ERR_PRINT("Error Invalid Param(W:%d, H:%d)\n", uiWidth, uiHeight);
		return TCC_FONT_ERROR_INVALID_PARAM;
	}

	if (g_uiDirection == TCC_FONT_DIRECTION_HORIZONTAL) {
		if( Type & 0x1 ){ 
			puiDestBuf  = g_puiDestBuf + ((uiY + (uiHeight - TCC_FONT_HIGHLIGHT_CORRECTION)) * uiDestPitch) + uiX;
			for(i=0; i<uiWidth; i++)
			{
				*puiDestBuf++ = uiDestFTColor;
			}
		}

		if( Type & 0x4 ){
			puiDestBuf  = g_puiDestBuf + (uiY * uiDestPitch) + uiX;
			for(i=0; i<uiWidth; i++)
			{
				*puiDestBuf++ = uiDestFTColor;
			}
		}

		if( Type & 0x8 ){
			puiDestBuf  = g_puiDestBuf + (uiY * uiDestPitch) + uiX;	
			for(i=0; i<(uiHeight - TCC_FONT_HIGHLIGHT_CORRECTION); i++)
			{
				*puiDestBuf = uiDestFTColor;	
				puiDestBuf += uiDestPitch;
			}
		}

		if( Type & 0x2 ){
			puiDestBuf  = g_puiDestBuf + (uiY * uiDestPitch) + uiX + (uiWidth - TCC_FONT_HIGHLIGHT_CORRECTION);	
			for(i=0; i<TCC_FONT_HIGHLIGHT_CORRECTION; i++)
			{
				*puiDestBuf++ = uiDestBGColor;	
			}

			puiDestBuf  = g_puiDestBuf + ((uiY + (uiHeight - TCC_FONT_HIGHLIGHT_CORRECTION)) * uiDestPitch) + uiX + (uiWidth - TCC_FONT_HIGHLIGHT_CORRECTION);	
			for(i=0; i<TCC_FONT_HIGHLIGHT_CORRECTION; i++)
			{
				*puiDestBuf++ = uiDestBGColor;	
			}

			puiDestBuf  = g_puiDestBuf + (uiY * uiDestPitch) + uiX + (uiWidth - TCC_FONT_HIGHLIGHT_CORRECTION);	
			for(i=0; i<=(uiHeight - TCC_FONT_HIGHLIGHT_CORRECTION); i++)
			{
				*puiDestBuf = uiDestFTColor;	
				puiDestBuf += uiDestPitch;
			}
		}
	}else {
		if (Type & 1) {
			puiDestBuf  = g_puiDestBuf + (uiY * uiDestPitch) + uiX;	
			for(i=0; i<uiHeight; i++)
			{
				*puiDestBuf = uiDestFTColor;
				puiDestBuf += uiDestPitch;
			}		
		}
		if (Type & 4) {
			puiDestBuf =g_puiDestBuf + (uiY * uiDestPitch) + uiX + (uiWidth - TCC_FONT_HIGHLIGHT_CORRECTION);
			for(i=0; i < uiHeight; i++)
			{
				*puiDestBuf = uiDestFTColor;
				puiDestBuf += uiDestPitch;
			}
		}
		if (Type & 8) {
			puiDestBuf  = g_puiDestBuf + (uiY * uiDestPitch) + uiX;
			for(i=0; i<(uiWidth-TCC_FONT_HIGHLIGHT_CORRECTION); i++)
				*puiDestBuf++ = uiDestFTColor;
		}
		if (Type & 2) {
			puiDestBuf = g_puiDestBuf + (uiY + (uiHeight - TCC_FONT_HIGHLIGHT_CORRECTION)) * uiDestPitch + uiX;
			for(i=0; i < TCC_FONT_HIGHLIGHT_CORRECTION; i++)
			{
				*puiDestBuf = uiDestBGColor;
				puiDestBuf += uiDestPitch;
			}
			puiDestBuf = g_puiDestBuf + (uiY + (uiHeight - TCC_FONT_HIGHLIGHT_CORRECTION))*uiDestPitch + uiX + (uiWidth - TCC_FONT_HIGHLIGHT_CORRECTION);
			for(i=0; i < TCC_FONT_HIGHLIGHT_CORRECTION; i++)
			{
				*puiDestBuf = uiDestBGColor;
				puiDestBuf += uiDestPitch;
			}
			puiDestBuf  = g_puiDestBuf + ((uiY + (uiHeight - TCC_FONT_HIGHLIGHT_CORRECTION)) * uiDestPitch) + uiX;	
			for(i=0; i<=(uiWidth - TCC_FONT_HIGHLIGHT_CORRECTION); i++)
				*puiDestBuf++ = uiDestFTColor;	
		}
	}

	return TCC_FONT_SUCCESS;
}

int tcc_fontDraw_drawUnderline(unsigned int uiX, unsigned int uiY, unsigned int uiWidth, unsigned int uiHeight)
{
	unsigned int* puiDestBuf = g_puiDestBuf;
	unsigned int uiDestWidth = g_uiDestWidth;
	unsigned int uiDestHeight = g_uiDestHeight;
	unsigned int uiDestPitch = g_uiDestPitch;
	unsigned int uiDestFTColor = g_uiDestLineEffectColor;

	int i;

	if( 0 == uiWidth || 0 == uiHeight)
	{
		ERR_PRINT("Error Invalid Param(W:%d, H:%d)\n", uiWidth, uiHeight);
		return TCC_FONT_ERROR_INVALID_PARAM;
	}

	if (g_uiDirection == TCC_FONT_DIRECTION_HORIZONTAL) {
		puiDestBuf  = g_puiDestBuf + (((uiHeight - 1) + uiY) * uiDestPitch) + uiX;	
		for(i=0; i<uiWidth; i++)
		{
			*puiDestBuf++ = uiDestFTColor;	
		}
	} else {
		puiDestBuf =g_puiDestBuf + uiX + (uiWidth-1) + uiY * uiDestPitch;
		for(i=0; i < uiHeight; i++)
		{
			*puiDestBuf = uiDestFTColor;
			puiDestBuf += uiDestPitch;
		}
	}

	return TCC_FONT_SUCCESS;
}

int tcc_fontDraw_drawBitmapGrey256(unsigned int uiX, unsigned int uiY, unsigned int uiWidth, unsigned int uiHeight, unsigned int uiPitch, unsigned char* pBuf)
{
	unsigned char* pucSrcBuf = pBuf;
	unsigned int* puiDestBuf = g_puiDestBuf;
	unsigned int uiDestWidth = g_uiDestWidth;
	unsigned int uiDestHeight = g_uiDestHeight;
	unsigned int uiDestPitch = g_uiDestPitch;
	unsigned int uiDestFTColor = g_uiDestForeColor;
	unsigned int uiDestBGColor = g_uiDestBackColor;

	unsigned char* pucSrc;
	unsigned char ucTemp;
	unsigned int* puiDst;
	
	int i, j, k;

	unsigned int uiAlpha;
/*
	if( NULL == pBuf || 0 == uiWidth || 0 == uiHeight || 0 == uiPitch ) {
		ERR_PRINT("Error Invalid Param(pBuf:0x%08X, W:%d, H:%d, P:%d)\n", pBuf, uiWidth, uiHeight, uiPitch);
		return TCC_FONT_ERROR_INVALID_PARAM;
	}
*/
	if( NULL == puiDestBuf || 0 == uiDestWidth || 0 == uiDestHeight || 0 == uiDestPitch ) {
		ERR_PRINT("Error Invalid Area(puiDestBuf:0x%08X, W:%d, H:%d, P:%d)\n", puiDestBuf, uiDestWidth, uiDestHeight, uiDestPitch);
		return TCC_FONT_ERROR_INVALID_AREA;
	}
	
	if( uiX + uiWidth > uiDestWidth ) {
		ERR_PRINT("Error Invalid Area(X:%d, W:%d, DestW:%d)\n", uiX, uiWidth, uiDestWidth);
		return TCC_FONT_ERROR_LINE_FULL;
	}

	//Rotate 0
	for(i=0; i<uiHeight; i++)
	{				
		pucSrc = pucSrcBuf + (i * uiPitch);
		puiDst = puiDestBuf + ((i + uiY) * uiDestPitch) + uiX;	

		for(j=0, k=0; j<uiWidth; j++, k++)
		{
			uiAlpha = (unsigned int)(*pucSrc++);
			if( uiAlpha ) {
				if( uiAlpha < 255 ) {
					*puiDst = ((255 - uiAlpha)<<24) | 0x00000000; 
				}
				else {
					*puiDst = uiDestFTColor;
				}
			}

			puiDst++;
		}
	}

	return TCC_FONT_SUCCESS;
}

int tcc_fontDraw_drawBitmapMono(unsigned int uiX, unsigned int uiY, unsigned int uiWidth, unsigned int uiHeight, unsigned int uiPitch, unsigned char* pBuf)
{
	unsigned char* pucSrcBuf = pBuf;
	unsigned int* puiDestBuf = g_puiDestBuf;
	unsigned int uiDestWidth = g_uiDestWidth;
	unsigned int uiDestHeight = g_uiDestHeight;
	unsigned int uiDestPitch = g_uiDestPitch;
	unsigned int uiDestFTColor = g_uiDestForeColor;
	unsigned int uiDestBGColor = g_uiDestBackColor;

	unsigned char* pucSrc, *pucSrcTmp;
	unsigned char ucTemp;
	unsigned int* puiDst, *puiDstTmp;

	int i, j, k;

	//ALOGE("[%d, %d, %d, %d, %d]\n", uiX, uiY, uiWidth, uiHeight, uiPitch);

/*
	if( NULL == pBuf || 0 == uiWidth || 0 == uiHeight || 0 == uiPitch ) {
		ERR_PRINT("Error Invalid Param(pBuf:0x%08X, W:%d, H:%d, P:%d)\n", pBuf, uiWidth, uiHeight, uiPitch);
		return TCC_FONT_ERROR_INVALID_PARAM;
	}
*/
	if( NULL == puiDestBuf || 0 == uiDestWidth || 0 == uiDestHeight || 0 == uiDestPitch ) {
		ERR_PRINT("Error Invalid Area(puiDestBuf:0x%08X, W:%d, H:%d, P:%d)\n", puiDestBuf, uiDestWidth, uiDestHeight, uiDestPitch);
		return TCC_FONT_ERROR_INVALID_AREA;
	}
	
	if( uiX + uiWidth > uiDestWidth ) {
		ERR_PRINT("Error Invalid Area(X:%d, W:%d, DestW:%d)\n", uiX, uiWidth, uiDestWidth);
		return TCC_FONT_ERROR_LINE_FULL;
	}

	//Rotate 0
#if 0
	for(i=0; i<uiHeight; i++)
	{				
		pucSrc = pucSrcBuf + (i * uiPitch);
		puiDst = puiDestBuf + ((i + uiY) * uiDestPitch) + uiX;	

		for(j=0, k=0; j<uiWidth; j++, k++)
		{
			if(k%8 == 0)
			{
				ucTemp = *pucSrc++;
				k = 0;
			}

			if(ucTemp & (0x80>>k))
			{								
				*puiDst = uiDestFTColor;	
			}

			puiDst++;
		}
	}
#else
	pucSrc = pucSrcBuf;
	puiDst = puiDestBuf + uiX + (uiY * uiDestPitch);
	for(i=0; i<uiHeight; i++)
	{
		puiDstTmp = puiDst;
		pucSrcTmp = pucSrc;
		k = 0x80;
		ucTemp = *pucSrcTmp++;
		for(j=0; j<uiWidth; j++)
		{
			if(k == 0) {
				ucTemp = *pucSrcTmp++;
				k = 0x80;
			}

			if(ucTemp & k) {
				*puiDstTmp = uiDestFTColor;
			}
			k >>= 1;

			puiDstTmp++;
		}
		pucSrc += uiPitch;
		puiDst += uiDestPitch;
	}
#endif
	return TCC_FONT_SUCCESS;
}

int tcc_fontDraw_drawBitmapDRCS(TCCFONTDRCSMODE Mode, unsigned int uiX, unsigned int uiY, unsigned int uiWidth, unsigned int uiHeight, unsigned int uiPitch, unsigned char* pBuf)
{
	unsigned char* pucSrcBuf = pBuf;
	unsigned int* puiDestBuf = g_puiDestBuf;
	unsigned int uiDestWidth = g_uiDestWidth;
	unsigned int uiDestHeight = g_uiDestHeight;
	unsigned int uiDestPitch = g_uiDestPitch;
	unsigned int uiDestForeColor = g_uiDestForeColor;
	unsigned int uiDestHalfForeColor = g_uiDestHalfForeColor;
	unsigned int uiDestBackColor = g_uiDestBackColor;
	unsigned int uiDestHalfBackColor = g_uiDestHalfBackColor;

	unsigned char* pucSrc;
	unsigned char ucTemp;
	unsigned int* puiDst;

	int i, j, k;

	if( Mode > TCC_FONT_DRCS_MULTI_GRADATION ){
		ERR_PRINT("Error Invalid Param(Mode:%d)\n", Mode);
		return TCC_FONT_ERROR_INVALID_PARAM;
	}
/*
	if( NULL == pBuf || 0 == uiWidth || 0 == uiHeight || 0 == uiPitch ) {
		ERR_PRINT("Error Invalid Param(pBuf:0x%08X, W:%d, H:%d, P:%d)\n", pBuf, uiWidth, uiHeight, uiPitch);
		return TCC_FONT_ERROR_INVALID_PARAM;
	}
*/
	if( NULL == puiDestBuf || 0 == uiDestWidth || 0 == uiDestHeight || 0 == uiDestPitch ) {
		ERR_PRINT("Error Invalid Area(puiDestBuf:0x%08X, W:%d, H:%d, P:%d)\n", puiDestBuf, uiDestWidth, uiDestHeight, uiDestPitch);
		return TCC_FONT_ERROR_INVALID_AREA;
	}
	
	if( uiX + uiWidth > uiDestWidth ) {
		ERR_PRINT("Error Invalid Area(X:%d, W:%d, DestW:%d)\n", uiX, uiWidth, uiDestWidth);
		return TCC_FONT_ERROR_LINE_FULL;
	}

	if( Mode == TCC_FONT_DRCS_2_GRADATION ) {
		pucSrc = pucSrcBuf;
		for(i=0, k=0; i<uiHeight; i++)
		{				
			puiDst = puiDestBuf + ((i + uiY) * uiDestPitch) + uiX;	

			for(j=0; j<uiWidth; j++, k+=1)
			{
				if(k%8 == 0)
				{
					ucTemp = *pucSrc++;
					k = 0;
				}

				if(ucTemp & (0x80>>k))
				{								
					*puiDst = uiDestForeColor;	
				}

				puiDst++;
			}
		}
	}
	else if( Mode == TCC_FONT_DRCS_MULTI_GRADATION ) {
		pucSrc = pucSrcBuf;
		for(i=0, k=0; i<uiHeight; i++)
		{				
			puiDst = puiDestBuf + ((i + uiY) * uiDestPitch) + uiX;	

			for(j=0; j<uiWidth; j++, k+=2)
			{
				if(k%8 == 0)
				{
					ucTemp = *pucSrc++;
					k = 0;
				}

				switch (ucTemp & (0xC0>>k))
				{	
					case 0xC0: case 0x30: case 0x0C: case 0x03:
						*puiDst = uiDestForeColor;	
					break;

					case 0x80: case 0x20: case 0x08: case 0x02:
						*puiDst = uiDestHalfForeColor;	
					break;
				
					case 0x40: case 0x10: case 0x04: case 0x01:
						*puiDst = uiDestHalfBackColor;	
					break;

				}

				puiDst++;
			}
		}
	}

	return TCC_FONT_SUCCESS;
}

#if 1
#define GETA(c)				(((c) & 0xFF000000) >> 24)
#define GETR(c)				(((c) & 0xFF0000) >> 16)
#define GETG(c)				(((c) & 0xFF00) >> 8)
#define GETB(c)				((c) & 0xFF)

#define MAKECOL(a, r, g, b)	(((a)<<24)|((r)<<16)|((g)<<8)|(b))
#else
#define GETA(c)				(((c) & 0xF000) >> 12)
#define GETR(c)				(((c) & 0xF00) >> 8)
#define GETG(c)				(((c) & 0xF0) >> 4)
#define GETB(c)				((c) & 0xF)

#define MAKECOL(a, r, g, b)	(((a)<<12)|((r)<<8)|((g)<<4)|(b))
#endif /* 0 */

int tcc_fontDraw_scaleBitmapDRCS(unsigned long* srcBuf, int src_w, int src_h, unsigned long* dstBuf, int dst_x, int dst_y, int dst_w, int dst_h, int dst_p)
{
	register int i, j;
#if 0
	register double src_dest_w = (double)(src_w - 1) / (double)(dst_w - 1);
	register double src_dest_h = (double)(src_h - 1) / (double)(dst_h - 1);
#else
	//on an experimental basis, it's to prevent 1seg DRCS from beong clipped out at right and bottom side
	register double src_dest_w = (double)(src_w)  / (double)(dst_w);
	register double src_dest_h = (double)(src_h) / (double)(dst_h);
#endif
	register double ci, cj;
	register int x1, y1, x2, y2;
	register double xoff, yoff;
	register unsigned c1_rgb[4], c2_rgb[4], c3_rgb[4], c4_rgb[4];
	register unsigned a, r, g, b;

	register long *line1, *line2;
	register long *dest_line;

	//ALOGD("[%d, %d, %d, %d, %d, %d, %d]\n", src_w, src_h, dst_x, dst_y, dst_w, dst_h, dst_p);

	cj = 0;
	for (j = 0; j < dst_h; j++)
	{
		y1 = cj;
		y2 = y1 < src_w - 1 ? y1 + 1 : src_w - 1;
		yoff = cj - y1;

		line1 = (long *)srcBuf + (src_w * y1);
		line2 = (long *)srcBuf + (src_w * y2);
		dest_line = (long *)dstBuf + (dst_p * (j + dst_y)) + dst_x;

		ci = 0;
		for (i = 0; i < dst_w; i++)
		{
			x1 = ci;
			x2 = x1 < src_w - 1 ? x1 + 1 : src_w - 1;

			c1_rgb[0] = GETA(line1[x1]);
			c1_rgb[1] = GETR(line1[x1]);
			c1_rgb[2] = GETG(line1[x1]);
			c1_rgb[3] = GETB(line1[x1]);

			c2_rgb[0] = GETA(line1[x2]);
			c2_rgb[1] = GETR(line1[x2]);
			c2_rgb[2] = GETG(line1[x2]);
			c2_rgb[3] = GETB(line1[x2]);

			c3_rgb[0] = GETA(line2[x1]);
			c3_rgb[1] = GETR(line2[x1]);
			c3_rgb[2] = GETG(line2[x1]);
			c3_rgb[3] = GETB(line2[x1]);

			c4_rgb[0] = GETA(line2[x2]);
			c4_rgb[1] = GETR(line2[x2]);
			c4_rgb[2] = GETG(line2[x2]);
			c4_rgb[3] = GETB(line2[x2]);

			xoff = ci - x1;

#if 0
			a = (c1_rgb[0] * (1 - xoff) + (c2_rgb[0] * xoff)) * (1 - yoff) + (c3_rgb[0] * (1 - xoff) + (c4_rgb[0] * xoff)) * yoff;
			r = (c1_rgb[1] * (1 - xoff) + (c2_rgb[1] * xoff)) * (1 - yoff) + (c3_rgb[1] * (1 - xoff) + (c4_rgb[1] * xoff)) * yoff;
			g = (c1_rgb[2] * (1 - xoff) + (c2_rgb[2] * xoff)) * (1 - yoff) + (c3_rgb[2] * (1 - xoff) + (c4_rgb[2] * xoff)) * yoff;
			b = (c1_rgb[3] * (1 - xoff) + (c2_rgb[3] * xoff)) * (1 - yoff) + (c3_rgb[3] * (1 - xoff) + (c4_rgb[3] * xoff)) * yoff;
#elif 0
			a = (c1_rgb[0] + c2_rgb[0] + c3_rgb[0] + c4_rgb[0])>>2;
			r = (c1_rgb[1] + c2_rgb[1] + c3_rgb[1] + c4_rgb[1])>>2;
			g = (c1_rgb[2] + c2_rgb[2] + c3_rgb[2] + c4_rgb[2])>>2;
			b = (c1_rgb[3] + c2_rgb[3] + c3_rgb[3] + c4_rgb[3])>>2;
#elif 0
			a = (c1_rgb[0] * (1 - yoff)) + c3_rgb[0] * yoff;
			r = (c1_rgb[1] * (1 - yoff)) + c3_rgb[1] * yoff;
			g = (c1_rgb[2] * (1 - yoff)) + c3_rgb[2] * yoff;
			b = (c1_rgb[3] * (1 - yoff)) + c3_rgb[3] * yoff;
#else
			a = c1_rgb[0];
			r = c1_rgb[1];
			g = c1_rgb[2];
			b = c1_rgb[3];
#endif    /* End of 0 */

			dest_line[i] = MAKECOL(a, r, g, b);

			ci += src_dest_w;
		}
		cj += src_dest_h;
	}

	return TCC_FONT_SUCCESS;
}
