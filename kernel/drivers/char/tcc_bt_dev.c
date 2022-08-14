/*
 * File:		drivers/char/tcc_bt_dev.c
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>
#include <linux/device.h>

#include <mach/bsp.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <asm/system.h>
#include <linux/tcc_bt_dev.h>
#include <mach/tcc_pca953x.h>

#include <linux/gpio.h>
#include <mach/gpio.h>
#include <asm/mach-types.h>

#if defined(CONFIG_CLOCK_TABLE)
#include <mach/clocktable.h>
static struct func_clktbl_t *bt_clktbl = NULL;
#endif


#ifndef ON
#define ON		1
#endif

#ifndef OFF
#define OFF		0
#endif

#define DEV_NAME "tcc_bt_dev"
static struct class *bt_dev_class;

typedef struct {
	int module;		// 0x12:CSR, 0x34:Broadcom
	int TMP1;
	int TMP2;
} tcc_bt_info_t;

static int tcc_bt_dev_open(struct inode *inode, struct file *file)
{
	printk("[## BT ##] tcc_bt_dev_open\n");
	return 0;
}

static int tcc_bt_dev_release(struct inode *inode, struct file *file)
{
	printk("[## BT ##] tcc_bt_dev_release\n");
	return 0;
}

static int tcc_bt_get_info(tcc_bt_info_t* arg)
{
	tcc_bt_info_t *info_t;
	int module_t;
	unsigned long ret;

	info_t = (tcc_bt_info_t *)arg;
	ret = copy_from_user(info_t, (tcc_bt_info_t *)arg, sizeof(tcc_bt_info_t));

	module_t = 0;

#if defined (CONFIG_TCC_CSR_BC0406_MODULE_SUPPORT)
	module_t = 0x12;
#elif defined (CONFIG_TCC_RDA_587X_MODULE_SUPPORT)
	module_t = 0x56;
#elif defined (CONFIG_TCC_BRCM_BCM4330_MODULE_SUPPORT)
	module_t = 0x34;
#elif defined (CONFIG_TCC_ATHEROS_AR3002_MODULE_SUPPORT)
	module_t = 0x78;
#endif

	printk("[## BT ##] module[0x%x]\n", module_t);

	info_t->module = module_t;

	ret = copy_to_user((tcc_bt_info_t *)arg, info_t, sizeof(tcc_bt_info_t));

	return 0;
}

int tcc_bt_dev_ioctl(struct file *file,
				 unsigned int cmd, void *arg)
{
	int *parm1;

	memset(&parm1, 0, sizeof(int));
	printk("[## BT ##] tcc_bt_dev_ioctl cmd[%x] arg[%x]\n", cmd, (int)arg);

	switch(cmd) {
		case IOCTL_BT_DEV_SPECIFIC:
			printk("[## BT ##] IOCTL_BT_DEV_SPECIFIC cmd[%x]\n", cmd);
			tcc_bt_get_info((tcc_bt_info_t*)arg);
			break;

		case IOCTL_BT_DEV_CLOCK_LIMIT:
#if defined(CONFIG_CLOCK_TABLE)
			parm1 = (int*)arg;
			if (bt_clktbl){
				clocktable_ctrl(bt_clktbl, 0, *parm1);	// resource, table index, enable/disable
				printk("[## BT ##] IOCTL_BT_SET_LIMIT_CLOCK cmd[%x] parm1[%x]\n", cmd, *parm1);
			}
#endif
			break;

		default :
			printk("[## BT ##] tcc_bt_dev_ioctl cmd[%x]\n", cmd);
			break;
	}

	return 0;
}

struct file_operations tcc_bt_dev_ops = {
	.owner				= THIS_MODULE,
	.unlocked_ioctl		= tcc_bt_dev_ioctl,
	.open				= tcc_bt_dev_open,
	.release			= tcc_bt_dev_release,
};

int init_module(void)
{
	int ret;

#if defined(CONFIG_MACH_TCC8920ST)
	if(system_rev != 0x9112)
		return 0;
#endif

#if defined(CONFIG_MACH_TCC8930ST)
	if(system_rev != 0x9312 && system_rev != 0x9313 && system_rev != 0x7231)
		return 0;
#endif

	printk("[## BT ##] init_module\n");

	ret = register_chrdev(BT_DEV_MAJOR_NUM, DEV_NAME, &tcc_bt_dev_ops);

	bt_dev_class = class_create(THIS_MODULE, DEV_NAME);
	device_create(bt_dev_class, NULL, MKDEV(BT_DEV_MAJOR_NUM, BT_DEV_MINOR_NUM), NULL, DEV_NAME);

#if defined(CONFIG_CLOCK_TABLE)
	bt_clktbl = clocktable_get("bt_clktbl");
	if (IS_ERR(bt_clktbl))
		bt_clktbl = NULL;
#endif

	if(ret < 0){
		printk("[## BT ##] [%d]fail to register the character device\n", ret);
		return ret;
	}

	return 0;
}

void cleanup_module(void)
{
	printk("[## BT ##] cleanup_module\n");
#if defined(CONFIG_CLOCK_TABLE)
	if (bt_clktbl) {
		if (bt_clktbl->count)
			clocktable_ctrl(bt_clktbl, 0, CLKTBL_DISABLE);
		clocktable_put(bt_clktbl);
		bt_clktbl = NULL;
	}
#endif
	unregister_chrdev(BT_DEV_MAJOR_NUM, DEV_NAME);
}

late_initcall(init_module);
module_exit(cleanup_module);

MODULE_AUTHOR("Telechips Inc. linux@telechips.com");
MODULE_DESCRIPTION("TCC_BT_DEV");
MODULE_LICENSE("GPL");
