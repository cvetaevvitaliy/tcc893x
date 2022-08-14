/*
 * tcc270.h  --  Voltage regulation for the KrossPower TCC270
 *
 * Copyright (C) 2013 by Telechips
 *
 * This program is free software
 *
 */

#ifndef REGULATOR_TCC270
#define REGULATOR_TCC270

#include <linux/regulator/machine.h>

typedef enum {
	TCC270_ID_DCDC1 = 0,
	TCC270_ID_DCDC2,
	TCC270_ID_DCDC3,
	TCC270_ID_LDO1,
	TCC270_ID_LDO2,
	TCC270_ID_LDO3,
	TCC270_ID_LDO4,
	TCC270_ID_LDO5,
	TCC270_ID_LDO6,
	TCC270_ID_LDO7,
	TCC270_ID_LDO8,		
	TCC270_ID_BOOST,
	TCC270_ID_MAX
};

/**
 * TCC270_subdev_data - regulator data
 * @id: regulator Id (either TCC270_ID_DCDC2 or TCC270_ID_DCDC3 or ,,,)
 * @name: regulator cute name (example for TCC270_ID_DCDC2: "vcc_core")
 * @platform_data: regulator init data (constraints, supplies, ...)
 */
struct tcc270_subdev_data {
	int                         id;
	char                        *name;
	struct regulator_init_data  *platform_data;
};

/**
 * tcc270_platform_data - platform data for tcc270
 * @num_subdevs: number of regulators used (may be 1 or 2 or ,,)
 * @subdevs: regulator used
 *           At most, there will be a regulator for TCC270_ID_DCDC2 and one for TCC270_ID_LDO5 voltages.
 * @init_irq: main chip irq initialize setting.
 */
struct tcc270_platform_data {
	int num_subdevs;
	struct tcc270_subdev_data *subdevs;
	int (*init_port)(int irq_num);
};

enum {
	TCC270_CHG_CURR_300mA = 0,
	TCC270_CHG_CURR_400mA,
	TCC270_CHG_CURR_500mA,
	TCC270_CHG_CURR_600mA,
	TCC270_CHG_CURR_700mA,
	TCC270_CHG_CURR_800mA,
	TCC270_CHG_CURR_900mA,
	TCC270_CHG_CURR_1000mA,
	TCC270_CHG_CURR_1100mA,
	TCC270_CHG_CURR_1200mA,
	TCC270_CHG_CURR_1300mA,
	TCC270_CHG_CURR_1400mA,
	TCC270_CHG_CURR_1500mA,
	TCC270_CHG_CURR_1600mA,
	TCC270_CHG_CURR_1700mA,
	TCC270_CHG_CURR_1800mA,
	TCC270_CHG_CURR_MAX
};

#define TCC270_CHG_OK			0x02
#define TCC270_CHG_ING			0x01
#define TCC270_CHG_NONE			0x00

extern int tcc270_battery_voltage(void);
extern int tcc270_acpin_voltage(void);
extern int tcc270_bat_detection(void);
extern int tcc270_acin_detect(void);
extern int tcc270_vbus_voltage(void);
extern void tcc270_power_off(void);
extern void tcc270_charge_current(unsigned char val);
extern int tcc270_charge_status(void);
extern void tcc270_dcdc_boost_enable(int val);
#endif

