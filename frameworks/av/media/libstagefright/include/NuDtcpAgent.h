/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NU_DTCP_AGENT_H_

#define NU_DTCP_AGENT_H_

#include <utils/String8.h>
#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AHandlerReflector.h>

namespace android {

struct ALooper;

struct NuDtcpAgent : public RefBase {
    NuDtcpAgent(const char* host, int32_t port);
    
    enum {
        kWhatSinkInitBlocked   = 'sibk',
        kWhatSinkInitUnblocked = 'siuk',
    };

    enum {
        DTCP_STACK_DISCONNECTED = 0,       // TCP disconnected
        DTCP_STACK_CONNECTED,              // TCP connected to other device
        DTCP_STACK_AUTHENTICATED,          // TCP connected and Autenticated (AKE)
		DTCP_STACK_AUTHENTICATED_NO_CONN,  // Authenticated but not TCP connected
        DTCP_STACK_UNAUTHENTICATED,        // Restarted Authentication
        DTCP_STACK_ALIVE,                  // polling for 'keep running' (1x/sec)
        DTCP_STACK_ERROR,                  // unexpected termination
        DTCP_STACK_KEY_NOT_CONFIRMED,      // Key not confirmed
        DTCP_STACK_KEY_EXPIRED,            // Key expired
        DTCP_STACK_MAKE_CONTENT_USABLE,    // Move Commitment event
        DTCP_STACK_DISCARD_CONTENT,        // idem
        DTCP_STACK_COMMIT_FAIL,            // idem
        DTCP_STACK_COMMIT_DONE             // idem
    };

    bool start();
    void stop();

    status_t getStatus();
    status_t getStackStatus();
    bool checkStackStatus(int32_t status);
	void close();
    void onMessageReceived(const sp<AMessage> &msg);

protected:
    virtual ~NuDtcpAgent();

private:
    void * mLibHandle;
    int (*mInitFunc) (const char *addr, int port);
    int (*mStatusFunc) ();
	void (*mCloseFunc) ();

    sp<ALooper> mLooper;    
    sp<AHandlerReflector<NuDtcpAgent> > mReflector;

    status_t mStatus;

    String8 mHost;
    int32_t mPort;

    DISALLOW_EVIL_CONSTRUCTORS(NuDtcpAgent);
};

}  // namespace android

#endif  // NU_DTCP_AGENT_H_
