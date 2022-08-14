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

#ifndef I_DXB_PLAYER_SERVICE_H
#define I_DXB_PLAYER_SERVICE_H

#include <IDxbPlayerClient.h>
#include <IDxbPlayer.h>

namespace android {

class IDxbPlayerService : public IInterface {
public:
    DECLARE_META_INTERFACE(DxbPlayerService);

    virtual sp<IDxbPlayer> create(pid_t pid, const sp<IDxbPlayerClient>& client) = 0;
    virtual int changeService(int standard) = 0;
};

class BnDxbPlayerService : public BnInterface<IDxbPlayerService> {
public:
    virtual status_t onTransact(uint32_t code,
                                const Parcel& data,
                                Parcel* reply,
                                uint32_t flags = 0);
};

}; // namespace android

#endif // I_DXB_PLAYER_SERVICE_H
