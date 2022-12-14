/* linux/arch/arm/mach-tcc892x/board-m805_892x-panel.c
 *
 * Copyright (C) 2010 Telechips, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/gpio.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/system.h>

#include <mach/gpio.h>
#include <mach/tcc_fb.h>
#include "devices.h"
#include "board-m805_892x.h"
#include <mach/tca_display_config.h>

#define DEFAULT_BACKLIGHT_BRIGHTNESS	102

static struct lcd_platform_data lcd_pdata = {
	.power_on	= GPIO_LCD_ON,
	.display_on	= GPIO_LCD_DISPLAY,
	.bl_on		= GPIO_LCD_BL,
	.reset		= GPIO_LCD_RESET,
};


static struct lcd_platform_data lvds_pdata = {
#if (0) //bscho_temp
	.power_on	= GPIO_LVDS_EN,
	.display_on	= GPIO_LVDSIVT_ON,
	.bl_on		= GPIO_LVDS_LP_CTRL,
	.reset		= GPIO_LVDS_LP_CTRL,
#endif
};


#ifdef CONFIG_LCD_LMS350DF01
static struct platform_device lms350df01_lcd = {
	.name	= "lms350df01_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif

#ifdef CONFIG_LCD_LMS480KF01
static struct platform_device lms480kf01_lcd = {
	.name	= "lms480kf01_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif

#ifdef CONFIG_LCD_DX08D11VM0AAA
static struct platform_device dx08d11vm0aaa_lcd = {
	.name	= "dx08d11vm0aaa_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif

#ifdef CONFIG_LCD_LB070WV6
static struct platform_device lb070wv6_lcd = {
	.name	= "lb070wv6_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif

#ifdef CONFIG_LCD_CLAA104XA01CW
static struct platform_device claa104xa01cw_lcd = {
	.name	= "claa104xa01cw_lcd",
	.dev	= {
		.platform_data	= &lvds_pdata,
	},
};
#endif

#ifdef CONFIG_LCD_HT121WX2
static struct platform_device ht121wx2_lcd = {
	.name	= "ht121wx2_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif

#ifdef CONFIG_LCD_TD043MGEB1
static struct platform_device td043mgeb1_lcd = {
	.name	= "td043mgeb1_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif

#ifdef CONFIG_LCD_TD070RDH 
static struct platform_device tm070rdh113_lcd = {
	.name	= "tm070rdh11_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif//CONFIG_LCD_TD070RDH 

#ifdef CONFIG_LCD_AT070TN93
static struct platform_device at070tn93_lcd = {
	.name	= "at070tn93_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif

#ifdef CONFIG_LCD_HDMI1280X720
static struct platform_device hdmi1280x720_lcd = {
	.name	= "hdmi1280x720_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif

#ifdef CONFIG_LCD_N101L6
static struct platform_device n101l6_lcd = {
	.name	= "n101l6_lcd",
	.dev	= {
		.platform_data	= &lvds_pdata,
	},
};
#endif

#ifdef CONFIG_LCD_CLAA102NA0DCW
static struct platform_device claa102na0dcw_lcd= {
	.name	= "claa102na0dcw_lcd",
	.dev	= {
		.platform_data	= &lvds_pdata,
	},
};
#endif
#ifdef CONFIG_LCD_ED090NA
static struct platform_device ed090na_lcd= {
	.name	= "ed090na_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif//

#ifdef CONFIG_LCD_KR080PA2S
static struct platform_device kr080pa2s_lcd = {
	.name	= "kr080pa2s_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif

#ifdef CONFIG_LCD_CLAA070NP01
static struct platform_device claa070np01_lcd = {
	.name	= "claa070np01_lcd",
	.dev	= {
		.platform_data	= &lvds_pdata,
	},
};

#endif
#ifdef CONFIG_LCD_HV070WSA
static struct platform_device hv070wsa_lcd = {
	.name	= "hv070wsa_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif
#ifdef CONFIG_LCD_FLD0800
static struct platform_device fld0800_lcd = {
        .name   = "fld0800_lcd",
        .dev    = {
                .platform_data  = &lcd_pdata,
        },
};
#endif
#ifdef CONFIG_LCD_CLAA070WP03
static struct platform_device claa070wp03_lcd = {
        .name   = "claa070wp03_lcd",
        .dev    ={
                .platform_data  = &lcd_pdata,
        },
};
#endif
#ifdef CONFIG_LCD_EJ070NA
static struct platform_device ej070na_lcd = {
	.name	= "ej070na_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif

static void m805_892x_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	struct lcd_panel *lcd_panel = tccfb_get_panel();

	if (lcd_panel->set_backlight_level)
		lcd_panel->set_backlight_level(lcd_panel, value);
}

static struct led_classdev m805_892x_backlight_led = {
	.name		= "lcd-backlight",
	.brightness	= DEFAULT_BACKLIGHT_BRIGHTNESS,
	.brightness_set	= m805_892x_brightness_set,
};

static int m805_892x_backlight_probe(struct platform_device *pdev)
{
	gpio_request(lcd_pdata.power_on, "lcd_on");
	gpio_request(lcd_pdata.bl_on, "lcd_bl");
	gpio_request(lcd_pdata.display_on, "lcd_display");
	gpio_request(lcd_pdata.reset, "lcd_reset");

	gpio_direction_output(lcd_pdata.power_on, 1);
	gpio_direction_output(lcd_pdata.bl_on, 1);
	gpio_direction_output(lcd_pdata.display_on, 1);
	gpio_direction_output(lcd_pdata.reset, 1);
	
	return led_classdev_register(&pdev->dev, &m805_892x_backlight_led);
}

static int m805_892x_backlight_remove(struct platform_device *pdev)
{
	led_classdev_unregister(&m805_892x_backlight_led);
	return 0;
}

static struct platform_driver m805_892x_backlight_driver = {
	.probe	= m805_892x_backlight_probe,
	.remove	= m805_892x_backlight_remove,
	.driver	= {
		.name	= "m805_892x-backlight",
		.owner	= THIS_MODULE,
	},
};

static struct platform_device m805_892x_backlight = {
	.name = "m805_892x-backlight",
};

int __init m805_892x_init_panel(void)
{
	unsigned int lcd_lcdc_num = 0;
	int ret;
	if (!machine_is_m805_892x())
		return 0;

	lcd_lcdc_num = tca_get_lcd_lcdc_num();

	lcd_pdata.lcdc_num	= lcd_lcdc_num;	
	if(system_rev == 0x2002 || system_rev == 0x2003 || system_rev == 0x2004 || system_rev == 0x2005 || system_rev == 0x2006 || system_rev == 0x2007 || system_rev == 0x2008 || system_rev == 0x2009){
		lcd_pdata.bl_on 		= TCC_GPC(0);
		lcd_pdata.lcdc_num	= 0;	
	}
	
	printk("supported LCD panel type %d  system_rev:0x%x \n", tcc_panel_id, system_rev);

	switch (tcc_panel_id) {
#ifdef CONFIG_LCD_LMS350DF01
	case PANEL_ID_LMS350DF01:
		platform_device_register(&lms350df01_lcd);
		break;
#endif

#ifdef CONFIG_LCD_LMS480KF01
	case PANEL_ID_LMS480KF01:
		platform_device_register(&lms480kf01_lcd);
		break;
#endif

#ifdef CONFIG_LCD_DX08D11VM0AAA
	case PANEL_ID_DX08D11VM0AAA:
		platform_device_register(&dx08d11vm0aaa_lcd);
		break;
#endif

#ifdef CONFIG_LCD_LB070WV6
	case PANEL_ID_LB070WV6:
		platform_device_register(&lb070wv6_lcd);
		break;
#endif

#ifdef CONFIG_LCD_CLAA104XA01CW
	case PANEL_ID_CLAA104XA01CW:
		platform_device_register(&claa104xa01cw_lcd);
		break;
#endif

#ifdef CONFIG_LCD_HT121WX2
	case PANEL_ID_HT121WX2:
		platform_device_register(&ht121wx2_lcd);
		break;
#endif

#ifdef CONFIG_LCD_TD043MGEB1
	case PANEL_ID_TD043MGEB1:
		platform_device_register(&td043mgeb1_lcd);
		break;
#endif

#ifdef CONFIG_LCD_AT070TN93
	case PANEL_ID_AT070TN93:
		platform_device_register(&at070tn93_lcd);
		break;
#endif

#ifdef CONFIG_LCD_HDMI1280X720
	case PANEL_ID_HDMI:
 		platform_device_register(&hdmi1280x720_lcd);
		break;
#endif

#ifdef CONFIG_LCD_TD070RDH 
	case PANEL_ID_TD070RDH11:
		platform_device_register(&tm070rdh113_lcd);
		
		break;
#endif//

#ifdef CONFIG_LCD_N101L6
	case PANEL_ID_N101L6:
		platform_device_register(&n101l6_lcd);
		break;
#endif//

#ifdef CONFIG_LCD_CLAA102NA0DCW
	case PANEL_ID_CLAA102NA0DCW:
		platform_device_register(&claa102na0dcw_lcd);
		break;
#endif//
#ifdef CONFIG_LCD_ED090NA
	case PANEL_ID_ED090NA:
		platform_device_register(&ed090na_lcd);
		break;
#endif//

#ifdef CONFIG_LCD_KR080PA2S
	case PANEL_ID_KR080PA2S:
		platform_device_register(&kr080pa2s_lcd);
		break;
#endif//

#ifdef CONFIG_LCD_CLAA070NP01
	case PANEL_ID_CLAA070NP01:
		platform_device_register(&claa070np01_lcd);
		break;
#endif//

#ifdef CONFIG_LCD_HV070WSA
	case PANEL_ID_HV070WSA:
		platform_device_register(&hv070wsa_lcd);
		break;
#endif
#ifdef CONFIG_LCD_FLD0800
	case PANEL_ID_FLD0800:
		platform_device_register(&fld0800_lcd);
		break;
#endif
#ifdef CONFIG_LCD_CLAA070WP03
        case PANEL_ID_CLAA070WP03:
                platform_device_register(&claa070wp03_lcd);
                break;
#endif
#ifdef CONFIG_LCD_EJ070NA
	case PANEL_ID_EJ070NA:
		platform_device_register(&ej070na_lcd);
		break;	
#endif


	default:
		pr_err("Not supported LCD panel type %d\n", tcc_panel_id);
		return -EINVAL;
	}

	ret = platform_device_register(&tcc_lcd_device);
	if (ret)
		return ret;

	platform_device_register(&m805_892x_backlight);
	ret = platform_driver_register(&m805_892x_backlight_driver);

	if (ret)
		return ret;

	return 0;
}



device_initcall(m805_892x_init_panel);
