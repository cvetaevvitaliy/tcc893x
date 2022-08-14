/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions andlimitations under the License.

****************************************************************************/

/****************************************************************************

Revision History

****************************************************************************

****************************************************************************/
#define LOG_TAG	"SPI"
#include <utils/Log.h>

#include "user_debug_levels.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <linux/types.h>
//#include <linux/spi/spidev.h>

#include "spidev.h"
#include "SPI.h"

#define DEV_NAME                "/dev/spidev0.0"
#define DEV_TEMP_BUF_SIZE    	4096 //2048	

static int g_SPIHandle = -1;
static char* g_pSPITXBuf = NULL;
static char* g_pSPIRXBuf = NULL;

/**************************************************************************
*  FUNCTION NAME : 
*     int SPI_Init(void)
*  
*  DESCRIPTION : Port config
*  INPUT:
*  OUTPUT:	
*  REMARK  :	
**************************************************************************/
int SPI_Init(void)
{
    unsigned char spi_mode;
    unsigned int spi_max_clock;

    g_SPIHandle = open(DEV_NAME, O_RDWR|O_NDELAY);
    if( g_SPIHandle < 0 )
    {
        printf("[SPI.c] Error SpiInitSPI_Handle:%d)\n", g_SPIHandle);
        DEBUG(DEB_LEV_ERR, "Error SpiInitSPI_Handle\n");
        return FALSE; 
    }

    g_pSPITXBuf = (char*)malloc(DEV_TEMP_BUF_SIZE);
    if( !g_pSPITXBuf )
    {
        printf("[SPI.c] Error SPI_Init(g_pSPITXBuf:0)\n");
        DEBUG(DEB_LEV_ERR, "Error SpiInit(g_pSPITXBuf:0)\n");
        close(g_SPIHandle);
        g_SPIHandle = 0;
        return FALSE;
    }

    g_pSPIRXBuf = (char*)malloc(DEV_TEMP_BUF_SIZE);
    if( !g_pSPIRXBuf )
    {
        printf("[SPI.c] Error SPI_Init(g_pSPIRXBuf:0)\n");
        DEBUG(DEB_LEV_ERR, "Error SpiInit(g_pSPIRXBuf:0)\n");
        close(g_SPIHandle);
        g_SPIHandle = 0;
        return FALSE;
    }

    ioctl(g_SPIHandle, SPI_IOC_RD_MODE, &spi_mode);
	spi_mode |= SPI_MODE_3;
    ioctl(g_SPIHandle, SPI_IOC_WR_MODE, &spi_mode);


    spi_max_clock = 10000000;
    ioctl(g_SPIHandle, SPI_IOC_WR_MAX_SPEED_HZ, &spi_max_clock);

    ioctl(g_SPIHandle, SPI_IOC_RD_MAX_SPEED_HZ, &spi_max_clock);
    printf("spi_max_clock:%d\n", spi_max_clock);

    return TRUE;
}

/**************************************************************************
*  FUNCTION NAME : 
*     int SPI_Destroy(void)
*  
*  DESCRIPTION : Port config
*  INPUT:
*  OUTPUT:	
*  REMARK  :	
**************************************************************************/
void SPI_Destroy(void)
{
    if( g_SPIHandle)
    {
        close(g_SPIHandle);
        g_SPIHandle = 0;	
    }

    if( g_pSPITXBuf)
    {
        free(g_pSPITXBuf);
        g_pSPITXBuf = NULL;
    }

    if( g_pSPIRXBuf)
    {
        free(g_pSPIRXBuf);
        g_pSPIRXBuf = NULL;
    }
}

/**************************************************************************
*  FUNCTION NAME : 
*     int SpiInit(void)
*  
*  DESCRIPTION : support to libDibDriverLibrary.a
*  INPUT:
*  OUTPUT:	
*  REMARK  :	
**************************************************************************/
int SpiInit(void)
{
    unsigned int spi_max_clock;

printf("[SPI.c] SpiInit 3..\n");
/*	
    ioctl(g_SPIHandle, SPI_IOC_RD_MAX_SPEED_HZ, &spi_max_clock);
    spi_max_clock = 4000000;
    ioctl(g_SPIHandle, SPI_IOC_WR_MAX_SPEED_HZ, &spi_max_clock);
*/
    return 0;
}

/**************************************************************************
*  FUNCTION NAME : 
*     int SpiWrite(unsigned char* buf, unsigned int size)
*  
*  DESCRIPTION : support to libDibDriverLibrary.a
*  INPUT:
*  OUTPUT:	
*  REMARK  :	
**************************************************************************/
int SpiWrite(unsigned char* buf, unsigned int size)
{
    struct spi_ioc_transfer msg;
    int ret;

    memcpy(g_pSPITXBuf, buf, size);
		
    msg.tx_buf = g_pSPITXBuf;
    msg.rx_buf = g_pSPIRXBuf;
    msg.len = size;
    msg.speed_hz = 10000000;
//    msg.speed_hz = 2000000;
    msg.bits_per_word = 8;
//    msg.cs_change = 1;

    ret =  ioctl(g_SPIHandle, SPI_IOC_MESSAGE(1), &msg);
    if( (unsigned int)ret != size )
    {
        printf("[SPI.c] Error SpiWrite1(size:%d, ret:%d)\n", size, ret);
  	return -1;
    }

    return ret;	
}

/**************************************************************************
*  FUNCTION NAME : 
*     int SpiRead(unsigned char* buf, unsigned int size)
*  
*  DESCRIPTION : support to libDibDriverLibrary.a
*  INPUT:
*  OUTPUT:	
*  REMARK  :	
**************************************************************************/

int SpiRead(unsigned char* buf, unsigned int size)
{
    struct spi_ioc_transfer msg;
    int ret;
    int i;

    memset(g_pSPITXBuf, 0, size);

    msg.tx_buf = g_pSPITXBuf;
    msg.rx_buf = g_pSPIRXBuf;
    msg.len = size;
    msg.speed_hz = 10000000;
//    msg.speed_hz  = 2000000;
    msg.bits_per_word = 8;
//    msg.cs_change = 1;

    ret = ioctl(g_SPIHandle, SPI_IOC_MESSAGE(1), &msg);
    if( (unsigned int)ret != size )
    {
        printf("[SPI.c] Error SpiRead(size:%d, ret:%d)\n", size, ret);
        return -1;
    }

    memcpy(buf, g_pSPIRXBuf, size);

    return ret;
}

int SpiReadDummyByteCsHigh(unsigned char* buf, unsigned int size)
{
    unsigned char spi_mode, temp_mode;
    struct spi_ioc_transfer msg;
    int ret;

return 0;

#if 0
    ioctl(g_SPIHandle, SPI_IOC_RD_MODE, &spi_mode);
	temp_mode = spi_mode | SPI_CS_HIGH;
    ioctl(g_SPIHandle, SPI_IOC_WR_MODE, &temp_mode);

    memset(g_pSPITXBuf, 0, size);

    msg.tx_buf = (__u64)g_pSPITXBuf;
    msg.rx_buf = (__u64)g_pSPIRXBuf;
    msg.len = size;
    msg.speed_hz = 10000000;
    msg.bits_per_word = 8;
//    msg.cs_change = 1;

    ret = ioctl(g_SPIHandle, SPI_IOC_MESSAGE(1), &msg);
    if( (unsigned int)ret != size )
    {
        printf("[SPI.c] Error SpiRead(size:%d, ret:%d)\n", size, ret);
        return -1;
    }

    ioctl(g_SPIHandle, SPI_IOC_WR_MODE, &spi_mode);

    memcpy(buf, g_pSPIRXBuf, size);

    return ret;
 #endif
}
