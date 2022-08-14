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

#define LOG_TAG	"TCC_SOCKET_UTIL"
#include <utils/Log.h>
#include <pthread.h>

#define LOG_DEBUG 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>

#include "tcc_socket.h"
#include "tcc_socket_util.h"

#if	LOG_DEBUG
#define DEBUG_PRINTF		ALOGE
#else
#define DEBUG_PRINTF		
#endif

#define	TS_PACKETSIZE 		(200)//	(188)
#define SOCKET_RECVBUFSIZE	512*1024

static TCCSOCKET_SET_IP SocketInfo;
static TCSOCK_HANDLE SockHandle;	
static int Socket_Thread_Runing;
static pthread_t Socket_Thread_Id;
static unsigned int	Socket_RecvBufSize;

unsigned char *Socket_DataBuf;
unsigned int	SocketDataReadSize = 64*1024;

static unsigned long get_ipv4_addr (const char *ip)
{
	DEBUG_PRINTF("%s %d \n", __func__, __LINE__);

	unsigned long sock_addr = 0;
	struct addrinfo hints, *res0;
	struct sockaddr *res;

	if (!ip)
	{
		return (0);
	}

	memset (&hints, 0, sizeof (hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	res0 = (struct addrinfo *) 0;
	if (!getaddrinfo (ip, NULL, &hints, &res0))
	{
		if (res0)
		{
			res = (struct sockaddr *) res0->ai_addr;
			sock_addr = (((struct sockaddr_in *) res)->sin_addr).s_addr;
		}
	}
	if (res0)
	{
		freeaddrinfo (res0);
	}

	return ntohl (sock_addr);
}

static int check_ipv4_type (const char *p_ip)
{
	DEBUG_PRINTF("%s %d \n", __func__, __LINE__);

	int       i_result = (-1);
	unsigned long int ul_ip = 0;

	if (p_ip)
	{
		ul_ip = get_ipv4_addr (p_ip);

		/* class select */
		if ((0x1000001 <= ul_ip) && (ul_ip <= 0x7efffffe))
		{
			i_result = 1;
		}
		else if ((0x80010001 <= ul_ip) && (ul_ip <= 0xbffffffe))
		{
			i_result = 2;
		}
		else if ((0xc0000101 <= ul_ip) && (ul_ip <= 0xdffffefe))
		{
			i_result = 3;
		}
		else if ((0xe0000000 <= ul_ip) && (ul_ip <= 0xefffffff))
		{
			i_result = 4;	/* multicasting */
		}
		else if ((0xf0000000 <= ul_ip) && (ul_ip <= 0xfefffffe))
		{
			i_result = 5;	/* reserved */
		}
	}

	return i_result;
}

/**************************************************************************
FUNCTION NAME : TCSOCKETUTIL_Thread
DESCRIPTION : data read & data send
INPUT	 :  
OUTPUT  : 
return  : 
**************************************************************************/
void TCSOCKUTIL_Thread(void *args)
{
	DEBUG_PRINTF("%s %d \n", __func__, __LINE__);
	int		data_readsize =0;
	unsigned int		ret =0;
	unsigned char 	*pucPayload;
	unsigned int 	uiPayloadSize;

	while(Socket_Thread_Runing)
	{
		if(SockHandle)
		{
			data_readsize = TCSOCKUTIL_Read(SockHandle, Socket_DataBuf, SocketDataReadSize, 500);

			if(data_readsize>0)
			{
				ret = TCRTP_Parse (Socket_DataBuf, data_readsize, &pucPayload, &uiPayloadSize);		

				if(ret < 0)
				{
					ALOGE ("Received Packet Error!!! %d", ret); 
				}	
				else
					tcc_manager_socket_senddata(pucPayload, uiPayloadSize);
			}
		}
		else
			usleep(5000);
		
	}

	DEBUG_PRINTF("%s %d TCSOCKUTIL_Thread join\n", __func__, __LINE__);
}

/**************************************************************************
FUNCTION NAME : TCSOCKUTIL_Init
DESCRIPTION : socket init
INPUT	 :  
OUTPUT  : 
return  : 
**************************************************************************/
int TCSOCKUTIL_Init(void)
{
	DEBUG_PRINTF("%s %d \n", __func__, __LINE__);
	int err = SOCKET_ERR_OK;

	Socket_Thread_Runing = NULL;
	Socket_Thread_Id = NULL;
	SockHandle = NULL;
	Socket_DataBuf = NULL;
	Socket_RecvBufSize = SOCKET_RECVBUFSIZE;
	
	return SOCKET_ERR_OK;
}

/**************************************************************************
FUNCTION NAME : TCSOCKUTIL_IPSetting
DESCRIPTION : socket ip setting
INPUT	 :  char* ip(ex: 239.1.1.1)
		    int portnum (ex:3990)	
OUTPUT  : 
return  : 
**************************************************************************/
int TCSOCKUTIL_IPSetting(char *pcIPstr, int iPort, int protocol)
{
	DEBUG_PRINTF("%s %d \n", __func__, __LINE__);
	int err = SOCKET_ERR_OK;

	strcpy((char *)SocketInfo.aucIPStr, pcIPstr);
	SocketInfo.uiPort = iPort;	
	SocketInfo.uiProtocol = protocol;	

	return err;
}

/**************************************************************************
FUNCTION NAME : TCSOCKUTIL_Start
DESCRIPTION : socket open, make data_read & data_send thread
INPUT	 :  
OUTPUT  : 
return  : 
**************************************************************************/
int TCSOCKUTIL_Start(void)
{
	DEBUG_PRINTF("%s %d \n", __func__, __LINE__);
	int iStatus;
	int err = SOCKET_ERR_OK;

	if(SockHandle != NULL)
	{
		TCSOCKUTIL_Stop();
	}

	SockHandle  = TCSOCKUTIL_Open((unsigned char *)SocketInfo.aucIPStr , SocketInfo.uiPort);

	DEBUG_PRINTF("%s %d SockHandle = %x \n", __func__, __LINE__, SockHandle);
	DEBUG_PRINTF("%s %d ip = %s port = %d \n", __func__, __LINE__, SocketInfo.aucIPStr, SocketInfo.uiPort);

	if(SockHandle == NULL)
	{
		return SOCKET_ERR_SOCKET_OPENFAIL;
	}

	Socket_DataBuf = malloc(SocketDataReadSize + TS_PACKETSIZE);
	if (Socket_DataBuf == NULL)
	{
		ALOGE ("CANNOT Alloc Read Buffer : Size(%d)!!!\n", SocketDataReadSize);
		return SOCKET_ERR_ERROR;
	}

	Socket_Thread_Runing =1;
	if (iStatus = pthread_create(&Socket_Thread_Id, NULL, &TCSOCKUTIL_Thread, NULL))
	{
		ALOGE("Error: fail to create Socket_Thread, status=%d\n", iStatus);
		return SOCKET_ERR_ERROR;
	}

	return err;
}

/**************************************************************************
FUNCTION NAME : TCSOCKUTIL_Stop
DESCRIPTION : socket close, destroy thread
INPUT	 :  
OUTPUT  : 
return  : 
**************************************************************************/
int TCSOCKUTIL_Stop(void)
{
	DEBUG_PRINTF("%s %d \n", __func__, __LINE__);
	int err = SOCKET_ERR_OK;

	Socket_Thread_Runing = NULL;

	if(SockHandle != NULL)
		TCSOCKUTIL_Close(SockHandle);

	SockHandle = NULL;

	if(Socket_Thread_Id != NULL)
		pthread_join(Socket_Thread_Id, NULL);

	Socket_Thread_Id = NULL;

	if(Socket_DataBuf != NULL)
		free (Socket_DataBuf);

	Socket_DataBuf = NULL;

	return err;
}


/**************************************************************************
FUNCTION NAME : TCSOCKUTIL_Stop
DESCRIPTION : socket close, destroy thread
INPUT	 :  
OUTPUT  : 
return  : 
**************************************************************************/
int TCSOCKUTIL_Command(int cmd)
{
	DEBUG_PRINTF("%s %d \n", __func__, __LINE__);
	int err = SOCKET_ERR_OK;

	switch(cmd)
	{

		default:
			break;
	}
	
	return err;
}

/**************************************************************************
FUNCTION NAME : TCSOCKUTIL_Open
DESCRIPTION : socket open
INPUT	 :  
OUTPUT  : 
return  : DxB_ERR_CODE
**************************************************************************/
TCSOCK_HANDLE TCSOCKUTIL_Open (char *pcIPstr, int iPort)
{
	DEBUG_PRINTF("%s %d \n", __func__, __LINE__);
	TCSocket *p_sock = NEW_TCSOCK ();
	socklen_t len = 0;
	int ret;

	if(p_sock == NULL)
		return NULL;
	
	if( p_sock->Init (p_sock, _UDP_))
	{
		if (p_sock->Bind (p_sock, iPort, NULL))
		{
			if( p_sock->SetRemoteAddr (p_sock, pcIPstr, iPort) )
			{
				if (check_ipv4_type (pcIPstr) == 4)
				{
					if(p_sock->Join_M (p_sock))
					{							
				#if	0
						socklen_t	len = 0;
						unsigned int Default_Socket_RecvBufSize;
						len = sizeof(Default_Socket_RecvBufSize);
						ret = p_sock->GetSockOpt (p_sock->m_hSocket, SOL_SOCKET, SO_RCVBUF, &Default_Socket_RecvBufSize, &len);
						if(ret != TRUE)
							ALOGE("getsockopt Error ret	= %d \n", ret );
						ALOGE("1. Default_Socket_RecvBufSize : %d len= %d\n", Default_Socket_RecvBufSize, len);
				#endif
						ret = TCSOCKUTIL_SetOpt(p_sock, SOL_SOCKET, SO_RCVBUF, &Socket_RecvBufSize);
						if(ret != TRUE)
							ALOGE("setsockopt Error ret	= %d \n", ret );

						return p_sock;
					}		
				}
				else
				{
					ret = TCSOCKUTIL_SetOpt(p_sock, SOL_SOCKET, SO_RCVBUF, &Socket_RecvBufSize);
					if(ret != TRUE)
						ALOGE("setsockopt Error ret	= %d \n", ret );

					return p_sock;
				}
			}	
		}
	}	
	DELETE_TCSOCK(p_sock);
	return SOCKET_ERR_OK;
}

/**************************************************************************
FUNCTION NAME : TCSOCKUTIL_SetOpt
DESCRIPTION : data read
INPUT	 :  handle :  socket handle
		    iLevel :  
		    iOptName :  	
		    pOptVal : 
OUTPUT  : ts data
return  : read size
**************************************************************************/
int TCSOCKUTIL_SetOpt(TCSOCK_HANDLE handle, int iLevel, int iOptName, const void *pOptVal)
{
	TCSocket *p_sock =  (TCSocket *)handle;
	socklen_t 	len = 0;

	len = sizeof(pOptVal);
	return p_sock->SetSockOpt(p_sock, iLevel, iOptName, pOptVal, len);
}

/**************************************************************************
FUNCTION NAME : TCSOCKUTIL_Read
DESCRIPTION : data read
INPUT	 :  handle :  socket handle
		    pucBuffer :  data buf
		    iBufferSize :  data read size	
		    iTimeOutMS : read timeout (msec)
OUTPUT  : ts data
return  : read size
**************************************************************************/
int TCSOCKUTIL_Read(TCSOCK_HANDLE handle, unsigned char *pucBuffer, int iBufferSize, int iTimeOutMS)
{
	TCSocket *p_sock =  (TCSocket *)handle;
	return p_sock->Recv(p_sock, pucBuffer, iBufferSize, iTimeOutMS);
}

/**************************************************************************
FUNCTION NAME : TCSOCKUTIL_Close
DESCRIPTION : socket close
INPUT	 :  handle :  socket handle
OUTPUT  : 
return  : DxB_ERR_CODE
**************************************************************************/
void TCSOCKUTIL_Close(TCSOCK_HANDLE handle)
{
	DEBUG_PRINTF("%s %d \n", __func__, __LINE__);

	TCSocket *p_sock =  (TCSocket *)handle;
	DELETE_TCSOCK(p_sock);
}
