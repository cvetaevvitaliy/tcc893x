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

#ifndef __TDMBINFO_H__
#define __TDMBINFO_H__

#define LOG_TAG	"TCC351X_TDMBINFO"
#include <utils/Log.h>


//#include "stdafx.h"
#include "fig.h"
//#include "TDMB/inc/DMBAPI.h"
//#include "TDMB/inc/DMBGlobal.h"
#include "Dmb_Types.h"
//#include "Global.h"
#include "TDMBInfo.h"

PENSEMBLEDB		g_tEnsembleDBList;
PSERVICEDB		g_tServiceDBList;
PCHANNELDB		g_tChannelDBList;

unsigned int	g_tEnsembleDBMaxSize;
unsigned int	g_tServiceDBMaxSize;
unsigned int	g_tChannelDBMaxSize;

//#define	PRINTF		ALOGE

PVOID
DMBDB_Init ( PVOID p , INT16U dbIndex )
{
	PDBSTATE pDB;

	pDB = (PDBSTATE) p;

	memset( p, 0x00, sizeof(TDBSTATE) );

	pDB->dbIndex = dbIndex;

	return p;
}

PVOID
DMBDB_Alloc(PVOID p)
{
	PDBSTATE pDB;

	pDB = (PDBSTATE) p;

	while( pDB != NULL )
	{
		if (pDB->State & DMB_DB_USED)	
		{
			pDB = pDB->Next;
		}
		else
		{
			pDB->State |= DMB_DB_USED;
			return (PVOID) pDB;
		}
	}	
	
	return (PVOID) NULL;	
}	

PVOID
DMBDB_Free(PVOID p)
{
	PDBSTATE pDB;

	pDB = (PDBSTATE) p;
	
	pDB->State &= ~DMB_DB_USED;

	return (PVOID) pDB;
}

PVOID
DMBDB_Link(PVOID pHead, PVOID pTail)
{
	PDBSTATE pDBHead, pDBTail;

	pDBHead = (PDBSTATE) pHead;
	pDBTail = (PDBSTATE) pTail;

	if (pDBHead == NULL)
	{
		return (PVOID) NULL;
	}

	pDBHead->Next = pDBTail;

	return (PVOID) pDBTail;
}

PVOID
DMBDB_Assigned(PVOID p)
{
	PDBSTATE pDB;

	if (p == NULL)
	{
		return NULL;
	}
	
	pDB = (PDBSTATE) p;

	if ( pDB->State & DMB_DB_USED )
	{
		return (PVOID) pDB;
	}
	
	return (PVOID) NULL;
}

void
DMB_InitEnsembleDB (PENSEMBLEDB pEnsemble)
{
	INT32U size; 
	PINT32U poffset;

	size = sizeof(TENSEMBLEDB) - sizeof(TDBSTATE);

	poffset = (PINT32U) ( ((INT32U)pEnsemble) + sizeof(TDBSTATE) );

	memset( poffset, 0x00, size );
}

void
DMB_InitServiceDB(PSERVICEDB pService)
{
	INT32U size; 
	PINT32U poffset;

	size = sizeof(TSERVICEDB) - sizeof(TDBSTATE);

	poffset = (PINT32U) ( ((INT32U)pService) + sizeof(TDBSTATE) );

	memset( poffset, 0x00, size );
}

void
DMB_InitChannelDB(PCHANNELDB pChannel)
{
	INT32U size; 
	PINT32U poffset;

	size = sizeof(TCHANNELDB) - sizeof(TDBSTATE);

	poffset = (PINT32U) ( ((INT32U)pChannel) + sizeof(TDBSTATE) );

	memset( poffset, 0x00, size );
}

void
DMB_InitDB(PDMB_INITDBINFO pInitDBInfo)
{
	INT32U i;

	PENSEMBLEDB pEnsemble,pEnsembleNext;
	PSERVICEDB 	pService,pServiceNext;
	PCHANNELDB 	pChannel,pChannelNext;

	g_tEnsembleDBList = pInitDBInfo->pEnsembleDBList;
	g_tServiceDBList = pInitDBInfo->pServiceDBList;
	g_tChannelDBList = pInitDBInfo->pChannelDBList;

	g_tEnsembleDBMaxSize = pInitDBInfo->ensembleDBMaxSize;
	g_tServiceDBMaxSize = pInitDBInfo->serviceDBMaxSize;
	g_tChannelDBMaxSize = pInitDBInfo->channelDBMaxSize; 

	for(i=0;i<g_tEnsembleDBMaxSize;i++)
	{
		pEnsemble = &g_tEnsembleDBList[ i ];
		
		DMBDB_Init( pEnsemble, i );
		DMB_InitEnsembleDB( pEnsemble );
		
		if ( i != (g_tEnsembleDBMaxSize - 1) )
		{
			pEnsembleNext = &g_tEnsembleDBList[ i+1 ];
			
			DMBDB_Link( pEnsemble, pEnsembleNext );
		}
		else
		{
			DMBDB_Link( pEnsemble, NULL );
		}
	}

	for(i=0;i<g_tServiceDBMaxSize;i++)
	{
		pService = &g_tServiceDBList[i];
		
		DMBDB_Init( pService, i );
		DMB_InitServiceDB( pService );

		if ( i != (g_tServiceDBMaxSize - 1) )
		{
			pServiceNext = &g_tServiceDBList[ i+1 ];
			
			DMBDB_Link( pService, pServiceNext );
		}
		else
		{
			DMBDB_Link( pService, NULL );
		}
	}

	for(i=0;i<g_tChannelDBMaxSize;i++)
	{
		pChannel = &g_tChannelDBList[i];

		DMBDB_Init( pChannel, i );
		DMB_InitChannelDB( pChannel );

		if ( i != (g_tChannelDBMaxSize - 1) )
		{
			pChannelNext = &g_tChannelDBList[ i+1 ];
			
			DMBDB_Link( pChannel, pChannelNext );
		}
		else
		{
			DMBDB_Link( pChannel, NULL);
		}
	}
}

PENSEMBLEDB
DMB_GetEnsembleDB(void)
{
	return (PENSEMBLEDB) DMBDB_Alloc( &g_tEnsembleDBList[ 0 ] );
}

PSERVICEDB
DMB_GetServiceDB(void)
{
	return (PSERVICEDB) DMBDB_Alloc( &g_tServiceDBList[ 0 ] );
}

PCHANNELDB
DMB_GetChannelDB(void)
{
	return (PCHANNELDB) DMBDB_Alloc( &g_tChannelDBList[ 0 ] );
}

PENSEMBLEDB
DMB_FreeEnsembleDB(PENSEMBLEDB pEnsemble)
{
	return (PENSEMBLEDB) DMBDB_Free( pEnsemble );
}

PSERVICEDB
DMB_FreeServiceDB(PSERVICEDB pService)
{
	return (PSERVICEDB) DMBDB_Free( pService );
}

PCHANNELDB
DMB_FreeChannelDB(PCHANNELDB pChannel)
{
	return (PCHANNELDB) DMBDB_Free( pChannel );
}


PENSEMBLEDB
DMB_AssignedEnsembleDB( PENSEMBLEDB pEnsemble )
{
	return (PENSEMBLEDB) DMBDB_Assigned( pEnsemble );
}

PSERVICEDB
DMB_AssignedServiceDB( PSERVICEDB pService )
{
	return (PSERVICEDB) DMBDB_Assigned( pService );
}

PCHANNELDB
DMB_AssignedChannelDB( PCHANNELDB pChannel )
{
	return (PCHANNELDB) DMBDB_Assigned( pChannel );
}
/////////////////////////////////////////////////////////////////////////////////////

/*
PENSEMBLEDB
DMB_AddEnsembleToEnsembleDB(PENSEMBLEDB pRootEnsembleDB, PENSEMBLEDB pEnsemble)
{
	PENSEMBLEDB tempEnsembleDB,findEnsembleDB;

	if ( (pEnsemble == NULL) || (pRootEnsembleDB == NULL) )
	{
		return (PENSEMBLEDB) NULL;
	}

	findEnsembleDB = pRootEnsembleDB;
	
	while( findEnsembleDB )
	{
		if (findEnsembleDB->Next == NULL)
		{
			tempEnsembleDB 				= findEnsembleDB->Next; //Last Null
			findEnsembleDB->Next 		= pEnsemble;
			findEnsembleDB->Next->Next 	= tempEnsembleDB;

			return pEnsemble;
		}

		findEnsembleDB = findEnsembleDB->Next;
	}
	
	return pEnsemble;
}
*/

PENSEMBLEDB
DMB_AddEnsembleToEnsembleDB(PPENSEMBLEDB ppRootEnsembleDB, PENSEMBLEDB pEnsemble)
{
	PENSEMBLEDB tempEnsembleDB, findEnsembleDB, pRootEnsembleDB;

	pRootEnsembleDB = *ppRootEnsembleDB;

	if ( pRootEnsembleDB == NULL)
	{
		*ppRootEnsembleDB = (PENSEMBLEDB) pEnsemble;

		return (PENSEMBLEDB) *ppRootEnsembleDB;
	}
	
	findEnsembleDB = *ppRootEnsembleDB;

	while( findEnsembleDB )
	{
		if (findEnsembleDB->Next == NULL)
		{
			tempEnsembleDB 				= findEnsembleDB->Next; //Last Null
			findEnsembleDB->Next 		= pEnsemble;
			findEnsembleDB->Next->Next 	= tempEnsembleDB;

			return pEnsemble;
		}

		findEnsembleDB = findEnsembleDB->Next;
	}
	
	return pEnsemble;
}


PENSEMBLEDB
DMB_FindEnsembleFromEnsembleDB(PENSEMBLEDB pRootEnsembleDB, PENSEMBLEDB pEnsemble)
{
	
	PENSEMBLEDB findEnsembleDB;

	if ( (pEnsemble == NULL) || (pRootEnsembleDB == NULL) )
	{
		return (PENSEMBLEDB) NULL;
	}

	findEnsembleDB = pRootEnsembleDB;
	
	while( findEnsembleDB ) // if not found ensembleDB 	return NULL
	{						// if ensembleDB is One 	return rootEnsembleDB
		if (findEnsembleDB == pEnsemble)
		{
			return findEnsembleDB;
		}
		
		findEnsembleDB = findEnsembleDB->Next;
	}
	
	return findEnsembleDB;
}


//return NULL mean
//if not found ensembleDB or Error
PENSEMBLEDB
DMB_RemoveEnsembleFromEnsembleDB(PPENSEMBLEDB ppRootEnsembleDB, PENSEMBLEDB pEnsemble)
{
	PENSEMBLEDB removeEnsembleDB,tempEnsembleDB,findEnsembleDB;

	findEnsembleDB = *ppRootEnsembleDB;

	if ( findEnsembleDB == pEnsemble)
	{
		*ppRootEnsembleDB = (PENSEMBLEDB) findEnsembleDB->Next;
		pEnsemble->Next = NULL;

		//2007-02-22 오후 2:25:18 대한민국 표준시
		// TCC78x reconfiguration handler 관련 수정
		return pEnsemble;
		//return (PENSEMBLEDB) NULL;
	}

	while(findEnsembleDB)
	{
		if (findEnsembleDB->Next == pEnsemble)
		{
			tempEnsembleDB = findEnsembleDB->Next->Next; //pEnsemble->next save pointer
			findEnsembleDB->Next = tempEnsembleDB;
			pEnsemble->Next = NULL;

			return pEnsemble;
		}

		findEnsembleDB = findEnsembleDB->Next;
	}
	
	return (PENSEMBLEDB) NULL;
}

PENSEMBLEDB
DMB_SearchEnsembleDBById(PENSEMBLEDB pRootEnsembleDB, INT16U EnsembleId)
{
	PENSEMBLEDB removeEnsembleDB,tempEnsembleDB,findEnsembleDB;

	findEnsembleDB = pRootEnsembleDB;

	while(findEnsembleDB)
	{
		if (findEnsembleDB->Id == EnsembleId)
		{
			return findEnsembleDB;
		}

		findEnsembleDB = findEnsembleDB->Next;
	}

	return (PENSEMBLEDB) NULL;
}

PENSEMBLEDB
DMB_SearchEnsembleDBByFreqIdx(PENSEMBLEDB pRootEnsembleDB, INT32U FreqIdx)
{
	PENSEMBLEDB removeEnsembleDB,tempEnsembleDB,findEnsembleDB;

	findEnsembleDB = pRootEnsembleDB;

	while(findEnsembleDB)
	{
		if (findEnsembleDB->FreqIndex == FreqIdx)
		{
			return findEnsembleDB;
		}

		findEnsembleDB = findEnsembleDB->Next;
	}

	return (PENSEMBLEDB) NULL;
}

PENSEMBLEDB
DMB_SearchEnsembleDBByEIdFreqIdx(PENSEMBLEDB pRootEnsembleDB, INT16U EnsembleId,INT32U FreqIdx)
{
	PENSEMBLEDB removeEnsembleDB,tempEnsembleDB,findEnsembleDB;

	findEnsembleDB = pRootEnsembleDB;

	while(findEnsembleDB)
	{
		if ((findEnsembleDB->Id == EnsembleId) && ((findEnsembleDB->FreqIndex == FreqIdx)))
		{
			return findEnsembleDB;
		}

		findEnsembleDB = findEnsembleDB->Next;
	}

	return (PENSEMBLEDB) NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/*
PSERVICEDB
DMB_AddServiceToServiceDB(PSERVICEDB pRootServiceDB, PSERVICEDB pService)
{
	PSERVICEDB tempServiceDB, findServiceDB;

	if ( (pService == NULL) || (pRootServiceDB == NULL) )
	{
		return (PSERVICEDB) NULL;
	}

	findServiceDB = pRootServiceDB;
	
	while( findServiceDB )
	{
		if (findServiceDB->Next == NULL)
		{
			tempServiceDB 				= findServiceDB->Next; //Last Null
			findServiceDB->Next 		= pService;
			findServiceDB->Next->Next	= tempServiceDB;

			return pService;
		}

		findServiceDB = findServiceDB->Next;
	}
	
	return pService;
}
*/

PSERVICEDB
DMB_AddServiceToServiceDB(PPSERVICEDB ppRootServiceDB, PSERVICEDB pService)
{
	PSERVICEDB tempServiceDB, findServiceDB,pRootServiceDB;

	pRootServiceDB = *ppRootServiceDB;

	if ( pRootServiceDB == NULL )
	{
		*ppRootServiceDB = (PSERVICEDB) pService;

		return (PSERVICEDB) *ppRootServiceDB;
	}

	findServiceDB = *ppRootServiceDB;
	
	while( findServiceDB )
	{
		if (findServiceDB->Next == NULL)
		{
			tempServiceDB 				= findServiceDB->Next; //Last Null
			findServiceDB->Next 		= pService;
			findServiceDB->Next->Next	= tempServiceDB;

			return pService;
		}

		findServiceDB = findServiceDB->Next;
	}
	
	return pService;
}

PSERVICEDB
DMB_FindServiceFromServiceDB(PSERVICEDB pRootServiceDB, PSERVICEDB pService)
{
	PSERVICEDB findServiceDB;

	if ( (pService == NULL) || (pRootServiceDB == NULL) )
	{
		return (PSERVICEDB) NULL;
	}

	findServiceDB = pRootServiceDB;
	
	while( findServiceDB ) // if not found ServiceDB 	return NULL
	{						// if ServiceDB is One 	return rootServiceDB
		if (findServiceDB == pService)
		{
			return findServiceDB;
		}
		
		findServiceDB = findServiceDB->Next;
	}
	
	return findServiceDB;
}


//return NULL mean
//if not found ServiceDB or Error
PSERVICEDB
DMB_RemoveServiceFromServiceDB(PSERVICEDB pRootServiceDB, PSERVICEDB pService)
{
	PSERVICEDB removeServiceDB,tempServiceDB,findServiceDB;

	findServiceDB = pRootServiceDB;

	while(findServiceDB)
	{
		if (findServiceDB->Next == pService)
		{
			tempServiceDB = findServiceDB->Next->Next; //pService->next save pointer
			findServiceDB->Next = tempServiceDB;

			return pService;
		}
	}
	
	return (PSERVICEDB) NULL;
}

PSERVICEDB
DMB_SearchServiceDBById(PSERVICEDB pRootServiceDB, INT32U ServiceId)
{
	PSERVICEDB removeServiceDB,tempServiceDB,findServiceDB;

	findServiceDB = pRootServiceDB;

	while(findServiceDB)
	{
		if (findServiceDB->Id == ServiceId)
		{
			return findServiceDB;
		}

		findServiceDB = findServiceDB->Next;
	}

	return (PSERVICEDB) NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

PCHANNELDB
DMB_AddChannelToChannelDB(PPCHANNELDB ppRootChannelDB, PCHANNELDB pChannel)
{
	PCHANNELDB tempChannelDB,findChannelDB;

	//if ( (pRootChannelDB == NULL) || (pChannel == NULL) )
	//{
	//	return (PCHANNELDB) NULL; 
	//}
	if ( *ppRootChannelDB == NULL )
	{
		*ppRootChannelDB = (PCHANNELDB) pChannel;

		return (PCHANNELDB) *ppRootChannelDB;
	}

	findChannelDB = *ppRootChannelDB;
	
	while( findChannelDB )
	{
		if (findChannelDB->Next == NULL)
		{
			tempChannelDB 				= findChannelDB->Next; //Last Null
			findChannelDB->Next 		= pChannel;
			findChannelDB->Next->Next	= tempChannelDB;

			return pChannel;
		}

		findChannelDB = findChannelDB->Next;
	}
	
	return pChannel;
}

PCHANNELDB
DMB_FindChannelFromChannelDB(PCHANNELDB pRootChannelDB, PCHANNELDB pChannel)
{
	
	PCHANNELDB findChannelDB;

	if ( (pChannel == NULL) || (pRootChannelDB == NULL) )
	{
		return (PCHANNELDB) NULL;
	}

	findChannelDB = pRootChannelDB;
	
	while( findChannelDB ) // if not found ChannelDB 	return NULL
	{						// if ChannelDB is One 	return rootChannelDB
		if (findChannelDB == pChannel)
		{
			return findChannelDB;
		}
		
		findChannelDB = findChannelDB->Next;
	}
	
	return findChannelDB;
}


//return NULL mean
//if not found ChannelDB or Error
PCHANNELDB
DMB_RemoveChannelFromChannelDB(PCHANNELDB pRootChannelDB, PCHANNELDB pChannel)
{
	PCHANNELDB removeChannelDB,tempChannelDB,findChannelDB;

	findChannelDB = pRootChannelDB;

	while(findChannelDB)
	{
		if (findChannelDB->Next == pChannel)
		{
			tempChannelDB = findChannelDB->Next->Next; //pChannel->next save pointer
			findChannelDB->Next = tempChannelDB;

			return pChannel;
		}
	}
	
	return (PCHANNELDB) NULL;
}

PCHANNELDB
DMB_SearchChannelDBById(PCHANNELDB pRootChannelDB, INT8U ChannelId)
{
	PCHANNELDB removeChannelDB,tempChannelDB,findChannelDB;

	findChannelDB = pRootChannelDB;

	while(findChannelDB)
	{
		if (findChannelDB->Id == ChannelId)
		{
			return findChannelDB;
		}

		findChannelDB = findChannelDB->Next;
	}

	return (PCHANNELDB) NULL;
}


// All of branch nodes related to EnsembleDB node is disconneted and return resource
// But EnsembleDB Node doesn't return resource only Initilized
PENSEMBLEDB
DMB_UnlinkEnsembleDB (PENSEMBLEDB pEnsembleDB)
{
	PSERVICEDB pServiceDB,pRootServiceDB;
	PCHANNELDB pChannelDB,pRootChannelDB;
	PENSEMBLEDB pEnsemble,pRootEnsembleDB;

	if ( DMB_AssignedEnsembleDB (pEnsembleDB) == NULL )
	{
		return ( PENSEMBLEDB ) NULL;
	}

	pRootEnsembleDB = pEnsembleDB;

	while( pRootEnsembleDB )
	{
		pRootServiceDB = pRootEnsembleDB->Service;
		
		while( pRootServiceDB )
		{
			pRootChannelDB = pRootServiceDB->Channel;

			while( pRootChannelDB )
			{
				pChannelDB		= pRootChannelDB;	
				pRootChannelDB	= pRootChannelDB->Next;
				
				DMB_FreeChannelDB ( pChannelDB );
				DMB_InitChannelDB ( pChannelDB );
			}

			pServiceDB		= pRootServiceDB;
			pRootServiceDB	= pRootServiceDB->Next;
			
			DMB_FreeServiceDB ( pServiceDB );
			DMB_InitServiceDB ( pServiceDB );
		}

		pEnsemble		= pRootEnsembleDB;
		pRootEnsembleDB = pRootEnsembleDB->Next;

		if (pEnsemble != pEnsembleDB) 
		{
			DMB_FreeEnsembleDB ( pEnsemble );
		}
		
		DMB_InitEnsembleDB ( pEnsemble );
	}

	return pEnsembleDB;
}


PENSEMBLEDB DMB_GetEnsembleServiceNumber( PENSEMBLEDB pEnsembleDB, PINT32U pVideo, PINT32U pAudio, PINT32U pData, PINT32U pAudio_Plus, PINT32U pTotal, TDMB_INFO *pINFO )
{
	PSERVICEDB	pRootServiceDB;
	PCHANNELDB	pRootChannelDB;
	PENSEMBLEDB pRootEnsembleDB;

	if (pEnsembleDB == NULL)
	{
		return ( PENSEMBLEDB ) NULL;
	}

	if ( DMB_AssignedEnsembleDB (pEnsembleDB) == NULL )
	{
		return ( PENSEMBLEDB ) NULL;
	}

	pRootEnsembleDB = pEnsembleDB;
	
	while( pRootEnsembleDB )
	{
		pRootServiceDB = pRootEnsembleDB->Service;

		while( pRootServiceDB )
		{
			
			pRootChannelDB = pRootServiceDB->Channel;
			while( pRootChannelDB )
			{
				if ( pRootChannelDB->Type == 1 )		//dab
				{
					*pAudio = *pAudio + 1;
					
					pINFO[*pTotal].Ensemble_Band = pRootEnsembleDB->Band;
					pINFO[*pTotal].Ensemble_Freq = pRootEnsembleDB->FreqIndex;
					pINFO[*pTotal].Ensemble_Id = pRootEnsembleDB->Id;
					pINFO[*pTotal].Service_Id = pRootServiceDB->Id;
					memcpy(pINFO[*pTotal].Ensemble_Label ,pRootEnsembleDB->Label, 20);
					memcpy(pINFO[*pTotal].Service_Label ,pRootServiceDB->Label, 30);
					memset(pINFO[*pTotal].Channel_Label,0x00, 20);
					memcpy(pINFO[*pTotal].Channel_Label,pRootChannelDB->SCLabel, 17);
					pINFO[*pTotal].Channel_Id = pRootChannelDB->Id;
					pINFO[*pTotal].Channel_Type = pRootChannelDB->Type;
					pINFO[*pTotal].Channel_BitRate = pRootChannelDB->BitRate;
					memcpy(pINFO[*pTotal].Reg, pRootChannelDB->Reg, 7);

					*pTotal += 1;
				}
				else if ( pRootChannelDB->Type == 2 )	//dmb
				{
					*pVideo = *pVideo + 1;
				
					pINFO[*pTotal].Ensemble_Band = pRootEnsembleDB->Band;
					pINFO[*pTotal].Ensemble_Freq = pRootEnsembleDB->FreqIndex;
					pINFO[*pTotal].Ensemble_Id = pRootEnsembleDB->Id;
					pINFO[*pTotal].Service_Id = pRootServiceDB->Id;
					memcpy(pINFO[*pTotal].Ensemble_Label ,pRootEnsembleDB->Label, 20);
					memcpy(pINFO[*pTotal].Service_Label ,pRootServiceDB->Label, 30);
					memset(pINFO[*pTotal].Channel_Label,0x00, 20);
					memcpy(pINFO[*pTotal].Channel_Label,pRootChannelDB->SCLabel, 17);
					pINFO[*pTotal].Channel_Id = pRootChannelDB->Id;
					pINFO[*pTotal].Channel_Type = pRootChannelDB->Type;
					pINFO[*pTotal].Channel_BitRate = pRootChannelDB->BitRate;
					memcpy(pINFO[*pTotal].Reg, pRootChannelDB->Reg, 7);
				
					*pTotal += 1;
				}
				else if ( pRootChannelDB->Type == 3 )	//data
				{
					*pData = *pData + 1;
				
					pINFO[*pTotal].Ensemble_Band = pRootEnsembleDB->Band;
					pINFO[*pTotal].Ensemble_Freq = pRootEnsembleDB->FreqIndex;
					pINFO[*pTotal].Ensemble_Id = pRootEnsembleDB->Id;
					pINFO[*pTotal].Service_Id = pRootServiceDB->Id;
					memcpy(pINFO[*pTotal].Ensemble_Label ,pRootEnsembleDB->Label, 20);
					memcpy(pINFO[*pTotal].Service_Label ,pRootServiceDB->Label, 30);
					memset(pINFO[*pTotal].Channel_Label,0x00, 20);
					memcpy(pINFO[*pTotal].Channel_Label,pRootChannelDB->SCLabel, 17);
					pINFO[*pTotal].Channel_Id = pRootChannelDB->Id;
					pINFO[*pTotal].Channel_Type = pRootChannelDB->Type;
					pINFO[*pTotal].Channel_BitRate = pRootChannelDB->BitRate;
					memcpy(pINFO[*pTotal].Reg, pRootChannelDB->Reg, 7);
				
					*pTotal += 1;
				}
				else if (pRootChannelDB->Type == 4)	//dab+
				{
					*pAudio_Plus = *pAudio_Plus + 1;
				
					pINFO[*pTotal].Ensemble_Band = pRootEnsembleDB->Band;
					pINFO[*pTotal].Ensemble_Freq = pRootEnsembleDB->FreqIndex;
					pINFO[*pTotal].Ensemble_Id = pRootEnsembleDB->Id;
					pINFO[*pTotal].Service_Id = pRootServiceDB->Id;
					memcpy(pINFO[*pTotal].Ensemble_Label ,pRootEnsembleDB->Label, 20);
					memcpy(pINFO[*pTotal].Service_Label ,pRootServiceDB->Label, 30);					
					memset(pINFO[*pTotal].Channel_Label,0x00, 20);
					memcpy(pINFO[*pTotal].Channel_Label,pRootChannelDB->SCLabel, 17);
					pINFO[*pTotal].Channel_Id = pRootChannelDB->Id;
					pINFO[*pTotal].Channel_Type = pRootChannelDB->Type;
					pINFO[*pTotal].Channel_BitRate = pRootChannelDB->BitRate;
					memcpy(pINFO[*pTotal].Reg, pRootChannelDB->Reg, 7);
				
					*pTotal += 1;
				}
				
				pRootChannelDB	= pRootChannelDB->Next;
			}

			pRootServiceDB	= pRootServiceDB->Next;
		}

		pRootEnsembleDB = pRootEnsembleDB->Next;
	}

	return pEnsembleDB;
}
// end of Added by user

#endif	/* __TDMBINFO_H__ */

/* end of file */

