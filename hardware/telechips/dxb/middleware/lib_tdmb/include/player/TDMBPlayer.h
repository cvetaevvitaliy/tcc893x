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

#ifndef ANDROID_TDMB_BPLAYER_H
#define ANDROID_TDMB_BPLAYER_H

#include <pthread.h>

#include <binder/IMemory.h>
#include <gui/Surface.h>

namespace android
{
typedef void (*notify_callback_f)(void* cookie, int msg, int ext1, int ext2, const Parcel *obj);

	typedef struct
	{
		unsigned int moduleidx;
		unsigned int Ensemble_Band;
		unsigned int Ensemble_Freq;
		unsigned int Ensemble_Id;		// add
		unsigned int Service_Id;
		char Ensemble_Label[20];		// add
		char Service_Label[20];
		char Channel_Label[17];
		unsigned int Channel_Id;
		unsigned int Channel_Type;
		unsigned int Channel_BitRate;
		char Reg[7];	// add
	}TDMB_INFO;

	class DABDLSData {
	public:
		char dlsData[256];
		uint16_t wDlsData[128];
		int dlsDataSize;
		int dlsDataType;
	};

	class TDMBPlayerListener : virtual public RefBase
	{
		public:
			virtual void notify(int msg, int ext1, int ext2, void *obj) = 0;
	};

	class TDMBPlayer : virtual public RefBase
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
				EVENT_AUDIO_OUTPUT = 6,
				EVENT_VIDEO_OUTPUT = 7,
				EVENT_RECORDING_COMPLETE = 8,
				EVENT_PARENTLOCK_COMPLETE = 9,
				EVENT_DLSDATA_UPDATE = 10,
				EVENT_EWS_UPDATE = 11,
				EVENT_DATA_UPDATE = 12,
				EVENT_ERROR = 13,
				EVENT_SERVICE_DIED = 99,
			};

			int miActiveCountrycode; //Countrycode of Now playing channel
			int miActiveChannelNumber; //ChannelNumber of Now playing channel
			int miActiveServiceID; //ServiceID of Now playing channel
			int miActivetotalAudioCnt;	//total Audio count of Now playing channel

			TDMBPlayer();
			~TDMBPlayer();

			status_t	setVideoSurfaceTexture(const sp<IGraphicBufferProducer>& bufferProducer);
			status_t	setListener(const sp<TDMBPlayerListener>& listener);
			status_t	prepare(unsigned int specific_info, int idx, char *pszTDMBDBPath);
			status_t	setprepare(void);
			status_t	start(int country_code);
			status_t	stop(int moduleidx);
			int			getSignalStrength(void *pSignalStrength);
			status_t	release();
			status_t	search(int moduleidx, int countryCode, int *pFreqList, int freqListCount);
			status_t	stopsearch(int moduleidx);
			status_t	manual_search(int moduleidx, int freq);
			status_t	releaseSurface();
			status_t    useSurface(int arg);
			int         setScreenMode(int screenMode, int left, int top, int right, int bottom);
			int		getAudioCnt();
			status_t setAudio(int	index);
			status_t	setChannelIndex(int channelIndex, int type, int moduleidx);
			status_t	manual_setChannel(int moduleidx, int arg);
			int         setCapture(char *filePath);
			int         setRecord(char *filePath);
			int         setRecStop();
			int  playSubtitle(int onoff);
			int			setLCDUpdate();
			status_t	setDisplayEnable();
			status_t	setDisplayDisable();
			status_t	setAudioMute(int isMute);
			status_t    setBBModuleIndex(int bbidx, int moduleidx);
			int	setSubtitle (int index);
			int	setAudioDualMono(int audio_sel);
			int setParentalRate (int age);
			status_t	setArea (int area_code);

			void	notify(int msg, int ext1, int ext2, const Parcel *obj);

			int	getThreadState(void) {return mThreadState;};
			void	setThreadState(int  State) {mThreadState = State;}; 

			void*	getDBHandle(void) {return m_pDBHandle;};
			void setDBHandle(void* pDBHandle) {m_pDBHandle = pDBHandle;};
			
			void	setDBState(int state) {m_ResetDB = state;};
			int	getDBState(void) {return m_ResetDB;};
			
			int	getBBTypeIdx(void) {return m_BBTypeIdx;};
			int	getBBModuleIdx(void) {return m_BBModuleIdx;};
			int	getBBPlayMode(void) {return m_BBPlayMode;};

			void*	getDxBProc(void) {return m_pDxBProc;};
			void	setDxBProc(void* pDxBProc) {m_pDxBProc = pDxBProc;};

			void setNotifyCallback(void *cookie, notify_callback_f notifyFunc) {
				mCookie = cookie; mNotify = notifyFunc;
			}

		private:
			Mutex			mLock;
			Mutex			mNotifyLock;
			sp<ANativeWindow>       mNativeWindow;
			sp<TDMBPlayerListener>	mListener;
			void					*mCookie;
			notify_callback_f		mNotify;
			pthread_t			mThread;
			int			mThreadState;
			void*	m_pDBHandle;
			int	m_ResetDB;

			int	m_BBTypeIdx;
			int	m_BBModuleIdx;
			int	m_BBPlayMode;

			void *m_pDxBProc;
	};

}; // namespace android

#endif // ANDROID_TDMB_BPLAYER_H

