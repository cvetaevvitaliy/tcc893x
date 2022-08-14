/* ==========================================================================
 * $File: //dwh/usb_iip/dev/software/DWC_usb3/driver/driver.h $
 * $Revision: #40 $
 * $Date: 2012/01/23 $
 * $Change: 1924640 $
 *
 * Synopsys SS USB3 Linux Software Driver and documentation (hereinafter,
 * "Software") is an Unsupported proprietary work of Synopsys, Inc. unless
 * otherwise expressly agreed to in writing between Synopsys and you.
 *
 * The Software IS NOT an item of Licensed Software or Licensed Product under
 * any End User Software License Agreement or Agreement for Licensed Product
 * with Synopsys or any supplement thereto. You are permitted to use and
 * redistribute this Software in source and binary forms, with or without
 * modification, provided that redistributions of source code must retain this
 * notice. You may not view, use, disclose, copy or distribute this file or
 * any information contained herein except pursuant to this license grant from
 * Synopsys. If you do not agree with this notice, including the disclaimer
 * below, then you are not authorized to use the Software.
 *
 * THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS" BASIS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * ========================================================================== */

#ifndef _DWC_DRIVER_H_
#define _DWC_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "debug.h"
#include <reg.h>
#include <arch/arm.h>

/** @file
 * This file contains the interface to the Linux driver.
 */

/**
 * The following parameters may be specified when loading the module. These
 * parameters define how the DWC_usb3 controller should be configured.
 * Parameter values are passed to the CIL initialization function
 * dwc_usb3_common_init.
 */
typedef struct dwc_usb3_core_params {	/** @< */
	int burst;		/** @< */
	int newcore;		/** @< */
	int phy;		/** @< */
	int wakeup;		/** @< */
	int pwrctl;		/** @< */
	int u1u2ctl;		/** @< */
	int lpmctl;		/** @< */
	int phyctl;		/** @< */
	int usb2mode;		/** @< */
	int hibernate;		/** @< */
	int hiberdisc;		/** @< */
	int clkgatingen;	/** @< */
	int ssdisquirk;		/** @< */
	int nobos;		/** @< */
	int loop;		/** @< */
	int nump;		/** @< */
	int rx;			/** @< */
	int tx0;		/** @< */
	int tx1;		/** @< */
	int tx2;		/** @< */
	int tx3;
} dwc_usb3_core_params_t;

/**
 * This structure is a wrapper that encapsulates the driver components used to
 * manage a single DWC_usb3 controller.
 */
typedef struct dwc_usb3_device {
	/**
	 * OS-specific stuff. KEEP THIS AT THE VERY BEGINNING OF THE DEVICE
	 * STRUCT. OSes such as FreeBSD and NetBSD require this.
	 */

	/** Base address returned from ioremap() */
	volatile unsigned char  *base;

	/** Register offset for Diagnostic API */
	unsigned int reg_offset;

#if 0		/* 012.08.13 */
	struct task_struct *pme_thread;
	int pme_ready;

	/** Count of threads inside Gadget API */
	atomic_t hiber_cnt;

	/** Hibernation state */
	atomic_t hibernate;
	
#define DWC_HIBER_AWAKE		0
#define DWC_HIBER_ENTER_NOSAVE	1
#define DWC_HIBER_ENTER_SAVE	2
#define DWC_HIBER_SLEEPING	3
#define DWC_HIBER_WAIT_LINK_UP	4
#define DWC_HIBER_SS_DIS_QUIRK	5
#endif /* 0 */
	/* ==== End of OS-specific stuff ==== */

	/** PCD structure */
	struct dwc_usb3_pcd pcd;

	/** Value from SNPSID register */
	unsigned int snpsid;

	/** Parameters that define how the core should be configured */
	dwc_usb3_core_params_t *core_params;

	/** Core Global registers starting at offset 000h. */
	dwc_usb3_core_global_regs_t  *core_global_regs;

#define DWC_EVENT_BUF_SIZE	256	// size in dwords
#define DWC_NUM_EVENT_BUFS	1
	/** Event Buffers for receiving interrupts. Up to 32 buffers are
	 * supported by the hardware, but we only use 1
	 */
	unsigned int *event_ptr[DWC_NUM_EVENT_BUFS];	/** @< */
	unsigned int *event_buf[DWC_NUM_EVENT_BUFS];	/** @< */
	dwc_dma_t event_buf_dma[DWC_NUM_EVENT_BUFS];

	/** Total RAM for FIFOs (Bytes) */
	unsigned short total_fifo_size;

	/** Size of Rx FIFO (Bytes) */
	unsigned short rx_fifo_size;

	/** Hardware Configuration - stored here for convenience */
	unsigned int hwparams0;	/** @< */
	unsigned int hwparams1;	/** @< */
	unsigned int hwparams2;	/** @< */
	unsigned int hwparams3;	/** @< */
	unsigned int hwparams4;	/** @< */
	unsigned int hwparams5;	/** @< */
	unsigned int hwparams6;	/** @< */
	unsigned int hwparams7;	/** @< */
	unsigned int hwparams8;

	/** Register state, saved across core hibernation */
	unsigned int dcfg_save;		/** @< */
	unsigned int dctl_save;		/** @< */
	unsigned int gtxfifosiz0_save;	/** @< */
	unsigned int gtxfifosiz1_save;	/** @< */
	unsigned int gtxfifosiz2_save;	/** @< */
	unsigned int gtxfifosiz3_save;	/** @< */
	unsigned int grxfifosiz0_save;

	/** True if common functionality has been initialized */
	unsigned int cmn_initialized		: 1;

	/** True if PCD has been initialized */
	unsigned int pcd_initialized		: 1;

	/** True if common IRQ handler has been installed */
	unsigned int common_irq_installed	: 1;

	/** True if sysfs functions have been installed */
	unsigned int sysfs_initialized		: 1;

	/** True if PHY interface bits in USBCFG have been initialized */
	unsigned int phy_init_done		: 1;

	/** True to save core state in hibernation */
	unsigned int hiber_state		: 1;

	/** True if waiting for connect before resuming from hibernation */
	unsigned int hiber_wait_connect		: 1;

	/** True if waiting for U0 state before sending remote wake */
	unsigned int hiber_wait_u0		: 1;
} dwc_usb3_device_t;

/**
 * Register read/write.
 */
#define dwc_rd32(dev, adr)			readl(adr)
//#define dwc_rd32(dev, adr)		({ unsigned int __val__; (dev)->hwparams1++; __val__ = readl(adr); __val__; })
#define dwc_wr32(dev, adr, val)		writel((val), (adr))
//#define dwc_wr32(dev, adr, val)		do { (dev)->hwparams1++; writel((val), (adr)); } while (0)

/**
 * Non-sleeping delays.
 */


//#define dwc_udelay(dev, us)	udelay(us)

//#define dwc_udelay(dev, us)	do { (dev)->hwparams1++; udelay(us); } while (0)
//#define dwc_mdelay(dev, ms)	mdelay(ms)
#define dwc_mdelay(dev, ms)	wait_phy_reset_timing(ms)
//#define dwc_mdelay(dev, ms)	do { (dev)->hwparams1++; mdelay(ms); } while (0)

/**
 * Sleeping delay.
 */
//#define dwc_msleep(dev, ms)	mdelay(ms)
#define dwc_msleep(dev, ms)	wait_phy_reset_timing(ms)
//#define dwc_msleep(dev, ms)	msleep(ms)

//#define dwc_msleep(dev, ms)	do { (dev)->hwparams1++; msleep(ms); } while (0)

/**
 * Debugging support - vanishes in non-debug builds.
 */

/** Prefix string for print macros. */
#define USB3_DWC	"dwc_usb3: "

#if 0
//#ifdef DEBUG
//# define dwc_debug(dev, x...)	printk(KERN_DEBUG USB3_DWC x )
#define dwc_debug(dev, x...)    dprintf(4, x) 
//# define dwc_debug(dev, x...)	do { (dev)->hwparams1++; printk(KERN_DEBUG USB3_DWC x ); } while (0)
#else
# define dwc_debug(dev, x...)	do {} while (0)
#endif /* DEBUG */

# define dwc_debug0(dev, fmt)			dwc_debug(dev, fmt)
# define dwc_debug1(dev, fmt, a)		dwc_debug(dev, fmt, a)
# define dwc_debug2(dev, fmt, a, b)		dwc_debug(dev, fmt, a, b)
# define dwc_debug3(dev, fmt, a, b, c)		dwc_debug(dev, fmt, a, b, c)
# define dwc_debug4(dev, fmt, a, b, c, d)	dwc_debug(dev, fmt, a, b, c, d)
# define dwc_debug5(dev, fmt, a, b, c, d, e) \
			dwc_debug(dev, fmt, a, b, c, d, e)
# define dwc_debug6(dev, fmt, a, b, c, d, e, f) \
			dwc_debug(dev, fmt, a, b, c, d, e, f)
# define dwc_debug7(dev, fmt, a, b, c, d, e, f, g) \
			dwc_debug(dev, fmt, a, b, c, d, e, f, g)
# define dwc_debug8(dev, fmt, a, b, c, d, e, f, g, h) \
			dwc_debug(dev, fmt, a, b, c, d, e, f, g, h)
# define dwc_debug9(dev, fmt, a, b, c, d, e, f, g, h, i) \
			dwc_debug(dev, fmt, a, b, c, d, e, f, g, h, i)
# define dwc_debug10(dev, fmt, a, b, c, d, e, f, g, h, i, j) \
			dwc_debug(dev, fmt, a, b, c, d, e, f, g, h, i, j)

#if defined(DEBUG) || defined(ISOC_DEBUG)
//# define dwc_isocdbg(dev, x...)	printk(USB3_DWC x )
#define dwc_isocdbg(dev, x...)    dprintf(4, x)
//# define dwc_isocdbg(dev, x...)	do { (dev)->hwparams1++; printk(USB3_DWC x ); } while (0)
#else
# define dwc_isocdbg(dev, x...)	do {} while (0)
#endif

# define dwc_isocdbg0(dev, fmt)			dwc_isocdbg(dev, fmt)
# define dwc_isocdbg1(dev, fmt, a)		dwc_isocdbg(dev, fmt, a)
# define dwc_isocdbg2(dev, fmt, a, b)		dwc_isocdbg(dev, fmt, a, b)
# define dwc_isocdbg3(dev, fmt, a, b, c)	dwc_isocdbg(dev, fmt, a, b, c)
# define dwc_isocdbg4(dev, fmt, a, b, c, d) \
			dwc_isocdbg(dev, fmt, a, b, c, d)
# define dwc_isocdbg5(dev, fmt, a, b, c, d, e) \
			dwc_isocdbg(dev, fmt, a, b, c, d, e)
# define dwc_isocdbg6(dev, fmt, a, b, c, d, e, f) \
			dwc_isocdbg(dev, fmt, a, b, c, d, e, f)

/**
 * Print an Error message.
 */
//#define dwc_error(dev, x...)	printk(KERN_ERR USB3_DWC x )
#define dwc_error(dev, x...)	 dprintf(4, x)
//#define dwc_error(dev, x...)	do { (dev)->hwparams1++; printk(KERN_ERR USB3_DWC x ); } while (0)

#define dwc_error0(dev, fmt)			dwc_error(dev, fmt)
#define dwc_error1(dev, fmt, a)			dwc_error(dev, fmt, a)
#define dwc_error2(dev, fmt, a, b)		dwc_error(dev, fmt, a, b)
#define dwc_error3(dev, fmt, a, b, c)		dwc_error(dev, fmt, a, b, c)
#define dwc_error4(dev, fmt, a, b, c, d)	dwc_error(dev, fmt, a, b, c, d)

/**
 * Print a Warning message.
 */
//#define dwc_warn(dev, x...)	printk(KERN_WARNING USB3_DWC x )
#define dwc_warn(dev, x...)	 dprintf(4, x)

//#define dwc_warn(dev, x...)	do { (dev)->hwparams1++; printk(KERN_WARNING USB3_DWC x ); } while (0)

#define dwc_warn0(dev, fmt)			dwc_warn(dev, fmt)
#define dwc_warn1(dev, fmt, a)			dwc_warn(dev, fmt, a)
#define dwc_warn2(dev, fmt, a, b)		dwc_warn(dev, fmt, a, b)
#define dwc_warn3(dev, fmt, a, b, c)		dwc_warn(dev, fmt, a, b, c)
#define dwc_warn4(dev, fmt, a, b, c, d)		dwc_warn(dev, fmt, a, b, c, d)

/**
 * Print an Informational message (normal but significant).
 */
//#define dwc_info(dev, x...)	printk(KERN_INFO USB3_DWC x )
#define dwc_info(dev, x...)	 dprintf(4, x)
//#define dwc_info(dev, x...)	do { (dev)->hwparams1++; printk(KERN_INFO USB3_DWC x ); } while (0)

#define dwc_info0(dev, fmt)			dwc_info(dev, fmt)
#define dwc_info1(dev, fmt, a)			dwc_info(dev, fmt, a)
#define dwc_info2(dev, fmt, a, b)		dwc_info(dev, fmt, a, b)
#define dwc_info3(dev, fmt, a, b, c)		dwc_info(dev, fmt, a, b, c)
#define dwc_info4(dev, fmt, a, b, c, d)		dwc_info(dev, fmt, a, b, c, d)

/**
 * Basic message printing.
 */
//#define dwc_print(dev, x...)	printk(USB3_DWC x )
#define dwc_print(dev, x...)	 dprintf(4, x)
//#define dwc_print(dev, x...)	do { (dev)->hwparams1++; printk(USB3_DWC x ); } while (0)

#define dwc_print0(dev, fmt)			dwc_print(dev, fmt)
#define dwc_print1(dev, fmt, a)			dwc_print(dev, fmt, a)
#define dwc_print2(dev, fmt, a, b)		dwc_print(dev, fmt, a, b)
#define dwc_print3(dev, fmt, a, b, c)		dwc_print(dev, fmt, a, b, c)
#define dwc_print4(dev, fmt, a, b, c, d)	dwc_print(dev, fmt, a, b, c, d)
#define dwc_print5(dev, fmt, a, b, c, d, e) \
			dwc_print(dev, fmt, a, b, c, d, e)

#ifdef CONFIG_HITECH
extern int dwc_usb3_init(struct pci_dev *dev);
extern void dwc_usb3_remove(struct pci_dev *dev);
#endif

#ifdef CONFIG_IPMATE
extern int dwc_usb3_init(struct lm_device *dev);
extern void dwc_usb3_remove(struct lm_device *dev);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _DWC_DRIVER_H_ */
