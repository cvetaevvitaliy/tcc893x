/*
 * Read/Write driver for X-Powers AXP
 *
 * Copyright (C) 2011 X-Powers, Ltd.
 *  Zhang Donglu <zhangdonglu@x-powers.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
 
#ifndef _LINUX_AXP_RW_H_
#define _LINUX_AXP_RW_H_

#include <linux/i2c.h>
#include <linux/power/axp-mfd.h>

struct i2c_client *axp;
EXPORT_SYMBOL_GPL(axp);

static inline int __axp_read(struct i2c_client *client,
				int reg, uint8_t *val)
{
	int ret;

	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0) {
		dev_err(&client->dev, "failed reading at 0x%02x\n", reg);
		return ret;
	}

	*val = (uint8_t)ret;
	return 0;
}
EXPORT_SYMBOL(__axp_read);

static inline int __axp_reads(struct i2c_client *client, int reg,
				 int len, uint8_t *val)
{
	int ret;

	if(client == NULL)
		printk("i2c client is NULL\n");
	
	ret = i2c_smbus_read_i2c_block_data(client, reg, len, val);
	if (ret < 0) {
		dev_err(&client->dev, "failed reading from 0x%02x\n", reg);
		return ret;
	}
	return 0;
}
EXPORT_SYMBOL(__axp_reads);

static inline int __axp_write(struct i2c_client *client,
				 int reg, uint8_t val)
{
	int ret;

	ret = i2c_smbus_write_byte_data(client, reg, val);
	if (ret < 0) {
		dev_err(&client->dev, "failed writing 0x%02x to 0x%02x\n",
				val, reg);
		return ret;
	}
	return 0;
}


static inline int __axp_writes(struct i2c_client *client, int reg,
				  int len, uint8_t *val)
{
	int ret;

	ret = i2c_smbus_write_i2c_block_data(client, reg, len, val);
	if (ret < 0) {
		dev_err(&client->dev, "failed writings to 0x%02x\n", reg);
		return ret;
	}
	return 0;
}

int axp_register_notifier(struct device *dev, struct notifier_block *nb,
				uint64_t irqs)
{
	struct axp_mfd_chip *chip = dev_get_drvdata(dev);

	chip->ops->enable_irqs(chip, irqs);
	if(NULL != nb) {
	    return blocking_notifier_chain_register(&chip->notifier_list, nb);
    }

    return 0;
}
EXPORT_SYMBOL_GPL(axp_register_notifier);

int axp_unregister_notifier(struct device *dev, struct notifier_block *nb,
				uint64_t irqs)
{
	struct axp_mfd_chip *chip = dev_get_drvdata(dev);

	chip->ops->disable_irqs(chip, irqs);
	if(NULL != nb) {
	    return blocking_notifier_chain_unregister(&chip->notifier_list, nb);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(axp_unregister_notifier);

int axp_write(struct device *dev, int reg, uint8_t val)
{
	return __axp_write(to_i2c_client(dev), reg, val);
}
EXPORT_SYMBOL_GPL(axp_write);

int axp_writes(struct device *dev, int reg, int len, uint8_t *val)
{
	return  __axp_writes(to_i2c_client(dev), reg, len, val);
}
EXPORT_SYMBOL_GPL(axp_writes);

int axp_read(struct device *dev, int reg, uint8_t *val)
{
	return __axp_read(to_i2c_client(dev), reg, val);
}
EXPORT_SYMBOL_GPL(axp_read);

int axp_reads(struct device *dev, int reg, int len, uint8_t *val)
{
	return __axp_reads(to_i2c_client(dev), reg, len, val);
}
EXPORT_SYMBOL_GPL(axp_reads);

int axp_set_bits(struct i2c_client *client , int reg, uint8_t bit_mask)
{
//	struct axp_mfd_chip *chip = dev_get_drvdata(dev);
	uint8_t reg_val;
	int ret = 0;

//	mutex_lock(&chip->lock);

	ret = __axp_read(client, reg, &reg_val);
	if (ret)
		goto out;

	if ((reg_val & bit_mask) != bit_mask) {
		reg_val |= bit_mask;
		ret = __axp_write(client, reg, reg_val);
	}
out:
//	mutex_unlock(&chip->lock);
	return ret;
}
EXPORT_SYMBOL_GPL(axp_set_bits);

int axp_clr_bits(struct i2c_client *client, int reg, uint8_t bit_mask)
{
//	struct axp_mfd_chip *chip = dev_get_drvdata(dev);
	uint8_t reg_val;
	int ret = 0;

//	mutex_lock(&chip->lock);

	ret = __axp_read(client, reg, &reg_val);
	if (ret)
		goto out;

	if (reg_val & bit_mask) {
		reg_val &= ~bit_mask;
		ret = __axp_write(client, reg, reg_val);
	}
out:
//	mutex_unlock(&chip->lock);
	return ret;
}
EXPORT_SYMBOL_GPL(axp_clr_bits);

int axp_update(struct i2c_client *client, int reg, uint8_t val, uint8_t mask)
{
//	struct axp_mfd_chip *chip = dev_get_drvdata(dev);
	uint8_t reg_val;
	int ret = 0;

//	mutex_lock(&chip->lock);

	ret = __axp_read(client, reg, &reg_val);
	if (ret)
		goto out;

	if ((reg_val & mask) != val) {
		reg_val = (reg_val & ~mask) | val;
		ret = __axp_write(client, reg, reg_val);
	}
out:
//	mutex_unlock(&chip->lock);
	return ret;
}
EXPORT_SYMBOL_GPL(axp_update);

struct device *axp_get_dev(void)
{
	return &axp->dev;
}
EXPORT_SYMBOL_GPL(axp_get_dev);

#endif