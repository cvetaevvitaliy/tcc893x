package com.android.settings.telechips.ethernet;

import com.android.settings.R;
import com.android.settings.telechips.ethernet.EthernetConfigDialog;
import com.android.settings.telechips.ethernet.EthernetEnabler;

import android.net.ethernet.EthernetManager;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.content.Context;

import com.android.settings.SettingsPreferenceFragment;

public class EthernetSettings extends SettingsPreferenceFragment {
	private static final String KEY_TOGGLE_ETH = "toggle_eth";
	private static final String KEY_CONF_ETH = "eth_config";
	private static final String ETH_SERVICE = "ethernet";
	private EthernetEnabler mEthEnabler;
	private EthernetConfigDialog mEthConfigDialog;
	private Preference mEthConfigPref;

	@Override
	public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
		super.onPreferenceTreeClick(preferenceScreen, preference);

		if (preference == mEthConfigPref) {
			mEthConfigDialog.show();
		}
		return false;
	}

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        addPreferencesFromResource(R.xml.ethernet_settings);
        final PreferenceScreen preferenceScreen = getPreferenceScreen();
        mEthConfigPref = preferenceScreen.findPreference(KEY_CONF_ETH);
        /*
         * TO DO:
         * Add new perference screen for Etherenet Configuration
         */

        initToggles();
    }
    @Override
    public void onResume() {
        super.onResume();

        mEthEnabler.resume();
    }

    @Override
    public void onPause() {
        super.onPause();
        mEthEnabler.pause();
    }
    private void initToggles() {

        mEthEnabler = new EthernetEnabler(
                getActivity(),
                (EthernetManager) getSystemService(Context.ETH_SERVICE),
                (CheckBoxPreference) findPreference(KEY_TOGGLE_ETH),
                (Preference) findPreference(KEY_CONF_ETH));
        mEthConfigDialog = new EthernetConfigDialog(getActivity(), mEthEnabler);
        mEthEnabler.setConfigDialog(mEthConfigDialog);
    }
}
