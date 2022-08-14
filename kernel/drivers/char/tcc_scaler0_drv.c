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
#include <linux/miscdevice.h>

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

#include "tcc_scaler0_drv.h"
#include <mach/tcc_scaler_ctrl.h>


static int debug	   		= 0;
#define dprintk(msg...)	if(debug) { 	printk( "TCC_Scaler0:  " msg); 	}
#define SCALER_PLATFORM_DEVICE

#if defined(CONFIG_ARCH_TCC893X)
volatile PVIOC_RDMA 			pRDMABase;
//volatile PVIOC_SC 			pSCALERBase;
volatile PVIOC_WMIX 			pWIXBase;
volatile PVIOC_WDMA 			pWDMABase;
volatile PVIOC_IREQ_CONFIG 	pIREQConfig;
#endif

typedef struct _intr_data_t {
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
} intr_data_t;
static intr_data_t sc_data;

#define DEVICE_NAME			"scaler"
#if defined(SCALER_PLATFORM_DEVICE)
#define DEVICE_ID			244
#else
#define MAJOR_ID			200
#define MINOR_ID			1
#endif

static struct clk *sc0_clk;

#if defined(CONFIG_ARCH_TCC893X)
extern void tccxxx_GetAddress(unsigned char format, unsigned int base_Yaddr, unsigned int src_imgx, unsigned int  src_imgy,
								unsigned int start_x, unsigned int start_y, unsigned int* Y, unsigned int* U,unsigned int* V);
#endif

extern int range_is_allowed(unsigned long pfn, unsigned long size);
static int tccxxx_scaler0_mmap(struct file *file, struct vm_area_struct *vma)
{
	if(range_is_allowed(vma->vm_pgoff, vma->vm_end - vma->vm_start) < 0) {
		printk(KERN_ERR	 "%s():  This address is not allowed. \n", __func__);
		return -EAGAIN;
	}

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	if(remap_pfn_range(vma,vma->vm_start, vma->vm_pgoff , vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
		printk(KERN_ERR	 "%s():  Virtual address page port error. \n", __func__);
		return -EAGAIN;
	}

	vma->vm_ops	= NULL;
	vma->vm_flags 	|= VM_IO;
	vma->vm_flags 	|= VM_RESERVED;
	
	return 0;
}

char tccxxx_scaler0_run(SCALER_TYPE *scale_img)
{
	int ret = 0;
	VIOC_SCALER_INFO_Type pScalerInfo;
	unsigned int pSrcBase0 = 0, pSrcBase1 = 0, pSrcBase2 = 0;
	unsigned int crop_width;
	
	dprintk("%s():  IN. \n", __func__);

	spin_lock_irq(&(sc_data.cmd_lock));

	#if defined(CONFIG_MACH_TCC8920ST) || defined(CONFIG_MACH_TCC8930ST)
		VIOC_RDMA_SetImageAlphaSelect(pRDMABase, 1);
		VIOC_RDMA_SetImageAlphaEnable(pRDMABase, 1);
	#else
		VIOC_RDMA_SetImageAlphaEnable(pRDMABase, 1);
	#endif
	VIOC_RDMA_SetImageFormat(pRDMABase, scale_img->src_fmt);

	//interlaced frame process ex) MPEG2
	if(scale_img->interlaced)
	{
		VIOC_RDMA_SetImageSize(pRDMABase, (scale_img->src_winRight - scale_img->src_winLeft), (scale_img->src_winBottom - scale_img->src_winTop)/2);
		VIOC_RDMA_SetImageOffset(pRDMABase, scale_img->src_fmt, scale_img->src_ImgWidth*2);
	}
	else
	{
		VIOC_RDMA_SetImageSize(pRDMABase, (scale_img->src_winRight - scale_img->src_winLeft), (scale_img->src_winBottom - scale_img->src_winTop));
		VIOC_RDMA_SetImageOffset(pRDMABase, scale_img->src_fmt, scale_img->src_ImgWidth);
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
	
	VIOC_RDMA_SetImageBase(pRDMABase, (unsigned int)pSrcBase0, (unsigned int)pSrcBase1, (unsigned int)pSrcBase2);

	pScalerInfo.BYPASS 			= FALSE;
	pScalerInfo.DST_WIDTH 		= (scale_img->dest_winRight - scale_img->dest_winLeft);
	pScalerInfo.DST_HEIGHT 		= (scale_img->dest_winBottom - scale_img->dest_winTop);
	pScalerInfo.OUTPUT_POS_X 		= 0;
	pScalerInfo.OUTPUT_POS_Y 		= 0;
	pScalerInfo.OUTPUT_WIDTH 		= pScalerInfo.DST_WIDTH;
	pScalerInfo.OUTPUT_HEIGHT 	= pScalerInfo.DST_HEIGHT;
	VIOC_API_SCALER_SetConfig(VIOC_SC1, &pScalerInfo);
	#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
		VIOC_API_SCALER_SetPlugIn(VIOC_SC1, VIOC_SC_RDMA_16);
	#else
		VIOC_API_SCALER_SetPlugIn(VIOC_SC1, VIOC_SC_RDMA_17);
	#endif
	VIOC_API_SCALER_SetUpdate(VIOC_SC1);
	VIOC_RDMA_SetImageEnable(pRDMABase); // SoC guide info.

	#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
		VIOC_CONFIG_WMIXPath(WMIX50, 1); // wmixer op is mixing mode.
	#else
		VIOC_CONFIG_WMIXPath(WMIX60, 1); // wmixer op is mixing mode.
	#endif
	VIOC_WMIX_SetSize(pWIXBase, pScalerInfo.DST_WIDTH, pScalerInfo.DST_HEIGHT);	
	VIOC_WMIX_SetUpdate(pWIXBase);

	VIOC_WDMA_SetImageFormat(pWDMABase, scale_img->dest_fmt);
	VIOC_WDMA_SetImageSize(pWDMABase, pScalerInfo.DST_WIDTH, pScalerInfo.DST_HEIGHT);
	VIOC_WDMA_SetImageOffset(pWDMABase, scale_img->dest_fmt, scale_img->dest_ImgWidth);
	VIOC_WDMA_SetImageBase(pWDMABase, (unsigned int)scale_img->dest_Yaddr, (unsigned int)scale_img->dest_Uaddr, (unsigned int)scale_img->dest_Vaddr);
	if((scale_img->src_fmt < SC_IMG_FMT_FCDEC) && (scale_img->dest_fmt > SC_IMG_FMT_FCDEC)) {
		VIOC_WDMA_SetImageR2YEnable(pWDMABase, 1);
	} else {
		VIOC_WDMA_SetImageR2YEnable(pWDMABase, scale_img->wdma_r2y);
		if(scale_img->wdma_r2y)
			VIOC_WDMA_SetImageR2YMode(pWDMABase, 0);
	}
	VIOC_WDMA_SetImageEnable(pWDMABase, 0);
	pWDMABase->uIRQSTS.nREG = 0xFFFFFFFF; // wdma status register all clear.

	spin_unlock_irq(&(sc_data.cmd_lock));

	if(scale_img->responsetype  == SCALER_POLLING)	{
		ret = wait_event_interruptible_timeout(sc_data.poll_wq,  sc_data.block_operating == 0, msecs_to_jiffies(500));
		if(ret <= 0) {
			sc_data.block_operating = 0;
			printk("%s():  time out(%d), line(%d). \n", __func__, ret, __LINE__);
		}		
	}
	else if(scale_img->responsetype  == SCALER_NOWAIT)	{
		if(scale_img->viqe_onthefly & 0x2)
			 sc_data.block_operating = 0;
	}

	return ret;
}

static unsigned int tccxxx_scaler0_poll(struct file *filp, poll_table *wait)
{
	int ret = 0;
	intr_data_t *msc_data = (intr_data_t *)filp->private_data;

	if(msc_data == NULL) {
		return -EFAULT;
	}

	poll_wait(filp, &(msc_data->poll_wq), wait);

	spin_lock_irq(&(msc_data->poll_lock));

	if (msc_data->block_operating == 0) 	{
		ret = (POLLIN|POLLRDNORM);
	}

	spin_unlock_irq(&(msc_data->poll_lock));
	
	return ret;
}

#if defined(CONFIG_ARCH_TCC892X) || defined(CONFIG_ARCH_TCC893X)
void tccxxx_convert_image_format(SCALER_TYPE *pScalerInfo)
{
	dprintk("before: src_fmt=%d, dst_fmt=%d. \n", pScalerInfo->src_fmt, pScalerInfo->dest_fmt);
	switch((unsigned char)pScalerInfo->src_fmt) {
		case SCALER_YUV422_sq0:
			pScalerInfo->src_fmt = SC_IMG_FMT_YCbCr422_SEQ_YUYV;
			break;
		case SCALER_YUV422_sq1:
			pScalerInfo->src_fmt = SC_IMG_FMT_YCbCr422_SEQ_UYVY;
			break;
		case SCALER_YUV422_sp:
			pScalerInfo->src_fmt = SC_IMG_FMT_YCbCr422_SEP;
			break;
		case SCALER_YUV420_sp:
			pScalerInfo->src_fmt = SC_IMG_FMT_YCbCr420_SEP;
			break;
		case SCALER_YUV422_inter:
			pScalerInfo->src_fmt = SC_IMG_FMT_YCbCr422_INT_TYPE0;
			break;
		case SCALER_YUV420_inter:
			pScalerInfo->src_fmt = SC_IMG_FMT_YCbCr420_INT_TYPE0;
			break;
		case SCALER_RGB565:
			pScalerInfo->src_fmt = SC_IMG_FMT_RGB565;
			break;
		case SCALER_RGB555:
			//pScalerInfo->src_fmt = SC_IMG_FMT_RGB555;
			//break;
		case SCALER_RGB454:
			pScalerInfo->src_fmt = SC_IMG_FMT_RGB555;
			break;
		case SCALER_RGB444:
			pScalerInfo->src_fmt = SC_IMG_FMT_RGB444;
			break;
		case SCALER_ARGB8888:
			pScalerInfo->src_fmt = SC_IMG_FMT_ARGB8888;
			break;
	}

	switch((unsigned char)pScalerInfo->dest_fmt) {
		case SCALER_YUV422_sq0:
			pScalerInfo->dest_fmt = SC_IMG_FMT_YCbCr422_SEQ_YUYV;
			break;
		case SCALER_YUV422_sq1:
			pScalerInfo->dest_fmt = SC_IMG_FMT_YCbCr422_SEQ_UYVY;
			break;
		case SCALER_YUV422_sp:
			pScalerInfo->dest_fmt = SC_IMG_FMT_YCbCr422_SEP;
			break;
		case SCALER_YUV420_sp:
			pScalerInfo->dest_fmt = SC_IMG_FMT_YCbCr420_SEP;
			break;
		case SCALER_YUV422_inter:
			pScalerInfo->dest_fmt = SC_IMG_FMT_YCbCr422_INT_TYPE0;
			break;
		case SCALER_YUV420_inter:
			pScalerInfo->dest_fmt = SC_IMG_FMT_YCbCr420_INT_TYPE0;
			break;
		case SCALER_RGB565:
			pScalerInfo->dest_fmt = SC_IMG_FMT_RGB565;
			break;
		case SCALER_RGB555:
			//pScalerInfo->dest_fmt = SC_IMG_FMT_RGB555;
			//break;
		case SCALER_RGB454:
			pScalerInfo->dest_fmt = SC_IMG_FMT_RGB555;
			break;
		case SCALER_RGB444:
			pScalerInfo->dest_fmt = SC_IMG_FMT_RGB444;
			break;
		case SCALER_ARGB8888:
			pScalerInfo->dest_fmt = SC_IMG_FMT_ARGB8888;
			break;
	}
	dprintk("after: src_fmt=%d, dst_fmt=%d. \n", pScalerInfo->src_fmt, pScalerInfo->dest_fmt);
}
#endif

static irqreturn_t tccxxx_scaler0_handler(int irq, void *client_data)
{  	
	intr_data_t *msc_data = (intr_data_t *)client_data;

	if(pWDMABase->uIRQSTS.nREG & VIOC_WDMA_IREQ_EOFR_MASK) {
		dprintk("WDMA Interrupt is VIOC_WDMA_IREQ_EOFR_MASK. \n");
		pWDMABase->uIRQSTS.nREG = 0xFFFFFFFF;   // wdma status register all clear.
	}
	dprintk("%s():  block_operating(%d), block_waiting(%d), cmd_count(%d), poll_count(%d). \n", __func__, 	\
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

long tccxxx_scaler0_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	SCALER_TYPE scaler_v;
	#if defined(CONFIG_ARCH_TCC893X)
	SCALER_PLUGIN_Type scaler_plugin;
	VIOC_SCALER_INFO_Type pScalerInfo;
	#endif
	intr_data_t *msc_data = (intr_data_t *)file->private_data;

	dprintk("%s():  cmd(%d), block_operating(%d), block_waiting(%d), cmd_count(%d), poll_count(%d). \n", __func__, 	\
			cmd, msc_data->block_operating, msc_data->block_waiting, msc_data->cmd_count, msc_data->poll_count);

	switch(cmd) {
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
					printk(KERN_ALERT "%s():  Not Supported copy_from_user(%d). \n", __func__, cmd);
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
				ret = tccxxx_scaler0_run(&scaler_v);
				if(ret < 0) msc_data->block_operating = 0;
			}
			mutex_unlock(&msc_data->io_mutex);
			return ret;

		case TCC_SCALER_VIOC_PLUGIN:
			mutex_lock(&msc_data->io_mutex);
			if(copy_from_user(&scaler_plugin,(SCALER_PLUGIN_Type *)arg, sizeof(SCALER_PLUGIN_Type))) {
				printk(KERN_ALERT "%s():  Not Supported copy_from_user(%d)\n", __func__, cmd);
				ret = -EFAULT;
			}

			// set to scaler & plug in.
			pScalerInfo.BYPASS 			= scaler_plugin.bypass_mode;
			pScalerInfo.SRC_WIDTH 		= scaler_plugin.src_width;
			pScalerInfo.SRC_HEIGHT 		= scaler_plugin.src_height;
			pScalerInfo.DST_WIDTH 		= scaler_plugin.dst_width;
			pScalerInfo.DST_HEIGHT 		= scaler_plugin.dst_height;
			pScalerInfo.OUTPUT_POS_X 		= scaler_plugin.dst_win_left;
			pScalerInfo.OUTPUT_POS_Y 		= scaler_plugin.dst_win_top;
			pScalerInfo.OUTPUT_WIDTH 		= (scaler_plugin.dst_width  - scaler_plugin.dst_win_left);
			pScalerInfo.OUTPUT_HEIGHT 	= (scaler_plugin.dst_height - scaler_plugin.dst_win_top);

			VIOC_SC_SetSWReset(scaler_plugin.scaler_no, 0xFF/*RDMA*/, 0xFF/*WDMA*/);
			VIOC_API_SCALER_SetConfig(scaler_plugin.scaler_no, &pScalerInfo);
			ret = VIOC_API_SCALER_SetPlugIn(scaler_plugin.scaler_no, scaler_plugin.plugin_path);
			VIOC_API_SCALER_SetUpdate(scaler_plugin.scaler_no);			
			mutex_unlock(&msc_data->io_mutex);
			return ret;

		case TCC_SCALER_VIOC_PLUGOUT:
			VIOC_RDMA_SetImageIntl(pRDMABase, 0);
			ret = VIOC_API_SCALER_SetPlugOut((unsigned int)arg);
			return ret;

		default:
			printk(KERN_ALERT "%s():  Not Supported SCALER0_IOCTL(%d). \n", __func__, cmd);
			break;			
	}

	return 0;
}
EXPORT_SYMBOL(tccxxx_scaler0_ioctl);

int tccxxx_scaler0_release(struct inode *inode, struct file *filp)
{
	int ret = 0;
	dprintk("%s():  In -open(%d), block(%d), wait(%d), cmd(%d), irq(%d) \n", __func__, sc_data.dev_opened, sc_data.block_operating, \
			sc_data.block_waiting, sc_data.cmd_count, sc_data.irq_reged);

	if(sc_data.dev_opened > 0) {
		sc_data.dev_opened--;
	}

	if(sc_data.dev_opened == 0) {
		if(sc_data.block_operating) {
			ret = wait_event_interruptible_timeout(sc_data.cmd_wq, sc_data.block_operating == 0, msecs_to_jiffies(200));
			if(ret <= 0) {
	 			printk("%s(%d):  timed_out block_operation:%d, cmd_count:%d. \n", __func__, ret, sc_data.block_waiting, sc_data.cmd_count);
			}
		}

		#if defined(CONFIG_ARCH_TCC893X)
			if(sc_data.irq_reged) {
				#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
					free_irq(INT_VIOC_WD6, &sc_data);
				#else
					free_irq(INT_VIOC_WD8, &sc_data);
				#endif
				sc_data.irq_reged = 0;
			}

			VIOC_CONFIG_PlugOut(VIOC_SC1);
			#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
				BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<6), (1<<6)); 	// WDMA6 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<14), (1<<14)); 	// WMIX5 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<29), (1<<29)); 	// SCALER1 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<16), (1<<16)); 	// RDMA16 reset

				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<16), (0<<16)); 	// RDMA16 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<29), (0<<29)); 	// SCALER1 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<14), (0<<14)); 	// WMIX5 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<6), (0<<6)); 	// WDMA6 reset
			#else
				BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<8), (1<<8)); 	// WDMA8 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<15), (1<<15)); 	// WMIX6 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<29), (1<<29)); 	// SCALER1 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<17), (1<<17)); 	// RDMA17 reset

				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<17), (0<<17)); 	// RDMA17 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<29), (0<<29)); 	// SCALER1 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<15), (0<<15)); 	// WMIX6 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<8), (0<<8)); 	// WDMA8 reset
			#endif
		#endif

		sc_data.block_operating = sc_data.block_waiting = 0;
		sc_data.poll_count = sc_data.cmd_count = 0;
	}

	clk_disable(sc0_clk);
	dprintk("%s():  Out - open(%d). \n", __func__, sc_data.dev_opened);

	return 0;
}
EXPORT_SYMBOL(tccxxx_scaler0_release);

int tccxxx_scaler0_open(struct inode *inode, struct file *filp)
{	
	int ret = 0;
	dprintk("%s():  In -open(%d), block(%d), wait(%d), cmd(%d), irq(%d) \n", __func__, sc_data.dev_opened, sc_data.block_operating, \
			sc_data.block_waiting, sc_data.cmd_count, sc_data.irq_reged);

	clk_enable(sc0_clk);

	if(!sc_data.irq_reged) {
		#if defined(CONFIG_ARCH_TCC893X)
			#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
				pRDMABase 		= (volatile PVIOC_RDMA)tcc_p2v((unsigned int)HwVIOC_RDMA16);
				pWIXBase 		= (volatile PVIOC_WMIX)tcc_p2v((unsigned int)HwVIOC_WMIX5);
				pWDMABase 		= (volatile PVIOC_WDMA)tcc_p2v((unsigned int)HwVIOC_WDMA06);
				pIREQConfig 		= (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_IREQ);

				BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<6), (1<<6)); 	// WDMA6 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<14), (1<<14)); 	// WMIX5 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<29), (1<<29)); 	// SCALER1 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<16), (1<<16)); 	// RDMA16 reset

				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<16), (0<<16)); 	// RDMA16 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<29), (0<<29)); 	// SCALER1 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<14), (0<<14)); 	// WMIX5 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<6), (0<<6)); 	// WDMA6 reset

				BITCSET(pIREQConfig->uMISC1.nREG, (1<<23), (0<<23)); 			// CFG_MISC1 :  STOP_REQ[BIT23] clear 0.

				VIOC_WDMA_SetIreqMask(pWDMABase, VIOC_WDMA_IREQ_EOFR_MASK, 0x0);
				ret = request_irq(INT_VIOC_WD6, tccxxx_scaler0_handler, IRQF_SHARED, "scaler0", &sc_data);
			#else
				pRDMABase 		= (volatile PVIOC_RDMA)tcc_p2v((unsigned int)HwVIOC_RDMA17);
				pWIXBase 		= (volatile PVIOC_WMIX)tcc_p2v((unsigned int)HwVIOC_WMIX6);
				pWDMABase 		= (volatile PVIOC_WDMA)tcc_p2v((unsigned int)HwVIOC_WDMA08);
				pIREQConfig 		= (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_IREQ);

				BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<8), (1<<8)); 	// WDMA8 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<15), (1<<15)); 	// WMIX6 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<29), (1<<29)); 	// SCALER1 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<17), (1<<17)); 	// RDMA17 reset

				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<17), (0<<17)); 	// RDMA17 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<29), (0<<29)); 	// SCALER1 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<15), (0<<15)); 	// WMIX6 reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<8), (0<<8)); 	// WDMA8 reset

				BITCSET(pIREQConfig->uMISC1.nREG, (1<<23), (0<<23)); 			// CFG_MISC1 :  STOP_REQ[BIT23] clear 0.

				VIOC_WDMA_SetIreqMask(pWDMABase, VIOC_WDMA_IREQ_EOFR_MASK, 0x0);
				ret = request_irq(INT_VIOC_WD8, tccxxx_scaler0_handler, IRQF_SHARED, "scaler0", &sc_data);
			#endif
		#endif

		if (ret) {
			clk_disable(sc0_clk);
			printk("Failed to aquire scaler0 irq. \n");
			return -EFAULT;
		}

		sc_data.irq_reged = 1;
	}

	sc_data.dev_opened++;
	filp->private_data = &sc_data;
	dprintk("%s():  Out - open(%d). \n", __func__, sc_data.dev_opened);
	
	return ret;	
}
EXPORT_SYMBOL(tccxxx_scaler0_open);

static struct file_operations tccxxx_scaler0_fops = 
{
#if !defined(SCALER_PLATFORM_DEVICE)
	.owner 			= THIS_MODULE,
#endif
	.unlocked_ioctl 	= tccxxx_scaler0_ioctl,
	.mmap 			= tccxxx_scaler0_mmap,
	.open 			= tccxxx_scaler0_open,
	.release 			= tccxxx_scaler0_release,
	.poll 			= tccxxx_scaler0_poll,
};

#if defined(SCALER_PLATFORM_DEVICE)
static struct miscdevice tccxxx_scaler0_misc_device = {
	DEVICE_ID,
	DEVICE_NAME,
	&tccxxx_scaler0_fops,
};

static int __init tccxxx_scaler0_probe(struct platform_device *pdev)
{
	memset(&sc_data, 0, sizeof(intr_data_t));

	spin_lock_init(&sc_data.poll_lock);
	spin_lock_init(&sc_data.cmd_lock);

	mutex_init(&sc_data.io_mutex);

	init_waitqueue_head(&sc_data.poll_wq);
	init_waitqueue_head(&sc_data.cmd_wq);

	#if defined(CONFIG_ARCH_TCC893X)
		sc0_clk = clk_get(NULL, "ddi");
		BUG_ON(sc0_clk == NULL);
	#endif

	if(misc_register(&tccxxx_scaler0_misc_device)) {
		dprintk(KERN_WARNING "tcc_scaler0_drv:  couldn't register device %d. \n", DEVICE_ID);
		return -EBUSY;
	}

	return 0;
}

static int tccxxx_scaler0_remove(struct platform_device *pdev)
{
	misc_deregister(&tccxxx_scaler0_misc_device);
	return 0;
}

static int tccxxx_scaler0_suspend(struct platform_device *pdev, pm_message_t state)
{
	printk("\n tccxxx_scaler0_drv:  Suspend In. \n");

	// todo : 

	printk("\n tccxxx_scaler0_drv:  Suspend Out. \n");

	return 0;
}

static int tccxxx_scaler0_resume(struct platform_device *pdev)
{
	printk("\n tccxxx_scaler0_drv:  Resume In. \n");

	if(sc_data.dev_opened > 0) {
		#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
			BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<6), (1<<6)); 	// WDMA6 reset
			BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<14), (1<<14)); 	// WMIX5 reset
			BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<29), (1<<29)); 	// SCALER1 reset
			BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<16), (1<<16)); 	// RDMA16 reset

			BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<16), (0<<16)); 	// RDMA16 reset
			BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<29), (0<<29)); 	// SCALER1 reset
			BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<14), (0<<14)); 	// WMIX5 reset
			BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<6), (0<<6)); 	// WDMA6 reset
		#else
			BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<8), (1<<8)); 	// WDMA8 reset
			BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<15), (1<<15)); 	// WMIX6 reset
			BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<29), (1<<29)); 	// SCALER1 reset
			BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<17), (1<<17)); 	// RDMA17 reset

			BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<17), (0<<17)); 	// RDMA17 reset
			BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (1<<29), (0<<29)); 	// SCALER1 reset
			BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<15), (0<<15)); 	// WMIX6 reset
			BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (1<<8), (0<<8)); 	// WDMA8 reset
		#endif
	}

	printk("\n tccxxx_scaler0_drv:  Resume Out. \n");

	return 0;
}

static struct platform_device tcc_scaler0_device = {
	.name 	= "tcc_scaler0_driver",
	.dev		= {
		.release 	= NULL,
	},
	.id	= 0,
};

static struct platform_driver tcc_scaler0_driver = {
	.driver 	= {
		.name 	= "tcc_scaler0_driver",
		.owner 	= THIS_MODULE,
	},
	.probe 		= tccxxx_scaler0_probe,
	.remove 		= tccxxx_scaler0_remove,
	.suspend 		= tccxxx_scaler0_suspend,
	.resume 		= tccxxx_scaler0_resume,
};
#endif

void __exit tccxxx_scaler0_cleanup(void)
{
	#if defined(SCALER_PLATFORM_DEVICE)
		platform_driver_unregister(&tcc_scaler0_driver);
		platform_device_unregister(&tcc_scaler0_device);
	#else
		unregister_chrdev(MAJOR_ID, DEVICE_NAME);
	#endif

	clk_put(sc0_clk);
	return;
}

static char banner[] __initdata = KERN_INFO "TCC Scaler0 Device Driver Initializing....... \n";
#if !defined(SCALER_PLATFORM_DEVICE)
static struct class *tcc_sc0_class;
#endif

int __init tccxxx_scaler0_init(void)
{
	printk(banner);

	#if defined(SCALER_PLATFORM_DEVICE)
		platform_device_register(&tcc_scaler0_device);
		platform_driver_register(&tcc_scaler0_driver);
	#else
		register_chrdev(MAJOR_ID, DEVICE_NAME, &tccxxx_scaler0_fops);
		tcc_sc0_class = class_create(THIS_MODULE, DEVICE_NAME);
		device_create(tcc_sc0_class, NULL, MKDEV(MAJOR_ID, MINOR_ID), NULL, DEVICE_NAME);

		memset(&sc_data, 0, sizeof(intr_data_t));

		spin_lock_init(&(sc_data.poll_lock));
		spin_lock_init(&(sc_data.cmd_lock));

		mutex_init(&(sc_data.io_mutex));

		init_waitqueue_head(&(sc_data.poll_wq));
		init_waitqueue_head(&(sc_data.cmd_wq));

		#if defined(CONFIG_ARCH_TCC893X)
			sc0_clk = clk_get(NULL, "lcdc0");
			BUG_ON(sc0_clk == NULL);
		#endif
	#endif

	return 0;
}

module_init(tccxxx_scaler0_init)
module_exit(tccxxx_scaler0_cleanup)

MODULE_AUTHOR("Telechips.");
MODULE_DESCRIPTION("TCC scaler driver");
MODULE_LICENSE("GPL");



