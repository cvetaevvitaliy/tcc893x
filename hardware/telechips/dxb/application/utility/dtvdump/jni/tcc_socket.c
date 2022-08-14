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

#define LOG_TAG	"TCC_SOCKET"
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
#include <pthread.h>

#include "tcc_socket.h"

#include <errno.h>

static BOOL TCSOCK_IsIP (char *szIP)
{
	int       i = 0;
	for (i = 0; szIP[i] != '\0'; i++)
	{
		if (!isdigit (szIP[i]) && (szIP[i] != '.'))
		{
			return FALSE;
		}
	}
	return TRUE;
}

#if 0
static BOOL TCSOCK_Init (TCSocket * h, int hSocket, SockType_e e)
{
	h->m_hSocket = hSocket;
	h->m_eSockType = e;
	return TRUE;
}
#endif

static BOOL TCSOCK_Init (TCSocket * h, SockType_e e)
{
	switch (e)
	{
	case _TCP_:
		h->m_hSocket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
		break;
	case _UDP_:
	case _MULTICAST_:
		h->m_hSocket = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#if 0
		{
			int       ttl = 4;
			setsockopt (h->m_hSocket, IPPROTO_IP, IP_TTL, (void *) (&ttl), (socklen_t) sizeof (ttl));
			setsockopt (h->m_hSocket, IPPROTO_IP, IP_MULTICAST_TTL, (void *) (&ttl), (socklen_t) sizeof (ttl));
		}
#endif
		break;
	default:
		break;
	}

	if (h->m_hSocket == INVALID_SOCKET)
	{
		return FALSE;
	}

	h->m_eSockType = e;
	return TRUE;
}

static void TCSOCK_SetPoll (TCSocket * h, int iTimeOut, int iEvents)
{
	h->m_xPoll.fd = h->m_hSocket;
	h->m_xPoll.events = iEvents;
	h->m_xPoll.revents = 0;
}

static BOOL TCSOCK_SetRemoteAddr (TCSocket * h, char *szIP, int iPort)
{
	if (szIP && (iPort > 0))
	{
		CLOSE_HANDLE (h->m_szIP, NULL, free);
		h->m_szIP = (char *) malloc (strlen (szIP) + 1);
		if (!h->m_szIP)
		{
			return FALSE;
		}

		strcpy (h->m_szIP, szIP);
		h->m_iPort = iPort;

		memset (&h->m_xAddr, 0, sizeof (h->m_xAddr));
		h->m_xAddr.sin_family = AF_INET;
		/* h->m_xAddr.sin_addr.s_addr = inet_addr(szIP); */
		h->GetAddr (h, szIP, &(h->m_xAddr.sin_addr.s_addr));
		h->m_xAddr.sin_port = htons ((unsigned short) iPort);

		return TRUE;
	}
	return FALSE;
}

static BOOL TCSOCK_Bind (TCSocket * h, int iPort, char *szIP /* = NULL */ )
{
	struct sockaddr_in xBindAddr;
	memset (&xBindAddr, 0, sizeof (xBindAddr));
	xBindAddr.sin_family = AF_INET;
	//xBindAddr.sin_addr.s_addr = szIP ? inet_addr(szIP) : htonl(INADDR_ANY);
	if (szIP)
	{
		h->GetAddr (h, szIP, &(xBindAddr.sin_addr.s_addr));
	}
	else
	{
		xBindAddr.sin_addr.s_addr = htonl (INADDR_ANY);
	}
	xBindAddr.sin_port = htons ((unsigned short) iPort);

	if (bind (h->m_hSocket, (struct sockaddr *) &xBindAddr, sizeof (struct sockaddr_in)) == 0)
	{
		return TRUE;
	}
	return FALSE;
}

static BOOL TCSOCK_SetBlockMode (TCSocket * h, BOOL b)
{
	if (h->m_hSocket != INVALID_SOCKET)
	{
		int       iFlag = 0;
		iFlag = fcntl (h->m_hSocket, F_GETFL);
		iFlag = b ? (iFlag & ~O_NONBLOCK) : (iFlag | O_NONBLOCK);
		fcntl (h->m_hSocket, F_SETFL, iFlag);
		return TRUE;
	}
	return FALSE;
}

static BOOL TCSOCK_Connect (TCSocket * h, int iTimeOut)
{
	BOOL      bRet = FALSE;
	int       iRet = -1, iFlag = 0;
	if (h->m_hSocket != INVALID_SOCKET)
	{
		iRet = connect (h->m_hSocket, (struct sockaddr *) &h->m_xAddr, sizeof (h->m_xAddr));
		if (iRet == 0)
		{
			bRet = TRUE;
		}
		else if (iTimeOut >= 0)
		{
			iFlag = fcntl (h->m_hSocket, F_GETFL);
			if ((iFlag != -1) && (iFlag & O_NONBLOCK))
			{
				h->SetPoll (h, iTimeOut, POLLOUT);
				iRet = poll (&h->m_xPoll, 1u, iTimeOut);
				if ((iRet > 0) && (h->m_xPoll.revents & POLLOUT))
				{
					bRet = TRUE;
				}
			}
		}
	}
	return bRet;
}

static BOOL TCSOCK_Listen (TCSocket * h, int iListenCnt)
{
	return (listen (h->m_hSocket, iListenCnt) == 0) ? TRUE : FALSE;
}

#if 0
static int TCSOCK_Accept (int iTimeOut)
{
	int       hSocket = INVALID_SOCKET;
	int       iRet = -1;
	socklen_t iAddrLen = sizeof (h->m_xAddr);

	SetTimeOut (iTimeOut, TRUE);
	iRet = select ((h->m_hSocket + 1), &h->m_xFD_Rx, NULL, NULL, &h->m_xTime);
	if (iRet > 0)
	{
		hSocket = accept (h->m_hSocket, (struct sockaddr *) &h->m_xAddr, &iAddrLen);
	}
	return hSocket;
}
#else
static int TCSOCK_Accept (TCSocket * h, int *phSocket, int iTimeOut)
{
	int       iRet = -1;
	if (phSocket)
	{
		socklen_t iAddrLen = sizeof (h->m_xAddr);
		h->SetPoll (h, iTimeOut, POLLIN | POLLPRI);
		iRet = poll (&h->m_xPoll, 1u, iTimeOut);
		if ((iRet > 0) && (h->m_xPoll.revents & (POLLIN | POLLPRI)))
		{
			*phSocket = accept (h->m_hSocket, (struct sockaddr *) &h->m_xAddr, &iAddrLen);
		}
	}
	return iRet;
}
#endif

/*
 * > 0: success, == 0: time out, < 0: error
 */
static int TCSOCK_Recv (TCSocket * h, BYTE * pBuf, int iBufSize, int iTimeOut)
{
	int       iRecvByte = -1;
	pthread_mutex_lock(&h->m_xlock);	
	h->SetPoll (h, iTimeOut, POLLIN | POLLPRI);
	iRecvByte = poll (&h->m_xPoll, 1u, iTimeOut);
	if (iRecvByte > 0)
	{
		if (h->m_xPoll.revents & (POLLIN | POLLPRI))
		{
			iRecvByte = recv (h->m_hSocket, pBuf, iBufSize, h->m_iFlag);
			if (iRecvByte == 0)
			{
				iRecvByte = -1;
			}
		}
		else
		{
			iRecvByte = -1;
		}
	}
	pthread_mutex_unlock(&h->m_xlock);	
	return iRecvByte;
}

static int TCSOCK_Send (TCSocket * h, BYTE * pBuf, int iBufSize, int iTimeOut)
{
	int       iSendByte = -1;
	h->SetPoll (h, iTimeOut, POLLOUT);
	iSendByte = poll (&h->m_xPoll, 1u, iTimeOut);
	if (iSendByte > 0)
	{
		if (h->m_xPoll.revents & (POLLOUT))
		{
			iSendByte = send (h->m_hSocket, pBuf, iBufSize, h->m_iFlag);
			if (iSendByte == 0)
			{
				iSendByte = -1;
			}
		}
		else
		{
			iSendByte = -1;
		}
	}
	return iSendByte;
}

static int TCSOCK_SendTo (TCSocket * h, BYTE * pBuf, int iBufSize)
{
	return sendto (h->m_hSocket, pBuf, iBufSize, h->m_iFlag, (struct sockaddr *) &h->m_xAddr, sizeof (h->m_xAddr));
}

static BOOL TCSOCK_JoinM (TCSocket * h)
{
	struct ip_mreq xJoinAddr;
	int       bReuse = 1;
	xJoinAddr.imr_multiaddr.s_addr = inet_addr (h->m_szIP);
	xJoinAddr.imr_interface.s_addr = INADDR_ANY;
	if (h->SetSockOpt (h, SOL_SOCKET, SO_REUSEADDR, (char *) &bReuse, sizeof (bReuse)))
	{
		return h->SetSockOpt (h, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &xJoinAddr, sizeof (xJoinAddr));
	}
	return FALSE;
}

static BOOL TCSOCK_DropM (TCSocket * h)
{
	struct ip_mreq xJoinAddr;
	xJoinAddr.imr_multiaddr.s_addr = inet_addr (h->m_szIP);
	xJoinAddr.imr_interface.s_addr = INADDR_ANY;
	return h->SetSockOpt (h, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *) &xJoinAddr, sizeof (xJoinAddr));
}

static BOOL TCSOCK_SetSockOpt (TCSocket * h, int iLevel, int iOptName, const void *pOptVal, socklen_t iOptLen)
{
	
	return (setsockopt (h->m_hSocket, iLevel, iOptName, pOptVal, iOptLen) == 0) ? TRUE : FALSE;
}

static BOOL TCSOCK_GetAddr (TCSocket * h, char *szHost, in_addr_t * pInAddr)
{
	if (szHost && pInAddr)
	{
		if (TCSOCK_IsIP (szHost))
		{
			*pInAddr = inet_addr (szHost);
			return TRUE;
		}
		else
		{
			struct addrinfo *pxAddrInfo = NULL;
			if (getaddrinfo (szHost, NULL, NULL, &pxAddrInfo) == 0)
			{
				if (pxAddrInfo)
				{
					*pInAddr = ((struct sockaddr_in *) (pxAddrInfo->ai_addr))->sin_addr.s_addr;
					freeaddrinfo (pxAddrInfo);
					return TRUE;
				}
			}
		}
	}
	return FALSE;

}

static int TCSOCK_GetSocket (TCSocket * h)
{
	return h->m_hSocket;
}

static void TCSOCK_SetSocket (TCSocket * h, int s)
{
	h->m_hSocket = s;
}

TCSocket *NEW_TCSOCK (void)
{
	TCSocket *h = NULL;

	h = malloc (sizeof (TCSocket));
	if (h)
	{
		memset (h, 0, sizeof (TCSocket));

		h->m_hSocket = INVALID_SOCKET;
		h->m_eSockType = _TCP_;
		h->m_szIP = NULL;
		h->m_iPort = 0;

		pthread_mutex_init(&h->m_xlock, NULL);
		
		memset (&h->m_xAddr, 0, sizeof (h->m_xAddr));
		h->m_iFlag = MSG_NOSIGNAL;
		h->Init = TCSOCK_Init;
		h->SetRemoteAddr = TCSOCK_SetRemoteAddr;
		h->Bind = TCSOCK_Bind;
		h->SetBlockMode = TCSOCK_SetBlockMode;
		h->Connect = TCSOCK_Connect;
		h->Recv = TCSOCK_Recv;
		h->Send = TCSOCK_Send;
		h->SendTo = TCSOCK_SendTo;
		h->SetSockOpt = TCSOCK_SetSockOpt;
		h->Join_M = TCSOCK_JoinM;
		h->Drop_M = TCSOCK_DropM;
		h->Listen = TCSOCK_Listen;
		h->GetAddr = TCSOCK_GetAddr;
		h->Accept = TCSOCK_Accept;
		h->GetSocket = TCSOCK_GetSocket;
		h->SetSocket = TCSOCK_SetSocket;
		h->SetPoll = TCSOCK_SetPoll;
	}

	return h;
}

TCSocket *DELETE_TCSOCK (TCSocket * h)
{

	pthread_mutex_lock(&h->m_xlock);	
	CLOSE_HANDLE (h->m_hSocket, INVALID_SOCKET, close);	
	CLOSE_HANDLE (h->m_szIP, NULL, free);
	pthread_mutex_unlock(&h->m_xlock);	

	pthread_mutex_destroy(&h->m_xlock);		
	CLOSE_HANDLE (h, NULL, free);
	return NULL;
}
