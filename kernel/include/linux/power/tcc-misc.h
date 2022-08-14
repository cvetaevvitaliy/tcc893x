/*
 * pmic temperature daemon
 * -- rocky
 */
 #ifndef _TCC_MISC_H_
 #define _TCC_MISC_H_

#include <linux/types.h>
#include <linux/device.h>

 typedef struct {
	int temp;
	int impedance;
} ts_map_t;

typedef enum {
	PMIC_STATE_NORMAL = 0,
	PMIC_STATE_UNDER_TEMP,
	PMIC_STATE_CHARGER_DISABLE,
	PMIC_STATE_CHARGER_ENABLE,
	PMIC_STATE_PWROFF,
} pmic_state_t;

#define TS_ALERT_EN		0	// TS temperature alert function enable
#define INTEMP_MONITOR_EN	1	// pmic internal temperature monitor enable

#define PMIC_DELAY_PWROFF		0
#define PMIC_PWROFF_DELAY		10000	// ms

#define TS_CURRENT	80	// uA
/*
 * ADC LSB, datasheet is 0.8 mV
 * voltage = reg * TS_ADC_LSB_M / TS_ADC_LSB_D mV
 */
#define TS_ADC_LSB_M	10
#define TS_ADC_LSB_D	10

#define TS_UNDER_TEMP		-10
#define TS_CHG_OVER_TEMP_SCHMITT_H	85
#define TS_CHG_OVER_TEMP_SCHMITT_L	40
#define TS_TEMP_PWROFF	90

/* temperature test threshhold history
 * 63 60 73
 * 63 60 73
 * 63 58 68
 */
#define INTEMP_CHG_OVER_TEMP_SCHMITT_H	63
#define INTEMP_CHG_OVER_TEMP_SCHMITT_L	58
#define INTEMP_TEMP_PWROFF	68

s32 pmic_read(u8 reg);
s32 pmic_write(u8 reg, u8 value);
void pmic_power_off(void);
void charger_enable(int en);
int charger_is_enable(void);
int intemp_adc(void);
int intemp_temp(void);
int ts_adc(void);
int ts_adc_to_temp(int adc);
int ts_temp_to_adc(int temp);
int ts_voltage(void);
int ts_temp(void);
void intemp_threshhold_info(void);
void ts_threshhold_info(void);
void ts_alert_normal(void);
void ts_alert_charger_disable(void);
void ts_alert_charger_disable(void);
void ts_daemon_call(void);
void pmic_daemon_call(void);
void pmic_debug_create_attrs(struct device *dev);
void tcc_misc_init(void);

//extern struct tcc_charger *tcccharger;
extern pmic_state_t pmic_state;
extern pmic_state_t pmic_state_next;
extern int ts_alert_type;
extern int emulate_enable;
extern int emulate_ts_temp;
extern int emulate_int_temp;


#endif
