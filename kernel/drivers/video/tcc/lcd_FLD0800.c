/*
 * lcd_FLD0800.c -- support for FLD0800 LVDS Panel
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
#include <asm/system.h>

#include <mach/tcc_fb.h>
#include <mach/gpio.h>
#include <mach/tca_lcdc.h>
#include <mach/TCC_LCD_Interface.h>
#include <mach/bsp.h>

#include <asm/mach-types.h>
#include <asm/io.h>
#include <mach/tca_tco.h>

static struct mutex panel_lock;
static char lcd_pwr_state;
static unsigned int lcd_bl_level;
extern void lcdc_initialize(struct lcd_panel *lcd_spec, unsigned int lcdc_num);
static struct clk *lvds_clk;

extern unsigned int lvds_stby;
extern unsigned int lvds_power;

#define     LVDS_VCO_45MHz        450000
#define     LVDS_VCO_60MHz        600000

static int fld0800_panel_init(struct lcd_panel *panel)
{
	printk("%s : %d\n", __func__, 0);
	return 0;
}

static int fld0800_set_power(struct lcd_panel *panel, int on, unsigned int lcd_num)
{
	PDDICONFIG	pDDICfg;
	unsigned int P, M, S, VSEL, TC;
	struct lcd_platform_data *pdata = panel->dev->platform_data;
	printk("%s : %d %d  \n", __func__, on, lcd_bl_level);

	mutex_lock(&panel_lock);
	panel->state = on;

	pDDICfg = (volatile PDDICONFIG)tcc_p2v(HwDDI_CONFIG_BASE);
	if (on) {

#if defined(CONFIG_MACH_TCC892X) || defined(CONFIG_MACH_TCC893X)		
		gpio_set_value(lvds_power, 1);
#else
	#if defined(CONFIG_MACH_M805_893X)
		if (system_rev == 0x5001 || system_rev == 0x5002 || system_rev == 0x5003)
		gpio_set_value(pdata->iod_en, 1);
	#endif
#endif // 

		gpio_set_value(pdata->power_on, 1);
		udelay(20);
		gpio_set_value(pdata->reset, 1);
		gpio_set_value(lvds_stby, 1);
#if defined(CONFIG_MACH_TCC892X) || defined(CONFIG_MACH_TCC893X)
		gpio_set_value(pdata->display_on, 1);
#endif
		mdelay(20);
		
		// LVDS power on
		clk_enable(lvds_clk);	
		
		lcdc_initialize(panel, lcd_num);

		//LVDS 6bit setting for internal dithering option !!!
		//tcc_lcdc_dithering_setting(lcd_num);

#if defined(CONFIG_ARCH_TCC892X) || defined(CONFIG_ARCH_TCC893X)
			
		// LVDS reset	
		pDDICfg->LVDS_CTRL.bREG.RST =1;
		pDDICfg->LVDS_CTRL.bREG.RST =0;		

#if 1
#if 1
             BITCSET(pDDICfg->LVDS_TXO_SEL0.nREG, 0xFFFFFFFF, 0x13121110);
		BITCSET(pDDICfg->LVDS_TXO_SEL1.nREG, 0xFFFFFFFF, 0x09081514);
		BITCSET(pDDICfg->LVDS_TXO_SEL2.nREG, 0xFFFFFFFF, 0x0D0C0B0A);
		BITCSET(pDDICfg->LVDS_TXO_SEL3.nREG, 0xFFFFFFFF, 0x03020100);
		BITCSET(pDDICfg->LVDS_TXO_SEL4.nREG, 0xFFFFFFFF, 0x1A190504);
		BITCSET(pDDICfg->LVDS_TXO_SEL5.nREG, 0xFFFFFFFF, 0x0E171618);
		BITCSET(pDDICfg->LVDS_TXO_SEL6.nREG, 0xFFFFFFFF, 0x1F07060F);
		BITCSET(pDDICfg->LVDS_TXO_SEL7.nREG, 0xFFFFFFFF, 0x1F1E1F1E);
		BITCSET(pDDICfg->LVDS_TXO_SEL8.nREG, 0xFFFFFFFF, 0x001E1F1E);
#else       

		BITCSET(pDDICfg->LVDS_TXO_SEL0.nREG, 0xFFFFFFFF, 0x15141312);
		BITCSET(pDDICfg->LVDS_TXO_SEL1.nREG, 0xFFFFFFFF, 0x0B0A1716);
		BITCSET(pDDICfg->LVDS_TXO_SEL2.nREG, 0xFFFFFFFF, 0x0F0E0D0C);
		BITCSET(pDDICfg->LVDS_TXO_SEL3.nREG, 0xFFFFFFFF, 0x05040302);
		BITCSET(pDDICfg->LVDS_TXO_SEL4.nREG, 0xFFFFFFFF, 0x1A190706);
		BITCSET(pDDICfg->LVDS_TXO_SEL5.nREG, 0xFFFFFFFF, 0x1F1E1F18);
		BITCSET(pDDICfg->LVDS_TXO_SEL6.nREG, 0xFFFFFFFF, 0x1F1E1F1E);
		BITCSET(pDDICfg->LVDS_TXO_SEL7.nREG, 0xFFFFFFFF, 0x1F1E1F1E);
		BITCSET(pDDICfg->LVDS_TXO_SEL8.nREG, 0xFFFFFFFF, 0x001E1F1E);
#endif
#else
		//LVDS 6bit setting for internal dithering option !!!
		BITCSET(pDDICfg->LVDS_TXO_SEL0.nREG, 0xFFFFFFFF, 0x0F0E0D0C);
		BITCSET(pDDICfg->LVDS_TXO_SEL1.nREG, 0xFFFFFFFF, 0x07061110);
		BITCSET(pDDICfg->LVDS_TXO_SEL2.nREG, 0xFFFFFFFF, 0x0B0A0908);
		BITCSET(pDDICfg->LVDS_TXO_SEL3.nREG, 0xFFFFFFFF, 0x03020100);
		BITCSET(pDDICfg->LVDS_TXO_SEL4.nREG, 0xFFFFFFFF, 0x1A190504);
		BITCSET(pDDICfg->LVDS_TXO_SEL5.nREG, 0xFFFFFFFF, 0x1F1E1F18);
		BITCSET(pDDICfg->LVDS_TXO_SEL6.nREG, 0xFFFFFFFF, 0x1F1E1F1E);
		BITCSET(pDDICfg->LVDS_TXO_SEL7.nREG, 0xFFFFFFFF, 0x1F1E1F1E);
		BITCSET(pDDICfg->LVDS_TXO_SEL8.nREG, 0xFFFFFFFF, 0x001E1F1E);

#endif

            //LVDS P,M,S,VSEL select
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
        		M = 10; P = 10; S = 0; VSEL = 0; TC = 4;		
        		BITCSET(pDDICfg->LVDS_CTRL.nREG, 0x00FFFFF0, (VSEL<<4)|(S<<5)|(M<<8)|(P<<15)|(TC<<21)); //LCDC1
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
		
#if defined(CONFIG_MACH_TCC892X) || defined(CONFIG_MACH_TCC893X)
		gpio_set_value(pdata->display_on, 0);
#endif		

		gpio_set_value(pdata->reset, 0);	//NC
		gpio_set_value(lvds_stby, 0);
		gpio_set_value(pdata->power_on, 0);

#if defined(CONFIG_MACH_TCC892X) || defined(CONFIG_MACH_TCC893X)
		gpio_set_value(lvds_power,0);
#else
	#if defined(CONFIG_MACH_M805_893X)
		if (system_rev == 0x5001 || system_rev == 0x5002 || system_rev == 0x5003)
		gpio_set_value(pdata->iod_en, 0);
	#endif
#endif // 

	}
	mutex_unlock(&panel_lock);

	if(lcd_pwr_state)
		panel->set_backlight_level(panel , lcd_bl_level);


	return 0;
}

static int fld0800_set_backlight_level(struct lcd_panel *panel, int level)
{
	#define MAX_BL_LEVEL	255	

	struct lcd_platform_data *pdata = panel->dev->platform_data;

	//printk("%s : level:%d  power:%d \n", __func__, level, panel->state);
	
	mutex_lock(&panel_lock);
	panel->bl_level = level;

	#define MAX_BACKLIGTH 255
 	if (level == 0) {
		tca_tco_pwm_ctrl(0, pdata->bl_on, MAX_BACKLIGTH, level);
	}
	else 
	{
		if(panel->state)
		{
			#if defined(CONFIG_ARCH_TCC892X)
			if(system_rev == 0x1005 || system_rev == 0x1006 || system_rev == 0x1007 || system_rev == 0x1008 || system_rev == 0x2002 || system_rev == 0x2003 || system_rev == 0x2004 || system_rev == 0x2005 || system_rev == 0x2006 || system_rev == 0x2007 || system_rev == 0x2008 || system_rev == 0x2009)
				tca_tco_pwm_ctrl(0, pdata->bl_on, MAX_BACKLIGTH, level);
			else
				tca_tco_pwm_ctrl(1, pdata->bl_on, MAX_BACKLIGTH, level);
			#elif defined(CONFIG_ARCH_TCC88XX) || defined(CONFIG_ARCH_TCC893X)
				tca_tco_pwm_ctrl(0, pdata->bl_on, MAX_BACKLIGTH, level);
			#else
				printk("%s :  Do not support chipset !!!\n",__func__);
			#endif//
		}
	}

 	mutex_unlock(&panel_lock);

	return 0;

}

struct lcd_panel fld0800_panel = {
	.name		= "FLD0800",
	.manufacturer	= "innolux",
	.id		= PANEL_ID_FLD0800,
	.xres		= 1024,
	.yres		= 600,
	.width		= 153,
	.height		= 90,
	.bpp		= 24,
	.clk_freq	= 512000,
	.clk_div	= 2,
	.bus_width	= 24,
	
	.lpw		= 20,
	.lpc		= 1024,
	.lswc		= 150,
	.lewc		= 150,
	.vdb		= 0,
	.vdf		= 0,

	.fpw1		= 2,
	.flc1		= 600,
	.fswc1		= 10,
	.fewc1		= 25,
	
	.fpw2		= 2,
	.flc2		= 600,
	.fswc2		= 10,
	.fewc2		= 25,	.sync_invert	= IV_INVERT | IH_INVERT,
	.init		= fld0800_panel_init,
	.set_power	= fld0800_set_power,
	.set_backlight_level = fld0800_set_backlight_level,
};

static int fld0800_probe(struct platform_device *pdev)
{
	struct lcd_platform_data *pdata = pdev->dev.platform_data;
	printk("%s : %s\n", __func__,  pdev->name);
    	printk(" ### %s ### \n",__func__);
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
	gpio_request(lvds_power , "lvds_power");

	gpio_direction_output(pdata->power_on, 1);
	gpio_direction_output(pdata->bl_on, 1);
	gpio_direction_output(pdata->display_on, 1);
	gpio_direction_output(pdata->reset, 1);

	gpio_direction_output(lvds_stby, 1);
	gpio_direction_output(lvds_power,1);

	fld0800_panel.state = 1;
	fld0800_panel.dev = &pdev->dev;
	#if defined(CONFIG_ARCH_TCC892X) || defined(CONFIG_ARCH_TCC893X)
		lvds_clk = clk_get(0, "lvds_phy"); //8920
		if(IS_ERR(lvds_clk))
		lvds_clk = NULL;
	#else
		lvds_clk = clk_get(0, "lvds");
		BUG_ON(lvds_clk == NULL);
	#endif
	clk_enable(lvds_clk);	
	tccfb_register_panel(&fld0800_panel);
	return 0;
}

static int fld0800_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver fld0800_lcd = {
	.probe	= fld0800_probe,
	.remove	= fld0800_remove,
	.driver	= {
		.name	= "fld0800_lcd",
		.owner	= THIS_MODULE,
	},
};

static __init int fld0800_init(void)
{
	printk("~ %s ~ \n", __func__);
	return platform_driver_register(&fld0800_lcd);
}

static __exit void fld0800_exit(void)
{
	platform_driver_unregister(&fld0800_lcd);
}

subsys_initcall(fld0800_init);
module_exit(fld0800_exit);

MODULE_DESCRIPTION("FLD0800 LCD driver");
MODULE_LICENSE("GPL");
