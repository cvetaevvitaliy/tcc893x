/****************************************************************************
 *   FileName    : cdmx_mpg.c
 *   Description : container demuxer of MPG
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

#ifdef INCLUDE_MPG_DMX

#include "../../cdk/cdk.h"
#include "../cdmx.h"
#include <cutils/properties.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	LOG
//
#define LOG_TAG    "CDMX_MPG"

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

#if 0
#define cdmx_mpg_malloc		((*pst_inst).stDmxInit.m_stMemoryFuncs).m_pfnMalloc
#define cdmx_mpg_free		((*pst_inst).stDmxInit.m_stMemoryFuncs).m_pfnFree
#define cdmx_mpg_memset		((*pst_inst).stDmxInit.m_stMemoryFuncs).m_pfnMemset
#define cdmx_mpg_memcpy		((*pst_inst).stDmxInit.m_stMemoryFuncs).m_pfnMemcpy
#define cdmx_mpg_memmove	((*pst_inst).stDmxInit.m_stMemoryFuncs).m_pfnMemmove
#define cdmx_mpg_realloc	((*pst_inst).stDmxInit.m_stMemoryFuncs).m_pfnRealloc
#else
#define cdmx_mpg_malloc		((*pst_inst).stDmxInit.m_CallbackFunc).m_pMalloc
#define cdmx_mpg_free		((*pst_inst).stDmxInit.m_CallbackFunc).m_pFree
#define cdmx_mpg_memset		((*pst_inst).stDmxInit.m_CallbackFunc).m_pMemset
#define cdmx_mpg_memcpy		((*pst_inst).stDmxInit.m_CallbackFunc).m_pMemcpy
#define cdmx_mpg_memmove	((*pst_inst).stDmxInit.m_CallbackFunc).m_pMemmove
#define cdmx_mpg_realloc	((*pst_inst).stDmxInit.m_CallbackFunc).m_pRealloc
#endif
#define cdmx_malloc 	cdmx_mpg_malloc
#define cdmx_free 		cdmx_mpg_free
#define cdmx_memset 	cdmx_mpg_memset
#define cdmx_memcpy 	cdmx_mpg_memcpy
#define cdmx_memmove	cdmx_mpg_memmove
#define cdmx_realloc 	cdmx_mpg_realloc


static 
long
find_seq_header( mpg_dmx_inst_t* pstInst )
{
	long ret = DMXRET_SUCCESS;
	mpg_dmx_inst_t* pst_inst = pstInst;

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
				long offset = 0;
				long l_loop_cnt = 0;

				unsigned char* pby_temp = (unsigned char*)cdmx_malloc(CDMX_MAX_FRAME_LEN);
				if ( pby_temp == NULL ) {
					ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
					return CDK_ERR_MALLOC;
				}
				DSTATUS("[Line:%04d] pby_temp: 0x%x\n", __LINE__, pby_temp);

				//ALOGI( "cdmx_get_seqhead start\n" );

				pstInst->stDmxGetStream.m_iGetStreamComponent = DMXTYPE_VIDEO;
				while(cnt_seq_header < 300) 
				{
					//ALOGI( "[MPG] find Sequence Header:%d !! \n", cnt_seq_header );
					do {
						//Reset output type before perform get_stream.
						pstInst->stDmxOutStream.m_iType = MPG_PACKET_NONE;

						//If output type is PCI, DSI, NAVI, or Caption, skip it only in CDK
						while (pstInst->stDmxOutStream.m_iType != MPG_PACKET_AUDIO && pstInst->stDmxOutStream.m_iType != MPG_PACKET_VIDEO) 
						{
							ret = pstInst->pfnDmxLib( MPG_OP_GETSTREAM, &pstInst->hDmxHandle, &pstInst->stDmxGetStream, &pstInst->stDmxOutStream );
							if (ret < 0) 
							{
								if( pby_temp ) cdmx_free(pby_temp);
								ERROR( "[%ld] Error: MPG_OP_GETSTREAM failed (%d)\n", ret, __LINE__ );
								return ret;
							}
						}
						cdmx_memcpy(pby_temp + offset, pstInst->stDmxOutStream.m_pData, pstInst->stDmxOutStream.m_iDataLength);
						offset += pstInst->stDmxOutStream.m_iDataLength;
					}
					while ((pstInst->stDmxOutStream.m_pData[3] == 0xB3) && (pstInst->stDmxOutStream.m_iType == MPG_PACKET_VIDEO)); 
					pstInst->stDmxOutStream.m_iDataLength = offset;

					if( pstInst->stDmxOutStream.m_iType == DMXTYPE_VIDEO )
					{
						// extract sequence header
						ret = cdmx_get_seqhead( pby_temp,//pstInst->stDmxOutStream.m_pbyStreamData, 
												pstInst->stDmxOutStream.m_iDataLength,
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

				STEP( "\tSequence header extraction finished: seek to zero \n" );
				// sequence header extraction finished (return ==1)

				pstInst->stDmxGetStream.m_iSeekTimeMSec = 0;
				pstInst->stDmxGetStream.m_iSeekMode = MPG_SEEK_DIR_FWD | MPG_SEEK_MODE_ABSOLUTE;
				STEP( "\t\t DMXOP_SEEK: seek to zero \n" );
				ret = pstInst->pfnDmxLib( MPG_OP_SEEK, &pstInst->hDmxHandle, &pstInst->stDmxGetStream, &pstInst->stDmxOutStream );
				if ( ret<0 ) 
				{
					ERROR( "[%ld] Error: MPG_OP_SEEK failed (%d)\n", ret, __LINE__ );
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
    mpg_dmx_inst_t			*pstInst,
	mpg_cdmx_video_info_t	*pstVideoInfo,
	mpg_cdmx_audio_info_t	*pstAudioInfo,
	unsigned long 	 		 ulType
	)
{
	mpg_dmx_inst_t			*pst_inst    = pstInst;
	cdmx_info_t				*pst_out_info= pstCdmxInfo;
	mpg_cdmx_file_info_t	*pst_finfo	 = pst_inst->stDmxInfo.m_pstFileInfo;
	mpg_cdmx_video_info_t	*pst_vinfo	 = pstVideoInfo;
	mpg_cdmx_audio_info_t	*pst_ainfo	 = pstAudioInfo;
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
		pst_out_info->m_sFileInfo.m_iSeekable			= (pst_finfo->m_ulDuration > 0)?1:0;
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
		pst_out_info->m_sVideoInfo.m_iAspectRatio 			= pst_vinfo->m_ulAspectRatio;
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
		pst_out_info->m_sAudioInfo.m_iBitRate			= 0;
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
mpg_set_stream_info ( 
    mpg_dmx_inst_t *pstInst
	)
{
	long i;
	mpg_dmx_inst_t  *pst_inst 		= pstInst;
	mpg_cdmx_info_t *pst_dmx_info 	= &pst_inst->stDmxInfo;
	mpg_dec_info_t  *pst_mpg_info 	= &pst_inst->stMpgInfo;

	//
	// file info
	//
	if ( pst_dmx_info->m_pstFileInfo == NULL )
	{
		pst_dmx_info->m_pstFileInfo = cdmx_malloc(sizeof(mpg_cdmx_file_info_t));
		if ( pst_dmx_info->m_pstFileInfo == NULL ) {
			ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
			return CDK_ERR_MALLOC;
		}
		DSTATUS("[Line:%04d] pstStreamIdList->plVideoIdList: 0x%x\n", __LINE__, pst_dmx_info->m_pstFileInfo);
		cdmx_memset( pst_dmx_info->m_pstFileInfo, 0, sizeof(mpg_cdmx_file_info_t) );
	}
	pst_dmx_info->m_pstFileInfo->m_pszFileName 	= pst_inst->stDmxInit.m_pOpenFileName;
	pst_dmx_info->m_pstFileInfo->m_ulDuration	= pst_mpg_info->m_Runningtime;
	pst_dmx_info->m_pstFileInfo->m_llFileSize 	= pst_mpg_info->m_FileSize;

	//
	// video info
	//
	if( pst_mpg_info->m_uiTotalVideoNum > 0 )
	{
		if ( pst_dmx_info->m_pstVideoInfoList == NULL )
		{
			pst_dmx_info->m_pstVideoInfoList = cdmx_malloc(sizeof(mpg_cdmx_video_info_t) * pst_mpg_info->m_uiTotalVideoNum);
			if ( pst_dmx_info->m_pstVideoInfoList == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] pst_dmx_info->m_pstVideoInfoList: 0x%x\n", __LINE__, pst_dmx_info->m_pstVideoInfoList);
			cdmx_memset( pst_dmx_info->m_pstVideoInfoList, 0, sizeof(mpg_cdmx_video_info_t) * pst_mpg_info->m_uiTotalVideoNum );
		}

		for(i = 0; i < pst_mpg_info->m_uiTotalVideoNum; i++)
		{
			mpg_video_info_t 		*pst_inp_info 	= &pst_mpg_info->m_pszVideoInfo[i];
			mpg_cdmx_video_info_t 	*pst_video_info = &pst_dmx_info->m_pstVideoInfoList[i];

			pst_video_info->m_ulStreamID  		= pst_inp_info->m_Index;
			pst_video_info->m_ulWidth			= pst_inp_info->m_Width;
			pst_video_info->m_ulHeight			= pst_inp_info->m_Height;
			pst_video_info->m_ulFrameRate 		= pst_inp_info->m_FrameRate;
			pst_video_info->m_ulFourCC			= pst_inp_info->m_FourCC;
			pst_video_info->m_pbyCodecPrivate 	= NULL;
			pst_video_info->m_ulCodecPrivateSize= 0;

			pst_video_info->m_ulAspectRatio		= pst_inp_info->m_AspectRatio;
			pst_video_info->m_ulBitRate			= pst_inp_info->m_BitRate;
			pst_video_info->m_lStartPTS			= pst_inp_info->m_StartPTS;
			pst_video_info->m_lEndPTS			= pst_inp_info->m_EndPTS;

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
		// set default 
		if( pst_mpg_info->m_uiDefaultVideo < pst_mpg_info->m_uiTotalVideoNum )
			pst_dmx_info->m_pstDefaultVideoInfo = &pst_dmx_info->m_pstVideoInfoList[pst_mpg_info->m_uiDefaultVideo];
		else
			pst_dmx_info->m_pstDefaultVideoInfo = &pst_dmx_info->m_pstVideoInfoList[0];
		pst_dmx_info->m_ulVideoStreamTotal = pst_mpg_info->m_uiTotalVideoNum;
	}

	//
	// audio info
	//
	if( pst_mpg_info->m_uiTotalAudioNum > 0 )
	{
		if( pst_dmx_info->m_pstAudioInfoList == NULL )
		{
			pst_dmx_info->m_pstAudioInfoList = cdmx_malloc(sizeof(mpg_cdmx_audio_info_t) * pst_mpg_info->m_uiTotalAudioNum);
			if ( pst_dmx_info->m_pstAudioInfoList == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] pst_dmx_info->m_pstVideoInfoList: 0x%x\n", __LINE__, pst_dmx_info->m_pstAudioInfoList);
			cdmx_memset( pst_dmx_info->m_pstAudioInfoList, 0, sizeof(mpg_cdmx_audio_info_t) * pst_mpg_info->m_uiTotalAudioNum );
		}

		for(i = 0; i < pst_mpg_info->m_uiTotalAudioNum; i++)
		{
			mpg_audio_info_t 		*pst_inp_info 	= &pst_mpg_info->m_pszAudioInfo[i];
			mpg_cdmx_audio_info_t 	*pst_audio_info = &pst_dmx_info->m_pstAudioInfoList[i];

			pst_audio_info->m_ulStreamID			= pst_inp_info->m_Index;
			pst_audio_info->m_ulSamplePerSec		= pst_inp_info->m_SamplePerSec;
			pst_audio_info->m_ulBitsPerSample		= pst_inp_info->m_BitsPerSample;
			pst_audio_info->m_ulChannels			= pst_inp_info->m_Channels;
			pst_audio_info->m_ulFormatTag			= pst_inp_info->m_FormatId;
			pst_audio_info->m_pbyCodecPrivate		= NULL;
			pst_audio_info->m_ulCodecPrivateSize	= 0;
			pst_audio_info->m_pszLanguage			= NULL;

			pst_audio_info->m_ulBitRate				= pst_inp_info->m_BitsRate;
			pst_audio_info->m_ulSyncWord			= pst_inp_info->m_SyncWord;
			pst_audio_info->m_ulChunkSize			= pst_inp_info->m_ChunkSize;
			pst_audio_info->m_lStartPTS				= pst_inp_info->m_StartPTS;
			pst_audio_info->m_lEndPTS				= pst_inp_info->m_EndPTS;

			DSTATUS( " [Audio_%03ld] - StreamID: %ld / %ld Hz / %ld bits / %ld Ch / FormatTag: 0x%04X / Language: \"%s\" \n",
						i,
						pst_audio_info->m_ulStreamID,
						pst_audio_info->m_ulSamplePerSec,
						pst_audio_info->m_ulBitsPerSample,
						pst_audio_info->m_ulChannels,
						(unsigned int)pst_audio_info->m_ulFormatTag,
						pst_audio_info->m_pszLanguage);
		}
		// set default 
		if( pst_mpg_info->m_uiDefaultAudio < pst_mpg_info->m_uiTotalAudioNum )
			pst_dmx_info->m_pstDefaultAudioInfo = &pst_dmx_info->m_pstAudioInfoList[pst_mpg_info->m_uiDefaultAudio];
		else
			pst_dmx_info->m_pstDefaultAudioInfo = &pst_dmx_info->m_pstAudioInfoList[0];
		pst_dmx_info->m_ulAudioStreamTotal = pst_mpg_info->m_uiTotalAudioNum;
	}

	return 0;
}

static
long
mpg_set_stream_info_sel_stream ( 
    mpg_dmx_inst_t 			*pstInst,
	mpg_video_info_t 		*pstSelVideoInfo,
	mpg_audio_info_t 		*pstSelAudioInfo,
	mpg_cdmx_video_info_t 	*pstOutVideoInfo,
	mpg_cdmx_audio_info_t 	*pstOutAudioInfo
	)
{
	mpg_dmx_inst_t *pst_inst = pstInst;
	mpg_cdmx_info_t *pst_dmx_info = &pst_inst->stDmxInfo;
	mpg_dec_info_t *pst_mpg_info = &pst_inst->stMpgInfo;

	//
	// video info
	//
	if( pstSelVideoInfo && pstOutVideoInfo )
	{
		mpg_video_info_t 		*pst_inp_info 	= pstSelVideoInfo;
		mpg_cdmx_video_info_t 	*pst_video_info = pstOutVideoInfo;

		pst_video_info->m_ulStreamID  		= pst_inp_info->m_Index;
		pst_video_info->m_ulWidth			= pst_inp_info->m_Width;
		pst_video_info->m_ulHeight			= pst_inp_info->m_Height;
		pst_video_info->m_ulFrameRate 		= pst_inp_info->m_FrameRate;
		pst_video_info->m_ulFourCC			= pst_inp_info->m_FourCC;
		pst_video_info->m_pbyCodecPrivate 	= NULL;
		pst_video_info->m_ulCodecPrivateSize= 0;

		pst_video_info->m_ulAspectRatio		= pst_inp_info->m_AspectRatio;
		pst_video_info->m_ulBitRate			= pst_inp_info->m_BitRate;
		pst_video_info->m_lStartPTS			= pst_inp_info->m_StartPTS;
		pst_video_info->m_lEndPTS			= pst_inp_info->m_EndPTS;

		DSTATUS( " [Video] - StreamID: %ld / %ldx%ld pix / %.3f fps / FourCC: \"%c%c%c%c\" \n",
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
		mpg_audio_info_t 		*pst_inp_info 	= pstSelAudioInfo;
		mpg_cdmx_audio_info_t 	*pst_audio_info = pstOutAudioInfo;

		pst_audio_info->m_ulStreamID			= pst_inp_info->m_Index;
		pst_audio_info->m_ulSamplePerSec		= pst_inp_info->m_SamplePerSec;
		pst_audio_info->m_ulBitsPerSample		= pst_inp_info->m_BitsPerSample;
		pst_audio_info->m_ulChannels			= pst_inp_info->m_Channels;
		pst_audio_info->m_ulFormatTag			= pst_inp_info->m_FormatId;
		pst_audio_info->m_pbyCodecPrivate		= NULL;
		pst_audio_info->m_ulCodecPrivateSize	= 0;
		pst_audio_info->m_pszLanguage			= NULL;

		pst_audio_info->m_ulBitRate				= pst_inp_info->m_BitsRate;
		pst_audio_info->m_ulSyncWord			= pst_inp_info->m_SyncWord;
		pst_audio_info->m_ulChunkSize			= pst_inp_info->m_ChunkSize;
		pst_audio_info->m_lStartPTS				= pst_inp_info->m_StartPTS;
		pst_audio_info->m_lEndPTS				= pst_inp_info->m_EndPTS;

		DSTATUS( " [Audio] - StreamID: %ld / %ld Hz / %ld bits / %ld Ch / FormatTag: 0x%04X / Language: \"%s\" \n",
					pst_audio_info->m_ulStreamID,
					pst_audio_info->m_ulSamplePerSec,
					pst_audio_info->m_ulBitsPerSample,
					pst_audio_info->m_ulChannels,
					(unsigned int)pst_audio_info->m_ulFormatTag,
					pst_audio_info->m_pszLanguage);
	}

	return 0;
}


static 
long
mpg_attach_start_code(
    mpg_dmx_inst_t 		*pstInst
	)
{
	long ret = 0;
	mpg_dmx_inst_t *pst_inst = pstInst;

	av_dmx_outstream_t output;
	output.m_pbyStreamData 		= pst_inst->stDmxOutStream.m_pData;
	output.m_lStreamDataSize	= pst_inst->stDmxOutStream.m_iDataLength;
	output.m_lStreamType		= pst_inst->stDmxOutStream.m_iType;
	output.m_lTimeStamp			= pst_inst->stDmxOutStream.m_iRefTime;
	output.m_lEndTimeStamp		= 0;
	// attach start code on pst_inst->stDmxOutStream.m_pbyStreamData
	ret = attach_start_code( (cdmx_inst_t*)pst_inst, 
							 (av_dmx_info_t*)&pst_inst->stDmxInfo,
							 (av_dmx_outstream_t*)&output, NULL );
	if(ret < 0) 
	{
		ALOGE( "[%ld] Error: in attach_start_code\n", ret );
		return ret;
	}
	pst_inst->stDmxOutStream.m_iDataLength = output.m_lStreamDataSize;

	return ret;
}


av_result_t 
cdmx_mpg( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 )
{
	av_result_t ret = DMXRET_SUCCESS;
	mpg_dmx_inst_t* pst_inst = (mpg_dmx_inst_t*)(*ptHandle); // assume that its memory is allocated.

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
			ret = open_library( (const char*)"/system/lib/libff4.so",
								(const char*)"TCC_MPG_DEC",
								&pv_dmx,
								&pv_handle );
			if (ret < 0) {
				return ret;
			}
			pst_inst->pfnDmxLib = ( av_result_t (*) (unsigned long, av_handle_t*, void*, void*) )(pv_dmx);
			pst_inst->pvLibHandle = pv_handle;
			STEP( "LibHandle loaded....(0x%x) (handle:0x%x) ...\n", pst_inst->pfnDmxLib, pst_inst->pvLibHandle );
		#else
			pst_inst->pfnDmxLib   = TCC_MPG_DEC;
			pst_inst->pvLibHandle = NULL;
		#endif
		}

		pst_inst->stCdmxCtrl.ulParsingMode = pst_init_param->ulParsingMode;
		//--------------------------------------------------
		// set init parameters
		//--------------------------------------------------
		pst_inst->stDmxInit.m_pOpenFileName = pst_init_param->pszFileName;
		
		pst_inst->stDmxInit.m_CallbackFunc.m_pMalloc			= pst_callback->m_pfMalloc;
		pst_inst->stDmxInit.m_CallbackFunc.m_pFree				= pst_callback->m_pfFree;
		pst_inst->stDmxInit.m_CallbackFunc.m_pNonCacheMalloc	= pst_callback->m_pfNonCacheMalloc;
		pst_inst->stDmxInit.m_CallbackFunc.m_pNonCacheFree		= pst_callback->m_pfNonCacheFree;
		pst_inst->stDmxInit.m_CallbackFunc.m_pMemcpy			= pst_callback->m_pfMemcpy;
		pst_inst->stDmxInit.m_CallbackFunc.m_pMemset			= pst_callback->m_pfMemset;
		pst_inst->stDmxInit.m_CallbackFunc.m_pRealloc			= pst_callback->m_pfRealloc;
		pst_inst->stDmxInit.m_CallbackFunc.m_pMemmove			= pst_callback->m_pfMemmove;
		pst_inst->stDmxInit.m_CallbackFunc.m_pFopen				= pst_callback->m_pfFopen;
		pst_inst->stDmxInit.m_CallbackFunc.m_pFread				= pst_callback->m_pfFread;
		pst_inst->stDmxInit.m_CallbackFunc.m_pFseek				= pst_callback->m_pfFseek;
		pst_inst->stDmxInit.m_CallbackFunc.m_pFtell				= pst_callback->m_pfFtell;
		pst_inst->stDmxInit.m_CallbackFunc.m_pFwrite			= pst_callback->m_pfFwrite;
		pst_inst->stDmxInit.m_CallbackFunc.m_pFclose			= pst_callback->m_pfFclose;

		pst_inst->stDmxInit.m_CallbackFunc.m_pUnlink			= pst_callback->m_pfUnlink;
		pst_inst->stDmxInit.m_CallbackFunc.m_pFeof				= pst_callback->m_pfFeof;
		pst_inst->stDmxInit.m_CallbackFunc.m_pFflush			= pst_callback->m_pfFflush;
		pst_inst->stDmxInit.m_CallbackFunc.m_pFseek64          	= pst_callback->m_pfFseek64;
		pst_inst->stDmxInit.m_CallbackFunc.m_pFtell64			= pst_callback->m_pfFtell64;

		pst_inst->stDmxInit.m_CallbackFunc.m_pCRCFread			= pst_callback->m_pfFread;
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

		// set init variables
		pst_inst->stDmxInit.m_FileCacheSize = MPG_BASIC_FILECACHE_SIZE;
		pst_inst->stDmxInit.m_GetStreamMode = (pst_inst->stCdmxCtrl.ulParsingMode & CDMX_MARKER_SEQUENTIAL_PLAY_MODE_BIT) ? MPG_MODE_SEQUENTIAL : MPG_MODE_INTERLEAVED;
	#if 0
		pst_inst->stDmxInit.m_UseCaptionInfo = 1;//If there is caption, ready to parse caption data.
		pst_inst->stDmxInit.m_CaptionIdx = 0x20;//0x20 is the most basical index for subtitle.
		pst_inst->stDmxInit.m_UseNavigationInfo = 1;
	#else
		pst_inst->stDmxInit.m_UseCaptionInfo = 0;
		pst_inst->stDmxInit.m_UseNavigationInfo = 0;
	#endif
		//If this field is set, parser returns play time.
		pst_inst->stDmxInit.m_ReturnDuration = 1;
		//Output Buffer is allocated inside parser.(Since V1.68)
		pst_inst->stDmxOutStream.m_pData = NULL;


		if ( pst_inst->stCdmxCtrl.ulParsingMode & CDMX_MARKER_MEDIA_SCAN_MODE_BIT )
		{
			STEP( "\tDMXOP_OPEN_FOR_MEDIASCAN 01\n" );
			ret = pst_inst->pfnDmxLib( MPG_OP_SCAN, &pst_inst->hDmxHandle, &pst_inst->stDmxInit, &pst_inst->stMpgInfo );
			STEP( "\tDMXOP_OPEN_FOR_MEDIASCAN 02\n" );
		}
		else
		{
			if ( (pst_inst->stCdmxCtrl.ulParsingMode & CDMX_MARKER_PLAYBACK_ONLY_MODE_BIT) || 
				 (pst_inst->stCdmxCtrl.ulParsingMode & CDMX_MARKER_IGNORE_FILE_SIZE_BIT  ) )
			{
				pst_inst->stDmxInit.m_ReturnDuration = 0;
			}
			STEP( "\tDMXOP_OPEN 01\n" );
			ret = pst_inst->pfnDmxLib( MPG_OP_INIT, &pst_inst->hDmxHandle, &pst_inst->stDmxInit, &pst_inst->stMpgInfo );
			STEP( "\tDMXOP_OPEN 02\n" );
		}
		if( ret < 0 )
		{
			ERROR( "[%ld] Error: DMXOP_OPEN failed (%d)\n", ret, __LINE__ );
            return ret;
        }
		STEP( "DMXOP_OPEN: END\n" );

		#if 0
		// test: seek to zero
		if ( !( pst_inst->stCdmxCtrl.ulParsingMode & CDMX_MARKER_MEDIA_SCAN_MODE_BIT ) )
		{
			pst_inst->stDmxGetStream.m_iGetStreamComponent = MPG_PACKET_VIDEO;
			pst_inst->stDmxGetStream.m_iSeekTimeMSec = 0;
			pst_inst->stDmxGetStream.m_iSeekMode = MPG_SEEK_DIR_FWD | MPG_SEEK_MODE_ABSOLUTE;
			STEP( "\t\t DMXOP_SEEK: seek to zero \n" );
			ret = pst_inst->pfnDmxLib( MPG_OP_SEEK, &pst_inst->hDmxHandle, &pst_inst->stDmxGetStream, &pst_inst->stDmxOutStream );
			if ( ret<0 ) 
			{
				ERROR( "[%ld] Error: MPG_OP_SEEK failed (%d)\n", ret, __LINE__ );
				return ret;
			}
		}
		#endif
		///////////////////////////////////////////////////////////////////////////////
		//
		// copy media info. to common structures
		//
		ret = mpg_set_stream_info ( pst_inst );
		if( ret < 0 )
		{
			ERROR( "[%ld] Error: mpg_get_stream_info failed (%d)\n", ret, __LINE__ );
            return ret;
        }

		///////////////////////////////////////////////////////////////////////////////
		//
		// set control variables & ..
		//
		{
			mpg_cdmx_file_info_t  *pst_finfo = pst_inst->stDmxInfo.m_pstFileInfo;
			mpg_cdmx_video_info_t *pst_vinfo = pst_inst->stDmxInfo.m_pstDefaultVideoInfo;

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
							 0, 
							 0 );

			cdmx_get_stream_id_list( &pst_inst->stCdmxBuff.stStrmIdList,
									 &pst_inst->stDmxInfo, 
									 sizeof(*pst_inst->stDmxInfo.m_pstDefaultVideoInfo), 
									 sizeof(*pst_inst->stDmxInfo.m_pstDefaultAudioInfo), 
									 0,
									 0 );
		}
		///////////////////////////////////////////////////////////////////////////////
		//
		// find_seq_header
		//
		if (!pst_inst->stCdmxCtrl.bSeqHeaderDone && pst_inst->stDmxInfo.m_pstDefaultVideoInfo)
		{
			if ( pst_inst->stDmxInfo.m_pstFileInfo->m_ulDuration >= 1000 ) 
			{
				pst_inst->stCdmxCtrl.bSeqHeaderDone = 1;
				STEP( "\tfind_seq_header: BEGIN\n" );
				ret = find_seq_header(pst_inst);
				if (ret < 0) {
					ERROR( "[%ld] Error: find_seq_header failed (%d)\n", ret, __LINE__ );
					return ret;
				}
				STEP( "\tfind_seq_header: END\n" );
			}
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
			mpg_cdmx_video_info_t* pst_vinfo = NULL;
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
			mpg_cdmx_audio_info_t *pst_ainfo = NULL;
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

		mpg_dec_sel_stream_t 	dmx_sel_strm;
		mpg_dec_sel_info_t		dmx_sel_info;

		STEP( "In CDMX_SEL_STREAM\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		cdmx_memset(&dmx_sel_strm, 0, sizeof(dmx_sel_strm));
		cdmx_memset(&dmx_sel_info, 0, sizeof(dmx_sel_info));

		dmx_sel_strm.m_ulSelType		= pst_inp_param->lSelType;
		dmx_sel_strm.m_ulVideoID		= pst_inp_param->lVideoID;
		dmx_sel_strm.m_ulAudioID		= pst_inp_param->lAudioID;
		dmx_sel_strm.m_ulSubtitleID	= pst_inp_param->lSubtitleID;

		ret = pst_inst->pfnDmxLib( MPG_OP_SELSTREAM, &pst_inst->hDmxHandle, &dmx_sel_strm, &dmx_sel_info );
		if( ret < 0 )
		{
			ERROR( "[%ld] Error: MPG_OP_SELSTREAM failed (%d)\n", ret, __LINE__ );
			return ret;
		}

		///////////////////////////////////////////////////////////////////////////////
		//
		// copy media info. to common structures
		//
		{
			mpg_cdmx_video_info_t 	st_out_video_info;
			mpg_cdmx_audio_info_t 	st_out_audio_info;
			cdmx_memset(&st_out_video_info,	 0, sizeof(mpg_cdmx_video_info_t));
			cdmx_memset(&st_out_audio_info,	 0, sizeof(mpg_cdmx_audio_info_t));
			ret = mpg_set_stream_info_sel_stream ( pst_inst, 
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
		long offset = 0;

		//STEP( "In CDMX_GETSTREAM\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		pst_inst->stDmxGetStream.m_iGetStreamComponent 		= pst_inp_param->m_iPacketType;
		pst_inst->stDmxGetStream.m_iInterestingCaptionIdx 	= pst_inp_param->m_iInterestingCaptionIdx;

		//If output type is PCI, DSI, NAVI, or Caption, skip it only in CDK
		if(pst_inst->stDmxInit.m_GetStreamMode == MPG_MODE_INTERLEAVED)
		{
			do
			{
				//Reset output type before perform get_stream.
				pst_inst->stDmxOutStream.m_iType = MPG_PACKET_NONE;
				while(pst_inst->stDmxOutStream.m_iType != MPG_PACKET_AUDIO && pst_inst->stDmxOutStream.m_iType != MPG_PACKET_VIDEO)
				{
					/*
					Start of example for Caption Decoding
					This is just example for decoding of caption which means (pst_inst->stDmxOutStream.m_iType == PACKET_SUBTITLE)
					This part shows how to decode Caption Data.(Caption is not used in CDK)
					*/
					if(0)//never get into here
					{
						if(pst_inst->stDmxOutStream.m_iType == MPG_PACKET_SUBTITLE)
						{
							mpg_caption_input_t		tCaptionInput;
							mpg_caption_output_t	tCaptionOutput;

							tCaptionInput.m_pData 		= pst_inst->stDmxOutStream.m_pData;
							tCaptionInput.m_iDataLength = pst_inst->stDmxOutStream.m_iDataLength;
							tCaptionInput.m_bCropEnable = 1;

							tCaptionOutput.m_pCaption = cdmx_malloc(pst_inst->stMpgInfo.m_pszVideoInfo[pst_inst->stMpgInfo.m_uiDefaultVideo].m_Width * pst_inst->stMpgInfo.m_pszVideoInfo[pst_inst->stMpgInfo.m_uiDefaultVideo].m_Height * 2);
							if ( tCaptionOutput.m_pCaption == NULL ) {
								ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
								return CDK_ERR_MALLOC;
							}
							DSTATUS("[Line:%04d] tCaptionOutput.m_pCaption: 0x%x\n", __LINE__, tCaptionOutput.m_pCaption);

							ret = pst_inst->pfnDmxLib( MPG_OP_SUBTITLE, &pst_inst->hDmxHandle, &tCaptionInput, &tCaptionOutput );
							if( ret < 0 )
							{
								ERROR( "[%ld] Error: MPG_OP_SUBTITLE failed (%d)\n", ret, __LINE__ );
							}
							cdmx_free(tCaptionOutput.m_pCaption);
						}
					}
					// End of example for Caption Decoding
					ret = pst_inst->pfnDmxLib( MPG_OP_GETSTREAM, &pst_inst->hDmxHandle, &pst_inst->stDmxGetStream, &pst_inst->stDmxOutStream );
					if( ret < 0 )
					{
						ERROR( "[%ld] Error: MPG_OP_GETSTREAM failed (%d)\n", ret, __LINE__ );
						if(ret == ERR_END_OF_FILE)
						{
							if(pst_inp_param->m_iPacketBuffSize == MPG_PACKET_AUDIO)
								return ERR_END_OF_AUDIO_FILE;
							else
								return ERR_END_OF_VIDEO_FILE;
						}
						return ret;
					}
				}
				// Output Buffer is allocated inside parser, so do not map output global output buffer to mpg output buffer.
				// in the end of get stream, copy data to global output buffer.
				cdk_memcpy(pst_inp_param->m_pPacketBuff + offset, pst_inst->stDmxOutStream.m_pData, pst_inst->stDmxOutStream.m_iDataLength);
				offset += pst_inst->stDmxOutStream.m_iDataLength;
			}while((pst_inst->stDmxOutStream.m_pData[3] == 0xB3) && (pst_inst->stDmxOutStream.m_iType == MPG_PACKET_VIDEO));

			pst_out_param->m_pPacketData = pst_inp_param->m_pPacketBuff;
			pst_out_param->m_iPacketSize = offset;
			pst_out_param->m_iPacketType = pst_inst->stDmxOutStream.m_iType;

			if( pst_inst->stDmxOutStream.m_iType == DMXTYPE_VIDEO )
			{
				// sequence header attachment
				if( (!pst_inst->stCdmxCtrl.bIsNotFirstFrame) && (pst_inst->stSeqHeader.m_iSeqHeaderDataLen > 0) )
				{
					ALOGE("SEQ_HEADER_ATTACHED!!!!!!!");
					pst_inst->stCdmxCtrl.bIsNotFirstFrame = 1;
					cdmx_memmove( (unsigned char *)(pst_out_param->m_pPacketData + pst_inst->stSeqHeader.m_iSeqHeaderDataLen), pst_out_param->m_pPacketData, pst_out_param->m_iPacketSize );
					cdmx_memcpy ( pst_out_param->m_pPacketData, pst_inst->stSeqHeader.m_pSeqHeaderData, pst_inst->stSeqHeader.m_iSeqHeaderDataLen );
					pst_out_param->m_iPacketSize += pst_inst->stSeqHeader.m_iSeqHeaderDataLen;
				}

				ret = mpg_attach_start_code( pst_inst );
				if(ret < 0) 
				{
					ALOGE( "[%ld] Error: in mpg_attach_start_code\n", ret );
					return ret;
				}
			}
		}//if(pst_inst->stDmxInit..m_GetStreamMode == MPG_MODE_INTERLEAVED)
		else
		{
			ret = pst_inst->pfnDmxLib( MPG_OP_GETSTREAM, &pst_inst->hDmxHandle, &pst_inst->stDmxGetStream, &pst_inst->stDmxOutStream );
			if( ret < 0 )
			{
				ERROR( "[%ld] Error: MPG_OP_GETSTREAM failed (%d)\n", ret, __LINE__ );
				if(ret == ERR_END_OF_FILE)
				{
					if(pst_inp_param->m_iPacketBuffSize == MPG_PACKET_AUDIO)
						return ERR_END_OF_AUDIO_FILE;
					else
						return ERR_END_OF_VIDEO_FILE;
				}
				return ret;
			}

			if( (pst_inst->stDmxOutStream.m_iType == DMXTYPE_AUDIO) && (pst_inp_param->m_pPacketBuff_ext1 != NULL))
			{
				cdmx_memcpy(pst_inp_param->m_pPacketBuff_ext1, pst_inst->stDmxOutStream.m_pData, pst_inst->stDmxOutStream.m_iDataLength);
				pst_out_param->m_pPacketData = pst_inp_param->m_pPacketBuff_ext1;
			}
			else
			{
			#if 1
				cdmx_memcpy(pst_inp_param->m_pPacketBuff, pst_inst->stDmxOutStream.m_pData, pst_inst->stDmxOutStream.m_iDataLength);
				pst_out_param->m_pPacketData = pst_inp_param->m_pPacketBuff;
			#else
				pst_out_param->m_pPacketData = pst_inst->stDmxOutStream.m_pData;
			#endif
			}

			pst_out_param->m_iPacketSize = pst_inst->stDmxOutStream.m_iDataLength;
			pst_out_param->m_iPacketType = pst_inst->stDmxOutStream.m_iType;
		} 

		//xelloss , CDK give pst_inst->stDmxOutStream.m_iRefTime=-1 until finding I-frame, just return -1
		pst_out_param->m_iTimeStamp = pst_inst->stDmxOutStream.m_iRefTime;

	}//CDMX_GETSTREAM
	else if( ulOpCode == CDMX_SEEK )
	{
		cdmx_seek_t		*pst_inp_param = (cdmx_seek_t  *)pParam1;
		cdmx_output_t	*pst_out_param = (cdmx_output_t*)pParam2;

		STEP( "In CDMX_SEEK\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;


		if(pst_inp_param->m_uiSeekMode == CDMX_SEEKMODE_RELATIVE_TIME)
		{
			if(pst_inp_param->m_iSeekTime >= 0)
			{
				pst_inst->stDmxGetStream.m_iSeekMode = MPG_SEEK_DIR_FWD | MPG_SEEK_MODE_RELATIVE;
				pst_inst->stDmxGetStream.m_iSeekTimeMSec = pst_inp_param->m_iSeekTime;
			}
			else
			{
				pst_inst->stDmxGetStream.m_iSeekMode = MPG_SEEK_DIR_BWD | MPG_SEEK_MODE_RELATIVE;
				pst_inst->stDmxGetStream.m_iSeekTimeMSec = pst_inp_param->m_iSeekTime * -1;
			}
		}
		else
		{
			pst_inst->stDmxGetStream.m_iSeekMode = MPG_SEEK_DIR_FWD | MPG_SEEK_MODE_ABSOLUTE;

			// SSG
			// when you seek to 0 right after init process, the MPG parser do not handle seek request due to m_iGetStreamComponent is unknown
			pst_inst->stDmxGetStream.m_iGetStreamComponent = MPG_PACKET_VIDEO;
			
			pst_inst->stDmxGetStream.m_iSeekTimeMSec = pst_inp_param->m_iSeekTime;//xelloss debug + gsMpgInfo.m_sFileInfo.m_StartTime; // SSG
			//ALOGD("cdmx_mpg CDMX_SEEK start Time %d SeekTime %d ",gsMpgInfo.m_sFileInfo.m_StartTime,pst_inp_param->m_iSeekTime);
		}

		ret = pst_inst->pfnDmxLib( MPG_OP_SEEK, &pst_inst->hDmxHandle, &pst_inst->stDmxGetStream, &pst_inst->stDmxOutStream );
		if( ret < 0 )
		{
			ERROR( "[%ld] Error: MPG_OP_SEEK failed (%d)\n", ret, __LINE__ );
			return ret;
		}

		if( pst_out_param )
		{
			STEP( "CDMX_SEEK Success\n" );
			pst_out_param->m_iTimeStamp  = pst_inst->stDmxOutStream.m_iRefTime;
		}
		// exceptional condition: need a seq.header.
		pst_inst->stCdmxCtrl.bIsNotFirstFrame = FALSE;
	}
	else if( ulOpCode == CDMX_CLOSE )
	{
		STEP( "In CDMX_CLOSE\n" );
		if( pst_inst == NULL )
			return DMXRET_INVALID_HANDLE;

		ret = pst_inst->pfnDmxLib( MPG_OP_CLOSE, &pst_inst->hDmxHandle, &pst_inst->stDmxGetStream, &pst_inst->stDmxOutStream );
		if( ret < 0 )
		{
			ERROR( "[%ld] Error: MPG_OP_CLOSE failed (%d)\n", ret, __LINE__ );
			return ret;
		}

		// mpg only
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


#endif//INCLUDE_MPG_DMX
