/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions 
andlimitations under the License.

****************************************************************************/

/****************************************************************************

Revision History

****************************************************************************

****************************************************************************/
#ifndef		_TCC_DXB_THREAD_H_
#define		_TCC_DXB_THREAD_H_

#ifdef __cplusplus
extern    "C"
{
#endif

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
}E_TCC_DXB_THREAD_PRIORITY;


typedef	void (*pThreadFunc)(void *);
int tcc_dxb_thread_create(void *pHandle, pThreadFunc Func, unsigned char *pucName, E_TCC_DXB_THREAD_PRIORITY ePriority, void *arg);
int tcc_dxb_thread_join(void *pHandle, void **pThreadRet);

#ifdef __cplusplus
}
#endif

#endif
