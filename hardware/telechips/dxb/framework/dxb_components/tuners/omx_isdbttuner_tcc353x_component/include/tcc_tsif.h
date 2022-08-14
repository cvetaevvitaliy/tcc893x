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

#define DMA_NORMAL_MODE     0x00
#define DMA_MPEG2TS_MODE    0x01

#define IOCTL_TSIF_DMA_START        _IO(TSIF_DEV_IOCTL, 1)
#define IOCTL_TSIF_DMA_STOP         _IO(TSIF_DEV_IOCTL, 2)
#define IOCTL_TSIF_GET_MAX_DMA_SIZE _IO(TSIF_DEV_IOCTL, 3)
#define IOCTL_TSIF_SET_PID          _IO(TSIF_DEV_IOCTL, 4)
#define IOCTL_TSIF_DXB_POWER        _IO(TSIF_DEV_IOCTL, 5)
#define IOCTL_TSIF_SET_PCRPID		_IO(TSIF_DEV_IOCTL, 6)
#define IOCTL_TSIF_GET_STC			_IO(TSIF_DEV_IOCTL, 7)
#define IOCTL_TSIF_RESET			_IO(TSIF_DEV_IOCTL, 8)

#endif /*__TCC_TSIF_H__*/
