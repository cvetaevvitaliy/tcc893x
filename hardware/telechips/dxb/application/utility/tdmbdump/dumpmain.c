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
#include "tcc_tsif.h"

#define	SUPPORT_TUNER_TCC351X

#define PRINTF(msg...)	printf(msg)

#define SRVTYPE_NONE                0x00
#define SRVTYPE_DAB                 0x01
#define SRVTYPE_DABPLUS             0x02
#define SRVTYPE_DATA                0x03
#define SRVTYPE_DMB                 0x04
#define SRVTYPE_FIDC                0x05
#define SRVTYPE_RAWDATA             0x06
#define SRVTYPE_FIC                 0x07
#define SRVTYPE_FIC_WITH_ERR        0x08
#define EWS_ANNOUNCE_FLAG                   0x09
#define RECONFIGURATION_FLAG                0x0a
#define EWS_ANNOUNCE_RECONFIGURATION_FLAG   0x0b


#define TSIF_PACKETSIZE			(188)
#define TSIF_MAX_PACKETCOUNT	(8190)
#define TSIF_INT_PACKETCOUNT	(39)


#define	STREAM_BUFFER_SIZE		(TSIF_PACKETSIZE*10)

typedef	struct
{
	FILE *pFP;
	unsigned char *pucData;
	unsigned int uiDataSize;
	unsigned int isWorking;
}DUMP_ARG;

extern OMX_ERRORTYPE omx_tcc351x_CSPI_STS_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
extern OMX_ERRORTYPE omx_tcc351x_tuner_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp);

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

void *dumpthread(void *arg)
{
	DUMP_ARG *pArg;
	pArg = (DUMP_ARG *)arg;
	pArg->isWorking = 1;
	fwrite(	pArg->pucData, 1, pArg->uiDataSize, pArg->pFP);
	pArg->isWorking = 0;
	return NULL;
}

int main(int argc, char *argv[])
{
	DUMP_ARG stArg;
	FILE *pfp;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_COMPONENTTYPE *openmaxStandComp;	
	unsigned char *pucMSCDMABuf, *pucBuf;
	pthread_t thread_id;
	int iTSIFHandle;
	int iPrevCC, iCC; 
	int err=0;
	int iARGS[3];
	int i, bufferpos, iPID;
	unsigned char *pucFile;
	int ret, iFreq, iDumpSizeKbyte, iTotalSaveSize;
	unsigned char ucService[7];
	char	*USAGE	=
		"Usage: tdmbdump frequency dumpsize file\n"
		"       frequency is Khz unit.\n"
		"       dumpsize is Kbyte unit.\n"
		"       ex)tdmbdump 181280 200 /nand/tdmb.ts\n"
	;

	if(argc < 4)
	{
		PRINTF("%s", USAGE);
		return -1;
	}	
	openmaxStandComp = NULL;
	pucMSCDMABuf = NULL;
	iTSIFHandle = -1;
	pfp = NULL;
	thread_id = NULL;
	memset(&stArg, 0x00, sizeof(DUMP_ARG));	
	iFreq = atoi(argv[1]);
	iDumpSizeKbyte = atoi(argv[2]);
	pucFile = argv[3];
	

	pfp = fopen(pucFile,"wb");
	if(pfp == NULL)
	{
		PRINTF("CANNOT Make file [%s] !!!\n", pucFile);
		goto _error_return_;		
	}
	
	PRINTF("Frequency[%dKhz] OutputPath[%s] Size[%dKbyte]\n", iFreq, pucFile, iDumpSizeKbyte);
	
	openmaxStandComp = calloc(1,sizeof(OMX_COMPONENTTYPE));
	if (!openmaxStandComp) 
	{
		goto _error_return_;
	}
	tuner_init(openmaxStandComp);

	pucMSCDMABuf = malloc(STREAM_BUFFER_SIZE*2 + TSIF_PACKETSIZE); //*2 dubble buffer
	if( NULL == pucMSCDMABuf )
	{	
		PRINTF("error malloc\n");
		goto _error_return_;
	}

	//BB Power ON
	iARGS[0] = 0;
	iARGS[1] = 2;	// 0:DVBT 1:ISDBT 2:TDMB
	//iARGS[1] = 0; // 0:DVBT 1:ISDBT 2:TDMB
	//iARGS[1] = 1; // 0:DVBT 1:ISDBT 2:TDMB

	//iARGS[2] = 0; //do not read data at tcc351x comp
	iARGS[2] = 1;
	err = OMX_SetParameter(openmaxStandComp,OMX_IndexVendorParamTunerOpen, iARGS);
	if(err != OMX_ErrorNone) 
	{
		PRINTF("error in OMX_IndexVendorParamTunerOpen\n");
		goto _error_return_;
	}	

#if 0
	iTSIFHandle = TCC_TSIF_Open(1, TSIF_MAX_PACKETCOUNT, 1, 1);
	if( iTSIFHandle < 0 ){	
		PRINTF("error TCCTSIF_Open\n");
		return -1;
	}
#endif

	iARGS[0] = iFreq;	
	err = OMX_SetParameter(openmaxStandComp,OMX_IndexVendorParamTunerFrequencySet, iARGS);
	if(err != OMX_ErrorNone) 
	{
		PRINTF("error in OMX_IndexVendorParamTunerFrequencySet\n");
		goto _error_return_;
	}

	while(1)
		usleep(5000);
	
#if 1
	int piArg[2];
	piArg[0] = (int)iARGS;
	piArg[1] = 1;
	iARGS[0] = SRVTYPE_DMB;
	iARGS[1] = ucService;
 	err = OMX_SetParameter(openmaxStandComp,OMX_IndexVendorParamTunerChannelSet, piArg);
	if(err != OMX_ErrorNone) 
	{
		PRINTF("error in OMX_IndexVendorParamTunerFrequencySet\n");
		goto _error_return_;
	}
#endif
	
	////////////////////////
	iPrevCC = -1;
	iTotalSaveSize = 0;
	PRINTF("Dump Started.....\n");
	//for(j=0;j<2;j++)
	bufferpos = 0;
	while(1)
	{			
		//PRINTF("buffer pos %d\n", bufferpos);
		for(i=0; i<STREAM_BUFFER_SIZE; i+=TSIF_PACKETSIZE)
		{
			pucBuf = pucMSCDMABuf+bufferpos*STREAM_BUFFER_SIZE+i;
			ret = TCC_TSIF_Read(iTSIFHandle, pucBuf, TSIF_PACKETSIZE);
			if( ret == TSIF_PACKETSIZE )
			{
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
		}

		iTotalSaveSize += STREAM_BUFFER_SIZE;		
		if( (iTotalSaveSize/1024)  > iDumpSizeKbyte)
		{
			PRINTF("\nDump Finished.....\n");
			break;
		}
		bufferpos++;
		bufferpos = bufferpos&0x1;		
	}
	if(pfp)
		pthread_join(thread_id, NULL);

	///////////////////////
	err = OMX_SetParameter(openmaxStandComp,OMX_IndexVendorParamTunerClose, NULL);
	
	if( iTSIFHandle >= 0 )
		TCC_TSIF_Close(iTSIFHandle);
	
	if(pfp)
		fclose(pfp);
	
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
		TCC_TSIF_Close(iTSIFHandle);
	
	if(pfp)
		fclose(pfp);
	
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
