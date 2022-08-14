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

#ifndef _TCC_OMX_PLUGIN_H
#define _TCC_OMX_PLUGIN_H

#include <media/hardware/OMXPluginBase.h>

namespace android {

struct TCCOMXPlugin : public OMXPluginBase {
    TCCOMXPlugin();
    virtual ~TCCOMXPlugin();

    virtual OMX_ERRORTYPE makeComponentInstance(
            const char *name,
            const OMX_CALLBACKTYPE *callbacks,
            OMX_PTR appData,
            OMX_COMPONENTTYPE **component);

    virtual OMX_ERRORTYPE destroyComponentInstance(
            OMX_COMPONENTTYPE *component);

    virtual OMX_ERRORTYPE enumerateComponents(
            OMX_STRING name,
            size_t size,
            OMX_U32 index);

    virtual OMX_ERRORTYPE getRolesOfComponent(
            const char *name,
            Vector<String8> *roles);

    virtual OMX_ERRORTYPE setupTunnel(
            OMX_COMPONENTTYPE *outputComponent,
            OMX_U32 outputPortIndex,
            OMX_COMPONENTTYPE *inputComponent,
            OMX_U32 inputPortIndex);

private:
    void *mLibHandle;

    typedef OMX_ERRORTYPE (*InitFunc)();
    typedef OMX_ERRORTYPE (*DeinitFunc)();
    typedef OMX_ERRORTYPE (*ComponentNameEnumFunc)(
            OMX_STRING, OMX_U32, OMX_U32);

    typedef OMX_ERRORTYPE (*GetHandleFunc)(
            OMX_HANDLETYPE *, OMX_STRING, OMX_PTR, OMX_CALLBACKTYPE *);

    typedef OMX_ERRORTYPE (*FreeHandleFunc)(OMX_HANDLETYPE *);

    typedef OMX_ERRORTYPE (*GetRolesOfComponentFunc)(
            OMX_STRING, OMX_U32 *, OMX_U8 **);

    typedef OMX_ERRORTYPE (*SetupTunnelFunc)(
            OMX_HANDLETYPE, OMX_U32, OMX_HANDLETYPE, OMX_U32);

    InitFunc mInit;
    DeinitFunc mDeinit;
    ComponentNameEnumFunc mComponentNameEnum;
    GetHandleFunc mGetHandle;
    FreeHandleFunc mFreeHandle;
    GetRolesOfComponentFunc mGetRolesOfComponentHandle;
    SetupTunnelFunc mSetupTunnelFunc;

    TCCOMXPlugin(const TCCOMXPlugin &);
    TCCOMXPlugin &operator=(const TCCOMXPlugin &);
};

}  // namespace android

#endif  // _TCC_OMX_PLUGIN_H
