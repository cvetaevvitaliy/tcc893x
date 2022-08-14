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

#include <binder/Parcel.h>

#include <IDxbPlayerService.h>

namespace android {

enum {
    CREATE = IBinder::FIRST_CALL_TRANSACTION,
    CHANGE_SERVICE,
};

class BpDxbPlayerService : public BpInterface<IDxbPlayerService>
{
public:
    BpDxbPlayerService(const sp<IBinder>& impl)
        : BpInterface<IDxbPlayerService>(impl)
    {
    }

    virtual sp<IDxbPlayer> create(pid_t pid, const sp<IDxbPlayerClient>& client)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IDxbPlayerService::getInterfaceDescriptor());
        data.writeInt32(pid);
        data.writeStrongBinder(client->asBinder());
        remote()->transact(CREATE, data, &reply);
        sp<IDxbPlayer> player = interface_cast<IDxbPlayer>(reply.readStrongBinder());
        return player;
    }

    virtual int changeService(int standard)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IDxbPlayerService::getInterfaceDescriptor());
        data.writeInt32(standard);
        remote()->transact(CHANGE_SERVICE, data, &reply);
        return reply.readInt32();
    }
};

IMPLEMENT_META_INTERFACE(DxbPlayerService, "com.telechips.android.IDxbPlayerService");

status_t BnDxbPlayerService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code) {
        case CREATE: {
            CHECK_INTERFACE(IDxbPlayerService, data, reply);
            pid_t pid = data.readInt32();
            sp<IDxbPlayerClient> client =
                interface_cast<IDxbPlayerClient>(data.readStrongBinder());
            sp<IDxbPlayer> player = create(pid, client);
            reply->writeStrongBinder(player->asBinder());
            return NO_ERROR;
        }
        case CHANGE_SERVICE: {
            CHECK_INTERFACE(IDxbPlayerService, data, reply);
            int standard = data.readInt32();
            reply->writeInt32(changeService(standard));
            return NO_ERROR;
        }
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

}; // namespace android
