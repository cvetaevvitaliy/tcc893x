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
 *   FileName    : vpu_ringbuffer_util.c
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/

#include <omxcore.h>
#include <vpu_ringbuffer_struct.h>

#define LOG_TAG	"VPU_RINGBUFFER_UTIL"
#if 1
#include <utils/Log.h>

static int DEBUG_ON = 1;

#define LOG_INPUT_BUFF_INFO           0  // input buffer information log - LOG_INPUT()
#define LOG_FRAME_SKIP_MODE           0  // VPU skip mode information log - LOG_SKIP()
#define LOG_DECODE_STATUS             0  // decode status log - LOG_DEC()
#define LOG_OUTPUT_STATUS             0  // output status log - LOG_OUT()
#define LOG_INPUT_INFO_QUEUE          0  // input information queue internal log - LOG_IIQUE()
#define LOG_INPUT_INFO_MANAGEMENT     0  // input information management log - LOG_IIMGE()
#define LOG_RING_BUFFER_STATUS        0  // VPU ring-buffer helper internal log - LOG_RING()
#define LOG_FEEDING_STATUS            0  // VPU feeder internal log - LOG_FEED()
#define LOG_DISPLAY_INFO_MANAGER      0  // display information manager log - LOG_DIMGR()
#define LOG_DISPLAY_INDEX_QUEUE       0  // display index queue internal log - LOG_IDXQUE()
#define LOG_BUFFER_CLEAR_STATUS       0  // buffer clear log - LOG_BUFCLR()
#define LOG_GRALLOC_MODE              0  // gralloc mode log - LOG_GRA()

#define PRINTF(msg...)	if (DEBUG_ON) { ALOGD( ": " msg);}
#define PRINTF_ERROR(msg...)	{ ALOGE( ": " msg);}

#define VPU_LOGV(CLOG_TAG, ...)	ALOGV("[LOG_"#CLOG_TAG"] "__VA_ARGS__)
#define VPU_LOGD(CLOG_TAG, ...)	ALOGD("[LOG_"#CLOG_TAG"] "__VA_ARGS__)
#define VPU_LOGI(CLOG_TAG, ...)	ALOGI("[LOG_"#CLOG_TAG"] "__VA_ARGS__)
#define VPU_LOGW(CLOG_TAG, ...)	ALOGW("[LOG_"#CLOG_TAG"] "__VA_ARGS__)
#define VPU_LOGE(CLOG_TAG, ...)	ALOGE("[LOG_"#CLOG_TAG"] "__VA_ARGS__)

#if LOG_INPUT_BUFF_INFO
#undef LOG_INPUT
#define LOG_INPUT(...)   VPU_LOGV(INPUT, __VA_ARGS__)
#else
#define LOG_INPUT(...)
#endif

#if LOG_FRAME_SKIP_MODE
#undef LOG_SKIP
#define LOG_SKIP(...)    VPU_LOGI(SKIP, __VA_ARGS__)
#else
#define LOG_SKIP(...)
#endif

#if LOG_DECODE_STATUS
#undef LOG_DEC
#define LOG_DEC(...)     VPU_LOGI(DEC, __VA_ARGS__)
#else
#define LOG_DEC(...)
#endif

#if LOG_OUTPUT_STATUS
#undef LOG_OUT
#define LOG_OUT(...)     VPU_LOGD(OUT, __VA_ARGS__)
#else
#define LOG_OUT(...)
#endif

#if LOG_INPUT_INFO_QUEUE
#undef LOG_IIQUE
#define LOG_IIQUE(...)   VPU_LOGD(IIQUE, __VA_ARGS__)
#else
#define LOG_IIQUE(...)
#endif

#if LOG_INPUT_INFO_MANAGEMENT
#undef LOG_IIMGE
#define LOG_IIMGE(...)   VPU_LOGI(IIMGE, __VA_ARGS__)
#else
#define LOG_IIMGE(...)
#endif

#if LOG_RING_BUFFER_STATUS
#undef LOG_RING
#define LOG_RING(...)    VPU_LOGI(RING, __VA_ARGS__)
#else
#define LOG_RING(...)
#endif

#if LOG_FEEDING_STATUS
#undef LOG_FEED
#define LOG_FEED(...)    VPU_LOGD(FEED, __VA_ARGS__)
#else
#define LOG_FEED(...)
#endif

#if LOG_DISPLAY_INFO_MANAGER
#undef LOG_DIMGR
#define LOG_DIMGR(...)   VPU_LOGD(DIMGR, __VA_ARGS__)
#else
#define LOG_DIMGR(...)
#endif

#if LOG_DISPLAY_INDEX_QUEUE
#undef LOG_IDXQUE
#define LOG_IDXQUE(...)  VPU_LOGI(IDXQUE, __VA_ARGS__)
#else
#define LOG_IDXQUE(...)
#endif

#if LOG_BUFFER_CLEAR_STATUS
#undef LOG_BUFCLR
#define LOG_BUFCLR(...)  VPU_LOGI(BUFCLR, __VA_ARGS__)
#else
#define LOG_BUFCLR(...)
#endif

#if LOG_GRALLOC_MODE
#undef LOG_GRA
#define LOG_GRA(...)     VPU_LOGV(GRALLOC, __VA_ARGS__)
#else
#define LOG_GRA(...)
#endif

#else
#endif

#if 0
extern int gPlayMode;
#endif

OMX_PTR *gstRingBuffState[2];

static
OMX_S32 
ScanPaddedSpace(
	ringbuff_info_t        *pstRingbuff,
	OMX_U8                  *pScanStart,
	OMX_U8                  *pScanEnd,
	OMX_U8                 **ppSpaceEnd
	)
{
#define PRESCAN_MAX		32

	OMX_U8  *p_curr_rp = pstRingbuff->pReadPtr;
	OMX_U8  *p_temp_rp = p_curr_rp;
	OMX_U32 *p_rp32 = (OMX_U32*)((((OMX_U32)p_curr_rp + 3) >> 2) << 2);
	OMX_U8  *p_pad_start = 0;
	OMX_U8  *p_pad_end = 0;
	OMX_S32 pad_size = 0;

	if( pScanStart < pScanEnd )
	{
		OMX_U8 *p_temp_end = p_curr_rp + PRESCAN_MAX;

		if( p_temp_end > pScanEnd )
			p_temp_end = pScanEnd;

		// search zero start to PRESCAN_MAX bytes
		while( p_temp_rp < p_temp_end ) {
			if( *p_temp_rp )
				p_pad_start = NULL;
			else if( p_pad_start == NULL )
				p_pad_start = p_temp_rp;
			p_temp_rp++;
		}

		if( p_pad_start )
		{
			p_temp_rp = p_pad_start;
			p_rp32 = (OMX_U32*)((((OMX_U32)p_temp_rp + 3) >> 2) << 2);

			while( p_temp_rp < p_rp32 && p_temp_rp < pScanEnd ) {
				if( *p_temp_rp ) {
					p_pad_end = p_temp_rp;
					break;
				}
				p_temp_rp++;
			}

			if( p_pad_end == NULL && p_temp_rp < pScanEnd )
			{
				while( p_rp32 < pScanEnd-3 ) {
					if( *p_rp32 )
						break;
					p_rp32++;
				}
				
				p_temp_rp = (OMX_U8*)p_rp32;
				while( p_temp_rp < pScanEnd ) {
					if( *p_temp_rp ) {
						p_pad_end = p_temp_rp;
						break;
					}
					p_temp_rp++;
				}
			}
	
			if( p_pad_end == NULL )
				p_pad_end = pScanEnd;

			pad_size = (OMX_S32)(p_pad_end - p_pad_start);

			ASSERT( pScanStart <= p_pad_end && p_pad_end <= pScanEnd );
		}
	}
	else 
	{
		OMX_S32 count;

		if( p_curr_rp < pScanEnd )
			count  = pScanEnd - p_curr_rp;
		else {
			count  = (OMX_S32)(pstRingbuff->pRingBuffEnd[VA] - p_curr_rp);
			count += (OMX_S32)(pScanEnd - pstRingbuff->pRingBuffBase[VA]);
		}

		if( count > PRESCAN_MAX )
			count = PRESCAN_MAX;

		while( count-- ) {
			if( *p_temp_rp )
				p_pad_start = NULL;
			else if( p_pad_start == NULL )
				p_pad_start = p_temp_rp;
			if( ++p_temp_rp >= pstRingbuff->pRingBuffEnd[VA] )
				p_temp_rp = pstRingbuff->pRingBuffBase[VA];
		}

		if( p_pad_start )
		{
			OMX_U8 *p_rgn_end = (OMX_U8*)((((OMX_U32)p_pad_start + 3) >> 2) << 2);

			p_rp32 = (OMX_U32*)p_rgn_end;
			p_temp_rp = p_pad_start;

			if( pScanStart > p_rgn_end && p_rgn_end > pScanEnd )
				p_rgn_end = pScanEnd;

			while( p_temp_rp < p_rgn_end ) {
				if( *p_temp_rp ) {
					p_pad_end = p_temp_rp;
					break;
				}
				p_temp_rp++;
			}

			if( p_pad_end == NULL && p_temp_rp != pScanEnd )
			{
				if( pScanStart < p_temp_rp )
				{
					p_rgn_end = pstRingbuff->pRingBuffEnd[VA];
	
					// rear
					while( p_rp32 < p_rgn_end-3 ) {
						if( *p_rp32 )
							break;
						p_rp32++;
					}
	
					// front
					if( p_rp32 >= p_rgn_end )
					{
						p_rp32 = pstRingbuff->pRingBuffBase[VA];
						p_rgn_end = (OMX_U8*)(((OMX_U32)pScanEnd >> 2) << 2);
			
						while( p_rp32 < p_rgn_end-3 ) {
							if( *p_rp32 )
								break;
							p_rp32++;
						}
	
						p_rgn_end = pScanEnd;
					}
	
					p_temp_rp = (OMX_U8*)p_rp32;
					while( p_temp_rp < p_rgn_end ) {
						if( *p_temp_rp ) {
							p_pad_end = p_temp_rp;
							break;
						}
						p_temp_rp++;
					}
				}
				else
				{
					// front
					p_rp32 = p_rgn_end;
					p_rgn_end = (OMX_U8*)(((OMX_U32)pScanEnd >> 2) << 2);

					while( p_rp32 < p_rgn_end-3 ) {
						if( *p_rp32 )
							break;
						p_rp32++;
					}

					p_temp_rp = (OMX_U8*)p_rp32;
					while( p_temp_rp < pScanEnd ) {
						if( *p_temp_rp ) {
							p_pad_end = p_temp_rp;
							break;
						}
						p_temp_rp++;
					}
				}
			}
	
			if( p_pad_end == NULL )
				p_pad_end = pScanEnd;

			if( p_pad_start < p_pad_end ) {
				pad_size  = (OMX_S32)(p_pad_end - p_pad_start);
			} 
			else {
				pad_size  = (OMX_S32)(pstRingbuff->pRingBuffEnd[VA] - p_pad_start);
				pad_size += (OMX_S32)(p_pad_end - pstRingbuff->pRingBuffBase[VA]);
			}

			ASSERT( p_pad_end <= pScanEnd || pScanStart <= p_pad_end );
		}
	}

	if( pad_size && ppSpaceEnd )
		*ppSpaceEnd = p_pad_end;

#if 0
	if( pad_size )
	{
		ALOGE("[MGE][PAD] PADSIZE: %5d / START: %6d / PADRGN: %6d ~ %6d / SCANRGN: %6d ~ %6d"
			  , pad_size
			  , p_curr_rp - pstRingbuff->pRingBuffBase[VA]
			  , p_pad_start - pstRingbuff->pRingBuffBase[VA]
			  , p_pad_end - pstRingbuff->pRingBuffBase[VA]
			  , pScanStart - pstRingbuff->pRingBuffBase[VA]
			  , pScanEnd - pstRingbuff->pRingBuffBase[VA]
			  );
	}
#endif

	return pad_size;
}

OMX_U32 ScanSequenceHeader(OMX_U32 video_coding_type, OMX_U8* inputCurrBuffer, OMX_U32 inputCurrLength, OMX_U32 *sequence_offset, OMX_U32 search_option, OMX_U8 uiDecoderID)
{
	unsigned int uiCode = 0xFFFFFFFF;
	OMX_U32 temp_sequence_offset = *sequence_offset;
	OMX_U32 code_type = CODETYPE_NONE;

	//ALOGE("==> MPEG2 : %d, AVC : %d, video_coding_type : %d", OMX_VIDEO_CodingMPEG2, OMX_VIDEO_CodingAVC, video_coding_type);

	if (OMX_VIDEO_CodingMPEG2 == video_coding_type)
	{
		unsigned int SEQUENCE_HEADER = 0x000001B3;
		unsigned int PICTURE_START = 0x00000100;

		for (; temp_sequence_offset < inputCurrLength; temp_sequence_offset++)
		{
			uiCode = (uiCode << 8) | inputCurrBuffer[temp_sequence_offset];
			if ((search_option & CODETYPE_HEADER) && (uiCode == SEQUENCE_HEADER))
			{
				code_type = CODETYPE_HEADER;
				temp_sequence_offset -= 3;
				break;
			}
			if ((search_option & CODETYPE_PICTURE) && (uiCode == PICTURE_START))
			{
				code_type = CODETYPE_PICTURE;
				temp_sequence_offset -= 3;
				break;
			}
		}
	}
	else if (OMX_VIDEO_CodingAVC == video_coding_type)
	{
		unsigned int MASK = 0xFFFFFF1F;
		unsigned int P_FRAME = 0x00000101;
		unsigned int I_FRAME = 0x00000105;
		unsigned int SPS = 0x00000107;
		unsigned int uiMask;

		for (; temp_sequence_offset < inputCurrLength; temp_sequence_offset++)
		{
			uiCode = (uiCode << 8) | inputCurrBuffer[temp_sequence_offset];
			uiMask = uiCode & MASK;
			if ((search_option & CODETYPE_HEADER) && (uiMask == SPS))
			{
				code_type = CODETYPE_HEADER;
				temp_sequence_offset -= 3;
				break;
			}
			if ((search_option & CODETYPE_PICTURE) && ((uiMask == P_FRAME) || (uiMask == I_FRAME)))
			{
				code_type = CODETYPE_PICTURE;
				temp_sequence_offset -= 3;
				break;
			}
		}
	}
	*sequence_offset = temp_sequence_offset;

	return code_type;
}

OMX_BOOL
InitInputQueue(
	input_queue_t  *pstQue,
	OMX_S32         lInitSize
	)
{
	if( pstQue->pstInfoRing )
	{
		TCC_free(pstQue->pstInfoRing);
	}

	memset(pstQue, 0, sizeof(input_queue_t));

	pstQue->pstInfoRing = TCC_malloc(sizeof(input_info_t) * lInitSize);
	if( pstQue->pstInfoRing == NULL )
	{
		DeinitInputQueue(pstQue);
		ALOGE("CreatePTSQueue() - out of memory");
		return OMX_FALSE;
	}
	pstQue->lQueSize = lInitSize;

	return OMX_TRUE;
}

void
DeinitInputQueue(
	input_queue_t  *pstQue
	)
{
	if( pstQue->pstInfoRing )
	{
		TCC_free( pstQue->pstInfoRing );
	}

	pstQue->pstInfoRing = NULL;
}

void
ClearInputQueue(
	input_queue_t  *pstQue
	)
{
	pstQue->lStartPos = 0;
	pstQue->lEndPos   = 0;
	pstQue->lQueCount = 0;
}

OMX_S32
GetInputInfo(
	input_queue_t  *pstQue,
	input_info_t   *pstInput
	)
{
	input_info_t  *p_ring = pstQue->pstInfoRing;
	OMX_S32        start  = pstQue->lStartPos;
	OMX_S32        size   = pstQue->lQueSize;
	OMX_S32        count  = pstQue->lQueCount;

	if( count == 0 ) {
		ALOGE("GetInputInfo() - internal error");
		return -1;
	}

	*pstInput = p_ring[start];

	if( ++start >= size )
		start -= size;

	if( --count )
		pstQue->lStartPos = start;
	else {
		pstQue->lStartPos	= 0;
		pstQue->lEndPos		= 0;
	}

	pstQue->lQueCount = count;

	LOG_IDXQUE("[GET  ][QUECNT: %3d] [PTS: %8d] [REGION: 0x%08X ~ 0x%08X] [LENGTH: %7d byte]"
			  , pstQue->lQueCount
			  , (OMX_S32)(pstInput->llTimestamp/1000)
			  , pstInput->pStartPtr
			  , pstInput->pEndPtr
			  , pstInput->lFilledLen
			  );

	return count;
}

OMX_S32 
ShowInputInfo(
	input_queue_t  *pstQue, 
	input_info_t   *pstInput
	)
{
	OMX_S32 count = pstQue->lQueCount;

	if( count == 0 ) {
		//ALOGE("ShowInputInfo() - info queue is empty");
		return -1;
	}

	*pstInput = pstQue->pstInfoRing[pstQue->lStartPos];

	LOG_IIQUE("[SHOW ][QUECNT: %3d] [PTS: %8d] [REGION: 0x%08X ~ 0x%08X] [LENGTH: %7d byte]"
			  , pstQue->lQueCount
			  , (OMX_S32)(pstInput->llTimestamp/1000)
			  , pstInput->pStartPtr
			  , pstInput->pEndPtr
			  , pstInput->lFilledLen
			  );

	return count;
}

OMX_S32 
ShowLastInputInfo(
	input_queue_t  *pstQue, 
	input_info_t   *pstInput
	)
{
	OMX_S32 count = pstQue->lQueCount;
	OMX_S32 end   = pstQue->lEndPos-1;

	if( count == 0 ) {
		//ALOGE("ShowLastInputInfo() - info queue is empty");
		return -1;
	}

	if( end < 0 )
		end = pstQue->lQueSize - 1;

	*pstInput = pstQue->pstInfoRing[end];

	LOG_IIQUE("[SHOWL][QUECNT: %3d] [PTS: %8d] [REGION: 0x%08X ~ 0x%08X] [LENGTH: %7d byte]"
			  , pstQue->lQueCount
			  , (OMX_S32)(pstInput->llTimestamp/1000)
			  , pstInput->pStartPtr
			  , pstInput->pEndPtr
			  , pstInput->lFilledLen
			  );

	return count;
}

OMX_S32 
ClearFirstInputInfo(
	input_queue_t  *pstQue
	)
{
	input_info_t  *p_ring = pstQue->pstInfoRing;
	OMX_S32        start  = pstQue->lStartPos;
	OMX_S32        size   = pstQue->lQueSize;
	OMX_S32        count  = pstQue->lQueCount;

	if( count == 0 ) {
		ALOGE("ClearFirstInputInfo() - internal error");
		return -1;
	}

	if( ++start >= size )
		start -= size;

	if( --count )
		pstQue->lStartPos = start;
	else {
		pstQue->lStartPos	= 0;
		pstQue->lEndPos		= 0;
	}

	pstQue->lQueCount = count;

	//LOG_IIQUE("[CLEAR][QUECNT: %3d]", pstQue->lQueCount);

	return count;
}

OMX_BOOL
InitRingBuffer(
	ringbuff_info_t		*pstRingInfo,
	OMX_PTR				 pVpuInstance
	)
{
	vdec_ring_buffer_out_t ring_stat; 
	OMX_S32 ret = 0;

	ALOGD("VPU ring-buffer initialize");

	pstRingInfo->pVpuInstance = pVpuInstance;
	pstRingInfo->pRingBuffBase[PA] = vpu_getBitstreamBufAddr(PA, pVpuInstance);
	pstRingInfo->pRingBuffBase[VA] = vpu_getBitstreamBufAddr(VA, pVpuInstance);
	pstRingInfo->lRingBuffSize = vpu_getBitstreamBufSize(pVpuInstance);
	pstRingInfo->pRingBuffEnd[PA] = pstRingInfo->pRingBuffBase[PA] + pstRingInfo->lRingBuffSize;
	pstRingInfo->pRingBuffEnd[VA] = pstRingInfo->pRingBuffBase[VA] + pstRingInfo->lRingBuffSize;

	ResetRingBuffer(pstRingInfo);

	if( (ret = vdec_vpu(VDEC_GET_RING_BUFFER_STATUS, NULL, NULL, &ring_stat, pVpuInstance)) < 0 )
	{
		ALOGE("[VPU_ERROR] [OP: VDEC_GET_RING_BUFFER_STATUS] [RET_CODE: %d]", ret);
		return OMX_FALSE;
	}

	LOG_RING("[INIT     ] - RING-BUFFER Environment - PA: 0x%08X / VA: 0x%08X / size: %d"
			 , pstRingInfo->pRingBuffBase[PA]
			 , pstRingInfo->pRingBuffBase[VA]
			 , pstRingInfo->lRingBuffSize);

	LOG_RING("[INIT     ] - [RP: %7d][WP: %7d] [EMPTY: %7d bytes] [BUFFERED: %7d bytes]"
			 , ring_stat.m_ptrReadAddr_PA-(OMX_U32)pstRingInfo->pRingBuffBase[PA]
			 , ring_stat.m_ptrWriteAddr_PA-(OMX_U32)pstRingInfo->pRingBuffBase[PA]
			 , pstRingInfo->lEmptySpace
			 , pstRingInfo->lWrittenBytes);

	return OMX_TRUE;
}

OMX_S32
UpdateRingBuffer(
	ringbuff_info_t			*pstRingInfo,
	OMX_BOOL				 bFromVpu
	)
{
	OMX_S32 ret = 0;

	if( bFromVpu )
	{
		vdec_ring_buffer_out_t ring_stat; 
		OMX_U8 *p_ring_base_pa = pstRingInfo->pRingBuffBase[PA];
		OMX_U8 *p_ring_base_va = pstRingInfo->pRingBuffBase[VA];
		OMX_U8 *p_prev_rp;
		OMX_U8 *p_curr_rp;
		OMX_S32 ret = 0;

		if( (ret = vdec_vpu(VDEC_GET_RING_BUFFER_STATUS, NULL, NULL, &ring_stat, pstRingInfo->pVpuInstance)) < 0 ) {
			ERROR("[VPU_ERROR] [OP: VDEC_GET_RING_BUFFER_STATUS] [RET_CODE: %d]", ret);
			return ret;
		}

		ASSERT( ((pstRingInfo->pReadPtr < pstRingInfo->pWritePtr) && (ring_stat.m_ptrReadAddr_PA <= ring_stat.m_ptrWriteAddr_PA)) 
				|| ((pstRingInfo->pReadPtr > pstRingInfo->pWritePtr)) );

		#if USE_AVAILABLE_SIZE_FROM_VPU
		ASSERT( (pstRingInfo->lEmptySpace + pstRingInfo->lWrittenBytes) <= (ring_stat.m_ulAvailableSpaceInRingBuffer) );
		#endif

		/* prev read pointer */
		p_prev_rp = pstRingInfo->pReadPtr;
		pstRingInfo->pPrevReadPtr = p_prev_rp;

		/* read pointer */
		p_curr_rp = p_ring_base_va + ((OMX_U8*)ring_stat.m_ptrReadAddr_PA - p_ring_base_pa);
		pstRingInfo->pReadPtr = p_curr_rp;


        /* This is exception code for invalid position of pReadPtr
         * It should be fixed with right way.
         */
        if(abs(p_curr_rp - p_prev_rp) < PRESCAN_MAX)
            p_curr_rp = p_prev_rp;

		/* used bytes */
		if( p_curr_rp < p_prev_rp )
			pstRingInfo->lUsedByte = (p_curr_rp - p_ring_base_va) + (pstRingInfo->pRingBuffEnd[VA] - p_prev_rp);
		else
			pstRingInfo->lUsedByte = p_curr_rp - p_prev_rp;
		ASSERT( pstRingInfo->lUsedByte <= (pstRingInfo->lRingBuffSize - (pstRingInfo->lEmptySpace + pstRingInfo->lWrittenBytes)) );

		/* empty size */
		#if USE_AVAILABLE_SIZE_FROM_VPU
		pstRingInfo->lEmptySpace = ring_stat.m_ulAvailableSpaceInRingBuffer - pstRingInfo->lWrittenBytes;
		#else
		{
			OMX_S32 temp_size;
			if( ring_stat.m_ptrReadAddr_PA <= ring_stat.m_ptrWriteAddr_PA )
				temp_size = pstRingInfo->lRingBuffSize - (OMX_S32)(ring_stat.m_ptrWriteAddr_PA - ring_stat.m_ptrReadAddr_PA);
			else
				temp_size = (OMX_S32)(ring_stat.m_ptrReadAddr_PA - ring_stat.m_ptrWriteAddr_PA);
			temp_size -= pstRingInfo->lWrittenBytes;
			pstRingInfo->lEmptySpace = temp_size;
		}
		#endif

		LOG_RING("[UPDATE-DN] - [RP: %7d][WP: %7d] [EMPTY: %7d bytes] [BUFFERED: %7d bytes] [USED : %7d bytes]"
				 , ring_stat.m_ptrReadAddr_PA-(OMX_U32)pstRingInfo->pRingBuffBase[PA]
				 , ring_stat.m_ptrWriteAddr_PA-(OMX_U32)pstRingInfo->pRingBuffBase[PA]
				 , pstRingInfo->lEmptySpace
				 , pstRingInfo->lWrittenBytes
				 , pstRingInfo->lUsedByte
				 );
	}
	else 
	{
#if UPDATE_WRITE_PTR_WITH_ALIGNED_LENGTH
		if( pstRingInfo->lWrittenBytes < WRITE_PTR_ALIGN_BYTES ) 
			return 0;
		else
		{
			OMX_S32 unaligned_byte = pstRingInfo->lWrittenBytes % WRITE_PTR_ALIGN_BYTES;

			if( (ret = vdec_vpu(VDEC_UPDATE_WRITE_BUFFER_PTR, NULL, (void*)(pstRingInfo->lWrittenBytes - unaligned_byte), (void*)pstRingInfo->bFlushRing, pstRingInfo->pVpuInstance)) < 0 ) {
				ERROR( "[VPU_ERROR] [OP: VDEC_UPDATE_WRITE_BUFFER_PTR] [RET_CODE: %d]", ret);
				return ret;
			}
			pstRingInfo->lWrittenBytes = unaligned_byte;
			pstRingInfo->bFlushRing = OMX_FALSE;
		}
#else
		if( (ret = vdec_vpu(VDEC_UPDATE_WRITE_BUFFER_PTR, NULL, (void*)pstRingInfo->lWrittenBytes, (void*)pstRingInfo->bFlushRing, pstRingInfo->pVpuInstance)) < 0 ) {
			ERROR( "[VPU_ERROR] [OP: VDEC_UPDATE_WRITE_BUFFER_PTR] [RET_CODE: %d]", ret);
			return ret;
		}
		pstRingInfo->lWrittenBytes = 0;
		pstRingInfo->bFlushRing = OMX_FALSE;
#endif

		#if CHECK_RING_STATUS_AFTER_UPDATE
		{
			vdec_ring_buffer_out_t ring_stat;
			if( (ret = vdec_vpu(VDEC_GET_RING_BUFFER_STATUS, NULL, NULL, &ring_stat, pstRingInfo->pVpuInstance)) < 0 ) {
				ERROR("[VPU_ERROR] [OP: VDEC_GET_RING_BUFFER_STATUS] [RET_CODE: %d]", ret);
				return ret;
			}

			if( (pstRingInfo->pWritePtr-pstRingInfo->lWrittenBytes) != pstRingInfo->pRingBuffBase[VA] + (ring_stat.m_ptrWriteAddr_PA-(OMX_U32)pstRingInfo->pRingBuffBase[PA]) ) {
				ERROR("[LOG_RING] [RP: %d / %d] [WP: %d / %d] "
					 , pstRingInfo->pReadPtr-(OMX_U32)pstRingInfo->pRingBuffBase[VA]		//rp offset (VA)
					 , ring_stat.m_ptrReadAddr_PA-(OMX_U32)pstRingInfo->pRingBuffBase[PA]	//rp offset (PA)
					 , pstRingInfo->pWritePtr-(OMX_U32)pstRingInfo->pRingBuffBase[VA]		//wp offset (VA)
					 , ring_stat.m_ptrWriteAddr_PA-(OMX_U32)pstRingInfo->pRingBuffBase[PA]	//wp offset (PA)
					 );
			}

			#if USE_AVAILABLE_SIZE_FROM_VPU
			if( pstRingInfo->lEmptySpace != ring_stat.m_ulAvailableSpaceInRingBuffer ) {
				ERROR("[LOG_RING] EMPTY: %d / %d", pstRingInfo->lEmptySpace, ring_stat.m_ulAvailableSpaceInRingBuffer);
			}
			#else
			{
				long temp_size;
				if( ring_stat.m_ptrReadAddr_PA <= ring_stat.m_ptrWriteAddr_PA )
					temp_size = pstRingInfo->lRingBuffSize - (OMX_S32)(ring_stat.m_ptrWriteAddr_PA - ring_stat.m_ptrReadAddr_PA);
				else
					temp_size = (OMX_S32)(ring_stat.m_ptrReadAddr_PA - ring_stat.m_ptrWriteAddr_PA);
				if( pstRingInfo->lEmptySpace != temp_size ) {
					ERROR("[LOG_RING] EMPTY: %d / %d", pstRingInfo->lEmptySpace, temp_size);
				}
			}
			#endif
		}
		#endif

		LOG_RING("[UPDATE-UP] - [RP: %7d][WP: %7d] [EMPTY: %7d bytes] [BUFFERED: %7d bytes]"
				 , pstRingInfo->pReadPtr-(OMX_U32)pstRingInfo->pRingBuffBase[VA]
				 , pstRingInfo->pWritePtr-(OMX_U32)pstRingInfo->pRingBuffBase[VA]
				 , pstRingInfo->lEmptySpace
				 , pstRingInfo->lWrittenBytes
				 );
	}

	return 0;
}

OMX_S32
PushInputInfo(
	input_queue_t      *pstQue,
	const input_info_t *pstInput
	)
{
	input_info_t  *p_ring = pstQue->pstInfoRing;
	OMX_S32        size   = pstQue->lQueSize;
	OMX_S32        count  = pstQue->lQueCount;
	OMX_S32        end    = pstQue->lEndPos;

	if( ++count > size )
	{
		input_info_t *p_tmp_ring;
		OMX_S32       cpy = pstQue->lStartPos;
		OMX_S32       new_size = size * 2;
		OMX_S32       i;

		ALOGI("Input information queue resizing");

		p_tmp_ring = TCC_malloc(sizeof(input_info_t) * new_size);
		if( p_tmp_ring == NULL ) {
			ALOGE("PushInputInfo() - out of memory");
			return -1;
		}

		for(i = 0; i < count-1; i++, cpy++)
		{
			if(cpy >= size)
				cpy -= size;
			p_tmp_ring[i] = p_ring[cpy];
		}

		size = new_size;
		end	= i;

		TCC_free(p_ring);
		p_ring = p_tmp_ring;
		pstQue->pstInfoRing = p_ring;
		pstQue->lQueSize = new_size;
		pstQue->lStartPos = 0;
		pstQue->lEndPos = end;
	}

	if( count == 1 ) {
		p_ring[pstQue->lEndPos++] = *pstInput;
		pstQue->lQueCount = 1;
	}
	else {
		p_ring[end] = *pstInput;

		if( ++pstQue->lEndPos >= size )
			pstQue->lEndPos -= size;
		pstQue->lQueCount = count;
	}

	LOG_IIQUE("[PUSH ][QUECNT: %3d] [PTS: %8d] [REGION: 0x%08X ~ 0x%08X] [LENGTH: %7d byte]"
			  , pstQue->lQueCount
			  , (OMX_S32)(pstInput->llTimestamp/1000)
			  , pstInput->pStartPtr
			  , pstInput->pEndPtr
			  , pstInput->lFilledLen
			  );

	return count;
}

OMX_BOOL
FillRingBuffer(
	ringbuff_info_t			*pstRingInfo,
	OMX_U8					*pInputBuff,
	OMX_S32					 lInputLength
	)
{
	if( pstRingInfo->lEmptySpace > (lInputLength + EMPTY_SIZE_MIN) ) 
	{
		OMX_U8  *p_ring_end  = pstRingInfo->pRingBuffEnd[VA];
		OMX_U8  *p_wp        = pstRingInfo->pWritePtr;
		OMX_U8  *p_data      = pInputBuff;
		OMX_S32  data_size   = lInputLength;
		OMX_S32  tmp_size    = (OMX_S32)(p_ring_end-p_wp);

		pstRingInfo->pPrevWritePtr = p_wp;

		if( tmp_size < data_size ) {
			OMX_U8  *p_ring_base = pstRingInfo->pRingBuffBase[VA];

			LOG_RING("[CHECK 0] - [p_wp:%x][p_data:%x][tmp_size:%d]"
					 , p_wp, p_data, tmp_size
					 );

			memcpy(p_wp, p_data, tmp_size);
			data_size -= tmp_size;
			p_data += tmp_size;

			LOG_RING("[CHECK 1] - [p_ring_base:%x][p_data:%x][data_size:%d]"
					 , p_ring_base, p_data, data_size
					 );
			memcpy(p_ring_base, p_data, data_size);
			p_wp = p_ring_base + data_size;
		}
		else {
			LOG_RING("[CHECK 2] - [p_ring_base:%x][p_data:%x][data_size:%d]"
					 , p_wp, p_data, data_size
					 );
			memcpy(p_wp, p_data, data_size);
			p_wp += data_size;
		}

		pstRingInfo->pWritePtr = p_wp;
		pstRingInfo->lWrittenBytes += lInputLength;
		pstRingInfo->lEmptySpace -= lInputLength;

		LOG_RING("[FILL     ] - [RP: %7d][WP: %7d] [EMPTY: %7d bytes] [BUFFERED: %7d bytes] [INPUT: %7d bytes]"
				 , pstRingInfo->pReadPtr-(OMX_U32)pstRingInfo->pRingBuffBase[VA]
				 , pstRingInfo->pWritePtr-(OMX_U32)pstRingInfo->pRingBuffBase[VA]
				 , pstRingInfo->lEmptySpace
				 , pstRingInfo->lWrittenBytes
				 , lInputLength
				 );

		return OMX_TRUE;
	}

	LOG_RING("[FILL     ] - [RP: %7d][WP: %7d] [EMPTY: %7d bytes] [BUFFERED: %7d bytes] [INPUT: %7d bytes] - [NOT ENOUGH]"
			 , pstRingInfo->pReadPtr-(OMX_U32)pstRingInfo->pRingBuffBase[VA]
			 , pstRingInfo->pWritePtr-(OMX_U32)pstRingInfo->pRingBuffBase[VA]
			 , pstRingInfo->lEmptySpace
			 , pstRingInfo->lWrittenBytes
			 , lInputLength
			 );

	return OMX_FALSE;
}

OMX_S32
ResetRingBuffer(
	ringbuff_info_t		*pstRingInfo
	)
{
	OMX_S32 ret = 0;

	pstRingInfo->pWritePtr     = pstRingInfo->pRingBuffBase[VA];
	pstRingInfo->pPrevWritePtr = pstRingInfo->pRingBuffBase[VA];
	pstRingInfo->pReadPtr      = pstRingInfo->pRingBuffBase[VA];
	pstRingInfo->lWrittenBytes = 0;
	pstRingInfo->lEmptySpace   = pstRingInfo->lRingBuffSize;
	pstRingInfo->bFlushRing    = OMX_TRUE;

#if FLUSH_RING_BEFORE_UPDATE_TIME
	if( (ret = vdec_vpu(VDEC_UPDATE_WRITE_BUFFER_PTR, NULL, (void*)0, (void*)pstRingInfo->bFlushRing, pstRingInfo->pVpuInstance)) < 0 )
		ERROR( "[VPU_ERROR] [OP: VDEC_UPDATE_WRITE_BUFFER_PTR] [RET_CODE: %d]", ret);
	pstRingInfo->bFlushRing = OMX_FALSE;

	if( ret == 0 ) {
		vdec_ring_buffer_out_t ring_stat; 

		if( (ret = vdec_vpu(VDEC_GET_RING_BUFFER_STATUS, NULL, NULL, &ring_stat, pstRingInfo->pVpuInstance)) < 0 )
		{
			ERROR("[VPU_ERROR] [OP: VDEC_GET_RING_BUFFER_STATUS] [RET_CODE: %d]", ret);
			return ret;
		}

		pstRingInfo->pReadPtr += ((OMX_U8*)ring_stat.m_ptrReadAddr_PA - pstRingInfo->pRingBuffBase[PA]);
		pstRingInfo->pWritePtr += ((OMX_U8*)ring_stat.m_ptrWriteAddr_PA - pstRingInfo->pRingBuffBase[PA]);
		pstRingInfo->pPrevWritePtr = pstRingInfo->pWritePtr;
		#if USE_AVAILABLE_SIZE_FROM_VPU
		pstRingInfo->lEmptySpace = ring_stat.m_ulAvailableSpaceInRingBuffer;
		#endif
	}
#else
#endif

	LOG_RING("[RESET    ] - [RP: %7d][WP: %7d] [EMPTY: %7d bytes] [BUFFERED: %7d bytes]"
			 , pstRingInfo->pReadPtr-(OMX_U32)pstRingInfo->pRingBuffBase[VA]
			 , pstRingInfo->pWritePtr-(OMX_U32)pstRingInfo->pRingBuffBase[VA]
			 , pstRingInfo->lEmptySpace
			 , pstRingInfo->lWrittenBytes
			 );

	return ret;
}

OMX_S32
FeedVpuDecoder(
	ringbuff_state_t		*pstRingBuffState,
	OMX_BUFFERHEADERTYPE	*pInputBuffer,
	OMX_U8                  *pBuffer,
	OMX_U32                  nFilledLen,
	OMX_TICKS				 llTimeStamp,
	OMX_BOOL                 bFeedFlag
	)
{
	if( FillRingBuffer(&pstRingBuffState->stRingBuffInfo, 
					   pBuffer,
					   nFilledLen) == OMX_TRUE )
	{
		input_info_t info;

		info.llTimestamp = llTimeStamp;  
		info.lFilledLen  = nFilledLen;
		info.pStartPtr   = pstRingBuffState->stRingBuffInfo.pPrevWritePtr;
		info.pEndPtr     = pstRingBuffState->stRingBuffInfo.pWritePtr;

		LOG_FEED("[PUSH  ] [PTS: %8d / REGION: %7d ~ %7d]"
				  , (OMX_S32)(info.llTimestamp/1000)
				  , info.pStartPtr - pstRingBuffState->stRingBuffInfo.pRingBuffBase[VA]
				  , info.pEndPtr - pstRingBuffState->stRingBuffInfo.pRingBuffBase[VA]
				  );

		if( PushInputInfo(&pstRingBuffState->stInputInfoQueue, &info) < 0 )
			return FEED_FAILED;

		pstRingBuffState->lInputCount++;

		LOG_FEED("[INPUT_CNT: %7d] [QUEUED TIME: %8d ~ %8d ms] [DIFF:%10lld us] [LIMIT:%10lld us]"
				 , pstRingBuffState->lInputCount
				 , (OMX_S32)(pstRingBuffState->llQueuedStartTime/1000)
				 , (OMX_S32)(info.llTimestamp/1000)
				 , info.llTimestamp - pstRingBuffState->llQueuedStartTime
				 , pstRingBuffState->llFeedLimitTimeDiff
				 );

		if( pstRingBuffState->llQueuedStartTime < 0 ) {
			pstRingBuffState->llQueuedStartTime = info.llTimestamp;
			if(pInputBuffer)
				pInputBuffer->nFilledLen = 0;
			return NEED_MORE_DATA;
		}

		if( (abs(info.llTimestamp - pstRingBuffState->llQueuedStartTime) < pstRingBuffState->llFeedLimitTimeDiff) )
		{
			if(pInputBuffer)
				pInputBuffer->nFilledLen = 0;
			return NEED_MORE_DATA;
		}

	#if UPDATE_WRITE_PTR_WITH_ALIGNED_LENGTH
		if( ((pstRingBuffState->stRingBuffInfo.lRingBuffSize - pstRingBuffState->stRingBuffInfo.lEmptySpace) < WRITE_PTR_ALIGN_BYTES*4) ) //FIXME
		{
			if(pInputBuffer)
				pInputBuffer->nFilledLen = 0;
			return NEED_MORE_DATA;
		}
	#endif
		if(bFeedFlag)
			pInputBuffer->nFlags |= OMX_BUFFERFLAG_ONLY_FEED_FRAME;
		else
		{
			if(pInputBuffer)
				pInputBuffer->nFilledLen = 0;
			pInputBuffer->nFlags &= ~OMX_BUFFERFLAG_ONLY_FEED_FRAME;
		}
	}
	else
	{
		LOG_FEED("[INPUT_CNT: %7d] [QUEUED TIME: %8d ~ %8d ms] [DIFF:%10lld us] [LIMIT:%10lld us] - [BUFFER FULL]"
				 , pstRingBuffState->lInputCount
				 , (OMX_S32)(pstRingBuffState->llQueuedStartTime/1000)
				 , (OMX_S32)(pInputBuffer->nTimeStamp/1000)
				 , pInputBuffer->nTimeStamp - pstRingBuffState->llQueuedStartTime
				 , pstRingBuffState->llFeedLimitTimeDiff
				 );
	}

	if( UpdateRingBuffer(&pstRingBuffState->stRingBuffInfo, OMX_FALSE) < 0 )
		return FEED_FAILED;

	return FEED_COMPLETE;
}

OMX_TICKS
GetCurrTimestamp(
	ringbuff_state_t *pstRingBuffState,
	OMX_TICKS        *pllTimestamp,
	OMX_BOOL          bDecodeSuccess,
	OMX_BOOL          bInterlaced
	)
{
	if( UpdateRingBuffer(&pstRingBuffState->stRingBuffInfo, OMX_TRUE) < 0 )
		return -1;

#if SINGLE_FRAME_INPUT_ENABLE
	if( pstVpuPrivate->bSingleFrameInput )
	{
		input_info_t   info;

		if( GetInputInfo(&pstRingBuffState->stInputInfoQueue, &info) < 0 )
			return -1;
		*pllTimestamp = info.llTimestamp;

		if( ShowInputInfo(&pstRingBuffState->stInputInfoQueue, &info) < 0 )
			info.llTimestamp = -1;
		pstRingBuffState->llQueuedStartTime = info.llTimestamp;

		LOG_IIMGE("[RESULT] --------- [CURRENT_BASE_PTS: %8d / NEXT_BASE_PTS: %8d] ---------"
				  , (OMX_S32)(*pllTimestamp/1000)
				  , (OMX_S32)(pstRingBuffState->llQueuedStartTime/1000)
				  );
	}
	else
#endif
	{
		input_info_t   info;
		OMX_U8        *p_curr_rp = pstRingBuffState->stRingBuffInfo.pReadPtr;
		OMX_U8        *p_prev_rp = pstRingBuffState->stRingBuffInfo.pPrevReadPtr;
		OMX_TICKS      candidate_pts[4];
		OMX_S32        candidate_byte[4];
		OMX_S32        candidate_count = 0;
		OMX_S32        index = 0;
		OMX_TICKS      result_pts = -1;
		OMX_U8        *p_org_rp = p_curr_rp;	// for debug

		if( ShowInputInfo(&pstRingBuffState->stInputInfoQueue, &info) < 0 )
			return -1;
	
		if( p_curr_rp == pstRingBuffState->stRingBuffInfo.pRingBuffEnd[VA] )
			p_curr_rp = 0;

		LOG_IIMGE("[SEARCH] [PTS: %8d / REGION: %7d ~ %7d (%5d Byte)] - [PRP: %7d ~ RP: %7d]"
				  , (OMX_S32)(info.llTimestamp/1000)
				  , info.pStartPtr - pstRingBuffState->stRingBuffInfo.pRingBuffBase[VA]
				  , info.pEndPtr - pstRingBuffState->stRingBuffInfo.pRingBuffBase[VA]
				  , info.lFilledLen
				  , p_prev_rp - pstRingBuffState->stRingBuffInfo.pRingBuffBase[VA]
				  , p_curr_rp - pstRingBuffState->stRingBuffInfo.pRingBuffBase[VA]
				  );

		if( info.pStartPtr < info.pEndPtr ) {
			if( info.pStartPtr <= p_curr_rp && p_curr_rp <= info.pEndPtr ) {
				if( bDecodeSuccess && ScanPaddedSpace(&pstRingBuffState->stRingBuffInfo, info.pStartPtr, info.pEndPtr, &p_curr_rp) > 0 )
					pstRingBuffState->stRingBuffInfo.pReadPtr = p_curr_rp;
				result_pts = info.llTimestamp;
			}
		}
		else {
			if( p_curr_rp <= info.pEndPtr || info.pStartPtr <= p_curr_rp ) {
				if( bDecodeSuccess && ScanPaddedSpace(&pstRingBuffState->stRingBuffInfo, info.pStartPtr, info.pEndPtr, &p_curr_rp) > 0 )
					pstRingBuffState->stRingBuffInfo.pReadPtr = p_curr_rp;
				result_pts = info.llTimestamp;
			}
		}

		if( result_pts < 0 )
		{
			if( info.pStartPtr < info.pEndPtr ) {
				candidate_byte[index] = (OMX_S32)(info.pEndPtr - p_prev_rp);
			}
			else {
				if( p_prev_rp < info.pEndPtr )
					candidate_byte[index]  = (OMX_S32)(info.pEndPtr - p_prev_rp);
				else {
					candidate_byte[index]  = (OMX_S32)(pstRingBuffState->stRingBuffInfo.pRingBuffEnd[VA] - p_prev_rp);
					candidate_byte[index] += (OMX_S32)(info.pEndPtr - pstRingBuffState->stRingBuffInfo.pRingBuffBase[VA]);
				}
			}
			candidate_pts[index] = info.llTimestamp;
			candidate_count++;

			while( 1 ) 
			{
				ClearFirstInputInfo(&pstRingBuffState->stInputInfoQueue);
				if( ShowInputInfo(&pstRingBuffState->stInputInfoQueue, &info) < 0 ) {
					info.pStartPtr   = 0;
					info.pEndPtr     = 0;
					info.llTimestamp = -1;	// queued input is not exist.
					break;
				}

				LOG_IIMGE("[SEARCH] [PTS: %8d / REGION: %7d ~ %7d (%5d Byte)] - [PRP: %7d ~ RP: %7d]"
						  , (OMX_S32)(info.llTimestamp/1000)
						  , info.pStartPtr - pstRingBuffState->stRingBuffInfo.pRingBuffBase[VA]
						  , info.pEndPtr - pstRingBuffState->stRingBuffInfo.pRingBuffBase[VA]
						  , info.lFilledLen
						  , p_prev_rp - pstRingBuffState->stRingBuffInfo.pRingBuffBase[VA]
						  , p_curr_rp - pstRingBuffState->stRingBuffInfo.pRingBuffBase[VA]
						  );

				if( ++index >= 4 ) 
					index = 0;
				candidate_pts[index] = info.llTimestamp;
				candidate_byte[index] = info.lFilledLen;
				candidate_count++;

				if( info.pStartPtr < info.pEndPtr ) {
					if( info.pStartPtr <= p_curr_rp && p_curr_rp <= info.pEndPtr ) {
						if( bDecodeSuccess && ScanPaddedSpace(&pstRingBuffState->stRingBuffInfo, info.pStartPtr, info.pEndPtr, &p_curr_rp) > 0 )
							pstRingBuffState->stRingBuffInfo.pReadPtr = p_curr_rp;
						candidate_byte[index]  = (OMX_S32)(p_curr_rp - info.pStartPtr);
						break;
					}
				}
				else {
					if( p_curr_rp <= info.pEndPtr ) {
						if( bDecodeSuccess && ScanPaddedSpace(&pstRingBuffState->stRingBuffInfo, info.pStartPtr, info.pEndPtr, &p_curr_rp) > 0 )
							pstRingBuffState->stRingBuffInfo.pReadPtr = p_curr_rp;
						candidate_byte[index]  = (OMX_S32)(pstRingBuffState->stRingBuffInfo.pRingBuffEnd[VA] - info.pStartPtr);
						candidate_byte[index] += (OMX_S32)(p_curr_rp - pstRingBuffState->stRingBuffInfo.pRingBuffBase[VA]);
						break;
					}
					else if( info.pStartPtr <= p_curr_rp ) {
						if( bDecodeSuccess && ScanPaddedSpace(&pstRingBuffState->stRingBuffInfo, info.pStartPtr, info.pEndPtr, &p_curr_rp) > 0 )
							pstRingBuffState->stRingBuffInfo.pReadPtr = p_curr_rp;
						candidate_byte[index]  = (OMX_S32)(p_curr_rp - info.pStartPtr);
						break;
					}
				}
			}
		}

		if( p_curr_rp == info.pEndPtr )
			ClearFirstInputInfo(&pstRingBuffState->stInputInfoQueue);

		LOG_IIMGE("[SEARCH][CANDIDATE: %d] [%ll6d (%6d Byte)] [%ll6d (%6d Byte)] [%ll6d (%6d Byte)] [%ll6d (%6d Byte)]"
				  , candidate_count
				  , candidate_pts[0]/1000
				  , candidate_byte[0]
				  , candidate_count > 1 ? candidate_pts[1]/1000 : -1
				  , candidate_count > 1 ? candidate_byte[1] : -1
				  , candidate_count > 2 ? candidate_pts[2]/1000 : -1
				  , candidate_count > 2 ? candidate_byte[2] : -1
				  , candidate_count > 3 ? candidate_pts[3]/1000 : -1
				  , candidate_count > 3 ? candidate_byte[3] : -1
				  );

		if( bDecodeSuccess )
		{
			if( bInterlaced )
			{
				if( candidate_count == 2 ) {	// candidate count == 2
					result_pts = candidate_pts[1];
				}
				else if( candidate_count == 3 ) {
					if( candidate_byte[0] < candidate_byte[2] )
						result_pts = candidate_pts[2];
					else
						result_pts = candidate_pts[1];
				}
				else if( candidate_count == 4 ) {
					result_pts = candidate_pts[2];
				}
				else if( candidate_count > 4 ) {
					if( ((candidate_byte[index] * 100) / info.lFilledLen) < 1 ) // < 1 percent
						result_pts = candidate_pts[index-1 < 0 ? 3 : index-1];
					else
						result_pts = candidate_pts[index];
				}
			}
			else
			{
				if( candidate_count == 2 ) {	// candidate count == 2
					if( candidate_byte[0] <= candidate_byte[1] )
						result_pts = candidate_pts[1];
					else
						result_pts = candidate_pts[0];
				}
				else if( candidate_count == 3 ) {
					result_pts = candidate_pts[1];	// second (mid) pts
				}
				else if( candidate_count >= 4 ) {
					if( (candidate_byte[index] * 100) / info.lFilledLen < 1 ) // < 1 percent
						result_pts = candidate_pts[index-1 < 0 ? 3 : index-1];
					else
						result_pts = candidate_pts[index];
				}
			}

			*pllTimestamp = result_pts;
		}
		else
		{
			*pllTimestamp = info.llTimestamp;
		}

		if( pstRingBuffState->llQueuedStartTime != info.llTimestamp )
			pstRingBuffState->llQueuedStartTime = info.llTimestamp;

		LOG_IIMGE("[RESULT] --------- [INPUT_RP: %8d / BASE_PTS: %8d] ---------"
				  , p_org_rp - pstRingBuffState->stRingBuffInfo.pRingBuffBase[VA]
				  , (OMX_S32)(*pllTimestamp/1000)
				  );
	}

	return 0;
}

void SetRingBuffStateInstance(OMX_PTR pInst, OMX_U32 uiDecoderID)
{
	gstRingBuffState[uiDecoderID] = pInst;
}

OMX_PTR GetRingBuffStateInstance(OMX_U32 uiDecoderID)
{
	return gstRingBuffState[uiDecoderID];
}
