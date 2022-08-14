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
#include <linux/delay.h>
#include <mach/io.h>
#include <linux/gpio.h>
#include <mach/bsp.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/err.h>

#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>

#include <asm/mach-types.h>

#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/platform_device.h>

#include "tcc_cm3_control.h"
#include <mach/TCC893xHWADec.h>
#include "adec.h"
#include "tcc_adec_proc.h"
#include <mach/TCC893xCM3AVN.h>
#include "tcc_avn_proc.h"

#define CM3_USE_FOR_AVN

static struct device *pdev_cm3 = NULL;
static struct clk *cm3_clk = NULL;
DECLARE_WAIT_QUEUE_HEAD( wq_cm3_cmd ) ;
static int cm3_cmd_status = 0;

static int retData[8];

static int (*cm3_proc)(unsigned long);

#undef cm3_dbg
#if 0
#define cm3_dbg(f, a...)  printk("== cm3_dbg == " f, ##a)
#else
#define cm3_dbg(f, a...)
#endif

int CM3_SEND_COMMAND(ARG_CM3_CMD *pInARG, ARG_CM3_CMD *pOutARG)
{
	int ret;
    PMAILBOX pMailBox = (volatile PMAILBOX)tcc_p2v(HwCORTEXM3_MAILBOX0_BASE);

	cm3_cmd_status = 1;
//    BITCLR( pMailBox->uMBOX_CTL_016.nREG, Hw5); //OEN low
    pMailBox->uMBOX_TX0.nREG = pInARG->uiCmdId;
    pMailBox->uMBOX_TX1.nREG = pInARG->uiParam0;
    pMailBox->uMBOX_TX2.nREG = pInARG->uiParam1;
    pMailBox->uMBOX_TX3.nREG = pInARG->uiParam2;
    pMailBox->uMBOX_TX4.nREG = pInARG->uiParam3;
    pMailBox->uMBOX_TX5.nREG = pInARG->uiParam4;
    pMailBox->uMBOX_TX6.nREG = pInARG->uiParam5;
    pMailBox->uMBOX_TX7.nREG = pInARG->uiParam6;
//    BITSET( pMailBox->uMBOX_CTL_016.nREG, Hw5); //OEN high

	cm3_dbg("CM3_SEND_COMMAND 0x%x, 0x%x, 0x%x, 0x%x\n", pInARG->uiCmdId, pInARG->uiParam0, pInARG->uiParam1, pInARG->uiParam2);

#if 0
	ret = wait_event_interruptible_timeout( wq_cm3_cmd, 
									cm3_cmd_status == 0, 
									msecs_to_jiffies(100)) ;
#else
	ret = wait_event_interruptible( wq_cm3_cmd, 
									cm3_cmd_status == 0) ;
#endif

	memcpy(pOutARG, retData, sizeof(ARG_CM3_CMD));

//	printk("wake up wq_cm3_cmd with ret(%d)\n", ret);
    return 0;
}


/*****************************************************************************
* Function Name : static void CM3_UnloadBinary(void);
* Description : CM3 Code Un-Loading
* Arguments :
******************************************************************************/
static void CM3_UnloadBinary(void)
{
    PCM3_TSD_CFG pTSDCfg = (volatile PCM3_TSD_CFG)tcc_p2v(HwCORTEXM3_TSD_CFG_BASE);
    BITSET(pTSDCfg->CM3_RESET.nREG, Hw1|Hw2); //m3 no reset
    cm3_dbg("%s:%d\n",__func__, __LINE__);
}


/*****************************************************************************
* Function Name : static void CM3_LoadBinary(void);
* Description : CM3 Code Loading
* Arguments :
******************************************************************************/
static void CM3_LoadBinary(void* binary, unsigned size)
{
//    volatile unsigned int * pCodeMem = (volatile unsigned int *)tcc_p2v(HwCORTEXM3_CODE_MEM_BASE);
    volatile unsigned int * pCodeMem = (volatile unsigned int *)0xF0000000;
    PCM3_TSD_CFG pTSDCfg = (volatile PCM3_TSD_CFG)tcc_p2v(HwCORTEXM3_TSD_CFG_BASE);
    CM3_UnloadBinary();
    cm3_dbg("%s:%d\n",__func__, __LINE__);
    pTSDCfg->REMAP0.bREG.ICODE = 0x1;
    pTSDCfg->REMAP0.bREG.DCODE = 0x1;    
    pTSDCfg->REMAP0.bREG.REMAP_2 = 0x2;    
    pTSDCfg->REMAP0.bREG.REMAP_3 = 0x3;    
    pTSDCfg->REMAP0.bREG.REMAP_4 = 0x4;    
    pTSDCfg->REMAP0.bREG.REMAP_5 = 0x5;    
    pTSDCfg->REMAP0.bREG.REMAP_6 = 0x6;    
    pTSDCfg->REMAP0.bREG.REMAP_7 = 0x7;    
    memcpy((void *)pCodeMem, binary, size);

	BITCLR(pTSDCfg->CM3_RESET.nREG, Hw1|Hw2); //m3 reset
}

/*****************************************************************************
* Function Name : static void MailBox_Configure(void);
* Description : MailBox register init
* Arguments :
******************************************************************************/
static void CM3_MailBox_Configure(void)
{
    volatile PMAILBOX pMailBoxMain = (volatile PMAILBOX)tcc_p2v(HwCORTEXM3_MAILBOX0_BASE);
    volatile PMAILBOX pMailBoxSub = (volatile PMAILBOX)tcc_p2v(HwCORTEXM3_MAILBOX1_BASE);
    volatile PCM3_TSD_CFG pTSDCfg = (volatile PCM3_TSD_CFG)tcc_p2v(HwCORTEXM3_TSD_CFG_BASE);
    BITSET(pMailBoxMain->uMBOX_CTL_016.nREG, Hw0|Hw1|Hw4|Hw5|Hw6);
    BITSET(pMailBoxSub->uMBOX_CTL_016.nREG, Hw0|Hw1|Hw4|Hw5|Hw6);
    BITSET(pTSDCfg->IRQ_MASK_POL.nREG, Hw16|Hw22); //IRQ_MASK_POL, IRQ, FIQ(CHANGE POLARITY)
}

static irqreturn_t CM3_MailBox_Handler(int irq, void *dev)
{	
    int i;
    char cmd, dmxid;
    PMAILBOX pMailBox = (volatile PMAILBOX)tcc_p2v(HwCORTEXM3_MAILBOX0_BASE);
	memcpy(retData, &(pMailBox->uMBOX_RX0), sizeof(int)*8);

#if 0
#if 0
	printk("MB : [0x%X][0x%X][0x%X][0x%X][0x%X][0x%X][0x%X][0x%X]\n", nReg[0], nReg[1], nReg[2], nReg[3], nReg[4], nReg[5], nReg[6], nReg[7]);
#else
	printk("MB : ");
	for(i=0; i<8; i++)
	{
		if(i == 1)
			printk("[0x%X]", retData[i]);
		else
			printk("[%d]", retData[i]);			
	}
	printk("\n");
#endif	
#endif
	wake_up_interruptible( &wq_cm3_cmd ) ;
	cm3_cmd_status = 0;
    return IRQ_HANDLED;
}

static long tcc_cm3_ctrl_ioctl(struct file *filp, 
							unsigned int cmd, unsigned long arg)
{
	int ret = 0;

//    printk("%s cmd: %d, arg: %x\n", __FUNCTION__, cmd, arg);

	switch (cmd) {
	case IOCTL_CM3_CTRL_OFF:
    {
        if(cm3_clk) {
            CM3_UnloadBinary();
            free_irq(MBOX_IRQ, NULL);
            clk_disable(cm3_clk);
            clk_put(cm3_clk);
            cm3_clk = NULL;
        }
	}
		break;
	case IOCTL_CM3_CTRL_ON:
    {
        cm3_clk = clk_get(NULL, "cortex-m3");
        if(IS_ERR(cm3_clk)) {
            printk("can't find cortex-m3 clk driver!");
            cm3_clk = NULL;
        } else {
            clk_set_rate(cm3_clk, 100*1000*1000);
            clk_enable(cm3_clk);	

			CM3_MailBox_Configure();

#if defined(CM3_USE_FOR_AVN)
    	    CM3_LoadBinary((void*)TCC893xCM3AVN, sizeof(TCC893xCM3AVN));
			cm3_proc = CM3_AVN_Proc;

#else
    	    CM3_LoadBinary((void*)TCC893xHWADec, sizeof(TCC893xHWADec));
			cm3_proc = ADEC_Proc;
#endif

            ret = request_irq(MBOX_IRQ, CM3_MailBox_Handler, IRQ_TYPE_LEVEL_LOW | IRQF_DISABLED, DEVICE_NAME, NULL);
            msleep(100); //Wait for CM3 Booting
            if(ret < 0)
	            printk("error : request_irq %d\n", ret);
        }

	}   
		break;
	case IOCTL_CM3_CTRL_CMD:
    {
		ret = (*cm3_proc)(arg);
	}   
		break;
	case IOCTL_CM3_CTRL_RESET:
	default:
		printk("cm3 error: unrecognized ioctl (0x%x)\n", cmd); 
		return -EINVAL;
		break;
	}

	return ret;
}

static int tcc_cm3_ctrl_release(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "%s::%d\n", __FUNCTION__, __LINE__);
	return 0;
}

static int tcc_cm3_ctrl_open(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "%s::%d\n", __FUNCTION__, __LINE__);
	return 0;
}

struct file_operations tcc_cm3_ctrl_fops =
{
	.owner          = THIS_MODULE,
	.open           = tcc_cm3_ctrl_open,
	.release        = tcc_cm3_ctrl_release,
	.unlocked_ioctl = tcc_cm3_ctrl_ioctl,
};

static struct class *tcc_cm3_ctrl_class;

static int tcc_cm3_probe(struct platform_device *pdev)
{
	int res;

	res = register_chrdev(CM3_CTRL_DEV_MAJOR, CM3_CTRL_DEV_NAME, &tcc_cm3_ctrl_fops);
	if (res < 0)
		return res;
	tcc_cm3_ctrl_class = class_create(THIS_MODULE, CM3_CTRL_DEV_NAME);
	if(NULL == device_create(tcc_cm3_ctrl_class, NULL, MKDEV(CM3_CTRL_DEV_MAJOR, CM3_CTRL_DEV_MINOR), NULL, CM3_CTRL_DEV_NAME))
		printk(KERN_INFO "%s device_create failed\n", __FUNCTION__);	

	pdev_cm3 = &pdev->dev;

	printk("%s\n", __func__);

	return 0;
}

static int tcc_cm3_remove(struct platform_device *pdev)
{
	device_destroy(tcc_cm3_ctrl_class, MKDEV(CM3_CTRL_DEV_MAJOR, CM3_CTRL_DEV_MINOR));
	class_destroy(tcc_cm3_ctrl_class);
	unregister_chrdev(CM3_CTRL_DEV_MAJOR, CM3_CTRL_DEV_NAME);

	return 0;
}

static struct platform_driver tcc_cm3_ctrl = {
	.probe	= tcc_cm3_probe,
	.remove	= tcc_cm3_remove,
	.driver	= {
		.name	= CM3_CTRL_DEV_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init tcc_cm3_ctrl_init(void)
{
	return platform_driver_register(&tcc_cm3_ctrl);
}

static void __exit tcc_cm3_ctrl_exit(void)
{
	platform_driver_register(&tcc_cm3_ctrl);
}

module_init(tcc_cm3_ctrl_init);
module_exit(tcc_cm3_ctrl_exit);

MODULE_AUTHOR("Telechips Inc.");
MODULE_DESCRIPTION("TCC Cortex M3 Control(Power, Reset..)");
MODULE_LICENSE("GPL");
