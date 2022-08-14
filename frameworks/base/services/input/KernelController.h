/*
 * Copyright (C) 2011 Telechips, Inc.
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

#ifndef _KERNEL_SPRITES_H
#define _KERNEL_SPRITES_H

namespace android {

class KernelController : public MessageHandler {
protected:
    virtual ~KernelController();

public:
    KernelController(const sp<Looper>& looper);

    void openTransaction();
    void closeTransaction();

    void setIcon(const SpriteIcon& icon);
    void setVisible(bool visible);
    void setPosition(float x, float y);

private:
    enum {
        MSG_UPDATE_SPRITES,
        MSG_DISPOSE_SURFACES,
    };

    enum {
        DIRTY_BITMAP = 1 << 0,
        DIRTY_POSITION = 1 << 1,
        DIRTY_VISIBILITY = 1 << 2,
        DIRTY_HOTSPOT = 1 << 3,
    };

    sp<Looper> mLooper;
    sp<WeakMessageHandler> mHandler;
    int mDevFB;

    mutable Mutex mLock;

    struct KernelSpriteState {
        inline KernelSpriteState() :
                dirty(0), visible(false), positionX(0), positionY(0) {
        }

        uint32_t dirty;

        SpriteIcon icon;
        bool visible;
        float positionX;
        float positionY;
    };

    struct Locked {
        KernelSpriteState state;

        uint32_t transactionNestingCount;
        bool deferredSpriteUpdate;
    } mLocked; // guarded by mLock

    void handleMessage(const Message& message);
    void invalidateLocked(uint32_t dirty);
    void invalidateSpriteLocked();
    void doUpdateSprites();
};

} // namespace android

#endif // _KERNEL_SPRITES_H
