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

#define PRINTF(msg...)	printf(msg)

#define TSIF_PACKETSIZE			(188)
#define TSIF_MAX_PACKETCOUNT	(8190)
#define TSIF_INT_PACKETCOUNT	(39)


#define	STREAM_BUFFER_SIZE		(TSIF_PACKETSIZE*50000)

typedef	struct
{
	FILE *pFP;
	unsigned char *pucData;
	unsigned int uiDataSize;
	unsigned int isWorking;
}DUMP_ARG;

static int giBaseBandType = 1; //1:dibcom, 2:tcc351x_cspi_sts, 3:tcc351x_i2c_sts, 4:mxl101sf

extern OMX_ERRORTYPE omx_linuxtv_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
extern OMX_ERRORTYPE omx_linuxtv_tuner_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp);
extern OMX_ERRORTYPE omx_tcc351x_CSPI_STS_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
extern OMX_ERRORTYPE omx_tcc351x_tuner_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp);
extern OMX_ERRORTYPE omx_mxl101sf_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
extern OMX_ERRORTYPE omx_mxl101sf_tuner_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp);

//GPSB TSIF module
extern int TCC_TSIF_Open(int packet_intr_cnt, int packet_dma_cnt ,int mode, int dma_mode);
extern int TCC_TSIF_DxBPower(int handle, int on_off);
extern int TCC_TSIF_SetPID(int handle, int PID);
extern int TCC_TSIF_RemovePID(int handle, int PID);
extern int TCC_TSIF_Close(int handle);
extern int TCC_TSIF_Read(int handle, unsigned char *buf, unsigned int read_size);
extern int TCC_TSIF_ReadEX(int handle, unsigned char *buf, unsigned int read_size);	
extern int TCC_TSIF_SetPCRPID(int handle, unsigned int uiPID, unsigned int index);
extern int TCC_TSIF_GetSTC(int handle, unsigned int index);

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

static void HexDump  (unsigned char *addr, unsigned int len)
{
 	unsigned char		*s=addr, *endPtr=(unsigned char*)((unsigned int)addr+len);
 	unsigned int		i, remainder=len%16;
	unsigned char ucPrintBuffer[128] = {0,};
	unsigned char ucWholePrintBuffer[1024] = {0,};
	
 	sprintf(ucPrintBuffer,"\n Offset        Hex Value         Ascii value\n");
	strcat(ucWholePrintBuffer, ucPrintBuffer);
 	
 	// print out 16 byte blocks.
 	while (s+16<=endPtr){
 		
 		// offset ì¶œë ¥.
 		sprintf(ucPrintBuffer,"0x%08lx  ", (long)(s-addr));
 		strcat(ucWholePrintBuffer, ucPrintBuffer);	
 		// 16 bytes ?¨ìœ„ë¡??´ìš© ì¶œë ¥.
 		for (i=0; i<16; i++){
 			sprintf(ucPrintBuffer,"%02x ", s[i]);
			strcat(ucWholePrintBuffer, ucPrintBuffer);
 		}
 		sprintf(ucPrintBuffer," ");
		strcat(ucWholePrintBuffer, ucPrintBuffer);
 		
 		for (i=0; i<16; i++){
 			if (s[i]>=32 && s[i]<=125)sprintf(ucPrintBuffer,"%c", s[i]);
 			else sprintf(ucPrintBuffer,".");

			strcat(ucWholePrintBuffer, ucPrintBuffer);
 		}
 		s += 16;
 		sprintf(ucPrintBuffer,"\n");
		strcat(ucWholePrintBuffer, ucPrintBuffer);
 	}
 	
 	// Print out remainder.
 	if (remainder){
 		
 		// offset ì¶œë ¥.
 		sprintf(ucPrintBuffer,"0x%08lx  ", (long)(s-addr));
		strcat(ucWholePrintBuffer, ucPrintBuffer);
 		
 		// 16 bytes ?¨ìœ„ë¡?ì¶œë ¥?˜ê³  ?¨ì? ê²?ì¶œë ¥.
 		for (i=0; i<remainder; i++){
 			sprintf(ucPrintBuffer,"%02x ", s[i]);
			strcat(ucWholePrintBuffer, ucPrintBuffer);
 		}
 		for (i=0; i<(16-remainder); i++){
 			sprintf(ucPrintBuffer,"   ");
			strcat(ucWholePrintBuffer, ucPrintBuffer);
 		}
 
 		sprintf(ucPrintBuffer," ");
		strcat(ucWholePrintBuffer, ucPrintBuffer);
 		for (i=0; i<remainder; i++){
 			if (s[i]>=32 && s[i]<=125) sprintf(ucPrintBuffer,"%c", s[i]);
 			else	sprintf(ucPrintBuffer,".");
			strcat(ucWholePrintBuffer, ucPrintBuffer);
 		}
 		for (i=0; i<(16-remainder); i++){
 			sprintf(ucPrintBuffer," ");
			strcat(ucWholePrintBuffer, ucPrintBuffer);
 		}
 		sprintf(ucPrintBuffer,"\n");
		strcat(ucWholePrintBuffer, ucPrintBuffer);
 	}
	PRINTF("%s",ucWholePrintBuffer);
 	return;
 }


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
        break;
    case 2:
	return omx_tcc351x_CSPI_STS_tuner_component_Init(phandle);
       	break;
    case 3:
       	return omx_tcc351x_I2C_STS_tuner_component_Init(phandle);
       	break;
     case 4:
        return NULL;//omx_mxl101sf_tuner_component_Init(phandle);
        break;
    }
	return 0;
}

int tuner_deinit(void *phandle)
{
    switch(giBaseBandType)
    {
    case 1:
	return omx_linuxtv_tuner_component_Deinit(phandle);
        break;
    case 2:    
    case 3:
	return omx_tcc351x_tuner_component_Deinit(phandle);
       	break;
     case 4:
        return NULL;//omx_mxl101sf_tuner_component_Deinit(phandle);
        break;       	
    }
    return 0;

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
	int iPidTable[32], i, bufferpos, iPidNum, iPID;
	unsigned char *pucFile;
	int ret, iFreq, iBW, iDumpSizeKbyte, iTotalSaveSize;
	TCCTSIF_FUNC_ENTRY stTSIFEntry;
	char    value[PROPERTY_VALUE_MAX];
	unsigned int   uiLen = 0, uiType = 0;
	
	char	*USAGE	=
		"Usage: dvbtdump frequency bandwidth dumpsize file [pid] [pid] ...\n"
		"       frequency, bandwidth is Khz unit.\n"
		"       dumpsize is Kbyte unit.\n"
		"       if there are no pids as an input arguments, save whole ts stream.\n"
	;

	if(argc < 5)
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
	memset(iPidTable, 0xff, 32*sizeof(int));
	iFreq = atoi(argv[1]);
	iBW = atoi(argv[2]);
	iDumpSizeKbyte = atoi(argv[3]);
	pucFile = argv[4];
	
	iPidNum = argc-5;
	if(iPidNum > 32)
		iPidNum = 32;
	for(i=0;i<iPidNum;i++)
	{
		iPidTable[i] = atoi(argv[5+i]);
	}

	pfp = fopen(pucFile,"wb");
	if(pfp == NULL)
	{
		PRINTF("CANNOT Make file [%s] !!!\n", pucFile);
		if(iPidNum == 0)
			goto _error_return_;

		PRINTF("Check only continuity error at PID[0x%X]!!!\n", iPidTable[0]);
		iPidNum = 1;
	}
	
	PRINTF("Frequency[%dKhz] BandWidth[%dKhz] OutputPath[%s] Size[%dKbyte] Pids[%d]\n", iFreq, iBW, pucFile, iDumpSizeKbyte, iPidNum);
	PRINTF("PIDs :: \n");
	for(i=0;i<iPidNum;i++)
	{
		PRINTF("[0x%X]", iPidTable[i]);	
	}
	PRINTF("\n");
	
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
	iARGS[1] = 0; // 0:DVBT 1:ISDBT
	iARGS[2] = giBaseBandType;
	err = OMX_SetParameter(openmaxStandComp,OMX_IndexVendorParamTunerOpen, iARGS);
	if(err != OMX_ErrorNone) 
	{
		PRINTF("error in OMX_IndexVendorParamTunerOpen\n");
		goto _error_return_;
	}	

	iTSIFHandle = stTSIFEntry.pfn_Open(1, TSIF_MAX_PACKETCOUNT, 1, 1);
	if( iTSIFHandle < 0 ){	
		PRINTF("error TCCTSIF_Open\n");
		return -1;
	}

	iARGS[0] = iFreq;
	iARGS[1] = iBW;
	err = OMX_SetParameter(openmaxStandComp,OMX_IndexVendorParamTunerFrequencySet, iARGS);
	if(err != OMX_ErrorNone) 
	{
		PRINTF("error in OMX_IndexVendorParamTunerFrequencySet\n");
		goto _error_return_;
	}

	for(i=0;i<iPidNum;i++)
	{
		stTSIFEntry.pfn_SetPID(iTSIFHandle, iPidTable[i]);
	}

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
			ret = stTSIFEntry.pfn_Read(iTSIFHandle, pucBuf, TSIF_PACKETSIZE);
			//HexDump(pucBuf, TSIF_PACKETSIZE);
			if( ret == TSIF_PACKETSIZE )
			{
				if(iPidNum == 1)
				{	
					//Check continuity error
					iPID = ((pucBuf[1] & 0x1F) << 8) | pucBuf[2];
					if(iPID ==  iPidTable[0])
                    {
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
		stTSIFEntry.pfn_Close(iTSIFHandle);
	
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
		stTSIFEntry.pfn_Close(iTSIFHandle);
	
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
