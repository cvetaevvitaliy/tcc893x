/*
 * tcc_font.c
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
#include <string.h>

#include "tcc_font.h" 
#include "tcc_fontFT.h"
#include "tcc_fontDraw.h"

/**************************************************************************
							DEFINE
**************************************************************************/
#if 1
	#define ERR_PRINT			ALOGE
	#define DBG_PRINT			ALOGD
#else
	#define ERR_PRINT			
	#define DBG_PRINT
#endif

/**************************************************************************
							VARIABLE DEFINE
**************************************************************************/
static unsigned int g_uiDirection;
static unsigned int g_uiBasePosition;
static unsigned int g_uiFontSize;
static unsigned int g_uiFontWidth;
static unsigned int g_uiFontHeight;
static unsigned int g_uiHoriSpace;
static unsigned int g_uiVertSpace;
static unsigned int g_uiFontTotalWidth;
static unsigned int g_uiFontTotalHeight;

static unsigned int *g_pDRCSScaleBuf = NULL;
/**************************************************************************
							FUNCTION DEFINE
**************************************************************************/
static void tcc_font_calcPosition(unsigned int uiX, unsigned int uiY, unsigned int* puiPosX, unsigned int* puiPosY)
{
	unsigned int uiPosX, uiPosY;

	uiPosX = uiX;
	uiPosY = uiY;

	switch(g_uiBasePosition)
	{
		case TCC_FONT_BASE_POSITION_TOP_LEFT:
			//Nothing
		break;

		case TCC_FONT_BASE_POSITION_TOP_MIDDLE:
			if( uiPosX > (g_uiFontTotalWidth / 2) ) {
				uiPosX -= (g_uiFontTotalWidth / 2);
			}
			else {
				uiPosX = 0;
			}
		break;

		case TCC_FONT_BASE_POSITION_BOTTOM:
			if( uiPosY > g_uiFontTotalHeight ) {
				uiPosY -= g_uiFontTotalHeight;
			}
			else {
				uiPosY = 0;
			}
		break;

		default:
		break;
	}

	*puiPosX = uiPosX;
	*puiPosY = uiPosY;

	return;
}

int tcc_font_init(void)
{
	int err;
	
	err = tcc_fontFT_init();
	if( err !=  TCC_FONT_SUCCESS) {
		ERR_PRINT("Error tcc_fontFT_init(%d)\n", err);
		return TCC_FONT_ERROR;
	}
	
	err = tcc_fontDraw_init();
	if( err !=  TCC_FONT_SUCCESS) {
		ERR_PRINT("Error tcc_fontDraw_init(%d)\n", err);
		return TCC_FONT_ERROR;
	}
	
	g_pDRCSScaleBuf = (unsigned int*)malloc(TCC_FONT_SCALE_BUF_SIZE);
	if( g_pDRCSScaleBuf == NULL ) {
		ERR_PRINT("Error g_pDRCSScaleBuf is NULL\n");
	}
	
	return TCC_FONT_SUCCESS;
}

int tcc_font_close(void)
{
	int err;
	int ret;
	
	ret = TCC_FONT_SUCCESS;

	if( g_pDRCSScaleBuf != NULL ) {
		free((void*)g_pDRCSScaleBuf);
		g_pDRCSScaleBuf = NULL;
	}
	
	err = tcc_fontDraw_close();
	if( err !=  TCC_FONT_SUCCESS) {
		ERR_PRINT("Error tcc_fontDraw_close(%d)\n", err);
		ret = TCC_FONT_ERROR;
	}		

	err = tcc_fontFT_close();
	if( err !=  TCC_FONT_SUCCESS) {
		ERR_PRINT("Error tcc_fontFT_close(%d)\n", err);
		ret = TCC_FONT_ERROR;
	}		

	return ret;
}

int tcc_font_setDestBuf(unsigned char *pBuf)
{
	return tcc_fontDraw_setDestBuf(pBuf);
}

int tcc_font_setDestRegion(unsigned int uiWidth, unsigned int uiHeight, unsigned int uiPitch)
{
	return tcc_fontDraw_setDestRegion(uiWidth, uiHeight, uiPitch);
}

int tcc_font_clearDestBuf(unsigned int uiColor)
{
	return tcc_fontDraw_clearDestBuf(uiColor);
}

int tcc_font_setDirection(TCCFONTDIRECTION Direction, TCCFONTBASEPOSITION BasePosition)
{
	g_uiDirection = Direction;
	g_uiBasePosition = BasePosition;

	tcc_fontDraw_setDirection((unsigned int)Direction);

	return TCC_FONT_SUCCESS;
}

int tcc_font_setFontSize(TCCFONTSIZE Size, unsigned int uiWidth, unsigned int uiHeight, unsigned int uiHoriSpace, unsigned int uiVertSpace)
{
	if( 0 == uiWidth || 0 == uiHeight ){
		ERR_PRINT("Err Invalid Param(uiWidth:%d, uiHeight:%d)\n", uiWidth, uiHeight);
		return TCC_FONT_ERROR_INVALID_PARAM;
	}

	switch(Size)
	{
		case TCC_FONT_SIZE_STANDARD:
			g_uiFontWidth = uiWidth;
			g_uiFontHeight = uiHeight;
			if( g_uiDirection == TCC_FONT_DIRECTION_HORIZONTAL ) {
				g_uiHoriSpace = uiHoriSpace/2;
				g_uiVertSpace = uiVertSpace/2;
			}
			else {
				g_uiHoriSpace = uiVertSpace/2;
				g_uiVertSpace = uiHoriSpace/2;
			}
			g_uiFontTotalWidth = g_uiFontWidth + uiHoriSpace;
			g_uiFontTotalHeight = g_uiFontHeight + uiVertSpace;
		break;
		
		case TCC_FONT_SIZE_MEDIUM:
			if( g_uiDirection == TCC_FONT_DIRECTION_HORIZONTAL ) {
				g_uiFontWidth = uiWidth/2;
				g_uiFontHeight = uiHeight;
				g_uiHoriSpace = uiHoriSpace/4;
				g_uiVertSpace = uiVertSpace/2;
			}
			else {
				g_uiFontWidth = uiWidth;
				g_uiFontHeight = uiHeight/2;
				g_uiHoriSpace = uiVertSpace/2;
				g_uiVertSpace = uiHoriSpace/4;
			}			
			g_uiFontTotalWidth = g_uiFontWidth + uiHoriSpace;
			g_uiFontTotalHeight = g_uiFontHeight + uiVertSpace;
		break;

		case TCC_FONT_SIZE_SMALL:
			g_uiFontWidth = uiWidth/2;
			g_uiFontHeight = uiHeight/2;
			if( g_uiDirection == TCC_FONT_DIRECTION_HORIZONTAL ) {
				g_uiHoriSpace = uiHoriSpace/4;
				g_uiVertSpace = uiVertSpace/4;
			}
			else {
				g_uiHoriSpace = uiVertSpace/4;
				g_uiVertSpace = uiHoriSpace/4;
			}
			g_uiFontTotalWidth = g_uiFontWidth + uiHoriSpace;
			g_uiFontTotalHeight = g_uiFontHeight + uiVertSpace;
		break;

		case TCC_FONT_SIZE_DOUBLE_HORIZONTAL:
			g_uiFontWidth = uiWidth*2;
			g_uiFontHeight = uiHeight;
			if( g_uiDirection == TCC_FONT_DIRECTION_HORIZONTAL ) {
				g_uiHoriSpace = uiHoriSpace;
				g_uiVertSpace = uiVertSpace/2;
			}
			else {
				g_uiHoriSpace = uiVertSpace;
				g_uiVertSpace = uiHoriSpace/2;
			}
			g_uiFontTotalWidth = g_uiFontWidth + uiHoriSpace;
			g_uiFontTotalHeight = g_uiFontHeight + uiVertSpace;
		break;

		case TCC_FONT_SIZE_DOUBLE_VERTICAL:
			g_uiFontWidth = uiWidth;
			g_uiFontHeight = uiHeight*2;
			if( g_uiDirection == TCC_FONT_DIRECTION_HORIZONTAL ) {
				g_uiHoriSpace = uiHoriSpace/2;
				g_uiVertSpace = uiVertSpace/2*3;
				g_uiFontTotalWidth = g_uiFontWidth + uiHoriSpace;
				g_uiFontTotalHeight = g_uiFontHeight + (g_uiVertSpace/3*4);
			}
			else {
				g_uiHoriSpace = uiVertSpace/2;
				g_uiVertSpace = uiHoriSpace;
				g_uiFontTotalWidth = g_uiFontWidth + uiHoriSpace;
				g_uiFontTotalHeight = g_uiFontHeight + uiVertSpace;
			}
		break;

		case TCC_FONT_SIZE_DOUBLE_HORIZONTAL_VERTICAL:
			g_uiFontWidth = uiWidth*2;
			g_uiFontHeight = uiHeight*2;
			if( g_uiDirection == TCC_FONT_DIRECTION_HORIZONTAL ) {
				g_uiHoriSpace = uiHoriSpace;
				g_uiVertSpace = uiVertSpace/2*3;
				g_uiFontTotalWidth = g_uiFontWidth + uiHoriSpace;
				g_uiFontTotalHeight = g_uiFontHeight + (g_uiVertSpace/3*4);
			}
			else {
				g_uiHoriSpace = uiVertSpace;
				g_uiVertSpace = uiHoriSpace;
				g_uiFontTotalWidth = g_uiFontWidth + uiHoriSpace;
				g_uiFontTotalHeight = g_uiFontHeight + uiVertSpace;
			}
		break;

		default:
			ERR_PRINT("Err Invalid Param(Size:%d)\n", Size);
			return TCC_FONT_ERROR_INVALID_PARAM;
		break;
	}

	g_uiFontSize = Size;

	return tcc_fontFT_setFontSize(g_uiFontWidth, g_uiFontHeight);
}

int tcc_font_setFontColor(unsigned int uiForeColor, unsigned int uiHalfForeColor, unsigned int uiBackColor, unsigned int uiHalfBackColor)
{
	return tcc_fontDraw_setFontColor(uiForeColor, uiHalfForeColor, uiBackColor, uiHalfBackColor);
}

int tcc_font_setFontLineEffectColor (unsigned int uiLineEffectColor)
{
	return tcc_fontDraw_setFontLineEffectColor (uiLineEffectColor);
}

int tcc_font_clearChar(TCCFONTTYPE Type, unsigned int uiX, unsigned int uiY)
{
	unsigned int uiPosX, uiPosY;
	int ret;

	tcc_font_calcPosition(uiX, uiY, &uiPosX, &uiPosY);

	ret = tcc_fontDraw_clearFontBG(uiPosX, uiPosY, g_uiFontTotalWidth, g_uiFontTotalHeight);
	if( ret ){
		ERR_PRINT("Err tcc_fontDraw_clearFontBG(%d)\n", ret);
	}

	if( g_uiDirection == TCC_FONT_DIRECTION_HORIZONTAL) {
		return g_uiFontWidth;
	}
	else {
		return g_uiFontHeight;
	}

	return TCC_FONT_ERROR;
}

int tcc_font_displayChar(TCCFONTTYPE Type, unsigned int uiNonSpaceChar, unsigned int uiChar, unsigned int uiX, unsigned int uiY)
{
	unsigned int uiPosX, uiPosY;
	int ret;

	tcc_font_calcPosition(uiX, uiY, &uiPosX, &uiPosY);

	ret = tcc_fontDraw_clearFontBG(uiPosX, uiPosY, g_uiFontTotalWidth, g_uiFontTotalHeight);
	if( ret ){
		ERR_PRINT("Err tcc_fontDraw_clearFontBG(%d)\n", ret);
	}

	if( uiNonSpaceChar ){
		ret = tcc_fontFT_drawChar(Type, uiNonSpaceChar, uiPosX+g_uiHoriSpace, uiPosY+g_uiVertSpace);
		if( ret < 0 ) {
			ERR_PRINT("Err tcc_fontFT_drawChar(non-space character %d)\n", ret);
		}
	}
	
	ret = tcc_fontFT_drawChar(Type, uiChar, uiPosX+g_uiHoriSpace, uiPosY+g_uiVertSpace);
	if( ret < 0 ) {
		ERR_PRINT("Err tcc_fontFT_drawChar(%d)\n", ret);
		return ret;
	}

	if( Type & TCC_FONT_TYPE_HIGHLIGHTING ) {
	#if 0
		ret = tcc_fontDraw_drawHighlighting(uiPosX, uiPosY+g_uiVertSpace, g_uiFontTotalWidth, g_uiFontHeight, Type);
	#else
		ret = tcc_fontDraw_drawHighlighting(uiPosX, uiPosY, g_uiFontTotalWidth, g_uiVertSpace + g_uiFontHeight, Type);
	#endif
		if( ret < 0 ) {
			ERR_PRINT("Err tcc_fontDraw_drawUnderline(%d)\n", ret);
		}
	}
	if( Type & TCC_FONT_TYPE_UNDERLINE ) {
		ret = tcc_fontDraw_drawUnderline(uiPosX, uiPosY, g_uiFontTotalWidth, g_uiVertSpace + g_uiFontHeight);
		if( ret < 0 ) {
			ERR_PRINT("Err tcc_fontDraw_drawUnderline(%d)\n", ret);
		}
	}
		
	if( g_uiDirection == TCC_FONT_DIRECTION_HORIZONTAL) {
		return g_uiFontWidth;
	}
	else {
		return g_uiFontHeight;
	}

	return TCC_FONT_ERROR;
}

int tcc_font_displayDRCS(TCCFONTDRCSMODE Mode, unsigned int uiX, unsigned int uiY, unsigned int uiBitmapWidth, unsigned int uiBitmapHeight, unsigned char* pBitmap)
{
	unsigned int uiPosX, uiPosY;
	int ret;

	tcc_font_calcPosition(uiX, uiY, &uiPosX, &uiPosY);

	ret = tcc_fontDraw_clearFontBG(uiPosX, uiPosY, g_uiFontTotalWidth, g_uiFontTotalHeight);
	if( ret ){
		ERR_PRINT("Err tcc_fontDraw_clearFontBG(%d)\n", ret);
	}

	uiPosX = uiPosX + g_uiHoriSpace;
	uiPosY = uiPosY + g_uiVertSpace;

	return tcc_fontDraw_drawBitmapDRCS(Mode, uiPosX, uiPosY, uiBitmapWidth, uiBitmapHeight, uiBitmapWidth, pBitmap);
}

int tcc_font_displayDRCSwithScale(TCCFONTDRCSMODE Mode, unsigned int uiX, unsigned int uiY, unsigned int uiWidth, unsigned int uiHeight, unsigned int uiBitmapWidth, unsigned int uiBitmapHeight, unsigned char* pBitmap)
{
	unsigned int uiPosX, uiPosY;
	unsigned int* puiDestBuf;
	unsigned int uiDestWidth, uiDestHeight, uiDestPitch;
	int ret;

	//Draw Bitmap
	puiDestBuf = tcc_fontDraw_getDestBuf();
	tcc_fontDraw_getDestRegion(&uiDestWidth, &uiDestHeight, &uiDestPitch);
	
	tcc_fontDraw_setDestBuf(g_pDRCSScaleBuf);
	tcc_fontDraw_setDestRegion(uiBitmapWidth, uiBitmapHeight, uiBitmapWidth);

	ret = tcc_fontDraw_clearFontBG(0, 0, uiBitmapWidth, uiBitmapHeight);
	if( ret ){
		ERR_PRINT("Err tcc_fontDraw_clearFontBG(%d)\n", ret);
	}

	ret = tcc_fontDraw_drawBitmapDRCS(Mode, 0, 0, uiBitmapWidth, uiBitmapHeight, uiBitmapWidth, pBitmap);
	if( ret ){
		ERR_PRINT("Err tcc_fontDraw_drawBitmapDRCS(%d)\n", ret);
		return ret;
	}

	tcc_fontDraw_setDestBuf(puiDestBuf);
	tcc_fontDraw_setDestRegion(uiDestWidth, uiDestHeight, uiDestPitch);

	//Scale
	tcc_font_calcPosition(uiX, uiY, &uiPosX, &uiPosY);

	ret = tcc_fontDraw_clearFontBG(uiPosX, uiPosY, g_uiFontTotalWidth, g_uiFontTotalHeight);
	if( ret ){
		ERR_PRINT("Err tcc_fontDraw_clearFontBG(%d)\n", ret);
	}

	uiPosX = uiPosX + g_uiHoriSpace;
	uiPosY = uiPosY + g_uiVertSpace;

	return tcc_fontDraw_scaleBitmapDRCS(g_pDRCSScaleBuf, uiBitmapWidth, uiBitmapHeight, puiDestBuf, uiPosX, uiPosY, uiWidth, uiHeight, uiDestPitch);
}

int tcc_font_DisplayString(unsigned short *pusString, unsigned int uiX, unsigned int uiY, unsigned int uiLineSpace)
{
	unsigned short* pusTemp;
	unsigned int uiPosX, uiPosY;
	unsigned int uiWidth, uiHeight, uiPitch;
	int iAdvanceX;
	int err;

	if( NULL == pusString ) {
		ERR_PRINT("Err Invalid Param(pusString:0)\n");
		return TCC_FONT_ERROR_INVALID_PARAM;
	}

	pusTemp = pusString;
	uiPosX = uiX;
	uiPosY = uiY;
	err = tcc_fontDraw_getDestRegion(&uiWidth, &uiHeight, &uiPitch);
	if( err ) {
		ERR_PRINT("Err Invalid Param(pusString:0)\n");
		return TCC_FONT_ERROR_INVALID_AREA;
	}

	while( *pusTemp )
	{
		iAdvanceX = tcc_fontFT_drawChar(TCC_FONT_TYPE_NONE, *pusTemp, uiPosX, uiPosY);
		if( iAdvanceX >= 0 ) {
			uiPosX += iAdvanceX;
			pusTemp++;
		}
		else if( iAdvanceX == TCC_FONT_ERROR_LINE_FULL ) {
			uiPosX = 0;
			uiPosY += uiLineSpace;
		}
	}

	return TCC_FONT_SUCCESS;
}

