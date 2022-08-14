/****************************************************************************
FileName    : kernel/arch/arm//mach-tcc893x/board-tcc8930st-dxb.c
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
#include <asm/system.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <mach/tcc_dxb_ctrl.h>
#include "board-tcc8930st.h"

#define SUPPORT_STB_TSIF_INTERFACE //Support TSIF interface at STB solution

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

#if defined(CONFIG_STB_BOARD_EV_TCC893X)
#define GPIO_DXB1_SFRM		TCC_GPB(12)
#define GPIO_DXB1_SCLK		TCC_GPB(11)
#define GPIO_DXB1_SDI		TCC_GPB(13)
#define GPIO_DXB1_SDO		TCC_GPB(14)
#else
#define GPIO_DXB1_SFRM		TCC_GPB(11)
#define GPIO_DXB1_SCLK		TCC_GPB(13)
#define GPIO_DXB1_SDI		TCC_GPB(14)
#define GPIO_DXB1_SDO		TCC_GPB(12)
#endif

#define	GPIO_DXB_YJ_PWDN	TCC_GPD(4)
#define GPIO_DXB_YJ_RST		TCC_GPD(5)

// DVBS2
#define GPIO_DXB_YJ_SP1V    TCC_GPB(25)
#define GPIO_DXB_YJ_S18V    TCC_GPB(26)
#define GPIO_DXB_YJ_SLEEP   TCC_GPB(12)
#define GPIO_DXB_YJ_LOCK    TCC_GPB(11)
#define GPIO_DXB_YJ_RESET   TCC_GPB(13)
#define GPIO_DXB_YJ_FAULT   TCC_GPB(14)
#define GPIO_DXB_YJ_VEN     TCC_GPD(23)

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

#define	MAX_DEVICE_NO	(3)
#define PWRCTRL_OFF     0
#define PWRCTRL_ON      1
#define PWRCTRL_AUTO    2

static int g_power_status[MAX_DEVICE_NO+1] = { 0,0,0,0 };	//0:pwr off state, 1:pwr on state
static unsigned int guiBoardType = BOARD_TDMB_TCC3150;
static unsigned int guiCtrlMode = PWRCTRL_AUTO;
static unsigned int guiIrq = 0;

int tcc_dxb_ctrl_interface(void)
{
    //0:Serial TSIF 1:Parallel TSIF 2:SPI
    if(guiBoardType == BOARD_DVBT_MXL101SF_YJ || guiBoardType == BOARD_DVBT2_MN88472_YJ || guiBoardType == BOARD_DVBS2_AVL6211_YJ)
        return 1;//Parallel TSIF

    return 0; //Serial TSIF
}

static void tcc_dxb_ctrl_power_off(int deviceIdx)
{

    int gpio_ant_pwr;

	if(machine_is_tcc8930st())
	{
		if(guiBoardType == BOARD_DXB_TCC3510 
		|| guiBoardType == BOARD_TDMB_TCC3161
		||  guiBoardType == BOARD_ISDBT_TCC353X)
		{
	if (guiBoardType == BOARD_ISDBT_TCC353X)
	{
#if defined(CONFIG_STB_BOARD_YJ8935T) || defined(CONFIG_STB_BOARD_YJ8930T) || defined(CONFIG_STB_BOARD_YJ8933T)                
		tcc_gpio_config(GPIO_DXB_YJ_PWDN, GPIO_FN(0));
		gpio_direction_output(GPIO_DXB_YJ_PWDN, 0);	//set PWDN# active
		tcc_gpio_config(GPIO_DXB_YJ_RST, GPIO_FN(0));
		gpio_direction_output(GPIO_DXB_YJ_RST, 0);	//set RST# active
		tcc_gpio_config(GPIO_DXB_ON, GPIO_FN(0));
		gpio_direction_output(GPIO_DXB_ON, 0);		//set LDO inactive
                return;
#endif                
	}

			if (g_power_status[deviceIdx]==0 || deviceIdx > MAX_DEVICE_NO)
				return;
			g_power_status[deviceIdx] = 0;
			switch(deviceIdx)
			{
			case 0:
				tcc_gpio_config(GPIO_DXB0_PD,  GPIO_FN(0));
				gpio_request(GPIO_DXB0_PD, NULL);
				gpio_direction_output(GPIO_DXB0_PD, 0); //TCC3511_PD Set Output Mode
				break;
			case 1:
				tcc_gpio_config(GPIO_DXB1_PD,  GPIO_FN(0));
				gpio_request(GPIO_DXB1_PD, NULL);
				gpio_direction_output(GPIO_DXB1_PD, 0); //TCC3511_PD Set Output Mode
				break;
			default:
				break;
			}
			if (g_power_status[0]==0 && g_power_status[1] && g_power_status[2]==0 && g_power_status[3]==0) {
				tcc_gpio_config(GPIO_DXB_ON,  GPIO_FN(0));
				gpio_request(GPIO_DXB_ON, NULL);
				gpio_direction_output(GPIO_DXB_ON, 0); //GPIO_DXB_ON Set Output Mode
			}
		}
        
        else if (guiBoardType == BOARD_DVBS2_AVL6211_YJ)
        {
        #if defined(CONFIG_STB_BOARD_YJ8935T) ||defined(CONFIG_STB_BOARD_YJ8930T) || defined(CONFIG_STB_BOARD_YJ8933T)
            //tcc_gpio_config(GPIO_DXB_YJ_SP1V, GPIO_FN(0));
            //gpio_direction_output(GPIO_DXB_YJ_SP1V, 0);
            tcc_gpio_config(GPIO_DXB_YJ_SLEEP, GPIO_FN(0));
            gpio_direction_output(GPIO_DXB_YJ_SLEEP, 1); // set demod inactive
            tcc_gpio_config(GPIO_DXB_YJ_VEN, GPIO_FN(0));
            gpio_direction_output(GPIO_DXB_YJ_VEN, 0); // set lnb inactive
        #endif
        }
        else if (guiBoardType == BOARD_DVBT_MXL101SF_YJ || guiBoardType == BOARD_DVBT2_MN88472_YJ)
        {
        #if defined(CONFIG_STB_BOARD_YJ8935T) || defined(CONFIG_STB_BOARD_YJ8933T)
            gpio_ant_pwr = TCC_GPG(18);
        #elif defined(CONFIG_STB_BOARD_YJ8930T)    
            gpio_ant_pwr = TCC_GPG(15);
        #else            
            //(TCC8930ST Main STB            
            gpio_ant_pwr = TCC_GPB(14);
            //Power Off
            tcc_gpio_config(TCC_GPE(28), GPIO_FN(0));
            gpio_request(TCC_GPE(28), NULL);
            gpio_direction_output(TCC_GPE(28), 0);		//set LDO inactive
        #endif    
            if (guiIrq)
                free_irq(guiIrq, NULL);
            guiIrq = 0;
            gpio_set_value(gpio_ant_pwr, 0);
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
}

static irqreturn_t isr_checking_ant_overload(int irq, void *dev_data)
{
    int gpio_ant_pwr, gpio_check_ant_overload;
   
#if defined(CONFIG_STB_BOARD_YJ8935T) || defined(CONFIG_STB_BOARD_YJ8933T)
    gpio_ant_pwr = TCC_GPG(18);
    gpio_check_ant_overload = TCC_GPG(19);    
#elif defined(CONFIG_STB_BOARD_YJ8930T)    
    gpio_ant_pwr = TCC_GPG(15);
    gpio_check_ant_overload = TCC_GPG(16);
#else    
    //(TCC8930ST Main STB
    gpio_ant_pwr = TCC_GPB(14);
    gpio_check_ant_overload = TCC_GPF(23);
#endif    
    printk("%s : ant overload = %d\n", __func__, gpio_get_value(gpio_check_ant_overload));	
    if(gpio_get_value(gpio_check_ant_overload) == 1)
    {
        gpio_set_value(gpio_ant_pwr, 1);	
    }
    else
    {
        gpio_set_value(gpio_ant_pwr, 0);	
    }
    return IRQ_HANDLED;
}

static void tcc_dxb_ctrl_power_on(int deviceIdx)
{
	int err = -ENOMEM;
	int gpio_ant_pwr, gpio_check_ant_overload, int_check_ant_overload;
	if(machine_is_tcc8930st())
	{
		if(guiBoardType == BOARD_DXB_TCC3510 
			|| guiBoardType == BOARD_TDMB_TCC3161
			||  guiBoardType == BOARD_ISDBT_TCC353X)
		
		{
#if defined(CONFIG_STB_BOARD_YJ8935T) || defined(CONFIG_STB_BOARD_YJ8930T) || defined(CONFIG_STB_BOARD_YJ8933T)
	if (guiBoardType == BOARD_ISDBT_TCC353X)
	{
		tcc_gpio_config(GPIO_DXB_ON, GPIO_FN(0));
		gpio_direction_output(GPIO_DXB_ON, 1);		//set LDO active
		tcc_gpio_config(GPIO_DXB_YJ_PWDN, GPIO_FN(0));
		gpio_direction_output(GPIO_DXB_YJ_PWDN, 1);	//set PWDN# inactive
		tcc_gpio_config(GPIO_DXB_YJ_RST, GPIO_FN(0));
		gpio_direction_output(GPIO_DXB_YJ_RST, 1);	//set RST# inactive
                return;
            }
#endif            

			if (deviceIdx > MAX_DEVICE_NO)	return;
			if (g_power_status[0]==0 && g_power_status[1]==0 && g_power_status[2]==0 && g_power_status[3]==0) {
				tcc_gpio_config(GPIO_DXB_ON,  GPIO_FN(0));
				gpio_request(GPIO_DXB_ON, NULL);
				gpio_direction_output(GPIO_DXB_ON, 1);
			}
			g_power_status[deviceIdx] = 1;
			switch(deviceIdx)
			{
			case 0:
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
				tcc_gpio_config(GPIO_DXB1_PD,  GPIO_FN(0));
				gpio_request(GPIO_DXB1_PD, NULL);
				gpio_direction_output(GPIO_DXB1_PD, 1);
	
				/* Reset# */
				tcc_gpio_config(GPIO_DXB1_RST,  GPIO_FN(0));
				gpio_request(GPIO_DXB1_RST, NULL);
				gpio_direction_output(GPIO_DXB1_RST, 0);
				msleep (20);
				gpio_set_value(GPIO_DXB1_RST, 1);	
				break;
			default:
				break;
			}
	}
	else if (guiBoardType == BOARD_DVBS2_AVL6211_YJ)
	{
	#if defined(CONFIG_STB_BOARD_YJ8935T) ||defined(CONFIG_STB_BOARD_YJ8930T) || defined(CONFIG_STB_BOARD_YJ8933T)
		tcc_gpio_config(GPIO_DXB_YJ_SLEEP, GPIO_FN(0));
		gpio_request(GPIO_DXB_YJ_SLEEP, NULL);
		gpio_direction_output(GPIO_DXB_YJ_SLEEP, 0); // set demod active
		tcc_gpio_config(GPIO_DXB_YJ_VEN, GPIO_FN(0));
		gpio_request(GPIO_DXB_YJ_VEN, NULL);
		gpio_direction_output(GPIO_DXB_YJ_VEN, 1); // set lnb active

		tcc_gpio_config(GPIO_DXB_YJ_RESET, GPIO_FN(0));
		gpio_request(GPIO_DXB_YJ_RESET, NULL);
		gpio_direction_output(GPIO_DXB_YJ_RESET, 0); //set RST# active
		msleep(300);
		gpio_direction_output(GPIO_DXB_YJ_RESET, 1); //set RST# inactive
	#endif
	}
        else if (guiBoardType == BOARD_DVBT_MXL101SF_YJ || guiBoardType == BOARD_DVBT2_MN88472_YJ)
	{
	#if defined(CONFIG_STB_BOARD_YJ8935T) || defined(CONFIG_STB_BOARD_YJ8933T)		
		gpio_ant_pwr = TCC_GPG(18);
		gpio_check_ant_overload = TCC_GPG(19);
		int_check_ant_overload = EXTINT_GPIOG_19;
        #elif defined(CONFIG_STB_BOARD_YJ8930T)
		gpio_ant_pwr = TCC_GPG(15);
		if (system_rev == 0x7231)
		{
			gpio_check_ant_overload = TCC_GPF(6);
			int_check_ant_overload = EXTINT_GPIOF_06;
		}
		else
		{
			gpio_check_ant_overload = TCC_GPG(16);
			int_check_ant_overload = EXTINT_GPIOG_16;
		}
        #else        
        //(TCC8930ST Main STB
        gpio_ant_pwr = TCC_GPB(14);
        gpio_check_ant_overload = TCC_GPF(23);
        int_check_ant_overload = EXTINT_GPIOF_23;
        //Power On
        tcc_gpio_config(TCC_GPE(28), GPIO_FN(0));
        gpio_request(TCC_GPE(28), NULL);
        gpio_direction_output(TCC_GPE(28), 1);		//set LDO active
	#endif
		switch (guiCtrlMode)
		{
		case PWRCTRL_OFF:
			break;
		case PWRCTRL_ON:
			tcc_gpio_config(gpio_ant_pwr, GPIO_FN(0)|GPIO_PULL_DISABLE); //ANT_PWR_CTRL
			gpio_request(gpio_ant_pwr, NULL);
			gpio_direction_output(gpio_ant_pwr, 0);

			gpio_set_value(gpio_ant_pwr, 1);
			break;
		case PWRCTRL_AUTO:
			tcc_gpio_config(gpio_ant_pwr, GPIO_FN(0)|GPIO_PULL_DISABLE); //ANT_PWR_CTRL
			gpio_request(gpio_ant_pwr, NULL);
			gpio_direction_output(gpio_ant_pwr, 0);
			
			tcc_gpio_config(gpio_check_ant_overload, GPIO_FN(0)|GPIO_PULL_DISABLE); //ANT_OVERLOAD
			gpio_request(gpio_check_ant_overload, NULL);
			gpio_direction_input(gpio_check_ant_overload);

			gpio_set_value(gpio_ant_pwr, 1);
			msleep (5);
			if(gpio_get_value(gpio_check_ant_overload) == 0) {
				printk("ANT_OVERLOAD is low\n");
				gpio_set_value(gpio_ant_pwr, 0);
				return;
			}

			//interrupt setting for ANT_OVERLOAD
			guiIrq = INT_EINT3;
			tcc_gpio_config_ext_intr(guiIrq, int_check_ant_overload);
			err = request_irq(guiIrq, isr_checking_ant_overload, IRQ_TYPE_EDGE_FALLING | IRQF_DISABLED, "ANT_OVERLOAD_F", NULL);
			//err = request_irq(guiIrq, isr_checking_ant_overload, IRQ_TYPE_EDGE_RISING  | IRQF_DISABLED, "ANT_OVERLOAD_R", NULL);
			if (err) {
				printk("Unable to request ANT_OVERLOAD IRQ[%d].\n", err);	
				guiIrq = 0;
			}
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
}

static void tcc_dxb_ctrl_power_reset(int deviceIdx)
{
#if defined(CONFIG_STB_BOARD_YJ8935T) || defined(CONFIG_STB_BOARD_YJ8930T) || defined(CONFIG_STB_BOARD_YJ8933T)
	if (guiBoardType == BOARD_ISDBT_TCC353X) {
		tcc_gpio_config(GPIO_DXB_YJ_RST, GPIO_FN(0));
		gpio_direction_output(GPIO_DXB_YJ_RST, 0);
		msleep(20);
		gpio_set_value(GPIO_DXB_YJ_RST, 1);
	}
#else    
	if(machine_is_tcc8930st())
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
}

static void tcc_dxb_ctrl_get_info(ST_CTRLINFO_ARG *pstCtrlInfo)
{
#if defined(CONFIG_STB_BOARD_YJ8935T) || defined(CONFIG_STB_BOARD_YJ8930T) || defined(CONFIG_STB_BOARD_YJ8933T)
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
/* We don't use initilize gpio for DXB port. We cannot know the board type
 * at booting.
 */
#if 0    
	if(machine_is_tcc8930st())
	{
#if defined(CONFIG_STB_BOARD_YJ8935T) || defined(CONFIG_STB_BOARD_YJ8930T) || defined(CONFIG_STB_BOARD_YJ8933T)
#else
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
#endif        
	}
#endif        
}

static struct resource tcc_dxb_resource[]  = {
#if defined(CONFIG_STB_BOARD_YJ8935T) || defined(CONFIG_STB_BOARD_YJ8933T)
        [0] = {
                .name   = "gpio_ant_overload",
                .start  = TCC_GPG(19),
                .end    = TCC_GPG(19),
                .flags  = IORESOURCE_IO,
        },
#elif defined(CONFIG_STB_BOARD_YJ8930T)
        [0] = {
                .name   = "gpio_ant_overload",
                .start  = TCC_GPG(16),
                .end    = TCC_GPG(16),
                .flags  = IORESOURCE_IO,
        },
#else // STB
        [0] = {
                .name   = "gpio_ant_overload",
                .start  = TCC_GPF(23),
                .end    = TCC_GPF(23),
                .flags  = IORESOURCE_IO,
        },
#endif
};

static void tcc_dxb_ctrl_power_pure_on(int deviceIdx)
{
   	switch(deviceIdx)
    {
    case 0:
        tcc_gpio_config(TCC_GPE(28),  GPIO_FN(0));
        gpio_request(TCC_GPE(28), NULL);
        gpio_direction_output(TCC_GPE(28), 0);
        gpio_set_value(TCC_GPE(28), 0);
		msleep (20);
		gpio_set_value(TCC_GPE(28), 1);
		msleep (20);			       
        break;
    case 1:
           
        break;
    default:
        break;
	}
}

static void tcc_dxb_ctrl_power_pure_off(int deviceIdx)
{
    switch(deviceIdx)
    {
    case 0:
        tcc_gpio_config(TCC_GPE(28),  GPIO_FN(0));
        gpio_request(TCC_GPE(28), NULL);
        gpio_direction_output(TCC_GPE(28), 0);
        gpio_set_value(TCC_GPE(28), 0);
	
        break;
    case 1:        
        break;
    default:
        break;
	}
}

static void tcc_dxb_ctrl_power_reset_low(int deviceIdx)
{
   	switch(deviceIdx)
    {
    case 0:
        tcc_gpio_config(TCC_GPD(4),  GPIO_FN(0));
        gpio_request(TCC_GPD(4), NULL);
        gpio_direction_output(TCC_GPD(4), 0);
        msleep (20);
        gpio_set_value(TCC_GPD(4), 0);	
        break;
    case 1:
        tcc_gpio_config(TCC_GPD(5),  GPIO_FN(0));
        gpio_request(TCC_GPD(5), NULL);
        gpio_direction_output(TCC_GPD(5), 0);
        msleep (20);
        gpio_set_value(TCC_GPD(5), 0);	
        break;
    default:
        break;
	}
}

static void tcc_dxb_ctrl_power_reset_high(int deviceIdx)
{
    switch(deviceIdx)
    {
    case 0:
        tcc_gpio_config(TCC_GPD(4),  GPIO_FN(0));
        gpio_request(TCC_GPD(4), NULL);
        gpio_direction_output(TCC_GPD(4), 0);
        msleep (20);
        gpio_set_value(TCC_GPD(4), 1);	
        break;
    case 1:
        tcc_gpio_config(TCC_GPD(5),  GPIO_FN(0));
        gpio_request(TCC_GPD(5), NULL);
        gpio_direction_output(TCC_GPD(5), 0);
        msleep (20);
        gpio_set_value(TCC_GPD(5), 1);	
        break;
    default:
        break;
	}
}

static struct tcc_dxb_platform_data tcc_dxb_pdata = {
	.init		= tcc_dxb_init,
	.power_off	= tcc_dxb_ctrl_power_off,
	.power_on	= tcc_dxb_ctrl_power_on,
	.power_pure_off     = tcc_dxb_ctrl_power_pure_off, // LDO Control Only
	.power_pure_on     = tcc_dxb_ctrl_power_pure_on, // LDO Control Only	
	.power_reset= tcc_dxb_ctrl_power_reset,
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
	.num_resources    = ARRAY_SIZE(tcc_dxb_resource),
	.resource         = tcc_dxb_resource,
};

#if defined(CONFIG_STB_BOARD_YJ8935T) ||defined(CONFIG_STB_BOARD_YJ8930T) || defined(CONFIG_STB_BOARD_YJ8933T)
static struct resource avl6211_resource[]  = {
	[0] = DEFINE_RES_MEM_NAMED(1, 1, "i2c"),                      // frontend i2c id
	[1] = DEFINE_RES_MEM_NAMED(0xC4, 1, "tuner_addr"),            // tuner i2c address
	[2] = DEFINE_RES_MEM_NAMED(0x0C, 1, "fe_addr"),               // demodulator i2c address
	[3] = DEFINE_RES_IO_NAMED(GPIO_DXB_YJ_SLEEP, 1, "fe_power"),  // demodulator sleep on/off
	[4] = DEFINE_RES_IO_NAMED(GPIO_DXB_YJ_RESET, 1, "fe_reset"),  // demodulator reset
	[5] = DEFINE_RES_IO_NAMED(GPIO_DXB_YJ_LOCK,  1, "fe_lock"),   // demodulator lock signal
	[6] = DEFINE_RES_IO_NAMED(GPIO_DXB_YJ_FAULT, 1, "fe_fault"),  // demodulator fault signal
	[7] = DEFINE_RES_IO_NAMED(GPIO_DXB_YJ_VEN,   1, "lnb_power"), // lnb power on/off
	[8] = DEFINE_RES_IO_NAMED(GPIO_DXB_YJ_SP1V,  1, "lnb_sp1v"),  // lnb power control 0
	[9] = DEFINE_RES_IO_NAMED(GPIO_DXB_YJ_S18V,  1, "lnb_s18v"),  // lnb power control 1
};

static  struct platform_device avl6211_device = {
	.name             = "avl6211",
	.id               = -1,
	.num_resources    = ARRAY_SIZE(avl6211_resource),
	.resource         = avl6211_resource,
};
#endif

static int __init tcc8930st_init_tcc_dxb_ctrl(void)
{
	if(!machine_is_tcc8930st())
		return 0;

#if defined(CONFIG_STB_BOARD_YJ8930T)
	if (system_rev == 0x7231)
	{
		tcc_dxb_resource[0].start = TCC_GPF(6);
		tcc_dxb_resource[0].end = TCC_GPF(6);
	}
#endif

	printk("%s (built %s %s)\n", __func__, __DATE__, __TIME__);
	platform_device_register(&tcc_dxb_device);
#if defined(CONFIG_STB_BOARD_YJ8935T) ||defined(CONFIG_STB_BOARD_YJ8930T) || defined(CONFIG_STB_BOARD_YJ8933T)
	platform_device_register(&avl6211_device);
#endif
	return 0;
}
device_initcall(tcc8930st_init_tcc_dxb_ctrl);

