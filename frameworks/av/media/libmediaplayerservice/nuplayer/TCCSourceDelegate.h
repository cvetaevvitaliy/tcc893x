#define LOG_TAG "TCCSourceDelegate"
#include <utils/Log.h>

namespace android {
template <typename T>
struct TCCSourceDelegate  : public RefBase {
public:
    TCCSourceDelegate() {
    }

    TCCSourceDelegate(sp<T> _subClass) {
        subClass = _subClass;
    }

    void setSubClass(sp<T> _subClass) {
        subClass = _subClass;
    }

    ~TCCSourceDelegate() {
    }

    status_t dequeueAccessUnit(bool audio, sp<ABuffer> *accessUnit) {
       return subClass->dequeueAccessUnit(audio, accessUnit);
    }
    void onDisconnected(const sp<AMessage> &msg) {
       subClass->onDisconnected(msg);
    }

private:
    sp<T> subClass;
};

}  // namespace android
