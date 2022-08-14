/****************************************************************************
One line to give the program's name and a brief idea of what it does.
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
#if defined(CONFIG_ARCH_TCC892X) || defined(CONFIG_ARCH_TCC893X)
#include <mach/irqs.h>
#include <mach/vioc_ireq.h>
#include <mach/vioc_rdma.h>
#include <mach/vioc_wdma.h>
#include <mach/vioc_wmix.h>
#include <mach/vioc_config.h>
#include <mach/vioc_scaler.h>
#include <mach/vioc_wmix.h>
#include <mach/vioc_api.h>
#if defined(CONFIG_ARCH_TCC892X)
#include <mach/vioc_plugin_tcc892x.h>
#elif defined(CONFIG_ARCH_TCC893X)
#include <mach/vioc_plugin_tcc893x.h>
#endif
#endif
#include <linux/poll.h>
#include "tcc_wmixer1.h"
#include <mach/tcc_wmixer_ioctrl.h>


#define WMIXER_DEBUG 		0
#define dprintk(msg...) 	if(WMIXER_DEBUG) { printk("WMIXER-1: " msg); }

volatile PVIOC_RDMA 			pWMIX1_rdma_base;
volatile PVIOC_RDMA 			pWMIX1_rdma1_base;
volatile PVIOC_WMIX 			pWMIX1_wmix_base;
volatile PVIOC_WDMA 			pWMIX1_wdma_base;
volatile PVIOC_IREQ_CONFIG 	pWMIX1_ireg_config;

typedef struct _INTERRUPT_DATA_T {
	// wait for poll  
	wait_queue_head_t 	poll_wq;
	spinlock_t 			poll_lock;
	unsigned int 			poll_count;

	// wait for ioctl command
	wait_queue_head_t 	cmd_wq;
	spinlock_t 			cmd_lock;
	unsigned int 			cmd_count;

	struct mutex 			io_mutex;
	unsigned char 		block_operating;
	unsigned char 		block_waiting;
	unsigned char 		irq_reged;
	unsigned int  			dev_opened;
} INTERRUPT_DATA_T;
static INTERRUPT_DATA_T wmixer1_data;

#define DEVICE_NAME			"wmixer1"
#define MAJOR_ID			225
#define MINOR_ID			1

static struct clk *wmixer1_clk;

extern int range_is_allowed(unsigned long pfn, unsigned long size);
static int tccxxx_wmixer1_mmap(struct file *file, struct vm_area_struct *vma)
{
	if(range_is_allowed(vma->vm_pgoff, vma->vm_end - vma->vm_start) < 0) {
		printk(KERN_ERR	"wmixer: this address is not allowed. \n");
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

int tccxxx_wmixer1_ctrl(WMIXER_INFO_TYPE *wmix_info)
{
	int ret = 0;
	VIOC_SCALER_INFO_Type sc_info;

	dprintk("%s():  \n", __func__);
	dprintk("Src  : addr:0x%x 0x%x 0x%x  fmt:%d \n", wmix_info->src_y_addr, wmix_info->src_u_addr, wmix_info->src_v_addr, wmix_info->src_fmt);
	dprintk("Dest: addr:0x%x 0x%x 0x%x  fmt:%d \n", wmix_info->dst_y_addr, wmix_info->dst_u_addr, wmix_info->dst_v_addr, wmix_info->dst_fmt);
	dprintk("Size : W:%d  H:%d \n", wmix_info->img_width, wmix_info->img_height);


	spin_lock_irq(&(wmixer1_data.cmd_lock));

	// set to RDMA
	VIOC_RDMA_SetImageFormat(pWMIX1_rdma_base, wmix_info->src_fmt);
	VIOC_RDMA_SetImageSize(pWMIX1_rdma_base, wmix_info->img_width, wmix_info->img_height);
	VIOC_RDMA_SetImageOffset(pWMIX1_rdma_base, wmix_info->src_fmt, wmix_info->img_width);
	VIOC_RDMA_SetImageBase(pWMIX1_rdma_base, wmix_info->src_y_addr, wmix_info->src_u_addr,  wmix_info->src_v_addr);

	// set to WMIX
	VIOC_WMIX_SetSize(pWMIX1_wmix_base, wmix_info->img_width, wmix_info->img_height);
	VIOC_WMIX_SetUpdate(pWMIX1_wmix_base);
	VIOC_RDMA_SetImageEnable(pWMIX1_rdma_base); // Soc guide info.

	// set to WRMA
	VIOC_WDMA_SetImageFormat(pWMIX1_wdma_base, wmix_info->dst_fmt);
	VIOC_WDMA_SetImageSize(pWMIX1_wdma_base, wmix_info->img_width, wmix_info->img_height);
	VIOC_WDMA_SetImageOffset(pWMIX1_wdma_base, wmix_info->dst_fmt, wmix_info->img_width);

	VIOC_WDMA_SetImageBase(pWMIX1_wdma_base, wmix_info->dst_y_addr, wmix_info->dst_u_addr, wmix_info->dst_v_addr);
	VIOC_WDMA_SetImageEnable(pWMIX1_wdma_base, 0/*OFF*/);
	pWMIX1_wdma_base->uIRQSTS.nREG = 0xFFFFFFFF; // wdma status register all clear.

	spin_unlock_irq(&(wmixer1_data.cmd_lock));

	if(wmix_info->rsp_type == WMIXER_POLLING) {
		ret = wait_event_interruptible_timeout(wmixer1_data.poll_wq, wmixer1_data.block_operating == 0, msecs_to_jiffies(500));
		if(ret <= 0) {
			 wmixer1_data.block_operating = 0;
			printk("wmixer time out: %d, Line: %d. \n", ret, __LINE__);
		}
	}

	return ret;
}

int tccxxx_wmixer1_alpha_scaling_ctrl(WMIXER_ALPHASCALERING_INFO_TYPE *aps_info)
{
	int ret = 0;
	VIOC_SCALER_INFO_Type sc_info;

	dprintk("%s():  \n", __func__);
	dprintk("Src:  addr:0x%x, 0x%x, 0x%x,  fmt:%d. \n", aps_info->src_y_addr, aps_info->src_u_addr, aps_info->src_v_addr, aps_info->src_fmt);
	dprintk("Dst:  addr:0x%x, 0x%x, 0x%x,  fmt:%d. \n", aps_info->dst_y_addr, aps_info->dst_u_addr, aps_info->dst_v_addr, aps_info->dst_fmt);
	dprintk("Size : src_w:%d, src_h:%d, dst_w:%d, dst_h:%d. \n", aps_info->src_img_width, aps_info->src_img_height, \
														   aps_info->dst_img_width, aps_info->dst_img_height);

	if(aps_info->rsp_type == WMIXER_POLLING) {
		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[1], (1<<03), (1<<03)); 	// WDMA3 reset
		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[1], (1<<12), (1<<12)); 	// WMIX3 reset
		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[0], (1<<28), (1<<28)); 	// SCALER0 reset
		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[0], (1<<12), (1<<12)); 	// RDMA12 reset
		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[0], (1<<13), (1<<13)); 	// RDMA13 reset

		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[0], (1<<12), (0<<12)); 	// RDMA12 reset
		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[0], (1<<13), (0<<13)); 	// RDMA13 reset
		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[0], (1<<28), (0<<28)); 	// SCALER0 reset
		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[1], (1<<12), (0<<12)); 	// WMIX3 reset
		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[1], (1<<03), (0<<03)); 	// WDMA3 reset
	}

	spin_lock_irq(&(wmixer1_data.cmd_lock));

	VIOC_RDMA_SetImageAlphaSelect(pWMIX1_rdma_base, 1);
	VIOC_RDMA_SetImageAlphaEnable(pWMIX1_rdma_base, 1);
	VIOC_RDMA_SetImageFormat(pWMIX1_rdma_base, aps_info->src_fmt);
	
	//interlaced frame process ex) MPEG2
	if(aps_info->interlaced)
	{
		VIOC_RDMA_SetImageSize(pWMIX1_rdma_base, aps_info->src_img_width, aps_info->src_img_height / 2);
		VIOC_RDMA_SetImageOffset(pWMIX1_rdma_base, aps_info->src_fmt, aps_info->src_img_width * 2);
	}

	else
	{
		VIOC_RDMA_SetImageSize(pWMIX1_rdma_base, aps_info->src_img_width, aps_info->src_img_height);
		VIOC_RDMA_SetImageOffset(pWMIX1_rdma_base, aps_info->src_fmt, aps_info->src_img_width);
	}
	
	VIOC_RDMA_SetImageBase(pWMIX1_rdma_base, aps_info->src_y_addr, aps_info->src_u_addr,  aps_info->src_v_addr);

	sc_info.BYPASS 			= FALSE /* 0 */;
	sc_info.DST_WIDTH 		= aps_info->dst_img_width;
	sc_info.DST_HEIGHT 		= aps_info->dst_img_height;
	sc_info.OUTPUT_POS_X 	= 0;
	sc_info.OUTPUT_POS_Y 		= 0;
	sc_info.OUTPUT_WIDTH 	= sc_info.DST_WIDTH;
	sc_info.OUTPUT_HEIGHT 	= sc_info.DST_HEIGHT;	
	VIOC_API_SCALER_SetConfig(VIOC_SC0, &sc_info);
	VIOC_API_SCALER_SetPlugIn(VIOC_SC0, VIOC_SC_RDMA_12);
	VIOC_API_SCALER_SetUpdate(VIOC_SC0);
	VIOC_RDMA_SetImageEnable(pWMIX1_rdma_base); // SoC guide info.

	VIOC_CONFIG_WMIXPath(WMIX30, 0); // wmixer op is by-pass mode.
	VIOC_WMIX_SetSize(pWMIX1_wmix_base, sc_info.DST_WIDTH, sc_info.DST_HEIGHT);
	VIOC_WMIX_SetUpdate(pWMIX1_wmix_base);

	VIOC_WDMA_SetImageFormat(pWMIX1_wdma_base, aps_info->dst_fmt);
	VIOC_WDMA_SetImageSize(pWMIX1_wdma_base, sc_info.DST_WIDTH, sc_info.DST_HEIGHT);
	VIOC_WDMA_SetImageOffset(pWMIX1_wdma_base, aps_info->dst_fmt, aps_info->dst_img_width);
	VIOC_WDMA_SetImageBase(pWMIX1_wdma_base, aps_info->dst_y_addr, aps_info->dst_u_addr, aps_info->dst_v_addr);
	if((aps_info->src_fmt < SC_IMG_FMT_FCDEC) && (aps_info->dst_fmt > SC_IMG_FMT_FCDEC)) {
		VIOC_WDMA_SetImageR2YEnable(pWMIX1_wdma_base, 1);
	}
	VIOC_WDMA_SetImageEnable(pWMIX1_wdma_base, 0/*OFF*/);
	pWMIX1_wdma_base->uIRQSTS.nREG = 0xFFFFFFFF; // wdma status register all clear.

	spin_unlock_irq(&(wmixer1_data.cmd_lock));

	if(aps_info->rsp_type == WMIXER_POLLING) {
		ret = wait_event_interruptible_timeout(wmixer1_data.poll_wq, wmixer1_data.block_operating == 0, msecs_to_jiffies(500));
		if(ret <= 0) {
			 wmixer1_data.block_operating = 0;
			printk("wmixer time out: %d, Line: %d. \n", ret, __LINE__);
		}
	}

	return ret;
}
EXPORT_SYMBOL(tccxxx_wmixer1_alpha_scaling_ctrl);

int tccxxx_wmixer1_alpha_mixing_ctrl(WMIXER_ALPHABLENDING_TYPE *apb_info)
{
	int ret = 0;
	VIOC_SCALER_INFO_Type sc_info;

	dprintk("%s():  \n", __func__);

	spin_lock_irq(&(wmixer1_data.cmd_lock));

	VIOC_RDMA_SetImageAlphaEnable(pWMIX1_rdma_base, 1);
	VIOC_RDMA_SetImageAlphaSelect(pWMIX1_rdma_base, 1);
	VIOC_RDMA_SetImageFormat(pWMIX1_rdma_base, apb_info->src0_fmt);
	VIOC_RDMA_SetImageSize(pWMIX1_rdma_base, apb_info->src0_width, apb_info->src0_height);
	VIOC_RDMA_SetImageOffset(pWMIX1_rdma_base, apb_info->src0_fmt, apb_info->src0_width);
	VIOC_RDMA_SetImageBase(pWMIX1_rdma_base, apb_info->src0_Yaddr, apb_info->src0_Uaddr, apb_info->src0_Vaddr);

	VIOC_RDMA_SetImageAlphaEnable(pWMIX1_rdma1_base, 1);
	VIOC_RDMA_SetImageAlphaSelect(pWMIX1_rdma1_base, 1);
	VIOC_RDMA_SetImageFormat(pWMIX1_rdma1_base, apb_info->src1_fmt);
	VIOC_RDMA_SetImageSize(pWMIX1_rdma1_base, apb_info->src1_width, apb_info->src1_height);
	VIOC_RDMA_SetImageOffset(pWMIX1_rdma1_base, apb_info->src1_fmt, apb_info->src1_width);
	VIOC_RDMA_SetImageBase(pWMIX1_rdma1_base, apb_info->src1_Yaddr, apb_info->src1_Uaddr, apb_info->src1_Vaddr);

	sc_info.BYPASS 			= TRUE;
	sc_info.DST_WIDTH 		= apb_info->dst_width;
	sc_info.DST_HEIGHT 		= apb_info->dst_height;
	sc_info.OUTPUT_POS_X 	= 0;
	sc_info.OUTPUT_POS_Y 		= 0;
	sc_info.OUTPUT_WIDTH 	= sc_info.DST_WIDTH;
	sc_info.OUTPUT_HEIGHT 	= sc_info.DST_HEIGHT;	
	VIOC_API_SCALER_SetConfig(VIOC_SC0, &sc_info);
	VIOC_API_SCALER_SetPlugIn(VIOC_SC0, VIOC_SC_RDMA_12);
	VIOC_API_SCALER_SetUpdate(VIOC_SC0);
	VIOC_RDMA_SetImageEnable(pWMIX1_rdma_base); // SoC guide info.
	VIOC_RDMA_SetImageEnable(pWMIX1_rdma1_base);

	VIOC_CONFIG_WMIXPath(WMIX30, 1); // wmixer op is mixing mode.
	VIOC_WMIX_SetSize(pWMIX1_wmix_base, apb_info->dst_width, apb_info->dst_height);
	// set to layer0 of WMIX.
	VIOC_API_WMIX_SetOverlayAlphaValueControl(pWMIX1_wmix_base, apb_info->src0_layer, apb_info->region, apb_info->src0_acon0, apb_info->src0_acon1);
	VIOC_API_WMIX_SetOverlayAlphaColorControl(pWMIX1_wmix_base, apb_info->src0_layer, apb_info->region, apb_info->src0_ccon0, apb_info->src0_ccon1);
	VIOC_API_WMIX_SetOverlayAlphaROPMode(pWMIX1_wmix_base, apb_info->src0_layer, apb_info->src0_rop_mode);
	VIOC_API_WMIX_SetOverlayAlphaSelection(pWMIX1_wmix_base, apb_info->src0_layer, apb_info->src0_asel);
	VIOC_API_WMIX_SetOverlayAlphaValue(pWMIX1_wmix_base, apb_info->src0_layer, apb_info->src0_alpha0, apb_info->src0_alpha1);
	// set to layer1 of WMIX.
	//VIOC_API_WMIX_SetOverlayAlphaValueControl(pWMIX1_wmix_base, apb_info->src1_layer, apb_info->region, apb_info->src1_acon0, apb_info->src1_acon1);
	//VIOC_API_WMIX_SetOverlayAlphaColorControl(pWMIX1_wmix_base, apb_info->src1_layer, apb_info->region, apb_info->src1_ccon0, apb_info->src1_ccon1);
	//VIOC_API_WMIX_SetOverlayAlphaROPMode(pWMIX1_wmix_base, apb_info->src1_layer, apb_info->src1_rop_mode);
	//VIOC_API_WMIX_SetOverlayAlphaSelection(pWMIX1_wmix_base, apb_info->src1_layer, apb_info->src1_asel);
	//VIOC_API_WMIX_SetOverlayAlphaValue(pWMIX1_wmix_base, apb_info->src1_layer, apb_info->src1_alpha0, apb_info->src1_alpha1);
	// update WMIX.
	VIOC_WMIX_SetUpdate(pWMIX1_wmix_base);

	VIOC_WDMA_SetImageFormat(pWMIX1_wdma_base, apb_info->dst_fmt);
	VIOC_WDMA_SetImageSize(pWMIX1_wdma_base, apb_info->dst_width, apb_info->dst_height);
	VIOC_WDMA_SetImageOffset(pWMIX1_wdma_base, apb_info->dst_fmt, apb_info->dst_width);
	VIOC_WDMA_SetImageBase(pWMIX1_wdma_base, apb_info->dst_Yaddr, apb_info->dst_Uaddr, apb_info->dst_Vaddr);
	VIOC_WDMA_SetImageEnable(pWMIX1_wdma_base, 0 /* OFF */);
	pWMIX1_wdma_base->uIRQSTS.nREG = 0xFFFFFFFF; // wdma status register all clear.

	spin_unlock_irq(&(wmixer1_data.cmd_lock));

	if(apb_info->rsp_type == WMIXER_POLLING) {
		ret = wait_event_interruptible_timeout(wmixer1_data.poll_wq, wmixer1_data.block_operating == 0, msecs_to_jiffies(500));
		if(ret <= 0) {
			 wmixer1_data.block_operating = 0;
			printk("wmixer time out: %d, Line: %d. \n", ret, __LINE__);
		}
	}

	return ret;
}
EXPORT_SYMBOL(tccxxx_wmixer1_alpha_mixing_ctrl);

static unsigned int tccxxx_wmixer1_poll(struct file *filp, poll_table *wait)
{
	int ret = 0;
	INTERRUPT_DATA_T *wmix1_data = (INTERRUPT_DATA_T *)filp->private_data;

	if(wmix1_data == NULL) 	return -EFAULT;

	poll_wait(filp, &(wmix1_data->poll_wq), wait);

	spin_lock_irq(&(wmix1_data->poll_lock));

	if(wmix1_data->block_operating == 0) 	ret = (POLLIN|POLLRDNORM);

	spin_unlock_irq(&(wmix1_data->poll_lock));

	return ret;
}

static irqreturn_t tccxxx_wmixer1_handler(int irq, void *client_data)
{  	
	INTERRUPT_DATA_T *wmix1_data = (INTERRUPT_DATA_T *)client_data;
	
	if(pWMIX1_wdma_base->uIRQSTS.nREG & VIOC_WDMA_IREQ_EOFR_MASK) {
		dprintk("WDMA Interrupt is VIOC_WDMA_IREQ_EOFR_MASK. \n");
		pWMIX1_wdma_base->uIRQSTS.nREG = 0xFFFFFFFF;   // wdma status register all clear.
	}
		dprintk("WDMA Interrupt is VIOC_WDMA_IREQ_EOFR_MASK. \n");

	if(wmix1_data->block_operating >= 1) 	
		wmix1_data->block_operating = 0;
		
	wake_up_interruptible(&(wmix1_data->poll_wq));

	if(wmix1_data->block_waiting) 	
		wake_up_interruptible(&wmix1_data->cmd_wq);

	return IRQ_HANDLED;
}

long tccxxx_wmixer1_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	WMIXER_INFO_TYPE wmix_info;
	WMIXER_ALPHASCALERING_INFO_TYPE alpha_scalering;
	WMIXER_ALPHABLENDING_TYPE alpha_blending;
	INTERRUPT_DATA_T *wmix1_data = (INTERRUPT_DATA_T *)file->private_data;
	dprintk("%s():  cmd(%d), block_operating(%d), block_waiting(%d), cmd_count(%d), poll_count(%d). \n", __func__,	\
					cmd, wmix1_data->block_operating, wmix1_data->block_waiting, wmix1_data->cmd_count, wmix1_data->poll_count);

	switch(cmd) {
		case TCC_WMIXER_IOCTRL:
		case TCC_WMIXER_IOCTRL_KERNEL:
			mutex_lock(&wmix1_data->io_mutex);
			if(wmix1_data->block_operating) {
				wmix1_data->block_waiting = 1;
				ret = wait_event_interruptible_timeout(wmix1_data->cmd_wq, wmix1_data->block_operating == 0, msecs_to_jiffies(200));
				if(ret <= 0) {
					wmix1_data->block_operating = 0;
					printk("[%d]: wmixer 0 timed_out block_operation:%d!! cmd_count:%d \n", ret, wmix1_data->block_waiting, wmix1_data->cmd_count);
				}
				ret = 0;
			}

			if(cmd == TCC_WMIXER_IOCTRL_KERNEL){
				memcpy(&wmix_info,(WMIXER_INFO_TYPE*)arg, sizeof(WMIXER_INFO_TYPE));
			}else{
				if(copy_from_user(&wmix_info, (WMIXER_INFO_TYPE *)arg, sizeof(WMIXER_INFO_TYPE))) {
					printk(KERN_ALERT "Not Supported copy_from_user(%d). \n", cmd);
					ret = -EFAULT;
				}
			}

			if(ret >= 0) {
				if(wmix1_data->block_operating >= 1) {
					printk("scaler + :: block_operating(%d) - block_waiting(%d) - cmd_count(%d) - poll_count(%d)!!!\n", 	\
								wmix1_data->block_operating, wmix1_data->block_waiting, wmix1_data->cmd_count, wmix1_data->poll_count);
				}

				wmix1_data->block_waiting = 0;
				wmix1_data->block_operating = 1;
				ret = tccxxx_wmixer1_ctrl(&wmix_info);
				if(ret < 0) 	wmix1_data->block_operating = 0;
			}
			mutex_unlock(&wmix1_data->io_mutex);
			return ret;

		case TCC_WMIXER_ALPHA_SCALING:
		case TCC_WMIXER_ALPHA_SCALING_KERNEL:
			mutex_lock(&wmix1_data->io_mutex);
			if(wmix1_data->block_operating) {
				wmix1_data->block_waiting = 1;
				ret = wait_event_interruptible_timeout(wmix1_data->cmd_wq, wmix1_data->block_operating == 0, msecs_to_jiffies(200));
				if(ret <= 0) {
					wmix1_data->block_operating = 0;
					printk("[%d]: wmixer 0 timed_out block_operation:%d!! cmd_count:%d \n", ret, wmix1_data->block_waiting, wmix1_data->cmd_count);
				}
				ret = 0;
			}

			if(cmd == TCC_WMIXER_ALPHA_SCALING_KERNEL)
				memcpy(&alpha_scalering,(WMIXER_ALPHASCALERING_INFO_TYPE *)arg, sizeof(WMIXER_ALPHASCALERING_INFO_TYPE));
			else
			{
				if(copy_from_user((void *)&alpha_scalering, (const void *)arg, sizeof(WMIXER_ALPHASCALERING_INFO_TYPE))) {
						printk(KERN_ALERT "Not Supported copy_from_user(%d). \n", cmd);
						ret = -EFAULT;
				}
			}

			if(ret >= 0) {
				if(wmix1_data->block_operating >= 1) {
					printk("scaler + :: block_operating(%d) - block_waiting(%d) - cmd_count(%d) - poll_count(%d)!!!\n", 	\
							wmix1_data->block_operating, wmix1_data->block_waiting, wmix1_data->cmd_count, wmix1_data->poll_count);
				}

				wmix1_data->block_waiting = 0;
				wmix1_data->block_operating = 1;
				ret = tccxxx_wmixer1_alpha_scaling_ctrl(&alpha_scalering);
				if(ret < 0) 	wmix1_data->block_operating = 0;
			}			
			mutex_unlock(&wmix1_data->io_mutex);
			return ret;
			
		case TCC_WMIXER_ALPHA_MIXING:			
			mutex_lock(&wmix1_data->io_mutex);
			if(wmix1_data->block_operating) {
				wmix1_data->block_waiting = 1;
				ret = wait_event_interruptible_timeout(wmix1_data->cmd_wq, wmix1_data->block_operating == 0, msecs_to_jiffies(200));
				if(ret <= 0) {
					wmix1_data->block_operating = 0;
					printk("[%d]: wmixer 0 timed_out block_operation:%d!! cmd_count:%d \n", ret, wmix1_data->block_waiting, wmix1_data->cmd_count);
				}
				ret = 0;
			}

			if(copy_from_user(&alpha_blending, (WMIXER_ALPHABLENDING_TYPE *)arg, sizeof(WMIXER_ALPHABLENDING_TYPE))) {
					printk(KERN_ALERT "Not Supported copy_from_user(%d). \n", cmd);
					ret = -EFAULT;
			}

			if(ret >= 0) {
				if(wmix1_data->block_operating >= 1) {
					printk("scaler + :: block_operating(%d) - block_waiting(%d) - cmd_count(%d) - poll_count(%d)!!!\n", 	\
								wmix1_data->block_operating, wmix1_data->block_waiting, wmix1_data->cmd_count, wmix1_data->poll_count);
				}

				wmix1_data->block_waiting = 0;
				wmix1_data->block_operating = 1;
				ret = tccxxx_wmixer1_alpha_mixing_ctrl(&alpha_blending);
				if(ret < 0) 	wmix1_data->block_operating = 0;
			}			
			mutex_unlock(&wmix1_data->io_mutex);
			return ret;

		default:
			printk(KERN_ALERT "not supported WMIXER IOCTL(0x%x). \n", cmd);
			break;			
	}

	return 0;
}
EXPORT_SYMBOL(tccxxx_wmixer1_ioctl);

int tccxxx_wmixer1_release(struct inode *inode, struct file *filp)
{
	int ret = 0;
	dprintk("wmixer_release IN:  %d'th, block(%d/%d), cmd(%d), irq(%d). \n", wmixer1_data.dev_opened, 			\
			wmixer1_data.block_operating, wmixer1_data.block_waiting, wmixer1_data.cmd_count, wmixer1_data.irq_reged);

	if(wmixer1_data.dev_opened > 0) 	wmixer1_data.dev_opened--;
	if(wmixer1_data.dev_opened == 0) {
		if(wmixer1_data.block_operating) {
			ret = wait_event_interruptible_timeout(wmixer1_data.cmd_wq, wmixer1_data.block_operating == 0, msecs_to_jiffies(200));
		}

		if(ret <= 0) {
 			printk("[%d]: wmixer timed_out block_operation: %d, cmd_count: %d. \n", ret, wmixer1_data.block_waiting, wmixer1_data.cmd_count);
		}

		if(wmixer1_data.irq_reged) {
			free_irq(INT_VIOC_WD3, &wmixer1_data);
			wmixer1_data.irq_reged = 0;
		}

		//VIOC_CONFIG_PlugOut(VIOC_SC0); // SCALER0 plug-out 
		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[1], (1<<03), (1<<03)); 	// WDMA3 reset
		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[1], (1<<12), (1<<12)); 	// WMIX3 reset
		//BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[0], (1<<28), (1<<28)); 	// SCALER0 reset
		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[0], (1<<12), (1<<12)); 	// RDMA12 reset
		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[0], (1<<13), (1<<13)); 	// RDMA13 reset

		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[0], (1<<12), (0<<12)); 	// RDMA12 reset
		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[0], (1<<13), (0<<13)); 	// RDMA13 reset
		//BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[0], (1<<28), (0<<28)); 	// SCALER0 reset
		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[1], (1<<12), (0<<12)); 	// WMIX3 reset
		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[1], (1<<03), (0<<03)); 	// WDMA3 reset

		wmixer1_data.block_operating = wmixer1_data.block_waiting = 0;
		wmixer1_data.poll_count = wmixer1_data.cmd_count = 0;
	}

	clk_disable(wmixer1_clk);

	dprintk("wmixer_release OUT:  %d'th. \n", wmixer1_data.dev_opened);
	return 0;
}
EXPORT_SYMBOL(tccxxx_wmixer1_release);

int tccxxx_wmixer1_open(struct inode *inode, struct file *filp)
{	
	int ret = 0;
	dprintk("wmixer1_open IN:  %d'th, block(%d/%d), cmd(%d), irq(%d). \n", wmixer1_data.dev_opened, 				\
			wmixer1_data.block_operating, wmixer1_data.block_waiting, wmixer1_data.cmd_count, wmixer1_data.irq_reged);

	clk_enable(wmixer1_clk);

	if(!wmixer1_data.irq_reged) {
		pWMIX1_rdma_base 		= (volatile PVIOC_RDMA)tcc_p2v((unsigned int)HwVIOC_RDMA12);
		pWMIX1_rdma1_base 	= (volatile PVIOC_RDMA)tcc_p2v((unsigned int)HwVIOC_RDMA13);
		pWMIX1_wmix_base 		= (volatile PVIOC_WMIX)tcc_p2v((unsigned int)HwVIOC_WMIX3);
		pWMIX1_wdma_base 	= (volatile PVIOC_WDMA)tcc_p2v((unsigned int)HwVIOC_WDMA03);
		pWMIX1_ireg_config 		= (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_IREQ);

		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[1], (1<<03), (1<<03)); 	// WDMA3 reset
		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[1], (1<<12), (1<<12)); 	// WMIX3 reset
		//BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[0], (1<<28), (1<<28)); 	// SCALER0 reset
		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[0], (1<<12), (1<<12)); 	// RDMA12 reset
		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[0], (1<<13), (1<<13)); 	// RDMA13 reset

		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[0], (1<<12), (0<<12)); 	// RDMA12 reset
		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[0], (1<<13), (0<<13)); 	// RDMA13 reset
		//BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[0], (1<<28), (0<<28)); 	// SCALER0 reset
		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[1], (1<<12), (0<<12)); 	// WMIX3 reset
		BITCSET(pWMIX1_ireg_config->uSOFTRESET.nREG[1], (1<<03), (0<<03)); 	// WDMA3 reset

		BITCSET(pWMIX1_ireg_config->uMISC1.nREG, (1<<23), (0<<23)); 			// CFG_MISC1 :  STOP_REQ[BIT23] clear 0.

		VIOC_WDMA_SetIreqMask(pWMIX1_wdma_base, VIOC_WDMA_IREQ_EOFR_MASK, 0x0);
		ret = request_irq(INT_VIOC_WD3, tccxxx_wmixer1_handler, IRQF_SHARED, "wmixer1", &wmixer1_data);
		if(ret) {
			clk_disable(wmixer1_clk);
			printk("failed to aquire wmixer request_irq. \n");
			return -EFAULT;
		}

		wmixer1_data.irq_reged = 1;
	}

	wmixer1_data.dev_opened++;
	filp->private_data = &wmixer1_data;
	
	dprintk("wmixer1_open OUT:  %d'th. \n", wmixer1_data.dev_opened);
	return ret;	
}
EXPORT_SYMBOL(tccxxx_wmixer1_open);


static struct file_operations tcc_wmixer1_fops = {
	.owner 				= THIS_MODULE,
	.unlocked_ioctl 		= tccxxx_wmixer1_ioctl,
	.mmap 				= tccxxx_wmixer1_mmap,
	.open 				= tccxxx_wmixer1_open,
	.release 				= tccxxx_wmixer1_release,
	.poll 				= tccxxx_wmixer1_poll,
};

void __exit
tccxxx_wmixer1_cleanup(void)
{
	unregister_chrdev(MAJOR_ID, DEVICE_NAME);
	clk_put(wmixer1_clk);
	return;
}

static char banner[] __initdata = KERN_INFO "TCC WMIXER-1 Driver Initializing. \n";
static struct class *wmixer1_class;

int __init 
tccxxx_wmixer1_init(void)
{
	printk(banner);
	
	register_chrdev(MAJOR_ID, DEVICE_NAME, &tcc_wmixer1_fops);
	wmixer1_class = class_create(THIS_MODULE, DEVICE_NAME);
	device_create(wmixer1_class, NULL, MKDEV(MAJOR_ID, MINOR_ID), NULL, DEVICE_NAME);

	memset(&wmixer1_data, 0, sizeof(INTERRUPT_DATA_T));

	spin_lock_init(&(wmixer1_data.poll_lock));
	spin_lock_init(&(wmixer1_data.cmd_lock));

	mutex_init(&(wmixer1_data.io_mutex));
	
	init_waitqueue_head(&(wmixer1_data.poll_wq));
	init_waitqueue_head(&(wmixer1_data.cmd_wq));

	wmixer1_clk = clk_get(NULL, "lcdc0");
	BUG_ON(wmixer1_clk == NULL);

	return 0;
}


MODULE_AUTHOR("Telechips.");
MODULE_DESCRIPTION("TCC WMIXER-1 Driver");
MODULE_LICENSE("GPL");


arch_initcall(tccxxx_wmixer1_init);
module_exit(tccxxx_wmixer1_cleanup);


