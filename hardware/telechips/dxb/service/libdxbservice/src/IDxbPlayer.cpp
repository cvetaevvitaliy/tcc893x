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

#include <IDxbPlayer.h>

namespace android {

enum {
    DISCONNECT = IBinder::FIRST_CALL_TRANSACTION,
    SET_VIDEO_SURFACETEXTURE,
    PREPARE,
    START,
    SET_CHANNEL,
    STOP,
    SET_PARAMETER,
    GET_PARAMETER,
};

class BpDxbPlayer : public BpInterface<IDxbPlayer>
{
public:
    BpDxbPlayer(const sp<IBinder>& impl)
        : BpInterface<IDxbPlayer>(impl)
    {
    }

    void disconnect()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IDxbPlayer::getInterfaceDescriptor());
        remote()->transact(DISCONNECT, data, &reply);
    }

    status_t setVideoSurfaceTexture(const sp<IGraphicBufferProducer>& bufferProducer)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IDxbPlayer::getInterfaceDescriptor());
        sp<IBinder> b(bufferProducer->asBinder());
        data.writeStrongBinder(b);
        remote()->transact(SET_VIDEO_SURFACETEXTURE, data, &reply);
        return reply.readInt32();
    }

    status_t prepare(const int bbType, const int idx, const char *pszDVBTDBPath)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IDxbPlayer::getInterfaceDescriptor());
        data.writeInt32(bbType);
		data.writeInt32(idx);
        if (pszDVBTDBPath != NULL)
            data.writeCString(pszDVBTDBPath);
        remote()->transact(PREPARE, data, &reply);
        return reply.readInt32();
    }

    status_t start(int country_code)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IDxbPlayer::getInterfaceDescriptor());
        data.writeInt32(country_code);
        remote()->transact(START, data, &reply);
        return reply.readInt32();
    }

	status_t setChannel(int mainRowID, int subRowID, int audioIndex, int audioMode, int raw_w, int raw_h, int view_w, int view_h, int ch_index)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IDxbPlayer::getInterfaceDescriptor());
        data.writeInt32(mainRowID);
        data.writeInt32(subRowID);
        data.writeInt32(audioIndex);
        data.writeInt32(audioMode);
        data.writeInt32(raw_w);
        data.writeInt32(raw_h);		
        data.writeInt32(view_w);		
        data.writeInt32(view_h);
        data.writeInt32(ch_index);
        remote()->transact(SET_CHANNEL, data, &reply);
        return reply.readInt32();
    }

    status_t stop(int idx)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IDxbPlayer::getInterfaceDescriptor());
        data.writeInt32(idx);
        remote()->transact(STOP, data, &reply);
        return reply.readInt32();
    }

    status_t setParameter(int key, const Parcel &request)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IDxbPlayer::getInterfaceDescriptor());
        data.writeInt32(key);
        if (request.dataSize() > 0) {
            data.appendFrom(const_cast<Parcel *>(&request), 0, request.dataSize());
        }
        remote()->transact(SET_PARAMETER, data, &reply);
        return reply.readInt32();
    }

    status_t getParameter(int key, Parcel *reply, int idx)
    {
        Parcel data;
        data.writeInterfaceToken(IDxbPlayer::getInterfaceDescriptor());
        data.writeInt32(key);
        data.writeInt32(idx);
        return remote()->transact(GET_PARAMETER, data, reply);
    }
};

IMPLEMENT_META_INTERFACE(DxbPlayer, "com.telechips.android.IDxbPlayer");

status_t BnDxbPlayer::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code)
    {
        case DISCONNECT:
        {
            CHECK_INTERFACE(IDxbPlayer, data, reply);
            disconnect();
            return NO_ERROR;
        }
        break;
        case SET_VIDEO_SURFACETEXTURE:
        {
            CHECK_INTERFACE(IDxbPlayer, data, reply);
            const sp<IGraphicBufferProducer>& bufferProducer =
                    interface_cast<IGraphicBufferProducer>(data.readStrongBinder());
            reply->writeInt32(setVideoSurfaceTexture(bufferProducer));
            return NO_ERROR;
        }
        break;
        case PREPARE:
        {
            CHECK_INTERFACE(IDxbPlayer, data, reply);
			int bbtype = data.readInt32();
			int idx = data.readInt32();
			char const* pszData = data.readCString();
            reply->writeInt32(prepare(bbtype, idx, pszData));
            return NO_ERROR;
        }
        break;
        case START:
        {
            CHECK_INTERFACE(IDxbPlayer, data, reply);
            int country_code = data.readInt32();
            reply->writeInt32(start(country_code));
            return NO_ERROR;
        }
        break;
        case SET_CHANNEL:
        {
            CHECK_INTERFACE(IDxbPlayer, data, reply);
            int mainRowID = data.readInt32();
            int subRowID = data.readInt32();
            int audioIndex = data.readInt32();
            int audioMode = data.readInt32();
            int raw_w = data.readInt32();
            int raw_h = data.readInt32();
            int view_w = data.readInt32();
            int view_h = data.readInt32();
            int ch_index = data.readInt32();
            reply->writeInt32(setChannel(mainRowID, subRowID, audioIndex, audioMode, raw_w, raw_h, view_w, view_h, ch_index));
            return NO_ERROR;
        }
        break;
        case STOP:
        {
            CHECK_INTERFACE(IDxbPlayer, data, reply);
			int moduleidx = data.readInt32();
            reply->writeInt32(stop(moduleidx));
            return NO_ERROR;
        }
        break;
        case SET_PARAMETER:
        {
            CHECK_INTERFACE(IDxbPlayer, data, reply);
            int key = data.readInt32();
            Parcel request;
            if (data.dataAvail() > 0) {
                request.appendFrom(
                        const_cast<Parcel *>(&data), data.dataPosition(), data.dataAvail());
            }
            request.setDataPosition(0);
            reply->writeInt32(setParameter(key, request));
            return NO_ERROR;
        }
        break;
        case GET_PARAMETER:
        {
            CHECK_INTERFACE(IDxbPlayer, data, reply);
            int key = data.readInt32();
            int moduleidx = data.readInt32();
            return getParameter(key, reply, moduleidx);
        }
        break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

}; // namespace android
