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
#define LOG_TAG	"TCC_TSIF"
#include <utils/Log.h>

#include "user_debug_levels.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <errno.h>

	

#include "tcc_tsif.h"		
//#define DEV_NAME "/dev/tcc-tsif"
#define DEV_NAME "/dev/tcc-tsif0"

#ifndef MAX_PID_TABLE_CNT
#define MAX_PID_TABLE_CNT	32
#endif
	
static struct tcc_tsif_param param;
static struct tcc_tsif_pid_param pid_param;
static struct tcc_tsif_pcr_param pcr_param;
static struct pollfd poll_evt;
static unsigned int guiFirstReadFlg = 0;

void TCC_TSIF_ShowPIDTable(void)
{
#if 1	
	int	i;
	DEBUG(DEFAULT_MESSAGES,"\n");
	for ( i = 0 ; i < pid_param.valid_data_cnt ; i++ ) 
	{
		DEBUG(DEFAULT_MESSAGES,"[%2d] : [0x%04x]\n", i, pid_param.pid_data[i]);
	}
	DEBUG(DEFAULT_MESSAGES,"PID TABLE ENTRY (%d ENTRIES).\n", pid_param.valid_data_cnt);
#endif	
}


/*!!!CAUTION, packet_intr_cnt is not divided count.
*             it is interrupt count.
*/
int	TCC_TSIF_Open(int packet_intr_cnt, int packet_dma_cnt ,int mode, int dma_mode)	
{
	int	ret;
	int	handle;
	unsigned int		i;

	param.ts_total_packet_cnt = packet_dma_cnt;
	param.ts_intr_packet_cnt = packet_intr_cnt;

	if(dma_mode == 0)
		param.dma_mode = DMA_NORMAL_MODE; 
	else
		param.dma_mode = DMA_MPEG2TS_MODE;	
		

	/***************************************************************************
	* spi support mode
	* SPI_MODE_0, SPI_MODE_1, SPI_MODE_2, SPI_MODE_3, SPI_CS_HIGH, SPI_LSB_FIRST
	#define SPI_CPHA		0x01
	#define SPI_CPOL		0x02
	#define SPI_MODE_0		(0|0)
	#define SPI_MODE_1		(0|SPI_CPHA)
	#define SPI_MODE_2		(SPI_CPOL|0)
	#define SPI_MODE_3		(SPI_CPOL|SPI_CPHA)
	#define SPI_CS_HIGH		0x04
	#define SPI_LSB_FIRST		0x08
	#define SPI_3WIRE		0x10
	#define SPI_LOOP		0x20
	***************************************************************************/
	switch(mode)
	{
		case 0:
			param.mode = 0x02;		//SPI_MODE_2;		//sharp(ISDBT)
		break;
		case 1:		
			param.mode = 0x04;		//SPI_CS_HIGH;		//dibcom(DVBT)
		break;	
		case 2:		
			param.mode = 0x01;
		break;
		case 3:
			param.mode = 0x00;
		break;
	}
	handle = open(DEV_NAME, O_RDWR | O_NDELAY);
	if(handle<0)
	{
        ALOGE("%s:%d[%d:%s]",__func__, __LINE__, errno, strerror(errno));
		return -1;
	}

	/******************************************************************
	* if "pid_param.valid_data_cnt == 0" => "PID filtering & sync byte  =  off"
	*******************************************************************/
	for(i=0;i<MAX_PID_TABLE_CNT; i++)
	{
		pid_param.pid_data[i] = 0;		//PAT_PID:0 
	}
	pid_param.valid_data_cnt = 0;
	return handle;

}

int TCC_TSIF_DxBPower(int handle, int on_off)
{
	if( 0 !=ioctl(handle, IOCTL_TSIF_DXB_POWER , &on_off))
	{
		DEBUG(DEB_LEV_ERR,"DXB Power Off Error\n");
	}
	return 0;
}

int	TCC_TSIF_ClearPIDTable(int handle, int PID)	
{
	int err;

	if((err = ioctl(handle, IOCTL_TSIF_DMA_STOP, &param))!=-1)
	{
		pid_param.valid_data_cnt =0 ;
		pid_param.pid_data[pid_param.valid_data_cnt] = PID;
		if (ioctl(handle, IOCTL_TSIF_SET_PID, &pid_param) != -1) 
		{
			if (ioctl(handle, IOCTL_TSIF_DMA_START, &param) != -1) 
			{
				TCC_TSIF_ShowPIDTable();
				return pid_param.valid_data_cnt;
			}
			else
			{
				DEBUG(DEB_LEV_ERR,"error IOCTL_TSIF_DMA_START ioctl !!!\n");
				DEBUG(DEB_LEV_ERR,"PID setting error\n");
				return 0;
			}
		}
		else
		{
			DEBUG(DEB_LEV_ERR,"error IOCTL_TSIF_SET_PID ioctl !!!\n");
			DEBUG(DEB_LEV_ERR,"PID setting error\n");
			return 0;
		}
	}
	else
	{
		DEBUG(DEB_LEV_ERR,"error IOCTL_TSIF_DMA_STOP ioctl !!!\n");
		DEBUG(DEB_LEV_ERR," error = %x\n", err);
		return 0;
	}
}
	
int	TCC_TSIF_SetPID(int handle, int PID)	
{
	int err, i;
	if(pid_param.valid_data_cnt>(MAX_PID_TABLE_CNT-1))
	{
		DEBUG(DEB_LEV_ERR,"check pid_param.valid_data_cnt = %d \n", pid_param.valid_data_cnt);
		DEBUG(DEB_LEV_ERR,"MAX pid_param.valid_data_cnt = 32 \n");
		DEBUG(DEB_LEV_ERR,"PID setting error\n");
		return 0;
	}

	for( i=0; i<pid_param.valid_data_cnt; i++)
	{
		if( pid_param.pid_data[i] == PID )
		{
			DEBUG(DEB_LEV_ERR,"We already have 0x%X PID.\n", PID);
			return 0;
		}
	}

	if((err = ioctl(handle, IOCTL_TSIF_DMA_STOP, &param))!=-1)
	{
		pid_param.pid_data[pid_param.valid_data_cnt] = PID;
		pid_param.valid_data_cnt ++;

		if (ioctl(handle, IOCTL_TSIF_SET_PID, &pid_param) != -1) 
		{
			if (ioctl(handle, IOCTL_TSIF_DMA_START, &param) != -1) 
			{
				TCC_TSIF_ShowPIDTable();
				return pid_param.valid_data_cnt;
			}
			else
			{
				DEBUG(DEB_LEV_ERR,"error IOCTL_TSIF_DMA_START ioctl !!!\n");
				DEBUG(DEB_LEV_ERR,"PID setting error\n");
				return 0;
			}
		}
		else
		{
			DEBUG(DEB_LEV_ERR,"error IOCTL_TSIF_SET_PID ioctl !!!\n");
			DEBUG(DEB_LEV_ERR,"PID setting error\n");
			return 0;
		}
	}
	else
	{
		DEBUG(DEB_LEV_ERR,"error IOCTL_TSIF_DMA_STOP ioctl !!!\n");
		DEBUG(DEB_LEV_ERR,"error = %x\n", err);
		return 0;
	}
}

int	TCC_TSIF_RemovePID(int handle, int PID)	
{
	int err, i;
	if( pid_param.valid_data_cnt < 1)
		return -1;
	
	if((err = ioctl(handle, IOCTL_TSIF_DMA_STOP, &param))!=-1)
	{
		for(i=0; i<pid_param.valid_data_cnt; i++)
		{
			if(pid_param.pid_data[i] == PID)
			{
				break;
			}
		}
		if( i == pid_param.valid_data_cnt )
			return -1; //cannot find pid
		
		for(;i<pid_param.valid_data_cnt; i++)
			pid_param.pid_data[i] = pid_param.pid_data[i+1];		

		pid_param.pid_data[i] = 0;
		pid_param.valid_data_cnt--;

		if (ioctl(handle, IOCTL_TSIF_SET_PID, &pid_param) != -1) 
		{
			if (ioctl(handle, IOCTL_TSIF_DMA_START, &param) != -1) 
			{
				TCC_TSIF_ShowPIDTable();
				return pid_param.valid_data_cnt;
			}
			else
			{
				DEBUG(DEB_LEV_ERR,"error IOCTL_TSIF_DMA_START ioctl !!!\n");
				DEBUG(DEB_LEV_ERR,"PID setting error\n");
				return 0;
			}
		}
		else
		{
			DEBUG(DEB_LEV_ERR,"error IOCTL_TSIF_SET_PID ioctl !!!\n");
			DEBUG(DEB_LEV_ERR,"PID setting error\n");
			return 0;
		}
	}
	else
	{
		DEBUG(DEB_LEV_ERR,"error IOCTL_TSIF_DMA_STOP ioctl !!!\n");
		DEBUG(DEB_LEV_ERR,"error = %x\n", err);
		return 0;
	}
}


int TCC_TSIF_Close(int handle) 
{
	pid_param.valid_data_cnt = 0;
	guiFirstReadFlg=0;
	close(handle);
	return 0;
}

int	TCC_TSIF_Read(int handle, unsigned char *buf, unsigned int read_size)	
{
	int	ret;
	int	cnt;


	if(!guiFirstReadFlg)
	{
		if(pid_param.valid_data_cnt>0)
		{
			if(param.dma_mode == DMA_MPEG2TS_MODE)
			{
				if (ioctl(handle, IOCTL_TSIF_SET_PID, &pid_param) ==-1) 
				{
					DEBUG(DEB_LEV_ERR,"error IOCTL_TSIF_SET_PID ioctl !!!\n");
					DEBUG(DEB_LEV_ERR,"PID setting error\n");
					return 0;
				}
			}
		}
			
		ret = ioctl(handle, IOCTL_TSIF_DMA_START, &param);
		guiFirstReadFlg =1;

		if(ret<0)
		{
			DEBUG(DEB_LEV_ERR,"spi open error\n");
			return 0;
		}
	}
	
	memset(&poll_evt, 0, sizeof(poll_evt));
	poll_evt.fd = handle;
	poll_evt.events = POLLIN;
	ret = poll(&poll_evt, 1u, 1000);
	if (ret > 0)
	{
		if (poll_evt.revents & POLLERR)
		{
			DEBUG(DEB_LEV_ERR,"poll POLLERR\n");
			ret = 0;
		} 
		else if (poll_evt.revents & POLLIN)
		{
			ret = read(handle, buf, read_size);

			return ret;
		}
	}
	return ret;	
}



int	TCC_TSIF_ReadEX(int handle, unsigned char buf, unsigned int read_size)	
{
	int	ret;
	int	cnt;


	if(!guiFirstReadFlg)
	{
		if(pid_param.valid_data_cnt>0)
		{
			if(param.dma_mode = DMA_MPEG2TS_MODE)
			{
				if (ioctl(handle, IOCTL_TSIF_SET_PID, &pid_param) ==-1) 
				{
					DEBUG(DEB_LEV_ERR,"error IOCTL_TSIF_SET_PID ioctl !!!\n");
					DEBUG(DEB_LEV_ERR,"PID setting error\n");
					return 0;
				}
			}
		}
			
		ret = ioctl(handle, IOCTL_TSIF_DMA_START, &param);
		guiFirstReadFlg =1;

		if(ret<0)
		{
			DEBUG(DEB_LEV_ERR,"spi open error\n");
			return 0;
		}
	}

	ret = read(handle, buf, read_size);
	return ret;

}

int	TCC_TSIF_SetPCRPID(int handle, unsigned int uiPID, unsigned int index)
{
	int ret;

	pcr_param.index = index;
	pcr_param.pcr_pid = uiPID;
	ret = ioctl(handle, IOCTL_TSIF_SET_PCRPID, &pcr_param);
	return ret;
}

int	TCC_TSIF_GetSTC(int handle, unsigned int index)
{
	int ret;
	unsigned int uiSTC;

	uiSTC = index;
	ret = ioctl(handle, IOCTL_TSIF_GET_STC, &uiSTC);
	return uiSTC;
}

void TCC_TSIF_ResetRead(void)
{
//	guiFirstReadFlg = 0;
}

