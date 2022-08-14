/*****************************************************************************************
 *
 * FILE NAME          : MN_I2C.C
 * 
 ****************************************************************************************/
#include <linux/delay.h>
#include <linux/i2c.h>

#include "../common/MN_DMD_common.h"
#include "../common/MN_DMD_driver.h"
#include "../common/MN_I2C.h"

//#define DMD_I2C_DEBUG
#ifdef DMD_I2C_DEBUG
DMD_u32_t	dmd_i2c_debug_flag = 1;
#endif
/* **************************************************** */
/* **************************************************** */
#define DMD_I2C_MAXSIZE     DMD_REGISTER_MAX

#define     DEBUG_PRITF     ALOGD

extern int mn88472_get_fe_config(struct i2c_adapter **i2c_handle);

int Ctrl_SetI2CAddr(char I2CSlaveAddr)
{
	return 0;
}

void DMD_API DMD_I2C_close(void)
{
}

DMD_ERROR_t DMD_API DMD_I2C_open(void)
{
	return DMD_E_OK;
}
/*! Write 1byte */
DMD_ERROR_t DMD_I2C_Write(DMD_u8_t	slvadr , DMD_u8_t adr , DMD_u8_t data )
{
	int ret;
	struct i2c_msg msg;
	struct i2c_adapter *i2c_handle;
	char buf[2];

    buf[0] = adr;
    buf[1] = data;

	// write value
	memset(&msg, 0, sizeof(msg));
	msg.addr = slvadr;
	msg.flags = 0;
	msg.buf = buf;
	msg.len = 2;

	mn88472_get_fe_config(&i2c_handle);
	ret = i2c_transfer(i2c_handle, &msg, 1);
	if(ret<0){
		printk(" %s: writereg error, errno is %d \n", __FUNCTION__, ret);
		return DMD_E_ERROR;
	}
	return DMD_E_OK;
}

/*! Read 1byte */
DMD_ERROR_t DMD_I2C_Read(DMD_u8_t	slvadr , DMD_u8_t adr , DMD_u8_t *data )
{
	int ret;
	struct i2c_msg msg;
	struct i2c_adapter *i2c_handle;
	char buf[1];

	buf[0] = adr;

	// write value
	memset(&msg, 0, sizeof(msg));
	msg.addr = slvadr;
	msg.flags = 0;
	msg.buf = buf;
	msg.len = 1;

	mn88472_get_fe_config(&i2c_handle);
	ret = i2c_transfer(i2c_handle, &msg, 1);
	if(ret<0){
		printk(" %s: writereg error, errno is %d \n", __FUNCTION__, ret);
		return DMD_E_ERROR;
	}

	// read value
	memset(&msg, 0, sizeof(msg));
	msg.addr = slvadr;
	msg.flags |=  I2C_M_RD;  //write  I2C_M_RD=0x01
	msg.buf = buf;
	msg.len = 1;

	mn88472_get_fe_config(&i2c_handle);
	ret = i2c_transfer(i2c_handle, &msg, 1);
	if(ret != 1)
	{
		printk("mxl101_readregister reg failure!\n");
		return DMD_E_ERROR;
	}

	*data = buf[0];

	return DMD_E_OK;
}

/*! Write/Read any Length*/
DMD_ERROR_t DMD_I2C_WriteRead(DMD_u8_t	slvadr , DMD_u8_t adr , DMD_u8_t* wdata , DMD_u32_t wlen , DMD_u8_t* rdata , DMD_u32_t rlen)
{
	int ret;
	struct i2c_msg msg;
	struct i2c_adapter *i2c_handle;	
	char wd[DMD_REGISTER_MAX + 2]; //32 is for saving address
	char rd[DMD_REGISTER_MAX];
	unsigned int i;

    wd[0] = adr;
	for(i=0;i<wlen;i++) 
	    wd[i+1] = (char) wdata[i];
   
	// write value
	memset(&msg, 0, sizeof(msg));
	msg.addr = slvadr;
	msg.flags = 0;
	msg.buf = wd;
	msg.len = wlen+1;

	mn88472_get_fe_config(&i2c_handle);
	ret = i2c_transfer(i2c_handle, &msg, 1);
	if(ret<0){
		printk(" %s: writereg error, errno is %d \n", __FUNCTION__, ret);
		return DMD_E_ERROR;
	}

    if( rlen == 0 || !rdata)
        return DMD_E_OK;

	// read value
	memset(&msg, 0, sizeof(msg));
	msg.addr = slvadr;
	msg.flags |=  I2C_M_RD;  //write  I2C_M_RD=0x01
	msg.buf = rd;
	msg.len = rlen;

	mn88472_get_fe_config(&i2c_handle);
	ret = i2c_transfer(i2c_handle, &msg, 1);
	if(ret != rlen)
	{
		printk("mxl101_readregister reg failure!\n");
		return DMD_E_ERROR;
	}

	for(i=0;i<rlen;i++) rdata[i] = (DMD_u8_t) rd[i];

	return DMD_E_OK;
}

/*! Timer wait */
DMD_ERROR_t DMD_wait( DMD_u32_t msecond ){

	//TODO: call timer wait function 
	udelay(msecond*1000);

	return DMD_E_OK;
}

/*! Get System Time (ms) */
DMD_ERROR_t DMD_timer( DMD_u32_t* tim ){

    *tim = jiffies_to_msecs(jiffies);

	return DMD_E_OK;
}
