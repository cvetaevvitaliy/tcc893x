/*
 * linux/arch/arm/mach-tcc893x/vioc_api.c
 * Author:  <linux@telechips.com>
 * Created: June 10, 2008
 * Description: TCC VIOC h/w block 
 *
 * Copyright (C) 2008-2009 Telechips
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see the file COPYING, or write
 * to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <linux/kernel.h>
#include <mach/bsp.h>
#include <mach/io.h>

#include <mach/vioc_api.h>

#include <mach/vioc_scaler.h>
#include <mach/vioc_rdma.h>
#include <mach/vioc_wdma.h>
#include <mach/vioc_wmix.h>
#include <mach/vioc_disp.h>
#include <mach/vioc_ireq.h>
#include <mach/vioc_config.h>
#include <mach/vioc_plugin_tcc893x.h>
#include <mach/vioc_outcfg.h>
#include <mach/vioc_lut.h>

/* Scaler Set */
/*
	API : VIOC_API_SCALER_Set/GetXXX()
*/
void VIOC_API_SCALER_SetConfig(unsigned int scaler, VIOC_SCALER_INFO_Type * Info)
{
	volatile PVIOC_SC pScaler;
	pScaler = (volatile PVIOC_SC)tcc_p2v((unsigned int)HwVIOC_SC0 + scaler * 0x100);

	VIOC_SC_SetBypass(pScaler, Info->BYPASS);
	//VIOC_SC_SetSrcSize((VIOC_SC *)baseAddr, Info->SRC_WIDTH, Info->SRC_HEIGHT);
	VIOC_SC_SetDstSize(pScaler, (Info->DST_WIDTH + 2), (Info->DST_HEIGHT + 2));
	VIOC_SC_SetOutPosition(pScaler, (Info->OUTPUT_POS_X + 1), (Info->OUTPUT_POS_Y + 1));
	VIOC_SC_SetOutSize(pScaler, Info->OUTPUT_WIDTH, Info->OUTPUT_HEIGHT);
}

void VIOC_API_SCALER_SetUpdate(unsigned int scaler)
{
	volatile PVIOC_SC pScaler;
	pScaler = (volatile PVIOC_SC)tcc_p2v((unsigned int)HwVIOC_SC0 + scaler * 0x100);

	VIOC_SC_SetUpdate(pScaler);
}

int VIOC_API_SCALER_SetInterruptEnable(unsigned int scaler, unsigned int interrupt)
{
	int ret = 0; // VIOC_DRIVER_NOERR;
	volatile PVIOC_SC pScaler;
	pScaler = (volatile PVIOC_SC)tcc_p2v((unsigned int)HwVIOC_SC0 + scaler * 0x100);

	if(scaler >= VIOC_SCALER_MAX)
	{
		ret = -3; // VIOC_DRIVER_ERR_INVALID;
	}
	else
	{
		VIOC_SC_SetInterruptMask(pScaler, interrupt, FALSE);
	}

	return ret;
}

int VIOC_API_SCALER_SetInterruptDiable(unsigned int scaler, unsigned int interrupt)
{
	int ret = 0; // VIOC_DRIVER_NOERR;
	volatile PVIOC_SC pScaler;
	pScaler = (volatile PVIOC_SC)tcc_p2v((unsigned int)HwVIOC_SC0 + scaler * 0x100);

	if(scaler >= VIOC_SCALER_MAX)
	{
		ret = -3; // VIOC_DRIVER_ERR_INVALID;
	}
	else
	{
		VIOC_SC_SetInterruptMask(pScaler, interrupt, TRUE);
	}

	return ret;
}

int VIOC_API_SCALER_SetPlugIn(unsigned int scaler, unsigned int path)
{
	int ret;

	ret = VIOC_CONFIG_PlugIn(scaler, path);
	if(ret == VIOC_DEVICE_CONNECTED)
		ret = VIOC_DRIVER_NOERR;
	else
		ret = VIOC_DRIVER_ERR;

	return ret;
}

int VIOC_API_SCALER_SetPlugOut(unsigned int scaler)
{
	int ret;

	ret = VIOC_CONFIG_PlugOut(scaler);
	if(ret == VIOC_PATH_DISCONNECTED)
		ret = VIOC_DRIVER_NOERR;
	else
		ret = VIOC_DRIVER_ERR;

	return ret;
}

void VIOC_API_IREQ_Init(void)
{
	VIOC_IREQ_RegisterHandle();
}



void vioc_display_device_reset(unsigned int device_num)
{
	#define DD0_DMA_CONNECT_CHECK(PlugInOut) 	((PlugInOut.connect_device == VIOC_SC_RDMA_00)|| (PlugInOut.connect_device == VIOC_SC_RDMA_01) \
													||(PlugInOut.connect_device == VIOC_SC_RDMA_02) || (PlugInOut.connect_device == VIOC_SC_RDMA_03) \
													|| (PlugInOut.connect_device == VIOC_SC_WDMA_00))
													
	#define DD1_DMA_CONNECT_CHECK(PlugInOut) 	((PlugInOut.connect_device == VIOC_SC_RDMA_04)|| (PlugInOut.connect_device == VIOC_SC_RDMA_05) \
													||(PlugInOut.connect_device == VIOC_SC_RDMA_06) || (PlugInOut.connect_device == VIOC_SC_RDMA_07)  \
													|| (PlugInOut.connect_device == VIOC_SC_WDMA_01))
	volatile PVIOC_IREQ_CONFIG pIREQConfig;

	volatile VIOC_DISP DISPBackup , *pDISPBackup;
	volatile VIOC_WMIX WMIXBackup;
      VIOC_WMIX * pWMIXBackup;
	volatile VIOC_RDMA RDMA0Backup, RDMA1Backup, RDMA2Backup, RDMA3Backup;
	VIOC_RDMA *pRDMA0Backup, *pRDMA1Backup, *pRDMA2Backup, *pRDMA3Backup;
	
	volatile unsigned int WMIXER_N, RDMA_0, RDMA_1, RDMA_2, RDMA_3;

	//RDMA check PlugInOut of VIOC plug in/out block 
	//volatile VIOC_PlugInOutCheck VIOC_PlugIn[VIOC_VIQE + 1];
	VIOC_PlugInOutCheck VIOC_PlugIn[VIOC_VIQE + 1];
	volatile unsigned int VIOC_PlugIn_reset[VIOC_VIQE + 1];
	volatile VIOC_SC SC0_Backup, SC1_Backup, SC2_Backup, SC3_Backup;
	volatile VIQE ViqeBackup;
	volatile int i;
	
	pIREQConfig = (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_IREQ);

	for(i = 0; i <= VIOC_VIQE; i++)
	{
		VIOC_PlugIn_reset[i] = false;		

		VIOC_CONFIG_Device_PlugState(i, &VIOC_PlugIn[i]);

		if(VIOC_PlugIn[i].enable)
		{
			if(device_num) {
				if(DD1_DMA_CONNECT_CHECK(VIOC_PlugIn[i]))
					VIOC_PlugIn_reset[i] = true;
			}
			else	{
				if(DD0_DMA_CONNECT_CHECK(VIOC_PlugIn[i]))
					VIOC_PlugIn_reset[i] = true;
			}
		}
	}
	
	if(device_num)	{
		pDISPBackup = (VIOC_DISP *)tcc_p2v(HwVIOC_DISP1);
		pWMIXBackup = (VIOC_WMIX *)tcc_p2v(HwVIOC_WMIX1);

		pRDMA0Backup = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA04);
		pRDMA1Backup = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA05);
		pRDMA2Backup = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA06);
		pRDMA3Backup = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA07);

		WMIXER_N = VIOC_WMIX1;

		RDMA_0 = VIOC_WMIX_RDMA_04;
		RDMA_1 = VIOC_WMIX_RDMA_05;
		RDMA_2 = VIOC_WMIX_RDMA_06;
		RDMA_3 = VIOC_WMIX_RDMA_07;
	}
	else 	{
		pDISPBackup = (VIOC_DISP *)tcc_p2v(HwVIOC_DISP0);
		pWMIXBackup = (VIOC_WMIX *)tcc_p2v(HwVIOC_WMIX0);
		
		pRDMA0Backup = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA00);
		pRDMA1Backup = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA01);
		pRDMA2Backup = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA02);
		pRDMA3Backup = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA03);

		WMIXER_N = VIOC_WMIX0;
		
		RDMA_0 = VIOC_WMIX_RDMA_00;
		RDMA_1 = VIOC_WMIX_RDMA_01;
		RDMA_2 = VIOC_WMIX_RDMA_02;
		RDMA_3 = VIOC_WMIX_RDMA_03;
	}

	// backup RDMA, WMIXER, DisplayDevice register
	RDMA0Backup = *pRDMA0Backup;
	RDMA1Backup = *pRDMA1Backup;
	RDMA2Backup = *pRDMA2Backup;
	RDMA3Backup = *pRDMA3Backup;

	DISPBackup = *pDISPBackup;
	
	WMIXBackup = *pWMIXBackup;

	if(VIOC_PlugIn_reset[VIOC_SC0])
		SC0_Backup = *(VIOC_SC *)tcc_p2v(HwVIOC_SC0);

	if(VIOC_PlugIn_reset[VIOC_SC1])
		SC1_Backup = *(VIOC_SC *)tcc_p2v(HwVIOC_SC1);

	if(VIOC_PlugIn_reset[VIOC_SC2])
		SC2_Backup = *(VIOC_SC *)tcc_p2v(HwVIOC_SC2);

	if(VIOC_PlugIn_reset[VIOC_SC3])
		SC3_Backup = *(VIOC_SC *)tcc_p2v(HwVIOC_SC3);

	if(VIOC_PlugIn_reset[VIOC_VIQE])
		ViqeBackup = *(VIQE *)tcc_p2v(HwVIOC_VIQE0_BASE);	
	
	// h/w block reset
	BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<(20+device_num)), (0x1<<(20+device_num))); // disp reset

	BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<(0+device_num)), (0x1<<(0+device_num))); // wdma reset

	BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<(9+WMIXER_N)), (0x1<<(9+WMIXER_N))); // wmix reset

	BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<RDMA_0), (0x1<<RDMA_0)); // rdma reset
	BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<RDMA_1), (0x1<<RDMA_1)); // rdma reset	
	BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<RDMA_2), (0x1<<RDMA_2)); // rdma reset
	BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<RDMA_3), (0x1<<RDMA_3)); // rdma reset


	if(VIOC_PlugIn_reset[VIOC_SC0])
		BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<(28+VIOC_SC0)), (0x1<<(28+VIOC_SC0))); // scaler 0 reset

	if(VIOC_PlugIn_reset[VIOC_SC1])
		BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<(28+VIOC_SC1)), (0x1<<(28+VIOC_SC1))); // scaler 1 reset

	if(VIOC_PlugIn_reset[VIOC_SC2])
		BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<(28+VIOC_SC2)), (0x1<<(28+VIOC_SC2))); // scaler 2 reset

	if(VIOC_PlugIn_reset[VIOC_SC3])
		BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<(28+VIOC_SC3)), (0x1<<(28+VIOC_SC3))); // scaler 3 reset

	if(VIOC_PlugIn_reset[VIOC_VIQE])
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<(16)), (0x1<<(16))); // VIQE reset

		
	// h/w block release reset 
	if(VIOC_PlugIn_reset[VIOC_VIQE])
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<(16)), (0x0<<(16))); // VIQE reset

	if(VIOC_PlugIn_reset[VIOC_SC3])
		BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<(28+VIOC_SC3)), (0x0<<(28+VIOC_SC3))); // scaler 3 reset

	if(VIOC_PlugIn_reset[VIOC_SC2])
		BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<(28+VIOC_SC2)), (0x0<<(28+VIOC_SC2))); // scaler 2 reset

	if(VIOC_PlugIn_reset[VIOC_SC1])
		BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<(28+VIOC_SC1)), (0x0<<(28+VIOC_SC1))); // scaler 1 reset

	if(VIOC_PlugIn_reset[VIOC_SC0])
		BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<(28+VIOC_SC0)), (0x0<<(28+VIOC_SC0))); // scaler 0 reset
	
	BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<RDMA_0), (0x0<<RDMA_0)); // rdma reset release
	BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<RDMA_1), (0x0<<RDMA_1)); // rdma reset	
	BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<RDMA_2), (0x0<<RDMA_2)); // rdma reset
	BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<RDMA_3), (0x0<<RDMA_3)); // rdma reset 
	
	BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<(9+WMIXER_N)), (0x0<<(9+WMIXER_N))); // wmix reset release

	BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<(0+device_num)), (0x0<<(0+device_num))); // wdma reset release

	BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<(0+device_num)), (0x0<<(0+device_num))); // wdma reset release

	BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<(20+device_num)), (0x0<<(20+device_num))); // disp reset release


	// restore VIQE, SCALER , RDMA, WMIXER, DisplayDevice register
	if(VIOC_PlugIn_reset[VIOC_VIQE]) {
		*(VIQE *)tcc_p2v(HwVIOC_VIQE0_BASE) = ViqeBackup;
	}

	if(VIOC_PlugIn_reset[VIOC_SC3]) {
		*(VIOC_SC *)tcc_p2v(HwVIOC_SC3) = SC3_Backup;
		VIOC_API_SCALER_SetUpdate(VIOC_SC3); // scaler 3 update
	}

	if(VIOC_PlugIn_reset[VIOC_SC2]) {
		*(VIOC_SC *)tcc_p2v(HwVIOC_SC2) = SC2_Backup;
		VIOC_API_SCALER_SetUpdate(VIOC_SC2); // scaler 2 update		
	}
	
	if(VIOC_PlugIn_reset[VIOC_SC1]) {
		*(VIOC_SC *)tcc_p2v(HwVIOC_SC1) = SC1_Backup;
		VIOC_API_SCALER_SetUpdate(VIOC_SC1); // scaler 1 update
	}

	if(VIOC_PlugIn_reset[VIOC_SC0]) {
		*(VIOC_SC *)tcc_p2v(HwVIOC_SC0) = SC0_Backup;
		VIOC_API_SCALER_SetUpdate(VIOC_SC0); // scaler 0 update
	}
		
	*pRDMA0Backup = RDMA0Backup;
	*pRDMA1Backup = RDMA1Backup;
	*pRDMA2Backup = RDMA2Backup;
	*pRDMA3Backup = RDMA3Backup;

	*pWMIXBackup = WMIXBackup;

	VIOC_RDMA_SetImageUpdate(pRDMA0Backup);
	VIOC_RDMA_SetImageUpdate(pRDMA1Backup);
	VIOC_RDMA_SetImageUpdate(pRDMA2Backup);
	VIOC_RDMA_SetImageUpdate(pRDMA3Backup);

	VIOC_WMIX_SetUpdate(pWMIXBackup);

	*pDISPBackup = DISPBackup;
	printk("VIOC_sc0:%d VIOC_sc1:%d VIOC_sc2:%d ", VIOC_PlugIn_reset[0], VIOC_PlugIn_reset[1], VIOC_PlugIn_reset[2]);
    	printk(" VIOC_sc3:%d VIOC_VIQE:%d  \n", VIOC_PlugIn_reset[3], VIOC_PlugIn_reset[4]);
}


int VIOC_API_WMIX_set_layer_order(unsigned int num, unsigned int order)
{
	VIOC_WMIX * pWMIX;
	unsigned int old_layer_order = 0;

	pWMIX = (VIOC_WMIX *)tcc_p2v((num == 0)?HwVIOC_WMIX0:HwVIOC_WMIX1); 

	printk("[%s] disp_num:%d, pWMIX:%p, ovp(%d -> %d), \n", 
		__func__, num, pWMIX, old_layer_order, order);	

	VIOC_WMIX_SetOverlayPriority(pWMIX, order);

	//BITCSET(pWMIX->uCCON0.nREG, 0xFF000000, 0x20000000);	//setting CCON value for alphablending

	VIOC_WMIX_SetUpdate(pWMIX);

	return 1;	
}

int VIOC_API_LUT_set_brightness(unsigned int lcdc_num, lut_ctrl_params *LutInfo)
{

	static VIOC_LUT * pLUTBase;
	unsigned int LUT_LCD_NUM;
	unsigned int LUT_LCD_VIDEO_CH;

	pLUTBase = (VIOC_LUT*)tcc_p2v(HwVIOC_LUT);

	if(lcdc_num)
	{
		LUT_LCD_NUM = VIOC_LUT_COMP1;
		LUT_LCD_VIDEO_CH = VIOC_LUT_RDMA_07;

	}
	else
	{
		LUT_LCD_NUM = VIOC_LUT_COMP0;
		LUT_LCD_VIDEO_CH = VIOC_LUT_RDMA_03;
	}	

	if(LutInfo->onoff)
		VIOC_LUT_SetMain(pLUTBase , LUT_LCD_NUM, LUT_LCD_VIDEO_CH, LUT_BRIGHTNESS, LutInfo->brightness, LutInfo->onoff);
	else
		VIOC_LUT_SetMain(pLUTBase , LUT_LCD_NUM, LUT_LCD_VIDEO_CH, LUT_NORMAL, LutInfo->brightness, LutInfo->onoff);

        return 0;

}

int VIOC_API_LUT_set_DVI(unsigned int lcdc_num, lut_ctrl_params *LutInfo)
{

	static VIOC_LUT * pLUTBase;
	unsigned int LUT_LCD_NUM;
	unsigned int LUT_LCD_VIDEO_CH;

	pLUTBase = (VIOC_LUT*)tcc_p2v(HwVIOC_LUT);

	if(lcdc_num)
	{
		LUT_LCD_NUM = VIOC_LUT_DEV1;
		LUT_LCD_VIDEO_CH = VIOC_LUT_DEV1;
	}
	else
	{
		LUT_LCD_NUM = VIOC_LUT_DEV0;
		LUT_LCD_VIDEO_CH = VIOC_LUT_DEV0;
	}	

	if(LutInfo->onoff)
		VIOC_LUT_SetMain(pLUTBase , LUT_LCD_NUM, 0, LUT_DVI, 0, LutInfo->onoff);
	else
		VIOC_LUT_SetMain(pLUTBase , LUT_LCD_NUM, 0, LUT_NORMAL, 0, LutInfo->onoff);

	return 0;
}