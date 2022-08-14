/****************************************************************************
 *   FileName    : cdmx_mkv.c
 *   Description : container demuxer of MKV
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

#ifdef INCLUDE_MKV_DMX

#include "../../cdk/cdk.h"
#include "../cdmx.h"
#include <cutils/properties.h>




//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	LOG
//
#define LOG_TAG    "CDMX_MKV"

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

#define cdmx_mkv_malloc		((*pst_inst).stDmxInit.m_stMemoryFuncs).m_pfnMalloc
#define cdmx_mkv_free		((*pst_inst).stDmxInit.m_stMemoryFuncs).m_pfnFree
#define cdmx_mkv_memset		((*pst_inst).stDmxInit.m_stMemoryFuncs).m_pfnMemset
#define cdmx_mkv_memcpy		((*pst_inst).stDmxInit.m_stMemoryFuncs).m_pfnMemcpy
#define cdmx_mkv_memmove	((*pst_inst).stDmxInit.m_stMemoryFuncs).m_pfnMemmove
#define cdmx_mkv_realloc	((*pst_inst).stDmxInit.m_stMemoryFuncs).m_pfnRealloc

#define cdmx_malloc 	cdmx_mkv_malloc
#define cdmx_free 		cdmx_mkv_free
#define cdmx_memset 	cdmx_mkv_memset
#define cdmx_memcpy 	cdmx_mkv_memcpy
#define cdmx_memmove	cdmx_mkv_memmove
#define cdmx_realloc 	cdmx_mkv_realloc

static void
print_dmx_metadata( 
   mkv_dmx_inst_t* pstInst 
   )
{
	unsigned int i;
	av_metadata_t* pst_meta;
	int index_flag = 0;

	mkv_dmx_info_t* pst_dmx_info = &pstInst->stDmxInfo;
	mkv_dmx_metadata_t* pst_meta_info = &pstInst->stDmxMeta;
	pstInst->stCdmxCtrl.bHasIdxTable = 0;
	pstInst->stCdmxCtrl.lLastKeyTimeStamp = 0;

	DSTATUS( "--------------------------------\n" );
	DSTATUS( " META DATA\n" );
	DSTATUS( "--------------------------------\n" );

	for( i = 0; i < pst_meta_info->m_ulMetaTotal; ++i )
	{
		pst_meta = &pst_meta_info->m_pstMetaList[i];

		switch( pst_meta->m_ulDataID )
		{
		case META_ID_COPYRIGHT:
		case META_ID_CREATION_TIME:
		case META_ID_PROFILE_LEVEL:
		case META_ID_CODEC_NAME:
			{
				if( pst_meta->m_ulDataType == META_TYPE_STRING )
				{
					DSTATUS( "--------------------------------\n" );
					DSTATUS( " CODEC TYPE[%d] \t: %s\n", i, pst_meta->m_unData.pszString );
					DSTATUS( "--------------------------------\n" );
				}
			}
			break;
		case META_ID_MANUFACTURER:
		case META_ID_LANGUAGE:
		case META_ID_ARTIST:
		case META_ID_ALBUM_TITLE:
		case META_ID_VENDOR_NAME:
			break;
		case META_ID_VKEY_TOTAL:
			{
				double avg_dis = 0;
				//-------------------------------------------------------------------------------------------------------
				//		INDEX INFORMATION
				//-------------------------------------------------------------------------------------------------------
				DSTATUS( "--------------------------------\n" );
				DSTATUS( " INDEX INFORMATION\n" );
				DSTATUS( "--------------------------------\n" );

				if( pst_meta->m_ulDataType == META_TYPE_SIGNED_LONG )
					index_flag = pst_meta->m_unData.lValue;
				else if( pst_meta->m_ulDataType == META_TYPE_UNSIGNED_LONG )
					index_flag = pst_meta->m_unData.ulValue;

				if( index_flag )
				{
					DSTATUS( " INDEX STATUS \t: INDEX EXISTS.\n" );

					DSTATUS( " TOTAL INDEX NUMBER : %d\n", index_flag );
					avg_dis = ( pst_dmx_info->m_pstFileInfo->m_ulDuration/1000.0 ) / (index_flag-1);
					DSTATUS( " AVG DURATION OF INDEX \t: %.3lf	Sec\n", avg_dis );
					DSTATUS( "\n" );
					// TODO: add your code here.
				}
			}
			break;

		case META_ID_VKEY_LAST_TIMESTAMP:
			{
				if( pst_meta->m_unData.ulValue )
				{
					double ul_lastkeytime_msec;
					int hour, minutes, sec;

					pstInst->stCdmxCtrl.lLastKeyTimeStamp = pst_meta->m_unData.ulValue;

					ul_lastkeytime_msec = pst_meta->m_unData.ulValue/1000.0;
					hour = (int)( ul_lastkeytime_msec / 3600.0 );
					if( hour )
					{
						ul_lastkeytime_msec -= 3600;
					}
					minutes = (int)( ul_lastkeytime_msec / 60.0 );
					sec = (int)(ul_lastkeytime_msec) % 60;

					// Timestamp of Last Key Frame
					DSTATUS( "--------------------------------\n" );
					DSTATUS( " LAST KEY TIMESTAMP : %02d:%02d:%02d   (%.3lf Sec)\n", hour, minutes, sec, pst_meta->m_unData.ulValue/1000.0 );
					DSTATUS( "--------------------------------\n" );
					// TODO: add your code here.
				}
			}
			break;
		case META_ID_DISPLAY_WIDTH:
			if( pst_meta->m_unData.ulValue )
			{
				// Display width for AspectRatio
				DSTATUS( "--------------------------------\n" );
				DSTATUS( " Display Width  : %4ld \n", pst_meta->m_unData.ulValue );
				DSTATUS( "--------------------------------\n" );
				// TODO: add your code here.
			}
			break;
		case META_ID_DISPLAY_HEIGHT:
			if( pst_meta->m_unData.ulValue )
			{
				// Display height for AspectRatio
				DSTATUS( "--------------------------------\n" );
				DSTATUS( " Display Height : %4ld \n", pst_meta->m_unData.ulValue );
				DSTATUS( "--------------------------------\n" );
				// TODO: add your code here.
			}
			break;
		}
	}

	if( index_flag == 0 )
	{
		DSTATUS( " INDEX STATUS \t: INDEX DOES NOT EXIST.\n" );
		return;
	}

	DSTATUS( "===================================================================\n\n\n" );
	pstInst->stCdmxCtrl.bHasIdxTable = index_flag;
}

static 
long
find_seq_header( mkv_dmx_inst_t* pstInst )
{
	long ret = DMXRET_SUCCESS;
	mkv_dmx_inst_t* pst_inst = pstInst;

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
			pstInst->stSeqHeader.m_pSeqHeaderData	 = pstInst->stDmxInfo.m_pstDefaultVideoInfo->m_pbyCodecPrivate;
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

				//ALOGI( "cdmx_get_seqhead start\n" );

				// extraction routine is available. (return == 1)
				pstInst->stSeqHeader.m_iSeqHeaderDataLen 	= 0;
				pstInst->stSeqHeader.m_pSeqHeaderData 		= NULL;

				pstInst->stDmxGetStream.m_pbyStreamBuff		= NULL;
				pstInst->stDmxGetStream.m_ulStreamBuffSize	= 0;
				pstInst->stDmxGetStream.m_ulStreamType		= DMXTYPE_VIDEO;

				while(cnt_seq_header < 300) 
				{
					//ALOGI( "[MKV] find Sequence Header:%d !! \n", cnt_seq_header );

					ret = pstInst->pfnDmxLib( DMXOP_GET_STREAM, &pstInst->hDmxHandle, &pstInst->stDmxGetStream, &pstInst->stDmxOutStream );
					if( ret < 0 )
					{
						if( ret == DMXRET_END_OF_STREAM )
						{
							ALOGE( "[MKV] Sequence Header not found !! \n" );

							if( pstInst->stDmxGetStream.m_ulStreamType == DMXTYPE_VIDEO )
								return ERR_END_OF_VIDEO_FILE;
							else if ( pstInst->stDmxGetStream.m_ulStreamType == DMXTYPE_AUDIO )
								return ERR_END_OF_AUDIO_FILE;
							else if ( pstInst->stDmxGetStream.m_ulStreamType == DMXTYPE_SUBTITLE )
								return ERR_END_OF_SUBTITLE_FILE;
							else if ( pstInst->stDmxGetStream.m_ulStreamType == DMXTYPE_NONE )
								return ERR_END_OF_FILE;
						}

						pstInst->pfnDmxLib( DMXOP_GET_LASTERROR, &pstInst->hDmxHandle, NULL, &pstInst->stDmxError );
						ALOGE( "[%ld] Error:%s, %lld\n", pstInst->stDmxError.m_lErrCode, pstInst->stDmxError.m_pszErrStatus, pstInst->stDmxError.m_llLastStatus );
						return ret;
					}

					if( pstInst->stDmxOutStream.m_ulStreamType == DMXTYPE_VIDEO )
					{
						// extract sequence header
						ret = cdmx_get_seqhead( pstInst->stDmxOutStream.m_pbyStreamData, 
												pstInst->stDmxOutStream.m_ulStreamDataSize,
												&pstInst->stCdmxBuff.pbySeqHeaderBuff,
												&pstInst->stCdmxBuff.ulSeqHeaderBuffSize, 
												pstInst->stDmxInfo.m_pstDefaultVideoInfo->m_ulFourCC );
						if( ret < 0 ) return ret;
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
							ERROR( "\t\t Seek Failed(%d) \n", ret );
							pstInst->pfnDmxLib( DMXOP_GET_LASTERROR, &pstInst->hDmxHandle, NULL, &pstInst->stDmxError );
							ALOGE( "[%ld] Error:%s, %lld\n", pstInst->stDmxError.m_lErrCode, pstInst->stDmxError.m_pszErrStatus, pstInst->stDmxError.m_llLastStatus );
							return ret;
						}

						STEP( "\t\t Get Video Frame \n" );
						while(cnt_seq_header < 300) 
						{
							//ALOGI( "[MKV] DMXOP_GET_STREAM:%d !! \n", cnt_seq_header );
							ret = pstInst->pfnDmxLib( DMXOP_GET_STREAM, &pstInst->hDmxHandle, &pstInst->stDmxGetStream, &pstInst->stDmxOutStream );
							if( ret < 0 )
							{
								if( ret == DMXRET_END_OF_STREAM )
								{
									ALOGE( "[MKV] Sequence Header not found !! \n" );
									if( pstInst->stDmxGetStream.m_ulStreamType == DMXTYPE_VIDEO )
										return ERR_END_OF_VIDEO_FILE;
									else if ( pstInst->stDmxGetStream.m_ulStreamType == DMXTYPE_AUDIO )
										return ERR_END_OF_AUDIO_FILE;
									else if ( pstInst->stDmxGetStream.m_ulStreamType == DMXTYPE_SUBTITLE )
										return ERR_END_OF_SUBTITLE_FILE;
									else if ( pstInst->stDmxGetStream.m_ulStreamType == DMXTYPE_NONE )
										return ERR_END_OF_FILE;
								}
								pstInst->pfnDmxLib( DMXOP_GET_LASTERROR, &pstInst->hDmxHandle, NULL, &pstInst->stDmxError );
								ALOGE( "[%ld] Error:%s, %lld\n", pstInst->stDmxError.m_lErrCode, pstInst->stDmxError.m_pszErrStatus, pstInst->stDmxError.m_llLastStatus );
								return ret;
							}

							if( pstInst->stDmxOutStream.m_ulStreamType == DMXTYPE_VIDEO )
							{
								if( pstInst->stDmxOutStream.m_ulStreamDataSize )
								{
									//ALOGI( "[MKV] call make_mpeg4_vol_header() !! \n" );
									ret = make_mpeg4_vol_header(
												 pstInst->stDmxOutStream.m_pbyStreamData,
												 pstInst->stDmxOutStream.m_ulStreamDataSize,
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
			}// end of if( cdmx_get_seqhead )
		}//if( (pstInst->stCdmxCtrl.ulParsingMode & CDMX_MARKER_MEDIA_SCAN_MODE_BIT) == 0 )
	}// end of else

	STEP( "\tm_ulCodecPrivateSize: %d, m_iSeqHeaderDataLen: %d \n", pstInst->stDmxInfo.m_pstDefaultVideoInfo->m_ulCodecPrivateSize, pstInst->stSeqHeader.m_iSeqHeaderDataLen );
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
    mkv_dmx_inst_t			*pstInst,
	mkv_dmx_video_info_t	*pstVideoInfo,
	mkv_dmx_audio_info_t	*pstAudioInfo,
	mkv_dmx_subtitle_info_t	*pstSubtitleInfo,
	unsigned long 	 		 ulType
	)
{
	mkv_dmx_inst_t			*pst_inst    = pstInst;
	cdmx_info_t				*pst_out_info= pstCdmxInfo;
	mkv_dmx_file_info_t		*pst_finfo	 = pst_inst->stDmxInfo.m_pstFileInfo;
	mkv_dmx_video_info_t	*pst_vinfo	 = pstVideoInfo;
	mkv_dmx_audio_info_t	*pst_ainfo	 = pstAudioInfo;
	mkv_dmx_subtitle_info_t	*pst_sinfo	 = pstSubtitleInfo;
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
		pst_out_info->m_sFileInfo.m_iSeekable			= pst_finfo->m_ulInfoFlags&MKVDINFO_SEEKABLE;
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
		pst_out_info->m_sVideoInfo.m_pszCodecName 			= NULL;
		pst_out_info->m_sVideoInfo.m_pszCodecVendorName 	= NULL;
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
		pst_out_info->m_sVideoInfo.m_iBitRate				= 0;
		if ( pst_finfo->m_ulDuration > 0 ) {
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
		pst_out_info->m_sVideoInfo.m_ulLastKeyTimestamp  	= pst_vinfo->m_ulLastKeyFrameTimeStamp;
		pst_out_info->m_sVideoInfo.m_ulStereoMode       	= pst_vinfo->m_ulStereoMode;
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
		pst_out_info->m_sAudioInfo.m_iBlockAlign		= pst_ainfo->m_ulBlockAlign;
		pst_out_info->m_sAudioInfo.m_iAvgBytesPerSec	= pst_ainfo->m_ulAvgBytesPerSec;
		pst_out_info->m_sAudioInfo.m_iAudioStreamID		= 0;
		pst_out_info->m_sAudioInfo.m_iBitRate			= 8 * pst_ainfo->m_ulAvgBytesPerSec;
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
	// 
	// [4] subtitle info
	//
	if ( ( ulType & SET_CDMX_INFO_SUBTITLE ) && ( pst_sinfo != NULL ) )
	{
		//FUNC("In set_cdmx_info: SET_CDMX_INFO_SUBTITLE \n");
		cdmx_memset( &pst_out_info->m_sSubtitleInfo, 0, sizeof(pst_out_info->m_sSubtitleInfo) );
		pst_out_info->m_sSubtitleInfo.m_iIsText				= 1;
		pst_out_info->m_sSubtitleInfo.m_iNumSubtitleStream	= pst_inst->stDmxInfo.m_ulSubtitleStreamTotal;
		pst_out_info->m_sSubtitleInfo.m_iCurrSubtitleIdx	= pst_sinfo->m_ulStreamID;
		pst_out_info->m_sSubtitleInfo.m_pszLanguage			= pst_sinfo->m_pszLanguage;
		pst_out_info->m_sSubtitleInfo.m_iPID 				= 0;
		pst_out_info->m_sSubtitleInfo.m_iCharEnc 			= 0;
		pst_out_info->m_sSubtitleInfo.m_iGraphicsType 		= 0;
		pst_out_info->m_sSubtitleInfo.m_iTotalNumber 		= 0;
		pst_out_info->m_sSubtitleInfo.m_iTrackIndexCache 	= 0;
		pst_out_info->m_sSubtitleInfo.m_pInfo				= pst_sinfo->m_pSubtitleInfo;
		pst_out_info->m_sSubtitleInfo.m_iInfoSize			= pst_sinfo->m_ulSubtitleInfoSize;
	}

}

av_result_t 
cdmx_mkv( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 )
{
	av_result_t ret = DMXRET_SUCCESS;
	mkv_dmx_inst_t* pst_inst = (mkv_dmx_inst_t*)(*ptHandle); // assume that its memory is allocated.

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
		//--------------------------------------------------
		// Create main handle
		//--------------------------------------------------
		//pst_inst = (dmx_inst_t*)pst_callback->m_pfMalloc( sizeof(dmx_inst_t) );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;
		//pst_callback->m_pfMemset(pst_inst, 0, sizeof(dmx_inst_t));

		{
		#ifdef CDMX_USE_DYNAMIC_LOADING
			void* pv_dmx;
			void* pv_handle;
			ret = open_library( (const char*)"/system/lib/libff1.so",
								(const char*)"TCC_MKV_DMX",
								&pv_dmx,
								&pv_handle );
			if (ret < 0) {
				return ret;
			}
			pst_inst->pfnDmxLib = ( av_result_t (*) (unsigned long, av_handle_t*, void*, void*) )(pv_dmx);
			pst_inst->pvLibHandle = pv_handle;
			STEP( "LibHandle loaded....(0x%x) (handle:0x%x) ...\n", pst_inst->pfnDmxLib, pst_inst->pvLibHandle );
		#else
			pst_inst->pfnDmxLib   = TCC_MKV_DMX;
			pst_inst->pvLibHandle = NULL;
		#endif
		}

		pst_inst->stCdmxCtrl.ulParsingMode = pst_init_param->ulParsingMode;
		//--------------------------------------------------
		// set init parameters
		//--------------------------------------------------
		pst_inst->stDmxInit.m_pszOpenFileName = pst_init_param->pszFileName;
		
		pst_inst->stDmxInit.m_stMemoryFuncs.m_pfnMalloc		= pst_callback->m_pfMalloc;
		pst_inst->stDmxInit.m_stMemoryFuncs.m_pfnFree		= pst_callback->m_pfFree;
		pst_inst->stDmxInit.m_stMemoryFuncs.m_pfnMemcmp		= pst_callback->m_pfMemcmp;
		pst_inst->stDmxInit.m_stMemoryFuncs.m_pfnMemcpy		= pst_callback->m_pfMemcpy;
		pst_inst->stDmxInit.m_stMemoryFuncs.m_pfnMemset		= pst_callback->m_pfMemset;
		pst_inst->stDmxInit.m_stMemoryFuncs.m_pfnRealloc	= pst_callback->m_pfRealloc;
		pst_inst->stDmxInit.m_stMemoryFuncs.m_pfnMemmove	= pst_callback->m_pfMemmove;

		pst_inst->stDmxInit.m_stFileFuncs.m_pfnFopen		= pst_callback->m_pfFopen;
		pst_inst->stDmxInit.m_stFileFuncs.m_pfnFread		= pst_callback->m_pfFread;
		pst_inst->stDmxInit.m_stFileFuncs.m_pfnFseek		= pst_callback->m_pfFseek;
		pst_inst->stDmxInit.m_stFileFuncs.m_pfnFtell		= pst_callback->m_pfFtell;
		pst_inst->stDmxInit.m_stFileFuncs.m_pfnFwrite		= pst_callback->m_pfFwrite;
		pst_inst->stDmxInit.m_stFileFuncs.m_pfnFclose		= pst_callback->m_pfFclose;
		pst_inst->stDmxInit.m_stFileFuncs.m_pfnFeof			= pst_callback->m_pfFeof;
		pst_inst->stDmxInit.m_stFileFuncs.m_pfnFflush		= pst_callback->m_pfFflush;
		pst_inst->stDmxInit.m_stFileFuncs.m_pfnFseek64		= pst_callback->m_pfFseek64;
		pst_inst->stDmxInit.m_stFileFuncs.m_pfnFtell64		= pst_callback->m_pfFtell64;
		cdmx_memory_func_init(&pst_inst->stDmxInit.m_stMemoryFuncs);

        pst_inst->stDmxInit.m_ulVideoFileCacheSize = 4*1024;
		pst_inst->stDmxInit.m_ulAudioFileCacheSize = 4*1024;
		pst_inst->stDmxInit.m_ulSeekFileCacheSize  = 1024;

		if ( pst_inst->stCdmxCtrl.ulParsingMode & CDMX_MARKER_MEDIA_SCAN_MODE_BIT )
		{
			STEP( "\tDMXOP_OPEN_FOR_MEDIASCAN 01\n" );
			ret = pst_inst->pfnDmxLib( DMXOP_OPEN_FOR_MEDIASCAN, &pst_inst->hDmxHandle, &pst_inst->stDmxInit, &pst_inst->stDmxInfo );
			STEP( "\tDMXOP_OPEN_FOR_MEDIASCAN 02\n" );
		}
		else
		{
			STEP( "\tDMXOP_OPEN 01\n" );
			ret = pst_inst->pfnDmxLib( DMXOP_OPEN, &pst_inst->hDmxHandle, &pst_inst->stDmxInit, &pst_inst->stDmxInfo );
			STEP( "\tDMXOP_OPEN 02\n" );
		}

		if( ret < 0 )
		{
			pst_inst->pfnDmxLib( DMXOP_GET_LASTERROR, &pst_inst->hDmxHandle, NULL, &pst_inst->stDmxError );
			ERROR( "[%ld] Error:%s, %lld\n", pst_inst->stDmxError.m_lErrCode, pst_inst->stDmxError.m_pszErrStatus, pst_inst->stDmxError.m_llLastStatus );
            return ret;
        }

		STEP( "DMXOP_OPEN: END\n" );

        if( (pst_inst->stCdmxCtrl.ulParsingMode & CDMX_MARKER_MEDIA_SCAN_MODE_BIT) == 0 )
		{
			//--------------------------------------------------------
			// [*] MKVDOP_GET_VERSION operation
			// --------------------------------------------------------
			char *psz_version = NULL;
			char *psz_build_date = NULL;

			//STEP( "\tMKVDOP_GET_VERSION: BEGIN\n" );

			ret = pst_inst->pfnDmxLib( MKVDOP_GET_VERSION, &pst_inst->hDmxHandle, &psz_version, &psz_build_date );
			if (ret < 0) {
				pst_inst->pfnDmxLib( DMXOP_GET_LASTERROR, &pst_inst->hDmxHandle, NULL, &pst_inst->stDmxError );
				ERROR( "[%ld] Error:%s, %lld\n", pst_inst->stDmxError.m_lErrCode, pst_inst->stDmxError.m_pszErrStatus, pst_inst->stDmxError.m_llLastStatus );
				return ret;
			}
			INFO("%s, %s\n", psz_version, psz_build_date);

			//STEP( "\tMKVDOP_GET_VERSION: END\n" );
		}

		///////////////////////////////////////////////////////////////////////////////
		//
		// set control variables & ..
		//
		{
			mkv_dmx_file_info_t	 *pst_finfo = pst_inst->stDmxInfo.m_pstFileInfo;
			mkv_dmx_video_info_t *pst_vinfo = pst_inst->stDmxInfo.m_pstDefaultVideoInfo;

			if( pst_finfo->m_pszFileName != pst_inst->stDmxInit.m_pszOpenFileName )
				pst_finfo->m_pszFileName = pst_inst->stDmxInit.m_pszOpenFileName;
			pst_inst->stCdmxCtrl.bSeekable = (pst_finfo->m_ulInfoFlags&MKVDINFO_SEEKABLE);

			if( pst_inst->stDmxInfo.m_ulVideoStreamTotal )
			{
				pst_inst->stCdmxCtrl.lLastKeyTimeStamp = pst_vinfo->m_ulLastKeyFrameTimeStamp;

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

			#if 0
			{
				//---------------------------------------------------------
				// [*] DMXOP_GET_METADATA operation
				//---------------------------------------------------------
				ret = pst_inst->pfnDmxLib( DMXOP_GET_METADATA, &pst_inst->hDmxHandle, NULL, &pst_inst->stDmxMeta );
				if( ret < 0 )
				{
					pst_inst->pfnDmxLib( DMXOP_GET_LASTERROR, &pst_inst->hDmxHandle, NULL, &pst_inst->stDmxError );
					ALOGE( "[%ld] Error:%s, %lld\n", pst_inst->stDmxError.m_lErrCode, pst_inst->stDmxError.m_pszErrStatus, pst_inst->stDmxError.m_llLastStatus );
					return ret;
				}
				print_dmx_metadata( pst_inst );

				// when index table does not exist, we can create it.
				// host option...if you want to create index
				#if 0
				if( !pst_inst->stCdmxCtrl.bHasIdxTable )
				{
					ret = pst_inst->pfnDmxLib( MKVDOP_CREATE_INDEX, &pst_inst->hDmxHandle, NULL, NULL );
					ret = pst_inst->pfnDmxLib( DMXOP_GET_METADATA, &pst_inst->hDmxHandle, NULL, &pst_inst->stDmxMeta );
					if( ret < 0 )
					{
						pst_inst->pfnDmxLib( DMXOP_GET_LASTERROR, &pst_inst->hDmxHandle, NULL, &pst_inst->stDmxError );
						ALOGE( "[%ld] Error:%s, %lld\n", pst_inst->stDmxError.m_lErrCode, pst_inst->stDmxError.m_pszErrStatus, pst_inst->stDmxError.m_llLastStatus );
						return ret;
					}
					print_dmx_metadata( pst_inst );
				}
				#endif
			}
			#endif
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
					   pst_inst->stDmxInfo.m_pstDefaultSubtitleInfo,
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
			mkv_dmx_video_info_t* pst_vinfo = NULL;
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
			mkv_dmx_audio_info_t *pst_ainfo = NULL;
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
			mkv_dmx_subtitle_info_t	*pst_sinfo = NULL;
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
		cdmx_select_stream_t	*pst_inp_param = (cdmx_select_stream_t*)pParam1;
		cdmx_info_t				*pst_out_param = (cdmx_info_t         *)pParam2;

		mkv_dmx_sel_stream_t 	dmx_sel_strm;
		mkv_dmx_sel_info_t		dmx_sel_info;

		STEP( "In CDMX_SEL_STREAM\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		cdmx_memset(&dmx_sel_strm, 0, sizeof(dmx_sel_strm));
		cdmx_memset(&dmx_sel_info, 0, sizeof(dmx_sel_info));

		dmx_sel_strm.m_ulSelType		= pst_inp_param->lSelType;
		dmx_sel_strm.m_ulVideoID		= pst_inp_param->lVideoID;
		dmx_sel_strm.m_ulAudioID		= pst_inp_param->lAudioID;
		dmx_sel_strm.m_ulSubtitleID	= pst_inp_param->lSubtitleID;

		//ret = pst_inst->pfnDmxLib( DMXOP_SELECT_STREAM, &pst_inst->hDmxHandle, &pst_inst->stDmxSelStream, &pst_inst->stDmxSelInfo );
		ret = pst_inst->pfnDmxLib( DMXOP_SELECT_STREAM, &pst_inst->hDmxHandle, &dmx_sel_strm, &dmx_sel_info );
		if( ret < 0 )
		{
			pst_inst->pfnDmxLib( DMXOP_GET_LASTERROR, &pst_inst->hDmxHandle, NULL, &pst_inst->stDmxError );
			ALOGE( "[%ld] Error:%s, %lld\n", pst_inst->stDmxError.m_lErrCode, pst_inst->stDmxError.m_pszErrStatus, pst_inst->stDmxError.m_llLastStatus );
			return ret;
		}

		set_cdmx_info (pst_out_param, pst_inst, 
					   dmx_sel_info.m_pstVideoInfo,
					   dmx_sel_info.m_pstAudioInfo,
					   dmx_sel_info.m_pstSubtitleInfo,
					   SET_CDMX_INFO_ALL);

		STEP( "\tCall cdmx_print_sel_info\n" );
		cdmx_print_sel_info( (av_dmx_video_info_t*)dmx_sel_info.m_pstVideoInfo, 
							 (av_dmx_audio_info_t*)dmx_sel_info.m_pstAudioInfo, 
							 (av_dmx_subtitle_info_t*)dmx_sel_info.m_pstSubtitleInfo, 0 );

	}//CDMX_SEL_STREAM
	else if( ulOpCode == CDMX_GETSTREAM )
	{
		cdmx_input_t  *pst_inp_param = (cdmx_input_t *)pParam1;
		cdmx_output_t *pst_out_param = (cdmx_output_t*)pParam2;

		//STEP( "In CDMX_GETSTREAM\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		pst_inst->stDmxGetStream.m_pbyStreamBuff	= pst_inp_param->m_pPacketBuff;
		pst_inst->stDmxGetStream.m_ulStreamBuffSize	= pst_inp_param->m_iPacketBuffSize;

	#if 0
		switch( pst_inp_param->m_iPacketType )
		{
		case PACKET_VIDEO:
			pst_inst->stDmxGetStream.m_ulStreamType = DMXTYPE_VIDEO;
			break;
		case PACKET_AUDIO:
			pst_inst->stDmxGetStream.m_ulStreamType = DMXTYPE_AUDIO;
			break;
		case PACKET_SUBTITLE:
			pst_inst->stDmxGetStream.m_ulStreamType = DMXTYPE_SUBTITLE;
			break;
		}
	#else
		pst_inst->stDmxGetStream.m_ulStreamType = pst_inp_param->m_iPacketType;
	#endif

		ret = pst_inst->pfnDmxLib( DMXOP_GET_STREAM, &pst_inst->hDmxHandle, &pst_inst->stDmxGetStream, &pst_inst->stDmxOutStream );
		if( ret < 0 )
		{
			if( ret == DMXRET_END_OF_STREAM )
			{
				STEP( "\tDMXRET_END_OF_STREAM\n" );
				if( pst_inst->stDmxGetStream.m_ulStreamType == DMXTYPE_VIDEO )
					return ERR_END_OF_VIDEO_FILE;
				else if ( pst_inst->stDmxGetStream.m_ulStreamType == DMXTYPE_AUDIO )
					return ERR_END_OF_AUDIO_FILE;
				else if ( pst_inst->stDmxGetStream.m_ulStreamType == DMXTYPE_SUBTITLE )
					return ERR_END_OF_SUBTITLE_FILE;
				else if ( pst_inst->stDmxGetStream.m_ulStreamType == DMXTYPE_NONE )
					return ERR_END_OF_FILE;
			}

			pst_inst->pfnDmxLib( DMXOP_GET_LASTERROR, &pst_inst->hDmxHandle, NULL, &pst_inst->stDmxError );
			ALOGE( "[%ld] Error:%s, %lld\n", pst_inst->stDmxError.m_lErrCode, pst_inst->stDmxError.m_pszErrStatus, pst_inst->stDmxError.m_llLastStatus );
			return ret;
		}

		pst_inst->stCdmxCtrl.bIsKeyframe = *((long*)pst_inst->stDmxOutStream.m_pSpecificData);
		if( pst_inst->stDmxOutStream.m_ulStreamType == DMXTYPE_VIDEO )
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

		pst_out_param->m_iPacketSize = pst_inst->stDmxOutStream.m_ulStreamDataSize;
		pst_out_param->m_iPacketType = pst_inst->stDmxOutStream.m_ulStreamType;
		pst_out_param->m_iTimeStamp  = pst_inst->stDmxOutStream.m_ulTimeStamp;
		pst_out_param->m_bKeyFrame   = pst_inst->stCdmxCtrl.bIsKeyframe;

		if( (pst_inst->stDmxOutStream.m_ulStreamType == DMXTYPE_AUDIO) && (pst_inp_param->m_pPacketBuff_ext1 != NULL))
		{
			cdmx_memcpy(pst_inp_param->m_pPacketBuff_ext1, pst_inst->stDmxOutStream.m_pbyStreamData, pst_inst->stDmxOutStream.m_ulStreamDataSize);
			pst_out_param->m_pPacketData = pst_inp_param->m_pPacketBuff_ext1;
		}
		else if( (pst_inst->stDmxOutStream.m_ulStreamType == DMXTYPE_SUBTITLE) && (pst_inp_param->m_pPacketBuff_ext2 != NULL))
		{
			cdmx_memcpy(pst_inp_param->m_pPacketBuff_ext2, pst_inst->stDmxOutStream.m_pbyStreamData, pst_inst->stDmxOutStream.m_ulStreamDataSize);
			pst_out_param->m_pPacketData = pst_inp_param->m_pPacketBuff_ext2;
		}
		else
		{
			pst_out_param->m_pPacketData = pst_inst->stDmxOutStream.m_pbyStreamData;
		}

		// INTERNAL_SUBTITLE_INCLUDE
		if( pst_inst->stDmxOutStream.m_ulStreamType == DMXTYPE_SUBTITLE )	// Richard subtitle_exception
		{
			// ALOGD("cdmx DMXTYPE_SUBTITLE, size(%d), stm(%d), etm(%d), fmt(%d)", pst_inst->stDmxOutStream.m_ulStreamDataSize, pst_inst->stDmxOutStream.m_ulTimeStamp, pst_inst->stDmxOutStream.m_ulEndTimeStamp,pst_inst->stDmxInfo.m_pstDefaultSubtitleInfo->m_ulSubtitleType);
			pst_out_param->m_iSubtitleType = pst_inst->stDmxInfo.m_pstDefaultSubtitleInfo->m_ulSubtitleType;
			pst_out_param->m_iEndTimeStamp = pst_inst->stDmxOutStream.m_ulEndTimeStamp;
		}

	}//CDMX_GETSTREAM
	else if( ulOpCode == CDMX_SEEK )
	{
		cdmx_seek_t		*pst_inp_param = (cdmx_seek_t  *)pParam1;
		cdmx_output_t	*pst_out_param = (cdmx_output_t*)pParam2;

		STEP( "In CDMX_SEEK(%d)\n", pst_inp_param->m_iSeekTime );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		if(pst_inp_param->m_uiSeekMode == CDMX_SEEKMODE_RELATIVE_TIME)
			pst_inst->stDmxSeek.m_ulSeekMode = DMXSEEK_RELATIVE_TIME;
		else if(pst_inp_param->m_uiSeekMode == CDMX_SEEKMODE_ABSOLUTE_TIME)
			pst_inst->stDmxSeek.m_ulSeekMode = DMXSEEK_DEFAULT;

		pst_inst->stDmxSeek.m_lSeekTime = pst_inp_param->m_iSeekTime;

		ret = pst_inst->pfnDmxLib( DMXOP_SEEK, &pst_inst->hDmxHandle, &pst_inst->stDmxSeek, &pst_inst->stDmxOutStream );
		if( ret < 0 )
		{
			if( ret == DMXRET_END_OF_STREAM )
			{
				if( pst_inst->stDmxGetStream.m_ulStreamType == DMXTYPE_VIDEO )
					return ERR_END_OF_VIDEO_FILE;
				else if ( pst_inst->stDmxGetStream.m_ulStreamType == DMXTYPE_AUDIO )
					return ERR_END_OF_AUDIO_FILE;
				else if ( pst_inst->stDmxGetStream.m_ulStreamType == DMXTYPE_SUBTITLE )
					return ERR_END_OF_SUBTITLE_FILE;
				else if ( pst_inst->stDmxGetStream.m_ulStreamType == DMXTYPE_NONE )
					return ERR_END_OF_FILE;
			}

			pst_inst->pfnDmxLib( DMXOP_GET_LASTERROR, &pst_inst->hDmxHandle, NULL, &pst_inst->stDmxError );
			ALOGE( "[%ld] Error:%s, %lld\n", pst_inst->stDmxError.m_lErrCode, pst_inst->stDmxError.m_pszErrStatus, pst_inst->stDmxError.m_llLastStatus );
			return ret;
		}

		if( pst_out_param )
		{
			STEP( "CDMX_SEEK Success(%d)\n", pst_inst->stDmxOutStream.m_ulTimeStamp );

			pst_inst->stCdmxCtrl.bIsKeyframe = *((long*)pst_inst->stDmxOutStream.m_pSpecificData);
			pst_out_param->m_pPacketData = pst_inst->stDmxOutStream.m_pbyStreamData;
			pst_out_param->m_iPacketSize = pst_inst->stDmxOutStream.m_ulStreamDataSize;
			pst_out_param->m_iPacketType = pst_inst->stDmxOutStream.m_ulStreamType;
			pst_out_param->m_iTimeStamp  = pst_inst->stDmxOutStream.m_ulTimeStamp;
			pst_out_param->m_bKeyFrame   = pst_inst->stCdmxCtrl.bIsKeyframe;
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
			pst_inst->pfnDmxLib( DMXOP_GET_LASTERROR, &pst_inst->hDmxHandle, NULL, &pst_inst->stDmxError );
			ALOGE( "[%ld] Error:%s, %lld\n", pst_inst->stDmxError.m_lErrCode, pst_inst->stDmxError.m_pszErrStatus, pst_inst->stDmxError.m_llLastStatus );
			return ret;
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

		//--------------------------------------------------
		// Delete main handle
		//--------------------------------------------------
		//cdmx_free(pst_inst);
		//pst_inst = NULL;
	}

	return ret;
}

#endif//INCLUDE_MKV_DMX
