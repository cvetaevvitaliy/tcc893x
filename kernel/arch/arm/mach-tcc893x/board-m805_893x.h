/*
 * linux/arch/arm/mach-tcc893x/board-m805_893x.h
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

#ifndef __ARCH_ARM_MACH_TCC892X_BOARD_M805_893X_H
#define __ARCH_ARM_MACH_TCC892X_BOARD_M805_893X_H


/**************************************************************
	GPIO Port
**************************************************************/

// PWR KEY
#define GPIO_PWR_KEY		0xFFFFFFFF

// Nand
#define GPIO_NAND_RDY0		TCC_GPA(16)
#define GPIO_NAND_RDY1		0xFFFFFFFF
#if defined(CONFIG_M805S_8925_0XX)
#define GPIO_NAND_WP		TCC_GPD(0)
#else
#define GPIO_NAND_WP		TCC_GPD(8)
#endif

// LCD
#define GPIO_LCD_ON 		TCC_GPE(24)
#define GPIO_LCD_BL 		TCC_GPC(0)
#define GPIO_LCD_RESET		TCC_GPB(29)
#define GPIO_LCD_DISPLAY	0xFFFFFFFF
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
#define GPIO_LCD_IODEN		TCC_GPE(15)
#else
#define GPIO_LCD_IODEN		TCC_GPE(19)
#endif

// DxB
#define GPIO_DXB0_SFRM		TCC_GPD(9)  //DXB_TSVALID
#define GPIO_DXB0_SCLK		TCC_GPD(8)  //DXB_TSCLK
#define INT_DXB0_IRQ		TCC_GPC(4)
#define GPIO_DXB0_SDI		TCC_GPD(7)  //DXB_TSDATA
#define GPIO_DXB0_SDO		TCC_GPD(10) //DXB_TSSYNC
#if defined (CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8933)
#define GPIO_DXB1_SFRM		TCC_GPD(23)
#define GPIO_DXB1_SCLK		TCC_GPD(22)
#define GPIO_DXB1_SDI		TCC_GPD(24)
#define GPIO_DXB1_SDO		TCC_GPD(25)
#elif defined(CONFIG_CHIP_TCC8935S)
#define GPIO_DXB1_SFRM		TCC_GPD(12)
#define GPIO_DXB1_SCLK		TCC_GPD(11)
#define GPIO_DXB1_SDI		TCC_GPD(13)
#define GPIO_DXB1_SDO		TCC_GPD(14)
#elif defined (CONFIG_CHIP_TCC8937S)
#define GPIO_DXB1_SFRM		TCC_GPG(1)
#define GPIO_DXB1_SCLK		TCC_GPG(0)
#define GPIO_DXB1_SDI		TCC_GPG(2)
#define GPIO_DXB1_SDO		TCC_GPG(3)
#endif

#define GPIO_DXB1_RST		
#define INT_DXB1_IRQ		

#define GPIO_DXB_PWDN		TCC_GPC(5)
#define GPIO_DXB_PWREN		TCC_GPC(7)
#define GPIO_DXB0_RST		TCC_GPC(6)
#define GPIO_RFSW_CTL0		TCC_GPC(8)

#if defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8933)
#define	GPIO_DXB2_PWDN	TCC_GPE(15)
#elif defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8937S)
#define	GPIO_DXB2_PWDN	TCC_GPE(19)
#endif
#define	GPIO_DXB2_RST		TCC_GPC(8)

#define GPIO_RFSW_CTL3		0xFFFFFFFF
#define GPIO_RFSW_CTL2		0xFFFFFFFF
#define GPIO_RFSW_CTL1		0xFFFFFFFF



#define GPIO_V_5P0_EN		TCC_GPE(26)

/**************************************************************
	Externel Interrupt
**************************************************************/

#define PMIC_IRQ			INT_EINT6

#endif
