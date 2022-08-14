/*
 *  Driver for MN88472 demodulator
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

#include <linux/dvb/frontend.h>
#include "../../../media/dvb/dvb-core/dvb_frontend.h"

#include "common/MN_DMD_driver.h"
#include "common/MN_DMD_device.h"
#include "mn88472_sample/MN88472_UserFunction.h"

/*****************************************************************************
 *
 * Defines
 *
 ******************************************************************************/
#if 0
#define dprintk(msg...)	 { printk( "[MN88472]" msg); }
#else
#define dprintk(msg...)	 
#endif

#define BOARD_TYPE	13


/*****************************************************************************
 *
 * structures
 *
 ******************************************************************************/
static struct mn88472_fe {
	DMD_PARAMETER_t param;
	struct dvb_frontend fe;
};


/*****************************************************************************
 *
 * Variables
 *
 ******************************************************************************/
static struct mn88472_fe mn88472_fe;


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
int mn88472_get_fe_config(struct i2c_adapter **i2c_handle)
{
    *i2c_handle = i2c_get_adapter(1);
    if (!i2c_handle) {
        printk("cannot get i2c adaptor\n");
        return 0;
    }
	return 1;
}

static int mn88472_i2c_gate_ctrl(struct dvb_frontend *fe, int enable)
{
	return 0;
}

static int mn88472_init(struct dvb_frontend *fe)
{
	struct mn88472_fe *demod = fe->demodulator_priv;

    DMD_open( &demod->param );
    DMD_init( &demod->param );
	return 0;
}

static int mn88472_sleep(struct dvb_frontend *fe)
{
	return 0;
}

static int mn88472_read_status(struct dvb_frontend *fe, fe_status_t * status)
{
	struct mn88472_fe *demod = fe->demodulator_priv;

	if(DMD_device_scan( &demod->param ) == DMD_E_OK)
	{
		*status = FE_HAS_LOCK|FE_HAS_SIGNAL|FE_HAS_CARRIER|FE_HAS_VITERBI|FE_HAS_SYNC;
	}
	else
	{
		*status = FE_TIMEDOUT;
	}
	return  0;
}

static int mn88472_read_ber(struct dvb_frontend *fe, u32 * ber)
{
	ber = NULL;
	return 0;
}

static int mn88472_read_signal_strength(struct dvb_frontend *fe, u16 *strength)
{
	struct mn88472_fe *demod = fe->demodulator_priv;

    DMD_get_info( &demod->param , DMD_E_INFO_CNR_INT );	
	*strength = demod->param.info[DMD_E_INFO_CNR_INT];
	return 0;
}

static int mn88472_read_snr(struct dvb_frontend *fe, u16 * snr)
{
	snr = NULL;
	return 0;
}

static int mn88472_read_ucblocks(struct dvb_frontend *fe, u32 * ucblocks)
{
	ucblocks = NULL;
	return 0;
}

static int mn88472_set_frontend(struct dvb_frontend *fe, struct dvb_frontend_parameters *p)
{
	struct mn88472_fe *demod = fe->demodulator_priv;
	const struct dtv_frontend_properties *c = &fe->dtv_property_cache;
	int old_bw = demod->param.bw;

    demod->param.freq = p->frequency; // KHz
	demod->param.funit = DMD_E_KHZ;

	switch(p->u.ofdm.bandwidth)
    {
    case BANDWIDTH_6_MHZ:
        demod->param.bw = DMD_E_BW_6MHZ;
        break;
    case BANDWIDTH_7_MHZ:
        demod->param.bw = DMD_E_BW_7MHZ;
        break;
    case BANDWIDTH_8_MHZ:
    default:
        demod->param.bw = DMD_E_BW_8MHZ;
        break;
    }
    
    if(c->delivery_system == SYS_DVBT)
    {
        // DVBT
        if(demod->param.system == DMD_E_DVBT2 || old_bw != demod->param.bw)
        {
           	demod->param.system = DMD_E_DVBT;
           	DMD_set_system( &demod->param );
        }
    }
    else if (c->delivery_system == SYS_DVBT2)
    {
        //DVBT2
        if(demod->param.system == DMD_E_DVBT || old_bw != demod->param.bw)
        {
           	demod->param.system = DMD_E_DVBT2;
           	DMD_set_system( &demod->param );
        }    
    }

    DMD_tune( &demod->param );	
	return  0;
}

static int mn88472_get_frontend(struct dvb_frontend *fe, struct dvb_frontend_parameters *p)
{
	return 0;
}

static void mn88472_release(struct dvb_frontend *fe)
{
}

static int mn88472_set_property(struct dvb_frontend* fe, struct dtv_property* tvp)
{
	struct mn88472_fe *demod = fe->demodulator_priv;
	const struct dtv_frontend_properties *c = &fe->dtv_property_cache;

	switch(tvp->cmd) {
	case DTV_DVBT2_PLP_ID:
		if (c->delivery_system == SYS_DVBT2)
		{
    	    DMD_set_info( &demod->param , DMD_E_INFO_DVBT2_SELECTED_PLP , tvp->u.data );
		}
		break;
	}
	return 0;
}

static int mn88472_get_property(struct dvb_frontend* fe, struct dtv_property* tvp)
{
	struct mn88472_fe *demod = fe->demodulator_priv;
	const struct dtv_frontend_properties *c = &fe->dtv_property_cache;

	switch(tvp->cmd) {
	case DTV_DVBT2_PLP_ID:
		if (c->delivery_system == SYS_DVBT2)
		{
    	    //Check MPLP information
	        int i;
    	    unsigned char pPLPIds[256];
	        unsigned char pNumPLPs;

			int *piPLPNum = tvp->u.buffer.reserved1; // PLP num
			int *piPLPIds = tvp->u.buffer.reserved2; // PLP ID

	        if( DMD_get_dataPLPs( pPLPIds, &pNumPLPs , &demod->param ) == DMD_E_OK )
	        {
		        for(i=0;i<pNumPLPs;i++)
		        {
		             piPLPIds[i] = pPLPIds[i];
		        }
		        *piPLPNum = pNumPLPs;
			}
			else
			{
            	pNumPLPs = 0;
	        }
		}
		break;
	}
	return 0;
}

static struct dvb_frontend_ops mn88472_fe_ops = {

	.info = {
		.name = "TCC DVB-T2 (MN88472)",
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

	.release = mn88472_release,

	.init = mn88472_init,
	.sleep = mn88472_sleep,
	.i2c_gate_ctrl = mn88472_i2c_gate_ctrl,

	.set_frontend = mn88472_set_frontend,
	.get_frontend = mn88472_get_frontend,

	.read_status = mn88472_read_status,
	.read_ber = mn88472_read_ber,
	.read_signal_strength = mn88472_read_signal_strength,
	.read_snr = mn88472_read_snr,
	.read_ucblocks = mn88472_read_ucblocks,

	.set_property = mn88472_set_property,
	.get_property = mn88472_get_property,
};

static int mn88472_fe_init(struct mn88472_fe *fe)
{
	int ret;

	memcpy(&fe->fe.ops, &mn88472_fe_ops, sizeof(struct dvb_frontend_ops));
	fe->fe.demodulator_priv = fe;

	ret = tcc_dxb_frontend_register(&fe->fe, BOARD_TYPE);

	return ret;
}

static void mn88472_fe_release(struct mn88472_fe *fe)
{
	tcc_dxb_frontend_unregister(&fe->fe);
}

static int __init mnfrontend_init(void)
{
	mn88472_fe_init(&mn88472_fe);

	dprintk(KERN_INFO "%s::[%s]\n", __FUNCTION__,__DATE__);

	return 0;
}

static void __exit mnfrontend_exit(void)
{    
	dprintk(KERN_INFO "%s\n", __FUNCTION__);

	mn88472_fe_release(&mn88472_fe);
}

module_init(mnfrontend_init);
module_exit(mnfrontend_exit);

MODULE_DESCRIPTION("TCC DVB-T2 (MN88472)");
MODULE_AUTHOR("C2-G1-3T");
MODULE_LICENSE("GPL");
