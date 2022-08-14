/* ==========================================================================
 * $File: //dwh/usb_iip/dev/software/DWC_usb3/driver/cil.h $
 * $Revision: #33 $
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

#ifndef _DWC_CIL_H_
#define _DWC_CIL_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 * This file contains the interface to the Core Interface Layer.
 */

/** @name CIL Initialization Functions
 * The following functions support initialization of the CIL driver component
 * and the DWC_usb3 controller.
 */
/** @{ */
extern int dwc_usb3_common_init(dwc_usb3_device_t *dev,
				volatile unsigned char   *base,
				dwc_usb3_core_params_t *core_params);
extern void dwc_usb3_common_remove(dwc_usb3_device_t *dev);
extern void dwc_usb3_core_device_init(dwc_usb3_device_t *dev, int soft_reset,
				      int restore);
extern void dwc_usb3_core_device_remove(dwc_usb3_device_t *dev);
/** @}*/

/** @name Peripheral CIL Functions
 * The following functions support managing the DWC_usb3 controller in
 * peripheral mode.
 */
/** @{ */
extern void dwc_usb3_fill_desc(dwc_usb3_dma_desc_t *desc, dwc_dma_t dma_addr,
			       unsigned int dma_len, unsigned int stream, unsigned int type,
			       unsigned int ctrlbits, int own);
extern void dwc_usb3_start_desc_chain(dwc_usb3_dma_desc_t *desc);
extern void dwc_usb3_end_desc_chain(dwc_usb3_dma_desc_t *desc);
extern void dwc_usb3_enable_desc(dwc_usb3_dma_desc_t *desc);
extern int dwc_usb3_xmit_fn_remote_wake(dwc_usb3_pcd_t *pcd, unsigned int intf);
extern int dwc_usb3_set_scratchpad_buf_array(dwc_usb3_pcd_t *pcd,
					     dwc_dma_t dma_addr);
extern int dwc_usb3_dep_cfg(dwc_usb3_pcd_t *pcd,
			    dwc_usb3_dev_ep_regs_t   *ep_reg,
			    unsigned int depcfg0, unsigned int depcfg1,
			    unsigned int depcfg2);
extern int dwc_usb3_xmit_host_role_request(dwc_usb3_pcd_t *pcd, unsigned int param);
extern int dwc_usb3_dep_xfercfg(dwc_usb3_pcd_t *pcd,
				dwc_usb3_dev_ep_regs_t   *ep_reg,
				unsigned int depstrmcfg);
extern unsigned int dwc_usb3_dep_getepstate(dwc_usb3_pcd_t *pcd,
					dwc_usb3_dev_ep_regs_t   *ep_reg);
extern int dwc_usb3_dep_sstall(dwc_usb3_pcd_t *pcd,
			       dwc_usb3_dev_ep_regs_t   *ep_reg);
extern int dwc_usb3_dep_cstall(dwc_usb3_pcd_t *pcd,
			       dwc_usb3_dev_ep_regs_t   *ep_reg);
extern int dwc_usb3_dep_startxfer(dwc_usb3_pcd_t *pcd,
				  dwc_usb3_dev_ep_regs_t   *ep_reg,
				  dwc_dma_t dma_addr, unsigned int stream_or_uf);
extern int dwc_usb3_dep_updatexfer(dwc_usb3_pcd_t *pcd,
				   dwc_usb3_dev_ep_regs_t   *ep_reg,
				   unsigned int tri);
extern int dwc_usb3_dep_endxfer(dwc_usb3_pcd_t *pcd,
				dwc_usb3_dev_ep_regs_t   *ep_reg,
				unsigned int tri, int force, void *condition);
#ifdef DWC_STAR_9000463548_WORKAROUND
extern int dwc_usb3_dep_endxfer_nowait(dwc_usb3_pcd_t *pcd,
				       dwc_usb3_dev_ep_regs_t   *ep_reg,
				       unsigned int tri, int force);
extern int dwc_usb3_dep_wait_endxfer(dwc_usb3_pcd_t *pcd,
				     dwc_usb3_dev_ep_regs_t   *ep_reg,
				     void *condition);
#endif
extern int dwc_usb3_dep_startnewcfg(dwc_usb3_pcd_t *pcd,
				    dwc_usb3_dev_ep_regs_t   *ep_reg,
				    unsigned int rsrcidx);
extern int dwc_usb3_enable_ep(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep);
extern int dwc_usb3_disable_ep(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep);
extern int dwc_usb3_get_device_speed(dwc_usb3_pcd_t *pcd);
extern int dwc_usb3_get_frame(dwc_usb3_pcd_t *pcd);
extern int dwc_usb3_get_link_state(dwc_usb3_pcd_t *pcd);
extern void dwc_usb3_set_link_state(dwc_usb3_pcd_t *pcd, int state);
extern void dwc_usb3_remote_wake(dwc_usb3_pcd_t *pcd, int function);
extern void dwc_usb3_set_address(dwc_usb3_pcd_t *pcd, int addr);
extern void dwc_usb3_suspend_usb2_phy(dwc_usb3_pcd_t *pcd);
extern void dwc_usb3_resume_usb2_phy(dwc_usb3_pcd_t *pcd);
extern void dwc_usb3_suspend_usb3_phy(dwc_usb3_pcd_t *pcd);
extern void dwc_usb3_resume_usb3_phy(dwc_usb3_pcd_t *pcd);
extern void dwc_usb3_accept_u1(dwc_usb3_pcd_t *pcd);
extern void dwc_usb3_accept_u2(dwc_usb3_pcd_t *pcd);
extern void dwc_usb3_enable_u1(dwc_usb3_pcd_t *pcd);
extern void dwc_usb3_enable_u2(dwc_usb3_pcd_t *pcd);
extern void dwc_usb3_disable_u1(dwc_usb3_pcd_t *pcd);
extern void dwc_usb3_disable_u2(dwc_usb3_pcd_t *pcd);
extern int dwc_usb3_u1_enabled(dwc_usb3_pcd_t *pcd);
extern int dwc_usb3_u2_enabled(dwc_usb3_pcd_t *pcd);
extern void dwc_usb3_clr_eps_enabled(dwc_usb3_pcd_t *pcd);
extern void dwc_usb3_do_test_mode(unsigned long data);
extern void dwc_usb3_dump_dev_registers(dwc_usb3_pcd_t *pcd);

#define dwc_usb3_is_hwo(desc)	((desc)->control & DWC_DSCCTL_HWO_BIT)
#define dwc_usb3_is_ioc(desc)	((desc)->control & DWC_DSCCTL_IOC_BIT)

#define dwc_usb3_get_xfercnt(desc)				\
	(((desc)->status >> DWC_DSCSTS_XFRCNT_SHIFT) &		\
	 (DWC_DSCSTS_XFRCNT_BITS >> DWC_DSCSTS_XFRCNT_SHIFT))

#define dwc_usb3_get_xfersts(desc)				\
	(((desc)->status >> DWC_DSCSTS_TRBRSP_SHIFT) &		\
	 (DWC_DSCSTS_TRBRSP_BITS >> DWC_DSCSTS_TRBRSP_SHIFT))

#define dwc_usb3_get_xfersofn(desc)				\
	(((desc)->control >> DWC_DSCCTL_STRMID_SOFN_SHIFT) &	\
	 (DWC_DSCCTL_STRMID_SOFN_BITS >> DWC_DSCCTL_STRMID_SOFN_SHIFT))

#define dwc_usb3_get_eventsofn(event)				\
	(((event) >> DWC_DEPEVT_ISOC_UFRAME_NUM_SHIFT) &	\
	 (DWC_DEPEVT_ISOC_UFRAME_NUM_BITS >> DWC_DEPEVT_ISOC_UFRAME_NUM_SHIFT))

/** @}*/

/** @name Common CIL Functions
 */
/** @{ */
extern unsigned int dwc_usb3_get_snpsid(dwc_usb3_device_t *dev, unsigned int addr_ofs);
extern void dwc_usb3_dump_global_registers(dwc_usb3_device_t *dev);
extern void dwc_usb3_set_tx_fifo_size(dwc_usb3_device_t *dev,
				      int sz[DWC_MAX_TX_FIFOS]);
extern void dwc_usb3_set_rx_fifo_size(dwc_usb3_device_t *dev, int size);
extern void dwc_usb3_init_eventbuf(dwc_usb3_device_t *dev, int bufno,
				unsigned int *addr, int size, dwc_dma_t dma_addr);
extern void dwc_usb3_dis_flush_eventbuf_intr(dwc_usb3_device_t *dev, int bufno);
extern void dwc_usb3_enable_common_interrupts(dwc_usb3_device_t *dev);
extern void dwc_usb3_enable_device_interrupts(dwc_usb3_device_t *dev);
extern int dwc_usb3_handle_event(dwc_usb3_device_t *dev);
extern int dwc_usb3_irq(dwc_usb3_device_t *dev, int irq);

extern void dis_eventbuf_intr(dwc_usb3_device_t *dev, int bufno);
extern void ena_eventbuf_intr(dwc_usb3_device_t *dev, int bufno);

/**
 * This function returns the current operating mode, host or device.
 *
 * @return 0 - Device Mode, 1 - Host Mode
 */
static __inline unsigned int dwc_usb3_mode(dwc_usb3_device_t *dev)
{
	return dwc_rd32(dev, &dev->core_global_regs->gsts) & 0x1;
}

/**
 * This function returns true if the current operating mode is Device.
 *
 * @return 1 - Device mode, 0 - Not Device mode
 */
static __inline int dwc_usb3_is_device_mode(dwc_usb3_device_t *dev)
{
	return dwc_usb3_mode(dev) != DWC_GSTS_HOST_MODE;
}

/**
 * This function returns true if the current operating mode is Host.
 *
 * @return 1 - Host mode, 0 - Not Host mode
 */
static __inline int dwc_usb3_is_host_mode(dwc_usb3_device_t *dev)
{
	return dwc_usb3_mode(dev) == DWC_GSTS_HOST_MODE;
}
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* _DWC_CIL_H_ */
