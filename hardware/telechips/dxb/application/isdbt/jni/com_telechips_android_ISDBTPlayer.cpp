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
#define LOG_TAG	"ISDBTPlayer-JNI"
#include <utils/Log.h>

#include <stdio.h>
#include <assert.h>

#include <jni.h>
#include <JNIHelp.h>
#include <android_runtime/AndroidRuntime.h>
#include <android_runtime/android_view_Surface.h>
#include <utils/Errors.h>
#include <cutils/properties.h>

#include "player/ISDBTPlayer.h"

#include<android/bitmap.h>
#include<stdlib.h>
#include<subtitle_main.h>
#include<subtitle_display.h>

using namespace android;

struct fields_t {
    jfieldID	context;
    jfieldID	surface;
    jmethodID	post_event;
};
static fields_t fields;

static Mutex sLock;

class JNIISDBTPlayerListener : public ISDBTPlayerListener {
public:
    JNIISDBTPlayerListener(JNIEnv *env, jobject thiz, jobject weak_thiz);
    ~JNIISDBTPlayerListener();
    void notify(int msg, int ext1, int ext2, void *obj);

private:
    JNIISDBTPlayerListener();
    jclass mClass;
    jobject mObject;
};

JNIISDBTPlayerListener::JNIISDBTPlayerListener(JNIEnv *env, jobject thiz, jobject weak_thiz)
{
    jclass clazz = env->GetObjectClass(thiz);
    if (clazz == NULL) {
        ALOGE("Can't find ISDBTPlayer");
        jniThrowException(env, "java/lang/Exception", NULL);
        return;
    }
    mClass = (jclass) env->NewGlobalRef(clazz);
    mObject = env->NewGlobalRef(weak_thiz);
	env->DeleteLocalRef(clazz);
}

JNIISDBTPlayerListener::~JNIISDBTPlayerListener()
{
    JNIEnv *env = AndroidRuntime::getJNIEnv();
    env->DeleteGlobalRef(mObject);
    env->DeleteGlobalRef(mClass);
}

void JNIISDBTPlayerListener::notify(int msg, int ext1, int ext2, void *obj)
{
    JNIEnv *env = AndroidRuntime::getJNIEnv();
    switch (msg) {
	case ISDBTPlayer::EVENT_SMARTCARDINFO_UPDATE:
	{
	       SCInfo *piEventData = (SCInfo *)obj;
	        jmethodID mid = env->GetMethodID(mClass, "getSCInfoDataObject", "()Lcom/telechips/android/isdbt/player/isdbt/SCInfoData;");
		jobject jsubobj = env->NewObject(mClass, mid);
		jobject jobj = env->CallObjectMethod(jsubobj, mid);
	        jclass clazz = env->GetObjectClass(jobj);
	        jfieldID fid;

#if 0
		ALOGE("piEventData->SCDataSize = %d\n", piEventData->SCDataSize);
		int i=0;
		for(i=0; i<piEventData->SCDataSize; i++)
			ALOGE("piEventData->SCData[%d] = %x \n", i, *(piEventData->SCData+i));
#endif

		jbyteArray scinfo_Array = env->NewByteArray(piEventData->SCDataSize);
		if (scinfo_Array == NULL) {
		    ALOGE("Couldn't allocate byte array for scinfo_data");
		    env->ExceptionClear();
		}
		else {
	            fid = env->GetFieldID(clazz, "SCData", "[B");
	            env->SetByteArrayRegion(scinfo_Array, 0, piEventData->SCDataSize, (jbyte*)piEventData->SCData);
	            env->SetObjectField(jobj, fid, scinfo_Array);
		}

		if (scinfo_Array) {
		    env->DeleteLocalRef(scinfo_Array);
		}

		fid = env->GetFieldID(clazz, "SCDataSize", "I");
		env->SetIntField(jobj, fid, piEventData->SCDataSize);

		env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, jobj);

		env->DeleteLocalRef(clazz);
		env->DeleteLocalRef(jobj);
		env->DeleteLocalRef(jsubobj);
	        break;
	}

	case ISDBTPlayer::EVENT_SUBTITLE_UPDATE:
	{
		jfieldID fid;
		jint size;
		jmethodID mid = NULL;
		jobject jsubobj = NULL, jobj = NULL;
		jclass clazz = NULL;
		
		SubtitleData* subt	= (SubtitleData*) obj;

		mid = env->GetMethodID(mClass, "getSubtitleDataObject", "()Lcom/telechips/android/isdbt/player/isdbt/SubtitleData;");
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

		fid = env->GetFieldID(clazz, "subtitle_update", "I");
		env->SetIntField(jobj, fid, subt->subtitle_update);		

		env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, jobj);

		if(clazz) env->DeleteLocalRef(clazz);
		if(jobj) env->DeleteLocalRef(jobj);
		if(jsubobj) env->DeleteLocalRef(jsubobj);
		break;
	}

	case ISDBTPlayer::EVENT_SUPERIMPOSE_UPDATE:
	{
		jfieldID fid;
		jint size;
		jmethodID mid = NULL;
		jobject jsuperobj = NULL, jobj = NULL;
		jclass clazz = NULL;
		
		SuperimposeData* superim	= (SuperimposeData*) obj;

		mid = env->GetMethodID(mClass, "getSubtitleDataObject", "()Lcom/telechips/android/isdbt/player/isdbt/SubtitleData;");
		if(mid == NULL) break;

		jsuperobj = env->NewObject(mClass, mid);
		if(jsuperobj == NULL){
			ALOGE("[%s] NewObject fail\n", __func__);
			break;
		}

		jobj = env->CallObjectMethod(jsuperobj, mid);
		if(jobj == NULL){
			ALOGE("[%s] CallObjectMethod fail\n", __func__);
			if(jsuperobj) env->DeleteLocalRef(jsuperobj);
			break;
		}
		
		clazz = env->GetObjectClass(jobj);
		if(clazz == NULL){
			ALOGE("[%s] GetObjectClass fail\n", __func__);
			if(jobj) env->DeleteLocalRef(jobj);
			if(jsuperobj) env->DeleteLocalRef(jsuperobj);
			break;
		}

		fid = env->GetFieldID(clazz, "superimpose_update", "I");
		env->SetIntField(jobj, fid, superim->superimpose_update);		

		env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, jobj);

		if(clazz) env->DeleteLocalRef(clazz);
		if(jobj) env->DeleteLocalRef(jobj);
		if(jsuperobj) env->DeleteLocalRef(jsuperobj);
		break;
	}

	case ISDBTPlayer::EVENT_PNG_UPDATE:
	{
		jfieldID fid;
		jint size;
		jmethodID mid = NULL;
		jobject jsuperobj = NULL, jobj = NULL;
		jclass clazz = NULL;
		
		SuperimposeData* png	= (SuperimposeData*) obj;

		mid = env->GetMethodID(mClass, "getSubtitleDataObject", "()Lcom/telechips/android/isdbt/player/isdbt/SubtitleData;");
		if(mid == NULL) break;

		jsuperobj = env->NewObject(mClass, mid);
		if(jsuperobj == NULL){
			ALOGE("[%s] NewObject fail\n", __func__);
			break;
		}

		jobj = env->CallObjectMethod(jsuperobj, mid);
		if(jobj == NULL){
			ALOGE("[%s] CallObjectMethod fail\n", __func__);
			if(jsuperobj) env->DeleteLocalRef(jsuperobj);
			break;
		}
		
		clazz = env->GetObjectClass(jobj);
		if(clazz == NULL){
			ALOGE("[%s] GetObjectClass fail\n", __func__);
			if(jobj) env->DeleteLocalRef(jobj);
			if(jsuperobj) env->DeleteLocalRef(jsuperobj);
			break;
		}

		fid = env->GetFieldID(clazz, "png_update", "I");
		env->SetIntField(jobj, fid, png->png_update);		

		env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, jobj);

		if(clazz) env->DeleteLocalRef(clazz);
		if(jobj) env->DeleteLocalRef(jobj);
		if(jsuperobj) env->DeleteLocalRef(jsuperobj);
		break;
	}

	case ISDBTPlayer::EVENT_VIDEODEFINITION_UPDATE:
	{
		VideoDefinitionData* video_definition	= (VideoDefinitionData*) obj;

		jmethodID mid = env->GetMethodID(mClass, "getVideoDefinitionDataObject", "()Lcom/telechips/android/isdbt/player/isdbt/VideoDefinitionData;");

		jobject jvideoobj = env->NewObject(mClass, mid);
		jobject jobj      = env->CallObjectMethod(jvideoobj, mid);
		jclass  clazz	  = env->GetObjectClass(jobj);

		jfieldID fid;


			// width
			fid = env->GetFieldID(clazz, "width", "I");
			env->SetIntField(jobj, fid, video_definition->width);

			// height
			fid = env->GetFieldID(clazz, "height", "I");
			env->SetIntField(jobj, fid, video_definition->height);

			// aspect ratio
			fid = env->GetFieldID(clazz, "aspect_ratio", "I");
			env->SetIntField(jobj, fid, video_definition->aspect_ratio);

			// frame rate
			fid = env->GetFieldID(clazz, "frame_rate", "I");
			env->SetIntField(jobj, fid, video_definition->frame_rate);

			//solution for screen mode change bug
		
			// main_DecoderID
			fid = env->GetFieldID(clazz, "main_DecoderID", "I");
			env->SetIntField(jobj, fid, video_definition->main_DecoderID);
		
		       // sub_width
			fid = env->GetFieldID(clazz, "sub_width", "I");
			env->SetIntField(jobj, fid, video_definition->sub_width);

			// sub_height
			fid = env->GetFieldID(clazz, "sub_height", "I");
			env->SetIntField(jobj, fid, video_definition->sub_height);

			// sub_DecoderID
			fid = env->GetFieldID(clazz, "sub_DecoderID", "I");
			env->SetIntField(jobj, fid, video_definition->sub_DecoderID);

			// sub_aspect ratio
			fid = env->GetFieldID(clazz, "sub_aspect_ratio", "I");
			env->SetIntField(jobj, fid, video_definition->sub_aspect_ratio);
		

			// DisplayID
			fid = env->GetFieldID(clazz, "DisplayID", "I");
			env->SetIntField(jobj, fid, video_definition->DisplayID);


		
		env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, jobj);

		env->DeleteLocalRef(clazz);
		env->DeleteLocalRef(jobj);
		
		env->DeleteLocalRef(jvideoobj);
		break;
	}

	case ISDBTPlayer::EVENT_EMERGENCY_INFO:
	{
		EWSData* ewsdata = (EWSData*)obj;
		jmethodID mid;
		jobject jewsobj;
		jobject jobj;
		jclass clazz;
		jfieldID fid;
		jintArray svc;

		mid = env->GetMethodID(mClass, "getEWSDataObject", "()Lcom/telechips/android/isdbt/player/isdbt/EWSData;");
		if(mid == NULL) break;

		jewsobj = env->NewObject(mClass, mid);
		if(jewsobj == NULL){
			ALOGE("[%s] NewObject fail\n", __func__);
			break;
		}

		jobj = env->CallObjectMethod(jewsobj, mid);
		if(jobj == NULL){
			ALOGE("[%s] CallObjectMethod fail\n", __func__);
			if(jewsobj) env->DeleteLocalRef(jewsobj);
			break;
		}

		clazz = env->GetObjectClass(jobj);
		if(clazz == NULL){
			ALOGE("[%s] GetObjectClass fail\n", __func__);
			if(jobj) env->DeleteLocalRef(jobj);
			if(jewsobj) env->DeleteLocalRef(jewsobj);
			break;
		}

		fid = env->GetFieldID(clazz, "ServiceID", "I");
		env->SetIntField(jobj, fid, ewsdata->serviceID);

		fid = env->GetFieldID(clazz, "StartEndFlag", "I");
		env->SetIntField(jobj, fid, ewsdata->startendflag);

		fid = env->GetFieldID(clazz, "SignalType", "I");
		env->SetIntField(jobj, fid, ewsdata->signaltype);

		fid = env->GetFieldID(clazz, "LocalCodeLength", "I");
		env->SetIntField(jobj, fid, ewsdata->area_code_length);

		svc = env->NewIntArray(256);
		if(svc == NULL) {
			env->SetIntField(jobj, fid, 0);
		} else {
			env->SetIntArrayRegion (svc, 0, 256, (jint*)ewsdata->localcode);
			fid = env->GetFieldID(clazz, "LocalCode", "[I");
			env->SetObjectField(jobj, fid, svc);
			env->DeleteLocalRef(svc);
		}

		env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, jobj);

		env->DeleteLocalRef(clazz);
		env->DeleteLocalRef(jobj);
		
		env->DeleteLocalRef(jewsobj);

		break;
	}
		case ISDBTPlayer::EVENT_DBINFO_UPDATE: {
			DBInfoData *pDBInfoData = (DBInfoData *)obj;
			
			int i, codeLength;
			jmethodID	mid;
			jobject   jclazzobj;
			jobject   jdbobj;
			jclass    jdbclazz;
			jfieldID  fieldID;
			jstring   jstr;
			jbyteArray	jbarr;

			//Channel DB
			{				
				mid = env->GetMethodID(mClass, "getDBChannelDataObject", "()Lcom/telechips/android/isdbt/player/isdbt/DB_CHANNEL_Data;");
				jclazzobj = env->NewObject(mClass, mid);
				jdbobj = env->CallObjectMethod(jclazzobj, mid);
				jdbclazz = env->GetObjectClass(jdbobj);

				fieldID = env->GetFieldID(jdbclazz, "channelNumber", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbCh.channelNumber);
				fieldID = env->GetFieldID(jdbclazz, "countryCode", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbCh.countryCode);
				fieldID = env->GetFieldID(jdbclazz, "audioPID", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbCh.audioPID);
				fieldID = env->GetFieldID(jdbclazz, "videoPID", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbCh.videoPID);
				fieldID = env->GetFieldID(jdbclazz, "stPID", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbCh.stPID);
				fieldID = env->GetFieldID(jdbclazz, "siPID", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbCh.siPID);
				fieldID = env->GetFieldID(jdbclazz, "PMT_PID", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbCh.PMT_PID);
				fieldID = env->GetFieldID(jdbclazz, "remoconID", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbCh.remoconID);
				fieldID = env->GetFieldID(jdbclazz, "serviceType", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbCh.serviceType);
				fieldID = env->GetFieldID(jdbclazz, "serviceID", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbCh.serviceID);
				fieldID = env->GetFieldID(jdbclazz, "regionID", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbCh.regionID);
				fieldID = env->GetFieldID(jdbclazz, "threedigitNumber", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbCh.threedigitNumber);
				fieldID = env->GetFieldID(jdbclazz, "TStreamID", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbCh.TStreamID);
				fieldID = env->GetFieldID(jdbclazz, "berAVG", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbCh.berAVG);
				fieldID = env->GetFieldID(jdbclazz, "preset", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbCh.preset);
				fieldID = env->GetFieldID(jdbclazz, "networkID", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbCh.networkID);
				fieldID = env->GetFieldID(jdbclazz, "areaCode", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbCh.areaCode);

			#if 0
				jbarr = env->NewByteArray(32);
				fieldID = env->GetFieldID(jdbclazz, "channelName", "[B");
				if (jbarr == NULL) {
					ALOGE("Couldn't allocate byte array for name data");
					env->ExceptionClear();
				} else {
					env->SetByteArrayRegion(jbarr, 0, 32, (jbyte*)pDBInfoData->dbCh.channelName);
					env->SetObjectField(jdbobj, fieldID, jbarr);
					env->DeleteLocalRef(jbarr);
				}
			#endif /* 0 */

				env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, 0, ext2, jdbobj);

				env->DeleteLocalRef(jdbclazz);
				env->DeleteLocalRef(jdbobj);
				env->DeleteLocalRef(jclazzobj);
			}

			// ServiceDB
			{
				mid = env->GetMethodID(mClass, "getDBServiceDataObject", "()Lcom/telechips/android/isdbt/player/isdbt/DB_SERVICE_Data;");
				jclazzobj = env->NewObject(mClass, mid);
				jdbobj = env->CallObjectMethod(jclazzobj, mid);
				jdbclazz = env->GetObjectClass(jdbobj);

				fieldID = env->GetFieldID(jdbclazz, "uiPCRPID", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbService.uiPCRPID);
				
				env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, 1, ext2, jdbobj);

				env->DeleteLocalRef(jdbclazz);
				env->DeleteLocalRef(jdbobj);
				env->DeleteLocalRef(jclazzobj);
			}

			// Audio DB
			for(i=0; i<5; i++)
			{
				mid = env->GetMethodID(mClass, "getDBAudioDataObject", "()Lcom/telechips/android/isdbt/player/isdbt/DB_AUDIO_Data;");
				jclazzobj = env->NewObject(mClass, mid);
				jdbobj	  = env->CallObjectMethod(jclazzobj, mid);
				jdbclazz  = env->GetObjectClass(jdbobj);

				fieldID = env->GetFieldID(jdbclazz, "uiServiceID", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbAudio[i].uiServiceID);
				fieldID = env->GetFieldID(jdbclazz, "uiCurrentChannelNumber", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbAudio[i].uiCurrentChannelNumber);
				fieldID = env->GetFieldID(jdbclazz, "uiCurrentCountryCode", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbAudio[i].uiCurrentCountryCode);
				fieldID = env->GetFieldID(jdbclazz, "uiAudioPID", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbAudio[i].uiAudioPID);
				fieldID = env->GetFieldID(jdbclazz, "uiAudioIsScrambling", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbAudio[i].uiAudioIsScrambling);
				fieldID = env->GetFieldID(jdbclazz, "uiAudioStreamType", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbAudio[i].uiAudioStreamType);
				fieldID = env->GetFieldID(jdbclazz, "uiAudioType", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbAudio[i].uiAudioType);

			#if 1
				codeLength = strlen(pDBInfoData->dbAudio[i].pucacLangCode);
				jbarr = env->NewByteArray(codeLength);
				fieldID = env->GetFieldID(jdbclazz, "ucacLangCode", "[B");
				if(jbarr == NULL)
				{
					ALOGE("Couldn't allocate byte array for name data");
					env->ExceptionClear();
				}
				else
				{
					env->SetByteArrayRegion(jbarr, 0, codeLength, (jbyte*)pDBInfoData->dbAudio[i].pucacLangCode);
					env->SetObjectField(jdbobj, fieldID, jbarr);
					env->DeleteLocalRef(jbarr);
				}
			#else
				if(strlen(pDBInfoData->dbAudio[i].pucacLangCode) > 0) {
					fieldID = env->GetFieldID(jdbclazz, "stracLangCode", "Ljava/lang/String;");
					jstr = env->NewString((uint16_t*)pDBInfoData->dbAudio[i].pucacLangCode, strlen(pDBInfoData->dbAudio[i].pucacLangCode)/2);
					if( jstr ) {
						env->SetObjectField(jdbobj, fieldID, jstr);
						env->DeleteLocalRef(jstr);
					}
				}
			#endif

				env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, 2, ext2, jdbobj);

				env->DeleteLocalRef(jdbclazz);
				env->DeleteLocalRef(jdbobj);
				env->DeleteLocalRef(jclazzobj);
			}
			// Subtitle DB
/*
			for(i=0; i<10; i++)
			{
				mid = env->GetMethodID(mClass, "getDBSubtitleDataObject", "()Lcom/telechips/android/isdbt/player/isdbt/DB_SUBTITLE_Data;");
				jclazzobj = env->NewObject(mClass, mid);
				jdbobj	  = env->CallObjectMethod(jclazzobj, mid);
				jdbclazz  = env->GetObjectClass(jdbobj);

				fieldID = env->GetFieldID(jdbclazz, "uiServiceID", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbSubtitle[i].uiServiceID);
				fieldID = env->GetFieldID(jdbclazz, "uiCurrentChannelNumber", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbSubtitle[i].uiCurrentChannelNumber);
				fieldID = env->GetFieldID(jdbclazz, "uiCurrentCountryCode", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbSubtitle[i].uiCurrentCountryCode);
				fieldID = env->GetFieldID(jdbclazz, "uiSubtitlePID", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbSubtitle[i].uiSubtitlePID);

			#if 1
				codeLength = strlen(pDBInfoData->dbSubtitle[i].pucacSubtLangCode);
				jbarr = env->NewByteArray(codeLength);
				fieldID = env->GetFieldID(jdbclazz, "ucacSubtLangCode", "[B");
				if(jbarr == NULL)
				{
					ALOGE("Couldn't allocate byte array for name data");
					env->ExceptionClear();
				}
				else
				{
					env->SetByteArrayRegion(jbarr, 0, codeLength, (jbyte*)pDBInfoData->dbSubtitle[i].pucacSubtLangCode);
					env->SetObjectField(jdbobj, fieldID, jbarr);
					env->DeleteLocalRef(jbarr);
				}
			#else
				if(strlen(pDBInfoData->dbSubtitle[i].pucacSubtLangCode) > 0) {
					fieldID = env->GetFieldID(jdbclazz, "stracSubtLangCode", "Ljava/lang/String;");
					jstr = env->NewString((uint16_t*)pDBInfoData->dbSubtitle[i].pucacSubtLangCode, strlen(pDBInfoData->dbSubtitle[i].pucacSubtLangCode)/2);
					if( jstr ) {
						env->SetObjectField(jdbobj, fieldID, jstr);
						env->DeleteLocalRef(jstr);
					}
				}
			#endif

				fieldID = env->GetFieldID(jdbclazz, "uiSubtType", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbSubtitle[i].uiSubtType);
				fieldID = env->GetFieldID(jdbclazz, "uiCompositionPageID", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbSubtitle[i].uiCompositionPageID);
				fieldID = env->GetFieldID(jdbclazz, "uiAncillaryPageID", "I");
				env->SetIntField(jdbobj, fieldID, pDBInfoData->dbSubtitle[i].uiAncillaryPageID);

				env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, 3, ext2, jdbobj);

				env->DeleteLocalRef(jdbclazz);
				env->DeleteLocalRef(jdbobj);
				env->DeleteLocalRef(jclazzobj);
			}
*/
			break;
		}
		
		case ISDBTPlayer::EVENT_DEBUGMODE_UPDATE: {
			jmethodID	mid	= env->GetMethodID(mClass, "getDebugModeDataObject", "()Lcom/telechips/android/isdbt/player/isdbt/DebugModeData;");

			jobject jdebugobj	= env->NewObject(mClass, mid);
			jobject jobj		= env->CallObjectMethod(jdebugobj, mid);
			jclass  clazz		= env->GetObjectClass(jobj);

			jfieldID fid;
			jbyteArray	barr;

			//	index
			fid	= env->GetFieldID(clazz, "iIndex", "I");
			env->SetIntField(jobj, fid, ext1);

			//	length
			fid	= env->GetFieldID(clazz, "iLength", "I");
			env->SetIntField(jobj, fid, ext2);

			//	message
			barr	= env->NewByteArray(200);
			fid	= env->GetFieldID(clazz, "msg", "[B");
			if(barr == NULL)
			{
				ALOGE("Couldn't allocate byte array for dls data");
				env->ExceptionClear();
			}
			else
			{
				if(obj != NULL)
				{
					env->SetByteArrayRegion(barr, 0, 200, (jbyte*)obj);
				}
				else
				{
					env->SetByteArrayRegion(barr, 0, 200, (jbyte*)"");
				}
				env->SetObjectField(jobj, fid, barr);

				env->DeleteLocalRef(barr);
			}


			env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, jobj);

			env->DeleteLocalRef(clazz);
			env->DeleteLocalRef(jobj);

			env->DeleteLocalRef(jdebugobj);

			break;
		}

	case ISDBTPlayer::EVENT_AUTOSEARCH_DONE:
		{
			// void *obj			
			//[0] = status  -1=failure, 1=success
			//[1] = rowID of channel DB for primary fullseg service in success
			//[2] = rowID of channel DB for primary 1seg service in success
			//[3] = total count for services
			//[4~35] = rowID of channel DB for all found services

			int *param = (int *)obj;

			jmethodID mid;
			jobject jsubobj;
			jobject jobj;
			jclass clazz;
			jfieldID fid;
			jintArray svc;

			mid = env->GetMethodID(mClass, "getAutoSearchInfoDataObject", "()Lcom/telechips/android/isdbt/player/isdbt/AutoSearchInfoData;");
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

			if (param == NULL) {
				fid = env->GetFieldID(clazz, "mAutoSearchStatus", "I");
				env->SetIntField(jobj, fid, -1);
			} else {
				fid = env->GetFieldID(clazz, "mAutoSearchStatus", "I");
				env->SetIntField(jobj, fid, *param++);

				fid = env->GetFieldID(clazz, "mAutoSearchFullSeg", "I");
				env->SetIntField(jobj, fid, *param++);

				fid = env->GetFieldID(clazz, "mAutoSearchOneSeg", "I");
				env->SetIntField(jobj, fid, *param++);

				fid = env->GetFieldID(clazz, "mAutoSearchTotalSvc", "I");
				env->SetIntField(jobj, fid, *param++);

				svc = env->NewIntArray(32);
				if(svc == NULL) {
					env->SetIntField(jobj, fid, 0);
				} else {
					env->SetIntArrayRegion (svc, 0, 32, (jint*)param);
					fid = env->GetFieldID(clazz, "mAutoSearchSvc", "[I");
					env->SetObjectField(jobj, fid, svc);
					env->DeleteLocalRef(svc);
				}
			}

			env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, jobj);
			//ext1 = status, ext2=0

			env->DeleteLocalRef(clazz);
			env->DeleteLocalRef(jobj);
			env->DeleteLocalRef(jsubobj);
		}
		break;
	default:
		env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, 0);		// fields.post_event -> java Event call(postEventFromNative)
	break;
	}
}

static sp<ISDBTPlayer> getISDBTPlayer(JNIEnv *env, jobject thiz)
{
    Mutex::Autolock l(sLock);
    ISDBTPlayer *const p = (ISDBTPlayer *) env->GetIntField(thiz, fields.context);
    return sp<ISDBTPlayer>(p);
}

static sp<ISDBTPlayer> setISDBTPlayer(JNIEnv *env, jobject thiz, const sp<ISDBTPlayer>& player)
{
    Mutex::Autolock l(sLock);
    sp<ISDBTPlayer> old = (ISDBTPlayer *) env->GetIntField(thiz, fields.context);
    if (player.get()) {
        player->incStrong(thiz);
    }
    if (old != 0) {
        old->decStrong(thiz);
    }
    env->SetIntField(thiz, fields.context, (int) player.get());
    return old;
}

static void process_player_call(JNIEnv *env, jobject thiz, status_t opStatus, const char *exception, const char *message)
{
    if (exception == NULL) {
        if (opStatus != (status_t) OK) {
            sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
            if (player != 0)
                player->notify(ISDBTPlayer::EVENT_ERROR, opStatus, 0, 0);
        }
    } else {
        // Throw exception
        if (opStatus != (status_t) OK) {
            jniThrowException(env, exception, message);
        }
    }
}

static void ISDBTPlayer_drawSubtitle(JNIEnv *env, jobject thiz, jobject stbitmap)
{	
	SUB_SYS_INFO_TYPE *pInfo = (SUB_SYS_INFO_TYPE*)get_sub_context();
	
	//ALOGI("--->ISDBTPlayer_drawSubtitle");
	AndroidBitmapInfo  SubBitmapinfo;
	int ret;

	if ((ret = AndroidBitmap_getInfo(env, stbitmap, &SubBitmapinfo)) < 0) {
		ALOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		return;
	}

	if ((ret = AndroidBitmap_lockPixels(env, stbitmap, &pInfo->subtitle_pixels)) < 0) {
		ALOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
		return;
	}

	memset(pInfo->subtitle_pixels, 0x0, sizeof(int)*pInfo->subtitle_size);
	subtitle_data_memcpy();
 	AndroidBitmap_unlockPixels(env, stbitmap);
}

static void ISDBTPlayer_drawSuperimpose(JNIEnv *env, jobject thiz, jobject sibitmap)
{
	SUB_SYS_INFO_TYPE *pInfo = (SUB_SYS_INFO_TYPE*)get_sub_context();
	
	//ALOGI("ISDBTPlayer_drawSuperimpose");
	AndroidBitmapInfo  SuperBitmapinfo;
	int ret;

	if ((ret = AndroidBitmap_getInfo(env, sibitmap, &SuperBitmapinfo)) < 0) {
		ALOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		return;
	}

	if ((ret = AndroidBitmap_lockPixels(env, sibitmap, &pInfo->superimpose_pixels)) < 0) {
		ALOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
		return;
	}

	memset(pInfo->superimpose_pixels, 0x0, sizeof(int)*pInfo->superimpose_size);
	superimpose_data_memcpy();
	AndroidBitmap_unlockPixels(env, sibitmap);
}

static void ISDBTPlayer_drawPng(JNIEnv *env, jobject thiz, jobject pngbitmap)
{
	SUB_SYS_INFO_TYPE *pInfo = (SUB_SYS_INFO_TYPE*)get_sub_context();
	
	//ALOGI("ISDBTPlayer_drawSuperimpose");
	AndroidBitmapInfo  PngBitmapinfo;
	int ret;

	if ((ret = AndroidBitmap_getInfo(env, pngbitmap, &PngBitmapinfo)) < 0) {
		ALOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		return;
	}

	if ((ret = AndroidBitmap_lockPixels(env, pngbitmap, &pInfo->png_pixels)) < 0) {
		ALOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
		return;
	}

	memset(pInfo->png_pixels, 0x0, sizeof(int)*pInfo->png_size);
	png_data_memcpy();
	AndroidBitmap_unlockPixels(env, pngbitmap);
}

static void setVideoSurface(const sp<ISDBTPlayer>& player, JNIEnv *env, jobject thiz)
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
static void ISDBTPlayer_prepare(JNIEnv *env, jobject thiz, jint specfic_info)
//static void ISDBTPlayer_prepare(JNIEnv *env, jobject thiz)
{
    ALOGV("prepare");
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
	
    process_player_call(env, thiz, player->prepare(specfic_info), NULL, NULL);
    setVideoSurface(player, env, thiz);
}

static void ISDBTPlayer_start(JNIEnv *env, jobject thiz, jint country_code)
{
    ALOGV("start");
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->start(country_code), NULL, NULL);
}

static void ISDBTPlayer_stop(JNIEnv *env, jobject thiz)
{
    ALOGV("stop");
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->stop(), NULL, NULL);
}

/*
static jint ISDBTPlayer_getVideoWidth(JNIEnv *env, jobject thiz)
{
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
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

static jint ISDBTPlayer_getVideoHeight(JNIEnv *env, jobject thiz)
{
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
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
*/

static void ISDBTPlayer_getSignalStrength(JNIEnv *env, jobject thiz, jobject obj)
{
	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return;
	}

	jclass clazz = env->GetObjectClass(obj);
	jfieldID fid;
	jint array_size;
	jintArray jarray;

	int	*sqinfo = new int[256];;
	int	size_sqinfo, *ptr_sqinfo_data;

	/* get SQ value from tuner component 			*/
	/* sqinfo[0] = size of SQ data in bytes				*/
	/* sqinfo[1] = percentage of 1seg signal quality		*/
	/* sqinfo[2] = percentage of fullseg signal quality	*/
	/* sqinfo[3~] = other informations about signal quality */
	/* refer to the implementation of OMX_IndexVendorConfigGetSignalStrength per each tuner component */
	*sqinfo = 0;
	if (0 != player->getSignalStrength(sqinfo)) {
		ALOGE("getSignalStrength failed");
		*sqinfo = 0;
	}
	size_sqinfo = *sqinfo;
	ptr_sqinfo_data = sqinfo+1;

	if (size_sqinfo < 0 ) size_sqinfo = 0;
	if (size_sqinfo > 256) size_sqinfo = 256;
	
	fid = env->GetFieldID(clazz, "SQInfoArraySize", "I");
	env->SetIntField(obj, fid, size_sqinfo);
	if (size_sqinfo != 0) {
		array_size = size_sqinfo;
		jarray = env->NewIntArray(array_size);
		if (jarray) {
			fid = env->GetFieldID(clazz, "SQInfoArray", "[I");
			env->SetIntArrayRegion(jarray, 0, array_size, (jint*)ptr_sqinfo_data);
			env->SetObjectField(obj, fid, jarray);
			env->DeleteLocalRef(jarray);
		}
	}
	delete sqinfo;
}

/*
static jboolean ISDBTPlayer_isPlaying(JNIEnv *env, jobject thiz)
{
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return false;
    }
    const jboolean is_playing = player->isPlaying();
    ALOGV("isPlaying: %d", is_playing);
    return is_playing;
}
*/

static void ISDBTPlayer_release(JNIEnv *env, jobject thiz)
{
    ALOGV("release");
    sp<ISDBTPlayer> player = setISDBTPlayer(env, thiz, 0);
    if (player != NULL) {
	player->release();
        player->setListener(0);
    }
}


static jint ISDBTPlayer_startTestMode(JNIEnv *env, jobject thiz, jint index)
{
	ALOGV("%s",__func__);
	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL)
	{
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	process_player_call(env, thiz, player->startTestMode(index), NULL, "ISDBTPlayer_startTestMode failed.");
	return 0;
}

static void ISDBTPlayer_search(JNIEnv *env, jobject thiz, jint scan_type, jint countryCode, jint area_code, jint channel_num, jint options)
{
	ALOGV("ISDBTPlayer_search: countryCode = %d", countryCode);
	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return;
	}
	status_t opStatus = player->search(scan_type, countryCode, area_code, channel_num, options);
	process_player_call(env, thiz, opStatus, NULL, NULL);
}

static void ISDBTPlayer_searchCancel(JNIEnv *env, jobject thiz)
{
    ALOGV("%s", __func__);
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->searchCancel(), NULL, NULL);
}

static void ISDBTPlayer_releaseSurface(JNIEnv *env, jobject thiz)
{
    ALOGV("%s", __func__);
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->releaseSurface(), NULL, NULL);
}

static jint ISDBTPlayer_useSurface(JNIEnv *env, jobject thiz, jint arg)
{
    ALOGV("%s", __func__);
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return -1;
    }
    process_player_call(env, thiz, player->useSurface(arg), NULL, NULL);
    return 0;
}

static void ISDBTPlayer_setSurface(JNIEnv *env, jobject thiz)
{

    ALOGV("%s", __func__);
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    setVideoSurface(player, env, thiz);
}

static jint ISDBTPlayer_setAudio(JNIEnv *env, jobject thiz, jint index)
{
    ALOGV("setAudio");
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return -1;
    }
    status_t opStatus = player->setAudio(index);
    process_player_call(env, thiz, opStatus, NULL, NULL);  
	return 0;
}

static void ISDBTPlayer_setChannel(JNIEnv *env, jobject thiz, jint mainRowID, \
		jint subRowID, jint audioIndex, jint audioMode, jint raw_w, jint raw_h, \
		jint view_w, jint view_h, jint ch_index)
{
    ALOGV("setChannel");
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
/*	
    Channel channel;
    jclass clazz = env->GetObjectClass(obj);

    jfieldID fid = env->GetFieldID(clazz, "channelNumber", "I");
    channel.channelNumber = env->GetIntField(obj, fid);
    ALOGV("channelNumber = %d", channel.channelNumber);

    fid = env->GetFieldID(clazz, "countryCode", "I");
    channel.countryCode = env->GetIntField(obj, fid);
    ALOGV("countryCode = 0x%x", channel.countryCode);

    fid = env->GetFieldID(clazz, "audioPID", "I");
    channel.audioPID = env->GetIntField(obj, fid);
    ALOGV("audioPID = %d", channel.audioPID);

    fid = env->GetFieldID(clazz, "videoPID", "I");
    channel.videoPID = env->GetIntField(obj, fid);
    ALOGV("videoPID =  %d", channel.videoPID);

    env->DeleteLocalRef(clazz);
    process_player_call(env, thiz, player->setChannel(&channel), NULL, NULL);  
*/
    status_t opStatus = player->setChannel(mainRowID, subRowID, audioIndex, audioMode, raw_w, raw_h, view_w, view_h, ch_index);
    process_player_call(env, thiz, opStatus, NULL, NULL);  
}

static void ISDBTPlayer_setDualChannel(JNIEnv *env, jobject thiz, jint channelIndex)
{
    ALOGV("setDualChannel");
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    status_t opStatus = player->setDualChannel(channelIndex);
    process_player_call(env, thiz, opStatus, NULL, NULL);  
}

static void ISDBTPlayer_getChannelInfo(JNIEnv *env, jobject thiz, jobject obj)
{
	int iChannelNumber, iServiceID;
	int iRemoconID, iThreeDigitNumber, iServiceType, iChannelNameLen;
	unsigned short *pusChannelName;
	int iTotalAudioPID, iTotalVideoPID, iTotalSubtitleLang;
	int iAudioMode, iVideoMode, iAudioLang1, iAudioLang2;
	int iStartMJD, iStartHH, iStartMM, iStartSS, iDurationHH, iDurationMM, iDurationSS, iEvtNameLen;
	unsigned short *pusEvtName;
 	int iLogoImageWidth, iLogoImageHeight;
	int iCaptionMgmtNumLang;
	unsigned int *pusLogoImage;

	ALOGV("getChannelInfo");
	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return;
	}

	jclass clazz = env->GetObjectClass(obj);
	jfieldID fid;

	fid = env->GetFieldID(clazz, "channelNumber", "I");
	iChannelNumber = env->GetIntField(obj, fid);

	fid = env->GetFieldID(clazz, "serviceID", "I");
	iServiceID = env->GetIntField(obj, fid);

	pusChannelName = new unsigned short[256];
	pusEvtName = new unsigned short[256];
	pusLogoImage = new unsigned int[5184];

	status_t opStatus = player->getChannelInfo(iChannelNumber, iServiceID, \
											   &iRemoconID, &iThreeDigitNumber, &iServiceType, pusChannelName, &iChannelNameLen, \
											   &iTotalAudioPID, &iTotalVideoPID, &iTotalSubtitleLang, \
											   &iAudioMode, &iVideoMode, &iAudioLang1, &iAudioLang2,
											   &iStartMJD, &iStartHH, &iStartMM, &iStartSS, &iDurationHH, &iDurationMM, &iDurationSS, pusEvtName, &iEvtNameLen, \
											   &iLogoImageWidth, &iLogoImageHeight, pusLogoImage, &iCaptionMgmtNumLang);

	//fid = env->GetFieldID(clazz, "remoconID", "I");
	//env->SetIntField(obj, fid, iRemoconID);

	//fid = env->GetFieldID(clazz, "threeDigitNumber", "I");
	//env->SetIntField(obj, fid, iThreeDigitNumber);

	//fid = env->GetFieldID(clazz, "serviceType", "I");
	//env->SetIntField(obj, fid, iServiceType);

	//fid = env->GetFieldID(clazz, "channelName", "Ljava/lang/String;");
	//jstring jstr = env->NewString(pusChannelName, iChannelNameLen/2);
	//if( jstr ){
	//	env->SetObjectField(obj, fid, jstr);
	//	env->DeleteLocalRef(jstr);
	//}

	fid = env->GetFieldID(clazz, "totalAudioPID", "I");
	env->SetIntField(obj, fid, iTotalAudioPID);

	fid = env->GetFieldID(clazz, "totalVideoPID", "I");
	env->SetIntField(obj, fid, iTotalVideoPID);

	fid = env->GetFieldID(clazz, "totalSubtitleLang", "I");
	env->SetIntField(obj, fid, iTotalSubtitleLang);

	fid = env->GetFieldID(clazz, "audioMode", "I");
	env->SetIntField(obj, fid, iAudioMode);
	
	fid = env->GetFieldID(clazz, "videoMode", "I");
	env->SetIntField(obj, fid, iVideoMode);

	fid = env->GetFieldID(clazz, "audioLang1", "I");
	env->SetIntField(obj, fid, iAudioLang1);

	fid = env->GetFieldID(clazz, "audioLang2", "I");
	env->SetIntField(obj, fid, iAudioLang2);

	fid = env->GetFieldID(clazz, "startMJD", "I");
	env->SetIntField(obj, fid, iStartMJD);
		
	fid = env->GetFieldID(clazz, "startHH", "I");
	env->SetIntField(obj, fid, iStartHH);
		
	fid = env->GetFieldID(clazz, "startMM", "I");
	env->SetIntField(obj, fid, iStartMM);
		
	fid = env->GetFieldID(clazz, "startSS", "I");
	env->SetIntField(obj, fid, iStartSS);
		
	fid = env->GetFieldID(clazz, "durationHH", "I");
	env->SetIntField(obj, fid, iDurationHH);
		
	fid = env->GetFieldID(clazz, "durationMM", "I");
	env->SetIntField(obj, fid, iDurationMM);

	fid = env->GetFieldID(clazz, "durationSS", "I");
	env->SetIntField(obj, fid, iDurationSS);

	fid = env->GetFieldID(clazz, "CaptionMgmtNumLang", "I");
	env->SetIntField(obj, fid, iCaptionMgmtNumLang);

	fid = env->GetFieldID(clazz, "eventName", "Ljava/lang/String;");
	jstring jstr = env->NewString(pusEvtName, iEvtNameLen/2);
	if( jstr ){
		env->SetObjectField(obj, fid, jstr);
		env->DeleteLocalRef(jstr);
	}

	fid = env->GetFieldID(clazz, "logoImageWidth", "I");
	env->SetIntField(obj, fid, iLogoImageWidth);
		
	fid = env->GetFieldID(clazz, "logoImageHeight", "I");
	env->SetIntField(obj, fid, iLogoImageHeight);

	if(iLogoImageWidth > 0 && iLogoImageHeight > 0){
		jint size = iLogoImageWidth*iLogoImageHeight;
		jintArray jarray = env->NewIntArray(size);
		if(jarray){
			fid = env->GetFieldID(clazz, "plogoImage", "[I");
			env->SetIntArrayRegion(jarray, 0, size, (jint*)pusLogoImage);
			env->SetObjectField(obj, fid, jarray);
			env->DeleteLocalRef(jarray);
		}
	}

	delete pusChannelName;
	delete pusEvtName;
	delete pusLogoImage;

	process_player_call(env, thiz, opStatus, NULL, NULL);  
}
/*
static jint ISDBTPlayer_getCountryCode(JNIEnv *env, jobject thiz)
{
    ALOGV("getCountryCode");
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return -1;
    }
    int countryCode = player->getCountryCode();
    return countryCode;
}
*/
static void ISDBTPlayer_setCountryCode(JNIEnv *env, jobject thiz, jint countryCode)
{
    ALOGV("setCountryCode");
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_player_call(env, thiz, player->setCountryCode(countryCode), NULL, "setCountryCode failed.");
}

static void ISDBTPlayer_setVolume(JNIEnv *env, jobject thiz, jfloat leftVolume, jfloat rightVolume)
{
    ALOGV("setVolume");
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return;
    }
    process_player_call(env, thiz, player->setVolume(leftVolume, rightVolume), NULL, "setVolume failed.");
}
/*
static void ISDBTPlayer_setDisplayPosition(JNIEnv *env, jobject thiz, jint x, jint y, jint width, jint height, jint rotate)
{
    ALOGV("setDisplayPosition");
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return;
    }
    process_player_call(env, thiz, player->setDisplayPosition(x, y, width, height, rotate), NULL, "setDisplayPosition failed.");
}
*/
static jint ISDBTPlayer_setCapture(JNIEnv *env, jobject thiz, jstring filePath)
{
    ALOGV("setCapture");
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
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

static jint ISDBTPlayer_setRecord(JNIEnv *env, jobject thiz, jstring filePath)
{
    ALOGV("setRecord");
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
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

static jint ISDBTPlayer_setRecStop(JNIEnv *env, jobject thiz)
{
    ALOGV("setRecStop");
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->setRecStop(), NULL, "setRecStop failed.");

    return 0;
}

static jint ISDBTPlayer_playSubtitle(JNIEnv *env, jobject thiz, jint onoff)
{
    ALOGV("playSubtitle : %d", onoff);
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->playSubtitle(onoff), NULL, "ISDBTPlayer_playSubtitle failed.");
    return 0;
}

static jint ISDBTPlayer_playSuperimpose(JNIEnv *env, jobject thiz, jint onoff)
{
    ALOGV("playSuperimpose : %d", onoff);
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->playSuperimpose(onoff), NULL, "ISDBTPlayer_playSuperimpose failed.");
    return 0;
}

static jint ISDBTPlayer_playPng(JNIEnv *env, jobject thiz, jint onoff)
{
    ALOGV("playPng : %d", onoff);
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->playPng(onoff), NULL, "ISDBTPlayer_playPng failed.");
    return 0;
}

static jint ISDBTPlayer_setSubtitle(JNIEnv *env, jobject thiz, jint index)
{
    ALOGV("setSubtitle : %d", index);
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->setSubtitle(index), NULL, "ISDBTPlayer_setSubtitle failed.");
    return 0;
}

static jint ISDBTPlayer_setSuperImpose(JNIEnv *env, jobject thiz, jint index)
{
    ALOGV("setSuperImpose : %d", index);
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->setSuperImpose(index), NULL, "ISDBTPlayer_setSuperImpose failed.");
    return 0;
}


static jint ISDBTPlayer_playTeletext(JNIEnv *env, jobject thiz, jint onoff)
{
    ALOGV("%s",__func__);
    return 0;
}

static jint ISDBTPlayer_setTeletext_UpdatePage(JNIEnv *env, jobject thiz, jint pageNumber)
{
    ALOGV("%s",__func__);
    return 0;
}

static jint ISDBTPlayer_setTeletext_CachePage_Insert(JNIEnv *env, jobject thiz, jint pageNumber)
{
    //ALOGV("%s",__func__);
    return 0;
}

static jint ISDBTPlayer_setTeletext_CachePage_Delete(JNIEnv *env, jobject thiz, jint pageNumber)
{
    ALOGV("%s",__func__);
    return 0;
}

static jint ISDBTPlayer_setAudioDualMono (JNIEnv *env, jobject thiz, jint audio_sel)
{
    ALOGV("setAudioDualMono : %d", audio_sel);
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->setAudioDualMono(audio_sel), NULL, "setAudioDualMono failed.");
    return 0;
}

static jint ISDBTPlayer_setParentalRate (JNIEnv *env, jobject thiz, jint age)
{
    ALOGV("setParentalRate - age:%d", age);
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->setParentalRate(age), NULL, "setParentalRate failed.");
    return 0;
}

static void ISDBTPlayer_setArea (JNIEnv *env, jobject thiz, jint area_code)
{
       ALOGV ("setArea - %d\n", area_code);
       sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
       if (player == NULL){
               jniThrowException(env, "java/lang/IllegalStateException", NULL);
               return;
       }

	process_player_call (env, thiz, player->setArea (area_code), NULL, "setArea failed.");
}
    
static void ISDBTPlayer_setPreset (JNIEnv *env, jobject thiz, jint area_code)
{
       ALOGV ("setPreset - %d\n", area_code);
       sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
       if (player == NULL){
               jniThrowException(env, "java/lang/IllegalStateException", NULL);
               return;
       }

	process_player_call (env, thiz, player->setPreset (area_code), NULL, "set preset failed.");
}

static void ISDBTPlayer_setHandover (JNIEnv *env, jobject thiz, jint country_code)
{
       ALOGV ("setHandover - %d\n", country_code);
       sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
       if (player == NULL){
               jniThrowException(env, "java/lang/IllegalStateException", NULL);
               return;
       }

	process_player_call (env, thiz, player->setHandover (country_code), NULL, "setHandover failed.");
}

static jint ISDBTPlayer_requestEPGUpdate(JNIEnv *env, jobject thiz, jint arg)
{
	ALOGV("%s", __func__);
	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL)
	{
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
	}

	process_player_call(env, thiz, player->requestEPGUpdate(arg), NULL, "ISDBTPlayer_updateEPGUpdate failed.");
	return 0;
}

static jint ISDBTPlayer_reqSCInfo(JNIEnv *env, jobject thiz, jint arg)
{
	ALOGV("%s", __func__);
	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL)
	{
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
	}

	process_player_call(env, thiz, player->reqSCInfo(arg), NULL, "ISDBTPlayer_getSCInfo failed.");
	return 0;
}

static jint ISDBTPlayer_setPlay(JNIEnv *env, jobject thiz, jstring filePath, jint raw_w, jint raw_h, jint view_w, jint view_h)
{
	ALOGV("setFilePlay");
	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	char strBuff[256];	
	const char *sz = env->GetStringUTFChars(filePath, 0) ;
	strcpy(strBuff, sz) ;
	env->ReleaseStringUTFChars(filePath, sz) ;

	process_player_call(env, thiz, player->setPlay(strBuff, raw_w, raw_h, view_w, view_h), NULL, "setPlay failed.");

	return 0;
}

static jint ISDBTPlayer_setPlayStop(JNIEnv *env, jobject thiz)
{
	ALOGV("setFilePlayStop");
	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	process_player_call(env, thiz, player->setPlayStop(), NULL, "setPlayStop failed.");

	return 0;
}

static jint ISDBTPlayer_setSeekTo(JNIEnv *env, jobject thiz, jint arg)
{
	ALOGV("setFilePlaySeekTo");

	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}
	
	int seekTime;
	process_player_call(env, thiz, player->setSeekTo(arg), NULL, "setSeekTo failed.");

	return 0;
}

static jint ISDBTPlayer_setPlaySpeed(JNIEnv *env, jobject thiz, jint arg)
{
	ALOGV("setFilePlaySpeed");
	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	//process_player_call(env, thiz, player->setPlaySpeed(arg), NULL, "setPlaySpeed failed.");

	return player->setPlaySpeed(arg);
}

static void ISDBTPlayer_setPlayPause(JNIEnv *env, jobject thiz, jboolean arg)
{
	ALOGV("ISDBTPlayer_setPlayPause");
	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return;
	}

	process_player_call(env, thiz, player->setPlayPause(arg), NULL, "setPlayPause failed.");
}

static void ISDBTPlayer_setUserLocalTimeOffset(JNIEnv *env, jobject thiz, jboolean bAutoMode, jint iUserLocalTimeOffset)
{
    ALOGV("%s",__func__);
    return;
}

static jint ISDBTPlayer_getDuration(JNIEnv *env, jobject thiz)
{
	//    ALOGV("getDuration");
	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	return player->getDuration();
}

static jint ISDBTPlayer_getCurrentPosition(JNIEnv *env, jobject thiz)
{
	//    ALOGV("getCurrentPosition");
	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	return player->getCurrentPosition();
}

static jint ISDBTPlayer_getCurrentReadPosition(JNIEnv *env, jobject thiz)
{
	//    ALOGV("getCurrentReadPosition");
	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	return player->getCurrentReadPosition();
}

static jint ISDBTPlayer_getCurrentWritePosition(JNIEnv *env, jobject thiz)
{
	//    ALOGV("getCurrentWritePosition");
	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	return player->getCurrentWritePosition();
}
/*
static jint ISDBTPlayer_getDiskTotalSize(JNIEnv *env, jobject thiz)
{
	//    ALOGV("getDiskTotalSize");
	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	return player->getTotalSize();
}

static jint ISDBTPlayer_getDiskMaxSize(JNIEnv *env, jobject thiz)
{
	//    ALOGV("getDiskMaxSize");
	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	return player->getMaxSize();
}

static jint ISDBTPlayer_getDiskFreeSize(JNIEnv *env, jobject thiz)
{
	//    ALOGV("getDiskFreeSize");
	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	return player->getFreeSize();
}

static jint ISDBTPlayer_getDiskBlockSize(JNIEnv *env, jobject thiz)
{
	//    ALOGV("getDiskBlockSize");
	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	return player->getBlockSize();
}
*/
static jstring ISDBTPlayer_getDiskFSType(JNIEnv *env, jobject thiz)
{
	//    ALOGV("getDiskFSType");
	char strFSType[12];
	jstring jstr;
	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
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

static jint ISDBTPlayer_getCurrentDateTime(JNIEnv *env, jobject thiz, jobject date_time)
{
	int iMJD, iHour, iMin, iSec;
	int iPolarity, iHourOffset, iMinOffset;

	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	player->getCurrentDateTime(&iMJD, &iHour, &iMin, &iSec, &iPolarity, &iHourOffset, &iMinOffset);

	jclass   clazz = env->GetObjectClass(date_time);
	jfieldID fid;

	fid = env->GetFieldID(clazz, "iMJD", "I");
	env->SetIntField(date_time, fid, iMJD);

	fid = env->GetFieldID(clazz, "iHour", "I");
	env->SetIntField(date_time, fid, iHour);

	fid = env->GetFieldID(clazz, "iMinute", "I");
	env->SetIntField(date_time, fid, iMin);

	fid = env->GetFieldID(clazz, "iSecond", "I");
	env->SetIntField(date_time, fid, iSec);

	fid = env->GetFieldID(clazz, "iPolarity", "I");
	env->SetIntField(date_time, fid, iPolarity);

	fid = env->GetFieldID(clazz, "iHourOffset", "I");
	env->SetIntField(date_time, fid, iHourOffset);

	fid = env->GetFieldID(clazz, "iMinOffset", "I");
	env->SetIntField(date_time, fid, iMinOffset);

	if (iHour < 24)	{
		if (iPolarity == 0) {
			iHour += iHourOffset;
			if ((iHour / 24) > 0) {
				iHour %= 24;
				iMJD += 1;
			}
			iMin += iMinOffset;
		} else {
			if (iHour >= iHourOffset) {
				iHour -= iHourOffset;
			} else {
				iHour = (iHour + 24) - iHourOffset;
				iMJD -= 1;
			}
			iMin -= iMinOffset;
		}
	}

	fid = env->GetFieldID(clazz, "iLocalMJD", "I");
	env->SetIntField(date_time, fid, iMJD);

	fid = env->GetFieldID(clazz, "iLocalHour", "I");
	env->SetIntField(date_time, fid, iHour);

	fid = env->GetFieldID(clazz, "iLocalMin", "I");
	env->SetIntField(date_time, fid, iMin);

	fid = env->GetFieldID(clazz, "iLocalSec", "I");
	env->SetIntField(date_time, fid, iSec);

	env->DeleteLocalRef(clazz);

	return 0;
}

/*
static jint ISDBTPlayer_setScreenMode(JNIEnv *env, jobject thiz, jint screenmode, jint left, jint top, jint right, jint bottom)
{
    ALOGV("%s",__func__);
    return 0;
}
*/

static void ISDBTPlayer_setFreqBand (JNIEnv *env, jobject thiz, jint freq_band)
{
	ALOGV("%s : %d", __func__, freq_band);
	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateExceptin", NULL);
	} else {
		process_player_call(env, thiz, player->setFreqBand(freq_band), NULL, "ISDBTPlayer_setFreqBand Fail");
	}
}

static void ISDBTPlayer_setFieldLog (JNIEnv *env, jobject thiz, jstring sPath, jint fOn_Off)
{
	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL) {
		jniThrowException(env, "java/lang/IllegalStateExceptin", NULL);
	} else {
		char strBuff[128];

		if (fOn_Off) {
			const char *sz = env->GetStringUTFChars(sPath, 0);
			strcpy (strBuff, sz);
			env->ReleaseStringUTFChars (sPath, sz);
		} else {
			memset (strBuff, 0, 128);		
		}

		process_player_call(env, thiz, player->setFieldLog(strBuff, fOn_Off), NULL, "ISDBTPlayer_setFieldLog Fail");
	}
}

static int ISDBTPlayer_CtrlLastFrame_Open(JNIEnv *env, jobject thiz)
{
    //ALOGV("CtrlLastFrame_Open\n");
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->CtrlLastFrame_Open(), NULL, "ISDBTPlayer_CtrlLastFrame_Open failed.");
    return 0;
}

static int ISDBTPlayer_CtrlLastFrame_Close(JNIEnv *env, jobject thiz)
{
    //ALOGV("CtrlLastFrame_Close\n");
    sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
    if (player == NULL) {
         jniThrowException(env, "java/lang/IllegalStateException", NULL);
         return -1;
    }

    process_player_call(env, thiz, player->CtrlLastFrame_Close(), NULL, "ISDBTPlayer_CtrlLastFrame_Close failed.");
    return 0;
}

static jstring ISDBTPlayer_property_get(JNIEnv *env, jobject thiz, jstring keyJ)
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

static void ISDBTPlayer_property_set(JNIEnv *env, jobject thiz, jstring keyJ, jstring valJ)
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

static int ISDBTPlayer_setPresetMode (JNIEnv *env, jobject thiz, jint preset_mode)
{
	ALOGV ("setPresetMode - %d\n", preset_mode);
	sp<ISDBTPlayer> player = getISDBTPlayer(env, thiz);
	if (player == NULL){
		jniThrowException(env, "java/lang/IllegalStateException", NULL);
		return -1;
	}

	int rtn = player->setPresetMode (preset_mode);
	process_player_call (env, thiz, rtn, NULL, NULL);
	return rtn;
}

static void ISDBTPlayer_native_setup(JNIEnv *env, jobject thiz, jobject weak_this)
{
    ALOGV("ISDBTPlayer_native_setup");
    sp<ISDBTPlayer> player = new ISDBTPlayer();
    if (player == NULL) {
        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
        return;
    }

    // create new listener
    sp<JNIISDBTPlayerListener> listener = new JNIISDBTPlayerListener(env, thiz, weak_this);
    player->setListener(listener);

    setISDBTPlayer(env, thiz, player);
}

static void ISDBTPlayer_native_finalize(JNIEnv *env, jobject thiz)
{
    ALOGV("native_finalize");
    ISDBTPlayer_release(env, thiz);
}

static const char *classPathName = "com/telechips/android/isdbt/player/isdbt/ISDBTPlayer";

static JNINativeMethod methods[] = {
  { "prepare", "(I)V", (void*)ISDBTPlayer_prepare },
  { "_start", "(I)V", (void*)ISDBTPlayer_start },
  { "_stop", "()V", (void*)ISDBTPlayer_stop },
//  { "getVideoWidth", "()I", (void*)ISDBTPlayer_getVideoWidth },
//  { "getVideoHeight", "()I", (void*)ISDBTPlayer_getVideoHeight },
  { "getSignalStrength", "(Lcom/telechips/android/isdbt/player/isdbt/SQInfoData;)V", (void*)ISDBTPlayer_getSignalStrength },
//  { "isPlaying", "()Z", (void*)ISDBTPlayer_isPlaying },
  { "_release", "()V", (void*)ISDBTPlayer_release },
  { "startTestMode", "(I)I", (void*)ISDBTPlayer_startTestMode },
  { "search", "(IIIII)V", (void*)ISDBTPlayer_search },
  { "searchCancel", "()V", (void*)ISDBTPlayer_searchCancel },
  { "releaseSurface", "()V", (void*)ISDBTPlayer_releaseSurface },
  { "setSurface", "()V", (void*)ISDBTPlayer_setSurface },
  { "useSurface", "(I)I", (void*)ISDBTPlayer_useSurface },
//  { "setScreenMode", "(IIIII)I", (void*)ISDBTPlayer_setScreenMode },
  { "setChannel", "(IIIIIIIII)V", (void*)ISDBTPlayer_setChannel },
  { "setDualChannel", "(I)V", (void*)ISDBTPlayer_setDualChannel },
  { "getChannelInfo", "(Lcom/telechips/android/isdbt/player/isdbt/Channel;)V", (void*)ISDBTPlayer_getChannelInfo }, 
//  { "getCountryCode", "()I", (void*)ISDBTPlayer_getCountryCode },
  { "setCountryCode", "(I)V", (void*)ISDBTPlayer_setCountryCode },
  { "setVolume", "(FF)V", (void*)ISDBTPlayer_setVolume },
//  { "setDisplayPosition", "(IIIII)V", (void*)ISDBTPlayer_setDisplayPosition },
  { "playSubtitle", "(I)I", (void*)ISDBTPlayer_playSubtitle },
  { "playSuperimpose", "(I)I", (void*)ISDBTPlayer_playSuperimpose }, 
  { "playPng", "(I)I", (void*)ISDBTPlayer_playPng}, 
  { "setSubtitle", "(I)I", (void *)ISDBTPlayer_setSubtitle },
  { "setSuperImpose", "(I)I", (void *)ISDBTPlayer_setSuperImpose },
  { "playTeletext", "(I)I", (void*)ISDBTPlayer_playTeletext },
  { "setTeletext_UpdatePage", "(I)I", (void*)ISDBTPlayer_setTeletext_UpdatePage },
  { "setTeletext_CachePage_Insert", "(I)I", (void*)ISDBTPlayer_setTeletext_CachePage_Insert },
  { "setTeletext_CachePage_Delete", "(I)I", (void*)ISDBTPlayer_setTeletext_CachePage_Delete },
  { "setCapture", "(Ljava/lang/String;)I", (void*)ISDBTPlayer_setCapture },
  { "setRecord", "(Ljava/lang/String;)I", (void*)ISDBTPlayer_setRecord },
  { "setRecStop", "()I", (void*)ISDBTPlayer_setRecStop },
  { "setAudio", "(I)I", (void*)ISDBTPlayer_setAudio }, 
  { "setAudioDualMono", "(I)I", (void*)ISDBTPlayer_setAudioDualMono }, 
  { "setParentalRate", "(I)I", (void*)ISDBTPlayer_setParentalRate }, 
  { "setArea", "(I)V", (void*)ISDBTPlayer_setArea },
  { "setPreset", "(I)V", (void*)ISDBTPlayer_setPreset },
  { "setHandover", "(I)V", (void*)ISDBTPlayer_setHandover },
  { "requestEPGUpdate", "(I)I", (void*)ISDBTPlayer_requestEPGUpdate },
  { "reqSCInfo", "(I)I", (void*)ISDBTPlayer_reqSCInfo }, 

  { "setPlay", "(Ljava/lang/String;IIII)I", (void*)ISDBTPlayer_setPlay },
  { "setPlayStop", "()I", (void*)ISDBTPlayer_setPlayStop },
  { "setSeekTo", "(I)I", (void*)ISDBTPlayer_setSeekTo },
  { "setPlaySpeed", "(I)I", (void*)ISDBTPlayer_setPlaySpeed },
  { "setPlayPause", "(Z)V", (void*)ISDBTPlayer_setPlayPause },
  { "setUserLocalTimeOffset", "(ZI)V", (void*)ISDBTPlayer_setUserLocalTimeOffset },
  { "getDuration", "()I", (void*)ISDBTPlayer_getDuration },
  { "getCurrentPosition", "()I", (void*)ISDBTPlayer_getCurrentPosition },
  { "getCurrentReadPosition", "()I", (void*)ISDBTPlayer_getCurrentReadPosition },
  { "getCurrentWritePosition", "()I", (void*)ISDBTPlayer_getCurrentWritePosition },
//  { "getDiskTotalSize", "()I", (void*)ISDBTPlayer_getDiskTotalSize },
//  { "getDiskMaxSize", "()I", (void*)ISDBTPlayer_getDiskMaxSize },
//  { "getDiskFreeSize", "()I", (void*)ISDBTPlayer_getDiskFreeSize },
//  { "getDiskBlockSize", "()I", (void*)ISDBTPlayer_getDiskBlockSize },
  { "getDiskFSType", "()Ljava/lang/String;", (void*)ISDBTPlayer_getDiskFSType },
  { "getCurrentDateTime", "(Lcom/telechips/android/isdbt/player/isdbt/DateTimeData;)I", (void*)ISDBTPlayer_getCurrentDateTime },
  { "setFreqBand", "(I)V", (void*)ISDBTPlayer_setFreqBand },
  { "setFieldLog", "(Ljava/lang/String;I)V", (void*)ISDBTPlayer_setFieldLog },
  { "drawSubtitle", "(Landroid/graphics/Bitmap;)V", (void*)ISDBTPlayer_drawSubtitle },
  { "drawSuperimpose", "(Landroid/graphics/Bitmap;)V", (void*)ISDBTPlayer_drawSuperimpose },
  { "drawPng", "(Landroid/graphics/Bitmap;)V", (void*)ISDBTPlayer_drawPng },  
  { "CtrlLastFrame_Open", "()I", (void*)ISDBTPlayer_CtrlLastFrame_Open },	
  { "CtrlLastFrame_Close", "()I", (void*)ISDBTPlayer_CtrlLastFrame_Close },	
  { "property_get", "(Ljava/lang/String;)Ljava/lang/String;", (void*) ISDBTPlayer_property_get },
  { "property_set", "(Ljava/lang/String;Ljava/lang/String;)V", (void*) ISDBTPlayer_property_set },
  { "setPresetMode", "(I)I", (void*)ISDBTPlayer_setPresetMode },
  { "native_setup", "(Ljava/lang/Object;)V", (void*)ISDBTPlayer_native_setup },
  { "native_finalize", "()V", (void*)ISDBTPlayer_native_finalize }
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

