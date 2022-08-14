package com.telechips.wfd;

import android.annotation.TargetApi;
import android.content.Context;
import android.content.res.Configuration;
import android.media.Ringtone;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.SystemProperties;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.preference.RingtonePreference;
import android.text.TextUtils;

import java.util.List;
import android.util.Log;
import android.net.wifi.p2p.WifiP2pManager.ActionListener;

/**
 * A {@link PreferenceActivity} that presents a set of application settings. On
 * handset devices, settings are presented as a single list. On tablets,
 * settings are split by category, with category headers shown to the left of
 * the list of settings.
 * <p>
 * See <a href="http://developer.android.com/design/patterns/settings.html">
 * Android Design: Settings</a> for design guidelines and the <a
 * href="http://developer.android.com/guide/topics/ui/settings.html">Settings
 * API Guide</a> for more information on developing a Settings UI.
 */
public class WifiDisplayAdvancedSettings extends PreferenceActivity implements
		Preference.OnPreferenceChangeListener {

	private static final String TAG = "WirelessDisplayAdvancedSetting";
	private static final String KEY_WFD_MIRACAST_CERT = "wfd_miracast_certi";
	//private static final String KEY_WFD_HDCP = "wfd_hdcp";
	private static final String KEY_WFD_GO_INTENT = "wfd_go_intent";
	private static final String KEY_WFD_LISTEN_CHANNEL = "wfd_listen_channel";
	private static final String KEY_WFD_OPERATING_CHANNEL = "wfd_operaing_channel";
	//private static final String KEY_WFD_AUDIO_CODEC = "wfd_audio_codec";
	private static final String KEY_WFD_PERSISTENT_GO = "wfd_persistent_go";
	//private static final String KEY_WFD_AUTO_PBC = "wfd_auto_pbc";
	//private static final String KEY_WFD_FILE_DUMP = "wfd_file_dump";
	private static final String KEY_WFD_DEBUG_MODE = "wfd_sink_debug";
	//private static final String KEY_WFD_FRAME_SKIP = "wfd_frame_skip";
	//private static final String KEY_WFD_FRAME_SKIP_TIME = "wfd_frame_skip_time";
	//private static final String KEY_WFD_DYNAMIC_FRAME_SKIP = "wfd_dynamic_frame_skip";
	//private static final String KEY_WFD_VGA = "wfd_vga";

	public static final String PROPERTY_LISTEN_CHANNEL = "tcc.wifi.p2p.listening.channel";
	public static final String PROPERTY_OPERATING_CHANNEL = "tcc.wifi.p2p.operating.channel";
	//public static final String PROPERTY_AUDIO_CODEC = "tcc.wfd.LPCM";
	public static final String PROPERTY_MIRACAST_CERT = "tcc.wifi.display.miracast";
	//public static final String PROPERTY_HDCP = "tcc.hdcp2.enable";
	public static final String PROPERTY_GO_INTENT = "tcc.wifi.p2p.go_intent";
	public static final String PROPERTY_PERSISTENT_GO = "tcc.wifi.p2p.persistent";
	//public static final String PROPERTY_AUTOMATIC_PBC = "tcc.wifi.p2p.auto_pbc"; // don't use auto pbc property from Kitkat
	//public static final String PROPERTY_FILE_DUMP = "tcc.wfd.filedump";
	public static final String PROPERTY_DEBUG_MODE = "tcc.wfd.sink.debug";
	//public static final String PROPERTY_FRAME_SKIP = "tcc.wfd.skip";
	//public static final String PROPERTY_FRAME_SKIP_TIME = "tcc.wfd.skip.time";
	//public static final String PROPERTY_DYNAMIC_FRAME_SKIP = "tcc.wfd.skip.dynamic";
	//public static final String PROPERTY_VGA = "tcc.wfd.vga";

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.wifi_display_advanced_settings);

	}

	@Override
	public void onResume() {
		super.onResume();
		initPreferences();
	}

	private void initPreferences() {
		/* Miracast Cert */
		CheckBoxPreference miracastCert = (CheckBoxPreference) findPreference(KEY_WFD_MIRACAST_CERT);

		int iMiracastCert = 0;
		String strMiracastCert = SystemProperties.get(PROPERTY_MIRACAST_CERT,
				"0");
		if (strMiracastCert != null)
			iMiracastCert = Integer.parseInt(strMiracastCert);

		if (iMiracastCert == 1)
			miracastCert.setChecked(true);
		else if (iMiracastCert == 0)
			miracastCert.setChecked(false);

		/* Persistent GO */
		CheckBoxPreference persistentGO = (CheckBoxPreference) findPreference(KEY_WFD_PERSISTENT_GO);

		int iPersistentGO = 0;
		String strPersistentGO = SystemProperties.get(PROPERTY_PERSISTENT_GO, "0");
		if (strPersistentGO != null)
			iPersistentGO = Integer.parseInt(strPersistentGO);

		if (iPersistentGO == 1)
			persistentGO.setChecked(true);
		else if (iMiracastCert == 0)
			persistentGO.setChecked(false);

		/* Automatic PBC */
        /*
		CheckBoxPreference autoPbc = (CheckBoxPreference) findPreference(KEY_WFD_AUTO_PBC);

        
		int iAutoPbc = 0;
		String strAutoPbc = SystemProperties.get(PROPERTY_AUTOMATIC_PBC, "0");
		if (strAutoPbc != null)
			iAutoPbc = Integer.parseInt(strAutoPbc);

		if (iAutoPbc == 1)
			autoPbc.setChecked(true);
		else if (iAutoPbc == 0)
			autoPbc.setChecked(false);		
        */
        
		/* File dump */
        /*
		CheckBoxPreference fileDump = (CheckBoxPreference) findPreference(KEY_WFD_FILE_DUMP);

		int iFileDump = 0;
		String strFileDump = SystemProperties.get(PROPERTY_FILE_DUMP, "0");
		if (strFileDump != null)
			iFileDump = Integer.parseInt(strFileDump);

		if (iFileDump == 1)
			fileDump.setChecked(true);
		else if (iFileDump == 0)
			fileDump.setChecked(false);				
		*/
		
		/* HDCP */
		/*
		ListPreference hdcpEnable = (ListPreference) findPreference(KEY_WFD_HDCP);
		String strHdcpEnable = SystemProperties.get(PROPERTY_HDCP);
		if (strHdcpEnable != null)
			hdcpEnable.setValue(strHdcpEnable);
		hdcpEnable.setOnPreferenceChangeListener(this);
		updateHDCPEnableSummary(hdcpEnable, strHdcpEnable);
		*/
		
		/* GO intent */
		ListPreference goIntent = (ListPreference) findPreference(KEY_WFD_GO_INTENT);
		String strGoIntent = SystemProperties.get(PROPERTY_GO_INTENT);
		if (strGoIntent != null)
			goIntent.setValue(strGoIntent);
		goIntent.setOnPreferenceChangeListener(this);
		updateGOIntentSummary(goIntent, strGoIntent);

		/* Listen Channel */
		ListPreference listenChannel = (ListPreference) findPreference(KEY_WFD_LISTEN_CHANNEL);
		String strListenChannel = SystemProperties.get(PROPERTY_LISTEN_CHANNEL);
		if (strListenChannel != null)
			goIntent.setValue(strListenChannel);
		listenChannel.setOnPreferenceChangeListener(this);
		updateListenChannelSummary(listenChannel, strListenChannel);

		/* Operating Channel */
		ListPreference operatingChannel = (ListPreference) findPreference(KEY_WFD_OPERATING_CHANNEL);
		String strOperatingChannel = SystemProperties
				.get(PROPERTY_OPERATING_CHANNEL);
		if (strOperatingChannel != null)
			operatingChannel.setValue(strOperatingChannel);
		operatingChannel.setOnPreferenceChangeListener(this);
		updateOperatingChannelSummary(operatingChannel, strOperatingChannel);

		/* WFDSink debug mode */
		CheckBoxPreference wfdSinkDebug = (CheckBoxPreference) findPreference(KEY_WFD_DEBUG_MODE);

		int iWfdSinkDebug = 0;
		String strWfdSinkDebug = SystemProperties.get(PROPERTY_DEBUG_MODE,
				"0");
		if (strWfdSinkDebug != null)
			iWfdSinkDebug = Integer.parseInt(strWfdSinkDebug);

		if (iWfdSinkDebug == 1)
			wfdSinkDebug.setChecked(true);
		else if (iWfdSinkDebug == 0)
			wfdSinkDebug.setChecked(false);

		/* Frame skip */
        /*
		CheckBoxPreference frameSkip = (CheckBoxPreference) findPreference(KEY_WFD_FRAME_SKIP);

		int iFrameSkip = 0;
		String strFrameSkip = SystemProperties.get(PROPERTY_FRAME_SKIP,	"0");
		if (strFrameSkip != null)
			iFrameSkip = Integer.parseInt(strFrameSkip);

		if (iFrameSkip == 1)
			frameSkip.setChecked(true);
		else if (iFrameSkip == 0)
			frameSkip.setChecked(false);
        */
        
		/* Frame Skip Time */
        /*
		ListPreference frameSkipTime = (ListPreference) findPreference(KEY_WFD_FRAME_SKIP_TIME);
		String strFrameSkipTime = SystemProperties.get(PROPERTY_FRAME_SKIP_TIME);
		if (strFrameSkipTime != null)
			frameSkipTime.setValue(strFrameSkipTime);
		frameSkipTime.setOnPreferenceChangeListener(this);
		updateFrameSkipTimeSummary(frameSkipTime, strFrameSkipTime);
        */
        
		/* Dynamic Frame skip */
		/*
		CheckBoxPreference dynamicFrameSkip = (CheckBoxPreference) findPreference(KEY_WFD_DYNAMIC_FRAME_SKIP);

		int iDynamicFrameSkip = 0;
		String strDynamicFrameSkip = SystemProperties.get(PROPERTY_DYNAMIC_FRAME_SKIP,	"0");
		if (strDynamicFrameSkip != null)
			iFrameSkip = Integer.parseInt(strDynamicFrameSkip);

		if (iDynamicFrameSkip == 1)
			dynamicFrameSkip.setChecked(true);
		else if (iDynamicFrameSkip == 0)
			dynamicFrameSkip.setChecked(false);
		*/
		
		/* VGA */
        /*
		CheckBoxPreference vga = (CheckBoxPreference) findPreference(KEY_WFD_VGA);

		int iVga = 0;
		String strVga = SystemProperties.get(PROPERTY_VGA,	"0");
		if (strVga != null)
			iVga = Integer.parseInt(strVga);

		if (iVga == 1)
			vga.setChecked(true);
		else if (iVga == 0)
			vga.setChecked(false);
		*/
		
		/* Audio Codec */
		/*
		ListPreference audioCodec = (ListPreference) findPreference(KEY_WFD_AUDIO_CODEC);
		String strAudioCodec = SystemProperties.get(PROPERTY_AUDIO_CODEC);
		if (strAudioCodec != null)
			audioCodec.setValue(strAudioCodec);
		audioCodec.setOnPreferenceChangeListener(this);
		updateAudioCodecSummary(audioCodec, strAudioCodec);
		*/
	}

    /*
	private void updateHDCPEnableSummary(Preference pref, String value) {
		if (value != null) {
			String[] values = getResources().getStringArray(
					R.array.wireless_display_setting_hdcp_values);
			String[] summaries = getResources().getStringArray(
					R.array.wireless_display_setting_hdcp_entries);
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
    */
    
	private void updateGOIntentSummary(Preference pref, String value) {
		if (value != null) {
			String[] values = getResources().getStringArray(
					R.array.wireless_display_setting_go_intent_values);
			String[] summaries = getResources().getStringArray(
					R.array.wireless_display_setting_go_intent_entries);
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
			String[] values = getResources().getStringArray(
					R.array.wireless_display_setting_listen_channel_values);
			String[] summaries = getResources().getStringArray(
					R.array.wireless_display_setting_listen_channel_entries);
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
			String[] values = getResources().getStringArray(
					R.array.wireless_display_setting_audio_codec_values);
			String[] summaries = getResources().getStringArray(
					R.array.wireless_display_setting_audio_codec_entries);
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
			String[] values = getResources().getStringArray(
					R.array.wireless_display_setting_operating_channel_values);
			String[] summaries = getResources().getStringArray(
					R.array.wireless_display_setting_operating_channel_entries);
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
	
	private void updateFrameSkipTimeSummary(Preference pref, String value) {
		if (value != null) {
			String[] values = getResources().getStringArray(
					R.array.wireless_display_setting_frame_skip_time_values);
			String[] summaries = getResources().getStringArray(
					R.array.wireless_display_setting_frame_skip_time_entries);
			for (int i = 0; i < values.length; i++) {
				if (value.equals(values[i])) {
					if (i < summaries.length) {
						pref.setSummary(summaries[i]);
						return;
					}
				}
			}
		}

		pref.setSummary(R.string.wireless_display_setting_frame_skip_time_summary);
	}

	@Override
	public boolean onPreferenceTreeClick(PreferenceScreen screen,
			Preference preference) {
		String key = preference.getKey();
		if (KEY_WFD_MIRACAST_CERT.equals(key)) {
			SystemProperties.set(PROPERTY_MIRACAST_CERT,
					((CheckBoxPreference) preference).isChecked() ? "1" : "0");
		} else if (KEY_WFD_PERSISTENT_GO.equals(key)) {
			SystemProperties.set(PROPERTY_PERSISTENT_GO,
					((CheckBoxPreference) preference).isChecked() ? "1" : "0");
/*            
		} else if (KEY_WFD_AUTO_PBC.equals(key)) {
			SystemProperties.set(PROPERTY_AUTOMATIC_PBC,
					((CheckBoxPreference) preference).isChecked() ? "1" : "0");
		} else if (KEY_WFD_FILE_DUMP.equals(key)) {
			SystemProperties.set(PROPERTY_FILE_DUMP,
					((CheckBoxPreference) preference).isChecked() ? "1" : "0");
*/
					
		} else if (KEY_WFD_DEBUG_MODE.equals(key)) {
			SystemProperties.set(PROPERTY_DEBUG_MODE,
					((CheckBoxPreference) preference).isChecked() ? "1" : "0");
/*          
		} else if (KEY_WFD_FRAME_SKIP.equals(key)) {
			SystemProperties.set(PROPERTY_FRAME_SKIP,
					((CheckBoxPreference) preference).isChecked() ? "1" : "0");
		
		} else if (KEY_WFD_DYNAMIC_FRAME_SKIP.equals(key)) {
			SystemProperties.set(PROPERTY_DYNAMIC_FRAME_SKIP,
					((CheckBoxPreference) preference).isChecked() ? "1" : "0");
		}else if (KEY_WFD_VGA.equals(key)) {
			SystemProperties.set(PROPERTY_VGA,
					((CheckBoxPreference) preference).isChecked() ? "1" : "0");
*/					
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
        
		/*
		if (KEY_WFD_FRAME_SKIP_TIME.equals(key)) {
			SystemProperties.set(PROPERTY_FRAME_SKIP_TIME, (String) newValue);
			setFrameSkipTimeOption(newValue);
			updateFrameSkipTimeSummary(preference, (String) newValue);
		}
		if (KEY_WFD_AUDIO_CODEC.equals(key)) {
			SystemProperties.set(PROPERTY_AUDIO_CODEC, (String) newValue);
			updateAudioCodecSummary(preference, (String) newValue);
		}
		*/
		return true;
	}

	private void setP2pListenChannelOption(Object newValue) {
		int channel = newValue != null ? Integer.parseInt(newValue.toString())
				: -1;
		SystemProperties.set(PROPERTY_LISTEN_CHANNEL,
				Integer.toString(channel));
		TelechipsWFDSink.getInstance().mWifiP2pManager.setListenChannel(
				TelechipsWFDSink.getInstance().mChannel, channel);
	}

	private void setP2pOperatingChannelOption(Object newValue) {
		int channel = newValue != null ? Integer.parseInt(newValue.toString())
				: -1;
		SystemProperties.set(PROPERTY_OPERATING_CHANNEL,	Integer.toString(channel));
        setWifiP2pChannels(0, channel);
        
	}

    /*
	private void setFrameSkipTimeOption(Object newValue) {
		int channel = newValue != null ? Integer.parseInt(newValue.toString())
				: -1;
		SystemProperties.set(PROPERTY_FRAME_SKIP_TIME,
				Integer.toString(channel));
	}
	*/

    private void setWifiP2pChannels(final int lc, final int oc) {
        Log.d(TAG, "Setting wifi p2p channel: lc=" + lc + ", oc=" + oc);
        TelechipsWFDSink.getInstance().mWifiP2pManager.setWifiP2pChannels(TelechipsWFDSink.getInstance().mChannel,
            lc, oc, new ActionListener() {
        @Override
        public void onSuccess() {
                Log.d(TAG, "Successfully set wifi p2p channels.");
        }

        @Override
        public void onFailure(int reason) {
            Log.e(TAG, "Failed to set wifi p2p channels with reason " + reason + ".");
        }
    });
}

}
