/*
 * Copyright (C) 2013 Telechips, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_DEBUG 1
#define LOG_TAG	"DVB_MANAGER_SOCKET"
#include <utils/Log.h>
#include <cutils/properties.h>

#include <fcntl.h>

#include "tcc_dxb_interface_demux.h"
#include "tcc_iptv_manager_socket.h" 

#if	LOG_DEBUG
#define DEBUG_PRINTF		ALOGE
#else
#define DEBUG_PRINTF		
#endif

static FILE *iptv_in_handle = 0;
static unsigned char DUMP_PAHT[256] = "/mnt/sdcard/iptv_";
static unsigned int  proprty_check_cnt =0;

/**************************************************************************
FUNCTION NAME : tcc_manager_socket_init
DESCRIPTION : socket init
INPUT	 :  
OUTPUT  : 
return  : DxB_ERR_CODE
**************************************************************************/
DxB_ERR_CODE tcc_manager_socket_data_dump(unsigned char *data_ptr, unsigned int data_size)
{
//	DEBUG_PRINTF("%s %d \n", __func__, __LINE__);
	int err = DxB_ERR_OK;
	int dump_mode =0;
	char value[PROPERTY_VALUE_MAX];

	if(proprty_check_cnt ==0)
	{
		property_get("tcc.iptv.dump", value, "0");
		dump_mode = atoi(value);

		if(dump_mode == 1)
		{
			if(iptv_in_handle == 0)
			{
				unsigned char path[256];
				
				struct timeval tv;
				unsigned char	time_val[256];
				gettimeofday(&tv, NULL);
				sprintf(time_val, "%d", tv.tv_sec);

				memset(path, 0, sizeof(path));
				strcat(path, DUMP_PAHT);
				strcat(path, time_val);
				strcat(path, ".ts");
				ALOGE("%s %d path =%s  \n", __func__, __LINE__, path);
				iptv_in_handle = fopen(path,"wb");
				if(!iptv_in_handle)
				{
					ALOGE("%s %d IPTV Dump File open Error \n", __func__, __LINE__);
					return -1;
				}
				ALOGE("Dump Start\n");
			}
			else
			{
				fwrite(data_ptr, 1, data_size, iptv_in_handle);
			}
		}
		else if(dump_mode ==0)
		{
			if(iptv_in_handle)
			{
				fclose(iptv_in_handle);
				iptv_in_handle = NULL;
				ALOGE("Dump Close\n");
			}
		}
		else
			ALOGE("Not Support Mode!!!!!!\n");
	}			
	else 
		proprty_check_cnt++;

	if(proprty_check_cnt>50)
		proprty_check_cnt =0;
	
	return err;
}

/**************************************************************************
FUNCTION NAME : tcc_manager_socket_init
DESCRIPTION : socket init
INPUT	 :  
OUTPUT  : 
return  : DxB_ERR_CODE
**************************************************************************/
DxB_ERR_CODE tcc_manager_socket_init(void)
{
	DEBUG_PRINTF("%s %d \n", __func__, __LINE__);
	int err = DxB_ERR_OK;
	
	err = TCSOCKUTIL_Init();

	return err;
}

/**************************************************************************
FUNCTION NAME : tcc_manager_socket_ipsetting
DESCRIPTION : socket ip setting
INPUT	 :  char* ip(ex: 239.1.1.1)
		    int portnum (ex:3990)	
OUTPUT  : 
return  : DxB_ERR_CODE
**************************************************************************/
DxB_ERR_CODE tcc_manager_socket_ipsetting(unsigned char* ip, unsigned int portnum, int protocol)
{
	DEBUG_PRINTF("%s %d \n", __func__, __LINE__);
	int err = DxB_ERR_OK;
	
	err = TCSOCKUTIL_IPSetting(ip, portnum, protocol);

	return err;
}

/**************************************************************************
FUNCTION NAME : tcc_manager_socket_start
DESCRIPTION : socket data read start. socket open & create thread
INPUT	 :  
OUTPUT  : 
return  : DxB_ERR_CODE
**************************************************************************/
DxB_ERR_CODE tcc_manager_socket_start(void)
{
	DEBUG_PRINTF("%s %d \n", __func__, __LINE__);
	int err = DxB_ERR_OK;

	err = TCSOCKUTIL_Start();

	return err;
}

/**************************************************************************
FUNCTION NAME : tcc_manager_socket_stop
DESCRIPTION : socket data read stop. socket close & join thread 
INPUT	 :  
OUTPUT  : 
return  : DxB_ERR_CODE
**************************************************************************/
DxB_ERR_CODE tcc_manager_socket_stop(void)
{
	DEBUG_PRINTF("%s %d \n", __func__, __LINE__);
	int err = DxB_ERR_OK;
	
	err = TCSOCKUTIL_Stop();

	return err;
}

/**************************************************************************
FUNCTION NAME : tcc_manager_socket_senddata
DESCRIPTION : ts data send to interface layer
INPUT	 :  char *data_prt :  ts_data ptr
		    int  data_size : ts_data size
OUTPUT  : 
return  : DxB_ERR_CODE
**************************************************************************/
void tcc_manager_socket_senddata(unsigned char *data_ptr, unsigned int data_size)
{
//	DEBUG_PRINTF("%s %d data_size = %d \n", __func__, __LINE__, data_size);
	int err = DxB_ERR_OK;

	TCC_DxB_DEMUX_Send_data(NULL, data_ptr, data_size);
	tcc_manager_socket_data_dump(data_ptr, data_size);
	return err;
}

/**************************************************************************
FUNCTION NAME : tcc_manager_socket_sendcommand
DESCRIPTION : cmd process.
INPUT	 :  int cmd : customer cmd
OUTPUT  : 
return  : DxB_ERR_CODE
**************************************************************************/
DxB_ERR_CODE tcc_manager_socket_command(int cmd)
{
	DEBUG_PRINTF("%s %d \n", __func__, __LINE__);
	int err = DxB_ERR_OK;

	TCSOCKUTIL_Command(cmd);
	return err;
}


