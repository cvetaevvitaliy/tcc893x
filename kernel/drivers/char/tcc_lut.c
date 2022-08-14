/*
 * linux/drivers/char/tcc_lut.c
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


#include <mach/vioc_rdma.h>
#include <mach/vioc_wdma.h>
#include <mach/vioc_wmix.h>
#include <mach/vioc_config.h>

#include <mach/vioc_api.h>
#include <mach/vioc_disp.h>
#include <mach/tcc_lut_ioctl.h>
#include <mach/vioc_lut.h>

#if defined(CONFIG_ARCH_TCC892X)
#include <mach/vioc_plugin_tcc892x.h>
#endif
#if defined(CONFIG_ARCH_TCC893X)
#include <mach/vioc_plugin_tcc893x.h>
#endif

#define TCC_LUT_DEBUG 		1
#define dprintk(msg...) 		if(TCC_LUT_DEBUG) { printk("TCC LUT: " msg); }

typedef struct _INTERRUPT_DATA_T {
	struct mutex 			hue_table_lock;
	unsigned int  			dev_opened;
} LUT_DRV_DATA_T;

static LUT_DRV_DATA_T lut_drv_data;

int dev_MajorNum;


long tcc_lut_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;

	LUT_DRV_DATA_T *lut_drv_data = (LUT_DRV_DATA_T *)file->private_data;

	switch(cmd) {
		case TCC_SW_LUT_HUE:
			{
				struct VIOC_LUT_INFO_Type lut_cmd;

				if(copy_from_user( &lut_cmd, argp, sizeof(lut_cmd))){
					printk("%s cmd:%d error \n", __func__, cmd);
					return -EFAULT;
				}
				
				if(lut_cmd.BaseAddr != lut_cmd.TarAddr)
				{
					int *base_y, *target_y;

					base_y = ioremap_nocache(lut_cmd.BaseAddr, lut_cmd.ImgSizeWidth * lut_cmd.ImgSizeHeight);
					target_y = ioremap_nocache(lut_cmd.TarAddr, lut_cmd.ImgSizeWidth * lut_cmd.ImgSizeHeight);

					if((base_y != NULL) && (target_y != NULL)) {
						memcpy(target_y, base_y, lut_cmd.ImgSizeWidth * lut_cmd.ImgSizeHeight);
					}

					if(base_y != NULL)
						iounmap(base_y);

					if (target_y != NULL)
						iounmap(target_y);
				}
				
				mutex_lock(&lut_drv_data->hue_table_lock);
				tcc_sw_hue_table_opt(lut_cmd.BaseAddr1, lut_cmd.TarAddr1, lut_cmd.ImgSizeWidth, lut_cmd.ImgSizeHeight);
				mutex_unlock(&lut_drv_data->hue_table_lock);			

			}
			return 0;

		case TCC_SW_LUT_SET:
			{
				struct VIOC_SW_LUT_SET_Type lut_cmd;

				if(copy_from_user(&lut_cmd, argp, sizeof(lut_cmd))){
					printk("%s cmd:%d error \n", __func__, cmd);
					return -EFAULT;
				}

				dprintk("sin : %d, cos : %d, saturation : %d \n", lut_cmd.sin_value, lut_cmd.cos_value, lut_cmd.saturation);
				
				mutex_lock(&lut_drv_data->hue_table_lock);
				tcc_sw_hue_table_set(lut_cmd.sin_value, lut_cmd.cos_value, lut_cmd.saturation);
				mutex_unlock(&lut_drv_data->hue_table_lock);	
			}
		
			return 0;

		case TCC_LUT_SET:
			{
				struct VIOC_LUT_VALUE_SET lut_cmd;
				VIOC_LUT *pLUT =(VIOC_LUT*)tcc_p2v(HwVIOC_LUT);

				if(copy_from_user((void *)&lut_cmd, (const void *)argp, sizeof(lut_cmd)))
					return -EFAULT;

				VIOC_LUT_Set_value(pLUT, lut_cmd.lut_number, lut_cmd.Gamma);
			}
			return 0;
			
		case TCC_LUT_PLUG_IN:
			{
				struct VIOC_LUT_PLUG_IN_SET lut_cmd;
				VIOC_LUT *pLUT =(VIOC_LUT*)tcc_p2v(HwVIOC_LUT);
				
				if(copy_from_user((void *)&lut_cmd, (const void *)arg, sizeof(lut_cmd)))
					return -EFAULT;
				
				if(!lut_cmd.enable) {
					VIOC_LUT_Enable(pLUT, lut_cmd.lut_number, FALSE);
				}
				else 	{
					VIOC_LUT_Plugin(pLUT,  lut_cmd.lut_number, lut_cmd.lut_plug_in_ch);
					VIOC_LUT_Enable(pLUT, lut_cmd.lut_number, TRUE);
				}
			}
			return 0;

		default:
			printk(KERN_ALERT "not supported LUT IOCTL(0x%x). \n", cmd);
			break;			
	}

	return 0;
}
EXPORT_SYMBOL(tcc_lut_ioctl);

int tcc_lut_release(struct inode *inode, struct file *filp)
{
	LUT_DRV_DATA_T *lut_drv_data = (LUT_DRV_DATA_T *)filp->private_data;
	
	if(lut_drv_data->dev_opened > 0) 	
	        lut_drv_data->dev_opened--;
	
	dprintk("lut_release:  %d'th. \n", lut_drv_data->dev_opened);
	return 0;
}
EXPORT_SYMBOL(tcc_lut_release);



int tcc_lut_open(struct inode *inode, struct file *filp)
{	
	int ret = 0;

	lut_drv_data.dev_opened++;
	filp->private_data = &lut_drv_data;
	
	printk("lut_open :  %d'th. \n", lut_drv_data.dev_opened);
	return ret;	
}
EXPORT_SYMBOL(tcc_lut_open);


static struct file_operations tcc_lut_fops = {
	.owner 				= THIS_MODULE,
	.unlocked_ioctl 		= tcc_lut_ioctl,
	.open 				= tcc_lut_open,
	.release 				= tcc_lut_release,
};

#define	DEVICE_NAME		"tcc_lut"

static char banner[] __initdata = KERN_INFO "2013/09/24 ver TCC LUT Driver Initializing. \n";
static struct class *tcc_lut_class;

void __exit
tcc_lut_cleanup(void)
{
	unregister_chrdev(dev_MajorNum, DEVICE_NAME);
}

int __init 
tcc_lut_init(void)
{
	int err;
      printk("\x1b[1;38m  %s \n", __func__);

	printk(banner);
      printk("\x1b[0m");
	  
	dev_MajorNum = register_chrdev(0, DEVICE_NAME, &tcc_lut_fops);

	if (dev_MajorNum < 0) {
		printk("%s: device failed widh %d\n", __func__, dev_MajorNum);
		err = -ENODEV;
	}

	tcc_lut_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(tcc_lut_class)) {
		err = PTR_ERR(tcc_lut_class);

	}
	device_create(tcc_lut_class,NULL,MKDEV(dev_MajorNum, 0),NULL,DEVICE_NAME);

	memset(&lut_drv_data, 0, sizeof(LUT_DRV_DATA_T));

	mutex_init(&(lut_drv_data.hue_table_lock));

	return 0;
}


MODULE_AUTHOR("Telechips.");
MODULE_DESCRIPTION("TCC LUT Driver");
MODULE_LICENSE("GPL");

module_init(tcc_lut_init);
module_exit(tcc_lut_cleanup);


