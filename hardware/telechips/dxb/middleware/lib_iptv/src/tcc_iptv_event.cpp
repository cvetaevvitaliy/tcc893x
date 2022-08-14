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

#define LOG_NDEBUG 0
#define LOG_TAG	"DVB_Event"


#include <utils/Log.h>
#include <cutils/properties.h>

#include "tcc_msg.h"
#include "tcc_iptv_proc.h"
#include "tcc_iptv_event.h"

#define   DEBUG_MSG(msg...)  ALOGD(msg)

typedef    void (*pEventFunc)(void *arg);
typedef struct
{
    pEventFunc pNotifyFunc; 
    unsigned int uiFlgFreeArg; //0:don't free *arg, 1:free *arg
    void *arg; //argument
}ST_DXBPROC_EVENT;
static TccMessage *gpDxBProcEvent;

void notify_recording_complete(void);
void notify_sendsi_data (TCC_IPTV_SI_DEC_CMD_t *pSIData_Cmd);
void notify_video_frame_output_start(void *pResult);


int TCCDxBEvent_Init(void)
{
    DEBUG_MSG("%s::%d",__func__, __LINE__);
    gpDxBProcEvent = new TccMessage(100);
    return 0;
}

int TCCDxBEvent_DeInit(void)
{
    unsigned int i, uielement;
	ST_DXBPROC_EVENT *p;
	uielement = gpDxBProcEvent->TccMessageGetCount();
	for(i=0;i<uielement;i++)
	{
		p =(ST_DXBPROC_EVENT *)gpDxBProcEvent->TccMessageGet();
		if(p->arg && p->uiFlgFreeArg)
            delete[] p->arg;
        delete p;			
	}
    delete gpDxBProcEvent;
    DEBUG_MSG("%s::%d",__func__, __LINE__);
    return 0;
}

unsigned int TCCDxBEvent_FirstFrameDisplayed(void *arg)
{
    ST_DXBPROC_EVENT *pEvent;
    DEBUG_MSG("%s::%d::[%d]",__func__, __LINE__, (int)arg);
    pEvent = new ST_DXBPROC_EVENT;
    pEvent->pNotifyFunc = (pEventFunc)notify_video_frame_output_start;
    pEvent->arg = NULL;
    pEvent->uiFlgFreeArg = 0;
    gpDxBProcEvent->TccMessagePut((void *)pEvent);
    return 0;
}


unsigned int TCCDxBEvent_SendSIData(void *arg, int arg_type)
{
	ST_DXBPROC_EVENT *pEvent;
	TCC_IPTV_SI_DEC_CMD_t *pSIData_Cmd = new TCC_IPTV_SI_DEC_CMD_t;

	memset(pSIData_Cmd, 0, sizeof(TCC_IPTV_SI_DEC_CMD_t));
	memcpy(pSIData_Cmd->SIData, arg, 1024);
	pSIData_Cmd->SIDataLength = 1024;
	pSIData_Cmd->SIDataType = arg_type;

	pEvent = new ST_DXBPROC_EVENT;
	pEvent->pNotifyFunc = (pEventFunc)notify_sendsi_data;
	pEvent->arg = pSIData_Cmd;
	pEvent->uiFlgFreeArg = true;

	gpDxBProcEvent->TccMessagePut((void *)pEvent);
	return 0;
}

unsigned int TCCDxBEvent_RecordingDone(void *arg)
{
    ST_DXBPROC_EVENT *pEvent;
    pEvent = new ST_DXBPROC_EVENT;
    pEvent->pNotifyFunc = (pEventFunc)notify_recording_complete;
    pEvent->arg = arg;
	pEvent->uiFlgFreeArg = false;
    gpDxBProcEvent->TccMessagePut((void *)pEvent);
    return 0;
}

unsigned int TCCDxBEvent_Process(void)
{
    int ret;
    ST_DXBPROC_EVENT *pEvent;
    ret = gpDxBProcEvent->TccMessageGetCount();
    if(ret)
    {
        pEvent =(ST_DXBPROC_EVENT *)gpDxBProcEvent->TccMessageGet();
        if(pEvent)
        {
            pEvent->pNotifyFunc(pEvent->arg);
            if(pEvent->arg && pEvent->uiFlgFreeArg)
                delete[]  pEvent->arg;
            delete pEvent;
        }
    }
    return ret;
}
