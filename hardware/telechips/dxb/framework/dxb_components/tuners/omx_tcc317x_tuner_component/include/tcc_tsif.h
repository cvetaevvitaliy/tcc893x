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

#ifndef __TCC_TSIF_H__
#define __TCC_TSIF_H__

/* Select GPSB channel */
#define TSIF_SPI_NUM 1

#define TSIF_DEV_IOCTL 252
//#define TSIF_DEV_NAME "tcc-tsif"
#define TSIF_DEV_NAME "tcc-tsif0"

#define TSIF_PACKET_SIZE 188
#define PID_MATCH_TABLE_MAX_CNT 32
#define MAX_MESSAGE_CNT 256
#define TSIF_PACKETSIZE	(188)
#define TSIF_MAX_PACKETCOUNT (8190)
//#define TSIF_MAX_PACKETCOUNT (5577)


struct tcc_tsif_param 
{
    unsigned int ts_total_packet_cnt;
    unsigned int ts_intr_packet_cnt;
    unsigned int mode;
	unsigned int dma_mode;  // DMACTR[MD]: DMA mode register
};

struct tcc_tsif_pid_param {
    unsigned int pid_data[PID_MATCH_TABLE_MAX_CNT];
    unsigned int valid_data_cnt;
};

struct tcc_tsif_pcr_param {
	unsigned int pcr_pid;
	unsigned int index;
};


/***************************************************************************
* SPI
***************************************************************************/
#define DMA_NORMAL_MODE     0x00
#define DMA_MPEG2TS_MODE    0x01

#define SPI_CPHA			0x01
#define SPI_CPOL			0x02
#define SPI_MODE_0			(0|0)
#define SPI_MODE_1			(0|SPI_CPHA)
#define SPI_MODE_2			(SPI_CPOL|0)
#define SPI_MODE_3			(SPI_CPOL|SPI_CPHA)
#define SPI_CS_HIGH			0x04
#define SPI_LSB_FIRST		0x08
#define SPI_3WIRE			0x10
#define SPI_LOOP			0x20
#define SPI_0X00			0x00

#define IOCTL_TSIF_DMA_START        _IO(TSIF_DEV_IOCTL, 1)
#define IOCTL_TSIF_DMA_STOP         _IO(TSIF_DEV_IOCTL, 2)
#define IOCTL_TSIF_GET_MAX_DMA_SIZE _IO(TSIF_DEV_IOCTL, 3)
#define IOCTL_TSIF_SET_PID          _IO(TSIF_DEV_IOCTL, 4)
#define IOCTL_TSIF_DXB_POWER		_IO(TSIF_DEV_IOCTL, 5)
#define IOCTL_TSIF_SET_PCRPID		_IO(TSIF_DEV_IOCTL, 6)
#define IOCTL_TSIF_GET_STC			_IO(TSIF_DEV_IOCTL, 7)
#define IOCTL_TSIF_RESET			_IO(TSIF_DEV_IOCTL, 8)
#define IOCTL_TSIF_ADD_PID          _IO(TSIF_DEV_IOCTL, 9)
#define IOCTL_TSIF_REMOVE_PID       _IO(TSIF_DEV_IOCTL, 10)
#define IOCTL_TSIF_HWDMX_START      _IO(TSIF_DEV_IOCTL, 11)
#define IOCTL_TSIF_HWDMX_STOP       _IO(TSIF_DEV_IOCTL, 12)

#endif /*__TCC_TSIF_H__*/
