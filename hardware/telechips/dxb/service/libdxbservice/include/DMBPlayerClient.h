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

#ifndef DMB_PLAYER_CLIENT_H
#define DMB_PLAYER_CLIENT_H

#include <DxbPlayerClient.h>

namespace android {

class DMBPlayerClient : public DxbPlayerClient {
public:
    DMBPlayerClient();

    enum {
        USE_SURFACE = 0,
        RELEASE_SURFACE,
        SEARCH,
        STOP_SEARCH,
        MANUAL_SEARCH,
        REC_STOP,
        RECORD,
        CAPTURE,
        DISP_UPDATE,
        DISP_ENABLE,
        AUDIO_MUTE,
        DISP_DISABLE,
        SET_MODULE,
        SET_MANUAL,
        SIGNAL_STRENGTH,
    };

    int   useSurface(int arg);
    int   releaseSurface();
    int   search(int countryCode, int *pFreqList, int freqListCount);
    int   stopsearch();
    int   manual_search(int freq);
    int   setRecStop();
    int   setRecord(char *filePath);
    int   setCapture(char *filePath);
    int   setLCDUpdate();
    int   setDisplayEnable();
    int   setDisplayDisable();
    int   setAudioMute(int isMute);
    int   setBBModuleIndex(unsigned int bbidx, unsigned int moduleidx);
    int   getSignalStrength(int *pSignalStrength);
    int   playprepare(const int bbType, const char *pszDVBTDBPath);
    int   playstop();
    int   setManualChannel(unsigned char *pArg, int len);

    status_t setChannelIndex(int mainRowID, int subRowID, int audioIndex, int audioMode = NULL, int raw_w = NULL, int raw_h = NULL, int view_w = NULL, int view_h = NULL, int ch_index = NULL);

protected:
    virtual ~DMBPlayerClient();

	int   mModuleIndex;
};

}; // namespace android

#endif // DMB_PLAYER_CLIENT_H
