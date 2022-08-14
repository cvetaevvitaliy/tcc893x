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
#include <mach/bsp.h>
#include <asm/mach-types.h>

#include "devices.h"
#include "board-tcc8930.h"

#if defined(CONFIG_MMC_TCC_SDHC)
extern void tcc_init_sdhc_devices(void);

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

#define TCC_MMC_SDIO_WIFI_USED

#ifdef  CONFIG_CHIP_TCC8930
/*
 * 0x1000 : TCC8930_D3_08X4_SV_0.1 - DDR3 1024(32Bit)
 */
typedef enum {
#if defined(CONFIG_MMC_TCC_SUPPORT_EMMC)
	TCC_MMC_TYPE_EMMC,
#endif
	TCC_MMC_TYPE_SD,
	TCC_MMC_TYPE_WIFI,
	TCC_MMC_TYPE_MAX
} tcc_mmc_type;

static struct mmc_port_config mmc_ports[] = {
#if defined(CONFIG_MMC_TCC_SUPPORT_EMMC)
	[TCC_MMC_TYPE_EMMC] = {
		.data0	= TCC_GPD(29),
		.data1	= TCC_GPD(28),
		.data2	= TCC_GPD(27),
		.data3	= TCC_GPD(26),
		.data4	= TCC_GPD(25),
		.data5	= TCC_GPD(24),
		.data6	= TCC_GPD(23),
		.data7	= TCC_GPD(22),
		.cmd	= TCC_GPD(30),
		.clk	= TCC_GPD(31),
		.wp = TCC_GPD(10),
		.func	= GPIO_FN(2),
		.width	= TCC_MMC_BUS_WIDTH_8,
		.strength = GPIO_CD(1),

		.cd	= TCC_MMC_PORT_NULL,	//TCC_GPB(14),
		.pwr	= TCC_MMC_PORT_NULL,	// do not used!
	},
#endif
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
		.wp = TCC_GPB(6),
		.func	= GPIO_FN(1),
		.width	= TCC_MMC_BUS_WIDTH_4,
		.strength = GPIO_CD(3),

		.cd	= TCC_GPB(12),
		.pwr	= TCC_MMC_PORT_NULL,
		.vctl = TCC_GPB(30), 
	},
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
		.wp = TCC_GPB(17),
		.func	= GPIO_FN(15),
		.width	= TCC_MMC_BUS_WIDTH_4,
		.strength = GPIO_CD(1),

		.cd	= TCC_GPB(13),
		.pwr	= GPIO_SD1_ON,
	},
};
#endif

#if defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8933) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
/*
 * 0x2000 : TCC8935_D3_08X4_2CS_SV0.2 - DDR3 1024MB(16Bit)
 */
typedef enum {
#if defined(CONFIG_MMC_TCC_SUPPORT_EMMC)
	TCC_MMC_TYPE_EMMC,
	TCC_MMC_TYPE_SD,
#else
	TCC_MMC_TYPE_SD,
	TCC_MMC_TYPE_WIFI,
#endif
	TCC_MMC_TYPE_MAX
} tcc_mmc_type;

static struct mmc_port_config mmc_ports[] = {
#if defined(CONFIG_MMC_TCC_SUPPORT_EMMC)
  #if defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8933)
	[TCC_MMC_TYPE_EMMC] = {
		.data0	= TCC_GPD(29),
		.data1	= TCC_GPD(28),
		.data2	= TCC_GPD(27),
		.data3	= TCC_GPD(26),
		.data4	= TCC_GPD(25),
		.data5	= TCC_GPD(24),
		.data6	= TCC_GPD(23),
		.data7	= TCC_GPD(22),
		.cmd	= TCC_GPD(30),
		.clk	= TCC_GPD(31),
		.wp = TCC_GPD(21),
		.func	= GPIO_FN(2),
		.width	= TCC_MMC_BUS_WIDTH_8,
		.strength = GPIO_CD(1),

		.cd	= TCC_MMC_PORT_NULL,	//TCC_GPB(14),
		.pwr	= TCC_MMC_PORT_NULL,	// do not used!
	},
  #else
	[TCC_MMC_TYPE_EMMC] = {
		.data0	= TCC_GPD(18),
		.data1	= TCC_GPD(17),
		.data2	= TCC_GPD(16),
		.data3	= TCC_GPD(15),
		.data4	= TCC_GPD(14),
		.data5	= TCC_GPD(13),
		.data6	= TCC_GPD(12),
		.data7	= TCC_GPD(11),
		.cmd	= TCC_GPD(19),
		.clk	= TCC_GPD(20),
		.wp = TCC_MMC_PORT_NULL,
		.func	= GPIO_FN(2),
		.width	= TCC_MMC_BUS_WIDTH_8,
		.strength = GPIO_CD(1),

		.cd	= TCC_MMC_PORT_NULL,	//TCC_GPB(14),
		.pwr	= TCC_MMC_PORT_NULL,	// do not used!
	},
  #endif
	[TCC_MMC_TYPE_SD] = {
		.data0	= TCC_GPF(26),
		.data1	= TCC_GPF(25),
		.data2	= TCC_GPF(24),
		.data3	= TCC_GPF(23),
		.data4	= TCC_MMC_PORT_NULL,
		.data5	= TCC_MMC_PORT_NULL,
		.data6	= TCC_MMC_PORT_NULL,
		.data7	= TCC_MMC_PORT_NULL,
		.cmd	= TCC_GPF(27),
		.clk	= TCC_GPF(28),
		.wp = TCC_MMC_PORT_NULL,
		.func	= GPIO_FN(3),
		.width	= TCC_MMC_BUS_WIDTH_4,
		.strength = GPIO_CD(1),

		.cd	= TCC_GPE(28),
		.pwr	= TCC_MMC_PORT_NULL,
	},
#else
	[TCC_MMC_TYPE_SD] = {
		.data0	= TCC_GPF(26),
		.data1	= TCC_GPF(25),
		.data2	= TCC_GPF(24),
		.data3	= TCC_GPF(23),
		.data4	= TCC_MMC_PORT_NULL,
		.data5	= TCC_MMC_PORT_NULL,
		.data6	= TCC_MMC_PORT_NULL,
		.data7	= TCC_MMC_PORT_NULL,
		.cmd	= TCC_GPF(27),
		.clk	= TCC_GPF(28),
		.wp = TCC_MMC_PORT_NULL,
		.func	= GPIO_FN(3),
		.width	= TCC_MMC_BUS_WIDTH_4,
		.strength = GPIO_CD(1),

		.cd	= TCC_GPE(28),
		.pwr	= TCC_MMC_PORT_NULL,	// do not used!
	},
  #if defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8933)
	[TCC_MMC_TYPE_WIFI] = {
		.data0	= TCC_GPD(29),
		.data1	= TCC_GPD(28),
		.data2	= TCC_GPD(27),
		.data3	= TCC_GPD(26),
		.data4	= TCC_MMC_PORT_NULL,
		.data5	= TCC_MMC_PORT_NULL,
		.data6	= TCC_MMC_PORT_NULL,
		.data7	= TCC_MMC_PORT_NULL,
		.cmd	= TCC_GPD(30),
		.clk	= TCC_GPD(31),
		.wp = TCC_GPD(21),
		.func	= GPIO_FN(2),
		.width	= TCC_MMC_BUS_WIDTH_4,
		.strength = GPIO_CD(1),

		.cd = TCC_GPE(26),	//TCC_GPB(14),
		/* NOTICE!
		 * If you want to control power of wifi in tcc8933 and/or tcc8935,
		 * you must tune the hardware by enabling the given port to adjust power port.
		 * After that, replace following pwr port to TCC_GPD(6) to operate wifi appropriately.
		 * Refer to the schematic of tcc8935 and/or tcc8933.
		 */
		.pwr = GPIO_SD0_ON,
	},
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
		.wp = TCC_MMC_PORT_NULL,
		.func	= GPIO_FN(2),
		.width	= TCC_MMC_BUS_WIDTH_4,
		.strength = GPIO_CD(1),

		.cd	= TCC_MMC_PORT_NULL,	//TCC_GPB(14),
		.pwr	= TCC_MMC_PORT_NULL,	// do not used!
	},
  #endif
#endif
};
#endif

static int tccUsedSDportNum = TCC_MMC_TYPE_MAX;

int tcc8930_mmc_init(struct device *dev, int id)
{
	struct mmc_port_config *mmc_port;
	int mmc_gpio_config;

	BUG_ON(id >= tccUsedSDportNum);

	mmc_port = &mmc_ports[id];

	// 1. Power Up
#if defined(CONFIG_MMC_TCC_SUPPORT_EMMC)
	if(id == TCC_MMC_TYPE_EMMC)
	{
		gpio_request(GPIO_SD0_ON, "eMMC_Power");
		gpio_direction_output(GPIO_SD0_ON, 1);
	}
#endif

	if(mmc_port->pwr != TCC_MMC_PORT_NULL)
	{
		gpio_request(mmc_port->pwr, "WiFi_Power");
		gpio_direction_output(mmc_port->pwr, 0);
		msleep(100);
		gpio_direction_output(mmc_port->pwr, 1);
	}

#if 0//defined(CONFIG_BROADCOM_WIFI) && defined(CONFIG_SUPPORT_BCM4335)
	gpio_request(TCC_GPB(31), "bcm4335");
    gpio_direction_output(TCC_GPB(31), 0);
    msleep(300); // FIXME:
	gpio_direction_output(TCC_GPB(31), 1);
#endif
	
	// 2. GPIO Function
	mmc_gpio_config = mmc_port->func | mmc_port->strength;
	tcc_gpio_config(mmc_port->data0, mmc_gpio_config);
	tcc_gpio_config(mmc_port->data1, mmc_gpio_config);
	tcc_gpio_config(mmc_port->data2, mmc_gpio_config);
	tcc_gpio_config(mmc_port->data3, mmc_gpio_config);

	if(mmc_port->width == TCC_MMC_BUS_WIDTH_8)
	{
		tcc_gpio_config(mmc_port->data4, mmc_gpio_config);
		tcc_gpio_config(mmc_port->data5, mmc_gpio_config);
		tcc_gpio_config(mmc_port->data6, mmc_gpio_config);
		tcc_gpio_config(mmc_port->data7, mmc_gpio_config);
	}

	tcc_gpio_config(mmc_port->cmd, mmc_gpio_config);
	tcc_gpio_config(mmc_port->clk, mmc_port->func | GPIO_CD(3));

	// 3. Card Detect
	if(mmc_port->cd != TCC_MMC_PORT_NULL)
	{
		tcc_gpio_config(mmc_port->cd, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(mmc_port->cd, "sd_cd");

		gpio_direction_input(mmc_port->cd);
	}
	if(mmc_port->wp != TCC_MMC_PORT_NULL)
	{
		tcc_gpio_config(mmc_port->wp, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(mmc_port->wp, "sd_wp");

		gpio_direction_input(mmc_port->wp);
	}

	return 0;
}

extern int wifi_stat;

int tcc8930_mmc_card_detect(struct device *dev, int id)
{
	BUG_ON(id >= tccUsedSDportNum);

#if defined(CONFIG_BCM4330_SUB_BOARD)
	if(id == TCC_MMC_TYPE_WIFI)
		return 1;
#endif

#if defined(CONFIG_BROADCOM_WIFI)
	if(id == TCC_MMC_TYPE_WIFI && !wifi_stat) {
		return 0;
	}else if(id == TCC_MMC_TYPE_WIFI && wifi_stat){
		return 1;
	}
#endif

#if defined(CONFIG_SUPPORT_BCM4335)
	if(id == TCC_MMC_TYPE_SD && !wifi_stat) {
		return 0;
	}else if(id == TCC_MMC_TYPE_SD && wifi_stat){
		return 1;
	}
#endif

	if(mmc_ports[id].cd == TCC_MMC_PORT_NULL)
		return 1;

	return gpio_get_value(mmc_ports[id].cd) ? 0 : 1;	
}

int tcc8930_mmc_suspend(struct device *dev, int id)
{
        tcc_gpio_config(mmc_ports[id].clk, GPIO_FN(0)|GPIO_PULL_DISABLE);
#if defined(CONFIG_BROADCOM_WIFI)
	if(id != TCC_MMC_TYPE_WIFI)
	if(mmc_ports[id].pwr != TCC_MMC_PORT_NULL)
		gpio_direction_output(mmc_ports[id].pwr, 0);
#endif
	return 0;
}

int tcc8930_mmc_resume(struct device *dev, int id)
{
        tcc_gpio_config(mmc_ports[id].clk, mmc_ports[id].func|GPIO_CD(3));
#if defined(CONFIG_BROADCOM_WIFI)
	if(id != TCC_MMC_TYPE_WIFI)
	if(mmc_ports[id].pwr != TCC_MMC_PORT_NULL)
		gpio_direction_output(mmc_ports[id].pwr, 1);
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
	return gpio_get_value(mmc_ports[TCC_MMC_TYPE_SD].cd) ? 0 : 1;
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


#ifndef CONFIG_MMC_TCC_DISABLE_SD30
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

#ifdef CONFIG_CHIP_TCC8930
/*
 * 0x1000 : TCC8930_D3_08X4_SV_0.1 - DDR3 1024(32Bit)
 */
struct tcc_mmc_platform_data tcc8930_mmc_platform_data[] = {
#if defined(CONFIG_MMC_TCC_SUPPORT_EMMC)		
	// [0]:eMMC(boot),   [1]:SD,   [2]:WiFi
	[TCC_MMC_TYPE_EMMC] = {
		.slot	= 3,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			| MMC_CAP_8_BIT_DATA
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
		.resume	= tcc8930_mmc_resume,
		.set_power = tcc8930_mmc_set_power,
		.set_bus_width = tcc8930_mmc_set_bus_width,
		.get_ro = tcc8930_mmc_get_ro,

		.cd_int_num = -1,
		//.cd_irq_num = INT_EINT5,
		.cd_ext_irq = -1,
		.peri_name = PERI_SDMMC3,
		.io_name = RB_SDMMC3CONTROLLER,
		.pic = HwINT1_SD3,
	},
#endif
	[TCC_MMC_TYPE_SD] = {
		.slot	= 0,
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
#if 0
			/* for SD3.0 - 1.8V, 30MHz, DDR mode*/
			| MMC_CAP_UHS_DDR50
			| MMC_CAP_1_8V_DDR
#endif
			,
		.f_min = 100000,
#if defined(CONFIG_MMC_TCC_SD30_SDR12)
		.f_max = 24000000,
#elif defined(CONFIG_MMC_TCC_SD30_SDR25)
		.f_max = 48000000,
#elif defined(CONFIG_MMC_TCC_SD30_SDR50)
		.f_max = 80000000,
#elif defined(CONFIG_MMC_TCC_SD30_DDR50)
		.f_max = 30000000,
#else
		//.f_max = 44000000,
		.f_max = 80000000,
#endif
#if 0
		/* for SD3.0 - 1.8V, 30MHz, DDR mode*/
		.f_max = 30000000,
#endif
		.ocr_mask = 
#ifndef CONFIG_MMC_TCC_DISABLE_SD30
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
#ifndef CONFIG_MMC_TCC_DISABLE_SD30
		.switch_voltage = tcc8930_mmc_switch_voltage,
#endif
		.get_ro = tcc8930_mmc_get_ro,
#if defined(CONFIG_BROADCOM_WIFI) && defined(CONFIG_SUPPORT_BCM4335)
		.cd_int_num = -1,
		.cd_irq_num = -1,
		.cd_ext_irq = -1,
#else
		.cd_int_num = HwINT0_EI5,
		.cd_irq_num = INT_EINT5,
		.cd_ext_irq = EXTINT_GPIOB_12,
#endif
		.peri_name = PERI_SDMMC0,
		.io_name = RB_SDMMC0CONTROLLER,
		.pic = HwINT1_SD0,
	},
	[TCC_MMC_TYPE_WIFI] = {
		.slot	= 2,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			/* MMC_CAP_8_BIT_DATA */
			/* | MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED */,
		.f_min	= 100000,
		.f_max	= 24000000,	/* support highspeed mode */
		.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34,
		.init	= tcc8930_mmc_init,
		.card_detect = tcc8930_mmc_card_detect,
		.cd_int_config = tcc8930_mmc_cd_int_config,
		.suspend = tcc8930_mmc_suspend,
		.resume	= tcc8930_mmc_resume,
		.set_power = tcc8930_mmc_set_power,
		.set_bus_width = tcc8930_mmc_set_bus_width,
		.get_ro = tcc8930_mmc_get_ro,
#if defined(CONFIG_BROADCOM_WIFI) && !defined(CONFIG_SUPPORT_BCM4335)
		.cd_int_num = -1,
		.cd_irq_num = -1,
		.cd_ext_irq = -1,
#else
		.cd_int_num = HwINT0_EI4,
		.cd_irq_num = INT_EINT4,
		.cd_ext_irq = EXTINT_GPIOB_13,
#endif
		.peri_name = PERI_SDMMC2,
		.io_name = RB_SDMMC2CONTROLLER,
		.pic = HwINT1_SD2,
	},
};
#endif

#if defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8933) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
/*
 * 0x2000 : TCC8935_D3_08X4_2CS_SV0.2 - DDR3 1024MB(16Bit)
 */
struct tcc_mmc_platform_data tcc8930_mmc_platform_data[] = {
#if defined(CONFIG_MMC_TCC_SUPPORT_EMMC)		
  #if defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8933)
	// [0]:SD/eMMC(for booting device),   [1]:SD/WiFi
	[TCC_MMC_TYPE_EMMC] = {
		.slot	= 3,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			| MMC_CAP_8_BIT_DATA
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
		.resume	= tcc8930_mmc_resume,
		.set_power = tcc8930_mmc_set_power,
		.set_bus_width = tcc8930_mmc_set_bus_width,
		.get_ro = tcc8930_mmc_get_ro,

		.cd_int_num = -1,
		//.cd_irq_num = INT_EINT5,
		.cd_ext_irq = -1,
		.peri_name = PERI_SDMMC3,
		.io_name = RB_SDMMC3CONTROLLER,
		.pic = HwINT1_SD3,
	},
  #else
	[TCC_MMC_TYPE_EMMC] = {
		.slot	= 0,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			| MMC_CAP_8_BIT_DATA
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
		.resume	= tcc8930_mmc_resume,
		.set_power = tcc8930_mmc_set_power,
		.set_bus_width = tcc8930_mmc_set_bus_width,
		.get_ro = tcc8930_mmc_get_ro,

		.cd_int_num = -1,
		//.cd_irq_num = INT_EINT5,
		.cd_ext_irq = -1,
		.peri_name = PERI_SDMMC0,
		.io_name = RB_SDMMC0CONTROLLER,
		.pic = HwINT1_SD0,
	},
  #endif
	[TCC_MMC_TYPE_SD] = {
		.slot	= 2,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			/* MMC_CAP_8_BIT_DATA */
			| MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED,
		.f_min	= 100000,
		.f_max	= 48000000,	/* support highspeed mode */
		.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34,
		.init	= tcc8930_mmc_init,
		.card_detect = tcc8930_mmc_card_detect,
		.cd_int_config = tcc8930_mmc_cd_int_config,
		.suspend = tcc8930_mmc_suspend,
		.resume	= tcc8930_mmc_resume,
		.set_power = tcc8930_mmc_set_power,
		.set_bus_width = tcc8930_mmc_set_bus_width,
		.get_ro = tcc8930_mmc_get_ro,
		.cd_int_num = HwINT0_EI4,
		.cd_irq_num = INT_EINT4,
		.cd_ext_irq = EXTINT_GPIOE_28,
		.peri_name = PERI_SDMMC2,
		.io_name = RB_SDMMC2CONTROLLER,
		.pic = HwINT1_SD2,
	},
#else	
	// [0]:SD(for external storage),   [1]:WiFi
	[TCC_MMC_TYPE_SD] = {
		.slot	= 2,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			/* MMC_CAP_8_BIT_DATA */
			| MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED,
		.f_min	= 100000,
		.f_max	= 48000000,	/* support highspeed mode */
		.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34,
		.init	= tcc8930_mmc_init,
		.card_detect = tcc8930_mmc_card_detect,
		.cd_int_config = tcc8930_mmc_cd_int_config,
		.suspend = tcc8930_mmc_suspend,
		.resume	= tcc8930_mmc_resume,
		.set_power = tcc8930_mmc_set_power,
		.set_bus_width = tcc8930_mmc_set_bus_width,
		.get_ro = tcc8930_mmc_get_ro,
		.cd_int_num = HwINT0_EI4,
		.cd_irq_num = INT_EINT4,
		.cd_ext_irq = EXTINT_GPIOE_28,
		.peri_name = PERI_SDMMC2,
		.io_name = RB_SDMMC2CONTROLLER,
		.pic = HwINT1_SD2,
	},
  #if defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8933)
	[TCC_MMC_TYPE_WIFI] = {
		.slot	= 3,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			/*| MMC_CAP_8_BIT_DATA*/
			| MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED,		// SD0 Slot
		.f_min	= 100000,
		.f_max	= 48000000,	/* support highspeed mode */
		.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34,
		.init	= tcc8930_mmc_init,
		.card_detect = tcc8930_mmc_card_detect,
		.cd_int_config = tcc8930_mmc_cd_int_config,
		.suspend = tcc8930_mmc_suspend,
		.resume	= tcc8930_mmc_resume,
		.set_power = tcc8930_mmc_set_power,
		.set_bus_width = tcc8930_mmc_set_bus_width,
		.get_ro = tcc8930_mmc_get_ro,
#if defined(CONFIG_BROADCOM_WIFI) && !defined(CONFIG_SUPPORT_BCM4335)
		.cd_int_num = -1,
		.cd_irq_num = -1,
		.cd_ext_irq = -1,
#else
		.cd_int_num = HwINT0_EI5,
		.cd_irq_num = INT_EINT5,
		.cd_ext_irq = EXTINT_GPIOE_26,
#endif
		.peri_name = PERI_SDMMC3,
		.io_name = RB_SDMMC3CONTROLLER,
		.pic = HwINT1_SD3,
	},
  #else
	[TCC_MMC_TYPE_WIFI] = {
		.slot	= 0,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			//| MMC_CAP_8_BIT_DATA
			| MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED,	
		.f_min	= 100000,
		.f_max	= 48000000,	/* support highspeed mode */
		.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34,
		.init	= tcc8930_mmc_init,
		.card_detect = tcc8930_mmc_card_detect,
		.cd_int_config = tcc8930_mmc_cd_int_config,
		.suspend = tcc8930_mmc_suspend,
		.resume	= tcc8930_mmc_resume,
		.set_power = tcc8930_mmc_set_power,
		.set_bus_width = tcc8930_mmc_set_bus_width,
		.get_ro = tcc8930_mmc_get_ro,
#if defined(CONFIG_BROADCOM_WIFI) && !defined(CONFIG_SUPPORT_BCM4335)
		.cd_int_num = -1,
		.cd_irq_num = -1,
		.cd_ext_irq = -1,
#else
		.cd_int_num = HwINT0_EI5,
		.cd_irq_num = INT_EINT5,
		.cd_ext_irq = EXTINT_GPIOE_26,
#endif
		.peri_name = PERI_SDMMC0,
		.io_name = RB_SDMMC0CONTROLLER,
		.pic = HwINT1_SD0,
	},
  #endif
#endif
};
#endif

static void tcc8930_mmc_port_setup(void)
{
#if !defined(CONFIG_MMC_TCC_SUPPORT_EMMC) && defined(CONFIG_CHIP_TCC8930)
	printk("[%s] SD/MMC boot slot is set to disabled gpio function due to leakage current issue.\n",
			__func__);
	tcc_gpio_config(TCC_GPD(22), GPIO_FN(2));
	tcc_gpio_config(TCC_GPD(23), GPIO_FN(2));
	tcc_gpio_config(TCC_GPD(24), GPIO_FN(2));
	tcc_gpio_config(TCC_GPD(25), GPIO_FN(2));
	tcc_gpio_config(TCC_GPD(26), GPIO_FN(2));
	tcc_gpio_config(TCC_GPD(27), GPIO_FN(2));
	tcc_gpio_config(TCC_GPD(28), GPIO_FN(2));
	tcc_gpio_config(TCC_GPD(29), GPIO_FN(2));
	tcc_gpio_config(TCC_GPD(30), GPIO_FN(2));
	tcc_gpio_config(TCC_GPD(31), GPIO_FN(2));
	tcc_gpio_config(TCC_GPD(10), GPIO_FN(0)|GPIO_PULL_DISABLE);	// Write Protection, Input
	tcc_gpio_config(TCC_GPB(14), GPIO_FN(0)|GPIO_PULL_DISABLE);	// Card Detection, Input
#endif

#if 0
	/*
	 * 0x1000 : TCC8930_D3_08X4_SV_0.1 - DDR3 1024(32Bit)
	 * 0x2000 : TCC8935_D3_08X4_2CS_SV0.2 - DDR3 1024MB(16Bit)
	 */
	if (system_rev == 0x2000 || system_rev == 0x3000)
	{
#if defined(CONFIG_MMC_TCC_SUPPORT_EMMC)		// [0]:eMMC,   [1]:SD,   [2]:WiFi
		mmc_ports[TCC_MMC_TYPE_EMMC].data0 = TCC_GPD(29);
		mmc_ports[TCC_MMC_TYPE_EMMC].data1 = TCC_GPD(28);
		mmc_ports[TCC_MMC_TYPE_EMMC].data2 = TCC_GPD(27);
		mmc_ports[TCC_MMC_TYPE_EMMC].data3 = TCC_GPD(26);
		mmc_ports[TCC_MMC_TYPE_EMMC].data4 = TCC_GPD(25);
		mmc_ports[TCC_MMC_TYPE_EMMC].data5 = TCC_GPD(24);
		mmc_ports[TCC_MMC_TYPE_EMMC].data6 = TCC_GPD(23);
		mmc_ports[TCC_MMC_TYPE_EMMC].data7 = TCC_GPD(22);
		mmc_ports[TCC_MMC_TYPE_EMMC].cmd = TCC_GPD(30);
		mmc_ports[TCC_MMC_TYPE_EMMC].clk = TCC_GPD(31);
		mmc_ports[TCC_MMC_TYPE_EMMC].func = GPIO_FN(2);
#endif

		mmc_ports[TCC_MMC_TYPE_SD].data0 = TCC_GPF(26);
		mmc_ports[TCC_MMC_TYPE_SD].data1 = TCC_GPF(25);
		mmc_ports[TCC_MMC_TYPE_SD].data2 = TCC_GPF(24);
		mmc_ports[TCC_MMC_TYPE_SD].data3 = TCC_GPF(23);
		mmc_ports[TCC_MMC_TYPE_SD].cmd = TCC_GPF(27);
		mmc_ports[TCC_MMC_TYPE_SD].clk = TCC_GPF(28);
		mmc_ports[TCC_MMC_TYPE_SD].cd = TCC_GPE(28);
		mmc_ports[TCC_MMC_TYPE_SD].func = GPIO_FN(3);
	}
#endif
}

static int __init tcc8930_init_mmc(void)
{
	if (!machine_is_tcc893x())
		return 0;

	tccUsedSDportNum = TCC_MMC_TYPE_MAX;

	tcc8930_mmc_port_setup();
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

