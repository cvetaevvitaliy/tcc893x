/****************************************************************************
 *   FileName    : WFDRTSPContoller.h
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-
distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall constitute any
express or implied warranty of any kind, including without limitation, any warranty of merchantability, 
fitness for a particular purpose or non-infringement of any patent, copyright or other third party intellectual 
property right. No warranty is made, express or implied, regarding the information's accuracy, 
completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of 
or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement 
between Telechips and Company.
*
****************************************************************************/

#ifndef A_WFD_CONTROLLER_H_

#define A_WFD_CONTROLLER_H_

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AHandlerReflector.h>
#include <media/stagefright/MediaExtractor.h>
#include <media/stagefright/DataSource.h>

namespace android {

struct ALooper;
struct WFDSinkHandler;

typedef enum
{
	UIBC_EVENT_TOUCH_ENABLE = 0,
	UIBC_EVENT_TOUCH_DISABLE,
	UIBC_EVENT_TOUCH_DOWN,
	UIBC_EVENT_TOUCH_UP,
	UIBC_EVENT_TOUCH_MOVE
}eUIBC_TOUCH_EVENT;


struct WFDRTSPController  : RefBase {
    WFDRTSPController(const sp<ALooper> &looper, const char *url);

    status_t connect();
    void disconnect();
    void prepareCancle();
    sp<DataSource> getDataSource();
    bool isWFDHandlerAlive();

    void pauseAsync();
    void playAsync();
    bool isPausing();
    bool getPauseState();

    int64_t getLastBufferTime();
    size_t approxDataRemaining(status_t *finalStatus);
    int64_t getFirstPacketTime(void);
    int32_t getFPS(void);
    int32_t getUIBCEnable();
    void    setUIBCEnable(int32_t enable);
    int32_t getUIBCSupport();
    void setUIBCSingleTouchEvent(float x_pos, float y_pos, int event);
    void onMessageReceived(const sp<AMessage> &msg);
    void initBufferClear();
    void setInfoMethods(void (*infoWrapper)(void *, int arg1, int arg2), void *cookie);


protected:
    virtual ~WFDRTSPController();

private:
    enum {
        kWhatConnectDone    = 2000,
        kWhatDisconnectDone,
        kWhatUIBC,
        kWhatM7CB,
        kWhatPauseCB,
        kWhatResumeCB,
        kWhatStandbyCB,
        kWhatPacketCB,
        kWhatErrorCB,
    };

    //mediaplayer.h
    enum {
        MEDIA_INFO_WFD_PAUSE = 1000,
        MEDIA_INFO_WFD_PLAY  = 1001,
        MEDIA_INFO_WFD_AUDIO_FRAME_SKIP  = 1002,
        MEDIA_INFO_WFD_VIDEO_FRAME_SKIP  = 1003,
        MEDIA_INFO_WFD_ERROR_INFO  = 1004,
        MEDIA_INFO_WFD_STANDBY  = 1005,
        MEDIA_INFO_WFD_HDCP_RETRY  = 1006,
        MEDIA_INFO_WFD_PACKET_LOSS = 1099,
        MEDIA_INFO_WFD_M7 = 1100,
        MEDIA_INFO_WFD_UIBC = 1101,
    };

    enum State {
        DISCONNECTED,
        CONNECTED,
        CONNECTING,
    };

    Mutex mLock;
    Condition mCondition;

    State mState;
    status_t mConnectionResult;
    sp<ALooper> mLooper;
    sp<WFDSinkHandler> mHandler;
    sp<AHandlerReflector<WFDRTSPController> > mReflector;

    void (*mInfoWrapper)(void *, int arg1, int arg2);
    void *mInfoCookie;

    DISALLOW_EVIL_CONSTRUCTORS(WFDRTSPController);
};

}  // namespace android

#endif  // A_WFD_CONTROLLER_H_
