/****************************************************************************
FileName    : kernel/drivers/video/tcc/vioc/tcc_vioc_manager.c
Description : 

Copyright (C) 2013 Telechips Inc.

This program is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA 02111-1307 USA
****************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/switch.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>

#include <asm/io.h>
#include <mach/bsp.h>
#include <mach/irqs.h>
#include <asm/mach-types.h>

#include <linux/kthread.h>  /* thread */
#include <linux/wait.h>
#include <linux/delay.h>    /* msleep_interruptible */

#if defined(CONFIG_ARCH_TCC892X)
#include <mach/vioc_plugin_tcc892x.h>
#endif
#if defined(CONFIG_ARCH_TCC893X)
#include <mach/vioc_plugin_tcc893x.h>
#endif
#include <mach/vioc_config.h>
#include <mach/tca_display_config.h>

#include <mach/tca_fb_output.h>

#include <mach/vioc_outcfg.h>
#include <mach/vioc_rdma.h>
#include <mach/vioc_wdma.h>
#include <mach/vioc_wmix.h>
#include <mach/vioc_disp.h>
#include <mach/vioc_config.h>
#include <mach/vioc_scaler.h>


/*---------------------------------------------------------------------------
 * Debug Option
 *---------------------------------------------------------------------------*/
#if defined(tcc_dbg)
#undef tcc_dbg(fmt,arg...)  
#endif

#if 1
#define tcc_dbg(fmt,arg...)  
#else
#define tcc_dbg(fmt,arg...)      printk(fmt,##arg)
#endif
/*---------------------------------------------------------------------------*/

//tern unsigned int tca_get_lcd_lcdc_num(viod);
//tern unsigned int tca_get_output_lcdc_num(viod);

extern unsigned int tccfb_output_get_mode(void);

extern void hdmi_start(void);
extern void hdmi_stop(void);

struct tcc_vioc_manager_data {
    struct task_struct *poll_task;
};

void TCC_OUTPUT_LCDC_Reset(void)
{
	unsigned int output_lcdc_num;
	unsigned int output_mode;
	volatile PVIOC_IREQ_CONFIG pIREQConfig;

	volatile VIOC_DISP DISPBackUp;
	volatile VIOC_WMIX WMIXBackUp;
	volatile VIQE  VIQEBackUp;
	volatile VIOC_RDMA RDMABackUp[8];
	volatile VIOC_SC SCBackUp[4];
 
	volatile VIOC_DISP *pDISPBase;
	volatile VIOC_WMIX *pWMIXBase;
	volatile VIOC_RDMA *pRDMABase[8];
	volatile VIOC_SC *pSCBase[4];

	volatile VIQE *pVIQEBase;

	VIOC_CONFIG_PATH_u *pConfigPath = NULL;
	unsigned int iPathSelect, idx;
	unsigned int sc_plug_in[4];
	unsigned int rdma_en[8];
	unsigned int viqe_plug_in;
	unsigned int deintl_s_plug_in;

	printk("%s, VIOC path is reset Start!!\n", __func__);

	//local_irq_disable();

	mdelay(40);

	pIREQConfig = (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_IREQ);

	output_lcdc_num = tca_get_output_lcdc_num();
	if(output_lcdc_num){
		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP1);
		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX1);
	} else {
		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP0);
		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX0);
	}

	DISPBackUp = *pDISPBase;
	WMIXBackUp = *pWMIXBase;

	if(output_lcdc_num){
		for(idx = 4; idx < 8; idx++) {
			pRDMABase[idx] = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA00 + (0x100 * idx));
			if( pRDMABase[idx]->uCTRL.bREG.IEN) {
				RDMABackUp[idx] = *pRDMABase[idx];
				rdma_en[idx] = 1;
				tcc_dbg("RDMA%d is enabled!!!\n", idx);
			} else {
				rdma_en[idx]= 0;
			}
		}
	} else {
		for(idx = 0; idx < 4; idx++) {
			pRDMABase[idx] = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA00 + (0x100 * idx));
			if( pRDMABase[idx]->uCTRL.bREG.IEN) {
				RDMABackUp[idx] = *pRDMABase[idx];
				rdma_en[idx] = 1;
				tcc_dbg("RDMA%d is enabled!!!\n", idx);
			} else {
				rdma_en[idx]= 0;
			}
		}
	}

	for(idx = 0; idx < 4; idx++) {
		pConfigPath = VIOC_CONFIG_GetPathStruct(VIOC_SC0 + idx);
		if(pConfigPath == NULL) {
			tcc_dbg("VIOC_SC%d is not plug-in!!!\n", idx);
			sc_plug_in[idx] = 0;
		}

		if(pConfigPath->nREG & 1<<31) {
			tcc_dbg("VIOC_SC%d is enabled!!!\n", idx);
			sc_plug_in[idx] = 1;
			pSCBase[idx] = (VIOC_SC*)tcc_p2v(HwVIOC_SC0 + (0x100 * idx));
			SCBackUp[idx] = *pSCBase[idx];
		} else {
			sc_plug_in[idx] = 0;
		}
		
		if( sc_plug_in[idx] ) {
			iPathSelect = (pConfigPath->nREG & 0xff);
			tcc_dbg("VIOC_SC%d path selection is %d \n", idx, iPathSelect);
			
		}
	}

	pConfigPath = VIOC_CONFIG_GetPathStruct(VIOC_VIQE);
	if(pConfigPath == NULL) {
		tcc_dbg("VIOC_VIQE is not plug-in!!!\n");
		viqe_plug_in = 0;
	}

	if(pConfigPath->nREG & 1<<31) {
		tcc_dbg("VIOC_VIQE is enabled!!!\n");
		viqe_plug_in = 1;
	} else {
		viqe_plug_in = 0;
	}
	
	if( viqe_plug_in ) {
		pVIQEBase = (VIQE*)tcc_p2v(HwVIOC_VIQE0);
		VIQEBackUp = *pVIQEBase;
 	}

	pConfigPath = VIOC_CONFIG_GetPathStruct(VIOC_DEINTLS);
	if(pConfigPath == NULL) {
		tcc_dbg("VIOC_DEINTLS is not plug-in!!!\n");
		deintl_s_plug_in = 0;
	}

	if(pConfigPath->nREG & 1<<31) {
		tcc_dbg("VIOC_DEINTLS is enabled!!!\n");
		deintl_s_plug_in = 1;
	} else {
		deintl_s_plug_in = 0;
	}

	{
		unsigned int idx = 0;

		volatile VIOC_RDMA *pRDMABase03;
		volatile VIQE *pVIQEBase_Temp;

		pRDMABase03 = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA03);
		pVIQEBase_Temp = (VIQE*)tcc_p2v(HwVIOC_VIQE0);

		printk("%s : RDMA03_CURADDR = ", __func__);
		for(idx = 0; idx < 10;idx++)
			printk("0x%08x, ",pRDMABase03->nCBASE);
		printk("\n");

		printk("%s VIQE_VCTRL = 0x%08x, VIQE_DI_CTRL = 0x%08x\n", __func__, (unsigned int)pVIQEBase_Temp->cVIQE_CTRL.nVIQE_CTRL.nREG, (unsigned int)pVIQEBase_Temp->cDEINTL_DMA.nDEINTL_CTRL.nREG);
	}

	output_mode = tccfb_output_get_mode();

	if(output_mode == TCC_OUTPUT_HDMI)
		hdmi_stop();

	VIOC_DISP_TurnOff((VIOC_DISP *)pDISPBase);
	printk("%s : VIOC_DISP_Wait_DisplayDone Start\n", __func__);
	VIOC_DISP_Wait_DisplayDone((VIOC_DISP *)pDISPBase);
	printk("%s : VIOC_DISP_Wait_DisplayDone End\n", __func__);
	
	if(output_lcdc_num){
		for(idx = 4; idx < 8; idx++) {
				VIOC_RDMA_SetImageDisable((VIOC_RDMA *)pRDMABase[idx]);
				tcc_dbg("VIOC_RDMA%d is disabled!!!\n", idx );
		}
	} else {
		for(idx = 0; idx < 4; idx++) {
				VIOC_RDMA_SetImageDisable((VIOC_RDMA *)pRDMABase[idx]);
				tcc_dbg("VIOC_RDMA%d is disabled!!!\n", idx );
		}
	}

	tcc_dbg("VIOC_DISP is reset!!!\n");

	printk("Clock Control(Clock Gating) Disable!!!\n");
	BITCSET(pIREQConfig->uCLKCTRL.nREG,(1<<8), (0<<8));		// Clock Control(Clock Gating) Disable
	printk("RDMA Auto Power Down Mode - Normal!!!\n");
	BITCSET(pIREQConfig->uAUTOPD.nREG,(1<<0), (0<<0));		// RDMA Auto Power Down Mode - Normal
	printk("WDMA Auto Power Down Mode - Normal!!!\n");
	BITCSET(pIREQConfig->uAUTOPD.nREG,(1<<3), (0<<3));		// WDMA Auto Power Down Mode - Normal

	if( output_lcdc_num ) {
		printk("DISP1 is reset!!!\n");
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<21), (1<<21));		// DISP1 reset
		printk("WMIX1 is reset!!!\n");
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<10), (1<<10));		// WMIX1 reset
		printk("WDMA1 is reset!!!\n");
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<1), (1<<1));		// WDMA1 reset
	} else {
		printk("DISP0 is reset!!!\n");
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<20), (1<<20));		// DISP0 reset
		printk("WMIX0 is reset!!!\n");
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<9), (1<<9));		// WMIX0 reset
		printk("WDMA0 is reset!!!\n");
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<0), (1<<0));		// WDMA0 reset
	}

	
	for( idx = 0; idx < 4; idx++) {
		if(sc_plug_in[idx]) {
			printk("SC%d is reset!!!\n", idx);
			BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<(28 + idx)), (1<<(28 + idx)));		// SCALER reset
		}
	}

	if( output_lcdc_num ) {	
		for( idx = 4; idx < 8; idx++) {
				printk("RDMA%d is reset!!!\n", idx);
				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<idx), (1<<idx));		// RDMA reset
		}
	} else {
		for( idx = 0; idx < 4; idx++) {
				printk("RDMA%d is reset!!!\n", idx);
				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<idx), (1<<idx));		// RDMA reset
		}
	}
	
	if( viqe_plug_in ) {
		printk("VIQE0 is reset!!!\n");
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<16), (1<<16));		// VIQE0 reset
	}

	if( deintl_s_plug_in) {
		printk("DEINTL_S is reset!!!\n");
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<17), (1<<17));		// DEINTL_S Reset
	}

	if( deintl_s_plug_in) {
		printk("DEINTL_S is reset clear!!!\n");
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<17), (0<<17));		// DEINTL_S Reset Clear
	}

	if( viqe_plug_in ) {
		printk("VIQE0  is reset clear!!!\n");
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<16), (0<<16));		// VIQE0 reset clear
	}

	if(output_lcdc_num){
		for( idx = 4; idx < 8; idx++) {
			printk("RDMA%d is reset clear!!!\n", idx);
				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<idx), (0<<idx));		// RDMA reset clear
		}
	} else {
		for( idx = 0; idx < 4; idx++) {
			printk("RDMA%d is reset clear!!!\n", idx);
				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<idx), (0<<idx));		// RDMA reset clear
		}
	}

	for( idx = 0; idx < 4; idx++) {
		if(sc_plug_in[idx]) {
			printk("SC%d is reset clear!!!\n", idx);
			BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<(28 + idx)), (0<<(28 + idx)));		// SCALER reset clear
		}
	}

	if( output_lcdc_num ) {
		printk("WMIX1 is reset clear!!!\n");
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<10), (0<<10));		// WMIX1 reset clear
		printk("DISP1 is reset clear!!!\n");
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<21), (0<<21));		// DISP1 reset clear
		printk("WDMA1 is reset clear!!!\n");
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<1), (0<<1));		// WDMA1 reset clear
	} else {
		printk("WMIX0  is reset clear!!!\n");
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<9), (0<<9));		// WMIX0 reset clear
		printk("DISP0  is reset clear!!!\n");
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<20), (0<<20));		// DISP0 reset clear
		printk("WDMA0  is reset clear!!!\n");
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<0), (0<<0));		// WDMA0 reset clear
	}

	printk("Clock Control(Clock Gating) Enable!!!\n");
	BITCSET(pIREQConfig->uCLKCTRL.nREG,(1<<8), (1<<8));		// Clock Control(Clock Gating) Enable
	printk("RDMA Auto Power Down Mode - Auto PWDN!!!\n");
	BITCSET(pIREQConfig->uAUTOPD.nREG,(1<<0), (1<<0));		// RDMA Auto Power Down Mode - Auto PWDN
	printk("WDMA Auto Power Down Mode - Auto PWDN!!!\n");
	BITCSET(pIREQConfig->uAUTOPD.nREG,(1<<3), (1<<3));		// WDMA Auto Power Down Mode - Auto PWDN

	tcc_dbg("VIOC_DISP is reset clear!!!\n");

	if(viqe_plug_in) {
	 	BITCSET(VIQEBackUp.cVIQE_CTRL.nVIQE_CTRL.nREG, 1<< 4, 0<<4);
		BITCSET(VIQEBackUp.cVIQE_CTRL.nVIQE_CTRL.nREG, 1<< 20, 0<<20);
		BITCSET(VIQEBackUp.cDEINTL_DMA.nDEINTL_CTRL.nREG, 1 << 16, 0<<16);

		*pVIQEBase = VIQEBackUp;
	}

	if(output_lcdc_num){
		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP1);
		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX1);
	} else {
		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP0);
		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX0);
	}

 	BITCSET(DISPBackUp.uCTRL.nREG, 1<< 0, 0<< 0);
	*pDISPBase = DISPBackUp;
	tcc_dbg("VIOC_DISP is restored!!!\n");

 	BITCSET(WMIXBackUp.uCTRL.nREG, 1<< 16, 0<<16);
	*pWMIXBase = WMIXBackUp;
	tcc_dbg("VIOC_WMIX is restored!!!\n");

	for( idx = 0; idx < 4; idx++) {
		if(sc_plug_in[idx]) {
			*pSCBase[idx] = SCBackUp[idx];
			tcc_dbg("VIOC_SC%d is restored!!!\n", idx );
		}
	}

	if(output_lcdc_num){
		for(idx = 4; idx < 8; idx++) {
			if(rdma_en[idx]) {
				BITCSET(RDMABackUp[idx].uCTRL.nREG, 1 << 28, 0 << 28);
				*pRDMABase[idx] = RDMABackUp[idx];
				tcc_dbg("VIOC_RDMA%d is restored!!!\n", idx );
			}
		}
	} else {
		for(idx = 0; idx < 4; idx++) {
			if(rdma_en[idx]) {
				BITCSET(RDMABackUp[idx].uCTRL.nREG, 1 << 28, 0 << 28);
				*pRDMABase[idx] = RDMABackUp[idx];
				tcc_dbg("VIOC_RDMA%d is restored!!!\n", idx );
			}
		}
	}

	VIOC_DISP_TurnOn((VIOC_DISP *)pDISPBase);

	if(output_mode == TCC_OUTPUT_HDMI)
		hdmi_start();

	VIOC_WMIX_SetUpdate((VIOC_WMIX *)pWMIXBase);


	for( idx = 0; idx < 4; idx++) {
		if(sc_plug_in[idx]) {
			VIOC_SC_SetUpdate((VIOC_SC *)pSCBase[idx]);
		}
	}

	//local_irq_enable();

	printk("%s, VIOC path is reset End!!\n", __func__);
}

static int tcc_vioc_manager_thread(void* vioc_manager_data)
{
	unsigned int output_lcdc_num;
 
	VIOC_DISP *pDISPBase;
	VIOC_RDMA *pRDMABase;
	VIQE *pVIQEBase;

    tcc_dbg("%s() in...\n\n", __func__);

    while(!kthread_should_stop()) 
    {
        /* polling time is 1000ms */
        msleep_interruptible(1000);

		output_lcdc_num = tca_get_output_lcdc_num();
		if(output_lcdc_num){
			pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP1);
			pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA07);
		} else {
			pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP0);
			pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA03);
		}
		
		pVIQEBase = (VIQE*)tcc_p2v(HwVIOC_VIQE0);
		
		if(pDISPBase->uLSTATUS.bREG.FU == 1) {
			unsigned int idx = 0;
		    printk("%s() DISP under-run is occurred!!!!!\n\n", __func__);
			printk("%s : RDMA03_CURADDR = ", __func__);
			for(idx = 0; idx < 10;idx++)
				printk("0x%08x, ",pRDMABase->nCBASE);
			printk("\n");

			printk("%s VIQE_VCTRL = 0x%08x, VIQE_DI_CTRL = 0x%08x\n", __func__, (unsigned int)pVIQEBase->cVIQE_CTRL.nVIQE_CTRL.nREG, (unsigned int)pVIQEBase->cDEINTL_DMA.nDEINTL_CTRL.nREG);
			
			//TCC_OUTPUT_LCDC_Reset();
		} else {
		    //tcc_dbg("%s() DISP is working normally!!!!!\n\n", __func__);
		}
    }

    tcc_dbg("%s() out...\n\n", __func__);

    return 0;
}

static int tcc_vioc_manager_probe(struct platform_device *pdev)
{
	struct tcc_vioc_manager_data *vioc_manager_data;

    tcc_dbg("tcc_vioc_manager_probe()...in \n\n");

	vioc_manager_data = kzalloc(sizeof(struct tcc_vioc_manager_data), GFP_KERNEL);
	if (!vioc_manager_data) {
		kfree(vioc_manager_data);
		return -ENOMEM;
	}

#if 0
    vioc_manager_data->poll_task = (struct task_struct *)(-EINVAL);
    vioc_manager_data->poll_task = kthread_create(tcc_vioc_manager_thread, vioc_manager_data, "tcc-vioc-manager-thread");
    if (IS_ERR(vioc_manager_data->poll_task)) {
        printk("\ntcc-vioc-manager-thread create error: %p\n", vioc_manager_data->poll_task);
		kfree(vioc_manager_data);
        return (-EINVAL);
    }
    wake_up_process(vioc_manager_data->poll_task);
#endif

    tcc_dbg("tcc-vioc_manager_probe()...out \n\n");

	return 0;

}

static int __devexit tcc_vioc_manager_remove(struct platform_device *pdev)
{
	struct tcc_vioc_manager_data *vioc_manager_data = platform_get_drvdata(pdev);

	kfree(vioc_manager_data);

	return 0;
}

static struct platform_driver tcc_vioc_manager_driver = {
		.probe		= tcc_vioc_manager_probe,
		.remove		= __devexit_p(tcc_vioc_manager_remove),
		.driver		= {
			.name	= "tcc-vioc-manager",
			.owner	= THIS_MODULE,
		},
};

static int __init tcc_vioc_manager_init(void)
{
    tcc_dbg("\n%s()...\n\n", __func__);
	return platform_driver_register(&tcc_vioc_manager_driver);
}

static void __exit tcc_vioc_manager_exit(void)
{
	platform_driver_unregister(&tcc_vioc_manager_driver);
}

module_init(tcc_vioc_manager_init);
module_exit(tcc_vioc_manager_exit);

MODULE_AUTHOR("Android ce team <android_ce@telechips.com>");
MODULE_DESCRIPTION("TCC VIOC Manager driver");
MODULE_LICENSE("GPL");


