/*****************************************************************************************
 *
 * FILE NAME          : MxL101SF_OEM_Drv.cpp
 * 
 * AUTHOR             : Brenndon Lee, Mahendra Kondur
 *
 * DATE CREATED       : 1/24/2008
 *                      6/3/2010 - Description of I2C protocol compatible to MxL
 *
 * DESCRIPTION        : This file contains I2C emulation routine
 *                      
 *****************************************************************************************
 *                Copyright (c) 2006, MaxLinear, Inc.
 ****************************************************************************************/
#define LOG_TAG	"MXL101SF_DVBT"
#include <utils/Log.h>

#include <stdio.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <sys/time.h>
#include "MxL101SF_OEM_Drv.h"

#define     DEBUG_PRITF     printf

//#define MXL101SF_CHIP_ADDR   0x60
#define I2C_SLAVE   0x0703  /* Change slave address         */
#define I2C_WRITE 0
#define I2C_READ  1

static int g_i2c_fd = -1, g_i2c_slave_addr = -1;

MXL_STATUS Ctrl_SetI2CAddr(UINT8 I2CSlaveAddr)
{
	int err;
	
	if(g_i2c_slave_addr == I2CSlaveAddr)	  
		return MXL_TRUE;
		
	g_i2c_slave_addr = I2CSlaveAddr;							
	DEBUG_PRITF("[%s] CHIP ADDR 0x%X \n", __func__, g_i2c_slave_addr);
	err = ioctl(g_i2c_fd, I2C_SLAVE /*IOCTL_SET_SLAVE_ADDR*/, g_i2c_slave_addr); /* 7*/
	if(err < 0)
	{
	    DEBUG_PRITF("[%s:%d] erro : %s \n", __func__, __LINE__, strerror(errno));
		return MXL_FALSE;
	}
	return MXL_TRUE;
}

MXL_STATUS Ctrl_I2cConnect(UINT8 I2CSlaveAddr)
{
    MXL_STATUS status = MXL_TRUE;    
    int i2c_port = 1, err;
    char filename[20];

    g_i2c_fd = -1;
    g_i2c_slave_addr = -1;

    sprintf(filename, "/dev/i2c-%d", i2c_port);
    g_i2c_fd = open(filename,O_RDWR);
    if (g_i2c_fd < 0)
    {
        DEBUG_PRITF("[%s:%d] erro : %s \n", __func__, __LINE__, strerror(errno));
        return MXL_FALSE;
    }
    Ctrl_SetI2CAddr(I2CSlaveAddr);	
    return status;
}

MXL_STATUS Ctrl_I2cDisConnect()
{
    MXL_STATUS status = MXL_TRUE;    
    if(g_i2c_fd >= 0)
        close(g_i2c_fd);

    g_i2c_fd = -1;
    return status;
}
/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : Ctrl_ReadRegister
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 1/24/2008
--|                 6/3/2010 - Description MxL101SF I2C read operation.
--|
--| DESCRIPTION   : This function does I2C read operation with MxL101SF.
--|                 The function accepts i2c slave address of MxL101SF along with
--|                 register address, pointer for register data and a pointer to a 
--|                 private data structure which can be used for custom specific 
--|                 operations. If private data structure is not required, user can
--|                 pass NULL.
--|
--| RETURN VALUE  : True or False
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS Ctrl_ReadRegister(UINT8 I2CSlaveAddr, 
                             UINT8 RegAddr, 
                             UINT8 *DataPtr, 
                             void *PrivateDataPtr)
{
    MXL_STATUS status = MXL_TRUE;
    int	err;
  char buf[2];

  /* OEM should implement I2C read protocol that complies with MxL101SF I2C
     format.

     +------+-+-----+-+-+----+-+-------+-+-+
     |MASTER|S|SADDR|W| |0xFB| |RegAddr| |P|
     +------+-+-----+-+-+----+-+-------+-+-+
     |SLAVE |         |A|    |A|       |A| |
     +------+-+-----+-+-+----+-+----+-+--+-+
     +------+-+-----+-+-+-------+--+-+
     |MASTER|S|SADDR|R| |       |MN|P|
     +------+-+-----+-+-+-------+--+-+
     |SLAVE |         |A| Data  |  | |
     +------+---------+-+-------+--+-+
     Legends: SADDR(I2c slave address), 
              S(Start condition), 
              MN(Master NACK), 
              P(Stop condition) */  
  
  	
    Ctrl_SetI2CAddr(I2CSlaveAddr);	
    buf[0] = 0xFB;
    buf[1] = RegAddr;
    err= write(g_i2c_fd, buf, 2);
    if (err!=2)
    {	
        /* ERROR HANDLING: i2c transaction failed */ 
        DEBUG_PRITF("[%s:%d] erro : %s \n", __func__, __LINE__, strerror(errno));
        return MXL_FALSE;
    }  

    err= read(g_i2c_fd, DataPtr, 1);
    if (err!=1)
    {	
        /* ERROR HANDLING: i2c transaction failed */ 
        DEBUG_PRITF("[%s:%d] erro : %s \n", __func__, __LINE__, strerror(errno));
        return MXL_FALSE;
    }  
    
  /* DEBUG_PRITF("MxL Reg Read : Addr - 0x%x, data - 0x%x\n", RegAddr, *DataPtr); */  
 
    return status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : Ctrl_WriteRegister
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 1/24/2008
--|                 6/3/2010 - Description MxL101SF I2C write operation.
--|
--| DESCRIPTION   : This function does I2C write operation with MxL101SF.
--|                 The function accepts i2c slave address of MxL101SF along with
--|                 register address, register data value and a pointer to a 
--|                 private data structure which can be used for custom specific 
--|                 operations. If private data structure is not required, user can
--|                 pass NULL.
--|
--| RETURN VALUE  : True or False
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS Ctrl_WriteRegister(UINT8 I2CSlaveAddr, 
                              UINT8 RegAddr, 
                              UINT8 RegData, 
                              void *PrivateDataPtr)
{
    MXL_STATUS status = MXL_TRUE;
    int	err;
    char buf[2];

  /* OEM should implement I2C write protocol that complies with MxL101SF I2C
     format.

     +------+-+-----+-+-+----------+-+----------+-+-+
     |MASTER|S|SADDR|W| |  RegAddr | | RegData  | |P|
     +------+-+-----+-+-+----------+-+----------+-+-+
     |SLAVE |         |A|          |A|          |A| |
     +------+---------+-+----------+-+----------+-+-+
     Legends: SADDR (I2c slave address), 
              S (Start condition), 
              A (Ack), N(NACK), 
              P(Stop condition) */
  
  // User should implememnt his own I2C write register routine corresponding to
  // his hardaware.   
	Ctrl_SetI2CAddr(I2CSlaveAddr);	
    buf[0] = RegAddr;
    buf[1] = RegData;

    err= write(g_i2c_fd, buf, 2);
    if(err != 2)
    {
        DEBUG_PRITF("[%s:%d] erro : %s \n", __func__, __LINE__, strerror(errno));
        return MXL_FALSE;
    }
    /* DEBUG_PRITF("MxL Reg Write : Addr - 0x%x, data - 0x%x\n", RegAddr, RegData); */    

    return status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : Ctrl_Sleep
--| 
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 8/31/2009
--|
--| DESCRIPTION   : This function will cause the calling thread to be suspended 
--|                 from execution until the number of milliseconds specified by 
--|                 the argument time has elapsed 
--|
--| RETURN VALUE  : True or False
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS Ctrl_Sleep(UINT16 TimeinMilliseconds)
{
  MXL_STATUS status = MXL_TRUE;

   /* User should implememnt his own sleep routine corresponding to
     his Operating System platform. */
  //DEBUG_PRITF("Ctrl_Sleep : %d msec's\n", TimeinMilliseconds);    
  usleep(TimeinMilliseconds*1000);
  return status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : Ctrl_GetTime
--| 
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 10/05/2009
--|
--| DESCRIPTION   : This function will return current system's timestamp in 
--|                 milliseconds resolution. 
--|
--| RETURN VALUE  : True or False
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS Ctrl_GetTime(UINT32 *TimeinMillisecondsPtr)
{
  struct timeval tv;
  /* *TimeinMillisecondsPtr = current systems timestamp in milliseconds.
     User should implement his own routine to get current system's timestamp 
     in milliseconds. */
  gettimeofday (&tv, NULL);
  *TimeinMillisecondsPtr = (long long) tv.tv_sec * 1000 + tv.tv_usec / 1000;
  return MXL_TRUE;
}
