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

/****************************************************************************

Revision History

****************************************************************************

****************************************************************************/
#define LOG_TAG	"TUNER_TCC317X"
#include <utils/Log.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "tcc_tsif.h"
#include "tcc_dxb_control.h"
#include "tcc317x_api.h"
#include "tcpal_os.h"
#include <omx_tcc317x_tuner_component.h>
#include "TDMBInfo.h"
#include "tdmb_tcc317x_tuner.h"
#include "tdmb_tuner_space.h"
#include "tcc317x_boot.h"
#include "tsemaphore.h"
#include "SimpleBuffer.h"
#include "tcc317x_common.h"
#include "omx_tcc_thread.h" 

#include "tcc317x_stream_process.h"
extern Tcc317xStreamCtrl_t Tcc317xStreamCtrl[2];  

#define USE_SIMPLE_AV_FIFO 
#ifdef USE_SIMPLE_AV_FIFO
static tsem_t* gpAVFifoSema[2] = { NULL, NULL }; 
static SRBUFFER gstAVFifo[2];
static I08S *gpAVFifoBuffer[2];
#define	AV_FIFO_SIZE	500*1024
#endif 

/* TSIF */
int giTSIFHandle[2] = { -1, -1 };
static pthread_t TCC317XThreadID[2];

#if defined (_STATE_GET_TRANSMISSIONMODE_)
I32U OFDM_READFLAG = 0;
I16U Transmission_mode = 0;
#endif

#ifdef _MULTI_SERVICE_
I32S gChannelSetFrequncy[MAX_FICPARSER_SUPPRT_BB] = {-1, -1};
#endif

/* DSP BuildDate: year(12bit), Month(4bit), day(8), hour(8) */
I32U Tcc317x_DSPBuildDate = ((2013<<20) | (12<<16) | (30<<8) | 16);

/* Stream Dump */
FILE *stfp;
#define STREAMDUMP_OPEN 	stfp = fopen("/storage/sdcard0/TDMB_RawData.bin", "wb");\
				if(stfp == NULL){\
					TcpalPrintLog((I08S *) "[    DUMP   ] FILE OPEN ERROR\n");\
					return;\
				}

#define STREAMDUMP_WRITE	fwrite(&pucMSCDMABuf[0],ret,1,stfp);
#define STREAMDUMP_CLOSE	fclose(stfp);
/***************************************************************************/

static TENSEMBLEDB		gpEnsembleDBList[MAX_FICPARSER_SUPPRT_BB][MAX_ENSEMBLEDB];
static TSERVICEDB			gpServiceDBList[MAX_FICPARSER_SUPPRT_BB][MAX_SERVICEDB];
static TCHANNELDB		gpChannelDBList[MAX_FICPARSER_SUPPRT_BB][MAX_CHANNELDB];
static E_TDMB_STATE		eTDMBState[MAX_FICPARSER_SUPPRT_BB];
static TDMB_INFO			g_tChannelInfo[MAX_FICPARSER_SUPPRT_BB][64];
static I32S giDABServiceCount[MAX_FICPARSER_SUPPRT_BB];
static I32S giDMBServiceCount[MAX_FICPARSER_SUPPRT_BB];
static I32S giDATAServiceCount[MAX_FICPARSER_SUPPRT_BB];
static I32S giTOTALServiceCount[MAX_FICPARSER_SUPPRT_BB];
static I32S giDABPLUSServiceCount[MAX_FICPARSER_SUPPRT_BB];
static I32S giCurrentActiveFreqency[MAX_FICPARSER_SUPPRT_BB];

static I32S g_tcc_dxb_ctrl_fd = -1;
I32S CurrentNumOfDiversity = 1;

extern I32S Tcc317xI2cOpen(I32S _moduleIndex);
extern I32S Tcc317xI2cClose(I32S _moduleIndex);
extern I32S Tcc317xTccspiOpen(I32S _moduleIndex);
extern I32S Tcc317xTccspiClose(I32S _moduleIndex);

Tcc317xOption_t Tcc317xOptions[TCC317X_MAX][TCC317X_DIVERSITY_MAX];

TcpalSemaphore_t Tcc317xTuneSema[TCC317X_MAX] = {0,0};
TcpalSemaphore_t Tcc317xStreamSema[TCC317X_MAX] = {0,0};
TcpalSemaphore_t Tcc317xFICParserSema[TCC317X_MAX] = {0,0};
I08S *Tcc317xTuneSemaName[TCC317X_MAX] = {"Tcc317xTuneSema0","Tcc317xTuneSem1"};
I08S *Tcc317xFICParserSemaName[TCC317X_MAX] = {"Tcc317xFICSema0","Tcc317xFICSem1"};
I08S *Tcc317xStreamSemaName[TCC317X_MAX] = {"Tcc317xStreamSema0","Tcc317xStreamSema1"};

extern Tcc317xOption_t Tcc317xOptionSingle;
extern Tcc317xRegisterConfig_t Tcc317xSingle[3];
extern Tcc317xHandle_t Tcc317xHandle[TCC317X_MAX][TCC317X_DIVERSITY_MAX];

#ifdef _DUAL_CHAIN_MODE_
extern Tcc317xOption_t Tcc317xOptionDualChainMode[2];
extern Tcc317xRegisterConfig_t Tcc317x_FirstBB_ChainMode[3];
extern Tcc317xRegisterConfig_t Tcc317x_LastBB_ChainMode[3];
#endif /*_DUAL_CHAIN_MODE_*/

extern I08U TCC317X_BOOT_DATA_TDMB[TCC317X_BOOT_SIZE_TDMB];
extern I08U g_DMBBuffer[MAX_FICPARSER_SUPPRT_BB][DMB_BUFFER_SIZE];
extern I32U gI2CHanleInit1;

I32S TCC317x_OpenFlag[4] = {0,0,0,0};
I32S holdStatus[2] = { 0, 0 };

static void TCC317XTDMB_TimeDelayMs(I32U uiTimeMs) 
{
	usleep(uiTimeMs*1000);
}

E_TDMB_STATE TCC317XTDMB_SetState(I32S moduleidx, E_TDMB_STATE eState)
{
	eTDMBState[moduleidx] = eState;
	return eTDMBState[moduleidx];
}

E_TDMB_STATE TCC317XTDMB_GetState(I32S moduleidx)
{	
	return eTDMBState[moduleidx];
}


void TCC317XTDMB_RegisterReadEX(I32S iHandleNum, I32U Addr, I08U *data, I08U bytesize)
{
	I32S i;
 
	if(iHandleNum==1 && gI2CHanleInit1==0)
		return;

	for(i=0; i<bytesize; i++)
	{ 
		Tcc317xApiRegisterRead(iHandleNum, 0, Addr+i, &data[i],bytesize);
	}
}

void TCC317XTDMB_RegisterWriteEX(I32S iHandleNum, I32U Addr, I08U data)
{
	if(iHandleNum==1 && gI2CHanleInit1==0)
		return;
	
	Tcc317xApiRegisterWrite(iHandleNum, 0, Addr, &data, 1);
}

I32S TCC317XTDMB_AvFifoBuffer_Init(I32U moduleidx)
{
#ifdef _DUAL_CHAIN_MODE_
	I32U i;
	for(i=0;i<2;i++){
		memset(&gstAVFifo[i], 0, sizeof(SRBUFFER));
		gpAVFifoBuffer[i] = (char *)malloc(AV_FIFO_SIZE);
		
		if(gpAVFifoBuffer[i] == NULL)
			return OMX_ErrorBadParameter;
		
		memset(gpAVFifoBuffer[i], 0, sizeof(SRBUFFER));
		QInitBuffer(&gstAVFifo[i], AV_FIFO_SIZE, gpAVFifoBuffer[i]);
		gpAVFifoSema[i] = (tsem_t*)TCC_calloc(1,sizeof(tsem_t));
		
		if(gpAVFifoSema[i] == NULL ){
			free(gpAVFifoBuffer[i]);
			gpAVFifoBuffer[i] = NULL;
			return OMX_ErrorBadParameter;
		}	
		tsem_init(gpAVFifoSema[i], 1);
	}
#else
	memset(&gstAVFifo[moduleidx], 0, sizeof(SRBUFFER));
	gpAVFifoBuffer[moduleidx] = (char *)malloc(AV_FIFO_SIZE);
	
	if(gpAVFifoBuffer[moduleidx] == NULL)
		return OMX_ErrorBadParameter;
	
	memset(gpAVFifoBuffer[moduleidx], 0, sizeof(SRBUFFER));
	QInitBuffer(&gstAVFifo[moduleidx], AV_FIFO_SIZE, gpAVFifoBuffer[moduleidx]);
	gpAVFifoSema[moduleidx] = (tsem_t*)TCC_calloc(1,sizeof(tsem_t));
	
	if(gpAVFifoSema[moduleidx] == NULL ){
		free(gpAVFifoBuffer[moduleidx]);
		gpAVFifoBuffer[moduleidx] = NULL;
		return OMX_ErrorBadParameter;
	}	
	tsem_init(gpAVFifoSema[moduleidx], 1);
#endif
	return TCC317X_RETURN_SUCCESS;
}

I32S TCC317XTDMB_AvFifoBuffer_DeInit(I32U moduleidx)
{	
#ifdef _DUAL_CHAIN_MODE_
	I32U i;
	for(i=0;i<2;i++){
		if( gpAVFifoBuffer[i] )
			free(gpAVFifoBuffer[i]);
		gpAVFifoBuffer[i] = NULL;
		memset(&gstAVFifo[i], 0, sizeof(SRBUFFER));	
		if(gpAVFifoSema[i] != NULL)
		{		
			tsem_deinit(gpAVFifoSema[i]);
			TCC_free(gpAVFifoSema[i]);
			gpAVFifoSema[i] = NULL;
		}
	}
#else
	if( gpAVFifoBuffer[moduleidx] )
		free(gpAVFifoBuffer[moduleidx]);
	gpAVFifoBuffer[moduleidx] = NULL;
	memset(&gstAVFifo[moduleidx], 0, sizeof(SRBUFFER));	
	if(gpAVFifoSema[moduleidx] != NULL)
	{		
		tsem_deinit(gpAVFifoSema[moduleidx]);
		TCC_free(gpAVFifoSema[moduleidx]);
		gpAVFifoSema[moduleidx] = NULL;
	}
#endif
	return TCC317X_RETURN_SUCCESS;
}

void TCC317XTDMB_PllSet(I32S moduleidx)
{
#ifdef _DUAL_CHAIN_MODE_
	I32U i;
	for(i=0;i<2;i++){
	#if defined (_SUPPORT_LBAND_)
		Tcc317xOptions[i][0].useLBAND = 1;
	#else
		Tcc317xOptions[i][0].useLBAND = 0;
	#endif

	#if defined (_OSC_24576kHz_)
		Tcc317xOptions[i][0].oscKhz = 24576;
		Tcc317xOptions[i][0].pll = 0x180E;
	#elif defined (_OSC_38400kHz_)
		Tcc317xOptions[i][0].oscKhz = 38400;
		Tcc317xOptions[i][0].pll = 0x0F0E;
	#else /* _OSC_19200kHz_ */
		Tcc317xOptions[i][0].oscKhz = 19200;
		Tcc317xOptions[i][0].pll = 0x0F06;
	#endif
	}
#else
	#if defined (_SUPPORT_LBAND_)
		Tcc317xOptions[moduleidx][0].useLBAND = 1;
	#else
		Tcc317xOptions[moduleidx][0].useLBAND = 0;
	#endif

	#if defined (_OSC_24576kHz_)
		Tcc317xOptions[moduleidx][0].oscKhz = 24576;
		Tcc317xOptions[moduleidx][0].pll = 0x180E;
	#elif defined (_OSC_38400kHz_)
		Tcc317xOptions[moduleidx][0].oscKhz = 38400;
		Tcc317xOptions[moduleidx][0].pll = 0x0F0E;
	#else /* _OSC_19200kHz_ */
		Tcc317xOptions[moduleidx][0].oscKhz = 19200;
		Tcc317xOptions[moduleidx][0].pll = 0x0F06;
	#endif
#endif /* _DUAL_CHAIN_MODE_ */
}

void TCC317XTDMB_FICbufferInit(I32S moduleidx)
{
	I32S i;
	TDMB_INITDBINFO initDBInfo[MAX_FICPARSER_SUPPRT_BB];

	if(moduleidx >= MAX_FICPARSER_SUPPRT_BB)
		return;
	
#ifdef _DUAL_CHAIN_MODE_
	/* FIC Parser Init */
	for(i=0; i<MAX_FICPARSER_SUPPRT_BB; i++)
	{
		DMB_BufferInit(i, &g_DMBBuffer[i][0]);
	}

	for(i=0; i<MAX_FICPARSER_SUPPRT_BB; i++)
	{
		initDBInfo[i].pEnsembleDBList = gpEnsembleDBList[i];
		initDBInfo[i].pServiceDBList = gpServiceDBList[i];
		initDBInfo[i].pChannelDBList = gpChannelDBList[i];
		
		initDBInfo[i].ensembleDBMaxSize = MAX_ENSEMBLEDB;
		initDBInfo[i].serviceDBMaxSize = MAX_SERVICEDB;
		initDBInfo[i].channelDBMaxSize = MAX_CHANNELDB;
		
		DMB_InitDB(i,&initDBInfo[i]);
	}
#else
	DMB_BufferInit(moduleidx, &g_DMBBuffer[moduleidx][0]);

	initDBInfo[moduleidx].pEnsembleDBList = gpEnsembleDBList[moduleidx];
	initDBInfo[moduleidx].pServiceDBList = gpServiceDBList[moduleidx];
	initDBInfo[moduleidx].pChannelDBList = gpChannelDBList[moduleidx];
	
	initDBInfo[moduleidx].ensembleDBMaxSize = MAX_ENSEMBLEDB;
	initDBInfo[moduleidx].serviceDBMaxSize = MAX_SERVICEDB;
	initDBInfo[moduleidx].channelDBMaxSize = MAX_CHANNELDB;
	
	DMB_InitDB(moduleidx, &initDBInfo[moduleidx]);
#endif
}

void TCC317XTDMB_OpenFlagInit(void){
	TcpalMemset(&TCC317x_OpenFlag[0], 0x00, sizeof(I32S)*2);
}

void TCC317XTDMB_OpenFlagSet(I32U moduleidx, I32U flag){
	TCC317x_OpenFlag[moduleidx] = flag;
}
I32S TCC317XTDMB_OpenFlagGet(I32U moduleidx){
	return TCC317x_OpenFlag[moduleidx];
}

I32S TCC317XTDMB_Init(I32S moduleidx, I32S iCountryCode, I32S commandInterface, I32S streamInterface)
{
	I32S ret,i;
	I32S optIdx = 1;

	/* Single */
	CurrentNumOfDiversity = 1;
	
#ifdef _DUAL_CHAIN_MODE_
	if(TCC317XTDMB_OpenFlagGet(0) == 1 || TCC317XTDMB_OpenFlagGet(1) == 1)	
		return TCC317X_RETURN_SUCCESS;

	ret = TCC317XTDMB_IO_PW_OPEN();
	if (ret != 0)
		return OMX_ErrorBadParameter;
	
	/* LDO Common Bus */
	TCC317XTDMB_IO_PW_OFF(0);
	TCC317XTDMB_IO_PW_ON(0);	
#else
	/* Reset Board Type Control */
	if(TCC317XTDMB_OpenFlagGet(0) == 0 && TCC317XTDMB_OpenFlagGet(1) == 0) {
		ret = TCC317XTDMB_IO_PW_OPEN();
		if (ret != 0)
			return OMX_ErrorBadParameter;
		
		/* reset other BB */
		if(moduleidx==0)
			TCC317XTDMB_IO_PW_RESET_LOW (1);
		else
			TCC317XTDMB_IO_PW_RESET_LOW (0);
	
		/* LDO Common Bus */
		TCC317XTDMB_IO_PW_OFF(moduleidx);
		TCC317XTDMB_IO_PW_ON(moduleidx);
	}
#endif /* _DUAL_CHAIN_MODE_ */

	/* Open I2C/TCCSPI Driver and Initialize */
	if (commandInterface == TCC317X_IF_TCCSPI)
		Tcc317xTccspiOpen(moduleidx);
	else if (commandInterface == TCC317X_IF_I2C)
		Tcc317xI2cOpen(moduleidx);
	else
		Tcc317xTccspiOpen(moduleidx);

#ifdef _DUAL_CHAIN_MODE_
	TCC317XTDMB_IO_PW_RESET(0);
	TCC317XTDMB_IO_PW_RESET(1);
#else
	TCC317XTDMB_IO_PW_RESET(moduleidx);
#endif 

	/* Tcc317x Option */
	if (streamInterface == TCC317X_STREAM_IO_MAINIO)
		optIdx = 0;
	else if (streamInterface == TCC317X_STREAM_IO_SPIMS)
		optIdx = 2;
	else
		optIdx = 1;

#ifdef _DUAL_CHAIN_MODE_
	/* 317x Option Copy Single & Dual Mode*/	
	for(i=0;i<2;i++){
		TcpalMemcpy (&Tcc317xOptions[i][0], &Tcc317xOptionDualChainMode[i], sizeof (Tcc317xOption_t));	
	}

	if(Tcc317xOptions[0][0].boardType == TCC317X_BOARD_DUAL_CHAINMODE && 
		Tcc317xOptions[1][0].boardType == TCC317X_BOARD_DUAL_CHAINMODE)
		TcpalPrintErr((I08S *) "# [TCC317X] Board type [Dual Chain] \n");

	for(i=0;i<2;i++){	
		Tcc317xOptions[i][0].commandInterface = (I08S) (commandInterface);				
		Tcc317xOptions[i][0].streamInterface = (I08S) (streamInterface);			
		Tcc317xOptions[i][0].useInterrupt = 0;
	}
	
	TcpalMemcpy(&Tcc317xOptions[0][0].config,&Tcc317x_FirstBB_ChainMode[optIdx],sizeof(Tcc317xRegisterConfig_t));
	TcpalMemcpy(&Tcc317xOptions[1][0].config,&Tcc317x_LastBB_ChainMode[optIdx],sizeof(Tcc317xRegisterConfig_t));
#else
	TcpalMemset (&Tcc317xOptions[moduleidx][0], 0x00, sizeof(Tcc317xOption_t) * 4);
	TcpalMemcpy (&Tcc317xOptions[moduleidx][0], &Tcc317xOptionSingle, sizeof(Tcc317xOption_t));	

	if(moduleidx==0)
		Tcc317xOptions[moduleidx][0].address = 0xA2;
	else
		Tcc317xOptions[moduleidx][0].address = 0xA4;

	if(Tcc317xOptions[moduleidx][0].boardType == TCC317X_BOARD_SINGLE)
		TcpalPrintErr((I08S *) "# [TCC317X] Board type [Single] \n");

	Tcc317xOptions[moduleidx][0].commandInterface = (I08S) (commandInterface);
	Tcc317xOptions[moduleidx][0].streamInterface = (I08S) (streamInterface);
	Tcc317xOptions[moduleidx][0].config = Tcc317xSingle[optIdx];
#endif /* _DUAL_CHAIN_MODE_ */

	TCC317XTDMB_PllSet(moduleidx);

#ifdef _DUAL_CHAIN_MODE_
	ret = Tcc317xApiOpen(0, &Tcc317xOptions[0][0], sizeof(Tcc317xOption_t));
	if (ret != TCC317X_RETURN_SUCCESS) {
		TcpalPrintLog((I08S *) "# [0]Tcc317xApiOpen Fail!!!\n");
		return OMX_ErrorBadParameter;
	}else
		TcpalPrintLog((I08S *) "# [0]Tcc317xApiOpen Success!!!\n");
	
	ret = Tcc317xApiOpen(1, &Tcc317xOptions[1][0], sizeof(Tcc317xOption_t));
	if (ret != TCC317X_RETURN_SUCCESS) {
		TcpalPrintLog((I08S *) "# [1]Tcc317xApiOpen Fail!!!\n");
		return OMX_ErrorBadParameter;
	}else
		TcpalPrintLog((I08S *) "# [1]Tcc317xApiOpen Success!!!\n");

	ret = Tcc317xApiInit(0, &TCC317X_BOOT_DATA_TDMB[0], TCC317X_BOOT_SIZE_TDMB);
	if (ret != TCC317X_RETURN_SUCCESS) {
		TcpalPrintLog((I08S *) "# [0]Tcc317xApiInit Fail!!!\n");
		return OMX_ErrorBadParameter;
	}else 
		TcpalPrintLog((I08S *) "# [0]Tcc317xApiInit Success!!!\n");	
	
	ret = Tcc317xApiInit(1, &TCC317X_BOOT_DATA_TDMB[0], TCC317X_BOOT_SIZE_TDMB);
	if (ret != TCC317X_RETURN_SUCCESS) {
		TcpalPrintLog((I08S *) "# [1]Tcc317xApiInit Fail!!!\n");
		return OMX_ErrorBadParameter;
	}else 
		TcpalPrintLog((I08S *) "# [1]Tcc317xApiInit Success!!!\n");		
#else
	ret = Tcc317xApiOpen(moduleidx, &Tcc317xOptions[moduleidx][0], sizeof(Tcc317xOption_t));
	if (ret != TCC317X_RETURN_SUCCESS) {
		TcpalPrintLog((I08S *) "# [%d] Tcc317xApiOpen Fail!!!\n",moduleidx);
		return OMX_ErrorBadParameter;
	}else
		TcpalPrintLog((I08S *) "# [%d] Tcc317xApiOpen Success!!!\n",moduleidx);

	ret = Tcc317xApiInit(moduleidx, &TCC317X_BOOT_DATA_TDMB[0], TCC317X_BOOT_SIZE_TDMB);
	if (ret != TCC317X_RETURN_SUCCESS) {
		TcpalPrintLog((I08S *) "# [%d] Tcc317xApiInit Fail!!!\n",moduleidx);
		return OMX_ErrorBadParameter;
	}else 
		TcpalPrintLog((I08S *) "# [%d] Tcc317xApiInit Success!!!\n",moduleidx);	
#endif /* _DUAL_CHAIN_MODE_ */

	DMBPROTOCOL_initEx(TCC317XTDMB_RegisterReadEX, TCC317XTDMB_RegisterWriteEX, TCC317XTDMB_TimeDelayMs); /* I2C or SPI */
	TDMBSPACE_CTuneSpace(iCountryCode); 
	TCC317XTDMB_FICbufferInit(moduleidx);

#ifdef _DUAL_CHAIN_MODE_
	for(i=0; i<2; i++)
	{
		Tcc317xStreamParserInit (i);
		giCurrentActiveFreqency[i] = -1;
		gChannelSetFrequncy[i] = -1;
	
		if(Tcc317xTuneSema[i] == 0){			
			TcpalCreateSemaphore(&Tcc317xTuneSema[i], (I08S *)(Tcc317xTuneSemaName[i]), 1); 
			TcpalCreateSemaphore(&Tcc317xFICParserSema[i], (I08S *)(Tcc317xFICParserSemaName[i]), 1);
		}
	}
	
	#ifdef USE_SIMPLE_AV_FIFO
	ret = TCC317XTDMB_AvFifoBuffer_Init(moduleidx);
	if(ret == OMX_ErrorBadParameter){
		TcpalPrintLog((I08S *) "# [%d] Av Fifo Init Fail!!!\n",moduleidx);
		return OMX_ErrorBadParameter;
	}
	#endif
#else
	Tcc317xStreamParserInit (moduleidx);
	giCurrentActiveFreqency[moduleidx] = -1;
	gChannelSetFrequncy[moduleidx] = -1;

	if(Tcc317xTuneSema[moduleidx] == 0){
		TcpalCreateSemaphore(&Tcc317xTuneSema[moduleidx], (I08S *)(Tcc317xTuneSemaName[moduleidx]), 1);	
		TcpalCreateSemaphore(&Tcc317xFICParserSema[moduleidx], (I08S *)(Tcc317xFICParserSemaName[moduleidx]), 1);
	}

#ifdef USE_SIMPLE_AV_FIFO
	ret = TCC317XTDMB_AvFifoBuffer_Init(moduleidx);
	if(ret == OMX_ErrorBadParameter){
		TcpalPrintLog((I08S *) "# [%d] Av Fifo Init Fail!!!\n",moduleidx);
		return OMX_ErrorBadParameter;
	}	
#endif

#endif /* _DUAL_CHAIN_MODE_ */
	return TCC317X_RETURN_SUCCESS;
}

void TCC317XTDMB_StreamThread(void *arg)
{
	int ret, idx = (int)arg;
	unsigned char *pucMSCDMABuf;
	pucMSCDMABuf = malloc(TSIF_PACKETSIZE);
	TcpalCreateSemaphore(&Tcc317xStreamSema[idx], (I08S *)(Tcc317xStreamSemaName[idx]), 1);

	while(giTSIFHandle[idx]>=0)
	{
		TcpalSemaphoreLock(&Tcc317xStreamSema[idx]);
		if(holdStatus[idx] == 0)
		{
			ret = TCC_TSIF_Read(idx, giTSIFHandle[idx], pucMSCDMABuf, TSIF_PACKETSIZE);

			if( ret == TSIF_PACKETSIZE )	
				Tcc317xStreamParsing (idx, pucMSCDMABuf, ret);			
			else			
				Tcc317xMonitoringInit (idx, 0);			
		}
		else		
			TCC317XTDMB_TimeDelayMs(1);
		
		TcpalSemaphoreUnLock(&Tcc317xStreamSema[idx]);
	}

	free(pucMSCDMABuf);
	holdStatus[idx] = 0;
}

void TCC317XTDMB_DataStreamThread(void *arg)
{
	int ret, idx = (int)arg;
	unsigned char *pucMSCDMABuf;
	pucMSCDMABuf = malloc(TSIF_PACKETSIZE);
	TcpalCreateSemaphore(&Tcc317xStreamSema[idx], (I08S *)(Tcc317xStreamSemaName[idx]), 1);

	while(giTSIFHandle[idx]>=0)
	{
		TcpalSemaphoreLock(&Tcc317xStreamSema[idx]);
		if(holdStatus[idx] == 0)
		{
			ret = TCC_TSIF_Read(idx, giTSIFHandle[idx], pucMSCDMABuf, TSIF_PACKETSIZE);
			
			if( ret == TSIF_PACKETSIZE )				
				Tcc317xStreamParsing (idx, pucMSCDMABuf, ret);			
			else			
				Tcc317xMonitoringInit (idx, 0);			
		}
		else		
			TCC317XTDMB_TimeDelayMs(1);
		
		TcpalSemaphoreUnLock(&Tcc317xStreamSema[idx]);
	}

	free(pucMSCDMABuf);
	holdStatus[idx] = 0;
}

void TCC317XTDMB_StreamThreadHold(int moduleidx)
{
	holdStatus[moduleidx] = 1;
}

void TCC317XTDMB_StreamThreadRun(int moduleidx)
{
	holdStatus[moduleidx] = 0;
}

I32S TCC317XTDMB_StreamInterface(I32S moduleidx, I32S *piArg, I32S dataInterface)
{
	int ret, ThreadID;

	if(piArg){
		if(dataInterface == TCC317X_STREAM_IO_STS)	
			giTSIFHandle[moduleidx] = TCC_TSIF_Open(moduleidx, 1, TSIF_MAX_PACKETCOUNT, 1 /*SPI_CS_HIGH*/, DMA_MPEG2TS_MODE);
		else
			giTSIFHandle[moduleidx] = TCC_TSIF_Open(moduleidx, 1, TSIF_MAX_PACKETCOUNT, 3 /*SPI_0X00*/, DMA_NORMAL_MODE);

		if( giTSIFHandle[moduleidx] < 0 ){	
			TcpalPrintLog((I08S *) "[  TDMB_CONTROL  ] [%d] Error TCC TSIF Open ...\n",moduleidx);
			return OMX_ErrorBadParameter;
		}else{	
			if(moduleidx == 0)
				ThreadID = TCCTHREAD_Create(&TCC317XThreadID[moduleidx], TCC317XTDMB_StreamThread, "tcc317x_av_thread", LOW_PRIORITY_10, moduleidx);

			if(moduleidx == 1)
				ThreadID = TCCTHREAD_Create(&TCC317XThreadID[moduleidx], TCC317XTDMB_DataStreamThread, "tcc317x_data_thread", LOW_PRIORITY_10, moduleidx);
			
			if(ThreadID)
			{
				TcpalPrintLog((I08S *) "[  TDMB_CONTROL  ] Can not make tcc317x_tdmb_thread !!\n");
				return OMX_ErrorBadParameter;
			}						
		}
	}
	return TCC317X_RETURN_SUCCESS;
}

I32S TCC317XTDMB_Close(I32S moduleidx, I32S commandInterface)
{
	I32S ret=TCC317X_RETURN_SUCCESS;

	if((moduleidx == 0 && TCC317XTDMB_OpenFlagGet(moduleidx) != 1) ||
		(moduleidx == 1 && TCC317XTDMB_OpenFlagGet(moduleidx) != 1)){
		TcpalPrintLog((I08S *) "[  TDMB_CONTROL  ] [%d] Device is Not Opend ..\n", moduleidx);
		return OMX_ErrorBadParameter;
	}	

	if(giTSIFHandle[moduleidx]>=0)
	{			
		TCC_TSIF_Close(moduleidx, giTSIFHandle[moduleidx]);
		giTSIFHandle[moduleidx] = -1;
		TCCTHREAD_Join(TCC317XThreadID[moduleidx],NULL);
	}
		
	giCurrentActiveFreqency[moduleidx] = -1;
	gChannelSetFrequncy[moduleidx] = -1;

	/* Stream Parser Sema */
	if(Tcc317xStreamSema[moduleidx] != 0){
		TcpalDeleteSemaphore(&Tcc317xStreamSema[moduleidx]);
		Tcc317xStreamSema[moduleidx] = 0;
	}

	/* Tune Sema */
	if(Tcc317xTuneSema[moduleidx] != 0){	
		TcpalDeleteSemaphore(&Tcc317xTuneSema[moduleidx]);	
		Tcc317xTuneSema[moduleidx] = 0;
	}

	/* Fic Parser Sema */
	if(Tcc317xFICParserSema[moduleidx] != 0){	
		TcpalDeleteSemaphore(&Tcc317xFICParserSema[moduleidx]);	
		Tcc317xFICParserSema[moduleidx] = 0;
	}
		
#ifdef _DUAL_CHAIN_MODE_
	/* Keep On Power LDO High */
	if(TCC317XTDMB_OpenFlagGet(0) == 1 && TCC317XTDMB_OpenFlagGet(1) == 1){
		ret = Tcc317xApiStreamStop(moduleidx);
	}else {		
		Tcc317xApiClose(0);		
		Tcc317xApiClose(1);
		TCC317XTDMB_IO_PW_RESET_LOW(0);
		TCC317XTDMB_IO_PW_RESET_LOW(1);				
		TCC317XTDMB_IO_PW_OFF(moduleidx);		

		#ifdef USE_SIMPLE_AV_FIFO
		TCC317XTDMB_AvFifoBuffer_DeInit(moduleidx);
		#endif

		/* close I2C/TCCSPI Driver */
		if (commandInterface == TCC317X_IF_TCCSPI){
			Tcc317xTccspiClose(0);
			Tcc317xTccspiClose(1);
		}else if (commandInterface == TCC317X_IF_I2C){
			Tcc317xI2cClose(0);
			Tcc317xI2cClose(1);
		}else{
			Tcc317xTccspiClose(0);
			Tcc317xTccspiClose(1);
		}
	}

	TCC317XTDMB_OpenFlagSet(moduleidx, 0);	
#else
	ret = Tcc317xApiClose(moduleidx);
	if(ret =! 0)
		ret = OMX_ErrorBadParameter;

	TCC317XTDMB_IO_PW_RESET_LOW(moduleidx);
	
	if(TCC317XTDMB_OpenFlagGet(0) == 1 && TCC317XTDMB_OpenFlagGet(1) == 1){
		/* LDO Keep on */
	}else
	TCC317XTDMB_IO_PW_OFF(moduleidx);

#ifdef USE_SIMPLE_AV_FIFO
	TCC317XTDMB_AvFifoBuffer_DeInit(moduleidx);
#endif

	/* close I2C/TCCSPI Driver */
	if (commandInterface == TCC317X_IF_TCCSPI)
		Tcc317xTccspiClose(moduleidx);
	else if (commandInterface == TCC317X_IF_I2C)
		Tcc317xI2cClose(moduleidx);
	else
		Tcc317xTccspiClose(moduleidx);	

	TCC317XTDMB_OpenFlagSet(moduleidx, 0);	
#endif /* _DUAL_CHAIN_MODE_ */

	return ret;
}

I32S TCC317XTDMB_ChannelScan(I32S moduleidx, I32S channel)
{
	I32S freq; 
	I32S bandwidth;
	I32S ret = OMX_ErrorBadParameter;
	I32S serviceIndex, searchIndex, iFoundServiceCount = 0;	
	
	//TcpalSemaphoreLock(&Tcc317xTuneSema[moduleidx]);
	giDABServiceCount[moduleidx] = 0;
	giDMBServiceCount[moduleidx] = 0;
	giDATAServiceCount[moduleidx] = 0;
	giTOTALServiceCount[moduleidx] = 0;
	giDABPLUSServiceCount[moduleidx] = 0;
		
	giCurrentActiveFreqency[moduleidx] = -1;
	gChannelSetFrequncy[moduleidx] = -1;
	freq = TDMBSPACE_GetFrequency(channel);
	if (0 < freq) {
		giCurrentActiveFreqency[moduleidx] = freq;

		TCC_HWDMX_Stop(giTSIFHandle[moduleidx]);		
		ret = Tcc317xApiChannelSelect(moduleidx, freq, NULL);
		Tcc317xStreamParserInit (moduleidx);		
		TCC317XTDMB_StreamThreadRun(moduleidx);		
		Tcc317xMonitoringInit(moduleidx, 0);
		TCC_HWDMX_Start(giTSIFHandle[moduleidx]);

		if(ret == TCC317X_RETURN_SUCCESS)
			ret = Tcc317xApiWaitLock(moduleidx);

		DAB_Buff_Init(moduleidx);
		Parsing_Buffer_Init(moduleidx, 0);
		
		if(ret == 0){		
			TCC317XTDMB_SetState(moduleidx,STATE_SEARCHING);			
			for(searchIndex = 0; searchIndex < 300; searchIndex++)
			{	
				if(TCC317XTDMB_GetState(moduleidx) == STATE_SEARCHED)
				{
					for(serviceIndex = 0; serviceIndex < giTOTALServiceCount[moduleidx]; serviceIndex++)
					{								
#if 0
						switch(g_tChannelInfo[moduleidx][serviceIndex].Channel_Type)
						{
						case 1:
							g_tChannelInfo[moduleidx][serviceIndex].Channel_Id = SRVTYPE_DAB;
							break;
						case 2:
							g_tChannelInfo[moduleidx][serviceIndex].Channel_Id = SRVTYPE_DMB;
							break;
						case 3: 						
							g_tChannelInfo[moduleidx][serviceIndex].Channel_Id = SRVTYPE_DATA;
							break;
						case 4:
							g_tChannelInfo[moduleidx][serviceIndex].Channel_Id = SRVTYPE_DABPLUS;
							break;
						default:
							g_tChannelInfo[moduleidx][serviceIndex].Channel_Id = SRVTYPE_NONE;
							break;
						}						
#endif						
						g_tChannelInfo[moduleidx][serviceIndex].moduleidx = moduleidx;
						//ALOGD("[  TDMB_CONTROL  ] A/V [%d] %s:%x:%u",moduleidx, g_tChannelInfo[moduleidx][serviceIndex].Service_Label, g_tChannelInfo[moduleidx][serviceIndex].Service_Id, g_tChannelInfo[moduleidx][serviceIndex].Service_Id);
						omx_tcc317x_tuner_component_searched_notify((void *)&g_tChannelInfo[moduleidx][serviceIndex], ++iFoundServiceCount);						
					}
					break;
				}	
				usleep(5000);
			}
		}else
			ret = OMX_ErrorBadParameter;
		
		TcpalPrintLog((I08S *) "[  TDMB_CONTROL  ] channel set [%d], Frequency [%d]\n", ret, freq);
	}	
	//TcpalSemaphoreUnLock(&Tcc317xTuneSema[moduleidx]);
	return ret;
}

I32S TCC317XTDMB_ChannelFreqScan(I32S moduleidx, I32S freq)
{
	I32S bandwidth;
	I32S ret = OMX_ErrorBadParameter;
	I32S serviceIndex, searchIndex, iFoundServiceCount = 0;	
	
	//TcpalSemaphoreLock(&Tcc317xTuneSema[moduleidx]);
	giDABServiceCount[moduleidx] = 0;
	giDMBServiceCount[moduleidx] = 0;
	giDATAServiceCount[moduleidx] = 0;
	giTOTALServiceCount[moduleidx] = 0;
	giDABPLUSServiceCount[moduleidx] = 0;
		
	giCurrentActiveFreqency[moduleidx] = -1;
	gChannelSetFrequncy[moduleidx] = -1;
	if (0 < freq) {
		giCurrentActiveFreqency[moduleidx] = freq;

		TCC_HWDMX_Stop(giTSIFHandle[moduleidx]);
		ret = Tcc317xApiChannelSelect(moduleidx, freq, NULL);
		Tcc317xStreamParserInit (moduleidx);
		TCC317XTDMB_StreamThreadRun(moduleidx);		
		Tcc317xMonitoringInit(moduleidx, 0);
		TCC_HWDMX_Start(giTSIFHandle[moduleidx]);

		if(ret == TCC317X_RETURN_SUCCESS)
			ret = Tcc317xApiWaitLock(moduleidx);

		DAB_Buff_Init(moduleidx);
		Parsing_Buffer_Init(moduleidx, 0);
		
		if(ret == 0){			
			TCC317XTDMB_SetState(moduleidx,STATE_SEARCHING);
			for(searchIndex = 0; searchIndex < 300; searchIndex++)
			{	
				if(TCC317XTDMB_GetState(moduleidx) == STATE_SEARCHED)
				{
					for(serviceIndex = 0; serviceIndex < giTOTALServiceCount[moduleidx]; serviceIndex++)
					{	 							
#if 0
						switch(g_tChannelInfo[moduleidx][serviceIndex].Channel_Type)
						{
						case 1:
							g_tChannelInfo[moduleidx][serviceIndex].Channel_Id = SRVTYPE_DAB;
							break;
						case 2:
							g_tChannelInfo[moduleidx][serviceIndex].Channel_Id = SRVTYPE_DMB;
							break;
						case 3:							
							g_tChannelInfo[moduleidx][serviceIndex].Channel_Id = SRVTYPE_DATA;
							break;
						case 4:
							g_tChannelInfo[moduleidx][serviceIndex].Channel_Id = SRVTYPE_DABPLUS;
							break;
						default:
							g_tChannelInfo[moduleidx][serviceIndex].Channel_Id = SRVTYPE_NONE;
							break;
						}						
#endif						
						g_tChannelInfo[moduleidx][serviceIndex].moduleidx = moduleidx;
						//ALOGD("[  TDMB_CONTROL  ] TPEG [%d] %s:%x:%u",moduleidx, g_tChannelInfo[moduleidx][serviceIndex].Service_Label, g_tChannelInfo[moduleidx][serviceIndex].Service_Id, g_tChannelInfo[moduleidx][serviceIndex].Service_Id);
						omx_tcc317x_tuner_component_searched_notify((void *)&g_tChannelInfo[moduleidx][serviceIndex], ++iFoundServiceCount);						
					}
					break;
				}	
				usleep(5000);
			}
		}else
			ret = OMX_ErrorBadParameter;
		
		TcpalPrintLog((I08S *) "[  TDMB_CONTROL  ] channel set [%d], Frequency [%d]\n", ret, freq);
	}
	//TcpalSemaphoreUnLock(&Tcc317xTuneSema[moduleidx]);
	return ret;
}

I32S TCC317XTDMB_ServiceSelect(I32S moduleidx, I32U uiServiceType, I08U *pucService)
{
	I32S ret;
	Tcc317xService tTDMBService;
	
	tTDMBService.pType = (pucService[0] >> 3) & 1;
	tTDMBService.identifier = (pucService[6] & 0x3f);
	tTDMBService.cuSize = ((pucService[3] & 3) << 8) | (pucService[4]);
	tTDMBService.startCu = ((pucService[1] & 3) << 8) | (pucService[2]);
	tTDMBService.reConfig = 0x02;
	tTDMBService.pLevel = pucService[0] & 0x07;
	tTDMBService.bitrate = pucService[5];
	
	if(pucService[0] & 0x10) /* DMB service */
	{
		tTDMBService.rsOn = 1;
		tTDMBService.serviceType = SRVTYPE_DMB;
	}else{
		tTDMBService.rsOn = 0;
		switch(uiServiceType)
		{
		          case 1:
		      		tTDMBService.serviceType = SRVTYPE_DAB;
		          		break;
		          case 2:
		      		tTDMBService.serviceType = SRVTYPE_DMB;
		          		break;
		          case 3:
		      		tTDMBService.serviceType = SRVTYPE_DATA;
		          		break;
		          case 4:
		      		tTDMBService.serviceType = SRVTYPE_DABPLUS;
		          		break;
		          default:
		      		tTDMBService.serviceType = SRVTYPE_DAB;
		          		break;
		}

	}

#if 0 /* Test Log */
	ALOGD("[ CC ] Ptype(%d), iden(%d), cusize(%d), startCu(%d), reConfig(%d), PLevel(%d), bitrate(%d), rsOn(%d), Service Type(%d)",
		tTDMBService.pType,
		tTDMBService.identifier,
		tTDMBService.cuSize,
		tTDMBService.startCu,
		tTDMBService.reConfig,
		tTDMBService.pLevel,
		tTDMBService.bitrate,
		tTDMBService.rsOn,
		tTDMBService.serviceType
		);
#endif

	ret = Tcc317xApiAddService(moduleidx, &tTDMBService);
	if(0 != ret)
		ret =  OMX_ErrorBadParameter;
	
	return ret;
}


I32S TCC317XTDMB_FrequencySet(I32S moduleidx, I32S iFreqKhz, I32S bLockOn)
{	
	I32S ret;

	ret = Tcc317xApiChannelSearch(moduleidx, iFreqKhz, NULL);	
	if(ret != 0)		
		return OMX_ErrorBadParameter;	

	Tcc317xStreamParserInit (moduleidx);
	Tcc317xMonitoringInit(moduleidx, 0);	
	if(CurrentNumOfDiversity>1)
		Tcc317xMonitoringInit(moduleidx, 1);
	
	if (bLockOn)
		ret = Tcc317xApiWaitLock(moduleidx);

	giCurrentActiveFreqency[moduleidx] = iFreqKhz;
	TcpalPrintLog((I08S *) "[  TDMB_CONTROL  ] Success frequency[%d:%d]\n", moduleidx, iFreqKhz);
	return ret;
}

#ifdef _MULTI_SERVICE_
I32S TCC317XTDMB_ChannelSet(I32S moduleidx, I32U *uiARG, I32S bLockOn, I32U ChControl)
{
	I32S ret,i, ChState = ChControl;
	I32S MaxServiceInfo = 7;
	I08U ucService[MaxServiceInfo];
	int setResult[2] = {0, 0};

#if defined (_STATE_GET_TRANSMISSIONMODE_)
	OFDM_READFLAG = 0;
#endif

	if(gChannelSetFrequncy[moduleidx] != uiARG[2]){
		ALOGD("[ CC ] [%d] Frequency Change (%d) -> (%d)",
			moduleidx,gChannelSetFrequncy[moduleidx],uiARG[2]);
		gChannelSetFrequncy[moduleidx] = uiARG[2];
		ChState = CHANNEL_DefaultSet;
	}

	/* copy seven service infomation */
	for(i=0;i<MaxServiceInfo;i++){
		ucService[i] = uiARG[4+i];
	}

	switch(ChState)
	{
		case CHANNEL_UNREG:
			ret = Tcc317xApiRemoveService(moduleidx, ucService[6]);
			if(ret != 0){
				ALOGE("[ CC ] [%d] Fail Remove Service (%d)",moduleidx, ucService[6]);
				ret = OMX_ErrorBadParameter;
			}else
				ALOGI("[ CC ] [%d] Remove Service (%d)",moduleidx, ucService[6]);
			break;

		case CHANNEL_REG:
			ret = TCC317XTDMB_ServiceSelect(moduleidx, uiARG[11], ucService);
			if(ret != 0){
				ALOGE("[ CC ] [%d] Fail Add Service Set (%d)",moduleidx, ucService[6] & 0x3f);
				ret = OMX_ErrorBadParameter;
			}else
				ALOGI("[ CC ] [%d] Add Service Set (%d)",moduleidx, ucService[6] & 0x3f);

			setResult[0] = moduleidx;
			setResult[1] = (ret == 0)? 1:0;
			omx_tcc317x_tuner_component_set_channel_notify((void*)setResult);
			break;

		default:	/* CHANNEL_DefaultSet */
			tsem_down(gpAVFifoSema[moduleidx]);
			QInitBuffer(&gstAVFifo[moduleidx], AV_FIFO_SIZE, gpAVFifoBuffer[moduleidx]);
			tsem_up(gpAVFifoSema[moduleidx]);

			ret = TCC317XTDMB_FrequencySet(moduleidx, uiARG[2], bLockOn);
			if(ret == 0)
			{
				TCC_HWDMX_Stop(giTSIFHandle[moduleidx]);
				TCC317XTDMB_StreamThreadRun(moduleidx);
				TCC_HWDMX_Start(giTSIFHandle[moduleidx]);

				ret = TCC317XTDMB_ServiceSelect(moduleidx, uiARG[11], ucService);
				if(ret != 0){
					ALOGE("[ CC ] [%d] Fail Service Defaultset (%d)",moduleidx, ucService[6] & 0x3f);
					ret = OMX_ErrorBadParameter;
				}else
					ALOGI("[ CC ] [%d] Service Defaultset (%d)",moduleidx, ucService[6] & 0x3f);

				setResult[0] = moduleidx;
				setResult[1] = (ret == 0)? 1:0;
				omx_tcc317x_tuner_component_set_channel_notify((void*)setResult);
			}
			else
				ret = OMX_ErrorBadParameter;
			break;
	}

	return ret;
}
#else
I32S TCC317XTDMB_ChannelSet(I32S moduleidx, I32U *uiARG, I32S bLockOn)
{
	I32S ret,i;
	I32S MaxServiceInfo = 7;
	I08U ucService[MaxServiceInfo];
	
#if defined (_STATE_GET_TRANSMISSIONMODE_)
	OFDM_READFLAG = 0;
#endif
	tsem_down(gpAVFifoSema[moduleidx]);
	QInitBuffer(&gstAVFifo[moduleidx], AV_FIFO_SIZE, gpAVFifoBuffer[moduleidx]);
	tsem_up(gpAVFifoSema[moduleidx]);

	ret = TCC317XTDMB_FrequencySet(moduleidx, uiARG[2], bLockOn);
	if(ret == 0)
	{	
		int setResult[2] = {0, 0};
		
		/* Copy to Seven kinds of service information */
		for(i=0;i<MaxServiceInfo;i++){
			ucService[i] = uiARG[4+i];
		}

		TCC_HWDMX_Stop(giTSIFHandle[moduleidx]);
		TCC317XTDMB_StreamThreadRun(moduleidx);
		TCC_HWDMX_Start(giTSIFHandle[moduleidx]);

		ret = TCC317XTDMB_ServiceSelect(moduleidx, uiARG[11], ucService);
		if(ret != 0)
			ret = OMX_ErrorBadParameter;
	
		setResult[0] = moduleidx;
		setResult[1] = (ret == 0)? 1:0;
	
		omx_tcc317x_tuner_component_set_channel_notify((void*)setResult);
	}
	else		
		ret = OMX_ErrorBadParameter;

	return ret;
}
#endif /*_MULTI_SERVICE_*/

I32S TCC317XTDMB_ReadFIFO(I32S iHandle, I08U *pucData, I32U uiSize)
{
	I32S read_data, moduleidx = 0;
		
	tsem_down(gpAVFifoSema[moduleidx]);
	read_data = QGetDataEX(&gstAVFifo[moduleidx], (char *)pucData, (long)uiSize);
	tsem_up(gpAVFifoSema[moduleidx]);
	if(read_data == 0)
	{
		//TcpalPrintLog((I08S *)"[  TDMB_CONTROL  ] SIZE[%d] :: [ERROR]There are no data to read !!\n", uiSize);
		usleep(5000);
	}

	return read_data;
} 

I32S TCC317XTDMB_AVServiceProcess(I32S moduleidx, I08U *pucData, I32U uiSize, I32S uiType, I32U SubChID)
{
	I32S write_data;
	
	tsem_down(gpAVFifoSema[moduleidx]);
	write_data = QPutData(&gstAVFifo[moduleidx],(char *)pucData,(long)uiSize);
	tsem_up(gpAVFifoSema[moduleidx]);
	
	//ALOGI("AV:%d", SubChID);
	if(write_data == 0)
	{		
		//TcpalPrintLog((I08S *)"[  TDMB_CONTROL  ] AV Service SIZE[%d] :: [ERROR] There are no Space in Fifo !!\n",  uiSize);
		usleep(5000); 
	}

	return write_data;
}

I32S TCC317XTDMB_DataServiceProcess(I32S moduleidx, I08U *pucData, I32U uiSize, I32S uiType, I32U SubChID)
{
	I32S write_data;

	//ALOGI("DATA:%d", SubChID);
	//ALOGI("[  TDMB_CONTROL  ] [%d] Data Stream -> Size:%d", moduleidx, uiSize);		
	omx_tcc317x_tuner_component_data_ch_notify(moduleidx, (void*)pucData, uiSize);
	return write_data;
}

I08U TCC317XTDMB_Get_Antenna(I32S moduleidx, I32U PcberAvg, DmbLock_t* Lockstate)
{	
	I32U retLevel = 0;
	I32U uiPCBER;

	if(Lockstate->CTO == 0 || Lockstate->CFO == 0){
		TcpalPrintLog((I08S *)"[  TDMB_CONTROL  ] [%d] Lock Fail (Antenna Level: 0)\n", moduleidx);
		return 0; /*retLevel 0*/
	}
	
	uiPCBER = (I32U)(PcberAvg);	
	
	if(uiPCBER <= 6000)
        		retLevel = 5;   // viterbi 10-6
	else if(uiPCBER <= 8000)
        		retLevel = 4;   // viterbi 10-5
	else if(uiPCBER <= 10000)
        		retLevel = 3;   // viterbi 10-4
	else if(uiPCBER <= 11000)
        		retLevel = 2;   // viterbi 10-3
	else if(uiPCBER <= 13000)
        		retLevel = 1;   // viterbi 10-3
	else if(uiPCBER > 13000)
        		retLevel = 0;   // viterbi 10-2

	return(retLevel);
}

I32S TCC317XTDMB_GetSignalStrength(I32S moduleidx, I32S diversityindex ,TCC317X_INFO* _info )
{
	I32U DSPVer, DDVer;
	DmbLock_t Lock;
	Tcc317xStatus_t Status;	

#if defined (_STATE_GET_TRANSMISSIONMODE_)
	mailbox_t _mailbox;

	if(OFDM_READFLAG == 0){
		Tcc317xApiMailboxRead (moduleidx, diversityindex, ((0x04<<11)| 0x01), &_mailbox);
		Transmission_mode = (_mailbox.data_array[1]&0xFFFF);

		if((_mailbox.data_array[1]&0xFFFF) == 0)
			OFDM_READFLAG = 0;
		else
			OFDM_READFLAG = 1;
	}
#endif /* STATE_TRANSMISSION_MODE */

	Tcc317xApiLockStatus(moduleidx, diversityindex, &Lock);
	Tcc317xGetSignalState(moduleidx, diversityindex, &Status);
	Tcc317xApiGetDSPCodeVersion(moduleidx, diversityindex, &DSPVer);
	Tcc317xApiGetDriverVersion(&DDVer);

	_info->SNR = Status.status.snrDb.avgValue;
	_info->RSSI = Status.status.rssi.currentValue;
	_info->PCBer = Status.status.pcber.avgValue;
	_info->TSPer = Status.status.tsper.avgValue;
	_info->FICBer = Status.fic_pcber;
	_info->MSCBer = Status.msc_pcber;
	_info->ViterBer = Status.status.viterbiber.avgValue;
	_info->RFLoopGain = Status.rfLoopGain;
	_info->BBLoopGain = Status.bbLoopGain;
	_info->DSPVer = DSPVer;
	_info->DDVer = DDVer;
	_info->DSPBuildDate = Tcc317x_DSPBuildDate;

	if(Lock.CTO == 1 && Lock.CFO == 1)
		_info->OFDM_Lock = 1;
	else
		_info->OFDM_Lock = 0;

#if defined (_SIGNAL_STATE_) 
	TcpalPrintLog((I08S *)"[%d][%d] RSSI(%d), SNR(%d , %d), PCBER(%d, %d), VBER(%d, %d), TSPER(%d, %d)\n",
		moduleidx, diversityindex,Status.status.rssi.currentValue, Status.status.snrDb.currentValue, Status.status.snrDb.avgValue,
		Status.status.pcber.currentValue, Status.status.pcber.avgValue, Status.status.viterbiber.currentValue,
		Status.status.viterbiber.avgValue, Status.status.tsper.currentValue, 	Status.status.tsper.avgValue);
#endif
	return TCC317XTDMB_Get_Antenna(moduleidx, Status.status.pcber.avgValue, &Lock);
}

I32S TCC317XTDMB_FICProcess(I32S moduleidx, I08U *pucData, I32U uiSize, I32U uiCRC, I32S uiType)
{
	I32S i;
	I32U uiNumberofFib, ret;
	uiNumberofFib = uiSize/32;	
	FIC_INFO_T * pFicInfo = DMB_Get_FIC_Info(moduleidx);
	
	for(i=0;i<uiNumberofFib;i++)
	{		
		if(pucData[i<<5] == NULL || pucData == NULL){
			TcpalPrintLog ((I08S*)"[  TDMB_CONTROL  ] [%d] fic buffer data is null. (%d)\n",moduleidx, uiSize);
			return TCC317X_RETURN_FAIL;
		}

		if(uiCRC)
			ret = FIBDecoding(moduleidx, &pucData[i<<5], (uiCRC>>i)&0x0001);
		else
			ret = FIBDecoding(moduleidx, &pucData[i<<5], 0);
		
		if(pFicInfo->psTMC_EWS->NumMsg)			
			omx_tcc317x_tuner_component_ews_notify(moduleidx, (void*)pFicInfo->psTMC_EWS);			

		if(gFicParserInfo[moduleidx].gucFig_RECONF == 1)			
       				TcpalPrintLog ((I08S*)"[  TDMB_CONTROL  ] [%d] RECONFIG Flag ON\n",moduleidx);
	}

	TcpalSemaphoreLock(&Tcc317xFICParserSema[moduleidx]);		
	ret = DMB_IsChannelUpdate(moduleidx);
	if(ret == CH_UPDATE_FULL_DATA || ret == CH_UPDATE_ESSENTIAL_DATA)
	{
		PENSEMBLEDB pNewEnsembleDB;
		PENSEMBLEDB status;
		if(TCC317XTDMB_GetState(moduleidx)!=STATE_SEARCHING){
			TcpalSemaphoreUnLock(&Tcc317xFICParserSema[moduleidx]);
			return TCC317X_RETURN_SUCCESS;
		}

		DMB_SortForFICParser(moduleidx);
		pNewEnsembleDB = DMB_GetEnsembleDB(moduleidx);
		status = DMB_TakeEnsembleTreeFormFIC (moduleidx, pNewEnsembleDB );
		pNewEnsembleDB->FreqIndex = giCurrentActiveFreqency[moduleidx];
		
		if(status == NULL)
		{
			DMB_UnlinkEnsembleDB( pNewEnsembleDB );
			DMB_FreeEnsembleDB(pNewEnsembleDB);
			TcpalPrintLog ((I08S*)"[  TDMB_CONTROL  ] DMBDB_GetNewEnsembleTree ERROR!!\n");
			TCC317XTDMB_SetState(moduleidx,STATE_SEARCHED);
			TcpalSemaphoreUnLock(&Tcc317xFICParserSema[moduleidx]);
			return TCC317X_RETURN_FAIL;
		}		
		giDABServiceCount[moduleidx] = 0; 
		giDMBServiceCount[moduleidx] = 0;
		giDATAServiceCount[moduleidx] = 0;
		giTOTALServiceCount[moduleidx] = 0;
		giDABPLUSServiceCount[moduleidx] = 0;
		
		DMB_GetEnsembleServiceNumber( pNewEnsembleDB, &giDMBServiceCount[moduleidx], &giDABServiceCount[moduleidx], 
			&giDATAServiceCount[moduleidx], &giDABPLUSServiceCount[moduleidx], &giTOTALServiceCount[moduleidx], &g_tChannelInfo[moduleidx][0]);
				
		DMB_UnlinkEnsembleDB( pNewEnsembleDB );
		DMB_FreeEnsembleDB(pNewEnsembleDB);
		TCC317XTDMB_SetState(moduleidx, STATE_SEARCHED);

		/*
		TcpalPrintLog ((I08S*)"[  TDMB_CONTROL  ] (%d) DMB[%d], DAB[%d], DATA[%d], DAB+[%d], TOTAL[%d], FRQ[%d]",
			moduleidx, giDMBServiceCount[moduleidx], giDABServiceCount[moduleidx], giDATAServiceCount[moduleidx],
			giDABPLUSServiceCount[moduleidx], giTOTALServiceCount[moduleidx], giCurrentActiveFreqency[moduleidx]);
		*/
		
	}
	else if(ret == CH_UPDATE_NO_DATA) {/*TcpalPrintLog ((I08S*)"[  TDMB_CONTROL  ] CH_UPDATE_NO_DATA\n");*/}

	TcpalSemaphoreUnLock(&Tcc317xFICParserSema[moduleidx]);
	return TCC317X_RETURN_SUCCESS;
}


I32S TCC317XTDMB_IO_PW_OPEN(void)
{
	I32S iBoardType, ret;
	ST_CTRLINFO_ARG stCtrl;
	
	g_tcc_dxb_ctrl_fd = open(DXB_CTRL_DEV_FILE, O_RDWR | O_NDELAY);
	if (g_tcc_dxb_ctrl_fd < 0) {
		TcpalPrintLog((I08S *) "[  TDMB_CONTROL  ]  CANNOT open %s :: %d\n",DXB_CTRL_DEV_FILE, (I32S) g_tcc_dxb_ctrl_fd);
		return OMX_ErrorBadParameter;
	}

	iBoardType = BOARD_DXB_TCC3171;
	ret = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_SET_BOARD, &iBoardType);
	if(ret != 0) {	
		close(g_tcc_dxb_ctrl_fd);	
		g_tcc_dxb_ctrl_fd = -1;
		TcpalPrintLog((I08S *)"[  TDMB_CONTROL  ]  ioctl(0x%x) error :: %d\n", IOCTL_DXB_CTRL_SET_BOARD, ret);
		return OMX_ErrorBadParameter;
	}

	ret = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_GET_CTLINFO, &stCtrl);
	if(ret != 0) {	
		close(g_tcc_dxb_ctrl_fd);	
		g_tcc_dxb_ctrl_fd = -1;
		TcpalPrintLog((I08S *)"[  TDMB_CONTROL  ]  ioctl(0x%x) error :: %d\n",  IOCTL_DXB_CTRL_GET_CTLINFO, ret);
		return OMX_ErrorBadParameter;
	}
	return TCC317X_RETURN_SUCCESS;
}

I32S TCC317XTDMB_IO_PW_ON(I32S moduleidx)
{
	I32U deviceIdx, ret = OMX_ErrorBadParameter;
	
	deviceIdx = moduleidx;
	ret = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_PURE_ON, &deviceIdx);
	if(ret != 0)
		TcpalPrintLog((I08S *)"[  TDMB_CONTROL  ]  Power On Error :: %d\n", ret);

	usleep(10000);
	return ret;
}

I32S TCC317XTDMB_IO_PW_OFF(I32S moduleidx)
{
	I32U deviceIdx, ret = OMX_ErrorBadParameter;
			
	deviceIdx = moduleidx;
	ret = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_PURE_OFF, &deviceIdx);
	if(ret != 0)
		TcpalPrintLog((I08S *)"[  TDMB_CONTROL  ]  Power Off Error :: %d\n", ret);

	usleep(10000);
	return ret;
}


I32S TCC317XTDMB_IO_PW_RESET_LOW(I32S moduleidx)
{
	I32S deviceIdx, ret = OMX_ErrorBadParameter;

	deviceIdx = moduleidx;
	ret = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_RESET_LOW, &deviceIdx);	
	if(ret != 0)
		TcpalPrintLog((I08S *)"[  TDMB_CONTROL  ]  Power Reset Low Error :: %d\n", ret);

	usleep(10000);
	return ret;
}

I32S TCC317XTDMB_IO_PW_RESET_HIGH(I32S moduleidx)
{
	I32S deviceIdx, ret = OMX_ErrorBadParameter;
	
	deviceIdx = moduleidx;
	ret = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_RESET_HIGH, &deviceIdx);
	if(ret != 0)
		TcpalPrintLog((I08S *)"[  TDMB_CONTROL  ]  Power Reset High Error :: %d\n", ret);

	usleep(10000);
	return ret;
}

I32S TCC317XTDMB_IO_PW_RESET(I32S moduleidx)
{
	I32U deviceIdx, ret = OMX_ErrorBadParameter;
	
	deviceIdx = moduleidx;
	ret = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_RESET, &deviceIdx);
	if(ret != 0)
		TcpalPrintLog((I08S *)"[  TDMB_CONTROL  ]  Power Reset Error :: %d\n", ret);

	usleep(10000);
	return ret;
}

