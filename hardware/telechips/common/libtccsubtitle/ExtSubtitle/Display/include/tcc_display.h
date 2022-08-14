/*
 * tcc_display.h
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

#ifndef _TCC_DISPLAY_H_
#define _TCC_DISPLAY_H_

#define TCC_DISPLAY_SUCCESS                      0
#define TCC_DISPLAY_ERROR                       (-1)
#define TCC_DISPLAY_ERROR_INVALID_PARAM         (-2)
#define TCC_DISPLAY_ERROR_INVALID_AREA          (-3)
#define TCC_DISPLAY_ERROR_MEMORY				(-4)
#define TCC_DISPLAY_ERROR_CANNOT_FIND_BITMAP    (-5)
#define TCC_DISPLAY_ERROR_LINE_FULL             (-6)
#define TCC_DISPLAY_ERROR_DRIVER_OPEN           (-7)

#define TCC_DISPLAY_DRIVER_CCFB                 "/dev/ccfb"

#define TCC_DISPLAY_FONT_COLOR                  (0x8410)
#define TCC_DISPLAY_FONT_BG_COLOR               (0x0000)

#define TCC_DISPLAY_BUFFER_NAME                 "overlay1"

#ifdef __cplusplus
extern "C" {
#endif

int tcc_display_open(void);
int tcc_display_close(void);
int tcc_display_setOnoff(int iOnoff);
int tcc_display_setFontSize(int iFontSize);
int tcc_display_setPosition(int iPositionY);
int tcc_display_displayString(unsigned short *pusString, int iStringLength);
int tcc_display_clearString(void);

#ifdef __cplusplus
}
#endif

#endif //_TCC_DISPLAY_H_
