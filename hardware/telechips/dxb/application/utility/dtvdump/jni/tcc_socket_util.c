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

static unsigned long get_ipv4_addr (const char *ip)
{
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

TCSOCK_HANDLE TCSOCKUTIL_Open (char *pcIPstr, int iPort)
{
	TCSocket *p_sock = NEW_TCSOCK ();
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
						int ret;
						unsigned int Socket_RecvBufSize = 1316*1024;
						ret = TCSOCKUTIL_SetOpt(p_sock, SOL_SOCKET, SO_RCVBUF, &Socket_RecvBufSize);
						if(ret != TRUE)
							ALOGE("setsockopt Error ret	= %d \n", ret );
						return p_sock;
					}		
				}
			}	
		}
	}	
	DELETE_TCSOCK(p_sock);
	return NULL;
}

int TCSOCKUTIL_SetOpt(TCSOCK_HANDLE handle, int iLevel, int iOptName, const void *pOptVal)
{
	TCSocket *p_sock =  (TCSocket *)handle;
	socklen_t 	len = 0;

	len = sizeof(pOptVal);
	return p_sock->SetSockOpt(p_sock, iLevel, iOptName, pOptVal, len);
}

int TCSOCKUTIL_Read(TCSOCK_HANDLE handle, unsigned char *pucBuffer, int iBufferSize, int iTimeOutMS)
{
	TCSocket *p_sock =  (TCSocket *)handle;
	return p_sock->Recv(p_sock, pucBuffer, iBufferSize, iTimeOutMS);

}

void TCSOCKUTIL_Close(TCSOCK_HANDLE handle)
{
	TCSocket *p_sock =  (TCSocket *)handle;
	DELETE_TCSOCK(p_sock);
}
