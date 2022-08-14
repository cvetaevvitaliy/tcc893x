/**
  OpenMAX FBDEV sink component. This component is a video sink that copies
  data to a Linux framebuffer device.

  Originally developed by Peter Littlefield
  Copyright (C) 2007-2008  STMicroelectronics and Agere Systems
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

//#define LOG_NDEBUG	0
#define LOG_TAG	"OMX_FBDevSink"
#include <utils/Log.h>

#include <cutils/properties.h>

#include <errno.h>
#include <sys/poll.h>
#include <omxcore.h>
#include <omx_fbdev_sink_component.h>
#include "tcc_dxb_interface_video.h"

#ifdef HAVE_ANDROID_OS
#include <linux/videodev2.h>
#include <mach/tcc_scaler_ioctrl.h>
#include <mach/tcc_grp_ioctrl.h>
#include <mach/tcc_cam_ioctrl.h>		// DxB_capture
#include <mach/tcc_jpeg_ioctl.h>
#include <mach/tccfb_ioctrl.h>
#include <libpmap/pmap.h>

#include <linux/fb.h>
#include <mach/tcc_composite_ioctl.h>
#endif

#include <tcc_video_common.h>
#include <OMX_TCC_Index.h>
#include "tcc_dxb_interface_demux.h"
#include "tcc_dxb_surface.h"

static TCCDxBSurface *mTCCDxBSurface = NULL;

//#define	SUPPORT_SAVE_OUTPUT //Save output to files
#ifdef	SUPPORT_SAVE_OUTPUT
static int giDumpFileIndex = 0;
#endif

static OMX_BOOL bfbsinkbypass = OMX_FALSE, bfSkipRendering = OMX_FALSE;

typedef long long			(*pfnDemuxGetSTC)(int itype, long long lpts, unsigned int index, int log);
static pfnDemuxGetSTC gfnDemuxGetSTCValue = NULL;

/*
giDisplayPhyAddr information table
----------------------------------------------------------------------
0 : y addr
1 : u addr
2 : v addr
3 : field info (top, bottom)
4 : vsync unique id
5 : pts
6 : stc
7 : is non-interlace frame? (0: interlace, 1:non-interlace)
8 : frame width
9 : framw height
10 : vsync enable
11 : unique_addr
12 : frame rate
13 : display index
14 : VPU instance index
*/
static DISP_INFO gDisplayInfo;
static unsigned int guiSTCDelay = 0; //usec
static unsigned int gAudioDelay = 0; //usec

static OMX_BOOL	STC_Delay_Change=OMX_FALSE;

/** we assume, frame rate = 25 fps ; so one frame processing time = 40000 us */
/** we assume, frame rate = 30 fps ; so one frame processing time = 33333 us */
static OMX_U32 nFrameProcessTime = 33333;				   // in micro second
/** Counter of sink component instance*/
static OMX_U32 nofbdev_sinkInstance = 0;
/** Maximum number of sink component instances */
#define MAX_COMPONENT_FBDEVSINK 2
#define DxB_MAX_SINK_WIDTH 	1920//320
#define DxB_MAX_SINK_HEIGHT 1088//240
#define FBDEVSINK_CHECK_COUNT_VIDEO_OUTPUT_MAX			4
#define FBDEVSINK_COUNT_VIDEO_OUTPUT_FOR_AUDIO_START	1

typedef struct
{
	int	region_id;
	int x;
	int y;
	int width;
	int height;
	int *data;
	
} TCC_DVB_DEC_SUBTITLE_t, *pTCC_DVB_DEC_SUBTITLE_t;

pthread_mutex_t		lockLCDUpdate;
void omx_fbdev_sink_jpeg_enc_notify(OMX_COMPONENTTYPE * openmaxStandComp, int nResult);
void* omx_fbdev_sink_BufferMgmtFunction (void* param);

struct _IsdbtProprietaryData {
	int	channel_index;
	int dummy;
};
struct _IsdbtProprietaryData stIsdbtProprietaryData;

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
#endif

	return bVsyncSupport;
}

OMX_ERRORTYPE omx_fbdev_sink_init_video_surface(int arg)
{
	mTCCDxBSurface = new TCCDxBSurface(arg);
	return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_fbdev_sink_deinit_video_surface()
{
	if (mTCCDxBSurface != NULL)
	{
		delete mTCCDxBSurface;
		mTCCDxBSurface = NULL;
	}
	return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_fbdev_sink_set_video_surface(void *nativeWindow)
{
	if (mTCCDxBSurface != NULL)
	{
		mTCCDxBSurface->SetVideoSurface((ANativeWindow*)nativeWindow);
	}
	return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_fbdev_sink_no_clockcomp_component_Constructor (OMX_COMPONENTTYPE * openmaxStandComp, OMX_STRING cComponentName)
{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	omx_fbdev_sink_component_PortType *pPort;
	omx_fbdev_sink_component_PrivateType *omx_fbdev_sink_component_Private;
	unsigned int uiVideoPhyBase = 0, uiVideoPhySize = 0, uiBufferMinCount;
	int i;

	if (!openmaxStandComp->pComponentPrivate)
	{
		DEBUG (DEB_LEV_FUNCTION_NAME, "In %s, allocating component\n", __func__);
		openmaxStandComp->pComponentPrivate = TCC_calloc (1, sizeof (omx_fbdev_sink_component_PrivateType));
		if (openmaxStandComp->pComponentPrivate == NULL)
		{
			return OMX_ErrorInsufficientResources;
		}
	}
	else
	{
		DEBUG (DEB_LEV_FUNCTION_NAME, "In %s, Error Component %x Already Allocated\n", __func__, (int) openmaxStandComp->pComponentPrivate);
	}

	omx_fbdev_sink_component_Private = (omx_fbdev_sink_component_PrivateType *)openmaxStandComp->pComponentPrivate;
	omx_fbdev_sink_component_Private->jpeg_enc_thread = NULL;
	omx_fbdev_sink_component_Private->ports = NULL;


  /** we could create our own port structures here
    * fixme maybe the base class could use a "port factory" function pointer?  
    */
	err = omx_base_sink_Constructor (openmaxStandComp, cComponentName);

	omx_fbdev_sink_component_Private->sPortTypesParam[OMX_PortDomainVideo].nStartPortNumber = 0;
	omx_fbdev_sink_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts = 2;


  /** Allocate Ports and call port constructor. */
	if (omx_fbdev_sink_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts && !omx_fbdev_sink_component_Private->ports)
	{
		omx_fbdev_sink_component_Private->ports =
			(omx_base_PortType**)TCC_calloc (omx_fbdev_sink_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts, sizeof (omx_base_PortType *));
		
		if (!omx_fbdev_sink_component_Private->ports) {
			return OMX_ErrorInsufficientResources;
		}
		for (i = 0; i < omx_fbdev_sink_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts; i++) {
			omx_fbdev_sink_component_Private->ports[i] = (omx_base_PortType*)TCC_calloc (1, sizeof (omx_fbdev_sink_component_PortType));
			if (!omx_fbdev_sink_component_Private->ports[i]) {
				return OMX_ErrorInsufficientResources;
			}
		}
	}

	base_video_port_Constructor (openmaxStandComp, &omx_fbdev_sink_component_Private->ports[0], 0, OMX_TRUE);
	base_video_port_Constructor (openmaxStandComp, &omx_fbdev_sink_component_Private->ports[1], 1, OMX_TRUE);

  /** Domain specific section for the allocated port. */
  	uiBufferMinCount = 6; //default

	static pmap_t pmap_video;
	pmap_get_info("video", &pmap_video);
	uiVideoPhySize = pmap_video.size;
	
	if(uiVideoPhySize)
  	{
		ALOGD("Video PHY Information :: 0x%X, 0x%X",uiVideoPhyBase, uiVideoPhySize);
		if(uiVideoPhySize < 54*1024*1024)
			uiBufferMinCount = 4;		
  	}	

	if(isVsyncSupport())
	{
		uiBufferMinCount = 2;
	}

	ALOGD("nBufferCountMin :: %d",uiBufferMinCount);

	for (i=0; i < 2;i++) {
		pPort = (omx_fbdev_sink_component_PortType *) omx_fbdev_sink_component_Private->ports[i];
		pPort->sPortParam.nBufferCountMin = uiBufferMinCount;
		pPort->sPortParam.nBufferCountActual = 20;
		pPort->sPortParam.nBufferSize = 1024;
		pPort->sPortParam.format.video.nFrameWidth = 0;	//DEFAULT_WIDTH;
		pPort->sPortParam.format.video.nFrameHeight = 0;	//DEFAULT_HEIGHT;
		pPort->sPortParam.format.video.nBitrate = 0;
		pPort->sPortParam.format.video.xFramerate = 30;
		pPort->sPortParam.format.video.eColorFormat = OMX_COLOR_FormatYUV420Planar;	//OMX_COLOR_Format24bitRGB888;

		//  Figure out stride, slice height, min buffer size
		pPort->sPortParam.format.video.nStride =
			fb_calcStride (pPort->sPortParam.format.video.nFrameWidth, pPort->sPortParam.format.video.eColorFormat);
		pPort->sPortParam.format.video.nSliceHeight = pPort->sPortParam.format.video.nFrameHeight;	//  No support for slices yet
		pPort->sPortParam.nBufferSize =
			(OMX_U32) abs (pPort->sPortParam.format.video.nStride) * pPort->sPortParam.format.video.nSliceHeight;

		pPort->sVideoParam.eColorFormat = OMX_COLOR_FormatYUV420Planar;	//OMX_COLOR_Format24bitRGB888;
		pPort->sVideoParam.xFramerate = 30;

		DEBUG (DEB_LEV_PARAMS, "In %s, bSize=%d stride=%d\n", __func__, (int) pPort->sPortParam.nBufferSize,
			(int) pPort->sPortParam.format.video.nStride);

		/** Set configs */
		setHeader (&pPort->omxConfigCrop, sizeof (OMX_CONFIG_RECTTYPE));
		pPort->omxConfigCrop.nPortIndex = i;
		pPort->omxConfigCrop.nLeft = pPort->omxConfigCrop.nTop = 0;
		pPort->omxConfigCrop.nWidth = pPort->omxConfigCrop.nHeight = 0;

		setHeader (&pPort->omxConfigRotate, sizeof (OMX_CONFIG_ROTATIONTYPE));
		pPort->omxConfigRotate.nPortIndex = i;
		pPort->omxConfigRotate.nRotation = 0;	//Default: No rotation (0 degrees)

		setHeader (&pPort->omxConfigMirror, sizeof (OMX_CONFIG_MIRRORTYPE));
		pPort->omxConfigMirror.nPortIndex = i;
		pPort->omxConfigMirror.eMirror = OMX_MirrorNone;	//Default: No mirroring

		setHeader (&pPort->omxConfigScale, sizeof (OMX_CONFIG_SCALEFACTORTYPE));
		pPort->omxConfigScale.nPortIndex = i;
		pPort->omxConfigScale.xWidth = pPort->omxConfigScale.xHeight = 0x10000;	//Default: No scaling (scale factor = 1)

		setHeader (&pPort->omxConfigOutputPosition, sizeof (OMX_CONFIG_POINTTYPE));
		pPort->omxConfigOutputPosition.nPortIndex = i;
		pPort->omxConfigOutputPosition.nX = pPort->omxConfigOutputPosition.nY = 0;	//Default: No shift in output position (0,0)
	}

  /** set the function pointers */
	omx_fbdev_sink_component_Private->destructor = omx_fbdev_sink_component_Destructor;
	omx_fbdev_sink_component_Private->BufferMgmtFunction = omx_fbdev_sink_BufferMgmtFunction;
	omx_fbdev_sink_component_Private->BufferMgmtCallback = omx_fbdev_sink_component_BufferMgmtCallback;
	openmaxStandComp->SetParameter = omx_fbdev_sink_component_SetParameter;
	openmaxStandComp->GetParameter = omx_fbdev_sink_component_GetParameter;
	openmaxStandComp->SetConfig = omx_fbdev_sink_component_SetConfig;
	openmaxStandComp->GetConfig = omx_fbdev_sink_component_GetConfig;
	omx_fbdev_sink_component_Private->messageHandler = omx_fbdev_sink_component_MessageHandler;

	omx_fbdev_sink_component_Private->displayflag[0] = 0;
	omx_fbdev_sink_component_Private->displayflag[1] = 0;
	
	omx_fbdev_sink_component_Private->output_ch_index = 0;
	
	bfbsinkbypass = OMX_FALSE;
	bfSkipRendering = OMX_FALSE;
	
	gfnDemuxGetSTCValue = NULL;
	guiSTCDelay = 0;
	gAudioDelay = 0;

	nofbdev_sinkInstance++;
	if (nofbdev_sinkInstance > MAX_COMPONENT_FBDEVSINK)
	{
		ALOGE( "Reached Max Instances %d\n", (int) nofbdev_sinkInstance);
		return OMX_ErrorInsufficientResources;
	}

	pthread_mutex_init(&lockLCDUpdate, NULL);

	stIsdbtProprietaryData.channel_index = -1;
	stIsdbtProprietaryData.dummy = 0;

#ifdef  SUPPORT_PVR
	omx_fbdev_sink_component_Private->nPVRFlags = 0;
#endif//SUPPORT_PVR

	return err;
}

OMX_ERRORTYPE omx_fbdev_sink_no_clockcomp_component_Init (OMX_COMPONENTTYPE * openmaxStandComp)
{
	return (omx_fbdev_sink_no_clockcomp_component_Constructor (openmaxStandComp, "OMX.tcc.fbdev.fbdev_sink"));
}


/** The Destructor 
 */
OMX_ERRORTYPE omx_fbdev_sink_component_Destructor (OMX_COMPONENTTYPE * openmaxStandComp)
{
	omx_fbdev_sink_component_PrivateType *omx_fbdev_sink_component_Private = (omx_fbdev_sink_component_PrivateType *)openmaxStandComp->pComponentPrivate;
	OMX_U32   i;

	if(omx_fbdev_sink_component_Private->jpeg_enc_thread)
		pthread_join(omx_fbdev_sink_component_Private->jpeg_enc_thread, NULL);

	/* frees port/s */
	if (omx_fbdev_sink_component_Private->ports)
	{
		for (i = 0; i < (omx_fbdev_sink_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts +
						 omx_fbdev_sink_component_Private->sPortTypesParam[OMX_PortDomainOther].nPorts); i++)
		{
			if (omx_fbdev_sink_component_Private->ports[i])
				omx_fbdev_sink_component_Private->ports[i]->PortDestructor (omx_fbdev_sink_component_Private->ports[i]);
		}
		TCC_free (omx_fbdev_sink_component_Private->ports);
		omx_fbdev_sink_component_Private->ports = NULL;
	}

	omx_base_sink_Destructor (openmaxStandComp);
	nofbdev_sinkInstance--;

	omx_fbdev_sink_component_Private->displayflag[0] = 0;
	omx_fbdev_sink_component_Private->displayflag[1] = 0;

	pthread_mutex_destroy(&lockLCDUpdate);

	stIsdbtProprietaryData.channel_index = -1;
	stIsdbtProprietaryData.dummy = 0;

	return OMX_ErrorNone;
}


/** The deinitialization function 
  * It deallocates the frame buffer memory, and closes frame buffer
  */
OMX_ERRORTYPE omx_fbdev_sink_component_Deinit (OMX_COMPONENTTYPE * openmaxStandComp)
{
	omx_fbdev_sink_component_PrivateType *omx_fbdev_sink_component_Private = (omx_fbdev_sink_component_PrivateType *)openmaxStandComp->pComponentPrivate;
	if( omx_fbdev_sink_component_Private->fb_fd > 0 )
		close (omx_fbdev_sink_component_Private->fb_fd);	
	omx_fbdev_sink_component_Private->fb_fd = -1;
	
	return OMX_ErrorNone;
}

/**  This function takes two inputs - 
  * @param width is the input picture width
  * @param omx_pxlfmt is the input openmax standard pixel format
  * It calculates the byte per pixel needed to display the picture with the input omx_pxlfmt
  * The output stride for display is thus omx_fbdev_sink_component_Private->product of input width and byte per pixel
  */
OMX_S32 fb_calcStride (OMX_U32 width, OMX_COLOR_FORMATTYPE omx_pxlfmt)
{
	OMX_U32   stride;
	OMX_U32   bpp;										   // bit per pixel

	switch (omx_pxlfmt)
	{
		case OMX_COLOR_FormatMonochrome:
			bpp = 1;
			break;
		case OMX_COLOR_FormatL2:
			bpp = 2;
		case OMX_COLOR_FormatL4:
			bpp = 4;
			break;
		case OMX_COLOR_FormatL8:
		case OMX_COLOR_Format8bitRGB332:
		case OMX_COLOR_FormatRawBayer8bit:
		case OMX_COLOR_FormatRawBayer8bitcompressed:
			bpp = 8;
			break;
		case OMX_COLOR_FormatRawBayer10bit:
			bpp = 10;
			break;
		case OMX_COLOR_FormatYUV411Planar:
		case OMX_COLOR_FormatYUV411PackedPlanar:
		case OMX_COLOR_Format12bitRGB444:
		case OMX_COLOR_FormatYUV420Planar:
		case OMX_COLOR_FormatYUV420PackedPlanar:
		case OMX_COLOR_FormatYUV420SemiPlanar:
		case OMX_COLOR_FormatYUV444Interleaved:
			bpp = 12;
			break;
		case OMX_COLOR_FormatL16:
		case OMX_COLOR_Format16bitARGB4444:
		case OMX_COLOR_Format16bitARGB1555:
		case OMX_COLOR_Format16bitRGB565:
		case OMX_COLOR_Format16bitBGR565:
		case OMX_COLOR_FormatYUV422Planar:
		case OMX_COLOR_FormatYUV422PackedPlanar:
		case OMX_COLOR_FormatYUV422SemiPlanar:
		case OMX_COLOR_FormatYCbYCr:
		case OMX_COLOR_FormatYCrYCb:
		case OMX_COLOR_FormatCbYCrY:
		case OMX_COLOR_FormatCrYCbY:
			bpp = 16;
			break;
		case OMX_COLOR_Format18bitRGB666:
		case OMX_COLOR_Format18bitARGB1665:
			bpp = 18;
			break;
		case OMX_COLOR_Format19bitARGB1666:
			bpp = 19;
			break;
		case OMX_COLOR_FormatL24:
		case OMX_COLOR_Format24bitRGB888:
		case OMX_COLOR_Format24bitBGR888:
		case OMX_COLOR_Format24bitARGB1887:
			bpp = 24;
			break;
		case OMX_COLOR_Format25bitARGB1888:
			bpp = 25;
			break;
		case OMX_COLOR_FormatL32:
		case OMX_COLOR_Format32bitBGRA8888:
		case OMX_COLOR_Format32bitARGB8888:
			bpp = 32;
			break;
		default:
			bpp = 0;
			break;
	}
	stride = (width * bpp) >> 3;	// division by 8 to get byte per pixel value
	return (OMX_S32) stride;
}

int fb_checksinkconfig(omx_fbdev_sink_component_PortType *pPort)
{
	unsigned int video_width, video_height;
	video_width = pPort->sPortParam.format.video.nFrameWidth;
	video_height = pPort->sPortParam.format.video.nFrameHeight;
	if(video_width == 0 || video_height == 0)
		return -1;
	
	if(video_width > DxB_MAX_SINK_WIDTH || video_height > DxB_MAX_SINK_HEIGHT)
		return -1;
	return 0;
}	

#ifdef  SUPPORT_PVR
static int CheckPVR(omx_fbdev_sink_component_PrivateType *omx_private, OMX_U32 ulInputBufferFlags)
{
	OMX_U32 iPVRState, iBufferState;

	iPVRState = (omx_private->nPVRFlags & PVR_FLAG_START) ? 1 : 0;
	iBufferState = (ulInputBufferFlags & OMX_BUFFERFLAG_FILE_PLAY) ? 1 : 0;
	if (iPVRState != iBufferState)
	{
		return 1; // skip
	}

	iPVRState = (omx_private->nPVRFlags & PVR_FLAG_TRICK) ? 1 : 0;
	iBufferState = (ulInputBufferFlags & OMX_BUFFERFLAG_TRICK_PLAY) ? 1 : 0;
	if (iPVRState != iBufferState)
	{
		return 1; // skip
	}

	if (ulInputBufferFlags & OMX_BUFFERFLAG_TRICK_PLAY)
	{
		return 3; // by-pass
	}

	if (omx_private->nPVRFlags & PVR_FLAG_PAUSE)
	{
		return 2; // pause
	}

	return 0; // OK
}
#endif//SUPPORT_PVR

/** buffer management callback function 
  * takes one input buffer and displays its contents 
  */
void omx_fbdev_sink_component_BufferMgmtCallback (OMX_COMPONENTTYPE * openmaxStandComp,
												  OMX_BUFFERHEADERTYPE * pInputBuffer)
{
	omx_fbdev_sink_component_PrivateType *omx_fbdev_sink_component_Private = (omx_fbdev_sink_component_PrivateType *)openmaxStandComp->pComponentPrivate;
	omx_fbdev_sink_component_PortType *pPort;

	OMX_U8   *input_src_ptr = (OMX_U8 *) (pInputBuffer->pBuffer);
	int iVideoWidth, iVideoHeight;
	int iFrameWidth, iFrameHeight;	
	unsigned int PA_Y, VA_Y;
	unsigned int PA_U, VA_U;
	unsigned int PA_V, VA_V;
	long long uiInputBufferTime;
	char *pa_addr, *va_addr;
	int send_size = 4, ret;
	unsigned int uiDecoderID;

#ifdef  SUPPORT_PVR
	ret = CheckPVR(omx_fbdev_sink_component_Private, pInputBuffer->nFlags);
	switch (ret)
	{
	case 1: // skip
		*((unsigned int*)pInputBuffer->pBuffer + 7) = -1; //Clear only this frame
		pInputBuffer->nFilledLen = 0;
		return;
	case 2: // pause
		usleep(1000);
		return;
	case 3: // by-pass
		pInputBuffer->nTimeStamp = 0;
		break;
	}
#endif//SUPPORT_PVR

	gDisplayInfo.format = 1;// when using MALI

	if (omx_fbdev_sink_component_Private->displayflag[pInputBuffer->nDecodeID]) {
		memcpy (&gDisplayInfo.unique_id, pInputBuffer->pBuffer + 24, 4); //Vsync Unique ID
		if(gDisplayInfo.unique_id == 0) {
			omx_fbdev_sink_component_Private->displayflag[pInputBuffer->nDecodeID] = 0;
		}
	}

    if(bfSkipRendering)
    {
   		ret = -1; //Clear only this frame
		memcpy (pInputBuffer->pBuffer + 28, &ret, 4);
        pInputBuffer->nFilledLen = 0;		
        return;	
    }

	if (omx_fbdev_sink_component_Private->output_ch_index == 0)
		pPort = (omx_fbdev_sink_component_PortType *) omx_fbdev_sink_component_Private->ports[0];
	else
		pPort = (omx_fbdev_sink_component_PortType *) omx_fbdev_sink_component_Private->ports[1];

   	if( fb_checksinkconfig(pPort) == -1)
	{
		//Not Ready fbdev
		DEBUG (DEFAULT_MESSAGES, "[FBSINK]Config NOT READY!!!\n");
		ret = -2; //Clear all frame
		memcpy (pInputBuffer->pBuffer + 28, &ret, 4);
		pInputBuffer->nFilledLen = 0;		
		return;
	}		

	if (omx_fbdev_sink_component_Private->output_ch_index != pInputBuffer->nDecodeID) {
		ret = -1;
		memcpy (pInputBuffer->pBuffer + 28, &ret, 4);
		pInputBuffer->nFilledLen = 0;
		return;
	}

	memcpy (&gDisplayInfo.vsync_enable, pInputBuffer->pBuffer + 48, 4); //Vsync enable
	memcpy (&gDisplayInfo.unique_addr, pInputBuffer->pBuffer + 52, 4); //unique_addr
//	memcpy (&gDisplayInfo.unique_id, pInputBuffer->pBuffer + 24, 4); //Vsync Unique ID

	gDisplayInfo.frame_rate = pPort->sPortParam.format.video.xFramerate; //frame rate
	memcpy (&gDisplayInfo.display_index, pInputBuffer->pBuffer + 56, 4);	//display index
	memcpy (&gDisplayInfo.vpu_index, pInputBuffer->pBuffer + 60, 4);	//VPU instance index

	if (pInputBuffer->nTimeStamp > 0 && gfnDemuxGetSTCValue && !bfbsinkbypass)
	{
	    uiInputBufferTime = pInputBuffer->nTimeStamp + guiSTCDelay;
	}
	else
	{
		uiInputBufferTime = 0;
	}

	if (uiInputBufferTime > 0)
	{
		long long llSTC, llPTS;
		/*In This case We don't use clock components.
		* At this moments We need to compare between STC(Sytem Time Clock) and PTS (Present Time Stamp)
		* STC : ms, PTS : us
		*/
		llPTS = uiInputBufferTime/1000;
		llPTS += gAudioDelay; //gAudioDelay is Android Audio system delay

		/* itype 0:Audio, 1:Video Other :: Get STC Value		
		* ret :: -1:drop frame, 0:display frame, 1:wait frame
		*/		
		llSTC = gfnDemuxGetSTCValue(DxB_SYNC_VIDEO, llPTS, pInputBuffer->nDecodeID, 1); //
		if(gDisplayInfo.vsync_enable == 1)
		{
			if(llSTC == DxB_SYNC_DROP)
			{
			    ret = -1; //Clear only this frame
				memcpy (pInputBuffer->pBuffer + 28, &ret, 4);
				pInputBuffer->nFilledLen = 0;
				return;
			}
            else if(llSTC == DxB_SYNC_BYPASS)
            {
                //display without checking pts in vsync mode
                uiInputBufferTime = 0;
            }
            else if(llSTC == DxB_SYNC_LONG_WAIT)
            {
   				usleep(5000); //Wait
				return;
            }

		}
		else
		{
			if(llSTC == DxB_SYNC_WAIT || llSTC == DxB_SYNC_LONG_WAIT)
			{				
				usleep(5000); //Wait
				return;
			}	
			else if(llSTC == DxB_SYNC_DROP)
			{
				pInputBuffer->nFilledLen = 0;
				return;
			}
		}
	}	

	//ALOGE("Current Buffer %d", pPort->pBufferSem->semval);
    memcpy (&iFrameWidth, pInputBuffer->pBuffer + 40, 4);
    memcpy (&iFrameHeight, pInputBuffer->pBuffer + 44, 4);

	iVideoWidth = pPort->sPortParam.format.video.nFrameWidth;
	iVideoHeight = pPort->sPortParam.format.video.nFrameHeight;
	if ((iVideoWidth != iFrameWidth) || (iVideoHeight != iFrameHeight)) {
	    ret = -1; //Clear only this frame
		memcpy (pInputBuffer->pBuffer + 28, &ret, 4);
		pInputBuffer->nFilledLen = 0;	
		ALOGE("%s : Clear this frame [%dx%d][%dx%d]!!!", "[FB]", iVideoWidth, iVideoHeight, iFrameWidth, iFrameHeight);
		return;
	}

	PA_Y = (*(input_src_ptr+3)<<24) | (*(input_src_ptr+2)<<16) | (*(input_src_ptr+1)<<8) | *(input_src_ptr) ; input_src_ptr+=4;
	PA_U = (*(input_src_ptr+3)<<24) | (*(input_src_ptr+2)<<16) | (*(input_src_ptr+1)<<8) | *(input_src_ptr) ; input_src_ptr+=4;
	PA_V = (*(input_src_ptr+3)<<24) | (*(input_src_ptr+2)<<16) | (*(input_src_ptr+1)<<8) | *(input_src_ptr) ; input_src_ptr+=4;

	VA_Y = (*(input_src_ptr+3)<<24) | (*(input_src_ptr+2)<<16) | (*(input_src_ptr+1)<<8) | *(input_src_ptr) ; input_src_ptr+=4;
	VA_U = (*(input_src_ptr+3)<<24) | (*(input_src_ptr+2)<<16) | (*(input_src_ptr+1)<<8) | *(input_src_ptr) ; input_src_ptr+=4;
	VA_V = (*(input_src_ptr+3)<<24) | (*(input_src_ptr+2)<<16) | (*(input_src_ptr+1)<<8) | *(input_src_ptr) ;

#ifdef	SUPPORT_SAVE_OUTPUT
{
	FILE *fp;
	char file_name[32];
	if( giDumpFileIndex < 20 )
	{
		sprintf(file_name, "/sdcard/in_display%d.raw", ++giDumpFileIndex);
		fp = fopen(file_name, "wb");
		if(fp)
		{
			ALOGE("dump file created : %s", file_name);
			fwrite((char *)VA_Y, 1, iVideoWidth*iVideoHeight, fp);
			fwrite((char *)VA_U, 1, iVideoWidth*iVideoHeight/4, fp);
			fwrite((char *)VA_V, 1, iVideoWidth*iVideoHeight/4, fp);
			fclose(fp);
			sync();
		}
	}	
}
#endif

	pthread_mutex_lock (&lockLCDUpdate);
	gDisplayInfo.frame_addr[0] = PA_Y;
	gDisplayInfo.frame_addr[1] = PA_U;
	gDisplayInfo.frame_addr[2] = PA_V;
	memcpy (&gDisplayInfo.field_info, pInputBuffer->pBuffer + 28, 4); //Field Information(Top, Bottom)

	gDisplayInfo.pts = uiInputBufferTime/1000;
	if(gDisplayInfo.vsync_enable == 1)
	{
		//memcpy (&gDisplayInfo.unique_id, pInputBuffer->pBuffer + 24, 4);
		gDisplayInfo.stc = gfnDemuxGetSTCValue(-1, NULL, pInputBuffer->nDecodeID, 1);
	}
    /* Send interlaced frame information
     * 0:interlaced frame, 1:Non-interlaced frame
     */
	memcpy (&gDisplayInfo.interlace, pInputBuffer->pBuffer + 32, 4);
	if(pPort->sPortParam.format.video.xFramerate <60)
	{
	    if(omx_fbdev_sink_component_Private->displayflag[pInputBuffer->nDecodeID] == 0)
	    {
	        /* We always inform interlaced frame information to renderer forcedly whether frame is
	         * progressive or not. if current frame is progressive, VIQE doesn't
	         * processing deinterlacing.therefore no qulity degrade doesn't
	         * happen. 
	         */
	        gDisplayInfo.interlace = 0;
	    }
	}

	if(STC_Delay_Change != OMX_TRUE)
		memcpy (&gDisplayInfo.unique_id, pInputBuffer->pBuffer + 24, 4); //Vsync Unique ID
	else
	{
		gDisplayInfo.unique_id =0;
		gDisplayInfo.interlace =0;
		STC_Delay_Change = OMX_FALSE;
	}


	//gDisplayInfo.interlace = 1;
	gDisplayInfo.frame_width = iFrameWidth;
	gDisplayInfo.frame_height = iFrameHeight;

	unsigned int uiPicType = 0;
	memcpy (&uiPicType, pInputBuffer->pBuffer + 36, 4);
	if(mTCCDxBSurface == NULL || mTCCDxBSurface->WriteFrameBuf(&gDisplayInfo) != 0)
	{
		ret = -2; //clear all frames
		memcpy (pInputBuffer->pBuffer + 28, &ret, 4);
		pthread_mutex_unlock (&lockLCDUpdate);
		pInputBuffer->nFilledLen = 0;
		return;
	}

	if ( omx_fbdev_sink_component_Private->displayflag[pInputBuffer->nDecodeID] < FBDEVSINK_CHECK_COUNT_VIDEO_OUTPUT_MAX )
	{
		if ( omx_fbdev_sink_component_Private->displayflag[pInputBuffer->nDecodeID] < FBDEVSINK_CHECK_COUNT_VIDEO_OUTPUT_MAX )
			++omx_fbdev_sink_component_Private->displayflag[pInputBuffer->nDecodeID];
		
		if ( omx_fbdev_sink_component_Private->displayflag[pInputBuffer->nDecodeID] == FBDEVSINK_COUNT_VIDEO_OUTPUT_FOR_AUDIO_START )
		{
			(*(omx_fbdev_sink_component_Private->callbacks->EventHandler)) (openmaxStandComp,
								omx_fbdev_sink_component_Private->callbackData,
								OMX_EventVendorNotifyStartTimeSyncWithVideo,
								0, 0, 0);
		}			

		if ( omx_fbdev_sink_component_Private->displayflag[pInputBuffer->nDecodeID] == 1 )
		{
		#if 0
		    int nCount;
		    if(gDisplayInfo.vsync_enable != 1)
		    {
			    for(nCount=0; nCount<2; nCount++)
			    {
					if(mTCCDxBSurface != NULL)
					{
						mTCCDxBSurface->WriteFrameBuf(&gDisplayInfo);
					}
			    }
		    }
		#endif
		    (*(omx_fbdev_sink_component_Private->callbacks->EventHandler)) (openmaxStandComp,
							    omx_fbdev_sink_component_Private->callbackData,
							    OMX_EventVendorBufferingStart,
							    stIsdbtProprietaryData.channel_index, 0, 0);
	    }

	}

	pthread_mutex_unlock (&lockLCDUpdate);
	pInputBuffer->nFilledLen = 0;
}

OMX_ERRORTYPE omx_fbdev_sink_component_SetConfig (OMX_IN OMX_HANDLETYPE hComponent,
												  OMX_IN OMX_INDEXTYPE nIndex, OMX_IN OMX_PTR pComponentConfigStructure)
{

	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_U32   portIndex;
	OMX_CONFIG_RECTTYPE *omxConfigCrop;
	OMX_CONFIG_ROTATIONTYPE *omxConfigRotate;
	OMX_CONFIG_MIRRORTYPE *omxConfigMirror;
	OMX_CONFIG_SCALEFACTORTYPE *omxConfigScale;
	OMX_CONFIG_POINTTYPE *omxConfigOutputPosition;
	OMX_PARAM_PORTDEFINITIONTYPE *pPortDef;

	/* Check which structure we are being fed and make control its header */
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *) hComponent;
	omx_fbdev_sink_component_PrivateType *omx_fbdev_sink_component_Private = (omx_fbdev_sink_component_PrivateType *)openmaxStandComp->pComponentPrivate;
	omx_fbdev_sink_component_PortType *pPort;

	if (pComponentConfigStructure == NULL)
	{
		return OMX_ErrorBadParameter;
	}

	DEBUG (DEB_LEV_SIMPLE_SEQ, "   Setting parameter %i\n", nIndex);

	switch (nIndex)
	{
		case OMX_IndexParamPortDefinition:
			pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *) pComponentConfigStructure;
			portIndex = pPortDef->nPortIndex;

			if (portIndex > (omx_fbdev_sink_component_Private->sPortTypesParam[OMX_PortDomainVideo].nStartPortNumber +
							 omx_fbdev_sink_component_Private->sPortTypesParam[OMX_PortDomainOther].nPorts))
			{
				return OMX_ErrorBadPortIndex;
			}
			if (portIndex < 2)
			{

				pPort = (omx_fbdev_sink_component_PortType *) omx_fbdev_sink_component_Private->ports[portIndex];

				pPort->sPortParam.nBufferCountActual = pPortDef->nBufferCountActual;
				//  Copy stuff from OMX_VIDEO_PORTDEFINITIONTYPE structure
				if (pPortDef->format.video.cMIMEType != NULL)
				{
					strcpy (pPort->sPortParam.format.video.cMIMEType, pPortDef->format.video.cMIMEType);
				}
				pPort->sPortParam.format.video.nFrameWidth = pPortDef->format.video.nFrameWidth;
				pPort->sPortParam.format.video.nFrameHeight = pPortDef->format.video.nFrameHeight;
				pPort->sPortParam.format.video.nBitrate = pPortDef->format.video.nBitrate;
				pPort->sPortParam.format.video.xFramerate = pPortDef->format.video.xFramerate;
				pPort->sPortParam.format.video.bFlagErrorConcealment = pPortDef->format.video.bFlagErrorConcealment;

				//  Figure out stride, slice height, min buffer size
				pPort->sPortParam.format.video.nStride =
					fb_calcStride (pPort->sPortParam.format.video.nFrameWidth, pPort->sVideoParam.eColorFormat);
				pPort->sPortParam.format.video.nSliceHeight = pPort->sPortParam.format.video.nFrameHeight;	//  No support for slices yet
				// Read-only field by spec

				pPort->sPortParam.nBufferSize =
					(OMX_U32) abs (pPort->sPortParam.format.video.nStride) * pPort->sPortParam.format.video.nSliceHeight;
				pPort->omxConfigCrop.nWidth = pPort->sPortParam.format.video.nFrameWidth;
				pPort->omxConfigCrop.nHeight = pPort->sPortParam.format.video.nFrameHeight;
			}			
			break;

		case OMX_IndexConfigCommonInputCrop:
			omxConfigCrop = (OMX_CONFIG_RECTTYPE *) pComponentConfigStructure;
			portIndex = omxConfigCrop->nPortIndex;
			if ((err = checkHeader (pComponentConfigStructure, sizeof (OMX_CONFIG_RECTTYPE))) != OMX_ErrorNone)
			{
				break;
			}
			if (portIndex < 2)
			{
				pPort = (omx_fbdev_sink_component_PortType *) omx_fbdev_sink_component_Private->ports[portIndex];
				pPort->omxConfigCrop.nLeft = omxConfigCrop->nLeft;
				pPort->omxConfigCrop.nTop = omxConfigCrop->nTop;
				pPort->omxConfigCrop.nWidth = omxConfigCrop->nWidth;
				pPort->omxConfigCrop.nHeight = omxConfigCrop->nHeight;
			}
			else
			{
				return OMX_ErrorBadPortIndex;
			}
			break;
		case OMX_IndexConfigCommonMirror:
			omxConfigMirror = (OMX_CONFIG_MIRRORTYPE *) pComponentConfigStructure;
			portIndex = omxConfigMirror->nPortIndex;
			if ((err = checkHeader (pComponentConfigStructure, sizeof (OMX_CONFIG_MIRRORTYPE))) != OMX_ErrorNone)
			{
				break;
			}
			if (portIndex < 2)
			{
				if (omxConfigMirror->eMirror == OMX_MirrorBoth || omxConfigMirror->eMirror == OMX_MirrorHorizontal)
				{
					//  Horizontal mirroring not yet supported
					return OMX_ErrorUnsupportedSetting;
				}
				pPort = (omx_fbdev_sink_component_PortType *) omx_fbdev_sink_component_Private->ports[portIndex];
				pPort->omxConfigMirror.eMirror = omxConfigMirror->eMirror;
			}
			else
			{
				return OMX_ErrorBadPortIndex;
			}
			break;
		case OMX_IndexConfigCommonScale:
			omxConfigScale = (OMX_CONFIG_SCALEFACTORTYPE *) pComponentConfigStructure;
			portIndex = omxConfigScale->nPortIndex;
			if ((err = checkHeader (pComponentConfigStructure, sizeof (OMX_CONFIG_SCALEFACTORTYPE))) != OMX_ErrorNone)
			{
				break;
			}
			if (portIndex < 2)
			{
				if (omxConfigScale->xWidth != 0x10000 || omxConfigScale->xHeight != 0x10000)
				{
					//  Scaling not yet supported
					return OMX_ErrorUnsupportedSetting;
				}
				pPort = (omx_fbdev_sink_component_PortType *) omx_fbdev_sink_component_Private->ports[portIndex];
				pPort->omxConfigScale.xWidth = omxConfigScale->xWidth;
				pPort->omxConfigScale.xHeight = omxConfigScale->xHeight;
			}
			else
			{
				return OMX_ErrorBadPortIndex;
			}
			break;
		case OMX_IndexConfigCommonOutputPosition:
			omxConfigOutputPosition = (OMX_CONFIG_POINTTYPE *) pComponentConfigStructure;
			portIndex = omxConfigOutputPosition->nPortIndex;
			if ((err = checkHeader (pComponentConfigStructure, sizeof (OMX_CONFIG_POINTTYPE))) != OMX_ErrorNone)
			{
				break;
			}
			if (portIndex < 2)
			{
				pPort = (omx_fbdev_sink_component_PortType *) omx_fbdev_sink_component_Private->ports[portIndex];
				pPort->omxConfigOutputPosition.nX = omxConfigOutputPosition->nX;
				pPort->omxConfigOutputPosition.nY = omxConfigOutputPosition->nY;
			}
			else
			{
				return OMX_ErrorBadPortIndex;
			}
			break;
		case OMX_IndexVendorChangeResolution:
			{
				OMX_RESOLUTIONTYPE *p_resolution_param = (OMX_RESOLUTIONTYPE *)pComponentConfigStructure;
				portIndex = p_resolution_param->nPortIndex;
				if ((err = checkHeader (pComponentConfigStructure, sizeof (OMX_RESOLUTIONTYPE))) != OMX_ErrorNone){
					break;
				}
			
				if (portIndex < 2) {
					if(portIndex == 0) pPort = (omx_fbdev_sink_component_PortType *)omx_fbdev_sink_component_Private->ports[OMX_BASE_SINK_INPUTPORT_INDEX];
					else pPort = (omx_fbdev_sink_component_PortType *)omx_fbdev_sink_component_Private->ports[OMX_BASE_SINK_INPUTPORT_INDEX+1];
				
					pPort->sPortParam.format.video.nFrameWidth = p_resolution_param->nWidth;
					pPort->sPortParam.format.video.nFrameHeight = p_resolution_param->nHeight;
					pPort->sPortParam.format.video.xFramerate = p_resolution_param->nFrameRate;
				} else {
					return OMX_ErrorBadPortIndex;
				}
			}
			break;	
		case OMX_IndexConfigLcdcUpdate:
			{
			    bfSkipRendering = OMX_TRUE;
				pthread_mutex_lock (&lockLCDUpdate);
				if(omx_fbdev_sink_component_Private->displayflag[omx_fbdev_sink_component_Private->output_ch_index])
				{
					//int nCount;
					//for(nCount=0; nCount<3; nCount++)
					{
						if(mTCCDxBSurface != NULL)
						{
							mTCCDxBSurface->WriteFrameBuf(&gDisplayInfo);
						}
					}
				}
				pthread_mutex_unlock (&lockLCDUpdate);
				bfSkipRendering = OMX_FALSE;
			}
			break;

		default:	// delegate to superclass
			return omx_base_component_SetConfig (hComponent, nIndex, pComponentConfigStructure);
	}
	return err;
}

OMX_ERRORTYPE omx_fbdev_sink_component_GetConfig (OMX_IN OMX_HANDLETYPE hComponent,
												  OMX_IN OMX_INDEXTYPE nIndex,
												  OMX_INOUT OMX_PTR pComponentConfigStructure)
{

	OMX_CONFIG_RECTTYPE *omxConfigCrop;
	OMX_CONFIG_ROTATIONTYPE *omxConfigRotate;
	OMX_CONFIG_MIRRORTYPE *omxConfigMirror;
	OMX_CONFIG_SCALEFACTORTYPE *omxConfigScale;
	OMX_CONFIG_POINTTYPE *omxConfigOutputPosition;

	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *) hComponent;
	omx_fbdev_sink_component_PrivateType *omx_fbdev_sink_component_Private = (omx_fbdev_sink_component_PrivateType *)openmaxStandComp->pComponentPrivate;
	omx_fbdev_sink_component_PortType *pPort;

	if (pComponentConfigStructure == NULL)
	{
		return OMX_ErrorBadParameter;
	}
	DEBUG (DEB_LEV_SIMPLE_SEQ, "   Getting configuration %i\n", nIndex);

	/* Check which structure we are being fed and fill its header */

	switch (nIndex)
	{
		case OMX_IndexConfigCommonInputCrop:
			omxConfigCrop = (OMX_CONFIG_RECTTYPE *) pComponentConfigStructure;
			setHeader (omxConfigCrop, sizeof (OMX_CONFIG_RECTTYPE));
			if (omxConfigCrop->nPortIndex < 2)
			{
				pPort =
					(omx_fbdev_sink_component_PortType *) omx_fbdev_sink_component_Private->ports[omxConfigCrop->nPortIndex];
				memcpy (omxConfigCrop, &pPort->omxConfigCrop, sizeof (OMX_CONFIG_RECTTYPE));
			}
			else
			{
				return OMX_ErrorBadPortIndex;
			}
			break;
		case OMX_IndexConfigCommonRotate:
			omxConfigRotate = (OMX_CONFIG_ROTATIONTYPE *) pComponentConfigStructure;
			setHeader (omxConfigRotate, sizeof (OMX_CONFIG_ROTATIONTYPE));
			if (omxConfigRotate->nPortIndex < 2)
			{
				pPort =
					(omx_fbdev_sink_component_PortType *) omx_fbdev_sink_component_Private->ports[omxConfigRotate->nPortIndex];
				memcpy (omxConfigRotate, &pPort->omxConfigRotate, sizeof (OMX_CONFIG_ROTATIONTYPE));
			}
			else
			{
				return OMX_ErrorBadPortIndex;
			}
			break;
		case OMX_IndexConfigCommonMirror:
			omxConfigMirror = (OMX_CONFIG_MIRRORTYPE *) pComponentConfigStructure;
			setHeader (omxConfigMirror, sizeof (OMX_CONFIG_MIRRORTYPE));
			if (omxConfigMirror->nPortIndex < 2)
			{
				pPort =
					(omx_fbdev_sink_component_PortType *) omx_fbdev_sink_component_Private->ports[omxConfigMirror->nPortIndex];
				memcpy (omxConfigMirror, &pPort->omxConfigMirror, sizeof (OMX_CONFIG_MIRRORTYPE));
			}
			else
			{
				return OMX_ErrorBadPortIndex;
			}
			break;
		case OMX_IndexConfigCommonScale:
			omxConfigScale = (OMX_CONFIG_SCALEFACTORTYPE *) pComponentConfigStructure;
			setHeader (omxConfigScale, sizeof (OMX_CONFIG_SCALEFACTORTYPE));
			if (omxConfigScale->nPortIndex < 2)
			{
				pPort =
					(omx_fbdev_sink_component_PortType *) omx_fbdev_sink_component_Private->ports[omxConfigScale->nPortIndex];
				memcpy (omxConfigScale, &pPort->omxConfigScale, sizeof (OMX_CONFIG_SCALEFACTORTYPE));
			}
			else
			{
				return OMX_ErrorBadPortIndex;
			}
			break;
		case OMX_IndexConfigCommonOutputPosition:
			omxConfigOutputPosition = (OMX_CONFIG_POINTTYPE *) pComponentConfigStructure;
			setHeader (omxConfigOutputPosition, sizeof (OMX_CONFIG_POINTTYPE));
			if (omxConfigOutputPosition->nPortIndex < 2)
			{
				pPort =
					(omx_fbdev_sink_component_PortType *) omx_fbdev_sink_component_Private->ports[omxConfigOutputPosition->nPortIndex];
				memcpy (omxConfigOutputPosition, &pPort->omxConfigOutputPosition, sizeof (OMX_CONFIG_POINTTYPE));
			}
			else
			{
				return OMX_ErrorBadPortIndex;
			}
			break;		
		default:	// delegate to superclass
			return omx_base_component_GetConfig (hComponent, nIndex, pComponentConfigStructure);
	}
	return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_fbdev_sink_component_SetParameter (OMX_IN OMX_HANDLETYPE hComponent,
													 OMX_IN OMX_INDEXTYPE nParamIndex,
													 OMX_IN OMX_PTR ComponentParameterStructure)
{
	OMX_S32   *piArg;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_PARAM_PORTDEFINITIONTYPE *pPortDef;
	OMX_VIDEO_PARAM_PORTFORMATTYPE *pVideoPortFormat;
	OMX_OTHER_PARAM_PORTFORMATTYPE *pOtherPortFormat;
	OMX_U32   portIndex;

	/* Check which structure we are being fed and make control its header */
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *) hComponent;
	omx_fbdev_sink_component_PrivateType *omx_fbdev_sink_component_Private = (omx_fbdev_sink_component_PrivateType *)openmaxStandComp->pComponentPrivate;
	omx_fbdev_sink_component_PortType *pPort;
	if (ComponentParameterStructure == NULL)
	{
		return OMX_ErrorBadParameter;
	}

	DEBUG (DEB_LEV_SIMPLE_SEQ, "   Setting parameter %i\n", nParamIndex);
	switch (nParamIndex)
	{
		case OMX_IndexParamPortDefinition:
			pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *) ComponentParameterStructure;
			portIndex = pPortDef->nPortIndex;
			err =
				omx_base_component_ParameterSanityCheck (hComponent, portIndex, pPortDef,
														 sizeof (OMX_PARAM_PORTDEFINITIONTYPE));
			if (err != OMX_ErrorNone)
			{
				ALOGE( "In %s Parameter Check Error=%x\n", __func__, err);
				break;
			}

			if (portIndex > (omx_fbdev_sink_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts +
							 omx_fbdev_sink_component_Private->sPortTypesParam[OMX_PortDomainOther].nPorts))
			{
				return OMX_ErrorBadPortIndex;
			}

			if (portIndex < 2)
			{

				pPort = (omx_fbdev_sink_component_PortType *) omx_fbdev_sink_component_Private->ports[portIndex];

				pPort->sPortParam.nBufferCountActual = pPortDef->nBufferCountActual;
				//  Copy stuff from OMX_VIDEO_PORTDEFINITIONTYPE structure
				if (pPortDef->format.video.cMIMEType != NULL)
				{
					strcpy (pPort->sPortParam.format.video.cMIMEType, pPortDef->format.video.cMIMEType);
				}
				pPort->sPortParam.format.video.nFrameWidth = pPortDef->format.video.nFrameWidth;
				pPort->sPortParam.format.video.nFrameHeight = pPortDef->format.video.nFrameHeight;
				pPort->sPortParam.format.video.nBitrate = pPortDef->format.video.nBitrate;
				pPort->sPortParam.format.video.xFramerate = pPortDef->format.video.xFramerate;
				pPort->sPortParam.format.video.bFlagErrorConcealment = pPortDef->format.video.bFlagErrorConcealment;

				//Figure out stride, slice height, min buffer size
				pPort->sPortParam.format.video.nStride =
					fb_calcStride (pPort->sPortParam.format.video.nFrameWidth, pPort->sVideoParam.eColorFormat);
				pPort->sPortParam.format.video.nSliceHeight = pPort->sPortParam.format.video.nFrameHeight;	//  No support for slices yet
				//Read-only field by spec

				pPort->sPortParam.nBufferSize =
					(OMX_U32) abs (pPort->sPortParam.format.video.nStride) * pPort->sPortParam.format.video.nSliceHeight;
				pPort->omxConfigCrop.nWidth = pPort->sPortParam.format.video.nFrameWidth;
				pPort->omxConfigCrop.nHeight = pPort->sPortParam.format.video.nFrameHeight;
			}			
			break;

		case OMX_IndexVendorParamDxBGetSTCFunction:
			gfnDemuxGetSTCValue = (pfnDemuxGetSTC) ComponentParameterStructure;
			omx_fbdev_sink_component_Private->displayflag[0] = 0;
			omx_fbdev_sink_component_Private->displayflag[1] = 0;
			break;

		case OMX_IndexVendorParamSetSinkByPass:
			{
				OMX_U32 ulDemuxId;
				piArg = (OMX_S32 *) ComponentParameterStructure;
				ulDemuxId = (OMX_U32) piArg[0];
				bfbsinkbypass = (OMX_BOOL) piArg[1];
			}
			break;

		case OMX_IndexVendorParamSetSubtitle:
			{
				TCC_DVB_DEC_SUBTITLE_t *piSubt = (TCC_DVB_DEC_SUBTITLE_t *)ComponentParameterStructure;
				(*(omx_fbdev_sink_component_Private->callbacks->EventHandler)) (openmaxStandComp,
										omx_fbdev_sink_component_Private->callbackData,
										OMX_EventVendorSubtitleEvent,
										0, 0, piSubt);
			}
			break;
			
		case OMX_IndexVendorParamVideoDisplayOutputCh:
			{
				piArg = (OMX_S32 *)ComponentParameterStructure;

				omx_fbdev_sink_component_Private->output_ch_index = piArg[0];
				ALOGE("[FBDEVSINK] Change Video Output to Ch[%d] \n", omx_fbdev_sink_component_Private->output_ch_index);
			}
			break;

		case OMX_IndexVendorSetSTCDelay:
			{
				guiSTCDelay = (unsigned int)ComponentParameterStructure;
				ALOGD("STC Delay %d (us)", guiSTCDelay);
				if(guiSTCDelay > 1000000)
					guiSTCDelay = 300000;

				STC_Delay_Change = OMX_TRUE;
			}
			break;

		case OMX_IndexVendorSetAudioDelay:
			{
				gAudioDelay = (unsigned int)ComponentParameterStructure;
				ALOGD("Audio Delay %d (ms)", gAudioDelay);
			}
			break;

		case OMX_IndexParamFBSetFirstFrame:
			{
				OMX_U32 ulDemuxId;
				piArg = (OMX_S32 *)ComponentParameterStructure;

				ulDemuxId = (OMX_U32) piArg[0];

				ALOGE("[FBDEVSINK] Set First Frame[%d] \n", ulDemuxId);
			}
			break;

		case OMX_IndexVendorParamVideoSurfaceState:
			{
				piArg = (OMX_S32 *)ComponentParameterStructure;
				if (mTCCDxBSurface != NULL)
				{
					if (*piArg)
						mTCCDxBSurface->UseSurface();
					else
						mTCCDxBSurface->ReleaseSurface();
				}
			}
			break;

		case OMX_IndexVendorParamISDBTProprietaryData:
			{
				piArg = (OMX_S32 *)ComponentParameterStructure;
				stIsdbtProprietaryData.channel_index = *piArg;
			}
			break;

#ifdef  SUPPORT_PVR
		case OMX_IndexVendorParamPlayStartRequest:
			{
				OMX_S32 ulPvrId;
				OMX_S8 *pucFileName;

				piArg = (OMX_S32 *) ComponentParameterStructure;
				ulPvrId = (OMX_S32) piArg[0];
				pucFileName = (OMX_S8 *) piArg[1];

				if ((omx_fbdev_sink_component_Private->nPVRFlags & PVR_FLAG_START) == 0)
				{
					omx_fbdev_sink_component_Private->nPVRFlags = PVR_FLAG_START | PVR_FLAG_CHANGED;
				}
			}
			break;

		case OMX_IndexVendorParamPlayStopRequest:
			{
				OMX_S32 ulPvrId;

				piArg = (OMX_S32 *) ComponentParameterStructure;
				ulPvrId = (OMX_S32) piArg[0];

				if (omx_fbdev_sink_component_Private->nPVRFlags & PVR_FLAG_START)
				{
					omx_fbdev_sink_component_Private->nPVRFlags = PVR_FLAG_CHANGED;
				}
			}
			break;

		case OMX_IndexVendorParamSeekToRequest:
			{
				OMX_S32 ulPvrId, nSeekTime;

				piArg = (OMX_S32 *) ComponentParameterStructure;
				ulPvrId = (OMX_S32) piArg[0];
				nSeekTime = (OMX_S32) piArg[1];

				if (omx_fbdev_sink_component_Private->nPVRFlags & PVR_FLAG_START)
				{
					if (nSeekTime < 0)
					{
						if (omx_fbdev_sink_component_Private->nPVRFlags & PVR_FLAG_TRICK)
						{
							omx_fbdev_sink_component_Private->nPVRFlags &= ~PVR_FLAG_TRICK;
							omx_fbdev_sink_component_Private->nPVRFlags |= PVR_FLAG_CHANGED;
						}
					}
					else
					{
						if ((omx_fbdev_sink_component_Private->nPVRFlags & PVR_FLAG_TRICK) == 0)
						{
							omx_fbdev_sink_component_Private->nPVRFlags |= (PVR_FLAG_TRICK | PVR_FLAG_CHANGED);
						}
					}
				}
			}
			break;

		case OMX_IndexVendorParamPlaySpeed:
			{
				OMX_S32 ulPvrId, nPlaySpeed;

				piArg = (OMX_S32 *) ComponentParameterStructure;
				ulPvrId = (OMX_S32) piArg[0];
				nPlaySpeed = (OMX_S32) piArg[1];

				if (omx_fbdev_sink_component_Private->nPVRFlags & PVR_FLAG_START)
				{
					if (nPlaySpeed == 1)
					{
						if (omx_fbdev_sink_component_Private->nPVRFlags & PVR_FLAG_TRICK)
						{
							omx_fbdev_sink_component_Private->nPVRFlags &= ~PVR_FLAG_TRICK;
							omx_fbdev_sink_component_Private->nPVRFlags |= PVR_FLAG_CHANGED;
						}
					}
					else
					{
						if ((omx_fbdev_sink_component_Private->nPVRFlags & PVR_FLAG_TRICK) == 0)
						{
							omx_fbdev_sink_component_Private->nPVRFlags |= (PVR_FLAG_TRICK | PVR_FLAG_CHANGED);
						}
					}
				}
			}
			break;

		case OMX_IndexVendorParamPlayPause:
			{
				OMX_S32 ulPvrId, nPlayPause;

				piArg = (OMX_S32 *) ComponentParameterStructure;
				ulPvrId = (OMX_S32) piArg[0];
				nPlayPause = (OMX_S32) piArg[1];

				if (omx_fbdev_sink_component_Private->nPVRFlags & PVR_FLAG_START)
				{
					if (nPlayPause == 0)
					{
						omx_fbdev_sink_component_Private->nPVRFlags |= PVR_FLAG_PAUSE;
					}
					else
					{
						omx_fbdev_sink_component_Private->nPVRFlags &= ~PVR_FLAG_PAUSE;
					}
				}
			}
			break;
#endif//SUPPORT_PVR

		default:	/*Call the base component function */
			return omx_base_component_SetParameter (hComponent, nParamIndex, ComponentParameterStructure);
	}
	return err;
}

OMX_ERRORTYPE omx_fbdev_sink_component_GetParameter (OMX_IN OMX_HANDLETYPE hComponent,
													 OMX_IN OMX_INDEXTYPE nParamIndex,
													 OMX_INOUT OMX_PTR ComponentParameterStructure)
{

	OMX_VIDEO_PARAM_PORTFORMATTYPE *pVideoPortFormat;
	OMX_OTHER_PARAM_PORTFORMATTYPE *pOtherPortFormat;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *) hComponent;
	omx_fbdev_sink_component_PrivateType *omx_fbdev_sink_component_Private = (omx_fbdev_sink_component_PrivateType *)openmaxStandComp->pComponentPrivate;
	if (ComponentParameterStructure == NULL)
	{
		return OMX_ErrorBadParameter;
	}
	DEBUG (DEB_LEV_SIMPLE_SEQ, "   Getting parameter %i\n", nParamIndex);
	/* Check which structure we are being fed and fill its header */
	switch (nParamIndex)
	{		
		case OMX_IndexVendorParamFBCapture:			// DxB_capture
			{
				pthread_mutex_lock (&lockLCDUpdate);
				{
					int ret_type;
					char                gJpegEncPath[256];
					strcpy(gJpegEncPath, (char *)ComponentParameterStructure);
					if (mTCCDxBSurface != NULL)
					{
						ret_type = mTCCDxBSurface->CaptureVideoFrame(gJpegEncPath);
					}
					omx_fbdev_sink_jpeg_enc_notify(openmaxStandComp, ret_type);
				}
				pthread_mutex_unlock (&lockLCDUpdate);
			}
			break;

		default:	/*Call the base component function */
			return omx_base_component_GetParameter (hComponent, nParamIndex, ComponentParameterStructure);
	}
	return err;
}

OMX_ERRORTYPE omx_fbdev_sink_component_MessageHandler (OMX_COMPONENTTYPE * openmaxStandComp,
													   internalRequestMessageType * message)
{

	omx_fbdev_sink_component_PrivateType *omx_fbdev_sink_component_Private =
		(omx_fbdev_sink_component_PrivateType *) openmaxStandComp->pComponentPrivate;
	OMX_ERRORTYPE err;
	OMX_STATETYPE eState;

	DEBUG (DEB_LEV_SIMPLE_SEQ, "In %s\n", __func__);
	eState = omx_fbdev_sink_component_Private->state;	//storing current state

	// Execute the base message handling
	err = omx_base_component_MessageHandler (openmaxStandComp, message);

	if (message->messageType == OMX_CommandStateSet)
	{
		if ((message->messageParam == OMX_StateIdle) && ((omx_fbdev_sink_component_Private->state == OMX_StateIdle	&& eState == OMX_StateExecuting) ||  eState == OMX_StatePause))
		{
			err = omx_fbdev_sink_component_Deinit (openmaxStandComp);
			if (err != OMX_ErrorNone)
			{
				ALOGE( "In %s Video Sink Deinit Failed Error=%x\n", __func__, err);
				return err;
			}
		}
	}
	return err;
}

void omx_fbdev_sink_jpeg_enc_notify(OMX_COMPONENTTYPE * openmaxStandComp, int nResult)
{
	omx_fbdev_sink_component_PrivateType *omx_fbdev_sink_component_Private = (omx_fbdev_sink_component_PrivateType *) openmaxStandComp->pComponentPrivate;
	(*(omx_fbdev_sink_component_Private->callbacks->EventHandler))
							(openmaxStandComp,
							omx_fbdev_sink_component_Private->callbackData,
							OMX_EventRecordingStop, /* The command was completed */
							0, /* The commands was a OMX_CommandStateSet */
							nResult, /* The state has been changed in message->messageParam2 */
							NULL);
}


void* omx_fbdev_sink_BufferMgmtFunction (void* param)
{
  OMX_COMPONENTTYPE* openmaxStandComp = (OMX_COMPONENTTYPE*)param;
  omx_base_component_PrivateType* omx_base_component_Private  = (omx_base_component_PrivateType*)openmaxStandComp->pComponentPrivate;
  omx_base_sink_PrivateType*      omx_base_sink_Private       = (omx_base_sink_PrivateType*)omx_base_component_Private;
  omx_base_PortType               *pInPort[2]; //                    = (omx_base_PortType *)omx_base_sink_Private->ports[OMX_BASE_SINK_INPUTPORT_INDEX];
  tsem_t*                         pInputSem[2]; //                   = pInPort->pBufferSem;
  queue_t*                        pInputQueue[2]; //                 = pInPort->pBufferQueue;
  OMX_BUFFERHEADERTYPE*           pInputBuffer[2]; //                = NULL;
  OMX_COMPONENTTYPE*              target_component;
  OMX_BOOL                        isInputBufferNeeded[2]; //         = OMX_TRUE;
  int                             inBufExchanged[2]; //              = 0;
  int i;

  DEBUG(DEB_LEV_FUNCTION_NAME, "In %s \n", __func__);

  for (i=0; i < 2; i++) {
	pInPort[i] = (omx_base_PortType *)omx_base_sink_Private->ports[i];
	pInputSem[i] = pInPort[i]->pBufferSem;
	pInputQueue[i] = pInPort[i]->pBufferQueue;
	pInputBuffer[i] = NULL;
	isInputBufferNeeded[i] = OMX_TRUE;
	inBufExchanged[i] = 0;
  }

  while(omx_base_component_Private->state == OMX_StateIdle || omx_base_component_Private->state == OMX_StateExecuting ||  omx_base_component_Private->state == OMX_StatePause || 
    omx_base_component_Private->transientState == OMX_TransStateLoadedToIdle){

    /*Wait till the ports are being flushed*/
    pthread_mutex_lock(&omx_base_sink_Private->flush_mutex);
    while( PORT_IS_BEING_FLUSHED(pInPort[0]) || PORT_IS_BEING_FLUSHED(pInPort[1])) {
      pthread_mutex_unlock(&omx_base_sink_Private->flush_mutex);

		for (i=0; i < 2; i++) {
			if(isInputBufferNeeded[i]==OMX_FALSE) {
				pInPort[i]->ReturnBufferFunction(pInPort[i],pInputBuffer[i]);
				inBufExchanged[i]--;
				pInputBuffer[i]=NULL;
				isInputBufferNeeded[i]=OMX_TRUE;
				DEBUG(DEB_LEV_FULL_SEQ, "Ports are flushing,so returning input buffer\n");
			}
		}
      DEBUG(DEB_LEV_FULL_SEQ, "In %s signalling flush all condition \n", __func__);

      tsem_up(omx_base_sink_Private->flush_all_condition);
      tsem_down(omx_base_sink_Private->flush_condition);
      pthread_mutex_lock(&omx_base_sink_Private->flush_mutex);
    }
    pthread_mutex_unlock(&omx_base_sink_Private->flush_mutex);

    /*No buffer to process. So wait here*/
    if((pInputSem[0]->semval==0 && isInputBufferNeeded[0]==OMX_TRUE ) && (pInputSem[1]->semval==0 && isInputBufferNeeded[1]==OMX_TRUE ) &&
      (omx_base_sink_Private->state != OMX_StateLoaded && omx_base_sink_Private->state != OMX_StateInvalid)) {
      DEBUG(DEB_LEV_SIMPLE_SEQ, "Waiting for input buffer \n");
      tsem_down(omx_base_sink_Private->bMgmtSem);
    }

    if(omx_base_sink_Private->state == OMX_StateLoaded || omx_base_sink_Private->state == OMX_StateInvalid) {
      DEBUG(DEB_LEV_FULL_SEQ, "In %s Buffer Management Thread is exiting\n",__func__);
      break;
    }

    DEBUG(DEB_LEV_SIMPLE_SEQ, "Waiting for input buffer semval=%d,%d \n",pInputSem[0]->semval, pInputSem[1]->semval);
	for (i=0; i < 2; i++) {
		if(pInputSem[i]->semval>0 && isInputBufferNeeded[i]==OMX_TRUE ) {
			tsem_down(pInputSem[i]);
			if(pInputQueue[i]->nelem>0){
				inBufExchanged[i]++;
				isInputBufferNeeded[i]=OMX_FALSE;
				pInputBuffer[i] = (OMX_BUFFERHEADERTYPE *)dequeue(pInputQueue[i]);
				if(pInputBuffer[i]== NULL){
					DEBUG(DEB_LEV_ERR, "Had NULL input buffer(%d)!!\n", i);
					goto omx_error_exit;
					//break;
				}
			}
		}
	}

	for (i=0; i < 2; i++) {
		if(isInputBufferNeeded[i]==OMX_FALSE) {
			if(pInputBuffer[i]->nFlags==OMX_BUFFERFLAG_EOS) {
				(*(omx_base_component_Private->callbacks->EventHandler))
					(openmaxStandComp,
					omx_base_component_Private->callbackData,
					OMX_EventBufferFlag, /* The command was completed */
					0, /* The commands was a OMX_CommandStateSet */
					pInputBuffer[i]->nFlags, /* The state has been changed in message->messageParam2 */
					NULL);
				pInputBuffer[i]->nFlags=0;
			}

			target_component=(OMX_COMPONENTTYPE*)pInputBuffer[i]->hMarkTargetComponent;
			if(target_component==(OMX_COMPONENTTYPE *)openmaxStandComp) {
				/*Clear the mark and generate an event*/
				(*(omx_base_component_Private->callbacks->EventHandler))
					(openmaxStandComp,
					omx_base_component_Private->callbackData,
					OMX_EventMark, /* The command was completed */
					1, /* The commands was a OMX_CommandStateSet */
					0, /* The state has been changed in message->messageParam2 */
					pInputBuffer[i]->pMarkData);
			} else if(pInputBuffer[i]->hMarkTargetComponent!=NULL){
				/*If this is not the target component then pass the mark*/
				DEBUG(DEB_LEV_FULL_SEQ, "Can't Pass Mark. This is a Sink!!\n");
			}

			if (omx_base_sink_Private->BufferMgmtCallback && pInputBuffer[i]->nFilledLen > 0) {
				if(omx_base_sink_Private->state == OMX_StateExecuting)
					(*(omx_base_sink_Private->BufferMgmtCallback))(openmaxStandComp, pInputBuffer[i]);
			}
			else {
				/*If no buffer management call back the explicitly consume input buffer*/
				pInputBuffer[i]->nFilledLen = 0;
			}
			/*Input Buffer has been completely consumed. So, get new input buffer*/

			if(omx_base_sink_Private->state==OMX_StatePause && !(PORT_IS_BEING_FLUSHED(pInPort[0]) || PORT_IS_BEING_FLUSHED(pInPort[1]))) {
				/*Waiting at paused state*/
				tsem_wait(omx_base_sink_Private->bStateSem);
			}

			/*Input Buffer has been completely consumed. So, return input buffer*/
			if(pInputBuffer[i]->nFilledLen==0) {
				pInPort[i]->ReturnBufferFunction(pInPort[i],pInputBuffer[i]);
				inBufExchanged[i]--;
				pInputBuffer[i]=NULL;
				isInputBufferNeeded[i] = OMX_TRUE;
			}
		}
	}
  }
omx_error_exit:
  DEBUG(DEB_LEV_SIMPLE_SEQ,"Exiting Buffer Management Thread\n");
  return NULL;
}

