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

import android.net.networkfilesystem.NetworkFileSystemDevInfo;
import android.os.Parcel;
import android.os.Parcelable;
import android.os.Parcelable.Creator;
import android.util.Log;

/**
 * @hide
 */
public class NetworkFileSystemDevInfo implements Parcelable {
    private final String TAG = "NetworkFileSystemDevInfo";
    private static final boolean DEBUG = false;

    private String label;
    private String ipaddr;
    private String path;
    private String mode;
    private String user;
    private String pass;
    private int state;

    public static final String NFS_CONN_MODE_NFS= "nfs";
    public static final String NFS_CONN_MODE_CIFS = "cifs";

    public NetworkFileSystemDevInfo() {
        if (DEBUG) Log.d(TAG, "NetworkFileSystemDevInfo");

        label = null;
        ipaddr = null;
        path = null;
        user = null;
        pass = null;
        mode = NFS_CONN_MODE_NFS;
        state = 0;
    }

    public void setLabel(String label) {
        if (DEBUG) Log.d(TAG, "setLabel label = " + label);
        this.label = label;
    }

    public String getLabel() {
        if (DEBUG) Log.d(TAG, "getIpAddress ip = " + this.label);
        return this.label;
    }

    public void setIpAddress(String ip) {
        if (DEBUG) Log.d(TAG, "setIpAddress ip = " + ip);
        this.ipaddr = ip;
    }

    public String getIpAddress() {
        if (DEBUG) Log.d(TAG, "getIpAddress ip = " + this.ipaddr);
        return this.ipaddr;
    }

    public void setPath(String path) {
        if (DEBUG) Log.d(TAG, "setPath path = " + path);
        this.path = path;
    }

    public String getPath() {
        if (DEBUG) Log.d(TAG, "getPath path = " + this.path);
        return this.path;
    }

    public void setUser(String user) {
        if (DEBUG) Log.d(TAG, "setUser user = " + user);
        this.user = user;
    }

    public String getUser() {
        if (DEBUG) Log.d(TAG, "getUser user = " + this.user);
        return this.user;
    }

    public void setPass(String pass) {
        //if (DEBUG) Log.d(TAG, "setPass pass = " + pass);
        this.pass = pass;
    }

    public String getPass() {
        //if (DEBUG) Log.d(TAG, "getUser pass = " + this.pass);
        return this.pass;
    }

    public boolean setConnectMode(String mode) {
        if (DEBUG) Log.d(TAG, "setConnectMode mode = " + mode);
        if (mode.equals(NFS_CONN_MODE_NFS) || mode.equals(NFS_CONN_MODE_CIFS)) {
            this.mode = mode;
            return true;
        }
        return false;
    }

    public String getConnectMode() {
        if (DEBUG) Log.d(TAG, "getConnectMode mode = " + this.mode);
        return this.mode;
    }

    public void setPersistState(int state) {
        if (DEBUG) Log.d(TAG, "setPersistState() = " + state);
        this.state = state;
    }

    public int getPersistState() {
        if (DEBUG) Log.d(TAG, "getPersistState() = " + this.state);
        return this.state;
    }

    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }

    public void writeToParcel(Parcel dest, int flags) {
        if (DEBUG) Log.d(TAG, "writeToParcel");
        dest.writeString(this.label);
        dest.writeString(this.ipaddr);
        dest.writeString(this.path);
        dest.writeString(this.mode);
        dest.writeString(this.user);
        dest.writeString(this.pass);
        dest.writeInt(this.state);
    }

    /** Implement the Parcelable interface {@hide} */
    public static final Creator<NetworkFileSystemDevInfo> CREATOR =
            new Creator<NetworkFileSystemDevInfo>() {
        public NetworkFileSystemDevInfo createFromParcel(Parcel in) {
            NetworkFileSystemDevInfo info = new NetworkFileSystemDevInfo();
            info.setLabel(in.readString());
            info.setIpAddress(in.readString());
            info.setPath(in.readString());
            info.setConnectMode(in.readString());
            info.setUser(in.readString());
            info.setPass(in.readString());
            info.setPersistState(in.readInt());
            return info;
        }

        public NetworkFileSystemDevInfo[] newArray(int size) {
            return new NetworkFileSystemDevInfo[size];
        }
    };
}
