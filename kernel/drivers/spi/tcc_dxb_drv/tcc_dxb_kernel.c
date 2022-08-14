/*
 *  Driver for TCC DXB
 *
 *  Written by C2-G1-3T
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.=
 */

#include <linux/platform_device.h>
#include <linux/interrupt.h>

#include "tcc_dmx.h"
#include "tcc_tsif_interface.h"
#include "tcc_tuner.h"
#include "tcc_fe.h"

#include "tcc_dxb_kernel.h"

/*****************************************************************************
 *
 * Defines
 *
 ******************************************************************************/
#if 0
#define dprintk(msg...)	 { printk( "[tcc_dxb]" msg); }
#else
#define dprintk(msg...)	 
#endif

// for DVBSx
#define FE_SET_USER_COMMAND 200
#define FE_GET_USER_COMMAND 201


/*****************************************************************************
 *
 * structures
 *
 ******************************************************************************/


/*****************************************************************************
 *
 * Variables
 *
 ******************************************************************************/
DVB_DEFINE_MOD_OPT_ADAPTER_NR(adapter_nr);

static struct tcc_dxb_instance gtcc_dxb;
static struct platform_device *pdev;
static int giBoardType = -1;


/*****************************************************************************
 *
 * External Functions
 *
 ******************************************************************************/


/*****************************************************************************
 *
 * Functions
 *
 ******************************************************************************/
static int tcc_dxb_frontend_register(struct dvb_frontend *fe, int board_type)
{
	int ret;

	giBoardType = board_type;
	memcpy(&fe->ops.tuner_ops, tcc_dxb_get_tuner_pos(), sizeof(struct dvb_tuner_ops));

	ret = dvb_register_frontend(&gtcc_dxb.adapter, fe);
    if (ret < 0)
    {
        printk("tcc_dxb_kernel : Frontend registration failed!\n");
        dvb_frontend_detach(fe);
    }
	return 0;
}
EXPORT_SYMBOL(tcc_dxb_frontend_register);

static int tcc_dxb_frontend_unregister(struct dvb_frontend *fe)
{
    dvb_unregister_frontend(fe);
    dvb_frontend_detach(fe);
	return 0;
}
EXPORT_SYMBOL(tcc_dxb_frontend_unregister);

static int tcc_dxb_frontend_ioctl(struct dvb_frontend *fe, unsigned int cmd, void *parg, unsigned int stage)
{
	struct dtv_properties *tvps = NULL;
	struct dtv_property *tvp = NULL;
	int i, err = 0;

	if (stage != DVB_FE_IOCTL_PRE)
		return 0;

	if(cmd == FE_SET_USER_COMMAND && fe->ops.set_property) {
		tvps = (struct dtv_properties __user *)parg;
		if ((tvps->num == 0) || (tvps->num > DTV_IOCTL_MAX_MSGS))
			return -EINVAL;
		tvp = kmalloc(tvps->num * sizeof(struct dtv_property), GFP_KERNEL);
		if (!tvp) {
			err = -ENOMEM;
			goto out;
		}
		if (copy_from_user(tvp, tvps->props, tvps->num * sizeof(struct dtv_property))) {
			err = -EFAULT;
			goto out;
		}
		for (i = 0; i < tvps->num; i++) {
			err = fe->ops.set_property(fe, tvp + i);
			if (err < 0)
				goto out;
			(tvp + i)->result = err;
		}
	} else
	if (cmd == FE_GET_USER_COMMAND && fe->ops.get_property) {
		tvps = (struct dtv_properties __user *)parg;
		if ((tvps->num == 0) || (tvps->num > DTV_IOCTL_MAX_MSGS))
			return -EINVAL;
		tvp = kmalloc(tvps->num * sizeof(struct dtv_property), GFP_KERNEL);
		if (!tvp) {
			err = -ENOMEM;
			goto out;
		}
		if (copy_from_user(tvp, tvps->props, tvps->num * sizeof(struct dtv_property))) {
			err = -EFAULT;
			goto out;
		}
		for (i = 0; i < tvps->num; i++) {
			err = fe->ops.get_property(fe, tvp + i);
			if (err < 0)
				goto out;
			(tvp + i)->result = err;
		}
		if (copy_to_user(tvps->props, tvp, tvps->num * sizeof(struct dtv_property))) {
			err = -EFAULT;
			goto out;
		}
	}
out:
	if (tvp)
		kfree(tvp);
	return err;
}

int tcc_dxb_get_board_type(void)
{
	return giBoardType;
}

struct tcc_dxb_instance* tcc_dxb_get_instance(void)
{
	return &gtcc_dxb;
}

static ssize_t tcc_dxb_kernel_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	//sscanf(buf, "%d", &giBoardType);
	return count;
}

static ssize_t tcc_dxb_kernel_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", giBoardType);
}
static DEVICE_ATTR(boardtype, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, tcc_dxb_kernel_show, tcc_dxb_kernel_store);

static int tcc_dxb_register(void)
{
	int ret, i;

	ret = dvb_register_adapter(&gtcc_dxb.adapter, DVB_NAME, THIS_MODULE, &pdev->dev, adapter_nr);

	gtcc_dxb.adapter.fe_ioctl_override = &tcc_dxb_frontend_ioctl;

	tccfrontend_init();

	for (i=0; i<TSIF_DEV_COUNT; i++)
	{
		tcc_tsif_init(&gtcc_dxb.tsif[i]);
	}

	for (i=0; i<DMX_DEV_COUNT; i++)
	{
		tcc_dmx_init(&gtcc_dxb.dmx[i]);
	}

	return 0;
}

static int tcc_dxb_unregister(void)
{
	int i;

	for (i=0; i<DMX_DEV_COUNT; i++)
	{
		tcc_dmx_deinit(&gtcc_dxb.dmx[i]);
	}

	for (i=0; i<TSIF_DEV_COUNT; i++)
	{
		tcc_tsif_deinit(&gtcc_dxb.tsif[i]);
	}

	tccfrontend_exit();

    dvb_unregister_adapter(&gtcc_dxb.adapter);

    return 0;
}

static int __init tcc_dxb_kernel_init(void)
{
	pdev = platform_device_register_simple(DRV_NAME, 0, NULL, 0);
	if (IS_ERR(pdev)) {
		printk(KERN_ERR "Failed to register platform device.\n");
		return -EINVAL;
	}

	if (device_create_file(&pdev->dev, &dev_attr_boardtype)) {
		printk(KERN_ERR "Failed to create file.\n");
	}

	tcc_dxb_register();

	dprintk(KERN_INFO "%s::[%s]\n", __FUNCTION__,__DATE__);

	return 0;
}

static void __exit tcc_dxb_kernel_exit(void)
{    
	dprintk(KERN_INFO "%s\n", __FUNCTION__);

	tcc_dxb_unregister();

	device_remove_file(&pdev->dev, &dev_attr_boardtype);

	platform_device_unregister(pdev);
}

module_init(tcc_dxb_kernel_init);
module_exit(tcc_dxb_kernel_exit);

MODULE_AUTHOR("Telechips Inc.");
MODULE_DESCRIPTION("TCCxxx dxb kernel module");
MODULE_LICENSE("GPL");

