/*
 * linux/arch/arm/mach-tcc893x/include/mach/vioc_lut.h
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

#include <mach/reg_physical.h>

#ifndef __VIOC_LUT_H__
#define	__VIOC_LUT_H__

#define LUT_NORMAL 	0
#define LUT_INVET	1
#define LUT_BRIGHTNESS 2
#define LUT_DVI				3


/* Interface APIs */
extern void VIOC_LUT_Set_LutBackUp_Init(void);
extern void VIOC_LUT_Set_value(VIOC_LUT *pLUT, unsigned int select,  unsigned int *LUT_Value);
extern void VIOC_LUT_ReStore(VIOC_LUT *pLUT, unsigned int select);
extern void VIOC_LUT_Set (VIOC_LUT *pLUT, unsigned int select, unsigned int inv, unsigned int bright_value);
extern void VIOC_LUT_Plugin (VIOC_LUT *pLUT, unsigned int lut_sel, unsigned int plug_in);
extern void VIOC_LUT_Enable (VIOC_LUT *pLUT, unsigned int lut_sel, unsigned int enable);
extern void VIOC_LUT_SetMain(VIOC_LUT *pLUT, unsigned int sel, unsigned int plugin, unsigned int inv, unsigned int bright_value, unsigned int onoff);

extern void tcc_sw_hue_table_set(int v_sin, int v_cos, int saturation);
extern int tcc_sw_hue_table_opt(unsigned int src_phy_cbcr, unsigned int tar_phy_cbcr, unsigned int width, unsigned int height);

#endif
