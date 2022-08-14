#include "TCCNuPlayer.h"
#include <media/stagefright/foundation/AMessage.h>
namespace android {
class TCCNuPlayerDelegate  : public RefBase {
public:
    TCCNuPlayerDelegate( sp<TCCNuPlayer> _subClass = NULL);
    virtual ~TCCNuPlayerDelegate();
    void onSourceNotify(const sp<AMessage> &msg);
    void setTCCRTSPSource(void* _rtspSource);
    void setOwner(void* owner);
private:
    sp<TCCNuPlayer> subClass;
    void*    mOwner;
};

}  // namespace android
