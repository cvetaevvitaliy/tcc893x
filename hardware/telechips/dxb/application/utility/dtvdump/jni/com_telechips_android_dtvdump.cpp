/*
 * Copyright (C) 2012 Telechips, Inc.  All rights reserved.
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

//#define LOG_NDEBUG 0
#define  LOG_TAG    "libdtvdump_jni"

#include <utils/Log.h>
#include <jni.h>
#include <assert.h>

#include "dtvdump.h"

#ifndef ALOGV
#define ALOGV LOGV
#endif

#ifndef ALOGD
#define ALOGD LOGD
#endif

#ifndef ALOGI
#define ALOGI LOGI
#endif

#ifndef ALOGE
#define ALOGE LOGE
#endif

static jint dump_init(JNIEnv *env, jobject thiz, jint iDxbType, jint iFreq, jint iBW, jint iVoltage, jint iMOD)
{
	return DxbDumpInit(iDxbType, iFreq, iBW, iVoltage, iMOD);
}

static jint dump_deinit(JNIEnv *env, jobject thiz)
{
	return DxbDumpDeinit();
}

static jint dump_start(JNIEnv *env, jobject thiz, jintArray piPidTable, jstring pcFile, jint iDumpSize)
{
	jint len = env->GetArrayLength(piPidTable);
	jint *ptr = env->GetIntArrayElements(piPidTable, NULL);
	const char *sz = env->GetStringUTFChars(pcFile, 0) ;

	jint ret = DxbDumpStart(len, ptr, sz, iDumpSize);

	env->ReleaseIntArrayElements(piPidTable, ptr, 0);
	env->ReleaseStringUTFChars(pcFile, sz);

	return ret;
}

static jint dump_stop(JNIEnv *env, jobject thiz)
{
	return DxbDumpStop();	
}

static jint sock_dump_start(JNIEnv *env, jobject thiz, jstring iIp, jint iPort, jstring pcFile, jint iDumpSize)
{
	const char *ip = env->GetStringUTFChars(iIp, 0) ;
	const char *sz = env->GetStringUTFChars(pcFile, 0) ;

	jint ret = SockDumpStart(ip, iPort, sz, iDumpSize);

	env->ReleaseStringUTFChars(iIp, ip);
	env->ReleaseStringUTFChars(pcFile, sz);

	return ret;
}

static jint dump_getSize(JNIEnv *env, jobject thiz)
{
	return GetDumpSize();
}

static jint dump_getFreeSpace(JNIEnv *env, jobject thiz, jstring pcPath)
{
	const char *sz = env->GetStringUTFChars(pcPath, 0) ;

	jint ret = GetDiskFreeSpace(sz);
	
	env->ReleaseStringUTFChars(pcPath, sz);
	
	return ret;
}

static const char *classPathName = "com/telechips/android/dtvdump/DtvDump";

static JNINativeMethod methods[] = {
    { "init", "(IIIII)I", (void*) dump_init },
    { "deinit", "()I", (void*) dump_deinit },
    { "start", "([ILjava/lang/String;I)I", (void*) dump_start },
    { "stop", "()I", (void*) dump_stop },
    { "sockstart", "(Ljava/lang/String;ILjava/lang/String;I)I", (void*) sock_dump_start },
    { "getSize", "()I", (void*) dump_getSize },
    { "getFreeSpace", "(Ljava/lang/String;)I", (void*) dump_getFreeSpace },
};

int registerNatives(JNIEnv* env)
{
    jclass clazz;

    clazz = env->FindClass(classPathName);
    if (clazz == NULL) {
        ALOGE("Native registration unable to find class '%s'", classPathName);
        return JNI_FALSE;
    }

    if (env->RegisterNatives(clazz, methods,
                             sizeof(methods) / sizeof(methods[0])) < 0) {
        ALOGE("RegisterNatives failed for '%s'", classPathName);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("ERROR: GetEnv failed");
        goto bail;
    }
    assert(env != NULL);

    if (registerNatives(env) != JNI_TRUE) {
        ALOGE("ERROR: registerNatives failed");
        goto bail;
    }
    result = JNI_VERSION_1_4;

bail:
    return result;
}
