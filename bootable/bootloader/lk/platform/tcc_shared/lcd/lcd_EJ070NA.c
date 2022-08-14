 /****************************************************************************
 *   FileName    : lcd_EJ070NA.c
 *   Description : support for EJ070NA LVDS Panel
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall constitute any express or implied warranty of any kind, including without limitation, any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright or other third party intellectual property right. No warranty is made, express or implied, regarding the information's accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between Telechips and Company.
*

****************************************************************************/

#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <tcc_lcd.h>


#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <reg.h>
#include <platform/iomap.h>
#include <dev/fbcon.h>

#ifdef EJ070NA
#include <dev/gpio.h>
#include <platform/gpio.h>

#include <platform/globals.h>
#include <tnftl/IO_TCCXXX.h>
#include "vioc/vioc_outcfg.h"

#define     LVDS_VCO_45MHz        450000
#define     LVDS_VCO_60MHz        600000

extern void tca_ckc_setpmupwroff( unsigned int periname , unsigned int isenable);

unsigned int lvds_stbyb;

#if defined(TCC_MIPI_USE)

#include <dev_mipi_dsi.h>
#include <i2c.h>

#define SLAVE_ADDR_SN65DSI83			0x58
unsigned int SN65DSI83_i2c_ch = I2C_CH_MASTER1;

static int SN65DSI83_read(unsigned char cmd, unsigned char *data)
{
	int ret;
	ret = i2c_xfer(SLAVE_ADDR_SN65DSI83, 1, &cmd, 1, data, SN65DSI83_i2c_ch);
	
	if (ret)  
		printf("%s: read failed.  cmd:0x%x, data:0x%x, result:0x%x\n", __func__, cmd, *data, ret);

	return ret;
}

static int SN65DSI83_write(unsigned char cmd, unsigned char value)
{
	unsigned char data= 0x00;

	unsigned char send_data[2];
	int ret;
	send_data[0] = cmd;
	send_data[1] = value;
	ret = i2c_xfer(SLAVE_ADDR_SN65DSI83, 2, send_data, 0, 0, SN65DSI83_i2c_ch);
	
	SN65DSI83_read(cmd, &data);
	printf("%s: write  cmd:0x%x, write :0x%x  read: 0x%x\n", __func__, cmd, value, data);
	return ret;
}

void SN65DSI83_init(void)
{
	unsigned char i;
	unsigned char data= 0x00;

	for(i = 0; i <=0x8; i++)
	{	
		SN65DSI83_read(i, &data);
		printf("%s:  cmd:0x%x, result:0x%x\n", __func__, i, data);
	}

	SN65DSI83_write(0x09, 0x00);
	SN65DSI83_write(0x0A, 0x03);
	SN65DSI83_write(0x0B, 0x18);
	SN65DSI83_write(0x0D, 0x00);
	SN65DSI83_write(0x10, 0x26);
	SN65DSI83_write(0x11, 0x00);
	SN65DSI83_write(0x12, 0x2a);
	SN65DSI83_write(0x13, 0x00);
	SN65DSI83_write(0x18, 0x78);
	SN65DSI83_write(0x19, 0x00);
	SN65DSI83_write(0x1A, 0x03);
	SN65DSI83_write(0x1B, 0x00);

	SN65DSI83_write(0x20, 0x00);
	SN65DSI83_write(0x21, 0x04);
	SN65DSI83_write(0x22, 0x00);
	SN65DSI83_write(0x23, 0x00);
	SN65DSI83_write(0x24, 0x00);
	SN65DSI83_write(0x25, 0x00);
	SN65DSI83_write(0x26, 0x00);
	SN65DSI83_write(0x27, 0x00);
	SN65DSI83_write(0x28, 0x20);
	SN65DSI83_write(0x29, 0x00); 
	SN65DSI83_write(0x2A, 0x00);
	SN65DSI83_write(0x2B, 0x00);
	SN65DSI83_write(0x2C, 0x14);
	SN65DSI83_write(0x2D, 0x00);
	SN65DSI83_write(0x2E, 0x00);
	SN65DSI83_write(0x2F, 0x00);
	SN65DSI83_write(0x30, 0x02);
	SN65DSI83_write(0x31, 0x00);
	SN65DSI83_write(0x32, 0x00);
	SN65DSI83_write(0x33, 0x00);
	SN65DSI83_write(0x34, 0x96);
	SN65DSI83_write(0x35, 0x00);
	SN65DSI83_write(0x36, 0x00);
	SN65DSI83_write(0x37, 0x00);
	SN65DSI83_write(0x38, 0x00);
	SN65DSI83_write(0x39, 0x00);
	SN65DSI83_write(0x3A, 0x00);
	SN65DSI83_write(0x3B, 0x00);
	SN65DSI83_write(0x3C, 0x00);
	SN65DSI83_write(0x3D, 0x00);
	SN65DSI83_write(0x3E, 0x00);

}

void sn65dsi83_init(void)
{
	unsigned char data= 0x00;
	printf("%s \n");
	SN65DSI83_init();

	SN65DSI83_write(0x0D, 0x1);

	SN65DSI83_read(0x0A, &data);
	
//	while(!(data & 0x80))
	{
		SN65DSI83_read(0x0A, &data);
	}


	SN65DSI83_read(0xE1, &data);
	printf("cmd:0x%x, result:0x%x \n", 0xE1, data);

	SN65DSI83_read(0xE5, &data);
	printf(" cmd:0x%x, result:0x%x \n", 0xE5, data);
}

static void MIPI_iniitialize(struct lcd_panel *lcd_spec)
{
	#define sim_debug(x) printf("sim %s \n", x)
	#define sim_value(x) 	printf("sim  0x%x \n", x)

	volatile unsigned int cnt;


	#if defined(TCC88XX) || defined(TCC892X) || defined(TCC893X)
	volatile PDDICONFIG		pDDICfg 	= (DDICONFIG *)HwDDI_CONFIG_BASE;
	#else
	volatile PDDICONFIG		pDDICfg 	= (DDICONFIG *)&HwDDI_CONFIG_BASE;
	#endif//
	
	volatile MIPI_DSI     *pHwMIPI_DSI = (MIPI_DSI *)HwMIPI_DSI;


	// MIPI PHY Reset
	pDDICfg->uMIPICFG.bReg.RESETN = 0; // MIPI PHY Reset
	for(cnt=0;cnt<0x10;cnt++);
	pDDICfg->uMIPICFG.bReg.RESETN = 1;

	VIOC_OUTCFG_SetOutConfig (VIOC_OUTCFG_MRGB, 0);

    // MIPI PHY PLL Setting
	pHwMIPI_DSI->PLLTMR      = 0x0800;

	#define TCC420
	//#define TCC320
	//#define TCC154

	#ifdef TCC420
	pHwMIPI_DSI->PHYACCHR     = (4<<5) | (1<<14); 

	pHwMIPI_DSI->PLLCTRL     = (0x0<<28)     // HSzeroCtl
				                        | (0x7<<24)     // FreqBand
				                        | (0x1<<23)     // PllEn
				                        | (0x0<<20)     // PREPRCtl
				                        | (  2<<13)     // P
				                        | (70<< 4)     // M
				                        | (  1<< 1);    // S

	#elif defined(TCC320)
	pHwMIPI_DSI->PHYACCHR     = (3<<5) | (1<<14); 

	pHwMIPI_DSI->PLLCTRL     = (0x0<<28)     // HSzeroCtl
				                        | (0x5<<24)     // FreqBand
				                        | (0x1<<23)     // PllEn
				                        | (0x0<<20)     // PREPRCtl
				                        | (  3<<13)     // P
				                        | (77<< 4)     // M
				                        | (  1<< 1);    // S

	#elif defined(TCC154)
	pHwMIPI_DSI->PHYACCHR     = (3<<5) | (1<<14); 

	pHwMIPI_DSI->PLLCTRL     = (0x0<<28)     // HSzeroCtl
				                        | (0x5<<24)     // FreqBand
				                        | (0x1<<23)     // PllEn
				                        | (0x0<<20)     // PREPRCtl
				                        | (  3<<13)     // P
				                        | (77<< 4)     // M
				                        | (  2<< 1);    // S

	#else
		#error not define PMS frequency
	#endif//

	pHwMIPI_DSI->ESCMODE     = (0x0<<21)     // STOPstate_cnt
	                    | (0x1<<20)     // ForceStopstate
	                    | (0x0<<16)     // ForceBta
	                    | (0x0<< 7)     // CmdLpdt
	                    | (0x0<< 6)     // TxLpdt
	                    | (0x0<< 4)     // TxTriggerRst
	                    | (0x0<< 3)     // TxUlpsDat
	                    | (0x0<< 2)     // TxUlpsExit
	                    | (0x0<< 1)     // TxUlpsClk
	                    | (0x0<< 0);    // TxUlpsClkExit

		lcd_delay_us(200000);

    pHwMIPI_DSI->ESCMODE     = (0x0<<21)     // STOPstate_cnt
                            | (0x0<<20)     // ForceStopstate
                            | (0x0<<16)     // ForceBta
                            | (0x0<< 7)     // CmdLpdt
                            | (0x0<< 6)     // TxLpdt
                            | (0x0<< 4)     // TxTriggerRst
                            | (0x0<< 3)     // TxUlpsDat
                            | (0x0<< 2)     // TxUlpsExit
                            | (0x0<< 1)     // TxUlpsClk
                            | (0x0<< 0);    // TxUlpsClkExit
		lcd_delay_us(200000);

    pHwMIPI_DSI->CLKCTRL     = (0x1<<31)     // TxRequestHsClk
                            | (0x1<<28)     // EscClkEn
                            | (0x0<<27)     // PLLBypass
                            | (0x0<<25)     // ByteClkSrc
                            | (0x1<<24)     // ByteClkEn
                            | (0x1<<23)     // LaneEscClkEn[4] - Data lane 3
                            | (0x1<<22)     // LaneEscClkEn[3] - Data lane 2
                            | (0x1<<21)     // LaneEscClkEn[2] - Data lane 1
                            | (0x1<<20)     // LaneEscClkEn[1] - Data lane 0
                            | (0x1<<19)     // LaneEscClkEn[0] - Clock lane
                            | (0x1<< 0);    // EscPrescaler
		lcd_delay_us(200000);

    pHwMIPI_DSI->MDRESOL     = (0x1<<31)     // MainStandby
                            | (  (lcd_spec->yres) <<16)     // MainVResol
                            | (  (lcd_spec->xres)<< 0);    // MainHResol

    pHwMIPI_DSI->MVPORCH     = (0x0<<28)     // CmdAllow
                            | (  ((lcd_spec->fewc1) + 1)<<16)     // StableVfp
                            | (  ((lcd_spec->fswc1) + 1)<< 0);    // MainVbp

    pHwMIPI_DSI->MHPORCH     = (  ((lcd_spec->lewc) + 1)<<16)     // MainHfp
                            | (  ((lcd_spec->lswc) + 1)<< 0);    // MainHbp

    // MBLANK ?

    pHwMIPI_DSI->MSYNC       = (  ((lcd_spec->fpw1) + 1)<<22)     // MainVsa
                            | (  ((lcd_spec->lpw) + 1)<< 0);    // MainHsa

    pHwMIPI_DSI->CONFIG      = (0x0<<28)     // EoT_r03
                            | (0x0<<27)     // SyncInform
                            | (0x1<<26)     // BurstMode
                            | (0x1<<25)     // VideoMode
                            | (0x0<<24)     // AutoMode
                            | (0x0<<23)     // HseMode
                            | (0x0<<22)     // HfpMode
                            | (0x0<<21)     // HbpMode
                            | (0x0<<20)     // HsaMode
                            | (0x0<<18)     // MainVc
                            | (0x1<<16)     // SubVc
                            | (0x7<<12)     // MainPixFormat
                            | (0x7<< 8)     // SubPixFormat
                            | (0x3<< 5)     // NumOfDataLane
                            | (0x1<< 4)     // LaneEn[4] - data lane 3 enabler
                            | (0x1<< 3)     // LaneEn[3] - data lane 2 enabler
                            | (0x1<< 2)     // LaneEn[2] - data lane 1 enabler
                            | (0x1<< 1)     // LaneEn[1] - data lane 0 enabler
                            | (0x1<< 0);    // LaneEn[0] - clock lane enabler


    //pHwMIPI_DSI->ESCMODE &= ~(0x1<<20); // ForceStopstate
    pHwMIPI_DSI->ESCMODE = 0;

    sim_debug("wait for NOT Stopstate");
    while ( ((pHwMIPI_DSI->STATUS>>8)&0x1) != 0x0 ); // wait for StopstateClk = 0
    //while ( ((pHwMIPI_DSI->STATUS>>0)&0xf) != 0x0 ); // wait for StopstateDat = 0
    sim_value(pHwMIPI_DSI->STATUS);


    // enter ulps mode
    pHwMIPI_DSI->ESCMODE     = (0x0<<21)     // STOPstate_cnt
                            | (0x0<<20)     // ForceStopstate
                            | (0x0<<16)     // ForceBta
                            | (0x0<< 7)     // CmdLpdt
                            | (0x0<< 6)     // TxLpdt
                            | (0x0<< 4)     // TxTriggerRst
                            | (0x1<< 3)     // TxUlpsDat
                            | (0x0<< 2)     // TxUlpsExit
                            | (0x1<< 1)     // TxUlpsClk
                            | (0x0<< 0);    // TxUlpsClkExit

    sim_debug("wait for ULPS");
    while ( ((pHwMIPI_DSI->STATUS>>9)&0x1) != 0x1 ); // wait for UlpsClk = 1
    while ( ((pHwMIPI_DSI->STATUS>>4)&0xf) != 0xf ); // wait for UlpsDat = 0xf
    sim_value(pHwMIPI_DSI->STATUS);
    // exit ulps mode
    pHwMIPI_DSI->ESCMODE     = (0x0<<21)     // STOPstate_cnt
                            | (0x0<<20)     // ForceStopstate
                            | (0x0<<16)     // ForceBta
                            | (0x0<< 7)     // CmdLpdt
                            | (0x0<< 6)     // TxLpdt
                            | (0x0<< 4)     // TxTriggerRst
                            | (0x1<< 3)     // TxUlpsDat
                            | (0x1<< 2)     // TxUlpsExit
                            | (0x1<< 1)     // TxUlpsClk
                            | (0x1<< 0);    // TxUlpsClkExit
		lcd_delay_us(200000);
    sim_debug("wait for NOT ULPS");
    while ( ((pHwMIPI_DSI->STATUS>>9)&0x1) != 0x0 ); // wait for UlpsClk = 0
    while ( ((pHwMIPI_DSI->STATUS>>4)&0xf) != 0x0 ); // wait for UlpsDat = 0
    sim_value(pHwMIPI_DSI->STATUS);
		lcd_delay_us(200000);

    pHwMIPI_DSI->ESCMODE     = (0x0<<21)     // STOPstate_cnt
                            | (0x0<<20)     // ForceStopstate
                            | (0x0<<16)     // ForceBta
                            | (0x0<< 7)     // CmdLpdt
                            | (0x0<< 6)     // TxLpdt
                            | (0x0<< 4)     // TxTriggerRst
                            | (0x1<< 3)     // TxUlpsDat
                            | (0x0<< 2)     // TxUlpsExit
                            | (0x1<< 1)     // TxUlpsClk
                            | (0x0<< 0);    // TxUlpsClkExit
    // wait 1ms?
		lcd_delay_us(200000);
    pHwMIPI_DSI->ESCMODE     = (0x0<<21)     // STOPstate_cnt
                            | (0x0<<20)     // ForceStopstate
                            | (0x0<<16)     // ForceBta
                            | (0x0<< 7)     // CmdLpdt
                            | (0x0<< 6)     // TxLpdt
                            | (0x0<< 4)     // TxTriggerRst
                            | (0x0<< 3)     // TxUlpsDat
                            | (0x0<< 2)     // TxUlpsExit
                            | (0x0<< 1)     // TxUlpsClk
                            | (0x0<< 0);    // TxUlpsClkExit
		lcd_delay_us(200000);

	sn65dsi83_init();		
		
}
#endif//

static int ej070na_panel_init(struct lcd_panel *panel)
{
	struct lcd_platform_data *pdata = &(panel->dev);

#if defined(TCC892X) || defined(TCC893X)
	if(pdata->lcdc_num ==1)
		lvds_stbyb = (GPIO_PORTE |27);
	else
		lvds_stbyb = (GPIO_PORTB |19);
#endif//

	tcclcd_gpio_config(pdata->display_on, GPIO_OUTPUT);
	tcclcd_gpio_config(pdata->bl_on, GPIO_OUTPUT);
	tcclcd_gpio_config(pdata->reset, GPIO_OUTPUT);
	tcclcd_gpio_config(pdata->power_on, GPIO_OUTPUT);
	tcclcd_gpio_config(lvds_stbyb, GPIO_OUTPUT);

	tcclcd_gpio_set_value(pdata->display_on, 0);
	tcclcd_gpio_set_value(pdata->bl_on, 0);
	tcclcd_gpio_set_value(pdata->reset, 0);
	tcclcd_gpio_set_value(pdata->power_on, 0);
	tcclcd_gpio_config(lvds_stbyb, 0);

	printf("%s : %d\n", __func__, 0);

	return 0;
}

static int ej070na_set_power(struct lcd_panel *panel, int on)
{
	unsigned int P, M, S, VSEL, TC;
	#if defined(TCC88XX) || defined(TCC892X) || defined(TCC893X)
	PDDICONFIG		pDDICfg 	= (DDICONFIG *)HwDDI_CONFIG_BASE;
	#else
	PDDICONFIG		pDDICfg 	= (DDICONFIG *)&HwDDI_CONFIG_BASE;
	#endif//
	struct lcd_platform_data *pdata = &(panel->dev);
	printf("%s onoff:%d  lcdc_num:%d \n", __func__, on, pdata->lcdc_num);

	if (on) {
		tcclcd_gpio_set_value(pdata->power_on, 1);
		lcd_delay_us(20);
		
		tcclcd_gpio_set_value(lvds_stbyb, 1);
		tcclcd_gpio_set_value(pdata->reset, 1);
		tcclcd_gpio_set_value(pdata->display_on, 1);

		

		lcdc_initialize(pdata->lcdc_num, panel);

		#if defined(TCC_MIPI_USE)
		MIPI_iniitialize(panel);
		#endif
		
		//LVDS 6bit setting for internal dithering option !!!
		//tcc_lcdc_dithering_setting(pdata->lcdc_num);
		
	#if defined(TCC892X) || defined(TCC893X)
		// LVDS reset	
		pDDICfg->LVDS_CTRL.bREG.RST =1;
		pDDICfg->LVDS_CTRL.bREG.RST =0;		

		#if 0
		BITCSET(pDDICfg->LVDS_TXO_SEL0.nREG, 0xFFFFFFFF, 0x15141312);
		BITCSET(pDDICfg->LVDS_TXO_SEL1.nREG, 0xFFFFFFFF, 0x0B0A1716);
		BITCSET(pDDICfg->LVDS_TXO_SEL2.nREG, 0xFFFFFFFF, 0x0F0E0D0C);
		BITCSET(pDDICfg->LVDS_TXO_SEL3.nREG, 0xFFFFFFFF, 0x05040302);
		BITCSET(pDDICfg->LVDS_TXO_SEL4.nREG, 0xFFFFFFFF, 0x1A190706);
		BITCSET(pDDICfg->LVDS_TXO_SEL5.nREG, 0xFFFFFFFF, 0x1F1E1F18);
		BITCSET(pDDICfg->LVDS_TXO_SEL6.nREG, 0xFFFFFFFF, 0x1F1E1F1E);
		BITCSET(pDDICfg->LVDS_TXO_SEL7.nREG, 0xFFFFFFFF, 0x1F1E1F1E);
		BITCSET(pDDICfg->LVDS_TXO_SEL8.nREG, 0xFFFFFFFF, 0x001E1F1E);
		#else
		BITCSET(pDDICfg->LVDS_TXO_SEL0.nREG, 0xFFFFFFFF, 0x13121110);
		BITCSET(pDDICfg->LVDS_TXO_SEL1.nREG, 0xFFFFFFFF, 0x09081514);
		BITCSET(pDDICfg->LVDS_TXO_SEL2.nREG, 0xFFFFFFFF, 0x0D0C0B0A);
		BITCSET(pDDICfg->LVDS_TXO_SEL3.nREG, 0xFFFFFFFF, 0x03020100);		
		BITCSET(pDDICfg->LVDS_TXO_SEL4.nREG, 0xFFFFFFFF, 0x1A190504);
		BITCSET(pDDICfg->LVDS_TXO_SEL5.nREG, 0xFFFFFFFF, 0x0E171618);
		BITCSET(pDDICfg->LVDS_TXO_SEL6.nREG, 0xFFFFFFFF, 0x1F07060F);
		BITCSET(pDDICfg->LVDS_TXO_SEL7.nREG, 0xFFFFFFFF, 0x1F1E1F1E);
		BITCSET(pDDICfg->LVDS_TXO_SEL8.nREG, 0xFFFFFFFF, 0x1F1E1F1E);
		#endif//

		
            #if defined(TCC893X)
            if( panel->clk_freq >= LVDS_VCO_45MHz && panel->clk_freq < LVDS_VCO_60MHz)
            {
                   M = 7; P = 7; S = 0; VSEL = 0; TC = 4;
            }
            else
            {
              	M = 10; P = 10; S = 0; VSEL = 0; TC = 4;		
            }
            #else
            		M = 10; P = 10; S = 0; VSEL = 0; TC = 4;		
            #endif
            
		BITCSET(pDDICfg->LVDS_CTRL.nREG, 0x00FFFFF0, (VSEL<<4)|(S<<5)|(M<<8)|(P<<15)|(TC<<21)); //LCDC1

		// LVDS Select LCDC 1
		if(pdata->lcdc_num ==1)
			BITCSET(pDDICfg->LVDS_CTRL.nREG, 0x3 << 30 , 0x1 << 30);
		else
			BITCSET(pDDICfg->LVDS_CTRL.nREG, 0x3 << 30 , 0x0 << 30);

	    	pDDICfg->LVDS_CTRL.bREG.RST = 1;	//  reset
	    		
	    	// LVDS enable
	  	pDDICfg->LVDS_CTRL.bREG.EN = 1;   // enable
	  	
	#else	
		// LVDS power on
		tca_ckc_setpmupwroff(PMU_LVDSPHY, ENABLE);

		pDDICfg->LVDS_TXO_SEL0 = 0x15141312; // SEL_03, SEL_02, SEL_01, SEL_00,
		pDDICfg->LVDS_TXO_SEL1 = 0x0B0A1716; // SEL_07, SEL_06, SEL_05, SEL_04,
		pDDICfg->LVDS_TXO_SEL2 = 0x0F0E0D0C; // SEL_11, SEL_10, SEL_09, SEL_08,
		pDDICfg->LVDS_TXO_SEL3 = 0x05040302; // SEL_15, SEL_14, SEL_13, SEL_12,
		pDDICfg->LVDS_TXO_SEL4 = 0x1A190706; // SEL_19, SEL_18, SEL_17, SEL_16,
		pDDICfg->LVDS_TXO_SEL5 = 0x1F1E1F18; //                         SEL_20,
		pDDICfg->LVDS_TXO_SEL6 = 0x1F1E1F1E;
		pDDICfg->LVDS_TXO_SEL7 = 0x1F1E1F1E;
		pDDICfg->LVDS_TXO_SEL8 = 0x1F1E1F1E;

		// LVDS Select
	//	BITCLR(pDDICfg->LVDS_CTRL, Hw0); //LCDC0
		BITSET(pDDICfg->LVDS_CTRL, Hw0); //LCDC1


		#ifdef TCC88XX

		M = 10; P = 10; S = 0; VSEL = 0; TC = 4;		
		BITCSET(pDDICfg->LVDS_CTRL.nREG, 0x00FFFFF0, (VSEL<<4)|(S<<5)|(M<<8)|(P<<15)|(TC<<21)); //LCDC1


	    BITSET(pDDICfg->LVDS_CTRL, Hw1);    // reset
		#endif//
		
	    // LVDS enable
	    BITSET(pDDICfg->LVDS_CTRL, Hw2);    // enable
	#endif


	}
	else 	{
		tcclcd_gpio_set_value(pdata->power_on, 0);
	}

	
	return 0;
}

static int ej070na_set_backlight_level(struct lcd_panel *panel, int level)
{
	struct lcd_platform_data *pdata = &(panel->dev);

	printf("%s : %d\n", __func__, level);

	if (level == 0) {
		tcclcd_gpio_set_value(pdata->bl_on, 0);
	} else {
		tcclcd_gpio_set_value(pdata->bl_on, 1);
	}

	return 0;
}

static struct lcd_panel ej070na_panel = {
	.name		= "ej070na",
	.manufacturer	= "innolux",
	.id		= PANEL_ID_EJ070NA,
	.xres		= 1024,
	.yres		= 600,
	.width		= 153,
	.height		= 90,
	.bpp		= 24,
	.clk_freq	= 512000,
	.clk_div	= 2,
	.bus_width	= 24,
	
	.lpw		= 20,
	.lpc		= 1024,
	.lswc		= 150,
	.lewc		= 150,
	.vdb		= 0,
	.vdf		= 0,

	.fpw1		= 2,
	.flc1		= 600,
	.fswc1		= 10,
	.fewc1		= 23,
	
	.fpw2		= 2,
	.flc2		= 600,
	.fswc2		= 10,
	.fewc2		= 23,
#if defined(TCC_MIPI_USE)
	.sync_invert	= IP_INVERT,//IV_INVERT | IH_INVERT,
#else
	.sync_invert	= IV_INVERT | IH_INVERT,
#endif//
	.init		= ej070na_panel_init,
	.set_power	= ej070na_set_power,
	.set_backlight_level = ej070na_set_backlight_level,
};

struct lcd_panel *tccfb_get_panel(void)
{
	return &ej070na_panel;
}
#endif//ej070na
