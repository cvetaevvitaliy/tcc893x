/****************************************************************************
One line to give the program's name and a brief idea of what it does.
Copyright (C) 2013 Telechips Inc.

This program is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA 02111-1307 USA
****************************************************************************/

#ifndef __RDA5888API_H__
#define __RDA5888API_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "linux/kernel.h"
#include "rda5888drv.h"

extern uint8 g_nRdamtvTimer;

void RDAEXT_DelayMs(uint32 ms);

#define RDAEXT_DebugPrint	printk	//kal_prompt_trace

#define RDAMTV_APP_MODULE	1
#define CAMERA_APP_MODULE	0

#define ATV_STATE	1
#define FM_STATE	0

typedef enum
{
	RDAMTV_POWER_OFF,
	RDAMTV_POWER_ON
}  RDAMTV_POWER_STATE;

/*functions for MMI/ISP/MEDIA*/
unsigned char rdamtv_if_get_cur_module(void);
void rdamtv_if_set_cur_module(unsigned char is_atv_app);
unsigned short rdamtv_if_get_chn_count(void);
void rdamtv_if_func_config(void);


#ifdef __cplusplus
}
#endif

#endif

