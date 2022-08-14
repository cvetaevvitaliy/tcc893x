/**

  Copyright (C) 2007-2008  STMicroelectronics
  Copyright (C) 2007-2008 Nokia Corporation and/or its subsidiary(-ies).
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

#include <omxcore.h>
#include <omx_base_video_port.h>
#include <omx_vpu_c7_ringbuffer_dec_component.h>
#include <OMX_Video.h>

#include <TCC_VPU_CODEC_EX.h>
#include <vdec.h>
#include <cdk.h>
#include <video_user_data.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <mach/tccfb_ioctrl.h>

#define LOG_TAG	"OMX_TCC_VPU_C7_RINGBUFFER_DEC"
#include <utils/Log.h>

#include <cutils/properties.h>

#include "tcc_dxb_interface_demux.h"

#define iTotalHandle 2

#if 1
/* This is not ready for tcc93xx.
 * Later We will use this option.
 */
#define		SUPPORT_SEARCH_IFRAME_ATSTARTING
#endif

#if 0
int gPlayMode = 0;
#endif

//#define   SUPPORT_SAVE_OUTPUTBUFFER //Save output to files
#ifdef	SUPPORT_SAVE_OUTPUTBUFFER
static int giDumpVideoFileIndex = 0;
#endif

#define		BUFFER_INDICATOR	   "TDXB"
#define		BUFFER_INDICATOR_SIZE  4

#define		VIDEO_BUFFERCOUNT_MIN		20
#define		VIDEO_BUFFERCOUNT_ACTUAL	20

static int DEBUG_ON = 0;
#define DBUG_MSG(msg...)	if (DEBUG_ON) { ALOGD( ": " msg);/* sleep(1);*/}
#define DPRINTF_DEC(msg...)	//ALOGI( ": " msg);
#define DPRINTF_DEC_STATUS(msg...)	//ALOGD( ": " msg);

#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
#define MARK_BUFF_NOT_USED	0xFFFFFFFF
#define MAX_CHECK_COUNT_FOR_CLEAR	200
OMX_BOOL bRingRollback = OMX_FALSE;
#endif

#define FB0_DEVICE		"/dev/graphics/fb0"

static int mFBdevice = -1;
static int bOutputMode = 0;
static OMX_BOOL bChangeDisplayID = OMX_FALSE;
static OMX_BOOL isRenderer = OMX_TRUE;
static OMX_BOOL isRefFrameBroken = OMX_FALSE;

OMX_TICKS isDecodTimeStamp[iTotalHandle];

static cdk_func_t *gspfVDecList[VCODEC_MAX] = {
	vdec_vpu,	//STD_AVC
	vdec_vpu,	//STD_VC1
	vdec_vpu,	//STD_MPEG2
	vdec_vpu,	//STD_MPEG4
	vdec_vpu,	//STD_H263
	vdec_vpu,	//STD_DIV3
	vdec_vpu,	//STD_RV
	vdec_vpu,	//
	0,			//STD_WMV78
	0			//STD_SORENSON263
};

/** The output decoded color format */
#define OUTPUT_DECODED_COLOR_FMT OMX_COLOR_FormatYUV420Planar

typedef enum VPU_Buffer_Mode
{
	RING_BUFFER_MODE = 0,
	FILE_BUFFER_MODE = 1
};

#ifdef VIDEO_USER_DATA_PROCESS 
	#define FLAG_ENABLE_USER_DATA 1
#else
	#define FLAG_ENABLE_USER_DATA 0
#endif

#define VIDEO_PICTYPE_MASK  0x7FFFFFFF

#define MAX_DECODE_INSTANCE_ERROR_CHCEK_CNT	300

#define VPU_FIELDPICTURE_PENDTIME_MAX	2000	//ms

//////////////////////////////////////////////////////////////////////////////////////
// state flags
#define STATE_SKIP_OUTPUT_B_FRAME           0x00000001

#define SET_FLAG(__p_vpu_private, __flag)    ((__p_vpu_private).ulFlags |= (__flag))
#define CLEAR_FLAG(__p_vpu_private, __flag)  ((__p_vpu_private).ulFlags &= ~(__flag))
#define CHECK_FLAG(__p_vpu_private, __flag)  ((__p_vpu_private).ulFlags & (__flag))
//////////////////////////////////////////////////////////////////////////////////////

void* omx_ring_video_twoport_filter_component_BufferMgmtFunction (void* param);
typedef long long (*pfnDemuxGetSTC)(int itype, long long lpts, unsigned int index, int log);
static pfnDemuxGetSTC gfnDemuxGetSTCValue = NULL;

extern void * vdec_alloc_instance(void);

unsigned int ring_dbg_get_time(void)
{
	struct timeval	time;
	unsigned int cur_time;

	gettimeofday(&time, NULL);
	cur_time = ((time.tv_sec * 1000) & 0x00ffffff) + (time.tv_usec / 1000);

	return cur_time;
}

static int getHWCID()
{
	char value[PROPERTY_VALUE_MAX];
	property_get("tcc.video.hwr.id", value, "");
	return atoi(value);
}

static int isVsyncSupport()
{
	char value[PROPERTY_VALUE_MAX];
	int bVsyncSupport = 0;

#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
	property_get("tcc.video.vsync.support", value, "0");
	if (atoi(value) != 0)
	{
		bVsyncSupport = 1;
	}

	property_get("tcc.hwc.dynamic.disable", value, "0");
	if (atoi(value) != 0)
	{
		bVsyncSupport = 0;
	}
#endif

	return bVsyncSupport;
}

static int isVsyncEnable()
{
	char value[PROPERTY_VALUE_MAX];
	int bVsyncEnable = 0;

	if (isVsyncSupport())
	{
#ifndef USE_LCD_VIDEO_VSYNC
		property_get("tcc.sys.output_mode_detected", value, "");
		if (atoi(value) != 0)
#endif
		{
			bVsyncEnable = 1;
		}
	}
	return bVsyncEnable;
}

#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
static int tcc_vsync_command(omx_videodec_component_PrivateType *omx_private, int arg1, int arg2)
{
	int ret = -2;
	if(mFBdevice >= 0)
	{
		if (arg1 & 0x80000000) {
			ret = ioctl(mFBdevice, arg1 & 0x7FFFFFFF, &arg2);
		} else {
			ret = ioctl(mFBdevice, arg1, arg2);
		}
		//ALOGE("%s:%d:%d:%d:%s",__func__, arg1, command2, ret, strerror(errno));
	}
	return ret;
}

static int clear_front_vpu_buffer(_VIDEO_DECOD_INSTANCE_ *pVDecInst)
{
	int nDevID = pVDecInst->Display_index[pVDecInst->out_index] >> 16;
	int nClearDispIndex = pVDecInst->Display_index[pVDecInst->out_index] & 0xFFFF;

	pVDecInst->Display_index[pVDecInst->out_index] = 0;

	//nClearDispIndex++; //CAUTION !!! to avoid NULL(0) data insertion.
	if( queue_ex(pVDecInst->pVPUDisplayedIndexQueue, nClearDispIndex) == 0 )
	{
		//DPRINTF_DEC_STATUS("@ DispIdx Queue %d", pVDecInst->Display_index[pVDecInst->in_index]);
		nClearDispIndex--;
		int ret = pVDecInst->gspfVDec (VDEC_BUF_FLAG_CLEAR, NULL, &nClearDispIndex, NULL, pVDecInst->pVdec_Instance);
		ALOGE ("%s VDEC_BUF_FLAG_CLEAR Err : [%d] index : %lu",__func__, ret, pVDecInst->Display_index[pVDecInst->out_index]);
	}
	pVDecInst->out_index = (pVDecInst->out_index + 1) % pVDecInst->max_fifo_cnt;
	pVDecInst->used_fifo_count--;

	return 0;
}

static int clear_vpu_buffer(int buffer_id, omx_videodec_component_PrivateType *omx_private, _VIDEO_DECOD_INSTANCE_ *pVDecInst)
{
	int cleared_buff_count = 0;
	int loopCount = 0;

	while(buffer_id < pVDecInst->Display_Buff_ID[pVDecInst->out_index] && loopCount < MAX_CHECK_COUNT_FOR_CLEAR)
	{
		usleep(5000);
		buffer_id = tcc_vsync_command(omx_private,TCC_LCDC_VIDEO_GET_DISPLAYED, 0);
		if (buffer_id < 0)	break;
		loopCount++;
	}

	if(buffer_id<0)
		buffer_id = pVDecInst->Display_Buff_ID[pVDecInst->out_index];

	while(buffer_id >= pVDecInst->Display_Buff_ID[pVDecInst->out_index] && pVDecInst->used_fifo_count>0)
	{
		clear_front_vpu_buffer(pVDecInst);
		cleared_buff_count++;
	}

//	bChangeDisplayID = OMX_FALSE;
	if(cleared_buff_count == 0)
	{
		tcc_vsync_command(omx_private,TCC_LCDC_VIDEO_CLEAR_FRAME, pVDecInst->Display_Buff_ID[pVDecInst->out_index]);
		ALOGE("Video Buffer Clear Sync Fail : %d %lu , loopcount(%d)\n", buffer_id, pVDecInst->Display_Buff_ID[pVDecInst->out_index], loopCount);

		if(loopCount == MAX_CHECK_COUNT_FOR_CLEAR) bRingRollback = OMX_TRUE;

		buffer_id = tcc_vsync_command(omx_private,TCC_LCDC_VIDEO_GET_DISPLAYED, 0);
		while(buffer_id >= pVDecInst->Display_Buff_ID[pVDecInst->out_index] && pVDecInst->used_fifo_count>0)
		{
			clear_front_vpu_buffer(pVDecInst);
		}
		if(isVsyncEnable()) {
			tcc_vsync_command(omx_private, TCC_LCDC_VIDEO_SKIP_FRAME_START, 0);
			tcc_vsync_command(omx_private, TCC_LCDC_VIDEO_SKIP_FRAME_END, 0);
		}
	}

	return 0;
}
#endif

static
OMX_S32
GetFrameType(
	OMX_S32 lVideoType,
	OMX_S32 lPictureType,
	OMX_S32 lPictureStructure
	)
{
	int frameType = 0; //unknown

	switch ( lVideoType )
	{
	case STD_VC1 :
		switch( (lPictureType>>3) ) //Frame or // FIELD_INTERLACED(TOP FIELD)
		{
		case PIC_TYPE_I:	frameType = PIC_TYPE_I; break;//I
		case PIC_TYPE_P:	frameType = PIC_TYPE_P; break;//P
		case 2:				frameType = PIC_TYPE_B; break;//B //DSTATUS( "BI  :" );
		case 3:				frameType = PIC_TYPE_B; break;//B //DSTATUS( "B   :" );
		case 4:				frameType = PIC_TYPE_B; break;//B //DSTATUS( "SKIP:" );
		}
		if( lPictureStructure == 3)
		{
			switch( (lPictureType&0x7) ) // FIELD_INTERLACED(BOTTOM FIELD)
			{
			case PIC_TYPE_I:	frameType = PIC_TYPE_I; break;//I
			case PIC_TYPE_P:	frameType = PIC_TYPE_P; break;//P
			case 2:				frameType = PIC_TYPE_B; break;//B //DSTATUS( "BI  :" );
			case 3:				frameType = PIC_TYPE_B; break;//B //DSTATUS( "B   :" );
			case 4:				frameType = PIC_TYPE_B; break;//B //DSTATUS( "SKIP:" );
			}
		}
		break;
	case STD_MPEG4 :
		switch( lPictureType )
		{
		case PIC_TYPE_I:	frameType = PIC_TYPE_I;	break;//I
		case PIC_TYPE_P:	frameType = PIC_TYPE_P;	break;//P
		case PIC_TYPE_B:	frameType = PIC_TYPE_B;	break;//B
		case PIC_TYPE_B_PB: frameType = PIC_TYPE_B;	break;//B of Packed PB-frame
		}
		break;
	case STD_MPEG2 :
	default:
		switch( lPictureType & 0xF )
		{
		case PIC_TYPE_I:	frameType = PIC_TYPE_I;	break;//I
		case PIC_TYPE_P:	frameType = PIC_TYPE_P;	break;//P
		case PIC_TYPE_B:	frameType = PIC_TYPE_B;	break;//B
		}
	}
	return frameType;
}

static int for_iptv_info_init(OMX_COMPONENTTYPE * openmaxStandComp)
{
	omx_videodec_component_PrivateType *omx_private;
	unsigned int 	i;
	
	omx_private = openmaxStandComp->pComponentPrivate;

	omx_private->stbroken_frame_info.broken_frame_cnt =0;
	for (i=0; i<32; i++)
		omx_private->stbroken_frame_info.broken_frame_index[i] = 0x100;

	omx_private->stbframe_skipinfo.bframe_skipcnt = 0;
	for (i=0; i<32; i++)
		omx_private->stbframe_skipinfo.bframe_skipindex[i] = 0x100;

	omx_private->stfristframe_dsipInfo.FirstFrame_DispStatus =0;

	omx_private->stIPTVActiveModeInfo.IPTV_Activemode =1;
	omx_private->stIPTVActiveModeInfo.IPTV_Old_Activemode = 0;
	omx_private->stIPTVActiveModeInfo.IPTV_Mode_change_cnt =0;
	return 0;
}

OMX_ERRORTYPE omx_ring_videodec_SendBufferFunction (omx_base_PortType * openmaxStandPort, OMX_BUFFERHEADERTYPE * pBuffer)
{

	OMX_ERRORTYPE err;
	OMX_U32   portIndex;
	OMX_COMPONENTTYPE *omxComponent = openmaxStandPort->standCompContainer;
	omx_base_component_PrivateType *omx_base_component_Private =
		(omx_base_component_PrivateType *) omxComponent->pComponentPrivate;
#if NO_GST_OMX_PATCH
	unsigned int i;
#endif
	OMX_U32	nDecodeID;
	omx_videodec_component_PrivateType *omx_private = (omx_videodec_component_PrivateType *)omx_base_component_Private;
	OMX_U32 buffer_unique_id;
	_VIDEO_DECOD_INSTANCE_ *pVDecInst;

	portIndex = (openmaxStandPort->sPortParam.eDir == OMX_DirInput) ? pBuffer->nInputPortIndex : pBuffer->nOutputPortIndex;
	DEBUG (DEB_LEV_FUNCTION_NAME, "In %s portIndex %lu (%s)\n", __func__, portIndex, omx_base_component_Private->name);

	if (portIndex != openmaxStandPort->sPortParam.nPortIndex)
	{
		DEBUG (DEB_LEV_ERR, "In %s: wrong port for this operation portIndex=%d port->portIndex=%d\n", __func__,
			   (int) portIndex, (int) openmaxStandPort->sPortParam.nPortIndex);
		return OMX_ErrorBadPortIndex;
	}

	if (omx_base_component_Private->state == OMX_StateInvalid)
	{
		DEBUG (DEB_LEV_ERR, "In %s: we are in OMX_StateInvalid\n", __func__);
		return OMX_ErrorInvalidState;
	}

	if (omx_base_component_Private->state != OMX_StateExecuting &&
		omx_base_component_Private->state != OMX_StatePause && omx_base_component_Private->state != OMX_StateIdle)
	{
		DEBUG (DEB_LEV_ERR, "In %s: we are not in executing/paused/idle state, but in %d\n", __func__,
			   omx_base_component_Private->state);
		return OMX_ErrorIncorrectStateOperation;
	}
	if (!PORT_IS_ENABLED (openmaxStandPort)
		|| (PORT_IS_BEING_DISABLED (openmaxStandPort) && !PORT_IS_TUNNELED_N_BUFFER_SUPPLIER (openmaxStandPort))
		|| (omx_base_component_Private->transientState == OMX_TransStateExecutingToIdle
			&& (PORT_IS_TUNNELED (openmaxStandPort) && !PORT_IS_BUFFER_SUPPLIER (openmaxStandPort))))
	{
		DEBUG (DEB_LEV_ERR, "In %s: Port %d is disabled comp = %s, transientState : %d \n", __func__, (int) portIndex,
			   omx_base_component_Private->name, omx_base_component_Private->transientState);
		return OMX_ErrorIncorrectStateOperation;
	}

	/* Temporarily disable this check for gst-openmax */
#if NO_GST_OMX_PATCH
	{
		OMX_BOOL  foundBuffer = OMX_FALSE;
		if (pBuffer != NULL && pBuffer->pBuffer != NULL)
		{
			for (i = 0; i < openmaxStandPort->sPortParam.nBufferCountActual; i++)
			{
				if (pBuffer->pBuffer == openmaxStandPort->pInternalBufferStorage[i]->pBuffer)
				{
					foundBuffer = OMX_TRUE;
					break;
				}
			}
		}
		if (!foundBuffer)
		{
			return OMX_ErrorBadParameter;
		}
	}
#endif

	if ((err = checkHeader (pBuffer, sizeof (OMX_BUFFERHEADERTYPE))) != OMX_ErrorNone)
	{
		DEBUG (DEB_LEV_ERR, "In %s: received wrong buffer header on input port\n", __func__);
		return err;
	}

	/* And notify the buffer management thread we have a fresh new buffer to manage */
	if (!PORT_IS_BEING_FLUSHED (openmaxStandPort)
		&& !(PORT_IS_BEING_DISABLED (openmaxStandPort) && PORT_IS_TUNNELED_N_BUFFER_SUPPLIER (openmaxStandPort)))
	{
		/* Clear Displayed Index */
		nDecodeID = pBuffer->nDecodeID;
		pVDecInst = &omx_private->pVideoDecodInstance[nDecodeID];

		memcpy (&buffer_unique_id, pBuffer->pBuffer + 24, 4);
		OMX_U32 bufferID = pVDecInst->Send_Buff_ID[buffer_unique_id%VPU_REQUEST_BUFF_COUNT];
		OMX_U16 nDevID =  bufferID >> 16;
		OMX_U16 nClearDispIndex = bufferID & 0xFFFF;

		pVDecInst->Send_Buff_ID[buffer_unique_id%VPU_REQUEST_BUFF_COUNT] = 0;
		if (pVDecInst->avcodecInited)
		{
			if(nClearDispIndex>0)
			{
				OMX_S32 ret = 0;

#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
				// to change video output device
				int before_bOutputMode = bOutputMode;
				memcpy (&bOutputMode, pBuffer->pBuffer + 48, 4);
				if(before_bOutputMode != bOutputMode)
				{
					while(pVDecInst->used_fifo_count>0) {
						clear_front_vpu_buffer(pVDecInst);
					}
				}

				memcpy (&ret, pBuffer->pBuffer + 28, 4);

				if(pVDecInst->max_fifo_cnt != 0 && ret>=0 && bOutputMode)
				{
					pVDecInst->Display_index[pVDecInst->in_index] = bufferID;
					pVDecInst->Display_Buff_ID[pVDecInst->in_index] = buffer_unique_id;

					DPRINTF_DEC_STATUS("DispIdx Queue 0x%x", pVDecInst->Display_index[pVDecInst->in_index]);
					pVDecInst->in_index = (pVDecInst->in_index + 1) % pVDecInst->max_fifo_cnt;
					pVDecInst->used_fifo_count++;

					if(pVDecInst->used_fifo_count == pVDecInst->max_fifo_cnt)
	    			{
						int cleared_buff_id;
						int cleared_buff_count = 0;

						cleared_buff_id = tcc_vsync_command(omx_private,TCC_LCDC_VIDEO_GET_DISPLAYED, 0) ;  // TCC_LCDC_HDMI_GET_DISPLAYED 
    					if(cleared_buff_id < 0)
    					{
							clear_front_vpu_buffer(pVDecInst);
	    				}
	    				else
	    				{
							ret = clear_vpu_buffer(cleared_buff_id, omx_private, pVDecInst);
							if(ret  < 0 )
							{
								ALOGE( "[VDEC_BUF_FLAG_CLEAR] Idx = %lu, ret = %lu", pVDecInst->Display_index[pVDecInst->out_index], ret );
							}										
    					}
    				}
				}
				else
#endif				    
				{
					DPRINTF_DEC_STATUS("%s DispIdx Queue %d, %d", __func__, nDevID, nClearDispIndex);

					//nClearDispIndex++; //CAUTION !!! to avoid NULL(0) data insertion.
					if( queue_ex(pVDecInst->pVPUDisplayedIndexQueue, nClearDispIndex) == 0 )
					{
						//DPRINTF_DEC_STATUS("@ DispIdx Queue %d", pVDecInst->Display_index[pVDecInst->in_index]);
						nClearDispIndex--;
						ret = pVDecInst->gspfVDec (VDEC_BUF_FLAG_CLEAR, NULL, &nClearDispIndex, NULL, pVDecInst->pVdec_Instance);
						ALOGE ("%s VDEC_BUF_FLAG_CLEAR[%u] Err : [%lu] index : %u",__func__, nDevID, ret, nClearDispIndex);
					}
#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT					
					if(pVDecInst->max_fifo_cnt != 0 && ret == -2 && bOutputMode)
					{
						DPRINTF_DEC_STATUS("Renderer is closed");
						while(pVDecInst->used_fifo_count>0) {
							clear_front_vpu_buffer(pVDecInst);
						}
					}
#endif
				}
			}
		}			
		queue (openmaxStandPort->pBufferQueue, pBuffer);
		tsem_up (openmaxStandPort->pBufferSem);
		DEBUG (DEB_LEV_PARAMS, "In %s Signalling bMgmtSem Port Index=%d\n", __func__, (int) portIndex);
		tsem_up (omx_base_component_Private->bMgmtSem);
	}
	else if (PORT_IS_BUFFER_SUPPLIER (openmaxStandPort))
	{
		DEBUG (DEB_LEV_FULL_SEQ, "In %s: Comp %s received io:%d buffer\n",
			   __func__, omx_base_component_Private->name, (int) openmaxStandPort->sPortParam.nPortIndex);
		queue (openmaxStandPort->pBufferQueue, pBuffer);
		tsem_up (openmaxStandPort->pBufferSem);
	}
	else
	{	// If port being flushed and not tunneled then return error
		DEBUG (DEB_LEV_FULL_SEQ, "In %s \n", __func__);
		return OMX_ErrorIncorrectStateOperation;
	}
	return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_ring_videodec_MPEG2component_Init (OMX_COMPONENTTYPE * openmaxStandComp)
{
	return omx_ring_videodec_component_Constructor (openmaxStandComp, VIDEO_DEC_MPEG2_NAME);
}

OMX_ERRORTYPE omx_ring_videodec_H264component_Init (OMX_COMPONENTTYPE * openmaxStandComp)
{
	return omx_ring_videodec_component_Constructor (openmaxStandComp, VIDEO_DEC_H264_NAME);
}

/** The Constructor of the video decoder component
  * @param openmaxStandComp the component handle to be constructed
  * @param cComponentName is the name of the constructed component
  */
OMX_ERRORTYPE omx_ring_videodec_component_Constructor (OMX_COMPONENTTYPE * openmaxStandComp, OMX_STRING cComponentName)
{

	OMX_ERRORTYPE eError = OMX_ErrorNone;
	omx_videodec_component_PrivateType *omx_private;
	omx_base_video_PortType *inPort, *outPort;
	OMX_U32   i;

	if (!openmaxStandComp->pComponentPrivate)
	{
		DBUG_MSG ("In %s, allocating component\n", __func__);
		openmaxStandComp->pComponentPrivate = TCC_calloc (1, sizeof (omx_videodec_component_PrivateType));
		if (openmaxStandComp->pComponentPrivate == NULL)
		{
			return OMX_ErrorInsufficientResources;
		}
	}
	else
	{
		DBUG_MSG ("In %s, Error Component %x Already Allocated\n", __func__, (int) openmaxStandComp->pComponentPrivate);
	}

	omx_private = openmaxStandComp->pComponentPrivate;

	omx_private->pVideoDecodInstance = (_VIDEO_DECOD_INSTANCE_*)TCC_malloc(sizeof(_VIDEO_DECOD_INSTANCE_) * iTotalHandle);

	omx_private->ports = NULL;

	eError = omx_base_filter_Constructor (openmaxStandComp, cComponentName);

	omx_private->sPortTypesParam[OMX_PortDomainVideo].nStartPortNumber = 0;
	omx_private->sPortTypesParam[OMX_PortDomainVideo].nPorts = 4;

		/** Allocate Ports and call port constructor. */
	if (omx_private->sPortTypesParam[OMX_PortDomainVideo].nPorts && !omx_private->ports)
	{
		omx_private->ports =
			TCC_calloc (omx_private->sPortTypesParam[OMX_PortDomainVideo].nPorts, sizeof (omx_base_PortType *));
		if (!omx_private->ports)
		{
			return OMX_ErrorInsufficientResources;
		}
		for (i = 0; i < omx_private->sPortTypesParam[OMX_PortDomainVideo].nPorts; i++)
		{
			omx_private->ports[i] = TCC_calloc (1, sizeof (omx_base_video_PortType));
			if (!omx_private->ports[i])
			{
				return OMX_ErrorInsufficientResources;
			}
		}
	}

	base_video_port_Constructor (openmaxStandComp, &omx_private->ports[0], 0, OMX_TRUE);
	base_video_port_Constructor (openmaxStandComp, &omx_private->ports[1], 1, OMX_TRUE);
	base_video_port_Constructor (openmaxStandComp, &omx_private->ports[2], 2, OMX_FALSE);
	base_video_port_Constructor (openmaxStandComp, &omx_private->ports[3], 3, OMX_FALSE);

	/** now it's time to know the video coding type of the component */
	if (!strcmp (cComponentName, VIDEO_DEC_H264_NAME))
	{
		omx_private->pVideoDecodInstance[0].video_coding_type = OMX_VIDEO_CodingAVC;
		omx_private->pVideoDecodInstance[0].gsVDecInit.m_iBitstreamFormat = STD_AVC;

		omx_private->pVideoDecodInstance[1].video_coding_type = OMX_VIDEO_CodingAVC;
		omx_private->pVideoDecodInstance[1].gsVDecInit.m_iBitstreamFormat = STD_AVC;
	}
	else if (!strcmp (cComponentName, VIDEO_DEC_MPEG2_NAME))
	{
		omx_private->pVideoDecodInstance[0].video_coding_type = OMX_VIDEO_CodingMPEG2;
		omx_private->pVideoDecodInstance[0].gsVDecInit.m_iBitstreamFormat = STD_MPEG2;

		omx_private->pVideoDecodInstance[1].video_coding_type = OMX_VIDEO_CodingMPEG2;
		omx_private->pVideoDecodInstance[1].gsVDecInit.m_iBitstreamFormat = STD_MPEG2;
	}
	else if (!strcmp (cComponentName, VIDEO_DEC_BASE_NAME))
	{
		//Do nothing!!
	}	
	else
	{
		// IL client specified an invalid component name 
		return OMX_ErrorInvalidComponentName;
	}

	for(i=0; i<iTotalHandle; i++)
	{
		isDecodTimeStamp[i] = 0;
		memset (&omx_private->pVideoDecodInstance[i].gsVDecInput, 0x00, sizeof (vdec_input_t));
		memset (&omx_private->pVideoDecodInstance[i].gsVDecOutput, 0x00, sizeof (vdec_output_t));
		memset (&omx_private->pVideoDecodInstance[i].gsVDecInit, 0x00, sizeof (vdec_init_t));
		memset (&omx_private->pVideoDecodInstance[i].stRingBuffState.stRingBuffInfo, 0x00, sizeof (ringbuff_info_t));
		memset (&omx_private->pVideoDecodInstance[i].stRingBuffState.stInputInfoQueue, 0x00, sizeof (input_queue_t));

		omx_private->pVideoDecodInstance[i].pVdec_Instance = vdec_alloc_instance();
		omx_private->pVideoDecodInstance[i].gspfVDec = gspfVDecList[omx_private->pVideoDecodInstance[i].gsVDecInit.m_iBitstreamFormat];

		if (omx_private->pVideoDecodInstance[i].gspfVDec == 0)
		{
			return OMX_ErrorComponentNotFound;
		}
	}

	/** here we can override whatever defaults the base_component constructor set
	* e.g. we can override the function pointers in the private struct  
	*/

	/** Domain specific section for the ports.   
	* first we set the parameter common to both formats
	*/
	//common parameters related to input port.  
	inPort = (omx_base_video_PortType *) omx_private->ports[OMX_DXB_VIDEOCOMPONENT_INPORT_MAIN];
	inPort->sPortParam.nBufferSize = VIDEO_DXB_IN_BUFFER_SIZE;
	inPort->sPortParam.format.video.xFramerate = 30;
	inPort->sPortParam.format.video.nFrameWidth = 0;//AVAILABLE_MAX_WIDTH;
	inPort->sPortParam.format.video.nFrameHeight = 0;//AVAILABLE_MAX_HEIGHT;
	inPort->sPortParam.nBufferCountMin = 6;
	inPort->sPortParam.nBufferCountActual = 8;

	inPort = (omx_base_video_PortType *) omx_private->ports[OMX_DXB_VIDEOCOMPONENT_INPORT_SUB];
	inPort->sPortParam.nBufferSize = VIDEO_DXB_IN_BUFFER_SIZE;
	inPort->sPortParam.format.video.xFramerate = 30;
	inPort->sPortParam.format.video.nFrameWidth = 0;//AVAILABLE_MAX_WIDTH;
	inPort->sPortParam.format.video.nFrameHeight = 0;//AVAILABLE_MAX_HEIGHT;
	inPort->sPortParam.nBufferCountMin = 6;
	inPort->sPortParam.nBufferCountActual = 8;

	//common parameters related to output port
	outPort = (omx_base_video_PortType *) omx_private->ports[OMX_DXB_VIDEOCOMPONENT_OUTPORT];
	outPort->sPortParam.format.video.eColorFormat = OUTPUT_DECODED_COLOR_FMT;
	outPort->sPortParam.format.video.nFrameWidth = 0;//AVAILABLE_MAX_WIDTH;
	outPort->sPortParam.format.video.nFrameHeight = 0;//AVAILABLE_MAX_HEIGHT;
	//outPort->sPortParam.nBufferSize = (OMX_U32) (AVAILABLE_MAX_WIDTH * AVAILABLE_MAX_HEIGHT * 3 / 2);
	outPort->sPortParam.format.video.xFramerate = 30;
	outPort->sPortParam.nBufferCountMin = VIDEO_BUFFERCOUNT_MIN;
	outPort->sPortParam.nBufferCountActual = VIDEO_BUFFERCOUNT_ACTUAL;
	outPort->sPortParam.nBufferSize = 1024;
	outPort->sVideoParam.xFramerate = 30;
	outPort->Port_SendBufferFunction = omx_ring_videodec_SendBufferFunction;

	outPort = (omx_base_video_PortType *) omx_private->ports[OMX_DXB_VIDEOCOMPONENT_OUTPORT_SUB];
	outPort->sPortParam.format.video.eColorFormat = OUTPUT_DECODED_COLOR_FMT;
	outPort->sPortParam.format.video.nFrameWidth = 0;//AVAILABLE_MAX_WIDTH;
	outPort->sPortParam.format.video.nFrameHeight = 0;//AVAILABLE_MAX_HEIGHT;
	//outPort->sPortParam.nBufferSize = (OMX_U32) (AVAILABLE_MAX_WIDTH * AVAILABLE_MAX_HEIGHT * 3 / 2);
	outPort->sPortParam.format.video.xFramerate = 30;
	outPort->sPortParam.nBufferCountMin = VIDEO_BUFFERCOUNT_MIN;
	outPort->sPortParam.nBufferCountActual = VIDEO_BUFFERCOUNT_ACTUAL;
	outPort->sPortParam.nBufferSize = 1024;
	outPort->sVideoParam.xFramerate = 30;
	outPort->Port_SendBufferFunction = omx_ring_videodec_SendBufferFunction;

	if (!omx_private->avCodecSyncSem)
	{
		omx_private->avCodecSyncSem = TCC_malloc (sizeof (tsem_t));
		if (omx_private->avCodecSyncSem == NULL)
		{
			return OMX_ErrorInsufficientResources;
		}
		tsem_init (omx_private->avCodecSyncSem, 0);
	}

	/** general configuration irrespective of any video formats
	* setting other parameters of omx_videodec_component_private  
	*/
	omx_private->BufferMgmtCallback = omx_ring_videodec_component_BufferMgmtCallback;

	/** initializing the codec context etc that was done earlier by ffmpeglibinit function */
	omx_private->messageHandler = omx_ring_videodec_component_MessageHandler;
	omx_private->destructor = omx_ring_videodec_component_Destructor;
	//omx_private->BufferMgmtFunction = omx_twoport_filter_component_BufferMgmtFunction;
	omx_private->BufferMgmtFunction = omx_ring_video_twoport_filter_component_BufferMgmtFunction;
	openmaxStandComp->SetParameter = omx_ring_videodec_component_SetParameter;
	openmaxStandComp->GetParameter = omx_ring_videodec_component_GetParameter;
	openmaxStandComp->SetConfig = omx_ring_videodec_component_SetConfig;
	openmaxStandComp->ComponentRoleEnum = omx_ring_videodec_component_ComponentRoleEnum;
	openmaxStandComp->GetExtensionIndex = omx_ring_videodec_component_GetExtensionIndex;

	omx_private->gHDMIOutput = OMX_FALSE;

	OMX_U32 hwc_id = (getHWCID() < iTotalHandle) ? iTotalHandle : 0;
	for(i=0; i<iTotalHandle; i++)
	{
		memset(&omx_private->pVideoDecodInstance[i].stRingBuffState, 0x00, sizeof(ringbuff_state_t));

		memset(omx_private->pVideoDecodInstance[i].Display_index, 0x00, sizeof(OMX_U32)*VPU_BUFF_COUNT);
		memset(omx_private->pVideoDecodInstance[i].Display_Buff_ID, 0x00, sizeof(OMX_U32)*VPU_BUFF_COUNT);
		memset(omx_private->pVideoDecodInstance[i].Send_Buff_ID, 0x00, sizeof(OMX_U32)*VPU_REQUEST_BUFF_COUNT);

		omx_private->pVideoDecodInstance[i].max_fifo_cnt = VPU_BUFF_COUNT;
		omx_private->pVideoDecodInstance[i].buffer_unique_id = 0;
		omx_private->pVideoDecodInstance[i].ulDecoderUID = hwc_id + i;

		omx_private->pVideoDecodInstance[i].pVPUDisplayedIndexQueue = (queue_t *)TCC_calloc(1,sizeof(queue_t));
		queue_init_ex(omx_private->pVideoDecodInstance[i].pVPUDisplayedIndexQueue, 32); //Max size is 32
		omx_private->pVideoDecodInstance[i].avcodecReady = OMX_FALSE;
		omx_private->pVideoDecodInstance[i].video_dec_idx = 0;
#ifdef VIDEO_USER_DATA_PROCESS
		omx_private->pVideoDecodInstance[i].pUserData_List_Array = TCC_malloc(sizeof(video_userdata_list_t)*MAX_USERDATA_LIST_ARRAY);
		UserDataCtrl_Init(omx_private->pVideoDecodInstance[i].pUserData_List_Array); 
#endif
		memset(omx_private->pVideoDecodInstance[i].dec_disp_info, 0x00, sizeof(dec_disp_info_t)*32);
		memset(&omx_private->pVideoDecodInstance[i].dec_disp_info_input, 0x00, sizeof(dec_disp_info_input_t));
		omx_private->pVideoDecodInstance[i].bVideoStarted = OMX_FALSE;
		omx_private->pVideoDecodInstance[i].bVideoPaused = OMX_FALSE;
#ifdef TS_TIMESTAMP_CORRECTION
		memset(&omx_private->pVideoDecodInstance[i].gsTSPtsInfo, 0x00, sizeof(ts_pts_ctrl));
#endif
		memset(&omx_private->pVideoDecodInstance[i].stVideoStart, 0x00, sizeof(VideoStartInfo));
		omx_private->pVideoDecodInstance[i].bitrate_mbps = 20; //default 20Mbps

		omx_private->pVideoDecodInstance[i].isVPUClosed  = OMX_TRUE;
	}
	memset(&(omx_private->gsMPEG2PtsInfo), 0x00, sizeof(mpeg2_pts_ctrl));

	omx_private->gVideo_FrameRate = 30;

	omx_private->iVsyncMode = isVsyncSupport();
	bOutputMode = isVsyncEnable();

	omx_private->iDisplayID = 0;
	omx_private->Resolution_Change = 0;

	omx_private->stVideoSubFunFlag.SupportFieldDecoding =0;
	omx_private->stVideoSubFunFlag.SupportIFrameSearchMode =1;
	omx_private->stVideoSubFunFlag.SupprotUsingErrorMB =0;
	omx_private->stVideoSubFunFlag.SupprotDirectDsiplay =0;

#if 1	//for Dual-decoding
	omx_private->stPreVideoInfo.iDecod_Instance = -1;
	omx_private->stPreVideoInfo.iDecod_status = -1;
	omx_private->stPreVideoInfo.uDecod_time = 0;
#endif

	if ((mFBdevice = open(FB0_DEVICE, O_RDWR)) <= 0) {
		ALOGE("can't open[%s] '%s'", strerror(errno), FB0_DEVICE);
	}

	for_iptv_info_init(openmaxStandComp);
	return eError;
}


/** The destructor of the video decoder component
  */
OMX_ERRORTYPE omx_ring_videodec_component_Destructor (OMX_COMPONENTTYPE * openmaxStandComp)
{
	omx_videodec_component_PrivateType *omx_private = openmaxStandComp->pComponentPrivate;
	OMX_U32   i;
	if (omx_private->avCodecSyncSem)
	{
		tsem_deinit (omx_private->avCodecSyncSem);
		TCC_free (omx_private->avCodecSyncSem);
		omx_private->avCodecSyncSem = NULL;
	}

	/* frees port/s */
	if (omx_private->ports)
	{
		for (i = 0; i < omx_private->sPortTypesParam[OMX_PortDomainVideo].nPorts; i++)
		{
			if (omx_private->ports[i])
				omx_private->ports[i]->PortDestructor (omx_private->ports[i]);
		}
		TCC_free (omx_private->ports);
		omx_private->ports = NULL;
	}

	DBUG_MSG ("Destructor of video decoder component is called\n");

	if (openmaxStandComp)
		omx_base_filter_Destructor (openmaxStandComp);

	for(i=0; i<iTotalHandle; i++)
	{
		queue_deinit(omx_private->pVideoDecodInstance[i].pVPUDisplayedIndexQueue);
		TCC_free(omx_private->pVideoDecodInstance[i].pVPUDisplayedIndexQueue);	
#ifdef VIDEO_USER_DATA_PROCESS
		if(omx_private->pVideoDecodInstance[i].pUserData_List_Array)
		{
			UserDataCtrl_ResetAll(omx_private->pVideoDecodInstance[i].pUserData_List_Array);
		}
#endif
		if(omx_private->pVideoDecodInstance[i].pVdec_Instance)
			vdec_release_instance(omx_private->pVideoDecodInstance[i].pVdec_Instance);
	}

	if(mFBdevice >= 0)
	{
		close(mFBdevice);
		mFBdevice = -1;
	}

	return OMX_ErrorNone;
}

/** It initializates the FFmpeg framework, and opens an FFmpeg videodecoder of type specified by IL client 
  */
OMX_ERRORTYPE omx_ring_videodec_component_LibInit (omx_videodec_component_PrivateType * omx_private, OMX_U8 uiDecoderID)
{
	int i;
	OMX_U32   target_codecID;
	OMX_S8    value[PROPERTY_VALUE_MAX];
	OMX_U32   uiHDMIOutputMode = 0;

	tsem_up (omx_private->avCodecSyncSem);

	{
		omx_private->pVideoDecodInstance[uiDecoderID].iDecod_status = -1;
		omx_private->pVideoDecodInstance[uiDecoderID].avcodecInited = 0;
		omx_private->pVideoDecodInstance[uiDecoderID].container_type = 0;

		omx_private->pVideoDecodInstance[uiDecoderID].iDecod_status = -1;

		/* reset VPU ring-buffer state */
		ResetRingBuffer(&omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.stRingBuffInfo);

		/* reset input/output information managers */
		ClearInputQueue(&omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.stInputInfoQueue);

		//ClearDispIdxQueue(&omx_private->pVideoDecodInstance[uiDecoderID].stDispIdxQueue);
		//ResetDispInfoManager(&omx_private->pVideoDecodInstance[uiDecoderID].stDispInfoMgr);

		omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.lInputCount = 0;
		omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.lRollbackCheckCount = 0;
		omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.llQueuedStartTime = -1;
		omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.llFeedLimitTimeDiff = FEED_LIMIT_TIMEDIFF_INIT;
		omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.lFrameSuccessCount = 0;
		omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.lNewFrameRate = 0;

		if (omx_private->pVideoDecodInstance[uiDecoderID].video_coding_type == OMX_VIDEO_CodingMPEG2)
		{
			/*below is default value fot broadcastring
			 */
			omx_private->pVideoDecodInstance[uiDecoderID].container_type = CONTAINER_TYPE_NONE;
			//omx_private->container_type = CONTAINER_TYPE_MPG;     
			omx_private->pVideoDecodInstance[uiDecoderID].cdmx_info.m_sVideoInfo.m_iFrameRate = 25;
		}
		else if (omx_private->pVideoDecodInstance[uiDecoderID].video_coding_type == OMX_VIDEO_CodingAVC)
		{
			/*below is default value fot broadcastring
			 */
			omx_private->pVideoDecodInstance[uiDecoderID].container_type = CONTAINER_TYPE_NONE;
			//omx_private->container_type = CONTAINER_TYPE_TS;
			omx_private->pVideoDecodInstance[uiDecoderID].cdmx_info.m_sVideoInfo.m_iFrameRate = 25;
		}
		omx_private->pVideoDecodInstance[uiDecoderID].guiDisplayBufFullCount = 0;
	}

	omx_private->pVideoDecodInstance[uiDecoderID].out_index = 0;
	omx_private->pVideoDecodInstance[uiDecoderID].in_index = 0;
#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
	omx_private->pVideoDecodInstance[uiDecoderID].used_fifo_count = 0;
#endif

	property_get ("tcc.sys.output_mode_detected", value, "");
	uiHDMIOutputMode = atoi (value);
	if (uiHDMIOutputMode == OUTPUT_HDMI)
	{
		ALOGD ("HDMI output enabled");
		omx_private->gHDMIOutput = OMX_TRUE;
	}

	omx_private->guiSkipBframeEnable = 0;
	omx_private->guiSkipBFrameNumber = 0;
	return OMX_ErrorNone;
}

/** It Deinitializates the ffmpeg framework, and close the ffmpeg video decoder of selected coding type
  */
void omx_ring_videodec_component_LibDeinit (omx_videodec_component_PrivateType * omx_private, OMX_U8 uiDecoderID)
{
	int ret, i, cnt=0;

	if (uiDecoderID >= iTotalHandle) {
		ALOGE ("In %s, Invalid decoder id(%d)\n", __func__, uiDecoderID);
		return;
	}

	isDecodTimeStamp[uiDecoderID] = 0;

	if(omx_private->pVideoDecodInstance[uiDecoderID].isVPUClosed == OMX_FALSE)
	{
		omx_private->pVideoDecodInstance[uiDecoderID].iDecod_status = -1;

		/* reset VPU ring-buffer state */
		ResetRingBuffer(&omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.stRingBuffInfo);
		/* reset input/output information managers */
		ClearInputQueue(&omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.stInputInfoQueue);

		omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.lInputCount = 0;
		omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.lRollbackCheckCount = 0;
		omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.llQueuedStartTime = -1;
		omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.lFrameSuccessCount = 0;
		omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.lNewFrameRate = 0;

		omx_private->pVideoDecodInstance[uiDecoderID].avcodecInited = 0;
		for(i=0; i<iTotalHandle; i++)
		{
			if(omx_private->pVideoDecodInstance[i].avcodecInited)
			{
				cnt++;
			}
		}

		omx_private->pVideoDecodInstance[uiDecoderID].gspfVDec (VDEC_SW_RESET, NULL, NULL, NULL, omx_private->pVideoDecodInstance[uiDecoderID].pVdec_Instance);
//		if(uiDecoderID==0 && omx_private->Resolution_Change == 0 && cnt<1)
//		{
//			omx_private->pVideoDecodInstance[uiDecoderID].gspfVDec (VDEC_HW_RESET, uiDecoderID, NULL, &omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput, omx_private->pVideoDecodInstance[uiDecoderID].pVdec_Instance);
//		}
		if(omx_private->pVideoDecodInstance[uiDecoderID].gspfVDec (VDEC_CLOSE, uiDecoderID, NULL, &omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput, omx_private->pVideoDecodInstance[uiDecoderID].pVdec_Instance) == 0)
			omx_private->pVideoDecodInstance[uiDecoderID].isVPUClosed = OMX_TRUE;

		//(*(omx_private->callbacks->EventHandler)) (openmaxStandComp, omx_private->callbackData,OMX_EventError, OMX_ErrorHardware, 0, NULL);

#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
		bRingRollback = OMX_FALSE;
		if(omx_private->iVsyncMode) {
//			tcc_vsync_command(omx_private, TCC_LCDC_VIDEO_END_VSYNC, NULL);
			omx_private->pVideoDecodInstance[uiDecoderID].in_index = 0;
			omx_private->pVideoDecodInstance[uiDecoderID].out_index = 0;
			omx_private->pVideoDecodInstance[uiDecoderID].used_fifo_count = 0;
//			tcc_vsync_command(omx_private, TCC_LCDC_VIDEO_START_VSYNC, VPU_BUFF_COUNT);
		}
#endif
	}
}

/** The Initialization function of the video decoder
  */
OMX_ERRORTYPE omx_ring_videodec_component_Initialize (OMX_COMPONENTTYPE * openmaxStandComp)
{

	omx_videodec_component_PrivateType *omx_private = openmaxStandComp->pComponentPrivate;
	OMX_ERRORTYPE eError = OMX_ErrorNone;

	/** Temporary First Output buffer size */
	omx_private->inputCurrBuffer = NULL;
	omx_private->inputCurrLength = 0;
	omx_private->isFirstBuffer = 1;
	return eError;
}

/** The Deinitialization function of the video decoder  
  */
OMX_ERRORTYPE omx_ring_videodec_component_Deinit (OMX_COMPONENTTYPE * openmaxStandComp)
{
	int i;
	omx_videodec_component_PrivateType *omx_private = openmaxStandComp->pComponentPrivate;
	OMX_ERRORTYPE eError = OMX_ErrorNone;

	for(i=0; i<iTotalHandle; i++)
	{
		if (omx_private->pVideoDecodInstance[i].avcodecReady)
		{
			omx_private->pVideoDecodInstance[i].avcodecReady = OMX_FALSE;
			omx_ring_videodec_component_LibDeinit (omx_private, i);
		}
	#ifdef VIDEO_USER_DATA_PROCESS
		if(omx_private->pVideoDecodInstance[i].pUserData_List_Array)
			UserDataCtrl_ResetAll(omx_private->pVideoDecodInstance[i].pUserData_List_Array); 
	#endif
	}

	return eError;
}

OMX_ERRORTYPE omx_ring_videodec_component_Change (OMX_COMPONENTTYPE * openmaxStandComp, OMX_STRING cComponentName, OMX_U8 uiDecoderID)
{

	OMX_ERRORTYPE eError = OMX_ErrorNone;
	omx_videodec_component_PrivateType *omx_private;
	omx_base_video_PortType *inPort, *outPort;
	OMX_U32   i;

	if (!openmaxStandComp->pComponentPrivate)
	{
		DBUG_MSG("In %s, Error Insufficient Resources\n", __func__);
		
		return OMX_ErrorInsufficientResources;
	}

	omx_private = openmaxStandComp->pComponentPrivate;

	memset (&omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput, 0x00, sizeof (vdec_input_t));
	memset (&omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput, 0x00, sizeof (vdec_output_t));
	memset (&omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit, 0x00, sizeof (vdec_init_t));
	omx_private->pVideoDecodInstance[uiDecoderID].bitrate_mbps = 20; //default 20Mbps
   
	/** now it's time to know the video coding type of the component */
	if (!strcmp (cComponentName, VIDEO_DEC_H264_NAME))
	{
		omx_private->pVideoDecodInstance[uiDecoderID].video_coding_type = OMX_VIDEO_CodingAVC;
		omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat = STD_AVC;
	}
	else if (!strcmp (cComponentName, VIDEO_DEC_MPEG2_NAME))
	{
		omx_private->pVideoDecodInstance[uiDecoderID].video_coding_type = OMX_VIDEO_CodingMPEG2;
		omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat = STD_MPEG2;
	}
	else
	{
		// IL client specified an invalid component name 
		return OMX_ErrorInvalidComponentName;
	}

	omx_private->pVideoDecodInstance[uiDecoderID].gspfVDec = gspfVDecList[omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat];

	/* common parameters related to output port */
	if (uiDecoderID==0) outPort = (omx_base_video_PortType *) omx_private->ports[OMX_DXB_VIDEOCOMPONENT_OUTPORT];
	else outPort = (omx_base_video_PortType *) omx_private->ports[OMX_DXB_VIDEOCOMPONENT_OUTPORT_SUB];
	/* settings of output port parameter definition */
		outPort->sPortParam.format.video.eColorFormat = OUTPUT_DECODED_COLOR_FMT;

	if (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat >= STD_AVC)
		omx_private->pVideoDecodInstance[uiDecoderID].gspfVDec = gspfVDecList[omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat];
	else
		omx_private->pVideoDecodInstance[uiDecoderID].gspfVDec = 0;

	if (omx_private->pVideoDecodInstance[uiDecoderID].gspfVDec == 0)
	{
		return OMX_ErrorComponentNotFound;
	}

	if (!omx_private->avCodecSyncSem)
	{
		omx_private->avCodecSyncSem = TCC_malloc (sizeof (tsem_t));
		if (omx_private->avCodecSyncSem == NULL)
		{
			return OMX_ErrorInsufficientResources;
		}
		tsem_init (omx_private->avCodecSyncSem, 0);
	}
	return eError;
}

static int get_total_handle_cnt(OMX_COMPONENTTYPE * openmaxStandComp)
{
	int i, cnt=0;
	omx_videodec_component_PrivateType *omx_private = openmaxStandComp->pComponentPrivate;

	for(i=0; i<iTotalHandle; i++)
	{
		if(omx_private->pVideoDecodInstance[i].avcodecInited)
		{
			cnt++;
		}
	}
	return cnt;
}

static int isPortChange (OMX_COMPONENTTYPE * openmaxStandComp, OMX_U8 uiDecoderID)
{
	omx_videodec_component_PrivateType *omx_private = openmaxStandComp->pComponentPrivate;
	omx_base_video_PortType *outPort;
	int   width, height, decoder_delay;
	int       ret = 0;
	int i;

	if (uiDecoderID == 0)
		outPort = (omx_base_video_PortType *)omx_private->ports[OMX_DXB_VIDEOCOMPONENT_OUTPORT];
	else
		outPort= (omx_base_video_PortType *)omx_private->ports[OMX_DXB_VIDEOCOMPONENT_OUTPORT_SUB];

   	width = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iWidth;
	height = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iHeight;

	if(width && height)
	{
		if (( omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iPicWidth != width) ||
				( omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iPicHeight != height))
		{
			/* if image size of encoded stream is changed ? */
			ALOGD("[%d]Resolution change detected (%dx%d ==> %dx%d)", uiDecoderID,
					omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iPicWidth,
					omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iPicHeight,
                        width,
                        height);
			outPort->sPortParam.format.video.nFrameWidth = 0;
			outPort->sPortParam.format.video.nFrameHeight = 0;

			omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iPicWidth = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iPicWidth = width;
			omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iPicHeight = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iPicHeight = height;
			omx_private->Resolution_Change = 1;

			ret = -2;

		} else {
			if( outPort->sPortParam.format.video.nFrameWidth && outPort->sPortParam.format.video.nFrameHeight )
			{
				if (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat == STD_AVC) {
					width -= omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropLeft;
					width -= omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropRight;
					height -= omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropBottom;
					height -= omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropTop;
				}
				if ((outPort->sPortParam.format.video.nFrameWidth != width) ||
						(outPort->sPortParam.format.video.nFrameHeight != height)) {
					ALOGD("[%d]%d - %d - %d, %d - %d - %d", uiDecoderID,
							omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iWidth,
							omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropLeft,
							omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropRight,
							omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iHeight,
							omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropTop,
							omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropBottom);
					ALOGD("[%d]Resolution change detected (%ldx%ld ==> %dx%d)", uiDecoderID,
							outPort->sPortParam.format.video.nFrameWidth,
							outPort->sPortParam.format.video.nFrameHeight,
							width,
							height);

					ret = 1;
				}
			}
			else
			{
				if (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat == STD_AVC) {
					width -= omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropLeft;
					width -= omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropRight;
					height -= omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropBottom;
					height -= omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropTop;
				}
				ret = 1;
			}
		}
	}

	if (width > AVAILABLE_MAX_WIDTH || ((width * height) > AVAILABLE_MAX_REGION))
	{
		ALOGE("[%d]%d x %d ==> MAX-Resolution(%d x %d) over!!", uiDecoderID,
			width, height, AVAILABLE_MAX_WIDTH, AVAILABLE_MAX_HEIGHT);
		ret = -1;
	}

	if (width < AVAILABLE_MIN_WIDTH || height < AVAILABLE_MIN_HEIGHT)
	{
		ALOGE("[d]%d x %d ==> MIN-Resolution(%d x %d) less!!", uiDecoderID,
			width, height, AVAILABLE_MIN_WIDTH, AVAILABLE_MIN_HEIGHT);
		ret = -1;
	}

	if (bChangeDisplayID==OMX_TRUE && ret != 1) {
		//this routine shoud be run only when ret is not 1
		int is_progressive;
		int iaspect_ratio;

		if (omx_private->iDisplayID == uiDecoderID) {
			if (omx_private->pVideoDecodInstance[uiDecoderID].avcodecInited) {
				is_progressive = omx_private->stVideoDefinition.bProgressive;
				iaspect_ratio= omx_private->stVideoDefinition.nAspectRatio;

				if (omx_private->iDisplayID == uiDecoderID) {
						(*(omx_private->callbacks->EventHandler)) (openmaxStandComp,
										   omx_private->callbackData,
										   OMX_EventVendorVideoChange,
										   is_progressive, iaspect_ratio, outPort);
				}

				(*(omx_private->callbacks->EventHandler)) (openmaxStandComp,
										   omx_private->callbackData,
										   OMX_EventPortSettingsChanged,
										   is_progressive, iaspect_ratio, outPort);

			#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
				if (isVsyncEnable())
				{
					tcc_vsync_command(omx_private, TCC_LCDC_VIDEO_SKIP_FRAME_START, 0);
					tcc_vsync_command(omx_private, TCC_LCDC_VIDEO_SKIP_FRAME_END, 0);
				}
			#endif
			}
			bChangeDisplayID = OMX_FALSE;
		}
	}

	if (ret == 1)
	{
		int  is_progressive, iaspect_ratio;
		ALOGD("Interlaced Info(%d,%d,%d,%d)", 
			omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iInterlace,
			omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iM2vProgressiveFrame, 
			omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iPictureStructure, 
			omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iInterlacedFrame);

		/* Check interlaced/progressive frame */
		switch( omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat )
		{
			case STD_AVC :
				{
					if((omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iM2vProgressiveFrame == 0
			            && omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iPictureStructure == 0x3)
			                || omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iInterlacedFrame
						|| ((omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iPictureStructure  ==1) 
						&& (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iInterlace ==0))
			                )
			        {
						if(omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iPictureStructure ==1)
							omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iTopFieldFirst = 1;

						is_progressive = 0;
					}
					else
					{
					    is_progressive = 1;
					}
				}
				break;
			case STD_MPEG2 :
			default :
				{
					if( omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iInterlace
			            || ( ( omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iM2vProgressiveFrame == 0 
			                   && omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iPictureStructure == 0x3 ) 
			                 || omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iInterlacedFrame ) )
					{
						is_progressive = 0;
					}
					else
					{
						is_progressive = 1;
					}
				}
				break;
		}

		if( is_progressive )
		{
			ALOGD("NON-Interlaced Frame!!!");
		}
		else
		{
			ALOGD("Interlaced Frame!!!");
		}

		omx_private->iInterlaceInfo = is_progressive;

		//outPort->bIsPortChanged = OMX_TRUE;       //This make a trobule in DVBT Player.
		outPort->sPortParam.format.video.nFrameWidth = width;
		outPort->sPortParam.format.video.nFrameHeight = height;

		iaspect_ratio = -1;
		if (omx_private->pVideoDecodInstance[uiDecoderID].video_coding_type == OMX_VIDEO_CodingMPEG2)
		{
			if(omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iAspectRateInfo)
			{
				iaspect_ratio = 0; // 16 : 9
				if(omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iAspectRateInfo == 2)
					iaspect_ratio = 1; // 4 : 3
			}
		}			

		omx_private->stVideoDefinition.nAspectRatio = iaspect_ratio;
		if (omx_private->iDisplayID == uiDecoderID) {
#ifdef TS_TIMESTAMP_CORRECTION
			ALOGD("[%d]Video Aspec Ratio %d (0[16:9], 1[4:3] :: %d)", uiDecoderID, iaspect_ratio, omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iAspectRateInfo);
		    decoder_delay = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iMinFrameBufferCount *\
		                                                     (omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iPTSInterval);
			decoder_delay += (int)omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.llFeedLimitTimeDiff;
			ALOGD("[%d]Video Decoder Delay %d ms : [min buffer:%d, interval:%d(us), FeedLimit:%lld(us)]", uiDecoderID, decoder_delay/1000,\
	                                                        omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iMinFrameBufferCount,\
	                                                        omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iPTSInterval,\
	                                                        omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.llFeedLimitTimeDiff);
#endif
			(*(omx_private->callbacks->EventHandler)) (openmaxStandComp,
														omx_private->callbackData,
														OMX_EventVendorSetSTCDelay,
														decoder_delay, 0, NULL);

			(*(omx_private->callbacks->EventHandler)) (openmaxStandComp,
														omx_private->callbackData,
														OMX_EventVendorVideoChange,
														is_progressive, iaspect_ratio, outPort);
		}

		(*(omx_private->callbacks->EventHandler)) (openmaxStandComp,
								   omx_private->callbackData,
								   OMX_EventPortSettingsChanged,
								   is_progressive, iaspect_ratio, outPort);

		omx_private->stVideoDefinition.nFrameWidth = width;
		omx_private->stVideoDefinition.nFrameHeight = height;
		omx_private->stVideoDefinition.nFrameRate = outPort->sPortParam.format.video.xFramerate;
		omx_private->stVideoDefinition.bProgressive = is_progressive;
		ALOGI ("ReSize Needed!! %ld x %ld -> %ld x %ld \n", outPort->sPortParam.format.video.nFrameWidth,
			  outPort->sPortParam.format.video.nFrameHeight, width, height);

		//adaption of DecoderID (for various resolution)
		if (uiDecoderID == 0){

			omx_private->stVideoDefinition_dual.nFrameWidth = width;
			omx_private->stVideoDefinition_dual.nFrameHeight = height;
			omx_private->stVideoDefinition_dual.MainDecoderID = uiDecoderID;
			omx_private->stVideoDefinition_dual.nAspectRatio= iaspect_ratio;
			omx_private->stVideoDefinition_dual.nFrameRate = outPort->sPortParam.format.video.xFramerate;
			
		} else {

			omx_private->stVideoDefinition_dual.nSubFrameWidth = width;
			omx_private->stVideoDefinition_dual.nSubFrameHeight = height;
			omx_private->stVideoDefinition_dual.SubDecoderID = uiDecoderID;
			omx_private->stVideoDefinition_dual.nSubAspectRatio= iaspect_ratio;
			
		}
		omx_private->stVideoDefinition_dual.DisplayID = omx_private->iDisplayID;

	if(!omx_private->Resolution_Change)
	{
		;
#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
        {
//            if(isVsyncEnable()) {
//                tcc_vsync_command(omx_private, TCC_LCDC_VIDEO_START_VSYNC, VPU_BUFF_COUNT);
//            }
        }
#endif		
	}

		if (omx_private->stVideoDefinition_dual.MainDecoderID == omx_private->stVideoDefinition_dual.SubDecoderID) {
			
				(*(omx_private->callbacks->EventHandler)) (openmaxStandComp,
								omx_private->callbackData,
											OMX_EventVendorVideoUserDataAvailable,
											OMX_BUFFERFLAG_EXTRADATA, 0, &omx_private->stVideoDefinition);
							
		} else {

				(*(omx_private->callbacks->EventHandler)) (openmaxStandComp,
								omx_private->callbackData,
											OMX_EventVendorVideoUserDataAvailable,
											OMX_BUFFERFLAG_EXTRADATA, 0, &omx_private->stVideoDefinition_dual);
		}

		
		ALOGW ("[%d]VideoSize %ld x %ld -> %ld x %ld \n", uiDecoderID, omx_private->stVideoDefinition.nFrameWidth,
			  omx_private->stVideoDefinition.nFrameHeight, width, height);
	
	}
	

		if (ret != 1) {

			if (omx_private->stVideoDefinition_dual.MainDecoderID != omx_private->stVideoDefinition_dual.SubDecoderID) {

				if (omx_private->stVideoDefinition_dual.DisplayID != omx_private->iDisplayID) {

					omx_private->stVideoDefinition_dual.DisplayID = omx_private->iDisplayID;
					(*(omx_private->callbacks->EventHandler)) (openmaxStandComp,
											omx_private->callbackData,
														OMX_EventVendorVideoUserDataAvailable,
														OMX_BUFFERFLAG_EXTRADATA, 0, &omx_private->stVideoDefinition_dual);
					ALOGW ("send event check (when seamless switching)\n");
				}
			}
		}
			
	return ret;
}

static int
disp_pic_info_sort( void* pParam1 )
{
	int i;
	dec_disp_info_ctrl_t  *pInfoCtrl = (dec_disp_info_ctrl_t*)pParam1;
	int usedIdx, startIdx, regIdx;

	usedIdx=0;
	startIdx = -1;
	for( i=0 ; i<32 ; i++ )
	{
		if( pInfoCtrl->m_iRegIdxPTS[i] > -1 )
		{
			if( startIdx == -1 )
			{
				startIdx = i;
			}
			usedIdx++;
		}
	}
	pInfoCtrl->m_iUsedIdxPTS = usedIdx;

	if( usedIdx > 0 )
	{
		regIdx = 0;
		for( i=startIdx ; i<32 ; i++ )		
		{
			if( pInfoCtrl->m_iRegIdxPTS[i] > -1 )
			{
				if( i != regIdx )
				{
					void * pswap;
					int iswap;

					iswap = pInfoCtrl->m_iRegIdxPTS[regIdx];
					pswap = pInfoCtrl->m_pRegInfoPTS[regIdx];
								
					pInfoCtrl->m_iRegIdxPTS[regIdx] = pInfoCtrl->m_iRegIdxPTS[i];
					pInfoCtrl->m_pRegInfoPTS[regIdx] = pInfoCtrl->m_pRegInfoPTS[i];

					pInfoCtrl->m_iRegIdxPTS[i] = iswap;
					pInfoCtrl->m_pRegInfoPTS[i] = pswap;
				}
				regIdx++;
				if( regIdx == usedIdx )
					break;
			}
		}
	}

	return usedIdx;
}

static void
disp_pic_info (int Opcode, void *pParam1, void *pParam2, void *pParam3,
			   omx_videodec_component_PrivateType * omx_private, OMX_U8 uiDecoderID)
{
	int       i;
	dec_disp_info_ctrl_t *pInfoCtrl = (dec_disp_info_ctrl_t *) pParam1;
	dec_disp_info_t *pInfo = (dec_disp_info_t *) pParam2;
	dec_disp_info_input_t *pInfoInput = (dec_disp_info_input_t *) pParam3;

	switch (Opcode)
	{
	case CVDEC_DISP_INFO_INIT:	//init.
		pInfoCtrl->m_iStdType = pInfoInput->m_iStdType;
		pInfoCtrl->m_iFmtType = pInfoInput->m_iFmtType;
		pInfoCtrl->m_iTimeStampType = pInfoInput->m_iTimeStampType;		
        
#ifdef TS_TIMESTAMP_CORRECTION
        omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iLatestPTS = 0;
        omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iRamainingDuration = 0;
        if(omx_private->pVideoDecodInstance[uiDecoderID].cdmx_info.m_sVideoInfo.m_iFrameRate!=0)
            omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iPTSInterval = (((1000 * 1000) << 10) / omx_private->pVideoDecodInstance[uiDecoderID].cdmx_info.m_sVideoInfo.m_iFrameRate) >> 10;
#endif


	case CVDEC_DISP_INFO_RESET:	//reset
		for (i = 0; i < 32; i++)
		{
			pInfoCtrl->m_iRegIdxPTS[i] = -1;	//unused
			pInfoCtrl->m_pRegInfoPTS[i] = (void *) &pInfo[i];
		}
		pInfoCtrl->m_iUsedIdxPTS = 0;

		if (pInfoCtrl->m_iTimeStampType == CDMX_DTS_MODE)	//Decode Timestamp (Decode order)
		{
			pInfoCtrl->m_iDecodeIdxDTS = 0;
			pInfoCtrl->m_iDispIdxDTS = 0;
			for (i = 0; i < 32; i++)
			{
				pInfoCtrl->m_iDTS[i] = 0;
			}
		}

#ifdef EXT_V_DECODER_TR_TEST
		memset (&gsEXT_F_frame_time, 0, sizeof (EXT_F_frame_time_t));
		gsextReference_Flag = 1;
		gsextP_frame_cnt = 0;
#endif
#ifdef TS_TIMESTAMP_CORRECTION
        omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iLatestPTS = 0;
        omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iRamainingDuration = 0;
		omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iRealPTS = 0;
		omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iInterpolationCount = 0;
#endif		
		break;

	case CVDEC_DISP_INFO_UPDATE:	//update
		{
			int       iDecodedIdx;
			int       usedIdx, startIdx, regIdx;
			dec_disp_info_t *pdec_disp_info;

			iDecodedIdx = pInfoInput->m_iFrameIdx;

			//save the side info.
			usedIdx = iDecodedIdx;
			if(pInfoCtrl->m_iRegIdxPTS[usedIdx] != -1)
			{
				ALOGE("Invalid Display Index [%d][%d] !!!", uiDecoderID, usedIdx);
			}
			
			pInfoCtrl->m_iRegIdxPTS[usedIdx] = iDecodedIdx;
			pdec_disp_info = (dec_disp_info_t *) pInfoCtrl->m_pRegInfoPTS[usedIdx];

			pdec_disp_info->m_iTimeStamp = pInfo->m_iTimeStamp;
			pdec_disp_info->m_iFrameType = pInfo->m_iFrameType;
			pdec_disp_info->m_iPicStructure = pInfo->m_iPicStructure;
			pdec_disp_info->m_iextTimeStamp = pInfo->m_iextTimeStamp;
			pdec_disp_info->m_iM2vFieldSequence = pInfo->m_iM2vFieldSequence;
			pdec_disp_info->m_iFrameDurationFactor = pInfo->m_iFrameDurationFactor;
			pdec_disp_info->m_iFrameSize = pInfo->m_iFrameSize;
			pdec_disp_info->m_iTopFieldFirst = pInfo->m_iTopFieldFirst;
			//ALOGE("UPDATE : %d", pdec_disp_info->m_iTopFieldFirst);
			pdec_disp_info->m_iIsProgressive = pInfo->m_iIsProgressive;
			pdec_disp_info->m_iWidth = pInfo->m_iWidth;
    		pdec_disp_info->m_iHeight = pInfo->m_iHeight;
    		pdec_disp_info->m_iFrameRate = pInfo->m_iFrameRate;

			/* update frame-rate */
			if( pdec_disp_info->m_iFrameRate && pInfoCtrl->m_iFrameRate != pdec_disp_info->m_iFrameRate ) {
				pInfoCtrl->m_iFrameRate = pdec_disp_info->m_iFrameRate;
				pInfoCtrl->m_iFrameDuration = 1000000000 / pInfoCtrl->m_iFrameRate;
			
				ALOGI("FRAME-RATE UPDATED - [FRAME-RATE: %d] [FRAME-DUE: %d us]"
						  , pInfoCtrl->m_iFrameRate
						  , pInfoCtrl->m_iFrameDuration
						  );
			}

			if (pInfoCtrl->m_iTimeStampType == CDMX_DTS_MODE)	//Decode Timestamp (Decode order)
			{
				if (iDecodedIdx >= 0 || (iDecodedIdx == -2 && pInfoCtrl->m_iStdType == STD_MPEG4))
				{
					pInfoCtrl->m_iDTS[pInfoCtrl->m_iDecodeIdxDTS] = pInfo->m_iTimeStamp;
					pInfoCtrl->m_iDecodeIdxDTS = (pInfoCtrl->m_iDecodeIdxDTS + 1) & 31;
				}
			}
		}
		break;
	case CVDEC_DISP_INFO_GET:	//display
		{
			dec_disp_info_t **pInfo = (dec_disp_info_t **) pParam2;
			int       dispOutIdx = pInfoInput->m_iFrameIdx;
			OMX_TICKS	  out_timestamp;

			//Presentation Timestamp (Display order)
			{
				*pInfo = 0;

				for (i = 0; i < 32; i++)
				{
					if (dispOutIdx == pInfoCtrl->m_iRegIdxPTS[i])
					{
						*pInfo = (dec_disp_info_t *) pInfoCtrl->m_pRegInfoPTS[i];
#if 1
	#ifdef TS_TIMESTAMP_CORRECTION
						int lastPTS = omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iLatestPTS;


						if( (lastPTS == (*pInfo)->m_iTimeStamp)
						 || (lastPTS > (*pInfo)->m_iTimeStamp && (lastPTS - (*pInfo)->m_iTimeStamp) < 10000)
						 || (lastPTS < (*pInfo)->m_iTimeStamp && ((*pInfo)->m_iTimeStamp - lastPTS) <= (pInfoCtrl->m_iFrameDuration / 2000))
						)
						{
							out_timestamp = lastPTS + (pInfoCtrl->m_iFrameDuration / 1000);
							//ALOGE("=====> INFO_GET 1.[iTimeStamp:%d][lastPTS:%d][correction:%d][out_timestamp:%lld]\n", (*pInfo)->m_iTimeStamp, lastPTS, (pInfoCtrl->m_iFrameDuration / 1000), out_timestamp);
						}
						else
						{
							out_timestamp = (*pInfo)->m_iTimeStamp;
							//ALOGE("=====> INFO_GET 2.[iTimeStamp:%d][lastPTS:%d][correction:%d][out_timestamp:%lld]\n", (*pInfo)->m_iTimeStamp, lastPTS, 0, out_timestamp);
						}

						omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iLatestPTS = out_timestamp;
	#else
						if( pInfoCtrl->m_iLatestPTS == (*pInfo)->m_iTimeStamp &&
							pInfoCtrl->m_iLatestPTS >= (*pInfo)->m_iTimeStamp )
						{
							if( pInfoCtrl->m_iFrameDurationFactor )
								out_timestamp = pInfoCtrl->m_iLatestPTS + ((pInfoCtrl->m_iFrameDuration * (*pInfo)->m_iFrameDurationFactor) >> 1);
							else
								out_timestamp = pInfoCtrl->m_iLatestPTS + pInfoCtrl->m_iFrameDuration;
						}
						else
						{
							out_timestamp = (*pInfo)->m_iTimeStamp;
						}
						pInfoCtrl->m_iLatestPTS = out_timestamp;
	#endif
						(*pInfo)->m_iTimeStamp = out_timestamp;
#else
	#ifdef TS_TIMESTAMP_CORRECTION
						if( 
							((*pInfo)->m_iTimeStamp == 0) 
							|| (
									((*pInfo)->m_iTimeStamp <= omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iLatestPTS)
									&&  (((*pInfo)->m_iTimeStamp + 1000 ) > omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iLatestPTS)
								)
						)
						{
							omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iInterpolationCount++;
							(*pInfo)->m_iTimeStamp = omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iRealPTS
					                     + omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iInterpolationCount * omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iPTSInterval/1000;
							//ALOGE("PTS Inter. %d intv %d cnt %d\n", (*pInfo)->m_iTimeStamp, omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iPTSInterval, omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iInterpolationCount );
						}
						else
						{
							omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iInterpolationCount = 0;
							omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iRealPTS = (*pInfo)->m_iTimeStamp;
							//ALOGE("PTS First. %d \n", (*pInfo)->m_iTimeStamp );
						}

						omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iLatestPTS = (*pInfo)->m_iTimeStamp;
						//omx_private->gsTSPtsInfo[uiDecoderID].m_iRamainingDuration = (*pInfo)->m_iFrameDuration;
	#endif
#endif
						//ALOGE("=====> %d", (*pInfo)->m_iTimeStamp);
						pInfoCtrl->m_iRegIdxPTS[i] = -1;	//unused
						break;
					}
				}
				
				//ALOGE("GET : %d", (*pInfo)->m_iTopFieldFirst);
			}

			if (pInfoCtrl->m_iTimeStampType == CDMX_DTS_MODE)	//Decode Timestamp (Decode order)
			{
				if (*pInfo != 0)
				{
					(*pInfo)->m_iTimeStamp = (*pInfo)->m_iextTimeStamp = pInfoCtrl->m_iDTS[pInfoCtrl->m_iDispIdxDTS];
					pInfoCtrl->m_iDispIdxDTS = (pInfoCtrl->m_iDispIdxDTS + 1) & 31;
				}
			}
		}
		break;
	}

	return;
}

#ifdef VIDEO_USER_DATA_PROCESS
static void GetUserDataListFromCDK( unsigned char *pUserData, video_userdata_list_t *pUserDataList )
{
	unsigned int i;
	unsigned char * pTmpPTR;
	unsigned char * pRealData;
	unsigned int nNumUserData;
	unsigned int nTotalSize;
	unsigned int nDataSize;

	memset( pUserDataList, 0, sizeof(video_userdata_list_t) );
	
	pTmpPTR = pUserData;
	nNumUserData = (pTmpPTR[0] << 8) | pTmpPTR[1];
	nTotalSize = (pTmpPTR[2] << 8) | pTmpPTR[3];

	pTmpPTR = pUserData + 8;
	pRealData = pUserData + (8 * 17);

	if( nNumUserData > MAX_USERDATA )
	{
		DBUG_MSG("[vdec_userdata] user data num is over max. %d\n", nNumUserData );
		nNumUserData = MAX_USERDATA;
	}
	
	for(i = 0;i < nNumUserData;i++)
	{
		nDataSize = (pTmpPTR[2] << 8) | pTmpPTR[3];

		/* */
		pUserDataList->arUserData[i].pData = pRealData;
		pUserDataList->arUserData[i].iDataSize = nDataSize;
		
		pTmpPTR += 8;
		pRealData += nDataSize;
	}

	/* */
	pUserDataList->iUserDataNum = nNumUserData;
}	

void process_user_data( OMX_IN  OMX_COMPONENTTYPE *h_component, video_userdata_list_t *pUserDataList )
{
	omx_videodec_component_PrivateType *p_priv = h_component->pComponentPrivate;

	unsigned long long ullPTS = (pUserDataList->ullPTS)*9/100;
	unsigned char fDiscontinuity = pUserDataList->fDiscontinuity;

	int i;

	for( i=0; i<pUserDataList->iUserDataNum; i++ )
	{
		video_userdata_t *pUserData = &pUserDataList->arUserData[i];

		OMX_U32 args[5];

		args[0] = pUserData->pData;
		args[1] = pUserData->iDataSize;
		args[2] = (OMX_U32)(pUserDataList->ullPTS>>32);
		args[3] = (OMX_U32)(pUserDataList->ullPTS&0xFFFFFFFF);
		args[4] = pUserDataList->fDiscontinuity;

		(*(p_priv->callbacks->EventHandler))(
			h_component, 
			p_priv->callbackData, 
			OMX_EventVendorVideoUserDataAvailable, 
			2, 
			3, 
			&args);

	}
}

void print_user_data (unsigned char *pUserData)
{
	int       i, j;
	unsigned char *pTmpPTR;
	unsigned char *pRealData;
	unsigned int nNumUserData;
	unsigned int nTotalSize;
	unsigned int nDataSize;

	pTmpPTR = pUserData;
	nNumUserData = (pTmpPTR[0] << 8) | pTmpPTR[1];
	nTotalSize = (pTmpPTR[2] << 8) | pTmpPTR[3];

	pTmpPTR = pUserData + 8;
	pRealData = pUserData + (8 * 17);

	DBUG_MSG ("\n***User Data Print***\n");
	for (i = 0; i < nNumUserData; i++)
	{
		nDataSize = (pTmpPTR[2] << 8) | pTmpPTR[3];
		DBUG_MSG ("[User Data][Idx : %02d][Size : %05d]", i, nDataSize);
		for (j = 0; j < nDataSize; j++)
		{
			DBUG_MSG ("%02x ", pRealData[j]);
		}
		pTmpPTR += 8;
		pRealData += nDataSize;
	}
}
#endif //VIDEO_USER_DATA_PROCESS

char * ring_print_pic_type (int iVideoType, int iPicType, int iPictureStructure)
{
	switch (iVideoType)
	{
	case STD_MPEG2:
		if (iPicType == PIC_TYPE_I)
			return "I :";
		else if (iPicType == PIC_TYPE_P)
			return "P :";
		else if (iPicType == PIC_TYPE_B)
			return "B :";
		else
			return "D :";	//D_TYPE
		break;

	case STD_MPEG4:
		if (iPicType == PIC_TYPE_I)
			return "I :";
		else if (iPicType == PIC_TYPE_P)
			return "P :";
		else if (iPicType == PIC_TYPE_B)
			return "B :";
		else if (iPicType == PIC_TYPE_B_PB)	//MPEG-4 Packed PB-frame
			return "pB:";
		else
			return "S :";	//S_TYPE
		break;	 
	default:
		if (iPicType == PIC_TYPE_I)
			return "I :";
		else if (iPicType == PIC_TYPE_P)
			return "P :";
		else if (iPicType == PIC_TYPE_B)
			return "B :";
		else
			return "U :";	//Unknown
	}
}

#if 1
#include <mach/tcc_scaler_ioctrl.h>
#include <mach/tcc_cam_ioctrl.h>
#include <sys/poll.h>
#include <libpmap/pmap.h>

#if defined(_TCC8920_)
#define SCALER_DEVICE  "/dev/scaler3"
#else
#define SCALER_DEVICE  "/dev/scaler1"
#endif

static int giPhyAddr[3];

#ifdef HAVE_ANDROID_OS
static pmap_t pmap_video;
#define DEST_PHY_BASE_ADDR	pmap_video.base
#define DEST_PHY_BASE_SIZE		pmap_video.size
#endif

#define FRAME_DROP_CNT			5

#include <sys/time.h>
static unsigned char file_name[256] = "/mnt/sdcard/tflash/input_";
static FILE *fp= 0;

int field_only_process(omx_videodec_component_PrivateType* omx_private, OMX_U8 uiDecoderID)
{
	SCALER_TYPE ScaleInfo;
	int m2m_fd = -1, m_copy = -1;
	int sWidth, sHeight;
	int dWidth, dHeight;
	struct pollfd poll_event[1];
	unsigned int I_PA_Y, I_PA_U, I_PA_V;
	unsigned int VA_Y, VA_U, VA_V;
	unsigned char *input_src_ptr;
	unsigned char *dest_ptr;

	input_src_ptr= (unsigned char *)omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pDispOut[PA];
//	ALOGE("%s %d \n", __func__, __LINE__);

	m2m_fd = open(SCALER_DEVICE, O_RDWR);
	if (m2m_fd <= 0) {
		ALOGE("can't open scaler device '%s'", SCALER_DEVICE);
		return -1;
	}
	pmap_get_info("ext_camera", &pmap_video);

//	ALOGE("%s %d input_src_ptr =%x \n", __func__, __LINE__, input_src_ptr);
	I_PA_Y = (*(input_src_ptr+3)<<24) | (*(input_src_ptr+2)<<16) | (*(input_src_ptr+1)<<8) | *(input_src_ptr) ;input_src_ptr+=4;
	I_PA_U = (*(input_src_ptr+3)<<24) | (*(input_src_ptr+2)<<16) | (*(input_src_ptr+1)<<8) | *(input_src_ptr) ; input_src_ptr+=4;
	I_PA_V = (*(input_src_ptr+3)<<24) | (*(input_src_ptr+2)<<16) | (*(input_src_ptr+1)<<8) | *(input_src_ptr) ; input_src_ptr+=4;
//	ALOGE("%s %d PA_Y = %x , PA_U = %x, PA_U = %x \n", __func__, __LINE__, PA_Y, PA_U, PA_V);

	giPhyAddr[0] = I_PA_Y;
	giPhyAddr[1] = I_PA_U;
	giPhyAddr[2] = I_PA_V;	

//	ALOGE("%s %d giPhyAddr[0]=%x , giPhyAddr[1]=%x, giPhyAddr[2]=%x \n", __func__, __LINE__, giPhyAddr[0], giPhyAddr[1], giPhyAddr[2]);

	sWidth = dWidth = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iPicWidth;
	sHeight= dHeight = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iPicHeight;
 
//	ALOGE("%s %d DEST_PHY_BASE_SIZE= %d \n", __func__, __LINE__, DEST_PHY_BASE_SIZE);
//	ALOGE("%s %d width=%d , height = %d \n", __func__, __LINE__, sWidth, sHeight);

	memset(&ScaleInfo, 0x00, sizeof(ScaleInfo));
	ScaleInfo.src_Yaddr 		= (char*)giPhyAddr[0];
	ScaleInfo.src_Uaddr 		= (char*)giPhyAddr[1];
	ScaleInfo.src_Vaddr 		= (char*)giPhyAddr[2];
	ScaleInfo.responsetype		= SCALER_INTERRUPT; //SCALER_NOWAIT;
	ScaleInfo.src_fmt			= SCALER_YUV420_inter;	
	ScaleInfo.src_ImgWidth		= sWidth;
	ScaleInfo.src_ImgHeight 	= sHeight;
	ScaleInfo.src_winLeft		= 0;
	ScaleInfo.src_winTop		= 0;
	ScaleInfo.src_winRight		= sWidth;
	ScaleInfo.src_winBottom 	= sHeight;
 	ScaleInfo.dest_Yaddr		= (char *)DEST_PHY_BASE_ADDR;
	ScaleInfo.dest_Uaddr		= (char *)(DEST_PHY_BASE_ADDR + (sWidth*dWidth)*2)  ;
	ScaleInfo.dest_Vaddr		= (char *)(DEST_PHY_BASE_ADDR + (sWidth*dWidth)*2 + (sWidth*dWidth));
	ScaleInfo.dest_fmt			= SCALER_YUV420_inter; 	//SCALER_YUV420_sp;
	ScaleInfo.dest_ImgWidth 	= sWidth;
	ScaleInfo.dest_ImgHeight	= sHeight;
	ScaleInfo.dest_winLeft		= 0;
	ScaleInfo.dest_winTop		= 0;
	ScaleInfo.dest_winRight 	= sWidth;
	ScaleInfo.dest_winBottom	= sHeight;
	ScaleInfo.interlaced		= 1;

#if 0		
	ALOGD("CAP Scaler-SRC Addr:: 0x%x, 0x%x, 0x%x", giPhyAddr[0], giPhyAddr[1], giPhyAddr[2]);
	ALOGD("CAP Scaler-SRC:: %d x %d, %d,%d ~ %d x %d", ScaleInfo.src_ImgWidth, ScaleInfo.src_ImgHeight, 
			ScaleInfo.src_winLeft, ScaleInfo.src_winTop, dWidth, dHeight);
	ALOGE("src_ImgWidth = %d, src_ImgHeight=%d, src_winLeft = %d, src_winTop = %d ~ dest_ImgWidth = %d , dest_ImgHeight=%d", ScaleInfo.src_ImgWidth, ScaleInfo.src_ImgHeight, 
			ScaleInfo.src_winLeft, ScaleInfo.src_winTop, dWidth, dHeight);
#endif
	if (ioctl(m2m_fd, TCC_SCALER_IOCTRL, &ScaleInfo) != 0)
	{
		ALOGE("%s %d Scaler Out Error!", __func__, __LINE__);
		close(m2m_fd);
		return -1;
	}

	if(ScaleInfo.responsetype == SCALER_INTERRUPT)
	{
		int ret;
		memset(poll_event, 0, sizeof(poll_event));
		poll_event[0].fd = m2m_fd;
		poll_event[0].events = POLLIN;
		ret = poll((struct pollfd*)poll_event, 1, 400);

		if (ret < 0) {
			ALOGE("%s %d m2m poll error\n", __func__, __LINE__);
			close(m2m_fd);
			return -1;
		}else if (ret == 0) {
			ALOGE("%s %d m2m poll timeout\n", __func__, __LINE__); 
			close(m2m_fd);
			return -1;
		}else if (ret > 0) {
			if (poll_event[0].revents & POLLERR) {
				ALOGE("%s %d m2m poll POLLERR\n", __func__, __LINE__);
				close(m2m_fd);
				return -1;
			}
		}
	}
	giPhyAddr[0] = DEST_PHY_BASE_ADDR;
	giPhyAddr[1] = (DEST_PHY_BASE_ADDR + (sWidth*dWidth)*2);
	giPhyAddr[2] = (DEST_PHY_BASE_ADDR + (sWidth*dWidth)*2 + (sWidth*dWidth));
	ALOGE("%s %d giPhyAddr[0]=%x , giPhyAddr[1]=%x, giPhyAddr[2]=%x \n", __func__, __LINE__, giPhyAddr[0], giPhyAddr[1], giPhyAddr[2]);

	memset(&ScaleInfo, 0x00, sizeof(ScaleInfo));
	ScaleInfo.src_Yaddr 		= (char*)giPhyAddr[0];
	ScaleInfo.src_Uaddr 		= (char*)giPhyAddr[1];
	ScaleInfo.src_Vaddr 		= (char*)giPhyAddr[2];
	ScaleInfo.responsetype		= SCALER_INTERRUPT;  //SCALER_NOWAIT;
	ScaleInfo.src_fmt			= SCALER_YUV420_inter;	
	ScaleInfo.src_ImgWidth		= sWidth;
	ScaleInfo.src_ImgHeight 	= sHeight;
	ScaleInfo.src_winLeft		= 0;
	ScaleInfo.src_winTop		= 0;
	ScaleInfo.src_winRight		= sWidth;
	ScaleInfo.src_winBottom 	= sHeight;
	ScaleInfo.dest_Yaddr		= (char*)I_PA_Y; //(char *)omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pDispOut[PA];
	ScaleInfo.dest_Uaddr		= (char*)I_PA_U;
	ScaleInfo.dest_Vaddr		= (char*)I_PA_V;
	ScaleInfo.dest_fmt			= SCALER_YUV420_inter; 	
	ScaleInfo.dest_ImgWidth 	= sWidth;
	ScaleInfo.dest_ImgHeight	= sHeight;
	ScaleInfo.dest_winLeft		= 0;
	ScaleInfo.dest_winTop		= 0;
	ScaleInfo.dest_winRight 	= sWidth;
	ScaleInfo.dest_winBottom	= sHeight;
	ScaleInfo.interlaced= FALSE;

#if 0		
	ALOGD("Dest CAP Scaler-SRC Addr:: 0x%x, 0x%x, 0x%x", giPhyAddr[0], giPhyAddr[1], giPhyAddr[2]);
	ALOGD("Dest CAP Scaler-SRC:: %d x %d, %d,%d ~ %d x %d", ScaleInfo.src_ImgWidth, ScaleInfo.src_ImgHeight, 
			ScaleInfo.src_winLeft, ScaleInfo.src_winTop, dWidth, dHeight);
#endif
	if (ioctl(m2m_fd, TCC_SCALER_IOCTRL, &ScaleInfo) != 0)
	{
		ALOGE("%s %d Scaler Out Error!",__func__, __LINE__);
		close(m2m_fd);
		return -1;
	}

	if(ScaleInfo.responsetype == SCALER_INTERRUPT)
	{
		int ret;
		memset(poll_event, 0, sizeof(poll_event));
		poll_event[0].fd = m2m_fd;
		poll_event[0].events = POLLIN;
		ret = poll((struct pollfd*)poll_event, 1, 400);

		if (ret < 0) {
			ALOGE("%s %d m2m poll error\n", __func__, __LINE__);
			close(m2m_fd);
			return -1;
		}else if (ret == 0) {
			ALOGE("%s %d m2m poll timeout\n", __func__, __LINE__); 
			close(m2m_fd);
			return -1;
		}else if (ret > 0) {
			if (poll_event[0].revents & POLLERR) {
				ALOGE("%s %d m2m poll POLLERR\n", __func__, __LINE__);
				close(m2m_fd);
				return -1;
			}
		}
	}

	close(m2m_fd);

	return 0;
}


int ring_field_decoding_process(omx_videodec_component_PrivateType* omx_private, unsigned int decoderID, unsigned int fieldInfo)
{
	if(omx_private->stIPTVActiveModeInfo.IPTV_PLAYMode)
	{
		if(omx_private->stIPTVActiveModeInfo.IPTV_Activemode == 2) 
		{
			if(fieldInfo == 1 || fieldInfo == 2)
			{
				field_only_process(omx_private, decoderID);
			}
			isRefFrameBroken = OMX_FALSE;
		}
		else if(fieldInfo == 1 || fieldInfo == 2)
		{
			omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iOutputStatus == VPU_DEC_OUTPUT_FAIL;
			return -1;
		}
	}
	else
	{
		if(fieldInfo == 1 || fieldInfo == 2)
		{
			field_only_process(omx_private, decoderID);
		}
	}

	return 0;
}

int ring_for_iptv_trickModeEnd_process(omx_videodec_component_PrivateType* omx_private, unsigned int decoderID)
{
	int	frame_drop_flag = 0;

	if(omx_private->stIPTVActiveModeInfo.IPTV_Old_Activemode ==2 && omx_private->stIPTVActiveModeInfo.IPTV_Activemode ==1 
		&& omx_private->stIPTVActiveModeInfo.IPTV_Mode_change_cnt<FRAME_DROP_CNT)  //for trick mode end
	{
#if 0
		if(omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iOutputStatus == VPU_DEC_OUTPUT_SUCCESS)		
		{
			frame_drop_flag =0;
			omx_private->stIPTVActiveModeInfo.IPTV_Mode_change_cnt++;
			ALOGE("%s %d trick->normal play frame drop cnt= %d \n", __func__, __LINE__, omx_private->stIPTVActiveModeInfo.IPTV_Mode_change_cnt);
		}
#else
		if((omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iPicType &0x00000003) == PIC_TYPE_B 
			&& (omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_SUCCESS
			||omx_private->pVideoDecodInstance[decoderID].iDecod_status == -RETCODE_INSUFFICIENT_BITSTREAM))
		{
			omx_private->stbframe_skipinfo.bframe_skipindex[omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx] 
				= omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx;
			omx_private->stbframe_skipinfo.bframe_skipcnt++;
//			ALOGE("%s %d omx_private->bframe_skipinfo.bframe_skipcnt = %d \n", __func__, __LINE__, omx_private->stbframe_skipinfo.bframe_skipcnt);
//			ALOGE("%s %d m_iDecodedIdx = %d \n", __func__, __LINE__, omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx);
		}
		if(omx_private->stbframe_skipinfo.bframe_skipcnt>0 
			&& omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iOutputStatus == VPU_DEC_OUTPUT_SUCCESS)		
		{
			int i;
			for (i=0; i<32; i++)
			{
				if(omx_private->stbframe_skipinfo.bframe_skipindex[i] == omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iDispOutIdx)
				{
					frame_drop_flag = -1;
					omx_private->stbframe_skipinfo.bframe_skipindex[i] = 0x100;
					omx_private->stbframe_skipinfo.bframe_skipcnt--;
					omx_private->stIPTVActiveModeInfo.IPTV_Mode_change_cnt++;
//					ALOGE("%s %d frame drop m_iDispOutIdx = %d  \n", __func__, __LINE__, omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iDispOutIdx);
//					ALOGE("%s %d omx_private->bframe_skipinfo.bframe_skipcnt= %d \n", __func__, __LINE__, omx_private->stbframe_skipinfo.bframe_skipcnt);
					break;
				}
			}
		}
#endif
	}
	return frame_drop_flag;	
}


int ring_for_iptv_brokenframe_check(omx_videodec_component_PrivateType* omx_private, unsigned int decoderID)
{
	int ret=0;

//	ALOGE("%s %d m_iNumOfErrMBs = %x \n", __func__, __LINE__, omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iNumOfErrMBs);
	
	if(omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iNumOfErrMBs >0 || isRefFrameBroken == OMX_TRUE)
	{
		omx_private->stbroken_frame_info.broken_frame_index[omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx] 
			= omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx;
		omx_private->stbroken_frame_info.broken_frame_iPicType[omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx] 
			= (unsigned char)(omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iPicType);
		omx_private->stbroken_frame_info.broken_frame_iNumOfErrMBs[omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx] 
			= omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iNumOfErrMBs;
		omx_private->stbroken_frame_info.broken_frame_cnt++;

//		ALOGE("%s %d omx_private->stbroken_frame_info.broken_frame_cnt = %d \n", __func__, __LINE__, omx_private->stbroken_frame_info.broken_frame_cnt);
//		ALOGE("%s %d m_iDecodedIdx = %d, m_iNumOfErrMBs = %d \n", __func__, __LINE__, omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx, omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iNumOfErrMBs);
//		ALOGE("%s %d broken_iDecodedIdx = %x , broken_iNumOfErrMBs=%d \n", __func__, __LINE__, omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx, omx_private->broken_frame_info.broken_frame_iNumOfErrMBs[omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx] );
	}

//	ALOGE("%s %d m_iDispOutIdx = %d \n", __func__, __LINE__, omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDispOutIdx);
//	ALOGE("%s %d isRefFrameBroken = %d \n", __func__, __LINE__, isRefFrameBroken);
	if(omx_private->stbroken_frame_info.broken_frame_cnt >0 && isRefFrameBroken != OMX_TRUE)
	{
		int i=0;
		for (i=0; i<32; i++)
		{
			if(omx_private->stbroken_frame_info.broken_frame_index[i] == omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iDispOutIdx
			&& omx_private->stbroken_frame_info.broken_frame_iNumOfErrMBs[i] != 0)
			{
				if(omx_private->stbroken_frame_info.broken_frame_iPicType[i] != PIC_TYPE_B)
				{
					isRefFrameBroken = OMX_TRUE;
				}	
				break;
			}		
		}
	}
	else if(isRefFrameBroken)
	{
		int i=0;
		for (i=0; i<32; i++)
		{
			if(omx_private->stbroken_frame_info.broken_frame_index[i] == omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iDispOutIdx)
			{
				if(omx_private->stbroken_frame_info.broken_frame_iPicType[i] == PIC_TYPE_I )
				{
					isRefFrameBroken = OMX_FALSE;
				}
			//	ALOGE("%s %d isRefFrameBroken = %d \n", __func__, __LINE__, isRefFrameBroken);
			//	ALOGE("%s %d m_iPicType = %x, broken_frame_cnt = %d \n", __func__, __LINE__, iPicType, omx_private->stbroken_frame_info.broken_frame_cnt);
				break;
			}		
		}
	}

	return ret;
}

int ring_for_iptv_brokenframe_process(omx_videodec_component_PrivateType* omx_private, unsigned int decoderID, unsigned int xFramerate)
{
	int	ret =0;
	
	if(isRefFrameBroken)
	{
		if(omx_private->stbroken_frame_info.broken_frame_cnt>xFramerate)
			isRefFrameBroken =0;
		ret =-1;
	}

//	ALOGE("%s %d isRefFrameBroken = %d \n", __func__, __LINE__, isRefFrameBroken);
//	ALOGE("%s %d broken_frame_cnt = %d \n", __func__, __LINE__, omx_private->stbroken_frame_info.broken_frame_cnt);

	if(omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iOutputStatus == VPU_DEC_OUTPUT_SUCCESS
		&& omx_private->stbroken_frame_info.broken_frame_cnt >0)
	{
		int i;
		for (i=0; i<32; i++)
		{
			if(omx_private->stbroken_frame_info.broken_frame_index[i] == omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iDispOutIdx)
			{
				omx_private->stbroken_frame_info.broken_frame_index[i] = 0x100;
				omx_private->stbroken_frame_info.broken_frame_iNumOfErrMBs[i] = 0;
				omx_private->stbroken_frame_info.broken_frame_cnt--;
				ret  = -1;
	//			ALOGE("%s %d frame drop m_iDispOutIdx = %d  \n", __func__, __LINE__, omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDispOutIdx);
	//			ALOGE("%s %d omx_private->broken_frame_info.broken_frame_cnt = %d \n", __func__, __LINE__, omx_private->stbroken_frame_info.broken_frame_cnt);
	//			ALOGE("%s %d i = %d, broken_iDecodedIdx = %x , broken_iNumOfErrMBs=%d \n", __func__, __LINE__, i, omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx, omx_private->stbroken_frame_info.broken_frame_iNumOfErrMBs[i] );
	//			ALOGE("%s %d frame drop m_iDispOutIdx = %d  \n", __func__, __LINE__, omx_private->broken_frame_info.broken_frame_iPicType[omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx]);
				break;
			}		
		}
	}

	return ret;
}

int ring_for_iptv_ChannelChange_process(OMX_COMPONENTTYPE * openmaxStandComp, unsigned int decoderID, unsigned int iPicType)
{
	omx_videodec_component_PrivateType *omx_private = openmaxStandComp->pComponentPrivate;
	int ret=0, Display_fieldInfo;

	Display_fieldInfo = ((omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iPicType& (1 << 15)) >> 15);
#if 0
	ALOGE("%s %d m_iPicType =%d \n", __func__, __LINE__, omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iPicType);
	ALOGE("%s %d Display_fieldInfo =%d \n", __func__, __LINE__, Display_fieldInfo);
	ALOGE("%s %d iPicType =%d \n", __func__, __LINE__, iPicType);
	ALOGE("%s %d FirstFrame_DispStatus =%d \n", __func__, __LINE__, omx_private->stfristframe_dsipInfo.FirstFrame_DispStatus);
	ALOGE("%s %d IPTV_Old_Activemode =%d, IPTV_Activemode = %d  \n", __func__, __LINE__, omx_private->stIPTVActiveModeInfo.IPTV_Old_Activemode, omx_private->stIPTVActiveModeInfo.IPTV_Activemode);
#endif

	if(omx_private->stIPTVActiveModeInfo.IPTV_Old_Activemode == 0 && omx_private->stIPTVActiveModeInfo.IPTV_Activemode==1)		//for IPTV channel change
	{
#if 0
		if(omx_private->stbroken_frame_info.broken_frame_cnt >0 && isRefFrameBroken != OMX_TRUE)
		{
			for (i=0; i<32; i++)
			{
				if(omx_private->stbroken_frame_info.broken_frame_index[i] == omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDispOutIdx)
				{
					if((((omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iPicType &VIDEO_PICTYPE_MASK)<<4)>>4)!= PIC_TYPE_B)
						isRefFrameBroken = OMX_TRUE;

					break;
				}		
			}
			ALOGE("%s %d isRefFrameBroken = %d \n", __func__, __LINE__, isRefFrameBroken);
		}
#endif
		if(omx_private->stfristframe_dsipInfo.FirstFrame_DispStatus==0)
		{
			if(iPicType!= PIC_TYPE_B &&
				(omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_SUCCESS
				|| omx_private->pVideoDecodInstance[decoderID].iDecod_status == -RETCODE_INSUFFICIENT_BITSTREAM))		
			{
				if(iPicType == PIC_TYPE_I)
				{
					if(Display_fieldInfo == 1) // top-field decoding
						omx_private->stfristframe_dsipInfo.TopFrame_DecStatus = 1;
					else
						omx_private->stfristframe_dsipInfo.BottomFrame_DecStatus = 1;
				}
				else if(iPicType == PIC_TYPE_P)
				{
					if(Display_fieldInfo == 1) // top-field decoding
						omx_private->stfristframe_dsipInfo.TopFrame_DecStatus = 1;
					else
						omx_private->stfristframe_dsipInfo.BottomFrame_DecStatus = 1;
				}

				if(omx_private->stfristframe_dsipInfo.TopFrame_DecStatus == 1 && omx_private->stfristframe_dsipInfo.BottomFrame_DecStatus == 1)
				{
					omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iOutputStatus = VPU_DEC_OUTPUT_SUCCESS;
					omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iDispOutIdx = 0;
					omx_private->stfristframe_dsipInfo.FirstFrame_DispStatus = 1;
					ret =1;
					ALOGE("%s %d first frame display \n", __func__, __LINE__);
					(*(omx_private->callbacks->EventHandler)) (openmaxStandComp, omx_private->callbackData,OMX_EventVendorVideoFirstDisplay, 0, 0, NULL);
				}
			}
			else if(omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_SUCCESS 
				&& iPicType == PIC_TYPE_B)
			{
				omx_private->stfristframe_dsipInfo.FirstFrame_DispStatus = 1;
				omx_private->stfristframe_dsipInfo.TopFrame_DecStatus = 0;
				omx_private->stfristframe_dsipInfo.BottomFrame_DecStatus = 0;
			}
		}
		else if(omx_private->pVideoDecodInstance[decoderID].gsVDecOutput.m_DecOutInfo.m_iOutputStatus == VPU_DEC_OUTPUT_SUCCESS)		
		{
	//		ALOGE("%s %d m_DecOutInfo.m_iDispOutIdx =%d \n", __func__, __LINE__, omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDispOutIdx);
			if(omx_private->stIPTVActiveModeInfo.IPTV_Mode_change_cnt<FRAME_DROP_CNT)
			{
				ret = -1;
				ALOGE("%s %d frame drop \n", __func__, __LINE__);
				omx_private->stIPTVActiveModeInfo.IPTV_Mode_change_cnt++;
			}
			else if(omx_private->stIPTVActiveModeInfo.IPTV_Mode_change_cnt ==FRAME_DROP_CNT)
			{
				omx_private->stIPTVActiveModeInfo.IPTV_Mode_change_cnt++;
			}
			omx_private->stfristframe_dsipInfo.TopFrame_DecStatus = 0;
			omx_private->stfristframe_dsipInfo.BottomFrame_DecStatus = 0;
			omx_private->stfristframe_dsipInfo.FirstFrame_DispStatus=1;
		}
	}
	
	return ret;	
}

#endif

#define CODETYPE_NONE		(0x00000000)
#define CODETYPE_HEADER		(0x00000001)
#define CODETYPE_PICTURE	(0x00000002)
#define CODETYPE_ALL		(CODETYPE_HEADER | CODETYPE_PICTURE)

OMX_U32 ring_SearchCodeType(omx_videodec_component_PrivateType* omx_private, OMX_U32 *input_offset, OMX_U32 search_option, OMX_U8 uiDecoderID)
{
	unsigned int uiCode = 0xFFFFFFFF;
	OMX_U32 temp_input_offset = *input_offset;
	OMX_U32 code_type = CODETYPE_NONE;

	if (OMX_VIDEO_CodingMPEG2 == omx_private->pVideoDecodInstance[uiDecoderID].video_coding_type)
	{
		unsigned int SEQUENCE_HEADER = 0x000001B3;
		unsigned int PICTURE_START = 0x00000100;

		for (; temp_input_offset < omx_private->inputCurrLength; temp_input_offset++)
		{
			uiCode = (uiCode << 8) | omx_private->inputCurrBuffer[temp_input_offset];
			if ((search_option & CODETYPE_HEADER) && (uiCode == SEQUENCE_HEADER))
			{
				code_type = CODETYPE_HEADER;
				temp_input_offset -= 3;
				break;
			}
			if ((search_option & CODETYPE_PICTURE) && (uiCode == PICTURE_START))
			{
				code_type = CODETYPE_PICTURE;
				temp_input_offset -= 3;
				break;
			}
		}
	}
	else if (OMX_VIDEO_CodingAVC == omx_private->pVideoDecodInstance[uiDecoderID].video_coding_type)
	{
		unsigned int MASK = 0xFFFFFF1F;
		unsigned int P_FRAME = 0x00000101;
		unsigned int I_FRAME = 0x00000105;
		unsigned int SPS = 0x00000107;
		unsigned int uiMask;

		for (; temp_input_offset < omx_private->inputCurrLength; temp_input_offset++)
		{
			uiCode = (uiCode << 8) | omx_private->inputCurrBuffer[temp_input_offset];
			uiMask = uiCode & MASK;
			if ((search_option & CODETYPE_HEADER) && (uiMask == SPS))
			{
				code_type = CODETYPE_HEADER;
				temp_input_offset -= 3;
				break;
			}
			if ((search_option & CODETYPE_PICTURE) && ((uiMask == P_FRAME) || (uiMask == I_FRAME)))
			{
				code_type = CODETYPE_PICTURE;
				temp_input_offset -= 3;
				break;
			}
		}
	}
	*input_offset = temp_input_offset;

	return code_type;
}

OMX_U32 ring_CheckVideoStart(OMX_COMPONENTTYPE * openmaxStandComp, OMX_U8 uiDecoderID)
{
	OMX_U8 ucCount, i;
	OMX_U32 ulVideoStart;
	OMX_U32 ulVideoFormat;
	omx_videodec_component_PrivateType *omx_private = openmaxStandComp->pComponentPrivate;
	omx_base_video_PortType *outPort;
    if(omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.nState != TCC_DXBVIDEO_OMX_Dec_None)
    {
        ulVideoStart = omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.nState;
        ulVideoFormat = omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.nFormat;
        omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.nState = TCC_DXBVIDEO_OMX_Dec_None;
        //omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.nFormat = 0;

        pthread_mutex_lock(&omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.mutex);
        if(ulVideoStart == TCC_DXBVIDEO_OMX_Dec_Stop)
        {
            #if (0)//def TCC_VIDEO_DISPLAY_BY_VSYNC_INT
            if(isVsyncEnable()) {
                tcc_vsync_command(omx_private, TCC_LCDC_VIDEO_END_VSYNC, 0);
            }
            #endif
            if (uiDecoderID==0) outPort = (omx_base_video_PortType *) omx_private->ports[OMX_DXB_VIDEOCOMPONENT_OUTPORT];
            else outPort = (omx_base_video_PortType *) omx_private->ports[OMX_DXB_VIDEOCOMPONENT_OUTPORT_SUB];
            outPort->sPortParam.format.video.nFrameWidth = 0;
            outPort->sPortParam.format.video.nFrameHeight = 0;
            if (omx_private->pVideoDecodInstance[uiDecoderID].avcodecReady)
            {
                omx_ring_videodec_component_LibDeinit (omx_private, uiDecoderID);
                omx_private->pVideoDecodInstance[uiDecoderID].avcodecReady = OMX_FALSE;
            }

            omx_private->pVideoDecodInstance[uiDecoderID].bVideoStarted = OMX_FALSE;
            omx_private->pVideoDecodInstance[uiDecoderID].buffer_unique_id = 0;
            omx_private->pVideoDecodInstance[uiDecoderID].ulDecoderUID += iTotalHandle;
			
            DBUG_MSG("%s - TCC_DXBVIDEO_OMX_Dec_Stop\n", __func__);
        }
        else if(ulVideoStart == TCC_DXBVIDEO_OMX_Dec_Start)
        {
            if(uiDecoderID==omx_private->iDisplayID)
                isRenderer = OMX_TRUE;

            if (omx_private->pVideoDecodInstance[uiDecoderID].avcodecReady)
            {             
                if(uiDecoderID==0) outPort = (omx_base_video_PortType *) omx_private->ports[OMX_DXB_VIDEOCOMPONENT_OUTPORT];
                else outPort = (omx_base_video_PortType *) omx_private->ports[OMX_DXB_VIDEOCOMPONENT_OUTPORT_SUB];
                outPort->sPortParam.format.video.nFrameWidth = 0;
                outPort->sPortParam.format.video.nFrameHeight = 0;
                omx_ring_videodec_component_LibDeinit (omx_private, uiDecoderID);
                omx_private->pVideoDecodInstance[uiDecoderID].avcodecReady = OMX_FALSE;
            }

            if (!omx_private->pVideoDecodInstance[uiDecoderID].avcodecReady)
            {
                omx_ring_videodec_component_LibInit (omx_private, uiDecoderID);
                omx_private->pVideoDecodInstance[uiDecoderID].avcodecReady = OMX_TRUE;
            }

            if(ulVideoFormat == TCC_DXBVIDEO_OMX_Codec_H264)
            {
                if(omx_private->pVideoDecodInstance[uiDecoderID].video_coding_type != OMX_VIDEO_CodingAVC)
                    omx_ring_videodec_component_Change(openmaxStandComp, VIDEO_DEC_H264_NAME, uiDecoderID);
            }
            else
            {
                if(omx_private->pVideoDecodInstance[uiDecoderID].video_coding_type != OMX_VIDEO_CodingMPEG2)
                    omx_ring_videodec_component_Change(openmaxStandComp, VIDEO_DEC_MPEG2_NAME, uiDecoderID);
            }
            omx_private->pVideoDecodInstance[uiDecoderID].video_dec_idx = 0;
            omx_private->pVideoDecodInstance[uiDecoderID].bVideoStarted = OMX_TRUE;
            DBUG_MSG("%s - TCC_DXBVIDEO_OMX_Dec_Start, Format=0x%x\n", __func__, ulVideoFormat);
        }
        pthread_mutex_unlock(&omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.mutex);
    }
    return 0;
}

int ring_delete_vpu_displayed_index(omx_videodec_component_PrivateType *omx_private, OMX_U8 uiDecoderID)
{
	int ret = 0, i;
	int nClearDispIndex;

	for(i = 0; i < 32; i++ ) //Max 32 elements
	{
		nClearDispIndex = (int)dequeue(omx_private->pVideoDecodInstance[uiDecoderID].pVPUDisplayedIndexQueue);
		if( nClearDispIndex ==  0 )
			break;
		
		nClearDispIndex--; //CAUTION !!! nClearDispIndex is added with 1 before. We must do -1.
		if ((ret = omx_private->pVideoDecodInstance[uiDecoderID].gspfVDec (VDEC_BUF_FLAG_CLEAR, NULL, &nClearDispIndex, NULL, omx_private->pVideoDecodInstance[uiDecoderID].pVdec_Instance)) < 0)
		{
			ALOGE ("%s VDEC_BUF_FLAG_CLEAR[%d] Err : [%d] index : %d",__func__, uiDecoderID, ret, nClearDispIndex);				
		}		
	}
	
	return ret;
}

static void set_cpu_clock(OMX_COMPONENTTYPE * openmaxStandComp, OMX_U8 uiDecoderID)
{
    omx_videodec_component_PrivateType *omx_private = openmaxStandComp->pComponentPrivate;
	omx_base_video_PortType *outPort = (omx_base_video_PortType *) omx_private->ports[OMX_DXB_VIDEOCOMPONENT_OUTPORT];

	if (omx_private->gHDMIOutput || (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iPicWidth >= 1440 || outPort->sPortParam.format.video.nFrameHeight >= 1080))
    {
        vpu_update_sizeinfo (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat, 40, 30, AVAILABLE_MAX_WIDTH, AVAILABLE_MAX_HEIGHT, omx_private->pVideoDecodInstance[uiDecoderID].pVdec_Instance);	//max-clock!!
		omx_private->pVideoDecodInstance[uiDecoderID].bitrate_mbps = 40; //set max
    }
    else if (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iPicWidth >= 720 || outPort->sPortParam.format.video.nFrameHeight >= 576)
    {
        vpu_update_sizeinfo (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat, 30, 30, AVAILABLE_MAX_WIDTH, AVAILABLE_MAX_HEIGHT, omx_private->pVideoDecodInstance[uiDecoderID].pVdec_Instance);	//max-clock!!
        omx_private->pVideoDecodInstance[uiDecoderID].bitrate_mbps = 30;
    }
    else
    {
        vpu_update_sizeinfo (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat, 15, 30,
                             omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iPicWidth,
                             omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iPicHeight,
                             omx_private->pVideoDecodInstance[uiDecoderID].pVdec_Instance);
        omx_private->pVideoDecodInstance[uiDecoderID].bitrate_mbps = 15;
    }
}

/** This function is used to process the input buffer and provide one output buffer
  */
void omx_ring_videodec_component_BufferMgmtCallback (OMX_COMPONENTTYPE * openmaxStandComp,
												OMX_BUFFERHEADERTYPE * pInputBuffer,
												OMX_BUFFERHEADERTYPE * pOutputBuffer)
{
	omx_videodec_component_PrivateType *omx_private = openmaxStandComp->pComponentPrivate;
	OMX_S32   ret;
	OMX_S32   nDisplayIndex = -1, nOutputFrameWidth, nOutputFrameHeight;
	OMX_U8   *outputCurrBuffer, *pInputBufer;
	int       decode_result;
	int       i, iSeqHeaderOffset;
	unsigned int uiDemuxID = 0, uiDecoderID = 0;
	dec_disp_info_t dec_disp_info_tmp;
	OMX_U8 uiDisplayID = omx_private->iDisplayID;
	int Decoding_fieldInfo = -1;
	unsigned char iPicType;
	OMX_TICKS  curr_timestamp;
	OMX_U32 dec_result = 0, input_flag = pInputBuffer->nFlags;

	omx_base_video_PortType *outPort;

//ALOGE("RING_BUFFER_MODE");

	if (omx_private->state != OMX_StateExecuting)
	{
		ALOGE ("=> omx_private->state != OMX_StateExecuting");
		return;
	}

	if( memcmp(pInputBuffer->pBuffer,BUFFER_INDICATOR,BUFFER_INDICATOR_SIZE)==0)
	{
		memcpy(&pInputBufer, pInputBuffer->pBuffer + BUFFER_INDICATOR_SIZE, 4);
		if(pInputBuffer->pBuffer[BUFFER_INDICATOR_SIZE+4] == 0xAA)
		{
			uiDemuxID = pInputBuffer->pBuffer[BUFFER_INDICATOR_SIZE+5];
			uiDecoderID = pInputBuffer->pBuffer[BUFFER_INDICATOR_SIZE+6];
			//LOGD("%s:Demuxer[%d]Decoder[%d]",__func__, uiDemuxID, uiDecoderID);
		}
	}
	else
	{
		pInputBufer = pInputBuffer->pBuffer;
		uiDecoderID = pInputBuffer->pBuffer[pInputBuffer->nFilledLen];
	}

	if (uiDecoderID==0)
	{
		outPort = (omx_base_video_PortType *)omx_private->ports[OMX_DXB_VIDEOCOMPONENT_OUTPORT];
	}
	else
	{
		outPort = (omx_base_video_PortType *)omx_private->ports[OMX_DXB_VIDEOCOMPONENT_OUTPORT_SUB];
	}

	nOutputFrameWidth = outPort->sPortParam.format.video.nFrameWidth;
	nOutputFrameHeight = outPort->sPortParam.format.video.nFrameHeight;

	pthread_mutex_lock(&omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.mutex);

	if (pInputBuffer->nFlags & OMX_BUFFERFLAG_TRICK_PLAY)
	{
		omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iLatestPTS = 0;
	}

	if(omx_private->pVideoDecodInstance[uiDecoderID].bVideoStarted == OMX_FALSE ||
		(omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.nState == TCC_DXBVIDEO_OMX_Dec_Stop))
	{
		pInputBuffer->nFilledLen = 0;
		pOutputBuffer->nFilledLen = 0;
		pthread_mutex_unlock(&omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.mutex);
		return;
	}

#if 1	// for dual-decoding
	if((omx_private->stPreVideoInfo.iDecod_Support_Country == TCC_DXBVIDEO_ISSUPPORT_BRAZIL  
		&& omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].gsVDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_SUCCESS)
		&& (uiDisplayID == TCC_DXBVIDEO_DISPLAY_SUB && uiDecoderID ==TCC_DXBVIDEO_DISPLAY_MAIN))
	{
		pInputBuffer->nFilledLen = 0;
		pOutputBuffer->nFilledLen = 0;
		pthread_mutex_unlock(&omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.mutex);
		return;
	}
#endif

#if 1 //for dual decoding
	if(omx_private->stPreVideoInfo.iDecod_status == -RETCODE_INSUFFICIENT_BITSTREAM)
	{
		if(omx_private->stPreVideoInfo.iDecod_Instance != uiDecoderID)
		{
			unsigned int cur_time;
			cur_time = ring_dbg_get_time();
			if (cur_time >= omx_private->stPreVideoInfo.uDecod_time)
				cur_time -= omx_private->stPreVideoInfo.uDecod_time;
			else
				cur_time = 0;
			if (cur_time >= VPU_FIELDPICTURE_PENDTIME_MAX) {
				ALOGE("%s %d Multi Instance Error need to pending release \n", __func__, __LINE__);
				omx_private->pVideoDecodInstance[omx_private->stPreVideoInfo.iDecod_Instance].gspfVDec (VDEC_SW_RESET, NULL, NULL, NULL, omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].pVdec_Instance);
			} else {
				long long llPTS = pInputBuffer->nTimeStamp/1000;
				long long llSTC = DxB_SYNC_OK;

				if (gfnDemuxGetSTCValue != NULL) {
					llSTC = gfnDemuxGetSTCValue (DxB_SYNC_VIDEO, llPTS, uiDecoderID, 0);
				}
				if (llSTC == DxB_SYNC_DROP){
					pInputBuffer->nFilledLen = 0;
				}
				
				pOutputBuffer->nFilledLen = 0;
				pthread_mutex_unlock(&omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.mutex);
				return;
			}
		} else {
			omx_private->stPreVideoInfo.uDecod_time = ring_dbg_get_time();
		}
	}
#endif

	/** Fill up the current input buffer when a new buffer has arrived */
	omx_private->inputCurrBuffer = pInputBufer;
	omx_private->inputCurrLength = pInputBuffer->nFilledLen;

	outputCurrBuffer = pOutputBuffer->pBuffer;
	pOutputBuffer->nFilledLen = 0;
	pOutputBuffer->nOffset = 0;
	pOutputBuffer->nDecodeID = uiDecoderID;

	if (omx_private->isFirstBuffer)
	{
		tsem_down (omx_private->avCodecSyncSem);
		omx_private->isFirstBuffer = 0;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	/*ZzaU :: remove NAL-STart Code when there are double codes. ex) AVI container */
    if (!omx_private->pVideoDecodInstance[uiDecoderID].avcodecInited)
    {
#if 0
		if(omx_private->inputCurrLength == 0xFFFFFFFF)
		{
			pInputBuffer->nFilledLen = 0;
			pthread_mutex_unlock(&omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.mutex);
			return;
		}
#endif
		omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iPicWidth			 = outPort->sPortParam.format.video.nFrameWidth;
		omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iPicHeight			 = outPort->sPortParam.format.video.nFrameHeight;
		omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_bEnableVideoCache	 = RING_BUFFER_MODE;
		omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_bEnableUserData		 = FLAG_ENABLE_USER_DATA;
		omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_pExtraData			 = NULL;
		omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iExtraDataLen		 = 0;

		omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_bM4vDeblk			 = 0;
		omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_uiDecOptFlags		 = 0;

		if(omx_private->stVideoSubFunFlag.SupportFieldDecoding)
			omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_uiDecOptFlags	|= (1 << 3);	//filed decoding enable	

		//omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_uiDecOptFlags		|= (1 << 4);	// pending status release in vpu

		omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_uiMaxResolution		 = 0;			//pCdk->m_uiVideoMaxResolution;         
		omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_bFilePlayEnable		 = 0;

		omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_bCbCrInterleaveMode	 = 1;			//YUV420 sp Interleave 
		{
			omx_private->pVideoDecodInstance[uiDecoderID].dec_disp_info_input.m_iStdType		= omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat;
			omx_private->pVideoDecodInstance[uiDecoderID].dec_disp_info_input.m_iTimeStampType	= CDMX_PTS_MODE;	// Presentation Timestamp (Display order)
			omx_private->pVideoDecodInstance[uiDecoderID].dec_disp_info_input.m_iFmtType		= omx_private->pVideoDecodInstance[uiDecoderID].container_type;
			disp_pic_info (CVDEC_DISP_INFO_INIT,
							(void *) &(omx_private->pVideoDecodInstance[uiDecoderID].dec_disp_info_ctrl),
							(void *) omx_private->pVideoDecodInstance[uiDecoderID].dec_disp_info,
							(void *) &(omx_private->pVideoDecodInstance[uiDecoderID].dec_disp_info_input),
							omx_private,
							uiDecoderID);
		}

		iSeqHeaderOffset = 0;
		if( ring_SearchCodeType(omx_private, &iSeqHeaderOffset, CODETYPE_HEADER, uiDecoderID) != CODETYPE_HEADER )
		{
			ALOGE ("[VDEC_INIT:%d]There are no sequence header !!!", uiDecoderID);
			pInputBuffer->nFilledLen = 0;
			pthread_mutex_unlock(&omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.mutex);
			return;
		}

		omx_private->inputCurrBuffer += iSeqHeaderOffset;
		omx_private->inputCurrLength -= iSeqHeaderOffset;
		//pInputBuffer->nFilledLen -= iSeqHeaderOffset;

		if(omx_private->Resolution_Change == 0 && get_total_handle_cnt(openmaxStandComp)<1)
		{
//			if( omx_private->pVideoDecodInstance[uiDecoderID].isVPUClosed == OMX_TRUE )
//				omx_private->pVideoDecodInstance[uiDecoderID].gspfVDec (VDEC_HW_RESET, uiDecoderID, &omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit, &omx_private->pVideoDecodInstance[uiDecoderID].bitrate_mbps, omx_private->pVideoDecodInstance[uiDecoderID].pVdec_Instance);
		}
		else if(omx_private->Resolution_Change == 1)
		{
			omx_private->Resolution_Change = 0;
		}

		/* VPU Initialize */
		if( omx_private->pVideoDecodInstance[uiDecoderID].isVPUClosed == OMX_TRUE ) {

			if ((uiDecoderID == uiDisplayID) && (omx_private->pVideoDecodInstance[uiDecoderID].buffer_unique_id == 0)) {
				tcc_vsync_command(omx_private, TCC_LCDC_HDMI_LASTFRAME | 0x80000000, omx_private->Resolution_Change);
			}

			if ((ret = omx_private->pVideoDecodInstance[uiDecoderID].gspfVDec (VDEC_INIT, uiDecoderID, &omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit, &omx_private->pVideoDecodInstance[uiDecoderID].bitrate_mbps, omx_private->pVideoDecodInstance[uiDecoderID].pVdec_Instance)) < 0)
			{
				ALOGE ("[VDEC_INIT:%d] [Err:%ld] video decoder init", uiDecoderID, ret);
				goto ERR_PROCESS;
			}

			/* VPU ring-buffer initialize */
			if(InitRingBuffer(&omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.stRingBuffInfo,
								omx_private->pVideoDecodInstance[uiDecoderID].pVdec_Instance) == FALSE)
			{
				goto ERR_PROCESS;
			}

			/* input buffer information queue init. */
			if(InitInputQueue(&omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.stInputInfoQueue, INPUT_INFO_QUEUE_INIT_SIZE) == FALSE)
			{
				goto ERR_PROCESS;
			}

			omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.pstVDecInput		= &omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput;
			omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.pstVDecOutput		= &omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput;
			omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.pisVPUClosed		= &omx_private->pVideoDecodInstance[uiDecoderID].isVPUClosed;
			omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.pavcodecInited	= &omx_private->pVideoDecodInstance[uiDecoderID].avcodecInited;

			omx_private->pVideoDecodInstance[uiDecoderID].isVPUClosed = OMX_FALSE;
		}

		#ifdef VIDEO_USER_DATA_PROCESS
		if(omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_bEnableUserData)
		{
			omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_UserDataAddr[PA]    = vpu_getUserDataBufAddr(PA, omx_private->pVideoDecodInstance[uiDecoderID].pVdec_Instance);
			omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_UserDataAddr[VA]    = vpu_getUserDataBufAddr(VA, omx_private->pVideoDecodInstance[uiDecoderID].pVdec_Instance);
			omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iUserDataBufferSize = vpu_getUserDataBufSize(omx_private->pVideoDecodInstance[uiDecoderID].pVdec_Instance);
		}
		#endif

		vpu_set_additional_frame_count (6, omx_private->pVideoDecodInstance[uiDecoderID].pVdec_Instance);

		ret = FeedVpuDecoder(&omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.stRingBuffInfo,
								pInputBuffer,
								omx_private->inputCurrBuffer,
								omx_private->inputCurrLength,
								pInputBuffer->nTimeStamp,
								0 == uiDecoderID);
		switch(ret)
		{
			case FEED_FAILED:
				goto ERR_PROCESS;
				break;

			case NEED_MORE_DATA:
				pthread_mutex_unlock(&omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.mutex);
				return; //FEED_MORE_DATA

			default:
				//FEED_COMPLETE
				break;
		}

		if ((ret = omx_private->pVideoDecodInstance[uiDecoderID].gspfVDec (VDEC_DEC_SEQ_HEADER, NULL, &omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput, &omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput, omx_private->pVideoDecodInstance[uiDecoderID].pVdec_Instance)) < 0)
		{
		#if 0
			{
				FILE	 *fp;
				char	  file_name[256];
				sprintf (file_name, "/storage/usb0/seq%d.raw", uiDecoderID);
				fp = fopen (file_name, "wb");
				if (fp)
				{
					fwrite ((char *) omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.stRingBuffInfo.pRingBuffBase[VA], 1, omx_private->inputCurrLength, fp);
					fclose (fp);
					sync ();
				}
			}
		#endif

			if(ret == -RETCODE_INVALID_STRIDE
				|| ret == -VPU_NOT_ENOUGH_MEM
				|| ret == -RETCODE_CODEC_SPECOUT
				|| ret == -RETCODE_CODEC_EXIT
				|| ret == -RETCODE_MULTI_CODEC_EXIT_TIMEOUT)
			{
				ALOGE("[VPU_ERROR-%d] [OP: VDEC_DEC_SEQ_HEADER] [RET_CODE: %d]", uiDecoderID, ret);
				pOutputBuffer->nFilledLen = 0;
				pthread_mutex_unlock(&omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.mutex);
				return;
			}

			if( UpdateRingBuffer(&omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.stRingBuffInfo, OMX_TRUE) < 0 )
			{
				goto ERR_PROCESS;
			}

			ALOGI("Retry sequence header initialization with more data");
			pthread_mutex_unlock(&omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.mutex);
			return;
		}

		if( UpdateRingBuffer(&omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.stRingBuffInfo, OMX_TRUE) < 0 )
		{
			ALOGE ("VDEC_DEC_SEQ_HEADER Decode Error , return ", uiDecoderID);
			pthread_mutex_unlock(&omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.mutex);
			return;
		}

		omx_private->pVideoDecodInstance[uiDecoderID].max_fifo_cnt	= VPU_BUFF_COUNT;
		omx_private->pVideoDecodInstance[uiDecoderID].avcodecInited	= 1;

		set_cpu_clock (openmaxStandComp, uiDecoderID);
#ifdef TS_TIMESTAMP_CORRECTION
        {
            unsigned int fRateInfoRes = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_uiFrameRateRes;
            unsigned int fRateInfoDiv = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_uiFrameRateDiv;

            omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iPTSInterval = 40000;
            outPort->sPortParam.format.video.xFramerate = 25;           
            if(fRateInfoRes && fRateInfoDiv)
            {                
                omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iPTSInterval = 1000000*fRateInfoDiv/fRateInfoRes;
				ALOGD("[%d]f_res %d f_div %d m-intr %d fdur %d\n", uiDecoderID, fRateInfoRes, fRateInfoDiv, omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iInterlace, omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iPTSInterval );

                if (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat == STD_AVC)
                {
                    /* !!! Be sure below checking is right or not. */
                    if(omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iInterlace != 1) //1:Frame, 0:Frame or Field
                        omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iPTSInterval = omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iPTSInterval*2;
                }                    

                ALOGD("%s:%d::[%d][%d]",__func__, __LINE__, omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iPTSInterval, omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iInterlace );
                if(omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iPTSInterval)
                {
                    unsigned int framerate_hz;
                    framerate_hz = 1000000/omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iPTSInterval;
                    ALOGD("%s:%d::FrameRate[%d]Hz",__func__, __LINE__, framerate_hz);
                    if(framerate_hz < 15)
                    {
                        framerate_hz = 15;
                        omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iPTSInterval = 66666;
                    }
                    if(framerate_hz>=60)
                    {
                        framerate_hz = 60;
                        omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iPTSInterval = 33000;
                    }
                    else
                    if(framerate_hz > 30 && framerate_hz != 50)
                    {
                        framerate_hz = 30;
                        omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iPTSInterval = 33000;
                    }

	                outPort->sPortParam.format.video.xFramerate = framerate_hz;
  #ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
        			tcc_vsync_command(omx_private,TCC_LCDC_VIDEO_SET_FRAMERATE, framerate_hz) ;  // TCC_LCDC_HDMI_GET_DISPLAYED 
  #endif
                }
                else
                {
                    omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iPTSInterval = 40000;
                    outPort->sPortParam.format.video.xFramerate = 25;
                }
            }
        }
#endif
		omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.llFeedLimitTimeDiff =\
					omx_private->pVideoDecodInstance[uiDecoderID].gsTSPtsInfo.m_iPTSInterval * 6;
		ALOGI("[DecoderID:%d] FeedLimitTimeDiff : %lld", uiDecoderID, omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.llFeedLimitTimeDiff);

#ifdef SUPPORT_SEARCH_IFRAME_ATSTARTING
        if(omx_private->stVideoSubFunFlag.SupportIFrameSearchMode)
        {
            omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iSkipFrameNum = 1;
            if(omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat == STD_MPEG2)
            {
                omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iFrameSearchEnable = 0x001;
            }
            else
			{
                omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iFrameSearchEnable = 0x201;	//I-frame (I-slice for H.264) : Non IDR-picture
            }
            omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iSkipFrameMode = VDEC_SKIP_FRAME_EXCEPT_I;
        }
        else
        {
            omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iSkipFrameMode = VDEC_SKIP_FRAME_DISABLE;
            omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iFrameSearchEnable = 0;
        }
#else
        omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iSkipFrameMode = VDEC_SKIP_FRAME_DISABLE;
        omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iFrameSearchEnable = 0;
#endif
    }

#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
	if(bRingRollback == OMX_TRUE)
	{
		ALOGE("Stream Rollback region");
		goto ERR_PROCESS;
	}	
#endif

	if (omx_private->pVideoDecodInstance[uiDecoderID].isVPUClosed == OMX_TRUE)
	{
		ALOGE ("Now VPU[%d] has been closed , return ", uiDecoderID);
		pthread_mutex_unlock(&omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.mutex);
		return;
	}

	ring_delete_vpu_displayed_index(omx_private, uiDecoderID);

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	/// Feed VPU decoder (fill ring-buffer)
	if( pInputBuffer->nFilledLen ) {
		if(!(pInputBuffer->nFlags & OMX_BUFFERFLAG_ONLY_FEED_FRAME)) {
			ret = FeedVpuDecoder(&omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.stRingBuffInfo,
									pInputBuffer,
									omx_private->inputCurrBuffer,
									omx_private->inputCurrLength,
									pInputBuffer->nTimeStamp,
									0 == uiDecoderID);
			switch(ret)
			{
				case FEED_FAILED:
					goto ERR_PROCESS;
					break;

				case NEED_MORE_DATA:
					pthread_mutex_unlock(&omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.mutex);
					return; //FEED_MORE_DATA

				default:
					//FEED_COMPLETE
					break;
			}
		}
	}

	if(omx_private->guiSkipBframeEnable)
	{
		//B-frame Skip Enable
		//ALOGE("[VDEC_DECODE]SKIP(B) START!!!");
		omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iSkipFrameMode = VDEC_SKIP_FRAME_ONLY_B;
		omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iSkipFrameNum = 1000;
	}

	ret = omx_private->pVideoDecodInstance[uiDecoderID].iDecod_status = omx_private->pVideoDecodInstance[uiDecoderID].gspfVDec (VDEC_DECODE, NULL, &omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput, &omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput, omx_private->pVideoDecodInstance[uiDecoderID].pVdec_Instance);

#if 1	//for Dual-decoding
	omx_private->stPreVideoInfo.iDecod_Instance = uiDecoderID;
	omx_private->stPreVideoInfo.iDecod_status = (ret == -RETCODE_INSUFFICIENT_BITSTREAM) ? ret:omx_private->pVideoDecodInstance[uiDecoderID].iDecod_status;
	omx_private->stPreVideoInfo.uDecod_time = ring_dbg_get_time();
#endif

	if (ret < 0)
	{
	
		if(ret == -RETCODE_CODEC_EXIT
			|| ret == -RETCODE_MULTI_CODEC_EXIT_TIMEOUT)
		{
			ALOGE ("[VDEC_DECODE:%d] [Err:%ld] video decode :: Len %d", uiDecoderID, ret, omx_private->inputCurrLength);
			goto ERR_PROCESS;
		}
		else if( ret == -RETCODE_INSUFFICIENT_BITSTREAM )
		{
			ringbuff_info_t *p_ring_info = &omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.stRingBuffInfo;
			input_info_t info_first;
			input_info_t info_last;

			//ERROR("[VPU_ERROR-%d:%d] Insufficient bitstream", uiDecoderID, omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodingStatus);

			if( ShowInputInfo(&omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.stInputInfoQueue, &info_first) >= 0 &&
			    ShowLastInputInfo(&omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.stInputInfoQueue, &info_last) >= 0 )
			{
				OMX_U8 *p_ring_base = p_ring_info->pRingBuffBase[VA];
				OMX_S32 start_pos = (OMX_S32)(info_first.pStartPtr - p_ring_base);
				OMX_S32 end_pos = (OMX_S32)(info_last.pEndPtr - p_ring_base);
				OMX_S32 data_size;

				if( start_pos < end_pos )
					data_size = end_pos - start_pos;
				else
					data_size = end_pos + (p_ring_info->lRingBuffSize - start_pos);

				DBUG_MSG("[INPUTQUE_STATE] [PTS: %8d ~ %8d] [PTR: %d ~ %d] [SIZE: %d]",
					 (OMX_S32)(info_first.llTimestamp/1000),
					 (OMX_S32)(info_last.llTimestamp/1000),
					 start_pos,
					 end_pos,
					 data_size);
			}

			DBUG_MSG("[RINGBUFF_STATE] [OMX_EMPTY: %d][OMX_WRITTEN: %d] [VPU_FILLED: %d]",
				 p_ring_info->lEmptySpace, 
				 p_ring_info->lWrittenBytes,
				 p_ring_info->lRingBuffSize - p_ring_info->lEmptySpace + p_ring_info->lWrittenBytes);

			//GetCurrTimestamp(&omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState, &curr_timestamp, OMX_FALSE, OMX_FALSE);

			pInputBuffer->nFilledLen = 0;
			pInputBuffer->nFlags &= ~OMX_BUFFERFLAG_ONLY_FEED_FRAME;

			pthread_mutex_unlock(&omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.mutex);
			return;
#if 0	//for Dual-decoding
			omx_private->stPreVideoInfo.iDecod_Instance = uiDecoderID;
			omx_private->stPreVideoInfo.iDecod_status = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodingStatus;
			omx_private->stPreVideoInfo.uDecod_time = ring_dbg_get_time();
			
			pInputBuffer->nFilledLen = 0;
			pOutputBuffer->nFilledLen = 0;
			pthread_mutex_unlock(&omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.mutex);
			return;
#endif
		}
		else
		{
			if(omx_private->pVideoDecodInstance[uiDecoderID].isVPUClosed == OMX_FALSE)
			{
				vdec_vpu(VDEC_CLOSE, NULL, NULL, &omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput, omx_private->pVideoDecodInstance[uiDecoderID].pVdec_Instance);
				omx_private->pVideoDecodInstance[uiDecoderID].isVPUClosed = OMX_TRUE;
			}
			pthread_mutex_unlock(&omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.mutex);
			return;
		}
	}

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	/// Get timestamp of the currently decoded frame
	///  - update ring buffer status
	///  - clear input information from the input info queue
	if( GetCurrTimestamp(&omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState, &curr_timestamp,
	                     omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodingStatus & (VPU_DEC_SUCCESS | VPU_DEC_SUCCESS_FIELD_PICTURE),
	                     //OMX_FALSE,
	                     omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iInterlacedFrame) < 0 )
	{
		goto ERR_PROCESS;
	}
#if 0
	//if(uiDecoderID == 0)
	{
		if(isDecodTimeStamp[uiDecoderID] == 0)
		{
			isDecodTimeStamp[uiDecoderID] = curr_timestamp;
		}

		if((curr_timestamp-isDecodTimeStamp[uiDecoderID]) >= omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.llFeedLimitTimeDiff)
		{
			pInputBuffer->nFilledLen = 0;
			pInputBuffer->nFlags &= ~OMX_BUFFERFLAG_ONLY_FEED_FRAME;
			isDecodTimeStamp[uiDecoderID] = 0;
		}
	}
#endif
//	if(uiDisplayID == uiDecoderID)
//		ALOGE("[DisplayID:%d]TimeStamp:%lld", uiDisplayID, curr_timestamp);

	//pInputBuffer->nTimeStamp = curr_timestamp;

	if(omx_private->guiSkipBframeEnable)
	{
		if( (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iPicType == PIC_TYPE_B)
			&& (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx == -2)
			&& (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_SUCCESS) )
		{
			if( omx_private->guiSkipBFrameNumber-- == 0)
			{
				//B-frame Skip Disable
				//ALOGE("[VDEC_DECODE]SKIP(B) END!!!");
				omx_private->guiSkipBframeEnable = 0;
				omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iSkipFrameMode = VDEC_SKIP_FRAME_DISABLE;
				omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iSkipFrameNum = 0;
			}	
			//else
			//	ALOGE("[VDEC_DECODE]SKIPPING(B) %d!!!", omx_private->guiSkipBFrameNumber);
		}	
	}	

	if(omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iFrameSearchEnable != 0
		&& omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx >= 0)
	{
		if(omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat == STD_MPEG2)
		{
			omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iSkipFrameMode = VDEC_SKIP_FRAME_ONLY_B;
			omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iSkipFrameNum = 1;
		}
		else
		{
			omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iSkipFrameMode = VDEC_SKIP_FRAME_DISABLE;
			omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iSkipFrameNum = 0;			
		}
		omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iFrameSearchEnable = 0; 
		SET_FLAG(omx_private->pVideoDecodInstance[uiDecoderID], STATE_SKIP_OUTPUT_B_FRAME);
	}
	else
	{
		if(omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat == STD_MPEG2)
		{
			if(omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iSkipFrameMode == VDEC_SKIP_FRAME_ONLY_B
				&& omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx >= 0 )
			{
				omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iSkipFrameMode = VDEC_SKIP_FRAME_DISABLE;
				omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iSkipFrameNum = 0;
				
				ALOGE("[CHANGE:%d] m_iBitstreamFormat[%d] m_iSkipFrameMode[%d] m_iSkipFrameNum[%d]", uiDecoderID,
					omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat,
					omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iSkipFrameMode,
					omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInput.m_iSkipFrameNum);
			}
		}
	}
	
	//m_iPicSideInfo :: bit0(H.264 Decoded Picture Buffer error notification) bit1(H.264 weighted prediction)
	//ALOGE("[VDEC_DECODE]iDecodingStatus(%d)iPicSideInfo(0x%x)",omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodingStatus, omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iPicSideInfo);
	if (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_BUF_FULL)
	{
		// Stream은 현재 Stream을 다시 돌려야 함.
		ALOGE ("[%d] VPU_DEC_BUF_FULL :: Len %d, OutputStatus(%d)", uiDecoderID, omx_private->inputCurrLength, omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iOutputStatus);
		if (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iOutputStatus == VPU_DEC_OUTPUT_SUCCESS)
			decode_result = 0;	// Display Index 정상 처리, Stream은 현재 Stream 재사용
		else
		{
			omx_private->pVideoDecodInstance[uiDecoderID].guiDisplayBufFullCount++;			
            ALOGD("%s:%d:[%d]",__func__, __LINE__, omx_private->pVideoDecodInstance[uiDecoderID].guiDisplayBufFullCount);
			if(omx_private->pVideoDecodInstance[uiDecoderID].guiDisplayBufFullCount > 15 )
			{
			    omx_private->pVideoDecodInstance[uiDecoderID].guiDisplayBufFullCount = 0;
				ALOGE("[VDEC_DECODE]Too many VPU_DEC_BUF_FULL !!! try to re-initialize");
				goto ERR_PROCESS;			
			}
			decode_result = 1;	// Display Index 미처리, Stream은 현재 Stream 재사용
		}
	}
	else
	{
		omx_private->pVideoDecodInstance[uiDecoderID].guiDisplayBufFullCount = 0;
		if (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iOutputStatus == VPU_DEC_OUTPUT_SUCCESS)
			decode_result = 2;	// Display Index 정상처리, Stream은 다음 Stream
		else
			decode_result = 3;	// Display Index 미처리, Stream은 다음 Stream
	}
	
	//Update TimeStamp!!
	if (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_SUCCESS
		&& omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx >= 0)
	{
		OMX_U32   width, height;
		OMX_S32   frame_rate = 0;
		OMX_S32   decoded_idx = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx;
		/* Frame-rate recalcuation */
		omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.lFrameSuccessCount++;
		if(0)// omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.llQueuedStartTime != curr_timestamp )
		{
			if( omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.llQueuedStartTime < curr_timestamp )
			{
				//CLEAR_FLAG(p_private, STATE_FRAME_RATE_UPDATE);
			}
			else if( omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.lFrameSuccessCount > 1 )
			{
				OMX_TICKS frame_due = (omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.llQueuedStartTime - curr_timestamp) / omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.lFrameSuccessCount;
				omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.lNewFrameRate = (OMX_S32)((OMX_TICKS)1000000000 / frame_due);
			}
			omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.lFrameSuccessCount = 0;
		}
#if 1
//		if(omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat == STD_MPEG2)
		{
			//ALOGE("=====> m_uiFrameRateRes:%d", omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_uiFrameRateRes);
			//ALOGE("=====> m_iM2vFrameRate:%d", omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iM2vFrameRate);
			frame_rate = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_uiFrameRateRes/1000;
			frame_rate = ((frame_rate & 0xffff) * 1000) / ((frame_rate >> 16) + 1);
		}

		if( omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.lNewFrameRate && frame_rate == 0 )
		{
			frame_rate = omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.lNewFrameRate;
			omx_private->pVideoDecodInstance[uiDecoderID].stRingBuffState.lNewFrameRate = 0;
		}
#else
		frame_rate = outPort->sPortParam.format.video.xFramerate * 1000;
#endif
#if 0        
		if(omx_private->iInterlaceInfo == 0) //1:NON-Interlaced Frame, 0:Interlaced Frame
		{
			frame_rate /= 2;
		}
#endif

		dec_disp_info_tmp.m_iTimeStamp = (int) (curr_timestamp / 1000);
		dec_disp_info_tmp.m_iFrameType = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iPicType;
		dec_disp_info_tmp.m_iPicStructure = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iPictureStructure;
		dec_disp_info_tmp.m_iextTimeStamp = 0;
		dec_disp_info_tmp.m_iM2vFieldSequence = 0;
		dec_disp_info_tmp.m_iFrameSize = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iConsumedBytes;	// gsCDmxOutput.m_iPacketSize;
		dec_disp_info_tmp.m_iFrameDurationFactor = 2;
		dec_disp_info_tmp.m_iFrameRate = frame_rate;

		width = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iWidth;
		height = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iHeight;
		if (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat == STD_AVC)
		{
			width -= omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropLeft;
			width -= omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropRight;
			height -= omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropBottom;
			height -= omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropTop;
		}
		dec_disp_info_tmp.m_iWidth = width;
		dec_disp_info_tmp.m_iHeight = height;

		switch (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat)
		{
		int  is_progressive;		    
		case STD_MPEG2:
			if (dec_disp_info_tmp.m_iPicStructure != 3)
			{
				dec_disp_info_tmp.m_iFrameDurationFactor = 2;
			}
			else if (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iInterlace == 0)
			{
				if (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iRepeatFirstField == 0)
					dec_disp_info_tmp.m_iFrameDurationFactor = 2;
				else
					dec_disp_info_tmp.m_iFrameDurationFactor =
						(omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iTopFieldFirst == 0) ? 4 : 6;
			}
			else
			{
				/* interlaced sequence */
				if (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iInterlacedFrame == 0)
					dec_disp_info_tmp.m_iFrameDurationFactor = 2;
				else
					dec_disp_info_tmp.m_iFrameDurationFactor =
						(omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iRepeatFirstField == 0) ? 2 : 3;
			}
			if( omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iInterlace
				|| ( ( omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iM2vProgressiveFrame == 0 
				&& omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iPictureStructure == 0x3 ) 
				|| omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iInterlacedFrame ) )
			{
				is_progressive = 0;
			}
			else
			{
				is_progressive = 1;
			}			
			dec_disp_info_tmp.m_iIsProgressive = is_progressive;
			dec_disp_info_tmp.m_iM2vFieldSequence = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iM2vFieldSequence;
			dec_disp_info_tmp.m_iTopFieldFirst = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iTopFieldFirst;
			omx_private->pVideoDecodInstance[uiDecoderID].dec_disp_info_input.m_iFrameIdx = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx;
			omx_private->pVideoDecodInstance[uiDecoderID].dec_disp_info_input.m_iFrameRate = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iM2vFrameRate;
			disp_pic_info (CVDEC_DISP_INFO_UPDATE,
							(void *) &(omx_private->pVideoDecodInstance[uiDecoderID].dec_disp_info_ctrl),
							(void *) &dec_disp_info_tmp,
							(void *) &(omx_private->pVideoDecodInstance[uiDecoderID].dec_disp_info_input),
							omx_private,
							uiDecoderID);
			#ifdef VIDEO_USER_DATA_PROCESS
			{
				video_userdata_list_t stUserDataList;
				GetUserDataListFromCDK((unsigned char *)omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_UserDataAddress[VA], &stUserDataList);
				//DBUG_MSG("[userdata] put %2d, %10d\n", omx_private->gsVDecOutput[uiDecoderID].m_DecOutInfo.m_iDecodedIdx,(int)(curr_timestamp/1000) );
				UserDataCtrl_Put(omx_private->pVideoDecodInstance[uiDecoderID].pUserData_List_Array,
								omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx,
								curr_timestamp, 0, &stUserDataList);
			}
			#endif
			break;

		case STD_AVC:            
		default:
			if((omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iM2vProgressiveFrame == 0
				&& omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iPictureStructure == 0x3)
				|| omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iInterlacedFrame
				|| ((omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iPictureStructure  ==1) 
				&& (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iInterlace ==0))
			)
			{
				if(omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iPictureStructure == 1)
					omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iTopFieldFirst = 1;

				is_progressive = 0;
			}
			else
			{
				is_progressive = 1;
			}

			dec_disp_info_tmp.m_iIsProgressive = is_progressive;
			dec_disp_info_tmp.m_iM2vFieldSequence = 0;
			dec_disp_info_tmp.m_iTopFieldFirst = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iTopFieldFirst;
			omx_private->pVideoDecodInstance[uiDecoderID].dec_disp_info_input.m_iFrameIdx = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx;
			disp_pic_info (CVDEC_DISP_INFO_UPDATE, (void *) &(omx_private->pVideoDecodInstance[uiDecoderID].dec_disp_info_ctrl), (void *) &dec_disp_info_tmp,
						   (void *) &(omx_private->pVideoDecodInstance[uiDecoderID].dec_disp_info_input), omx_private, uiDecoderID);
			break;
		}
		DPRINTF_DEC ("IN-Buffer :: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
					 pInputBufer[0], pInputBufer[1], pInputBufer[2],
					 pInputBufer[3], pInputBufer[4], pInputBufer[5],
					 pInputBufer[6], pInputBufer[7], pInputBufer[8],
					 pInputBufer[9]);
		//current decoded frame info
		DPRINTF_DEC ("[In - %s][N:%4d][LEN:%6d][RT:%8d] [DecoIdx:%2d][DecStat:%d][FieldSeq:%d][TR:%8d] ",
					 ring_print_pic_type (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat, dec_disp_info_tmp.m_iFrameType,
									 dec_disp_info_tmp.m_iPicStructure), omx_private->pVideoDecodInstance[uiDecoderID].video_dec_idx, pInputBuffer->nFilledLen,
					 (int) (curr_timestamp / 1000), omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx,
					 omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodingStatus,
					 omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iM2vFieldSequence,
					 omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iRvTimestamp);
	}
	else
	{
		DPRINTF_DEC ("IN-Buffer :: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
			  pInputBufer[0], pInputBufer[1], pInputBufer[2],
			  pInputBufer[3], pInputBufer[4], pInputBufer[5],
			  pInputBufer[6], pInputBufer[7], pInputBufer[8],
			  pInputBufer[9]);
		DPRINTF_DEC ("[Err In - %s][N:%4d][LEN:%6d][RT:%8d] [DecoIdx:%2d][DecStat:%d][OutStat:%d][FieldSeq:%d][TR:%8d] ",
			  ring_print_pic_type (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat, dec_disp_info_tmp.m_iFrameType,
							  dec_disp_info_tmp.m_iPicStructure), omx_private->pVideoDecodInstance[uiDecoderID].video_dec_idx, pInputBuffer->nFilledLen,
			  (int) (curr_timestamp / 1000), omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx,
			  omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodingStatus,
			  omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iOutputStatus,
			  omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iM2vFieldSequence,
			  omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iRvTimestamp);

		omx_private->pVideoDecodInstance[uiDecoderID].guiDisplayBufFullCount++;			
		DPRINTF_DEC("%s:%d:[%d]",__func__, __LINE__, omx_private->pVideoDecodInstance[uiDecoderID].guiDisplayBufFullCount);
		if(omx_private->pVideoDecodInstance[uiDecoderID].guiDisplayBufFullCount > 100 )
		{
			omx_private->pVideoDecodInstance[uiDecoderID].guiDisplayBufFullCount = 0;
			ALOGE("[VDEC_DECODE:%d]Too many VPU_DEC_BUF_FULL !!! try to re-initialize", uiDecoderID);
			goto ERR_PROCESS;			
		}
	}

	if(omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iNumOfErrMBs>0)
		ALOGE("%s %d m_iNumOfErrMBs = %d \n", __func__, __LINE__, omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iNumOfErrMBs);

	Decoding_fieldInfo = ((omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iPicType& (3 << 16)) >> 16);
	iPicType =(unsigned char)(omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iPicType &VIDEO_PICTYPE_MASK);

	if(omx_private->stVideoSubFunFlag.SupportFieldDecoding)
	{
		ret = ring_field_decoding_process(omx_private, uiDecoderID, Decoding_fieldInfo);
		if(ret !=0)
			decode_result =3;
	}

	if(omx_private->stIPTVActiveModeInfo.IPTV_PLAYMode)
	{
		ret = ring_for_iptv_trickModeEnd_process(omx_private, uiDecoderID);
		if(ret == -1)
			decode_result =3;

		ret  = ring_for_iptv_brokenframe_check(omx_private, uiDecoderID);
		ret  = ring_for_iptv_ChannelChange_process(openmaxStandComp, uiDecoderID, iPicType);
		if(ret == -1)
			decode_result =3;
		else if(ret== 1)
			uiDisplayID = uiDecoderID =0;

		ret  = ring_for_iptv_brokenframe_process(omx_private, uiDecoderID, outPort->sPortParam.format.video.xFramerate);
		if(ret == -1)
			decode_result =3;
	}	

	//////////////////////////////////////////////////////////////////////////////////////////
	if((omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_DETECT_RESOLUTION_CHANGE) &&
		((omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iPicWidth != omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iWidth) ||
		(omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pInitialInfo->m_iPicHeight != omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iHeight)))
	{
		ALOGE("%s %d \n", __func__, __LINE__);
		ret = isPortChange (openmaxStandComp, uiDecoderID);
		if (ret < 0)	//max-resolution over!!
		{
			goto ERR_PROCESS;
		}
	}

	if (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iOutputStatus == VPU_DEC_OUTPUT_SUCCESS/* && uiDisplayID == uiDecoderID*/)
	{	
		ret = isPortChange (openmaxStandComp, uiDecoderID);
		if (ret < 0)	//max-resolution over!!
		{
			goto ERR_PROCESS;
		}
		
		if(omx_private->stIPTVActiveModeInfo.IPTV_PLAYMode
			&& (omx_private->stfristframe_dsipInfo.TopFrame_DecStatus == 1 && omx_private->stfristframe_dsipInfo.BottomFrame_DecStatus == 1))
		{
			/* physical address */
			for (i = 0; i < 3; i++)
				memcpy (pOutputBuffer->pBuffer + i * 4, &omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pCurrOut[PA][i], 4);

			/* logical address */
			for (i = 3; i < 6; i++)
				memcpy (pOutputBuffer->pBuffer + i * 4, &omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pCurrOut[VA][i - 3], 4);
		}
		else
		{
			/* physical address */
			for (i = 0; i < 3; i++)
				memcpy (pOutputBuffer->pBuffer + i * 4, &omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pDispOut[PA][i], 4);

			/* logical address */
			for (i = 3; i < 6; i++)
				memcpy (pOutputBuffer->pBuffer + i * 4, &omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pDispOut[VA][i - 3], 4);
		}
		
		/*Save Display out index in order to clear it at sendbufferfunction
		 */
		memcpy (pOutputBuffer->pBuffer + 24, &omx_private->pVideoDecodInstance[uiDecoderID].buffer_unique_id, 4);
		*(unsigned int *)(pOutputBuffer->pBuffer + 48) = isVsyncEnable();

		// "+1" is meaningless. (main_unique_addr:1, sub_unique_addr:2)
		//*(unsigned int *)(pOutputBuffer->pBuffer + 52) = uiDecoderID + 1;
		*(unsigned int *)(pOutputBuffer->pBuffer + 52) = omx_private->pVideoDecodInstance[uiDecoderID].ulDecoderUID + 1;
		
		omx_private->pVideoDecodInstance[uiDecoderID].Send_Buff_ID[omx_private->pVideoDecodInstance[uiDecoderID].buffer_unique_id%VPU_REQUEST_BUFF_COUNT] 
				= (uiDecoderID<<16) + omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDispOutIdx + 1;
	
		nDisplayIndex = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDispOutIdx;
		//Get TimeStamp!!
		{
			dec_disp_info_t *pdec_disp_info = NULL;
			int dispOutIdx = omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDispOutIdx;
			omx_private->pVideoDecodInstance[uiDecoderID].dec_disp_info_input.m_iFrameIdx = dispOutIdx;
			disp_pic_info (CVDEC_DISP_INFO_GET,
							(void *) &(omx_private->pVideoDecodInstance[uiDecoderID].dec_disp_info_ctrl),
							(void *) &pdec_disp_info,
							(void *) &(omx_private->pVideoDecodInstance[uiDecoderID].dec_disp_info_input),
							omx_private,
							uiDecoderID);

			if (pdec_disp_info != (dec_disp_info_t *) 0)
			{
				if (CHECK_FLAG(omx_private->pVideoDecodInstance[uiDecoderID], STATE_SKIP_OUTPUT_B_FRAME))
				{
					OMX_S32 pic_type = GetFrameType(omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat,
													pdec_disp_info->m_iFrameType, pdec_disp_info->m_iPicStructure);
					if (pic_type == PIC_TYPE_I || pic_type == PIC_TYPE_P)
					{
						CLEAR_FLAG(omx_private->pVideoDecodInstance[uiDecoderID], STATE_SKIP_OUTPUT_B_FRAME);
					}
					else
					{
						ALOGD("Skip B-frame before I-frame");
						decode_result = 3;
					}
				}

				pOutputBuffer->nTimeStamp = (OMX_TICKS) pdec_disp_info->m_iTimeStamp * 1000;	//pdec_disp_info->m_iM2vFieldSequence * 1000;

				memcpy (pOutputBuffer->pBuffer + 28, &pdec_disp_info->m_iTopFieldFirst, 4);
				//ALOGE("OutputBuffer : %d", pdec_disp_info->m_iTopFieldFirst);
				memcpy (pOutputBuffer->pBuffer + 32, &pdec_disp_info->m_iIsProgressive, 4);
				memcpy (pOutputBuffer->pBuffer + 36, &pdec_disp_info->m_iFrameType, 4);
				memcpy (pOutputBuffer->pBuffer + 40, &pdec_disp_info->m_iWidth, 4);
				memcpy (pOutputBuffer->pBuffer + 44, &pdec_disp_info->m_iHeight, 4);
				memcpy (pOutputBuffer->pBuffer + 56, &dispOutIdx, 4);
				memcpy (pOutputBuffer->pBuffer + 60, &uiDecoderID, 4);
				nOutputFrameWidth = pdec_disp_info->m_iWidth;
				nOutputFrameHeight = pdec_disp_info->m_iHeight;

				#ifdef VIDEO_USER_DATA_PROCESS
				if (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_bEnableUserData)
				{
					video_userdata_list_t *pUserDataList;
					if( UserDataCtrl_Get( omx_private->pVideoDecodInstance[uiDecoderID].pUserData_List_Array, dispOutIdx, &pUserDataList ) >= 0 )
					{
						DBUG_MSG("[userdata]\t\tget %2d, %10d\n", dispOutIdx ,(int)(pUserDataList->ullPTS/1000) );
						print_user_data (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_UserDataAddress[VA]);
						process_user_data( openmaxStandComp, pUserDataList);
						UserDataCtrl_Clear( pUserDataList );
					}
				}
				#endif

				DPRINTF_DEC ("[Out - %s][N:%4d][LEN:%6d][RT:%8d] [DispIdx:%2d][OutStat:%d][FieldSeq:%d][TR:%8d] ",
							 ring_print_pic_type (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_iBitstreamFormat,
											 pdec_disp_info->m_iFrameType, pdec_disp_info->m_iPicStructure),
							 omx_private->pVideoDecodInstance[uiDecoderID].video_dec_idx, pdec_disp_info->m_iFrameSize, pdec_disp_info->m_iTimeStamp,
							 omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDispOutIdx,
							 omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iOutputStatus,
							 pdec_disp_info->m_iM2vFieldSequence, pdec_disp_info->m_iextTimeStamp);
			}
			else
			{
				//exception process!! temp!!
				pOutputBuffer->nTimeStamp = curr_timestamp;
			}
		}
		
		if(decode_result != 3)
		{
			omx_private->pVideoDecodInstance[uiDecoderID].video_dec_idx++;
			omx_private->pVideoDecodInstance[uiDecoderID].buffer_unique_id++;
		}
	}
	else
	{
        if( (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodingStatus != VPU_DEC_SUCCESS)
			&& (omx_private->pVideoDecodInstance[uiDecoderID].iDecod_status != -RETCODE_INSUFFICIENT_BITSTREAM) )
			ALOGE ("[VDEC_DECODE:%d] NO-OUTPUT!! m_iDispOutIdx = %d, m_iDecodedIdx = %d, m_iOutputStatus = %d, m_iDecodingStatus = %d, m_iNumOfErrMBs = %d", uiDecoderID, omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDispOutIdx, omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodedIdx, omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iOutputStatus, omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iDecodingStatus, omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_iNumOfErrMBs);
	}

	// To process input stream retry or next frame
	switch (decode_result)
	{
	case 0:	// Display Output Success, Decode Failed Due to Buffer Full
		pOutputBuffer->nFilledLen =
					nOutputFrameWidth * nOutputFrameHeight * 3 / 2;		
		break;

	case 1:	// Display Output Failed, Decode Failed Due to Buffer Full
		break;

	case 2:	// Display Output Success, Decode Success
		pOutputBuffer->nFilledLen =
					nOutputFrameWidth * nOutputFrameHeight * 3 / 2;		
		break;

	case 3:	// Display Output Failed, Decode Success        			
		break;

	default:
		break;
	}

	if(omx_private->pVideoDecodInstance[uiDecoderID].bVideoPaused == OMX_TRUE /*|| uiDisplayID != uiDecoderID*/)
	{
		pOutputBuffer->nFilledLen = 0;
 	}
 
	if( pOutputBuffer->nFilledLen == 0 )
	{
		if( nDisplayIndex != -1 )
		{
		#ifdef VIDEO_USER_DATA_PROCESS
			if (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecInit.m_bEnableUserData)
			{
				video_userdata_list_t *pUserDataList;
				if( UserDataCtrl_Get( omx_private->pVideoDecodInstance[uiDecoderID].pUserData_List_Array, nDisplayIndex, &pUserDataList ) >= 0 )
				{
					DBUG_MSG("[userdata]\t\tget %2d, %10d\n", nDisplayIndex ,(int)(pUserDataList->ullPTS/1000) );
					print_user_data (omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_DecOutInfo.m_UserDataAddress[VA]);
					//process_user_data( openmaxStandComp, pUserDataList);
					UserDataCtrl_Clear( pUserDataList );
				}
			}
		#endif

			nDisplayIndex++;
			DPRINTF_DEC_STATUS("%s DispIdx Queue %d, %d", __func__, uiDecoderID, nDisplayIndex);
			if( queue_ex(omx_private->pVideoDecodInstance[uiDecoderID].pVPUDisplayedIndexQueue, nDisplayIndex) == 0 )
			{
				nDisplayIndex--;
				ret = omx_private->pVideoDecodInstance[uiDecoderID].gspfVDec (VDEC_BUF_FLAG_CLEAR, NULL, &nDisplayIndex, NULL, omx_private->pVideoDecodInstance[uiDecoderID].pVdec_Instance);
				ALOGE ("%s VDEC_BUF_FLAG_CLEAR Err : [%d] index : %d",__func__, ret, nDisplayIndex);
			}
		}
	}

#ifdef	SUPPORT_SAVE_OUTPUTBUFFER
	{
		FILE     *fp;
		char      file_name[32];
		int       iVideoWidth, iVideoHeight;
		if (giDumpVideoFileIndex < 20)
		{
			sprintf (file_name, "/sdcard/out_video%d.raw", ++giDumpVideoFileIndex);
			fp = fopen (file_name, "wb");
			if (fp)
			{
				iVideoWidth = outPort->sPortParam.format.video.nFrameWidth;
				iVideoHeight = outPort->sPortParam.format.video.nFrameHeight;
				ALOGE ("dump file created : %s (%dx%d)", file_name, iVideoWidth, iVideoHeight);
				fwrite ((char *) omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pDispOut[VA][0], 1, iVideoWidth * iVideoHeight, fp);
				fwrite ((char *) omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pDispOut[VA][1], 1, iVideoWidth * iVideoHeight / 4, fp);
				fwrite ((char *) omx_private->pVideoDecodInstance[uiDecoderID].gsVDecOutput.m_pDispOut[VA][2], 1, iVideoWidth * iVideoHeight / 4, fp);
				fclose (fp);
				sync ();
			}
		}
	}
#endif
	//ALOGE("PTS = %lld", pOutputBuffer->nTimeStamp);    
	pthread_mutex_unlock(&omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.mutex);
	return;

  ERR_PROCESS:

	pOutputBuffer->nFilledLen = 0;
	pInputBuffer->nFilledLen = 0;	

	if (omx_private->pVideoDecodInstance[0].iDecod_status == -RETCODE_INSUFFICIENT_BITSTREAM ||
		omx_private->pVideoDecodInstance[1].iDecod_status == -RETCODE_INSUFFICIENT_BITSTREAM)
	{
		if(omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_SUB].isVPUClosed  == OMX_FALSE)
		{
			isDecodTimeStamp[TCC_DXBVIDEO_DISPLAY_SUB] = 0;
			omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_SUB].iDecod_status = -1;

			/* reset VPU ring-buffer state */
			ResetRingBuffer(&omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_SUB].stRingBuffState.stRingBuffInfo);
			/* reset input/output information managers */
			ClearInputQueue(&omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_SUB].stRingBuffState.stInputInfoQueue);

			disp_pic_info (CVDEC_DISP_INFO_RESET, (void *) &(omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_SUB].dec_disp_info_ctrl), \
							(void *) omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_SUB].dec_disp_info, \
						   (void *) &(omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_SUB].dec_disp_info_input), omx_private, TCC_DXBVIDEO_DISPLAY_SUB);

			omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_SUB].stRingBuffState.lInputCount = 0;
			omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_SUB].stRingBuffState.lRollbackCheckCount = 0;
			omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_SUB].stRingBuffState.llQueuedStartTime = -1;
			omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_SUB].stRingBuffState.lFrameSuccessCount = 0;
			omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_SUB].stRingBuffState.lNewFrameRate = 0;

			omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_SUB].avcodecInited = 0;

			omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_SUB].gspfVDec (VDEC_SW_RESET, NULL, NULL, NULL, omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_SUB].pVdec_Instance);
			if(omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_SUB].gspfVDec (VDEC_CLOSE, TCC_DXBVIDEO_DISPLAY_SUB, NULL, &omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_SUB].gsVDecOutput, omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_SUB].pVdec_Instance) == 0)
				omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_SUB].isVPUClosed = OMX_TRUE;

#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
			bRingRollback = OMX_FALSE;
			if(omx_private->iVsyncMode) {
				omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_SUB].in_index = 0;
				omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_SUB].out_index = 0;
				omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_SUB].used_fifo_count = 0;
			}
#endif
		}

		if(omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].isVPUClosed == OMX_FALSE)
		{
			isDecodTimeStamp[TCC_DXBVIDEO_DISPLAY_MAIN] = 0;
			omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].iDecod_status = -1;

			/* reset VPU ring-buffer state */
			ResetRingBuffer(&omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].stRingBuffState.stRingBuffInfo);
			/* reset input/output information managers */
			ClearInputQueue(&omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].stRingBuffState.stInputInfoQueue);

			disp_pic_info (CVDEC_DISP_INFO_RESET, (void *) &(omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].dec_disp_info_ctrl), \
							(void *) omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].dec_disp_info, \
						   (void *) &(omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].dec_disp_info_input), omx_private, TCC_DXBVIDEO_DISPLAY_MAIN);

			omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].stRingBuffState.lInputCount = 0;
			omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].stRingBuffState.lRollbackCheckCount = 0;
			omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].stRingBuffState.llQueuedStartTime = -1;
			omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].stRingBuffState.lFrameSuccessCount = 0;
			omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].stRingBuffState.lNewFrameRate = 0;

			omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].avcodecInited = 0;

			omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].gspfVDec (VDEC_SW_RESET, NULL, NULL, NULL, omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].pVdec_Instance);
//			omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].gspfVDec (VDEC_HW_RESET, TCC_DXBVIDEO_DISPLAY_MAIN, NULL, &omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].gsVDecOutput, omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].pVdec_Instance);
			if(omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].gspfVDec (VDEC_CLOSE, TCC_DXBVIDEO_DISPLAY_MAIN, NULL, &omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].gsVDecOutput, omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].pVdec_Instance) == 0)
				omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].isVPUClosed = OMX_TRUE;

#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
			bRingRollback = OMX_FALSE;
			if(omx_private->iVsyncMode) {
				omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].in_index = 0;
				omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].out_index = 0;
				omx_private->pVideoDecodInstance[TCC_DXBVIDEO_DISPLAY_MAIN].used_fifo_count = 0;
			}
#endif
			omx_private->stPreVideoInfo.iDecod_Instance = -1;
			omx_private->stPreVideoInfo.iDecod_status = -1;
			omx_private->stPreVideoInfo.uDecod_time = 0;
		}
		ALOGE ("%s:%d: Multi instance error END\n", __func__, __LINE__);
	}
	else
	{
		for(i=iTotalHandle-1;i>=0;i--)
		{
			isDecodTimeStamp[i] = 0;

			if(omx_private->pVideoDecodInstance[i].isVPUClosed == OMX_FALSE)
			{
				omx_private->pVideoDecodInstance[i].iDecod_status = -1;
	
				/* reset VPU ring-buffer state */
				ResetRingBuffer(&omx_private->pVideoDecodInstance[i].stRingBuffState.stRingBuffInfo);
				/* reset input/output information managers */
				ClearInputQueue(&omx_private->pVideoDecodInstance[i].stRingBuffState.stInputInfoQueue);
	
				disp_pic_info (CVDEC_DISP_INFO_RESET, (void *) &(omx_private->pVideoDecodInstance[i].dec_disp_info_ctrl), \
								(void *) omx_private->pVideoDecodInstance[i].dec_disp_info, \
							   (void *) &(omx_private->pVideoDecodInstance[i].dec_disp_info_input), omx_private, i);
	
				omx_private->pVideoDecodInstance[i].stRingBuffState.lInputCount = 0;
				omx_private->pVideoDecodInstance[i].stRingBuffState.lRollbackCheckCount = 0;
				omx_private->pVideoDecodInstance[i].stRingBuffState.llQueuedStartTime = -1;
				omx_private->pVideoDecodInstance[i].stRingBuffState.lFrameSuccessCount = 0;
				omx_private->pVideoDecodInstance[i].stRingBuffState.lNewFrameRate = 0;
	
				omx_private->pVideoDecodInstance[i].avcodecInited = 0;
//				if(i==0 && get_total_handle_cnt(openmaxStandComp)<1)
//				{
//					omx_private->pVideoDecodInstance[i].gspfVDec (VDEC_HW_RESET, i, NULL, &omx_private->pVideoDecodInstance[i].gsVDecOutput, omx_private->pVideoDecodInstance[i].pVdec_Instance);
//				}
				omx_private->pVideoDecodInstance[i].gspfVDec (VDEC_SW_RESET, NULL, NULL, NULL, omx_private->pVideoDecodInstance[i].pVdec_Instance);
				if(omx_private->pVideoDecodInstance[i].gspfVDec (VDEC_CLOSE, i, NULL, &omx_private->pVideoDecodInstance[i].gsVDecOutput, omx_private->pVideoDecodInstance[i].pVdec_Instance) == 0)
					omx_private->pVideoDecodInstance[i].isVPUClosed = OMX_TRUE;
	
				//(*(omx_private->callbacks->EventHandler)) (openmaxStandComp, omx_private->callbackData,OMX_EventError, OMX_ErrorHardware, 0, NULL);
#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
				bRingRollback = OMX_FALSE;
				if(omx_private->iVsyncMode) {
		//			tcc_vsync_command(omx_private, TCC_LCDC_VIDEO_END_VSYNC, NULL);
					omx_private->pVideoDecodInstance[i].in_index = 0;
					omx_private->pVideoDecodInstance[i].out_index = 0;
					omx_private->pVideoDecodInstance[i].used_fifo_count = 0;
		//			tcc_vsync_command(omx_private, TCC_LCDC_VIDEO_START_VSYNC, VPU_BUFF_COUNT);
				}
#endif
			}
		}
	}

	pthread_mutex_unlock(&omx_private->pVideoDecodInstance[uiDecoderID].stVideoStart.mutex);
	return;
}

OMX_ERRORTYPE omx_ring_videodec_component_SetParameter (OMX_IN OMX_HANDLETYPE hComponent,
												   OMX_IN OMX_INDEXTYPE nParamIndex,
												   OMX_IN OMX_PTR ComponentParameterStructure)
{

	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_U32   portIndex;
	INT32    *piArg;

	/* Check which structure we are being fed and make control its header */
	OMX_COMPONENTTYPE *openmaxStandComp = hComponent;
	omx_videodec_component_PrivateType *omx_private = openmaxStandComp->pComponentPrivate;
	omx_base_video_PortType *port;
	if (ComponentParameterStructure == NULL)
	{
		return OMX_ErrorBadParameter;
	}

	DBUG_MSG ("   Setting parameter %i\n", nParamIndex);
	switch (nParamIndex)
	{
	case OMX_IndexParamPortDefinition:
		break;		

	case OMX_IndexParamStandardComponentRole:
		{
			OMX_PARAM_COMPONENTROLETYPE *pComponentRole;
			pComponentRole = ComponentParameterStructure;
			if (!strcmp ((char *) pComponentRole->cRole, VIDEO_DEC_TCC_H264_ROLE))
			{
				omx_private->pVideoDecodInstance[omx_private->iDisplayID].video_coding_type = OMX_VIDEO_CodingAVC;
			}			
			else if (!strcmp ((char *) pComponentRole->cRole, VIDEO_DEC_MPEG2_ROLE))
			{
				omx_private->pVideoDecodInstance[omx_private->iDisplayID].video_coding_type = OMX_VIDEO_CodingMPEG2;
			}
			else
			{
				return OMX_ErrorBadParameter;
			}
			break;
		}

	case OMX_IndexParamVideoSkipFrames:
		omx_private->guiSkipBframeEnable = 1;//enable
		omx_private->guiSkipBFrameNumber = (unsigned int)ComponentParameterStructure; 		
		break;

	case OMX_IndexParamVideoSetStart:
		{
			OMX_VIDEO_PARAM_STARTTYPE *pStartInfo;
			pStartInfo = (OMX_VIDEO_PARAM_STARTTYPE*) ComponentParameterStructure;
            if(pStartInfo->bStartFlag == TRUE)
            {
                //in case of starting, if the state is starting, it is error
                if( omx_private->pVideoDecodInstance[pStartInfo->nDevID].bVideoStarted == OMX_TRUE )
                    return OMX_ErrorBadParameter;

            }
            else//if(pStartInfo->bStartFlag == FALSE)
            {
                //in case of stopping, if the state is not starting, it is error
				omx_private->pVideoDecodInstance[pStartInfo->nDevID].stRingBuffState.lInputCount = 0;
				omx_private->pVideoDecodInstance[pStartInfo->nDevID].stRingBuffState.lRollbackCheckCount = 0;
				omx_private->pVideoDecodInstance[pStartInfo->nDevID].stRingBuffState.llQueuedStartTime = -1;
				omx_private->pVideoDecodInstance[pStartInfo->nDevID].stRingBuffState.lFrameSuccessCount = 0;
				omx_private->pVideoDecodInstance[pStartInfo->nDevID].stRingBuffState.lNewFrameRate = 0;
                if( omx_private->pVideoDecodInstance[pStartInfo->nDevID].bVideoStarted != OMX_TRUE )
                    return OMX_ErrorBadParameter;

            }

			omx_private->pVideoDecodInstance[pStartInfo->nDevID].stVideoStart.nDevID = pStartInfo->nDevID;
			omx_private->pVideoDecodInstance[pStartInfo->nDevID].stVideoStart.nFormat = pStartInfo->nVideoFormat;
			if(pStartInfo->bStartFlag)
			{
				omx_private->pVideoDecodInstance[pStartInfo->nDevID].stVideoStart.nState = TCC_DXBVIDEO_OMX_Dec_Start;

				omx_private->stPreVideoInfo.iDecod_Instance = -1;
				omx_private->stPreVideoInfo.iDecod_status = -1;
				omx_private->stPreVideoInfo.uDecod_time = 0;
			}
			else
			{
				omx_private->pVideoDecodInstance[pStartInfo->nDevID].stVideoStart.nState = TCC_DXBVIDEO_OMX_Dec_Stop;
			}
			ring_CheckVideoStart(openmaxStandComp, pStartInfo->nDevID);
			if(pStartInfo->bStartFlag == TRUE)
    			for_iptv_info_init(openmaxStandComp);
		}
		break;

	case OMX_IndexParamVideoSetPause:
		{
			OMX_VIDEO_PARAM_PAUSETYPE *pPauseInfo;
			pPauseInfo = (OMX_VIDEO_PARAM_PAUSETYPE*) ComponentParameterStructure;
			omx_private->pVideoDecodInstance[omx_private->iDisplayID].bVideoPaused = pPauseInfo->bPauseFlag;				
		}
		break;

	case OMX_IndexVendorParamSetVideoPause:
		{
			OMX_U32	ulDemuxId;
			OMX_S32	 *piArg;
			piArg = (OMX_S32 *) ComponentParameterStructure;
			ulDemuxId = (OMX_U32) piArg[0];
			if (piArg[1] == 1) {
				tcc_vsync_command(omx_private, TCC_LCDC_VIDEO_SKIP_FRAME_START, NULL);
			} else {
				tcc_vsync_command(omx_private, TCC_LCDC_VIDEO_SKIP_FRAME_END, NULL);
			}
			omx_private->pVideoDecodInstance[ulDemuxId].gsTSPtsInfo.m_iLatestPTS = 0;
			omx_private->pVideoDecodInstance[ulDemuxId].bVideoPaused = (OMX_BOOL) piArg[1];
		}
		break;

	case OMX_IndexVendorParamIframeSearchEnable:
		{
#ifdef	SUPPORT_SEARCH_IFRAME_ATSTARTING
			pthread_mutex_lock(&omx_private->pVideoDecodInstance[omx_private->iDisplayID].stVideoStart.mutex);

			omx_private->pVideoDecodInstance[omx_private->iDisplayID].gsVDecInput.m_iSkipFrameNum = 1;
			if(omx_private->pVideoDecodInstance[omx_private->iDisplayID].gsVDecInit.m_iBitstreamFormat == STD_MPEG2)
			{
				omx_private->pVideoDecodInstance[omx_private->iDisplayID].gsVDecInput.m_iFrameSearchEnable = 0x001;
			}
			else
			{
	//			omx_private->pVideoDecodInstance[omx_private->iDisplayID].gsVDecInput.m_iFrameSearchEnable = 0x001;	//I-frame (IDR-picture for H.264)
				omx_private->pVideoDecodInstance[omx_private->iDisplayID].gsVDecInput.m_iFrameSearchEnable = 0x201;	//I-frame (I-slice for H.264) : Non IDR-picture
			}
			omx_private->pVideoDecodInstance[omx_private->iDisplayID].gsVDecInput.m_iSkipFrameMode = VDEC_SKIP_FRAME_EXCEPT_I;

			pthread_mutex_unlock(&omx_private->pVideoDecodInstance[omx_private->iDisplayID].stVideoStart.mutex);
#endif
		}
		break;	

	case OMX_IndexVendorParamVideoDisplayOutputCh:
		{
			OMX_S8 *ulDemuxId = (OMX_S8 *)ComponentParameterStructure;
			int	iDisplayID = omx_private->iDisplayID;

			if (iDisplayID < 2) pthread_mutex_lock(&omx_private->pVideoDecodInstance[iDisplayID].stVideoStart.mutex);

			if(omx_private->iDisplayID != *ulDemuxId) {
				bChangeDisplayID = OMX_TRUE;
				{
				}
			}
			omx_private->iDisplayID = *ulDemuxId;

			if (iDisplayID < 2) pthread_mutex_unlock(&omx_private->pVideoDecodInstance[iDisplayID].stVideoStart.mutex);
		}
		break;

	case OMX_IndexVendorParamVideoSurfaceState:
		{
			OMX_S32	 *piArg = (OMX_S32 *) ComponentParameterStructure;
			isRenderer = (OMX_BOOL) (*piArg);
		}
		break;

	case OMX_IndexVendorParamPlayStopRequest:
		{
			OMX_U32 *ulDemuxId = (OMX_S32 *) ComponentParameterStructure;
			omx_private->pVideoDecodInstance[*ulDemuxId].stVideoStart.nState = TCC_DXBVIDEO_OMX_Dec_Stop;
			ring_CheckVideoStart(openmaxStandComp, *ulDemuxId);
		}
		break;

	case OMX_IndexVendorParamPlayStartRequest:
		{
			OMX_U32 *ulDemuxId = (OMX_S32 *) ComponentParameterStructure;
			omx_private->pVideoDecodInstance[*ulDemuxId].stVideoStart.nState = TCC_DXBVIDEO_OMX_Dec_Start;
			ring_CheckVideoStart(openmaxStandComp, *ulDemuxId);
		}
		break;

	case OMX_IndexVendorParamVideoIsSupportCountry:
		{
			OMX_U32 ulDevId, ulCountry;
			OMX_S32	 *piArg;
			piArg = (OMX_S32 *) ComponentParameterStructure;
			ulDevId = (OMX_U32) piArg[0];
			omx_private->stPreVideoInfo.iDecod_Support_Country = (OMX_U32) piArg[1];

			ALOGE("%s %d iDecod_Support_Country = %d  \n", __func__, __LINE__, omx_private->stPreVideoInfo.iDecod_Support_Country);

		}
		break;

	case OMX_IndexVendorParamSeekToRequest:
		{
			OMX_U32 ulDevId, *pulHandle;
			OMX_U32 bStop;
			OMX_S32	 *piArg;
			piArg = (OMX_S32 *) ComponentParameterStructure;
			ulDevId = (OMX_U32) piArg[0];
			bStop = (OMX_U32) piArg[1];
			if (bStop == 1)
				omx_private->pVideoDecodInstance[ulDevId].stVideoStart.nState = TCC_DXBVIDEO_OMX_Dec_Stop;
			else
				omx_private->pVideoDecodInstance[ulDevId].stVideoStart.nState = TCC_DXBVIDEO_OMX_Dec_Start;

			ring_CheckVideoStart(openmaxStandComp, ulDevId);
		}
		break;

	case OMX_IndexVendorParamDxBActiveModeSet:
		{
			OMX_S32	 *ActiveMode;
			OMX_S32	 *piArg;
			OMX_VIDEO_PARAM_STARTTYPE *pStartInfo;
			pStartInfo = (OMX_VIDEO_PARAM_STARTTYPE*) ComponentParameterStructure;

			piArg = (OMX_S32 *) ComponentParameterStructure;
			ActiveMode = (OMX_U32) piArg[1];

			pthread_mutex_lock(&omx_private->pVideoDecodInstance[pStartInfo->nDevID].stVideoStart.mutex);

			omx_private->stIPTVActiveModeInfo.IPTV_PLAYMode =1;
			omx_private->stIPTVActiveModeInfo.IPTV_Old_Activemode = omx_private->stIPTVActiveModeInfo.IPTV_Activemode;
			omx_private->stIPTVActiveModeInfo.IPTV_Activemode = ActiveMode;
			omx_private->stIPTVActiveModeInfo.IPTV_Mode_change_cnt =0;

			omx_private->stbframe_skipinfo.bframe_skipcnt = 0;
			{
				int i;
				for (i=0; i<32; i++)
					omx_private->stbframe_skipinfo.bframe_skipindex[i] = 0x100;
			}
			pthread_mutex_unlock(&omx_private->pVideoDecodInstance[pStartInfo->nDevID].stVideoStart.mutex);			
		}
		break;

	case OMX_IndexVendorParamVideoSupportFieldDecoding:
		{
			OMX_S32	 *piArg = (OMX_S32 *) ComponentParameterStructure;
			omx_private->stVideoSubFunFlag.SupportFieldDecoding = (OMX_BOOL) (*piArg);
		}
		break;

	case OMX_IndexVendorParamVideoSupportIFrameSearch:
		{
			OMX_S32	 *piArg = (OMX_S32 *) ComponentParameterStructure;
			omx_private->stVideoSubFunFlag.SupportIFrameSearchMode = (OMX_BOOL) (*piArg);
		}
		break;

	case OMX_IndexVendorParamVideoSupportUsingErrorMB:
		{
			OMX_S32	 *piArg = (OMX_S32 *) ComponentParameterStructure;
			omx_private->stVideoSubFunFlag.SupprotUsingErrorMB = (OMX_BOOL) (*piArg);
		}
		break;

	case OMX_IndexVendorParamVideoSupportDirectDisplay:
		{
			OMX_S32	 *piArg = (OMX_S32 *) ComponentParameterStructure;
			omx_private->stVideoSubFunFlag.SupprotDirectDsiplay = (OMX_BOOL) (*piArg);
		}
		break;

	case OMX_IndexVendorParamDxBGetSTCFunction:
		{
			gfnDemuxGetSTCValue = (pfnDemuxGetSTC)ComponentParameterStructure;
		}
		break;
#if 0
	case OMX_IndexVendorParamPlaySpeed: 
	{
		UINT32 ulDemuxId, *pulHandle;
		INT32 iPlaySpeed;
		piArg = (INT32 *) ComponentParameterStructure;
		ulDemuxId = (UINT32) piArg[0];
		iPlaySpeed = (INT32) piArg[1];
	
		if (gPlayMode != 0) {
			if (gPlayMode == 1 && iPlaySpeed != 1) ;//LinuxTV_Pause();
			if (gPlayMode != 1 && iPlaySpeed == 1) ;//LinuxTV_Resume();
			gPlayMode = iPlaySpeed;
		}
	}
	break;
#endif
	default:	/*Call the base component function */
		return omx_base_component_SetParameter (hComponent, nParamIndex, ComponentParameterStructure);
	}
	return eError;
}

OMX_ERRORTYPE omx_ring_videodec_component_GetParameter (OMX_IN OMX_HANDLETYPE hComponent,
												   OMX_IN OMX_INDEXTYPE nParamIndex,
												   OMX_INOUT OMX_PTR ComponentParameterStructure)
{

	omx_base_video_PortType *port;
	OMX_ERRORTYPE eError = OMX_ErrorNone;

	OMX_COMPONENTTYPE *openmaxStandComp = hComponent;
	omx_videodec_component_PrivateType *omx_private = openmaxStandComp->pComponentPrivate;
	if (ComponentParameterStructure == NULL)
	{
		return OMX_ErrorBadParameter;
	}
	DBUG_MSG ("   Getting parameter %i\n", nParamIndex);
	/* Check which structure we are being fed and fill its header */
	switch (nParamIndex)
	{	
	case OMX_IndexParamStandardComponentRole:
		{
			OMX_PARAM_COMPONENTROLETYPE *pComponentRole;
			pComponentRole = ComponentParameterStructure;
			if ((eError =
				 checkHeader (ComponentParameterStructure, sizeof (OMX_PARAM_COMPONENTROLETYPE))) != OMX_ErrorNone)
			{
				break;
			}
			
			if (omx_private->pVideoDecodInstance[omx_private->iDisplayID].video_coding_type == OMX_VIDEO_CodingAVC)
			{
				strcpy ((char *) pComponentRole->cRole, VIDEO_DEC_TCC_H264_ROLE);
			}			
			else if (omx_private->pVideoDecodInstance[omx_private->iDisplayID].video_coding_type == OMX_VIDEO_CodingMPEG2)
			{
				strcpy ((char *) pComponentRole->cRole, VIDEO_DEC_MPEG2_ROLE);
			}
			else
			{
				strcpy ((char *) pComponentRole->cRole, "\0");
			}
			break;
		}

	case OMX_IndexParamInterlaceInfo:
		{
			int *p_iInterlaceInfo = (int *)ComponentParameterStructure;

			*p_iInterlaceInfo = omx_private->iInterlaceInfo;
		}
		break;

	case OMX_IndexVendorParamDxBGetVideoInfo:
		{
			int 	ulHandle;
			int	*piArg;
			int	*width, *height;

			piArg = (int *) ComponentParameterStructure;
			ulHandle = (int) piArg[0];
			width = (int *)piArg[1];
			height = (int *)piArg[2];

			if(omx_private->pVideoDecodInstance[omx_private->iDisplayID].gsVDecOutput.m_pInitialInfo != NULL)
			{
				if(width != NULL && omx_private->pVideoDecodInstance[omx_private->iDisplayID].gsVDecOutput.m_pInitialInfo->m_iPicWidth)
					*width = omx_private->pVideoDecodInstance[omx_private->iDisplayID].gsVDecOutput.m_pInitialInfo->m_iPicWidth;
				if(height != NULL && omx_private->pVideoDecodInstance[omx_private->iDisplayID].gsVDecOutput.m_pInitialInfo->m_iPicHeight)
					*height = omx_private->pVideoDecodInstance[omx_private->iDisplayID].gsVDecOutput.m_pInitialInfo->m_iPicHeight;
			}
			else
			{
				if(width != NULL)
					*width = 0;
				if(height != NULL)
					*height = 0;
			}				
		}
		break;

	default:	/*Call the base component function */
		return omx_base_component_GetParameter (hComponent, nParamIndex, ComponentParameterStructure);
	}
	return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_ring_videodec_component_MessageHandler (OMX_COMPONENTTYPE * openmaxStandComp,
													 internalRequestMessageType * message)
{
	omx_videodec_component_PrivateType *omx_private =
		(omx_videodec_component_PrivateType *) openmaxStandComp->pComponentPrivate;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_STATETYPE eCurrentState = omx_private->state;
	int i;

	DBUG_MSG ("In %s\n", __func__);

	if (message->messageType == OMX_CommandStateSet)
	{
		if ((message->messageParam == OMX_StateExecuting) && (omx_private->state == OMX_StateIdle))
		{
			for(i=0; i<iTotalHandle; i++)
			{
				if (!omx_private->pVideoDecodInstance[i].avcodecReady)
				{
					memset(&omx_private->pVideoDecodInstance[i].stVideoStart,0x0, sizeof(VideoStartInfo));
					pthread_mutex_init(&omx_private->pVideoDecodInstance[i].stVideoStart.mutex, NULL);
					err = omx_ring_videodec_component_LibInit (omx_private, i);
				}
				omx_private->pVideoDecodInstance[i].avcodecReady = OMX_TRUE;
			}

			if (err != OMX_ErrorNone)
			{
				return OMX_ErrorNotReady;
			}
		}
		else if ((message->messageParam == OMX_StateIdle) && (omx_private->state == OMX_StateLoaded))
		{
			err = omx_ring_videodec_component_Initialize (openmaxStandComp);
			if (err != OMX_ErrorNone)
			{
				ALOGE ("In %s Video Decoder Init Failed Error=%x\n", __func__, err);
				return err;
			}
		}
		else if ((message->messageParam == OMX_StateLoaded) && (omx_private->state == OMX_StateIdle))
		{
			err = omx_ring_videodec_component_Deinit (openmaxStandComp);
			if (err != OMX_ErrorNone)
			{
				ALOGE ("In %s Video Decoder Deinit Failed Error=%x\n", __func__, err);
				return err;
			}
		}
	}
	// Execute the base message handling
	err = omx_base_component_MessageHandler (openmaxStandComp, message);

	if (message->messageType == OMX_CommandStateSet)
	{
		if ((message->messageParam == OMX_StateIdle)
			&& (eCurrentState == OMX_StateExecuting || eCurrentState == OMX_StatePause))
		{
			for(i=0; i<iTotalHandle; i++)
			{
				if (omx_private->pVideoDecodInstance[i].avcodecReady)
				{
					omx_private->pVideoDecodInstance[i].avcodecReady = OMX_FALSE;
					omx_ring_videodec_component_LibDeinit (omx_private, i);
					pthread_mutex_destroy(&omx_private->pVideoDecodInstance[i].stVideoStart.mutex);
				}
				DeinitInputQueue(&omx_private->pVideoDecodInstance[i].stRingBuffState.stInputInfoQueue);
			}
		}
	}
	return err;
}

OMX_ERRORTYPE omx_ring_videodec_component_ComponentRoleEnum (OMX_IN OMX_HANDLETYPE hComponent,
														OMX_OUT OMX_U8 * cRole, OMX_IN OMX_U32 nIndex)
{

	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *) hComponent;
	omx_videodec_component_PrivateType *omx_private =
		(omx_videodec_component_PrivateType *) openmaxStandComp->pComponentPrivate;

	if (nIndex == 0)
	{
		if (omx_private->pVideoDecodInstance[omx_private->iDisplayID].video_coding_type == OMX_VIDEO_CodingAVC)
		{
			strcpy ((char *) cRole, VIDEO_DEC_TCC_H264_ROLE);
		}
		else if (omx_private->pVideoDecodInstance[omx_private->iDisplayID].video_coding_type == OMX_VIDEO_CodingMPEG2)
		{
			strcpy ((char *) cRole, VIDEO_DEC_MPEG2_ROLE);
		}	
	}
	else
	{
		return OMX_ErrorUnsupportedIndex;
	}

	return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_ring_videodec_component_SetConfig (OMX_HANDLETYPE hComponent,
												OMX_INDEXTYPE nIndex, OMX_PTR pComponentConfigStructure)
{

	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_VENDOR_EXTRADATATYPE *pExtradata;

	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *) hComponent;
	omx_videodec_component_PrivateType *omx_private =
		(omx_videodec_component_PrivateType *) openmaxStandComp->pComponentPrivate;

	if (pComponentConfigStructure == NULL)
	{
		return OMX_ErrorBadParameter;
	}
	DBUG_MSG ("   Getting configuration %i\n", nIndex);
	/* Check which structure we are being fed and fill its header */
	switch (nIndex)
	{	
	default:	// delegate to superclass
		return omx_base_component_SetConfig (hComponent, nIndex, pComponentConfigStructure);
	}
	return err;
}

OMX_ERRORTYPE omx_ring_videodec_component_GetExtensionIndex (OMX_IN OMX_HANDLETYPE hComponent,
														OMX_IN OMX_STRING cParameterName,
														OMX_OUT OMX_INDEXTYPE * pIndexType)
{

	DBUG_MSG ("In  %s \n", __func__);	
	return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_ring_VideoDec_Default_ComponentInit(OMX_COMPONENTTYPE *openmaxStandComp)
{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	err = omx_ring_videodec_component_Constructor(openmaxStandComp,VIDEO_DEC_BASE_NAME);
	return err;
}


void* omx_ring_video_twoport_filter_component_BufferMgmtFunction (void* param)
{
  OMX_COMPONENTTYPE* openmaxStandComp = (OMX_COMPONENTTYPE*)param;
  omx_videodec_component_PrivateType* omx_base_component_Private=(omx_base_component_PrivateType*)openmaxStandComp->pComponentPrivate;
  omx_base_filter_PrivateType* omx_base_filter_Private = (omx_base_filter_PrivateType*)omx_base_component_Private;
  omx_base_PortType *pInPort[2];
  omx_base_PortType *pOutPort[2];
  tsem_t* pInputSem[2];
  tsem_t* pOutputSem[2];
  queue_t* pInputQueue[2];
  queue_t* pOutputQueue[2];
  OMX_BUFFERHEADERTYPE* pOutputBuffer[2];
  OMX_BUFFERHEADERTYPE* pInputBuffer[2];
  OMX_BOOL isInputBufferNeeded[2],isOutputBufferNeeded[2];
  int inBufExchanged[2],outBufExchanged[2];
  int i, reqCallback[2];

  DEBUG(DEB_LEV_FULL_SEQ, "In %s\n", __func__);

  for (i=0; i < 2; i++) {
	pInPort[i] = (omx_base_PortType *)omx_base_filter_Private->ports[i];
	pInputSem[i] = pInPort[i]->pBufferSem;
	pInputQueue[i]= pInPort[i]->pBufferQueue;
	isInputBufferNeeded[i] = OMX_TRUE;
	inBufExchanged[i] = 0;
	pInputBuffer[i] = NULL;
  }
  for (i=0; i < 2; i++) {
	pOutPort[i] = (omx_base_PortType *)omx_base_filter_Private->ports[2+i];
	pOutputSem[i] = pOutPort[i]->pBufferSem;
	pOutputQueue[i]= pOutPort[i]->pBufferQueue;
	isOutputBufferNeeded[i] = OMX_TRUE;
	outBufExchanged[i] = 0;
	pOutputBuffer[i] = NULL;
  }

  reqCallback[0] = reqCallback[1] = OMX_FALSE;

  while(omx_base_filter_Private->state == OMX_StateIdle || omx_base_filter_Private->state == OMX_StateExecuting ||  omx_base_filter_Private->state == OMX_StatePause || 
    omx_base_filter_Private->transientState == OMX_TransStateLoadedToIdle) {

    /*Wait till the ports are being flushed*/
    pthread_mutex_lock(&omx_base_filter_Private->flush_mutex);
    while( PORT_IS_BEING_FLUSHED(pInPort[0]) || PORT_IS_BEING_FLUSHED(pInPort[1]) || PORT_IS_BEING_FLUSHED(pOutPort[0]) || PORT_IS_BEING_FLUSHED(pOutPort[1]) ) {
      pthread_mutex_unlock(&omx_base_filter_Private->flush_mutex);

      DEBUG(DEB_LEV_FULL_SEQ, "In %s 1 signalling flush all cond iE=%d,%d,iF=%d,%d,oE=%d,%d,oF=%d,%d iSemVal=%d,%d,oSemval=%d,%d\n",
        __func__,inBufExchanged[0],inBufExchanged[1],isInputBufferNeeded[0],isInputBufferNeeded[1],outBufExchanged[0],outBufExchanged[1],
        isOutputBufferNeeded[0],isOutputBufferNeeded[1],pInputSem[0]->semval,pInputSem[1]->semval,pOutputSem[0]->semval, pOutputSem[1]->semval);

      for (i=0; i < 2; i++) {
        if(isOutputBufferNeeded[i]==OMX_FALSE && PORT_IS_BEING_FLUSHED(pOutPort[i])) {
          pOutPort[i]->ReturnBufferFunction(pOutPort[i],pOutputBuffer[i]);
          outBufExchanged[i]--;
          pOutputBuffer[i]=NULL;
          isOutputBufferNeeded[i]=OMX_TRUE;
          DEBUG(DEB_LEV_FULL_SEQ, "OutPorts are flushing,so returning output buffer\n");
        }
      }

      for (i=0; i < 2; i++) {
        if(isInputBufferNeeded[i]==OMX_FALSE && PORT_IS_BEING_FLUSHED(pInPort[i])) {
          pInPort[i]->ReturnBufferFunction(pInPort[i],pInputBuffer[i]);
          inBufExchanged[i]--;
          pInputBuffer[i]=NULL;
          isInputBufferNeeded[i]=OMX_TRUE;
          DEBUG(DEB_LEV_FULL_SEQ, "InPorts[%d] are flushing,so returning input buffer\n", i);
        }
      }

      DEBUG(DEB_LEV_FULL_SEQ, "In %s 2 signalling flush all cond iE=%d,%d,iF=%d,%d,oE=%d,%d,oF=%d,%d iSemVal=%d,%d,oSemval=%d,%d\n",
        __func__,inBufExchanged[0],inBufExchanged[1],isInputBufferNeeded[0],isInputBufferNeeded[1],outBufExchanged[0],outBufExchanged[1],
        isOutputBufferNeeded[0],isOutputBufferNeeded[1],pInputSem[0]->semval,pInputSem[1]->semval,pOutputSem[0]->semval, pOutputSem[1]->semval);

      tsem_up(omx_base_filter_Private->flush_all_condition);
      tsem_down(omx_base_filter_Private->flush_condition);
      pthread_mutex_lock(&omx_base_filter_Private->flush_mutex);
    }
    pthread_mutex_unlock(&omx_base_filter_Private->flush_mutex);

    /*No buffer to process. So wait here*/
    if((isInputBufferNeeded[0]==OMX_TRUE && pInputSem[0]->semval==0) && (isInputBufferNeeded[1]==OMX_TRUE && pInputSem[1]->semval==0) &&
      (omx_base_filter_Private->state != OMX_StateLoaded && omx_base_filter_Private->state != OMX_StateInvalid)) {
      //Signalled from EmptyThisBuffer or FillThisBuffer or some thing else
      DEBUG(DEB_LEV_FULL_SEQ, "Waiting for next input/output buffer\n");
      tsem_down(omx_base_filter_Private->bMgmtSem);
    }
    if(omx_base_filter_Private->state == OMX_StateLoaded || omx_base_filter_Private->state == OMX_StateInvalid) {
      DEBUG(DEB_LEV_FULL_SEQ, "In %s Buffer Management Thread is exiting\n",__func__);
      break;
    }

	if((isOutputBufferNeeded[0]==OMX_TRUE && pOutputSem[0]->semval==0) && (isOutputBufferNeeded[1]==OMX_TRUE && pOutputSem[1]->semval==0) &&
		(omx_base_filter_Private->state != OMX_StateLoaded && omx_base_filter_Private->state != OMX_StateInvalid) &&
		!(PORT_IS_BEING_FLUSHED(pInPort[0]) || PORT_IS_BEING_FLUSHED(pInPort[1]) || PORT_IS_BEING_FLUSHED(pOutPort[0]) || PORT_IS_BEING_FLUSHED(pOutPort[1]))) {
			//Signalled from EmptyThisBuffer or FillThisBuffer or some thing else
			DEBUG(DEB_LEV_FULL_SEQ, "Waiting for next input/output buffer\n");
			tsem_down(omx_base_filter_Private->bMgmtSem);
	}
	if(omx_base_filter_Private->state == OMX_StateLoaded || omx_base_filter_Private->state == OMX_StateInvalid) {
		DEBUG(DEB_LEV_FULL_SEQ, "In %s Buffer Management Thread is exiting\n",__func__);
		break;
	}

	DEBUG(DEB_LEV_FULL_SEQ, "Waiting for input buffer semval=%d,%d \n",pInputSem[0]->semval,pInputSem[1]->semval);
	for (i=0; i < 2; i++) {
		if ((pInputSem[i]->semval > 0 && isInputBufferNeeded[i]==OMX_TRUE) && pOutputSem[i]->semval >0 && isOutputBufferNeeded[i]==OMX_TRUE) {
			reqCallback[i] = OMX_TRUE;
		}
	}

	if (reqCallback[0]==OMX_TRUE || reqCallback[1]==OMX_TRUE) {
		for (i=0; i < 2; i++) {
			if (reqCallback[i]==OMX_TRUE) {
				if (pInputSem[i]->semval > 0 && isInputBufferNeeded[i]==OMX_TRUE) {
					tsem_down(pInputSem[i]);
					if(pInputQueue[i]->nelem > 0) {
						inBufExchanged[i]++;
						isInputBufferNeeded[i]=OMX_FALSE;
						pInputBuffer[i] = dequeue(pInputQueue[i]);
						if (pInputBuffer[i]==NULL) {
							DEBUG(DEB_LEV_FULL_SEQ, "Had NULL input buffer!!\n");
							goto	omx_error_exit;
						}
						if(checkHeader(pInputBuffer[i], sizeof(OMX_BUFFERHEADERTYPE)))
						{
							DEBUG(DEB_LEV_FULL_SEQ, "In %s, crashed input buffer!!\n", __func__);
							isInputBufferNeeded[i] = OMX_TRUE;
							inBufExchanged[i]--;
							pInputBuffer[i] = NULL;
						}
					}
				}

			    /*When we have input buffer to process then get one output buffer*/
				if (pOutputSem[i]->semval > 0 && isOutputBufferNeeded[i]==OMX_TRUE) {
					tsem_down (pOutputSem[i]);
					if (pOutputQueue[i]->nelem > 0) {
						outBufExchanged[i]++;
						isOutputBufferNeeded[i]=OMX_FALSE;
						pOutputBuffer[i] = dequeue(pOutputQueue[i]);
						if(pOutputBuffer[i] == NULL) {
							DEBUG(DEB_LEV_FULL_SEQ, "Had NULL output buffer!! op is=%d,iq=%d\n",pOutputSem[i]->semval,pOutputQueue[i]->nelem);
							goto omx_error_exit;
						}
						if(checkHeader(pOutputBuffer[i], sizeof(OMX_BUFFERHEADERTYPE)))
						{
							DEBUG(DEB_LEV_FULL_SEQ, "crashed output buffer!!\n");
							isOutputBufferNeeded[i]=OMX_TRUE;
							outBufExchanged[i]--;
							pOutputBuffer[i] = NULL;
						}
					}
				}

				if(isInputBufferNeeded[i]==OMX_FALSE) {
					if(pInputBuffer[i]->hMarkTargetComponent != NULL){
						if((OMX_COMPONENTTYPE*)pInputBuffer[i]->hMarkTargetComponent ==(OMX_COMPONENTTYPE *)openmaxStandComp) {
							/*Clear the mark and generate an event*/
							(*(omx_base_filter_Private->callbacks->EventHandler))
								(openmaxStandComp,
								omx_base_filter_Private->callbackData,
								OMX_EventMark, /* The command was completed */
								1, /* The commands was a OMX_CommandStateSet */
								0, /* The state has been changed in message->messageParam2 */
								pInputBuffer[i]->pMarkData);
						} else {
							/*If this is not the target component then pass the mark*/
							omx_base_filter_Private->pMark.hMarkTargetComponent = pInputBuffer[i]->hMarkTargetComponent;
							omx_base_filter_Private->pMark.pMarkData			  = pInputBuffer[i]->pMarkData;
						}
						pInputBuffer[i]->hMarkTargetComponent = NULL;
					}
				}

				if(isInputBufferNeeded[i]==OMX_FALSE && isOutputBufferNeeded[i]==OMX_FALSE) {
					if(omx_base_filter_Private->pMark.hMarkTargetComponent != NULL){
						pOutputBuffer[i]->hMarkTargetComponent = omx_base_filter_Private->pMark.hMarkTargetComponent;
						pOutputBuffer[i]->pMarkData			= omx_base_filter_Private->pMark.pMarkData;
						omx_base_filter_Private->pMark.hMarkTargetComponent = NULL;
						omx_base_filter_Private->pMark.pMarkData			= NULL;
					}

					pOutputBuffer[i]->nTimeStamp = pInputBuffer[i]->nTimeStamp;

					if (omx_base_filter_Private->BufferMgmtCallback && pInputBuffer[i]->nFilledLen > 0) {
						if(omx_base_filter_Private->state == OMX_StateExecuting) {
							(*(omx_base_filter_Private->BufferMgmtCallback))(openmaxStandComp, pInputBuffer[i], pOutputBuffer[i]);			  
						}
						if(pInputBuffer[i]->nFlags == OMX_BUFFERFLAG_SEEKONLY) {
							pOutputBuffer[i]->nFlags = pInputBuffer[i]->nFlags;
							pInputBuffer[i]->nFlags = 0;
						}

						if(pInputBuffer[i]->nFlags == OMX_BUFFERFLAG_SYNCFRAME) {
							pOutputBuffer[i]->nFlags = pInputBuffer[i]->nFlags;
							pInputBuffer[i]->nFlags = 0;
						}
					} else {
					/*It no buffer management call back the explicitly consume input buffer*/
						pInputBuffer[i]->nFilledLen = 0;
					}

					if(pInputBuffer[i]->nFlags == OMX_BUFFERFLAG_STARTTIME) {
						DEBUG(DEB_LEV_FULL_SEQ, "Detected	START TIME flag in the input[%d] buffer filled len=%d\n", i, (int)pInputBuffer[i]->nFilledLen);
						pOutputBuffer[i]->nFlags = pInputBuffer[i]->nFlags;
						pInputBuffer[i]->nFlags = 0;
					}

					if(((pInputBuffer[i]->nFlags & OMX_BUFFERFLAG_EOS) == 1) && pInputBuffer[i]->nFilledLen==0 ) {
						pOutputBuffer[i]->nFlags=pInputBuffer[i]->nFlags;
						pInputBuffer[i]->nFlags=0;
						(*(omx_base_filter_Private->callbacks->EventHandler))
							(openmaxStandComp,
							omx_base_filter_Private->callbackData,
							OMX_EventBufferFlag, /* The command was completed */
							1, /* The commands was a OMX_CommandStateSet */
							pOutputBuffer[i]->nFlags, /* The state has been changed in message->messageParam2 */
							NULL);
						omx_base_filter_Private->bIsEOSReached = OMX_TRUE;
					}

					if(PORT_IS_BEING_CHANGED(pOutPort[i])) {
						ALOGD("PORT_IS_BEING_CHANGED before !! ");
						tsem_wait(omx_base_component_Private->bPortChangeSem);
						ALOGD("PORT_IS_BEING_CHANGED after !!  ");
					}

					if(omx_base_filter_Private->state==OMX_StatePause &&
						!(PORT_IS_BEING_FLUSHED(pInPort[0]) || PORT_IS_BEING_FLUSHED(pInPort[1]) || PORT_IS_BEING_FLUSHED(pOutPort[0]) || PORT_IS_BEING_FLUSHED(pOutPort[1]))
					) {
						/*Waiting at paused state*/
						ALOGE("In %s, component state is paused\n", __func__);
						tsem_wait(omx_base_component_Private->bStateSem);
					}

					/*If EOS and Input buffer Filled Len Zero then Return output buffer immediately*/
					if((pOutputBuffer[i]->nFilledLen != 0) || ((pOutputBuffer[i]->nFlags & OMX_BUFFERFLAG_EOS) == 1) || (omx_base_filter_Private->bIsEOSReached == OMX_TRUE)) {
						pOutPort[i]->ReturnBufferFunction(pOutPort[i],pOutputBuffer[i]);
						outBufExchanged[i]--;
						pOutputBuffer[i]=NULL;
						isOutputBufferNeeded[i]=OMX_TRUE;
					}

					if(omx_base_filter_Private->state==OMX_StatePause && 
						!(PORT_IS_BEING_FLUSHED(pInPort[0]) || PORT_IS_BEING_FLUSHED(pInPort[1]) || PORT_IS_BEING_FLUSHED(pOutPort[0]) || PORT_IS_BEING_FLUSHED(pOutPort[1]))
					) {
						/*Waiting at paused state*/
						ALOGE("In %s, component state is paused\n", __func__);
						tsem_wait(omx_base_component_Private->bStateSem);
					}

					/*Input Buffer has been completely consumed. So, return input buffer*/
					if((isInputBufferNeeded[i]== OMX_FALSE) && (pInputBuffer[i]->nFilledLen==0)) {
						pInPort[i]->ReturnBufferFunction(pInPort[i],pInputBuffer[i]);
						inBufExchanged[i]--;
						pInputBuffer[i]=NULL;
						isInputBufferNeeded[i]=OMX_TRUE;
					}

					if (isInputBufferNeeded[i]==OMX_TRUE && isOutputBufferNeeded[i]==OMX_TRUE)
						reqCallback[i]=OMX_FALSE;
				}
			}
			else {	//if(reqCallback[i]==OMX_TRUE)
				continue;
			}
		} //for (i=0; i < 2; i++)
	}
	else {
		//tsem_down(omx_base_filter_Private->bMgmtSem);
		usleep(5000);
	}
  }

omx_error_exit:
  DEBUG(DEB_LEV_SIMPLE_SEQ,"Exiting Buffer Management Thread\n");
  return NULL;
}


