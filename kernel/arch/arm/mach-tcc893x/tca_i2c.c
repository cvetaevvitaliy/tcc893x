/****************************************************************************
 *   FileName    : tca_i2c.c
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <asm/mach-types.h>

#include <mach/bsp.h>
#include <mach/tca_i2c.h>
#include <mach/gpio.h>
#include <asm/system.h>


/*****************************************************************************
* Function Name : tca_i2c_setgpio(int ch)
* Description: I2C port configuration
* input parameter:
*       int ch;   	// I2C master channel
******************************************************************************/
void tca_i2c_setgpio(int ch)
{
	//PI2CPORTCFG i2c_portcfg = (PI2CPORTCFG)tcc_p2v(HwI2C_PORTCFG_BASE);

	switch (ch)
	{
		case 0:
			#if defined(CONFIG_MACH_TCC8930ST)
				#if defined(CONFIG_STB_BOARD_DONGLE_TCC893X)
					//I2C[16] - GPIOE[14][15]
					//i2c_portcfg->PCFG0.bREG.MASTER0 = 16;
					BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x000000FF, 16);
					tcc_gpio_config(TCC_GPE(17), GPIO_FN15|GPIO_OUTPUT|GPIO_LOW);
					tcc_gpio_config(TCC_GPE(19), GPIO_FN15|GPIO_OUTPUT|GPIO_LOW);
				#elif defined(CONFIG_STB_BOARD_YJ8935T) || defined(CONFIG_STB_BOARD_YJ8933T)
				#elif defined(CONFIG_STB_BOARD_UPC_TCC893X)
				#else
					//I2C[26] - TCC_GPG[18][19]
					//i2c_portcfg->PCFG0.bREG.MASTER0 = 26;
					BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x000000FF, 26);
					tcc_gpio_config(TCC_GPG(18), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
					tcc_gpio_config(TCC_GPG(19), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
				#endif
			#else
				if(system_rev == 0x1000){
					//I2C[8] - GPIOB[9][10]
					BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x000000FF, 8);
					tcc_gpio_config(TCC_GPB(9), GPIO_FN11|GPIO_OUTPUT|GPIO_LOW);
					tcc_gpio_config(TCC_GPB(10), GPIO_FN11|GPIO_OUTPUT|GPIO_LOW);
				}else if(system_rev == 0x2000 || system_rev == 0x3000){
					//I2C[18] - GPIOF[13][14]
					BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x000000FF, 18);
					tcc_gpio_config(TCC_GPF(13), GPIO_FN10|GPIO_OUTPUT|GPIO_LOW);
					tcc_gpio_config(TCC_GPF(14), GPIO_FN10|GPIO_OUTPUT|GPIO_LOW);
				}else if(machine_is_m805_893x()){
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
					BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x000000FF, 12);
					tcc_gpio_config(TCC_GPC(2), GPIO_FN7|GPIO_OUTPUT|GPIO_LOW);
					tcc_gpio_config(TCC_GPC(3), GPIO_FN7|GPIO_OUTPUT|GPIO_LOW);
#else
					BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x000000FF, 13);
					tcc_gpio_config(TCC_GPC(20), GPIO_FN7|GPIO_OUTPUT|GPIO_LOW);
					tcc_gpio_config(TCC_GPC(21), GPIO_FN7|GPIO_OUTPUT|GPIO_LOW);
#endif
				}
			#endif
			break;
		case 1:
			#if defined(CONFIG_MACH_TCC8930ST)
				#if defined(CONFIG_STB_BOARD_YJ8935T) || defined(CONFIG_STB_BOARD_YJ8933T)
					//I2C[16] - GPIOE[14][15]
					//i2c_portcfg->PCFG0.bREG.MASTER1 = 16;
					BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x0000FF00, 16<<8);
					tcc_gpio_config(TCC_GPE(17), GPIO_FN15|GPIO_OUTPUT|GPIO_LOW);
					tcc_gpio_config(TCC_GPE(19), GPIO_FN15|GPIO_OUTPUT|GPIO_LOW);
				#else
					//I2C[22] - GPIOG[2][3]
					//i2c_portcfg->PCFG0.bREG.MASTER1 = 22;
				if (system_rev == 0x7231) {
					BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x0000FF00, 20<<8);
					tcc_gpio_config(TCC_GPF(19), GPIO_FN10|GPIO_OUTPUT|GPIO_LOW);
					tcc_gpio_config(TCC_GPF(20), GPIO_FN10|GPIO_OUTPUT|GPIO_LOW);
				} else {
					BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x0000FF00, 22<<8);
					tcc_gpio_config(TCC_GPG(2), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
					tcc_gpio_config(TCC_GPG(3), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
				}
				#endif
			
			#else
				if(system_rev == 0x1000){
					//I2C[21] - GPIOF[27][28]
					BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x0000FF00, 21<<8);
					tcc_gpio_config(TCC_GPF(27), GPIO_FN10|GPIO_OUTPUT|GPIO_LOW);
					tcc_gpio_config(TCC_GPF(28), GPIO_FN10|GPIO_OUTPUT|GPIO_LOW);
				}else if(system_rev == 0x2000 || system_rev == 0x3000){
					//I2C[28] - GPIO_ADC[2][3]
					BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x0000FF00, 28<<8);
					tcc_gpio_config(TCC_GPADC(2), GPIO_FN3|GPIO_OUTPUT|GPIO_LOW);
					tcc_gpio_config(TCC_GPADC(3), GPIO_FN3|GPIO_OUTPUT|GPIO_LOW);
				}else if(machine_is_m805_893x()){
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
					BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x0000FF00, 25<<8);
					tcc_gpio_config(TCC_GPG(12), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
					tcc_gpio_config(TCC_GPG(13), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
#else
					BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x0000FF00, 22<<8);
					tcc_gpio_config(TCC_GPG(12), GPIO_FN14|GPIO_OUTPUT|GPIO_LOW);
					tcc_gpio_config(TCC_GPG(13), GPIO_FN14|GPIO_OUTPUT|GPIO_LOW);
#endif
				}
			#endif
			break;
		case 2:
			#if defined(CONFIG_MACH_TCC8930ST)
				#if defined(CONFIG_iTV_PLATFORM)
					//I2C[22] - GPIOG[2][3]
					//i2c_portcfg->PCFG0.bREG.MASTER2 = 22;
					BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x00FF0000, 22<<16);
					tcc_gpio_config(TCC_GPG(2), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
					tcc_gpio_config(TCC_GPG(3), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
				#endif
				#if defined(CONFIG_VIDEO_TCCXX_HDMI_IN)
					BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x00FF0000, 26<<16);
					tcc_gpio_config(TCC_GPG(18), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
					tcc_gpio_config(TCC_GPG(19), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
				#endif
			#else
				if(system_rev == 0x1000){
					//I2C[15] - GPIOC[30][31]
					BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x00FF0000, 15<<16);
					tcc_gpio_config(TCC_GPC(30), GPIO_FN7|GPIO_OUTPUT|GPIO_LOW);
					tcc_gpio_config(TCC_GPC(31), GPIO_FN7|GPIO_OUTPUT|GPIO_LOW);
				}else if(machine_is_m805_893x()){
					BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x00FF0000, 18<<16);
					tcc_gpio_config(TCC_GPF(13), GPIO_FN10|GPIO_OUTPUT|GPIO_LOW);
					tcc_gpio_config(TCC_GPF(14), GPIO_FN10|GPIO_OUTPUT|GPIO_LOW);
				}
			#endif
			break;
		case 3:
			//I2C[27] - GPIO_HDMI[2][3]
			BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0xFF000000, 27<<24);
			#if defined(CONFIG_MACH_TCC8930ST)
			tcc_gpio_config(TCC_GPHDMI(2), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);
			tcc_gpio_config(TCC_GPHDMI(3), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);
			#endif
			break;
		default:
			break;
	}
}

