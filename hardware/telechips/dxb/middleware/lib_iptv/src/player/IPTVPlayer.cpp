/*
 * Copyright 2009 Telechips, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "IPTVPlayer"

#include <unistd.h>
#include <android_runtime/AndroidRuntime.h>

#include "player/IPTVPlayer.h"
#include "tcc_dxb_interface_type.h"
#include "tcc_iptv_proc.h"
#include "tcc_iptv_event.h"

#include "sqlite3.h"
#include <cutils/properties.h>

#include <utils/Log.h>

#define LOG_TAG	"IPTVPlayer"
#define DEBUG_MSG(msg...)	ALOGI(msg)
#define	printf(msg...)		ALOGI(msg)
#define DEBUG(n, msg...)	ALOGI(msg)

sqlite3  *g_pIPTVDBHandle = NULL;

using namespace android;

IPTVPlayer *gpiptvInstance = NULL;						   // XXX
TCCDXBPROC_SET_IP Arg;
TCCDXBPROC_SET 	Stream_Arg;

enum {
    STOP = 0,
    RUN,
    REQUEST_EXIT,
};

void IPTVPlayer::ThreadFunc (void *arg)
{
	IPTVPlayer *player = (IPTVPlayer *) arg;
	if (player == NULL)
		return;

	while (player->mThreadState == RUN)
	{
		usleep(5000);
	}
}

void IPTVPlayer::UI_ThreadFunc (void *arg)
{
	IPTVPlayer *player = (IPTVPlayer *) arg;
	if (player == NULL)
		return;

	TCCDxBEvent_Init();
	while (player->mThreadState == RUN) {
		if( TCCDxBEvent_Process() == 0)
	        {		
		       usleep(5000);		
	        }
        }
    player->mThreadState = STOP;
	TCCDxBEvent_DeInit();
}

IPTVPlayer::IPTVPlayer ()
{
	gpiptvInstance = this;
	miActiveCountrycode = -1;	//Countrycode of Now playing channel
	miActiveChannelNumber = -1;	//ChannelNumber of Now playing channel
	miActiveServiceID = -1;	//ServiceID of Now playing channel
	mThreadState = STOP;
	mNativeWindow = NULL;

	TCCDxBProc_InitVideoSurface(0); // 0: HWC, 1: Mali
}

IPTVPlayer::~IPTVPlayer ()
{
	TCCDxBProc_DeinitVideoSurface();
	if (mNativeWindow != NULL) {
		mNativeWindow.clear();
		mNativeWindow = NULL;
	}
}

int IPTVPlayer::prepare ()
{
	int res;
	DEBUG_MSG ("%s", __func__);

    while(mThreadState != STOP) usleep(1000);
    mThreadState = RUN;
	AndroidRuntime::createJavaThread ("IPTVPlayerThread", ThreadFunc, (void *) this);
	AndroidRuntime::createJavaThread ("IPTVPlayerUIThread", UI_ThreadFunc, (void *) this);

	return OK;
}

status_t IPTVPlayer::release ()
{
	DEBUG_MSG ("%s", __func__);

	gpiptvInstance = NULL;
	TCCDxBProc_DeInit();

	mThreadState = REQUEST_EXIT;

	return OK;
}

status_t IPTVPlayer::releaseSurface()
{
	DEBUG_MSG ("%s", __func__);
	TCCDxBProc_VideoReleaseSurface();
	return OK;
}

status_t IPTVPlayer::useSurface()
{
	DEBUG_MSG ("%s", __func__);

	TCCDxBProc_VideoUseSurface();
	return OK;
}

status_t IPTVPlayer::setVideoSurfaceTexture(const sp<IGraphicBufferProducer>& bufferProducer)
{
	DEBUG_MSG ("%s", __func__);
	Mutex::Autolock _l (mLock);
	if (bufferProducer != NULL) {
		sp<ANativeWindow> nativeWindow = new Surface(bufferProducer);
		TCCDxBProc_SetVideoSurface(nativeWindow.get());
		if (mNativeWindow != NULL) {
			mNativeWindow.clear();
		}
		mNativeWindow = nativeWindow;
	} else {
		DEBUG_MSG("%s: bufferProducer is null", __func__);
	}
	return OK;
}

status_t IPTVPlayer::start ()
{
	TCCDxBProc_Init(NULL);
	return OK;
}

status_t IPTVPlayer::stop ()
{
	TCCDxBProc_Stop();
	return OK;
}

bool IPTVPlayer::isPlaying ()
{
	return true;
}

status_t IPTVPlayer::getVideoWidth (int *w)
{
	TCCDxBProc_GetVideoInfo(w , 0);
	return OK;
}

int IPTVPlayer::getVideoHeight (int *h)
{
	TCCDxBProc_GetVideoInfo(0, h);
	return OK;
}

status_t IPTVPlayer::setVolume (int volume)
{
	return OK;
}

status_t IPTVPlayer::setVolume (float leftVolume, float rightVolume)
{
	DEBUG_MSG ("In %s leftVolume(%f) rightVolume(%f)\n", __func__, leftVolume, rightVolume);
	return OK;
}

status_t IPTVPlayer::setDisplayPosition (int x, int y, int width, int height, int rotate)
{
	DEBUG_MSG ("In %s x(%d) y(%d) width(%d) height(%d) rotate(%d)\n", __func__, x, y, width, height, rotate);
	return OK;
}

int IPTVPlayer::setCapture (char *filePath)
{
	DEBUG_MSG ("%s::%s", __func__, filePath);
	//TCCDxBSvc_CaptureVideoFrame (filePath);
	return OK;
}

int IPTVPlayer::setRecord (char *filePath)
{
	DEBUG_MSG ("%s::%s", __func__, filePath);
	return OK;
}

int IPTVPlayer::setRecStop ()
{
	DEBUG_MSG ("%s", __func__);
	return OK;
}

int IPTVPlayer::setLCDUpdate ()
{
	DEBUG_MSG ("%s", __func__);
	return OK;
}

int IPTVPlayer::setPIDInfo(int audio_pid, int video_pid, int pcr_pid, int audio_type, int video_type)
{
	ALOGE("%s %d audio_pid=%x,  video_pid=%x\n", __func__, __LINE__, audio_pid, video_pid);

	Stream_Arg.iAudioPID= audio_pid;	
	Stream_Arg.iVideoPID= video_pid;	
	Stream_Arg.iPcrPID = pcr_pid;	
	Stream_Arg.iAudioStreamType= audio_type;	
	Stream_Arg.iVideoStreamType = video_type;	
	return 0;
}

int IPTVPlayer::setIPInfo (int port, char *ip, int protocol)
{
	if(port ==0)
	{
		char value[PROPERTY_VALUE_MAX];

		property_get("tcc.iptv.set.ip", value, "");
		strcpy((char *)Arg.aucIPStr, value);
		property_get("tcc.iptv.set.port", value, "");
		Arg.uiPort = atoi(value);

		ALOGE("%s %d ip : %s \n", __func__, __LINE__, Arg.aucIPStr);
		ALOGE("%s %d port  = %d \n", __func__, __LINE__, Arg.uiPort);
	}
	else
	{
		strcpy((char *)Arg.aucIPStr, ip);
		Arg.uiPort = port;	
		Arg.uiProtocol = protocol;	
	}
	TCCDxBProc_SocketIpsetting(&Arg);
	return 0;
}

int IPTVPlayer::setSocketInit ()
{
	ALOGE ("%s", __func__);
	TCCDxBProc_Socketinit();
	return 0;
}

int IPTVPlayer::setSocketStart ()
{
	ALOGE ("%s", __func__);
	TCCDxBProc_SocketStart();
	return 0;
}

int IPTVPlayer::setSocketStop ()
{
	ALOGE ("%s", __func__);
	TCCDxBProc_SocketStop();
	return 0;
}

int IPTVPlayer::setSocketCommand(int cmd)
{
	DEBUG_MSG ("%s", __func__);
	TCCDxBProc_SocketCommand(cmd);
	return 0;
}

int IPTVPlayer::execute ()
{
	ALOGE("%s %d ip : %s \n", __func__, __LINE__, Arg.aucIPStr);
	ALOGE("%s %d port  = %d \n", __func__, __LINE__, Arg.uiPort);

	TCCDxBProc_MultiCastingSet(&Arg, & Stream_Arg);
	
	return 0;
}

int IPTVPlayer::setActiveMode(int activemode)
{
	DEBUG_MSG ("%s", __func__);
	TCCDxBProc_SetActiveMode(activemode);
	return 0;
}



status_t IPTVPlayer::setListener (const sp < IPTVPlayerListener > &listener)
{
	DEBUG_MSG ("setListener");
	Mutex::Autolock _l (mLock);
	if (mListener != NULL)
		mListener.clear ();
	mListener = listener;
	return NO_ERROR;
}

void IPTVPlayer::notify (int msg, int ext1, int ext2, void *obj)
{
	DEBUG_MSG ("message received msg=%d, ext1=%d, ext2=%d", msg, ext1, ext2);
	sp < IPTVPlayerListener > listener = mListener;
	if (listener != 0)
	{
		Mutex::Autolock _l (mNotifyLock);
		listener->notify (msg, ext1, ext2, obj);
	}
}

void notify_search_percent_update (int iPercent)
{
	IPTVPlayer *player = gpiptvInstance;
	ALOGV ("notify_search_percent_update %d%", iPercent);

	if (player == NULL)
	{
		return;
	}
	player->notify (IPTVPlayer::EVENT_SEARCH_PERCENT, iPercent, iPercent, 0);
}

void notify_channel_complete ()
{
	DEBUG_MSG ("notify_channel_complete");

	IPTVPlayer *player = gpiptvInstance;
	if (player == NULL)
	{
		return;
	}

	if (g_pIPTVDBHandle != NULL)
	{
		int rc, iChannelNum;
		char szSQL[1024] = { 0 };
		sqlite3_stmt *stmt;
		sprintf (szSQL, "SELECT channelNumber FROM channels WHERE _id=1");
		sqlite3_prepare (g_pIPTVDBHandle, szSQL, strlen (szSQL), &stmt, NULL);
		rc = sqlite3_step (stmt);
		if (rc == SQLITE_ROW)
		{
			iChannelNum = sqlite3_column_int (stmt, 0);
		}
		sqlite3_finalize (stmt);
	}

	player->notify (IPTVPlayer::EVENT_SEARCH_COMPLETE, 0, 0, 0);
}

void notify_recording_complete (void)
{
	ALOGV ("notify_recording_complete");
	IPTVPlayer *player = gpiptvInstance;
	if (player == NULL)
	{
		return;
	}
	player->notify (IPTVPlayer::EVENT_RECORDING_COMPLETE, 0, 0, 0);
}


void notify_sendsi_data (TCC_IPTV_SI_DEC_CMD_t *pSIData_Cmd)
{
	ALOGE ("notify_sendnit_data");
	IPTVPlayer *player = gpiptvInstance;

	ALOGE ("data = %x size = %d", pSIData_Cmd->SIData, pSIData_Cmd->SIDataLength);

	IPTVSIData SiData;
	SiData.siDataSize = pSIData_Cmd->SIDataLength;
	SiData.siDataType = pSIData_Cmd->SIDataType;

	memcpy(SiData.siData, pSIData_Cmd->SIData, pSIData_Cmd->SIDataLength);

	if (player == NULL)
	{
		return;
	}
	player->notify (IPTVPlayer::EVENT_DVB_PARSING_COMPLETE, 0, 0, (void *)&SiData);

}


void notify_video_frame_output_start(void *pResult)
{
    ALOGV("notify_video_frame_output_start");

    IPTVPlayer *player = gpiptvInstance;
    if (player == NULL) {
        return;
    }

    player->notify(IPTVPlayer::EVENT_VIDEO_OUTPUT, 0, 0, 0);
}


