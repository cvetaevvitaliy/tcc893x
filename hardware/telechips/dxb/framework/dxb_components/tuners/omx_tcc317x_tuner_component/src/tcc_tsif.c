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
#define DEV_NAME "/dev/tcc-tsif"
#define DEV_NAME_0 "/dev/tcc-tsif0"
#define DEV_NAME_1 "/dev/tcc-tsif1"

#ifndef MAX_PID_TABLE_CNT
#define MAX_PID_TABLE_CNT	32
#endif
	
static struct tcc_tsif_param param;
static struct tcc_tsif_pid_param pid_param;
static struct tcc_tsif_pcr_param pcr_param;
static struct pollfd poll_evt;
static unsigned int guiFirstReadFlg[2] = { 0, 0 };

void TCC_TSIF_ShowPIDTable(void)
{
	int	i;	
	
	for ( i = 0 ; i < pid_param.valid_data_cnt ; i++ ) 
	{
		DEBUG(DEB_LEV_ERR,"[     TSIF     ] [%2d] : [0x%04x]\n", i, pid_param.pid_data[i]);
	}
	DEBUG(DEB_LEV_ERR,"[     TSIF     ] PID TABLE ENTRY (%d ENTRIES).\n", pid_param.valid_data_cnt);
}


int TCC_TSIF_Open(int moduleidx, int packet_intr_cnt, int packet_dma_cnt ,int mode, int dma_mode)	
{
	int ret, handle;	
	unsigned int i;	

	param.ts_total_packet_cnt = packet_dma_cnt;
	param.ts_intr_packet_cnt = packet_intr_cnt;

	if(dma_mode == 0)
		param.dma_mode = DMA_NORMAL_MODE; 
	else
		param.dma_mode = DMA_MPEG2TS_MODE;	


	switch(mode)
	{
		case 0:
			param.mode = SPI_CPOL;
			break;
		case 1:		
			param.mode = SPI_CS_HIGH;
			break;	
		case 2:		
			param.mode = SPI_CPHA;
			break;
		case 3:
			param.mode = SPI_0X00;
			break;
	}
	switch(moduleidx)
	{
		case 0:
		default:
			handle = open(DEV_NAME_0, O_RDWR | O_NDELAY);
			break;

		case 1:
			handle = open(DEV_NAME_1, O_RDWR | O_NDELAY);
			break;
	}

	if(handle<0)
	{
		DEBUG(DEB_LEV_ERR,"[     TSIF     ] [%d] TCC_TSIF_Open Error :[%d:%s]", moduleidx ,errno, strerror(errno));
		return -1;
	}else
		DEBUG(DEB_LEV_ERR,"[     TSIF     ] [%d] TCC TSIF Open Success .. \n",moduleidx);

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
		DEBUG(DEB_LEV_ERR,"[     TSIF     ] DXB Power Off Error\n");
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
				DEBUG(DEB_LEV_ERR,"[     TSIF     ] error IOCTL_TSIF_DMA_START ioctl !!!\n");
				DEBUG(DEB_LEV_ERR,"[     TSIF     ] PID setting error\n");
				return 0;
			}
		}
		else
		{
			DEBUG(DEB_LEV_ERR,"[     TSIF     ] error IOCTL_TSIF_SET_PID ioctl !!!\n");
			DEBUG(DEB_LEV_ERR,"[     TSIF     ] PID setting error\n");
			return 0;
		}
	}
	else
	{
		DEBUG(DEB_LEV_ERR,"[     TSIF     ] error IOCTL_TSIF_DMA_STOP ioctl !!!\n");
		DEBUG(DEB_LEV_ERR,"[     TSIF     ] error = %x\n", err);
		return 0;
	}
}
	
int	TCC_TSIF_SetPID(int handle, int PID)	
{
	int err, i;
	if(pid_param.valid_data_cnt>(MAX_PID_TABLE_CNT-1))
	{
		DEBUG(DEB_LEV_ERR,"[     TSIF     ] check pid_param.valid_data_cnt = %d \n", pid_param.valid_data_cnt);
		DEBUG(DEB_LEV_ERR,"[     TSIF     ] MAX pid_param.valid_data_cnt = 32 \n");
		DEBUG(DEB_LEV_ERR,"[     TSIF     ] PID setting error\n");
		return 0;
	}

	for( i=0; i<pid_param.valid_data_cnt; i++)
	{
		if( pid_param.pid_data[i] == PID )
		{
			DEBUG(DEB_LEV_ERR,"[     TSIF     ] We already have 0x%X PID.\n", PID);
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
				DEBUG(DEB_LEV_ERR,"[     TSIF     ] error IOCTL_TSIF_DMA_START ioctl !!!\n");
				DEBUG(DEB_LEV_ERR,"[     TSIF     ] PID setting error\n");
				return 0;
			}
		}
		else
		{
			DEBUG(DEB_LEV_ERR,"[     TSIF     ] error IOCTL_TSIF_SET_PID ioctl !!!\n");
			DEBUG(DEB_LEV_ERR,"[     TSIF     ] PID setting error\n");
			return 0;
		}
	}
	else
	{
		DEBUG(DEB_LEV_ERR,"[     TSIF     ] error IOCTL_TSIF_DMA_STOP ioctl !!!\n");
		DEBUG(DEB_LEV_ERR,"[     TSIF     ] error = %x\n", err);
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
				DEBUG(DEB_LEV_ERR,"[     TSIF     ] error IOCTL_TSIF_DMA_START ioctl !!!\n");
				DEBUG(DEB_LEV_ERR,"[     TSIF     ] PID setting error\n");
				return 0;
			}
		}
		else
		{
			DEBUG(DEB_LEV_ERR,"[     TSIF     ] error IOCTL_TSIF_SET_PID ioctl !!!\n");
			DEBUG(DEB_LEV_ERR,"[     TSIF     ] PID setting error\n");
			return 0;
		}
	}
	else
	{
		DEBUG(DEB_LEV_ERR,"[     TSIF     ] error IOCTL_TSIF_DMA_STOP ioctl !!!\n");
		DEBUG(DEB_LEV_ERR,"[     TSIF     ] error = %x\n", err);
		return 0;
	}
}


int TCC_TSIF_Close(int moduleidx, int handle) 
{
	pid_param.valid_data_cnt = 0;
	guiFirstReadFlg[moduleidx]=0;
	close(handle);
	return 0;
}

int	TCC_TSIF_Read(int moduleidx, int handle, unsigned char *buf, unsigned int read_size)	
{
	int	ret;
	int	cnt;
	
	if(!guiFirstReadFlg[moduleidx])
	{		
		if(pid_param.valid_data_cnt>0)
		{
			if(param.dma_mode == DMA_MPEG2TS_MODE)
			{
				if (ioctl(handle, IOCTL_TSIF_SET_PID, &pid_param) ==-1) 
				{
					DEBUG(DEB_LEV_ERR,"[     TSIF     ] error IOCTL_TSIF_SET_PID ioctl !!!\n");
					DEBUG(DEB_LEV_ERR,"[     TSIF     ] PID setting error\n");
					return 0;
				}
			}
		}
			
		ret = ioctl(handle, IOCTL_TSIF_DMA_START, &param);
		guiFirstReadFlg[moduleidx] = 1;

		if(ret<0)
		{
			DEBUG(DEB_LEV_ERR,"[     TSIF     ] spi open error\n");
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
			DEBUG(DEB_LEV_ERR,"[     TSIF     ] poll POLLERR\n");
			ret = 0;
		} 
		else if (poll_evt.revents & POLLIN)
		{
			ret = read(handle, buf, read_size);			
			return ret;
		}
	}
	else{/*ALOGE("[%d] READ TIMEOUT", handle);*/}
	
	return ret;	
}



int	TCC_TSIF_ReadEX(int moduleidx, int handle, unsigned char buf, unsigned int read_size)	
{
	int	ret;
	int	cnt;


	if(!guiFirstReadFlg[moduleidx])
	{
		if(pid_param.valid_data_cnt>0)
		{
			if(param.dma_mode = DMA_MPEG2TS_MODE)
			{
			if (ioctl(handle, IOCTL_TSIF_SET_PID, &pid_param) ==-1) 
			{
				DEBUG(DEB_LEV_ERR,"[     TSIF     ] error IOCTL_TSIF_SET_PID ioctl !!!\n");
				DEBUG(DEB_LEV_ERR,"[     TSIF     ] PID setting error\n");
				return 0;
			}
		}
		}
			
		ret = ioctl(handle, IOCTL_TSIF_DMA_START, &param);
		guiFirstReadFlg[moduleidx] = 1;

		if(ret<0)
		{
			DEBUG(DEB_LEV_ERR,"[     TSIF     ] spi open error\n");
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

int TCC_HWDMX_Stop(int handle)
{
	int ret;

	if(handle < 0){
		DEBUG(DEB_LEV_ERR,"[     TSIF     ] HWDMX STOP Handle Error !!!\n");
		return -1;
	}

	ret = ioctl(handle, IOCTL_TSIF_HWDMX_STOP, &param);
	if(ret<0)	
		DEBUG(DEB_LEV_ERR,"[     TSIF     ] HWDMX STOP Error !!!\n");
		
	return ret;
}

int TCC_HWDMX_Start(int handle)
{
	int ret;

	if(handle < 0){
		DEBUG(DEB_LEV_ERR,"[     TSIF     ] HWDMX START Handle Error !!!\n");
		return -1;
	}

	ret = ioctl(handle, IOCTL_TSIF_HWDMX_START, &param);
	if(ret<0)	
		DEBUG(DEB_LEV_ERR,"[     TSIF     ] HWDMX START Error !!!\n");
		
	return ret;
}

