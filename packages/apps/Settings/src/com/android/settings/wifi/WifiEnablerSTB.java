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

package com.android.settings.wifi;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.NetworkInfo;
import android.net.wifi.SupplicantState;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.preference.Preference;
import android.preference.CheckBoxPreference;
import android.provider.Settings;
import android.os.SystemProperties;
import android.widget.Toast;

import com.android.settings.R;
import com.android.settings.WirelessSettings;

import java.util.concurrent.atomic.AtomicBoolean;

import android.util.Log;
import android.os.SystemProperties;

public class WifiEnablerSTB implements Preference.OnPreferenceChangeListener  {
    private final Context mContext;
    private final CheckBoxPreference mCheckBox;
    private final CharSequence mOriginalSummary;
    private AtomicBoolean mConnected = new AtomicBoolean(false);
    private static final String TAG = "WifiEnablerSTB";
    private final WifiManager mWifiManager;
    private boolean mStateMachineEvent;
    private final IntentFilter mIntentFilter;
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (WifiManager.WIFI_STATE_CHANGED_ACTION.equals(action)) {
                handleWifiStateChanged(intent.getIntExtra(
                        WifiManager.EXTRA_WIFI_STATE, WifiManager.WIFI_STATE_UNKNOWN));
            } else if (WifiManager.SUPPLICANT_STATE_CHANGED_ACTION.equals(action)) {
                if (!mConnected.get()) {
                    handleStateChanged(WifiInfo.getDetailedStateOf((SupplicantState)
                            intent.getParcelableExtra(WifiManager.EXTRA_NEW_STATE)));
                }
            } else if (WifiManager.NETWORK_STATE_CHANGED_ACTION.equals(action)) {
                NetworkInfo info = (NetworkInfo) intent.getParcelableExtra(
                        WifiManager.EXTRA_NETWORK_INFO);
                mConnected.set(info.isConnected());
                handleStateChanged(info.getDetailedState());
            }
        }
    };

    public WifiEnablerSTB(Context context, CheckBoxPreference checkBox) {
        mContext = context;
	mCheckBox = checkBox;
	mOriginalSummary = checkBox.getSummary();
        checkBox.setPersistent(false);

        mWifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        mIntentFilter = new IntentFilter(WifiManager.WIFI_STATE_CHANGED_ACTION);
        // The order matters! We really should not depend on this. :(
        mIntentFilter.addAction(WifiManager.SUPPLICANT_STATE_CHANGED_ACTION);
        mIntentFilter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
    }

    public void resume() {
        boolean wifiForcedEnable = SystemProperties.get("tcc.wifi.forced.enable","0").equals("1");
        // Wi-Fi state is sticky, so just let the receiver update UI
        mContext.registerReceiver(mReceiver, mIntentFilter);
	mCheckBox.setOnPreferenceChangeListener(this);
    }

    public void pause() {
        mContext.unregisterReceiver(mReceiver);
	mCheckBox.setOnPreferenceChangeListener(null);
    }
/*

    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        //Do nothing if called as a result of a state machine event
        if (mStateMachineEvent) {
            return;
        }
        // Show toast message if Wi-Fi is not allowed in airplane mode
        if (isChecked && !WirelessSettings.isRadioAllowed(mContext, Settings.System.RADIO_WIFI)) {
            Toast.makeText(mContext, R.string.wifi_in_airplane_mode, Toast.LENGTH_SHORT).show();
            // Reset switch to off. No infinite check/listenenr loop.
            buttonView.setChecked(false);
        }
    }
*/
    public boolean onPreferenceChange(Preference preference, Object value) {
        boolean enable = (Boolean) value;
        // Show toast message if Wi-Fi is not allowed in airplane mode
        if (enable && !WirelessSettings
                .isRadioAllowed(mContext, Settings.System.RADIO_WIFI)) {
            Toast.makeText(mContext, R.string.wifi_in_airplane_mode,
                    Toast.LENGTH_SHORT).show();
            return false;
        }

        // Disable tethering if enabling Wifi
        int wifiApState = mWifiManager.getWifiApState();
        if (enable && ((wifiApState == WifiManager.WIFI_AP_STATE_ENABLING) ||
                (wifiApState == WifiManager.WIFI_AP_STATE_ENABLED))) {
            mWifiManager.setWifiApEnabled(null, false);
        }

        if (mWifiManager.setWifiEnabled(enable)) {
            // Intent has been taken into account, disable until new state is active
            mCheckBox.setEnabled(false);
        } else {
            // Error
            mCheckBox.setSummary(R.string.wifi_error);
            Toast.makeText(mContext, R.string.wifi_error, Toast.LENGTH_SHORT).show();
        }
        return false;
    }

    private void handleWifiStateChanged(int state) {
        switch (state) {
            case WifiManager.WIFI_STATE_ENABLING:
                mCheckBox.setSummary(R.string.wifi_starting);
                mCheckBox.setEnabled(false);
                break;
            case WifiManager.WIFI_STATE_ENABLED:
                mCheckBox.setChecked(true);
                mCheckBox.setSummary(null);
                mCheckBox.setEnabled(true);
                break;
            case WifiManager.WIFI_STATE_DISABLING:
                mCheckBox.setSummary(R.string.wifi_stopping);
                mCheckBox.setEnabled(false);
                break;
            case WifiManager.WIFI_STATE_DISABLED:
                mCheckBox.setChecked(false);
                mCheckBox.setSummary(mOriginalSummary);
                mCheckBox.setEnabled(true);
                //SystemProperties.set("ctl.stop", "tcc_r_service");
                SystemProperties.set("tcc.wifi.display", "0");
                SystemProperties.set("tcc.wifi.display.command", "wifioff");
                break;
            default:
                mCheckBox.setChecked(false);
                mCheckBox.setSummary(R.string.wifi_error);
                mCheckBox.setEnabled(true);
                break;
        }
    }


    private void handleStateChanged(@SuppressWarnings("unused") NetworkInfo.DetailedState state) {
        // After the refactoring from a CheckBoxPreference to a Switch, this method is useless since
        // there is nowhere to display a summary.
        // This code is kept in case a future change re-introduces an associated text.
        /*
        // WifiInfo is valid if and only if Wi-Fi is enabled.
        // Here we use the state of the switch as an optimization.
        if (state != null && mSwitch.isChecked()) {
            WifiInfo info = mWifiManager.getConnectionInfo();
            if (info != null) {
                //setSummary(Summary.get(mContext, info.getSSID(), state));
            }
        }
        */
	if (state != null && mCheckBox.isChecked()) {
            WifiInfo info = mWifiManager.getConnectionInfo();
            if (info != null) {
                mCheckBox.setSummary(Summary.get(mContext, info.getSSID2(), state));
            }
        }
    }
}
