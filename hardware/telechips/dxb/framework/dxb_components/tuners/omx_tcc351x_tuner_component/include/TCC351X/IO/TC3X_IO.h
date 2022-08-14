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

#ifndef __TC3X_IO_h__
#define __TC3X_IO_h__

#include "TC3X_IO_UserDefine.h"
#include "TC3X_IO_UTIL.h"
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>

#ifndef NULL
#define NULL 0
#endif

#define TC_SEMAPHORE    sem_t
#define TC_QUEUE        int
#define TC_TASK         pthread_t

void      TC3X_IO_Host_Preset (int moduleidx);

#if defined(USE_ANDROID) || defined(USE_LINUX)
long long TC3X_IO_GET_TIMECNT_ms (void);
#endif
unsigned int TC3X_IO_GET_TIMECNT_us (void);

void      TC3X_IO_PW_RESET (int moduleidx);
void      TC3X_IO_PW_ON (int moduleidx);
void      TC3X_IO_PW_DN (int moduleidx);
void      TC3X_IO_Sleep (unsigned int ms);

void      TC3X_IO_Set_AntennaPower(int arg);
void      TC3X_IO_Set_SPIHandler (int moduleidx, FN_v_ii BB_SPI_SLV_LISR, FN_v_i BB_SPI_SLV_HISR);
void      TC3X_IO_Set_BBIntHandler (int moduleidx, FN_v_i BB_INT_LISR, FN_v_i BB_INT_HISR);

#if defined (STDDEF_DVBH)
void      TC3X_IO_Set_5mTimerHandler (int moduleidx, FN_v_i BB_HISR_5mTIMER);
#endif // STDDEF_DVBH

#if defined (STDDEF_DVBH)
void      TC3X_IO_ACTIVE_HISR_5mTimer (int moduleidx);
#endif // STDDEF_DVBH
void      TC3X_IO_ACTIVE_HISR_SPI (int moduleidx);

void      TC3X_IO_IF_SemaphoreInit (int moduleidx);
void      TC3X_IO_IF_SemaphoreDelete (int moduleidx);

#if defined (STDDEF_DVBH)
void      TC3X_IO_RS_SemaphoreInit (int moduleidx);
void      TC3X_IO_RS_SemaphoreDelete (int moduleidx);
#endif // STDDEF_DVBH

void      TC3X_IO_MailBox_SemaphoreInit (int moduleidx);
void      TC3X_IO_MailBox_SemaphoreDelete (int moduleidx);

void      TC3X_IO_OP_Mailbox_SemaphoreInit (int moduleidx);
void      TC3X_IO_OP_Mailbox_SemaphoreDelete (int moduleidx);

int       TC3X_IO_IF_LOCK (int moduleidx);
int       TC3X_IO_IF_UnLOCK (int moduleidx);
int       TC3X_IO_MailBox_LOCK (int moduleidx);
int       TC3X_IO_MailBox_UnLOCK (int moduleidx);
int       TC3X_IO_OP_Mailbox_LOCK (int moduleidx);
int       TC3X_IO_OP_Mailbox_UnLOCK (int moduleidx);

#if defined (STDDEF_DVBH)
int       TC3X_IO_RS_LOCK (int moduleidx);
int       TC3X_IO_RS_UnLOCK (int moduleidx);
#endif // STDDEF_DVBH

void     *TC3X_IO_QUE_INT_Create (int moduleidx);
void      TC3X_IO_QUE_INT_Delete (int moduleidx);

#if defined (STDDEF_DVBH)
void     *TC3X_IO_QUE_TIMES_Create (int moduleidx);
void      TC3X_IO_QUE_TIMES_Delete (int moduleidx);
void     *TC3X_IO_QUE_RS_Create (int moduleidx);
void      TC3X_IO_QUE_RS_Delete (int moduleidx);
void     *TC3X_IO_QUE_MPE_Create (int moduleidx);
void      TC3X_IO_QUE_MPE_Delete (int moduleidx);
#endif // STDDEF_DVBH

int       TC3X_IO_QUE_Reset (void *queue);
int       TC3X_IO_QUE_Send (void *queue, void *message);
int       TC3X_IO_QUE_Receive (void *queue, void *message, unsigned int suspend);

void      TC3X_IO_InterruptTask_Create (int moduleidx, void (*pTC3XTask_Interrupt) (void));
void      TC3X_IO_InterruptTask_Delete (int moduleidx);
#if defined (STDDEF_DVBH)
void      TC3X_IO_DVBH_Manage_Task_Create (int moduleidx, void (*pTC3XTask_DVBH_Manage) (void));
void      TC3X_IO_DVBH_Manage_Task_Delete (int moduleidx);
void      TC3X_IO_MPEFECTask_Create (int moduleidx, void (*pTC3XTask_MPEFEC) (void));
void      TC3X_IO_MPEFECTask_Delete (int moduleidx);
void      TC3X_IO_TimeSlicingTask_Create (int moduleidx, void (*pTC3XTask_TimeSlicing) (void));
void      TC3X_IO_TimeSlicingTask_Delete (int moduleidx);
#endif // STDDEF_DVBH

void      TC3X_IO_HISR_BB_INT_Create (int moduleidx);
void      TC3X_IO_HISR_BB_INT_Delete (int moduleidx);

#if defined (STDDEF_DVBH)
void      TC3X_IO_HISR_TC3X_TIMER_5M_Create (int moduleidx);
void      TC3X_IO_HISR_TC3X_TIMER_5M_Delete (int moduleidx);
#endif // STDDEF_DVBH

void      TC3X_IO_BB_INT_Ctrl (int moduleidx, int command, int TrigMode);
void      TC3X_IO_Stream_SPI_Ctrl (int moduleidx, int command, unsigned char **pStreamBuff, int PktSize, int PktNum, int StreamIO);

void      TC3X_IO_LISR_BB_INT_Handler0 (void);
void      TC3X_IO_HISR_BB_INT_Handler0 (void);
void      TC3X_IO_LISR_SPI_Handler0 (void);
void      TC3X_IO_HISR_SPI_Handler0 (void);

void      TC3X_IO_LISR_BB_INT_Handler1 (void);
void      TC3X_IO_HISR_BB_INT_Handler1 (void);
void      TC3X_IO_LISR_SPI_Handler1 (void);
void      TC3X_IO_HISR_SPI_Handler1 (void);

#if defined (STDDEF_DVBH)
void      TC3X_IO_LISR_5mTimer_Handler0 (void);
void      TC3X_IO_LISR_5mTimer_Handler1 (void);
void      TC3X_IO_HISR_5mTimer_Handler0 (void);
void      TC3X_IO_HISR_5mTimer_Handler1 (void);
#endif // STDDEF_DVBH
void      TC3X_IO_SDIO_INT_EN (int moduleidx);
void      TC3X_IO_SDIO_INT_Ctrl (int moduleidx, int command);
void     *TC3X_IO_malloc (unsigned int size);
void     *TC3X_IO_memset (void *p, int init, int size);
void     *TC3X_IO_memcpy (void *des, void *src, int size);
int       TC3X_IO_free (void *p);

#endif
