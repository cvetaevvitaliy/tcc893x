/****************************************************************************
 *   FileName    : cdmx_audio.c
 *   Description : container demuxer of AUDIO
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall constitute any express or implied warranty of any kind, including without limitation, any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright or other third party intellectual property right. No warranty is made, express or implied, regarding the information¡¯s accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between Telechips and Company.
*
****************************************************************************/
#include "utils/Log.h"
#include "../../cdk/cdk_core.h"

#ifdef INCLUDE_AUDIO_DMX

#include "../../cdk/cdk.h"
#include "../cdmx.h"
#include <cutils/properties.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	LOG
//
#define LOG_TAG    "CDMX_AUDIO"

//#define ENABLE_LOGD
//#define ENABLE_LOG_FUNC
//#define ENABLE_LOG_STEP
//#define DBG_PRINT_DATA

#ifdef ENABLE_LOGD
#undef DPRINTF
#undef DSTATUS
#define DPRINTF(x...) ALOGD(x)
#define DSTATUS(x...) ALOGD(x)
#endif


#ifdef ENABLE_LOG_FUNC
#define FUNC(...)			{ALOGE  ("[FUNC] " __VA_ARGS__);}
#else
#define FUNC(...)			{DPRINTF("[FUNC] " __VA_ARGS__);}
#endif

#ifdef ENABLE_LOG_STEP
#define STEP(...)  			{ALOGI	("[STEP] " __VA_ARGS__);}
#else
#define STEP(...)  			{DPRINTF("[STEP] " __VA_ARGS__);}
#endif

#define INFO(...)			{ALOGD  ("[INFO] " __VA_ARGS__);}
#define ERROR(...)  		{ALOGE(__VA_ARGS__);}
#define DATA(...)  			{ALOGD(__VA_ARGS__);}
#define LOG_DEC(...) 		{ALOGD(__VA_ARGS__);}


typedef cdk_callback_func_t cdmx_callback_func_t;

#define cdmx_audio_malloc	((*pst_inst).stDmxInit.m_sCallbackFunc).m_pMalloc
#define cdmx_audio_free		((*pst_inst).stDmxInit.m_sCallbackFunc).m_pFree
#define cdmx_audio_memset	((*pst_inst).stDmxInit.m_sCallbackFunc).m_pMemset
#define cdmx_audio_memcpy	((*pst_inst).stDmxInit.m_sCallbackFunc).m_pMemcpy
#define cdmx_audio_memmove	((*pst_inst).stDmxInit.m_sCallbackFunc).m_pMemmove
#define cdmx_audio_realloc	((*pst_inst).stDmxInit.m_sCallbackFunc).m_pRealloc

#define cdmx_malloc 	cdmx_audio_malloc
#define cdmx_free 		cdmx_audio_free
#define cdmx_memset 	cdmx_audio_memset
#define cdmx_memcpy 	cdmx_audio_memcpy
#define cdmx_memmove	cdmx_audio_memmove
#define cdmx_realloc 	cdmx_audio_realloc

#define SET_CDMX_INFO_FILE 		1
#define SET_CDMX_INFO_VIDEO 	2
#define SET_CDMX_INFO_AUDIO 	4
#define SET_CDMX_INFO_SUBTITLE 	8
#define SET_CDMX_INFO_ALL 		(SET_CDMX_INFO_FILE|SET_CDMX_INFO_VIDEO|SET_CDMX_INFO_AUDIO|SET_CDMX_INFO_SUBTITLE)
static
void 
set_cdmx_info(
    cdmx_info_t			*pstCdmxInfo,
    audio_dmx_inst_t	*pstInst,
	unsigned long 	 	 ulType
	)
{
	audio_dmx_inst_t	*pst_inst    = pstInst;
	cdmx_info_t			*p_out_param = pstCdmxInfo;

	//
	// [1] file info
	//
	if ( ulType & SET_CDMX_INFO_FILE )
	{
		//FUNC("In set_cdmx_info: SET_CDMX_INFO_FILE \n");
		cdmx_memset( &p_out_param->m_sFileInfo, 0, sizeof(p_out_param->m_sFileInfo) );
		p_out_param->m_sFileInfo.m_pszOpenFileName		= pst_inst->stDmxInfo.m_sFileInfo.m_pszOpenFileName;
		p_out_param->m_sFileInfo.m_pszCopyright			= NULL;
		p_out_param->m_sFileInfo.m_pszCreationTime		= NULL;
		p_out_param->m_sFileInfo.m_iRunningtime			= pst_inst->stDmxInfo.m_sFileInfo.m_iRunningtime;
		p_out_param->m_sFileInfo.m_lFileSize			= (int64_t)pst_inst->stDmxInfo.m_sFileInfo.m_iFileSize;
		p_out_param->m_sFileInfo.m_bHasIndex			= 0;
		p_out_param->m_sFileInfo.m_iSuggestedBufferSize	= 0;
		p_out_param->m_sFileInfo.m_iTotalStreams		= 0;
		p_out_param->m_sFileInfo.m_iTimeScale			= 0;
		p_out_param->m_sFileInfo.m_lUserDataPos			= 0;
		p_out_param->m_sFileInfo.m_iUserDataLen			= 0;
		p_out_param->m_sFileInfo.m_pcTitle				= NULL;				
		p_out_param->m_sFileInfo.m_pcArtist				= NULL;
		p_out_param->m_sFileInfo.m_pcAlbum				= NULL;
		p_out_param->m_sFileInfo.m_pcAlbumArtist		= NULL;
		p_out_param->m_sFileInfo.m_pcYear				= NULL;
		p_out_param->m_sFileInfo.m_pcWriter				= NULL;
		p_out_param->m_sFileInfo.m_pcAlbumArt			= NULL;
		p_out_param->m_sFileInfo.m_iAlbumArtSize		= 0;
		p_out_param->m_sFileInfo.m_pcGenre				= NULL;
		p_out_param->m_sFileInfo.m_pcCompilation		= NULL;
		p_out_param->m_sFileInfo.m_pcCDTrackNumber		= NULL;
		p_out_param->m_sFileInfo.m_pcDiscNumber			= NULL;
		p_out_param->m_sFileInfo.m_pcLocation			= NULL;
		p_out_param->m_sFileInfo.m_iSeekable			= 1;
		p_out_param->m_sFileInfo.m_ulSecureType			= 0;
		p_out_param->m_sFileInfo.m_iFragmentedMp4		= 0;
	}
	// 
	// [3] audio info
	//
	if ( ulType & SET_CDMX_INFO_AUDIO )
	{
		//FUNC("In set_cdmx_info: SET_CDMX_INFO_AUDIO \n");
		cdmx_memset( &p_out_param->m_sAudioInfo, 0, sizeof(p_out_param->m_sAudioInfo) );
		p_out_param->m_sAudioInfo.m_iTotalNumber		= pst_inst->stDmxInfo.m_sAudioInfo.m_iTotalNumber;
		p_out_param->m_sAudioInfo.m_iSamplePerSec		= pst_inst->stDmxInfo.m_sAudioInfo.m_iSamplePerSec;
		p_out_param->m_sAudioInfo.m_iBitsPerSample		= pst_inst->stDmxInfo.m_sAudioInfo.m_iBitsPerSample;
		p_out_param->m_sAudioInfo.m_iChannels			= pst_inst->stDmxInfo.m_sAudioInfo.m_iChannels;
		p_out_param->m_sAudioInfo.m_iFormatId			= pst_inst->stDmxInfo.m_sAudioInfo.m_iFormatId;
		p_out_param->m_sAudioInfo.m_ulmax_bit_rate		= 0;
		p_out_param->m_sAudioInfo.m_ulavg_bit_rate		= 0;
		p_out_param->m_sAudioInfo.m_pszCodecName		= NULL;
		p_out_param->m_sAudioInfo.m_pszCodecVendorName	= NULL;
		p_out_param->m_sAudioInfo.m_pExtraData			= pst_inst->stDmxInfo.m_sAudioInfo.m_pExtraData;
		p_out_param->m_sAudioInfo.m_iExtraDataLen		= pst_inst->stDmxInfo.m_sAudioInfo.m_iExtraDataLen;
		p_out_param->m_sAudioInfo.m_pszlanguageInfo		= NULL;
		p_out_param->m_sAudioInfo.m_iFramesPerSample	= 0;
		p_out_param->m_sAudioInfo.m_iTrackTimeScale		= 0;
		p_out_param->m_sAudioInfo.m_iSamplesPerPacket	= 0;
		p_out_param->m_sAudioInfo.m_iNumAudioStream		= 0;
		p_out_param->m_sAudioInfo.m_iCurrAudioIdx		= 0;
		p_out_param->m_sAudioInfo.m_iBlockAlign			= pst_inst->stDmxInfo.m_sAudioInfo.m_iBlockAlign;
		p_out_param->m_sAudioInfo.m_iAvgBytesPerSec		= 0;
		p_out_param->m_sAudioInfo.m_iAudioStreamID		= 0;
		p_out_param->m_sAudioInfo.m_iBitRate			= pst_inst->stDmxInfo.m_sAudioInfo.m_iBitRate;
		p_out_param->m_sAudioInfo.m_iActualRate			= 0;
		p_out_param->m_sAudioInfo.m_sAudioQuality		= 0;
		p_out_param->m_sAudioInfo.m_sFlavorIndex		= 0;
		p_out_param->m_sAudioInfo.m_iBitsPerFrame		= 0;
		p_out_param->m_sAudioInfo.m_iGranularity		= 0;
		p_out_param->m_sAudioInfo.m_iOpaqueDataSize		= 0;
		p_out_param->m_sAudioInfo.m_pOpaqueData			= 0;
		p_out_param->m_sAudioInfo.m_iSamplesPerFrame	= 0;
		p_out_param->m_sAudioInfo.m_iframeSizeInBits	= 0;
		p_out_param->m_sAudioInfo.m_iSampleRate			= 0;
		p_out_param->m_sAudioInfo.m_scplStart			= 0;
		p_out_param->m_sAudioInfo.m_scplQBits			= 0;
		p_out_param->m_sAudioInfo.m_snRegions			= 0;
		p_out_param->m_sAudioInfo.m_sdummy				= 0;
		p_out_param->m_sAudioInfo.m_ulSubType			= 0;
		p_out_param->m_sAudioInfo.m_iMinDataSize		= pst_inst->stDmxInfo.m_sAudioInfo.m_iMinDataSize;
	}
}

static 
long 
determine_audio_dmx(
    audio_dmx_inst_t		*pstInst,
	cdmx_callback_func_t	*pstCallback,
	char 					*pszFileName
   )
{
	long ret = DMXRET_SUCCESS;
	void *fp_inp;
	unsigned char buffer[16];
	unsigned long last_pos;
	audio_dmx_inst_t	 *pst_inst = pstInst;
	cdmx_callback_func_t *pf_callback = pstCallback;

	FUNC("In determine_audio_dmx\n");

	static cdk_audio_func_t *gspfCDmxAudioList[10] =
	{
		TCC_AAC_DMX,  TCC_MP23_DMX, TCC_MP23_DMX, 0, TCC_FLAC_DMX, TCC_APE_DMX,  0,
	#ifdef INCLUDE_MP3HD_DEC
		TCC_MP3HD_DMX,
	#else
		0,
	#endif
	}; 


	fp_inp = pf_callback->m_pfFopen(pszFileName, "rb");
	if( fp_inp == NULL )
	{
		ERROR("cdmx_audio file open failed \n");
		return DMXRET_FILE_OPEN_FAILED;
	}

	pf_callback->m_pfFread( buffer, 10, 1, fp_inp );

	pstInst->stDmxInit.m_uiId3TagOffset = 0;
	if( !cdk_strncmp((char*)buffer, "ID3", 3) )
	{
		pstInst->stDmxInit.m_uiId3TagOffset = (buffer[6] << 21) | (buffer[7] << 14) | (buffer[8] <<  7) | (buffer[9] <<  0);
		pstInst->stDmxInit.m_uiId3TagOffset += 10;
	}

	pf_callback->m_pfFseek(fp_inp, pstInst->stDmxInit.m_uiId3TagOffset, 0);	// skip id3 tag

	last_pos = pf_callback->m_pfFtell( fp_inp );

	pf_callback->m_pfFread( buffer, 16, 1, fp_inp );

	if( ( cdk_strncmp((char*)&buffer[0], "RIFF", 4) == 0)  && ( cdk_strncmp((char*)&buffer[8], "WAVE", 4) == 0 ) )
	{
		pstInst->stDmxInit.m_uiDmxType = CONTAINER_TYPE_AUDIO_WAV;
		pstInst->pfnDmxLib = gspfCDmxAudioList[CONTAINER_TYPE_AUDIO_WAV];
		INFO( " Audio Demuxer is WAV\n" );
	}
	else if( !cdk_strncmp((char*)buffer, "fLaC", 4) )
	{
		pstInst->stDmxInit.m_uiDmxType = CONTAINER_TYPE_AUDIO_FLAC;
		pstInst->pfnDmxLib = gspfCDmxAudioList[CONTAINER_TYPE_AUDIO_FLAC];
		pstInst->stDmxInfo.m_sAudioInfo.m_iFormatId = AV_AUDIO_FLAC;
		INFO( " Audio Demuxer is FLAC\n" );
	}
	else if( cdk_strncmp((char*)&buffer[0], "ADIF", 4) == 0 )	// AAC ADIF
	{
		pstInst->stDmxInit.m_uiDmxType = CONTAINER_TYPE_AUDIO_AAC;
		pstInst->pfnDmxLib = gspfCDmxAudioList[CONTAINER_TYPE_AUDIO_AAC];
		pstInst->stDmxInfo.m_sAudioInfo.m_iFormatId = AV_AUDIO_AAC;
		INFO( " Audio Demuxer is AAC-ADIF\n" );
	}
	else if( !cdk_strncmp((char*)buffer, "MAC ", 4) )
	{
		pstInst->stDmxInit.m_uiDmxType = CONTAINER_TYPE_AUDIO_APE;
		pstInst->pfnDmxLib = gspfCDmxAudioList[CONTAINER_TYPE_AUDIO_APE];
		pstInst->stDmxInfo.m_sAudioInfo.m_iFormatId = AV_AUDIO_APE;
		INFO( " Audio Demuxer is APE\n" );
	}
	else if( ( ret = is_mpeg_audio( &pstInst->stDmxInit, fp_inp ) ) > 0 ) // mp3 or mp2
	{
		if( ret == 3 )
		{
		#ifdef INCLUDE_MP3HD_DEC
			int iret;
			iret = CheckMP3HD(&pstInst->stDmxInit.m_sCallbackFunc, fp_inp);
			INFO("Check MP3HD %d", iret);
			if (iret) 
			{
				pstInst->stDmxInit.m_uiDmxType = CONTAINER_TYPE_AUDIO_MP3HD;
				pstInst->pfnDmxLib = gspfCDmxAudioList[CONTAINER_TYPE_AUDIO_MP3HD];
				pstInst->stDmxInfo.m_sAudioInfo.m_iFormatId = AV_AUDIO_MP3HD;
				INFO(" Audio Demuxer is MP3HD\n");
			} 
			else 
		#endif
			{
				pstInst->stDmxInit.m_uiDmxType = CONTAINER_TYPE_AUDIO_MP3;
				pstInst->pfnDmxLib = gspfCDmxAudioList[CONTAINER_TYPE_AUDIO_MP3];
				pstInst->stDmxInfo.m_sAudioInfo.m_iFormatId = AV_AUDIO_MP3;
				INFO(" Audio Demuxer is MP3\n");
			}
		}
		else if(ret == 2 || ret == 1)
		{
			pstInst->stDmxInit.m_uiDmxType = CONTAINER_TYPE_AUDIO_MP2;
			pstInst->pfnDmxLib = gspfCDmxAudioList[CONTAINER_TYPE_AUDIO_MP2];
			pstInst->stDmxInfo.m_sAudioInfo.m_iFormatId = AV_AUDIO_MP2;
			if( ret == 2)
			{
				INFO( " Audio Demuxer is MP2\n" );
			}
			else
			{
				DSTATUS( " Audio Demuxer is MP1\n" );
			}
		}
	}
	else	//AAC ADTS
	{
		// check AAC ADTS or LATM
		unsigned char *pucStream;
		int iRet, iRead;
		int iBuffSize = 768*10;

		pucStream = (unsigned char *)pf_callback->m_pfMalloc( iBuffSize );
		if( pucStream == NULL )
			return -1;	

		pf_callback->m_pfFseek( fp_inp, last_pos, SEEK_SET );
		
		iRead = (int)pf_callback->m_pfFread( pucStream, 1, iBuffSize, fp_inp );
			
		iRet = tcc_aac_dmx_check_adts(pucStream, iRead);

		if(iRet == 0 )
		{
			pstInst->stDmxInit.m_uiDmxType = CONTAINER_TYPE_AUDIO_AAC;
			pstInst->pfnDmxLib = gspfCDmxAudioList[CONTAINER_TYPE_AUDIO_AAC];
			pstInst->stDmxInfo.m_sAudioInfo.m_iFormatId = AV_AUDIO_AAC;
			INFO( " Audio Demuxer is AAC-ADTS\n" );
		}
		else
		{
			pstInst->pfnDmxLib = NULL;
			INFO( " Unknown Format\n" );
		}
		
		pf_callback->m_pfFree( pucStream );
	}
	
	pf_callback->m_pfFclose( fp_inp );

	if( pstInst->pfnDmxLib == NULL )
		return DMXRET_FAILED;

	return ret;
}


av_result_t 
cdmx_audio( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 )
{
	av_result_t ret = DMXRET_SUCCESS;
	audio_dmx_inst_t* pst_inst = (audio_dmx_inst_t*)(*ptHandle); // assume that its memory is allocated.

	if( ulOpCode == CDMX_INIT )
	{
		cdmx_init_t				*pst_init_param = (cdmx_init_t*)pParam1;
		cdmx_callback_func_t	*pst_callback   = (cdmx_callback_func_t*)pst_init_param->pstCallback;
		
		if( pst_callback->m_pfMalloc	== 0 ||
			pst_callback->m_pfFree		== 0 ||
			pst_callback->m_pfMemcpy	== 0 ||
			pst_callback->m_pfMemset	== 0 ||
			pst_callback->m_pfMemmove	== 0 ||
			pst_callback->m_pfRealloc	== 0 )
		{
			return DMXRET_FAILED;
		}

		STEP( "In CDMX_INIT\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		//--------------------------------------------------
		// find audio demuxer
		//--------------------------------------------------
		ret = determine_audio_dmx(pst_inst, pst_callback, pst_init_param->pszFileName);
		if (ret < 0) {
			ERROR( "determine_audio_dmx failed ...\n" );
			return ret;
		}

		#if 0
		{
			void* pv_dmx;
			void* pv_handle;
			ret = open_library( (const char*)"/system/lib/libTCC_AUDIO_DMX.so",
								(const char*)"TCC_AUDIO_DMX",
								&pv_dmx,
								&pv_handle );
			if (ret < 0) {
				return ret;
			}
			pst_inst->pfnDmxLib = ( av_result_t (*) (unsigned long, av_handle_t*, void*, void*) )(pv_dmx);
			pst_inst->pvLibHandle = pv_handle;
			STEP( "LibHandle loaded....(0x%x) (handle:0x%x) ...\n", pst_inst->pfnDmxLib, pst_inst->pvLibHandle );
		}
		#endif

		//--------------------------------------------------
		// set init parameters
		//--------------------------------------------------
		pst_inst->stDmxInit.m_pOpenFileName = (char*)pst_init_param->pszFileName;
		
		pst_inst->stDmxInit.m_sCallbackFunc.m_pMalloc			= pst_callback->m_pfMalloc;
		pst_inst->stDmxInit.m_sCallbackFunc.m_pFree				= pst_callback->m_pfFree;
		pst_inst->stDmxInit.m_sCallbackFunc.m_pNonCacheMalloc	= pst_callback->m_pfNonCacheMalloc;
		pst_inst->stDmxInit.m_sCallbackFunc.m_pNonCacheFree		= pst_callback->m_pfNonCacheFree;
		pst_inst->stDmxInit.m_sCallbackFunc.m_pMemcpy			= pst_callback->m_pfMemcpy;
		pst_inst->stDmxInit.m_sCallbackFunc.m_pMemset			= pst_callback->m_pfMemset;
		pst_inst->stDmxInit.m_sCallbackFunc.m_pRealloc			= pst_callback->m_pfRealloc;
		pst_inst->stDmxInit.m_sCallbackFunc.m_pMemmove			= pst_callback->m_pfMemmove;
		pst_inst->stDmxInit.m_sCallbackFunc.m_pFopen			= pst_callback->m_pfFopen;
		pst_inst->stDmxInit.m_sCallbackFunc.m_pFread			= pst_callback->m_pfFread;
		pst_inst->stDmxInit.m_sCallbackFunc.m_pFseek			= pst_callback->m_pfFseek;
		pst_inst->stDmxInit.m_sCallbackFunc.m_pFtell			= pst_callback->m_pfFtell;
		pst_inst->stDmxInit.m_sCallbackFunc.m_pFwrite			= pst_callback->m_pfFwrite;
		pst_inst->stDmxInit.m_sCallbackFunc.m_pFclose			= pst_callback->m_pfFclose;
		pst_inst->stDmxInit.m_sCallbackFunc.m_pFseek64          = pst_callback->m_pfFseek64;
		pst_inst->stDmxInit.m_sCallbackFunc.m_pFtell64			= pst_callback->m_pfFtell64;
		pst_inst->stDmxInit.m_sCallbackFunc.m_pStrncmp          = cdk_strncmp;

		{
			av_memory_func_t st_mem_funcs;
			st_mem_funcs.m_pfnMalloc	= pst_callback->m_pfMalloc;
			st_mem_funcs.m_pfnFree		= pst_callback->m_pfFree;
			st_mem_funcs.m_pfnMemcmp	= pst_callback->m_pfMemcmp;
			st_mem_funcs.m_pfnMemcpy	= pst_callback->m_pfMemcpy;
			st_mem_funcs.m_pfnMemset	= pst_callback->m_pfMemset;
			st_mem_funcs.m_pfnRealloc	= pst_callback->m_pfRealloc;
			st_mem_funcs.m_pfnMemmove	= pst_callback->m_pfMemmove;
			cdmx_memory_func_init(&st_mem_funcs);
		}

		STEP( "\tAUDIO_DMX_OPEN 01\n" );
		ret = pst_inst->pfnDmxLib( AUDIO_DMX_OPEN, &pst_inst->hDmxHandle, &pst_inst->stDmxInit, &pst_inst->stDmxInfo );
		STEP( "\tAUDIO_DMX_OPEN 02\n" );
		if( ret < 0 )
		{
			ERROR( "[%ld] Error: AUDIO_DMX_OPEN failed (%d)\n", ret, __LINE__ );
            return ret;
        }

		STEP( "AUDIO_DMX_OPEN: END\n" );

		if(pst_inst->stDmxInit.m_uiDmxType == CONTAINER_TYPE_AUDIO_AAC)
		{
			if(pst_inst->stDmxInfo.m_sAudioInfo.m_iAacBSAC == 1)
				pst_inst->stDmxInfo.m_sAudioInfo.m_iFormatId = AV_AUDIO_BSAC_CDK;
		}

		///////////////////////////////////////////////////////////////////////////////
		//
		// set control variables & ..
		//
		{
			audio_dmx_file_info_t st_fileinfo;
			audio_dmx_audio_info_t st_audioinfo;
			audio_dmx_info_list_t st_dmxinfo_list;
			
			st_dmxinfo_list.m_lAudioStreamTotal = 1;
			st_dmxinfo_list.m_pstFileInfo = &st_fileinfo;
			st_dmxinfo_list.m_pstDefaultAudioInfo = &st_audioinfo;		
			//st_dmxinfo_list.m_pstAudioInfoList= &st_audioinfo;			
			st_dmxinfo_list.m_pstDefaultVideoInfo = NULL;
			st_dmxinfo_list.m_pstVideoInfoList = NULL;			
			st_dmxinfo_list.m_lVideoStreamTotal = 0;

			st_fileinfo.m_llFileSize = pst_inst->stDmxInfo.m_sFileInfo.m_iFileSize;
			st_fileinfo.m_pszFileName = pst_inst->stDmxInfo.m_sFileInfo.m_pszOpenFileName = pst_init_param->pszFileName;
			st_fileinfo.m_ulDuration = pst_inst->stDmxInfo.m_sFileInfo.m_iRunningtime;
			
			st_audioinfo.m_ulStreamID = pst_inst->stDmxInfo.m_sAudioInfo.m_iFormatId;
			st_audioinfo.m_ulSamplePerSec = pst_inst->stDmxInfo.m_sAudioInfo.m_iSamplePerSec;
			st_audioinfo.m_ulBitsPerSample = pst_inst->stDmxInfo.m_sAudioInfo.m_iBitsPerSample;
			st_audioinfo.m_ulChannels = pst_inst->stDmxInfo.m_sAudioInfo.m_iChannels;
			st_audioinfo.m_ulFormatTag = pst_inst->stDmxInfo.m_sAudioInfo.m_iFormatId;
			st_audioinfo.m_pbyCodecPrivate = pst_inst->stDmxInfo.m_sAudioInfo.m_pExtraData;
			st_audioinfo.m_ulCodecPrivateSize = pst_inst->stDmxInfo.m_sAudioInfo.m_iExtraDataLen;
			st_audioinfo.m_pszLanguage = NULL;
	
			cdmx_print_info( &st_dmxinfo_list, 
							 0, 
							 sizeof(audio_dmx_info_list_t), 
							 0, 
							 0 );

			/*cdmx_get_stream_id_list( &pst_inst->stCdmxBuff.stStrmIdList,
									 &st_dmxinfo_list, 
									 0, 
									 sizeof(audio_dmx_info_list_t), 
									 0, 
									 0 );
									 */

		}
	}
	else if( ulOpCode == CDMX_GET_INFO )
	{
		cdmx_stream_id_list_t	*pst_out_strmidlist = (cdmx_stream_id_list_t*)pParam1;
		cdmx_info_t				*pst_out_info_param = (cdmx_info_t          *)pParam2;

		STEP( "In CDMX_GET_INFO\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		/////////////////////////////////////////////////////////////////
		//
		// Copy the media info of demuxer into tcc_cdk_warpper
		// 
		// 
		// [*] All Stream list Ids
		//
		if( pst_out_strmidlist )
		{
			pst_out_strmidlist->plVideoIdList 		= pst_inst->stCdmxBuff.stStrmIdList.plVideoIdList;
			pst_out_strmidlist->plAudioIdList 		= pst_inst->stCdmxBuff.stStrmIdList.plAudioIdList;
			pst_out_strmidlist->plSubtitleIdList 	= pst_inst->stCdmxBuff.stStrmIdList.plSubtitleIdList;
			pst_out_strmidlist->plGraphicsIdList 	= pst_inst->stCdmxBuff.stStrmIdList.plGraphicsIdList;
		}

		set_cdmx_info (pst_out_info_param, pst_inst, 
					   SET_CDMX_INFO_FILE|SET_CDMX_INFO_AUDIO);

	}//CDMX_OP_GET_INFO
	else if( ulOpCode == CDMX_GETSTREAM )
	{
		cdmx_input_t	*p_input_param  = (cdmx_input_t *)pParam1;
		cdmx_output_t	*p_output_param = (cdmx_output_t*)pParam2;

		//STEP( "In CDMX_GETSTREAM\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		pst_inst->stDmxGetStream.m_pPacketBuff		= p_input_param->m_pPacketBuff;
		pst_inst->stDmxGetStream.m_iPacketBuffSize	= p_input_param->m_iPacketBuffSize;
		pst_inst->stDmxGetStream.m_iPacketType		= p_input_param->m_iPacketType;

		// for adif
		pst_inst->stDmxOutStream.m_iPacketSize 		= p_output_param->m_iPacketSize;
		pst_inst->stDmxOutStream.m_iDecodedSamples 	= p_output_param->m_iDecodedSamples;

		ret = pst_inst->pfnDmxLib( AUDIO_DMX_GETFRAME, &pst_inst->hDmxHandle, &pst_inst->stDmxGetStream, &pst_inst->stDmxOutStream );
		if( ret < 0 )
		{
			if( ret == ERR_AUDIO_DMX_END_OF_FILE )
				ret = ERR_END_OF_AUDIO_FILE;

			ERROR( "[%ld] Error: AUDIO_DMX_GETFRAME failed (%d)\n", ret, __LINE__ );
			return ret;
		}

		p_output_param->m_pPacketData = pst_inst->stDmxOutStream.m_pPacketData;
		p_output_param->m_iPacketSize = pst_inst->stDmxOutStream.m_iPacketSize;
		p_output_param->m_iPacketType = DMXTYPE_AUDIO;
		p_output_param->m_iTimeStamp  = pst_inst->stDmxOutStream.m_iTimeStamp;
		p_output_param->m_iEndOfFile  = pst_inst->stDmxOutStream.m_iEndOfFile;

	}//CDMX_OP_GETSTREAM
	else if( ulOpCode == CDMX_SEEK )
	{
		cdmx_seek_t		*p_seek_param	= (cdmx_seek_t*)pParam1;
		cdmx_output_t	*p_output_param = (cdmx_output_t*)pParam2;

		STEP( "In CDMX_SEEK\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		pst_inst->stDmxSeek.m_iSeekMode 	= p_seek_param->m_uiSeekMode; 
		pst_inst->stDmxSeek.m_iSeekTimeMSec = p_seek_param->m_iSeekTime;
		pst_inst->stDmxOutStream.m_iEndOfFile = 0;

		ret = pst_inst->pfnDmxLib( AUDIO_DMX_SEEK, &pst_inst->hDmxHandle, &pst_inst->stDmxSeek, &pst_inst->stDmxOutStream );
		if( ret < 0 )
		{
			if( ret == ERR_AUDIO_DMX_END_OF_FILE )
				ret = ERR_END_OF_AUDIO_FILE;

			ERROR( "[%ld] Error: AUDIO_DMX_SEEK failed (%d)\n", ret, __LINE__ );
			return ret;
		}

		if( p_output_param )
		{
			STEP( "CDMX_SEEK Success\n" );
			p_output_param->m_iTimeStamp  = pst_inst->stDmxOutStream.m_iTimeStamp;
		}
	}
	else if( ulOpCode == CDMX_CLOSE )
	{
		STEP( "In CDMX_CLOSE\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		if( pst_inst->pfnDmxLib != NULL )
		{
			ret = pst_inst->pfnDmxLib( AUDIO_DMX_CLOSE, &pst_inst->hDmxHandle, NULL, NULL );
			if( ret < 0 )
			{
				ERROR( "[%ld] Error: AUDIO_DMX_CLOSE failed (%d)\n", ret, __LINE__ );
				return ret;
			}
		}

		if( pst_inst->stCdmxBuff.stStrmIdList.plVideoIdList )
		{
			cdmx_free(pst_inst->stCdmxBuff.stStrmIdList.plVideoIdList);
			pst_inst->stCdmxBuff.stStrmIdList.plVideoIdList = NULL;
		}
		if( pst_inst->stCdmxBuff.stStrmIdList.plAudioIdList )
		{
			cdmx_free(pst_inst->stCdmxBuff.stStrmIdList.plAudioIdList);
			pst_inst->stCdmxBuff.stStrmIdList.plAudioIdList = NULL;
		}
		if( pst_inst->stCdmxBuff.stStrmIdList.plSubtitleIdList )
		{
			cdmx_free(pst_inst->stCdmxBuff.stStrmIdList.plSubtitleIdList);
			pst_inst->stCdmxBuff.stStrmIdList.plSubtitleIdList = NULL;
		}
		if( pst_inst->stCdmxBuff.stStrmIdList.plGraphicsIdList )
		{
			cdmx_free(pst_inst->stCdmxBuff.stStrmIdList.plGraphicsIdList);
			pst_inst->stCdmxBuff.stStrmIdList.plGraphicsIdList = NULL;
		}

		if(pst_inst->stCdmxBuff.pbySeqHeaderBuff)
		{
			cdmx_free(pst_inst->stCdmxBuff.pbySeqHeaderBuff);
			pst_inst->stCdmxBuff.pbySeqHeaderBuff = NULL;
		}

		if(pst_inst->stCdmxBuff.pbyStreamBuff)
		{
			cdmx_free(pst_inst->stCdmxBuff.pbyStreamBuff);
			pst_inst->stCdmxBuff.pbyStreamBuff = NULL;
		}

		if ( pst_inst->pvLibHandle ) 
		{
			ret = close_library(pst_inst->pvLibHandle);
			if ( ret < 0 ) 
			{
				ERROR("closing library failed\n");
			}
			pst_inst->pvLibHandle = NULL;
		}

		pst_inst->hDmxHandle = NULL;
	}
#ifdef TEST_CHECK_MPEG
	else if( ulOpCode == CDMX_MPEG_AUDIO_CHECK )
	{
		audio_dmx_init_t 	*pst_audio_init = (audio_dmx_init_t*)pParam1;
		void				*fp_inp 		= pParam2;
		ret = is_mpeg_audio(pst_audio_init, fp_inp);
	}
	else if( ulOpCode == CDMX_MPEG_AUDIO_INIT )
	{
		int* pi_handle	= (int*)pParam1;
		*pi_handle = check_mpeg_audio_init();
	}
	else if( ulOpCode == CDMX_MPEG_AUDIO_GET_LAYER )
	{
		int				 i_handle		= (int			 )(*ptHandle); 	// input
		int				*pi_layer		= (int			*)pParam1;		// output
		cdmx_output_t	*pst_out_param 	= (cdmx_output_t*)pParam2;		// input
		*pi_layer = check_mpeg_audio_get_layer( i_handle, pst_out_param->m_pPacketData, pst_out_param->m_iPacketSize );
	}
	else if( ulOpCode == CDMX_MPEG_AUDIO_CLOSE )
	{
		int				 i_handle		= (int			 )(*ptHandle); 	// input
		check_mpeg_audio_close(i_handle);
	}
#endif

	return ret;
}

#endif//INCLUDE_AUDIO_DMX
