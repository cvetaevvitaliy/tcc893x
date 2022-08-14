/**

  Copyright (C) 2009-2010 Telechips Inc.

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA
  02110-1301  USA

*/

/****************************************************************************
*   FileName    : omx_tcc_thread.h
*   Description : 
****************************************************************************
*
*   TCC Version 1.0
*   Copyright (c) Telechips, Inc.
*   ALL RIGHTS RESERVED
*
****************************************************************************/

/****************************************************************************

Revision History

****************************************************************************

****************************************************************************/
#ifndef		_OMX_TCC_THREAD_H_
#define		_OMX_TCC_THREAD_H_

typedef enum
{
	HIGH_PRIORITY_0 = 20,	//highest
	HIGH_PRIORITY_1,
	HIGH_PRIORITY_2,
	HIGH_PRIORITY_3,
	HIGH_PRIORITY_4,
	HIGH_PRIORITY_5,
	HIGH_PRIORITY_6,
	HIGH_PRIORITY_7,
	HIGH_PRIORITY_8,
	HIGH_PRIORITY_9,
	HIGH_PRIORITY_10,
	LOW_PRIORITY_0,
	LOW_PRIORITY_1,
	LOW_PRIORITY_2,
	LOW_PRIORITY_3,
	LOW_PRIORITY_4,
	LOW_PRIORITY_5,
	LOW_PRIORITY_6,
	LOW_PRIORITY_7,
	LOW_PRIORITY_8,
	LOW_PRIORITY_9,
	LOW_PRIORITY_10,
	LOW_PRIORITY_11,		//lowest
	PREDEF_PRIORITY			//determine by name
}E_TCCTHREAD_PRIORITY;


typedef	void (*pThreadFunc)(void *);
int TCCTHREAD_Create(void *pHandle, pThreadFunc Func, char *pcName, E_TCCTHREAD_PRIORITY ePriority, void *arg);
int TCCTHREAD_Join(void *pHandle, void **pThreadRet);
#endif
