#ifndef	_LINUX_TCC_SPLY_H_
#define	_LINUX_TCC_SPLY_H_

#include <linux/mutex.h>

/* PMIC CHARGER CONTROL part */
#define TCC270_CHGCTL1_REG					0x01
#define TCC270_CHGCTL2_REG					0x02
#define TCC270_CHGCTL3_REG					0x03
#define TCC270_CHGCTL4_REG					0x04
#define TCC270_CHGCTL5_REG					0x05
#define TCC270_CHGCTL6_REG					0x06
#define TCC270_CHGCTL7_REG					0x07
/* IRQ PMIC Charger part */
#define TCC270_IRQ1ENABLE_REG				0x30
#define TCC270_IRQ1STATUS_REG				0x31
#define TCC270_IRQ2ENABLE_REG				0x32
#define TCC270_IRQ2STATUS_REG				0x33
#define TCC270_IRQ3ENABLE_REG				0x34
#define TCC270_IRQ3STATUS_REG				0x35

#define TCC270_IRQ1_STATUS_REG				0x31
#define TCC270_VBATS_HIGH_REG				0x58
#define TCC270_VBATS_LOW_REG				0x59
#define TCC270_IRQ1_ACIN_CONNECTED			0x40
#define TCC270_REG_VCELL_MSB				0x58
#define TCC270_REG_CURRENT_MSB				0x76
#define TCC270_CHGCONTROL2_REG				0x02
#define TCC270_CHGCONTROL4_REG				0x04

#define TCC270_REG_IRQ_CTL					0x50
#define TCC270_REG_IRQ_FLAG 				0x51
#define TCC270_REG_VALRT_MAXTH				0x53
#define TCC270_REG_VALRT_MIN1TH 			0x54
#define TCC270_REG_VALRT_MIN2TH 			0x55
#define TCC270_REG_TALRT_MAXTH				0x56
#define TCC270_REG_TALRT_MINTH				0x57
#define TCC270_REG_VCELL_MSB				0x58
#define TCC270_REG_VCELL_LSB				0x59
#define TCC270_REG_INT_TEMPERATUE_MSB		0x5B
#define TCC270_REG_INT_TEMPERATUE_LSB		0x5C
#define TCC270_REG_EXT_TEMPERATUE_MSB		0x5E
#define TCC270_REG_EXT_TEMPERATUE_LSB		0x5F
#define TCC270_REG_TIMER					0x60
#define TCC270_REG_CHANNEL_MSB				0x62
#define TCC270_REG_CHANNEL_LSB				0x63
#define TCC270_REG_CURRENT_MSB				0x76
#define TCC270_REG_CURRENT_LSB				0x77
#define TCC270_REG_QCHGH_MSB				0x78
#define TCC270_REG_QCHGH_LSB				0x79
#define TCC270_REG_QCHGL_MSB				0x7A
#define TCC270_REG_QCHGL_LSB				0x7B
#define TCC270_REG_QDCHGH_MSB				0x7C
#define TCC270_REG_QDCHGH_LSB				0x7D
#define TCC270_REG_QDCHGL_MSB				0x7E
#define TCC270_REG_QDCHGL_LSB				0x7F

#define OCVREG0 			0x00		//3.1328
#define OCVREG1 			0x01		//3.2736
#define OCVREG2 			0x02		//3.4144
#define OCVREG3 			0x05		//3.5552
#define OCVREG4 			0x07		//3.6256
#define OCVREG5 			0x0D		//3.6608
#define OCVREG6 			0x10		//3.6960
#define OCVREG7 			0x1A		//3.7312
#define OCVREG8 			0x24		//3.7664
#define OCVREG9 			0x2E		//3.8016
#define OCVREGA 			0x35		//3.8368
#define OCVREGB 			0x3D		//3.8720
#define OCVREGC 			0x49		//3.9424
#define OCVREGD 			0x54		//4.0128
#define OCVREGE 			0x5C		//4.0832
#define OCVREGF 			0x64		//4.1536

#define OCVVOL0 			3132
#define OCVVOL1 			3273
#define OCVVOL2 			3414
#define OCVVOL3 			3555
#define OCVVOL4 			3625
#define OCVVOL5 			3660
#define OCVVOL6 			3696
#define OCVVOL7 			3731
#define OCVVOL8 			3766
#define OCVVOL9 			3801
#define OCVVOLA 			3836
#define OCVVOLB 			3872
#define OCVVOLC 			3942
#define OCVVOLD 			4012
#define OCVVOLE 			4083
#define OCVVOLF 			4153

#define IRQ_CTL_BIT_TMX  (1 << 5)
#define IRQ_CTL_BIT_TMN  (1 << 4)
#define IRQ_CTL_BIT_VMX  (1 << 2)
#define IRQ_CTL_BIT_VMN1 (1 << 1)
#define IRQ_CTL_BIT_VMN2 (1 << 0)

#define IRQ_FLG_BIT_TMX  (1 << 5)
#define IRQ_FLG_BIT_TMN  (1 << 4)
#define IRQ_FLG_BIT_VMX  (1 << 2)
#define IRQ_FLG_BIT_VMN1 (1 << 1)
#define IRQ_FLG_BIT_VMN2 (1 << 0)

#define CHANNEL_H_BIT_CLRQDCHG  (1 << 7)
#define CHANNEL_H_BIT_CLRQCHG   (1 << 6)

#define CHANNEL_L_BIT_CADC_EN   (1 << 7)
#define CHANNEL_L_BIT_INTEMPCH  (1 << 6)
#define CHANNEL_L_BIT_AINCH     (1 << 2)
#define CHANNEL_L_BIT_VBATSCH   (1 << 1)
#define CHANNEL_L_BIT_VADC_EN   (1 << 0)

#define NORMAL_POLL 10  /* 10 sec */
#define SUSPEND_POLL (30*60) /* 30 min */

#define HIGH_TEMP_THRES	650
#define HIGH_TEMP_RECOVER	430
#define LOW_TEMP_THRES (-30)
#define LOW_TEMP_RECOVER 0
#define TEMP_ABNORMAL_COUNT	3

#define TALRTMAX_VALUE  0x38 //65.39'C 0x9
#define TALRTMIN_VALUE  0x11 //-18.75'C 0x17
#define VALRTMAX_VALUE  0xDC //4297mV
#define VALRTMIN1_VALUE 0xB8 //3600mV
#define VALRTMIN2_VALUE 0x99 //3000mV
#define TRLS_VALUE      55   //5'C
#define VRLS_VALUE      100  //100mV
#define BATTSAMPLE      16
#define IRQ_THRES_UNIT 1953
#define GPIO_GAUGE_ALERT 4

#define TIMER1				5	
#define TIMER2				3
#define TIMER3				2
#define TIMER4				4
#define TIMER5				3
#define TIMER6				1
#define BATCAPCORRATE		10
#define SHUTDOWNRATE		0
#define SHUTDOWNVOL			3500

#define ABS(x)				((x) >0 ? (x) : -(x) )
#define TCC270_VOL_MAX  12

#define TCC_CHG_ATTR(_name)					\
{											\
	.attr = { .name = #_name,.mode = 0664 },\
	.show =  _name##_show,					\
	.store = _name##_store,					\
}

static int Total_Cap = 0;
static int Cap_Index = 0;
static int cap_count = 0;
static int Bat_Cap_Buffer[12];
static int state_count = 0;
static int base_cap = 0;
static int bat_cap = 0;
static int startcorcap = 0;
static int cap_count1 = 0,cap_count2 = 0,cap_count3 = 0,cap_count4 = 0;

typedef enum{
	CHG,
	DCHG
}operation_mode;

typedef enum{
	MAXTEMP,
	MINTEMP,
	MAXVOLT,
	MINVOLT1,
	MINVOLT2,
	TEMP_RLS,
	VOLT_RLS,
	LAST_TYPE
}alert_type;


struct tcc_adc_res {//struct change
	uint16_t vbat_res;
	uint16_t ibat_res;
	uint16_t ichar_res;
	uint16_t idischar_res;
	uint16_t vac_res;
	uint16_t iac_res;
	uint16_t vusb_res;
	uint16_t iusb_res;
};

struct tcc_charger {
	/*i2c device*/
	struct i2c_client *master;
  
  	struct delayed_work work;
  	unsigned interval;
  	struct power_supply_info *battery_info;

	u8 temp_high_cnt;
	u8 temp_low_cnt;
	u8 temp_recover_cnt;

	struct mutex mutex;

	/* charger status */
	bool bat_det;
	bool is_on;
	bool is_finish;
	bool ac_not_enough;
	bool ac_det;
	bool usb_det;
	bool ac_valid;
	bool usb_valid;
	bool ext_valid;
	bool bat_current_direction;
	bool in_short;
	bool batery_active;
	bool low_charge_current;
	bool int_over_temp;
	bool charge_on;
	bool suspend_poll;

	/* max voltage IRQ flag */
  	bool max_volt_irq;
  	/* min voltage1 IRQ flag */
  	bool min_volt1_irq;  
  	/* min voltage2 IRQ flag */
  	bool min_volt2_irq;
  	/* max temperature IRQ flag */
  	bool max_temp_irq;
  	/* min temperature IRQ flag */
  	bool min_temp_irq;
	
	int vbat;
	int pbat;
	int vac;
	int vusb;
	int ocv;
	
	/*rest time*/
	int rest_cap;
	int ocv_rest_cap;
	int base_restcap;
	int rest_time;

	int suspend_status;
	int ac_online;
	int usb_online;
	int batt_present;
	int charge_status;

	ktime_t	last_poll;
  	/* battery voltage */
  	u16 vcell;
  	/* battery current */
  	s16 curr;
  	/* battery current offset */
  	u8 curr_offset;
  	/* AIN voltage */
  	u16 ain_volt;
  	/* battery external temperature */
  	s16 ext_temp;
  	/* charge coulomb counter */
  	u32 chg_cc;
  	u32 chg_cc_unuse;
  	/* discharge coulomb counter */
  	u32 dchg_cc;
  	u32 dchg_cc_unuse;
  	/* battery capacity */
  	u8 soc;
  	u16 time_interval;
  	u16 pre_gauge_timer;
    
  	u8 online;
  	u8 status;
  	u8 health;

  	/* IRQ flag */
  	u8 irq_flag;
  
};

typedef struct {
	unsigned char vbus_usable:1;//bit 0
	unsigned char acin_usable:1;//bit 1
	unsigned char reserved2:2;  //bit 3-2
	unsigned char chgstat:2;    //bit 5-4
	unsigned char reserved1:2;  //bit 7-6
}chgctl1_type;

typedef struct {
	unsigned char hz:1;          //bit 0
	unsigned char chgbc_en:1;    //bit 1
	unsigned char te:1;          //bit 2
	unsigned char reserved:1;    //bit 3
	unsigned char ccchg_to:2;    //bit 5-4
	unsigned char prechg_to:2;   //bit 7-6
}chgctl2_type;

typedef struct {
	unsigned char supercap:1;    //bit 0
	unsigned char reserved:1;    //bit 1
	unsigned char voreg:6;       //bit 7-2
}chgctl3_type;

typedef struct {
	unsigned char reserved:1;    //bit 0
	unsigned char aicr:2;        //bit 2-1
	unsigned char icc:4;         //bit 6-3
	unsigned char chg_rst:1;     //bit 7
}chgctl4_type;

typedef struct {
	unsigned char reserved2:4;   //bit 3-0
	unsigned char dpm:2;         //bit 5-4
	unsigned char reserved1:2;   //bit 7-6
}chgctl5_type;

typedef struct {
	unsigned char iprec:1;       //bit 0
	unsigned char ieoc:1;        //bit 1
	unsigned char vprec:3;       //bit 4-2
	unsigned char reserved:3;    //bit 7-5
}chgctl6_type;

typedef struct {
	unsigned char reserved2:4;    //bit 3-0
	unsigned char chgc_en:1;      //bit 4
	unsigned char chg_buck_mode:1;//bit 5
	unsigned char batd_en:1;      //bit 6
	unsigned char reserved1:1;    //bit 7
}chgctl7_type;

typedef struct {
	unsigned char bat_absense_irq:1;
	unsigned char bat_present_irq:1;
	unsigned char vbusout_irq:1;
	unsigned char vbusin_irq:1;
	unsigned char vbusovp_irq:1;
	unsigned char acout_irq:1;
	unsigned char acin_irq:1;
	unsigned char acinovp_irq:1;
}irqenable1_type;

typedef struct {
	unsigned char chtermi_irq:1;
	unsigned char chbatovi_irq:1;
	unsigned char chgoodi_vbus_irq:1;
	unsigned char chbadi_vbus_irq:1;
	unsigned char chslpi_vbus_irq:1;
	unsigned char chgoodi_acin_irq:1;
	unsigned char chbadi_acin_irq:1;
	unsigned char chslpi_acin_irq:1;
}irqenable2_type;

typedef struct {
	unsigned char to_ccirq:1;
	unsigned char to_pcirq:1;
	unsigned char appm_irq:1;
	unsigned char chfchg_irq:1;
	unsigned char chrdy_irq:1;
	unsigned char chvsregi_irq:1;
	unsigned char chtregi_irq:1;
	unsigned char chrchgi_irq:1;
}irqenable3_type;

struct chgctl_type{
	chgctl1_type chgctl1;
	chgctl2_type chgctl2;
	chgctl3_type chgctl3;
	chgctl4_type chgctl4;
	chgctl5_type chgctl5;
	chgctl6_type chgctl6;
	chgctl7_type chgctl7;
	irqenable1_type chgirqenable1;
	unsigned char chgirqstatus1;
	irqenable2_type chgirqenable2;
	unsigned char chgirqstatus2;
	irqenable3_type chgirqenable3;
	unsigned char chgirqstatus3;
	int charge_current;
	int charge_current_max;
	int charge_current_min;
	int charge_current_step;
	int (*get_config)(int option); //Option:true -> get from reg; Option:false -> print the current config
	int (*set_config)(void);  // Directly set from the configuration
	int (*set_charge_current)(int val);
	int (*get_charge_current)(void);
};

static struct chgctl_type tcc270_chargectl= {
	.chgctl2 = {
		.prechg_to = 0,
		.ccchg_to = 0x01,
		.reserved = 0,
		.te = 0x1,
		.chgbc_en = 1,
		.hz = 0,
	},
	.chgctl3 = {
		.voreg = 0x23,
		.reserved = 0,
		.supercap = 0,
	},
	.chgctl4 = {
		.chg_rst = 0,
		.icc = 0,
		.aicr = 0x02,
		.reserved = 0,
	},
	.chgctl5 = {
		.reserved1 = 0,
		.dpm = 0x03,
		.reserved2 = 0,
	},
	.chgctl6 = {
		.reserved = 0,
		.vprec = 0x05,
		.ieoc = 0,
		.iprec = 0,
	},
	.chgctl7 = {
		.reserved1 = 0,
		.batd_en = 0,
		.chg_buck_mode = 0,
		.chgc_en = 0x01,
		.reserved2 = 0,
	},
	.chgirqenable1 = {
		.acinovp_irq = 1,
		.acin_irq = 1,
		.acout_irq = 1,
		.vbusovp_irq = 1,
		.vbusin_irq = 1,
		.vbusout_irq = 1,
		.bat_present_irq = 1,
		.bat_absense_irq = 1,
	},
	.chgirqstatus1 = 0,
	.chgirqenable2 = {
		.chslpi_acin_irq = 1,
		.chbadi_acin_irq = 1,
		.chgoodi_acin_irq = 0,
		.chslpi_vbus_irq = 1,
		.chbadi_vbus_irq = 1,
		.chgoodi_vbus_irq = 0,
		.chbatovi_irq = 1,
		.chtermi_irq = 1,
	},
	.chgirqstatus2 = 0,
	.chgirqenable3 = {
		.chrchgi_irq = 0,
		.chtregi_irq = 1,
		.chvsregi_irq = 1,
		.chrdy_irq = 0,
		.chfchg_irq = 0,
		.appm_irq = 1,
		.to_pcirq = 1,
		.to_ccirq = 1,
	},
	.chgirqstatus3 = 0,
	.charge_current = 500,
	.charge_current_max = 2000,
	.charge_current_min = 500,
	.charge_current_step = 100,
};

#endif
