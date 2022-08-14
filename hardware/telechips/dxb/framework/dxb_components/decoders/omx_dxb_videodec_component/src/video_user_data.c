/**

  Copyright (C) 2009-2010 Telechips Inc.

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA
  02110-1301  USA

*/

/****************************************************************************
 *   FileName    : video_user_data.c
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/
#include <video_user_data.h>
#include "TCCMemory.h"
#ifdef VIDEO_USER_DATA_PROCESS
//#define RECORD_USER_DATA
#if 0//def RECORD_USER_DATA
	#include <StreamWriter.h>
#endif

#define LOG_TAG	"OMX_TCC_VIDEO_DEC"
#include <utils/Log.h>

static int DEBUG_ON = 0;

#define PRINTF(msg...)	if (DEBUG_ON) { ALOGD( ": " msg);}
#define PRINTF_ERROR(msg...)	{ ALOGE( ": " msg);}

int UserDataCtrl_Clear( video_userdata_list_t *pUserDataList )
{
	int i;
	for( i=0; i<pUserDataList->iUserDataNum; i++ )
	{
		if( pUserDataList->arUserData[i].pData )
		{
			TCC_free( pUserDataList->arUserData[i].pData );
			pUserDataList->arUserData[i].pData = NULL;
		}
		pUserDataList->arUserData[i].iDataSize = 0;
	}
	pUserDataList->iUserDataNum = 0;
	pUserDataList->iValidCount = 0;

	return 0;
}

int UserDataCtrl_Init( video_userdata_list_t *pUserDataListArray )
{
	PRINTF( "[%s]\n", __func__ );
	memset( pUserDataListArray, 0, MAX_USERDATA_LIST_ARRAY * sizeof(video_userdata_list_t) );

	return 0;
}

int UserDataCtrl_ResetAll( video_userdata_list_t *pUserDataListArray )
{
	int i,j;

	PRINTF( "[%s]\n", __func__ );

	for( i=0; i<MAX_USERDATA_LIST_ARRAY; i++)
	{
		video_userdata_list_t *pUserDataList = &pUserDataListArray[i];

		if( pUserDataList->iValidCount > 0 )
		{
			UserDataCtrl_Clear( pUserDataList );
		}
	}

	return 0;
}

int UserDataCtrl_Put( video_userdata_list_t *pUserDataListArray, int iIndex,
                             unsigned long long ullPTS, unsigned char fDiscontinuity, video_userdata_list_t *pUserDataList )
{
	int i;
	video_userdata_list_t *pEmptyList;

	PRINTF("[%s] Idx %2d, %llu\n", __func__, iIndex, ullPTS);

	for( i=0; i<MAX_USERDATA_LIST_ARRAY; i++)
	{
		if( pUserDataListArray[i].iValidCount == 0 )
		{
			break;
		}
	}

	if( i >= MAX_USERDATA_LIST_ARRAY )
	{
		PRINTF_ERROR( "[%s] iIndex(%d) bigger than max %d\n", __func__, iIndex, MAX_USERDATA_LIST_ARRAY );
		return -1;
	}

	pEmptyList = &pUserDataListArray[i];
	
	pEmptyList->iIndex = iIndex;
	pEmptyList->ullPTS = ullPTS;
	pEmptyList->fDiscontinuity = fDiscontinuity;
	pEmptyList->iValidCount = MAX_USERDATA_LIST_ARRAY-1;

	for( i=0; i<pUserDataList->iUserDataNum; i++ )
	{
		pEmptyList->arUserData[i].pData = TCC_malloc( pUserDataList->arUserData[i].iDataSize );
		if( pEmptyList->arUserData[i].pData == NULL )
		{
			PRINTF_ERROR( "[%s] Error. UserData->pData TCC_malloc fail. size %d\n", __func__, pUserDataList->arUserData[i].iDataSize );
			UserDataCtrl_Clear( pEmptyList );
			return -1;
		}
		memcpy( pEmptyList->arUserData[i].pData
		      , pUserDataList->arUserData[i].pData, pUserDataList->arUserData[i].iDataSize );
		pEmptyList->arUserData[i].iDataSize = pUserDataList->arUserData[i].iDataSize;

		pEmptyList->iUserDataNum++;
	}

	return 0;
}

int UserDataCtrl_Get( video_userdata_list_t *pUserDataListArray, int iIndex,
							 video_userdata_list_t **ppUserDataList )
{
	int i;
	int iFoundIndex = -1;
	video_userdata_list_t *pUserDataList;

	for( i=0; i<MAX_USERDATA_LIST_ARRAY; i++ )
	{
		pUserDataList = &pUserDataListArray[i];
		if( pUserDataList->iValidCount > 0 )
		{
			if( pUserDataList->iIndex == iIndex )
			{
				/* found matched PTS */
				iFoundIndex = i;
			}
			else if( --pUserDataList->iValidCount <= 0 )
			{
				/* remove past PTS */
				UserDataCtrl_Clear( pUserDataList );
			}
		}
	}

	if( iFoundIndex >= 0 )
	{
		*ppUserDataList = &pUserDataListArray[iFoundIndex];
		return iFoundIndex;
	}
	return -1;
}

#if 0
static unsigned char* dump_hex(unsigned char *pBuf, int size )
{
	int j;
	int str_length = size;
	char *szDumpString = TCC_malloc(str_length*3+1);
	char *p_strbuf = szDumpString;
	for( j=0; j<str_length; j++)
	{
		p_strbuf += sprintf( p_strbuf, "%02X ", pBuf[j] );
	}
	*p_strbuf = 0;
	//printf( "%s\n", szDumpString);
	//TCC_malloc( szDumpString );
	return (unsigned char*)szDumpString;
}
#endif

#if 0
void disp_user_data(unsigned char * pUserData, int iOutputStatus, DVBT_AuInfo* p_fifo_info, void *param )
{
	int i, j;
	unsigned char * pTmpPTR;
	unsigned char * pRealData;
	unsigned int nNumUserData;
	unsigned int nTotalSize;
	unsigned int nDataSize;

	pTmpPTR = pUserData;
	nNumUserData = (pTmpPTR[0] << 8) | pTmpPTR[1];
	nTotalSize = (pTmpPTR[2] << 8) | pTmpPTR[3];

	pTmpPTR = pUserData + 8;
	pRealData = pUserData + (8 * 17);

	//printf( "\nret=%d\n", iOutputStatus);
	for(i = 0;i < nNumUserData;i++)
	{
		nDataSize = (pTmpPTR[2] << 8) | pTmpPTR[3];

		PRINTF( "[User Data][Idx : %02d][Size : %05d]", i, nDataSize);
		for(j = 0;j < nDataSize;j++)
		{
			PRINTF( "%02x ", pRealData[j]);
		}
		PRINTF( "\n");

		pTmpPTR += 8;
		pRealData += nDataSize;
	}
}
#endif

#if 0//def RECORD_USER_DATA
void record_user_data( int iFileHandle, video_userdata_list_t *pUserDataList )
{
	unsigned long long ullPTS = (pUserDataList->ullPTS)*9/100;
	unsigned char fDiscontinuity = pUserDataList->fDiscontinuity;
	int iFrameSize = 0;
	int iOutputStatus = 0;

	int i;
	unsigned char szTempBuf[64];
	unsigned char *pTempBuf;
	unsigned char *pSrc;

	unsigned char fCCdataFound = FALSE;
	
	pTempBuf = szTempBuf;

	*pTempBuf++ = 0xA5;
	*pTempBuf++ = 0x5A;
	*pTempBuf++ = 0xA5;
	*pTempBuf++ = 0x5A;

	pSrc = (unsigned char *)&ullPTS;
	pSrc += sizeof(ullPTS)-1;
	for( i=0; i<sizeof(ullPTS); i++)
	{
		*pTempBuf++ = *pSrc--;
	}
	
	pSrc = (unsigned char *)&iFrameSize;
	pSrc += sizeof(iFrameSize)-1;
	for( i=0; i<sizeof(iFrameSize);i++)
	{
		*pTempBuf++ = *pSrc--;
	}

	*pTempBuf++= fDiscontinuity;
	*pTempBuf++= (unsigned char)iOutputStatus;

	StreamWriter_Write( iFileHandle, szTempBuf, (int)(pTempBuf-szTempBuf) );

	for( i=0; i<pUserDataList->iUserDataNum; i++ )
	{
		if( memcmp( pUserDataList->arUserData[i].pData, ATSC_identifier, 4 ) == 0 )
		{
			szTempBuf[0] = (pUserDataList->arUserData[i].iDataSize >> 8) &0xFF;
			szTempBuf[1] = pUserDataList->arUserData[i].iDataSize & 0xFF;
			StreamWriter_Write( iFileHandle, szTempBuf, 2);
			StreamWriter_Write( iFileHandle, pUserDataList->arUserData[i].pData, pUserDataList->arUserData[i].iDataSize );
			fCCdataFound = TRUE;
		}
	}

	if( fCCdataFound == FALSE )
	{
		szTempBuf[0] = 0;
		szTempBuf[1] = 0;
		StreamWriter_Write( iFileHandle, szTempBuf, 2);
	}
}
#endif //ifdef RECORD_USER_DATA

#endif //ifdef VIDEO_USER_DATA_PROCESS

