
/****************************************************************************
 * linux/drivers/video/TCC_LCD_interface.c
 *
 * Author:  <linux@telechips.com>
 * Created: March 18, 2012
 * Description: TCC LCD Driver
 *
 * Copyright (C) 20010-2011 Telechips 
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
 *****************************************************************************/
/*****************************************************************************
*
* Header Files Include
*
******************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>

#include <asm/io.h>

#include <mach/bsp.h>
#include <mach/gpio.h>
#include <mach/TCC_LCD_Interface.h>


#if 1
#define dprintk(msg...)	 { printk( "VIOC_interface: " msg); }
#else
#define dprintk(msg...)	 
#endif



void LCDC_IO_Set (char lcdctrl_num, unsigned bit_per_pixel)
{
	int i;

	if(lcdctrl_num)	{
		tcc_gpio_config(TCC_GPE(0), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);		//LHSYNC
		tcc_gpio_config(TCC_GPE(1), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);		//LVSYNC
		tcc_gpio_config(TCC_GPE(26), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);	// LPXCLK
		tcc_gpio_config(TCC_GPE(27), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);	//LACBIAS
	}
	else {
		tcc_gpio_config(TCC_GPB(0), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);		// LPXCLK
		tcc_gpio_config(TCC_GPB(1), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);		//LHSYNC
		tcc_gpio_config(TCC_GPB(2), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);		//LVSYNC
		tcc_gpio_config(TCC_GPB(19), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);	//LACBIAS

	}


	switch (bit_per_pixel) {
		case 24:
			for(i = 18 ; i < 24; i++)	{
				if(lcdctrl_num)	{
					tcc_gpio_config(TCC_GPE(2 + i), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);
				}
				else {
					tcc_gpio_config(TCC_GPB(4 + i), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);
				}
			}

		case 18:
			for(i = 16 ; i < 18; i++)	{
				if(lcdctrl_num)	{
					tcc_gpio_config(TCC_GPE(2 + i), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);
				}
				else {
					tcc_gpio_config(TCC_GPB(3 + i), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);
				}
			}
			
		case 16:
			for(i = 8 ; i < 16; i++)	{
				if(lcdctrl_num)	{
					tcc_gpio_config(TCC_GPE(2 + i), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);
				}
				else {
					tcc_gpio_config(TCC_GPB(3 + i), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);
				}
			}
			
		case 8:
			for(i = 0 ; i < 8; i++)	{
				if(lcdctrl_num)	{
					tcc_gpio_config(TCC_GPE(2 + i), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);
				}
				else {
					tcc_gpio_config(TCC_GPB(3 + i), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);
				}
			}
			break;
			
		default:
			// do nothing
			break;
	}

}

#define LCD_PORT_DISABLE	0	
void LCDC_IO_Disable (char lcdctrl_num, unsigned bit_per_pixel)
{
	int i;

	if(lcdctrl_num)	{
		tcc_gpio_config(TCC_GPE(0), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);		//LHSYNC
		tcc_gpio_config(TCC_GPE(1), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);		//LVSYNC
		tcc_gpio_config(TCC_GPE(26), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);	// LPXCLK
		tcc_gpio_config(TCC_GPE(27), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);	//LACBIAS
	}
	else {
		tcc_gpio_config(TCC_GPB(0), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);		// LPXCLK
		tcc_gpio_config(TCC_GPB(1), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);		//LHSYNC
		tcc_gpio_config(TCC_GPB(2), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);		//LVSYNC
		tcc_gpio_config(TCC_GPB(19), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);	//LACBIAS
	}


	switch (bit_per_pixel) {
		case 24:
			for(i = 18 ; i < 24; i++)	{
				if(lcdctrl_num)	{
					tcc_gpio_config(TCC_GPE(2 + i), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);
				}
				else {
					tcc_gpio_config(TCC_GPB(4 + i), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);
				}
			}

		case 18:
			for(i = 16 ; i < 18; i++)	{
				if(lcdctrl_num)	{
					tcc_gpio_config(TCC_GPE(2 + i), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);
				}
				else {
					tcc_gpio_config(TCC_GPB(3 + i), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);
				}
			}
			
		case 16:
			for(i = 8 ; i < 16; i++)	{
				if(lcdctrl_num)	{
					tcc_gpio_config(TCC_GPE(2 + i), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);
				}
				else {
					tcc_gpio_config(TCC_GPB(3 + i), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);
				}
			}
			
		case 8:
			for(i = 0 ; i < 8; i++)	{
				if(lcdctrl_num)	{
					tcc_gpio_config(TCC_GPE(2 + i), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);
				}
				else {
					tcc_gpio_config(TCC_GPB(3 + i), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);
				}
			}
			break;
			
		default:
			// do nothing
			break;
	}

}





