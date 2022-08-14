
#include "NuPlayerSource.h"

#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaData.h>

namespace android {

struct TCCRTSPSource  : public RefBase {

    TCCRTSPSource(void* _baseClass);
    virtual ~TCCRTSPSource();
    status_t dequeueAccessUnit(bool audio, sp<ABuffer> *accessUnit);
    void onDisconnected(const sp<AMessage> &msg);
private:
    void* mBaseClass;
};

}  // namespace android
