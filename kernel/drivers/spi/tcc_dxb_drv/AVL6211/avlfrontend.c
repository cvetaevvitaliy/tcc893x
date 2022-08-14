/*
 *  Driver for AVL6211 demodulator
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

#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include "../../../media/dvb/dvb-core/dvb_frontend.h"
#include "include/IBSP.h"
#include "include/IRx.h"
#include "include/ITuner.h"
#include "include/IDiseqc.h"
#include "include/IBlindscanAPI.h"
#include "AVL6211_API.h"

MODULE_PARM_DESC(DEBUG, "\n\t\t Debug Mode");
static int DEBUG = 0;
module_param(DEBUG, int, S_IRUGO);


/*****************************************************************************
 *
 * Defines
 *
 ******************************************************************************/
#define dprintk(msg...)	 { if (DEBUG) printk( "[AVL6211]" msg); }
#define BOARD_TYPE	14

#define DTV_BLIND_SCAND_START		DTV_MAX_COMMAND + 1
#define DTV_BLIND_SCAND_CANCEL		DTV_MAX_COMMAND + 2
#define DTV_BLIND_SCAND_RESET		DTV_MAX_COMMAND + 3
#define DTV_BLIND_SCAND_GET_STATE	DTV_MAX_COMMAND + 4
#define DTV_BLIND_SCAND_GET_INFO	DTV_MAX_COMMAND + 5


/*****************************************************************************
 *
 * structures
 *
 ******************************************************************************/
struct avl6211 {
	struct dvb_frontend fe;
	struct AVL_DVBSx_Chip AVLChip;
	struct AVL_Tuner Tuner;
	struct AVL_DVBSx_BlindScanAPI_Setting BSsetting;
	int index;

	int i2c;
	int fe_addr;
	int tuner_addr;
	int gpio_fe_power;
	int gpio_fe_reset;
	int gpio_fe_lock;
	int gpio_fe_fault;
	int gpio_lnb_power;
	int gpio_lnb_sp1v;
	int gpio_lnb_s18v;
};


/*****************************************************************************
 *
 * External Functions
 *
 ******************************************************************************/
extern int tcc_dxb_frontend_register(struct dvb_frontend *fe, int board_type);
extern int tcc_dxb_frontend_unregister(struct dvb_frontend *fe);


/*****************************************************************************
 *
 * Variables
 *
 ******************************************************************************/
static struct avl6211 avl6211_fe;


 /*****************************************************************************
 *
 * Functions
 *
 ******************************************************************************/
static int AVL6211_Reset(struct avl6211 *demod)
{
	tcc_gpio_config(demod->gpio_fe_reset, GPIO_FN(0));
	gpio_request(demod->gpio_fe_reset, NULL);
	gpio_direction_output(demod->gpio_fe_reset, 0);
	AVL_DVBSx_IBSP_Delay(300);
	gpio_direction_output(demod->gpio_fe_reset, 1);

	return 0;
}

static int AVL6211_PowerCtrl(struct avl6211 *demod, int on)
{
	tcc_gpio_config(demod->gpio_fe_power, GPIO_FN(0));
	gpio_request(demod->gpio_fe_power, NULL);
	gpio_direction_output(demod->gpio_fe_power, on);

	return 0;
}

static int AVL6211_LnbPowerCtrl(struct avl6211 *demod, int on)
{
	tcc_gpio_config(demod->gpio_lnb_power, GPIO_FN(0));
	gpio_request(demod->gpio_lnb_power, NULL);
	gpio_direction_output(demod->gpio_lnb_power, on);

	return 0;
}

static int avl6211_i2c_gate_ctrl(struct dvb_frontend *fe, int enable)
{
	return 0;
}

static int avl6211_init(struct dvb_frontend *fe)
{
	struct avl6211 *demod = fe->demodulator_priv;
	AVL_DVBSx_ErrorCode r = AVL_DVBSx_EC_OK;

	AVL6211_Reset(demod);
	AVL_DVBSx_IBSP_Delay(100);
	AVL6211_PowerCtrl(demod, 0);
    //This function do all the initialization work.It should be called only once at the beginning.It needn't be recalled when we want to lock a new channel.
	r = AVL6211_Initialize(&demod->AVLChip, &demod->Tuner);
	if(AVL_DVBSx_EC_OK != r)
	{
		dprintk("[%s] Initialization Err:0x%x\n", __func__, r);
		return (r);
	}
	dprintk("[%s] Initialization success\n", __func__);
	AVL_DVBSx_IBSP_Delay(200);

	return (r);
}

static int avl6211_sleep(struct dvb_frontend *fe)
{
	struct avl6211 *demod = fe->demodulator_priv;

	AVL6211_PowerCtrl(demod, 1);

	return 0;
}

static int avl6211_read_status(struct dvb_frontend *fe, fe_status_t * status)
{
	struct avl6211 *demod = fe->demodulator_priv;
	unsigned char s;

	s = AVL6211_GETLockStatus(&demod->AVLChip);
	if(s == 1)
		*status = FE_HAS_LOCK|FE_HAS_SIGNAL|FE_HAS_CARRIER|FE_HAS_VITERBI|FE_HAS_SYNC;
	else
		*status = FE_TIMEDOUT;

	dprintk("[%s] Status = 0x%x\n", __func__, *status);

	return  0;
}

static int avl6211_read_ber(struct dvb_frontend *fe, u32 * ber)
{
	struct avl6211 *demod = fe->demodulator_priv;

#if 0
	*ber = AVL_Get_Quality_Percent(&demod->AVLChip);
#else
	*ber = AVL6211_GETBer(&demod->AVLChip); //This function should only be called if the input signal is a DVBS signal
#endif

	dprintk("[%s] BER = %d\n", __func__, *ber);

	return 0;
}

static int avl6211_read_signal_strength(struct dvb_frontend *fe, u16 *strength)
{
	struct avl6211 *demod = fe->demodulator_priv;

#if 1
	*strength = AVL_Get_Level_Percent(&demod->AVLChip);
#else
	*strength = AVL6211_GETSignalLevel(&demod->AVLChip);
#endif

	dprintk("[%s] STR = %d\n", __func__, *strength);

	return 0;
}

static int avl6211_read_snr(struct dvb_frontend *fe, u16 * snr)
{
	struct avl6211 *demod = fe->demodulator_priv;

#if 0
	*snr = AVL_Get_Quality_Percent(&demod->AVLChip);
#else
	*snr = AVL6211_GETSnr(&demod->AVLChip)/100;
#endif

	dprintk("[%s] SNR = %d\n", __func__, *snr);

	return 0;
}

static int avl6211_read_ucblocks(struct dvb_frontend *fe, u32 * ucblocks)
{
	ucblocks = NULL;

	return 0;
}

static int avl6211_diseqc_reset_overload(struct dvb_frontend* fe)
{
	return  0;
}

static int avl6211_diseqc_send_master_cmd(struct dvb_frontend* fe, struct dvb_diseqc_master_cmd* cmd)
{
	struct avl6211 *demod = fe->demodulator_priv;
	AVL_DVBSx_ErrorCode r = AVL_DVBSx_EC_OK;
	AVL_uchar ucData[8];
	struct AVL_DVBSx_Diseqc_TxStatus TxStatus;
	int i;

	for(i=0;i<cmd->msg_len;i++)
	{
		ucData[i]=cmd->msg[i];
		dprintk("%x ",cmd->msg[i]);
	}
	
	r = AVL_DVBSx_IDiseqc_SendModulationData(ucData, cmd->msg_len, &demod->AVLChip);
	if(r != AVL_DVBSx_EC_OK)
	{
		dprintk("[%s] Send data Err:0x%x\n", __func__, r);
		return (r);
	}

	i = 100;
	do
	{
		i--;
		AVL_DVBSx_IBSP_Delay(1);
		r = AVL_DVBSx_IDiseqc_GetTxStatus(&TxStatus, &demod->AVLChip);
	}
	while((TxStatus.m_TxDone != 1) && i);
	if(r != AVL_DVBSx_EC_OK)
	{
		dprintk("[%s] Output data Err:0x%x\n", __func__, r);
	}		

	return (r);
}

static int avl6211_diseqc_recv_slave_reply(struct dvb_frontend* fe, struct dvb_diseqc_slave_reply* reply)
{
	return  0;
}

static int avl6211_diseqc_send_burst(struct dvb_frontend* fe, fe_sec_mini_cmd_t minicmd)
{
	#define TONE_COUNT				8

	struct avl6211 *demod = fe->demodulator_priv;
	AVL_DVBSx_ErrorCode r = AVL_DVBSx_EC_OK;
 	struct AVL_DVBSx_Diseqc_TxStatus sTxStatus;
	AVL_uchar ucTone = 0;
	int i;

	if(minicmd == SEC_MINI_A)
		ucTone = 1;
	else if(minicmd == SEC_MINI_B)
		ucTone = 0;

  	r = AVL_DVBSx_IDiseqc_SendTone(ucTone, TONE_COUNT, &demod->AVLChip);
	if(r != AVL_DVBSx_EC_OK)
	{
		dprintk("[%s] Send tone %d Err:0x%x\n", __func__, ucTone, r);
		return (r);
	}

	i = 100;
    do
    {
    	i--;
		AVL_DVBSx_IBSP_Delay(1);
	    r = AVL_DVBSx_IDiseqc_GetTxStatus(&sTxStatus, &demod->AVLChip);   //Get current status of the Diseqc transmitter data FIFO.
    }
    while((sTxStatus.m_TxDone != 1) && i);			//Wait until operation finished.
    if(r != AVL_DVBSx_EC_OK)
    {
		dprintk("[%s] Output tone %d Err:0x%x\n", __func__, ucTone, r);
    }

	return (r);
}

static int avl6211_set_tone(struct dvb_frontend* fe, fe_sec_tone_mode_t tone)
{
	struct avl6211 *demod = fe->demodulator_priv;
	AVL_DVBSx_ErrorCode r = AVL_DVBSx_EC_OK;

	if(tone == SEC_TONE_ON)
	{
		r = AVL_DVBSx_IDiseqc_StartContinuous(&demod->AVLChip);
		if(r != AVL_DVBSx_EC_OK)
		{
			dprintk("[%s] Diseqc Start Err:0x%x\n", __func__, r);
		}	
	}
	else
	{
		r = AVL_DVBSx_IDiseqc_StopContinuous(&demod->AVLChip);
		if(r != AVL_DVBSx_EC_OK)
		{
			dprintk("[%s] Diseqc Stop Err:0x%x\n", __func__, r);
		}	
	}

	return r;
}

static int avl6211_set_voltage(struct dvb_frontend* fe, fe_sec_voltage_t voltage)
{
	struct avl6211 *demod = fe->demodulator_priv;
	AVL_DVBSx_ErrorCode r = AVL_DVBSx_EC_OK;	
	AVL_uchar nValue = 1;

	if(voltage == SEC_VOLTAGE_OFF)
	{
		AVL6211_LnbPowerCtrl(demod, 0);//lnb power off
	}
	else
	{
		AVL6211_LnbPowerCtrl(demod, 1);//lnb power on

		if(voltage ==  SEC_VOLTAGE_13)
			nValue = 0;
		else if(voltage == SEC_VOLTAGE_18)
			nValue = 1;

		r = AVL_DVBSx_IDiseqc_SetLNBOut(nValue, &demod->AVLChip);
		if(r != AVL_DVBSx_EC_OK)
		{
			dprintk("[%s] SetLNBOut Err:0x%x\n", __func__, r);
		}
	}
	
	return r;
}

static int avl6211_enable_high_lnb_voltage(struct dvb_frontend* fe, long arg)
{
	return  0;
}

static int avl6211_set_frontend(struct dvb_frontend *fe)
{
	struct dtv_frontend_properties *p = &fe->dtv_property_cache;
	struct avl6211 *demod = fe->demodulator_priv;
	AVL_DVBSx_ErrorCode r = AVL_DVBSx_EC_OK;

	r = AVL6211_LockChannel(&demod->AVLChip, &demod->Tuner, p->frequency, p->symbol_rate);
	if (AVL_DVBSx_EC_OK != r)
	{
		dprintk("[%s] Lock channel failed !\n", __func__);
		return (r);
	}

	dprintk("[%s] Lock channel success !(freq = %dMHz, symbol_rate = %dKHz)\n", __func__, p->frequency/1000, p->symbol_rate/1000);
	return (r);
}

static int avl6211_get_frontend(struct dvb_frontend *fe, struct dvb_frontend_parameters *p)
{
	return 0;
}

static void avl6211_release(struct dvb_frontend *fe)
{
}

static int avl6211_set_property(struct dvb_frontend* fe, struct dtv_property* tvp)
{
	AVL_DVBSx_ErrorCode r = AVL_DVBSx_EC_OK;
	struct avl6211 *demod = fe->demodulator_priv;

	if (tvp->cmd <= DTV_MAX_COMMAND)
		return 0;

	switch (tvp->cmd) {
	case DTV_BLIND_SCAND_START: // blind scan start
		r = AVL_DVBSx_IBlindScanAPI_Start(&demod->AVLChip, &demod->Tuner, &demod->BSsetting);
		if (AVL_DVBSx_EC_OK != r)
		{
			printk("[blind scan] start error![%d]\n", r);
		}
		else
		{
			dprintk("[blind scan] start\n");
		}
		break;
	case DTV_BLIND_SCAND_CANCEL: // blind scan cancel
		r = AVL_DVBSx_IBlindScanAPI_Exit(&demod->AVLChip, &demod->BSsetting);
		if (AVL_DVBSx_EC_OK != r)
		{
			printk("[blind scan] cancel error![%d]\n", r);
		}
		else
		{
			dprintk("[blind scan] cancel\n");
		}
		break;
	case DTV_BLIND_SCAND_RESET: // blind scan reset
		dprintk("[blind scan] reset\n");
		demod->index = 0;
		AVL_DVBSx_IBlindScanAPI_Initialize(&demod->BSsetting);//this function set the parameters blind scan process needed.
		AVL6211_BlindScan_Reset(&demod->AVLChip, &demod->BSsetting);
		break;
	}
	return (AVL_DVBSx_EC_OK == r) ? 1 : -1;
}

static int avl6211_get_property(struct dvb_frontend* fe, struct dtv_property* tvp)
{
	AVL_DVBSx_ErrorCode r = AVL_DVBSx_EC_OK;
	struct avl6211 *demod = fe->demodulator_priv;
	struct AVL_DVBSx_Channel *pChannel;

	if (tvp->cmd <= DTV_MAX_COMMAND)
		return 0;

	switch (tvp->cmd) {
	case DTV_BLIND_SCAND_GET_STATE: // blind scan get state
		r = AVL_DVBSx_IBlindScanAPI_GetCurrentScanStatus(&demod->AVLChip, &demod->BSsetting);
		if (AVL_DVBSx_EC_GeneralFail == r)
		{
			printk("[blind scan] get state error\n");
			tvp->u.data = 2; // Exit
		}
		if (AVL_DVBSx_EC_OK == r)
		{
			dprintk("[blind scan] get state OK\n");
			tvp->u.data = 1; // Get
		}
		if (AVL_DVBSx_EC_Running == r)
		{
			dprintk("[blind scan] get state Wait\n");
			tvp->u.data = 0; // Wait
		}
		r = AVL_DVBSx_EC_OK;
		break;
	case DTV_BLIND_SCAND_GET_INFO: // blind scan get info
		tvp->u.buffer.len = AVL_DVBSx_IBlindscanAPI_GetProgress(&demod->BSsetting);
		while(demod->index < demod->BSsetting.m_uiChannelCount) //display new TP info found in current stage
		{
			pChannel = &demod->BSsetting.channels[demod->index++];
			tvp->u.buffer.reserved1[0] = demod->index;
			tvp->u.buffer.reserved1[1] = pChannel->m_uiFrequency_kHz/1000;
			tvp->u.buffer.reserved1[2] = pChannel->m_uiSymbolRate_Hz/1000;
			dprintk("[blind scan] get info (%d/100) Index=%d, Freq=%dMHz, Sym=%dKHz\n", tvp->u.buffer.len, tvp->u.buffer.reserved1[0], tvp->u.buffer.reserved1[1], tvp->u.buffer.reserved1[2]);
		}
		break;
	}
	return (AVL_DVBSx_EC_OK == r) ? 1 : -1;
}

static struct dvb_frontend_ops avl6211_fe_ops = {

	.info = {
		.name = "TCC DVB-S2 (AVL6211)",
		.type = FE_QPSK,
		.frequency_min = 850000,
		.frequency_max = 2300000,
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

	.delsys = { SYS_DVBS },

	.release = avl6211_release,

	.init = avl6211_init,
	.sleep = avl6211_sleep,
	.i2c_gate_ctrl = avl6211_i2c_gate_ctrl,

	.set_frontend = avl6211_set_frontend,
	.get_frontend = avl6211_get_frontend,

	.read_status = avl6211_read_status,
	.read_ber = avl6211_read_ber,
	.read_signal_strength = avl6211_read_signal_strength,
	.read_snr = avl6211_read_snr,
	.read_ucblocks = avl6211_read_ucblocks,

	.diseqc_reset_overload = avl6211_diseqc_reset_overload,
	.diseqc_send_master_cmd = avl6211_diseqc_send_master_cmd,
	.diseqc_recv_slave_reply = avl6211_diseqc_recv_slave_reply,
	.diseqc_send_burst = avl6211_diseqc_send_burst,
	.set_tone = avl6211_set_tone,
	.set_voltage = avl6211_set_voltage,
	.enable_high_lnb_voltage = avl6211_enable_high_lnb_voltage,

	.set_property = avl6211_set_property,
	.get_property = avl6211_get_property,
};

static int avl6211_fe_init(struct platform_device *pdev, struct avl6211 *demod)
{
	struct resource *res;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "i2c");
	if (!res) {
		return -EINVAL;
	}
	demod->i2c = res->start;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "tuner_addr");
	if (!res) {
		return -EINVAL;
	}
	demod->tuner_addr = res->start;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "fe_addr");
	if (!res) {
		return -EINVAL;
	}
	demod->fe_addr = res->start;

	res = platform_get_resource_byname(pdev, IORESOURCE_IO, "fe_power");
	if (!res) {
		return -EINVAL;
	}
	demod->gpio_fe_power = res->start;

	res = platform_get_resource_byname(pdev, IORESOURCE_IO, "fe_reset");
	if (!res) {
		return -EINVAL;
	}
	demod->gpio_fe_reset = res->start;

	res = platform_get_resource_byname(pdev, IORESOURCE_IO, "fe_lock");
	if (!res) {
		return -EINVAL;
	}
	demod->gpio_fe_lock = res->start;

	res = platform_get_resource_byname(pdev, IORESOURCE_IO, "fe_fault");
	if (!res) {
		return -EINVAL;
	}
	demod->gpio_fe_fault = res->start;

	res = platform_get_resource_byname(pdev, IORESOURCE_IO, "lnb_power");
	if (!res) {
		return -EINVAL;
	}
	demod->gpio_lnb_power = res->start;

	res = platform_get_resource_byname(pdev, IORESOURCE_IO, "lnb_sp1v");
	if (!res) {
		return -EINVAL;
	}
	demod->gpio_lnb_sp1v = res->start;

	res = platform_get_resource_byname(pdev, IORESOURCE_IO, "lnb_s18v");
	if (!res) {
		return -EINVAL;
	}
	demod->gpio_lnb_s18v = res->start;

	tcc_gpio_config(demod->gpio_fe_lock, GPIO_FN(0));
	gpio_request(demod->gpio_fe_lock, NULL);
	gpio_direction_input(demod->gpio_fe_lock);

	tcc_gpio_config(demod->gpio_fe_fault, GPIO_FN(0));
	gpio_request(demod->gpio_fe_fault, NULL);
	gpio_direction_input(demod->gpio_fe_fault);

	tcc_gpio_config(demod->gpio_lnb_sp1v, GPIO_FN(0));
	gpio_request(demod->gpio_lnb_sp1v, NULL);
	gpio_direction_input(demod->gpio_lnb_sp1v);

	tcc_gpio_config(demod->gpio_lnb_s18v, GPIO_FN(0));
	gpio_request(demod->gpio_lnb_s18v, NULL);
	gpio_direction_input(demod->gpio_lnb_s18v);

	return 0;
}

static int avl6211_fe_probe(struct platform_device *pdev)
{
	int ret;

	ret = avl6211_fe_init(pdev, &avl6211_fe);
	if (ret == 0)
	{
		memcpy(&avl6211_fe.fe.ops, &avl6211_fe_ops, sizeof(struct dvb_frontend_ops));
		avl6211_fe.fe.demodulator_priv = &avl6211_fe;
		ret = tcc_dxb_frontend_register(&avl6211_fe.fe, BOARD_TYPE);
	}

	return ret;
}

static int avl6211_fe_remove(struct platform_device *pdev)
{
	tcc_dxb_frontend_unregister(&avl6211_fe.fe);

	return 0;
}

static struct platform_driver avl6211_fe_driver = {
	.probe		= avl6211_fe_probe,
	.remove		= avl6211_fe_remove,
	.driver		= {
		.name	= "avl6211",
		.owner	= THIS_MODULE,
	}
};

static int __init avlfrontend_init(void)
{
	dprintk(KERN_INFO "%s::[%s]\n", __FUNCTION__, __DATE__);

	return platform_driver_register(&avl6211_fe_driver);
}

static void __exit avlfrontend_exit(void)
{    
	dprintk(KERN_INFO "%s\n", __FUNCTION__);

	platform_driver_unregister(&avl6211_fe_driver);
}

module_init(avlfrontend_init);
module_exit(avlfrontend_exit);

MODULE_DESCRIPTION("TCC DVB-S2 (AVL6211)");
MODULE_AUTHOR("C2-G1-3T");
MODULE_LICENSE("GPL");
