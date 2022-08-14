/*
 * xHCI host controller driver PCI Bus Glue.
 *
 * Copyright (C) 2008 Intel Corp.
 *
 * Author: Sarah Sharp
 * Some code borrowed from the Linux EHCI driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

//#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/platform_device.h>

#include <mach/bsp.h>
#include <asm/mach-types.h>

#include "xhci.h"

extern int tcc_usb30_vbus_init(void);
extern int tcc_usb30_vbus_exit(void);
extern int tcc_xhci_vbus_ctrl(int on);
extern void tcc_usb30_clkset(int on);


static const char hcd_name[] = "xhci_hcd";

#if 0
#define REG_WR32(a,b)	    (*((volatile int *)(tcc_p2v(a))) = b)
#define REG_RD32(a,b)		(b=*((volatile int *)(tcc_p2v(a))))
#endif

PUSBPHYCFG gUSBPHYCFG = (PUSBPHYCFG)tcc_p2v(HwUSBPHYCFG_BASE);
PUSB3_XHCI 	gUSB3_XHCI = (PUSB3_XHCI)tcc_p2v(HwUSBLINK_BASE);
PUSB3_GLB	gUSB3_GLB = (PUSB3_GLB)tcc_p2v(HwUSBGLOBAL_BASE);

#if defined(CONFIG_USB_XHCI_SS_MODE)
#define USB3_SPEED_MODE 0
#else
#define USB3_SPEED_MODE 1
#endif

static inline const char *platform_name(const struct platform_device *pdev)
{
	return dev_name(&pdev->dev);
}

static void tcc_start_xhci(void)
{
	tcc_usb30_clkset(1);
}

static void tcc_stop_xhci(void)
{
	tcc_usb30_clkset(0);
}

void set_mode_usb20_only (void)
{
	BITSET(gUSBPHYCFG->LCFG,Hw30);
}


void wait_phy_reset_timing (unsigned int count)
{
    unsigned int i;
    unsigned int tmp;

    // Wait 10us
    tmp = 0;
    for (i=0;i<count;i++)
        tmp++;
}

void usb30_software_reset (void)
{
    // Assert reset
    //HwUSB30->sGlobal.uGCTL.bReg.CoreSoftReset = 1;
    BITSET(gUSB3_GLB->GCTL.nREG, Hw11);
	
    //HwUSB30->sGlobal.uGUSB2PHYCFG.bReg.PHYSoftRst = 1;
    BITSET(gUSB3_GLB->GUSB2PHYCFG.nREG, Hw31);
	
    //HwUSB30->sGlobal.uGUSB3PIPECTL.bReg.PHYSoftRst = 1;
    BITSET(gUSB3_GLB->GUSB3PIPECTL.nREG, Hw31);
	
    // turn on all PHY circuits
    //HwHSB_CFG->uUSB30PHY_CFG1.bReg.test_powerdown_hsp = 0; // turn on HS circuit
    //HwHSB_CFG->uUSB30PHY_CFG1.bReg.test_powerdown_ssp = 0; // turn on SS circuit
    BITCLR(gUSBPHYCFG->UPCR1,Hw9|Hw8);

    // wait T1 (>10us)
    wait_phy_reset_timing(300*2);

    // De-assert reset
    //HwUSB30->sGlobal.uGUSB2PHYCFG.bReg.PHYSoftRst = 0;
    BITCLR(gUSB3_GLB->GUSB2PHYCFG.nREG, Hw31);
	
    //HwUSB30->sGlobal.uGUSB3PIPECTL.bReg.PHYSoftRst = 0;
    BITCLR(gUSB3_GLB->GUSB3PIPECTL.nREG, Hw31);

    // Wait T3-T1 (>165us)
    wait_phy_reset_timing(4800*2);

    //HwUSB30->sGlobal.uGCTL.bReg.CoreSoftReset = 0;
    BITCLR(gUSB3_GLB->GCTL.nREG, Hw11);
}


void powerdown_ss (void)
{
	PUSBPHYCFG USBPHYCFG = (PUSBPHYCFG)tcc_p2v(HwUSBPHYCFG_BASE);

    //HwUSB30->sGlobal.uGUSB2PHYCFG.bReg.PHYSoftRst = 1; // Assert Global PHY reset
    BITSET(gUSB3_GLB->GUSB2PHYCFG.nREG, Hw31);
	//HwHSB_CFG->uUSB30PHY_CFG1.bReg.test_powerdown_ssp = 1; // turn off SS circuit
	BITSET(USBPHYCFG->UPCR1,Hw9);

    // Wait T1 (>10us)
    //msleep(1);
    wait_phy_reset_timing(300*2);

    //HwUSB30->sGlobal.uGUSB2PHYCFG.bReg.PHYSoftRst = 0; // De-Assert Global PHY reset
    BITCLR(gUSB3_GLB->GUSB2PHYCFG.nREG, Hw31);

    // Wait T3-T1 (>165us)
    //msleep(1);
    wait_phy_reset_timing(4800*2);
}

void wait_phy_clock_valid(void)
{
    unsigned int uTmp;

    // Wait USB30 PHY initialization finish. PHY FREECLK should be stable.
    while(1) {
        // GDBGLTSSM : Check LTDB Sub State is non-zero
        // When PHY FREECLK is valid, LINK mac_resetn is released and LTDB Sub State change to non-zero
        //uTmp = HwUSB30->sGlobal.uGDBGLTSSM.bReg.LTDBSubState;
        uTmp = readl(tcc_p2v(HwUSBGLOBAL_BASE+0x64));
        uTmp = (uTmp>>18)&0xF;

        // Break when LTDB Sub state is non-zero and CNR is zero
        if (uTmp != 0){
            break;
        }
    }
}

#define TXVRT_SHIFT 6
#define TXVRT_MASK (0xF << TXVRT_SHIFT)

#define TXRISE_SHIFT 10
#define TXRISE_MASK (0x3 << TXRISE_SHIFT)

static void tcc_xhci_phy_set(void)
{
	PUSBPHYCFG USBPHYCFG = (PUSBPHYCFG)tcc_p2v(HwUSBPHYCFG_BASE);

	//	printk(KERN_CRIT "\x1b[1;33m%s: dwc_usb3_phy_init()\x1b[0m\n", dwc_driver_name);
	// Initialize All PHY & LINK CFG Registers
	USBPHYCFG->UPCR0 = 0xB5484068;
	USBPHYCFG->UPCR1 = 0x0000041C;
	USBPHYCFG->UPCR2 = 0x919E14C8;
	USBPHYCFG->UPCR3 = 0x4AB05D00;
	#if !defined(CONFIG_TCC_USB_DRD)
    USBPHYCFG->UPCR4 = 0x00000000;
	#endif
	USBPHYCFG->UPCR5 = 0x00108001;
	USBPHYCFG->LCFG	 = 0x80420013;

#if defined(CONFIG_TCC_USB_DRD)
	BITSET(USBPHYCFG->UPCR4, Hw20|Hw21);
#endif
	// De-assert Resets
	// USB30PHY_CFG1[01:01] : PIPE_RESETN = 1'b1;
	// USB30PHY_CFG1[02:02] : UTMI_RESET(=PORTRESET) = 1'b0;
	// USB30PHY_CFG1[03:03] : PHY_RESET = 1'b0;
	// USB30PHY_CFG1[10:10] : COMMONONN = 1'b1;
	// USB30LINK_CFG[25:25] : VCC_RESETN = 1'b1;
	// USB30LINK_CFG[26:26] : VAUX_RESETN = 1'b1;

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

	// USB20 Only Mode
#if defined(CONFIG_USB_XHCI_SS_MODE)
	BITCLR(USBPHYCFG->LCFG, Hw30);
    BITCLR(USBPHYCFG->UPCR1,Hw9);
#else
	BITSET(USBPHYCFG->LCFG, Hw30);
    BITSET(USBPHYCFG->UPCR1,Hw9);
#endif
    //HC DC controll
    BITCSET(USBPHYCFG->UPCR2, TXVRT_MASK, 0xC << TXVRT_SHIFT);
    BITCSET(USBPHYCFG->UPCR2, TXRISE_MASK, 0x3 << TXRISE_SHIFT);
	//USBPHYCFG->LCFG	 |= Hw30;	//USB2.0 HS only mode
    //USBPHYCFG->UPCR1 |= Hw9;	//test_powerdown_ssp
    //BITCLR(USBPHYCFG->UPCR1,Hw9);
    //USBPHYCFG->UPCR1 |= Hw8; 	//test_powerdown_hsp

	// Wait USB30 PHY initialization finish. PHY FREECLK should be stable.
	wait_phy_clock_valid();
}

extern void tcc_xhci_phy_on(void);
extern void tcc_xhci_phy_off(void);
extern void tcc_xhci_phy_ctrl(int on);//taejin TODO
void tcc_init_usb3host (uint mode) {

#if !defined(CONFIG_TCC_USB_DRD)
	tcc_usb30_vbus_init();
#endif

	/* USB HOST Power Enable */
	if (tcc_xhci_vbus_ctrl(1) != 0) {
		printk(KERN_ERR "xhci-tcc: USB HOST VBUS failed\n");
		return;
	}
	/* USB SS Nano-Phy Enable */
	tcc_start_xhci();
	tcc_xhci_phy_ctrl(1);
	tcc_xhci_phy_set();
		
    // Set host mode
	BITCSET(gUSB3_GLB->GCTL.nREG,(Hw12|Hw13), Hw12);
#if 0
    //HwHSB_CFG->uUSB30PHY_CFG4.nReg = 0x00A00000;
    BITSET(gUSBPHYCFG->UPCR4,Hw23|Hw21);
    BITCLR(gUSBPHYCFG->UPCR4,Hw22|Hw20);

	//OTG block Disable
    BITSET(gUSBPHYCFG->UPCR5,Hw15);

    usb30_software_reset();
#endif
    //HwUSB30_GLOBAL->GSBUSCFG0 = 0x2;
    BITSET(gUSB3_GLB->GSBUSCFG0.nREG, Hw1);
	
    // if mode == 1, that is usb 2.0 only mode,
    if (mode)
    {
		printk("\x1b[1;33mXHCI Only USB 2.0 Mode Support  \x1b[0m\n");
        powerdown_ss();
    	set_mode_usb20_only();
    }
	else
		printk("\x1b[1;33mXHCI USB 3.0 Mode Support  \x1b[0m\n");

	//BITSET(gUSB3_XHCI->PORTSC1.nREG, Hw0);  //Device is present on port
	BITSET(gUSB3_XHCI->PORTSC1.nREG, Hw1);	//Port Enable! mandatory
	//BITSET(gUSB3_XHCI->PORTSC1.nREG, Hw9);	//portis not in the powered-off state.

	//BITSET(gUSB3_XHCI->USBCMD.nREG, Hw0);		//run bit

    //HW Auto Retry enable
    BITSET(gUSB3_GLB->GUCTL.nREG, Hw14);

}


typedef void (*xhci_get_quirks_t)(struct device *, struct xhci_hcd *);

static void tcc_xhci_quirks(struct device *dev, struct xhci_hcd *xhci)
{
        /* Don't use MSI interrupt */
        xhci->quirks |= XHCI_BROKEN_MSI;
}

int xhci_gen_setup(struct usb_hcd *hcd, xhci_get_quirks_t get_quirks)
{
	struct xhci_hcd		*xhci;
	struct device		*dev = hcd->self.controller;
	int			retval;
	u32			temp;

	hcd->self.sg_tablesize = TRBS_PER_SEGMENT - 2;

	if (usb_hcd_is_primary_hcd(hcd)) {
		xhci = kzalloc(sizeof(struct xhci_hcd), GFP_KERNEL);
		if (!xhci)
			return -ENOMEM;
		*((struct xhci_hcd **) hcd->hcd_priv) = xhci;
		xhci->main_hcd = hcd;
		/* Mark the first roothub as being USB 2.0.
		 * The xHCI driver will register the USB 3.0 roothub.
		 */
		 #if 1
		hcd->speed = HCD_USB2;
		hcd->self.root_hub->speed = USB_SPEED_HIGH;
		#else
		hcd->speed = HCD_USB3;
		hcd->self.root_hub->speed = USB_SPEED_SUPER;
		#endif
		/*
		 * USB 2.0 roothub under xHCI has an integrated TT,
		 * (rate matching hub) as opposed to having an OHCI/UHCI
		 * companion controller.
		 */
		hcd->has_tt = 1;
	} else {
		/* xHCI private pointer was set in xhci_pci_probe for the second
		 * registered roothub.
		 */
		xhci = hcd_to_xhci(hcd);
		temp = xhci_readl(xhci, &xhci->cap_regs->hcc_params);
		if (HCC_64BIT_ADDR(temp)) {
			xhci_dbg(xhci, "Enabling 64-bit DMA addresses.\n");
			dma_set_mask(hcd->self.controller, DMA_BIT_MASK(64));
		} else {
			dma_set_mask(hcd->self.controller, DMA_BIT_MASK(32));
		}
		return 0;
	}
	

	xhci->cap_regs = hcd->regs;
	xhci->op_regs = hcd->regs +
		HC_LENGTH(xhci_readl(xhci, &xhci->cap_regs->hc_capbase));
	xhci->run_regs = hcd->regs +
		(xhci_readl(xhci, &xhci->cap_regs->run_regs_off) & RTSOFF_MASK);
	/* Cache read-only capability registers */
	xhci->hcs_params1 = xhci_readl(xhci, &xhci->cap_regs->hcs_params1);
	xhci->hcs_params2 = xhci_readl(xhci, &xhci->cap_regs->hcs_params2);
	xhci->hcs_params3 = xhci_readl(xhci, &xhci->cap_regs->hcs_params3);
	
	xhci->hcc_params = xhci_readl(xhci, &xhci->cap_regs->hc_capbase);
	xhci->hci_version = HC_VERSION(xhci->hcc_params);
	
	xhci->hcc_params = xhci_readl(xhci, &xhci->cap_regs->hcc_params);
	xhci_print_registers(xhci);

	get_quirks(dev, xhci);

	/* Make sure the HC is halted. */
	retval = xhci_halt(xhci);
	if (retval){
		printk("\x1b[1;31m func:%s, xhci_halt fail!!\x1b[0m\n",__func__);
		goto error;
	}
	
	xhci_dbg(xhci, "Resetting HCD\n");
	/* Reset the internal HC memory state and registers. */
	retval = xhci_reset(xhci);
	if (retval)
		goto error;
	xhci_dbg(xhci, "Reset complete\n");

	temp = xhci_readl(xhci, &xhci->cap_regs->hcc_params);
	if (HCC_64BIT_ADDR(temp)) {
		xhci_dbg(xhci, "Enabling 64-bit DMA addresses.\n");
		dma_set_mask(hcd->self.controller, DMA_BIT_MASK(64));
	} else {
		dma_set_mask(hcd->self.controller, DMA_BIT_MASK(32));
	}

	xhci_dbg(xhci, "Calling HCD init\n");
	/* Initialize HCD and host controller data structures. */
	retval = xhci_init(hcd);
	if (retval)
		goto error;
	xhci_dbg(xhci, "Called HCD init\n");
	return 0;
error:
	kfree(xhci);
	return retval;
}


static int xhci_tcc_setup(struct usb_hcd *hcd)
{
	struct xhci_hcd         *xhci;
	int                     retval;
	//printk("\x1b[1;31m%s, line:%d Start!!\x1b[0m\n",__func__,__LINE__);
	retval = xhci_gen_setup(hcd, tcc_xhci_quirks);
	if (retval)
	{
	    return retval;
	}

	xhci = hcd_to_xhci(hcd);
	if (!usb_hcd_is_primary_hcd(hcd))
	    return 0;

	xhci->sbrn = HCD_USB3;
//	xhci->sbrn = HCD_USB2;
	xhci_dbg(xhci, "Got SBRN %u\n", (unsigned int) xhci->sbrn);

	return retval;
}

static const struct hc_driver xhci_tcc_hc_driver = {
	.description =		hcd_name,
	.product_desc =		XHCI_PRODUCT_DESC,
	.hcd_priv_size =	sizeof(struct xhci_hcd *),

	/*
	 * generic hardware linkage
	 */
	.irq =			xhci_irq,
	.flags =		HCD_MEMORY | HCD_USB3 | HCD_SHARED,

	/*
	 * basic lifecycle operations
	 */
	.reset =		xhci_tcc_setup,
	.start =		xhci_run,
#ifdef CONFIG_PM
	//.pci_suspend =          xhci_tcc_suspend,
	//.pci_resume =           xhci_tcc_resume,
#endif
	.stop =			xhci_stop,
	.shutdown =		xhci_shutdown,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue =		xhci_urb_enqueue,
	.urb_dequeue =		xhci_urb_dequeue,
	.alloc_dev =		xhci_alloc_dev,
	.free_dev =		xhci_free_dev,
	.alloc_streams =	xhci_alloc_streams,
	.free_streams =		xhci_free_streams,
	.add_endpoint =		xhci_add_endpoint,
	.drop_endpoint =	xhci_drop_endpoint,
	.endpoint_reset =	xhci_endpoint_reset,
	.check_bandwidth =	xhci_check_bandwidth,
	.reset_bandwidth =	xhci_reset_bandwidth,
	.address_device =	xhci_address_device,
	.update_hub_device =	xhci_update_hub_device,
	.reset_device =		xhci_discover_or_reset_device,

	/*
	 * scheduling support
	 */
	.get_frame_number =	xhci_get_frame,

	/* Root hub support */
	.hub_control =		xhci_hub_control,
	.hub_status_data =	xhci_hub_status_data,
	.bus_suspend =		xhci_bus_suspend,
	.bus_resume =		xhci_bus_resume,
};

static int usb_hcd_tcc_probe(struct platform_device *pdev, const struct hc_driver *driver)
{
    struct usb_hcd          *hcd;
    struct resource *res;
	int irq;
    int err;

    if (usb_disabled())
        return -ENODEV;

    if (!driver)
        return -EINVAL;

    hcd = usb_create_hcd(driver, &pdev->dev, dev_name(&pdev->dev));
    if (!hcd) {
        err = -ENOMEM;
        goto fail_hcd;
    }

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev,
			"Found SS with no register addr. Check %s setup!\n",
			dev_name(&pdev->dev));
		err = -ENODEV;
		goto fail_io;
	}
	
	hcd->rsrc_start = res->start;
	hcd->rsrc_len = res->end - res->start + 1;
	hcd->regs = (void __iomem *)(int)(hcd->rsrc_start);	
	
	irq = platform_get_irq(pdev, 0);
    if (!irq) {
        dev_err(&pdev->dev, "Failed to get IRQ\n");
        err = -ENODEV;
        goto fail;
    }
	hcd->irq = irq;

    err = usb_add_hcd(hcd, irq, IRQF_SHARED| IRQF_DISABLED/**/);
    if (err) {
        dev_err(&pdev->dev, "Failed to add USB HCD\n");
        goto fail;
    }

	platform_set_drvdata(pdev, hcd);
	
    return err;

fail:
    iounmap(hcd->regs);
fail_io:
	tcc_stop_xhci();
fail_hcd:
    kfree(hcd);
    return err;
}

static int __devinit xhci_tcc_drv_probe(struct platform_device *pdev)
{
    //struct exynos_xhci_hcd *exynos_xhci;
    struct usb_hcd *hcd;
    struct xhci_hcd *xhci;
	//struct hc_driver* driver = &xhci_tcc_hc_driver;
	int irq;
    int err;

	tcc_init_usb3host(USB3_SPEED_MODE);
	
    /* Register the USB 2.0 roothub.
     * FIXME: USB core must know to register the USB 2.0 roothub first.
     * This is sort of silly, because we could just set the HCD driver flags
     * to say USB 2.0, but I'm not sure what the implications would be in
     * the other parts of the HCD code.
     */
    err = usb_hcd_tcc_probe(pdev, &xhci_tcc_hc_driver);
    if (err)
		return err;

    hcd = dev_get_drvdata(&pdev->dev);
    xhci = hcd_to_xhci(hcd);
    xhci->shared_hcd = usb_create_shared_hcd(&xhci_tcc_hc_driver, &pdev->dev, platform_name(pdev), hcd);
	
    if (!xhci->shared_hcd) {
        dev_err(&pdev->dev, "Unable to create HCD\n");
        err = -ENOMEM;
        goto dealloc_usb2_hcd;
    }

    /* Set the xHCI pointer before exynos_xhci_setup() (aka hcd_driver.reset)
     * is called by usb_add_hcd().
     */
    *((struct xhci_hcd **) xhci->shared_hcd->hcd_priv) = xhci;
	
	irq = platform_get_irq(pdev, 0);
	
    err = usb_add_hcd(xhci->shared_hcd, irq,/* */IRQF_DISABLED |IRQF_SHARED);
    if (err)
      	goto put_usb3_hcd;

    /* Roothub already marked as USB 3.0 speed */
    return err;

put_usb3_hcd:
    usb_put_hcd(xhci->shared_hcd);
dealloc_usb2_hcd:
    usb_remove_hcd(hcd);
    return err;
}

static void xhci_tcc_remove(struct platform_device *pdev)
{
	struct xhci_hcd *xhci;
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	xhci = hcd_to_xhci(hcd);
	if (xhci->shared_hcd) {
		usb_remove_hcd(xhci->shared_hcd);
		usb_put_hcd(xhci->shared_hcd);
	}

	usb_remove_hcd(hcd);
	usb_put_hcd(hcd);

	local_irq_disable();
	usb_hcd_irq(0, hcd);
	local_irq_enable();
	
	tcc_xhci_phy_off();
	tcc_xhci_phy_ctrl(0);
	tcc_stop_xhci();
	tcc_xhci_vbus_ctrl(0);
	
	kfree(xhci);
}

#ifdef CONFIG_PM
static int xhci_tcc_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct usb_hcd          *hcd;
	struct xhci_hcd         *xhci;
    int                     retval = 0;
		
	hcd = dev_get_drvdata(&pdev->dev);
    if (!hcd)
            return -EINVAL;

    xhci = hcd_to_xhci(hcd);

    if (hcd->state != HC_STATE_SUSPENDED || xhci->shared_hcd->state != HC_STATE_SUSPENDED){
            return -EINVAL;
	}

    retval = xhci_suspend(xhci);

	/* Telechips specific routine */
	tcc_xhci_phy_off();
	tcc_stop_xhci();
   tcc_xhci_vbus_ctrl(0);
   //tcc_usb30_vbus_exit();
#if !defined(CONFIG_TCC_USB_DRD)
   tcc_usb30_vbus_exit();
#endif
    return retval;
}

static int xhci_tcc_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct usb_hcd          *hcd;
	struct xhci_hcd         *xhci;
	int			retval = 0;

	hcd = dev_get_drvdata(&pdev->dev);
	if (!hcd)
	    return -EINVAL;

	tcc_init_usb3host(USB3_SPEED_MODE);

	xhci = hcd_to_xhci(hcd);
	retval = xhci_resume(xhci, 0);

	return retval;
}
static const struct dev_pm_ops xhci_tcc_pmops = {
    .suspend	= xhci_tcc_suspend,
    .resume		= xhci_tcc_resume,
};

#define XHCI_TCC_PMOPS &xhci_tcc_pmops
#endif /* CONFIG_PM */

/*-------------------------------------------------------------------------*/

static struct platform_driver xhci_tcc_driver = {
	.probe		= xhci_tcc_drv_probe,
	.remove		= __exit_p(xhci_tcc_remove),
	.shutdown	= usb_hcd_platform_shutdown,
	.driver = {
		.name	= (char *) "tcc-xhci",
		.owner	= THIS_MODULE,
		.pm		= XHCI_TCC_PMOPS,
	}
};

