/*
 * linux/arch/arm/mach-tcc893x/devices.
 *
 * Copyright (C) 2011 Telechips, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/module.h>

#include <asm/setup.h>
#include <asm/io.h>
#include <asm/mach/flash.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/persistent_ram.h>
#include <linux/platform_data/dwc3-tcc.h>

#include <mach/bsp.h>
#include <plat/pmap.h>
#include <mach/mmc.h>
#include <mach/tcc_fb.h>
#include <mach/ohci.h>
#include <mach/tcc_cam_ioctrl.h>
#include <mach/tccfb_ioctrl.h>

#include <linux/delay.h>

#ifdef CONFIG_ANDROID_RAM_CONSOLE
static struct resource ram_console_resources[] = {
	{
		.flags = IORESOURCE_MEM,
	},
};

static struct platform_device ram_console_device = {
	.name		= "ram_console",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(ram_console_resources),
	.resource	= ram_console_resources,
};

static struct persistent_ram_descriptor ram_console_desc = {
	.name = "ram_console",
};

static struct persistent_ram ram_console_ram = {
	.num_descs = 1,
	.descs = &ram_console_desc,
};

void __init tcc_add_ram_console(void)
{
	struct resource *res;
	pmap_t pmap_ram_console;
	if (!pmap_get_info("ram_console", &pmap_ram_console))
		return;

	ram_console_desc.size = pmap_ram_console.size;

	ram_console_ram.start = pmap_ram_console.base;
	ram_console_ram.size = pmap_ram_console.size;
	persistent_ram_early_init(&ram_console_ram);

	res = platform_get_resource(&ram_console_device, IORESOURCE_MEM, 0);
	if (!res) {
		pr_err("Failed to get resource for ram console\n");
		return;
	}
	res->start = pmap_ram_console.base;
	res->end = pmap_ram_console.base + pmap_ram_console.size - 1;
	platform_device_register(&ram_console_device);
}
#endif

#ifdef CONFIG_MMC_TCC_SDHC
#include <mach/mmc.h>

/*----------------------------------------------------------------------
 * Device	  : SD/MMC resource
 * Description: tcc8930_sdhc_resource[4][2]
 *----------------------------------------------------------------------*/
static struct resource tcc_sdhc_resource[4][2] = {
	[0] = {		//SDHC Host Controller 0
		[0] = {
			.start	= tcc_p2v(HwSDMMC0_BASE),
			.end	= tcc_p2v(HwSDMMC0_BASE + 0x1ff),
			.flags	= IORESOURCE_MEM,
		},
		[1] = {
			.start	= INT_SDMMC0,
			.end	= INT_SDMMC0,
			.flags	= IORESOURCE_IRQ,
		},
	},
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
#else
	[1] = {		//SDHC Host Controller 1
		[0] = {
			.start	= tcc_p2v(HwSDMMC1_BASE),
			.end	= tcc_p2v(HwSDMMC1_BASE + 0x1ff),
			.flags	= IORESOURCE_MEM,
		},
		[1] = {
			.start	= INT_SDMMC1,
			.end	= INT_SDMMC1,
			.flags	= IORESOURCE_IRQ,
		},
	},
#endif
	[2] = {		//SDHC Host Controller 2
		[0] = {
			.start	= tcc_p2v(HwSDMMC2_BASE),
			.end	= tcc_p2v(HwSDMMC2_BASE + 0x1ff),
			.flags	= IORESOURCE_MEM,
		},
		[1] = {
			.start	= INT_SDMMC2,
			.end	= INT_SDMMC2,
			.flags	= IORESOURCE_IRQ,
		},
	},
	[3] = {		//SDHC Host Controller 3
		[0] = {
			.start	= tcc_p2v(HwSDMMC3_BASE),
			.end	= tcc_p2v(HwSDMMC3_BASE + 0x1ff),
			.flags	= IORESOURCE_MEM,
		},
		[1] = {
			.start	= INT_SDMMC3,
			.end	= INT_SDMMC3,
			.flags	= IORESOURCE_IRQ,
		},
	},
};

#if defined(CONFIG_MMC_TCC_SDHC0)
static u64 tcc_device_sdhc0_dmamask = 0xffffffffUL;
struct platform_device tcc_sdhc0_device = {
	.name			= "tcc-sdhc0",
	.id 			= 0,
	.num_resources	= ARRAY_SIZE(tcc_sdhc_resource[0]),
	.resource		= (struct resource *)&tcc_sdhc_resource[0],
	.dev			= {
		.dma_mask			= &tcc_device_sdhc0_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};
#endif

#if defined(CONFIG_MMC_TCC_SDHC1)
static u64 tcc_device_sdhc1_dmamask = 0xffffffffUL;
struct platform_device tcc_sdhc1_device = {
	.name			= "tcc-sdhc1",
	.id 			= 1,
	.num_resources	= ARRAY_SIZE(tcc_sdhc_resource[1]),
	.resource		= (struct resource *)&tcc_sdhc_resource[1],
	.dev			= {
		.dma_mask			= &tcc_device_sdhc1_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};
#endif

#if defined(CONFIG_MMC_TCC_SDHC2)
static u64 tcc_device_sdhc2_dmamask = 0xffffffffUL;
struct platform_device tcc_sdhc2_device = {
	.name			= "tcc-sdhc2",
	.id 			= 2,
	.num_resources	= ARRAY_SIZE(tcc_sdhc_resource[2]),
	.resource		= (struct resource *)&tcc_sdhc_resource[2],
	.dev			= {
		.dma_mask			= &tcc_device_sdhc2_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};
#endif

#if defined(CONFIG_MMC_TCC_SDHC3)
static u64 tcc_device_sdhc3_dmamask = 0xffffffffUL;
struct platform_device tcc_sdhc3_device = {
	.name			= "tcc-sdhc3",
	.id 			= 3,
	.num_resources	= ARRAY_SIZE(tcc_sdhc_resource[3]),
	.resource		= (struct resource *)&tcc_sdhc_resource[3],
	.dev			= {
		.dma_mask			= &tcc_device_sdhc3_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};
#endif

extern struct tcc_mmc_platform_data tcc8930_mmc_platform_data[];

void tcc_init_sdhc_devices(void)
{
	int core;

#if defined(CONFIG_MMC_TCC_SDHC0)
	core = tcc8930_mmc_platform_data[0].slot % 4;
	tcc_sdhc0_device.num_resources	= ARRAY_SIZE(tcc_sdhc_resource[core]);
	tcc_sdhc0_device.resource		= (struct resource *)&tcc_sdhc_resource[core];
#endif
#if defined(CONFIG_MMC_TCC_SDHC1)
	core = tcc8930_mmc_platform_data[1].slot % 4;
	tcc_sdhc1_device.num_resources	= ARRAY_SIZE(tcc_sdhc_resource[core]);
	tcc_sdhc1_device.resource		= (struct resource *)&tcc_sdhc_resource[core];
#endif
#if defined(CONFIG_MMC_TCC_SDHC2)
	core = tcc8930_mmc_platform_data[2].slot % 4;
	tcc_sdhc2_device.num_resources	= ARRAY_SIZE(tcc_sdhc_resource[core]);
	tcc_sdhc2_device.resource		= (struct resource *)&tcc_sdhc_resource[core];
#endif
#if defined(CONFIG_MMC_TCC_SDHC3)
	core = tcc8930_mmc_platform_data[3].slot % 4;
	tcc_sdhc3_device.num_resources	= ARRAY_SIZE(tcc_sdhc_resource[core]);
	tcc_sdhc3_device.resource		= (struct resource *)&tcc_sdhc_resource[core];
#endif
}
#endif

#if defined(CONFIG_I2C_TCC)
/*----------------------------------------------------------------------
 * Device     : I2C resource
 * Description: tcc8930_i2c_core0_resources
 *				tcc8930_i2c_core1_resources
 *				tcc8930_i2c_core2_resources
 *				tcc8930_i2c_core3_resources
 *				tcc8930_i2c_smu_resources
 *
 * [0] = {										// channel
 *		.name	= "master0"						// channel name
 *		.start	= tcc_p2v(HwI2C_MASTER0_BASE),	// base address
 *		.end	= HwI2C_PORTCFG_BASE + 0x0C     // IRQSTR
 *		.flags	= IORESOURCE_IRQ,				// resource type
 * },
 *----------------------------------------------------------------------*/
#if defined(CONFIG_I2C_TCC_CORE0)
static struct resource tcc8930_i2c_core0_resources[] = {
	[0] = {
		.name	= "master0",
		.start  = HwI2C_MASTER0_BASE,
		.end    = HwI2C_PORTCFG_BASE + 0x0C,
		.flags	= IORESOURCE_IO,
	},
};
struct platform_device tcc8930_i2c_core0_device = {
    .name           = "tcc-i2c",
    .id             = 0,
    .resource       = tcc8930_i2c_core0_resources,
    .num_resources  = ARRAY_SIZE(tcc8930_i2c_core0_resources),
};
#endif
#if defined(CONFIG_I2C_TCC_CORE1)
static struct resource tcc8930_i2c_core1_resources[] = {
	[0] = {
		.name	= "master1",
        .start  = HwI2C_MASTER1_BASE,
        .end    = HwI2C_PORTCFG_BASE + 0x0C,
		.flags	= IORESOURCE_IO,
    },
};
struct platform_device tcc8930_i2c_core1_device = {
    .name           = "tcc-i2c",
    .id             = 1,
    .resource       = tcc8930_i2c_core1_resources,
    .num_resources  = ARRAY_SIZE(tcc8930_i2c_core1_resources),
};
#endif
#if defined(CONFIG_I2C_TCC_CORE2)
static struct resource tcc8930_i2c_core2_resources[] = {
	[0] = {
		.name	= "master2",
		.start  = HwI2C_MASTER2_BASE,
		.end    = HwI2C_PORTCFG_BASE + 0x0C,
		.flags	= IORESOURCE_IO,
	},
};
struct platform_device tcc8930_i2c_core2_device = {
    .name           = "tcc-i2c",
    .id             = 2,
    .resource       = tcc8930_i2c_core2_resources,
    .num_resources  = ARRAY_SIZE(tcc8930_i2c_core2_resources),
};
#endif
#if defined(CONFIG_I2C_TCC_CORE3)
static struct resource tcc8930_i2c_core3_resources[] = {
	[0] = {
		.name	= "master3",
        .start  = HwI2C_MASTER3_BASE,
        .end    = HwI2C_PORTCFG_BASE + 0x0C,
		.flags	= IORESOURCE_IO,
    },
};
struct platform_device tcc8930_i2c_core3_device = {
    .name           = "tcc-i2c",
    .id             = 3,
    .resource       = tcc8930_i2c_core3_resources,
    .num_resources  = ARRAY_SIZE(tcc8930_i2c_core3_resources),
};
#endif
#if defined(CONFIG_I2C_TCC_SMU)
static struct resource tcc8930_i2c_smu_resources[] = {
	[0] = {
		.name	= "master0",
		.start  = HwSMUI2C_BASE,
        .end    = HwSMUI2C_BASE + 0x80,
		.flags	= IORESOURCE_IO,
    }
};
struct platform_device tcc8930_i2c_smu_device = {
    .name           = "tcc-i2c",
    .id             = 4,
    .resource       = tcc8930_i2c_smu_resources,
    .num_resources  = ARRAY_SIZE(tcc8930_i2c_smu_resources),
};
#endif
#endif  /* #if defined(CONFIG_I2C_TCC) */

#if defined(CONFIG_TCC_GMAC) || defined(CONFIG_TCC_GMAC_MODULE)

static u64 tcc_gmac_dma_mask = ~(u32)0;
static struct resource tcc_gmac_resources[] = {
	[0] = {
		.start	= tcc_p2v(HwGMAC_BASE),
		.end	= tcc_p2v(HwGMAC_BASE) + 0x2000,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_GMAC,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device tcc_gmac_device = {
	.name			= "tcc-gmac",
	.id 			= -1, 
	.dev			= {
		.dma_mask = &tcc_gmac_dma_mask,
		.coherent_dma_mask = 0xffffffffUL,
	},
	.num_resources	= ARRAY_SIZE(tcc_gmac_resources),
	.resource		= tcc_gmac_resources,
};
#endif

#if defined(CONFIG_SERIAL_TCC) || defined(CONFIG_SERIAL_TCC_MODULE)
/*----------------------------------------------------------------------
 * Device     : UART resource
 * Description: uart0_resources
 *              uart1_resources
 *              uart2_resources (not used)
 *              uart3_resources (not used)
 *              uart4_resources (not used)
 *              uart5_resources (not used)
 *----------------------------------------------------------------------*/
static struct resource uart0_resources[] = {
    /* PA -> VA */
    [0] = {
        .start  = tcc_p2v(HwUART0_BASE),
        .end    = tcc_p2v(HwUART0_BASE) + 0xff,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART0,
        .end    = INT_UART0,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc8930_uart0_device = {
    .name           = "tcc-uart",
    .id             = 0,
    .resource       = uart0_resources,
    .num_resources  = ARRAY_SIZE(uart0_resources),
};

static struct resource uart1_resources[] = {
    [0] = {
        .start  = tcc_p2v(HwUART1_BASE),
        .end    = tcc_p2v(HwUART1_BASE) + 0xff,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART1,
        .end    = INT_UART1,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc8930_uart1_device = {
    .name           = "tcc-uart",
    .id             = 1,
    .resource       = uart1_resources,
    .num_resources  = ARRAY_SIZE(uart1_resources),
};

static struct resource uart2_resources[] = {
    [0] = {
        .start  = tcc_p2v(HwUART2_BASE),
        .end    = tcc_p2v(HwUART2_BASE) + 0xff,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART2,
        .end    = INT_UART2,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc8930_uart2_device = {
    .name           = "tcc-uart",
    .id             = 2,
    .resource       = uart2_resources,
    .num_resources  = ARRAY_SIZE(uart2_resources),
};

static struct resource uart3_resources[] = {
    [0] = {
        .start  = tcc_p2v(HwUART3_BASE),
        .end    = tcc_p2v(HwUART3_BASE) + 0xff,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART3,
        .end    = INT_UART3,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc8930_uart3_device = {
    .name           = "tcc-uart",
    .id             = 3,
    .resource       = uart3_resources,
    .num_resources  = ARRAY_SIZE(uart3_resources),
};

static struct resource uart4_resources[] = {
    [0] = {
        .start  = tcc_p2v(HwUART4_BASE),
        .end    = tcc_p2v(HwUART4_BASE) + 0xff,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART4,
        .end    = INT_UART4,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc8930_uart4_device = {
    .name           = "tcc-uart",
    .id             = 4,
    .resource       = uart4_resources,
    .num_resources  = ARRAY_SIZE(uart4_resources),
};

static struct resource uart5_resources[] = {
    [0] = {
        .start  = tcc_p2v(HwUART5_BASE),
        .end    = tcc_p2v(HwUART5_BASE) + 0xff,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART5,
        .end    = INT_UART5,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc8930_uart5_device = {
    .name           = "tcc-uart",
    .id             = 5,
    .resource       = uart5_resources,
    .num_resources  = ARRAY_SIZE(uart5_resources),
};
#endif

#if defined(CONFIG_USB_XHCI_TCC) || defined(CONFIG_USB_XHCI_TCC_MODULE)
static u64 tcc8930_device_xhci_dmamask = 0xffffffffUL;

static struct resource tcc8930_xhci_hs_resources[] = {
	[0] = {
		.start = tcc_p2v(HwUSBLINK_BASE),
		.end   = tcc_p2v(HwUSBLINK_BASE) + 0xC848,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_USB30,
		.end   = INT_USB30,
		.flags = IORESOURCE_IRQ,
	}
};

struct platform_device xhci_hs_device = {
	.name			= "tcc-xhci",
	.id				= -1,
	.num_resources  = ARRAY_SIZE(tcc8930_xhci_hs_resources),
	.resource       = tcc8930_xhci_hs_resources,
	.dev			= {
		.dma_mask 			= &tcc8930_device_xhci_dmamask,
		.coherent_dma_mask	= 0xffffffff,
		//.platform_data	= tcc8930_ehci_hs_data,
	},
};

#endif
/*----------------------------------------------------------------------
 * Device     : USB HOST2.0 EHCI resource
 * Description: tcc8930_ehci_resources
 *----------------------------------------------------------------------*/
#if defined(CONFIG_USB_EHCI_TCC) || defined(CONFIG_USB_EHCI_TCC_MODULE)
static u64 tcc8930_device_ehci_dmamask = 0xffffffffUL;
 
static struct resource tcc8930_ehci_hs_resources[] = {
	[0] = {
		.start = tcc_p2v(HwUSB20HOST_EHCI_BASE),
		.end   = tcc_p2v(HwUSB20HOST_EHCI_BASE) + 0x108,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_USB20H,
		.end   = INT_USB20H,
		.flags = IORESOURCE_IRQ,
	}
};

struct platform_device ehci_hs_device = {
	.name			= "tcc-ehci",
	.id				= -1,
	.num_resources  = ARRAY_SIZE(tcc8930_ehci_hs_resources),
	.resource       = tcc8930_ehci_hs_resources,
	.dev			= {
		.dma_mask 			= &tcc8930_device_ehci_dmamask,
		.coherent_dma_mask	= 0xffffffff,
		//.platform_data	= tcc8930_ehci_hs_data,
	},
};

static int tcc8930_echi_fs_init(struct device *dev)
{
	return 0;
}

static struct tccohci_platform_data tcc8930_ehci_fs_platform_data = {
	.port_mode	= USBOHCI_PPM_PERPORT,
	.init		= tcc8930_echi_fs_init,
};

static struct resource tcc8930_ehci_fs_resources[] = {
	[0] = {
		.start = tcc_p2v(HwUSB20HOST_OHCI_BASE),
		.end   = tcc_p2v(HwUSB20HOST_OHCI_BASE) + 0x0fff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_USB20H,
		.end   = INT_USB20H,
		.flags = IORESOURCE_IRQ,
	}
};

struct platform_device ehci_fs_device = {
	.name			= "tcc-ohci",
	.id				= 1,
	.num_resources  = ARRAY_SIZE(tcc8930_ehci_fs_resources),
	.resource       = tcc8930_ehci_fs_resources,
	.dev			= {
		.dma_mask 			= &tcc8930_device_ehci_dmamask,
		.coherent_dma_mask	= 0xffffffff,
		.platform_data		= &tcc8930_ehci_fs_platform_data,
	},
};
#else
static inline void tcc8930_init_ehci(void) {}
#endif /*CONFIG_USB_EHCI_TCC  || CONFIG_USB_EHCI_TCC_MODULE*/

/*----------------------------------------------------------------------
 * Device     : USB DWC OTG resource
 * Description: dwc_otg_resources
 *----------------------------------------------------------------------*/
#if defined(CONFIG_TCC_DWC_OTG) || defined(CONFIG_TCC_DWC_OTG_MODULE)
static u64 tcc8930_dwc_otg_dmamask = 0xffffffffUL;
struct resource dwc_otg_resources[] = {
	[0] = {
		.start	= tcc_p2v(HwUSBOTG_BASE),
		.end	= tcc_p2v(HwUSBOTG_BASE) + 0x100,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_UOTG,
		.end   = INT_UOTG,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device tcc8930_dwc_otg_device = {
	.name			= "dwc_otg",
	.id				= 0,
	.resource		= dwc_otg_resources,
	.num_resources	= ARRAY_SIZE(dwc_otg_resources),
	.dev			= {
		.dma_mask 			= &tcc8930_dwc_otg_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
};
#endif

/*----------------------------------------------------------------------
 * Device     : USB DWC OTG resource
 * Description: dwc_otg_resources
 *----------------------------------------------------------------------*/
#if defined(CONFIG_USB_DWC3) || defined(CONFIG_USB_DWC3_MODULE)
extern int tcc_usb30_vbus_init(void);
extern int tcc_usb30_vbus_exit(void);
extern int tcc_dwc3_vbus_ctrl(int on);

extern void tcc_usb30_phy_off(void);
extern void tcc_usb30_phy_on(void);

extern int tcc_usb30_clkset(int on);

#define TXVRT_SHIFT 6
#define TXVRT_MASK (0xF << TXVRT_SHIFT)
#define TXRISE_SHIFT 10
#define TXRISE_MASK (0x3 << TXRISE_SHIFT)

static void tcc_stop_dwc3(void)
{
	tcc_usb30_clkset(0);
}

static void tcc_start_dwc3(void)
{
	tcc_usb30_clkset(1);
}

#include <asm/mach-types.h>

static int tcc_usb30_phy_init(struct platform_device *pdev, int type)
{
	PUSBPHYCFG USBPHYCFG = (PUSBPHYCFG)tcc_p2v(HwUSBPHYCFG_BASE);
	PUSB3_GLB	gUSB3_GLB = (PUSB3_GLB)tcc_p2v(HwUSBGLOBAL_BASE);

   	unsigned int uTmp, time_delay = 0;
	
	while (1) {
#if !defined(CONFIG_TCC_USB_DRD)
      tcc_usb30_vbus_init();
#endif

		tcc_dwc3_vbus_ctrl(1);
		tcc_start_dwc3();
		tcc_usb30_phy_on();

		msleep(1);
		//msleep(time_delay);
		
		// Initialize All PHY & LINK CFG Registers
		USBPHYCFG->UPCR0 = 0xB5484068;
		USBPHYCFG->UPCR1 = 0x0000041C;
		USBPHYCFG->UPCR2 = 0x919E14C8;
		USBPHYCFG->UPCR3 = 0x4AB05D00;
#if !defined(CONFIG_TCC_USB_DRD)
		USBPHYCFG->UPCR4 = 0x00000000;
#endif
		USBPHYCFG->UPCR5 = 0x00108001;
		USBPHYCFG->LCFG = 0x80420013;
#if defined(CONFIG_TCC_USB_DRD)
		BITSET(USBPHYCFG->UPCR4, Hw20|Hw21); //ID pin interrupt enable
#endif
		msleep(1);
		
		USBPHYCFG->UPCR1 = 0x00000412;
		USBPHYCFG->LCFG	 = 0x86420013;

		// PHY Clock Setting (all are reset values)
		// USB3PHY_CFG0[26:21] : FSEL = 6'b101010 
		// USB3PHY_CFG0[28:27] : REFCLKSEL = 2'b10
		// USB3PHY_CFG0[29:29] : REF_SSP_EN = 1'b1
		// USB3PHY_CFG0[30:30] : REF_USE_PAD = 1'b0
		// USB3PHY_CFG0[31:31] : SSC_EN = 1'b1

		// Setting LINK Suspend clk dividor (all are reset values)
		//HwHSB_CFG->uUSB30LINK_CFG.bReg.SUSCLK_DIV_EN = 0x1;
		//HwHSB_CFG->uUSB30LINK_CFG.bReg.SUSCLK_DIV = 0x3; // busclk / (3+1) = 250 / 4 = 15.625MHz

		//HC DC controll
		BITCSET(USBPHYCFG->UPCR2, TXVRT_MASK, 0xC << TXVRT_SHIFT);
		BITCSET(gUSB3_GLB->GCTL.nREG,(Hw12|Hw13), Hw13);
#if 1
		// USB20 Only Mode
		USBPHYCFG->LCFG	 |= Hw30;	//USB2.0 HS only mode
		USBPHYCFG->UPCR1 |= Hw9;	//test_powerdown_ssp
#else
		// USB 3.0
		BITCLR(USBPHYCFG->LCFG, Hw30);
		BITCLR(USBPHYCFG->UPCR1,Hw9);

		//USBPHYCFG->UPCR1 |= Hw8; 	//test_powerdown_hsp
#endif
		// Wait USB30 PHY initialization finish. PHY FREECLK should be stable.
		msleep(time_delay++);
		// GDBGLTSSM : Check LTDB Sub State is non-zero
		// When PHY FREECLK is valid, LINK mac_resetn is released and LTDB Sub State change to non-zero
		uTmp = readl(tcc_p2v(HwUSBDEVICE_BASE+0x164));
		uTmp = (uTmp>>18)&0xF;

		// Break when LTDB Sub state is non-zero and CNR is zero
		if (uTmp != 0 || time_delay > 500) {
		        break;
		}
	}
	
	if(uTmp)
		printk("\x1b[1;33mtime delay = %d  \x1b[0m\n",time_delay);
	else
		printk("\x1b[1;31mPHY stable is not guaranteed\x1b[0m\n");
	
	return 0;
}

static int tcc_usb30_phy_exit(struct platform_device *pdev, int type)
{
   tcc_usb30_phy_off();
   tcc_stop_dwc3();
   tcc_dwc3_vbus_ctrl(0);
#if !defined(CONFIG_TCC_USB_DRD)
   tcc_usb30_vbus_exit();
#endif

   return 0;
}

static struct dwc3_tcc_data dwc3_tcc_data = {
	.phy_init = tcc_usb30_phy_init,
	.phy_exit = tcc_usb30_phy_exit,
};

static u64 tcc8930_dwc3_dmamask = 0xffffffffUL;
struct resource dwc3_resources[] = {
	[0] = {
		.start	= HwUSBLINK_BASE,
		.end	= HwUSBLINK_BASE + 0xCFFF,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_USB30,
		.end   = INT_USB30,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device tcc8930_dwc3_device = {
	.name		= "tcc-dwc3",
	.id				= 0,
	.resource		= dwc3_resources,
	.num_resources	= ARRAY_SIZE(dwc3_resources),
	.resource       = dwc3_resources,
	.dev			= {
	.dma_mask 			= &tcc8930_dwc3_dmamask,
	.coherent_dma_mask	= 0xffffffff,
		.platform_data		= &dwc3_tcc_data,
	},
};
#endif

#if defined(CONFIG_FB_TCC)
/*----------------------------------------------------------------------
 * Device     : LCD Frame Buffer resource
 * Description: 
 *----------------------------------------------------------------------*/

static u64 tcc_device_lcd_dmamask = 0xffffffffUL;

struct platform_device tcc_lcd_device = {
	.name	  = "tccfb",
	.id		  = -1,
	.dev      = {
		.dma_mask		    = &tcc_device_lcd_dmamask,
//		.coherent_dma_mask	= 0xffffffffUL
	}
};
#endif//

int tcc_panel_id = -1;

static int __init parse_tag_tcc_panel(const struct tag *tag)
{
	int *id = (int *) &tag->u;

	tcc_panel_id = *id;

	return 0;
}
__tagtable(ATAG_TCC_PANEL, parse_tag_tcc_panel);

int tcc_is_camera_enable = -1;

static int __init parse_tag_tcc_is_camera_enable(const struct tag *tag)
{
        int *is_camera_enable = (int *) &tag->u;

        tcc_is_camera_enable = *is_camera_enable;

        return 0;
}
__tagtable(ATAG_CAMERA, parse_tag_tcc_is_camera_enable);

#define ATAG_CHIPREV	0x54410010

int tcc_chip_rev = -1;

static int __init parse_tag_chip_revision(const struct tag *tag)
{
        int *chip_rev = (int *) &tag->u;

        tcc_chip_rev = (unsigned int)*chip_rev;

        return 0;
}
__tagtable(ATAG_CHIPREV, parse_tag_chip_revision);

EXPORT_SYMBOL(tcc_chip_rev);

struct display_platform_data tcc_display_data = {
	.resolution = 0,
	.output = 0,
	.hdmi_resolution = 0,
	.composite_resolution = 0,
	.component_resolution = 0,
	.hdmi_mode = 0
};

//#if(1) // defined(CONFIG_TCC_ADC) || defined(CONFIG_TCC_ADC_MODULE)
/*----------------------------------------------------------------------
 * Device	  : ADC resource
 * Description: tcc8930_adc_resource
 *----------------------------------------------------------------------*/
struct resource tcc8930_adc_resources[] = {
	[0] = {
		.start	= HwTSADC_BASE,
		.end	= HwTSADC_BASE + 0x24,
		.flags	= IORESOURCE_MEM,
	},
};

struct platform_device tcc8930_adc_device = {
	.name		= "tcc-adc",
	.id			= -1,
	.resource	= tcc8930_adc_resources,
	.num_resources	= ARRAY_SIZE(tcc8930_adc_resources),
};
//#endif 

#ifdef CONFIG_BATTERY_TCC
struct platform_device tcc_battery_device = {
    .name           = "tcc-battery",
    .id             = -1,
};
#endif /* CONFIG_BATTERY_TCC */

/*----------------------------------------------------------------------
 * Device     : SPI(GPSB) Master resource
 * Description: 
 *----------------------------------------------------------------------*/
#if defined(CONFIG_SPI_TCCXXXX_MASTER)
static struct resource spi0_resources[] = {
	[0] = {
		.start = tcc_p2v(HwGPSB0_BASE),
		.end   = tcc_p2v(HwGPSB0_BASE + 0x38),
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_GPSB0_DMA,
		.end   = INT_GPSB0_DMA,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {	    
	    .name   = "gpsb0",
#if   defined(CONFIG_MACH_TCC8930ST)		        
   		.start	= 6,/* Port6 GPIO_B[11:14] */
		.end	= 6,		
#else
		.start	= 4,/* Port4 GPIO_D[17:20] */
		.end	= 4,
#endif		
		.flags	= IORESOURCE_IO,
	},
};

struct platform_device tcc8930_spi0_device = {
	.name		= "tcc-spi",
	.id			= 0,
	.resource	= spi0_resources,
	.num_resources	= ARRAY_SIZE(spi0_resources),
};


static struct resource spi1_resources[] = {
	[0] = {
		.start = tcc_p2v(HwGPSB2_BASE),
		.end   = tcc_p2v(HwGPSB2_BASE + 0x38),
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_GPSB2_DMA,
		.end   = INT_GPSB2_DMA,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {	    
	    .name   = "gpsb2",
		.start	= 5,
		.end	= 5,
		.flags	= IORESOURCE_IO,
	},
};

struct platform_device tcc8930_spi1_device = {
	.name		= "tcc-spi",
	.id			= 1,
	.resource	= spi1_resources,
	.num_resources	= ARRAY_SIZE(spi1_resources),
};
#endif

/*----------------------------------------------------------------------
 * Device     : RTC resource
 * Description: tcc8930_rtc_resources
 *----------------------------------------------------------------------*/
#if defined(CONFIG_RTC_DRV_TCC8930)
struct resource tcc8930_rtc_resource[] = {
    [0] = {
        .start	= tcc_p2v(HwRTC_BASE),
        .end	= tcc_p2v(HwRTC_BASE + 0x50),
        .flags	= IORESOURCE_MEM,
    },
    [1] = {
        .start	= INT_RTC,
        .end	= INT_RTC,
        .flags	= IORESOURCE_IRQ,
    }
};

struct platform_device tcc8930_rtc_device = {
    .name           = "tcc-rtc",
    .id             = -1,
    .resource       = tcc8930_rtc_resource,
    .num_resources  = ARRAY_SIZE(tcc8930_rtc_resource),
};
#endif  /* CONFIG_RTC_DRV_TCC */

/*----------------------------------------------------------------------
 * Device     : remote control resource
 * Description:
 *----------------------------------------------------------------------*/
#if defined(CONFIG_INPUT_TCC_REMOTE)
struct resource tcc8930_remote_resources[] = {
#if 1
	[0] = {
		.start	= tcc_p2v(HwREMOTE_BASE),
		.end	= tcc_p2v(HwREMOTE_BASE+0x44),
		.flags	= IORESOURCE_MEM,
	},
    [1] = {
	    .start = INT_REMOCON,
	    .end   = INT_REMOCON,
        .flags = IORESOURCE_IRQ,
    },
#endif
};

struct platform_device tcc8930_remote_device = {
	.name			= "tcc-remote",
	.id				= -1,
	.resource		= tcc8930_remote_resources,
	.num_resources	= ARRAY_SIZE(tcc8930_remote_resources),
};
#if 0
static inline void tcc8930_init_remote(void)
{
	platform_device_register(&tcc8930_remote_device);
}
#endif
#endif /* CONFIG_INPUT_TCC_REMOTE */

/*----------------------------------------------------------------------
 * Device     : SPI(TSIF) Slave resource
 * Description:
 *----------------------------------------------------------------------*/
#if defined(CONFIG_SPI_TCCXXXX_TSIF_SLAVE)
static struct resource tsif_resources[] = {
	[0] = {
		.start = tcc_p2v(HwGPSB1_BASE),
		.end   = tcc_p2v(HwGPSB1_BASE + 0x38),
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_GPSB1_DMA,
		.end   = INT_GPSB1_DMA,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {
	    .name   = "gpsb1",
#if   defined(CONFIG_MACH_TCC8930ST)		        
   		.start	= 4,/* Port4 GPIO_D[17:20] */
		.end	= 4,
#elif defined(CONFIG_MACH_M805_893X)
   		.start	= 10,/* Port10 GPIO_C[10:13] */
		.end	= 10,
#else
   		.start	= 5,/* Port5 GPIO_B[0:3] */
		.end	= 5,
#endif        
		.flags	= IORESOURCE_IO,
	}
};

struct platform_device tcc_tsif_device = {
	.name		= "tcc-tsif",
	.id		= -1,
	.resource	= tsif_resources,
	.num_resources	= ARRAY_SIZE(tsif_resources),
};

/*----------------------------------------------------------------------
 *  * Device     : TSIF Block
 *   * Description:
 *    *----------------------------------------------------------------------*/
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
static struct resource tsif_ex_resources[] = {
    [0] = {
		.start = tcc_p2v(HwTSIF0_BASE),
		.end   = tcc_p2v(HwTSIF0_BASE + 0x10000),
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_TSIF_DMA0,
		.end   = INT_TSIF_DMA0,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {
        .name   = "tsif0",
   		.start	= 0,/* Port0 GPIO_D[7:10] */
		.end	= 0,
		.flags	= IORESOURCE_IO,
	}
};

struct platform_device tcc_tsif_ex_device = {
	.name		= "tcc-tsif_ex",
	.id			= -1,
	.resource	= tsif_ex_resources,
	.num_resources	= ARRAY_SIZE(tsif_ex_resources),
};
#endif

#endif

#if defined(CONFIG_TCC_ECID_SUPPORT)
struct platform_device tcc_cpu_id_device = {
	.name	= "cpu-id",
	.id		= -1,
};
#endif

#if defined(CONFIG_SND_SOC) || defined(CONFIG_SND_SOC_MODULE)
static struct platform_device tcc_pcm = {
	.name	= "tcc-pcm-audio",
	.id	= -1,
};

static struct platform_device tcc_dai = {
	.name	= "tcc-dai",
	.id	= -1,
};

static struct platform_device tcc_iec958 = {
	.name	= "tcc-iec958",
	.id	= -1,
};

//Planet 20120615 Audio Driver Improvement Start	
static struct platform_device tcc_pcm_ch1 = {
	.name	= "tcc-pcm-audio-ch1",
	.id	= -1,
};

static struct platform_device tcc_dai_ch1 = {
	.name	= "tcc-dai-ch1",
	.id	= -1,
};

static struct platform_device tcc_iec958_ch1 = {
	.name	= "tcc-iec958-ch1",
	.id	= -1,
};
//Planet 20120615 Audio Driver Improvement End

#if defined(CONFIG_SND_SOC_WM8524)
static struct platform_device tcc_wm8524 = {
	.name	= "tcc-wm8524",
	.id	= -1,
};
#endif

static void tcc_init_audio(void)
{
	platform_device_register(&tcc_pcm);
	platform_device_register(&tcc_dai);
	platform_device_register(&tcc_iec958);

	platform_device_register(&tcc_pcm_ch1);		//Planet 20120615 Audio Driver Improvement
	platform_device_register(&tcc_dai_ch1);		//Planet 20120615 Audio Driver Improvement
	platform_device_register(&tcc_iec958_ch1);	//Planet 20120615 Audio Driver Improvement

#if defined(CONFIG_SND_SOC_WM8524)
	platform_device_register(&tcc_wm8524);
#endif
}
#else
static void tcc_init_audio(void){;}
#endif

static struct platform_device tcc_cm3_ctrl = {
       .name   = "tcc_cm3_ctrl",
       .id     = -1,
};

static int __init tcc893x_init_devices(void)
{
#ifdef CONFIG_ANDROID_RAM_CONSOLE
	tcc_add_ram_console();
#endif

    tcc_init_audio();

    platform_device_register(&tcc_cm3_ctrl);

#if defined(CONFIG_SPI_TCCXXXX_MASTER)
 #if   defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8933) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
  #if defined(CONFIG_MACH_M805_893X)
   #if defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8933)
    spi0_resources[2].start = 2; /* Port2 GPIO_D[22:25] */
    spi0_resources[2].end = 2;
   #elif defined(CONFIG_CHIP_TCC8935S)
    spi0_resources[2].start = 3; /* Port2 GPIO_D[11:14] */
    spi0_resources[2].end = 3;
   #elif defined(CONFIG_CHIP_TCC8937S) 
    spi0_resources[2].start = 17; /* Port2 GPIO_G[00:03] */
    spi0_resources[2].end = 17;
   #endif
  #elif defined(CONFIG_STB_BOARD_YJ8935T)	|| defined(CONFIG_STB_BOARD_YJ8933T)/* Port 6 GPIO_B[11:14] */
  #else
    spi0_resources[2].start = 12; /* Port12 GPIO_E[17:20] */
    spi0_resources[2].end = 12;
  #endif
 #elif defined(CONFIG_CHIP_TCC8930)
  #if defined(CONFIG_STB_BOARD_EV_TCC893X) || defined(CONFIG_STB_BOARD_YJ8930T)
  #else
    spi0_resources[2].start = 17; /* Port17 GPIO_G[0:3] */
    spi0_resources[2].end = 17;
  #endif
 #endif
#endif    

#if defined(CONFIG_SPI_TCCXXXX_TSIF_SLAVE)
 #if   defined(CONFIG_MACH_TCC893X)
    tsif_resources[2].start = 17; /* Port17 GPIO_G[0:3] */
    tsif_resources[2].end = 17;
 #endif
#endif    
	return 0;
}
arch_initcall(tcc893x_init_devices);
