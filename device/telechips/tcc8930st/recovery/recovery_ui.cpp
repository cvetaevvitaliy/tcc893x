/*
 * Copyright (C) 2011 The Android Open Source Project
 * Copyright (C) 2012 Telechips, Inc.  All rights reserved.
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

#include <linux/input.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include "common.h"
#include "device.h"
#include "screen_ui.h"

const char* HEADERS[] = { "Up/Down button to move highlight;",
                          "Enter button to select.",
                          "Menu button to hide text.",
                          "",
                          NULL };

const char* ITEMS[] = { "reboot system now",
                        "apply update from ADB",
                        "apply update from SD card",
                        "apply update from USB device",
                        "wipe data/factory reset",
                        "wipe cache partition",
                       NULL };

#define REM_DPAD_UP             19
#define REM_DPAD_DOWN           20
#define REM_DPAD_LEFT           21
#define REM_DPAD_RIGHT          22
#define REM_ENTER               66
#define REM_MENU                82

class GrouperUI : public ScreenRecoveryUI {
  public:
    GrouperUI() :
        consecutive_power_keys(0) {
}

    virtual KeyAction CheckKey(int key) {
        if (IsKeyPressed(KEY_POWER) && key == KEY_VOLUMEUP) {
            return TOGGLE;
        }
        if (key == KEY_POWER) {
            ++consecutive_power_keys;
            if (consecutive_power_keys >= 7) {
                return REBOOT;
            }
        } else {
            consecutive_power_keys = 0;
        }
        return ENQUEUE;
}

  private:
    int consecutive_power_keys;
};


class GrouperDevice : public Device {
  public:
    GrouperDevice() :
        ui(new GrouperUI) {
        }

    RecoveryUI* GetUI() { return ui; }

    int HandleMenuKey(int key_code, int visible) {
        if (visible) {
            switch (key_code) {
                case KEY_DOWN:               // 108
                case KEY_VOLUMEDOWN:         // 114
                case REM_DPAD_DOWN:          // 20(KEY_T)
                    return kHighlightDown;

                case KEY_UP:                 // 103
                case KEY_VOLUMEUP:           // 115
                case REM_DPAD_UP:            // 19(KEY_R)
                    return kHighlightUp;

                case KEY_ENTER:              // 28
                case KEY_POWER:              // 116
                case REM_ENTER:              // 66(KEY_F8)
                    return kInvokeItem;
            }
        }

        switch (key_code) {
            case KEY_COMPOSE:            // 127
            case REM_MENU:               // 82(KEY_KP0)
                ui->ShowText(!visible);
        }

        return kNoAction;
    }

    BuiltinAction InvokeMenuItem(int menu_position) {
        switch (menu_position) {
            case 0: return REBOOT;
            case 1: return APPLY_ADB_SIDELOAD;
            case 2: return APPLY_EXT;
            case 3: return APPLY_USB;
            case 4: return WIPE_DATA;
            case 5: return WIPE_CACHE;
            default: return NO_ACTION;
        }
    }

    const char* const* GetMenuHeaders() { return HEADERS; }
    const char* const* GetMenuItems() { return ITEMS; }

  private:
    RecoveryUI* ui;
};

Device* make_device() {
    return new GrouperDevice;
}
