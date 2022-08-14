/*
 * include/linux/spi/tcc_tsif.h
 *
 * Author:  <linux@telechips.com>
 * Created: 1st April, 2009
 * Description: Driver for Telechips SPI (GPSB) Controllers
 *              SPI slave mode for DXB TSIF (Transport Stream Interface) modules
 *
 * Copyright (c) Telechips, Inc.
 *
 */
#ifndef __TCC_TSIF_H__
#define __TCC_TSIF_H__

/* Select GPSB channel */
#define TSIF_SPI_NUM 1

#define TSIF_DEV_IOCTL 252
#define TSIF_DEV_NAME "tcc-tsif"

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
#define IOCTL_TSIF_DXB_POWER   _IO(TSIF_DEV_IOCTL, 5)
#define IOCTL_TSIF_SET_PCRPID		_IO(TSIF_DEV_IOCTL, 6)
#define IOCTL_TSIF_GET_STC			_IO(TSIF_DEV_IOCTL, 7)

int	TCC_TSIF_Open(int packet_intr_cnt, int packet_dma_cnt ,int mode, int dma_mode);
int TCC_TSIF_DxBPower(int handle, int on_off);
int	TCC_TSIF_SetPID(int handle, int PID);
int	TCC_TSIF_RemovePID(int handle, int PID);
int TCC_TSIF_Close(int handle);
int	TCC_TSIF_Read(int handle, unsigned char *buf, unsigned int read_size);
int	TCC_TSIF_ReadEX(int handle, unsigned char *buf, unsigned int read_size);	
int	TCC_TSIF_SetPCRPID(int handle, unsigned int uiPID, unsigned int index);
int	TCC_TSIF_GetSTC(int handle, unsigned int index);

#endif /*__TCC_TSIF_H__*/
