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

namespace android {
template <typename T>
struct TCCAMPEG4ElementaryAssemblerDelegate {
    TCCAMPEG4ElementaryAssemblerDelegate() {
    }

    ~TCCAMPEG4ElementaryAssemblerDelegate() {
    }

    void setSubClass(T* _subClass) {
        subClass = _subClass;
    }

    ARTPAssembler::AssemblyStatus addPacket(const sp<ARTPSource> &source) { 
        return subClass->addPacket(source);
    }

private:
    T* subClass;
};
}  // namespace android

