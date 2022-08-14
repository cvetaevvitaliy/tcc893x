/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.android.settings.deviceinfo;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.os.UserManager;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.os.SystemProperties;

import com.android.settings.R;
import com.android.settings.SettingsPreferenceFragment;
import com.android.settings.Utils;

/**
 * USB storage settings.
 */
public class UsbSettings extends SettingsPreferenceFragment {

    private static final String TAG = "UsbSettings";

    private static final String KEY_MTP = "usb_mtp";
    private static final String KEY_PTP = "usb_ptp";
   private static final String KEY_HOST = "usb_host";
   private static final String KEY_DEVICE = "usb_device";	

    private UsbManager mUsbManager;
    private CheckBoxPreference mMtp;
    private CheckBoxPreference mPtp;
   private CheckBoxPreference mHost;	
   private CheckBoxPreference mDevice;	
    private boolean mUsbAccessoryMode;

    private final BroadcastReceiver mStateReceiver = new BroadcastReceiver() {
        public void onReceive(Context content, Intent intent) {
            String action = intent.getAction();
         String product = SystemProperties.get("ro.build.product", "");					
         String chip_product = SystemProperties.get("ro.chip.product", "");					
         String model_product = SystemProperties.get("ro.product.model", "");
            if (action.equals(UsbManager.ACTION_USB_STATE)) {
               mUsbAccessoryMode = intent.getBooleanExtra(UsbManager.USB_FUNCTION_ACCESSORY, false);
               Log.e(TAG, "UsbAccessoryMode " + mUsbAccessoryMode);
            }
         if ((model_product.equals("TCC8930_STB_EV") || model_product.equals("TCC8930_YJ8930T") || model_product.equals("TCC8935_YJ8935T") || product.equals("tcc893x") || product.equals("m805_893x")) && !chip_product.equals("tcc8935s")) {					
            String usb_mode = SystemProperties.get("sys.usb.mode", "");
            if (usb_mode.equals("usb_device"))
               updateToggles(UsbManager.USB_FUNCTION_DEVICE);				
            else if (usb_mode.equals("usb_host"))
               updateToggles(UsbManager.USB_FUNCTION_HOST);				
         } else {
            updateToggles(mUsbManager.getDefaultFunction());	
         }
      }
    };

    private PreferenceScreen createPreferenceHierarchy() {
      String product = SystemProperties.get("ro.build.product", "");			
      String chip_product = SystemProperties.get("ro.chip.product", "");
      String model_product = SystemProperties.get("ro.product.model", "");
      boolean device_connected = "1".equals(SystemProperties.get("sys.usb.connected"));
        PreferenceScreen root = getPreferenceScreen();
        if (root != null) {
            root.removeAll();
        }
      if ((!device_connected && (model_product.equals("TCC8930_STB_EV") || model_product.equals("TCC8930_YJ8930T") || model_product.equals("TCC8935_YJ8935T")) || product.equals("tcc893x") || product.equals("m805_893x")) && !chip_product.equals("tcc8935s")) {					
         addPreferencesFromResource(R.xml.usb_mode_settings);		
      } else {
        addPreferencesFromResource(R.xml.usb_settings);
      }
        root = getPreferenceScreen();

      if ((!device_connected && (model_product.equals("TCC8930_STB_EV") || model_product.equals("TCC8930_YJ8930T") || model_product.equals("TCC8935_YJ8935T")) || product.equals("tcc893x") || product.equals("m805_893x")) && !chip_product.equals("tcc8935s")) {					
         mHost = (CheckBoxPreference)root.findPreference(KEY_HOST);
         mDevice = (CheckBoxPreference)root.findPreference(KEY_DEVICE);	  
      } else {
        mMtp = (CheckBoxPreference)root.findPreference(KEY_MTP);
        mPtp = (CheckBoxPreference)root.findPreference(KEY_PTP);
      }

        UserManager um = (UserManager) getActivity().getSystemService(Context.USER_SERVICE);
        if (um.hasUserRestriction(UserManager.DISALLOW_USB_FILE_TRANSFER)) {
            mMtp.setEnabled(false);
            mPtp.setEnabled(false);
        }

        return root;
    }

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mUsbManager = (UsbManager)getSystemService(Context.USB_SERVICE);
    }

    @Override
    public void onPause() {
        super.onPause();
        getActivity().unregisterReceiver(mStateReceiver);
    }

    @Override
    public void onResume() {
        super.onResume();

        // Make sure we reload the preference hierarchy since some of these settings
        // depend on others...
        createPreferenceHierarchy();

        // ACTION_USB_STATE is sticky so this will call updateToggles
        getActivity().registerReceiver(mStateReceiver,
                new IntentFilter(UsbManager.ACTION_USB_STATE));
    }

    private void updateToggles(String function) {
      String product = SystemProperties.get("ro.build.product", "");	
      String chip_product = SystemProperties.get("ro.chip.product", "");					
      String usb_drd_mode = SystemProperties.get("sys.drd.mode", "");
      String model_product = SystemProperties.get("ro.product.model", "");
      boolean device_connected = "1".equals(SystemProperties.get("sys.usb.connected"));

      if ((!device_connected && (model_product.equals("TCC8930_STB_EV") || model_product.equals("TCC8930_YJ8930T") || model_product.equals("TCC8935_YJ8935T")) || product.equals("tcc893x") || product.equals("m805_893x")) && !chip_product.equals("tcc8935s")) {					

         if (UsbManager.USB_FUNCTION_HOST.equals(function)) {
            mHost.setChecked(true);	
            mDevice.setChecked(false);	
         } else if (UsbManager.USB_FUNCTION_DEVICE.equals(function)) {
            mHost.setChecked(false);	
            mDevice.setChecked(true);	
         } else  {
            mHost.setChecked(false);	
            mDevice.setChecked(false);			
         }

         if (!mUsbAccessoryMode) {
            //Enable MTP and PTP switch while USB is not in Accessory Mode, otherwise disable it
            Log.e(TAG, "USB Normal Mode");
            mHost.setEnabled(true);	
            mDevice.setEnabled(true);			
         } else {
            Log.e(TAG, "USB Accessory Mode");
            mHost.setEnabled(false);	
            mDevice.setEnabled(false);			
         }

      } else {

        if (UsbManager.USB_FUNCTION_MTP.equals(function)) {
            mMtp.setChecked(true);
            mPtp.setChecked(false);
        } else if (UsbManager.USB_FUNCTION_PTP.equals(function)) {
            mMtp.setChecked(false);
            mPtp.setChecked(true);
         } else if (UsbManager.USB_FUNCTION_HOST.equals(function)) {
            mMtp.setChecked(false);
            mPtp.setChecked(false);  
         } else if (UsbManager.USB_FUNCTION_DEVICE.equals(function)) {
            mMtp.setChecked(false);
            mPtp.setChecked(false);
         } else  {
            mMtp.setChecked(false);
            mPtp.setChecked(false);	  
         }

        UserManager um = (UserManager) getActivity().getSystemService(Context.USER_SERVICE);
        if (um.hasUserRestriction(UserManager.DISALLOW_USB_FILE_TRANSFER)) {
            Log.e(TAG, "USB is locked down");
            mMtp.setEnabled(false);
            mPtp.setEnabled(false);
        } else if (!mUsbAccessoryMode) {
            //Enable MTP and PTP switch while USB is not in Accessory Mode, otherwise disable it
            Log.e(TAG, "USB Normal Mode");
            mMtp.setEnabled(true);
            mPtp.setEnabled(true);	  
         } else {
            Log.e(TAG, "USB Accessory Mode");
            mMtp.setEnabled(false);
            mPtp.setEnabled(false);  
         }
      }
   }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
         String product = SystemProperties.get("ro.build.product", "");	
         String chip_product = SystemProperties.get("ro.chip.product", "");					
         String usb_drd_mode = SystemProperties.get("sys.drd.mode", "");
         String model_product = SystemProperties.get("ro.product.model", "");
         boolean device_connected = "1".equals(SystemProperties.get("sys.usb.connected"));

        // Don't allow any changes to take effect as the USB host will be disconnected, killing
        // the monkeys
        if (Utils.isMonkeyRunning()) {
            return true;
        }
        // If this user is disallowed from using USB, don't handle their attempts to change the
        // setting.
        UserManager um = (UserManager) getActivity().getSystemService(Context.USER_SERVICE);
        if (um.hasUserRestriction(UserManager.DISALLOW_USB_FILE_TRANSFER)) {
            return true;
        }

         if ((!device_connected && (model_product.equals("TCC8930_STB_EV") || model_product.equals("TCC8930_YJ8930T") || model_product.equals("TCC8935_YJ8935T")) || product.equals("tcc893x") || product.equals("m805_893x")) && !chip_product.equals("tcc8935s")) {					
            if (preference == mHost) {
               if (usb_drd_mode.equals("usb_device")) {
                  Log.e(TAG, "USB port status not change, port is device mode");
                  updateToggles(UsbManager.USB_FUNCTION_DEVICE);
               } else {
                  mUsbManager.setCurrentFunction(UsbManager.USB_FUNCTION_HOST, true);
                  updateToggles(UsbManager.USB_FUNCTION_HOST);
               }
            } else if (preference == mDevice) {
               if (usb_drd_mode.equals("usb_host")) {
                  Log.e(TAG, "USB port status not change, port is host mode");
                  updateToggles(UsbManager.USB_FUNCTION_HOST);
               } else {
                  mUsbManager.setCurrentFunction(UsbManager.USB_FUNCTION_DEVICE, true);
                  updateToggles(UsbManager.USB_FUNCTION_DEVICE);
               }
            }
         } else {
            if (preference == mMtp) {
               mUsbManager.setCurrentFunction(UsbManager.USB_FUNCTION_MTP, true);
               updateToggles(UsbManager.USB_FUNCTION_MTP);
            } else if (preference == mPtp) {
               mUsbManager.setCurrentFunction(UsbManager.USB_FUNCTION_PTP, true);
               updateToggles(UsbManager.USB_FUNCTION_PTP);
            }
	  }
        return true;
    }
}
