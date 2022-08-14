/*
 * Copyright (C) 2013 Telechips, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_NDEBUG 0
#define LOG_TAG	"IPTVPlayer-JNI"
#include <utils/Log.h>

#include <stdio.h>
#include <assert.h>

#include <jni.h>
#include <JNIHelp.h>
#include <android_runtime/AndroidRuntime.h>
#include <android_runtime/android_view_Surface.h>
#include <utils/Errors.h>

#include "player/IPTVPlayer.h"

using namespace android;

struct fields_t {
    jfieldID	context;
    jfieldID	surface;
    jmethodID	post_event;
};
static fields_t fields;

static Mutex sLock;

class JNIIPTVPlayerListener : public IPTVPlayerListener {
public:
    JNIIPTVPlayerListener(JNIEnv *env, jobject thiz, jobject weak_thiz);
    ~JNIIPTVPlayerListener();
    void notify(int msg, int ext1, int ext2, void *obj);

private:
    JNIIPTVPlayerListener();
    jclass mClass;
    jobject mObject;
};

JNIIPTVPlayerListener::JNIIPTVPlayerListener(JNIEnv *env, jobject thiz, jobject weak_thiz)
{
    jclass clazz = env->GetObjectClass(thiz);
    if (clazz == NULL) {
        ALOGE("Can't find IPTVPlayer");
        jniThrowException(env, "java/lang/Exception", NULL);
        return;
    }
    mClass = (jclass) env->NewGlobalRef(clazz);
    mObject = env->NewGlobalRef(weak_thiz);
	env->DeleteLocalRef(clazz);
}

JNIIPTVPlayerListener::~JNIIPTVPlayerListener()
{
    JNIEnv *env = AndroidRuntime::getJNIEnv();
    env->DeleteGlobalRef(mObject);
    env->DeleteGlobalRef(mClass);
}

void JNIIPTVPlayerListener::notify(int msg, int ext1, int ext2, void *obj)
{
    JNIEnv *env = AndroidRuntime::getJNIEnv();
    switch (msg) {
    case IPTVPlayer::EVENT_CHANNEL_UPDATE:
    {
        env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, 0);	// fields.post_event -> java Event call(postEventFromNative)
        break;
    }
	case IPTVPlayer::EVENT_DVB_PARSING_COMPLETE:
		{
#if 1
			jstring jstr;
			IPTVSIData *pIptvSiData = (IPTVSIData *) obj;
		
			jmethodID mid = env->GetMethodID(mClass, "getIPTVSIDataObject", "()Lcom/telechips/android/iptv/IPTVSIData;");
		
			jobject jiptvobj = env->NewObject(mClass, mid);
			jobject jobj = env->CallObjectMethod(jiptvobj, mid);
		
			jclass clazz = env->GetObjectClass(jobj);
			int strSize = pIptvSiData->siDataSize;
			jfieldID fid = env->GetFieldID(clazz, "strSIData", "Ljava/lang/String;");
			fid = env->GetFieldID(clazz, "strSIData", "Ljava/lang/String;");
#if 0
			if( pIptvSiData->siDataType == 4 ) {	// UNICODE
				uint16_t uSIData[128];
				strSize = pIptvSiData->siDataSize/2;
				memcpy(uSIData, pIptvSiData->siData, pIptvSiData->siDataSize);
				jstr = env->NewString(uSIData, strSize);
				if( jstr ) {
					env->SetObjectField(jobj, fid, jstr);
					env->DeleteLocalRef(jstr);
				}
			}
			else {
				jbyteArray RetArray = env->NewByteArray(pIptvSiData->siDataSize);
				if (RetArray == NULL) {
					ALOGE("Couldn't allocate byte array for si data");
					env->ExceptionClear();
				}
				else {
					fid = env->GetFieldID(clazz, "siData", "[B");
					env->SetByteArrayRegion(RetArray, 0, pIptvSiData->siDataSize, (jbyte*)pIptvSiData->siData);
					env->SetObjectField(jobj, fid, RetArray);
				}
		
				if (RetArray) {
					env->DeleteLocalRef(RetArray);
				}
			}
#else
		{
			jbyteArray RetArray = env->NewByteArray(pIptvSiData->siDataSize);
			if (RetArray == NULL) {
				ALOGE("Couldn't allocate byte array for si data");
				env->ExceptionClear();
			}
			else {
				fid = env->GetFieldID(clazz, "SIData", "[B");
				env->SetByteArrayRegion(RetArray, 0, pIptvSiData->siDataSize, (jbyte*)pIptvSiData->siData);
				env->SetObjectField(jobj, fid, RetArray);
			}

			if (RetArray) {
				env->DeleteLocalRef(RetArray);
			}
		}
#endif
		
			fid = env->GetFieldID(clazz, "siDataSize", "I");
			env->SetIntField(jobj, fid, strSize);
		
			fid = env->GetFieldID(clazz, "siDataType", "I");
			env->SetIntField(jobj, fid, pIptvSiData->siDataType);
		
			env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, jobj);
		
			env->DeleteLocalRef(clazz);
			env->DeleteLocalRef(jobj);
			env->DeleteLocalRef(jiptvobj);
#endif
		}
	break;

    default:
        env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, 0);		// fields.post_event -> java Event call(postEventFromNative)
        break;
    }
}

static sp<IPTVPlayer> getIPTVPlayer(JNIEnv *env, jobject thiz)
{
    Mutex::Autolock l(sLock);
    IPTVPlayer *const p = (IPTVPlayer *) env->GetIntField(thiz, fields.context);
    return sp<IPTVPlayer>(p);
}

static sp<IPTVPlayer> setIPTVPlayer(JNIEnv *env, jobject thiz, const sp<IPTVPlayer>& player)
{
    Mutex::Autolock l(sLock);
    sp<IPTVPlayer> old = (IPTVPlayer *) env->GetIntField(thiz, fields.context);
    if (player != NULL && player.get()) {
        player->incStrong(thiz);
    }
    if (old != 0) {
        old->decStrong(thiz);
    }
    env->SetIntField(thiz, fields.context, (int)((player != NULL)? player.get():NULL));
    return old;
}

static void process_player_call(JNIEnv *env, jobject thiz, status_t opStatus, const char *exception, const char *message)
{
    if (exception == NULL) {
        if (opStatus != (status_t) OK) {
            sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
            if (player != 0)
                player->notify(IPTVPlayer::EVENT_ERROR, opStatus, 0, 0);
        }
    } else {
        // Throw exception
        if (opStatus != (status_t) OK) {
            jniThrowException(env, exception, message);
        }
    }
}

static void setVideoSurface(const sp<IPTVPlayer>& player, JNIEnv *env, jobject thiz)
{
	jobject jsurface = env->GetObjectField(thiz, fields.surface);
	sp<IGraphicBufferProducer> new_st;
	if(jsurface != NULL) {
		sp<Surface> surface(android_view_Surface_getSurface(env, jsurface));
		if(surface != NULL) {
			new_st = surface->getIGraphicBufferProducer();
			if (new_st == NULL) {
				jniThrowException(env, "java/lang/IllegalArgumentException",
					"The surface does not have a binding SurfaceTexture!");
				return;
			}
		} else {
			jniThrowException(env, "java/lang/IllegalArgumentException",
				"The surface has been released");
			return;
		}
	}

	ALOGV("prepare: SurfaceTexture=%p", new_st.get());
	player->setVideoSurfaceTexture(new_st);
}

static void IPTVPlayer_prepare(JNIEnv *env, jobject thiz)
{
    ALOGV("prepare");
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->prepare(), "java/io/IOException", "Prepare failed.");
}

static void IPTVPlayer_setSurface(JNIEnv *env, jobject thiz)
{

    ALOGV("%s", __func__);
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    setVideoSurface(player, env, thiz);
}


static void IPTVPlayer_start(JNIEnv *env, jobject thiz)
{
    ALOGV("start");
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->start(), NULL, NULL);
}

static void IPTVPlayer_stop(JNIEnv *env, jobject thiz)
{
    ALOGV("stop");
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->stop(), NULL, NULL);
}

static jint IPTVPlayer_getVideoWidth(JNIEnv *env, jobject thiz)
{
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }
    int w;
    if (0 != player->getVideoWidth(&w)) {
        ALOGE("getVideoWidth failed");
        w = 0;
    }
    ALOGV("getVideoWidth: %d", w);
    return w;
}

static jint IPTVPlayer_getVideoHeight(JNIEnv *env, jobject thiz)
{
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }
    int h;
    if (0 != player->getVideoHeight(&h)) {
        ALOGE("getVideoHeight failed");
        h = 0;
    }
    ALOGV("getVideoHeight: %d", h);
    return h;
}

static jboolean IPTVPlayer_isPlaying(JNIEnv *env, jobject thiz)
{
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return false;
    }
    const jboolean is_playing = player->isPlaying();
    ALOGV("isPlaying: %d", is_playing);
    return is_playing;
}

static void IPTVPlayer_release(JNIEnv *env, jobject thiz)
{
    ALOGV("release");
    sp<IPTVPlayer> player = setIPTVPlayer(env, thiz, 0);
    if (player != NULL) {
	player->release();
        player->setListener(0);
    }
}

static void IPTVPlayer_setVolume(JNIEnv *env, jobject thiz, jfloat leftVolume, jfloat rightVolume)
{
    ALOGV("setVolume");
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return;
    }
    process_player_call(env, thiz, player->setVolume(leftVolume, rightVolume), NULL, "setVolume failed.");
}

static void IPTVPlayer_setDisplayPosition(JNIEnv *env, jobject thiz, jint x, jint y, jint width, jint height, jint rotate)
{
    ALOGV("setDisplayPosition");
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return;
    }
    process_player_call(env, thiz, player->setDisplayPosition(x, y, width, height, rotate), NULL, "setDisplayPosition failed.");
}

static jint IPTVPlayer_setCapture(JNIEnv *env, jobject thiz, jstring filePath)
{
    ALOGV("setCapture");
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    char strBuff[128];	
    const char *sz = env->GetStringUTFChars(filePath, 0) ;
    strcpy(strBuff, sz) ;
    env->ReleaseStringUTFChars(filePath, sz) ;

    process_player_call(env, thiz, player->setCapture(strBuff), NULL, "setCapture failed.");

    return 0;
}

static jint IPTVPlayer_setRecord(JNIEnv *env, jobject thiz, jstring filePath)
{
    ALOGV("setRecord");
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    char strBuff[128];	
    const char *sz = env->GetStringUTFChars(filePath, 0) ;
    strcpy(strBuff, sz) ;
    env->ReleaseStringUTFChars(filePath, sz) ;

    process_player_call(env, thiz, player->setRecord(strBuff), NULL, "setRecord failed.");

    return 0;
}

static jint IPTVPlayer_setRecStop(JNIEnv *env, jobject thiz)
{
    ALOGV("setRecStop");
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->setRecStop(), NULL, "setRecStop failed.");

    return 0;
}

static jint IPTVPlayer_setLCDUpdate(JNIEnv *env, jobject thiz)
{
    ALOGV("IPTVPlayer_setLCDUpdate");
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->setLCDUpdate(), NULL, "setLCDUpdate failed.");

    return 0;
}

static jint IPTVPlayer_setPIDInfo(JNIEnv *env, jobject thiz, jint audio_pid, jint video_pid, jint pcr_pid, jint audio_type, jint  vieo_type)
{
    ALOGV("%s",__func__);
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->setPIDInfo(audio_pid, video_pid, pcr_pid, audio_type, vieo_type), NULL, "IPTVPlayer_setPIDInfo failed.");
    return 0;
}

static jint IPTVPlayer_setIPInfo(JNIEnv *env, jobject thiz, jint port, jstring ip, jint protocol)
{
    ALOGV("%s",__func__);
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    char strBuff[128];	
    const char *sz = env->GetStringUTFChars(ip, 0) ;
    strcpy(strBuff, sz) ;
    env->ReleaseStringUTFChars(ip, sz) ;

    process_player_call(env, thiz, player->setIPInfo(port,strBuff, protocol), NULL, "IPTVPlayer_setIPInfo failed.");
    return 0;
}

static jint IPTVPlayer_setSocketInit(JNIEnv *env, jobject thiz)
{
    ALOGV("%s",__func__);
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->setSocketInit(), NULL, "IPTVPlayer_setSocketInit failed.");
    return 0;
}

static jint IPTVPlayer_setSocketStart(JNIEnv *env, jobject thiz)
{
    ALOGV("%s",__func__);
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->setSocketStart(), NULL, "IPTVPlayer_setSocketIStart failed.");
    return 0;
}

static jint IPTVPlayer_setSocketStop(JNIEnv *env, jobject thiz)
{
    ALOGV("%s",__func__);
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->setSocketStop(), NULL, "IPTVPlayer_setSocketIStop failed.");
    return 0;
}

static jint IPTVPlayer_setSocketCommand(JNIEnv *env, jobject thiz, jint cmd)
{
    ALOGV("%s",__func__);
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->setSocketCommand(cmd), NULL, "IPTVPlayer_setSocketICommand failed.");
    return 0;
}

static jint IPTVPlayer_execute(JNIEnv *env, jobject thiz)
{
    ALOGV("%s",__func__);
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->execute(), NULL, "IPTVPlayer_play failed.");
    return 0;
}

static void IPTVPlayer_useSurface(JNIEnv *env, jobject thiz)
{
    ALOGV("%s",__func__);
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->useSurface(), NULL, NULL);
}

static void IPTVPlayer_ReleaseSurface(JNIEnv *env, jobject thiz)
{
    ALOGV("start");
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->releaseSurface(), NULL, NULL);
}

static jint IPTVPlayer_setAcitveMode(JNIEnv *env, jobject thiz, jint activemode)
{
    ALOGV("%s",__func__);
    sp<IPTVPlayer> player = getIPTVPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->setActiveMode(activemode), NULL, "IPTVPlayer_setActiveMode failed.");
    return 0;
}

static void IPTVPlayer_native_setup(JNIEnv *env, jobject thiz, jobject weak_this)
{
    ALOGV("IPTVPlayer_native_setup");
    sp<IPTVPlayer> player = new IPTVPlayer();
    if (player == NULL) {
        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
        return;
    }

    // create new listener
    sp<JNIIPTVPlayerListener> listener = new JNIIPTVPlayerListener(env, thiz, weak_this);
    player->setListener(listener);

    setIPTVPlayer(env, thiz, player);
}

static void IPTVPlayer_native_finalize(JNIEnv *env, jobject thiz)
{
    ALOGV("native_finalize");
    IPTVPlayer_release(env, thiz);
}

static const char *classPathName = "com/telechips/android/iptv/IPTVPlayer";

static JNINativeMethod methods[] = {
  { "prepare", "()V", (void*)IPTVPlayer_prepare },
  { "_start", "()V", (void*)IPTVPlayer_start },
  { "_stop", "()V", (void*)IPTVPlayer_stop },
  { "getVideoWidth", "()I", (void*)IPTVPlayer_getVideoWidth },
  { "getVideoHeight", "()I", (void*)IPTVPlayer_getVideoHeight },
  { "isPlaying", "()Z", (void*)IPTVPlayer_isPlaying },
  { "_release", "()V", (void*)IPTVPlayer_release },
  { "setVolume", "(FF)V", (void*)IPTVPlayer_setVolume },
  { "setDisplayPosition", "(IIIII)V", (void*)IPTVPlayer_setDisplayPosition },
  { "setPIDInfo", "(IIIII)I", (void*)IPTVPlayer_setPIDInfo },
  { "setIPInfo", "(ILjava/lang/String;I)I", (void*)IPTVPlayer_setIPInfo },
  { "setSurface", "()V", (void*)IPTVPlayer_setSurface },
  { "setSocketInit", "()I", (void*)IPTVPlayer_setSocketInit},
  { "setSocketStart", "()I", (void*)IPTVPlayer_setSocketStart},
  { "setSocketStop", "()I", (void*)IPTVPlayer_setSocketStop},
  { "setSocketCommand", "(I)I", (void*)IPTVPlayer_setSocketCommand},
  { "execute", "()I", (void*)IPTVPlayer_execute },
  { "useSurface", "()V", (void*)IPTVPlayer_useSurface },
  { "releaseSurface", "()V", (void*)IPTVPlayer_ReleaseSurface },
  { "setActiveMode", "(I)I", (void*)IPTVPlayer_setAcitveMode },
  { "setCapture", "(Ljava/lang/String;)I", (void*)IPTVPlayer_setCapture },
  { "setRecord", "(Ljava/lang/String;)I", (void*)IPTVPlayer_setRecord },
  { "setRecStop", "()I", (void*)IPTVPlayer_setRecStop },
  { "setLCDUpdate", "()I", (void*)IPTVPlayer_setLCDUpdate },
  { "native_setup", "(Ljava/lang/Object;)V", (void*)IPTVPlayer_native_setup },
  { "native_finalize", "()V", (void*)IPTVPlayer_native_finalize }
};

/*
 * Register several native methods for one class.
 */
static int registerNativeMethods(JNIEnv* env, const char* className,
    JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;

    clazz = env->FindClass(className);
    if (clazz == NULL) {
        ALOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }

    fields.context = env->GetFieldID(clazz, "mNativeContext", "I");
    if (fields.context == NULL) {
        ALOGE("Can't find mNativeContext field");
        return JNI_FALSE;
    }

    fields.post_event = env->GetStaticMethodID(clazz, "postEventFromNative",
                                               "(Ljava/lang/Object;IIILjava/lang/Object;)V");	// java에서 notify msg 수신하는 부분.
    if (fields.post_event == NULL) {
        ALOGE("Can't find postEventFromNative method");
        return JNI_FALSE;
    }
    fields.surface = env->GetFieldID(clazz, "mSurface", "Landroid/view/Surface;");
    if (fields.surface == NULL) {
        ALOGE("Can't find android/view/Surface");
        return JNI_FALSE;
    }

    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {	// java -> cpp 연결하는 부분.
        ALOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

/*
 * Register native methods for all classes we know about.
 *
 * returns JNI_TRUE on success.
 */
static int registerNatives(JNIEnv* env)
{
  if (!registerNativeMethods(env, classPathName,
                 methods, sizeof(methods) / sizeof(methods[0]))) {
    return JNI_FALSE;
  }

  return JNI_TRUE;
}

/*
 * This is called by the VM when the shared library is first loaded.
 */
jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    ALOGD("%s", __func__);

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("ERROR: GetEnv failed\n");
        goto bail;
    }
    assert(env != NULL);

    if (registerNatives(env) != JNI_TRUE) {
        ALOGE("ERROR: registerNatives failed");
        goto bail;
    }

    /* success -- return valid version number */
    result = JNI_VERSION_1_4;

bail:
    return result;
}

