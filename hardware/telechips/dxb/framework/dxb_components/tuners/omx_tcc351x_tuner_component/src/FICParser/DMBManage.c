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

#define LOG_TAG	"TCC351X_DMBMANAGE"
#include <utils/Log.h>

//#define	LOGD		ALOGE
//#define	LOGD


#include "fig.h"
#include "Dmb_Types.h"
#include "TDMBInfo.h"

#define DMB_BUFFER_SIZE 1024 * 60
#define MAX_DAB_NUM	    4

/*************************************************************************************************	
*						UEP Table	
*	+---------------+-----------+-----------------------------------+
*	|Bit rate(4bits)|Protect Lv |	 	SubCh Size(9bits)	  	    |
*	|---------------|---+---+---|---+---+---+---+---+---+---+---+---|
*	| x | x | x | x | x | x | x | x | x | x | x | x | x | x | x | x |
*	+---------------+-----------+-----------------------------------+
*
***************************************************************************************************/
const unsigned short UEP_Table[64] = { 
	0x0810, 0x0615, 0x0418, 0x021d, 0x0023, 0x1818, 0x161d, 0x1423,	
	0x122a, 0x1034, 0x281d, 0x2623, 0x242a, 0x2234, 0x3820, 0x362a, 
	0x3430, 0x323a, 0x3046, 0x4828, 0x4634, 0x443a, 0x4246, 0x4054, 
	0x5830, 0x563a, 0x5446, 0x5254, 0x5068, 0x683a, 0x6646, 0x6454, 
	0x6268, 0x7840, 0x7654, 0x7460, 0x7274, 0x708c, 0x8850, 0x8668, 
	0x8474, 0x828c, 0x80a8, 0x9860, 0x9674, 0x948c, 0x92a8, 0x90d0, 
	0xa874, 0xa68c, 0xa4a8, 0xa2d0, 0xa0e8, 0xb880, 0xb6a8, 0xb4c0, 
	0xb2e8, 0xb118, 0xc8a0, 0xc6d0, 0xc318, 0xd8c0, 0xd518, 0xd1a0 };

FIC_INFO_T gFicInfo[MAX_DAB_NUM];
unsigned char g_DMBBuffer[DMB_BUFFER_SIZE];

int address_align(uint val)
{
	uint ret = 0;

	while((val & 0x3)!=0) 
	{
		val++;
		ret++;
	}
	return ret;
}

void DAB_Buff_Init(int modindex)
{
	int i;	
	const int nTuner = 0;
	
	FIC_INFO_T * pFicInfo;
	pFicInfo = &gFicInfo[nTuner];

	if(nTuner < MAX_DAB_NUM)
	{
    	memset(pFicInfo->psEsb, 0x00, sizeof(EnsembleInfo));
    	memset(pFicInfo->psSubCh, 0x00, _NumOfSubCh*sizeof(SubChInfo));
    	memset(pFicInfo->psService, 0x00, _NumOfSvc*sizeof(ServiceInfo));
    	memset(pFicInfo->psSvcComponent, 0x00, _NumOfSvcComp*sizeof(SvcComponentInfo));
    	memset(pFicInfo->psDateTime, 0x00, sizeof(DateTimeInfo));
    	memset(pFicInfo->psProgType, 0x00, _NumOfProgType*sizeof(ProgramTypeInfo));
    	memset(pFicInfo->psProgNumber, 0x00, _NumOfProgramNum*sizeof(ProgNumberInfo));
    	memset(pFicInfo->psAnnouncement, 0x00, _NumOfAnn*sizeof(AnnouncementInfo));
    	memset(pFicInfo->psTIIDatabase, 0x00, _NumOfMainId*sizeof(TIIDatabaseInfo));
    	memset(pFicInfo->psTMC_EWS, 0x00, sizeof(TMC_EWSInfo));
   		memset(pFicInfo->psDabTmcInfo, 0x00, sizeof(DabTmcInfoT));
    	memset(pFicInfo->psUserApp, 0x00, _NumOfUserApp*sizeof(UserAppInfo));

        	//initial value
    	for( i = 0; i < _NumOfSvcComp; i++)
    	{
    		pFicInfo->psSvcComponent[i].SCIdS = 0x0f;
    	}

    	for( i = 0; i < _NumOfUserApp; i++)
    	{
    		pFicInfo->psUserApp[i].SCIdS = 0x0f;
    	}
    	
    	memset(pFicInfo->psSrvLinkInfo,0x00,_NumServiceLink*sizeof(ServiceLinkInfo));
    	memset(pFicInfo->psFilist,0x00,_NumOfFI*sizeof(DABFrequencyInfo));
    	memset(pFicInfo->psOEService,0x00,_NumOfOESvc*sizeof(OEServiceInfo));
    	memset(pFicInfo->psFIG6,0x00,sizeof(FIG6Info));
    }
}

void DAB_Buffer_Alignment(unsigned char *Inst /** <DMB DAB 운영 버퍼를 위한 배열 주소> */, int modindex)
{
	uint size=0;	
	const int nTuner = 0;
	FIC_INFO_T * pFicInfo;

	memset(Inst, 0x00, DMB_BUFFER_SIZE);
	pFicInfo = &gFicInfo[nTuner];

	if(nTuner < MAX_DAB_NUM)
	{
	    	pFicInfo->psEsb = ( EnsembleInfo *)Inst;
	    	size += sizeof(EnsembleInfo);
	    	size += address_align(size);
	    	Ensemble_Mem_Align(modindex, pFicInfo->psEsb);

	    	pFicInfo->psSubCh = ( SubChInfo *)(Inst + size);
	    	size += _NumOfSubCh*sizeof(SubChInfo);
	    	size += address_align(size);
	    	SubCh_Mem_Align(modindex, pFicInfo->psSubCh, _NumOfSubCh);

	    	pFicInfo->psService = ( ServiceInfo *)(Inst + size); 
	    	size += _NumOfSvc*sizeof(ServiceInfo);
	    	size += address_align(size);
	    	Service_Mem_Align(modindex, pFicInfo->psService, _NumOfSvc);

	    	pFicInfo->psSvcComponent = ( SvcComponentInfo *)(Inst + size);
	    	size += _NumOfSvcComp*sizeof(SvcComponentInfo);
	    	size += address_align(size);
	    	SvcComponent_Mem_Align(modindex, pFicInfo->psSvcComponent, _NumOfSvcComp);

	       	pFicInfo->psDateTime = ( DateTimeInfo *)(Inst + size);
	    	size += sizeof(DateTimeInfo);
	    	size += address_align(size);
	    	DateTime_Mem_Align(modindex, pFicInfo->psDateTime);

	    	pFicInfo->psProgType = ( ProgramTypeInfo *)(Inst + size);
	    	size += _NumOfProgType*sizeof(ProgramTypeInfo);
	    	size += address_align(size);
	    	ProgramType_Mem_Align(modindex, pFicInfo->psProgType, _NumOfProgType);

	    	pFicInfo->psProgType_download = (ProgramTypedownInfo *) (Inst + size);
	    	size += sizeof(ProgramTypeInfo);
	    	size += address_align(size);
	    	ProgramType_Download_Mem_Align(modindex, pFicInfo->psProgType_download);

	    	pFicInfo->psProgNumber = ( ProgNumberInfo *)(Inst + size);
	    	size += _NumOfProgramNum*sizeof(ProgNumberInfo);
	    	size += address_align(size);
	    	ProgNumber_Mem_Align(modindex, pFicInfo->psProgNumber, _NumOfProgramNum);

	    	pFicInfo->psAnnouncement = ( AnnouncementInfo *)(Inst + size);
	    	size += _NumOfAnn*sizeof(AnnouncementInfo);
	    	size += address_align(size);
	    	Announcement_Mem_Align(modindex, pFicInfo->psAnnouncement, _NumOfAnn);

	    	pFicInfo->psUserApp = ( UserAppInfo *)(Inst + size);
	       	size += _NumOfUserApp*sizeof(UserAppInfo);
	    	size += address_align(size);
	    	UserApp_Mem_Align(modindex, pFicInfo->psUserApp, _NumOfUserApp);

	    	pFicInfo->psTIIDatabase = ( TIIDatabaseInfo *)(Inst + size);
	    	size += _NumOfMainId*sizeof(TIIDatabaseInfo);
	    	size += address_align(size);
	    	TII_Mem_Align(modindex, pFicInfo->psTIIDatabase, _NumOfMainId);

	    	pFicInfo->psTMC_EWS = ( TMC_EWSInfo *)(Inst + size);
	    	size += sizeof(TMC_EWSInfo);
	    	size += address_align(size);
	    	TMC_EWS_Mem_Align(modindex, pFicInfo->psTMC_EWS);

	        pFicInfo->psDabTmcInfo = ( DabTmcInfoT *)(Inst + size);
	       	size += sizeof(DabTmcInfoT);
	       	size += address_align(size);
	      	DabTmc_Mem_Align(modindex, pFicInfo->psDabTmcInfo);

	    	pFicInfo->pFIG5_Paging = Inst + size;
	    	size +=12;
	    	size += address_align(size);
	    	Paging_Mem_Align(modindex, pFicInfo->pFIG5_Paging);

	    	pFicInfo->psSrvLinkInfo = ( ServiceLinkInfo*)(Inst + size);
	    	size += _NumServiceLink*sizeof(ServiceLinkInfo);
	      	size += address_align(size);
	    	ServiceLinkInfo_Mem_Align(modindex, pFicInfo->psSrvLinkInfo, _NumServiceLink);

	       	pFicInfo->psFilist = ( DABFrequencyInfo*)(Inst + size);
	    	size += _NumOfFI*sizeof(DABFrequencyInfo);
	    	size += address_align(size);
	    	FrequencyListInfo_Mem_Align(modindex, pFicInfo->psFilist, _NumOfFI);

	    	pFicInfo->psOEService= ( OEServiceInfo*)(Inst + size);
	    	size += _NumOfOESvc*sizeof(OEServiceInfo);
	    	size += address_align(size);
	    	OEServiceInfo_Mem_Align(modindex, pFicInfo->psOEService, _NumOfOESvc);

	    	pFicInfo->psFIG6 = ( FIG6Info *)(Inst + size);
	    	size +=sizeof(FIG6Info);
	    	size += address_align(size);
	    	FIG6Info_Mem_Align(modindex, pFicInfo->psFIG6);
	   }
}

//UserApplication type가져오기 추가.
//INT16U DMB_GetUserAppTy(INT32U srvId, INT32U srv_comp_Id)
INT16U DMB_GetUserAppTy(PSERVICEDB pService, PCHANNELDB pChannel)
{
	int i;
	FIC_INFO_T * pFicInfo;
	const int nTuner = 0;
	
	pFicInfo = &gFicInfo[nTuner];
	UserAppInfo * pCur_user_app;

	for(i=0; i< _NumOfUserApp; i++)
	{
		pCur_user_app = pFicInfo->psUserApp + i;

		if(pService->Id == pCur_user_app->SId && pChannel->SCIdS == pCur_user_app->SCIdS)
		{
			//060817 oprsystem [Add]
			pChannel->UserAppInfo[0].Type = pCur_user_app->App[0].AppTy;
			pChannel->UserAppInfo[0].DataLen = pCur_user_app->App[0].AppLen;
			memcpy( &(pChannel->UserAppInfo[0].Data[0]), &(pCur_user_app->App[0].AppData[0]), 24 );
			
			return pCur_user_app->App[0].AppTy;
		}
	}
	return 0;
}


INT32U
DMB_BufferInit(PINT8U Inst, int modindex)
{
	DAB_Buffer_Alignment(Inst,modindex);
	DAB_Buff_Init(modindex);
	return 1;
}

INT32U
DMB_IsChannelUpdate()
{
	INT32U SvcNum , SvcCompNum,SubChNum;
	INT32U i, j;
	INT16U CrcErrFlag;
	INT8U buffer[2];
	FIC_INFO_T * pFicInfo;
	const int nTuner = 0;
	
	pFicInfo = &gFicInfo[nTuner];	
	SvcNum = pFicInfo->psEsb->NumSvc;
	SvcCompNum = pFicInfo->psEsb->NumSvcComp;
	SubChNum = pFicInfo->psEsb->NumSubCh;

	if(!SvcCompNum)	
		return CH_UPDATE_NO_DATA;
	
	if(!SubChNum)	
		return CH_UPDATE_NO_DATA;		

	if (pFicInfo->psEsb->EnsemLabel[0] == 0x00)	
		return CH_UPDATE_NO_DATA;	

	if(SvcNum)
	{
		for(i=0; i< SvcNum; i++) 
		{
			if((pFicInfo->psService+i)->SvcLabel[0] == 0)
			{
				return CH_UPDATE_NO_DATA;
			} 
		}
	} 
	else 		
		return CH_UPDATE_NO_DATA;
	

	for(i=0;i<SvcCompNum;i++)
	{
		if (pFicInfo->psSvcComponent[i].SubCh_FIDCId == 0xff)
		{
			return CH_UPDATE_NO_DATA;
		}
	}

	for(i=0;i<SvcCompNum;i++)
	{
		if( pFicInfo->psSvcComponent[i].TMId == 3 || pFicInfo->psSvcComponent[i].ASCTy_DSCTy == 5)
		{
			if(pFicInfo->psSvcComponent[i].ASCTy_DSCTy == 0)
				return CH_UPDATE_ESSENTIAL_DATA;

			for(j = 0; j < SvcCompNum; j++)
			{
				if(pFicInfo->psSvcComponent[i].SId == pFicInfo->psUserApp[j].SId && pFicInfo->psSvcComponent[i].SCIdS == pFicInfo->psUserApp[j].SCIdS)
					break;
			}
			if( j == SvcCompNum)
				return CH_UPDATE_ESSENTIAL_DATA;

			if(pFicInfo->psSvcComponent[i].SCIdS == 0x0f)
				return CH_UPDATE_ESSENTIAL_DATA;
#if 0
			if(FIC_IsSIUpdate() == 0)
				return CH_UPDATE_ESSENTIAL_DATA;
#endif
		}

#if 0
		//dmb(visual radio)
		if(psSvcComponent[i].ASCTy_DSCTy == 0x18)
		{
		}
#endif
		if(pFicInfo->psSvcComponent[i].Order)
		{
			if((pFicInfo->psSvcComponent+i)->Label[0] == 0)
			{
				return CH_UPDATE_ESSENTIAL_DATA;
			}
		}
	}
	return CH_UPDATE_FULL_DATA;
}

INT32U
DMB_TakeChannelDBFromFIC( PCHANNELDB pChannel, INT32U SvcCompIndex, INT32U SubChIndex)
{
	INT16U	uiUEPval;
	INT8U	EEP8[4] = {0, 1, 2, 3};
	INT8U	EEP32[4] = {4, 5, 6, 7};
	INT8U	UEP_Index, tmp, opt, protect;

	INT32U   BitRateTable[14] = 
		{32,48,56,64,80,96,112,128,
		 160,192,224,256,320,384};
	FIC_INFO_T * pFicInfo;
	const int nTuner = 0;
	
	pFicInfo = &gFicInfo[nTuner];
	
	int i;

	pChannel->Reg[ 6 ] = ( pFicInfo->psSubCh[SubChIndex].SubChId | 0x80 );
	pChannel->Reg[ 1 ] = ( pFicInfo->psSubCh[SubChIndex].StrtAdd & 0x0300 ) >> 8 ;
	pChannel->Reg[ 2 ] = ( pFicInfo->psSubCh[SubChIndex].StrtAdd & 0x00ff );	

	pChannel->Id = pFicInfo->psSubCh[SubChIndex].SubChId;

	//matching component with sub_ch
	for(i = 0; i < _NumOfSubCh; i++)
	{
		if(pChannel->Id == pFicInfo->psSubCh[i].SubChId)
			break;
	}

	if(i < _NumOfSubCh)
	{
		pChannel->SCIdS = pFicInfo->psSvcComponent[i].SCIdS;
		pChannel->DG_flag = pFicInfo->psSvcComponent[i].DG_MFflag;
	}
	else
	{
		pChannel->SCIdS = 0;
		pChannel->DG_flag = 0;
	}

	if( pFicInfo->psSubCh[SubChIndex].Form_Opt_Prot & 0x08 )
	{	
		opt 	= ( pFicInfo->psSubCh[SubChIndex].Form_Opt_Prot & 0x04 ) ? 1 : 0;
		protect = ( pFicInfo->psSubCh[SubChIndex].Form_Opt_Prot & 0x03 );
				
		if( opt )
		{ //32n kbps
			pChannel->Reg[ 0 ] = ( EEP32[ protect ] | 0x08 );
		}
		else
		{
			pChannel->Reg[ 0 ] = ( EEP8[ protect ] | 0x08 );
		}

		pChannel->PL 		= ( pChannel->Reg[ 0 ] & 0x07 );
		pChannel->Reg[ 3 ] 	= ( pFicInfo->psSubCh[SubChIndex].SubChSize & 0x0300 ) >> 8;
		pChannel->Reg[ 4 ] 	= ( pFicInfo->psSubCh[SubChIndex].SubChSize & 0x00ff );
						
		if( opt )
		{			
			switch( protect )
			{
				case 0: pChannel->Reg[5] = pFicInfo->psSubCh[SubChIndex].SubChSize / 27;// Bit rate
						break;
				case 1: pChannel->Reg[5] = pFicInfo->psSubCh[SubChIndex].SubChSize / 21;// Bit rate
						break;
				case 2: pChannel->Reg[5] = pFicInfo->psSubCh[SubChIndex].SubChSize / 18;// Bit rate
						break;
				case 3: pChannel->Reg[5] = pFicInfo->psSubCh[SubChIndex].SubChSize / 15;// Bit rate
						break;
			}

			pChannel->BitRate = pChannel->Reg[5] * 32;
		}
		else
		{
			switch(protect)
			{
				case 0: pChannel->Reg[5] = pFicInfo->psSubCh[SubChIndex].SubChSize / 12;// Bit rate
						break;
				case 1: pChannel->Reg[5] = pFicInfo->psSubCh[SubChIndex].SubChSize / 8;	// Bit rate
						break;
				case 2: pChannel->Reg[5] = pFicInfo->psSubCh[SubChIndex].SubChSize / 6;	// Bit rate
						break;
				case 3: pChannel->Reg[5] = pFicInfo->psSubCh[SubChIndex].SubChSize / 4;	// Bit rate
						break;
			}
			pChannel->BitRate = pChannel->Reg[5] * 8;
		}
	}
	else
	{	// UEP(Short form)
		UEP_Index = pFicInfo->psSubCh[SubChIndex].TblIndex;
	
		uiUEPval = UEP_Table[ UEP_Index ];
		tmp =  (INT8U)( (uiUEPval & 0x0e00) >> 9 );			// Protection Level
		pChannel->Reg[0] = (INT8U) ( (uiUEPval & 0x0e00) >> 9 );		// Protection Level
		pChannel->Reg[3] = (INT8U) ( (uiUEPval & 0x0100) >> 8 );	// size CU high bit[8]
		pChannel->Reg[4] = (INT8U) ( uiUEPval & 0x00ff );		// size CU low bit[7-0]
		pChannel->Reg[5] = (INT8U) ( (uiUEPval & 0xf000) >> 12 );	// Bit rate

		pChannel->BitRate =  BitRateTable[ pChannel->Reg[ 5 ] ];
	}
				
	// Channel Creation						
	pChannel->Reg[0] |= 0x80;		
					
	//// DMB
	if( pFicInfo->psSvcComponent[SvcCompIndex].ASCTy_DSCTy == 0x18 )
	{
		//RS ON
		pChannel->Reg[0] |= 0x10; 
	}
	
	return 1;
}

void
DMB_TaskEureka147Label(char *dest,char *src)
{
	int index=0;
	for( index=0; index<16; index++) {
		if(src[index]==' ' && src[index+1]==' ')
			dest[index] = NULL;
		else
			dest[index] = src[index];
	}
}

/* 
 * ===  FUNCTION  ======================================================================
 * 
 *         Name:  DMB_TakeSCINFOFromFIC
 * 
 *  Description:  CAS 와 관련된 정보를 FIC Parser 영역에서 추출
 * 
 * - PARAMETER -------------------------------------------------------------------------
 *      Mode   Type             Name            Description
 * -------------------------------------------------------------------------------------
 *        in:  ServiceId, SubChId
 *    in-out:  pscinfo
 *       out:  pcaflag - 해당 service component 의 CAflag 내용을 리턴함
 *    return:  0 : pscinfo 가 valid 함
 			   1 : pscinfo 의 주소가 NULL 임
		       2 : FIC Parser 영역의 주소가 올바르지 않음 (psEsb)
		       3 : FIC Parser 영역의 주소가 올바르지 않음 (psService)
		       4 : FIC Parser 영역의 주소가 올바르지 않음 (psSvcComponent)
		       5 : FIC Parser 영역의 주소가 올바르지 않음 (psSubCh)
		       
			   10 : SubChannel Id 를 찾지 못함 
		       11 : Service Component 를 찾지 못함
			   12 : Service Id 를 찾지 못함 

		       20 : FIC Parser 영역의 데이터가 아직 들어 오지 않음
		       0xFF : 알수 없는 에러.	   
 * -------------------------------------------------------------------------------------
 *    Author:  SangWon, Ki
 *   Created:  2006-12-13 오전 10:35:01 대한민국 표준시
 *  Revision:  none
 * =====================================================================================
 */
INT32U
DMB_TakeSCINFOFromFIC( PSC_INFO pscinfo ,PINT8U pcaflag,INT32U ServiceId, INT32U SubChId )
{
	INT32U svcnum,compnum,chnum;
	INT32U i,j,k;

	SubChInfo			*pch;
	ServiceInfo			*psvc;
	SvcComponentInfo	*pcomp;
	
	FIC_INFO_T * pFicInfo;
	const int nTuner = 0;
	
	pFicInfo = &gFicInfo[nTuner];
	pch = NULL;
	psvc = NULL;
	pcomp = NULL;

	if (pFicInfo->psEsb == NULL)
	{
		return (2);		
	}

	svcnum = pFicInfo->psEsb->NumSvc;
	compnum = pFicInfo->psEsb->NumSvcComp;
	chnum = pFicInfo->psEsb->NumSubCh;

	if ( (!svcnum) || (!compnum) || (!chnum) )	
	{
		return (20);
	}

	for(i=0; i<svcnum; i++)
	{
		psvc = &pFicInfo->psService[i];

		if (psvc == NULL)
		{
			return (3);
		}

		if ( psvc->SId == ServiceId )            /* Service ID 를 파서 영역에서 찾음 */
		{
			for ( j = 0; j < compnum; j += 1 )
			{
				pcomp = &pFicInfo->psSvcComponent[j];

				if (pcomp == NULL)
				{
					return (4);
				}

				if (pcomp->SId == ServiceId)         /* Service ID 를 SvcComponent 영역에서 찾음 */
				{
					for ( k = 0; k < chnum; k += 1 )
					{
						pch = &pFicInfo->psSubCh[k];

						if (pch == NULL)
						{
							return (5);
						}

						if (pch->SubChId == pcomp->SubCh_FIDCId) /* Sub Channel ID 를 SubChInfo 영역에서 찾음 */
						{
							pscinfo->tm_id = pcomp->TMId;
							
							pscinfo->ca_org[0] = (unsigned char) (pcomp->CAOrg >> 8) 	& 0x00ff;
							pscinfo->ca_org[1] = (unsigned char) (pcomp->CAOrg) 		& 0x00ff;

							pscinfo->subch_id = pch->SubChId;
							pscinfo->sc_id = pcomp->SCId;

							if ( pcomp->TMId == 0 )
							{
								pscinfo->asc_ty = pcomp->ASCTy_DSCTy;
								pscinfo->dsc_ty = 0;
							}
							else
							{
								pscinfo->asc_ty = 0;
								pscinfo->dsc_ty = pcomp->ASCTy_DSCTy;
							}

							pscinfo->dg_flag = pcomp->DG_MFflag;
							pscinfo->packet_addr = pcomp->PackAdd;

							memcpy(pscinfo->service_label, psvc->SvcLabel, sizeof(16));
							pscinfo->service_label[16] = '\0';
							
							memcpy(pscinfo->ensemble_label, pFicInfo->psEsb->EnsemLabel, sizeof(16));
							pscinfo->ensemble_label[16] = '\0';

							*pcaflag = pcomp->CAflag;

							return (0);
						}
					}

					return (10);
				}
			}
			return (11);
		}
	}

	return (12);

	return (0xFF);

}

PENSEMBLEDB
DMB_TakeEnsembleTreeFormFIC(PENSEMBLEDB pEnsemble)
{
	INT32U MaxSvcCount,MaxSvcCompCount,MaxSubChCount;
	INT32U SvcIndex,SvcCompIndex,SubChIndex;
	INT8U *StrType = "[Unkown]";

	PSERVICEDB  pService;
	PCHANNELDB  pChannel;
	
	FIC_INFO_T * pFicInfo;
	const int nTuner = 0;
	
	pFicInfo = &gFicInfo[nTuner];
	
	pEnsemble->Id 			= pFicInfo->psEsb->EId;
	pEnsemble->SvcNum 		= pFicInfo->psEsb->NumSvc;
	pEnsemble->SvcCompNum 	= pFicInfo->psEsb->NumSvcComp;
	pEnsemble->SubChNum 	= pFicInfo->psEsb->NumSubCh;	
	MaxSvcCount				= pFicInfo->psEsb->NumSvc;
	MaxSvcCompCount			= pFicInfo->psEsb->NumSvcComp;
	MaxSubChCount			= pFicInfo->psEsb->NumSubCh;
	pEnsemble->pEnsemble    = pFicInfo->psEsb;
	pEnsemble->charset		= pFicInfo->psEsb->charset;

	DMB_TaskEureka147Label( &(pEnsemble->Label[ 0 ]), (char *) &(pFicInfo->psEsb->EnsemLabel[ 0 ]) );

	for( SvcIndex = 0; SvcIndex < MaxSvcCount; SvcIndex++ )
	{
		pService = DMB_GetServiceDB();
		if(pService == NULL)
		{
			return (NULL);
		}

		pService->Id = pFicInfo->psService[SvcIndex].SId;
		pService->charset = pFicInfo->psService[SvcIndex].charset;
		DMB_TaskEureka147Label( &(pService->Label[ 0 ]), (char *) &(pFicInfo->psService[SvcIndex].SvcLabel[ 0 ]) );

		DMB_AddServiceToServiceDB( &pEnsemble->Service, pService );

		pService->ChannelCount = 0;
		pService->pService     = &pFicInfo->psService[SvcIndex];

		for( SvcCompIndex = 0; SvcCompIndex < MaxSvcCompCount; SvcCompIndex++ )
		{
			if ( pFicInfo->psSvcComponent[SvcCompIndex].SId == pFicInfo->psService[SvcIndex].SId )
			{
				for( SubChIndex = 0; SubChIndex < MaxSubChCount; SubChIndex++ )
				{
					if ( pFicInfo->psSvcComponent[SvcCompIndex].SubCh_FIDCId == pFicInfo->psSubCh[SubChIndex].SubChId )
					{
						pChannel = DMB_GetChannelDB();

						if(pService == NULL)
						{
							return (NULL);
						}
						
						pChannel->pSvcComp = &pFicInfo->psSvcComponent[SvcCompIndex];
						pChannel->pSubCh   = &pFicInfo->psSubCh[SubChIndex];

						DMB_AddChannelToChannelDB( &pService->Channel, pChannel );

						DMB_TakeChannelDBFromFIC( pChannel, SvcCompIndex, SubChIndex );

						pChannel->Id = pFicInfo->psSubCh[SubChIndex].SubChId;
						pChannel->Primary =  pFicInfo->psSvcComponent[SvcCompIndex].Order;
						
						pChannel->TMId = pFicInfo->psSvcComponent[SvcCompIndex].TMId;
						pChannel->ASCTy_DSCTy = pFicInfo->psSvcComponent[SvcCompIndex].ASCTy_DSCTy;
						pChannel->SCId	= pFicInfo->psSvcComponent[SvcCompIndex].SCId;
						pChannel->SCIdS	= pFicInfo->psSvcComponent[SvcCompIndex].SCIdS;
						pChannel->DG_flag = pFicInfo->psSvcComponent[SvcCompIndex].DG_MFflag;
						pChannel->PacketAddr = pFicInfo->psSvcComponent[SvcCompIndex].PackAdd;
					
						pChannel->UserApplType = DMB_GetUserAppTy(pService, pChannel);						
						pChannel->DSCTyExtension = 0;						
						pChannel->charset = pFicInfo->psSvcComponent[SvcCompIndex].charset;	
						DMB_TaskEureka147Label( pChannel->SCLabel, (char *)pFicInfo->psSvcComponent[SvcCompIndex].Label );	

						if ( pFicInfo->psSvcComponent[SvcCompIndex].ASCTy_DSCTy == 0x3F )
						{
							//StrType = " [DABP]";
							pChannel->Type = 4;
						}
						else if ( pFicInfo->psSvcComponent[SvcCompIndex].TMId == 0x00 )
						{
							//StrType = " [DAB]";
							pChannel->Type = 1;
						}
						else if ( pFicInfo->psSvcComponent[SvcCompIndex].ASCTy_DSCTy == 0x18 )
						{
							//StrType = " [DMB]";
							pChannel->Type = 2;
						}
						else
						{
							//StrType = " [DATA]";
							pChannel->Type = 3;
						}							
						//strncat(&pService->Label[0],StrType,strlen(StrType));					
						//sprintf(&pChannel->SCLabel[0],"%s %s",StrType, (char *)pFicInfo->psSvcComponent[SvcCompIndex].Label);
						pService->ChannelCount++;
					}
				}
			}
		}
	}
	return pEnsemble;
}

void DMB_SortForFICParser(void)
{
	uint a, b, count;
	ServiceInfo tempService;
	SvcComponentInfo tempSvcComp;
	SubChInfo tempSubCh;
	FIC_INFO_T * pFicInfo;
	const int nTuner = 0;
	
	pFicInfo = &gFicInfo[nTuner];
	
	count = pFicInfo->psEsb->NumSvc;

	for(a=1; a<count; ++a) 
	{
		for(b=count-1; b>=a; --b) 
		{
			if(pFicInfo->psService[b-1].SId > pFicInfo->psService[b].SId )
			{
				memcpy(&tempService, &pFicInfo->psService[b-1], sizeof(ServiceInfo));
				memcpy(&pFicInfo->psService[b-1], &pFicInfo->psService[b], sizeof(ServiceInfo));
				memcpy(&pFicInfo->psService[b], &tempService, sizeof(ServiceInfo));
			}
		}
	}

	count =pFicInfo->psEsb->NumSvcComp;

	for(a=1; a<count; ++a) 
	{
		for(b=count-1; b>=a; --b) 
		{
			if(pFicInfo->psSvcComponent[b-1].SId > pFicInfo->psSvcComponent[b].SId )
			{
				memcpy(&tempSvcComp, &pFicInfo->psSvcComponent[b-1], sizeof(SvcComponentInfo));
				memcpy(&pFicInfo->psSvcComponent[b-1], &pFicInfo->psSvcComponent[b], sizeof(SvcComponentInfo));
				memcpy(&pFicInfo->psSvcComponent[b], &tempSvcComp, sizeof(SvcComponentInfo));
			}
		}
	}

	count =pFicInfo->psEsb->NumSubCh;

	for(a=1; a<count; ++a) 
	{
		for(b=count-1; b>=a; --b) 
		{
			if(pFicInfo->psSubCh[b-1].SubChId > pFicInfo->psSubCh[b].SubChId )
			{
				memcpy(&tempSubCh, &pFicInfo->psSubCh[b-1], sizeof(SubChInfo));
				memcpy(&pFicInfo->psSubCh[b-1], &pFicInfo->psSubCh[b], sizeof(SubChInfo));
				memcpy(&pFicInfo->psSubCh[b], &tempSubCh, sizeof(SubChInfo));
			}
		}
	}
}

FIC_INFO_T * DMB_Get_FIC_Info(void)
{
	FIC_INFO_T * pFicInfo;
	const int nTuner = 0;
	
	pFicInfo = &gFicInfo[nTuner];

	return pFicInfo;
}

/* ----- End of File ---- */
