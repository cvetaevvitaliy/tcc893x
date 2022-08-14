/*
 * Copyright (C) 2009 Telechips, Inc.
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

#ifndef ANDROID_IPTV_BPLAYER_H
#define ANDROID_IPTV_BPLAYER_H

#include <pthread.h>

#include <binder/IMemory.h>
#include <gui/Surface.h>

namespace android {

class Channel {
public:
    int channelNumber;
    int countryCode;
    int audioPID;
    int videoPID;
	int serviceID;
	int channelFreq;
    char channelName[20];
};

class IPTVSIData {
public:
	char 	siData[1024];
	uint16_t wsiData[128];
	int 		siDataSize;
	int 		siDataType;
};

class IPTVPlayerListener : virtual public RefBase {
public:
    virtual void notify(int msg, int ext1, int ext2, void *obj) = 0;
};

class IPTVPlayer : virtual public RefBase {
public:
    enum event_type {
        EVENT_NOP = 0,
        EVENT_PREPARED = 1,
        EVENT_SEARCH_COMPLETE = 2,
        EVENT_CHANNEL_UPDATE = 3,
		EVENT_TUNNING_COMPLETE = 4,
        EVENT_SEARCH_PERCENT = 5,
        EVENT_VIDEO_OUTPUT = 6,
        EVENT_RECORDING_COMPLETE = 7,
        EVENT_DVB_PARSING_COMPLETE =8,
        EVENT_ERROR,
    };

	int miActiveCountrycode; //Countrycode of Now playing channel
	int miActiveChannelNumber; //ChannelNumber of Now playing channel
	int miActiveServiceID; //ServiceID of Now playing channel
	

    IPTVPlayer();
    ~IPTVPlayer();

    status_t	setVideoSurfaceTexture(const sp<IGraphicBufferProducer>& bufferProducer);
    status_t	setListener(const sp<IPTVPlayerListener>& listener);
    status_t	prepare();
    status_t	start();
    status_t	stop();
    bool	isPlaying();
    status_t	getVideoWidth(int *w);
    status_t	getVideoHeight(int *h);
    status_t	release();	
    status_t	useSurface();
    status_t	releaseSurface();	
    int		getCountryCode();
    status_t	setCountryCode(int countryCode);
	status_t    setVolume(int volume);
    status_t	setVolume(float leftVolume, float rightVolume);
    status_t	setDisplayPosition(int x, int y, int width, int height, int rotate);
    int         setCapture(char *filePath);
    int         setRecord(char *filePath);
    int         setRecStop();
	int 		setPIDInfo(int audio_pid, int video_pid, int pcr_pid, int audio_type, int video_type);
	int 		setIPInfo (int port, char *ip, int protocol);
	int         setSocketInit();
	int         setSocketStart();
	int         setSocketStop();
	int         setSocketCommand(int cmd);
	int         execute();
	int		setActiveMode(int activemode);
	int         setLCDUpdate();
    void	notify(int msg, int ext1, int ext2, void *obj);

    static void	ThreadFunc(void *arg);
    static void	UI_ThreadFunc(void *arg);

private:
    Mutex			mLock;
    Mutex			mNotifyLock;
    sp<ANativeWindow>       mNativeWindow;
    sp<IPTVPlayerListener>	mListener;
    pthread_t			mThread;
    int			mThreadState;
};

}; // namespace android

#endif // ANDROID_IPTV_BPLAYER_H

