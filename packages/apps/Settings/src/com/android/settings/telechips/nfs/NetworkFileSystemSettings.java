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

import android.os.Bundle;
import android.content.Context;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceScreen;
import android.widget.Toast;
import android.net.networkfilesystem.NetworkFileSystemManager;
import android.util.Log;

import com.android.settings.SettingsPreferenceFragment;
import com.android.settings.R;

public class NetworkFileSystemSettings extends SettingsPreferenceFragment {
    private static final String KEY_TOGGLE_NFS = "toggle_nfs";
    private static final String KEY_CONF_NFS = "nfs_config";
    private NetworkFileSystemEnabler mNfsEnabler;
    private NetworkFileSystemConfigDialog mNfsConfigDialog;
    private Preference mNfsConfigPref;
    private static final boolean DEBUG = true;
    private static final String TAG = "NetworkFileSystemSettings";

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        super.onPreferenceTreeClick(preferenceScreen, preference);
        if (preference == mNfsConfigPref) {
            mNfsConfigDialog.show();
        }
        return false;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        addPreferencesFromResource(R.xml.nfs_settings);
        final PreferenceScreen preferenceScreen = getPreferenceScreen();
        mNfsConfigPref = preferenceScreen.findPreference(KEY_CONF_NFS);
        /*
         * TO DO:
         * Add new perference screen for Network File System Configuration
	     */
		if (DEBUG) Log.d(TAG, "onCreate");

        initToggles();
    }
    @Override
    public void onResume() {
        super.onResume();
        mNfsEnabler.resume();
    }

    @Override
    public void onPause() {
        super.onPause();
        mNfsEnabler.pause();
    }
    private void initToggles() {

        mNfsEnabler = new NetworkFileSystemEnabler(
                getActivity(),
                (NetworkFileSystemManager) getSystemService(Context.NFS_SERVICE),
                (CheckBoxPreference) findPreference(KEY_TOGGLE_NFS));
        mNfsConfigDialog = new NetworkFileSystemConfigDialog(getActivity(), mNfsEnabler);
        mNfsEnabler.setConfigDialog(mNfsConfigDialog);
    }
}
