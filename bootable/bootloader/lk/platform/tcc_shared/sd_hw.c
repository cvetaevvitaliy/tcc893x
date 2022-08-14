#include <platform/globals.h>

#include <platform/structures_smu_pmu.h>
#include <platform/reg_physical.h>
#include <config.h>
#include <dev/gpio.h>
#include <platform/tcc_ckc.h>
#include <platform/gpio.h>
#include <sdmmc/sd_hw.h>
#include <debug.h>
#include <platform/iomap.h>

#ifndef NULL
#define NULL 0
#endif
#ifndef dim
#define dim(x) (sizeof(x)/sizeof(x[0]))
#endif
#define SD_ASSERT(x)				if(!(x)) while(1);
#define	_tca_delay()				{ volatile int i; for (i=0; i<1000; i++); }


#if defined(SD_UPDATE_INCLUDE)
#define SLOT_TYPE_1				SLOT_ATTR_UPDATE
#define SLOT_BUSWIDTH_1			4
#define SLOT_MAX_SPEED_1		SLOT_MAX_SPEED_NORMAL

	#if defined(CONFIG_MB_TCC8803_2CS_V60)
	#define FEATURE_TCC8803F_2CS_V6_JM5_SLOT			//V6.0
	#define CARD_DETECT_FUNC_1		CardDetectPortFor_2CS_JM5
	#define WRITE_PROTECT_FUNC_1	WriteProtectPort_2CS_JM5
	#else
	#define FEATURE_TCC93_88_8910_DEMO_JM5_SLOT			//V5.0, V6.0
	#define CARD_DETECT_FUNC_1		CardDetectPortForJM5
	#define WRITE_PROTECT_FUNC_1	WriteProtectPortJM5
	#endif
#endif

#if defined(TCC892X)
	#if defined(BOARD_TCC892X_STB_DEMO)
		#if defined(TARGET_TCC8925_STB_DONGLE) || defined(TARGET_TCC8925_HDB892F)
		#define SLOT_TYPE_0				SLOT_ATTR_BOOT
		#define SLOT_BUSWIDTH_0			4
		#define SLOT_MAX_SPEED_0		SLOT_MAX_SPEED_NORMAL

		#define FEATURE_TCC93_88_8910_DEMO_JM2_SLOT			//V5.0, V6.0
		#define CARD_DETECT_FUNC_0		CardDetectPortForJM2
		#define WRITE_PROTECT_FUNC_0	WriteProtectPortJM2
		#else
		#define SLOT_TYPE_1				SLOT_ATTR_BOOT
		#define SLOT_BUSWIDTH_1			4
		#define SLOT_MAX_SPEED_1		SLOT_MAX_SPEED_NORMAL

		#define FEATURE_TCC93_88_8910_DEMO_JM2_SLOT			//V5.0, V6.0
		#define CARD_DETECT_FUNC_1		CardDetectPortForJM2
		#define WRITE_PROTECT_FUNC_1	WriteProtectPortJM2
		#endif /* TARGET_TCC8925_STB_DONGLE */
	#else
		#if (HW_REV >= 0x1005)	//||(HW_REV == 0x1006)||(HW_REV == 0x1007)||(HW_REV == 0x1008)
		#define SLOT_TYPE_0				SLOT_ATTR_BOOT
		#define SLOT_BUSWIDTH_0			4
		#define SLOT_MAX_SPEED_0		SLOT_MAX_SPEED_NORMAL

		#define FEATURE_TCC93_88_8910_DEMO_JM2_SLOT			//V5.0, V6.0
		#define CARD_DETECT_FUNC_0		CardDetectPortForJM2
		#define WRITE_PROTECT_FUNC_0	WriteProtectPortJM2
		#else	//(HW_REV >= 0x1000)&&(HW_REV <= 0x1004)
		#define SLOT_TYPE_1				SLOT_ATTR_BOOT
		#define SLOT_BUSWIDTH_1			4
		#define SLOT_MAX_SPEED_1		SLOT_MAX_SPEED_NORMAL

		#define FEATURE_TCC93_88_8910_DEMO_JM2_SLOT			//V5.0, V6.0
		#define CARD_DETECT_FUNC_1		CardDetectPortForJM2
		#define WRITE_PROTECT_FUNC_1	WriteProtectPortJM2
		#endif
	#endif
#elif defined(TCC893X)
	#if defined(BOARD_TCC893X_STB_DEMO)
		#if defined(TARGET_TCC8930_EV) || defined(TARGET_TCC8935_DONGLE) || defined(TARGET_TCC8935_YJ8935T) || defined(TARGET_TCC8930_YJ8930T) ||defined(TARGET_TCC8933_YJ8933T)
			#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
			#define SLOT_TYPE_0				SLOT_ATTR_BOOT
			#if HW_REV==0x9313
			#define SLOT_BUSWIDTH_0                 8
			#define SLOT_MAX_SPEED_0                SLOT_MAX_SPEED_DDR
			#else
			#define SLOT_BUSWIDTH_0			4
			#define SLOT_MAX_SPEED_0		SLOT_MAX_SPEED_NORMAL
			#endif

			#define FEATURE_TCC93_88_8910_DEMO_JM2_SLOT			//V5.0, V6.0
			#define CARD_DETECT_FUNC_0		CardDetectPortForJM2
			#define WRITE_PROTECT_FUNC_0	WriteProtectPortJM2
			#else
			#define SLOT_TYPE_3				SLOT_ATTR_BOOT

			#if HW_REV==0x9312
			#define SLOT_BUSWIDTH_3			8
			#define SLOT_MAX_SPEED_3		SLOT_MAX_SPEED_DDR
			#else
			#define SLOT_BUSWIDTH_3			4
			#define SLOT_MAX_SPEED_3		SLOT_MAX_SPEED_NORMAL
			#endif

			#define FEATURE_TCC93_88_8910_DEMO_JM2_SLOT			//V5.0, V6.0
			#define CARD_DETECT_FUNC_3		CardDetectPortForJM2
			#define WRITE_PROTECT_FUNC_3	WriteProtectPortJM2
			#endif
		#else
		#define SLOT_TYPE_1				SLOT_ATTR_BOOT
		#define SLOT_BUSWIDTH_1			4
		#define SLOT_MAX_SPEED_1		SLOT_MAX_SPEED_NORMAL

		#define FEATURE_TCC93_88_8910_DEMO_JM2_SLOT			//V5.0, V6.0
		#define CARD_DETECT_FUNC_1		CardDetectPortForJM2
		#define WRITE_PROTECT_FUNC_1	WriteProtectPortJM2
		#endif
	#else	/* MID */
  	#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
    	#define SLOT_TYPE_0				SLOT_ATTR_BOOT
			#define SLOT_BUSWIDTH_0			4
			#define SLOT_MAX_SPEED_0		SLOT_MAX_SPEED_NORMAL

			#define FEATURE_TCC93_88_8910_DEMO_JM2_SLOT			//V5.0, V6.0
			#define CARD_DETECT_FUNC_0		CardDetectPortForJM2
			#define WRITE_PROTECT_FUNC_0	WriteProtectPortJM2
		#else
			#define SLOT_TYPE_3				SLOT_ATTR_BOOT
			#define SLOT_BUSWIDTH_3			4
			#define SLOT_MAX_SPEED_3		SLOT_MAX_SPEED_NORMAL

			#define FEATURE_TCC93_88_8910_DEMO_JM2_SLOT			//V5.0, V6.0
			#define CARD_DETECT_FUNC_3		CardDetectPortForJM2
			#define WRITE_PROTECT_FUNC_3	WriteProtectPortJM2
		#endif
	#endif
#endif	//#if defined(BOARD_TCC880X_STB_DEMO)

///////////////////////////////////////////////////////////////////////////////
#if defined(FEATURE_TCC93_88_8910_DEMO_JM5_SLOT)
static int CardDetectPortForJM5(void)
{
	PGPIO pGPIO = (PGPIO)(HwGPIO_BASE);
	return (pGPIO->GPFDAT&Hw13);
}
static int WriteProtectPortJM5(void)
{
	PGPIO pGPIO = (PGPIO)(HwGPIO_BASE);
	return (pGPIO->GPFDAT&Hw12);
}
#endif
#if defined(FEATURE_TCC93_88_8910_DEMO_JM2_SLOT)
static int CardDetectPortForJM2(void)
{
#if defined(TCC892X)	//for TCC892x
	#if defined(TARGET_M805_892X_EVM) || defined(BOARD_TCC892X_STB_DEMO)
		return 0;	// 0: inserted,  1: removed
	#else
		#if (HW_REV == 0x1005)||(HW_REV == 0x1007)
		return (gpio_get(TCC_GPB(14)));
		#elif (HW_REV == 0x1006)
		return (gpio_get(TCC_GPE(26)));
		#elif (HW_REV == 0x1008)
		return (gpio_get(TCC_GPE(1)));
		#else	//(HW_REV >= 0x1000)&&(HW_REV <= 0x1004)
		return (gpio_get(TCC_GPD(12)));
		#endif
	#endif
#elif defined(TCC893X)
	#if defined(BOARD_TCC893X_STB_DEMO)
		return 0;
	#else
		#if (HW_REV == 0x2000 || HW_REV == 0x3000)
		return (gpio_get(TCC_GPE(26)));
		#else
		return (gpio_get(TCC_GPB(14)));
		#endif
	#endif
#else	//for TCC880x

	PGPIO pGPIO = (PGPIO)(HwGPIO_BASE);

#if (HW_REV == 0x0614 || HW_REV == 0x0615 || HW_REV == 0x0621 || HW_REV == 0x0622 || HW_REV == 0x0623 || HW_REV == 0x0624)
	return (pGPIO->GPADAT&Hw13);
#else
	return (pGPIO->GPFDAT&Hw10);
#endif
#endif
}
static int WriteProtectPortJM2(void)
{
#if defined(TCC892X)	//for TCC892x
	#if defined(TARGET_M805_892X_EVM) || defined(BOARD_TCC892X_STB_DEMO)
		return 0;	// 0: Card is not write protected,  1: Card is write protected
	#else
		#if (HW_REV == 0x1005)||(HW_REV == 0x1007)||(HW_REV == 0x1008)
		return (gpio_get(TCC_GPD(10)));
		#elif (HW_REV == 0x1006)
		return (gpio_get(TCC_GPD(21)));
		#else	//(HW_REV >= 0x1000)&&(HW_REV <= 0x1004)
		return (gpio_get(TCC_GPF(29)));
		#endif
	#endif
#elif defined(TCC893X)
	#if defined(BOARD_TCC893X_STB_DEMO)
		return 0;
	#else
		#if (HW_REV == 0x2000 || HW_REV == 0x3000)
		return (gpio_get(TCC_GPD(21)));
		#else
		return (gpio_get(TCC_GPD(10)));
		#endif
	#endif
#else	//for TCC880x

	PGPIO pGPIO = (PGPIO)(HwGPIO_BASE);

	return (pGPIO->GPFDAT&Hw11);
#endif
}
#endif
#if defined(FEATURE_TCC8803F_2CS_V6_JM5_SLOT)
static int CardDetectPortFor_2CS_JM5(void)
{
	PGPIO pGPIO = (PGPIO)(HwGPIO_BASE);
	return (pGPIO->GPADAT&Hw15);
}
static int WriteProtectPort_2CS_JM5(void)
{
	PGPIO pGPIO = (PGPIO)(HwGPIO_BASE);
	return (pGPIO->GPFDAT&Hw12);
}
#endif
#if defined(FEATURE_TCC8803F_2CS_V6_JM2_SLOT)
static int CardDetectPortFor_2CS_JM2(void)
{
	PGPIO pGPIO = (PGPIO)(HwGPIO_BASE);
	return (pGPIO->GPADAT&Hw13);
}
static int WriteProtectPort_2CS_JM2(void)
{
	PGPIO pGPIO = (PGPIO)(HwGPIO_BASE);
	return (pGPIO->GPFDAT&Hw11);
}
#endif
///////////////////////////////////////////////////////////////////////////////


/////No #ifdef, #ifndef right///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef SLOT_TYPE_0
#define SLOT_TYPE_0				0
#define SLOT_BUSWIDTH_0			4
#define SLOT_MAX_SPEED_0		SLOT_MAX_SPEED_NORMAL
#define CARD_DETECT_FUNC_0		NULL
#define WRITE_PROTECT_FUNC_0	NULL
#endif
#ifndef SLOT_TYPE_1
#define SLOT_TYPE_1				0
#define SLOT_BUSWIDTH_1			4
#define SLOT_MAX_SPEED_1		SLOT_MAX_SPEED_NORMAL
#define CARD_DETECT_FUNC_1		NULL
#define WRITE_PROTECT_FUNC_1	NULL
#endif
#ifndef SLOT_TYPE_2
#define SLOT_TYPE_2				0
#define SLOT_BUSWIDTH_2			4
#define SLOT_MAX_SPEED_2		SLOT_MAX_SPEED_NORMAL
#define CARD_DETECT_FUNC_2		NULL
#define WRITE_PROTECT_FUNC_2	NULL
#endif
#ifndef SLOT_TYPE_3
#define SLOT_TYPE_3				0
#define SLOT_BUSWIDTH_3			4
#define SLOT_MAX_SPEED_3		SLOT_MAX_SPEED_NORMAL
#define CARD_DETECT_FUNC_3		NULL
#define WRITE_PROTECT_FUNC_3	NULL
#endif

SD_HW_SLOTINFO_T sSdHwSlotInfo[] =
{
	{
		0/*SD Host Number*/, 
		HwSDMMC0_BASE/*SD Slot Register Address*/,
		SLOT_TYPE_0/*type*/, 
		SLOT_BUSWIDTH_0/*max bus width*/, 
		SLOT_MAX_SPEED_0/*speed*/, 
		CARD_DETECT_FUNC_0, 
		WRITE_PROTECT_FUNC_0,
		HwSDMMC_CHCTRL_CH0_CAP0,
		HwSDMMC_CHCTRL_CH0_CAP1
	},

	{
		1/*SD Host Number*/, 
		HwSDMMC1_BASE/*SD Slot Register Address*/,
		SLOT_TYPE_1/*type*/, 
		SLOT_BUSWIDTH_1/*max bus width*/, 
		SLOT_MAX_SPEED_1/*speed*/, 
		CARD_DETECT_FUNC_1, 
		WRITE_PROTECT_FUNC_1,
		HwSDMMC_CHCTRL_CH1_CAP0,
		HwSDMMC_CHCTRL_CH1_CAP1
	},

	{
		2/*SD Host Number*/, 
		HwSDMMC2_BASE/*SD Slot Register Address*/,
		SLOT_TYPE_2/*type*/, 
		SLOT_BUSWIDTH_2/*max bus width*/, 
		SLOT_MAX_SPEED_2/*speed*/, 
		CARD_DETECT_FUNC_2, 
		WRITE_PROTECT_FUNC_2,
		HwSDMMC_CHCTRL_CH2_CAP0,
		HwSDMMC_CHCTRL_CH2_CAP1
	},

	{
		3/*SD Host Number*/, 
		HwSDMMC3_BASE/*SD Slot Register Address*/,
		SLOT_TYPE_3/*type*/, 
		SLOT_BUSWIDTH_3/*max bus width*/, 
		SLOT_MAX_SPEED_3/*speed*/, 
		CARD_DETECT_FUNC_3, 
		WRITE_PROTECT_FUNC_3,
		HwSDMMC_CHCTRL_CH3_CAP0,
		HwSDMMC_CHCTRL_CH3_CAP1
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int SD_HW_IsSdHostNeeded(unsigned char ucHostIndex)
{
	int i;

	for( i=0 ; i<(int)dim(sSdHwSlotInfo) ; i++ )
	{
		if(sSdHwSlotInfo[i].ucHostNum == ucHostIndex
			&& sSdHwSlotInfo[i].uiSlotAttr != 0)
			return 1;
	}

	return 0;
}

static void SD_HW_InitializePort(void)
{
#if defined(TCC892X)
#else
	PGPIO pGPIO = (PGPIO)(HwGPIO_BASE);
#endif

#if defined(FEATURE_TCC8803F_2CS_V6_JM5_SLOT)
	//========================================================
	// SD/MMC: JM5, SD Port 1 Power ON (GPIO_F13)
	//========================================================
	BITCLR(pGPIO->GPFFN1, 0x00F00000);
	BITSET(pGPIO->GPFEN, Hw13);
	BITSET(pGPIO->GPFDAT, Hw13);

	//========================================================
	// SD/MMC: JM5(GPIO_F18~23=SD1, GPIO_F13=CD), SD Port 1
	//========================================================
	BITCSET(pGPIO->GPFFN2, 0xFFFFFF00, 0x22222200);	// SD1_D0 ~ SD1_D3, SD1_CLK, SD1_CMD
	BITCLR(pGPIO->GPAEN, Hw15);
	BITCLR(pGPIO->GPFEN, Hw12);
	BITCSET(pGPIO->GPFFN1, 0x000F0000, 0x00000000);	// GPIO_F12 --> SD1_WP
	BITCSET(pGPIO->GPAFN1, 0xF0000000, 0x00000000);	// GPIO_A15 --> SD1_CD
	BITCSET(pGPIO->GPFCD1, 0x0000FFF0, 0x0000D550);	// SD1_CLK(GPIO_F23) Strength: 3, Others:1
#endif

#if defined(FEATURE_TCC8803F_2CS_V6_JM2_SLOT)
	//========================================================
	// SD/MMC: JM2, SD Port 3 Power ON (GPIO_F10)
	//========================================================
	BITCLR(pGPIO->GPFFN1, 0x00000F00);
	BITSET(pGPIO->GPFEN, Hw10);
	BITSET(pGPIO->GPFDAT, Hw10);

	//========================================================
	// SD/MMC: JM2(GPIO_F0~9=SD3, GPIO_F10=CD), SD Port 3
	//========================================================
	BITCSET(pGPIO->GPFFN0, 0xFFFFFFFF, 0x22222222);	// SD3_D0 ~ SD3_D7
	BITCSET(pGPIO->GPFFN1, 0x000000FF, 0x00000022);	// SD3_CLK, SD3_CMD
	BITCLR(pGPIO->GPAEN, Hw13);
	BITCLR(pGPIO->GPFEN, Hw11);
	BITCSET(pGPIO->GPAFN1, 0x00F00000, 0x00000000);	// GPIO_A13 --> SD3_CD
	BITCSET(pGPIO->GPFFN1, 0x0000F000, 0x00000000);	// GPIO_F11 --> SD3_WP
	BITCSET(pGPIO->GPFCD0, 0x000FFFFF, 0x000D5555);	// SD3_CLK(GPIO_F9) Strength: 3, Others:1
#endif

#if defined(FEATURE_TCC93_88_8910_DEMO_JM5_SLOT)
	//========================================================
	// SD/MMC: JM5(GPIO_F18~23=SD1, GPIO_F13=CD), SD Port 1
	//========================================================
	BITCSET(pGPIO->GPFFN2, 0xFFFFFF00, 0x22222200);	// SD1_D0 ~ SD1_D3, SD1_CLK, SD1_CMD
	BITCLR(pGPIO->GPFEN, Hw13|Hw12);
	BITCSET(pGPIO->GPFFN1, 0x00FF0000, 0x00000000);	// GPIO_F13 --> SD1_CD, GPIO_F12 --> SD1_WP
	BITCSET(pGPIO->GPFCD1, 0x0000FFF0, 0x0000D550);	// SD1_CLK(GPIO_F23) Strength: 3, Others:1
#endif //defined(FEATURE_TCC93_88_8910_DEMO_JM5_SLOT)

#if defined(FEATURE_TCC93_88_8910_DEMO_JM2_SLOT)
#if defined(TCC892X)	//for TCC892x
#if defined(BOARD_TCC892X_STB_DEMO)
#if defined(TARGET_TCC8925_STB_DONGLE) || defined(TARGET_TCC8925_HDB892F)
	//=====================================================
	// SD/MMC: JM2(GPIO_D15~20=SD0), SD Port 4
	//=====================================================
	gpio_config(TCC_GPD(18) , GPIO_FN2|GPIO_CD2);	// SD0_D0
	gpio_config(TCC_GPD(17) , GPIO_FN2|GPIO_CD2);	// SD0_D1
	gpio_config(TCC_GPD(16) , GPIO_FN2|GPIO_CD2);	// SD0_D2
	gpio_config(TCC_GPD(15) , GPIO_FN2|GPIO_CD2);	// SD0_D3
	#if HW_REV==0x9112
       gpio_config(TCC_GPD(14) , GPIO_FN2|GPIO_CD2);   // SD0_D4
       gpio_config(TCC_GPD(13) , GPIO_FN2|GPIO_CD2);   // SD0_D5
       gpio_config(TCC_GPD(12) , GPIO_FN2|GPIO_CD2);   // SD0_D6
       gpio_config(TCC_GPD(11) , GPIO_FN2|GPIO_CD2);   // SD0_D7
	#endif
	gpio_config(TCC_GPD(20) , GPIO_FN2|GPIO_CD3);	// SD0_CLK
	gpio_config(TCC_GPD(19) , GPIO_FN2|GPIO_CD3);	// SD0_CMD
#else
	//=====================================================
	// SD/MMC: JM2(GPIO_D15~20=SD0), SD Port 4
	//=====================================================
	gpio_config(TCC_GPF(19) , GPIO_FN2|GPIO_CD2);	// SD0_D0
	gpio_config(TCC_GPF(20) , GPIO_FN2|GPIO_CD2);	// SD0_D1
	gpio_config(TCC_GPF(21) , GPIO_FN2|GPIO_CD2);	// SD0_D2
	gpio_config(TCC_GPF(22) , GPIO_FN2|GPIO_CD2);	// SD0_D3

	gpio_config(TCC_GPF(17) , GPIO_FN2|GPIO_CD3);	// SD0_CLK
	gpio_config(TCC_GPF(18) , GPIO_FN2|GPIO_CD3);	// SD0_CMD
#endif
#else
#if (HW_REV >= 0x1005)	//||(HW_REV == 0x1006)||(HW_REV == 0x1007)||(HW_REV == 0x1008)
#if (HW_REV == 0x1008)
	//========================================================
	// SD_CARD0 Power ON (GPIO_C20)
	//========================================================
	gpio_config(TCC_GPC(20) , GPIO_FN0);
	gpio_set(TCC_GPC(20) , 1);
#endif

	//=====================================================
	// SD/MMC: JM2(GPIO_D11~20=SD0, GPIO_B14=CD), SD Port 4
	//=====================================================
	gpio_config(TCC_GPD(18) , GPIO_FN2|GPIO_CD2);	// SD0_D0
	gpio_config(TCC_GPD(17) , GPIO_FN2|GPIO_CD2);	// SD0_D1
	gpio_config(TCC_GPD(16) , GPIO_FN2|GPIO_CD2);	// SD0_D2
	gpio_config(TCC_GPD(15) , GPIO_FN2|GPIO_CD2);	// SD0_D3
	gpio_config(TCC_GPD(14) , GPIO_FN2|GPIO_CD2);	// SD0_D4
	gpio_config(TCC_GPD(13) , GPIO_FN2|GPIO_CD2);	// SD0_D5
	gpio_config(TCC_GPD(12) , GPIO_FN2|GPIO_CD2);	// SD0_D6
	gpio_config(TCC_GPD(11) , GPIO_FN2|GPIO_CD2);	// SD0_D7

	gpio_config(TCC_GPD(20) , GPIO_FN2|GPIO_CD3);	// SD0_CLK
	gpio_config(TCC_GPD(19) , GPIO_FN2|GPIO_CD3);	// SD0_CMD
#else	//(HW_REV >= 0x1000)&&(HW_REV <= 0x1004)
#if defined(TARGET_M805_892X_EVM)
	//=====================================================
	// SD/MMC: JM2(GPIO_D15~20=SD0), SD Port 4
	//=====================================================
	gpio_config(TCC_GPD(18) , GPIO_FN2|GPIO_CD2);	// SD0_D0
	gpio_config(TCC_GPD(17) , GPIO_FN2|GPIO_CD2);	// SD0_D1
	gpio_config(TCC_GPD(16) , GPIO_FN2|GPIO_CD2);	// SD0_D2
	gpio_config(TCC_GPD(15) , GPIO_FN2|GPIO_CD2);	// SD0_D3

	gpio_config(TCC_GPD(20) , GPIO_FN2|GPIO_CD3);	// SD0_CLK
	gpio_config(TCC_GPD(19) , GPIO_FN2|GPIO_CD3);	// SD0_CMD
#else
	//=====================================================
	// SD/MMC: JM2(GPIO_F17~26=SD1, GPIO_D12=CD), SD Port 5
	//=====================================================
	gpio_config(TCC_GPF(19) , GPIO_FN2|GPIO_CD2);	// SD1_D0
	gpio_config(TCC_GPF(20) , GPIO_FN2|GPIO_CD2);	// SD1_D1
	gpio_config(TCC_GPF(21) , GPIO_FN2|GPIO_CD2);	// SD1_D2
	gpio_config(TCC_GPF(22) , GPIO_FN2|GPIO_CD2);	// SD1_D3
	gpio_config(TCC_GPF(23) , GPIO_FN2|GPIO_CD2);	// SD1_D4
	gpio_config(TCC_GPF(24) , GPIO_FN2|GPIO_CD2);	// SD1_D5
	gpio_config(TCC_GPF(25) , GPIO_FN2|GPIO_CD2);	// SD1_D6
	gpio_config(TCC_GPF(26) , GPIO_FN2|GPIO_CD2);	// SD1_D7

	gpio_config(TCC_GPF(17) , GPIO_FN2|GPIO_CD3);	// SD1_CLK
	gpio_config(TCC_GPF(18) , GPIO_FN2|GPIO_CD3);	// SD1_CMD
#endif
#endif
#endif

#endif

#endif	//#if defined(FEATURE_TCC93_88_8910_DEMO_JM2_SLOT)

	/* TCC 893x SD/MMC Port Configuration */
#if defined(TCC893X)
#if defined(CONFIG_CHIP_TCC8930)
	#if defined(BOARD_TCC893X_STB_DEMO)
    gpio_config(TCC_GPD(29) , GPIO_FN2|GPIO_CD2);    // SD0_D0
    gpio_config(TCC_GPD(28) , GPIO_FN2|GPIO_CD2);    // SD0_D1
    gpio_config(TCC_GPD(27) , GPIO_FN2|GPIO_CD2);    // SD0_D2
    gpio_config(TCC_GPD(26) , GPIO_FN2|GPIO_CD2);    // SD0_D3

    gpio_config(TCC_GPD(30) , GPIO_FN2|GPIO_CD3);    // SD0_CLK
    gpio_config(TCC_GPD(31) , GPIO_FN2|GPIO_CD3);    // SD0_CMD
	#else
	/* 0x1000 : TCC8930_D3_08X4_SV0.1 - DDR3 1024(32Bit) */	
	gpio_config(TCC_GPD(29) , GPIO_FN2|GPIO_CD2);	// SD3_D0
	gpio_config(TCC_GPD(28) , GPIO_FN2|GPIO_CD2);	// SD3_D1
	gpio_config(TCC_GPD(27) , GPIO_FN2|GPIO_CD2);	// SD3_D2
	gpio_config(TCC_GPD(26) , GPIO_FN2|GPIO_CD2);	// SD3_D3
	gpio_config(TCC_GPD(25) , GPIO_FN2|GPIO_CD2);	// SD3_D4
	gpio_config(TCC_GPD(24) , GPIO_FN2|GPIO_CD2);	// SD3_D5
	gpio_config(TCC_GPD(23) , GPIO_FN2|GPIO_CD2);	// SD3_D6
	gpio_config(TCC_GPD(22) , GPIO_FN2|GPIO_CD2);	// SD3_D7
	gpio_config(TCC_GPD(30) , GPIO_FN2|GPIO_CD3);	// SD3_CMD
	gpio_config(TCC_GPD(31) , GPIO_FN2|GPIO_CD3);	// SD3_CLK
	#endif
	#elif defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8933) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8937S)
	#if defined(BOARD_TCC893X_STB_DEMO)
		#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8937S)
		gpio_config(TCC_GPD(18) , GPIO_FN2|GPIO_CD2);    // SD0_D0
		gpio_config(TCC_GPD(17) , GPIO_FN2|GPIO_CD2);    // SD0_D1
		gpio_config(TCC_GPD(16) , GPIO_FN2|GPIO_CD2);    // SD0_D2
		gpio_config(TCC_GPD(15) , GPIO_FN2|GPIO_CD2);    // SD0_D3
		#if HW_REV==0x9313
		gpio_config(TCC_GPD(14) , GPIO_FN2|GPIO_CD2);    // SD3_D4
		gpio_config(TCC_GPD(13) , GPIO_FN2|GPIO_CD2);    // SD3_D5
		gpio_config(TCC_GPD(12) , GPIO_FN2|GPIO_CD2);    // SD3_D6
		gpio_config(TCC_GPD(11) , GPIO_FN2|GPIO_CD2);    // SD3_D7
		#endif
		gpio_config(TCC_GPD(20) , GPIO_FN2|GPIO_CD3);    // SD0_CLK
		gpio_config(TCC_GPD(19) , GPIO_FN2|GPIO_CD3);    // SD0_CMD
		#else
		gpio_config(TCC_GPD(29) , GPIO_FN2|GPIO_CD2);    // SD0_D0
		gpio_config(TCC_GPD(28) , GPIO_FN2|GPIO_CD2);    // SD0_D1
		gpio_config(TCC_GPD(27) , GPIO_FN2|GPIO_CD2);    // SD0_D2
		gpio_config(TCC_GPD(26) , GPIO_FN2|GPIO_CD2);    // SD0_D3
		#if HW_REV==0x9312
		gpio_config(TCC_GPD(25) , GPIO_FN2|GPIO_CD2);    // SD3_D4
		gpio_config(TCC_GPD(24) , GPIO_FN2|GPIO_CD2);    // SD3_D5
		gpio_config(TCC_GPD(23) , GPIO_FN2|GPIO_CD2);    // SD3_D6
		gpio_config(TCC_GPD(22) , GPIO_FN2|GPIO_CD2);    // SD3_D7
		#endif
		gpio_config(TCC_GPD(30) , GPIO_FN2|GPIO_CD3);    // SD0_CLK
		gpio_config(TCC_GPD(31) , GPIO_FN2|GPIO_CD3);    // SD0_CMD
		#endif
	#else /*TCC8935*/
		#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8937S)
		/*TCC8935S*/
		/*TCC8937S*/
		/* TCC8935X_D3_08X4_2CS_SV1.0 - DDR3 1024MB(16Bit) */
		gpio_config(TCC_GPD(18) , GPIO_FN2|GPIO_CD2);	// SD3_D0
		gpio_config(TCC_GPD(17) , GPIO_FN2|GPIO_CD2);	// SD3_D1
		gpio_config(TCC_GPD(16) , GPIO_FN2|GPIO_CD2);	// SD3_D2
		gpio_config(TCC_GPD(15) , GPIO_FN2|GPIO_CD2);	// SD3_D3
		gpio_config(TCC_GPD(14) , GPIO_FN2|GPIO_CD2);	// SD3_D4
		gpio_config(TCC_GPD(13) , GPIO_FN2|GPIO_CD2);	// SD3_D5
		gpio_config(TCC_GPD(12) , GPIO_FN2|GPIO_CD2);	// SD3_D6
		gpio_config(TCC_GPD(11) , GPIO_FN2|GPIO_CD2);	// SD3_D7
		gpio_config(TCC_GPD(19) , GPIO_FN2|GPIO_CD3);	// SD3_CMD
		gpio_config(TCC_GPD(20) , GPIO_FN2|GPIO_CD3);	// SD3_CLK
		#else
		/* TCC8935X_D3_08X4_2CS_SV0.2 - DDR3 1024MB(16Bit) */
		gpio_config(TCC_GPD(29) , GPIO_FN2|GPIO_CD2);	// SD3_D0
		gpio_config(TCC_GPD(28) , GPIO_FN2|GPIO_CD2);	// SD3_D1
		gpio_config(TCC_GPD(27) , GPIO_FN2|GPIO_CD2);	// SD3_D2
		gpio_config(TCC_GPD(26) , GPIO_FN2|GPIO_CD2);	// SD3_D3
		gpio_config(TCC_GPD(25) , GPIO_FN2|GPIO_CD2);	// SD3_D4
		gpio_config(TCC_GPD(24) , GPIO_FN2|GPIO_CD2);	// SD3_D5
		gpio_config(TCC_GPD(23) , GPIO_FN2|GPIO_CD2);	// SD3_D6
		gpio_config(TCC_GPD(22) , GPIO_FN2|GPIO_CD2);	// SD3_D7
		gpio_config(TCC_GPD(30) , GPIO_FN2|GPIO_CD3);	// SD3_CMD
		gpio_config(TCC_GPD(31) , GPIO_FN2|GPIO_CD3);	// SD3_CLK
		#endif
	#endif
#endif
#endif
}

static void SD_HW_InitializeClock(void)
{
	//=====================================================
	// SD Channel Control Register Clock Enable
	//=====================================================
	tca_ckc_setiopwdn(RB_SDMMCCHANNELCONTROLLER,0);		// SD Channel Control Register Clock Enable

	if(SD_HW_IsSdHostNeeded(0))
	{
		//=====================================================
		// SD Host Controller #0 Clock Enable
		//=====================================================
		if (SD_HW_IS_DDR_MODE(0))
			tca_ckc_setperi(PERI_SDMMC0, ENABLE, 400000);
		else
			tca_ckc_setperi(PERI_SDMMC0, ENABLE, 480000);
		_tca_delay();
		tca_ckc_setiopwdn(RB_SDMMC0CONTROLLER,0);
	}

	if(SD_HW_IsSdHostNeeded(1))
	{
#if !(defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8937S))
		//=====================================================
		// SD Host Controller #1 Clock Enable
		//=====================================================
		if (SD_HW_IS_DDR_MODE(1))
			tca_ckc_setperi(PERI_SDMMC1, ENABLE, 400000);
		else
			tca_ckc_setperi(PERI_SDMMC1, ENABLE, 480000);
		_tca_delay();
		tca_ckc_setiopwdn(RB_SDMMC1CONTROLLER,0);
#endif
	}

	if(SD_HW_IsSdHostNeeded(2))
	{
		//=====================================================
		// SD Host Controller #2 Clock Enable
		//=====================================================
		if (SD_HW_IS_DDR_MODE(2))
			tca_ckc_setperi(PERI_SDMMC2, ENABLE, 400000);
		else
			tca_ckc_setperi(PERI_SDMMC2, ENABLE, 480000);
		_tca_delay();
		tca_ckc_setiopwdn(RB_SDMMC2CONTROLLER,0);
	}

	if(SD_HW_IsSdHostNeeded(3))
	{
		//=====================================================
		// SD Host Controller #3 Clock Enable
		//=====================================================
		if (SD_HW_IS_DDR_MODE(3))
			tca_ckc_setperi(PERI_SDMMC3, ENABLE, 400000);
		else
			tca_ckc_setperi(PERI_SDMMC3, ENABLE, 480000);
		_tca_delay();
		tca_ckc_setiopwdn(RB_SDMMC3CONTROLLER,0);
	}
}

void SD_HW_Initialize(void)
{
	PIOBUSCFG pIOBUSCfg = (PIOBUSCFG)(HwIOBUSCFG_BASE);
	PSDCHCTRL pSDCTRL = (PSDCHCTRL)HwSDMMC_CHCTRL_BASE;

	SD_HW_InitializePort();
	SD_HW_InitializeClock();

	//=====================================================
	// Peri block reset
	//=====================================================
	/* Channel Control Register Reset */
#if defined(TCC892X) || defined(TCC893X)
	pIOBUSCfg->HRSTEN0.bREG.SDMMC_CTRL = 0;
	_tca_delay();
	pIOBUSCfg->HRSTEN0.bREG.SDMMC_CTRL = 1;
	_tca_delay();
#else
	BITCLR(pIOBUSCfg->HRSTEN0, Hw4);
	_tca_delay();
	BITSET(pIOBUSCfg->HRSTEN0, Hw4);
	_tca_delay();
#endif

	if(SD_HW_IsSdHostNeeded(0))
	{
		/* SD Host Controller #0 Reset */
#if defined(TCC892X)
		pIOBUSCfg->HRSTEN0.bREG.SDMMC0 = 0;
		_tca_delay();
		pIOBUSCfg->HRSTEN0.bREG.SDMMC0 = 1;
		_tca_delay();
		pSDCTRL->SD0PRESET1.nREG = 0x0CFF9870;
#elif defined(TCC893X)
		pIOBUSCfg->HRSTEN0.bREG.SDMMC0 = 0;
		_tca_delay();
		pIOBUSCfg->HRSTEN0.bREG.SDMMC0 = 1;
		_tca_delay();
//		pSDCTRL->SD0PRESET1.nREG = 0x0CFF9870;
#else
		BITCLR(pIOBUSCfg->HRSTEN0, Hw5);
		_tca_delay();
		BITSET(pIOBUSCfg->HRSTEN0, Hw5);
		_tca_delay();
		pSDCTRL->SD0PRESET1 = 0x0CFF9870;
#endif
	}

	if(SD_HW_IsSdHostNeeded(1))
	{
		/* SD Host Controller #1 Reset */
#if defined(TCC892X)
		pIOBUSCfg->HRSTEN0.bREG.SDMMC1 = 0;
		_tca_delay();
		pIOBUSCfg->HRSTEN0.bREG.SDMMC1 = 1;
		_tca_delay();

		pSDCTRL->SD1PRESET1.nREG = 0x0CFF9870;
#elif defined(TCC893X)
		pIOBUSCfg->HRSTEN0.bREG.SDMMC1 = 0;
		_tca_delay();
		pIOBUSCfg->HRSTEN0.bREG.SDMMC1 = 1;
		_tca_delay();

//		pSDCTRL->SD1PRESET1.nREG = 0x0CFF9870;
#else
		BITCLR(pIOBUSCfg->HRSTEN1, Hw3);
		_tca_delay();
		BITSET(pIOBUSCfg->HRSTEN1, Hw3);
		_tca_delay();

		pSDCTRL->SD1PRESET1 = 0x0CFF9870;
#endif
	}

	if(SD_HW_IsSdHostNeeded(2))
	{
		/* SD Host Controller #2 Reset */
#if defined(TCC892X)
		pIOBUSCfg->HRSTEN0.bREG.SDMMC2 = 0;
		_tca_delay();
		pIOBUSCfg->HRSTEN0.bREG.SDMMC2 = 1;
		_tca_delay();

		pSDCTRL->SD1PRESET1.nREG = 0x0CFF9870;
#elif defined(TCC893X)
		pIOBUSCfg->HRSTEN0.bREG.SDMMC2 = 0;
		_tca_delay();
		pIOBUSCfg->HRSTEN0.bREG.SDMMC2 = 1;
		_tca_delay();

//		pSDCTRL->SD1PRESET1.nREG = 0x0CFF9870;
#else
		BITCLR(pIOBUSCfg->HRSTEN1, Hw4);
		_tca_delay();
		BITSET(pIOBUSCfg->HRSTEN1, Hw4);
		_tca_delay();

		pSDCTRL->SD1PRESET1 = 0x0CFF9870;
#endif
	}

	if(SD_HW_IsSdHostNeeded(3))
	{
		/* SD Host Controller #3 Reset */
#if defined(TCC892X)
		pIOBUSCfg->HRSTEN0.bREG.SDMMC3 = 0;
		_tca_delay();
		pIOBUSCfg->HRSTEN0.bREG.SDMMC3 = 1;
		_tca_delay();

		pSDCTRL->SD3PRESET1.nREG = 0x0CFF9870;
#elif defined(TCC893X)
		pIOBUSCfg->HRSTEN0.bREG.SDMMC3 = 0;
		_tca_delay();
		pIOBUSCfg->HRSTEN0.bREG.SDMMC3 = 1;
		_tca_delay();

//		pSDCTRL->SD3PRESET1.nREG = 0x0CFF9870;
#else
		BITCLR(pIOBUSCfg->HRSTEN1, Hw5);
		_tca_delay();
		BITSET(pIOBUSCfg->HRSTEN1, Hw5);
		_tca_delay();

		pSDCTRL->SD3PRESET1 = 0x0CFF9870;
#endif
	}

	_tca_delay();
#if defined(TCC892X) || defined(TCC893X)
	pSDCTRL->SD0CMDDAT.nREG = 0x00000000;
	pSDCTRL->SD1CMDDAT.nREG = 0x00000000;
	pSDCTRL->SD2CMDDAT.nREG = 0x00000000;
	pSDCTRL->SD3CMDDAT.nREG = 0x00000000;
#else
	pSDCTRL->SD0CMDDAT = 0x00000000;
	pSDCTRL->SD1CMDDAT = 0x00000000;
	pSDCTRL->SD2CMDDAT = 0x00000000;
	pSDCTRL->SD3CMDDAT = 0x00000000;
#endif
	_tca_delay();
}

static int SD_HW_Convert_SlotIndex_To_HwSlotInfoIndex(int iSlotIndex)
{
	int i,iSlotIndexTemp=0;

	for( i=0 ; i<(int)dim(sSdHwSlotInfo) ; i++ )
	{
		if(sSdHwSlotInfo[i].uiSlotAttr == 0)
			continue;
		if(iSlotIndexTemp++ == iSlotIndex)
			return i;
	}
	SD_ASSERT(0);

	return 0;
}

static int SD_HW_Convert_HwSlotInfoIndex_To_SlotIndex(int iHwSlotIndex)
{
	int i,iSlotIndex=-1;

	SD_ASSERT( iHwSlotIndex < (int)dim(sSdHwSlotInfo) );
	for( i=0 ; i<=iHwSlotIndex ; i++ )
	{
		if(sSdHwSlotInfo[i].uiSlotAttr == 0)
			continue;
		iSlotIndex++;
	}
	SD_ASSERT(iSlotIndex>=0);

	return iSlotIndex;
}

int SD_HW_Get_CardDetectPort(int iSlotIndex)
{
	int iIndex = SD_HW_Convert_SlotIndex_To_HwSlotInfoIndex(iSlotIndex);

	SD_ASSERT( iIndex < (int)dim(sSdHwSlotInfo) );

	if(sSdHwSlotInfo[iIndex].CardDetect)
		return sSdHwSlotInfo[iIndex].CardDetect();

	return 1;
}

int SD_HW_Get_WriteProtectPort(int iSlotIndex)
{
	int iIndex = SD_HW_Convert_SlotIndex_To_HwSlotInfoIndex(iSlotIndex);

	SD_ASSERT( iSlotIndex < (int)dim(sSdHwSlotInfo) );

	if(sSdHwSlotInfo[iIndex].WriteProtect)
		return sSdHwSlotInfo[iIndex].WriteProtect();

	return 0;
}

PSDSLOTREG_T SD_HW_GetSdSlotReg(int iSlotIndex)
{
	int iIndex = SD_HW_Convert_SlotIndex_To_HwSlotInfoIndex(iSlotIndex);

	SD_ASSERT( iIndex < (int)dim(sSdHwSlotInfo) );

	return (PSDSLOTREG_T)sSdHwSlotInfo[iIndex].ulSdSlotRegAddr;
}

int SD_HW_GetMaxBusWidth(int iSlotIndex)
{
	int iIndex = SD_HW_Convert_SlotIndex_To_HwSlotInfoIndex(iSlotIndex);
	SD_ASSERT( iIndex < (int)dim(sSdHwSlotInfo) );
	return sSdHwSlotInfo[iIndex].iMaxBusWidth;
}

int SD_HW_IsSupportHighSpeed(int iSlotIndex)
{
	int iIndex = SD_HW_Convert_SlotIndex_To_HwSlotInfoIndex(iSlotIndex);

	SD_ASSERT( iIndex < (int)dim(sSdHwSlotInfo) );

	if(sSdHwSlotInfo[iIndex].eSlotSpeed == SLOT_MAX_SPEED_HIGH)
		return 1;
	else
		return 0;
}

int SD_HW_GetTotalSlotCount(void)
{
	int i;
	int iCount=0;

	for( i=0 ; i<(int)dim(sSdHwSlotInfo) ; i++ )
	{
		if(sSdHwSlotInfo[i].uiSlotAttr != 0)
			iCount++;
	}

	return iCount;
}

int SD_HW_GetBootSlotIndex(void)
{
	int i;

	for( i=0 ; i<(int)dim(sSdHwSlotInfo) ; i++ )
	{
		if(sSdHwSlotInfo[i].uiSlotAttr & SLOT_ATTR_BOOT)
			return SD_HW_Convert_HwSlotInfoIndex_To_SlotIndex(i);
	}

	return -1;
}

int SD_HW_GetUpdateSlotIndex(void)
{
	int i;

	for( i=0 ; i<(int)dim(sSdHwSlotInfo) ; i++ )
	{
		if(sSdHwSlotInfo[i].uiSlotAttr & SLOT_ATTR_UPDATE)
			return SD_HW_Convert_HwSlotInfoIndex_To_SlotIndex(i);
	}

	return -1;
}

void SDMMC_ms_delay(unsigned long ul_ms)
{
#define TCFG_TCKSEL(x) 		(x<<4)
#define TCFG_CON 		Hw1
#define TCFG_EN 		Hw0
	PTIMER pTIMER = (TIMER*)TCC_TIMER_BASE;
	volatile unsigned int baseCounter;
	volatile unsigned int  timed;
#if defined(TCC892X) || defined(TCC893X)
	pTIMER->TCFG1.nREG= TCFG_TCKSEL(5)|TCFG_CON|TCFG_EN;
	baseCounter = pTIMER->TCNT1.bREG.TCNT;
	do{
		timed = (pTIMER->TCNT1.bREG.TCNT - baseCounter) & 0xFFFF;
	}while(timed < 12 * ul_ms);//1ms
	BITCLR(pTIMER->TCFG1.nREG, TCFG_EN);
#else
	pTIMER->TCFG1= TCFG_TCKSEL(5)|TCFG_CON|TCFG_EN;
	baseCounter = pTIMER->TCNT1;
	do{
		timed = (pTIMER->TCNT1 - baseCounter) & 0xFFFF;
	}while(timed < 12 * ul_ms);//1ms
	BITCLR(pTIMER->TCFG1, TCFG_EN);
#endif
}

extern void *memcpy(void *, const void *, unsigned int);

void SDMMC_memcpy(void *pvTgt, const void *pvSrc, unsigned int uiLen)
{
#ifdef _LINUX_
	if( ( ((unsigned int)pvTgt)&0x3 ) || ( ((unsigned int)pvSrc)&0x3 ) )
	{
		unsigned char *pucTgt = (unsigned char*)pvTgt;
		unsigned char *pucSrc = (unsigned char*)pvSrc;
		while(uiLen--)
			*pucTgt++ = *pucSrc++;
	}
	else
	{
		memcpy(pvTgt,pvSrc,uiLen);
	}
#else
	memcpy(pvTgt,pvSrc,uiLen);
#endif
}

int SD_HW_IS_HIGH_SPEED(int slot_num)
{
	int i;

	for( i=0 ; i<(int)dim(sSdHwSlotInfo) ; i++ )
	{
		if(sSdHwSlotInfo[i].uiSlotAttr & SLOT_ATTR_BOOT) {
			//printf("slot num %d\n", i);
			return (int)sSdHwSlotInfo[i].eSlotSpeed;
		}
	}

	return 0;
#if 0
	return (int)sSdHwSlotInfo[slot_num].eSlotSpeed;
#endif
}

int SD_HW_IS_DDR_MODE(int slot_num)
{
	int i;

	for( i=0 ; i<(int)dim(sSdHwSlotInfo) ; i++ )
	{
		if(sSdHwSlotInfo[i].uiSlotAttr & SLOT_ATTR_BOOT)
			return (sSdHwSlotInfo[i].eSlotSpeed == SLOT_MAX_SPEED_DDR);
	}
	return 0;
}

int SD_HW_SET_DDR_CAP(int slot_num)
{
	int i;

	for( i=0 ; i<(int)dim(sSdHwSlotInfo) ; i++ )
	{
		if(sSdHwSlotInfo[i].uiSlotAttr & SLOT_ATTR_BOOT) 
		{
			*(volatile unsigned long*)sSdHwSlotInfo[i].CAPPREG0_CH = 0x8CFF9870;
			*(volatile unsigned long*)sSdHwSlotInfo[i].CAPPREG1_CH = 0x00000007;
			//*(volatile unsigned long*)0x76020860 = 0x00000007;
			//*(volatile unsigned long*)0x76020864 = 0x8CFF9870;
		}
	}
	return 0;
}
