/*
 * Battery charger driver for X-Powers AXP
 *
 * Copyright (C) 2011 X-Powers, Ltd.
 *  Zhang Donglu <zhangdonglu@x-powers.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/slab.h>

#include <linux/seq_file.h>
#include <linux/input.h>
#include <linux/power/axp-sply.h>
#include <asm/div64.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#define DBG_AXP_PSY 1
#if  DBG_AXP_PSY
#define DBG_PSY_MSG(format,args...)   printk("[AXP]"format,##args)
#else
#define DBG_PSY_MSG(format,args...)   do {} while (0)
#endif

//#include "axp-sply.h"

void axp_charger_update_state(struct axp_charger *charger);
void axp_charger_update(struct axp_charger *charger);
static int axp_ibat_to_mA(uint16_t reg);

#include "axp-init.h"
#include "axp-debug.h"
#include "axp-noirq.h"
#include "axp-rw.h"

#include "axp-calocv.h"
#include "axp-calcou.h"

static struct wake_lock vbus_wake_lock;

#ifdef CONFIG_HAS_EARLYSUSPEND
static struct early_suspend axp_early_suspend;
#endif

static inline int axp_vbat_to_mV(uint16_t reg)
{
	return (reg) * 1100 / 1000;
}

static inline int axp_vdc_to_mV(uint16_t reg)
{
	return (reg) * 1700 / 1000;
}

static inline int axp_ibat_to_mA(uint16_t reg)
{
	return (reg) * 500 / 1000;
}

static inline int axp_iac_to_mA(uint16_t reg)
{
	return (reg) * 625 / 1000;
}

static inline int axp_iusb_to_mA(uint16_t reg)
{
	return (reg) * 375 / 1000;
}

static inline void axp_read_adc(struct axp_charger *charger,
  struct axp_adc_res *adc)
{
	uint8_t tmp[8];
	__axp_reads(charger->master,AXP_VACH_RES,8,tmp);
	adc->vac_res = ((uint16_t) tmp[0] << 4 )| (tmp[1] & 0x0f);
	adc->iac_res = ((uint16_t) tmp[2] << 4 )| (tmp[3] & 0x0f);
	adc->vusb_res = ((uint16_t) tmp[4] << 4 )| (tmp[5] & 0x0f);
	adc->iusb_res = ((uint16_t) tmp[6] << 4 )| (tmp[7] & 0x0f);
	__axp_reads(charger->master,AXP_VBATH_RES,6,tmp);
	adc->vbat_res = ((uint16_t) tmp[0] << 4 )| (tmp[1] & 0x0f);
#if defined (CONFIG_REGULATOR_AXP202)
	adc->ichar_res = ((uint16_t) tmp[2] << 4 )| (tmp[3] & 0x0f);
#endif
#if defined (CONFIG_REGULATOR_AXP192)
	adc->ichar_res = ((uint16_t) tmp[2] << 5 )| (tmp[3] & 0x1f);
#endif
	adc->idischar_res = ((uint16_t) tmp[4] << 5 )| (tmp[5] & 0x1f);
}


void axp_charger_update_state(struct axp_charger *charger)
{
	uint8_t val[2];
	uint16_t tmp;

	__axp_reads(charger->master,AXP_CHARGE_STATUS,2,val);
	tmp = (val[1] << 8 )+ val[0];

	charger->bat_det 				= (tmp & AXP_STATUS_BATEN)?1:0;
	charger->ac_det 				= (tmp & AXP_STATUS_ACEN)?1:0;
	charger->ac_valid 				= (tmp & AXP_STATUS_ACVA)?1:0;	
#if defined(CONFIG_USB_CHARGING)
	charger->usb_det 				= (tmp & AXP_STATUS_USBEN)?1:0;	
	charger->usb_valid 				= (tmp & AXP_STATUS_USBVA)?1:0;
	charger->is_on 					= (tmp & AXP_STATUS_INCHAR)?1:0;	
#else
	charger->usb_det 				= 0;
	charger->usb_valid 				= 0;
	if(charger->ac_det)
		charger->is_on					= 1;
	else
		charger->is_on					= 0;
#endif
	charger->ext_valid 				= charger->ac_valid | charger->usb_valid;
	charger->bat_current_direction 	= (tmp & AXP_STATUS_BATCURDIR)?1:0;
	charger->in_short 				= (tmp& AXP_STATUS_ACUSBSH)?1:0;
	charger->batery_active 			= (tmp & AXP_STATUS_BATINACT)?1:0;
	charger->low_charge_current 	= (tmp & AXP_STATUS_CHACURLOEXP)?1:0;
	charger->int_over_temp 			= (tmp & AXP_STATUS_ICTEMOV)?1:0;
	__axp_read(charger->master,AXP_CHARGE_CONTROL1,val);
	charger->charge_on 			= ((val[0] & 0x80)?1:0);
	
}
EXPORT_SYMBOL(axp_charger_update_state);

void axp_charger_update(struct axp_charger *charger)
{
	uint16_t tmp;	
	axp_read_adc(charger, &charger->adc);
	tmp = charger->adc.vbat_res;
	charger->vbat = axp_vbat_to_mV(tmp);
	charger->ibat = ABS(axp_ibat_to_mA(charger->adc.ichar_res)-axp_ibat_to_mA(charger->adc.idischar_res));
	tmp = charger->adc.vac_res;
	charger->vac = axp_vdc_to_mV(tmp);
	tmp = charger->adc.iac_res;
	charger->iac = axp_iac_to_mA(tmp);
	tmp = charger->adc.vusb_res;
	charger->vusb = axp_vdc_to_mV(tmp);
	tmp = charger->adc.iusb_res;
	charger->iusb = axp_iusb_to_mA(tmp);
	if(!charger->ext_valid){
		charger->disvbat =  charger->vbat;
		charger->disibat =  charger->ibat;
	}
}
EXPORT_SYMBOL(axp_charger_update);


int axp_charger_init(struct axp_charger *charger)
{
	int ret = -1;
	
#if 0
	axp_set_charge(charger);
	if(ret){
		printk("[AXP]Uable to set charger parameters\n");
		return ret;
	}
#endif
	ret = axp_set_ocv();
	if(ret){
		printk("[AXP]Unable to set ocv/capicaty\n");
		return ret;
	}	

	ret = axp_set_rdcclose();
	if(ret){
		printk("[AXP]Unable to close rdc calculate\n");
		return ret;
	}

	axp_charger_update_state(charger);
	axp_charger_update(charger);
	if(charger->bat_det == 1)
		axp_battery_infoinit(charger);
	
}
EXPORT_SYMBOL(axp_charger_init);

