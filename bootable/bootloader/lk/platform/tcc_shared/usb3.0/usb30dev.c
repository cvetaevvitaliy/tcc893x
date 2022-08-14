#include "usb3.0/os_dep.h"
#include "usb3.0/hw.h"
#include "usb3.0/usb.h"
#include "usb3.0/pcd.h"
#include "usb3.0/driver.h"
#include "usb3.0/cil.h"
#include "usb3.0/usb30dev.h"
#include "lib/heap.h"
#include "usb/usb_defs.h"
#include "usb/usb_manager.h"
#include <platform/iomap.h>
#include <platform/reg_physical.h>
#include "usb/usb_device.h"
#include <debug.h>
#if !defined(_LINUX_)
#include "memory.h"
#endif


#define CTAG_S  "\x1b[1;33m"
#define CTAG_S_R  "\x1b[1;31m"
#define CTAG_E  "\x1b[0m"

//=============================================================================
//
//                           [ CONST DATA DEFINE ]
//
//=============================================================================
/* For Signature */
#define USB30DEV_IO_SIGNATURE				'U','S','B','3','0','_','I','O','_'
#define USB30DEV_IO_VERSION					'V','0','.','0','0','1'
static const unsigned char USB30DEV_IO_C_Version[] = {USB30DEV_IO_SIGNATURE, SIGN_OS ,SIGN_CHIPSET, USB30DEV_IO_VERSION, NULL};
static USBDEV_ERROR_T g_USB30GDEV_IO_LastError = USBDEV_ERROR_FAILURE;

//=============================================================================
//
//                           [ DEBUG MECRO DEFINE ]
//
//=============================================================================
#define udev_trace(f, a...)		dprintf(2,"[%s:%d]\n" f, __func__, __LINE__, ##a)
#define udev_err_debug(f, a...)		dprintf(2,"[%s:%d] " f, __func__, __LINE__, ##a)
#define ep0_debug(f, a...)		dprintf(DEBUG_EP0, f, ##a)
#define ep_debug(f, a...)		dprintf(DEBUG_EP, f, ##a)
#define que_debug(f, a...)  		dprintf(DEBUG_QUE, f, ##a)
#define isr_debug(f, a...)		dprintf(DEBUG_ISR, f, ##a)
#define isr_handle_debug(f, a...)  	dprintf(DEBUG_ISR_HANDLE, f, ##a)

// Register Read/Write command
#define REG_WR32(a,b)		(*((volatile int *)(a)) = b)
#define REG_RD32(a,b)		(b=*((volatile int *)(a)))

//=============================================================================
//
//                           [ LOCAL FUNCTIONS DEFINE ]
//
//=============================================================================
#if defined(_LINUX_)
extern void memcpy(void *pvDest, const void *pvSrc, unsigned long iCount);
extern void memset(void *pvDest, char cChar, unsigned long iCount);
#endif
static unsigned int USB30DEV_IO_EP_IN_StartNextTransfer(unsigned int nEndPoint);
static unsigned int USB30DEV_IO_EP_OUT_StartNextTransfer(unsigned int nEndPoint);

//=============================================================================
//
//                           [ GLOBAL VARIABLE DEFINE ]
//
//=============================================================================
dwc_usb3_pcd_req_t	g_ep0_req;
dwc_usb3_pcd_ep_t 	g_ep0;
dwc_usb3_pcd_ep_t 	g_out_ep[DWC_MAX_EPS - 1];
dwc_usb3_pcd_ep_t 	g_in_ep[DWC_MAX_EPS - 1];

#define XFER_STATE_IDLE					0
#define XFER_STATE_WORKING				1
#define XFER_STATE_WAIT_POP				2

#define QUEUE_STATE_IDLE				0
#define QUEUE_STATE_ACTIVATE			1
#define QUEUE_STATE_DONE				2

#define QUEUE_MAX						16
#define QUEUE_EP0_SIZE					128

typedef struct {
	USBDEV_QUEUE_T 				queue[QUEUE_MAX];
	unsigned char 				bQueueNum;
	volatile unsigned char 		iFirstQ;
	volatile unsigned char 		iLastQ;
	volatile unsigned char 		xferState;
	unsigned int				MPS;
	dwc_usb3_pcd_req_t			req;
} EP_IN_T;
static EP_IN_T gUSB30DEV_IO_EpIn[16];
typedef struct {
	USBDEV_QUEUE_T 				queue[QUEUE_MAX];
	unsigned char 				bQueueNum;
	volatile unsigned char 		iFirstQ;
	volatile unsigned char 		iLastQ;
	volatile unsigned char 		iPrevFirstQ;
	unsigned int 				MPS;
	unsigned int 				xferLeftLength;
	volatile unsigned char 		xferState;
	unsigned char 				bZeroPacket;
	dwc_usb3_pcd_req_t			req;
} EP_OUT_T;
static EP_OUT_T gUSB30DEV_IO_EpOut[16];
static unsigned char gUSB30DEV_IO_EP_OUT_QueuedFlag_for_IsReady[16];
#if defined(TCC893X)
static unsigned int 		gUSB30DEV_ISR_MODE = FALSE;
#endif
static USBDEV_ENDPOINT_T	g_USBDEV_ENDPOINT_T[16];
static unsigned int		gEP_T_Index = 0;
static unsigned int		USB30DEV_IO_Lock = 1;
static USBDEVICE_IO_DRIVER_T	g_USB30DEV_IO_Driver;
static dwc_usb3_device_t	g_dwc_usb3_device;
static dwc_usb3_core_params_t	dwc_usb3_module_params = {
	.burst = 0,
	.newcore = 1,
	.phy = 1,
	.wakeup = 0,
#ifdef DWC_STAR_9000446947_WORKAROUND
	.pwrctl = 0,
#else
# if defined(DWC_STAR_9000449814_WORKAROUND) || \
     defined(DWC_STAR_9000459034_WORKAROUND)
	.pwrctl = 2,
# else
	.pwrctl = 3,
# endif
#endif
	.u1u2ctl = 0,
	.lpmctl = 0,
	.phyctl = 0,
	.usb2mode = 2,
	.hibernate = 0,
	.hiberdisc = 1,
	.clkgatingen = 0,
	.ssdisquirk = 0,
	.nobos = 0,
	.loop = 0,
	.nump = 16,
	.rx = 0,
	.tx0 = 0, .tx1 = 0, .tx2 = 0, .tx3 = 0,
};

void usb30_poweron_reset_usb20_mode(void);
void usb30_software_reset(void);

void USB30DEV_memcpy(void *dst, void *src, unsigned long cnt)
{
#if defined(_LINUX_)
	if(((unsigned int)dst)&0x3 || ((unsigned int)src)&0x3)
	{
		char *p_bDst = (char*)dst;
		char *p_bSrc = (char*)src;
		while(cnt--)
			*p_bDst++ = *p_bSrc++;
	}
	else
	{
		memcpy(dst,src,cnt);
	}
#else
	memcpy(dst,src,cnt);
#endif
}

dwc_usb3_device_t *USB30DEV_Get_Device_t(void)
{
	return &g_dwc_usb3_device;
}

static void USB30DEV_ResetQueue(void)
{
	int i,j;

	for(i=0;i<16;i++)
	{
		gUSB30DEV_IO_EpIn[i].iFirstQ = 0;
		gUSB30DEV_IO_EpIn[i].iLastQ = 0;
		gUSB30DEV_IO_EpIn[i].xferState = XFER_STATE_IDLE;

		gUSB30DEV_IO_EpOut[i].iFirstQ = 0;
		gUSB30DEV_IO_EpOut[i].iLastQ = 0;
		gUSB30DEV_IO_EpOut[i].iPrevFirstQ = gUSB30DEV_IO_EpOut[i].bQueueNum;
		gUSB30DEV_IO_EpOut[i].xferLeftLength = 0;
		gUSB30DEV_IO_EpOut[i].bZeroPacket = 0;
		gUSB30DEV_IO_EpOut[i].xferState = XFER_STATE_IDLE;

		for(j=0;j<QUEUE_MAX;j++)
		{
			gUSB30DEV_IO_EpIn[i].queue[j].state = QUEUE_STATE_IDLE;
			gUSB30DEV_IO_EpIn[i].queue[j].actualLen = 0;
			gUSB30DEV_IO_EpOut[i].queue[j].state = QUEUE_STATE_IDLE;
			gUSB30DEV_IO_EpOut[i].queue[j].actualLen = 0;
		}

		gUSB30DEV_IO_EP_OUT_QueuedFlag_for_IsReady[i] = FALSE;
	}
}

static int USB30DEV_DWC_EP_Enable(USBDEV_ENDPOINT_T *pEP)
{
	dwc_usb3_device_t *usb3_dev;
	dwc_usb3_pcd_ep_t *pcd_ep;
	dwc_usb3_pcd_t *pcd;
	dma_addr_t desc_dma;
	usb_endpoint_descriptor_t	ep_desc;

	char *desc;
	int desc_size, num, dir, type, num_trbs;
	int retval = 0;

	usb3_dev = USB30DEV_Get_Device_t();
	pcd = &usb3_dev->pcd;

	ep_debug("[%s] pEP->index: %d\n", __func__, pEP->index);

	ep_desc.bLength				= pEP->uiPreOutSize;
	#ifdef TCC_USB_30_USE
	ep_desc.bDescriptorType		= pEP->bDescriptorType;
	#endif
	ep_desc.bEndpointAddress 	= pEP->index;
	ep_desc.bEndpointAddress 	= UE_SET_DIR(ep_desc.bEndpointAddress, pEP->isIn);		// set dir
	ep_desc.bmAttributes		= pEP->transferType;
	//ep_desc.wMaxPacketSize	= pEP->maximumPacketSize;
	USETW(ep_desc.wMaxPacketSize,  pEP->maximumPacketSize);
	ep_desc.bInterval 			= pEP->bInterval;

	num 	= UE_GET_ADDR(ep_desc.bEndpointAddress);
	dir 	= UE_GET_DIR(ep_desc.bEndpointAddress);
	type 	= UE_GET_XFERTYPE(ep_desc.bmAttributes);

	if (num > DWC_MAX_EPS)
	{
		udev_err_debug("Endpoint Address over DWC_MAX_EPS : %d!\n", num);
		return -1;
	}

	if(dir == UE_DIR_IN)
	{
		pcd_ep = pcd->in_ep[num-1];
		gUSB30DEV_IO_EpIn[pEP->index].MPS = pEP->maximumPacketSize;
	}
	else
	{
		pcd_ep = pcd->out_ep[num-1];
		gUSB30DEV_IO_EpOut[pEP->index].MPS = pEP->maximumPacketSize;
	}

	/* Allocate the number of TRBs based on EP type */
	switch (type) {
	case UE_INTERRUPT:
		num_trbs 	= DWC_NUM_INTR_TRBS;
		desc_size 	= num_trbs * 16;
		break;
	case UE_ISOCHRONOUS:
		num_trbs 	= DWC_NUM_ISOC_TRBS + 1; /* +1 for link TRB */
		desc_size 	= num_trbs * 16;
		break;
	default:
		num_trbs 	= DWC_NUM_BULK_TRBS;
		desc_size 	= num_trbs * 16;
		break;
	}

	ep_debug("ep%d-%s=%p phys=%d pcd_ep=%p\n",
		  num, dir == UE_DIR_IN ? "IN" : "OUT", pcd_ep,
		  pcd_ep->dwc_ep.phys, pcd_ep);

	/* Set the TRB allocation for this EP */
	desc = dma_alloc(128, desc_size);
	desc_dma = (dma_addr_t)desc;
	if (!desc)
		return -1;
	memset(desc, 0, desc_size);
	
	ep_debug("%sep%d-%s phys=%d pcd_ep=%p TRB:0x%x%s\n",
		    CTAG_S_R,  num, dir == UE_DIR_IN ? "IN " : "OUT",
		  pcd_ep->dwc_ep.phys, pcd_ep, desc_dma, CTAG_E);
	
	/* Init the pcd_ep structure */
	pcd_ep->dwc_ep.dma_desc 	= desc;
	pcd_ep->dwc_ep.dma_desc_dma = desc_dma;
	pcd_ep->dwc_ep.dma_desc_size = desc_size;
	pcd_ep->dwc_ep.num_desc =
		type == UE_ISOCHRONOUS ? num_trbs - 1 : num_trbs;
	pcd_ep->dwc_ep.desc_avail = pcd_ep->dwc_ep.num_desc;
	pcd_ep->dwc_ep.desc_idx = 0;
	pcd_ep->dwc_ep.hiber_desc_idx = 0;
	pcd_ep->dwc_ep.num = num;

	retval = dwc_usb3_pcd_ep_enable(pcd, pcd_ep, (const void *)&ep_desc);
	return retval;
}

/**
 * This function disables an EP
 */
static int USB30DEV_DWC_EP_Disable(USBDEV_ENDPOINT_T *pEP)
{
	dwc_usb3_device_t *usb3_dev;	
	dwc_usb3_pcd_ep_t *pcd_ep;
	dwc_usb3_pcd_t *pcd;
	dma_addr_t desc_dma;
	usb_endpoint_descriptor_t	ep_desc;
	int desc_size, num, dir, type;
	int retval = 0;
	char *desc;
	
	usb3_dev = USB30DEV_Get_Device_t();
	pcd = &usb3_dev->pcd;

	ep_desc.bLength				= pEP->uiPreOutSize;
	ep_desc.bDescriptorType		= pEP->bDescriptorType;
	ep_desc.bEndpointAddress 	= pEP->index;
	ep_desc.bEndpointAddress 	= UE_SET_DIR(ep_desc.bEndpointAddress, pEP->isIn);		// set dir
	ep_desc.bmAttributes		= pEP->transferType;
	//ep_desc.wMaxPacketSize		= pEP->maximumPacketSize;
	USETW(ep_desc.wMaxPacketSize,  pEP->maximumPacketSize);
	ep_desc.bInterval 			= pEP->bInterval;

	num = UE_GET_ADDR(ep_desc.bEndpointAddress);
	dir = UE_GET_DIR(ep_desc.bEndpointAddress);
	type = UE_GET_XFERTYPE(ep_desc.bmAttributes);

	if (num > DWC_MAX_EPS)
	{
		udev_err_debug("Endpoint Address over DWC_MAX_EPS : %d!\n", num);
		return -1;		
	}

	if(dir == UE_DIR_IN)
		pcd_ep = pcd->in_ep[num-1];
	else
		pcd_ep = pcd->out_ep[num-1];

	retval = dwc_usb3_pcd_ep_disable(pcd, pcd_ep);

	desc = pcd_ep->dwc_ep.dma_desc;
	desc_dma = pcd_ep->dwc_ep.dma_desc_dma;
	desc_size = pcd_ep->dwc_ep.dma_desc_size;

	if (desc) {
		pcd_ep->dwc_ep.dma_desc = NULL;
		pcd_ep->dwc_ep.dma_desc_dma = 0;
		pcd_ep->dwc_ep.num = 0;
	}

	/* Free any existing TRB allocation for this EP */
	if (desc)
		//dma_free_coherent(NULL, desc_size, desc, desc_dma);
		dma_free((void *)desc_dma);

	if (retval)
		return -1;
	
	return 0;
}

static void USB30DEV_do_setup(dwc_usb3_pcd_t *pcd)
{
	usb_device_request_t ctrl = pcd->ep0_setup_pkt->req;
	dwc_usb3_pcd_ep_t *ep0 = pcd->ep0;
	uint16_t wvalue, wlength;

	wvalue 	= UGETW(ctrl.wValue);
	wlength = UGETW(ctrl.wLength);

	/* Clean up the request queue */
	ep0->dwc_ep.stopped = 0;
	ep0->dwc_ep.three_stage = 1;

	if (ctrl.bmRequestType & UE_DIR_IN) {
		ep0->dwc_ep.is_in = 1;
		pcd->ep0state = EP0_IN_DATA_PHASE;
	} else {
		ep0->dwc_ep.is_in = 0;
		pcd->ep0state = EP0_OUT_DATA_PHASE;
	}

	///////////////////////////////////////////
	// --- Standard Request handling --- //
	///////////////////////////////////////////
	USBDEV_EP0_Setup((USB_DEVICE_REQUEST_T*)pcd->ep0_setup_pkt);
}

void USB30DEV_Handle_ep0(dwc_usb3_pcd_t *pcd, uint32_t event)
{
	dwc_usb3_pcd_req_t *req = NULL;

	req = pcd->ep0_req;

	if (pcd->ep0state == EP0_IDLE) {
		pcd->request_config = 0;
		USB30DEV_do_setup(pcd);
	} else {
		dwc_usb3_handle_ep0(pcd, req, event);
	}
}

static void USB30DEV_IO_EP_IN_Isr(unsigned int nEndPoint);
static void USB30DEV_IO_EP_OUT_Isr(unsigned int nEndPoint);
void USB30DEV_Complete_Request(dwc_usb3_pcd_t *pcd, dwc_usb3_pcd_ep_t *ep,
				  uint32_t event)
{
	dwc_usb3_pcd_req_t *req = NULL;
	int ret;

	if (ep->dwc_ep.is_in)
		req = &gUSB30DEV_IO_EpIn[ep->dwc_ep.num].req;
	else
		req = &gUSB30DEV_IO_EpOut[ep->dwc_ep.num].req;

	ret = dwc_usb3_ep_complete_request(pcd, ep, req, event);
	if (ret<0)
	{
		_dprintf("dwc_usb3_ep_complete_request ret fail\n");
		return;
	}
	
	if (ep->dwc_ep.is_in)
		USB30DEV_IO_EP_IN_Isr(ep->dwc_ep.num);
	else
		USB30DEV_IO_EP_OUT_Isr(ep->dwc_ep.num);
}

static void USB30DEV_PowerOn_Reset (void) 
{
    uint uTmp;

    // Zero Initialization USB Device Datastructure using HSBDMAX
    // Assert Core Soft reset 
    uTmp = Hw30; // CSftRst
    REG_WR32(HwUSBDEVICE_BASE + 0x704, uTmp); // DCTL

    // Wait Controller de-assert 'Core Soft reset'
    while(uTmp != 0)
        REG_RD32(HwUSBDEVICE_BASE + 0x704, uTmp); // DCTL

    // GUSB2PHYCFG, Set 8bit PHY interface and UTMI+ interface
    REG_RD32(HwUSBDEVICE_BASE + 0x200, uTmp); // GUSB2PHYCFG
    REG_WR32(HwUSBDEVICE_BASE + 0x200, uTmp&(~(Hw3|Hw4))); // GUSB2PHYCFG

    // Program Event Buffer Address
    // Program Global Control Register (GCTL)
    // Bypass Scrambling and Set Shorter Training sequence for simulation
    /* GCTL[31:19] is decided by suspend_clk = 15.625mhz, scale = suspend_clk/16khz, 15625/16 = 977(dec) = 3D1(Hex)
       when left shifted, it becomes 1d48 assuming [18:16]==0. */
    REG_WR32(HwUSBDEVICE_BASE + 0x110, 0x1E882008);  // GCTL    - ram_clk_sel=pipe3_clk, scramble-off, device mode

    // Program Device Config(DCFG) Register
    //DCFG[31:21]  = 0;      // Reserved
    //DCFG[20:17]  = 4'hf;   // NumP
    //DCFG[16:12]  = 0;      // Interrupt Number
    //DCFG[11:10]  = 0;      // Periodic Frame Interval, 80%
    //DCFG[9:3]    = 7'h0;   // DevAddr
    //DCFG[2:0]    = 3'b100; // DevSpeed -> 0:HS, 1:FS, 4:SS
    REG_WR32(HwUSBDEVICE_BASE + 0x700, 0xF<<17); // DCFG

    // Program Device Event Enable Register (DEVTEN)
    // 2 : Connection Done Enable
    // 1 : USB Reset Enable
    REG_WR32(HwUSBDEVICE_BASE + 0x708, Hw2 | Hw1); // DEVTEN

    // Check GCTL, DCFG, only for simulation check 
    //REG_RD32(HwUSBDEVICE_BASE + 0x110, uTmp);  // GCTL
    //REG_RD32(HwUSBDEVICE_BASE + 0x700, uTmp); // DCFG

    // DCTL.Run! (readey to receive SOF)
    //DCTL[31]    = 1;      Run
    //DCTL[30:23] = 0;      CSftRst,LSftRst,HIRD_Thres,AppL1Res
    //DCTL[22:20] = 0;      Reserved
    //DCTL[19:16] = 0;      KeepConnect,L1HibernationEn,CRS,CSS
    //DCTL[15:13] = 0;      Reserved
    //DCTL[12:9]  = 0;      InitU2Ena,AcceptU2ena,InitU1Ena,AcceptU1Ena
    //DCTL[8:5]   = 0;      Link State Change - No action
    //DCTL[4:1]   = 0;      Test Control
    //DCTL[0]     = 0;      Reserved
    REG_WR32(HwUSBDEVICE_BASE + 0x704, Hw31);

//    while(!HwHSBDMAX->fifo_control.bReg.idle);
}

static void USB30DEV_PHY_Init (void) {
	PUSBPHYCFG USBPHYCFG = (PUSBPHYCFG)TCC_USBPHYCFG_BASE;
    unsigned int uTmp;
    // Initialize All PHY & LINK CFG Registers
    USBPHYCFG->UPCR0 = 0xB5484068;
    USBPHYCFG->UPCR1 = 0x0000041C;
    USBPHYCFG->UPCR2 = 0x919E14C8;
    USBPHYCFG->UPCR3 = 0x4AB05D00;
    USBPHYCFG->UPCR4 = 0x00000000;
    USBPHYCFG->UPCR5 = 0x00108001;
	USBPHYCFG->LCFG	 = 0x80420013;

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
	USBPHYCFG->LCFG	 |= Hw30;
    USBPHYCFG->UPCR1 |= Hw9;

    // Wait USB30 PHY initialization finish. PHY FREECLK should be stable.
    while(1) {
        // GDBGLTSSM : Check LTDB Sub State is non-zero
        // When PHY FREECLK is valid, LINK mac_resetn is released and LTDB Sub State change to non-zero
        uTmp = readl(HwUSBDEVICE_BASE+0x164);
        uTmp = (uTmp>>18)&0xF;

        // Break when LTDB Sub state is non-zero and CNR is zero
        if (uTmp != 0){
            break;
        }
    }
}

/**
 * This function initializes the PCD portion of the driver.
 *
 * @param dev The device context.
 */
int USB30DEV_DWC_PCD_Init(dwc_usb3_device_t *usb3_dev)
{
	dwc_usb3_pcd_t *pcd = &usb3_dev->pcd;
	int retval = -1;
	int i;

	/* Set the PCD's EP0 request pointer to the wrapper's request */
	pcd->ep0_req = &g_ep0_req;

	/* Set the PCD's EP array pointers to the wrapper's EPs */
	pcd->ep0 = &g_ep0;
	for (i = 0; i < DWC_MAX_EPS - 1; i++) {
		pcd->out_ep[i] = &g_out_ep[i];
		pcd->in_ep[i] = &g_in_ep[i];
	}
	
	/* Allocate the EP0 packet buffers */
	pcd->ep0_setup_pkt = dma_alloc(128, sizeof(*pcd->ep0_setup_pkt) * 5);
	pcd->ep0_setup_pkt_dma = (dwc_dma_t)pcd->ep0_setup_pkt;
	if (!pcd->ep0_setup_pkt)
		goto fail;
	
	pcd->ep0_status_buf = dma_alloc(128, DWC_STATUS_BUF_SIZE);
	pcd->ep0_status_buf_dma = (dwc_dma_t)pcd->ep0_status_buf;
	if (!pcd->ep0_status_buf)
		goto fail;

	/* Allocate the EP0 DMA descriptors */
	pcd->ep0_setup_desc = dma_alloc(128, sizeof(dwc_usb3_dma_desc_t));
	pcd->ep0_setup_desc_dma = (dwc_dma_t)pcd->ep0_setup_desc;
	if (!pcd->ep0_setup_desc)
		goto fail;
	
	pcd->ep0_in_desc = dma_alloc(128, sizeof(dwc_usb3_dma_desc_t));
	pcd->ep0_in_desc_dma = (dwc_dma_t)pcd->ep0_in_desc;
	if (!pcd->ep0_in_desc)
		goto fail;
	
	pcd->ep0_out_desc = dma_alloc(128, sizeof(dwc_usb3_dma_desc_t));
	pcd->ep0_out_desc_dma = (dwc_dma_t)pcd->ep0_out_desc;
	if (!pcd->ep0_out_desc)
		goto fail;

	/* Init the PCD (also enables interrupts and sets Run/Stop bit) */
	retval = dwc_usb3_pcd_init(usb3_dev);
	if (retval) {
		_dprintf("dwc_usb3_pcd_init error\n");
		goto fail;
	}

	return 0;

fail:

	return retval;
}

#if defined(TCC893X)
int USB30DEV_DWC_CORE_Init(USBDEV_T mode)
#else
int USB30DEV_DWC_CORE_Init(void)
#endif
{
	dwc_usb3_device_t *usb3_dev;
	unsigned int addr_ofs = 0xc000;
	int retval = 0;

	usb30_poweron_reset_usb20_mode();
	usb30_software_reset();

	usb3_dev = USB30DEV_Get_Device_t();

	USB30DEV_PHY_Init();
	USB30DEV_PowerOn_Reset();
	gEP_T_Index = 0;
	//================================================
	// Map the DWC_usb3 Core memory into virtual address space.
	//================================================
	usb3_dev->base = (unsigned char *)HwUSBLINK_BASE;

	//================================================
	// Read SNPSID
	//================================================
	udev_trace("+Read SNPSID\n");
	usb3_dev->snpsid = dwc_usb3_get_snpsid(usb3_dev, addr_ofs);
	udev_trace("-Read SNPSID: 0x%08x!\n", usb3_dev->snpsid);

	usb3_dev->event_buf_dma[0] = (dwc_dma_t)dma_alloc(128, DWC_EVENT_BUF_SIZE);
	usb3_dev->event_buf[0] = (unsigned int*)usb3_dev->event_buf_dma[0];

	//================================================
	// Initialize the DWC_usb3 Core.
	//================================================
	if (dwc_usb3_common_init(usb3_dev, usb3_dev->base + addr_ofs, &dwc_usb3_module_params))
	{
		udev_err_debug("CIL initialization failed!\n");
		return -1;
	}

	//================================================
	// Initialize the PCD
	//================================================
	retval = USB30DEV_DWC_PCD_Init(usb3_dev);
	if (retval) {
		udev_err_debug("PCD initialization failed!\n");
		return -1;
	}
#if defined(TCC893X)
	if (mode == MODE_FASTBOOT)
		gUSB30DEV_ISR_MODE = TRUE;
	else
		gUSB30DEV_ISR_MODE = FALSE;
#endif
	USB30DEV_IO_Lock = 0;
	return 0;
}

static void *USB30DEV_IO_Get_QueueDMA_Buffer(unsigned int *pSize)
{
	void *pQueueBuff = NULL;

	pQueueBuff = dma_alloc(4096, 512*1024*2);
	*pSize = 512*1024*2;

	return pQueueBuff;
}

static void USB30DEV_IO_SetLastError(USBDEV_ERROR_T error)
{
	g_USB30GDEV_IO_LastError = error;
}

static int USB30DEV_IO_AllocQueue(PUSBDEV_QUEUE_ALLOC_T pQueueAlloc)
{
	unsigned int i;
	unsigned char *pQueueBuff, *pQueueBuffBase;
	unsigned int uiMaxBuffQueueSize;

	pQueueBuff = (unsigned char*)USB30DEV_IO_Get_QueueDMA_Buffer(&uiMaxBuffQueueSize);
	pQueueBuffBase = pQueueBuff;

	// Queue allocation for EP0
	gUSB30DEV_IO_EpIn[0].bQueueNum = 1;
	gUSB30DEV_IO_EpIn[0].queue[0].buff = pQueueBuff;
	gUSB30DEV_IO_EpIn[0].queue[0].buffSize = QUEUE_EP0_SIZE;
	pQueueBuff = &pQueueBuff[QUEUE_EP0_SIZE];
	
	gUSB30DEV_IO_EpOut[0].bQueueNum = 1;
	gUSB30DEV_IO_EpOut[0].queue[0].buff = pQueueBuff;
	gUSB30DEV_IO_EpOut[0].queue[0].buffSize = QUEUE_EP0_SIZE;
	pQueueBuff = &pQueueBuff[QUEUE_EP0_SIZE];

	for( i=1 ; i<16 ; i++ )
	{
		gUSB30DEV_IO_EpIn[i].bQueueNum = 0;
		gUSB30DEV_IO_EpOut[i].bQueueNum = 0;
	}

	while(pQueueAlloc)
	{
		unsigned char ep_index = pQueueAlloc->bEpAddress&0x7F;
		if(pQueueAlloc->bQueueNum > QUEUE_MAX)
		{
			USB30DEV_IO_SetLastError(USBDEV_ERROR_QUEUE_ALLOC_QUEUE_NUM);
			return FALSE;
		}

		if(pQueueAlloc->bEpAddress&0x80/*IN*/)
		{
			gUSB30DEV_IO_EpIn[ep_index].bQueueNum = pQueueAlloc->bQueueNum;
			for(i=0 ; i<gUSB30DEV_IO_EpIn[ep_index].bQueueNum ; i++)
			{
				gUSB30DEV_IO_EpIn[ep_index].queue[i].buff = pQueueBuff;
				gUSB30DEV_IO_EpIn[ep_index].queue[i].buffSize = pQueueAlloc->uiQueueBuffSize;
				pQueueBuff = &pQueueBuff[pQueueAlloc->uiQueueBuffSize];

			}
		}
		else
		{
			gUSB30DEV_IO_EpOut[ep_index].bQueueNum = pQueueAlloc->bQueueNum;
			gUSB30DEV_IO_EpOut[ep_index].iPrevFirstQ = gUSB30DEV_IO_EpOut[ep_index].bQueueNum;
			for(i=0 ; i<gUSB30DEV_IO_EpOut[ep_index].bQueueNum ; i++)
			{
				gUSB30DEV_IO_EpOut[ep_index].queue[i].buff = pQueueBuff;
				gUSB30DEV_IO_EpOut[ep_index].queue[i].buffSize = pQueueAlloc->uiQueueBuffSize;
				pQueueBuff = &pQueueBuff[pQueueAlloc->uiQueueBuffSize];
			}
		}

		pQueueAlloc = pQueueAlloc->pNext;
	}

	if(((unsigned int)pQueueBuff-(unsigned int)pQueueBuffBase) > uiMaxBuffQueueSize)
	{
		USB30DEV_IO_SetLastError(USBDEV_ERROR_QUEUE_ALLOC_BUFFER_SIZE);
		return FALSE;
	}

	return TRUE;
}

static void USB30DEV_IO_Isr(void);
static unsigned int USB30DEV_IO_IsConnected(void)
{
	dwc_usb3_device_t *usb3_dev;
	dwc_usb3_pcd_t *pcd;
	int state;
	int speed;

	usb3_dev = USB30DEV_Get_Device_t();
	pcd = &usb3_dev->pcd;
	state = dwc_usb3_get_link_state(pcd);
	speed = dwc_usb3_get_device_speed(pcd);
	pcd->speed = speed;

#if defined(TCC893X)
	if (gUSB30DEV_ISR_MODE == FALSE) {
		USB30DEV_IO_Isr();
	}
#else
#if !defined(FEATURE_USB_ISR)
	USB30DEV_IO_Isr();
#endif
#endif
	if (state ==  DWC_LINK_STATE_ON)
		return TRUE;
	else
		return FALSE;
}

static void USB30DEV_IO_Init(void)
{
#if defined(TCC893X)
	dwc_usb3_device_t *usb3_dev;
	
	if (gUSB30DEV_ISR_MODE == TRUE) {
		usb3_dev = USB30DEV_Get_Device_t();
		dwc_usb3_enable_device_interrupts(usb3_dev);
	}
#else
#if defined(FEATURE_USB_ISR)
        {
		dwc_usb3_device_t *usb3_dev;

		usb3_dev = USB30DEV_Get_Device_t();
		dwc_usb3_enable_device_interrupts(usb3_dev);
	}
#endif
#endif
}

static void USB30DEV_IO_Reset(void)
{
	dwc_usb3_device_t *usb3_dev;
	dwc_usb3_pcd_t *pcd;
	dwc_usb3_pcd_ep_t *ep;
	int i;

	usb3_dev = USB30DEV_Get_Device_t();
	pcd = &usb3_dev->pcd;

	/* If UsbReset comes without Disconnect first, fake it, because the
	 * gadget may need to see a disconnect first. The Synopsys UAS gadget
	 * needs this.
	 */
	if (pcd->state != DWC_STATE_UNCONNECTED) {
		//int hibersave = pcd->usb3_dev->core_params->hibernate;

		//pcd->usb3_dev->core_params->hibernate = 0;
		dwc_usb3_clr_eps_enabled(pcd);
		dwc_usb3_pcd_stop(pcd);
		pcd->state = DWC_STATE_UNCONNECTED;
		//pcd->usb3_dev->core_params->hibernate = hibersave;
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

	USB30DEV_ResetQueue();	
}

static void USB30DEV_IO_EP_Stall(unsigned char epAddress)
{
	dwc_usb3_device_t *usb3_dev;	
	dwc_usb3_pcd_t *pcd;
	dwc_usb3_pcd_ep_t *ep;
	int num;
	
	usb3_dev = USB30DEV_Get_Device_t();
	pcd = &usb3_dev->pcd;

	num = (unsigned int)(epAddress & 0x0F);
	if( epAddress & 0x80 )
		ep = pcd->in_ep[num];
	else
		ep = pcd->out_ep[num];
	
	_dprintf("Stall EP%d\n", num);
	
	ep->dwc_ep.is_in = 0;
	dwc_usb3_ep_set_stall(pcd, ep);
	ep->dwc_ep.stopped = 1;
	pcd->ep0state = EP0_IDLE;
	dwc_usb3_ep0_out_start(pcd);
}

static void USB30DEV_IO_EP_Unstall(unsigned char epAddress)
{
	dwc_usb3_device_t *usb3_dev;	
	dwc_usb3_pcd_t *pcd;
	dwc_usb3_pcd_ep_t *ep;
	dwc_usb3_dev_ep_regs_t   *ep_reg;
	int num;
	
	usb3_dev = USB30DEV_Get_Device_t();
	pcd = &usb3_dev->pcd;

	num = (unsigned int)(epAddress & 0x0F);
	if( epAddress & 0x80 )
		ep = pcd->in_ep[num];
	else
		ep = pcd->out_ep[num];

	_dprintf("UnStall EP%d\n", num);
	_dprintf("ep_num=%d is_in=%d\n", ep->dwc_ep.num, ep->dwc_ep.is_in);

	if (ep->dwc_ep.is_in)
		ep_reg = ep->dwc_ep.in_ep_reg;
	else
		ep_reg = ep->dwc_ep.out_ep_reg;

	dwc_usb3_dep_cstall(pcd, ep_reg);
}

static void USB30DEV_IO_EP_UnstallEnable(unsigned char epAddress, unsigned int enable)
{
	_dprintf("[%s%s%s]!!!!!!!!!!\n", CTAG_S, __func__, CTAG_E);
}

static unsigned int USB30DEV_IO_EP_IsStalled(unsigned char epAddress)
{
	_dprintf("[%s%s%s]!!!!!!!!!!\n", CTAG_S, __func__, CTAG_E);
	return TRUE;
}

static void USB30DEV_IO_EP_Flush(USBDEV_ENDPOINT_T *pEP)
{
	_dprintf("[%s%s%s]!!!!!!!!!!\n", CTAG_S, __func__, CTAG_E);
}

static unsigned int USB30DEV_IO_EP0_Read(void *pBuffer, unsigned int bufLenInByte)
{
	_dprintf("[%s%s%s]!!!!!!!!!!\n", CTAG_S, __func__, CTAG_E);
	return 0;
}

static int USB30DEV_IO_Handshake(dwc_usb3_device_t *dev, volatile unsigned int   *ptr,
		      unsigned int mask, unsigned int done)
{
	unsigned int usec = 100000;
	unsigned int result;

	do {
		result = dwc_rd32(dev, ptr);
		if ((result & mask) == done) {
			//_dprintf( "after dalepena=%08x\n", result);
			return 1;
		}

		usec -= 1;
	} while (usec > 0);

	return 0;
}

static unsigned int USB30DEV_IO_EP0_Write(void *pBuffer, unsigned int bufLenInByte)
{
	dwc_usb3_device_t *usb3_dev;	
	dwc_usb3_pcd_t *pcd;
	dwc_usb3_pcd_req_t *req;
	dwc_usb3_pcd_ep_t *ep0;
	unsigned int writePacketSize,PktCnt;
	
	usb3_dev = USB30DEV_Get_Device_t();
	pcd = &usb3_dev->pcd;
	req = pcd->ep0_req;
	ep0 = pcd->ep0;
	
	if (( pBuffer == NULL) && (bufLenInByte == 0))
	{
		//setup_in_status_phase(pcd, NULL, 0);
		// Zero-Length Packet
		USB30DEV_IO_Handshake(pcd->usb3_dev, &pcd->dev_global_regs->dalepena, 1, 0);
		pcd->ep0state = EP0_OUT_WAIT_NRDY;
		ep0->dwc_ep.is_in = 1;
		dwc_usb3_handle_ep0(pcd, req, 0);
	}
	else
	{
		writePacketSize = min(gUSB30DEV_IO_EpIn[0].queue[0].buffSize,bufLenInByte);
		if(writePacketSize)
		{
			PktCnt = (writePacketSize+64-1)>>6/*/64*/;
			ep0_debug("[%s%s%s] PktCnt: %d, EP0 Write Byte: %d\n", CTAG_S, __func__, CTAG_E, PktCnt, bufLenInByte );
		}
		else
		{
			PktCnt = 1;
		}
		
		bufLenInByte -= writePacketSize;
		
		USB30DEV_memcpy(gUSB30DEV_IO_EpIn[0].queue[0].buff, pBuffer, writePacketSize);

		pcd->ep0_status_pending = 1;
		req->dwc_req.length = writePacketSize;
		req->dwc_req.buf[0] = (char *)gUSB30DEV_IO_EpIn[0].queue[0].buff;
		req->dwc_req.bufdma[0] = (dwc_dma_t)gUSB30DEV_IO_EpIn[0].queue[0].buff;
		req->dwc_req.actual = 0;
		ep0->dwc_ep.is_in = 1;

		dwc_usb3_ep0_start_transfer(pcd, pcd->ep0_req);
	}

	return 0;
}

static void USB30DEV_IO_SetAddress(unsigned short address)
{
	dwc_usb3_device_t *usb3_dev;	
	dwc_usb3_pcd_t *pcd;

	usb3_dev = USB30DEV_Get_Device_t();
	pcd = &usb3_dev->pcd;

	//if (ctrl.bmRequestType == UT_DEVICE)
	{
		dwc_usb3_set_address(pcd, address);
		pcd->ep0->dwc_ep.is_in = 1;
		pcd->ep0state = EP0_IN_WAIT_NRDY;
		if (address)
			pcd->state = DWC_STATE_ADDRESSED;
		else
			pcd->state = DWC_STATE_DEFAULT;
	}
}

static void USB30DEV_IO_TestMode(unsigned char value)
{
	_dprintf("[%s%s%s]!!!!!!!!!!\n", CTAG_S, __func__, CTAG_E);
}

static void USB30DEV_IO_EP_IN_Isr(unsigned int nEndPoint)
{
	unsigned char iFirstQ = gUSB30DEV_IO_EpIn[nEndPoint].iFirstQ;
	PUSBDEV_QUEUE_T pFirstQ = &gUSB30DEV_IO_EpIn[nEndPoint].queue[iFirstQ];
	dwc_usb3_device_t *usb3_dev;	
	dwc_usb3_pcd_t *pcd;
	dwc_usb3_pcd_req_t *req;
	dwc_usb3_pcd_ep_t *ep;

	usb3_dev = USB30DEV_Get_Device_t();
	pcd = &usb3_dev->pcd;

	if ( nEndPoint == 0 )
	{
		ep = pcd->ep0;
		req = pcd->ep0_req;
	}
	else
	{
		ep = pcd->in_ep[nEndPoint - 1];
		req = &gUSB30DEV_IO_EpIn[nEndPoint].req;		
	}
	
	//ep_debug("[%s%s] EP [%d]%s\n", CTAG_S, __func__, nEndPoint, CTAG_E);
	ep_debug("Transfer compl req->length: %d,  pLastQ->xferLen: %d req->dwc_req.actual: %d\n", req->dwc_req.length, pFirstQ->xferLen, req->dwc_req.actual);

	pFirstQ->actualLen = req->dwc_req.actual;
	dwc_usb3_request_done(pcd, ep, req, 0);
	
	pFirstQ->state = QUEUE_STATE_DONE;
	if( USB30DEV_IO_EP_IN_StartNextTransfer(nEndPoint) == FALSE ) {
		gUSB30DEV_IO_EpIn[nEndPoint].xferState = XFER_STATE_IDLE;
	}
}

static void USB30DEV_IO_EP_OUT_Isr(unsigned int nEndPoint)
{
	unsigned char iLastQ = gUSB30DEV_IO_EpOut[nEndPoint].iLastQ;
	unsigned char iLastQ_Next = ((iLastQ+1) >= gUSB30DEV_IO_EpOut[nEndPoint].bQueueNum)? 0 : (iLastQ+1);
	PUSBDEV_QUEUE_T pLastQ = &gUSB30DEV_IO_EpOut[nEndPoint].queue[iLastQ];
	dwc_usb3_device_t *usb3_dev;	
	dwc_usb3_pcd_t *pcd;
	dwc_usb3_pcd_req_t *req;
	dwc_usb3_pcd_ep_t *ep;

	usb3_dev = USB30DEV_Get_Device_t();
	pcd = &usb3_dev->pcd;

	if ( nEndPoint == 0 )
	{
		ep = pcd->ep0;
		req = pcd->ep0_req;
	}
	else
	{
		ep = pcd->in_ep[nEndPoint - 1];
		req = &gUSB30DEV_IO_EpOut[nEndPoint].req;		
	}
	
	ep_debug("Transfer compl req->length: %d,  pLastQ->xferLen: %d req->dwc_req.actual: %d\n", req->dwc_req.length, pLastQ->xferLen, req->dwc_req.actual);
	 
	pLastQ->actualLen = req->dwc_req.actual;
	dwc_usb3_request_done(pcd, ep, req, 0);

	pLastQ->state = QUEUE_STATE_DONE;
	gUSB30DEV_IO_EpOut[nEndPoint].xferState = XFER_STATE_IDLE;
	gUSB30DEV_IO_EpOut[nEndPoint].iLastQ = iLastQ_Next;
	USB30DEV_IO_EP_OUT_StartNextTransfer(nEndPoint);
}

/**
 * This interrupt indicates that an EP has a pending interrupt.
 */
void USB30DEV_IO_Handle_EP_ISR(dwc_usb3_pcd_t *pcd, int physep, unsigned int event)
{
	dwc_usb3_pcd_ep_t *ep;
	int epnum, is_in;
	char *dir;

	isr_handle_debug( "[\x1b[1;32m0x%04X\x1b[0m]\n", event & 0xFFFF);

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

	isr_debug( "EP%d-%s: type=%d mps=%d\n",
		   ep->dwc_ep.num, (ep->dwc_ep.is_in ? "IN" : "OUT"),
		   ep->dwc_ep.type, ep->dwc_ep.maxpacket);
/* evnet 
-------------------------------------------------------
| 15 14 13 12 | 11 10  9  8 | 7  6  5  4 | 3  2  1  0 |
-------------------------------------------------------
| Event Status| -  - | Event Type |   EP Phy Num   |ep|
-------------------------------------------------------
0xca:  0000       00       00  11      00  101       0
0x4a:  0000       00       00  01      00  101       0
*/

	switch (event & DWC_DEPEVT_INTTYPE_BITS) {
	//==========================================
	// Transfer Complete 
	//==========================================
	case DWC_DEPEVT_XFER_CMPL << DWC_DEPEVT_INTTYPE_SHIFT:
		ep->dwc_ep.xfer_started = 0;

		if (ep->dwc_ep.type != UE_ISOCHRONOUS) {
			/* Complete the transfer */
			if (epnum == 0)
				USB30DEV_Handle_ep0(pcd,event);
			else
			{
				isr_handle_debug("EP [%d]-%s Transfer Complete -> USB30DEV_Complete_Request\n", ep->dwc_ep.num, dir);
				USB30DEV_Complete_Request(pcd, ep, event);
			}
			
		} else {
			isr_debug("EP [%d] %s xfer complete for ISOC EP!\n", epnum, dir);
		}

		break;

	//==========================================
	// Transfer progress 
	//==========================================
	case DWC_DEPEVT_XFER_IN_PROG << DWC_DEPEVT_INTTYPE_SHIFT:
		if (ep->dwc_ep.type == UE_ISOCHRONOUS) {
			/* Complete the transfer */
			isr_debug("[%s%s: %d%s] ep type: UE_ISOCHRONOUS\n", CTAG_S, __func__, __LINE__, CTAG_E);
			//dwc_usb3_os_complete_request(pcd, ep, event);
		} else {
			isr_debug("EP [%d] %s xfer in progress for non-ISOC EP!\n", epnum, dir);

			/* Complete the transfer */
			if (epnum == 0)
				USB30DEV_Handle_ep0(pcd,event);
			else
			{
				isr_handle_debug("EP [%d]-%s Transfer progress !!!\n", ep->dwc_ep.num, dir);
				USB30DEV_Complete_Request(pcd, ep, event);
			}
		}

		break;

	//==========================================
	// Transfer Not Ready
	//==========================================
	case DWC_DEPEVT_XFER_NRDY << DWC_DEPEVT_INTTYPE_SHIFT:
		isr_debug("EP [%d]-%s xfer not ready, state: %d\n",epnum, dir, pcd->ep0state);

		if (epnum == 0) {
			switch (pcd->ep0state) {
			case EP0_IN_WAIT_GADGET:
				{
					ep0_debug("[%s%s: %d EP0_IN_WAIT_GADGET %s ] \n", CTAG_S, __func__, __LINE__, CTAG_E);					
				}
			case EP0_IN_WAIT_NRDY:
				if (is_in)
				{
					ep0_debug("[%s%s: %d%s] EP0_IN_WAIT_NRDY\n", CTAG_S, __func__, __LINE__, CTAG_E);			
					USB30DEV_Handle_ep0(pcd, event);
				}
				break;
			case EP0_OUT_WAIT_GADGET:
				{
					ep0_debug("[%s%s: %d EP0_OUT_WAIT_GADGET %s ] \n",  CTAG_S, __func__, __LINE__, CTAG_E);					
				}
			case EP0_OUT_WAIT_NRDY:
				if (!is_in)
				{
					ep0_debug("[%s%s: %d%s] EP0_OUT_WAIT_NRDY\n", CTAG_S, __func__, __LINE__, CTAG_E);			
					USB30DEV_Handle_ep0(pcd, event);
				}
				break;
			
			default:
				break;
			}
		} else if (ep->dwc_ep.type == UE_ISOCHRONOUS) {
			isr_debug("EP [%d]-%s xfer not ready for ISOC EP!\n", epnum, dir);
			if (!ep->dwc_ep.xfer_started)
			{
				isr_debug("[%s%s: %d%s] ep type: UE_ISOCHRONOUS\n", CTAG_S, __func__, __LINE__, CTAG_E);
				//dwc_usb3_os_isoc_ep_start(pcd, ep, event);
			}
			break;
		}

		break;

	case DWC_DEPEVT_FIFOXRUN << DWC_DEPEVT_INTTYPE_SHIFT:
		_dprintf("EP [%d]-%s FIFO Underrun Error!\n",
			   epnum, dir);
		break;

	case DWC_DEPEVT_EPCMD_CMPL << DWC_DEPEVT_INTTYPE_SHIFT:
		_dprintf("EP [%d]-%s Command Complete!\n",
			   epnum, dir);
		break;
	}
}

static void USB30DEV_IO_Isr(void)
{
	dwc_usb3_device_t *dev;	

	dev = USB30DEV_Get_Device_t();
	dwc_usb3_handle_event(dev);
}

static unsigned int USB30DEV_IO_EP_Enable(USBDEV_ENDPOINT_T *pEP, unsigned int enable)
{
	int res;
	if (enable)
	{
		res = USB30DEV_DWC_EP_Enable(pEP);
		// Endpoint Backup for Diable EP
		if(gEP_T_Index < 16) {
			memcpy(&g_USBDEV_ENDPOINT_T[gEP_T_Index], pEP, sizeof(USBDEV_ENDPOINT_T));
			++gEP_T_Index;
		}
	}
	else
	{
		res = USB30DEV_DWC_EP_Disable(pEP);
	}
	
	ep_debug("[%s] res = %d\n", __func__, res);
	
	return TRUE;	
}

void USB30DEV_IO_EP_Disable_ALL(void)
{
	unsigned int i;
	
	for( i = 0; i < gEP_T_Index; ++i )
	{
		USB30DEV_IO_EP_Enable( &g_USBDEV_ENDPOINT_T[i], FALSE);
	}

	gEP_T_Index = 0;
}

static unsigned int USB30DEV_IO_EP_SetQueue(USBDEV_ENDPOINT_T *pEP, unsigned int size);
static unsigned int USB30DEV_IO_EP_IsReady(USBDEV_ENDPOINT_T *pEP)
{
	if( pEP->index == 0 )
		return FALSE;

	if( pEP->isIn == TRUE )
	{
		if( gUSB30DEV_IO_EpIn[pEP->index].xferState == XFER_STATE_IDLE )
			return TRUE;
	}
	else
	{
		if( gUSB30DEV_IO_EP_OUT_QueuedFlag_for_IsReady[pEP->index] == TRUE )
		{
			unsigned char iFirstQ = gUSB30DEV_IO_EpOut[pEP->index].iFirstQ;
			PUSBDEV_QUEUE_T pFirstQ = &gUSB30DEV_IO_EpOut[pEP->index].queue[iFirstQ];
			if( pFirstQ->state == QUEUE_STATE_DONE )
				return pFirstQ->actualLen;
		}
		else
		{
			que_debug("USB30DEV_IO_EP_IsReady EP%d\n",pEP->index);
			USB30DEV_IO_EP_SetQueue(pEP,gUSB30DEV_IO_EpOut[pEP->index].MPS);
			gUSB30DEV_IO_EP_OUT_QueuedFlag_for_IsReady[pEP->index] = TRUE;
		}
	}

	return FALSE;
}

static void USB30DEV_IO_EP_IN_StartTransfer(unsigned int nEndPoint)
{
	dwc_usb3_device_t *usb3_dev;	
	dwc_usb3_pcd_t *pcd;
	dwc_usb3_pcd_req_t *req;
	dwc_usb3_pcd_ep_t *ep;

	unsigned int PktCnt,XferSize,maxPacketSize;
	unsigned char iFirstQ = gUSB30DEV_IO_EpIn[nEndPoint].iFirstQ;
	PUSBDEV_QUEUE_T pFirstQ = &gUSB30DEV_IO_EpIn[nEndPoint].queue[iFirstQ];
	
	usb3_dev = USB30DEV_Get_Device_t();
	pcd = &usb3_dev->pcd;

	if ( nEndPoint == 0 )
	{
		ep = pcd->ep0;
		req = pcd->ep0_req;
	}
	else
	{
		ep = pcd->in_ep[nEndPoint - 1];
		req = &gUSB30DEV_IO_EpIn[nEndPoint].req;
	}
	
	if(pFirstQ->xferLen==0)
	{
		XferSize=0;
		PktCnt=1;
	}
	else
	{
		if(nEndPoint==0)
		{
			maxPacketSize = 64;
			XferSize = min(0x7F, min(pFirstQ->buffSize,pFirstQ->xferLen - pFirstQ->actualLen));
			PktCnt=(XferSize+maxPacketSize-1)/maxPacketSize;
			if( PktCnt > 3 )
			{
				XferSize = maxPacketSize*3;
				PktCnt = 3;
			}
		}
		else
		{
			maxPacketSize = ep->dwc_ep.maxpacket;
			XferSize = min(0x7FFFF, min(pFirstQ->buffSize,pFirstQ->xferLen - pFirstQ->actualLen));
			PktCnt=(XferSize+maxPacketSize-1)/maxPacketSize;
			if( PktCnt > 0x3FF )
			{
				XferSize = maxPacketSize*0x3FF;
				PktCnt = 0x3FF;
			}
		}
	}
	
	req->dwc_req.numbuf 	= 1;
	req->dwc_req.buf[0] 	= (char *)pFirstQ->buff;
	req->dwc_req.bufdma[0] 	= (dwc_dma_t)pFirstQ->buff;
	req->dwc_req.buflen[0]	= XferSize;
	req->dwc_req.length 	= pFirstQ->xferLen;

//	ep_debug("[%s%s%s] -> start_transfer\n", CTAG_S, __func__, CTAG_E);
	ep_debug("%sStTerf%s: EP [%d] IN\n", CTAG_S, CTAG_E,  nEndPoint  );
	dwc_usb3_ep_start_transfer(pcd, ep, req, 0);
	pFirstQ->actualLen += XferSize;
}

static unsigned int USB30DEV_IO_EP_IN_StartNextTransfer(unsigned int nEndPoint)
{
	unsigned char iFirstQ = gUSB30DEV_IO_EpIn[nEndPoint].iFirstQ;
	unsigned char iFirstQ_Next = ((iFirstQ+1) >= gUSB30DEV_IO_EpIn[nEndPoint].bQueueNum) ? 0 : (iFirstQ+1);

	if( iFirstQ != gUSB30DEV_IO_EpIn[nEndPoint].iLastQ )
	{
		PUSBDEV_QUEUE_T pFirstQ_Next = &gUSB30DEV_IO_EpIn[nEndPoint].queue[iFirstQ_Next];
		ep_debug("EP [%d] IN iFirstQ changed %d -> %d\n",nEndPoint, gUSB30DEV_IO_EpIn[nEndPoint].iFirstQ, iFirstQ_Next);
		gUSB30DEV_IO_EpIn[nEndPoint].iFirstQ = iFirstQ_Next;
		
		if( pFirstQ_Next->state == QUEUE_STATE_ACTIVATE )
		{
			ep_debug("EP_IN_StartNextTransfer");
			USB30DEV_IO_EP_IN_StartTransfer(nEndPoint);
			return TRUE;
		}
	}

	return FALSE;
}

static void USB30DEV_IO_EP_OUT_StartTransfer(unsigned int nEndPoint)
{
	unsigned int PktCnt,XferSize;
	dwc_usb3_device_t *usb3_dev;	
	dwc_usb3_pcd_t *pcd;
	dwc_usb3_pcd_req_t *req;
	dwc_usb3_pcd_ep_t *ep;

	unsigned char iLastQ = gUSB30DEV_IO_EpOut[nEndPoint].iLastQ;
	PUSBDEV_QUEUE_T pLastQ = &gUSB30DEV_IO_EpOut[nEndPoint].queue[iLastQ];

	XferSize = pLastQ->xferLen;
	PktCnt = (XferSize+gUSB30DEV_IO_EpOut[nEndPoint].MPS-1)/gUSB30DEV_IO_EpOut[nEndPoint].MPS;
	
	usb3_dev = USB30DEV_Get_Device_t();
	pcd = &usb3_dev->pcd;

	if ( nEndPoint == 0 )
	{
		ep = pcd->ep0;
		req = pcd->ep0_req;
	}
	else
	{
		ep = pcd->out_ep[nEndPoint - 1];
		req = &gUSB30DEV_IO_EpOut[nEndPoint].req;
	}

	req->dwc_req.numbuf 	= 1;
	req->dwc_req.buf[0] 	= (char *)pLastQ->buff;
	req->dwc_req.bufdma[0] 	= (dwc_dma_t)pLastQ->buff;
	req->dwc_req.buflen[0]	= pLastQ->xferLen;
	req->dwc_req.length 	= pLastQ->xferLen;

	while (USB30DEV_IO_Lock) {
		_dprintf("==================================>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> io lock\n");
	}
	USB30DEV_IO_Lock = 1;
	ep_debug("%sStTerf%s: EP [%d] OUT\n", CTAG_S, CTAG_E, nEndPoint);
	dwc_usb3_ep_start_transfer(pcd, ep, req, 0);
	USB30DEV_IO_Lock = 0;
	ep_debug("Tgr EP OUT pkt=%d size=%d\n",PktCnt,XferSize);

}

static unsigned int USB30DEV_IO_EP_OUT_StartNextTransfer(unsigned int nEndPoint)
{
	unsigned char iLastQ = gUSB30DEV_IO_EpOut[nEndPoint].iLastQ;
	PUSBDEV_QUEUE_T pLastQ = &gUSB30DEV_IO_EpOut[nEndPoint].queue[iLastQ];

	//ep_debug("EP [%d]\n",nEndPoint);
	
	if( gUSB30DEV_IO_EpOut[nEndPoint].xferState != XFER_STATE_WORKING
		&& pLastQ->state == QUEUE_STATE_IDLE && (gUSB30DEV_IO_EpOut[nEndPoint].xferLeftLength||gUSB30DEV_IO_EpOut[nEndPoint].bZeroPacket) )
	{
		pLastQ->xferLen = min(gUSB30DEV_IO_EpOut[nEndPoint].xferLeftLength,pLastQ->buffSize);
		if(!pLastQ->xferLen)
			gUSB30DEV_IO_EpOut[nEndPoint].bZeroPacket = 0;
		gUSB30DEV_IO_EpOut[nEndPoint].xferLeftLength -= pLastQ->xferLen;
		pLastQ->state = QUEUE_STATE_ACTIVATE;
		gUSB30DEV_IO_EpOut[nEndPoint].xferState = XFER_STATE_WORKING;
		USB30DEV_IO_EP_OUT_StartTransfer(nEndPoint);
		return TRUE;
	}

	return FALSE;
}

static PUSBDEV_QUEUE_T USB30DEV_IO_EP_GetQueue(USBDEV_ENDPOINT_T *pEP)
{
	dwc_usb3_device_t *dev;	
	dwc_usb3_pcd_t *pcd;
	dwc_usb3_pcd_req_t *req;
	dwc_usb3_pcd_ep_t *ep;
	
	dev = USB30DEV_Get_Device_t();
	pcd = &dev->pcd;

	if ( pEP->index == 0 )
	{
		ep = pcd->ep0;
		req = pcd->ep0_req;
	}
	else
	{
		if ( pEP->isIn)
		{
			ep = pcd->in_ep[pEP->index - 1];
			req = &gUSB30DEV_IO_EpIn[pEP->index].req;
		}
		else
		{
			ep = pcd->out_ep[pEP->index - 1];
			req = &gUSB30DEV_IO_EpOut[pEP->index].req;
		}
	}

	if( pEP->isIn )
	{
		unsigned char iLastQ = gUSB30DEV_IO_EpIn[pEP->index].iLastQ;
		unsigned char iLastQ_Next = ((iLastQ+1) >= gUSB30DEV_IO_EpIn[pEP->index].bQueueNum)? 0 : (iLastQ+1);
		while( iLastQ_Next == gUSB30DEV_IO_EpIn[pEP->index].iFirstQ )
		{
			if(USB30DEV_IO_IsConnected() == FALSE)
				return NULL;
		}
		
		que_debug("GetQueue IN EP [%d] iLastQ=%d\n",pEP->index,iLastQ);
		return &gUSB30DEV_IO_EpIn[pEP->index].queue[iLastQ];
	}
	else
	{
		unsigned char iFirstQ = gUSB30DEV_IO_EpOut[pEP->index].iFirstQ;
		PUSBDEV_QUEUE_T pFirstQ = &gUSB30DEV_IO_EpOut[pEP->index].queue[iFirstQ];
		while( pFirstQ->state == QUEUE_STATE_ACTIVATE )
		{
			if(USB30DEV_IO_IsConnected() == FALSE)
				return NULL;			
		}
		
		if( pFirstQ->state == QUEUE_STATE_DONE )
		{
			que_debug("GetQueue OUT EP [%d] iFirstQ=%d size=%d\n",pEP->index,iFirstQ,pFirstQ->actualLen);
			if( gUSB30DEV_IO_EpOut[pEP->index].iPrevFirstQ < gUSB30DEV_IO_EpOut[pEP->index].bQueueNum )
			{
				unsigned char iPrevFirstQ = gUSB30DEV_IO_EpOut[pEP->index].iPrevFirstQ;
				PUSBDEV_QUEUE_T pPrevFirstQ = &gUSB30DEV_IO_EpOut[pEP->index].queue[iPrevFirstQ];
				pPrevFirstQ->actualLen = 0;
				pPrevFirstQ->state = QUEUE_STATE_IDLE;

				dwc_usb3_request_done(pcd, ep, req, 0);

				que_debug("DelQueue iPrevFirstQ=%d",iPrevFirstQ);
				USB30DEV_IO_EP_OUT_StartNextTransfer(pEP->index);
			}
			gUSB30DEV_IO_EpOut[pEP->index].iPrevFirstQ = iFirstQ;
			gUSB30DEV_IO_EpOut[pEP->index].iFirstQ = ((iFirstQ+1) >= gUSB30DEV_IO_EpOut[pEP->index].bQueueNum)? 0 : (iFirstQ+1);
			return pFirstQ;
		}
	}
	return NULL;
}

static unsigned int USB30DEV_IO_EP_SetQueue(USBDEV_ENDPOINT_T *pEP, unsigned int size)
{
	if( pEP->isIn )
	{
		unsigned char iLastQ = gUSB30DEV_IO_EpIn[pEP->index].iLastQ;
		unsigned char iLastQ_Next = ((iLastQ+1) >= gUSB30DEV_IO_EpIn[pEP->index].bQueueNum) ? 0 : (iLastQ+1);
		if( iLastQ_Next != gUSB30DEV_IO_EpIn[pEP->index].iFirstQ )
		{
			PUSBDEV_QUEUE_T pLastQ = &gUSB30DEV_IO_EpIn[pEP->index].queue[iLastQ];
			unsigned int xferLen = min(size,pLastQ->buffSize);

			pLastQ->xferLen = xferLen;
			pLastQ->actualLen = 0;
			pLastQ->state = QUEUE_STATE_ACTIVATE;
			gUSB30DEV_IO_EpIn[pEP->index].iLastQ = iLastQ_Next;

			que_debug("SetQueue IN EP [%d] xferLen=%d iLastQ_Next=%d",pEP->index,xferLen,iLastQ_Next);

			if(gUSB30DEV_IO_EpIn[pEP->index].xferState != XFER_STATE_WORKING)
			{
				gUSB30DEV_IO_EpIn[pEP->index].xferState = XFER_STATE_WORKING;
				que_debug("  -> IN start transfer\n");
				USB30DEV_IO_EP_IN_StartTransfer(pEP->index);
			}

			return xferLen;
		}
	}
	else
	{
		que_debug("SetQueue OUT EP%d\n",pEP->index);

		if( gUSB30DEV_IO_EP_OUT_QueuedFlag_for_IsReady[pEP->index] == TRUE )
		{
			unsigned char iFirstQ = gUSB30DEV_IO_EpOut[pEP->index].iFirstQ;
			PUSBDEV_QUEUE_T pFirstQ = &gUSB30DEV_IO_EpOut[pEP->index].queue[iFirstQ];

			gUSB30DEV_IO_EP_OUT_QueuedFlag_for_IsReady[pEP->index] = FALSE;
			return min(pFirstQ->xferLen,size);
		}
		
		gUSB30DEV_IO_EpOut[pEP->index].xferLeftLength += size;
		if(!size)
			gUSB30DEV_IO_EpOut[pEP->index].bZeroPacket = 1;
		USB30DEV_IO_EP_OUT_StartNextTransfer(pEP->index);
		
		return size;
	}
	return 0;
}

static USBDEV_ERROR_T USB30DEV_IO_GetLastError(void)
{
	return g_USB30GDEV_IO_LastError;
}

static USBDEVICE_IO_DRIVER_T g_USB30DEV_IO_Driver = {
	USB30DEV_IO_C_Version,
	USBDEV_FULL_SPEED,
	USB30DEV_IO_AllocQueue,
	USB30DEV_IO_IsConnected,
	USB30DEV_IO_Init,
	USB30DEV_IO_Reset,
	USB30DEV_IO_EP_Stall,
	USB30DEV_IO_EP_Unstall,
	USB30DEV_IO_EP_UnstallEnable,
	USB30DEV_IO_EP_IsStalled,
	USB30DEV_IO_EP_Flush,
	USB30DEV_IO_EP0_Read,
	USB30DEV_IO_EP0_Write,
	USB30DEV_IO_SetAddress,
	USB30DEV_IO_TestMode,
	USB30DEV_IO_Isr,
	USB30DEV_IO_EP_Enable,
	USB30DEV_IO_EP_IsReady,
	USB30DEV_IO_EP_GetQueue,
	USB30DEV_IO_EP_SetQueue,
	USB30DEV_IO_GetLastError
};

USBDEVICE_IO_DRIVER_T *USB30DEV_IO_GetDriver(void)
{
	return &g_USB30DEV_IO_Driver;
}
