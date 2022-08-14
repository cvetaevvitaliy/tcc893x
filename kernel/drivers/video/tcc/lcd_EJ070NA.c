/*
 * lcd_EJ070NA.c -- support for EJ070NA LVDS Panel
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
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>


#include <mach/tcc_fb.h>
#include <mach/gpio.h>
#include <mach/tca_lcdc.h>
#include <mach/TCC_LCD_Interface.h>
#include <mach/bsp.h>

#include <mach/vioc_outcfg.h>

#include <asm/mach-types.h>
#include <asm/io.h>
#include <mach/tca_tco.h>
#include <mach/tcc_fb.h>

#define NORMAL_COLOR           "\033[0m"
#define RED_COLOR              "\033[1;31m"
#define GREEN_COLOR            "\033[1;32m" 
#define COLORERR_COLOR         "\033[1;37;41m"
#define COLORWRN_COLOR         "\033[0;31m"

#ifdef CONFIG_TCC_MIPI
#define EJ070NA_MIPI_LCD
#define MIPI_LCD_TEST
#endif//

static struct mutex panel_lock;
static char lcd_pwr_state;
static unsigned int lcd_bl_level;
extern void lcdc_initialize(struct lcd_panel *lcd_spec, unsigned int lcdc_num);
static struct clk *lvds_clk;

extern unsigned int lvds_stby;
extern unsigned int lvds_power;
struct lcd_panel ej070na_panel;

#define     LVDS_VCO_45MHz        450000
#define     LVDS_VCO_60MHz        600000

#ifdef EJ070NA_MIPI_LCD

struct tcc_i2c_chip_info {
	unsigned gpio_start;
	uint16_t reg_output;
	uint16_t reg_direction;

	struct i2c_client *client;
	struct gpio_chip gpio_chip;
};


static const struct i2c_device_id MIPI_SN65DSI83_id[] = {
	{ "MIPI_SN65DSI83_pane_i2c", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, MIPI_SN65DSI83_id);

#define SLAVE_ADDR_SN65DSI83			0x58

void SN65DSI83_read(unsigned reg, unsigned *value)
{
	struct i2c_client *client = dev_get_drvdata(ej070na_panel.dev);

	*value = i2c_smbus_read_byte_data(client, reg);
}

void SN65DSI83_write(unsigned reg, unsigned value)
{
	struct i2c_client *client = dev_get_drvdata(ej070na_panel.dev);	
	i2c_smbus_write_byte_data(client, reg, value);
}

void SN65DSI83_Check(void)
{
	unsigned char i;
	unsigned char data= 0x00;

	for(i = 0; i <=0x8; i++)
	{	
		SN65DSI83_read(i, &data);
		printk("%s %s:  cmd:0x%x, result:0x%x  %s\n",COLORWRN_COLOR,  __func__, i, data, NORMAL_COLOR);
	}
	
}
void SN65DSI83_init(void)
{
	unsigned char data= 0x00;
	unsigned i = 0;

	printk("%s %s: LINE:%d  %s\n",COLORWRN_COLOR,  __func__ , __LINE__, NORMAL_COLOR);

	SN65DSI83_write(0x09, 0x00);
	SN65DSI83_write(0x0A, 0x03);
	SN65DSI83_write(0x0B, 0x18);
	SN65DSI83_write(0x0D, 0x00);
	SN65DSI83_write(0x10, 0x26);
	SN65DSI83_write(0x11, 0x00);
	SN65DSI83_write(0x12, 0x2a);
	SN65DSI83_write(0x13, 0x00);
	SN65DSI83_write(0x18, 0x78);
	SN65DSI83_write(0x19, 0x00);
	SN65DSI83_write(0x1A, 0x03);
	SN65DSI83_write(0x1B, 0x00);

	SN65DSI83_write(0x20, 0x00);
	SN65DSI83_write(0x21, 0x04);
	SN65DSI83_write(0x22, 0x00);
	SN65DSI83_write(0x23, 0x00);
	SN65DSI83_write(0x24, 0x00);
	SN65DSI83_write(0x25, 0x00);
	SN65DSI83_write(0x26, 0x00);
	SN65DSI83_write(0x27, 0x00);
	SN65DSI83_write(0x28, 0x20);
	SN65DSI83_write(0x29, 0x00); 
	SN65DSI83_write(0x2A, 0x00);
	SN65DSI83_write(0x2B, 0x00);
	SN65DSI83_write(0x2C, 0x14);
	SN65DSI83_write(0x2D, 0x00);
	SN65DSI83_write(0x2E, 0x00);
	SN65DSI83_write(0x2F, 0x00);
	SN65DSI83_write(0x30, 0x02);
	SN65DSI83_write(0x31, 0x00);
	SN65DSI83_write(0x32, 0x00);
	SN65DSI83_write(0x33, 0x00);
	SN65DSI83_write(0x34, 0x96);
	SN65DSI83_write(0x35, 0x00);
	SN65DSI83_write(0x36, 0x00);
	SN65DSI83_write(0x37, 0x00);
	SN65DSI83_write(0x38, 0x00);
	SN65DSI83_write(0x39, 0x00);
	SN65DSI83_write(0x3A, 0x00);
	SN65DSI83_write(0x3B, 0x00);
	SN65DSI83_write(0x3C, 0x00);
	SN65DSI83_write(0x3D, 0x00);
	SN65DSI83_write(0x3E, 0x00);

	SN65DSI83_write(0x0D, 0x1);
	SN65DSI83_read(0x0A, &data);
		printk("##~~0x%x~~ \n", data);
	while((!(data & 0x80)) && (i <= 100) )
	{
		SN65DSI83_read(0x0A, &data);
		printk("~~0x%x~~ \n", data);
		i++;
	}

}

void SN65DSI83_video_stop(void)
{
	unsigned char data= 0x00;
	printk("%s %s: LINE:%d  %s\n",COLORWRN_COLOR,  __func__ , __LINE__, NORMAL_COLOR);
	SN65DSI83_write(0x0D, 0);
	SN65DSI83_read(0x0A, &data);
	printk("~~0x%x~~ \n", data);
}

void mipi_lcd_test(char level, struct lcd_panel *panel)
{
	struct lcd_platform_data *pdata = panel->dev->platform_data;

		switch(level)
		{
			case 211:
				SN65DSI83_video_stop();
				break;
			case 210:
				//mipi power
				if(panel->db_out_ops && panel->db_out_ops->disable)
					panel->db_out_ops->disable(panel);
				break;

			case 212:
				VIOC_OUTCFG_SetOutConfig (VIOC_OUTCFG_MRGB, pdata->lcdc_num);
				if(panel->db_out_ops && panel->db_out_ops->enable)
					panel->db_out_ops->enable(panel);
				break;

			case 213:
				SN65DSI83_init();
				break;	
			case 214:
				SN65DSI83_Check();
				break;			
			case 215:
				SN65DSI83_video_stop();
				if(panel->db_out_ops && panel->db_out_ops->disable)
					panel->db_out_ops->disable(panel);
				break;

			case 216:
				VIOC_OUTCFG_SetOutConfig (VIOC_OUTCFG_MRGB, pdata->lcdc_num);
				if(panel->db_out_ops && panel->db_out_ops->enable)
					panel->db_out_ops->enable(panel);

				SN65DSI83_init();

				break;	
		}

}

static int MIPI_SN65DSI83_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct tcc_i2c_chip_info *info;

	info = kzalloc(sizeof(struct tcc_i2c_chip_info), GFP_KERNEL);
	if (info == NULL) {
		printk("MIPI_SN65DSI83_bl: no memory for i2c\n");
		return -ENOMEM;
	}

	info->client = client;
	i2c_set_clientdata(client, info);

	return 0;
}

static int MIPI_SN65DSI83_i2c_remove(struct i2c_client *client)
{
	struct bl_info *info = i2c_get_clientdata(client);
	kfree(info);
	return 0;
}
#endif//

static int ej070na_panel_init(struct lcd_panel *panel)
{
	printk("%s : %d\n", __func__, 0);
	return 0;
}

static int ej070na_set_power(struct lcd_panel *panel, int on, unsigned int lcd_num)
{
	PDDICONFIG	pDDICfg;
	unsigned int P, M, S, VSEL, TC;
	struct lcd_platform_data *pdata = panel->dev->platform_data;
	printk("%s : %d %d  \n", __func__, on, lcd_bl_level);

	mutex_lock(&panel_lock);
	panel->state = on;

	pDDICfg = (volatile PDDICONFIG)tcc_p2v(HwDDI_CONFIG_BASE);
	if (on) {
		
		gpio_set_value(lvds_power, 1);
		gpio_set_value(pdata->power_on, 1);
		udelay(20);
		gpio_set_value(pdata->reset, 1);
		gpio_set_value(lvds_stby, 1);
		gpio_set_value(pdata->display_on, 1);
		mdelay(20);
		
		lcdc_initialize(panel, lcd_num);

		#ifdef EJ070NA_MIPI_LCD
			VIOC_OUTCFG_SetOutConfig (VIOC_OUTCFG_MRGB, pdata->lcdc_num);
			if(panel->db_out_ops && panel->db_out_ops->enable)
				panel->db_out_ops->enable(panel);
			SN65DSI83_init();
		#else
			// LVDS power on
			clk_enable(lvds_clk);	

			#if defined(CONFIG_ARCH_TCC892X) || defined(CONFIG_ARCH_TCC893X)
			// LVDS reset	
			pDDICfg->LVDS_CTRL.bREG.RST =1;
			pDDICfg->LVDS_CTRL.bREG.RST =0;		
			BITCSET(pDDICfg->LVDS_TXO_SEL0.nREG, 0xFFFFFFFF, 0x13121110);
			BITCSET(pDDICfg->LVDS_TXO_SEL1.nREG, 0xFFFFFFFF, 0x09081514);
			BITCSET(pDDICfg->LVDS_TXO_SEL2.nREG, 0xFFFFFFFF, 0x0D0C0B0A);
			BITCSET(pDDICfg->LVDS_TXO_SEL3.nREG, 0xFFFFFFFF, 0x03020100);		
			BITCSET(pDDICfg->LVDS_TXO_SEL4.nREG, 0xFFFFFFFF, 0x1A190504);
			BITCSET(pDDICfg->LVDS_TXO_SEL5.nREG, 0xFFFFFFFF, 0x0E171618);
			BITCSET(pDDICfg->LVDS_TXO_SEL6.nREG, 0xFFFFFFFF, 0x1F07060F);
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
		
        		M = 10; P = 10; S = 0; VSEL = 0; TC = 4;		
        		BITCSET(pDDICfg->LVDS_CTRL.nREG, 0x00FFFFF0, (VSEL<<4)|(S<<5)|(M<<8)|(P<<15)|(TC<<21)); //LCDC1

			BITSET(pDDICfg->LVDS_CTRL, Hw1);	// reset
			
			// LVDS enable
			BITSET(pDDICfg->LVDS_CTRL, Hw2);	// enable
			#endif		
		#endif//

		msleep(100);

	}
	else 
	{

		#ifdef EJ070NA_MIPI_LCD
			SN65DSI83_video_stop();
			if(panel->db_out_ops && panel->db_out_ops->disable)
				panel->db_out_ops->disable(panel);
		#else
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
		#endif//
	
		gpio_set_value(pdata->reset, 0);	//NC

		gpio_set_value(lvds_stby, 0);

		gpio_set_value(pdata->power_on, 0);

		gpio_set_value(lvds_power, 0);
	}
	mutex_unlock(&panel_lock);

	if(lcd_pwr_state)
		panel->set_backlight_level(panel , lcd_bl_level);


	return 0;
}

static int ej070na_set_backlight_level(struct lcd_panel *panel, int level)
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

		#ifdef MIPI_LCD_TEST
		mipi_lcd_test(level, panel);
		#endif//
	}

 	mutex_unlock(&panel_lock);

	return 0;

}

struct lcd_panel ej070na_panel = {
	.db_type		 = TCC_DB_OUT_DSI,
	.name		= "EJ070NA",
	.manufacturer	= "CHIMEI",
	.id		= PANEL_ID_EJ070NA,
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
	.fewc1		= 23,
	
	.fpw2		= 2,
	.flc2		= 600,
	.fswc2		= 10,
	.fewc2		= 23,	
#if defined(EJ070NA_MIPI_LCD)
	.sync_invert	= IP_INVERT,//IV_INVERT | IH_INVERT,
#else
	.sync_invert	= IV_INVERT | IH_INVERT,
#endif//

	.init		= ej070na_panel_init,
	.set_power	= ej070na_set_power,
	.set_backlight_level = ej070na_set_backlight_level,
};

static int ej070na_probe(struct platform_device *pdev)
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

#ifdef EJ070NA_MIPI_LCD
	struct i2c_adapter *adap = i2c_get_adapter(1);
	struct i2c_client *i2c;
	struct i2c_board_info MIPI_SN65DSI83_panel_i2c_info = {
		I2C_BOARD_INFO("MIPI_SN65DSI83_panel_i2c", SLAVE_ADDR_SN65DSI83 >> 1),
		.platform_data = &ej070na_panel,
	};

	/* TODO: add channel configuration */
	i2c = i2c_new_device(adap , &MIPI_SN65DSI83_panel_i2c_info);
	if (!i2c)
		return -EINVAL;
	
	dev_set_drvdata(&pdev->dev, i2c);

	tcc_db_set_output(&ej070na_panel);
#endif//

	ej070na_panel.state = 1;
	ej070na_panel.dev = &pdev->dev;
	#if defined(CONFIG_ARCH_TCC892X) || defined(CONFIG_ARCH_TCC893X)
		lvds_clk = clk_get(0, "lvds_phy"); //8920
		if(IS_ERR(lvds_clk))
		lvds_clk = NULL;
	#else
		lvds_clk = clk_get(0, "lvds");
		BUG_ON(lvds_clk == NULL);
	#endif
	clk_enable(lvds_clk);	
	tccfb_register_panel(&ej070na_panel);
	return 0;
}

static int ej070na_remove(struct platform_device *pdev)
{
	return 0;
}
#ifdef EJ070NA_MIPI_LCD
static struct i2c_driver MIPI_SN65DSI83_panel_i2c = {
	.probe		= MIPI_SN65DSI83_i2c_probe,
	.remove		= MIPI_SN65DSI83_i2c_remove,
	.driver = {
		.name	= "MIPI_SN65DSI83_panel_i2c",
		.owner	= THIS_MODULE,
	},
	.id_table	= MIPI_SN65DSI83_id,
};
#endif//

static struct platform_driver ej070na_lcd = {
	.probe	= ej070na_probe,
	.remove	= ej070na_remove,
	.driver	= {
		.name	= "ej070na_lcd",
		.owner	= THIS_MODULE,
	},
};

static __init int ej070na_init(void)
{
	printk("~ %s ~ \n", __func__);
#ifdef EJ070NA_MIPI_LCD
	i2c_add_driver(&MIPI_SN65DSI83_panel_i2c);
#endif//
	return platform_driver_register(&ej070na_lcd);
}

static __exit void ej070na_exit(void)
{
#ifdef EJ070NA_MIPI_LCD
	i2c_del_driver(&MIPI_SN65DSI83_panel_i2c);
#endif//
	platform_driver_unregister(&ej070na_lcd);
}

subsys_initcall(ej070na_init);
module_exit(ej070na_exit);

MODULE_DESCRIPTION("ej070na LCD driver");
MODULE_LICENSE("GPL");
