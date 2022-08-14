/*
 * tcc270.c  --  Voltage and current regulation for the TCC270
 *
 * Copyright (C) 2013 by Telechips
 *
 * This program is free software;
 *
 */
#include <linux/module.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/power/tcc-sply.h>
#include <asm/mach-types.h>

#include <linux/regulator/tcc270.h>

#if defined(DEBUG)
#define dbg(msg...) printk("TCC270: " msg)
#else
#define dbg(msg...)
#endif

#define TCC270_MIN_UV   700000
#define TCC270_MAX_UV  3500000

/********************************************************************
	Define Types
********************************************************************/
struct tcc270_voltage_t {
	int uV;
	u8  val;
};

struct tcc270_data {
	struct i2c_client *client;
	unsigned int min_uV;
	unsigned int max_uV;
	struct work_struct work;
#if defined(CONFIG_REGULATOR_TCC270_PEK)
	struct input_dev *input_dev;
	struct timer_list timer;
#endif
	struct regulator_dev *rdev[0];
};
#define TCC270_VBATS_HIGH_REG					0x58
#define TCC270_VBATS_LOW_REG					0x59


/********************************************************************
	I2C Command & Values
********************************************************************/
//config & setting
#define TCC270_DEVICE_ID_REG 					0x00
#define TCC270_RESET_CONTROL_REG				0x15
#define TCC270_POK_TIME_SETTING_REG			0x19
#define TCC270_SHDN_CONTROL_REG				0x1A
#define TCC270_POWEROFF_CONTROL_REG			0x1B
#define TCC270_OFF_MODE_CONFIG_REG			0x5D

// DC/DC
#define TCC270_BUCK1_CONTROL_REG				0x08
#define TCC270_BUCK2_CONTROL_REG				0x09
#define TCC270_BUCK3_CONTROL_REG				0x0A
#define TCC270_BOOST_REG						0x0B
#define TCC270_BUCK_MODE_REG					0x0C
#define TCC270_VSYS_BUCK_ENABLE_REG			0x17	

// LDO
#define TCC270_LDO2_CONTROL_REG				0x0D
#define TCC270_LDO3_CONTROL_REG				0x0E
#define TCC270_LDO4_CONTROL_REG				0x0F
#define TCC270_LDO5_CONTROL_REG				0x10
#define TCC270_LDO6_CONTROL_REG				0x11
#define TCC270_LDO7_CONTROL_REG				0x12
#define TCC270_LDO1_8_CONTROL_REG				0x13
#define TCC270_LDO_MODE_REG					0x14
#define TCC270_LDO_ENABLE_REG					0x18

// GPIO
#define TCC270_GPIO0_FUNCTION_REG				0x1C
#define TCC270_GPIO1_FUNCTION_REG				0x1D
#define TCC270_GPIO2_FUNCTION_REG				0x1E	
#define TCC270_DCDC4_OCP_REG					0xA9

// EVENT & IRQ
#define TCC270_EVENT_STANBY_MODE_REG			0x16
#define TCC270_OFF_EVENT_STATUS_REG			0x20
#define TCC270_IRQ1_ENABLE_REG					0x30
#define TCC270_IRQ1_STATUS_REG					0x31
#define TCC270_IRQ2_ENABLE_REG					0x32
#define TCC270_IRQ2_STATUS_REG					0x33
#define TCC270_IRQ3_ENABLE_REG					0x34
#define TCC270_IRQ3_STATUS_REG					0x35
#define TCC270_IRQ4_ENABLE_REG					0x36
#define TCC270_IRQ4_STATUS_REG					0x37
#define TCC270_IRQ5_ENABLE_REG					0x38
#define TCC270_IRQ5_STATUS_REG					0x39
#define TCC270_IRQ_CONTROL_REG				0x50
#define TCC270_IRQ_FLG_REG						0x51

// CHARGE CONFIG
#define TCC270_CHGCONTROL1_REG				0x01
#define TCC270_CHGCONTROL2_REG				0x02
#define TCC270_CHGCONTROL3_REG				0x03
#define TCC270_CHGCONTROL4_REG				0x04
#define TCC270_CHGCONTROL5_REG				0x05
#define TCC270_CHGCONTROL6_REG				0x06
#define TCC270_CHGCONTROL7_REG				0x07

// VOLTAGE & CURRENT
#define TCC270_VBATS_HIGH_REG					0x58
#define TCC270_VBATS_LOW_REG					0x59
#define TCC270_INTEMP_HIGH_REG					0x5A
#define TCC270_INTEMP_LOW_REG					0x5B
#define TCC270_RSVD2_REG						0x5C
#define TCC270_AIN_VOLTAGE_HIGH_REG			0x5E
#define TCC270_AIN_VOLTAGE_LOW_REG			0x5F
#define TCC270_ACIN_VOLTAGE_HIGH_REG			0x64
#define TCC270_ACIN_VOLTAGE_LOW_REG			0x65
#define TCC270_VBUS_VOLTAGE_HIGH_REG			0x66
#define TCC270_VBUS_VOLTAGE_LOW_REG			0x67
#define TCC270_VSYS_VOLTAGE_HIGH_REG			0x68
#define TCC270_VSYS_VOLTAGE_LOW_REG			0x69
#define TCC270_GPIO0_VOLTAGE_HIGH_REG		0x6A
#define TCC270_GPIO0_VOLTAGE_LOW_REG			0x6B
#define TCC270_GPIO1_VOLTAGE_HIGH_REG		0x6C
#define TCC270_GPIO1_VOLTAGE_LOW_REG			0x6D
#define TCC270_GPIO2_VOLTAGE_HIGH_REG		0x6E
#define TCC270_GPIO2_VOLTAGE_LOW_REG			0x6F
#define TCC270_BUCK1_VOLTAGE_HIGH_REG		0x70
#define TCC270_BUCK1_VOLTAGE_LOW_REG			0x71
#define TCC270_BUCK2_VOLTAGE_HIGH_REG		0x72
#define TCC270_BUCK2_VOLTAGE_LOW_REG			0x73
#define TCC270_BUCK3_VOLTAGE_HIGH_REG		0x74
#define TCC270_BUCK3_VOLTAGE_LOW_REG			0x75
#define TCC270_VBAT_CURRENT_HIGH_REG			0x76
#define TCC270_VBAT_CURRENT_LOW_REG			0x77

// COULOMB COUNTER
#define TCC270_COULOMB_TIMER_HIGH_REG 		0x60
#define TCC270_COULOMB_TIMER_LOW_REG		0x61
#define TCC270_COULOMB_CHANNEL_HIGH_REG 	0x62
#define TCC270_COULOMB_CHANNEL_LOW_REG		0x63
#define TCC270_COULOMB_COUNTER_CHG_H_H		0x78
#define TCC270_COULOMB_COUNTER_CHG_H_L		0x79
#define TCC270_COULOMB_COUNTER_CHG_L_H		0x7A
#define TCC270_COULOMB_COUNTER_CHG_L_L		0x7B
#define TCC270_COULOMB_COUNTER_DISCHG_H_H		0x7C
#define TCC270_COULOMB_COUNTER_DISCHG_H_L		0x7D
#define TCC270_COULOMB_COUNTER_DISCHG_L_H		0x7E
#define TCC270_COULOMB_COUNTER_DISCHG_L_L		0x7F

// UNKNOWN
#define TCC270_RSVD1_REG						0x52
#define TCC270_VARLTMAX_REG					0x53
#define TCC270_VARLTMIN1_REG					0x54
#define TCC270_VARLTMIN2_REG					0x55
#define TCC270_TARLTMAX_REG					0x56
#define TCC270_TARLTMIN_REG					0x57
#define TCC270_AIN_HIGH_REG 					0x64
#define TCC270_AIN_LOW_REG 				0x65



// IRQ values
#define TCC270_IRQ1_ACIN_OVP					0x80
#define TCC270_IRQ1_ACIN_CONNECTED			0x40
#define TCC270_IRQ1_ACIN_REMOVED				0x20
#define TCC270_IRQ1_VBUS_OVP					0x10
#define TCC270_IRQ1_VBUS_CONNECTED			0x08
#define TCC270_IRQ1_VBUS_REMOVED				0x04
#define TCC270_IRQ1_BAT_ABSENCE				0x01

#define TCC270_IRQ2_ACIN_SLEEP_MODE			0x80
#define TCC270_IRQ2_ACIN_BAD					0x40
#define TCC270_IRQ2_ACIN_GOOD					0x20
#define TCC270_IRQ2_VBUS_SLEEP_MODE			0x10
#define TCC270_IRQ2_VBUS_BAD					0x08
#define TCC270_IRQ2_VBUS_GOOD					0x04
#define TCC270_IRQ2_BAT_OVP					0x02
#define TCC270_IRQ2_CHARGE_TERMINATED			0x01

#define TCC270_IRQ3_RECHARGE_REQUEST			0x80
#define TCC270_IRQ3_CHARGER_WARNING1			0x40
#define TCC270_IRQ3_CHARGER_WARNING2			0x20
#define TCC270_IRQ3_READY_TO_CHARGE			0x10
#define TCC270_IRQ3_FAST_CHARGE				0x08
#define TCC270_IRQ3_CHG_INSUFFICIENT			0x04
#define TCC270_IRQ3_TIMEOUT_PRECHARGE		0x02
#define TCC270_IRQ3_TIMEOUT_FASTCHARGE		0x01

#define TCC270_IRQ4_OVER_TEMPERATURE			0x80
#define TCC270_IRQ4_BUCK1_LOWER				0x40
#define TCC270_IRQ4_BUCK2_LOWER				0x20
#define TCC270_IRQ4_BUCK3_LOWER				0x10
#define TCC270_IRQ4_POK_SHORT_PRESS			0x08
#define TCC270_IRQ4_POK_LONG_PRESS			0x04
#define TCC270_IRQ4_BOOST_LOWER				0x02
#define TCC270_IRQ4_VSYS_LOWER				0x01

#define TCC270_IRQ5_POK_FORCE_SHDN			0x80
#define TCC270_IRQ5_POK_RISING_EDGE			0x40
#define TCC270_IRQ5_POK_FALLING_EDGE			0x20
#define TCC270_IRQ5_RSTINB_DEGLITCH			0x10
#define TCC270_IRQ5_GPIO2_INPUT_EDGE			0x08
#define TCC270_IRQ5_GPIO1_INPUT_EDGE			0x04
#define TCC270_IRQ5_GPIO0_INPUT_EDGE			0x02


/* initial setting values */
#define TCC270_IRQ1_1 ( 0 \
		)
#define TCC270_IRQ4_1 ( 0 \
			)


#define TCC270_IRQ1 ( 0 \
	| TCC270_IRQ1_ACIN_CONNECTED \
	| TCC270_IRQ1_ACIN_REMOVED \
		)
	
#define TCC270_IRQ2 ( 0 \
		)
		
#define TCC270_IRQ3 ( 0 \
		)
		
#define TCC270_IRQ4 ( 0 \
	| TCC270_IRQ4_POK_SHORT_PRESS \
	| TCC270_IRQ4_POK_LONG_PRESS \
		)
		
#define TCC270_IRQ5 ( 0 \
		)

/* LDO1 voltage level */
static struct tcc270_voltage_t ldo1_voltages[] = {
	{ 1500, 0x00 }, { 1800, 0x01 }, { 2500, 0x02 }, { 3000, 0x03 },
	{ 3300, 0x04 }, { 3300, 0x05 }, { 3300, 0x06 }, { 3300, 0x07 },
};

#define NUM_LDO1	ARRAY_SIZE(ldo1_voltages)


/********************************************************************
	Variables
********************************************************************/
static struct workqueue_struct *tcc270_wq;
static struct i2c_client *tcc270_i2c_client = NULL;
static int tcc270_acin_det = 0;
static int tcc270_bat_det = 0;
static int tcc270_charge_sts = 0;
static unsigned int tcc270_suspend_status = 0;
#if defined(CONFIG_BATTERY_ANDROID)
extern int tcc_poll_charge_source(void);
extern void tcc_charger_init(struct tcc_charger *charger);
extern struct tcc_charger *charger_tcc;
#endif

static int tcc270_i2c_read(char* rxData, int rxLength)
{
	int rc;
	struct i2c_client *client = tcc270_i2c_client;
	struct i2c_adapter *adap = client->adapter;
	struct i2c_msg msg[2] = {
	//msg 1  send the addr + cmd reg addr
	{
		.addr = client->addr,
		.flags = client->flags,
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

static int tcc270_i2c_write(char* txData, int txLength)
{
	int rc;
	struct i2c_client *client = tcc270_i2c_client;
	struct i2c_adapter *adap = client->adapter;
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
static int tcc270_read_reg(u8 reg, u8 *data, u8 len)
{
	struct i2c_adapter *adap = tcc270_i2c_client->adapter;
	struct i2c_msg msgs[2];
	int ret;
	
	msgs[0].addr = tcc270_i2c_client->addr;
	msgs[0].flags = tcc270_i2c_client->flags;
	msgs[0].len = 1;
	msgs[0].buf = &reg;

	msgs[1].addr = tcc270_i2c_client->addr;
	msgs[1].flags = tcc270_i2c_client->flags | I2C_M_RD;
	msgs[1].len = len;
	msgs[1].buf = data;
	
	ret = i2c_transfer(adap, msgs, 2);
	 
	return (ret == 2)? len : ret;									 
}

/********************************************************************
	Functions (Global)
********************************************************************/
int tcc270_acpin_voltage(void)
{
	u8 data[2];
  	u16 temp;
  	s16 temp_return;
  	int sign = 0;

	
  if (tcc270_read_reg(TCC270_REG_CURRENT_MSB, data, 2) < 0) {
	printk(KERN_ERR "%s: Failed to read CURRENT\n", __func__);
  }

  temp = (data[0]<<8) | data[1];
  if (data[0] & (1 << 7)) {
	sign = 1;
	temp = (((temp & 0x7FFF) * 3125) / 10 + 50) / 1000;
  }else
		temp = ((temp * 3125) / 10 - 50) / 1000;

  if (sign)
	temp *= -1;
	temp_return = temp;
	return temp_return;
}

int tcc270_bat_detection(void)
{
	return tcc270_bat_det;
}


int tcc270_battery_voltage(void)
{
	signed long data[2];
	int ret = 4200;

	data[0] = i2c_smbus_read_byte_data(tcc270_i2c_client, TCC270_VBATS_HIGH_REG);
	data[1] = i2c_smbus_read_byte_data(tcc270_i2c_client, TCC270_VBATS_LOW_REG);
	ret = (data[0] << 8) | data[1];

	//dbg("%s: %dmV\n", __func__, ret);
	return ret;
}

int tcc270_acin_detect(void)
{
	return tcc270_acin_det;
}

void tcc270_power_off(void)
{
	int value;

	dbg("%s\n", __func__);	
	
	value = i2c_smbus_read_byte_data(tcc270_i2c_client, TCC270_SHDN_CONTROL_REG);
	value = value | 0x80;
	i2c_smbus_write_byte_data(tcc270_i2c_client, TCC270_SHDN_CONTROL_REG, value);
}

void tcc270_dcdc_boost_enable(int en)
{
	char regdata, xfer[2];
	int ret;

	dbg("%s: ++\n", __func__);

	xfer[0] = regdata = TCC270_VSYS_BUCK_ENABLE_REG;
	ret = tcc270_i2c_read(&regdata,1);
	if (ret<0)
	{
		dbg("read buck enable reg fail\n");
	}
	else {
		xfer[1] = ( en? (regdata|(0x1<<3)):(regdata& ~(0x1<<3)));
		ret = tcc270_i2c_write(xfer,2);
	}
	//mdelay(200);
	
}

EXPORT_SYMBOL(tcc270_bat_detection);
EXPORT_SYMBOL(tcc270_acpin_voltage);
EXPORT_SYMBOL(tcc270_battery_voltage);
EXPORT_SYMBOL(tcc270_acin_detect);
EXPORT_SYMBOL(tcc270_power_off);
//EXPORT_SYMBOL(tcc270_charge_current);
//EXPORT_SYMBOL(tcc270_charge_status);
//EXPORT_SYMBOL(tcc270_get_batt_discharge_current);
//EXPORT_SYMBOL(tcc270_get_batt_charge_current);

/*******************************************
	Functions (Internal)
*******************************************/
static void tcc270_work_func(struct work_struct *work)
{
	struct tcc270_data* tcc270 = container_of(work, struct tcc270_data, work);
	unsigned char data[5];
	int i2c_send_count =0;
	
	dbg("%s\n", __func__);

	data[0] =0;
	data[1] =0;
	data[2] =0;
	data[3] =0;
	data[4] =0;
	
	data[0] = i2c_smbus_read_byte_data(tcc270->client, TCC270_IRQ1_STATUS_REG);
	while(i2c_smbus_read_byte_data(tcc270->client, TCC270_IRQ1_STATUS_REG) != 0)
	{
		i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ1_STATUS_REG, 0xFF);
		if(i2c_send_count++ > 10)
			break;
	}
	i2c_send_count =0;
	
	data[1] = i2c_smbus_read_byte_data(tcc270->client, TCC270_IRQ2_STATUS_REG);
	while(i2c_smbus_read_byte_data(tcc270->client, TCC270_IRQ2_STATUS_REG) != 0)
	{
		i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ2_STATUS_REG, 0xFF);
		if(i2c_send_count++ > 10)
			break;
	}
	i2c_send_count =0;
		
	data[2] = i2c_smbus_read_byte_data(tcc270->client, TCC270_IRQ3_STATUS_REG);
	while(i2c_smbus_read_byte_data(tcc270->client, TCC270_IRQ3_STATUS_REG) != 0)
	{
		i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ3_STATUS_REG, 0xFF);
		if(i2c_send_count++ > 10)
			break;
	}
	i2c_send_count =0;
		
	data[3] = i2c_smbus_read_byte_data(tcc270->client, TCC270_IRQ4_STATUS_REG);
	while(i2c_smbus_read_byte_data(tcc270->client, TCC270_IRQ4_STATUS_REG) != 0)
	{
		i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ4_STATUS_REG, 0xFF);
		if(i2c_send_count++ > 10)
			break;
	}
	i2c_send_count =0;
	
	data[4] = i2c_smbus_read_byte_data(tcc270->client, TCC270_IRQ5_STATUS_REG);
	while(i2c_smbus_read_byte_data(tcc270->client, TCC270_IRQ5_STATUS_REG) != 0)
	{
		i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ5_STATUS_REG, 0xFF);
		if(i2c_send_count++ > 10)
			break;
	}
	i2c_send_count =0;

	i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ1_ENABLE_REG, TCC270_IRQ1_1);
	//i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ2_ENABLE_REG, NULL);
	//i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ3_ENABLE_REG, NULL);
	i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ4_ENABLE_REG, TCC270_IRQ4_1);
	//i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ5_ENABLE_REG, NULL);
	
	if(data[0] & TCC270_IRQ1_BAT_ABSENCE){
		dbg("BAT absence, IRQ status\n");
		tcc270_bat_det = 1;
	}
	if(data[0] & TCC270_IRQ1_ACIN_OVP){
		dbg("ACIN over-voltage, IRQ status\n");
	}
	if (data[0]&TCC270_IRQ1_ACIN_CONNECTED) {
		dbg("ACIN connected, IRQ status\n");
		tcc270_acin_det = 1;
		printk("AC_IN : %x\n", data[0]);
#if defined(CONFIG_BATTERY_ANDROID)
		tcc_poll_charge_source();
#endif
	}
	if (data[0]&TCC270_IRQ1_ACIN_REMOVED) {
		dbg("ACIN removed, IRQ status\n");
		tcc270_acin_det = 0;
		tcc270_charge_sts = TCC270_CHG_NONE;
		printk("AC_OUT : %x\n", data[0]);
#if defined(CONFIG_BATTERY_ANDROID)
		tcc_poll_charge_source();
#endif
	}
	if (data[0]&TCC270_IRQ1_VBUS_OVP) {
		dbg("VBUS over-voltage, IRQ status\n");
	}
	if (data[0]&TCC270_IRQ1_VBUS_CONNECTED) {
		dbg("VBUS connected, IRQ status\n");
#if defined(CONFIG_BATTERY_ANDROID)
		tcc_poll_charge_source();
#endif
	}
	if (data[0]&TCC270_IRQ1_VBUS_REMOVED) {
		dbg("VBUS removed, IRQ status\n");
#if defined(CONFIG_BATTERY_ANDROID)
		tcc_poll_charge_source();
#endif
	}
	if (data[0]&TCC270_IRQ1_BAT_ABSENCE) {
		dbg("BAT absence, IRQ status\n");
	}
	if (data[1]&TCC270_IRQ2_ACIN_SLEEP_MODE) {
		dbg("Charger fault. ACIN Sleep mode, IRQ status\n");
	}
	if (data[1]&TCC270_IRQ2_ACIN_BAD) {
		dbg("Charger fault. Bad ACIN, IRQ status\n");
	}
	if (data[1]&TCC270_IRQ2_ACIN_GOOD) {
		dbg("Good ACIN, IRQ status\n");
	}
	if (data[1]&TCC270_IRQ2_VBUS_SLEEP_MODE) {
		dbg("Charger fault. VBUS Sleep mode, IRQ status\n");
	}
	if (data[1]&TCC270_IRQ2_VBUS_BAD) {
		dbg("Charger fault. Bad VBUS, IRQ status\n");
	}
	if (data[1]&TCC270_IRQ2_VBUS_GOOD) {
		dbg("Good VBUS, IRQ status\n");
	}
	if (data[1]&TCC270_IRQ2_BAT_OVP) {
		dbg("Battery over-voltage, IRQ status\n");
	}
	if (data[1]&TCC270_IRQ2_CHARGE_TERMINATED) {
		dbg("Charge terminated , IRQ status\n");
	}
	if (data[2]&TCC270_IRQ3_RECHARGE_REQUEST) {
		dbg("Charge recharge request, IRQ status\n");
	}
	if (data[2]&TCC270_IRQ3_CHARGER_WARNING1) {
		dbg("Charge warning. Thermal regulation loop active, IRQ status\n");
	}
	if (data[2]&TCC270_IRQ3_CHARGER_WARNING2) {
		dbg("Charge warning. Input voltage regulation loop active , IRQ status\n");
	}
	if (data[2]&TCC270_IRQ3_READY_TO_CHARGE) {
		dbg("ACIN or VBUS power good and CHG_BUCK is ready for charging path, IRQ status\n");
	}
	if (data[2]&TCC270_IRQ3_FAST_CHARGE) {
		dbg("Operates in fast-Charge state, IRQ status\n");
	}
	if (data[2]&TCC270_IRQ3_TIMEOUT_PRECHARGE) {
		dbg("Pre-charge timeout. AC plug or USB plug or CHG_EN rising edge can trigger, IRQ status\n");
	}
	if (data[2]&TCC270_IRQ3_TIMEOUT_FASTCHARGE) {
		dbg("Fast charge timeout. AC plug or USB plug or CHG_EN or recharge signal rising edge can clear, IRQ status\n");
	}
	if (data[3]&TCC270_IRQ4_OVER_TEMPERATURE) {
		dbg("Internal over temperature, IRQ status\n");
	}
	if (data[3]&TCC270_IRQ4_BUCK1_LOWER) {
		dbg("Buck1 ouput voltage is lower than 10%, IRQ status\n");
	}
	if (data[3]&TCC270_IRQ4_BUCK2_LOWER) {
		dbg("Buck2 ouput voltage is lower than 10%, IRQ status\n");
	}
	if (data[3]&TCC270_IRQ4_BUCK3_LOWER) {
		dbg("Buck3 ouput voltage is lower than 10%, IRQ status\n");
	}
	if (data[3]&TCC270_IRQ4_POK_SHORT_PRESS) {
#if defined(CONFIG_REGULATOR_TCC270_PEK)
    		del_timer(&tcc270->timer);
		input_event(tcc270->input_dev, EV_KEY, KEY_POWER, 1);
		input_event(tcc270->input_dev, EV_SYN, 0, 0);
		tcc270->timer.expires = jiffies + msecs_to_jiffies(100);
		add_timer(&tcc270->timer);
#endif		
		dbg("POK short press, IRQ status\n");
	}
	if (data[3]&TCC270_IRQ4_POK_LONG_PRESS){
#if defined(CONFIG_REGULATOR_TCC270_PEK)
        del_timer(&tcc270->timer);
		input_event(tcc270->input_dev, EV_KEY, KEY_POWER, 1);
		input_event(tcc270->input_dev, EV_SYN, 0, 0);
		tcc270->timer.expires = jiffies + msecs_to_jiffies(1000);
		add_timer(&tcc270->timer);
#endif		
		dbg("POK long press, IRQ status\n");
	}
	if (data[3]&TCC270_IRQ4_BOOST_LOWER) {
		dbg("BOOST output is lower than 10%, IRQ status\n");
	}
	if (data[3]&TCC270_IRQ4_VSYS_LOWER) {
		dbg("SYS voltage is lower than VOFF, IRQ status\n");
	}
	if (data[4]&TCC270_IRQ5_POK_FORCE_SHDN) {
		dbg("Key-press forced shutdown, IRQ status\n");
	}
	if (data[4]&TCC270_IRQ5_POK_RISING_EDGE) {
		dbg("POK press rising edge, IRQ status\n");
	}
	if (data[4]&TCC270_IRQ5_POK_FALLING_EDGE) {
		dbg("POK press falling edge, IRQ status\n");
	}
	if (data[4]&TCC270_IRQ5_RSTINB_DEGLITCH) {
		dbg("From high to low input into RSTINB pin with over 100ms deglitch time, IRQ status\n");
	}
	if (data[4]&TCC270_IRQ5_GPIO2_INPUT_EDGE) {
		dbg("GPIO2 input edge trigger, IRQ status\n");
	}
	if (data[4]&TCC270_IRQ5_GPIO1_INPUT_EDGE) {
		dbg("GPIO1 input edge trigger, IRQ status\n");
	}
	if (data[4]&TCC270_IRQ5_GPIO0_INPUT_EDGE) {
		dbg("GPIO0 input edge trigger, IRQ status\n");
	}

	i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ1_ENABLE_REG, TCC270_IRQ1);
	i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ2_ENABLE_REG, TCC270_IRQ2);
	i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ3_ENABLE_REG, TCC270_IRQ3);
	i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ4_ENABLE_REG, TCC270_IRQ4);
	i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ5_ENABLE_REG, TCC270_IRQ5);
	
	while(i2c_smbus_read_byte_data(tcc270->client, TCC270_IRQ1_STATUS_REG) != 0)
	{
		i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ1_STATUS_REG, 0xFF);
		if(i2c_send_count++ > 10)
			break;
	}
	i2c_send_count = 0;
	while(i2c_smbus_read_byte_data(tcc270->client, TCC270_IRQ2_STATUS_REG) != 0)
	{
		i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ2_STATUS_REG, 0xFF);
		if(i2c_send_count++ > 10)
			break;
	}
	i2c_send_count = 0;
	while(i2c_smbus_read_byte_data(tcc270->client, TCC270_IRQ3_STATUS_REG) != 0)
	{
		i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ3_STATUS_REG, 0xFF);
		if(i2c_send_count++ > 10)
			break;
	}
	i2c_send_count = 0;
	while(i2c_smbus_read_byte_data(tcc270->client, TCC270_IRQ4_STATUS_REG) != 0)
	{
		i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ4_STATUS_REG, 0xFF);
		if(i2c_send_count++ > 10)
			break;
	}
	i2c_send_count = 0;
	while(i2c_smbus_read_byte_data(tcc270->client, TCC270_IRQ5_STATUS_REG) != 0)
	{
		i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ5_STATUS_REG, 0xFF);
		if(i2c_send_count++ > 10)
			break;
	}
	i2c_send_count = 0;
	enable_irq(tcc270->client->irq);
}

static irqreturn_t tcc270_interrupt(int irqno, void *param)
{
	struct tcc270_data *tcc270 = (struct tcc270_data *)param;
	dbg("%s\n", __func__);

	disable_irq_nosync(tcc270->client->irq);
	queue_work(tcc270_wq, &tcc270->work);
	return IRQ_HANDLED;
}

#if defined(CONFIG_REGULATOR_TCC270_PEK)
static void tcc270_timer_func(unsigned long data)
{
	struct tcc270_data *tcc270 = (struct tcc270_data *)data;
	dbg("%s\n", __func__);

	input_event(tcc270->input_dev, EV_KEY, KEY_POWER, 0);
	input_event(tcc270->input_dev, EV_SYN, 0, 0);
}
#endif

static int tcc270_dcdc_set_voltage(struct regulator_dev *rdev, int min_uV, int max_uV, unsigned *selector)
{
	struct tcc270_data* tcc270 = rdev_get_drvdata(rdev);
	int id = rdev_get_id(rdev);
	u8 reg, value = 0, old_value = 0;
	int max, step, vrc, ret;

	if(tcc270_suspend_status)
		return -EBUSY;

	switch (id) {
		case TCC270_ID_DCDC1:
			reg = TCC270_BUCK1_CONTROL_REG;
			max = 2275; step = 25; vrc = 2; //(max v : 2.275v, step : 25mv , vrc : 2bit)
			break;
		case TCC270_ID_DCDC2:
			reg = TCC270_BUCK2_CONTROL_REG;
			max = 3500; step = 25; vrc = 1; //(max v : 3.5v, step : 25mv , vrc : 1bit)
			break;
		case TCC270_ID_DCDC3:
			reg = TCC270_BUCK3_CONTROL_REG;
			max = 3500; step = 50; vrc = 2; //(max v : 3.5v, step : 50mv , vrc : 2bit)
			break;
		default:
			return -1;
	}

	min_uV /= 1000;

	if (min_uV > max || min_uV < 700){
		dbg("%s: Wrong BUCK%d value!\n",__func__, id);
		return -EINVAL;
	}

	
	old_value = i2c_smbus_read_byte_data(tcc270->client, reg);
	value = ((min_uV - 700) / step) << vrc;
	value = value | (old_value & (vrc + vrc/2));


	dbg("%s: reg:0x%x value:%dmV\n", __func__, reg, min_uV);

	ret = i2c_smbus_write_byte_data(tcc270->client, reg, value);
	udelay(50);

	return ret;
}

static int tcc270_dcdc_get_voltage(struct regulator_dev *rdev)
{
	struct tcc270_data* tcc270 = rdev_get_drvdata(rdev);
	int id = rdev_get_id(rdev);
	u8 reg;
	int ret, voltage = 0, max, step, vrc;

	switch (id) {
		case TCC270_ID_DCDC1:
			reg = TCC270_BUCK1_CONTROL_REG;
			max = 2275; step = 25; vrc = 2; //(max v : 2.275v, step : 25mv , vrc : 2bit)
			break;
		case TCC270_ID_DCDC2:
			reg = TCC270_BUCK2_CONTROL_REG;
			max = 3500; step = 25; vrc = 1; //(max v : 3.5v, step : 25mv , vrc : 1bit)
			break;
		case TCC270_ID_DCDC3:
			reg = TCC270_BUCK3_CONTROL_REG;
			max = 3500; step = 50; vrc = 2; //(max v : 3.5v, step : 50mv , vrc : 2bit)
			break;
		default:
			return -1;
	}

	ret = i2c_smbus_read_byte_data(tcc270->client, reg);
	if (ret < 0)
		return -EINVAL;

	voltage = (700 + (ret >> vrc)*step) * 1000;
	
	if ( voltage/1000 > max)
		return -EINVAL;

	dbg("%s: reg:0x%x value:%dmV\n", __func__, reg, voltage/1000);
	return voltage;
}

static struct regulator_ops tcc270_dcdc_ops = {
	.set_voltage = tcc270_dcdc_set_voltage,
	.get_voltage = tcc270_dcdc_get_voltage,
};

static int tcc270_ldo_set_voltage(struct regulator_dev *rdev, int min_uV, int max_uV, unsigned *selector)
{
	struct tcc270_data* tcc270 = rdev_get_drvdata(rdev);
	int id = rdev_get_id(rdev);
	u8 reg, value = 0;
	u8 step = 0, rsv = 0;
	unsigned int i, min, max;

	switch (id) {
		case TCC270_ID_LDO1:
			break;
		case TCC270_ID_LDO2:
			reg = TCC270_LDO2_CONTROL_REG;	break;
		case TCC270_ID_LDO3:
			reg = TCC270_LDO3_CONTROL_REG;	break;
		case TCC270_ID_LDO4:
			reg = TCC270_LDO4_CONTROL_REG;   break;			
		case TCC270_ID_LDO5:
			reg = TCC270_LDO5_CONTROL_REG;   break;						
		case TCC270_ID_LDO6:			
			reg = TCC270_LDO6_CONTROL_REG;	break;		
		case TCC270_ID_LDO7:			
			reg = TCC270_LDO7_CONTROL_REG;	break;	
		case TCC270_ID_LDO8:			
			reg = TCC270_LDO1_8_CONTROL_REG;	 		
			break;
		default:
			return -1;
	}

	switch(id){
		case TCC270_ID_LDO1:
			break;
		case TCC270_ID_LDO2:
		case TCC270_ID_LDO3:
			min = 700; max = 3500; step = 25; rsv = 0;
			break;
		case TCC270_ID_LDO4:
		case TCC270_ID_LDO5:
		case TCC270_ID_LDO6:			
		case TCC270_ID_LDO7:			
		case TCC270_ID_LDO8:			
			min = 1000; max = 3300; step = 100; rsv = 3;
			break;
		default:
			return -1;
	}

	min_uV /= 1000;

	if(id == TCC270_ID_LDO1){
		for (i = 0; i < NUM_LDO1; i++) {
			if (ldo1_voltages[i].uV >= min_uV) {
				value = ldo1_voltages[i].val;
				break;
			}
		}
	}
	else{
		if(min_uV > max || min_uV < min){
			dbg("%s: Wrong ldo value!\n", __func__);
			return -1;
		}
		value = ((min_uV - min) / step) << rsv;
	}
	
	dbg("%s: reg:0x%x value:%dmV\n", __func__, reg, min_uV);
	return i2c_smbus_write_byte_data(tcc270->client, reg, value);
}

static int tcc270_ldo_get_voltage(struct regulator_dev *rdev)
{
	struct tcc270_data* tcc270 = rdev_get_drvdata(rdev);
	int id = rdev_get_id(rdev);
	u8 reg, ret;
	u8 step = 0, rsv = 0;
	int i, min, max, voltage;

	switch (id) {
		case TCC270_ID_LDO1:
			break;
		case TCC270_ID_LDO2:
			reg = TCC270_LDO2_CONTROL_REG;	break;
		case TCC270_ID_LDO3:
			reg = TCC270_LDO3_CONTROL_REG;	break;
		case TCC270_ID_LDO4:
			reg = TCC270_LDO4_CONTROL_REG;   break;			
		case TCC270_ID_LDO5:
			reg = TCC270_LDO5_CONTROL_REG;   break;						
		case TCC270_ID_LDO6:			
			reg = TCC270_LDO6_CONTROL_REG;	break;		
		case TCC270_ID_LDO7:			
			reg = TCC270_LDO7_CONTROL_REG;	break;	
		case TCC270_ID_LDO8:			
			reg = TCC270_LDO1_8_CONTROL_REG;	 		
			break;
		default:
			return -1;
	}

	switch(id){
		case TCC270_ID_LDO1:
			break;
		case TCC270_ID_LDO2:
		case TCC270_ID_LDO3:
			min = 700; max = 3500; step = 25; rsv = 0;
			break;
		case TCC270_ID_LDO4:
		case TCC270_ID_LDO5:
		case TCC270_ID_LDO6:			
		case TCC270_ID_LDO7:			
		case TCC270_ID_LDO8:			
			min = 1000; max = 3300; step = 100; rsv = 3;
			break;
		default:
			return -1;
	}

	ret = i2c_smbus_read_byte_data(tcc270->client, reg);
	if (ret < 0)
		return -EINVAL;

	if(id == TCC270_ID_LDO1){
		for (i = 0; i < NUM_LDO1; i++) {
			if (ldo1_voltages[i].val == ret) {
				voltage = ldo1_voltages[i].uV*1000;
				break;
			}
		}
	}
	else{
		voltage = (min + ((ret >> rsv) * step )) * 1000;
		if ( voltage/1000 > max)
			return -EINVAL;
	}

	dbg("%s: reg:0x%x value: %dmV\n", __func__, reg, voltage/1000);
	return voltage;
}

static int tcc270_ldo_enable(struct regulator_dev *rdev)
{
	struct tcc270_data* tcc270 = rdev_get_drvdata(rdev);
	int id = rdev_get_id(rdev);
	u8 reg, value;

	if(id < TCC270_ID_LDO2 || id > TCC270_ID_LDO8)
		return -EINVAL;
	
	reg = TCC270_LDO_ENABLE_REG;
	value  = (u8)i2c_smbus_read_byte_data(tcc270->client, reg);
	value |= (1<<(id-5));

	dbg("%s: id:%d\n", __func__, id);
	return i2c_smbus_write_byte_data(tcc270->client, reg, value);
}

static int tcc270_ldo_disable(struct regulator_dev *rdev)
{
	struct tcc270_data* tcc270 = rdev_get_drvdata(rdev);
	int id = rdev_get_id(rdev);
	u8 reg, value;

	if(id < TCC270_ID_LDO2 || id > TCC270_ID_LDO8)
		return -EINVAL;
	
	reg = TCC270_LDO_ENABLE_REG;
	value  = (u8)i2c_smbus_read_byte_data(tcc270->client, reg);
	value &= ~(1<<(id-5));

	dbg("%s: id:%d\n", __func__, id);
	return i2c_smbus_write_byte_data(tcc270->client, reg, value);
}

static int tcc270_ldo_is_enabled(struct regulator_dev *rdev)
{
	return 0;
}

static struct regulator_ops tcc270_ldo_ops = {
	.set_voltage = tcc270_ldo_set_voltage,
	.get_voltage = tcc270_ldo_get_voltage,
	.enable	  = tcc270_ldo_enable,
	.disable	 = tcc270_ldo_disable,
	.is_enabled  = tcc270_ldo_is_enabled,
};

static struct regulator_desc tcc270_reg[] = {
	{
		.name = "dcdc1",
		.id = TCC270_ID_DCDC1,
		.ops = &tcc270_dcdc_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},	
	{
		.name = "dcdc2",
		.id = TCC270_ID_DCDC2,
		.ops = &tcc270_dcdc_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "dcdc3",
		.id = TCC270_ID_DCDC3,
		.ops = &tcc270_dcdc_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "ldo1",
		.id = TCC270_ID_LDO1,
		.ops = &tcc270_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.n_voltages = 5,
		.owner = THIS_MODULE,
	},
	{
		.name = "ldo2",
		.id = TCC270_ID_LDO2,
		.ops = &tcc270_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "ldo3",
		.id = TCC270_ID_LDO3,
		.ops = &tcc270_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "ldo4",
		.id = TCC270_ID_LDO4,
		.ops = &tcc270_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "ldo5",
		.id = TCC270_ID_LDO5,
		.ops = &tcc270_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "ldo6",
		.id = TCC270_ID_LDO6,
		.ops = &tcc270_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "ldo7",
		.id = TCC270_ID_LDO7,
		.ops = &tcc270_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "ldo8",
		.id = TCC270_ID_LDO8,
		.ops = &tcc270_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
};
#define NUM_OUPUT	ARRAY_SIZE(tcc270_reg)

static int tcc270_pmic_probe(struct i2c_client *client, const struct i2c_device_id *i2c_id)
{
	struct regulator_dev **rdev;
	struct tcc270_platform_data *pdata = client->dev.platform_data;
	struct tcc270_data *tcc270;
	int i, id, ret = -ENOMEM;
	int i2c_send_count = 0;
	unsigned int tcc270_id;
 
	tcc270 = kzalloc(sizeof(struct tcc270_data) +
			sizeof(struct regulator_dev *) * (NUM_OUPUT + 1),
			GFP_KERNEL);
	if (!tcc270)
		goto out;

	tcc270->client = client;
	tcc270_i2c_client = client;

	tcc270->min_uV = TCC270_MIN_UV;
	tcc270->max_uV = TCC270_MAX_UV;

	rdev = tcc270->rdev;

	i2c_set_clientdata(client, rdev);

	tcc270_id = i2c_smbus_read_byte_data(client, 0x00);
	if ((tcc270_id&0xFF) == 0xFF) {
		ret = -ENODEV;
		goto err_nodev;
	} else {
		printk("######## %s: %x ########\n", __func__, tcc270_id);
	}

	INIT_WORK(&tcc270->work, tcc270_work_func);

	for (i = 0; i < pdata->num_subdevs && i <= NUM_OUPUT; i++) {
		id = pdata->subdevs[i].id;
		if (!pdata->subdevs[i].platform_data) {
			rdev[i] = NULL;
			continue;
		}
		if (id >= TCC270_ID_MAX) {
			dev_err(&client->dev, "invalid regulator id %d\n", id);
			goto err;
		}
		rdev[i] = regulator_register(&tcc270_reg[id], &client->dev,
						 pdata->subdevs[i].platform_data,
						 tcc270,NULL);
		if (IS_ERR(rdev[i])) {
			ret = PTR_ERR(rdev[i]);
			dev_err(&client->dev, "failed to register %s\n",
				tcc270_reg[id].name);
			goto err;
		}
	}


	/* Current and voltage channels need open */
	i2c_smbus_write_byte_data(client, TCC270_COULOMB_CHANNEL_LOW_REG, 0xff);
	i2c_smbus_write_byte_data(client, TCC270_COULOMB_CHANNEL_HIGH_REG, 0xff);
	

	/*Battery charge configuration*/
	//i2c_smbus_write_byte_data(client, TCC270_CHGCONTROL7_REG, 0x50); // BAT_DET when charge done enable
	//i2c_smbus_write_byte_data(client, TCC270_CHGCONTROL4_REG, 0x1C); // charge current 0.8A
	//i2c_smbus_write_byte_data(client, TCC270_CHGCONTROL3_REG, 0x50); // battery 3.7Volt regulation
	i2c_smbus_write_byte_data(client, TCC270_CHGCONTROL2_REG, 0x2);	//charge enable
	/* Start up time set to 2S, long time key press time set to 1S, long press power off time set to 6S */
 	i2c_smbus_write_byte_data(client, TCC270_POK_TIME_SETTING_REG, 0x86); // start up:2s, long pok: 1s, shdn: 6s

	if( (int)(tcc270_acpin_voltage()) > 0)
		tcc270_acin_det = 1;
	else
		tcc270_acin_det = 0;

	if (pdata->init_port) {
		pdata->init_port(client->irq);
	}

	if (client->irq) {
		/* irq enable */
		#if 1
		i2c_smbus_write_byte_data(client, TCC270_IRQ1_ENABLE_REG, TCC270_IRQ1);
		i2c_smbus_write_byte_data(client, TCC270_IRQ2_ENABLE_REG, TCC270_IRQ2);
		i2c_smbus_write_byte_data(client, TCC270_IRQ3_ENABLE_REG, TCC270_IRQ3);
		i2c_smbus_write_byte_data(client, TCC270_IRQ4_ENABLE_REG, TCC270_IRQ4);
		i2c_smbus_write_byte_data(client, TCC270_IRQ5_ENABLE_REG, TCC270_IRQ5);
		#else // temporary all enable
		i2c_smbus_write_byte_data(client, TCC270_IRQ1_ENABLE_REG, 0xff);
		i2c_smbus_write_byte_data(client, TCC270_IRQ2_ENABLE_REG, 0xff);
		i2c_smbus_write_byte_data(client, TCC270_IRQ3_ENABLE_REG, 0xff);
		i2c_smbus_write_byte_data(client, TCC270_IRQ4_ENABLE_REG, 0xff);
		i2c_smbus_write_byte_data(client, TCC270_IRQ5_ENABLE_REG, 0xff);
		#endif

		if (request_irq(client->irq, tcc270_interrupt, IRQ_TYPE_EDGE_FALLING|IRQF_DISABLED, "tcc270_irq", tcc270)) {
			dev_err(&client->dev, "could not allocate IRQ_NO(%d) !\n", client->irq);
			ret = -EBUSY;
			goto err;
		}
	}

	while(i2c_smbus_read_byte_data(tcc270->client, TCC270_IRQ1_STATUS_REG) != 0)
	{
		i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ1_STATUS_REG, 0xFF);
		if(i2c_send_count++ > 10)
			break;
	}
	i2c_send_count = 0;
	while(i2c_smbus_read_byte_data(tcc270->client, TCC270_IRQ2_STATUS_REG) != 0)
	{
		i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ2_STATUS_REG, 0xFF);
		if(i2c_send_count++ > 10)
			break;
	}
	i2c_send_count = 0;
	while(i2c_smbus_read_byte_data(tcc270->client, TCC270_IRQ3_STATUS_REG) != 0)
	{
		i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ3_STATUS_REG, 0xFF);
		if(i2c_send_count++ > 10)
			break;
	}
	i2c_send_count = 0;
	while(i2c_smbus_read_byte_data(tcc270->client, TCC270_IRQ4_STATUS_REG) != 0)
	{
		i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ4_STATUS_REG, 0xFF);
		if(i2c_send_count++ > 10)
			break;
	}
	i2c_send_count = 0;
	while(i2c_smbus_read_byte_data(tcc270->client, TCC270_IRQ5_STATUS_REG) != 0)
	{
		i2c_smbus_write_byte_data(tcc270->client, TCC270_IRQ5_STATUS_REG, 0xFF);
		if(i2c_send_count++ > 10)
			break;
	}
	i2c_send_count = 0;
		
#if defined(CONFIG_REGULATOR_TCC270_PEK)
	// register input device for power key.
	tcc270->input_dev = input_allocate_device();
	if (tcc270->input_dev == NULL) {
		ret = -ENOMEM;
		dev_err(&client->dev, "%s: Failed to allocate input device\n", __func__);
		goto err_input_dev_alloc_failed;
	}

	tcc270->input_dev->evbit[0] = BIT(EV_KEY);
	tcc270->input_dev->name = "tcc270 power-key";
	set_bit(KEY_POWER & KEY_MAX, tcc270->input_dev->keybit);
	ret = input_register_device(tcc270->input_dev);
	if (ret) {
		dev_err(&client->dev, "%s: Unable to register %s input device\n", __func__, tcc270->input_dev->name);
		goto err_input_register_device_failed;
	}

	init_timer(&tcc270->timer);
	tcc270->timer.data = (unsigned long)tcc270;
	tcc270->timer.function = tcc270_timer_func;
#endif

#if defined(CONFIG_BATTERY_ANDROID)
	if(charger_tcc != NULL)
		charger_tcc->master = client;
	//tcc_charger_init(charger_tcc);
#endif

	dev_info(&client->dev, "TCC270 regulator driver loaded\n");
	return 0;

#if defined(CONFIG_REGULATOR_TCC270_PEK)
err_input_register_device_failed:
	input_free_device(tcc270->input_dev);
err_input_dev_alloc_failed:
#endif
err:
	while (--i >= 0)
		regulator_unregister(rdev[i]);
err_nodev:
	kfree(tcc270);
out:
	tcc270_i2c_client = NULL;
	return ret;
}

static int tcc270_pmic_remove(struct i2c_client *client)
{
	struct regulator_dev **rdev = i2c_get_clientdata(client);
	struct tcc270_data* tcc270 = NULL;
	int i;

	for (i=0 ; (rdev[i] != NULL) && (i<TCC270_ID_MAX) ; i++)
		tcc270 = rdev_get_drvdata(rdev[i]);

#if defined(CONFIG_REGULATOR_TCC270_PEK)
	if (tcc270)
		del_timer(&tcc270->timer);
#endif

	for (i = 0; i <= NUM_OUPUT; i++)
		if (rdev[i])
			regulator_unregister(rdev[i]);
	kfree(rdev);
	i2c_set_clientdata(client, NULL);
	tcc270_i2c_client = NULL;

	return 0;
}

static int tcc270_pmic_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int i, ret = 0;
	struct regulator_dev **rdev = i2c_get_clientdata(client);
	struct tcc270_data* tcc270 = NULL;

	for (i=0 ; (rdev[i] != NULL) && (i<TCC270_ID_MAX) ; i++)
		tcc270 = rdev_get_drvdata(rdev[i]);

	if (client->irq)
		disable_irq(client->irq);

	/* clear irq status */
	i2c_smbus_write_byte_data(client, TCC270_IRQ1_STATUS_REG, 0xFF);
	i2c_smbus_write_byte_data(client, TCC270_IRQ2_STATUS_REG, 0xFF);
	i2c_smbus_write_byte_data(client, TCC270_IRQ3_STATUS_REG, 0xFF);
	i2c_smbus_write_byte_data(client, TCC270_IRQ4_STATUS_REG, 0xFF);
	i2c_smbus_write_byte_data(client, TCC270_IRQ5_STATUS_REG, 0xFF);

	if (tcc270) {
		ret = cancel_work_sync(&tcc270->work);
		flush_workqueue(tcc270_wq);
	}

	if (ret && client->irq)
		enable_irq(client->irq);

	tcc270_suspend_status = 1;

	return 0;
}

static int tcc270_pmic_resume(struct i2c_client *client)
{
	int i;
	struct regulator_dev **rdev = i2c_get_clientdata(client);
	struct tcc270_data* tcc270 = NULL;

	for (i=0 ; (rdev[i] != NULL) && (i<TCC270_ID_MAX) ; i++)
		tcc270 = rdev_get_drvdata(rdev[i]);

	#if defined(CONFIG_REGULATOR_TCC270_PEK)
	if (tcc270)
		queue_work(tcc270_wq, &tcc270->work);
	#else
	if (client->irq)
		enable_irq(client->irq);

	/* clear irq status */
	i2c_smbus_write_byte_data(client, TCC270_IRQ1_STATUS_REG, 0xFF);
	i2c_smbus_write_byte_data(client, TCC270_IRQ2_STATUS_REG, 0xFF);
	i2c_smbus_write_byte_data(client, TCC270_IRQ3_STATUS_REG, 0xFF);
	i2c_smbus_write_byte_data(client, TCC270_IRQ4_STATUS_REG, 0xFF);
	i2c_smbus_write_byte_data(client, TCC270_IRQ5_STATUS_REG, 0xFF);
	#endif

	tcc270_suspend_status = 0;
	return 0;
}

static const struct i2c_device_id tcc270_id[] = {
	{ "tcc270", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, tcc270_id);

static struct i2c_driver tcc270_pmic_driver = {
	.probe	  = tcc270_pmic_probe,
	.remove	 = tcc270_pmic_remove,
	.suspend	= tcc270_pmic_suspend,
	.resume	 = tcc270_pmic_resume,
	.driver	 = {
		.name   = "tcc270",
	},
	.id_table   = tcc270_id,
};

static int __init tcc270_pmic_init(void)
{
	tcc270_wq = create_singlethread_workqueue("tcc270_wq");
	if (!tcc270_wq)
		return -ENOMEM;

	return i2c_add_driver(&tcc270_pmic_driver);
}
subsys_initcall(tcc270_pmic_init);

static void __exit tcc270_pmic_exit(void)
{
	i2c_del_driver(&tcc270_pmic_driver);
	if (tcc270_wq)
		destroy_workqueue(tcc270_wq);}
module_exit(tcc270_pmic_exit);

/* Module information */
MODULE_DESCRIPTION("TCC270 regulator driver");
MODULE_AUTHOR("Telechips");
MODULE_LICENSE("GPL");

