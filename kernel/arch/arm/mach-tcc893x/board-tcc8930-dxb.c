/* 
 * arch/arm/mach-tcc893x/board-tcc8930-dxb.c
 *
 * Author:  <linux@telechips.com>
 * Created: 10th Jun, 2008 
 * Description: Telechips Linux DxB Power Control
 *
 * Copyright (c) Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <asm/mach-types.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <mach/tcc_dxb_ctrl.h>
#include "board-tcc8930.h"

////////////////////////////////////////////////
#if defined(CONFIG_CHIP_TCC8930)
#define GPIO_DXB0_SCLK				TCC_GPB(1)
#define GPIO_DXB0_SFRM				TCC_GPB(2)
#define GPIO_DXB0_SDI				TCC_GPB(3)
#define GPIO_DXB0_SDO				TCC_GPB(0)
#define GPIO_DXB0_RST				TCC_GPC(21)
#define INT_DXB0_IRQ				TCC_GPB(15)
#define GPIO_DXB1_SCLK				TCC_GPG(0)
#define GPIO_DXB1_SFRM				TCC_GPG(1)
#define GPIO_DXB1_SDI				TCC_GPG(2)
#define GPIO_DXB1_SDO				TCC_GPG(3)
#define GPIO_DXB1_RST				TCC_GPC(24)
#define INT_DXB1_IRQ				TCC_GPE(31)

//Below pins are not used at system_rev == 0x1005
#define GPIO_DXB_UARTTX				TCC_GPA(24)
#define GPIO_DXB_UARTRX				TCC_GPA(25)
#elif defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8937S)
#define GPIO_DXB0_SCLK				TCC_GPD(8)
#define GPIO_DXB0_SFRM				TCC_GPD(9)
#define GPIO_DXB0_SDI				TCC_GPD(7)
#define GPIO_DXB0_SDO				TCC_GPD(10)
#define GPIO_DXB0_RST				TCC_GPG(3)
#define INT_DXB0_IRQ				TCC_GPD(6)
#define GPIO_DXB1_SCLK				TCC_GPE(12)
#define GPIO_DXB1_SFRM				TCC_GPE(13)
#define GPIO_DXB1_SDI				TCC_GPE(14)
#define GPIO_DXB1_SDO				TCC_GPE(15)
#define GPIO_DXB1_RST				TCC_GPG(4)
#define INT_DXB1_IRQ				TCC_GPG(19)

//Below pins are not used at system_rev == 0x1005
#define GPIO_DXB_UARTTX				TCC_GPE(28)
#define GPIO_DXB_UARTRX				TCC_GPE(29)
#elif defined(CONFIG_CHIP_TCC8933) || defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8937)
#define GPIO_DXB0_SCLK				TCC_GPD(8)
#define GPIO_DXB0_SFRM				TCC_GPD(9)
#define GPIO_DXB0_SDI				TCC_GPD(7)
#define GPIO_DXB0_SDO				TCC_GPD(10)
#define GPIO_DXB0_RST				TCC_GPG(3)
#define INT_DXB0_IRQ				TCC_GPD(6)
#define GPIO_DXB1_SCLK				TCC_GPE(20)
#define GPIO_DXB1_SFRM				TCC_GPE(18)
#define GPIO_DXB1_SDI				TCC_GPE(17)
#define GPIO_DXB1_SDO				TCC_GPE(19)
#define GPIO_DXB1_RST				TCC_GPG(4)
#define INT_DXB1_IRQ				TCC_GPG(19)

//Below pins are not used at system_rev == 0x1005
#define GPIO_DXB_UARTTX				TCC_GPE(28)
#define GPIO_DXB_UARTRX				TCC_GPE(29)
#else
	#error
#endif

/////////////////////////////////////////////////

static int gGPIO_DXB0_SCLK = GPIO_DXB0_SCLK;
static int gGPIO_DXB0_SFRM = GPIO_DXB0_SFRM;
static int gGPIO_DXB0_SDI = GPIO_DXB0_SDI;
static int gGPIO_DXB0_SDO = GPIO_DXB0_SDO;
static int gGPIO_DXB0_RST = GPIO_DXB0_RST;
static int gGPIO_DXB1_SCLK = GPIO_DXB1_SCLK;
static int gGPIO_DXB1_SFRM = GPIO_DXB1_SFRM;
static int gGPIO_DXB1_SDI = GPIO_DXB1_SDI;
static int gGPIO_DXB1_SDO = GPIO_DXB1_SDO;
static int gGPIO_DXB1_RST = GPIO_DXB1_RST;
static int gINT_DXB1_IRQ = INT_DXB1_IRQ;
static int gINT_DXB0_IRQ = INT_DXB0_IRQ;


static unsigned int guiBoardType = BOARD_TDMB_TCC3150;

static void tcc_dxb_ctrl_power_off(int deviceIdx)
{
	if(machine_is_tcc893x())
	{
		if(guiBoardType == BOARD_DXB_TCC3510 
		|| guiBoardType == BOARD_TDMB_TCC3161
		||  guiBoardType == BOARD_ISDBT_TCC353X)
		{
			switch(deviceIdx)
			{
			case 0:
				tcc_gpio_config(GPIO_DXB1_PD,  GPIO_FN(0));
				gpio_request(GPIO_DXB1_PD, NULL);
				gpio_direction_output(GPIO_DXB1_PD, 0);
				gpio_set_value(gGPIO_DXB0_RST, 0);
				break;
			case 1:
				tcc_gpio_config(GPIO_DXB2_PD,  GPIO_FN(0));
				gpio_request(GPIO_DXB2_PD, NULL);
				gpio_direction_output(GPIO_DXB2_PD, 0);
				gpio_set_value(gGPIO_DXB1_RST, 0);
				break;
			default:
				break;
			}
		}
		else if(guiBoardType == BOARD_DVBT_DIB9090)
		{
			tcc_gpio_config(GPIO_DXB1_PD,  GPIO_FN(0));
			gpio_request(GPIO_DXB1_PD, NULL);
			gpio_direction_output(GPIO_DXB1_PD, 0);
			gpio_set_value(gGPIO_DXB1_RST, 0);
		}
		else if(guiBoardType == BOARD_TDMB_TCC3150)
		{
			switch(deviceIdx)
			{
			case 0:
				gpio_set_value(GPIO_DXB1_PD, 0);
				gpio_set_value(gGPIO_DXB0_RST, 0);
				break;
			default:
				break;
			}
		}
		else if(guiBoardType == BOARD_DXB_TCC3171)
		{
			switch(deviceIdx)
			{
			case 0:
				tcc_gpio_config(GPIO_DXB1_PD,  GPIO_FN(0));
				gpio_request(GPIO_DXB1_PD, NULL);
				gpio_direction_output(GPIO_DXB1_PD, 0);
				gpio_set_value(gGPIO_DXB0_RST, 0);
				break;
			case 1:
				tcc_gpio_config(GPIO_DXB2_PD,  GPIO_FN(0));
				gpio_request(GPIO_DXB2_PD, NULL);
				gpio_direction_output(GPIO_DXB2_PD, 0);
				gpio_set_value(gGPIO_DXB1_RST, 0);
				break;
			default:
				break;
			}
		}

		/* GPIO_EXPAND DXB_ON Power-off */
		gpio_request(GPIO_DXB_ON,NULL);
		gpio_direction_output(GPIO_DXB_ON, 0);
		/* GPIO_EXPAND DEEPPWDN Power-off */
		gpio_request(GPIO_DXB1_PD,NULL);
		gpio_direction_output(GPIO_DXB1_PD, 0);
	}
}

static void tcc_dxb_ctrl_power_on(int deviceIdx)
{
	if(machine_is_tcc893x())
	{
		/* GPIO_EXPAND DXB_ON Power-on */
		gpio_request(GPIO_DXB_ON,NULL);
		gpio_direction_output(GPIO_DXB_ON, 1);

		/* GPIO_EXPAND DEEPPWDN Power-on */
		gpio_request(GPIO_DXB1_PD,NULL);
		gpio_direction_output(GPIO_DXB1_PD, 1);

		if(guiBoardType == BOARD_DXB_TCC3510 
			|| guiBoardType == BOARD_TDMB_TCC3161
			||  guiBoardType == BOARD_ISDBT_TCC353X)
		
		{
			switch(deviceIdx)
			{
			case 0:
				tcc_gpio_config(GPIO_DXB1_PD, GPIO_FN(0));
				tcc_gpio_config(gGPIO_DXB0_RST, GPIO_FN(0));

				gpio_request(GPIO_DXB1_PD, NULL);
				gpio_direction_output(GPIO_DXB1_PD, 0);
				gpio_set_value(gGPIO_DXB0_RST, 0);
				gpio_set_value(GPIO_DXB1_PD, 0);
				msleep (20);
				gpio_set_value(GPIO_DXB1_PD, 1);
				msleep (20);

				gpio_request(gGPIO_DXB0_RST, NULL);
				gpio_direction_output(gGPIO_DXB0_RST, 0);
				msleep (20);
				gpio_set_value(gGPIO_DXB0_RST, 1);
				break;
			case 1:
				tcc_gpio_config(GPIO_DXB2_PD,  GPIO_FN(0));
				tcc_gpio_config(gGPIO_DXB1_RST,  GPIO_FN(0));
	
				gpio_request(GPIO_DXB2_PD, NULL);
				gpio_direction_output(GPIO_DXB2_PD, 0);
	
				gpio_set_value(gGPIO_DXB1_RST, 0);
				gpio_set_value(GPIO_DXB2_PD, 0);
				msleep (20);
				gpio_set_value(GPIO_DXB2_PD, 1);
				msleep (20);
	
				gpio_request(gGPIO_DXB1_RST, NULL);
				gpio_direction_output(gGPIO_DXB1_RST, 0);
				msleep (20);
				gpio_set_value(gGPIO_DXB1_RST, 1);
				break;
			default:
				break;
			}
		}
		else if(guiBoardType == BOARD_DVBT_DIB9090)
		{
			tcc_gpio_config(GPIO_DXB1_PD, GPIO_FN(0));
			tcc_gpio_config(gGPIO_DXB1_RST, GPIO_FN(0));

			gpio_request(GPIO_DXB1_PD, NULL);
			gpio_direction_output(GPIO_DXB1_PD, 0);
			gpio_set_value(gGPIO_DXB1_RST, 0);
			gpio_set_value(GPIO_DXB1_PD, 0);
			msleep (20);
			gpio_set_value(GPIO_DXB1_PD, 1);
			msleep (20);

			gpio_request(gGPIO_DXB1_RST, NULL);
			gpio_direction_output(gGPIO_DXB1_RST, 0);
			msleep (20);
			gpio_set_value(gGPIO_DXB1_RST, 1);

			/* RF Control */
			tcc_gpio_config(gGPIO_DXB0_SDO,  GPIO_FN(0));
			gpio_request(gGPIO_DXB0_SDO, NULL);
			gpio_direction_output(gGPIO_DXB0_SDO, 0);
			gpio_set_value(gGPIO_DXB0_SDO, 1);
		}
		else if(guiBoardType == BOARD_TDMB_TCC3150)
		{
			switch(deviceIdx)
			{
			case 0:
				gpio_set_value(GPIO_DXB1_PD, 1);
				break;
			default:
				break;
			}
		}
		else if(guiBoardType == BOARD_DXB_TCC3171)
		{
			switch(deviceIdx)
			{
			case 0:
				tcc_gpio_config(GPIO_DXB1_PD, GPIO_FN(0));
				tcc_gpio_config(gGPIO_DXB0_RST, GPIO_FN(0));

				gpio_request(GPIO_DXB1_PD, NULL);
				gpio_direction_output(GPIO_DXB1_PD, 0);
				gpio_set_value(gGPIO_DXB0_RST, 0);
				gpio_set_value(GPIO_DXB1_PD, 0);
				msleep (20);
				gpio_set_value(GPIO_DXB1_PD, 1);
				msleep (20);

				gpio_request(gGPIO_DXB0_RST, NULL);
				gpio_direction_output(gGPIO_DXB0_RST, 0);
				msleep (20);
				gpio_set_value(gGPIO_DXB0_RST, 1);
				break;
			case 1:
				tcc_gpio_config(GPIO_DXB2_PD,  GPIO_FN(0));
				tcc_gpio_config(gGPIO_DXB1_RST,  GPIO_FN(0));
	
				gpio_request(GPIO_DXB2_PD, NULL);
				gpio_direction_output(GPIO_DXB2_PD, 0);
	
				gpio_set_value(gGPIO_DXB1_RST, 0);
				gpio_set_value(GPIO_DXB2_PD, 0);
				msleep (20);
				gpio_set_value(GPIO_DXB2_PD, 1);
				msleep (20);
	
				gpio_request(gGPIO_DXB1_RST, NULL);
				gpio_direction_output(gGPIO_DXB1_RST, 0);
				msleep (20);
				gpio_set_value(gGPIO_DXB1_RST, 1);
				break;
			default:
				break;
			}
		}
	}
}

static void tcc_dxb_ctrl_power_reset(int deviceIdx)
{
	if(machine_is_tcc893x())
	{
		if(guiBoardType == BOARD_DXB_TCC3510 
		|| guiBoardType == BOARD_TDMB_TCC3161
		||  guiBoardType == BOARD_ISDBT_TCC353X)
		{
			switch(deviceIdx)
			{
			case 0:
				tcc_gpio_config(gGPIO_DXB0_RST, GPIO_FN(0));
				gpio_request(gGPIO_DXB0_RST, NULL);
				gpio_direction_output(gGPIO_DXB0_RST, 0);
				msleep (20);
				gpio_set_value(gGPIO_DXB0_RST, 1);
				break;
			case 1:
				tcc_gpio_config(gGPIO_DXB1_RST, GPIO_FN(0));
				gpio_request(gGPIO_DXB1_RST, NULL);
				gpio_direction_output(gGPIO_DXB1_RST, 0);
				msleep (20);
				gpio_set_value(gGPIO_DXB1_RST, 1);
				break;
			default:
				break;
			}
		}
		else if(guiBoardType == BOARD_DVBT_DIB9090)
        {
           	tcc_gpio_config(gGPIO_DXB0_RST, GPIO_FN(0));
			gpio_request(gGPIO_DXB0_RST, NULL);
			gpio_direction_output(gGPIO_DXB0_RST, 0);
			msleep (100);
			gpio_set_value(gGPIO_DXB0_RST, 1);
			msleep (100);
        }
		else if(guiBoardType == BOARD_TDMB_TCC3150)	
		{
			switch(deviceIdx)
			{
			case 0:
				gpio_set_value(gGPIO_DXB0_RST, 0);
				msleep(100);		
				gpio_set_value(gGPIO_DXB0_RST, 1);
				msleep(100);		
				break;
			default:
				break;
			}
		}
		else if(guiBoardType == BOARD_DXB_TCC3171)
		{
			switch(deviceIdx)
			{
			case 0:
				tcc_gpio_config(gGPIO_DXB0_RST, GPIO_FN(0));
				gpio_request(gGPIO_DXB0_RST, NULL);
				gpio_direction_output(gGPIO_DXB0_RST, 0);
				msleep (20);
				gpio_set_value(gGPIO_DXB0_RST, 1);
				break;
			case 1:
				tcc_gpio_config(gGPIO_DXB1_RST, GPIO_FN(0));
				gpio_request(gGPIO_DXB1_RST, NULL);
				gpio_direction_output(gGPIO_DXB1_RST, 0);
				msleep (20);
				gpio_set_value(gGPIO_DXB1_RST, 1);
				break;
			default:
				break;
			}
		}
	}
}

static void tcc_dxb_ctrl_power_pure_on(int deviceIdx)
{
	if(machine_is_tcc893x())
	{
		/* GPIO_EXPAND DXB_ON Power-on */
		gpio_request(GPIO_DXB_ON,NULL);
		gpio_direction_output(GPIO_DXB_ON, 1);

		/* GPIO_EXPAND DEEPPWDN Power-on */
		gpio_request(GPIO_DXB1_PD,NULL);
		gpio_direction_output(GPIO_DXB1_PD, 1);

		
		if(guiBoardType == BOARD_DXB_TCC3171)
		{
			switch(deviceIdx)
			{
			case 0:			
				tcc_gpio_config(GPIO_DXB1_PD, GPIO_FN(0));
				gpio_request(GPIO_DXB1_PD, NULL);
				gpio_direction_output(GPIO_DXB1_PD, 0);
				gpio_set_value(GPIO_DXB1_PD, 0);
				msleep (20);
				gpio_set_value(GPIO_DXB1_PD, 1);
				msleep (20);				

				break;
			case 1:			
				tcc_gpio_config(GPIO_DXB2_PD,  GPIO_FN(0));	
				gpio_request(GPIO_DXB2_PD, NULL);
				gpio_direction_output(GPIO_DXB2_PD, 0);
	
				gpio_set_value(GPIO_DXB2_PD, 0);
				msleep (20);
				gpio_set_value(GPIO_DXB2_PD, 1);
				msleep (20);
				break;
			default:
				break;
			}
		}else
			printk("tcc_dxb_ctrl_power_pure_on :: Wrong Board Type (%d) \n",guiBoardType);		
	}
}

static void tcc_dxb_ctrl_power_pure_off(int deviceIdx)
{
	if(machine_is_tcc893x())
	{
		if(guiBoardType == BOARD_DXB_TCC3171)
		{
			switch(deviceIdx)
			{
			case 0:				
				tcc_gpio_config(GPIO_DXB1_PD,  GPIO_FN(0));
				gpio_request(GPIO_DXB1_PD, NULL);
				gpio_direction_output(GPIO_DXB1_PD, 0);
				gpio_set_value(GPIO_DXB1_PD, 0);
				break;
			case 1:				
				tcc_gpio_config(GPIO_DXB2_PD,  GPIO_FN(0));
				gpio_request(GPIO_DXB2_PD, NULL);
				gpio_direction_output(GPIO_DXB2_PD, 0);
				gpio_set_value(GPIO_DXB2_PD, 0);
				break;
			default:
				break;
			}
		}else
			printk("tcc_dxb_ctrl_power_pure_off :: Wrong Board Type (%d) \n",guiBoardType);	

		/* GPIO_EXPAND DXB_ON Power-off */
		gpio_request(GPIO_DXB_ON,NULL);
		gpio_direction_output(GPIO_DXB_ON, 0);
		/* GPIO_EXPAND DEEPPWDN Power-off */
		gpio_request(GPIO_DXB1_PD,NULL);
		gpio_direction_output(GPIO_DXB1_PD, 0);
	}
}

static void tcc_dxb_ctrl_power_reset_low(int deviceIdx)
{
	if(machine_is_tcc893x())
	{
		if(guiBoardType == BOARD_DXB_TCC3171)
		{
			switch(deviceIdx)
			{
			case 0:				
				tcc_gpio_config(gGPIO_DXB0_RST, GPIO_FN(0));
				gpio_request(gGPIO_DXB0_RST, NULL);
				gpio_direction_output(gGPIO_DXB0_RST, 0);
				msleep (20);
				gpio_set_value(gGPIO_DXB0_RST, 0);
				break;
			case 1:				
				tcc_gpio_config(gGPIO_DXB1_RST, GPIO_FN(0));
				gpio_request(gGPIO_DXB1_RST, NULL);
				gpio_direction_output(gGPIO_DXB1_RST, 0);
				msleep (20);
				gpio_set_value(gGPIO_DXB1_RST, 0);
				break;
			default:
				break;
			}
		}
	}
}

static void tcc_dxb_ctrl_power_reset_high(int deviceIdx)
{	
	if(machine_is_tcc893x())
	{
		if(guiBoardType == BOARD_DXB_TCC3171)
		{
			switch(deviceIdx)
			{
			case 0:
				tcc_gpio_config(gGPIO_DXB0_RST, GPIO_FN(0));
				gpio_request(gGPIO_DXB0_RST, NULL);
				gpio_direction_output(gGPIO_DXB0_RST, 0);
				msleep (20);
				gpio_set_value(gGPIO_DXB0_RST, 1);
				break;
			case 1:
				tcc_gpio_config(gGPIO_DXB1_RST, GPIO_FN(0));
				gpio_request(gGPIO_DXB1_RST, NULL);
				gpio_direction_output(gGPIO_DXB1_RST, 0);
				msleep (20);
				gpio_set_value(gGPIO_DXB1_RST, 1);
				break;
			default:
				break;
			}
		}
	}
}

static void tcc_dxb_ctrl_set_board(unsigned int uiboardtype)
{
	guiBoardType = uiboardtype;
	if(guiBoardType == BOARD_TDMB_TCC3150)
	{
		#if !defined(CONFIG_PN544_NFC) // This GPIO is used with the NFC demo board on TCC demo board
		tcc_gpio_config(gINT_DXB0_IRQ,  GPIO_FN(0));
		gpio_request(gINT_DXB0_IRQ, NULL);
		gpio_direction_input(gINT_DXB0_IRQ);
		#endif

		tcc_gpio_config(GPIO_DXB1_PD,  GPIO_FN(0));
		gpio_request(GPIO_DXB1_PD, NULL);
		gpio_direction_output(GPIO_DXB1_PD, 0);

		tcc_gpio_config(gGPIO_DXB0_RST,	GPIO_FN(0));
		gpio_request(gGPIO_DXB0_RST, NULL);
		gpio_direction_output(gGPIO_DXB0_RST, 0);
	}
}

static void tcc_dxb_ctrl_get_info(ST_CTRLINFO_ARG *pstCtrlInfo)
{
	if(guiBoardType == BOARD_DVBT_DIB9090M_PA)
		pstCtrlInfo->uiI2C = 4; //this is only for tcc9300cm
	else	
		pstCtrlInfo->uiI2C = 1;
}

static void tcc_dxb_ctrl_set_ctrl_mode(unsigned int flag)
{
}

static void tcc_dxb_ctrl_rf_path(unsigned int flag)
{
}

static void tcc_dxb_init(void)
{
	if(machine_is_tcc893x())
	{
	}
}

static struct tcc_dxb_platform_data tcc_dxb_pdata = {
	.init		= tcc_dxb_init,
	.power_off	= tcc_dxb_ctrl_power_off,
	.power_on	= tcc_dxb_ctrl_power_on,
	.power_pure_off     = tcc_dxb_ctrl_power_pure_off, // LDO Control Only
	.power_pure_on     = tcc_dxb_ctrl_power_pure_on, // LDO Control Only
	.power_reset 	= tcc_dxb_ctrl_power_reset,
	.power_reset_low 	= tcc_dxb_ctrl_power_reset_low, // Reset Type Dual control
	.power_reset_high = tcc_dxb_ctrl_power_reset_high, // Reset Type Dual control
	.rf_path	= tcc_dxb_ctrl_rf_path,
	.set_board	= tcc_dxb_ctrl_set_board,
	.get_info	= tcc_dxb_ctrl_get_info,
    .set_ctrl_mode = tcc_dxb_ctrl_set_ctrl_mode,
};

static struct platform_device tcc_dxb_device = {
	.name	= "tcc_dxb_ctrl",
	.dev	= {
		.platform_data	= &tcc_dxb_pdata,
	},
};

static int __init tcc8930_init_tcc_dxb_ctrl(void)
{
	if(!machine_is_tcc893x() || machine_is_m801_88())
		return 0;

	printk("%s (built %s %s)\n", __func__, __DATE__, __TIME__);
	platform_device_register(&tcc_dxb_device);
	return 0;
}
device_initcall(tcc8930_init_tcc_dxb_ctrl);
