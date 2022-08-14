/**
 * Copyright (C) 2012, Telechips Limited
 * by ZzaU(B070371)
 */

#define LOG_TAG "TCCVideoRenderer"
#include <utils/Log.h>

#include <Renderer_if.h>

#include <ext_render_wrapper.h>

using android::sp;
using android::Surface;

typedef struct _wrender_ {
	RendererIf *mRender;
}_wrender_;

void * wCreateRender( void *pNativeWindow, void *surf /*Surface pointer*/,
        unsigned int inWidth, unsigned int inHeight, tFRAME_BUF_FORMAT inFormat, int rotationDegrees, int cntOutBuff, bool aSyncMode)
{
    _wrender_ *pInst = NULL;
	sp<Surface> pSurface = NULL;
	sp<IGraphicBufferProducer> new_st = NULL;
    unsigned int count_outBuffer;

    if (surf == NULL && pNativeWindow == NULL)
		return NULL;	

    if (surf != NULL)
    {
        pSurface = reinterpret_cast<Surface*>(surf);
        
    	if (pSurface != NULL)
            new_st = pSurface->getIGraphicBufferProducer();
    }

    count_outBuffer = cntOutBuff == 0 ? DEF_BUFFER_CNT : cntOutBuff;
    pInst = (_wrender_*)malloc(sizeof(_wrender_));
    
    pInst->mRender = new RendererIf(new_st, pNativeWindow, inWidth, inHeight, inFormat, rotationDegrees, count_outBuffer, aSyncMode);
    
    if( !(pInst->mRender->IsInit_OK()) ){
        free(pInst);
        return NULL;
    }

    return pInst;
}

void wDestoryRender( void *pInst )
{
    if(pInst)
	{
		_wrender_ * pWrender = (_wrender_ *) pInst;
    
    	if( pWrender->mRender != NULL)
    	{
       		delete pWrender->mRender;
    		pWrender->mRender = NULL;
    	}

        if( pWrender != NULL) {
            free(pWrender);
        }
    }
}

int wRender( void *pInst, unsigned int Yaddr, unsigned int Uaddr, unsigned int Vaddr, char bAddrPhy, int64_t timestamp_ms, /*TCC_PLATFORM_PRIVATE_PMEM_INFO */ void *plat_priv ) 
{
    if(pInst)
	{
		_wrender_ * pWrender = (_wrender_ *) pInst;
    
    	if(!pWrender->mRender){
    		return -1;
    	}
    	
        pWrender->mRender->Render(Yaddr, Uaddr, Vaddr, bAddrPhy, timestamp_ms, (TCC_PLATFORM_PRIVATE_PMEM_INFO *)plat_priv);
    }

    return 1;
}

