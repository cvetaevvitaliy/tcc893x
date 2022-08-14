/****************************************************************************
 *   FileName    : cdmx.c
 *   Description : 
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
#include "../cdk/cdk_core.h"
#include <dlfcn.h>

#ifdef TCC_VPU_C7_INCLUDE
//#define ATTACH_HEADER_VP8 // it's working on vpu component.
#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	LOG
//
#define LOG_TAG    "CDMX"

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


static void* (*CDMX_MALLOC	) ( unsigned int );
static void  (*CDMX_FREE	) ( void* );
static int   (*CDMX_MEMCMP	) ( const void*, const void*, unsigned int );
static void* (*CDMX_MEMCPY	) ( void*, const void*, unsigned int );
static void* (*CDMX_MEMMOVE	) ( void*, const void*, unsigned int );
static void* (*CDMX_MEMSET	) ( void*, int, unsigned int );
static void* (*CDMX_REALLOC	) ( void*, unsigned int );

typedef struct cdk_mpeg4_vol
{
	unsigned char*	m_pbyPtr;
	unsigned long	m_ulPos;
	unsigned long	m_ulData;
	unsigned long	m_ulUsedBytes;
} cdk_mpeg4_vol;

static cdk_mpeg4_vol gs_stVolPutData;

static const unsigned long 
gs_ulStuffBits[8] = 
{
	0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F
};

static inline 
unsigned long
log2bin (
	unsigned long ulVal
	)
{
	long cnt = 0;

	while( ulVal )
	{
		ulVal >>= 1;
		cnt++;
	}
	return cnt;
}

static void
vol_putbits_init (
	unsigned char* pbyBitsOutbuf
	) 
{
	gs_stVolPutData.m_pbyPtr		= pbyBitsOutbuf;
	gs_stVolPutData.m_ulPos			= 0;
	gs_stVolPutData.m_ulData		= 0;
	gs_stVolPutData.m_ulUsedBytes	= 0;
}


static unsigned long 
vol_putbits (
	unsigned char	ucNumBits,
	unsigned long 	ulValue,
	unsigned long 	ulOption	//0 : not return
								//1 : return used bytes after bit stuffing for byte align
	)
{
	unsigned long ret = 0;
	long i, bit_pos, bit_data;
	long write_bits, used_bytes;
	unsigned char* pby_bitptr;
	static const unsigned long bit_mask[32] = 
	{ 	
		0x00000000, 0x00000001, 0x00000003, 0x00000007, 0x0000000F, 0x0000001F, 0x0000003F, 0x0000007F,
		0x000000FF, 0x000001FF, 0x000003FF, 0x000007FF, 0x00000FFF, 0x00001FFF, 0x00003FFF, 0x00007FFF,
		0x0000FFFF, 0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF, 0x001FFFFF, 0x003FFFFF, 0x007FFFFF,
		0x00FFFFFF, 0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF, 0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF 
	};

	if( (ucNumBits == 0) && (ulOption == 0) )
		return 0;

	bit_pos		= gs_stVolPutData.m_ulPos;	
	pby_bitptr	= gs_stVolPutData.m_pbyPtr;	
	bit_data	= gs_stVolPutData.m_ulData;
	used_bytes	= gs_stVolPutData.m_ulUsedBytes;

	write_bits = bit_pos + ucNumBits;
	if( write_bits < 32 )
	{
		bit_data = (bit_data<<ucNumBits) | ulValue;
		bit_pos +=  ucNumBits;
	}
	else
	{
		i = 32-bit_pos;
		bit_data = (bit_data<<i)|((ulValue<<(32-ucNumBits))>>bit_pos);

		*pby_bitptr++ = (unsigned char)((bit_data>>24)&0x0FF);
		*pby_bitptr++ = (unsigned char)((bit_data>>16)&0x0FF);
		*pby_bitptr++ = (unsigned char)((bit_data>> 8)&0x0FF);
		*pby_bitptr++ = (unsigned char)((bit_data    )&0x0FF);
		bit_pos		= ucNumBits - i;
		bit_data	= ulValue & bit_mask[bit_pos];
		used_bytes += 4;

	}

	if( ulOption ) 
	{
		if( bit_pos > 24 )
		{
			bit_pos -= 24;
			i = bit_data>>bit_pos;
			*pby_bitptr++ = (unsigned char)((i>>16)&0x0FF);
			*pby_bitptr++ = (unsigned char)((i>> 8)&0x0FF);
			*pby_bitptr++ = (unsigned char)((i    )&0x0FF);
			used_bytes += 3;
		}
		else if ( bit_pos > 16 )
		{
			bit_pos -= 16;
			i = bit_data>>bit_pos;
			*pby_bitptr++ = (unsigned char)((i>> 8)&0x0FF);
			*pby_bitptr++ = (unsigned char)((i    )&0x0FF);
			used_bytes += 2;
		}
		else if ( bit_pos > 8 )
		{
			bit_pos -= 8;
			i = bit_data>>bit_pos;
			*pby_bitptr++ = (unsigned char)((i    )&0x0FF);
			used_bytes += 1;
		}
		if( bit_pos > 0 )
		{
			i = 8 - (bit_pos & 0x7);
			if(i != 8)
			{
				*pby_bitptr++ = (unsigned char)( ((bit_data&bit_mask[bit_pos])<<i)|gs_ulStuffBits[i - 1] );
				used_bytes += 1;
			}
		}
		ret = used_bytes;
	}

	gs_stVolPutData.m_pbyPtr		= pby_bitptr;
	gs_stVolPutData.m_ulPos			= bit_pos;
	gs_stVolPutData.m_ulData		= bit_data;
	gs_stVolPutData.m_ulUsedBytes	= used_bytes;

	return ret;
}

long
make_mpeg4_vol_header( 
	unsigned char 	 	 *pbyStreamData,
	unsigned long  	  	  ulStreamSize,
	unsigned char		**ppbyDest,
	unsigned long	 	 *pulDestSize,
	unsigned long 		  ulWidth,
	unsigned long 		  ulHeight,
	unsigned long 		  bIsAsp 
	)
{
	// should be made VOL header.
	unsigned char* pby_vop = pbyStreamData;
	unsigned long idx;
	unsigned long found = 0;

	for( idx = 0; idx < ulStreamSize; ++idx )
	{
		if( pby_vop[idx+2] == 0x01 && pby_vop[idx+3] == 0xb6 )
		{
			if( (pby_vop[idx+4]&0xc0) == 0 ) // VOP type I
			{
				found = 1;
				break;
			}
		}
	}

	if( found )
	{
		long time_incr = 0;
		long modulo = 0;
		long bits = 0;
		long cnt = 0;

		unsigned char* pby_outbuf;
		long vol_len = 20;

		if( *ppbyDest == NULL )
		{
			*ppbyDest = CDMX_MALLOC( vol_len );
			if( *ppbyDest == NULL ) 
			{
				ALOGE( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] *ppbyDest: 0x%x\n", __LINE__, *ppbyDest);
			*pulDestSize = vol_len;
			CDMX_MEMSET( *ppbyDest, 0, vol_len );
		}

		pby_outbuf = *ppbyDest;

		cnt = 4;
		bits = 2;

		while ( ((pby_vop[cnt]<<bits)&0x80) != 0 )	/* time_base */
		{
			bits++;
			if( bits == 8 )
			{
				bits = 0;
				cnt++;
			}
			modulo++;
		}
		bits++; // to the next bit
		if( bits == 8 )
		{
			bits = 0;
			cnt++;
		}
		bits++; // skip for marker bit
		if( bits == 8 )
		{
			bits = 0;
			cnt++;
		}

		while ( ((pby_vop[cnt]<<bits)&0x80) == 0 )
		{
			bits++;
			if( bits == 8 )
			{
				bits = 0;
				cnt++;
			}
			time_incr++;
		}

		vol_putbits_init(pby_outbuf);

		//vol start code
		vol_putbits(32, 0x120, 0);
		//random accessible vol
		vol_putbits(1, 0, 0);

		// vo_type
		if(bIsAsp){
			vol_putbits(8, 17, 0);
		}else{
			vol_putbits(8, 1, 0);
		}

		vol_putbits(1, 0, 0);
		//aspect ratio
		vol_putbits(4, 1, 0);

		//vol control
		vol_putbits(1, 1, 0);
		//chroma format
		vol_putbits(2, 1, 0); // 4:2:0 format

		//low delay
		if(bIsAsp){
			vol_putbits(1, 0, 0);
		}else{
			vol_putbits(1, 1, 0);
		}

		//vbv
		vol_putbits(1, 0, 0);
		//vo layer shape
		vol_putbits(2, 0, 0); //rect

		//marker
		vol_putbits(1, 1, 0);

//////////////////////////////////////////////////////////////////////////
		//vop time increment resolution
		vol_putbits(16, (1<<time_incr)-1, 0);
		//marker
		vol_putbits(1, 1, 0);
		//vop rate
		vol_putbits(1, 1, 0);
		//vop time increment
		vol_putbits((unsigned char)time_incr, 1, 0);
//////////////////////////////////////////////////////////////////////////

		//marker
		vol_putbits(1, 1, 0);
		//width
		vol_putbits(13, ulWidth, 0);
		//marker
		vol_putbits(1, 1, 0);
		//height
		vol_putbits(13, ulHeight, 0);
		//marker
		vol_putbits(1, 1, 0);
		//interlaced
		vol_putbits(1, 0, 0);
		//OBMC
		vol_putbits(1, 1, 0);
		//splite Enable
		vol_putbits(1, 0, 0);
		//not 8_bit
		vol_putbits(1, 0, 0);
		//quant type
		vol_putbits(1, 0, 0);
		//complexity estimation disable
		vol_putbits(1, 1, 0);
		//resync_marker disable
		vol_putbits(1, 1, 0);
		//data_partitioned
		vol_putbits(1, 0, 0);
		//scalability
		vol_putbits(1, 0, 0);
		//bit stuffing for byte align
		//scalability and bit stuffing for byte align

		*pulDestSize = vol_putbits(1, 0, 1);
        return 0;
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	demuxer information
//
void
cdmx_memory_func_init( 
    av_memory_func_t *pstMemoryFuncs
	)
{
	CDMX_MALLOC		= pstMemoryFuncs->m_pfnMalloc;
	CDMX_FREE		= pstMemoryFuncs->m_pfnFree;
	CDMX_MEMCMP		= pstMemoryFuncs->m_pfnMemcmp;
	CDMX_MEMCPY		= pstMemoryFuncs->m_pfnMemcpy;
	CDMX_MEMSET		= pstMemoryFuncs->m_pfnMemset;
	CDMX_REALLOC	= pstMemoryFuncs->m_pfnRealloc;
	CDMX_MEMMOVE	= pstMemoryFuncs->m_pfnMemmove;
}

void 
cdmx_print_data(
   unsigned char	*pbyData, 
   long 	 		 lDatalen,
   long 	 		 lDivisor,
   long 	 		 lMaxCount
   )
{
	long i = 0, j = 0;
	long divisor, quotient, remain, division;
	unsigned char* ptr = pbyData;
	long data_len = (lDatalen < lMaxCount) ? lDatalen : lMaxCount;
	divisor = lDivisor;

	quotient = data_len % divisor;
	division = data_len / divisor;
	remain = data_len - (quotient * divisor);
	DATA("[DATA:0x%08x] [LEN:%ld(%ld)] [Div:%ld(%ld)] [Quo:%ld] [Rem:%ld] \n", (unsigned int)ptr, lDatalen, data_len, division, divisor, quotient, remain);
#ifdef __ANDROID__

	for (i=0; i<division; i++) 
	{
		switch (divisor) 
		{
		case 4:
			DATA("0x%02x 0x%02x 0x%02x 0x%02x ", ptr[j+0], ptr[j+1], ptr[j+2], ptr[j+3]);
			break;
		case 8:
			DATA("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ", 
				 ptr[j+0], ptr[j+1], ptr[j+2], ptr[j+3], ptr[j+4], ptr[j+5], ptr[j+6], ptr[j+7]);
			break;
		case 16:
			DATA("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ", 
				 ptr[j+0], ptr[j+1], ptr[j+2], ptr[j+3], ptr[j+4], ptr[j+5], ptr[j+6], ptr[j+7],
				 ptr[j+8], ptr[j+9], ptr[j+10], ptr[j+11], ptr[j+12], ptr[j+13], ptr[j+14], ptr[j+15]);

			break;
		}
		j+= divisor;
	}

	switch (quotient) 
	{
	case 1: DATA("0x%02x ", ptr[j+0]); break;
	case 2: DATA("0x%02x 0x%02x ", ptr[j+0], ptr[j+1]); break;
	case 3: DATA("0x%02x 0x%02x 0x%02x ", ptr[j+0], ptr[j+1], ptr[j+2]); break;
	case 4: DATA("0x%02x 0x%02x 0x%02x 0x%02x ", ptr[j+0], ptr[j+1], ptr[j+2], ptr[j+3]); break;
	case 5: DATA("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ", ptr[j+0], ptr[j+1], ptr[j+2], ptr[j+3], ptr[j+4]); break;
	case 6: DATA("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ", ptr[j+0], ptr[j+1], ptr[j+2], ptr[j+3], ptr[j+4], ptr[j+5]); break;
	case 7: DATA("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ", ptr[j+0], ptr[j+1], ptr[j+2], ptr[j+3], ptr[j+4], ptr[j+5], ptr[j+6]); break;
	case 8: DATA("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ", ptr[j+0], ptr[j+1], ptr[j+2], ptr[j+3], ptr[j+4], ptr[j+5], ptr[j+6], ptr[j+7]); break;
	case 9: DATA("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ", ptr[j+0], ptr[j+1], ptr[j+2], ptr[j+3], ptr[j+4], ptr[j+5], ptr[j+6], ptr[j+7], ptr[j+8]); break;
	case 10:
		DATA("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ", 
			 ptr[j+0], ptr[j+1], ptr[j+2], ptr[j+3], ptr[j+4], ptr[j+5], ptr[j+6], ptr[j+7], ptr[j+8], ptr[j+9]); break;
	case 11:
		DATA("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ", 
			 ptr[j+0], ptr[j+1], ptr[j+2], ptr[j+3], ptr[j+4], ptr[j+5], ptr[j+6], ptr[j+7], ptr[j+8], ptr[j+9], ptr[j+10]); break;
	case 12:
		DATA("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ", 
			 ptr[j+0], ptr[j+1], ptr[j+2], ptr[j+3], ptr[j+4], ptr[j+5], ptr[j+6], ptr[j+7], ptr[j+8], ptr[j+9], ptr[j+10], ptr[j+11]); break;
	case 13:
		DATA("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ", 
			 ptr[j+0], ptr[j+1], ptr[j+2], ptr[j+3], ptr[j+4], ptr[j+5], ptr[j+6], ptr[j+7], ptr[j+8], ptr[j+9], ptr[j+10], ptr[j+11], ptr[j+12]); break;
	case 14:
		DATA("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ", 
			 ptr[j+0], ptr[j+1], ptr[j+2], ptr[j+3], ptr[j+4], ptr[j+5], ptr[j+6], ptr[j+7], ptr[j+8], ptr[j+9], ptr[j+10], ptr[j+11], ptr[j+12], ptr[j+13]); break;
	case 15:
		DATA("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ", 
			 ptr[j+0], ptr[j+1], ptr[j+2], ptr[j+3], ptr[j+4], ptr[j+5], ptr[j+6], ptr[j+7], ptr[j+8], ptr[j+9], ptr[j+10], ptr[j+11], ptr[j+12], ptr[j+13], ptr[j+14]); break;
	case 16:
		DATA("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ", 
			 ptr[j+0], ptr[j+1], ptr[j+2], ptr[j+3], ptr[j+4], ptr[j+5], ptr[j+6], ptr[j+7], ptr[j+8], ptr[j+9], ptr[j+10], ptr[j+11], ptr[j+12], ptr[j+13], ptr[j+14], ptr[j+15]); break;
	}

#else
	for (i=0; i<division; i++) {
		for (j=0; j<divisor; j++) {
			DATA("0x%02x ", ptr[j] );
		}
		DATA("\n");
		ptr += divisor;
	}
	for (i=0; i<quotient; i++) {
		DATA("0x%02x ", ptr[i] );
	}
	DATA("\n");
#endif
}

void
cdmx_print_info (
    void 	*pDmxInfo,
	int 	 lVideoInfoLength,
	int 	 lAudioInfoLength,
	int 	 lSubtitleInfoLength,
	int 	 lGraphicsInfoLength
	)
{
	av_dmx_info_t		*pst_dmx_info	= (av_dmx_info_t*)pDmxInfo;
	av_dmx_file_info_t	*pst_file_info	= pst_dmx_info->m_pstFileInfo;

	FUNC( "In cdmx_print_info\n" );
	if ( pst_file_info ) 
	{
		DSTATUS( "=============================================================\n" );
		DSTATUS( "                  Demuxer Init Information                   \n" );
		DSTATUS( "=============================================================\n" );
		DSTATUS( " << Input File Info >>  \n" );
		DSTATUS( "  1. File Name       : \"%s\"      \n", pst_file_info->m_pszFileName);
		DSTATUS( "  2. Duration        : %ld ms      \n", pst_file_info->m_lDuration);
		DSTATUS( "  3. File Size       : %lld byte   \n", pst_file_info->m_llFileSize);
	}
	if( lVideoInfoLength && pst_dmx_info->m_lVideoStreamTotal > 0 )
	{
		av_dmx_video_info_t	*pst_video_info = pst_dmx_info->m_pstDefaultVideoInfo;

		DSTATUS( " << Default Video Info >>  \n" );
		DSTATUS( "  1. StreamID        : %ld           \n",	pst_video_info->m_lStreamID);
		DSTATUS( "  2. Frame Size      : %ld x %ld pix \n",	pst_video_info->m_lWidth, pst_video_info->m_lHeight);
		DSTATUS( "  3. Frame Rate      : %.3f fps      \n",	(float)pst_video_info->m_lFrameRate/1000);
		DSTATUS( "  4. FourCC          : \"%c%c%c%c\"  \n",	(char)(pst_video_info->m_ulFourCC >> 0 ),
															(char)(pst_video_info->m_ulFourCC >> 8 ),
															(char)(pst_video_info->m_ulFourCC >> 16),
															(char)(pst_video_info->m_ulFourCC >> 24));
		DSTATUS( "  5. Extra Length    : %ld Byte     \n",	pst_video_info->m_lExtraLength);
	}
	if( lAudioInfoLength && pst_dmx_info->m_lAudioStreamTotal > 0 )
	{
		av_dmx_audio_info_t	*pst_audio_info = pst_dmx_info->m_pstDefaultAudioInfo;

		DSTATUS( " << Default Audio Info >>  \n" );
		DSTATUS( "  1. StreamID        : %ld         \n", pst_audio_info->m_lStreamID);
		DSTATUS( "  2. Sampling Rate   : %ld Hz      \n", pst_audio_info->m_lSamplePerSec);
		DSTATUS( "  3. Bits per Sample : %ld bits    \n", pst_audio_info->m_lBitsPerSample);
		DSTATUS( "  4. Channels        : %ld Ch      \n", pst_audio_info->m_lChannels);
		DSTATUS( "  5. FormatTag       : 0x%04X      \n", (unsigned int)pst_audio_info->m_lFormatTag);
		DSTATUS( "  6. Extra Length    : %ld Byte    \n", pst_audio_info->m_lExtraLength);
		DSTATUS( "  7. Language        : \"%s\"      \n", pst_audio_info->m_pszLanguage);
	}
	if( lSubtitleInfoLength && pst_dmx_info->m_lSubtitleStreamTotal > 0 )
	{
		av_dmx_subtitle_info_t *pst_subtitle_info = pst_dmx_info->m_pstDefaultSubtitleInfo;

		DSTATUS( " << Default Subtitle Info >>  \n" );
		DSTATUS( "  1. StreamID        : %ld         \n", pst_subtitle_info->m_lStreamID);
		DSTATUS( "  2. Language        : \"%s\"      \n", pst_subtitle_info->m_pszLanguage);
	}
	if( lGraphicsInfoLength && pst_dmx_info->m_lGraphicsStreamTotal > 0 )
	{
		av_dmx_graphics_info_t *pst_graphics_info = pst_dmx_info->m_pstDefaultGraphicsInfo;

		DSTATUS( " << Default Graphics Info >>  \n" );
		DSTATUS( "  1. StreamID        : %ld         \n", pst_graphics_info->m_lStreamID);
		DSTATUS( "  2. Language        : \"%s\"      \n", pst_graphics_info->m_pszLanguage);
	}

	DSTATUS( "=============================================================\n" );
}

void
cdmx_print_sel_info (
    av_dmx_video_info_t		*pstVideoInfo,
	av_dmx_audio_info_t		*pstAudioInfo,
	av_dmx_subtitle_info_t 	*pstSubtitleInfo,
	av_dmx_graphics_info_t 	*pstGraphicsInfo
	)
{
	FUNC( "In cdmx_print_sel_info\n" );

	DSTATUS( "=============================================================\n" );
	DSTATUS( "                Selected Stream Information                  \n" );
	DSTATUS( "=============================================================\n" );
	if( pstVideoInfo )
	{
		DSTATUS( " << Video Info >>  \n" );
		DSTATUS( "  1. StreamID        : %ld           \n", pstVideoInfo->m_lStreamID);
		DSTATUS( "  2. Frame Size      : %ld x %ld pix \n", pstVideoInfo->m_lWidth, pstVideoInfo->m_lHeight);
		DSTATUS( "  3. Frame Rate      : %.3f fps      \n", (float)pstVideoInfo->m_lFrameRate/1000);
		DSTATUS( "  4. FourCC          : \"%c%c%c%c\"  \n", (char)(pstVideoInfo->m_ulFourCC >> 0 ),
															(char)(pstVideoInfo->m_ulFourCC >> 8 ),
															(char)(pstVideoInfo->m_ulFourCC >> 16),
															(char)(pstVideoInfo->m_ulFourCC >> 24));
		DSTATUS( "  5. Extra Length    : %ld           \n", pstVideoInfo->m_lExtraLength);
	}
	if( pstAudioInfo )
	{
		DSTATUS( " << Audio Info >>  \n" );
		DSTATUS( "  1. StreamID        : %ld          \n", pstAudioInfo->m_lStreamID);
		DSTATUS( "  2. Sampling Rate   : %ld Hz       \n", pstAudioInfo->m_lSamplePerSec);
		DSTATUS( "  3. Bits per Sample : %ld bits     \n", pstAudioInfo->m_lBitsPerSample);
		DSTATUS( "  4. Channels        : %ld Ch       \n", pstAudioInfo->m_lChannels);
		DSTATUS( "  5. FormatTag       : 0x%04X       \n", (unsigned int)pstAudioInfo->m_lFormatTag);
		DSTATUS( "  6. Extra Length    : %ld Byte     \n", pstAudioInfo->m_lExtraLength);
		DSTATUS( "  7. Language        : \"%s\"       \n", pstAudioInfo->m_pszLanguage);
	}
	if( pstSubtitleInfo )
	{
		DSTATUS( " << Subtitle Info >>  \n" );
		DSTATUS( "  1. StreamID        : %ld          \n", pstSubtitleInfo->m_lStreamID);
		DSTATUS( "  2. Language        : \"%s\"       \n", pstSubtitleInfo->m_pszLanguage);
	}
	if( pstGraphicsInfo )
	{
		DSTATUS( " << Graphics Info >>  \n" );
		DSTATUS( "  1. StreamID        : %ld          \n", pstGraphicsInfo->m_lStreamID);
		DSTATUS( "  2. Language        : \"%s\"       \n", pstGraphicsInfo->m_pszLanguage);
	}

	DSTATUS( "=============================================================\n" );
}

long
cdmx_get_stream_id_list (
    cdmx_stream_id_list_t 	*pstStreamIdList,
    void 					*pDmxInfo,
	long 	 			 	 lVideoInfoLength,
	long 	 			 	 lAudioInfoLength,
	long 	 			 	 lSubtitleInfoLength,
	long 	 			 	 lGraphicsInfoLength
	)
{
	long i;
	unsigned char	*pby_info_list;
	av_dmx_info_t	*pst_dmx_info = (av_dmx_info_t*)pDmxInfo;

	FUNC( "In cdmx_get_stream_id_list\n" );
	DSTATUS( "===================================================================\n" );
	DSTATUS( "                            Stream List                            \n" );
	DSTATUS( "===================================================================\n" );
	if( lVideoInfoLength && pst_dmx_info->m_lVideoStreamTotal > 0 )
	{
		av_dmx_video_info_t	*pst_video_info;

		pby_info_list = (unsigned char*)pst_dmx_info->m_pstVideoInfoList;

		if ( pstStreamIdList->plVideoIdList == NULL )
		{
			pstStreamIdList->plVideoIdList = (long *)CDMX_MALLOC( sizeof(long) * pst_dmx_info->m_lVideoStreamTotal );
			if ( pstStreamIdList->plVideoIdList == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] pstStreamIdList->plVideoIdList: 0x%x\n", __LINE__, pstStreamIdList->plVideoIdList);
			CDMX_MEMSET( pstStreamIdList->plVideoIdList, 0, sizeof(long) * pst_dmx_info->m_lVideoStreamTotal );
		}

		for(i = 0; i < pst_dmx_info->m_lVideoStreamTotal; i++)
		{
			pst_video_info = (av_dmx_video_info_t*)pby_info_list;
			pstStreamIdList->plVideoIdList[i] = pst_video_info->m_lStreamID;
			
			DSTATUS( " [Video_%03ld] - StreamID: %ld / %ldx%ld pix / %.3f fps / FourCC: \"%c%c%c%c\" \n",
						i,
						pst_video_info->m_lStreamID,
						pst_video_info->m_lWidth,
						pst_video_info->m_lHeight,
						(float)pst_video_info->m_lFrameRate/1000,
						(char)(pst_video_info->m_ulFourCC >> 0 ),
						(char)(pst_video_info->m_ulFourCC >> 8 ),
						(char)(pst_video_info->m_ulFourCC >> 16),
						(char)(pst_video_info->m_ulFourCC >> 24)
					);

			pby_info_list += lVideoInfoLength;
		}
	}

	if( lAudioInfoLength && pst_dmx_info->m_lAudioStreamTotal > 0 )
	{
		av_dmx_audio_info_t	*pst_audio_info;

		pby_info_list = (unsigned char*)pst_dmx_info->m_pstAudioInfoList;

		if ( pstStreamIdList->plAudioIdList == NULL )
		{
			pstStreamIdList->plAudioIdList = (long *)CDMX_MALLOC( sizeof(long) * pst_dmx_info->m_lAudioStreamTotal );
			if ( pstStreamIdList->plAudioIdList == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] pstStreamIdList->plAudioIdList: 0x%x\n", __LINE__, pstStreamIdList->plAudioIdList);
			CDMX_MEMSET( pstStreamIdList->plAudioIdList, 0, sizeof(long) * pst_dmx_info->m_lAudioStreamTotal );
		}

		for(i = 0; i < pst_dmx_info->m_lAudioStreamTotal; i++)
		{
			pst_audio_info = (av_dmx_audio_info_t*)pby_info_list;
			pstStreamIdList->plAudioIdList[i] = pst_audio_info->m_lStreamID;

			DSTATUS( " [Audio_%03ld] - StreamID: %ld / %ld Hz / %ld bits / %ld Ch / FormatTag: 0x%04X / Language: \"%s\" \n",
						i,
						pst_audio_info->m_lStreamID,
						pst_audio_info->m_lSamplePerSec,
						pst_audio_info->m_lBitsPerSample,
						pst_audio_info->m_lChannels,
						(unsigned int)pst_audio_info->m_lFormatTag,
						pst_audio_info->m_pszLanguage);

			pby_info_list += lAudioInfoLength;
		}
	}

	if( lSubtitleInfoLength && pst_dmx_info->m_lSubtitleStreamTotal > 0 )
	{
		av_dmx_subtitle_info_t *pst_subtitle_info;

		pby_info_list = (unsigned char*)pst_dmx_info->m_pstSubtitleInfoList;

		if ( pstStreamIdList->plSubtitleIdList == NULL )
		{
			pstStreamIdList->plSubtitleIdList = (long *)CDMX_MALLOC( sizeof(long) * pst_dmx_info->m_lSubtitleStreamTotal );
			if ( pstStreamIdList->plSubtitleIdList == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] pstStreamIdList->plSubtitleIdList: 0x%x\n", __LINE__, pstStreamIdList->plSubtitleIdList);
			CDMX_MEMSET( pstStreamIdList->plSubtitleIdList, 0, sizeof(long) * pst_dmx_info->m_lSubtitleStreamTotal );
		}

		for(i = 0; i < pst_dmx_info->m_lSubtitleStreamTotal; i++)
		{
			pst_subtitle_info = (av_dmx_subtitle_info_t*)pby_info_list;
			pstStreamIdList->plSubtitleIdList[i] = pst_subtitle_info->m_lStreamID;

			DSTATUS( " [Subtitle_%03ld] - StreamID: %ld / Language: \"%s\" \n",
						i,
						pst_subtitle_info->m_lStreamID,
						pst_subtitle_info->m_pszLanguage);

			pby_info_list += lSubtitleInfoLength;
		}
	}

	if( lGraphicsInfoLength && pst_dmx_info->m_lGraphicsStreamTotal > 0 )
	{
		av_dmx_graphics_info_t *pst_graphics_info;

		pby_info_list = (unsigned char*)pst_dmx_info->m_pstGraphicsInfoList;

		if ( pstStreamIdList->plGraphicsIdList == NULL )
		{
			pstStreamIdList->plGraphicsIdList = (long *)CDMX_MALLOC( sizeof(long) * pst_dmx_info->m_lGraphicsStreamTotal );
			if ( pstStreamIdList->plGraphicsIdList == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] pstStreamIdList->plGraphicsIdList: 0x%x\n", __LINE__, pstStreamIdList->plGraphicsIdList);
			CDMX_MEMSET( pstStreamIdList->plGraphicsIdList, 0, sizeof(long) * pst_dmx_info->m_lGraphicsStreamTotal );
		}

		for(i = 0; i < pst_dmx_info->m_lGraphicsStreamTotal; i++)
		{
			pst_graphics_info = (av_dmx_graphics_info_t*)pby_info_list;
			pstStreamIdList->plGraphicsIdList[i] = pst_graphics_info->m_lStreamID;

			DSTATUS( " [Graphics_%03ld] - StreamID: %ld / Language: \"%s\" \n",
						i,
						pst_graphics_info->m_lStreamID,
						pst_graphics_info->m_pszLanguage);

			pby_info_list += lGraphicsInfoLength;
		}
	}

	DSTATUS( "===================================================================\n" );
	return 0;
}





//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	Video sequence header extractor
//
#define MPEG4_VOL_STARTCODE_MIN		0x00000120	// MPEG-4 
#define MPEG4_VOL_STARTCODE_MAX		0x0000012F	// MPEG-4 
#define MPEG4_VOP_STARTCODE			0x000001B6	// MPEG-4 
#define MPEG12_SEQHEAD_STARTCODE	0x000001B3	// MPEG-1/2
#define VC1AP_SEQHEAD_STARTCODE		0x0000010F	// VC-1
#define AVS_SEQHEAD_STARTCODE		0x000001B0	// AVS

#define MAX_SEQ_HEADER_ALLOC_SIZE   0x0007D000  // 500KB

static 
int 
cdmx_discard_seqheader (cdmx_seq_header_t* p_gs_stDmxSeqHeader)
{
	// free Sequence Header Buffer
	CDMX_FREE(p_gs_stDmxSeqHeader->m_pSeqHeaderData);  
	return 0;
}

static 
int 
cdmx_extract_normal_seqheader(
    const unsigned char	 *pbyData, 
	long				  lDataSize,
	unsigned char		**ppbySeqHead,
	long				 *plHeadLength,
	unsigned long		  ulSyncword
	)
{
	unsigned long syncword = 0xFFFFFFFF;
	int	start_pos = -1;
	int end_pos = -1;
	int i;

	syncword <<= 8; 
	syncword |= pbyData[0];
	syncword <<= 8; 
	syncword |= pbyData[1];
	syncword <<= 8; 
	syncword |= pbyData[2];

	for(i = 3; i < lDataSize; i++) {
		syncword <<= 8; 
		syncword |= pbyData[i];

		if( (syncword >> 8) == 1 ) {	// 0x 000001??
			if( syncword == ulSyncword )
				start_pos = i-3;
			else if( start_pos >= 0 || *plHeadLength > 0 ) {
				end_pos = i-3;
				break;
			}
		}
	}

	if( start_pos >= 0 ) {
		if( end_pos >= 0 ) {
			*plHeadLength = end_pos-start_pos;
			*ppbySeqHead = CDMX_MALLOC( *plHeadLength );     // allocate memory for sequence header array 
			if ( *ppbySeqHead == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] *ppbySeqHead: 0x%x\n", __LINE__, *ppbySeqHead);
			CDMX_MEMCPY(*ppbySeqHead, pbyData + start_pos, *plHeadLength);
			return 1;
		}
		else {
			*plHeadLength = lDataSize - start_pos;
			*ppbySeqHead = CDMX_MALLOC( *plHeadLength );     // allocate memory for sequence header array 
			if ( *ppbySeqHead == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] *ppbySeqHead: 0x%x\n", __LINE__, *ppbySeqHead);
			CDMX_MEMCPY(*ppbySeqHead, pbyData + start_pos, *plHeadLength);
			return 0;
		}
	}
	else if( *plHeadLength > 0 ) 
	{
		// If we already found the start point of the sequence header in previous frame data, attach new DATA behind it.
		if( end_pos < 0 )
			end_pos = lDataSize;
		
		if ( *plHeadLength + end_pos > MAX_SEQ_HEADER_ALLOC_SIZE ) // check the maximum threshold
			return 0;

		*ppbySeqHead = CDMX_REALLOC(*ppbySeqHead , *plHeadLength + end_pos);     // re-allocate memory for sequence header array 
		if ( *ppbySeqHead == NULL ) {
			ERROR( "[Err:%d] re-malloc failed in %s(%d) \n", CDK_ERROR_REALLOC, __FILE__, __LINE__ );
			return CDK_ERROR_REALLOC;
		}
		DSTATUS("[Line:%04d] *ppbySeqHead: 0x%x\n", __LINE__, *ppbySeqHead);
		CDMX_MEMCPY(*ppbySeqHead + *plHeadLength, pbyData, end_pos);
		*plHeadLength += end_pos;
		return 1;
	}

	return 0;
}

static
int 
cdmx_extract_mpeg4_seqheader(
    const unsigned char	 *pbyData, 
	long				  lDataSize,
	unsigned char		**ppbySeqHead,
	long				 *plHeadLength
	)
{
	unsigned long syncword = 0xFFFFFFFF;
	int	start_pos = -1;
	int end_pos = -1;
	int i;

	syncword <<= 8; 
	syncword |= pbyData[0];
	syncword <<= 8; 
	syncword |= pbyData[1];
	syncword <<= 8; 
	syncword |= pbyData[2];

	for(i = 3; i < lDataSize; i++) {
		syncword <<= 8; 
		syncword |= pbyData[i];

		if( (syncword >> 8) == 1 ) {	// 0x 000001??
			if( syncword >= MPEG4_VOL_STARTCODE_MIN &&
				syncword <= MPEG4_VOL_STARTCODE_MAX )
				start_pos = i-3;
			else if( start_pos >= 0 || *plHeadLength > 0 ) {
                if ( syncword == MPEG4_VOP_STARTCODE )
                {
                    end_pos = i-3;
                    break;
                }
			}
		}
	}

	if( start_pos >= 0 ) {
		if( end_pos >= 0 ) {
			*plHeadLength = end_pos-start_pos;
			*ppbySeqHead = CDMX_MALLOC( *plHeadLength );     // allocate memory for sequence header array 
			if ( *ppbySeqHead == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] *ppbySeqHead: 0x%x\n", __LINE__, *ppbySeqHead);
			CDMX_MEMCPY(*ppbySeqHead, pbyData + start_pos, *plHeadLength);
			return 1;
		}
		else {
			*plHeadLength = lDataSize - start_pos;
			*ppbySeqHead = CDMX_MALLOC( *plHeadLength );     // allocate memory for sequence header array 
			if ( *ppbySeqHead == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] *ppbySeqHead: 0x%x\n", __LINE__, *ppbySeqHead);
			CDMX_MEMCPY(*ppbySeqHead, pbyData + start_pos, *plHeadLength);
			return 0;
		}
	}
	else if( *plHeadLength > 0 ) {
		if( end_pos < 0 )
			end_pos = lDataSize;

		if ( *plHeadLength + end_pos > MAX_SEQ_HEADER_ALLOC_SIZE ) // check the maximum threshold
			return 0;

		*ppbySeqHead = CDMX_REALLOC(*ppbySeqHead , *plHeadLength + end_pos);     // re-allocate memory for sequence header array 
		if ( *ppbySeqHead == NULL ) {
			ERROR( "[Err:%d] re-malloc failed in %s(%d) \n", CDK_ERROR_REALLOC, __FILE__, __LINE__ );
			return CDK_ERROR_REALLOC;
		}
		DSTATUS("[Line:%04d] *ppbySeqHead: 0x%x\n", __LINE__, *ppbySeqHead);
		CDMX_MEMCPY(*ppbySeqHead + *plHeadLength, pbyData, end_pos);
		*plHeadLength += end_pos;
		return 1;
	}

	return 0;
}

static
int 
cdmx_extract_h264_seqheader(
    const unsigned char	 *pbyStreamData, 
	long				  lStreamDataSize,
	unsigned char		**ppbySeqHeaderData,
	long				 *plSeqHeaderSize
	)
{
	long i;
	long l_seq_start_pos = 0, l_seq_end_pos = 0, l_seq_length = 0; // Start Position, End Position, Length of the sequence header
	long l_sps_found = 0;
	long l_pps_found = 0;

	unsigned long ul_read_word_buff;	   	    	            //4 byte temporary buffer 
	unsigned long ul_masking_word_seq          = 0x0FFFFFFF;    //Masking Value for finding H.264 sequence header
	unsigned long ul_masking_word_sync         = 0x00FFFFFF;    //Masking Value for finding sync word of H.264
	unsigned long ul_h264_result_word_seq_SPS  = 0x07010000;    //Masking result should be this value in case of SPS. SPS Sequence header of H.264 must start by "00 00 01 x7"
	unsigned long ul_h264_result_word_seq_PPS  = 0x08010000;    //Masking result should be this value in case of PPS. PPS Sequence header of H.264 must start by "00 00 01 x8"
	unsigned long ul_h264_result_word_sync     = 0x00010000;    //Masking result should be this value. Sequence header of H.264 must start by "00 00 01 x7"

	if ( lStreamDataSize < 4 )
		return 0; // there's no Seq. header in this frame. we need the next frame.

	if ( *plSeqHeaderSize > 0 )
	{
		// we already find the sps, pps in previous frame
		l_sps_found = 1;
		l_pps_found = 1;
		l_seq_start_pos = 0;
	}
	else
	{
		// find the SPS of H.264 
		ul_read_word_buff = 0;
		ul_read_word_buff |= (pbyStreamData[0] << 8);
		ul_read_word_buff |= (pbyStreamData[1] << 16);
		ul_read_word_buff |= (pbyStreamData[2] << 24);

		for ( i = 0; i < lStreamDataSize-4; i++ )      
		{
			ul_read_word_buff = ul_read_word_buff >> 8;
			ul_read_word_buff &= 0x00FFFFFF; 
			ul_read_word_buff |= (pbyStreamData[i+3] << 24);

			if ( (ul_read_word_buff & ul_masking_word_seq) == ul_h264_result_word_seq_SPS ) 
			{
				// SPS Sequence Header has been detected
				l_sps_found = 1;              
				l_seq_start_pos = i;          // save the start position of the sequence header 

				break;                        
			}

			// Continue to find the sps in next loop
		}

		if ( l_sps_found == 1 )
		{
			// Now, let's start to find the PPS of the Seq. header.

			i = i + 4; 
			ul_read_word_buff = 0;
			ul_read_word_buff |= (pbyStreamData[i] << 8);
			ul_read_word_buff |= (pbyStreamData[i+1] << 16);
			ul_read_word_buff |= (pbyStreamData[i+2] << 24);

			for (  ; i < lStreamDataSize - 4; i++ )    
			{
				ul_read_word_buff = ul_read_word_buff >> 8;
				ul_read_word_buff &= 0x00FFFFFF; 
				ul_read_word_buff |= (pbyStreamData[i+3] << 24);

				if ( (ul_read_word_buff & ul_masking_word_seq) == ul_h264_result_word_seq_PPS ) 
				{
					// PPS has been detected. 
					l_pps_found = 1;

					break;
				}

				// Continue to find the pps in next loop
			}
		}
	}

	if ( l_pps_found == 1 )
	{
		// Now, let's start to find the next sync word to find the end position of Seq. Header

		if ( *plSeqHeaderSize > 0 )
			i = 0;     // we already find the sps, pps in previous frame
		else
			i = i + 4;
		ul_read_word_buff = 0;
		ul_read_word_buff |= (pbyStreamData[i] << 8);
		ul_read_word_buff |= (pbyStreamData[i+1] << 16);
		ul_read_word_buff |= (pbyStreamData[i+2] << 24);

		for ( ; i < lStreamDataSize - 4; i++ )    
		{
			ul_read_word_buff = ul_read_word_buff >> 8;
			ul_read_word_buff &= 0x00FFFFFF; 
			ul_read_word_buff |= (pbyStreamData[i+3] << 24);

			if ( (ul_read_word_buff & ul_masking_word_sync) == ul_h264_result_word_sync ) 
			{
				long l_cnt_zeros = 0;       // to count extra zeros ahead of "00 00 01"

				// next sync-word has been found.
				l_seq_end_pos = i - 1;      // save the end position of the sequence header (00 00 01 case)

				// any zeros can be added ahead of "00 00 01" sync word by H.264 specification. Count the number of these leading zeros.
				while (1)
				{
					l_cnt_zeros++;
					if ( pbyStreamData[i-l_cnt_zeros] == 0 )    
					{
						l_seq_end_pos = l_seq_end_pos -1;    // decrease the end position of Seq. Header by 1.
					}
					else
						break;
				}
				
				if ( *plSeqHeaderSize > 0 )
				{
					// we already find the sps, pps in previous frame
					l_seq_length = l_seq_end_pos - l_seq_start_pos + 1;       

					if ( l_seq_length > 0 )
					{
						if ( *plSeqHeaderSize + l_seq_length > MAX_SEQ_HEADER_ALLOC_SIZE ) // check the maximum threshold
							return 0;

						*ppbySeqHeaderData = CDMX_REALLOC(*ppbySeqHeaderData , *plSeqHeaderSize + l_seq_length );     // allocation memory for sequence header array (must free this at the CLOSE step)
						if ( *ppbySeqHeaderData == NULL ) {
							ERROR( "[Err:%d] re-malloc failed in %s(%d) \n", CDK_ERROR_REALLOC, __FILE__, __LINE__ );
							return CDK_ERROR_REALLOC;
						}
						DSTATUS("[Line:%04d] *ppbySeqHeaderData: 0x%x\n", __LINE__, *ppbySeqHeaderData);
						CDMX_MEMCPY( (unsigned char*) (*ppbySeqHeaderData) + *plSeqHeaderSize , &pbyStreamData[l_seq_start_pos], l_seq_length);   // save the seq. header to array
						*plSeqHeaderSize = *plSeqHeaderSize + l_seq_length;
					}
					
					return 1;
					
				}
				else
				{
					// calculate the length of the sequence header
					l_seq_length = l_seq_end_pos - l_seq_start_pos + 1;       

					if ( l_seq_length > 0 )
					{
						*ppbySeqHeaderData = CDMX_MALLOC( l_seq_length );     // allocation memory for sequence header array (must free this at the CLOSE step)
						if ( *ppbySeqHeaderData == NULL ) {
							ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
							return CDK_ERR_MALLOC;
						}
						DSTATUS("[Line:%04d] *ppbySeqHeaderData: 0x%x\n", __LINE__, *ppbySeqHeaderData);
						CDMX_MEMCPY( (unsigned char*) (*ppbySeqHeaderData), &pbyStreamData[l_seq_start_pos], l_seq_length);   // save the seq. header to array
						*plSeqHeaderSize = l_seq_length;

						return 1;  // We've found the sequence header successfully
					}
				}
			}

			// Continue to find the sync-word in next loop
		}
	}

	if ( l_sps_found == 1 && l_pps_found == 1)
	{
		// we found sps and pps, but we couldn't find the next sync word yet
		l_seq_end_pos = lStreamDataSize - 1;
		l_seq_length = l_seq_end_pos - l_seq_start_pos + 1;        // calculate the length of the sequence header

		if ( *plSeqHeaderSize > 0 )
		{
			// we already saved the sps, pps in previous frame
			if ( l_seq_length > 0 )
			{
				if ( *plSeqHeaderSize + l_seq_length > MAX_SEQ_HEADER_ALLOC_SIZE )     // check the maximum threshold
					return 0;

				*ppbySeqHeaderData = CDMX_REALLOC(*ppbySeqHeaderData , *plSeqHeaderSize + l_seq_length );     // allocate memory for sequence header array (must free this at the CLOSE step)
				if ( *ppbySeqHeaderData == NULL ) {
					ERROR( "[Err:%d] re-malloc failed in %s(%d) \n", CDK_ERROR_REALLOC, __FILE__, __LINE__ );
					return CDK_ERROR_REALLOC;
				}
				DSTATUS("[Line:%04d] *ppbySeqHeaderData: 0x%x\n", __LINE__, *ppbySeqHeaderData);
				CDMX_MEMCPY( (unsigned char*) (*ppbySeqHeaderData) + *plSeqHeaderSize , &pbyStreamData[l_seq_start_pos], l_seq_length);   // save the seq. header to array
				*plSeqHeaderSize = *plSeqHeaderSize + l_seq_length;
			}

		}
		else
		{
			*ppbySeqHeaderData = CDMX_MALLOC( l_seq_length );           // allocate memory for sequence header array (must free this at the CLOSE step)
			if ( *ppbySeqHeaderData == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] *ppbySeqHeaderData: 0x%x\n", __LINE__, *ppbySeqHeaderData);
			CDMX_MEMCPY( (unsigned char*) (*ppbySeqHeaderData), &pbyStreamData[l_seq_start_pos], l_seq_length);   // save the seq. header to array
			*plSeqHeaderSize = *plSeqHeaderSize + l_seq_length;
		}
	}

	return 0; // We couldn't find the complete sequence header yet. We need to search the next frame data.
}

static
int 
cdmx_extract_vc1AP_seqheader(
    const unsigned char	 *pbyStreamData, 
	long				  lStreamDataSize,
	unsigned char		**ppbySeqHeaderData,
	long				 *plSeqHeaderSize
	)
{
	long i;
	long l_seq_start_pos = 0, l_seq_end_pos = 0, l_seq_length = 0;	// Start Position, End Position, Length of the sequence header
	long l_ssc_found = 0;
	long l_epc_found = 0;

	unsigned long ul_read_word_buff;							//4 byte temporary buffer 
	unsigned long ul_masking_word_seq           = 0x0FFFFFFF;	//Masking Value for finding VC1 AP sequence header
	unsigned long ul_masking_word_sync          = 0x0FFFFFFF;	//Masking Value for finding sync word of VC1 AP
	unsigned long ul_vc1AP_result_word_seq_SC   = 0x0f010000;	//Masking result should be this value in case of SC. SC Sequence header of VC1 AP must start by "00 00 01 xF"
	unsigned long ul_vc1AP_result_word_seq_EPC  = 0x0e010000;	//Masking result should be this value in case of EPC. EPC Sequence header of VC1 AP must start by "00 00 01 xE"
	unsigned long ul_vc1AP_result_word_sync     = 0x0d010000;	//Masking result should be this value. Sequence header of VC1 AP must start by "00 00 01 0D"

	if ( lStreamDataSize < 4 )
		return 0; // there's no Seq. header in this frame. we need the next frame.

	if ( *plSeqHeaderSize > 0 )
	{
		// we already find the sc, epc in previous frame
		l_ssc_found = 1;
		l_epc_found = 1;
		l_seq_start_pos = 0;
	}
	else
	{
		// find the SC of VC1 AP 
		ul_read_word_buff = 0;
		ul_read_word_buff |= (pbyStreamData[0] << 8);
		ul_read_word_buff |= (pbyStreamData[1] << 16);
		ul_read_word_buff |= (pbyStreamData[2] << 24);

		for ( i = 0; i < lStreamDataSize-4; i++ )      
		{
			ul_read_word_buff = ul_read_word_buff >> 8;
			ul_read_word_buff &= 0x00FFFFFF; 
			ul_read_word_buff |= (pbyStreamData[i+3] << 24);

			if ( (ul_read_word_buff & ul_masking_word_seq) == ul_vc1AP_result_word_seq_SC ) 
			{
				// SC Sequence Header has been detected
				l_ssc_found = 1;              
				l_seq_start_pos = i;          // save the start position of the sequence header 
				break;                        
			}
			// Continue to find the sc in next loop
		}

		if ( l_ssc_found == 1 )
		{
			// Now, let's start to find the EPC of the Seq. header.

			i = i + 4; 
			ul_read_word_buff = 0;
			ul_read_word_buff |= (pbyStreamData[i] << 8);
			ul_read_word_buff |= (pbyStreamData[i+1] << 16);
			ul_read_word_buff |= (pbyStreamData[i+2] << 24);

			for (  ; i < lStreamDataSize - 4; i++ )    
			{
				ul_read_word_buff = ul_read_word_buff >> 8;
				ul_read_word_buff &= 0x00FFFFFF; 
				ul_read_word_buff |= (pbyStreamData[i+3] << 24);

				if ( (ul_read_word_buff & ul_masking_word_seq) == ul_vc1AP_result_word_seq_EPC ) 
				{
					// EPC has been detected. 
					l_epc_found = 1;
					break;
				}
				// Continue to find the epc in next loop
			}
		}
	}

	if ( l_epc_found == 1 )
	{
		// Now, let's start to find the next sync word to find the end position of Seq. Header

		if ( *plSeqHeaderSize > 0 )
			i = 0;     // we already find the sc, epc in previous frame
		else
			i = i + 4;
		ul_read_word_buff = 0;
		ul_read_word_buff |= (pbyStreamData[i] << 8);
		ul_read_word_buff |= (pbyStreamData[i+1] << 16);
		ul_read_word_buff |= (pbyStreamData[i+2] << 24);

		for ( ; i < lStreamDataSize - 4; i++ )    
		{
			ul_read_word_buff = ul_read_word_buff >> 8;
			ul_read_word_buff &= 0x00FFFFFF; 
			ul_read_word_buff |= (pbyStreamData[i+3] << 24);

			if ( (ul_read_word_buff & ul_masking_word_sync) == ul_vc1AP_result_word_sync ) 
			{
				long l_cnt_zeros = 0;       // to count extra zeros ahead of "00 00 01"

				// next sync-word has been found.
				l_seq_end_pos = i - 1;      // save the end position of the sequence header (00 00 01 case)

				// any zeros can be added ahead of "00 00 01" sync word by VC1 AP specification. Count the number of these leading zeros.
				while (1)
				{
					l_cnt_zeros++;
					if ( pbyStreamData[i-l_cnt_zeros] == 0 )    
					{
						l_seq_end_pos = l_seq_end_pos -1;    // decrease the end position of Seq. Header by 1.
					}
					else
						break;
				}
				
				if ( *plSeqHeaderSize > 0 )
				{
					// we already find the sc, epc in previous frame
					l_seq_length = l_seq_end_pos - l_seq_start_pos + 1;       

					if ( l_seq_length > 0 )
					{
						if ( *plSeqHeaderSize + l_seq_length > MAX_SEQ_HEADER_ALLOC_SIZE ) // check the maximum threshold
							return 0;
						*ppbySeqHeaderData = CDMX_REALLOC(*ppbySeqHeaderData , *plSeqHeaderSize + l_seq_length );     // allocation memory for sequence header array (must free this at the CLOSE step)
						if ( *ppbySeqHeaderData == NULL ) {
							ERROR( "[Err:%d] re-malloc failed in %s(%d) \n", CDK_ERROR_REALLOC, __FILE__, __LINE__ );
							return CDK_ERROR_REALLOC;
						}
						DSTATUS("[Line:%04d] *ppbySeqHeaderData: 0x%x\n", __LINE__, *ppbySeqHeaderData);
						CDMX_MEMCPY( (unsigned char*) (*ppbySeqHeaderData) + *plSeqHeaderSize , &pbyStreamData[l_seq_start_pos], l_seq_length);   // save the seq. header to array
						*plSeqHeaderSize = *plSeqHeaderSize + l_seq_length;
					}
					return 1;
				}
				else
				{
					// calculate the length of the sequence header
					l_seq_length = l_seq_end_pos - l_seq_start_pos + 1;       

					if ( l_seq_length > 0 )
					{
						*ppbySeqHeaderData = CDMX_MALLOC( l_seq_length );     // allocation memory for sequence header array (must free this at the CLOSE step)
						if ( *ppbySeqHeaderData == NULL ) {
							ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
							return CDK_ERR_MALLOC;
						}
						DSTATUS("[Line:%04d] *ppbySeqHeaderData: 0x%x\n", __LINE__, *ppbySeqHeaderData);
						CDMX_MEMCPY( (unsigned char*) (*ppbySeqHeaderData), &pbyStreamData[l_seq_start_pos], l_seq_length);   // save the seq. header to array
						*plSeqHeaderSize = l_seq_length;
						return 1;  // We've found the sequence header successfully
					}
				}
			}
			// Continue to find the sync-word in next loop
		}
	}

	if ( l_ssc_found == 1 && l_epc_found == 1)
	{
		// we found sc and epc, but we couldn't find the next sync word yet
		l_seq_end_pos = lStreamDataSize - 1;
		l_seq_length = l_seq_end_pos - l_seq_start_pos + 1;        // calculate the length of the sequence header

		if ( *plSeqHeaderSize > 0 )
		{
			// we already saved the sc, epc in previous frame
			if ( l_seq_length > 0 )
			{
				if ( *plSeqHeaderSize + l_seq_length > MAX_SEQ_HEADER_ALLOC_SIZE )     // check the maximum threshold
					return 0;
				*ppbySeqHeaderData = CDMX_REALLOC(*ppbySeqHeaderData , *plSeqHeaderSize + l_seq_length );     // allocate memory for sequence header array (must free this at the CLOSE step)
				if ( *ppbySeqHeaderData == NULL ) {
					ERROR( "[Err:%d] re-malloc failed in %s(%d) \n", CDK_ERROR_REALLOC, __FILE__, __LINE__ );
					return CDK_ERROR_REALLOC;
				}
				DSTATUS("[Line:%04d] *ppbySeqHeaderData: 0x%x\n", __LINE__, *ppbySeqHeaderData);
				CDMX_MEMCPY( (unsigned char*) (*ppbySeqHeaderData) + *plSeqHeaderSize , &pbyStreamData[l_seq_start_pos], l_seq_length);   // save the seq. header to array
				*plSeqHeaderSize = *plSeqHeaderSize + l_seq_length;
			}

		}
		else
		{
			*ppbySeqHeaderData = CDMX_MALLOC( l_seq_length );           // allocate memory for sequence header array (must free this at the CLOSE step)
			if ( *ppbySeqHeaderData == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] *ppbySeqHeaderData: 0x%x\n", __LINE__, *ppbySeqHeaderData);
			CDMX_MEMCPY( (unsigned char*) (*ppbySeqHeaderData), &pbyStreamData[l_seq_start_pos], l_seq_length);   // save the seq. header to array
			*plSeqHeaderSize = *plSeqHeaderSize + l_seq_length;
		}
	}

	return 0; // We couldn't find the complete sequence header yet. We need to search the next frame data.
}

int // -1: not implemented, 0: none, 1: extracted / implemented
cdmx_get_seqhead(
    const unsigned char	 *pbyData, 
	long				  lDataSize,
	unsigned char		**ppbySeqHead,
	long				 *plHeadLength,
	unsigned long		  ulFourCC
	)
{
	switch( ulFourCC ) 
	{
		/* MPEG-4 AVC / H.264 */
	case FOURCC_avc1: case FOURCC_AVC1:
	case FOURCC_h264: case FOURCC_H264:
	case FOURCC_x264: case FOURCC_X264:
	case FOURCC_vssh: case FOURCC_VSSH:
	case FOURCC_davc: case FOURCC_DAVC:
		if( pbyData == NULL ) 
		{
			return 1;
		}
		else
			return cdmx_extract_h264_seqheader(pbyData, lDataSize, ppbySeqHead, plHeadLength);

		/* H.263 (not implemented) */
	case FOURCC_s263: case FOURCC_h263:
	case FOURCC_S263: case FOURCC_H263:
		break;

		/* Sorenson H.264 (not implemented) */
	case FOURCC_flv1: case FOURCC_FLV1:
		break;

		/* MPEG-4 */
	case FOURCC_mp4v: case FOURCC_MP4V:
	case FOURCC_SEDG:
	case FOURCC_RMP4:
	case FOURCC_xvid: case FOURCC_XVID: case FOURCC_Xvid:
	case FOURCC_divx: case FOURCC_DIVX:	
	case FOURCC_dx50: case FOURCC_DX50:
	case FOURCC_fmd4: case FOURCC_FMD4:
	case FOURCC_3iv2: case FOURCC_3IV2:
	case FOURCC_fvfw: case FOURCC_FVFW:
	case FOURCC_divf: case FOURCC_DIVF:
	case FOURCC_fmp4: case FOURCC_FMP4:
	case FOURCC_rmp4: case FOURCC_XviD:
	case FOURCC_DVX1: case FOURCC_dvx1:
	case FOURCC_MP4S: case FOURCC_mp4s:
	case FOURCC_M4S2: case FOURCC_m4s2:
		if( pbyData == NULL ) 
		{
			return 1;
		}
		return cdmx_extract_mpeg4_seqheader(pbyData, lDataSize, ppbySeqHead, plHeadLength);

		/* MPEG-1/2 */
	case FOURCC_mpeg: case FOURCC_MPEG:
	case FOURCC_MP2V: case FOURCC_mp2v:
	case FOURCC_MPG2: case FOURCC_mpg2: 
	case FOURCC_mpg1:
	case FOURCC_m2v1: case FOURCC_hdv1:
	case FOURCC_m1v1: 
		if( pbyData == NULL ) 
		{
			return 1;
		}
		return cdmx_extract_normal_seqheader(pbyData, lDataSize, ppbySeqHead, plHeadLength, MPEG12_SEQHEAD_STARTCODE);

		/* VC-1 Simple/Main Profile (not implemented) */
	case FOURCC_WMV3: case FOURCC_wmv3:
		break;

		/* VC-1 Advanced Profile */
	case FOURCC_WVC1: case FOURCC_wvc1:
	case FOURCC_WMVA: case FOURCC_wmva:
	case FOURCC_VC1:  case FOURCC_vc1:
		if( pbyData == NULL ) 
		{
			return 1;
		}
		return cdmx_extract_vc1AP_seqheader(pbyData, lDataSize, ppbySeqHead, plHeadLength);

		/* WMV-8 (not implemented) */
	case FOURCC_WMV2: case FOURCC_wmv2:
		break;

		/* WMV-7 (not implemented) */
	case FOURCC_WMV1: case FOURCC_wmv1:
		break;

		/* DIVX-3 (not implemented) */
	case FOURCC_div3: case FOURCC_DIV3:
	case FOURCC_div4: case FOURCC_DIV4:
	case FOURCC_mp43: case FOURCC_MP43:
		break;

		/* RV (not implemented) */
	case FOURCC_rv30: case FOURCC_RV30: 
	case FOURCC_rv40: case FOURCC_RV40: case FOURCC_RV89COMBO: 
		break;

		/* Moiton JPEG (not implemented) */
	case FOURCC_MJPG: case FOURCC_mjpg:
	case FOURCC_IJPG:
	case FOURCC_AVRn:
	case FOURCC_jpeg:
		break;

		/* AVS */
	case FOURCC_AVS: case FOURCC_avs:
	case FOURCC_cavs:
		if( pbyData == NULL )
		{
			return 1;
		}
		return cdmx_extract_normal_seqheader(pbyData, lDataSize, ppbySeqHead, plHeadLength, AVS_SEQHEAD_STARTCODE);

		/* unknown standard */
	case FOURCC_mpg4: case FOURCC_MPG4:
	case FOURCC_col1: case FOURCC_COL1:
	case FOURCC_mp42: case FOURCC_MP42:
	default:
		// not supported
		break;
	}

	// not implemented
	//return -1;
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//  for VPU
//

static inline 
void 
cdmx_put_le32(
   unsigned char *pbyOut,
   unsigned long  ulInpData
   )
{
	*(pbyOut+0) = (unsigned char)((ulInpData)>> 0);
	*(pbyOut+1) = (unsigned char)((ulInpData)>> 8);
	*(pbyOut+2) = (unsigned char)((ulInpData)>>16);
	*(pbyOut+3) = (unsigned char)((ulInpData)>>24);
}

static inline 
void 
cdmx_put_le16(
   unsigned char *pbyOut,
   unsigned long  ulInpData
   )
{
	*(pbyOut+0) = (unsigned char)((ulInpData)>> 0);
	*(pbyOut+1) = (unsigned char)((ulInpData)>> 8);
}

static 
int
set_vpuc7_startcode( 
    long			 bIsNotFirstFrame,
	unsigned long 	 ulWidth,
	unsigned long 	 ulHeight,
	unsigned long	 ulStreamDataSize,
	unsigned long 	 ulCodec,
	unsigned char 	*pbyOutData,
	unsigned long	*pulOutDataSize
	)
{
	unsigned char *pby_buff = pbyOutData;

	if(pbyOutData == NULL)
		return CDK_ERR_MALLOC;

	if( bIsNotFirstFrame )
	{
		//STEP( "Not the first frame\n" );
		*pulOutDataSize = 12;
	}
	else
	{
		FUNC( "In set_vpuc7_startcode\n" )

		// Make Seq Header
		// Only for the first frame
		*pulOutDataSize = 44;

		if( ulCodec == CDMX_VCODEC_DIV3 )
		{
			STEP( "Make Seq Header for DivX3\n" );

			*pby_buff++ = (unsigned char)'C';
			*pby_buff++ = (unsigned char)'N';
			*pby_buff++ = (unsigned char)'M';
			*pby_buff++ = (unsigned char)'V';
			cdmx_put_le16(pby_buff, 0x00); pby_buff += 2;
			cdmx_put_le16(pby_buff, 0x20); pby_buff += 2;
			*pby_buff++ = (unsigned char)'D';
			*pby_buff++ = (unsigned char)'I';
			*pby_buff++ = (unsigned char)'V';
			*pby_buff++ = (unsigned char)'3';
		}
		else if( ulCodec == CDMX_VCODEC_VP8 )
		{
			STEP( "Make Seq Header for VP8\n" );

			*pby_buff++ = (unsigned char)'D';
			*pby_buff++ = (unsigned char)'K';
			*pby_buff++ = (unsigned char)'I';
			*pby_buff++ = (unsigned char)'F';
			cdmx_put_le16(pby_buff, 0x00); pby_buff += 2;
			cdmx_put_le16(pby_buff, 0x20); pby_buff += 2;
			*pby_buff++ = (unsigned char)'V';
			*pby_buff++ = (unsigned char)'P';
			*pby_buff++ = (unsigned char)'8';
			*pby_buff++ = (unsigned char)'0';
		}
		cdmx_put_le16(pby_buff, ulWidth);	pby_buff += 2;	
		cdmx_put_le16(pby_buff, ulHeight);	pby_buff += 2;
		cdmx_put_le32(pby_buff, 30);	 	pby_buff += 4;
		cdmx_put_le32(pby_buff, 1);		 	pby_buff += 4;
		cdmx_put_le32(pby_buff, 3000);	 	pby_buff += 4;
		cdmx_put_le32(pby_buff, 0);		 	pby_buff += 4;
	}

	// Make Pic Header
	// For all frames
	cdmx_put_le32(pby_buff, ulStreamDataSize);	pby_buff += 4;
	cdmx_put_le32(pby_buff, 0);	pby_buff += 4;
	cdmx_put_le32(pby_buff, 0);	pby_buff += 4;

	return 0;
}

static 
long
set_first_frame_with_seqheader_vc1_sp_mp( 
    unsigned char	 	 *pbySeqHeaderData,
	unsigned long  	  	  ulSeqHeaderSize,
	unsigned char 	 	 *pbyStreamData,		// [in]
	unsigned long  	  	 *pulStreamSize,		// [in, out]
	unsigned char 	 	 *pbyOutStreamData,		// [out]
	unsigned char		**ppTempBuff,
	unsigned long  	 	 *pulTempBuffSize,
	unsigned long  	 	  ulStreamTimeStamp,
	av_dmx_video_info_t	 *pstVideoInfo
	)
{
	unsigned long width, height, rcv_additional_header_size=12, bitrate=0, framerate;
	unsigned long frame_len_rcv;
	unsigned long pre_roll = 0xf0000000;
	unsigned long rcv_version = 0xc5000000;

	unsigned long offset = 0;
	unsigned long cpylen = 0;

	FUNC( "In set_first_frame_with_seqheader_vc1_sp_mp\n" );
	if( *ppTempBuff == NULL )
	{
		*ppTempBuff = CDMX_MALLOC( CDMX_MAX_FRAME_LEN/*buf_size*/ );
		if ( *ppTempBuff == NULL ) {
			ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
			return CDK_ERR_MALLOC;
		}
		DSTATUS("[Line:%04d] *ppTempBuff: 0x%x\n", __LINE__, *ppTempBuff);
		*pulTempBuffSize = CDMX_MAX_FRAME_LEN;
		CDMX_MEMSET( *ppTempBuff, 0, CDMX_MAX_FRAME_LEN );
	}
	else
	{
		unsigned long max_size = 40 + *pulStreamSize + ulSeqHeaderSize;

		if ( *pulTempBuffSize < max_size ) 
		{
			*pulTempBuffSize = max_size;
			CDMX_FREE(*ppTempBuff);
			*ppTempBuff = CDMX_MALLOC( *pulTempBuffSize );
			if ( *ppTempBuff == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] *ppTempBuff: 0x%x\n", __LINE__, *ppTempBuff);
			CDMX_MEMSET( *ppTempBuff, 0, *pulTempBuffSize );
		}
	}

	width     = pstVideoInfo->m_lWidth;
	height    = pstVideoInfo->m_lHeight;
	framerate = pstVideoInfo->m_lFrameRate;
	frame_len_rcv = *pulStreamSize;
	frame_len_rcv |= 0x80000000;

	// [0] rcv_version
	cpylen = 4;
	CDMX_MEMCPY( *ppTempBuff+offset, &rcv_version,		cpylen ); offset += cpylen;
	// [4] size of seqheader 
 	cpylen = 4;
	CDMX_MEMCPY( *ppTempBuff+offset, &ulSeqHeaderSize,	cpylen ); offset += cpylen;
	// [8] seqheader 
	cpylen = ulSeqHeaderSize;
	CDMX_MEMCPY( *ppTempBuff+offset, pbySeqHeaderData,	cpylen ); offset += cpylen;
	// [] height
	cpylen = 4;
	CDMX_MEMCPY( *ppTempBuff+offset, &height, 			cpylen ); offset += cpylen;
	// [] width
	cpylen = 4;
	CDMX_MEMCPY( *ppTempBuff+offset, &width, 			cpylen ); offset += cpylen;
	// [] rcv_additional_header_size
	cpylen = 4;
	CDMX_MEMCPY( *ppTempBuff+offset, &rcv_additional_header_size, cpylen ); offset += cpylen;
	// [] pre_roll
	cpylen = 4;
	CDMX_MEMCPY( *ppTempBuff+offset, &pre_roll, 	cpylen ); offset += cpylen;
	// [] bitrate
	cpylen = 4;
	CDMX_MEMCPY( *ppTempBuff+offset, &bitrate, 	cpylen ); offset += cpylen;
	// [] framerate
	cpylen = 4;
	CDMX_MEMCPY( *ppTempBuff+offset, &framerate,	cpylen ); offset += cpylen;
	// [] frame_len_rcv
	cpylen = 4;
	CDMX_MEMCPY( *ppTempBuff+offset, &frame_len_rcv, cpylen ); offset += cpylen;
	// [] timestamp
	cpylen = 4;
	CDMX_MEMCPY( *ppTempBuff+offset, &ulStreamTimeStamp, cpylen ); offset += cpylen;
	// [] p_buf
	cpylen = *pulStreamSize;
	CDMX_MEMCPY( *ppTempBuff+offset, pbyStreamData, cpylen ); offset += cpylen;

	// update data & size
	*pulStreamSize = offset;
	CDMX_MEMCPY( pbyOutStreamData, *ppTempBuff, *pulStreamSize );

	return 0;
}

static 
long 
set_first_frame_with_seqheader_vc1_ap(
    unsigned char	 	 *pbySeqHeaderData,
	unsigned long  	  	  ulSeqHeaderSize,
	unsigned char 	 	 *pbyStreamData,		// [in]
	unsigned long  	  	 *pulStreamSize,		// [in, out]
	unsigned char 	 	 *pbyOutStreamData,		// [out]
	unsigned char		**ppTempBuff,
	unsigned long  	 	 *pulTempBuffSize
	)
{
	FUNC( "In set_first_frame_with_seqheader_vc1_ap\n" );
	if( *ppTempBuff == NULL )
	{
		*ppTempBuff = CDMX_MALLOC( CDMX_MAX_FRAME_LEN/*buf_size*/ );
		if ( *ppTempBuff == NULL ) {
			ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
			return CDK_ERR_MALLOC;
		}
		DSTATUS("[Line:%04d] *ppTempBuff: 0x%x\n", __LINE__, *ppTempBuff);
		*pulTempBuffSize = CDMX_MAX_FRAME_LEN;
		CDMX_MEMSET( *ppTempBuff, 0, CDMX_MAX_FRAME_LEN );
	}
	else
	{
		unsigned long max_size = 4 + *pulStreamSize + (ulSeqHeaderSize-1);

		if ( *pulTempBuffSize < max_size ) 
		{
			*pulTempBuffSize = max_size;
			CDMX_FREE(*ppTempBuff);
			*ppTempBuff = CDMX_MALLOC( *pulTempBuffSize );
			if ( *ppTempBuff == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] *ppTempBuff: 0x%x\n", __LINE__, *ppTempBuff);
			CDMX_MEMSET( *ppTempBuff, 0, *pulTempBuffSize );
		}
	}

	ulSeqHeaderSize --;
	CDMX_MEMCPY( *ppTempBuff, pbySeqHeaderData+1, ulSeqHeaderSize );

	if ( pbyStreamData[0]==0 && pbyStreamData[1]==0 && pbyStreamData[2]==1 )
	{
		CDMX_MEMCPY( *ppTempBuff+ulSeqHeaderSize, pbyStreamData, *pulStreamSize );
		*pulStreamSize = ulSeqHeaderSize + *pulStreamSize;
	}
	else
	{
		unsigned long ul_start_code = 0x0d010000;
		CDMX_MEMCPY( *ppTempBuff+ulSeqHeaderSize, 	&ul_start_code, 4 );
		CDMX_MEMCPY( *ppTempBuff+ulSeqHeaderSize+4, 	 pbyStreamData, *pulStreamSize );
		*pulStreamSize = ulSeqHeaderSize+4 + *pulStreamSize;
	}

	CDMX_MEMCPY( pbyOutStreamData, *ppTempBuff, *pulStreamSize );

	return 0;
}


static
inline void 
cdmx_write_be32( unsigned char *pbyOut, unsigned long val )
{
	pbyOut[0] = val>>24;
	pbyOut[1] = val>>16;
	pbyOut[2] = val>> 8;
	pbyOut[3] = val;
}

static 
long
set_first_frame_with_seqheader_rv(
    unsigned char	 	 *pbySeqHeaderData,
	unsigned long  	  	  ulSeqHeaderSize,
	unsigned char 	 	 *pbyStreamData,		// [in]
	unsigned long  	  	 *pulStreamSize,		// [in, out]
	unsigned char 	 	 *pbyOutStreamData,		// [out]
    unsigned char		**ppTempBuff,
	unsigned long  	 	 *pulTempBuffSize
	)
{
	unsigned long ul_num_packets = 0;
	unsigned long ul_num_segment = 0;
	unsigned long ul_val = 0;
	unsigned long offset = 0;
	unsigned long i = 0;

	FUNC( "In set_first_frame_with_seqheader_rv\n" );
	if( *ppTempBuff == NULL )
	{
		*ppTempBuff = CDMX_MALLOC( CDMX_MAX_FRAME_LEN/*buf_size*/ );
		if ( *ppTempBuff == NULL ) {
			ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
			return CDK_ERR_MALLOC;
		}
		DSTATUS("[Line:%04d] *ppTempBuff: 0x%x\n", __LINE__, *ppTempBuff);
		*pulTempBuffSize = CDMX_MAX_FRAME_LEN;
		CDMX_MEMSET( *ppTempBuff, 0, CDMX_MAX_FRAME_LEN );
	}
	else
	{
		unsigned long max_size = *pulStreamSize + ulSeqHeaderSize;

		if ( *pulTempBuffSize < max_size ) 
		{
			*pulTempBuffSize = max_size;
			CDMX_FREE(*ppTempBuff);
			*ppTempBuff = CDMX_MALLOC( *pulTempBuffSize );
			if ( *ppTempBuff == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] *ppTempBuff: 0x%x\n", __LINE__, *ppTempBuff);
			CDMX_MEMSET( *ppTempBuff, 0, *pulTempBuffSize );
		}
	}

	// [1]
	CDMX_MEMCPY(*ppTempBuff, pbySeqHeaderData, ulSeqHeaderSize); offset += ulSeqHeaderSize;

	// [2]
	ul_num_packets = pbyStreamData[0] + 1;
	ul_num_segment = 2*ul_num_packets;
	ul_val = *pulStreamSize - (ul_num_segment*4 + 1);
	cdmx_write_be32(*ppTempBuff+offset, ul_val); offset += 4;

	// [3]
	CDMX_MEMSET(*ppTempBuff+offset, 0, 12); offset += 12;

	// [4]
	ul_val = ul_num_packets;
	cdmx_write_be32(*ppTempBuff+offset, ul_val); offset += 4;

	// [5]
	for(i=0; i< ul_num_segment; i++)	//segment : valid + offset
	{
		cdmx_write_be32( *ppTempBuff+offset+(i*4), *( pbyStreamData+1+(i*4) ) ); offset += 4;
	}
	CDMX_MEMCPY( *ppTempBuff+offset, pbyStreamData+1+(i*4), (*pulStreamSize-1+(i*4)) ); offset += *pulStreamSize-1+(i*4);

	*pulStreamSize = offset;
	CDMX_MEMCPY( pbyOutStreamData, *ppTempBuff, *pulStreamSize );
	return 0;
}

static 
long
set_first_frame_with_seqheader_avc(
    unsigned char	 	 *pbySeqHeaderData,
	unsigned long  	  	  ulSeqHeaderSize,
	unsigned char 	 	 *pbyStreamData,		// [in]
	unsigned long  	  	 *pulStreamSize,		// [in, out]
	unsigned char 	 	 *pbyOutStreamData,		// [out]
    unsigned char		**ppTempBuff,
	unsigned long  	 	 *pulTempBuffSize,
	long  	 	  		  lNalLengthSize
	)
{
	long i = 0;
	unsigned char *pby_sour = NULL;
	unsigned char *pby_dest = NULL;
	unsigned long sour_size = 0;
	unsigned long sour_idx = 0;
	unsigned long dest_idx = 0;

	FUNC( "In set_first_frame_with_seqheader_avc\n" );
	if( *ppTempBuff == NULL )
	{
		*ppTempBuff = CDMX_MALLOC( CDMX_MAX_FRAME_LEN/*buf_size*/ );
		if ( *ppTempBuff == NULL ) {
			ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
			return CDK_ERR_MALLOC;
		}
		DSTATUS("[Line:%04d] *ppTempBuff: 0x%x\n", __LINE__, *ppTempBuff);
		*pulTempBuffSize = CDMX_MAX_FRAME_LEN;
		CDMX_MEMSET( *ppTempBuff, 0, CDMX_MAX_FRAME_LEN );
	}
	else
	{
		unsigned long max_size = ulSeqHeaderSize + *pulStreamSize;

		if ( *pulTempBuffSize < max_size ) 
		{
			*pulTempBuffSize = max_size;
			CDMX_FREE(*ppTempBuff);
			*ppTempBuff = CDMX_MALLOC( *pulTempBuffSize );
			if ( *ppTempBuff == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] *ppTempBuff: 0x%x\n", __LINE__, *ppTempBuff);
			CDMX_MEMSET( *ppTempBuff, 0, *pulTempBuffSize );
		}
	}

	if( lNalLengthSize < 0) 
	{
		ALOGE( "[Err:%d] Invalid NALsize detected in %s(%d) \n", CDK_ERR_NAL_SIZE, __FILE__, __LINE__ );
		return CDK_ERR_NAL_SIZE;
	}

	// seq. header
	CDMX_MEMCPY( *ppTempBuff, pbySeqHeaderData, ulSeqHeaderSize );
	
	// frame data
	pby_sour = pbyStreamData;
	pby_dest = *ppTempBuff+ulSeqHeaderSize;
	sour_size = *pulStreamSize;
	sour_idx = 0;
	dest_idx = 0;

	for(;;)
	{
		long nalsize = 0;

		if( sour_idx >= sour_size ) 
			break;

		// get length of Nal
		for( i = 0; i < lNalLengthSize; i++ )
			nalsize = (nalsize<<8) | pby_sour[sour_idx++];

		if( (nalsize <= 0) || (nalsize > ((long)sour_size-lNalLengthSize)) )
		{
			ALOGE( "[Err:%d] Invalid NALsize detected in %s(%d) \n", CDK_ERR_NAL_SIZE, __FILE__, __LINE__ );
			return CDK_ERR_NAL_SIZE;
		}

		// put startcode
		pby_dest[dest_idx+0] = 0; 
		pby_dest[dest_idx+1] = 0;
		pby_dest[dest_idx+2] = 0; 
		pby_dest[dest_idx+3] = 1;
		dest_idx += 4;
		CDMX_MEMCPY( &pby_dest[dest_idx], &pby_sour[sour_idx], nalsize );

		sour_idx += nalsize;
		dest_idx += nalsize;
	}

	// write seq. header + framedata
	*pulStreamSize = ulSeqHeaderSize + dest_idx;
	CDMX_MEMCPY( pbyOutStreamData, *ppTempBuff, *pulStreamSize );

#if 0

	INFO("pbyStreamData\n");
	cdmx_print_data( pbyStreamData, *pulStreamSize, 16, 128 );

#endif

	return 0;
}

static 
long
set_frame_avc(
    unsigned char 	 	 *pbyStreamData,		// [in]
    unsigned long  	  	 *pulStreamSize,		// [in, out]
    unsigned char 	 	 *pbyOutStreamData,		// [out]
    unsigned char		**ppTempBuff,
	unsigned long  	 	 *pulTempBuffSize,
	long  	 	  		  lNalLengthSize
	)
{
	long i = 0;
	unsigned char *pby_sour = NULL;
	unsigned char *pby_dest = NULL;
	unsigned long sour_size = 0;
	unsigned long sour_idx = 0;
	unsigned long dest_idx = 0;

	//FUNC( "In set_frame_avc\n" );
	if( lNalLengthSize < 0) 
	{
		ALOGE( "[Err:%d] Invalid NALsize detected in %s(%d) \n", CDK_ERR_NAL_SIZE, __FILE__, __LINE__ );
		return CDK_ERR_NAL_SIZE;
	}

	if( lNalLengthSize == 4 )
	{
		// in this case, 
		// the startcode can be directly put in a position where the size of stream is written.
		pby_sour = pbyStreamData;
		pby_dest = pbyOutStreamData;
		sour_size = *pulStreamSize;
		sour_idx = 0;
		dest_idx = 0;

		for(;;)
		{
			long nalsize = 0;

			if( sour_idx >= sour_size ) 
				break;

			// get length of Nal
			for( i = 0; i < lNalLengthSize; i++ )
				nalsize = (nalsize<<8) | pby_sour[sour_idx++];

			if( (nalsize <= 0) || (nalsize > ((long)sour_size-lNalLengthSize)) )
			{
				ALOGE( "[Err:%d] Invalid NALsize detected in %s(%d) \n", CDK_ERR_NAL_SIZE, __FILE__, __LINE__ );
				return CDK_ERR_NAL_SIZE;
			}

			// put startcode
			pby_dest[dest_idx+0] = 0; 
			pby_dest[dest_idx+1] = 0;
			pby_dest[dest_idx+2] = 0; 
			pby_dest[dest_idx+3] = 1;
			dest_idx += 4;

			sour_idx += nalsize;
			dest_idx += nalsize;
		}
	}
	else
	{
		if( *ppTempBuff == NULL )
		{
			*ppTempBuff = CDMX_MALLOC( CDMX_MAX_FRAME_LEN/*buf_size*/ );
			if ( *ppTempBuff == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] *ppTempBuff: 0x%x\n", __LINE__, *ppTempBuff);
			*pulTempBuffSize = CDMX_MAX_FRAME_LEN;
			CDMX_MEMSET( *ppTempBuff, 0, CDMX_MAX_FRAME_LEN );
		}
		else
		{
			unsigned long max_size = *pulStreamSize + 256; // we'll support maximum 256 nals.

			if ( *pulTempBuffSize < max_size ) 
			{
				*pulTempBuffSize = max_size;
				CDMX_FREE(*ppTempBuff);
				*ppTempBuff = CDMX_MALLOC( *pulTempBuffSize );
				if ( *ppTempBuff == NULL ) {
					ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
					return CDK_ERR_MALLOC;
				}
				DSTATUS("[Line:%04d] *ppTempBuff: 0x%x\n", __LINE__, *ppTempBuff);
				CDMX_MEMSET( *ppTempBuff, 0, *pulTempBuffSize );
			}
		}

		pby_sour = pbyStreamData;
		sour_size = *pulStreamSize;

		pby_dest = *ppTempBuff;
		sour_idx = 0;
		dest_idx = 0;

		for(;;)
		{
			long nalsize = 0;

			if( sour_idx >= sour_size ) 
				break;

			// get length of Nal
			for( i = 0; i < lNalLengthSize; i++ )
				nalsize = (nalsize<<8) | pby_sour[sour_idx++];

			if( (nalsize <= 0) || (nalsize > ((long)sour_size-lNalLengthSize)) )
			{
				ALOGE( "[Err:%d] Invalid NALsize detected in %s(%d) \n", CDK_ERR_NAL_SIZE, __FILE__, __LINE__ );
				return CDK_ERR_NAL_SIZE;
			}

			// put startcode
			pby_dest[dest_idx+0] = 0; 
			pby_dest[dest_idx+1] = 0;
			pby_dest[dest_idx+2] = 0; 
			pby_dest[dest_idx+3] = 1;
			dest_idx += 4;
			CDMX_MEMCPY( &pby_dest[dest_idx], &pby_sour[sour_idx], nalsize );

			sour_idx += nalsize;
			dest_idx += nalsize;
		}

		// write new framedata
		*pulStreamSize = dest_idx;
		CDMX_MEMCPY( pbyOutStreamData, *ppTempBuff, *pulStreamSize );
	}

	return 0;
}


static 
long
set_first_frame_with_seqheader_normal(
    unsigned char	 	 *pbySeqHeaderData,
	unsigned long  	  	  ulSeqHeaderSize,
	unsigned char 	 	 *pbyStreamData,		// [in]
	unsigned long  	  	 *pulStreamSize,		// [in, out]
	unsigned char 	 	 *pbyOutStreamData,		// [out]
    unsigned char		**ppTempBuff,
	unsigned long  	 	 *pulTempBuffSize
	)
{
	FUNC( "In set_first_frame_with_seqheader_normal\n" );
	if( *ppTempBuff == NULL )
	{
		*ppTempBuff = CDMX_MALLOC( CDMX_MAX_FRAME_LEN/*buf_size*/ );
		if ( *ppTempBuff == NULL ) {
			ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
			return CDK_ERR_MALLOC;
		}
		DSTATUS("[Line:%04d] *ppTempBuff: 0x%x\n", __LINE__, *ppTempBuff);
		*pulTempBuffSize = CDMX_MAX_FRAME_LEN;
		CDMX_MEMSET( *ppTempBuff, 0, CDMX_MAX_FRAME_LEN );
	}
	else
	{
		unsigned long max_size = ulSeqHeaderSize + *pulStreamSize;

		if ( *pulTempBuffSize < max_size ) 
		{
			*pulTempBuffSize = max_size;
			CDMX_FREE(*ppTempBuff);
			*ppTempBuff = CDMX_MALLOC( *pulTempBuffSize );
			if ( *ppTempBuff == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] *ppTempBuff: 0x%x\n", __LINE__, *ppTempBuff);
			CDMX_MEMSET( *ppTempBuff, 0, *pulTempBuffSize );
		}
	}

	if( ulSeqHeaderSize )
	{
		CDMX_MEMCPY( *ppTempBuff, pbySeqHeaderData, ulSeqHeaderSize );
	}
	CDMX_MEMCPY( *ppTempBuff+ulSeqHeaderSize, pbyStreamData, *pulStreamSize );
	*pulStreamSize = ulSeqHeaderSize + *pulStreamSize;
	CDMX_MEMCPY( pbyOutStreamData, *ppTempBuff, *pulStreamSize );

	return 0;
}

static 
long
set_frame_vc1_sp_mp( 
	unsigned char 	 	 *pbyStreamData,		// [in]
	unsigned long  	  	 *pulStreamSize,		// [in, out]
	unsigned char 	 	 *pbyOutStreamData,		// [out]
	unsigned char		**ppTempBuff,
	unsigned long  	 	 *pulTempBuffSize,
	unsigned long  	 	  ulStreamTimeStamp,
	long  	 	  		  bIsKeyFrame
	)
{
	unsigned long frame_len_rcv;
	unsigned long offset = 0;
	unsigned long cpylen = 0;

	//FUNC( "In set_frame_vc1_sp_mp\n" );
	if( *ppTempBuff == NULL )
	{
		*ppTempBuff = CDMX_MALLOC( CDMX_MAX_FRAME_LEN/*buf_size*/ );
		if ( *ppTempBuff == NULL ) {
			ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
			return CDK_ERR_MALLOC;
		}
		DSTATUS("[Line:%04d] *ppTempBuff: 0x%x\n", __LINE__, *ppTempBuff);
		*pulTempBuffSize = CDMX_MAX_FRAME_LEN;
		CDMX_MEMSET( *ppTempBuff, 0, CDMX_MAX_FRAME_LEN );
	}
	else
	{
		unsigned long max_size = 8 + *pulStreamSize;

		if ( *pulTempBuffSize < max_size ) 
		{
			*pulTempBuffSize = max_size;
			CDMX_FREE(*ppTempBuff);
			*ppTempBuff = CDMX_MALLOC( *pulTempBuffSize );
			if ( *ppTempBuff == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] *ppTempBuff: 0x%x\n", __LINE__, *ppTempBuff);
			CDMX_MEMSET( *ppTempBuff, 0, *pulTempBuffSize );
		}
	}

	frame_len_rcv = *pulStreamSize;
	if ( bIsKeyFrame )
		frame_len_rcv |= 0x80000000;

	// [] frame_len_rcv
	cpylen = 4;
	CDMX_MEMCPY( *ppTempBuff+offset, &frame_len_rcv, 	 cpylen ); offset += cpylen;
	// [] timestamp
	cpylen = 4;
	CDMX_MEMCPY( *ppTempBuff+offset, &ulStreamTimeStamp, cpylen ); offset += cpylen;
	// [] p_buf
	cpylen = *pulStreamSize;
	CDMX_MEMCPY( *ppTempBuff+offset, pbyStreamData, 	 cpylen ); offset += cpylen;

	// update data & size
	*pulStreamSize = offset;
	CDMX_MEMCPY( pbyOutStreamData, *ppTempBuff, *pulStreamSize );

	return 0;
}

static 
long
set_frame_vc1_ap( 
	unsigned char 	 	 *pbyStreamData,		// [in]
	unsigned long  	  	 *pulStreamSize,		// [in, out]
	unsigned char 	 	 *pbyOutStreamData,		// [out]
	unsigned char		**ppTempBuff,
	unsigned long  	 	 *pulTempBuffSize
	)
{
	unsigned long frame_len_rcv;
	unsigned long offset = 0;
	unsigned long cpylen = 0;

	//FUNC( "In set_frame_vc1_ap\n" );
	if (*ppTempBuff == NULL) 
	{
		*ppTempBuff = CDMX_MALLOC(CDMX_MAX_FRAME_LEN/*buf_size*/);
		if ( *ppTempBuff == NULL ) {
			ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
			return CDK_ERR_MALLOC;
		}
		DSTATUS("[Line:%04d] *ppTempBuff: 0x%x\n", __LINE__, *ppTempBuff);
		*pulTempBuffSize = CDMX_MAX_FRAME_LEN;
		CDMX_MEMSET(*ppTempBuff, 0, CDMX_MAX_FRAME_LEN);
	} 
	else 
	{
		unsigned long max_size = 4 + *pulStreamSize;

		if (*pulTempBuffSize < max_size) 
		{
			*pulTempBuffSize = max_size;
			CDMX_FREE(*ppTempBuff);
			*ppTempBuff = CDMX_MALLOC(*pulTempBuffSize);
			if ( *ppTempBuff == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] *ppTempBuff: 0x%x\n", __LINE__, *ppTempBuff);
			CDMX_MEMSET(*ppTempBuff, 0, *pulTempBuffSize);
		}
	}

	if ( !( pbyStreamData[0]==0 && pbyStreamData[1]==0 && pbyStreamData[2]==1 ) )
	{
		unsigned long ul_start_code = 0x0d010000;
		CDMX_MEMCPY( *ppTempBuff, 	&ul_start_code, 4 );
		CDMX_MEMCPY( *ppTempBuff+4, 	 pbyStreamData, *pulStreamSize );
		*pulStreamSize = 4 + *pulStreamSize;

		CDMX_MEMCPY( pbyOutStreamData, *ppTempBuff, *pulStreamSize );
	}

	return 0;
}

static 
long
set_frame_rv(
	unsigned char 	 	 *pbyStreamData,		// [in]
	unsigned long  	  	 *pulStreamSize,		// [in, out]
	unsigned char 	 	 *pbyOutStreamData,		// [out]
	unsigned char		**ppTempBuff,
	unsigned long  	 	 *pulTempBuffSize
	)
{
	unsigned long ul_num_packets = 0;
	unsigned long ul_num_segment = 0;
	unsigned long ul_val = 0;
	unsigned long offset = 0;
	unsigned long i = 0;

	//FUNC( "In set_frame_rv\n" );
	if( *ppTempBuff == NULL )
	{
		*ppTempBuff = CDMX_MALLOC( CDMX_MAX_FRAME_LEN/*buf_size*/ );
		if ( *ppTempBuff == NULL ) {
			ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
			return CDK_ERR_MALLOC;
		}
		DSTATUS("[Line:%04d] *ppTempBuff: 0x%x\n", __LINE__, *ppTempBuff);
		*pulTempBuffSize = CDMX_MAX_FRAME_LEN;
		CDMX_MEMSET( *ppTempBuff, 0, CDMX_MAX_FRAME_LEN );
	}
	else
	{
		unsigned long max_size = *pulStreamSize;

		if ( *pulTempBuffSize < max_size ) 
		{
			*pulTempBuffSize = max_size;
			CDMX_FREE(*ppTempBuff);
			*ppTempBuff = CDMX_MALLOC( *pulTempBuffSize );
			if ( *ppTempBuff == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] *ppTempBuff: 0x%x\n", __LINE__, *ppTempBuff);
			CDMX_MEMSET( *ppTempBuff, 0, *pulTempBuffSize );
		}
	}

	// [2]
	ul_num_packets = pbyStreamData[0] + 1;
	ul_num_segment = 2*ul_num_packets;
	ul_val = *pulStreamSize - (ul_num_segment*4 + 1);
	cdmx_write_be32(*ppTempBuff+offset, ul_val); offset += 4;

	// [3]
	CDMX_MEMSET(*ppTempBuff+offset, 0, 12); offset += 12;

	// [4]
	ul_val = ul_num_packets;
	cdmx_write_be32(*ppTempBuff+offset, ul_val); offset += 4;

	// [5]
	for(i=0; i< ul_num_segment; i++)	//segment : valid + offset
	{
		cdmx_write_be32( *ppTempBuff+offset+(i*4), *( pbyStreamData+1+(i*4) ) ); offset += 4;
	}
	CDMX_MEMCPY( *ppTempBuff+offset, pbyStreamData+1+(i*4), (*pulStreamSize-1+(i*4)) ); offset += *pulStreamSize-1+(i*4);

	*pulStreamSize = offset;
	CDMX_MEMCPY( pbyOutStreamData, *ppTempBuff, *pulStreamSize );

	return 0;
}

#define BE16(x) ((((unsigned char*)(x))[0] << 8) | ((unsigned char*)(x))[1])
long
avcc_to_bytestream( 
    unsigned char	 *pbyPriv,
	unsigned long 	  ulPrivSize,
	unsigned char	**ppbyDest,
	unsigned long	 *pulDestSize
	)
{
	unsigned char* pby_dest = NULL;
	unsigned long  ul_dest_idx = 0;

	unsigned char* pby_sour = NULL;
	unsigned long  ul_sour_idx = 0;

	long cnt, i, nalsize, nal_length_size;

	FUNC( "In avcc_to_bytestream\n" );
	if( ulPrivSize < 7 ) 
	{
		DPRINTF( "[Err:%d] avcC too short length is 0x%x in %s(%d) \n", CDK_ERR_INVALID_PARAM, (unsigned int)ulPrivSize, __FILE__, __LINE__ );
		return -1;
	}
	if( *pbyPriv != 1 ) 
	{
		DPRINTF( "[Err:%d] Unknown avcC version 0x%x in %s(%d) \n", CDK_ERR_INVALID_PARAM, (unsigned int)*pbyPriv, __FILE__, __LINE__ );
		return -1;
	}

	if( *ppbyDest == NULL )
	{
		*ppbyDest = CDMX_MALLOC( ulPrivSize + 2*2*128 ); // sps:128 nals / pps:128 nals
		if ( *ppbyDest == NULL ) {
			ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
			return CDK_ERR_MALLOC;
		}
		DSTATUS("[Line:%04d] *ppbyDest: 0x%x\n", __LINE__, *ppbyDest);
		*pulDestSize = ulPrivSize;
		CDMX_MEMSET( *ppbyDest, 0, ulPrivSize );
	}
	else
	{
		unsigned long max_size = ulPrivSize;

		if ( *pulDestSize < max_size ) 
		{
			*pulDestSize = max_size;
			CDMX_FREE(*ppbyDest);
			*ppbyDest = CDMX_MALLOC( *pulDestSize );
			if ( *ppbyDest == NULL ) {
				ERROR( "[Err:%d] malloc failed in %s(%d) \n", CDK_ERR_MALLOC, __FILE__, __LINE__ );
				return CDK_ERR_MALLOC;
			}
			DSTATUS("[Line:%04d] *ppbyDest: 0x%x\n", __LINE__, *ppbyDest);
			CDMX_MEMSET( *ppbyDest, 0, *pulDestSize );
		}
	}
	pby_dest = *ppbyDest;
	pby_sour = pbyPriv;

	/* length of sps and pps in the avcC have 2 bytes */
	nal_length_size = 2;

	// Decode sps
	ul_sour_idx = 5;
	cnt = pby_sour[ul_sour_idx] & 0x1f; // Number of sps
	ul_sour_idx ++;
	for( i = 0; i < cnt; i++ ) 
	{
		nalsize = BE16( &pby_sour[ul_sour_idx] );
		ul_sour_idx += 2;

		pby_dest[ul_dest_idx+0] = 0; 
		pby_dest[ul_dest_idx+1] = 0; 
		pby_dest[ul_dest_idx+2] = 0; 
		pby_dest[ul_dest_idx+3] = 1;
		ul_dest_idx += 4;

		CDMX_MEMCPY( &pby_dest[ul_dest_idx], &pby_sour[ul_sour_idx], nalsize );
		ul_dest_idx += nalsize;
		ul_sour_idx += nalsize;
	}
	// Decode pps
	cnt = pby_sour[ul_sour_idx]; // Number of pps
	ul_sour_idx ++;
	for( i = 0; i < cnt; i++ ) 
	{
		nalsize = BE16( &pby_sour[ul_sour_idx] );
		ul_sour_idx += 2;

		pby_dest[ul_dest_idx+0] = 0; 
		pby_dest[ul_dest_idx+1] = 0; 
		pby_dest[ul_dest_idx+2] = 0; 
		pby_dest[ul_dest_idx+3] = 1;
		ul_dest_idx += 4;

		CDMX_MEMCPY( &pby_dest[ul_dest_idx], &pby_sour[ul_sour_idx], nalsize );
		ul_dest_idx += nalsize;
		ul_sour_idx += nalsize;
	}

#ifndef ATTACH_SEQ_HEADER_FOR_VPU_DEC
	pby_dest[ul_dest_idx+0] = 0; 
	pby_dest[ul_dest_idx+1] = 0; 
	pby_dest[ul_dest_idx+2] = 0; 
	pby_dest[ul_dest_idx+3] = 1;
	ul_dest_idx += 4;
#endif

	*pulDestSize = ul_dest_idx;

	// nal length size
	nal_length_size = ((*(((char*)(pbyPriv))+4))&0x03)+1;
	return nal_length_size;
}

long
attach_start_code(
    cdmx_inst_t			*pstInst,
	av_dmx_info_t		*pstDmxInfo,
	av_dmx_outstream_t 	*pstDmxOutStream, //[in, out]
    unsigned char       *pbyOutStreamData //[out]
	)
{
	long ret = 0;
	unsigned long  ul_buff_size = 0;
	unsigned char *pby_buff = NULL;

	if( pbyOutStreamData == NULL ) // use pstDmxOutStream->m_pbyStreamData
		pbyOutStreamData = pstDmxOutStream->m_pbyStreamData;

	//FUNC( "In attach_start_code\n" );
#ifdef TCC_VPU_C7_INCLUDE
	if( pstInst->stCdmxCtrl.lCodec == CDMX_VCODEC_DIV3 
	#ifdef ATTACH_HEADER_VP8
		|| pstInst->stCdmxCtrl.lCodec == CDMX_VCODEC_VP8 
	#endif
		)
	{
		unsigned char buff[256];
		pby_buff = buff;

		set_vpuc7_startcode(pstInst->stCdmxCtrl.bIsNotFirstFrame,
							pstDmxInfo->m_pstDefaultVideoInfo->m_lWidth, 
							pstDmxInfo->m_pstDefaultVideoInfo->m_lHeight, 
							pstDmxOutStream->m_lStreamDataSize,
							pstInst->stCdmxCtrl.lCodec,
							pby_buff, &ul_buff_size);

		if( pstDmxOutStream->m_pbyStreamData != pbyOutStreamData )
		{
			CDMX_MEMCPY ( pbyOutStreamData, pby_buff, ul_buff_size);
			CDMX_MEMCPY ( pbyOutStreamData + ul_buff_size, pstDmxOutStream->m_pbyStreamData, pstDmxOutStream->m_lStreamDataSize);

			pstDmxOutStream->m_lStreamDataSize += ul_buff_size;

			if(pstInst->stCdmxCtrl.bIsNotFirstFrame == FALSE )
			{
				pstInst->stCdmxCtrl.bIsNotFirstFrame = TRUE;

				#ifdef DBG_PRINT_DATA
				if ( pstDmxOutStream->m_lStreamDataSize )
				{
					cdmx_print_data(pbyOutStreamData, pstDmxOutStream->m_lStreamDataSize, 16, 128);
				}
				#endif
			}
		}		else
		{
			CDMX_MEMMOVE ( (unsigned char *)(pstDmxOutStream->m_pbyStreamData + ul_buff_size),
						   pstDmxOutStream->m_pbyStreamData,
						   pstDmxOutStream->m_lStreamDataSize); // shift stream data

			CDMX_MEMCPY ( pstDmxOutStream->m_pbyStreamData, pby_buff, ul_buff_size);

			pstDmxOutStream->m_lStreamDataSize += ul_buff_size;

			if(pstInst->stCdmxCtrl.bIsNotFirstFrame == FALSE )
			{
				pstInst->stCdmxCtrl.bIsNotFirstFrame = TRUE;

				#ifdef DBG_PRINT_DATA
				if ( pstDmxOutStream->m_lStreamDataSize )
				{
					cdmx_print_data(pstDmxOutStream->m_pbyStreamData, pstDmxOutStream->m_lStreamDataSize, 16, 128);
				}
				#endif
			}
		}
	}
	else
#endif
	{
		if( pstInst->stCdmxCtrl.bIsNotFirstFrame == FALSE )
		{
			// it is a frist frame.
			pstInst->stCdmxCtrl.bIsNotFirstFrame = TRUE;

			switch (pstInst->stCdmxCtrl.lCodec) {
			case CDMX_VCODEC_VC1_SP_MP:
				ret = set_first_frame_with_seqheader_vc1_sp_mp(
						pstInst->stSeqHeader.m_pSeqHeaderData,
						pstInst->stSeqHeader.m_iSeqHeaderDataLen,
						pstDmxOutStream->m_pbyStreamData,
						(unsigned long*)&pstDmxOutStream->m_lStreamDataSize,
						pbyOutStreamData,
						&pstInst->stCdmxBuff.pbyStreamBuff,
						&pstInst->stCdmxBuff.ulStreamBuffSize,
						 pstDmxOutStream->m_lTimeStamp,
						 pstDmxInfo->m_pstDefaultVideoInfo
						 );
				break;

			case CDMX_VCODEC_VC1_AP:
				ret = set_first_frame_with_seqheader_vc1_ap(
						pstInst->stSeqHeader.m_pSeqHeaderData,
						pstInst->stSeqHeader.m_iSeqHeaderDataLen,
						pstDmxOutStream->m_pbyStreamData,
						(unsigned long*)&pstDmxOutStream->m_lStreamDataSize,
						pbyOutStreamData,
						&pstInst->stCdmxBuff.pbyStreamBuff,
						&pstInst->stCdmxBuff.ulStreamBuffSize
						 );
				break;

			case CDMX_VCODEC_WMV78:
				break;

			case CDMX_VCODEC_RV89:
				ret = set_first_frame_with_seqheader_rv(
						pstInst->stSeqHeader.m_pSeqHeaderData,
						pstInst->stSeqHeader.m_iSeqHeaderDataLen,
						pstDmxOutStream->m_pbyStreamData,
						(unsigned long*)&pstDmxOutStream->m_lStreamDataSize,
						pbyOutStreamData,
						&pstInst->stCdmxBuff.pbyStreamBuff,
						&pstInst->stCdmxBuff.ulStreamBuffSize
						 );

				break;

			case CDMX_VCODEC_AVC:
				ret = set_first_frame_with_seqheader_avc(
						pstInst->stSeqHeader.m_pSeqHeaderData,
						pstInst->stSeqHeader.m_iSeqHeaderDataLen,
						pstDmxOutStream->m_pbyStreamData,
						(unsigned long*)&pstDmxOutStream->m_lStreamDataSize,
						pbyOutStreamData,
						&pstInst->stCdmxBuff.pbyStreamBuff,
						&pstInst->stCdmxBuff.ulStreamBuffSize, 
						pstInst->stCdmxCtrl.lAvcNalLengthSize
						);
				break;

			default:
				if(pstInst->stSeqHeader.m_iSeqHeaderDataLen)
				{
					ret = set_first_frame_with_seqheader_normal(
							pstInst->stSeqHeader.m_pSeqHeaderData,
							pstInst->stSeqHeader.m_iSeqHeaderDataLen,
							pstDmxOutStream->m_pbyStreamData,
							(unsigned long*)&pstDmxOutStream->m_lStreamDataSize,
							pbyOutStreamData,
							&pstInst->stCdmxBuff.pbyStreamBuff,
							&pstInst->stCdmxBuff.ulStreamBuffSize 
							);
				}
				break;
			}

			#ifdef DBG_PRINT_DATA
			if ( pstDmxOutStream->m_lStreamDataSize )
			{
				cdmx_print_data(pbyOutStreamData, pstDmxOutStream->m_lStreamDataSize, 16, 128);
			}
			#endif
		}
		else // if( pstInst->stCdmxCtrl.bIsNotFirstFrame )
		{
			switch (pstInst->stCdmxCtrl.lCodec) {
			case CDMX_VCODEC_AVC:
				ret = set_frame_avc(
						pstDmxOutStream->m_pbyStreamData,
						(unsigned long*)&pstDmxOutStream->m_lStreamDataSize,
						pbyOutStreamData,
						&pstInst->stCdmxBuff.pbyStreamBuff,
						&pstInst->stCdmxBuff.ulStreamBuffSize, 
						pstInst->stCdmxCtrl.lAvcNalLengthSize
						);
				break;

			case CDMX_VCODEC_VC1_SP_MP:
				ret = set_frame_vc1_sp_mp(
						pstDmxOutStream->m_pbyStreamData,
						(unsigned long*)&pstDmxOutStream->m_lStreamDataSize,
						pbyOutStreamData,
						&pstInst->stCdmxBuff.pbyStreamBuff,
						&pstInst->stCdmxBuff.ulStreamBuffSize,
						pstDmxOutStream->m_lTimeStamp,
						pstInst->stCdmxCtrl.bIsKeyframe 
						);
				break;

			case CDMX_VCODEC_VC1_AP:
				ret = set_frame_vc1_ap(
						pstDmxOutStream->m_pbyStreamData,
						(unsigned long*)&pstDmxOutStream->m_lStreamDataSize,
						pbyOutStreamData,
						&pstInst->stCdmxBuff.pbyStreamBuff,
						&pstInst->stCdmxBuff.ulStreamBuffSize 
						);
				break;

			case CDMX_VCODEC_RV89:
				ret = set_frame_rv(
						pstDmxOutStream->m_pbyStreamData,
						(unsigned long*)&pstDmxOutStream->m_lStreamDataSize,
						pbyOutStreamData,
						&pstInst->stCdmxBuff.pbyStreamBuff,
						&pstInst->stCdmxBuff.ulStreamBuffSize 
						);
				break;

			default:
				break;
			}
		}
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//  dynamic loading
//
long 
open_library(
    const char	 *pszLibName,		// so
	const char	 *pszLibFuncName,	// api
	void		**ppvLibFunc,
	void		**ppvLibHandle
	)
{
	long ret = 0;
	FUNC("In open_library");
	void* pv_handle  = NULL;
	void* pv_libfunc = NULL;

    pv_handle = dlopen(pszLibName, RTLD_NOW);
    // dlopen() returns NULL if there were any issues opening the library
    if (pv_handle == NULL)
    {
        // check for errors
        const char* p_err = dlerror();
        if (p_err == NULL) {
            // No error reported, but no handle to the library
            ERROR("load_dmx: Error opening library (%s) but no error reported", pszLibName);
        } else {
            // Error reported
            ERROR("load_dmx: Error opening library (%s): %s", pszLibName, p_err);
        }
		return CDK_ERR_LIB_LOAD_FAILED;
    }

	pv_libfunc = dlsym(pv_handle, pszLibFuncName);
	if (pv_libfunc == NULL) {
        // check for errors
        const char* p_err = dlerror();
        if (NULL == p_err) {
            // No error reported, but no symbol was found
            ERROR("load_dmx: Could not access (%s) symbol in library but no error reported", pszLibFuncName);
        } else {
            // Error reported
            ERROR("load_dmx: Could not access (%s) symbol in library [%s]", pszLibFuncName, p_err);
        }
		return CDK_ERR_LIB_LOAD_FAILED;
	}

	*ppvLibFunc   = pv_libfunc;
	*ppvLibHandle = pv_handle;

	DSTATUS("Loading %s(0x%x) (handle:0x%x) ...\n", pszLibFuncName, pv_libfunc, pv_handle);
	return ret;
}

long 
close_library(
	void	*pvLibHandle
	)
{
	long ret = 0;

	FUNC( "In close_library(handle:0x%x)\n", pvLibHandle );
	if (0 != dlclose(pvLibHandle))
	{
		// dlclose() returns non-zero value if close failed, check for errors
		const char* p_err = dlerror();
		if (NULL != p_err) {
			ERROR("OsclSharedLibrary::Close: Error closing library: %s", p_err);
		} else {
			ERROR("OsclSharedLibrary::Close: Error closing library, no error reported");
		}
		return -1;
	}
	return ret;
}

