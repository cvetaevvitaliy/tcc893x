/****************************************************************************
 *   FileName    : dumpmain.c
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
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <pthread.h>
#include "OMX_Core.h"
#include "OMX_Component.h"
#include "OMX_Types.h"
#include "OMX_TCC_Index.h"
#include <cutils/properties.h>
#include "TCC_Util.h"


#include "tcc_dxb_queue.h"
#include <sys/time.h>
#include <stdio.h>

#include <fcntl.h>         // O_WRONLY
#include <unistd.h>        // write(), close()


#define	MAX_QUEUESIZE		1000
static tcc_dxb_queue_t* DumpQueue = NULL;
static int Dump_Thread_Runing;

//#define NON_BLOCK_WRITE
#ifdef NON_BLOCK_WRITE
static int pfp;
#else
FILE *pfp;
#endif

typedef struct{
	unsigned int data_size;
	unsigned char *data;
}ISDBT_DUMP_TYPE;


#define	SUPPORT_TUNER_TCC351X


#define PRINTF(msg...)	printf(msg)

#define TSIF_PACKETSIZE			(188)
#define TSIF_MAX_PACKETCOUNT	(8190)
#define TSIF_INT_PACKETCOUNT	(39)

//#define	STREAM_BUFFER_SIZE		(TSIF_PACKETSIZE*50000)
#define	STREAM_BUFFER_SIZE		(TSIF_PACKETSIZE*1024*4)

#define TS_SYNC_WORD		0x47

static MOUNTP *dumpstat = NULL;
static unsigned char *dump_path;
	
#define TSIF_INTR_PACKET_CNT	1

typedef	struct
{
	FILE *pFP;
	unsigned char *pucData;
	unsigned int uiDataSize;
	unsigned int isWorking;
}DUMP_ARG;


extern OMX_ERRORTYPE omx_isdbt_tuner_toshiba_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
extern OMX_ERRORTYPE omx_isdbt_tuner_toshiba_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp);
extern OMX_ERRORTYPE omx_isdbt_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
extern OMX_ERRORTYPE omx_isdbt_tuner_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp);
extern OMX_ERRORTYPE omx_tcc351x_CSPI_STS_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
extern OMX_ERRORTYPE omx_tcc351x_tuner_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp);

extern int TCC_TSIF_Open(int packet_intr_cnt, int packet_dma_cnt ,int mode, int dma_mode);
extern int TCC_TSIF_DxBPower(int handle, int on_off);
extern int TCC_TSIF_SetPID(int handle, int PID);
extern int TCC_TSIF_RemovePID(int handle, int PID);
extern int TCC_TSIF_Close(int handle);
extern int TCC_TSIF_Read(int handle, unsigned char *buf, unsigned int read_size);
extern int TCC_TSIF_ReadEX(int handle, unsigned char *buf, unsigned int read_size);	
extern int TCC_TSIF_SetPCRPID(int handle, unsigned int uiPID, unsigned int index);
extern int TCC_TSIF_GetSTC(int handle, unsigned int index);

extern void* tcc_dxb_malloc (unsigned int iSize);
extern int tcc_dxb_free(void *pvPtr);
extern void* tcc_dxb_calloc (unsigned int isize_t, unsigned int iSize);

typedef struct
{
	int	(*pfn_Open)(int packet_intr_cnt, int packet_dma_cnt ,int mode, int dma_mode);
	int (*pfn_DxBPower)(int handle, int on_off);
	int	(*pfn_SetPID)(int handle, int PID);
	int	(*pfn_RemovePID)(int handle, int PID);
	int (*pfn_Close)(int handle);
	int	(*pfn_Read)(int handle, unsigned char *buf, unsigned int read_size);
	int	(*pfn_ReadEX)(int handle, unsigned char *buf, unsigned int read_size);	
	int	(*pfn_SetPCRPID)(int handle, unsigned int uiPID, unsigned int index);
	int	(*pfn_GetSTC)(int handle, unsigned int index);
}TCCTSIF_FUNC_ENTRY;

void tsif_set_funcentry(int tisf_module, TCCTSIF_FUNC_ENTRY *pstEntry)
{
	//GPSB TSIF
	pstEntry->pfn_Open = TCC_TSIF_Open;
	pstEntry->pfn_DxBPower = TCC_TSIF_DxBPower;
	pstEntry->pfn_SetPID = TCC_TSIF_SetPID;
	pstEntry->pfn_RemovePID = TCC_TSIF_RemovePID;
	pstEntry->pfn_Close = TCC_TSIF_Close;
	pstEntry->pfn_Read = TCC_TSIF_Read;
	pstEntry->pfn_ReadEX = TCC_TSIF_ReadEX;
	pstEntry->pfn_SetPCRPID = TCC_TSIF_SetPCRPID;
	pstEntry->pfn_GetSTC = TCC_TSIF_GetSTC;
}

int tuner_init(void *phandle)
{
#ifdef		SUPPORT_TUNER_TCC351X		
	return omx_tcc351x_CSPI_STS_tuner_component_Init(phandle);
#endif

}

int tuner_deinit(void *phandle)
{
#ifdef		SUPPORT_TUNER_TCC351X		
	return omx_tcc351x_tuner_component_Deinit(phandle);
#endif

}

int disk_open(void)
{
	dumpstat = dfopen();
	return 0;
}

int disk_close(void)
{
	if(dumpstat)
	{
		dfclose(dumpstat);
		dumpstat = NULL;
	}	
	return 0;
}

long long  disk_GetFreeKbyte(void)
{
	unsigned long long  free_size_kbyte;
	if(dumpstat)
	{
		if(dfgetex(dumpstat, dump_path))
		{
			free_size_kbyte = dumpstat->size.avail;
			//printf("%s[%d] %dKbyte",__func__, giRecDiskType, free_size_kbyte);
			return free_size_kbyte; //unit kbyte
		}	
	}	
	//printf("%s return 0", __func__);
	return 0;
}


int dump_PushQueue(unsigned char *pucBuf,  unsigned int ibufSize)
{
	ISDBT_DUMP_TYPE *scdumpcmd;

	scdumpcmd = (ISDBT_DUMP_TYPE *)tcc_dxb_malloc(sizeof(ISDBT_DUMP_TYPE));
	if(scdumpcmd == NULL){
		PRINTF("[%s] PushQueue allocation fail\n", __func__);
		return -1; 
	}
	
	scdumpcmd->data_size = ibufSize;
	scdumpcmd->data = tcc_dxb_malloc(ibufSize);
	memcpy(scdumpcmd->data, pucBuf, ibufSize);

	if( tcc_dxb_queue_ex(DumpQueue, scdumpcmd) == 0 )
	{
		tcc_dxb_free(scdumpcmd->data);
		tcc_dxb_free(scdumpcmd);
	}
	
	return -1;
}

void dump_DeinitQueueData(void)
{
	ISDBT_DUMP_TYPE *scdumpcmd;
	int Queue_cnt=0;

	Queue_cnt = tcc_dxb_getquenelem(DumpQueue);

//	LOGE("%s %d Queue_cnt = %d \n", __func__, __LINE__, Queue_cnt);

	if(Queue_cnt>0)
	{
		int i=0;
		for(i=0; i<Queue_cnt; i++)
		{
			scdumpcmd =  tcc_dxb_dequeue(DumpQueue);
			if(scdumpcmd)
			{
				tcc_dxb_free(scdumpcmd->data);
				tcc_dxb_free(scdumpcmd);
			}
		}
	}
}
void *dumpthread(void *arg)
{
#if 1
	ISDBT_DUMP_TYPE *scdumpcmd;
	unsigned int		ret =0;
	unsigned char 	*pucPayload;
	unsigned int 	uiPayloadSize;

	while(Dump_Thread_Runing)
	{
		if(tcc_dxb_getquenelem(DumpQueue))
		{
			scdumpcmd =  tcc_dxb_dequeue(DumpQueue);
			if(scdumpcmd)
			{
				if(pfp)
#ifdef NON_BLOCK_WRITE
					ret = write(pfp, scdumpcmd->data, scdumpcmd->data_size);
					if(ret != scdumpcmd->data_size)
						PRINTF("%s %d write error org_size= %d write_size= %d, \n", __func__, __LINE__, scdumpcmd->data_size, ret);						
#else
					ret = fwrite(scdumpcmd->data, 1, scdumpcmd->data_size, pfp);
					if(ret != scdumpcmd->data_size)
						PRINTF("%s %d write error org_size= %d write_size= %d, \n", __func__, __LINE__, scdumpcmd->data_size, ret);						
#endif

				tcc_dxb_free(scdumpcmd->data);
				tcc_dxb_free(scdumpcmd);
			}
		}
		else
			usleep(1000);
	}

	PRINTF("%s %d TCSOCKDUMPUTIL_Thread join\n", __func__, __LINE__);

	return NULL;

#else
	DUMP_ARG *pArg;
	pArg = (DUMP_ARG *)arg;
	pArg->isWorking = 1;
	fwrite(pArg->pucData, 1, pArg->uiDataSize, pArg->pFP);
	pArg->isWorking = 0;
	return NULL;
#endif
}

int main(int argc, char *argv[])
{
	DUMP_ARG stArg;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_COMPONENTTYPE *openmaxStandComp;	
	unsigned char *pucMSCDMABuf, *pucBuf;
	pthread_t thread_id;
	int iTSIFHandle;
	int iPrevCC, iCC; 
	int err=0;
	int iARGS[2];
	int iPidTable[32], i, bufferpos, iPidNum, iPID;
	unsigned char *pucFile;
	unsigned int ret, iFreq=0, iBW=0;
	int channel;
	TCCTSIF_FUNC_ENTRY stTSIFEntry;
	char    value[PROPERTY_VALUE_MAX];
	unsigned int   uiLen = 0, uiType = 0;
	int error_endicator =0;
	unsigned long long iDumpSizeKbyte, iTotalSaveSize=0;
	unsigned long long  free_size=0;
	int iStatus;
	int fd_flags;
	
	char	*USAGE	=
//		"Usage: isdbtdump frequency bandwidth dumpsize file [pid] [pid] ...\n"
		"Usage: isdbtdump channel dumpsize file [pid] [pid] ...\n"
//		"       frequency, bandwidth is Khz unit.\n"
		"       channel is channel num.\n"
		"       dumpsize is Kbyte unit.\n"
		"       if there are no pids as an input arguments, save whole ts stream.\n"
	;

	if(argc <4)
	{
		PRINTF("%s", USAGE);
		return -1;
	}	
	openmaxStandComp = (OMX_COMPONENTTYPE *)NULL;
	pucMSCDMABuf = (unsigned char *)NULL;
	iTSIFHandle = -1;
	pfp = NULL;
	thread_id = NULL;
	memset(&stArg, 0x00, sizeof(DUMP_ARG));	
	memset(iPidTable, 0xff, 32*sizeof(int));
#if 1
	channel = atoi(argv[1]);
#else
	iFreq = atoi(argv[1]);
	iBW = atoi(argv[2]);
#endif
	iDumpSizeKbyte = atoi(argv[2]);
	pucFile = (unsigned char *)argv[3];

	dump_path = pucFile;
	disk_open();

	if(argc>4)
	{
		iPidNum = argc-4;
		if(iPidNum > 32)
			iPidNum = 32;
		for(i=0;i<iPidNum;i++)
		{
			iPidTable[i] = atoi(argv[4+i]);
		}

#ifdef NON_BLOCK_WRITE
		pfp = open(pucFile , O_WRONLY | O_CREAT , 0644);
		fd_flags = fcntl(pfp, F_GETFL);

		if (fd_flags < 0) {
		         PRINTF("fcntl error");
		         close(pfp);
		         goto _error_return_;
		}

		if (-1 == fcntl(pfp, F_SETFL, fd_flags | O_NONBLOCK|O_SYNC)) 
		{
		          PRINTF("error fcntl - set non-block");
		          close(pfp);
		          goto _error_return_;
		}
#else
		pfp = fopen(pucFile,"wb");
		if(pfp == NULL)
		{
			PRINTF("CANNOT Make file [%s] !!!\n", pucFile);
			goto _error_return_;
		}
#endif
		
		PRINTF("Frequency[%dKhz] BandWidth[%dKhz] OutputPath[%s] Size[%lldKbyte] Pids[%d]\n", iFreq, iBW, pucFile, iDumpSizeKbyte, iPidNum);
		PRINTF("PIDs :: \n");
		for(i=0;i<iPidNum;i++)
		{
			PRINTF("[0x%X]", iPidTable[i]);	
		}
		PRINTF("\n");
	}
	else
	{
#ifdef NON_BLOCK_WRITE
		pfp = open(pucFile , O_WRONLY | O_CREAT , 0644);
		fd_flags = fcntl(pfp, F_GETFL);

		if (fd_flags < 0) {
		         PRINTF("fcntl error");
		         close(pfp);
		         goto _error_return_;
		}

		if (-1 == fcntl(pfp, F_SETFL, fd_flags | O_NONBLOCK)) 
		{
		          PRINTF("error fcntl - set non-block");
		          close(pfp);
		          goto _error_return_;
		}
#else
		pfp = fopen(pucFile,"wb");
		if(pfp == NULL)
		{			PRINTF("CANNOT Make file [%s] !!!\n", pucFile);
			goto _error_return_;
		}
#endif
		iPidNum =0;
	}

	openmaxStandComp = calloc(1,sizeof(OMX_COMPONENTTYPE));
	if (!openmaxStandComp) 
	{
		goto _error_return_;
	}

	tsif_set_funcentry(0, &stTSIFEntry); //GPSB module
	tuner_init(openmaxStandComp);

	pucMSCDMABuf = malloc(STREAM_BUFFER_SIZE*2 + TSIF_PACKETSIZE); //*2 dubble buffer
	if( NULL == pucMSCDMABuf )
	{	
		PRINTF("error malloc\n");
		goto _error_return_;
	}

	//BB Power ON
	iARGS[0] = 0;
	iARGS[1] = 1; // 0:DVBT 1:ISDBT
	err = OMX_SetParameter(openmaxStandComp,OMX_IndexVendorParamTunerOpen, iARGS);
	if(err != OMX_ErrorNone) 
	{
		PRINTF("error in OMX_IndexVendorParamTunerOpen\n");
		goto _error_return_;
	}	

	iTSIFHandle = stTSIFEntry.pfn_Open(TSIF_INTR_PACKET_CNT, TSIF_MAX_PACKETCOUNT, 1, 1);
	if( iTSIFHandle < 0 ){	
		PRINTF("error TCCTSIF_Open\n");
		return -1;
	}

	for(i=0;i<iPidNum;i++)
	{
		stTSIFEntry.pfn_SetPID(iTSIFHandle, iPidTable[i]);
	}

#if 1
	iARGS[0] = channel;
	iARGS[1] = 1;
	err = OMX_SetParameter(openmaxStandComp,OMX_IndexVendorParamTunerChannelSet, iARGS);
	if(err != OMX_ErrorNone) 
	{
		PRINTF("error in OMX_IndexVendorParamTunerChannelSet\n");
		goto _error_return_;
	}
#else
	iARGS[0] = iFreq;
	iARGS[1] = iBW;
	err = OMX_SetParameter(openmaxStandComp,OMX_IndexVendorParamTunerFrequencySet, iARGS);
	if(err != OMX_ErrorNone) 
	{
		PRINTF("error in OMX_IndexVendorParamTunerFrequencySet\n");
		goto _error_return_;
	}
#endif

	DumpQueue = tcc_dxb_calloc(1,sizeof(tcc_dxb_queue_t));
	tcc_dxb_queue_init_ex(DumpQueue, MAX_QUEUESIZE);	

	Dump_Thread_Runing =1;

	if (iStatus = pthread_create(&thread_id, NULL, dumpthread, NULL))
	{
		PRINTF("Error: fail to create Dump_Thread, status=%d\n", iStatus);
		goto _error_return_;
	}


	////////////////////////
	iPrevCC = -1;
	iTotalSaveSize = 0;
	PRINTF("Dump Started.....\n");
	//for(j=0;j<2;j++)
	bufferpos = 0;
	while(1)
	{			
#if 0
		ret = stTSIFEntry.pfn_Read(iTSIFHandle, pucBuf, TSIF_PACKETSIZE*TSIF_INTR_PACKET_CNT);

		if(pucBuf[0] == TS_SYNC_WORD)
		{	
			dump_PushQueue(pucBuf, ret);
			iTotalSaveSize += ret;		
		}
#else

		//PRINTF("buffer pos %d\n", bufferpos);
		for(i=0; i<STREAM_BUFFER_SIZE; i+=TSIF_PACKETSIZE*TSIF_INTR_PACKET_CNT)
		{
			if(!error_endicator)
			{
				pucBuf = pucMSCDMABuf+bufferpos*STREAM_BUFFER_SIZE+i;
			}
			
			ret = stTSIFEntry.pfn_Read(iTSIFHandle, pucBuf, TSIF_PACKETSIZE*TSIF_INTR_PACKET_CNT);
			error_endicator =0;

			if( ret == TSIF_PACKETSIZE*TSIF_INTR_PACKET_CNT)
			{
				error_endicator = (pucBuf[1] & 0x80);

				if(error_endicator == NULL && (pucBuf[0] == TS_SYNC_WORD))
				{	
					if(iPidNum == 1)
					{	
						//Check continuity error
						iPID = ((pucBuf[1] & 0x1F) << 8) | pucBuf[2];
						iCC = pucBuf[3]&0x0F;
						if(iPrevCC != -1)
						{
							//Check continuity
							if(iCC != ((iPrevCC+1)&0x0F))
				                    {
				                        PRINTF("PID[0x%X] Continuity Error %d :: %d\n", iPID, iPrevCC, iCC );
				                    }
						}
						iPrevCC = iCC;
					}
				}
				else
				{
					i-=(TSIF_PACKETSIZE*TSIF_INTR_PACKET_CNT);
	//				PRINTF(" TS Error error_endicator = %x, pucBuf[0] = %x, pucBuf[1] = %x \n", error_endicator, pucBuf[0], pucBuf[1]);
				}
			}
			else
			{
				PRINTF("Read Error %d\n", ret);
			}
		}
		/*Save pucMSCDMABuf+bufferpos*STREAM_BUFFER_SIZE
		*/
		PRINTF("@");
		if(pfp)
		{
			stArg.pFP = pfp;
			stArg.pucData = pucMSCDMABuf+bufferpos*STREAM_BUFFER_SIZE;
			stArg.uiDataSize = STREAM_BUFFER_SIZE;			
#if 1
			dump_PushQueue(stArg.pucData, stArg.uiDataSize);
#else
			while(1)
			{
				if(stArg.isWorking)
				{
					//PRINTF("Waiting Until thread close !!!\n");
					PRINTF("W");
					usleep(5000);
				}	
				else
					break;
			}
			
			stArg.isWorking = 0;
			ret = pthread_create(&thread_id, NULL, dumpthread, &stArg);
			if(ret != 0)
			{
				PRINTF("cann't create thread\n");			
				goto _error_return_;
			}
#endif
		}

		iTotalSaveSize += STREAM_BUFFER_SIZE;		

		free_size = disk_GetFreeKbyte();
		if(free_size < 10*1024) //10M
		{
			PRINTF("%s Dump Finished .. free_szie = %lldKbyte", __func__, free_size);
			break;
		}
		bufferpos++;
		bufferpos = bufferpos&0x1;		
#endif

		if( (iTotalSaveSize/1024)  > iDumpSizeKbyte)
		{
			PRINTF("\nDump Finished.....\n");
			break;
		}
//		PRINTF("Current File Szie = %lld \n", iTotalSaveSize);
	}
	Dump_Thread_Runing = NULL;

//	if(pfp)
//		pthread_join(thread_id, NULL);

	///////////////////////
	err = OMX_SetParameter(openmaxStandComp,OMX_IndexVendorParamTunerClose, NULL);

	if(thread_id != NULL)
		pthread_join(thread_id, NULL);
	thread_id = NULL;
	
	if(DumpQueue)
	{
		dump_DeinitQueueData();

		tcc_dxb_queue_deinit(DumpQueue);
		tcc_dxb_free(DumpQueue);
		DumpQueue = NULL;
	}
	
	if( iTSIFHandle >= 0 )
		stTSIFEntry.pfn_Close(iTSIFHandle);
	
	if(pfp)
#ifdef NON_BLOCK_WRITE
		close(pfp);
#else
		fclose(pfp);
#endif
	pfp = NULL;
	
	disk_close();
	sync ();
	
	if(pucMSCDMABuf)
		free(pucMSCDMABuf);

	if(openmaxStandComp)
	{
		tuner_deinit(openmaxStandComp);    	
		free(openmaxStandComp);
	}		
	PRINTF("Return With Success !!!\n");
    return 0;
_error_return_:
	if( iTSIFHandle >= 0 )
		stTSIFEntry.pfn_Close(iTSIFHandle);
	
	if(pfp)
		fclose(pfp);

	disk_close();
	
	if(pucMSCDMABuf)
		free(pucMSCDMABuf);
	if(openmaxStandComp)
	{
		tuner_deinit(openmaxStandComp);    	
		free(openmaxStandComp);
	}
	PRINTF("Return With Failure !!!\n");
	return -1;
}
