/****************************************************************************
FileName    : kernel/arch/arm//mach-tcc892x/board-tcc8920st-dxb.c
Description : 

Copyright (C) 2013 Telechips Inc.

This program is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA 02111-1307 USA
****************************************************************************/

#include <asm/mach-types.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <mach/tcc_dxb_ctrl.h>
#include <mach/tca_tsif.h>
#include "board-tcc8920st.h"

// DxB
#ifdef  SUPPORT_STB_TSIF_INTERFACE
////////////////////////////////////////
#define GPIO_DXB_ON		    TCC_GPD(6)

#define GPIO_DXB0_PD		TCC_GPD(2)
#define GPIO_DXB1_PD		TCC_GPD(3)

#define GPIO_DXB0_RST		TCC_GPD(4)
#define GPIO_DXB1_RST		TCC_GPD(5)
#define INT_DXB0_IRQ		TCC_GPD(0)
#define INT_DXB1_IRQ		TCC_GPD(1)

#define GPIO_DXB0_SFRM		TCC_GPD(9)
#define GPIO_DXB0_SCLK		TCC_GPD(8)
#define GPIO_DXB0_SDI		TCC_GPD(7)
#define GPIO_DXB0_SDO		TCC_GPD(10)

#define GPIO_DXB1_SFRM		TCC_GPB(11)
#define GPIO_DXB1_SCLK		TCC_GPB(13)
#define GPIO_DXB1_SDI		TCC_GPB(14)
#define GPIO_DXB1_SDO		TCC_GPB(12)

#define	GPIO_HDB892F_DXB_PD	TCC_GPD(4)
#define	GPIO_HDB892F_DXB_RST	TCC_GPD(5)
////////////////////////////////////////
#else
////////////////////////////////////////
#define GPIO_DXB_ON		    TCC_GPD(15)

#define GPIO_DXB0_PD		TCC_GPF(4)
#define GPIO_DXB1_PD		TCC_GPD(16)

#define GPIO_DXB0_RST		TCC_GPF(6)
#define GPIO_DXB1_RST		TCC_GPD(21)
#define INT_DXB0_IRQ		TCC_GPB(15)
#define INT_DXB1_IRQ		TCC_GPE(30)

#define GPIO_DXB0_SFRM		TCC_GPD(20)
#define GPIO_DXB0_SCLK		TCC_GPD(18)
#define GPIO_DXB0_SDI		TCC_GPD(17)
#define GPIO_DXB0_SDO		TCC_GPD(19)

#define GPIO_DXB1_SFRM		TCC_GPB(11)
#define GPIO_DXB1_SCLK		TCC_GPB(13)
#define GPIO_DXB1_SDI		TCC_GPB(14)
#define GPIO_DXB1_SDO		TCC_GPB(12)
////////////////////////////////////////
#endif

#define PWRCTRL_OFF     0
#define PWRCTRL_ON      1
#define PWRCTRL_AUTO    2

static unsigned int guiBoardType = BOARD_TDMB_TCC3150;
static unsigned int guiCtrlMode = PWRCTRL_AUTO;

int tcc_dxb_ctrl_interface(void)
{
    //0:Serial TSIF 1:Parallel TSIF 2:SPI
    if(guiBoardType == BOARD_DVBT_MXL101SF_YJ || guiBoardType == BOARD_DVBT2_MN88472_YJ)
        return 1;//Parallel TSIF

    return 0; //Serial TSIF
}

static void tcc_dxb_ctrl_power_off(int deviceIdx)
{
#ifdef      CONFIG_STB_BOARD_HDB892F
	if (guiBoardType == BOARD_ISDBT_TCC353X) {
		//tcc_gpio_config (GPIO_HDB892F_DXB_RST, GPIO_FN(0));
		//gpio_request(GPIO_HDB892F_DXB_RST, NULL);
		//gpio_direction_output(GPIO_HDB892F_DXB_RST, 0);

		//tcc_gpio_config(GPIO_HDB892F_DXB_PD,  GPIO_FN(0));
		//gpio_request(GPIO_HDB892F_DXB_PD, NULL);
		//gpio_direction_output(GPIO_HDB892F_DXB_PD, 0);

		//tcc_gpio_config(GPIO_DXB_ON,  GPIO_FN(0));
		//gpio_request(GPIO_DXB_ON, NULL);
		//gpio_direction_output(GPIO_DXB_ON, 0); //GPIO_DXB_ON Set Output Mode
	} else {
		free_irq(INT_EI3, NULL);
		gpio_set_value(TCC_GPG(18), 0);
	}
#else    
	if(machine_is_tcc8920st())
	{
		if(guiBoardType == BOARD_DXB_TCC3510 
		|| guiBoardType == BOARD_TDMB_TCC3161
		||  guiBoardType == BOARD_ISDBT_TCC353X)
		{
			switch(deviceIdx)
			{
			case 0:
				tcc_gpio_config(GPIO_DXB_ON,  GPIO_FN(0));
				tcc_gpio_config(GPIO_DXB0_PD,  GPIO_FN(0));
				gpio_request(GPIO_DXB0_PD, NULL);
				gpio_direction_output(GPIO_DXB0_PD, 0); //TCC3511_PD Set Output Mode
				gpio_request(GPIO_DXB_ON, NULL);
				gpio_direction_output(GPIO_DXB_ON, 0); //GPIO_DXB_ON Set Output Mode
				break;
			case 1:			
				break;
			default:
				break;
			}
		}
		else if(guiBoardType == BOARD_DVBT_DIB9090)
		{        
		}
		else if(guiBoardType == BOARD_TDMB_TCC3150)
		{
			switch(deviceIdx)
			{
			case 0:		
				break;
			default:
				break;
			}
		}
	}
#endif	
}

static irqreturn_t isr_checking_ant_overload(int irq, void *dev_data)
{
    printk("%s : ant overload = %d\n", __func__, gpio_get_value(TCC_GPG(19)));	
    if(gpio_get_value(TCC_GPG(19)) == 1)
        gpio_set_value(TCC_GPG(18), 1);	
    else
        gpio_set_value(TCC_GPG(18), 0);	
	return IRQ_HANDLED;
}


static void tcc_dxb_ctrl_power_on(int deviceIdx)
{
#ifdef      CONFIG_STB_BOARD_HDB892F
	int err = -ENOMEM;
	if (guiBoardType == BOARD_ISDBT_TCC353X) {
		tcc_gpio_config(GPIO_DXB_ON,  GPIO_FN(0));
		gpio_request(GPIO_DXB_ON, NULL);
		gpio_direction_output(GPIO_DXB_ON, 1);

		tcc_gpio_config(GPIO_HDB892F_DXB_PD,  GPIO_FN(0));
		gpio_request(GPIO_HDB892F_DXB_PD, NULL);
		gpio_direction_output(GPIO_HDB892F_DXB_PD, 1);

		tcc_gpio_config(GPIO_HDB892F_DXB_RST,  GPIO_FN(0));
		gpio_request(GPIO_HDB892F_DXB_RST, NULL);
		gpio_direction_output(GPIO_HDB892F_DXB_RST, 0);
		msleep (20);
		gpio_set_value(GPIO_HDB892F_DXB_RST, 1);
	} else {
		switch (guiCtrlMode)
		{
		case PWRCTRL_OFF:
			break;
		case PWRCTRL_ON:
			tcc_gpio_config(TCC_GPG(18), GPIO_FN(0)|GPIO_PULL_DISABLE); //ANT_PWR_CTRL
			gpio_request(TCC_GPG(18), NULL);
			gpio_direction_output(TCC_GPG(18), 0);

			gpio_set_value(TCC_GPG(18), 1);
			break;
		case PWRCTRL_AUTO:
			tcc_gpio_config(TCC_GPG(18), GPIO_FN(0)|GPIO_PULL_DISABLE); //ANT_PWR_CTRL
			gpio_request(TCC_GPG(18), NULL);
			gpio_direction_output(TCC_GPG(18), 0);
			
			tcc_gpio_config(TCC_GPG(19), GPIO_FN(0)|GPIO_PULL_DISABLE); //ANT_OVERLOAD
			gpio_request(TCC_GPG(19), NULL);
			gpio_direction_input(TCC_GPG(19));

			gpio_set_value(TCC_GPG(18), 1);
			msleep (5);
			if(gpio_get_value(TCC_GPG(19)) == 0)
			{
				printk("ANT_OVERLOAD is low\n");
				gpio_set_value(TCC_GPG(18), 0);
				return;
			}

			//interrupt setting for ANT_OVERLOAD G18
			tcc_gpio_config_ext_intr(INT_EI3, EXTINT_GPIOG_19);
			err = request_irq(INT_EI3, isr_checking_ant_overload, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_DISABLED,
				  "ANT_OVERLOAD", NULL);
			if (err) {
				printk("Unable to request ANT_OVERLOAD IRQ.\n");
			}
			break;
		}
	}
#else    
	if(machine_is_tcc8920st())
	{
		if(guiBoardType == BOARD_DXB_TCC3510 
			|| guiBoardType == BOARD_TDMB_TCC3161
			||  guiBoardType == BOARD_ISDBT_TCC353X)
		
		{
			switch(deviceIdx)
			{
			case 0:
				tcc_gpio_config(GPIO_DXB_ON,  GPIO_FN(0));
				gpio_request(GPIO_DXB_ON, NULL);
				gpio_direction_output(GPIO_DXB_ON, 1);
	
				tcc_gpio_config(GPIO_DXB0_PD,  GPIO_FN(0));
				gpio_request(GPIO_DXB0_PD, NULL);
				gpio_direction_output(GPIO_DXB0_PD, 1);
	
				/* Reset# */
				tcc_gpio_config(GPIO_DXB0_RST,  GPIO_FN(0));
				gpio_request(GPIO_DXB0_RST, NULL);
				gpio_direction_output(GPIO_DXB0_RST, 0);
				msleep (20);
				gpio_set_value(GPIO_DXB0_RST, 1);	
				break;
			case 1:
				break;
			default:
				break;
			}
		}
		else if(guiBoardType == BOARD_DVBT_DIB9090)
        {
        }
		else if(guiBoardType == BOARD_TDMB_TCC3150)
		{
			switch(deviceIdx)
			{
			case 0:
				break;
			default:
				break;
			}
		}
	}
#endif	
}

static void tcc_dxb_ctrl_power_reset(int deviceIdx)
{
#ifdef      CONFIG_STB_BOARD_HDB892F
	if (guiBoardType == BOARD_ISDBT_TCC353X)
	{
		tcc_gpio_config(GPIO_HDB892F_DXB_RST, GPIO_FN(0));
		gpio_direction_output(GPIO_HDB892F_DXB_RST, 0);
		msleep (20);
		gpio_set_value(GPIO_HDB892F_DXB_RST, 1);
	}
#else    
	if(machine_is_tcc8920st())
	{
		if(guiBoardType == BOARD_DXB_TCC3510 
		|| guiBoardType == BOARD_TDMB_TCC3161
		||  guiBoardType == BOARD_ISDBT_TCC353X)
		{
			switch(deviceIdx)
			{
			case 0:
				tcc_gpio_config(GPIO_DXB0_RST, GPIO_FN(0));
				gpio_request(GPIO_DXB0_RST, NULL);
				gpio_direction_output(GPIO_DXB0_RST, 0);
				msleep (20);
				gpio_set_value(GPIO_DXB0_RST, 1);
				break;
			case 1:
				break;
			default:
				break;
			}
		}
		else if(guiBoardType == BOARD_DVBT_DIB9090)
        {
           	
        }
		else if(guiBoardType == BOARD_TDMB_TCC3150)	
		{
			switch(deviceIdx)
			{
			case 0:
			
				break;
			default:
				break;
			}
		}
	}
#endif	
}

static void tcc_dxb_ctrl_set_board(unsigned int uiboardtype)
{
	guiBoardType = uiboardtype;
	printk("%s[%d]\n", __func__, guiBoardType);
}

static void tcc_dxb_ctrl_get_info(ST_CTRLINFO_ARG *pstCtrlInfo)
{
#ifdef      CONFIG_STB_BOARD_HDB892F
    pstCtrlInfo->uiI2C = 1;
#else    
	if(guiBoardType == BOARD_DVBT_DIB9090M_PA)
		pstCtrlInfo->uiI2C = 4; //this is only for tcc9300cm
	else	
		pstCtrlInfo->uiI2C = 0;
#endif	
}

static void tcc_dxb_ctrl_set_ctrl_mode(unsigned int flag)
{
	printk("[%s] Mode change %d -> %d\n", __func__, guiCtrlMode, flag);
	guiCtrlMode = flag;
}

static void tcc_dxb_ctrl_rf_path(unsigned int flag)
{
}

static void tcc_dxb_init(void)
{
	if(machine_is_tcc8920st())
	{
		/*PULL_UP is disabled to save current.*/
		
		//TCC_GPE(2)
		tcc_gpio_config(GPIO_DXB0_SFRM, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB0_SFRM, NULL);
		gpio_direction_output(GPIO_DXB0_SFRM, 0);
	
		//TCC_GPE(3)
		tcc_gpio_config(GPIO_DXB0_SCLK, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB0_SCLK, NULL);
		gpio_direction_output(GPIO_DXB0_SCLK, 0);
	
		//TCC_GPE(4)
		tcc_gpio_config(GPIO_DXB0_RST, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB0_RST, NULL);
		gpio_direction_output(GPIO_DXB0_RST, 0);
	
		//TCC_GPE(5)
		tcc_gpio_config(INT_DXB0_IRQ, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(INT_DXB0_IRQ, NULL);
		gpio_direction_output(INT_DXB0_IRQ, 0);
	
		//TCC_GPE(6)
		tcc_gpio_config(GPIO_DXB0_SDI, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB0_SDI, NULL);
		gpio_direction_output(GPIO_DXB0_SDI, 0);
	
		//TCC_GPE(7)
		tcc_gpio_config(GPIO_DXB0_SDO, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB0_SDO, NULL);
		gpio_direction_output(GPIO_DXB0_SDO, 0);
	
		//TCC_GPD(5)
		tcc_gpio_config(GPIO_DXB1_SFRM, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB1_SFRM, NULL);
		gpio_direction_output(GPIO_DXB1_SFRM, 0);
	
		//TCC_GPD(6)
		tcc_gpio_config(GPIO_DXB1_SCLK, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB1_SCLK, NULL);
		gpio_direction_output(GPIO_DXB1_SCLK, 0);
	
		//TCC_GPD(7)
		tcc_gpio_config(GPIO_DXB1_SDI, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB1_SDI, NULL);
		gpio_direction_output(GPIO_DXB1_SDI, 0);
	
		//TCC_GPD(8)
		tcc_gpio_config(GPIO_DXB1_SDO, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB1_SDO, NULL);
		gpio_direction_output(GPIO_DXB1_SDO, 0);
	
		//TCC_GPD(9)
		tcc_gpio_config(GPIO_DXB1_RST, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB1_RST, NULL);
		gpio_direction_output(GPIO_DXB1_RST, 0);
	
		//TCC_GPD(10)
		tcc_gpio_config(INT_DXB1_IRQ, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(INT_DXB1_IRQ, NULL);
		gpio_direction_output(INT_DXB1_IRQ, 0);
	
#if 0	
		//smart Card Interface for CAS
		tcc_gpio_config(TCC_GPD(13), GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(TCC_GPD(13), NULL);
		gpio_direction_output(TCC_GPD(13), 0);
	
		//smart Card Interface for CAS
		tcc_gpio_config(TCC_GPD(14), GPIO_FN(0)|GPIO_PULL_DISABLE);//
		gpio_request(TCC_GPD(14), NULL);
		gpio_direction_output(TCC_GPD(14), 0);
#endif /* 0 */

//////////////////////////////////////////////////////////////////////////////////////////////////////

		tcc_gpio_config(GPIO_DXB0_PD, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB0_PD, NULL);
		gpio_direction_output(GPIO_DXB0_PD, 0);

		tcc_gpio_config(GPIO_DXB_ON, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB_ON, NULL);
		gpio_direction_output(GPIO_DXB_ON, 0);
		
#if 0
		tcc_gpio_config(GPIO_DXB_UARTTX, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB_UARTTX, NULL);
		gpio_direction_output(GPIO_DXB_UARTTX, 0);

		//TCC_GPE(5)
		tcc_gpio_config(GPIO_DXB_UARTRX, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB_UARTRX, NULL);
		gpio_direction_output(GPIO_DXB_UARTRX, 0);
#endif
	}
}

static struct tcc_dxb_platform_data tcc_dxb_pdata = {
	.init		= tcc_dxb_init,
	.power_off	= tcc_dxb_ctrl_power_off,
	.power_on	= tcc_dxb_ctrl_power_on,
	.power_reset= tcc_dxb_ctrl_power_reset,
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

static int __init tcc8920_init_tcc_dxb_ctrl(void)
{
	if(!machine_is_tcc8920st() || machine_is_m801_88())
		return 0;

	printk("%s (built %s %s)\n", __func__, __DATE__, __TIME__);
	platform_device_register(&tcc_dxb_device);
	return 0;
}
device_initcall(tcc8920_init_tcc_dxb_ctrl);
