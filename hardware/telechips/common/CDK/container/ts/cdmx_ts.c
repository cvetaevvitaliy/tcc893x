/****************************************************************************
 *   FileName    : cdmx_ts.c
 *   Description : container demuxer of TS
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

#ifdef INCLUDE_TS_DMX

#include "../../cdk/cdk.h"
#include "../cdmx.h"
#include <cutils/properties.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	LOG
//
#define LOG_TAG    "CDMX_TS"

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

#define cdmx_ts_malloc		((*pst_inst).stDmxInit.m_stMemFuncs).m_pfnMalloc
#define cdmx_ts_free		((*pst_inst).stDmxInit.m_stMemFuncs).m_pfnFree
#define cdmx_ts_memset		((*pst_inst).stDmxInit.m_stMemFuncs).m_pfnMemset
#define cdmx_ts_memcpy		((*pst_inst).stDmxInit.m_stMemFuncs).m_pfnMemcpy
#define cdmx_ts_memmove		((*pst_inst).stDmxInit.m_stMemFuncs).m_pfnMemmove
#define cdmx_ts_realloc		((*pst_inst).stDmxInit.m_stMemFuncs).m_pfnRealloc

#define cdmx_malloc 	cdmx_ts_malloc
#define cdmx_free 		cdmx_ts_free
#define cdmx_memset 	cdmx_ts_memset
#define cdmx_memcpy 	cdmx_ts_memcpy
#define cdmx_memmove	cdmx_ts_memmove
#define cdmx_realloc 	cdmx_ts_realloc

#define SET_CDMX_INFO_FILE 		1
#define SET_CDMX_INFO_VIDEO 	2
#define SET_CDMX_INFO_AUDIO 	4
#define SET_CDMX_INFO_SUBTITLE 	8
#define SET_CDMX_INFO_ALL 		(SET_CDMX_INFO_FILE|SET_CDMX_INFO_VIDEO|SET_CDMX_INFO_AUDIO|SET_CDMX_INFO_SUBTITLE)
static
void 
set_cdmx_info(
    cdmx_info_t				*pstCdmxInfo,
    ts_dmx_inst_t			*pstInst,
	ts_cdmx_video_info_t	*pstVideoInfo,
	ts_cdmx_audio_info_t	*pstAudioInfo,
	ts_cdmx_subtitle_info_t	*pstSubtitleInfo,
	unsigned long 	 	 ulType
	)
{
	ts_dmx_inst_t				*pst_inst    = pstInst;
	cdmx_info_t					*pst_out_info= pstCdmxInfo;
	ts_cdmx_file_info_t			*pst_finfo 	 = pst_inst->stDmxInfo.m_pstFileInfo;
	ts_cdmx_video_info_t		*pst_vinfo 	 = pstVideoInfo;
	ts_cdmx_audio_info_t		*pst_ainfo 	 = pstAudioInfo;
	ts_cdmx_subtitle_info_t		*pst_sinfo	 = pstSubtitleInfo;

	//
	// [1] file info
	//
	if ( ( ulType & SET_CDMX_INFO_FILE ) && ( pst_finfo != NULL ) )
	{
		//FUNC("In set_cdmx_info: SET_CDMX_INFO_FILE \n");
		cdmx_memset( &pst_out_info->m_sFileInfo, 0, sizeof(pst_out_info->m_sFileInfo) );
		pst_out_info->m_sFileInfo.m_pszOpenFileName		= pst_finfo->m_pszFileName;
		pst_out_info->m_sFileInfo.m_pszCopyright		= NULL;
		pst_out_info->m_sFileInfo.m_pszCreationTime		= NULL;
		pst_out_info->m_sFileInfo.m_iRunningtime		= pst_finfo->m_ulDuration;
		pst_out_info->m_sFileInfo.m_lFileSize			= pst_finfo->m_llFileSize;
		pst_out_info->m_sFileInfo.m_bHasIndex			= 0;
		pst_out_info->m_sFileInfo.m_iSuggestedBufferSize= 0;
		pst_out_info->m_sFileInfo.m_iTotalStreams		= 0;
		pst_out_info->m_sFileInfo.m_iTimeScale			= 0;
		pst_out_info->m_sFileInfo.m_lUserDataPos		= 0;
		pst_out_info->m_sFileInfo.m_iUserDataLen		= 0;
		pst_out_info->m_sFileInfo.m_pcTitle				= NULL;				
		pst_out_info->m_sFileInfo.m_pcArtist			= NULL;
		pst_out_info->m_sFileInfo.m_pcAlbum				= NULL;
		pst_out_info->m_sFileInfo.m_pcAlbumArtist		= NULL;
		pst_out_info->m_sFileInfo.m_pcYear				= NULL;
		pst_out_info->m_sFileInfo.m_pcWriter			= NULL;
		pst_out_info->m_sFileInfo.m_pcAlbumArt			= NULL;
		pst_out_info->m_sFileInfo.m_iAlbumArtSize		= 0;
		pst_out_info->m_sFileInfo.m_pcGenre				= NULL;
		pst_out_info->m_sFileInfo.m_pcCompilation		= NULL;
		pst_out_info->m_sFileInfo.m_pcCDTrackNumber		= NULL;
		pst_out_info->m_sFileInfo.m_pcDiscNumber		= NULL;
		pst_out_info->m_sFileInfo.m_pcLocation			= NULL;
		pst_out_info->m_sFileInfo.m_iSeekable			= 1;// TS Supports Scanning Method for seek operation even if there's no index.
		pst_out_info->m_sFileInfo.m_ulSecureType		= pst_finfo->m_ulSecureType;
		pst_out_info->m_sFileInfo.m_iFragmentedMp4		= 0;
	}
	// 
	// [2] video info
	//
	if ( ( ulType & SET_CDMX_INFO_VIDEO ) && ( pst_vinfo != NULL ) )
	{
		//FUNC("In set_cdmx_info: SET_CDMX_INFO_VIDEO \n");
		cdmx_memset( &pst_out_info->m_sVideoInfo, 0, sizeof(pst_out_info->m_sVideoInfo) );
		pst_out_info->m_sVideoInfo.m_iWidth          		= pst_vinfo->m_ulWidth;
		pst_out_info->m_sVideoInfo.m_iHeight         		= pst_vinfo->m_ulHeight;
		pst_out_info->m_sVideoInfo.m_iFrameRate      		= pst_vinfo->m_ulFrameRate;
		pst_out_info->m_sVideoInfo.m_iFourCC         		= pst_vinfo->m_ulFourCC;
		if( pst_out_info->m_sVideoInfo.m_iFourCC == FOURCC_H264 && pst_vinfo->m_bIsStereo )
			pst_out_info->m_sVideoInfo.m_iFourCC = FOURCC_MVC;
		pst_out_info->m_sVideoInfo.m_ulmax_bit_rate 		= 0;
		pst_out_info->m_sVideoInfo.m_ulavg_bit_rate 		= 0;
		pst_out_info->m_sVideoInfo.m_pszCodecName 			= NULL;
		pst_out_info->m_sVideoInfo.m_pszCodecVendorName 	= NULL;
		pst_out_info->m_sVideoInfo.m_pExtraData      		= NULL;
		pst_out_info->m_sVideoInfo.m_iExtraDataLen   		= 0;
		pst_out_info->m_sVideoInfo.m_iNumVideoStream 		= pst_inst->stDmxInfo.m_ulVideoStreamTotal;
		pst_out_info->m_sVideoInfo.m_iCurrVideoIdx   		= pst_vinfo->m_ulStreamID;
		pst_out_info->m_sVideoInfo.m_iBitsPerSample 		= 0;
		pst_out_info->m_sVideoInfo.m_iTotalNumber 			= 0;
		pst_out_info->m_sVideoInfo.m_iKeyFrameNumber 		= 0;
		pst_out_info->m_sVideoInfo.m_bAvcC 					= 0;
		pst_out_info->m_sVideoInfo.m_iTrackTimeScale 		= 0;
		pst_out_info->m_sVideoInfo.m_iLastKeyTime 			= 0;
		pst_out_info->m_sVideoInfo.m_iRotateDegree 			= 0;
		pst_out_info->m_sVideoInfo.m_iVideoStreamID 		= 0;
		pst_out_info->m_sVideoInfo.m_iHasPayloadAspectRatio = 0;
		pst_out_info->m_sVideoInfo.m_iAspectRatioX 			= 0;
		pst_out_info->m_sVideoInfo.m_iAspectRatioY 			= 0;
		pst_out_info->m_sVideoInfo.m_lAvgBitRate 			= 0;
		pst_out_info->m_sVideoInfo.m_iAspectRatio 			= 0;
		pst_out_info->m_sVideoInfo.m_iBitRate				= 0;
		pst_out_info->m_sVideoInfo.m_ulLength 				= 0;
		pst_out_info->m_sVideoInfo.m_ulEXTV1_type1 			= 0;
		pst_out_info->m_sVideoInfo.m_ulEXTV1_type2 			= 0;
		pst_out_info->m_sVideoInfo.m_usEXTV1Cnt 			= 0;
		pst_out_info->m_sVideoInfo.m_usPadWidth 			= 0;
		pst_out_info->m_sVideoInfo.m_usPadHeight 			= 0;
		pst_out_info->m_sVideoInfo.m_sdummy 				= 0;
		pst_out_info->m_sVideoInfo.m_ulFramesPerSecond 		= 0;
		pst_out_info->m_sVideoInfo.m_ulEXTV1Size 			= 0;
		pst_out_info->m_sVideoInfo.m_ulEXTV1Data 			= 0;
		pst_out_info->m_sVideoInfo.m_ulLastKeyTimestamp  	= 0;
		pst_out_info->m_sVideoInfo.m_ulStereoMode       	= 0;
	}
	// 
	// [3] audio info
	//
	if ( ( ulType & SET_CDMX_INFO_AUDIO ) && ( pst_ainfo != NULL ) )
	{
		//FUNC("In set_cdmx_info: SET_CDMX_INFO_AUDIO \n");
		cdmx_memset( &pst_out_info->m_sAudioInfo, 0, sizeof(pst_out_info->m_sAudioInfo) );
		pst_out_info->m_sAudioInfo.m_iTotalNumber		= 0;
		pst_out_info->m_sAudioInfo.m_iSamplePerSec		= pst_ainfo->m_ulSamplePerSec;
		pst_out_info->m_sAudioInfo.m_iBitsPerSample		= pst_ainfo->m_ulBitsPerSample;
		pst_out_info->m_sAudioInfo.m_iChannels			= pst_ainfo->m_ulChannels;
		pst_out_info->m_sAudioInfo.m_iFormatId			= pst_ainfo->m_ulFormatTag;
		pst_out_info->m_sAudioInfo.m_ulmax_bit_rate		= 0;
		pst_out_info->m_sAudioInfo.m_ulavg_bit_rate		= 0;
		pst_out_info->m_sAudioInfo.m_pszCodecName		= NULL;
		pst_out_info->m_sAudioInfo.m_pszCodecVendorName	= NULL;
		pst_out_info->m_sAudioInfo.m_pExtraData			= pst_ainfo->m_pbyCodecPrivate;
		pst_out_info->m_sAudioInfo.m_iExtraDataLen		= pst_ainfo->m_ulCodecPrivateSize;
		pst_out_info->m_sAudioInfo.m_pszlanguageInfo	= pst_ainfo->m_pszLanguage;
		pst_out_info->m_sAudioInfo.m_iFramesPerSample	= 0;
		pst_out_info->m_sAudioInfo.m_iTrackTimeScale	= 0;
		pst_out_info->m_sAudioInfo.m_iSamplesPerPacket	= 0;
		pst_out_info->m_sAudioInfo.m_iNumAudioStream	= pst_inst->stDmxInfo.m_ulAudioStreamTotal;
		pst_out_info->m_sAudioInfo.m_iCurrAudioIdx		= pst_ainfo->m_ulStreamID;
		pst_out_info->m_sAudioInfo.m_iBlockAlign		= 0;
		pst_out_info->m_sAudioInfo.m_iAvgBytesPerSec	= 0;
		pst_out_info->m_sAudioInfo.m_iAudioStreamID		= 0;
		pst_out_info->m_sAudioInfo.m_iBitRate			= pst_ainfo->m_ulBitRate;
		pst_out_info->m_sAudioInfo.m_iActualRate		= 0;
		pst_out_info->m_sAudioInfo.m_sAudioQuality		= 0;
		pst_out_info->m_sAudioInfo.m_sFlavorIndex		= 0;
		pst_out_info->m_sAudioInfo.m_iBitsPerFrame		= 0;
		pst_out_info->m_sAudioInfo.m_iGranularity		= 0;
		pst_out_info->m_sAudioInfo.m_iOpaqueDataSize	= 0;
		pst_out_info->m_sAudioInfo.m_pOpaqueData		= 0;
		pst_out_info->m_sAudioInfo.m_iSamplesPerFrame	= 0;
		pst_out_info->m_sAudioInfo.m_iframeSizeInBits	= 0;
		pst_out_info->m_sAudioInfo.m_iSampleRate		= 0;
		pst_out_info->m_sAudioInfo.m_scplStart			= 0;
		pst_out_info->m_sAudioInfo.m_scplQBits			= 0;
		pst_out_info->m_sAudioInfo.m_snRegions			= 0;
		pst_out_info->m_sAudioInfo.m_sdummy				= 0;
		pst_out_info->m_sAudioInfo.m_ulSubType			= pst_ainfo->m_ulSubType;
		pst_out_info->m_sAudioInfo.m_iMinDataSize		= 0;
	}
	// 
	// [4] subtitle info
	//
	if ( ( ulType & SET_CDMX_INFO_SUBTITLE ) && ( pst_sinfo != NULL ) )
	{
		//FUNC("In set_cdmx_info: SET_CDMX_INFO_SUBTITLE \n");
		cdmx_memset( &pst_out_info->m_sSubtitleInfo, 0, sizeof(pst_out_info->m_sSubtitleInfo) );
		pst_out_info->m_sSubtitleInfo.m_iIsText				= 0;
		pst_out_info->m_sSubtitleInfo.m_iNumSubtitleStream	= pst_inst->stDmxInfo.m_ulSubtitleStreamTotal;
		pst_out_info->m_sSubtitleInfo.m_iCurrSubtitleIdx	= pst_sinfo->m_ulStreamID;
		pst_out_info->m_sSubtitleInfo.m_pszLanguage			= pst_sinfo->m_pszLanguage;
		pst_out_info->m_sSubtitleInfo.m_iPID 				= 0;
		pst_out_info->m_sSubtitleInfo.m_iCharEnc 			= 0;
		pst_out_info->m_sSubtitleInfo.m_iGraphicsType 		= pst_sinfo->m_ulSubtitleType;
		pst_out_info->m_sSubtitleInfo.m_iTotalNumber 		= 0;
		pst_out_info->m_sSubtitleInfo.m_iTrackIndexCache 	= 0;
		pst_out_info->m_sSubtitleInfo.m_pInfo				= NULL;
		pst_out_info->m_sSubtitleInfo.m_iInfoSize			= 0;
	}

}

static
unsigned long
get_total_num_es(
    ts_dmx_inst_t  	*pstInst,
    tsd_info_t		*pstTsdInfo,
	unsigned long 	 ulESType
	)
{
	unsigned long total_num = 0;
	int i, j;

#ifdef SUPPORT_MULTI_PROG

	ts_dmx_inst_t *pst_inst = pstInst;
	unsigned long total_nb_es = 0;
	unsigned long *pul_es_ids = NULL;

	for (i = 0; i < pstTsdInfo->m_lProgramTotal; i++) {
		total_nb_es += pstTsdInfo->m_pstProgramInfoList[i].m_ulNbES;
	}
	pul_es_ids = (unsigned long*)cdmx_malloc((unsigned long)*total_nb_es);
	cdmx_memset(pul_es_ids, 0, (unsigned long)*total_nb_es);

	for (i = 0; i < pstTsdInfo->m_lProgramTotal; i++) {
		for (j = 0; j < pstTsdInfo->m_pstProgramInfoList[i].m_ulNbES; j++) {
			if (pstTsdInfo->m_pstProgramInfoList[i].m_pstEsInfoList[j].m_ulEsType == ulESType) {
				pul_es_ids[total_num] = pstTsdInfo->m_pstProgramInfoList[i].m_pstEsInfoList[j].m_ulElementPID;
				total_num++;
			}
		}
		break;
	}

	if (total_num > 1) {
		i = total_num;
		long curr_id = 0;
		long prev_id = -1;

		while (i > 0) {
			i--;
			curr_id = pul_es_ids[i];
			if (curr_id == prev_id) {
				total_num--;
			}
			prev_id = curr_id;
		}
	}
	cdmx_free(pul_es_ids); 

#else

	for(i = 0; i < pstTsdInfo->m_lProgramTotal; i++) {
		if( pstTsdInfo->m_pstProgramInfoList[i].m_ulProgramNum == pstTsdInfo->m_stDefProgInfo.m_ulProgramNum ) {
			for(j = 0; j < pstTsdInfo->m_pstProgramInfoList[i].m_ulNbES; j++) {
				if(pstTsdInfo->m_pstProgramInfoList[i].m_pstEsInfoList[j].m_ulEsType == ulESType)
					total_num++;
			}
			break;
		}
	}

#endif

	return total_num;
}

static
long
ts_set_stream_info ( 
    ts_dmx_inst_t *pstInst
	)
{
	ts_dmx_inst_t  	*pst_inst 		= pstInst;
	ts_cdmx_info_t	*pst_dmx_info 	= &pst_inst->stDmxInfo;
	tsd_info_t  	*pst_tsd_info 	= &pst_inst->stTsdInfo;

	//
	// file info
	//
	if ( pst_dmx_info->m_pstFileInfo == NULL )
	{
		pst_dmx_info->m_pstFileInfo = cdmx_malloc(sizeof(ts_cdmx_file_info_t));
		if ( pst_dmx_info->m_pstFileInfo == NULL ) {
			ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
			return CDK_ERR_MALLOC;
		}
		DSTATUS("[Line:%04d] pstStreamIdList->plVideoIdList: 0x%x\n", __LINE__, pst_dmx_info->m_pstFileInfo);
		cdmx_memset( pst_dmx_info->m_pstFileInfo, 0, sizeof(ts_cdmx_file_info_t) );
	}
	pst_dmx_info->m_pstFileInfo->m_pszFileName 	= pst_inst->stDmxInit.m_pszOpenFileName;
	pst_dmx_info->m_pstFileInfo->m_ulDuration	= pst_tsd_info->m_stDefProgInfo.m_lDuration;
	//pst_dmx_info->m_pstFileInfo->m_llFileSize
	pst_dmx_info->m_pstFileInfo->m_ulSecureType = pst_tsd_info->m_stDefProgInfo.m_ulSecureType;
	pst_dmx_info->m_pstFileInfo->m_ulBuffChangeStatus = pst_tsd_info->m_stDefProgInfo.m_ulBuffChangeStatus;


	//
	// video info
	//
	pst_inst->stDmxInfo.m_ulVideoStreamTotal = get_total_num_es(pst_inst, pst_tsd_info, ES_TYPE_VIDEO);
	DSTATUS("pst_inst->stDmxInfo.m_ulVideoStreamTotal: %d\n", pst_inst->stDmxInfo.m_ulVideoStreamTotal);
	if( pst_inst->stDmxInfo.m_ulVideoStreamTotal > 1 )
	{
		DSTATUS("VideoStreamTotal: 1\n");
		pst_inst->stDmxInfo.m_ulVideoStreamTotal = 1;
	}

	if( pst_inst->stDmxInfo.m_ulVideoStreamTotal > 0 )
	{
		long i, j;
		long index = 0;
		long prog_total = 0;

		if ( pst_dmx_info->m_pstVideoInfoList == NULL )
		{
			pst_dmx_info->m_pstVideoInfoList = cdmx_malloc(sizeof(ts_cdmx_video_info_t) * pst_inst->stDmxInfo.m_ulVideoStreamTotal);
			if ( pst_dmx_info->m_pstVideoInfoList == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] pst_dmx_info->m_pstVideoInfoList: 0x%x\n", __LINE__, pst_dmx_info->m_pstVideoInfoList);
			cdmx_memset( pst_dmx_info->m_pstVideoInfoList, 0, sizeof(ts_cdmx_video_info_t) * pst_inst->stDmxInfo.m_ulVideoStreamTotal );
		}

		// cannot support multi-program
		//prog_total = pst_tsd_info->m_lProgramTotal
		prog_total = 1;// support only one program

		for(i = 0; i < prog_total; i++) 
		{
			for(j = 0; j < pst_tsd_info->m_pstProgramInfoList[i].m_ulNbES; j++) 
			{
				tsd_esinfo_t *pst_esinfo = &pst_tsd_info->m_pstProgramInfoList[i].m_pstEsInfoList[j];
				if(pst_esinfo->m_ulEsType == ES_TYPE_VIDEO)
				{
					tsd_videoinfo_t			*pst_inp_info = &pst_esinfo->m_unInfo.stVideo;
					ts_cdmx_video_info_t 	*pst_out_info = &pst_dmx_info->m_pstVideoInfoList[index];

					pst_out_info->m_ulStreamID  		= pst_inp_info->m_ulEsPID;
					pst_out_info->m_ulWidth				= pst_inp_info->m_lWidth;
					pst_out_info->m_ulHeight			= pst_inp_info->m_lHeight;
					pst_out_info->m_ulFrameRate 		= pst_inp_info->m_lFrameRate;
					pst_out_info->m_ulFourCC			= pst_inp_info->m_lFourCC;
					pst_out_info->m_pbyCodecPrivate 	= NULL;
					pst_out_info->m_ulCodecPrivateSize	= 0;
					pst_out_info->m_bIsStereo			= pst_inp_info->m_bIsStereo;

					DSTATUS( " [Video_%03ld] - StreamID: %ld / %ldx%ld pix / %.3f fps / FourCC: \"%c%c%c%c\" \n",
								i,
								pst_out_info->m_ulStreamID,
								pst_out_info->m_ulWidth,
								pst_out_info->m_ulHeight,
								(float)pst_out_info->m_ulFrameRate/1000,
								(char)(pst_out_info->m_ulFourCC >> 0 ),
								(char)(pst_out_info->m_ulFourCC >> 8 ),
								(char)(pst_out_info->m_ulFourCC >> 16),
								(char)(pst_out_info->m_ulFourCC >> 24)
							);
					index++;
				}
			}
		}
		// set default 
		pst_dmx_info->m_pstDefaultVideoInfo = &pst_dmx_info->m_pstVideoInfoList[0];
	}
	//
	// audio info
	//
	pst_inst->stDmxInfo.m_ulAudioStreamTotal = get_total_num_es(pst_inst, pst_tsd_info, ES_TYPE_AUDIO);
	DSTATUS("pst_inst->stDmxInfo.m_ulAudioStreamTotal: %d\n", pst_inst->stDmxInfo.m_ulAudioStreamTotal);

	if( pst_inst->stDmxInfo.m_ulAudioStreamTotal > 0 )
	{
		long i, j;
		long index = 0;
		long prog_total = 0;

		if( pst_dmx_info->m_pstAudioInfoList == NULL )
		{
			pst_dmx_info->m_pstAudioInfoList = cdmx_malloc(sizeof(ts_cdmx_audio_info_t) * pst_inst->stDmxInfo.m_ulAudioStreamTotal);
			if ( pst_dmx_info->m_pstAudioInfoList == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] pst_dmx_info->m_pstAudioInfoList: 0x%x\n", __LINE__, pst_dmx_info->m_pstAudioInfoList);
			cdmx_memset( pst_dmx_info->m_pstAudioInfoList, 0, sizeof(ts_cdmx_audio_info_t) * pst_inst->stDmxInfo.m_ulAudioStreamTotal );
		}

		// cannot support multi-program
		//prog_total = pst_tsd_info->m_lProgramTotal
		prog_total = 1;// support only one program

		for(i = 0; i < prog_total; i++) 
		{
			for(j = 0; j < pst_tsd_info->m_pstProgramInfoList[i].m_ulNbES; j++) 
			{
				tsd_esinfo_t *pst_esinfo = &pst_tsd_info->m_pstProgramInfoList[i].m_pstEsInfoList[j];
				if(pst_esinfo->m_ulEsType == ES_TYPE_AUDIO)
				{
					tsd_audioinfo_t			*pst_inp_info = &pst_esinfo->m_unInfo.stAudio;
					ts_cdmx_audio_info_t 	*pst_out_info = &pst_dmx_info->m_pstAudioInfoList[index];

					pst_out_info->m_ulStreamID			= pst_inp_info->m_ulEsPID;
					pst_out_info->m_ulSamplePerSec		= pst_inp_info->m_lSamplePerSec;
					pst_out_info->m_ulBitsPerSample		= pst_inp_info->m_lBitPerSample;
					pst_out_info->m_ulChannels			= pst_inp_info->m_lChannels;
					pst_out_info->m_ulFormatTag			= pst_inp_info->m_ulFormatTag;
					pst_out_info->m_pbyCodecPrivate		= NULL;
					pst_out_info->m_ulCodecPrivateSize	= 0;
					pst_out_info->m_pszLanguage			= pst_inp_info->m_szLanguageCode;
					pst_out_info->m_ulBitRate			= pst_inp_info->m_lBitRate;
					pst_out_info->m_ulSubType			= pst_inp_info->m_ulSubType;

					DSTATUS( " [Audio_%03ld] - StreamID: %ld / %ld Hz / %ld bits / %ld Ch / FormatTag: 0x%04X / Language: \"%s\" \n",
								i,
								pst_out_info->m_ulStreamID,
								pst_out_info->m_ulSamplePerSec,
								pst_out_info->m_ulBitsPerSample,
								pst_out_info->m_ulChannels,
								(unsigned int)pst_out_info->m_ulFormatTag,
								pst_out_info->m_pszLanguage);
					index++;
				}
			}
		}
		// set default 
		pst_dmx_info->m_pstDefaultAudioInfo = &pst_dmx_info->m_pstAudioInfoList[0];
	}
	//
	// subtitle info
	//
	pst_inst->stDmxInfo.m_ulSubtitleStreamTotal = get_total_num_es(pst_inst, pst_tsd_info, ES_TYPE_GRAPHICS);
	DSTATUS("pst_inst->stDmxInfo.m_ulSubtitleStreamTotal: %d\n", pst_inst->stDmxInfo.m_ulSubtitleStreamTotal);

	if( pst_inst->stDmxInfo.m_ulSubtitleStreamTotal > 0 )
	{
		long i, j;
		long index = 0;
		long prog_total = 0;

		if( pst_dmx_info->m_pstSubtitleInfoList == NULL )
		{
			pst_dmx_info->m_pstSubtitleInfoList = cdmx_malloc(sizeof(ts_cdmx_subtitle_info_t) * pst_inst->stDmxInfo.m_ulSubtitleStreamTotal);
			if ( pst_dmx_info->m_pstSubtitleInfoList == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] pst_dmx_info->m_pstSubtitleInfoList: 0x%x\n", __LINE__, pst_dmx_info->m_pstSubtitleInfoList);
			cdmx_memset( pst_dmx_info->m_pstSubtitleInfoList, 0, sizeof(ts_cdmx_subtitle_info_t) * pst_inst->stDmxInfo.m_ulSubtitleStreamTotal );
		}

		// cannot support multi-program
		//prog_total = pst_tsd_info->m_lProgramTotal
		prog_total = 1;// support only one program

		for(i = 0; i < prog_total; i++) 
		{
			for(j = 0; j < pst_tsd_info->m_pstProgramInfoList[i].m_ulNbES; j++) 
			{
				tsd_esinfo_t *pst_esinfo = &pst_tsd_info->m_pstProgramInfoList[i].m_pstEsInfoList[j];
				if(pst_esinfo->m_ulEsType == ES_TYPE_GRAPHICS)
				{
					tsd_graphicsinfo_t		*pst_inp_info = &pst_esinfo->m_unInfo.stGraphics;
					ts_cdmx_subtitle_info_t	*pst_out_info = &pst_dmx_info->m_pstSubtitleInfoList[index];

					pst_out_info->m_ulStreamID		= pst_inp_info->m_ulEsPID;
					pst_out_info->m_pszLanguage		= pst_inp_info->m_szLanguageCode;
					pst_out_info->m_ulSubtitleType	= pst_inp_info->m_ulGraphicsType;

					DSTATUS( " [Subtitle_%03ld] - StreamID: %ld / Language: \"%s\" \n",
								i,
								pst_out_info->m_ulStreamID,
								pst_out_info->m_pszLanguage);
					index++;
				}
			}
		}
		// set default 
		pst_dmx_info->m_pstDefaultSubtitleInfo = &pst_dmx_info->m_pstSubtitleInfoList[0];
	}

	return 0;
}

static
long
ts_set_stream_info_sel_stream ( 
    ts_dmx_inst_t 				*pstInst,
	tsd_videoinfo_t				*pstTsdVidInfo,
	tsd_audioinfo_t				*pstTsdAudInfo,
	tsd_graphicsinfo_t			*pstTsdGraInfo,
	ts_cdmx_video_info_t 		*pstOutVideoInfo,
	ts_cdmx_audio_info_t 		*pstOutAudioInfo,
	ts_cdmx_subtitle_info_t 	*pstOutSubtitleInfo
	)
{
	//
	// video info
	//
	if( pstTsdVidInfo && pstOutVideoInfo )
	{
		tsd_videoinfo_t 		*pst_inp_info = pstTsdVidInfo;
		ts_cdmx_video_info_t 	*pst_out_info = pstOutVideoInfo;

		pst_out_info->m_ulStreamID  		= pst_inp_info->m_ulEsPID;
		pst_out_info->m_ulWidth				= pst_inp_info->m_lWidth;
		pst_out_info->m_ulHeight			= pst_inp_info->m_lHeight;
		pst_out_info->m_ulFrameRate 		= pst_inp_info->m_lFrameRate;
		pst_out_info->m_ulFourCC			= pst_inp_info->m_lFourCC;
		pst_out_info->m_pbyCodecPrivate 	= NULL;
		pst_out_info->m_ulCodecPrivateSize	= 0;
		pst_out_info->m_bIsStereo				= pst_inp_info->m_bIsStereo;

		DSTATUS( " [Video] - StreamID: %ld / %ldx%ld pix / %.3f fps / FourCC: \"%c%c%c%c\" \n",
					pst_out_info->m_ulStreamID,
					pst_out_info->m_ulWidth,
					pst_out_info->m_ulHeight,
					(float)pst_out_info->m_ulFrameRate/1000,
					(char)(pst_out_info->m_ulFourCC >> 0 ),
					(char)(pst_out_info->m_ulFourCC >> 8 ),
					(char)(pst_out_info->m_ulFourCC >> 16),
					(char)(pst_out_info->m_ulFourCC >> 24) );
	}

	//
	// audio info
	//
	if( pstTsdAudInfo && pstOutAudioInfo )
	{
		tsd_audioinfo_t 		*pst_inp_info = pstTsdAudInfo;
		ts_cdmx_audio_info_t 	*pst_out_info = pstOutAudioInfo;

		pst_out_info->m_ulStreamID			= pst_inp_info->m_ulEsPID;
		pst_out_info->m_ulSamplePerSec		= pst_inp_info->m_lSamplePerSec;
		pst_out_info->m_ulBitsPerSample		= pst_inp_info->m_lBitPerSample;
		pst_out_info->m_ulChannels			= pst_inp_info->m_lChannels;
		pst_out_info->m_ulFormatTag			= pst_inp_info->m_ulFormatTag;
		pst_out_info->m_pbyCodecPrivate		= NULL;
		pst_out_info->m_ulCodecPrivateSize	= 0;
		pst_out_info->m_pszLanguage			= pst_inp_info->m_szLanguageCode;
		pst_out_info->m_ulBitRate			= pst_inp_info->m_lBitRate;
		pst_out_info->m_ulSubType			= pst_inp_info->m_ulSubType;

		DSTATUS( " [Audio] - StreamID: %ld / %ld Hz / %ld bits / %ld Ch / FormatTag: 0x%04X / Language: \"%s\" \n",
					pst_out_info->m_ulStreamID,
					pst_out_info->m_ulSamplePerSec,
					pst_out_info->m_ulBitsPerSample,
					pst_out_info->m_ulChannels,
					(unsigned int)pst_out_info->m_ulFormatTag,
					pst_out_info->m_pszLanguage);
	}

	//
	// subtitle info
	//
	if( pstTsdGraInfo && pstOutSubtitleInfo )
	{
		tsd_graphicsinfo_t 		*pst_inp_info = pstTsdGraInfo;
		ts_cdmx_subtitle_info_t *pst_out_info = pstOutSubtitleInfo;

		pst_out_info->m_ulStreamID		= pst_inp_info->m_ulEsPID;
		pst_out_info->m_pszLanguage		= pst_inp_info->m_szLanguageCode;
		pst_out_info->m_ulSubtitleType	= pst_inp_info->m_ulGraphicsType;

		DSTATUS( " [Subtitle] - StreamID: %ld / Language: \"%s\" \n",
					pst_out_info->m_ulStreamID,
					pst_out_info->m_pszLanguage);
	}

	return 0;
}

static
int
find_sequence_header(
	unsigned char   *pbyDataBuff, 
	long             lDataLength, 
	unsigned long    ulFourCC
)
{
	unsigned long syncword = 0xFFFFFFFF;
	unsigned char *p_buff_end = pbyDataBuff + lDataLength;

	DPRINTF("find sequence header...");

	if( lDataLength < 4 )
		return 0;

	syncword <<= 8; syncword |= *pbyDataBuff++;
	syncword <<= 8; syncword |= *pbyDataBuff++;
	syncword <<= 8; syncword |= *pbyDataBuff++;

	switch( ulFourCC ) 
	{
	case FOURCC_mpeg:
		while( pbyDataBuff < p_buff_end ) {
			syncword <<= 8;
			syncword |= *pbyDataBuff++;
			if( syncword == 0x000001B3 ) { //sequence start code
				DPRINTF("sequence header is found! %d", lDataLength + p_buff_end - pbyDataBuff);
				return 1;
			}
		}
		break;

	case FOURCC_MP4V:
		p_buff_end -= 4;
		while( pbyDataBuff < p_buff_end ) {
			syncword <<= 8;
			syncword |= *pbyDataBuff++;
			if( (syncword >> 8) == 0x000001 && (syncword & 0xFF) <= 0x1F )
			{
				syncword <<= 8; syncword |= *pbyDataBuff++;
				syncword <<= 8; syncword |= *pbyDataBuff++;
				syncword <<= 8; syncword |= *pbyDataBuff++;
				syncword <<= 8; syncword |= *pbyDataBuff++;

				if( syncword >= 0x00000120 &&  // video_object_layer_start_code MIN
					syncword <= 0x0000012F )   // video_object_layer_start_code MAX
				{
					DPRINTF("sequence header is found! %d", lDataLength + p_buff_end - pbyDataBuff);
					return 1;
				}
											   
				if( (syncword >> 10) == 32 ) { // short_video_start_marker
					DPRINTF("sequence header is found! %d", lDataLength + p_buff_end - pbyDataBuff);
					return 1;
				}
			}
		}
		break;

	case FOURCC_H264:
		while( pbyDataBuff < p_buff_end ) {
			syncword <<= 8;
			syncword |= *pbyDataBuff++;
			if( (syncword & 0xFFFFFF1F) == 0x00000107 ) { //SPS is found
				DPRINTF("sequence header is found! %d", lDataLength + p_buff_end - pbyDataBuff);
				return 1;
			}
		}
		break;

	case FOURCC_wvc1:
		while( pbyDataBuff < p_buff_end ) {
			syncword <<= 8;
			syncword |= *pbyDataBuff++;
			if( syncword == 0x0000010F ) { //sequence start code
				DPRINTF("sequence header is found! %d", lDataLength + p_buff_end - pbyDataBuff);
				return 1;
			}
		}
		break;

	case FOURCC_AVS:
		while( pbyDataBuff < p_buff_end ) {
			syncword <<= 8;
			syncword |= *pbyDataBuff++;
			if( syncword == 0x000001B0 ) { //sequence start code
				DPRINTF("sequence header is found! %d", lDataLength + p_buff_end - pbyDataBuff);
				return 1;
			}
		}
		break;

	default:
		DPRINTF("FourCC %c%c%c%c is not supported", 
				(char)((ulFourCC      ) & 0xFF), 
				(char)((ulFourCC >> 8 ) & 0xFF), 
				(char)((ulFourCC >> 16) & 0xFF), 
				(char)((ulFourCC >> 24) & 0xFF)
				);
		return 1;
	}

	return 0;
}

av_result_t 
cdmx_ts( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 )
{
	av_result_t ret = DMXRET_SUCCESS;
	ts_dmx_inst_t* pst_inst = (ts_dmx_inst_t*)(*ptHandle); // assume that its memory is allocated.

	if( ulOpCode == CDMX_PRE_INIT )
	{
		STEP( "In CDMX_PRE_INIT\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		cdmx_pre_init_t *pst_pre_init = (cdmx_pre_init_t *)pParam1;
		if( pst_pre_init )
		{
			tsdext_config_scan_endpts_t* pst_conf_scan_endpts = &pst_inst->stConfScanEndpts;

			STEP("In CDMX_PRE_INIT: %d %d %d", pst_pre_init->m_lParam[0], pst_pre_init->m_lParam[1], pst_pre_init->m_lParam[2]);

			// use user defined init. check option 
			pst_conf_scan_endpts->ulExtTag = TSDEXT_TAG_CONFIG_SCAN_ENDPTS;
			pst_conf_scan_endpts->lExtBytes = sizeof(tsdext_config_scan_endpts_t);
			pst_conf_scan_endpts->pNextField = 0;

			pst_conf_scan_endpts->llStartOffset = (tsd_s64_t)(pst_pre_init->m_lParam[0] * 1024);
			pst_conf_scan_endpts->llMaxOffset   = (tsd_s64_t)(pst_pre_init->m_lParam[1] * 1024);
			pst_conf_scan_endpts->llStepSize    = (tsd_s64_t)(pst_pre_init->m_lParam[2] * 1024);

			DSTATUS("CDMX_PRE_INIT: llStartOffset = %lld, llMaxOffset = %lld, llStepSize = %lld", 
					pst_conf_scan_endpts->llStartOffset, pst_conf_scan_endpts->llMaxOffset, pst_conf_scan_endpts->llStepSize);

			pst_inst->stCdmxCtrl.bPreInitDone = 1;
		}
	}
	else if( ulOpCode == CDMX_INIT )
	{
		cdmx_init_t				*pst_init_param = (cdmx_init_t*)pParam1;
		cdmx_callback_func_t	*pst_callback   = (cdmx_callback_func_t*)pst_init_param->pstCallback;
		char value[PROPERTY_VALUE_MAX];

		STEP( "In CDMX_INIT\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		{
		#ifdef CDMX_USE_DYNAMIC_LOADING
			void* pv_dmx;
			void* pv_handle;
			ret = open_library( (const char*)"/system/lib/libff5.so",
								(const char*)"TCC_TS_DMX",
								&pv_dmx,
								&pv_handle );
			if (ret < 0) {
				return ret;
			}
			pst_inst->pfnDmxLib = ( av_result_t (*) (unsigned long, av_handle_t*, void*, void*) )(pv_dmx);
			pst_inst->pvLibHandle = pv_handle;
			STEP( "LibHandle loaded....(0x%x) (handle:0x%x) ...\n", pst_inst->pfnDmxLib, pst_inst->pvLibHandle );
		#else
			pst_inst->pfnDmxLib   = TCC_TS_DMX;
			pst_inst->pvLibHandle = NULL;
		#endif
		}

		pst_callback->m_pfMemset( &pst_inst->stDmxInit, 0, sizeof(pst_inst->stDmxInit) );
		pst_callback->m_pfMemset( &pst_inst->stTsdInfo, 0, sizeof(pst_inst->stTsdInfo) );

		pst_inst->stCdmxCtrl.ulParsingMode = pst_init_param->ulParsingMode;
		if( pst_init_param->ulOptions & CDMXINIT_SPDIF_MULTIMODE )
			pst_inst->stCdmxCtrl.lSpdifMultiMode = 1;
		//--------------------------------------------------
		// set init parameters
		//--------------------------------------------------
		pst_inst->stDmxInit.m_pszOpenFileName = pst_init_param->pszFileName;

		pst_inst->stDmxInit.m_stMemFuncs.m_pfnMalloc		= pst_callback->m_pfMalloc;
		pst_inst->stDmxInit.m_stMemFuncs.m_pfnFree			= pst_callback->m_pfFree;
		pst_inst->stDmxInit.m_stMemFuncs.m_pfnMemcmp		= pst_callback->m_pfMemcmp;
		pst_inst->stDmxInit.m_stMemFuncs.m_pfnMemcpy		= pst_callback->m_pfMemcpy;
		pst_inst->stDmxInit.m_stMemFuncs.m_pfnMemset		= pst_callback->m_pfMemset;
		pst_inst->stDmxInit.m_stMemFuncs.m_pfnRealloc		= pst_callback->m_pfRealloc;
		pst_inst->stDmxInit.m_stMemFuncs.m_pfnMemmove		= pst_callback->m_pfMemmove;

		pst_inst->stDmxInit.m_stFileFuncs.m_pfnFopen		= pst_callback->m_pfFopen;
		pst_inst->stDmxInit.m_stFileFuncs.m_pfnFread		= pst_callback->m_pfFread;
		pst_inst->stDmxInit.m_stFileFuncs.m_pfnFseek		= pst_callback->m_pfFseek;
		pst_inst->stDmxInit.m_stFileFuncs.m_pfnFtell		= pst_callback->m_pfFtell;
		pst_inst->stDmxInit.m_stFileFuncs.m_pfnFclose		= pst_callback->m_pfFclose;
		pst_inst->stDmxInit.m_stFileFuncs.m_pfnFeof			= pst_callback->m_pfFeof;
		pst_inst->stDmxInit.m_stFileFuncs.m_pfnFflush		= pst_callback->m_pfFflush;
		pst_inst->stDmxInit.m_stFileFuncs.m_pfnFseek64		= pst_callback->m_pfFseek64;
		pst_inst->stDmxInit.m_stFileFuncs.m_pfnFtell64		= pst_callback->m_pfFtell64;
		cdmx_memory_func_init(&pst_inst->stDmxInit.m_stMemFuncs);

		pst_inst->stDmxInit.m_ulSrcType = SOURCE_TYPE_FILE;
		pst_inst->stDmxInit.m_ulTsType = TS_FORMAT_UNKNOWN;

		DSTATUS("TS_CDMX_INIT : ulParsingMode(%08x)", pst_inst->stCdmxCtrl.ulParsingMode);

		if( pst_inst->stCdmxCtrl.ulParsingMode & CDMX_MARKER_SEQUENTIAL_PLAY_MODE_BIT )
		{
			// Remark : Limitation of using TSDOPT_STREAMBUFF mode
			//  - This option requires that system should use 'fixed'-size FIFO element.
			//  - But, Android sequential mode player uses 'variable'-size FIFO element.
			//pst_inst->stDmxInit.m_ulOption |= TSDOPT_STREAMBUFF; // Do not use this mode yet
			pst_inst->stDmxInit.m_ulOption |= TSDOPT_USERBUFF;
		} 
		else 
		{
			pst_inst->stDmxInit.m_ulOption = (TSDOPT_SELECTIVE | TSDOPT_USERBUFF);
		}
		pst_inst->stDmxInit.m_ulOption |= TSDOPT_FIND_ENCINFO;
		pst_inst->stDmxInit.m_ulOption |= TSDOPT_MERGE_PES;
		{
			property_get("tcc.hdcp2.enable", value, "0");

			if (atoi(value) == 0) {
				ALOGE("tcc.hdcp2.enable  == 0");
				pst_inst->stDmxInit.m_stSecureFuncs.m_pfnSecureProc = NULL;
			} else {
				tsd_secure_funcs_t hdcp_func = { SecureParserProcess };
				pst_inst->stDmxInit.m_stSecureFuncs = hdcp_func;
			}

			property_get("tcc.cdmx.ts.crc.disable", value, "0");
			if( atoi(value) ) {
				ALOGE("tcc.cdmx.ts.crc.disable");
				pst_inst->stDmxInit.m_ulOption |= TSDOPT_DONT_CHECKCRC;
			}
		}
		pst_inst->stDmxInit.m_lMergeLimitBytes = 1024*512;
		pst_inst->stDmxInit.m_ulFIOBlockSizeNormal = 0;
		pst_inst->stDmxInit.m_ulFIOBlockSizeSeek = 0;

		if ( pst_inst->stCdmxCtrl.bPreInitDone ) 
			pst_inst->stDmxInit.m_pExtension = &pst_inst->stConfScanEndpts;	// from CDMX_PRE_INIT
		else
			pst_inst->stDmxInit.m_pExtension = NULL;

		if(pst_inst->stDmxInit.m_ulOption & TSDOPT_STREAMBUFF)
		{
			// Set buffer length (0 : disable)
			pst_inst->stDmxInit.m_lVideoBuffLength = 0;		
			pst_inst->stDmxInit.m_lAudioBuffLength = 0;
			pst_inst->stDmxInit.m_lGraphicsBuffLength = 0;
			pst_inst->stDmxInit.m_lSubtitleBuffLength = 0;

			// Set user stream buffer (NULL : disable)
			pst_inst->stDmxInit.m_pbyVideoBuffer = NULL; 
			pst_inst->stDmxInit.m_pbyAudioBuffer = NULL; 
			pst_inst->stDmxInit.m_pbyGraphicsBuffer = NULL;
			pst_inst->stDmxInit.m_pbySubtitleBuffer = NULL;

			pst_inst->stDmxStrmBuff.m_lVideoBuffLength = 0;
			pst_inst->stDmxStrmBuff.m_lAudioBuffLength = 0;
			pst_inst->stDmxStrmBuff.m_lGraphicsBuffLength = 0;
			pst_inst->stDmxStrmBuff.m_lSubtitleBuffLength = 0;

			pst_inst->stDmxStrmBuff.m_pbyVideoBuffer = NULL;
			pst_inst->stDmxStrmBuff.m_pbyAudioBuffer = NULL;
			pst_inst->stDmxStrmBuff.m_pbyGraphicsBuffer = NULL;
			pst_inst->stDmxStrmBuff.m_pbySubtitleBuffer = NULL;
		}

		if ( pst_inst->stCdmxCtrl.ulParsingMode & CDMX_MARKER_PROGRESSIVE_PLAY_MODE_BIT)
		{
			if (pst_inst->stDmxInit.m_pExtension != NULL) 
			{
				// to support progressive play
				pst_inst->stDmxInit.m_ulOption = 0;
				pst_inst->stDmxInit.m_ulOption |= TSDOPT_USERBUFF;
				pst_inst->stDmxInit.m_ulOption |= TSDOPT_FAST_OPEN;

				property_get("tcc.cdmx.ts.crc.disable", value, "0");
				if( atoi(value) )
					pst_inst->stDmxInit.m_ulOption |= TSDOPT_DONT_CHECKCRC;

				// file read size
				pst_inst->stDmxInit.m_ulFIOBlockSizeNormal = 64 * 1024;

				// 188 byte TS format fixed
				//pst_inst->stDmxInit.m_ulTsType = TS_FORMAT_NORMAL;
			}

			if (pst_inst->stCdmxCtrl.ulParsingMode & CDMX_MARKER_PLAYBACK_ONLY_MODE_BIT) 
			{
				pst_inst->stDmxInit.m_ulOption |= TSDOPT_PROGRESSIVE_FILE;
			}

			if(pst_inst->stCdmxCtrl.ulParsingMode & CDMX_MARKER_WFD_PLAYBACK_MODE_BIT) {
				pst_inst->stDmxInit.m_ulTsType = TS_FORMAT_NORMAL;
				pst_inst->stDmxInit.m_ulOption &= ~TSDOPT_FIND_ENCINFO;
				pst_inst->stDmxInit.m_ulOption |= TSDOPT_SKIP_PACKET_LOSS_PES; //Update for Release 13.12.11
				pst_inst->stDmxInit.m_ulFIOBlockSizeNormal = 4 * 1024;

				property_get("tcc.hdcp2.session.started", value, "0");
				if( atoi(value) )
					pst_inst->stDmxInit.m_ulOption |= TSDOPT_HDCP_SESSION_FORCED;
			}
		}
		//pst_inst->stDmxInit.m_ulOption |= TSDOPT_LEGACY_SEEK_PARAM_USED;

		if ( pst_inst->stCdmxCtrl.ulParsingMode & CDMX_MARKER_MEDIA_SCAN_MODE_BIT )
		{
			STEP( "\tDMXOP_OPEN_FOR_MEDIASCAN 01\n" );
			// SSG, temporarily set sequential mode due to error on close time 
			// it will be fixed next library
			pst_inst->stDmxInit.m_ulOption &= ~TSDOPT_SELECTIVE; 
			if( pst_inst->pfnDmxLib( TSDOP_OPEN_MEDIAINFO, NULL, &pst_inst->stDmxInit, &pst_inst->stTsdInfo ) == FALSE )
			{
				ret = pst_inst->pfnDmxLib(TSDOP_GET_LAST_ERROR, NULL, 0, 0);
				ERROR( "[%ld] Error: TSDOP_OPEN_MEDIAINFO failed (%d)\n", ret, __LINE__ );
				return ret;
			}	
			STEP( "\tDMXOP_OPEN_FOR_MEDIASCAN 02\n" );
		}
		else
		{
			STEP( "\tDMXOP_OPEN 01\n" );
			if( pst_inst->pfnDmxLib( TSDOP_OPEN, NULL, &pst_inst->stDmxInit, &pst_inst->stTsdInfo ) == FALSE )
			{
				ret = pst_inst->pfnDmxLib(TSDOP_GET_LAST_ERROR, NULL, 0, 0);
				ERROR( "[%ld] Error: TSDOP_OPEN failed (%d)\n", ret, __LINE__ );
				return (ret == 0) ? -1 : ret;
			}		
			STEP( "\tDMXOP_OPEN 02\n" );
		}
		pst_inst->hDmxHandle = pst_inst->stTsdInfo.m_hTsd;
		// SPDIF_AC3_TRUEHD Start
		if((pst_inst->stTsdInfo.m_stDefProgInfo.m_stAudioInfo.m_ulSubType == AUDIO_SUBTYPE_AC3_LOSSLESS) && (pst_inst->stCdmxCtrl.lSpdifMultiMode == 1))
		{
			tsd_demux_config_t config;	// SPDIF_AC3_TRUEHD
			config.m_ulOption = TSDOPT_SUB_STREAM_ONLY; 
			if(pst_inst->pfnDmxLib(TSDOP_DEMUX_CONFIG, pst_inst->hDmxHandle, &config, NULL) == 0)
			{
				ret = pst_inst->pfnDmxLib(TSDOP_GET_LAST_ERROR, pst_inst->hDmxHandle, 0, 0);
				ERROR( "[%ld] Error: TSDOP_DEMUX_CONFIG failed (%d)\n", ret, __LINE__ );
				return ret;
			}
		}
		pst_inst->stCdmxCtrl.lSpdifMultiMode = 0;
		// SPDIF_AC3_TRUEHD End
		if (pst_inst->stCdmxCtrl.ulParsingMode & CDMX_MARKER_PROGRESSIVE_PLAY_MODE_BIT)
		{
			if(pst_inst->stTsdInfo.m_stDefProgInfo.m_stVideoInfo.m_lWidth == 0)
				return pst_inst->stCdmxCtrl.ulParsingMode;
		}

		///////////////////////////////////////////////////////////////////////////////
		//
		// copy media info. to common structures
		//
		ret = ts_set_stream_info ( pst_inst );
		if( ret < 0 )
		{
			ERROR( "[%ld] Error: ts_set_stream_info failed (%d)\n", ret, __LINE__ );
            return ret;
        }

		///////////////////////////////////////////////////////////////////////////////
		//
		// set control variables & ..
		//
		{
			ts_cdmx_video_info_t *pst_vinfo = pst_inst->stDmxInfo.m_pstDefaultVideoInfo;

			if( pst_inst->stDmxInfo.m_ulVideoStreamTotal )
			{
				switch (pst_vinfo->m_ulFourCC) 
				{
				case FOURCC_AVC1: case FOURCC_avc1:
				case FOURCC_H264: case FOURCC_h264:
				case FOURCC_X264: case FOURCC_x264:
					if ( pst_vinfo->m_ulCodecPrivateSize ) 
					{
						pst_inst->stCdmxCtrl.lCodec = CDMX_VCODEC_AVC;
					}
					break;
				case FOURCC_mp4v: case FOURCC_MP4V:
					pst_inst->stCdmxCtrl.lCodec = CDMX_VCODEC_MPEG4;
					break;
				case FOURCC_WMV3: case FOURCC_wmv3:
					pst_inst->stCdmxCtrl.lCodec = CDMX_VCODEC_VC1_SP_MP;
					break;
				case FOURCC_WMVA: case FOURCC_wmva:
				case FOURCC_WVC1: case FOURCC_wvc1:
				case FOURCC_VC1:  case FOURCC_vc1:
					pst_inst->stCdmxCtrl.lCodec = CDMX_VCODEC_VC1_AP;
					break;
				case FOURCC_WMV1: case FOURCC_wmv1:
				case FOURCC_WMV2: case FOURCC_wmv2:
					pst_inst->stCdmxCtrl.lCodec = CDMX_VCODEC_WMV78;
					break;
				case FOURCC_RV30: case FOURCC_rv30:
				case FOURCC_RV40: case FOURCC_rv40:
					pst_inst->stCdmxCtrl.lCodec = CDMX_VCODEC_RV89;
					break;
				case FOURCC_DIV3: case FOURCC_div3:
				case FOURCC_DIV4: case FOURCC_div4:
				case FOURCC_MP43: case FOURCC_mp43:
					pst_inst->stCdmxCtrl.lCodec = CDMX_VCODEC_DIV3;
					break;
				case FOURCC_VP80: case FOURCC_vp80:
				case FOURCC_V_VP: case FOURCC_v_vp:
					pst_inst->stCdmxCtrl.lCodec = CDMX_VCODEC_VP8;
					break;
				default:
					pst_inst->stCdmxCtrl.lCodec = 0;
					break;
				}
				STEP( "\tCodec: %d\n", pst_inst->stCdmxCtrl.lCodec );
			}

			cdmx_print_info( &pst_inst->stDmxInfo, 
							 sizeof(*pst_inst->stDmxInfo.m_pstDefaultVideoInfo), 
							 sizeof(*pst_inst->stDmxInfo.m_pstDefaultAudioInfo), 
							 sizeof(*pst_inst->stDmxInfo.m_pstDefaultSubtitleInfo), 
							 0 );

			cdmx_get_stream_id_list( &pst_inst->stCdmxBuff.stStrmIdList,
									 &pst_inst->stDmxInfo, 
									 sizeof(*pst_inst->stDmxInfo.m_pstDefaultVideoInfo), 
									 sizeof(*pst_inst->stDmxInfo.m_pstDefaultAudioInfo), 
									 sizeof(*pst_inst->stDmxInfo.m_pstDefaultSubtitleInfo),
									 0 );
		}
		///////////////////////////////////////////////////////////////////////////////
		//
		// find_seq_header
		//
		#if 0
		if (!pst_inst->stCdmxCtrl.bSeqHeaderDone && pst_inst->stDmxInfo.m_pstDefaultVideoInfo)
		{
			pst_inst->stCdmxCtrl.bSeqHeaderDone = 1;
			STEP( "\tfind_seq_header: BEGIN\n" );
			ret = find_seq_header(pst_inst);
			if (ret < 0) {
				ALOGE( "[%ld] Error in find_seq_header%lld\n", ret);
				return ret;
			}
			STEP( "\tfind_seq_header: END\n" );
		}
		#endif
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
					   pst_inst->stDmxInfo.m_pstDefaultVideoInfo,
					   pst_inst->stDmxInfo.m_pstDefaultAudioInfo,
					   pst_inst->stDmxInfo.m_pstDefaultSubtitleInfo,
					   SET_CDMX_INFO_ALL);

	}//CDMX_GET_INFO
	else if( ulOpCode == CDMX_GET_INFO_SEL_STREAM )
	{
		cdmx_select_stream_t *pst_inp_param = (cdmx_select_stream_t*)pParam1;
		cdmx_info_t			 *pst_out_param = (cdmx_info_t         *)pParam2;

		STEP( "In CDMX_GET_INFO_SEL_STREAM\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		/////////////////////////////////////////////////////////////////
		//
		// Copy the media info of demuxer into tcc_cdk_warpper
		// 
		// 
		// [1] video info
		//
		if ( pst_inp_param->lSelType & DMXTYPE_VIDEO ) 
        {
			unsigned long i = 0;
			ts_cdmx_video_info_t* pst_vinfo = NULL;
			for( i = 0; i < pst_inst->stDmxInfo.m_ulVideoStreamTotal; ++i )
			{
				if( pst_inst->stDmxInfo.m_pstVideoInfoList[i].m_ulStreamID == pst_inp_param->lVideoID )
				{
					pst_vinfo = &pst_inst->stDmxInfo.m_pstVideoInfoList[i];
					break;
				}
			}
			set_cdmx_info (pst_out_param, 
						   pst_inst,
						   pst_vinfo,
						   NULL,
						   NULL,
						   SET_CDMX_INFO_VIDEO);
		}
		// 
		// [2] audio info
		//
		if ( pst_inp_param->lSelType & DMXTYPE_AUDIO ) 
        {
			unsigned long i = 0;
			ts_cdmx_audio_info_t *pst_ainfo = NULL;
			for( i = 0; i < pst_inst->stDmxInfo.m_ulAudioStreamTotal; ++i )
			{
				if( pst_inst->stDmxInfo.m_pstAudioInfoList[i].m_ulStreamID == pst_inp_param->lAudioID )
				{
					pst_ainfo = &pst_inst->stDmxInfo.m_pstAudioInfoList[i];
					break;
				}
			}
			set_cdmx_info (pst_out_param, 
						   pst_inst,
						   NULL,
						   pst_ainfo,
						   NULL,
						   SET_CDMX_INFO_AUDIO);
		}
		// 
		// [3] subtitle info
		//
		if ( pst_inp_param->lSelType & DMXTYPE_SUBTITLE ) 
		{
			unsigned long i = 0;
			ts_cdmx_subtitle_info_t	*pst_sinfo = NULL;
			for( i = 0; i < pst_inst->stDmxInfo.m_ulSubtitleStreamTotal; ++i )
			{
				if( pst_inst->stDmxInfo.m_pstSubtitleInfoList[i].m_ulStreamID == pst_inp_param->lSubtitleID )
				{
					pst_sinfo = &pst_inst->stDmxInfo.m_pstSubtitleInfoList[i];
					break;
				}
			}
			set_cdmx_info (pst_out_param, 
						   pst_inst,
						   NULL,
						   NULL,
						   pst_sinfo,
						   SET_CDMX_INFO_SUBTITLE);
		}

	}
	else if( ulOpCode == CDMX_SEL_STREAM )
	{
		cdmx_select_stream_t *pst_inp_param = (cdmx_select_stream_t*)pParam1;
		cdmx_info_t			 *pst_out_param = (cdmx_info_t         *)pParam2;
		tsd_selprog_t 		  dmx_sel_progra_info;
		tsd_selinfo_t 		  dmx_sel_output_info;

		STEP( "In CDMX_SEL_STREAM\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		DSTATUS("CDMX_SEL_STREAM - VideoID:%d, AudioID:%d, GraphicsID:%d, SubtitleID:%d", 
			 pst_inp_param->lVideoID, pst_inp_param->lAudioID, pst_inp_param->lGraphicsID, pst_inp_param->lSubtitleID);

		{
			// SPDIF_AC3_TRUEHD Start
			if(pst_inst->stCdmxCtrl.lSpdifTrueHDSub == 1)
			{
				tsd_demux_config_t config;	// SPDIF_AC3_TRUEHD
				config.m_ulOption = TSDOPT_SUB_STREAM_ONLY; 
				if(pst_inst->pfnDmxLib(TSDOP_DEMUX_CONFIG, pst_inst->hDmxHandle, &config, NULL) == 0)
				{
					ret = pst_inst->pfnDmxLib(TSDOP_GET_LAST_ERROR, pst_inst->hDmxHandle, 0, 0);
					ERROR( "[%ld] Error: TSDOP_DEMUX_CONFIG failed (%d)\n", ret, __LINE__ );
					return ret;
				}
			}
			pst_inst->stCdmxCtrl.lSpdifTrueHDSub = 0;
			// SPDIF_AC3_TRUEHD End
		}

		cdmx_memset(&dmx_sel_progra_info, 0, sizeof(tsd_selprog_t));

		if( pst_inp_param->lSelType & DMXTYPE_VIDEO )    
			dmx_sel_progra_info.m_ulSelectType |= TSDSEL_VIDEO;
		if( pst_inp_param->lSelType & DMXTYPE_AUDIO )    
			dmx_sel_progra_info.m_ulSelectType |= TSDSEL_AUDIO;
		if( pst_inp_param->lSelType & DMXTYPE_GRAPHICS ) 
			dmx_sel_progra_info.m_ulSelectType |= TSDSEL_GRAPHICS;
		if( pst_inp_param->lSelType & DMXTYPE_SUBTITLE ) 
			dmx_sel_progra_info.m_ulSelectType |= TSDSEL_SUBTITLE;

		dmx_sel_progra_info.m_ulVideoPID    = pst_inp_param->lVideoID;
		dmx_sel_progra_info.m_ulAudioPID    = pst_inp_param->lAudioID;
		dmx_sel_progra_info.m_ulGrapicsPID  = pst_inp_param->lGraphicsID;
		dmx_sel_progra_info.m_ulSubtitlePID = pst_inp_param->lSubtitleID;

		if( pst_inst->pfnDmxLib(TSDOP_SEL_PROG, pst_inst->hDmxHandle, &dmx_sel_progra_info, &dmx_sel_output_info) == FALSE )
		{
			ret = pst_inst->pfnDmxLib(TSDOP_GET_LAST_ERROR, pst_inst->hDmxHandle, 0, 0);
			ERROR( "[%ld] Error: TSDOP_SEL_PROG failed (%d)\n", ret, __LINE__ );
			return ret;
		}
		
		//ts_report_program_info(&dmx_sel_output_info);

		///////////////////////////////////////////////////////////////////////////////
		//
		// copy media info. to common structures
		//
		{
			ts_cdmx_video_info_t 	st_out_video_info;
			ts_cdmx_audio_info_t 	st_out_audio_info;
			ts_cdmx_subtitle_info_t st_out_subtitle_info;
			cdmx_memset(&st_out_video_info,	 0, sizeof(st_out_video_info));
			cdmx_memset(&st_out_audio_info,	 0, sizeof(st_out_audio_info));
			cdmx_memset(&st_out_subtitle_info,	 0, sizeof(st_out_subtitle_info));
			ret = ts_set_stream_info_sel_stream ( pst_inst, 
												  &dmx_sel_output_info.m_stVideoInfo,
												  &dmx_sel_output_info.m_stAudioInfo,
												  &dmx_sel_output_info.m_stGraphicsInfo,
												  &st_out_video_info,
												  &st_out_audio_info,
												  &st_out_subtitle_info );
			if( ret < 0 )
			{
				ERROR( "[%ld] Error: ts_set_stream_info_sel_stream failed (%d)\n", ret, __LINE__ );
				return ret;
			}

			set_cdmx_info (pst_out_param, pst_inst, 
						   &st_out_video_info,
						   &st_out_audio_info,
						   &st_out_subtitle_info,
						   SET_CDMX_INFO_ALL);

			STEP( "\tCall cdmx_print_sel_info\n" );
			cdmx_print_sel_info( (av_dmx_video_info_t*)&st_out_video_info, 
								 (av_dmx_audio_info_t*)&st_out_audio_info, 
								 (av_dmx_subtitle_info_t*)&st_out_subtitle_info, 
								 0 );
		}

	}//CDMX_SEL_STREAM
	else if( ulOpCode == CDMX_GETSTREAM )
	{
		cdmx_input_t	*pst_inp_param = (cdmx_input_t *)pParam1;
		cdmx_output_t	*pst_out_param = (cdmx_output_t*)pParam2;

		//STEP( "In CDMX_GETSTREAM\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		if(pst_inst->stDmxInit.m_ulOption & TSDOPT_STREAMBUFF)
		{
			if( pst_inst->stDmxStrmBuff.m_lVideoBuffLength == 0 && pst_inst->stDmxStrmBuff.m_lAudioBuffLength == 0)
			{
				pst_inst->stDmxStrmBuff.m_pbyVideoBuffer = pst_inp_param->m_pPacketBuff;
				pst_inst->stDmxStrmBuff.m_pbyAudioBuffer = pst_inp_param->m_pPacketBuff_ext1;
				pst_inst->stDmxStrmBuff.m_lVideoBuffLength = pst_inst->stDmxStrmBuff.m_lAudioBuffLength = pst_inp_param->m_iPacketBuffSize;
				if( pst_inst->pfnDmxLib(TSDOP_SETUP_BUFFER, pst_inst->hDmxHandle, &pst_inst->stDmxStrmBuff, NULL) == FALSE )
				{
					ret = pst_inst->pfnDmxLib(TSDOP_GET_LAST_ERROR, pst_inst->hDmxHandle, 0, 0);
					DPRINTF( "[CDMX:Err%d] TSDOP_SETUP_BUFFER failed", ret );
					return ret;
				}
				
				pst_inst->stDmxGetStreamBuff.m_pbyNextVideoBuff = NULL;
				pst_inst->stDmxGetStreamBuff.m_pbyNextAudioBuff = NULL;
			}
			else
			{
				pst_inst->stDmxGetStreamBuff.m_pbyNextVideoBuff = pst_inp_param->m_pPacketBuff;
				pst_inst->stDmxGetStreamBuff.m_pbyNextAudioBuff = pst_inp_param->m_pPacketBuff_ext1;
			}

			if( pst_inst->pfnDmxLib(TSDOP_GET_STREAM, pst_inst->hDmxHandle, &pst_inst->stDmxGetStreamBuff, &pst_inst->stDmxOutStreamBuff) == FALSE )
			{
				ret = pst_inst->pfnDmxLib(TSDOP_GET_LAST_ERROR, pst_inst->hDmxHandle, 0, 0);
				DPRINTF( "[CDMX:Err%d] pst_inst->pfnDmxLib TSDOP_GET_FRAME failed \n", ret );
				if( ret == TSDERR_END_OF_STREAM || ret == 0 )
				{
					if( pst_inst->stDmxGetFrame.m_ulStreamType == ES_TYPE_VIDEO )
						ret = ERR_END_OF_VIDEO_FILE;
					else if( pst_inst->stDmxGetFrame.m_ulStreamType == ES_TYPE_AUDIO )
						ret = ERR_END_OF_AUDIO_FILE;
					else
						ret = ERR_END_OF_FILE;
				}
				return ret;
			}
			else
			{
				cdmx_memset(pst_out_param, 0, sizeof(cdmx_output_t));
				if( pst_inst->stDmxOutStreamBuff.m_ulStreamType == ES_TYPE_VIDEO )
				{
					pst_out_param->m_iPacketType =	AV_PACKET_VIDEO;
					pst_out_param->m_pPacketData = pst_inp_param->m_pPacketBuff;
				}
				else if( pst_inst->stDmxOutStreamBuff.m_ulStreamType == ES_TYPE_AUDIO )
				{
					pst_out_param->m_iPacketType =	AV_PACKET_AUDIO;
					pst_out_param->m_pPacketData = pst_inp_param->m_pPacketBuff_ext1;
				}
				pst_out_param->m_iPacketSize = pst_inst->stDmxOutStreamBuff.m_lDataLength;
				pst_out_param->m_iTimeStamp = pst_inst->stDmxOutStreamBuff.m_lTimeStamp;
			}		
		}
		else
		{
TSDMX_GETSTREAM_START:

			pst_inst->stDmxGetFrame.m_pbyDataBuff = pst_inp_param->m_pPacketBuff;
			pst_inst->stDmxGetFrame.m_lBuffLength = pst_inp_param->m_iPacketBuffSize;

			if( pst_inp_param->m_iPacketType == AV_PACKET_VIDEO )
				pst_inst->stDmxGetFrame.m_ulStreamType = ES_TYPE_VIDEO;
			else if( pst_inp_param->m_iPacketType == AV_PACKET_AUDIO)	
				pst_inst->stDmxGetFrame.m_ulStreamType = ES_TYPE_AUDIO;

			if( pst_inst->pfnDmxLib(TSDOP_GET_FRAME, pst_inst->hDmxHandle, &pst_inst->stDmxGetFrame, &pst_inst->stDmxOutFrame) == FALSE )
			{
				ret = pst_inst->pfnDmxLib(TSDOP_GET_LAST_ERROR, pst_inst->hDmxHandle, 0, 0);
				DPRINTF( "[CDMX:Err%d] pst_inst->pfnDmxLib TSDOP_GET_FRAME failed \n", ret );
				if( ret == TSDERR_END_OF_STREAM )
				{
					if( pst_inst->stDmxGetFrame.m_ulStreamType == ES_TYPE_VIDEO )
						ret = ERR_END_OF_VIDEO_FILE;
					else if( pst_inst->stDmxGetFrame.m_ulStreamType == ES_TYPE_AUDIO )
						ret = ERR_END_OF_AUDIO_FILE;
					else
						ret = ERR_END_OF_FILE;
				}
				return ret;
			}
			else
			{
				cdmx_memset(pst_out_param, 0, sizeof(cdmx_output_t));
				pst_out_param->m_pPacketData 	= pst_inst->stDmxOutFrame.m_pbyDataBuff;
				pst_out_param->m_iPacketSize 	= pst_inst->stDmxOutFrame.m_lDataLength;
				pst_out_param->m_iTimeStamp 	= pst_inst->stDmxOutFrame.m_lTimeStamp;
				pst_out_param->m_bPacketDiscont = pst_inst->stDmxOutFrame.m_bPacketDiscont;

				if ( pst_out_param->m_bPacketDiscont == 1 ) {
					property_set("tcc.wfd.IDR.request", "1");
				}

				if( pst_inst->stDmxOutFrame.m_ulSecureType == SECURE_TYPE_HDCP ) {
					hdcp_secure_info_t *p_info = (hdcp_secure_info_t*)pst_inst->stDmxOutFrame.m_pSecureInfo;
					pst_out_param->m_ulSecureType = CDMX_SECURE_TYPE_HDCP;
					pst_out_param->m_enSecureInfo.stHdcpInfo.ulStreamCounter = p_info->ulStreamCounter;
					pst_out_param->m_enSecureInfo.stHdcpInfo.ullInputCounter = p_info->ullInputCounter;
				}

				if( pst_inst->stDmxOutFrame.m_ulStreamType == ES_TYPE_VIDEO )
				{
					if (pst_inst->stCdmxCtrl.bIsNotFirstFrame == 0)
					{
						if( pst_inst->stDmxOutFrame.m_ulSecureType == 0 ) {
							if( !find_sequence_header(pst_inst->stDmxOutFrame.m_pbyDataBuff, 
													  pst_inst->stDmxOutFrame.m_lDataLength, 
													  pst_inst->stDmxInfo.m_pstDefaultVideoInfo->m_ulFourCC) )
								goto TSDMX_GETSTREAM_START;
						}
						pst_inst->stCdmxCtrl.bIsNotFirstFrame = 1;
					}
					pst_out_param->m_iPacketType = AV_PACKET_VIDEO;
				}
				else if( pst_inst->stDmxOutFrame.m_ulStreamType == ES_TYPE_AUDIO )
				{
					pst_out_param->m_iPacketType = AV_PACKET_AUDIO;
					if(pst_inp_param->m_pPacketBuff_ext1 != NULL)
					{
						cdmx_memcpy(pst_inp_param->m_pPacketBuff_ext1, pst_inst->stDmxOutFrame.m_pbyDataBuff, pst_inst->stDmxOutFrame.m_lDataLength);
						pst_out_param->m_pPacketData = pst_inp_param->m_pPacketBuff_ext1;
					}
				}
				else if( pst_inst->stDmxOutFrame.m_ulStreamType == ES_TYPE_GRAPHICS )
				{
					pst_out_param->m_iPacketType = AV_PACKET_SUBTITLE;	
					if(pst_inp_param->m_pPacketBuff_ext2 != NULL)
					{
						cdmx_memcpy(pst_inp_param->m_pPacketBuff_ext2, pst_inst->stDmxOutFrame.m_pbyDataBuff, pst_inst->stDmxOutFrame.m_lDataLength);
						pst_out_param->m_pPacketData = pst_inp_param->m_pPacketBuff_ext2;
					}
				}
			}
		}
	}//CDMX_GETSTREAM
	else if( ulOpCode == CDMX_SEEK )
	{
		cdmx_seek_t		*pst_inp_param = (cdmx_seek_t  *)pParam1;
		cdmx_output_t	*pst_out_param = (cdmx_output_t*)pParam2;
		tsd_seek_t 		st_seek_input;
		tsd_seek_t 		st_seek_output;

		STEP( "In CDMX_SEEK\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		if(pst_inp_param->m_uiSeekMode == CDMX_SEEKMODE_RELATIVE_TIME)
		{
			DPRINTF( "[CDK_DMX] Relative time not supported in TS Demuxer \n");
			return -1;
		}

		st_seek_input.m_lSeekTime = pst_inp_param->m_iSeekTime;

		if (pst_inst->stDmxInit.m_ulOption & TSDOPT_PROGRESSIVE_FILE)
		{
			st_seek_input.m_ulFlags = TSDSEEK_EXTERNAL; 
		}
		else if (pst_inst->stDmxInit.m_ulOption & TSDOPT_FAST_OPEN)
		{
			st_seek_input.m_ulFlags = TSDSEEK_FAST_METHOD;
		}
		else
		{
			st_seek_input.m_ulFlags = 0; 
		}

		if( pst_inst->pfnDmxLib( TSDOP_FRAME_SEEK, pst_inst->hDmxHandle, &st_seek_input, &st_seek_output ) == FALSE )
		{
			ret = pst_inst->pfnDmxLib(TSDOP_GET_LAST_ERROR, pst_inst->hDmxHandle, 0, 0);
			if( ret == TSDERR_END_OF_STREAM || 
				ret == TSDERR_SEEK_FAILED_EOF ||
				ret == TSDERR_SEEK_FAILED )
				return ret = ERR_END_OF_FILE;

			return ret;
		}

		if (pst_inst->stDmxInit.m_ulOption & TSDOPT_PROGRESSIVE_FILE) {
			pst_out_param->m_iTimeStamp = pst_inp_param->m_iSeekTime;
		} else {
			pst_out_param->m_iTimeStamp = st_seek_output.m_lSeekTime;
		}
		// exceptional condition: need a seq.header.
		pst_inst->stCdmxCtrl.bIsNotFirstFrame = FALSE;
	}
	else if( ulOpCode == CDMX_CLOSE )
	{
		STEP( "In CDMX_CLOSE\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		ret = pst_inst->pfnDmxLib( TSDOP_CLOSE, pst_inst->hDmxHandle, NULL, NULL );
		if( ret == FALSE )
		{
			ERROR( "[%ld] Error: TSDOP_CLOSE failed (%d)\n", ret, __LINE__ );
			return -1;
		}

		// ts only
		if( pst_inst->stDmxInfo.m_pstFileInfo )
		{
			cdmx_free(pst_inst->stDmxInfo.m_pstFileInfo);
			pst_inst->stDmxInfo.m_pstFileInfo = NULL;
		}
		if( pst_inst->stDmxInfo.m_pstVideoInfoList )
		{
			cdmx_free(pst_inst->stDmxInfo.m_pstVideoInfoList);
			pst_inst->stDmxInfo.m_pstVideoInfoList = NULL;
		}
		if( pst_inst->stDmxInfo.m_pstAudioInfoList )
		{
			cdmx_free(pst_inst->stDmxInfo.m_pstAudioInfoList);
			pst_inst->stDmxInfo.m_pstAudioInfoList = NULL;
		}
		if( pst_inst->stDmxInfo.m_pstSubtitleInfoList )
		{
			cdmx_free(pst_inst->stDmxInfo.m_pstSubtitleInfoList);
			pst_inst->stDmxInfo.m_pstSubtitleInfoList = NULL;
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

	return ret;

}

#endif//INCLUDE_TS_DMX
