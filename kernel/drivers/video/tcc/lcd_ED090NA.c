/*
 * lcd_ED090NA.c -- support for ED090NA LVDS Panel
 *
 * Copyright (C) 2009, 2010 Telechips, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/module.h>

#include <mach/tcc_fb.h>
#include <mach/gpio.h>
#include <mach/tca_lcdc.h>
#include <mach/TCC_LCD_Interface.h>
#include <mach/bsp.h>

#include <asm/mach-types.h>
#include <asm/io.h>

static struct mutex panel_lock;
static char lcd_pwr_state;
static unsigned int lcd_bl_level;
extern void lcdc_initialize(struct lcd_panel *lcd_spec, unsigned int lcdc_num);
static struct clk *lvds_clk;


unsigned int lvds_stby;
unsigned int lvds_power;

#define     LVDS_VCO_45MHz        450000
#define     LVDS_VCO_60MHz        600000

static int ed090na_panel_init(struct lcd_panel *panel)
{
	printk("%s : %d\n", __func__, 0);
	return 0;
}

static int ed090na_set_power(struct lcd_panel *panel, int on, unsigned int lcd_num)
{
	PDDICONFIG	pDDICfg;
	unsigned int P, M, S, VSEL, TC;
	struct lcd_platform_data *pdata = panel->dev->platform_data;
	printk("%s : %d %d  \n", __func__, on, lcd_bl_level);

	mutex_lock(&panel_lock);
	lcd_pwr_state = on;

	pDDICfg = (volatile PDDICONFIG)tcc_p2v(HwDDI_CONFIG_BASE);
	if (on) {
		gpio_set_value(lvds_power, 1);
		gpio_set_value(pdata->power_on, 1);
		udelay(20);
		gpio_set_value(pdata->reset, 1);
		gpio_set_value(lvds_stby, 1);
		gpio_set_value(pdata->display_on, 1);
		mdelay(20);
		
		// LVDS power on
		clk_enable(lvds_clk);	
		
		lcdc_initialize(panel, lcd_num);

#if defined(CONFIG_ARCH_TCC892X) || defined(CONFIG_ARCH_TCC893X)
			
		// LVDS reset	
		pDDICfg->LVDS_CTRL.bREG.RST =1;
		pDDICfg->LVDS_CTRL.bREG.RST =0;		

		BITCSET(pDDICfg->LVDS_TXO_SEL0.nREG, 0xFFFFFFFF, 0x15141312);
		BITCSET(pDDICfg->LVDS_TXO_SEL1.nREG, 0xFFFFFFFF, 0x0B0A1716);
		BITCSET(pDDICfg->LVDS_TXO_SEL2.nREG, 0xFFFFFFFF, 0x0F0E0D0C);
		BITCSET(pDDICfg->LVDS_TXO_SEL3.nREG, 0xFFFFFFFF, 0x05040302);
		BITCSET(pDDICfg->LVDS_TXO_SEL4.nREG, 0xFFFFFFFF, 0x1A190706);
		BITCSET(pDDICfg->LVDS_TXO_SEL5.nREG, 0xFFFFFFFF, 0x1F1E1F18);
		BITCSET(pDDICfg->LVDS_TXO_SEL6.nREG, 0xFFFFFFFF, 0x1F1E1F1E);
		BITCSET(pDDICfg->LVDS_TXO_SEL7.nREG, 0xFFFFFFFF, 0x1F1E1F1E);
		BITCSET(pDDICfg->LVDS_TXO_SEL8.nREG, 0xFFFFFFFF, 0x001E1F1E);

            #if defined(CONFIG_ARCH_TCC893X)
            if( panel->clk_freq >= LVDS_VCO_45MHz && panel->clk_freq < LVDS_VCO_60MHz)
            {
                   M = 7; P = 7; S = 0; VSEL = 0; TC = 4;
            }
            else
            {
              	M = 10; P = 10; S = 0; VSEL = 0; TC = 4;		
            }
            #else
            		M = 10; P = 10; S = 0; VSEL = 0; TC = 4;		
            #endif
            
		BITCSET(pDDICfg->LVDS_CTRL.nREG, 0x00FFFFF0, (VSEL<<4)|(S<<5)|(M<<8)|(P<<15)|(TC<<21)); //LCDC1

		// LVDS Select LCDC 1
		if(lcd_num)
			BITCSET(pDDICfg->LVDS_CTRL.nREG, 0x3 << 30 , 0x1 << 30);
		else
			BITCSET(pDDICfg->LVDS_CTRL.nREG, 0x3 << 30 , 0x0 << 30);
		
	    	pDDICfg->LVDS_CTRL.bREG.RST = 1;	//  reset
	  	pDDICfg->LVDS_CTRL.bREG.EN = 1;   // enable
#else
		// LVDS reset
		BITSET(pDDICfg->LVDS_CTRL, Hw1);	// reset
		BITCLR(pDDICfg->LVDS_CTRL, Hw1);	// normal
	
		pDDICfg->LVDS_TXO_SEL0 = 0x15141312; // SEL_03, SEL_02, SEL_01, SEL_00,
		pDDICfg->LVDS_TXO_SEL1 = 0x0B0A1716; // SEL_07, SEL_06, SEL_05, SEL_04,
		pDDICfg->LVDS_TXO_SEL2 = 0x0F0E0D0C; // SEL_11, SEL_10, SEL_09, SEL_08,
		pDDICfg->LVDS_TXO_SEL3 = 0x05040302; // SEL_15, SEL_14, SEL_13, SEL_12,
		pDDICfg->LVDS_TXO_SEL4 = 0x1A190706; // SEL_19, SEL_18, SEL_17, SEL_16,
		pDDICfg->LVDS_TXO_SEL5 = 0x1F1E1F18; // 						SEL_20,
		pDDICfg->LVDS_TXO_SEL6 = 0x1F1E1F1E;
		pDDICfg->LVDS_TXO_SEL7 = 0x1F1E1F1E;
		pDDICfg->LVDS_TXO_SEL8 = 0x1F1E1F1E;
		
		// LVDS Select
	//	BITCLR(pDDICfg->LVDS_CTRL, Hw0); //LCDC0
		BITSET(pDDICfg->LVDS_CTRL, Hw0); //LCDC1
	
	
		#ifdef CONFIG_ARCH_TCC88XX
		{

			M = 10; P = 10; S = 0; VSEL = 0;
			BITCSET(pDDICfg->LVDS_CTRL, Hw21- Hw4, (VSEL<<4)|(S<<5)|(M<<8)|(P<<15)); //LCDC1
		}

		BITSET(pDDICfg->LVDS_CTRL, Hw1);	// reset
		#endif//
		
		// LVDS enable
		BITSET(pDDICfg->LVDS_CTRL, Hw2);	// enable
#endif		
		msleep(100);

	}
	else 
	{
		#if defined(CONFIG_ARCH_TCC892X) || defined(CONFIG_ARCH_TCC893X)
			pDDICfg->LVDS_CTRL.bREG.RST = 1;		// reset		
			pDDICfg->LVDS_CTRL.bREG.EN = 0;		// reset		
		#else	
			#if defined(CONFIG_ARCH_TCC88XX)		
				BITCLR(pDDICfg->LVDS_CTRL, Hw1);	// reset			
			#endif
			BITCLR(pDDICfg->LVDS_CTRL, Hw2);	// disable
		#endif
		
		clk_disable(lvds_clk);	
		gpio_set_value(pdata->display_on, 0);
		
		gpio_set_value(pdata->reset, 0);

		gpio_set_value(lvds_stby, 0);

		gpio_set_value(pdata->power_on, 0);

		gpio_set_value(lvds_power, 0);

	}
	mutex_unlock(&panel_lock);

	if(lcd_pwr_state)
		panel->set_backlight_level(panel , lcd_bl_level);


	return 0;
}

static int ed090na_set_backlight_level(struct lcd_panel *panel, int level)
{
	struct lcd_platform_data *pdata = panel->dev->platform_data;

	//printk("### %s power =(%d) level = (%d) \n",__func__,lcd_pwr_state,level);

	mutex_lock(&panel_lock);
	lcd_bl_level = level;
	
	if (level == 0) {
		gpio_set_value(pdata->bl_on, 0);
	} else {
		if(lcd_pwr_state)
			gpio_set_value(pdata->bl_on, 1);
	}
	mutex_unlock(&panel_lock);
	
	return 0;
}

struct lcd_panel ed090na_panel = {
	.name		= "ED090NA",
	.manufacturer	= "innolux",
	.id		= PANEL_ID_ED090NA,
	.xres		= 1280,
	.yres		= 800,
	.width		= 235,
	.height		= 163,
	.bpp		= 24,
	.clk_freq	= 680000,
	.clk_div	= 2,
	.bus_width	= 24,
	
	.lpw		= 10,
	.lpc		= 1280,
	.lswc	= 74,
	.lewc	= 74,
	.vdb		= 0,
	.vdf		= 0,

	.fpw1		= 2,
	.flc1		= 800,
	.fswc1		= 10,
	.fewc1		= 25,
	
	.fpw2		= 2,
	.flc2		= 800,
	.fswc2		= 10,
	.fewc2		= 25,
	.sync_invert	= IV_INVERT | IH_INVERT,
	.init		= ed090na_panel_init,
	.set_power	= ed090na_set_power,
	.set_backlight_level = ed090na_set_backlight_level,
};

static int ed090na_probe(struct platform_device *pdev)
{
	struct lcd_platform_data *pdata = pdev->dev.platform_data;
	printk("%s : %s\n", __func__,  pdev->name);
	mutex_init(&panel_lock);
	lcd_pwr_state = 1;
#if defined(CONFIG_ARCH_TCC892X) || defined(CONFIG_ARCH_TCC893X)
	if(pdata->lcdc_num ==1)
		lvds_stby = TCC_GPE(27);
	else
		lvds_stby = TCC_GPB(19);
#else
	lvds_stby  = TCC_GPC(24);
#endif//

	lvds_power = TCC_GPEXT5(7);

	gpio_request(pdata->power_on, "lcd_on");
	gpio_request(pdata->bl_on, "lcd_bl");
	gpio_request(pdata->display_on, "lcd_display");
	gpio_request(pdata->reset, "lcd_reset");

	gpio_request(lvds_stby, "lcd_stbyb");
	gpio_request(lvds_power, "lvds_power"); 

	gpio_direction_output(pdata->power_on, 1);
	gpio_direction_output(pdata->bl_on, 1);
	gpio_direction_output(pdata->display_on, 1);
	gpio_direction_output(pdata->reset, 1);

	gpio_direction_output(lvds_stby, 1);
	gpio_direction_output(lvds_power, 1);


	ed090na_panel.dev = &pdev->dev;
	#if defined(CONFIG_ARCH_TCC892X) || defined(CONFIG_ARCH_TCC893X)
		lvds_clk = clk_get(0, "lvds_phy"); //8920
		if(IS_ERR(lvds_clk))
		lvds_clk = NULL;
	#else
		lvds_clk = clk_get(0, "lvds");
		BUG_ON(lvds_clk == NULL);
	#endif
	clk_enable(lvds_clk);	
	tccfb_register_panel(&ed090na_panel);
	return 0;
}

static int ed090na_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver ed090na_lcd = {
	.probe	= ed090na_probe,
	.remove	= ed090na_remove,
	.driver	= {
		.name	= "ed090na_lcd",
		.owner	= THIS_MODULE,
	},
};

static __init int ed090na_init(void)
{
	printk("~ %s ~ \n", __func__);
	return platform_driver_register(&ed090na_lcd);
}

static __exit void ed090na_exit(void)
{
	platform_driver_unregister(&ed090na_lcd);
}

subsys_initcall(ed090na_init);
module_exit(ed090na_exit);

MODULE_DESCRIPTION("ED090NA LCD driver");
MODULE_LICENSE("GPL");
