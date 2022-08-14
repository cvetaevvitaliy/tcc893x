/*
 * tcc_fontDraw.h
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

#ifndef _TCC_FONTDRAW_H_
#define _TCC_FONTDRAW_H_

int tcc_fontDraw_init(void);
int tcc_fontDraw_close(void);
int tcc_fontDraw_setDestBuf(unsigned char *pBuf);
int tcc_fontDraw_setDestRegion(unsigned int uiWidth, unsigned int uiHeight, unsigned int uiPitch);
int tcc_fontDraw_setFontColor(unsigned int uiForeColor, unsigned int uiHalfForeColor, unsigned int uiBackColor, unsigned int uiHalfBackColor);
int tcc_fontDraw_setFontLineEffectColor (unsigned int uiLineEffectColor);

unsigned int* tcc_fontDraw_getDestBuf(void);
int tcc_fontDraw_getDestRegion(unsigned int* puiWidth, unsigned int* puiHeight, unsigned int *puiPitch);

int tcc_fontDraw_clearDestBuf(unsigned int uiColor);
int tcc_fontDraw_clearFontBG(unsigned int uiX, unsigned int uiY, unsigned int uiWidth, unsigned int uiHeight);
int tcc_fontDraw_drawHighlighting(unsigned int uiX, unsigned int uiY, unsigned int uiWidth, unsigned int uiHeight, unsigned int Type);
int tcc_fontDraw_drawUnderline(unsigned int uiX, unsigned int uiY, unsigned int uiWidth, unsigned int uiHeight);
int tcc_fontDraw_drawBitmapGrey256(unsigned int uiX, unsigned int uiY, unsigned int uiWidth, unsigned int uiHeight, unsigned int uiPitch, unsigned char* pBuf);
int tcc_fontDraw_drawBitmapMono(unsigned int uiX, unsigned int uiY, unsigned int uiWidth, unsigned int uiHeight, unsigned int uiPitch, unsigned char* pBuf);
int tcc_fontDraw_drawBitmapDRCS(TCCFONTDRCSMODE Mode, unsigned int uiX, unsigned int uiY, unsigned int uiWidth, unsigned int uiHeight, unsigned int uiPitch, unsigned char* pBuf);
int tcc_fontDraw_scaleBitmapDRCS(unsigned long* srcBuf, int src_w, int src_h, unsigned long* dstBuf, int dst_x, int dst_y, int dst_w, int dst_h, int dst_p);
void tcc_fontDraw_setDirection(unsigned int uiDirection);

#endif //_TCC_FONTDRAW_H_
