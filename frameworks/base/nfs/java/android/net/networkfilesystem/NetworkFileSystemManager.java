/*
 * Copyright (C) 2010 Telechips, Inc.
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

package android.net.networkfilesystem;

import java.util.List;

import android.annotation.SdkConstant;
import android.annotation.SdkConstant.SdkConstantType;
import android.net.wifi.IWifiManager;
import android.os.Handler;
import android.os.RemoteException;
import android.util.Log;

/**
 * @hide
 */
public class NetworkFileSystemManager {
    public static final String TAG = "NetworkFileSystemManager";
    public static final int NFS_DEVICE_SCAN_RESULT_READY = 0;
    public static final String NFS_STATE_CHANGED_ACTION =
            "android.net.networkfilesystem.NFS_STATE_CHANGED";

    public static final String EXTRA_NFS_STATE = "nfs_state";

    public static final int EVENT_NFS_UNKNOWN = 0;
    public static final int EVENT_NFS_MOUNTED = 1;
    public static final int EVENT_NFS_UNMOUNTED	= 2;

    INetworkFileSystemManager mService;

    public NetworkFileSystemManager(INetworkFileSystemManager service, Handler handler) {
        Log.i(TAG, "Init NetworkFileSystem Manager");
        mService = service;
    }

    public void setNfsConfig(NetworkFileSystemDevInfo info) {
        try {
            mService.setNfsConfig(info);
        } catch (RemoteException e) {
            Log.i(TAG, "Can not update nfs device info");
        }
    }

    public NetworkFileSystemDevInfo getNfsConfig(String label) {
        try {
            return mService.getNfsConfig(label);
        } catch (RemoteException e) {
            Log.i(TAG, "Can not get nfs config");
        }
        return null;
    }

    public boolean setNfsEnabled(String label, boolean enable) {
        try {
            return mService.setNfsEnabled(label, enable);
        } catch (RemoteException e) {
            Log.i(TAG,"setNfsEnabled err");
        }
        return false;
    }

    public String getNfsState(String label) {
        try {
            return mService.getNfsState(label);
        } catch (RemoteException e) {
            return null;
        }
    }
}
