/* drivers/input/touchscreen/ft5x06_ts.c
 *
 * FocalTech ft5x0x TouchScreen driver.
 *
 * Copyright (c) 2010  Focal tech Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/i2c.h>
#include <linux/input.h>
#include "ft5606_ts.h"
#include "ft5606_ex_fun.h"
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <mach/gpio.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/input/mt.h>

struct ts_event {
	u16 au16_x[CFG_MAX_TOUCH_POINTS];	/*x coordinate */
	u16 au16_y[CFG_MAX_TOUCH_POINTS];	/*y coordinate */
	u8 au8_touch_event[CFG_MAX_TOUCH_POINTS];	/*touch event:
					0 -- down; 1-- contact; 2 -- contact */
	u8 au8_finger_id[CFG_MAX_TOUCH_POINTS];	/*touch ID */
	u16 pressure;
	u8 touch_point;
};

struct ft5x0x_ts_data {
	unsigned int irq;
	unsigned int x_max;
	unsigned int y_max;
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct ts_event event;
	//struct ft5x0x_platform_data *pdata;
	//struct early_suspend early_suspend;

    struct work_struct     pen_event_work;
    struct workqueue_struct *ts_workqueue;
};


struct ft5x0x_ts_data *g_ft5x0x_ts;

static int ft5x0x_ts_suspend(struct i2c_client *client, pm_message_t mesg);
static int ft5x0x_ts_resume(struct i2c_client *client);

/*
*ft5x0x_i2c_Read-read data and write data by i2c
*@client: handle of i2c
*@writebuf: Data that will be written to the slave
*@writelen: How many bytes to write
*@readbuf: Where to store data read from slave
*@readlen: How many bytes to read
*
*Returns negative errno, else the number of messages executed
*
*
*/
int ft5x0x_i2c_Read(struct i2c_client *client, char *writebuf,
		    int writelen, char *readbuf, int readlen)
{
	int ret;

	if (writelen > 0) {
		struct i2c_msg msgs[] = {
			{
			 .addr = client->addr,
			 .flags = 0,
			 .len = writelen,
			 .buf = writebuf,
			 },
			{
			 .addr = client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 2);
		if (ret < 0)
			dev_err(&client->dev, "f%s: i2c read error.\n",
				__func__);
	} else {
		struct i2c_msg msgs[] = {
			{
			 .addr = client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 1);
		if (ret < 0)
			dev_err(&client->dev, "%s:i2c read error.\n", __func__);
	}
	return ret;
}
/*write data by i2c*/
int ft5x0x_i2c_Write(struct i2c_client *client, char *writebuf, int writelen)
{
	int ret;

	struct i2c_msg msg[] = {
		{
		 .addr = client->addr,
		 .flags = 0,
		 .len = writelen,
		 .buf = writebuf,
		 },
	};

	ret = i2c_transfer(client->adapter, msg, 1);
	if (ret < 0)
		dev_err(&client->dev, "%s i2c write error.\n", __func__);

	return ret;
}

int ft5x0x_write(struct i2c_client *client, u8 regaddr, u8 regvalue)
{
	unsigned char buf[2] = {0};
	buf[0] = regaddr;
	buf[1] = regvalue;

	return ft5x0x_i2c_Write(client, buf, sizeof(buf));
}

/*release the point*/
static void ft5x0x_ts_release(struct ft5x0x_ts_data *data)
{
	input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
	input_sync(data->input_dev);
}

/*Read touch point information when the interrupt  is asserted.*/
static int ft5x0x_read_Touchdata(struct ft5x0x_ts_data *data)
{
	struct ts_event *event = &data->event;
	u8 buf[POINT_READ_BUF] = { 0 };
	int ret = -1;
	int i = 0;
	u8 pointid = FT_MAX_ID;

	ret = ft5x0x_i2c_Read(data->client, buf, 1, buf, POINT_READ_BUF);
	if (ret < 0) {
		dev_err(&data->client->dev, "%s read touchdata failed.\n",
			__func__);
		return ret;
	}
	memset(event, 0, sizeof(struct ts_event));

	event->touch_point = 0;
	for (i = 0; i < CFG_MAX_TOUCH_POINTS; i++) {
		pointid = (buf[FT_TOUCH_ID_POS + FT_TOUCH_STEP * i]) >> 4;
		if (pointid >= FT_MAX_ID)
			break;
		else
			event->touch_point++;
		event->au16_x[i] =
		    (s16) (buf[FT_TOUCH_X_H_POS + FT_TOUCH_STEP * i] & 0x0F) <<
		    8 | (s16) buf[FT_TOUCH_X_L_POS + FT_TOUCH_STEP * i];
		event->au16_y[i] =
		    (s16) (buf[FT_TOUCH_Y_H_POS + FT_TOUCH_STEP * i] & 0x0F) <<
		    8 | (s16) buf[FT_TOUCH_Y_L_POS + FT_TOUCH_STEP * i];
		event->au8_touch_event[i] =
		    buf[FT_TOUCH_EVENT_POS + FT_TOUCH_STEP * i] >> 6;
		event->au8_finger_id[i] =
		    (buf[FT_TOUCH_ID_POS + FT_TOUCH_STEP * i]) >> 4;
	}

	event->pressure = FT_PRESS;

	return 0;
}

/*
*report the point information
*/
static void ft5x0x_report_value(struct ft5x0x_ts_data *data)
{
	struct ts_event *event = &data->event;
	int i = 0;

	if (event->touch_point > 0) {
	for (i = 0; i < event->touch_point; i++) {
		/* LCD view area */
		if (event->au16_x[i] < data->x_max
		    && event->au16_y[i] < data->y_max) {
			input_report_abs(data->input_dev, ABS_MT_POSITION_X,
					 event->au16_x[i]);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y,
					 event->au16_y[i]);
			//input_report_abs(data->input_dev, ABS_MT_PRESSURE,
					 //event->pressure);
			input_report_abs(data->input_dev, ABS_MT_TRACKING_ID,
					 event->au8_finger_id[i]);
			if (event->au8_touch_event[i] == 0
			    || event->au8_touch_event[i] == 2) {
				input_report_abs(data->input_dev,
						 ABS_MT_TOUCH_MAJOR,
						 event->pressure);
				input_report_abs(data->input_dev, ABS_MT_PRESSURE, event->pressure);

				input_report_key(data->input_dev,BTN_TOUCH,1);
			} else {
				input_report_abs(data->input_dev,
						 ABS_MT_TOUCH_MAJOR, 0);
				input_report_abs(data->input_dev, ABS_MT_PRESSURE, 0);
				input_report_key(data->input_dev,BTN_TOUCH,0);
			}
		}

		input_mt_sync(data->input_dev);
	}
	input_sync(data->input_dev);
	} else {
	//if (event->touch_point == 0)
		ft5x0x_ts_release(data);
	}
}

/*The ft5x0x device will signal the host about TRIGGER_FALLING.
*Processed when the interrupt is asserted.
*/
static irqreturn_t ft5x0x_ts_interrupt(int irq, void *dev_id)
{
	/*struct ft5x0x_ts_data *ft5x0x_ts = dev_id;
	int ret = 0;
	disable_irq_nosync(INT_EI2);

	ret = ft5x0x_read_Touchdata(ft5x0x_ts);
	if (ret == 0)
		ft5x0x_report_value(ft5x0x_ts);

	enable_irq(INT_EI2);

	return IRQ_HANDLED;*/

	struct ft5x0x_ts_data *ft5x0x_ts = dev_id;

	//printk("%s\n", __func__);
    disable_irq_nosync(INT_IRQ);
    if (!work_pending(&ft5x0x_ts->pen_event_work)) {
        queue_work(ft5x0x_ts->ts_workqueue, &ft5x0x_ts->pen_event_work);
    }

    return IRQ_HANDLED;
}

static void ft5x0x_ts_pen_irq_work(struct work_struct *work)
{
    int ret = -1;
	//printk("%s\n", __func__);

/*#if CFG_SUPPORT_READ_LEFT_DATA
    do
    {
        ret = ft5x0x_read_data();    
        if (ret >= 0) {    
            ft5x0x_report_value();
        }
    }while (ret > 0);

#else
    ret = ft5x0x_read_data();    
    if (ret == 0) {    
        ft5x0x_report_value();
    }
#endif*/
	//struct ft5x0x_ts_data *ft5x0x_ts = dev_id;
	//struct ft5x0x_ts_data *ft5x0x_ts;

    //ft5x0x_ts = container_of(work, struct ft5x0x_ts_data, ts_workqueue);

	ret = ft5x0x_read_Touchdata(g_ft5x0x_ts);
	if (ret == 0)
		ft5x0x_report_value(g_ft5x0x_ts);

    enable_irq(INT_IRQ);
}

static int ft5x0x_ts_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	/*struct ft5x0x_platform_data *pdata =
	    (struct ft5x0x_platform_data *)client->dev.platform_data;*/
	struct ft5x0x_ts_data *ft5x0x_ts;
	struct input_dev *input_dev;
	int err = 0;
	int ret = 0;
	unsigned char uc_reg_value;
	unsigned char uc_reg_addr;

	//pdata->x_max = FT_MAX_X;
	//pdata->y_max = FT_MAX_Y;
	//IRQF_TRIGGER_FALLING = IRQF_TRIGGER_FALLING;

	//pdata->reset = RST_GPIO;

	//pdata->irq = INT_EI2;
	
	printk("++ %s\n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}

	ft5x0x_ts = kzalloc(sizeof(struct ft5x0x_ts_data), GFP_KERNEL);

	if (!ft5x0x_ts) {
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}

	i2c_set_clientdata(client, ft5x0x_ts);
	ft5x0x_ts->irq = INT_IRQ;
	ft5x0x_ts->client = client;
	//ft5x0x_ts->pdata = pdata;
	ft5x0x_ts->x_max = FT_MAX_X - 1;
	ft5x0x_ts->y_max = FT_MAX_Y - 1;
#ifdef CONFIG_PM
	err = gpio_request(RST_GPIO, "ft5x0x reset");
	if (err < 0) {
		dev_err(&client->dev, "%s:failed to set gpio reset.\n",
			__func__);
		goto exit_request_reset;
	}

	ret = gpio_request(INT_GPIO, "TS_INT");	//Request IO
	if (ret < 0) 
	{
		printk("gpio_request INT_GPIO fail\n");
	}

	//tcc_gpio_config(INT_GPIO, GPIO_FN(0)|GPIO_PULLUP);
	tcc_gpio_config(INT_GPIO, GPIO_FN(0));
	gpio_direction_input(INT_GPIO);
	tcc_gpio_config_ext_intr(INT_IRQ, INT_CFG);

	tcc_gpio_config(RST_GPIO, GPIO_FN(0)|GPIO_PULL_DISABLE);
	gpio_direction_output(RST_GPIO, 1);
	msleep(20);
	gpio_direction_output(RST_GPIO, 0);
	msleep(10);
	gpio_direction_output(RST_GPIO, 1);
	msleep(300);
#endif

    INIT_WORK(&ft5x0x_ts->pen_event_work, ft5x0x_ts_pen_irq_work);

    ft5x0x_ts->ts_workqueue = create_singlethread_workqueue(dev_name(&client->dev));
    if (!ft5x0x_ts->ts_workqueue) {
        err = -ESRCH;
        goto exit_create_singlethread;
    }

    err = request_irq(INT_IRQ, ft5x0x_ts_interrupt, IRQF_TRIGGER_FALLING, "ft5x0x_ts", ft5x0x_ts);
    if (err < 0) {
        dev_err(&client->dev, "ft5x0x_probe: request irq failed\n");
        goto exit_irq_request_failed;
    }

	g_ft5x0x_ts = ft5x0x_ts; 

	/*err = request_threaded_irq(INT_EI2, NULL, ft5x0x_ts_interrupt,
				   IRQF_TRIGGER_FALLING, client->dev.driver->name,
				   ft5x0x_ts);
	if (err < 0) {
		dev_err(&client->dev, "ft5x0x_probe: request irq failed\n");
		goto exit_irq_request_failed;
	}*/
	disable_irq(INT_IRQ);

	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		dev_err(&client->dev, "failed to allocate input device\n");
		goto exit_input_dev_alloc_failed;
	}

	ft5x0x_ts->input_dev = input_dev;

	set_bit(ABS_MT_TOUCH_MAJOR, input_dev->absbit);
	set_bit(ABS_MT_POSITION_X, input_dev->absbit);
	set_bit(ABS_MT_POSITION_Y, input_dev->absbit);
	set_bit(ABS_MT_PRESSURE, input_dev->absbit);

	input_set_abs_params(input_dev,
			     ABS_MT_POSITION_X, 0, ft5x0x_ts->x_max, 0, 0);
	input_set_abs_params(input_dev,
			     ABS_MT_POSITION_Y, 0, ft5x0x_ts->y_max, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, PRESS_MAX, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, PRESS_MAX, 0, 0);
	input_set_abs_params(input_dev,
			     ABS_MT_TRACKING_ID, 0, CFG_MAX_TOUCH_POINTS, 0, 0);

	set_bit(EV_KEY, input_dev->evbit);
	set_bit(EV_ABS, input_dev->evbit);

	input_dev->name = FT5X0X_NAME;
	err = input_register_device(input_dev);
	if (err) {
		dev_err(&client->dev,
			"ft5x0x_ts_probe: failed to register input device: %s\n",
			dev_name(&client->dev));
		goto exit_input_register_device_failed;
	}
	/*make sure CTP already finish startup process */
	msleep(150);

	/*get some register information */
	uc_reg_addr = FT5x0x_REG_FW_VER;
	ft5x0x_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	dev_dbg(&client->dev, "[FTS] Firmware version = 0x%x\n", uc_reg_value);

	uc_reg_addr = FT5x0x_REG_POINT_RATE;
	ft5x0x_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	dev_dbg(&client->dev, "[FTS] report rate is %dHz.\n",
		uc_reg_value * 10);

	uc_reg_addr = FT5X0X_REG_THGROUP;
	ft5x0x_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	dev_dbg(&client->dev, "[FTS] touch threshold is %d.\n",
		uc_reg_value * 4);


	fts_ctpm_auto_upgrade(client);
	//ft5x0x_create_sysfs(client);

	enable_irq(INT_IRQ);
	printk("-- %s\n", __func__);
	return 0;

exit_input_register_device_failed:
	input_free_device(input_dev);

exit_input_dev_alloc_failed:
	free_irq(INT_IRQ, ft5x0x_ts);
#ifdef CONFIG_PM
exit_request_reset:
	gpio_free(RST_GPIO);
#endif

exit_create_singlethread:
    printk("==singlethread error =\n");
exit_irq_request_failed:
	i2c_set_clientdata(client, NULL);
	kfree(ft5x0x_ts);
exit_alloc_data_failed:
exit_check_functionality_failed:
	return err;
}

static int ft5x0x_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	//struct ft5x0x_ts_data *ts = container_of(handler, struct ft5x0x_ts_data,
	//					early_suspend);
	struct ft5x0x_ts_data *ts = i2c_get_clientdata(client);
	int err;
	char buf[2] = {0xa5, 0x3};
	int val;
	int reg;

	dev_dbg(&g_ft5x0x_ts->client->dev, "[FTS]ft5x0x suspend\n");
	disable_irq(INT_IRQ);
	//cancel_work_sync(&g_ft5x0x_ts->pen_event_work);
	//flush_workqueue(g_ft5x0x_ts->ts_workqueue);
	//mdelay(10);

	//err = ft5x0x_i2c_Write(g_ft5x0x_ts->client, buf, sizeof(buf));
	err = ft5x0x_i2c_Write(ts->client, buf, sizeof(buf));
	//err = ft5x0x_write(g_ft5x0x_ts->client, 0xa5, 0x3);
	if (err < 0) {
		printk("ft5x0x enter sleep mode failed\n");
	}

	/*reg = 0xa5;
	ft5x0x_i2c_Read(g_ft5x0x_ts->client, &reg, 1, &val, 1);
	printk("[FTS] sleep register = 0x%x\n", val);*/

	printk("%s\n", __func__);

	return 0;
}

static int ft5x0x_ts_resume(struct i2c_client *client)
{
	//struct ft5x0x_ts_data *ts = container_of(handler, struct ft5x0x_ts_data,
	//					early_suspend);
	struct ft5x0x_ts_data *ts = i2c_get_clientdata(client);
	int val;
	int reg;

	dev_dbg(&g_ft5x0x_ts->client->dev, "[FTS]ft5x0x resume.\n");
	gpio_set_value(RST_GPIO, 0);
	msleep(20);
	gpio_set_value(RST_GPIO, 1);
	enable_irq(INT_IRQ);

	/*reg = 0xa5;
	ft5x0x_i2c_Read(g_ft5x0x_ts->client, &reg, 1, &val, 1);
	printk("[FTS] sleep register = 0x%x\n", val);*/

	printk("%s\n", __func__);

	return 0;
}

static int __devexit ft5x0x_ts_remove(struct i2c_client *client)
{
	struct ft5x0x_ts_data *ft5x0x_ts;
	ft5x0x_ts = i2c_get_clientdata(client);
	input_unregister_device(ft5x0x_ts->input_dev);
	#ifdef CONFIG_PM
	gpio_free(RST_GPIO);
	#endif
	free_irq(INT_IRQ, ft5x0x_ts);
	kfree(ft5x0x_ts);
	i2c_set_clientdata(client, NULL);
	return 0;
}

static const struct i2c_device_id ft5x0x_ts_id[] = {
	{"Goodix-TS"/*FT5X0X_NAME*/, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, ft5x0x_ts_id);

static struct i2c_driver ft5x0x_ts_driver = {
	.probe = ft5x0x_ts_probe,
	.remove = __devexit_p(ft5x0x_ts_remove),
	.id_table = ft5x0x_ts_id,
	.suspend = ft5x0x_ts_suspend,
	.resume = ft5x0x_ts_resume,
	.driver = {
		   .name = "Goodix-TS",//FT5X0X_NAME,
		   .owner = THIS_MODULE,
		   },
};

static int __init ft5x0x_ts_init(void)
{
	int ret;
	ret = i2c_add_driver(&ft5x0x_ts_driver);
	if (ret) {
		printk(KERN_WARNING "Adding ft5x0x driver failed "
		       "(errno = %d)\n", ret);
	} else {
		pr_info("Successfully added driver %s\n",
			ft5x0x_ts_driver.driver.name);
	}
	return ret;
}

static void __exit ft5x0x_ts_exit(void)
{
	i2c_del_driver(&ft5x0x_ts_driver);
}

module_init(ft5x0x_ts_init);
module_exit(ft5x0x_ts_exit);

MODULE_AUTHOR("<luowj>");
MODULE_DESCRIPTION("FocalTech ft5x0x TouchScreen driver");
MODULE_LICENSE("GPL");
