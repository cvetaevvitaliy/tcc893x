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
#define LOG_TAG	"TCC_THREAD"

#include <pthread.h>
#include <stdlib.h>
#include <utils/Log.h>
#include "tcc_dxb_thread.h"

//#define	 NOPRIORITY_SUPPORT
#define	 DBG_PRINTF		ALOGD	

typedef struct TCC_DXB_THREAD_COUSTOM_DEF
{
	unsigned char *pucname;
	int sched_type;
	E_TCC_DXB_THREAD_PRIORITY epriority;
} ST_TCC_DXB_THREAD_COUSTOM_DEF;


ST_TCC_DXB_THREAD_COUSTOM_DEF stTCCThreadTable[] = 
{
	{	"default"      ,					SCHED_RR,	LOW_PRIORITY_11		},		
	{	"OMX.tcc.broadcast.tuner.atsc",		SCHED_RR,	LOW_PRIORITY_11		},
	{	"OMX.tcc.broadcast.dvbt",			SCHED_RR,	LOW_PRIORITY_11		},
	{	"OMX.tcc.broadcast.isdbt",			SCHED_RR,	LOW_PRIORITY_11		},	
	{	"OMX.tcc.broadcast.demux.atsc",		SCHED_FIFO,	HIGH_PRIORITY_0		},
	{	"OMX.tcc.broadcast.dxb_demux",		SCHED_FIFO,	HIGH_PRIORITY_0		},
	{	"OMX.tcc.broadcast.iptv_demux",		SCHED_FIFO,	HIGH_PRIORITY_0		},
	{	"OMX.tcc.broadcast.section",		SCHED_RR,	HIGH_PRIORITY_7		},
	{	"OMX.tcc.video_decoder",			SCHED_RR,	HIGH_PRIORITY_7		},
	{	"OMX.tcc.audio_decoder",			SCHED_RR,	HIGH_PRIORITY_7		},
	{	"OMX.TCC.mpeg2dec",					SCHED_RR,	HIGH_PRIORITY_7		},
	{	"OMX.TCC.avcdec",					SCHED_RR,	HIGH_PRIORITY_7		},
	{	"OMX.TCC.mp2dec",					SCHED_RR,	HIGH_PRIORITY_7		},
	{	"OMX.TCC.aacdec",					SCHED_RR,	HIGH_PRIORITY_7		},
	{	"OMX.TCC.ac3dec",					SCHED_RR,	HIGH_PRIORITY_7		},
	{	"OMX.tcc.fbdev.fbdev_sink",			SCHED_RR,	HIGH_PRIORITY_7		},
	{	"OMX.tcc.alsa.alsasink",			SCHED_RR,	HIGH_PRIORITY_7		},
	{	NULL,								0,			0 					},
};


int tcc_dxb_thread_create(void *pHandle, pThreadFunc Func, unsigned char *pucName, E_TCC_DXB_THREAD_PRIORITY ePriority, void *arg)
{
	int i, ret = 0;
	int iSchedType, iPriorityOffset, iMaxPriority;
	struct sched_param param;
	pthread_attr_t p_tattr;
	
#ifdef		NOPRIORITY_SUPPORT
	return pthread_create(pHandle, NULL, Func,  arg);
#else

	iSchedType = -1;
	if( ePriority >= PREDEF_PRIORITY )
		ePriority = HIGH_PRIORITY_9;
	
	if(pucName)
	{		
		for( i=0; stTCCThreadTable[i].pucname != NULL; i++ )
		{
			if( !strcmp(stTCCThreadTable[i].pucname, pucName) )
			{				
				iPriorityOffset = stTCCThreadTable[i].epriority;	
				iSchedType = stTCCThreadTable[i].sched_type;	
				DBG_PRINTF("%s %d %d", pucName, ePriority, iSchedType);
				break;
			}
		}
	}
	
	if(iSchedType == -1)
	{
		if(ePriority >= LOW_PRIORITY_0 )
		{
			#if 1
			iSchedType = SCHED_OTHER;		
			//iPriorityOffset = ePriority-LOW_PRIORITY_0;		
			iPriorityOffset = 0;
			#else
			iSchedType = SCHED_RR;
			iPriorityOffset = ePriority;
			#endif
		}
		else
		{
			iSchedType = SCHED_RR;
			iPriorityOffset = ePriority;
		}	
	}	
	/* initialized with default attributes */
	ret |= pthread_attr_init (&p_tattr);
	
	//ret |= pthread_attr_setinheritsched(p_tattr, PTHREAD_EXPLICIT_SCHED);  
	ret |= pthread_attr_setscope(&p_tattr, PTHREAD_SCOPE_SYSTEM);	
	ret |= pthread_attr_setschedpolicy(&p_tattr, iSchedType);
	/* safe to get existing scheduling param */
	ret |= pthread_attr_getschedparam (&p_tattr, &param);
	
	/* set the priority; others are unchanged */
	iMaxPriority = sched_get_priority_max(iSchedType);
	if( iMaxPriority > iPriorityOffset )
		param.sched_priority = iMaxPriority - iPriorityOffset;
	else
		param.sched_priority = iMaxPriority;
	//ALOGE("MAX %d,Offset %d, Type %d", sched_get_priority_max(iSchedType), iPriorityOffset, iSchedType);
	/* setting the new scheduling param */
	ret |= pthread_attr_setschedparam (&p_tattr, &param);
	if(ret != 0)
	{
		DBG_PRINTF("%s::%d thread(%s) error %d", __func__,__LINE__, pucName, ret);
		return ret;
	}
	
	ret = pthread_create(pHandle, &p_tattr, Func,  arg);	
	if(ret != 0)
	{
		DBG_PRINTF("%s::%d thread(%s) error %d", __func__,__LINE__, pucName, ret);
		return ret;
	}
	i = strlen (pucName);
	if (i > 0) {
		char *pName = pucName;
		if (i > 15) 
			pName += (i-15);
		pthread_setname_np (*(pthread_t *)pHandle, pName);
	}
	
	DBG_PRINTF("%s::%s Success!! Priority(%d)Type(%d)", __func__, pucName, param.sched_priority, iSchedType);
	return ret;
#endif	
}

int tcc_dxb_thread_join(void *pHandle, void **pThreadRet)
{
	return pthread_join(pHandle, pThreadRet);
}
