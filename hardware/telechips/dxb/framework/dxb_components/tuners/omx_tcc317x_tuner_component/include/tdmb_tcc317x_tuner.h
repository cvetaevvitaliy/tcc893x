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

#ifndef _TDMB_TUNER_TCC317X_H_
#define _TDMB_TUNER_TCC317X_H_

/******************************************************************************
* include 
******************************************************************************/
#include "tcpal_types.h"
#include "tcc317x_monitoring.h"
#include "tcc317x_tdmb.h"

/*****************************************************************************
* define
******************************************************************************/
#ifndef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif

#define BER_SCALE_FACTOR 100000.0

/* TDMB Data Service */
#define MAX_TCC317X     4
#define MAX_SUBCH_CNT  (80)

/*****************************************************************************
* structures
******************************************************************************/
typedef enum
{
	SEARCH_START = 0,
	SEARCH_CONTINUE,
	SEARCH_STOP
} E_SEARCH_CMD;

#ifdef _MULTI_SERVICE_
typedef enum
{
	CHANNEL_UNREG = 0,
	CHANNEL_REG,
	CHANNEL_DefaultSet
}E_CHANNEL_COMMAND;
#endif

typedef struct
{
	I32U uiMinChannel;
	I32U uiMaxChannel;
	I32U uiCountryCode;
}TUNER_SEARCH_INFO;

typedef struct
{
	I32U Channel;
	I32U Freq;
	I32U Search_Stop_Flag;
}ISDBT_CHANNEL_INFO;

typedef enum
{
	STATE_IDLE,
	STATE_SEARCHING,	
	STATE_SEARCHED,	
	STATE_TDMB_PLAY,
	STATE_DAB_PLAY,
	STATE_MAX
}E_TDMB_STATE;

/* TDMB Data Service  */
typedef struct
{
    unsigned char *pPoint;
} tDabMSCSubChBuffIdx;

typedef struct
{
    unsigned char *pFICBuffer;
    unsigned char *pDMBBuffer;
    unsigned char *pDABDATABuffer;

    unsigned int FICSize;
    unsigned char FICType[12];   // each 384 bytes
    unsigned char FICCRC[12];   // each 384 bytes
    unsigned int DMBSize;
    unsigned int DABDATASize[12];
    unsigned int DABDATATotalSize;

    int DABDATAServiceIdx[12];
} tDMBReadFormat;

typedef struct
{
	I32S RSSI;
	I32U SNR;
	I32U PCBer;
	I32U TSPer;
	I32U ViterBer;	
	I32U FICBer;
	I32U MSCBer;
	I08U RFLoopGain;
	I08U BBLoopGain;
	I08U OFDM_Lock;
	I32U DSPVer;	
	I32U DDVer;
	I32U DSPBuildDate;
}TCC317X_INFO;

/*****************************************************************************
* Variables
******************************************************************************/
I32U    ISDBT_Tunertype;
ISDBT_CHANNEL_INFO *ISDBT_ChannelInfo;

/* TDMB Data Service  */
tDabMSCSubChBuffIdx g_DabMscSubChBuffIdx[MAX_TCC317X][MAX_TDMBMSC_NUM][MAX_SUBCH_CNT];
unsigned int g_DabMscSubChBuffSize[MAX_TCC317X][MAX_TDMBMSC_NUM][MAX_SUBCH_CNT];
unsigned int g_DabMscSubChBuffCnt[MAX_TCC317X][MAX_TDMBMSC_NUM];
unsigned int g_DabMscSubChBuffSubch[MAX_TCC317X][MAX_TDMBMSC_NUM];

/******************************************************************************
* Declarations
******************************************************************************/
void TCC317XTDMB_StreamThread(void *arg);
void TCC317XTDMB_PllSet(I32S moduleidx);
void TCC317XTDMB_FICbufferInit(I32S moduleidx);
void TCC317XTDMB_DataStreamThread(void *arg);
void TCC317XTDMB_StreamThreadHold(int moduleidx);
void TCC317XTDMB_StreamThreadRun(int moduleidx);
void TCC317XTDMB_OpenFlagInit(void);
void TCC317XTDMB_OpenFlagSet(I32U moduleidx, I32U flag);
I32S TCC317XTDMB_OpenFlagGet(I32U moduleidx);
I32S TCC317XTDMB_Init(I32S moduleidx, I32S iCountryCode ,I32S commandInterface, I32S streamInterface);
I32S TCC317XTDMB_ChannelScan(I32S moduleidx, I32S channel);
I32S TCC317XTDMB_ChannelFreqScan(I32S moduleidx, I32S freq);
#ifdef _MULTI_SERVICE_
I32S TCC317XTDMB_ChannelSet(I32S moduleidx, I32U *uiARG, I32S bLockOn, I32U ChControl);
#else
I32S TCC317XTDMB_ChannelSet(I32S moduleidx, I32U *uiARG, I32S bLockOn);
#endif 

I32S TCC317XTDMB_FrequencySet(I32S moduleidx, I32S iFreqKhz, I32S bLockOn);
I32S TCC317XTDMB_ServiceSelect(I32S moduleidx, I32U uiServiceType, I08U *pucService);
I32S TCC317XTDMB_GetSignalStrength(I32S moduleidx, I32S diversityindex, TCC317X_INFO* _info);
I32S TCC317XTDMB_StreamInterface(I32S moduleidx, I32S *piArg, I32S dataInterface);
I32S TCC317XTDMB_Close(I32S moduleidx, I32S commandInterface);

/******************************************************************************
* Power Control
******************************************************************************/
I32S TCC317XTDMB_IO_PW_OPEN(void);
I32S TCC317XTDMB_IO_PW_ON(I32S moduleidx);
I32S TCC317XTDMB_IO_PW_OFF(I32S moduleidx);
I32S TCC317XTDMB_IO_PW_RESET(I32S moduleidx);
I32S TCC317XTDMB_IO_PW_RESET_LOW(I32S moduleidx);
I32S TCC317XTDMB_IO_PW_RESET_HIGH(I32S moduleidx);

/******************************************************************************
* State
******************************************************************************/
I08U TCC317XTDMB_Get_Antenna(I32S moduleidx, I32U PcberAvg, DmbLock_t* Lockstate);
E_TDMB_STATE TCC317XTDMB_GetState(I32S moduleidx);
E_TDMB_STATE TCC317XTDMB_SetState(I32S moduleidx,E_TDMB_STATE eState);

/******************************************************************************
* Stream Service
******************************************************************************/
void Tcc317xStreamParserInit (I32S _moduleIndex);
void Tcc317xStreamParsing (I32S _moduleIndex, I08U * _data, I32S _size);
I32S TCC317XTDMB_ReadFIFO(I32S iHandle, I08U *pucData, I32U uiSize);
I32S TCC317XTDMB_FICProcess(I32S moduleidx, I08U *pucData, I32U uiSize, I32U uiCRC, I32S uiType);
I32S TCC317XTDMB_AvFifoBuffer_Init(I32U moduleidx);
I32S TCC317XTDMB_AvFifoBuffer_DeInit(I32U moduleidx);
I32S TCC317XTDMB_AVServiceProcess(I32S moduleidx, I08U *pucData, I32U uiSize, I32S uiType, I32U SubChID);
I32S TCC317XTDMB_DataServiceProcess(I32S moduleidx, I08U *pucData, I32U uiSize, I32S uiType, I32U SubChID);

/******************************************************************************
* TSIF Driver
******************************************************************************/
int TCC_TSIF_Open(int moduleidx, int packet_intr_cnt, int packet_dma_cnt ,int mode, int dma_mode);
int TCC_TSIF_Close(int moduleidx, int handle);
int TCC_TSIF_Read(int moduleidx, int handle, unsigned char *buf, unsigned int read_size);
int TCC_HWDMX_Stop(int handle);
int TCC_HWDMX_Start(int handle);

/* TDMB Data Service*/
static void TCC317XTDMB_DataServiceFlush(I32S moduleidx);
static void TCC317XTDMB_PopDataService (I32S moduleidx, tDMBReadFormat *pDMBFormatData);
static I32S TCC317XTDMB_PushDataService(I32S moduleidx, I08U *pStream, I32U size, I32U subchId);
#endif

