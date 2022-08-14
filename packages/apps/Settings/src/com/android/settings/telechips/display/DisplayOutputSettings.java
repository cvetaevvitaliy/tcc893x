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

package com.android.settings.telechips.display;

import android.util.Log;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.preference.MultiSelectListPreference;
import android.provider.Settings;
import android.os.Bundle;
import android.os.SystemProperties;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import java.util.ArrayList;

import android.app.TelechipsDisplayManager;
import android.content.BroadcastReceiver;
import android.content.IntentFilter;

import android.app.AlertDialog;
import android.content.DialogInterface;
import java.util.Timer;
import java.util.TimerTask;
import java.util.HashSet;
import java.util.Set;

import com.android.settings.R;
import com.android.settings.SettingsPreferenceFragment;

public class DisplayOutputSettings extends SettingsPreferenceFragment implements
        Preference.OnPreferenceChangeListener {
    static final String LOG_TAG = "DisplayOutputSettings";

    /**
     * Output Select, HDMI, Component, Composite
     */
    public static final String OUTPUT_MODE = "output_mode";

    public static final String HDMI_RESOLUTION = "hdmi_resolution";
    public static final String HDMI_AUTO_SELECT = "hdmi_auto_select";
    public static final String HDMI_CEC_SELECT = "hdmi_cec_select";
    public static final String HDMI_MODE = "hdmi_mode";
    public static final String HDMI_ASPECT_RATIO = "hdmi_aspect_ratio";
    public static final String HDMI_COLOR_SPACE = "hdmi_color_space";
    public static final String HDMI_COLOR_DEPTH = "hdmi_color_depth";
    public static final String COMPOSITE_MODE = "composite_mode";
    public static final String COMPONENT_RESOLUTION = "component_resolution";

    private static final int FALLBACK_OUTPUT_MODE_VALUE = 0;
    private static final int FALLBACK_HDMI_RESOLUTION_VALUE = 125;
    private static final int FALLBACK_HDMI_AUTO_SELECT = 0;
    private static final int FALLBACK_HDMI_CEC_SELECT = 0;
    private static final int FALLBACK_HDMI_MODE_VALUE = 0;
    private static final int FALLBACK_HDMI_ASPECT_RATIO_VALUE  = 0;
    private static final int FALLBACK_HDMI_COLOR_SPACE_VALUE = 0;
    private static final int FALLBACK_HDMI_COLOR_DEPTH_VALUE = 0;
    private static final int FALLBACK_COMPOSITE_MODE_VALUE = 0;
    private static final int FALLBACK_COMPONENT_RESOLUTION_VALUE = 0;

    private static final String KEY_OUTPUT_MODE_CATEGORY = "output_mode_settings_category";
    private static final String KEY_OUTPUT_MODE = "output_mode";

    private static final String KEY_HDMI_CATEGORY = "hdmi_settings_category";
    private static final String KEY_HDMI_RESOLUTION = "hdmi_resolution";
    private static final String KEY_HDMI_AUTO_SELECT = "hdmi_auto_select";
    private static final String KEY_HDMI_CEC_SELECT = "hdmi_cec_select";
    private static final String KEY_HDMI_MODE = "hdmi_mode";
    private static final String KEY_HDMI_ASPECT_RATIO = "hdmi_aspect_ratio";
    private static final String KEY_HDMI_COLOR_SPACE= "hdmi_color_space";
    private static final String KEY_HDMI_COLOR_DEPTH = "hdmi_color_depth";

    private static final String KEY_COMPOSITE_CATEGORY = "composite_settings_category";
    private static final String KEY_COMPOSITE_MODE = "composite_mode";

    private static final String KEY_COMPONENT_CATEGORY = "component_settings_category";
    private static final String KEY_COMPONENT_RESOLUTION = "component_resolution";

    private static final String KEY_OSD_CATEGORY = "osd_position_setting_category";
    private static final String KEY_OSD_SETTING = "osd_setting";

    public static final String NATIVE_MODE = "native_mode";
    public static final String NATIVE_MODE_SELECT = "native_mode_select";
    public static final String KEY_NATIVE_MODE_SELECT = "native_mode_select";
    private static final String KEY_NATIVE_MODE_CATEGORY = "native_video_mode_category";
    private static final String KEY_NATIVE_MODE_SETTING = "native_video_mode";

    private ListPreference mOutputMode;
    private ListPreference mHDMIResolution;
    private CheckBoxPreference mHDMIAutoSelect;
    private CheckBoxPreference mHDMICECSelect;
    private ListPreference mHDMIMode;
    private ListPreference mHDMIAspectRatio;
    private ListPreference mHDMIColorSpace;
    private ListPreference mHDMIColorDepth;
    private ListPreference mCompositeMode;
    private ListPreference mComponentResolution;
    private CheckBoxPreference mNativeModeSelect;
    private MultiSelectListPreference mNativeMode;

    private String product;
    private boolean mHDMI_active;
    private boolean mComposite_active;
    private boolean mComponent_active;
    private boolean mOSD_active;
    private boolean mCEC_active;
    private boolean mHDMI_color_depth_enable;

    private boolean hdmi_fhd_resolution;
    private boolean hdmi_portable_resolution;

    private TelechipsDisplayManager mDM;

    private AlertDialog alert_confirm;
    private int alert_confirm_type = 0;
    private int alert_confirm_value = 0;
    private int alert_count = 13;

    private final static int STATUS_UPDATE = 0;
    private final static int COUNT_UPDATE = 1;

    private final BroadcastReceiver mOutputModeReceiver = new BroadcastReceiver() {
    @Override
    public void onReceive(Context context, Intent intent) {
    Log.d(LOG_TAG, "intent.getAction() " + intent.getAction());

        if (intent.getAction().equals(TelechipsDisplayManager.DISPLAY_MODE_CHANGED_ACTION)) {
           handleOutputModeChanged(
                intent.getIntExtra(TelechipsDisplayManager.EXTRA_DISPLAY_MODE, TelechipsDisplayManager.DISPLAY_OUTPUT_LCD),
                intent.getIntExtra(TelechipsDisplayManager.EXTRA_PREVIOUS_DISPLAY_MODE, TelechipsDisplayManager.DISPLAY_OUTPUT_LCD));
            }

    if (intent.getAction().equals(TelechipsDisplayManager.DISPLAY_RESOLUTION_CHANGED_ACTION)) {
           handleOutputResolutionChanged(
                intent.getIntExtra(TelechipsDisplayManager.EXTRA_DISPLAY_RESOLUTION, TelechipsDisplayManager.DISPLAY_DEFAULT_HDMI_RESOLUTION),
                intent.getIntExtra(TelechipsDisplayManager.EXTRA_PREVIOUS_DISPLAY_RESOLUTION, TelechipsDisplayManager.DISPLAY_DEFAULT_HDMI_RESOLUTION));
            }

    if (intent.getAction().equals(TelechipsDisplayManager.HDMI_MODE_CHANGED_ACTION)) {
           handleHDMIModeChanged(
                intent.getIntExtra(TelechipsDisplayManager.EXTRA_HDMI_MODE, TelechipsDisplayManager.DISPLAY_DEFAULT_HDMI_MODE),
                intent.getIntExtra(TelechipsDisplayManager.EXTRA_PREVIOUS_HDMI_MODE, TelechipsDisplayManager.DISPLAY_DEFAULT_HDMI_MODE));
            }
        }
    };

    private Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
				boolean mediaBox = SystemProperties.get("tcc.solution.mbox","0").equals("1");
		
                switch (msg.what) {
                        case STATUS_UPDATE:
                               if(alert_confirm_type == 0){
                                    Settings.System.putInt(getContentResolver(), OUTPUT_MODE, alert_confirm_value);
                                    mOutputMode.setValue(Settings.System.getString(getContentResolver(), OUTPUT_MODE));
                               } else if(alert_confirm_type == 1){
                                     Settings.System.putInt(getContentResolver(), HDMI_RESOLUTION, alert_confirm_value);
                                     mHDMIResolution.setValue(Settings.System.getString(getContentResolver(), HDMI_RESOLUTION));
                               } else if(alert_confirm_type == 2){
                                     Settings.System.putInt(getContentResolver(), HDMI_MODE, alert_confirm_value);
                                     mHDMIMode.setValue(Settings.System.getString(getContentResolver(), HDMI_MODE));

					if(!mediaBox) {
						if("1".equals(mHDMIMode.getValue())){
							int resolution = Settings.System.getInt(getContentResolver(), HDMI_RESOLUTION, FALLBACK_HDMI_RESOLUTION_VALUE);
							if(resolution == 2 || resolution == 3){
								Settings.System.putInt(getContentResolver(), HDMI_RESOLUTION, 125);
								mHDMIResolution.setValue("125");
								updateHDMIResolutionSummary(mHDMIResolution.getValue());
							}

							String[] HDMIResolutionArray = getResources().getStringArray(R.array.dvi_resolution_entries);
							String[] HDMIvalues = getResources().getStringArray(R.array.dvi_resolution_values);

							mHDMIResolution.setEntries(	HDMIResolutionArray);
							mHDMIResolution.setEntryValues(HDMIvalues);

							mNativeModeSelect.setChecked(false);
							mNativeModeSelect.setEnabled(false);
							mNativeMode.setEnabled(false);
						} else if("0".equals(mHDMIMode.getValue())){
							String[] HDMIResolutionArray = getResources().getStringArray(R.array.hdmi_resolution_entries);
							String[] HDMIvalues = getResources().getStringArray(R.array.hdmi_resolution_values);

							mHDMIResolution.setEntries(HDMIResolutionArray);
							mHDMIResolution.setEntryValues(HDMIvalues);

							mNativeModeSelect.setEnabled(true);
						}
					}else {
						String[] HDMIResolutionArray = getResources().getStringArray(R.array.hdmi_resolution_entries);
						String[] HDMIvalues = getResources().getStringArray(R.array.hdmi_resolution_values);

						mHDMIResolution.setEntries(HDMIResolutionArray);
						mHDMIResolution.setEntryValues(HDMIvalues);

						mNativeModeSelect.setEnabled(true);
					}
                               } else if(alert_confirm_type == 3){
                                        Settings.System.putInt(getContentResolver(), COMPOSITE_MODE, alert_confirm_value);
                                        mCompositeMode.setValue(Settings.System.getString(getContentResolver(), COMPOSITE_MODE));
                                } else if(alert_confirm_type == 4){
                                        Settings.System.putInt(getContentResolver(), COMPONENT_RESOLUTION, alert_confirm_value);
                                        mComponentResolution.setValue(Settings.System.getString(getContentResolver(), COMPONENT_RESOLUTION));
                                }

                                Log.e(LOG_TAG, "set previous value: " + alert_confirm_value);

                                break;
                        case COUNT_UPDATE:
                                alert_count--;
                                alert_confirm.setMessage("This setting will be reset after " + alert_count + " seconds");

                                if(alert_count == 0){
                                        alert_count = 13;
                                        alert_confirm.cancel();
                                        handler.sendEmptyMessageDelayed(STATUS_UPDATE,0);
                                } else{
                                        handler.sendEmptyMessageDelayed(COUNT_UPDATE,1000);
                                }
                                break;
                        default:
                                break;
                }
                super.handleMessage(msg);
        }
   };
    /** Called when the activity is first created. */

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

    mDM = (TelechipsDisplayManager)getSystemService(Context.TELECHIPS_DISPLAY_SERVICE);
    IntentFilter intentFilter = new IntentFilter();
    intentFilter.addAction(TelechipsDisplayManager.DISPLAY_MODE_CHANGED_ACTION);
    intentFilter.addAction(TelechipsDisplayManager.DISPLAY_RESOLUTION_CHANGED_ACTION);
    intentFilter.addAction(TelechipsDisplayManager.HDMI_MODE_CHANGED_ACTION);
    getActivity().registerReceiver(mOutputModeReceiver, intentFilter);

        ContentResolver resolver = getContentResolver();

        addPreferencesFromResource(R.xml.display_output_settings);

    product = SystemProperties.get("ro.product.model");

    mHDMI_active = "true".equals(SystemProperties.get("ro.system.hdmi_active"));

    mComposite_active = "true".equals(SystemProperties.get("ro.system.composite_active"));

    mComponent_active = "true".equals(SystemProperties.get("ro.system.component_active"));

    hdmi_fhd_resolution = "fullhd".equals(SystemProperties.get("ro.system.hdmi_max_resolution"));
    hdmi_portable_resolution = "true".equals(SystemProperties.get("ro.system.hdmi_portable"));

    mCEC_active = "true".equals(SystemProperties.get("ro.system.cec_active"));

    mOSD_active = "true".equals(SystemProperties.get("ro.system.osd_active"));

    mHDMI_color_depth_enable = "1".equals(SystemProperties.get("persist.sys.color_depth_enable"));
//      Log.v(LOG_TAG, "onCreate HDMI: " + mHDMI_active + " Composite: " + mComposite_active + " Component: " + mComponent_active  + " fhd_resolution): " + hdmi_fhd_resolution +  " portabeHDMI: " +hdmi_portable_resolution);


        mOutputMode = (ListPreference) findPreference(KEY_OUTPUT_MODE);
        //mOutputMode.setValue(String.valueOf(Settings.System.getInt(
        //        resolver, OUTPUT_MODE, FALLBACK_OUTPUT_MODE_VALUE)));
        mOutputMode.setValue(SystemProperties.get("persist.sys.output_mode","0"));
        mOutputMode.setOnPreferenceChangeListener(this);

	    final CharSequence[] entries = mOutputMode.getEntries();
        final CharSequence[] values = mOutputMode.getEntryValues();
        ArrayList<CharSequence> revisedEntries = new ArrayList<CharSequence>();
        ArrayList<CharSequence> revisedValues = new ArrayList<CharSequence>();

		boolean mediaBox = SystemProperties.get("tcc.solution.mbox","0").equals("1");
		int display_type = SystemProperties.getInt("tcc.output.display.type", 1);

        mHDMIResolution = (ListPreference) findPreference(KEY_HDMI_RESOLUTION);
        mHDMIResolution.setValue(String.valueOf(Settings.System.getInt(resolver, HDMI_RESOLUTION, FALLBACK_HDMI_RESOLUTION_VALUE)));
        mHDMIResolution.setOnPreferenceChangeListener(this);

        //mHDMIAutoSelect = (CheckBoxPreference) findPreference(KEY_HDMI_AUTO_SELECT);
        //mHDMIAutoSelect.setChecked(Settings.System.getInt(
        //        resolver, HDMI_AUTO_SELECT, FALLBACK_HDMI_AUTO_SELECT) > 0);

        mHDMICECSelect = (CheckBoxPreference) findPreference(KEY_HDMI_CEC_SELECT);
        mHDMICECSelect.setChecked(Settings.System.getInt(
         resolver, HDMI_CEC_SELECT, FALLBACK_HDMI_CEC_SELECT) > 0);

        mHDMIMode = (ListPreference) findPreference(KEY_HDMI_MODE);
        mHDMIMode.setValue(String.valueOf(Settings.System.getInt(resolver, HDMI_MODE, FALLBACK_HDMI_MODE_VALUE)));
        mHDMIMode.setOnPreferenceChangeListener(this);

	    if(!hdmi_fhd_resolution)
	    {
	        // HDMI max resolution is HD size
	        String[] HDMIResolutionArray = getResources().getStringArray(R.array.hd_hdmi_resolution_entries);
	        String[] HDMIvalues = getResources().getStringArray(R.array.hd_hdmi_resolution_values);

	        mHDMIResolution.setEntries(HDMIResolutionArray);
	        mHDMIResolution.setEntryValues(HDMIvalues);
	    }
	    else if(hdmi_portable_resolution)
	    {
	        // HDMI max resolution is HD size
	        String[] HDMIResolutionArray = getResources().getStringArray(R.array.portable_hdmi_resolution_entries);
	        String[] HDMIvalues = getResources().getStringArray(R.array.portable_hdmi_resolution_values);

	        mHDMIResolution.setEntries(HDMIResolutionArray);
	        mHDMIResolution.setEntryValues(HDMIvalues);
	    }

        mHDMIAspectRatio = (ListPreference) findPreference(KEY_HDMI_ASPECT_RATIO);
        mHDMIAspectRatio.setValue(String.valueOf(Settings.System.getInt(resolver, HDMI_ASPECT_RATIO, FALLBACK_HDMI_ASPECT_RATIO_VALUE)));
        mHDMIAspectRatio.setOnPreferenceChangeListener(this);

        mHDMIColorSpace = (ListPreference) findPreference(KEY_HDMI_COLOR_SPACE);
        mHDMIColorSpace.setValue(String.valueOf(Settings.System.getInt(resolver, HDMI_COLOR_SPACE, FALLBACK_HDMI_COLOR_SPACE_VALUE)));
        mHDMIColorSpace.setOnPreferenceChangeListener(this);

        mHDMIColorDepth = (ListPreference) findPreference(KEY_HDMI_COLOR_DEPTH);
        mHDMIColorDepth.setValue(String.valueOf(Settings.System.getInt(resolver, HDMI_COLOR_DEPTH, FALLBACK_HDMI_COLOR_DEPTH_VALUE)));
        mHDMIColorDepth.setOnPreferenceChangeListener(this);

        mCompositeMode = (ListPreference) findPreference(KEY_COMPOSITE_MODE);
        mCompositeMode.setValue(String.valueOf(Settings.System.getInt(resolver, COMPOSITE_MODE, FALLBACK_COMPOSITE_MODE_VALUE)));
        mCompositeMode.setOnPreferenceChangeListener(this);

        mComponentResolution = (ListPreference) findPreference(KEY_COMPONENT_RESOLUTION);
        mComponentResolution.setValue(String.valueOf(Settings.System.getInt(resolver, COMPONENT_RESOLUTION, FALLBACK_COMPONENT_RESOLUTION_VALUE)));
        mComponentResolution.setOnPreferenceChangeListener(this);

        //Native Framerate Menu
        mNativeModeSelect = (CheckBoxPreference) findPreference(KEY_NATIVE_MODE_SELECT);
        mNativeModeSelect.setChecked(Settings.System.getInt(resolver, NATIVE_MODE_SELECT, 0) > 0);

        mNativeMode = (MultiSelectListPreference) findPreference(KEY_NATIVE_MODE_SETTING);
        int savedValue = Settings.System.getInt(resolver, NATIVE_MODE, 0);
        CharSequence[] nativeValues = mNativeMode.getEntryValues();
        Set<String> newValues = new HashSet<String>();

        for(int i=0 ; i < nativeValues.length ; i ++){
            if( (savedValue & (1<<i)) !=0){
                Log.i(LOG_TAG," nativeValues string " + nativeValues[i].toString());
                newValues.add(nativeValues[i].toString());
            }
        }
        mNativeMode.setValues(newValues);
        mNativeMode.setOnPreferenceChangeListener(this);

        disableUnusableItems(Integer.parseInt(mOutputMode.getValue()));

		if(mediaBox) {
			if(display_type == 2){
				getPreferenceScreen().removePreference(findPreference(KEY_OUTPUT_MODE_CATEGORY));
				getPreferenceScreen().removePreference(findPreference(KEY_OUTPUT_MODE));
				mComposite_active = false;
				mComponent_active = false;
			} else if(display_type == 1){
				mComponent_active = false;
			} else {
				int display_mode = SystemProperties.getInt("persist.sys.display.mode", 1);

				if(display_mode == 3) {
					getPreferenceScreen().removePreference(findPreference(KEY_OUTPUT_MODE_CATEGORY));
					getPreferenceScreen().removePreference(findPreference(KEY_OUTPUT_MODE));
				} else if(display_mode == 1 || display_mode == 2) {
					getPreferenceScreen().removePreference(findPreference(KEY_OUTPUT_MODE_CATEGORY));
					getPreferenceScreen().removePreference(findPreference(KEY_OUTPUT_MODE));
					mComponent_active = false;
				}
			}
		}

	    if(!mediaBox){
	        getPreferenceScreen().removePreference(findPreference(KEY_NATIVE_MODE_CATEGORY));
	        getPreferenceScreen().removePreference(findPreference(KEY_NATIVE_MODE_SETTING));
	        getPreferenceScreen().removePreference(findPreference(KEY_NATIVE_MODE_SELECT));
	    }

	    if(hdmi_portable_resolution)
	    {
	        getPreferenceScreen().removePreference(findPreference(KEY_OUTPUT_MODE_CATEGORY));
	        getPreferenceScreen().removePreference(findPreference(KEY_OUTPUT_MODE));
	    }

	    if(!mHDMI_active)
	    {
	        getPreferenceScreen().removePreference(findPreference(KEY_HDMI_CATEGORY));
	        getPreferenceScreen().removePreference(findPreference(KEY_HDMI_RESOLUTION));
	        getPreferenceScreen().removePreference(findPreference(KEY_HDMI_MODE));
	        getPreferenceScreen().removePreference(findPreference(KEY_HDMI_CEC_SELECT));
			getPreferenceScreen().removePreference(findPreference(KEY_HDMI_ASPECT_RATIO));
			getPreferenceScreen().removePreference(findPreference(KEY_HDMI_COLOR_SPACE));
			getPreferenceScreen().removePreference(findPreference(KEY_HDMI_COLOR_DEPTH));
	    }

	    if(!mComposite_active)
	    {
	        getPreferenceScreen().removePreference(findPreference(KEY_COMPOSITE_CATEGORY));
	        getPreferenceScreen().removePreference(findPreference(KEY_COMPOSITE_MODE));
	    }

	    if(!mComponent_active)
	    {
	        getPreferenceScreen().removePreference(findPreference(KEY_COMPONENT_CATEGORY));
	        getPreferenceScreen().removePreference(findPreference(KEY_COMPONENT_RESOLUTION));
	    }

	    if(!mOSD_active)
	    {
	        getPreferenceScreen().removePreference(findPreference(KEY_OSD_CATEGORY));
	        getPreferenceScreen().removePreference(findPreference(KEY_OSD_SETTING));
	    }

        if(!mCEC_active)
        {
            getPreferenceScreen().removePreference(findPreference(KEY_HDMI_CEC_SELECT));
        }

	    if(mediaBox) {
			if(mHDMI_color_depth_enable) {
				// Don't remove hdmi color depth menu
			} else {
				// Remove hdmi color depth menu
				getPreferenceScreen().removePreference(findPreference(KEY_HDMI_COLOR_DEPTH));
			}
	    } else {
	        getPreferenceScreen().removePreference(findPreference(KEY_HDMI_ASPECT_RATIO));
	        getPreferenceScreen().removePreference(findPreference(KEY_HDMI_COLOR_SPACE));
	        getPreferenceScreen().removePreference(findPreference(KEY_HDMI_COLOR_DEPTH));
	    }

	    if(mediaBox)
	    {
	        if(mHDMI_active)
	        {
	            revisedEntries.add(entries[1]);
	            revisedValues.add(values[1]);
	        }

	        if(mComposite_active)
	        {
	            revisedEntries.add(entries[2]);
	            revisedValues.add(values[2]);
	        }

	        if(mComponent_active)
	        {
	            revisedEntries.add(entries[3]);
	            revisedValues.add(values[3]);
	        }

	        mOutputMode.setEntries(revisedEntries.toArray(new CharSequence[revisedEntries.size()]));
	        mOutputMode.setEntryValues(revisedValues.toArray(new CharSequence[revisedValues.size()]));
	    }
	    else
	    {
	        revisedEntries.add(entries[0]);
	        revisedValues.add(values[0]);

	        if(mHDMI_active)
	        {
	            revisedEntries.add(entries[1]);
	            revisedValues.add(values[1]);
	        }

	        if(mComposite_active)
	        {
	            revisedEntries.add(entries[2]);
	            revisedValues.add(values[2]);
	        }

	        if(mComponent_active)
	        {
	            revisedEntries.add(entries[3]);
	            revisedValues.add(values[3]);
	        }

	        if (revisedEntries.size() != entries.length || revisedValues.size() != values.length)
	        {
	            mOutputMode.setEntries(revisedEntries.toArray(new CharSequence[revisedEntries.size()]));
	            mOutputMode.setEntryValues(revisedValues.toArray(new CharSequence[revisedValues.size()]));
	        }
	    }
    }

    @Override
    public void onDestroy() {
    super.onDestroy();
        getActivity().unregisterReceiver(mOutputModeReceiver);
    }

    @Override
    public void onResume() {
        super.onResume();
    updateState(true);
    }

    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        if (preference == mHDMIAutoSelect)
    {
/*
            int value = mHDMIAutoSelect.isChecked() ? 1 : 0;
        Log.v(LOG_TAG, "Select: " + value);
        Settings.System.putInt(getContentResolver(), HDMI_AUTO_SELECT,value);
*/
        }

    if(preference == mHDMICECSelect)
    {
        int value = mHDMICECSelect.isChecked() ? 1 : 0;
            Log.v(LOG_TAG, "Select: " + value);
            Settings.System.putInt(getContentResolver(), HDMI_CEC_SELECT, value);
    }

    if(preference == mNativeModeSelect)
    {
        int value = mNativeModeSelect.isChecked() ? 1 : 0;
        Settings.System.putInt(getContentResolver(), NATIVE_MODE_SELECT, value);

        mNativeMode.setEnabled(mNativeModeSelect.isChecked());

        Log.i(LOG_TAG, "mNativeMode Select: " );
    }

        return true;
    }

    private void disableUnusableItems(int outputMode) {
		int display_mode = SystemProperties.getInt("persist.sys.display.mode", 1);
		boolean mediaBox = SystemProperties.get("tcc.solution.mbox","0").equals("1");

		if(display_mode == 3) {
			mOutputMode.setEnabled(false);
			if(outputMode == 3) { /* component mode */
				mHDMIResolution.setEnabled(false);
				mHDMIMode.setEnabled(false);
				mCompositeMode.setEnabled(true);
				mComponentResolution.setEnabled(true);
				mHDMICECSelect.setChecked(false);
				mHDMICECSelect.setEnabled(false);
				mNativeModeSelect.setEnabled(false);
				mNativeMode.setEnabled(false);
			} else {
				mHDMIResolution.setEnabled(true);
				mHDMIMode.setEnabled(true);
				mCompositeMode.setEnabled(true);
				mComponentResolution.setEnabled(false);
				mHDMICECSelect.setEnabled(true);
				mNativeModeSelect.setEnabled(true);
				mNativeMode.setEnabled(mNativeModeSelect.isChecked());
				if(mediaBox) {
					int hdmi_resolution = SystemProperties.getInt("persist.sys.hdmi_resolution", 1);
					if( hdmi_resolution == 6 || hdmi_resolution == 7 || hdmi_resolution == 8)
						mHDMIAspectRatio.setEnabled(true);
					else
						mHDMIAspectRatio.setEnabled(false);

					mHDMIColorSpace.setEnabled(true);

					if(mHDMI_color_depth_enable)
						mHDMIColorDepth.setEnabled(true);
					else
						mHDMIColorDepth.setEnabled(false);
				}
			}
		} else if(display_mode == 2) {
			mOutputMode.setEnabled(false);
			if(outputMode == 2) { /* composite output */
				mHDMIResolution.setEnabled(false);
				mHDMIMode.setEnabled(false);
				mCompositeMode.setEnabled(true);
				mComponentResolution.setEnabled(false);
				mHDMICECSelect.setChecked(false);
				mHDMICECSelect.setEnabled(false);
				mNativeModeSelect.setEnabled(false);
				mNativeMode.setEnabled(false);
				if(mediaBox) {
					mHDMIAspectRatio.setEnabled(false);
					mHDMIColorSpace.setEnabled(false);
					mHDMIColorDepth.setEnabled(false);
				}
			} else {
				mHDMIResolution.setEnabled(true);
				mHDMIMode.setEnabled(true);
				mCompositeMode.setEnabled(true);
				mComponentResolution.setEnabled(false);
				mHDMICECSelect.setEnabled(true);
				mNativeModeSelect.setEnabled(true);
				mNativeMode.setEnabled(mNativeModeSelect.isChecked());
				if(mediaBox) {
					int hdmi_resolution = SystemProperties.getInt("persist.sys.hdmi_resolution", 1);
					if( hdmi_resolution == 6 || hdmi_resolution == 7 || hdmi_resolution == 8)
						mHDMIAspectRatio.setEnabled(true);
					else
						mHDMIAspectRatio.setEnabled(false);

					mHDMIColorSpace.setEnabled(true);

					if(mHDMI_color_depth_enable)
						mHDMIColorDepth.setEnabled(true);
					else
						mHDMIColorDepth.setEnabled(false);
				}
			}
		} else {
			switch (outputMode) {
			case 0:
				mHDMIResolution.setEnabled(false);
				mHDMIMode.setEnabled(false);
				mCompositeMode.setEnabled(false);
				mComponentResolution.setEnabled(false);
				mNativeModeSelect.setEnabled(false);
				mNativeMode.setEnabled(false);
				if(mediaBox) {
					mHDMIAspectRatio.setEnabled(false);
					mHDMIColorSpace.setEnabled(false);
					mHDMIColorDepth.setEnabled(false);
				}
				break;

			case 1:
				mHDMIResolution.setEnabled(true);
				mHDMIMode.setEnabled(true);
				mCompositeMode.setEnabled(false);
				mComponentResolution.setEnabled(false);
				mHDMICECSelect.setEnabled(true);
				mNativeModeSelect.setEnabled(true);
				mNativeMode.setEnabled(mNativeModeSelect.isChecked());
				if(mediaBox) {
					int hdmi_resolution = SystemProperties.getInt("persist.sys.hdmi_resolution", 1);
					if( hdmi_resolution == 6 || hdmi_resolution == 7 || hdmi_resolution == 8)
						mHDMIAspectRatio.setEnabled(true);
					else
						mHDMIAspectRatio.setEnabled(false);

					mHDMIColorSpace.setEnabled(true);

					if(mHDMI_color_depth_enable)
						mHDMIColorDepth.setEnabled(true);
					else
						mHDMIColorDepth.setEnabled(false);
				}
				break;

			case 2:
				mHDMIResolution.setEnabled(false);
				mHDMIMode.setEnabled(false);
				mCompositeMode.setEnabled(true);
				mComponentResolution.setEnabled(false);
				mHDMICECSelect.setChecked(false);
				mHDMICECSelect.setEnabled(false);
				mNativeModeSelect.setEnabled(false);
				mNativeMode.setEnabled(false);
				if(mediaBox) {
					mHDMIAspectRatio.setEnabled(false);
					mHDMIColorSpace.setEnabled(false);
					mHDMIColorDepth.setEnabled(false);
				}
				break;

			case 3:
				mHDMIResolution.setEnabled(false);
				mHDMIMode.setEnabled(false);
				mCompositeMode.setEnabled(false);
				mComponentResolution.setEnabled(true);
				mHDMICECSelect.setChecked(false);
				mHDMICECSelect.setEnabled(false);
				mNativeModeSelect.setEnabled(false);
				mNativeMode.setEnabled(false);
				if(mediaBox) {
					mHDMIAspectRatio.setEnabled(false);
					mHDMIColorSpace.setEnabled(false);
					mHDMIColorDepth.setEnabled(false);
				}
				break;
			}
		}
    }

    private void updateState(boolean force) {
        updateOutputModeSummary(mOutputMode.getValue());
        updateHDMIResolutionSummary(mHDMIResolution.getValue());
        updateHDMIModeSummary(mHDMIMode.getValue());
    updateCompositeModeSummary(mCompositeMode.getValue());
    updateComponentResolutionSummary(mComponentResolution.getValue());
        updateHDMIAspectRatioSummary(mHDMIAspectRatio.getValue());
        updateHDMIColorSpaceSummary(mHDMIColorSpace.getValue());
        updateHDMIColorDepthSummary(mHDMIColorDepth.getValue());
    }

    private void updateOutputModeSummary(Object value) {
        CharSequence[] summaries = getResources().getTextArray(R.array.output_mode_summaries);
        CharSequence[] values = mOutputMode.getEntryValues();

        Log.e(LOG_TAG, "updateOutputModeSummary :" + value + " length:"+ values.length);

        mOutputMode.setSummary(summaries[Integer.parseInt((String)value)]);
    }

    private void updateHDMIResolutionSummary(Object value) {

	int dvi_menu_enable = 0;
	int hdmi_mode = SystemProperties.getInt("persist.sys.hdmi_mode", 1);
	int hdmi_detected_mode = SystemProperties.getInt("persist.sys.hdmi_detected_mode", 1);
	boolean mediaBox = SystemProperties.get("tcc.solution.mbox","0").equals("1");

	String[] DVI_Entries = getResources().getStringArray(R.array.dvi_resolution_entries);
	String[] DVI_values = getResources().getStringArray(R.array.dvi_resolution_values);
 	
    CharSequence[] HDMI_Summaries = getResources().getTextArray(R.array.hdmi_resolution_summaries);
    CharSequence[] values = getResources().getTextArray(R.array.hdmi_resolution_values);

	switch(hdmi_mode) {
		//Setting menu is Auto
		case 125:
		{
			if(hdmi_detected_mode == 0) {
				//HDMI detected!!!
				dvi_menu_enable = 0;
			} else if(hdmi_detected_mode == 1)  {
				//DVI detected!!!
				dvi_menu_enable = 1;
			} else {
				//Default HDMI
				dvi_menu_enable = 0;
			}
			break;
		}
		//Setting menu is HDMI
		case 0:
		{
			dvi_menu_enable = 0;
			break;
		}
		//Setting menu is DVI
		case 1:
		{
	 		dvi_menu_enable = 1;
			break;
		}
		//default is HDMI
		default:
		{
			dvi_menu_enable = 0;
			break;
		}
	}

	if(dvi_menu_enable == 1){
		HDMI_Summaries = getResources().getTextArray(R.array.dvi_resolution_entries);
		values = getResources().getTextArray(R.array.dvi_resolution_values);
		mHDMIResolution.setEntries(DVI_Entries);
		mHDMIResolution.setEntryValues(DVI_values);
	}

    Log.v(LOG_TAG, "updateHDMIResolutionSummary value_" + value );

    for (int i = 0; i < values.length; i++) {
            if (values[i].equals(value)) {
        if(values[i].equals("125")){
            int j = SystemProperties.getInt("persist.sys.hdmi_detected_res", 1) + 1;
            mHDMIResolution.setSummary(HDMI_Summaries[i]+ " [" + HDMI_Summaries[j] + "]");
        } else{
            mHDMIResolution.setSummary(HDMI_Summaries[i]);
        }

        break;
            }
        }

	if(mediaBox) {
		int hdmi_resolution = SystemProperties.getInt("persist.sys.hdmi_resolution", 1);
		if( hdmi_resolution == 6 || hdmi_resolution == 7 || hdmi_resolution == 8)
			mHDMIAspectRatio.setEnabled(true);
		else
			mHDMIAspectRatio.setEnabled(false);
		}

    }

    private void updateHDMIModeSummary(Object value) {
        CharSequence[] summaries = getResources().getTextArray(R.array.hdmi_mode_summaries);
        CharSequence[] values = mHDMIMode.getEntryValues();
        for (int i = 0; i < values.length; i++) {
            if (values[i].equals(value)) {
                mHDMIMode.setSummary(summaries[i]);
                break;
            }
        }
    }

    private void updateHDMIAspectRatioSummary(Object value) {
        CharSequence[] summaries = getResources().getTextArray(R.array.hdmi_aspect_ratio_summaries);
        CharSequence[] values = mHDMIAspectRatio.getEntryValues();
        for (int i = 0; i < values.length; i++) {
            if (values[i].equals(value)) {
                mHDMIAspectRatio.setSummary(summaries[i]);
                break;
            }
        }
    }

    private void updateHDMIColorSpaceSummary(Object value) {
        CharSequence[] summaries = getResources().getTextArray(R.array.hdmi_color_space_summaries);
        CharSequence[] values = mHDMIColorSpace.getEntryValues();
        for (int i = 0; i < values.length; i++) {
            if (values[i].equals(value)) {
                mHDMIColorSpace.setSummary(summaries[i]);
                break;
            }
        }
    }

    private void updateHDMIColorDepthSummary(Object value) {
        CharSequence[] summaries = getResources().getTextArray(R.array.hdmi_color_depth_summaries);
        CharSequence[] values = mHDMIColorDepth.getEntryValues();
        for (int i = 0; i < values.length; i++) {
            if (values[i].equals(value)) {
                mHDMIColorDepth.setSummary(summaries[i]);
                break;
            }
        }
    }

    private void updateCompositeModeSummary(Object value) {
        CharSequence[] summaries = getResources().getTextArray(R.array.composite_mode_summaries);
        CharSequence[] values = mCompositeMode.getEntryValues();
        for (int i = 0; i < values.length; i++) {
            if (values[i].equals(value)) {
                mCompositeMode.setSummary(summaries[i]);
                break;
            }
        }
    }

    private void updateComponentResolutionSummary(Object value) {
        CharSequence[] summaries = getResources().getTextArray(R.array.component_resolution_summaries);
        CharSequence[] values = mComponentResolution.getEntryValues();
        for (int i = 0; i < values.length; i++) {
            if (values[i].equals(value)) {
                mComponentResolution.setSummary(summaries[i]);
                break;
            }
        }
    }

    private void createAlertDialogForOutput() {
        boolean mediaBox = SystemProperties.get("tcc.solution.mbox", "0").equals("1");
        if(mediaBox)
        {
            alert_confirm = new AlertDialog.Builder(this.getActivity())
                .setTitle("Do you want to change output setting?")
                .setMessage("This setting will be reset after " + alert_count + " seconds")
                .setNegativeButton("CANCEL", new DialogInterface.OnClickListener()
                {
                    @Override
                    public void onClick(DialogInterface OutputDialog, int which)
                    {
                        OutputDialog.dismiss();
                        handler.removeMessages(COUNT_UPDATE);
                        alert_count = 13;

						//Ouptut mode(HDMI/Composite/Component)
                        if(alert_confirm_type == 0){
                            Settings.System.putInt(getContentResolver(), OUTPUT_MODE, alert_confirm_value);
                            mOutputMode.setValue(Settings.System.getString(getContentResolver(), OUTPUT_MODE));
                        }
						// HDMI resolution
                        else if(alert_confirm_type == 1){
                            Settings.System.putInt(getContentResolver(), HDMI_RESOLUTION, alert_confirm_value);
                            mHDMIResolution.setValue(Settings.System.getString(getContentResolver(), HDMI_RESOLUTION));
                        }
						//HDMI mode(Auto/HDMI/DVI)
                        else if(alert_confirm_type == 2){
                            Settings.System.putInt(getContentResolver(), HDMI_MODE, alert_confirm_value);
                            mHDMIMode.setValue(Settings.System.getString(getContentResolver(), HDMI_MODE));

							if("1".equals(mHDMIMode.getValue())){
								int resolution = Settings.System.getInt(getContentResolver(), HDMI_RESOLUTION, FALLBACK_HDMI_RESOLUTION_VALUE);
								if(resolution == 2 || resolution == 3){
									Settings.System.putInt(getContentResolver(), HDMI_RESOLUTION, 125);
									mHDMIResolution.setValue("125");
									updateHDMIResolutionSummary(mHDMIResolution.getValue());
								}

								String[] HDMIResolutionArray = getResources().getStringArray(R.array.dvi_resolution_entries);
								String[] HDMIvalues = getResources().getStringArray(R.array.dvi_resolution_values);

								mHDMIResolution.setEntries(	HDMIResolutionArray);
								mHDMIResolution.setEntryValues(HDMIvalues);

								mNativeModeSelect.setChecked(false);
								mNativeModeSelect.setEnabled(false);
								mNativeMode.setEnabled(false);
							} else if("0".equals(mHDMIMode.getValue())){
								String[] HDMIResolutionArray = getResources().getStringArray(R.array.hdmi_resolution_entries);
								String[] HDMIvalues = getResources().getStringArray(R.array.hdmi_resolution_values);

								mHDMIResolution.setEntries(HDMIResolutionArray);
								mHDMIResolution.setEntryValues(HDMIvalues);

								mNativeModeSelect.setEnabled(true);
							}
                        }
						//HDMI Aspect Ratio
						else if(alert_confirm_type == 3) {
                            Settings.System.putInt(getContentResolver(), HDMI_ASPECT_RATIO, alert_confirm_value);
                            mHDMIAspectRatio.setValue(Settings.System.getString(getContentResolver(), HDMI_ASPECT_RATIO));		
						}
						//HDMI Color Space
						else if(alert_confirm_type == 4) {
                            Settings.System.putInt(getContentResolver(), HDMI_COLOR_SPACE, alert_confirm_value);
                            mHDMIColorSpace.setValue(Settings.System.getString(getContentResolver(), HDMI_COLOR_SPACE));							
						}
						//HDMI Color Depth
						else if(alert_confirm_type == 5) {
                            Settings.System.putInt(getContentResolver(), HDMI_COLOR_DEPTH, alert_confirm_value);
                            mHDMIColorDepth.setValue(Settings.System.getString(getContentResolver(), HDMI_COLOR_DEPTH));							
						}
						//Composite Resoltuion(NTSC/PAL)
                        else if(alert_confirm_type == 6){
                            Settings.System.putInt(getContentResolver(), COMPOSITE_MODE, alert_confirm_value);
                            mCompositeMode.setValue(Settings.System.getString(getContentResolver(), COMPOSITE_MODE));
                        }
						//Component Resolution(720p/1080i)
                        else if(alert_confirm_type == 7){
                            Settings.System.putInt(getContentResolver(), COMPONENT_RESOLUTION, alert_confirm_value);
                            mComponentResolution.setValue(Settings.System.getString(getContentResolver(), COMPONENT_RESOLUTION));
                        }
                    }
                })
                .setPositiveButton("OK", new DialogInterface.OnClickListener()
                {
                    @Override
                    public void onClick(DialogInterface OutputDialog, int which)
                    {
                    	boolean mediaBox = SystemProperties.get("tcc.solution.mbox","0").equals("1");
                        OutputDialog.dismiss();
                        handler.removeMessages(COUNT_UPDATE);
                        alert_count = 13;

						//Ouptut mode(HDMI/Composite/Component)
                        if(alert_confirm_type == 0){
                            if(SystemProperties.getInt("persist.sys.display.mode",1) == 3){
                                if(SystemProperties.getInt("persist.sys.output_mode",1) == 2)
                                    mOutputMode.setValue("2");
                                else if(SystemProperties.getInt("persist.sys.hdmi_detected",1) == 1)
                                    mOutputMode.setValue("1");
                                else if(SystemProperties.getInt("persist.sys.hdmi_detected",1) == 0)
                                    mOutputMode.setValue("3");

                                updateOutputModeSummary(mOutputMode.getValue());
                                disableUnusableItems(Integer.parseInt(mOutputMode.getValue()));
                            } else{
                                updateOutputModeSummary(mOutputMode.getValue());
                                disableUnusableItems(Settings.System.getInt(getContentResolver(), OUTPUT_MODE, FALLBACK_OUTPUT_MODE_VALUE));
                            }
                        }
						// HDMI resolution
                        else if(alert_confirm_type == 1){
                            updateHDMIResolutionSummary(mHDMIResolution.getValue());
                        }
						//HDMI mode(Auto/HDMI/DVI)
                        else if(alert_confirm_type == 2){
							if("1".equals(mHDMIMode.getValue())){
								int resolution = Settings.System.getInt(getContentResolver(), HDMI_RESOLUTION, FALLBACK_HDMI_RESOLUTION_VALUE);
								if(resolution == 2 || resolution == 3){
									Settings.System.putInt(getContentResolver(), HDMI_RESOLUTION, 125);
									mHDMIResolution.setValue("125");
									updateHDMIResolutionSummary(mHDMIResolution.getValue());
								}

								String[] HDMIResolutionArray = getResources().getStringArray(R.array.dvi_resolution_entries);
								String[] HDMIvalues = getResources().getStringArray(R.array.dvi_resolution_values);

								mHDMIResolution.setEntries(	HDMIResolutionArray);
								mHDMIResolution.setEntryValues(HDMIvalues);

								mNativeModeSelect.setChecked(false);
								mNativeModeSelect.setEnabled(false);
								mNativeMode.setEnabled(false);
							} else if("0".equals(mHDMIMode.getValue())){
								String[] HDMIResolutionArray = getResources().getStringArray(R.array.hdmi_resolution_entries);
								String[] HDMIvalues = getResources().getStringArray(R.array.hdmi_resolution_values);

								mHDMIResolution.setEntries(HDMIResolutionArray);
								mHDMIResolution.setEntryValues(HDMIvalues);

								mNativeModeSelect.setEnabled(true);
							}
                            updateHDMIModeSummary(mHDMIMode.getValue());
                        }
						//HDMI Aspect Ratio
						else if(alert_confirm_type == 3) {
                            updateHDMIAspectRatioSummary(mHDMIAspectRatio.getValue());
						}
						//HDMI Color Space
						else if(alert_confirm_type == 4) {
                            updateHDMIColorSpaceSummary(mHDMIColorSpace.getValue());
						}
						//HDMI Color Depth
						else if(alert_confirm_type == 5) {
                            updateHDMIColorDepthSummary(mHDMIColorDepth.getValue());
						}
						//Composite Resoltuion(NTSC/PAL)
                        else if(alert_confirm_type == 6){
                            updateCompositeModeSummary(mCompositeMode.getValue());
                        }
						//Component Resolution(720p/1080i)
                        else if(alert_confirm_type == 7){
                            updateComponentResolutionSummary(mComponentResolution.getValue());
                        }
                    }
                })
                .setCancelable(false)
                .show();

           handler.sendEmptyMessageDelayed(COUNT_UPDATE,0);
        }
    }

    public boolean onPreferenceChange(Preference preference, Object objValue) {
        final String key = preference.getKey();
    boolean mBox = SystemProperties.get("tcc.solution.mbox", "0").equals("1");

        if (KEY_OUTPUT_MODE.equals(key)) {
           int value = Integer.parseInt((String) objValue);
           try {
		        alert_confirm_type = 0;
		        alert_confirm_value = Settings.System.getInt(getContentResolver(), OUTPUT_MODE, FALLBACK_OUTPUT_MODE_VALUE);
                Settings.System.putInt(getContentResolver(), OUTPUT_MODE, value);
        if(!mBox){
            disableUnusableItems(value);
            updateOutputModeSummary(objValue);
        }

        if(alert_confirm_value != value)
            createAlertDialogForOutput();
            } catch (NumberFormatException e) {
                Log.e(LOG_TAG, "could not persist output mode setting");
            }
        }
        if (KEY_HDMI_RESOLUTION.equals(key)) {
           int value = Integer.parseInt((String) objValue);
		       try {
		        alert_confirm_type = 1;
		        alert_confirm_value = Settings.System.getInt(getContentResolver(), HDMI_RESOLUTION, FALLBACK_HDMI_RESOLUTION_VALUE);
                Settings.System.putInt(getContentResolver(), HDMI_RESOLUTION, value);
                if(!mBox)
            updateHDMIResolutionSummary(objValue);

        if(alert_confirm_value != value)
            createAlertDialogForOutput();
            } catch (NumberFormatException e) {
                Log.e(LOG_TAG, "could not persist HDMI resolution setting");
            }
        }

        if (KEY_HDMI_MODE.equals(key)) {
            int value = Integer.parseInt((String) objValue);
            try {
		        alert_confirm_type = 2;
		        alert_confirm_value = Settings.System.getInt(getContentResolver(), HDMI_MODE, FALLBACK_HDMI_MODE_VALUE);
                Settings.System.putInt(getContentResolver(), HDMI_MODE, value);
                if(!mBox)
            updateHDMIModeSummary(objValue);

        if(alert_confirm_value != value)
            createAlertDialogForOutput();
            } catch (NumberFormatException e) {
                Log.e(LOG_TAG, "could not persist HDMI mode setting");
            }
        }

        if (KEY_HDMI_ASPECT_RATIO.equals(key)) {
           int value = Integer.parseInt((String) objValue);

	       try {
				alert_confirm_type = 3;
				alert_confirm_value = Settings.System.getInt(getContentResolver(), HDMI_ASPECT_RATIO, FALLBACK_HDMI_ASPECT_RATIO_VALUE);
				Settings.System.putInt(getContentResolver(), HDMI_ASPECT_RATIO, value);
				if(!mBox)
					updateHDMIAspectRatioSummary(objValue);

				if(alert_confirm_value != value)
					createAlertDialogForOutput();
            } catch (NumberFormatException e) {
                Log.e(LOG_TAG, "could not persist HDMI Aspect Ratio setting");
            }
        }

        if (KEY_HDMI_COLOR_SPACE.equals(key)) {
           int value = Integer.parseInt((String) objValue);

	       try {
				alert_confirm_type = 4;
				alert_confirm_value = Settings.System.getInt(getContentResolver(), HDMI_COLOR_SPACE, FALLBACK_HDMI_COLOR_SPACE_VALUE);
				Settings.System.putInt(getContentResolver(), HDMI_COLOR_SPACE, value);
				if(!mBox)
					updateHDMIColorSpaceSummary(objValue);

				if(alert_confirm_value != value)
					createAlertDialogForOutput();
            } catch (NumberFormatException e) {
                Log.e(LOG_TAG, "could not persist HDMI Color Space setting");
            }
        }

        if (KEY_HDMI_COLOR_DEPTH.equals(key)) {
           int value = Integer.parseInt((String) objValue);

	       try {
				alert_confirm_type = 5;
				alert_confirm_value = Settings.System.getInt(getContentResolver(), HDMI_COLOR_DEPTH, FALLBACK_HDMI_COLOR_DEPTH_VALUE);
				Settings.System.putInt(getContentResolver(), HDMI_COLOR_DEPTH, value);
				if(!mBox)
					updateHDMIColorDepthSummary(objValue);

				if(alert_confirm_value != value)
					createAlertDialogForOutput();
            } catch (NumberFormatException e) {
                Log.e(LOG_TAG, "could not persist HDMI Color Depth setting");
            }
        }

        if (KEY_COMPOSITE_MODE.equals(key)) {
            int value = Integer.parseInt((String) objValue);
            try {
        alert_confirm_type = 6;
        alert_confirm_value = Settings.System.getInt(getContentResolver(), COMPOSITE_MODE, FALLBACK_COMPOSITE_MODE_VALUE);
                Settings.System.putInt(getContentResolver(), COMPOSITE_MODE, value);
                if(!mBox)
            updateCompositeModeSummary(objValue);

        if(alert_confirm_value != value)
            createAlertDialogForOutput();
            } catch (NumberFormatException e) {
                Log.e(LOG_TAG, "could not persist composite video mode setting");
            }
        }
        if (KEY_COMPONENT_RESOLUTION.equals(key)) {
            int value = Integer.parseInt((String) objValue);
            try {
        alert_confirm_type = 7;
        alert_confirm_value = Settings.System.getInt(getContentResolver(), COMPONENT_RESOLUTION, FALLBACK_COMPONENT_RESOLUTION_VALUE);
                Settings.System.putInt(getContentResolver(), COMPONENT_RESOLUTION, value);
                if(SystemProperties.getInt("persist.sys.component_mode", 1) == 0)
                    SystemProperties.set("persist.sys.component_resize", Integer.toString(3));
                else
                    SystemProperties.set("persist.sys.component_resize", Integer.toString(0));

                if(!mBox)
            updateComponentResolutionSummary(objValue);

        if(alert_confirm_value != value)
            createAlertDialogForOutput();
            } catch (NumberFormatException e) {
                Log.e(LOG_TAG, "could not persist component resolution setting");
            }
        }

        if (KEY_NATIVE_MODE_SETTING.equals(key)) {
            Set<String> modeValues = (Set<String>)objValue;
            CharSequence[] entries = mNativeMode.getEntryValues();
            int value =0;

            //Log.i(LOG_TAG, "modeValues " +modeValues);

            try {
                for (int i = 0; i < entries.length; i++) {
                    if(modeValues.contains(entries[i].toString())){
                        value += Integer.parseInt(entries[i].toString());

                        //Log.i(LOG_TAG, " entries.string : " +entries[i].toString() + " value : " + value);
                    }
                }
                Settings.System.putInt(getContentResolver(), NATIVE_MODE, value);

            } catch (NumberFormatException e) {
                Log.e(LOG_TAG, "could not persist component resolution setting");
            }

        }

        return true;
    }

    private void handleOutputModeChanged(int OutputMode, int previousOutputMode) {

    int mode =0;

    if(mDM == null){
        return;
    }

    mode = mDM.getOutputMode();

    Log.d(LOG_TAG, "handleOutputModeChanged OutputMode = "
            + OutputMode + " previousOutputMode = "
            + previousOutputMode + " current mode = " +mode);

    //update menu
    mOutputMode.setValue(String.valueOf(OutputMode));
    disableUnusableItems(OutputMode);
    updateOutputModeSummary(mOutputMode.getValue());
    }

    private void handleOutputResolutionChanged(int OutputResolution, int previousOutputResolution) {

    int resolution =0;

    if(mDM == null){
         return;
    }

    resolution = mDM.getOutputResolution();

    Log.d(LOG_TAG, "handleOutputResolutionChanged OutputResolution = "
            + OutputResolution + " previousOutputResolution = "
            + previousOutputResolution + " current resolution = " +resolution);

    //update menu
    mHDMIResolution.setValue(String.valueOf(OutputResolution));
    updateHDMIResolutionSummary(mHDMIResolution.getValue());
    }

    private void handleHDMIModeChanged(int HDMIMode, int previousHDMIMode) {

    int mode = 0;

    if(mDM == null){
         return;
    }

    mode = mDM.getHDMIMode();

    Log.d(LOG_TAG, "handleHDMIModeChanged HDMIMode = "
            + HDMIMode + " previousHDMIMode = "
            + previousHDMIMode + " currentHDMIMode = " + mode);

    //update menu
    mHDMIMode.setValue(String.valueOf(HDMIMode));
    updateHDMIModeSummary(mHDMIMode.getValue());
    }
}
