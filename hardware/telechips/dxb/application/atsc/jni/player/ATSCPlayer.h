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

#ifndef ANDROID_ATSC_BPLAYER_H
#define ANDROID_ATSC_BPLAYER_H

#include <pthread.h>

#include <binder/IMemory.h>
#include <gui/Surface.h>

namespace android {

class SubtitleData
{
	public:
		int region_id;
		int x;
		int y;
		int width;
		int height;
		int *data;
};

class ATSCPlayerListener : virtual public RefBase {
public:
    virtual void notify(int msg, int ext1, int ext2, void *obj) = 0;
};

class ATSCPlayer : virtual public RefBase {
public:
	enum event_type {
		EVENT_NOP 					= 0,
		EVENT_PREPARED 				= 1,
		EVENT_SEARCH_COMPLETE 		= 2,
		EVENT_SEARCH_PERCENT 		= 3,
		EVENT_SEARCH_DONEINFO 		= 4,
		EVENT_VIDEO_OUTPUT 			= 5,
		EVENT_RECORDING_COMPLETE 	= 6,
		EVENT_SIGNAL_STATUS 		= 7, 
		EVENT_EPGUPDATE_DONE 		= 8,
		EVENT_SCRAMBLED_DONE 		= 9,		
		EVENT_TIME_UPDATE   		= 10, 
		EVENT_SUBTITLE_UPDATE 		= 11,
		EVENT_ERROR 				= 12
	};

	typedef enum {
		LANGUAGE_NONE = -1, 
		LANGUAGE_ENG, 
		LANGUAGE_KOR
	} LANGUAGE_TYPE;

	int miActiveCountrycode; //Countrycode of Now playing channel
	int miActiveChannelNumber; //ChannelNumber of Now playing channel
	int miActiveServiceType; //ServiceType of Now playing channel
	int miActiveServiceID; //ServiceID of Now playing channel
	
	LANGUAGE_TYPE meActiveLanguage;
	char mszLanguageCode[4];

	ATSCPlayer();
	~ATSCPlayer();

	status_t	setVideoSurfaceTexture(const sp<IGraphicBufferProducer>& bufferProducer);
	status_t	setListener(const sp<ATSCPlayerListener>& listener);
	status_t	prepare(char *pstr_DB_Path);
	status_t	start(int country_code);
	status_t	stop();
	bool	isPlaying();
	status_t	getVideoWidth(int *w);
	status_t	getVideoHeight(int *h);
	status_t getSignalStrength(void *pSignalStrength);
	status_t	release();	
	status_t	search(int countryCode, int modulationFlag);
	status_t	manualSearch(int countryCode, int modulationFlag, int frequencyKhz, int deleteDB);
	status_t	stopsearch();
	status_t	releaseSurface();
	status_t	useSurface(int arg);
	int		getCountryCode();
	status_t	setCountryCode(int countryCode);
	status_t	setChannel(int rowID);
	status_t    setVolume(int volume);
	status_t	setVolume(float leftVolume, float rightVolume);
	status_t	setDisplayPosition(int x, int y, int width, int height, int rotate);
	int         setCapture(char *filePath);
	int         setRecord(char *filePath);
	int         setRecStop();
	int         startSubtitle(int onoff);
	int         playSubtitle(int onoff);
	int         setLCDUpdate();
	status_t    setDisplayEnable();
	status_t    setDisplayDisable();
	int         enableCC(int EnabelDisable);
	int		setCCServiceNum(int ServiceNum);
	int  setAudioLanguage(int AudioLanguage);
	int  setAspectRatio(int AspectRatio);
	void	notify(int msg, int ext1, int ext2, void *obj);

	static void	ThreadFunc(void *arg);
	static void	UI_ThreadFunc(void *arg);

private:
	Mutex			mLock;
	Mutex			mNotifyLock;
	sp<ANativeWindow>       mNativeWindow;
	sp<ATSCPlayerListener>	mListener;
	pthread_t			mThread;
	bool			mThreadRun;
};

}; // namespace android

#endif // ANDROID_ATSC_BPLAYER_H

