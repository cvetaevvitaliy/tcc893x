/*
 * tcc_font.h
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

#ifndef _TCC_FONT_H_
#define _TCC_FONT_H_

#define TCC_FONT_SUCCESS								0
#define TCC_FONT_ERROR									(-1)
#define TCC_FONT_ERROR_INVALID_PARAM					(-2)
#define TCC_FONT_ERROR_INVALID_AREA						(-3)
#define TCC_FONT_ERROR_CANNOT_FIND_BITMAP				(-4)
#define TCC_FONT_ERROR_LINE_FULL						(-5)

#define TCC_FONT_SCALE_BUF_SIZE							20736

#define TCC_FONT_HIGHLIGHT_CORRECTION 					3

typedef enum
{
	TCC_FONT_DIRECTION_HORIZONTAL,
	TCC_FONT_DIRECTION_VERTICAL

} TCCFONTDIRECTION;

typedef enum
{
	TCC_FONT_BASE_POSITION_TOP_LEFT,
	TCC_FONT_BASE_POSITION_TOP_MIDDLE,
	TCC_FONT_BASE_POSITION_BOTTOM

} TCCFONTBASEPOSITION;

typedef enum
{
	TCC_FONT_SIZE_STANDARD,
	TCC_FONT_SIZE_MEDIUM,
	TCC_FONT_SIZE_SMALL,
	TCC_FONT_SIZE_DOUBLE_HORIZONTAL,
	TCC_FONT_SIZE_DOUBLE_VERTICAL,
	TCC_FONT_SIZE_DOUBLE_HORIZONTAL_VERTICAL

} TCCFONTSIZE;

typedef enum
{
	TCC_FONT_TYPE_NONE = 0x0,
	TCC_FONT_TYPE_HIGHLIGHTING = 0xF,
	TCC_FONT_TYPE_OUTLINE = 0x10,
	TCC_FONT_TYPE_UNDERLINE = 0x20,

} TCCFONTTYPE;

typedef enum
{
	TCC_FONT_DRCS_2_GRADATION,
	TCC_FONT_DRCS_MULTI_GRADATION,
	TCC_FONT_DRCS_2_COLOUR_WITH_COMPRESSION,
	TCC_FONT_DRCS_MULTI_COLOUR_WITH_COMPRESSION

} TCCFONTDRCSMODE;

int tcc_font_init(void);
int tcc_font_close(void);

int tcc_font_setDestBuf(unsigned char *pBuf);
int tcc_font_setDestRegion(unsigned int uiWidth, unsigned int uiHeight, unsigned int uiPitch);
int tcc_font_clearDestBuf(unsigned int uiColor);

int tcc_font_setDirection(TCCFONTDIRECTION Direction, TCCFONTBASEPOSITION BasePosition);

int tcc_font_setFontSize( TCCFONTSIZE Size, unsigned int uiWidth, unsigned int uiHeight, unsigned int uiHoriSpace, unsigned int uiVertSpace);
int tcc_font_setFontColor(unsigned int uiForeColor, unsigned int uiHalfForeColor, unsigned int uiBackColor, unsigned int uiHalfBackColor);
int tcc_font_setFontLineEffectColor (unsigned int uiLineEffectColor);

int tcc_font_clearChar(TCCFONTTYPE Type, unsigned int uiX, unsigned int uiY);
int tcc_font_displayChar(TCCFONTTYPE Type, unsigned int uiNonSpaceChar, unsigned int uiChar, unsigned int uiX, unsigned int uiY);

int tcc_font_displayDRCS(TCCFONTDRCSMODE Mode, unsigned int uiX, unsigned int uiY, unsigned int uiBitmapWidth, unsigned int uiBitmapHeight, unsigned char* pBitmap);
int tcc_font_displayDRCSwithScale(TCCFONTDRCSMODE Mode, unsigned int uiX, unsigned int uiY, unsigned int uiWidth, unsigned int uiHeight, unsigned int uiBitmapWidth, unsigned int uiBitmapHeight, unsigned char* pBitmap);

#endif //_TCC_FONT_H_
