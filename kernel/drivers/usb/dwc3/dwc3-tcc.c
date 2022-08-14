/*
 * Copyright (C) 2013 Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/platform_data/dwc3-tcc.h>
#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <mach/tcc_board_power.h>

#include <mach/bsp.h>
#include <mach/gpio.h>
#include "core.h"

struct dwc3_tcc {
	struct platform_device	*dwc3;
	struct device		*dev;

	struct clk		*clk;
};

static int __devinit dwc3_tcc_probe(struct platform_device *pdev)
{
	struct dwc3_tcc_data	*pdata = pdev->dev.platform_data;
	struct platform_device	*dwc3;
	struct dwc3_tcc	*tcc;
	struct clk		*clk;

	int			devid;
	int			ret = -ENOMEM;

   printk("%s\n", __func__);

   struct resource tmp_resources[] = {
      [0] = {
         .start	= HwUSBLINK_BASE,
         .end	   = HwUSBLINK_BASE + 0xCFFF,
         .flags	= IORESOURCE_MEM,
      },
      [1] = {
         .start = INT_USB30,
         .end   = INT_USB30,
         .flags = IORESOURCE_IRQ,
      },
   };

	tcc = kzalloc(sizeof(*tcc), GFP_KERNEL);
	if (!tcc) {
		dev_err(&pdev->dev, "not enough memory\n");
		goto err0;
	}

	platform_set_drvdata(pdev, tcc);

	devid = dwc3_get_device_id();
	if (devid < 0)
		goto err1;

	dwc3 = platform_device_alloc("dwc3", devid);
	if (!dwc3) {
		dev_err(&pdev->dev, "couldn't allocate dwc3 device\n");
		goto err2;
	}

	dma_set_coherent_mask(&dwc3->dev, pdev->dev.coherent_dma_mask);

	dwc3->dev.parent = &pdev->dev;
	dwc3->dev.dma_mask = pdev->dev.dma_mask;
	dwc3->dev.dma_parms = pdev->dev.dma_parms;
	tcc->dwc3	= dwc3;
	tcc->dev	= &pdev->dev;
	tcc->clk	= clk;

	/* PHY initialization */
	if (!pdata) {
		dev_dbg(&pdev->dev, "missing platform data\n");
	} else {
		if (pdata->phy_init)
			pdata->phy_init(pdev, pdata->phy_type);
	}

   pdev->resource = tmp_resources;
	ret = platform_device_add_resources(dwc3, pdev->resource,
			pdev->num_resources);
	if (ret) {
		dev_err(&pdev->dev, "couldn't add resources to dwc3 device\n");
		goto err4;
	}

	ret = platform_device_add(dwc3);
	if (ret) {
		dev_err(&pdev->dev, "failed to register dwc3 device\n");
		goto err4;
	}

	return 0;

err4:
	if (pdata && pdata->phy_exit)
		pdata->phy_exit(pdev, pdata->phy_type);

err3:
	platform_device_put(dwc3);
err2:
	dwc3_put_device_id(devid);
err1:
	kfree(tcc);
err0:
	return ret;
}

static int __devexit dwc3_tcc_remove(struct platform_device *pdev)
{
	struct dwc3_tcc	*tcc = platform_get_drvdata(pdev);
	struct dwc3_tcc_data *pdata = pdev->dev.platform_data;

	platform_device_unregister(tcc->dwc3);

	dwc3_put_device_id(tcc->dwc3->id);

	if (pdata && pdata->phy_exit)
		pdata->phy_exit(pdev, pdata->phy_type);

	platform_set_drvdata(pdev, NULL);

	kfree(tcc);

	return 0;
}


#ifdef CONFIG_PM
static int dwc3_tcc_suspend(struct platform_device *pdev, pm_message_t state)
{
   printk("%s\n", __func__);
	struct dwc3_tcc	*tcc = platform_get_drvdata(pdev);
  	struct dwc3_tcc_data *pdata = pdev->dev.platform_data;

	if (pdata && pdata->phy_exit)
		pdata->phy_exit(pdev, pdata->phy_type);

	return 0;
}

static int dwc3_tcc_resume(struct platform_device *pdev)
{
   printk("%s\n", __func__);
	struct dwc3_tcc	*tcc = platform_get_drvdata(pdev);
	struct dwc3_tcc_data *pdata = pdev->dev.platform_data;

	if (pdata->phy_init)
		pdata->phy_init(pdev, pdata->phy_type);	
   return 0;
}
#endif

static struct platform_driver dwc3_tcc_driver = {
	.probe		= dwc3_tcc_probe,
#ifdef CONFIG_PM
   .suspend    = dwc3_tcc_suspend,
   .resume     = dwc3_tcc_resume,
#endif
	.remove		= __devexit_p(dwc3_tcc_remove),
	.driver		= {
		.name	= "tcc-dwc3",
	},
};

module_platform_driver(dwc3_tcc_driver);

MODULE_ALIAS("platform:tcc-dwc3");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DesignWare USB3 TCC Glue Layer");
