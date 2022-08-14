/* ==========================================================================
 * $File: //dwh/usb_iip/dev/software/DWC_usb3/driver/pcd.h $
 * $Revision: #60 $
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
#ifndef _DWC_PCD_H_
#define _DWC_PCD_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 *
 * This file contains the structures, constants, and interfaces for
 * the Perpherial Contoller Driver (PCD).
 *
 * The Peripheral Controller Driver (PCD) for Linux will implement the
 * Gadget API, so that the existing Gadget drivers can be used. For
 * the Mass Storage Function driver the File-backed USB Storage Gadget
 * (FBS) driver will be used.  The FBS driver supports the
 * Control-Bulk (CB), Control-Bulk-Interrupt (CBI), and Bulk-Only
 * transports.
 *
 */

/** Maximum number of Tx FIFOs. Depends on the RTL configuration. No way to
 * probe the value at runtime
 */
#define DWC_MAX_TX_FIFOS	4

/** Maximum number of physical EPs. Depends on the RTL configuration. No way to
 * probe the value at runtime
 */
#define DWC_MAX_PHYS_EP		8

/** Maximum number of data buffers per TRB. OS/application specific */
#define DWC_MAX_DATA_BUFS	13

/** Maximum number of EPs, defined by USB spec */
#define DWC_MAX_EPS		16

/** Maxpacket size for EP0, defined by USB3 spec */
#define DWC_MAX_EP0_SIZE	512

/** Maxpacket size for any EP, defined by USB3 spec */
#define DWC_MAX_PACKET_SIZE	1024

/**
 * States of EP0
 */
typedef enum ep0_state {
	EP0_IDLE,
	EP0_IN_DATA_PHASE,
	EP0_OUT_DATA_PHASE,
	EP0_IN_WAIT_GADGET,
	EP0_OUT_WAIT_GADGET,
	EP0_IN_WAIT_NRDY,
	EP0_OUT_WAIT_NRDY,
	EP0_IN_STATUS_PHASE,
	EP0_OUT_STATUS_PHASE,
	EP0_STALL,
} ep0state_e;

typedef enum pcd_state {
	DWC_STATE_UNCONNECTED,	/* no host */
	DWC_STATE_DEFAULT,
	DWC_STATE_ADDRESSED,
	DWC_STATE_CONFIGURED,
} pcdstate_e;

struct dwc_usb3_device;
struct dwc_usb3_pcd;

/**
 * The <code>dwc_req</code> structure represents the state of a single
 * transfer request when acting in device mode. It contains the data items
 * needed for a request to be started and completed.
 */
typedef struct dwc_req {
	dwc_usb3_dma_desc_t *trb;	/**< TRB or TRB chain for this req */
	dwc_dma_t trbdma;		/**< DMA address of TRB or TRB chain */
	unsigned int length;		/**< total length of data bufs */
	unsigned int actual;		/**< actual amt of data transferred */
	unsigned int stream;		/**< stream # of this request */

	int flags;	/**< request flags - bits 8-31 are OS-specific */
#define DWC_PCD_REQ_ZERO	0x001
#define DWC_PCD_REQ_STARTED	0x002
#define DWC_PCD_REQ_MAP_DMA	0x100
#define DWC_PCD_REQ_IN		0x200

	int numbuf;				/**< number of data bufs */
	char *buf[DWC_MAX_DATA_BUFS];		/**< data buffers */
	dwc_dma_t bufdma[DWC_MAX_DATA_BUFS];	/**< DMA addrs of data bufs */
	unsigned int buflen[DWC_MAX_DATA_BUFS];	/**< length of data bufs */
} dwc_req_t;

/** DWC_usb3 request structure.
 * This structure is used to form a list of requests.
 */
typedef struct dwc_usb3_pcd_req {
	/** DWC_usb3 request data */
	dwc_req_t dwc_req;

	/* ==== The rest is OS-specific ==== */

#ifdef LINUX_KERNEL
	/** List entry for EP queue */
	struct list_head entry;

	/** USB request */
	struct usb_request usb_req;
#endif /* LINUX_KERNEL */
} dwc_usb3_pcd_req_t;

#ifdef LINUX_KERNEL		/* 012.08.13 */
#define dwc_usb3_get_pcd_req(usbreq) \
	container_of((usbreq), dwc_usb3_pcd_req_t, usb_req)
#endif /* LINUX_KERNEL */

/**
 * The <code>dwc_ep</code> structure represents the state of a single EP
 * when acting in device mode. It contains the data items needed for an EP
 * to be activated and transfer packets.
 */
typedef struct dwc_ep {
	/** Pointer to PCD */
	struct dwc_usb3_pcd *pcd;

	/** Pointer to OUT EP register */
	dwc_usb3_dev_ep_regs_t   *out_ep_reg;

	/** Pointer to IN EP register */
	dwc_usb3_dev_ep_regs_t   *in_ep_reg;

	/** Physical EP number */
	unsigned char phys;

	/** USB EP number */
	unsigned char num;

	/** EP type: 0 - Control, 1 - ISOC, 2 - BULK, 3 - INTR */
	unsigned char type;

	/** 'bInterval' value for Isoc EPs */
	unsigned char intvl;

	/** Max Packet bytes */
	unsigned short maxpacket;

	/** 'mult' value for SS Isoc EPs */
	unsigned char mult;

	/** Max burst size for SS EPs (0 - 15, actual burst is 1 - 16) */
	unsigned char maxburst;

	/** Number of streams for SS Bulk EPs (0 - 16, actual number is 2^n) */
	unsigned char num_streams;

	/** Tx FIFO # for IN EPs */
	unsigned char tx_fifo_num;

	/** The Transfer Resource Index from the Start Transfer command */
	unsigned char tri_out;	/** @< */
	unsigned char tri_in;

	/** Status of the queue */
	unsigned int stopped		: 1;	/** @< */
	unsigned int disabling		: 1;	/** @< */
	unsigned int queue_sof		: 1;

	/** @name Transfer state */
	/** @{ */
	/** Send ZLP */
	unsigned int send_zlp		: 1;

	/** Stall clear flag */
	unsigned int stall_clear_flag	: 1;

	/** True if 3-stage control transfer */
	unsigned int three_stage	: 1;

	/** True if transfer has been started on EP */
	unsigned int xfer_started	: 1;
	/** @} */

	/** EP direction 0 = OUT */
	unsigned int is_in		: 1;

	/** EP active */
	unsigned int active		: 1;

	/** DATA start PID for INTR and BULK EP */
	unsigned int data_pid_start	: 1;

	/** EP has been enabled for this configuration */
	unsigned int ena_once		: 1;

	/** EP was in stalled state when entering hibernation */
	unsigned int stalled_save	: 1;

	/** Saved parameters from the last DEPCFG for this EP. Used when
	 * resetting the sequence number
	 */
	unsigned int param0out;	/** @< */
	unsigned int param1out;	/** @< */
	unsigned int param0in;	/** @< */
	unsigned int param1in;

	/** EP state, saved across core hibernation */
	unsigned int save_state;

	/** Last event received while going into hibernation */
	unsigned int last_event;

	/** Number of DMA descriptors available */
	int desc_avail;

	/* ==== The rest is OS-specific ==== */

	/** Condition variable for EPCMD_CMPL interrupt */
	unsigned int condition;
//#ifdef LINUX_KERNEL		/* 012.08.13 */

	/** Pointer to USB EP descriptor */
	const usb_endpoint_descriptor_t *usb_ep_desc;

	/** Queue of dwc_usb3_pcd_reqs */
//	struct list_head queue;

	/** Array of DMA descriptors for this EP */
	char *dma_desc;		/** @< */
	dwc_dma_t dma_desc_dma;	/** @< */
	int dma_desc_size;	/** @< */
	int num_desc;		/** @< */
	int desc_idx;		/** @< */
//#endif /* LINUX_KERNEL */
	int hiber_desc_idx;
} dwc_ep_t;

/** PCD EP structure.
 * This structure describes an EP, there is an array of EP pointers in the
 * PCD structure.
 */
typedef struct dwc_usb3_pcd_ep {
	/** DWC_usb3 EP data */
	dwc_ep_t dwc_ep;

	/* ==== The rest is OS-specific ==== */

#ifdef LINUX_KERNEL		/* 012.08.13 */
	/** USB EP */
	struct usb_ep usb_ep;
#endif /* LINUX_KERNEL */
} dwc_usb3_pcd_ep_t;

#ifdef LINUX_KERNEL		/* 012.08.13 */
#define dwc_usb3_get_pcd_ep(usbep) \
	container_of((usbep), dwc_usb3_pcd_ep_t, usb_ep)
#endif /* LINUX_KERNEL */

#define dwc_usb3_pcd_ep_to_pcd(pcd_ep)	((pcd_ep)->dwc_ep.pcd)
#define dwc_usb3_pcd_ep_num(pcd_ep)	((pcd_ep)->dwc_ep.num)
#define dwc_usb3_pcd_ep_type(pcd_ep)	((pcd_ep)->dwc_ep.type)
#define dwc_usb3_pcd_ep_is_in(pcd_ep)	((pcd_ep)->dwc_ep.is_in)

struct dwc_hiber_scratchpad_array {
	unsigned long long	dma_addr[15];
};

/** DWC_usb3 PCD Structure.
 * This structure encapsulates the data for the dwc_usb3 PCD.
 */
typedef struct dwc_usb3_pcd {
	/** The DWC otg device pointer */
	struct dwc_usb3_device *usb3_dev;

	/** USB3 link state */
	int link_state;

	/** State of the device */
	pcdstate_e state;

	/** State of EP0 */
	ep0state_e ep0state;

	/** EP0 Status Request is pending */
	unsigned int ep0_status_pending		: 1;

	/** Indicates when SET CONFIGURATION Request is in process */
	unsigned int request_config		: 1;

	/** State of the Remote Wakeup Enable */
	unsigned int remote_wakeup_enable	: 1;

	/** State of the Latency Tolerance Messaging Enable */
	unsigned int ltm_enable			: 1;

	/** True if ready for remote wakeup request from user */
	unsigned int wkup_rdy			: 1;

	/** True if we have enabled some EPs */
	unsigned int eps_enabled		: 1;

	/** True if UTE has made some config changes */
	unsigned int ute_change			: 1;

#ifdef DWC_STAR_9000463548_WORKAROUND
	unsigned int configuring		: 1;
#endif
	unsigned int devnum			: 4;

#ifdef CONFIG_USB_OTG_DWC
	/** Set when user writes to 'hnp' sysfs attribute */
	unsigned int wants_host			: 1;

	/** For set feature (b_hnp_enable) */
	unsigned int b_hnp_enable		: 1;
#endif
	/** EP0 */
	dwc_usb3_pcd_ep_t *ep0;

	/** Array of OUT EPs (not including EP0) */
	dwc_usb3_pcd_ep_t *out_ep[DWC_MAX_EPS - 1];

	/** Array of IN EPs (not including EP0) */
	dwc_usb3_pcd_ep_t *in_ep[DWC_MAX_EPS - 1];

	/** Pointer to device Global registers.
	 * Device Global Registers starting at offset 800h
	 */
	dwc_usb3_dev_global_regs_t   *dev_global_regs;

	/** Device Logical OUT EP-Specific Registers 600h-7FCh */
	dwc_usb3_dev_ep_regs_t   *out_ep_regs;

	/** Device Logical IN EP-Specific Registers 300h-4FCh */
	dwc_usb3_dev_ep_regs_t   *in_ep_regs;

	/** Scratchpad buffers for hibernation support */
	void *hiber_scratchpad[15];	/** @< */
	struct dwc_hiber_scratchpad_array *hiber_scratchpad_array; /** @< */
	dwc_dma_t hiber_scratchpad_array_dma;

	/** EP0 state, saved across core hibernation */
	unsigned int ep0_out_save_state;	/** @< */
	unsigned int ep0_in_save_state;

	/** Last event received while going into hibernation */
	unsigned int last_event;

	/** 'dummy' request, for EP0 only */
	dwc_usb3_pcd_req_t *ep0_req;

#ifdef DWC_UTE
	/** size of Rx FIFO, requested by UTE */
	unsigned rxf_size;

	/** size of Tx FIFOs, requested by UTE */
	unsigned txf_size[DWC_MAX_TX_FIFOS];

	/** mapping of Tx FIFO for each physical EP, requested by UTE */
	unsigned txf_map[DWC_MAX_PHYS_EP];

	/** Rx FIFO default size */
	unsigned def_rxf_size;

	/** Tx FIFOs, default size */
	unsigned def_txf_size[DWC_MAX_TX_FIFOS];
#endif

	/** Thresholding enable flags and length variables */
	unsigned short rx_thr_en;		/** @< */
	unsigned short iso_tx_thr_en;		/** @< */
	unsigned short non_iso_tx_thr_en;	/** @< */
	unsigned short rx_thr_length;		/** @< */
	unsigned short tx_thr_length;

	/* Device configuration information */
	unsigned char speed;	/**< Device Speed - 0:Unk 1:LS 2:FS 3:HS 4:Var 5:SS */
	unsigned char num_out_eps;	/**< Number # of Rx EP range: 0-15 except ep0 */
	unsigned char num_in_eps;	/**< Number # of Tx EP range: 0-15 except ep0 */

	/** The TEST mode to enter when test_mode_tasklet is executed */
	unsigned char test_mode;

	/* ==== The rest is OS-specific ==== */

#ifdef LINUX_KERNEL		/* 012.08.13 */
	/** The spinlock for the PCD */
	spinlock_t lock;
#endif /* LINUX_KERNEL */

	/** The associated gadget */
//	struct usb_gadget *gadget;

	/** Tasklet to defer starting of TEST mode transmissions until
	 * Status Phase has been completed
	 */
//	struct tasklet_struct test_mode_tasklet;

	/** Count of pending Requests */
	unsigned request_pending;

	/**
	 * Pointers to the DMA Descriptors for EP0 Control transfers
	 * (virtual and physical)
	 */

	/** Descriptor for SETUP packets */
	dwc_usb3_dma_desc_t *ep0_setup_desc;	/** @< */
	dwc_dma_t ep0_setup_desc_dma;

	/** Descriptor for Data Out or Status Out phases */
	dwc_usb3_dma_desc_t *ep0_out_desc;	/** @< */
	dwc_dma_t ep0_out_desc_dma;

	/** Descriptor for Data In or Status In phases */
	dwc_usb3_dma_desc_t *ep0_in_desc;	/** @< */
	dwc_dma_t ep0_in_desc_dma;

	/** Data packet buffer used to return data for GET_STATUS and
	 *  GET_DESCRIPTOR(BOS) up to 512 bytes in length
	 */
	unsigned char *ep0_status_buf;	/** @< */
	dwc_dma_t ep0_status_buf_dma;
	#define DWC_STATUS_BUF_SIZE 512

	/** SETUP packet buffer for EP0 */
	union {
		usb_device_request_t req;
		unsigned int d32[2];
		char d8[8];
	} *ep0_setup_pkt;		/** @< */
	dwc_dma_t ep0_setup_pkt_dma;
} dwc_usb3_pcd_t;

/* OS-specific functions called from core code */
extern void dwc_usb3_os_get_trb(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep,
				dwc_usb3_pcd_req_t *req);
extern void dwc_usb3_os_handle_ep0(dwc_usb3_pcd_t *pcd, unsigned int event);
extern void dwc_usb3_mark_ep_queue_not_started(dwc_usb3_pcd_t *pcd,
					       dwc_usb3_pcd_ep_t *ep);
extern void dwc_usb3_os_complete_request(dwc_usb3_pcd_t *pcd,
					 dwc_usb3_pcd_ep_t *ep, unsigned int event);
extern void dwc_usb3_os_start_next_request(dwc_usb3_pcd_t *pcd,
					   dwc_usb3_pcd_ep_t *ep);
extern void dwc_usb3_os_isoc_ep_start(dwc_usb3_pcd_t *pcd,
				      dwc_usb3_pcd_ep_t *ep, unsigned int event);
extern void dwc_usb3_os_request_nuke(dwc_usb3_pcd_t *pcd,
				     dwc_usb3_pcd_ep_t *ep);
//extern void dwc_usb3_os_task_schedule(struct tasklet_struct *tasklet);

/* OS-independent functions in core code */
extern void dwc_usb3_ep0_activate(dwc_usb3_pcd_t *pcd, int restore);
extern void dwc_usb3_ep_activate(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep,
				 int restore);
extern void dwc_usb3_ep0_out_start(dwc_usb3_pcd_t *pcd);
extern void dwc_usb3_ep0_start_transfer(dwc_usb3_pcd_t *pcd,
					dwc_usb3_pcd_req_t *req);
extern void dwc_usb3_ep_start_transfer(dwc_usb3_pcd_t *pcd,
				       dwc_usb3_pcd_ep_t *ep,
				       dwc_usb3_pcd_req_t *req, unsigned int event);
extern void dwc_usb3_stop_all_xfers(dwc_usb3_pcd_t *pcd);
extern int dwc_usb3_ep_complete_request(dwc_usb3_pcd_t *pcd,
					dwc_usb3_pcd_ep_t *ep,
					dwc_usb3_pcd_req_t *req,
					unsigned int event);
extern void dwc_usb3_ep_set_stall(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep);
extern void dwc_usb3_ep_clear_stall(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep);
extern dwc_usb3_pcd_ep_t *dwc_usb3_get_out_ep(dwc_usb3_pcd_t *pcd,
					      unsigned int ep_num);
extern dwc_usb3_pcd_ep_t *dwc_usb3_get_in_ep(dwc_usb3_pcd_t *pcd,
					     unsigned int ep_num);
extern dwc_usb3_pcd_ep_t *dwc_usb3_get_ep_by_addr(dwc_usb3_pcd_t *pcd,
						  unsigned short index);
#ifdef DEBUG_EP0
extern void dwc_usb3_print_ep0_state(dwc_usb3_pcd_t *pcd);
#endif
extern void dwc_usb3_handle_ep0(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_req_t *req,
				unsigned int event);
extern void dwc_usb3_request_done(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep,
				  dwc_usb3_pcd_req_t *req, int status);
extern void dwc_usb3_pcd_stop(dwc_usb3_pcd_t *pcd);
extern void dwc_usb3_handle_ep_intr(dwc_usb3_pcd_t *pcd, int physep,
				    unsigned int event);
extern int dwc_usb3_handle_dev_intr(dwc_usb3_pcd_t *pcd, unsigned int event);
extern void dwc_usb3_handle_connect_done_intr(dwc_usb3_pcd_t *pcd);
extern void dwc_enter_hibernation(dwc_usb3_pcd_t *pcd, int save_state);
extern void dwc_exit_hibernation_after_connect(dwc_usb3_pcd_t *pcd,
					       int connected);
extern int dwc_exit_hibernation(dwc_usb3_pcd_t *pcd, int restore_state);
extern int dwc_wait_pme_thread(void *data);
extern int dwc_usb3_handle_pme_intr(struct dwc_usb3_device *dev);

/* Functions in gadget interface code */
extern int dwc_usb3_pcd_setup(dwc_usb3_pcd_t *pcd, void *data);
extern int dwc_usb3_pcd_disconnect(dwc_usb3_pcd_t *pcd);
extern int dwc_usb3_pcd_set_speed(dwc_usb3_pcd_t *pcd, int speed);
extern int dwc_usb3_pcd_suspend(dwc_usb3_pcd_t *pcd);
extern int dwc_usb3_pcd_resume(dwc_usb3_pcd_t *pcd);
extern int dwc_usb3_pcd_complete(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *pcd_ep,
				 dwc_usb3_pcd_req_t *pcd_req, int status);
//extern int dwc_usb3_wakeup(struct usb_gadget *gadget);

extern int dwc_usb3_pcd_init(struct dwc_usb3_device *dev);
extern void dwc_usb3_pcd_remove(struct dwc_usb3_device *dev);
extern int dwc_usb3_pcd_ep_enable(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep,
				const void *ep_desc);
extern int dwc_usb3_pcd_ep_disable(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep);
extern void dwc_usb3_pcd_fill_req(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep,
				dwc_usb3_pcd_req_t *req, int numbuf,
				void **buf, dwc_dma_t *bufdma, unsigned int *buflen,
				unsigned int stream, int req_flags);
extern int dwc_usb3_pcd_ep_queue(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep,
				dwc_usb3_pcd_req_t *req, int req_flags,
				int q_empty);
extern void dwc_usb3_pcd_ep_dequeue(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep,
				dwc_usb3_pcd_req_t *req, unsigned int stream);
extern void dwc_usb3_pcd_ep_set_halt(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep,
				int value);
extern int dwc_usb3_get_frame_number(dwc_usb3_pcd_t *pcd);

#ifdef __cplusplus
}
#endif

#endif /* _DWC_PCD_H_ */
