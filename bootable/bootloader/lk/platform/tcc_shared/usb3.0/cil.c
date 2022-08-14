/* ==========================================================================
 * $File: //dwh/usb_iip/dev/software/DWC_usb3/driver/cil.c $
 * $Revision: #96 $
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
 *
 * The Core Interface Layer provides basic services for accessing and
 * managing the DWC_usb3 hardware. These services are used by both the
 * Peripheral Controller Driver and the On The Go Driver.
 *
 * The CIL manages the memory map for the core so that the PCD and OTGD
 * don't have to do this separately. The CIL also performs basic services
 * that are not specific to either the host or device modes of operation.
 * These services include all functionality that requires specific
 * knowledge of the CSR layout or the DMA descriptor (TRB) layout. Also
 * included are services for invoking each of the commands provided by
 * the DGCMD and DEPCMD registers (see the "Control and Status Registers"
 * chapter of the USB3 controller databook for details).
 *
 * The Core Interface Layer has the following requirements:
 * - Provides basic controller operations.
 * - Minimal use of OS services.
 * - The OS services used will be abstracted by using inline functions
 *   or macros.
 *
 */

#include "usb3.0/os_dep.h"
#include "usb3.0/hw.h"
#include "usb3.0/usb.h"
#include "usb3.0/pcd.h"
#include "usb3.0/driver.h"
#include "usb3.0/cil.h"
#define cil_debug(f, a...)    		dprintf(3,"" f, ##a) 

/**
 * Fill in the four dwords of a DMA descriptor (aka a TRB).
 */
void dwc_usb3_fill_desc(dwc_usb3_dma_desc_t *desc, dwc_dma_t dma_addr,
			unsigned int dma_len, unsigned int stream, unsigned int type,
			unsigned int ctrlbits, int own)
{
	desc->bptl = (unsigned int)(dma_addr & 0xffffffffU);
#ifdef DWC_64_BIT_ARCH
	desc->bpth = (unsigned int)((dma_addr >> 32U) & 0xffffffffU);
#else
	desc->bpth = 0;
#endif
	desc->status = dma_len << DWC_DSCSTS_XFRCNT_SHIFT;

	/* Note: If type is 0, leave original control bits intact (for isoc) */
	if (type)
		desc->control = type << DWC_DSCCTL_TRBCTL_SHIFT;

	desc->control |= (stream << DWC_DSCCTL_STRMID_SOFN_SHIFT) | ctrlbits;

	/* Must do this last! */
	if (own)
		desc->control |= DWC_DSCCTL_HWO_BIT;
}

/**
 * Make a DMA descriptor the start of a chain by setting its CHN bit and
 * clearing its IOC bit.
 *
 * @param desc The DMA descriptor (TRB) to operate on.
 */
void dwc_usb3_start_desc_chain(dwc_usb3_dma_desc_t *desc)
{
	desc->control |= DWC_DSCCTL_CHN_BIT;
	desc->control &= ~DWC_DSCCTL_IOC_BIT;
}

/**
 * Make a DMA descriptor the end of a chain by clearing its CHN bit and
 * setting its IOC bit.
 *
 * @param desc The DMA descriptor (TRB) to operate on.
 */
void dwc_usb3_end_desc_chain(dwc_usb3_dma_desc_t *desc)
{
	desc->control &= ~DWC_DSCCTL_CHN_BIT;
	desc->control |= DWC_DSCCTL_IOC_BIT;
}

/**
 * Enable a DMA descriptor by setting its HWO bit.
 *
 * @param desc The DMA descriptor (TRB) to operate on.
 */
void dwc_usb3_enable_desc(dwc_usb3_dma_desc_t *desc)
{
	desc->control |= DWC_DSCCTL_HWO_BIT;
}

/**
 * Spin on register bit until handshake completes or times out.
 *
 * @param dev   Programming view of DWC_usb3 controller.
 * @param ptr   Address of register to read.
 * @param mask  Bit to look at in result of read.
 * @param done  Value of the bit when handshake succeeds.
 * @return 1 when the mask bit has the specified value (handshake done),
 *         0 when timeout has passed (handshake failed).
 */

volatile int usb_nop_count = 0;
#define _ASM_NOP_ { usb_nop_count++; }

void usb_delay_us(unsigned int us)
{
	int i;
	while(us--)
	{
		for(i=0 ; i<10; i++)
			_ASM_NOP_ 
	}
}

#define dwc_udelay(dev, us)	usb_delay_us(us)


static int _handshake(dwc_usb3_device_t *dev, volatile unsigned int   *ptr,
		      unsigned int mask, unsigned int done)
{
	unsigned int usec = 100000;
	unsigned int result;

	//cil_debug( "%s()\n", __func__);

	do {
		result = dwc_rd32(dev, ptr);
		if ((result & mask) == done) {
			cil_debug( "after DEPCMD=%08x\n", result);
			return 1;
		}

		dwc_udelay(dev, 1);
		usec -= 1;
	} while (usec > 0);

	return 0;
}

#define handshake(_dev, _ptr, _mask, _done) ({				\
	int _retval = _handshake(_dev, _ptr, _mask, _done);		\
	if (!_retval)							\
		dwc_error2(_dev, "handshake failed in %s():%d\n",	\
			   __func__, __LINE__);				\
	_retval;							\
})

/*
 * Routines for sending the various Device commands to the hardware.
 * See description of DGCMD register in "Control and Status Registers"
 * section of the USB3 controller databook.
 */

/**
 * Send TRANSMIT FUNCTION WAKE DEVICE NOTIFICATION command to Device
 */
int dwc_usb3_xmit_fn_remote_wake(dwc_usb3_pcd_t *pcd, unsigned int intf)
{
	dwc_debug1(pcd->usb3_dev, "%s()\n", __func__);

	/* Set param */
	dwc_wr32(pcd->usb3_dev, &pcd->dev_global_regs->dgcmdpar, intf);

	/* Start the command */
	dwc_wr32(pcd->usb3_dev, &pcd->dev_global_regs->dgcmd,
		 DWC_DGCMD_XMIT_FUNC_WAKE_DEV_NOTIF | DWC_DGCMD_ACT_BIT);
	dwc_debug1(pcd->usb3_dev, "DGCMD=%08x\n",
		   DWC_DGCMD_XMIT_FUNC_WAKE_DEV_NOTIF | DWC_DGCMD_ACT_BIT);

	/* Wait for command completion */
	handshake(pcd->usb3_dev, &pcd->dev_global_regs->dgcmd,
		  DWC_DGCMD_ACT_BIT, 0);

	return 0;
}

#ifdef CONFIG_USB_OTG_DWC
/**
 * Send ROLE REQUEST device notification
 */
int dwc_usb3_xmit_host_role_request(dwc_usb3_pcd_t *pcd, u32 param)
{
#ifdef DEBUG
	char *type = "UNKNOWN";

	if (param == DWC_DGCMDPAR_HOST_ROLE_REQ_INITIATE)
		type = "INITIATE";
	else if (param == DWC_DGCMDPAR_HOST_ROLE_REQ_CONFIRM)
		type = "CONFIRM";

	dwc_debug2(pcd->usb3_dev, "%s() - %s\n", __func__, type);
#endif

	/* Set param */
	dwc_wr32(pcd->usb3_dev, &pcd->dev_global_regs->dgcmdpar, param);

	/* Start the command */
	dwc_wr32(pcd->usb3_dev, &pcd->dev_global_regs->dgcmd,
		 DWC_DGCMD_XMIT_HOST_ROLE_REQUEST | DWC_DGCMD_ACT_BIT);

	dwc_debug1(pcd->usb3_dev, "DGCMD=%08x\n",
		   DWC_DGCMD_XMIT_HOST_ROLE_REQUEST | DWC_DGCMD_ACT_BIT);
	dwc_debug1(pcd->usb3_dev, "DGCMDPAR=%08x\n", param);

	/* Wait for command completion */
	handshake(pcd->usb3_dev, &pcd->dev_global_regs->dgcmd,
		  DWC_DGCMD_ACT_BIT, 0);

	return 0;
}
#endif

/**
 * Send SET SCRATCHPAD BUFFER ARRAY command to Device
 */
int dwc_usb3_set_scratchpad_buf_array(dwc_usb3_pcd_t *pcd, dwc_dma_t dma_addr)
{
	dwc_debug2(pcd->usb3_dev, "%s(%lx)\n", __func__,
		   (unsigned long)dma_addr);

	/* Set param */
	dwc_wr32(pcd->usb3_dev, &pcd->dev_global_regs->dgcmdpar,
		 dma_addr & 0xffffffffU);

	/* Start the command */
	dwc_wr32(pcd->usb3_dev, &pcd->dev_global_regs->dgcmd,
		 DWC_DGCMD_SET_SCRATCHPAD_ARRAY_ADR_LO | DWC_DGCMD_ACT_BIT);
	dwc_debug1(pcd->usb3_dev, "DGCMD=%08x\n",
		   DWC_DGCMD_SET_SCRATCHPAD_ARRAY_ADR_LO | DWC_DGCMD_ACT_BIT);

	/* Wait for command completion */
	handshake(pcd->usb3_dev, &pcd->dev_global_regs->dgcmd,
		  DWC_DGCMD_ACT_BIT, 0);

	return 0;
}

/*
 * Routines for sending the various Endpoint commands to the hardware.
 * See description of DEPCMDn register in "Control and Status Registers"
 * section of the USB3 controller databook.
 */

/**
 * Send DEPCFG command to EP
 */
int dwc_usb3_dep_cfg(dwc_usb3_pcd_t *pcd,
		     dwc_usb3_dev_ep_regs_t   *ep_reg,
		     unsigned int depcfg0, unsigned int depcfg1, unsigned int depcfg2)
{
	dwc_debug1(pcd->usb3_dev, "dep_cfg, ep_reg=%p\n", ep_reg);

	/* Set param 2 */
	dwc_wr32(pcd->usb3_dev, &ep_reg->depcmdpar2, depcfg2);
	dwc_debug1(pcd->usb3_dev, "DEPCFG2=%08x\n", depcfg2);

	/* Set param 1 */
	dwc_wr32(pcd->usb3_dev, &ep_reg->depcmdpar1, depcfg1);
	dwc_debug1(pcd->usb3_dev, "DEPCFG1=%08x\n", depcfg1);

	/* Set param 0 */
	dwc_wr32(pcd->usb3_dev, &ep_reg->depcmdpar0, depcfg0);
	dwc_debug1(pcd->usb3_dev, "DEPCFG0=%08x\n", depcfg0);

	/* Start the command */
	dwc_wr32(pcd->usb3_dev, &ep_reg->depcmd,
		 DWC_EPCMD_SET_EP_CFG | DWC_EPCMD_ACT_BIT);
	dwc_debug1(pcd->usb3_dev, "DEPCMD=%08x\n",
		   DWC_EPCMD_SET_EP_CFG | DWC_EPCMD_ACT_BIT);

	/* Wait for command completion */
	handshake(pcd->usb3_dev, &ep_reg->depcmd, DWC_EPCMD_ACT_BIT, 0);

	return 0;
}

/**
 * Send DEPXFERCFG command to EP
 */
int dwc_usb3_dep_xfercfg(dwc_usb3_pcd_t *pcd,
			 dwc_usb3_dev_ep_regs_t   *ep_reg,
			 unsigned int depstrmcfg)
{
	dwc_debug1(pcd->usb3_dev, "dep_xfercfg, ep_reg=%p\n", ep_reg);

	/* Set param 0 */
	dwc_wr32(pcd->usb3_dev, &ep_reg->depcmdpar0, depstrmcfg);
	dwc_debug1(pcd->usb3_dev, "DEPSTRMCFG=%08x\n", depstrmcfg);

	/* Start the command */
	dwc_wr32(pcd->usb3_dev, &ep_reg->depcmd,
		 DWC_EPCMD_SET_XFER_CFG | DWC_EPCMD_ACT_BIT);
	dwc_debug1(pcd->usb3_dev, "DEPCMD=%08x\n",
		   DWC_EPCMD_SET_XFER_CFG | DWC_EPCMD_ACT_BIT);

	/* Wait for command completion */
	handshake(pcd->usb3_dev, &ep_reg->depcmd, DWC_EPCMD_ACT_BIT, 0);

	return 0;
}

/**
 * Send DEPGETEPSTATE command to EP
 */
unsigned int dwc_usb3_dep_getepstate(dwc_usb3_pcd_t *pcd,
				 dwc_usb3_dev_ep_regs_t   *ep_reg)
{
	unsigned int retval;

	dwc_debug1(pcd->usb3_dev, "dep_getepstate, ep_reg=%p\n", ep_reg);

	/* Start the command */
	dwc_wr32(pcd->usb3_dev, &ep_reg->depcmd,
		 DWC_EPCMD_GET_EP_STATE | DWC_EPCMD_ACT_BIT);
	dwc_debug1(pcd->usb3_dev, "DEPCMD=%08x\n",
		   DWC_EPCMD_GET_EP_STATE | DWC_EPCMD_ACT_BIT);

	/* Wait for command completion */
	handshake(pcd->usb3_dev, &ep_reg->depcmd, DWC_EPCMD_ACT_BIT, 0);

	/* Get state returned in DEPCMDPAR2 */
	retval = dwc_rd32(pcd->usb3_dev, &ep_reg->depcmdpar2);

	return retval;
}

/**
 * Send DEPSSTALL command to EP
 */
int dwc_usb3_dep_sstall(dwc_usb3_pcd_t *pcd,
			dwc_usb3_dev_ep_regs_t   *ep_reg)
{
	dwc_debug1(pcd->usb3_dev, "dep_sstall, ep_reg=%p\n", ep_reg);

	/* Start the command */
	dwc_wr32(pcd->usb3_dev, &ep_reg->depcmd,
		 DWC_EPCMD_SET_STALL | DWC_EPCMD_ACT_BIT);
	dwc_debug1(pcd->usb3_dev, "DEPCMD=%08x\n",
		   DWC_EPCMD_SET_STALL | DWC_EPCMD_ACT_BIT);

	/* Wait for command completion */
	handshake(pcd->usb3_dev, &ep_reg->depcmd, DWC_EPCMD_ACT_BIT, 0);

	return 0;
}

/**
 * Send DEPCSTALL command to EP
 */
int dwc_usb3_dep_cstall(dwc_usb3_pcd_t *pcd,
			dwc_usb3_dev_ep_regs_t   *ep_reg)
{
	dwc_debug1(pcd->usb3_dev, "dep_cstall, ep_reg=%p\n", ep_reg);

	/* Start the command */
	dwc_wr32(pcd->usb3_dev, &ep_reg->depcmd,
		 DWC_EPCMD_CLR_STALL | DWC_EPCMD_ACT_BIT);
	dwc_debug1(pcd->usb3_dev, "DEPCMD=%08x\n",
		   DWC_EPCMD_CLR_STALL | DWC_EPCMD_ACT_BIT);

	/* Wait for command completion */
	handshake(pcd->usb3_dev, &ep_reg->depcmd, DWC_EPCMD_ACT_BIT, 0);

	return 0;
}

/**
 * Send DEPSTRTXFER command to EP
 */
int dwc_usb3_dep_startxfer(dwc_usb3_pcd_t *pcd,
			   dwc_usb3_dev_ep_regs_t   *ep_reg,
			   dwc_dma_t dma_addr, unsigned int stream_or_uf)
{
	unsigned int depcmd;

	dwc_debug1(pcd->usb3_dev, "dep_startxfer, ep_reg=%p\n", ep_reg);

	/* Set param 1 */
	dwc_wr32(pcd->usb3_dev, &ep_reg->depcmdpar1, dma_addr & 0xffffffffU);
	dwc_debug1(pcd->usb3_dev, "TDADDRLO=%08lx\n",
		   (unsigned long)(dma_addr & 0xffffffffU));

	/* Set param 0 */
#ifdef DWC_64_BIT_ARCH
	dwc_wr32(pcd->usb3_dev, &ep_reg->depcmdpar0,
		 (dma_addr >> 32U) & 0xffffffffU);
	dwc_debug1(pcd->usb3_dev, "TDADDRHI=%08lx\n",
		   (unsigned long)((dma_addr >> 32U) & 0xffffffffU));
#else
	dwc_wr32(pcd->usb3_dev, &ep_reg->depcmdpar0, 0);
	dwc_debug1(pcd->usb3_dev, "TDADDRHI=%08x\n", 0);
#endif
	/* Start the command */
	dwc_wr32(pcd->usb3_dev, &ep_reg->depcmd,
		 (stream_or_uf << DWC_EPCMD_STR_NUM_OR_UF_SHIFT) |
		 DWC_EPCMD_START_XFER | DWC_EPCMD_ACT_BIT);
	dwc_debug1(pcd->usb3_dev, "DEPCMD=%08x\n",
		   (stream_or_uf << DWC_EPCMD_STR_NUM_OR_UF_SHIFT) |
		   DWC_EPCMD_START_XFER | DWC_EPCMD_ACT_BIT);

	/* Wait for command completion */
	handshake(pcd->usb3_dev, &ep_reg->depcmd, DWC_EPCMD_ACT_BIT, 0);

	depcmd = dwc_rd32(pcd->usb3_dev, &ep_reg->depcmd);

	return (depcmd >> DWC_EPCMD_XFER_RSRC_IDX_SHIFT) &
	       (DWC_EPCMD_XFER_RSRC_IDX_BITS >> DWC_EPCMD_XFER_RSRC_IDX_SHIFT);
}

/**
 * Send DEPUPDTXFER command to EP
 */
int dwc_usb3_dep_updatexfer(dwc_usb3_pcd_t *pcd,
			    dwc_usb3_dev_ep_regs_t   *ep_reg,
			    unsigned int tri)
{
	dwc_debug1(pcd->usb3_dev, "dep_updxfer, ep_reg=%p\n", ep_reg);

	/* Start the command */
	dwc_wr32(pcd->usb3_dev, &ep_reg->depcmd,
		 (tri << DWC_EPCMD_XFER_RSRC_IDX_SHIFT) |
		 DWC_EPCMD_UPDATE_XFER | DWC_EPCMD_ACT_BIT);
	dwc_debug1(pcd->usb3_dev, "DEPCMD=%08x\n",
		   (tri << DWC_EPCMD_XFER_RSRC_IDX_SHIFT) |
		   DWC_EPCMD_UPDATE_XFER | DWC_EPCMD_ACT_BIT);

	/* Wait for command completion */
	handshake(pcd->usb3_dev, &ep_reg->depcmd, DWC_EPCMD_ACT_BIT, 0);

	return 0;
}

/**
 * Send DEPENDXFER command to EP
 */
int dwc_usb3_dep_endxfer(dwc_usb3_pcd_t *pcd,
			 dwc_usb3_dev_ep_regs_t   *ep_reg,
			 unsigned int tri, int force, void *condition)
{
	unsigned int depcmd;

	dwc_debug1(pcd->usb3_dev, "dep_endxfer, ep_reg=%p\n", ep_reg);

	/* Fill end transfer command */
	depcmd = (tri << DWC_EPCMD_XFER_RSRC_IDX_SHIFT) | DWC_EPCMD_END_XFER;
	depcmd |= DWC_EPCMD_ACT_BIT;
	if (force)
		depcmd |= DWC_EPCMD_HP_FRM_BIT;

	/* Start the command. */
	dwc_wr32(pcd->usb3_dev, &ep_reg->depcmd, depcmd);
	dwc_debug1(pcd->usb3_dev, "DEPCMD=%08x\n", depcmd);

	/* Wait for command completion */
	handshake(pcd->usb3_dev, &ep_reg->depcmd, DWC_EPCMD_ACT_BIT, 0);

	return 0;
}

#ifdef DWC_STAR_9000463548_WORKAROUND
int dwc_usb3_dep_endxfer_nowait(dwc_usb3_pcd_t *pcd,
				dwc_usb3_dev_ep_regs_t   *ep_reg,
				unsigned int tri, int force)
{
	unsigned int depcmd;

	dwc_debug1(pcd->usb3_dev, "dep_endxfer_nowait, ep_reg=%p\n", ep_reg);

	/* Fill end transfer command */
	depcmd = (tri << DWC_EPCMD_XFER_RSRC_IDX_SHIFT) | DWC_EPCMD_END_XFER;
	depcmd |= DWC_EPCMD_ACT_BIT;
	if (force)
		depcmd |= DWC_EPCMD_HP_FRM_BIT;

	/* Start the command. */
	dwc_wr32(pcd->usb3_dev, &ep_reg->depcmd, depcmd);
	dwc_debug1(pcd->usb3_dev, "DEPCMD=%08x\n", depcmd);

	return 0;
}

int dwc_usb3_dep_wait_endxfer(dwc_usb3_pcd_t *pcd,
			      dwc_usb3_dev_ep_regs_t   *ep_reg,
			      void *condition)
{
	/* Wait for command completion */
	handshake(pcd->usb3_dev, &ep_reg->depcmd, DWC_EPCMD_ACT_BIT, 0);

	return 0;
}
#endif

/**
 * Send DEPSTRTNEWCFG command to EP
 */
int dwc_usb3_dep_startnewcfg(dwc_usb3_pcd_t *pcd,
			     dwc_usb3_dev_ep_regs_t   *ep_reg,
			     unsigned int rsrcidx)
{
	dwc_debug1(pcd->usb3_dev, "dep_startnewcfg, ep_reg=%p\n", ep_reg);

	/* Start the command */
	dwc_wr32(pcd->usb3_dev, &ep_reg->depcmd,
		 (rsrcidx << DWC_EPCMD_XFER_RSRC_IDX_SHIFT) |
		 DWC_EPCMD_START_NEW_CFG | DWC_EPCMD_ACT_BIT);
	dwc_debug1(pcd->usb3_dev, "DEPCMD=%08x\n",
		   (rsrcidx << DWC_EPCMD_XFER_RSRC_IDX_SHIFT) |
		   DWC_EPCMD_START_NEW_CFG | DWC_EPCMD_ACT_BIT);

	/* Wait for command completion */
	handshake(pcd->usb3_dev, &ep_reg->depcmd, DWC_EPCMD_ACT_BIT, 0);

	return 0;
}

/**********************/

/**
 * Enable an EP in the DALEPENA register.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 * @param ep  The EP to enable.
 * @return    0 if succesful, -DWC_E_BUSY if already enabled.
 */
int dwc_usb3_enable_ep(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep)
{
	unsigned int ep_index_num, dalepena;

	ep_index_num = ep->dwc_ep.num * 2;

	if (ep->dwc_ep.is_in)
		ep_index_num += 1;

	dalepena = dwc_rd32(pcd->usb3_dev, &pcd->dev_global_regs->dalepena);

	/* If we have already enabled this EP, leave it alone
	 * (shouldn't happen)
	 */
	if (dalepena & (1 << ep_index_num))
		return -DWC_E_BUSY;

	dalepena |= 1 << ep_index_num;
	dwc_wr32(pcd->usb3_dev, &pcd->dev_global_regs->dalepena, dalepena);
	dwc_print1(pcd->usb3_dev, "enable_ep: DALEPENA=%08x\n", dalepena);
	return 0;
}

/**
 * Disable an EP in the DALEPENA register.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 * @param ep  The EP to disable.
 * @return    0 if succesful, -DWC_E_INVALID if already disabled.
 */
int dwc_usb3_disable_ep(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep)
{
	unsigned int ep_index_num, dalepena;

	ep_index_num = ep->dwc_ep.num * 2;

	if (ep->dwc_ep.is_in)
		ep_index_num += 1;

	dalepena = dwc_rd32(pcd->usb3_dev, &pcd->dev_global_regs->dalepena);

	/* If we have already disabled this EP, leave it alone
	 * (shouldn't happen)
	 */
	if (!(dalepena & (1 << ep_index_num)))
		return -DWC_E_INVALID;

	dalepena &= ~(1 << ep_index_num);
	dwc_wr32(pcd->usb3_dev, &pcd->dev_global_regs->dalepena, dalepena);
	dwc_print1(pcd->usb3_dev, "disable_ep: DALEPENA=%08x\n", dalepena);
	return 0;
}

/**
 * Get the device speed from the device status register and convert it
 * to USB speed constant.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 * @return    The device speed.
 */
int dwc_usb3_get_device_speed(dwc_usb3_pcd_t *pcd)
{
	unsigned int dsts;
	int speed = USB_SPEED_UNKNOWN;

	dsts = dwc_rd32(pcd->usb3_dev, &pcd->dev_global_regs->dsts);

	switch ((dsts >> DWC_DSTS_CONNSPD_SHIFT) &
		(DWC_DSTS_CONNSPD_BITS >> DWC_DSTS_CONNSPD_SHIFT)) {
	case DWC_SPEED_HS_PHY_30MHZ_OR_60MHZ:
		dwc_debug0(pcd->usb3_dev, "HIGH SPEED\n");
		speed = USB_SPEED_HIGH;
		break;

	case DWC_SPEED_FS_PHY_30MHZ_OR_60MHZ:
	case DWC_SPEED_FS_PHY_48MHZ:
		dwc_debug0(pcd->usb3_dev, "FULL SPEED\n");
		speed = USB_SPEED_FULL;
		break;

	case DWC_SPEED_LS_PHY_6MHZ:
		dwc_debug0(pcd->usb3_dev, "LOW SPEED\n");
		speed = USB_SPEED_LOW;
		break;

	case DWC_SPEED_SS_PHY_125MHZ_OR_250MHZ:
		dwc_debug0(pcd->usb3_dev, "SUPER SPEED\n");
		speed = USB_SPEED_SUPER;
		break;
	}

	return speed;
}

/**
 * Get the current microframe number.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 * @return    The current microframe number.
 */
int dwc_usb3_get_frame(dwc_usb3_pcd_t *pcd)
{
	unsigned int dsts;

	/* read current frame/microframe number from DSTS register */
	dsts = dwc_rd32(pcd->usb3_dev, &pcd->dev_global_regs->dsts);

	return (dsts >> DWC_DSTS_SOF_FN_SHIFT) &
	       (DWC_DSTS_SOF_FN_BITS >> DWC_DSTS_SOF_FN_SHIFT);
}

/**
 * Get the current link state.
 *
 * If the LNR (Link-state Not Ready) field of DSTS is set to 1, continue polling
 * the DSTS register until LNR=0.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 * @return    The current link state.
 */
int dwc_usb3_get_link_state(dwc_usb3_pcd_t *pcd)
{
	unsigned int status, state;

	for (;;) {
		status = dwc_rd32(pcd->usb3_dev, &pcd->dev_global_regs->dsts);
		if (status & DWC_DSTS_LNR_BIT)
			dwc_udelay(pcd->usb3_dev, 1);
		else
			break;
	}

	status = dwc_rd32(pcd->usb3_dev, &pcd->dev_global_regs->dsts);
	state = (status >> DWC_DSTS_USBLNK_STATE_SHIFT) &
		(DWC_DSTS_USBLNK_STATE_BITS >> DWC_DSTS_USBLNK_STATE_SHIFT);
	dwc_debug2(pcd->usb3_dev, "status=0x%08x state=%d\n", status, state);

	return state;
}

/**
 * Set state of USB link.
 *
 * @param pcd:   Programming view of DWC_usb3 peripheral controller.
 * @param state: Link state to set.
 */
void dwc_usb3_set_link_state(dwc_usb3_pcd_t *pcd, int state)
{
	unsigned int dctl;

	dctl = dwc_rd32(pcd->usb3_dev, &pcd->dev_global_regs->dctl);
	dctl &= ~DWC_DCTL_ULST_CHNG_REQ_BITS;
	dwc_wr32(pcd->usb3_dev, &pcd->dev_global_regs->dctl, dctl);

	dctl = dwc_rd32(pcd->usb3_dev, &pcd->dev_global_regs->dctl);
	dctl &= ~DWC_DCTL_ULST_CHNG_REQ_BITS;
	dctl |= state << DWC_DCTL_ULST_CHNG_REQ_SHIFT;
	dwc_wr32(pcd->usb3_dev, &pcd->dev_global_regs->dctl, dctl);
}

/**
 * Send a Remote Wakeup to the host.
 *
 * @param pcd:      Programming view of DWC_usb3 peripheral controller.
 * @param function: Function that caused the remote wakeup.
 */
void dwc_usb3_remote_wake(dwc_usb3_pcd_t *pcd, int function)
{
	/* For USB 3.0, send function remote wake notification */
	if (pcd->speed == USB_SPEED_SUPER)
		dwc_usb3_xmit_fn_remote_wake(pcd, function);
}

/**
 * Set the Device Address.
 *
 * @param pcd  Programming view of DWC_usb3 peripheral controller.
 * @param addr The address to set.
 */
void dwc_usb3_set_address(dwc_usb3_pcd_t *pcd, int addr)
{
	unsigned int dcfg;

	dcfg = dwc_rd32(pcd->usb3_dev, &pcd->dev_global_regs->dcfg);
	dcfg &= ~DWC_DCFG_DEVADDR_BITS;
	dcfg |= addr << DWC_DCFG_DEVADDR_SHIFT;
	dwc_wr32(pcd->usb3_dev, &pcd->dev_global_regs->dcfg, dcfg);
}

/**
 * Suspend the USB2 Phy.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 */
void dwc_usb3_suspend_usb2_phy(dwc_usb3_pcd_t *pcd)
{
	unsigned int usb2phycfg;

	usb2phycfg = dwc_rd32(pcd->usb3_dev,
			      &pcd->usb3_dev->core_global_regs->gusb2phycfg[0]);
	usb2phycfg |= DWC_USB2PHYCFG_SUS_PHY_BIT;
	dwc_wr32(pcd->usb3_dev,
		 &pcd->usb3_dev->core_global_regs->gusb2phycfg[0], usb2phycfg);
}

/**
 * Un-suspend the USB2 Phy.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 */
void dwc_usb3_resume_usb2_phy(dwc_usb3_pcd_t *pcd)
{
	unsigned int usb2phycfg;

	usb2phycfg = dwc_rd32(pcd->usb3_dev,
			      &pcd->usb3_dev->core_global_regs->gusb2phycfg[0]);
	usb2phycfg &= ~DWC_USB2PHYCFG_SUS_PHY_BIT;
	dwc_wr32(pcd->usb3_dev,
		 &pcd->usb3_dev->core_global_regs->gusb2phycfg[0], usb2phycfg);
}

/**
 * Suspend the USB3 Phy.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 */
void dwc_usb3_suspend_usb3_phy(dwc_usb3_pcd_t *pcd)
{
	unsigned int pipectl;

	pipectl = dwc_rd32(pcd->usb3_dev,
			   &pcd->usb3_dev->core_global_regs->gusb3pipectl[0]);
	pipectl |= DWC_PIPECTL_SUS_PHY_BIT;
	dwc_wr32(pcd->usb3_dev,
		 &pcd->usb3_dev->core_global_regs->gusb3pipectl[0], pipectl);
}

/**
 * Un-suspend the USB3 Phy.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 */
void dwc_usb3_resume_usb3_phy(dwc_usb3_pcd_t *pcd)
{
	unsigned int pipectl;

	pipectl = dwc_rd32(pcd->usb3_dev,
			   &pcd->usb3_dev->core_global_regs->gusb3pipectl[0]);
	pipectl &= ~DWC_PIPECTL_SUS_PHY_BIT;
	dwc_wr32(pcd->usb3_dev,
		 &pcd->usb3_dev->core_global_regs->gusb3pipectl[0], pipectl);
}

/**
 * Enable the Device to accept U1 control commands from the Host.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 */
void dwc_usb3_accept_u1(dwc_usb3_pcd_t *pcd)
{
	unsigned int dctl;

	dctl = dwc_rd32(pcd->usb3_dev, &pcd->dev_global_regs->dctl);
	dctl |= DWC_DCTL_ACCEPT_U1_EN_BIT;
	dwc_wr32(pcd->usb3_dev, &pcd->dev_global_regs->dctl, dctl);
}

/**
 * Enable the Device to accept U2 control commands from the Host.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 */
void dwc_usb3_accept_u2(dwc_usb3_pcd_t *pcd)
{
	unsigned int dctl;

	dctl = dwc_rd32(pcd->usb3_dev, &pcd->dev_global_regs->dctl);
	dctl |= DWC_DCTL_ACCEPT_U2_EN_BIT;
	dwc_wr32(pcd->usb3_dev, &pcd->dev_global_regs->dctl, dctl);
}

/**
 * Enable U1 sleep.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 */
void dwc_usb3_enable_u1(dwc_usb3_pcd_t *pcd)
{
	unsigned int dctl;

	dctl = dwc_rd32(pcd->usb3_dev, &pcd->dev_global_regs->dctl);
	dctl |= DWC_DCTL_INIT_U1_EN_BIT;
	dwc_wr32(pcd->usb3_dev, &pcd->dev_global_regs->dctl, dctl);
}

/**
 * Enable U2 sleep.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 */
void dwc_usb3_enable_u2(dwc_usb3_pcd_t *pcd)
{
	unsigned int dctl;

	dctl = dwc_rd32(pcd->usb3_dev, &pcd->dev_global_regs->dctl);
	dctl |= DWC_DCTL_INIT_U2_EN_BIT;
	dwc_wr32(pcd->usb3_dev, &pcd->dev_global_regs->dctl, dctl);
}

/**
 * Disable U1 sleep.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 */
void dwc_usb3_disable_u1(dwc_usb3_pcd_t *pcd)
{
	unsigned int dctl;

	dctl = dwc_rd32(pcd->usb3_dev, &pcd->dev_global_regs->dctl);
	dctl &= ~DWC_DCTL_INIT_U1_EN_BIT;
	dwc_wr32(pcd->usb3_dev, &pcd->dev_global_regs->dctl, dctl);
}

/**
 * Disable U2 sleep.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 */
void dwc_usb3_disable_u2(dwc_usb3_pcd_t *pcd)
{
	unsigned int dctl;

	dctl = dwc_rd32(pcd->usb3_dev, &pcd->dev_global_regs->dctl);
	dctl &= ~DWC_DCTL_INIT_U2_EN_BIT;
	dwc_wr32(pcd->usb3_dev, &pcd->dev_global_regs->dctl, dctl);
}

/**
 * Test whether U1 sleep is enabled.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 * @return    1 if enabled, 0 if not.
 */
int dwc_usb3_u1_enabled(dwc_usb3_pcd_t *pcd)
{
	unsigned int dctl;

	dctl = dwc_rd32(pcd->usb3_dev, &pcd->dev_global_regs->dctl);
	return !!(dctl & DWC_DCTL_INIT_U1_EN_BIT);
}

/**
 * Test whether U2 sleep is enabled.
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 * @return    1 if enabled, 0 if not.
 */
int dwc_usb3_u2_enabled(dwc_usb3_pcd_t *pcd)
{
	unsigned int dctl;

	dctl = dwc_rd32(pcd->usb3_dev, &pcd->dev_global_regs->dctl);
	return !!(dctl & DWC_DCTL_INIT_U2_EN_BIT);
}

/**
 * Clear 'eps_enabled' flag and 'ena_once' flags for all EPs, so EPs will get
 * completely reconfigured by SetConfig and SetInterface.
 */
void dwc_usb3_clr_eps_enabled(dwc_usb3_pcd_t *pcd)
{
	dwc_usb3_pcd_ep_t *ep;
	int i;

	pcd->eps_enabled = 0;

	for (i = 0; i < pcd->num_in_eps; i++) {
		ep = pcd->in_ep[i];
		if (ep)
			ep->dwc_ep.ena_once = 0;
	}

	for (i = 0; i < pcd->num_out_eps; i++) {
		ep = pcd->out_ep[i];
		if (ep)
			ep->dwc_ep.ena_once = 0;
	}
}

/**
 * This function is called when the SET_FEATURE TEST_MODE Setup packet
 * is sent from the host. The Device Control register is written with
 * the Test Mode bits set to the specified Test Mode. This is done as
 * a tasklet so that the "Status" phase of the control transfer
 * completes before transmitting the TEST packets.
 */
/*
 * @todo This has not been tested since the tasklet struct was put
 * into the PCD struct!
 *
 */
void dwc_usb3_do_test_mode(unsigned long data)
{
	dwc_usb3_pcd_t *pcd = (dwc_usb3_pcd_t *)data;
	int test_mode = pcd->test_mode;
	unsigned int dctl;

	dwc_warn1(pcd->usb3_dev,
		  "%s() has not been tested since being rewritten!\n",
		  __func__);

	dctl = dwc_rd32(pcd->usb3_dev, &pcd->dev_global_regs->dctl);
	dctl &= ~DWC_DCTL_TSTCTL_BITS;

	switch (test_mode) {
	case 1: // TEST_J
		dctl |= 1 << DWC_DCTL_TSTCTL_SHIFT;
		break;

	case 2: // TEST_K
		dctl |= 2 << DWC_DCTL_TSTCTL_SHIFT;
		break;

	case 3: // TEST_SE0_NAK
		dctl |= 3 << DWC_DCTL_TSTCTL_SHIFT;
		break;

	case 4: // TEST_PACKET
		dctl |= 4 << DWC_DCTL_TSTCTL_SHIFT;
		break;

	case 5: // TEST_FORCE_ENABLE
		dctl |= 5 << DWC_DCTL_TSTCTL_SHIFT;
		break;
	}

	dwc_wr32(pcd->usb3_dev, &pcd->dev_global_regs->dctl, dctl);
}

/**
 * This function calculates the number of IN EPs (excluding EP0)
 * using GHWPARAMS3 register values
 *
 * @param dev Programming view of DWC_usb3 controller.
 */
static int calc_num_in_eps(dwc_usb3_device_t *dev)
{
	unsigned int num_in_eps = (dev->hwparams3 >> DWC_HWP3_NUM_IN_EPS_SHIFT) &
		(DWC_HWP3_NUM_IN_EPS_BITS >> DWC_HWP3_NUM_IN_EPS_SHIFT);

	return num_in_eps - 1;
}

/**
 * This function calculates the number of OUT EPs (excluding EP0)
 * using GHWPARAMS3 register values
 *
 * @param dev Programming view of DWC_usb3 controller.
 */
static int calc_num_out_eps(dwc_usb3_device_t *dev)
{
	unsigned int num_eps = (dev->hwparams3 >> DWC_HWP3_NUM_EPS_SHIFT) &
		(DWC_HWP3_NUM_EPS_BITS >> DWC_HWP3_NUM_EPS_SHIFT);
	unsigned int num_in_eps = (dev->hwparams3 >> DWC_HWP3_NUM_IN_EPS_SHIFT) &
		(DWC_HWP3_NUM_IN_EPS_BITS >> DWC_HWP3_NUM_IN_EPS_SHIFT);

	return num_eps - num_in_eps - 1;
}

/**
 * This function is called to initialize the DWC_usb3 CSR data
 * structures. The register addresses in the device structures
 * are initialized from the base address supplied by the caller.
 * The calling function must make the OS calls to get the base
 * address of the DWC_usb3 controller registers. The core_params
 * argument holds the parameters that specify how the core should
 * be configured.
 *
 * @param dev         Programming view of DWC_usb3 controller.
 * @param base        Base address of DWC_usb3 core registers.
 * @param core_params Pointer to the core configuration parameters.
 */
int dwc_usb3_common_init(dwc_usb3_device_t *dev, volatile unsigned char   *base,
			 dwc_usb3_core_params_t *core_params)
{
	dwc_usb3_pcd_t *pcd;
	unsigned int temp;

	dwc_debug3(dev, "%s(%p,%p)\n", __func__, base, core_params);

	dev->core_params = core_params;
	dev->core_global_regs = (dwc_usb3_core_global_regs_t   *)
					(base + DWC_CORE_GLOBAL_REG_OFFSET);

#ifdef COSIM
	/* scramble-off, scaledown */
	dwc_wr32(dev, &dev->core_global_regs->gctl, 0x38);
#endif

	pcd = &dev->pcd;

	pcd->dev_global_regs = (dwc_usb3_dev_global_regs_t   *)
					(base + DWC_DEV_GLOBAL_REG_OFFSET);
	pcd->in_ep_regs = (dwc_usb3_dev_ep_regs_t   *)
					(base + DWC_DEV_IN_EP_REG_OFFSET);
	pcd->out_ep_regs = (dwc_usb3_dev_ep_regs_t   *)
					(base + DWC_DEV_OUT_EP_REG_OFFSET);

	/*
	 * Store the contents of the hardware configuration registers here for
	 * easy access later.
	 */
	dev->hwparams0 = dwc_rd32(dev, &dev->core_global_regs->ghwparams0);
	dev->hwparams1 = dwc_rd32(dev, &dev->core_global_regs->ghwparams1);
	dev->hwparams2 = dwc_rd32(dev, &dev->core_global_regs->ghwparams2);
	dev->hwparams3 = dwc_rd32(dev, &dev->core_global_regs->ghwparams3);
	dev->hwparams4 = dwc_rd32(dev, &dev->core_global_regs->ghwparams4);
	dev->hwparams5 = dwc_rd32(dev, &dev->core_global_regs->ghwparams5);
	dev->hwparams6 = dwc_rd32(dev, &dev->core_global_regs->ghwparams6);
	dev->hwparams7 = dwc_rd32(dev, &dev->core_global_regs->ghwparams7);
	dev->hwparams8 = dwc_rd32(dev, &dev->core_global_regs->ghwparams8);

	temp = dwc_rd32(dev, &pcd->dev_global_regs->dcfg);
	cil_debug( "dcfg=%08x\n", temp);

	cil_debug( "mode=%0x\n",
		   (dev->hwparams0 >> DWC_HWP0_MODE_SHIFT) &
		   (DWC_HWP0_MODE_BITS >> DWC_HWP0_MODE_SHIFT));
	cil_debug( "num_ep=%d\n",
		   (dev->hwparams3 >> DWC_HWP3_NUM_EPS_SHIFT) &
		   (DWC_HWP3_NUM_EPS_BITS >> DWC_HWP3_NUM_EPS_SHIFT));
	cil_debug( "num_in_ep=%d\n",
		   (dev->hwparams3 >> DWC_HWP3_NUM_IN_EPS_SHIFT) &
		   (DWC_HWP3_NUM_IN_EPS_BITS >> DWC_HWP3_NUM_IN_EPS_SHIFT));
	cil_debug( "dfq_fifo_depth=%d\n",
	       (dev->hwparams5 >> DWC_HWP5_DFQ_FIFO_DEPTH_SHIFT) &
	       (DWC_HWP5_DFQ_FIFO_DEPTH_BITS >> DWC_HWP5_DFQ_FIFO_DEPTH_SHIFT));
	cil_debug( "dwq_fifo_depth=%d\n",
	       (dev->hwparams5 >> DWC_HWP5_DWQ_FIFO_DEPTH_SHIFT) &
	       (DWC_HWP5_DWQ_FIFO_DEPTH_BITS >> DWC_HWP5_DWQ_FIFO_DEPTH_SHIFT));
	cil_debug( "txq_fifo_depth=%d\n",
	       (dev->hwparams5 >> DWC_HWP5_TXQ_FIFO_DEPTH_SHIFT) &
	       (DWC_HWP5_TXQ_FIFO_DEPTH_BITS >> DWC_HWP5_TXQ_FIFO_DEPTH_SHIFT));
	cil_debug( "rxq_fifo_depth=%d\n",
	       (dev->hwparams5 >> DWC_HWP5_RXQ_FIFO_DEPTH_SHIFT) &
	       (DWC_HWP5_RXQ_FIFO_DEPTH_BITS >> DWC_HWP5_RXQ_FIFO_DEPTH_SHIFT));

	/* Initialize parameters from Hardware configuration registers. */
	dev->pcd.num_in_eps = calc_num_in_eps(dev);
	dev->pcd.num_out_eps = calc_num_out_eps(dev);

	return 0;
}

/**
 * This function frees any structures allocated by dwc_usb3_common_init().
 *
 * @param dev Programming view of DWC_usb3 controller.
 */
void dwc_usb3_common_remove(dwc_usb3_device_t *dev)
{
}

/**
 * This function returns the contents of the SNPSID register.
 *
 * @param dev      Programming view of DWC_usb3 controller.
 * @param addr_ofs Offset to the Device registers in the CSR space, needed
 *                 because this function can be called early, before the normal
 *                 register access routines are functional.
 * @return         The SNPSID value.
 */
unsigned int dwc_usb3_get_snpsid(dwc_usb3_device_t *dev, unsigned int addr_ofs)
{
	unsigned int snpsid;

	snpsid = dwc_rd32(dev, dev->base + addr_ofs + 0x120);
	return snpsid;
}

/**
 * This function reads the core global registers and prints them
 *
 * @param dev Programming view of DWC_usb3 controller.
 */
void dwc_usb3_dump_global_registers(dwc_usb3_device_t *dev)
{
	volatile unsigned int   *addr;

	dwc_print0(dev, "Core Global Registers\n");
	addr = &dev->core_global_regs->gsbuscfg0;
	dwc_print2(dev, "GSBUSCFG0	@0x%08lx : 0x%08x\n",
		   (unsigned long)addr, dwc_rd32(dev, addr));
	addr = &dev->core_global_regs->gsbuscfg1;
	dwc_print2(dev, "GSBUSCFG1	@0x%08lx : 0x%08x\n",
		   (unsigned long)addr, dwc_rd32(dev, addr));
	addr = &dev->core_global_regs->gusb2phycfg[0];
	dwc_print2(dev, "USB2PHYCFG0	@0x%08lx : 0x%08x\n",
		   (unsigned long)addr, dwc_rd32(dev, addr));
	addr = &dev->core_global_regs->gevten;
	dwc_print2(dev, "GEVTEN		@0x%08lx : 0x%08x\n",
		   (unsigned long)addr, dwc_rd32(dev, addr));
	addr = &dev->core_global_regs->grxfifosiz[0];
	dwc_print2(dev, "GRXFIFOSIZ0	@0x%08lx : 0x%08x\n",
		   (unsigned long)addr, dwc_rd32(dev, addr));
	addr = &dev->core_global_regs->gtxfifosiz[0];
	dwc_print2(dev, "GTXFIFOSIZ0	@0x%08lx : 0x%08x\n",
		   (unsigned long)addr, dwc_rd32(dev, addr));
	addr = &dev->core_global_regs->gtxfifosiz[1];
	dwc_print2(dev, "GTXFIFOSIZ1	@0x%08lx : 0x%08x\n",
		   (unsigned long)addr, dwc_rd32(dev, addr));
	addr = &dev->core_global_regs->gtxfifosiz[2];
	dwc_print2(dev, "GTXFIFOSIZ2	@0x%08lx : 0x%08x\n",
		   (unsigned long)addr, dwc_rd32(dev, addr));
	addr = &dev->core_global_regs->gtxfifosiz[3];
	dwc_print2(dev, "GTXFIFOSIZ3	@0x%08lx : 0x%08x\n",
		   (unsigned long)addr, dwc_rd32(dev, addr));
	addr = &dev->core_global_regs->gusb2i2cctl[0];
	dwc_print2(dev, "GUSB2I2CCTL0	@0x%08lx : 0x%08x\n",
		   (unsigned long)addr, dwc_rd32(dev, addr));
	addr = &dev->core_global_regs->ggpio;
	dwc_print2(dev, "GGPIO		@0x%08lx : 0x%08x\n",
		   (unsigned long)addr, dwc_rd32(dev, addr));
	addr = &dev->core_global_regs->guid;
	dwc_print2(dev, "GUID		@0x%08lx : 0x%08x\n",
		   (unsigned long)addr, dwc_rd32(dev, addr));
	addr = &dev->core_global_regs->gsnpsid;
	dwc_print2(dev, "GSNPSID		@0x%08lx : 0x%08x\n",
		   (unsigned long)addr, dwc_rd32(dev, addr));
}

/**
 * This functions reads the device registers and prints them
 *
 * @param pcd Programming view of DWC_usb3 peripheral controller.
 */
void dwc_usb3_dump_dev_registers(dwc_usb3_pcd_t *pcd)
{
	volatile unsigned int   *addr;

	dwc_print0(pcd->usb3_dev, "Device Global Registers\n");
	addr = &pcd->dev_global_regs->dcfg;
	dwc_print2(pcd->usb3_dev, "DCFG		@0x%08lx : 0x%08x\n",
		   (unsigned long)addr, dwc_rd32(pcd->usb3_dev, addr));
	addr = &pcd->dev_global_regs->dctl;
	dwc_print2(pcd->usb3_dev, "DCTL		@0x%08lx : 0x%08x\n",
		   (unsigned long)addr, dwc_rd32(pcd->usb3_dev, addr));
	addr = &pcd->dev_global_regs->dsts;
	dwc_print2(pcd->usb3_dev, "DSTS		@0x%08lx : 0x%08x\n",
		   (unsigned long)addr, dwc_rd32(pcd->usb3_dev, addr));
}

void dwc_usb3_set_tx_fifo_size(dwc_usb3_device_t *dev, int sz[DWC_MAX_TX_FIFOS])
{
	dwc_usb3_core_global_regs_t   *global_regs =
						dev->core_global_regs;
	int i, ram_width, ram_depth, size, prev_start, txsz[DWC_MAX_TX_FIFOS];

	ram_width = ((dev->hwparams0 >> DWC_HWP0_MDWIDTH_SHIFT) &
		     (DWC_HWP0_MDWIDTH_BITS >> DWC_HWP0_MDWIDTH_SHIFT))
		    / 8;
	ram_depth = ((dev->hwparams7 >> DWC_HWP7_RAM1_DEPTH_SHIFT) &
		     (DWC_HWP7_RAM1_DEPTH_BITS >> DWC_HWP7_RAM1_DEPTH_SHIFT))
		    * ram_width;
	size = dwc_rd32(dev, &global_regs->gtxfifosiz[0]);
	prev_start = (size >> DWC_FIFOSZ_STARTADDR_SHIFT) &
		(DWC_FIFOSZ_STARTADDR_BITS >> DWC_FIFOSZ_STARTADDR_SHIFT);

	for (i = 0; i < DWC_MAX_TX_FIFOS; i++) {
		size = sz[i];

		if (i == 0 && size && size < 512 + 2 * ram_width) {
			dwc_print1(dev, "Requested Tx FIFO %d size too small\n",
				   i);
			dwc_print0(dev, "Not setting Tx FIFO sizes\n");
			goto txerr;
		}

		if (!size) {
			/* Default to 512 for EP0, 1K for others */
			size = i ? 1024 : 512;
			size += 2 * ram_width;
		}

		size = (size + ram_width - 1) & ~(ram_width - 1);
		dwc_debug3(dev,
			   "Tx FIFO %d size = %d bytes out of %d available\n",
			   i, size, ram_depth);
		if (size > ram_depth) {
			dwc_print1(dev, "Requested Tx FIFO %d size too large\n",
				   i);
			dwc_print0(dev, "Not setting Tx FIFO sizes\n");
			goto txerr;
		}

		txsz[i] = size;
		ram_depth -= size;
	}

	for (i = 0; i < DWC_MAX_TX_FIFOS; i++) {
		size = txsz[i];
		dwc_debug2(dev, "Setting GTXFIFOSIZ%d = 0x%08x\n", i,
			   ((size / ram_width) << DWC_FIFOSZ_DEPTH_SHIFT) |
			   (prev_start << DWC_FIFOSZ_STARTADDR_SHIFT));
		dwc_wr32(dev, &global_regs->gtxfifosiz[i],
			 ((size / ram_width) << DWC_FIFOSZ_DEPTH_SHIFT) |
			 (prev_start << DWC_FIFOSZ_STARTADDR_SHIFT));
		dwc_debug2(dev, "GTXFIFOSIZ%d = 0x%08x\n", i,
			   dwc_rd32(dev, &global_regs->gtxfifosiz[i]));
		prev_start += size / ram_width;
	}
txerr:
	return;
}

void dwc_usb3_set_rx_fifo_size(dwc_usb3_device_t *dev, int size)
{
	dwc_usb3_core_global_regs_t   *global_regs =
						dev->core_global_regs;
	int ram_width, ram_depth, prev_start, sz;

	ram_width = ((dev->hwparams0 >> DWC_HWP0_MDWIDTH_SHIFT) &
		     (DWC_HWP0_MDWIDTH_BITS >> DWC_HWP0_MDWIDTH_SHIFT))
		    / 8;
	ram_depth = ((dev->hwparams7 >> DWC_HWP7_RAM2_DEPTH_SHIFT) &
		     (DWC_HWP7_RAM2_DEPTH_BITS >> DWC_HWP7_RAM2_DEPTH_SHIFT))
		    * ram_width;
	sz = dwc_rd32(dev, &global_regs->grxfifosiz[0]);
	prev_start = (sz >> DWC_FIFOSZ_STARTADDR_SHIFT) &
		     (DWC_FIFOSZ_STARTADDR_BITS >> DWC_FIFOSZ_STARTADDR_SHIFT);

	if (size < 512 + 24 + 16 + (ram_width == 16 ? 24 : 0)) {
		dwc_print0(dev, "Requested Rx FIFO size too small\n");
		dwc_print0(dev, "Not setting Rx FIFO size\n");
		goto rxerr;
	}

	size = (size + ram_width - 1) & ~(ram_width - 1);
	if (size > ram_depth) {
		dwc_print0(dev, "Requested Rx FIFO size too large\n");
		dwc_print0(dev, "Not setting Rx FIFO size\n");
		goto rxerr;
	}

	_dprintf( "Setting GRXFIFOSIZ0 = 0x%08x\n",
		   ((size / ram_width) << DWC_FIFOSZ_DEPTH_SHIFT) |
		   (prev_start << DWC_FIFOSZ_STARTADDR_SHIFT));
	dwc_wr32(dev, &global_regs->grxfifosiz[0],
		 ((size / ram_width) << DWC_FIFOSZ_DEPTH_SHIFT) |
		 (prev_start << DWC_FIFOSZ_STARTADDR_SHIFT));
	_dprintf( "GRXFIFOSIZ0 = 0x%08x\n",
		   dwc_rd32(dev, &global_regs->grxfifosiz[0]));
	dwc_debug2(dev, "Rx FIFO size = %d bytes out of %d available\n",
		   size, ram_depth);
rxerr:
	return;
}

/**
 * This function initializes the DWC_usb3 controller registers.
 *
 * @param dev        Programming view of DWC_usb3 controller.
 * @param soft_reset If true a soft reset of the core is performed.
 *
 */
void dwc_usb3_core_device_init(dwc_usb3_device_t *dev, int soft_reset,
			       int restore)
{
	dwc_usb3_core_global_regs_t   *global_regs =
						dev->core_global_regs;
	dwc_usb3_pcd_t *pcd = &dev->pcd;
	int ram_width, ram_depth, size, txsz[DWC_MAX_TX_FIFOS];
	unsigned int temp;

	dwc_print1(dev, "%s()\n", __func__);

	/*
	 * TODO Workaround: PCD can't handle soft reset during HNP. RTL issue
	 * will be fixed. Skip the reset when called with soft_reset=0. When
	 * not configured for OTG do the reset unconditionally.
	 */
	if (soft_reset) {
		/* Soft-reset the core */
		temp = dwc_rd32(dev, &pcd->dev_global_regs->dctl);
		temp &= ~DWC_DCTL_RUN_STOP_BIT;
		temp |= DWC_DCTL_CSFT_RST_BIT;
		dwc_wr32(dev, &pcd->dev_global_regs->dctl, temp);

		/* Wait for core to come out of reset */
		do {
			dwc_msleep(dev, 1);
			temp = dwc_rd32(dev, &pcd->dev_global_regs->dctl);
		} while (temp & DWC_DCTL_CSFT_RST_BIT);

		/* Wait for at least 3 PHY clocks */
		dwc_msleep(dev, 1);
	}

	pcd->link_state = 0;
	pcd->wkup_rdy = 0;

	/*
	 * @Note: The following are specific to the Synopsys FPGA demo board!
	 */
	if (dev->core_params->phy == 2) {		// TI Phy
		/* Set Turnaround Time = 9 (8-bit UTMI+ / ULPI) */
		temp = dwc_rd32(dev, &global_regs->gusb2phycfg[0]);
		temp &= ~DWC_USB2PHYCFG_USB_TRD_TIM_BITS;
		temp |= 9 << DWC_USB2PHYCFG_USB_TRD_TIM_SHIFT;
		dwc_wr32(dev, &global_regs->gusb2phycfg[0], temp);

		///* Force no PHY suspend */
		//dwc_wr32(dev, dev->base + 0x80004, 0x8);

	} else if (dev->core_params->phy == 1) {	// SNPS Phy
		/* Set Turnaround Time = 9 (8-bit UTMI+ / ULPI) */
		temp = dwc_rd32(dev, &global_regs->gusb2phycfg[0]);
		temp &= ~DWC_USB2PHYCFG_USB_TRD_TIM_BITS;
		temp |= 9 << DWC_USB2PHYCFG_USB_TRD_TIM_SHIFT;
		dwc_wr32(dev, &global_regs->gusb2phycfg[0], temp);

	} else {					// RocketIO Phy
		if (dev->core_params->usb2mode == 0) {
			/* Set rx-eq, differential swing */
			dwc_wr32(dev, dev->base + 0x80008, 0x41);
#ifdef LECROY
			/* Rx-detect for LeCroy */
			dwc_wr32(dev, dev->base + 0x80004, 0x200);
#else
			dwc_wr32(dev, dev->base + 0x80004, 0);
#endif
		}
	}

	/*
	 * NOTE: The following code for setting the FIFO sizes only works for
	 * cores configured with the 3 RAM option. Setting FIFO sizes for the
	 * 2 RAM option is not implemented.
	 */

	/*
	 * Set Tx FIFO sizes
	 */
	ram_width = ((dev->hwparams0 >> DWC_HWP0_MDWIDTH_SHIFT) &
		     (DWC_HWP0_MDWIDTH_BITS >> DWC_HWP0_MDWIDTH_SHIFT))
		    / 8;
	ram_depth = ((dev->hwparams7 >> DWC_HWP7_RAM1_DEPTH_SHIFT) &
		     (DWC_HWP7_RAM1_DEPTH_BITS >> DWC_HWP7_RAM1_DEPTH_SHIFT))
		    * ram_width;
	dwc_debug2(dev, "RAM width:%d RAM1 depth:%d\n", ram_width, ram_depth);
	size = dwc_rd32(dev, &global_regs->gtxfifosiz[0]);
	cil_debug( "Initial GTXFIFOSIZ0 = 0x%08x\n", size);
	size = dwc_rd32(dev, &global_regs->gtxfifosiz[1]);
	cil_debug( "Initial GTXFIFOSIZ1 = 0x%08x\n", size);
	size = dwc_rd32(dev, &global_regs->gtxfifosiz[2]);
	cil_debug( "Initial GTXFIFOSIZ2 = 0x%08x\n", size);
	size = dwc_rd32(dev, &global_regs->gtxfifosiz[3]);
	cil_debug( "Initial GTXFIFOSIZ3 = 0x%08x\n", size);

	/* Only set if non-default Tx FIFO sizes were requested */
	if (dev->core_params->tx0 || dev->core_params->tx1 ||
	    dev->core_params->tx2 || dev->core_params->tx3) {
		txsz[0] = dev->core_params->tx0;
		txsz[1] = dev->core_params->tx1;
		txsz[2] = dev->core_params->tx2;
		txsz[3] = dev->core_params->tx3;
		dwc_usb3_set_tx_fifo_size(dev, txsz);
	}

	/*
	 * Set Rx FIFO size
	 */
	ram_depth = ((dev->hwparams7 >> DWC_HWP7_RAM2_DEPTH_SHIFT) &
		     (DWC_HWP7_RAM2_DEPTH_BITS >> DWC_HWP7_RAM2_DEPTH_SHIFT))
		    * ram_width;
	cil_debug( "RAM2 depth:%d\n", ram_depth);
	size = dwc_rd32(dev, &global_regs->grxfifosiz[0]);
	cil_debug( "Initial GRXFIFOSIZ0 = 0x%08x\n", size);
	size = dev->core_params->rx;

	/* Only set if non-default Rx FIFO size was requested */
	if (size)
		dwc_usb3_set_rx_fifo_size(dev, size);

	temp = dwc_rd32(dev, &global_regs->gctl);
	temp &= ~(DWC_GCTL_PRT_CAP_DIR_BITS | DWC_GCTL_SCALE_DOWN_SHIFT);

#ifdef DWC_STAR_9000468158_WORKAROUND
	temp |= DWC_GCTL_U2RSTECN_BIT;
#endif

#ifdef CONFIG_USB_OTG_DWC
	temp |= DWC_GCTL_PRT_CAP_OTG << DWC_GCTL_PRT_CAP_DIR_SHIFT;
#else
	temp |= DWC_GCTL_PRT_CAP_DEVICE << DWC_GCTL_PRT_CAP_DIR_SHIFT;
#endif

#ifdef COSIM
	/* Scale down, disable scrambling */
	temp |= (3 << DWC_GCTL_SCALE_DOWN_SHIFT) | DWC_GCTL_DIS_SCRAMBLE_BIT;
#else
	if (dev->core_params->phy == 2) {		// TI Phy
		/* Set power down scale, disable scrambling */
		temp |= (0x270 << DWC_GCTL_PWR_DN_SCALE_SHIFT) /*|
			DWC_GCTL_DIS_SCRAMBLE_BIT*/;
	} else if (dev->core_params->phy == 1) {	// SNPS Phy
		/* Set LFPS filter */
		dwc_wr32(dev, &global_regs->gusb3pipectl[0],
			 DWC_PIPECTL_LFPS_FILTER_BIT |
			 (1 << DWC_PIPECTL_TX_DEMPH_SHIFT));

		/* Set power down scale, disable scrambling */
		temp |= (0x270 << DWC_GCTL_PWR_DN_SCALE_SHIFT) /*|
			DWC_GCTL_DIS_SCRAMBLE_BIT*/;
	} else {					// RocketIO Phy
		/* Set power down scale, disable scrambling */
		temp |= (0x1e84 << DWC_GCTL_PWR_DN_SCALE_SHIFT) |
			DWC_GCTL_DEBUG_ATTACH_BIT | DWC_GCTL_DIS_SCRAMBLE_BIT;
	}
#endif
	dwc_wr32(dev, &global_regs->gctl, temp);

	/* Initialize the Event Buffer registers */
	dwc_usb3_init_eventbuf(dev, 0, dev->event_buf[0], DWC_EVENT_BUF_SIZE,
			       dev->event_buf_dma[0]);
	dev->event_ptr[0] = dev->event_buf[0];

	/* If forcing to a USB2 mode was requested */
	if (dev->core_params->usb2mode == 1) {
		/* Set speed to Full */
		temp = dwc_rd32(dev, &pcd->dev_global_regs->dcfg);
		temp &= ~(DWC_DCFG_DEVSPD_BITS << DWC_DCFG_DEVSPD_SHIFT);
		temp |= DWC_SPEED_FS_PHY_30MHZ_OR_60MHZ
						<< DWC_DCFG_DEVSPD_SHIFT;
		dwc_wr32(dev, &pcd->dev_global_regs->dcfg, temp);

	} else if (dev->core_params->usb2mode == 2) {
		/* Set speed to High */
		temp = dwc_rd32(dev, &pcd->dev_global_regs->dcfg);
		temp &= ~(DWC_DCFG_DEVSPD_BITS << DWC_DCFG_DEVSPD_SHIFT);
		temp |= DWC_SPEED_HS_PHY_30MHZ_OR_60MHZ
						<< DWC_DCFG_DEVSPD_SHIFT;
		dwc_wr32(dev, &pcd->dev_global_regs->dcfg, temp);

	} else {
		/* Set speed to Super */
		temp = dwc_rd32(dev, &pcd->dev_global_regs->dcfg);
		temp &= ~(DWC_DCFG_DEVSPD_BITS << DWC_DCFG_DEVSPD_SHIFT);
		temp |= DWC_SPEED_SS_PHY_125MHZ_OR_250MHZ
						<< DWC_DCFG_DEVSPD_SHIFT;
		dwc_wr32(dev, &pcd->dev_global_regs->dcfg, temp);
	}

	/* If LPM enable was requested */
	if (dev->core_params->lpmctl) {
		/* Set LPMCap bit */
		temp = dwc_rd32(dev, &pcd->dev_global_regs->dcfg);
		temp |= DWC_DCFG_LPM_CAP_BIT;
		dwc_wr32(dev, &pcd->dev_global_regs->dcfg, temp);

		if (dev->core_params->lpmctl > 1) {
			/* Set AppL1Res bit */
			temp = dwc_rd32(dev, &pcd->dev_global_regs->dctl);
			temp |= DWC_DCTL_APP_L1_RES_BIT;
			dwc_wr32(dev, &pcd->dev_global_regs->dctl, temp);
		}
	}

	/* If non-default NUMP was requested */
	if (dev->core_params->nump > 0 && dev->core_params->nump <= 16) {
		/* Set NUMP */
		temp = dwc_rd32(dev, &pcd->dev_global_regs->dcfg);
		temp &= ~DWC_DCFG_NUM_RCV_BUF_BITS;
		temp |= dev->core_params->nump << DWC_DCFG_NUM_RCV_BUF_SHIFT;
		dwc_wr32(dev, &pcd->dev_global_regs->dcfg, temp);
	}

	if (!restore) {
		/* Set device address to 0 */
		dwc_usb3_set_address(pcd, 0);
	}

	/* Enable hibernation if supported */
	if (dev->core_params->hibernate &&
	    (dev->hwparams1 & DWC_HWP1_EN_PWROPT_BITS) ==
	    (DWC_EN_PWROPT_HIBERNATION << DWC_HWP1_EN_PWROPT_SHIFT)) {
		/* Enable global hibernation bit */
		temp = dwc_rd32(dev, &global_regs->gctl);
		temp |= DWC_GCTL_GBL_HIBER_EN_BIT;
		if (dev->core_params->clkgatingen)
			temp &= ~DWC_GCTL_DSBL_CLCK_GTNG_BIT;
		else
			temp |= DWC_GCTL_DSBL_CLCK_GTNG_BIT;
		dwc_wr32(dev, &global_regs->gctl, temp);

		if (dev->core_params->lpmctl) {
			/* Set L1 hibernation values */
			temp = dwc_rd32(dev, &pcd->dev_global_regs->dctl);
			temp &= ~DWC_DCTL_HIRD_THR_BITS;
			//temp |= 0x1e << DWC_DCTL_HIRD_THR_SHIFT;
			temp |= 0x1f << DWC_DCTL_HIRD_THR_SHIFT;

			/* Enable L1 hibernation */
			temp |= DWC_DCTL_L1_HIBER_EN_BIT;
			dwc_wr32(dev, &pcd->dev_global_regs->dctl, temp);
		}

		if (!restore) {
			/* Issue Set Scratchpad Buffer Array command */
			dwc_usb3_set_scratchpad_buf_array(pcd,
					pcd->hiber_scratchpad_array_dma);
		}

		/* Set GUSB2PHYCFG[6] (suspend 2.0 phy) */
		temp = dwc_rd32(dev, &global_regs->gusb2phycfg[0]);
		temp |= DWC_USB2PHYCFG_SUS_PHY_BIT;
		if (dev->core_params->lpmctl)
			temp |= DWC_USB2PHYCFG_ENBL_SLP_M_BIT;
		dwc_wr32(dev, &global_regs->gusb2phycfg[0], temp);

		/* Set GUSB3PIPECTL[17] (suspend SS phy) */
		temp = dwc_rd32(dev, &global_regs->gusb3pipectl[0]);
		temp |= DWC_PIPECTL_SUS_PHY_BIT;
		dwc_wr32(dev, &global_regs->gusb3pipectl[0], temp);
	} else {
		if (dev->core_params->phyctl) {
			/* Enable Phy suspend */
			dwc_usb3_suspend_usb3_phy(pcd);
			dwc_usb3_suspend_usb2_phy(pcd);
		} else {
			/* Disable Phy suspend */
			dwc_usb3_resume_usb3_phy(pcd);
			dwc_usb3_resume_usb2_phy(pcd);
		}
	}

#ifndef CONFIG_USB_OTG_DWC
	/* Enable Global and Device interrupts */
	//dwc_usb3_enable_device_interrupts(dev);
#endif
	/* Activate EP0 */
	dwc_usb3_ep0_activate(pcd, restore);

	/* Start EP0 to receive SETUP packets */
	dwc_usb3_ep0_out_start(pcd);

	/* Enable EP0-OUT/IN in DALEPENA register */
	dwc_wr32(dev, &pcd->dev_global_regs->dalepena, 3);
	pcd->eps_enabled = 0;

#ifndef CONFIG_USB_OTG_DWC
	/* Set Run/Stop bit, and Keep-Connect bit if hibernation enabled */
	temp = dwc_rd32(dev, &pcd->dev_global_regs->dctl);
	temp |= DWC_DCTL_RUN_STOP_BIT;
	if (dev->core_params->hibernate &&
	    (dev->hwparams1 & DWC_HWP1_EN_PWROPT_BITS) ==
	    (DWC_EN_PWROPT_HIBERNATION << DWC_HWP1_EN_PWROPT_SHIFT))
		temp |= DWC_DCTL_KEEP_CONNECT_BIT;
	dwc_wr32(dev, &pcd->dev_global_regs->dctl, temp);
#endif
}

/**
 * This function deinitializes the DWC_usb3 controller registers,
 * as needed.
 *
 * @param dev Programming view of DWC_usb3 controller.
 *
 */
void dwc_usb3_core_device_remove(dwc_usb3_device_t *dev)
{
	dwc_usb3_core_global_regs_t   *global_regs =
						dev->core_global_regs;
	dwc_usb3_pcd_t *pcd = &dev->pcd;
	unsigned int temp;

#if 0		/* 012.08.13 */
	if (atomic_read(&dev->hibernate) >= DWC_HIBER_SLEEPING)
		return;
#endif /* 0 */

	/* Clear the Run/Stop and Keep-Connect bits */
	temp = dwc_rd32(dev, &pcd->dev_global_regs->dctl);
	temp &= ~(DWC_DCTL_RUN_STOP_BIT | DWC_DCTL_KEEP_CONNECT_BIT);
	dwc_wr32(dev, &pcd->dev_global_regs->dctl, temp);

	/* Disable device interrupts */
	dwc_wr32(dev, &pcd->dev_global_regs->devten, 0);

	/* Disable hibernation if supported */
	if (dev->core_params->hibernate &&
	    (dev->hwparams1 & DWC_HWP1_EN_PWROPT_BITS) ==
	    (DWC_EN_PWROPT_HIBERNATION << DWC_HWP1_EN_PWROPT_SHIFT)) {
		/* Clear GUSB3PIPECTL[17] (suspend SS phy) */
		temp = dwc_rd32(dev, &global_regs->gusb3pipectl[0]);
		temp &= ~DWC_PIPECTL_SUS_PHY_BIT;
		dwc_wr32(dev, &global_regs->gusb3pipectl[0], temp);

		/* Clear GUSB2PHYCFG[6] (suspend 2.0 phy) */
		temp = dwc_rd32(dev, &global_regs->gusb2phycfg[0]);
		temp &= ~DWC_USB2PHYCFG_SUS_PHY_BIT;
		dwc_wr32(dev, &global_regs->gusb2phycfg[0], temp);

		/* Disable L1 hibernation */
		temp = dwc_rd32(dev, &pcd->dev_global_regs->dctl);
		temp &= ~DWC_DCTL_L1_HIBER_EN_BIT;
		dwc_wr32(dev, &pcd->dev_global_regs->dctl, temp);

		/* Disable global hibernation bit */
		temp = dwc_rd32(dev, &global_regs->gctl);
		temp &= ~DWC_GCTL_GBL_HIBER_EN_BIT;
		temp |= DWC_GCTL_DSBL_CLCK_GTNG_BIT;
		dwc_wr32(dev, &global_regs->gctl, temp);
	}
}
