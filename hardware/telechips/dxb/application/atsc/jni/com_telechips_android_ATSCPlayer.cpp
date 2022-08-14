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
#define LOG_TAG	"ATSCPlayer-JNI"
#include <utils/Log.h>

#include <stdio.h>
#include <assert.h>

#include <jni.h>
#include <JNIHelp.h>
#include <android_runtime/AndroidRuntime.h>
#include <android_runtime/android_view_Surface.h>
#include <utils/Errors.h>

#include "player/ATSCPlayer.h"

using namespace android;

struct fields_t {
    jfieldID	context;
    jfieldID	surface;
    jmethodID	post_event;
};
static fields_t fields;

static Mutex sLock;

class JNIATSCPlayerListener : public ATSCPlayerListener {
public:
	JNIATSCPlayerListener(JNIEnv *env, jobject thiz, jobject weak_thiz);
	~JNIATSCPlayerListener();
	void notify(int msg, int ext1, int ext2, void *obj);

private:
	JNIATSCPlayerListener();
	jclass mClass;
	jobject mObject;
};

JNIATSCPlayerListener::JNIATSCPlayerListener(JNIEnv *env, jobject thiz, jobject weak_thiz)
{
	jclass clazz = env->GetObjectClass(thiz);
	if(clazz == NULL) {
		ALOGE("Can't find ATSCPlayer");
		jniThrowException(env, "java/lang/Exception", NULL);
		return;
	}
	mClass = (jclass) env->NewGlobalRef(clazz);
	mObject = env->NewGlobalRef(weak_thiz);
	env->DeleteLocalRef(clazz);
}

JNIATSCPlayerListener::~JNIATSCPlayerListener()
{
	JNIEnv *env = AndroidRuntime::getJNIEnv();
	env->DeleteGlobalRef(mObject);
	env->DeleteGlobalRef(mClass);
}

void JNIATSCPlayerListener::notify(int msg, int ext1, int ext2, void *obj)
{
	JNIEnv *env = AndroidRuntime::getJNIEnv();
	switch (msg) {
		case  ATSCPlayer::EVENT_SUBTITLE_UPDATE:
		{
			jfieldID fid;
			jint size;
			jmethodID mid = NULL;
			jobject jsubobj = NULL, jobj = NULL;
			jclass clazz = NULL;
			
			SubtitleData* subt	= (SubtitleData*)obj;

			mid = env->GetMethodID(mClass, "getSubtitleDataObject", "()Lcom/telechips/android/atsc/SubtitleData;");
			if(mid == NULL) break;

			jsubobj = env->NewObject(mClass, mid);
			if(jsubobj == NULL){
				ALOGE("[%s] NewObject fail\n", __func__);
				break;
			}

			jobj = env->CallObjectMethod(jsubobj, mid);
			if(jobj == NULL){
				ALOGE("[%s] CallObjectMethod fail\n", __func__);
				if(jsubobj) env->DeleteLocalRef(jsubobj);
				break;
			}
			
			clazz = env->GetObjectClass(jobj);
			if(clazz == NULL){
				ALOGE("[%s] GetObjectClass fail\n", __func__);
				if(jobj) env->DeleteLocalRef(jobj);
				if(jsubobj) env->DeleteLocalRef(jsubobj);
				break;
			}

			fid = env->GetFieldID(clazz, "region_id", "I");
			env->SetIntField(jobj, fid, subt->region_id);

			if( subt->region_id >= 0 )
			{
				fid = env->GetFieldID(clazz, "x", "I");
				env->SetIntField(jobj, fid, subt->x);

				fid = env->GetFieldID(clazz, "y", "I");
				env->SetIntField(jobj, fid, subt->y);

				fid = env->GetFieldID(clazz, "width", "I");
				env->SetIntField(jobj, fid, subt->width);

				fid = env->GetFieldID(clazz, "height", "I");
				env->SetIntField(jobj, fid, subt->height);

				size = subt->width*subt->height;
				if(size == 0){
					ALOGE("[%s] size fail\n", __func__);
					if(clazz) env->DeleteLocalRef(clazz);
					if(jobj) env->DeleteLocalRef(jobj);
					if(jsubobj) env->DeleteLocalRef(jsubobj);
					break;
				}
				jintArray bmpBuf = env->NewIntArray(size);
				if (bmpBuf == NULL){
					ALOGE("[%s] bmpBuf alloc fail\n", __func__);
					if(clazz) env->DeleteLocalRef(clazz);
					if(jobj) env->DeleteLocalRef(jobj);
					if(jsubobj) env->DeleteLocalRef(jsubobj);
					break;
				}
				fid = env->GetFieldID(clazz, "data", "[I");
				if(bmpBuf == NULL) {
					ALOGE("Couldn't allocate int array for subt data");
					env->ExceptionClear();
				}else {
					env->SetIntArrayRegion(bmpBuf, 0, size, (jint*)subt->data);

					env->SetObjectField(jobj, fid, bmpBuf);
					env->DeleteLocalRef(bmpBuf);
					
					if( subt->data != NULL ){
						free(subt->data);
						subt->data = NULL;
					}
				}
				
				env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, jobj);

				if(clazz) env->DeleteLocalRef(clazz);
				if(jobj) env->DeleteLocalRef(jobj);
				if(jsubobj) env->DeleteLocalRef(jsubobj);
			}
			else
			{
				fid = env->GetFieldID(clazz, "x", "I");
				env->SetIntField(jobj, fid, subt->x);

				fid = env->GetFieldID(clazz, "y", "I");
				env->SetIntField(jobj, fid, subt->y);

				fid = env->GetFieldID(clazz, "width", "I");
				env->SetIntField(jobj, fid, subt->width);

				fid = env->GetFieldID(clazz, "height", "I");
				env->SetIntField(jobj, fid, subt->height);

				fid = env->GetFieldID(clazz, "data", "[I");
				env->SetObjectField(jobj, fid, NULL);

				env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, jobj);
				
				if(clazz) env->DeleteLocalRef(clazz);
				if(jobj) env->DeleteLocalRef(jobj);
				if(jsubobj) env->DeleteLocalRef(jsubobj);
			}
		}
		break;
		
		default:
			env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, 0);
		break;
    }
}

static sp<ATSCPlayer> getATSCPlayer(JNIEnv *env, jobject thiz)
{
	Mutex::Autolock l(sLock);
	ATSCPlayer *const p = (ATSCPlayer *)env->GetIntField(thiz, fields.context);
	return sp<ATSCPlayer>(p);
}

static sp<ATSCPlayer> setATSCPlayer(JNIEnv *env, jobject thiz, const sp<ATSCPlayer>& player)
{
	Mutex::Autolock l(sLock);
	sp<ATSCPlayer> old = (ATSCPlayer *) env->GetIntField(thiz, fields.context);
	if(player != NULL && player.get()) {
		player->incStrong(thiz);
	}
	if(old != 0) {
		old->decStrong(thiz);
	}
	env->SetIntField(thiz, fields.context, (int)((player != NULL) ? player.get() : NULL));
	return old;
}

static void process_player_call(JNIEnv *env, jobject thiz, status_t opStatus, const char *exception, const char *message)
{
	if (exception == NULL) {
		if (opStatus != (status_t) OK) {
			sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
			if(player != 0)
				player->notify(ATSCPlayer::EVENT_ERROR, opStatus, 0, 0);
		}
	} else {
		// Throw exception
		if (opStatus != (status_t) OK) {
			jniThrowException(env, exception, message);
		}
	}
}

static void setVideoSurface(const sp<ATSCPlayer>& player, JNIEnv *env, jobject thiz)
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

static void ATSCPlayer_prepare(JNIEnv *env, jobject thiz, jstring DB_Path)
{
	ALOGV("prepare");

	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if(player == NULL)
	{
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return;
	}

	char strBuff[128];		
	const char *sz = env->GetStringUTFChars(DB_Path, 0);
	
	strcpy(strBuff, sz);
	env->ReleaseStringUTFChars(DB_Path, sz);
	
	process_player_call(env, thiz, player->prepare(strBuff), "java/io/IOException", "Prepare failed.");
}

static void ATSCPlayer_start(JNIEnv *env, jobject thiz, jint country_code)
{
	ALOGV("start");
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return;
	}
	process_player_call(env, thiz, player->start(country_code), NULL, NULL);
}

static void ATSCPlayer_stop(JNIEnv *env, jobject thiz)
{
	ALOGV("stop");
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return;
	}
	process_player_call(env, thiz, player->stop(), NULL, NULL);
}

static jint ATSCPlayer_getVideoWidth(JNIEnv *env, jobject thiz)
{
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
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

static jint ATSCPlayer_getVideoHeight(JNIEnv *env, jobject thiz)
{
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
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

static jint ATSCPlayer_getSignalStrength(JNIEnv *env, jobject thiz)
{
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return 0;
	}
	unsigned int LV = 0;
	if (0 != player->getSignalStrength(&LV)) {
		ALOGE("getSNRStrength failed");
		LV  = 0;
	}
	ALOGV("getSNRStrength : %d", LV);
	return LV;
}

static jboolean ATSCPlayer_isPlaying(JNIEnv *env, jobject thiz)
{
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return false;
	}
	const jboolean is_playing = player->isPlaying();
	ALOGV("isPlaying: %d", is_playing);
	return is_playing;
}

static void ATSCPlayer_release(JNIEnv *env, jobject thiz)
{
	ALOGV("release");
	sp<ATSCPlayer> player = setATSCPlayer(env, thiz, 0);
	if (player != NULL) {
		player->release();
		player->setListener(0);
	}
}

static void ATSCPlayer_search(JNIEnv *env, jobject thiz, jint countryCode, jint modulationFlag)
{
	ALOGV("ATSCPlayer_search: countryCode = %d modulationFlag = 0x%x", countryCode, modulationFlag);
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return;
	}
	status_t opStatus = player->search(countryCode, modulationFlag);
	process_player_call(env, thiz, opStatus, NULL, NULL);
}

static void ATSCPlayer_manualSearch(JNIEnv *env, jobject thiz, jint countryCode, jint modulationFlag, jint frequencyKhz, jint deleteDB)
{
	ALOGV("ATSCPlayer_manualSearch: countryCode = %d modulationFlag = 0x%x frequencyKhz = %d deleteDB = %d", 
		countryCode, modulationFlag, frequencyKhz, deleteDB);
	
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);

	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return;
	}
	status_t opStatus = player->manualSearch(countryCode, modulationFlag, frequencyKhz, deleteDB);
	process_player_call(env, thiz, opStatus, NULL, NULL);
}

static void ATSCPlayer_stop_search(JNIEnv *env, jobject thiz)
{
	ALOGV("ATSCPlayer_stop_search");
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return;
	}
	process_player_call(env, thiz, player->stopsearch(), NULL, NULL);
}

static void ATSCPlayer_releaseSurface(JNIEnv *env, jobject thiz)
{
	ALOGV("%s", __func__);
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return;
	}
	process_player_call(env, thiz, player->releaseSurface(), NULL, NULL);
}

static void ATSCPlayer_setSurface(JNIEnv *env, jobject thiz)
{

	ALOGV("%s", __func__);
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return;
	}
	setVideoSurface(player, env, thiz);
}

static jint ATSCPlayer_useSurface(JNIEnv *env, jobject thiz, jint arg)
{
	ALOGV("%s", __func__);
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}
	process_player_call(env, thiz, player->useSurface(arg), NULL, NULL);
	return 0;
}

static void ATSCPlayer_setChannel(JNIEnv *env, jobject thiz, jint rowID)
{
	ALOGV("setChannel");
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return;
	}
	status_t opStatus = player->setChannel(rowID);
	process_player_call(env, thiz, opStatus, NULL, NULL);  
}
	
static jint ATSCPlayer_getCountryCode(JNIEnv *env, jobject thiz)
{
	ALOGV("getCountryCode");
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}
	int countryCode = player->getCountryCode();
	return countryCode;
}

static void ATSCPlayer_setCountryCode(JNIEnv *env, jobject thiz, jint countryCode)
{
	ALOGV("setCountryCode");
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return;
	}
	process_player_call(env, thiz, player->setCountryCode(countryCode), NULL, "setCountryCode failed.");
}

static void ATSCPlayer_setVolume(JNIEnv *env, jobject thiz, jfloat leftVolume, jfloat rightVolume)
{
	ALOGV("setVolume");
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return;
	}
	process_player_call(env, thiz, player->setVolume(leftVolume, rightVolume), NULL, "setVolume failed.");
}

static void ATSCPlayer_setDisplayPosition(JNIEnv *env, jobject thiz, jint x, jint y, jint width, jint height, jint rotate)
{
	ALOGV("setDisplayPosition");
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return;
	}
	process_player_call(env, thiz, player->setDisplayPosition(x, y, width, height, rotate), NULL, "setDisplayPosition failed.");
}

static jint ATSCPlayer_setCapture(JNIEnv *env, jobject thiz, jstring filePath)
{
	ALOGV("setCapture");
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
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

static jint ATSCPlayer_setRecord(JNIEnv *env, jobject thiz, jstring filePath)
{
	ALOGV("setRecord");
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
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

static jint ATSCPlayer_setRecStop(JNIEnv *env, jobject thiz)
{
	ALOGV("setRecStop");
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	process_player_call(env, thiz, player->setRecStop(), NULL, "setRecStop failed.");

	return 0;
}

static jint ATSCPlayer_setLCDUpdate(JNIEnv *env, jobject thiz)
{
	ALOGV("ATSCPlayer_setLCDUpdate");
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	process_player_call(env, thiz, player->setLCDUpdate(), NULL, "setLCDUpdate failed.");

	return 0;
}

static jint ATSCPlayer_playSubtitle(JNIEnv *env, jobject thiz, jint onoff)
{
	ALOGV("%s",__func__);
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	process_player_call(env, thiz, player->playSubtitle(onoff), NULL, "ATSCPlayer_playSubtitle failed.");
	return 0;
}

static void ATSCPlayer_setDisplayEnable(JNIEnv *env, jobject thiz)
{
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return;
	}
	process_player_call(env, thiz, player->setDisplayEnable(), NULL, NULL);
}

static void ATSCPlayer_setDisplayDisable(JNIEnv *env, jobject thiz)
{
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return;
	}
	process_player_call(env, thiz, player->setDisplayDisable(), NULL, NULL);
}

static jint ATSCPlayer_enableCC(JNIEnv *env, jobject thiz, jint enabledisable)
{
	ALOGV("enableCC : %d", enabledisable);
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	process_player_call(env, thiz, player->enableCC(enabledisable), NULL, "ATSCPlayer_enableCC failed.");
	return 0;
}

static jint ATSCPlayer_setCCServiceNum(JNIEnv *env, jobject thiz, jint CCServiceNum)
{
	ALOGV("setCCServiceNum : %d", CCServiceNum);
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	process_player_call(env, thiz, player->setCCServiceNum(CCServiceNum), NULL, "ATSCPlayer_setCCServiceNum failed.");
	return 0;
}

static jint ATSCPlayer_setAudioLanguage(JNIEnv *env, jobject thiz, jint AudioLanguage)
{
	ALOGV("setAudioLanguage : %d", AudioLanguage);
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	process_player_call(env, thiz, player->setAudioLanguage(AudioLanguage), NULL, "ATSCPlayer_setAudioLanguage failed.");
	return 0;
}

static jint ATSCPlayer_setAspectRatio(JNIEnv *env, jobject thiz, jint AspectRatio)
{
	ALOGV("setAspectRatio : %d", AspectRatio);
	sp<ATSCPlayer> player = getATSCPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	process_player_call(env, thiz, player->setAspectRatio(AspectRatio), NULL, "ATSCPlayer_setAspectRatio failed.");
	return 0;
}

static void ATSCPlayer_native_setup(JNIEnv *env, jobject thiz, jobject weak_this)
{
	ALOGV("ATSCPlayer_native_setup");
	sp<ATSCPlayer> player = new ATSCPlayer();
	if (player == NULL) {
		jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
		return;
	}

	// create new listener
	sp<JNIATSCPlayerListener> listener = new JNIATSCPlayerListener(env, thiz, weak_this);
	player->setListener(listener);
	
	setATSCPlayer(env, thiz, player);
}

static void ATSCPlayer_native_finalize(JNIEnv *env, jobject thiz)
{
	ALOGV("native_finalize");
	ATSCPlayer_release(env, thiz);
}

static const char *classPathName = "com/telechips/android/atsc/ATSCPlayer";

static JNINativeMethod methods[] = {
	{ "prepare", "(Ljava/lang/String;)V", (void*)ATSCPlayer_prepare },
	{ "_start", "(I)V", (void*)ATSCPlayer_start },
	{ "_stop", "()V", (void*)ATSCPlayer_stop },
	{ "getVideoWidth", "()I", (void*)ATSCPlayer_getVideoWidth },
	{ "getVideoHeight", "()I", (void*)ATSCPlayer_getVideoHeight },
	{ "getSignalStrength", "()I", (void*)ATSCPlayer_getSignalStrength },
	{ "isPlaying", "()Z", (void*)ATSCPlayer_isPlaying },
	{ "_release", "()V", (void*)ATSCPlayer_release },
	{ "search", "(II)V", (void*)ATSCPlayer_search }, 
	{ "manualSearch", "(IIII)V", (void*)ATSCPlayer_manualSearch }, 
	{ "searchCancel", "()V", (void*)ATSCPlayer_stop_search },
	{ "releaseSurface", "()V", (void*)ATSCPlayer_releaseSurface },
	{ "setSurface", "()V", (void*)ATSCPlayer_setSurface },
	{ "useSurface", "(I)I", (void*)ATSCPlayer_useSurface },
	{ "setChannel", "(I)V", (void*)ATSCPlayer_setChannel },
	{ "getCountryCode", "()I", (void*)ATSCPlayer_getCountryCode },
	{ "setCountryCode", "(I)V", (void*)ATSCPlayer_setCountryCode },
	{ "setVolume", "(FF)V", (void*)ATSCPlayer_setVolume },
	{ "playSubtitle", "(I)I", (void*)ATSCPlayer_playSubtitle },
	{ "setCapture", "(Ljava/lang/String;)I", (void*)ATSCPlayer_setCapture },
	{ "setRecord", "(Ljava/lang/String;)I", (void*)ATSCPlayer_setRecord },
	{ "setRecStop", "()I", (void*)ATSCPlayer_setRecStop },
	{ "setLCDUpdate", "()I", (void*)ATSCPlayer_setLCDUpdate },
	{ "setDisplayEnable", "()V", (void*)ATSCPlayer_setDisplayEnable },
	{ "setDisplayDisable", "()V", (void*)ATSCPlayer_setDisplayDisable },
	{ "enableCC", "(I)I", (void*)ATSCPlayer_enableCC},
	{ "setCCServiceNum", "(I)I", (void*)ATSCPlayer_setCCServiceNum},	
	{ "setAudioLanguage", "(I)I", (void*)ATSCPlayer_setAudioLanguage},	
	{ "setAspectRatio", "(I)I", (void*)ATSCPlayer_setAspectRatio},	
	{ "native_setup", "(Ljava/lang/Object;)V", (void*)ATSCPlayer_native_setup }, 
	{ "native_finalize", "()V", (void*)ATSCPlayer_native_finalize }
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
static int registerNatives(JNIEnv *env)
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
jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
	JNIEnv *env = NULL;
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
