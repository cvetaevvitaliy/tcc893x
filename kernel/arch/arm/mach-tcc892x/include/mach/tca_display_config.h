/****************************************************************************
 * FileName    : kernel/arch/arm/mach-tcc893x/include/mach/tca_display_config.h
 * Description : 
 *
 * Copyright (C) 2013 Telechips Inc.
 *
 * This program is free software; you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA 02111-1307 USA
 * ****************************************************************************/


#ifndef TCA_DISPLAY_CONFIG_H
#define TCA_DISPLAY_CONFIG_H

extern unsigned int tca_get_lcd_lcdc_num(void);
extern unsigned int tca_get_output_lcdc_num(void);

#if defined(CONFIG_TCC8925S_DISP_PORT_CHANGE)
extern unsigned int tca_fb_output_path_changed(unsigned int onoff);
#endif


#endif

