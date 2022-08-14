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


#ifndef __TCSOCKET_H__
#define __TCSOCKET_H__

#include <stdint.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/poll.h>


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef msleep
#define msleep(X) usleep((X)*1000)
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif


#define CLOSE_HANDLE(H, N, F) if( H != N ) { F(H); H = N; }
#define SET_F(F, X) ((F) |= (X))
#define CLR_F(F, X) ((F) &= ~(X))
#define TOG_F(F, X) ((F) ^= (X))
#define IS_F(F, X)  ((F) & (X))


typedef int BOOL;
typedef unsigned long DWORD;
typedef unsigned char BYTE;
typedef unsigned int UINT;
typedef unsigned long ULONG;

typedef enum _SockType_e {
    _TCP_,
    _UDP_,
    _MULTICAST_,
} SockType_e;


typedef struct TCSocket_ TCSocket;
struct TCSocket_ {
    int m_hSocket;
    int m_iFlag;
    SockType_e m_eSockType;
    char* m_szIP;
    int m_iPort;
	pthread_mutex_t    m_xlock;
    struct sockaddr_in m_xAddr;
    struct timeval m_xTime;
    struct pollfd m_xPoll;


    BOOL (*Init)(TCSocket *h, SockType_e e);
    BOOL (*SetRemoteAddr)(TCSocket *h, char* szIP, int iPort);
    BOOL (*Bind)(TCSocket *h, int iPort, char *szIP);
    BOOL (*SetBlockMode)(TCSocket *h, BOOL b);
    BOOL (*Connect)(TCSocket *h, int iTimeOut);
    int (*Recv)(TCSocket *h, BYTE *pBuf, int iBufSize, int iTimeOut);
    int (*Send)(TCSocket *h, BYTE *pBuf, int iBufSize, int iTimeOut);
    int (*SendTo)(TCSocket *h, BYTE *pBuf, int iBufSize);
    BOOL (*SetSockOpt)(TCSocket *h, int iLevel, int iOptName, const void *pOptVal, socklen_t iOptLen);
    BOOL (*Join_M)(TCSocket *h);
    BOOL (*Drop_M)(TCSocket *h);
    BOOL (*Listen)(TCSocket *h, int iListenCnt);
    BOOL (*GetAddr)(TCSocket *h, char* szHost, in_addr_t *pInAddr);

    int (*Accept)(TCSocket *h, int *phSocket, int iTimeOut);


    int (*GetSocket)(TCSocket *h);
    void (*SetSocket)(TCSocket *h, int s);
    void (*SetPoll)(TCSocket *h, int iTimeOut, int iEvents);

};

extern TCSocket *NEW_TCSOCK();
extern TCSocket *DELETE_TCSOCK(TCSocket *h);

#endif // __TCSOCKET_H__

