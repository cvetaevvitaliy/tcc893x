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
#define LOG_TAG	"TCC351X_TDMB"
#include <utils/Log.h>

//#include "globals.h"
#include "TC3X_Common.h"
#include "TCC351X_BOOT_TDMB.h"
#include "TDMBInfo.h"
#include "TCC351X_tdmb.h"

#include "tsemaphore.h"
#include "SimpleBuffer.h"


//#define DMB_BUFFER_SIZE 1024 * 30
//static unsigned char g_DMBBuffer[DMB_BUFFER_SIZE];
extern unsigned char g_DMBBuffer[];

static TENSEMBLEDB			gpEnsembleDBList[MAX_ENSEMBLEDB];
static TSERVICEDB			gpServiceDBList[MAX_SERVICEDB];
static TCHANNELDB			gpChannelDBList[MAX_CHANNELDB];

static TDMB_INFO			g_tChannelInfo[MAX_CHANNELDB];
static unsigned int giDABServiceCount, giDMBServiceCount, giDATAServiceCount, giTOTALServiceCount, giDABPLUSServiceCount;
static int giCurrentActiveFreqency;
static int giSearchStop = 0;
static	E_TDMB_STATE		eTDMBState;

#define		USE_SIMPLE_AV_FIFO
#ifdef		USE_SIMPLE_AV_FIFO
static tsem_t* 	gpAVFifoSema = NULL;
static SRBUFFER gstAVFifo;
static char		*gpAVFifoBuffer;
#define	AV_FIFO_SIZE	500*1024
#endif

long long g_FIC_Tick_ms = 0;

#define	PRINTF				ALOGD
#define _PCBER_RESOLUTION_ 100000

void TC3X_IO_CSPI_Reg_Read_Wrap(int iHandleNum, unsigned int Addr, unsigned char *data, unsigned char bytesize);
void TC3X_IO_CSPI_Reg_Write_Wrap(int iHandleNum, unsigned int Addr, unsigned char data);
void TC3X_IO_I2C_Reg_Read_Wrap(int iHandleNum, unsigned int Addr, unsigned char *data, unsigned char bytesize);
void TC3X_IO_I2C_Reg_Write_Wrap(int iHandleNum, unsigned int Addr, unsigned char data);

extern void omx_tcc351x_tuner_component_searched_notify(void *pChannelInfo, int searchedCount);
extern void omx_tcc351x_tuner_component_ews_notify(void *pst_EWS);

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
 		
 		// offset 출력.
 		sprintf(ucPrintBuffer,"0x%08lx  ", (long)(s-addr));
 		strcat(ucWholePrintBuffer, ucPrintBuffer);	
 		// 16 bytes 단위로 내용 출력.
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
 		
 		// offset 출력.
 		sprintf(ucPrintBuffer,"0x%08lx  ", (long)(s-addr));
		strcat(ucWholePrintBuffer, ucPrintBuffer);
 		
 		// 16 bytes 단위로 출력하고 남은 것 출력.
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


static void dmb_TimeDelayMs(unsigned int uiTimeMs) 
{
	usleep(uiTimeMs*1000);
}

E_TDMB_STATE TCC351XTDMB_SetState(E_TDMB_STATE eState)
{
	eTDMBState = eState;
	return eTDMBState;
}

E_TDMB_STATE TCC351XTDMB_GetState(void)
{	
	return eTDMBState;
}

int TCC351XTDMB_Open(int iCountryCode, int iCmdInterface,int iDataInterface)
{
	int size;
	TDMB_INITDBINFO initDBInfo;
	TC3X_API_Init(0, 1);
	TC3X_API_IOOpen(0, iCmdInterface, iDataInterface);		
	TC3X_API_BBOpen(0);
	TC3X_API_BBPreInit(0, TC3X_STD_DMB);
	TC3X_API_PreSet_PLLData(0, 0x60,0x06,0x60,0x03,19200);

	if(iCmdInterface == TC3X_IF_CSPI )
	{
		//SPI
		DMBPROTOCOL_initEx(TC3X_IO_CSPI_Reg_Read_Wrap, TC3X_IO_CSPI_Reg_Write_Wrap, dmb_TimeDelayMs);
		
	}
	else
	{
		//I2C
		DMBPROTOCOL_initEx(TC3X_IO_I2C_Reg_Read_Wrap, TC3X_IO_I2C_Reg_Write_Wrap, dmb_TimeDelayMs);
	}
	
	TC3X_API_ColdBootTable(0, &TCC351X_BOOT_DATA_TDMB[0],TCC351X_BOOT_SIZE_TDMB, CMD_BB_COLDBOOT_NORMAL);
	TC3X_API_BBInit(0);

    #if 1
    DMB_BufferInit(g_DMBBuffer,0);
    #else
	size = DMB_BufferInit(g_DMBBuffer, DMB_BUFFER_SIZE, _NumOfSvc);
	if(size > DMB_BUFFER_SIZE)
	{
		PRINTF("g_DMBBuffer is small.\n");
		return 1;
	}
	#endif

	initDBInfo.pEnsembleDBList = gpEnsembleDBList;
	initDBInfo.pServiceDBList = gpServiceDBList;
	initDBInfo.pChannelDBList = gpChannelDBList;

	initDBInfo.ensembleDBMaxSize = MAX_ENSEMBLEDB;
	initDBInfo.serviceDBMaxSize = MAX_SERVICEDB;
	initDBInfo.channelDBMaxSize = MAX_CHANNELDB;
	DMB_InitDB(&initDBInfo);
	TDMBSPACE_CTuneSpace(iCountryCode);	
	giCurrentActiveFreqency = -1;
#ifdef		USE_SIMPLE_AV_FIFO
	memset(&gstAVFifo, 0, sizeof(SRBUFFER));
	gpAVFifoBuffer = (char *)malloc(AV_FIFO_SIZE);
	if(gpAVFifoBuffer == NULL)
		return 1;
	memset(gpAVFifoBuffer, 0, sizeof(SRBUFFER));

	QInitBuffer(&gstAVFifo, AV_FIFO_SIZE, gpAVFifoBuffer);

	gpAVFifoSema = (tsem_t*)TCC_calloc(1,sizeof(tsem_t));
	if(gpAVFifoSema == NULL )
	{
		free(gpAVFifoBuffer);
		gpAVFifoBuffer = NULL;
		return 1;
	}	
	tsem_init(gpAVFifoSema, 1);
#endif	
	PRINTF("%s Success, Band[%d]", __func__, iCountryCode);
	return 0;
}

int TCC351XTDMB_Close(void)
{
	TC3X_API_Close(0);
	giCurrentActiveFreqency = -1;
	
#ifdef		USE_SIMPLE_AV_FIFO
	if( gpAVFifoBuffer )
		free(gpAVFifoBuffer);
	gpAVFifoBuffer = NULL;
	memset(&gstAVFifo, 0, sizeof(SRBUFFER));
	if(gpAVFifoSema)
	{
		tsem_deinit(gpAVFifoSema);
		TCC_free(gpAVFifoSema);
		gpAVFifoSema = NULL;
	}	
#endif		
	return 0;
}

int TCC351XTDMB_ServiceControl(unsigned int uiServiceType, unsigned char *pucService, int icontrol)
{
	int ret;
	TDMBService_t tTDMBService;		
	tTDMBService.PType = (pucService[0] >> 3) & 1;
	tTDMBService.PLevel = pucService[0] & 0x07;
	tTDMBService.StartCU = ((pucService[1] & 3) << 8) | (pucService[2]);
	tTDMBService.CUSize = ((pucService[3] & 3) << 8) | (pucService[4]);

	tTDMBService.BitRate = pucService[5];
	tTDMBService.ReConfig = 0x02;
	tTDMBService.SubChID = (pucService[6] & 0x3f);
	if(pucService[0] & 0x10)		/* DMB service */
	{
		tTDMBService.RS_ON = 1;
		tTDMBService.ServType = SRVTYPE_DMB;
	}
	else
	{
		tTDMBService.RS_ON = 0;
		switch(uiServiceType)
		{
	          case 1:
	      		tTDMBService.ServType = SRVTYPE_DAB;
	          break;
	          case 2:
	      		tTDMBService.ServType = SRVTYPE_DMB;
	          break;
	          case 3:
	      		tTDMBService.ServType = SRVTYPE_DATA;
	          break;
	          case 4:
	      		tTDMBService.ServType = SRVTYPE_DABPLUS;
	          break;
	          default:
	      		tTDMBService.ServType = SRVTYPE_DAB;
	          break;
		}
	}

/****
	//Frequency : 181280 (UI)
	
	tTDMBService.ServType = 0x00000004;
	tTDMBService.PType = 0x00000001;
	tTDMBService.SubChID = 0x00000000;
	tTDMBService.CUSize = 0x00000114;
	tTDMBService.StartCU = 0x00000000;
	tTDMBService.ReConfig = 0x00000002;
	tTDMBService.RS_ON = 0x00000001;
	tTDMBService.PLevel = 0x00000002;
	tTDMBService.BitRate = 0x0000002e;
*****/	
	/*icontrol : 1:Select service, 0:Release service
	*/
	ret = TC3X_API_SET_TDMB_Service(0, icontrol, &tTDMBService);
	if(ret == -1)
		return 1;
	
	return 0;


}

int TCC351XTDMB_ServiceSelect(unsigned int uiServiceType, unsigned char *pucService)
{
	return TCC351XTDMB_ServiceControl(uiServiceType, pucService, 1);
	
}

int TCC351XTDMB_ServiceRelease(unsigned int uiServiceType, unsigned char *pucService)
{
	return TCC351XTDMB_ServiceControl(uiServiceType, pucService, 0);
}

int TCC351XTDMB_ChannelSet(int iChannel)
{
    int iFreq;        
    iFreq=TDMBSPACE_GetFrequency(iChannel);
	PRINTF("%s index[%d]frequency[%d]", __func__, iChannel, iFreq);
    return TCC351XTDMB_FrequencySet(iFreq, 1);
}

int TCC351XTDMB_FrequencySet(int iFreqKhz, int bLockOn)
{	
	int ret;

    g_FIC_Tick_ms = TC3X_IO_GET_TIMECNT_ms();
	
	giCurrentActiveFreqency = -1;
	ret = TC3X_API_SetFreq(0, 1500, iFreqKhz, NULL);
	if(ret != 1)
		return 1;
	if (bLockOn)
		ret = TC3X_API_GetTDMB_Lock_Wait(0, 0);
	else
		ret = 1;
	if(ret != 1)
		return 1;	

	giCurrentActiveFreqency = iFreqKhz;
	PRINTF("%s Success frequency[%d]", __func__, iFreqKhz);
	return 0;
}

int TCC351XTDMB_ParseData(unsigned char *pucData, unsigned int uiSize)
{	
	int ret;
	ret = TC3X_API_ParseData (0, 0, pucData, uiSize);
	if(ret != 1)
		return 1;	
	return 0;
}

int TCC351XTDMB_AVServiceProcess(int moduleidx, unsigned char *pucData, unsigned int uiSize, int uiType)
{
	int write_data;
	//PRINTF("%s SIZE[%d] \n", __func__, uiSize);
	//HexDump(pucData, 32);
#ifdef		USE_SIMPLE_AV_FIFO
	tsem_down(gpAVFifoSema);	
	write_data = QPutData(&gstAVFifo,(char *)pucData,(long)uiSize);
	tsem_up(gpAVFifoSema);
	if(write_data == 0)
	{
		PRINTF("%s SIZE[%d] :: [ERROR]There are no Space in Fifo !!!!\n", __func__, uiSize);
		usleep(5000);
	}
#endif
	return 0;
}

int TCC351XTDMB_DataServiceProcess(int moduleidx, unsigned char *pucData, unsigned int uiSize, int uiType)
{
	int write_data;
	PRINTF("%s SIZE[%d] \n", __func__, uiSize);
	HexDump(pucData, 32);
	return 0;
}

int TCC351XTDMB_ReadFIFO(int iHandle, unsigned char *pucData, unsigned int uiSize)
{
	int read_data;	
	//PRINTF("%s SIZE[%d]\n", __func__, uiSize);
#ifdef		USE_SIMPLE_AV_FIFO
	tsem_down(gpAVFifoSema);
	read_data = QGetDataEX(&gstAVFifo, (char *)pucData, (long)uiSize);
	tsem_up(gpAVFifoSema);
	if(read_data == 0)
	{
		//PRINTF("%s SIZE[%d] :: [ERROR]There are no data to read !!!!\n", __func__, uiSize);
		usleep(5000);
	}
#endif
	return read_data;
}

int TCC351XTDMB_FICProcess(int moduleidx, unsigned char *pucData, unsigned int uiSize, unsigned int uiCRC, int uiType)
{
	int i;
	unsigned int uiNumberofFib, ret;
	uiNumberofFib = uiSize/32;
	//PRINTF("%s[%d, %d, %d]\n", __func__, uiSize, uiCRC, uiType);

	FIC_INFO_T * pFicInfo = DMB_Get_FIC_Info();

	if(uiNumberofFib)
		g_FIC_Tick_ms = TC3X_IO_GET_TIMECNT_ms();

	for(i=0;i<uiNumberofFib;i++)
	{
		if(uiCRC)
		{
			FIBDecoding(moduleidx, &pucData[i<<5], (uiCRC>>i)&0x0001);				// FIB decoding

			if(pFicInfo->psTMC_EWS->NumMsg)
			{
//				ALOGW("todo. 1. Please insert EWS Processing \n");
				// todo. Please insert EWS Processing
				// Sample
				omx_tcc351x_tuner_component_ews_notify((void*)pFicInfo->psTMC_EWS);
			}

			if(gFicParserInfo[0].gucFig_RECONF == 1)
			{
        		PRINTF("[%d] RECONFIG Flag ON\n", __LINE__);
			}
		}
		else
		{
			FIBDecoding(moduleidx, &pucData[i<<5], 0);				// FIB decoding

			if(pFicInfo->psTMC_EWS->NumMsg)
			{				
//				ALOGW("todo. 2. Please insert EWS Processing \n");
				// todo. Please insert EWS Processing
				// Sample
				omx_tcc351x_tuner_component_ews_notify((void*)pFicInfo->psTMC_EWS);
			}

			if(gFicParserInfo[0].gucFig_RECONF == 1)
			{
        		PRINTF("[%d] RECONFIG Flag ON\n", __LINE__);
			}
		}
	}	

	ret = DMB_IsChannelUpdate();
	if(ret == CH_UPDATE_FULL_DATA)
	{
		PENSEMBLEDB pNewEnsembleDB;
		PENSEMBLEDB status;				
		if(TCC351XTDMB_GetState()!=STATE_SEARCHING)
			return 0;
		
		//PRINTF("%s CH_UPDATE_FULL_DATA\n", __func__);
		DMB_SortForFICParser();
		pNewEnsembleDB = DMB_GetEnsembleDB();
		status = DMB_TakeEnsembleTreeFormFIC ( pNewEnsembleDB );
		pNewEnsembleDB->FreqIndex = giCurrentActiveFreqency;
		PRINTF("%s:%s\n", pNewEnsembleDB->Label, pNewEnsembleDB->Service->Label);
		//PRINTF("%d:%d\n", pNewEnsembleDB->FreqIndex, pNewEnsembleDB->Service->ChannelCount);		
		if(status == NULL)
		{
			DMB_UnlinkEnsembleDB( pNewEnsembleDB );
			DMB_FreeEnsembleDB(pNewEnsembleDB);
			PRINTF("[DMBDB_GetNewEnsembleTree:%d] ERROR!!\n", __LINE__);
			TCC351XTDMB_SetState(STATE_SEARCHED);
			return 1;
		}
		giDABServiceCount = giDMBServiceCount = giDATAServiceCount = giTOTALServiceCount = giDABPLUSServiceCount = 0;
		DMB_GetEnsembleServiceNumber( pNewEnsembleDB, &giDMBServiceCount, &giDABServiceCount, &giDATAServiceCount, &giDABPLUSServiceCount, &giTOTALServiceCount, g_tChannelInfo);
		PRINTF("%s DMB[%d]DAB[%d]TOTAL[%d]",__func__, giDMBServiceCount, giDABServiceCount, giTOTALServiceCount);
		DMB_UnlinkEnsembleDB( pNewEnsembleDB );
		DMB_FreeEnsembleDB(pNewEnsembleDB);
		TCC351XTDMB_SetState(STATE_SEARCHED);
	}
	else if(ret == CH_UPDATE_NO_DATA)
	{
		//PRINTF("%s CH_UPDATE_NO_DATA\n", __func__);
		//TCC351XTDMB_SetState(STATE_SEARCHED);
	}
	return 0;
}


int TCC351XTDMB_GetMinChannel(void)
{
    return TDMBSPACE_GetMinChannel();
}

int TCC351XTDMB_GetMaxChannel(void)
{
    return TDMBSPACE_GetMaxChannel();
}

int TCC351XTDMB_SearchChannels(void)
{
	int min_ch, max_ch, i, j, ii, iFoundServiceCount;

	iFoundServiceCount = 0;
	min_ch = TDMBSPACE_GetMinChannel();
	max_ch = TDMBSPACE_GetMaxChannel();
	TCC351XTDMB_SetState(STATE_IDLE);	
	PRINTF("%s min[%d]max[%d]", __func__, min_ch, max_ch);
	for(i=min_ch; i<=max_ch; i++)
	{		
		if(giSearchStop == 1) {
			TCC351XTDMB_SetState(STATE_IDLE);	
			giSearchStop = 0;
			break;
		}
		giDABServiceCount = giDMBServiceCount = giDATAServiceCount = giTOTALServiceCount = giDABPLUSServiceCount = 0;
		TCC_TSIF_ResetRead();
		DAB_Buff_Init(0);
		Parsing_Buffer_Init(0, 0); // defined at parser
		if(TCC351XTDMB_ChannelSet(i) == 0)
		{
			TCC351XTDMB_SetState(STATE_SEARCHING);
			for(ii =0; ii < 300; ii++ )
			{	
				if(giSearchStop == 1) {
					TCC351XTDMB_SetState(STATE_IDLE);	
					break;
				}
				if(TCC351XTDMB_GetState()==STATE_SEARCHED)
				{
					for(j=0;j<giTOTALServiceCount;j++)
					{
						//PRINTF("%s upload channel information %d", __func__, j);		

						/*Caution!!!
						* We need to change type from FIC sapce to TCC351X Space
						*/
						switch(g_tChannelInfo[j].Channel_Type)
						{
						case 1:  //DAB
							g_tChannelInfo[j].Channel_Id = SRVTYPE_DAB;
							break;
						case 2: //DMB
							g_tChannelInfo[j].Channel_Id = SRVTYPE_DMB;
							break;
						case 3: //DATA
							g_tChannelInfo[j].Channel_Id = SRVTYPE_DATA;
							break;
						case 4: //DABPluse
							g_tChannelInfo[j].Channel_Id = SRVTYPE_DABPLUS;
							break;
						default:
							g_tChannelInfo[j].Channel_Id = SRVTYPE_NONE;	
							break;	
						}
						omx_tcc351x_tuner_component_searched_notify((void *)&g_tChannelInfo[j], ++iFoundServiceCount);
					}
					break;
				}	
				usleep(5000);
			}
		}
		omx_tcc351x_tuner_component_searched_notify(NULL, ((i+1)*100/(max_ch+1)));
	}
	omx_tcc351x_tuner_component_searched_notify(NULL, 0);

	return 0;
}

int TCC351XTDMB_ChannelIndexScan(unsigned int uiFreqIndex)
{
	int serviceIndex, searchIndex, iFoundServiceCount = 0;
	giDABServiceCount = giDMBServiceCount = giDATAServiceCount = giTOTALServiceCount = giDABPLUSServiceCount = 0;
	TCC_TSIF_ResetRead();
	DAB_Buff_Init(0);
	Parsing_Buffer_Init(0, 0); // defined at parser
	if(TCC351XTDMB_ChannelSet(uiFreqIndex) == 0)
	{
		TCC351XTDMB_SetState(STATE_SEARCHING);
		for(searchIndex = 0; searchIndex < 300; searchIndex++)
		{	
			if(TCC351XTDMB_GetState()==STATE_SEARCHED)
			{
				for(serviceIndex = 0; serviceIndex < giTOTALServiceCount; serviceIndex++)
				{
					//PRINTF("%s upload channel information %d", __func__, j);		

					/*Caution!!!
					* We need to change type from FIC sapce to TCC351X Space
					*/
					switch(g_tChannelInfo[serviceIndex].Channel_Type)
					{
					case 1:  //DAB
						g_tChannelInfo[serviceIndex].Channel_Id = SRVTYPE_DAB;
						break;

					case 2: //DMB
						g_tChannelInfo[serviceIndex].Channel_Id = SRVTYPE_DMB;
						break;

					case 3: //DATA
						g_tChannelInfo[serviceIndex].Channel_Id = SRVTYPE_DATA;
						break;

					case 4: //DABPluse
						g_tChannelInfo[serviceIndex].Channel_Id = SRVTYPE_DABPLUS;
						break;

					default:
						g_tChannelInfo[serviceIndex].Channel_Id = SRVTYPE_NONE;
						break;
					}
					omx_tcc351x_tuner_component_searched_notify((void *)&g_tChannelInfo[serviceIndex], ++iFoundServiceCount);
				}
				break;
			}	
			usleep(5000);
		}
	}
	return 0;
}

int TCC351XTDMB_FrequencyScan(unsigned int uiFreq)
{
	int serviceIndex, searchIndex, iFoundServiceCount = 0;
	giDABServiceCount = giDMBServiceCount = giDATAServiceCount = giTOTALServiceCount = giDABPLUSServiceCount = 0;
	TCC_TSIF_ResetRead();
	DAB_Buff_Init(0);
	Parsing_Buffer_Init(0, 0); // defined at parser
	if(TCC351XTDMB_FrequencySet(uiFreq, 1) == 0)
	{
		TCC351XTDMB_SetState(STATE_SEARCHING);
		for(searchIndex = 0; searchIndex < 300; searchIndex++)
		{	
			if(TCC351XTDMB_GetState()==STATE_SEARCHED)
			{
				for(serviceIndex = 0; serviceIndex < giTOTALServiceCount; serviceIndex++)
				{
					//PRINTF("%s upload channel information %d", __func__, j);		

					/*Caution!!!
					* We need to change type from FIC sapce to TCC351X Space
					*/
					switch(g_tChannelInfo[serviceIndex].Channel_Type)
					{
					case 1:  //DAB
						g_tChannelInfo[serviceIndex].Channel_Id = SRVTYPE_DAB;
						break;

					case 2: //DMB
						g_tChannelInfo[serviceIndex].Channel_Id = SRVTYPE_DMB;
						break;

					case 3: //DATA
						g_tChannelInfo[serviceIndex].Channel_Id = SRVTYPE_DATA;
						break;

					case 4: //DABPluse
						g_tChannelInfo[serviceIndex].Channel_Id = SRVTYPE_DABPLUS;
						break;

					default:
						g_tChannelInfo[serviceIndex].Channel_Id = SRVTYPE_NONE;
						break;
					}
					omx_tcc351x_tuner_component_searched_notify((void *)&g_tChannelInfo[serviceIndex], ++iFoundServiceCount);
				}
				break;
			}	
			usleep(5000);
		}
	}
	return 0;
}

int TCC351XTDMB_SearchStopChannels(void)
{
	giSearchStop = 1;
	return 0;
}

#if 1
int TCC351XTDMB_GetSignalStrength(unsigned int *pSNR, unsigned int *pPCBER, double *pdRSSI)
{
	double dSNR, dPCBER, dRSSI, dSNRdb;
	double dPCBERAvg, dRSSIAvg, dSNRdbAvg;
	int moduleidx, DeviceIdx;
	t_TDMBLock Lock;

	moduleidx = 0, DeviceIdx = 0;

	TC3X_API_GetTDMB_Lock (0, 0, &Lock);       
	TC3X_API_GetTDMB_SNR (moduleidx,DeviceIdx, &dSNR, &dSNRdb, &dSNRdbAvg);
	TC3X_API_GetTDMB_RSSI (moduleidx,DeviceIdx, &dRSSI, &dRSSIAvg);
	TC3X_API_GetTDMB_PCBER (moduleidx,DeviceIdx, &dPCBER, &dPCBERAvg);

	*pSNR = (unsigned int)dSNRdbAvg;
	*pPCBER = (unsigned int)(dPCBERAvg*_PCBER_RESOLUTION_);
	*pdRSSI = dRSSIAvg;

	if(pPCBER[0]>20000)
		pPCBER[0]=20000;

	return  TDMB_Get_TCC35XX_Antenna(dPCBERAvg, Lock.CTO, Lock.CFO);
}
#else
int TCC351XTDMB_GetSignalStrength(unsigned int *pSNR, unsigned int *pPCBER, double *pdRSSI)
{
	double dSNR, dPCBER, dRSSI, dSNRdb, dMovingAVG;
	int moduleidx, DeviceIdx;
	t_TDMBLock Lock;

	moduleidx = 0, DeviceIdx = 0;

	TC3X_API_GetTDMB_Lock (0, 0, &Lock);     
	TC3X_API_GetTDMB_SNR (moduleidx,DeviceIdx, &dSNR, &dSNRdb, &dMovingAVG);
	TC3X_API_GetTDMB_RSSI (moduleidx,DeviceIdx, &dRSSI, &dMovingAVG);
	TC3X_API_GetTDMB_PCBER (moduleidx,DeviceIdx, &dPCBER, &dMovingAVG);

	*pSNR = (unsigned int)dSNRdb;
	*pPCBER = (unsigned int)(dPCBER *_PCBER_RESOLUTION_);
	*pdRSSI = dRSSI;

	if(pPCBER[0]>20000)
		pPCBER[0]=20000;

	return TDMB_Get_TCC35XX_Antenna(dMovingAVG, Lock.CTO, Lock.CFO);
}
#endif

long long TCC351XTDMB_Util_GetTimeInterval(long long startTick)
{
    long long Cnt;

    if (TC3X_IO_GET_TIMECNT_ms () > startTick)
        Cnt = (long long)((TC3X_IO_GET_TIMECNT_ms () - startTick)*1000);
    else
        Cnt = (long long)(((0xFFFFFFFFFFFFFFFFLL - startTick) + TC3X_IO_GET_TIMECNT_ms () + 1)*1000);

    return Cnt;
}

