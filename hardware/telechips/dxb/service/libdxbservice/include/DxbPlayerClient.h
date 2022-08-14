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

#ifndef DXB_PLAYER_CLIENT_H
#define DXB_PLAYER_CLIENT_H

#include <IDxbPlayerService.h>
#include <IDxbPlayerClient.h>
#include <IDxbPlayer.h>

#define USE_DEATH_RECIPIENT

namespace android {

class DxbPlayerClientListener: virtual public RefBase {
public:
    virtual void notify(int msg, int ext1, int ext2, const Parcel *obj) = 0;
};

class DxbPlayerClient : public BnDxbPlayerClient
#ifdef USE_DEATH_RECIPIENT	
    , private IBinder::DeathRecipient
#endif	
{
public:
    DxbPlayerClient();

    void     notify(int msg, int ext1, int ext2, const Parcel *obj = NULL);
    void     disconnect();

    status_t setListener(const sp<DxbPlayerClientListener>& listener);
    status_t setVideoSurfaceTexture(const sp<IGraphicBufferProducer>& bufferProducer);
    status_t prepare(char *pszDVBTDBPath = NULL);
    status_t prepare(int bbType, int idx = 0, char *pszDVBTDBPath = NULL);
    status_t start(int country_code = 0);
    status_t setChannel(int mainRowID, int subRowID, int audioIndex, int audioMode = NULL, int raw_w = NULL, int raw_h = NULL, int view_w = NULL, int view_h = NULL, int ch_index = NULL);
    status_t stop(int idx = 0);

protected:
    virtual ~DxbPlayerClient();

    const sp<IDxbPlayerService>& getService();

    sp<IDxbPlayer> mPlayer;
    sp<DxbPlayerClientListener> mListener;

private:
    sp<IDxbPlayerService> mService;

#ifdef USE_DEATH_RECIPIENT	
    virtual void binderDied(const wp<IBinder>& who);
#endif
};

}; // namespace android

#endif // DXB_PLAYER_CLIENT_H
