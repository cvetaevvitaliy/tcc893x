
#define LOG_TAG "TCCExt_Player"
#include <utils/Log.h>

#ifdef USE_STAGEFRIGHT_IF
#include <Playback_main.h>
#else
#include <Playback_if.h>
#endif
using android::sp;
using android::Surface;
//using android::PlaybackMain;

#ifdef USE_STAGEFRIGHT_IF
PlaybackMain *mPlayback = NULL;
#else
PlaybackIf *mPlayback = NULL;
#endif

int createExt(
        void *surf,
        int codec_info,
        size_t decodedWidth, size_t decodedHeight) 
{        
	if (surf == 0)
		return -1;
	
	sp<Surface> pSurface = NULL;
	sp<IGraphicBufferProducer> new_st = NULL;

    pSurface = reinterpret_cast<Surface*>(surf);
    
	if (pSurface != NULL) {
        new_st = pSurface->getIGraphicBufferProducer();
    } else {
        ALOGE("Surface is NULL.");
        return -1;
    }

#ifdef USE_STAGEFRIGHT_IF
    mPlayback = new PlaybackMain();
#else
    mPlayback = new PlaybackIf();
#endif
    
    if( android::OK != mPlayback->set_Surface(new_st, 1))
        return -1;
    if( android::OK != mPlayback->configure_playback(codec_info, decodedWidth, decodedHeight))
        return -1;

    return 1;
}

void destoryExt()
{
	if( mPlayback != NULL)
	{
   		delete mPlayback;
		mPlayback = NULL;
	}
}

int startExt()
{
	if(!mPlayback){
		return -1;
	}
    
    if( android::OK != mPlayback->start_playback())
        return -1;

    return 1;
}

int stopExt()
{
	if(!mPlayback){
		return -1;
	}

    if( android::OK != mPlayback->stop_playback())
        return -1;

    return 1;
}

int ProcessExt(const uint8_t *data, uint32_t size, int64_t timestamp_ms) 
{
	if(!mPlayback){
		return -1;
	}
	
	mPlayback->receivedPacket(data, size, timestamp_ms);

    return 1;
}

int destoryRenderExt()
{
#ifndef USE_STAGEFRIGHT_IF
	if(!mPlayback){
		return -1;
	}
	
	mPlayback->stop_renderer();
#endif

    return 1;
}

int createRenderExt(void *surf)
{
#ifndef USE_STAGEFRIGHT_IF
	if(!mPlayback){
		return -1;
	}
	
	if (surf == 0)
		return -1;
	
	sp<Surface> pSurface = NULL;
	sp<IGraphicBufferProducer> new_st = NULL;

    pSurface = reinterpret_cast<Surface*>(surf);
    
	if (pSurface != NULL) {
        new_st = pSurface->getIGraphicBufferProducer();
    } else {
        ALOGE("Surface is NULL.");
        return -1;
    }

    if( android::OK != mPlayback->set_Surface(new_st, 1))
        return -1;
#endif

    return 1;
}

