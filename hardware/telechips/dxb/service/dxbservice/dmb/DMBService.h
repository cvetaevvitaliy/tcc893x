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

#ifndef TCC_DMB_PLAYSER_SERVICE_H
#define TCC_DMB_PLAYSER_SERVICE_H

#include <libpmap/pmap.h>
#include <media/hardware/HardwareAPI.h>
#include <utils/SortedVector.h>

#include <IDxbPlayerService.h>

#include <TDMBPlayer.h>

#define MULTI_PLAYER

namespace android {

class DMBService : public BnDxbPlayerService {

    class Client;

public:
    static int instantiate();
    void    removeClient(wp<Client> client);
    virtual sp<IDxbPlayer> create(pid_t pid, const sp<IDxbPlayerClient>& client);
    virtual int changeService(int standard);

    static void notify(void* cookie, int msg, int ext1, int ext2, const Parcel *obj);

#ifndef MULTI_PLAYER
    sp<TDMBPlayer>       mPlayer;
#endif
    int                  mState;

private:
    class Client : public BnDxbPlayer {

    	friend class DMBService;

        pid_t                mPid;
        sp<DMBService>       mService;
        sp<TDMBPlayer>       mPlayer;
        sp<IDxbPlayerClient> mNotifyClient;

        int mModuleIndex;

        void notify(int msg, int ext1, int ext2, const Parcel *obj);
        void setPlayer(const sp<TDMBPlayer> &player);

        Client(const sp<DMBService>& service, pid_t pid, const sp<IDxbPlayerClient>& notify_client);
        ~Client();

        // Common
        virtual void     disconnect();
        virtual status_t setVideoSurfaceTexture(const sp<IGraphicBufferProducer>& bufferProducer);
        virtual status_t prepare(const int bbType, const int idx, const char *pszTDMBDBPath);
        virtual status_t start(int country_code);
		virtual status_t setChannel(int rowID, int audioIndex, int subtitleIndex, int audioMode, int raw_w, int raw_h, int view_w, int view_h, int ch_index);
        virtual status_t stop(int idx);
        virtual status_t setParameter(int key, const Parcel &request);
        virtual status_t getParameter(int key, Parcel *reply, int idx);

        int   useSurface(int arg);
        int   releaseSurface();
        int   search(int moduleidx, int countryCode, int *pFreqList, int freqListCount);
        int   stopsearch(int moduleidx);
        int   manual_search(int moduleidx, int freq);
        int   manual_channel(int module_idx, int arg);
        int   setRecStop();
        int   setRecord(char *filePath);
        int   setCapture(char *filePath);
		int   setLCDUpdate();
		int   setDisplayEnable();
		int   setDisplayDisable();
		int   setAudioMute(int isMute);
		int   setBBModuleIndex(int bbidx, int moduleidx);
        int   getSignalStrength(void *pSignalStrength);
    };

    DMBService();
    virtual ~DMBService();

    mutable Mutex mLock;
    SortedVector< wp<Client> > mClients;
};

}; // namespace

#endif //TCC_DMB_PLAYSER_SERVICE_H
