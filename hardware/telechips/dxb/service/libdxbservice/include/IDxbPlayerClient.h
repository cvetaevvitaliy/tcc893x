/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions 
andlimitations under the License.

****************************************************************************/

#ifndef I_DXB_PLAYERC_LIENT_H
#define I_DXB_PLAYERC_LIENT_H

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

namespace android {

class IDxbPlayerClient: public IInterface
{
public:
    DECLARE_META_INTERFACE(DxbPlayerClient);

    virtual void notify(int msg, int ext1, int ext2, const Parcel *obj) = 0;
};

// ----------------------------------------------------------------------------

class BnDxbPlayerClient: public BnInterface<IDxbPlayerClient>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

}; // namespace android

#endif // I_DXB_PLAYERC_LIENT_H
