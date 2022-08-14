/*
 *  Driver for MXL101 demodulator
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

#define __DVB_CORE__

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/gpio.h>

#include "include/demod_MxL101SF.h"

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

#define BOARD_TYPE	12


/*****************************************************************************
 *
 * structures
 *
 ******************************************************************************/
static struct mxl101_fe {
	struct dvb_frontend fe;
};


/*****************************************************************************
 *
 * Variables
 *
 ******************************************************************************/
static struct mxl101_fe mxl101_fe;


/*****************************************************************************
 *
 * External Functions
 *
 ******************************************************************************/
extern int tcc_dxb_frontend_register(struct dvb_frontend *fe, int board_type);
extern int tcc_dxb_frontend_unregister(struct dvb_frontend *fe);


 /*****************************************************************************
 *
 * Functions
 *
 ******************************************************************************/
int mxl101_get_fe_config(struct mxl101_fe_config *cfg)
{
    struct i2c_adapter *i2c_handle;
    cfg->i2c_id = 1;//frontend_i2c;
    cfg->demod_addr = 0;//frontend_demod_addr;
    cfg->tuner_addr = 0;//frontend_tuner_addr;
    cfg->reset_pin = 0;//frontend_reset;

    i2c_handle = i2c_get_adapter(cfg->i2c_id);
    if (!i2c_handle) {
        printk("cannot get i2c adaptor\n");
        return 0;
    }
    cfg->i2c_adapter = i2c_handle;
	return 1;
}
 
static int mxl101_i2c_gate_ctrl(struct dvb_frontend *fe, int enable)
{
	return 0;
}

static int mxl101_init(struct dvb_frontend *fe)
{
	MxL101SF_Init();
	return 0;
}

static int mxl101_sleep(struct dvb_frontend *fe)
{
	return 0;
}

static int mxl101_read_status(struct dvb_frontend *fe, fe_status_t * status)
{
	if(MxL101SF_GetLock() == 1)
	{
		*status = FE_HAS_LOCK|FE_HAS_SIGNAL|FE_HAS_CARRIER|FE_HAS_VITERBI|FE_HAS_SYNC;
	}
	else
	{
		*status = FE_TIMEDOUT;
	}

	return  0;
}

static int mxl101_read_ber(struct dvb_frontend *fe, u32 * ber)
{
	*ber = MxL101SF_GetBER();
	return 0;
}

static int mxl101_read_signal_strength(struct dvb_frontend *fe, u16 *strength)
{
	*strength = MxL101SF_GetSigStrength();
	return 0;
}

static int mxl101_read_snr(struct dvb_frontend *fe, u16 * snr)
{
	*snr = MxL101SF_GetSNR();
	return 0;
}

static int mxl101_read_ucblocks(struct dvb_frontend *fe, u32 * ucblocks)
{
	ucblocks = NULL;
	return 0;
}

static int mxl101_set_frontend(struct dvb_frontend *fe, struct dvb_frontend_parameters *p)
{
	UINT8 bandwidth;

	switch(p->u.ofdm.bandwidth)
    {
    case BANDWIDTH_6_MHZ:
		bandwidth = 6;
		break;
	case BANDWIDTH_7_MHZ:
		bandwidth = 7;
		break;
    case BANDWIDTH_8_MHZ:
    default:
		bandwidth = 8;
		break;
	}

	MxL101SF_Tune(p->frequency*1000, bandwidth);

	return  0;
}

static int mxl101_get_frontend(struct dvb_frontend *fe, struct dvb_frontend_parameters *p)
{
	return 0;
}

static void mxl101_release(struct dvb_frontend *fe)
{
}

static struct dvb_frontend_ops mxl101_fe_ops = {

	.info = {
		.name = "TCC DVB-T (MxL101)",
		.type = FE_OFDM,
		.frequency_min = 51000,
		.frequency_max = 858000,
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

	.delsys = { SYS_DVBT },

	.release = mxl101_release,

	.init = mxl101_init,
	.sleep = mxl101_sleep,
	.i2c_gate_ctrl = mxl101_i2c_gate_ctrl,

	.set_frontend = mxl101_set_frontend,
	.get_frontend = mxl101_get_frontend,

	.read_status = mxl101_read_status,
	.read_ber = mxl101_read_ber,
	.read_signal_strength = mxl101_read_signal_strength,
	.read_snr = mxl101_read_snr,
	.read_ucblocks = mxl101_read_ucblocks,
};

static int mxl101_fe_init(struct mxl101_fe *fe)
{
	int ret;

	memcpy(&fe->fe.ops, &mxl101_fe_ops, sizeof(struct dvb_frontend_ops));
	fe->fe.demodulator_priv = fe;

	ret = tcc_dxb_frontend_register(&fe->fe, BOARD_TYPE);

	return ret;
}

static void mxl101_fe_release(struct mxl101_fe *fe)
{
	tcc_dxb_frontend_unregister(&fe->fe);
}

static int __init mxlfrontend_init(void)
{
	mxl101_fe_init(&mxl101_fe);

	dprintk(KERN_INFO "%s::[%s][RET = %d]\n", __FUNCTION__,__DATE__);

	return 0;
}

static void __exit mxlfrontend_exit(void)
{    
	dprintk(KERN_INFO "%s\n", __FUNCTION__);

	mxl101_fe_release(&mxl101_fe);
}

module_init(mxlfrontend_init);
module_exit(mxlfrontend_exit);

MODULE_DESCRIPTION("TCC DVB-T (MxL101)");
MODULE_AUTHOR("C2-G1-3T");
MODULE_LICENSE("GPL");
