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

#ifndef _TCC_IPTV_PROCESS_H_
#define	_TCC_IPTV_PROCESS_H_

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************
* defines 
******************************************************************************/
#ifndef TRUE
#define TRUE                1
#endif

#ifndef FALSE
#define FALSE               0
#endif

typedef struct
{
	void *pArg;
}TCCDXBPROC_INIT;

typedef struct
{
	void *pArg;
}TCCDXBPROC_SEARCH;

typedef struct
{
	int iVideoPID;
	int iAudioPID;
	int iPcrPID;
	int iVideoStreamType;
	int iAudioStreamType;	
}TCCDXBPROC_SET;

typedef struct
{
	unsigned char aucIPStr[32];
	unsigned int uiPort;	
	unsigned int uiProtocol; //Not yet support.
}TCCDXBPROC_SET_IP;


int TCCDxBProc_Init(TCCDXBPROC_INIT *pArg);
int TCCDxBProc_DeInit(void);
int TCCDxBProc_Stop(void);
int TCCDxBProc_Socketinit(void);
int TCCDxBProc_SocketIpsetting(TCCDXBPROC_SET_IP *pArg);
int TCCDxBProc_SocketStart(void);
int TCCDxBProc_SocketStop(void);
int TCCDxBProc_SocketCommand(unsigned int cmd);
int TCCDxBProc_SetActiveMode(unsigned int activemode);
int TCCDxBProc_GetVideoInfo(int *width, int *height);
int TCCDxBProc_MultiCastingSet(TCCDXBPROC_SET_IP *pArg, TCCDXBPROC_SET *pStream_Arg);
void TCCDxBProc_senddata(unsigned char *data_ptr, unsigned int data_size);
int TCCDxBProc_Fast(void);
int TCCDxBProc_InitVideoSurface(int arg);
int TCCDxBProc_DeinitVideoSurface();
int TCCDxBProc_SetVideoSurface(void *nativeWidow);
int TCCDxBProc_VideoUseSurface(void);
int TCCDxBProc_VideoReleaseSurface(void);

#ifdef __cplusplus
}
#endif

#endif

