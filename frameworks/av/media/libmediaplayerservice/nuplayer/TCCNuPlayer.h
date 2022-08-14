#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaData.h>

namespace android {

struct TCCNuPlayer : public RefBase {
public:
    TCCNuPlayer(void* _baseClass);
    virtual ~TCCNuPlayer();
    void onSourceNotify(const sp<AMessage> &msg);
    void* baseClass;

    DISALLOW_EVIL_CONSTRUCTORS(TCCNuPlayer);
};

}  // namespace android
