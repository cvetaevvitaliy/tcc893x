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
#if defined(CONFIG_ARCH_TCC893X)
#include <mach/irqs.h>
#include <mach/vioc_global.h>
#include <mach/vioc_ireq.h>
#include <mach/vioc_rdma.h>
#include <mach/vioc_wdma.h>
#include <mach/vioc_wmix.h>
#include <mach/vioc_config.h>
#include <mach/vioc_scaler.h>
#include <mach/vioc_api.h>
#include <mach/tccfb_address.h>
#endif

#include <linux/poll.h>

#include "tcc_scaler1_drv.h"
#include <mach/tcc_scaler_ctrl.h>

static int debug	   		= 0;
#define dprintk(msg...)	if(debug) { 	printk( "TCC_Scaler1:  " msg); 	}

#if defined(CONFIG_ARCH_TCC893X)
volatile PVIOC_RDMA 			pRDMABase1;
volatile PVIOC_WMIX 			pWIXBase1;
volatile PVIOC_WDMA 			pWDMABase1;
volatile PVIOC_IREQ_CONFIG 	pIREQConfig1;
#endif

typedef struct _intr_data_t {
//wait for Poll
	wait_queue_head_t 	poll_wq;
	spinlock_t 			poll_lock;
	unsigned int 			poll_count;

//wait for Ioctl command
	wait_queue_head_t 	cmd_wq;
	spinlock_t 			cmd_lock;
	unsigned int 			cmd_count;

	struct mutex 			io_mutex;
	unsigned char 		block_operating;
	unsigned char 		block_waiting;
	unsigned char 		irq_reged;
	unsigned int  			dev_opened;
} intr_data_t;
static intr_data_t sc1_data;

#define DEVICE_NAME			"scaler1"
#define MAJOR_ID			201
#define MINOR_ID			1

static struct clk *sc1_clk;

extern int range_is_allowed(unsigned long pfn, unsigned long size);
static int tccxxx_scaler1_mmap(struct file *file, struct vm_area_struct *vma)
{
	if(range_is_allowed(vma->vm_pgoff, vma->vm_end - vma->vm_start) < 0){
		printk(KERN_ERR  "%s():  This address is not allowed. \n", __func__);
		return -EAGAIN;
	}

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	if(remap_pfn_range(vma,vma->vm_start, vma->vm_pgoff , vma->vm_end - vma->vm_start, vma->vm_page_prot))
	{
		printk(KERN_ERR  "%s():  Virtual address page port error. \n", __func__);
		return -EAGAIN;
	}

	vma->vm_ops	= NULL;
	vma->vm_flags 	|= VM_IO;
	vma->vm_flags 	|= VM_RESERVED;
	
	return 0;
}

char tccxxx_scaler1_run(SCALER_TYPE *scale_img)
{
	char ret = 0;
	VIOC_SCALER_INFO_Type pScalerInfo;
	unsigned int pSrcBase0 = 0, pSrcBase1 = 0, pSrcBase2 = 0;
	unsigned int crop_width;

	dprintk("%s():  \n", __func__);
	dprintk("Src:  addr:0x%x 0x%x 0x%x  fmt:%d \n", (unsigned int)scale_img->src_Yaddr, (unsigned int)scale_img->src_Uaddr, 	\
			(unsigned int)scale_img->src_Vaddr, scale_img->src_fmt);
	dprintk("Dest:  addr:0x%x 0x%x 0x%x  fmt:%d \n", (unsigned int)scale_img->dest_Yaddr, (unsigned int)scale_img->dest_Uaddr, 	\
			(unsigned int)scale_img->dest_Vaddr, scale_img->dest_fmt);
	dprintk("Size:  Src_W:%d, Src_H:%d, Dst_W:%d, Dst_H:%d. \n", scale_img->src_ImgWidth, scale_img->src_ImgHeight, 	\
			scale_img->dest_ImgWidth, scale_img->dest_ImgHeight);

	spin_lock_irq(&(sc1_data.cmd_lock));

	#if defined(CONFIG_MACH_TCC8920ST) || defined(CONFIG_MACH_TCC8930ST)
	VIOC_RDMA_SetImageAlphaSelect(pRDMABase1, 1);
	VIOC_RDMA_SetImageAlphaEnable(pRDMABase1, 1);
	#else
	VIOC_RDMA_SetImageAlphaEnable(pRDMABase1, 1);
	#endif
	VIOC_RDMA_SetImageFormat(pRDMABase1, scale_img->src_fmt);

	if(scale_img->interlaced)
	{
		VIOC_RDMA_SetImageSize(pRDMABase1, (scale_img->src_winRight - scale_img->src_winLeft), (scale_img->src_winBottom - scale_img->src_winTop)/2);
		VIOC_RDMA_SetImageOffset(pRDMABase1, scale_img->src_fmt, scale_img->src_ImgWidth*2);
	}
	else
	{
		VIOC_RDMA_SetImageSize(pRDMABase1, (scale_img->src_winRight - scale_img->src_winLeft), (scale_img->src_winBottom - scale_img->src_winTop));
		VIOC_RDMA_SetImageOffset(pRDMABase1, scale_img->src_fmt, scale_img->src_ImgWidth);
	}

	pSrcBase0 = (unsigned int)scale_img->src_Yaddr;
	pSrcBase1 = (unsigned int)scale_img->src_Uaddr;
	pSrcBase2 = (unsigned int)scale_img->src_Vaddr;
	if(scale_img->src_fmt > SC_IMG_FMT_ARGB6666) { // address limitation!!
		dprintk("%s():  src addr is not allocate. \n", __func__);
		crop_width 				= scale_img->src_winRight - scale_img->src_winLeft;
		scale_img->src_winLeft 	= (scale_img->src_winLeft>>3)<<3;
		scale_img->src_winRight = scale_img->src_winLeft + crop_width;
		scale_img->src_winRight = scale_img->src_winLeft + (scale_img->src_winRight - scale_img->src_winLeft);
		tccxxx_GetAddress(scale_img->src_fmt, (unsigned int)scale_img->src_Yaddr, 		\
									scale_img->src_ImgWidth, scale_img->src_ImgHeight, 	\
									scale_img->src_winLeft, scale_img->src_winTop, 		\
									&pSrcBase0, &pSrcBase1, &pSrcBase2);
	}
	
	VIOC_RDMA_SetImageBase(pRDMABase1, (unsigned int)pSrcBase0, (unsigned int)pSrcBase1, (unsigned int)pSrcBase2);

	pScalerInfo.BYPASS 			= FALSE;
	pScalerInfo.DST_WIDTH 		= (scale_img->dest_winRight - scale_img->dest_winLeft);
	pScalerInfo.DST_HEIGHT 		= (scale_img->dest_winBottom - scale_img->dest_winTop);
	pScalerInfo.OUTPUT_POS_X 		= 0;
	pScalerInfo.OUTPUT_POS_Y 		= 0;
	pScalerInfo.OUTPUT_WIDTH 		= pScalerInfo.DST_WIDTH;
	pScalerInfo.OUTPUT_HEIGHT 	= pScalerInfo.DST_HEIGHT;
	#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S) || defined(CONFIG_MACH_TCC8930ST)
		VIOC_API_SCALER_SetConfig(VIOC_SC2, &pScalerInfo);
		VIOC_API_SCALER_SetPlugIn(VIOC_SC2, VIOC_SC_RDMA_07);
		VIOC_API_SCALER_SetUpdate(VIOC_SC2);
	#else
		VIOC_API_SCALER_SetConfig(VIOC_SC0, &pScalerInfo);
		VIOC_API_SCALER_SetPlugIn(VIOC_SC0, VIOC_SC_RDMA_03);
		VIOC_API_SCALER_SetUpdate(VIOC_SC0);
	#endif
	VIOC_RDMA_SetImageEnable(pRDMABase1); // SoC guide info.

	#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S) || defined(CONFIG_MACH_TCC8930ST)
		VIOC_CONFIG_WMIXPath(WMIX13, 1); // wmixer op is mixing mode.
	#else
		VIOC_CONFIG_WMIXPath(WMIX03, 1); // wmixer op is mixing mode.
	#endif
	VIOC_WMIX_SetSize(pWIXBase1, pScalerInfo.DST_WIDTH, pScalerInfo.DST_HEIGHT);
	VIOC_WMIX_SetUpdate(pWIXBase1);

	 VIOC_WDMA_SetImageFormat(pWDMABase1, scale_img->dest_fmt);
	VIOC_WDMA_SetImageSize(pWDMABase1, pScalerInfo.DST_WIDTH, pScalerInfo.DST_HEIGHT);
	VIOC_WDMA_SetImageOffset(pWDMABase1, scale_img->dest_fmt, scale_img->dest_ImgWidth);
	VIOC_WDMA_SetImageBase(pWDMABase1, (unsigned int)scale_img->dest_Yaddr, (unsigned int)scale_img->dest_Uaddr, (unsigned int)scale_img->dest_Vaddr);
	if((scale_img->src_fmt < SC_IMG_FMT_FCDEC) && (scale_img->dest_fmt > SC_IMG_FMT_FCDEC)) {
		VIOC_WDMA_SetImageR2YEnable(pWDMABase1, 1);
 	}
	else {
		VIOC_WDMA_SetImageR2YEnable(pWDMABase1, scale_img->wdma_r2y);
		if(scale_img->wdma_r2y)
			VIOC_WDMA_SetImageR2YMode(pWDMABase1, 0);
	}
	VIOC_WDMA_SetImageEnable(pWDMABase1, 0);
	pWDMABase1->uIRQSTS.nREG = 0xFFFFFFFF; // wdma status register all clear.

	spin_unlock_irq(&(sc1_data.cmd_lock));

	if(scale_img->responsetype == SCALER_POLLING) {
		ret = wait_event_interruptible_timeout(sc1_data.poll_wq,  sc1_data.block_operating == 0, msecs_to_jiffies(500));
		if(ret <= 0) {
			sc1_data.block_operating = 0;
			printk("%s():  time out(%d), line(%d). \n", __func__, ret, __LINE__);
		}
	} else if(scale_img->responsetype == SCALER_NOWAIT) {
		if(scale_img->viqe_onthefly & 0x2) sc1_data.block_operating = 0;
	}
	
	return ret;
}

char tccxxx_scaler1_data_copy_run(SCALER_DATA_COPY_TYPE *copy_info)
{
	int ret = 0;
	VIOC_SCALER_INFO_Type pScalerInfo;

	dprintk("%s():  \n", __func__);
	dprintk("Src  : addr:0x%x 0x%x 0x%x  fmt:%d \n", copy_info->src_y_addr, copy_info->src_u_addr, copy_info->src_v_addr, copy_info->src_fmt);
	dprintk("Dest: addr:0x%x 0x%x 0x%x  fmt:%d \n", copy_info->dst_y_addr, copy_info->dst_u_addr, copy_info->dst_v_addr, copy_info->dst_fmt);
	dprintk("Size : W:%d  H:%d \n", copy_info->img_width, copy_info->img_height);


	spin_lock_irq(&(sc1_data.cmd_lock));

	VIOC_RDMA_SetImageFormat(pRDMABase1, copy_info->src_fmt);
	VIOC_RDMA_SetImageSize(pRDMABase1, copy_info->img_width, copy_info->img_height);
	VIOC_RDMA_SetImageOffset(pRDMABase1, copy_info->src_fmt, copy_info->img_width);
	VIOC_RDMA_SetImageBase(pRDMABase1, (unsigned int)copy_info->src_y_addr, (unsigned int)copy_info->src_u_addr,  (unsigned int)copy_info->src_v_addr);

	pScalerInfo.BYPASS 			= TRUE;
	pScalerInfo.DST_WIDTH 		= copy_info->img_width;
	pScalerInfo.DST_HEIGHT 		= copy_info->img_height;
	pScalerInfo.OUTPUT_POS_X 		= 0;
	pScalerInfo.OUTPUT_POS_Y 		= 0;
	pScalerInfo.OUTPUT_WIDTH 		= pScalerInfo.DST_WIDTH;
	pScalerInfo.OUTPUT_HEIGHT 	= pScalerInfo.DST_HEIGHT;
	#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)|| defined(CONFIG_MACH_TCC8930ST)
		VIOC_API_SCALER_SetConfig(VIOC_SC2, &pScalerInfo);
		VIOC_API_SCALER_SetPlugIn(VIOC_SC2, VIOC_SC_RDMA_07);
		VIOC_API_SCALER_SetUpdate(VIOC_SC2);
	#else
		VIOC_API_SCALER_SetConfig(VIOC_SC0, &pScalerInfo);
		VIOC_API_SCALER_SetPlugIn(VIOC_SC0, VIOC_SC_RDMA_03);
		VIOC_API_SCALER_SetUpdate(VIOC_SC0);
	#endif
	VIOC_RDMA_SetImageEnable(pRDMABase1); // SoC guide info.

	#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S) || defined(CONFIG_MACH_TCC8930ST)
		VIOC_CONFIG_WMIXPath(WMIX13, 1); // wmixer op is mixing mode.
	#else
		VIOC_CONFIG_WMIXPath(WMIX03, 1); // wmixer op is mixing mode.
	#endif
	VIOC_WMIX_SetSize(pWIXBase1, copy_info->img_width, copy_info->img_height);
	VIOC_WMIX_SetUpdate(pWIXBase1);

	VIOC_WDMA_SetImageFormat(pWDMABase1, copy_info->dst_fmt);
	VIOC_WDMA_SetImageSize(pWDMABase1, copy_info->img_width, copy_info->img_height);
	VIOC_WDMA_SetImageOffset(pWDMABase1, copy_info->dst_fmt, copy_info->img_width);
	VIOC_WDMA_SetImageBase(pWDMABase1, (unsigned int)copy_info->dst_y_addr, (unsigned int)copy_info->dst_u_addr, (unsigned int)copy_info->dst_v_addr);
	VIOC_WDMA_SetImageEnable(pWDMABase1, 0/*OFF*/);
	pWDMABase1->uIRQSTS.nREG = 0xFFFFFFFF; // wdma status register all clear.

	spin_unlock_irq(&(sc1_data.cmd_lock));

	if(copy_info->rsp_type == SCALER_POLLING) {
		ret = wait_event_interruptible_timeout(sc1_data.poll_wq, sc1_data.block_operating == 0, msecs_to_jiffies(500));
		if(ret <= 0) {
			 sc1_data.block_operating = 0;
			printk("wmixer time out: %d, Line: %d. \n", ret, __LINE__);
		}
	}

	return ret;
}

static unsigned int tccxxx_scaler1_poll(struct file *filp, poll_table *wait)
{
	int ret = 0;
	intr_data_t *msc_data = (intr_data_t *)filp->private_data;

	if (msc_data == NULL) {
		return -EFAULT;
	}
	
	poll_wait(filp, &(msc_data->poll_wq), wait);

	spin_lock_irq(&(msc_data->poll_lock));

	if (msc_data->block_operating == 0) 	{
		ret =  (POLLIN|POLLRDNORM);
	}

	spin_unlock_irq(&(msc_data->poll_lock));
	
	return ret;
}

static irqreturn_t tccxxx_scaler1_handler(int irq, void *client_data)
{  	
	intr_data_t *msc_data = (intr_data_t *)client_data;
	
	if(pWDMABase1->uIRQSTS.nREG & VIOC_WDMA_IREQ_EOFR_MASK) {
		dprintk("WDMA Interrupt is VIOC_WDMA_IREQ_EOFR_MASK. \n");
		pWDMABase1->uIRQSTS.nREG = 0xFFFFFFFF;   // wdma status register all clear.
	}
	dprintk("%s():  tccxxx_scaler_handler :0  block_operating(%d) - block_waiting(%d) - cmd_count(%d) - poll_count(%d)!!!\n", __func__, 	\
			msc_data->block_operating, msc_data->block_waiting, msc_data->cmd_count, msc_data->poll_count);
	
	if(msc_data->block_operating >= 1) {
		msc_data->block_operating = 0;
	}

	wake_up_interruptible(&(msc_data->poll_wq));

	if(msc_data->block_waiting) {
		wake_up_interruptible(&msc_data->cmd_wq);
	}

	return IRQ_HANDLED;
}

#if defined(CONFIG_ARCH_TCC892X) || defined(CONFIG_ARCH_TCC893X)
extern void tccxxx_convert_image_format(SCALER_TYPE *pScalerInfo);
#endif
long tccxxx_scaler1_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	SCALER_TYPE scaler_v;
	SCALER_DATA_COPY_TYPE copy_info;
	intr_data_t *msc_data = (intr_data_t *)file->private_data;
	
	dprintk("%s():  cmd(%d), block_operating(%d), block_waiting(%d), cmd_count(%d), poll_count(%d). \n", __func__, 	\
		cmd, msc_data->block_operating, msc_data->block_waiting, msc_data->cmd_count, msc_data->poll_count);	

	switch(cmd)
	{
		case TCC_SCALER_IOCTRL:			
		case TCC_SCALER_IOCTRL_KERENL:
			mutex_lock(&msc_data->io_mutex);
			if(msc_data->block_operating) {
				msc_data->block_waiting = 1;
				ret = wait_event_interruptible_timeout(msc_data->cmd_wq, msc_data->block_operating == 0, msecs_to_jiffies(200));
				if(ret <= 0) {
					msc_data->block_operating = 0;
					printk("%s(%d):  timed_out block_operation(%d), cmd_count(%d). \n", __func__, ret, msc_data->block_waiting, msc_data->cmd_count);
				}
				ret = 0;
			}

			if(cmd == TCC_SCALER_IOCTRL_KERENL) {
				memcpy(&scaler_v,(SCALER_TYPE*)arg,sizeof(scaler_v));
			} else {
				if(copy_from_user(&scaler_v,(SCALER_TYPE*)arg,sizeof(scaler_v))) {
					printk(KERN_ALERT "%s():  Not Supported copy_from_user(%d)\n", __func__, cmd);
					ret = -EFAULT;
				}
			}

			if(ret >= 0) {
				if(msc_data->block_operating >= 1) {
					printk("%s():  block_operating(%d), block_waiting(%d), cmd_count(%d), poll_count(%d). \n", __func__, 	\
						msc_data->block_operating, msc_data->block_waiting, msc_data->cmd_count, msc_data->poll_count);
				}

				#if defined(CONFIG_ARCH_TCC892X) || defined(CONFIG_ARCH_TCC893X)
				tccxxx_convert_image_format(&scaler_v);
				#endif

				msc_data->block_waiting = 0;
				msc_data->block_operating = 1;
				ret = tccxxx_scaler1_run(&scaler_v);

				if(ret < 0) {
					msc_data->block_operating = 0;	
				}
			}
			mutex_unlock(&msc_data->io_mutex);
			return ret;

		case TCC_SCALER_VIOC_DATA_COPY:
			mutex_lock(&msc_data->io_mutex);
			if(msc_data->block_operating) {
				msc_data->block_waiting = 1;
				ret = wait_event_interruptible_timeout(msc_data->cmd_wq, msc_data->block_operating == 0, msecs_to_jiffies(200));
				if(ret <= 0) {
					msc_data->block_operating = 0;
					printk("%s(%d):  wmixer 0 timed_out block_operation(%d), cmd_count(%d). \n", __func__, ret, msc_data->block_waiting, msc_data->cmd_count);
				}
				ret = 0;
			}

			if(copy_from_user(&copy_info, (SCALER_DATA_COPY_TYPE *)arg, sizeof(SCALER_DATA_COPY_TYPE))) {
				printk(KERN_ALERT "%s():  Not Supported copy_from_user(%d)\n", __func__, cmd);
				ret = -EFAULT;
			}

			if(ret >= 0) {
				if(msc_data->block_operating >= 1) {
					printk("%s():  block_operating(%d), block_waiting(%d), cmd_count(%d), poll_count(%d). \n", __func__, 	\
						msc_data->block_operating, msc_data->block_waiting, msc_data->cmd_count, msc_data->poll_count);
				}

				msc_data->block_waiting = 0;
				msc_data->block_operating = 1;
				ret = tccxxx_scaler1_data_copy_run(&copy_info);
				if(ret < 0) 	msc_data->block_operating = 0;
			}
			mutex_unlock(&msc_data->io_mutex);
			return ret;

		default:
			printk(KERN_ALERT "%s():  Not Supported SCALER1_IOCTL(%d)\n", __func__, cmd);
			break;			
	}

	return 0;
}
EXPORT_SYMBOL(tccxxx_scaler1_ioctl);

int tccxxx_scaler1_release(struct inode *inode, struct file *filp)
{
	int ret = 0;
	dprintk("%s():  In -open(%d), block(%d), wait(%d), cmd(%d), irq(%d) \n", __func__, sc1_data.dev_opened, sc1_data.block_operating, 	\
			sc1_data.block_waiting, sc1_data.cmd_count, sc1_data.irq_reged);

	if(sc1_data.dev_opened > 0) {
		sc1_data.dev_opened--;
	}

	if(sc1_data.dev_opened == 0) {
		if(sc1_data.block_operating) {
			ret = wait_event_interruptible_timeout(sc1_data.cmd_wq, sc1_data.block_operating == 0, msecs_to_jiffies(200));
			if(ret <= 0)	{
	 			printk("%s(%d):  line :%d scaler 1 timed_out block_operation:%d!! cmd_count:%d \n", __func__, ret, __LINE__, sc1_data.block_operating, sc1_data.cmd_count);
			}
		}

		if(sc1_data.irq_reged)	{
			#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S) || defined(CONFIG_MACH_TCC8930ST)
				free_irq(INT_VIOC_WD1, &sc1_data);
			#else
				free_irq(INT_VIOC_WD0, &sc1_data);
			#endif
			sc1_data.irq_reged = 0;
		}

		#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S) || defined(CONFIG_MACH_TCC8930ST)
			VIOC_CONFIG_PlugOut(VIOC_SC2);
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[1], (1<<1), (1<<1)); 	// WDMA1 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[1], (1<<10), (1<<10)); 	// WMIX1 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[0], (1<<30), (1<<30)); 	// SCALER2 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[0], (1<<7), (1<<7)); 	// RDMA7 reset

			BITCSET(pIREQConfig1->uSOFTRESET.nREG[0], (1<<7), (0<<7)); 	// RDMA7 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[0], (1<<30), (0<<30)); 	// SCALER2 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[1], (1<<10), (0<<10)); 	// WMIX1 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[1], (1<<1), (0<<1)); 	// WDMA1 reset
		#else
			VIOC_CONFIG_PlugOut(VIOC_SC0);
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[1], 1, 1); 				// WDMA0 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[1], (1<<9), (1<<9)); 	// WMIX0 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[0], (1<<28), (1<<28)); 	// SCALER0 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[0], (1<<3), (1<<3)); 	// RDMA3 reset

			BITCSET(pIREQConfig1->uSOFTRESET.nREG[0], (1<<3), (0<<3)); 	// RDMA3 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[0], (1<<28), (0<<28)); 	// SCALER0 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[1], (1<<9), (0<<9)); 	// WMIX0 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[1], 1, 0); 				// WDMA0 reset
		#endif

		sc1_data.block_operating = sc1_data.block_waiting = 0;
		sc1_data.poll_count = sc1_data.cmd_count = 0;
	}

	clk_disable(sc1_clk);
	dprintk("%s():  Out - open(%d). \n", __func__, sc1_data.dev_opened);

	return 0;
}
EXPORT_SYMBOL(tccxxx_scaler1_release);

int tccxxx_scaler1_open(struct inode *inode, struct file *filp)
{	
	int ret = 0;

	dprintk("%s():  In -open(%d), block(%d), wait(%d), cmd(%d), irq(%d) \n", __func__,  sc1_data.dev_opened, sc1_data.block_operating, 	\
			sc1_data.block_waiting, sc1_data.cmd_count, sc1_data.irq_reged);

	clk_enable(sc1_clk);
	
	if(!sc1_data.irq_reged) {
		#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S) || defined(CONFIG_MACH_TCC8930ST)
			pRDMABase1 	= (volatile PVIOC_RDMA)tcc_p2v((unsigned int)HwVIOC_RDMA07);
			pWIXBase1 		= (volatile PVIOC_WMIX)tcc_p2v((unsigned int)HwVIOC_WMIX1);
			pWDMABase1 	= (volatile PVIOC_WDMA)tcc_p2v((unsigned int)HwVIOC_WDMA01);
			pIREQConfig1 	= (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_IREQ);

			BITCSET(pIREQConfig1->uSOFTRESET.nREG[1], (1<<1), (1<<1)); 	// WDMA1 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[1], (1<<10), (1<<10)); 	// WMIX1 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[0], (1<<30), (1<<30)); 	// SCALER2 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[0], (1<<7), (1<<7)); 	// RDMA7 reset

			BITCSET(pIREQConfig1->uSOFTRESET.nREG[0], (1<<7), (0<<7)); 	// RDMA7 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[0], (1<<30), (0<<30)); 	// SCALER2 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[1], (1<<10), (0<<10)); 	// WMIX1 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[1], (1<<1), (0<<1)); 	// WDMA1 reset

			BITCSET(pIREQConfig1->uMISC1.nREG, (1<<23), (0<<23)); 		// CFG_MISC1 :  STOP_REQ[BIT23] clear 0.

			VIOC_WDMA_SetIreqMask(pWDMABase1, VIOC_WDMA_IREQ_EOFR_MASK, 0x0);
			ret = request_irq(INT_VIOC_WD1, tccxxx_scaler1_handler, IRQF_SHARED, "scaler1", &sc1_data);
		#else
			pRDMABase1 	= (volatile PVIOC_RDMA)tcc_p2v((unsigned int)HwVIOC_RDMA03);
			pWIXBase1 		= (volatile PVIOC_WMIX)tcc_p2v((unsigned int)HwVIOC_WMIX0);
			pWDMABase1 	= (volatile PVIOC_WDMA)tcc_p2v((unsigned int)HwVIOC_WDMA00);
			pIREQConfig1 	= (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_IREQ);

			BITCSET(pIREQConfig1->uSOFTRESET.nREG[1], 1, 1); 				// WDMA0 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[1], (1<<9), (1<<9)); 	// WMIX0 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[0], (1<<28), (1<<28)); 	// SCALER0 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[0], (1<<3), (1<<3)); 	// RDMA03 reset

			BITCSET(pIREQConfig1->uSOFTRESET.nREG[0], (1<<3), (0<<3)); 	// RDMA03 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[0], (1<<28), (0<<28)); 	// SCALER0 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[1], (1<<9), (0<<9)); 	// WMIX0 reset
			BITCSET(pIREQConfig1->uSOFTRESET.nREG[1], 1, 0); 				// WDMA0 reset

			BITCSET(pIREQConfig1->uMISC1.nREG, (1<<23), (0<<23)); 		// CFG_MISC1 :  STOP_REQ[BIT23] clear 0.

			VIOC_WDMA_SetIreqMask(pWDMABase1, VIOC_WDMA_IREQ_EOFR_MASK, 0x0);
			ret = request_irq(INT_VIOC_WD0, tccxxx_scaler1_handler, IRQF_SHARED, "scaler1", &sc1_data);
		#endif
		
		if (ret) {
			clk_disable(sc1_clk);
			printk("Failed to aquire scaler1 irq. \n");
			return -EFAULT;
		}

		sc1_data.irq_reged = 1;
	}

	sc1_data.dev_opened++;
	filp->private_data = &sc1_data;
	
	dprintk("%s():  Out - open(%d). \n", __func__,  sc1_data.dev_opened);
	
	return ret;	
}
EXPORT_SYMBOL(tccxxx_scaler1_open);


static struct file_operations tccxxx_scaler1_fops = 
{
	.owner			= THIS_MODULE,
	.unlocked_ioctl	= tccxxx_scaler1_ioctl,
	.mmap			= tccxxx_scaler1_mmap,
	.open			= tccxxx_scaler1_open,
	.release			= tccxxx_scaler1_release,
	.poll				= tccxxx_scaler1_poll,
};

void __exit tccxxx_scaler1_cleanup(void)
{
	unregister_chrdev(MAJOR_ID, DEVICE_NAME);
	clk_put(sc1_clk);
	return;
}

static char banner[] __initdata = KERN_INFO "TCC Scaler1 Device Driver Initializing....... \n";
static struct class *tcc_sc1_class;

int __init tccxxx_scaler1_init(void)
{
	printk(banner);
	
	register_chrdev(MAJOR_ID, DEVICE_NAME, &tccxxx_scaler1_fops);
	tcc_sc1_class = class_create(THIS_MODULE, DEVICE_NAME);
	device_create(tcc_sc1_class, NULL, MKDEV(MAJOR_ID, MINOR_ID), NULL, DEVICE_NAME);

	memset(&sc1_data, 0, sizeof(intr_data_t));
	spin_lock_init(&(sc1_data.poll_lock));
	spin_lock_init(&(sc1_data.cmd_lock));
	mutex_init(&(sc1_data.io_mutex));
	init_waitqueue_head(&(sc1_data.poll_wq));
	init_waitqueue_head(&(sc1_data.cmd_wq));

	sc1_clk = clk_get(NULL, "lcdc0");
	BUG_ON(sc1_clk == NULL);
	
	return 0;
}


MODULE_AUTHOR("Telechips.");
MODULE_DESCRIPTION("TCC scaler1 driver");
MODULE_LICENSE("GPL");


arch_initcall(tccxxx_scaler1_init);
module_exit(tccxxx_scaler1_cleanup);


