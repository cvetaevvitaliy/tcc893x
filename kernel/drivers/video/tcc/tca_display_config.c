/*
 * linux/drivers/video/tcc/tca_disaplay_config.c
 *
 * Based on:    
 * Author:  <linux@telechips.com>
 * Created: Jan 11, 2012
 * Description: TCC LCD Controller Number define
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

#include <mach/tca_display_config.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <mach/reg_physical.h>
#include <mach/io.h>
#include <mach/globals.h>
#include <asm/system.h>

unsigned int tca_get_lcd_lcdc_num(void)
{
	#ifdef CONFIG_LCD_LCDC0_USE
		return 0;
	#else
		#if defined(CONFIG_ARCH_TCC892X) 
			#if defined(CONFIG_CHIP_TCC8925S)
				#if defined(CONFIG_STB_BOARD_DONGLE)
					return 1;
				#else
					return 0;
				#endif
			#elif defined(CONFIG_MACH_TCC8920ST)
					return 0;
			#else
				if(system_rev == 0x1006 || system_rev == 0x1008)
					return 0;
				else
					return 1;
			#endif
		#elif defined(CONFIG_ARCH_TCC893X)
			#if defined(CONFIG_MACH_TCC8930ST)
				return 1;
			#else
				#if defined(CONFIG_CHIP_TCC8930)
					return 1;
				#elif defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
					return 0;	
				#endif
			#endif
		#endif
	#endif

	//default LCDC number
	return 1;
}
//EXPORT_SYMBOL(tca_get_lcd_lcdc_num);

unsigned int tca_get_output_lcdc_num(void)
{
	#ifdef CONFIG_LCD_LCDC0_USE
		return 1;
	#else
		#if defined(CONFIG_ARCH_TCC892X)
			#if defined(CONFIG_CHIP_TCC8925S)
				#if defined(CONFIG_STB_BOARD_DONGLE)
					return 0;
				#else
					return 1;
				#endif
			#elif defined(CONFIG_MACH_TCC8920ST)
					return 1;
			#else
				if(system_rev == 0x1006 || system_rev == 0x1008)
					return 1;
				else
					return 0;			
			#endif
		#elif defined(CONFIG_ARCH_TCC893X)
			#if defined(CONFIG_MACH_TCC8930ST)
				return 0;
			#else
				#if defined(CONFIG_CHIP_TCC8930)
					return 0;
				#elif defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
					return 1;		
				#endif
			#endif
		#endif
	#endif

	//default HDMI LCDC number
	return 0;
}
//EXPORT_SYMBOL(tca_get_output_lcdc_num);

#if defined(CONFIG_TCC8925S_DISP_PORT_CHANGE)

#define	LCD2_SEL			(Hw28|Hw29)
#define	LCD1_SEL			(Hw26|Hw27)
#define	LCD0_SEL			(Hw24|Hw25)

unsigned int tca_fb_output_path_change(unsigned int onoff)
{

	/* select Display hw sush as LCD or HDMI */
	/* TCC892X , TCC893X DISPLAY 1 - HDMI */
	/* TCC892X , TCC893X DISPLAY 0 - LCD */
	/* Otherwise, it is opposite */
	
	VIOC_IREQ_CONFIG 	*pIREQConfig;
	pIREQConfig = (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_IREQ);

	if(onoff)
	{
		BITCSET(pIREQConfig->uMISC1.nREG, LCD0_SEL, 1 << 24);	// DISP 0 - DISPLAY 1
		BITCSET(pIREQConfig->uMISC1.nREG, LCD1_SEL, 0 << 26);	// DISP 1 - DISPLAY 0
		//BITCSET(pIREQConfig->uMISC1.nREG, 0x0F000000, 0x1000000);
		
	}
	else
	{
		BITCSET(pIREQConfig->uMISC1.nREG, LCD0_SEL, 0 << 24 );	// DISP 0 - DISPLAY 0
		BITCSET(pIREQConfig->uMISC1.nREG, LCD1_SEL, 1 << 26);	// DISP 1 - DISPLAY 1
		//BITCSET(pIREQConfig->uMISC1.nREG, 0x0F000000, 0x4000000);
	}

	return 0;
}
//EXPORT_SYMBOL(tca_fb_output_path_change);

#endif
