/*
 * Copyright (C) 2011 Telechips Inc.
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

#ifndef _NETWORKVOLUME_H
#define _NETWORKVOLUME_H

#include <utils/List.h>

#include "Volume.h"

class NetworkVolume : public Volume {
protected:
    const char* mMountpoint;
    const char* mFuseMountpoint;

public:
    NetworkVolume(VolumeManager *vm, const fstab_rec* rec, int flags);
    virtual ~NetworkVolume();

    const char *getMountpoint() { return mMountpoint; }
    const char *getFuseMountpoint() { return mFuseMountpoint; }

    int mountVol();
    int unmountVol(bool force, bool revert);
    int getVolInfo(struct volume_info *v) { return 0; }

protected:
    int getDeviceNodes(dev_t *devs, int max) { return 0; }
    int updateDeviceInfo(char *new_path, int new_major, int new_minor) { return 0; }
    virtual void revertDeviceInfo(void) {}
    int isDecrypted() { return 0; }
    int getFlags() { return 0; }

private:
    int doUnmount(const char *path, bool force);
};

typedef android::List<NetworkVolume *> NetworkVolumeCollection;

#endif
