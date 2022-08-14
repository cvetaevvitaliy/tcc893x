/**
 * Copyright (C) 2012, Telechips Limited
 * by ZzaU(B070371)
 */

#ifndef RENDERER_IF_H_

#define RENDERER_IF_H_

#include <media/stagefright/MediaBuffer.h>
//#include <media/stagefright/MediaSource.h>
//#include <media/stagefright/OMXClient.h>
#include <gui/Surface.h>
#include <gui/IGraphicBufferProducer.h>

#include <utils/List.h>
#include <common_if.h>
#include <libpmap/pmap.h>

#include <mach/tcc_video_private.h>


using namespace android;

#define USE_WMIXER_FOR_COPY
#define USE_BUFFER_MANAGE_DIRECT

#define DEF_BUFFER_CNT 6

typedef enum RENDER_INDEXTYPE {
	RENDER_IndexConfigAsyncMode = 0,
	RENDER_IndexConfigCropInfo,
}RENDER_INDEXTYPE;

typedef struct RENDER_ASYNCMODETYPE {
	bool bAsync;
} RENDER_ASYNCMODETYPE;

typedef struct RENDER_CROPINFOTYPE {
	int32_t left;
	int32_t top;
	int32_t right;
	int32_t bottom;
} RENDER_CROPINFOTYPE;

class RendererIf {
public:
/*!
 *************************************************************************************************
 * \brief
 *		RendererIf/~RendererIf	: Constructor / Destructor of Renderer
 * \param 0
 *		[in] bufferProducer		: pointer of IGraphicBufferProducer
 *		[in] nativeWindow		: pointer of ANativeWindow
 *			=> The two argument(surfaceTexture, pNativeWindow) are mutually exclusive.
 *   			So, one of those arguments should be NULL.
 *		[in] inFrame_width		: width of input frame 
 *		[in] inFrame_height		: height of input frame
 *		[in] inBuffer_format	: format of input frame (raw data)
 *		[in] rotationDegrees    : rotation value of original video, (0, 90, 180, 270)
 *		[in] outbuffer_count    : buffer count allocated for NativeWindow
 *		[in] bAsync_mode		: Camera preview and videos are rate-limited on the producer side.
 *								  If enabled, we use async mode to always show the most recent frame at the cost of requiring an additional buffer.
 *************************************************************************************************
 */
	RendererIf(const sp<IGraphicBufferProducer> &bufferProducer, void *nativeWindow, unsigned int inFrame_width, unsigned int inFrame_height, tFRAME_BUF_FORMAT inBuffer_format, int rotationDegrees = 0, int outbuffer_count = DEF_BUFFER_CNT, bool bASync_mode = true );
    ~RendererIf();

/*!
 *************************************************************************************************
 * \brief
 *		Render	: Call this to display frame.
 * \param 0
 *		[in] Yaddr		: Y(Luminance) address of frame that will be displayed.
 *		[in] Uaddr		: U/UV(Chrominance) address of frame that will be displayed.
 *		[in] Vaddr		: V(Chrominance) address of frame that will be displayed.
 *		[in] bAddrPhy   : It indicates if Yaddr,Uaddr and Vaddr is physical address or not.
 *		[in] timestamp_ms	: Timestamd of current frame (unit is milliseconds.)
  *		[in] plat_priv	: if the frame data is from h/w decoder, it should be delivered.
 * \return
 *		If successful, Render() returns 0 or plus. Otherwise, it returns a minus value.
 *************************************************************************************************
 */
	status_t Render(unsigned int Yaddr, unsigned int Uaddr, unsigned int Vaddr, char bAddrPhy, int64_t timestamp_ms, TCC_PLATFORM_PRIVATE_PMEM_INFO *plat_priv = NULL);

/*!
 *************************************************************************************************
 * \brief
 *		SetConfig	: Call this to change some configuration related to SurfaceTexture.
 * \param 0
 *		[in] nIndex		: index.
 *		[in] pParam		: parameter pointer.
 * \return
 *		If successful, SetConfig() returns 0 or plus. Otherwise, it returns a minus value.
 *************************************************************************************************
 */
	int SetConfig(RENDER_INDEXTYPE nIndex, void *pParam);

/*!
 *************************************************************************************************
 * \brief
 *		IsInit_OK	: Call this to check if Renderer instance is created properly. 
 * \return
 *		If successful, IsInit_OK() returns true. Otherwise, it returns false.
 *************************************************************************************************
 */
	bool IsInit_OK(){return bInited_OK;}

private:
	enum BufferStatus {
	    OWNED_BY_US,
	    OWNED_BY_NATIVE_WINDOW
	};

	struct BufferInfo {
	    BufferStatus mStatus;
	    MediaBuffer *mMediaBuffer;
	};

	int _move_data_toGralloc(unsigned int YSrc, unsigned int USrc, unsigned int VSrc, unsigned char *YDest);
	int _memcpy_data_toGralloc(unsigned int YSrc, unsigned int USrc, unsigned int VSrc, unsigned char *YDest);
	bool _moveFrame(unsigned int Y, unsigned int U, unsigned int V, char isPhyAddr, void *plat_priv, sp<GraphicBuffer> graphicBuffer);

	void _render_local(sp<GraphicBuffer> graphicBuffer, int64_t ts_ms);
	int _setSyncMode(bool bAsync);
	int _setRotation(int32_t rotationDegrees);
	int _setCrop(int32_t left, int32_t top, int32_t right, int32_t bottom);
	status_t _allocateBuffers();
	status_t _freeBuffers();
	status_t _cancelBuffer(BufferInfo *info);
	BufferInfo* _dequeueBuffer();
	void _bufferRelease(MediaBuffer *buffer);

	void _fill_private_data(char bPhy_addr, int fd_val, void *plat_priv);
	
protected:
	sp<ANativeWindow> pNativeWindow;
	Vector<BufferInfo> mOutBuffers;
	List<MediaBuffer *> mFilledBuffers;
	gralloc_module_t const *mGrallocModule;
	
	mutable Mutex mLock;
	unsigned int mWidth;
	unsigned int mHeight;
	tFRAME_BUF_FORMAT mInFormat;
	int	mOutFormat;
	int nBufferCnt;
	bool bInited_OK;

	int	mTmem_fd;
	void* 	mTMapInfo;
	pmap_t	mUmpReservedPmap;
	bool bDRAM_16bit;

	int mFd_Copy;
	bool bASyncMode;
#ifdef USE_BUFFER_MANAGE_DIRECT
	int mIdx_codecInstance;
#endif
};

#endif

