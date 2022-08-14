/**
 * Copyright (C) 2013, Telechips Limited
 * by ZzaU(B070371)
 */

#include <jni.h>
#include <gui/Surface.h>
#include <utils/Vector.h>


#define TEST_TCCIF_DIRECT_INTERFACE
#ifdef TEST_TCCIF_DIRECT_INTERFACE
#include "tccif_direct_sample.h"
#else
#include "tccif_modified_stagefright_sample.h"
#endif
#include <android_runtime/android_graphics_SurfaceTexture.h>
#include <android_runtime/android_view_Surface.h>

extern "C" {

jfieldID    mSurface_texture;

#ifdef TEST_TCCIF_DIRECT_INTERFACE
DirectIf_Ex *pSample = NULL;
#else
StageFrightIf_Ex *pSample = NULL;
#endif

    static void created_instance(void)
    {
        if( !pSample )
        {
    #ifdef TEST_TCCIF_DIRECT_INTERFACE
            pSample = new DirectIf_Ex();
    #else
            pSample = new StageFrightIf_Ex();
    #endif
        }
    }

    JNIEXPORT jint JNICALL Java_com_telechips_v2ip_TccInterface_setCameraSurface(JNIEnv* env, jobject thiz, jobject jsurface, jint p1, jint p2, jint p3, jint p4)
    {
        ALOGI("JNI :: set_CameraSurface");
      
		sp<IGraphicBufferProducer> gbp; 
        sp<Surface> native_surfaceTmp;

		if(jsurface)
		{
			native_surfaceTmp = android_view_Surface_getSurface(env, jsurface);
			if(jsurface != NULL)
			{
				gbp = native_surfaceTmp->getIGraphicBufferProducer();
			}
			 else {
           		ALOGE("Surface is NULL.");
           		return 0;
        	}
		}

        created_instance();
        pSample->set_CameraSurface(gbp, p1, p2, p3, p4);
        
        return 0;
    }

    JNIEXPORT jint JNICALL Java_com_telechips_v2ip_TccInterface_setVideoSurface(JNIEnv* env, jobject thiz, jobject jsurface)
    {
        ALOGI("JNI :: set_VideoSurface");
        
        jclass clazz = env->FindClass("com/telechips/v2ip/TccInterface");

        jfieldID surfTexture = env->GetFieldID(clazz, "mNativeSurfaceTexture", "I");
        sp<IGraphicBufferProducer> old_st = reinterpret_cast<IGraphicBufferProducer*>(env->GetIntField(thiz, surfTexture));
        if (old_st != NULL) {
            old_st->decStrong(thiz);
        }
        
        sp<Surface> native_surfaceTmp(android_view_Surface_getSurface(env, jsurface));

        sp<IGraphicBufferProducer> new_st;
        if (native_surfaceTmp != NULL) {
            new_st = native_surfaceTmp->getIGraphicBufferProducer();
	    if( new_st != NULL )
            	new_st->incStrong(thiz);
        } else {
            ALOGE("Surface is NULL.");
            return 0;
        }

        env->SetIntField(thiz, surfTexture, (int)new_st.get());

        // This will fail if the media player has not been initialized yet. This
        // can be the case if setDisplay() on MediaPlayer.java has been called
        // before setDataSource(). The redundant call to setVideoSurfaceTexture()
        // in prepare/prepareAsync covers for this case.
        created_instance();
        pSample->set_VideoSurface(new_st);
        mSurface_texture = surfTexture;

        return 0;
    }

    JNIEXPORT jint JNICALL Java_com_telechips_v2ip_TccInterface_startOperation(JNIEnv* env, jobject thiz, jstring packageName)
    {
        ALOGI("JNI :: start");

        created_instance();

       // Convert client name jstring to String16
        const char16_t *rawClientName = env->GetStringChars(packageName, NULL);
        jsize rawClientNameLen = env->GetStringLength(packageName);
        String16 clientName(rawClientName, rawClientNameLen);
        env->ReleaseStringChars(packageName, rawClientName);

        pSample->start(clientName);
        
        return 0;
    }

    JNIEXPORT jint JNICALL Java_com_telechips_v2ip_TccInterface_stopOperation(JNIEnv* env, jobject thiz)
    {
        ALOGI("JNI :: stop");

        created_instance();
        pSample->stop();
        
        return 0;
    }

    JNIEXPORT void Java_com_telechips_v2ip_TccInterface_deallocate(JNIEnv* env, jobject thiz)
    { 
        ALOGI("JNI :: deallocate");
        
        sp<IGraphicBufferProducer> old_st = reinterpret_cast<IGraphicBufferProducer*>(env->GetIntField(thiz, mSurface_texture));
        if (old_st != NULL) {
            old_st->decStrong(thiz);
        }
        pSample->stop();
        delete pSample;
        pSample = NULL;
    }

    JNIEXPORT void JNICALL Java_com_telechips_v2ip_TccInterface_allocate(JNIEnv* env, jobject thiz)
    {
        ALOGI("JNI :: allocate");
        created_instance();
    }

    JNIEXPORT jint JNICALL Java_com_telechips_v2ip_TccInterface_changeFramerate(JNIEnv* env, jobject thiz, jint p1)
    {
        ALOGI("JNI :: change_framerate");

        created_instance();
        pSample->change_framerate(p1);
        
        return 0;
    }

    JNIEXPORT jint JNICALL Java_com_telechips_v2ip_TccInterface_changeBitrate(JNIEnv* env, jobject thiz, jint p1)
    {
        ALOGI("JNI :: change_bitrate");

        created_instance();
        pSample->change_bitrate(p1);
        
        return 0;
    }

    JNIEXPORT jint JNICALL Java_com_telechips_v2ip_TccInterface_requestIframe(JNIEnv* env, jobject thiz)
    {
        ALOGI("JNI :: request_intraframe");

        created_instance();
        pSample->request_intraframe();
        
        return 0;
    }

}

