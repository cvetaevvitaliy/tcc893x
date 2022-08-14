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

package com.android.settings.bluetooth;

import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.provider.Settings;
import android.widget.CompoundButton;
import android.widget.Toast;
import android.preference.Preference;
import android.preference.CheckBoxPreference;
import android.os.SystemProperties;

import com.android.settings.R;
import com.android.settings.WirelessSettings;

/**
 * BluetoothEnabler is a helper to manage the Bluetooth on/off checkbox
 * preference. It turns on/off Bluetooth and ensures the summary of the
 * preference reflects the current state.
 */
public final class BluetoothEnablerSTB implements Preference.OnPreferenceChangeListener {
    private final Context mContext;
    private final CheckBoxPreference mCheckBox;
    private final CharSequence mOriginalSummary;

    private final LocalBluetoothAdapter mLocalAdapter;
    private final IntentFilter mIntentFilter;

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);
            handleStateChanged(state);
        }
    };

    public BluetoothEnablerSTB(Context context, CheckBoxPreference checkBox) {
        mContext = context;
	mCheckBox = checkBox;
	mOriginalSummary = checkBox.getSummary();

        LocalBluetoothManager manager = LocalBluetoothManager.getInstance(context);
        if (manager == null) {
            // Bluetooth is not supported
            mLocalAdapter = null;
            mCheckBox.setEnabled(false);
        } else {
            mLocalAdapter = manager.getBluetoothAdapter();
        }
        mIntentFilter = new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED);
    }

    public void resume() {
        if (mLocalAdapter == null) {
            mCheckBox.setEnabled(false);
            return;
        }

        // Bluetooth state is not sticky, so set it manually
        handleStateChanged(mLocalAdapter.getBluetoothState());

        mContext.registerReceiver(mReceiver, mIntentFilter);
	mCheckBox.setOnPreferenceChangeListener(this);
    }

    public void pause() {
        if (mLocalAdapter == null) {
            return;
        }

        mContext.unregisterReceiver(mReceiver);
    	mCheckBox.setOnPreferenceChangeListener(null);
    }

/*
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        // Show toast message if Bluetooth is not allowed in airplane mode
        if (isChecked &&
                !WirelessSettings.isRadioAllowed(mContext, Settings.System.RADIO_BLUETOOTH)) {
            Toast.makeText(mContext, R.string.wifi_in_airplane_mode, Toast.LENGTH_SHORT).show();
            // Reset switch to off
            buttonView.setChecked(false);
        }

        if (mLocalAdapter != null) {
            mLocalAdapter.setBluetoothEnabled(isChecked);
                mCheckBox.setEnabled(false);
        }
            mCheckBox.setEnabled(false);
    }
*/
    public boolean onPreferenceChange(Preference preference, Object value) {
        boolean enable = (Boolean) value;

        // Show toast message if Bluetooth is not allowed in airplane mode
        if (enable && !WirelessSettings
                .isRadioAllowed(mContext, Settings.System.RADIO_BLUETOOTH)) {
            Toast.makeText(mContext, R.string.wifi_in_airplane_mode,
                    Toast.LENGTH_SHORT).show();
            return false;
        }

        if((SystemProperties.get("ro.memtype")).compareTo("ddr3") == 0){
            if((SystemProperties.get("persist.sys.output_mode")).compareTo("1") == 0){
                Toast.makeText(mContext , "Can not use Bluetooth when HDMI is on!", Toast.LENGTH_SHORT).show();
            }else{
                mLocalAdapter.setBluetoothEnabled(enable);
                mCheckBox.setEnabled(false);
            }
        }else{
            mLocalAdapter.setBluetoothEnabled(enable);
            mCheckBox.setEnabled(false);
        }


        // Don't update UI to opposite state until we're sure
        return false;
    }

    void handleStateChanged(int state) {
        switch (state) {
            case BluetoothAdapter.STATE_TURNING_ON:
                mCheckBox.setSummary(R.string.bluetooth_turning_on);
		mCheckBox.setEnabled(false);
                break;
            case BluetoothAdapter.STATE_ON:
                mCheckBox.setChecked(true);
		mCheckBox.setSummary(null);
                mCheckBox.setEnabled(true);
                break;
            case BluetoothAdapter.STATE_TURNING_OFF:
		mCheckBox.setSummary(R.string.bluetooth_turning_off);
                mCheckBox.setEnabled(false);
                break;
            case BluetoothAdapter.STATE_OFF:
                mCheckBox.setChecked(false);
		mCheckBox.setSummary(mOriginalSummary);
                mCheckBox.setEnabled(true);
                break;
            default:
                mCheckBox.setChecked(false);
		mCheckBox.setSummary(R.string.wifi_error);
                mCheckBox.setEnabled(true);
        }
    }
}
