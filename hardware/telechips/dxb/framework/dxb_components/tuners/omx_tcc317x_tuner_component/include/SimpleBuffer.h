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

#ifndef _SIMPLEBUFFER_H_
#define _SIMPLEBUFFER_H_
#ifdef __cplusplus
extern "C" {
#endif


typedef struct SIMPLE_RINGBUFFER_FOR_STREAM
{
	unsigned int 	mHead,mTail;
	unsigned int	mMaxBufferSize;
	char *pBuffer;
	char mState;
	
} SRBUFFER;

//#define 		REC_Q_SIZE		1024*1024
#define		BUFFER_EMPTY		1
#define		BUFFER_FULL		2
#define		BUFFER_NORMAL		3

extern void QInitBuffer(SRBUFFER* pBuffer,unsigned int size,char* pBuff);
extern int QGetData(SRBUFFER* pBuffer, char * pBuff, long RequestSize);
extern int QPutData(SRBUFFER* pBuffer, char * pBuff, long RequestSize);
extern int QGetDataEX(SRBUFFER* pBuffer, char * pBuff, long RequestSize) ;
extern int QHowManyData(SRBUFFER* pBuffer);

#ifdef __cplusplus
}
#endif

#endif	// _SIMPLEBUFFER_H_

/* end of file */

