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

package com.android.settings.telechips.nfs;

import android.content.Context;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.networkfilesystem.NetworkFileSystemManager;
import android.preference.Preference;
import android.preference.CheckBoxPreference;
import android.os.SystemProperties;
import android.os.Environment;
import android.widget.Toast;
import android.util.Log;

import com.android.settings.R;

public class NetworkFileSystemEnabler implements Preference.OnPreferenceChangeListener {
		private static final boolean DEBUG = true;
		private static final String TAG = "NetworkFileSystemEnabler";
		public static final String NFS_NAME = "/storage/nfs";
		public static final String NFS_PATH = "/storage/nfs";

	    private Context mContext;
	    private NetworkFileSystemManager mNFSManager;
	    private CheckBoxPreference mNFSCheckBoxPref;
	    private NetworkFileSystemConfigDialog mNFSConfigDialog;
		private final IntentFilter mIntentFilter;

	    public NetworkFileSystemEnabler(Context context,
			NetworkFileSystemManager networkfilesystemManager,
	            CheckBoxPreference networkfilesystemCheckBoxPreference) {
	        mContext = context;

	        mNFSCheckBoxPref = networkfilesystemCheckBoxPreference;
	        mNFSManager = networkfilesystemManager;
	        networkfilesystemCheckBoxPreference.setPersistent(false);

			boolean wifiState = SystemProperties.get("dhcp.wlan0.result").equals("ok");
			boolean ethState = SystemProperties.get("dhcp.eth0.result").equals("ok");
			if( !wifiState && !ethState)
			{
				Toast toast = Toast.makeText(mContext,"Network not enabled...",Toast.LENGTH_SHORT);
				toast.show();
				mNFSCheckBoxPref.setEnabled(false);
			}
	        if(mNFSManager.getNfsState(NFS_NAME).equals(Environment.MEDIA_MOUNTED)) {
				if(DEBUG) Log.d(TAG, "NetworkFileSystemEnabler nfsEnable : Mounted");

				mNFSCheckBoxPref.setChecked(true);
				mNFSCheckBoxPref.setSummary(R.string.nfs_toggle_summary_off);
	        }
			else
			{
				mNFSCheckBoxPref.setChecked(false);
				mNFSCheckBoxPref.setSummary(R.string.nfs_toggle_summary_on);
			}

			mIntentFilter = new IntentFilter();
			mIntentFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
			mIntentFilter.addAction(Intent.ACTION_MEDIA_REMOVED);
			mIntentFilter.addDataScheme("file");
	    }

	    public void setConfigDialog (NetworkFileSystemConfigDialog dialog) {
			mNFSConfigDialog = dialog;
	    }

		public NetworkFileSystemManager getManager() {
			return mNFSManager;
	    }

	    public void resume() {
			mContext.registerReceiver(mNFSStateReceiver, mIntentFilter);
	        mNFSCheckBoxPref.setOnPreferenceChangeListener(this);
	    }

	    public void pause() {
	        mContext.unregisterReceiver(mNFSStateReceiver);
	        mNFSCheckBoxPref.setOnPreferenceChangeListener(null);
	    }

		public boolean onPreferenceChange(Preference preference, Object newValue) {
			setNfsEnabled((Boolean)newValue);
			return false;
		}

	    public void setNfsEnabled(final boolean enable) {
	        String state = mNFSManager.getNfsState(NFS_NAME);
			if((state.equals(Environment.MEDIA_MOUNTED)) && enable)
				return;
			
			if(DEBUG) Log.i(TAG,"setNfsEnabled enable " + enable + " state " + state);

			boolean wifiState = SystemProperties.get("dhcp.wlan0.result").equals("ok");
			boolean ethState = SystemProperties.get("dhcp.eth0.result").equals("ok");

			if(wifiState || ethState)
			{
				// Disable button
		        mNFSCheckBoxPref.setEnabled(false);

				if (mNFSManager.getNfsConfig(NFS_NAME) == null)
				{
					// Now, kick off the setting dialog to get the configurations
					mNFSConfigDialog.show();
					return;
				}else {
					if(!mNFSManager.setNfsEnabled(NFS_NAME, enable))
					{
					    mNFSCheckBoxPref.setEnabled(true);
						//mNFSManager.setNetworkFileSystemState(mNFSManager.NFS_STATE_ENABLED);
			            mNFSCheckBoxPref.setSummary(enable ? R.string.nfs_error_mount : R.string.nfs_error_unmount);
					}
				}
			}
	    }

		private final BroadcastReceiver mNFSStateReceiver = new BroadcastReceiver() {

	        @Override
	        public void onReceive(Context context, Intent intent) {
                if(DEBUG) Log.d(TAG, "intent.getAction() " + intent);
				if(intent.getData().getPath().equals(NFS_PATH))
					handleNfsStateChanged(intent.getAction());
	        }
	    };

	    private void handleNfsStateChanged(String state) {
	        if (state.equals(Intent.ACTION_MEDIA_MOUNTED)) {
	            mNFSCheckBoxPref.setChecked(true);
	            mNFSCheckBoxPref.setSummary(R.string.nfs_toggle_summary_off);
	            mNFSCheckBoxPref.setEnabled(true);
	        } else if (state.equals(Intent.ACTION_MEDIA_REMOVED)) {
	            mNFSCheckBoxPref.setChecked(false);
	            mNFSCheckBoxPref.setSummary(R.string.nfs_toggle_summary_on);
	            mNFSCheckBoxPref.setEnabled(true);
	        }
	    }
}
