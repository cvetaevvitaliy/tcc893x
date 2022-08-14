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

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <linux/types.h>

#include "TC3X_IO_CSPI.h"
#include "TC3X_IO_UTIL.h"
#include "TC3X_IO.h"

#include "SPI.h"
#include "spidev.h"

int    ghSPI[MAX_TCC3X];                              /* CSPI mode. added by KST 091111 */

#define SPI_MAX_DMA_2048
//#define SPI_MAX_DMA_4096

#ifdef SPI_MAX_DMA_2048
#define DMA_MAX 2048
#else
#define DMA_MAX 4096
#endif

unsigned char SPI_TMPBUFF[DMA_MAX];
extern tSTDInfo g_tSTDInfo[MAX_TCC3X];

void SetupSPI (int moduleidx)
{
    unsigned char spi_mode = 0;
    unsigned int spi_max_clock;

    switch (moduleidx)
    {
    case 0:
        // use D port
        ghSPI[moduleidx] = open ("/dev/spidev0.0", O_RDWR | O_NDELAY);
        break;

    case 1:
        // use D port
        ghSPI[moduleidx] = open ("/dev/spidev0.0", O_RDWR | O_NDELAY);
        break;

    case 2:
        PRINTF_LV_0 ("[SetupSPI Error] Please insert code \n");
        break;

    case 3:
        PRINTF_LV_0 ("[SetupSPI Error] Please insert code \n");
        break;

    default:
        PRINTF_LV_0 ("[SetupSPI Error] Please insert code \n");
        break;
    }

    if (ghSPI[moduleidx]<1)
    {
        PRINTF_LV_0 ("INVALID_HANDLE_VALUE==ghSPI");
    }
    else
    {
        //SET SPI using IOCTL
        ioctl(ghSPI[moduleidx], SPI_IOC_RD_MODE, &spi_mode);
    	PRINTF_LV_0("spimode:%d\n", spi_mode);
    	spi_mode |= SPI_MODE_3;
        ioctl(ghSPI[moduleidx], SPI_IOC_WR_MODE, &spi_mode);


        spi_max_clock = 5000000; //20000000;
        ioctl(ghSPI[moduleidx], SPI_IOC_WR_MAX_SPEED_HZ, &spi_max_clock);
        ioctl(ghSPI[moduleidx], SPI_IOC_RD_MAX_SPEED_HZ, &spi_max_clock);
        PRINTF_LV_2("spi_max_clock:%d\n", spi_max_clock);
    }
}

TC3X_IO_Err TC3X_IO_CSPI_Close (int moduleidx)
{
    close (ghSPI[moduleidx]);
    ghSPI[moduleidx] = 0;
    return TC3X_IO_SUCCESS;
}

int SPIWriteString (int moduleidx, unsigned char * pData, unsigned int Length)
{
    int bret;
    struct spi_ioc_transfer msg;

    bret = 0;
    TC3X_IO_memset(&msg, 0x00, sizeof(msg));
    msg.tx_buf = pData;
    msg.rx_buf = &SPI_TMPBUFF[0];
    msg.len = Length;
    msg.speed_hz = 5000000;
    msg.bits_per_word = 8;

    bret =  ioctl(ghSPI[moduleidx], SPI_IOC_MESSAGE(1), &msg);
    if( (unsigned int)bret != Length )
    {
        PRINTF_LV_0("[SPI.c] Error SpiWrite1(size:%d, ret:%d)\n", Length, bret);
      	return -1;
    }
    return bret;
}

int SPIReadString (int moduleidx, unsigned char * pBufIn, unsigned char * pBufOut, unsigned char Length)
{
    int bret;
    struct spi_ioc_transfer msg;

    bret = 0;
    TC3X_IO_memset(&msg, 0x00, sizeof(msg));
    msg.tx_buf = pBufIn;
    msg.rx_buf = pBufOut;
    msg.len = Length;
    msg.speed_hz = 5000000;
    msg.bits_per_word = 8;

    bret =  ioctl(ghSPI[moduleidx], SPI_IOC_MESSAGE(1), &msg);
    if( (unsigned int)bret != Length )
    {
        PRINTF_LV_0("[SPI.c] Error SpiWrite1(size:%d, ret:%d)\n", Length, bret);
      	return -1;
    }
    return bret;
}

TC3X_IO_Err TC3X_IO_CSPI_Init (int moduleidx)
{
    // command : 0 5(opt.) 52 (1byte) 53 (4bytes)
    SetupSPI (moduleidx);
    TC3X_IO_PW_ON (moduleidx);
    TC3X_IO_PW_RESET (moduleidx);
    return TC3X_IO_SUCCESS;
}

//=======================================================================================
//
//  Read / Write API
//
//=======================================================================================

unsigned int TC3X_IO_CSPI_Reg_Read (int moduleidx, int ChipAddr, int RegAddr, TC3X_IO_Err * pError)     // use 52cmd
{
    unsigned int retv;

    TC3X_IO_IF_LOCK (moduleidx);
    TC3X_IO_CSPI_CMD52 (moduleidx, 0, RegAddr, (unsigned char *) &retv);
    retv = (retv & 0xff);
    TC3X_IO_IF_UnLOCK (moduleidx);
    return retv;
}

TC3X_IO_Err TC3X_IO_CSPI_Reg_Write (int moduleidx, int ChipAddr, int RegAddr, unsigned int data)        // use 52cmd
{
    unsigned char inputdata;

    TC3X_IO_IF_LOCK (moduleidx);
    inputdata = (data & 0xff);
    TC3X_IO_CSPI_CMD52 (moduleidx, 1, RegAddr, &inputdata);
    TC3X_IO_IF_UnLOCK (moduleidx);
    return TC3X_IO_SUCCESS;
}

TC3X_IO_Err TC3X_IO_CSPI_Reg_ReadEx (int moduleidx, int ChipAddr, int RegAddr, unsigned char *data, int iSize)  // use 53cmd
{
    unsigned int c512;
    unsigned int cremain;
    unsigned int i;
    int       result;

    c512 = (iSize / 512);
    cremain = (iSize % 512);

    TC3X_IO_IF_LOCK (moduleidx);

    for (i = 0; i < c512; i++)
    {
        result = TC3X_IO_CSPI_CMD53 (moduleidx, 0, RegAddr, (unsigned char *) &data[i * 512], 512, 1);

        if (result == (-1))
        {
            TC3X_IO_CSPI_CMD53 (moduleidx, 0, RegAddr, (unsigned char *) &data[i * 512], 512, 1);
        }
    }

    if (cremain)
    {
        result = TC3X_IO_CSPI_CMD53 (moduleidx, 0, RegAddr, (unsigned char *) &data[i * 512], cremain, 1);
        if (result == (-1))
        {
            result = TC3X_IO_CSPI_CMD53 (moduleidx, 0, RegAddr, (unsigned char *) &data[i * 512], cremain, 1);
        }
    }

    TC3X_IO_IF_UnLOCK (moduleidx);
    return TC3X_IO_SUCCESS;
}

TC3X_IO_Err TC3X_IO_CSPI_Reg_WriteEx (int moduleidx, int ChipAddr, int RegAddr, unsigned char *pCh, int iSize)  // use 53cmd
{
    unsigned int c512;
    unsigned int cremain;
    unsigned int i;
    int       result;

    c512 = (iSize / 512);
    cremain = (iSize % 512);

    TC3X_IO_IF_LOCK (moduleidx);

    for (i = 0; i < c512; i++)
    {
        result = TC3X_IO_CSPI_CMD53 (moduleidx, 1, RegAddr, (unsigned char *) &pCh[i * 512], 512, 1);

        if (result == (-1))
        {
            result = TC3X_IO_CSPI_CMD53 (moduleidx, 1, RegAddr, (unsigned char *) &pCh[i * 512], 512, 1);
        }
    }

    if (cremain)
    {
        result = TC3X_IO_CSPI_CMD53 (moduleidx, 1, RegAddr, (unsigned char *) &pCh[i * 512], cremain, 1);
        if (result == (-1))
        {
            result = TC3X_IO_CSPI_CMD53 (moduleidx, 1, RegAddr, (unsigned char *) &pCh[i * 512], cremain, 1);
        }
    }

    TC3X_IO_IF_UnLOCK (moduleidx);
    return TC3X_IO_SUCCESS;
}

// not use semaphore

unsigned int TC3X_IO_CSPI_Reg_Read_InnerSem (int moduleidx, int ChipAddr, int RegAddr, TC3X_IO_Err * pError)
{
    unsigned int retv;
    TC3X_IO_CSPI_CMD52 (moduleidx, 0, RegAddr, (unsigned char *) &retv);
    retv = (retv & 0xff);
    return retv;
}

TC3X_IO_Err TC3X_IO_CSPI_Reg_Write_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned int data)
{
    unsigned char inputdata;
    inputdata = (data & 0xff);
    TC3X_IO_CSPI_CMD52 (moduleidx, 1, RegAddr, &inputdata);
    return TC3X_IO_SUCCESS;
}

TC3X_IO_Err TC3X_IO_CSPI_Reg_ReadEx_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned char *data, int iSize)
{
    unsigned int c512;
    unsigned int cremain;
    unsigned int i;
    int       result;

    c512 = (iSize / 512);
    cremain = (iSize % 512);

    for (i = 0; i < c512; i++)
    {
        result = TC3X_IO_CSPI_CMD53 (moduleidx, 0, RegAddr, (unsigned char *) &data[i * 512], 512, 1);

        if (result == (-1))
        {
            result = TC3X_IO_CSPI_CMD53 (moduleidx, 0, RegAddr, (unsigned char *) &data[i * 512], 512, 1);
        }
    }

    if (cremain)
    {
        result = TC3X_IO_CSPI_CMD53 (moduleidx, 0, RegAddr, (unsigned char *) &data[i * 512], cremain, 1);
        if (result == (-1))
        {
            result = TC3X_IO_CSPI_CMD53 (moduleidx, 0, RegAddr, (unsigned char *) &data[i * 512], cremain, 1);
        }
    }
    return TC3X_IO_SUCCESS;
}

TC3X_IO_Err TC3X_IO_CSPI_Reg_WriteEx_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned char *pCh, int iSize)
{
    unsigned int c512;
    unsigned int cremain;
    unsigned int i;
    int       result;

    c512 = (iSize / 512);
    cremain = (iSize % 512);

    for (i = 0; i < c512; i++)
    {
        result = TC3X_IO_CSPI_CMD53 (moduleidx, 1, RegAddr, (unsigned char *) &pCh[i * 512], 512, 1);

        if (result == (-1))
        {
            result = TC3X_IO_CSPI_CMD53 (moduleidx, 1, RegAddr, (unsigned char *) &pCh[i * 512], 512, 1);
        }
    }

    if (cremain)
    {
        result = TC3X_IO_CSPI_CMD53 (moduleidx, 1, RegAddr, (unsigned char *) &pCh[i * 512], cremain, 1);
        if (result == (-1))
        {
            result = TC3X_IO_CSPI_CMD53 (moduleidx, 1, RegAddr, (unsigned char *) &pCh[i * 512], cremain, 1);
        }
    }

    return TC3X_IO_SUCCESS;
}

//=======================================================================================
//
//  SDIO - SPI
//
//=======================================================================================

// R5
int TC3X_IO_CSPI_CMD52 (int moduleidx, unsigned char write_flag,        // write flag  0: read   1: write
                        unsigned short reg_addr,           // SDIO register address
                        unsigned char *buf)                // data buffer to be read and write
{
    unsigned char buffer[10 + 2];                          // +n dummy for align
    unsigned char crc;
    unsigned char tem_buf[12];

    buffer[0] = 0x74;   // start bit & direction & CMD52
    buffer[1] = ((write_flag & 0x01) << 7) | 0x10 | 0x08 | 0x00 | ((reg_addr >> 15) & 0x03);    // function number = 1, RAW = 1, Stuff = 0 //
    buffer[2] = (reg_addr >> 7) & 0xFF;
    buffer[3] = (reg_addr << 1) & 0xFE; // Stuff = 0//

    if (write_flag)     // write //
        buffer[4] = buf[0];
    else
        buffer[4] = 0x00;       /* read. stuff 0 */

    crc = TC3X_IO_UTIL_CRC7 (buffer, 5);
    buffer[5] = (crc << 1) | 0x01;      //crc + end bit(1)
    buffer[6] = 0xff;   // wait clk

    buffer[7] = 0xff;   // r5 - spi (status)
    buffer[8] = 0xff;   // r5 - spi (data)

    buffer[9] = 0xff;   // end

    buffer[10] = 0xff;  // dummy
    buffer[11] = 0xff;  // dummy

    SPIReadString (moduleidx, buffer, tem_buf, 12);

    if (tem_buf[7] & 0x08)
    {
        PRINTF_LV_0 ("52 CRC error[%d]///", write_flag);
        PRINTF_LV_0 ("[%x][%x][%x][%x][%x][%x][%x][%x][%x]\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4],
                buffer[5], buffer[6], tem_buf[7], tem_buf[8]);
        return (-1);
    }

    if (!write_flag)
        buf[0] = tem_buf[8];

    return 0;
}

// multibyte access // R5
int TC3X_IO_CSPI_CMD53 (int moduleidx, unsigned char write_flag,        // write flag  0: read   1: write
                        unsigned int reg_addr,             // SDIO register address
                        unsigned char *buf,                // data buffer to be read and write
                        unsigned short len, int fixedmode)
{
    unsigned char buffer[512 + 14 + 12 + 12];              // +n dummy for align
    unsigned char crc;
    int       cpoint;
    unsigned short tempcrc;
    unsigned char tem_buf[15];
    unsigned char CRC16[2];

    if (len > 512)      // MAX read size = 512 byte //
        return (-1);

    // Read
    // Command(6) + Wait(1) + R5(2) + Start_token(1) + Data(N) + CRC16(2) + Response(1) + End_wait(1) = N+14 bytes
    if (write_flag == 0)
    {
        buffer[0] = 0xff;       // dummy
        buffer[1] = 0xff;       // dummy
        buffer[2] = 0x75;       // start bit & direction & CMD53

        if (fixedmode)
        {
            buffer[3] = ((write_flag & 0x01) << 7) | 0x10 | 0x00 | 0x00 | ((reg_addr >> 15) & 0x03);
        }
        else
        {
            buffer[3] = ((write_flag & 0x01) << 7) | 0x10 | 0x00 | 0x04 | ((reg_addr >> 15) & 0x03);
        }

        buffer[4] = (reg_addr >> 7) & 0xFF;
        buffer[5] = ((reg_addr << 1) & 0xFE) | ((len >> 8) & 0x01);
        buffer[6] = (len & 0xff);
        crc = TC3X_IO_UTIL_CRC7 (&buffer[2], 5);
        buffer[7] = (crc << 1) | 0x01;  //crc + end bit(1)

        buffer[8] = 0xff;       // wait
        buffer[9] = 0xff;       // R5 (stat)
        buffer[10] = 0xff;      // R5 (RW Data-Stuff 0)

        buffer[11] = 0xff;      // start token

        cpoint = 0;
        TC3X_IO_memset (&buffer[11], 0xff, len + 24);
        SPIReadString (moduleidx, &buffer[cpoint], tem_buf, 12);
        cpoint += 12;

        if (tem_buf[9] & 0x08)
        {
            PRINTF_LV_0 ("CMD53 Read CRC error[%x][%d][%d]\n", tem_buf[9], reg_addr, len);
            SPIWriteString (moduleidx, &buffer[11], 16);        /* SPI CMD initial */
            return (-1);
        }

        if (tem_buf[11] != 0xfe)
        {
            PRINTF_LV_0 ("CMD53 token error[%x][%x]\n", buffer[9], tem_buf[2]);
            return (-1);
        }

        // Data(N)
        SPIReadString (moduleidx, &buffer[cpoint], buf, len);
        cpoint += len;

        // CRC16(2) + Response(1) + End_wait(1) + dummy(2. for WORD align)
        SPIReadString (moduleidx, &buffer[cpoint], tem_buf, 12);
        cpoint += 4;
    }
    else
    {
        buffer[0] = 0x75;       // start bit & direction & CMD53

        if (fixedmode)
        {
            buffer[1] = ((write_flag & 0x01) << 7) | 0x10 | 0x00 | 0x00 | ((reg_addr >> 15) & 0x03);
        }
        else
        {
            buffer[1] = ((write_flag & 0x01) << 7) | 0x10 | 0x00 | 0x04 | ((reg_addr >> 15) & 0x03);
        }

        buffer[2] = (reg_addr >> 7) & 0xFF;
        buffer[3] = ((reg_addr << 1) & 0xFE) | ((len >> 8) & 0x01);
        buffer[4] = (len & 0xff);
        crc = TC3X_IO_UTIL_CRC7 (buffer, 5);
        buffer[5] = (crc << 1) | 0x01;  //crc + end bit(1)

        buffer[6] = 0xff;       // wait
        buffer[7] = 0xff;       // R5 (stat)
        buffer[8] = 0xff;       // R5 (RW Data-Stuff 0)

        // Write
        // Command(6) + Wait(1) + R5(2) + Start_token(1) + Data(N) + CRC16(2) + Response(1) + End_wait(1) = N+14 bytes
        cpoint = 0;
        TC3X_IO_memset (&buffer[9], 0xff, len + 6);

        buffer[10] = 0xfe;

        // command (7)
        SPIWriteString (moduleidx, buffer, 7);
        cpoint += 7;

        // wait (1) + R5 (2) + Start_token(1)
        SPIReadString (moduleidx, &buffer[cpoint], tem_buf, 4);
        cpoint += 4;

        if (tem_buf[0] & 0x08)
        {
            PRINTF_LV_0 ("CMD53 CRC error///");
            PRINTF_LV_0 ("[%x][%x][%x][%x][%x][%x][%x][%x][%x][%x][%x]/[%x][%x]\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4],
                    buffer[5], buffer[6], buffer[7], buffer[8], buffer[9], buffer[10], tem_buf[0], tem_buf[1]);

            SPIWriteString (moduleidx, &buffer[11], 16);        /* SPI CMD initial */
            return (-1);
        }

        // Data(N)
        SPIWriteString (moduleidx, buf, len);
        cpoint += len;

        // CRC16(2) 
        tempcrc = TC3X_IO_UTIL_CRC16 (buf, 0, len);
        CRC16[0] = ((tempcrc >> 8) & 0x0ff);
        CRC16[1] = (tempcrc & 0x0ff);

        SPIWriteString (moduleidx, CRC16, 2);
        cpoint += 2;

        // Response(1) + End_wait(1)
        SPIReadString (moduleidx, &buffer[cpoint], tem_buf, 2);

        if ((tem_buf[0] & 0x1f) != 0x05)
        {
            PRINTF_LV_0 ("CMD53 Write Err [0x%x]\n", tem_buf[0]);
            return (-1);
        }

        cpoint += 2;
    }
    return 0;
}

void TC3X_IO_CSPI_Reg_Read_Wrap(int iHandleNum, unsigned int Addr, unsigned char *data, unsigned char bytesize)
{
    int i;
	
    iHandleNum = 0;

	for(i=0; i<bytesize; i++)
	{
		data[i] = TC3X_IO_CSPI_Reg_Read (iHandleNum, 0x00, Addr+i, NULL);
	}
}

void TC3X_IO_CSPI_Reg_Write_Wrap(int iHandleNum, unsigned int Addr, unsigned char data)
{
    iHandleNum = 0;
    TC3X_IO_CSPI_Reg_Write (iHandleNum, 0x00, Addr, data);
}

