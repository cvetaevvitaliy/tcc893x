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

//#define LOG_NDEBUG 0
#define LOG_TAG "DxbPlayerClient"
#include <utils/Log.h>

#include <binder/IServiceManager.h>

#include <DxbPlayerClient.h>

#define NOTIFY_SERVICE_DIED	99

namespace android {

const sp<IDxbPlayerService>& DxbPlayerClient::getService()
{
    if (mService == 0) {
        sp<IServiceManager> sm = defaultServiceManager();
        sp<IBinder> binder;
        do {
            binder = sm->getService(String16("tcc.dxb.server"));
            if (binder != 0) {
                break;
            }
            ALOGW("DxbPlayerService not published, waiting...");
            usleep(500000); // 0.5 s
        } while (true);
        mService = interface_cast<IDxbPlayerService>(binder);
    }
    return mService;
}

#ifdef USE_DEATH_RECIPIENT
void DxbPlayerClient::binderDied(const wp<IBinder>& who)
{
	ALOGW("%s", __func__);
	notify(NOTIFY_SERVICE_DIED, 0, 0, NULL);
}
#endif

DxbPlayerClient::DxbPlayerClient() {
    ALOGV("%s", __func__);

    const sp<IDxbPlayerService>& service(getService());
    mPlayer = 0;
    mListener = 0;
    if (service != 0) {
        mPlayer = service->create(getpid(), this);
#ifdef USE_DEATH_RECIPIENT
        mService->asBinder()->linkToDeath(static_cast<IBinder::DeathRecipient*>(this));
    /*
        if(mPlayer != 0)
        mPlayer->asBinder()->linkToDeath(static_cast<IBinder::DeathRecipient*>(this));
    */
#endif
    }
}

DxbPlayerClient::~DxbPlayerClient() {
    ALOGV("%s", __func__);
#ifdef USE_DEATH_RECIPIENT
    if(mService != 0)
        mService->asBinder()->unlinkToDeath(static_cast<IBinder::DeathRecipient*>(this));
#endif

    mPlayer.clear();
    mListener.clear();
    mService.clear();
}

void DxbPlayerClient::notify(int msg, int ext1, int ext2, const Parcel *obj) {
	//ALOGV("%s", __func__);
	
    if (mListener != 0) {
        mListener->notify(msg, ext1, ext2, obj);
    }
}

void DxbPlayerClient::disconnect() {
    if (mPlayer != 0) {
        mPlayer->disconnect();
        mPlayer.clear();
    }
}

status_t DxbPlayerClient::setListener(const sp<DxbPlayerClientListener>& listener) {
    mListener = listener;
    return OK;
}

status_t DxbPlayerClient::setVideoSurfaceTexture(const sp<IGraphicBufferProducer>& bufferProducer) {
    if (mPlayer != 0) {
        mPlayer->setVideoSurfaceTexture(bufferProducer);
    }
    return OK;
}

status_t DxbPlayerClient::prepare(char *pszDVBTDBPath) {
    if (mPlayer != 0) {
        mPlayer->prepare(0, 0, pszDVBTDBPath);
    }
    return OK;
}

status_t DxbPlayerClient::prepare(int idx, int bbType, char *pszDVBTDBPath) {
    if (mPlayer != 0) {
        mPlayer->prepare(idx, bbType, pszDVBTDBPath);
    }
    return OK;
}

status_t DxbPlayerClient::start(int country_code) {
    if (mPlayer != 0) {
        mPlayer->start(country_code);
    }
    return OK;
}

status_t DxbPlayerClient::setChannel(int mainRowID, int subRowID, int audioIndex, int audioMode, int raw_w, int raw_h, int view_w, int view_h, int ch_index) {
    if (mPlayer != 0) {
        mPlayer->setChannel(mainRowID, subRowID, audioIndex, audioMode, raw_w, raw_h, view_w, view_h, ch_index);
    }
    return OK;
}

status_t DxbPlayerClient::stop(int idx) {
    if (mPlayer != 0) {
        mPlayer->stop(idx);
    }
    return OK;
}

}; // namespace android
