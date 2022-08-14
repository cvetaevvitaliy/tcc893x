/*
 *  Driver for TCC Tuner
 *
 *  Written by C2-G1-3T
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.=
 */

#include <linux/interrupt.h>

#include "tcc_dmx.h"
#include "tcc_tsif_interface.h"
#include "tcc_dxb_kernel.h"

#include "tcc_tuner.h"

#include "../drivers/char/tcc_dxb_ctrl/tcc_dxb_control.h"

/*****************************************************************************
 *
 * Defines
 *
 ******************************************************************************/
#if 0
#define dprintk(msg...)	 { printk( "[tcc_tuner]" msg); }
#else
#define dprintk(msg...)	 
#endif


/*****************************************************************************
 *
 * structures
 *
 ******************************************************************************/


/*****************************************************************************
 *
 * Variables
 *
 ******************************************************************************/
static int giDeviceIdx = 0;
static int giBoardType = -1;


/*****************************************************************************
 *
 * External Functions
 *
 ******************************************************************************/
extern int tcc_dxb_ctrl_ioctl_ex(unsigned int cmd, unsigned long arg);


 /*****************************************************************************
 *
 * Functions
 *
 ******************************************************************************/
static int tcc_tuner_release(struct dvb_frontend *fe)
{
	return 0;
}

static int tcc_tuner_init(struct dvb_frontend *fe)
{
	int ret = 0;

	giDeviceIdx = 0;
	giBoardType = tcc_dxb_get_board_type();

	if (giBoardType >= 0)
	{
		ret = tcc_dxb_ctrl_ioctl_ex(IOCTL_DXB_CTRL_SET_BOARD, (unsigned long)&giBoardType);
/*
		if (ret == 0)
			ret = tcc_dxb_ctrl_ioctl_ex(IOCTL_DXB_CTRL_ON, (unsigned long)&giDeviceIdx);
*/
	}

	if (ret == 0)
	{
		ret = tcc_tsif_interface_open(fe->dvb->device);
	}

	return ret;
}

static int tcc_tuner_sleep(struct dvb_frontend *fe)
{
	tcc_tsif_interface_close();

	if (giBoardType >= 0)
	{
/*
		tcc_dxb_ctrl_ioctl_ex(IOCTL_DXB_CTRL_OFF, (unsigned long)&giDeviceIdx);
*/
	}
	return 0;
}

static struct dvb_tuner_ops tcc_tuner_ops = {

	.info = {
		.name = "TCC TUNER",
	
		.frequency_min = 0,
		.frequency_max = 0,
		.frequency_step = 0,
	
		.bandwidth_min = 0,
		.bandwidth_max = 0,
		.bandwidth_step = 0,
	},

	.release = tcc_tuner_release,
	.init = tcc_tuner_init,
	.sleep = tcc_tuner_sleep,
};

struct dvb_tuner_ops* tcc_dxb_get_tuner_pos(void)
{
	return &tcc_tuner_ops;
}
