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


#ifndef ANDROID_ISDBT_BPLAYER_H
#define ANDROID_ISDBT_BPLAYER_H

#include <pthread.h>

#include <binder/IMemory.h>
#include <gui/Surface.h>

namespace android
{
	typedef struct
	{
		unsigned char ucHour;
		unsigned char ucMinute;
		unsigned char ucSecond;
	} TIME_STRUCT_TYPE;

	typedef struct
	{
		unsigned short uiMJD;
		TIME_STRUCT_TYPE stTime;
	}  DATE_TIME_STRUCT_TYPE;

	class DB_CHANNEL_Data
	{
		public:
			int channelNumber;
			int countryCode;
			int uiVersionNum;
			int audioPID;
			int videoPID;
			int stPID;
			int siPID;
			int PMT_PID;
			int remoconID;
			int serviceType;
			int serviceID;
			int regionID;
			int threedigitNumber;
			int TStreamID;
			int berAVG;
			int preset;
			int networkID;
			int areaCode;
			short *channelName;
	};

	class DB_SERVICE_Data
	{
		public:
			int uiPCRPID;
	};

	class DB_AUDIO_Data
	{
		public:
			int uiServiceID;
			int uiCurrentChannelNumber;
			int uiCurrentCountryCode;
			int usNetworkID;
			int uiAudioPID;
			int uiAudioIsScrambling;
			int uiAudioStreamType;
			int uiAudioType;
			char *pucacLangCode;
	};

	class DB_SUBTITLE_Data
	{
		public:
			int uiServiceID;
			int uiCurrentChannelNumber;
			int uiCurrentCountryCode;
			int uiSubtitlePID;
			char *pucacSubtLangCode;
			int uiSubtType;
			int uiCompositionPageID;
			int uiAncillaryPageID;
	};

	class DBInfoData
	{
		public:
			DB_CHANNEL_Data dbCh;
			DB_SERVICE_Data dbService;
			DB_AUDIO_Data dbAudio[5];
			DB_SUBTITLE_Data dbSubtitle[10];
	};

	class Channel
	{
		public:
			int channelNumber;
			int countryCode;
			int audioPID;
			int videoPID;
			int serviceID;
			int channelFreq;
			char channelName[20];
	};

	class SCInfo
	{
		public:
			char SCDataSize;
		 	char SCData[196];
	};

	class SubtitleData
	{
		public:
			int seg_type;
			int output_type;
			int x;
			int y;
			int w;
			int h;
			int pitch_w;
			int pitch_h;
			int subtitle_update;
	};

	class SuperimposeData
	{
		public:
			int superimpose_update;
			int png_update;
	};


	class VideoDefinitionData
	{
		public:
			int width;
			int height;
			int aspect_ratio;
			int frame_rate;
			int main_DecoderID;
			int sub_width;
			int sub_height;
			int sub_aspect_ratio;
			int sub_DecoderID;
			int DisplayID;
	};
	
	
	class EWSData
	{
		public:
			int serviceID;
			int startendflag;
			int signaltype;
			int area_code_length;
			int localcode[256];
	};
	
	class ISDBTPlayerListener : virtual public RefBase
	{
		public:
			virtual void notify(int msg, int ext1, int ext2, void *obj) = 0;
	};

	class ISDBTPlayer : virtual public RefBase
	{
		public:
			enum event_type
			{
				EVENT_NOP = 0,
				EVENT_PREPARED = 1,
				EVENT_SEARCH_COMPLETE = 2,
				EVENT_CHANNEL_UPDATE = 3,
				EVENT_TUNNING_COMPLETE = 4,
				EVENT_SEARCH_PERCENT = 5,
				EVENT_VIDEO_OUTPUT = 6,
				EVENT_SUPERIMPOSE_UPDATE		= 7,
				EVENT_RECORDING_COMPLETE		= 8,
				EVENT_SUBTITLE_UPDATE			= 9,
				EVENT_VIDEODEFINITION_UPDATE	= 10,
				EVENT_DEBUGMODE_UPDATE			= 11,
				EVENT_PLAYING_COMPLETE			= 12,
				EVENT_PLAYING_CURRENT_TIME		= 13,
				EVENT_DBINFO_UPDATE				= 14,
				EVENT_DBINFORMATION_UPDATE		= 15,
				EVENT_ERROR						= 16,
				EVENT_PNG_UPDATE				= 17,

				// ISDBT
				EVENT_EPG_UPDATE				= 30,
				EVENT_PARENTLOCK_COMPLETE		= 31,
				EVENT_HANDOVER_CHANNEL			= 32,
				EVENT_EMERGENCY_INFO			= 33,
				EVENT_SMARTCARD_ERROR			= 34,
				EVENT_SMARTCARDINFO_UPDATE		= 35,

				EVENT_AUTOSEARCH_UPDATE			= 36,
				EVENT_AUTOSEARCH_DONE			= 37, 
			};

			ISDBTPlayer();
			~ISDBTPlayer();

			void BackupLayerOrder(void);
			void RestoreLayerOrder(void);

			status_t	setVideoSurfaceTexture(const sp<IGraphicBufferProducer>& bufferProducer);
			status_t	setListener(const sp<ISDBTPlayerListener>& listener);
			status_t	prepare(unsigned int	specific_info);
			status_t	start(int country_code);
			status_t	stop();
			bool	isPlaying();
			int	startTestMode(int index);
			status_t	getVideoWidth(int *w);
			status_t	getVideoHeight(int *h);
			int			getSignalStrength(int *sqinfo);
			status_t	release();
			status_t	search(int scan_type, int country_code, int area_code, int channel_num, int options);
			status_t	searchCancel();
			status_t	releaseSurface();
			status_t    useSurface(int arg);
			int         setScreenMode(int screenMode, int left, int top, int right, int bottom);
			int		getCountryCode();
			status_t	setCountryCode(int countryCode);
			int		getAudioCnt();
			status_t setAudio(int	index);
			status_t setChannel(int mainRowID, int subRowID, int audioIndex, int audioMode, int raw_w, int raw_h, int view_w, int view_h, int ch_index);
			status_t	setDualChannel(int channelIndex);
			status_t	getChannelInfo(int iChannelNumber, int iServiceID, \
									 int *piRemoconID, int *piThreeDigitNumber, int *piServiceType, unsigned short *pusChannelName, int *piChannelNameLen, \
									 int *piTotalAudioPID, int *piTotalVideoPID, int *piTotalSubtitleLang, \
									 int *piAudioMode, int *piVideoMode, int *piAudioLang1, int *piAudioLang2, \
									 int *piStartMJD, int *piStartHH, int *piStartMM, int *piStartSS, int *piDurationHH, int *piDurationMM, int *piDurationSS, unsigned short *pusEvtName, int *piEvtNameLen, \
									 int *piLogoImageWidth, int *piLogoImageHeight, unsigned int *pusLogoImage,  int *piCaptionMgmtNumLang);
			status_t    setVolume(int volume);
			status_t	setVolume(float leftVolume, float rightVolume);
			status_t	setDisplayPosition(int x, int y, int width, int height, int rotate);
			int         setCapture(char *filePath);
			int         setRecord(char *filePath);
			int         setRecStop();
			int  playSubtitle(int onoff);
			int  playSuperimpose(int onoff);			
			int  playPng(int onoff);			
			int			setLCDUpdate();
			int	setSubtitle (int index);
			int	setSuperImpose (int index);
			int	setAudioDualMono(int audio_sel);
			int setParentalRate (int age);
			status_t	setArea (int area_code);
			status_t	setPreset (int area_code);
			int         setHandover(int country_code);
			int         requestEPGUpdate(int day_offset);
			int		reqSCInfo(int arg);
			int setFreqBand (int freq_band);
			int setFieldLog (char *sPath, int fOn_Off);

			int setPlay(char *filePath, int raw_w, int raw_h, int view_w, int view_h);
			int setPlayStop();
			int setSeekTo(int seekTime);
			int setPlaySpeed(int iSpeed);
			int setPlayPause(bool bPlayPause);
			int getDuration();
			int getCurrentPosition();
			int getCurrentReadPosition();
			int getCurrentWritePosition();
			int getTotalSize();
			int getMaxSize();
			int getFreeSize();
			int getBlockSize();
			char* getFSType(char *pFSType);
			int getCurrentDateTime(int *piMJD, int *piHour, int *piMin, int *piSec, int *piPolarity, int *piHourOffset, int *piMinOffset);
			int setPresetMode(int preset_mode);
			int goToSleep();
			int CtrlLastFrame_Open();
			int CtrlLastFrame_Close();
			
			void	notify(int msg, int ext1, int ext2, void *obj);

			static void	ThreadFunc(void *arg);
			static void	UI_ThreadFunc(void *arg);

		private:
			Mutex			mLock;
			Mutex			mNotifyLock;
			sp<ANativeWindow>       mNativeWindow;
			sp<ISDBTPlayerListener>	mListener;
			pthread_t			mThread;
			int			mThreadState;
	};

}; // namespace android

#endif // ANDROID_ISDBT_BPLAYER_H

