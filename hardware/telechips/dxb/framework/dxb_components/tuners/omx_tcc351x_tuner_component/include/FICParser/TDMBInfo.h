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

#ifndef _TDMB_INFO_H_
#define _TDMB_INFO_H_

//#include "TCCresource.h"
#include "Dmb_Types.h"
#include "fig.h"

#define	MAX_ENSEMBLEDB		(12)
#define	MAX_SERVICEDB		(64)
#define	MAX_CHANNELDB		(64)

/* This must be modified, when MAX_ENSEMBLEDB, MAX_SERVICEDB, MAX_CHANNELDB is modified */
/* Maximum DMB Service Count */
#define	UI_DMBSVC_CNT_MAX	(10*20)

#define DMB_DB_USED (0x0001 << 0)

#define CH_UPDATE_NO_DATA			0
#define CH_UPDATE_ESSENTIAL_DATA	1
#define CH_UPDATE_FULL_DATA			2

#define MAX_USERINFO_NUM_PER_CHANNEL	6

typedef struct _TDBSTATE
{
	INT16U	State;
	INT16U	dbIndex;
	struct _TDBSTATE 	*Next;
}TDBSTATE,*PDBSTATE;

typedef struct
{
	INT16U	Type;
	INT16U	DataLen;
	INT8U	Data[24];
}TUSERAPPINFO, *PUSERAPPINFO;

typedef struct _TCHANNELDB
{
	TDBSTATE			DB;
	INT32U				Type;   // 1 = DAB, 2 = DMB, 3 = DATA	
	INT32U				Id;
	INT32U				BitRate;
	INT8U				PL; //Protection Level
	INT8U				SCId;
	INT8U				SCIdS;
	INT8U				DG_flag;	//parser에는 FIC 0/3의 DG뿐이 아니고 0/4의 MF도 같은 변수로 사용. 구분에 주의.
	INT16U				PacketAddr;
	INT16U				UserApplType;
	INT8U				TMId;
	INT8U				ASCTy_DSCTy;
	INT32U				Primary;
	INT8U				Reg[7];		/* this is the value that 3100's 'D0~D6' register. */
	//oprsystem 060817 [Add]
	INT8U				charset;	
	char				SCLabel[17]; 
	INT16U				DSCTyExtension;

	INT8U				NumOfUserAppl;
	TUSERAPPINFO		UserAppInfo[MAX_USERINFO_NUM_PER_CHANNEL];

	SvcComponentInfo	*pSvcComp; 
	SubChInfo			*pSubCh;
	struct _TCHANNELDB	*Next;
	INT32U				Tag;
	PVOID				Link;
}TCHANNELDB,*PCHANNELDB,**PPCHANNELDB;

typedef struct _TSERVICEDB
{
	TDBSTATE			DB;
	INT32U				Id;
	INT8U				charset;	
	char				Label[30];
	ServiceInfo			*pService;
	PCHANNELDB			Channel;
	INT32U				ChannelCount;
	struct _TSERVICEDB			*Next;
	INT32U				Tag;
	PVOID				Link;
}TSERVICEDB,*PSERVICEDB,**PPSERVICEDB;

typedef struct _TENSEMBLEDB
{
	TDBSTATE			DB;
	INT32U				Band;
	INT32U				FreqIndex;
	INT16U				Id;
	
	//oprsystem 060817 [Add]
	INT8U				GlobalDefinition;

	INT32U				SvcNum;
	INT32U				SvcCompNum;
	INT32U				SubChNum;
	INT8U				charset;
	char				Label[20];
	EnsembleInfo		*pEnsemble;
	PSERVICEDB			Service;
	struct _TENSEMBLEDB		*Next;
	PVOID				Link;
	INT32U				Tag;
}TENSEMBLEDB,*PENSEMBLEDB,**PPENSEMBLEDB;

typedef struct
{
	INT8U  Reg[7];
} TSUB_CHANNEL_INFO, *PSUB_CHANNEL_INFO;

typedef struct
{
	INT16U				EnsembleID;
	INT8U				SubChannelID;
	INT32U				ServiceID;
	INT32U				Frequency;		/* Not Frequency Index */
	INT32U				Band;
	INT32U				TimeOut;
	INT32U				OPTION;
	TSUB_CHANNEL_INFO	tSubChInfo;
	PENSEMBLEDB			pEnsembleDB;
} TDMB_CHANNEL_INFO, *PDMB_CHANNEL_INFO;

typedef struct
{
	int            curPlayIdx;

	unsigned short curEnsembleIdx;
	unsigned short curServiceIdx;
	unsigned short curSubChIdx;
	unsigned short dummy;

	unsigned short numOfEnsemble;	/* Total Number of Ensembles */
	unsigned short numOfService;	/* Total Number of Services */

	PENSEMBLEDB   pEnsembleDBRoot;	/* Root Ensemble DB pointer */

} TDMB_ENSEMBLEINFO;

typedef struct
{
	PENSEMBLEDB		pEnsembleDBList;
	PSERVICEDB		pServiceDBList;
	PCHANNELDB		pChannelDBList;

	unsigned int	ensembleDBMaxSize;
	unsigned int	serviceDBMaxSize;
	unsigned int	channelDBMaxSize;
} TDMB_INITDBINFO, *PDMB_INITDBINFO;

//2006-12-13 oprsystem [add]

typedef struct _SC_INFO_
{
	int tm_id;
	unsigned char ca_org[2];
	int subch_id;
	int sc_id;
	int asc_ty;
	int dsc_ty;
	int dg_flag;
	int packet_addr;
	char ensemble_label[17];
	char service_label[17];

}SC_INFO, *PSC_INFO;/* ----------  end of struct SC_INFO  ---------- */

// Added by user
typedef struct
{
	unsigned int Ensemble_Band;
	unsigned int Ensemble_Freq;
	unsigned int Ensemble_Id;		// add
	unsigned int Service_Id;
	char Ensemble_Label[20];		// add
	char Service_Label[30];
	char Channel_Label[20];
	unsigned int Channel_Id;
	unsigned int Channel_Type;
	unsigned int Channel_BitRate;
	char Reg[7];	// add
}TDMB_INFO;
// end of Added by user

void DMB_InitDB(PDMB_INITDBINFO pInitDBInfo);

PENSEMBLEDB DMB_GetEnsembleDB(void);
PSERVICEDB 	DMB_GetServiceDB(void);
PCHANNELDB 	DMB_GetChannelDB(void);

PENSEMBLEDB DMB_FreeEnsembleDB(PENSEMBLEDB pEnsemble);
PSERVICEDB  DMB_FreeServiceDB(PSERVICEDB pService);
PCHANNELDB  DMB_FreeChannelDB(PCHANNELDB pChannel);

PENSEMBLEDB DMB_AssignedEnsembleDB( PENSEMBLEDB pEnsemble );
PSERVICEDB DMB_AssignedServiceDB( PSERVICEDB pService );
PCHANNELDB DMB_AssignedChannelDB( PCHANNELDB pChannel );

//PENSEMBLEDB DMB_AddEnsembleToEnsembleDB(PENSEMBLEDB pRootEnsembleDB, PENSEMBLEDB pEnsemble);
PENSEMBLEDB DMB_AddEnsembleToEnsembleDB(PPENSEMBLEDB ppRootEnsembleDB, PENSEMBLEDB pEnsemble);
PENSEMBLEDB DMB_FindEnsembleFromEnsembleDB(PENSEMBLEDB pRootEnsemble, PENSEMBLEDB pEnsemble);
PENSEMBLEDB DMB_RemoveEnsembleFromEnsembleDB(PPENSEMBLEDB ppRootEnsembleDB, PENSEMBLEDB pEnsemble);
PENSEMBLEDB DMB_SearchEnsembleDBById(PENSEMBLEDB pRootEnsembleDB, INT16U EnsembleId);
PENSEMBLEDB DMB_SearchEnsembleDBByFreqIdx(PENSEMBLEDB pRootEnsembleDB, INT32U FreqIdx);
PENSEMBLEDB DMB_SearchEnsembleDBByEIdFreqIdx(PENSEMBLEDB pRootEnsembleDB, INT16U EnsembleId,INT32U FreqIdx);

PSERVICEDB DMB_AddServiceToServiceDB(PPSERVICEDB ppRootServiceDB, PSERVICEDB pService);
//PSERVICEDB DMB_AddServiceToServiceDB(PSERVICEDB pRootServiceDB, PSERVICEDB pService);
PSERVICEDB DMB_FindServiceFromServiceDB(PSERVICEDB pRootServiceDB, PSERVICEDB pService);
PSERVICEDB DMB_RemoveServiceFromServiceDB(PSERVICEDB pRootServiceDB, PSERVICEDB pService);
PSERVICEDB DMB_SearchServiceDBById(PSERVICEDB pRootServiceDB, INT32U ServiceId);

//PCHANNELDB DMB_AddChannelToChannelDB(PCHANNELDB pRootChannelDB, PCHANNELDB pChannel);
PCHANNELDB DMB_AddChannelToChannelDB(PPCHANNELDB ppRootChannelDB, PCHANNELDB pChannel);
PCHANNELDB DMB_FindChannelFromChannelDB(PCHANNELDB pRootChannelDB, PCHANNELDB pChannel);
PCHANNELDB DMB_RemoveChannelFromChannelDB(PCHANNELDB pRootChannelDB, PCHANNELDB pChannel);
PCHANNELDB DMB_SearchChannelDBById(PCHANNELDB pRootChannelDB, INT8U ChannelId);
PENSEMBLEDB DMB_UnlinkEnsembleDB (PENSEMBLEDB pEnsembleDB);

PENSEMBLEDB DMB_TakeEnsembleTreeFormFIC(PENSEMBLEDB pEnsemble);
extern INT32U DMB_IsChannelUpdate(void);
extern INT32U DMB_BufferInit(PINT8U Inst, int modindex);
extern void DAB_Buff_Init(int modindex);

//INT32U DMB_TakeSCINFOFromFIC( PSC_INFO pscinfo ,INT32U ServiceId, INT32U SubChId );
INT32U DMB_TakeSCINFOFromFIC( PSC_INFO pscinfo ,PINT8U pcaflag,INT32U ServiceId, INT32U SubChId );
void DMB_SortForFICParser(void);
FIC_INFO_T * DMB_Get_FIC_Info(void);
#endif

/* end of file */

