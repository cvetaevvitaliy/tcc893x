/* linux/include/asm-arm/arch-tcc88xx/irqs.h
 *
 * Author: <linux@telechips.com>
 * Created: August 30, 2010
 * Copyright (C) 2010 Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef __ASM_ARCH_TCC88XX_IRQS_H
#define __ASM_ARCH_TCC88XX_IRQS_H

#define INT_LOCALTIMER		29

/*
 * IRQ numbers for interrupt handler
 */
#define GIC_SPI_OFFSET      32
#define	INT_TC0             (0+GIC_SPI_OFFSET)	// 0   Timer 0 interrupt enable 
#define	INT_TC1             (1+GIC_SPI_OFFSET)	// 1   Timer 32 interrupt enable 
#define	INT_SMU_I2C         (2+GIC_SPI_OFFSET)	// 2   SMU_I2C interrupt enable 
#define	INT_EINT0           (3+GIC_SPI_OFFSET)	// 3   External interrupt 0 enable 
#define	INT_EINT1           (4+GIC_SPI_OFFSET)	// 4   External interrupt 1 enable 
#define	INT_EINT2           (5+GIC_SPI_OFFSET)	// 5   External interrupt 2 enable 
#define	INT_EINT3           (6+GIC_SPI_OFFSET)	// 6   External interrupt 3 enable 
#define	INT_EINT4           (7+GIC_SPI_OFFSET)	// 7   External interrupt 4 enable 
#define	INT_EINT5           (8+GIC_SPI_OFFSET)	// 8   External interrupt 5 enable 
#define	INT_EINT6           (9+GIC_SPI_OFFSET)	// 9   External interrupt 6 enable 
#define	INT_EINT7           (10+GIC_SPI_OFFSET)	// 10  External interrupt 7 enable 
#define	INT_EINT8           (11+GIC_SPI_OFFSET)	// 11  External interrupt 8 enable 
#define	INT_EINT9           (12+GIC_SPI_OFFSET)	// 12  External interrupt 9 enable 
#define	INT_EINT10          (13+GIC_SPI_OFFSET)	// 13  External interrupt 10 enable 
#define	INT_EINT11          (14+GIC_SPI_OFFSET)	// 14  External interrupt 11 enable 
#define	INT_JPEG1           (15+GIC_SPI_OFFSET)	// 15  JPEG Encoder interrupt enable 
#define	INT_MALIPPMMU0      (16+GIC_SPI_OFFSET)	// 16  3D Pixel Processor MMU interrupt enable 
#define	INT_MALIGPMMU       (17+GIC_SPI_OFFSET)	// 17  3D Geometry Processor MMU interrupt enable 
#define	INT_DENALI          (18+GIC_SPI_OFFSET)	// 18  Denali Controller interrupt enable
#define	INT_L2CACHE         (19+GIC_SPI_OFFSET)	// 19  L2 cache interrupt enable
#define	INT_LCDC            (20+GIC_SPI_OFFSET)	// 20  LCD controller interrupt enable 
#define	INT_JPEG0           (21+GIC_SPI_OFFSET)	// 21  JPEG interrupt enable (Video codec)
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
#define	INT_ARMPMU1        (22+GIC_SPI_OFFSET)	// 22  ARM PMU 1 interrupt enable
#else
#endif
#define	INT_VCDC            (23+GIC_SPI_OFFSET)	// 23  Video CODEC interrupt enable 
#define	INT_MALIPP0         (24+GIC_SPI_OFFSET)	// 24  3D Pixel Processor interrupt enable 
#define	INT_MALIGP          (25+GIC_SPI_OFFSET)	// 25  3D Geometry Processor interrupt enable 
#define	INT_MALIPMU         (26+GIC_SPI_OFFSET)	// 26  3D PMU interrupt enable 
#define	INT_OVERLAY         (27+GIC_SPI_OFFSET)	// 27  Overlay Mixer interrupt enable 
#define	INT_TSADC           (28+GIC_SPI_OFFSET)	// 28  TSADC interrupt enable 
#define	INT_GDMA            (29+GIC_SPI_OFFSET)	// 29  General DMA controller interrupt enable  (IOBUS)
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
#define	INT_MIPIDSI         (31+GIC_SPI_OFFSET)	// 31  MIPI DSI interrupt enable
#else
#define	INT_HSIODMAX        (30+GIC_SPI_OFFSET)	// 30  HSIO Bus DMAX interrupt enable
#define	INT_CM3MB           (31+GIC_SPI_OFFSET)	// 31  CM3 MAILBOX interrupt enable
#endif
#define	INT_SDMMC3          (32+GIC_SPI_OFFSET)	// 32 0  SDMMC 3 interrupt enable 
#define	INT_SDMMC2          (33+GIC_SPI_OFFSET)	// 33 1  SDMMC 2 interrupt enable 
#define	INT_HDMI            (34+GIC_SPI_OFFSET)	// 34 2  HDMI interrupt enable 
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
#else
#define	INT_MALIPP1         (35+GIC_SPI_OFFSET) // 35 3  3D Pixel Processor 1 interrupt enable 
#endif
#define	INT_GPSB            (36+GIC_SPI_OFFSET)	// 36 4  GPSB Interrupt Enable 
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
#else
#define	INT_IDE             (37+GIC_SPI_OFFSET) // 37 5  IDE Interrupt enable
#endif
#define	INT_I2C             (38+GIC_SPI_OFFSET)	// 38 6  I2C interrupt enable
#define	INT_MPEFEC          (39+GIC_SPI_OFFSET)	// 39 7  MPEFEC interrupt enable 
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
#define	INT_TZASC           (40+GIC_SPI_OFFSET) // 40 8  TZASC (IMEM/DMEM/PERI)
#else
#define	INT_MALIPPMMU1      (40+GIC_SPI_OFFSET) // 40 8  3D Pixel Processor MMU 1 interrupt enable
#endif
#define	INT_NFC             (41+GIC_SPI_OFFSET)	// 41 9  Nand flash controller interrupt enable 
#define	INT_REMOCON         (42+GIC_SPI_OFFSET)	// 42 10 Remote Control interrupt enable 
#define	INT_RTC             (43+GIC_SPI_OFFSET)	// 43 11 RTC interrupt enable 
#define	INT_SDMMC0          (44+GIC_SPI_OFFSET)	// 44 12 SD/MMC 0 interrupt enable 
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
#else
#define	INT_SDMMC1          (45+GIC_SPI_OFFSET)	// 45 13 SD/MMC 1 interrupt enable 
#endif
#define	INT_GDMAHS          (46+GIC_SPI_OFFSET)	// 46 14 General DMA controller interrupt enable  (HSIOBUS)
#define	INT_UART            (47+GIC_SPI_OFFSET)	// 47 15 UART interrupt enable 
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
#define	INT_UOTG           (48+GIC_SPI_OFFSET)	// 48 16 USB OTG interrupt enable 
#else
#define	INT_USB30          (48+GIC_SPI_OFFSET)	// 48 16 USB 3.0 interrupt enable 
#endif
#define	INT_USB20H          (49+GIC_SPI_OFFSET)	// 49 17 USB 2.0 HOST interrupt enable 
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
#define	INT_MALIPP1         (50+GIC_SPI_OFFSET) // 50 18 3D Pixel Processor 1 interrupt enable
#define	INT_MALIPPMMU1      (51+GIC_SPI_OFFSET)	// 51 19 3D Pixel Processor 1 MMU interrupt enable
#else
#define	INT_RFX             (50+GIC_SPI_OFFSET) // 50 18  RFX Interrupt enable
#define	INT_GMAC            (51+GIC_SPI_OFFSET)	// 51 19 GMAC interrupt enable 
#endif
#define	INT_CIPHER          (52+GIC_SPI_OFFSET)	// 52 20 Cipher interrupt enable 
#define	INT_TSIF            (53+GIC_SPI_OFFSET)	// 53 21 TS interface interrupt enable 
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
#else
#define	INT_CM3TSDEMUX      (54+GIC_SPI_OFFSET) // 54 22  CM3 TSDEMUX interrupt enable
#define	INT_CAN             (55+GIC_SPI_OFFSET) // 55 23  CAN Interrupt enable
#endif
#define	INT_USBVBON         (56+GIC_SPI_OFFSET)	// 56 24 USB OTG VBOFF interrupt enable 
//#define	INT_USBVBOFF        (57+GIC_SPI_OFFSET)	// 57 25 USB OTG VBON interrupt enable 
#define	INT_OTGID        	(57+GIC_SPI_OFFSET)	// 57 25 USB OTG VBON interrupt enable 
#define	INT_ADMA1           (58+GIC_SPI_OFFSET)	// 58 26 AUDIO 1 DMA interrupt enable 
#define	INT_AUDIO1          (59+GIC_SPI_OFFSET)	// 59 27 AUDIO 1 interrupt enable 
#define	INT_ADMA0           (60+GIC_SPI_OFFSET)	// 60 28 AUDIO 0 DMA interrupt enable 
#define	INT_AUDIO0          (61+GIC_SPI_OFFSET)	// 61 29 AUDIO 0 interrupt enable 
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
#define INT_ARMPMU0	        (63+GIC_SPI_OFFSET)
#define INT_NUM             INT_ARMPMU0
#else
#define	INT_CPUBUSDMAX      (62+GIC_SPI_OFFSET)	// 62 30 CPU Bus DMAX interrupt enable
#define	INT_USB30RTUNE      (63+GIC_SPI_OFFSET)	// 63 31 ARM PMU interrupt enable 
#define INT_ARMPMU0	    (64+GIC_SPI_OFFSET)
#define INT_ARMPMU1	    (65+GIC_SPI_OFFSET)
#define INT_NUM             INT_ARMPMU1
#endif

/*
 * IRQ_UT numbers for UART[0:7]
 */
#define INT_UART_BASE	(INT_NUM + 1)
#define INT_UART0	(0 + INT_UART_BASE)
#define INT_UART1	(1 + INT_UART_BASE)
#define INT_UART2	(2 + INT_UART_BASE)
#define INT_UART3	(3 + INT_UART_BASE)
#define INT_UART4	(4 + INT_UART_BASE)
#define INT_UART5	(5 + INT_UART_BASE)
#define INT_UART6	(6 + INT_UART_BASE)
#define INT_UART7	(7 + INT_UART_BASE)
#define INT_UART_NUM	INT_UART7

/*
 * GPSB-IRQ numbers for GPSB0 & GPSB0
 */
#define INT_GPSB_BASE	(INT_UART_NUM + 1)
#define INT_GPSB0_DMA	(0 + INT_GPSB_BASE)
#define INT_GPSB1_DMA	(1 + INT_GPSB_BASE)
#define INT_GPSB2_DMA	(2 + INT_GPSB_BASE)

#define INT_GPSB0_CORE	(3 + INT_GPSB_BASE)
#define INT_GPSB1_CORE	(4 + INT_GPSB_BASE)
#define INT_GPSB2_CORE	(5 + INT_GPSB_BASE)
#define INT_GPSB3_CORE	(6 + INT_GPSB_BASE)
#define INT_GPSB4_CORE	(7 + INT_GPSB_BASE)
#define INT_GPSB5_CORE	(8 + INT_GPSB_BASE)
#define INT_GPSB_NUM	INT_GPSB5_CORE

/*
 * DMA-IRQ numbers
 */
#define INT_DMA_BASE	(INT_GPSB_NUM + 1)
#define INT_DMA0_CH0	(0 + INT_DMA_BASE)
#define INT_DMA0_CH1	(1 + INT_DMA_BASE)
#define INT_DMA0_CH2	(2 + INT_DMA_BASE)
#define INT_DMA1_CH0	(3 + INT_DMA_BASE)
#define INT_DMA1_CH1	(4 + INT_DMA_BASE)
#define INT_DMA1_CH2	(5 + INT_DMA_BASE)
#define INT_DMA2_CH0	(6 + INT_DMA_BASE)
#define INT_DMA2_CH1	(7 + INT_DMA_BASE)
#define INT_DMA2_CH2	(8 + INT_DMA_BASE)
#define INT_DMA_NUM		INT_DMA2_CH2

/*
 * TC0-IRQ numbers for Timer/Counter 0~5
 */
#define INT_TC_BASE		(INT_DMA_NUM + 1)
#define INT_TC_TI0		(0 + INT_TC_BASE)
#define INT_TC_TI1		(1 + INT_TC_BASE)
#define INT_TC_TI2		(2 + INT_TC_BASE)
#define INT_TC_TI3		(3 + INT_TC_BASE)
#define INT_TC_TI4		(4 + INT_TC_BASE)
#define INT_TC_TI5		(5 + INT_TC_BASE)
#define INT_TC_NUM		INT_TC_TI5

/*
 * DMA-IRQ numbers
 */
#define INT_VIOC_BASE          	(INT_TC_NUM + 1)
#define INT_VIOC_DEV0           (0 + INT_VIOC_BASE)
#define INT_VIOC_DEV1           (1 + INT_VIOC_BASE)
#define INT_VIOC_DEV2           (2 + INT_VIOC_BASE)
#define INT_VIOC_TIMER          (3 + INT_VIOC_BASE)
#define INT_VIOC_RD0            (4 + INT_VIOC_BASE)
#define INT_VIOC_RD1            (5 + INT_VIOC_BASE)
#define INT_VIOC_RD2            (6 + INT_VIOC_BASE)
#define INT_VIOC_RD3            (7 + INT_VIOC_BASE)
#define INT_VIOC_RD4            (8 + INT_VIOC_BASE)
#define INT_VIOC_RD5            (9 + INT_VIOC_BASE)
#define INT_VIOC_RD6            (10 + INT_VIOC_BASE)
#define INT_VIOC_RD7            (11 + INT_VIOC_BASE)
#define INT_VIOC_RD8            (12 + INT_VIOC_BASE)
#define INT_VIOC_RD9            (13 + INT_VIOC_BASE)
#define INT_VIOC_RD10           (14 + INT_VIOC_BASE)
#define INT_VIOC_RD11           (15 + INT_VIOC_BASE)
#define INT_VIOC_RD12           (16 + INT_VIOC_BASE)
#define INT_VIOC_RD13           (17 + INT_VIOC_BASE)
#define INT_VIOC_RD14           (18 + INT_VIOC_BASE)
#define INT_VIOC_RD15           (19 + INT_VIOC_BASE)
#define INT_VIOC_RD16           (20 + INT_VIOC_BASE)
#define INT_VIOC_RD17           (21 + INT_VIOC_BASE)
#define INT_VIOC_RESERVED_22    (22 + INT_VIOC_BASE)
#define INT_VIOC_RESERVED_23    (23 + INT_VIOC_BASE)
#define INT_VIOC_MMU            (24 + INT_VIOC_BASE)
#define INT_VIOC_RESERVED_25    (25 + INT_VIOC_BASE)
#define INT_VIOC_RESERVED_26    (26 + INT_VIOC_BASE)
#define INT_VIOC_RESERVED_27    (27 + INT_VIOC_BASE)
#define INT_VIOC_FIFO0          (28 + INT_VIOC_BASE)
#define INT_VIOC_FIFO1          (29 + INT_VIOC_BASE)
#define INT_VIOC_RESERVED_30    (30 + INT_VIOC_BASE)
#define INT_VIOC_RESERVED_31    (31 + INT_VIOC_BASE)

#define INT_VIOC_WD0            (32 + INT_VIOC_BASE)
#define INT_VIOC_WD1            (33 + INT_VIOC_BASE)
#define INT_VIOC_WD2            (34 + INT_VIOC_BASE)
#define INT_VIOC_WD3            (35 + INT_VIOC_BASE)
#define INT_VIOC_WD4            (36 + INT_VIOC_BASE)
#define INT_VIOC_WD5            (37 + INT_VIOC_BASE)
#define INT_VIOC_WD6            (38 + INT_VIOC_BASE)
#define INT_VIOC_WD7            (39 + INT_VIOC_BASE)
#define INT_VIOC_WD8            (40 + INT_VIOC_BASE)
#define INT_VIOC_RESERVED_41    (41 + INT_VIOC_BASE)
#define INT_VIOC_RESERVED_42    (42 + INT_VIOC_BASE)
#define INT_VIOC_RESERVED_43    (43 + INT_VIOC_BASE)
#define INT_VIOC_RESERVED_44    (44 + INT_VIOC_BASE)
#define INT_VIOC_RESERVED_45    (45 + INT_VIOC_BASE)
#define INT_VIOC_RESERVED_46    (46 + INT_VIOC_BASE)
#define INT_VIOC_RESERVED_47    (47 + INT_VIOC_BASE)
#define INT_VIOC_WMIX0          (48 + INT_VIOC_BASE)
#define INT_VIOC_WMIX1          (49 + INT_VIOC_BASE)
#define INT_VIOC_WMIX2          (50 + INT_VIOC_BASE)
#define INT_VIOC_WMIX3          (51 + INT_VIOC_BASE)
#define INT_VIOC_WMIX4          (52 + INT_VIOC_BASE)
#define INT_VIOC_WMIX5          (53 + INT_VIOC_BASE)
#define INT_VIOC_RESERVED54     (54 + INT_VIOC_BASE)
#define INT_VIOC_RESERVED55     (55 + INT_VIOC_BASE)
#define INT_VIOC_RESERVED56     (56 + INT_VIOC_BASE)
#define INT_VIOC_VIQE           (57 + INT_VIOC_BASE)
#define INT_VIOC_SC0            (58 + INT_VIOC_BASE)
#define INT_VIOC_SC1            (59 + INT_VIOC_BASE)
#define INT_VIOC_SC2            (60 + INT_VIOC_BASE)
#define INT_VIOC_SC3            (61 + INT_VIOC_BASE)
#define INT_VIOC_NUM		    INT_VIOC_SC3

/*
 * TSIF-IRQ numbers
 */
#define INT_TSIF_BASE		(INT_VIOC_NUM + 1)
#define INT_TSIF_DMA0		(0 + INT_TSIF_BASE)
#define INT_TSIF_DMA1		(1 + INT_TSIF_BASE)
#define INT_TSIF_DMA2		(2 + INT_TSIF_BASE)
#define INT_TSIF_NUM		INT_TSIF_DMA2

/*
 * NR_IRQ:
 */
#if 1
#define NR_IRQS         (INT_TSIF_NUM + 1)
#else
#define NR_IRQS ((INT_NUM + 1)					\
		 + (INT_UART_NUM - INT_UT_BASE + 1)		\
		 + (INT_GPSB_NUM - INT_GPSB_BASE + 1)	\
		 + (INT_DMA_NUM - INT_DMA_BASE + 1)		\
		 + (INT_TC_NUM - INT_TC_BASE + 1)		\
                 )
#endif

#ifndef __ASSEMBLY__
extern void tcc_init_irq(void);
#endif

#endif  /* ASM_ARCH_TCC88XX_IRQS_H */
