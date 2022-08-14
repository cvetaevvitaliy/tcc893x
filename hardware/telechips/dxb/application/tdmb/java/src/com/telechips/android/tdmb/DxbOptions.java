/*
 * Copyright (C) 2013 Telechips, Inc.
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

package com.telechips.android.tdmb;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.KeyEvent;
import android.view.WindowManager;

import com.telechips.android.tdmb.util.DxbStorage;

public class DxbOptions extends PreferenceActivity implements Preference.OnPreferenceChangeListener
{	
	private static final String TAG = "[[[DxbOptions]]] ";

	static Information.OPTION	gInfo_Option;

	/*	select key	*/
		public static final String KEY_STORAGE_SELECT			= "storage_select";

	private ListPreference mStorageSelect = null;

	private boolean isbackpressed;
	
	@Override
	public void onCreate(Bundle saveInstanceState)
	{
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onCreate()");
		super.onCreate(saveInstanceState);
        
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        
		addPreferencesFromResource(R.xml.options_tdmb);
        
		setDefault_Value();

		getListView().setBackgroundColor(Color.rgb(16, 16, 16)); 		
		isbackpressed = false;
		gInfo_Option	= Manager_Setting.g_Information.cOption;
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		initIntentFilter();
	}

	private void initIntentFilter() {
		// Storage
		{
			IntentFilter	commandFilter	= new IntentFilter();
			commandFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
			commandFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
			registerReceiver(mBroadcastReceiver, commandFilter);
		}
	}

	@Override
	public void onStart()
	{
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onStart()");
		super.onStart();
	}

	@Override
	protected void onResume()
	{
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onResume()");
		super.onResume();
		
		setStorageValue();
		updateState(true);
	}	

	@Override
	protected void onPause()
	{
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onPause()");
		super.onPause();
		if(DxbView.intentSubActivity != null && isbackpressed == false)
		{
			if(DxbPlayer.mPlayer != null)
			{
				DxbPlayer.releaseSurface();
				DxbPlayer.mPlayer.stop();
				DxbPlayer.mPlayer.release();
				
				DxbPlayer.mPlayer	= null;
			}
		}
	}

	@Override
	public void onStop()
	{
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onStop()");
		super.onStop();
		this.finish();
	}

    @Override
	public void onDestroy()
	{
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onDestroy()");

		if(DxbView.intentSubActivity != null && isbackpressed == false)
		{
			if(DxbPlayer.mChannelManager != null)
			{
				DxbPlayer.mChannelManager.close();
				DxbPlayer.mChannelManager = null;
			}
			
			if(DxbView.mDB != null)
			{
				DxbView.mDB.close();
				DxbView.mDB = null;
			}
		}
		
		super.onDestroy();
		unregisterReceiver(mBroadcastReceiver);
	}

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        int keyCode = event.getKeyCode();
        switch (keyCode)
		{
			case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
			case KeyEvent.KEYCODE_MEDIA_PLAY:
			case KeyEvent.KEYCODE_MEDIA_PAUSE:
				return true;

			default:
				break;
        }

        return super.dispatchKeyEvent(event);
    }

	@Override 
	public void onBackPressed(){	
		Log.d("BACK KEY", "onBackPressed Called");
		isbackpressed = true;
		super.onBackPressed();  
	}

	static void setResumeValue()
	{
		Log.i(TAG, "setResumeValue()");
		
		gInfo_Option	= Manager_Setting.g_Information.cOption;

		SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(Manager_Setting.mContext);
		String list_value;
		
		//	Storage
			list_value = pref.getString(KEY_STORAGE_SELECT, "");
			if(list_value != "")
			{
				gInfo_Option.storage	= list_value;
			}
	}
	
	private void updateState(boolean force)
	{
		Log.i(TAG, "updateState()");
		
		updateStorageSelectSummary();
	}
	
	private void updateStorageSelectSummary() {
		Log.i(TAG, "updateStorageSelectSummary()");
		String sData = DxbStorage.getPath_Device();
		if(sData != null) {
			mStorageSelect.setSummary(sData);
		} else {
			mStorageSelect.setSummary("");
		}
	}

	public boolean onPreferenceChange(Preference preference, Object objValue)
	{
		final String key = preference.getKey();
		Log.i(TAG, "onPreferenceChange() - " + key);
		
		if(KEY_STORAGE_SELECT.equals(key))
		{
			try
			{
				DxbStorage.setStorage((String)objValue);
				gInfo_Option.storage	= (String)objValue;
				updateStorageSelectSummary();
			}
			catch(NumberFormatException e)
			{
				//Log.e(TAG, "could not persist screen output select", e);
			}
		}
	
		return true;
	}
	
	private void setDefault_Value()
	{
		Log.i(TAG, "setDefault_Value()");
		
		mStorageSelect = (ListPreference) findPreference(KEY_STORAGE_SELECT);
		if (mStorageSelect != null) {
			setStorageValue();
			mStorageSelect.setOnPreferenceChangeListener(this);
		}
	}
	
	private void setStorageValue()
	{
		Log.i(TAG, "setStorageValue()");

		ArrayList<CharSequence> entries = new ArrayList<CharSequence>();
		ArrayList<CharSequence> entry_values = new ArrayList<CharSequence>();
		File[] storageVolumes = DxbStorage.getVolumeList();

		int length = (storageVolumes == null) ? 0 : storageVolumes.length;
		for (int i = 0; i < length; i++) {
			File storageVolume = storageVolumes[i];
			if (storageVolume.isDirectory()) {
				if (storageVolume.canWrite()) {
					entries.add(storageVolume.getName());
				} else if (!storageVolume.canExecute()) {
					entries.add(storageVolume.getName() + " (Not mounted)");
				} else {
					continue;
				}
				entry_values.add(storageVolume.getPath());
			}
		}
		mStorageSelect.setEntries(entries.toArray(new CharSequence[entries.size()]));
		mStorageSelect.setEntryValues(entry_values.toArray(new CharSequence[entry_values.size()]));

		CharSequence[] values = mStorageSelect.getEntryValues();
		for (int i = 0; i < values.length; i++) {
			String value = values[i].toString();
			if (DxbStorage.getPath_Device().equals(value)) {
				mStorageSelect.setValue(value);
				return;
			}
		}
		if(values.length < 1) {
			DxbStorage.setStorage(DxbStorage.DEFAULT);
			return;
		}
		
		DxbStorage.setStorage(DxbStorage.DEFAULT);
		mStorageSelect.setValue(values[0].toString());
		updateStorageSelectSummary();
	}

	public static boolean isMountpointMounted(String path)
	{
		//Log.i(TAG, "isMountpointMounted: " + path);	
		String read;
		try {
			BufferedReader bp = new BufferedReader(new FileReader("/proc/mounts"));
			while((read = bp.readLine())!=null) {
				if (read.contains(path)) {
					bp.close();
					return true;
				}
			}
			bp.close();
		} catch(IOException ex) {
			return false;
		}
		return false;
	}

	private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver()
	{ 
		@Override
		public void onReceive(final Context context, Intent intent)
		{
			final String action = intent.getAction();
			if(Intent.ACTION_MEDIA_MOUNTED.equals(action)
					||	Intent.ACTION_MEDIA_UNMOUNTED.equals(action)
			)
			{
				setStorageValue();
				updateState(true);
			}
		}
	};
}
