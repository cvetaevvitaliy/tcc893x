/**
  OpenMAX FBDEV sink component. This component is a video sink that copies
  data to a >inux framebuffer device.

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

#ifndef _OMX_FBDEV_SINK_COMPONENT_H_
#define _OMX_FBDEV_SINK_COMPONENT_H_

#include <OMX_Types.h>
#include <OMX_Component.h>
#include <OMX_Core.h>
#include <OMX_Video.h>
#include <OMX_IVCommon.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#include <omx_base_video_port.h>
#include <omx_base_sink.h>
#include <linux/fb.h>
#include "TCCFile.h"
#include "TCCMemory.h"


typedef enum
{
    OMX_IndexConfigLcdcUpdate = 0x7E000001,
    OMX_IndexParamFBSetFirstFrame
}TC_DXBFBDEV_OMX_INDEXVENDORTYPE;



/**  Filename of devnode for framebuffer device
  *  Should somehow be passed from client
  */
//#define FBDEV_FILENAME  "/dev/fb0" 	// donghee_dxb_android_adap
#define FBDEV_FILENAME  "/dev/graphics/fb0" 

/* for display mode */
enum {	
	FBSINK_MODE_INIT = 0,
	FBSINK_MODE_STANDARD,
	FBSINK_MODE_FULL,
	FBSINK_MODE_ZOOM,
	FBSINK_MODE_LCD = 0x40000000,	
	FBSINK_MODE_TVOUT = 0x80000000,
	FBSINK_MODE_END = 0xFFFFFFFF
};


/* for scannig mode */
enum {
	FBSINK_PROGRESSIVE_SCAN = 1,
	FBSINK_INTERLACED_SCAN,
};

/* for dec output color format */
enum {
	DEC_OUT_YUV420INT = 0,
	DEC_OUT_YUV420SEP = 1,
	DEC_OUT_MAX
};

#ifdef __cplusplus
extern "C" {
#endif

/** FBDEV sink port component port structure.
  */
DERIVEDCLASS(omx_fbdev_sink_component_PortType, omx_base_video_PortType)
#define omx_fbdev_sink_component_PortType_FIELDS omx_base_video_PortType_FIELDS \
  /** @param omxConfigCrop Crop rectangle of image */ \
  OMX_CONFIG_RECTTYPE omxConfigCrop; \
  /** @param omxConfigRotate Set rotation angle of image */ \
  OMX_CONFIG_ROTATIONTYPE omxConfigRotate; \
  /** @param omxConfigMirror Set mirroring of image */ \
  OMX_CONFIG_MIRRORTYPE omxConfigMirror; \
  /** @param omxConfigScale Set scale factors */ \
  OMX_CONFIG_SCALEFACTORTYPE omxConfigScale; \
  /** @param omxConfigOutputPosition Top-Left offset from intermediate buffer to output buffer */ \
  OMX_CONFIG_POINTTYPE omxConfigOutputPosition;
ENDCLASS(omx_fbdev_sink_component_PortType)

/** FBDEV sink port component private structure.
  * see the define above
  * @param fd The file descriptor for the framebuffer 
  * @param vscr_info The fb_var_screeninfo structure for the framebuffer 
  * @param fscr_info The fb_fix_screeninfo structure for the framebuffer
  * @param scr_data Pointer to the mmapped memory for the framebuffer 
  * @param fbpxlfmt frame buffer pixel format
  * @param fbwidth frame buffer display width 
  * @param fbheight frame buffer display height 
  * @param fbbpp frame buffer pixel depth
  * @param fbstride frame buffer display stride 
  * @param xScale the scale of the media clock
  * @param eState the state of the media clock
  * @param product frame buffer memory area 
  * @param frameDropFlag the flag active on scale change indicates that frames are to be dropped 
  * @param dropFrameCount counts the number of frames dropped 
  */
DERIVEDCLASS(omx_fbdev_sink_component_PrivateType, omx_base_sink_PrivateType)
#define omx_fbdev_sink_component_PrivateType_FIELDS omx_base_sink_PrivateType_FIELDS \
  int                          displayflag[2]; \
  int                          overlay_fd; \
  int                          scaler_fd; \
  int                          graphic_fd; \
  int                          fb_fd; \
  int                          composite_fd; \
  struct                       fb_var_screeninfo vscr_info; \
  struct                       fb_fix_screeninfo fscr_info; \
  unsigned char                *scr_ptr; \
  OMX_COLOR_FORMATTYPE         fbpxlfmt; \
  OMX_U32                      fbwidth; \
  OMX_U32                      fbheight; \
  OMX_U32                      fbbpp; \
  OMX_S32                      fbstride; \
  OMX_S32                      xScale; \
  OMX_TIME_CLOCKSTATE          eState; \
  OMX_U32                      product;\
  OMX_BOOL                     frameDropFlag;\
  int                          dropFrameCount; \
  pthread_t                    jpeg_enc_thread; \
  int						   output_ch_index; \
  OMX_U32                      nPVRFlags;
ENDCLASS(omx_fbdev_sink_component_PrivateType)

/* Component private entry points declaration */
OMX_ERRORTYPE omx_fbdev_sink_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,OMX_STRING cComponentName);
OMX_ERRORTYPE omx_fbdev_sink_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_fbdev_sink_component_Initialize(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_fbdev_sink_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_fbdev_sink_component_MessageHandler(OMX_COMPONENTTYPE* , internalRequestMessageType*);
OMX_ERRORTYPE omx_fbdev_sink_no_clockcomp_component_Init (OMX_COMPONENTTYPE * openmaxStandComp);

// tcc_dxb_surface
OMX_ERRORTYPE omx_fbdev_sink_init_video_surface(int arg);
OMX_ERRORTYPE omx_fbdev_sink_deinit_video_surface();
OMX_ERRORTYPE omx_fbdev_sink_set_video_surface(void *nativeWindow);

void omx_fbdev_sink_component_BufferMgmtCallback(
  OMX_COMPONENTTYPE *openmaxStandComp,
  OMX_BUFFERHEADERTYPE* pInputBuffer);

OMX_ERRORTYPE omx_fbdev_sink_component_SetConfig(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nIndex,
  OMX_IN  OMX_PTR pComponentConfigStructure);

OMX_ERRORTYPE omx_fbdev_sink_component_GetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_INOUT OMX_PTR ComponentParameterStructure);

OMX_ERRORTYPE omx_fbdev_sink_component_SetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_IN  OMX_PTR ComponentParameterStructure);

OMX_ERRORTYPE omx_fbdev_sink_component_GetConfig(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nIndex,
  OMX_INOUT OMX_PTR pComponentConfigStructure);

/** finds video stride  from input dimension and color format */
OMX_S32 fb_calcStride(OMX_U32 width, OMX_COLOR_FORMATTYPE omx_pxlfmt);


#ifdef __cplusplus
}
#endif

#endif
