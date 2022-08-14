/*
# Copyright (C) 2013 Telechips, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
*/

package com.android.settings.wfd;

import android.content.Context;
import android.net.wifi.p2p.WifiP2pManager;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceScreen;
import android.provider.Settings;
import android.provider.Settings.Global;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import com.android.settings.R;
import com.android.settings.SettingsPreferenceFragment;
import com.android.settings.Utils;
import android.os.SystemProperties;

public class WirelessDisplayAdvancedSettings extends SettingsPreferenceFragment
        implements Preference.OnPreferenceChangeListener {

    private static final String TAG = "WirelessDisplayAdvancedSetting";
    private static final String KEY_WFD_MIRACAST_CERT = "wfd_miracast_certi";
    //private static final String KEY_WFD_HDCP = "wfd_hdcp";
    private static final String KEY_WFD_GO_INTENT = "wfd_go_intent";
    private static final String KEY_WFD_LISTEN_CHANNEL = "wfd_listen_channel";
    private static final String KEY_WFD_OPERATING_CHANNEL = "wfd_operaing_channel";
    private static final String KEY_WFD_AUDIO_CODEC = "wfd_audio_codec";
    private static final String KEY_WFD_PERSISTENT_GO = "wfd_persistent_go";

	private static final String PROPERTY_LISTEN_CHANNEL = "tcc.wifi.p2p.listening.channel";
	private static final String PROPERTY_OPERATING_CHANNEL = "tcc.wifi.p2p.operating.channel";
	private static final String PROPERTY_AUDIO_CODEC = "tcc.wfd.LPCM";
	private static final String PROPERTY_MIRACAST_CERT = "tcc.wifi.display.miracast";	
	//private static final String PROPERTY_HDCP = "tcc.hdcp2.enable";
	private static final String PROPERTY_GO_INTENT = "tcc.wifi.p2p.go_intent";
	private static final String PROPERTY_PERSISTENT_GO = "tcc.wifi.p2p.persistent";
	
    private WifiP2pManager mWifiP2pManager;
    private WifiP2pManager.Channel mChannel;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.wireless_display_advanced_settings);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
		Context context = getActivity();
        mWifiP2pManager = (WifiP2pManager) getSystemService(Context.WIFI_P2P_SERVICE);
        if(mWifiP2pManager != null) {
		    mChannel = mWifiP2pManager.initialize(context, getActivity().getMainLooper(), null);
		    if (mChannel == null)
		        mWifiP2pManager = null;
		}

    }

    @Override
    public void onResume() {
        super.onResume();
        initPreferences();
    }

    private void initPreferences() {
		/* Miracast Cert */
        CheckBoxPreference miracastCert = (CheckBoxPreference) findPreference(KEY_WFD_MIRACAST_CERT);

		int iMiracastCert=0;
		String strMiracastCert = SystemProperties.get(PROPERTY_MIRACAST_CERT,"0");
		if(strMiracastCert != null)
        	iMiracastCert = Integer.parseInt(strMiracastCert);

		if(iMiracastCert==1)
	        miracastCert.setChecked(true);
		else if(iMiracastCert==0)
			miracastCert.setChecked(false);

		/* Persistent GO */
        CheckBoxPreference persistentGO = (CheckBoxPreference) findPreference(KEY_WFD_PERSISTENT_GO);

		int iPersistentGO=0;
		String strPersistentGO = SystemProperties.get(PROPERTY_PERSISTENT_GO,"0");
		if(strPersistentGO != null)
        	iPersistentGO = Integer.parseInt(strPersistentGO);

		if(iPersistentGO==1)
	        persistentGO.setChecked(true);
		else if(iMiracastCert==0)
			persistentGO.setChecked(false);

		/* HDCP */
		/*
        ListPreference hdcpEnable = (ListPreference) findPreference(KEY_WFD_HDCP);
		String strHdcpEnable = SystemProperties.get(PROPERTY_HDCP);
		if(strHdcpEnable!=null)
	        hdcpEnable.setValue(strHdcpEnable);
		hdcpEnable.setOnPreferenceChangeListener(this);
		updateHDCPEnableSummary(hdcpEnable, strHdcpEnable);
		*/
		
		/* GO intent */
		ListPreference goIntent = (ListPreference) findPreference(KEY_WFD_GO_INTENT);
		String strGoIntent = SystemProperties.get(PROPERTY_GO_INTENT);
		if(strGoIntent!=null)
	        goIntent.setValue(strGoIntent);
		goIntent.setOnPreferenceChangeListener(this);
		updateGOIntentSummary(goIntent, strGoIntent);

		/* Listen Channel */
		ListPreference listenChannel = (ListPreference) findPreference(KEY_WFD_LISTEN_CHANNEL);
		String strListenChannel = SystemProperties.get(PROPERTY_LISTEN_CHANNEL);
		if(strListenChannel!=null)
	        goIntent.setValue(strListenChannel);
		listenChannel.setOnPreferenceChangeListener(this);
		updateListenChannelSummary(listenChannel, strListenChannel);
		
		/* Operating Channel */
		ListPreference operatingChannel = (ListPreference) findPreference(KEY_WFD_OPERATING_CHANNEL);
		String strOperatingChannel = SystemProperties.get(PROPERTY_OPERATING_CHANNEL);
		if(strOperatingChannel!=null)
	        operatingChannel.setValue(strOperatingChannel);
		operatingChannel.setOnPreferenceChangeListener(this);
		updateOperatingChannelSummary(operatingChannel, strOperatingChannel);

		/* Audio Codec */
		ListPreference audioCodec = (ListPreference) findPreference(KEY_WFD_AUDIO_CODEC);
		String strAudioCodec = SystemProperties.get(PROPERTY_AUDIO_CODEC);
		if(strAudioCodec!=null)
	        audioCodec.setValue(strAudioCodec);
		audioCodec.setOnPreferenceChangeListener(this);
		updateAudioCodecSummary(audioCodec, strAudioCodec);
		
    }

    private void updateHDCPEnableSummary(Preference pref, String value) {
        if (value != null) {
            String[] values = getResources().getStringArray(R.array.wireless_display_setting_hdcp_values);
            String[] summaries = getResources().getStringArray(R.array.wireless_display_setting_hdcp_entries);
            for (int i = 0; i < values.length; i++) {
                if (value.equals(values[i])) {
                    if (i < summaries.length) {
                        pref.setSummary(summaries[i]);
                        return;
                    }
                }
            }
        }

        pref.setSummary(R.string.wireless_display_setting_hdcp_summary);
    }

    private void updateGOIntentSummary(Preference pref, String value) {
        if (value != null) {
            String[] values = getResources().getStringArray(R.array.wireless_display_setting_go_intent_values);
            String[] summaries = getResources().getStringArray(R.array.wireless_display_setting_go_intent_entries);
            for (int i = 0; i < values.length; i++) {
                if (value.equals(values[i])) {
                    if (i < summaries.length) {
                        pref.setSummary(summaries[i]);
                        return;
                    }
                }
            }
        }

        pref.setSummary(R.string.wireless_display_setting_go_intent_summary);
    }

    private void updateListenChannelSummary(Preference pref, String value) {
        if (value != null) {
            String[] values = getResources().getStringArray(R.array.wireless_display_setting_listen_channel_values);
            String[] summaries = getResources().getStringArray(R.array.wireless_display_setting_listen_channel_entries);
            for (int i = 0; i < values.length; i++) {
                if (value.equals(values[i])) {
                    if (i < summaries.length) {
                        pref.setSummary(summaries[i]);
                        return;
                    }
                }
            }
        }

        pref.setSummary(R.string.wireless_display_setting_listen_channel_summary);
    }

    private void updateAudioCodecSummary(Preference pref, String value) {
        if (value != null) {
            String[] values = getResources().getStringArray(R.array.wireless_display_setting_audio_codec_values);
            String[] summaries = getResources().getStringArray(R.array.wireless_display_setting_audio_codec_entries);
            for (int i = 0; i < values.length; i++) {
                if (value.equals(values[i])) {
                    if (i < summaries.length) {
                        pref.setSummary(summaries[i]);
                        return;
                    }
                }
            }
        }

        pref.setSummary(R.string.wireless_display_setting_audio_codec_summary);
    }

    private void updateOperatingChannelSummary(Preference pref, String value) {
        if (value != null) {
            String[] values = getResources().getStringArray(R.array.wireless_display_setting_operating_channel_values);
            String[] summaries = getResources().getStringArray(R.array.wireless_display_setting_operating_channel_entries);
            for (int i = 0; i < values.length; i++) {
                if (value.equals(values[i])) {
                    if (i < summaries.length) {
                        pref.setSummary(summaries[i]);
                        return;
                    }
                }
            }
        }

        pref.setSummary(R.string.wireless_display_setting_operating_channel_summary);
    }


    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen screen, Preference preference) {
        String key = preference.getKey();
        if (KEY_WFD_MIRACAST_CERT.equals(key)) {
			SystemProperties.set(PROPERTY_MIRACAST_CERT, ((CheckBoxPreference) preference).isChecked() ? "1" : "0");
        } else if (KEY_WFD_PERSISTENT_GO.equals(key)) {
			SystemProperties.set(PROPERTY_PERSISTENT_GO, ((CheckBoxPreference) preference).isChecked() ? "1" : "0");
	    } else {
            return super.onPreferenceTreeClick(screen, preference);
        }
        return true;
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {    
        String key = preference.getKey();

/*
        if (KEY_WFD_HDCP.equals(key)) {
			SystemProperties.set(PROPERTY_HDCP, (String) newValue);
			updateHDCPEnableSummary(preference, (String) newValue);
        }
*/
        if (KEY_WFD_GO_INTENT.equals(key)) {
			SystemProperties.set(PROPERTY_GO_INTENT, (String) newValue);
			updateGOIntentSummary(preference, (String) newValue);		
        }

        if (KEY_WFD_LISTEN_CHANNEL.equals(key)) {		
			SystemProperties.set(PROPERTY_LISTEN_CHANNEL, (String) newValue);
			setP2pListenChannelOption(newValue);
			updateListenChannelSummary(preference, (String) newValue);
        }

        if (KEY_WFD_OPERATING_CHANNEL.equals(key)) {	
			SystemProperties.set(PROPERTY_OPERATING_CHANNEL, (String) newValue);
			setP2pOperatingChannelOption(newValue);
			updateOperatingChannelSummary(preference, (String) newValue);
        }

        if (KEY_WFD_AUDIO_CODEC.equals(key)) {	
			SystemProperties.set(PROPERTY_AUDIO_CODEC, (String) newValue);
			updateAudioCodecSummary(preference, (String) newValue);
        }

        return true;
    }

    private void setP2pListenChannelOption(Object newValue) {
        int channel = newValue != null ? Integer.parseInt(newValue.toString()) : -1;
        SystemProperties.set(PROPERTY_LISTEN_CHANNEL, Integer.toString(channel));
        mWifiP2pManager.setListenChannel(mChannel, channel);
    }
	
    private void setP2pOperatingChannelOption(Object newValue) {
        int channel = newValue != null ? Integer.parseInt(newValue.toString()) : -1;
        SystemProperties.set(PROPERTY_OPERATING_CHANNEL, Integer.toString(channel));
    }
    
}

