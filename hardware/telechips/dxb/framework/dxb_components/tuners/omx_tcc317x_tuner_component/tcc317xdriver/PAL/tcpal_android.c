/****************************************************************************
 *   FileName    : Tcpal_android.c
 *   Description : OS glue Function
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-
distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall 
constitute any express or implied warranty of any kind, including without limitation, any warranty 
of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright 
or other third party intellectual property right. No warranty is made, express or implied, 
regarding the information's accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of 
or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement 
between Telechips and Company.
*
****************************************************************************/

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <utils/Log.h>
#include <semaphore.h>
#include <poll.h>
#include <errno.h>

#include "tcc317x_common.h"

/* Telechips Android (Android Platform Layer) */
I08S Tcc317xDebugStr[1024];

/* For Debug Print */
I32S TcpalPrintLog(const I08S * _fmt, ...)
{
	va_list ap;

	va_start(ap, _fmt);
	vsprintf(Tcc317xDebugStr, _fmt, ap);
	va_end(ap);

	ALOGD("%s", Tcc317xDebugStr);
	return TCC317X_RETURN_SUCCESS;
}

I32S TcpalPrintErr(const I08S * _fmt, ...)
{
	va_list ap;

	va_start(ap, _fmt);
	vsprintf(Tcc317xDebugStr, _fmt, ap);
	va_end(ap);

	ALOGD("%s", Tcc317xDebugStr);
	return TCC317X_RETURN_SUCCESS;
}

I32S TcpalPrintStatus(const I08S * _fmt, ...)
{
	va_list ap;

	va_start(ap, _fmt);
	vsprintf(Tcc317xDebugStr, _fmt, ap);
	va_end(ap);

	ALOGD("%s", Tcc317xDebugStr);
	return TCC317X_RETURN_SUCCESS;
}

/* For TimeCheck */
#define MAX_TIMECNT 0xFFFFFFFFFFFFFFFFLL
TcpalTime_t TcpalGetCurrentTimeCount_ms(void)
{
	TcpalTime_t tickcount = 0;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	tickcount = (long long) tv.tv_sec * 1000 + tv.tv_usec / 1000;
	return tickcount;
}

TcpalTime_t TcpalGetTimeIntervalCount_ms(TcpalTime_t _startTimeCount)
{
	TcpalTime_t count = 0;

	if (TcpalGetCurrentTimeCount_ms() >= _startTimeCount)
		count = TcpalGetCurrentTimeCount_ms() - _startTimeCount;
	else
		count =
		    ((MAX_TIMECNT - _startTimeCount) +
		     TcpalGetCurrentTimeCount_ms() + 1);
	return count;
}

/* for sleep */
void TcpalSleep(I32S _ms)
{
	usleep(_ms * 1000);
}

void TcpaluSleep(I32S _us)
{
	usleep(_us);
}

/* for memory allocation, free, set */
void *TcpalMalloc(I32U _size)
{
	void *ptr = NULL;

	if (!_size)
		ptr = NULL;
	else
		ptr = malloc(_size);

	return ptr;
}

I32S TcpalFree(void *_ptr)
{
	I32S error;
	error = TCC317X_RETURN_SUCCESS;

	if (_ptr == NULL) {
		error = TCC317X_RETURN_FAIL_NULL_ACCESS;
	} else {
		free(_ptr);
		_ptr = NULL;
	}
	return TCC317X_RETURN_SUCCESS;
}

void *TcpalMemset(void *_dest, I32U _data, I32U _cnt)
{
	void *ptr = NULL;
	if (_dest == NULL)
		ptr = NULL;
	else
		ptr = memset(_dest, _data, _cnt);
	return ptr;
}

void *TcpalMemcpy(void *_dest, const void *_src, I32U _cnt)
{
	void *ptr = NULL;
	if ((_dest == NULL) || (_src == NULL))
		ptr = NULL;
	else
		ptr = memcpy(_dest, _src, _cnt);
	return ptr;
}

/* For Semaphore */
I32S TcpalCreateSemaphore(TcpalSemaphore_t * _semaphore, I08S * _name,
			  I32U _initialCount)
{
	sem_t *sem;
	sem = (sem_t *) TcpalMalloc(sizeof(sem_t));
	if (sem)
		sem_init(sem, 0, _initialCount);
	else
		return TCC317X_RETURN_FAIL_NULL_ACCESS;

	_semaphore[0] = (TcpalSemaphore_t) (sem);
	return TCC317X_RETURN_SUCCESS;
}

I32S TcpalDeleteSemaphore(TcpalSemaphore_t * _semaphore)
{
	sem_t *sem;
	sem = (sem_t *)(_semaphore[0]);
	sem_destroy(sem);
	TcpalFree(sem);
	return TCC317X_RETURN_SUCCESS;
}

I32S TcpalSemaphoreLock(TcpalSemaphore_t * _semaphore)
{
	sem_t *sem;
	sem = (sem_t *)(_semaphore[0]);
	sem_wait(sem);
	return TCC317X_RETURN_SUCCESS;
}

I32S TcpalSemaphoreUnLock(TcpalSemaphore_t * _semaphore)
{
	sem_t *sem;
	sem = (sem_t *)(_semaphore[0]);
	sem_post(sem);
	return TCC317X_RETURN_SUCCESS;
}
