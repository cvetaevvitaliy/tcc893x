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
#if defined(CONFIG_ARCH_TCC893X)
#include <mach/vioc_global.h>
#elif defined(CONFIG_ARCH_TCC892X)
#include <mach/vioc_plugin_tcc892x.h>
#endif
#endif
#include <linux/poll.h>
#include "tcc_wmixer.h"

#include <mach/tcc_wmixer_ioctrl.h>


#define WMIXER_DEBUG 		0
#define dprintk(msg...) 		if(WMIXER_DEBUG) { printk("WMIXER: " msg); }

volatile PVIOC_RDMA 			pWMIX_rdma_base;
volatile PVIOC_WMIX 			pWMIX_wmix_base;
volatile PVIOC_WDMA 			pWMIX_wdma_base;
#if defined(CONFIG_CHIP_TCC8925S)
volatile PVIOC_RDMA 			pWMIX_rdma1_base;
volatile PVIOC_SC 				pWMIX_scaler_base;
#endif
volatile PVIOC_IREQ_CONFIG 	pWMIX_ireg_config;

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
static INTERRUPT_DATA_T wmixer_data;

#define DEVICE_NAME			"wmixer"
#define MAJOR_ID			210
#define MINOR_ID			1

static struct clk *wmixer_clk;

extern int range_is_allowed(unsigned long pfn, unsigned long size);
static int tccxxx_wmixer_mmap(struct file *file, struct vm_area_struct *vma)
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

#if defined(CONFIG_CHIP_TCC8925S)
char tccxxx_wmixer_alpha_scaling_ctrl(WMIXER_ALPHASCALERING_INFO_TYPE *wmix_info)
{
	int ret = 0;
	VIOC_SCALER_INFO_Type sc_info;

	dprintk("%s \n", __func__);
	dprintk("Src:  addr:0x%x, 0x%x, 0x%x,  fmt:%d. \n", wmix_info->src_y_addr, wmix_info->src_u_addr, wmix_info->src_v_addr, wmix_info->src_fmt);
	dprintk("Dst:  addr:0x%x, 0x%x, 0x%x,  fmt:%d. \n", wmix_info->dst_y_addr, wmix_info->dst_u_addr, wmix_info->dst_v_addr, wmix_info->dst_fmt);
	dprintk("Size : src_w:%d, src_h:%d, dst_w:%d, dst_h:%d. \n", wmix_info->src_img_width, wmix_info->src_img_height, \
														   wmix_info->dst_img_width, wmix_info->dst_img_height);

	spin_lock_irq(&(wmixer_data.cmd_lock));

	// set to RDMA
	VIOC_RDMA_SetImageAlphaSelect(pWMIX_rdma_base, 1);
	VIOC_RDMA_SetImageAlphaEnable(pWMIX_rdma_base, 1);
	VIOC_RDMA_SetImageFormat(pWMIX_rdma_base, wmix_info->src_fmt);
	VIOC_RDMA_SetImageSize(pWMIX_rdma_base, wmix_info->src_img_width, wmix_info->src_img_height);
	VIOC_RDMA_SetImageOffset(pWMIX_rdma_base, wmix_info->src_fmt, wmix_info->src_img_width);
	VIOC_RDMA_SetImageBase(pWMIX_rdma_base, wmix_info->src_y_addr, wmix_info->src_u_addr,  wmix_info->src_v_addr);

	// set to SC
	sc_info.BYPASS 			= FALSE /* 0 */;
	sc_info.DST_WIDTH 		= wmix_info->dst_img_width;
	sc_info.DST_HEIGHT 		= wmix_info->dst_img_height;
	sc_info.OUTPUT_POS_X 	= 0;
	sc_info.OUTPUT_POS_Y 		= 0;
	sc_info.OUTPUT_WIDTH 	= sc_info.DST_WIDTH;
	sc_info.OUTPUT_HEIGHT 	= sc_info.DST_HEIGHT;
	VIOC_API_SCALER_SetConfig(VIOC_SC0, &sc_info);
	VIOC_API_SCALER_SetPlugIn(VIOC_SC0, VIOC_SC_RDMA_12);
	VIOC_API_SCALER_SetUpdate(VIOC_SC0);
	VIOC_RDMA_SetImageEnable(pWMIX_rdma_base); // SoC guide info.

	// set to WMIX
	VIOC_CONFIG_WMIXPath(WMIX30, 0/* bypass path */);
	VIOC_WMIX_SetSize(pWMIX_wmix_base, sc_info.DST_WIDTH, sc_info.DST_HEIGHT);
	VIOC_WMIX_SetUpdate(pWMIX_wmix_base);

	// set to WDMA
	VIOC_WDMA_SetImageFormat(pWMIX_wdma_base, wmix_info->dst_fmt);
	VIOC_WDMA_SetImageSize(pWMIX_wdma_base, sc_info.DST_WIDTH, sc_info.DST_HEIGHT);
	VIOC_WDMA_SetImageOffset(pWMIX_wdma_base, wmix_info->dst_fmt, wmix_info->dst_img_width);
	VIOC_WDMA_SetImageBase(pWMIX_wdma_base, wmix_info->dst_y_addr, wmix_info->dst_u_addr, wmix_info->dst_v_addr);
	if((wmix_info->src_fmt < SC_IMG_FMT_FCDEC) && (wmix_info->dst_fmt > SC_IMG_FMT_FCDEC)) {
		VIOC_WDMA_SetImageR2YEnable(pWMIX_wdma_base, 1);
	}
	VIOC_WDMA_SetImageEnable(pWMIX_wdma_base, 0/*OFF*/);
	pWMIX_wdma_base->uIRQSTS.nREG = 0xFFFFFFFF; // wdma status register all clear.

	spin_unlock_irq(&(wmixer_data.cmd_lock));

	if(wmix_info->rsp_type == WMIXER_POLLING) {
		ret = wait_event_interruptible_timeout(wmixer_data.poll_wq, wmixer_data.block_operating == 0, msecs_to_jiffies(500));
		if(ret <= 0) {
			 wmixer_data.block_operating = 0;
			printk("wmixer time out: %d, Line: %d. \n", ret, __LINE__);
		}
	}

	return ret;
}
//EXPORT_SYMBOL(tccxxx_wmixer_alpha_scaling_ctrl);

char tccxxx_wmixer_alpha_mixing_ctrl(WMIXER_ALPHABLENDING_TYPE *apb_info)
{
	int ret = 0;

	dprintk("%s \n", __func__);

	spin_lock_irq(&(wmixer_data.cmd_lock));

	// set to VRDMA12
	VIOC_RDMA_SetImageAlphaEnable(pWMIX_rdma_base, 1);
	VIOC_RDMA_SetImageAlphaSelect(pWMIX_rdma_base, 1);
	VIOC_RDMA_SetImageFormat(pWMIX_rdma_base, apb_info->src0_fmt);
	VIOC_RDMA_SetImageSize(pWMIX_rdma_base, apb_info->src0_width, apb_info->src0_height);
	VIOC_RDMA_SetImageOffset(pWMIX_rdma_base, apb_info->src0_fmt, apb_info->src0_width);
	VIOC_RDMA_SetImageBase(pWMIX_rdma_base, apb_info->src0_Yaddr, apb_info->src0_Uaddr, apb_info->src0_Vaddr);

	// set to VRDMA13
	VIOC_RDMA_SetImageAlphaEnable(pWMIX_rdma1_base, 1);
	VIOC_RDMA_SetImageAlphaSelect(pWMIX_rdma1_base, 1);
	VIOC_RDMA_SetImageFormat(pWMIX_rdma1_base, apb_info->src1_fmt);
	VIOC_RDMA_SetImageSize(pWMIX_rdma1_base, apb_info->src1_width, apb_info->src1_height);
	VIOC_RDMA_SetImageOffset(pWMIX_rdma1_base, apb_info->src1_fmt, apb_info->src1_width);
	VIOC_RDMA_SetImageBase(pWMIX_rdma1_base, apb_info->src1_Yaddr, apb_info->src1_Uaddr, apb_info->src1_Vaddr);
	VIOC_RDMA_SetImageEnable(pWMIX_rdma_base); // SoC guide info.
	VIOC_RDMA_SetImageEnable(pWMIX_rdma1_base);

	// set to WMIX
	VIOC_CONFIG_WMIXPath(WMIX30, 1 /* mixing path */);
	VIOC_WMIX_SetSize(pWMIX_wdma_base, apb_info->dst_width, apb_info->dst_height);
	// set to layer0 of WMIX.
	VIOC_API_WMIX_SetOverlayAlphaValueControl(pWMIX_wdma_base, apb_info->src0_layer, apb_info->region, apb_info->src0_acon0, apb_info->src0_acon1);
	VIOC_API_WMIX_SetOverlayAlphaColorControl(pWMIX_wdma_base, apb_info->src0_layer, apb_info->region, apb_info->src0_ccon0, apb_info->src0_ccon1);
	VIOC_API_WMIX_SetOverlayAlphaROPMode(pWMIX_wdma_base, apb_info->src0_layer, apb_info->src0_rop_mode);
	VIOC_API_WMIX_SetOverlayAlphaSelection(pWMIX_wdma_base, apb_info->src0_layer, apb_info->src0_asel);
	VIOC_API_WMIX_SetOverlayAlphaValue(pWMIX_wdma_base, apb_info->src0_layer, apb_info->src0_alpha0, apb_info->src0_alpha1);
	// set to layer1 of WMIX.
	//VIOC_API_WMIX_SetOverlayAlphaValueControl(pWMIX_wdma_base, apb_info->src1_layer, apb_info->region, apb_info->src1_acon0, apb_info->src1_acon1);
	//VIOC_API_WMIX_SetOverlayAlphaColorControl(pWMIX_wdma_base, apb_info->src1_layer, apb_info->region, apb_info->src1_ccon0, apb_info->src1_ccon1);
	//VIOC_API_WMIX_SetOverlayAlphaROPMode(pWMIX_wdma_base, apb_info->src1_layer, apb_info->src1_rop_mode);
	//VIOC_API_WMIX_SetOverlayAlphaSelection(pWMIX_wdma_base, apb_info->src1_layer, apb_info->src1_asel);
	//VIOC_API_WMIX_SetOverlayAlphaValue(pWMIX_wdma_base, apb_info->src1_layer, apb_info->src1_alpha0, apb_info->src1_alpha1);
	// update WMIX.
	VIOC_WMIX_SetUpdate(pWMIX_wdma_base);

	// set to VWRMA
	VIOC_WDMA_SetImageFormat(pWMIX_wdma_base, apb_info->dst_fmt);
	VIOC_WDMA_SetImageSize(pWMIX_wdma_base, apb_info->dst_width, apb_info->dst_height);
	VIOC_WDMA_SetImageOffset(pWMIX_wdma_base, apb_info->dst_fmt, apb_info->dst_width);
	VIOC_WDMA_SetImageBase(pWMIX_wdma_base, apb_info->dst_Yaddr, apb_info->dst_Uaddr, apb_info->dst_Vaddr);
	VIOC_WDMA_SetImageEnable(pWMIX_wdma_base, 0 /* OFF */);
	pWMIX_wdma_base->uIRQSTS.nREG = 0xFFFFFFFF; // wdma status register all clear.

	spin_unlock_irq(&(wmixer_data.cmd_lock));

	if(apb_info->rsp_type == WMIXER_POLLING) {
		ret = wait_event_interruptible_timeout(wmixer_data.poll_wq, wmixer_data.block_operating == 0, msecs_to_jiffies(500));
		if(ret <= 0) {
			 wmixer_data.block_operating = 0;
			printk("wmixer time out: %d, Line: %d. \n", ret, __LINE__);
		}
	}

	return ret;
}
//EXPORT_SYMBOL(tccxxx_wmixer_alpha_mixing_ctrl);
#endif


int tccxxx_wmixer_ctrl(WMIXER_INFO_TYPE *wmix_info)
{
	int ret = 0;
	//VIOC_SCALER_INFO_Type sc_info;

	dprintk("%s():  \n", __func__);
	dprintk("Src  : addr:0x%x 0x%x 0x%x  fmt:%d \n", wmix_info->src_y_addr, wmix_info->src_u_addr, wmix_info->src_v_addr, wmix_info->src_fmt);
	dprintk("Dest: addr:0x%x 0x%x 0x%x  fmt:%d \n", wmix_info->dst_y_addr, wmix_info->dst_u_addr, wmix_info->dst_v_addr, wmix_info->dst_fmt);
	dprintk("Size : W:%d  H:%d \n", wmix_info->img_width, wmix_info->img_height);


	spin_lock_irq(&(wmixer_data.cmd_lock));

	// set to RDMA
	VIOC_RDMA_SetImageFormat(pWMIX_rdma_base, wmix_info->src_fmt);
	VIOC_RDMA_SetImageSize(pWMIX_rdma_base, wmix_info->img_width, wmix_info->img_height);
	VIOC_RDMA_SetImageOffset(pWMIX_rdma_base, wmix_info->src_fmt, wmix_info->img_width);
	VIOC_RDMA_SetImageBase(pWMIX_rdma_base, wmix_info->src_y_addr, wmix_info->src_u_addr,  wmix_info->src_v_addr);

	if (wmix_info->src_fmt > VIOC_IMG_FMT_COMP)	{
		VIOC_RDMA_SetY2RConvertEnable(pWMIX_rdma_base, 1);
	}
	else {
		VIOC_RDMA_SetY2RConvertEnable(pWMIX_rdma_base, 0);
	}
	
	// set to WMIX
	VIOC_WMIX_SetSize(pWMIX_wmix_base, wmix_info->img_width, wmix_info->img_height);
	VIOC_WMIX_SetUpdate(pWMIX_wmix_base);
	VIOC_RDMA_SetImageEnable(pWMIX_rdma_base); // Soc guide info.

	// set to WRMA
	VIOC_WDMA_SetImageFormat(pWMIX_wdma_base, wmix_info->dst_fmt);
	VIOC_WDMA_SetImageSize(pWMIX_wdma_base, wmix_info->img_width, wmix_info->img_height);
	VIOC_WDMA_SetImageOffset(pWMIX_wdma_base, wmix_info->dst_fmt, wmix_info->img_width);

	VIOC_WDMA_SetImageBase(pWMIX_wdma_base, wmix_info->dst_y_addr, wmix_info->dst_u_addr, wmix_info->dst_v_addr);
	VIOC_WDMA_SetImageEnable(pWMIX_wdma_base, 0/*OFF*/);

	if (wmix_info->dst_fmt > VIOC_IMG_FMT_COMP)	{
		VIOC_WDMA_SetImageR2YEnable(pWMIX_wdma_base, 1);
	}
	else {
		VIOC_WDMA_SetImageR2YEnable(pWMIX_wdma_base, 0);
	}
		
	pWMIX_wdma_base->uIRQSTS.nREG = 0xFFFFFFFF; // wdma status register all clear.

	spin_unlock_irq(&(wmixer_data.cmd_lock));

	if(wmix_info->rsp_type == WMIXER_POLLING) {
		ret = wait_event_interruptible_timeout(wmixer_data.poll_wq, wmixer_data.block_operating == 0, msecs_to_jiffies(500));
		if(ret <= 0) {
			 wmixer_data.block_operating = 0;
			printk("wmixer time out: %d, Line: %d. \n", ret, __LINE__);
		}
	}

	return ret;
}

static unsigned int tccxxx_wmixer_poll(struct file *filp, poll_table *wait)
{
	int ret = 0;
	INTERRUPT_DATA_T *wmix_data = (INTERRUPT_DATA_T *)filp->private_data;

	if(wmix_data == NULL) 	return -EFAULT;

	poll_wait(filp, &(wmix_data->poll_wq), wait);

	spin_lock_irq(&(wmix_data->poll_lock));

	if(wmix_data->block_operating == 0) 	ret = (POLLIN|POLLRDNORM);

	spin_unlock_irq(&(wmix_data->poll_lock));

	return ret;
}

static irqreturn_t tccxxx_wmixer_handler(int irq, void *client_data)
{  	
	INTERRUPT_DATA_T *wmix_data = (INTERRUPT_DATA_T *)client_data;
	
	if(pWMIX_wdma_base->uIRQSTS.nREG & VIOC_WDMA_IREQ_EOFR_MASK) {
		dprintk("WDMA Interrupt is VIOC_WDMA_IREQ_EOFR_MASK. \n");
		pWMIX_wdma_base->uIRQSTS.nREG = 0xFFFFFFFF;   // wdma status register all clear.
	}
		dprintk("WDMA Interrupt is VIOC_WDMA_IREQ_EOFR_MASK. \n");

	if(wmix_data->block_operating >= 1) 	
		wmix_data->block_operating = 0;
		
	wake_up_interruptible(&(wmix_data->poll_wq));

	if(wmix_data->block_waiting) 	
		wake_up_interruptible(&wmix_data->cmd_wq);

	return IRQ_HANDLED;
}

long tccxxx_wmixer_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	WMIXER_INFO_TYPE wmix_info;
	#if defined(CONFIG_CHIP_TCC8925S)
		WMIXER_ALPHASCALERING_INFO_TYPE alpha_scalering;
		WMIXER_ALPHABLENDING_TYPE alpha_blending;
	#endif
	INTERRUPT_DATA_T *wmix_data = (INTERRUPT_DATA_T *)file->private_data;
	dprintk("%s():  cmd(%d), block_operating(%d), block_waiting(%d), cmd_count(%d), poll_count(%d). \n", __func__,	\
					cmd, wmix_data->block_operating, wmix_data->block_waiting, wmix_data->cmd_count, wmix_data->poll_count);

	switch(cmd) {
		case TCC_WMIXER_IOCTRL:
		case TCC_WMIXER_IOCTRL_KERNEL:
			mutex_lock(&wmix_data->io_mutex);
			if(wmix_data->block_operating) {
				wmix_data->block_waiting = 1;
				ret = wait_event_interruptible_timeout(wmix_data->cmd_wq, wmix_data->block_operating == 0, msecs_to_jiffies(200));
				if(ret <= 0) {
					wmix_data->block_operating = 0;
					printk("[%d]: wmixer 0 timed_out block_operation:%d!! cmd_count:%d \n", ret, wmix_data->block_waiting, wmix_data->cmd_count);
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
				if(wmix_data->block_operating >= 1) {
					printk("scaler + :: block_operating(%d) - block_waiting(%d) - cmd_count(%d) - poll_count(%d)!!!\n", 	\
								wmix_data->block_operating, wmix_data->block_waiting, wmix_data->cmd_count, wmix_data->poll_count);
				}

				wmix_data->block_waiting = 0;
				wmix_data->block_operating = 1;
				ret = tccxxx_wmixer_ctrl(&wmix_info);
				if(ret < 0) 	wmix_data->block_operating = 0;
			}
			mutex_unlock(&wmix_data->io_mutex);
			return ret;

		#if defined(CONFIG_CHIP_TCC8925S)
			case TCC_WMIXER_ALPHA_SCALING:
				mutex_lock(&wmix_data->io_mutex);

				if(wmix_data->block_operating) {
					wmix_data->block_waiting = 1;
					ret = wait_event_interruptible_timeout(wmix_data->cmd_wq, wmix_data->block_operating == 0, msecs_to_jiffies(200));
					if(ret <= 0) {
						wmix_data->block_operating = 0;
						printk("[%d]: wmixer 0 timed_out block_operation:%d!! cmd_count:%d \n", ret, wmix_data->block_waiting, wmix_data->cmd_count);
					}
					ret = 0;
				}

				if(copy_from_user(&alpha_scalering, (WMIXER_ALPHASCALERING_INFO_TYPE *)arg, sizeof(WMIXER_ALPHASCALERING_INFO_TYPE))) {
						printk(KERN_ALERT "Not Supported copy_from_user(%d). \n", cmd);
						ret = -EFAULT;
				}

				if(ret >= 0) {
					if(wmix_data->block_operating >= 1) {
						printk("scaler + :: block_operating(%d) - block_waiting(%d) - cmd_count(%d) - poll_count(%d)!!!\n", 	\
									wmix_data->block_operating, wmix_data->block_waiting, wmix_data->cmd_count, wmix_data->poll_count);
					}

					wmix_data->block_waiting = 0;
					wmix_data->block_operating = 1;
					ret = tccxxx_wmixer_alpha_scaling_ctrl(&alpha_scalering);
					if(ret < 0) 	wmix_data->block_operating = 0;
				}

				mutex_unlock(&wmix_data->io_mutex);
				return ret;

			case TCC_WMIXER_ALPHA_MIXING:
				mutex_lock(&wmix_data->io_mutex);

				if(wmix_data->block_operating) {
					wmix_data->block_waiting = 1;
					ret = wait_event_interruptible_timeout(wmix_data->cmd_wq, wmix_data->block_operating == 0, msecs_to_jiffies(200));
					if(ret <= 0) {
						wmix_data->block_operating = 0;
						printk("[%d]: wmixer 0 timed_out block_operation:%d!! cmd_count:%d \n", ret, wmix_data->block_waiting, wmix_data->cmd_count);
					}
					ret = 0;
				}

				if(copy_from_user(&alpha_blending, (WMIXER_ALPHABLENDING_TYPE *)arg, sizeof(WMIXER_ALPHABLENDING_TYPE))) {
						printk(KERN_ALERT "Not Supported copy_from_user(%d). \n", cmd);
						ret = -EFAULT;
				}

				if(ret >= 0) {
					if(wmix_data->block_operating >= 1) {
						printk("scaler + :: block_operating(%d) - block_waiting(%d) - cmd_count(%d) - poll_count(%d)!!!\n", 	\
									wmix_data->block_operating, wmix_data->block_waiting, wmix_data->cmd_count, wmix_data->poll_count);
					}

					wmix_data->block_waiting = 0;
					wmix_data->block_operating = 1;
					ret = tccxxx_wmixer_alpha_mixing_ctrl(&alpha_blending);
					if(ret < 0) 	wmix_data->block_operating = 0;
				}

				mutex_unlock(&wmix_data->io_mutex);
				return ret;
		#endif

		default:
			printk(KERN_ALERT "not supported WMIXER IOCTL(0x%x). \n", cmd);
			break;			
	}

	return 0;
}
EXPORT_SYMBOL(tccxxx_wmixer_ioctl);

int tccxxx_wmixer_release(struct inode *inode, struct file *filp)
{
	int ret = 0;
	dprintk("wmixer_release IN:  %d'th, block(%d/%d), cmd(%d), irq(%d). \n", wmixer_data.dev_opened, 			\
			wmixer_data.block_operating, wmixer_data.block_waiting, wmixer_data.cmd_count, wmixer_data.irq_reged);

	if(wmixer_data.dev_opened > 0) 	wmixer_data.dev_opened--;
	if(wmixer_data.dev_opened == 0) {
		if(wmixer_data.block_operating) {
			ret = wait_event_interruptible_timeout(wmixer_data.cmd_wq, wmixer_data.block_operating == 0, msecs_to_jiffies(200));
		}

		if(ret <= 0) {
 			printk("[%d]: wmixer timed_out block_operation: %d, cmd_count: %d. \n", ret, wmixer_data.block_waiting, wmixer_data.cmd_count);
		}

		if(wmixer_data.irq_reged) {
			#if defined(CONFIG_ARCH_TCC893X)
				#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
					free_irq(INT_VIOC_WD3, &wmixer_data);
				#else
					free_irq(INT_VIOC_WD6, &wmixer_data);
				#endif
			#else
				#if defined(CONFIG_CHIP_TCC8925S)
					free_irq(INT_VIOC_WD3, &wmixer_data);
				#else
					free_irq(INT_VIOC_WD4, &wmixer_data);
				#endif
			#endif
			wmixer_data.irq_reged = 0;
		}

		#if defined(CONFIG_ARCH_TCC893X)
			#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[1], (1<<3), (1<<3)); 	 	// WDMA3 reset
				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[1], (1<<12), (1<<12)); 	// WMIX3 reset
				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[0], (1<<12), (1<<12)); 	// RDMA12 reset

				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[0], (1<<12), (0<<12)); 	// RDMA12 reset
				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[1], (1<<12), (0<<12)); 	// WMIX3 reset
				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[1], (1<<3), (0<<3)); 		// WDMA3 reset
			#else
				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[1], (1<<6), (1<<6)); 	 	// WDMA6 reset
				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[1], (1<<14), (1<<14)); 	// WMIX5 reset
				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[0], (1<<16), (1<<16)); 	// RDMA16 reset

				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[0], (1<<16), (0<<16)); 	// RDMA16 reset
				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[1], (1<<14), (0<<14)); 	// WMIX5 reset
				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[1], (1<<6), (0<<6)); 		// WDMA6 reset
			#endif
		#else
			#if defined(CONFIG_CHIP_TCC8925S)
				VIOC_WMIX_SetSWReset(VIOC_WMIX3, VIOC_WMIX_RDMA_12, VIOC_WMIX_WDMA_03);
				VIOC_SC_SetSWReset(VIOC_SC0, 0xFF, 0xFF);
			#else
				VIOC_WMIX_SetSWReset(VIOC_WMIX4, VIOC_WMIX_RDMA_14, VIOC_WMIX_WDMA_04);
			#endif
		#endif

		wmixer_data.block_operating = wmixer_data.block_waiting = 0;
		wmixer_data.poll_count = wmixer_data.cmd_count = 0;
	}

	clk_disable(wmixer_clk);

	dprintk("wmixer_release OUT:  %d'th. \n", wmixer_data.dev_opened);
	return 0;
}
EXPORT_SYMBOL(tccxxx_wmixer_release);

int tccxxx_wmixer_open(struct inode *inode, struct file *filp)
{	
	int ret = 0;
	dprintk("wmixer_open IN:  %d'th, block(%d/%d), cmd(%d), irq(%d). \n", wmixer_data.dev_opened, 				\
			wmixer_data.block_operating, wmixer_data.block_waiting, wmixer_data.cmd_count, wmixer_data.irq_reged);

	clk_enable(wmixer_clk);

	if(!wmixer_data.irq_reged) {
		#if defined(CONFIG_ARCH_TCC893X)
			#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
				pWMIX_rdma_base 	= (volatile PVIOC_RDMA)tcc_p2v((unsigned int)HwVIOC_RDMA12);
				pWMIX_wmix_base 	= (volatile PVIOC_WMIX)tcc_p2v((unsigned int)HwVIOC_WMIX3);
				pWMIX_wdma_base 	= (volatile PVIOC_WDMA)tcc_p2v((unsigned int)HwVIOC_WDMA03);
				pWMIX_ireg_config 	= (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_IREQ);

				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[1], (1<<3), (1<<3)); 	 	// WDMA3 reset
				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[1], (1<<12), (1<<12)); 	// WMIX3 reset
				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[0], (1<<12), (1<<12)); 	// RDMA12 reset

				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[0], (1<<12), (0<<12)); 	// RDMA12 reset
				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[1], (1<<12), (0<<12)); 	// WMIX3 reset
				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[1], (1<<3), (0<<3)); 		// WDMA3 reset

				BITCSET(pWMIX_ireg_config->uMISC1.nREG, (1<<23), (0<<23)); 			// CFG_MISC1 :  STOP_REQ[BIT23] clear 0.

				VIOC_WDMA_SetIreqMask(pWMIX_wdma_base, VIOC_WDMA_IREQ_EOFR_MASK, 0x0);
				ret = request_irq(INT_VIOC_WD3, tccxxx_wmixer_handler, IRQF_SHARED, "wmixer", &wmixer_data);
			#else
				pWMIX_rdma_base 	= (volatile PVIOC_RDMA)tcc_p2v((unsigned int)HwVIOC_RDMA16);
				pWMIX_wmix_base 	= (volatile PVIOC_WMIX)tcc_p2v((unsigned int)HwVIOC_WMIX5);
				pWMIX_wdma_base 	= (volatile PVIOC_WDMA)tcc_p2v((unsigned int)HwVIOC_WDMA06);
				pWMIX_ireg_config 	= (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_IREQ);

				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[1], (1<<6), (1<<6)); 	 	// WDMA6 reset
				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[1], (1<<14), (1<<14)); 	// WMIX5 reset
				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[0], (1<<16), (1<<16)); 	// RDMA16 reset

				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[0], (1<<16), (0<<16)); 	// RDMA16 reset
				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[1], (1<<14), (0<<14)); 	// WMIX5 reset
				BITCSET(pWMIX_ireg_config->uSOFTRESET.nREG[1], (1<<6), (0<<6)); 		// WDMA6 reset

				BITCSET(pWMIX_ireg_config->uMISC1.nREG, (1<<23), (0<<23)); 			// CFG_MISC1 :  STOP_REQ[BIT23] clear 0.

				VIOC_WDMA_SetIreqMask(pWMIX_wdma_base, VIOC_WDMA_IREQ_EOFR_MASK, 0x0);
				ret = request_irq(INT_VIOC_WD6, tccxxx_wmixer_handler, IRQF_SHARED, "wmixer", &wmixer_data);
			#endif
		#else
			#if defined(CONFIG_CHIP_TCC8925S)
				pWMIX_rdma_base = (volatile PVIOC_RDMA)tcc_p2v((unsigned int)HwVIOC_RDMA12);
				pWMIX_rdma1_base = (volatile PVIOC_RDMA)tcc_p2v((unsigned int)HwVIOC_RDMA13);
				pWMIX_scaler_base = (volatile PVIOC_RDMA)tcc_p2v((unsigned int)HwVIOC_SC0);
				pWMIX_wmix_base = (volatile PVIOC_WMIX)tcc_p2v((unsigned int)HwVIOC_WMIX3);
				pWMIX_wdma_base = (volatile PVIOC_WDMA)tcc_p2v((unsigned int)HwVIOC_WDMA03);

				VIOC_WMIX_SetSWReset(VIOC_WMIX3, VIOC_WMIX_RDMA_12, VIOC_WMIX_WDMA_03);
				VIOC_SC_SetSWReset(VIOC_SC0, VIOC_WMIX_RDMA_13, 0xFF);
				VIOC_WDMA_SetIreqMask(pWMIX_wdma_base, VIOC_WDMA_IREQ_EOFR_MASK, 0x0);
				ret = request_irq(INT_VIOC_WD3, tccxxx_wmixer_handler, IRQF_SHARED, "wmixer", &wmixer_data);
			#else
				pWMIX_rdma_base = (volatile PVIOC_RDMA)tcc_p2v((unsigned int)HwVIOC_RDMA14);
				pWMIX_wmix_base = (volatile PVIOC_WMIX)tcc_p2v((unsigned int)HwVIOC_WMIX4);
				pWMIX_wdma_base = (volatile PVIOC_WDMA)tcc_p2v((unsigned int)HwVIOC_WDMA04);

				VIOC_WMIX_SetSWReset(VIOC_WMIX4, VIOC_WMIX_RDMA_14, VIOC_WMIX_WDMA_04);
				VIOC_WDMA_SetIreqMask(pWMIX_wdma_base, VIOC_WDMA_IREQ_EOFR_MASK, 0x0);
				ret = request_irq(INT_VIOC_WD4, tccxxx_wmixer_handler, IRQF_SHARED, "wmixer", &wmixer_data);
			#endif
		#endif

		if(ret) {
			clk_disable(wmixer_clk);
			printk("failed to aquire wmixer request_irq. \n");
			return -EFAULT;
		}

		wmixer_data.irq_reged = 1;
	}

	wmixer_data.dev_opened++;
	filp->private_data = &wmixer_data;
	
	dprintk("wmixer_open OUT:  %d'th. \n", wmixer_data.dev_opened);
	return ret;	
}
EXPORT_SYMBOL(tccxxx_wmixer_open);


static struct file_operations tcc_wmixer_fops = {
	.owner 				= THIS_MODULE,
	.unlocked_ioctl 	= tccxxx_wmixer_ioctl,
	.mmap 				= tccxxx_wmixer_mmap,
	.open 				= tccxxx_wmixer_open,
	.release 			= tccxxx_wmixer_release,
	.poll 				= tccxxx_wmixer_poll,
};

void __exit
tccxxx_wmixer_cleanup(void)
{
	unregister_chrdev(MAJOR_ID, DEVICE_NAME);
	clk_put(wmixer_clk);
	return;
}

static char banner[] __initdata = KERN_INFO "TCC WMIXER Driver Initializing. \n";
static struct class *wmixer_class;

int __init 
tccxxx_wmixer_init(void)
{
	printk(banner);
	
	register_chrdev(MAJOR_ID, DEVICE_NAME, &tcc_wmixer_fops);
	wmixer_class = class_create(THIS_MODULE, DEVICE_NAME);
	device_create(wmixer_class, NULL, MKDEV(MAJOR_ID, MINOR_ID), NULL, DEVICE_NAME);

	memset(&wmixer_data, 0, sizeof(INTERRUPT_DATA_T));

	spin_lock_init(&(wmixer_data.poll_lock));
	spin_lock_init(&(wmixer_data.cmd_lock));

	mutex_init(&(wmixer_data.io_mutex));
	
	init_waitqueue_head(&(wmixer_data.poll_wq));
	init_waitqueue_head(&(wmixer_data.cmd_wq));

	wmixer_clk = clk_get(NULL, "lcdc0");
	BUG_ON(wmixer_clk == NULL);

	return 0;
}


MODULE_AUTHOR("Telechips.");
MODULE_DESCRIPTION("TCC WMIXER Driver");
MODULE_LICENSE("GPL");


arch_initcall(tccxxx_wmixer_init);
module_exit(tccxxx_wmixer_cleanup);


