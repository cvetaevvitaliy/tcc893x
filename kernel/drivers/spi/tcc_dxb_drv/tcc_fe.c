/*
 *  Driver for TCC Frontend (Dummy)
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

#include "tcc_fe.h"

/*****************************************************************************
 *
 * Defines
 *
 ******************************************************************************/
#if 0
#define dprintk(msg...)	 { printk( "[tcc_fe]" msg); }
#else
#define dprintk(msg...)	 
#endif


/*****************************************************************************
 *
 * structures
 *
 ******************************************************************************/
struct tcc_fe {
	struct dvb_frontend fe;
};

/*****************************************************************************
 *
 * Variables
 *
 ******************************************************************************/
static struct tcc_fe tcc_fe;


/*****************************************************************************
 *
 * External Functions
 *
 ******************************************************************************/


 /*****************************************************************************
 *
 * Functions
 *
 ******************************************************************************/
static int tcc_i2c_gate_ctrl(struct dvb_frontend *fe, int enable)
{
	return 0;
}

static int tcc_read_status(struct dvb_frontend* fe, fe_status_t* status)
{
	return 0;
}

static int tcc_read_ber(struct dvb_frontend* fe, u32* ber)
{
	return 0;
}

static int tcc_read_signal_strength(struct dvb_frontend* fe, u16* strength)
{
	return 0;
}

static int tcc_read_snr(struct dvb_frontend* fe, u16* snr)
{
	return 0;
}

static int tcc_read_ucblocks(struct dvb_frontend* fe, u32* ucblocks)
{
	return 0;
}

static int tcc_get_frontend(struct dvb_frontend* fe)
{
	return 0;
}

static int tcc_set_frontend(struct dvb_frontend* fe)
{
	return 0;
}

static int tcc_sleep(struct dvb_frontend* fe)
{
	tcc_tsif_interface_close();
	return 0;
}

static int tcc_init(struct dvb_frontend* fe)
{
	tcc_tsif_interface_open(fe->dvb->device);
	return 0;
}

static void tcc_release(struct dvb_frontend* fe)
{
}

static struct dvb_frontend_ops tcc_fe_ops = {

	.info = {
		.name = "TCC DXB",
		.type = FE_OFDM,
		.frequency_min = 0,
		.frequency_max = 0,
		.frequency_stepsize = 0,           /* kHz for QPSK frontends */
		.frequency_tolerance = 0,
		.symbol_rate_min = 0,
		.symbol_rate_max = 0,
		.caps =
			FE_CAN_FEC_1_2 | FE_CAN_FEC_2_3 | FE_CAN_FEC_3_4 |
			FE_CAN_FEC_5_6 | FE_CAN_FEC_7_8 | FE_CAN_FEC_AUTO |
			FE_CAN_QPSK | FE_CAN_QAM_AUTO |
			FE_CAN_TRANSMISSION_MODE_AUTO |
			FE_CAN_GUARD_INTERVAL_AUTO |
			FE_CAN_HIERARCHY_AUTO |
			FE_CAN_RECOVER |
			FE_CAN_MUTE_TS
	},

	.release = tcc_release,

	.init = tcc_init,
	.sleep = tcc_sleep,
	.i2c_gate_ctrl = tcc_i2c_gate_ctrl,

	.set_frontend = tcc_set_frontend,
	.get_frontend = tcc_get_frontend,

	.read_status = tcc_read_status,
	.read_ber = tcc_read_ber,
	.read_signal_strength = tcc_read_signal_strength,
	.read_snr = tcc_read_snr,
	.read_ucblocks = tcc_read_ucblocks,
};

static int tcc_fe_init(struct tcc_fe *fe)
{
	int ret;
	struct tcc_dxb_instance *tcc_dxb = tcc_dxb_get_instance();

	memcpy(&fe->fe.ops, &tcc_fe_ops, sizeof(struct dvb_frontend_ops));
	fe->fe.demodulator_priv = fe;

	ret = dvb_register_frontend(&tcc_dxb->adapter, &fe->fe);
    if (ret < 0)
    {
        printk("tcc_dxb_kernel : Frontend registration failed!\n");
        dvb_frontend_detach(&fe->fe);
    }
	
	return ret;
}

static void tcc_fe_release(struct tcc_fe *fe)
{
    dvb_unregister_frontend(&fe->fe);
    dvb_frontend_detach(&fe->fe);
}

int tccfrontend_init(void)
{
	tcc_fe_init(&tcc_fe);

	return 0;
}

void tccfrontend_exit(void)
{    
	tcc_fe_release(&tcc_fe);
}