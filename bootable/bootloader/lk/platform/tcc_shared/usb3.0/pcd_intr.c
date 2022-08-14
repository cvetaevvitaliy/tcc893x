/* ==========================================================================
 * $File: //dwh/usb_iip/dev/software/DWC_usb3/driver/pcd_intr.c $
 * $Revision: #101 $
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
 * This file contains the implementation of the PCD Interrupt handlers.
 *
 * The PCD handles the device interrupts. Many conditions can cause a
 * device interrupt. When an interrupt occurs, the device interrupt
 * service routine determines the cause of the interrupt and
 * dispatches handling to the appropriate function. These interrupt
 * handling functions are described below.
 */

#include "usb3.0/os_dep.h"
#include "usb3.0/hw.h"
#include "usb3.0/usb.h"
#include "usb3.0/pcd.h"
#include "usb3.0/driver.h"
#include "usb3.0/cil.h"
#include "usb/usb_device.h"

#define pcd_intr_dprintf(f, a...)	dprintf(2,"[%s:%d] " f, __func__, __LINE__, ##a)

#ifdef DWC_UTE
# include "ute_if.h"
#endif

/**
 * This interrupt indicates that the USB link state has changed to L2, U3, or
 * (if L1 Hibernation is enabled) L1, and software intervention is required.
 */
static int handle_hiber_req_intr(dwc_usb3_pcd_t *pcd, unsigned int event)
{
#if 0		/* 012.08.13 */
	int hird, is_superspeed;
	unsigned int state;

	dwc_print0(pcd->usb3_dev, "HIBERNATION REQUEST\n");

	is_superspeed = !!(event & DWC_DEVT_HIBER_SS_BIT);
	state = (event >> DWC_DEVT_HIBER_STATE_SHIFT) &
		(DWC_DEVT_HIBER_STATE_BITS >> DWC_DEVT_HIBER_STATE_SHIFT);
	dwc_print2(pcd->usb3_dev, "state=%x, is_superspeed=%d\n",
		   state, is_superspeed);

	/* TODO: Workaround */
	if (!(state == DWC_LINK_STATE_SUSPEND ||
	      //state == DWC_LINK_STATE_SS_DIS ||
	      (!is_superspeed && (state == DWC_LINK_STATE_SLEEP)))) {
		dwc_print0(pcd->usb3_dev, "HIBERNATION not handled\n");
		return 1;
	} /* End workaround */

	hird = (event >> DWC_DEVT_HIBER_HIRD_SHIFT) &
	       (DWC_DEVT_HIBER_HIRD_BITS >> DWC_DEVT_HIBER_HIRD_SHIFT);
	dwc_debug5(pcd->usb3_dev, "%s(%u), state=%d hird=%d ss=%d\n",
		   __func__, event, state, hird, is_superspeed);

	/* Enter hibernation if supported */
	if (pcd->usb3_dev->core_params->hibernate &&
	    (pcd->usb3_dev->hwparams1 & DWC_HWP1_EN_PWROPT_BITS) ==
	    (DWC_EN_PWROPT_HIBERNATION << DWC_HWP1_EN_PWROPT_SHIFT)) {
		/* Tell kernel thread to save state and enter hibernation */
		atomic_set(&pcd->usb3_dev->hibernate, DWC_HIBER_ENTER_SAVE);
		return 1;
	}
#endif /* 0 */

	return 0;
}

/**
 * This interrupt indicates that the device has been disconnected.
 */
static int handle_disconnect_intr(dwc_usb3_pcd_t *pcd)
{
	dwc_print0(pcd->usb3_dev, "DISCONNECT\n");

	dwc_usb3_clr_eps_enabled(pcd);
	dwc_usb3_pcd_stop(pcd);
	pcd->state = DWC_STATE_UNCONNECTED;

	/* Enter hibernation if supported */
#if 0		/* 012.08.13 */
	if (pcd->usb3_dev->core_params->hibernate &&
	    pcd->usb3_dev->core_params->hiberdisc &&
	    (pcd->usb3_dev->hwparams1 & DWC_HWP1_EN_PWROPT_BITS) ==
	    (DWC_EN_PWROPT_HIBERNATION << DWC_HWP1_EN_PWROPT_SHIFT)) {
		/* Tell kernel thread to enter hibernation */
		atomic_set(&pcd->usb3_dev->hibernate, DWC_HIBER_ENTER_NOSAVE);
		return 1;
	}
#endif /* 0 */

	return 0;
}

/**
 * This interrupt occurs when a USB Reset is detected. When the USB Reset
 * Interrupt occurs, all transfers are stopped and the device state is set
 * to DEFAULT.
 */
static void handle_usb_reset_intr(dwc_usb3_pcd_t *pcd)
{
	dwc_usb3_pcd_ep_t *ep;
	int i;

	dwc_print0(pcd->usb3_dev, "USB RESET\n");

	/* If UsbReset comes without Disconnect first, fake it, because the
	 * gadget may need to see a disconnect first. The Synopsys UAS gadget
	 * needs this.
	 */
	if (pcd->state != DWC_STATE_UNCONNECTED) {
		int hibersave = pcd->usb3_dev->core_params->hibernate;

		pcd->usb3_dev->core_params->hibernate = 0;
		handle_disconnect_intr(pcd);
		pcd->usb3_dev->core_params->hibernate = hibersave;
	} else {
		/* Stop any active xfers on the non-EP0 endpoints */
		dwc_usb3_stop_all_xfers(pcd);
	}

	dwc_usb3_clr_eps_enabled(pcd);

	/* Clear stall on each EP */
	for (i = 0; i < pcd->num_in_eps; i++) {
		ep = pcd->in_ep[i];
		if (ep->dwc_ep.stopped)
			dwc_usb3_ep_clear_stall(pcd, ep);
	}
	for (i = 0; i < pcd->num_out_eps; i++) {
		ep = pcd->out_ep[i];
		if (ep->dwc_ep.stopped)
			dwc_usb3_ep_clear_stall(pcd, ep);
	}

	/* Set Device Address to 0 */
	dwc_usb3_set_address(pcd, 0);

	pcd->remote_wakeup_enable = 0;
	pcd->ltm_enable = 0;
}

/**
 * This interrupt occurs when a Connect Done is detected.
 * Read the device status register and set the device speed in the data
 * structure. Set up EP0 to receive SETUP packets.
 */

USBDEVICE_IO_DRIVER_T *pIoDriver;
void dwc_usb3_handle_connect_done_intr(dwc_usb3_pcd_t *pcd)
{
	dwc_usb3_pcd_ep_t *ep0 = pcd->ep0;
	unsigned int diepcfg0, doepcfg0, diepcfg1, doepcfg1;
	dwc_usb3_dev_ep_regs_t __iomem *ep_reg;
	int speed;

	/* The timing on reconnect after hibernation is so tight that we
	 * cannot afford the overhead of printing this to the dmesg log!
	 */
	if (!pcd->usb3_dev->core_params->hibernate ||
	    (pcd->usb3_dev->hwparams1 & DWC_HWP1_EN_PWROPT_BITS) !=
	    (DWC_EN_PWROPT_HIBERNATION << DWC_HWP1_EN_PWROPT_SHIFT))
		dwc_print0(pcd->usb3_dev, "CONNECT\n");

#ifdef DEBUG_EP0
	dwc_usb3_print_ep0_state(pcd);
#endif
	ep0->dwc_ep.stopped = 0;
	speed = dwc_usb3_get_device_speed(pcd);
	pcd->speed = speed;

	
	pIoDriver = USB30DEV_IO_GetDriver();

	if (pcd->speed == USB_SPEED_FULL)
	{
		pIoDriver->currentSpeed = USBDEV_FULL_SPEED;
		_dprintf(": Full Speed\n");
	}
	else if (pcd->speed == USB_SPEED_HIGH)
	{
		pIoDriver->currentSpeed = USBDEV_HIGH_SPEED;
		_dprintf(": High Speed\n");
	}
	else
	{
		_dprintf("Not Support Speed mode!!!\n");
	}

#ifdef DWC_STAR_9000483510_WORKAROUND
	if (speed == USB_SPEED_SUPER)
		handle_usb_reset_intr(pcd);
#endif
//	dwc_usb3_pcd_set_speed(pcd, speed);

	/*
	 * Set the MPS of EP0 based on the connection speed
	 */
	diepcfg0 = DWC_USB3_EP_TYPE_CONTROL << DWC_EPCFG0_EPTYPE_SHIFT;
	diepcfg0 |= DWC_CFG_ACTION_MODIFY << DWC_EPCFG0_CFG_ACTION_SHIFT;
	diepcfg1 = DWC_EPCFG1_XFER_CMPL_BIT | DWC_EPCFG1_XFER_IN_PROG_BIT |
		   DWC_EPCFG1_XFER_NRDY_BIT | DWC_EPCFG1_EP_DIR_BIT;

	doepcfg0 = DWC_USB3_EP_TYPE_CONTROL << DWC_EPCFG0_EPTYPE_SHIFT;
	doepcfg0 |= DWC_CFG_ACTION_MODIFY << DWC_EPCFG0_CFG_ACTION_SHIFT;
	doepcfg1 = DWC_EPCFG1_XFER_CMPL_BIT | DWC_EPCFG1_XFER_IN_PROG_BIT |
		   DWC_EPCFG1_XFER_NRDY_BIT;

	switch (speed) {
	case USB_SPEED_SUPER:
		diepcfg0 |= 512 << DWC_EPCFG0_MPS_SHIFT;
		doepcfg0 |= 512 << DWC_EPCFG0_MPS_SHIFT;
		break;

	case USB_SPEED_HIGH:
	case USB_SPEED_FULL:
		diepcfg0 |= 64 << DWC_EPCFG0_MPS_SHIFT;
		doepcfg0 |= 64 << DWC_EPCFG0_MPS_SHIFT;
		break;

	case USB_SPEED_LOW:
		diepcfg0 |= 8 << DWC_EPCFG0_MPS_SHIFT;
		doepcfg0 |= 8 << DWC_EPCFG0_MPS_SHIFT;
		break;
	}

#ifdef DWC_UTE
	ep0->dwc_ep.tx_fifo_num = pcd->txf_map[0];
#endif
	diepcfg0 |= ep0->dwc_ep.tx_fifo_num << DWC_EPCFG0_TXFNUM_SHIFT;

	/* Issue "DEPCFG" command to EP0-OUT */
	ep_reg = &pcd->out_ep_regs[0];
	dwc_usb3_dep_cfg(pcd, ep_reg, doepcfg0, doepcfg1, 0);

	/* Issue "DEPCFG" command to EP0-IN */
	ep_reg = &pcd->in_ep_regs[0];
	dwc_usb3_dep_cfg(pcd, ep_reg, diepcfg0, diepcfg1, 0);

	pcd->state = DWC_STATE_DEFAULT;

	//if (pcd->usb3_dev->hiber_wait_connect) {
	//	/* Already did 'Perform the steps in Section 9.1.3
	//	 * "Initialization on Connect Done" in [DWSS]'.
	//	 */
	//
	//	_dprintf( "Hibernation wait connect\n");
	//	pcd->usb3_dev->hiber_wait_connect = 0;
	//	dwc_exit_hibernation_after_connect(pcd, 0);
	//}
}

/**
 * This interrupt indicates that the USB link state has changed.
 */
static int handle_link_status_change_intr(dwc_usb3_pcd_t *pcd)
{
	int state;
	int speed;

	state = dwc_usb3_get_link_state(pcd);
	speed = dwc_usb3_get_device_speed(pcd);
	pcd->speed = speed;
	dwc_print2(pcd->usb3_dev, "LINK state=%d speed=%d\n", state, speed);

	switch (state) {
	case DWC_LINK_STATE_ON:
		if (pcd->usb3_dev->hiber_wait_u0) {
			pcd->speed = speed;
			if (pcd->remote_wakeup_enable)
				dwc_usb3_remote_wake(pcd, 0);
			pcd->usb3_dev->hiber_wait_u0 = 0;
		}

		/* If transitioning from 3->0 */
		if (pcd->link_state == DWC_LINK_STATE_SUSPEND) {
			_dprintf(
				   "Enabling function remote wake\n");
			pcd->wkup_rdy = 1;
		} else {
			pcd->wkup_rdy = 0;
		}

		pcd->link_state = state;
		break;

	case DWC_LINK_STATE_SUSPEND:
		/* If transitioning to 3 */
		if (pcd->link_state != DWC_LINK_STATE_SUSPEND)
			dwc_usb3_pcd_suspend(pcd);
		/* FALL-THRU */

	default:
		pcd->link_state = state;
		pcd->wkup_rdy = 0;
	}

	return 0;
}

/**
 * This interrupt indicates that the DWC_usb3 controller has detected a
 * resume or remote wakeup sequence. If the DWC_usb3 controller is in
 * low power mode, the handler must brings the controller out of low
 * power mode. The controller automatically begins resume
 * signaling. The handler schedules a time to stop resume signaling.
 */
static void handle_wakeup_detected_intr(dwc_usb3_pcd_t *pcd)
{
	dwc_usb3_device_t *dev = pcd->usb3_dev;

	dwc_debug0(dev, "++Resume and Remote Wakeup Detected Interrupt++\n");

	if (dwc_usb3_is_device_mode(dev)) {
		dwc_debug1(dev, "DSTS=0x%01x\n",
			   dwc_rd32(dev, &pcd->dev_global_regs->dsts));
		dwc_usb3_pcd_resume(pcd);
	}
}

/**
 * This interrupt indicates the end of the portion of the micro-frame
 * for periodic transactions.
 */
static void handle_end_periodic_frame_intr(dwc_usb3_pcd_t *pcd)
{
	_dprintf( "EOPF\n");
}

/**
 * This function handles the SOF Interrupts. At this time the SOF Interrupt
 * is disabled.
 */
static void handle_sof_intr(dwc_usb3_pcd_t *pcd)
{
	_dprintf( "SOF\n");
}

/**
 * This interrupt indicates that an EP has a pending interrupt.
 */
void dwc_usb3_handle_ep_intr(dwc_usb3_pcd_t *pcd, int physep, unsigned int event)
{
	dwc_usb3_pcd_ep_t *ep;
	int epnum, is_in, temp;
	char *dir;
#if 0
	dwc_usb3_dev_ep_regs_t __iomem *ep_reg;
#endif

	dwc_debug4(pcd->usb3_dev, "%s(%p,%d,0x%01x)\n",
		   __func__, pcd, physep, event);

	/* Physical Out EPs are even, physical In EPs are odd */
	is_in = physep & 1;
	epnum = (physep >> 1) & 0xf;

	/* Get EP pointer */
	if (is_in) {
		ep = dwc_usb3_get_in_ep(pcd, epnum);
		dir = "IN";
	} else {
		ep = dwc_usb3_get_out_ep(pcd, epnum);
		dir = "OUT";
	}

	dwc_debug1(pcd->usb3_dev, "%s EP intr\n", dir);

#ifdef VERBOSE
	dwc_debug4(pcd->usb3_dev, "EP%d-%s: type=%d mps=%d\n",
		   ep->dwc_ep.num, (ep->dwc_ep.is_in ? "IN" : "OUT"),
		   ep->dwc_ep.type, ep->dwc_ep.maxpacket);
#endif
#if 0		/* 012.08.13 */
	temp = atomic_read(&pcd->usb3_dev->hibernate);
	if (temp >= DWC_HIBER_SLEEPING && temp != DWC_HIBER_SS_DIS_QUIRK) {
		dwc_info3(pcd->usb3_dev,
			  "EP%d-%s: got event 0x%08x while hibernating\n",
			  ep->dwc_ep.num, (ep->dwc_ep.is_in ? "IN" : "OUT"),
			  event);
		ep->dwc_ep.last_event = event;
		return;
	}
#endif /* 0 */

	switch (event & DWC_DEPEVT_INTTYPE_BITS) {

	//==========================================
	// Transfer Complete 
	//==========================================
	case DWC_DEPEVT_XFER_CMPL << DWC_DEPEVT_INTTYPE_SHIFT:
#ifdef VERBOSE
		dwc_debug2(pcd->usb3_dev, "[EP%d] %s xfer complete\n",
			   epnum, dir);
#endif
		ep->dwc_ep.xfer_started = 0;

		if (ep->dwc_ep.type != UE_ISOCHRONOUS) {
			/* Complete the transfer */
			if (epnum == 0)
				dwc_usb3_os_handle_ep0(pcd, event);
			else
				dwc_usb3_os_complete_request(pcd, ep, event);
		} else {
			dwc_print2(pcd->usb3_dev,
				   "[EP%d] %s xfer complete for ISOC EP!\n",
				   epnum, dir);
		}

		break;

	//==========================================
	// Transfer progress 
	//==========================================
	case DWC_DEPEVT_XFER_IN_PROG << DWC_DEPEVT_INTTYPE_SHIFT:
#ifdef VERBOSE
		dwc_debug2(pcd->usb3_dev, "[EP%d] %s xfer in progress\n",
			   epnum, dir);
#endif
		if (ep->dwc_ep.type == UE_ISOCHRONOUS) {
			/* Complete the transfer */
			dwc_usb3_os_complete_request(pcd, ep, event);
		} else {
			dwc_print2(pcd->usb3_dev,
				"[EP%d] %s xfer in progress for non-ISOC EP!\n",
				 epnum, dir);

			/* Complete the transfer */
			if (epnum == 0)
				dwc_usb3_os_handle_ep0(pcd, event);
			else
				dwc_usb3_os_complete_request(pcd, ep, event);
		}

		break;

	//==========================================
	// Transfer Not Ready
	//==========================================
	case DWC_DEPEVT_XFER_NRDY << DWC_DEPEVT_INTTYPE_SHIFT:
		dwc_debug2(pcd->usb3_dev, "[EP%d] %s xfer not ready\n",
			   epnum, dir);

		if (epnum == 0) {
			switch (pcd->ep0state) {
			case EP0_IN_WAIT_GADGET:
			case EP0_IN_WAIT_NRDY:
				if (is_in)
					dwc_usb3_os_handle_ep0(pcd, event);
				break;
			case EP0_OUT_WAIT_GADGET:
			case EP0_OUT_WAIT_NRDY:
				if (!is_in)
					dwc_usb3_os_handle_ep0(pcd, event);
				break;
			default:
				break;
			}
		} else if (ep->dwc_ep.type == UE_ISOCHRONOUS) {
			dwc_isocdbg2(pcd->usb3_dev,
				     "[EP%d] %s xfer not ready for ISOC EP!\n",
				     epnum, dir);
			if (!ep->dwc_ep.xfer_started)
				dwc_usb3_os_isoc_ep_start(pcd, ep, event);
			break;
		}

		break;

	case DWC_DEPEVT_FIFOXRUN << DWC_DEPEVT_INTTYPE_SHIFT:
		dwc_error2(pcd->usb3_dev, "[EP%d] %s FIFO Underrun Error!\n",
			   epnum, dir);
		break;

	case DWC_DEPEVT_EPCMD_CMPL << DWC_DEPEVT_INTTYPE_SHIFT:
		dwc_error2(pcd->usb3_dev, "[EP%d] %s Command Complete!\n",
			   epnum, dir);
		break;
	}
}

extern void USB30DEV_IO_EP_Disable_ALL(void);
/**
 * PCD interrupt handler.
 *
 * The PCD handles the device interrupts. Many conditions can cause a
 * device interrupt. When an interrupt occurs, the device interrupt
 * service routine determines the cause of the interrupt and
 * dispatches handling to the appropriate function.
 */
int dwc_usb3_handle_dev_intr(dwc_usb3_pcd_t *pcd, unsigned int event)
{
	unsigned int dint = (event >> DWC_DEVT_SHIFT) &
			(DWC_DEVT_BITS >> DWC_DEVT_SHIFT);
	int temp, ret = 0;

	switch (dint) {
	case DWC_DEVT_DISCONN:
		_dprintf( "Disconnect\n");
		ret = handle_disconnect_intr(pcd);
		USB30DEV_IO_EP_Disable_ALL();
		break;

	case DWC_DEVT_USBRESET:
		_dprintf( "USB Reset\n");
		handle_usb_reset_intr(pcd);
		USBDEV_Reset();
		break;

	case DWC_DEVT_CONNDONE:
		_dprintf( "Connect Done");
		dwc_usb3_handle_connect_done_intr(pcd);
		break;

	case DWC_DEVT_EOPF:
		_dprintf( "End of Periodic Frame\n");
		handle_end_periodic_frame_intr(pcd);
		break;

	case DWC_DEVT_SOF:
		_dprintf( "Start of Frame\n");
		handle_sof_intr(pcd);
		break;

	case DWC_DEVT_ERRATICERR:
		_dprintf( "Erratic Error\n");
		break;

	case DWC_DEVT_CMD_CMPL:
		_dprintf( "Command Complete\n");
		break;

	case DWC_DEVT_OVERFLOW:
		_dprintf( "Overflow\n");
		break;

	default:
		pcd_intr_dprintf( "Unknown event! dint: %d\n", dint);
	}

	return ret;
}
