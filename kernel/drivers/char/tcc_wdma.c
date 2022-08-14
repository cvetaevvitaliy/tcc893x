/*
 * drivers/char/tcc_wdma.c
 *
 * Copyright (C) 2004 Texas Instruments, Inc. 
 *
 * Video-for-Linux (Version 2) graphic capture driver for
 * the OMAP H2 and H3 camera controller.
 *
 * Adapted from omap24xx driver written by Andy Lowe (source@mvista.com)
 * Copyright (C) 2003-2004 MontaVista Software, Inc.
 * 
 * This package is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. 
 * 
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED 
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. 
 *
 * History:
 *   27/03/05   Vladimir Barinov - Added support for power management
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>
#include <asm/mach/map.h>
#include <mach/bsp.h>
#include <plat/pmap.h>
#include <linux/fs.h> 


#include <mach/irqs.h>
#include <mach/vioc_ireq.h>
#include <mach/vioc_rdma.h>
#include <mach/vioc_wdma.h>
#include <mach/vioc_wmix.h>
#include <mach/vioc_config.h>

#include <mach/vioc_scaler.h>
#include <mach/vioc_wmix.h>
#include <mach/vioc_api.h>
#include <mach/vioc_disp.h>
#include <linux/poll.h>
#include <mach/tccfb_ioctrl.h>
#include "tcc_wdma.h"
#include <mach/tcc_wdma_ioctrl.h>
#include <mach/tcc_fb.h>
#include <mach/tca_display_config.h>

#if defined(CONFIG_ARCH_TCC892X)
#include <mach/vioc_plugin_tcc892x.h>
#endif
#if defined(CONFIG_ARCH_TCC893X)
#include <mach/vioc_plugin_tcc893x.h>
#endif

#define WDMA_DEBUG 		0
#define dprintk(msg...) 	if(WDMA_DEBUG) { printk("WDMA: " msg); }

volatile PVIOC_DISP 	pWDMA_disp_base;
volatile PVIOC_WMIX 	pWDMA_wmix_base;
volatile PVIOC_WDMA 	pWDMA_wdma_base;
volatile PVIOC_SC 		pWDMA_scaler;


typedef struct _INTERRUPT_DATA_T {
	// wait for poll  
	wait_queue_head_t 	poll_wq;
	spinlock_t 			poll_lock;
	unsigned int 		poll_count;

	// wait for ioctl command
	wait_queue_head_t 	cmd_wq;
	spinlock_t 			cmd_lock;
	unsigned int 		cmd_count;

	struct mutex 		io_mutex;
	unsigned char 		block_operating;
	unsigned char 		block_waiting;
	unsigned char 		irq_reged;
	unsigned int  		dev_opened;
} INTERRUPT_DATA_T;
static INTERRUPT_DATA_T wdma_data;

#define DEVICE_NAME			"wdma"

#if 0
#define MAJOR_ID			214
#define MINOR_ID			1
#endif

static struct clk *wdma_clk;
char Fb_Lcdc_Num = 0;
unsigned	int MajorNum;

extern void tccxxx_GetAddress(unsigned char format, unsigned int base_Yaddr, unsigned int src_imgx, unsigned int  src_imgy,
									unsigned int start_x, unsigned int start_y, unsigned int* Y, unsigned int* U,unsigned int* V);

extern int range_is_allowed(unsigned long pfn, unsigned long size);
static int tccxxx_wdma_mmap(struct file *file, struct vm_area_struct *vma)
{
	if(range_is_allowed(vma->vm_pgoff, vma->vm_end - vma->vm_start) < 0) {
		printk(KERN_ERR	"wdma: this address is not allowed. \n");
		return -EAGAIN;
	}

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	if(remap_pfn_range(vma,vma->vm_start, vma->vm_pgoff , vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
		return -EAGAIN;
	}

	vma->vm_ops		=  NULL;
	vma->vm_flags 	|= VM_IO;
	vma->vm_flags 	|= VM_RESERVED;

	return 0;
}

char tccxxx_wdma_ctrl(unsigned int argp)
{
	#define RGBtoYUV			(1)

	struct lcd_panel *panel;
	VIOC_WDMA_IMAGE_INFO_Type ImageCfg;
	unsigned int base_addr = 0, Wmix_Height = 0, Wmix_Width = 0, DDevice = 0;
	int ret = 0;
	int addr_Y,  addr_U, addr_V;

      memset((char *)&ImageCfg, 0x00, sizeof(ImageCfg));
	panel = tccfb_get_panel();

	if(copy_from_user( &ImageCfg, (VIOC_WDMA_IMAGE_INFO_Type *)argp, sizeof(ImageCfg))){
		printk("### %s error \n",__func__);
		return -EFAULT;
	}

	ImageCfg.Interlaced = 0;
	ImageCfg.ContinuousMode = 0;
	ImageCfg.SyncMode = 0;
	ImageCfg.ImgSizeWidth = panel->xres;
	ImageCfg.ImgSizeHeight = panel->yres;

	ImageCfg.ImgFormat = TCC_LCDC_IMG_FMT_YUV420SP;

	base_addr = (unsigned int )ImageCfg.BaseAddress;
	
	tccxxx_GetAddress(ImageCfg.ImgFormat, (unsigned int)base_addr, ImageCfg.TargetWidth, ImageCfg.TargetHeight,
	 						0, 0, &addr_Y, &addr_U,&addr_V);

	addr_U = GET_ADDR_YUV42X_spU(base_addr, ImageCfg.TargetWidth, ImageCfg.TargetHeight);

	if(ImageCfg.ImgFormat == TCC_LCDC_IMG_FMT_YUV420SP)
		addr_V = GET_ADDR_YUV420_spV(addr_U, ImageCfg.TargetWidth, ImageCfg.TargetHeight);
	else
		addr_V = GET_ADDR_YUV422_spV(addr_U, ImageCfg.TargetWidth, ImageCfg.TargetHeight);

	ImageCfg.BaseAddress1 = addr_U;
	ImageCfg.BaseAddress2 = addr_V;

	
	VIOC_WMIX_GetSize(pWDMA_wmix_base, &Wmix_Height, &Wmix_Width);

	DDevice = VIOC_DISP_Get_TurnOnOff(pWDMA_disp_base);
	if((Wmix_Width ==0) || (Wmix_Height ==0) || (DDevice == 0))
	{
		wdma_data.block_operating = 0;
		printk("Error tccxxx_wdma_ctrl W:%d H:%d DD-Power:%d \n", Wmix_Width, Wmix_Height, DDevice);
		return 0;
	}

	dprintk("src  w:%d h:%d base:0x%08x  \n",ImageCfg.ImgSizeWidth,ImageCfg.ImgSizeHeight,ImageCfg.BaseAddress);
	dprintk("dest w:%d h:%d  %d  \n",ImageCfg.TargetWidth,ImageCfg.TargetHeight, Fb_Lcdc_Num);
 	dprintk("wmixer size  %d %d  : base1:0x%08x  base2:0x%08x  \n",Wmix_Width, Wmix_Height, ImageCfg.BaseAddress1, ImageCfg.BaseAddress2);



	  /* scaler setting */
	VIOC_SC_SetBypass (pWDMA_scaler, OFF);
	VIOC_SC_SetSrcSize(pWDMA_scaler, Wmix_Width, Wmix_Height);	  
	VIOC_SC_SetDstSize (pWDMA_scaler, ImageCfg.TargetWidth, ImageCfg.TargetHeight);
	VIOC_SC_SetOutSize (pWDMA_scaler, ImageCfg.TargetWidth, ImageCfg.TargetHeight);	
	VIOC_SC_SetUpdate(pWDMA_scaler);

	spin_lock_irq(&(wdma_data.cmd_lock));

	if(Fb_Lcdc_Num)
	{
		VIOC_CONFIG_PlugIn(VIOC_SC0, VIOC_SC_WDMA_01);
		VIOC_CONFIG_WMIXPath(WMIX10, 1 /* Mixing */);
		VIOC_CONFIG_WMIXPath(WMIX13, 1 /* Mixing */); 
	}

	else
	{
		VIOC_CONFIG_PlugIn(VIOC_SC0, VIOC_SC_WDMA_00);
		VIOC_CONFIG_WMIXPath(WMIX00, 1 /* Mixing */);
		VIOC_CONFIG_WMIXPath(WMIX03, 1 /* Mixing */);
	}
	
       VIOC_WDMA_SetWifiDisplayImageConfig(pWDMA_wdma_base, &ImageCfg, RGBtoYUV);

	spin_unlock_irq(&(wdma_data.cmd_lock));

	ret = wait_event_interruptible_timeout(wdma_data.poll_wq, wdma_data.block_operating == 0, msecs_to_jiffies(100));
	if(ret <= 0) {
		wdma_data.block_operating = 0;
		printk("wdma time out: %d, Line: %d. \n", ret, __LINE__);
	}
	//VIOC_WDMA_SetImageEnable(pWDMA_wdma_base,0);

	VIOC_CONFIG_PlugOut(VIOC_SC0); 



   	if (copy_to_user( (VIOC_WDMA_IMAGE_INFO_Type *)argp, &ImageCfg, sizeof(ImageCfg))) {
		return -EFAULT;
	}
	
	return ret;
}

static unsigned int tccxxx_wdma_poll(struct file *filp, poll_table *wait)
{
	int ret = 0;
	INTERRUPT_DATA_T *wdma_data = (INTERRUPT_DATA_T *)filp->private_data;

	if(wdma_data == NULL) 	return -EFAULT;

	poll_wait(filp, &(wdma_data->poll_wq), wait);

	spin_lock_irq(&(wdma_data->poll_lock));

	if(wdma_data->block_operating == 0) 	ret = (POLLIN|POLLRDNORM);

	spin_unlock_irq(&(wdma_data->poll_lock));

	return ret;
}

static irqreturn_t tccxxx_wdma_handler(int irq, void *client_data)
{  	
	INTERRUPT_DATA_T *wdma_data = client_data;
	
	if(pWDMA_wdma_base->uIRQSTS.nREG & VIOC_WDMA_IREQ_EOFR_MASK) {

		if(wdma_data->block_operating >= 1) 	
			wdma_data->block_operating = 0;
			
		wake_up_interruptible(&(wdma_data->poll_wq));

		if(wdma_data->block_waiting) 	
			wake_up_interruptible(&wdma_data->cmd_wq);
	}

	pWDMA_wdma_base->uIRQSTS.nREG = 0xFFFFFFFF;   // wdma status register all clear.


	return IRQ_HANDLED;
}

long tccxxx_wdma_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	//WMIXER_INFO_TYPE wmix_info;
	INTERRUPT_DATA_T *wdma_data = (INTERRUPT_DATA_T *)file->private_data;
 	dprintk("wdma:  cmd(0x%x), block_operating(0x%x), block_waiting(0x%x), cmd_count(0x%x), poll_count(0x%x). \n", 	\
 					cmd, wdma_data->block_operating, wdma_data->block_waiting, wdma_data->cmd_count, wdma_data->poll_count);
	
	switch(cmd) {
		case TCC_WDMA_IOCTRL:
			mutex_lock(&wdma_data->io_mutex);

            if(wdma_data->block_operating) {
				wdma_data->block_waiting = 1;
				ret = wait_event_interruptible_timeout(wdma_data->cmd_wq, wdma_data->block_operating == 0, msecs_to_jiffies(200));
				if(ret <= 0) {
					wdma_data->block_operating = 0;
					printk("[%d]: wdma 0 timed_out block_operation:%d!! cmd_count:%d \n", ret, wdma_data->block_waiting, wdma_data->cmd_count);
				}
				ret = 0;
			}
			
			if(ret >= 0) {
				if(wdma_data->block_operating >= 1) {
					printk("scaler + :: block_operating(%d) - block_waiting(%d) - cmd_count(%d) - poll_count(%d)!!!\n", 	\
								wdma_data->block_operating, wdma_data->block_waiting, wdma_data->cmd_count, wdma_data->poll_count);
				}

				wdma_data->block_waiting = 0;
				wdma_data->block_operating = 1;

				ret = tccxxx_wdma_ctrl(arg);// function call

				if(ret < 0) 
                    wdma_data->block_operating = 0;
			}
			mutex_unlock(&wdma_data->io_mutex);

			return 0;

		default:
			printk(KERN_ALERT "not supported WMIXER IOCTL(0x%x). \n", cmd);
			break;			
	}

	return 0;
}
EXPORT_SYMBOL(tccxxx_wdma_ioctl);

int tccxxx_wdma_release(struct inode *inode, struct file *filp)
{
	int ret = 0;
	printk("wdma_release IN:  %d'th, block(%d/%d), cmd(%d), irq(%d). \n", wdma_data.dev_opened, 			\
			wdma_data.block_operating, wdma_data.block_waiting, wdma_data.cmd_count, wdma_data.irq_reged);

	if(wdma_data.dev_opened > 0) 	
        wdma_data.dev_opened--;

	if(wdma_data.dev_opened == 0) {
		if(wdma_data.block_operating) {
			ret = wait_event_interruptible_timeout(wdma_data.cmd_wq, wdma_data.block_operating == 0, msecs_to_jiffies(200));
		}

		if(ret <= 0) {
 			printk("[%d]: wdma timed_out block_operation: %d, cmd_count: %d. \n", ret, wdma_data.block_waiting, wdma_data.cmd_count);
		}

		if(wdma_data.irq_reged) {

			if(Fb_Lcdc_Num)
				free_irq(INT_VIOC_WD1, &wdma_data);
			else
				free_irq(INT_VIOC_WD0, &wdma_data);
			
			wdma_data.irq_reged = 0;
		}

		//VIOC_WMIX_SetSWReset(VIOC_WMIX4, VIOC_WMIX_RDMA_14, VIOC_WMIX_WDMA_04);

		wdma_data.block_operating = wdma_data.block_waiting = 0;
		wdma_data.poll_count = wdma_data.cmd_count = 0;
	}

	
	clk_disable(wdma_clk);

	dprintk("wdma_release OUT:  %d'th. \n", wdma_data.dev_opened);
	return 0;
}
EXPORT_SYMBOL(tccxxx_wdma_release);



int tccxxx_wdma_open(struct inode *inode, struct file *filp)
{	
	int ret = 0;

	printk("wdma_open IN:  %d'th, block(%d/%d), cmd(%d), irq(%d). \n", wdma_data.dev_opened, 				\
			wdma_data.block_operating, wdma_data.block_waiting, wdma_data.cmd_count, wdma_data.irq_reged);

	clk_enable(wdma_clk);

	if(!wdma_data.irq_reged) {		
		if(Fb_Lcdc_Num)
		{
			pWDMA_disp_base = (volatile PVIOC_DISP)tcc_p2v((unsigned int)HwVIOC_DISP1);		
			pWDMA_wmix_base = (volatile PVIOC_WMIX)tcc_p2v((unsigned int)HwVIOC_WMIX1);		
			pWDMA_wdma_base = (volatile PVIOC_WDMA)tcc_p2v((unsigned int)HwVIOC_WDMA01);
			ret = request_irq(INT_VIOC_WD1, tccxxx_wdma_handler, IRQF_SHARED, "wdma", &wdma_data);
		}
		else
		{
			pWDMA_disp_base = (volatile PVIOC_DISP)tcc_p2v((unsigned int)HwVIOC_DISP0);		
			pWDMA_wmix_base = (volatile PVIOC_WMIX)tcc_p2v((unsigned int)HwVIOC_WMIX0);		
			pWDMA_wdma_base = (volatile PVIOC_WDMA)tcc_p2v((unsigned int)HwVIOC_WDMA00);
			ret = request_irq(INT_VIOC_WD0, tccxxx_wdma_handler, IRQF_SHARED, "wdma", &wdma_data);
		}

		pWDMA_scaler = (volatile PVIOC_SC)tcc_p2v(HwVIOC_SC0);

		//VIOC_WMIX_SetSWReset(VIOC_WMIX1, VIOC_WMIX_RDMA_14, VIOC_WMIX_WDMA_01);
		VIOC_WDMA_SetIreqMask(pWDMA_wdma_base, VIOC_WDMA_IREQ_EOFR_MASK, 0x0);

		
		if(ret) {
			clk_disable(wdma_clk);
			printk("failed to aquire wdma request_irq. \n");
			return -EFAULT;
		}

		wdma_data.irq_reged = 1;
	}

	wdma_data.dev_opened++;
	filp->private_data = &wdma_data;
	
	dprintk("wdma_open OUT:  %d'th. \n", wdma_data.dev_opened);
	return ret;	
}
EXPORT_SYMBOL(tccxxx_wdma_open);


static struct file_operations tcc_wdma_fops = {
	.owner 				= THIS_MODULE,
	.unlocked_ioctl 	= tccxxx_wdma_ioctl,
	.mmap 				= tccxxx_wdma_mmap,
	.open 				= tccxxx_wdma_open,
	.release 			= tccxxx_wdma_release,
	.poll 				= tccxxx_wdma_poll,
};

void __exit
tccxxx_wdma_cleanup(void)
{
	//unregister_chrdev(MAJOR_ID, DEVICE_NAME);
	unregister_chrdev( MajorNum, DEVICE_NAME);
	clk_put(wdma_clk);
	return;
}

static char banner[] __initdata = KERN_INFO "2013/01/08 ver TCC WDMA Driver Initializing. \n";
static struct class *wdma_class;

int __init 
tccxxx_wdma_init(void)
{
	int err;
	printk(banner);

	MajorNum = register_chrdev(0, DEVICE_NAME, &tcc_wdma_fops);
	if (MajorNum < 0) {
		printk("%s: device failed widh %d\n", __func__, MajorNum);
		err = -ENODEV;

	}

	wdma_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(wdma_class)) {
		err = PTR_ERR(wdma_class);

	}
	device_create(wdma_class,NULL,MKDEV(MajorNum, 0),NULL,DEVICE_NAME);

	#if defined(CONFIG_MACH_TCC8920ST) || defined(CONFIG_MACH_TCC8930ST)
		Fb_Lcdc_Num = tca_get_output_lcdc_num();
	#else
		Fb_Lcdc_Num = tca_get_lcd_lcdc_num();
	#endif//

	memset(&wdma_data, 0, sizeof(INTERRUPT_DATA_T));

	spin_lock_init(&(wdma_data.poll_lock));
	spin_lock_init(&(wdma_data.cmd_lock));

	mutex_init(&(wdma_data.io_mutex));
	
	init_waitqueue_head(&(wdma_data.poll_wq));
	init_waitqueue_head(&(wdma_data.cmd_wq));

	wdma_clk = clk_get(NULL, "ddi");
	BUG_ON(wdma_clk == NULL);

	return 0;
}


MODULE_AUTHOR("Telechips.");
MODULE_DESCRIPTION("TCC WDMA Driver");
MODULE_LICENSE("GPL");


arch_initcall(tccxxx_wdma_init);
module_exit(tccxxx_wdma_cleanup);


