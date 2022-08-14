/*
 * drd_otg.c: common usb otg control API
 *
 *  Copyright (C) 2013, Telechips, Inc.
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>

#include <asm/io.h>
#include <asm/mach-types.h>

#include <mach/bsp.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/kdev_t.h>

#include <mach/reg_physical.h>
#include <mach/structures_hsio.h>
#include <mach/gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>

#define OTG_HOST       1
#define OTG_DEVICE     0

#define DRIVER_AUTHOR "Taejin"
#define DRIVER_DESC "'eXtensible' DRD otg Driver"
//#define DBG_FORCE_DRD_DEVICE_MODE

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_AUTHOR (DRIVER_AUTHOR);
MODULE_LICENSE ("GPL");

struct platform_device *pdev;
int cnt;
int cmode;

struct usb3_data {
   struct work_struct     wq;
   unsigned long           flag;
};
struct usb3_data *drd_otg_mod_wq;


struct drd_dev {
	struct device *dev;

	bool enabled;
	struct mutex mutex;
	bool connected;
	bool sw_connected;
	struct work_struct work;
	struct delayed_work init_work;
};

static struct class *drd_class;
static struct drd_dev *_drd_dev;

static char mode_string[256];
static char def_mode_string[256];

static ssize_t enable_show(struct device *pdev, struct device_attribute *attr, char *buf)
{
	struct drd_dev *dev = dev_get_drvdata(pdev);
	return sprintf(buf, "%d\n", dev->enabled);
}

static ssize_t enable_store(struct device *pdev, struct device_attribute *attr, const char *buff, size_t size)
{
	//struct drd_dev *dev = dev_get_drvdata(pdev);
   return size;
}

static ssize_t state_show(struct device *pdev, struct device_attribute *attr, char *buf)
{
	//struct drd_dev *dev = dev_get_drvdata(pdev);
	return 0;
}

#define DESCRIPTOR_ATTR(field, format_string)				\
static ssize_t								\
field ## _show(struct device *dev, struct device_attribute *attr,	\
		char *buf)						\
{									\
	return sprintf(buf, format_string, device_desc.field);		\
}									\
static ssize_t								\
field ## _store(struct device *dev, struct device_attribute *attr,	\
		const char *buf, size_t size)				\
{									\
	int value;							\
	if (sscanf(buf, format_string, &value) == 1) {			\
		device_desc.field = value;				\
		return size;						\
	}								\
	return -1;							\
}									\
static DEVICE_ATTR(field, S_IRUGO | S_IWUSR, field ## _show, field ## _store);

#define DESCRIPTOR_STRING_ATTR(field, buffer)				\
static ssize_t								\
field ## _show(struct device *dev, struct device_attribute *attr,	\
		char *buf)						\
{									\
	return sprintf(buf, "%s", buffer);				\
}									\
static ssize_t								\
field ## _store(struct device *dev, struct device_attribute *attr,	\
		const char *buf, size_t size)				\
{									\
	if (size >= sizeof(buffer))					\
		return -EINVAL;						\
	return strlcpy(buffer, buf, sizeof(buffer));			\
}									\
static DEVICE_ATTR(field, S_IRUGO | S_IWUSR, field ## _show, field ## _store);


DESCRIPTOR_STRING_ATTR(drdmode, mode_string)
DESCRIPTOR_STRING_ATTR(defmode, def_mode_string)

static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR, enable_show, enable_store);
static DEVICE_ATTR(state, S_IRUGO, state_show, NULL);

static struct device_attribute *drd_usb_attributes[] = {
	&dev_attr_drdmode,
	&dev_attr_defmode,
	NULL
};

extern int tcc_usb30_vbus_init(void);
extern int tcc_usb30_vbus_exit(void);
extern int tcc_dwc3_vbus_ctrl(int on);
extern void tcc_usb30_phy_off(void);
extern void tcc_usb30_phy_on(void);
extern int tcc_usb30_clkset(int on);

static int set_otg_irq_flag = 0;

static void tcc_stop_dwc3(void)
{
   tcc_usb30_clkset(0);
}

static void tcc_start_dwc3(void)
{
   tcc_usb30_clkset(1);
}

int tcc_drd_vbus_ctrl(int on)
{
   if (machine_is_tcc893x()) {
      int gpio_otg0_vbus_en, gpio_otg_en;

      gpio_otg0_vbus_en = TCC_GPEXT2(13); //ÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿ

      gpio_request(gpio_otg0_vbus_en, "otg0_vbus_en");

      /* Don't control gpio_hs_host_en because this power also supported in USB core. */
      gpio_direction_output(gpio_otg0_vbus_en, (on)?1:0);

      return 0;
   } else if(machine_is_tcc8930st()) {
      return 0;
   }

   return -1;
}

static void dwc_usb3_phy_init(void)
{
   PUSBPHYCFG USBPHYCFG = (PUSBPHYCFG)tcc_p2v(HwUSBPHYCFG_BASE);
   unsigned int uTmp;

   tcc_usb30_vbus_init();
   tcc_dwc3_vbus_ctrl(1);
   tcc_drd_vbus_ctrl(1);
   tcc_start_dwc3();
   tcc_usb30_phy_on();

   // Initialize All PHY & LINK CFG Registers
   USBPHYCFG->UPCR0 = 0xB5484068;
   USBPHYCFG->UPCR1 = 0x0000041C;
   USBPHYCFG->UPCR2 = 0x919E14C8;
   USBPHYCFG->UPCR3 = 0x4AB05D00;
   USBPHYCFG->UPCR4 = 0x00000000;
   USBPHYCFG->UPCR5 = 0x00108001;
   USBPHYCFG->LCFG  = 0x80420013;

   BITSET(USBPHYCFG->UPCR4, Hw20|Hw21); //ID pin interrupt enable

   USBPHYCFG->UPCR1 = 0x00000412;
   USBPHYCFG->LCFG  = 0x86420013;

#if 0
   USBPHYCFG->LCFG  |= Hw30;       //USB2.0 HS only mode
   USBPHYCFG->UPCR1 |= Hw9;   //test_powerdown_ssp
#else
   BITCLR(USBPHYCFG->LCFG, Hw30);
   BITCLR(USBPHYCFG->UPCR1,Hw9);
   //USBPHYCFG->UPCR1 |= Hw8;         //test_powerdown_hsp
#endif

   // Wait USB30 PHY initialization finish. PHY FREECLK should be stable.
   while(1) {
      // GDBGLTSSM : Check LTDB Sub State is non-zero
      // When PHY FREECLK is valid, LINK mac_resetn is released and LTDB Sub State change to non-zero
      uTmp = readl(tcc_p2v(HwUSBDEVICE_BASE+0x164));
      uTmp = (uTmp>>18)&0xF;

      // Break when LTDB Sub state is non-zero and CNR is zero
      if (uTmp != 0){
         break;
      }
   }
}

spinlock_t lock;
static irqreturn_t dwc_usb3_otg_host_dwc_irq(int irq, struct platform_device *pdev )
{
   printk("\x1b[1;36m Device mode -> Host mode(line : %d)\x1b[0m\n",__LINE__);
   spin_lock(&lock);

   drd_otg_mod_wq->flag = 0;
   schedule_work(&drd_otg_mod_wq->wq);

   spin_unlock(&lock);

   return IRQ_HANDLED;
}

static irqreturn_t dwc_usb3_otg_device_dwc_irq(int irq, struct platform_device *pdev )
{
   printk("\x1b[1;36m Host mode -> Device mode(line: %d)  \x1b[0m\n",__LINE__);

   spin_lock(&lock);

   drd_otg_mod_wq->flag = 1;
   schedule_work(&drd_otg_mod_wq->wq);

   spin_unlock(&lock);

   return IRQ_HANDLED;
}

extern void msleep(unsigned int msecs);
static void tcc_usb30_module_change(struct work_struct *work)
{
   int retval = 0;
   struct usb3_data *p =  container_of(work, struct usb3_data, wq);
   unsigned long flag = p->flag;

   if( set_otg_irq_flag )
      free_irq(INT_OTGID, pdev);
   set_otg_irq_flag = 0;

   if (flag) {
      //host -> device
      tcc_drd_vbus_ctrl(0);
      printk("DRD USB DEVICE mode\n");
	   strncpy(mode_string, "usb_device", sizeof(mode_string) - 1);
   } else {
      //device ->host
      tcc_drd_vbus_ctrl(1);
      printk("DRD USB HOST mode\n");
	   strncpy(mode_string, "usb_host", sizeof(mode_string) - 1);
   }

   if(!flag){
      printk("\x1b[1;38mChange Falling -> Rising (Catch Host to Device)\x1b[0m\n");
      retval = request_irq(INT_OTGID, &dwc_usb3_otg_device_dwc_irq, IRQF_SHARED|IRQ_TYPE_EDGE_RISING, "USB30_IRQ11", pdev);
      if (retval) {
         dev_err(&pdev->dev, "request rising edge of irq%d failed!\n", INT_OTGID);
         retval = -EBUSY;
      } else {
         cmode = OTG_HOST;
         set_otg_irq_flag = 1;
      }
   } else {
      printk("\x1b[1;38mChange Rising -> Falling (Catch Device to Host) \x1b[0m\n");
      retval = request_irq(INT_OTGID, &dwc_usb3_otg_host_dwc_irq, IRQF_SHARED|IRQ_TYPE_EDGE_FALLING, "USB30_IRQ5", pdev);
      if (retval) {
         dev_err(&pdev->dev, "request falling edge of irq%d failed!\n", INT_OTGID);
         retval = -EBUSY;
      }else {
         cmode = OTG_DEVICE;
         set_otg_irq_flag = 1;
      }
   }

   return;

End:
   printk("usb3.0 otg mode change fail.\n");
}

extern void msleep(unsigned int msecs);

static void drd_tcc_set_default_mode(void)
{
#ifdef DBG_FORCE_DRD_DEVICE_MODE
	strncpy(def_mode_string, "usb_device", sizeof(def_mode_string) - 1);
#else
	#if defined(CONFIG_STB_BOARD_EV_TCC893X) || defined(CONFIG_STB_BOARD_YJ8935T) || defined(CONFIG_STB_BOARD_YJ8930T)
		strncpy(def_mode_string, "usb_host", sizeof(def_mode_string) - 1);
	#else
		strncpy(def_mode_string, "usb_device", sizeof(def_mode_string) - 1);
	#endif
#endif
}

static int __devinit drd_tcc_drv_probe(struct platform_device *pdev2)
{
   int retval = 0;
   spin_lock_init(&lock);
   pdev = pdev2;
   cnt = 0;

   printk("\x1b[1;33mfunc: %s, line: %d  \x1b[0m\n",__func__,__LINE__);
   /* Set gps_pwrctrl data
   */
   drd_otg_mod_wq = (struct usb3_data *)kzalloc(sizeof(struct usb3_data), GFP_KERNEL);
   if (drd_otg_mod_wq == NULL) {
      retval = -ENOMEM;
   }

   //drd_otg_mod_wq->flag = 0;
   INIT_WORK(&drd_otg_mod_wq->wq, tcc_usb30_module_change);

   printk("\x1b[1;39mfunc: %s, line: %d Rising \x1b[0m\n",__func__,__LINE__);
   retval = request_irq(INT_OTGID, &dwc_usb3_otg_device_dwc_irq,
         IRQF_SHARED|IRQ_TYPE_EDGE_RISING,
         "USB30_IRQ", pdev);

   if (retval) {
      dev_err(&pdev->dev, "request rising edge of irq%d failed!\n", INT_OTGID);
      retval = -EBUSY;
      goto fail_drd_change;
   } else {
      cmode = OTG_HOST;
      set_otg_irq_flag = 1;
   }

	strncpy(mode_string, "usb_host", sizeof(mode_string) - 1);
	drd_tcc_set_default_mode();
	
   msleep(100);
   dwc_usb3_phy_init();

   return retval;

fail_drd_change:

   printk("fail_drd_change\n");

   return retval;
}

static void drd_tcc_remove(struct platform_device *pdev)
{
   tcc_usb30_vbus_exit();
}

static int drd_otg_suspend(struct platform_device *dev, pm_message_t state)
{
   PUSBPHYCFG USBPHYCFG = (PUSBPHYCFG)tcc_p2v(HwUSBPHYCFG_BASE);
   printk("%d\n", __func__);
   // USB 3.0 PHY Power down
   USBPHYCFG->UPCR1 |= Hw3;
   USBPHYCFG->UPCR1 |= (Hw8 | Hw9);

   if( set_otg_irq_flag )
      free_irq(INT_OTGID, pdev);

   set_otg_irq_flag = 0;
   BITCLR(USBPHYCFG->UPCR4, Hw20|Hw21);

   tcc_usb30_vbus_exit();
   return 0;
}

static int drd_otg_resume(struct platform_device *dev)
{
   printk("%d\n", __func__);

   spin_lock(&lock);

   tcc_usb30_vbus_init();
   drd_otg_mod_wq->flag = 0;
   schedule_work(&drd_otg_mod_wq->wq);
   spin_unlock(&lock);
	//msleep(100);

   return 0;
}

struct platform_device drd_otg_device = {
   .name                       = "tcc-drd",
   .id                         = -1,
   //.num_resources  = ARRAY_SIZE(tcc8930_xhci_hs_resources),
   //.resource       = tcc8930_xhci_hs_resources,
   .dev                        = {
      //.dma_mask                      = &tcc8930_device_xhci_dmamask,
      //.coherent_dma_mask     = 0xffffffff,
      //.platform_data = tcc8930_ehci_hs_data,
   },
};

static struct platform_driver drd_otg_driver = {
   .probe              = drd_tcc_drv_probe,
   .remove             = __exit_p(drd_tcc_remove),
   .suspend    = drd_otg_suspend,
   .resume             = drd_otg_resume,
   //.shutdown = NULL,
   .driver = {
      .name    = (char *) "tcc-drd",
      .owner   = THIS_MODULE,
      //.pm            = EHCI_TCC_PMOPS,
   }
};

static int drd_create_device(struct drd_dev *dev)
{
	struct device_attribute **attrs = drd_usb_attributes;
	struct device_attribute *attr;
	int err;

	dev->dev = device_create(drd_class, NULL,
					MKDEV(0, 0), NULL, "drd0");
	if (IS_ERR(dev->dev))
		return PTR_ERR(dev->dev);

	dev_set_drvdata(dev->dev, dev);

	while ((attr = *attrs++)) {
		err = device_create_file(dev->dev, attr);
		if (err) {
			device_destroy(drd_class, dev->dev->devt);
			return err;
		}
	}
	return 0;
}

static int __init drd_otg_init(void)
{
   int retval = 0;
	struct drd_dev *dev;
	int err;

	drd_class = class_create(THIS_MODULE, "drd_otg");
	if (IS_ERR(drd_class))
		return PTR_ERR(drd_class);

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	err = drd_create_device(dev);
	if (err) {
		class_destroy(drd_class);
		kfree(dev);
		return err;
	}

	_drd_dev = dev;

   retval = platform_device_register(&drd_otg_device);
   if (retval < 0){
      printk("drd device register fail!\n");
      return retval;
   }

   retval = platform_driver_register(&drd_otg_driver);
   if (retval < 0){
      printk("drd drvier register fail!\n");
      return retval;
   }
   return retval;
}
module_init(drd_otg_init);

