/*
 * Copyright (C) 2012 Telechips, Inc.  All rights reserved.
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

//#define LOG_NDEBUG 0
#define  LOG_TAG    "libdtvdump"

#include <utils/Log.h>
#include <cutils/properties.h>

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <pthread.h>

#include "OMX_Core.h"
#include "OMX_Component.h"
#include "OMX_Types.h"
#include "OMX_TCC_Index.h"
#include "TCC_Util.h"
#include "linuxtv_utils.h"
#include "dtvdump.h"
#include "tcc_socket_util.h"
#include "tcc_rtp.h"

#if 0
#define PRINTF(msg...)	ALOGI(msg)
#else
#define PRINTF(msg...)	while(0);
#endif

#define TSIF_PACKETSIZE			(188)
#define TSIF_MAX_PACKETCOUNT	(8190)
#define TSIF_INT_PACKETCOUNT	(39)
#define	STREAM_BUFFER_SIZE		(TSIF_PACKETSIZE*50000)

#define SOCKET_READ_SIZE			(1316*64)
#define	SOCKET_BUFFER_SIZE		(SOCKET_READ_SIZE*128)

typedef	struct
{
	FILE *pFP;
	unsigned char *pucData;
	unsigned int uiDataSize;
	unsigned int isWorking;
	unsigned int isRunningThread;	
}DUMP_ARG;

static OMX_COMPONENTTYPE *gpOpenmaxStandComp = NULL;
static int giDxbType = 0;      //0: DVBT, 1: ISDBT
static int giBaseBandType = 1; //1:dibcom, 2:tcc351x_cspi_sts, 3:tcc351x_i2c_sts, 4:mxl101sf, 5:mn88472
static int giDumpState = 0;
static pthread_t giThreadID = -1;
static int giDumpSizeKbyte = 0;
static long long giWriteSizeKbyte = 0;

static TCSOCK_HANDLE gpSockHandleAV;

//DVBT
OMX_ERRORTYPE omx_linuxtv_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_linuxtv_tuner_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_tcc351x_CSPI_STS_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_tcc351x_I2C_STS_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_tcc351x_tuner_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_mxl101sf_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_mxl101sf_tuner_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_mn88472_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_mn88472_tuner_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp);
//ISDBT
OMX_ERRORTYPE omx_tcc353x_CSPI_STS_tuner_component_Init (OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_tcc353x_I2C_STS_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_tcc353x_CSPI_STS_tuner_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp);
//ATSC
OMX_ERRORTYPE omx_atsc_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_atsc_tuner_component_DeInit(OMX_COMPONENTTYPE *openmaxStandComp);

long long dtvdump_getsystemtime(void)
{
	long long systime;
	struct timeval tv;

	gettimeofday (&tv, NULL);
	systime = (long long) tv.tv_sec * 1000 + tv.tv_usec / 1000;
	return systime;
}

static OMX_ERRORTYPE DVBTTunerInit(OMX_COMPONENTTYPE *phandle)
{
    char    value[PROPERTY_VALUE_MAX];
    unsigned int uiLen=0, uiData = 0;
    uiLen = property_get ("tcc.dxb.dvbt.baseband", value, "");
   	if(uiLen)
	{
		uiData = atoi (value);
		PRINTF("property_get[tcc.dxb.dvbt.baseband]::%d", uiData);
		giBaseBandType = uiData; 
    }

	switch(giBaseBandType)
    {
    case 1:
        return omx_linuxtv_tuner_component_Init(phandle);
    case 2:
       	return omx_tcc351x_CSPI_STS_tuner_component_Init(phandle);
    case 3:
       	return omx_tcc351x_I2C_STS_tuner_component_Init(phandle);
    case 4:
        return omx_mxl101sf_tuner_component_Init(phandle);
    case 5:
        return omx_mn88472_tuner_component_Init(phandle);        
    }
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE DVBTTunerDeInit(OMX_COMPONENTTYPE *phandle)
{
    switch(giBaseBandType)
    {
    case 1:
        return omx_linuxtv_tuner_component_Deinit(phandle);
    case 2:    
    case 3:
       	return omx_tcc351x_tuner_component_Deinit(phandle);
    case 4:
        return omx_mxl101sf_tuner_component_Deinit(phandle);
    case 5:
        return omx_mn88472_tuner_component_Deinit(phandle);
    }
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE ISDBTTunerInit(OMX_COMPONENTTYPE *phandle)
{	
	OMX_ERRORTYPE eError = OMX_ErrorUndefined;
	int err = omx_tcc353x_CSPI_STS_tuner_component_Init(phandle);
	int iARGS[5];
	iARGS[0] = 0;
	iARGS[1] = 1;
	iARGS[2] = 2;
	eError = OMX_SetParameter(gpOpenmaxStandComp, (OMX_INDEXTYPE)OMX_IndexVendorParamTunerOpen, iARGS);

	if(eError != OMX_ErrorNone) 
	{
		giBaseBandType = 3;
		eError = OMX_SetParameter(gpOpenmaxStandComp, (OMX_INDEXTYPE)OMX_IndexVendorParamTunerClose, NULL);
		err = omx_tcc353x_CSPI_STS_tuner_component_Deinit(phandle);
		return omx_tcc353x_I2C_STS_tuner_component_Init(phandle);
	}
	else if(eError == OMX_ErrorNone)
	{	
		giBaseBandType = 2;
		eError = OMX_SetParameter(gpOpenmaxStandComp, (OMX_INDEXTYPE)OMX_IndexVendorParamTunerClose, NULL);
		err = omx_tcc353x_CSPI_STS_tuner_component_Deinit(phandle);
	return omx_tcc353x_CSPI_STS_tuner_component_Init(phandle);	
}
	else
	{
		return OMX_ErrorUndefined;
	}
	return OMX_ErrorNone;
}

static OMX_ERRORTYPE ISDBTTunerDeInit(OMX_COMPONENTTYPE *phandle)
{
	return omx_tcc353x_CSPI_STS_tuner_component_Deinit(phandle);
}

static OMX_ERRORTYPE ATSCTunerInit(OMX_COMPONENTTYPE *phandle)
{
	OMX_ERRORTYPE eError = OMX_ErrorUndefined;

	eError = omx_atsc_tuner_component_Init(phandle);
	
	return eError;
}

static OMX_ERRORTYPE ATSCTunerDeInit(OMX_COMPONENTTYPE *phandle)
{
	OMX_ERRORTYPE eError = OMX_ErrorUndefined;

	eError = omx_atsc_tuner_component_DeInit(phandle);
	
	return eError;
}

static void *WriteThread(void *arg)
{
	DUMP_ARG *pArg;
	pArg = (DUMP_ARG *)arg;
	pArg->isRunningThread = 1;
	pArg->isWorking = 0;
	int write_ret;
	long long write_strtime, write_endtime;
	
	while (pArg->isRunningThread)
	{
		if (pArg->isWorking)
		{
			//	PRINTF("%s:S[%p]\n", __func__, pArg->pucData);
			write_strtime = dtvdump_getsystemtime();
			write_ret = fwrite(pArg->pucData, 1, pArg->uiDataSize, pArg->pFP);
			write_endtime = dtvdump_getsystemtime();
			//	PRINTF("%s:E[%p]\n", __func__, pArg->pucData);
			ALOGE("Write (%d)bytes (%d)ms ret(%d) \n", pArg->uiDataSize, (int)(write_endtime - write_strtime), write_ret);
    		giWriteSizeKbyte += pArg->uiDataSize;
			pArg->isWorking = 0;
		}
		else
		{
			usleep(50*1000);
		}
	}

	pArg->isWorking = 0;
	return NULL;
}

static void *ReadThread(void *arg)
{
	FILE *pfp = (FILE *)arg;
	pthread_t thread_id = 0;
	unsigned char *pucMSCDMABuf, *pucBuf;
	DUMP_ARG stArg;
	int ret, bufferpos, iTotalSaveSize, i, iReadSize, WaitCount;
	int streamBufferSize = STREAM_BUFFER_SIZE;
	struct timeval nowTime, afterTime;

	pucMSCDMABuf = (unsigned char *)malloc(STREAM_BUFFER_SIZE*2 + TSIF_PACKETSIZE); //*2 dubble buffer
	if( NULL == pucMSCDMABuf )
	{	
		PRINTF("error malloc\n");
		goto _error_return_;
	}

	ret = pthread_create(&thread_id, NULL, WriteThread, &stArg);
	if(ret != 0)
	{
		PRINTF("cann't create thread\n");			
		goto _error_return_;
	}
	PRINTF("WriteThread's thread_id = %d \n", thread_id);

	PRINTF("\nDump Started.....\n");

	iTotalSaveSize = 0;
	bufferpos = 0;
	iReadSize = 0;

	memset(&stArg, 0x00, sizeof(DUMP_ARG));	
	stArg.isRunningThread = 1;
	while(giDumpState == 1)
	{		
		gettimeofday(&nowTime, NULL);
		for(i=0; i<streamBufferSize; )
		{
			pucBuf = pucMSCDMABuf+bufferpos*streamBufferSize+i;
			ret = linuxtv_read(pucBuf, TSIF_PACKETSIZE);
			gettimeofday(&afterTime, NULL);
			//ALOGE("[ReadThread] ret = %d, index = %d, time = %d\n", ret, i, (int)(afterTime.tv_sec - nowTime.tv_sec));
			if(i && (int)(afterTime.tv_sec - nowTime.tv_sec) > 10)
				streamBufferSize = i;
			if( ret > 0 )
			    i += TSIF_PACKETSIZE;
			if (giDumpSizeKbyte && ((iTotalSaveSize + i)/1024 >= giDumpSizeKbyte))
				break;
			if(giDumpState != 1)
				break;
		}
		iReadSize = i;

		// Save pucMSCDMABuf+bufferpos*STREAM_BUFFER_SIZE
		PRINTF("now [%dKB] to [%dKB]\n", iTotalSaveSize/1024, giDumpSizeKbyte);

		ALOGE("Read Data (%d)Bytes\n", iReadSize);
		
		WaitCount = 0;
		while(1)
		{
			if(stArg.isWorking)
			{
				//PRINTF("Waiting Until thread close !!!\n");
				//PRINTF("W\n");
				usleep(5000);
				++WaitCount;
			}	
			else
				break;
		}

		if (WaitCount)
		{
			ALOGE("WaitCount = %dmS \n", WaitCount*5);
		}
		
		stArg.pFP = pfp;
		stArg.pucData = pucMSCDMABuf+bufferpos*streamBufferSize;
		stArg.uiDataSize = iReadSize;			
		stArg.isWorking = 1;
		
		iTotalSaveSize += iReadSize;		
		if(giDumpSizeKbyte && (iTotalSaveSize/1024)  >= giDumpSizeKbyte)
		{
			PRINTF("\nDump Finished.....\n");
			usleep(5000000);
			break;
		}
		bufferpos++;
		bufferpos = bufferpos&0x1;		
	}

_error_return_:

	if (thread_id)
	{
		stArg.isRunningThread = 0;
		pthread_join(thread_id, NULL);
		fclose(pfp);
	}

	if(pucMSCDMABuf)
		free(pucMSCDMABuf);

	linuxtv_close();

	giDumpState = 0;

	return NULL;
}

static void *SockReadThread(void *arg)
{
	FILE *pfp = (FILE *)arg;
	pthread_t thread_id = 0;
	unsigned char *pucMSCDMABuf, *pucBuf, *pucPayload;
	unsigned int uiPayloadSize = 0;
	DUMP_ARG stArg;
	int ret, bufferpos, iTotalSaveSize, i, iReadSize, WaitCount, data_readsize;

	pucMSCDMABuf = (unsigned char *)malloc(SOCKET_BUFFER_SIZE*2 + SOCKET_READ_SIZE);
	if( NULL == pucMSCDMABuf )
	{	
		PRINTF("error malloc\n");
		goto _sock_error_return_;
	}

	ret = pthread_create(&thread_id, NULL, WriteThread, &stArg);
	if(ret != 0)
	{
		PRINTF("cann't create thread\n");			
		goto _sock_error_return_;
	}
	PRINTF("WriteThread's thread_id = %d \n", thread_id);

	PRINTF("\nDump Started.....\n");

	iTotalSaveSize = 0;
	bufferpos = 0;
	iReadSize = 0;

	memset(&stArg, 0x00, sizeof(DUMP_ARG));	
	stArg.isRunningThread = 1;
	while(giDumpState == 1)
	{		
		for(i=0; i<SOCKET_BUFFER_SIZE; )
		{
			pucBuf = pucMSCDMABuf+bufferpos*SOCKET_BUFFER_SIZE+i;
			data_readsize = TCSOCKUTIL_Read(gpSockHandleAV, pucBuf, SOCKET_READ_SIZE, 500);
			if(giDumpState != 1)
				break;
			if (giDumpSizeKbyte && ((iTotalSaveSize + i)/1024 >= giDumpSizeKbyte))
				break;
			if(data_readsize > 0) {
				ret = TCRTP_Parse(pucBuf, data_readsize, &pucPayload, &uiPayloadSize);
				if(ret < 0) {
					PRINTF("Received Packet Error!!! %d\n", ret);
				}
				else {
					i += uiPayloadSize;
				}
			}
		}
		iReadSize = i;

		// Save pucMSCDMABuf+bufferpos*STREAM_BUFFER_SIZE
		PRINTF("now [%dKB] to [%dKB]\n", iTotalSaveSize/1024, giDumpSizeKbyte);

		ALOGE("Read Data (%d)Bytes\n", iReadSize);
		
		WaitCount = 0;
		while(1)
		{
			if(stArg.isWorking)
			{
				//PRINTF("Waiting Until thread close !!!\n");
				//PRINTF("W\n");
				usleep(5000);
				++WaitCount;
			}	
			else
				break;
		}

		if (WaitCount)
		{
			ALOGE("WaitCount = %dmS \n", WaitCount*5);
		}
		
		stArg.pFP = pfp;
		stArg.pucData = pucMSCDMABuf+bufferpos*SOCKET_BUFFER_SIZE;
		stArg.uiDataSize = iReadSize;			
		stArg.isWorking = 1;
		
		iTotalSaveSize += iReadSize;		
		if(giDumpSizeKbyte && (iTotalSaveSize/1024)  >= giDumpSizeKbyte)
		{
			PRINTF("\nDump Finished.....\n");
			break;
		}
		bufferpos++;
		bufferpos = bufferpos&0x1;		
	}

_sock_error_return_:

	if (thread_id)
	{
		stArg.isRunningThread = 0;
		pthread_join(thread_id, NULL);
		fclose(pfp);
	}

	if(pucMSCDMABuf)
		free(pucMSCDMABuf);

	TCSOCKUTIL_Close(gpSockHandleAV);

	giDumpState = 0;

	return NULL;
}

int DxbDumpInit(int iDxbType, int iFreq, int iBW, int iVoltage, int iMOD)
{
#ifndef     DUMP_FROM_DMX1    
	OMX_ERRORTYPE eError = OMX_ErrorUndefined;
	int iARGS[5];

	if (gpOpenmaxStandComp)
	{
		DxbDumpDeinit();
	}

	gpOpenmaxStandComp = (OMX_COMPONENTTYPE *)calloc(1,sizeof(OMX_COMPONENTTYPE));
	if (gpOpenmaxStandComp == NULL)
	{
		return -1;
	}

	system("echo performance>/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");

	giDxbType = iDxbType;
	switch(giDxbType)
	{
	case 3: // ATSC
		eError = ATSCTunerInit(gpOpenmaxStandComp);
		break;

	case 1: // ISDBT
		eError = ISDBTTunerInit(gpOpenmaxStandComp);
		break;

	case 0: // DVBT
	case 2: // DVBS
		eError = DVBTTunerInit(gpOpenmaxStandComp);
		break;

	default:
		return -2;		
	}

	if (eError != OMX_ErrorNone)
	{
		DxbDumpDeinit();
		return -2;
	}

	/* Tuner Open */
	if (giDxbType == 0 || giDxbType == 1 || giDxbType == 2)
	{
		iARGS[0] = 0;
		iARGS[1] = iDxbType; // 0:DVBT 1:ISDBT
		iARGS[2] = giBaseBandType;
		eError = OMX_SetParameter(gpOpenmaxStandComp, (OMX_INDEXTYPE)OMX_IndexVendorParamTunerOpen, iARGS);
		if(eError != OMX_ErrorNone) 
		{
			PRINTF("error in OMX_IndexVendorParamTunerOpen\n");
			DxbDumpDeinit();
			return -3;
		}
	}	
	else if (giDxbType == 3)
	{
		iARGS[0] = 0;
		iARGS[1] = iDxbType; // 2:ATSC
		iARGS[2] = giBaseBandType;
		eError = OMX_SetParameter(gpOpenmaxStandComp, (OMX_INDEXTYPE)OMX_IndexVendorParamTunerOpen, iARGS);
		if(eError != OMX_ErrorNone) 
		{
			PRINTF("error in OMX_IndexVendorParamTunerOpen\n");
			DxbDumpDeinit();
			return -3;
		}
	}

	/* Set Voltage */
	if (giDxbType == 0 || giDxbType == 1 || giDxbType == 2)
	{
		if (giBaseBandType == 1)
		{
			eError = OMX_SetParameter(gpOpenmaxStandComp, OMX_IndexVendorParamSetVoltage, (OMX_PTR)iVoltage);
			if(eError != OMX_ErrorNone)
			{
				PRINTF("error in OMX_IndexVendorParamSetVoltage\n");
			}
		}
	}

	/* TunerFrequencySet */
	if (giDxbType == 0 || giDxbType == 1 || giDxbType == 2)
	{
		iARGS[0] = iFreq;
		if (giBaseBandType == 1)
			iARGS[1] = iBW * 1000;	// BandWidth
		else
			iARGS[1] = iBW;			// BandWidth
		iARGS[2] = 1; //Lock wait, 1:wait till locking, 0:skip locking
		iARGS[3] = 0; //System, in DVBT 0:DVBT, 1:DVBT2
		eError = OMX_SetParameter(gpOpenmaxStandComp, (OMX_INDEXTYPE)OMX_IndexVendorParamTunerFrequencySet, iARGS);
		if(eError != OMX_ErrorNone) 
		{
			PRINTF("error in OMX_IndexVendorParamTunerFrequencySet\n");
			DxbDumpDeinit();
			return -4;
		}
	}
	else if (giDxbType == 3)
	{
		iARGS[0] = iFreq;
		iARGS[1] = iMOD;	// Modulation
		iARGS[2] = 0;
		iARGS[3] = 0;
		eError = OMX_SetParameter(gpOpenmaxStandComp, (OMX_INDEXTYPE)OMX_IndexVendorParamTunerFrequencySet, iARGS);
		if(eError != OMX_ErrorNone) 
		{
			PRINTF("error in OMX_IndexVendorParamTunerFrequencySet\n");
			DxbDumpDeinit();
			return -4;
		}
	}
#endif	
	return 0;
}

int DxbDumpDeinit()
{
#ifndef     DUMP_FROM_DMX1    
	if(gpOpenmaxStandComp)
	{
		if (giDumpState != 0)
		{
			DxbDumpStop();
		}

		OMX_SetParameter(gpOpenmaxStandComp, (OMX_INDEXTYPE)OMX_IndexVendorParamTunerClose, NULL);

		switch(giDxbType)
		{
		case 3: // ATSC
			ATSCTunerDeInit(gpOpenmaxStandComp);
			break;

		case 1: // ISDBT
			ISDBTTunerDeInit(gpOpenmaxStandComp);
			break;

		case 0: // DVBT
		case 2: // DVBS
		default:
			DVBTTunerDeInit(gpOpenmaxStandComp);
			break;
		}
		free(gpOpenmaxStandComp);
		gpOpenmaxStandComp = NULL;
		system("echo interactive>/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
	}
#endif
	return 0;
}

int DxbDumpStart(int iPidNum, int *piPidTable, const char *pcFile, int iDumpSize)
{
	FILE *pfp;

	giDumpState = 0;
	giWriteSizeKbyte = 0;
	giDumpSizeKbyte = iDumpSize;
#ifdef     DUMP_FROM_DMX1    
	if (giDumpState == 0 && iPidNum > 0)
#else
    if (gpOpenmaxStandComp && giDumpState == 0 && iPidNum > 0)        
#endif        
	{
		pfp = fopen(pcFile, "wb");
		if (pfp)
		{
			if (linuxtv_init(piPidTable, iPidNum) == 0)
			{
			    if (pthread_create(&giThreadID, NULL, ReadThread, pfp) != 0)
			    {
				    giDumpState = 0;
				    linuxtv_close();
				    fclose(pfp);
			    }
				else
				{
					giDumpState = 1;
				}
			}
		}
	}
	return (giDumpState == 1) ? 0 : -1;
}

int SockDumpStart(const char * iIp, int iPort, const char *pcFile, int iDumpSize)
{
	FILE *pfp;

	giDumpState = 0;
	giWriteSizeKbyte = 0;
	giDumpSizeKbyte = iDumpSize;
	if (giDumpState == 0) {
		pfp = fopen(pcFile, "wb");
		if (pfp)
		{
			gpSockHandleAV = TCSOCKUTIL_Open(iIp, iPort);
			if(gpSockHandleAV == NULL) {
				PRINTF("SOCKET Open Error !!!\n");
				DxbDumpStop();
				fclose(pfp);
				return -1;
			}
			if (pthread_create(&giThreadID, NULL, SockReadThread, pfp) != 0) 
			{
				giDumpState = 0;
				if(gpSockHandleAV != NULL)
					TCSOCKUTIL_Close(gpSockHandleAV);
				DxbDumpStop();
				fclose(pfp);
			}
			else {
				giDumpState = 1;
			}
		}
	}
	return (giDumpState == 1) ? 0 : -1;
}

int DxbDumpStop()
{
	giDumpState = 2;
	pthread_join(giThreadID, NULL);

	return 0;
}

int GetDumpSize()
{
	return (int)(giWriteSizeKbyte / 1024);
}

int GetDiskFreeSpace(const char *pcPath)
{
	MOUNTP *pMP;
	MOUNTP *pDiskStat = dfopen();
	int iFreeSize = 0;

	if (pDiskStat)
	{
		pMP = dfgetex(pDiskStat, (unsigned char *)pcPath);
		if (pMP)
			iFreeSize = pDiskStat->size.avail;//*1024;
		dfclose(pDiskStat);
	}
	return iFreeSize;	
}
