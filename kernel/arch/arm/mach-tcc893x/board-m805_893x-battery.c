/* linux/arch/arm/mach-tcc893x/board-m805_893x-battery.c
 *
 * Copyright (C) 2013 Telechips, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/regulator/machine.h>
#include <linux/platform_data/android_battery.h>
#include <mach/gpio.h>
#include <mach/bsp.h>
#include <asm/mach-types.h>
#include <asm/system.h>
#include "devices.h"
#include "board-m805_893x.h"

static struct android_bat_callbacks *bat_callbacks;

#define CHG_AC_RUNNING_CURRENT 800
#define CHG_AC_SLEEP_CURRENT 1200
#define CHG_USB_CURRENT 500


#if defined(CONFIG_REGULATOR_AXP192)
#include <linux/regulator/axp192.h>
#include <linux/power/axp-sply.h>
extern void axp_charging_monitor(struct axp_charger *charger);
extern void axp_charger_update_state(struct axp_charger *charger);
extern void axp_charger_update(struct axp_charger *charger);
extern int axp_set_chargecurrent(int cur);
extern int axp_suspend(struct axp_charger *charger);
extern int axp_resume(struct axp_charger *charger);
struct axp_charger *charger_axp;
EXPORT_SYMBOL(charger_axp);
#endif

#if defined(CONFIG_REGULATOR_AXP202)
#include <linux/regulator/axp202.h>
#include <linux/power/axp-sply.h>
extern void axp_charging_monitor(struct axp_charger *charger);
extern void axp_charger_update_state(struct axp_charger *charger);
extern void axp_charger_update(struct axp_charger *charger);
extern int axp_set_chargecurrent(int cur);
extern int axp_get_temp(struct axp_charger *charger);
extern int axp_suspend(struct axp_charger *charger);
extern int axp_resume(struct axp_charger *charger);
struct axp_charger *charger_axp;
EXPORT_SYMBOL(charger_axp);
#endif

#if defined(CONFIG_REGULATOR_TCC270)
#include <linux/regulator/tcc270.h>
#include <linux/power/tcc-sply.h>
extern void tcc_charging_monitor(struct tcc_charger *charger);
extern void tcc_charger_update_state(struct tcc_charger *charger);
extern void tcc_charger_update(struct tcc_charger *charger);
extern int tcc_set_chargecurrent(int cur);
extern int tcc_get_temp(struct tcc_charger *charger);
extern int tcc_suspend(struct tcc_charger *charger);
extern int tcc_resume(struct tcc_charger *charger);
struct tcc_charger *charger_tcc;
EXPORT_SYMBOL(charger_tcc);
#endif

static int tcc_bat_charge_source = -1; // initial charge source

static int tcc_get_capacity(void)
{
#if defined(CONFIG_REGULATOR_AXP192) || defined(CONFIG_REGULATOR_AXP202)
	axp_charging_monitor(charger_axp);
	return charger_axp->rest_cap;
#endif
#if defined(CONFIG_REGULATOR_TCC270)
	tcc_charging_monitor(charger_tcc);
	return charger_tcc->rest_cap;
#endif

}

static int tcc_get_current(int *cur){
#if defined(CONFIG_REGULATOR_AXP192) || defined(CONFIG_REGULATOR_AXP202)	
	axp_charger_update_state(charger_axp);
	axp_charger_update(charger_axp);
	*cur = charger_axp->disibat;
	return 0;
#endif
#if defined(CONFIG_REGULATOR_TCC270)
	tcc_charger_update(charger_tcc);
	*cur = charger_tcc->curr;
	return 0;
#endif

}

static int tcc_get_voltage(void){
#if defined(CONFIG_REGULATOR_AXP192) || defined(CONFIG_REGULATOR_AXP202)	
	axp_charging_monitor(charger_axp);
	return charger_axp->ocv * 1000;
#endif
#if defined(CONFIG_REGULATOR_TCC270)	
	//tcc_charging_monitor(charger_tcc);
	return charger_tcc->vcell * 1000;
#endif

}

static void tcc_bat_register_callbacks(struct android_bat_callbacks *ptr)
{
	printk("%s\n",__func__);
	bat_callbacks = ptr;
}

static void tcc_bat_unregister_callbacks(void)
{
	printk("%s\n",__func__);
	bat_callbacks = NULL;
}

int tcc_poll_charge_source(void){
#if defined(CONFIG_REGULATOR_AXP192) || defined(CONFIG_REGULATOR_AXP202)	
	int temp_charge_source = -1;
	// TODO: wake_lock

	axp_charger_update_state(charger_axp);
	if(charger_axp->usb_det) temp_charge_source = CHARGE_SOURCE_USB;
	else if(charger_axp->ac_det) temp_charge_source = CHARGE_SOURCE_AC;
	else temp_charge_source = CHARGE_SOURCE_NONE;
	
	if(tcc_bat_charge_source != temp_charge_source)
	{
		if(charger_axp->usb_det && bat_callbacks && bat_callbacks->charge_source_changed){
			bat_callbacks->charge_source_changed(bat_callbacks, CHARGE_SOURCE_USB);
			tcc_bat_charge_source = CHARGE_SOURCE_USB;
			return CHARGE_SOURCE_USB;
		}else if(charger_axp->ac_det && bat_callbacks && bat_callbacks->charge_source_changed){
			bat_callbacks->charge_source_changed(bat_callbacks, CHARGE_SOURCE_AC);			
			tcc_bat_charge_source = CHARGE_SOURCE_AC;			
			return CHARGE_SOURCE_AC;
		}else if(bat_callbacks && bat_callbacks->charge_source_changed){
			bat_callbacks->charge_source_changed(bat_callbacks, CHARGE_SOURCE_NONE);			
			tcc_bat_charge_source = CHARGE_SOURCE_NONE;			
			return CHARGE_SOURCE_NONE;
		}
	}
	return tcc_bat_charge_source;
#endif	
#if defined(CONFIG_REGULATOR_TCC270)	
	int temp_charge_source = -1;
	// TODO: wake_lock

	tcc_charger_update_state(charger_tcc);
	if(charger_tcc->usb_det) temp_charge_source = CHARGE_SOURCE_USB;
	else if(charger_tcc->ac_det) temp_charge_source = CHARGE_SOURCE_AC;
	else temp_charge_source = CHARGE_SOURCE_NONE;
	
	if(tcc_bat_charge_source != temp_charge_source)
	{
		if(charger_tcc->usb_det && bat_callbacks && bat_callbacks->charge_source_changed){
			bat_callbacks->charge_source_changed(bat_callbacks, CHARGE_SOURCE_USB);
			tcc_bat_charge_source = CHARGE_SOURCE_USB;
			return CHARGE_SOURCE_USB;
		}else if(charger_tcc->ac_det && bat_callbacks && bat_callbacks->charge_source_changed){
			bat_callbacks->charge_source_changed(bat_callbacks, CHARGE_SOURCE_AC);			
			tcc_bat_charge_source = CHARGE_SOURCE_AC;			
			return CHARGE_SOURCE_AC;
		}else if(bat_callbacks && bat_callbacks->charge_source_changed){
			bat_callbacks->charge_source_changed(bat_callbacks, CHARGE_SOURCE_NONE);			
			tcc_bat_charge_source = CHARGE_SOURCE_NONE; 		
			return CHARGE_SOURCE_NONE;
		}
	}
	return tcc_bat_charge_source;
#endif	

}
EXPORT_SYMBOL(tcc_poll_charge_source);

static void tcc_set_charging_current(int android_charge_source)
{
#if defined(CONFIG_REGULATOR_AXP192) || defined(CONFIG_REGULATOR_AXP202)	
	if(android_charge_source == CHARGE_SOURCE_AC)
		axp_set_chargecurrent(CHG_AC_RUNNING_CURRENT);
	else if(android_charge_source == CHARGE_SOURCE_USB)
		axp_set_chargecurrent(CHG_USB_CURRENT);
#endif
#if defined(CONFIG_REGULATOR_TCC270)	
	if(android_charge_source == CHARGE_SOURCE_AC)
		tcc_set_chargecurrent(CHG_AC_RUNNING_CURRENT);
	else if(android_charge_source == CHARGE_SOURCE_USB)
		tcc_set_chargecurrent(CHG_USB_CURRENT);
#endif

}

static void tcc_set_charging_enable(int en)
{
#if defined(CONFIG_REGULATOR_AXP192) || defined(CONFIG_REGULATOR_AXP202)	
	if(en){
		if(charger_axp->ac_det)
			axp_set_chargecurrent(CHG_AC_RUNNING_CURRENT);
		else if(charger_axp->usb_det)
			axp_set_chargecurrent(CHG_USB_CURRENT);
	}else
		axp_set_chargecurrent(0); // charge disable
#endif
#if defined(CONFIG_REGULATOR_TCC270)	
	if(en){
		if(charger_tcc->ac_det)
			tcc_set_chargecurrent(CHG_AC_RUNNING_CURRENT);
		else if(charger_tcc->usb_det)
			tcc_set_chargecurrent(CHG_USB_CURRENT);
	}else
		tcc_set_chargecurrent(0); // charge disable
#endif

}

static int tcc_get_temperature(int *cur)
{
#if defined(CONFIG_REGULATOR_AXP192) || defined(CONFIG_REGULATOR_AXP202)	
// if bat temperature is not exist, you can use internal temperature of PMU
	#if defined(CONFIG_BATTERY_TEMP)
		//TODO:check battery temperature
	#else // use internal temperature of PMU
		#if defined(CONFIG_REGULATOR_AXP202) // AXP192 does not support internal temperature
			return axp_get_temp(charger_axp);
		#else
			return 55;
		#endif
	#endif
#else
	return 55;
#endif
#if defined(CONFIG_REGULATOR_TCC270)	
	return tcc_get_temp(charger_tcc);
#endif

}

static int tcc_battery_suspend(void)
{
#if defined(CONFIG_REGULATOR_AXP192) || defined(CONFIG_REGULATOR_AXP202)	
	return axp_suspend(charger_axp);
#endif
#if defined(CONFIG_REGULATOR_TCC270)	
	return tcc_suspend(charger_tcc);
#endif

}

static int tcc_battery_resume(void)
{
#if defined(CONFIG_REGULATOR_AXP192) || defined(CONFIG_REGULATOR_AXP202)	
	return axp_resume(charger_axp);
#endif
#if defined(CONFIG_REGULATOR_TCC270)	
	return tcc_resume(charger_tcc);
#endif

}

struct android_bat_platform_data tcc_charger_pdata = {
	.register_callbacks			= tcc_bat_register_callbacks,
	.unregister_callbacks		= tcc_bat_unregister_callbacks,
	.set_charging_current 		= tcc_set_charging_current,
	.set_charging_enable 		= tcc_set_charging_enable,
	.battery_suspend			= tcc_battery_suspend,
	.battery_resume				= tcc_battery_resume,
	.poll_charge_source 		= tcc_poll_charge_source,
	.get_capacity				= tcc_get_capacity,
	.get_temperature			= tcc_get_temperature,
	.get_voltage_now			= tcc_get_voltage,
	.get_current_now			= tcc_get_current,
#if defined(CONFIG_REGULATOR_AXP192) || defined(CONFIG_REGULATOR_AXP202)		
	.temp_high_threshold		= 60,
	.temp_high_recovery		= 42,
	.temp_low_recovery		= 0,
	.temp_low_threshold		= 50,
	.full_charging_time			= 12 * 60 * 60,
	.recharging_time			= 2 * 60 * 60,
	.recharging_voltage		= 4200,
#elif defined(CONFIG_REGULATOR_TCC270)
	.temp_high_threshold		= 650,
	.temp_high_recovery		= 430,
	.temp_low_recovery		= 0,
	.temp_low_threshold		= (-30),
	.full_charging_time			= 12 * 60 * 60, // 12h
	.recharging_time			= 2 * 60 * 60, // 2h
	.recharging_voltage		= 4200, // 4.2 v
#else
	.temp_high_threshold		= 60, // 60'C
	.temp_high_recovery 	= 42, // 42'C
	.temp_low_recovery		= 0,
	.temp_low_threshold 	= 50, // 50'C
	.full_charging_time 		= 12 * 60 * 60, // 12h
	.recharging_time			= 2 * 60 * 60, // 2h
	.recharging_voltage 	= 4200, // 4.2 v
#endif
};

static struct platform_device tcc_charger_device = {
	.name		= "android-battery",
	.id		= -1,
	.dev = {
		.platform_data = &tcc_charger_pdata
	},
};

void __init m805_893x_init_battery(void)
{
	if (!machine_is_m805_893x())
		return;

	printk("%s\n",__func__);

#if defined(CONFIG_REGULATOR_AXP192) || defined(CONFIG_REGULATOR_AXP202)		
	charger_axp = kzalloc(sizeof(*charger_axp), GFP_KERNEL);
	if (charger_axp == NULL)
		return -ENOMEM;

	charger_axp->master = NULL;
#endif
#if defined(CONFIG_REGULATOR_TCC270)		
		charger_tcc = kzalloc(sizeof(*charger_tcc), GFP_KERNEL);
		if (charger_tcc == NULL)
			return -ENOMEM;
	
		charger_tcc->master = NULL;
#endif

	platform_device_register(&tcc_charger_device);

}
