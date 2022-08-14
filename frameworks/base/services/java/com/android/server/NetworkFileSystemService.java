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

package com.android.server;

import java.net.UnknownHostException;
import android.net.networkfilesystem.INetworkFileSystemManager;
import android.net.networkfilesystem.NetworkFileSystemManager;
import android.net.networkfilesystem.NetworkFileSystemDevInfo;
import android.util.Log;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ContentValues;
import android.net.wifi.WifiManager;
import android.net.ethernet.EthernetManager;
import android.os.SystemProperties;
import android.os.RemoteException;
import android.os.IBinder;
import android.os.storage.IMountService;
import android.os.ServiceManager;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.os.storage.StorageResultCode;
import android.os.Environment;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;


/**
 *
 * @hide
 */
public class NetworkFileSystemService<syncronized> extends INetworkFileSystemManager.Stub {
    private static final String TAG = "NetworkFileSystemService";
    private static final boolean DEBUG =  false;

    private Context mContext;

    public static final int NFS_STATE_DISABLED = 1;
    public static final int NFS_STATE_ENABLED = 2;

    // Access using getMountService()
    private IMountService mMountService = null;
    private StorageManager mStorageManager;

    private DatabaseHelper mDBHelper = null;
    
    private boolean mQBEnale = false;
    private boolean mQBForceDisable = false;
    private String mQBBootwith = "not_yet";
    
    private static final String DATABASE_NAME             = "nfs_settings.db";
    private static final int DATABASE_VERSION             = 1;
    private static final String DB_NFS_INFO               = "nfs_info";
    private static final String KEY_LOCAL_PATH            = "local_path";
    private static final String KEY_CONNECT_MODE          = "connect_mode";
    private static final String KEY_REMOTE_IP             = "remote_ip";
    private static final String KEY_REMOTE_PATH           = "remote_path";
    private static final String KEY_USER_ID               = "user_id";
    private static final String KEY_USER_PW               = "user_pw";
    private static final String KEY_PERSIST_STATE         = "persist_state";
    private static final String DATABASE_FEEDTABLE_CREATE = "create table "
                                                          + DB_NFS_INFO
                                                          +	" ("
                                                          +	KEY_LOCAL_PATH      + " text, "
                                                          +	KEY_CONNECT_MODE    + " text, "
                                                          +	KEY_REMOTE_IP       + " text, "
                                                          +	KEY_REMOTE_PATH     + " text, "
                                                          +	KEY_USER_ID         + " text, "
                                                          +	KEY_USER_PW         + " text, "
                                                          +	KEY_PERSIST_STATE   + " integer)";

	/**
     * Builds the database.  This version has extra support for using the version field
     * as a mode flags field, and configures the database columns depending on the mode bits
     * (features) requested by the extending class.
     * 
     * @hide
     */
    private class DatabaseHelper extends SQLiteOpenHelper {
        public DatabaseHelper(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);
        }
		@Override
        public void onCreate(SQLiteDatabase db) {
            db.execSQL(DATABASE_FEEDTABLE_CREATE);
        }
		@Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            db.execSQL("drop table " + DB_NFS_INFO);
            onCreate(db);
        }
    }

    public NetworkFileSystemService(Context context) {
        mContext = context;
		/*
        IntentFilter nfs_filter = new IntentFilter();
        nfs_filter.addAction(Intent.ACTION_MEDIA_MOUNTED);
        nfs_filter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        nfs_filter.addDataScheme("file");
        mContext.registerReceiver(mNfsReceiver, nfs_filter);

        IntentFilter net_filter = new IntentFilter();
        net_filter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
        net_filter.addAction(EthernetManager.ETH_STATE_CHANGED_ACTION);
        mContext.registerReceiver(mNetworkReceiver, net_filter);
		*/
        
        mQBEnale = SystemProperties.getBoolean("ro.QB.enable", false);
        
        if(!mQBEnale)
        	mDBHelper = new DatabaseHelper(context);

        mStorageManager = (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);
    }

    public synchronized void setNfsConfig(NetworkFileSystemDevInfo info) {
    	if( mDBHelper == null ) {
    		mDBHelper = new DatabaseHelper(mContext);
    	}
    	
        SQLiteDatabase db = mDBHelper.getWritableDatabase();
        ContentValues cv = new ContentValues();

        cv.put(KEY_LOCAL_PATH, info.getLabel());
        cv.put(KEY_CONNECT_MODE, info.getConnectMode());
        cv.put(KEY_REMOTE_IP, info.getIpAddress());
        cv.put(KEY_REMOTE_PATH, info.getPath());
        cv.put(KEY_USER_ID, info.getUser());
        cv.put(KEY_USER_PW, info.getPass());
        cv.put(KEY_PERSIST_STATE, info.getPersistState());

        if (getNfsConfig(info.getLabel())==null) {
            db.insert(DB_NFS_INFO, null, cv);
        } else {
            db.update(DB_NFS_INFO, cv, KEY_LOCAL_PATH+"='"+info.getLabel()+"'", null);
        }
        
        if( !mQBForceDisable && mQBEnale && mQBBootwith.equals("not_yet") ) {
        	mQBBootwith = SystemProperties.get("tcc.QB.boot.with", "not_yet");
        	
        	if( mQBBootwith.equals("not_yet") ) {
	        	mDBHelper.close();
	        	mDBHelper = null;
        	}
        }
    }

    public synchronized NetworkFileSystemDevInfo getNfsConfig(String path) {
        NetworkFileSystemDevInfo info = new NetworkFileSystemDevInfo();

        if( mDBHelper == null ) {
    		mDBHelper = new DatabaseHelper(mContext);
    	}
        
        SQLiteDatabase db = mDBHelper.getReadableDatabase();
        String sql = "SELECT * FROM " + DB_NFS_INFO + " WHERE " + KEY_LOCAL_PATH + "='" + path + "';";
        Cursor result = db.rawQuery(sql, null);
        if ((result != null) && (result.getCount() > 0)) {
            result.moveToFirst();
            info.setLabel(result.getString(0));
            info.setConnectMode(result.getString(1));
            info.setIpAddress(result.getString(2));
            info.setPath(result.getString(3));
            info.setUser(result.getString(4));
            info.setPass(result.getString(5));
            info.setPersistState(result.getInt(6));
            result.close();
            return info;
        }
        if (result != null) {
            result.close();
        }
        
        if( mQBEnale && mQBBootwith.equals("not_yet") ) {
        	mQBBootwith = SystemProperties.get("tcc.QB.boot.with", "not_yet");
        	
        	if( mQBBootwith.equals("not_yet") ) {
	        	mDBHelper.close();
	        	mDBHelper = null;
        	}
        }

        return null;
    }

    public synchronized boolean setNfsEnabled(String path, boolean state) {
        boolean wifiState = SystemProperties.get("dhcp.wlan0.result").equals("ok");
        boolean ethState = SystemProperties.get("dhcp.eth0.result").equals("ok");

        if (wifiState || ethState) {
            if (state) {
                return doMountNfs(path);
            } else if (!state && getNfsState(path).equals(Environment.MEDIA_MOUNTED)) {
                return doUnmountNfs(path);
            }
        } else {
            if (DEBUG) Log.i(TAG,"network not connected !!!!!!!!!!");
        }
        return false;
    }

    public String getNfsState(String path) {
        if (path == null) {
            StorageVolume volumes[] = mStorageManager.getVolumeList();
            int i;
            for (i = 0; i < volumes.length; i++) {
                if (getNfsConfig(volumes[i].getPath()) == null) {
                    continue;
                }
                String state = mStorageManager.getVolumeState(volumes[i].getPath());
                if (state != null && state.equals(Environment.MEDIA_MOUNTED)) {
                    return Environment.MEDIA_MOUNTED;
                }
            }
            return Environment.MEDIA_UNMOUNTED;
        } else {
            return mStorageManager.getVolumeState(path);
        }
    }

    private boolean doMountNfs(String path) {
        NetworkFileSystemDevInfo info = getNfsConfig(path);
        if (info == null) {
            return false;
        }

        IMountService mountService = getMountService();
        try {
            mountService.mountVolume(path);
            //if ((mountService.mountVolume(path) == StorageResultCode.OperationFailedInternalError) || (mountService.mountVolume(path) == StorageResultCode.OperationFailedNoMedia)) {
            //    return false;
            //}
        } catch (RemoteException e) {
            return false;
        }
        return true;
    }

    private void UnmountAllNfs() {
        IMountService mountService = getMountService();
        StorageVolume volumes[] = mStorageManager.getVolumeList();

        int i;
        for (i = 0; i < volumes.length; i++) {
            doUnmountNfs(volumes[i].getPath());
        }
    }

    private boolean doUnmountNfs(String path) {
        NetworkFileSystemDevInfo info = getNfsConfig(path);
        if (info == null) {
            return false;
        }

        IMountService mountService = getMountService();
        try {
            //if (mountService.getVolumeState(path).equals(Environment.MEDIA_MOUNTED)) {
                mountService.unmountVolume(path, true, false);
            //}
        } catch (RemoteException e) {
            return false;
        }
        return true;
    }

    private synchronized IMountService getMountService() {
        if (mMountService == null) {
            IBinder service = ServiceManager.getService("mount");
            if (service != null) {
                mMountService = IMountService.Stub.asInterface(service);
            } else {
                Log.e(TAG, "Can't get mount service");
            }
        }
        return mMountService;
    }
	/*
    private final BroadcastReceiver mNfsReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (DEBUG) Log.d(TAG, "Intent "+ intent);

            String action = intent.getAction();
            String path = intent.getData().getPath();

            if (getNfsConfig(path) == null) {
                return;
            }
            if (action.equals(Intent.ACTION_MEDIA_MOUNTED)) {
                final Intent nfs_intent = new Intent(NetworkFileSystemManager.NFS_STATE_CHANGED_ACTION);
                nfs_intent.putExtra(NetworkFileSystemManager.EXTRA_NFS_STATE, NetworkFileSystemManager.EVENT_NFS_MOUNTED);
                mContext.sendStickyBroadcast(nfs_intent);
            } else if (action.equals(Intent.ACTION_MEDIA_UNMOUNTED) && !getNfsState(null).equals(Environment.MEDIA_MOUNTED)) {
                final Intent nfs_intent = new Intent(NetworkFileSystemManager.NFS_STATE_CHANGED_ACTION);
                nfs_intent.putExtra(NetworkFileSystemManager.EXTRA_NFS_STATE, NetworkFileSystemManager.EVENT_NFS_UNMOUNTED);
                mContext.sendStickyBroadcast(nfs_intent);
            }
        }
    };

    private final BroadcastReceiver mNetworkReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (DEBUG) Log.d(TAG, "Intent "+ intent);

            String action = intent.getAction();
            if (!action.equals(WifiManager.WIFI_STATE_CHANGED_ACTION) && !action.equals(EthernetManager.ETH_STATE_CHANGED_ACTION)) {
                return;
            }

            boolean wifiState = SystemProperties.get("dhcp.wlan0.result").equals("ok");
            boolean ethState = SystemProperties.get("dhcp.eth0.result").equals("ok");
            if (DEBUG) Log.d(TAG, " wifiState " + wifiState + " ethState " +ethState);
            if (!wifiState && !ethState) {
               	UnmountAllNfs();
            }
        }
    };
    */
    
}
