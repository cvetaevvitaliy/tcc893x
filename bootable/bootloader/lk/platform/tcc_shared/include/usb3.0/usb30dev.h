#ifndef _USB30CORE_H
#define _USB30CORE_H

int 	USB30DEV_dwc_core_Init(void);
//extern void 	USB30DEV_handle_ep_intr(dwc_usb3_pcd_t *pcd, int physep, unsigned int event);

#define DEBUG_EP0			2
#define DEBUG_EP			2
#define DEBUG_QUE			2
#define DEBUG_ISR			2
#define DEBUG_ISR_HANDLE		2
#define DEBUG_CIL_INTR_EVENT		2

#endif //_USB30CORE_H

