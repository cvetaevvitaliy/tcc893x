/* ==========================================================================
 * $File: //dwh/usb_iip/dev/software/DWC_usb3/driver/pcd_hiber.c $
 * $Revision: #5 $
 * $Date: 2012/01/23 $
 * $Change: 1924659 $
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

/** @file
 * This file contains the PCD hibernation code.
 */

#include "os_dep.h"
#include "hw.h"
#include "usb.h"
#include "pcd.h"
#include "driver.h"
#include "cil.h"

/**
 * This function sends the core into hibernation, saving the core's runtime
 * state if requested.
 */
void dwc_enter_hibernation(dwc_usb3_pcd_t *pcd, int save_state)
{
	dwc_usb3_device_t *dev = pcd->usb3_dev;
	dwc_usb3_dev_ep_regs_t __iomem *ep_reg;
	dwc_usb3_pcd_ep_t *ep;
	int num_in_eps, num_out_eps, i;
	uint32_t temp;
	int cnt;

	dwc_debug2(dev, "%s(%d)\n", __func__, save_state);
	num_in_eps = pcd->num_in_eps;
	num_out_eps = pcd->num_out_eps;

	dev->hiber_wait_u0 = 0;
	atomic_set(&dev->hibernate, DWC_HIBER_SLEEPING);

	/* Issue an "End Transfer" command for all active transfers with the
	 * ForceRM field set to 0, including the default control endpoint 0.
	 */
	ep = pcd->ep0;
	if (ep->dwc_ep.active) {
		if (ep->dwc_ep.tri_in) {
			dwc_debug0(dev, "EndXfer on phys EP1 (IN)\n");
			ep_reg = ep->dwc_ep.in_ep_reg;
			ep->dwc_ep.condition = 0;
			dwc_usb3_dep_endxfer(pcd, ep_reg, ep->dwc_ep.tri_in, 0,
					     &ep->dwc_ep.condition);
			ep->dwc_ep.tri_in = 0;
		}

		dwc_debug0(dev, "EndXfer on phys EP0 (OUT)\n");
		ep_reg = ep->dwc_ep.out_ep_reg;
		ep->dwc_ep.condition = 0;
		dwc_usb3_dep_endxfer(pcd, ep_reg, ep->dwc_ep.tri_out, 0,
				     &ep->dwc_ep.condition);
		ep->dwc_ep.tri_out = 0;
	}

	for (i = 0; i < num_in_eps; i++) {
		ep = pcd->in_ep[i];
		if (ep->dwc_ep.active && ep->dwc_ep.tri_in) {
			dwc_debug1(dev, "EndXfer on phys EP%d (IN)\n",
				   i * 2 + 3);
			ep_reg = ep->dwc_ep.in_ep_reg;
			ep->dwc_ep.condition = 0;
			dwc_usb3_dep_endxfer(pcd, ep_reg, ep->dwc_ep.tri_in, 0,
					     &ep->dwc_ep.condition);
			ep->dwc_ep.tri_in = 0;
		}
		ep->dwc_ep.xfer_started = 0;
	}

	for (i = 0; i < num_out_eps; i++) {
		ep = pcd->out_ep[i];
		if (ep->dwc_ep.active && ep->dwc_ep.tri_out) {
			dwc_debug1(dev, "EndXfer on phys EP%d (OUT)\n",
				   i * 2 + 2);
			ep_reg = ep->dwc_ep.out_ep_reg;
			ep->dwc_ep.condition = 0;
			dwc_usb3_dep_endxfer(pcd, ep_reg, ep->dwc_ep.tri_out, 0,
					     &ep->dwc_ep.condition);
			ep->dwc_ep.tri_out = 0;
		}
		ep->dwc_ep.xfer_started = 0;
	}

	if (save_state) {
		dwc_debug0(dev, "Saving state - 1\n");

		/* Issue a "Get Endpoint State" endpoint command for each active
		 * endpoint, and save the bits that are returned for use after
		 * coming out of hibernation.
		 *
		 * In addition, software must remember if the endpoint is
		 * currently in a Halted state. The endpoint is in a Halted
		 * state if software has issued a "Set STALL" command and has
		 * not issued a "Clear STALL" command.
		 */
		ep = pcd->ep0;
		if (ep->dwc_ep.active) {
			dwc_debug0(dev, "Saving state of phys EP1 (IN)\n");
			ep_reg = ep->dwc_ep.in_ep_reg;
			pcd->ep0_in_save_state =
				dwc_usb3_dep_getepstate(pcd, ep_reg);
			dwc_debug1(dev, "IN save state=%08x\n",
				   pcd->ep0_in_save_state);
			dwc_debug0(dev, "Saving state of phys EP0 (OUT)\n");
			ep_reg = ep->dwc_ep.out_ep_reg;
			pcd->ep0_out_save_state =
				dwc_usb3_dep_getepstate(pcd, ep_reg);
			dwc_debug1(dev, "OUT save state=%08x\n",
				   pcd->ep0_out_save_state);
			ep->dwc_ep.stalled_save = ep->dwc_ep.stopped;
		}

		for (i = 0; i < num_in_eps; i++) {
			ep = pcd->in_ep[i];
			if (ep->dwc_ep.active) {
				dwc_debug1(dev,
					   "Saving state of phys EP%d (IN)\n",
					   i * 2 + 3);
				ep_reg = ep->dwc_ep.in_ep_reg;
				ep->dwc_ep.save_state =
					dwc_usb3_dep_getepstate(pcd, ep_reg);
				ep->dwc_ep.stalled_save = ep->dwc_ep.stopped;
			}
		}

		for (i = 0; i < num_out_eps; i++) {
			ep = pcd->out_ep[i];
			if (ep->dwc_ep.active) {
				dwc_debug1(dev,
					   "Saving state of phys EP%d (OUT)\n",
					   i * 2 + 2);
				ep_reg = ep->dwc_ep.out_ep_reg;
				ep->dwc_ep.save_state =
					dwc_usb3_dep_getepstate(pcd, ep_reg);
				ep->dwc_ep.stalled_save = ep->dwc_ep.stopped;
			}
		}
	}

	/* Set DCTL.RunStop to 0, DCTL.KeepConnect to 1 (or 0 if disconnected),
	 * and wait for DSTS.Halted to be set to 1. Software must service any
	 * events that are generated while it is waiting for Halted to be set
	 * to 1.
	 */
	temp = dwc_rd32(dev, &pcd->dev_global_regs->dctl);
	temp &= ~DWC_DCTL_RUN_STOP_BIT;
	if (!save_state)
		temp &= ~DWC_DCTL_KEEP_CONNECT_BIT;
	dwc_wr32(dev, &pcd->dev_global_regs->dctl, temp);

	dwc_debug0(dev, "Cleared Run/Stop bit, waiting for Halt bit\n");
	do {
		if (!dwc_usb3_handle_event(dev))
			dwc_udelay(dev, 1);
		temp = dwc_rd32(dev, &pcd->dev_global_regs->dsts);
	} while (!(temp & DWC_DSTS_DEV_CTRL_HLT_BIT));

	if (save_state) {
		dwc_debug0(dev, "Saving state - 2\n");

		/* Read the D* registers (DCTL, DCFG, DEVTEN) and G* registers
		 * (GSBUSCFG0/1, GCTL, GTXTHRCFG, GRXTHRCFG, GTXFIFOSIZn,
		 * GRXFIFOSIZ0, GUSB3PIPECTL0, GUSB2PHYCFG0) and save their
		 * state.
		 */
		dev->dcfg_save = dwc_rd32(dev, &pcd->dev_global_regs->dcfg);
		dwc_debug(dev, "DCFG=%08x\n", dev->dcfg_save);
		dev->dctl_save = dwc_rd32(dev, &pcd->dev_global_regs->dctl);
		dev->gtxfifosiz0_save =
			dwc_rd32(dev, &dev->core_global_regs->gtxfifosiz[0]);
		dev->gtxfifosiz1_save =
			dwc_rd32(dev, &dev->core_global_regs->gtxfifosiz[1]);
		dev->gtxfifosiz2_save =
			dwc_rd32(dev, &dev->core_global_regs->gtxfifosiz[2]);
		dev->gtxfifosiz3_save =
			dwc_rd32(dev, &dev->core_global_regs->gtxfifosiz[3]);
		dev->grxfifosiz0_save =
			dwc_rd32(dev, &dev->core_global_regs->grxfifosiz[0]);
		dwc_usb3_set_scratchpad_buf_array(pcd,
					pcd->hiber_scratchpad_array_dma);

		/* Set the DCTL.CSS bit and wait for the save state process to
		 * complete by polling for DSTS.SSS to equal 0.
		 */
		temp = dwc_rd32(dev, &pcd->dev_global_regs->dsts);
		dwc_debug1(dev, "DSTS before=%1x\n", temp);
		temp = dwc_rd32(dev, &pcd->dev_global_regs->dctl);
		dwc_debug1(dev, "DCTL before=%1x\n", temp);
		temp |= DWC_DCTL_CSS_BIT;
		dwc_wr32(dev, &pcd->dev_global_regs->dctl, temp);
		temp = dwc_rd32(dev, &pcd->dev_global_regs->dctl);
		dwc_debug1(dev, "DCTL after=%1x\n", temp);

		dwc_debug0(dev, "Set CSS bit, waiting for SSS bit clear\n");
		do {
			dwc_udelay(dev, 1);
			temp = dwc_rd32(dev, &pcd->dev_global_regs->dsts);
		} while (temp & DWC_DSTS_SSS_BIT);
		dwc_debug1(dev, "DSTS after=%1x\n", temp);
#if 0
		/* If the save failed, the DSTS.SRE field will indicate an
		 * error.
		 */
		if (temp & DWC_DSTS_SRE_BIT) {
			dwc_error0(dev, "### Save state failed! ###\n");

			/* What should we do here ?? */
			return;
		}
#endif
	}

	/* Communicate with the power controller to set the power state to D3 */
	temp = dwc_rd32(dev, dev->base + 0x80004);
	dwc_debug1(dev, "R1=%01x before write\n", temp);
	temp |= 0x3000;
	dwc_wr32(dev, dev->base + 0x80004, temp);
	temp = dwc_rd32(dev, dev->base + 0x80004);
	dwc_debug1(dev, "R1=%01x after write\n", temp);

	/* Wait until both PMUs confirm that they have entered D3 */
	dwc_debug0(dev, "Asked for D3 state, waiting for response\n");
	cnt = 0;
	do {
		dwc_udelay(dev, 1);
		temp = dwc_rd32(dev, dev->base + 0x80010);
		if (++cnt > 10000000) {
			cnt = 0;
			dwc_debug1(dev, "%01x\n", temp);
			//break;
		}
	} while ((temp & 0x33) != 0x33);

	dev->pme_ready = 1;
	dwc_info0(dev, "In D3\n");

	/* Remove core well power */
	// This is "faked" by the FPGA
}

static void dwc_reenable_xfer_and_restart(dwc_usb3_pcd_t *pcd,
					  dwc_usb3_pcd_ep_t *ep)
{
	dwc_usb3_dev_ep_regs_t __iomem *ep_reg;
	dwc_usb3_dma_desc_t *desc;
	dma_addr_t desc_dma;
	uint8_t *tri;
	int i, owned;

	dwc_debug1(pcd->usb3_dev, "%s()\n", __func__);
	dwc_debug2(pcd->usb3_dev, "phys EP%d-%s\n", ep->dwc_ep.phys,
		   ep->dwc_ep.is_in ? "IN" : "OUT");
	desc = (dwc_usb3_dma_desc_t *)ep->dwc_ep.dma_desc;

	/* Find the first non-hw-owned TRB */
	for (i = 0; i < ep->dwc_ep.num_desc; i++, desc++) {
		if (!dwc_usb3_is_hwo(desc) &&
		    !(dwc_usb3_get_xfersts(desc) & DWC_TRBRSP_XFER_IN_PROG)) {
			dwc_debug1(pcd->usb3_dev,
				   "Found non-hw-owned TRB at %d\n", i);
			break;
		}
	}

	if (i == ep->dwc_ep.num_desc)
		desc = (dwc_usb3_dma_desc_t *)ep->dwc_ep.dma_desc;

	/* Find the first hw-owned TRB */
	for (i = 0, owned = -1; i < ep->dwc_ep.num_desc; i++) {
		/* If status == 4, this TRB's xfer was in progress prior to
		 * entering hibernation
		 */
		if (dwc_usb3_get_xfersts(desc) & DWC_TRBRSP_XFER_IN_PROG) {
			dwc_debug1(pcd->usb3_dev,
				   "Found in-progress TRB at %d\n", i);

			/* Set HWO back to 1 so the xfer can continue */
			desc->control |= DWC_DSCCTL_HWO_BIT;
			owned = i;
			break;
		}

		/* Save the index of the first TRB with the HWO bit set */
		if (dwc_usb3_is_hwo(desc)) {
			dwc_debug1(pcd->usb3_dev, "Found hw-owned TRB at %d\n",
				   i);

			owned = i;
			break;
		}

		desc++;
		if (desc == (dwc_usb3_dma_desc_t *)ep->dwc_ep.dma_desc +
							ep->dwc_ep.num_desc)
			desc = (dwc_usb3_dma_desc_t *)ep->dwc_ep.dma_desc;
	}

	wmb();
	ep->dwc_ep.hiber_desc_idx = 0;

	if (owned < 0)
		/* No TRB had HWO bit set, fine */
		return;

	dwc_debug1(pcd->usb3_dev, "desc=%08lx\n", (unsigned long)desc);

	/* Restart of Isoc EPs is deferred until the host polls the EP */
	if (ep->dwc_ep.type == UE_ISOCHRONOUS) {
		dwc_debug0(pcd->usb3_dev,
			   "Deferring restart until host polls\n");
		ep->dwc_ep.hiber_desc_idx = owned + 1;
		return;
	}

#ifdef VERBOSE
	dwc_debug4(pcd->usb3_dev, "%08x %08x %08x %08x\n",
		   *((unsigned *)desc), *((unsigned *)desc + 1),
		   *((unsigned *)desc + 2), *((unsigned *)desc + 3));
#endif
	/* Now restart at the first TRB found with HWO set */
	if (ep->dwc_ep.is_in) {
		ep_reg = ep->dwc_ep.in_ep_reg;
		tri = &ep->dwc_ep.tri_in;
	} else {
		ep_reg = ep->dwc_ep.out_ep_reg;
		tri = &ep->dwc_ep.tri_out;
	}

	dwc_debug0(pcd->usb3_dev, "Restarting xfer\n");
	desc = (dwc_usb3_dma_desc_t *)(ep->dwc_ep.dma_desc + owned * 16);
	desc_dma = (dma_addr_t)
		((unsigned long)ep->dwc_ep.dma_desc_dma + owned * 16);
	dwc_debug1(pcd->usb3_dev, "desc=%08lx\n", (unsigned long)desc);

#ifdef VERBOSE
	dwc_debug5(pcd->usb3_dev, "%08x %08x %08x %08x (%08x)\n",
		   *((unsigned *)desc), *((unsigned *)desc + 1),
		   *((unsigned *)desc + 2), *((unsigned *)desc + 3),
		   (unsigned)desc_dma);
#endif
	*tri = dwc_usb3_dep_startxfer(pcd, ep_reg, desc_dma, 0);
}

/**
 * This function finishes exiting from hibernation once the device is connected.
 */
void dwc_exit_hibernation_after_connect(dwc_usb3_pcd_t *pcd, int connected)
{
	dwc_usb3_pcd_ep_t *ep;
	int num_in_eps, num_out_eps, i;
	uint32_t temp;

	dwc_debug2(pcd->usb3_dev, "%s(%d)\n", __func__, connected);

	/* Perform the steps in Section 9.1.5 "Initialization on
	 * SetConfiguration or SetInterface Command" in [DWSS].
	 *
	 * While issuing the DEPCFG commands, set each endpoint's sequence
	 * number and flow control state to the value read during the save.
	 *
	 * If the endpoint was in the Halted state prior to entering
	 * hibernation, software must issue the "Set STALL" endpoint command
	 * to put the endpoint back into the Halted state.
	 */
	num_in_eps = pcd->num_in_eps;
	num_out_eps = pcd->num_out_eps;

	pcd->eps_enabled = 0;

	ep = pcd->ep0;
	if (ep->dwc_ep.active)
		ep->dwc_ep.stalled_save = 0;

	for (i = 0; i < num_in_eps; i++) {
		ep = pcd->in_ep[i];
		if (ep->dwc_ep.active) {
			ep->dwc_ep.ena_once = 0;
			dwc_debug1(pcd->usb3_dev, "Activating phys EP%d (IN)\n",
				   i * 2 + 3);
			dwc_usb3_ep_activate(pcd, ep, 1);
			if (ep->dwc_ep.stalled_save) {
				dwc_debug1(pcd->usb3_dev,
					   "Stalling phys EP%d (IN)\n",
					   i * 2 + 3);
				dwc_usb3_ep_set_stall(pcd, ep);
				ep->dwc_ep.stopped = 1;
			}
		}
	}

	for (i = 0; i < num_out_eps; i++) {
		ep = pcd->out_ep[i];
		if (ep->dwc_ep.active) {
			ep->dwc_ep.ena_once = 0;
			dwc_debug1(pcd->usb3_dev,
				   "Activating phys EP%d (OUT)\n", i * 2 + 2);
			dwc_usb3_ep_activate(pcd, ep, 1);
			if (ep->dwc_ep.stalled_save) {
				dwc_debug1(pcd->usb3_dev,
					   "Stalling phys EP%d (OUT)\n",
					   i * 2 + 2);
				dwc_usb3_ep_set_stall(pcd, ep);
				ep->dwc_ep.stopped = 1;
			}
		}
	}

	/* (In this step, software re-configures the existing endpoints and
	 * starts their transfers).
	 *
	 * When software issued the EndXfer command with the ForceRM field set
	 * to '0' prior to entering hibernation, the core may have written back
	 * an active TRB for the transfer, setting the HWO bit to '0'. Software
	 * must ensure that the TRB is valid and set the HWO bit back to '1'
	 * prior to re-starting the transfer in this step.
	 */
	for (i = 0; i < num_in_eps; i++) {
		ep = pcd->in_ep[i];
		if (ep->dwc_ep.active) {
			dwc_debug1(pcd->usb3_dev,
				   "Restarting xfer on phys EP%d (IN)\n",
				   i * 2 + 3);
			dwc_reenable_xfer_and_restart(pcd, ep);
		}
	}

	for (i = 0; i < num_out_eps; i++) {
		ep = pcd->out_ep[i];
		if (ep->dwc_ep.active) {
			dwc_debug1(pcd->usb3_dev,
				   "Restarting xfer on phys EP%d (OUT)\n",
				   i * 2 + 2);
			dwc_reenable_xfer_and_restart(pcd, ep);
		}
	}

	/* If the core is already connected to the host (DSTS.USBLnkSt == 0, 2,
	 * 3, 14, or 15), set DCTL.ULStChngReq to '8' as described in Table 7-47
	 * of [DWSS].
	 *
	 * If the host initiated resume, this step completes the transition to
	 * U0. If the host did not initiate resume, this step causes the device
	 * to initiate resume (remote wakeup).
	 */
	if (connected) {
		dwc_debug0(pcd->usb3_dev, "Already connected,"
			   " requesting link state Recovery\n");
		dwc_usb3_set_link_state(pcd, DWC_LINK_STATE_REQ_RECOVERY);
		//dwc_udelay(pcd->usb3_dev, 1);
		//dwc_usb3_set_link_state(pcd, 0);
		pcd->usb3_dev->hiber_wait_u0 = 1;
		temp = dwc_usb3_get_link_state(pcd);
		dwc_debug1(pcd->usb3_dev, "Link state %d\n", temp);
	} else {
		pcd->usb3_dev->hiber_wait_u0 = 0;
	}
}

/**
 * This function wakes the core from hibernation.
 */
int dwc_exit_hibernation(dwc_usb3_pcd_t *pcd, int restore_state)
{
	dwc_usb3_device_t *dev = pcd->usb3_dev;
	uint32_t temp;
	int cnt;

	dwc_debug2(dev, "%s(%d)\n", __func__, restore_state);

	/* Enable core well power */
	// This is "faked" by the FPGA

	/* Communicate with the power controller to set the power state to D0 */
	dev->pme_ready = 0;
	temp = dwc_rd32(dev, dev->base + 0x80004);
	dwc_debug1(dev, "R1=%01x before write\n", temp);
	temp &= ~0x3000;
	dwc_wr32(dev, dev->base + 0x80004, temp);
	temp = dwc_rd32(dev, dev->base + 0x80004);
	dwc_debug1(dev, "R1=%01x after write\n", temp);

	/* Wait until both PMUs confirm that they have entered D0 */
	dwc_debug0(dev, "Asked for D0 state, waiting for response\n");
	cnt = 0;
	do {
		dwc_udelay(dev, 1);
		temp = dwc_rd32(dev, dev->base + 0x80010);
		if (++cnt > 10000000) {
			cnt = 0;
			dwc_debug1(dev, "%01x\n", temp);
		}
	} while ((temp & 0x33) != 0);

	dwc_info0(dev, "In D0\n");
	atomic_set(&dev->hibernate, DWC_HIBER_AWAKE);

	if (restore_state) {
		dwc_debug0(dev, "Restoring state\n");
#if 0
		/* If the reset value of GSBUSCFG0/1 is not correct, write this
		 * register with the desired value.
		 */
		dwc_wr32(dev, &dev->core_global_regs->gsbuscfg0,
			 dev->gsbuscfg0_save);
		dwc_wr32(dev, &dev->core_global_regs->gsbuscfg1,
			 dev->gsbuscfg1_save);
#endif
		/* Issue a "Set Scratchpad Buffer Array" device generic command
		 * and wait for completion by polling the DGCMD.CmdAct bit.
		 */
		dwc_usb3_set_scratchpad_buf_array(pcd,
					pcd->hiber_scratchpad_array_dma);

		/* Write '1' to DCTL.CRS to start the restore process and wait
		 * for completion by polling the DSTS.RSS bit.
		 */
		temp = dwc_rd32(dev, &pcd->dev_global_regs->dsts);
		dwc_debug1(dev, "DSTS before=%1x\n", temp);
		temp = dwc_rd32(dev, &pcd->dev_global_regs->dctl);
		dwc_debug1(dev, "DCTL before=%1x\n", temp);
		temp |= DWC_DCTL_CRS_BIT;
		dwc_wr32(dev, &pcd->dev_global_regs->dctl, temp);
		temp = dwc_rd32(dev, &pcd->dev_global_regs->dctl);
		dwc_debug1(dev, "DCTL after=%1x\n", temp);

		dwc_debug0(dev, "Set CRS bit, waiting for RSS bit clear\n");
		do {
			dwc_udelay(dev, 1);
			temp = dwc_rd32(dev, &pcd->dev_global_regs->dsts);
		} while (temp & DWC_DSTS_RSS_BIT);
		dwc_debug1(dev, "DSTS after=%1x\n", temp);
#if 0
		/* If the restore failed, the DSTS.SRE field will indicate an
		 * error.
		 */
		if (temp & DWC_DSTS_SRE_BIT) {
			dwc_error0(dev, "### Restore state failed! ###\n");

			/* What should we do here ?? */
			return 1;
		}
#endif
		/* (Restore the remaining D* and G* registers) */
		dwc_wr32(dev, &pcd->dev_global_regs->dcfg, dev->dcfg_save);
		dwc_debug1(dev, "DCFG=%08x\n", dev->dcfg_save);
		temp = dwc_rd32(dev, &pcd->dev_global_regs->dcfg);
		dwc_debug1(dev, "DCFG read back %08x\n", temp);
		dwc_wr32(dev, &pcd->dev_global_regs->dctl, dev->dctl_save);
		dwc_wr32(dev, &dev->core_global_regs->gtxfifosiz[0],
			 dev->gtxfifosiz0_save);
		dwc_wr32(dev, &dev->core_global_regs->gtxfifosiz[1],
			 dev->gtxfifosiz1_save);
		dwc_wr32(dev, &dev->core_global_regs->gtxfifosiz[2],
			 dev->gtxfifosiz2_save);
		dwc_wr32(dev, &dev->core_global_regs->gtxfifosiz[3],
			 dev->gtxfifosiz3_save);
		dwc_wr32(dev, &dev->core_global_regs->grxfifosiz[0],
			 dev->grxfifosiz0_save);
	}

	/* Configure the core as described in [DWSS] Section 9.1.1 "Device
	 * Power-On or Soft Reset," excluding the first step (Soft Reset).
	 */
	dwc_usb3_core_device_init(dev, 0, restore_state);

#ifdef CONFIG_USB_OTG_DWC
	/* Enable Device mode interrupts */
	dwc_usb3_enable_device_interrupts(pcd->usb3_dev);

	/* Set Run/Stop bit, and Keep-Connect bit if hibernation enabled */
	temp = dwc_rd32(pcd->usb3_dev, &pcd->dev_global_regs->dctl);
	temp |= DWC_DCTL_RUN_STOP_BIT;
	if (pcd->usb3_dev->core_params->hibernate &&
	    (pcd->usb3_dev->hwparams1 & DWC_HWP1_EN_PWROPT_BITS) ==
	    (DWC_EN_PWROPT_HIBERNATION << DWC_HWP1_EN_PWROPT_SHIFT))
		temp |= DWC_DCTL_KEEP_CONNECT_BIT;
	dwc_wr32(pcd->usb3_dev, &pcd->dev_global_regs->dctl, temp);
#endif

	/* Read the DSTS register to see the current link state. */
	temp = dwc_usb3_get_link_state(pcd);

	/* If the core is not connected to the host, wait for a Connect Done
	 * event.
	 */
	if (temp > DWC_LINK_STATE_U3 && temp != DWC_LINK_STATE_EARLY_SUSPEND &&
	    temp != DWC_LINK_STATE_RESUME) {
		if (temp != DWC_LINK_STATE_RESET) {
			dev->hiber_wait_connect = 1;

			if (temp == DWC_LINK_STATE_SS_DIS &&
			    pcd->speed != USB_SPEED_SUPER &&
			    dev->core_params->ssdisquirk) {
				atomic_set(&dev->hibernate,
					   DWC_HIBER_SS_DIS_QUIRK);
				return DWC_HIBER_SS_DIS_QUIRK;
			}

			dwc_debug1(dev, "Link state %d, waiting for connect"
				   " before exiting from hibernation\n", temp);
		}

		/* If the DSTS.USBLnkSt equals 14, it means a USB reset was
		 * received while the core was entering or exiting hibernation.
		 * Prior to performing the steps in sections 9.1.2 and 9.1.3,
		 * software must write Resume (8) into the DCTL.ULStChngReq
		 * field.
		 */
		if (temp == DWC_LINK_STATE_RESET) {
			dwc_debug0(dev, "USB Reset received during hibernation,"
				   " requesting link state Recovery\n");
			dwc_usb3_set_link_state(pcd,
						DWC_LINK_STATE_REQ_RECOVERY);
			//dwc_mdelay(pcd->usb3_dev, 1);
			//dwc_usb3_set_link_state(pcd, 0);
			atomic_set(&dev->hibernate, DWC_HIBER_WAIT_LINK_UP);
			return DWC_HIBER_WAIT_LINK_UP;
		}
	} else {
		dwc_debug1(dev, "Link state %d, exiting from hibernation\n",
			   temp);
		dwc_debug0(dev, "Exiting from hibernation\n");
		dev->hiber_wait_connect = 0;

		/* Perform the steps in Section 9.1.3 "Initialization on
		 * Connect Done" in [DWSS].
		 */
		dwc_usb3_handle_connect_done_intr(pcd);

		dwc_exit_hibernation_after_connect(pcd, 1);
	}

	return 0;
}

static void dwc_wait_for_link_up(dwc_usb3_pcd_t *pcd)
{
	unsigned long flags;
	uint32_t temp;
	int count = 10;

	while (count-- > 0) {
		msleep(10);
		spin_lock_irqsave(&pcd->lock, flags);
		temp = dwc_usb3_get_link_state(pcd);
		if (temp == 0) {
			dwc_debug0(pcd->usb3_dev,
				   "Exiting from hibernation 2\n");
			pcd->usb3_dev->hiber_wait_connect = 0;

			/* Perform the steps in Section 9.1.3 "Initialization on
			 * Connect Done" in [DWSS].
			 */
			dwc_usb3_handle_connect_done_intr(pcd);
			dwc_exit_hibernation_after_connect(pcd, 0);
			count = 0;
		}

		spin_unlock_irqrestore(&pcd->lock, flags);
	}
}

/**
 * Kernel thread that monitors the dev->hibernate variable and the 'debug'
 * register in the FPGA. dev->hibernate is set to 1 or 2 from the interrupt
 * handler when the core is requesting to enter hibernation. This thread checks
 * whether it is safe to do so (dev->hiber_cnt == 0) and then sets
 * dev->hibernate to 3 and puts the core into hibernation. dev->hibernate >= 3
 * tells the rest of the driver that it cannot access the hardware.
 *
 * When the core is in hibernation, this thread periodically reads the 'debug'
 * register to see if the core is requesting to exit hibernation. If so, it
 * brings the core out of hibernation.
 *
 * Note that this is NOT the way a "real" device would enter and exit
 * hibernation. This code is ONLY for testing hibernation on the Synopsys
 * HAPS platform.
 */
int dwc_wait_pme_thread(void *data)
{
	dwc_usb3_pcd_t *pcd = (dwc_usb3_pcd_t *)data;
	dwc_usb3_device_t *dev = pcd->usb3_dev;
	unsigned long flags;
	uint32_t temp;

	dwc_debug2(dev, "%s(%p)\n", __func__, data);

	/* Allow the thread to be killed by a signal, but set the signal mask
	 * to block everything but INT, TERM, KILL, and USR1. */
	allow_signal(SIGINT);
	allow_signal(SIGTERM);
	allow_signal(SIGKILL);
	allow_signal(SIGUSR1);

	set_user_nice(current, -10);

	/* Allow the thread to be frozen */
	set_freezable();

	for (;;) {
		spin_lock_irqsave(&pcd->lock, flags);
		temp = atomic_read(&dev->hibernate);

		if (atomic_read(&dev->hiber_cnt) == 0) {
			if (temp == DWC_HIBER_ENTER_NOSAVE ||
			    temp == DWC_HIBER_ENTER_SAVE) {
				dev->hiber_state =
					(temp == DWC_HIBER_ENTER_SAVE);
				dwc_enter_hibernation(pcd, dev->hiber_state);
				temp = DWC_HIBER_SLEEPING;
			}
		}

		if (temp == DWC_HIBER_WAIT_LINK_UP) {
			atomic_set(&dev->hibernate, DWC_HIBER_AWAKE);
			spin_unlock_irqrestore(&pcd->lock, flags);
			dwc_wait_for_link_up(pcd);
			spin_lock_irqsave(&pcd->lock, flags);
		}

		if (temp == DWC_HIBER_SS_DIS_QUIRK) {
			int i;

			spin_unlock_irqrestore(&pcd->lock, flags);

			for (i = 0; i < 500; i++) {
				if (atomic_read(&dev->hibernate) !=
				    DWC_HIBER_SS_DIS_QUIRK) {
					dwc_info1(dev,
						"breaking loop after %d ms\n",
						i);
					break;
				}

				msleep(1);
			}

			if (atomic_read(&dev->hibernate) ==
						DWC_HIBER_SS_DIS_QUIRK) {
				if (dwc_usb3_get_link_state(pcd) ==
							DWC_LINK_STATE_SS_DIS) {
					spin_lock_irqsave(&pcd->lock, flags);
					dwc_enter_hibernation(pcd, 0);
					spin_unlock_irqrestore(&pcd->lock,
								flags);
				} else {
					atomic_set(&dev->hibernate,
							DWC_HIBER_AWAKE);
				}
			}

			spin_lock_irqsave(&pcd->lock, flags);
		}

		spin_unlock_irqrestore(&pcd->lock, flags);
		if (kthread_should_stop())
			break;
		msleep(1);
	}

	return 0;
}

int dwc_usb3_handle_pme_intr(dwc_usb3_device_t *dev)
{
	uint32_t temp;
	int ret;

	dwc_info1(dev, "%s()\n", __func__);

	temp = dwc_rd32(dev, dev->base + 0x80010);
	if ((temp & 0x44) != 0) {
		dwc_debug1(dev, "calling dwc_exit_hibernation() Debug=%01x\n",
			   temp);
		ret = dwc_exit_hibernation(&dev->pcd, (temp & 0x88) != 0);
		if (ret)
			dwc_debug1(dev, "dwc_exit_hibernation() returned %d\n",
				   ret);
	}

	return 1;
}
