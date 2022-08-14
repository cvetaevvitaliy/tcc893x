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
#define LOG_TAG "DMBPlayerClient"
#include <utils/Log.h>

#include <DMBPlayerClient.h>

namespace android {

DMBPlayerClient::DMBPlayerClient() {
    ALOGV("%s", __func__);
	mModuleIndex = 0;
}

DMBPlayerClient::~DMBPlayerClient() {
    ALOGV("%s", __func__);
}

int DMBPlayerClient::useSurface(int arg) {
    ALOGV("%s", __func__);

    Parcel data;
    if (mPlayer != 0) {
        data.writeInt32(arg);
        return mPlayer->setParameter(USE_SURFACE, data);
    }
    return 0;
}

int DMBPlayerClient::releaseSurface() {
    ALOGV("%s", __func__);

    Parcel data;
    if (mPlayer != 0) {
        return mPlayer->setParameter(RELEASE_SURFACE, data);
    }
    return 0;
}

int DMBPlayerClient::search(int countryCode, int *pFreqList, int freqListCount) {
    ALOGV("%s", __func__);

    Parcel data;
    if (mPlayer != 0) {
		data.writeInt32(mModuleIndex);
        data.writeInt32(countryCode);
        data.writeInt32(freqListCount);
		for(int i=0; i<freqListCount; i++)
		{
			data.writeInt32(pFreqList[i]);
		}
        return mPlayer->setParameter(SEARCH, data);
    }
    return 0;
}

int DMBPlayerClient::stopsearch() {
    ALOGV("%s", __func__);

    Parcel data;
    if (mPlayer != 0) {
		data.writeInt32(mModuleIndex);
        return mPlayer->setParameter(STOP_SEARCH, data);
    }
    return 0;
}

int DMBPlayerClient::manual_search(int freq)
{
    Parcel data;
    if (mPlayer != 0) {
		data.writeInt32(mModuleIndex);
        data.writeInt32(freq);
        return mPlayer->setParameter(MANUAL_SEARCH, data);
    }
	return 0;
}

int DMBPlayerClient::setRecStop() {
    ALOGV("%s", __func__);

    Parcel data;
    if (mPlayer != 0) {
        return mPlayer->setParameter(REC_STOP, data);
    }
    return 0;
}

int DMBPlayerClient::setRecord(char *filePath) {
    ALOGV("%s", __func__);

    Parcel data;
    if (mPlayer != 0) {
        data.setData((uint8_t*)filePath, strlen(filePath));
        return mPlayer->setParameter(RECORD, data);
    }
    return 0;
}

int DMBPlayerClient::setCapture(char *filePath) {
    ALOGV("%s", __func__);

    Parcel data;
    if (mPlayer != 0) {
        data.setData((uint8_t*)filePath, strlen(filePath));
        return mPlayer->setParameter(CAPTURE, data);
    }
    return 0;
}

int DMBPlayerClient::setLCDUpdate() {
    ALOGV("%s", __func__);

    Parcel data;
    if (mPlayer != 0) {
        return mPlayer->setParameter(DISP_UPDATE, data);
    }
    return 0;
}

int DMBPlayerClient::setDisplayEnable() {
    ALOGV("%s", __func__);

    Parcel data;
    if (mPlayer != 0) {
        return mPlayer->setParameter(DISP_ENABLE, data);
    }
    return 0;
}

int DMBPlayerClient::setDisplayDisable() {
    ALOGV("%s", __func__);

    Parcel data;
    if (mPlayer != 0) {
        return mPlayer->setParameter(DISP_DISABLE, data);
    }
    return 0;
}

int DMBPlayerClient::setAudioMute(int isMute)
{
    ALOGV("%s", __func__);

    Parcel data;
    if (mPlayer != 0) {
		data.writeInt32((int)isMute);
        return mPlayer->setParameter(AUDIO_MUTE, data);
    }
    return 0;
}

int DMBPlayerClient::setBBModuleIndex(unsigned int bbidx, unsigned int moduleidx)
{
    ALOGD("%s::%d", __func__, moduleidx);

	mModuleIndex = moduleidx;

    Parcel data;
    if (mPlayer != 0) {
		data.writeInt32(bbidx);
        data.writeInt32(moduleidx);
        return mPlayer->setParameter(SET_MODULE, data);
    }
    return 0;
}

int DMBPlayerClient::setManualChannel(unsigned char *pArg, int len)
{
    ALOGD("%s::%d", __func__, mModuleIndex);

    Parcel data, arg;
    if (mPlayer != 0) {
        data.writeInt32(mModuleIndex);
        arg.setData(pArg, len);
        data.appendFrom(const_cast<Parcel *>(&arg), 0, arg.dataSize());
        return mPlayer->setParameter(SET_MANUAL, data);
    }
    return 0;
}

status_t DMBPlayerClient::setChannelIndex(int mainRowID, int subRowID, int audioIndex, int audioMode, int raw_w, int raw_h, int view_w, int view_h, int ch_index) {
    ALOGD("%s::%d", __func__, mModuleIndex);

    if (mPlayer != 0)
	{
        setChannel(mainRowID, subRowID, mModuleIndex, audioMode, raw_w, raw_h, view_w, view_h, ch_index);
    }
    return OK;
}

int DMBPlayerClient::getSignalStrength(int *pSignalStrength) {
    ALOGV("%s:%d", __func__, mModuleIndex);

    Parcel request;
    int ret = 0;
    if (mPlayer != 0) {
//        request.writeInt32(mModuleIndex);
//        mPlayer->setParameter(SIGNAL_STRENGTH, request);

        ret = mPlayer->getParameter(SIGNAL_STRENGTH, &request, mModuleIndex);
        pSignalStrength[0] = request.readInt32();
        pSignalStrength[1] = request.readInt32();
        pSignalStrength[2] = request.readInt32();
        pSignalStrength[3] = request.readInt32();
        pSignalStrength[4] = request.readInt32();
        pSignalStrength[5] = request.readInt32();
        pSignalStrength[6] = request.readInt32();
        pSignalStrength[7] = request.readInt32();
        pSignalStrength[8] = request.readInt32();
        pSignalStrength[9] = request.readInt32();
        pSignalStrength[10] = request.readInt32();
        pSignalStrength[11] = request.readInt32();
        pSignalStrength[12] = request.readInt32();
        pSignalStrength[13] = request.readInt32();
    }
    return ret;
}

int DMBPlayerClient::playprepare(const int bbType, const char *pszDVBTDBPath)
{
    ALOGV("%s:%d", __func__, mModuleIndex);

    if (mPlayer != 0) {
        mPlayer->prepare(bbType, mModuleIndex, pszDVBTDBPath);
    }
    return 0;
}

int DMBPlayerClient::playstop() {
    ALOGV("%s:%d", __func__, mModuleIndex);

    if (mPlayer != 0) {
        mPlayer->stop(mModuleIndex);
    }
    return 0;
}

}; // namespace android
