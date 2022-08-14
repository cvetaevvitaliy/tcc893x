#ifndef _LINUX_TCC_RW_H_
#define _LINUX_TCC_RW_H_

#include <linux/i2c.h>
#include <linux/power/tcc-mfd.h>

struct i2c_client *tcc270;
EXPORT_SYMBOL_GPL(tcc270);
#if 0
static int tcc270_chg_i2c_read(char* rxData, int rxLength)
{
	int rc;
	struct i2c_client *client = tcc270->client;
	struct i2c_adapter *adap = client->adapter;
	struct i2c_msg msg[2] = {
	//msg 1  send the addr + cmd reg addr
	{
		.addr = client->addr,
		.flags = client-> flags,
		.len = 1,
		.buf = rxData,
	},
	//msg 2 send the addr & it will return reg addr data
	{
		.addr = client->addr,
		.flags = client->flags | I2C_M_RD,
		.len = rxLength,
		.buf = rxData,
	},
	};
	rc = i2c_transfer(adap, msg, 2);
	return rc;
}
EXPORT_SYMBOL(tcc270_chg_i2c_read);

static int tcc270_chg_i2c_write(char* txData, int txLength)
{
	int rc;
	struct i2c_client *client = tcc270->client;
	struct i2c_adapter *adap = client->adapter;
	struct i2c_msg msg= {
		//data must include one reg addr and one or several datas into data buff
		.addr = client->addr,
		.flags = client->flags,
		.len = txLength,
		.buf = txData,
	};
	rc = i2c_transfer(adap, &msg, 1);
	return rc;
}
EXPORT_SYMBOL(tcc270_chg_i2c_write);
#endif

#if 0
static int tcc270_read_reg(struct i2c_client *client, u8 reg, u8 *data, u8 len)
{
	struct i2c_adapter *adap = client->adapter;
	struct i2c_msg msgs[2];
	int ret;
	
	msgs[0].addr = client->addr;
	msgs[0].flags = client->flags;
	msgs[0].len = 1;
	msgs[0].buf = &reg;

	msgs[1].addr = client->addr;
	msgs[1].flags = client->flags | I2C_M_RD;
	msgs[1].len = len;
	msgs[1].buf = data;
	
	ret = i2c_transfer(adap, msgs, 2);
	 
	return (ret == 2)? len : ret;									 
}
EXPORT_SYMBOL(tcc270_read_reg);

static int tcc270_write_reg(struct i2c_client *client,
				u8 reg, u8 *data, u8 len)
{
	struct i2c_adapter *adap = client->adapter;
	struct i2c_msg msg;
	int ret;
	char *tx_buf = (char *)kmalloc(len + 1, GFP_KERNEL);
	
	if(!tx_buf)
		return -ENOMEM;
	tx_buf[0] = reg;
	memcpy(tx_buf+1, data, len);
	
	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.len = len + 1;
	msg.buf = (char *)tx_buf;

	ret = i2c_transfer(adap, &msg, 1);
	kfree(tx_buf);
	return (ret == 1) ? len : ret; 
}
EXPORT_SYMBOL(tcc270_write_reg);
#endif

static inline int __tcc270_read(struct i2c_client *client,
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
EXPORT_SYMBOL(__tcc270_read);

static inline int __tcc270_reads(struct i2c_client *client, int reg,
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
EXPORT_SYMBOL(__tcc270_reads);

static inline int __tcc270_write(struct i2c_client *client,
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
EXPORT_SYMBOL(__tcc270_write);

static inline int __tcc270_writes(struct i2c_client *client, int reg,
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
EXPORT_SYMBOL(__tcc270_writes);
#if 0
int tcc270_register_notifier(struct device *dev, struct notifier_block *nb,
				uint64_t irqs)
{
	struct tcc270_mfd_chip *chip = dev_get_drvdata(dev);

	chip->ops->enable_irqs(chip, irqs);
	if(NULL != nb) {
		return blocking_notifier_chain_register(&chip->notifier_list, nb);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(axp_register_notifier);

int tcc270_unregister_notifier(struct device *dev, struct notifier_block *nb,
				uint64_t irqs)
{
	struct tcc270_mfd_chip *chip = dev_get_drvdata(dev);

	chip->ops->disable_irqs(chip, irqs);
	if(NULL != nb) {
		return blocking_notifier_chain_unregister(&chip->notifier_list, nb);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(tcc270_unregister_notifier);
#endif
int tcc270_write(struct device *dev, int reg, uint8_t val)
{
	return __tcc270_write(to_i2c_client(dev), reg, val);
}
EXPORT_SYMBOL_GPL(tcc270_write);

int tcc270_writes(struct device *dev, int reg, int len, uint8_t *val)
{
	return	__tcc270_writes(to_i2c_client(dev), reg, len, val);
}
EXPORT_SYMBOL_GPL(tcc270_writes);

int tcc270_read(struct device *dev, int reg, uint8_t *val)
{
	return __tcc270_read(to_i2c_client(dev), reg, val);
}
EXPORT_SYMBOL_GPL(tcc270_read);

int tcc270_reads(struct device *dev, int reg, int len, uint8_t *val)
{
	return __tcc270_reads(to_i2c_client(dev), reg, len, val);
}
EXPORT_SYMBOL_GPL(tcc270_reads);

int tcc270_set_bits(struct i2c_client *client , int reg, uint8_t bit_mask)
{
//	struct tcc270_mfd_chip *chip = dev_get_drvdata(dev);
	uint8_t reg_val;
	int ret = 0;

//	mutex_lock(&chip->lock);

	ret = __tcc270_read(client, reg, &reg_val);
	if (ret)
		goto out;

	if ((reg_val & bit_mask) != bit_mask) {
		reg_val |= bit_mask;
		ret = __tcc270_write(client, reg, reg_val);
	}
out:
//	mutex_unlock(&chip->lock);
	return ret;
}
EXPORT_SYMBOL_GPL(tcc270_set_bits);

int tcc270_clr_bits(struct i2c_client *client, int reg, uint8_t bit_mask)
{
//	struct axp_mfd_chip *chip = dev_get_drvdata(dev);
	uint8_t reg_val;
	int ret = 0;

//	mutex_lock(&chip->lock);

	ret = __tcc270_read(client, reg, &reg_val);
	if (ret)
		goto out;

	if (reg_val & bit_mask) {
		reg_val &= ~bit_mask;
		ret = __tcc270_write(client, reg, reg_val);
	}
out:
//	mutex_unlock(&chip->lock);
	return ret;
}
EXPORT_SYMBOL_GPL(tcc270_clr_bits);

int tcc270_update(struct i2c_client *client, int reg, uint8_t val, uint8_t mask)
{
//	struct axp_mfd_chip *chip = dev_get_drvdata(dev);
	uint8_t reg_val;
	int ret = 0;

//	mutex_lock(&chip->lock);

	ret = __tcc270_read(client, reg, &reg_val);
	if (ret)
		goto out;

	if ((reg_val & mask) != val) {
		reg_val = (reg_val & ~mask) | val;
		ret = __tcc270_write(client, reg, reg_val);
	}
out:
//	mutex_unlock(&chip->lock);
	return ret;
}
EXPORT_SYMBOL_GPL(tcc270_update);

struct device *tcc270_get_dev(void)
{
	return &tcc270->dev;
}
EXPORT_SYMBOL_GPL(tcc270_get_dev);

#endif

