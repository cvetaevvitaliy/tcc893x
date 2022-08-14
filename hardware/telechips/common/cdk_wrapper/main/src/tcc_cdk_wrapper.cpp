/* ------------------------------------------------------------------
 * Copyright (C) 2009-2010 Telechips 
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */

#include "tcc_cdk_wrapper.h"
#include "cdk_core.h"
#include "cdk_audio.h"
#include "utils/Log.h"
#include <audio/tcc_ape_dmx.h>
#include "stdlib.h"
#include "MessageQueue.h"
#include <cutils/properties.h>

#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ABitReader.h>

#include <HDCP2Sink.h>

#undef	LOG_TAG
#define LOG_TAG	"TCCCDKWrapper"

#define EXIST_VIDEO		1
#define EXIST_AUDIO		2

#define MAX_VIDEO_WIDTH 1920
#define MAX_VIDEO_HEIGHT 1088


//#define DEBUG
#ifdef DEBUG
#define LOGMSG(x...) ALOGD(x)
#define LOGERR(x...) ALOGE(x)
#define LOGINFO(x...) ALOGI(x)
#else
#define LOGMSG(x...)
#define LOGERR(x...)
#define LOGINFO(x...)
#endif

//#define PRINT_DEBUG_LOG
#ifdef PRINT_DEBUG_LOG
#define DLOGD(x...) ALOGD(x)
#define DLOGE(x...) ALOGE(x)
#define DLOGI(x...) ALOGI(x)
#else
#define DLOGD(x...)
#define DLOGE(x...)
#define DLOGI(x...)
#endif

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


//#define m_pstCdmxInst (static_cast<cdmx_inst_t*>(m_pvCdmxInst))
#define m_pstCdmxInst ( (cdmx_inst_t*)(m_pvCdmxInst) )
#define CDMX_INST(pstInst) 	( (cdmx_inst_t*)(pstInst) )
#define CDMX_HANDLE(handle) (unsigned long*)(&(handle))

#ifdef MERGE_FRAME_AVG
#define MPG_CODE_SEQUENCE_START 0x000001B3
#define MPG_CODE_PICTURE_START 0x00000100

/* MPEG2 start code */
#define MPEG2_PICTURE_START     0x00000100
#define MPEG2_SEQUENCE_HEADER   0x000001B3
//#define MPEG2_SEQUENCE_END      0x000001B7
//#define MPEG2_GROUP_OF_PICTURES 0x000001B8

/* WVC1 start code */
#define WVC1_VIDEO_HEADER       0x0000010F
#define WVC1_VIDEO_FRAME        0x0000010D

/* MPEG4 start code */
#define MPEG4_VIDEO_HEADER      0x00000100
#define MPEG4_VIDEO_FRAME       0x000001B6

#define MPEG4_VOP_START		0x000001B6
#define MPEG4_VIDEO_OBJECT_START_MAX		0x0000011F

/* AVS start code */
#define AVS_VIDEO_HEADER        0x000001B0
#define AVS_VIDEO_I_FRAME       0x000001B3
#define AVS_VIDEO_PB_FRAME      0x000001B6

//H.264 define
#define NAL_UNSPECIFIED1	0
#define NAL_SLICE			1
#define NAL_SLICE_DPA		2
#define NAL_SLICE_DPB		3
#define NAL_SLICE_DPC		4
#define NAL_SLICE_IDR		5	 // ref_idc != 0 
#define NAL_SEI				6    // ref_idc == 0 
#define NAL_SPS				7
#define NAL_PPS				8
#define NAL_AUD				9
#define NAL_END_OF_SEQUENCE 10
#define NAL_END_OF_STREAM	11
#define NAL_FILLER_DATA		12
#define NAL_SPS_EXTENSION   13
#define NAL_AUX_PICTURE		19

#define PREV_NONE	0x00
#define PREV_SLICE	0x01
#define PREV_ETC	0x02
#define PREV_AUD	0x04

//int g_test_seek_flag = 0; --> m_bSeekFlag

//h.264 merge_function
static inline 
long scan_h264_aud( 
	unsigned char *pbyBuffer, 
	long lDataLength
	)
{
	unsigned char *buff = pbyBuffer;
	unsigned char *buff_end = pbyBuffer + lDataLength;
	unsigned long sync;

	sync = ((unsigned long)buff[0] << 8) | ((unsigned long)buff[1]);

	while( buff+4 < buff_end )
	{
		sync <<= 8;
		sync |= buff[2];

		if( (sync & 0x00FFFFFF) == 0x000001 )
		{
			if( (buff[3] & 0x1F) == NAL_AUD )
				return 1;	// AUD Found
		}
		buff++;
	}

	return 0; // AUD Not found
}

int32_t 
TCC_CDK_Wrapper::MergeVideoFrameAVC (
	unsigned char *pbyDst, long lDstBuffSize,
	unsigned char *pbySrc, long lSrcDataSize,
	long* plUsedBytes,	 
	long* plReadBytes,
	long *pbUsingAUD
    )
{
//	static long	prev_status; --> m_lPrevStatus
//	static long	sps_pps_dect; --> m_lSpsPpsDect
    unsigned char *buff = pbySrc;
    unsigned char *buff_start = pbySrc;
    unsigned char *buff_end = pbySrc + lSrcDataSize;
    unsigned long sync;
    int j;  //for debug

    sync = ((unsigned long)buff[0] << 8) | ((unsigned long)buff[1]);

    if ( *pbUsingAUD )
    {
        while ( buff+4 < buff_end )
        {
            sync <<= 8;
            sync |= buff[2];

            if ( (sync & 0x00FFFFFF) == 0x000001 )
            {
                if ( (buff[3] & 0x1F) == NAL_AUD )
                {
CHANGE_USED_AUD:
                    if ( m_lPrevStatus & PREV_AUD || 
                         m_lPrevStatus & PREV_SLICE )
                    {
                        long tmp_size = buff - buff_start;

                        memcpy(pbyDst, buff_start, tmp_size);
                        *plUsedBytes = tmp_size;
                        *plReadBytes = buff - pbySrc;
                        m_lPrevStatus = PREV_NONE;
                        return 1;
                    } else
                    {
                        buff_start = buff;
                        m_lPrevStatus = PREV_AUD;
                    }
                }
            }
            buff++;
        }   
    } else
    {
        while ( buff+4 < buff_end )
        {
            sync <<= 8;
            sync |= buff[2];

            if ( (sync & 0x00FFFFFF) == 0x000001 )
            {
                switch ( buff[3] & 0x1F )
                {
                case NAL_AUD:
                    *pbUsingAUD = 1;
                    goto CHANGE_USED_AUD;

                case NAL_SPS:
                case NAL_PPS:
                    if ( m_lPrevStatus & PREV_SLICE )
                    {
                        long tmp_size = buff - buff_start;

                        memcpy(pbyDst, buff_start, tmp_size);
                        *plUsedBytes = tmp_size;
                        *plReadBytes = buff - pbySrc;
                        m_lPrevStatus = PREV_NONE;
                        m_lSpsPpsDect = PREV_NONE;
                        return 1;
                    } else if ( m_lPrevStatus == PREV_NONE )
                    {
                        buff_start = buff;
                        m_lPrevStatus = PREV_ETC;
                        m_lSpsPpsDect = PREV_ETC;
                    }
                    break;

                case NAL_SLICE_IDR:
                case NAL_SLICE:
                case NAL_SLICE_DPA:
                case NAL_SLICE_DPB:
                case NAL_SLICE_DPC:
                    //if( prev_status & PREV_SLICE && sps_pps_dect == PREV_ETC)
                    // [AVG] for multi-slice
                    if( ( buff[4] & 0x80 ) == 0 )
                            continue;
                    if ( m_lPrevStatus & PREV_SLICE)
                    {
                        long tmp_size = buff - buff_start;

                        memcpy(pbyDst, buff_start, tmp_size);                               
                        *plUsedBytes = tmp_size;
                        *plReadBytes = buff - pbySrc;
                        m_lPrevStatus = PREV_NONE;
                        m_lSpsPpsDect = PREV_NONE;
                        return 1;
                    } else if ( m_lPrevStatus == PREV_NONE)
                    //else if( sps_pps_dect == PREV_ETC)
                    {
                        buff_start = buff;
                    }

                    m_lPrevStatus = PREV_SLICE;
                    break;
                }
            }
            buff++;
        }   
    }

    //if( prev_status && sps_pps_dect)
    if ( m_lPrevStatus )
    {
        long tmp_size = buff - buff_start;

        memcpy(pbyDst, buff_start, tmp_size);
        *plUsedBytes = tmp_size;
        *plReadBytes = buff - pbySrc;
    }

    return 0;
}

int32_t
TCC_CDK_Wrapper::TSD_GetMpegFrameStartPos(
	unsigned char *pbyDst, int lDstBuffSize,
	unsigned char *pbySrc, int lSrcDataSize,
	long *plUsedBytes, long *plReadBytes,
	unsigned long picture_header_type,
	unsigned long picture_header_type2,
	unsigned long sequence_header_type)
{
//	static int is_after_seq;		----------> m_after_seq
//	static int is_after_pic;		----------> m_after_pic
	unsigned char   *buff = pbySrc;
	unsigned char *buff_start = pbySrc;
	unsigned char   *buff_end = pbySrc + lSrcDataSize;
	unsigned long		sync;

//	ALOGE("TSD_GetMpegFrameStartPos");

	if( pbySrc == NULL )
	{
		m_after_seq = FALSE;
		return 0;
	}

	sync = ((unsigned long)buff[0] << 16) | ((unsigned long)buff[1] << 8) | ((unsigned long)buff[2]);

	while( (buff+3) < buff_end )
	{
		sync <<= 8;
		sync |= buff[3];
		
		if( sync == picture_header_type ||sync == picture_header_type2)
		{
			if( m_after_seq == TRUE )
			{
				m_after_seq = FALSE;
				m_after_pic = TRUE;
			}
			else
			{
				long tmp_size = buff - buff_start;
				memcpy(pbyDst, buff_start, tmp_size);
				*plUsedBytes = tmp_size;
				*plReadBytes = buff - pbySrc;
				m_after_seq = TRUE;
				return 1;
			}
		}
		else if( sync == sequence_header_type )
		{
			if(m_after_pic == TRUE)
			{
				long tmp_size = buff - buff_start;
				memcpy(pbyDst, buff_start, tmp_size);
				*plUsedBytes = tmp_size;
				*plReadBytes = buff - pbySrc;
				m_after_seq = TRUE;
				m_after_pic = FALSE;
				return 1;
			}
			m_after_seq = TRUE;
			buff_start = buff;
			buff++;
			continue;
			//return (int)(buff - buff_start);
		}
		buff++;
	}

	//if( prev_status )
	{
		long tmp_size = buff - buff_start;
		memcpy(pbyDst, buff_start, tmp_size);
		*plUsedBytes = tmp_size;
		*plReadBytes = buff - pbySrc;
	}
	return 0;
}
#endif

using namespace android;

#ifdef DIVX_DRM5
#define TCC_DIVXDRM_RESERVED_SIZE	80
#endif

#define USE_PV_MP3_CODEC 0 

#define MAX_CORRUPTED_FRAME_COUNT 200
#define MAX_CONSECUTIVE_GET_FIFO_FAIL_COUNT 5000

// --------------------------------------------------
// global variables
// --------------------------------------------------

static uint32_t	guiLastpts;

#define SUPPORT_AC3		0
#define SUPPORT_DDP		1
#define SUPPORT_DTS		2
#define SUPPORT_WMA		3
#define SUPPORT_DRA		4
#define SUPPORT_RA		5
#define SUPPORT_AAC		6
#define SUPPORT_BSAC	7
#define SUPPORT_MP3		8
#define SUPPORT_MP2		9
#define SUPPORT_MP1		10
#define SUPPORT_VORBIS	11
#define SUPPORT_FLAC	12
#define SUPPORT_APE		13
#define SUPPORT_WAV		14

#define SUPPORT_AMR		15
#define SUPPORT_AMRWBP	16
#define SUPPORT_EVRC	17
#define SUPPORT_QCELP	18

#define NUM_AUDIO_CODEC	SUPPORT_QCELP + 1

static int ALL_AUDIO_CHECKED = 0;
static int AUDIO_SUPPORT[NUM_AUDIO_CODEC] = {0};

static char *tAudioSupportName[] = {
	"tcc.audio.support.ac3",	// SUPPORT_AC3		0 
	"tcc.audio.support.ddp",    // SUPPORT_DDP		1 
	"tcc.audio.support.dts",    // SUPPORT_DTS		2 
	"tcc.audio.support.wma",    // SUPPORT_WMA		3 
	"tcc.audio.support.dra",    // SUPPORT_DRA		4 
	"tcc.audio.support.ra",     // SUPPORT_RA		5 
	"tcc.audio.support.aac",    // SUPPORT_AAC		6 
	"tcc.audio.support.bsac",   // SUPPORT_BSAC		7 
	"tcc.audio.support.mp3",    // SUPPORT_MP3		8 
	"tcc.audio.support.mp2",    // SUPPORT_MP2		9 
	"tcc.audio.support.mp1",    // SUPPORT_MP1		10
	"tcc.audio.support.vorbis",	// SUPPORT_VORBIS	11
	"tcc.audio.support.flac",   // SUPPORT_FLAC		12
	"tcc.audio.support.ape",    // SUPPORT_APE		13
	"tcc.audio.support.wav",    // SUPPORT_WAV		14
	"tcc.audio.support.amr",    // SUPPORT_AMR		15                  
	"tcc.audio.support.amrwbp", // SUPPORT_AMRWBP	16
	"tcc.audio.support.evrc",   // SUPPORT_EVRC		17
	"tcc.audio.support.qcelp",  // SUPPORT_QCELP	18
	NULL                            
};

// --------------------------------------------------
// local file functions, variables, class
// --------------------------------------------------

class CDK_ITEM
{
	public:
		CDK_ITEM();

		cdmx_info_t 	gsCDmxInfo;
		vdec_init_t 	gsVDecInit;
		cdmx_output_t 	gsCDmxOutput;
		cdmx_input_t 	gsCDmxInput;
		cdmx_seek_t 	gsCdmxSeek;
};

CDK_ITEM::CDK_ITEM()
{
	memset(&gsCDmxInfo, 0, sizeof(cdmx_info_t));
	memset(&gsVDecInit, 0, sizeof(vdec_init_t));
	memset(&gsCDmxOutput, 0, sizeof(cdmx_output_t));
	memset(&gsCDmxInput, 0, sizeof(cdmx_input_t));
	memset(&gsCdmxSeek, 0, sizeof(cdmx_seek_t));
}

static void
get_ext_name( char* pPathName, char* pExtName )
{
	int temp;
	char* p_str;

	p_str = pPathName;
	temp = cdk_strlen( p_str );

	p_str = &pPathName[temp-1];
	while( *p_str )
	{
		if( *p_str == '.' ) 
		{
			p_str++;
			break;
		}
		p_str--;
	}

	cdk_strcpy( pExtName, p_str );
}


const short gAC3FrmsizeTab[3][38] = {
	{	64, 64, 80, 80, 96, 96, 112, 112, 
		128, 128, 160, 160, 192, 192, 224, 224,
		256, 256, 320, 320, 384, 384, 448, 448,
		512, 512, 640, 640, 768, 768, 896, 896,
		1024, 1024, 1152, 1152, 1280, 1280 },
	{	69, 70, 87, 88, 104, 105, 121, 122,
		139, 140, 174, 175, 208, 209, 243, 244,
		278, 279, 348, 349, 417, 418, 487, 488,
		557, 558, 696, 697, 835, 836, 975, 976,
		1114, 1115, 1253, 1254, 1393, 1394 },
	{	96, 96, 120, 120, 144, 144, 168, 168,
		192, 192, 240, 240, 288, 288, 336, 336,
		384, 384, 480, 480, 576, 576, 672, 672,
		768, 768, 960, 960, 1152, 1152, 1344, 1344,
		1536, 1536, 1728, 1728, 1920, 1920 } 
};

/***********************************************************************
 * get audio packet
 * return:
 *		0 - success
 *		negative values - failure
 *						  : -768(0xFFFFFD00) - input param error or timeout error
 *						  : Otherwise - demuxer error
 **********************************************************************/
int  TCC_CDK_Wrapper::identify_audio_get_stream(void* pCDK, unsigned int loopCount)
{
	unsigned int loopFlag;
	unsigned int loopCnt;
	int ret;
	CDK_ITEM* pstCDK = (CDK_ITEM*)pCDK;
	cdmx_input_t*	st_cdmx_input  = &pstCDK->gsCDmxInput;
	cdmx_output_t*	st_cdmx_output = &pstCDK->gsCDmxOutput;

	if( (st_cdmx_input->m_pPacketBuff == NULL) || (st_cdmx_input->m_iPacketBuffSize < 0) )
	{
		return 0xFFFFFD00; //error
	}
	if( st_cdmx_input->m_iPacketType != AV_PACKET_AUDIO )
	{
		st_cdmx_input->m_iPacketType = AV_PACKET_AUDIO;
	}

	loopFlag = 0;
	loopCnt = 0;
	if( loopCount == 0 )
	{
		loopFlag = 1;
	}
	loopCnt = loopCount;

	do 
	{
		//get stream
		ret = m_pfnCdmx( CDMX_GETSTREAM, CDMX_HANDLE(m_pvCdmxInst), st_cdmx_input, st_cdmx_output );
		if( ret < 0 )
		{
			return ret; //demuxer error
		}

		//check the type of output packet
		if( st_cdmx_output->m_iPacketType == AV_PACKET_AUDIO )
		{
			return 0; //find audio packet
		}
		loopCnt--;
	}
	while( (loopFlag)|| (loopCnt > 0) );

	return 0xFFFFFD00; //error
}

int TCC_CDK_Wrapper::identify_audio_seek_zero(void* pCDK)
{
	int ret=0;
	CDK_ITEM* pstCDK = (CDK_ITEM*)pCDK;
	cdmx_seek_t		*pst_cdmx_seek   = &pstCDK->gsCdmxSeek;
	cdmx_output_t	*pst_cdmx_output = &pstCDK->gsCDmxOutput;

	pst_cdmx_seek->m_uiSeekMode = SEEK_ABSOLUTE;
	pst_cdmx_seek->m_iSeekTime = 0;

	ret = m_pfnCdmx( CDMX_SEEK, CDMX_HANDLE(m_pvCdmxInst), pst_cdmx_seek, pst_cdmx_output );
	return ret;
}

int TCC_CDK_Wrapper::identify_ac3_stream_sync(void* pCDK, unsigned char* pucBitstreamBufAddr, int iBitstreamBufSize)
{
	int			ret;
	int			i_stream_read_bytes = 0;
	int			i;
	int			i_fscode;
	int			i_framsizcod;
	int			i_frame_size;
	int			i_bsid;
	CDK_ITEM* pstCDK = (CDK_ITEM*)pCDK;
	unsigned char*	puc_packet_buf = (unsigned char*)pucBitstreamBufAddr;
	cdmx_input_t*	st_cdmx_input  = &pstCDK->gsCDmxInput;
	cdmx_output_t*	st_cdmx_output = &pstCDK->gsCDmxOutput;

	// DDP / DD / TrueHD / Unsupported identification 

	st_cdmx_input->m_iPacketType = AV_PACKET_AUDIO;
	
	// audio stream read
	while( 1 )
	{
		st_cdmx_input->m_pPacketBuff = (unsigned char*)puc_packet_buf;
		st_cdmx_input->m_iPacketBuffSize = iBitstreamBufSize - i_stream_read_bytes;

		ret = identify_audio_get_stream( pCDK, 30 );

		if( ret < 0 )
		{
			if(ret == 0xFFFFFD00)
			{
				ret = identify_audio_seek_zero( pCDK );
				if(ret < 0)
				{
					return CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
				}
				return AUDIO_ID_AC3;
			}
			else if( ret == ERR_END_OF_AUDIO_FILE || ret == ERR_END_OF_FILE )
				break;
		}

		if(	st_cdmx_output->m_pPacketData[4] == 0xf8 && st_cdmx_output->m_pPacketData[5] == 0x72 && st_cdmx_output->m_pPacketData[6] == 0x6f )
		{
			// TrueHD streams
			if( st_cdmx_output->m_pPacketData[7] == 0xba ) 
			{
				PrintInfo("TrueHD not supported\n");
				return AUDIO_ID_TRUEHD;
			}
			// MLP streams
			else if( st_cdmx_output->m_pPacketData[7] == 0xbb )
			{
				PrintInfo("MLP not supported\n");
				return CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
			}
		}
		i_stream_read_bytes += st_cdmx_output->m_iPacketSize;
		puc_packet_buf += st_cdmx_output->m_iPacketSize;
		if( i_stream_read_bytes > 1024 * 30 ) break; // 30kBytes read
	}

	ret = identify_audio_seek_zero( pCDK );

	if(ret < 0)
		return CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;

	st_cdmx_output->m_pPacketData = (unsigned char *)pucBitstreamBufAddr;
	for( i = 0 ; i < i_stream_read_bytes-1 ; i ++ )
	{
		if( st_cdmx_output->m_pPacketData[i] == 0x0b && st_cdmx_output->m_pPacketData[i+1] == 0x77 )
		{
			i_fscode = (st_cdmx_output->m_pPacketData[i+4]>>6)&0x3;
			i_framsizcod = (st_cdmx_output->m_pPacketData[i+4]&0x3f);
			i_bsid = (st_cdmx_output->m_pPacketData[i+5]>>3);
			
			// DDP
			if( i_bsid == 16 )
			{
				i_frame_size = st_cdmx_output->m_pPacketData[i+2]<<8;
				i_frame_size |= st_cdmx_output->m_pPacketData[i+3];
				i_frame_size = (i_frame_size&0x3ff);
				i_frame_size = (i_frame_size+1)*2;
			}
			// AC3
			else if( i_bsid < 11 )
			{
				if((i_fscode == 3) || (i_framsizcod>37))
					continue;
				i_frame_size = (gAC3FrmsizeTab[i_fscode][i_framsizcod]<<1);
			}
			else
			{
				continue;
			}

			if( i + i_frame_size > i_stream_read_bytes-1 )
				return CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
			else
			{
				if( st_cdmx_output->m_pPacketData[i+i_frame_size] == 0x0b && st_cdmx_output->m_pPacketData[i+i_frame_size+1] == 0x77 )
				{
					if( i_bsid == 16 )
					{
						//PrintInfo("DDP not supported\n");
						return AUDIO_ID_DDP; // actually it is ddp
					}
					else
						return AUDIO_ID_AC3;
				}
			}
		}
		
		else if( st_cdmx_output->m_pPacketData[i] == 0x77 && st_cdmx_output->m_pPacketData[i+1] == 0x0b )
		{
			i_fscode = (st_cdmx_output->m_pPacketData[i+5]>>6)&0x3;
			i_framsizcod = (st_cdmx_output->m_pPacketData[i+5]&0x3f);
			i_bsid = (st_cdmx_output->m_pPacketData[i+4]>>3);
			
			// DDP
			if( i_bsid == 16 )
			{
				i_frame_size = st_cdmx_output->m_pPacketData[i+2];
				i_frame_size |= st_cdmx_output->m_pPacketData[i+3]<<8;
				i_frame_size = (i_frame_size&0x3ff);
				i_frame_size = (i_frame_size+1)*2;
			}
			// AC3
			else if( i_bsid < 11 )
			{
				if((i_fscode == 3) || (i_framsizcod>37))
					continue;
				i_frame_size = (gAC3FrmsizeTab[i_fscode][i_framsizcod]<<1);
			}
			else
			{
				continue;
			}

			if( i + i_frame_size > i_stream_read_bytes-1 )
				return CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
			else
			{
				if( st_cdmx_output->m_pPacketData[i+i_frame_size] == 0x77 && st_cdmx_output->m_pPacketData[i+i_frame_size+1] == 0x0b )
				{
					if( i_bsid == 16 )
					{
						//PrintInfo("DDP not supported\n");
						return AUDIO_ID_DDP; // actually it is ddp
					}
					else
						return AUDIO_ID_AC3;
				}
			}
		}
	}

	return CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
}

/* check mpeg audio layer */
int TCC_CDK_Wrapper::identify_mpeg_stream(void* pCDK, unsigned char* pucBitstreamBufAddr, int iBitstreamBufSize, int iDmxType, int iAudioType)
{	
	int handle = 0, i_atype;
	int ilayer = 0;

	CDK_ITEM* pstCDK = (CDK_ITEM*)pCDK;
	cdmx_input_t*	st_cdmx_input  = &pstCDK->gsCDmxInput;
	cdmx_output_t*	st_cdmx_output = &pstCDK->gsCDmxOutput;
	cdmx_seek_t		st_cdmx_seek   = pstCDK->gsCdmxSeek;
	cdmx_info_t     st_cdmx_info   = pstCDK->gsCDmxInfo;

	if(iDmxType != CONTAINER_TYPE_MP4 )
	{
		return 0xFFFFFD00;
	}

	if(st_cdmx_info.m_sFileInfo.m_iSeekable == 0)
	{
		//return iAudioType;
		return 0xFFFFFD00;
	}

	i_atype = iAudioType;

#ifndef TEST_CHECK_MPEG
	handle = check_mpeg_audio_init();
#else
	long ret = cdmx_audio( CDMX_MPEG_AUDIO_INIT, NULL, &handle, NULL );
	if ( ret < 0 ) {
		ALOGE("[CDK_WRAPPER] [Err:%4d] CDMX_MPEG_AUDIO_INIT init", ret );
		return TCC_CDK_WRAPPER_DEMUXER_ERROR;
	}
#endif

	if( handle )
	{
		int ret;
		int iTryCnt = 0;

		do
		{
			if( iTryCnt > 38 )
			{
				break;
			}
			st_cdmx_input->m_pPacketBuff = (unsigned char*)pucBitstreamBufAddr;
			st_cdmx_input->m_iPacketBuffSize = iBitstreamBufSize;
			st_cdmx_input->m_iPacketType = AV_PACKET_AUDIO;

			ret = identify_audio_get_stream( pCDK, 30 );

			if( ret < 0 )
			{
				ilayer = 4;
				break;
			}

		#ifndef TEST_CHECK_MPEG
			ilayer = check_mpeg_audio_get_layer( handle, st_cdmx_output->m_pPacketData, st_cdmx_output->m_iPacketSize );
		#else
			ret = cdmx_audio( CDMX_MPEG_AUDIO_GET_LAYER, (av_handle_t*)&handle, &ilayer, &pstCDK->gsCDmxOutput );
			if ( ret < 0 ) {
				ALOGE("[CDK_WRAPPER] [Err:%4d] CDMX_MPEG_AUDIO_GET_LAYER init", ret );
				return TCC_CDK_WRAPPER_DEMUXER_ERROR;
			}
		#endif

			iTryCnt++;
		}while( ilayer < 0 );

		if( ilayer == 1 )
		{
			i_atype = AUDIO_ID_MP1;
			DSTATUS( "[CDK_CORE] Audio Codec is MP1\n" );
		}
		else if( ilayer == 2 )
		{
			i_atype = AUDIO_ID_MP2;
			DSTATUS( "[CDK_CORE] Audio Codec is MP2\n" );
		}
		else if( ilayer == 3 )
		{
			i_atype = AUDIO_ID_MP3;
			DSTATUS( "[CDK_CORE] Audio Codec is MP3\n" );
		}
		else if( ilayer == 4 )
		{
			i_atype = 0xFFFFFD00;
		}	
		else
		{
			DPRINTF( "[CDK_CORE] Not supported Audio Codec Type\n");
			i_atype = CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
		}

		#ifndef TEST_CHECK_MPEG
			check_mpeg_audio_close(handle);
		#else
			ret = cdmx_audio( CDMX_MPEG_AUDIO_CLOSE, (av_handle_t*)&handle, NULL, NULL );
			if ( ret < 0 ) {
				ALOGE("[CDK_WRAPPER] [Err:%4d] CDMX_MPEG_AUDIO_CLOSE init", ret );
				return TCC_CDK_WRAPPER_DEMUXER_ERROR;
			}
		#endif

			
		ret = identify_audio_seek_zero( pCDK );

		if(ret < 0)
		{
			DPRINTF( "[CDK_CORE] Not supported Audio Codec Type\n");
			i_atype = 0xFFFFFD00;
		}
	}

	return i_atype;
}

int TCC_CDK_Wrapper::identify_dts_stream_sync(void* pCDK, unsigned char* pucBitstreamBufAddr, int iBitstreamBufSize)
{
	int			ret, i;
	int			Frame_size;
	int			iPacketNum = 0;
	int			err_type;
	
	CDK_ITEM* pstCDK = (CDK_ITEM*)pCDK;
	cdmx_input_t*	st_cdmx_input  = &pstCDK->gsCDmxInput;
	cdmx_output_t*	st_cdmx_output = &pstCDK->gsCDmxOutput;
	
	// audio stream read
	st_cdmx_input->m_iPacketType = AV_PACKET_AUDIO;

	err_type = CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
	while(1)
	{
		st_cdmx_input->m_pPacketBuff = (unsigned char*)pucBitstreamBufAddr;
		st_cdmx_input->m_iPacketBuffSize = iBitstreamBufSize;

		ret = identify_audio_get_stream( pCDK, 30 );

		if( ret < 0 )
		{
			if(ret == 0xFFFFFD00)
			{
				err_type = AUDIO_ID_DTS;
				break;
			}
			else
			{
				break;
			}
		}
		
		if(st_cdmx_output->m_iPacketSize != 0)
		{
			st_cdmx_output->m_pPacketData = (unsigned char *)pucBitstreamBufAddr;

			for( i = 0 ; i < st_cdmx_output->m_iPacketSize-3 ; i ++ )
			{
				if( (st_cdmx_output->m_pPacketData[i] == 0x7f) &&
					(st_cdmx_output->m_pPacketData[i+1] == 0xfe) &&
					(st_cdmx_output->m_pPacketData[i+2] == 0x80) &&
					(st_cdmx_output->m_pPacketData[i+3] == 0x01))
				{
					// Calculate Frame Size 
					Frame_size = ((st_cdmx_output->m_pPacketData[i+5]&0x3)<<12) | (st_cdmx_output->m_pPacketData[i+6])<<4 | (st_cdmx_output->m_pPacketData[i+7]&0xF0)>>4;
					Frame_size = Frame_size +1;
					
					err_type = AUDIO_ID_DTS;

					if (Frame_size == st_cdmx_output->m_iPacketSize)
					{
						st_cdmx_input->m_pPacketBuff = (unsigned char*)(pucBitstreamBufAddr + Frame_size);
						st_cdmx_input->m_iPacketBuffSize = iBitstreamBufSize - Frame_size;
						ret = identify_audio_get_stream( pCDK, 30 );
						st_cdmx_output->m_iPacketSize += Frame_size;
						st_cdmx_output->m_pPacketData = (unsigned char *)pucBitstreamBufAddr;
					}

					for (int j = Frame_size ; j <= st_cdmx_output->m_iPacketSize - 4 ; j++)
					{
						if( *((int*)(st_cdmx_output->m_pPacketData+j)) == 0x25205864) 
						{
							err_type = AUDIO_ID_DTSHD;			
							break;
						}
					}
					break;
				}
				else if( (st_cdmx_output->m_pPacketData[i] == 0xfe) &&
					(st_cdmx_output->m_pPacketData[i+1] == 0x7f) &&
					(st_cdmx_output->m_pPacketData[i+2] == 0x01) &&
					(st_cdmx_output->m_pPacketData[i+3] == 0x80))
				{
					// Calculate Frame Size
					Frame_size =((st_cdmx_output->m_pPacketData[i+4]&0x3)<<12) | (st_cdmx_output->m_pPacketData[i+7])<<4 | (st_cdmx_output->m_pPacketData[i+6]&0xF0)>>4;
					Frame_size = Frame_size +1;

					err_type = AUDIO_ID_DTS;

					if (Frame_size == st_cdmx_output->m_iPacketSize)
					{
						st_cdmx_input->m_pPacketBuff = (unsigned char*)(pucBitstreamBufAddr + Frame_size);
						st_cdmx_input->m_iPacketBuffSize = iBitstreamBufSize - Frame_size;
						ret = identify_audio_get_stream( pCDK, 30 );
						st_cdmx_output->m_iPacketSize += Frame_size;
						st_cdmx_output->m_pPacketData = (unsigned char *)pucBitstreamBufAddr;
					}
					
					for (int j = Frame_size ; j <= st_cdmx_output->m_iPacketSize - 4 ; j++)
					{
						if( *((int*)(st_cdmx_output->m_pPacketData+j)) == 0x20256458) 
						{
							err_type = AUDIO_ID_DTSHD;			
							break;
						}
					}
					break;
				}
				else if( (st_cdmx_output->m_pPacketData[i] == 0xff) &&
					(st_cdmx_output->m_pPacketData[i+1] == 0x1f) &&
					(st_cdmx_output->m_pPacketData[i+2] == 0x00) &&
					(st_cdmx_output->m_pPacketData[i+3] == 0xe8) )
				{

					err_type = AUDIO_ID_DTS;
					break;			
				}
				else if( (st_cdmx_output->m_pPacketData[i] == 0x1f) &&
					(st_cdmx_output->m_pPacketData[i+1] == 0xff) &&
					(st_cdmx_output->m_pPacketData[i+2] == 0xe8) &&
					(st_cdmx_output->m_pPacketData[i+3] == 0x00) )
				{
					err_type = AUDIO_ID_DTS;
					break;
				}
			}
			iPacketNum++;
		}
		if(iPacketNum >= 100 || err_type == AUDIO_ID_DTS || err_type == AUDIO_ID_DTSHD) break;		
	}			

	ret = identify_audio_seek_zero( pCDK );

	if(ret < 0)
		return CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;

	return err_type;
}


int TCC_CDK_Wrapper::Recheck_AC3_DTS(void* pCDK, unsigned char* pucBitstreamBufAddr, int iBitstreamBufSize, int iDmxType, int type)
{
	int ret;
	int i_goto_enable = 1;
	int i_ret;
	CDK_ITEM* pstCDK = (CDK_ITEM*)pCDK;
	cdmx_info_t     st_cdmx_info   = pstCDK->gsCDmxInfo;

	if(st_cdmx_info.m_sFileInfo.m_iSeekable == 0)
	{
		if(type == AV_AUDIO_AC3)
		{
//#if defined(INCLUDE_AC3_DEC)
			if(AUDIO_SUPPORT[SUPPORT_AC3])
			{
				PrintInfo( "[CDK_CORE] Audio Codec is AC3\n" );
				return AUDIO_ID_AC3;
			}
			else if(AUDIO_SUPPORT[SUPPORT_DDP])
			{
				PrintInfo( "[CDK_CORE] Audio Codec is DDP\n" );
				return AUDIO_ID_DDP;
			}
//#elif defined(INCLUDE_DDP_DEC)
//			PrintInfo( "[CDK_CORE] Audio Codec is DDP\n" );
//			return AUDIO_ID_DDP;
//#endif
		}
		if(type == AV_AUDIO_DTS)
		{
			return AUDIO_ID_DTS;
		}
	}

	if(type == AV_AUDIO_AC3)
	{
//#if defined(INCLUDE_AC3_DEC)
		if(AUDIO_SUPPORT[SUPPORT_AC3])
		{
			PrintInfo( "[CDK_CORE] Audio Codec is AC3\n" );
			ret = AUDIO_ID_AC3;
		}
		else if(AUDIO_SUPPORT[SUPPORT_DDP])
		{
			PrintInfo( "[CDK_CORE] Audio Codec is DDP\n" );
			ret = AUDIO_ID_DDP;
		}
//#elif defined(INCLUDE_DDP_DEC)
//		PrintInfo( "[CDK_CORE] Audio Codec is DDP\n" );
//		ret = AUDIO_ID_DDP;
//#endif

		/* only TS, MP4 */
		if( iDmxType == CONTAINER_TYPE_MP4)
		{
			i_ret = identify_ac3_stream_sync(pCDK, pucBitstreamBufAddr, iBitstreamBufSize);
			if( CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC == i_ret)
			{
				PrintInfo( "[CDK_CORE] Unknown dolby stream\n");
				if( i_goto_enable ) 
				{
					i_goto_enable = 0;
				}
				ret =  CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
			}
			else if( i_ret == AUDIO_ID_TRUEHD )
			{
				PrintInfo( "[CDK_CORE] Audio Codec is TrueHD\n" );
				PrintInfo( "[CDK_CORE] But True-HD is not supported yet!\n" );
				ret = CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
			}
			else if(i_ret == AUDIO_ID_AC3)
			{
//#if defined(INCLUDE_AC3_DEC)
				if(AUDIO_SUPPORT[SUPPORT_AC3])
				{
					PrintInfo( "[CDK_CORE] Audio Codec is AC3\n" );
					ret = AUDIO_ID_AC3;
				}
				else if(AUDIO_SUPPORT[SUPPORT_DDP])
				{
					PrintInfo( "[CDK_CORE] Audio Codec is DDP\n" );
					ret = AUDIO_ID_DDP;
				}
//#elif defined(INCLUDE_DDP_DEC)
//				PrintInfo( "[CDK_CORE] Audio Codec is DDP\n" );
//				ret = AUDIO_ID_DDP;
//#endif
			}
		}
		else if( iDmxType == CONTAINER_TYPE_TS )
		{
			if((st_cdmx_info.m_sAudioInfo.m_ulSubType==3)) 
			{
				PrintInfo( "[CDK_CORE] Audio Codec is TrueHD\n" );
				if (AUDIO_SUPPORT[SUPPORT_AC3] == 2)
				{
					ret = AUDIO_ID_TRUEHD;
				}
				else
				{
					PrintInfo( "[CDK_CORE] But True-HD is not supported yet!\n" );
					ret = CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
				}
			}
			else if((st_cdmx_info.m_sAudioInfo.m_ulSubType==5)|| (st_cdmx_info.m_sAudioInfo.m_ulSubType==4)) 
			{
				PrintInfo( "[CDK_CORE] Audio Codec is DDP\n" );
				if (AUDIO_SUPPORT[SUPPORT_DDP])
				{
					ret = AUDIO_ID_DDP;
				}
				else
				{
					PrintInfo( "[CDK_CORE] But True-HD is not supported yet!\n" );
					ret = CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
				}
			}
			else if((st_cdmx_info.m_sAudioInfo.m_ulSubType==0)) 
			{
//#if defined(INCLUDE_AC3_DEC)
				if(AUDIO_SUPPORT[SUPPORT_AC3])
				{
					PrintInfo( "[CDK_CORE] Audio Codec is AC3\n" );
					ret = AUDIO_ID_AC3;
				}
				else if(AUDIO_SUPPORT[SUPPORT_DDP])
				{
					PrintInfo( "[CDK_CORE] Audio Codec is DDP\n" );
					ret = AUDIO_ID_DDP;
				}
//#elif defined(INCLUDE_DDP_DEC)
//				PrintInfo( "[CDK_CORE] Audio Codec is DDP\n" );
//				ret = AUDIO_ID_DDP;
//#endif
			}
			else
			{
				PrintInfo( "[CDK_CORE] It seems that this stream is not pure ac3. unsupported subtype(%d) \n", st_cdmx_info.m_sAudioInfo.m_ulSubType);
				ret = CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
			}
		}
		
		/* Recheck DTS stream */
		if(i_goto_enable == 0)
		{
			/* only TS, AVI, MKV, MP4 */
			if(iDmxType == CONTAINER_TYPE_TS || iDmxType == CONTAINER_TYPE_AVI || iDmxType == CONTAINER_TYPE_MKV || iDmxType == CONTAINER_TYPE_MP4)
			{
				i_ret = identify_dts_stream_sync(pCDK, pucBitstreamBufAddr, iBitstreamBufSize);
				if( i_ret == CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC)
				{
					PrintInfo( "[CDK_CORE] It seems that this stream is not pure DTS. \n");
					ret = CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
				}
				else if( i_ret == AUDIO_ID_DTS)
				{
					PrintInfo( "[CDK_CORE] Audio Codec is DTS\n" );
					ret =  AUDIO_ID_DTS;
				}
				else if( i_ret == AUDIO_ID_DTSHD)
				{
					PrintInfo( "[CDK_CORE] Audio Codec is DTSHD\n" );
					ret =  AUDIO_ID_DTSHD;
				}
				else
				{
					PrintInfo( "[CDK_CORE] Audio Codec is not supported\n" );
					ret =  CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
				}
			}
		}
	}
	else if(type == AV_AUDIO_DTS)
	{
		ret =  AUDIO_ID_DTS;

		/* only TS, AVI, MKV, MP4 */
		if(iDmxType == CONTAINER_TYPE_TS || iDmxType == CONTAINER_TYPE_AVI || iDmxType == CONTAINER_TYPE_MKV || iDmxType == CONTAINER_TYPE_MP4)
		{
			i_ret = identify_dts_stream_sync(pCDK, pucBitstreamBufAddr, iBitstreamBufSize);
			if( i_ret == CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC )
			{
				PrintInfo( "[CDK_CORE] It seems that this stream is not pure DTS. \n");
				if( i_goto_enable ) 
				{
					i_goto_enable = 0;
				}
				ret = CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
			}
			else if( i_ret == AUDIO_ID_DTSHD)
			{
				PrintInfo( "[CDK_CORE] Audio Codec is DTSHD\n" );
				ret =  AUDIO_ID_DTSHD;
			}
			else
			{
				PrintInfo( "[CDK_CORE] Audio Codec is DTS\n" );
				ret =  AUDIO_ID_DTS;
			}
		}

		/* Recheck AC3 stream */
		if(i_goto_enable == 0)
		{
			/* only TS, MP4 */
			if( iDmxType == CONTAINER_TYPE_MP4)
			{
				i_ret = identify_ac3_stream_sync(pCDK, pucBitstreamBufAddr, iBitstreamBufSize);
				if( CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC == i_ret)
				{
					PrintInfo( "[CDK_CORE] Unknown dolby stream\n");
					ret =  CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
				}
				else if( i_ret == AUDIO_ID_TRUEHD )
				{
					PrintInfo( "[CDK_CORE] Audio Codec is TrueHD\n" );
					PrintInfo( "[CDK_CORE] But True-HD is not supported yet!\n" );
					ret = CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
				}
				else if(i_ret == AUDIO_ID_AC3)
				{
//#if defined(INCLUDE_AC3_DEC)
					if(AUDIO_SUPPORT[SUPPORT_AC3])
					{
						PrintInfo( "[CDK_CORE] Audio Codec is AC3\n" );
						ret = AUDIO_ID_AC3;
					}
					else if(AUDIO_SUPPORT[SUPPORT_DDP])
					{
						PrintInfo( "[CDK_CORE] Audio Codec is DDP\n" );
						ret = AUDIO_ID_DDP;
					}
//#elif defined(INCLUDE_DDP_DEC)
//					PrintInfo( "[CDK_CORE] Audio Codec is DDP\n" );
//					ret = AUDIO_ID_DDP;
//#endif
				}
			}
			else if( iDmxType == CONTAINER_TYPE_TS )
			{
				PrintInfo( "[CDK_CORE] It seems that this stream is not pure ac3. \n");
				ret = CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
			}
		}

	}

	return ret;

}

int TCC_CDK_Wrapper::Recheck_MP123(void* pCDK, unsigned char* pucBitstreamBufAddr, int iBitstreamBufSize, int iDmxType, int type)
{
	int i_atype;

	if( iDmxType != CONTAINER_TYPE_AUDIO )
	{
		i_atype = identify_mpeg_stream(pCDK, pucBitstreamBufAddr, iBitstreamBufSize, iDmxType, type);
	}
	else
	{
		i_atype = type;
	}
	
	switch( i_atype )
	{
		case AUDIO_ID_MP1:
			PrintInfo( "[CDK_CORE] Audio Codec is MP1\n" );
			return AUDIO_ID_MP1;
		case AUDIO_ID_MP2:
			PrintInfo( "[CDK_CORE] Audio Codec is MP2\n" );
			return AUDIO_ID_MP2;
		case AUDIO_ID_MP3:
			PrintInfo( "[CDK_CORE] Audio Codec is MP3\n" );
			return AUDIO_ID_MP3;
		default:
			PrintInfo( "[CDK_CORE] It seems that this stream is not mpeg-audio. \n");
			return CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
	}
}

#ifdef SUPPORT_AAC_ELD_PROFILE
/* ISO/IEC 14496-3 Part 3: Table 1.14 - Syntax of GetAudioObjectType() */
static uint32_t GetAudioObjectType( ABitReader *bits)
{
	uint32_t audioObjectType = bits->getBits(5);
	if(audioObjectType == 31) 
	{
		audioObjectType = 32 + bits->getBits(6) /* audioObjectTypeExt */;
	}
	
	return audioObjectType;
}

static uint32_t GetObjectTypeFromAudioSpecificConfig(const uint8_t *pucConfig, uint32_t uiConfigLen)
{	
	if(uiConfigLen < 2)
		return -1;

	ABitReader bits(pucConfig, uiConfigLen);
	
	uint32_t audioObjectType = GetAudioObjectType(&bits);
	uint32_t samplingFequencyIndex = bits.getBits(4);
	if(samplingFequencyIndex == 0xf)
	{
		bits.getBits(24);	/* samplingFrequency */
	}
	
	bits.getBits(4);	/* channelConfiguration */
	
	if(audioObjectType == 5 || audioObjectType == 29)	// SBR or PS
	{
		samplingFequencyIndex = bits.getBits(4);	/* extensionSamplingFequencyIndex */
		if(samplingFequencyIndex == 0xf)
		{
			bits.getBits(24);	/* extensionSamplingFrequency */
		}
		audioObjectType = GetAudioObjectType(&bits);
	}
	
	return audioObjectType;
}
#endif

void TCC_CDK_Wrapper::MakeAACSpecificConfigData(uint8_t *config, uint32_t profile, uint32_t samplerate, uint32_t numChannel)
{
	/* make AudioSpecificConfigData for google aac decoder */
	static const int32_t kSamplingFreq[] = {
        96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350, 0
    };
	
	if(config != NULL)
	{
		int sampling_freq_index;
		
		for(sampling_freq_index = 0; sampling_freq_index < 14; sampling_freq_index++)
		{
			if (samplerate == kSamplingFreq[sampling_freq_index]) 
			{
				break;
			}
		}

		config[0] = ((profile + 1) << 3) | (sampling_freq_index >> 1);
	    config[1] = ((sampling_freq_index << 7) & 0x80) | (numChannel << 3);
	}
}

/****************************************************************/
#define PROPERTY_TYPE_CONTAINER 0 
#define PROPERTY_TYPE_VIDEO 1 
#define PROPERTY_TYPE_AUDIO 2 
char gs_szNameContainer[] = "tcc.container.support.";
char gs_szNameVideo[] = "tcc.video.support.";
char gs_szNameAudio[] = "tcc.audio.support.";
#define NOT_SUPPORTED_PROP_VALUE -1
int32_t
TCC_CDK_Wrapper::CheckProperty(
   unsigned long ulType, 
   const char *pszExt, 
   const char *pszDef // default value is "1" true or "0" false
   )
{
    char value[PROPERTY_VALUE_MAX];
	char szName[64];
	memset(value,  0, PROPERTY_VALUE_MAX);
	memset(szName, 0, 64);

	switch (ulType) {
	case PROPERTY_TYPE_CONTAINER:
		// mp4, avi, mpg, flv, ogg, mkv, webm, mov, ts
		// wav, flac
		strcpy(szName, gs_szNameContainer);
		break;
	case PROPERTY_TYPE_VIDEO:
		// h264, mpeg4, h263, mvc, vc1, wmv7, wmv8, divx ...
		strcpy(szName, gs_szNameVideo);
		break;
	case PROPERTY_TYPE_AUDIO:
		strcpy(szName, gs_szNameAudio);
		break;
	default:
		ALOGE("Invalid value(%d) of Param1\n", ulType );
		return -2;
	}

	strcat(szName, pszExt);
	property_get(szName, value, pszDef);
	int is_supported = atoi(value);
	if (is_supported == 0) {
		ALOGE("%s is NOT supported\n", pszExt );
		return NOT_SUPPORTED_PROP_VALUE;
	}
	return 0;
}

int TCC_CDK_Wrapper::get_input_container_type(char* pFileName, const char *pszExt)
{
	void* fp_inp = cdk_fopen( pFileName, "rb" );
	char buffer[64];
	int filetype = CONTAINER_TYPE_NONE;
	if( fp_inp == NULL )
	{
		PrintError("[CDK_CORE:Err%d] get_input_container_type file open failed", -1);
		return -1;
	}
	// Check Sync Byte = MP4 File - 'FtypBoxType'
	cdk_fread( buffer, 64, 1, fp_inp );
	LOGMSG("%02x %02x %02x %02x %02x %02x", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5] );
	LOGMSG("%02x %02x %02x %02x %02x %02x", buffer[6], buffer[7], buffer[8], buffer[9], buffer[10], buffer[11] );
	if( cdk_strncmp(static_cast<char*>(&buffer[4]), "ftyp", 4) == 0 ||
		cdk_strncmp(static_cast<char*>(&buffer[4]), "skip", 4) == 0 ||
		cdk_strncmp(static_cast<char*>(&buffer[4]), "mdat", 4) == 0 ) // this file is MOV file
	{
		if ( CheckProperty(PROPERTY_TYPE_CONTAINER, "mp4") < 0 )
			return CONTAINER_TYPE_NONE;		
		PrintInfo("[CDK_WRAPPER] Container is MP4\n" );
		filetype = CONTAINER_TYPE_MP4;
	}
	else if( cdk_strncmp(static_cast<char*>(&buffer[0]), "RIFF", 4) == 0 )
	{
		if( cdk_strncmp(static_cast<char*>(&buffer[8]), "AVI ", 4) == 0 ||
			cdk_strncmp(static_cast<char*>(&buffer[8]), "AVIX", 4) == 0 )
		{
			if ( CheckProperty(PROPERTY_TYPE_CONTAINER, "avi") < 0 )
				return CONTAINER_TYPE_NONE;
			PrintInfo("[CDK_WRAPPER] Container is AVI\n" );
			filetype = CONTAINER_TYPE_AVI;
		}
		else if( cdk_strncmp(static_cast<char*>(&buffer[8]), "CDXA", 4) == 0 )
		{
			if ( CheckProperty(PROPERTY_TYPE_CONTAINER, "mpg") < 0 )
				return CONTAINER_TYPE_NONE;
			PrintInfo("[CDK_WRAPPER] Container is MPG\n" );
			filetype = CONTAINER_TYPE_MPG;
		}
		else if( cdk_strncmp(static_cast<char*>(&buffer[8]), "WAVE", 4) == 0 )
		{
			if ( CheckProperty(PROPERTY_TYPE_CONTAINER, "wav") < 0 )
				return CONTAINER_TYPE_NONE;
			PrintInfo("[CDK_WRAPPER] Container is WAV\n" );
			filetype = CONTAINER_TYPE_AUDIO;
		}
	}
	else if ( cdk_strncmp(static_cast<char*>(&buffer[0]), "FLV", 3) == 0)
	{
		if ( CheckProperty(PROPERTY_TYPE_CONTAINER, "flv") < 0 )
			return CONTAINER_TYPE_NONE;
		PrintInfo("[CDK_WRAPPER] Container is FLV\n" );
		filetype = CONTAINER_TYPE_FLV;
	}
	else if(cdk_strncmp(static_cast<char*>(&buffer[0]), ".RMF", 4) == 0 )
	{
		// default is false
		if ( CheckProperty(PROPERTY_TYPE_CONTAINER, "ext1", "0") < 0 )
			return CONTAINER_TYPE_NONE;
		PrintInfo("[CDK_WRAPPER] Container is EXT1\n" );
		filetype = CONTAINER_TYPE_EXT1;
	}
	else if(cdk_strncmp(static_cast<char*>(&buffer[0]), "OggS", 4) == 0 )
	{
		if ( CheckProperty(PROPERTY_TYPE_CONTAINER, "ogg") < 0 )
			return CONTAINER_TYPE_NONE;
		PrintInfo("[CDK_WRAPPER] Container is OGG\n" );
		filetype = CONTAINER_TYPE_OGG;
	}
	// Check Sync Byte ASF file
	else if( buffer[0] == 0x30 )
	{
		unsigned int rebuf[4];
		int i;
		for (i=0;i<4;i++)
		{
			rebuf[i] = static_cast<unsigned long>(buffer[4*i]&0xFF)<<24  | static_cast<unsigned long>(buffer[4*i+1]&0xFF)<<16 |
				static_cast<unsigned long>(buffer[4*i+2]&0xFF)<<8 | static_cast<unsigned long>(buffer[4*i+3]&0xFF) ;
		}

		if(	   (rebuf[0] == 0x3026b275) && (rebuf[1] == 0x8e66cf11) 
				&& (rebuf[2] == 0xa6d900aa) && (rebuf[3] == 0x0062ce6c) )
		{
			if ( CheckProperty(PROPERTY_TYPE_CONTAINER, "asf", "0") < 0 )
				return CONTAINER_TYPE_NONE;
			PrintInfo("[CDK_WRAPPER] Container is ASF\n" );
			filetype = CONTAINER_TYPE_ASF;
		}
	}
	else if( buffer[0] == 0x1a && buffer[1] == 0x45 && buffer[2] == 0xdf && buffer[3] == 0xa3)
	{
		uint64_t filesize = 0;
		GetCurrentFileSize(filesize);
		if(filesize < 0x7fffffff)
		{
			cdk_fseek(fp_inp, 4, SEEK_SET);
		}
		else
		{
			cdk_fseek64(fp_inp, 4, SEEK_SET);
		}
		
		char p[50];
		cdk_fread(p, 1, 50, fp_inp);
		
		int count = 0;
        for (count = 0 ; count < 40 ; count++)
        {
            if (p[count] == 0x42 && p[count+1] == 0x82 )
            {
                if ( strncmp(static_cast<char*>(&p[count+3]), "matroska", 8) == 0 )
				{
					if ( CheckProperty(PROPERTY_TYPE_CONTAINER, "mkv") < 0 )
						return CONTAINER_TYPE_NONE;
					PrintInfo("[CDK_WRAPPER] Container is mkv\n" );
					filetype = CONTAINER_TYPE_MKV;
				}
                else if ( strncmp(static_cast<char*>(&p[count+3]), "webm", 4) == 0 )
				{
					if ( CheckProperty(PROPERTY_TYPE_CONTAINER, "webm") < 0 )
						return CONTAINER_TYPE_NONE;
					PrintInfo("[CDK_WRAPPER] Container is WebM\n" );
					filetype = CONTAINER_TYPE_MKV;
				}
            }
        }
	}
	else if ( strncmp(static_cast<char*>(&buffer[4]), "ftypqt", 6) == 0)
	{
		if ( CheckProperty(PROPERTY_TYPE_CONTAINER, "mov") < 0 )
			return CONTAINER_TYPE_NONE;
		PrintInfo("[CDK_WRAPPER] Container is MOV\n" );
		filetype = CONTAINER_TYPE_MP4; // mov
	}
	else if ( buffer[0] == 0x00 && buffer[1] == 0x00 && buffer[2] == 0x01 && buffer[3] == 0xba)
	{
		if (CheckProperty(PROPERTY_TYPE_CONTAINER, "mpg") < 0) 
			return CONTAINER_TYPE_NONE;
		PrintInfo("[CDK_WRAPPER] Container is MPG\n" );
		filetype = CONTAINER_TYPE_MPG;
	}
	else if ( buffer[0] == 0x47 && buffer[1] == 0x40)
	{
		if (CheckProperty(PROPERTY_TYPE_CONTAINER, "ts") < 0) 
			return CONTAINER_TYPE_NONE;
		PrintInfo("[CDK_WRAPPER] Container is TS\n" );
		filetype = CONTAINER_TYPE_TS;
	}
	else	
	{
		uint64_t filesize = 0;
		GetCurrentFileSize(filesize);
		if(filesize < 0x7fffffff)
		{
			cdk_fseek(fp_inp, 0, SEEK_SET);
		}
		else
		{
			cdk_fseek64(fp_inp, 0, SEEK_SET);
		}
		char p[1024];
		// skip id3 tag :: START
		unsigned int id3tagsize = 0;
		cdk_fread(p, 1, 10, fp_inp);
		if (!cdk_strncmp(p, "ID3", 3)) {
			id3tagsize = (p[6] << 21) | (p[7] << 14) | (p[8] <<  7) | (p[9] <<  0);
			id3tagsize += 10;
		}
		if(filesize < 0x7fffffff)
		{
			cdk_fseek(fp_inp, id3tagsize, SEEK_SET);
		}
		else
		{
			cdk_fseek64(fp_inp, id3tagsize, SEEK_SET);
		}
		// skip id3 tag :: END
		cdk_fread(p, 1, 1024, fp_inp);

		uint32_t count;

		if(p[0] == 'f' && p[1] == 'L' && p[2] == 'a' && p[3] == 'C')	// TELECHIPS, LJH, 2011.03.10
		{
			if (CheckProperty(PROPERTY_TYPE_CONTAINER, "flac") < 0) 
				return CONTAINER_TYPE_NONE;
			PrintInfo("[CDK_WRAPPER] Container is FLAC\n" );
			filetype = CONTAINER_TYPE_AUDIO;
		}

		if ( filetype == CONTAINER_TYPE_NONE )
		{
			for (count = 1 ; count < 1024-3 ; count++)
			{
				if (p[count] == 0 && p[count+1] == 0 && p[count+2] == 0x01 && p[count+3] == 0xba)
				{
					if (CheckProperty(PROPERTY_TYPE_CONTAINER, "mpg") < 0) 
						return CONTAINER_TYPE_NONE;
					PrintInfo("[CDK_WRAPPER] Container is MPG\n" );
					filetype = CONTAINER_TYPE_MPG;
					break;
				}
			}
		}

		if ( filetype == CONTAINER_TYPE_NONE )
		{
			for (count = 0; count < 512; count++) /* up to 1024 */
			{
				if (0x47 == p[count])
				{
					if (    ((0x47 == p[count + (188 * 1)]) && (0x47 == p[count + (188 * 2)]))
					     || ((0x47 == p[count + (192 * 1)]) && (0x47 == p[count + (192 * 2)])) )
					{
						if (CheckProperty(PROPERTY_TYPE_CONTAINER, "ts") < 0) 
							return CONTAINER_TYPE_NONE;
						PrintInfo("[CDK_WRAPPER] Container is TS\n" );
						filetype = CONTAINER_TYPE_TS;
						break;
					}
				}
			}
		}

		if ( filetype == CONTAINER_TYPE_NONE )
		{
			for (count = 1 ; count < 1024-3 ; count++)
			{
				if (p[count] == 'R' && p[count+1] == 'I' && p[count+2] == 'F' && p[count+3] == 'F')
				{
					if (p[count+8] == 'A' && p[count+9] == 'V' && p[count+10] == 'I' && p[count+11] == ' ')
					{
						if (CheckProperty(PROPERTY_TYPE_CONTAINER, "avi") < 0) 
							return CONTAINER_TYPE_NONE;
						PrintInfo("[CDK_WRAPPER] Container is AVI\n" );
						filetype = CONTAINER_TYPE_AVI;
						break;
					}
					if (p[count+8] == 'A' && p[count+9] == 'V' && p[count+10] == 'I' && p[count+11] == 'X')
					{
						if (CheckProperty(PROPERTY_TYPE_CONTAINER, "avi") < 0) 
							return CONTAINER_TYPE_NONE;
						PrintInfo("[CDK_WRAPPER] Container is AVI\n" );
						filetype = CONTAINER_TYPE_AVI;
						break;
					}
				}
			}
		}

		//filename extension Sniff.
		if ( filetype == CONTAINER_TYPE_NONE && pszExt )
		{
			if( ( cdk_strncmp(pszExt, "MP4", 3) == 0 ) ||
				( cdk_strncmp(pszExt, "MOV", 3) == 0 ) ||
				( cdk_strncmp(pszExt, "3GP", 3) == 0 ) )
			{
				if (CheckProperty(PROPERTY_TYPE_CONTAINER, "mp4") < 0) 
					return CONTAINER_TYPE_NONE;
				if (CheckProperty(PROPERTY_TYPE_CONTAINER, "mov") < 0) 
					return CONTAINER_TYPE_NONE;
				if (CheckProperty(PROPERTY_TYPE_CONTAINER, "3gp") < 0) 
					return CONTAINER_TYPE_NONE;
				PrintInfo("[CDK_WRAPPER] Container is MP4\n");
				filetype = CONTAINER_TYPE_MP4;
			}
			else if( ( cdk_strncmp(pszExt, "MPG", 3) == 0 ) ||
					 ( cdk_strncmp(pszExt, "VOB", 3) == 0 ) )
			{
				if (CheckProperty(PROPERTY_TYPE_CONTAINER, "mpg") < 0) 
					return CONTAINER_TYPE_NONE;
				if (CheckProperty(PROPERTY_TYPE_CONTAINER, "vob") < 0) 
					return CONTAINER_TYPE_NONE;
				PrintInfo("[CDK_WRAPPER] Container is MPG\n");
				filetype = CONTAINER_TYPE_MPG;
			}
			else if( ( cdk_strncmp(pszExt, "MKV", 3) == 0 ) ||
					 ( cdk_strncmp(pszExt, "WEBM", 4) == 0 ) )
			{
				if (CheckProperty(PROPERTY_TYPE_CONTAINER, "mkv") < 0) 
					return CONTAINER_TYPE_NONE;
				PrintInfo("[CDK_WRAPPER] Container is MKV\n");
				filetype = CONTAINER_TYPE_MKV;
			}
			else if( cdk_strncmp(pszExt, "AVI", 3) == 0 )
			{
				if (CheckProperty(PROPERTY_TYPE_CONTAINER, "avi") < 0) 
					return CONTAINER_TYPE_NONE;
				PrintInfo("[CDK_WRAPPER] Container is AVI\n");
				filetype = CONTAINER_TYPE_AVI;
			}
			else if( cdk_strncmp(pszExt, "TS", 2) == 0 ||
					 cdk_strncmp(pszExt, "TP", 2) == 0 ||
					 cdk_strncmp(pszExt, "TRP", 3) == 0 ||
					 cdk_strncmp(pszExt, "M2TS", 4) == 0 )
			{
				if (CheckProperty(PROPERTY_TYPE_CONTAINER, "ts") < 0) 
					return CONTAINER_TYPE_NONE;
				PrintInfo("[CDK_WRAPPER] Container is TS\n");
				filetype = CONTAINER_TYPE_TS;
			}
			else if( ( cdk_strncmp(pszExt, "WMV", 3) == 0 ) ||
					 ( cdk_strncmp(pszExt, "ASF", 3) == 0 ) ||
					 ( cdk_strncmp(pszExt, "WMA", 3) == 0 ) )
			{
				if (CheckProperty(PROPERTY_TYPE_CONTAINER, "asf", "0") < 0) 
					return CONTAINER_TYPE_NONE;
				PrintInfo("[CDK_WRAPPER] Container is ASF\n");
				filetype = CONTAINER_TYPE_ASF;
			}
			else
			{
				filetype = CONTAINER_TYPE_NONE;
			}
		}
		
		if ( filetype == CONTAINER_TYPE_NONE )
			filetype = CONTAINER_TYPE_AUDIO;
	}

	cdk_fclose( fp_inp );

	return filetype;
}

int TCC_CDK_Wrapper::get_video_codec_type( int iFourCC )
{
    int fourcc = 0;

#define av_toupper(ch) \
	( (ch) >= 'a' && (ch) <= 'z') ? (ch) - 32: (ch)

    fourcc = ( (av_toupper((iFourCC >> 0) & 255)) <<  0 ) |	
             ( (av_toupper((iFourCC >> 8) & 255)) <<  8 ) |	
             ( (av_toupper((iFourCC >>16) & 255)) << 16 ) |	
             ( (av_toupper((iFourCC >>24) & 255)) << 24 );

#undef av_toupper

	switch( fourcc ) 
	{
	case FOURCC_AVC1: case FOURCC_H264: case FOURCC_X264: case FOURCC_VSSH: case FOURCC_DAVC: case FOURCC_Z264:
		if (CheckProperty(PROPERTY_TYPE_VIDEO, "h264") < 0) 
			return CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC;
		PrintInfo("[CDK_WRAPPER] Video Codec is H.264\n" );
		return STD_AVC;

	case FOURCC_MVC:
		if (CheckProperty(PROPERTY_TYPE_VIDEO, "mvc") < 0) 
			return CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC;
		PrintInfo("[CDK_WRAPPER] Video Codec is MVC\n" );
		return STD_MVC;

    case FOURCC_S263: case FOURCC_H263:
		if (CheckProperty(PROPERTY_TYPE_VIDEO, "h263") < 0) 
			return CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC;
		PrintInfo("[CDK_WRAPPER] Video Codec is H.263\n" );
		return STD_H263;

	case FOURCC_FLV1:
		if (CheckProperty(PROPERTY_TYPE_VIDEO, "sorensonh263") < 0) 
			return CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC;
		PrintInfo( "[CDK_WRAPPER] Video Codec is Sorenson's H.263\n" );
    #ifdef INCLUDE_SORENSON263_DEC
		return STD_SORENSON263; // s/w codec
    #else
		return STD_SH263; // vpu
    #endif

    case FOURCC_DX50:
		if (CheckProperty(PROPERTY_TYPE_VIDEO, "divx5") < 0) 
			return CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC;
		PrintInfo("[CDK_WRAPPER] Video Codec is DIVX5\n" );
		return STD_MPEG4;

	case FOURCC_DIVX:
		if (CheckProperty(PROPERTY_TYPE_VIDEO, "divx") < 0) 
			return CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC;
		PrintInfo("[CDK_WRAPPER] Video Codec is DIVX\n" );
		return STD_MPEG4;

    case FOURCC_DIV3:
		if (CheckProperty(PROPERTY_TYPE_VIDEO, "divx3") < 0) 
			return CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC;
        PrintInfo("[CDK_WRAPPER] Video Codec is DIVX3\n" );
		return STD_DIV3; 

    case FOURCC_DIV4:
		if (CheckProperty(PROPERTY_TYPE_VIDEO, "divx4") < 0) 
			return CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC;
        PrintInfo("[CDK_WRAPPER] Video Codec is DIVX4\n" );
		return STD_DIV3; 

	case FOURCC_XVID:
		if (CheckProperty(PROPERTY_TYPE_VIDEO, "xvid") < 0) 
			return CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC;
    case FOURCC_MP4V: case FOURCC_3IV2: case FOURCC_FMP4: case FOURCC_DIGI:
	case FOURCC_MP4S: case FOURCC_M4S2: case FOURCC_SEDG: case FOURCC_RMP4:
    case FOURCC_DVX1: case FOURCC_DIVF: case FOURCC_FVFW: case FOURCC_FMD4:
    case FOURCC_UMP4: case FOURCC_SMP4: case FOURCC_WV1F: case FOURCC_EM4A:
		if (CheckProperty(PROPERTY_TYPE_VIDEO, "mpeg4") < 0) 
			return CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC;
		PrintInfo("[CDK_WRAPPER] Video Codec is MPEG-4\n" );
		return STD_MPEG4;

    case FOURCC_MP43:
		if (CheckProperty(PROPERTY_TYPE_VIDEO, "msmpeg4v3") < 0) 
			return CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC;
        PrintInfo("[CDK_WRAPPER] Video Codec is MP43\n" );
		return STD_DIV3; 

    case FOURCC_MPEG: case FOURCC_MPG1: case FOURCC_M1V1: 
		if (CheckProperty(PROPERTY_TYPE_VIDEO, "mpeg1") < 0) 
			return CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC;
		PrintInfo("[CDK_WRAPPER] Video Codec is MPEG-1\n" );
		return STD_MPEG2;

	case FOURCC_MP2V: case FOURCC_MPG2: case FOURCC_M2V1: case FOURCC_HDV1: case FOURCC_HDV2: 
		if (CheckProperty(PROPERTY_TYPE_VIDEO, "mpeg2") < 0) 
			return CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC;
		PrintInfo("[CDK_WRAPPER] Video Codec is MPEG-2\n" );
		return STD_MPEG2;

	case FOURCC_RV30: case FOURCC_RV40: case FOURCC_RV89COMBO: 
		if (CheckProperty(PROPERTY_TYPE_VIDEO, "rv", "0") < 0) 
			return CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC;
        PrintInfo("[CDK_WRAPPER] Video Codec is RV\n" );
		return STD_RV;

    case FOURCC_V_VP: case FOURCC_VP80:
		if (CheckProperty(PROPERTY_TYPE_VIDEO, "vp8") < 0) 
			return CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC;
        PrintInfo("[CDK_WRAPPER] Video Codec is VP8\n" );
        return STD_VP8;

    case FOURCC_VP90:
		if (CheckProperty(PROPERTY_TYPE_VIDEO, "vp9") < 0) 
			return CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC;
        PrintInfo("[CDK_WRAPPER] Video Codec is VP9\n" );
        return CODEC_VP9;

	case FOURCC_WMV3: case FOURCC_WMV9: 
	case FOURCC_WMVA: case FOURCC_WVC1: 
	case FOURCC_WMVP: case FOURCC_WMVR: 
	case FOURCC_VC1:
		if (CheckProperty(PROPERTY_TYPE_VIDEO, "vc1", "0") < 0) 
			return CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC;
		PrintInfo("[CDK_WRAPPER] Video Codec is VC-1\n" );
		return STD_VC1;

#ifdef INCLUDE_WMV78_DEC
	case FOURCC_WMV1: 
		if (CheckProperty(PROPERTY_TYPE_VIDEO, "wmv7", "0") < 0) 
			return CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC;
		PrintInfo( "[CDK_WRAPPER] Video Codec is WMV7\n" );
		return STD_WMV78;

	case FOURCC_WMV2:
		if (CheckProperty(PROPERTY_TYPE_VIDEO, "wmv8", "0") < 0) 
			return CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC;
		PrintInfo( "[CDK_WRAPPER] Video Codec is WMV8\n" );
		return STD_WMV78;
#endif

#if !defined(TCC_89XX_INCLUDE)
//#if !defined(TCC_8925S_INCLUDE) && !defined(TCC_8935S_INCLUDE)
    case FOURCC_MJPG: case FOURCC_IJPG: case FOURCC_AVRN: case FOURCC_JPEG:
		if (CheckProperty(PROPERTY_TYPE_VIDEO, "mjpeg") < 0) 
			return CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC;
		PrintInfo( "[CDK_WRAPPER] Video Codec is MJPEG\n" );
		return STD_MJPG;
//#endif
    case FOURCC_AVS: case FOURCC_CAVS:
		if (CheckProperty(PROPERTY_TYPE_VIDEO, "avs") < 0) 
			return CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC;
		PrintInfo( "[CDK_WRAPPER] Video Codec is AVS\n" );
		return STD_AVS;
#endif		

	default:
		PrintInfo("[CDK_WRAPPER] get_video_codec_type failed (0x%0x)\n", iFourCC );
		return CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC;
	}

	return 0;
}

int TCC_CDK_Wrapper::get_audio_codec_type( void* pCDK, int iFourId, int iDmxType, uint32_t iOpenMode )
{
	int		value;
	
	switch( iFourId ) 
	{
	case AV_AUDIO_AAC: case AV_AUDIO_MS_AAC: case AV_AUDIO_MS_AAC2:
	case AV_AUDIO_RAAAC: case AV_AUDIO_RACP: case AV_AUDIO_AAC_706D:
		if (CheckAudioCodecUse(0, SUPPORT_AAC))
		{
#ifdef SUPPORT_AAC_ELD_PROFILE
			CDK_ITEM* pstCDK = static_cast<CDK_ITEM*>(pCDK);
			cdmx_info_t* pst_cdmx_info = &pstCDK->gsCDmxInfo;
#endif			

			PrintInfo("[CDK_WRAPPER] Audio Codec is AAC\n" );
			
#ifdef SUPPORT_AAC_ELD_PROFILE
			if(pst_cdmx_info->m_sAudioInfo.m_pExtraData && pst_cdmx_info->m_sAudioInfo.m_iExtraDataLen >= 2)
			{
				uint32_t audioObjectType = GetObjectTypeFromAudioSpecificConfig((const uint8_t *)pst_cdmx_info->m_sAudioInfo.m_pExtraData, 
																				pst_cdmx_info->m_sAudioInfo.m_iExtraDataLen);
				
				if ( audioObjectType == 17 /* Error Resilient(ER) AAC Low Complexity */
				  || audioObjectType == 23 /* Error Resilient(ER) AAC Low Delay */
				  || audioObjectType == 39 /* Error Resilient(ER) AAC Enhanced Low Delay */)
				{
					ALOGD("use google aac decoder (Fraunhofer FDK AAC Codec) to decode this objecttype(%d)", audioObjectType);	
					return AUDIO_ID_AAC_GOOGLE;
				}				
				else		
				{
					//ALOGD("use tcc aac decoder to decode this objecttype(%d)", audioObjectType);	
					return AUDIO_ID_AAC;
				}
			}
			else
#endif			
			return AUDIO_ID_AAC;
		}
		break;
	case AV_AUDIO_MP3:
		if (CheckAudioCodecUse(0, SUPPORT_MP3))
		{
			if (iOpenMode & CDMX_MARKER_MEDIA_SCAN_MODE_BIT)
				return AUDIO_ID_MP3;
			value = identify_mpeg_stream(pCDK, pStreamBuffer, AUDIO_BUFFER_SIZE, iDmxType, AV_AUDIO_MP3);
			if(value == 0xFFFFFD00)
			{
				PrintInfo("[CDK_WRAPPER] Audio Codec is MP3\n" );
				return AUDIO_ID_MP3;
			}
			return value;
		}
		break;
	case AV_AUDIO_AMR_NB:
		if (CheckAudioCodecUse(0, SUPPORT_AMR))
		{			
			PrintInfo("[CDK_WRAPPER] Audio Codec is AMR\n" );
			return AUDIO_ID_AMR;
		}
		break;
	case AV_AUDIO_AMR_WB:
	case AV_AUDIO_AMR_WP:	
		if (CheckAudioCodecUse(0, SUPPORT_AMRWBP))
		{
			PrintInfo("[CDK_WRAPPER] Audio Codec is AMR WB or WB+\n" );
			return AUDIO_ID_AMRWBP;
		}
		break;
//#if defined(INCLUDE_AC3_DEC)
	case AV_AUDIO_AC3:
	case AV_AUDIO_RAAC3:
	case AV_AUDIO_FORCC_AC3:		
		if (iOpenMode & CDMX_MARKER_MEDIA_SCAN_MODE_BIT)
			return AUDIO_ID_AC3;
		if ( (AUDIO_SUPPORT[SUPPORT_AC3]) || (AUDIO_SUPPORT[SUPPORT_DDP]) )
		{
			value = Recheck_AC3_DTS(pCDK, pStreamBuffer, VIDEO_BUFFER_SIZE, iDmxType, AV_AUDIO_AC3);
			PrintInfo("[CDK_WRAPPER] Audio Codec is 0x%x\n", value );
			return value;
		}
		else
		{
			return CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
		}
	case AV_AUDIO_EAC3:
		if (AUDIO_SUPPORT[SUPPORT_DDP])
		{
			PrintInfo( "[CDK_WRAPPER] Audio Codec is DDP using DDPDEC library\n" );
			return AUDIO_ID_DDP;
		}
		else
		{
			return CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
		}
//#elif defined(INCLUDE_DDP_DEC)
//	case AV_AUDIO_AC3:
//	case AV_AUDIO_RAAC3:
//    case AV_AUDIO_FORCC_AC3:		
//		if (iOpenMode & CDMX_MARKER_MEDIA_SCAN_MODE_BIT)
//			return AUDIO_ID_AC3;
//		value = Recheck_AC3_DTS(pCDK, pStreamBuffer, VIDEO_BUFFER_SIZE, iDmxType, AV_AUDIO_AC3);
//		return value;
//	case AV_AUDIO_EAC3:		
//		PrintInfo( "[CDK_WRAPPER] Audio Codec is DDP using DDPDEC library\n" );
//		return AUDIO_ID_DDP;
//#endif
	case AV_AUDIO_MS_PCM:
	case AV_AUDIO_MS_PCM_SWAP:
	case AV_AUDIO_MS_EXTENSIBLE:
		if (CheckAudioCodecUse(0, SUPPORT_WAV))
		{
			PrintInfo("[CDK_WRAPPER] Audio Codec is PCM\n" );
			return AUDIO_ID_WAV;
		}
		break;
	case AV_AUDIO_MP2:
		if (CheckAudioCodecUse(0, SUPPORT_MP2))
		{
			PrintInfo("[CDK_WRAPPER] Audio Codec is MP2\n" );
			return AUDIO_ID_MP2;
		}
		break;
	case AV_AUDIO_DTS:
		if (iOpenMode & CDMX_MARKER_MEDIA_SCAN_MODE_BIT)
			return AUDIO_ID_DTS;
		if(AUDIO_SUPPORT[SUPPORT_DTS])
		{
			value = Recheck_AC3_DTS(pCDK, pStreamBuffer, VIDEO_BUFFER_SIZE, iDmxType, AV_AUDIO_DTS);
			PrintInfo("[CDK_WRAPPER] Audio Codec is 0x%x\n", value );
			return value;
		}
		else
		{
			PrintInfo("[CDK_WRAPPER] Audio Codec is DTS - Not supported!!!\n" );
			return CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
		}
	case AV_AUDIO_QCELP:
		if(CheckAudioCodecUse(0, SUPPORT_QCELP))
		{
			PrintInfo("[CDK_WRAPPER] Audio Codec is QCELP\n" );
			return AUDIO_ID_QCELP;
		}
		break;
	case AV_AUDIO_MS_AUDIO1:
		if(CheckAudioCodecUse(0, SUPPORT_WMA))
		{	
			PrintInfo("[CDK_WRAPPER] Audio Codec is WMA Standard V1\n" );
			return AUDIO_ID_WMA;
		}
		break;
	case AV_AUDIO_WMA_STANDARD:	
		if(CheckAudioCodecUse(0, SUPPORT_WMA))
		{
			PrintInfo("[CDK_WRAPPER] Audio Codec is WMA Standard V2\n" );
			return AUDIO_ID_WMA;
		}
		break;
	case AV_AUDIO_WMA_PRO:
		if(CheckAudioCodecUse(0, SUPPORT_WMA))
		{
			PrintInfo("[CDK_WRAPPER] Audio Codec is WMA Professional\n" );
			return AUDIO_ID_WMA;
		}
		break;
	case AV_AUDIO_WMA_LOSSLESS:	
		if(CheckAudioCodecUse(0, SUPPORT_WMA))
		{
			PrintInfo("[CDK_WRAPPER] Audio Codec is WMA Lossless\n" );
			return AUDIO_ID_WMA;
		}
		break;
	case AV_AUDIO_WMA_VOICE:
		if(CheckAudioCodecUse(0, SUPPORT_WMA))
		{
			PrintInfo("[CDK_WRAPPER] Audio Codec is WMA Voice\n" );
			return AUDIO_ID_WMA;
		}
		break;
	case AV_AUDIO_EVRC:
	case AV_AUDIO_EVRC_B:
		if(CheckAudioCodecUse(0, SUPPORT_EVRC))
		{
			PrintInfo("[CDK_WRAPPER] Audio Codec is EVRC\n" );
			return AUDIO_ID_EVRC;
		}
		break;
	case AV_AUDIO_COOK:
	case AV_AUDIO_cook:
	case AV_AUDIO_kooc:
		if(AUDIO_SUPPORT[SUPPORT_RA])
		{
			PrintInfo("[CDK_WRAPPER] Audio Codec is COOK\n" );
			return AUDIO_ID_COOK;
		}
		break;
	case AV_AUDIO_FLAC:
		if(CheckAudioCodecUse(0, SUPPORT_FLAC))
		{
			PrintInfo("[CDK_WRAPPER] Audio Codec is FLAC\n" );
			return AUDIO_ID_FLAC;
		}
		break;
	case AV_AUDIO_APE:
		if(CheckAudioCodecUse(0, SUPPORT_APE))
		{
			PrintInfo("[CDK_WRAPPER] Audio Codec is APE\n" );
			return AUDIO_ID_APE;
		}
		break;
	case AV_AUDIO_MS_ADPCM:
		if(CheckAudioCodecUse(0, SUPPORT_WAV))
		{
			PrintInfo("[CDK_WRAPPER] Audio Codec is MSADPCM\n" );
			return AUDIO_ID_WAV;
		}
		break;
	case AV_AUDIO_INTEL_DVI_ADPCM:
		if(CheckAudioCodecUse(0, SUPPORT_WAV))
		{
			PrintInfo("[CDK_WRAPPER] Audio Codec is IMAADPCM\n" );
			return AUDIO_ID_WAV;
		}
		break;
	case AV_AUDIO_QT_IMA_ADPCM:
		if(CheckAudioCodecUse(0, SUPPORT_WAV))
		{
			PrintInfo("[CDK_CORE] Audio Codec is QuickTime IMAADPCM\n" );
			return AUDIO_ID_WAV;
		}
		break;
	case AV_AUDIO_MS_ALAW:
		if(CheckAudioCodecUse(0, SUPPORT_WAV))
		{
			PrintInfo("[CDK_CORE] Audio Codec is A-LAW\n" );
			return AUDIO_ID_WAV;
		}
		break;
	case AV_AUDIO_MS_ULAW:
		if(CheckAudioCodecUse(0, SUPPORT_WAV))
		{
			PrintInfo("[CDK_CORE] Audio Codec is U-LAW\n" );
			return AUDIO_ID_WAV;
		}
		break;
	case AV_AUDIO_OGG_VORBIS:
	case AV_AUDIO_OGG_VORBIS_MODE1:	
	case AV_AUDIO_OGG_VORBIS_MODE1P:	
	case AV_AUDIO_OGG_VORBIS_MODE2:	
	case AV_AUDIO_OGG_VORBIS_MODE2P:	
	case AV_AUDIO_OGG_VORBIS_MODE3:	
	case AV_AUDIO_OGG_VORBIS_MODE3P:	
	case AV_AUDIO_OGG_VORBIS_PACKET:	
	case AV_AUDIO_OGG_VORBIS_INTERNAL_VIDEO :
	case AV_AUDIO_OGG_VORBIS_INTERNAL_AUDIO :
		if(CheckAudioCodecUse(0, SUPPORT_VORBIS))
		{
			PrintInfo("[CDK_WRAPPER] Audio Codec is VORBIS\n" );
			return AUDIO_ID_VORBIS;	
		}
		break;
	case AV_AUDIO_BSAC_MP4:
	case AV_AUDIO_BSAC_CDK:
		if(CheckAudioCodecUse(0, SUPPORT_BSAC))
		{
			PrintInfo("[CDK_WRAPPER] Audio Codec is BSAC\n" );
			return AUDIO_ID_BSAC;	
		}
		break;
	case AV_AUDIO_MP3HD:
		if(CheckAudioCodecUse(0, SUPPORT_MP3))
		{
			PrintInfo("[CDK_WRAPPER] Audio Codec is mp3HD\n" );
			return AUDIO_ID_MP3HD;
		}
		break;
	case AV_AUDIO_MP1:
		if(CheckAudioCodecUse(0, SUPPORT_MP1))
		{
			PrintInfo("[CDK_WRAPPER] Audio Codec is MP1\n" );
			return AUDIO_ID_MP1;
		}
		break;
	case AUDIO_ID_TRUEHD:
		PrintInfo( "[CDK_WAPPER] Audio Codec is TrueHD\n" );
		if (AUDIO_SUPPORT[SUPPORT_AC3] < 2)
		{
			PrintInfo( "[CDK_WRAPPER] But True-HD is not supported yet!\n" );
			return CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
		}
		else
		{
			return AUDIO_ID_TRUEHD;
		}
	default:
		PrintInfo("[CDK_WRAPPER] Not supported Audio Codec Type iFourId(%d) FIXME\n", iFourId );
		return CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
	}

	return CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
}

int TCC_CDK_Wrapper::callback_func_init( cdk_callback_func_t* psFunc )
{
	cdk_malloc		= psFunc->m_pfMalloc;
	cdk_nc_malloc	= psFunc->m_pfNonCacheMalloc;
	cdk_free		= psFunc->m_pfFree;
	cdk_nc_free		= psFunc->m_pfNonCacheFree;
	cdk_memcpy		= psFunc->m_pfMemcpy;
	cdk_memset		= psFunc->m_pfMemset;
	cdk_realloc		= psFunc->m_pfRealloc;
	cdk_memmove		= psFunc->m_pfMemmove;

	cdk_sys_malloc_physical_addr = psFunc->m_pfPhysicalAlloc;
	cdk_sys_free_physical_addr	 = psFunc->m_pfPhysicalFree;
	cdk_sys_malloc_virtual_addr	 = psFunc->m_pfVirtualAlloc;
	cdk_sys_free_virtual_addr	 = psFunc->m_pfVirtualFree;

	cdk_fopen		= psFunc->m_pfFopen;
	cdk_fread		= psFunc->m_pfFread;
	cdk_fseek		= psFunc->m_pfFseek;
	cdk_ftell		= psFunc->m_pfFtell;
	cdk_fwrite		= psFunc->m_pfFwrite;
	cdk_fclose		= psFunc->m_pfFclose;
	cdk_unlink		= psFunc->m_pfUnlink;

	cdk_fseek64		= psFunc->m_pfFseek64;
	cdk_ftell64		= psFunc->m_pfFtell64;
	return CDK_ERR_NONE;
}




// -----------------------------------------------------------------
// class member functions
// -----------------------------------------------------------------


// constructor
TCC_CDK_Wrapper::TCC_CDK_Wrapper() :
	iFilePtr(NULL),
	pCallback(NULL),
	mProfilingMode(false),
	mSourceScheme(FILE_SOURCE_SCHEME),
	i_ctype(0),
	iAudID(0),
	iVidID(0),
	iSubID(0),
	iAudPacketNum(AV_PACKET_AUDIO),
	iVidPacketNum(AV_PACKET_VIDEO),
	iPacketNumUnderrun(AV_PACKET_NONE),
	audio_end(false),
	video_end(false),
	codec_exist(0),
	bSentFirstVideoFrame(false),
	m_pvCdmxInst(NULL),
	iVidIDList(NULL),
	iAudIDList(NULL),
	iSubIDList(NULL),
 	iDefaultVidStreamID(-1),
 	iDefaultAudStreamID(-1),
 	iDefaultSubStreamID(-1),
	iGetMetadataMode(false),
	iGetThumbnailMode(false),
	mSeekCancelled(false),
	mLastVideoPTS(0),
	mLastAudioPTS(0),
	bIsUnderSkimming(false),//JS Baek
	bPrepared(false),
#ifdef MERGE_FRAME_AVG
	m_lPrevStatus(0), // for Defragmentation
	m_lSpsPpsDect(0),
	m_pbyTempBuffBackup(NULL),
	m_ulPreviousTime(0),
	m_bSeekFlag(0),
    m_lStartPos(0),
    m_lEndPos(0),
    m_lDataLen(0),
    m_lAudUsed(-1),
    m_lVideoCodecType(0),
	m_lFrame_complete(0),
	m_lUsed_bytes(0),
	m_lRead_bytes(0),
	m_lFrame_size(0),
	m_ulSync(0),
#endif
	bIsQueueEmpty(false),
	pStreamBuffer(NULL),
	bIsDecryptionNeeded(false),
	mEnableHdcp2(0)
{
	FUNC("In TCC_CDK_Wrapper");

	memset(&iCdk, 0, sizeof(cdk_t));
    memset(&iFileInfo, 0, sizeof(iFileInfo));

	pItem = static_cast<CDK_ITEM_PTR>(new CDK_ITEM());

	memset(ipParserLibExist, 0, 11);

	iGetStreamMode = CDK_GETSTREAM_SELECTIVE_MODE;	// Default mode 

	mDemuxerThread = NULL;

#ifdef INTERNAL_SUBTITLE_INCLUDE
	SetSentSubtitleEvent(false);
	SetDetectIntSubtitle(false);
	set_inter_subtitle_type(TCC_MEDIAEVENT_NONE);
#endif

#ifdef PGS_CAPTION_SUPPORT_INCLUDE
	SetSentPGSEvent(false);
#endif

    char value[PROPERTY_VALUE_MAX];
    property_get("tcc.media.profile", value, "off");
    mProfilingMode = (!strcmp(value, "on")) ? true: false;
}

// destructor
TCC_CDK_Wrapper::~TCC_CDK_Wrapper()
{
	FUNC("In ~TCC_CDK_Wrapper");

	int32_t ret = CDKClose();
	if (ret != CDK_ERR_NONE)
	{
		ALOGE("CDKClose() Error %d!!\n", ret ); 
	}

	memset(&iCdk, 0, sizeof(cdk_t));

	if (pCallback)
	{
		delete pCallback;
	}

	iFilePtr = NULL;

	if (iFileInfo.iNumVidStreams > 0)
	{
		iVidInfo.removeItemsAt(0, iFileInfo.iNumVidStreams);
	}

	if (iFileInfo.iNumAudStreams > 0)
	{
		iAudInfo.removeItemsAt(0, iFileInfo.iNumAudStreams);
	}

	if (iFileInfo.iNumSubtitleStreams > 0)
	{
		iSubInfo.removeItemsAt(0, iFileInfo.iNumSubtitleStreams);
	}

	if (pStreamBuffer)
	{
		free(pStreamBuffer);
	}

	if (pItem)
	{
		delete (CDK_ITEM*)pItem;
	}
}

void TCC_CDK_Wrapper::CDKInit()
{
	FUNC("In CDKInit");

	/////////////////////////////////////////////////
	// common variable initialize
	/////////////////////////////////////////////////

	if( pStreamBuffer )
		free(pStreamBuffer);
	pStreamBuffer = static_cast<uint8_t*>(malloc(VIDEO_BUFFER_SIZE));

	// create
	for(int i = 0; i < 1024; i++ )
		iCdk.CDK_SIZE[i] = 0;

#ifdef INTERNAL_SUBTITLE_INCLUDE
	SetSentSubtitleEvent(false);
	SetDetectIntSubtitle(false);
	set_inter_subtitle_type(TCC_MEDIAEVENT_NONE);
#endif	
#ifdef PGS_CAPTION_SUPPORT_INCLUDE
	SetSentPGSEvent(false);
#endif
    bIsWFDPlayMode = false;
}

int32_t TCC_CDK_Wrapper::CDKCloseCore()
{
	int32_t ret = 0;
	
	if (m_pfnCdmx)
	{
		ret = m_pfnCdmx( CDMX_CLOSE, CDMX_HANDLE(m_pvCdmxInst), NULL, NULL );
	}
	return ret;
}

int32_t TCC_CDK_Wrapper::CDKClose()
{
	int32_t ret = 0;

	if(ipParserLibExist[i_ctype] == 1)
	{
        if (iGetStreamMode & DEMUXER_THREAD_ENABLED_MODE) {
			if( mDemuxerThread != NULL )
				ret = mDemuxerThread->Close(TCCDemuxerThread::MSG_SYNC_OFF);
        } else {
            if (iGetStreamMode & DEMUXER_LOOPER_ENABLED_MODE) {
                mCDKDemuxHandler->stop();
                mCDKDemuxHandler.clear();
                mCDKDemuxHandler = NULL;
		    }
            ret = CDKCloseCore();
        }
	}

	if( iGetStreamMode & DEMUXER_THREAD_ENABLED_MODE )
		destroyDemuxerThread();

	StreamQueue *pStreamQueue;
	int32_t size = iDemuxStreamQueues.size();
	for(int32_t i = 0; i < size; i++) {
		if( pStreamQueue = iDemuxStreamQueues.valueAt(i) )
			delete pStreamQueue;
		iDemuxStreamQueues.removeItem(i);
	}

#ifdef INTERNAL_SUBTITLE_INCLUDE
	if(GetSentSubtitleEvent() == true)
	{
		m_InterSubtitle.Destroy();
		SetSentSubtitleEvent(false);
		SetDetectIntSubtitle(false);
		set_inter_subtitle_type(TCC_MEDIAEVENT_NONE);
	}
#endif

#ifdef PGS_CAPTION_SUPPORT_INCLUDE
	if(GetSentPGSEvent() == true)
	{
		m_InterSubtitle.DestoryCaptionDisplayThread();
		SetSentPGSEvent(false);
	}
#endif	

	ret = DeleteCdmxInstance();
	return ret;
}

int TCC_CDK_Wrapper::CheckAudioCodecUse(int32_t iCheckAll, int32_t iCheckIndex)
{
	char value[PROPERTY_VALUE_MAX];
	
	if(ALL_AUDIO_CHECKED && !iCheckAll)
	{
		return AUDIO_SUPPORT[iCheckIndex];
	}
		
	for (int32_t i = (iCheckAll ? 0 : iCheckIndex) ; i < NUM_AUDIO_CODEC; i++)
	{
		if (tAudioSupportName[i] != NULL)
		{
			property_get(tAudioSupportName[i], value, "1");
			AUDIO_SUPPORT[i] = atoi(value);
			
			if(!iCheckAll)
			{
				return AUDIO_SUPPORT[iCheckIndex];
			}
		}
	}
	
	if(iCheckAll)
	{
		ALL_AUDIO_CHECKED = 1;
	}
	
	return 0;
}

int32_t TCC_CDK_Wrapper::LoadExternalSetting()
{
	int32_t ret = 0;
	char value[PROPERTY_VALUE_MAX];
	int bitstream = 0;
    
	property_get("persist.sys.spdif_setting", value, "");
	if (!strcmp(value, "2"))
	{
		bitstream = 1;
	}
	else if (!strcmp(value, "4"))
	{
		bitstream = 2;
	}
	
	/* Check Audio Codec */
	//CheckAudioCodecUse(1, 0);
	
	CheckAudioCodecUse(0, SUPPORT_AC3);
	CheckAudioCodecUse(0, SUPPORT_DDP);
	CheckAudioCodecUse(0, SUPPORT_DTS);

	// note : AC3 decoder and DDP decoder can not be applied simultaneously. 
	// So both 'AUDIO_SUPPORT[SUPPORT_AC3]' and 'AUDIO_SUPPORT[SUPPORT_DDP]' should not be active at the same time.
	if ((AUDIO_SUPPORT[SUPPORT_AC3] == 1) && (AUDIO_SUPPORT[SUPPORT_DDP] == 1))
	{
		AUDIO_SUPPORT[SUPPORT_AC3] = AUDIO_SUPPORT[SUPPORT_DDP] = 0;
		ALOGD("Can't use AC3/DDP library same time. Please Choose One!");	
	}

	if (bitstream == 1)
	{
		AUDIO_SUPPORT[SUPPORT_AC3] = 1;
		AUDIO_SUPPORT[SUPPORT_DTS] = 1;
		AUDIO_SUPPORT[SUPPORT_DDP] = 2;
	}
	else if (bitstream == 2)
	{
		// 2 means HD and non HD is separated.
		AUDIO_SUPPORT[SUPPORT_AC3] = 1;//2;
		AUDIO_SUPPORT[SUPPORT_DTS] = 2;
		AUDIO_SUPPORT[SUPPORT_DDP] = 2;
	}

	CheckAudioCodecUse(0, SUPPORT_RA);

	//CheckAudioCodecUse(0, SUPPORT_WMA);
	return TCC_CDK_WRAPPER_OK;
}

int32_t
TCC_CDK_Wrapper::CreateCdmxInstance(
    unsigned long ulCdmxType
	)
{
	FUNC("In CreateCdmxInstance\n");
	int32_t ret = CDK_ERR_NONE;

	switch (ulCdmxType)
	{
	case CONTAINER_TYPE_MKV:
		m_ulCdmxInstSize = sizeof(mkv_dmx_inst_t);
		m_pfnCdmx = cdmx_mkv;
		break;
	case CONTAINER_TYPE_MP4:
		m_ulCdmxInstSize = sizeof(mp4_dmx_inst_t);
		m_pfnCdmx = cdmx_mp4;
		break;
	case CONTAINER_TYPE_AVI:
		m_ulCdmxInstSize = sizeof(avi_dmx_inst_t);
		m_pfnCdmx = cdmx_avi;
		break;
	case CONTAINER_TYPE_AUDIO:
		m_ulCdmxInstSize = sizeof(audio_dmx_inst_t);
		m_pfnCdmx = cdmx_audio;
		break;
	case CONTAINER_TYPE_FLV:
		m_ulCdmxInstSize = sizeof(flv_dmx_inst_t);
		m_pfnCdmx = cdmx_flv;
		break;
	case CONTAINER_TYPE_MPG:
		m_ulCdmxInstSize = sizeof(mpg_dmx_inst_t);
		m_pfnCdmx = cdmx_mpg;
		break;
	case CONTAINER_TYPE_OGG:
		m_ulCdmxInstSize = sizeof(ogg_dmx_inst_t);
		m_pfnCdmx = cdmx_ogg;
		break;
	case CONTAINER_TYPE_TS:
		m_ulCdmxInstSize = sizeof(ts_dmx_inst_t);
		m_pfnCdmx = cdmx_ts;
		break;
#ifdef INCLUDE_ASF_DMX
	case CONTAINER_TYPE_ASF:
		m_ulCdmxInstSize = sizeof(asf_dmx_inst_t);
		m_pfnCdmx = cdmx_asf;
		break;
#endif
#ifdef INCLUDE_EXT1_DMX
	case CONTAINER_TYPE_EXT1:
		m_ulCdmxInstSize = sizeof(ext1_dmx_inst_t);
		m_pfnCdmx = cdmx_ext1;
		break;
#endif
	case CONTAINER_TYPE_NONE:
	default:
		return TCC_CDK_WRAPPER_ERROR;
	}

	// memory allocation
	m_pvCdmxInst = (cdmx_inst_t*)malloc( m_ulCdmxInstSize );
	if( m_pvCdmxInst == NULL )
	{
		ALOGE( "[%ld] Error: malloc failed (%d)\n", ret, __LINE__ );
		return TCC_CDK_WRAPPER_INSUFFICIENT_BUFFER_SIZE;
	}
	memset( m_pvCdmxInst, 0, m_ulCdmxInstSize );
	DSTATUS( "Allocated Instance Size: (%d)\n", m_ulCdmxInstSize );
	return ret;
}

int32_t
TCC_CDK_Wrapper::ResetCdmxInstance(void)
{
	FUNC("In ResetCdmxInstance\n");
	int32_t ret = CDK_ERR_NONE;

	if (m_pvCdmxInst) 
	{
		//memset( (void*)&(CDMX_INST(m_pvCdmxInst)->stCdmxCtrl) , 0, sizeof(cdmx_ctrl_t) );
		memset( m_pvCdmxInst, 0, m_ulCdmxInstSize );
	}
	return ret;
}

int32_t
TCC_CDK_Wrapper::DeleteCdmxInstance(void)
{
	FUNC("In DeleteCdmxInstance\n");
	int32_t ret = CDK_ERR_NONE;

	if (m_pvCdmxInst) 
	{
		free(m_pvCdmxInst);
		m_pvCdmxInst = NULL;
		m_ulCdmxInstSize = 0;
	}
	return ret;
}

String8 TCC_CDK_Wrapper::ConvertVidTypeToMimeType(unsigned long ulType)
{
	String8	mimetype; 
	char value[PROPERTY_VALUE_MAX];
	property_get("tcc.video.notsupported.config", value, "1");
	// 0: Do NOT connect on list of files
	// 1: This video cannot be played.(default)
	// 2: The file has to be played if there is even one video or audio.
	int i_flag = atoi(value);
	if (i_flag == 0) {
		ALOGE("Do NOT connect on list of files");
		mimetype = TCC_MIME_VIDEO_UNKNOWN; // don't register on gallery
	}
	else
	{
		switch (ulType)
		{
			case STD_AVC:
				mimetype = TCC_MIME_H264; 
				break;
			case STD_H263:
				mimetype = TCC_MIME_H263; 
				break;
			case STD_MPEG4:
				mimetype = TCC_MIME_MPEG4; 
				break;
			case STD_MPEG2:
				mimetype = TCC_MIME_MPEG2; 
				break;
			case STD_VC1:
				mimetype = TCC_MIME_VC1;
				break;
			case STD_DIV3:
				mimetype = TCC_MIME_DIVX; 
				break;
			case STD_RV:
				mimetype = TCC_MIME_REAL_VIDEO; 
				break;
			case STD_SH263:
				mimetype = TCC_MIME_FLV1; 
				break;
			case STD_MJPG:
				mimetype = TCC_MIME_MJPEG; 
				break;
			case STD_AVS:
				mimetype = TCC_MIME_AVS; 
				break;
			case STD_VP8:
				mimetype = TCC_MIME_VP8; 
				break;
			case STD_MVC:
				mimetype = TCC_MIME_MVC; 
				break;
		#ifdef INCLUDE_SORENSON263_DEC
			case STD_SORENSON263:
				mimetype = TCC_MIME_FLV1; 
				break;
		#endif
		#ifdef INCLUDE_WMV78_DEC
			case STD_WMV78:
				mimetype = TCC_MIME_WMV_1_2;
				break;
		#endif
			case CODEC_VP9:
				mimetype = TCC_MIME_VP9; 
				break;
			case CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC:
				mimetype = TCC_MIME_VIDEO_NOTSUPPORTED; // cannot play video
				break;
			default:
				break;
		}
	}

	return mimetype;
}

String8	TCC_CDK_Wrapper::ConvertAudTypeToMimeType(unsigned long ulType)
{
	String8	mimetype; 

	switch (ulType)
	{
		case AUDIO_ID_AAC:
			mimetype = TCC_MIME_AAC_TCC;  //TCC_MIME_MPEG4_AUDIO
			break;
		case AUDIO_ID_AAC_GOOGLE: // for google aac decoder
			mimetype = TCC_MIME_AAC_GOOGLE; 
			break; 
		case AUDIO_ID_MP3:
		#if USE_PV_MP3_CODEC
			mimetype = TCC_MIME_MP3; 
			break;
		#else
			mimetype = TCC_MIME_MP3_TCC; 
			break;
		#endif
		case AUDIO_ID_AMR:
			mimetype = TCC_MIME_AMR_NB; 
			break;
		case AUDIO_ID_AMRWBP:
			mimetype = TCC_MIME_AMR_WB; 
			break;
		case AUDIO_ID_WAV:
			mimetype = TCC_MIME_PCM_TCC; //TCC_MIME_PCM
			break; 
		case AUDIO_ID_MP2:
			mimetype = TCC_MIME_MP2; 
			break;
		case AUDIO_ID_DTS:
			if (AUDIO_SUPPORT[SUPPORT_DTS]) // AudioOutput update Planet 20121120
			{
				mimetype = TCC_MIME_DTS;
			}
			break;
		case AUDIO_ID_DTSHD:
			if (AUDIO_SUPPORT[SUPPORT_DTS] == 2)
			{
				mimetype = TCC_MIME_DTSHD;
			}
			else if (AUDIO_SUPPORT[SUPPORT_DTS] == 1)
			{
				mimetype = TCC_MIME_DTS;
			}
			break;
		case AUDIO_ID_QCELP:
			mimetype = TCC_MIME_QCELP; 
			break;
		case AUDIO_ID_WMA:
			if (AUDIO_SUPPORT[SUPPORT_WMA])
			{
				mimetype = TCC_MIME_WMA;
			}
			break;
		case AUDIO_ID_EVRC:
			mimetype = TCC_MIME_EVRC; 
			break;
		case AUDIO_ID_COOK:
			if (AUDIO_SUPPORT[SUPPORT_RA])
			{
				mimetype = TCC_MIME_REAL_AUDIO;
			}
			break;
		case AUDIO_ID_FLAC:
			mimetype = TCC_MIME_FLAC; 
			break;
		case AUDIO_ID_APE:
			mimetype = TCC_MIME_APE; 
			break;
		case AUDIO_ID_VORBIS:
			mimetype = TCC_MIME_VORBIS; 
			break;
		case AUDIO_ID_AC3:
			if (AUDIO_SUPPORT[SUPPORT_AC3] == 1 || AUDIO_SUPPORT[SUPPORT_DDP] == 1)
			{
				mimetype = TCC_MIME_AC3;
			}
			break;
		case AUDIO_ID_DDP:
			ALOGD("get mimetype AUDIO_ID_DDP AUDIO_SUPPORT[SUPPORT_DDP] %d ",AUDIO_SUPPORT[SUPPORT_DDP]);
			if (AUDIO_SUPPORT[SUPPORT_DDP] == 1)
			{
				mimetype = TCC_MIME_AC3;
			}
			else if (AUDIO_SUPPORT[SUPPORT_DDP] == 2)
			{
				mimetype = TCC_MIME_DDP;
			}
			break;
		case AUDIO_ID_TRUEHD:
			if (AUDIO_SUPPORT[SUPPORT_AC3] == 2)
			{
				mimetype = TCC_MIME_TRUEHD;
			}
			else if (AUDIO_SUPPORT[SUPPORT_AC3] == 1)
			{
				mimetype = TCC_MIME_AC3;
			}
			break;
		case AUDIO_ID_MP1:
			// MP1 audio is decoded by the MP3 decoder
			mimetype = TCC_MIME_MP3_TCC; 
			break;	
		default:
			mimetype = TCC_MIME_AUDIO_UNKNOWN;
			break;
	}

	return mimetype;
}

String8	TCC_CDK_Wrapper::ConvertSubTypeToMimeType(unsigned long ulType)
{
	String8	mimetype; 
	switch (ulType)
	{
	case 0:
		mimetype = TCC_MIME_TEXT_GRAPHIC;
		break;
	case 1:
		mimetype = TCC_MIME_TEXT_3GPP;
		break;
	default: 
		mimetype = TCC_MIME_TEXT_UNKNOWN;
		break;
	}
	return mimetype;
}


int32_t TCC_CDK_Wrapper::CopyMediaInfo( unsigned long ulParsingMode, void* pvStrmIdList )
{
	int32_t ret = CDK_ERR_NONE;
	int i;
	cdmx_info_t	*pst_cdmx_info = &(static_cast<CDK_ITEM*>(pItem)->gsCDmxInfo);
	cdmx_stream_id_list_t *pst_strm_id_list = (cdmx_stream_id_list_t *)pvStrmIdList;
	cdmx_info_t	st_cdmx_info_temp;
	cdmx_info_t	*pst_curr_info = NULL;
	cdmx_select_stream_t st_sel_stream;

	FUNC("In CopyMediaInfo \n");

	////////////////////////////////////////////////////////////////
	//
	// copy cdmx media info. into local
	//
	//
	// [*] stream id
	//
	{
		iVidIDList = (int32_t*)pst_strm_id_list->plVideoIdList;
		iAudIDList = (int32_t*)pst_strm_id_list->plAudioIdList;
		iSubIDList = (int32_t*)pst_strm_id_list->plSubtitleIdList;

		iDefaultVidStreamID = (*pst_cdmx_info).m_sVideoInfo.m_iCurrVideoIdx;
		iDefaultAudStreamID = (*pst_cdmx_info).m_sAudioInfo.m_iCurrAudioIdx;
		iDefaultSubStreamID = (*pst_cdmx_info).m_sSubtitleInfo.m_iCurrSubtitleIdx;
	}
	//
	// File info.
	//
	{
		iFileInfo.iDuration 			= (uint32_t)((*pst_cdmx_info).m_sFileInfo.m_iRunningtime);
		iFileInfo.iBitRate				= (uint32_t)((*pst_cdmx_info).m_sVideoInfo.m_iBitRate); // kbps
		iFileInfo.iTimeScale 			= (uint32_t)((*pst_cdmx_info).m_sFileInfo.m_iTimeScale);
		iFileInfo.iNumStreams			= 0;
		iFileInfo.iNumVidStreams 		= (uint32_t)((*pst_cdmx_info).m_sVideoInfo.m_iNumVideoStream);
		iFileInfo.iNumAudStreams 		= (uint32_t)((*pst_cdmx_info).m_sAudioInfo.m_iNumAudioStream);
		iFileInfo.iNumSubtitleStreams 	= (uint32_t)((*pst_cdmx_info).m_sSubtitleInfo.m_iNumSubtitleStream);
		iFileInfo.iContainerType		= i_ctype;
		iFileInfo.iSeekable				= (bool)(*pst_cdmx_info).m_sFileInfo.m_iSeekable;
		iFileInfo.bHasIndex				= ((*pst_cdmx_info).m_sVideoInfo.m_iKeyFrameNumber > 0)?true:false;
		iFileInfo.m_pcTitle 			= (*pst_cdmx_info).m_sFileInfo.m_pcTitle;
		iFileInfo.m_pcArtist 			= (*pst_cdmx_info).m_sFileInfo.m_pcArtist;
		iFileInfo.m_pcAlbum 			= (*pst_cdmx_info).m_sFileInfo.m_pcAlbum;
		iFileInfo.m_pcAlbumArtist		= (*pst_cdmx_info).m_sFileInfo.m_pcAlbumArtist;
		iFileInfo.m_pcYear 				= (*pst_cdmx_info).m_sFileInfo.m_pcYear;
		iFileInfo.m_pcWriter 			= (*pst_cdmx_info).m_sFileInfo.m_pcWriter;
		iFileInfo.m_pcAlbumArt 			= (*pst_cdmx_info).m_sFileInfo.m_pcAlbumArt;
		iFileInfo.m_iAlbumArtSize 		= (*pst_cdmx_info).m_sFileInfo.m_iAlbumArtSize;
		iFileInfo.m_pcGenre 			= (*pst_cdmx_info).m_sFileInfo.m_pcGenre;
		iFileInfo.m_pcCompilation 		= (*pst_cdmx_info).m_sFileInfo.m_pcCompilation;	
		iFileInfo.m_pcCDTrackNumber 	= (*pst_cdmx_info).m_sFileInfo.m_pcCDTrackNumber;
		iFileInfo.m_pcDiscNumber 		= (*pst_cdmx_info).m_sFileInfo.m_pcDiscNumber;
		iFileInfo.m_pcLocation 			= (*pst_cdmx_info).m_sFileInfo.m_pcLocation;
		iFileInfo.m_iFragmentedMp4 		= (*pst_cdmx_info).m_sFileInfo.m_iFragmentedMp4;

		if ( (iFileInfo.iBitRate == 0) && (iFileInfo.iDuration > 0) )
		{
			uint64_t filesize = 0;
			GetCurrentFileSize(filesize);
			#if 0
			uint64_t bitrate;
			// ((filesize/1024)*8)/(iFileInfo.iDuration/1000)
			bitrate = ((filesize/1024)*8)*1000/iFileInfo.iDuration;
			iFileInfo.iBitRate = (uint32_t)bitrate;
			#endif
			//duration:  *1000
			//bitrate: kbps
			iFileInfo.iBitRate = (uint32_t)( (filesize*8.0*1000)/(iFileInfo.iDuration*1000.0) );
		}

		if (i_ctype == CONTAINER_TYPE_AUDIO ) 
		{
			iFileInfo.iSeekable = true; 
			iFileInfo.iNumAudStreams = 1;
			iFileInfo.iNumVidStreams = 0;
		}

        // player doesn't support multi-video ts file. 
        // we need to fix it. refer to cdmx_ts()
        if (/*i_ctype != CONTAINER_TYPE_TS  &&*/
            i_ctype != CONTAINER_TYPE_MKV && 
            i_ctype != CONTAINER_TYPE_AVI &&
            i_ctype != CONTAINER_TYPE_ASF && 
            i_ctype != CONTAINER_TYPE_MP4 &&
            i_ctype != CONTAINER_TYPE_MPG)
        {
            if (iFileInfo.iNumVidStreams > 1)
                iFileInfo.iNumVidStreams = 1;
        }

        // only TS, AVI, MKV, ASF, MP4 parsers support multi-audio streams. 
        // others support only default audio stream.
        if (i_ctype != CONTAINER_TYPE_TS  &&
            i_ctype != CONTAINER_TYPE_MKV && 
            i_ctype != CONTAINER_TYPE_AVI &&
            i_ctype != CONTAINER_TYPE_ASF && 
            i_ctype != CONTAINER_TYPE_MP4 && 
            i_ctype != CONTAINER_TYPE_MPG)
        {
            if (iFileInfo.iNumAudStreams > 1)
                iFileInfo.iNumAudStreams = 1;
        }

		// only TS, MKV, MP4 parsers support subtitle streams.
		if (i_ctype != CONTAINER_TYPE_TS  &&
			i_ctype != CONTAINER_TYPE_MKV && 
			i_ctype != CONTAINER_TYPE_MP4)
		{
			if (iFileInfo.iNumSubtitleStreams > 0)
				iFileInfo.iNumSubtitleStreams = 0;
		}
	
		// at the media scan mode, multi stream is not considered.
		if ( ulParsingMode & CDMX_MARKER_MEDIA_SCAN_MODE_BIT ) 
		{
			if (iFileInfo.iNumVidStreams > 1)
				iFileInfo.iNumVidStreams = 1;

			if (iFileInfo.iNumAudStreams > 1)
				iFileInfo.iNumAudStreams = 1;
		}

		iFileInfo.iNumStreams = iFileInfo.iNumVidStreams + iFileInfo.iNumAudStreams;
		if (iFileInfo.iNumVidStreams > 0)
			iVidInfo.setCapacity(iFileInfo.iNumVidStreams);
		if (iFileInfo.iNumAudStreams > 0)
			iAudInfo.setCapacity(iFileInfo.iNumAudStreams);
		if (iFileInfo.iNumSubtitleStreams > 0)
			iSubInfo.setCapacity(iFileInfo.iNumSubtitleStreams);
	}
	//
	// Video info.
	//
	{
		if (iFileInfo.iNumVidStreams == 0)
		{
			ALOGI("This file has no video stream");
			video_end = true;
		}
		else
		{
			for (i = 0 ; i < iFileInfo.iNumVidStreams ; i++)
			{
				TCC_CDK_VideoInfo vid_info;
				if ( i == 0 ) 
				{
					// use default
					pst_curr_info = pst_cdmx_info;
				}
				else  
				{
					memset(&st_sel_stream, 		0, sizeof(st_sel_stream    ));
					memset(&st_cdmx_info_temp,  0, sizeof(st_cdmx_info_temp));

					st_sel_stream.lSelType = DMXTYPE_VIDEO;
					st_sel_stream.lVideoID = iVidIDList[i];

					DSTATUS("************************************************\n");
					DSTATUS("Video(%d), StreamId(%d)\n", i, st_sel_stream.lVideoID);

					ret = m_pfnCdmx( CDMX_GET_INFO_SEL_STREAM, CDMX_HANDLE(m_pvCdmxInst), &st_sel_stream, &st_cdmx_info_temp );
					if( ret < 0 )
					{
						ALOGE("[CDK_WRAPPER:Err%d] CDMX_GET_INFO_SEL_STREAM failed", ret );
						pst_curr_info = pst_cdmx_info;
					}
					else
					{
						pst_curr_info = &st_cdmx_info_temp;
					}
				}
				
				// retrieve info
				vid_info.iID				= -1;
				//vid_info.i_vtype
				//vid_info.mimetype
				vid_info.iWidth				= pst_curr_info->m_sVideoInfo.m_iWidth;
				vid_info.iHeight 			= pst_curr_info->m_sVideoInfo.m_iHeight;
				vid_info.iFrameRate 		= pst_curr_info->m_sVideoInfo.m_iFrameRate;
				vid_info.iBitRate 			= pst_curr_info->m_sVideoInfo.m_iBitRate;
				// do not assign addr. here.
				//vid_info.pExtraData		= pst_curr_info->m_sVideoInfo.m_pExtraData;
				vid_info.iExtraDataLen 		= pst_curr_info->m_sVideoInfo.m_iExtraDataLen;
				vid_info.iFourCC 			= pst_curr_info->m_sVideoInfo.m_iFourCC;
				vid_info.iRotateDegree 		= pst_curr_info->m_sVideoInfo.m_iRotateDegree;
				//vid_info.pCodecExtraData	= (uint8_t*)&pst_curr_info->m_sVideoInfo;
				vid_info.iCodecExtraDataLen	= sizeof(pst_curr_info->m_sVideoInfo);

				DSTATUS("[Line:%04d] pst_curr_info->m_sVideoInfo.m_pExtraData: 0x%x\n", __LINE__, pst_curr_info->m_sVideoInfo.m_pExtraData);
				DSTATUS("[Line:%04d] &pst_curr_info->m_sVideoInfo: 0x%x\n", __LINE__, &pst_curr_info->m_sVideoInfo);

				//
				// find video codec 
				//
				vid_info.i_vtype = get_video_codec_type( vid_info.iFourCC );
				if(vid_info.i_vtype != CDK_ERR_NOT_SUPPORTED_VIDEO_CODEC)
				{
					codec_exist |= EXIST_VIDEO;
					video_end = false;
					vid_info.iID = i;
				}
				else
				{
					ALOGW("[CDK_WRAPPER] video decoder not found (FourCC: 0x%x), (i_atype: %d) \n", vid_info.iFourCC, vid_info.i_vtype );
					if ( !(codec_exist&EXIST_VIDEO) )
						video_end = true;
				}
				//
				// find mime type 
				//
				vid_info.mimetype = ConvertVidTypeToMimeType(vid_info.i_vtype);
				if (vid_info.iWidth * vid_info.iHeight > MAX_VIDEO_WIDTH * MAX_VIDEO_HEIGHT)
				{
					ALOGE("This video resolution is not supported");

					vid_info.mimetype = TCC_MIME_VIDEO_NOTSUPPORTED; // cannot play video
					char value[PROPERTY_VALUE_MAX];
					property_get("tcc.video.notsupported.config", value, "1");
					// 0: Do NOT connect on list of files
					// 1: This video cannot be played.(default)
					// 2: The file has to be played if there is even one video or audio.
					int i_flag = atoi(value);
					if (i_flag == 0) {
						ALOGE("Do NOT connect on list of files");
						vid_info.mimetype = TCC_MIME_VIDEO_UNKNOWN; // don't register on gallery
					}
				}

				//
				// Copy video info 
				//
				iVidInfo.push(vid_info);

				// codec private data
				iVidInfo.editTop().pExtraData = (uint8_t*)malloc(vid_info.iExtraDataLen);
				if( iVidInfo.editTop().pExtraData == NULL ) 
				{
					ALOGE( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
					return CDK_ERR_MALLOC;
				}
				DSTATUS("[Line:%04d] iVidInfo.editTop().pExtraData: 0x%x\n", __LINE__, iVidInfo.editTop().pExtraData);
				memcpy(iVidInfo.editTop().pExtraData, pst_curr_info->m_sVideoInfo.m_pExtraData, vid_info.iExtraDataLen);

				// For component init
				iVidInfo.editTop().pCodecExtraData = (uint8_t*)malloc(vid_info.iCodecExtraDataLen);
				if( iVidInfo.editTop().pCodecExtraData == NULL ) 
				{
					ALOGE( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
					return CDK_ERR_MALLOC;
				}
				DSTATUS("[Line:%04d] iVidInfo.editTop().pCodecExtraData: 0x%x\n", __LINE__, iVidInfo.editTop().pCodecExtraData);
				memcpy(iVidInfo.editTop().pCodecExtraData, &pst_curr_info->m_sVideoInfo, vid_info.iCodecExtraDataLen);


				DSTATUS("\t\t video stream %d info\n", i);
				DSTATUS("\t\t ID %d, Width %d, Height %d, FrameRate %d, Bitrate %d bps, FourCC %x, Extra data len %d, component Extra data len %d\n", 
						  iVidInfo[i].iID, iVidInfo[i].iWidth, iVidInfo[i].iHeight, iVidInfo[i].iFrameRate, iVidInfo[i].iBitRate, iVidInfo[i].iFourCC, iVidInfo[i].iExtraDataLen, iVidInfo[i].iCodecExtraDataLen);
				DSTATUS("------------------------------------------------\n");
			}// end of for
		}

		if(video_end == true)
		{
			char value[PROPERTY_VALUE_MAX];
			property_get("tcc.video.notsupported.config", value, "1");
			// 0: Do NOT connect on list of files
			// 1: This video cannot be played.(default)
			// 2: The file has to be played if there is even one video or audio.
			int i_flag = atoi(value);
			if (i_flag == 2) {
				iFileInfo.iNumVidStreams = 0;
			}
		}
	}
	//
	// Audio info.
	//
	{
        TCC_CDK_AudioInfo aud_info;

		if (iFileInfo.iNumAudStreams == 0)
		{
			ALOGI("This file has no audio stream");
			audio_end = true;
		}
		else
		{
			for (i = 0 ; i < iFileInfo.iNumAudStreams ; i++)
			{
				if ( i == 0 ) 
				{
					// use default
					pst_curr_info = pst_cdmx_info;
				}
				else
				{
					memset(&st_sel_stream, 		0, sizeof(st_sel_stream    ));
					memset(&st_cdmx_info_temp,  0, sizeof(st_cdmx_info_temp));

					st_sel_stream.lSelType = DMXTYPE_AUDIO;
					st_sel_stream.lAudioID = iAudIDList[i];

					DSTATUS("************************************************\n");
					DSTATUS("Audio(%d), StreamId(%d)\n", i, st_sel_stream.lAudioID);

					ret = m_pfnCdmx( CDMX_GET_INFO_SEL_STREAM, CDMX_HANDLE(m_pvCdmxInst), &st_sel_stream, &st_cdmx_info_temp );
					if( ret < 0 )
					{
						ALOGE("[CDK_WRAPPER:Err%d] CDMX_GET_INFO_SEL_STREAM failed", ret );
						pst_curr_info = pst_cdmx_info;
					}
					else
					{
						pst_curr_info = &st_cdmx_info_temp;
					}
				}
				
				// retrieve info
				aud_info.iID				= -1;
				//aud_info.i_atype;
				//aud_info.mimetype;
				aud_info.iSamplesPerSec		= pst_curr_info->m_sAudioInfo.m_iSamplePerSec;
				aud_info.iBitsPerSample		= pst_curr_info->m_sAudioInfo.m_iBitsPerSample;
				aud_info.iBitRate			= pst_curr_info->m_sAudioInfo.m_iBitRate; // bps, not kbps
				aud_info.iChannels 			= pst_curr_info->m_sAudioInfo.m_iChannels;
				aud_info.iFormatId 			= pst_curr_info->m_sAudioInfo.m_iFormatId;
				aud_info.iAvgBytesPerSec	= pst_curr_info->m_sAudioInfo.m_iAvgBytesPerSec;
				//aud_info.pCodecExtraData	= (uint8_t*)&pst_curr_info->m_sAudioInfo; // for component init
				aud_info.iCodecExtraDataLen = sizeof(pst_curr_info->m_sAudioInfo); // for component init
				aud_info.pszLanguageInfo	= pst_curr_info->m_sAudioInfo.m_pszlanguageInfo;

				//
				// find audio codec 
				//
				aud_info.i_atype = get_audio_codec_type( static_cast<CDK_ITEM*>(pItem), aud_info.iFormatId, i_ctype, ulParsingMode );

				// Not support 'MC_COOK'
				if((aud_info.i_atype==AUDIO_ID_COOK) && (aud_info.iChannels>2))
				{
					aud_info.i_atype = CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC;
					ALOGI("\t\t audio ID = MC_COOK \n");                  
				}

				if(aud_info.i_atype != CDK_ERR_NOT_SUPPORTED_AUDIO_CODEC)
				{
					codec_exist |= EXIST_AUDIO;
					audio_end = false;
					aud_info.iID = i;
				}
				else
				{
					ALOGW("[CDK_WRAPPER] audio decoder not found (formatid: %x), (i_atype: %d) \n", aud_info.iFormatId, aud_info.i_atype );
					if ( !(codec_exist&EXIST_AUDIO) )
						audio_end = true;
				}
				//
				// find mime type 
				//
				aud_info.mimetype = ConvertAudTypeToMimeType(aud_info.i_atype);

				if (aud_info.iSamplesPerSec > 352800)
				{
					ALOGE("sampling rate of audio stream ID %d is %d", i, aud_info.iSamplesPerSec);
					ALOGE("TCC Android doesn't support over 352.8Khz sampling rate audio");
					aud_info.mimetype = TCC_MIME_AUDIO_UNKNOWN;
				}

				// currently vorbis codec in AVI file is not supported. Because it has no data for codec initialization.
				if (aud_info.i_atype == AUDIO_ID_VORBIS && i_ctype == CONTAINER_TYPE_AVI)
				{
					ALOGE("Vorbis codec in AVI file is not supported");
					aud_info.mimetype = TCC_MIME_AUDIO_UNKNOWN;
				}

				// AudioOutput update Planet 20121120
				if (AUDIO_SUPPORT[SUPPORT_DTS] == 2 && (/*aud_info.i_atype == AUDIO_ID_DTS || */aud_info.i_atype == AUDIO_ID_DTSHD || aud_info.i_atype == AUDIO_ID_TRUEHD ))	// SPDIF_AC3_TRUEHD
				{
					// temporarily force sampling rate to 192kHz for HBR output
					aud_info.iSamplesPerSec = 192000;
				}

				//
				// Copy audio info 
				//
				iAudInfo.push(aud_info);

				if (aud_info.i_atype == AUDIO_ID_AAC_GOOGLE) // for google aac decoder
				{
					if(pst_curr_info->m_sAudioInfo.m_pExtraData || pst_curr_info->m_sAudioInfo.m_iExtraDataLen >= 2)
					{
						//ALOGI("copy AudioSpecificConfigData for google aac decoder");
						iAudInfo.editTop().iCodecExtraDataLen = pst_curr_info->m_sAudioInfo.m_iExtraDataLen;
						iAudInfo.editTop().pCodecExtraData = (uint8_t*)malloc(iAudInfo[i].iCodecExtraDataLen);
						memcpy(iAudInfo.editTop().pCodecExtraData, pst_curr_info->m_sAudioInfo.m_pExtraData, iAudInfo[i].iCodecExtraDataLen);
					}
					else			
					{
						//ALOGI("make AudioSpecificConfigData for google aac decoder");
						iAudInfo.editTop().iCodecExtraDataLen = 2;
						iAudInfo.editTop().pCodecExtraData = (uint8_t*)malloc(2);
						MakeAACSpecificConfigData(iAudInfo.editTop().pCodecExtraData, 
												  1 /* AAC LC */, 
												  pst_curr_info->m_sAudioInfo.m_iSamplePerSec, 
												  pst_curr_info->m_sAudioInfo.m_iChannels);
					}
				}
				else
				{
					// codec private data
					iAudInfo.editTop().pCodecExtraData = (uint8_t*)malloc(aud_info.iCodecExtraDataLen);
					if( iAudInfo.editTop().pCodecExtraData == NULL ) 
					{
						ALOGE( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
						return CDK_ERR_MALLOC;
					}
					DSTATUS("[Line:%04d] iAudInfo.editTop().pCodecExtraData: 0x%x\n", __LINE__, iAudInfo.editTop().pCodecExtraData);
					memcpy(iAudInfo.editTop().pCodecExtraData, &pst_curr_info->m_sAudioInfo, aud_info.iCodecExtraDataLen);
				}

				DSTATUS("\t\t audio stream %d info\n", i);
				DSTATUS("\t\t ID %d, Bits/samples %d, Bitrate %d bps, Channels %d, Samples/Sec %d Hz, Format %x, Extra data len %d, Component Extra data len %d\n", 
					  iAudInfo[i].iID, iAudInfo[i].iBitsPerSample, iAudInfo[i].iBitRate, iAudInfo[i].iChannels, 
					  iAudInfo[i].iSamplesPerSec, iAudInfo[i].iFormatId, pst_curr_info->m_sAudioInfo.m_iExtraDataLen, iAudInfo[i].iCodecExtraDataLen);
				DSTATUS("------------------------------------------------\n");
			}// end of for
		}
	}
	//
	// Subtitle info.
	//
	{
		for (i = 0 ; i < iFileInfo.iNumSubtitleStreams ; i++)
		{
			TCC_CDK_SubtitleInfo sub_info;
			if ( i == 0 ) 
			{
				// use default
				pst_curr_info = pst_cdmx_info;
			}
			else
			{
				memset(&st_sel_stream, 		0, sizeof(st_sel_stream    ));
				memset(&st_cdmx_info_temp,  0, sizeof(st_cdmx_info_temp));

				st_sel_stream.lSelType = DMXTYPE_SUBTITLE;
				st_sel_stream.lSubtitleID = iSubIDList[i];

				DSTATUS("************************************************\n");
				DSTATUS("Subtitle(%d), StreamId(%d)\n", i, st_sel_stream.lSubtitleID);

				ret = m_pfnCdmx( CDMX_GET_INFO_SEL_STREAM, CDMX_HANDLE(m_pvCdmxInst), &st_sel_stream, &st_cdmx_info_temp );
				if( ret < 0 )
				{
					ALOGE("[CDK_WRAPPER:Err%d] CDMX_GET_INFO_SEL_STREAM failed", ret );
					pst_curr_info = pst_cdmx_info;
				}
				else
				{
					pst_curr_info = &st_cdmx_info_temp;
				}
			}

			// retrieve info
			sub_info.iID				= i;
			sub_info.i_stype			= pst_curr_info->m_sSubtitleInfo.m_iIsText;
			sub_info.mimetype 			= ConvertSubTypeToMimeType(sub_info.i_stype);
			sub_info.iMaxInputSize		= SUBTITLE_ONE_ELEMENT_MAX;
			sub_info.pszLanguageInfo	= pst_curr_info->m_sSubtitleInfo.m_pszLanguage;

			//
			// Copy subtitle info 
			//
			iSubInfo.push(sub_info);

			DSTATUS("\t\t ID %d, Format %d\n", iSubInfo[i].iID, iSubInfo[i].i_stype);
			DSTATUS("------------------------------------------------\n");
		}
	}

	PrintInfo("==========================================================================");

#ifdef MERGE_FRAME_AVG
    // for defragmentation
    if(!bIsWFDPlayMode)
    {
        int flag = 0;
        char value[PROPERTY_VALUE_MAX];
        memset(value, 0, PROPERTY_VALUE_MAX);
    
        if( i_ctype == CONTAINER_TYPE_TS )
        {
            property_get("tcc.defragmentation.ts", value, "1");
            flag = atoi(value);
            if( flag == 0 )
            {
                ALOGE("Disable Defragmentation of TS file");
                m_lVideoCodecType = -1;
                return 0;
            }
        }
        else if( i_ctype == CONTAINER_TYPE_MPG )
        {
            property_get("tcc.defragmentation.mpg", value, "1");
            flag = atoi(value);
            if( flag == 0 )
            {
                ALOGE("Disable Defragmentation of MPG file");
                m_lVideoCodecType = -1;
                return 0;
            }
        }

        if ( i_ctype == CONTAINER_TYPE_TS || i_ctype == CONTAINER_TYPE_MPG)
        {
            int type = get_video_codec_type( (*pst_cdmx_info).m_sVideoInfo.m_iFourCC );
            switch (type)
            {
            case STD_MPEG2:
                property_get("tcc.defragmentation.mpeg2", value, "1");
                flag = atoi(value);
                if(flag)
                {
                    //ALOGD("[DEBUG] Set Defragmentation(MPEG-2) flag");
                    m_lVideoCodecType = type;
                    m_ulSeqHeaderType = MPEG2_SEQUENCE_HEADER;
                    m_ulPicHeaderType = MPEG2_PICTURE_START;
                    m_ulPicHeaderType2 = MPEG2_PICTURE_START;
                }
                else
                    m_lVideoCodecType = -1;
                break;
            case STD_MPEG4:
                //ALOGD("[DEBUG] Set Defragmentation(MPEG4) flag");
                property_get("tcc.defragmentation.mpeg4", value, "1");
                flag = atoi(value);
                if(flag)
                {
                    //ALOGD("[DEBUG] Set Defragmentation(MPEG4) flag");
                    m_lVideoCodecType = type;
                    m_ulSeqHeaderType = MPEG4_VIDEO_HEADER;
                    m_ulPicHeaderType = MPEG4_VOP_START;
                    m_ulPicHeaderType2 = MPEG4_VOP_START;					
                }
                else
                    m_lVideoCodecType = -1;
                break;
            case STD_AVC:                
                property_get("tcc.defragmentation.avc", value, "1");
                flag = atoi(value);
                if(flag)
                {
	             //ALOGD("[DEBUG] Set Defragmentation(AVC) flag");
                    m_lVideoCodecType = type;
                }
                else
                    m_lVideoCodecType = -1;
                break;
            case STD_AVS:                
                property_get("tcc.defragmentation.avs", value, "1");
                flag = atoi(value);
                if(flag)
                {
                    //ALOGD("[DEBUG] Set Defragmentation(AVS) flag");
                    m_lVideoCodecType = type;
		      m_ulSeqHeaderType = AVS_VIDEO_HEADER;
                    m_ulPicHeaderType = AVS_VIDEO_I_FRAME;
		      m_ulPicHeaderType2 = AVS_VIDEO_PB_FRAME;
                }
                else
                    m_lVideoCodecType = -1;
                break;
            case STD_VC1:
                property_get("tcc.defragmentation.vc1", value, "1");
                flag = atoi(value);
                if(flag)
                {
                    //ALOGD("[DEBUG] Set Defragmentation(VC1) flag");
                    m_lVideoCodecType = type;
                    m_ulSeqHeaderType = WVC1_VIDEO_HEADER;
                    m_ulPicHeaderType = WVC1_VIDEO_FRAME;
                    m_ulPicHeaderType2 = WVC1_VIDEO_FRAME;					
                }
                else
                    m_lVideoCodecType = -1;
                break;
            default:
                m_lVideoCodecType = -1;
                break;
            }
        }
    }
#endif // MERGE_FRAME_AVG

	return ret;
}

int32_t TCC_CDK_Wrapper::InitCdmx (unsigned long ulParsingMode)
{
	int32_t ret = CDK_ERR_NONE;
	CDK_ITEM* CDK = static_cast<CDK_ITEM*>(pItem);
	cdk_core_t* pCdk = &(iCdk.m_sCore);

	FUNC("In InitCdmx \n");

	// ------------------------------
	//  CDMX_INIT
	// ------------------------------
	clock_t start, end;
	if (mProfilingMode) {
		start = clock();
	}

	{
		cdmx_init_t cdmx_init;
		cdmx_init.pszFileName 	= pCdk->m_pcOpenFileName;
		cdmx_init.pstCallback 	= pCdk->m_psCallback;
		cdmx_init.ulParsingMode = ulParsingMode;
		cdmx_init.ulOptions		= 0;

		if ( AUDIO_SUPPORT[SUPPORT_AC3] == 2 ) {
			cdmx_init.ulOptions	|= CDMXINIT_SPDIF_MULTIMODE;
		}
		DSTATUS("file: %s, mode: %d", cdmx_init.pszFileName, cdmx_init.ulParsingMode);

		ret = m_pfnCdmx( CDMX_INIT, CDMX_HANDLE(m_pvCdmxInst), &cdmx_init, NULL );
		if ( ret < 0 ) {
			ALOGE("[CDK_WRAPPER] [Err:%4d] demuxer init", ret );
			ret = m_pfnCdmx( CDMX_CLOSE, CDMX_HANDLE(m_pvCdmxInst), NULL, NULL );
			return TCC_CDK_WRAPPER_DEMUXER_ERROR;
		}
	}
	if (mProfilingMode) {
		end = clock();
		ALOGD("file demuxer init time = %d ms", (end - start) * 1000 / CLOCKS_PER_SEC);
	}

	guiLastpts = 0;
	ipParserLibExist[i_ctype] = 1;

	// ------------------------------
	//  CDMX_GET_INFO: retrieve default stream info
	// ------------------------------
	{
		cdmx_stream_id_list_t st_strm_id_list;
		ret = m_pfnCdmx( CDMX_GET_INFO, CDMX_HANDLE(m_pvCdmxInst), &st_strm_id_list, &CDK->gsCDmxInfo );
		if ( ret < 0 ) {
			ALOGE("[CDK_WRAPPER] [Err:%4d] demuxer init", ret );
			ret = m_pfnCdmx( CDMX_CLOSE, CDMX_HANDLE(m_pvCdmxInst), NULL, NULL );
			return TCC_CDK_WRAPPER_DEMUXER_ERROR;
		}

		/////////////////////////////////////////////////////////////////
		//
		// Copy the cdmx media info. into local
		//
		CopyMediaInfo(ulParsingMode, &st_strm_id_list);
	}

	if( bIsDecryptionNeeded == false && CDK->gsCDmxInfo.m_sFileInfo.m_ulSecureType != 0 )
		bIsDecryptionNeeded = true;	

	return ret;
}

int32_t TCC_CDK_Wrapper::CloseCdmx (void)
{
	int32_t ret = CDK_ERR_NONE;
	CDK_ITEM* CDK = static_cast<CDK_ITEM*>(pItem);
	cdk_core_t* pCdk = &(iCdk.m_sCore);

	FUNC("In CloseCdmx\n");

	ret = m_pfnCdmx( CDMX_CLOSE, CDMX_HANDLE(m_pvCdmxInst), NULL, NULL );

	if (video_end == true) video_end = false;
	if (audio_end == true) audio_end = false;

	iVidIDList = NULL;
	iAudIDList = NULL;
	iSubIDList = NULL;

	if (iFileInfo.iNumVidStreams > 0)
	{
		iVidInfo.removeItemsAt(0, iFileInfo.iNumVidStreams);
	}

	if (iFileInfo.iNumAudStreams > 0)
	{
		iAudInfo.removeItemsAt(0, iFileInfo.iNumAudStreams);
	}

	if (iFileInfo.iNumSubtitleStreams > 0)
	{
		iSubInfo.removeItemsAt(0, iFileInfo.iNumSubtitleStreams);
	}

	// Reinitialize container-dependent variables of each demuxer which supports sequential mode in cdmx.c. 
	ret = ResetCdmxInstance();

	return ret;
}

int32_t TCC_CDK_Wrapper::SelectCdkDemuxMode()
{
    int32_t mode = 0;
	char value[PROPERTY_VALUE_MAX];
    memset(value, 0, PROPERTY_VALUE_MAX);
    property_get("tcc.parser.sequential", value, "");

    // cdk demux mode property values :
    // 0 - selective mode
    // 1 - sequential mode without a demuxer thread
    // 2 - sequential mode with a demuxer thread
    // 3 - sequential mode with another demuxer thread
    if (strlen(value) > 0)
    {
        mode = atoi(value);
    }
    else
    {
        switch (mSourceScheme)
        {
            case HTTP_SOURCE_SCHEME:
                mode = 3;
                break;
            case RTSP_SOURCE_SCHEME:
            case HDCP2_SOURCE_SCHEME:
                mode = 2;
                break;
            default:
                mode = 2;
                break;
        }
    }

    return mode;
}

int32_t TCC_CDK_Wrapper::ContainerParse(uint32_t aMode, int32_t aSourceScheme, const char *pszExt)
{
	int32_t ret = CDK_ERR_NONE;
	char value[PROPERTY_VALUE_MAX];
	CDK_ITEM* CDK = static_cast<CDK_ITEM*>(pItem);
	cdk_core_t* pCdk = &(iCdk.m_sCore);
	int i_parsing_mode = 0;
	
	FUNC("In ContainerParse - aMode:0x%08x \n", aMode);

	mSourceScheme = aSourceScheme;

    //--------------------------------------------------
    // Set Callback Functions
    //--------------------------------------------------
	{
		uint32_t cb_type_option = NORMAL_FUNCTIONS;
		if (mSourceScheme != FILE_SOURCE_SCHEME)
		{
			// In case of broken file, we need to check invalid offset of I/O in seek(). 
			// If not, it causes an error. 
			if ((aMode & TCC_CDK_WRAPPER_PARSER_IGNORE_FILE_SIZE) ||
				(aMode & TCC_CDK_WRAPPER_PARSER_PROGRESSIVE_READ_MODE))
			{
				cb_type_option |= STREAMING_SEEK_FUNCTION;
				cb_type_option |= STREAMING_EOF_FUNCTION;
			}
		}

		ret = SetCallbackFunctions(cb_type_option);
		if (ret != TCC_CDK_WRAPPER_OK)
		{
			PrintError("[CDK_WRAPPER] [Err:%4d] fail setting callback functions", ret);
			return TCC_CDK_WRAPPER_ERROR;
		}
	}
	//--------------------------------------------------
	// Load external setting values 
	//--------------------------------------------------
	LoadExternalSetting();

	//--------------------------------------------------
	// Identify Container type
	//--------------------------------------------------
	i_ctype  = get_input_container_type(pCdk->m_pcOpenFileName, pszExt);
	if( i_ctype == CONTAINER_TYPE_NONE )
	{
		ALOGE("[CDK_WRAPPER] [Err:%4d] demuxer not found! (i_ctype: %d)\n", ret, i_ctype );
		return TCC_CDK_WRAPPER_DEMUXER_ERROR;
	}

	//////////////////////////////////////////////////////////////////////////////////////
	//
	//  Create Instance 
	//
	ret = CreateCdmxInstance( (unsigned long)i_ctype );
	if ( ret < 0 ) {
		ALOGE("[CDK_WRAPPER] [Err:%4d] CreateCdmxInstance failed... \n", ret );
		return ret;
	}

	///////////////////////////////////////////////////////////////////////////////////////
	//
	//  Determine Parsing & Play mode of demuxer
	//
	i_parsing_mode 	  = CDMX_MARKER_DEFAULT_PLAY_MODE_BIT;
	iGetMetadataMode  = (aMode & TCC_CDK_WRAPPER_PARSER_GETMETADATAMODE ) ? true : false;
	iGetThumbnailMode = (aMode & TCC_CDK_WRAPPER_PARSER_GETTHUMBNAILMODE) ? true : false;

	STEP("\t\t\t [001][ParsingMode: %d][iGetMetadataMode: %d][iGetThumbnailMode: %d]\n", i_parsing_mode, iGetMetadataMode, iGetThumbnailMode);

    // to use media scan op code for speed up
	if (iGetMetadataMode == true && iGetThumbnailMode == false) 
	{
		i_parsing_mode |= CDMX_MARKER_MEDIA_SCAN_MODE_BIT;
		mSourceScheme = FILE_SOURCE_SCHEME;
		STEP("\t\t\t [002][ParsingMode: %d]\n", i_parsing_mode);
	}
	else if (mSourceScheme != FILE_SOURCE_SCHEME)
	{
		i_parsing_mode |= CDMX_MARKER_PROGRESSIVE_PLAY_MODE_BIT;
		if (aMode & TCC_CDK_WRAPPER_PARSER_PROGRESSIVE_READ_MODE)
		{
			i_parsing_mode |= CDMX_MARKER_PLAYBACK_ONLY_MODE_BIT;
		}
		else
		{
		    if (aMode & TCC_CDK_WRAPPER_PARSER_IGNORE_FILE_SIZE)
			{
			    i_parsing_mode |= CDMX_MARKER_IGNORE_FILE_SIZE_BIT;
			}

			if (i_ctype == CONTAINER_TYPE_TS)
			{
				// Use user defined init. setting option
				memset(value, 0, PROPERTY_VALUE_MAX);
				property_get("tcc.tsdmx.init-params", value, "1");
				if(atoi(value) != 0)	{
					memset(value, 0, PROPERTY_VALUE_MAX);
					property_get("tcc.tsdmx.param1", value, "256");  // default : 128 * 1024
					int iParam1 = atoi(value);
					memset(value, 0, PROPERTY_VALUE_MAX);
					property_get("tcc.tsdmx.param2", value, "2560"); // default : 5 * 1024 * 1024
					int iParam2 = atoi(value);
					memset(value, 0, PROPERTY_VALUE_MAX);
					property_get("tcc.tsdmx.param3", value, "256");  // default : 64 * 1024
					int iParam3 = atoi(value);
					ALOGD("param1 = %d, param2 = %d, param3 = %d", iParam1, iParam2, iParam3);

					{
						cdmx_pre_init_t pre_init;
						memset(&pre_init, 0, sizeof(pre_init));
						pre_init.m_lParam[0] = iParam1;
						pre_init.m_lParam[1] = iParam2;
						pre_init.m_lParam[2] = iParam3;
						ret = m_pfnCdmx( CDMX_PRE_INIT, CDMX_HANDLE(m_pvCdmxInst), &pre_init, NULL );
						if ( ret < 0 ) {
							ALOGE("[CDK_WRAPPER] [Err:%4d] demuxer CDMX_PRE_INIT", ret );
							return TCC_CDK_WRAPPER_DEMUXER_ERROR;
						}
					}
				}
			}
            else if (i_ctype == CONTAINER_TYPE_FLV) {
				memset(value, 0, PROPERTY_VALUE_MAX);
				property_get("tcc.http.flv.opt", value, "0");
				if(atoi(value) != 0) {
                    ALOGD("tcc.http.flv.opt enabled");
					i_parsing_mode |= CDMX_MARKER_PLAYBACK_ONLY_MODE_BIT;
                }
            }
		}

		STEP("\t\t\t [003][ParsingMode: %d]\n", i_parsing_mode);
	}

    int32_t enableSequentialMode = 0;
    memset(value, 0, PROPERTY_VALUE_MAX);
    property_get("tcc.dmx.sequential", value, "0");
    enableSequentialMode = atoi(value);
    
    int32_t enableProgressiveMode = 0;
    memset(value, 0, PROPERTY_VALUE_MAX);
    property_get("tcc.dmx.progressive", value, "0");
    enableProgressiveMode = atoi(value);
    if (enableProgressiveMode) {
		i_parsing_mode |= CDMX_MARKER_PROGRESSIVE_PLAY_MODE_BIT;
    }

	STEP("\t\t\t [004][ParsingMode: %d]\n", i_parsing_mode);

	// enable parsing thread for all of container types 
	int thread_enable = 0;
	if( iGetMetadataMode == false && iGetThumbnailMode == false ) 
	{
		property_get("tcc.parser.thread.enable", value, "1");
		thread_enable = atoi(value);

		// container exceptions
		if( CONTAINER_TYPE_EXT1 == i_ctype 
			|| (CONTAINER_TYPE_MP4 == i_ctype && ((aMode & TCC_CDK_WRAPPER_PARSER_FRAGMENTED_MP4)!=TCC_CDK_WRAPPER_PARSER_FRAGMENTED_MP4)) )
			thread_enable = 0;
	}

	if ( (!pszExt || (pszExt && cdk_strncmp(pszExt, "audio", 5))) &&
		 ( (CONTAINER_TYPE_MKV 	 == i_ctype) || 
		   (CONTAINER_TYPE_TS  	 == i_ctype) || 
		   (CONTAINER_TYPE_EXT1 == i_ctype) || 
		   (CONTAINER_TYPE_ASF   == i_ctype) || 
		   (CONTAINER_TYPE_AVI   == i_ctype && (mSourceScheme || enableSequentialMode > 0)) || 
		   (CONTAINER_TYPE_MP4   == i_ctype && (mSourceScheme || enableSequentialMode > 0)) || 
		   (CONTAINER_TYPE_MP4   == i_ctype && (aMode & TCC_CDK_WRAPPER_PARSER_FRAGMENTED_MP4)) || 
		   (CONTAINER_TYPE_FLV   == i_ctype && mSourceScheme) || 
		   (CONTAINER_TYPE_MPG   == i_ctype && mSourceScheme) ) )
	{
        int cdk_demux_mode_prop_val = SelectCdkDemuxMode();
        if ((cdk_demux_mode_prop_val == 3) &&
             (CONTAINER_TYPE_MKV == i_ctype || CONTAINER_TYPE_MPG == i_ctype))
        {
            // [Workaround] Some demuxers seem to work well with the below demuxer thread only.
            cdk_demux_mode_prop_val = 2;
        }

        if (cdk_demux_mode_prop_val >= 1 && iGetMetadataMode == false && iGetThumbnailMode == false)
        {
            ALOGE("ContainerParse: cdk_demux_mode_prop_val %d", cdk_demux_mode_prop_val);
            /* Determine cdk wrapper mode. */
            iGetStreamMode = CDK_GETSTREAM_SEQUENTIAL_MODE;
            if (cdk_demux_mode_prop_val == 2)
                iGetStreamMode |= DEMUXER_THREAD_ENABLED_MODE;
            else if (cdk_demux_mode_prop_val == 3)
                iGetStreamMode |= DEMUXER_LOOPER_ENABLED_MODE;

            /* Determine cdk mode. */
            i_parsing_mode |= CDMX_MARKER_SEQUENTIAL_PLAY_MODE_BIT;
            if(aMode & TCC_CDK_WRAPPER_PARSER_WFDPLAYBACKMODE) 
            {
                i_parsing_mode |= CDMX_MARKER_WFD_PLAYBACK_MODE_BIT;
                bIsWFDPlayMode= true;
                char value[PROPERTY_VALUE_MAX];
                property_get("tcc.hdcp2.enable", value, "0");
                mEnableHdcp2 = atoi(value);
            }
            ALOGI("ContainerParse() : Init. cdk demux mode - SEQUENTIAL (dmx mode:%d)", cdk_demux_mode_prop_val);
        }

		STEP("\t\t\t [005][ParsingMode: %d]\n", i_parsing_mode);
	}
	else 
    {
        iGetStreamMode = CDK_GETSTREAM_SELECTIVE_MODE;
        if (thread_enable) {
            iGetStreamMode |= DEMUXER_THREAD_ENABLED_MODE;
        }
    }

	if (iGetMetadataMode == true ) 
		i_parsing_mode |= CDMX_MARKER_METADATA_MODE_BIT;

	///////////////////////////////////////////////////////////////////////////////////////
	//
	//  Initialize Demuxer
	//
    {
        ret = InitCdmx(i_parsing_mode);
        if ( ret < 0 ) {
            ALOGE("[CDK_WRAPPER] InitCdmx failed");
            return ret;
        }

        //
        //  Change mode (close & init)
        //
        if( (iGetMetadataMode == false) && (thread_enable == 0) )
        {
            if (iGetStreamMode & CDK_GETSTREAM_SEQUENTIAL_MODE)
            {
                if( !(iFileInfo.iNumVidStreams > 0 && video_end == false) ||
                        !(iFileInfo.iNumAudStreams > 0 && audio_end == false) )
                {
                    ret = CloseCdmx();
                    if ( ret < 0 ) {
                        ALOGE("[CDK_WRAPPER] CloseCdmx failed");
                        return ret;
                    }

                    // we don't need to set the demuxer mode as sequential in case of audio (or video) only playback.
                    iGetStreamMode = CDK_GETSTREAM_SELECTIVE_MODE;

                    // Now, recreate the CDMX with selective mode.
                    i_parsing_mode &= ~CDMX_MARKER_SEQUENTIAL_PLAY_MODE_BIT;
                    i_parsing_mode |= CDMX_MARKER_DEFAULT_PLAY_MODE_BIT;
                    ALOGI("[ContainerParse] Recreate the CDMX with selective mode, i_parsing_mode = %d\n", i_parsing_mode);

                    ret = InitCdmx(i_parsing_mode);
                    if ( ret < 0 ) {
                        ALOGE("[CDK_WRAPPER] InitCdmx failed\n");
                        return ret;
                    }
                }
            }
            else if ( (iGetStreamMode & CDK_GETSTREAM_SELECTIVE_MODE) && iFileInfo.m_iFragmentedMp4 )
            {
                ret = CloseCdmx();
                if ( ret < 0 ) {
                    ALOGE("[CDK_WRAPPER] CloseCdmx failed");
                    return ret;
                }

                int cdk_demux_mode_prop_val = SelectCdkDemuxMode();

                if (cdk_demux_mode_prop_val >= 1 && iGetMetadataMode == false && iGetThumbnailMode == false)
                {
                    /* Determine cdk wrapper mode. */
                    iGetStreamMode = CDK_GETSTREAM_SEQUENTIAL_MODE;
                    if (cdk_demux_mode_prop_val == 2)
                        iGetStreamMode |= DEMUXER_THREAD_ENABLED_MODE;
                    else if (cdk_demux_mode_prop_val == 3)
                        iGetStreamMode |= DEMUXER_LOOPER_ENABLED_MODE;

                    /* Determine cdk mode. */
                    i_parsing_mode |= CDMX_MARKER_SEQUENTIAL_PLAY_MODE_BIT;
                    if(aMode & TCC_CDK_WRAPPER_PARSER_WFDPLAYBACKMODE)
                    {
                        i_parsing_mode |= CDMX_MARKER_WFD_PLAYBACK_MODE_BIT;
                        bIsWFDPlayMode= true;
                        char value[PROPERTY_VALUE_MAX];
                        property_get("tcc.hdcp2.enable", value, "0");
                        mEnableHdcp2 = atoi(value);
                    }
                    ALOGI("[ContainerParse] Recreate the CDMX with SEQUENTIAL "
                            "mode(%d) for fragmented mp4, i_parsing_mode = %d", cdk_demux_mode_prop_val, i_parsing_mode);

                    ret = InitCdmx(i_parsing_mode);
                    if ( ret < 0 ) {
                        ALOGE("[CDK_WRAPPER] InitCdmx failed\n");
                        return ret;
                    }
                }
            }
        }
    }

	///////////////////////////////////////////////////////////////////////////////////////
	//
	//  Get Metadata
	//
	if ( !( (mSourceScheme != FILE_SOURCE_SCHEME) && (aMode & TCC_CDK_WRAPPER_PARSER_PROGRESSIVE_READ_MODE) ) ) 
	{
		// if container type is audio, metadata will be extracted
		// only for WMA, APE, FLAC
		ReadMetadata();
	}

    if( iGetStreamMode & CDK_GETSTREAM_SEQUENTIAL_MODE ||
        iGetStreamMode & DEMUXER_THREAD_ENABLED_MODE ||
        iGetStreamMode & DEMUXER_LOOPER_ENABLED_MODE)
	{
	#ifdef USE_HDCP_DECRYPTION
		if( GetDecryptionNeed() == false )
	#endif
		{
			if( pStreamBuffer )
			{
				LOGMSG("[CDK_WRAPPER] free pStreamBuffer === iGetStreamMode:%d\n", iGetStreamMode);
				free(pStreamBuffer);
				pStreamBuffer = NULL;
			}
		}
	}

	return TCC_CDK_WRAPPER_OK;
}

void TCC_CDK_Wrapper::SetVideoSkipModeOptions(uint32_t aMode, float aRate)
{
	cdk_core_t* pCdk = &(iCdk.m_sCore);
	CDK_ITEM* CDK = static_cast<CDK_ITEM*>(pItem);

	pCdk->m_fPlaybackSpeed = aRate;

	switch (aMode)
	{
	case TCC_CDK_WRAPPER_VIDEO_NORMAL_PLAY:
		pCdk->m_iPlayMode = 0;
		break;
	case TCC_CDK_WRAPPER_VIDEO_FRAME_NO_SKIP:
		pCdk->m_iPlayMode = PLAYMODE_SKIMMING_WITH_DISCARD_AUDIO;	
		bIsQueueEmpty = false;
		break;
	case TCC_CDK_WRAPPER_VIDEO_FRAME_SKIMMING:
		if (CDK->gsCDmxInfo.m_sFileInfo.m_iSeekable) {
			pCdk->m_iPlayMode = PLAYMODE_SKIMMING_WITH_SEEK;
			bIsQueueEmpty = false;
		} 
		else
			ALOGE("This file do not support skimming mode!");
		break;
	case TCC_CDK_WRAPPER_VIDEO_FRAME_SKIP:
		pCdk->m_iPlayMode = PLAYMODE_SKIMMING_WITH_DISCARD_AUDIO;
		bIsQueueEmpty = false;
		break;
	default:
		break;
	}
	ALOGI("[SKIM] change playmode: %08X (speed: %.3f)", pCdk->m_iPlayMode, pCdk->m_fPlaybackSpeed);
}

Vector <TCC_CDK_VideoInfo> * 
TCC_CDK_Wrapper::GetVideoInfo()
{
	return &iVidInfo;
}

Vector <TCC_CDK_AudioInfo> *
TCC_CDK_Wrapper::GetAudioInfo()
{
	return &iAudInfo;
}

Vector <TCC_CDK_SubtitleInfo> *
TCC_CDK_Wrapper::GetSubtitleInfo()
{
	return &iSubInfo;
}

TCC_CDK_FileInfo* TCC_CDK_Wrapper::GetFileInfo()
{
	return &iFileInfo;
}

void TCC_CDK_Wrapper::ConfirmMediaInfo()
{
    char value[PROPERTY_VALUE_MAX];
    
	if (iFileInfo.iNumAudStreams > 0)
	{
		uint32_t i;
		for (i = 0 ; i < iFileInfo.iNumAudStreams ; i++)
		{
			if (iAudInfo[i].iSamplesPerSec > 352800)
			{
				ALOGE("sampling rate of audio stream ID %d is %d", i, iAudInfo[i].iSamplesPerSec);
				ALOGE("TCC Android doesn't support over 352.8Khz sampling rate audio");
				iAudInfo.editItemAt(i).mimetype = TCC_MIME_AUDIO_UNKNOWN;
			}

			// currently vorbis codec in AVI file is not supported. Because it has no data for codec initialization.
			if (iAudInfo[i].i_atype == AUDIO_ID_VORBIS && i_ctype == CONTAINER_TYPE_AVI)
			{
				ALOGE("Vorbis codec in AVI file is not supported");
				iAudInfo.editItemAt(i).mimetype = TCC_MIME_AUDIO_UNKNOWN;
			}

			// AudioOutput update Planet 20121120
			if (AUDIO_SUPPORT[SUPPORT_DTS] == 2 && (/*iAudInfo[i].i_atype == AUDIO_ID_DTS || */iAudInfo[i].i_atype == AUDIO_ID_DTSHD || iAudInfo[i].i_atype == AUDIO_ID_TRUEHD ))	// SPDIF_AC3_TRUEHD
			{
				// temporarily force sampling rate to 192kHz for HBR output
				iAudInfo.editItemAt(i).iSamplesPerSec = 192000;
			}
		}
	}
   
    if (iFileInfo.iNumVidStreams > 0)
	{
		uint32_t i;
		for (i = 0 ; i < iFileInfo.iNumVidStreams ; i++)
		{
			if (iVidInfo[i].iWidth * iVidInfo[i].iHeight > MAX_VIDEO_WIDTH * MAX_VIDEO_HEIGHT)
			{
                ALOGE("This video resolution is not supported");

                iVidInfo.editItemAt(i).mimetype = TCC_MIME_VIDEO_NOTSUPPORTED; // cannot play video
                char value[PROPERTY_VALUE_MAX];
                property_get("tcc.video.notsupported.config", value, "1");
                // 0: Do NOT connect on list of files
                // 1: This video cannot be played.(default)
                // 2: The file has to be played if there is even one video or audio.
                int i_flag = atoi(value);
                if (i_flag == 0) {
                    ALOGE("Do NOT connect on list of files");
                    iVidInfo.editItemAt(i).mimetype = TCC_MIME_VIDEO_UNKNOWN; // don't register on gallery
                }
                
                #if 0 // The file has to be played if there is even one video or audio.
            	if (iFileInfo.iNumAudStreams > 0)
            	{
            		uint32_t j;
            		for (j = 0 ; j < iFileInfo.iNumAudStreams ; j++)
            		{
						iAudInfo.editItemAt(j).mimetype = TCC_MIME_AUDIO_UNKNOWN;
            		}
                } 
                #endif
			}
            
		}
	}
}


int32_t TCC_CDK_Wrapper::SelectVideoStream(int32_t aID)
{
	int32_t ret;
	CDK_ITEM* CDK = static_cast<CDK_ITEM*>(pItem);

	if (iVidID != aID && aID < (int32_t)iFileInfo.iNumVidStreams)
	{
		cdmx_select_stream_t st_sel_strm;
		memset(&st_sel_strm, 0, sizeof(st_sel_strm));

		st_sel_strm.lSelType = DMXTYPE_VIDEO;
		st_sel_strm.lAudioID = iVidIDList[aID];

		ret = m_pfnCdmx( CDMX_SEL_STREAM, CDMX_HANDLE(m_pvCdmxInst), &st_sel_strm, &CDK->gsCDmxInfo );
		if ( ret < 0 ) {
			ALOGE("[Err:%4d] CDMX_SEL_STREAM failed \n", ret );
			ret = m_pfnCdmx( CDMX_CLOSE, CDMX_HANDLE(m_pvCdmxInst), NULL, NULL );
			return TCC_CDK_WRAPPER_SELECT_STREAM_ERROR;
		}
		iVidID = aID; 
	}

	return TCC_CDK_WRAPPER_OK;
}

int32_t TCC_CDK_Wrapper::SelectAudioStream(int32_t aID)
{
	int32_t ret;
	CDK_ITEM* CDK = static_cast<CDK_ITEM*>(pItem);

	if (iAudID != aID && aID < (int32_t)iFileInfo.iNumAudStreams)
	{
		cdmx_select_stream_t st_sel_strm;
		memset(&st_sel_strm, 0, sizeof(st_sel_strm));

		st_sel_strm.lSelType = DMXTYPE_AUDIO;
		st_sel_strm.lAudioID = iAudIDList[aID];

		if( iAudInfo[aID].i_atype == AUDIO_ID_TRUEHD )
			CDMX_INST(m_pvCdmxInst)->stCdmxCtrl.lSpdifTrueHDSub = 1;
		else
			CDMX_INST(m_pvCdmxInst)->stCdmxCtrl.lSpdifTrueHDSub = 0;

		ret = m_pfnCdmx( CDMX_SEL_STREAM, CDMX_HANDLE(m_pvCdmxInst), &st_sel_strm, &CDK->gsCDmxInfo );
		if ( ret < 0 ) {
			ALOGE("[Err:%4d] CDMX_SEL_STREAM failed \n", ret );
			ret = m_pfnCdmx( CDMX_CLOSE, CDMX_HANDLE(m_pvCdmxInst), NULL, NULL );
			return TCC_CDK_WRAPPER_SELECT_STREAM_ERROR;
		}
		iAudID = aID; 
	}
	
	codec_exist |= EXIST_AUDIO;
	audio_end = false;
	
	return TCC_CDK_WRAPPER_OK;
}

int32_t TCC_CDK_Wrapper::GetSubtitleType(void)
{
	CDK_ITEM* CDK = static_cast<CDK_ITEM*>(pItem);
	int preview_mode = 0;
	char value[PROPERTY_VALUE_MAX];
    int ret = 0;
	int output_device = 0;
	memset(value, 0, PROPERTY_VALUE_MAX);
	property_get("tcc.solution.preview", value, "");
	preview_mode = atoi(value);
	
	if(iFileInfo.iNumSubtitleStreams > 0 && (preview_mode == 0))
	{
#ifdef PGS_CAPTION_SUPPORT_INCLUDE			
        if(i_ctype == CONTAINER_TYPE_TS)
        {
           if((iGetStreamMode & CDK_GETSTREAM_SEQUENTIAL_MODE) && (iGetMetadataMode == false) && (GetSentPGSEvent() == false))
           {
              output_device = m_InterSubtitle.CreateCaptionDisplayThread();
              if(output_device != -1)
              {	
                 ret = m_InterSubtitle.gs_decoder_open(CDK->gsCDmxInfo.m_sVideoInfo.m_iWidth,
                                                           CDK->gsCDmxInfo.m_sVideoInfo.m_iHeight);
                 if(ret){
                    SetSentPGSEvent(true);
                    set_inter_subtitle_type(TCC_MEDIAEVENT_READY_TO_DISPLAY_PGS);
                    
                    if(output_device == OUTPUT_DEVICE_LCD)
                       return TCC_MEDIAEVENT_READY_TO_DISPLAY_PGS;
                    else if(output_device == OUTPUT_DEVICE_EXTEND_DEVICE)
                       return TCC_MEDIAEVENT_NONE;
                 }
                 else{
                    m_InterSubtitle.gs_decoder_close();
                    return TCC_MEDIAEVENT_NONE;
                 }
              }
           }
        }
#endif
#ifdef INTERNAL_SUBTITLE_INCLUDE
       if(i_ctype == CONTAINER_TYPE_MKV || i_ctype == CONTAINER_TYPE_MP4)
       {
          m_InterSubtitle.Create( SUBTITLE_MAX_FIFO_NUM, SUBTITLE_RINGBUF_SIZE, SUBTITLE_ONE_ELEMENT_MAX ) ;
          SetSentSubtitleEvent(true);
          SetDetectIntSubtitle(true);
		  set_inter_subtitle_type(TCC_MEDIAEVENT_READY_TO_DISPLAY_TIMEDTEXT);
          return TCC_MEDIAEVENT_READY_TO_DISPLAY_TIMEDTEXT;
       }
#endif
	}
	return TCC_MEDIAEVENT_NONE;
}

int32_t TCC_CDK_Wrapper::SelectSubtitleStream(int32_t sID)
{
	int32_t subID; 
	int32_t ret;
	CDK_ITEM* CDK = static_cast<CDK_ITEM*>(pItem);

	if(iFileInfo.iNumSubtitleStreams > 0)
	{
		subID = sID - iFileInfo.iNumVidStreams - iFileInfo.iNumAudStreams;
		if(subID < 0 )
		{
			PrintError("[SelectSubtitleStream:Err%d] subID is invalid", subID );
			return TCC_CDK_WRAPPER_SELECT_STREAM_ERROR;
		}
	}

	if (iSubID != subID && subID < (int32_t)iFileInfo.iNumSubtitleStreams)
	{
		cdmx_select_stream_t st_sel_strm;
		memset(&st_sel_strm, 0, sizeof(st_sel_strm));

	#ifdef PGS_CAPTION_SUPPORT_INCLUDE
		if(i_ctype == CONTAINER_TYPE_TS && iFileInfo.iNumSubtitleStreams > 0 && GetSentPGSEvent() == true)
		{
			m_InterSubtitle.gs_decoder_reset(); //Reset PGS Decoder
			m_InterSubtitle.ClearPGSQueue();
		}

		if(i_ctype == CONTAINER_TYPE_TS)
		{
			st_sel_strm.lSelType = DMXTYPE_GRAPHICS;
			st_sel_strm.lGraphicsID = iSubIDList[subID];
		}
		else
	#endif
		{
			st_sel_strm.lSelType = DMXTYPE_SUBTITLE;
			st_sel_strm.lSubtitleID = iSubIDList[subID];
		}

		ret = m_pfnCdmx( CDMX_SEL_STREAM, CDMX_HANDLE(m_pvCdmxInst), &st_sel_strm, &CDK->gsCDmxInfo );
		if ( ret < 0 ) {
			ALOGE("[Err:%4d] CDMX_SEL_STREAM failed \n", ret );
			ret = m_pfnCdmx( CDMX_CLOSE, CDMX_HANDLE(m_pvCdmxInst), NULL, NULL );
			return TCC_CDK_WRAPPER_SELECT_STREAM_ERROR;
		}
		iSubID = subID; 

		ClearStreamBuffer(AV_PACKET_SUBTITLE);
	}	


	return TCC_CDK_WRAPPER_OK;
}

int32_t TCC_CDK_Wrapper::GetCurrentSubtitleStream(void)
{
	return iSubID;
}

char* TCC_CDK_Wrapper::GetLangSubtitleStream(void)
{
	int32_t ret;
	CDK_ITEM* CDK = static_cast<CDK_ITEM*>(pItem);

	if ((int32_t)iFileInfo.iNumSubtitleStreams > 0)
	{
		return CDK->gsCDmxInfo.m_sSubtitleInfo.m_pszLanguage;
	}
	
	return NULL;
}

int32_t TCC_CDK_Wrapper::PrepareDemuxerWithAppropriateMode(int64_t prefillUs)
{
	bool video_exist = (iFileInfo.iNumVidStreams > 0 && video_end == false) ? true : false;
	bool audio_exist = (iFileInfo.iNumAudStreams > 0 && audio_end == false) ? true : false;
	bool subtitle_exist = (iFileInfo.iNumSubtitleStreams > 0 && (video_end == false || audio_end == false) && (i_ctype == CONTAINER_TYPE_MP4)) ? true : false;

	if( iGetStreamMode & CDK_GETSTREAM_SELECTIVE_MODE )
		ALOGI("[PrepareDemuxer] iGetStreamMode: CDK_GETSTREAM_SELECTIVE_MODE");
	if( iGetStreamMode & CDK_GETSTREAM_SEQUENTIAL_MODE )
		ALOGI("[PrepareDemuxer] iGetStreamMode: CDK_GETSTREAM_SEQUENTIAL_MODE");
	if( iGetStreamMode & DEMUXER_THREAD_ENABLED_MODE )
		ALOGI("[PrepareDemuxer] iGetStreamMode: DEMUXER_THREAD_ENABLED_MODE");
	if( iGetStreamMode & DEMUXER_LOOPER_ENABLED_MODE )
		ALOGI("[PrepareDemuxer] iGetStreamMode: DEMUXER_LOOPER_ENABLED_MODE");

	if( iGetMetadataMode == true )
		return TCC_CDK_WRAPPER_OK;

	if( iGetStreamMode & CDK_GETSTREAM_SEQUENTIAL_MODE )
	{
		uint32_t sequentialbufferSize;
		char value[PROPERTY_VALUE_MAX];

		// setup video buffer and stream queue
		if( video_exist ) {
			property_get("tcc.demuxer.video.bufsize", value, "0");
			sequentialbufferSize = atoi(value);
			if (sequentialbufferSize == 0)
				sequentialbufferSize = VIDEO_QUEUE_BUFFER_SIZE;
			if(iDemuxStreamQueues.indexOfKey(AV_PACKET_VIDEO) < 0) {
				iDemuxStreamQueues.add(AV_PACKET_VIDEO, 
							new StreamQueue(AV_PACKET_VIDEO, sequentialbufferSize, VIDEO_ELEMENT_SIZE));
				PrintInfo("%s : use StreamQueue(video)", __func__);
			}
		}

		// setup audio buffer and stream queue
		if( audio_exist ) {
			property_get("tcc.demuxer.audio.bufsize", value, "0");
			sequentialbufferSize = atoi(value);
			if (sequentialbufferSize == 0)
				sequentialbufferSize = AUDIO_QUEUE_BUFFER_SIZE;
			if(iDemuxStreamQueues.indexOfKey(AV_PACKET_AUDIO) < 0) {
				iDemuxStreamQueues.add(AV_PACKET_AUDIO, 
							new StreamQueue(AV_PACKET_AUDIO, sequentialbufferSize, AUDIO_ELEMENT_SIZE));
				PrintInfo("%s : use StreamQueue(audio)", __func__);
			}
		}

		// setup subtitle buffer and stream queue
		if( subtitle_exist ) {
			if(iDemuxStreamQueues.indexOfKey(AV_PACKET_SUBTITLE) < 0) {
				iDemuxStreamQueues.add(AV_PACKET_SUBTITLE, 
							new StreamQueue(AV_PACKET_SUBTITLE, SUBTITLE_QUEUE_BUFFER_SIZE, SUBTITLE_ELEMENT_SIZE));
				PrintInfo("%s : use StreamQueue(subtitle)", __func__);
			}
		}

		if( iGetStreamMode & DEMUXER_THREAD_ENABLED_MODE ) 
		{
			mDemuxerGetFrameReqCnt = mDemuxerGetFrameRespCnt = 0;
			if(mDemuxerThread == NULL)
			{
				int queidx;
				if( (queidx = iDemuxStreamQueues.indexOfKey(AV_PACKET_VIDEO)) >= 0 )
					iDemuxStreamQueues.valueAt(queidx)->SetBufferingThreshold(StreamQueue::VIDEO, StreamQueue::THRESHOLD_TYPE_TIME);
				if( (queidx = iDemuxStreamQueues.indexOfKey(AV_PACKET_AUDIO)) >= 0 )
					iDemuxStreamQueues.valueAt(queidx)->SetBufferingThreshold(StreamQueue::AUDIO, StreamQueue::THRESHOLD_TYPE_TIME);
				if( (queidx = iDemuxStreamQueues.indexOfKey(AV_PACKET_SUBTITLE)) >= 0 )
					iDemuxStreamQueues.valueAt(queidx)->SetBufferingThreshold(StreamQueue::SUBTITLE, StreamQueue::THRESHOLD_TYPE_COUNT);

				createAndRunDemuxerThread();

				// guarantee demuxer thread not to underrun at the start time of the playback.
				int32_t prefillDurationVideoMs = bIsWFDPlayMode ? 0 : 1000;
				int32_t prefillDurationAudioMs = prefillDurationVideoMs;
				mDemuxerThread->FeedData(TCCDemuxerThread::CMD_FEED, StreamQueue::THRESHOLD_TYPE_TIME, 
								prefillDurationVideoMs, prefillDurationAudioMs, TCCDemuxerThread::MSG_SYNC_ON);
			}
		}
		else if( iGetStreamMode & DEMUXER_LOOPER_ENABLED_MODE ) 
		{ 
			mCDKDemuxHandler = new CDKDemuxHandler(this);
			mCDKDemuxHandler->start(prefillUs / 1000);
		}
	}
	else if( iGetStreamMode & CDK_GETSTREAM_SELECTIVE_MODE ) 
	{
		if( iGetStreamMode & DEMUXER_THREAD_ENABLED_MODE || iGetStreamMode & DEMUXER_LOOPER_ENABLED_MODE )
		{
			uint32_t sequentialbufferSize;
			char value[PROPERTY_VALUE_MAX];
	
			// setup video buffer and stream queue
			if( video_exist ) {
				property_get("tcc.demuxer.video.bufsize", value, "0");
				sequentialbufferSize = atoi(value);
				if (sequentialbufferSize == 0)
					sequentialbufferSize = VIDEO_QUEUE_BUFFER_SIZE;
				if(iDemuxStreamQueues.indexOfKey(AV_PACKET_VIDEO) < 0) {
					iDemuxStreamQueues.add(AV_PACKET_VIDEO, 
								new StreamQueue(AV_PACKET_VIDEO, sequentialbufferSize, VIDEO_ELEMENT_SIZE));
					PrintInfo("%s : use StreamQueue(video)", __func__);
				}
			}
	
			// setup audio buffer and stream queue
			if( audio_exist ) {
				property_get("tcc.demuxer.audio.bufsize", value, "0");
				sequentialbufferSize = atoi(value);
				if (sequentialbufferSize == 0)
					sequentialbufferSize = AUDIO_QUEUE_BUFFER_SIZE;
				if(iDemuxStreamQueues.indexOfKey(AV_PACKET_AUDIO) < 0) {
					iDemuxStreamQueues.add(AV_PACKET_AUDIO, 
								new StreamQueue(AV_PACKET_AUDIO, sequentialbufferSize, AUDIO_ELEMENT_SIZE));
					PrintInfo("%s : use StreamQueue(audio)", __func__);
				}
			}

			// setup subtitle buffer and stream queue
			if( subtitle_exist ) {
				if(iDemuxStreamQueues.indexOfKey(AV_PACKET_SUBTITLE) < 0) {
					iDemuxStreamQueues.add(AV_PACKET_SUBTITLE, 
								new StreamQueue(AV_PACKET_SUBTITLE, SUBTITLE_QUEUE_BUFFER_SIZE, SUBTITLE_ELEMENT_SIZE));
					PrintInfo("%s : use StreamQueue(subtitle)", __func__);
				}
			}

            if (iGetStreamMode & DEMUXER_THREAD_ENABLED_MODE) {
                mDemuxerGetFrameReqCnt = mDemuxerGetFrameRespCnt = 0;
                if(mDemuxerThread == NULL)
                {
                    int queidx;
                    if( (queidx = iDemuxStreamQueues.indexOfKey(AV_PACKET_VIDEO)) >= 0 )
                        iDemuxStreamQueues.valueAt(queidx)->SetBufferingThreshold(StreamQueue::VIDEO, StreamQueue::THRESHOLD_TYPE_TIME);
                    if( (queidx = iDemuxStreamQueues.indexOfKey(AV_PACKET_AUDIO)) >= 0 )
                        iDemuxStreamQueues.valueAt(queidx)->SetBufferingThreshold(StreamQueue::AUDIO, StreamQueue::THRESHOLD_TYPE_TIME);
                    if( (queidx = iDemuxStreamQueues.indexOfKey(AV_PACKET_SUBTITLE)) >= 0 )
                        iDemuxStreamQueues.valueAt(queidx)->SetBufferingThreshold(StreamQueue::SUBTITLE, StreamQueue::THRESHOLD_TYPE_COUNT);

                    createAndRunDemuxerThread();

                    // guarantee demuxer thread not to underrun at the start time of the playback.
                    int32_t prefillDurationVideoMs = bIsWFDPlayMode ? 0 : 1000;
                    int32_t prefillDurationAudioMs = prefillDurationVideoMs;
                    mDemuxerThread->FeedData(TCCDemuxerThread::CMD_FEED, StreamQueue::THRESHOLD_TYPE_TIME,
                            prefillDurationVideoMs, prefillDurationAudioMs, TCCDemuxerThread::MSG_SYNC_ON);
                }
            }
            else
            {
                mCDKDemuxHandler = new CDKDemuxHandler(this);
                mCDKDemuxHandler->start(prefillUs / 1000);
            }
		}
	}

	bPrepared = true;

	return TCC_CDK_WRAPPER_OK;
}

int32_t TCC_CDK_Wrapper::PrepareDemuxerWithSelectiveMode(bool *pbModeChanged)
{
	int32_t ret;
	int i_parsing_mode = 0;

	if( iGetStreamMode & CDK_GETSTREAM_SELECTIVE_MODE ) {
		ALOGI("[PrepareDemuxer] mode: selective --> selective without thread");
		iGetStreamMode = CDK_GETSTREAM_SELECTIVE_MODE;
		return TCC_CDK_WRAPPER_OK;
	}

    if (pbModeChanged != NULL)
		*pbModeChanged = true;
	ALOGI("[PrepareDemuxer] mode: sequential --> selective without thread");

	ret = CloseCdmx ();
	if ( ret < 0 ) {
		ALOGE("[CDK_WRAPPER] CloseCdmx failed\n");
		return ret;
	}
	
	// we don't need to set the demuxer mode as sequential in case of audio (or video) only playback.
	iGetStreamMode = CDK_GETSTREAM_SELECTIVE_MODE;

	// Now, recreate the CDMX with selective mode.
    i_parsing_mode &= ~CDMX_MARKER_SEQUENTIAL_PLAY_MODE_BIT;
    i_parsing_mode |= CDMX_MARKER_DEFAULT_PLAY_MODE_BIT;

	ALOGV("[CDK_WRAPPER] i_parsing_mode = %d\n", i_parsing_mode);
	
	ret = InitCdmx (i_parsing_mode);
	if ( ret < 0 ) {
		ALOGE("[CDK_WRAPPER] InitCdmx failed\n");
		return ret;
	}

	return TCC_CDK_WRAPPER_OK;
}

int32_t TCC_CDK_Wrapper::GetLastKeyFramePTS()
{
    CDK_ITEM* CDK = static_cast<CDK_ITEM*>(pItem);

    return CDK->gsCDmxInfo.m_sVideoInfo.m_iLastKeyTime;
}

void TCC_CDK_Wrapper::GetMIMEType()
{
	uint32_t i;
	uint32_t uiUnknownFormatCount=0;

	for (i = 0 ; i < iFileInfo.iNumVidStreams ; i++)
	{
        iVidInfo.editItemAt(i).mimetype = TCC_MIME_VIDEO_NOTSUPPORTED; // cannot play video
        char value[PROPERTY_VALUE_MAX];
        property_get("tcc.video.notsupported.config", value, "1");
        // 0: Do NOT connect on list of files
        // 1: This video cannot be played.(default)
        // 2: The file has to be played if there is even one video or audio.
        int i_flag = atoi(value);
        if (i_flag == 0) {
            ALOGE("Do NOT connect on list of files");
            iVidInfo.editItemAt(i).mimetype = TCC_MIME_VIDEO_UNKNOWN; // don't register on gallery
        }

		if (iVidInfo[i].iID != -1)
		{
			switch (iVidInfo[i].i_vtype)
			{
				case STD_AVC:
					iVidInfo.editItemAt(i).mimetype = TCC_MIME_H264; break;
				case STD_H263:
					iVidInfo.editItemAt(i).mimetype = TCC_MIME_H263; break;
				case STD_MPEG4:
					iVidInfo.editItemAt(i).mimetype = TCC_MIME_MPEG4; break;
				case STD_MPEG2:
					iVidInfo.editItemAt(i).mimetype = TCC_MIME_MPEG2; break;
				case STD_VC1:
					iVidInfo.editItemAt(i).mimetype = TCC_MIME_VC1;
 					break;
				case STD_DIV3:
					iVidInfo.editItemAt(i).mimetype = TCC_MIME_DIVX; break;
				case STD_RV:
					iVidInfo.editItemAt(i).mimetype = TCC_MIME_REAL_VIDEO;
					break;
				case STD_SH263:
					iVidInfo.editItemAt(i).mimetype = TCC_MIME_FLV1; break;
				#ifdef INCLUDE_SORENSON263_DEC
				case STD_SORENSON263:
					iVidInfo.editItemAt(i).mimetype = TCC_MIME_FLV1; break;
				#endif
				#ifdef INCLUDE_WMV78_DEC
				case STD_WMV78:
					iVidInfo.editItemAt(i).mimetype = TCC_MIME_WMV_1_2;
 					break;
				#endif
				case STD_MJPG:
					iVidInfo.editItemAt(i).mimetype = TCC_MIME_MJPEG; break;
				case STD_AVS:
					iVidInfo.editItemAt(i).mimetype = TCC_MIME_AVS; break;
				case STD_VP8:
					iVidInfo.editItemAt(i).mimetype = TCC_MIME_VP8; break;
				case STD_MVC:
					iVidInfo.editItemAt(i).mimetype = TCC_MIME_MVC; break;
				case CODEC_VP9:
					iVidInfo.editItemAt(i).mimetype = TCC_MIME_VP9; break;
				default:
					uiUnknownFormatCount++; break;
			}
		}
		else
		{
			uiUnknownFormatCount++;
		}
	}

#if 0 // The file has to be played if there is even one video or audio.
	if (uiUnknownFormatCount == i && iFileInfo.iNumVidStreams > 0)
	{
		PrintError("[CDK_CORE:Err%d] Audio is ignored because video cannot be supported.", -1);
		for (i = 0 ; i < iFileInfo.iNumAudStreams ; i++)
		{
			iAudInfo.editItemAt(i).mimetype = TCC_MIME_AUDIO_UNKNOWN;
		}
		return;
	}
#endif

	for (i = 0 ; i < iFileInfo.iNumAudStreams ; i++)
	{
		iAudInfo.editItemAt(i).mimetype = TCC_MIME_AUDIO_UNKNOWN;

		if (iAudInfo[i].iID != -1)
		{
			switch (iAudInfo[i].i_atype)
			{
				case AUDIO_ID_AAC:
					iAudInfo.editItemAt(i).mimetype = TCC_MIME_AAC_TCC; break; //TCC_MIME_MPEG4_AUDIO; break;
				case AUDIO_ID_AAC_GOOGLE:
					iAudInfo.editItemAt(i).mimetype = TCC_MIME_AAC_GOOGLE; break; // for google aac decoder
				case AUDIO_ID_MP3:
				#if USE_PV_MP3_CODEC
					iAudInfo.editItemAt(i).mimetype = TCC_MIME_MP3; break;
				#else
					iAudInfo.editItemAt(i).mimetype = TCC_MIME_MP3_TCC; break;
				#endif
				case AUDIO_ID_AMR:
					iAudInfo.editItemAt(i).mimetype = TCC_MIME_AMR_NB; break;
				case AUDIO_ID_AMRWBP:
					iAudInfo.editItemAt(i).mimetype = TCC_MIME_AMR_WB; break;
				case AUDIO_ID_WAV:
					iAudInfo.editItemAt(i).mimetype = TCC_MIME_PCM_TCC; break; //TCC_MIME_PCM; break;
				case AUDIO_ID_MP2:
					iAudInfo.editItemAt(i).mimetype = TCC_MIME_MP2; break;
				case AUDIO_ID_DTS:
					if (AUDIO_SUPPORT[SUPPORT_DTS]) // AudioOutput update Planet 20121120
					{
						iAudInfo.editItemAt(i).mimetype = TCC_MIME_DTS;
					}
 					break;
				case AUDIO_ID_DTSHD:
					if (AUDIO_SUPPORT[SUPPORT_DTS] == 2)
					{
						iAudInfo.editItemAt(i).mimetype = TCC_MIME_DTSHD;
					}
					else if (AUDIO_SUPPORT[SUPPORT_DTS] == 1)
					{
						iAudInfo.editItemAt(i).mimetype = TCC_MIME_DTS;
					}
 					break;
				case AUDIO_ID_QCELP:
					iAudInfo.editItemAt(i).mimetype = TCC_MIME_QCELP; break;
				case AUDIO_ID_WMA:
					if (AUDIO_SUPPORT[SUPPORT_WMA])
					{
						iAudInfo.editItemAt(i).mimetype = TCC_MIME_WMA;
					}
 					break;
				case AUDIO_ID_EVRC:
					iAudInfo.editItemAt(i).mimetype = TCC_MIME_EVRC; break;
				case AUDIO_ID_COOK:
					if (AUDIO_SUPPORT[SUPPORT_RA])
					{
						iAudInfo.editItemAt(i).mimetype = TCC_MIME_REAL_AUDIO;
					}
 					break;
				case AUDIO_ID_FLAC:
					iAudInfo.editItemAt(i).mimetype = TCC_MIME_FLAC; break;
				case AUDIO_ID_APE:
					iAudInfo.editItemAt(i).mimetype = TCC_MIME_APE; break;
				case AUDIO_ID_VORBIS:
					iAudInfo.editItemAt(i).mimetype = TCC_MIME_VORBIS; break;
				case AUDIO_ID_AC3:
					if (AUDIO_SUPPORT[SUPPORT_AC3] == 1 || AUDIO_SUPPORT[SUPPORT_DDP] == 1)
					{
						iAudInfo.editItemAt(i).mimetype = TCC_MIME_AC3;
					}
 					break;
				case AUDIO_ID_DDP:
					ALOGD("get mimetype AUDIO_ID_DDP AUDIO_SUPPORT[SUPPORT_DDP] %d ",AUDIO_SUPPORT[SUPPORT_DDP]);
					if (AUDIO_SUPPORT[SUPPORT_DDP] == 1)
					{
						iAudInfo.editItemAt(i).mimetype = TCC_MIME_AC3;
					}
					else if (AUDIO_SUPPORT[SUPPORT_DDP] == 2)
					{
						iAudInfo.editItemAt(i).mimetype = TCC_MIME_DDP;
					}
					break;
				case AUDIO_ID_TRUEHD:
					if (AUDIO_SUPPORT[SUPPORT_AC3] == 2)
					{
						iAudInfo.editItemAt(i).mimetype = TCC_MIME_TRUEHD;
					}
					else if (AUDIO_SUPPORT[SUPPORT_AC3] == 1)
					{
						iAudInfo.editItemAt(i).mimetype = TCC_MIME_AC3;
					}
 					break;
 				case AUDIO_ID_MP1:
 					// MP1 audio is decoded by the MP3 decoder
 					iAudInfo.editItemAt(i).mimetype = TCC_MIME_MP3_TCC; 
 					break;	
				default: break;
			}
		}
	}

	for (i = 0 ; i < iFileInfo.iNumSubtitleStreams ; i++)
	{
		iSubInfo.editItemAt(i).mimetype = TCC_MIME_TEXT_UNKNOWN;

		if (iSubInfo[i].iID != -1)
		{
			switch (iSubInfo[i].i_stype)
			{
				case 0:
					iSubInfo.editItemAt(i).mimetype = TCC_MIME_TEXT_GRAPHIC;
					break;
				case 1:
					iSubInfo.editItemAt(i).mimetype = TCC_MIME_TEXT_3GPP;
					break;
				default: break;
			}
		}
	}
}

void TCC_CDK_Wrapper::CDMXErrorProcess(int32_t& ret, int32_t packetNum)
{
	if ((ret == ERR_END_OF_AUDIO_FILE) || (ret == ERR_END_OF_VIDEO_FILE) || (ret == ERR_END_OF_FILE))
	{
		LOGMSG("[CDMXErrorProcess] End of stream(s): ret:%d", ret);
		if(packetNum == AV_PACKET_VIDEO) 
			video_end = true;
		else if(packetNum == AV_PACKET_AUDIO)
			audio_end = true;
		else
			video_end = audio_end = true;

		ret = ( audio_end && video_end ) ? TCC_CDK_WRAPPER_END_OF_FILE : TCC_CDK_WRAPPER_END_OF_STREAM;
	}
	else
	{
		LOGMSG("[CDMXErrorProcess] Demuxer Error: ret:%d", ret);
		ret = (packetNum < 0) ? TCC_CDK_WRAPPER_SEEK_ERROR : TCC_CDK_WRAPPER_DEMUXER_ERROR;
	}
}

#ifdef MERGE_FRAME_AVG
int32_t
TCC_CDK_Wrapper::DefragmentFrame (
    uint8_t*		pTempBuff,
    StreamQueue*	pStreamQueue,
    uint8_t*        arBuffer, 
    uint32_t&       aSize, 
    uint32_t&       aTimeStamp, 
    uint32_t&       aExtra
    )
{
    int32_t ret = 0;
	int32_t queue_empty_cnt = 0;	//20121023 add for skimming
#if 0	
	long frame_complete = 0;
	long used_bytes = 0;
	long read_bytes = 0;
	long frame_size = 0;
	unsigned long sync = 0;
#endif
    
    if ( m_bSeekFlag || !bSentFirstVideoFrame )
    {
        m_ulPreviousTime = 0;
		m_ulCurrentTime = 0;
		m_bMultiplePacketFrame = 0;
        m_pbyTempBuffBackup = NULL;
		//m_bSeekFlag = 0;
		m_lDataLen = 0;
		m_after_seq = FALSE;
		m_after_pic = FALSE;
		m_lStartPos = 0;
		m_lEndPos = 0;
		m_lAudUsed = -1;
		m_lSpsPpsDect = 0;
		m_lPrevStatus = 0;
		m_bQueueEmpty = 0;
    }
    
    if ( m_lDataLen  && m_pbyTempBuffBackup )
    {
        pTempBuff = m_pbyTempBuffBackup;
    
        if (m_lVideoCodecType == STD_AVC)
        {
			m_lFrame_complete = MergeVideoFrameAVC( arBuffer + m_lFrame_size, 0,
				pTempBuff+m_lStartPos, m_lDataLen,
				&m_lUsed_bytes,
				&m_lRead_bytes,
				&m_lAudUsed);
		}
	else if(m_lVideoCodecType == STD_MPEG2 ||m_lVideoCodecType == STD_MPEG4 ||m_lVideoCodecType == STD_VC1 ||m_lVideoCodecType == STD_AVS)
		{
			m_lFrame_complete = TSD_GetMpegFrameStartPos(	arBuffer , 0,
				pTempBuff+m_lStartPos, m_lDataLen,
				&m_lUsed_bytes,
				&m_lRead_bytes,
				m_ulPicHeaderType,
				m_ulPicHeaderType2,
				m_ulSeqHeaderType);
		}

		m_lFrame_size  += m_lUsed_bytes;
		m_lStartPos += m_lRead_bytes;
		m_lDataLen  -= m_lRead_bytes;

		if (m_lFrame_complete)
		{
			aSize = m_lFrame_size;
			m_lFrame_size = 0;
			//add for test skimming 20121016
			m_lUsed_bytes = 0;
			m_lRead_bytes = 0;
			m_ulSync = 0;
            if (m_lDataLen)
            {
                m_pbyTempBuffBackup = pTempBuff;
                aTimeStamp = m_ulPreviousTime;
            } 
            else
            {
                PrintError("MERGE_FRAME ERROR1");
            }
            return TCC_CDK_WRAPPER_OK;
        }
    
        if (m_lDataLen <= 4)
        {
            unsigned char *p_tmp_buff = pTempBuff + m_lStartPos;
			m_ulSync = ((unsigned long)p_tmp_buff[0] << 16) | ((unsigned long)p_tmp_buff[1] << 8) | (unsigned long)p_tmp_buff[2];
			if ( (m_ulSync & 0x00FFFFFF) != 0x000001  && m_lVideoCodecType == STD_AVC)
			{
				memcpy(arBuffer + m_lFrame_size, p_tmp_buff, m_lDataLen);
				m_lFrame_size += m_lDataLen;
				m_lDataLen -= m_lDataLen; // ???
				m_lStartPos = 0;
			} 
	     		else if(((m_ulSync & 0x00FFFFFF) != m_ulPicHeaderType || (m_ulSync & 0x00FFFFFF) != m_ulPicHeaderType2) && (m_ulSync & 0x00FFFFFF) != m_ulSeqHeaderType 
		 	&& (m_lVideoCodecType == STD_MPEG2 || m_lVideoCodecType == STD_MPEG4 || m_lVideoCodecType == STD_VC1 ||m_lVideoCodecType == STD_AVS))
			{
				memcpy(arBuffer + m_lFrame_size, p_tmp_buff, m_lDataLen);
				m_lFrame_size += m_lDataLen;
				m_lDataLen -= m_lDataLen; // ???
				m_lStartPos = 0;
			}
			else if( (m_ulSync & 0x00FFFFFF) == 0x000001  && m_lVideoCodecType == STD_AVC)
			{
				memcpy(pTempBuff, p_tmp_buff, m_lDataLen);
			}
		}

		if (!m_lDataLen)
			pStreamQueue->Pop();
	}

	while (m_lFrame_complete == 0)
    {
    	if(m_lDataLen)
    	{
			memcpy(m_MergeFrame_Temp, pTempBuff, m_lDataLen);
			pStreamQueue->Pop();
			pTempBuff = pStreamQueue->GetOneStream(&aSize, &aTimeStamp, NULL, NULL, &ret);
			if(aSize == 0)
			{
				queue_empty_cnt++;

				int index = iDemuxStreamQueues.indexOfKey(AV_PACKET_VIDEO);
				if( index >= 0 )
					SendReqMsgToFeedDataIfUnderruned(StreamQueue::THRESHOLD_TYPE_CLIP_TIME, index);

				//for test 20121011
				if(bIsUnderSkimming)
					m_bSeekFlag = 0;
				if (ret < 0)
				{
					CDMXErrorProcess(ret, AV_PACKET_VIDEO);
					if ((DEMUXER_THREAD_ENABLED_MODE & iGetStreamMode) && aExtra != SKIP_OPERATION_RESUME)
					{
						if (ret == TCC_CDK_WRAPPER_END_OF_FILE)
						{
							mDemuxerThread->SetState(TCCDemuxerThread::STATE_FINISHED);
						}
					}
					return ret;
				}
				if(queue_empty_cnt > 100)
				{
					//LOGD("queue-empty");
					return TCC_CDK_WRAPPER_GET_FRAME_NEXT_TIME;
				}
			}
				memcpy(m_MergeFrame_Temp +m_lDataLen , pTempBuff, aSize);
				pTempBuff = m_MergeFrame_Temp;			
				aSize +=  m_lDataLen;
				m_lDataLen = 0;
    	}
	else
		{
			pTempBuff = pStreamQueue->GetOneStream(&aSize, &aTimeStamp, NULL, NULL, &ret);
			if(aSize == 0)
			{
				queue_empty_cnt++;

				int index = iDemuxStreamQueues.indexOfKey(AV_PACKET_VIDEO);
				if( index >= 0 )
					SendReqMsgToFeedDataIfUnderruned(StreamQueue::THRESHOLD_TYPE_CLIP_TIME, index);

				m_bQueueEmpty = 1;
				if (ret < 0)
				{
					CDMXErrorProcess(ret, AV_PACKET_VIDEO);
					if ((DEMUXER_THREAD_ENABLED_MODE & iGetStreamMode) && aExtra != SKIP_OPERATION_RESUME)
					{
						if (ret == TCC_CDK_WRAPPER_END_OF_FILE)
						{
							mDemuxerThread->SetState(TCCDemuxerThread::STATE_FINISHED);
						}
					}
					return ret;
				}
				if(queue_empty_cnt > 100)
				{
					//LOGD("queue-empty - 1");
					return TCC_CDK_WRAPPER_GET_FRAME_NEXT_TIME;
				}
			}
		}
	
        if (ret < 0)
        {
            CDMXErrorProcess(ret, AV_PACKET_VIDEO);
            if ((DEMUXER_THREAD_ENABLED_MODE & iGetStreamMode) && aExtra != SKIP_OPERATION_RESUME)
            {
                if (ret == TCC_CDK_WRAPPER_END_OF_FILE)
                {
                    mDemuxerThread->SetState(TCCDemuxerThread::STATE_FINISHED);
                }
            }
            return ret;
        }
    
        if (pTempBuff != NULL)
        {
			m_lDataLen = aSize;
			m_lStartPos = 0;
			m_bQueueEmpty = 0;
            if (m_lVideoCodecType == STD_AVC)
            {
                if ( m_lAudUsed < 0 )
                    m_lAudUsed = scan_h264_aud(pTempBuff+m_lStartPos, m_lDataLen);
            }
    
            if (m_lVideoCodecType == STD_AVC)
            {
				m_lFrame_complete = MergeVideoFrameAVC( arBuffer+m_lFrame_size, 0,
					pTempBuff + m_lStartPos, m_lDataLen,
					&m_lUsed_bytes,
					&m_lRead_bytes,
					&m_lAudUsed );
			}
	     else if(m_lVideoCodecType == STD_MPEG2 || m_lVideoCodecType == STD_MPEG4 ||m_lVideoCodecType == STD_VC1 ||m_lVideoCodecType == STD_AVS)
			{
MERGE_MPEG:		m_lFrame_complete = TSD_GetMpegFrameStartPos( arBuffer + m_lFrame_size, 0,
					pTempBuff+m_lStartPos, m_lDataLen,
					&m_lUsed_bytes,
					&m_lRead_bytes,
					m_ulPicHeaderType,
								m_ulPicHeaderType2,
					m_ulSeqHeaderType);
			}

			m_lFrame_size += m_lUsed_bytes;
			m_lStartPos += m_lRead_bytes;
			m_lDataLen -= m_lRead_bytes;

			if (m_lDataLen <= 4)
			{
				unsigned char *p_tmp_buff = pTempBuff + m_lStartPos;
				m_ulSync = ((unsigned long)p_tmp_buff[0] << 16) | ((unsigned long)p_tmp_buff[1] << 8) | (unsigned long)p_tmp_buff[2];
				if ( (m_ulSync & 0x00FFFFFF) != 0x000001  && m_lVideoCodecType == STD_AVC)
				{
					memcpy(arBuffer + m_lFrame_size, p_tmp_buff, m_lDataLen);
					m_lFrame_size += m_lDataLen;
					m_lDataLen -= m_lDataLen; // ???
					m_lStartPos = 0;
				} 
                else if(((m_ulSync & 0x00FFFFFF) != m_ulPicHeaderType || (m_ulSync & 0x00FFFFFF) != m_ulPicHeaderType2) && (m_ulSync & 0x00FFFFFF) != m_ulSeqHeaderType 
		 	&& (m_lVideoCodecType == STD_MPEG2 || m_lVideoCodecType == STD_MPEG4 || m_lVideoCodecType == STD_VC1 ||m_lVideoCodecType == STD_AVS))
				{
					memcpy(arBuffer + m_lFrame_size, p_tmp_buff, m_lDataLen);
					m_lFrame_size += m_lDataLen;
					m_lDataLen -= m_lDataLen; // ???
					m_lStartPos = 0;
				} 
				else
				{
					PrintError("MERGE_FRAME ERROR3");
				}


			}

			if(m_lFrame_size > (4*1024*1024))
				return TCC_CDK_WRAPPER_UNKNOWN_FILE_TYPE;
			
			if (!m_lFrame_complete)
			{
				pStreamQueue->Pop();
				m_ulPreviousTime = aTimeStamp;
				m_bMultiplePacketFrame = 1;
				m_bSeekFlag = 0;	//add 20121011
			} 
			else
			{
				aSize = m_lFrame_size;
				m_lFrame_size = 0;
				//add for test skimming 20121016
				m_lUsed_bytes = 0;
				m_lRead_bytes = 0;
				m_ulSync = 0;

                if(!aSize && (m_lVideoCodecType == STD_MPEG2 || m_lVideoCodecType == STD_MPEG4 || m_lVideoCodecType == STD_VC1 ||m_lVideoCodecType == STD_AVS))
			goto MERGE_MPEG;

		  if (m_lDataLen)
                {
                    m_pbyTempBuffBackup = pTempBuff;
					if(m_bMultiplePacketFrame || !m_bSeekFlag)
					{
						m_ulCurrentTime = aTimeStamp;							
						aTimeStamp = m_ulPreviousTime;						
						m_ulPreviousTime = m_ulCurrentTime;
						m_bMultiplePacketFrame = 0;
						m_bSeekFlag = 0;
					}
					else if(!m_bMultiplePacketFrame && m_bSeekFlag)
					{
						m_ulPreviousTime = aTimeStamp;
						m_bSeekFlag = 0;
					}
					else
					{
						m_bSeekFlag = 0;
					}				
				} 
				else
				{
                    PrintError("MERGE_FRAME ERROR4");
                }
            }
    
        } 
        else
        {
        	PrintError("[TCC_CDK_Wrapper] Fail to get a video stream");
        }
    }

    if (aSize != 0)
    {
        if (!bSentFirstVideoFrame)
        {
            bSentFirstVideoFrame = true;
        }
		m_lFrame_complete = 0;	//add for skimming 20121016
        // Escape the loop
        return TCC_CDK_WRAPPER_OK;
    }

    return TCC_CDK_WRAPPER_OK;
}

int32_t
TCC_CDK_Wrapper::DefragmentFrame_Selective (
    uint8_t*		pTempBuff,
    void* CDK,
    uint8_t*        arBuffer, 
    uint32_t&       aSize, 
    uint32_t&       aTimeStamp, 
    uint32_t&       aExtra
    )
{
    int32_t ret = 0;
/*	
	long frame_complete = 0;
	long used_bytes = 0;
	long read_bytes = 0;
	long frame_size = 0;
	unsigned long sync = 0;
*/	

    CDK_ITEM* pCDK = static_cast<CDK_ITEM*>(CDK);
    
    if ( m_bSeekFlag || !bSentFirstVideoFrame )
    {
        m_ulPreviousTime = 0;
		m_ulCurrentTime = 0;
		m_bMultiplePacketFrame = 0;
        m_pbyTempBuffBackup = NULL;
		//m_bSeekFlag = 0;
		m_lDataLen = 0;

		m_after_seq = FALSE;
		m_after_pic = FALSE;
		m_lStartPos = 0;
		m_lEndPos = 0;
		m_lAudUsed = -1;
		m_lSpsPpsDect = 0;
		m_lPrevStatus = 0;
    }
    
    if ( m_lDataLen  && m_pbyTempBuffBackup )
    {
        pTempBuff = m_pbyTempBuffBackup;
    
        if (m_lVideoCodecType == STD_AVC)
        {
			m_lFrame_complete = MergeVideoFrameAVC( arBuffer + m_lFrame_size, 0,
				pTempBuff+m_lStartPos, m_lDataLen,
				&m_lUsed_bytes,
				&m_lRead_bytes,
				&m_lAudUsed);
		}
	else if(m_lVideoCodecType == STD_MPEG2 || m_lVideoCodecType == STD_MPEG4 ||m_lVideoCodecType == STD_VC1 ||m_lVideoCodecType == STD_AVS)
		{
			m_lFrame_complete = TSD_GetMpegFrameStartPos(	arBuffer , 0,
				pTempBuff+m_lStartPos, m_lDataLen,
				&m_lUsed_bytes,
				&m_lRead_bytes,
				m_ulPicHeaderType,
								m_ulPicHeaderType2,
				m_ulSeqHeaderType);
		}

		m_lFrame_size  += m_lUsed_bytes;
		m_lStartPos += m_lRead_bytes;
		m_lDataLen  -= m_lRead_bytes;

		if (m_lFrame_complete)
		{
			aSize = m_lFrame_size;
			m_lFrame_size = 0;
			//add for test skimming 20121016
			m_lUsed_bytes = 0;
			m_lRead_bytes = 0;
			m_ulSync = 0;
            if (m_lDataLen)
            {
                m_pbyTempBuffBackup = pTempBuff;
                aTimeStamp = m_ulPreviousTime;
            } 
            else
            {
                PrintError("MERGE_FRAME ERROR1");
            }
            return TCC_CDK_WRAPPER_OK;
        }
    
        if (m_lDataLen <= 4)
        {
            unsigned char *p_tmp_buff = pTempBuff + m_lStartPos;
			m_ulSync = ((unsigned long)p_tmp_buff[0] << 16) | ((unsigned long)p_tmp_buff[1] << 8) | (unsigned long)p_tmp_buff[2];
			if ( (m_ulSync & 0x00FFFFFF) != 0x000001  && m_lVideoCodecType == STD_AVC)
			{
				memcpy(arBuffer + m_lFrame_size, p_tmp_buff, m_lDataLen);
				m_lFrame_size += m_lDataLen;
				m_lDataLen -= m_lDataLen; // ???
				m_lStartPos = 0;
			} 
	     else if(((m_ulSync & 0x00FFFFFF) != m_ulPicHeaderType || (m_ulSync & 0x00FFFFFF) != m_ulPicHeaderType2) && (m_ulSync & 0x00FFFFFF) != m_ulSeqHeaderType 
		 	&& (m_lVideoCodecType == STD_MPEG2 || m_lVideoCodecType == STD_MPEG4 || m_lVideoCodecType == STD_VC1 ||m_lVideoCodecType == STD_AVS))
			{
				memcpy(arBuffer + m_lFrame_size, p_tmp_buff, m_lDataLen);
				m_lFrame_size += m_lDataLen;
				m_lDataLen -= m_lDataLen; // ???
				m_lStartPos = 0;
			}
			else if( (m_ulSync & 0x00FFFFFF) == 0x000001  && m_lVideoCodecType == STD_AVC)
			{
				memcpy(pTempBuff, p_tmp_buff, m_lDataLen);
			}
		}

	}

	while (m_lFrame_complete == 0)
	{
		if (m_lDataLen) 
		{
			pTempBuff += m_lDataLen;
			pCDK->gsCDmxInput.m_pPacketBuff += m_lDataLen;
			pCDK->gsCDmxOutput.m_iPacketSize -= m_lDataLen;
			ret = m_pfnCdmx(CDMX_GETSTREAM, CDMX_HANDLE(m_pvCdmxInst), &pCDK->gsCDmxInput, &pCDK->gsCDmxOutput);
			if (ret < 0) {
				ALOGE("[CDK_WRAPPER:%d] Error:%d\n", __LINE__, ret);
			}
			aTimeStamp = pCDK->gsCDmxOutput.m_iTimeStamp;
			pTempBuff -= m_lDataLen;
			pCDK->gsCDmxInput.m_pPacketBuff -= m_lDataLen;
			pCDK->gsCDmxOutput.m_iPacketSize += m_lDataLen;
			m_lDataLen = 0;
		} 
		else 
		{
			ret = m_pfnCdmx(CDMX_GETSTREAM, CDMX_HANDLE(m_pvCdmxInst), &pCDK->gsCDmxInput, &pCDK->gsCDmxOutput);
			if (ret < 0) {
				ALOGE("[CDK_WRAPPER:%d] Error:%d\n", __LINE__, ret);
			}
			aTimeStamp = pCDK->gsCDmxOutput.m_iTimeStamp;
		}

		if (pTempBuff != NULL) 
		{
			m_lDataLen = pCDK->gsCDmxOutput.m_iPacketSize;
			m_lStartPos = 0;
			if (m_lVideoCodecType == STD_AVC) 
			{
				if (m_lAudUsed < 0) m_lAudUsed = scan_h264_aud(pTempBuff + m_lStartPos, m_lDataLen);
			}

			if (m_lVideoCodecType == STD_AVC) 
			{
				m_lFrame_complete = MergeVideoFrameAVC(arBuffer + m_lFrame_size, 0,
													   pTempBuff + m_lStartPos, m_lDataLen,
													   &m_lUsed_bytes,
													   &m_lRead_bytes,
													   &m_lAudUsed);
			} 
			else if (m_lVideoCodecType == STD_MPEG2 || m_lVideoCodecType == STD_MPEG4 || m_lVideoCodecType == STD_VC1 || m_lVideoCodecType == STD_AVS) 
			{
MERGE_MPEG:
				m_lFrame_complete = TSD_GetMpegFrameStartPos(arBuffer + m_lFrame_size, 0, 
															 pTempBuff + m_lStartPos, m_lDataLen,
															 &m_lUsed_bytes,
															 &m_lRead_bytes,
															 m_ulPicHeaderType,
															 m_ulPicHeaderType2,
															 m_ulSeqHeaderType);
			}

			m_lFrame_size += m_lUsed_bytes;
			m_lStartPos += m_lRead_bytes;
			m_lDataLen -= m_lRead_bytes;

			if (m_lDataLen <= 4) 
			{
				unsigned char *p_tmp_buff = pTempBuff + m_lStartPos;
				m_ulSync = ((unsigned long)p_tmp_buff[0] << 16) | ((unsigned long)p_tmp_buff[1] << 8) | (unsigned long)p_tmp_buff[2];
				if ((m_ulSync & 0x00FFFFFF) != 0x000001  && m_lVideoCodecType == STD_AVC) {
					memcpy(arBuffer + m_lFrame_size, p_tmp_buff, m_lDataLen);
					m_lFrame_size += m_lDataLen;
					m_lDataLen -= m_lDataLen; // ???
					m_lStartPos = 0;
				} else if (((m_ulSync & 0x00FFFFFF) != m_ulPicHeaderType || (m_ulSync & 0x00FFFFFF) != m_ulPicHeaderType2) && (m_ulSync & 0x00FFFFFF) != m_ulSeqHeaderType
						   && (m_lVideoCodecType == STD_MPEG2 || m_lVideoCodecType == STD_MPEG4 || m_lVideoCodecType == STD_VC1 || m_lVideoCodecType == STD_AVS)) {
					memcpy(arBuffer + m_lFrame_size, p_tmp_buff, m_lDataLen);
					m_lFrame_size += m_lDataLen;
					m_lDataLen -= m_lDataLen; // ???
					m_lStartPos = 0;
				} else {
					PrintError("MERGE_FRAME ERROR3");
				}
			}

			if (!m_lFrame_complete) {
				m_ulPreviousTime = aTimeStamp;
				m_bMultiplePacketFrame = 1;
			} else {
				aSize = m_lFrame_size;
				m_lFrame_size = 0;
				//add for test skimming 20121016
				m_lUsed_bytes = 0;
				m_lRead_bytes = 0;
				m_ulSync = 0;

				if (!aSize && (m_lVideoCodecType == STD_MPEG2 || m_lVideoCodecType == STD_MPEG4 || m_lVideoCodecType == STD_VC1 || m_lVideoCodecType == STD_AVS)) goto MERGE_MPEG;

				if (m_lDataLen) {
					m_pbyTempBuffBackup = pTempBuff;
					if (m_bMultiplePacketFrame || !m_bSeekFlag) {
						m_ulCurrentTime = aTimeStamp;
						aTimeStamp = m_ulPreviousTime;
						m_ulPreviousTime = m_ulCurrentTime;
						m_bMultiplePacketFrame = 0;
						m_bSeekFlag = 0;
					} else if (!m_bMultiplePacketFrame && m_bSeekFlag) {
						m_ulPreviousTime = aTimeStamp;
						m_bSeekFlag = 0;
					} else {
						m_bSeekFlag = 0;
					}
				} else {
					PrintError("MERGE_FRAME ERROR4");
				}
			}

		} else {
			PrintError("[TCC_CDK_Wrapper] Fail to get a video stream");
		}
	}

    if (aSize != 0)
    {
        if (!bSentFirstVideoFrame)
        {
            bSentFirstVideoFrame = true;
        }
		m_lFrame_complete = 0;
        // Escape the loop
        return ret;
    }

    return TCC_CDK_WRAPPER_OK;
}
#endif //#ifdef MERGE_FRAME_AVG

#ifdef USE_HDCP_DECRYPTION
bool TCC_CDK_Wrapper::GetDecryptionNeed(void)
{
	if( mEnableHdcp2 ) {
        if(HDCP2_SOURCE_SCHEME == mSourceScheme)
            return true;

		return bIsDecryptionNeeded;
    }

	return false;
}

int32_t TCC_CDK_Wrapper::Stream_Decryption_process(const uint8_t * inData, uint8_t * outData, uint32_t& dataSize, uint32_t streamCtr, uint64_t inputCtr)
{
#if 1 
        return hdcp2_Sink_Process(inData, outData, ((dataSize + 15) >> 4 ) << 4, &streamCtr, &inputCtr);
#else // for simple test of none DRM contents without Decryption.
        memcpy(outData, inData, dataSize);
        return 0;
#endif
}

int32_t TCC_CDK_Wrapper::Stream_Copy_process(const uint8_t * inData, uint8_t * outData, uint32_t& dataSize)
{
	// for simple test of none DRM contents without secured copy.
    memcpy(outData, inData, dataSize);
    return 0;
}

#endif

int32_t TCC_CDK_Wrapper::GetStreamData(uint32_t aType, uint8_t* arBuffer, uint32_t& aSize, uint32_t& aTimeStamp, uint32_t& aExtra, int32_t &iParam, int32_t &iPacketDisCont )
{
	int32_t ret = 0;
	int32_t packet_num = 0;
	iParam = TCC_MEDIAEVENT_NONE; // INTERNAL_SUBTITLE_INCLUDE

	if (aType == STREAM_TYPE_VIDEO)
	{
		if(video_end == true) 
			return TCC_CDK_WRAPPER_END_OF_STREAM; 

		packet_num = AV_PACKET_VIDEO;
	}
	else if (aType == STREAM_TYPE_AUDIO)
	{
		if(audio_end == true) 
			return TCC_CDK_WRAPPER_END_OF_STREAM; 

		packet_num = AV_PACKET_AUDIO;
	}
	else if (aType == STREAM_TYPE_TEXT)
	{
		if(i_ctype == CONTAINER_TYPE_MKV || i_ctype == CONTAINER_TYPE_TS)
			return TCC_CDK_WRAPPER_END_OF_STREAM;
			
		if(audio_end == true && video_end == true) 
			return TCC_CDK_WRAPPER_END_OF_STREAM;

		packet_num = AV_PACKET_SUBTITLE;
	}
	else
	{
		return TCC_CDK_WRAPPER_INVALID_STREAM_ID;
	}

	if (aSize == 0)
	{
		return TCC_CDK_WRAPPER_INSUFFICIENT_BUFFER_SIZE;
	}

	//------------------------------------------------
	// Container Get Stream
	//------------------------------------------------
    if( CDK_GETSTREAM_SEQUENTIAL_MODE & iGetStreamMode ||
        DEMUXER_THREAD_ENABLED_MODE & iGetStreamMode ||
        DEMUXER_LOOPER_ENABLED_MODE & iGetStreamMode )
	{
		ssize_t			index;
		StreamQueue*	pStreamQueue;
		cdk_core_t*		pCdk = &(iCdk.m_sCore);
		bool			is_sync = false;

		uint8_t*		pby_out_buff = NULL; // re-name local variable for readability. pTempBuff -> pby_out_buff

		index = iDemuxStreamQueues.indexOfKey(packet_num);
		if(index < 0) {
			PrintError("[TCC_CDK_Wrapper]:[%5d] Unknown Packet Type = [%d]", __LINE__, packet_num);
			return TCC_CDK_WRAPPER_SELECT_STREAM_ERROR;
		}

		pStreamQueue = iDemuxStreamQueues.valueAt(index);

        if ((DEMUXER_THREAD_ENABLED_MODE & iGetStreamMode) && aExtra != SKIP_OPERATION_RESUME)
        {
            if(mDemuxerThread->GetState() != TCCDemuxerThread::STATE_RUNNING)
                mDemuxerThread->SetState(TCCDemuxerThread::STATE_RUNNING);

            if(pStreamQueue->GetState() != StreamQueue::STOPPED) {
                if(packet_num == AV_PACKET_SUBTITLE)
                    SendReqMsgToFeedDataIfUnderruned(StreamQueue::THRESHOLD_TYPE_CLIP_COUNT, index, packet_num, pStreamQueue->GetMinBufferingThreshold(), TCCDemuxerThread::MSG_SYNC_ON);
                else
                    SendReqMsgToFeedDataIfUnderruned(StreamQueue::THRESHOLD_TYPE_CLIP_TIME, index);
            }
        }

        if (pStreamQueue->GetLength() == 0)
        {
            if (aExtra != SKIP_OPERATION_RESUME) {
                if (DEMUXER_THREAD_ENABLED_MODE & iGetStreamMode)
                {
                    PrintError("[GetStreamData] StreamQueue(packet_num:%d) is empty - Q state:%d", 
                            packet_num, mDemuxerThread->GetState()); 
                    return TCC_CDK_WRAPPER_GET_FRAME_NEXT_TIME;
                }

                if (DEMUXER_LOOPER_ENABLED_MODE & iGetStreamMode)
                {
                    if (iPacketNumUnderrun == AV_PACKET_NONE) 
                    {
                        iPacketNumUnderrun = iDemuxStreamQueues.keyAt(index);
                    }

                    if (mCDKDemuxHandler->isAlive()) {
						int32_t vlen = -1, alen = -1, slen = -1;
						int queidx;
						if( (queidx = iDemuxStreamQueues.indexOfKey(AV_PACKET_VIDEO)) >= 0 )
							vlen = iDemuxStreamQueues.valueAt(queidx)->GetLength();
						if( (queidx = iDemuxStreamQueues.indexOfKey(AV_PACKET_AUDIO)) >= 0 )
							alen = iDemuxStreamQueues.valueAt(queidx)->GetLength();
						if( (queidx = iDemuxStreamQueues.indexOfKey(AV_PACKET_SUBTITLE)) >= 0 )
							slen = iDemuxStreamQueues.valueAt(queidx)->GetLength();
                        ALOGW("streamQ is empty !! (V:%d, A:%d, S:%d)", vlen, alen, slen); 
                        return TCC_CDK_WRAPPER_GET_FRAME_NEXT_TIME;
                    } 
                    return TCC_CDK_WRAPPER_GET_FRAME_CANCELLED;
                }
            }

			if( (CDK_GETSTREAM_SEQUENTIAL_MODE & iGetStreamMode) == 0 )
				return TCC_CDK_WRAPPER_GET_FRAME_NEXT_TIME;

			#ifdef USE_HDCP_DECRYPTION
			if( GetDecryptionNeed() )
			{
				secure_info_t secure_info;
				ret = FillStreamQueues(StreamQueue::THRESHOLD_TYPE_NONE, aSize, aTimeStamp, packet_num, pStreamBuffer, &secure_info);
				if(ret < 0)
				{
					CDMXErrorProcess(ret, packet_num);
					return ret;
				}

				if( secure_info.ulSecureType == CDMX_SECURE_TYPE_HDCP ) {
					ret = Stream_Decryption_process( pStreamBuffer, 
													 arBuffer, 
													 aSize, 
													 secure_info.enSecureInfo.stHdcpInfo.ulStreamCounter, 
													 secure_info.enSecureInfo.stHdcpInfo.ullInputCounter );
					if( ret < 0 ) {
						ALOGE("[TCC_CDK_Wrapper] Failied to decrypt on SecureOS");
						return TCC_CDK_WRAPPER_DECRYPTION_FAILED;
					}
				}
				else {
					ret = Stream_Copy_process(pStreamBuffer, arBuffer, aSize);
					if( ret < 0 ) {
						ALOGE("[TCC_CDK_Wrapper] Failied to copy on SecureOS");
						return TCC_CDK_WRAPPER_SECURE_COPY_FAILED;
					}
				}
			}
			else
			#endif
			{
				ret = FillStreamQueues(StreamQueue::THRESHOLD_TYPE_NONE, aSize, aTimeStamp, packet_num, arBuffer);
				if (ret < 0)
				{
					CDMXErrorProcess(ret, packet_num);
					return ret;
				}
			}
        }
        else
        {
            if (iPacketNumUnderrun != AV_PACKET_NONE 
                && iPacketNumUnderrun != iDemuxStreamQueues.keyAt(index)) 
            {
                return TCC_CDK_WRAPPER_GET_FRAME_NEXT_TIME;
            }
            iPacketNumUnderrun = AV_PACKET_NONE;

#ifdef MERGE_FRAME_AVG
            // for defragmentation
            if ((m_lVideoCodecType >= 0) && (packet_num == AV_PACKET_VIDEO) 
                    && ((CONTAINER_TYPE_TS == i_ctype) || (CONTAINER_TYPE_MPG == i_ctype)) && !bIsWFDPlayMode)
            {
                ret = this->DefragmentFrame( pby_out_buff, pStreamQueue, arBuffer, aSize, aTimeStamp, aExtra );
                if(ret)
                    return ret;												
            } 
            else // normal
#endif//#ifdef MERGE_FRAME_AVG
            {
				secure_info_t secure_info;

                pby_out_buff = pStreamQueue->GetOneStream(&aSize, &aTimeStamp, &secure_info, &iPacketDisCont, &ret);
                if(ret < 0)
                {
                    CDMXErrorProcess(ret, packet_num);
                    if (aExtra != SKIP_OPERATION_RESUME)
                    {
                        if (DEMUXER_THREAD_ENABLED_MODE & iGetStreamMode) 
                        {
                            if (ret == TCC_CDK_WRAPPER_END_OF_FILE) 
                            {
                                mDemuxerThread->SetState(TCCDemuxerThread::STATE_FINISHED);
                            }
                        } 
                        else if (DEMUXER_LOOPER_ENABLED_MODE & iGetStreamMode) 
                        {
                            mCDKDemuxHandler->stop();
                        }
                    }
                    return ret;
                }

                if (pby_out_buff != NULL)
                {
				#ifdef USE_HDCP_DECRYPTION
					if( GetDecryptionNeed() )
					{
						if ( secure_info.ulSecureType == CDMX_SECURE_TYPE_HDCP ) {
							ret = Stream_Decryption_process( pby_out_buff, 
															 arBuffer, 
															 aSize, 
															 secure_info.enSecureInfo.stHdcpInfo.ulStreamCounter, 
															 secure_info.enSecureInfo.stHdcpInfo.ullInputCounter );
							if( ret < 0 ) {
								ALOGE("[TCC_CDK_Wrapper] Failied to decrypt on SecureOS");
								return TCC_CDK_WRAPPER_DECRYPTION_FAILED;
							}
						}
						else {
							ret = Stream_Copy_process(pby_out_buff, arBuffer, aSize);
							if( ret < 0 ) {
								ALOGE("[TCC_CDK_Wrapper] Failied to copy on SecureOS");
								return TCC_CDK_WRAPPER_SECURE_COPY_FAILED;
							}
						}
					}
					else
				#endif
					{
						memcpy((void *)arBuffer, (const void *)pby_out_buff, aSize);
					}
                    pStreamQueue->Pop();
#ifdef DEBUG
                    //PrintHexData(packet_num, 1, arBuffer);
#endif
                }
                else
                {
                    PrintError("[TCC_CDK_Wrapper] Fail to get a %s stream", (packet_num == AV_PACKET_VIDEO) ? "VIDEO" : "AUDIO");
                }

                if(aSize != 0)
                {
                    if(packet_num == AV_PACKET_VIDEO && !bSentFirstVideoFrame)
                    {
                        bSentFirstVideoFrame = true;
                    }
                    // Escape the loop
                    return TCC_CDK_WRAPPER_OK;
                }
            }// else
        }
	}
	else
	{
		bool is_sync = false;
		bool is_defragmented = false;
		cdk_core_t* pCdk = &(iCdk.m_sCore);

		CDK_ITEM* CDK = static_cast<CDK_ITEM*>(pItem);
		memset(&CDK->gsCDmxOutput, 0, sizeof(cdmx_output_t));
		memset(&CDK->gsCDmxInput, 0, sizeof(cdmx_input_t));

		LOGMSG("buffer %x, size %d, type %d", arBuffer, aSize, packet_num);
		int32_t iCorruptedFrameCount = 0;
		while(1)
		{
			// get frame
			if ( (packet_num == AV_PACKET_AUDIO && i_ctype == CONTAINER_TYPE_ASF) || i_ctype == CONTAINER_TYPE_AUDIO)
			{
				CDK->gsCDmxInput.m_pPacketBuff = pStreamBuffer;
				CDK->gsCDmxInput.m_iPacketBuffSize = AUDIO_BUFFER_SIZE;
			}
			else
			{
				CDK->gsCDmxInput.m_pPacketBuff = pStreamBuffer;
				CDK->gsCDmxInput.m_iPacketBuffSize = aSize;
			}
			CDK->gsCDmxInput.m_iPacketType = packet_num;

		#ifdef MERGE_FRAME_AVG
            if ( (m_lVideoCodecType >= 0) && (packet_num == AV_PACKET_VIDEO) && ((CONTAINER_TYPE_TS == i_ctype) || (CONTAINER_TYPE_MPG== i_ctype)) && !bIsWFDPlayMode)
			{
				is_defragmented = true;
				CDK->gsCDmxInput.m_pPacketBuff = m_MergeFrame_Temp;
				CDK->gsCDmxInput.m_iPacketBuffSize = aSize;
				ret = this->DefragmentFrame_Selective( m_MergeFrame_Temp, CDK, arBuffer, aSize, aTimeStamp, aExtra );
			}
			else
		#endif
			{
				ret = m_pfnCdmx( CDMX_GETSTREAM, CDMX_HANDLE(m_pvCdmxInst), &CDK->gsCDmxInput, &CDK->gsCDmxOutput );
			}
			if( ret < 0 )
			{
				if( ret == ERR_END_OF_AUDIO_FILE )
				{
					PrintError("[CDK_WRAPPER] end of audio file");
					audio_end = true;
					return TCC_CDK_WRAPPER_END_OF_STREAM;
				}
				else if( ret == ERR_END_OF_VIDEO_FILE )
				{
					PrintError("[CDK_WRAPPER] end of video file");
					video_end = true;
					return TCC_CDK_WRAPPER_END_OF_STREAM;
				}
				else if( ret == ERR_END_OF_FILE )
				{
					PrintError("[CDK_WRAPPER] end of file");
					return TCC_CDK_WRAPPER_END_OF_FILE;
				}
				else if( ret == -11 ) // NAL size is not correct.
				{
					// 20120823 by kris park
					ALOGE("Nalsize error detected. So as to skip this, seek to 100ms after current pts. mLastVideoPTS(%d) mLastAudioPTS(%d)",mLastVideoPTS,mLastAudioPTS);
					if(guiLastpts < ((mLastVideoPTS<mLastAudioPTS)?mLastAudioPTS:mLastVideoPTS))
						guiLastpts = (mLastVideoPTS<mLastAudioPTS)?mLastAudioPTS:mLastVideoPTS;
					guiLastpts+=100;
					ret = MediaSeek(guiLastpts, SKIP_OPERATION_RESUME);
					if (ret < 0)
					{
						ALOGE("%s <%d> guiLastpts(%d)", __func__, __LINE__, guiLastpts); 
						video_end = audio_end = true;
						return TCC_CDK_WRAPPER_END_OF_FILE;
					}
					if((guiLastpts>(mLastVideoPTS+500))||(guiLastpts>(mLastAudioPTS+500)))
					{
						video_end = audio_end = true;
						ALOGE("Too many Nalsize error occurred. EOF Processed");
						return TCC_CDK_WRAPPER_END_OF_FILE;
					}


					continue;
				}
				else if( ret == -12 ) // comes from ASF parser. it means that this packet is corrupted.
				{
					if (++iCorruptedFrameCount == MAX_CORRUPTED_FRAME_COUNT)
					{
						PrintError("[CDK_WRAPPER] Can't parse this file due to corrupted packet: %d", ret);
						return TCC_CDK_WRAPPER_DEMUXER_ERROR;
					}
					continue;
				}
				else
				{
					PrintError("[CDK_WRAPPER] Demuxer Error: %d", ret);
					return TCC_CDK_WRAPPER_DEMUXER_ERROR;
				}

				if( audio_end && video_end )
					return TCC_CDK_WRAPPER_END_OF_FILE;
			}
#ifdef INTERNAL_SUBTITLE_INCLUDE
			else if(CDK->gsCDmxOutput.m_iPacketType == AV_PACKET_SUBTITLE && (CONTAINER_TYPE_MKV == i_ctype))
			{
				if(GetDetectIntSubtitle() == true)
				{
					m_InterSubtitle.SetSubtitle((SUBTITLE_TYPE)CDK->gsCDmxOutput.m_iSubtitleType, CDK->gsCDmxOutput.m_iTimeStamp,
												 CDK->gsCDmxOutput.m_iEndTimeStamp, CDK->gsCDmxOutput.m_iPacketSize, CDK->gsCDmxOutput.m_pPacketData);
					PrintError("%s SetSubtitle event(%d), sub num(%d)", __func__, GetSentSubtitleEvent(), m_InterSubtitle.GetSubtitleNum());
					PrintError("SubtitleType[%d] TimeStamp[%d]", CDK->gsCDmxOutput.m_iSubtitleType, CDK->gsCDmxOutput.m_iTimeStamp);
				}
			}
#endif
			else
			{
				if( CDK->gsCDmxOutput.m_iPacketType == AV_PACKET_VIDEO)
					mLastVideoPTS = CDK->gsCDmxOutput.m_iTimeStamp;
				else if( CDK->gsCDmxOutput.m_iPacketType == AV_PACKET_AUDIO)
					mLastAudioPTS = CDK->gsCDmxOutput.m_iTimeStamp;

#ifdef MERGE_FRAME_AVG
				if(aSize && !bIsWFDPlayMode)
					break;
#endif
				if( CDK->gsCDmxOutput.m_iPacketSize )
				{
					break;
				}
			}
		}// the end of while(1) container

		if( is_sync == true )
			iParam = TCC_MEDIAEVENT_SYNC_FRAME;

		if( packet_num == AV_PACKET_VIDEO )
		{
			if(!bSentFirstVideoFrame) 
				bSentFirstVideoFrame = true;

			if( is_defragmented == false )
			{
				bool copy_done = false;

				#ifdef DIVX_DRM5
				if(CDK->gsCDmxOutput.m_divxdrm_info.m_usIsThisFrameEncrypted) {
					uint8_t *p_drminfo = (uint8_t*)&CDK->gsCDmxOutput.m_divxdrm_info;
					memcpy(arBuffer, p_drminfo+2, 10);
					memcpy(arBuffer+TCC_DIVXDRM_RESERVED_SIZE, CDK->gsCDmxOutput.m_pPacketData, CDK->gsCDmxOutput.m_iPacketSize);
					aSize = CDK->gsCDmxOutput.m_iPacketSize + TCC_DIVXDRM_RESERVED_SIZE;
					copy_done = true;
				}
				#endif
	
				#ifdef USE_HDCP_DECRYPTION
				if( GetDecryptionNeed() )
				{
					if( CDK->gsCDmxOutput.m_ulSecureType == CDMX_SECURE_TYPE_HDCP ) {
						uint32_t packet_size = CDK->gsCDmxOutput.m_iPacketSize;
						ret = Stream_Decryption_process( CDK->gsCDmxOutput.m_pPacketData, 
														 arBuffer, 
														 packet_size, 
														 CDK->gsCDmxOutput.m_enSecureInfo.stHdcpInfo.ulStreamCounter, 
														 CDK->gsCDmxOutput.m_enSecureInfo.stHdcpInfo.ullInputCounter );
						if( ret < 0 ) {
							ALOGE("[TCC_CDK_Wrapper] Failied to decrypt on SecureOS");
							return TCC_CDK_WRAPPER_DECRYPTION_FAILED;
						}
						aSize = CDK->gsCDmxOutput.m_iPacketSize;
						copy_done = true;
					}
					else {
						uint32_t data_len = (uint32_t)CDK->gsCDmxOutput.m_iPacketSize;
						if( aSize < data_len )
							data_len = aSize;
						ret = Stream_Copy_process(CDK->gsCDmxOutput.m_pPacketData, arBuffer, data_len);
						if( ret < 0 ) {
							ALOGE("[TCC_CDK_Wrapper] Failied to copy on SecureOS");
							return TCC_CDK_WRAPPER_SECURE_COPY_FAILED;
						}
						aSize = data_len;
						copy_done = true;
					}
				}
				#endif
	
				if( copy_done == false ) {
					int32_t data_len = CDK->gsCDmxOutput.m_iPacketSize;
					if( aSize < data_len )
						data_len = aSize;
					memcpy(arBuffer, CDK->gsCDmxOutput.m_pPacketData, data_len);
					aSize = data_len;
				}

				aTimeStamp = CDK->gsCDmxOutput.m_iTimeStamp;
			}

			aExtra = CDK->gsCDmxOutput.m_uiUseCodecSpecific;
		}
		else if( packet_num == AV_PACKET_AUDIO )
		{
			#ifdef USE_HDCP_DECRYPTION
			if( GetDecryptionNeed() )
			{
				if( CDK->gsCDmxOutput.m_ulSecureType == CDMX_SECURE_TYPE_HDCP ) {
					uint32_t packet_size = CDK->gsCDmxOutput.m_iPacketSize;
					ret = Stream_Decryption_process( CDK->gsCDmxOutput.m_pPacketData, 
													 arBuffer, 
													 packet_size, 
													 CDK->gsCDmxOutput.m_enSecureInfo.stHdcpInfo.ulStreamCounter, 
						    						 CDK->gsCDmxOutput.m_enSecureInfo.stHdcpInfo.ullInputCounter );
					if( ret < 0 ) {
						ALOGE("[TCC_CDK_Wrapper] Failied to decrypt on SecureOS");
						return TCC_CDK_WRAPPER_DECRYPTION_FAILED;
					}
					aSize = CDK->gsCDmxOutput.m_iPacketSize;
				}
				else {
					uint32_t data_len = (uint32_t)CDK->gsCDmxOutput.m_iPacketSize;
					if( aSize < data_len )
						data_len = aSize;
					ret = Stream_Copy_process(CDK->gsCDmxOutput.m_pPacketData, arBuffer, data_len);
					if( ret < 0 ) {
						ALOGE("[TCC_CDK_Wrapper] Failied to copy on SecureOS");
						return TCC_CDK_WRAPPER_SECURE_COPY_FAILED;
					}
					aSize = data_len;
				}
			}
			else
			#endif
			{
				int32_t data_len = CDK->gsCDmxOutput.m_iPacketSize;
				if( aSize < data_len )
					data_len = aSize;
				memcpy(arBuffer, CDK->gsCDmxOutput.m_pPacketData, data_len);
				aSize = data_len;
			}

			aTimeStamp = CDK->gsCDmxOutput.m_iTimeStamp;
			aExtra = CDK->gsCDmxOutput.m_uiUseCodecSpecific;
		}
#ifdef INTERNAL_SUBTITLE_INCLUDE
		else if( packet_num == AV_PACKET_SUBTITLE && (i_ctype == CONTAINER_TYPE_MP4))
		{
			int32_t data_len = CDK->gsCDmxOutput.m_iPacketSize;
			if( aSize < data_len )
				data_len = aSize;

			if(GetDetectIntSubtitle() == true)
			{
				m_InterSubtitle.SetSubtitle((SUBTITLE_TYPE)CDK->gsCDmxOutput.m_iSubtitleType, CDK->gsCDmxOutput.m_iTimeStamp,
											CDK->gsCDmxOutput.m_iEndTimeStamp, CDK->gsCDmxOutput.m_iPacketSize-2, CDK->gsCDmxOutput.m_pPacketData+2);
				PrintError("%s SetSubtitle event(%d), sub num(%d)", __func__, GetSentSubtitleEvent(), m_InterSubtitle.GetSubtitleNum());
				PrintError("SubtitleType[%d] TimeStamp[%d]", CDK->gsCDmxOutput.m_iSubtitleType, CDK->gsCDmxOutput.m_iTimeStamp);
			}

			memcpy(arBuffer, CDK->gsCDmxOutput.m_pPacketData, data_len);
			aSize = data_len;
			aTimeStamp = CDK->gsCDmxOutput.m_iTimeStamp;
			aExtra = CDK->gsCDmxOutput.m_uiUseCodecSpecific;
		}
#endif
	}
	
	return TCC_CDK_WRAPPER_OK;
}

void TCC_CDK_Wrapper::ClearStreamBuffer(int32_t packet_num)	
{
	if (packet_num == STREAM_TYPE_ALL) 
	{
		StreamQueue* pStreamQueue;
	    const int numQueues = iDemuxStreamQueues.size();
	    for(int i = 0; i < numQueues; i++)
		{
			pStreamQueue = iDemuxStreamQueues.valueAt(i);
			if (pStreamQueue) {
		        pStreamQueue->InitBufferSimple();
				pStreamQueue->SetState(StreamQueue::START);
		    }
		}
	}
	else
    {
		int queidx = iDemuxStreamQueues.indexOfKey(packet_num);
		if( queidx >= 0 )
			iDemuxStreamQueues.valueAt(queidx)->InitBufferSimple();
	}	
}

void TCC_CDK_Wrapper::SkipAudioStream(void)
{
    int index = iDemuxStreamQueues.indexOfKey(AV_PACKET_AUDIO);
    iDemuxStreamQueues.valueAt(index)->SkipAudioStream();
}

uint32_t TCC_CDK_Wrapper::GetVideoBufferedTime(void)
{
    int index = iDemuxStreamQueues.indexOfKey(AV_PACKET_VIDEO);

    return iDemuxStreamQueues.valueAt(index)->GetBufferedTime();
}

int32_t TCC_CDK_Wrapper::MediaSeekCore(uint32_t targetTS)
{
	CDK_ITEM* CDK = static_cast<CDK_ITEM*>(pItem);
	CDK->gsCdmxSeek.m_uiSeekMode = SEEK_ABSOLUTE;
	CDK->gsCdmxSeek.m_iSeekTime = (int)targetTS;
	long ret = 0;

	PrintInfo("[CDK_WRAPPER] Absolute Seek (DstTime: %d)", CDK->gsCdmxSeek.m_iSeekTime);

	mLastVideoPTS = -1;
	mLastAudioPTS = -1;

	ret = m_pfnCdmx( CDMX_SEEK, CDMX_HANDLE(m_pvCdmxInst), &CDK->gsCdmxSeek, &CDK->gsCDmxOutput );
	return ret;
}

int32_t TCC_CDK_Wrapper::MediaSeek(uint32_t targetTS, uint32_t aOperationMarker)
{
	PrintInfo("TCC_CDK_Wrapper::MediaSeek In - iGetStreamMode(%d), aOperationMarker(%08x)", iGetStreamMode, aOperationMarker);
#ifdef MERGE_FRAME_AVG
	m_bSeekFlag = 1;
#endif
	// If iSeekable flag is false, do not seek
	if (iFileInfo.iSeekable == false && targetTS != 0)
	{
		PrintError("MediaSeek() : iFileInfo.iSeekable(%d), targetTS(%d)", iFileInfo.iSeekable, targetTS);
		return TCC_CDK_WRAPPER_SEEK_ERROR;
	}

#ifdef PGS_CAPTION_SUPPORT_INCLUDE
	if(i_ctype == CONTAINER_TYPE_TS && iFileInfo.iNumSubtitleStreams > 0 && GetSentPGSEvent() == true)
	{
		m_InterSubtitle.gs_decoder_reset(); //Reset PGS Decoder
		m_InterSubtitle.ClearPGSQueue();
	}
#endif

    if (!(aOperationMarker & SKIP_OPERATION_PAUSE))
    {
        if (DEMUXER_THREAD_ENABLED_MODE & iGetStreamMode) {
            mDemuxerThread->SetState(TCCDemuxerThread::STATE_PAUSED);

            PrintInfo("TCC_CDK_Wrapper::MediaSeek - Request 'Pause' to parser thread...");
            mDemuxerThread->Pause(TCCDemuxerThread::MSG_SYNC_ON);
        } else if (DEMUXER_LOOPER_ENABLED_MODE & iGetStreamMode) {
            mCDKDemuxHandler->pause();
        }
    }

	// If seek direction is backward, stream end flag must be resetted to zero.
	// but for all cases, the stream flag reset is valid 
	// because seek function is always check end of stream.
	if ((iFileInfo.iNumAudStreams > 0) && (iAudInfo[iAudID].iID != -1))
	{
		audio_end = false;
	}
	if ((iFileInfo.iNumVidStreams > 0) && (iVidInfo[iVidID].iID != -1))
	{
		video_end = false;
	}

	//------------------------------------------------
	// Container Seek
	//------------------------------------------------
	int32_t ret;
	cdk_core_t* pCdk = &(iCdk.m_sCore);

	if (((DEMUXER_THREAD_ENABLED_MODE & iGetStreamMode) || (DEMUXER_LOOPER_ENABLED_MODE & iGetStreamMode)) 
		&& (!(aOperationMarker & SKIP_OPERATION_PAUSE)) &&
		(pCdk->m_iPlayMode & (PLAYMODE_SKIMMING_WITH_SEEK | PLAYMODE_SKIMMING_WITH_DISCARD_AUDIO)) == 0 )
	{
		if(!(aOperationMarker & SKIP_OPERATION_SEEK))
		{
			if (DEMUXER_THREAD_ENABLED_MODE & iGetStreamMode) {
				ret = mDemuxerThread->SeekTo(targetTS);
				if (ret < 0) {
                    LOGMSG("[MediaSeek:%d] SeekTo(%d) failed: %d", __LINE__, targetTS, ret);
					return TCC_CDK_WRAPPER_SEEK_ERROR;
				}
			} else if (DEMUXER_LOOPER_ENABLED_MODE & iGetStreamMode) {
				ret = mCDKDemuxHandler->seekTo(targetTS);
                if (ret < 0) {
                    LOGMSG("[MediaSeek:%d] SeekTo(%d) failed: %d", __LINE__, targetTS, ret);
                    mCDKDemuxHandler->resume();
					return TCC_CDK_WRAPPER_SEEK_ERROR;
				}
			}
		}
	}
	else
	{
		ret = MediaSeekCore(targetTS);
		PrintInfo("[CDK_WRAPPER] Seek result %d", ret);
		if(ret < 0) {
			LOGMSG("[MediaSeek:%d] MediaSeekCore(%d) failed: %d", __LINE__, targetTS, ret);
			return TCC_CDK_WRAPPER_SEEK_ERROR;
		}
	}

	if( CDK_GETSTREAM_SEQUENTIAL_MODE & iGetStreamMode  // sequential mode
		|| DEMUXER_THREAD_ENABLED_MODE & iGetStreamMode || DEMUXER_LOOPER_ENABLED_MODE & iGetStreamMode) // selective mode & thread enabled
	{
		if (!bSentFirstVideoFrame) 
		{
			ResetFirstFrameFlag();
		}

		if (!(aOperationMarker & SKIP_OPERATION_SEEK)) 
		{
		    if (!(DEMUXER_LOOPER_ENABLED_MODE & iGetStreamMode))
			{
                ClearStreamBuffer(STREAM_TYPE_ALL);
			}
		}

		if((!(aOperationMarker & SKIP_OPERATION_RESUME)) && 
		   (pCdk->m_iPlayMode & (PLAYMODE_SKIMMING_WITH_SEEK | PLAYMODE_SKIMMING_WITH_DISCARD_AUDIO)) == 0 )
		{
			// Toggle demuxer thread to pre-load frames.
			PrintInfo("TCC_CDK_Wrapper::MediaSeek - Request 'Resume' to parser thread...");

			if (DEMUXER_THREAD_ENABLED_MODE & iGetStreamMode) {

				mDemuxerThread->SetState(TCCDemuxerThread::STATE_RUNNING);

				uint32_t uiDemuxerProcCmd; 
				int32_t iParam1, iParam2, iCmdOption;

				if (!bSentFirstVideoFrame) 
				{
					uiDemuxerProcCmd = StreamQueue::THRESHOLD_TYPE_TIME;
					iParam1 = 500; 
					iParam2 = 500;
					iCmdOption = TCCDemuxerThread::MSG_SYNC_ON;
				}
				else
				{
					uiDemuxerProcCmd = StreamQueue::THRESHOLD_TYPE_CLIP_COUNT;
					iParam1 = AV_PACKET_VIDEO; 
					iParam2 = 40; 
					iCmdOption = TCCDemuxerThread::MSG_SYNC_ON;
				}

				mDemuxerThread->Resume(uiDemuxerProcCmd, iParam1, iParam2, iCmdOption); 
			} else if (DEMUXER_LOOPER_ENABLED_MODE & iGetStreamMode) {
                mCDKDemuxHandler->resume();
			}
		} else {
			ALOGE("TCC_CDK_Wrapper::MediaSeek - Fail to request 'Resume' to parser thread...");
		}
	}
	PrintInfo("TCC_CDK_Wrapper::MediaSeek out");
	return TCC_CDK_WRAPPER_OK;
}

void TCC_CDK_Wrapper::ResetFirstFrameFlag()
{
	switch (i_ctype)
	{
		case CONTAINER_TYPE_MKV:
			break;
		case CONTAINER_TYPE_MP4:
			break;
		case CONTAINER_TYPE_AVI:
			break;
        case CONTAINER_TYPE_ASF:
            #ifdef INCLUDE_ASF_DMX
            #endif
			break;
        case CONTAINER_TYPE_EXT1:
            #ifdef INCLUDE_EXT1_DMX
            #endif
			break;
		case CONTAINER_TYPE_FLV:
			break;
		default:
			break;
	}
}

void TCC_CDK_Wrapper::Reset()
{
	if ((iFileInfo.iNumAudStreams > 0) && (iAudInfo[iAudID].iID != -1))
	{
		audio_end = false;
	}
	if ((iFileInfo.iNumVidStreams > 0) && (iVidInfo[iVidID].iID != -1))
	{
		video_end = false;
	}

	bSentFirstVideoFrame = false;
}

void TCC_CDK_Wrapper::ReadMetadata()
{
	PrintInfo("[CDK_WRAPPER] Read meta data in" );

#if 0
	if ( (i_ctype == CONTAINER_TYPE_AUDIO || (i_ctype == CONTAINER_TYPE_ASF && iFileInfo.iNumVidStreams == 0))
			&& iFileInfo.iNumAudStreams > 0 && mSourceScheme == FILE_SOURCE_SCHEME)
	{
		if (iAudInfo[iAudID].i_atype == AUDIO_ID_APE)
		{
			ape_taginfo_t* tag_info = &(reinterpret_cast<ape_dmx_info_t*>((static_cast<gsAUDIO*>(pCDMX_Global))->gsAudioDmxHandle))->m_st_taginfo;
			char16_t metadata[256];	

			utf8_to_utf16((const uint8_t*)tag_info->m_cArtist, strlen(tag_info->m_cArtist), metadata);
			sMetadataArtist.setTo((const char16_t*)metadata);

			utf8_to_utf16((const uint8_t*)tag_info->m_cAlbum, strlen(tag_info->m_cAlbum), metadata);
			sMetadataAlbum.setTo((const char16_t*)metadata);

			utf8_to_utf16((const uint8_t*)tag_info->m_cTitle, strlen(tag_info->m_cTitle), metadata);
			sMetadataTitle.setTo((const char16_t*)metadata);

			utf8_to_utf16((const uint8_t*)tag_info->m_cTrack, strlen(tag_info->m_cTrack), metadata);
			sMetadataTrack.setTo((const char16_t*)metadata);

			utf8_to_utf16((const uint8_t*)tag_info->m_cGenre, strlen(tag_info->m_cGenre), metadata);
			sMetadataGenre.setTo((const char16_t*)metadata);

			utf8_to_utf16((const uint8_t*)tag_info->m_cYear, strlen(tag_info->m_cYear), metadata);
			sMetadataYear.setTo((const char16_t*)metadata);

			utf8_to_utf16((const uint8_t*)tag_info->m_cComment, strlen(tag_info->m_cComment), metadata);
			sMetadataComment.setTo((const char16_t*)metadata);
		}
		else if (iAudInfo[iAudID].i_atype == AUDIO_ID_FLAC)
		{
			int32_t error;
			TCCMetadataParser* pMetadataParser;

			pMetadataParser = new TCCMetadataParser();
			error = pMetadataParser->ParseMetadata(AUDIO_ID_FLAC, iFilePtr);

			if (error != -1)
			{
				TCCMetadata* metadata;
				metadata = pMetadataParser->GetMetadata();

				sMetadataArtist = metadata->sArtist;
				sMetadataAlbum = metadata->sAlbum;
				sMetadataTitle = metadata->sTitle;
				sMetadataTrack = metadata->sTrack;
				sMetadataGenre = metadata->sGenre;
				sMetadataComment = metadata->sComment;
				sMetadataYear = metadata->sYear;
			}

			delete(pMetadataParser);
		}
		else if (iAudInfo[iAudID].i_atype == AUDIO_ID_WMA)
		{
			int32_t error;
			TCCMetadataParser* pMetadataParser;
			
			pMetadataParser = new TCCMetadataParser();
			error = pMetadataParser->ParseMetadata(AUDIO_ID_WMA, iFilePtr);

			if (error != -1)
			{
				TCCMetadata* metadata;
				metadata = pMetadataParser->GetMetadata();

				sMetadataAlbum = metadata->sAlbum;
				sMetadataTitle = metadata->sTitle;
				sMetadataTrack = metadata->sTrack;
				sMetadataGenre = metadata->sGenre;
				sMetadataAuthor = metadata->sAuthor;
				sMetadataCopyright = metadata->sCopyright;
				sMetadataDescription = metadata->sDescription;
				sMetadataRating = metadata->sRating;
				sMetadataComposer = metadata->sComposer;
				sMetadataYear = metadata->sYear;
			}
			
			delete pMetadataParser;
		}
	}
	#endif
	
	PrintInfo("[CDK_WRAPPER] Read meta data out" );
}

String16* TCC_CDK_Wrapper::GetMetadataString(int32_t key)
{
	switch(key)
	{
		case TCC_CDK_WRAPPER_METADATA_ARTIST:
			return &sMetadataArtist;
		case TCC_CDK_WRAPPER_METADATA_ALBUM:
			return &sMetadataAlbum;
		case TCC_CDK_WRAPPER_METADATA_TITLE:
			return &sMetadataTitle;
		case TCC_CDK_WRAPPER_METADATA_TRACK:
			return &sMetadataTrack;
		case TCC_CDK_WRAPPER_METADATA_GENRE:
			return &sMetadataGenre;
		case TCC_CDK_WRAPPER_METADATA_YEAR:
			return &sMetadataYear;
		case TCC_CDK_WRAPPER_METADATA_COMMENT:
			return &sMetadataComment;
		case TCC_CDK_WRAPPER_METADATA_AUTHOR:
			return &sMetadataAuthor;
		case TCC_CDK_WRAPPER_METADATA_DESCRIPTION:
			return &sMetadataDescription;
		case TCC_CDK_WRAPPER_METADATA_RATING:
			return &sMetadataRating;
		case TCC_CDK_WRAPPER_METADATA_COMPOSER:
			return &sMetadataComposer;
		case TCC_CDK_WRAPPER_METADATA_COPYRIGHT:
			return &sMetadataCopyright;
		default: 
			return NULL;
	}
}

bool TCC_CDK_Wrapper::ExistMetadataValue(int32_t key)
{
	switch(key)
	{
		case TCC_CDK_WRAPPER_METADATA_ARTIST:
			if (sMetadataArtist.size() > 0)
				return true;
			else
				return false;
		case TCC_CDK_WRAPPER_METADATA_ALBUM:
			if (sMetadataAlbum.size() > 0)
				return true;
			else
				return false;
		case TCC_CDK_WRAPPER_METADATA_TITLE:
			if (sMetadataTitle.size() > 0)
				return true;
			else
				return false;
		case TCC_CDK_WRAPPER_METADATA_TRACK:
			if (sMetadataTrack.size() > 0)
				return true;
			else
				return false;
		case TCC_CDK_WRAPPER_METADATA_GENRE:
			if (sMetadataGenre.size() > 0)
				return true;
			else
				return false;
		case TCC_CDK_WRAPPER_METADATA_YEAR:
			if (sMetadataYear.size() > 0)
				return true;
			else
				return false;
		case TCC_CDK_WRAPPER_METADATA_COMMENT:
			if (sMetadataComment.size() > 0)
				return true;
			else
				return false;
		case TCC_CDK_WRAPPER_METADATA_AUTHOR:
			if (sMetadataAuthor.size() > 0)
				return true;
			else
				return false;
		case TCC_CDK_WRAPPER_METADATA_DESCRIPTION:
			if (sMetadataDescription.size() > 0)
				return true;
			else
				return false;
		case TCC_CDK_WRAPPER_METADATA_RATING:
			if (sMetadataRating.size() > 0)
				return true;
			else
				return false;
		case TCC_CDK_WRAPPER_METADATA_COMPOSER:
			if (sMetadataComposer.size() > 0)
				return true;
			else
				return false;
		case TCC_CDK_WRAPPER_METADATA_COPYRIGHT:
			if (sMetadataCopyright.size() > 0)
				return true;
			else
				return false;
		default: 
			return false;
	}
}

int32_t TCC_CDK_Wrapper::IsMP2File()
{
	CDK_ITEM* CDK = static_cast<CDK_ITEM*>(pItem);
	cdk_core_t* pCdk = &(iCdk.m_sCore);

	int32_t ret = 0;

	audio_dmx_init_t audioInit;
	
	//--------------------------------------------------
	// Set Callback Functions 
	//--------------------------------------------------
	ret = SetCallbackFunctions(NORMAL_FUNCTIONS);

	if (ret != TCC_CDK_WRAPPER_OK)
	{
		PrintError("[CDK_WRAPPER] [Err:%4d] fail setting callback functions", ret);
		return TCC_CDK_WRAPPER_ERROR;
	}

	audioInit.m_pOpenFileName = pCdk->m_pcOpenFileName;

	audioInit.m_sCallbackFunc.m_pMalloc			= pCallback->m_pfMalloc;
	audioInit.m_sCallbackFunc.m_pFree			= pCallback->m_pfFree;
	audioInit.m_sCallbackFunc.m_pNonCacheMalloc	= pCallback->m_pfNonCacheMalloc;
	audioInit.m_sCallbackFunc.m_pNonCacheFree	= pCallback->m_pfNonCacheFree;
	audioInit.m_sCallbackFunc.m_pMemcpy			= pCallback->m_pfMemcpy;
	audioInit.m_sCallbackFunc.m_pMemset			= pCallback->m_pfMemset;
	audioInit.m_sCallbackFunc.m_pRealloc		= pCallback->m_pfRealloc;
	audioInit.m_sCallbackFunc.m_pMemmove		= pCallback->m_pfMemmove;
	audioInit.m_sCallbackFunc.m_pFopen			= pCallback->m_pfFopen;
	audioInit.m_sCallbackFunc.m_pFread			= pCallback->m_pfFread;
	audioInit.m_sCallbackFunc.m_pFseek			= pCallback->m_pfFseek;
	audioInit.m_sCallbackFunc.m_pFtell			= pCallback->m_pfFtell;
	audioInit.m_sCallbackFunc.m_pFwrite			= pCallback->m_pfFwrite;
	audioInit.m_sCallbackFunc.m_pFclose			= pCallback->m_pfFclose;
	audioInit.m_sCallbackFunc.m_pStrncmp         = cdk_strncmp;

	audioInit.m_sCallbackFunc.m_pFseek64			= pCallback->m_pfFseek64;
	audioInit.m_sCallbackFunc.m_pFtell64			= pCallback->m_pfFtell64;

	void* fp_inp;
	fp_inp = pCallback->m_pfFopen(pCdk->m_pcOpenFileName, "rb");
	
	if( fp_inp == NULL )
	{
		PrintError("[CDMX:Err%d] cdmx_audio file open failed \n", -1 );
		return TCC_CDK_WRAPPER_ERROR;
	}

	unsigned char buffer[16];
	pCallback->m_pfFread( buffer, 10, 1, fp_inp );

	audioInit.m_uiId3TagOffset = 0;

	if( !cdk_strncmp((char*)buffer, "ID3", 3) )
	{
		audioInit.m_uiId3TagOffset = (buffer[6] << 21) | (buffer[7] << 14) | (buffer[8] <<  7) | (buffer[9] <<  0);
		audioInit.m_uiId3TagOffset += 10;
	}

	pCallback->m_pfFseek(fp_inp, audioInit.m_uiId3TagOffset, 0); // skip id3 tag

#ifndef TEST_CHECK_MPEG
	ret = is_mpeg_audio(&audioInit, fp_inp);
#else
	ret = cdmx_audio( CDMX_MPEG_AUDIO_CHECK, NULL, &audioInit, fp_inp );
#endif

	pCallback->m_pfFclose(fp_inp);

	if (ret == 2)
	{
		return TCC_CDK_WRAPPER_OK;
	}
	else
	{
		return TCC_CDK_WRAPPER_UNKNOWN_FILE_TYPE;
	}
}

int32_t TCC_CDK_Wrapper::IsAACFile()
{
	CDK_ITEM* CDK = static_cast<CDK_ITEM*>(pItem);
	cdk_core_t* pCdk = &(iCdk.m_sCore);

	int32_t ret;

	//--------------------------------------------------
	// Set Callback Functions 
	//--------------------------------------------------
	ret = SetCallbackFunctions(NORMAL_FUNCTIONS);

	if (ret != TCC_CDK_WRAPPER_OK)
	{
		PrintError("[CDK_WRAPPER] [Err:%4d] fail setting callback functions", ret);
		return TCC_CDK_WRAPPER_ERROR;
	}

	void* fp_inp;
	fp_inp = pCallback->m_pfFopen(pCdk->m_pcOpenFileName, "rb");
	
	if (fp_inp == NULL)
	{
		PrintError("[CDMX:Err%d] cdmx_audio file open failed \n", -1);
		return TCC_CDK_WRAPPER_ERROR;
	}

	// skip id3 tag
	unsigned char buffer[16];
	unsigned int id3tagOffset = 0;
	pCallback->m_pfFread(buffer, 10, 1, fp_inp);
	if (!cdk_strncmp((char*)buffer, "ID3", 3))
	{
		id3tagOffset = (buffer[6] << 21) | (buffer[7] << 14) | (buffer[8] <<  7) | (buffer[9] <<  0);
		id3tagOffset += 10;
	}

	pCallback->m_pfFseek(fp_inp, id3tagOffset, 0); // skip id3 tag

	// AAC check routine start
	int iRet = 0, iRead = 0;
	unsigned char *pucStream;
	int iBuffSize = 768*10;

	pucStream = (unsigned char *)pCallback->m_pfMalloc(iBuffSize);
	if (pucStream == NULL)
	{
		return TCC_CDK_WRAPPER_ERROR;
	}

	iRead = (int)pCallback->m_pfFread( pucStream, 1, iBuffSize, fp_inp );

	iRet = tcc_aac_dmx_check_adts(pucStream, iRead);

	pCallback->m_pfFree(pucStream);
	pCallback->m_pfFclose(fp_inp);

	if (iRet == 0)
	{
		PrintInfo(" Audio Demuxer is AAC-ADTS");
		return TCC_CDK_WRAPPER_OK;
	}
	else
	{
		PrintInfo(" Unknown Format");
		return TCC_CDK_WRAPPER_UNKNOWN_FILE_TYPE;
	}
}

// INTERNAL_SUBTITLE_INCLUDE
void TCC_CDK_Wrapper::SetSentSubtitleEvent(bool value)
{
	bSentSubtitleEvent = value;
}

bool TCC_CDK_Wrapper::GetSentSubtitleEvent(void)
{
	return bSentSubtitleEvent;
}

void TCC_CDK_Wrapper::SetDetectIntSubtitle(bool value)
{
	bDetectIntSubtitle = value;
}

bool TCC_CDK_Wrapper::GetDetectIntSubtitle(void)
{
	return bDetectIntSubtitle;
}

int32_t TCC_CDK_Wrapper::GetFirstSubtitle(void)
{
   if(get_inter_subtitle_type() == TCC_MEDIAEVENT_READY_TO_DISPLAY_TIMEDTEXT)
   {
		TCC_SUBTITLE_ELEMENT *pEle ;

		if( m_InterSubtitle.GetSubtitle( &pEle ) )
		{
			if( pEle )
			{
				m_InterSubtitle.PopSubtitle( pEle->uDataSize ) ;
			}
		}
		else
		{
			PrintInfo("wrapper subtitle empty(%d)", m_InterSubtitle.GetSubtitleNum() ) ;
			return 0 ;
	    	}
	    	return (int)pEle ; 
   }
   else if(get_inter_subtitle_type() == TCC_MEDIAEVENT_READY_TO_DISPLAY_PGS)
   {

		TCC_PGS_STREAM *pDecodedPGS ;

		pDecodedPGS = m_InterSubtitle.GetDecodedPGS() ;

		return (int)pDecodedPGS;
   
   }
   else
   {
       ALOGE("Can't get subtitle element!");
       return NULL;
   }
}

int32_t TCC_CDK_Wrapper::SetSubtitleSeek(int msec)
{
	return m_InterSubtitle.SetSubtitleSeek( msec ) ; 
}

void TCC_CDK_Wrapper::SetSentPGSEvent(bool value)
{
	bSentPGSEvent = value;
}

bool TCC_CDK_Wrapper::GetSentPGSEvent(void)
{
	return bSentPGSEvent;
}

int TCC_CDK_Wrapper::SetCurrentPlaybackPosition(int msec)
{
	return m_InterSubtitle.SetCurrentPlaybackPosition( msec ) ; 
}

void TCC_CDK_Wrapper::set_inter_subtitle_type(int32_t type) {
    mInterSubtitleType = type;
}

int32_t TCC_CDK_Wrapper::get_inter_subtitle_type(void) {
    return mInterSubtitleType;
}

void TCC_CDK_Wrapper::PushSubtitleData(void) {
	CDK_ITEM* CDK = static_cast<CDK_ITEM*>(pItem);

	if((AV_PACKET_SUBTITLE == CDK->gsCDmxOutput.m_iPacketType) && (iFileInfo.iNumSubtitleStreams > 0))
	{
#ifdef INTERNAL_SUBTITLE_INCLUDE
		if(GetDetectIntSubtitle() == true)
		{
			if(i_ctype == CONTAINER_TYPE_MKV)
			{
				m_InterSubtitle.SetSubtitle((SUBTITLE_TYPE)CDK->gsCDmxOutput.m_iSubtitleType, CDK->gsCDmxOutput.m_iTimeStamp, 
											CDK->gsCDmxOutput.m_iEndTimeStamp, CDK->gsCDmxOutput.m_iPacketSize, CDK->gsCDmxOutput.m_pPacketData);
			}
			else if(i_ctype == CONTAINER_TYPE_MP4)
			{
				m_InterSubtitle.SetSubtitle((SUBTITLE_TYPE)CDK->gsCDmxOutput.m_iSubtitleType, CDK->gsCDmxOutput.m_iTimeStamp,
											CDK->gsCDmxOutput.m_iEndTimeStamp, CDK->gsCDmxOutput.m_iPacketSize-2, CDK->gsCDmxOutput.m_pPacketData+2);
			}

			PrintInfo("%s SetSubtitle event(%d), sub num(%d)", __func__, GetSentSubtitleEvent(), m_InterSubtitle.GetSubtitleNum()) ;
			PrintInfo("SubtitleType[%d] TimeStamp[%d]", CDK->gsCDmxOutput.m_iSubtitleType, CDK->gsCDmxOutput.m_iTimeStamp);
		}
#endif
#ifdef PGS_CAPTION_SUPPORT_INCLUDE
		if((GetSentPGSEvent() == true) && (i_ctype == CONTAINER_TYPE_TS))
		{ 
			 m_InterSubtitle.gs_decode(CDK->gsCDmxOutput.m_iTimeStamp, CDK->gsCDmxOutput.m_iPacketSize,	CDK->gsCDmxOutput.m_pPacketData);
		}
#endif
	}
}

void TCC_CDK_Wrapper::ClearAudioBuffer()      
{
    Mutex::Autolock autoLock(mLock);

	if ((DEMUXER_THREAD_ENABLED_MODE & iGetStreamMode) && mDemuxerThread.get() )
	{
		mDemuxerThread->SetState(TCCDemuxerThread::STATE_PAUSED);

		int index;
		if( (index = iDemuxStreamQueues.indexOfKey(AV_PACKET_AUDIO)) >= 0 )
			iDemuxStreamQueues.valueAt(index)->InitBufferSimple();

		mDemuxerThread->SetState(TCCDemuxerThread::STATE_RUNNING);
	}
	else if ((DEMUXER_LOOPER_ENABLED_MODE & iGetStreamMode) && mCDKDemuxHandler.get())
	{
	    mCDKDemuxHandler->pause();

		int index;
		if( (index = iDemuxStreamQueues.indexOfKey(AV_PACKET_AUDIO)) >= 0 )
			iDemuxStreamQueues.valueAt(index)->InitBufferSimple();

        mCDKDemuxHandler->resume();
	}
}

void TCC_CDK_Wrapper::PrintHexData(unsigned int type, unsigned int process, unsigned char * p)
{
	PrintInfo("%s: %s - %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x " 
			           "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", 
					   (process == 1) ? "Dequeue" : "Enqueue", (type == 1) ? "Video" : "Audio",
					   p[ 0+16*0], p[ 1+16*0], p[ 2+16*0], p[ 3+16*0], p[ 4+16*0], p[ 5+16*0], p[ 6+16*0], p[ 7+16*0], 
					   p[ 8+16*0], p[ 9+16*0], p[10+16*0], p[11+16*0], p[12+16*0], p[13+16*0], p[14+16*0], p[15+16*0], 
					   p[ 0+16*1], p[ 1+16*1], p[ 2+16*1], p[ 3+16*1], p[ 4+16*1], p[ 5+16*1], p[ 6+16*1], p[ 7+16*1], 
					   p[ 8+16*1], p[ 9+16*1], p[10+16*1], p[11+16*1], p[12+16*1], p[13+16*1], p[14+16*1], p[15+16*1]);
}

int32_t TCC_CDK_Wrapper::FillStreamQueues(uint32_t op_code, uint32_t& aParam1, uint32_t& aParam2, int32_t iReqPacketNum, uint8_t* pPacketBuffer, secure_info_t *pstSecureInfo)
{
	int32_t ret = TCC_CDK_WRAPPER_OK;
	StreamQueue *pVideoQueue = NULL;
	StreamQueue *pAudioQueue = NULL;
	StreamQueue *pSubtitleQueue = NULL;
	StreamQueue *pStreamQueue = NULL;
	const int numQueues = iDemuxStreamQueues.size();

	uint32_t aCallbackParam1 = 0;
	uint32_t aCallbackParam2 = 0;

	switch(op_code)
	{
		case StreamQueue::THRESHOLD_TYPE_NONE:
			fpCheckBufferingCondition = &TCC_CDK_Wrapper::DoNotCheckBufferingCondition;
		break;
		
		case StreamQueue::THRESHOLD_TYPE_TIME:
		{
			fpCheckBufferingCondition = &TCC_CDK_Wrapper::CheckBufferingConditionByTime;
			// aParam1 : video buffer duration, aParam2 : audio buffer duration 
			aCallbackParam1 = aParam1;
			aCallbackParam2 = aParam2;
		}
		break;

		case StreamQueue::THRESHOLD_TYPE_CLIP_TIME:
		{
			fpCheckBufferingCondition = &TCC_CDK_Wrapper::CheckBufferingConditionByClipTime;
			// aParam1 : target clip, aParam2 : clip buffer duration 
			aCallbackParam1 = aParam1;
			aCallbackParam2 = aParam2;
		}
		break;

		case StreamQueue::THRESHOLD_TYPE_COUNT:
		{
			// aParam1 : video count, aParam2 : audio count
			fpCheckBufferingCondition = &TCC_CDK_Wrapper::CheckBufferingConditionByCount;
			aCallbackParam2 = (aParam1 > aParam2) ? aParam1 : aParam2;
		}
		break;

		case StreamQueue::THRESHOLD_TYPE_CLIP_COUNT:
		{
			fpCheckBufferingCondition = &TCC_CDK_Wrapper::CheckBufferingConditionByCount;
			// aParam1 : target clip, aParam2 : max.count 
			aCallbackParam2 = aParam2;
		}
		break;
		
		default:
		{
			PrintError("%s : Invalid op_code detected !!", __func__);
			return TCC_CDK_WRAPPER_ERROR;
		}
	}

	int queidx;
	if( (queidx = iDemuxStreamQueues.indexOfKey(AV_PACKET_VIDEO)) >= 0 )
		pVideoQueue = iDemuxStreamQueues.valueAt(queidx);
	if( (queidx = iDemuxStreamQueues.indexOfKey(AV_PACKET_AUDIO)) >= 0 )
		pAudioQueue = iDemuxStreamQueues.valueAt(queidx);
	if( (queidx = iDemuxStreamQueues.indexOfKey(AV_PACKET_SUBTITLE)) >= 0 )
		pSubtitleQueue = iDemuxStreamQueues.valueAt(queidx);

	//------------------------------------------------
	// Container Get Stream
	//------------------------------------------------
	while (CheckIfMoreBufferingIsNeeded(aCallbackParam1, aCallbackParam2) == TRUE)
	{
		if(op_code != StreamQueue::THRESHOLD_TYPE_NONE)
		{
			if(mDemuxerThread->GetState() == TCCDemuxerThread::STATE_PAUSED)
			{
				PrintInfo("%s : PAUSED", __func__);
				return TCC_CDK_WRAPPER_PAUSE_REQUEST;
			}

			if(mDemuxerThread->GetState() == TCCDemuxerThread::STATE_FINISHED)
			{
				PrintInfo("%s : FINISHED", __func__);
				return TCC_CDK_WRAPPER_END_OF_STREAM;
			}
		}
		
		for(int i = 0; i < numQueues; i++)
		{
			if((op_code == StreamQueue::THRESHOLD_TYPE_NONE) ? (iReqPacketNum != iDemuxStreamQueues.keyAt(i)) : true)
			{
				pStreamQueue = iDemuxStreamQueues.valueAt(i);
				if (pStreamQueue->GetState() == StreamQueue::STOPPED) 
				{
					PrintInfo("%s : STOPPED", __func__);
					return ret;
				}

				if (pStreamQueue->GetOneFifo(pStreamQueue->GetElementSize()) < 1)
				{
					PrintInfo("PacketNum(%d): Fail to GetOneFifo - num:time(%d:%d)", iDemuxStreamQueues.keyAt(i), 
							pStreamQueue->GetLength(), 
							pStreamQueue->GetBufferedTime()); 
					return (op_code == StreamQueue::THRESHOLD_TYPE_NONE) ? TCC_CDK_WRAPPER_GET_FRAME_NEXT_TIME
					                                                     : TCC_CDK_WRAPPER_OK;
				}

				if(pStreamQueue->GetState() == StreamQueue::STOPPING)
				{
					if (pStreamQueue->WriteFifo(NULL, NULL, pStreamQueue->GetStreamErrorCode()) >= 0)
					{
						pStreamQueue->Push();
						pStreamQueue->SetState(StreamQueue::STOPPED);
						ALOGI("FillStreamQueues - index:%d state = STOPPED", i);
					}

					return TCC_CDK_WRAPPER_OK;
				}
			}
		}

		CDK_ITEM* CDK = static_cast<CDK_ITEM*>(pItem);
		if(!CDK) return TCC_CDK_WRAPPER_ERROR;

		CDK->gsCDmxInput.m_pPacketBuff = 0;
		CDK->gsCDmxInput.m_pPacketBuff_ext1 = 0;

		if(op_code != StreamQueue::THRESHOLD_TYPE_NONE)
		{
			if( pVideoQueue ) {
				CDK->gsCDmxInput.m_pPacketBuff = pVideoQueue->GetWriteAddress();
				if( pAudioQueue )
					CDK->gsCDmxInput.m_pPacketBuff_ext1 = pAudioQueue->GetWriteAddress();
				if( pSubtitleQueue )
					CDK->gsCDmxInput.m_pPacketBuff_ext2 = pSubtitleQueue->GetWriteAddress();
			}
			else if( pAudioQueue ) {
				CDK->gsCDmxInput.m_pPacketBuff = pAudioQueue->GetWriteAddress();
				if( pSubtitleQueue )
					CDK->gsCDmxInput.m_pPacketBuff_ext2 = pSubtitleQueue->GetWriteAddress();
			}
		}
		else
		{
			if(iReqPacketNum == AV_PACKET_VIDEO)
			{
				CDK->gsCDmxInput.m_pPacketBuff = pPacketBuffer;
				if( pAudioQueue )
					CDK->gsCDmxInput.m_pPacketBuff_ext1 = pAudioQueue->GetWriteAddress();
				if( pSubtitleQueue )
					CDK->gsCDmxInput.m_pPacketBuff_ext2 = pSubtitleQueue->GetWriteAddress();
			}
			else if(iReqPacketNum == AV_PACKET_AUDIO)
			{
				if( pVideoQueue ) {
					CDK->gsCDmxInput.m_pPacketBuff = pVideoQueue->GetWriteAddress();
					CDK->gsCDmxInput.m_pPacketBuff_ext1 = pPacketBuffer;
					if( pSubtitleQueue )
						CDK->gsCDmxInput.m_pPacketBuff_ext2 = pSubtitleQueue->GetWriteAddress();
				}
				else {
					CDK->gsCDmxInput.m_pPacketBuff = pPacketBuffer;
					if( pSubtitleQueue )
						CDK->gsCDmxInput.m_pPacketBuff_ext2 = pSubtitleQueue->GetWriteAddress();
				}
			}
			else if(iReqPacketNum == AV_PACKET_SUBTITLE)
			{
				if( pVideoQueue ) {
					CDK->gsCDmxInput.m_pPacketBuff = pVideoQueue->GetWriteAddress();
					if( pAudioQueue )
						CDK->gsCDmxInput.m_pPacketBuff_ext1 = pAudioQueue->GetWriteAddress();
					CDK->gsCDmxInput.m_pPacketBuff_ext2 = pPacketBuffer;
				}
				else {
					CDK->gsCDmxInput.m_pPacketBuff = pAudioQueue->GetWriteAddress();
					CDK->gsCDmxInput.m_pPacketBuff_ext2 = pPacketBuffer;
				}
			}
		}

		CDK->gsCDmxInput.m_iPacketBuffSize = VIDEO_ELEMENT_SIZE;
		CDK->gsCDmxInput.m_iPacketType = AV_PACKET_NONE;	// PacketType has no meaning in sequential mode

		if(CDK->gsCDmxInput.m_pPacketBuff == NULL)
		{
			PrintError("%s : Invalid write address !!", __func__);
			return TCC_CDK_WRAPPER_ERROR;
		}
		
		ret = m_pfnCdmx( CDMX_GETSTREAM, CDMX_HANDLE(m_pvCdmxInst), &CDK->gsCDmxInput, &CDK->gsCDmxOutput );
		if (ret < 0)
		{
			PrintError("%s : GetStream return %d", __func__, ret);
		
			for(int i = 0; i < numQueues; i++)
			{
				pStreamQueue = iDemuxStreamQueues.valueAt(i);
				if (pStreamQueue->WriteFifo(NULL, NULL, ret) < 0)
				{
					pStreamQueue->SetStreamErrorCode(ret);
					pStreamQueue->SetState(StreamQueue::STOPPING);
				}
				else
				{
					pStreamQueue->Push();
					pStreamQueue->SetState(StreamQueue::STOPPED);
					ALOGE("FillStreamQueues - index:%d state = STOPPED", i);
				}
			}
			break; // Escape the loop
		}

		if (mEnableHdcp2 && CDK->gsCDmxOutput.m_iPacketType == AV_PACKET_VIDEO && CDK->gsCDmxOutput.m_iPacketSize == 8) {
			break;
		}

#if 1//JS Baek
		if( CDK->gsCDmxOutput.m_iPacketType == AV_PACKET_VIDEO)
		{
			mLastVideoPTS = CDK->gsCDmxOutput.m_iTimeStamp;
		}
		else if( CDK->gsCDmxOutput.m_iPacketType == AV_PACKET_AUDIO)
		{
			mLastAudioPTS = CDK->gsCDmxOutput.m_iTimeStamp;
			if(bIsUnderSkimming) {
				if( pAudioQueue && pAudioQueue->GetLength() )
						pAudioQueue->InitBufferSimple();
				break;
			}
		}
#else
		if( CDK->gsCDmxOutput.m_iPacketType == AV_PACKET_VIDEO)
			mLastVideoPTS = CDK->gsCDmxOutput.m_iTimeStamp;
		else if( CDK->gsCDmxOutput.m_iPacketType == AV_PACKET_AUDIO)
			mLastAudioPTS = CDK->gsCDmxOutput.m_iTimeStamp;
#endif
		ssize_t index = iDemuxStreamQueues.indexOfKey(CDK->gsCDmxOutput.m_iPacketType);
		if(index >= 0)
		{
			if (CDK->gsCDmxOutput.m_pPacketData != NULL && CDK->gsCDmxOutput.m_iPacketSize > 0)
			{
				if(op_code != StreamQueue::THRESHOLD_TYPE_NONE)
				{
					int32_t fifo_result;
					write_info_t wr_info = {NULL, 0, 0};
		
					pStreamQueue = iDemuxStreamQueues.valueAt(index);
		
					if( CDK->gsCDmxInput.m_pPacketBuff != CDK->gsCDmxOutput.m_pPacketData && CDK->gsCDmxInput.m_pPacketBuff_ext1 != CDK->gsCDmxOutput.m_pPacketData )
						wr_info.pbyDataBuffer = CDK->gsCDmxOutput.m_pPacketData;
					wr_info.ulDataSize = CDK->gsCDmxOutput.m_iPacketSize;
					wr_info.ulTimestamp = CDK->gsCDmxOutput.m_iTimeStamp;
		
					if( mEnableHdcp2 && CDK->gsCDmxOutput.m_ulSecureType == CDMX_SECURE_TYPE_HDCP ) {
						secure_info_t sec_info;
						sec_info.ulSecureType = CDMX_SECURE_TYPE_HDCP;
						sec_info.enSecureInfo.stHdcpInfo.ulStreamCounter = CDK->gsCDmxOutput.m_enSecureInfo.stHdcpInfo.ulStreamCounter;
						sec_info.enSecureInfo.stHdcpInfo.ullInputCounter = CDK->gsCDmxOutput.m_enSecureInfo.stHdcpInfo.ullInputCounter;
						fifo_result = pStreamQueue->WriteFifo(&wr_info, &sec_info);
					}
					else
						fifo_result = pStreamQueue->WriteFifo(&wr_info);

					if (fifo_result > 0)
					{
						pStreamQueue->Push();
					}
					else
					{
						PrintError("[TCC_CDK_Wrapper] WriteFifo Error - packet(%d)", CDK->gsCDmxOutput.m_iPacketType);
					}
				}
				else
				{
					if(iReqPacketNum == iDemuxStreamQueues.keyAt(index))
					{
						aParam1 = CDK->gsCDmxOutput.m_iPacketSize;
						aParam2 = CDK->gsCDmxOutput.m_iTimeStamp;

						if( pstSecureInfo )
						{
							if( CDK->gsCDmxOutput.m_ulSecureType == CDMX_SECURE_TYPE_HDCP ) {
								pstSecureInfo->ulSecureType = CDMX_SECURE_TYPE_HDCP;
								pstSecureInfo->enSecureInfo.stHdcpInfo.ulStreamCounter = CDK->gsCDmxOutput.m_enSecureInfo.stHdcpInfo.ulStreamCounter;
								pstSecureInfo->enSecureInfo.stHdcpInfo.ullInputCounter = CDK->gsCDmxOutput.m_enSecureInfo.stHdcpInfo.ullInputCounter;
							}
							else {
								pstSecureInfo->ulSecureType = 0;
							}
						}
					}
					else
					{
						if( (index = iDemuxStreamQueues.indexOfKey(CDK->gsCDmxOutput.m_iPacketType)) >= 0 )
						{
							int32_t fifo_result;
							write_info_t wr_info;

							pStreamQueue = iDemuxStreamQueues.valueAt(index);
	
							wr_info.pbyDataBuffer = NULL;
							wr_info.ulDataSize = CDK->gsCDmxOutput.m_iPacketSize;
							wr_info.ulTimestamp = CDK->gsCDmxOutput.m_iTimeStamp;
	
							if( mEnableHdcp2 && CDK->gsCDmxOutput.m_ulSecureType == CDMX_SECURE_TYPE_HDCP ) {
								secure_info_t sec_info;
								sec_info.ulSecureType = CDMX_SECURE_TYPE_HDCP;
								sec_info.enSecureInfo.stHdcpInfo.ulStreamCounter = CDK->gsCDmxOutput.m_enSecureInfo.stHdcpInfo.ulStreamCounter;
								sec_info.enSecureInfo.stHdcpInfo.ullInputCounter = CDK->gsCDmxOutput.m_enSecureInfo.stHdcpInfo.ullInputCounter;
								fifo_result = pStreamQueue->WriteFifo(&wr_info, &sec_info);
							}
							else
								fifo_result = pStreamQueue->WriteFifo(&wr_info);
	
							if (fifo_result > 0)
							{
								pStreamQueue->Push();
							}
							else
							{
								PrintError("[TCC_CDK_Wrapper] WriteFifo Error - packet(%d)", CDK->gsCDmxOutput.m_iPacketType);
							}
						}
						else
						{
							PrintError("[TCC_CDK_Wrapper] WriteFifo Error - packet(%d)", CDK->gsCDmxOutput.m_iPacketType);
						}
					}
				}

				PushSubtitleData();
			}
			else
			{
				PrintError("[TCC_CDK_Wrapper] Demuxer Error: %d", ret);
			}
			
			if( op_code == StreamQueue::THRESHOLD_TYPE_CLIP_COUNT ) 
			{
				if(CDK->gsCDmxOutput.m_iPacketType == aParam1)
				{
					aCallbackParam1++;
				}
			}
			else if( op_code == StreamQueue::THRESHOLD_TYPE_COUNT ) 
			{
				aCallbackParam1++;
			}
		}
		else
		{
			if(AV_PACKET_SUBTITLE == CDK->gsCDmxOutput.m_iPacketType)
			{
				PushSubtitleData();
			}
			else
			{
				PrintError("FillStreamQueue: Unknown Packet Type = [%d]", CDK->gsCDmxOutput.m_iPacketType);
			}
		}

		if(op_code == StreamQueue::THRESHOLD_TYPE_NONE)
		{
			if(iReqPacketNum == CDK->gsCDmxOutput.m_iPacketType)
			{
				break; // Escape the loop
			}
		}
	} // End of while loop 
		
#if 0	// buffering state debug
	if(op_code != StreamQueue::THRESHOLD_TYPE_NONE)
	{
		int32_t vlen = -1, alen = -1;
		uint32_t vtime = 0, atime = 0;

		if( pVideoQueue ) {
			vlen = pVideoQueue->GetLength();
			vtime = pVideoQueue->GetBufferedTime();
		}
		if( pAudioQueue ) {
			alen = pAudioQueue->GetLength();
			atime = pAudioQueue->GetBufferedTime();
		}
			
		PrintInfo("[SEQUENTIAL BUFFERING] OP(%d: %d,%d), V(frm:%d, time:%d), A(frm:%d, time:%d)"
				  , op_code
				  , aCallbackParam1
				  , aCallbackParam2
				  , vlen, vtime
				  , alen, atime);
	}
#endif

	return TCC_CDK_WRAPPER_OK;
}

int32_t TCC_CDK_Wrapper::FillStreamQueue(uint32_t op_code, uint32_t& aParam1, uint32_t& aParam2)
{
	int32_t ret = TCC_CDK_WRAPPER_OK;
	StreamQueue *pVideoQueue = NULL;
	StreamQueue *pAudioQueue = NULL;
	StreamQueue *pSubtitleQueue = NULL;
	StreamQueue* pStreamQueue = NULL;
	const int numQueues = iDemuxStreamQueues.size();

	uint32_t aCallbackParam1 = 0;
	uint32_t aCallbackParam2 = 0;

	switch(op_code)
	{
	case StreamQueue::THRESHOLD_TYPE_NONE:
			fpCheckBufferingCondition = &TCC_CDK_Wrapper::DoNotCheckBufferingCondition;
			break;
		
	case StreamQueue::THRESHOLD_TYPE_TIME:
		fpCheckBufferingCondition = &TCC_CDK_Wrapper::CheckBufferingConditionByTime;
		// aParam1 : video buffer duration, aParam2 : audio buffer duration 
		aCallbackParam1 = aParam1;
		aCallbackParam2 = aParam2;
		break;

	case StreamQueue::THRESHOLD_TYPE_CLIP_TIME:
		fpCheckBufferingCondition = &TCC_CDK_Wrapper::CheckBufferingConditionByClipTime;
		// aParam1 : target clip, aParam2 : clip buffer duration 
		aCallbackParam1 = aParam1;
		aCallbackParam2 = aParam2;
		break;

	case StreamQueue::THRESHOLD_TYPE_COUNT:
		// aParam1 : video count, aParam2 : audio count
		fpCheckBufferingCondition = &TCC_CDK_Wrapper::CheckBufferingConditionByCount;
		aCallbackParam2 = (aParam1 > aParam2) ? aParam1 : aParam2;
		break;

	case StreamQueue::THRESHOLD_TYPE_CLIP_COUNT:
		fpCheckBufferingCondition = &TCC_CDK_Wrapper::CheckBufferingConditionByCount;
		// aParam1 : target clip, aParam2 : max.count 
		aCallbackParam2 = aParam2;
		break;
		
	default:
		PrintError("%s : Invalid op_code detected !!", __func__);
		return TCC_CDK_WRAPPER_ERROR;
	}

	int queidx;
	if( (queidx = iDemuxStreamQueues.indexOfKey(AV_PACKET_VIDEO)) >= 0 )
		pVideoQueue = iDemuxStreamQueues.valueAt(queidx);
	if( (queidx = iDemuxStreamQueues.indexOfKey(AV_PACKET_AUDIO)) >= 0 )
		pAudioQueue = iDemuxStreamQueues.valueAt(queidx);
	if( (queidx = iDemuxStreamQueues.indexOfKey(AV_PACKET_SUBTITLE)) >= 0 )
		pSubtitleQueue = iDemuxStreamQueues.valueAt(queidx);

	//------------------------------------------------
	// Container Get Stream
	//------------------------------------------------
	CDK_ITEM* CDK = static_cast<CDK_ITEM*>(pItem);
	if(!CDK) return TCC_CDK_WRAPPER_ERROR;
	while( CheckIfMoreBufferingIsNeeded(aCallbackParam1, aCallbackParam2) == TRUE )
	{
		// check whether the thread is paused or finished state
		if(mDemuxerThread->GetState() == TCCDemuxerThread::STATE_PAUSED) {
			PrintInfo("%s : PAUSED", __func__);
			return TCC_CDK_WRAPPER_PAUSE_REQUEST;
		}
		if(mDemuxerThread->GetState() == TCCDemuxerThread::STATE_FINISHED) {
			PrintInfo("%s : FINISHED", __func__);
			return TCC_CDK_WRAPPER_END_OF_STREAM;
		}

		if(	op_code == StreamQueue::THRESHOLD_TYPE_CLIP_COUNT
			&& aParam1 == AV_PACKET_SUBTITLE
			&& (pSubtitleQueue != NULL && pSubtitleQueue->GetLength() == 0) ) {
			pStreamQueue = pSubtitleQueue;
			CDK->gsCDmxInput.m_iPacketType = AV_PACKET_SUBTITLE;
			CDK->gsCDmxInput.m_iPacketBuffSize = SUBTITLE_ELEMENT_SIZE;
		}
		else {
			// set target stream type to extract
			if( video_end == false && (audio_end == true || (mLastVideoPTS < mLastAudioPTS && pAudioQueue->GetLength()) || pVideoQueue->GetLength() == 0) ) {
				pStreamQueue = pVideoQueue;
				CDK->gsCDmxInput.m_iPacketType = AV_PACKET_VIDEO;
				CDK->gsCDmxInput.m_iPacketBuffSize = VIDEO_ELEMENT_SIZE;
			} 
			else if( audio_end == false ){
				pStreamQueue = pAudioQueue;
				CDK->gsCDmxInput.m_iPacketType = AV_PACKET_AUDIO;
				CDK->gsCDmxInput.m_iPacketBuffSize = AUDIO_ELEMENT_SIZE;
			}
			else {
				for(int i = 0; i < numQueues; i++) {
					pStreamQueue = iDemuxStreamQueues.valueAt(i);
					if (pStreamQueue->WriteFifo(NULL, NULL, ret) < 0) {
						pStreamQueue->SetStreamErrorCode(ret);
						pStreamQueue->SetState(StreamQueue::STOPPING);
					}
					else {
						pStreamQueue->Push();
						pStreamQueue->SetState(StreamQueue::STOPPED);
						ALOGE("FillStreamQueues - index:%d state = STOPPED", i);
					}
				}
				break; // Escape the loop
			}
		}

		// check whether the queue is stopped state
		if(pStreamQueue->GetState() == StreamQueue::STOPPED) {
			PrintInfo("%s : STOPPED", __func__);
			return ret;
		}

		// check whether the queue is full
		if(pStreamQueue->GetOneFifo(pStreamQueue->GetElementSize()) < 1) {
			PrintInfo("PacketNum(%d): Fail to GetOneFifo - num:time(%d:%d)", CDK->gsCDmxInput.m_iPacketType, 
					pStreamQueue->GetLength(), 
					pStreamQueue->GetBufferedTime()); 
			return TCC_CDK_WRAPPER_OK;
		}

		// check whether the queue is stopping state
		if(pStreamQueue->GetState() == StreamQueue::STOPPING) {
			if(pStreamQueue->WriteFifo(NULL, NULL, pStreamQueue->GetStreamErrorCode()) >= 0) {
				pStreamQueue->Push();
				pStreamQueue->SetState(StreamQueue::STOPPED);
				ALOGI("FillStreamQueues - type:%d state = STOPPED", CDK->gsCDmxInput.m_iPacketType);
			}
			return TCC_CDK_WRAPPER_OK;
		}

		// set output buffer
		CDK->gsCDmxInput.m_pPacketBuff = pStreamQueue->GetWriteAddress();
		CDK->gsCDmxInput.m_pPacketBuff_ext1 = 0;
		
		ret = m_pfnCdmx( CDMX_GETSTREAM, CDMX_HANDLE(m_pvCdmxInst), &CDK->gsCDmxInput, &CDK->gsCDmxOutput );
		if( ret < 0) {
			PrintError("%s : GetStream return %d", __func__, ret);

			for(int i = 0; i < numQueues; i++) {
				pStreamQueue = iDemuxStreamQueues.valueAt(i);
				if (pStreamQueue->WriteFifo(NULL, NULL, ret) < 0) {
					pStreamQueue->SetStreamErrorCode(ret);
					pStreamQueue->SetState(StreamQueue::STOPPING);
				}
				else {
					pStreamQueue->Push();
					pStreamQueue->SetState(StreamQueue::STOPPED);
					ALOGE("FillStreamQueues - index:%d state = STOPPED", i);
				}
			}

			break; // Escape the loop
		}

#if 1//JS Baek
		if( CDK->gsCDmxOutput.m_iPacketType == AV_PACKET_VIDEO)
			mLastVideoPTS = CDK->gsCDmxOutput.m_iTimeStamp;
		else if( CDK->gsCDmxOutput.m_iPacketType == AV_PACKET_AUDIO) {
			mLastAudioPTS = CDK->gsCDmxOutput.m_iTimeStamp;
			if(bIsUnderSkimming) {
				if( pAudioQueue && pAudioQueue->GetLength() )
						pAudioQueue->InitBufferSimple();
				break;
			}
		}
#else
		if( CDK->gsCDmxOutput.m_iPacketType == AV_PACKET_VIDEO)
			mLastVideoPTS = CDK->gsCDmxOutput.m_iTimeStamp;
		else if( CDK->gsCDmxOutput.m_iPacketType == AV_PACKET_AUDIO)
			mLastAudioPTS = CDK->gsCDmxOutput.m_iTimeStamp;
#endif

		if (mEnableHdcp2 && CDK->gsCDmxOutput.m_iPacketType == AV_PACKET_VIDEO && CDK->gsCDmxOutput.m_iPacketSize == 8) {
			break;
		}

		ssize_t index = iDemuxStreamQueues.indexOfKey(CDK->gsCDmxOutput.m_iPacketType);
		if(index >= 0)
		{
			if(CDK->gsCDmxOutput.m_pPacketData == NULL || CDK->gsCDmxOutput.m_iPacketSize <= 0)
				PrintError("[TCC_CDK_Wrapper] Demuxer Error: %d", ret);
			else
			{
				int32_t fifo_result;
				write_info_t wr_info = {NULL, 0, 0};
	
				pStreamQueue = iDemuxStreamQueues.valueAt(index);
	
				if( CDK->gsCDmxInput.m_pPacketBuff != CDK->gsCDmxOutput.m_pPacketData )
					wr_info.pbyDataBuffer = CDK->gsCDmxOutput.m_pPacketData;
				wr_info.ulDataSize = CDK->gsCDmxOutput.m_iPacketSize;
				wr_info.ulTimestamp = CDK->gsCDmxOutput.m_iTimeStamp;
	
				if( mEnableHdcp2 && CDK->gsCDmxOutput.m_ulSecureType == CDMX_SECURE_TYPE_HDCP ) {
					secure_info_t sec_info;
					sec_info.ulSecureType = CDMX_SECURE_TYPE_HDCP;
					sec_info.enSecureInfo.stHdcpInfo.ulStreamCounter = CDK->gsCDmxOutput.m_enSecureInfo.stHdcpInfo.ulStreamCounter;
					sec_info.enSecureInfo.stHdcpInfo.ullInputCounter = CDK->gsCDmxOutput.m_enSecureInfo.stHdcpInfo.ullInputCounter;
					fifo_result = pStreamQueue->WriteFifo(&wr_info, &sec_info);
				}
				else
					fifo_result = pStreamQueue->WriteFifo(&wr_info);

				if (fifo_result > 0)
					pStreamQueue->Push();
				else
					PrintError("[TCC_CDK_Wrapper] WriteFifo Error - packet(%d)", CDK->gsCDmxOutput.m_iPacketType);

				PushSubtitleData();
			}
			
			if( op_code == StreamQueue::THRESHOLD_TYPE_CLIP_COUNT ) {
				if(AV_PACKET_SUBTITLE == aParam1)
					aCallbackParam1++; 
				else if(CDK->gsCDmxOutput.m_iPacketType == aParam1)
					aCallbackParam1++;
			}
			else if( op_code == StreamQueue::THRESHOLD_TYPE_COUNT ) {
				aCallbackParam1++;
			}
		}
		else
		{
			if( AV_PACKET_SUBTITLE != CDK->gsCDmxOutput.m_iPacketType || iFileInfo.iNumSubtitleStreams <= 0)
				PrintError("FillStreamQueue: Unknown Packet Type = [%d]", CDK->gsCDmxOutput.m_iPacketType);
		}
	} // End of while loop 
		
#if 0	// buffering state debug
	{
		int32_t vlen = -1, alen = -1;
		uint32_t vtime = 0, atime = 0;

		if( pVideoQueue ) {
			vlen = pVideoQueue->GetLength();
			vtime = pVideoQueue->GetBufferedTime();
		}
		if( pAudioQueue ) {
			alen = pAudioQueue->GetLength();
			atime = pAudioQueue->GetBufferedTime();
		}

		PrintInfo("[SELECTIVE BUFFERING] OP(%d: %d,%d), V(frm:%d, time:%d), A(frm:%d, time:%d)"
				  , op_code
				  , aCallbackParam1
				  , aCallbackParam2
				  , vlen, vtime
				  , alen, atime);
	}
#endif

	return TCC_CDK_WRAPPER_OK;
}

////////////////////////////////////////////////////////////////////////////////
int32_t TCC_CDK_Wrapper::FillStreamQueues(bool *queueFulled) {
	StreamQueue *pVideoQueue = NULL;
	StreamQueue *pAudioQueue = NULL;
	StreamQueue *pSubtitleQueue = NULL;
	StreamQueue *pStreamQueue = NULL;
	const int32_t numQueues = iDemuxStreamQueues.size();

    if (queueFulled != NULL) {
        *queueFulled = false;
    }

	int queidx;
	if( (queidx = iDemuxStreamQueues.indexOfKey(AV_PACKET_VIDEO)) >= 0 )
		pVideoQueue = iDemuxStreamQueues.valueAt(queidx);
	if( (queidx = iDemuxStreamQueues.indexOfKey(AV_PACKET_AUDIO)) >= 0 )
		pAudioQueue = iDemuxStreamQueues.valueAt(queidx);
	if( (queidx = iDemuxStreamQueues.indexOfKey(AV_PACKET_SUBTITLE)) >= 0 )
		pSubtitleQueue = iDemuxStreamQueues.valueAt(queidx);

	//------------------------------------------------
	// Container Get Stream
	//------------------------------------------------
	
	for (int32_t i = 0; i < numQueues; i++) {
		pStreamQueue = iDemuxStreamQueues.valueAt(i);
		if (pStreamQueue->GetState() == StreamQueue::STOPPED) {
			PrintInfo("FillStreamQueues: STOPPED");
			bool allStopped = true;
			for (int32_t j = 0; j < numQueues; j++) {
				if (iDemuxStreamQueues.valueAt(j)->GetState() 
					    != StreamQueue::STOPPED) {
                    allStopped = false;
					break;
				}
			}
			return allStopped ? TCC_CDK_WRAPPER_END_OF_FILE : TCC_CDK_WRAPPER_OK;
		}

		if (pStreamQueue->GetOneFifo(pStreamQueue->GetElementSize()) < 1) {
			PrintInfo("PacketNum(%d): Fail to GetOneFifo - num:time(%d:%d)", iDemuxStreamQueues.keyAt(i), 
					pStreamQueue->GetLength(), 
					pStreamQueue->GetBufferedTime()); 
            if (queueFulled != NULL) {
                *queueFulled = true;
            }
			return TCC_CDK_WRAPPER_OK;
		}

		if (pStreamQueue->GetState() == StreamQueue::STOPPING) {
			if (pStreamQueue->WriteFifo(NULL, NULL, pStreamQueue->GetStreamErrorCode()) >= 0) {
				pStreamQueue->Push();
				pStreamQueue->SetState(StreamQueue::STOPPED);
				ALOGI("FillStreamQueues - index:%d state = STOPPED", i);
			}

			return TCC_CDK_WRAPPER_OK;
		}
	}

	CDK_ITEM* CDK = static_cast<CDK_ITEM*>(pItem);
	if (!CDK) return TCC_CDK_WRAPPER_ERROR;

	CDK->gsCDmxInput.m_pPacketBuff = 0;
	CDK->gsCDmxInput.m_pPacketBuff_ext1 = 0;

	if( pVideoQueue ) {
		CDK->gsCDmxInput.m_pPacketBuff = pVideoQueue->GetWriteAddress();
		if( pAudioQueue )
			CDK->gsCDmxInput.m_pPacketBuff_ext1 = pAudioQueue->GetWriteAddress();
		if( pSubtitleQueue )
			CDK->gsCDmxInput.m_pPacketBuff_ext2 = pSubtitleQueue->GetWriteAddress();
	}
	else {
		if( pAudioQueue )
            CDK->gsCDmxInput.m_pPacketBuff = pAudioQueue->GetWriteAddress();
		if( pSubtitleQueue )
			CDK->gsCDmxInput.m_pPacketBuff_ext2 = pSubtitleQueue->GetWriteAddress();
	}

    if (CDK->gsCDmxInput.m_pPacketBuff == NULL
		|| (pVideoQueue && pAudioQueue && CDK->gsCDmxInput.m_pPacketBuff_ext1 == NULL)) {
		ALOGE("FillStreamQueues: Invalid write address !!");
		return TCC_CDK_WRAPPER_ERROR;
	}

	CDK->gsCDmxInput.m_iPacketBuffSize = VIDEO_ELEMENT_SIZE;
	CDK->gsCDmxInput.m_iPacketType = AV_PACKET_NONE;

	int32_t ret;
    for (int32_t i = 0; i < 10; i++) {

		ret = m_pfnCdmx( CDMX_GETSTREAM, CDMX_HANDLE(m_pvCdmxInst), &CDK->gsCDmxInput, &CDK->gsCDmxOutput );
        if (ret < 0) {
            ALOGE("FillStreamQueues: GetStream return %d, Seek cancelled %d", ret, mSeekCancelled);

            if (mSeekCancelled) {
                mSeekCancelled = false;
                return TCC_CDK_WRAPPER_END_OF_STREAM;
            }

            if (ret == -11)  {
                // set_first_frame_with_seqheader (Invalid NALsize detected)
                ALOGE("FillStreamQueues: GetStream again - trial %d", i);
                continue;
            }
        }
        break;
    }

    if (ret < 0) {
        for (int32_t i = 0; i < numQueues; i++) {
            pStreamQueue = iDemuxStreamQueues.valueAt(i);
            if (pStreamQueue->WriteFifo(NULL, NULL, ret) < 0) {
                pStreamQueue->SetStreamErrorCode(ret);
                pStreamQueue->SetState(StreamQueue::STOPPING);
            } else {
                pStreamQueue->Push();
                pStreamQueue->SetState(StreamQueue::STOPPED);
                ALOGE("FillStreamQueues - index:%d state = STOPPED", i);
            }
        }

        return TCC_CDK_WRAPPER_OK;
    }

	if (CDK->gsCDmxOutput.m_iPacketType == AV_PACKET_VIDEO) {
		mLastVideoPTS = CDK->gsCDmxOutput.m_iTimeStamp;
	} else if (CDK->gsCDmxOutput.m_iPacketType == AV_PACKET_AUDIO) {
		mLastAudioPTS = CDK->gsCDmxOutput.m_iTimeStamp;
		if(bIsUnderSkimming) {
			if( pAudioQueue && pAudioQueue->GetLength() )
				pAudioQueue->InitBufferSimple();
			return TCC_CDK_WRAPPER_OK;
		}
	}

	ssize_t index = iDemuxStreamQueues.indexOfKey(CDK->gsCDmxOutput.m_iPacketType);
	if (index >= 0) {
		if (CDK->gsCDmxOutput.m_pPacketData != NULL && CDK->gsCDmxOutput.m_iPacketSize > 0) {
			int32_t fifo_result;
			write_info_t wr_info;

			pStreamQueue = iDemuxStreamQueues.valueAt(index);

			wr_info.pbyDataBuffer = NULL;
			wr_info.ulDataSize = CDK->gsCDmxOutput.m_iPacketSize;
			wr_info.ulTimestamp = CDK->gsCDmxOutput.m_iTimeStamp;

			if( mEnableHdcp2 && CDK->gsCDmxOutput.m_ulSecureType == CDMX_SECURE_TYPE_HDCP ) {
				secure_info_t sec_info;
				sec_info.ulSecureType = CDMX_SECURE_TYPE_HDCP;
				sec_info.enSecureInfo.stHdcpInfo.ulStreamCounter = CDK->gsCDmxOutput.m_enSecureInfo.stHdcpInfo.ulStreamCounter;
				sec_info.enSecureInfo.stHdcpInfo.ullInputCounter = CDK->gsCDmxOutput.m_enSecureInfo.stHdcpInfo.ullInputCounter;
				fifo_result = pStreamQueue->WriteFifo(&wr_info, &sec_info);
			}
			else
				fifo_result = pStreamQueue->WriteFifo(&wr_info);

			if (fifo_result > 0) {
				pStreamQueue->Push();
			} else {
				ALOGE("FillStreamQueues: WriteFifo Error - packet(%d)", CDK->gsCDmxOutput.m_iPacketType);
			}

			PushSubtitleData();
		} else {
			ALOGE("FillStreamQueues: Demuxer Error");
		}
	} else {
        if (AV_PACKET_SUBTITLE != CDK->gsCDmxOutput.m_iPacketType)
            PrintError("FillStreamQueues: Unknown Packet Type = [%d]", CDK->gsCDmxOutput.m_iPacketType);
    }

    return TCC_CDK_WRAPPER_OK;
}
////////////////////////////////////////////////////////////////////////////////

bool TCC_CDK_Wrapper::CheckIfMoreBufferingIsNeeded(uint32_t& aParam1, uint32_t& aParam2)
{
	if( iGetStreamMode & CDK_GETSTREAM_SEQUENTIAL_MODE == 0 ) 
	{
		int queidx;
		StreamQueue *pStreamQueue = NULL;

		if( (queidx = iDemuxStreamQueues.indexOfKey(AV_PACKET_VIDEO)) >= 0 ) {
			pStreamQueue = iDemuxStreamQueues.valueAt(queidx);
			if( pStreamQueue->GetLength() == 0 )
				return true;
		}

		if( (queidx = iDemuxStreamQueues.indexOfKey(AV_PACKET_AUDIO)) >= 0 ) {
			pStreamQueue = iDemuxStreamQueues.valueAt(queidx);
			if( pStreamQueue->GetLength() == 0 )
				return true;
		}
	}

	return (this->*fpCheckBufferingCondition)(aParam1, aParam2);
}

bool TCC_CDK_Wrapper::DoNotCheckBufferingCondition(uint32_t& aParam1, uint32_t& aParam2)
{
	return true;
}

bool TCC_CDK_Wrapper::CheckBufferingConditionByCount(uint32_t& aAmount, uint32_t& iCount)
{
	return (aAmount < iCount) ? true : false;
}

bool TCC_CDK_Wrapper::CheckBufferingConditionByTime(uint32_t& aVideoTime, uint32_t& aAudioTime)
{
	bool ret = false;
	int queidx;

	if( iVidPacketNum >= 0 ) {
		queidx = iDemuxStreamQueues.indexOfKey(iVidPacketNum);
		if( queidx >= 0 && iDemuxStreamQueues.valueAt(queidx)->GetBufferedTime() < aVideoTime )
			ret = true;
	}

	if( iAudPacketNum >= 0 ) {
		queidx = iDemuxStreamQueues.indexOfKey(iAudPacketNum);
		if( queidx >= 0 && iDemuxStreamQueues.valueAt(queidx)->GetBufferedTime() < aAudioTime )
			ret = true;
	}

	return ret;
}

bool TCC_CDK_Wrapper::CheckBufferingConditionByClipTime(uint32_t& aTargetClipNum, uint32_t& aClipTime)
{
	int queidx = iDemuxStreamQueues.indexOfKey(aTargetClipNum);

	if( queidx >= 0 )
		return (iDemuxStreamQueues.valueAt(queidx)->GetBufferedTime() < aClipTime) ? true : false;
	else
		return false;
}

void TCC_CDK_Wrapper::SendReqMsgToFeedDataIfUnderruned(uint32_t aDemuxerProcCmd, uint32_t index, uint32_t aParam1, uint32_t aParam2, uint32_t aCmdOption)
{
	StreamQueue* pStreamQueue = iDemuxStreamQueues.valueAt(index);
	uint32_t iCurrBufferingAmount = pStreamQueue->GetCurrBufferingAmount();
	if(iCurrBufferingAmount < pStreamQueue->GetMinBufferingThreshold())
	{
		if(DoesPrevReqMsgToFeedDataRemain() == false)
		{
			PrintInfo("GetStreamData: DMX Q(packet_num:%d) - Amount:%d/%d", iDemuxStreamQueues.keyAt(index), 
			                                                                iCurrBufferingAmount, 
																			pStreamQueue->GetMinBufferingThreshold());
			if(aParam1 && aParam2)
			{
				mDemuxerThread->FeedData(TCCDemuxerThread::CMD_FEED, aDemuxerProcCmd, aParam1, aParam2, aCmdOption);
			}
			else
			{
				mDemuxerThread->FeedData(TCCDemuxerThread::CMD_FEED, aDemuxerProcCmd, 
						iDemuxStreamQueues.keyAt(index), pStreamQueue->GetMaxBufferingThreshold(), aCmdOption); 
			}
		}
	}
}

void TCC_CDK_Wrapper::feedMore(int64_t videoThresholdUs, int64_t audioThresholdUs) 
{
    if (!(iGetStreamMode & DEMUXER_LOOPER_ENABLED_MODE)) {
        // Do nothing..
        return;
    }

    int32_t minDurationMs = (videoThresholdUs > audioThresholdUs) 
                          ? audioThresholdUs / 1000 : videoThresholdUs / 1000;

	ALOGI("feedMore %d ms", minDurationMs);
    mCDKDemuxHandler->feedOnce(minDurationMs);
}

int32_t TCC_CDK_Wrapper::checkQueuedDurationUs(int64_t *durationUs, bool *fulled) 
{
    *durationUs = 0;
    const int numQueues = iDemuxStreamQueues.size();
    if (numQueues == 0) {
        return INVALID_OPERATION;
    }

	int minIndex;
    for(int i = 0; i < numQueues; i++) {
        StreamQueue* pStreamQueue = iDemuxStreamQueues.valueAt(i);
        int64_t queuedDurationUs = pStreamQueue->GetBufferedTime() * 1000;

		if (i == 0 || queuedDurationUs < *durationUs) {
            *durationUs = queuedDurationUs;
            minIndex = i;
		}

		if (pStreamQueue->GetQueuedState() == StreamQueue::STREAM_Q_FULL) {
            if (mProfilingMode) {
                ALOGW("Q full: %s", (iDemuxStreamQueues.keyAt(i) == AV_PACKET_VIDEO) 
                                    ? "video" : "audio");
            }
            if (fulled != NULL) {
                *fulled = true;
            }
		}

        if (mProfilingMode) {
            uint32_t w_off, r_off;
            ALOGD("[PROFILE] demux: %s - %7.2f secs, %6d EA, avail %12d(w: %10d, r: %10d)", 
                    (iDemuxStreamQueues.keyAt(i) == AV_PACKET_VIDEO) ? "video" : "audio", 
                    (queuedDurationUs / 1E6), pStreamQueue->GetLength(), 
                    pStreamQueue->GetAvailBytes(w_off, r_off), w_off, r_off);
        }
    }

    if (fulled != NULL && (audio_end || video_end)) {
        *fulled = true;
    }
#if 0
    ALOGV("min.Q duration: %s, %.2f secs, %d EA", 
            (iDemuxStreamQueues.keyAt(minIndex) == AV_PACKET_VIDEO) ? "video" : "audio", 
            ((*durationUs) / 1E6), iDemuxStreamQueues.valueAt(minIndex)->GetLength());
#endif

    return TCC_CDK_WRAPPER_OK;
}

void TCC_CDK_Wrapper::cancelSeekIfNecessary(bool cancelled) {
    mSeekCancelled = cancelled;
}

void TCC_CDK_Wrapper::signalUnderrun(bool underrun, int64_t thresholdUs) 
{
    if (DEMUXER_LOOPER_ENABLED_MODE & iGetStreamMode) {
        mCDKDemuxHandler->signalUnderrun(underrun, thresholdUs);
    }
}

void TCC_CDK_Wrapper::createAndRunDemuxerThread()
{
    Mutex::Autolock lock(mLock);

    if(mDemuxerThread != NULL)
    {
        PrintError("%s: Prev. Demuxer Thread exists ..", __func__);
        mDemuxerThread->Close(TCCDemuxerThread::MSG_SYNC_ON);
        destroyDemuxerThread();
    }

    mDemuxerThread = new TCCDemuxerThread(this);
    mDemuxerThread->SetState(TCCDemuxerThread::STATE_RUNNING);
    mDemuxerThread->run("TCCDemuxerThread", ANDROID_PRIORITY_FOREGROUND);

    ALOGI("%s : activated", __func__);
}

void TCC_CDK_Wrapper::destroyDemuxerThread()
{
	if(mDemuxerThread != NULL)
	{ 
		sp<TCCDemuxerThread> myDemuxerThread;
		{
			Mutex::Autolock lock(mLock);
			myDemuxerThread = mDemuxerThread;
		}

		myDemuxerThread->requestExit();
		milliseconds(500);
		status_t status = myDemuxerThread->requestExitAndWait();

		{
			Mutex::Autolock lock(mLock);
			myDemuxerThread.clear();
		}

		ALOGI("Demuxer Thread is destroyed.");
	}
	else
	{
		PrintError("Demuxer Thread is not activated.");
		return;
	}
}

bool TCC_CDK_Wrapper::DoesPrevReqMsgToFeedDataRemain(uint32_t aCmdOption)
{
	bool bRet = false;

	if(aCmdOption == TCCDemuxerThread::MSG_SYNC_OFF)
	{
		mDemuxerGetFrameReqCnt++;
		if(mDemuxerGetFrameReqCnt == 0xFFFFFFFF)
		{
			// max. gap before roll over : 1024
			mDemuxerGetFrameReqCnt = (mDemuxerGetFrameRespCnt > 0xFFFFFBFF) 
				? (mDemuxerGetFrameRespCnt + 1) : 0;
		}

		if((mDemuxerGetFrameReqCnt > mDemuxerGetFrameRespCnt) && 
				((mDemuxerGetFrameReqCnt - mDemuxerGetFrameRespCnt) > 1))
		{
			// Skip this request msg.
			mDemuxerGetFrameReqCnt--;
			bRet = true;
		}
	}

	return bRet;
}

bool TCC_CDK_Wrapper::SanityCheckBufferSpaceAvailability()
{
	const int numQueues = iDemuxStreamQueues.size();

	if( iGetStreamMode & CDK_GETSTREAM_SEQUENTIAL_MODE ) 
	{
		for(int i = 0; i < numQueues; i++) {
			StreamQueue* pStreamQueue = iDemuxStreamQueues.valueAt(i);
			if(pStreamQueue->GetQueuedState() == StreamQueue::STREAM_Q_FULL) {
				return false;
			}
		}
	}
	else
	{
		for(int i = 0; i < numQueues; i++) {
			StreamQueue* pStreamQueue = iDemuxStreamQueues.valueAt(i);
			if( pStreamQueue->GetQueuedState() == StreamQueue::STREAM_Q_EMPTY ) {
				return true;
			}
			if( pStreamQueue->GetQueuedState() == StreamQueue::STREAM_Q_FULL) {
				return false;
			}
		}
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////
bool TCC_CDK_Wrapper::TCCDemuxerThread::threadLoop()
{
	mCDKWrapper->demuxerThread();

	iCommandQueues.removeItem(MessageQueue::REQ);
	iCommandQueues.removeItem(MessageQueue::RESP);

	delete pDemuxerReqCmdQ;
	delete pDemuxerRespCmdQ;

	return false; // loop until we need to quit
}

void TCC_CDK_Wrapper::TCCDemuxerThread::FeedData(uint32_t aCommand, int32_t aParam1, int32_t aParam2, int32_t aParam3, int32_t aCmdOption)
{
	iCommandQueues.valueFor(MessageQueue::REQ)->Send(aCommand, aParam1, aParam2, aParam3, aCmdOption);

	if(aCmdOption == TCCDemuxerThread::MSG_SYNC_ON)
	{
		Message RespMsg;
		iCommandQueues.valueFor(MessageQueue::RESP)->Receive(&RespMsg);
	}
}

void TCC_CDK_Wrapper::TCCDemuxerThread::Pause(int32_t aCmdOption)
{
	iCommandQueues.valueFor(MessageQueue::REQ)->Send(TCCDemuxerThread::CMD_PAUSE, aCmdOption);

	if(aCmdOption == TCCDemuxerThread::MSG_SYNC_ON)
	{
		Message RespMsg;
		iCommandQueues.valueFor(MessageQueue::RESP)->Receive(&RespMsg);
	}
}

int32_t TCC_CDK_Wrapper::TCCDemuxerThread::SeekTo(int32_t targetTS, int32_t aCmdOption)
{
	int32_t ret = ERR_END_OF_FILE;
	iCommandQueues.valueFor(MessageQueue::REQ)->Send(TCCDemuxerThread::CMD_SEEK, aCmdOption, targetTS);

	if(aCmdOption == TCCDemuxerThread::MSG_SYNC_ON)
	{
		Message RespMsg;
		iCommandQueues.valueFor(MessageQueue::RESP)->Receive(&RespMsg);
		if(RespMsg.command == TCCDemuxerThread::CMD_SEEK)
		{
			ret = RespMsg.arg1;
		}
	}

	return ret;
}

void TCC_CDK_Wrapper::TCCDemuxerThread::Resume(uint32_t aDemuxerProcCmd, int32_t aParam1, int32_t aParam2, int32_t aCmdOption)
{
	iCommandQueues.valueFor(MessageQueue::REQ)->Send(TCCDemuxerThread::CMD_RESUME, aDemuxerProcCmd, aParam1, aParam2, aCmdOption);
	
	if(aCmdOption == TCCDemuxerThread::MSG_SYNC_ON)
	{
		Message RespMsg;
		iCommandQueues.valueFor(MessageQueue::RESP)->Receive(&RespMsg);
	}
}

int32_t TCC_CDK_Wrapper::TCCDemuxerThread::Close(int32_t aCmdType)
{
	 int32_t ret = ERR_END_OF_FILE;

	iCommandQueues.valueFor(MessageQueue::REQ)->Send(TCCDemuxerThread::CMD_CLOSE, aCmdType);

	if(aCmdType == TCCDemuxerThread::MSG_SYNC_ON)
	{
		Message RespMsg;
		iCommandQueues.valueFor(MessageQueue::RESP)->Receive(&RespMsg);
		if(RespMsg.command == TCCDemuxerThread::CMD_CLOSE)
		{
			ret = RespMsg.arg1;
		}
	}

	return ret;
}
////////////////////////////////////////////////////////////////////////////////

int TCC_CDK_Wrapper::demuxerThread()
{
	Message msg;
	bool  bLooping = true;
	int32_t ret = TCC_CDK_WRAPPER_OK;
	uint32_t sync_option = TCCDemuxerThread::MSG_SYNC_OFF;

	while(bLooping)
	{
		mDemuxerThread->iCommandQueues.valueFor(MessageQueue::REQ)->Receive(&msg);
		PrintInfo("[%s] command : %d", __func__, msg.command);

		switch(msg.command)
		{
			case TCCDemuxerThread::CMD_FEEDMORE:
			{
				ALOGV("CMD_FEEDMORE");
			}
			case TCCDemuxerThread::CMD_FEED:
			{
				if(++mDemuxerGetFrameRespCnt == 0xFFFFFFFF) mDemuxerGetFrameRespCnt = 0;

				if(SanityCheckBufferSpaceAvailability() == true)
				{
					if( iGetStreamMode & CDK_GETSTREAM_SEQUENTIAL_MODE )
						ret = FillStreamQueues(msg.arg1, msg.arg2, msg.arg3);
					else
						ret = FillStreamQueue(msg.arg1, msg.arg2, msg.arg3);
				}

				if(ret == TCC_CDK_WRAPPER_ERROR)
				{
					PrintError("[%s] ret:%d, ready to loop out..", __func__, ret);
					bLooping = false;
				}
				sync_option = msg.arg4;
			}
			break;

			case TCCDemuxerThread::CMD_PAUSE:
			{
				sync_option = msg.arg1;
			}
			break;

			case TCCDemuxerThread::CMD_SEEK:
			{
				ret = MediaSeekCore(msg.arg2);
				sync_option = TCCDemuxerThread::MSG_SYNC_ON;
			}
			break;

			case TCCDemuxerThread::CMD_RESUME:
			{
				// determine if data feeding is needed to avoid underrun (just after seek state).
				if( msg.arg2 == 0 && msg.arg3 == 0 )
					ret = msg.arg1;
				else {
					if( iGetStreamMode & CDK_GETSTREAM_SEQUENTIAL_MODE )
						ret = FillStreamQueues(msg.arg1, msg.arg2, msg.arg3);
					else
						ret = FillStreamQueue(msg.arg1, msg.arg2, msg.arg3);
				}

				if(ret == TCC_CDK_WRAPPER_ERROR)
				{
					PrintError("[%s] ret:%d, ready to loop out..", __func__, ret);
					bLooping = false;
				}
				sync_option = msg.arg4;
			}
			break;

			case TCCDemuxerThread::CMD_CLOSE:
			{
				bLooping = false;
				ret = CDKCloseCore();
				sync_option = msg.arg1;
			}
			break;

			default:
				PrintError("[%s] - unsupported command : %d", __func__, msg.command);
				sync_option = TCCDemuxerThread::MSG_SYNC_OFF;
			break;
		}

		if(sync_option == TCCDemuxerThread::MSG_SYNC_ON)
		{
			mDemuxerThread->iCommandQueues.valueFor(MessageQueue::RESP)->Send(msg.command, ret);
			PrintInfo("[%s] - command(%d) End", __func__, msg.command);
		}
	}

	PrintInfo("%s thread loop out", __func__);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
bool TCC_CDK_Wrapper::CDKDemuxHandler::isAlive() {
	return !mStopped;
}

void TCC_CDK_Wrapper::CDKDemuxHandler::signalUnderrun(bool underrun, int64_t thresholdUs) {
    mThreshold = underrun ? thresholdUs : kESLowWatermarkUs;
	mUnderrun = underrun;
    ALOGI("signalUnderrun %d", underrun);
}

void TCC_CDK_Wrapper::CDKDemuxHandler::start(int32_t prefillMs) {
	mLooper->registerHandler(this);
	mLooper->start(false, false, PRIORITY_FOREGROUND);

    if (prefillMs) {
		Mutex::Autolock autoLock(mHandlerLock);

	    sp<AMessage> msg = new AMessage(kWhatStart, id());

		msg->setInt32("prefillMs", prefillMs);
        mPrefilled = false;
		msg->post();

		while (mPrefilled == false) {
			mHandlerCondition.wait(mHandlerLock);
		}
	} else {
	    (new AMessage(kWhatFeed, id()))->post();
	}
}

void TCC_CDK_Wrapper::CDKDemuxHandler::stop() {
    if (mStopped) return;

	Mutex::Autolock autoLock(mHandlerLock);

	ALOGD("CDKDemuxHandler::stopping....");
	(new AMessage(kWhatStop, id()))->post();
	while (!mStopped) {
		mHandlerCondition.wait(mHandlerLock);
	}
	ALOGD("CDKDemuxHandler::stopped");
}

void TCC_CDK_Wrapper::CDKDemuxHandler::feedOnce(int32_t feedMs) {
	if (feedMs) {
		Mutex::Autolock autoLock(mHandlerLock);

        mFeedPending = true;

		sp<AMessage> msg = new AMessage(kWhatFeedOnce, id());
		msg->setInt32("feedMs", feedMs);
	    msg->post();

		while (mFeedPending == true) {
			mHandlerCondition.wait(mHandlerLock);
		} 
	}

	(new AMessage(kWhatFeed, id()))->post();
}

void TCC_CDK_Wrapper::CDKDemuxHandler::pause() {
	if (mPausePending) {
		return;
	}

	Mutex::Autolock autoLock(mHandlerLock);

    mPausePending = true;

	(new AMessage(kWhatPause, id()))->post();
	while (!mPaused) {
		mHandlerCondition.wait(mHandlerLock);
	}
    mPausePending = false;
}

void TCC_CDK_Wrapper::CDKDemuxHandler::resume(int32_t prefillMs) {
	if (prefillMs) {
		feedOnce(prefillMs);
        (new AMessage(kWhatFeed, id()))->post();
	} else {
        if (mResumePending) {
            return;
        }

        Mutex::Autolock autoLock(mHandlerLock);

        mResumePending = true;
        (new AMessage(kWhatResume, id()))->post();

        while (!mResumed) {
            mHandlerCondition.wait(mHandlerLock);
        }
        mResumePending = false;
        mResumed = false;
	}
}

int32_t TCC_CDK_Wrapper::CDKDemuxHandler::seekTo(int32_t targetTS) {
    Mutex::Autolock autoLock(mHandlerLock);

    sp<AMessage> msg = new AMessage(kWhatSeek, id());
	msg->setInt32("timestamp", targetTS);

	CHECK(mAsyncResult == NULL);
	msg->post();

	while (mAsyncResult == NULL) {
		mHandlerCondition.wait(mHandlerLock);
	}

	int32_t result;
	CHECK(mAsyncResult->findInt32("result", &result));
    mAsyncResult.clear();

	return result;
}

void TCC_CDK_Wrapper::CDKDemuxHandler::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatStart:
        {
            ALOGE("kWhatStart");
            int32_t prefillMs;
            if (msg->findInt32("prefillMs", &prefillMs)) {
                ALOGD("prefill %d ms frames", prefillMs);
                bool queueFulled;
                for (int32_t i = 0;; i++) {
                    int32_t err = mOwner->FillStreamQueues(&queueFulled);
                    if (err == TCC_CDK_WRAPPER_ERROR) {
                        ALOGE("kWhatStart: err %d", err);
                        (new AMessage(kWhatStop, id()))->post();
                        return;
                    }

                    uint32_t videoDurationMs = prefillMs;
                    uint32_t audioDurationMs = prefillMs;
                    if (err != TCC_CDK_WRAPPER_OK || queueFulled 
                            || !mOwner->CheckBufferingConditionByTime(videoDurationMs, audioDurationMs)) {
                        Mutex::Autolock autoLock(mHandlerLock);

                        mPrefilled = true;
                        mHandlerCondition.signal();

                        CHECK_NE(prefillMs, 0);
                        mApproxFeedCount = (i < 10) ? 10 : ((1000 * i) / prefillMs) / 5;
                        ALOGD("fill stream %d times, approx %d err %d", i, mApproxFeedCount, err);
                        break;
                    }
                }
            } 

            (new AMessage(kWhatFeed, id()))->post();

            break;
        }

        case kWhatStop:
        {
            ALOGI("kWhatStop");
            mStopped = true;
            mFeedPending = false;

            mHandlerCondition.signal();
            break;
        }

        case kWhatFeed:
        {
            if (mPaused || mStopped || mPausePending) {
                ALOGW("kWhatFeed: paused (%d stopped %d pause pending %d)", mPaused, mStopped, mPausePending);
                break;
            }

            int64_t durationUs = 0;
            int64_t delayUs = 0;

            bool queueFulled = false;
            do {
                int32_t err = mOwner->FillStreamQueues(&queueFulled);
                if (err == TCC_CDK_WRAPPER_ERROR) {
                    ALOGE("kWhatFeed: ready to loop out.. (err %d)", err);
                    (new AMessage(kWhatStop, id()))->post();
                    return;
                }

                if (err == TCC_CDK_WRAPPER_END_OF_FILE) {
                    ALOGE("kWhatFeed: EOF");
                    return;
                }

                if (err == TCC_CDK_WRAPPER_GET_FRAME_CANCELLED) {
                    ALOGE("kWhatFeed: stop frame feeding");
                    mStopped = true;
                    return;
                }

                if (queueFulled) {
                    delayUs = 100000ll;
                    break;
                }

                if (mFeedPending == true) {
                    ALOGW("feed pended....");
                    break;
                }

                if (TCC_CDK_WRAPPER_OK != mOwner->checkQueuedDurationUs(&durationUs)) {
                    break;
                }
            } while (durationUs < mThreshold && !mPausePending && !mStopped);

            if (!mPausePending && !mStopped) {
                if (delayUs == 0 && !mUnderrun) {
                    if (durationUs > kESHighWatermarkUs) {
                        delayUs = 100000ll;
                    } else if (durationUs >= kESMinWatermarkUs * 3) {
                        delayUs = 50000ll;
                    } else if (durationUs >= kESLowWatermarkUs) {
                        delayUs = 30000ll;
                    }
                }
                msg->post(delayUs);
            }

            break;
        }

        case kWhatFeedOnce:
        {
            if (mStopped) {
                break;
            }

            int32_t feedMs;
            CHECK(msg->findInt32("feedMs", &feedMs));

            uint32_t param1 = feedMs;

            Mutex::Autolock autoSerializer(mSerializer);

            while (mOwner->CheckBufferingConditionByTime(param1, param1)) {
                int32_t err = mOwner->FillStreamQueues();
                if (err == TCC_CDK_WRAPPER_ERROR) {
                    ALOGE("kWhatFeedMore: ready to loop out.. (err %d)", err);
                    (new AMessage(kWhatStop, id()))->post();
                    break;
                }

                if (err == TCC_CDK_WRAPPER_GET_FRAME_CANCELLED) {
                    ALOGE("kWhatFeedMore: stop frame feeding");
                    mStopped = true;

                    break;
                }
            }

            Mutex::Autolock autoLock(mHandlerLock);

            mFeedPending = false;
            break;
        }

        case kWhatPause:
        {
            ALOGI("kWhatPause");
            mPaused = true;
            mHandlerCondition.signal();
            break;
        }

        case kWhatSeek:
        {
            int32_t targetTS;
            CHECK(msg->findInt32("timestamp", &targetTS));

            ALOGI("kWhatSeek: timestamp %.2f sec", targetTS / 1E3);

            int32_t ret = mOwner->MediaSeekCore(targetTS);

            Mutex::Autolock autoLock(mHandlerLock);

            CHECK(mAsyncResult == NULL);

            mAsyncResult = new AMessage;
            mAsyncResult->setInt32("result", ret);

            mHandlerCondition.signal();
            break;
        }

        case kWhatResume:
        {
            ALOGI("kWhatResume");
            mPaused = false;
            mStopped = false;

            mOwner->ClearStreamBuffer(STREAM_TYPE_ALL);

            if (mOwner->FillStreamQueues() == TCC_CDK_WRAPPER_ERROR) {
                (new AMessage(kWhatStop, id()))->post();
            } else {
                (new AMessage(kWhatFeed, id()))->post();
            }

            Mutex::Autolock autoLock(mHandlerLock);

            mResumed = true;
            mHandlerCondition.signal();
            break;
        }
    }
}
