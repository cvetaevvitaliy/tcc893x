/****************************************************************************
 *   FileName    : cdmx_ogg.c
 *   Description : container demuxer of OGG
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

#ifdef INCLUDE_OGG_DMX

#include "../../cdk/cdk.h"
#include "../cdmx.h"
#include <cutils/properties.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	LOG
//
#define LOG_TAG    "CDMX_OGG"

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

#define cdmx_ogg_malloc		((*pst_inst).stDmxInit.m_sCallbackFuncMem).m_pfnMalloc
#define cdmx_ogg_free		((*pst_inst).stDmxInit.m_sCallbackFuncMem).m_pfnFree
#define cdmx_ogg_memset		((*pst_inst).stDmxInit.m_sCallbackFuncMem).m_pfnMemset
#define cdmx_ogg_memcpy		((*pst_inst).stDmxInit.m_sCallbackFuncMem).m_pfnMemcpy
#define cdmx_ogg_memmove	((*pst_inst).stDmxInit.m_sCallbackFuncMem).m_pfnMemmove
#define cdmx_ogg_realloc	((*pst_inst).stDmxInit.m_sCallbackFuncMem).m_pfnRealloc

#define cdmx_malloc 	cdmx_ogg_malloc
#define cdmx_free 		cdmx_ogg_free
#define cdmx_memset 	cdmx_ogg_memset
#define cdmx_memcpy 	cdmx_ogg_memcpy
#define cdmx_memmove	cdmx_ogg_memmove
#define cdmx_realloc 	cdmx_ogg_realloc

static 
long
find_seq_header( ogg_dmx_inst_t* pstInst )
{
	long ret = DMXRET_SUCCESS;
	ogg_dmx_inst_t* pst_inst = pstInst;

	// Get Seq Header
	if( pstInst->stDmxInfo.m_pstDefaultVideoInfo->m_ulCodecPrivateSize )
	{
		if( pstInst->stCdmxCtrl.lCodec == CDMX_VCODEC_AVC )
		{
			STEP( "\tCall avcc_to_bytestream BEGIN\n" );
			pstInst->stCdmxCtrl.lAvcNalLengthSize = avcc_to_bytestream( 
														 pstInst->stDmxInfo.m_pstDefaultVideoInfo->m_pbyCodecPrivate, 
														 pstInst->stDmxInfo.m_pstDefaultVideoInfo->m_ulCodecPrivateSize,
														&pstInst->stCdmxBuff.pbySeqHeaderBuff,
														&pstInst->stCdmxBuff.ulSeqHeaderBuffSize 
														);

			pstInst->stSeqHeader.m_pSeqHeaderData 		= pstInst->stCdmxBuff.pbySeqHeaderBuff;
			pstInst->stSeqHeader.m_iSeqHeaderDataLen 	= pstInst->stCdmxBuff.ulSeqHeaderBuffSize;

			// to prevent an unexpected error
			if(pstInst->stCdmxCtrl.lAvcNalLengthSize <= 0)
			{
				ERROR( "[Error Concealment] AvcNalLengthSize = %d, \n", pstInst->stCdmxCtrl.lAvcNalLengthSize );

				if(pstInst->stCdmxBuff.pbySeqHeaderBuff)
				{
					cdmx_free(pstInst->stCdmxBuff.pbySeqHeaderBuff);
					pstInst->stCdmxBuff.pbySeqHeaderBuff = NULL;
				}
				pstInst->stCdmxCtrl.lCodec = 0;
				pstInst->stCdmxCtrl.lAvcNalLengthSize = 0;

				pstInst->stDmxInfo.m_pstDefaultVideoInfo->m_ulCodecPrivateSize = 0;
				pstInst->stDmxInfo.m_pstDefaultVideoInfo->m_pbyCodecPrivate = NULL;

				pstInst->stSeqHeader.m_pSeqHeaderData = NULL;
				pstInst->stSeqHeader.m_iSeqHeaderDataLen = 0;
			}
			else
			{
				STEP( "\tcall cdmx_print_info END\n" );

				cdmx_memcpy( pstInst->stDmxInfo.m_pstDefaultVideoInfo->m_pbyCodecPrivate, 
							 pstInst->stSeqHeader.m_pSeqHeaderData, 
							 pstInst->stSeqHeader.m_iSeqHeaderDataLen );
				pstInst->stDmxInfo.m_pstDefaultVideoInfo->m_ulCodecPrivateSize = pstInst->stSeqHeader.m_iSeqHeaderDataLen;
			}
		}
		else
		{
			pstInst->stSeqHeader.m_pSeqHeaderData	  = pstInst->stDmxInfo.m_pstDefaultVideoInfo->m_pbyCodecPrivate;
			pstInst->stSeqHeader.m_iSeqHeaderDataLen = pstInst->stDmxInfo.m_pstDefaultVideoInfo->m_ulCodecPrivateSize;
		}

	}//if( pstInst->stDmxInfo.m_pstDefaultVideoInfo->m_ulCodecPrivateSize )
	else
	{
		if( (pstInst->stCdmxCtrl.ulParsingMode & CDMX_MARKER_MEDIA_SCAN_MODE_BIT) == 0 )
		{
			STEP( "\tIt has no CodecPrivate, Call cdmx_get_seqhead\n" );

			ret = cdmx_get_seqhead(0, 0, 0, 0, pstInst->stDmxInfo.m_pstDefaultVideoInfo->m_ulFourCC);
			if( ret < 0 ) return ret;
			if( ret > 0 ) // check Seq. extraction routine is available.
			{
				long found_seq_header = 0;
				long cnt_seq_header = 0;

				unsigned char* pby_temp = (unsigned char*)cdmx_malloc(CDMX_MAX_FRAME_LEN);
				if ( pby_temp == NULL ) {
					ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
					return CDK_ERR_MALLOC;
				}
				DSTATUS("[Line:%04d] pby_temp: 0x%x\n", __LINE__, pby_temp);

				//ALOGI( "cdmx_get_seqhead start\n" );

				// extraction routine is available. (return == 1)
				pstInst->stSeqHeader.m_iSeqHeaderDataLen 	= 0;
				pstInst->stSeqHeader.m_pSeqHeaderData 		= NULL;

				pstInst->stDmxGetStream.m_pbyStreamBuff		= pby_temp;
				pstInst->stDmxGetStream.m_lStreamBuffSize	= CDMX_MAX_FRAME_LEN;
				pstInst->stDmxGetStream.m_lStreamType		= DMXTYPE_VIDEO;

				while(cnt_seq_header < 300) 
				{
					//ALOGI( "[OGG] find Sequence Header:%d !! \n", cnt_seq_header );

					ret = pstInst->pfnDmxLib( DMXOP_GET_STREAM, &pstInst->hDmxHandle, &pstInst->stDmxGetStream, &pstInst->stDmxOutStream );
					if( ret < 0 )
					{
						if( pby_temp ) cdmx_free(pby_temp);
						ERROR( "[%ld] Error: DMXOP_GET_STREAM failed (%d)\n", ret, __LINE__ );
						return ret;
					}

					if( pstInst->stDmxOutStream.m_lStreamType == DMXTYPE_VIDEO )
					{
						// extract sequence header
						ret = cdmx_get_seqhead( pstInst->stDmxOutStream.m_pbyStreamData, 
												pstInst->stDmxOutStream.m_lStreamDataSize,
												&pstInst->stCdmxBuff.pbySeqHeaderBuff,
												&pstInst->stCdmxBuff.ulSeqHeaderBuffSize, 
												pstInst->stDmxInfo.m_pstDefaultVideoInfo->m_ulFourCC );
						if( ret < 0 ) 
						{
							if( pby_temp ) cdmx_free(pby_temp);
							return ret;
						}
						if( ret > 0 ) 
						{
							found_seq_header = 1;
							STEP( "\t\t OK! Sequence Header found !! \n" );
							pstInst->stSeqHeader.m_pSeqHeaderData 		= pstInst->stCdmxBuff.pbySeqHeaderBuff;
							pstInst->stSeqHeader.m_iSeqHeaderDataLen 	= pstInst->stCdmxBuff.ulSeqHeaderBuffSize;
							break;
						}
					}
					cnt_seq_header++;
				}// end of while

				if(found_seq_header==0)
				{
					if( pstInst->stCdmxCtrl.lCodec == CDMX_VCODEC_MPEG4 )
					{
						STEP( "\t\t Make MPEG-4 VOL header BEGIN \n" );
						cnt_seq_header = 0;
						pstInst->stDmxSeek.m_lSeekTime = 0;
						pstInst->stDmxSeek.m_ulSeekMode = DMXSEEK_DEFAULT;

						STEP( "\t\t DMXOP_SEEK: seek to zero \n" );
						ret = pstInst->pfnDmxLib( DMXOP_SEEK, &pstInst->hDmxHandle, &pstInst->stDmxSeek, &pstInst->stDmxOutStream );
						if ( ret<0 ) 
						{
							if( pby_temp ) cdmx_free(pby_temp);
							ERROR( "[%ld] Error: DMXOP_SEEK failed (%d)\n", ret, __LINE__ );
							return ret;
						}

						STEP( "\t\t Get Video Frame \n" );
						while(cnt_seq_header < 300) 
						{
							//ALOGI( "[OGG] DMXOP_GET_STREAM:%d !! \n", cnt_seq_header );
							ret = pstInst->pfnDmxLib( DMXOP_GET_STREAM, &pstInst->hDmxHandle, &pstInst->stDmxGetStream, &pstInst->stDmxOutStream );
							if( ret < 0 )
							{
								if( pby_temp ) cdmx_free(pby_temp);
								ERROR( "[%ld] Error: DMXOP_GET_STREAM failed (%d)\n", ret, __LINE__ );
								return ret;
							}

							if( pstInst->stDmxOutStream.m_lStreamType == DMXTYPE_VIDEO )
							{
								if( pstInst->stDmxOutStream.m_lStreamDataSize )
								{
									//ALOGI( "[OGG] call make_mpeg4_vol_header() !! \n" );
									ret = make_mpeg4_vol_header(
												 pstInst->stDmxOutStream.m_pbyStreamData,
												 pstInst->stDmxOutStream.m_lStreamDataSize,
												&pstInst->stCdmxBuff.pbySeqHeaderBuff,
												&pstInst->stCdmxBuff.ulSeqHeaderBuffSize, 
												 pstInst->stDmxInfo.m_pstDefaultVideoInfo->m_ulWidth,
												 pstInst->stDmxInfo.m_pstDefaultVideoInfo->m_ulHeight,
												 0 
												 );
									if(ret==0)
									{
										STEP( "\t\t Make MPEG-4 VOL header END \n" );
										pstInst->stSeqHeader.m_pSeqHeaderData 		= pstInst->stCdmxBuff.pbySeqHeaderBuff;
										pstInst->stSeqHeader.m_iSeqHeaderDataLen 	= pstInst->stCdmxBuff.ulSeqHeaderBuffSize;
										break;
									}
								}
							}
							cnt_seq_header++;
						}// end of while
					}
				}// end of if(found_seq_header==0)

				STEP( "\tSequence header extraction finished: seek to zero \n" );
				// sequence header extraction finished (return ==1)
				pstInst->stDmxSeek.m_lSeekTime = 0;
				pstInst->stDmxSeek.m_ulSeekMode = DMXSEEK_DEFAULT;

				ret = pstInst->pfnDmxLib( DMXOP_SEEK, &pstInst->hDmxHandle, &pstInst->stDmxSeek, &pstInst->stDmxOutStream );
				if( ret < 0 )
				{
					ERROR( "[%ld] Error: DMXOP_SEEK failed (%d)\n", ret, __LINE__ );
				}

				if( pby_temp ) cdmx_free(pby_temp);

			}// end of if( cdmx_get_seqhead )
		}//if( (pstInst->stCdmxCtrl.ulParsingMode & CDMX_MARKER_MEDIA_SCAN_MODE_BIT) == 0 )
	}// end of else

	STEP( "\tm_ulCodecPrivateSize(m_iSeqHeaderDataLen): %d \n", pstInst->stSeqHeader.m_iSeqHeaderDataLen );
	#ifdef DBG_PRINT_DATA
	if ( pstInst->stSeqHeader.m_iSeqHeaderDataLen )
	{
		cdmx_print_data(pstInst->stSeqHeader.m_pSeqHeaderData, pstInst->stSeqHeader.m_iSeqHeaderDataLen, 16, 128);
	}
	#endif
	
	return ret;
}

#define SET_CDMX_INFO_FILE 		1
#define SET_CDMX_INFO_VIDEO 	2
#define SET_CDMX_INFO_AUDIO 	4
#define SET_CDMX_INFO_SUBTITLE 	8
#define SET_CDMX_INFO_ALL 		(SET_CDMX_INFO_FILE|SET_CDMX_INFO_VIDEO|SET_CDMX_INFO_AUDIO|SET_CDMX_INFO_SUBTITLE)
static
void 
set_cdmx_info(
    cdmx_info_t				*pstCdmxInfo,
    ogg_dmx_inst_t			*pstInst,
	ogg_cdmx_video_info_t	*pstVideoInfo,
	ogg_cdmx_audio_info_t	*pstAudioInfo,
	unsigned long 	 		 ulType
	)
{
	ogg_dmx_inst_t			*pst_inst    = pstInst;
	cdmx_info_t				*pst_out_info= pstCdmxInfo;
	ogg_cdmx_file_info_t	*pst_finfo	 = pst_inst->stDmxInfo.m_pstFileInfo;
	ogg_cdmx_video_info_t	*pst_vinfo	 = pstVideoInfo;
	ogg_cdmx_audio_info_t	*pst_ainfo	 = pstAudioInfo;
	//
	// [1] file info
	//
	if ( ( ulType & SET_CDMX_INFO_FILE ) && ( pst_finfo != NULL ) )
	{
		//FUNC("In set_cdmx_info: SET_CDMX_INFO_FILE \n");
		cdmx_memset( &pst_out_info->m_sFileInfo, 0, sizeof(pst_out_info->m_sFileInfo) );
		pst_out_info->m_sFileInfo.m_pszOpenFileName		= pst_finfo->m_pszFileName;
		pst_out_info->m_sFileInfo.m_pszCopyright		= pst_finfo->m_pszCopyright;
		pst_out_info->m_sFileInfo.m_pszCreationTime		= pst_finfo->m_pszCreationTime;
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
		pst_out_info->m_sFileInfo.m_iSeekable			= pst_finfo->m_bSeekable;
		pst_out_info->m_sFileInfo.m_ulSecureType		= 0;
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
		pst_out_info->m_sVideoInfo.m_ulmax_bit_rate 		= 0;
		pst_out_info->m_sVideoInfo.m_ulavg_bit_rate 		= 0;
		pst_out_info->m_sVideoInfo.m_pszCodecName 			= pst_vinfo->m_pszCodecName;
		pst_out_info->m_sVideoInfo.m_pszCodecVendorName 	= pst_vinfo->m_pszCodecVendorName;
		pst_out_info->m_sVideoInfo.m_pExtraData      		= pst_vinfo->m_pbyCodecPrivate;
		pst_out_info->m_sVideoInfo.m_iExtraDataLen   		= pst_vinfo->m_ulCodecPrivateSize;
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
		pst_out_info->m_sVideoInfo.m_iBitRate				= pst_vinfo->m_ulBitRate;
		if ( pst_out_info->m_sVideoInfo.m_iBitRate == 0 && pst_finfo->m_ulDuration > 0 ) {
			//duration: *1000
			//bitrate: kbps
			pst_out_info->m_sVideoInfo.m_iBitRate = (int)( (pst_finfo->m_llFileSize*8.0*1000)/(pst_finfo->m_ulDuration*1000.0) ); // kbps
		}
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
		pst_out_info->m_sAudioInfo.m_iChannels			= (pst_ainfo->m_ulChannels > 8) ? 0 : pst_ainfo->m_ulChannels;
		pst_out_info->m_sAudioInfo.m_iFormatId			= pst_ainfo->m_ulFormatTag;
		pst_out_info->m_sAudioInfo.m_ulmax_bit_rate		= 0;
		pst_out_info->m_sAudioInfo.m_ulavg_bit_rate		= 0;
		pst_out_info->m_sAudioInfo.m_pszCodecName		= pst_ainfo->m_pszCodecName;
		pst_out_info->m_sAudioInfo.m_pszCodecVendorName	= pst_ainfo->m_pszCodecVendorName;
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
		pst_out_info->m_sAudioInfo.m_ulSubType			= 0;
		pst_out_info->m_sAudioInfo.m_iMinDataSize		= 0;
	}
}

static
long
ogg_set_stream_info ( 
    ogg_dmx_inst_t *pstInst
	)
{
	long i, ret = 0;
	ogg_dmx_inst_t 	*pst_inst = pstInst;
	ogg_cdmx_info_t	*pst_dmx_info = &pst_inst->stDmxInfo;
	ogg_dmx_info_t 	*pst_ogg_info = &pst_inst->stOggInfo;
	ogg_dmx_info_t 	*pst_ogg_curr_info = NULL;

	ogg_dmx_sel_stream_t   dmx_sel_strm;
	ogg_dmx_sel_info_t 	   dmx_sel_info;

	//
	// file info
	//
	if ( pst_dmx_info->m_pstFileInfo == NULL )
	{
		pst_dmx_info->m_pstFileInfo = cdmx_malloc(sizeof(ogg_cdmx_file_info_t));
		if ( pst_dmx_info->m_pstFileInfo == NULL ) {
			ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
			return CDK_ERR_MALLOC;
		}
		DSTATUS("[Line:%04d] pstStreamIdList->plVideoIdList: 0x%x\n", __LINE__, pst_dmx_info->m_pstFileInfo);
		cdmx_memset( pst_dmx_info->m_pstFileInfo, 0, sizeof(ogg_cdmx_file_info_t) );
	}
	pst_dmx_info->m_pstFileInfo->m_pszFileName 		= pst_ogg_info->m_sFileInfo.m_pszOpenFileName;
	pst_dmx_info->m_pstFileInfo->m_ulDuration		= pst_ogg_info->m_sFileInfo.m_iRunningtime;
	pst_dmx_info->m_pstFileInfo->m_llFileSize 		= pst_ogg_info->m_sFileInfo.m_lFileSize;

	pst_dmx_info->m_pstFileInfo->m_pszCopyright		= pst_ogg_info->m_sFileInfo.m_pszCopyright;				
	pst_dmx_info->m_pstFileInfo->m_pszCreationTime	= pst_ogg_info->m_sFileInfo.m_pszCreationTime;				
	pst_dmx_info->m_pstFileInfo->m_ulTotalBitRate	= pst_ogg_info->m_sFileInfo.m_uiTotalBitrate;
	pst_dmx_info->m_pstFileInfo->m_bSeekable		= pst_ogg_info->m_sFileInfo.m_uiSeekalble;

	//
	// video info
	//

	if(pst_ogg_info->m_sVideoInfo.m_iWidth < 0)		pst_ogg_info->m_sVideoInfo.m_iWidth = 0;
	if(pst_ogg_info->m_sVideoInfo.m_iHeight < 0)	pst_ogg_info->m_sVideoInfo.m_iHeight = 0;
	
	if( pst_ogg_info->m_sVideoInfo.m_iWidth && pst_ogg_info->m_sVideoInfo.m_iHeight )
	{
		long l_total_video_num = 1;
		if ( pst_dmx_info->m_pstVideoInfoList == NULL )
		{
			pst_dmx_info->m_pstVideoInfoList = cdmx_malloc(sizeof(ogg_cdmx_video_info_t) * l_total_video_num);
			if ( pst_dmx_info->m_pstVideoInfoList == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] pst_dmx_info->m_pstVideoInfoList: 0x%x\n", __LINE__, pst_dmx_info->m_pstVideoInfoList);
			cdmx_memset( pst_dmx_info->m_pstVideoInfoList, 0, sizeof(ogg_cdmx_video_info_t) * l_total_video_num );
		}

		for(i = 0; i < l_total_video_num; i++)
		{
			ogg_dmx_video_info_t 	*pst_inp_info = NULL;
			ogg_cdmx_video_info_t 	*pst_out_info = &pst_dmx_info->m_pstVideoInfoList[i];

			if( i > 1 )
			{
				// select track and get info
				cdmx_memset(&dmx_sel_strm, 0, sizeof(dmx_sel_strm));
				cdmx_memset(&dmx_sel_info, 0, sizeof(dmx_sel_info));

				dmx_sel_strm.m_lSelType = DMXTYPE_VIDEO|DMXTYPE_AUDIO;
				dmx_sel_strm.m_lVideoID = i;
				dmx_sel_strm.m_lAudioID = 0;
				dmx_sel_info.m_pstVideoInfo = &pst_ogg_info->m_sVideoInfo;
				dmx_sel_info.m_pstAudioInfo = &pst_ogg_info->m_sAudioInfo;

				DSTATUS( "[VideoID:%d] [AudioID:%d] \n", dmx_sel_strm.m_lVideoID, dmx_sel_strm.m_lAudioID );
				ret = pst_inst->pfnDmxLib( DMXOP_SELECT_STREAM, &pst_inst->hDmxHandle, &dmx_sel_strm, &dmx_sel_info );
				if( ret < 0 )
				{
					ERROR( "[%ld] Error: DMXOP_SELECT_STREAM failed (%d)\n", ret, __LINE__ );
					return ret;
				}
			}
			pst_inp_info = &pst_ogg_info->m_sVideoInfo;

			pst_out_info->m_ulStreamID  		= i;
			pst_out_info->m_ulWidth				= pst_inp_info->m_iWidth;
			pst_out_info->m_ulHeight			= pst_inp_info->m_iHeight;
			pst_out_info->m_ulFrameRate 		= pst_inp_info->m_iFrameRate;
			pst_out_info->m_ulFourCC			= pst_inp_info->m_iFourCC;
			pst_out_info->m_pbyCodecPrivate 	= pst_inp_info->m_pExtraData;
			pst_out_info->m_ulCodecPrivateSize	= pst_inp_info->m_iExtraDataLen;

			pst_out_info->m_pszCodecName		= pst_inp_info->m_pszCodecName;
			pst_out_info->m_pszCodecVendorName	= pst_inp_info->m_pszCodecVendorName;
			pst_out_info->m_ulBitRate			= pst_inp_info->m_uiBitrate;


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
		}
		// set default 
		pst_dmx_info->m_pstDefaultVideoInfo = &pst_dmx_info->m_pstVideoInfoList[0];
		pst_dmx_info->m_ulVideoStreamTotal = l_total_video_num;
	}
	//
	// audio info
	//
//	if( pst_ogg_info->m_sAudioInfo.m_iTotalNumber > 0 )
//		pst_ogg_info->m_sAudioInfo.m_iTotalNumber = 1;		//not support multiaudio selection [20140116_Helena]

	if( pst_ogg_info->m_sAudioInfo.m_iTotalNumber > 0 )
	{
		long l_total_audio_num = pst_ogg_info->m_sAudioInfo.m_iTotalNumber;
		if( pst_dmx_info->m_pstAudioInfoList == NULL )
		{
			pst_dmx_info->m_pstAudioInfoList = cdmx_malloc(sizeof(ogg_cdmx_audio_info_t) * l_total_audio_num);
			if ( pst_dmx_info->m_pstAudioInfoList == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] pst_dmx_info->m_pstVideoInfoList: 0x%x\n", __LINE__, pst_dmx_info->m_pstAudioInfoList);
			cdmx_memset( pst_dmx_info->m_pstAudioInfoList, 0, sizeof(ogg_cdmx_audio_info_t) * l_total_audio_num );
		}

		for(i = 0; i < l_total_audio_num; i++)
		{
			ogg_dmx_audio_info_t 	*pst_inp_info = NULL;
			ogg_cdmx_audio_info_t 	*pst_out_info = &pst_dmx_info->m_pstAudioInfoList[i];

			if( i > 1 )
			{
				// select track and get info
				cdmx_memset(&dmx_sel_strm, 0, sizeof(dmx_sel_strm));
				cdmx_memset(&dmx_sel_info, 0, sizeof(dmx_sel_info));

				dmx_sel_strm.m_lSelType = DMXTYPE_VIDEO|DMXTYPE_AUDIO;
				dmx_sel_strm.m_lVideoID = 0;
				dmx_sel_strm.m_lAudioID = i;
				dmx_sel_info.m_pstVideoInfo = &pst_ogg_info->m_sVideoInfo;
				dmx_sel_info.m_pstAudioInfo = &pst_ogg_info->m_sAudioInfo;

				DSTATUS( "[VideoID:%d] [AudioID:%d] \n", dmx_sel_strm.m_lVideoID, dmx_sel_strm.m_lAudioID );
				ret = pst_inst->pfnDmxLib( DMXOP_SELECT_STREAM, &pst_inst->hDmxHandle, &dmx_sel_strm, &dmx_sel_info );
				if( ret < 0 )
				{
					ERROR( "[%ld] Error: DMXOP_SELECT_STREAM failed (%d)\n", ret, __LINE__ );
					return ret;
				}
			}
			pst_inp_info = &pst_ogg_info->m_sAudioInfo;

			pst_out_info->m_ulStreamID			= i;
			pst_out_info->m_ulSamplePerSec		= pst_inp_info->m_iSamplePerSec;
			pst_out_info->m_ulBitsPerSample		= pst_inp_info->m_iBitsPerSample;
			pst_out_info->m_ulChannels			= pst_inp_info->m_iChannels;
			pst_out_info->m_ulFormatTag			= pst_inp_info->m_iFormatId;
			pst_out_info->m_pbyCodecPrivate		= pst_inp_info->m_pExtraData;
			pst_out_info->m_ulCodecPrivateSize	= pst_inp_info->m_iExtraDataLen;
			pst_out_info->m_pszLanguage			= NULL;

			pst_out_info->m_pszCodecName		= pst_inp_info->m_pszCodecName;
			pst_out_info->m_pszCodecVendorName	= pst_inp_info->m_pszCodecVendorName;
			pst_out_info->m_ulBitRate			= pst_inp_info->m_uiBitrate;

			DSTATUS( " [Audio_%03ld] - StreamID: %ld / %ld Hz / %ld bits / %ld Ch / FormatTag: 0x%04X / Language: \"%s\" \n",
						i,
						pst_out_info->m_ulStreamID,
						pst_out_info->m_ulSamplePerSec,
						pst_out_info->m_ulBitsPerSample,
						pst_out_info->m_ulChannels,
						(unsigned int)pst_out_info->m_ulFormatTag,
						pst_out_info->m_pszLanguage);
		}
		// set default 
		pst_dmx_info->m_pstDefaultAudioInfo = &pst_dmx_info->m_pstAudioInfoList[0];
		pst_dmx_info->m_ulAudioStreamTotal = l_total_audio_num;
	}

	// select default tracks
	if ( pst_dmx_info->m_ulVideoStreamTotal > 1 || pst_dmx_info->m_ulAudioStreamTotal > 1 ) 
	{
		// select track and get info
		cdmx_memset(&dmx_sel_strm, 0, sizeof(dmx_sel_strm));
		cdmx_memset(&dmx_sel_info, 0, sizeof(dmx_sel_info));
		if( pst_dmx_info->m_ulVideoStreamTotal > 1 )
		{
			dmx_sel_strm.m_lSelType |= DMXTYPE_VIDEO;
			dmx_sel_strm.m_lVideoID = 0;//?
			if ( pst_dmx_info->m_pstDefaultVideoInfo == NULL )
				pst_dmx_info->m_pstDefaultVideoInfo = &pst_dmx_info->m_pstVideoInfoList[0];
			DSTATUS( "Set default [VideoID:%d]\n", dmx_sel_strm.m_lVideoID );
		}
		if( pst_dmx_info->m_ulAudioStreamTotal > 1 )
		{
			dmx_sel_strm.m_lSelType |= DMXTYPE_AUDIO;
			dmx_sel_strm.m_lAudioID = 0;//?
			if ( pst_dmx_info->m_pstDefaultAudioInfo == NULL )
				pst_dmx_info->m_pstDefaultAudioInfo = &pst_dmx_info->m_pstAudioInfoList[0];
			DSTATUS( "Set default [AudioID:%d] \n", dmx_sel_strm.m_lAudioID );
		}

		dmx_sel_info.m_pstVideoInfo = &pst_ogg_info->m_sVideoInfo;
		dmx_sel_info.m_pstAudioInfo = &pst_ogg_info->m_sAudioInfo;
		ret = pst_inst->pfnDmxLib( DMXOP_SELECT_STREAM, &pst_inst->hDmxHandle, &dmx_sel_strm, &dmx_sel_info );
		if( ret < 0 )
		{
			ERROR( "[%ld] Error: DMXOP_SELECT_STREAM failed (%d)\n", ret, __LINE__ );
			return ret;
		}
	}

	return 0;
}

static
long
ogg_set_stream_info_sel_stream ( 
    ogg_dmx_inst_t 			*pstInst,
	ogg_dmx_video_info_t 	*pstSelVideoInfo,
	ogg_dmx_audio_info_t 	*pstSelAudioInfo,
	ogg_cdmx_video_info_t 	*pstOutVideoInfo,
	ogg_cdmx_audio_info_t 	*pstOutAudioInfo
	)
{
	long i;
	ogg_dmx_inst_t  *pst_inst = pstInst;
	ogg_cdmx_info_t *pst_dmx_info = &pst_inst->stDmxInfo;
	ogg_dmx_info_t  *pst_ogg_info = &pst_inst->stOggInfo;

	//
	// video info
	//
	if( pstSelVideoInfo && pstOutVideoInfo )
	{
		ogg_dmx_video_info_t 	*pst_inp_info 	= pstSelVideoInfo;
		ogg_cdmx_video_info_t 	*pst_video_info = pstOutVideoInfo;

		pst_video_info->m_ulStreamID  		= 0;
		pst_video_info->m_ulWidth			= pst_inp_info->m_iWidth;
		pst_video_info->m_ulHeight			= pst_inp_info->m_iHeight;
		pst_video_info->m_ulFrameRate 		= pst_inp_info->m_iFrameRate;
		pst_video_info->m_ulFourCC			= pst_inp_info->m_iFourCC;
		pst_video_info->m_pbyCodecPrivate 	= pst_inp_info->m_pExtraData;
		pst_video_info->m_ulCodecPrivateSize= pst_inp_info->m_iExtraDataLen;

		pst_video_info->m_pszCodecName		= pst_inp_info->m_pszCodecName;
		pst_video_info->m_pszCodecVendorName= pst_inp_info->m_pszCodecVendorName;
		pst_video_info->m_ulBitRate			= pst_inp_info->m_uiBitrate;

		DSTATUS( " [Video_%03ld] - StreamID: %ld / %ldx%ld pix / %.3f fps / FourCC: \"%c%c%c%c\" \n",
					i,
					pst_video_info->m_ulStreamID,
					pst_video_info->m_ulWidth,
					pst_video_info->m_ulHeight,
					(float)pst_video_info->m_ulFrameRate/1000,
					(char)(pst_video_info->m_ulFourCC >> 0 ),
					(char)(pst_video_info->m_ulFourCC >> 8 ),
					(char)(pst_video_info->m_ulFourCC >> 16),
					(char)(pst_video_info->m_ulFourCC >> 24)
				);
	}

	//
	// audio info
	//
	if( pstSelAudioInfo && pstOutAudioInfo )
	{
		ogg_dmx_audio_info_t 	*pst_inp_info 	= pstSelAudioInfo;
		ogg_cdmx_audio_info_t 	*pst_audio_info = pstOutAudioInfo;

		pst_audio_info->m_ulStreamID			= 0;
		pst_audio_info->m_ulSamplePerSec		= pst_inp_info->m_iSamplePerSec;
		pst_audio_info->m_ulBitsPerSample		= pst_inp_info->m_iBitsPerSample;
		pst_audio_info->m_ulChannels			= pst_inp_info->m_iChannels;
		pst_audio_info->m_ulFormatTag			= pst_inp_info->m_iFormatId;
		pst_audio_info->m_pbyCodecPrivate		= pst_inp_info->m_pExtraData;
		pst_audio_info->m_ulCodecPrivateSize	= pst_inp_info->m_iExtraDataLen;
		pst_audio_info->m_pszLanguage			= NULL;

		pst_audio_info->m_pszCodecName			= pst_inp_info->m_pszCodecName;
		pst_audio_info->m_pszCodecVendorName	= pst_inp_info->m_pszCodecVendorName;
		pst_audio_info->m_ulBitRate				= pst_inp_info->m_uiBitrate;

		DSTATUS( " [Audio_%03ld] - StreamID: %ld / %ld Hz / %ld bits / %ld Ch / FormatTag: 0x%04X / Language: \"%s\" \n",
					i,
					pst_audio_info->m_ulStreamID,
					pst_audio_info->m_ulSamplePerSec,
					pst_audio_info->m_ulBitsPerSample,
					pst_audio_info->m_ulChannels,
					(unsigned int)pst_audio_info->m_ulFormatTag,
					pst_audio_info->m_pszLanguage);
	}

	return 0;
}



av_result_t 
cdmx_ogg( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 )
{
	av_result_t ret = DMXRET_SUCCESS;
	ogg_dmx_inst_t* pst_inst = (ogg_dmx_inst_t*)(*ptHandle); // assume that its memory is allocated.

	if( ulOpCode == CDMX_INIT )
	{
		cdmx_init_t				*pst_init_param = (cdmx_init_t*)pParam1;
		cdmx_callback_func_t	*pst_callback   = (cdmx_callback_func_t*)pst_init_param->pstCallback;

		STEP( "In CDMX_INIT\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		{
		#ifdef CDMX_USE_DYNAMIC_LOADING
			void* pv_dmx;
			void* pv_handle;
			ret = open_library( (const char*)"/system/lib/libff9.so",
								(const char*)"TCC_OGG_DMX",
								&pv_dmx,
								&pv_handle );
			if (ret < 0) {
				return ret;
			}
			pst_inst->pfnDmxLib = ( av_result_t (*) (unsigned long, av_handle_t*, void*, void*) )(pv_dmx);
			pst_inst->pvLibHandle = pv_handle;
			STEP( "LibHandle loaded....(0x%x) (handle:0x%x) ...\n", pst_inst->pfnDmxLib, pst_inst->pvLibHandle );
		#else
			pst_inst->pfnDmxLib   = TCC_OGG_DMX;
			pst_inst->pvLibHandle = NULL;
		#endif
		}

		pst_inst->stCdmxCtrl.ulParsingMode = pst_init_param->ulParsingMode;
		//--------------------------------------------------
		// set init parameters
		//--------------------------------------------------
		pst_inst->stDmxInit.m_pOpenFileName = pst_init_param->pszFileName;
		
		pst_inst->stDmxInit.m_sCallbackFuncMem.m_pfnMalloc		= pst_callback->m_pfMalloc;
		pst_inst->stDmxInit.m_sCallbackFuncMem.m_pfnFree		= pst_callback->m_pfFree;
		pst_inst->stDmxInit.m_sCallbackFuncMem.m_pfnMemcmp		= pst_callback->m_pfMemcmp;
		pst_inst->stDmxInit.m_sCallbackFuncMem.m_pfnMemcpy		= pst_callback->m_pfMemcpy;
		pst_inst->stDmxInit.m_sCallbackFuncMem.m_pfnMemset		= pst_callback->m_pfMemset;
		pst_inst->stDmxInit.m_sCallbackFuncMem.m_pfnRealloc		= pst_callback->m_pfRealloc;
		pst_inst->stDmxInit.m_sCallbackFuncMem.m_pfnMemmove		= pst_callback->m_pfMemmove;

		pst_inst->stDmxInit.m_sCallbackFuncFile.m_pfnFopen		= pst_callback->m_pfFopen;
		pst_inst->stDmxInit.m_sCallbackFuncFile.m_pfnFread		= pst_callback->m_pfFread;
		pst_inst->stDmxInit.m_sCallbackFuncFile.m_pfnFseek		= pst_callback->m_pfFseek;
		pst_inst->stDmxInit.m_sCallbackFuncFile.m_pfnFtell		= pst_callback->m_pfFtell;
		pst_inst->stDmxInit.m_sCallbackFuncFile.m_pfnFwrite		= pst_callback->m_pfFwrite;
		pst_inst->stDmxInit.m_sCallbackFuncFile.m_pfnFclose		= pst_callback->m_pfFclose;
		pst_inst->stDmxInit.m_sCallbackFuncFile.m_pfnFeof		= pst_callback->m_pfFeof;
		pst_inst->stDmxInit.m_sCallbackFuncFile.m_pfnFflush		= pst_callback->m_pfFflush;
		pst_inst->stDmxInit.m_sCallbackFuncFile.m_pfnFseek64	= pst_callback->m_pfFseek64;
		pst_inst->stDmxInit.m_sCallbackFuncFile.m_pfnFtell64	= pst_callback->m_pfFtell64;
		cdmx_memory_func_init(&pst_inst->stDmxInit.m_sCallbackFuncMem);

		pst_inst->stDmxInit.m_uiOpenOption 		= OGG_OPENOPT_SELECTIVE;
		pst_inst->stDmxInit.m_uiFileCacheSize	= 1024*4;

		if ( pst_inst->stCdmxCtrl.ulParsingMode & CDMX_MARKER_MEDIA_SCAN_MODE_BIT )
		{
			STEP( "\tDMXOP_OPEN_FOR_MEDIASCAN 01\n" );
			ret = pst_inst->pfnDmxLib( DMXOP_OPEN_FOR_MEDIASCAN, &pst_inst->hDmxHandle, &pst_inst->stDmxInit, &pst_inst->stOggInfo );
			STEP( "\tDMXOP_OPEN_FOR_MEDIASCAN 02\n" );
		}
		else
		{
			STEP( "\tDMXOP_OPEN 01\n" );
			ret = pst_inst->pfnDmxLib( DMXOP_OPEN, &pst_inst->hDmxHandle, &pst_inst->stDmxInit, &pst_inst->stOggInfo );
			STEP( "\tDMXOP_OPEN 02\n" );
		}

		if( ret < 0 )
		{
			ERROR( "[%ld] Error: DMXOP_OPEN failed (%d)\n", ret, __LINE__ );
            return ret;
        }

		STEP( "DMXOP_OPEN: END\n" );

		///////////////////////////////////////////////////////////////////////////////
		//
		// copy media info. to common structures
		//
		ret = ogg_set_stream_info ( pst_inst );
		if( ret < 0 )
		{
			ERROR( "[%ld] Error: ogg_set_stream_info failed (%d)\n", ret, __LINE__ );
            return ret;
        }

		///////////////////////////////////////////////////////////////////////////////
		//
		// set control variables & ..
		//
		{
			ogg_cdmx_file_info_t  *pst_finfo = pst_inst->stDmxInfo.m_pstFileInfo;
			ogg_cdmx_video_info_t *pst_vinfo = pst_inst->stDmxInfo.m_pstDefaultVideoInfo;

			if( pst_finfo->m_pszFileName != pst_inst->stDmxInit.m_pOpenFileName )
				pst_finfo->m_pszFileName = pst_inst->stDmxInit.m_pOpenFileName;
			pst_inst->stCdmxCtrl.bSeekable = pst_finfo->m_bSeekable;

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
							 //sizeof(*pst_inst->stDmxInfo.m_pstDefaultVideoInfo), 
							 //sizeof(*pst_inst->stDmxInfo.m_pstDefaultAudioInfo), 
							 sizeof(ogg_cdmx_video_info_t),
							 sizeof(ogg_cdmx_audio_info_t),
							 0, 
							 0 );

			cdmx_get_stream_id_list( &pst_inst->stCdmxBuff.stStrmIdList,
									 &pst_inst->stDmxInfo, 
//									 sizeof(*pst_inst->stDmxInfo.m_pstDefaultVideoInfo), 
//									 sizeof(*pst_inst->stDmxInfo.m_pstDefaultAudioInfo), 
							 		 sizeof(ogg_cdmx_video_info_t),
							 		 sizeof(ogg_cdmx_audio_info_t),
									 0,
									 0 );
		}
		///////////////////////////////////////////////////////////////////////////////
		//
		// find_seq_header
		//
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
					   SET_CDMX_INFO_ALL);

	}//CDMX_GET_INFO
	else if( ulOpCode == CDMX_GET_INFO_SEL_STREAM )
	{
		cdmx_select_stream_t	*pst_inp_param = (cdmx_select_stream_t*)pParam1;
		cdmx_info_t				*pst_out_param = (cdmx_info_t         *)pParam2;

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
			ogg_cdmx_video_info_t* pst_vinfo = NULL;
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
						   SET_CDMX_INFO_VIDEO);
		}
		// 
		// [2] audio info
		//
		if ( pst_inp_param->lSelType & DMXTYPE_AUDIO ) 
        {
			unsigned long i = 0;
			ogg_cdmx_audio_info_t *pst_ainfo = NULL;
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
						   SET_CDMX_INFO_AUDIO);
		}
	}
	else if( ulOpCode == CDMX_SEL_STREAM )
	{
		cdmx_select_stream_t	*pst_inp_param = (cdmx_select_stream_t*)pParam1;
		cdmx_info_t				*pst_out_param = (cdmx_info_t         *)pParam2;

		ogg_dmx_sel_stream_t 	dmx_sel_strm;
		ogg_dmx_sel_info_t		dmx_sel_info;

		STEP( "In CDMX_SEL_STREAM\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		cdmx_memset(&dmx_sel_strm, 0, sizeof(dmx_sel_strm));
		cdmx_memset(&dmx_sel_info, 0, sizeof(dmx_sel_info));

		dmx_sel_strm.m_lSelType		= pst_inp_param->lSelType;
		dmx_sel_strm.m_lVideoID		= pst_inp_param->lVideoID;
		dmx_sel_strm.m_lAudioID		= pst_inp_param->lAudioID;

		//ret = pst_inst->pfnDmxLib( DMXOP_SELECT_STREAM, &pst_inst->hDmxHandle, &pst_inst->stDmxSelStream, &pst_inst->stDmxSelInfo );
		ret = pst_inst->pfnDmxLib( DMXOP_SELECT_STREAM, &pst_inst->hDmxHandle, &dmx_sel_strm, &dmx_sel_info );
		if( ret < 0 )
		{
			ERROR( "[%ld] Error: DMXOP_SELECT_STREAM failed (%d)\n", ret, __LINE__ );
			return ret;
		}

		///////////////////////////////////////////////////////////////////////////////
		//
		// copy media info. to common structures
		//
		{
			ogg_cdmx_video_info_t 	st_out_video_info;
			ogg_cdmx_audio_info_t 	st_out_audio_info;
			cdmx_memset(&st_out_video_info,	 0, sizeof(ogg_cdmx_video_info_t));
			cdmx_memset(&st_out_audio_info,	 0, sizeof(ogg_cdmx_audio_info_t));
			ret = ogg_set_stream_info_sel_stream ( pst_inst, 
												   dmx_sel_info.m_pstVideoInfo,
												   dmx_sel_info.m_pstAudioInfo,
												   &st_out_video_info,
												   &st_out_audio_info );
			if( ret < 0 )
			{
				ERROR( "[%ld] Error: mpg_get_stream_info failed (%d)\n", ret, __LINE__ );
				return ret;
			}

			set_cdmx_info (pst_out_param, pst_inst, 
						   &st_out_video_info,
						   &st_out_audio_info,
						   SET_CDMX_INFO_ALL);

			STEP( "\tCall cdmx_print_sel_info\n" );
			cdmx_print_sel_info( (av_dmx_video_info_t*)&st_out_video_info, 
								 (av_dmx_audio_info_t*)&st_out_audio_info, 
								 0, 0 );
		}

	}//CDMX_SEL_STREAM
	else if( ulOpCode == CDMX_GETSTREAM )
	{
		cdmx_input_t  *pst_inp_param = (cdmx_input_t *)pParam1;
		cdmx_output_t *pst_out_param = (cdmx_output_t*)pParam2;

		//STEP( "In CDMX_GETSTREAM\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		pst_inst->stDmxGetStream.m_pbyStreamBuff	= pst_inp_param->m_pPacketBuff;
		pst_inst->stDmxGetStream.m_lStreamBuffSize	= pst_inp_param->m_iPacketBuffSize;
		pst_inst->stDmxGetStream.m_lStreamType 		= pst_inp_param->m_iPacketType;

		ret = pst_inst->pfnDmxLib( DMXOP_GET_STREAM, &pst_inst->hDmxHandle, &pst_inst->stDmxGetStream, &pst_inst->stDmxOutStream );
		if( ret < 0 )
		{
			ERROR( "[%ld] Error: DMXOP_GET_STREAM failed (%d)\n", ret, __LINE__ );
			return ret;
		}

		if( pst_inst->stDmxOutStream.m_lStreamType == DMXTYPE_VIDEO )
		{
			// attach start code on pst_inst->stDmxOutStream.m_pbyStreamData
			ret = attach_start_code( (cdmx_inst_t*)pst_inst, 
									 (av_dmx_info_t*)&pst_inst->stDmxInfo,
									 (av_dmx_outstream_t*)&pst_inst->stDmxOutStream, NULL );
			if(ret < 0) 
			{
				ALOGE( "[%ld] Error: in attach_start_code\n", ret );
				return ret;
			}
		}

		pst_out_param->m_iPacketSize = pst_inst->stDmxOutStream.m_lStreamDataSize;
		pst_out_param->m_iPacketType = pst_inst->stDmxOutStream.m_lStreamType;
		pst_out_param->m_iTimeStamp  = pst_inst->stDmxOutStream.m_lTimeStamp;

		if( (pst_inst->stDmxOutStream.m_lStreamType == DMXTYPE_AUDIO) && (pst_inp_param->m_pPacketBuff_ext1 != NULL))
		{
			cdmx_memcpy(pst_inp_param->m_pPacketBuff_ext1, pst_inst->stDmxOutStream.m_pbyStreamData, pst_inst->stDmxOutStream.m_lStreamDataSize);
			pst_out_param->m_pPacketData = pst_inp_param->m_pPacketBuff_ext1;
		}
		else
		{
			pst_out_param->m_pPacketData = pst_inst->stDmxOutStream.m_pbyStreamData;
		}

	}//CDMX_GETSTREAM
	else if( ulOpCode == CDMX_SEEK )
	{
		cdmx_seek_t		*pst_inp_param = (cdmx_seek_t  *)pParam1;
		cdmx_output_t	*pst_out_param = (cdmx_output_t*)pParam2;

		STEP( "In CDMX_SEEK\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		if(pst_inp_param->m_uiSeekMode == CDMX_SEEKMODE_RELATIVE_TIME)
			pst_inst->stDmxSeek.m_ulSeekMode = OGG_SEEKMODE_RELATIVE_TIME;
		else if(pst_inp_param->m_uiSeekMode == CDMX_SEEKMODE_ABSOLUTE_TIME)
			pst_inst->stDmxSeek.m_ulSeekMode = OGG_SEEKMODE_ABSOLUTE_TIME;

		pst_inst->stDmxSeek.m_lSeekTime = pst_inp_param->m_iSeekTime;

		ret = pst_inst->pfnDmxLib( DMXOP_SEEK, &pst_inst->hDmxHandle, &pst_inst->stDmxSeek, &pst_inst->stDmxOutStream );
		if( ret < 0 )
		{
			ERROR( "[%ld] Error: DMXOP_SEEK failed (%d)\n", ret, __LINE__ );
			return ret;
		}

		if( pst_out_param )
		{
			STEP( "CDMX_SEEK Success\n" );
			pst_out_param->m_iTimeStamp  = pst_inst->stDmxOutStream.m_lTimeStamp;
		}
		// exceptional condition: need a seq.header.
		pst_inst->stCdmxCtrl.bIsNotFirstFrame = FALSE;
	}
	else if( ulOpCode == CDMX_CLOSE )
	{
		STEP( "In CDMX_CLOSE\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		ret = pst_inst->pfnDmxLib( DMXOP_CLOSE, &pst_inst->hDmxHandle, NULL, NULL );
		if( ret < 0 )
		{
			ERROR( "[%ld] Error: DMXOP_CLOSE failed (%d)\n", ret, __LINE__ );
			return ret;
		}

		// ogg only
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

		// common
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

#endif//INCLUDE_OGG_DMX
