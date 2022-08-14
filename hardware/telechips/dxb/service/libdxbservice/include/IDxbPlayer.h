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

#ifndef I_DXB_PLAYER_H
#define I_DXB_PLAYER_H

#include <binder/IInterface.h>
#include <gui/Surface.h>

namespace android {

class IDxbPlayer : public IInterface {
public:
    DECLARE_META_INTERFACE(DxbPlayer);

    virtual void     disconnect() = 0;

	virtual status_t setVideoSurfaceTexture(const sp<IGraphicBufferProducer>& bufferProducer) = 0;
    virtual status_t prepare(const int bbType, const int idx, const char *pszDVBTDBPath) = 0;
    virtual status_t start(int country_code) = 0;
    virtual status_t setChannel(int mainRowID, int subRowID, int audioIndex, int audioMode, int raw_w, int raw_h, int view_w, int view_h, int ch_index) = 0;
    virtual status_t stop(int idx) = 0;
    virtual status_t setParameter(int key, const Parcel &request) = 0;
    virtual status_t getParameter(int key, Parcel *reply, int idx=0) = 0;
};

class BnDxbPlayer : public BnInterface<IDxbPlayer> {
public:
    virtual status_t onTransact(uint32_t code,
                                const Parcel& data,
                                Parcel* reply,
                                uint32_t flags = 0);
};

}; // namespace android

#endif // I_DXB_PLAYER_H
