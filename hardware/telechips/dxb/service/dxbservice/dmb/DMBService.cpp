/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions 
andlimitations under the License.

****************************************************************************/

/****************************************************************************

Revision History

****************************************************************************

****************************************************************************/

#define LOG_NDEBUG 0
#define LOG_TAG "DMBService"
#include <utils/Log.h>
#include <cutils/properties.h>

#include <binder/Parcel.h>
#include <binder/IServiceManager.h>

#include <DMBPlayerClient.h>

#include "DMBService.h"

namespace android {

static Mutex sLock[2];

int freqList[64] = { 0 };

enum {
    UNPREPARED = 0,
    STOPPED,
    PLAYING,
};

int DMBService::instantiate()
{
    ALOGV("%s", __func__);
    int r = defaultServiceManager()->addService(String16("tcc.dxb.server"), new DMBService());
    return r;
}

DMBService::DMBService()
{
    ALOGV("%s", __func__);
}

DMBService::~DMBService()
{
    ALOGV("%s", __func__);
}

int DMBService::changeService(int standard)
{
    int ret = -1;
    char value[PROPERTY_VALUE_MAX];

    if (standard == 2) // dmb standard = 2
    {
        ret = 1;
    }
    else if (mClients.size() == 0)
    {
        sprintf(value, "%d", standard);
        property_set("tcc.dxb.standard", value);
        ret = kill(getpid(), SIGKILL);
    }
    else
    {
        ALOGE("Not Change Service: Client Num = %d", mClients.size());
    }

    return ret;
}

sp<IDxbPlayer> DMBService::create(pid_t pid, const sp<IDxbPlayerClient>& client)
{
    ALOGV("%s, ClientNum:%d->%d", __func__, mClients.size(), mClients.size() + 1);
    sp<Client> c = new Client(this, pid, client);
    wp<Client> w = c;
    {
        Mutex::Autolock lock(mLock);
        {
#ifdef MULTI_PLAYER
            sp<TDMBPlayer> Player = new TDMBPlayer;
            Player->setNotifyCallback(this, notify);
            mState = UNPREPARED;

            c->setPlayer(Player);
#else
            if (mClients.size() == 0)
            {
                mPlayer = new TDMBPlayer;
                mPlayer->setNotifyCallback(this, notify);
                mState = UNPREPARED;
            }
            c->setPlayer(mPlayer);
#endif

            mClients.add(w);
        }
    }
    return c;
}

void DMBService::removeClient(wp<Client> client)
{
    ALOGV("%s", __func__);
    Mutex::Autolock lock(mLock);
    {
        mClients.remove(client);
#ifndef MULTI_PLAYER		
        if (mClients.size() == 0)
        {
            sp<TDMBPlayer> p = mPlayer;

            mPlayer.clear();

            if (p != 0)
            {
                p->setNotifyCallback(0, 0);
                p->release();
            }
        }
#endif
    }
}

void DMBService::notify(void* cookie, int msg, int ext1, int ext2, const Parcel *obj)
{
    //ALOGV("%s", __func__);
    DMBService* service = static_cast<DMBService*>(cookie);
    if (service == NULL) {
        return;
    }
	
    for (int i = 0, n = service->mClients.size(); i < n; ++i) {
        sp<Client> c = service->mClients[i].promote();
        if (c != 0) c->notify(msg, ext1, ext2, obj);
    }
}

DMBService::Client::Client(const sp<DMBService>& service, pid_t pid, const sp<IDxbPlayerClient>& notify_client)
{
    ALOGV("%s", __func__);
    mPid = pid;
    mService = service;
    mNotifyClient = notify_client;
    mModuleIndex = -1;
}

DMBService::Client::~Client()
{
    ALOGV("%s", __func__);
    wp<Client> client(this);
    disconnect();

#ifdef MULTI_PLAYER
	if(mPlayer != 0) {
		mPlayer->setNotifyCallback(0, 0);
		mPlayer->release();
	}

	mPlayer.clear();
#endif

    mService->removeClient(client);
}

void DMBService::Client::notify(int msg, int ext1, int ext2, const Parcel *obj)
{
    //ALOGV("%s", __func__);

    if(mModuleIndex < 0 || mModuleIndex >= 2)
        return;
		
    Mutex::Autolock l(sLock[mModuleIndex]);
    if(mNotifyClient != 0 && mNotifyClient.get())
        mNotifyClient->notify(msg, ext1, ext2, obj);
}

void DMBService::Client::setPlayer(const sp<TDMBPlayer> &player)
{
    mPlayer = player;
}

void DMBService::Client::disconnect()
{
    ALOGV("%s", __func__);

    if(mModuleIndex < 0 || mModuleIndex >= 2)
    {
        ALOGV("%s mModuleIndex err(%d)", __func__, mModuleIndex);
    }
    else
    {
        Mutex::Autolock l(sLock[mModuleIndex]);

        if(mModuleIndex == 0)
        {
            if (mService->mState == PLAYING)
            {
                mPlayer->stop(mModuleIndex);
            }
            mService->mState = STOPPED;
        }
        else
        {
            mPlayer->stop(mModuleIndex);
        }
    }

    mNotifyClient.clear();
    //mPlayer.clear();
}

status_t DMBService::Client::setVideoSurfaceTexture(const sp<IGraphicBufferProducer>& bufferProducer)
{
    ALOGV("%s", __func__);
    mPlayer->setVideoSurfaceTexture(bufferProducer);
    return OK;
}

status_t DMBService::Client::prepare(const int bbType, const int idx, const char *pszTDMBDBPath)
{
    ALOGV("%s, type:%d, idx:%d, %s", __func__, bbType, idx, pszTDMBDBPath);

    if (mService->mState == UNPREPARED)
	    mService->mState = STOPPED;

    mPlayer->prepare(bbType, idx, (char *)pszTDMBDBPath);

    return OK;
}

status_t DMBService::Client::start(int country_code)
{
    ALOGV("%s", __func__);

    if (mService->mState == STOPPED)
    {
        mPlayer->start(country_code);
    }
    mService->mState = PLAYING;

    return OK;
}

status_t DMBService::Client::setChannel(int rowID, int audioIndex, int subtitleIndex, int audioMode, int raw_w, int raw_h, int view_w, int view_h, int ch_index)
{
    ALOGV("%s::%d", __func__, subtitleIndex);
    if(subtitleIndex == 0)
    {
        if (mService->mState == PLAYING)
        {
            mPlayer->setChannelIndex(rowID, audioIndex, subtitleIndex);
        }
    }
    else
    {
        mPlayer->setChannelIndex(rowID, audioIndex, subtitleIndex);
    }

    return OK;
}

status_t DMBService::Client::stop(int idx)
{
    ALOGV("%s", __func__);

    if(idx == 0)
    {
        if (mService->mState == PLAYING)
        {
            mPlayer->stop(idx);
        }
        mService->mState = STOPPED;
    }
    else
    {
        mPlayer->stop(idx);
    }

    return OK;
}

status_t DMBService::Client::setParameter(int key, const Parcel &request)
{
    ALOGV("%s", __func__);

    int ret = OK;

    switch (key)
    {
        case DMBPlayerClient::USE_SURFACE:
        {
            int arg = request.readInt32();
            ret = useSurface(arg);
        }
        break;
        case DMBPlayerClient::RELEASE_SURFACE:
        {
            ret = releaseSurface();
        }
        break;
        case DMBPlayerClient::SEARCH:
        {
            int module_idx = request.readInt32();
            int countryCode = request.readInt32();
            int freqListCount = request.readInt32();
            for(int i=0; i<freqListCount; i++)
            {
                freqList[i] = request.readInt32();
            }
            ret = search(module_idx, countryCode, freqList, freqListCount);
        }
        break;
        case DMBPlayerClient::STOP_SEARCH:
        {
            int bbidx = request.readInt32();
            ret = stopsearch(bbidx);
        }
        break;
        case DMBPlayerClient::MANUAL_SEARCH:
        {
            int bbidx = request.readInt32();
            int freq = request.readInt32();
            ret = manual_search(bbidx, freq);
        }
        break;
        case DMBPlayerClient::SET_MANUAL:
        {
            int module_idx = request.readInt32();

            TDMB_INFO Info;
            memcpy((unsigned char*)&Info, request.data() + request.dataPosition(), sizeof(Info));
        /*
            ALOGD("moduleidx : %d", Info.moduleidx);
            ALOGD("Ensemble_Freq : %d", Info.Ensemble_Freq);
            ALOGD("Ensemble_Id : %d", Info.Ensemble_Id);
            ALOGD("Service_Id : %d", Info.Service_Id);
            ALOGD("Service_Label : %s", Info.Service_Label);
            ALOGD("Channel_Id : %d", Info.Channel_Id);
            ALOGD("Channel_Type : %d", Info.Channel_Type);
            ALOGD("Reg : (%02X, %02X, %02X, %02X, %02X, %02X, %02X)", Info.Reg[0], Info.Reg[1], Info.Reg[2], Info.Reg[3], Info.Reg[4], Info.Reg[5], Info.Reg[6]);
        */

            ret = manual_channel(module_idx, (int)&Info);
        }
        break;
	case DMBPlayerClient::REC_STOP:
        {
            ret = setRecStop();
        }
        break;
        case DMBPlayerClient::RECORD:
        {
            char filePath[255];
            memset(filePath, 0, 255);
            memcpy(filePath, request.data(), request.dataAvail());
            ret = setRecord(filePath);
        }
        break;
        case DMBPlayerClient::CAPTURE:
        {
            char filePath[255];
            memset(filePath, 0, 255);
            memcpy(filePath, request.data(), request.dataAvail());
            ret = setCapture(filePath);
        }
        break;
        case DMBPlayerClient::DISP_UPDATE:
        {
            ret = setLCDUpdate();
        }
        break;
        case DMBPlayerClient::DISP_ENABLE:
        {
            ret = setDisplayEnable();
        }
        break;
        case DMBPlayerClient::DISP_DISABLE:
        {
            ret = setDisplayDisable();
        }
        break;
        case DMBPlayerClient::SET_MODULE:
        {
            int bbidx = request.readInt32();
            int module_idx = request.readInt32();
            ret = setBBModuleIndex(bbidx, module_idx);
        }
        break;
        case DMBPlayerClient::AUDIO_MUTE:
        {
            int isMute = request.readInt32();
            ret = setAudioMute(isMute);
        }
        break;
//        case DMBPlayerClient::SIGNAL_STRENGTH:
//        {
//            mModuleIndex = request.readInt32();
//        }
//        break;
    }
    return ret;
}

status_t DMBService::Client::getParameter(int key, Parcel *reply, int idx)
{
	/*
	  0 : antenna level
	  1 : snr value
	  2 : pcber value
	  3 : rssi value
	  4 : vber value
	*/
    //ALOGV("%s", __func__);
    switch (key)
    {
        case DMBPlayerClient::SIGNAL_STRENGTH:
        {
            int pSignalStrength[14] = {100, 100, 0, 0, 0};
            pSignalStrength[3] = idx;
            //ALOGV("DMBPlayerClient::SIGNAL_STRENGTH::%d", idx);
            getSignalStrength(pSignalStrength);
            reply->writeInt32(pSignalStrength[0]);
            reply->writeInt32(pSignalStrength[1]);
            reply->writeInt32(pSignalStrength[2]);
            reply->writeInt32(pSignalStrength[3]);
            reply->writeInt32(pSignalStrength[4]);
            reply->writeInt32(pSignalStrength[5]);
            reply->writeInt32(pSignalStrength[6]);
            reply->writeInt32(pSignalStrength[7]);
            reply->writeInt32(pSignalStrength[8]);
            reply->writeInt32(pSignalStrength[9]);
            reply->writeInt32(pSignalStrength[10]);
            reply->writeInt32(pSignalStrength[11]);
            reply->writeInt32(pSignalStrength[12]);
            reply->writeInt32(pSignalStrength[13]);
        }
        break;
    }
    return OK;
}

int DMBService::Client::useSurface(int arg)
{
    ALOGV("%s", __func__);
    return mPlayer->useSurface(arg);
}

int DMBService::Client::releaseSurface()
{
    ALOGV("%s", __func__);
    return mPlayer->releaseSurface();
}

int DMBService::Client::search(int module_idx, int countryCode, int *pFreqList, int freqListCount)
{
    ALOGV("%s", __func__);
    return mPlayer->search(module_idx, countryCode, pFreqList, freqListCount);
}

int DMBService::Client::stopsearch(int module_idx)
{
    ALOGV("%s", __func__);
    return mPlayer->stopsearch(module_idx);
}

int DMBService::Client::manual_search(int module_idx, int freq)
{
    ALOGV("%s", __func__);
    return mPlayer->manual_search(module_idx, freq);
}

int DMBService::Client::manual_channel(int module_idx, int arg)
{
    ALOGV("%s", __func__);
	return mPlayer->manual_setChannel(module_idx, arg);
}

int DMBService::Client::setRecStop()
{
    ALOGV("%s", __func__);
    return mPlayer->setRecStop();
}

int DMBService::Client::setRecord(char *filePath)
{
    ALOGV("%s", __func__);
    return mPlayer->setRecord(filePath);
}

int DMBService::Client::setCapture(char *filePath)
{
    ALOGV("%s", __func__);
    return mPlayer->setCapture(filePath);
}

#if 0
int DMBService::Client::setChannelIndex(int rowID, int type)
{
    ALOGV("%s", __func__);
    return mPlayer->setChannelIndex(rowID, type);
}
#endif

int DMBService::Client::setLCDUpdate()
{
    ALOGV("%s", __func__);
    return mPlayer->setLCDUpdate();
}

int DMBService::Client::setDisplayEnable()
{
    ALOGV("%s", __func__);
    return mPlayer->setDisplayEnable();
}

int DMBService::Client::setDisplayDisable()
{
    ALOGV("%s", __func__);
    return mPlayer->setDisplayDisable();
}

int DMBService::Client::setAudioMute(int isMute)
{
    ALOGV("%s", __func__);
    return mPlayer->setAudioMute(isMute);
}

int DMBService::Client::setBBModuleIndex(int bbidx, int module_idx)
{
    ALOGV("%s", __func__);
    mModuleIndex = module_idx;
    return mPlayer->setBBModuleIndex(bbidx, module_idx);
}

int DMBService::Client::getSignalStrength(void *pSignalStrength)
{
    //ALOGV("%s", __func__);
    return mPlayer->getSignalStrength(pSignalStrength);
}

}; // namespace android

