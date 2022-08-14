/*
 * Copyright (C) 2013 The Android Open Source Project
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
#define LOG_TAG	"TDMBPlayer-JNI"
#include <utils/Log.h>

#include <stdio.h>
#include <assert.h>

#include <jni.h>
#include <JNIHelp.h>
#include <android_runtime/AndroidRuntime.h>
#include <android_runtime/android_view_Surface.h>
#include <utils/Errors.h>
#include <cutils/properties.h>

#include "player/TDMBPlayer.h"
#include <DMBPlayerClient.h>

using namespace android;

struct fields_t {
    jfieldID	context;
    jfieldID	surface;
    jmethodID	post_event;
};
static fields_t fields;

static Mutex sLock;
pthread_mutex_t		lockTDMBRecording;

int freqList[64] = { 0 };

class JNITDMBPlayerListener : public DxbPlayerClientListener {
public:
    JNITDMBPlayerListener(JNIEnv *env, jobject thiz, jobject weak_thiz);
    ~JNITDMBPlayerListener();
    virtual void notify(int msg, int ext1, int ext2, const Parcel *obj = NULL);

private:
    JNITDMBPlayerListener();
    jclass mClass;
    jobject mObject;
};

JNITDMBPlayerListener::JNITDMBPlayerListener(JNIEnv *env, jobject thiz, jobject weak_thiz)
{
    jclass clazz = env->GetObjectClass(thiz);
    if (clazz == NULL) {
        ALOGE("Can't find TDMBPlayer");
        jniThrowException(env, "java/lang/Exception", NULL);
        return;
    }
    mClass = (jclass) env->NewGlobalRef(clazz);
    mObject = env->NewGlobalRef(weak_thiz);
    env->DeleteLocalRef(clazz);
}

JNITDMBPlayerListener::~JNITDMBPlayerListener()
{
    JNIEnv *env = AndroidRuntime::getJNIEnv();
    env->DeleteGlobalRef(mObject);
    env->DeleteGlobalRef(mClass);
}

void JNITDMBPlayerListener::notify(int msg, int ext1, int ext2, const Parcel *obj)
{
    JNIEnv *env = AndroidRuntime::getJNIEnv();
    switch (msg) {
    case TDMBPlayer::EVENT_CHANNEL_UPDATE:
    {
            TDMB_INFO *ch = (TDMB_INFO*) obj->data();
		
        jmethodID mid = env->GetMethodID(mClass, "getChannelObject", "()Lcom/telechips/android/tdmb/player/Channel;");
        jobject jchobj = env->NewObject(mClass, mid);
        jobject jobj = env->CallObjectMethod(jchobj, mid);
	jclass clazz = env->GetObjectClass(jobj);

        jfieldID fid = env->GetFieldID(clazz, "ensembleName", "Ljava/lang/String;");
        jstring jstr = env->NewStringUTF(ch->Ensemble_Label);
        if( jstr ) {
            env->SetObjectField(jobj, fid, jstr);
            env->DeleteLocalRef(jstr);
        }

        fid = env->GetFieldID(clazz, "ensembleID", "I");
        env->SetIntField(jobj, fid, ch->Ensemble_Id);

        fid = env->GetFieldID(clazz, "ensembleFreq", "I");
        env->SetIntField(jobj, fid, ch->Ensemble_Freq);
 
        fid = env->GetFieldID(clazz, "serviceName", "Ljava/lang/String;");
        jstr = env->NewStringUTF(ch->Service_Label);
        if( jstr ) {
            env->SetObjectField(jobj, fid, jstr);
            env->DeleteLocalRef(jstr);
        }

        fid = env->GetFieldID(clazz, "serviceID", "I");
        env->SetIntField(jobj, fid, ch->Service_Id);

        fid = env->GetFieldID(clazz, "channelName", "Ljava/lang/String;");
        jstr = env->NewStringUTF(ch->Channel_Label);
        if( jstr ) {
            env->SetObjectField(jobj, fid, jstr);
            env->DeleteLocalRef(jstr);
        }

        fid = env->GetFieldID(clazz, "channelID", "I");
        env->SetIntField(jobj, fid, ch->Channel_Id);

        fid = env->GetFieldID(clazz, "type", "I");
        env->SetIntField(jobj, fid, ch->Channel_Type);

        fid = env->GetFieldID(clazz, "bitrate", "I");
        env->SetIntField(jobj, fid, ch->Channel_BitRate);

        jintArray iarr = env->NewIntArray(7);
        int tempbuf[7];
        for (int i = 0; i < 7; i++) {
            tempbuf[i] = ch->Reg[i];
        }
        env->SetIntArrayRegion(iarr, 0, 7, tempbuf);
        fid = env->GetFieldID(clazz, "reg", "[I");
        env->SetObjectField(jobj, fid, iarr);
        env->DeleteLocalRef(iarr);
        env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, jobj);	// fields.post_event -> java Event call(postEventFromNative)

	env->DeleteLocalRef(clazz);
	env->DeleteLocalRef(jobj);
        env->DeleteLocalRef(jchobj);
        break;
    }

    case TDMBPlayer::EVENT_DLSDATA_UPDATE:
    {
        int *piEventData = (int *)obj->data();
        jstring jstr;
        DABDLSData DabDlsData, *pDabDlsData;
		pDabDlsData = &DabDlsData;
        DabDlsData.dlsDataType = *(piEventData++);
        DabDlsData.dlsDataSize = *(piEventData++);
        memcpy(DabDlsData.dlsData, (char*)piEventData, DabDlsData.dlsDataSize);

        jmethodID mid = env->GetMethodID(mClass, "getDABDLSDataObject", "()Lcom/telechips/android/tdmb/player/DABDLSData;");

        jobject jdabobj = env->NewObject(mClass, mid);
        jobject jobj = env->CallObjectMethod(jdabobj, mid);

        jclass clazz = env->GetObjectClass(jobj);
        int strSize = pDabDlsData->dlsDataSize;
        jfieldID fid = env->GetFieldID(clazz, "strDLSData", "Ljava/lang/String;");
        fid = env->GetFieldID(clazz, "strDLSData", "Ljava/lang/String;");
        if( pDabDlsData->dlsDataType == 4 ) {	// UNICODE
            uint16_t uDLSData[128];
            strSize = pDabDlsData->dlsDataSize/2;
            memcpy(uDLSData, pDabDlsData->dlsData, pDabDlsData->dlsDataSize);
            jstr = env->NewString(uDLSData, strSize);
            if( jstr ) {
                env->SetObjectField(jobj, fid, jstr);
                env->DeleteLocalRef(jstr);
            }
        }
        else {
            jbyteArray RetArray = env->NewByteArray(pDabDlsData->dlsDataSize);
            if (RetArray == NULL) {
                ALOGE("Couldn't allocate byte array for dls data");
                env->ExceptionClear();
            }
            else {
                fid = env->GetFieldID(clazz, "dlsData", "[B");
                env->SetByteArrayRegion(RetArray, 0, pDabDlsData->dlsDataSize, (jbyte*)pDabDlsData->dlsData);
                env->SetObjectField(jobj, fid, RetArray);
            }

            if (RetArray) {
                env->DeleteLocalRef(RetArray);
            }
        }

        fid = env->GetFieldID(clazz, "dlsDataSize", "I");
        env->SetIntField(jobj, fid, strSize);

        fid = env->GetFieldID(clazz, "dlsDataType", "I");
        env->SetIntField(jobj, fid, pDabDlsData->dlsDataType);

        env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, jobj);

        env->DeleteLocalRef(clazz);
        env->DeleteLocalRef(jobj);
		env->DeleteLocalRef(jdabobj);

        break;
    }

    case TDMBPlayer::EVENT_EWS_UPDATE:
    {
        int *piEventData = (int *)obj->data();
        jmethodID mid = env->GetMethodID(mClass, "getEWSDataObject", "()Lcom/telechips/android/tdmb/player/EWSData;");
        jobject jewsobj = env->NewObject(mClass, mid);
        jobject jobj = env->CallObjectMethod(jewsobj, mid);
        jclass clazz = env->GetObjectClass(jobj);
        jfieldID fid;
        int messageID = *(piEventData++);
        int messageLen = *(piEventData++);
        unsigned char * messageData = (unsigned char *)(piEventData);

        jbyteArray messageArray = env->NewByteArray(messageLen);
        if (messageArray == NULL) {
            ALOGE("Couldn't allocate byte array for ews data");
            env->ExceptionClear();
        }
        else {
            fid = env->GetFieldID(clazz, "EWSMessage", "[B");
            env->SetByteArrayRegion(messageArray, 0, messageLen, (jbyte*)messageData);
            env->SetObjectField(jobj, fid, messageArray);
        }

        if (messageArray) {
            env->DeleteLocalRef(messageArray);
        }

        fid = env->GetFieldID(clazz, "EWSMessageID", "I");
        env->SetIntField(jobj, fid, messageID);

        fid = env->GetFieldID(clazz, "EWSMessageLen", "I");
        env->SetIntField(jobj, fid, messageLen);

        env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, jobj);

        env->DeleteLocalRef(clazz);
        env->DeleteLocalRef(jobj);
            env->DeleteLocalRef(jewsobj);

        //delete [] messageData;

        break;
    }

    case TDMBPlayer::EVENT_DATA_UPDATE:
    {
        int *piEventData = (int *)obj->data();
        int dataSize = *(piEventData++);
        unsigned char *pDataMsg = (unsigned char *)piEventData;

        //ALOGI("TDMBPlayer::EVENT_DATA_UPDATE::%d", dataSize);
        break;
    }

    default:
        env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, 0);		// fields.post_event -> java Event call(postEventFromNative)
        break;
    }
}

static sp<DMBPlayerClient> getTDMBPlayer(JNIEnv *env, jobject thiz)
{
    Mutex::Autolock l(sLock);
    DMBPlayerClient *const p = (DMBPlayerClient *) env->GetIntField(thiz, fields.context);
    return sp<DMBPlayerClient>(p);
}

static sp<DMBPlayerClient> setTDMBPlayer(JNIEnv *env, jobject thiz, const sp<DMBPlayerClient>& player)
{
    ALOGV("setTDMBPlayer");
    Mutex::Autolock l(sLock);
    sp<DMBPlayerClient> old = (DMBPlayerClient *) env->GetIntField(thiz, fields.context);
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
    ALOGV("process_player_call");
    if (exception == NULL) {
        if (opStatus != (status_t) OK) {
            sp<DMBPlayerClient> player = getTDMBPlayer(env, thiz);
            if (player != 0)
                player->notify(TDMBPlayer::EVENT_ERROR, opStatus, 0, 0);
        }
    } else {
        // Throw exception
        if (opStatus != (status_t) OK) {
            jniThrowException(env, exception, message);
        }
    }
}

static void setVideoSurface(const sp<DMBPlayerClient>& player, JNIEnv *env, jobject thiz)
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

static void TDMBPlayer_prepare(JNIEnv *env, jobject thiz, jint basebandType, jstring dbPath)
{
    ALOGV("TDMBPlayer_prepare");
    sp<DMBPlayerClient> player = getTDMBPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    const char *sz = NULL;

    if(dbPath != NULL)
        sz = env->GetStringUTFChars(dbPath, 0) ;
	
    process_player_call(env, thiz, player->playprepare(basebandType, sz), "java/io/IOException", "Prepare failed.");

    if(dbPath != NULL)
        env->ReleaseStringUTFChars(dbPath, sz) ;
}

static void TDMBPlayer_start(JNIEnv *env, jobject thiz, jint country_code)
{
    ALOGV("TDMBPlayer_start");
    sp<DMBPlayerClient> player = getTDMBPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->start(country_code), NULL, NULL);
}

static void TDMBPlayer_stop(JNIEnv *env, jobject thiz)
{
    ALOGV("TDMBPlayer_stop");
    sp<DMBPlayerClient> player = getTDMBPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->playstop(), NULL, NULL);
}

/*
  0 : antenna level
  1 : snr value
  2 : pcber value
  3 : rssi value
  4 : vber value
*/
const int fieldCount = 14;
int iSignalStrength[fieldCount] = { 0 };

static jintArray TDMBPlayer_getSignalStrength(JNIEnv *env, jobject thiz)
{
    sp<DMBPlayerClient> player = getTDMBPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }

    if (0 != player->getSignalStrength(iSignalStrength)) {
        ALOGE("getSignalStrength failed");
    }

	jintArray ret = NULL;
	ret = env->NewIntArray(fieldCount);
	if (ret != NULL) {
		jint* a2 = (jint*)env->GetPrimitiveArrayCritical(ret, 0);
		if (a2) {
			memcpy(a2, iSignalStrength, sizeof(int)*fieldCount);
			env->ReleasePrimitiveArrayCritical(ret, a2, 0);
		}
	}
	return ret;
}

static void TDMBPlayer_release(JNIEnv *env, jobject thiz)
{
    ALOGV("TDMBPlayer_release");
    sp<DMBPlayerClient> player = setTDMBPlayer(env, thiz, 0);
    if (player != NULL) {
        player->setListener(0);
        player->disconnect();
    }
}

static void TDMBPlayer_search(JNIEnv *env, jobject thiz, jint countryCode, jintArray jfreqList, jint freqListCount)
{
    ALOGV("TDMBPlayer_search : code:%d", countryCode);
    sp<DMBPlayerClient> player = getTDMBPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
	memset(freqList, 0x00, sizeof(int)*64);
	if(freqListCount>0) {
	    env->GetIntArrayRegion(jfreqList, 0, freqListCount, freqList);
	}
    process_player_call(env, thiz, player->search(countryCode, freqList, freqListCount), NULL, NULL);
}

static void TDMBPlayer_stop_search(JNIEnv *env, jobject thiz)
{
    ALOGV("TDMBPlayer_stop_search");
    sp<DMBPlayerClient> player = getTDMBPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->stopsearch(), NULL, NULL);
}

static void TDMBPlayer_manual_search(JNIEnv *env, jobject thiz, jint freq)
{
    ALOGV("TDMBPlayer_manual_search : frequency:%d", freq);
    sp<DMBPlayerClient> player = getTDMBPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->manual_search(freq), NULL, NULL);
}


static void TDMBPlayer_releaseSurface(JNIEnv *env, jobject thiz)
{
    ALOGV("%s", __func__);
    sp<DMBPlayerClient> player = getTDMBPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->releaseSurface(), NULL, NULL);
}

static jint TDMBPlayer_useSurface(JNIEnv *env, jobject thiz, jint arg)
{
    ALOGV("%s", __func__);
    sp<DMBPlayerClient> player = getTDMBPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return -1;
    }
    process_player_call(env, thiz, player->useSurface(arg), NULL, NULL);
    return 0;
}

static void TDMBPlayer_setSurface(JNIEnv *env, jobject thiz)
{

    ALOGV("%s", __func__);
    sp<DMBPlayerClient> player = getTDMBPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    setVideoSurface(player, env, thiz);
}

static void TDMBPlayer_setChannelIndex(JNIEnv *env, jobject thiz, jint channelIndex, jint type)
{
    ALOGV("TDMBPlayer_setChannelIndex");
    sp<DMBPlayerClient> player = getTDMBPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->setChannelIndex(channelIndex, type, 0), NULL, "setChannelIndex failed.");
}

static void TDMBPlayer_setManualChannel(JNIEnv *env, jobject thiz, jintArray channelInfo)
{
    ALOGV("TDMBPlayer_setManualChannel");
    sp<DMBPlayerClient> player = getTDMBPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    TDMB_INFO chInfo;
    jint *ji_buf = env->GetIntArrayElements(channelInfo, NULL);
    chInfo.moduleidx = ji_buf[0];
    chInfo.Ensemble_Freq = ji_buf[1];
    chInfo.Ensemble_Id = ji_buf[2];
    chInfo.Service_Id = ji_buf[3];
    chInfo.Channel_Id = ji_buf[4];
    chInfo.Channel_Type = ji_buf[5];
    chInfo.Channel_BitRate = ji_buf[6];
    for(int i = 0; i < 7; i++) {
        chInfo.Reg[i] = ji_buf[7 + i];
    }
    env->ReleaseIntArrayElements(channelInfo, ji_buf, 0);
    process_player_call(env, thiz, player->setManualChannel((unsigned char*)&chInfo, sizeof(TDMB_INFO)), NULL, "setChannelIndex failed.");
}

static jint TDMBPlayer_setCapture(JNIEnv *env, jobject thiz, jstring filePath)
{
    ALOGV("TDMBPlayer_setCapture");
    sp<DMBPlayerClient> player = getTDMBPlayer(env, thiz);
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

static jint TDMBPlayer_setRecord(JNIEnv *env, jobject thiz, jstring filePath)
{
    ALOGV("TDMBPlayer_setRecord");
    sp<DMBPlayerClient> player = getTDMBPlayer(env, thiz);
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

static jint TDMBPlayer_setRecStop(JNIEnv *env, jobject thiz)
{
    ALOGV("TDMBPlayer_setRecStop");
    sp<DMBPlayerClient> player = getTDMBPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->setRecStop(), NULL, "setRecStop failed.");

    return 0;
}

static jint TDMBPlayer_setLCDUpdate(JNIEnv *env, jobject thiz)
{
    ALOGV("TDMBPlayer_setLCDUpdate");
    sp<DMBPlayerClient> player = getTDMBPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->setLCDUpdate(), NULL, "setLCDUpdate failed.");

    return 0;
}

static void TDMBPlayer_native_setup(JNIEnv *env, jobject thiz, jobject weak_this)
{
    ALOGV("TDMBPlayer_native_setup");
    sp<DMBPlayerClient> player = new DMBPlayerClient();
    if (player == NULL) {
        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
        return;
    }

    // create new listener
    sp<JNITDMBPlayerListener> listener = new JNITDMBPlayerListener(env, thiz, weak_this);
    player->setListener(listener);

    setTDMBPlayer(env, thiz, player);
}

static void TDMBPlayer_native_finalize(JNIEnv *env, jobject thiz)
{
    ALOGV("TDMBPlayer_native_finalize");
    TDMBPlayer_release(env, thiz);
}

static jint TDMBPlayer_setBBModuleIndex(JNIEnv *env, jobject thiz, jint bbidx, jint moduleidx)
{
    ALOGD("TDMBPlayer_setBBModuleIndex::%d::%d", bbidx, moduleidx);
    sp<DMBPlayerClient> player = getTDMBPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }
    player->setBBModuleIndex(bbidx, moduleidx);

	return 0;
}

static void TDMBPlayer_setDisplayEnable(JNIEnv *env, jobject thiz)
{
    ALOGV("TDMBPlayer_setDisplayEnable");
    sp<DMBPlayerClient> player = getTDMBPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->setDisplayEnable(), NULL, NULL);
}

static void TDMBPlayer_setDisplayDisable(JNIEnv *env, jobject thiz)
{
    ALOGV("TDMBPlayer_setDisplayDisable");
    sp<DMBPlayerClient> player = getTDMBPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->setDisplayDisable(), NULL, NULL);
}

static void setAudioMute(JNIEnv *env, jobject thiz, jboolean isMute)
{
    ALOGV("TDMBPlayer_setAudioMute");
    sp<DMBPlayerClient> player = getTDMBPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->setAudioMute(isMute), NULL, NULL);
}

static const char *classPathName = "com/telechips/android/tdmb/player/TDMBPlayer";

static JNINativeMethod methods[] = {
  { "prepare", "(ILjava/lang/String;)V", (void*)TDMBPlayer_prepare },
  { "_start", "(I)V", (void*)TDMBPlayer_start },
  { "_stop", "()V", (void*)TDMBPlayer_stop },
  { "getSignalStrength", "()[I", (void*)TDMBPlayer_getSignalStrength },
  { "_release", "()V", (void*)TDMBPlayer_release },
  { "search", "(I[II)V", (void*)TDMBPlayer_search },
  { "searchCancel", "()V", (void*)TDMBPlayer_stop_search },
  { "manual_search", "(I)V", (void*)TDMBPlayer_manual_search },
  { "releaseSurface", "()V", (void*)TDMBPlayer_releaseSurface },
  { "setSurface", "()V", (void*)TDMBPlayer_setSurface },
  { "useSurface", "(I)I", (void*)TDMBPlayer_useSurface },
  { "setChannelIndex", "(II)V", (void*)TDMBPlayer_setChannelIndex },
  { "manual_setChannel", "([I)V", (void*)TDMBPlayer_setManualChannel },
  { "setCapture", "(Ljava/lang/String;)I", (void*)TDMBPlayer_setCapture },
  { "setRecord", "(Ljava/lang/String;)I", (void*)TDMBPlayer_setRecord },
  { "setRecStop", "()I", (void*)TDMBPlayer_setRecStop },
  { "setLCDUpdate", "()I", (void*)TDMBPlayer_setLCDUpdate },
  { "native_setup", "(Ljava/lang/Object;)V", (void*)TDMBPlayer_native_setup },
  { "native_finalize", "()V", (void*)TDMBPlayer_native_finalize },
  { "setBBModuleIndex", "(II)I", (void*)TDMBPlayer_setBBModuleIndex },
  { "setDisplayEnable", "()V", (void*)TDMBPlayer_setDisplayEnable },
  { "setDisplayDisable", "()V", (void*)TDMBPlayer_setDisplayDisable },
  { "setAudioMute", "(Z)V", (void*)setAudioMute },
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
