/*
 * tcc_fontFT.c
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

#include <ft2build.h>
#include FT_FREETYPE_H

#include "tcc_font.h"
#include "tcc_fontFT.h"
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
static FT_Library	g_FTlib;
static FT_Face		g_FTface;

static unsigned int g_BaseLineOffset;

/**************************************************************************
							FUNCTION
**************************************************************************/
static void tcc_fontFT_PrintFaceInfo(FT_Face face)
{
	int i;
	
	DBG_PRINT("num_faces  = %ld\n", face->num_faces );
	DBG_PRINT("face_index = %ld\n", face->face_index );

	DBG_PRINT("face_flags = 0x%04X\n", face->face_flags );
	if( face->face_flags & FT_FACE_FLAG_SCALABLE ) DBG_PRINT(" FT_FACE_FLAG_SCALABLE    (0x%04X)\n",FT_FACE_FLAG_SCALABLE);
	if( face->face_flags & FT_FACE_FLAG_FIXED_SIZES ) DBG_PRINT(" FT_FACE_FLAG_FIXED_SIZES (0x%04X)\n",FT_FACE_FLAG_FIXED_SIZES);
	if( face->face_flags & FT_FACE_FLAG_FIXED_WIDTH ) DBG_PRINT(" FT_FACE_FLAG_FIXED_WIDTH (0x%04X)\n",FT_FACE_FLAG_FIXED_WIDTH);
	if( face->face_flags & FT_FACE_FLAG_SFNT ) DBG_PRINT(" FT_FACE_FLAG_SFNT        (0x%04X)\n",FT_FACE_FLAG_SFNT);
	if( face->face_flags & FT_FACE_FLAG_HORIZONTAL ) DBG_PRINT(" FT_FACE_FLAG_HORIZONTAL  (0x%04X)\n",FT_FACE_FLAG_HORIZONTAL);
	if( face->face_flags & FT_FACE_FLAG_VERTICAL ) DBG_PRINT(" FT_FACE_FLAG_VERTICAL    (0x%04X)\n",FT_FACE_FLAG_VERTICAL);
	if( face->face_flags & FT_FACE_FLAG_KERNING ) DBG_PRINT(" FT_FACE_FLAG_KERNING     (0x%04X)\n",FT_FACE_FLAG_KERNING);
	if( face->face_flags & FT_FACE_FLAG_FAST_GLYPHS ) DBG_PRINT(" FT_FACE_FLAG_FAST_GLYPHS (0x%04X)\n",FT_FACE_FLAG_FAST_GLYPHS);
	if( face->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS ) DBG_PRINT(" FT_FACE_FLAG_MULTIPLE_MASTERS (0x%04X)\n",FT_FACE_FLAG_MULTIPLE_MASTERS);
	if( face->face_flags & FT_FACE_FLAG_GLYPH_NAMES ) DBG_PRINT(" FT_FACE_FLAG_GLYPH_NAMES (0x%04X)\n",FT_FACE_FLAG_GLYPH_NAMES);
	if( face->face_flags & FT_FACE_FLAG_EXTERNAL_STREAM ) DBG_PRINT(" FT_FACE_FLAG_EXTERNAL_STREAM (0x%04X)\n",FT_FACE_FLAG_EXTERNAL_STREAM);
	if( face->face_flags & FT_FACE_FLAG_HINTER ) DBG_PRINT("	 FT_FACE_FLAG_HINTER      (0x%04X)\n",FT_FACE_FLAG_HINTER);

	DBG_PRINT("style_flags = 0x%X\n", face->style_flags );
	if( face->style_flags & FT_STYLE_FLAG_ITALIC ) DBG_PRINT(" FT_STYLE_FLAG_ITALIC (0x%X)\n",FT_STYLE_FLAG_ITALIC);
	if( face->style_flags & FT_STYLE_FLAG_BOLD ) DBG_PRINT(" FT_STYLE_FLAG_BOLD   (0x%X)\n",FT_STYLE_FLAG_BOLD);

	DBG_PRINT("num_glyphs  = %d\n", face->num_glyphs );
	DBG_PRINT("family_name = %s\n", face->family_name );
	DBG_PRINT("style_name  = %s\n", face->style_name );

	DBG_PRINT("num_fixed_sizes = %d\n", face->num_fixed_sizes );
	for( i=0; i<face->num_fixed_sizes; i++ )
	{
		FT_Bitmap_Size* pstBitmapSize = &face->available_sizes[i];
		DBG_PRINT(" available_sizes[%2d] = [ height=%2d, width=%2d, size=%d, x_ppem=%d, y_ppem=%d ]\n",
		                                      i, pstBitmapSize->height, pstBitmapSize->width, pstBitmapSize->size,
		                                      pstBitmapSize->x_ppem, pstBitmapSize->y_ppem );
	}

	DBG_PRINT("num_charmaps = %d\n", face->num_charmaps );
	DBG_PRINT("bbox = ( xMin=%d, xMax=%d, yMin=%d, yMax=%d )\n",
	                                face->bbox.xMin, face->bbox.xMax, face->bbox.yMin, face->bbox.yMax );
	DBG_PRINT("units_per_EM = %d\n", face->units_per_EM );
	DBG_PRINT("ascender = %d\n", face->ascender );
	DBG_PRINT("descender = %d\n", face->descender );
	DBG_PRINT("height = %d\n", face->height );
	DBG_PRINT("max_advance_width = %d\n", face->max_advance_width );
	DBG_PRINT("max_advance_height = %d\n", face->max_advance_height );
	DBG_PRINT("underline_position = %d\n", face->underline_position );
	DBG_PRINT("underline_thickness = %d\n", face->underline_thickness );
	
	return;	
}

static void tcc_fontFT_PrintGlyphSlotInfo(FT_GlyphSlot glyphslot)
{	
	DBG_PRINT("bitmap_top=%d, left=%d\n", glyphslot->bitmap_top, glyphslot->bitmap_left);
	DBG_PRINT("bitmap.rows=%d, width=%d, pitch=%d\n", glyphslot->bitmap.rows, glyphslot->bitmap.width, glyphslot->bitmap.pitch);
	DBG_PRINT("bitmap.pixel_mode=%d, num_grays=%d\n", glyphslot->bitmap.pixel_mode, glyphslot->bitmap.num_grays);

	DBG_PRINT("metrics.width=%d, height=%d\n", glyphslot->metrics.width, glyphslot->metrics.height);
	DBG_PRINT("metrics.hX=%d, hY=%d, hA=%d\n", glyphslot->metrics.horiBearingX, glyphslot->metrics.horiBearingY, glyphslot->metrics.horiAdvance);
	DBG_PRINT("metrics.vX=%d, vY=%d, vA=%d\n", glyphslot->metrics.vertBearingX, glyphslot->metrics.vertBearingY, glyphslot->metrics.vertAdvance);

	DBG_PRINT("advance.x=%d, y=%d", glyphslot->advance.x, glyphslot->advance.y);

	return;	
}

int tcc_fontFT_init(void)
{
	FT_Error err;

	g_FTlib = NULL;

	err = FT_Init_FreeType(&g_FTlib);
	if( err ) {
		ERR_PRINT("Error FT_Init_Freetype(%d)\n", err);
		return TCC_FONT_ERROR;
	}

	err= FT_New_Face(g_FTlib, TCC_FONT_FREETYPE_FILE_PATH, 0, &g_FTface);
	if( err ) {
		ERR_PRINT("Error FT_New_Face(%d)\n", err);
		return TCC_FONT_ERROR;
	}

	g_BaseLineOffset = 0;

	//tcc_fontFT_PrintFaceInfo(g_FTface);
	
	return TCC_FONT_SUCCESS;
}

int tcc_fontFT_close(void)
{
	FT_Error err;
	int ret;

	ret = TCC_FONT_SUCCESS;
	
	err = FT_Done_Face(g_FTface);
	if( err ) {
		ERR_PRINT("Error FT_Done_Face(%d)\n", err);
		ret = TCC_FONT_ERROR;
	}

	err = FT_Done_FreeType(g_FTlib);
	if( err ) {
		ERR_PRINT("Error FT_DoneFreeType(%d)\n", err);
		ret = TCC_FONT_ERROR;
	}

	return ret;
}

int tcc_fontFT_setFontSize(unsigned int uiWidth, unsigned int uiHeight)
{
	FT_BBox *pBBox;
	FT_Error err;

	if( 0 == uiWidth || 0 == uiHeight ) {
		ERR_PRINT("Error Invalid Param(%d, %d)\n", uiWidth, uiHeight);
		return TCC_FONT_ERROR_INVALID_PARAM;
	}

	err = FT_Set_Char_Size(g_FTface, uiWidth<<6, uiHeight<<6, uiWidth, uiHeight);
	if( err ) {
		ERR_PRINT("Error FT_Set_Char_Size(%d)\n", err);
		return TCC_FONT_ERROR;
	}

	err = FT_Set_Pixel_Sizes(g_FTface, uiWidth, uiHeight);
	if( err ) {
		ERR_PRINT("Error FT_Set_Pixel_Sizes(%d)\n", err);
		return TCC_FONT_ERROR;
	}

	pBBox = &g_FTface->bbox;
	if( pBBox->yMax == pBBox->yMin ) {
		ERR_PRINT("Error Invalid Bounding Box(%d, %d)\n", pBBox->yMax, pBBox->yMin);
		return TCC_FONT_ERROR;
	}

	g_BaseLineOffset = (((uiHeight*pBBox->yMax)<<1)/(pBBox->yMax-pBBox->yMin) +1) >> 1;

	return TCC_FONT_SUCCESS;
}

int tcc_FontFT_getRealFontSize(unsigned int uiChar, unsigned int* puiWidth, unsigned int* puiHeight)
{
	FT_GlyphSlot glyphslot;
	FT_UInt glyphindex;
	FT_Error err;

	glyphslot = g_FTface->glyph;
	
	glyphindex = FT_Get_Char_Index( g_FTface, uiChar);
	if( 0 == glyphindex ) {
		ERR_PRINT("Err FT_Get_Char_Index(%d)\n", glyphindex);
		return TCC_FONT_ERROR;
	}

	err = FT_Load_Glyph( g_FTface, glyphindex, FT_LOAD_DEFAULT|FT_LOAD_NO_BITMAP);
	if( err ) {
		ERR_PRINT("Err FT_Load_Glyph(%d)\n", err);
		return TCC_FONT_ERROR;
	}
	
 	*puiWidth = g_FTface->size->metrics.x_ppem;
  	*puiHeight = g_FTface->size->metrics.y_ppem; 

	return TCC_FONT_SUCCESS;
}

int tcc_fontFT_drawChar(unsigned int uiType, unsigned int uiChar, unsigned int uiX, unsigned int uiY)
{
	unsigned int x, y;
	unsigned int mode;
	FT_GlyphSlot glyphslot;
	FT_UInt glyphindex;
	FT_Bitmap bitmap;
	FT_Error err;

	glyphslot = g_FTface->glyph;

	glyphindex = FT_Get_Char_Index(g_FTface, uiChar);
	if( 0 == glyphindex ) {
		ERR_PRINT("Err FT_Get_Char_Index(%d 0x%04X)\n", glyphindex, uiChar);
	}

	err = FT_Load_Glyph(g_FTface, glyphindex, FT_LOAD_DEFAULT|FT_LOAD_NO_BITMAP);
	if( err ) {
		ERR_PRINT("Err FT_Render_Glyph(%d 0x%04X)\n", err, uiChar);
		return TCC_FONT_ERROR_CANNOT_FIND_BITMAP;
	}

	//if( uiType & TCC_FONT_TYPE_HIGHLIGHTING ) {
	//	FT_Outline_Embolden(&g_FTface->glyph->outline, (g_FTface->size->metrics.x_ppem*5/100)<<6);
	//}

	if( uiType & TCC_FONT_TYPE_OUTLINE ){
		mode = FT_RENDER_MODE_NORMAL;
	}
	else {
		mode = FT_RENDER_MODE_MONO;
	}
	err = FT_Render_Glyph(g_FTface->glyph, mode);
	if( err ) {
		ERR_PRINT("Err FT_Render_Glyph(%d 0x%04X)\n", err, uiChar);
		return TCC_FONT_ERROR_CANNOT_FIND_BITMAP;
	}

	//err = tcc_fontDraw_clearFontBG(uiX, uiY, g_FTface->size->metrics.x_ppem, g_FTface->size->metrics.y_ppem);
	//if( err ){
	//	ERR_PRINT("Err tcc_fontDraw_clearFontBG(%d)\n", err);
	//	return err;
	//}

 	x = uiX + glyphslot->bitmap_left;
	y = uiY;
	if( g_BaseLineOffset > glyphslot->bitmap_top ) {
		y += g_BaseLineOffset - glyphslot->bitmap_top;
	}

	bitmap = glyphslot->bitmap;

	if( bitmap.pixel_mode == FT_PIXEL_MODE_GRAY ) {
		err = tcc_fontDraw_drawBitmapGrey256( x, y, bitmap.width, bitmap.rows, bitmap.pitch, bitmap.buffer);
		if( err ) {
			ERR_PRINT("Err tcc_fontDraw_drawBitmapGrey256(%d)\n", err);
			return err;
		}
	}
	else if( bitmap.pixel_mode == FT_PIXEL_MODE_MONO ) {
		err = tcc_fontDraw_drawBitmapMono( x, y, bitmap.width, bitmap.rows, bitmap.pitch, bitmap.buffer);
		if( err ) {
			ERR_PRINT("Err tcc_fontDraw_drawBitmapMono(%d)\n", err);
			return err;
		}
	}
 
	//tcc_fontFT_PrintGlyphSlotInfo(glyphslot);
	
	return (glyphslot->advance.x>>6);
}


