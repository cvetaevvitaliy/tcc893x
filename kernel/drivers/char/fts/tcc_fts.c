/****************************************************************************
FileName    : kernel/drivers/char/fts/tcc_fts.c
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
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/ctype.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/blkpg.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/buffer_head.h>
#include <linux/syscalls.h>
#include <linux/kthread.h>
#include <linux/completion.h>

#include <asm/segment.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/scatterlist.h>
#include <asm/mach-types.h>
#include <asm/memory.h>
#include <asm/dma.h>

#include <mach/tcc_fts_ioctl.h>
#include "tcc_fts.h"

#define DEV_NAME	"fts"

static int major;
static struct class *fts_class;
static struct device *fts_dev;

struct completion fts_completion;

static int reset_crash_counter(void)
{
	struct file *filp = NULL;
	mm_segment_t oldfs;
	int ret, err;

	unsigned char crash_count = 0, recovery_count = 0;
	unsigned char data[512];
	unsigned char page_cnt;

	loff_t offset = FTS_CRASH_COUNT_OFFSET;

	oldfs = get_fs();
	set_fs(get_ds());

	filp = filp_open(FTS_PATH_MISC, O_RDWR, GFP_KERNEL);

	if(IS_ERR(filp)){
		DBG(KERN_INFO "FTS Block Device Open Fail !!\n");
		err = PTR_ERR(filp);
		goto out;
	}
	
	for(page_cnt=0; page_cnt<FTS_CRASH_COUNT_PAGE_NUM; page_cnt++)
	{
		offset = FTS_CRASH_COUNT_OFFSET + FTS_CRASH_COUNT_PAGE_SIZE*page_cnt;
		
		ret = vfs_read(filp, (char*)&data[0], 512, &offset);

		if(ret < 0){
			DBG(KERN_INFO "FTS Block Device Read Fail!!\n");
			goto err;
		}

		if((data[0] == 0xff && data[1] == 0xff && data[2] == 0xff) || (data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00))
		{
			break;
		}
		else
		{
			if(data[0] == 'T' && data[1] == 'C' && data[2] == 'C')
			{
				crash_count = data[3];
				recovery_count = data[4];
			}
		}
	}

	if(page_cnt >= FTS_CRASH_COUNT_PAGE_NUM)
	{
		page_cnt = 0;
	}
	else
	{
		if(page_cnt)
			page_cnt--;
	}

	offset = FTS_CRASH_COUNT_OFFSET + FTS_CRASH_COUNT_PAGE_SIZE*page_cnt;

	DBG(KERN_INFO "crash_count: %d, recovery_count: %d, page_cnt: %d\n", crash_count, recovery_count, page_cnt);

	if(crash_count == 0){
		DBG(KERN_INFO "IGNOR to Reset \n");
	}else{
		DBG(KERN_INFO "Reset Crash Count: %d-->0\n", crash_count);

		memset(data, 0x00, 512);

		data[0] = 'T';
		data[1] = 'C';
		data[2] = 'C';
		crash_count = data[3] = 0;
		recovery_count = data[4] = 0;

		ret = vfs_write(filp, (char*)&data[0], 512, &offset);

		if(ret < 0){ 
			DBG(KERN_INFO "FTS Block Device Write Fail !!\n");
			goto err;
		}
	}

	filp_close(filp, NULL);
	set_fs(oldfs);
	return 0;

err:
	filp_close(filp, NULL);
	set_fs(oldfs);

out:
	return -1;
}

static int save_output_setting(char *output_data)
{
	struct file *filp = NULL;
	mm_segment_t oldfs;
	int ret, err, count;

	unsigned char data[512];

	loff_t offset = FTS_OUTPUT_SETTING_OFFSET;

	oldfs = get_fs();
	set_fs(get_ds());

	filp = filp_open(FTS_PATH_TCC, O_RDWR, GFP_KERNEL);

	if(IS_ERR(filp)){
		DBG(KERN_INFO "FTS Block Device Open Fail !!\n");
		err = PTR_ERR(filp);
		goto out;
	}

	for(count=0; count<FTS_OUTPUT_SETTING_PAGE_NUM; count++)
	{
		offset = FTS_OUTPUT_SETTING_OFFSET + FTS_OUTPUT_SETTING_PAGE_SIZE*count;
		
		ret = vfs_read(filp, (char*)&data[0], 512, &offset);

		if(ret < 0){
			DBG(KERN_INFO "FTS Block Device Read Fail!!\n");
			goto err;
		}

		if((data[0] == 0xff && data[1] == 0xff && data[2] == 0xff) || (data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00))
		{
			memset(data, 0x00, 512);
			memcpy(data, output_data, 16);

			//printk("%s, page_num: %d, %c%c%c %d %d %d %d %d %d %d %d (1)\n", __func__, count, 
			//		data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10]);

			DBG(KERN_INFO "page_num: %d, output_mode: %d, output_resolution: %d\n", count, data[3], data[4]);
		
			offset = FTS_OUTPUT_SETTING_OFFSET + FTS_OUTPUT_SETTING_PAGE_SIZE*count;
	
			ret = vfs_write(filp, (char*)&data[0], 512, &offset);

			if(ret < 0){ 
				DBG(KERN_INFO "FTS Block Device Write Fail!!(1)\n");
				goto err;
			}

			break;
		}
	}

	if(count >= FTS_OUTPUT_SETTING_PAGE_NUM)
	{
		memset(data, 0x00, 512);

		for(count=0; count<FTS_OUTPUT_SETTING_PAGE_NUM; count++)
		{
			offset = FTS_OUTPUT_SETTING_OFFSET + FTS_OUTPUT_SETTING_PAGE_SIZE*count;

			ret = vfs_write(filp, (char*)&data[0], 512, &offset);

			if(ret < 0){ 
				DBG(KERN_INFO "FTS Block Device Write Fail!!(2)\n");
				goto err;
			}
		}		

		memcpy(data, output_data, 16);

		//printk("%s, page_num: %d, %c%c%c %d %d %d %d %d %d %d %d (2)\n", __func__, 0, 
		//		data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10]);

		DBG(KERN_INFO "page_num: %d, output_mode: %d, output_resolution: %d\n", 0, data[3], data[4]);
		
		offset = FTS_OUTPUT_SETTING_OFFSET;

		ret = vfs_write(filp, (char*)&data[0], 512, &offset);

		if(ret < 0){ 
			DBG(KERN_INFO "FTS Block Device Write Fail!!(3)\n");
			goto err;
		}
	}

	filp_close(filp, NULL);
	set_fs(oldfs);
	return 0;

err:
	filp_close(filp, NULL);
	set_fs(oldfs);

out:
	return -1;
}

int fts_reset_crash_counter(void *data)
{
	reset_crash_counter();
	complete_and_exit(&fts_completion, 0);

	return 0;
}

int fts_save_output_setting(void *data)
{
	save_output_setting(data);
	complete_and_exit(&fts_completion, 0);

	return 0;
}

static long fts_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int res;
	char data[16];
	struct task_struct *tsk;

	switch(cmd)
	{
		case CRASH_COUNTER_RESET:
			DBG(KERN_INFO "BootUP Complete And Reset Crash Counter to Zero \n");

			tsk = kthread_run(fts_reset_crash_counter, NULL, "crash_counter");
			wait_for_completion(&fts_completion);
			break;

		case OUTPUT_SETTING_SET:
			DBG(KERN_INFO "Save Output Settings \n");

			if(copy_from_user((void *)&data[0], (const void *)arg, sizeof(data)))
				return -EFAULT;

			tsk = kthread_run(fts_save_output_setting, &data[0], "output_setting");
			wait_for_completion(&fts_completion);
			break;

		default:
			printk("err: Unkown Command(%d) \n", cmd);
			res = -1;
			break;
	}

	return res;
}

static int fts_open(struct inode *inode, struct file *filp)
{
	DBG(KERN_INFO "FTS Drvier Opened \n");
	try_module_get(THIS_MODULE);
	return 0;
}

static int fts_release(struct inode *inode, struct file *filp)
{
	DBG(KERN_INFO "FTS Drvier Releaseed \n");
	module_put(THIS_MODULE);
	return 0;
}

static struct file_operations fts_fops = 
{
	.owner 		= THIS_MODULE,
	.open 		= fts_open,
	.release 	= fts_release,
	.read		= NULL,
	.write		= NULL,
	.unlocked_ioctl = fts_ioctl
};

static int __init fts_init(void)
{
	void *ptr_err;

	DBG(KERN_INFO "Start FTS Driver Initial\n");

	init_completion(&fts_completion);

	if((major = register_chrdev(0, DEV_NAME, &fts_fops))<0)
		return major;

	fts_class = class_create(THIS_MODULE, DEV_NAME);
	if(IS_ERR(ptr_err = fts_class))
		goto err2;

	fts_dev = device_create(fts_class, NULL, MKDEV(major , 0), NULL, DEV_NAME);
	if(IS_ERR(ptr_err = fts_dev))
		goto err;

	return 0;

err:
	class_destroy(fts_class);
err2:
	unregister_chrdev(major, DEV_NAME);
	return PTR_ERR(ptr_err);
}

static void __exit fts_exit(void)
{
	device_destroy(fts_class, MKDEV(major,0));
	class_unregister(fts_class);
	class_destroy(fts_class);
	unregister_chrdev(major,DEV_NAME);

	DBG(KERN_INFO "FTS Driver Exit !\n");
}

module_init(fts_init);
module_exit(fts_exit);

MODULE_AUTHOR("Telechips");
MODULE_DESCRIPTION("TCC FTS Driver");
MODULE_LICENSE("GPL");
