/****************************************************************************
FileName    : kernel/arch/arm//mach-tcc893x/board-tcc8930st-mmc.c
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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/mmc/host.h>
#include <linux/gpio.h>

#include <mach/mmc.h>

#include <mach/gpio.h>
#include <mach/irqs.h>
//#include <mach/bsp.h>
#include <mach/bsp.h>
#include <asm/mach-types.h>
#include <asm/system.h>

#include "devices.h"
#include "board-tcc8930st.h"

#if defined(CONFIG_MMC_TCC_SDHC)
extern void tcc_init_sdhc_devices(void);

#if defined(CONFIG_WIFI_PWR_CTL)
extern int wifi_stat;
#endif

struct tcc_mmc_platform_data tcc8930_mmc_platform_data[];

typedef enum {
	TCC_MMC_BUS_WIDTH_4 = 4,
	TCC_MMC_BUS_WIDTH_8 = 8
} tcc_mmc_bus_width_type;

#define TCC_MMC_PORT_NULL	0x0FFFFFFF

// PIC 0
#define HwINT0_EI4					Hw7						// R/W, External Interrupt 4 enable
#define HwINT0_EI5					Hw8						// R/W, External Interrupt 5 enable

// PIC 1
#define HwINT1_SD0					Hw12					// R/W, SD/MMC 0 interrupt enable
#define HwINT1_SD1					Hw13					// R/W, SD/MMC 1 interrupt enable
#define HwINT1_SD2	 				Hw1 					// R/W, SD/MMC 2 Interrupt enable
#define HwINT1_SD3		 			Hw0 					// R/W, SD/MMC 3 Interrupt enable

#if defined(CONFIG_STB_BOARD_DONGLE_TCC893X)
#define TCC_MMC_SDIO_WIFI_USED

#if defined(CONFIG_HDMI_DONGLE_WIFI_REALTEK_TCC893x) || defined(CONFIG_BROADCOM_WIFI)
	#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
		#define WIFI_SDMMC_PORT		3
		#define WIFI_PERI_SDMMC		PERI_SDMMC3
		#define WIFI_RB_SDMMC		RB_SDMMC3CONTROLLER
		#define WIFI_HwINT1_SD		HwINT1_SD3
	#else
		#define WIFI_SDMMC_PORT		2
		#define WIFI_PERI_SDMMC		PERI_SDMMC2
		#define WIFI_RB_SDMMC		RB_SDMMC2CONTROLLER
		#define WIFI_HwINT1_SD		HwINT1_SD2
	#endif
#else
#define WIFI_SDMMC_PORT		4
#define WIFI_PERI_SDMMC		PERI_SDMMC0
#define WIFI_RB_SDMMC		RB_SDMMC0CONTROLLER
#define WIFI_HwINT1_SD		HwINT1_SD0
#endif

#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
#define EMMC_SDMMC_PORT		4
#define EMMC_PERI_SDMMC		PERI_SDMMC0
#define EMMC_RB_SDMMC		RB_SDMMC0CONTROLLER
#define EMMC_HwINT1_SD		HwINT1_SD0
#else
#define EMMC_SDMMC_PORT		3
#define EMMC_PERI_SDMMC		PERI_SDMMC3
#define EMMC_RB_SDMMC		RB_SDMMC3CONTROLLER
#define EMMC_HwINT1_SD		HwINT1_SD3
#endif

typedef enum {
	#if defined(CONFIG_MMC_TCC_SUPPORT_EMMC)
	TCC_MMC_TYPE_EMMC,
	#endif
	TCC_MMC_TYPE_WIFI,
	TCC_MMC_TYPE_MAX
} tcc_mmc_type;

static struct mmc_port_config mmc_ports[] = {
	#if defined(CONFIG_MMC_TCC_SUPPORT_EMMC)
		#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
			[TCC_MMC_TYPE_EMMC] = {
				.data0	= TCC_GPD(18),
				.data1	= TCC_GPD(17),
				.data2	= TCC_GPD(16),
				.data3	= TCC_GPD(15),
			#if defined(CONFIG_BROADCOM_WIFI)
				.data4  = TCC_GPD(14),
				.data5  = TCC_GPD(13),
				.data6  = TCC_GPD(12),
				.data7  = TCC_GPD(11),
			#else
				.data4	= TCC_MMC_PORT_NULL,
				.data5	= TCC_MMC_PORT_NULL,
				.data6	= TCC_MMC_PORT_NULL,
				.data7	= TCC_MMC_PORT_NULL,
			#endif
				.cmd	= TCC_GPD(19),
				.clk		= TCC_GPD(20),
				.wp		= TCC_MMC_PORT_NULL,
				.func	= GPIO_FN(2),
				.width	= TCC_MMC_BUS_WIDTH_4,

				.cd		= TCC_MMC_PORT_NULL,
				.pwr	= TCC_MMC_PORT_NULL,
			},
		#else
			[TCC_MMC_TYPE_EMMC] = {
				.data0	= TCC_GPD(29),
				.data1	= TCC_GPD(28),
				.data2	= TCC_GPD(27),
				.data3	= TCC_GPD(26),
			#if defined(CONFIG_BROADCOM_WIFI)
				.data4  = TCC_GPD(25),
				.data5  = TCC_GPD(24),
				.data6  = TCC_GPD(23),
				.data7  = TCC_GPD(22),
			#else
				.data4	= TCC_MMC_PORT_NULL,
				.data5	= TCC_MMC_PORT_NULL,
				.data6	= TCC_MMC_PORT_NULL,
				.data7	= TCC_MMC_PORT_NULL,
			#endif
				.cmd	= TCC_GPD(31),
				.clk		= TCC_GPD(30),
				.wp		= TCC_MMC_PORT_NULL,
				.func	= GPIO_FN(2),
				.width	= TCC_MMC_BUS_WIDTH_4,

				.cd		= TCC_MMC_PORT_NULL,
				.pwr	= TCC_MMC_PORT_NULL,
			},
		#endif
	#endif
	
	#if defined(CONFIG_HDMI_DONGLE_WIFI_REALTEK_TCC893x) || defined(CONFIG_BROADCOM_WIFI)
		#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
			[TCC_MMC_TYPE_WIFI] = {
				.data0	= TCC_GPB(20),
				.data1	= TCC_GPB(21),
				.data2	= TCC_GPB(22),
				.data3	= TCC_GPB(23),
				.data4	= TCC_MMC_PORT_NULL,
				.data5	= TCC_MMC_PORT_NULL,
				.data6	= TCC_MMC_PORT_NULL,
				.data7	= TCC_MMC_PORT_NULL,
				.cmd	= TCC_GPB(19),
				.clk		= TCC_GPB(28),
				.wp		= TCC_MMC_PORT_NULL,
				.func	= GPIO_FN(3),
				.width	= TCC_MMC_BUS_WIDTH_4,

				.cd	= TCC_MMC_PORT_NULL,
			#if defined(CONFIG_HDMI_DONGLE_WIFI_REALTEK_TCC893x)
				.pwr	= TCC_MMC_PORT_NULL,
			#else
				.pwr	= TCC_GPD(21),
			#endif
			},
		#else
			[TCC_MMC_TYPE_WIFI] = {
				.data0	= TCC_GPB(20),
				.data1	= TCC_GPB(21),
				.data2	= TCC_GPB(22),
				.data3	= TCC_GPB(23),
				.data4	= TCC_MMC_PORT_NULL,
				.data5	= TCC_MMC_PORT_NULL,
				.data6	= TCC_MMC_PORT_NULL,
				.data7	= TCC_MMC_PORT_NULL,
				.cmd	= TCC_GPB(19),
				.clk		= TCC_GPB(28),
				.wp		= TCC_MMC_PORT_NULL,
				.func	= GPIO_FN(15),
				.width	= TCC_MMC_BUS_WIDTH_4,

				.cd	= TCC_MMC_PORT_NULL,
			#if defined(CONFIG_HDMI_DONGLE_WIFI_REALTEK_TCC893x)
				.pwr	= TCC_MMC_PORT_NULL,
			#else
				.pwr	= TCC_GPD(21),
			#endif
			},
		#endif
	#else
		[TCC_MMC_TYPE_WIFI] = {
			.data0	= TCC_GPD(18),
			.data1	= TCC_GPD(17),
			.data2	= TCC_GPD(16),
			.data3	= TCC_GPD(15),
			.data4	= TCC_MMC_PORT_NULL,
			.data5	= TCC_MMC_PORT_NULL,
			.data6	= TCC_MMC_PORT_NULL,
			.data7	= TCC_MMC_PORT_NULL,
			.cmd	= TCC_GPD(19),
			.clk	= TCC_GPD(20),
			.wp		= TCC_MMC_PORT_NULL,
			.func	= GPIO_FN(2),
			.width	= TCC_MMC_BUS_WIDTH_4,

			.cd	= TCC_MMC_PORT_NULL,
			.pwr	= TCC_GPD(21),
		},
	#endif
};

#elif defined(CONFIG_STB_BOARD_UPC_TCC893X) || defined(CONFIG_STB_BOARD_YJ8933T)
#define TCC_MMC_SD_CARD_USED

typedef enum {
	#if defined(CONFIG_MMC_TCC_SUPPORT_EMMC)
	TCC_MMC_TYPE_EMMC,
	#endif
	#if defined(TCC_MMC_SD_CARD_USED)
	TCC_MMC_TYPE_SD,
	#endif
	TCC_MMC_TYPE_MAX
} tcc_mmc_type;

#if defined(CONFIG_STB_BOARD_YJ8933T)
#define TFCD_EXT_INT		EXTINT_GPIOF_02
#define TFCD_GPIO_PORT		TCC_GPF(2)
#else
#define TFCD_EXT_INT		EXTINT_GPIOE_28
#define TFCD_GPIO_PORT		TCC_GPE(28)
#endif
#define TFCD_SDMMC_PORT		2

#define TFCD_PERI_SDMMC		PERI_SDMMC2
#define TFCD_RB_SDMMC		RB_SDMMC2CONTROLLER
#define TFCD_HwINT1_SD		HwINT1_SD2

#define EMMC_SDMMC_PORT		3
#define EMMC_PERI_SDMMC		PERI_SDMMC3
#define EMMC_RB_SDMMC		RB_SDMMC3CONTROLLER
#define EMMC_HwINT1_SD		HwINT1_SD3

static struct mmc_port_config mmc_ports[] = {
	#if defined(CONFIG_MMC_TCC_SUPPORT_EMMC)
		[TCC_MMC_TYPE_EMMC] = {
			.data0	= TCC_GPD(29),
			.data1	= TCC_GPD(28),
			.data2	= TCC_GPD(27),
			.data3	= TCC_GPD(26),
			.data4	= TCC_MMC_PORT_NULL,
			.data5	= TCC_MMC_PORT_NULL,
			.data6	= TCC_MMC_PORT_NULL,
			.data7	= TCC_MMC_PORT_NULL,
			.cmd	= TCC_GPD(31),
			.clk	= TCC_GPD(30),
			.wp		= TCC_MMC_PORT_NULL,
			.func	= GPIO_FN(2),
			.width	= TCC_MMC_BUS_WIDTH_4,
			.strength = GPIO_CD(1),

			.cd		= TCC_MMC_PORT_NULL,
			.pwr	= TCC_MMC_PORT_NULL,	// do not used!
		},
	#endif

	#if defined(TCC_MMC_SD_CARD_USED)
		[TCC_MMC_TYPE_SD] = {
			.data0	= TCC_GPB(20),
			.data1	= TCC_GPB(21),
			.data2	= TCC_GPB(22),
			.data3	= TCC_GPB(23),
			.data4	= TCC_MMC_PORT_NULL,
			.data5	= TCC_MMC_PORT_NULL,
			.data6	= TCC_MMC_PORT_NULL,
			.data7	= TCC_MMC_PORT_NULL,
			.cmd	= TCC_GPB(19),
			.clk	= TCC_GPB(28),
			.wp		= TCC_MMC_PORT_NULL,
			.func	= GPIO_FN(15),
			.width	= TCC_MMC_BUS_WIDTH_4,
			.strength = GPIO_CD(3),

			.cd	= TFCD_GPIO_PORT,
			.pwr	= TCC_MMC_PORT_NULL,
		},
	#endif
};

#elif defined(CONFIG_STB_BOARD_YJ8935T)
#define TCC_MMC_SDIO_WIFI_USED

typedef enum {
	#if defined(CONFIG_MMC_TCC_SUPPORT_EMMC)
	TCC_MMC_TYPE_EMMC,
	#endif
	#if defined(TCC_MMC_SD_CARD_USED)
	TCC_MMC_TYPE_SD,
	#endif
	#if defined(TCC_MMC_SDIO_WIFI_USED)
	TCC_MMC_TYPE_WIFI,
	#endif
	TCC_MMC_TYPE_MAX
} tcc_mmc_type;

#define TFCD_EXT_INT		EXTINT_GPIOF_02
#define TFCD_GPIO_PORT		TCC_GPF(2)

#define TFCD_SDMMC_PORT		4
#define TFCD_PERI_SDMMC		PERI_SDMMC0
#define TFCD_RB_SDMMC		RB_SDMMC0CONTROLLER
#define TFCD_HwINT1_SD		HwINT1_SD0

#define EMMC_SDMMC_PORT		3
#define EMMC_PERI_SDMMC		PERI_SDMMC3
#define EMMC_RB_SDMMC		RB_SDMMC3CONTROLLER
#define EMMC_HwINT1_SD		HwINT1_SD3

#define WIFI_SDMMC_PORT		2
#define WIFI_PERI_SDMMC		PERI_SDMMC2
#define WIFI_RB_SDMMC		RB_SDMMC2CONTROLLER
#define WIFI_HwINT1_SD		HwINT1_SD2

static struct mmc_port_config mmc_ports[] = {
	#if defined(CONFIG_MMC_TCC_SUPPORT_EMMC)
		[TCC_MMC_TYPE_EMMC] = {
			.data0	= TCC_GPD(29),
			.data1	= TCC_GPD(28),
			.data2	= TCC_GPD(27),
			.data3	= TCC_GPD(26),
			.data4	= TCC_MMC_PORT_NULL,
			.data5	= TCC_MMC_PORT_NULL,
			.data6	= TCC_MMC_PORT_NULL,
			.data7	= TCC_MMC_PORT_NULL,
			.cmd	= TCC_GPD(31),
			.clk	= TCC_GPD(30),
			.wp		= TCC_MMC_PORT_NULL,
			.func	= GPIO_FN(2),
			.width	= TCC_MMC_BUS_WIDTH_4,
			.strength = GPIO_CD(1),

			.cd		= TCC_MMC_PORT_NULL,
			.pwr	= TCC_MMC_PORT_NULL,	// do not used!
		},
	#endif

	#if defined(TCC_MMC_SD_CARD_USED)
		[TCC_MMC_TYPE_SD] = {
			.data0	= TCC_GPD(18),
			.data1	= TCC_GPD(17),
			.data2	= TCC_GPD(16),
			.data3	= TCC_GPD(15),
			.data4	= TCC_MMC_PORT_NULL,
			.data5	= TCC_MMC_PORT_NULL,
			.data6	= TCC_MMC_PORT_NULL,
			.data7	= TCC_MMC_PORT_NULL,
			.cmd	= TCC_GPD(19),
			.clk	= TCC_GPD(20),
			.wp		= TCC_MMC_PORT_NULL,
			.func	= GPIO_FN(2),
			.width	= TCC_MMC_BUS_WIDTH_4,

			.cd		= TFCD_GPIO_PORT,
			.pwr	= TCC_MMC_PORT_NULL,
		},
	#endif

	#if defined(TCC_MMC_SDIO_WIFI_USED)
		[TCC_MMC_TYPE_WIFI] = {
			.data0	= TCC_GPB(20),
			.data1	= TCC_GPB(21),
			.data2	= TCC_GPB(22),
			.data3	= TCC_GPB(23),
			.data4	= TCC_MMC_PORT_NULL,
			.data5	= TCC_MMC_PORT_NULL,
			.data6	= TCC_MMC_PORT_NULL,
			.data7	= TCC_MMC_PORT_NULL,
			.cmd	= TCC_GPB(19),
			.clk	= TCC_GPB(28),
			.wp		= TCC_MMC_PORT_NULL,
			.func	= GPIO_FN(15),
			.width	= TCC_MMC_BUS_WIDTH_4,

			.cd		= TCC_MMC_PORT_NULL,
			.pwr	= TCC_MMC_PORT_NULL,
		},
	#endif
};

#else	// TCC893x STB
#define TCC_MMC_SD_CARD_USED
#if defined(CONFIG_STB_BOARD_YJ8930T)
#define TCC_MMC_SDIO_WIFI_USED
#endif

typedef enum {
	#if defined(CONFIG_MMC_TCC_SUPPORT_EMMC)
	TCC_MMC_TYPE_EMMC,
	#endif
	#if defined(TCC_MMC_SD_CARD_USED)
	TCC_MMC_TYPE_SD,
	#endif
	#if defined(TCC_MMC_SDIO_WIFI_USED)
	TCC_MMC_TYPE_WIFI,
	#endif
	TCC_MMC_TYPE_MAX
} tcc_mmc_type;

#define EMMC_SDMMC_PORT		3
#define EMMC_PERI_SDMMC		PERI_SDMMC3
#define EMMC_RB_SDMMC		RB_SDMMC3CONTROLLER
#define EMMC_HwINT1_SD		HwINT1_SD3

#define TFCD_EXT_INT		EXTINT_GPIOG_11
#define TFCD_GPIO_PORT		TCC_GPG(11)
#define TFCD_SDMMC_PORT		0
#define TFCD_PERI_SDMMC		PERI_SDMMC0
#define TFCD_RB_SDMMC		RB_SDMMC0CONTROLLER
#define TFCD_HwINT1_SD		HwINT1_SD0

#define WIFI_SDMMC_PORT		1
#define WIFI_PERI_SDMMC		PERI_SDMMC1
#define WIFI_RB_SDMMC		RB_SDMMC1CONTROLLER
#define WIFI_HwINT1_SD		HwINT1_SD1

static struct mmc_port_config mmc_ports[] = {
	#if defined(CONFIG_MMC_TCC_SUPPORT_EMMC)
		[TCC_MMC_TYPE_EMMC] = {
			.data0	= TCC_GPD(29),
			.data1	= TCC_GPD(28),
			.data2	= TCC_GPD(27),
			.data3	= TCC_GPD(26),
			.data4	= TCC_MMC_PORT_NULL,
			.data5	= TCC_MMC_PORT_NULL,
			.data6	= TCC_MMC_PORT_NULL,
			.data7	= TCC_MMC_PORT_NULL,
			.cmd	= TCC_GPD(31),
			.clk	= TCC_GPD(30),
			.wp		= TCC_MMC_PORT_NULL,
			.func	= GPIO_FN(2),
			.width	= TCC_MMC_BUS_WIDTH_4,
			.strength = GPIO_CD(1),

			.cd		= TCC_MMC_PORT_NULL,
			.pwr	= TCC_MMC_PORT_NULL,	// do not used!
		},
	#endif

	#if defined(TCC_MMC_SD_CARD_USED)
		[TCC_MMC_TYPE_SD] = {
			.data0	= TCC_GPSD(2),
			.data1	= TCC_GPSD(3),
			.data2	= TCC_GPSD(4),
			.data3	= TCC_GPSD(5),
			.data4	= TCC_MMC_PORT_NULL,
			.data5	= TCC_MMC_PORT_NULL,
			.data6	= TCC_MMC_PORT_NULL,
			.data7	= TCC_MMC_PORT_NULL,
			.cmd	= TCC_GPSD(1),
			.clk	= TCC_GPSD(0),
			.wp		= TCC_GPF(29),
			.func	= GPIO_FN(1),
			.width	= TCC_MMC_BUS_WIDTH_4,
			.strength = GPIO_CD(3),

			.cd		= TFCD_GPIO_PORT,
			.pwr	= TCC_MMC_PORT_NULL,
			.vctl	= TCC_GPD(23), 
		},
	#endif

	#if defined(TCC_MMC_SDIO_WIFI_USED)
        [TCC_MMC_TYPE_WIFI] = {
			.data0	= TCC_GPF(15),
			.data1	= TCC_GPF(16),
			.data2	= TCC_GPF(17),
			.data3	= TCC_GPF(18),
			.data4	= TCC_MMC_PORT_NULL,
			.data5	= TCC_MMC_PORT_NULL,
			.data6	= TCC_MMC_PORT_NULL,
			.data7	= TCC_MMC_PORT_NULL,
			.cmd	= TCC_GPF(14),
			.clk	= TCC_GPF(13),
			.wp		= TCC_MMC_PORT_NULL,
			.func	= GPIO_FN(2),
			.width	= TCC_MMC_BUS_WIDTH_4,

			.cd		= TCC_MMC_PORT_NULL,
			.pwr	= TCC_MMC_PORT_NULL,
		},
	#endif
};
#endif

#if defined(TCC_MMC_SD_CARD_USED)
#define CONFIG_TCC_SD_PORT_RESTORE	//for SD Power-off in the STB
#endif

static int tccUsedSDportNum = TCC_MMC_TYPE_MAX;

int tcc8930_mmc_init(struct device *dev, int id)
{
	BUG_ON(id >= TCC_MMC_TYPE_MAX);

#if defined(CONFIG_STB_BOARD_YJ8930T)
	if (system_rev == 0x7231)
	{
		mmc_ports[TCC_MMC_TYPE_SD].data0 = TCC_GPF(15);
		mmc_ports[TCC_MMC_TYPE_SD].data1 = TCC_GPF(16);
		mmc_ports[TCC_MMC_TYPE_SD].data2 = TCC_GPF(17);
		mmc_ports[TCC_MMC_TYPE_SD].data3 = TCC_GPF(18);
		mmc_ports[TCC_MMC_TYPE_SD].data4 = TCC_MMC_PORT_NULL;
		mmc_ports[TCC_MMC_TYPE_SD].data5 = TCC_MMC_PORT_NULL;
		mmc_ports[TCC_MMC_TYPE_SD].data6 = TCC_MMC_PORT_NULL;
		mmc_ports[TCC_MMC_TYPE_SD].data7 = TCC_MMC_PORT_NULL;
		mmc_ports[TCC_MMC_TYPE_SD].cmd = TCC_GPF(14);
		mmc_ports[TCC_MMC_TYPE_SD].clk = TCC_GPF(13);
		mmc_ports[TCC_MMC_TYPE_SD].wp = TCC_GPF(29);
		mmc_ports[TCC_MMC_TYPE_SD].func = GPIO_FN(2);
		mmc_ports[TCC_MMC_TYPE_SD].strength = GPIO_CD(0);
		mmc_ports[TCC_MMC_TYPE_SD].cd = TFCD_GPIO_PORT;
		mmc_ports[TCC_MMC_TYPE_SD].pwr = TCC_MMC_PORT_NULL;
		mmc_ports[TCC_MMC_TYPE_SD].vctl = TCC_MMC_PORT_NULL;

		mmc_ports[TCC_MMC_TYPE_WIFI].data0 = TCC_GPSD(2);
		mmc_ports[TCC_MMC_TYPE_WIFI].data1 = TCC_GPSD(3);
		mmc_ports[TCC_MMC_TYPE_WIFI].data2 = TCC_GPSD(4);
		mmc_ports[TCC_MMC_TYPE_WIFI].data3 = TCC_GPSD(5);
		mmc_ports[TCC_MMC_TYPE_WIFI].data4 = TCC_MMC_PORT_NULL;
		mmc_ports[TCC_MMC_TYPE_WIFI].data5 = TCC_MMC_PORT_NULL;
		mmc_ports[TCC_MMC_TYPE_WIFI].data6 = TCC_MMC_PORT_NULL;
		mmc_ports[TCC_MMC_TYPE_WIFI].data7 = TCC_MMC_PORT_NULL;
		mmc_ports[TCC_MMC_TYPE_WIFI].cmd = TCC_GPSD(1);
		mmc_ports[TCC_MMC_TYPE_WIFI].clk = TCC_GPSD(0);
		mmc_ports[TCC_MMC_TYPE_WIFI].wp = TCC_MMC_PORT_NULL;
		mmc_ports[TCC_MMC_TYPE_WIFI].func = GPIO_FN(1);
		mmc_ports[TCC_MMC_TYPE_WIFI].strength = GPIO_CD(3);
		mmc_ports[TCC_MMC_TYPE_WIFI].cd = TCC_MMC_PORT_NULL;
		mmc_ports[TCC_MMC_TYPE_WIFI].pwr = TCC_MMC_PORT_NULL;
		mmc_ports[TCC_MMC_TYPE_WIFI].vctl = TCC_MMC_PORT_NULL;
	}
#endif

	if(mmc_ports[id].pwr != TCC_MMC_PORT_NULL)
	{
		gpio_request(mmc_ports[id].pwr, "sd_power");

		#if defined(TCC_MMC_SDIO_WIFI_USED)
		if(id == TCC_MMC_TYPE_WIFI)
		{
			//gpio_request(mmc_ports[id].pwr, "wifi_pre_power");
			gpio_direction_output(mmc_ports[id].pwr, 0);
			msleep(100);
			gpio_direction_output(mmc_ports[id].pwr, 1);

		}
		#endif
	}

	#if defined(CONFIG_TCC_SD_PORT_RESTORE)
	if(id == TCC_MMC_TYPE_SD)
	{
		gpio_request(mmc_ports[id].data0, "sd_d0");
		gpio_request(mmc_ports[id].data1, "sd_d1");
		gpio_request(mmc_ports[id].data2, "sd_d2");
		gpio_request(mmc_ports[id].data3, "sd_d3");

		gpio_request(mmc_ports[id].cmd, "sd_cmd");
		gpio_request(mmc_ports[id].clk, "sd_clk");
	}
	#endif

	tcc_gpio_config(mmc_ports[id].data0, mmc_ports[id].func | GPIO_CD(1));
	tcc_gpio_config(mmc_ports[id].data1, mmc_ports[id].func | GPIO_CD(1));
	tcc_gpio_config(mmc_ports[id].data2, mmc_ports[id].func | GPIO_CD(1));
	tcc_gpio_config(mmc_ports[id].data3, mmc_ports[id].func | GPIO_CD(1));

	if(mmc_ports[id].width == TCC_MMC_BUS_WIDTH_8)
	{
		tcc_gpio_config(mmc_ports[id].data4, mmc_ports[id].func | GPIO_CD(1));
		tcc_gpio_config(mmc_ports[id].data5, mmc_ports[id].func | GPIO_CD(1));
		tcc_gpio_config(mmc_ports[id].data6, mmc_ports[id].func | GPIO_CD(1));
		tcc_gpio_config(mmc_ports[id].data7, mmc_ports[id].func | GPIO_CD(1));
	}

	tcc_gpio_config(mmc_ports[id].cmd, mmc_ports[id].func | GPIO_CD(1));
	tcc_gpio_config(mmc_ports[id].clk, mmc_ports[id].func | GPIO_CD(3));

	#if defined(CONFIG_BROADCOM_WIFI)
	if(id == TCC_MMC_TYPE_WIFI)
	{
		tcc_gpio_config(mmc_ports[id].data0, GPIO_PULLUP);
		tcc_gpio_config(mmc_ports[id].data1, GPIO_PULLUP);
		tcc_gpio_config(mmc_ports[id].data2, GPIO_PULLUP);
		tcc_gpio_config(mmc_ports[id].data3, GPIO_PULLUP);
		tcc_gpio_config(mmc_ports[id].cmd, GPIO_PULLUP);
	}
	#endif

	if(mmc_ports[id].cd != TCC_MMC_PORT_NULL)
	{
		tcc_gpio_config(mmc_ports[id].cd, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(mmc_ports[id].cd, "sd_cd");

		gpio_direction_input(mmc_ports[id].cd);
	}

	if(mmc_ports[id].wp != TCC_MMC_PORT_NULL)
	{
		tcc_gpio_config(mmc_ports[id].wp, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(mmc_ports[id].wp, "sd_wp");

		gpio_direction_input(mmc_ports[id].wp);
	}
	
	return 0;
}

extern int wifi_stat;

int tcc8930_mmc_card_detect(struct device *dev, int id)
{
	BUG_ON(id >= TCC_MMC_TYPE_MAX);

#if defined(CONFIG_BROADCOM_WIFI)
	if(id == TCC_MMC_TYPE_WIFI && !wifi_stat) {
		return 0;
	}else if(id == TCC_MMC_TYPE_WIFI && wifi_stat){
		return 1;
	}
#endif

	if(mmc_ports[id].cd == TCC_MMC_PORT_NULL)
		return 1;

	return gpio_get_value(mmc_ports[id].cd) ? 0 : 1;	
}

int tcc8930_mmc_suspend(struct device *dev, int id)
{
	#if defined(TCC_MMC_SDIO_WIFI_USED)
	if(id == TCC_MMC_TYPE_WIFI) {
		if(mmc_ports[id].pwr != TCC_MMC_PORT_NULL)
			gpio_direction_output(mmc_ports[id].pwr, 0);
	}
	#endif

	#if defined(CONFIG_TCC_SD_PORT_RESTORE)
	if (id == TCC_MMC_TYPE_SD)
	{
		/* GPIO mode */
		tcc_gpio_config(mmc_ports[id].data0, GPIO_FN(0));
		tcc_gpio_config(mmc_ports[id].data1, GPIO_FN(0));
		tcc_gpio_config(mmc_ports[id].data2, GPIO_FN(0));
		tcc_gpio_config(mmc_ports[id].data3, GPIO_FN(0));

		tcc_gpio_config(mmc_ports[id].cmd, GPIO_FN(0));
		tcc_gpio_config(mmc_ports[id].clk, GPIO_FN(0));

		/* output mode - 1:high, 0:low */
		gpio_direction_output(mmc_ports[id].data0, 0);
		gpio_direction_output(mmc_ports[id].data1, 0);
		gpio_direction_output(mmc_ports[id].data2, 0);
		gpio_direction_output(mmc_ports[id].data3, 0);

		gpio_direction_output(mmc_ports[id].cmd, 0);
		gpio_direction_output(mmc_ports[id].clk, 0);
	}
	#endif

	return 0;
}

int tcc8930_mmc_resume(struct device *dev, int id)
{
	#if defined(TCC_MMC_SDIO_WIFI_USED)
	if (id == TCC_MMC_TYPE_WIFI) {
		if(mmc_ports[id].pwr != TCC_MMC_PORT_NULL)
			gpio_direction_output(mmc_ports[id].pwr, 1);
	}
	#endif

 	return 0;
}

int tcc8930_mmc_set_power(struct device *dev, int id, int on)
{
	if (on) {
		/* power */
		if(mmc_ports[id].pwr != TCC_MMC_PORT_NULL)
		{
			gpio_direction_output(mmc_ports[id].pwr, 1);

			mdelay(1);
		}
	} else {

		//mdelay(10);
	}

	return 0;
}

int tcc8930_mmc_set_bus_width(struct device *dev, int id, int width)
{
	return 0;
}

int tcc8930_mmc_cd_int_config(struct device *dev, int id, unsigned int cd_irq)
{
	if(tcc8930_mmc_platform_data[id].cd_int_num > 0)
	{
		tcc_gpio_config_ext_intr(tcc8930_mmc_platform_data[id].cd_irq_num, tcc8930_mmc_platform_data[id].cd_ext_irq);
	}
	else
	{
		return -1;
	}	

	return 0;
}

//Start : Wakeup for SD Insert->Remove in suspend. - 120109, hjbae
int tcc893x_sd_card_detect(void)
{
	#if defined(TCC_MMC_SD_CARD_USED)
	return gpio_get_value(mmc_ports[TCC_MMC_TYPE_SD].cd) ? 0 : 1;
	#else
	return 0;
	#endif
}
//End

int tcc8930_mmc_get_ro(struct device *dev, int id)
{
	if (mmc_ports[id].wp != TCC_MMC_PORT_NULL) {
		if (gpio_get_value(mmc_ports[id].wp)) {
			printk("[tcc_mmc] New card is inserted with write protect mode.\n");
			return 1;
		} else {
			printk("[tcc_mmc] New card is inserted without write protect mode.\n");
			return 0;
		}
	}
	printk("[tcc_mmc] Write protect mode is NOT supported in this hardware.\n");
	return 0;
}

#if !defined(CONFIG_MMC_TCC_DISABLE_SD30) && defined(CONFIG_CHIP_TCC8930)
int tcc8930_mmc_switch_voltage(struct device *dev, int id, int on)
{
	if (on) {
		printk("[mmc] %s, Down SD30 voltage from 3.3V to 1.8V...\n", __func__);
		gpio_request(mmc_ports[id].vctl, "SD30_VCTL");
		//printk("[mmc] gpio_get_value(mmc_ports[id].vctl) : %d\n", gpio_get_value(mmc_ports[id].vctl));
		tcc_gpio_config(mmc_ports[id].vctl, GPIO_FN(0)
				|
				/*GPIO_PULLUP*/
				/*GPIO_PULLDOWN*/
				GPIO_PULL_DISABLE
				| GPIO_LOW | GPIO_OUTPUT
				);
		//printk("[mmc] gpio_get_value(mmc_ports[id].vctl) : %d\n", gpio_get_value(mmc_ports[id].vctl));
		mdelay(100);
	} else if (!on && !gpio_get_value(mmc_ports[id].vctl)) {
		printk("[mmc] %s, Up SD30 voltage from 1.8V to 3.3V...\n", __func__);
		gpio_request(mmc_ports[id].vctl, "SD30_VCTL");
		//printk("[mmc] gpio_get_value(mmc_ports[id].vctl) : %d\n", gpio_get_value(mmc_ports[id].vctl));
		tcc_gpio_config(mmc_ports[id].vctl, GPIO_FN(0)
				|
				/*GPIO_PULLUP*/
				/*GPIO_PULLDOWN*/
				GPIO_PULL_DISABLE
				| GPIO_HIGH | GPIO_OUTPUT
				);
		//printk("[mmc] gpio_get_value(mmc_ports[id].vctl) : %d\n", gpio_get_value(mmc_ports[id].vctl));
		mdelay(100);
	}

	return 0;
}
#endif

struct tcc_mmc_platform_data tcc8930_mmc_platform_data[] = {
	#if defined(CONFIG_MMC_TCC_SUPPORT_EMMC)		// [0]:eMMC,   [1]:SD,   [2]:WiFi
	[TCC_MMC_TYPE_EMMC] = {
		.slot	= EMMC_SDMMC_PORT,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
#if defined(CONFIG_BROADCOM_WIFI)
			| MMC_CAP_8_BIT_DATA
#endif
#if defined(CONFIG_MMC_TCC_HIGHSPEED_MODE) || defined(CONFIG_MMC_TCC_DDR_MODE)
			| MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED
#endif
#ifdef CONFIG_MMC_TCC_DDR_MODE
			| MMC_CAP_UHS_DDR50 | MMC_CAP_1_8V_DDR
#endif
			,	
		.f_min	= 100000,
		.f_max	= MMC_MAX_CLOCK_SPEED,
		.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34,
		.init	= tcc8930_mmc_init,
		.card_detect = tcc8930_mmc_card_detect,
		.cd_int_config = tcc8930_mmc_cd_int_config,
		.suspend = tcc8930_mmc_suspend,
		.resume = tcc8930_mmc_resume,
		.set_power = tcc8930_mmc_set_power,
		.set_bus_width = tcc8930_mmc_set_bus_width,
		.get_ro = tcc8930_mmc_get_ro,

		.cd_int_num = -1,
		.cd_ext_irq = -1,
		.peri_name = EMMC_PERI_SDMMC,
		.io_name = EMMC_RB_SDMMC,
		.pic = EMMC_HwINT1_SD,
	},
	#endif

	#if defined(TCC_MMC_SD_CARD_USED)
	[TCC_MMC_TYPE_SD] = {
		.slot	= TFCD_SDMMC_PORT,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			| MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED
#if defined(CONFIG_MMC_TCC_SD30_SDR12)
			/* for SD3.0 - 1.8V, 25MHz, 12.5MB/sec(Maximum) */
			| MMC_CAP_UHS_SDR12   			
			| MMC_CAP_MAX_CURRENT_200
#elif defined(CONFIG_MMC_TCC_SD30_SDR25)
			/* for SD3.0 - 1.8V, 50MHz, 25MB/sec(Maximum) */
			| MMC_CAP_UHS_SDR25   
			| MMC_CAP_MAX_CURRENT_400
			| MMC_CAP_DRIVER_TYPE_A
#elif defined(CONFIG_MMC_TCC_SD30_SDR50)
			/* for SD3.0 - 1.8V, 100MHz, 50MB/sec(Maximum) */
			| MMC_CAP_UHS_SDR50   
			| MMC_CAP_MAX_CURRENT_600
			| MMC_CAP_DRIVER_TYPE_C
#elif defined(CONFIG_MMC_TCC_SD30_DDR50)
			/* for SD3.0 - 1.8V, 50MHz, 50MB/sec(Maximum) */
			| MMC_CAP_UHS_DDR50
			| MMC_CAP_1_8V_DDR
			//| MMC_CAP_MAX_CURRENT_400
#endif
#if 0
			| MMC_CAP_UHS_SDR104  /* for SD3.0 - 1.8V, 208MHz, 104MB/sec */
			| MMC_CAP_MAX_CURRENT_800
			| MMC_CAP_DRIVER_TYPE_D
#endif
			,
		.f_min	= 100000,
#if defined(CONFIG_MMC_TCC_SD30_SDR12)
		.f_max = 24000000,
#elif defined(CONFIG_MMC_TCC_SD30_SDR25)
		.f_max = 48000000,
#elif defined(CONFIG_MMC_TCC_SD30_SDR50)
		.f_max = 80000000,
#elif defined(CONFIG_MMC_TCC_SD30_DDR50)
		.f_max = 30000000,
#else
		.f_max = 45000000,
#endif
		.ocr_mask = 
#if !defined(CONFIG_MMC_TCC_DISABLE_SD30) && defined(CONFIG_CHIP_TCC8930)
			MMC_VDD_165_195 |
#endif
			MMC_VDD_32_33 | MMC_VDD_33_34,
		.init	= tcc8930_mmc_init,
		.card_detect = tcc8930_mmc_card_detect,
		.cd_int_config = tcc8930_mmc_cd_int_config,
		.suspend = tcc8930_mmc_suspend,
		.resume	= tcc8930_mmc_resume,
		.set_power = tcc8930_mmc_set_power,
		.set_bus_width = tcc8930_mmc_set_bus_width,
#if !defined(CONFIG_MMC_TCC_DISABLE_SD30) && defined(CONFIG_CHIP_TCC8930)
		.switch_voltage = tcc8930_mmc_switch_voltage,
#endif
		.get_ro = tcc8930_mmc_get_ro,
#if defined(CONFIG_BROADCOM_WIFI) && defined(CONFIG_STB_BOARD_YJ8930T)
		.cd_int_num = -1,
		.cd_irq_num = -1,
		.cd_ext_irq = -1,
#else
		.cd_int_num = HwINT0_EI5,
		.cd_irq_num = INT_EINT5,
		.cd_ext_irq = TFCD_EXT_INT,
#endif
		.peri_name = TFCD_PERI_SDMMC,
		.io_name = TFCD_RB_SDMMC,
		.pic = TFCD_HwINT1_SD,
	},
	#endif

	#if defined(TCC_MMC_SDIO_WIFI_USED)
	[TCC_MMC_TYPE_WIFI] = {
		.slot	= WIFI_SDMMC_PORT,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			| MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED,
		.f_min	= 100000,
		.f_max	= 48000000,	/* support highspeed mode */
//		.f_max	= 24000000,	// Only Atheros WiFi(AR6102)
		.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34,
		.init	= tcc8930_mmc_init,
		.card_detect = tcc8930_mmc_card_detect,
		.cd_int_config = tcc8930_mmc_cd_int_config,
		.suspend = tcc8930_mmc_suspend,
		.resume	= tcc8930_mmc_resume,
		.set_power = tcc8930_mmc_set_power,
		.set_bus_width = tcc8930_mmc_set_bus_width,
		.get_ro = tcc8930_mmc_get_ro,

		.cd_int_num = -1,
		.cd_irq_num = -1,
		.cd_ext_irq = -1,
		.peri_name = WIFI_PERI_SDMMC,
		.io_name = WIFI_RB_SDMMC,
		.pic = WIFI_HwINT1_SD,
	},
	#endif

	#if 0	//for Example
	[x] = {
		.slot	= 2,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			/*| MMC_CAP_8_BIT_DATA */
			| MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED,
		.f_min	= 100000,
		.f_max	= 48000000,	/* support highspeed mode */
		.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34,
		.init	= tcc8930_mmc_init,
		.card_detect = tcc8930_mmc_card_detect,
		.suspend = tcc8930_mmc_suspend,
		.resume	= tcc8930_mmc_resume,
		.set_power = tcc8930_mmc_set_power,
		.set_bus_width = tcc8930_mmc_set_bus_width,

		.cd_int_num = -1,
		.peri_name = PERI_SDMMC2,
		.io_name = RB_SDMMC2CONTROLLER,
		.pic = HwINT1_SD2,
	},
	#endif
};

static int __init tcc8930_init_mmc(void)
{
	if (!machine_is_tcc8930st())
		return 0;

	printk("%s\n",__func__);

#if defined(CONFIG_STB_BOARD_YJ8930T)
	if (system_rev == 0x7231)
	{
		tcc8930_mmc_platform_data[TCC_MMC_TYPE_SD].slot = 1;
		tcc8930_mmc_platform_data[TCC_MMC_TYPE_SD].peri_name = PERI_SDMMC1;
		tcc8930_mmc_platform_data[TCC_MMC_TYPE_SD].io_name = RB_SDMMC1CONTROLLER;
		tcc8930_mmc_platform_data[TCC_MMC_TYPE_SD].pic = HwINT1_SD1;
		tcc8930_mmc_platform_data[TCC_MMC_TYPE_SD].f_max = 40000000;
		tcc8930_mmc_platform_data[TCC_MMC_TYPE_WIFI].slot = 0;
        tcc8930_mmc_platform_data[TCC_MMC_TYPE_WIFI].peri_name = PERI_SDMMC0;
        tcc8930_mmc_platform_data[TCC_MMC_TYPE_WIFI].io_name = RB_SDMMC0CONTROLLER;
        tcc8930_mmc_platform_data[TCC_MMC_TYPE_WIFI].pic = HwINT1_SD0;
	}
#endif

	tcc_init_sdhc_devices();

	printk("%s(%d)\n",__func__, tccUsedSDportNum);

#if defined(CONFIG_MMC_TCC_SDHC0)
	if (tccUsedSDportNum > 0)
	{
		tcc_sdhc0_device.dev.platform_data = &tcc8930_mmc_platform_data[0];
		platform_device_register(&tcc_sdhc0_device);
	}
#endif
#if defined(CONFIG_MMC_TCC_SDHC1)
	if (tccUsedSDportNum > 1)
	{
		tcc_sdhc1_device.dev.platform_data = &tcc8930_mmc_platform_data[1];
		platform_device_register(&tcc_sdhc1_device);
	}
#endif
#if defined(CONFIG_MMC_TCC_SDHC2)
	if (tccUsedSDportNum > 2)
	{
		tcc_sdhc2_device.dev.platform_data = &tcc8930_mmc_platform_data[2];
		platform_device_register(&tcc_sdhc2_device);
	}
#endif
#if defined(CONFIG_MMC_TCC_SDHC3)
	if (tccUsedSDportNum > 3)
	{
		tcc_sdhc3_device.dev.platform_data = &tcc8930_mmc_platform_data[3];
		platform_device_register(&tcc_sdhc3_device);
	}
#endif

	return 0;
}
device_initcall(tcc8930_init_mmc);
#endif

