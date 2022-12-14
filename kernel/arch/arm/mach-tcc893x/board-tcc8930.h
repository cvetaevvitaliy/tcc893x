/*
 * linux/arch/arm/mach-tcc893x/board-tcc8930.h
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

#ifndef __ARCH_ARM_MACH_TCC893X_BOARD_TCC8930_H
#define __ARCH_ARM_MACH_TCC893X_BOARD_TCC8930_H


/**************************************************************
	GPIO Port
**************************************************************/

// Nand
#define GPIO_NAND_RDY0		TCC_GPA(16)
#if defined(CONFIG_CHIP_TCC8930)
#define GPIO_NAND_RDY1		TCC_GPD(11)
#define GPIO_NAND_WP		TCC_GPB(8)
#elif defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8933) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
#define GPIO_NAND_RDY1		0xFFFFFFFF
#define GPIO_NAND_WP		TCC_GPD(0)
#else
	#error
#endif

// LCD
#if defined(CONFIG_CHIP_TCC8930)
#define GPIO_LCD_BL			TCC_GPF(16)
#ifdef CONFIG_TOUCHSCREEN_FT5606
#define GPIO_LCD_DISPLAY	NULL
#else
#define GPIO_LCD_DISPLAY	TCC_GPB(7)
#endif
#elif defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8933) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
#define GPIO_LCD_BL			TCC_GPG(5)
#define GPIO_LCD_DISPLAY	TCC_GPB(28)
#else
	#error
#endif

/**************************************************************
	GPIO Expander
**************************************************************/

// GPEXT1
#define GPIO_LCD_ON			TCC_GPEXT1(0)
#define GPIO_CODEC_ON		TCC_GPEXT1(1)
#define GPIO_SD0_ON			TCC_GPEXT1(2)
#define GPIO_SD1_ON			TCC_GPEXT1(3)
#define GPIO_EXT_CODEC_ON	TCC_GPEXT1(5)
#define GPIO_GPS_PWREN		TCC_GPEXT1(6)
#define GPIO_BT_ON			TCC_GPEXT1(7)

#define GPIO_DXB_ON			TCC_GPEXT1(8)
#define GPIO_IPOD_ON		TCC_GPEXT1(9)
#define GPIO_CAS_ON			TCC_GPEXT1(10)
#define GPIO_FMTC_ON		TCC_GPEXT1(11)
#define GPIO_PCAM_PWR_ON	TCC_GPEXT1(12)
#define GPIO_CAM1_ON		TCC_GPEXT1(13)
#define GPIO_CAM2_ON		TCC_GPEXT1(14)
#define GPIO_ATAPI_ON		TCC_GPEXT1(15)

// GPEXT2
#define GPIO_MUTE_CTL		TCC_GPEXT2(0)
#define GPIO_LVDSIVT_ON		TCC_GPEXT2(1)
#define GPIO_LVDS_LP_CTRL	TCC_GPEXT2(2)
#define GPIO_AUTH_RST		TCC_GPEXT2(3)
#define GPIO_BT_RST			TCC_GPEXT2(4)
#define GPIO_SDWF_RST		TCC_GPEXT2(5)
#define GPIO_CAS_RST		TCC_GPEXT2(6)
#define GPIO_CAS_GP			TCC_GPEXT2(7)

#define GPIO_DXB1_PD		TCC_GPEXT2(8)
#define GPIO_DXB2_PD		TCC_GPEXT2(9)
#define GPIO_PWR_CONTROL0	TCC_GPEXT2(10)
#define GPIO_PWR_CONTROL1	TCC_GPEXT2(11)
#define GPIO_HDD_RST		TCC_GPEXT2(12)
#define GPIO_OTG0_VBUS_EN	TCC_GPEXT2(13)
#define GPIO_OTG1_VBUS_EN	TCC_GPEXT2(14)
#define GPIO_HOST_VBUS_EN	TCC_GPEXT2(15)

// GPEXT3
#define GPIO_FMT_RST		TCC_GPEXT3(0)
#define GPIO_FMT_IRQ		TCC_GPEXT3(1)
#define GPIO_BT_WAKE		TCC_GPEXT3(2)
#define GPIO_BT_HWAKE		TCC_GPEXT3(3)
#define GPIO_PHY2_ON		TCC_GPEXT3(4)
#define GPIO_COMPASS_RST	TCC_GPEXT3(5)
#define GPIO_CAM_FL_EN		TCC_GPEXT3(6)
#define GPIO_CAM2_FL_EN		TCC_GPEXT3(7)

#define GPIO_CAM2_RST		TCC_GPEXT3(8)
#define GPIO_CAM2_PWDN		TCC_GPEXT3(9)
#define GPIO_LCD_RESET		TCC_GPEXT3(10)
#define GPIO_TV_SLEEP		TCC_GPEXT3(11)
#define GPIO_ETH_ON			TCC_GPEXT3(12)
#define GPIO_ETH_RST		TCC_GPEXT3(13)
#define GPIO_SMART_AUX1		TCC_GPEXT3(14)
#define GPIO_SMART_AUX2		TCC_GPEXT3(15)

// GPEXT5	(SV60 Power Sub Board)
#define GPIO_V_5P0_EN		TCC_GPEXT5(0)
#define GPIO_OTG_EN			TCC_GPEXT5(1)
#define GPIO_HS_HOST_EN		TCC_GPEXT5(2)
#define GPIO_FS_HOST_EN		TCC_GPEXT5(3)
#define GPIO_HDMI_EN		TCC_GPEXT5(4)
#define GPIO_MIPI_EN		TCC_GPEXT5(5)
#define GPIO_SATA_EN		TCC_GPEXT5(6)
#define GPIO_LVDS_EN		TCC_GPEXT5(7)

#define GPIO_ATV_EN			TCC_GPEXT5(8)
#define GPIO_ATAPI_IPOD_EN	TCC_GPEXT5(9)
#define GPIO_USB_CHARGE_SEL	TCC_GPEXT5(10)
#define GPIO_USB_SUSPEND	TCC_GPEXT5(11)
#define GPIO_CHARGE_EN		TCC_GPEXT5(12)
#define GPIO_SD30_PWR_EN	TCC_GPEXT5(13)
#define GPIO_SD30_PWR_CTL	TCC_GPEXT5(14)
#define INT_CHARGE_FINISH	TCC_GPEXT5(15)


/**************************************************************
	Externel Interrupt
**************************************************************/

#define PMIC_IRQ			INT_EINT6

#endif
