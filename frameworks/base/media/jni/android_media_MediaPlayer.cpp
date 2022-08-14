/*
**
** Copyright 2007, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

//#define LOG_NDEBUG 0
#define LOG_TAG "MediaPlayer-JNI"
#include "utils/Log.h"

#include <media/mediaplayer.h>
#include <media/MediaPlayerInterface.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <utils/threads.h>
#include "jni.h"
#include "JNIHelp.h"
#include "android_runtime/AndroidRuntime.h"
#include "android_runtime/android_view_Surface.h"
#include "android_runtime/Log.h"
#include "utils/Errors.h"  // for status_t
#include "utils/KeyedVector.h"
#include "utils/String8.h"
#include "android_media_Utils.h"

#include "android_os_Parcel.h"
#include "android_util_Binder.h"
#include <binder/Parcel.h>
#include <gui/IGraphicBufferProducer.h>
#include <gui/Surface.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>

// VIEW_SUBTITLE
#ifdef USE_TCC_SUBTITLE
#include "tcc_subtitle_interface.h"
#include "tcc_display.h"
#endif

// ----------------------------------------------------------------------------

using namespace android;

// ----------------------------------------------------------------------------

struct fields_t {
    jfieldID    context;
    jfieldID    surface_texture;

    jmethodID   post_event;

    jmethodID   proxyConfigGetHost;
    jmethodID   proxyConfigGetPort;
    jmethodID   proxyConfigGetExclusionList;
};
static fields_t fields;

static Mutex sLock;

// ----------------------------------------------------------------------------
// ref-counted object for callbacks
class JNIMediaPlayerListener: public MediaPlayerListener
{
public:
    JNIMediaPlayerListener(JNIEnv* env, jobject thiz, jobject weak_thiz);
    ~JNIMediaPlayerListener();
    virtual void notify(int msg, int ext1, int ext2, const Parcel *obj = NULL);
private:
    JNIMediaPlayerListener();
    jclass      mClass;     // Reference to MediaPlayer class
    jobject     mObject;    // Weak ref to MediaPlayer Java object to call on
};

JNIMediaPlayerListener::JNIMediaPlayerListener(JNIEnv* env, jobject thiz, jobject weak_thiz)
{

    // Hold onto the MediaPlayer class for use in calling the static method
    // that posts events to the application thread.
    jclass clazz = env->GetObjectClass(thiz);
    if (clazz == NULL) {
        ALOGE("Can't find android/media/MediaPlayer");
        jniThrowException(env, "java/lang/Exception", NULL);
        return;
    }
    mClass = (jclass)env->NewGlobalRef(clazz);

    // We use a weak reference so the MediaPlayer object can be garbage collected.
    // The reference is only used as a proxy for callbacks.
    mObject  = env->NewGlobalRef(weak_thiz);
}

JNIMediaPlayerListener::~JNIMediaPlayerListener()
{
    // remove global references
    JNIEnv *env = AndroidRuntime::getJNIEnv();
    env->DeleteGlobalRef(mObject);
    env->DeleteGlobalRef(mClass);
}


// VIEW_SUBTITLE
typedef enum
{
  SUBTITLE_NONE = 0,
  SUBTITLE_EXTERNAL,
  SUBTITLE_INTERNAL,
  SUBTITLE_INTERNAL_PGS
} SUBTITLE_TYPE;

typedef enum
{
  SUBTITLE_DISPLAY_VIEW = 0,
  SUBTITLE_DISPLAY_CCFB
} SUBTITLE_DISPLAY_TYPE;

static jint g_SubtitleType = SUBTITLE_NONE;
static jint g_SubtitleDispType = SUBTITLE_DISPLAY_VIEW;

void JNIMediaPlayerListener::notify(int msg, int ext1, int ext2, const Parcel *obj)
{
    JNIEnv *env = AndroidRuntime::getJNIEnv();
    if (obj && obj->dataSize() > 0) {
        jobject jParcel = createJavaParcelObject(env);
        if (jParcel != NULL) {
            Parcel* nativeParcel = parcelForJavaObject(env, jParcel);
            nativeParcel->setData(obj->data(), obj->dataSize());
            env->CallStaticVoidMethod(mClass, fields.post_event, mObject,
                    msg, ext1, ext2, jParcel);
        }
    } else {
        env->CallStaticVoidMethod(mClass, fields.post_event, mObject,
                msg, ext1, ext2, NULL);
    }
    if (env->ExceptionCheck()) {
        ALOGW("An exception occurred while notifying an event.");
        LOGW_EX(env);
        env->ExceptionClear();
    }
}

// ----------------------------------------------------------------------------

static sp<MediaPlayer> getMediaPlayer(JNIEnv* env, jobject thiz)
{
    Mutex::Autolock l(sLock);
    MediaPlayer* const p = (MediaPlayer*)env->GetIntField(thiz, fields.context);
    return sp<MediaPlayer>(p);
}

static sp<MediaPlayer> setMediaPlayer(JNIEnv* env, jobject thiz, const sp<MediaPlayer>& player)
{
    Mutex::Autolock l(sLock);
    sp<MediaPlayer> old = (MediaPlayer*)env->GetIntField(thiz, fields.context);
    if (player.get()) {
        player->incStrong((void*)setMediaPlayer);
    }
    if (old != 0) {
        old->decStrong((void*)setMediaPlayer);
    }
    env->SetIntField(thiz, fields.context, (int)player.get());
    return old;
}

// If exception is NULL and opStatus is not OK, this method sends an error
// event to the client application; otherwise, if exception is not NULL and
// opStatus is not OK, this method throws the given exception to the client
// application.
static void process_media_player_call(JNIEnv *env, jobject thiz, status_t opStatus, const char* exception, const char *message)
{
    if (exception == NULL) {  // Don't throw exception. Instead, send an event.
        if (opStatus != (status_t) OK) {
            sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
            if (mp != 0) mp->notify(MEDIA_ERROR, opStatus, 0);
        }
    } else {  // Throw exception!
        if ( opStatus == (status_t) INVALID_OPERATION ) {
            jniThrowException(env, "java/lang/IllegalStateException", NULL);
        } else if ( opStatus == (status_t) PERMISSION_DENIED ) {
            jniThrowException(env, "java/lang/SecurityException", NULL);
        } else if ( opStatus != (status_t) OK ) {
            if (strlen(message) > 230) {
               // if the message is too long, don't bother displaying the status code
               jniThrowException( env, exception, message);
            } else {
               char msg[256];
                // append the status code to the message
               sprintf(msg, "%s: status=0x%X", message, opStatus);
               jniThrowException( env, exception, msg);
            }
        }
    }
}

static void
android_media_MediaPlayer_setDataSourceAndHeaders(
        JNIEnv *env, jobject thiz, jstring path,
        jobjectArray keys, jobjectArray values) {

    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    if (path == NULL) {
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return;
    }

    const char *tmp = env->GetStringUTFChars(path, NULL);
    if (tmp == NULL) {  // Out of memory
        return;
    }
    ALOGV("setDataSource: path %s", tmp);

    String8 pathStr(tmp);
    env->ReleaseStringUTFChars(path, tmp);
    tmp = NULL;

    // We build a KeyedVector out of the key and val arrays
    KeyedVector<String8, String8> headersVector;
    if (!ConvertKeyValueArraysToKeyedVector(
            env, keys, values, &headersVector)) {
        return;
    }

    status_t opStatus =
        mp->setDataSource(
                pathStr,
                headersVector.size() > 0? &headersVector : NULL);

    process_media_player_call(
            env, thiz, opStatus, "java/io/IOException",
            "setDataSource failed." );
}

static void
android_media_MediaPlayer_setDataSourceFD(JNIEnv *env, jobject thiz, jobject fileDescriptor, jlong offset, jlong length)
{
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    if (fileDescriptor == NULL) {
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return;
    }
    int fd = jniGetFDFromFileDescriptor(env, fileDescriptor);
    ALOGV("setDataSourceFD: fd %d", fd);
    process_media_player_call( env, thiz, mp->setDataSource(fd, offset, length), "java/io/IOException", "setDataSourceFD failed." );
}

static void
android_media_MediaPlayer_setVsync(JNIEnv *env, jobject thiz, jboolean vsync)
{
    ALOGV("setVsync: %d", vsync);
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_media_player_call( env, thiz, mp->setVsync(vsync), NULL, NULL );
}

static sp<IGraphicBufferProducer>
getVideoSurfaceTexture(JNIEnv* env, jobject thiz) {
    IGraphicBufferProducer * const p = (IGraphicBufferProducer*)env->GetIntField(thiz, fields.surface_texture);
    return sp<IGraphicBufferProducer>(p);
}

static void
decVideoSurfaceRef(JNIEnv *env, jobject thiz)
{
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        return;
    }

    sp<IGraphicBufferProducer> old_st = getVideoSurfaceTexture(env, thiz);
    if (old_st != NULL) {
        old_st->decStrong((void*)decVideoSurfaceRef);
    }
}

static void
setVideoSurface(JNIEnv *env, jobject thiz, jobject jsurface, jboolean mediaPlayerMustBeAlive)
{
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        if (mediaPlayerMustBeAlive) {
            jniThrowException(env, "java/lang/IllegalStateException", NULL);
        }
        return;
    }

    decVideoSurfaceRef(env, thiz);

    sp<IGraphicBufferProducer> new_st;
    if (jsurface) {
        sp<Surface> surface(android_view_Surface_getSurface(env, jsurface));
        if (surface != NULL) {
            new_st = surface->getIGraphicBufferProducer();
            if (new_st == NULL) {
                jniThrowException(env, "java/lang/IllegalArgumentException",
                    "The surface does not have a binding SurfaceTexture!");
                return;
            }
            new_st->incStrong((void*)decVideoSurfaceRef);
        } else {
            jniThrowException(env, "java/lang/IllegalArgumentException",
                    "The surface has been released");
            return;
        }
    }

    env->SetIntField(thiz, fields.surface_texture, (int)new_st.get());

    // This will fail if the media player has not been initialized yet. This
    // can be the case if setDisplay() on MediaPlayer.java has been called
    // before setDataSource(). The redundant call to setVideoSurfaceTexture()
    // in prepare/prepareAsync covers for this case.
    mp->setVideoSurfaceTexture(new_st);
}

static void
android_media_MediaPlayer_setVideoSurface(JNIEnv *env, jobject thiz, jobject jsurface)
{
    setVideoSurface(env, thiz, jsurface, true /* mediaPlayerMustBeAlive */);
}

static void
android_media_MediaPlayer_prepare(JNIEnv *env, jobject thiz)
{
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    // Handle the case where the display surface was set before the mp was
    // initialized. We try again to make it stick.
    sp<IGraphicBufferProducer> st = getVideoSurfaceTexture(env, thiz);
    mp->setVideoSurfaceTexture(st);

    process_media_player_call( env, thiz, mp->prepare(), "java/io/IOException", "Prepare failed." );
}

static void
android_media_MediaPlayer_prepareAsync(JNIEnv *env, jobject thiz)
{
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    // Handle the case where the display surface was set before the mp was
    // initialized. We try again to make it stick.
    sp<IGraphicBufferProducer> st = getVideoSurfaceTexture(env, thiz);
    mp->setVideoSurfaceTexture(st);

    process_media_player_call( env, thiz, mp->prepareAsync(), "java/io/IOException", "Prepare Async failed." );
}

static void
android_media_MediaPlayer_start(JNIEnv *env, jobject thiz)
{
    ALOGV("start");
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_media_player_call( env, thiz, mp->start(), NULL, NULL );
}

static void
android_media_MediaPlayer_stop(JNIEnv *env, jobject thiz)
{
    ALOGV("stop");
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_media_player_call( env, thiz, mp->stop(), NULL, NULL );
}

static void
android_media_MediaPlayer_pause(JNIEnv *env, jobject thiz)
{
    ALOGV("pause");
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_media_player_call( env, thiz, mp->pause(), NULL, NULL );
}

static jboolean
android_media_MediaPlayer_isPlaying(JNIEnv *env, jobject thiz)
{
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return false;
    }
    const jboolean is_playing = mp->isPlaying();

    ALOGV("isPlaying: %d", is_playing);
    return is_playing;
}

static void
android_media_MediaPlayer_seekTo(JNIEnv *env, jobject thiz, int msec)
{
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    ALOGV("seekTo: %d(msec)", msec);
    process_media_player_call( env, thiz, mp->seekTo(msec), NULL, NULL );
}

static int
android_media_MediaPlayer_getVideoWidth(JNIEnv *env, jobject thiz)
{
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }
    int w;
    if (0 != mp->getVideoWidth(&w)) {
        ALOGE("getVideoWidth failed");
        w = 0;
    }
    ALOGV("getVideoWidth: %d", w);
    return w;
}

static int
android_media_MediaPlayer_getVideoHeight(JNIEnv *env, jobject thiz)
{
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }
    int h;
    if (0 != mp->getVideoHeight(&h)) {
        ALOGE("getVideoHeight failed");
        h = 0;
    }
    ALOGV("getVideoHeight: %d", h);
    return h;
}


static int
android_media_MediaPlayer_getCurrentPosition(JNIEnv *env, jobject thiz)
{
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }
    int msec;
    process_media_player_call( env, thiz, mp->getCurrentPosition(&msec), NULL, NULL );
    ALOGV("getCurrentPosition: %d (msec)", msec);
    return msec;
}

static int
android_media_MediaPlayer_getDuration(JNIEnv *env, jobject thiz)
{
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }
    int msec;
    process_media_player_call( env, thiz, mp->getDuration(&msec), NULL, NULL );
    ALOGV("getDuration: %d (msec)", msec);
    return msec;
}

static void
android_media_MediaPlayer_reset(JNIEnv *env, jobject thiz)
{
    ALOGV("reset");
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_media_player_call( env, thiz, mp->reset(), NULL, NULL );
}

static void
android_media_MediaPlayer_setAudioStreamType(JNIEnv *env, jobject thiz, int streamtype)
{
    ALOGV("setAudioStreamType: %d", streamtype);
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_media_player_call( env, thiz, mp->setAudioStreamType((audio_stream_type_t) streamtype) , NULL, NULL );
}

static void
android_media_MediaPlayer_setLooping(JNIEnv *env, jobject thiz, jboolean looping)
{
    ALOGV("setLooping: %d", looping);
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_media_player_call( env, thiz, mp->setLooping(looping), NULL, NULL );
}

static jboolean
android_media_MediaPlayer_isLooping(JNIEnv *env, jobject thiz)
{
    ALOGV("isLooping");
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return false;
    }
    return mp->isLooping();
}

static void
android_media_MediaPlayer_setVolume(JNIEnv *env, jobject thiz, float leftVolume, float rightVolume)
{
    ALOGV("setVolume: left %f  right %f", leftVolume, rightVolume);
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_media_player_call( env, thiz, mp->setVolume(leftVolume, rightVolume), NULL, NULL );
}

// Sends the request and reply parcels to the media player via the
// binder interface.
static jint
android_media_MediaPlayer_invoke(JNIEnv *env, jobject thiz,
                                 jobject java_request, jobject java_reply)
{
    sp<MediaPlayer> media_player = getMediaPlayer(env, thiz);
    if (media_player == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return UNKNOWN_ERROR;
    }

    Parcel *request = parcelForJavaObject(env, java_request);
    Parcel *reply = parcelForJavaObject(env, java_reply);

    // Don't use process_media_player_call which use the async loop to
    // report errors, instead returns the status.
    return media_player->invoke(*request, reply);
}

// Sends the new filter to the client.
static jint
android_media_MediaPlayer_setMetadataFilter(JNIEnv *env, jobject thiz, jobject request)
{
    sp<MediaPlayer> media_player = getMediaPlayer(env, thiz);
    if (media_player == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return UNKNOWN_ERROR;
    }

    Parcel *filter = parcelForJavaObject(env, request);

    if (filter == NULL ) {
        jniThrowException(env, "java/lang/RuntimeException", "Filter is null");
        return UNKNOWN_ERROR;
    }

    return media_player->setMetadataFilter(*filter);
}

static jboolean
android_media_MediaPlayer_getMetadata(JNIEnv *env, jobject thiz, jboolean update_only,
                                      jboolean apply_filter, jobject reply)
{
    sp<MediaPlayer> media_player = getMediaPlayer(env, thiz);
    if (media_player == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return false;
    }

    Parcel *metadata = parcelForJavaObject(env, reply);

    if (metadata == NULL ) {
        jniThrowException(env, "java/lang/RuntimeException", "Reply parcel is null");
        return false;
    }

    metadata->freeData();
    // On return metadata is positioned at the beginning of the
    // metadata. Note however that the parcel actually starts with the
    // return code so you should not rewind the parcel using
    // setDataPosition(0).
    return media_player->getMetadata(update_only, apply_filter, metadata) == OK;
}

// This function gets some field IDs, which in turn causes class initialization.
// It is called from a static block in MediaPlayer, which won't run until the
// first time an instance of this class is used.
static void
android_media_MediaPlayer_native_init(JNIEnv *env)
{
    jclass clazz;

    clazz = env->FindClass("android/media/MediaPlayer");
    if (clazz == NULL) {
        return;
    }

    fields.context = env->GetFieldID(clazz, "mNativeContext", "I");
    if (fields.context == NULL) {
        return;
    }

    fields.post_event = env->GetStaticMethodID(clazz, "postEventFromNative",
                                               "(Ljava/lang/Object;IIILjava/lang/Object;)V");
    if (fields.post_event == NULL) {
        return;
    }

    fields.surface_texture = env->GetFieldID(clazz, "mNativeSurfaceTexture", "I");
    if (fields.surface_texture == NULL) {
        return;
    }

    clazz = env->FindClass("android/net/ProxyProperties");
    if (clazz == NULL) {
        return;
    }

    fields.proxyConfigGetHost =
        env->GetMethodID(clazz, "getHost", "()Ljava/lang/String;");

    fields.proxyConfigGetPort =
        env->GetMethodID(clazz, "getPort", "()I");

    fields.proxyConfigGetExclusionList =
        env->GetMethodID(clazz, "getExclusionList", "()Ljava/lang/String;");
}

static void
android_media_MediaPlayer_native_setup(JNIEnv *env, jobject thiz, jobject weak_this)
{
    ALOGV("native_setup");
    sp<MediaPlayer> mp = new MediaPlayer();
    if (mp == NULL) {
        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
        return;
    }

    // create new listener and give it to MediaPlayer
    sp<JNIMediaPlayerListener> listener = new JNIMediaPlayerListener(env, thiz, weak_this);
    mp->setListener(listener);

    // Stow our new C++ MediaPlayer in an opaque field in the Java object.
    setMediaPlayer(env, thiz, mp);
}

static void
android_media_MediaPlayer_release(JNIEnv *env, jobject thiz)
{
    ALOGV("release");
    decVideoSurfaceRef(env, thiz);
    sp<MediaPlayer> mp = setMediaPlayer(env, thiz, 0);
    if (mp != NULL) {
        // this prevents native callbacks after the object is released
        mp->setListener(0);
        mp->disconnect();
    }
}

static void
android_media_MediaPlayer_native_finalize(JNIEnv *env, jobject thiz)
{
    ALOGV("native_finalize");
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp != NULL) {
        ALOGW("MediaPlayer finalized without being released");
    }
    android_media_MediaPlayer_release(env, thiz);
}

static void android_media_MediaPlayer_set_audio_session_id(JNIEnv *env,  jobject thiz, jint sessionId) {
    ALOGV("set_session_id(): %d", sessionId);
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_media_player_call( env, thiz, mp->setAudioSessionId(sessionId), NULL, NULL );
}

static jint android_media_MediaPlayer_get_audio_session_id(JNIEnv *env,  jobject thiz) {
    ALOGV("get_session_id()");
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }

    return mp->getAudioSessionId();
}

static void
android_media_MediaPlayer_setAuxEffectSendLevel(JNIEnv *env, jobject thiz, jfloat level)
{
    ALOGV("setAuxEffectSendLevel: level %f", level);
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_media_player_call( env, thiz, mp->setAuxEffectSendLevel(level), NULL, NULL );
}

static void android_media_MediaPlayer_attachAuxEffect(JNIEnv *env,  jobject thiz, jint effectId) {
    ALOGV("attachAuxEffect(): %d", effectId);
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }
    process_media_player_call( env, thiz, mp->attachAuxEffect(effectId), NULL, NULL );
}

static jint
android_media_MediaPlayer_pullBatteryData(JNIEnv *env, jobject thiz, jobject java_reply)
{
    sp<IBinder> binder = defaultServiceManager()->getService(String16("media.player"));
    sp<IMediaPlayerService> service = interface_cast<IMediaPlayerService>(binder);
    if (service.get() == NULL) {
        jniThrowException(env, "java/lang/RuntimeException", "cannot get MediaPlayerService");
        return UNKNOWN_ERROR;
    }

    Parcel *reply = parcelForJavaObject(env, java_reply);

    return service->pullBatteryData(reply);
}

static jint
android_media_MediaPlayer_setRetransmitEndpoint(JNIEnv *env, jobject thiz,
                                                jstring addrString, jint port) {
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return INVALID_OPERATION;
    }

    const char *cAddrString = NULL;

    if (NULL != addrString) {
        cAddrString = env->GetStringUTFChars(addrString, NULL);
        if (cAddrString == NULL) {  // Out of memory
            return NO_MEMORY;
        }
    }
    ALOGV("setRetransmitEndpoint: %s:%d",
            cAddrString ? cAddrString : "(null)", port);

    status_t ret;
    if (cAddrString && (port > 0xFFFF)) {
        ret = BAD_VALUE;
    } else {
        ret = mp->setRetransmitEndpoint(cAddrString,
                static_cast<uint16_t>(port));
    }

    if (NULL != addrString) {
        env->ReleaseStringUTFChars(addrString, cAddrString);
    }

    if (ret == INVALID_OPERATION ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
    }

    return ret;
}

static jboolean
android_media_MediaPlayer_setUIBCParameter(JNIEnv *env, jobject thiz, jint key, jobject java_request)
{
    ALOGD("setParameter: key %d", key);
    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return false;
    }

    Parcel *request = parcelForJavaObject(env, java_request);
    status_t err = mp->setParameter(key, *request);
    if (err == OK) {
        ALOGD("setParameter end true: key %d", key);
        return true;
    } else {
    ALOGD("setParameter end false: key %d", key);
        return false;
    }
}

static void
android_media_MediaPlayer_setNextMediaPlayer(JNIEnv *env, jobject thiz, jobject java_player)
{
    ALOGV("setNextMediaPlayer");
    sp<MediaPlayer> thisplayer = getMediaPlayer(env, thiz);
    if (thisplayer == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", "This player not initialized");
        return;
    }
    sp<MediaPlayer> nextplayer = (java_player == NULL) ? NULL : getMediaPlayer(env, java_player);
    if (nextplayer == NULL && java_player != NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", "That player not initialized");
        return;
    }

    if (nextplayer == thisplayer) {
        jniThrowException(env, "java/lang/IllegalArgumentException", "Next player can't be self");
        return;
    }
    // tie the two players together
    process_media_player_call(
            env, thiz, thisplayer->setNextMediaPlayer(nextplayer),
            "java/lang/IllegalArgumentException",
            "setNextMediaPlayer failed." );
    ;
}

static void
android_media_MediaPlayer_updateProxyConfig(
        JNIEnv *env, jobject thiz, jobject proxyProps)
{
    ALOGV("updateProxyConfig");
    sp<MediaPlayer> thisplayer = getMediaPlayer(env, thiz);
    if (thisplayer == NULL) {
        return;
    }

    if (proxyProps == NULL) {
        thisplayer->updateProxyConfig(
                NULL /* host */, 0 /* port */, NULL /* exclusionList */);
    } else {
        jstring hostObj = (jstring)env->CallObjectMethod(
                proxyProps, fields.proxyConfigGetHost);

        const char *host = env->GetStringUTFChars(hostObj, NULL);

        int port = env->CallIntMethod(proxyProps, fields.proxyConfigGetPort);

        jstring exclusionListObj = (jstring)env->CallObjectMethod(
                proxyProps, fields.proxyConfigGetExclusionList);

        const char *exclusionList =
            env->GetStringUTFChars(exclusionListObj, NULL);

        if (host != NULL && exclusionListObj != NULL) {
            thisplayer->updateProxyConfig(host, port, exclusionList);
        }

        if (exclusionList != NULL) {
            env->ReleaseStringUTFChars(exclusionListObj, exclusionList);
            exclusionList = NULL;
        }

        if (host != NULL) {
            env->ReleaseStringUTFChars(hostObj, host);
            host = NULL;
        }
    }
}

// ----------------------------------------------------------------------------

// VIEW_SUBTITLE
#ifdef USE_TCC_SUBTITLE
static jint
android_media_MediaPlayer_openSubtitle(JNIEnv*env, jobject thiz, jstring path, jint bytecharactersize, jint ccfb)
{
	 char *filename = (char*)env->GetStringUTFChars(path, NULL);
	 sp<MediaPlayer> mp = getMediaPlayer(env, thiz);

	 g_SubtitleType = SUBTITLE_NONE;
	 g_SubtitleDispType = SUBTITLE_DISPLAY_VIEW;

	 if( tccSubtitle_Success == tccSubtitle_open(filename, bytecharactersize) ){
		g_SubtitleType = SUBTITLE_EXTERNAL;
		ALOGD("subtitle file exists....");
	 }
	 else
	 {
		Parcel reply;
        int ret = 0 ;

		do
		{
		    mp->getParameter(KEY_PARAMETER_CHECK_INTERSUBTITLE_EXISTENCE, &reply);
			ret = reply.readInt32();

		}while(ret == -1);
		
		if(ret == 1)
		{
			ALOGD("mkv internal subtitle is ready to work....");
            g_SubtitleType = SUBTITLE_INTERNAL;
		}
		else if(ret == 2)
		{
			ALOGD("PGS is ready to work....");
			g_SubtitleType = SUBTITLE_INTERNAL_PGS;
		}
		else
		{ 
			ALOGD("No - Internal Subtitle");
            g_SubtitleType = SUBTITLE_NONE;
		}
	 }

	return g_SubtitleType;
}

static jboolean
android_media_MediaPlayer_getSubtitle(JNIEnv* env, jobject thiz, jobject obj)
{
	int iLangIdx;
	int iStartPTS, iEndPTS;
	int iLineCount, iDataSize;
	char *pData;
	int iCharset;
	int ret;

	if( g_SubtitleType == SUBTITLE_NONE  )
	{
		ALOGE("getSubtitle : Subtitle is NOT available") ;
		return false ;
	}

	if( g_SubtitleType == SUBTITLE_EXTERNAL )
	{
        ret = tccSubtitle_getFrame(&iLangIdx, &iStartPTS, &iEndPTS, &iLineCount, &iDataSize, &pData);
        if( tccSubtitle_Success != ret ) {
	        //ALOGE("error tccSubtitle_getFrame(ret:%d)", ret);
            return false ;
	    }
		
		jclass clazz = env->GetObjectClass(obj);

		jfieldID fid = env->GetFieldID(clazz, "mSubtitleClassIdx", "I");
		env->SetIntField(obj, fid, iLangIdx);

		fid = env->GetFieldID(clazz, "mSubtitleStartPTS", "I");
		env->SetIntField(obj, fid, iStartPTS);

		fid = env->GetFieldID(clazz, "mSubtitleEndPTS", "I");
		env->SetIntField(obj, fid, iEndPTS);

		iCharset = tccSubtitle_getOutputTextCharacterSet();
		if( tccSubtitle_CharacterSet_ANSI == iCharset ){ 
			fid = env->GetFieldID(clazz, "mSubtitleArray", "[B");
			jbyteArray jarray = env->NewByteArray(iDataSize);
			if( jarray ){
				env->SetByteArrayRegion(jarray, 0, iDataSize, (jbyte *)pData);
				env->SetObjectField(obj, fid, jarray);
				env->DeleteLocalRef(jarray);
			}
			
			fid = env->GetFieldID(clazz, "mSubtitleArrayLen", "I");
			env->SetIntField(obj, fid, iDataSize);
		}
		else if( tccSubtitle_CharacterSet_UNICODE == iCharset ){
			uint16_t uData[1024];
			memcpy(uData, pData, iDataSize);

			fid = env->GetFieldID(clazz, "mSubtitleString", "Ljava/lang/String;");
			jstring jstr = env->NewString(uData, iDataSize/2);
			if( jstr ){
				env->SetObjectField(obj, fid, jstr);
				env->DeleteLocalRef(jstr);
			}
		}
		else if( tccSubtitle_CharacterSet_UTF8 == iCharset ){
			fid = env->GetFieldID(clazz, "mSubtitleString", "Ljava/lang/String;");
			jstring jstr = env->NewStringUTF(pData);
			if( jstr ){
				env->SetObjectField(obj, fid, jstr);
				env->DeleteLocalRef(jstr);
			}
		}

		fid = env->GetFieldID(clazz, "mSubtitleCharSet", "I");
		env->SetIntField(obj, fid, iCharset);
		
		//ALOGV("S:%d, E:%d\n", iStartPTS, iEndPTS);

		return true;
	}
	else if( g_SubtitleType == SUBTITLE_INTERNAL )	// INTERNAL_SUBTITLE_INCLUDE
	{
		// mkv internal case.
		sp<MediaPlayer> mp = getMediaPlayer(env, thiz);

		if (mp == NULL ) {
				jniThrowException(env, "java/lang/IllegalStateException", NULL);
				return false ;
		}

		Parcel reply;
		mp->getParameter(KEY_PARAMETER_INTERSUBTITLE_GET_TEXTDATA, &reply);

		int32_t StartTm, EndTm, TxtSize, ret;
		int32_t *pSubData, *TxTData;
		pSubData = (int32_t *)reply.readInt32();

		if (pSubData == NULL) 
		{
			ALOGV("Fail GetSubtitle (NULL)");
			return false ;
		}

		StartTm = reply.readInt32();
		EndTm = reply.readInt32();
		TxtSize = reply.readInt32();
		
		TxTData = (int32_t *)malloc(TxtSize);
		ret = reply.read(TxTData,TxtSize);
		if(ret != 0)
		{
			ALOGV("Fail GetSubtitle (NULL)");
		}

		jclass clazz = env->GetObjectClass(obj);

		jfieldID fid = env->GetFieldID(clazz, "mSubtitleStartPTS", "I");
		env->SetIntField(obj, fid, StartTm);

		fid = env->GetFieldID(clazz, "mSubtitleEndPTS", "I");
		env->SetIntField(obj, fid, EndTm);

		fid = env->GetFieldID(clazz, "mSubtitleString", "Ljava/lang/String;");
		jstring jstr = env->NewString((jchar *)(TxTData), (jsize)(TxtSize)/2);
		if( jstr ){
			env->SetObjectField(obj, fid, jstr);
			env->DeleteLocalRef(jstr);
		}
        fid = env->GetFieldID(clazz, "mSubtitleCharSet", "I");
		env->SetIntField(obj, fid, tccSubtitle_CharacterSet_UNICODE);
		free((int32_t *)(TxTData));
		return true ;
	}
	else if( g_SubtitleType == SUBTITLE_INTERNAL_PGS )
	{
		// Image Subtitle case.
		sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
		if (mp == NULL ) {
				jniThrowException(env, "java/lang/IllegalStateException", NULL);
				return false ;
		}
		Parcel reply;
		mp->getParameter(KEY_PARAMETER_INTERSUBTITLE_GET_PGSDATA, &reply);
		int32_t SrcWidth, SrcHeight, DstWidth, DstHeight, OffsetX, OffsetY, StartTm, EndTm, StreamSize, ret;
		int *pPGSData;
		unsigned char *ImageData = NULL; 

		pPGSData = (int *)reply.readInt32();
		if (pPGSData == NULL) 
		{
			ALOGV("Fail GetSubtitle  pPGSData == (NULL)");
			return false ;
		}

		SrcWidth   =  reply.readInt32();
		SrcHeight  =  reply.readInt32();
		DstWidth   =  reply.readInt32();
		DstHeight  =  reply.readInt32();
		OffsetX    =  reply.readInt32();
		OffsetY    =  reply.readInt32();
		StartTm    =  reply.readInt32();
		EndTm      =  reply.readInt32();
		StreamSize =  reply.readInt32();

		if(StreamSize != 0)
		{
			int *Temp = NULL; 
			ImageData = (unsigned char*)malloc(StreamSize);
			ret = reply.read(ImageData,StreamSize);
		}

		jclass clazz = env->GetObjectClass(obj);

		jfieldID fid = env->GetFieldID(clazz, "mPGSStartTm", "I");
		env->SetIntField(obj, fid, StartTm);

		fid = env->GetFieldID(clazz, "mSrcPGSWidth", "I");
		env->SetIntField(obj, fid, SrcWidth);

		fid = env->GetFieldID(clazz, "mSrcPGSHeigth", "I");
		env->SetIntField(obj, fid, SrcHeight);

		fid = env->GetFieldID(clazz, "mDstPGSWidth", "I");
		env->SetIntField(obj, fid, DstWidth);

		fid = env->GetFieldID(clazz, "mDstPGSHeigth", "I");
		env->SetIntField(obj, fid, DstHeight);
		
		fid = env->GetFieldID(clazz, "mPGSOffsetX", "I");
		env->SetIntField(obj, fid, OffsetX);

		fid = env->GetFieldID(clazz, "mPGSOffsetY", "I");
		env->SetIntField(obj, fid, OffsetY);
		
		fid = env->GetFieldID(clazz, "mPGSStreamLen", "I");
		env->SetIntField(obj, fid, StreamSize);
			
		jintArray jStreamBuf = env->NewIntArray(SrcWidth*SrcHeight);
		if( jStreamBuf == NULL )
		{
			ALOGE("Couldn't allocate int array for subt data");
			env->ExceptionClear();
		}
		else
		{	
			env->SetIntArrayRegion(jStreamBuf, 0, SrcWidth*SrcHeight,  (const jint *)ImageData);
			fid = env->GetFieldID(clazz, "mPGSStreamArray", "[I");
			env->SetObjectField(obj, fid, jStreamBuf);
			env->DeleteLocalRef(jStreamBuf);
		}

		free((int *)(ImageData));
		return true ;
	}
	return false;
}

static jboolean
android_media_MediaPlayer_seekSubtitle(JNIEnv* env, jobject thiz, jint msec)
{
	int ret;

	ALOGV("JNI android_media_MediaPlayer_seekSubtitle IN msec(%d)", msec) ;

	if( g_SubtitleType == SUBTITLE_NONE ) 
	{	
		ALOGE("seekSubtitle : Subtitle is NOT available") ;
		return false ;
	}

	if( g_SubtitleType == SUBTITLE_EXTERNAL )
	{
		ret = tccSubtitle_seek(msec);
		if( tccSubtitle_Success != ret ) {
			ALOGE("error tccSubtitle_seek(ret:%d)", ret);
			return false ;
		}	
		
		ALOGV("seekSubtitle: %d\n", msec);

		return true;
	}
	else if( g_SubtitleType == SUBTITLE_INTERNAL )	// INTERNAL_SUBTITLE_INCLUDE
	{
		sp<MediaPlayer> mp = getMediaPlayer(env, thiz);

		if (mp == NULL ) {
			ALOGE("%s getMediaPlayer ERR", __func__ ) ;	
			return false ;
		}

		Parcel request;
		request.writeInt32(msec);
		mp->setParameter(KEY_PARAMETER_INTERSUBTITLE_SET_SEEKPOSITION, request);

		return true ;
	}
	return false;
}

static jboolean
android_media_MediaPlayer_getSubtitleClass(JNIEnv* env, jobject thiz, jobject obj)
{
	char *classname;
	int classnamelen;
	int classnum;
	char fieldID[32];
	int i;

	if (g_SubtitleType == SUBTITLE_EXTERNAL ) {
		if( tccSubtitle_Success == tccSubtitle_getClassNums(&classnum) ) {
			jclass clazz = env->GetObjectClass(obj);

			jfieldID fid = env->GetFieldID(clazz, "mSubtitleClassNum", "I");
			env->SetIntField(obj, fid, classnum);
/*
			for(i=0; i<classnum; i++)
			{
				if( tccSubtitle_Success == tccSubtitle_getClassName(i, &classname) ){
					sprintf(fieldID, "mSubtitleClass%d", i);		
					fid = env->GetFieldID(clazz, fieldID, "Ljava/lang/String;");
					jstring jstr = env->NewStringUTF(classname);
					if( jstr ){
						env->SetObjectField(obj, fid, jstr);
						env->DeleteLocalRef(jstr);
					}
				}	
			}
*/

			fid = env->GetFieldID(clazz, "mSubtitleClass", "[Ljava/lang/String;");
			jobject tmp = env->GetObjectField(obj, fid);
			jobjectArray* objarray = reinterpret_cast<jobjectArray*>(&tmp);

			for(i=0; i<classnum; i++)
			{
				if( tccSubtitle_Success == tccSubtitle_getClassName(i, &classname) ){
					env->SetObjectArrayElement(*objarray, i, env->NewStringUTF(classname));
				}	
			}

			return true;
		}
	}
	else if( g_SubtitleType == SUBTITLE_INTERNAL || g_SubtitleType == SUBTITLE_INTERNAL_PGS ) {
		sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
		if (mp == NULL ) {
			ALOGE("%s getMediaPlayer ERR", __func__ ) ;	
			return false ;
		}

		Parcel reply;
		mp->getParameter(KEY_PARAMETER_INTERSUBTITLE_GET_CLASS_NUM, &reply);
		classnum = (int)reply.readInt32();

		jclass clazz = env->GetObjectClass(obj);

		jfieldID fid = env->GetFieldID(clazz, "mSubtitleClassNum", "I");
		env->SetIntField(obj, fid, classnum);

		fid = env->GetFieldID(clazz, "mSubtitleClass", "[Ljava/lang/String;");
		jobject tmp = env->GetObjectField(obj, fid);
		jobjectArray* objarray = reinterpret_cast<jobjectArray*>(&tmp);
		
		for(i=0; i<classnum; i++)
		{
			classnamelen = reply.readInt32();
			classname = (char*)calloc(classnamelen+1, sizeof(char));
			reply.read(classname, classnamelen);

			env->SetObjectArrayElement(*objarray, i, env->NewStringUTF(classname));

			free(classname);	
		}

		return true;
	}

	return false;
}

static jboolean
android_media_MediaPlayer_setSubtitleClass(JNIEnv* env, jobject thiz, jint num, jintArray sequence)
{
	int classindex[3] = {0, };
	int ret;

	if (g_SubtitleType == SUBTITLE_EXTERNAL ) {
		env->GetIntArrayRegion(sequence, 0, num, classindex);
		ret = tccSubtitle_selectClass(num, classindex);
		if( tccSubtitle_Success != ret ) {
			ALOGE("error tccSubtitle_selectClass(ret:%d)", ret);
			return false;
		}

		if( g_SubtitleDispType == SUBTITLE_DISPLAY_CCFB ) {
			tcc_display_clearString();
		}

		return true;
	}
	else if( g_SubtitleType == SUBTITLE_INTERNAL || g_SubtitleType == SUBTITLE_INTERNAL_PGS ) {
		sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
		if (mp == NULL ) {
			ALOGE("%s getMediaPlayer ERR", __func__ ) ;	
			return false ;
		}

		env->GetIntArrayRegion(sequence, 0, num, classindex);

		Parcel request;
		request.writeInt32(classindex[0]);
		mp->setParameter(KEY_PARAMETER_INTERSUBTITLE_SET_CLASS, request);

		return true;
	}

	return false;
}

static  void
android_media_MediaPlayer_closeSubtitle(JNIEnv* env, jobject thiz)
{
	int ret;

	if( g_SubtitleType == SUBTITLE_EXTERNAL )
	{
		ret = tccSubtitle_close();

		if( tccSubtitle_Success != ret )
			ALOGE("error tccSubtitle_close(ret:%d)", ret);
	}

	g_SubtitleType = SUBTITLE_NONE;

	return;
}
#endif /* USE_TCC_SUBTITLE */

// ----------------------------------------------------------------------------

static JNINativeMethod gMethods[] = {
    {
        "_setDataSource",
        "(Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;)V",
        (void *)android_media_MediaPlayer_setDataSourceAndHeaders
    },

    {"_setDataSource",       "(Ljava/io/FileDescriptor;JJ)V",    (void *)android_media_MediaPlayer_setDataSourceFD},
    {"_setVideoSurface",    "(Landroid/view/Surface;)V",        (void *)android_media_MediaPlayer_setVideoSurface},
    {"prepare",             "()V",                              (void *)android_media_MediaPlayer_prepare},
    {"prepareAsync",        "()V",                              (void *)android_media_MediaPlayer_prepareAsync},
    {"_start",              "()V",                              (void *)android_media_MediaPlayer_start},
    {"_stop",               "()V",                              (void *)android_media_MediaPlayer_stop},
    {"getVideoWidth",       "()I",                              (void *)android_media_MediaPlayer_getVideoWidth},
    {"getVideoHeight",      "()I",                              (void *)android_media_MediaPlayer_getVideoHeight},
    {"seekTo",              "(I)V",                             (void *)android_media_MediaPlayer_seekTo},
    {"_pause",              "()V",                              (void *)android_media_MediaPlayer_pause},
    {"isPlaying",           "()Z",                              (void *)android_media_MediaPlayer_isPlaying},
    {"getCurrentPosition",  "()I",                              (void *)android_media_MediaPlayer_getCurrentPosition},
    {"getDuration",         "()I",                              (void *)android_media_MediaPlayer_getDuration},
    {"_release",            "()V",                              (void *)android_media_MediaPlayer_release},
    {"_reset",              "()V",                              (void *)android_media_MediaPlayer_reset},
    {"setAudioStreamType",  "(I)V",                             (void *)android_media_MediaPlayer_setAudioStreamType},
    {"setLooping",          "(Z)V",                             (void *)android_media_MediaPlayer_setLooping},
    {"isLooping",           "()Z",                              (void *)android_media_MediaPlayer_isLooping},
    {"setVolume",           "(FF)V",                            (void *)android_media_MediaPlayer_setVolume},
    {"native_invoke",       "(Landroid/os/Parcel;Landroid/os/Parcel;)I",(void *)android_media_MediaPlayer_invoke},
    {"native_setMetadataFilter", "(Landroid/os/Parcel;)I",      (void *)android_media_MediaPlayer_setMetadataFilter},
    {"native_getMetadata", "(ZZLandroid/os/Parcel;)Z",          (void *)android_media_MediaPlayer_getMetadata},
    {"native_init",         "()V",                              (void *)android_media_MediaPlayer_native_init},
    {"native_setup",        "(Ljava/lang/Object;)V",            (void *)android_media_MediaPlayer_native_setup},
    {"native_finalize",     "()V",                              (void *)android_media_MediaPlayer_native_finalize},
    {"getAudioSessionId",   "()I",                              (void *)android_media_MediaPlayer_get_audio_session_id},
    {"setAudioSessionId",   "(I)V",                             (void *)android_media_MediaPlayer_set_audio_session_id},
    {"setAuxEffectSendLevel", "(F)V",                           (void *)android_media_MediaPlayer_setAuxEffectSendLevel},
    {"attachAuxEffect",     "(I)V",                             (void *)android_media_MediaPlayer_attachAuxEffect},
    {"native_pullBatteryData", "(Landroid/os/Parcel;)I",        (void *)android_media_MediaPlayer_pullBatteryData},
    {"native_setRetransmitEndpoint", "(Ljava/lang/String;I)I",  (void *)android_media_MediaPlayer_setRetransmitEndpoint},
    {"setNextMediaPlayer",  "(Landroid/media/MediaPlayer;)V",   (void *)android_media_MediaPlayer_setNextMediaPlayer},
    {"setVsync",            "(Z)V",                             (void *)android_media_MediaPlayer_setVsync},

    // VIEW_SUBTITLE
#ifdef USE_TCC_SUBTITLE
    {"openSubtitle",        "(Ljava/lang/String;I)I",           (void *)android_media_MediaPlayer_openSubtitle},
    {"getSubtitle",         "(Landroid/media/MediaSubtitle;)Z",	(void *)android_media_MediaPlayer_getSubtitle},
    {"seekSubtitle",        "(I)Z",                             (void *)android_media_MediaPlayer_seekSubtitle},
    {"getSubtitleClass",    "(Landroid/media/MediaSubtitle;)Z",	(void *)android_media_MediaPlayer_getSubtitleClass},
    {"setSubtitleClass",    "(I[I)Z",                           (void *)android_media_MediaPlayer_setSubtitleClass}, 
    {"closeSubtitle",       "()V",                              (void *)android_media_MediaPlayer_closeSubtitle},
#endif
    {"setUIBCParameter",        "(ILandroid/os/Parcel;)Z",          (void *)android_media_MediaPlayer_setUIBCParameter},
    {"updateProxyConfig", "(Landroid/net/ProxyProperties;)V", (void *)android_media_MediaPlayer_updateProxyConfig},
};

static const char* const kClassPathName = "android/media/MediaPlayer";

// This function only registers the native methods
static int register_android_media_MediaPlayer(JNIEnv *env)
{
    return AndroidRuntime::registerNativeMethods(env,
                "android/media/MediaPlayer", gMethods, NELEM(gMethods));
}

extern int register_android_media_ImageReader(JNIEnv *env);
extern int register_android_media_Crypto(JNIEnv *env);
extern int register_android_media_Drm(JNIEnv *env);
extern int register_android_media_MediaCodec(JNIEnv *env);
extern int register_android_media_MediaExtractor(JNIEnv *env);
extern int register_android_media_MediaCodecList(JNIEnv *env);
extern int register_android_media_MediaMetadataRetriever(JNIEnv *env);
extern int register_android_media_MediaMuxer(JNIEnv *env);
extern int register_android_media_MediaRecorder(JNIEnv *env);
extern int register_android_media_MediaScanner(JNIEnv *env);
extern int register_android_media_ResampleInputStream(JNIEnv *env);
extern int register_android_media_MediaProfiles(JNIEnv *env);
extern int register_android_media_AmrInputStream(JNIEnv *env);
extern int register_android_mtp_MtpDatabase(JNIEnv *env);
extern int register_android_mtp_MtpDevice(JNIEnv *env);
extern int register_android_mtp_MtpServer(JNIEnv *env);

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("ERROR: GetEnv failed\n");
        goto bail;
    }
    assert(env != NULL);

    if (register_android_media_ImageReader(env) < 0) {
        ALOGE("ERROR: ImageReader native registration failed");
        goto bail;
    }

    if (register_android_media_MediaPlayer(env) < 0) {
        ALOGE("ERROR: MediaPlayer native registration failed\n");
        goto bail;
    }

    if (register_android_media_MediaRecorder(env) < 0) {
        ALOGE("ERROR: MediaRecorder native registration failed\n");
        goto bail;
    }

    if (register_android_media_MediaScanner(env) < 0) {
        ALOGE("ERROR: MediaScanner native registration failed\n");
        goto bail;
    }

    if (register_android_media_MediaMetadataRetriever(env) < 0) {
        ALOGE("ERROR: MediaMetadataRetriever native registration failed\n");
        goto bail;
    }

    if (register_android_media_AmrInputStream(env) < 0) {
        ALOGE("ERROR: AmrInputStream native registration failed\n");
        goto bail;
    }

    if (register_android_media_ResampleInputStream(env) < 0) {
        ALOGE("ERROR: ResampleInputStream native registration failed\n");
        goto bail;
    }

    if (register_android_media_MediaProfiles(env) < 0) {
        ALOGE("ERROR: MediaProfiles native registration failed");
        goto bail;
    }

    if (register_android_mtp_MtpDatabase(env) < 0) {
        ALOGE("ERROR: MtpDatabase native registration failed");
        goto bail;
    }

    if (register_android_mtp_MtpDevice(env) < 0) {
        ALOGE("ERROR: MtpDevice native registration failed");
        goto bail;
    }

    if (register_android_mtp_MtpServer(env) < 0) {
        ALOGE("ERROR: MtpServer native registration failed");
        goto bail;
    }

    if (register_android_media_MediaCodec(env) < 0) {
        ALOGE("ERROR: MediaCodec native registration failed");
        goto bail;
    }

    if (register_android_media_MediaExtractor(env) < 0) {
        ALOGE("ERROR: MediaCodec native registration failed");
        goto bail;
    }

    if (register_android_media_MediaMuxer(env) < 0) {
        ALOGE("ERROR: MediaMuxer native registration failed");
        goto bail;
    }

    if (register_android_media_MediaCodecList(env) < 0) {
        ALOGE("ERROR: MediaCodec native registration failed");
        goto bail;
    }

    if (register_android_media_Crypto(env) < 0) {
        ALOGE("ERROR: MediaCodec native registration failed");
        goto bail;
    }

    if (register_android_media_Drm(env) < 0) {
        ALOGE("ERROR: MediaDrm native registration failed");
        goto bail;
    }

    /* success -- return valid version number */
    result = JNI_VERSION_1_4;

bail:
    return result;
}

// KTHXBYE
