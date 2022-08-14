
#include "usb/usb_manager.h"
#include "platform/reg_physical.h"
int fwdn_connect = 0;

#include "debug.h"
#include <debug.h>
#include <string.h>
#include <stdlib.h>
#include <kernel/thread.h>
#include <kernel/event.h>
#include <dev/udc.h>


extern char FirmwareVersion[];

int check_fwdn_mode(void)
{
	if ( FirmwareVersion[3] != '?')
		return 0;
	else
		return 1;
}

void fwdn_start(void)
{
	dprintf(INFO, "FWDN start\n");

	fwdn_connect = 1;

#ifdef SNOR_INCLUDE
	SNOR_Init();
#endif

#if DISPLAY_SPLASH_SCREEN
	dprintf(INFO, "Display Init: Start\n");
	display_init();
	dprintf(INFO, "Display Init: Done\n");
#endif

	USB_Init(USBDEV_VTC);
	USB_DEVICE_On();

	for(;;)
	{
#if defined(TCC893X) && defined(TCC_USB_30_USE)
		USB_Polling_Isr();
#else
		#if !defined(FEATURE_USB_ISR)
		USB_Isr();
		#endif
#endif
		USB_DEVICE_Run();
	}
}
