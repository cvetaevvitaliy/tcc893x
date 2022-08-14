/*
 * Battery charger driver for TCC270
 *
 * Copyright (C) 2013 Telechips.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/notifier.h>
#include <linux/usb.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/input.h>
#include <linux/power/tcc-sply.h>
#include <linux/regulator/tcc270.h>
#include <asm/div64.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#define DBG_TCC_PSY 1
#if  DBG_TCC_PSY
#define DBG_PSY_MSG(format,args...)   printk("[TCC270]"format,##args)
#else
#define DBG_PSY_MSG(format,args...)   do {} while (0)
#endif

#define BATTERY_CAP_CUR	3000
u8 irq_thres[LAST_TYPE];

void tcc_charger_update_state(struct tcc_charger *charger);
void tcc_charger_update(struct tcc_charger *charger);

//#include "tcc-rw.h"

struct tcc_charger *charger_tcc270;
static u32 base_vol = 0;

#ifdef CONFIG_HAS_EARLYSUSPEND
static struct early_suspend tcc_early_suspend;
#endif

const int tcc270_bat_voltage_cap[] = {
	41000,
	40300,
	39600,  
	38900,	
	38200,	
	37500,	
	36800,  
	36100, 
	35400,  
	34700,
	34000,
};

const int tcc270_bat_voltage_cap_ac[] = {
	43000,
	42200,
	41350,  
	41400,	
	39300,	
	38600,	
	38000,  
	37500, 
	37000,  
	36500,
	36000,
};

static uint8_t tcc270_vol2cap(int ocv)
{
	if(ocv >= OCVVOLF)
	{
		return OCVREGF;
	}
	else if(ocv < OCVVOL0)
	{
		return OCVREG0;
	}
	else if(ocv < OCVVOL1)
	{
		return OCVREG0 + (OCVREG1 - OCVREG0) * (ocv - OCVVOL0) / (OCVVOL1 - OCVVOL0);
	}
	else if(ocv < OCVVOL2)
	{
		return OCVREG1 + (OCVREG2 - OCVREG1) * (ocv - OCVVOL1) / (OCVVOL2 - OCVVOL1);
	}
	else if(ocv < OCVVOL3)
	{
		return OCVREG2 + (OCVREG3 - OCVREG2) * (ocv - OCVVOL2) / (OCVVOL3 - OCVVOL2);
	}
	else if(ocv < OCVVOL4)
	{
		return OCVREG3 + (OCVREG4 - OCVREG3) * (ocv - OCVVOL3) / (OCVVOL4 - OCVVOL3);
	}
	else if(ocv < OCVVOL5)
	{
		return OCVREG4 + (OCVREG5 - OCVREG4) * (ocv - OCVVOL4) / (OCVVOL5 - OCVVOL4);
	}
	else if(ocv < OCVVOL6)								 
	{
		return OCVREG5 + (OCVREG6 - OCVREG5) * (ocv - OCVVOL5) / (OCVVOL6 - OCVVOL5);
	}
	else if(ocv < OCVVOL7)
	{
		return OCVREG6 + (OCVREG7 - OCVREG6) * (ocv - OCVVOL6) / (OCVVOL7 - OCVVOL6);
	}
	else if(ocv < OCVVOL8)
	{
		return OCVREG7 + (OCVREG8 - OCVREG7) * (ocv - OCVVOL7) / (OCVVOL8 - OCVVOL7);
	}
	else if(ocv < OCVVOL9)
	{
		return OCVREG8 + (OCVREG9 - OCVREG8) * (ocv - OCVVOL8) / (OCVVOL9 - OCVVOL8);
	}
	else if(ocv < OCVVOLA)
	{
		return OCVREG9 + (OCVREGA - OCVREG9) * (ocv - OCVVOL9) / (OCVVOLA - OCVVOL9);
	}
	else if(ocv < OCVVOLB)
	{
		return OCVREGA + (OCVREGB - OCVREGA) * (ocv - OCVVOLA) / (OCVVOLB - OCVVOLA);
	}
	else if(ocv < OCVVOLC)
	{
		return OCVREGB + (OCVREGC - OCVREGB) * (ocv - OCVVOLB) / (OCVVOLC - OCVVOLB);
	}
	else if(ocv < OCVVOLD)
	{
		return OCVREGC + (OCVREGD - OCVREGC) * (ocv - OCVVOLC) / (OCVVOLD - OCVVOLC);
	}
	else if(ocv < OCVVOLE)
	{
		return OCVREGD + (OCVREGE - OCVREGD) * (ocv - OCVVOLD) / (OCVVOLE - OCVVOLD);
	}
	else if(ocv < OCVVOLF)
	{
		return OCVREGE + (OCVREGF - OCVREGE) * (ocv - OCVVOLE) / (OCVVOLF - OCVVOLE);
	}
	else
	{
		return 1;
	}
}

static int tcc270_vol2cap2(int temp)
{
	const int *pbattery_vols;
	int info_temp;
	temp = temp * 10;
	if( charger_tcc270->ac_det == 1 )
		pbattery_vols = tcc270_bat_voltage_cap_ac;
	else
		pbattery_vols = tcc270_bat_voltage_cap;
	
	if(temp > pbattery_vols[0])
	{
		info_temp = 100;
		printk("info_temp: %d\n",info_temp);
		return info_temp;
	}
	else if( temp > pbattery_vols[1] && temp <= pbattery_vols[0])
	{
		info_temp = 90 + (temp - pbattery_vols[1])/((pbattery_vols[0] - pbattery_vols[1])/10);
		printk("info_temp: %d\n",info_temp);
		return info_temp;
	}
	else if( temp > pbattery_vols[2] && temp <= pbattery_vols[1])
	{
		info_temp = 80 + (temp - pbattery_vols[2])/((pbattery_vols[1] - pbattery_vols[2])/10);
		printk("info_temp: %d\n",info_temp);
		return info_temp;
	}
	else if( temp > pbattery_vols[3] && temp <= pbattery_vols[2])
	{
		info_temp = 70 + (temp - pbattery_vols[3])/((pbattery_vols[2] - pbattery_vols[3])/10);
		printk("info_temp: %d\n",info_temp);
		return info_temp;
	}
	else if( temp > pbattery_vols[4] && temp <= pbattery_vols[3])
	{
		info_temp = 60 + (temp - pbattery_vols[4])/((pbattery_vols[3] - pbattery_vols[4])/10);
		printk("info_temp: %d\n",info_temp);
		return info_temp;
	}
	else if( temp > pbattery_vols[5] && temp <= pbattery_vols[4])
	{
		info_temp = 50 + (temp - pbattery_vols[5])/((pbattery_vols[4] - pbattery_vols[5])/10);
		printk("info_temp: %d\n",info_temp);
		return info_temp;
	}
	else if( temp > pbattery_vols[6] && temp <= pbattery_vols[5])
	{
		info_temp = 40 + (temp - pbattery_vols[6])/((pbattery_vols[5] - pbattery_vols[6])/10);
		printk("info_temp: %d\n",info_temp);
		return info_temp;
	}
	else if( temp > pbattery_vols[7] && temp <= pbattery_vols[6])
	{
		info_temp = 30 + (temp - pbattery_vols[7])/((pbattery_vols[6] - pbattery_vols[7])/10);
		printk("info_temp: %d\n",info_temp);
		return info_temp;
	}
	else if( temp > pbattery_vols[8] && temp <= pbattery_vols[7])
	{
		info_temp = 20 + (temp - pbattery_vols[8])/((pbattery_vols[7] - pbattery_vols[8])/10);
		printk("info_temp: %d\n",info_temp);
		return info_temp;
	}
	else if( temp >= pbattery_vols[9] && temp <= pbattery_vols[8])
	{
		info_temp = 10 + (temp - pbattery_vols[9])/((pbattery_vols[8] - pbattery_vols[9])/10);
		printk("info_temp: %d\n",info_temp);
		return info_temp;
	}
	else if( temp >= pbattery_vols[10] && temp <= pbattery_vols[9])
	{
		info_temp = (temp - pbattery_vols[10])/((pbattery_vols[9] - pbattery_vols[10])/10);
		printk("info_temp: %d\n",info_temp);
		return info_temp;
	}
	else
	{
		info_temp = 2;
		printk("info_temp: %d\n",info_temp);
		return info_temp;
	}
}


void tcc270_set_status(int status)
{
  charger_tcc270->status = status; 
}

void tcc270_set_online(bool present)
{
  charger_tcc270->online = present;
}

static int tcc270_chg_i2c_read(char* rxData, int rxLength)
{
	int rc;
	struct i2c_client *client = charger_tcc270->master;
	struct i2c_adapter *adap = charger_tcc270->master->adapter;
	struct i2c_msg msg[2] = {
	//msg 1  send the addr + cmd reg addr
	{
		.addr = client->addr,
		.flags = client-> flags,
		.len = 1,
		.buf = rxData,
	},
	//msg 2 send the addr & it will return reg addr data
	{
		.addr = client->addr,
		.flags = client->flags | I2C_M_RD,
		.len = rxLength,
		.buf = rxData,
	},
	};
	rc = i2c_transfer(adap, msg, 2);
	return rc;
}
EXPORT_SYMBOL(tcc270_chg_i2c_read);

static int tcc270_chg_i2c_write(char* txData, int txLength)
{
	int rc;
	struct i2c_client *client = charger_tcc270->master;
	struct i2c_adapter *adap = charger_tcc270->master->adapter;
	struct i2c_msg msg= {
		//data must include one reg addr and one or several datas into data buff
		.addr = client->addr,
		.flags = client->flags,
		.len = txLength,
		.buf = txData,
	};
	rc = i2c_transfer(adap, &msg, 1);
	return rc;
}
EXPORT_SYMBOL(tcc270_chg_i2c_write);

static int tcc270_read_reg(u8 reg, u8 *data, u8 len)
{
	struct i2c_adapter *adap = charger_tcc270->master->adapter;
	struct i2c_msg msgs[2];
	int ret;
	
	msgs[0].addr = charger_tcc270->master->addr;
	msgs[0].flags = charger_tcc270->master->flags;
	msgs[0].len = 1;
	msgs[0].buf = &reg;

	msgs[1].addr = charger_tcc270->master->addr;
	msgs[1].flags = charger_tcc270->master->flags | I2C_M_RD;
	msgs[1].len = len;
	msgs[1].buf = data;
	
	ret = i2c_transfer(adap, msgs, 2);
	 
	return (ret == 2)? len : ret;									 
}

static int tcc270_write_reg(u8 reg, u8 *data, u8 len)
{
	struct i2c_adapter *adap = charger_tcc270->master->adapter;
	struct i2c_msg msg;
	int ret;
	char *tx_buf = (char *)kmalloc(len + 1, GFP_KERNEL);
	
	if(!tx_buf)
		return -ENOMEM;
	tx_buf[0] = reg;
	memcpy(tx_buf+1, data, len);
	
	msg.addr = charger_tcc270->master->addr;
	msg.flags = charger_tcc270->master->flags;
	msg.len = len + 1;
	msg.buf = (char *)tx_buf;

	ret = i2c_transfer(adap, &msg, 1);
	kfree(tx_buf);
	return (ret == 1) ? len : ret; 
}


void bubblesort(unsigned int Number[],unsigned int num)
{
	int i,j;
	unsigned int temp;
	for(i=0 ; i<(int)(num-1) ; i++)
	{
		for(j=i+1;j<(int)num;j++)
		{
			if(Number[j] != 0 &&  Number[i]>Number[j])
			{
				temp   = Number[i];
				Number[i] = Number[j];
				Number[j] = temp;
			}
		}
	}
}

static int tcc270_chg_set_config(void)
{
	int ret = 0;
	char xfer[7] = {0};
	struct chgctl_type* chgc = &tcc270_chargectl;

	dbg("%s: ++\n", __func__);
	xfer[0] = TCC270_CHGCTL2_REG;
	mutex_lock(&charger_tcc270->mutex);
	memcpy(&xfer[1], &chgc->chgctl2, 6);
	mutex_unlock(&charger_tcc270->mutex);
	ret = tcc270_chg_i2c_write(xfer, 7);
	if (ret < 0)
	{
		dbg("write xfer fail\n");
		return -1;
	}

	xfer[0] = TCC270_IRQ1ENABLE_REG;
	mutex_lock(&charger_tcc270->mutex);
	memcpy(&xfer[1], &chgc->chgirqenable1, 1);
	mutex_unlock(&charger_tcc270->mutex);
	ret = tcc270_chg_i2c_write(xfer, 2);
	if (ret < 0)
	{
		dbg("write xfer fail\n");
		return -1;
	}
	
	xfer[0] = TCC270_IRQ1ENABLE_REG;
	mutex_lock(&charger_tcc270->mutex);
	memcpy(&xfer[1], &chgc->chgirqenable2, 1);
	mutex_unlock(&charger_tcc270->mutex);
	ret = tcc270_chg_i2c_write(xfer, 2);
	if (ret < 0)
	{
		dbg("write xfer fail\n");
		return -1;
	}

	xfer[0] = TCC270_IRQ3ENABLE_REG;
	mutex_lock(&charger_tcc270->mutex);
	memcpy(&xfer[1], &chgc->chgirqenable3, 1);
	mutex_unlock(&charger_tcc270->mutex);
	ret = tcc270_chg_i2c_write(xfer, 2);
	if (ret < 0)
	{
		dbg("write xfer fail\n");
		return -1;
	}

	dbg("%s: --\n", __func__);
	return 0;
}

static int tcc270_chg_get_config(int option)
{
	int ret = 0;
	int i;
	char xfer[6] = {0};
	struct chgctl_type* chgc = &tcc270_chargectl;

	dbg("%s: ++\n", __func__);
	if (option){
		xfer[0] = TCC270_CHGCTL2_REG;
		ret = tcc270_chg_i2c_read(xfer, 6);
		if (ret < 0)
		{
			dbg("read xfer fail\n");
			return -1;
		}
		mutex_lock(&charger_tcc270->mutex);
		memcpy(&(chgc->chgctl2), xfer, 6);
		mutex_unlock(&charger_tcc270->mutex);

		xfer[0] = TCC270_IRQ1ENABLE_REG;
		ret = tcc270_chg_i2c_read(xfer, 1);
		if (ret < 0)
		{
			dbg("read xfer fail\n");
			return -1;
		}
		mutex_lock(&charger_tcc270->mutex);
		memcpy(&(chgc->chgirqenable1), xfer, 1);
		mutex_unlock(&charger_tcc270->mutex);

		xfer[0] = TCC270_IRQ2ENABLE_REG;
		ret = tcc270_chg_i2c_read(xfer, 1);
		if (ret < 0)
		{
			dbg("read xfer fail\n");
			return -1;
		}
		mutex_lock(&charger_tcc270->mutex);
		memcpy(&(chgc->chgirqenable2), xfer, 1);
		mutex_unlock(&charger_tcc270->mutex);

		xfer[0] = TCC270_IRQ3ENABLE_REG;
		ret = tcc270_chg_i2c_read(xfer, 1);
		if (ret < 0)
		{
			dbg("read xfer fail\n");
			return -1;
		}
		mutex_lock(&charger_tcc270->mutex);
		memcpy(&(chgc->chgirqenable3), xfer, 1);
		mutex_unlock(&charger_tcc270->mutex);
	}

	mutex_lock(&charger_tcc270->mutex);
	memcpy(xfer, &(chgc->chgctl2), 6);
	for (i=0; i<6; i++)
		dbg("%s: chgctl%d, value = 0x%02x\n", __func__, i+2, xfer[i]);
	dbg("%s: irqenable1 value = 0x%02x\n", __func__, chgc->chgirqenable1);
	dbg("%s: irqenable2 value = 0x%02x\n", __func__, chgc->chgirqenable2);
	dbg("%s: irqenable3 value = 0x%02x\n", __func__, chgc->chgirqenable3);
	mutex_unlock(&charger_tcc270->mutex);
	dbg("%s: --\n", __func__);
	return 0;
}

static int tcc270_chg_set_current(int chg_current)
{
	int ret;
	char xfer[2];
	struct chgctl_type* chgc = &(tcc270_chargectl);
	unsigned char old_icc;
	int cn;

	dbg("%s: ++\n", __func__);
	mutex_lock(&charger_tcc270->mutex);
	if (chg_current < chgc->charge_current_min || chg_current > chgc->charge_current_max)
	{
		dbg("no valid charge_current %d\n", chg_current);
		goto charger_set_current_fail;
	}
	if (chg_current == chgc->charge_current)
	{
		dbg("No need to set the same current twice\n");
		goto charger_set_current_fail;
	}

	old_icc = chgc->chgctl4.icc;
	cn = (chg_current - chgc->charge_current_min + chgc->charge_current_step/2)/chgc->charge_current_step;
	chgc->chgctl4.icc = (unsigned char)cn;

	xfer[0]=TCC270_CHGCTL4_REG;
	memcpy(&xfer[1], &(chgc->chgctl4), 1);
	ret = tcc270_chg_i2c_write(xfer,2);
	if (ret<0)
	{
		chgc->chgctl4.icc = old_icc; // if write fail , restore to the original value
		pr_err("%s: set charge current fail\n", __func__);
		goto charger_set_current_fail;
	}

	charger_set_current_fail:
	mutex_unlock(&charger_tcc270->mutex);
	dbg("%s: --\n", __func__);
	return 0;
}

static int tcc270_chg_get_current(void)
{
	int ret;
	struct chgctl_type* chgc = &(tcc270_chargectl);
	dbg("%s: ++\n", __func__);
	mutex_lock(&charger_tcc270->mutex);
	dbg("charge_current = %d\n", chgc->charge_current);
	ret = chgc->charge_current;
	mutex_unlock(&charger_tcc270->mutex);
	dbg("%s: --\n", __func__);
	return ret;
}
static int tcc270_chg_reg_init(void)
{
	struct chgctl_type* chgc = &(tcc270_chargectl);
	/* TODO */
	/* charger config and status enable bit definition */
	dbg("%s: ++\n", __func__);
	chgc->set_config();
	dbg("%s: --\n", __func__);
	return 0;
}

int tcc_get_temp(struct tcc_charger *charger)
{
	return charger_tcc270->ext_temp;
}
EXPORT_SYMBOL(tcc_get_temp);

static void tcc270_get_vcell(struct i2c_client *client)
{
  u8 data[2];
	
  if (tcc270_read_reg(TCC270_REG_VCELL_MSB, data, 2) < 0){
	printk(KERN_ERR "%s: Failed to read Voltage\n", __func__);
  }
		
  charger_tcc270->vcell = ((data[0] << 8) + data[1]) * 61 / 100;
	charger_tcc270->curr_offset = (15444 * charger_tcc270->vcell - 27444000) / 10000;
		
#ifdef CONFIG_TCC270_DBG
  printk(KERN_INFO "[TCC270] vcell: %d, offset: %d\n", charger_tcc270->vcell, charger_tcc270->curr_offset);
#endif
}
static void tcc270_get_current(struct i2c_client *client)
{
  u8 data[2];
  u16 temp;
  int sign = 0;

  if (tcc270_read_reg(TCC270_REG_CURRENT_MSB, data, 2) < 0) {
	printk(KERN_ERR "%s: Failed to read CURRENT\n", __func__);
  }

  temp = (data[0]<<8) | data[1];
  if (data[0] & (1 << 7)) {
	sign = 1;
	temp = (((temp & 0x7FFF) * 3125) / 10 + charger_tcc270->curr_offset) / 1000;
  }else
		temp = ((temp * 3125) / 10 - charger_tcc270->curr_offset) / 1000;

  if (sign)
	temp *= -1;

	charger_tcc270->curr = temp;
#ifdef CONFIG_TCC270_DBG
  printk(KERN_INFO "[TCC270] current: %d\n", charger_tcc270->curr);
#endif
}

static void tcc270_get_external_temp(struct i2c_client *client)
{
  u8 data[2];
  long int temp;

  if (tcc270_read_reg(TCC270_REG_EXT_TEMPERATUE_MSB, data, 2) < 0) {
	printk(KERN_ERR "%s: Failed to read TEMPERATURE\n", __func__);
  }
  charger_tcc270->ain_volt = (data[0] * 256 + data[1]) * 61 / 100;
  temp =  (charger_tcc270->ain_volt * (-91738) + 81521000) / 100000;
  charger_tcc270->ext_temp = (int)temp;
	
	if (charger_tcc270->ext_temp >= HIGH_TEMP_THRES) {
		if (charger_tcc270->health != POWER_SUPPLY_HEALTH_OVERHEAT)
			charger_tcc270->temp_high_cnt++;
	} else if (charger_tcc270->ext_temp <= HIGH_TEMP_RECOVER && charger_tcc270->ext_temp >= LOW_TEMP_RECOVER) {
		if (charger_tcc270->health == POWER_SUPPLY_HEALTH_OVERHEAT ||
			charger_tcc270->health == POWER_SUPPLY_HEALTH_COLD)
			charger_tcc270->temp_recover_cnt++;
	} else if (charger_tcc270->ext_temp <= LOW_TEMP_THRES) {
		if (charger_tcc270->health != POWER_SUPPLY_HEALTH_COLD)
			charger_tcc270->temp_low_cnt++;
	} else {
		charger_tcc270->temp_high_cnt = 0;
		charger_tcc270->temp_low_cnt = 0;
		charger_tcc270->temp_recover_cnt = 0;
	}
	
	if (charger_tcc270->temp_high_cnt >= TEMP_ABNORMAL_COUNT) {
	 charger_tcc270->health = POWER_SUPPLY_HEALTH_OVERHEAT;
	 charger_tcc270->temp_high_cnt = 0;
	} else if (charger_tcc270->temp_low_cnt >= TEMP_ABNORMAL_COUNT) {
	 charger_tcc270->health = POWER_SUPPLY_HEALTH_COLD;
	 charger_tcc270->temp_low_cnt = 0;
	} else if (charger_tcc270->temp_recover_cnt >= TEMP_ABNORMAL_COUNT) {
	 charger_tcc270->health = POWER_SUPPLY_HEALTH_GOOD;
	 charger_tcc270->temp_recover_cnt = 0;
	}
#ifdef CONFIG_TCC270_DBG
  printk(KERN_INFO "[TCC270] external temperature: %d\n", charger_tcc270->ext_temp);
#endif
}

static void tcc270_clear_cc(operation_mode mode)
{  
  u8 data[2];
	
  if (tcc270_read_reg(TCC270_REG_CHANNEL_MSB, data, 2) < 0){
	printk(KERN_ERR "%s: failed to read channel\n", __func__);
  }

  if (mode == CHG)
		data[0] = data[0] | CHANNEL_H_BIT_CLRQCHG;
	else
		data[0] = data[0] | CHANNEL_H_BIT_CLRQDCHG;
		
  if (tcc270_write_reg(TCC270_REG_CHANNEL_MSB, data, 2) < 0){
	printk(KERN_ERR "%s: failed to write channel\n", __func__);
  }
}

static void tcc270_get_chg_cc(struct i2c_client *client)
{
  u8 data[4];
  u32 qh_old,ql_old,qh_new,ql_new;
  u32 cc_masec,offset;
  
  if (tcc270_read_reg(TCC270_REG_QCHGH_MSB, data, 4) < 0){
	printk(KERN_ERR "%s: Failed to read QCHG\n", __func__);
  }
  qh_old = (data[0]<<8) + data[1];
  ql_old = (data[2]<<8) + data[3];
  
  if (tcc270_read_reg(TCC270_REG_QCHGH_MSB, data, 4) < 0){
	printk(KERN_ERR "%s: Failed to read QCHG\n", __func__);
  }
  qh_new = (data[0]<<8) + data[1];
  ql_new = (data[2]<<8) + data[3];
	
  if (qh_new > qh_old){
	 cc_masec = (((qh_new<<16) + ql_new) * 50134) / 10;
  }else if (qh_new == qh_old){
	if (ql_new >= ql_old){
	  cc_masec = (((qh_new<<16) + ql_new) * 50134) / 10;
	}else {  
	  cc_masec = (((qh_old<<16) + ql_old) * 50134) / 10;
		}
  } 
  
	offset = charger_tcc270->curr_offset * charger_tcc270->time_interval;
			  
  if (cc_masec != 0){
		cc_masec = (cc_masec - offset) / 1000;
	}

#ifdef CONFIG_RT5018_DBG
  printk(KERN_INFO "[TCC270] chg_cc_mAsec: %d\n", cc_masec);
#endif

	charger_tcc270->chg_cc = (cc_masec + charger_tcc270->chg_cc_unuse) / BATTERY_CAP_CUR;
  charger_tcc270->chg_cc_unuse = (cc_masec + charger_tcc270->chg_cc_unuse) % BATTERY_CAP_CUR;
#ifdef CONFIG_RT5018_DBG
  printk(KERN_INFO "[TCC270] chg_cc_mAH: %d\n", charger_tcc270->chg_cc);
#endif
  tcc270_clear_cc(CHG);
}

static void tcc270_get_dchg_cc(struct i2c_client *client)
{
  u8 data[4];
  u32 qh_old,ql_old,qh_new,ql_new;
  u32 cc_masec,offset;
  
  if (tcc270_read_reg(TCC270_REG_QDCHGH_MSB, data, 4) < 0){
	printk(KERN_ERR "%s: Failed to read QDCHG\n", __func__);
  }
  qh_old = (data[0]<<8) + data[1];
  ql_old = (data[2]<<8) + data[3];
  
  if (tcc270_read_reg(TCC270_REG_QDCHGH_MSB, data, 4) < 0){
	printk(KERN_ERR "%s: Failed to read QDCHG\n", __func__);
  }
  qh_new = (data[0]<<8) + data[1];
  ql_new = (data[2]<<8) + data[3];
  
  if (qh_new > qh_old){
	 cc_masec = (((qh_new<<16) + ql_new) * 50134) / 10;
  }else if (qh_new == qh_old){
	if (ql_new >= ql_old){
	  cc_masec = (((qh_new<<16) + ql_new) * 50134) / 10;
	}else {  
	  cc_masec = (((qh_old<<16) + ql_old) * 50134) / 10;
		}
  } 
  
	offset = charger_tcc270->curr_offset * charger_tcc270->time_interval;
			  
  if (cc_masec != 0){
		cc_masec = (cc_masec - offset) / 1000;
	}

#ifdef CONFIG_TCC270_DBG
  printk(KERN_INFO "[TCC270] dchg_cc_mAsec: %d\n", cc_masec);
#endif

	charger_tcc270->dchg_cc = (cc_masec + charger_tcc270->dchg_cc_unuse) / BATTERY_CAP_CUR;
  charger_tcc270->dchg_cc_unuse = (cc_masec + charger_tcc270->dchg_cc_unuse) % BATTERY_CAP_CUR;
#ifdef CONFIG_TCC270_DBG
  printk(KERN_INFO "[TCC270] dchg_cc_mAH: %d\n", charger_tcc270->dchg_cc);
#endif
	tcc270_clear_cc(DCHG);
}

static void tcc270_get_irq_flag(struct i2c_client *client)
{
  u8 data[1];

  if (tcc270_read_reg(TCC270_REG_IRQ_FLAG, data, 1) < 0){
	printk(KERN_ERR "%s: Failed to read irq_flag\n", __func__);
  }
		
  charger_tcc270->irq_flag = data[0];
#ifdef CONFIG_TCC270_DBG
  printk(KERN_INFO "[TCC270] IRQ_FLG 0x%x\n", charger_tcc270->irq_flag);
#endif
}

static void tcc270_get_timer(struct i2c_client *client)
{
  u8 data[2];
	u16 gauge_timer;
	
  if (tcc270_read_reg(TCC270_REG_TIMER, data, 2) < 0){
	printk(KERN_ERR "%s: Failed to read Timer\n", __func__);
  }
		
  gauge_timer = (data[0] << 8) + data[1];
  if (gauge_timer > charger_tcc270->pre_gauge_timer)
		charger_tcc270->time_interval = gauge_timer - charger_tcc270->pre_gauge_timer;
	else	
		charger_tcc270->time_interval = 65536 - charger_tcc270->pre_gauge_timer + gauge_timer;
		
  charger_tcc270->pre_gauge_timer = gauge_timer;
#ifdef CONFIG_TCC270_DBG
  printk(KERN_INFO "[TCC270] timer %d , interval %d\n", gauge_timer,charger_tcc270->time_interval);
#endif
}

static void tcc270_get_soc(struct i2c_client *client)
{
  charger_tcc270->soc = charger_tcc270->rest_cap;
}

static void tcc270_channel_cc(bool enable)
{
  u8 data[1];
	
  if (tcc270_read_reg(TCC270_REG_CHANNEL_LSB, data, 1) < 0){
	printk(KERN_ERR "%s: failed to read channel\n", __func__);
  }

  if (enable){
	data[0] = data[0] | 0x80;
  }else { 
	data[0] = data[0] & 0x7F;
  }
	
  if (tcc270_write_reg(TCC270_REG_CHANNEL_LSB, data, 1) < 0){
	printk(KERN_ERR "%s: failed to write channel\n", __func__);
  }
}

static void tcc270_register_init(struct i2c_client *client)
{  
  u8 data[1];
	
  /* enable the channel of current,qc,ain,vbat and vadc */
  if (tcc270_read_reg(TCC270_REG_CHANNEL_LSB, data, 1) < 0){
	printk("%s: failed to read channel\n", __func__);
  }
  data[0] = data[0] |
						CHANNEL_L_BIT_CADC_EN |
						CHANNEL_L_BIT_AINCH |
						CHANNEL_L_BIT_VBATSCH |
						CHANNEL_L_BIT_VADC_EN;
  if (tcc270_write_reg(TCC270_REG_CHANNEL_LSB, data, 1) < 0){
	printk("%s: failed to write channel\n", __func__);
  }
	/* set the alert threshold value */
	irq_thres[MAXTEMP]	= TALRTMAX_VALUE;
	irq_thres[MINTEMP]	= TALRTMIN_VALUE;
	irq_thres[MAXVOLT]	= VALRTMAX_VALUE;
	irq_thres[MINVOLT1] = VALRTMIN1_VALUE;
	irq_thres[MINVOLT2] = VALRTMIN2_VALUE;
	irq_thres[TEMP_RLS] = TRLS_VALUE;
	irq_thres[VOLT_RLS] = VRLS_VALUE;

	charger_tcc270->chg_cc_unuse = 0;
	charger_tcc270->dchg_cc_unuse = 0;
	charger_tcc270->pre_gauge_timer = 0;
	charger_tcc270->online = 1;
	charger_tcc270->status = POWER_SUPPLY_STATUS_DISCHARGING;
	charger_tcc270->health = POWER_SUPPLY_HEALTH_GOOD;
#ifdef CONFIG_TCC270_DBG
  printk(KERN_INFO "[TCC270] register initialized\n");
#endif
}

static void tcc270_alert_setting(alert_type type, bool enable)
{
	u8 data[1];
	
  if (tcc270_read_reg(TCC270_REG_IRQ_CTL, data, 1) < 0){
	printk(KERN_ERR "%s: Failed to read CONFIG\n", __func__);
  }

  if(enable){
		switch(type){
			case MAXTEMP:
				data[0] |= IRQ_CTL_BIT_TMX; //Enable max temperature alert
				charger_tcc270->max_temp_irq = true;
				#ifdef CONFIG_TCC270_DBG
				printk(KERN_INFO "Enable min temperature alert");
				#endif
				break;
			case MINTEMP:
				data[0] |= IRQ_CTL_BIT_TMN; //Enable min temperature alert
				charger_tcc270->min_temp_irq = true;	
				#ifdef CONFIG_TCC270_DBG
				printk(KERN_INFO "Enable max temperature alert");
				#endif
				break;
			case MAXVOLT:
				data[0] |= IRQ_CTL_BIT_VMX; //Enable max voltage alert
				charger_tcc270->max_volt_irq = true;
				#ifdef CONFIG_TCC270_DBG
				printk(KERN_INFO "Enable max voltage alert");
				#endif
				break;
			case MINVOLT1:
				data[0] |= IRQ_CTL_BIT_VMN1; //Enable min1 voltage alert	
				charger_tcc270->min_volt1_irq = true;
				#ifdef CONFIG_TCC270_DBG
				printk(KERN_INFO "Enable min1 voltage alert");
				#endif
				break;
			case MINVOLT2:
				data[0] |= IRQ_CTL_BIT_VMN2; //Enable min2 voltage alert
				charger_tcc270->min_volt2_irq = true;
				#ifdef CONFIG_TCC270_DBG
				printk(KERN_INFO "Enable min2 voltage alert");
				#endif
				break;
			default:
				break;
		}
	}else{
		switch(type){
			case MAXTEMP:
				data[0] = data[0] &~ IRQ_CTL_BIT_TMX; //Disable max temperature alert
				charger_tcc270->max_temp_irq = false;
				#ifdef CONFIG_TCC270_DBG
				printk(KERN_INFO "Disable min temperature alert");
				#endif
				break;
			case MINTEMP:
				data[0] = data[0] &~ IRQ_CTL_BIT_TMN; //Disable min temperature alert
				charger_tcc270->min_temp_irq = false;
				#ifdef CONFIG_TCC270_DBG
				printk(KERN_INFO "Disable max temperature alert");
				#endif
				break;
			case MAXVOLT:
				data[0] = data[0] &~ IRQ_CTL_BIT_VMX; //Disable max voltage alert
				charger_tcc270->max_volt_irq = false;
				#ifdef CONFIG_TCC270_DBG
				printk(KERN_INFO "Disable max voltage alert");
				#endif
				break;
			case MINVOLT1:
				data[0] = data[0] &~ IRQ_CTL_BIT_VMN1; //Disable min1 voltage alert 
				charger_tcc270->min_volt1_irq = false;
				#ifdef CONFIG_TCC270_DBG
				printk(KERN_INFO "Disable min1 voltage alert");
				#endif
				break;
			case MINVOLT2:
				data[0] = data[0] &~ IRQ_CTL_BIT_VMN2; //Disable min2 voltage alert
				charger_tcc270->min_volt2_irq = false;
				#ifdef CONFIG_TCC270_DBG
				printk(KERN_INFO "Disable min2 voltage alert");
				#endif
				break;
			default:
				break;
		}
	}
  if (tcc270_write_reg(TCC270_REG_IRQ_CTL, data, 1) < 0)
		printk(KERN_ERR "%s: failed to write IRQ control\n", __func__);
}	
static void tcc270_alert_threshold_init(struct i2c_client *client)
{
  u8 data[1];

  /* TALRT MAX threshold setting */
  data[0] = irq_thres[MAXTEMP];
  if (tcc270_write_reg(TCC270_REG_TALRT_MAXTH, data, 1) < 0)
		printk(KERN_ERR "%s: failed to write TALRT MAX threshold\n", __func__); 
  /* TALRT MIN threshold setting */
  data[0] = irq_thres[MINTEMP];
  if (tcc270_write_reg(TCC270_REG_TALRT_MINTH, data, 1) < 0)
		printk(KERN_ERR "%s: failed to write TALRT MIN threshold\n", __func__); 
  /* VALRT MAX threshold setting */
  data[0] = irq_thres[MAXVOLT];
  if (tcc270_write_reg(TCC270_REG_VALRT_MAXTH, data, 1) < 0)
		printk(KERN_ERR "%s: failed to write VALRT MAX threshold\n", __func__); 
  /* VALRT MIN1 threshold setting */
  data[0] = irq_thres[MINVOLT1];
  if (tcc270_write_reg(TCC270_REG_VALRT_MIN1TH, data, 1) < 0)
		printk(KERN_ERR "%s: failed to write VALRT MIN1 threshold\n", __func__);	
  /* VALRT MIN2 threshold setting */
  data[0] = irq_thres[MINVOLT2];
  if (tcc270_write_reg(TCC270_REG_VALRT_MIN2TH, data, 1) < 0)
		printk(KERN_ERR "%s: failed to write VALRT MIN2 threshold\n", __func__);	
}

static void tcc270_alert_init(struct i2c_client *client)
{
  /* Set RT5018 gauge alert configuration */
  tcc270_alert_threshold_init(client);
	/* Enable gauge alert function */
	tcc270_alert_setting(MAXTEMP,true);
	tcc270_alert_setting(MINTEMP,true);
	tcc270_alert_setting(MAXVOLT,true);
	tcc270_alert_setting(MINVOLT1,true);
	tcc270_alert_setting(MINVOLT2,true);	
}

void tcc270_irq_handler(void)
{
  tcc270_get_irq_flag(charger_tcc270->master);

  if ((charger_tcc270->irq_flag) & IRQ_FLG_BIT_TMX){	
		printk(KERN_INFO "[TCC270]: Min temperature IRQ received\n");
		tcc270_alert_setting(MAXTEMP,false);
		charger_tcc270->max_temp_irq = false;
	}
  if ((charger_tcc270->irq_flag) & IRQ_FLG_BIT_TMN){
		printk(KERN_INFO "[TCC270]: Max temperature IRQ received\n");
		tcc270_alert_setting(MINTEMP,false);
		charger_tcc270->min_temp_irq = false; 
	}
  if ((charger_tcc270->irq_flag) & IRQ_FLG_BIT_VMX){
		printk(KERN_INFO "[TCC270]: Max voltage IRQ received\n");
		tcc270_alert_setting(MAXVOLT,false);
		charger_tcc270->max_volt_irq = false;
	}
  if ((charger_tcc270->irq_flag) & IRQ_FLG_BIT_VMN1){
		printk(KERN_INFO "[TCC270]: Min voltage1 IRQ received\n");
		tcc270_alert_setting(MINVOLT1,false);
		charger_tcc270->min_volt1_irq = false;
	}
  if ((charger_tcc270->irq_flag) & IRQ_FLG_BIT_VMN2){
		printk(KERN_INFO "[TCC270]: Min voltage2 IRQ received\n");
		tcc270_alert_setting(MINVOLT2,false);
		charger_tcc270->min_volt2_irq = false;
	}
	
	//wake_lock(&chip->monitor_wake_lock);
	//schedule_delayed_work(&chip->monitor_work, 0);
}

static void tcc270_update_state(struct i2c_client *client)
{
  /* Update voltage */
  tcc270_get_vcell(client);
  /* Update current */
  tcc270_get_current(client);
  /* Update external temperature */
  tcc270_get_external_temp(client);
  /* Read timer */
  tcc270_get_timer(client);
  /* Update chg cc */
  tcc270_get_chg_cc(client);
  /* Update dchg cc */
  tcc270_get_dchg_cc(client);
  /* Update SOC */
  tcc270_get_soc(client);

  if ((charger_tcc270->max_temp_irq == false) &&
		 (((irq_thres[MAXTEMP] * IRQ_THRES_UNIT) / 100 - charger_tcc270->ain_volt) > irq_thres[TEMP_RLS])){
		tcc270_alert_setting(MAXTEMP,true);
	}else if ((charger_tcc270->min_temp_irq == false) &&
					  ((charger_tcc270->ain_volt - (irq_thres[MINTEMP] * IRQ_THRES_UNIT) / 100) > irq_thres[TEMP_RLS])){
		tcc270_alert_setting(MINTEMP,true);
	}else if ((charger_tcc270->max_volt_irq == false) &&
					((((irq_thres[MAXVOLT] * IRQ_THRES_UNIT) / 100) - charger_tcc270->vcell) > irq_thres[VOLT_RLS])){
		tcc270_alert_setting(MAXVOLT,true);
	}else if ((charger_tcc270->min_volt1_irq == false) &&
					((charger_tcc270->vcell - ((irq_thres[MINVOLT1] * IRQ_THRES_UNIT) / 100)) > irq_thres[VOLT_RLS])){
		tcc270_alert_setting(MINVOLT1,true);				
	}else if ((charger_tcc270->min_volt2_irq == false) &&
					((charger_tcc270->vcell - ((irq_thres[MINVOLT2] * IRQ_THRES_UNIT) / 100)) > irq_thres[VOLT_RLS])){
		tcc270_alert_setting(MINVOLT2,true);						
	}
}

int tcc_set_chargecurrent(int cur)
{
	int ret = 0;
	unsigned long old_icc;

#if 0
	if(cur) {
	
	old_icc = i2c_smbus_read_byte_data(charger_tcc270->master, TCC270_CHGCONTROL4_REG);
	old_icc = old_icc | (3<<3);
	ret = i2c_smbus_write_byte_data(charger_tcc270->master, TCC270_CHGCONTROL4_REG, old_icc);
	
	old_icc = i2c_smbus_read_byte_data(charger_tcc270->master, TCC270_CHGCONTROL2_REG);
	old_icc = old_icc | (1<<1);
	ret = i2c_smbus_write_byte_data(charger_tcc270->master, TCC270_CHGCONTROL2_REG, old_icc);
	tcc270_chg_set_current(cur);
	}
	else
	{
		old_icc = i2c_smbus_read_byte_data(charger_tcc270->master, TCC270_CHGCONTROL2_REG);
		old_icc = old_icc &~ (1<<1);
		ret = i2c_smbus_write_byte_data(charger_tcc270->master, TCC270_CHGCONTROL2_REG, old_icc);
	}
#endif

	return ret;
}
EXPORT_SYMBOL(tcc_set_chargecurrent);

int tcc_read_adc(struct tcc_charger *charger)
{
	unsigned long data[2];
	unsigned long adcValue = 0;
	int i;
	int adc_samples[BATTSAMPLE];
	int ret =0;

	//mutex_lock(&charger->mutex);
	for(i=0;i<BATTSAMPLE;i++)
	{
		do{
			data[0] = i2c_smbus_read_byte_data(charger->master, TCC270_VBATS_HIGH_REG);
		} while( data[0] < 0);
		do{
			data[1] = i2c_smbus_read_byte_data(charger->master, TCC270_VBATS_LOW_REG);
		} while( data[1] < 0);
		ret = (data[0] << 8) | data[1];
		adc_samples[i] = ret;
	}
	bubblesort(adc_samples, BATTSAMPLE);
	
	for(i=(BATTSAMPLE/2 - 4);i< (BATTSAMPLE/2 + 4) ; i++)
		adcValue += adc_samples[i];
	adcValue /= 8;
	//mutex_unlock(&charger->mutex);
	return adcValue;
}

void tcc_charger_update_state(struct tcc_charger *charger)
{	
	dbg("%s\n", __func__);
	charger->ac_det  = tcc270_acin_detect();
}
EXPORT_SYMBOL(tcc_charger_update_state);

void tcc270_calibration(struct tcc_charger *charger)
{
	int pre_rest_cap,rt_rest_cap,rest_cap;
	uint8_t val;
	
	pre_rest_cap = charger->rest_cap;
	
	rt_rest_cap = charger->ocv_rest_cap;
	rest_cap = charger->rest_cap;
	charger->rest_cap = 100 * (base_vol / BATTERY_CAP_CUR) + base_cap;

	if((rt_rest_cap < (BATCAPCORRATE+1))){			
		startcorcap = rt_rest_cap;
		DBG_PSY_MSG("\n ============Capacity Calibration Start============ \n");
		DBG_PSY_MSG("\n ============   Rest  Capacity  =  %d  ============ \n",rt_rest_cap);
	}
		
	if(charger->rest_cap > 100){
		DBG_PSY_MSG("%s->%d: charger->rest_vol > 100\n",__FUNCTION__,__LINE__);
		charger->rest_cap = 100;
	}
	else if (charger->rest_cap < 0){
		DBG_PSY_MSG("%s->%d: charger->rest_vol < 0\n",__FUNCTION__,__LINE__);
		charger->rest_cap = 0;
	}
		
	if((rt_rest_cap > 98) && (charger->rest_cap < 99)){
		DBG_PSY_MSG("%s->%d:(rt_rest_cap > 98) && (charger->rest_cap < 99)\n",__FUNCTION__,__LINE__);
		if(cap_count1 >= TIMER5) {
			charger->rest_cap ++;
			base_cap ++;
			cap_count1 = 0;
		} else {
			cap_count1++;
		}
	} else {
		cap_count1 = 0;
	}
		
	if((charger->vcell >= 4100) && (charger->rest_cap < 100)){
		DBG_PSY_MSG("%s->%d:(charger->vcell >= 4100) && (charger->rest_vol < 100) && (charger->bat_current_direction == 0) && (charger->ext_valid)\n",__FUNCTION__,__LINE__);
		if(cap_count2 >= TIMER5) {
			charger->rest_cap++;
			base_cap++;
			cap_count2 = 0;
		} else {
			cap_count2++;
		}
	} else {
		cap_count2 = 0;
	}

		
	if((rt_rest_cap > (BATCAPCORRATE-1)) && (charger->rest_cap < BATCAPCORRATE)){
		DBG_PSY_MSG("%s->%d(rt_rest_cap > %d) && (charger->rest_cap < %d) && charger->bat_current_direction == 0\n",__FUNCTION__,__LINE__,BATCAPCORRATE-1,BATCAPCORRATE);
		if(pre_rest_cap > charger->rest_cap){
			base_cap = pre_rest_cap - 100 * (base_vol / BATTERY_CAP_CUR);
		}
		charger->rest_cap =  pre_rest_cap;
	}
			
		
	if((rt_rest_cap < BATCAPCORRATE) && (charger->rest_cap > (BATCAPCORRATE-1))){
		DBG_PSY_MSG("%s->%d(rt_rest_cap < %d) && (charger->rest_cap > %d) && (charger->bat_current_direction == 0)\n",__FUNCTION__,__LINE__,BATCAPCORRATE,BATCAPCORRATE-1);
		if(cap_count3 >= TIMER5){
			charger->rest_cap --;
			base_cap --;
			cap_count3 = 0;
		} else {
			cap_count3++;
		}
	} else {
		cap_count3 = 0;
	}
		
		
	if((rt_rest_cap > (SHUTDOWNRATE+1)) && (charger->rest_cap < (SHUTDOWNRATE+2))){
		DBG_PSY_MSG("%s->%d(rt_rest_cap > %d) && (charger->rest_cap < %d) && charger->bat_current_direction == 0\n",__FUNCTION__,__LINE__,(SHUTDOWNRATE+1),(SHUTDOWNRATE+2));
		if(pre_rest_cap > charger->rest_cap){
			base_cap = pre_rest_cap - 100 * (base_vol / BATTERY_CAP_CUR);
		}
		charger->rest_cap =  pre_rest_cap;
	}
			
		
	if((rt_rest_cap < (SHUTDOWNRATE+2)) && (charger->rest_cap > (SHUTDOWNRATE+1))){
		DBG_PSY_MSG("%s->%d(rt_rest_cap < %d) && (charger->rest_cap > %d) && (charger->bat_current_direction == 0)\n",__FUNCTION__,__LINE__,(SHUTDOWNRATE+2),(SHUTDOWNRATE+1));
		if(cap_count4 >= TIMER5){
			charger->rest_cap --;
			base_cap --;
			cap_count4 = 0;
		} else {
			cap_count4++;
		}
	} else {
		cap_count4 = 0;
	}

		
	if((charger->rest_cap >= 100)){
		DBG_PSY_MSG("%s->%d:[Before fix] charger->rest_vol = %d\n",__FUNCTION__,__LINE__, charger->rest_cap);
		charger->rest_cap = 99;
		base_cap = 99 - 100 * (base_vol / BATTERY_CAP_CUR);
	}

	if((charger->rest_cap == 100)){
		bat_cap = ABS(charger_tcc270->dchg_cc - charger_tcc270->chg_cc) / ( 100 - startcorcap) * 100;  
		DBG_PSY_MSG("\n ===========Capacity calibration finished============ \n");
		DBG_PSY_MSG("\n ============   Battery capacity =  %d   ============ \n",bat_cap);
		base_cap = 100 - 100 * (base_vol / BATTERY_CAP_CUR);
		
	}
	else {
		charger->rest_cap = rt_rest_cap;
		DBG_PSY_MSG(" first init charger->rest_cap = %d\n",charger->rest_cap);   
		/* full */
		if(charger->vcell >= 4100 && !charger->ac_det)
			charger->rest_cap = 100;
		/* charging */
		if(charger->ac_det && charger->rest_cap >= 100)
			charger->rest_cap = 99;
	}
	
	if(charger->rest_cap - pre_rest_cap){
		DBG_PSY_MSG("battery vol change: %d->%d \n", pre_rest_cap, charger->rest_cap);
		pre_rest_cap = charger->rest_cap;
	}
}

void tcc270_ocv_calculation(struct tcc_charger *charger)
{
	uint8_t val;
	uint8_t v[5];
	int pre_rest_cap;
	int k;
	int rt_rest_cap;
	int rest_cap;
	int cap_index_p;

	pre_rest_cap = charger->rest_cap;
	val = tcc270_vol2cap(charger_tcc270->vcell);
	
	if(state_count){
		rt_rest_cap = (int) (val & 0x7F);
		rest_cap = charger->rest_cap;
		state_count ++;
		if(state_count >= TIMER2){
			state_count = 0;
		}
	} else {
		rt_rest_cap = (int) (val & 0x7F);
		if((charger->bat_det == 0) || (rt_rest_cap == 127) ){
			rt_rest_cap = 100;
		}

		Total_Cap -= Bat_Cap_Buffer[Cap_Index];
		if(Cap_Index == 0){
			cap_index_p = TCC270_VOL_MAX - 1;
		} else {
			cap_index_p = Cap_Index - 1;
		}

		Bat_Cap_Buffer[Cap_Index] = rt_rest_cap;
		Total_Cap += Bat_Cap_Buffer[Cap_Index];
		Cap_Index++;
		if(Cap_Index == TCC270_VOL_MAX){
			Cap_Index = 0;
		}

		rest_cap = (Total_Cap + TCC270_VOL_MAX / 2 ) / TCC270_VOL_MAX;
		
		DBG_PSY_MSG("Before Modify:Cap_Index = %d,val = 0x%x,pre_rest_cap = %d,rest_cap = %d\n",Cap_Index,val,pre_rest_cap,rest_cap);

		if(charger->ac_det && (rest_cap < pre_rest_cap)){
			rest_cap = pre_rest_cap;
		} 
		DBG_PSY_MSG("After Modify:val = 0x%x,pre_rest_cap = %d,rest_cap = %d\n",val,pre_rest_cap,rest_cap);

		/* full */
		if(charger->vcell >= 4100 && !charger->ac_det){
			rest_cap = 100;
			for(k = 0;k < TCC270_VOL_MAX; k++){
				Bat_Cap_Buffer[k] = rest_cap;
			}
			Total_Cap = rest_cap * TCC270_VOL_MAX;
			charger->bat_current_direction = 1;
		}

		/* charging*/
		if(charger->ac_det && rest_cap == 100){
			rest_cap = 99;
		}

		if(rest_cap > pre_rest_cap){
			if(cap_count >= TIMER5){
				charger->rest_cap++;
				cap_count = 0;
			} else {
				cap_count++;
			}
		} else if(rest_cap < pre_rest_cap){
			if(cap_count >= TIMER5){
				charger->rest_cap --;
				cap_count = 0;
			} else {
				cap_count++;
			}
		} else {
			cap_count = 0;
		}
	}

	/* if battery volume changed, inform uevent */
	if(charger->rest_cap - pre_rest_cap){
		printk("battery vol change: %d->%d \n", pre_rest_cap, charger->rest_cap);
		pre_rest_cap = charger->rest_cap;
	}
}

void tcc270_cc_calculation( struct tcc_charger *charger)
{
	base_vol += charger_tcc270->chg_cc;
	base_vol -= charger_tcc270->dchg_cc;

	charger->rest_cap = 100 * base_vol / BATTERY_CAP_CUR;
}

void tcc_charging_monitor(struct tcc_charger *charger)
{
	int temp;
	int temp_acpin;
	int info_temp;
	int ret = 0;
	int pre_rest_cap;
	
	dbg("%s\n", __func__);
	
	tcc_charger_update_state(charger);
	tcc270_update_state(charger_tcc270->master);
	
	temp = charger_tcc270->vcell;
		
	base_vol += charger_tcc270->chg_cc;
	base_vol -= charger_tcc270->dchg_cc;

	//tcc270_ocv_calculation(charger);
	tcc270_cc_calculation(charger);
	//tcc270_calibration(charger);
	
	//tcc270_vol2cap(charger_tcc270->vcell);

}
EXPORT_SYMBOL(tcc_charging_monitor);

void tcc_charger_update(struct tcc_charger *charger)
{
	uint8_t val[2];
	const int *pbattery_vols;
	int temp;
	int info_temp;
	
	dbg("%s\n", __func__);
	
	//tcc270_update_state(charger_tcc270->master);
	//tcc270_irq_handler();
	//tcc270_update_state(charger_tcc270->master);

}
EXPORT_SYMBOL(tcc_charger_update);

void tcc_battery_infoinit(struct tcc_charger *charger)
{
	uint8_t val;
	int temp;
	int i,k;
	
	dbg("%s\n", __func__);

	//tcc270_chg_set_config();
	//tcc270_alert_init(charger_tcc270->master);
	tcc270_register_init(charger_tcc270->master);
	tcc_charger_update_state(charger);
	tcc270_update_state(charger_tcc270->master);
	if((int)(charger_tcc270->curr) > 0)
		charger->ac_det = 1;
	else
		charger->ac_det = 0;

	val = tcc270_vol2cap(charger_tcc270->vcell);

	charger->ocv_rest_cap = val;
	charger->rest_cap = (int) (val & 0x7F);
	
	if(/*(charger->bat_det == 0) ||*/ (charger->rest_cap == 127)){
		charger->rest_cap = 100;
	}
	
	base_cap = charger->rest_cap;
	base_vol = charger->rest_cap*BATTERY_CAP_CUR / 100;
	bat_cap = BATTERY_CAP_CUR;

	memset(Bat_Cap_Buffer, 0, sizeof(Bat_Cap_Buffer));
	for(k = 0;k < TCC270_VOL_MAX; k++){
		Bat_Cap_Buffer[k] = charger->rest_cap;
	}
	
	Total_Cap = charger->rest_cap * TCC270_VOL_MAX;
}
EXPORT_SYMBOL(tcc_battery_infoinit);
int tcc_suspend(struct tcc_charger *charger)
{
	
	//tcc270_channel_cc(false);
	tcc_battery_infoinit(charger);
	tcc_charging_monitor(charger);
	return 0;
}
EXPORT_SYMBOL(tcc_suspend);

int tcc_resume(struct tcc_charger *charger)
{
	//tcc270_channel_cc(true);
	tcc_battery_infoinit(charger);
	tcc_charging_monitor(charger);

	return 0;
}
EXPORT_SYMBOL(tcc_resume);

int tcc_charger_init(struct tcc_charger *charger)
{
	int ret = -1;
	int i;
	charger_tcc270 = charger;
	mutex_init(&charger->mutex);

	tcc270_get_current(charger_tcc270->master);
	if((int)(charger_tcc270->curr) > 0)
		charger->ac_det = 1;
	else
		charger->ac_det = 0;
	
	//if(charger->bat_det == 0)
	//for(i=0;i<3;i++)
		tcc_battery_infoinit(charger);
	tcc_charger_update_state(charger);
	tcc_charger_update(charger);
		
	return 0;
}
EXPORT_SYMBOL(tcc_charger_init);

