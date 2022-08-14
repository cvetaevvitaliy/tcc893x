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

#define LOG_TAG "KernelController"

//#define LOG_NDEBUG 0

#include "SpriteController.h"
#include "KernelController.h"

#include <fcntl.h>
#include <mach/tccfb_ioctrl.h>
#include <cutils/properties.h>

#define FB_DEV_NAME "/dev/graphics/fb0"

namespace android {

KernelController::KernelController(const sp<Looper>& looper) :
        mLooper(looper) {
    if ((mDevFB = open(FB_DEV_NAME, O_RDWR)) < 0) {
        ALOGE("Failed to open \"%s\"", FB_DEV_NAME);
    } else {
        ALOGI("Ready Kernel Pointer Controller");
    }

    mHandler = new WeakMessageHandler(this);

    mLocked.transactionNestingCount = 0;
    mLocked.deferredSpriteUpdate = false;
}

KernelController::~KernelController() {
    if (mDevFB >= 0) {
        close(mDevFB);
    }
}

void KernelController::openTransaction() {
    AutoMutex _l(mLock);

    mLocked.transactionNestingCount += 1;
}

void KernelController::closeTransaction() {
    AutoMutex _l(mLock);

    LOG_ALWAYS_FATAL_IF(mLocked.transactionNestingCount == 0,
            "Sprite closeTransaction() called but there is no open sprite transaction");

    mLocked.transactionNestingCount -= 1;
    if (mLocked.transactionNestingCount == 0 && mLocked.deferredSpriteUpdate) {
        mLocked.deferredSpriteUpdate = false;
        mLooper->sendMessage(mHandler, Message(MSG_UPDATE_SPRITES));
    }
}

void KernelController::invalidateSpriteLocked() {
    if (mLocked.transactionNestingCount != 0) {
        mLocked.deferredSpriteUpdate = true;
    } else {
        mLooper->sendMessage(mHandler, Message(MSG_UPDATE_SPRITES));
    }
}

void KernelController::handleMessage(const Message& message) {
    switch (message.what) {
    case MSG_UPDATE_SPRITES:
        doUpdateSprites();
        break;
    case MSG_DISPOSE_SURFACES:
        break;
    }
}

void KernelController::doUpdateSprites() {
    unsigned int arg[4];
    KernelSpriteState update;
    char value[PROPERTY_VALUE_MAX];

    property_get ("persist.sys.hdmi_mode", value, "");
    arg[2] = atoi(value);
	property_get ("persist.sys.output_mode", value, "");
	arg[2] |= (atoi(value)<<8);

    { // acquire lock
        AutoMutex _l(mLock);

        update = mLocked.state;
        arg[3] = (unsigned int)mLocked.state.icon.bitmap.getPixels();

        mLocked.state.dirty = 0;
    } // release lock

    if (update.dirty & DIRTY_BITMAP) {
        arg[0] = update.icon.bitmap.width();
        arg[1] = update.icon.bitmap.height();
        ioctl(mDevFB, TCC_LCDC_MOUSE_ICON, arg);
    }

    if ((update.dirty & DIRTY_VISIBILITY) && !update.visible) {
        if (update.visible) {
            arg[0] = 1;
        } else {
            arg[0] = 0;
        }
        ioctl(mDevFB, TCC_LCDC_MOUSE_SHOW, arg);
    }

    if ((update.dirty & (DIRTY_POSITION | DIRTY_VISIBILITY)) && update.visible) {
        arg[0] = update.positionX - update.icon.hotSpotX;
        arg[1] = update.positionY - update.icon.hotSpotY;
        ioctl(mDevFB, TCC_LCDC_MOUSE_MOVE, arg);
    }
}

void KernelController::setIcon(const SpriteIcon& icon) {
    AutoMutex _l(mLock);

    uint32_t dirty;
    if (icon.isValid()) {
        icon.bitmap.copyTo(&mLocked.state.icon.bitmap, SkBitmap::kARGB_8888_Config);
        if (!mLocked.state.icon.isValid()
                || mLocked.state.icon.hotSpotX != icon.hotSpotX
                || mLocked.state.icon.hotSpotY != icon.hotSpotY) {
            mLocked.state.icon.hotSpotX = icon.hotSpotX;
            mLocked.state.icon.hotSpotY = icon.hotSpotY;
            dirty = DIRTY_BITMAP | DIRTY_HOTSPOT;
        } else {
            dirty = DIRTY_BITMAP;
        }
    } else if (mLocked.state.icon.isValid()) {
        mLocked.state.icon.bitmap.reset();
        dirty = DIRTY_BITMAP | DIRTY_HOTSPOT;
    } else {
        return; // setting to invalid icon and already invalid so nothing to do
    }

    invalidateLocked(dirty);
}

void KernelController::setVisible(bool visible) {
    AutoMutex _l(mLock);

    if (mLocked.state.visible != visible) {
        mLocked.state.visible = visible;
        invalidateLocked(DIRTY_VISIBILITY);
    }
}

void KernelController::setPosition(float x, float y) {
    AutoMutex _l(mLock);

    if (mLocked.state.positionX != x || mLocked.state.positionY != y) {
        mLocked.state.positionX = x;
        mLocked.state.positionY = y;
        invalidateLocked(DIRTY_POSITION);
    }
}

void KernelController::invalidateLocked(uint32_t dirty) {
    bool wasDirty = mLocked.state.dirty;
    mLocked.state.dirty |= dirty;

    if (!wasDirty) {
        invalidateSpriteLocked();
    }
}

} // namespace android
