/*
 * Copyright (c) 2011 Telechips, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <platform/globals.h>
#include <platform/reg_physical.h>
#include <platform/tcc_ckc.h>
#include <dev/axp192.h>
#include <i2c.h>

/************************************************************
* Function    : clock_init()
* Description :
*    - increase fbus clock (1.2V or higher level)
************************************************************/
void clock_init(void)
{
	tca_ckc_setfbusctrl( FBUS_CPU,    ENABLE, 8500000);	/*FBUS_CPU      850 MHz */	// 1.2V
#if defined(TARGET_TCC8935_UPC) || defined(TARGET_TCC8935_DONGLE)
	#if defined(DRAM_DDR3)
	tca_ckc_setfbusctrl( FBUS_MEM,    ENABLE, 5120000);	/*FBUS_MEM      514.5 MHz */
	#else
	tca_ckc_setfbusctrl( FBUS_MEM,    ENABLE, 3000000);	/*FBUS_MEM      300 MHz */
	#endif
	tca_ckc_setfbusctrl( FBUS_DDI,    ENABLE, 2840000);	/*FBUS_DDI      284.5 MHz */
	tca_ckc_setfbusctrl( FBUS_GPU,   DISABLE, 2740000);	/*FBUS_GRP      328.1 MHz */
	tca_ckc_setfbusctrl( FBUS_IO,     ENABLE, 1535000);	/*FBUS_IOB      153.8 MHz */
	tca_ckc_setfbusctrl( FBUS_VBUS,  DISABLE, 2560000);	/*FBUS_VBUS     256.3 MHz */
	tca_ckc_setfbusctrl( FBUS_VCORE, DISABLE, 2560000);	/*FBUS_VCODEC   256.3 MHz */
	tca_ckc_setfbusctrl( FBUS_HSIO,   ENABLE, 2047000);	/*FBUS_HSIO     213.6 MHz */
	tca_ckc_setfbusctrl( FBUS_SMU,    ENABLE, 1535000);	/*FBUS_SMU      153.8 MHz */
#if !defined(CONFIG_CHIP_TCC8933S) && !defined(CONFIG_CHIP_TCC8935S) && !defined(CONFIG_CHIP_TCC8937S)
	tca_ckc_setfbusctrl( FBUS_G2D,   DISABLE, 2840000);	/*FBUS_G2D      284.5 MHz */
	tca_ckc_setfbusctrl( FBUS_CM3,   DISABLE, 2047000);	/*FBUS_CM3      213.6 MHz */
#endif
#else  // TCC_CORE_1_1V_USE
	#if defined(DRAM_DDR3)
	tca_ckc_setfbusctrl( FBUS_MEM,    ENABLE, 6000000);	/*FBUS_MEM      600 MHz */
	#else
	tca_ckc_setfbusctrl( FBUS_MEM,    ENABLE, 3000000);	/*FBUS_MEM      300 MHz */
	#endif
	tca_ckc_setfbusctrl( FBUS_DDI,    ENABLE, 3333334);	/*FBUS_DDI      333 MHz */
	tca_ckc_setfbusctrl( FBUS_GPU,   DISABLE, 3333334);	/*FBUS_GRP      384 MHz */
	tca_ckc_setfbusctrl( FBUS_IO,     ENABLE, 1800000);	/*FBUS_IOB      180 MHz */
	tca_ckc_setfbusctrl( FBUS_VBUS,  DISABLE, 3000000);	/*FBUS_VBUS     300 MHz */
	tca_ckc_setfbusctrl( FBUS_VCORE, DISABLE, 3000000);	/*FBUS_VCODEC   300 MHz */
	tca_ckc_setfbusctrl( FBUS_HSIO,   ENABLE, 2500000);	/*FBUS_HSIO     250 MHz */
	tca_ckc_setfbusctrl( FBUS_SMU,    ENABLE, 1800000);	/*FBUS_SMU      180 MHz */
#if !defined(CONFIG_CHIP_TCC8933S) && !defined(CONFIG_CHIP_TCC8935S) && !defined(CONFIG_CHIP_TCC8937S)
	tca_ckc_setfbusctrl( FBUS_G2D,   DISABLE, 3333334);	/*FBUS_G2D      333 MHz */
	tca_ckc_setfbusctrl( FBUS_CM3,   DISABLE, 2500000);	/*FBUS_CM3      250 MHz */
#endif
#endif  // TCC_CORE_1_1V_USE
}

/************************************************************
* Function    : clock_init_early()
* Description :
*    - set pll/peri-clock and
*    - set fbus clock to low (1.0V level)
************************************************************/
void clock_init_early(void)
{
	tca_ckc_init();

	tca_ckc_setfbusctrl( FBUS_IO,     ENABLE,   60000);	/*FBUS_IO		6 MHz */
	tca_ckc_setfbusctrl( FBUS_SMU,    ENABLE,   60000);	/*FBUS_SMU		6 MHz */

//	tca_ckc_setpll( 5940000, 0, PLLSRC_XIN);	// for cpu
//	tca_ckc_setpll(12000000, 1, PLLSRC_XIN);	// for memory bus
#if defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8937S)
	tca_ckc_setpll(10000000, 2, PLLSRC_XIN);
	tca_ckc_setpll( 7200000, 3, PLLSRC_XIN);
#else
#if defined(TCC_CORE_1_1V_USE)
	tca_ckc_setpll( 5000000, 2, PLLSRC_XIN);
#if defined(CONFIG_AUDIO_PLL_USE)
	tca_ckc_setpll( 3750000, 3, PLLSRC_XIN);
#else
	tca_ckc_setpll( 5680000, 3, PLLSRC_XIN);
#endif
	tca_ckc_setpll( 5940000, 4, PLLSRC_XIN);
	tca_ckc_setpll( 6140000, 5, PLLSRC_XIN);
#else  // TCC_CORE_1_1V_USE
	tca_ckc_setpll(10000000, 2, PLLSRC_XIN);
#if defined(CONFIG_AUDIO_PLL_USE)
	tca_ckc_setpll( 3750000, 3, PLLSRC_XIN);
#else
	tca_ckc_setpll( 7680000, 3, PLLSRC_XIN);
#endif
	#if defined(TARGET_TCC8935_UPC) || defined(TARGET_TCC8935_DONGLE)
	tca_ckc_setpll( 5480000, 4, PLLSRC_XIN);
	#else
	tca_ckc_setpll( 5940000, 4, PLLSRC_XIN);
	#endif
	tca_ckc_setpll( 7200000, 5, PLLSRC_XIN);
#endif  // TCC_CORE_1_1V_USE
#endif

//	tca_ckc_setfbusctrl( FBUS_CPU,    ENABLE, 4000000);	/*FBUS_CPU      400 MHz */	// 1.0V
//	tca_ckc_setfbusctrl( FBUS_MEM,    ENABLE, 1250000);	/*FBUS_MEM      160 MHz */
	tca_ckc_setfbusctrl( FBUS_DDI,    ENABLE, 1250000);	/*FBUS_DDI      290 MHz */
	tca_ckc_setfbusctrl( FBUS_GPU,   DISABLE, 1250000);	/*FBUS_GRP      320 MHz */
	tca_ckc_setfbusctrl( FBUS_IO,     ENABLE,  720000);	/*FBUS_IO       190 MHz */
	tca_ckc_setfbusctrl( FBUS_VBUS,  DISABLE,  990000);	/*FBUS_VBUS     300 MHz */
	tca_ckc_setfbusctrl( FBUS_VCORE, DISABLE, 1250000);	/*FBUS_VCODEC   290 MHz */
	tca_ckc_setfbusctrl( FBUS_HSIO,   ENABLE, 1080000);	/*FBUS_HSIO     240 MHz */
	tca_ckc_setfbusctrl( FBUS_SMU,    ENABLE, 1000000);	/*FBUS_SMU      200 MHz */
#if !defined(CONFIG_CHIP_TCC8933S) && !defined(CONFIG_CHIP_TCC8935S) && !defined(CONFIG_CHIP_TCC8937S)
	tca_ckc_setfbusctrl( FBUS_G2D,   DISABLE, 1000000); /*FBUS_G2D      100 MHz */
	tca_ckc_setfbusctrl( FBUS_CM3,   DISABLE, 1000000); /*FBUS_CM3      100 MHz */
#endif
	// init Peri. Clock
	tca_ckc_setperi(PERI_TCX       , DISABLE,  120000);
	tca_ckc_setperi(PERI_TCT       ,  ENABLE,  120000);
	tca_ckc_setperi(PERI_TCZ       ,  ENABLE,  120000);
	tca_ckc_setperi(PERI_LCD0      , DISABLE,   10000);
	tca_ckc_setperi(PERI_LCDSI0    , DISABLE,   10000);
	tca_ckc_setperi(PERI_LCD1      ,  ENABLE,  960000);
	tca_ckc_setperi(PERI_LCDSI1    , DISABLE,   10000);
	tca_ckc_setperi(PERI_RESERVED07, DISABLE,   10000);
	tca_ckc_setperi(PERI_LCDTIMER  , DISABLE,   10000);
#if defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8937S)
	tca_ckc_setperi(PERI_JPEG      , DISABLE,   10000);
#else
	tca_ckc_setperi(PERI_RESERVED09, DISABLE,   10000);
#endif
	tca_ckc_setperi(PERI_RESERVED10, DISABLE,   10000);
	tca_ckc_setperi(PERI_RESERVED11, DISABLE,   10000);
	tca_ckc_setperi(PERI_GMAC      , DISABLE,   10000);
	tca_ckc_setperi(PERI_USBOTG    , DISABLE,   10000);
	tca_ckc_setperi(PERI_RESERVED14, DISABLE,   10000);
	tca_ckc_setperi(PERI_OUT0      , DISABLE,   10000);
	tca_ckc_setperi(PERI_USB20H    , DISABLE,   10000);
	tca_ckc_setperi(PERI_HDMI      , DISABLE,   10000);
	tca_ckc_setperi(PERI_HDMIA     , DISABLE,   10000);
	tca_ckc_setperi(PERI_OUT1      , DISABLE,   10000);
#if defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8937S)
	tca_ckc_setperi(PERI_EHI       , DISABLE,   10000);
#else
	tca_ckc_setperi(PERI_REMOTE    , DISABLE,   10000);
#endif
	tca_ckc_setperi(PERI_SDMMC0    , DISABLE,   10000);
#if !defined(CONFIG_CHIP_TCC8933S) && !defined(CONFIG_CHIP_TCC8935S) && !defined(CONFIG_CHIP_TCC8937S)
	tca_ckc_setperi(PERI_SDMMC1    , DISABLE,   10000);
#endif
	tca_ckc_setperi(PERI_SDMMC2    , DISABLE,   10000);
	tca_ckc_setperi(PERI_SDMMC3    , DISABLE,   10000);
	tca_ckc_setperi(PERI_ADAI1     , DISABLE,   10000);
	tca_ckc_setperi(PERI_ADAM1     , DISABLE,   10000);
	tca_ckc_setperi(PERI_SPDIF1    , DISABLE,   10000);
	tca_ckc_setperi(PERI_ADAI0     , DISABLE,   10000);
	tca_ckc_setperi(PERI_ADAM0     , DISABLE,   10000);
#if !defined(CONFIG_CHIP_TCC8933S) && !defined(CONFIG_CHIP_TCC8935S) && !defined(CONFIG_CHIP_TCC8937S)
	tca_ckc_setperi(PERI_SPDIF0    , DISABLE,   10000);
#endif
	tca_ckc_setperi(PERI_PDM       , DISABLE,   10000);
	tca_ckc_setperi(PERI_RESERVED32, DISABLE,   10000);
	tca_ckc_setperi(PERI_ADC       ,  ENABLE,  120000);
	tca_ckc_setperi(PERI_I2C0      ,  ENABLE,   40000);
	tca_ckc_setperi(PERI_I2C1      , DISABLE,   40000);
	tca_ckc_setperi(PERI_I2C2      , DISABLE,   40000);
	tca_ckc_setperi(PERI_I2C3      , DISABLE,   40000);
	tca_ckc_setperi(PERI_UART0     ,  ENABLE,  480000);
	tca_ckc_setperi(PERI_UART1     , DISABLE,   10000);
	tca_ckc_setperi(PERI_UART2     , DISABLE,   10000);
	tca_ckc_setperi(PERI_UART3     , DISABLE,   10000);
#if !defined(CONFIG_CHIP_TCC8933S) && !defined(CONFIG_CHIP_TCC8935S) && !defined(CONFIG_CHIP_TCC8937S)
	tca_ckc_setperi(PERI_UART4     , DISABLE,   10000);
	tca_ckc_setperi(PERI_UART5     , DISABLE,   10000);
	tca_ckc_setperi(PERI_UART6     , DISABLE,   10000);
	tca_ckc_setperi(PERI_UART7     , DISABLE,   10000);
#endif
	tca_ckc_setperi(PERI_GPSB0     , DISABLE,   10000);
	tca_ckc_setperi(PERI_GPSB1     , DISABLE,   10000);
	tca_ckc_setperi(PERI_GPSB2     , DISABLE,   10000);
#if !defined(CONFIG_CHIP_TCC8933S) && !defined(CONFIG_CHIP_TCC8935S) && !defined(CONFIG_CHIP_TCC8937S)
	tca_ckc_setperi(PERI_GPSB3     , DISABLE,   10000);
	tca_ckc_setperi(PERI_GPSB4     , DISABLE,   10000);
	tca_ckc_setperi(PERI_GPSB5     , DISABLE,   10000);
	tca_ckc_setperi(PERI_CAN0      , DISABLE,   10000);
	tca_ckc_setperi(PERI_CAN1      , DISABLE,   10000);
	tca_ckc_setperi(PERI_OUT2      , DISABLE,   10000);
	tca_ckc_setperi(PERI_OUT3      , DISABLE,   10000);
	tca_ckc_setperi(PERI_OUT4      , DISABLE,   10000);
	tca_ckc_setperi(PERI_OUT5      , DISABLE,   10000);
	tca_ckc_setperi(PERI_TSTX0     , DISABLE,   10000);
	tca_ckc_setperi(PERI_TSTX1     , DISABLE,   10000);
	//tca_ckc_setperi(PERI_PKTGEN0   , DISABLE,   10000);
	//tca_ckc_setperi(PERI_PKTGEN1   , DISABLE,   10000);
	//tca_ckc_setperi(PERI_PKTGEN2   , DISABLE,   10000);
	//tca_ckc_setperi(PERI_PKTGEN3   , DISABLE,   10000);
#endif
}
