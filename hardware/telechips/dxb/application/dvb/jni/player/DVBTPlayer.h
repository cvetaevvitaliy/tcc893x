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


#ifndef ANDROID_DVBT_BPLAYER_H
#define ANDROID_DVBT_BPLAYER_H

#include <pthread.h>

#include <binder/IMemory.h>
#include <gui/Surface.h>

namespace android {

typedef struct
{
	char name[6];
	int frequency1; // primary frequency in Khz
	int frequency2; // alternative frequency
	int finetune; // finetune offset in Khz, 0=no finetune
	short bandwidth1; // 6/7/8 Mhz
	short bandwidth2; // alternative bandwidth
	short alt_selected; // alternative selected or not
}OneChannel;


typedef struct
{
	char name[32]; // name of the tune space
	unsigned int countryCode; // country code
	unsigned int space; // 0=terrestrial, 1=cable, 2=satellite
	unsigned int typicalBandwidth; // typical bandwidth
	unsigned int minChannel; // min channel
	unsigned int maxChannel; // max channel
	int finetunes[4]; // pilot finetunes
	unsigned int numbers; // number of channels
	OneChannel channels[128];
}TuneSpace;

typedef struct
{
   unsigned char ucHour;
   unsigned char ucMinute;
   unsigned char ucSecond;
} TIME_STRUCT;

typedef struct
{
   unsigned short uiMJD;
   TIME_STRUCT stTime;
}  DATE_TIME_STRUCT;

class DVBTPlayerListener : virtual public RefBase {
public:
    virtual void notify(int msg, int ext1, int ext2, const Parcel *obj) = 0;
};

class DVBTPlayer : virtual public RefBase {
public:
	enum event_type {
		EVENT_NOP                      = 0,
		EVENT_PREPARED                 = 1,
		EVENT_SEARCH_COMPLETE          = 2,
		EVENT_TUNNING_COMPLETE         = 3,
		EVENT_SEARCH_PERCENT           = 4,
		EVENT_VIDEO_OUTPUT             = 5,
		EVENT_RECORDING_COMPLETE       = 6,
		EVENT_SUBTITLE_UPDATE          = 7,
		EVENT_VIDEODEFINITION_UPDATE   = 8,
		EVENT_DEBUGMODE_UPDATE         = 9,
		EVENT_PLAYING_COMPLETE         = 10,
		EVENT_PLAYING_CURRENT_TIME     = 11,
		EVENT_DBINFO_UPDATE            = 12,
		EVENT_DBINFORMATION_UPDATE     = 13,
		EVENT_SCRAMBLED_DONE           = 14,
		EVENT_TELETEXT_UPDATE          = 15,
		EVENT_ERROR                    = 20,
	};

	int miActiveCountrycode; //Countrycode of Now playing channel
	int miActiveChannelNumber; //ChannelNumber of Now playing channel
	int miActiveDVBPLDId; //uiDVBPLDId of Now playing channel
	int miActiveServiceID; //ServiceID of Now playing channel
	int miActiveAudioPID; //Audio PID of Now playing channel
	int miActiveVideoPID; //Video PID of Now playing channel	
	int miActiveSubtitlePID; //Subtitle PID of Now playing channel
	int miActiveAudioStreamType; //Audio Stream type
	int miActiveVideoStreamType; //Video Stream type
	int miActivePCRPID; //PCR Pid
	int miRecordChannelNumber; // ChannelNumber of recording channel

	DVBTPlayer();
	~DVBTPlayer();

	status_t    setVideoSurfaceTexture(const sp<IGraphicBufferProducer>& bufferProducer);
	status_t    setListener(const sp<DVBTPlayerListener>& listener);
	status_t    prepare(char *pszDVBTDBPath);
	status_t    start(int country_code);
	status_t    stop();
	status_t    getSignalStrength(void *pSignalStrength);
	status_t    setAntennaPower(int arg);
	status_t    release();
	status_t    startTestMode(int index);
	status_t    getRFInformation(unsigned char **ppucData, unsigned int *puiSize);
	status_t    setFrequency(int ich);
	status_t    search(int countryCode);
	status_t    manualSearch(int frequencyKhz, int bandwidthMhz);
	status_t    stopsearch();
	status_t    releaseSurface();
	status_t    useSurface(int arg);
	int         getCountryCode();
	status_t    setCountryCode(int countryCode);
	status_t    setChannel(int rowID, int audioIndex, int subtitleIndex);
	status_t    setVolume(int audioID, int audioVolume);
	status_t    onAudioDescription(bool isOnAudioDescription);
	status_t    setCapture(char *filePath);
	status_t    setRecord(char *filePath);
	status_t    setRecStop();
	status_t    setPlay(char *filePath);
	status_t    setPlayStop();
	status_t    setSeekTo(int seekTime);
	status_t    setPlaySpeed(int iSpeed);
	status_t    setPlayPause(bool bPlayPause);
	status_t    setUserLocalTimeOffset(bool bAutoMode, int iUserLocalTimeOffset);
	int         getDuration();
	int         getCurrentPosition();
	int         getCurrentReadPosition();
	int         getCurrentWritePosition();
	status_t    getFSType(char *pFSType);
	status_t    startSubtitle(int onoff, int subtitleIndex);
	status_t    playSubtitle(int onoff,int subtitleIndex);
	status_t    playTeletext(int onoff);
	status_t    setTeletext_UpdatePage(int pageNumber);
	status_t    setTeletext_CachePage_Insert(int pageNumber);
	status_t    setTeletext_CachePage_Delete(int pageNumber);
	status_t    setLCDUpdate();
	status_t    setDisplayEnable();
	status_t    setDisplayDisable();
	status_t    setAudio(int audioIndex);
	status_t    setStereo(int arg);
	status_t    requestEPGUpdate(int arg);
	DATE_TIME_STRUCT* getCurrentDateTime();
	status_t    goToSleep();
	status_t    setTuneSpace(TuneSpace* space);
	status_t    setServiceDB(int fromfile, void *pChHeaderInfo);

	void        notify(int msg, int ext1, int ext2, const Parcel *obj = NULL);

	static void UI_ThreadFunc(void *arg);

// For DVBSx
	status_t    SetLnbVoltage(int iArg);
	status_t    SetTone(int iArg);
	status_t    DiseqcSendBurst(int iArg);
	status_t    DiseqcSendCMD(unsigned char *pucCMD, unsigned int uiLen);
	status_t    BlindScanReset();
	status_t    BlindScanStart();
	status_t    BlindScanCancel();
	status_t    BlindScanGetState(unsigned int *puiState);
	status_t    BlindScanGetInfo(unsigned int *puiInfo);
// End

private:
	Mutex                   mLock;
	Mutex                   mNotifyLock;
	sp<ANativeWindow>       mNativeWindow;
	sp<DVBTPlayerListener>  mListener;
	pthread_t               mThread;
	int                     mThreadState;

	status_t    loadServiceDB(int rowID, int *iPLPID);
	status_t    loadAudioDB(int iCountryCode, int iServiceID, int iChannelNumber, int iDVBPLDId);
	status_t    loadSubtitleDB(int iCountryCode, int iServiceID, int iChannelNumber, int iDVBPLDId);
	status_t    getAudioInfo(int audioIndex, int *audioPID, int *visual_impaired_audioPID, int *audioStreamType, int *visual_impaired_audiostreamtype);
	status_t    getSubtitleInfo(int subtitleIndex, int *pucSubtitle_PID, int *pusCompositionPageId, int *pusAncillaryPageId);
	status_t    startChannel(int audioIndex, int subtitleIndex, int iDVBPLDId);
};

}; // namespace android

#endif // ANDROID_DVBT_BPLAYER_H

