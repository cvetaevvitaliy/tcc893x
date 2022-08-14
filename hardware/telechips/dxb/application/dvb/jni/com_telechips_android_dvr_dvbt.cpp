/*
 * Copyright (C) 2008 The Android Open Source Project
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
#define LOG_TAG	"DVBTPlayer-JNI"
#include <utils/Log.h>

#include <jni.h>
#include <JNIHelp.h>
#include <android_runtime/AndroidRuntime.h>
#include <android_runtime/android_view_Surface.h>

#include <cutils/properties.h>
#include <binder/Parcel.h>
#include <android_os_Parcel.h>

#include "player/DVBTPlayer.h"

using namespace android;

struct fields_t {
    jfieldID	context;
    jfieldID	surface;
    jmethodID	post_event;
};
static fields_t fields;

static Mutex sLock;

class JNIDVBTPlayerListener : public DVBTPlayerListener {
public:
    JNIDVBTPlayerListener(JNIEnv *env, jobject thiz, jobject weak_thiz);
    ~JNIDVBTPlayerListener();
    virtual void notify(int msg, int ext1, int ext2, const Parcel *obj = NULL);

private:
    JNIDVBTPlayerListener();
    jclass mClass;
    jobject mObject;
};

JNIDVBTPlayerListener::JNIDVBTPlayerListener(JNIEnv *env, jobject thiz, jobject weak_thiz)
{
	jclass clazz = env->GetObjectClass(thiz);
	if (clazz == NULL)
	{
		ALOGE("Can't find DVBTPlayer");
		jniThrowException(env, "java/lang/Exception", NULL);
		
		return;
	}
 
	mClass = (jclass) env->NewGlobalRef(clazz);
	mObject = env->NewGlobalRef(weak_thiz);
	env->DeleteLocalRef(clazz);
}

JNIDVBTPlayerListener::~JNIDVBTPlayerListener()
{
    JNIEnv *env = AndroidRuntime::getJNIEnv();
    env->DeleteGlobalRef(mObject);
    env->DeleteGlobalRef(mClass);
}

void JNIDVBTPlayerListener::notify(int msg, int ext1, int ext2, const Parcel *obj)
{
	JNIEnv *env = AndroidRuntime::getJNIEnv();
	if (obj && obj->dataSize() > 0) {
		jobject jParcel = createJavaParcelObject(env);
		if (jParcel != NULL) {
			Parcel* nativeParcel = parcelForJavaObject(env, jParcel);
			nativeParcel->setData(obj->data(), obj->dataSize());
			env->CallStaticVoidMethod(mClass, fields.post_event, mObject,
					msg, ext1, ext2, jParcel);
			env->DeleteLocalRef(jParcel);
		}
	} else {
		env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, NULL);
	}
}

static sp<DVBTPlayer> getDVBTPlayer(JNIEnv *env, jobject thiz)
{
    Mutex::Autolock l(sLock);
    DVBTPlayer *const p = (DVBTPlayer *) env->GetIntField(thiz, fields.context);
    return sp<DVBTPlayer>(p);
}

static sp<DVBTPlayer> setDVBTPlayer(JNIEnv *env, jobject thiz, const sp<DVBTPlayer>& player)
{
    Mutex::Autolock l(sLock);
    sp<DVBTPlayer> old = (DVBTPlayer *) env->GetIntField(thiz, fields.context);
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
            sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
            if (player != 0)
                player->notify(DVBTPlayer::EVENT_ERROR, opStatus, 0, 0);
        }
    } else {
        // Throw exception
        if (opStatus != (status_t) OK) {
            jniThrowException(env, exception, message);
        }
    }
}

static void setVideoSurface(const sp<DVBTPlayer>& player, JNIEnv *env, jobject thiz)
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

static void DVBTPlayer_prepare(JNIEnv *env, jobject thiz, jstring dbPath)
{
    ALOGV("prepare");
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }    
    char strBuff[128];	
    const char *sz = env->GetStringUTFChars(dbPath, 0) ;
    strcpy(strBuff, sz) ;
    env->ReleaseStringUTFChars(dbPath, sz) ;
    process_player_call(env, thiz, player->prepare(strBuff), "java/io/IOException", "Prepare failed.");
}

static void DVBTPlayer_start(JNIEnv *env, jobject thiz, jint country_code)
{
    ALOGV("start");
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->start(country_code), NULL, NULL);
}

static void DVBTPlayer_stop(JNIEnv *env, jobject thiz)
{
    ALOGV("stop");
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->stop(), NULL, NULL);
}

static jint DVBTPlayer_getSignalStrength(JNIEnv *env, jobject thiz)
{
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }
    unsigned int LV[5] = {0, 0, 0, 0, 0};
    if (0 != player->getSignalStrength(LV)) {
        ALOGE("getSNRStrength failed");
        LV[0] = 0;
    }
    //ALOGV("getSNRStrength : %d", LV);
    return (int)(LV[0] / 20);
}

static jint DVBTPlayer_getSignalState(JNIEnv *env, jobject thiz, jobject signal)
{
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return -1;
    }

    char msg[255] = "";
    unsigned int state[5] = {0, 0, 0, 0, (unsigned int)msg};
    if (0 == player->getSignalStrength(state)) {
        jclass   clazz = env->GetObjectClass(signal);
        jfieldID fid;

        fid = env->GetFieldID(clazz, "Strength", "I");
        env->SetIntField(signal, fid, state[0]);

        fid = env->GetFieldID(clazz, "Quality", "I");
        env->SetIntField(signal, fid, state[1]);

        fid = env->GetFieldID(clazz, "SNR", "I");
        env->SetIntField(signal, fid, state[2]);

        fid = env->GetFieldID(clazz, "BER", "I");
        env->SetIntField(signal, fid, state[3]);

        if (strlen(msg) > 0) {
            fid = env->GetFieldID(clazz, "Message", "Ljava/lang/String;");
            jstring jstr = env->NewStringUTF(msg);
            if( jstr ) {
                env->SetObjectField(signal, fid, jstr);
                env->DeleteLocalRef(jstr);
            }
        }

        env->DeleteLocalRef(clazz);
    } else {
        ALOGE("Failed to get the state of signal");
        return -2;
    }

    return 0;
}

static void DVBTPlayer_setAntennaPower(JNIEnv *env, jobject thiz, jint arg)
{
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    player->setAntennaPower(arg);
}

static jint DVBTPlayer_SetLnbVoltage(JNIEnv *env, jobject thiz, jint arg)
{
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }
    return player->SetLnbVoltage(arg);
}

static jint DVBTPlayer_SetTone(JNIEnv *env, jobject thiz, jint arg)
{
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }
    return player->SetTone(arg);
}

static jint DVBTPlayer_DiseqcSendBurst(JNIEnv *env, jobject thiz, jint arg)
{
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }
    return player->DiseqcSendBurst(arg);
}

static jint DVBTPlayer_DiseqcSendCMD(JNIEnv *env, jobject thiz, jbyteArray arg)
{
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }

    jbyte CMD[128];
    jsize size = env->GetArrayLength(arg);
    env->GetByteArrayRegion(arg, 0, size, CMD);

    return player->DiseqcSendCMD((unsigned char *)CMD, size);
}

static jint DVBTPlayer_BlindScanReset(JNIEnv *env, jobject thiz)
{
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }

    if (player->BlindScanReset() == OK)
        return 0;

    return -1;
}

static jint DVBTPlayer_BlindScanStart(JNIEnv *env, jobject thiz)
{
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }

    if (player->BlindScanStart() == OK)
        return 0;

    return -1;
}

static jint DVBTPlayer_BlindScanCancel(JNIEnv *env, jobject thiz)
{
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }

    if (player->BlindScanCancel() == OK)
        return 0;

    return -1;
}

static jint DVBTPlayer_BlindScanGetState(JNIEnv *env, jobject thiz)
{
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }

	unsigned int state;
    if (player->BlindScanGetState(&state) == OK)
        return state;

    return -1;
}

static jobject DVBTPlayer_BlindScanGetInfo(JNIEnv *env, jobject thiz)
{
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }

    unsigned int arg[4];
    if (player->BlindScanGetInfo(arg) == OK)
    {
        jobject jParcel = createJavaParcelObject(env);
        if (jParcel != NULL) {
            Parcel* nativeParcel = parcelForJavaObject(env, jParcel);
            nativeParcel->setData((uint8_t*)arg, 16);
            return jParcel;
        }
    }
    return NULL;
}

static void DVBTPlayer_release(JNIEnv *env, jobject thiz)
{
    ALOGV("release");
    sp<DVBTPlayer> player = setDVBTPlayer(env, thiz, 0);
    if (player != NULL) {
	player->release();
        player->setListener(0);
    }
}

static jint DVBTPlayer_startTestMode(JNIEnv *env, jobject thiz, jint index)
{
	ALOGV("%s",__func__);
	sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
	if (player == NULL)
	{
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	process_player_call(env, thiz, player->startTestMode(index), NULL, "DVBTPlayer_startTestMode failed.");
	return 0;
}

static void DVBTPlayer_search(JNIEnv *env, jobject thiz, jint countryCode)
{
    ALOGV("DVBTPlayer_search: countryCode = %d", countryCode);
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
	status_t opStatus = player->search(countryCode);
    process_player_call(env, thiz, opStatus, NULL, NULL);
}

static void DVBTPlayer_manualSearch(JNIEnv *env, jobject thiz, jint frequencyKhz, jint bandwidthMhz)
{
    ALOGV("DVBTPlayer_manualSearch: frequency = %d, bandwidth = %d", frequencyKhz, bandwidthMhz);
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
	status_t opStatus = player->manualSearch(frequencyKhz, bandwidthMhz);
    process_player_call(env, thiz, opStatus, NULL, NULL);
}

static void DVBTPlayer_stop_search(JNIEnv *env, jobject thiz)
{
    ALOGV("DVBTPlayer_stop_search");
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->stopsearch(), NULL, NULL);
}

static void DVBTPlayer_releaseSurface(JNIEnv *env, jobject thiz)
{
    ALOGV("%s", __func__);
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->releaseSurface(), NULL, NULL);
}

static jint DVBTPlayer_useSurface(JNIEnv *env, jobject thiz, jint arg)
{
    ALOGV("%s", __func__);
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return -1;
    }
    process_player_call(env, thiz, player->useSurface(arg), NULL, NULL);
    return 0;
}

static void DVBTPlayer_setSurface(JNIEnv *env, jobject thiz)
{

    ALOGV("%s", __func__);
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    setVideoSurface(player, env, thiz);
}

static void DVBTPlayer_setChannel(JNIEnv *env, jobject thiz, jint rowID, jint audioIndex, jint subtitleIndex)
{
    ALOGV("setChannel");
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    status_t opStatus = player->setChannel(rowID, audioIndex, subtitleIndex);
    process_player_call(env, thiz, opStatus, NULL, NULL);  
}

static void DVBTPlayer_setCountryCode(JNIEnv *env, jobject thiz, jint countryCode)
{
    ALOGV("setCountryCode");
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->setCountryCode(countryCode), NULL, "setCountryCode failed.");
}

static void DVBTPlayer_setTuneSpace(JNIEnv *env, jobject thiz, jobject space)
{
    ALOGV("setTuneSpace");
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    TuneSpace stTuneSpace;
    jclass       clazz = env->GetObjectClass(space);
    jfieldID     fid;
    jstring      strTemp;
    jint         intTemp;
    jintArray    intArray;
    jobject      objTemp;
    jobjectArray objArray;
    jclass       subClazz;

    fid = env->GetFieldID(clazz, "name", "Ljava/lang/String;");
    strTemp = (jstring)env->GetObjectField(space, fid);
    if (strTemp)
    {
        const char *szName = env->GetStringUTFChars(strTemp, NULL);
        strcpy(stTuneSpace.name, szName);
        env->ReleaseStringUTFChars(strTemp, szName) ;
	}
	else
	{
		stTuneSpace.name[0] = NULL;
	}

    fid = env->GetFieldID(clazz, "countryCode", "I");
    stTuneSpace.countryCode = env->GetIntField(space, fid);

    fid = env->GetFieldID(clazz, "space", "I");
    stTuneSpace.space = env->GetIntField(space, fid);

    fid = env->GetFieldID(clazz, "typicalBandwidth", "I");
    stTuneSpace.typicalBandwidth = env->GetIntField(space, fid);

    fid = env->GetFieldID(clazz, "minChannel", "I");
    stTuneSpace.minChannel = env->GetIntField(space, fid);

    fid = env->GetFieldID(clazz, "maxChannel", "I");
    stTuneSpace.maxChannel = env->GetIntField(space, fid);

    fid = env->GetFieldID(clazz, "finetunes", "[I");
    intArray = (jintArray)env->GetObjectField(space, fid);
    env->GetIntArrayRegion(intArray, 0, 4, stTuneSpace.finetunes);

    fid = env->GetFieldID(clazz, "numbers", "I");
    stTuneSpace.numbers = env->GetIntField(space, fid);

    fid = env->GetFieldID(clazz, "channels", "[Lcom/telechips/android/dvb/player/dvb/OneChannel;");
    objArray = (jobjectArray)env->GetObjectField(space, fid);
    for (int i = 0; i < stTuneSpace.numbers; i++)
    {
        objTemp = (jobject)env->GetObjectArrayElement(objArray, i);
        subClazz = env->GetObjectClass(objTemp);

        fid = env->GetFieldID(subClazz, "name", "Ljava/lang/String;");
        strTemp = (jstring)env->GetObjectField(objTemp, fid);
        if (strTemp)
        {
            const char *szName = env->GetStringUTFChars(strTemp, NULL);
            strcpy(stTuneSpace.channels[i].name, szName);
            env->ReleaseStringUTFChars(strTemp, szName) ;
        }
        else
        {
            stTuneSpace.channels[i].name[0] = NULL;
        }

        fid = env->GetFieldID(subClazz, "frequency1", "I");
        stTuneSpace.channels[i].frequency1 = env->GetIntField(objTemp, fid);

        fid = env->GetFieldID(subClazz, "frequency2", "I");
        stTuneSpace.channels[i].frequency2 = env->GetIntField(objTemp, fid);

        fid = env->GetFieldID(subClazz, "finetune", "I");
        stTuneSpace.channels[i].finetune = env->GetIntField(objTemp, fid);

        fid = env->GetFieldID(subClazz, "bandwidth1", "I");
        stTuneSpace.channels[i].bandwidth1 = env->GetIntField(objTemp, fid);

        fid = env->GetFieldID(subClazz, "bandwidth2", "I");
        stTuneSpace.channels[i].bandwidth2 = env->GetIntField(objTemp, fid);

        fid = env->GetFieldID(subClazz, "alt_selected", "I");
        stTuneSpace.channels[i].alt_selected = env->GetIntField(objTemp, fid);

        env->DeleteLocalRef(subClazz);
    }

    env->DeleteLocalRef(clazz);

	player->setTuneSpace(&stTuneSpace);
}

static jint DVBTPlayer_setCapture(JNIEnv *env, jobject thiz, jstring filePath)
{
    ALOGV("setCapture");
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
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

static jint DVBTPlayer_setRecord(JNIEnv *env, jobject thiz, jstring filePath)
{
    ALOGV("setRecord");
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
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

static jint DVBTPlayer_setRecStop(JNIEnv *env, jobject thiz)
{
    ALOGV("setRecStop");
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->setRecStop(), NULL, "setRecStop failed.");

    return 0;
}

static jint DVBTPlayer_setPlay(JNIEnv *env, jobject thiz, jstring filePath)
{
    ALOGV("setFilePlay");
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    char strBuff[256];	
    const char *sz = env->GetStringUTFChars(filePath, 0) ;
    strcpy(strBuff, sz) ;
    env->ReleaseStringUTFChars(filePath, sz) ;

    process_player_call(env, thiz, player->setPlay(strBuff), NULL, "setPlay failed.");

    return 0;
}

static jint DVBTPlayer_setPlayStop(JNIEnv *env, jobject thiz)
{
    ALOGV("setFilePlayStop");
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->setPlayStop(), NULL, "setPlayStop failed.");

    return 0;
}

static jint DVBTPlayer_setSeekTo(JNIEnv *env, jobject thiz, jint arg)
{
    ALOGV("setFilePlaySeekTo");
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    int seekTime;

    process_player_call(env, thiz, player->setSeekTo(arg), NULL, "setSeekTo failed.");

    return 0;
}

static jint DVBTPlayer_setPlaySpeed(JNIEnv *env, jobject thiz, jint arg)
{
    ALOGV("setFilePlaySpeed");
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    //process_player_call(env, thiz, player->setPlaySpeed(arg), NULL, "setPlaySpeed failed.");

    return player->setPlaySpeed(arg);
}

static void DVBTPlayer_setPlayPause(JNIEnv *env, jobject thiz, jboolean arg)
{
    ALOGV("DVBTPlayer_setPlayPause");
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return;
    }

    process_player_call(env, thiz, player->setPlayPause(arg), NULL, "setPlayPause failed.");

}

static void DVBTPlayer_setUserLocalTimeOffset(JNIEnv *env, jobject thiz, jboolean bAutoMode, jint iUserLocalTimeOffset)
{
    ALOGV("DVBTPlayer_setUserLocalTimeOffset");
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return;
    }

    process_player_call(env, thiz, player->setUserLocalTimeOffset(bAutoMode, iUserLocalTimeOffset), NULL, "setUserLocalTimeOffset failed.");

}

static jint DVBTPlayer_getDuration(JNIEnv *env, jobject thiz)
{
//    ALOGV("getDuration");
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    return player->getDuration();
}

static jint DVBTPlayer_getCurrentPosition(JNIEnv *env, jobject thiz)
{
//    ALOGV("getCurrentPosition");
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    return player->getCurrentPosition();
}

static jint DVBTPlayer_getCurrentReadPosition(JNIEnv *env, jobject thiz)
{
//    ALOGV("getCurrentReadPosition");
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    return player->getCurrentReadPosition();
}

static jint DVBTPlayer_getCurrentWritePosition(JNIEnv *env, jobject thiz)
{
//    ALOGV("getCurrentWritePosition");
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    return player->getCurrentWritePosition();
}

static jstring DVBTPlayer_getDiskFSType(JNIEnv *env, jobject thiz)
{
//    ALOGV("getDiskFSType");
	char strFSType[12];
	jstring jstr;
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return NULL;
    }

	memset(strFSType, 0x00, 12);
	player->getFSType(strFSType);
	//ALOGE("%s, %s", __func__, strFSType);
	jstr = env->NewStringUTF(strFSType);
	if( jstr ) {
		//env->DeleteLocalRef(jstr);
		//ALOGV("getFSType = %s", strFSType);
	} else {
		ALOGV("getFSType = NULL");
	}

    return jstr;
}

static jint DVBTPlayer_getCurrentDateTime(JNIEnv *env, jobject thiz, jobject date_time)
{
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    DATE_TIME_STRUCT *pDateTime = player->getCurrentDateTime();

    jclass   clazz = env->GetObjectClass(date_time);
    jfieldID fid;

    // MJD
    fid = env->GetFieldID(clazz, "iMJD", "I");
    env->SetIntField(date_time, fid, pDateTime->uiMJD);

    // Hour
    fid = env->GetFieldID(clazz, "iHour", "I");
    env->SetIntField(date_time, fid, pDateTime->stTime.ucHour);

    // Minute
    fid = env->GetFieldID(clazz, "iMinute", "I");
    env->SetIntField(date_time, fid, pDateTime->stTime.ucMinute);

    // Second
    fid = env->GetFieldID(clazz, "iSecond", "I");
    env->SetIntField(date_time, fid, pDateTime->stTime.ucSecond);

    env->DeleteLocalRef(clazz);

    return 0;
}

static jint DVBTPlayer_playSubtitle(JNIEnv *env, jobject thiz, jint onoff, jint subtitleIndex)
{
    ALOGV("%s",__func__);
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->playSubtitle(onoff, subtitleIndex), NULL, "DVBTPlayer_playSubtitle failed.");
    return 0;
}

static jint DVBTPlayer_requestEPGUpdate(JNIEnv *env, jobject thiz, jint arg)
{
    ALOGV("%s",__func__);
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }
    process_player_call(env, thiz, player->requestEPGUpdate(arg), NULL, "DVBTPlayer_updateEPGUpdate failed.");
    return 0;
}

static jint DVBTPlayer_setAudio(JNIEnv *env, jobject thiz, jint audioIndex)
{
    ALOGV("%s",__func__);
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }
    process_player_call(env, thiz, player->setAudio(audioIndex), NULL, "DVBTPlayer_setAudio failed.");
    return 0;
}

static jint DVBTPlayer_setStereo(JNIEnv *env, jobject thiz, jint mode)
{
    ALOGV("%s",__func__);
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }
    process_player_call(env, thiz, player->setStereo(mode), NULL, "DVBTPlayer_setStereo failed.");
    return 0;
}

static jint DVBTPlayer_playTeletext(JNIEnv *env, jobject thiz, jint onoff)
{
	ALOGV("%s",__func__);
	sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
	if (player == NULL)
	{
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	process_player_call(env, thiz, player->playTeletext(onoff), NULL, "DVBTPlayer_playTeletext failed.");
	return 0;
}

static jint DVBTPlayer_setTeletext_UpdatePage(JNIEnv *env, jobject thiz, jint pageNumber)
{
	ALOGV("%s",__func__);
	sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
	if (player == NULL)
	{
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	process_player_call(env, thiz, player->setTeletext_UpdatePage(pageNumber), NULL, "DVBTPlayer_setTeletext_UpdatePage failed.");
	return 0;
}

static jint DVBTPlayer_setTeletext_CachePage_Insert(JNIEnv *env, jobject thiz, jint pageNumber)
{
	//ALOGV("%s",__func__);
	sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
	if (player == NULL)
	{
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	process_player_call(env, thiz, player->setTeletext_CachePage_Insert(pageNumber), NULL, "DVBTPlayer_setTeletext_CachePage_Insert failed.");
	return 0;
}

static jint DVBTPlayer_setTeletext_CachePage_Delete(JNIEnv *env, jobject thiz, jint pageNumber)
{
	ALOGV("%s",__func__);
	sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
	if (player == NULL)
	{
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	process_player_call(env, thiz, player->setTeletext_CachePage_Delete(pageNumber), NULL, "DVBTPlayer_setTeletext_CachePage_Delete failed.");
	return 0;
}

static jint DVBTPlayer_goToSleep(JNIEnv *env, jobject thiz)
{
	ALOGV("%s", __func__);
	sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
	if (player == NULL)
	{
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	process_player_call(env, thiz, player->goToSleep(), NULL, "DVBTPlayer_goToSleep failed.");
	return 0;
}

static jint DVBTPlayer_setFrequency(JNIEnv *env, jobject thiz, jint channel)
{
	ALOGV("%s", __func__);
	sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
	if (player == NULL)
	{
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}
	process_player_call(env, thiz, player->setFrequency(channel), NULL, "DVBTPlayer_setFrequency failed.");
	return 0;
}

static jint DVBTPlayer_setVolume(JNIEnv *env, jobject thiz, jint audioID, jint audioVolume)
{
	ALOGV("%s", __func__);
	sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
	if (player == NULL)
	{
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	process_player_call(env, thiz, player->setVolume(audioID, audioVolume), NULL, "DVBTPlayer_setVolume failed.");
	return 0;
}

static jint DVBTPlayer_onOutputAudioDescription(JNIEnv *env, jobject thiz, jboolean isOnAudioDescription)
{
    ALOGV("DVBTPlayer_setPlayPause");
    sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
		 return -1;
    }

    process_player_call(env, thiz, player->onAudioDescription(isOnAudioDescription), NULL, "onOutputAudioDescription failed.");
	return 0;
}

static jobject DVBTPlayer_getRFInformation(JNIEnv *env, jobject thiz)
{
	ALOGV("%s", __func__);
	sp<DVBTPlayer> player = getDVBTPlayer(env, thiz);
	if (player == NULL)
	{
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return NULL;
	}

	unsigned char *pucData;
	unsigned int uiSize = 0;
	if (player->getRFInformation(&pucData, &uiSize) == 0)
	{
		if (uiSize != 0)
		{
			jobject jParcel = createJavaParcelObject(env);
			if (jParcel != NULL) {
				Parcel* nativeParcel = parcelForJavaObject(env, jParcel);
				nativeParcel->setData(pucData, uiSize);
			}
			return jParcel;
		}
	}
	return NULL;
}

static jstring DVBTPlayer_property_get(JNIEnv *env, jobject thiz, jstring keyJ)
{
    int len;
    const char* key;
    char buf[PROPERTY_VALUE_MAX];
    jstring rvJ = NULL;

    if (keyJ == NULL) {
        jniThrowNullPointerException(env, "key must not be null.");
        goto error;
    }

    key = env->GetStringUTFChars(keyJ, NULL);

    len = property_get(key, buf, "");
    if (len >= 0) {
        rvJ = env->NewStringUTF(buf);
    } else {
        rvJ = env->NewStringUTF("");
    }

    env->ReleaseStringUTFChars(keyJ, key);

error:
    return rvJ;
}

static void DVBTPlayer_property_set(JNIEnv *env, jobject thiz, jstring keyJ, jstring valJ)
{
    int err;
    const char* key;
    const char* val;

    if (keyJ == NULL) {
        jniThrowNullPointerException(env, "key must not be null.");
        return ;
    }
    key = env->GetStringUTFChars(keyJ, NULL);

    if (valJ == NULL) {
        val = "";       /* NULL pointer not allowed here */
    } else {
        val = env->GetStringUTFChars(valJ, NULL);
    }

    err = property_set(key, val);

    env->ReleaseStringUTFChars(keyJ, key);

    if (valJ != NULL) {
        env->ReleaseStringUTFChars(valJ, val);
    }

    if (err < 0) {
        jniThrowException(env, "java/lang/RuntimeException",
                          "failed to set system property");
    }
}

static void DVBTPlayer_native_setup(JNIEnv *env, jobject thiz, jobject weak_this)
{
    ALOGV("DVBTPlayer_native_setup");
    sp<DVBTPlayer> player = new DVBTPlayer();
    if (player == NULL) {
        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
        return;
    }

    // create new listener
    sp<JNIDVBTPlayerListener> listener = new JNIDVBTPlayerListener(env, thiz, weak_this);
    player->setListener(listener);

    setDVBTPlayer(env, thiz, player);
}

static void DVBTPlayer_native_finalize(JNIEnv *env, jobject thiz)
{
    ALOGV("native_finalize");
    DVBTPlayer_release(env, thiz);
}

static const char *classPathName = "com/telechips/android/dvb/player/dvb/DVBTPlayer";

static JNINativeMethod methods[] = {
  { "prepare", "(Ljava/lang/String;)V", (void*)DVBTPlayer_prepare },
  { "_start", "(I)V", (void*)DVBTPlayer_start },
  { "_stop", "()V", (void*)DVBTPlayer_stop },
  { "setTuneSpace", "(Lcom/telechips/android/dvb/player/dvb/TuneSpace;)I", (void*)DVBTPlayer_setTuneSpace },
  { "getSignalStrength", "()I", (void*)DVBTPlayer_getSignalStrength },
  { "getSignalState", "(Lcom/telechips/android/dvb/player/dvb/SignalState;)I", (void*)DVBTPlayer_getSignalState },
  { "setAntennaPower", "(I)V", (void*)DVBTPlayer_setAntennaPower },
  { "SetLnbVoltage", "(I)I", (void*)DVBTPlayer_SetLnbVoltage },
  { "SetTone", "(I)I", (void*)DVBTPlayer_SetTone },
  { "DiseqcSendBurst", "(I)I", (void*)DVBTPlayer_DiseqcSendBurst },
  { "DiseqcSendCMD", "([B)I", (void*)DVBTPlayer_DiseqcSendCMD },
  { "BlindScanReset", "()I", (void*)DVBTPlayer_BlindScanReset },
  { "BlindScanStart", "()I", (void*)DVBTPlayer_BlindScanStart },
  { "BlindScanCancel", "()I", (void*)DVBTPlayer_BlindScanCancel },
  { "BlindScanGetState", "()I", (void*)DVBTPlayer_BlindScanGetState },
  { "BlindScanGetInfo", "()Landroid/os/Parcel;", (void*)DVBTPlayer_BlindScanGetInfo },
  { "_release", "()V", (void*)DVBTPlayer_release },
  { "search", "(I)V", (void*)DVBTPlayer_search },
  { "manualSearch", "(II)V", (void*)DVBTPlayer_manualSearch },
  { "searchCancel", "()V", (void*)DVBTPlayer_stop_search },
  { "releaseSurface", "()V", (void*)DVBTPlayer_releaseSurface },
  { "setSurface", "()V", (void*)DVBTPlayer_setSurface },
  { "useSurface", "(I)I", (void*)DVBTPlayer_useSurface },
  { "setChannel", "(III)V", (void*)DVBTPlayer_setChannel },
  { "setCountryCode", "(I)V", (void*)DVBTPlayer_setCountryCode },
  { "playSubtitle", "(II)I", (void*)DVBTPlayer_playSubtitle },
  { "playTeletext", "(I)I", (void*)DVBTPlayer_playTeletext },
  { "setTeletext_UpdatePage", "(I)I", (void*)DVBTPlayer_setTeletext_UpdatePage },
  { "setTeletext_CachePage_Insert", "(I)I", (void*)DVBTPlayer_setTeletext_CachePage_Insert },
  { "setTeletext_CachePage_Delete", "(I)I", (void*)DVBTPlayer_setTeletext_CachePage_Delete },
  { "setCapture", "(Ljava/lang/String;)I", (void*)DVBTPlayer_setCapture },
  { "setRecord", "(Ljava/lang/String;)I", (void*)DVBTPlayer_setRecord },
  { "setRecStop", "()I", (void*)DVBTPlayer_setRecStop },
  { "setPlay", "(Ljava/lang/String;)I", (void*)DVBTPlayer_setPlay },
  { "setPlayStop", "()I", (void*)DVBTPlayer_setPlayStop },
  { "setSeekTo", "(I)I", (void*)DVBTPlayer_setSeekTo },
  { "setPlaySpeed", "(I)I", (void*)DVBTPlayer_setPlaySpeed },
  { "setPlayPause", "(Z)V", (void*)DVBTPlayer_setPlayPause },
  { "setUserLocalTimeOffset", "(ZI)V", (void*)DVBTPlayer_setUserLocalTimeOffset },
  { "getDuration", "()I", (void*)DVBTPlayer_getDuration },
  { "getCurrentPosition", "()I", (void*)DVBTPlayer_getCurrentPosition },
  { "getCurrentReadPosition", "()I", (void*)DVBTPlayer_getCurrentReadPosition },
  { "getCurrentWritePosition", "()I", (void*)DVBTPlayer_getCurrentWritePosition },
  { "getDiskFSType", "()Ljava/lang/String;", (void*)DVBTPlayer_getDiskFSType },
  { "getCurrentDateTime", "(Lcom/telechips/android/dvb/player/dvb/DateTimeData;)I", (void*)DVBTPlayer_getCurrentDateTime },
  { "requestEPGUpdate", "(I)I", (void*)DVBTPlayer_requestEPGUpdate },
  { "setAudio", "(I)I", (void*)DVBTPlayer_setAudio },
  { "setStereo", "(I)I", (void*)DVBTPlayer_setStereo },
  { "goToSleep", "()I", (void*)DVBTPlayer_goToSleep },
  { "setFrequency", "(I)I", (void*)DVBTPlayer_setFrequency },
  { "setVolume", "(II)I", (void*)DVBTPlayer_setVolume },
  { "onOutputAudioDescription", "(Z)I", (void*)DVBTPlayer_onOutputAudioDescription},
  { "getRFInformation", "()Landroid/os/Parcel;", (void*)DVBTPlayer_getRFInformation },
  { "property_get", "(Ljava/lang/String;)Ljava/lang/String;", (void*) DVBTPlayer_property_get },
  { "property_set", "(Ljava/lang/String;Ljava/lang/String;)V", (void*) DVBTPlayer_property_set },
  { "native_setup", "(Ljava/lang/Object;)V", (void*)DVBTPlayer_native_setup },
  { "native_finalize", "()V", (void*)DVBTPlayer_native_finalize }
};

// setDisplayEnable, setDisplayDisable +
// setLCDUpdate +
// setDisplayPosition -

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

