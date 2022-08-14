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

#include "ARTPAssembler.h"

#include <media/stagefright/foundation/AString.h>

#include <utils/List.h>
#include <utils/RefBase.h>

#include <OMX_Audio.h>

namespace android {

struct ABuffer;
struct AMessage;

struct TCCAMPEG4ElementaryAssembler {

    TCCAMPEG4ElementaryAssembler(void* _baseClass);
    ~TCCAMPEG4ElementaryAssembler();

    ARTPAssembler::AssemblyStatus addPacket(const sp<ARTPSource> &source);

private:
    void* baseClass;
};

}  // namespace android
