/*
 * tcc_fontFT.h
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
 
#ifndef _TCC_FONTFT_H_
#define _TCC_FONTFT_H_

#define TCC_FONT_FREETYPE_FILE_PATH			"/system/fonts/DroidSansFallback_DxB.ttf"

int tcc_fontFT_init(void);
int tcc_fontFT_close(void);
int tcc_fontFT_setFontSize(unsigned int uiWidth, unsigned int uiHeight);
int tcc_FontFT_getRealFontSize(unsigned int uiChar, unsigned int* puiWidth, unsigned int* puiHeight);
int tcc_fontFT_drawChar(unsigned int uiType, unsigned int uiChar, unsigned int uiX, unsigned int uiY);

#endif //_TCC_FONTFT_H_
