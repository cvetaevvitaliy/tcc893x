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
package com.telechips.android.dvb.option;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.os.Bundle;
import android.os.Handler;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnKeyListener;
import android.view.inputmethod.InputMethodManager;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.content.pm.ActivityInfo;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.database.Cursor;
import android.text.Editable;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.ArrayList;

import com.telechips.android.dvb.DVBPlayerActivity;
import com.telechips.android.dvb.DxbView;
import com.telechips.android.dvb.R;
import com.telechips.android.dvb.player.DxbPlayer;
import com.telechips.android.dvb.player.DxbPVR.LOCAL_STATE;
import com.telechips.android.dvb.schedule.Alert;
import com.telechips.android.dvb.util.DxbStorage;
import com.telechips.android.dvb.util.DxbUtil;

public class DxbOptions extends PreferenceActivity implements Preference.OnPreferenceChangeListener, Preference.OnPreferenceClickListener
{
	private static final String TAG = "[[[DxbOptions]]] ";

	private static Context mContext;

	public static final String KEY_LANGUAGE_SUBTITLE_SELECT	= "language_subtitle_select";
	public static final String KEY_LANGUAGE_AUDIO_SELECT	= "language_audio_select";
	public static final String KEY_AUDIO_DESCRIPTION		= "key_audio_description";
	public static final String KEY_AUDIO_MODE				= "key_audio_mode";
	public static final String KEY_COUNTRYCODE_SELECT		= "countrycode_select";
	public static final String KEY_SCAN_AUTO				= "key_auto_search";
	public static final String KEY_SCAN_MANUAL_SELECT		= "key_manual_search";
//	public static final String KEY_TIME_ZONE_SELECT			= "time_zone_select";
	public static final String KEY_STORAGE_SELECT			= "storage_select";

	private ListPreference mLanguage_SubtitleSelect;
	private ListPreference mLanguage_AudioSelect;
	private PreferenceScreen	mAudioDescriptionSelect;
	private ListPreference mAudioMode_Select;
	private ListPreference mCountryCodeSelect;
	private PreferenceScreen mAutoSearchSelect;
	private PreferenceScreen mScan_ManualSelect;
	private PreferenceScreen mDishSetupSelect;
	private ListPreference	mDBManagementSelect;
	private PreferenceScreen mUnicableConfigSelect;
	
	private ListPreference mTimeZoneSelect;
	private EditTextPreference	mParentalRatingSelect;
	private EditTextPreference	mChangePasswordSelect;
	private ListPreference mAntennaPowerSelect;
	private ListPreference mStorageSelect;
	
	/*	dialog value	*/
		Builder	mBuilder;
		View textEntryView;
		static Dialog	mDialog;

	private int	iCount_Language_Audio;
	private Cursor	audio_cursor	= null;

	public static boolean isbackpressed;
	public static boolean isuserleavehint;
	private boolean isSchedulerStart;
	
	public Intent intent_dish_setup;

	private Handler mHandler_Main = new Handler();

	public static DxbOptions getInstance() {
		return (DxbOptions)mContext;
	}

	private DVBPlayerActivity getMainActivity() {
		return DVBPlayerActivity.getInstance();
	}

	private DxbPlayer getPlayer() {
		return getMainActivity().getPlayer();
	}

	private class Default_Config_Thread extends Thread {
		public Default_Config_Thread(Runnable runnable) {
			super(runnable);
		}
		
		@Override
		public void run() {
			super.run();
			mHandler_Main.post(mDefault_Config_Update);
		}
	}

	private class Default_Config_Runnable implements Runnable {
		public void run() {
			Log.i(TAG, "Default_Config_Runnable()");
			setDefault_Value();
		}
	}

    private Runnable mDefault_Config_Runnable = new Runnable() {
        public void run() {
			Default_Config_Runnable defaultRunnable = new Default_Config_Runnable();
			Default_Config_Thread defaultThread = new Default_Config_Thread(defaultRunnable);
			defaultThread.start();
        }
    };

    private Runnable mDefault_Config_Update = new Runnable() {
        public void run() {
			Log.i(TAG, "mDefault_Config_Update()");
			updateState(true);
        }
    };

	@SuppressWarnings("deprecation")
	@Override
	public void onCreate(Bundle saveInstanceState)
	{
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onCreate()");
		
		super.onCreate(saveInstanceState);

		getPlayer().isDxbOptionActivityON = true;
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
		
		if(getPlayer().isDVB_S2())
		{
			addPreferencesFromResource(R.xml.options_dvbs);
		}
		else
		{
			addPreferencesFromResource(R.xml.options);
		}

		if(getPlayer().getLocalPlayState() != LOCAL_STATE.STOP)
		{
			getPreferenceScreen().removePreference(findPreference("scan_category"));
		}

		getListView().setBackgroundColor(Color.rgb(16, 16, 16));
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		isbackpressed = false;
		isuserleavehint	= false;
		isSchedulerStart = false;
		mContext = this;

		initIntentFilter();
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

		getPlayer().isDxbOptionActivityON = true;
		mHandler_Main.post(mDefault_Config_Runnable);
	}

	@Override
	protected void onPause()
	{
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onPause()");
		super.onPause();
		
		if(		(getMainActivity().intentOptionActivity != null)
			&&	(getPlayer().isDishSetupActivityOn == false)
			&&	(isbackpressed == false)
			&&	(isSchedulerStart == false)
		)
		{
			if(getPlayer().isValid())
			{
				getPlayer().releaseSurface();
				getPlayer().stop();
				getPlayer().release();
			}
			
			if(getPlayer().isValid() == false)
			{
				getPlayer().isOFF = true;
			}
			if(!isuserleavehint)
			{
				getPlayer().isFINISH	= true;
			}

			this.finish();
		}
	}

	@Override
	public void onStop()
	{
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onStop()");
		getPlayer().isDxbOptionActivityON = false;
		super.onStop();
		this.finish();
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onStop() finish");
	}

	@Override
	public void onDestroy()
	{
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onDestroy()");
		getPlayer().isDxbOptionActivityON = false;

		super.onDestroy();
		
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onDestroy() finish");
		unregisterReceiver(mReceiverIntent);
		unregisterReceiver(mBroadcastReceiver);
	}

	@Override
	public boolean dispatchKeyEvent(KeyEvent event)
	{
		//Log.i(TAG, "dispatchKeyEvent(event:" + event + ")");
		
		int keyCode = event.getKeyCode();
		//Log.d(TAG, "dispatchKeyEvent(keyCode:" + keyCode + ")");
		switch (keyCode)
		{
			case KeyEvent.KEYCODE_ENTER:
				mParentalRatingSelect.setText(null);
				mChangePasswordSelect.setText(null);
			break;
				    	
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
	public void onBackPressed()
	{
		Log.d("BACK KEY", "onBackPressed Called");
		isbackpressed = true;
		super.onBackPressed();  
	}
	
	@Override
	public void onUserLeaveHint()
	{
		Log.d("HOME KEY", "onUserLeaveHint Called");
		
		isuserleavehint	= true;
		
		super.onUserLeaveHint();
	}
	
	private void updateState(boolean force)
	{
		Log.i(TAG, "updateState()");
		
		CharSequence	Entries[];
		
		Entries	= mLanguage_SubtitleSelect.getEntries();
		mLanguage_SubtitleSelect.setSummary(Entries[getPlayer().cOption.language_subtitle]);

		Entries	= mLanguage_AudioSelect.getEntries();
		mLanguage_AudioSelect.setSummary(Entries[getPlayer().cOption.language_audio]);
		
		if (mAudioMode_Select != null)
		{
			mAudioMode_Select.setValueIndex(getPlayer().cOption.audio_mode);
			updateAudioModeSummary(mAudioMode_Select.getValue());
		}

		updateLanguage_AudioDescriptionSummary();

		if (mCountryCodeSelect != null)
		{
			mCountryCodeSelect.setValueIndex(getPlayer().cOption.countrycode);
			updateCountryCodeSelectSummary(mCountryCodeSelect.getValue());
		}
		
		if(mTimeZoneSelect != null)
			updateTimeZoneSelectSummary(mTimeZoneSelect.getValue());

		updateParentalRatingSelectSummary(getPlayer().cOption.age);
		
		if(mAntennaPowerSelect != null)
			updateAntennaPowerSelectSummary(mAntennaPowerSelect.getValue());
		
		updateStorageSelectSummary();
	}
	
	private void updateLanguage_SubtitleSelectSummary(Object value)
	{
		Log.i(TAG, "updateLanguage_SubtitleSelectSummary()");
		
		CharSequence	Entries[]	= mLanguage_SubtitleSelect.getEntries();
		mLanguage_SubtitleSelect.setSummary(Entries[Integer.parseInt((String)value)]);
	}
	
	private void updateLanguage_AudioSelectSummary(Object value)
	{
		Log.i(TAG, "updateLanguage_AudioSelectSummary()");
		
		if(iCount_Language_Audio <= 0)
		{
			mLanguage_AudioSelect.setSummary(R.string.no_information);
		}
		else
		{
			CharSequence	Entries[]	= mLanguage_AudioSelect.getEntries();
			mLanguage_AudioSelect.setSummary(Entries[Integer.parseInt((String)value)]);
		}
	}

	private void updateLanguage_AudioDescriptionSummary()
	{
		Log.i(TAG, "updateLanguage_AudioDescriptionSummary()");

		if(mAudioDescriptionSelect != null)
		{
			mAudioDescriptionSelect.setSummary(getResources().getString(R.string.relative_vol) + (Integer.toString(getPlayer().cOption.relative_vol*5)));
		}
	}
	
	private void updateAudioModeSummary(Object value)
	{
		Log.i(TAG, "updateAudio_ModeSummary()");

		CharSequence[] summaries = getResources().getTextArray(R.array.audio_mode_summaries);
		CharSequence values[] = mAudioMode_Select.getEntryValues();
		
		for(int i=0 ; i<values.length ; i++)
		{
			if(values[i].equals(value))
			{
				mAudioMode_Select.setSummary(summaries[i]);
				break;
			}
		}
	}

	private void updateCountryCodeSelectSummary(Object value)
	{
		Log.i(TAG, "updateCountryCodeSelectSummary()");
		
		CharSequence[] summaries = getResources().getTextArray(R.array.countrycode_select_summaries);
		CharSequence[] values = mCountryCodeSelect.getEntryValues();
		for(int i=0 ; i<values.length ; i++)
		{
			if(values[i].equals(value))
			{
				mCountryCodeSelect.setSummary(summaries[i]);
				
				break;
			}
		}
	}
	
	private void updateTimeZoneSelectSummary(Object value)
	{
		Log.i(TAG, "updateTimeZoneSelectSummary(value = " + value + ")");
		
		CharSequence[] summaries	= getResources().getTextArray(R.array.time_zone_select_entries);
		CharSequence[] values	= mTimeZoneSelect.getEntryValues();
		for(int i=0 ; i<values.length ; i++)
		{
			if(values[i].equals(value))
			{
				Log.d(TAG, "summaries[i] = " + summaries[i]);
				
				if(i == 0)
				{
					mTimeZoneSelect.setSummary(summaries[i]);
				}
				else if(i == 1)
				{
					CharSequence[] summaries_index	= getResources().getTextArray(R.array.time_zone_manual_select_entries);

					String	sSummaries	= summaries[i] + " - " + summaries_index[getPlayer().cOption.time_zone_index];
					mTimeZoneSelect.setSummary(sSummaries);
				}
				
				break;
			}
		}
	}
	
	private void updateParentalRatingSelectSummary(int value)
	{
		Log.i(TAG, "updateParentalRatingSelectSummary(value=" + value + ")");
		
		if(value >= 0)
		{
			CharSequence[] summaries = getResources().getTextArray(R.array.parental_rating_select_summaries);
			mParentalRatingSelect.setSummary(summaries[value]);
		}
	}
	
	private void updateAntennaPowerSelectSummary(Object value)
	{
		Log.i(TAG, "updateAntennaPowerSelectSummary(value = " + value + ")");
		
		CharSequence[] summaries	= getResources().getTextArray(R.array.antenna_power_select_entries);
		CharSequence[] values	= mAntennaPowerSelect.getEntryValues();
		for(int i=0 ; i<values.length ; i++)
		{
			if(values[i].equals(value))
			{
				mAntennaPowerSelect.setSummary(summaries[i]);
				
				break;
			}
		}
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

	public boolean onPreferenceClick(Preference preference)
	{
		final String key = preference.getKey();
		Log.i(TAG, "onPreferenceClick(key=" + key + ")");
		
		if(KEY_AUDIO_DESCRIPTION.equals(key))
		{
			audioDescription();
		}
		else if(KEY_SCAN_AUTO.equals(key))
		{
			confirmScan();
		}
		else if(KEY_SCAN_MANUAL_SELECT.equals(key))
		{
			manualScan();
		}
		else if(getResources().getString(R.string.key_dish_setup_select).equals(key))
		{
			Log.d(TAG, "R.string.key_unicable_config_select");
			getPlayer().isDishSetupActivityOn = true;
			
			Intent	intent_dish_setup	= new Intent();
			intent_dish_setup.setClass(this, DishSetupActivity.class);
			startActivity(intent_dish_setup);
		}
		else if(getResources().getString(R.string.key_unicable_config_select).equals(key))
		{
			
		}

		return false;
	}
	
	public boolean onPreferenceChange(Preference preference, Object objValue)
	{
		final String key = preference.getKey();
		int value;
		Log.i(TAG, "onPreferenceChange() - " + key);

		if(KEY_LANGUAGE_SUBTITLE_SELECT.equals(key))
		{
			value = Integer.parseInt((String) objValue);
			try {
				getPlayer().cOption.language_subtitle	= value;
				updateLanguage_SubtitleSelectSummary(objValue);
			} catch(NumberFormatException e) {
				//Log.e(TAG, "could not persist screen output select", e);
			}
		}
		else if(KEY_LANGUAGE_AUDIO_SELECT.equals(key))
		{
			value = Integer.parseInt((String) objValue);
			try
			{
				getPlayer().cOption.language_audio	= value;
				getPlayer().setAudio(value);
				updateLanguage_AudioSelectSummary(objValue);
			}
			catch(NumberFormatException e)
			{
				//Log.e(TAG, "could not persist screen output select", e);
			}
		}
		else if(KEY_AUDIO_MODE.equals(key))
		{
			value = Integer.parseInt((String) objValue);
			try
			{
				getPlayer().cOption.audio_mode = value;
				getPlayer().setStereo(value);
				updateAudioModeSummary(objValue);
			}
			catch(NumberFormatException e)
			{
			}
		}
		else if(KEY_COUNTRYCODE_SELECT.equals(key))
		{
			value = Integer.parseInt((String) objValue);
			try
			{
				//Log.d(TAG, "value = " + value);
				//Log.d(TAG, "gInfo_Option.countrycode = " + gInfo_Option.countrycode);
				if(getPlayer().cOption.countrycode != value)
				{
/*
					AlertDialog.Builder alert	= new AlertDialog.Builder(this);
					
					alert.setIcon(R.drawable.icon);
					alert.setTitle(R.string.countrycode);
					alert.setMessage(R.string.for_applying_a_country_code_the_application_will_be_closed);
					
					alert.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener()
					{
						public void onClick(DialogInterface dialog, int which)
						{
							getPlayer().isOFF	= true;
							
							finish();
						}
					});
					alert.setCancelable(false);
					
					alert.show();
*/
					getPlayer().cOption.countrycode = value;
					updateCountryCodeSelectSummary(objValue);
				}
			}
			catch(NumberFormatException e)
			{
				//Log.e(TAG, "could not persist screen output select", e);
			}
		}
		else if(getResources().getString(R.string.key_db_management_select).equals(key))
		{
			value = Integer.parseInt((String) objValue);
			try
			{
				File	sd	= new File(DxbStorage.SDCARD0_PATH);
				File	usb	= new File(DxbStorage.USB0_PATH);
				
				if(!usb.isDirectory() || !usb.canWrite())
				{
					update_message(getResources().getStringArray(R.array.db_management_select_entries)[value], getResources().getString(R.string.no_usb_device_insert));
				}
				else if(!sd.isDirectory() || !sd.canWrite())
				{
					update_message(getResources().getStringArray(R.array.db_management_select_entries)[value], getResources().getString(R.string.no_sd_device_insert));
				}
				else
				{
					copy_DBManagement(sd, usb, value);
				}

			}
			catch(NumberFormatException e)
			{
				//Log.e(TAG, "could not persist screen output select", e);
			}
		}
		else if(getResources().getString(R.string.key_time_zone_select).equals(key))
		{
			value = Integer.parseInt((String) objValue);
			try
			{
				if(value == 0)
				{
					getPlayer().cOption.time_zone_type	= value;
					getPlayer().setTimeZone(value, 0);
					updateTimeZoneSelectSummary(objValue);
				}
				else if(value == 1)
				{
					mBuilder	= new AlertDialog.Builder(this);
					
					mBuilder.setTitle(getResources().getString(R.string.time_zone_manual));
					mBuilder.setSingleChoiceItems(R.array.time_zone_manual_select_entries, getPlayer().cOption.time_zone_index, ListenerOnClick_selectTimeZone);					
					mBuilder.setPositiveButton(getResources().getString(R.string.cancel), ListenerOnClick_selectTimeZone_Cancel);
					mBuilder.setOnCancelListener(ListenerOnClick_selectTimeZone_Back);
					
					mDialog	= mBuilder.create();
					mDialog.show();
				}
			}
			catch(NumberFormatException e)
			{
				//Log.e(TAG, "could not persist screen output select", e);
			}
		}
		else if(getResources().getString(R.string.key_parental_rating_select).equals(key))
		{
			try
			{
				Editable	text	= mParentalRatingSelect.getEditText().getText();
				if(text.toString().equalsIgnoreCase(getPlayer().cOption.password))
				{
					mBuilder	= new AlertDialog.Builder(this);
					
					mBuilder.setTitle(getResources().getString(R.string.age));
					mBuilder.setSingleChoiceItems(	R.array.parental_rating_select_entries,
													getPlayer().cOption.age,
													new DialogInterface.OnClickListener()
													{
														public void onClick(DialogInterface dialog, int item)
														{
															dialog.dismiss();
															
															getPlayer().setParentalRate(item);
															updateParentalRatingSelectSummary(item);
														}
													}
												);
					
					mBuilder.setPositiveButton(getResources().getString(R.string.cancel), null);
					mDialog	= mBuilder.create();
					mDialog.show();
				}
				else
				{
					mBuilder	= new AlertDialog.Builder(this);
					mBuilder.setTitle(getResources().getString(R.string.parental_rating));
					mBuilder.setMessage(getResources().getString(R.string.please_enter_the_correct_password));

					mBuilder.setPositiveButton(	getResources().getString(R.string.ok), null);
					
					mBuilder.show();
				}
			}
			catch(NumberFormatException e)
			{
				//Log.e(TAG, "could not persist screen output select", e);
			}
		}
		else if(getResources().getString(R.string.key_change_password_select).equals(key))
		{
			try
			{
				Editable	text	= mChangePasswordSelect.getEditText().getText();
				if(text.toString().equalsIgnoreCase(getPlayer().cOption.password))
				{
					mDialog	= new Dialog(this);
					
					mDialog.setContentView(R.layout.option_change_password);
					mDialog.setTitle(getResources().getString(R.string.set_password));
					mDialog.setOnKeyListener(new DialogInterface.OnKeyListener()
					{
						public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event)
						{
							//Log.d(TAG, "mDialog.setOnKeyListener --> onKey(dialog = " + dialog);
							//Log.d(TAG, "keyCode = " + keyCode + ", event=" + event + ")" );
							if(		(event.getAction() == KeyEvent.ACTION_DOWN)
								&&	(		(keyCode == KeyEvent.KEYCODE_DPAD_DOWN)
										||	(keyCode == KeyEvent.KEYCODE_ENTER)
									)
								&&	!mDialog.findViewById(R.id.change_password_edit_new).isFocused()
								&&	mDialog.findViewById(R.id.change_password_edit_confirm).isFocused()
							)
							{
								EditText	editText	= (EditText)mDialog.findViewById(R.id.change_password_edit_confirm);
								InputMethodManager	imm	= (InputMethodManager)getSystemService(INPUT_METHOD_SERVICE);
								imm.hideSoftInputFromWindow(editText.getWindowToken(), 0);
							}
							else if(keyCode == KeyEvent.KEYCODE_DPAD_UP)
							{
								EditText	editText	= (EditText)mDialog.findViewById(R.id.change_password_edit_confirm);
								InputMethodManager	imm	= (InputMethodManager)getSystemService(INPUT_METHOD_SERVICE);
								imm.showSoftInput(editText, InputMethodManager.SHOW_FORCED);
							}
							
							return false;
						}
					});
					
					Button	bCancel	= (Button)mDialog.findViewById(R.id.change_password_cancel);
					bCancel.setOnClickListener(	new View.OnClickListener()
												{
													public void onClick(View v)
													{
														mDialog.dismiss();
													}
												});
					
					Button	bOk	= (Button)mDialog.findViewById(R.id.change_password_ok);
					bOk.setOnClickListener(	new View.OnClickListener()
											{
												public void onClick(View v)
												{
													EditText	etNewPassword	= (EditText)mDialog.findViewById(R.id.change_password_edit_new);
													EditText	etConfirmPassword	= (EditText)mDialog.findViewById(R.id.change_password_edit_confirm);
													
													String	sNewPassword	= etNewPassword.getText().toString();
													String	sConfirmPassword	= etConfirmPassword.getText().toString();
													
													Log.d(TAG, "sNewPassword = " + sNewPassword);
													Log.d(TAG, "sConfirmPassword = " + sConfirmPassword);
													
													if(sNewPassword.equalsIgnoreCase(sConfirmPassword))
													{
														mDialog.dismiss();
														
														getPlayer().setChangePassword(sNewPassword);
														
														update_message(getResources().getString(R.string.change_password), getResources().getString(R.string.your_password_has_been_changed));
													}
													else
													{
														update_message(getResources().getString(R.string.change_password), getResources().getString(R.string.please_enter_the_correct_password));
													}
												}
											});
					
					mDialog.show();
				}
				else
				{
					mBuilder	= new AlertDialog.Builder(this);
					mBuilder.setTitle(getResources().getString(R.string.change_password));
					mBuilder.setMessage(getResources().getString(R.string.please_enter_the_correct_password));

					mBuilder.setPositiveButton(	getResources().getString(R.string.ok), null);
					
					mBuilder.show();
				}
			}
			catch(NumberFormatException e)
			{
				//Log.e(TAG, "could not persist screen output select", e);
			}
		}
		else if(KEY_STORAGE_SELECT.equals(key))
		{
			try
			{
				DxbStorage.setStorage((String)objValue);
				updateStorageSelectSummary();
			}
			catch(NumberFormatException e)
			{
				//Log.e(TAG, "could not persist screen output select", e);
			}
		}
		
		return true;
	}
	
	void update_message(String sTitle, String sMSG)
	{
		Builder builder	= new AlertDialog.Builder(this);
		builder.setTitle(sTitle);
		builder.setMessage(sMSG);

		builder.setPositiveButton(	getResources().getString(R.string.ok), null);
		
		builder.show();
		
	}
	
	DialogInterface.OnClickListener ListenerOnClick_selectTimeZone	= new DialogInterface.OnClickListener()
	{
		public void onClick(DialogInterface dialog, int item)
		{
			Log.i(TAG, "ListenerOnClick_selectTimeZone --> onClick() --> getPlayer().cOption.time_zone_type=" + getPlayer().cOption.time_zone_type);

			dialog.dismiss();
			
			getPlayer().setTimeZone(1, item);
			updateTimeZoneSelectSummary(mTimeZoneSelect.getValue());
		}
	};
	
	DialogInterface.OnClickListener ListenerOnClick_selectTimeZone_Cancel	= new DialogInterface.OnClickListener()
	{
		public void onClick(DialogInterface dialog, int item)
		{
			Log.i(TAG, "ListenerOnClick_selectTimeZone_Cancel --> onClick() --> getPlayer().cOption.time_zone_type=" + getPlayer().cOption.time_zone_type);

			CharSequence[] values	= mTimeZoneSelect.getEntryValues();
			mTimeZoneSelect.setValue(values[getPlayer().cOption.time_zone_type].toString());
		}
	};
	
	DialogInterface.OnCancelListener ListenerOnClick_selectTimeZone_Back	= new DialogInterface.OnCancelListener()
	{
		public void onCancel(DialogInterface dialog)
		{
			Log.i(TAG, "ListenerOnClick_selectTimeZone_Back --> onClick() --> getPlayer().cOption.time_zone_type=" + getPlayer().cOption.time_zone_type);

			CharSequence[] values	= mTimeZoneSelect.getEntryValues();
			mTimeZoneSelect.setValue(values[getPlayer().cOption.time_zone_type].toString());
		}
	};
	
	@SuppressWarnings("deprecation")
	private void setDefault_Value()
	{
		Log.i(TAG, "setDefault_Value()");
		
		mLanguage_SubtitleSelect = (ListPreference)findPreference(KEY_LANGUAGE_SUBTITLE_SELECT);
		if (mLanguage_SubtitleSelect != null) {
			setLanguage_SubtitleValue();
			mLanguage_SubtitleSelect.setOnPreferenceChangeListener(this);
		}

		mLanguage_AudioSelect = (ListPreference)findPreference(KEY_LANGUAGE_AUDIO_SELECT);
		if (mLanguage_AudioSelect != null) {
			setLanguage_AudioValue();
			mLanguage_AudioSelect.setOnPreferenceChangeListener(this);
		}
		
		mAudioDescriptionSelect	= (PreferenceScreen)findPreference(KEY_AUDIO_DESCRIPTION);
		if(mAudioDescriptionSelect != null)
		{
			if(getPlayer().isSTB())
			{
				mAudioDescriptionSelect.setOnPreferenceClickListener(this);
			}
			else
			{
				mAudioDescriptionSelect.setEnabled(false);
			}
		}
		
		mAudioMode_Select = (ListPreference)findPreference(KEY_AUDIO_MODE);
		if(mAudioMode_Select != null) {
			setAudioModeValue();
			mAudioMode_Select.setOnPreferenceChangeListener(this);
		}
		
		mCountryCodeSelect = (ListPreference) findPreference(KEY_COUNTRYCODE_SELECT);
		if (mCountryCodeSelect != null) {
			setCountryCodeValue();
			mCountryCodeSelect.setOnPreferenceChangeListener(this);
		}

		mAutoSearchSelect	= (PreferenceScreen)findPreference(KEY_SCAN_AUTO);
		if (mAutoSearchSelect != null)
			mAutoSearchSelect.setOnPreferenceClickListener(this);

		mScan_ManualSelect	= (PreferenceScreen)findPreference(KEY_SCAN_MANUAL_SELECT);
		if (mScan_ManualSelect != null)
			mScan_ManualSelect.setOnPreferenceClickListener(this);
		
		mDishSetupSelect	= (PreferenceScreen)findPreference(getResources().getString(R.string.key_dish_setup_select));
		if(mDishSetupSelect != null)
			mDishSetupSelect.setOnPreferenceClickListener(this);
		
		mDBManagementSelect	= (ListPreference)findPreference(getResources().getString(R.string.key_db_management_select));
		if(mDBManagementSelect != null)
			mDBManagementSelect.setOnPreferenceChangeListener(this);
		
		mUnicableConfigSelect	= (PreferenceScreen)findPreference(getResources().getString(R.string.key_unicable_config_select));
		if(mUnicableConfigSelect != null)
			mUnicableConfigSelect.setOnPreferenceClickListener(this);
		
		mTimeZoneSelect	= (ListPreference)findPreference(getResources().getString(R.string.key_time_zone_select));
		if(mTimeZoneSelect != null)
		{
			setTimeZoneValue();
			mTimeZoneSelect.setOnPreferenceChangeListener(this);
		}
		
		mParentalRatingSelect	= (EditTextPreference)findPreference(getResources().getString(R.string.key_parental_rating_select));
		if(mParentalRatingSelect != null)
		{
			mParentalRatingSelect.setOnPreferenceChangeListener(this);
			mParentalRatingSelect.setText(null);
		}
		mParentalRatingSelect.getEditText().setOnKeyListener(new OnKeyListener()
		{
			public boolean onKey(View v, int keyCode, KeyEvent event)
			{
				//Log.i(TAG, "mParentalRatingSelect.getEditText().setOnKeyListener --> keyCode=" + keyCode + ", event=" + event);
				if(		(event.getAction() == KeyEvent.ACTION_DOWN)
					&&	(		(keyCode == KeyEvent.KEYCODE_DPAD_DOWN)
							||	(keyCode == KeyEvent.KEYCODE_ENTER)
						)
				)
				{
					EditText	editText	= mParentalRatingSelect.getEditText();
					InputMethodManager	imm	= (InputMethodManager)getSystemService(INPUT_METHOD_SERVICE);
					imm.hideSoftInputFromWindow(editText.getWindowToken(), 0);
				}
				else if(keyCode == KeyEvent.KEYCODE_DPAD_UP && event.getAction() == KeyEvent.ACTION_UP)
				{
					EditText	editText	= mParentalRatingSelect.getEditText();
					InputMethodManager	imm	= (InputMethodManager)getSystemService(INPUT_METHOD_SERVICE);
					imm.showSoftInput(editText, InputMethodManager.SHOW_FORCED);
				}
				
				return false;
			}
		});
		
		mChangePasswordSelect	= (EditTextPreference)findPreference(getResources().getString(R.string.key_change_password_select));
		if(mChangePasswordSelect != null)
		{
			mChangePasswordSelect.setOnPreferenceChangeListener(this);
			mChangePasswordSelect.setText(null);
		}
		mChangePasswordSelect.getEditText().setOnKeyListener(new OnKeyListener()
		{
			public boolean onKey(View v, int keyCode, KeyEvent event)
			{
				//Log.i(TAG, "mParentalRatingSelect.getEditText().setOnKeyListener --> keyCode=" + keyCode + ", event=" + event);
				if(		(event.getAction() == KeyEvent.ACTION_DOWN)
					&&	(		(keyCode == KeyEvent.KEYCODE_DPAD_DOWN)
							||	(keyCode == KeyEvent.KEYCODE_ENTER)
						)
				)
				{
					EditText	editText	= mChangePasswordSelect.getEditText();
					InputMethodManager	imm	= (InputMethodManager)getSystemService(INPUT_METHOD_SERVICE);
					imm.hideSoftInputFromWindow(editText.getWindowToken(), 0);
				}
				else if(keyCode == KeyEvent.KEYCODE_DPAD_UP && event.getAction() == KeyEvent.ACTION_UP)
				{
					EditText	editText	= mChangePasswordSelect.getEditText();
					InputMethodManager	imm	= (InputMethodManager)getSystemService(INPUT_METHOD_SERVICE);
					imm.showSoftInput(editText, InputMethodManager.SHOW_FORCED);
				}
				
				return false;
			}
		});

		mStorageSelect = (ListPreference) findPreference(KEY_STORAGE_SELECT);
		if (mStorageSelect != null) {
			setStorageValue();
			mStorageSelect.setOnPreferenceChangeListener(this);
		}
	}
	
	TextView	tvRelative_Vol	= null;
	Button		bRelative_Vol_left	= null;
	Button		bRelative_Vol_right	= null;
	private void audioDescription()
	{
		textEntryView	= View.inflate(getInstance(), R.layout.option_audio_description, null);
		mDialog	= new AlertDialog.Builder(getInstance())
						.setTitle(getResources().getString(R.string.audio_description))
						.setView(textEntryView)
						.setPositiveButton(R.string.done, new DialogInterface.OnClickListener()
							{
								public void onClick(DialogInterface dialog, int whichButton)
								{
									mDialog.dismiss();
								}
							})
						.setOnCancelListener(new DialogInterface.OnCancelListener()
							{
								public void onCancel(DialogInterface dialog)
								{
									mDialog.dismiss();
								}
							})
						.setOnKeyListener(new DialogInterface.OnKeyListener()
							{
								public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event)
								{
									if(event.getAction() == KeyEvent.ACTION_DOWN)
									{
										if(keyCode == KeyEvent.KEYCODE_DPAD_LEFT)
										{
											getPlayer().setRelativeVol(getPlayer().cOption.relative_vol - 1);
											tvRelative_Vol.setText(Integer.toString(getPlayer().cOption.relative_vol*5));
											updateLanguage_AudioDescriptionSummary();
										}
										else if(keyCode == KeyEvent.KEYCODE_DPAD_RIGHT)
										{
											getPlayer().setRelativeVol(getPlayer().cOption.relative_vol + 1);
											tvRelative_Vol.setText(Integer.toString(getPlayer().cOption.relative_vol*5));
											updateLanguage_AudioDescriptionSummary();
										}
									}
									
									return false;
								}
							})
						.create();
		
		tvRelative_Vol	= (TextView)textEntryView.findViewById(R.id.relative_vol_level);
		bRelative_Vol_left	= (Button)textEntryView.findViewById(R.id.relative_vol_left);
		bRelative_Vol_right	= (Button)textEntryView.findViewById(R.id.relative_vol_right);
		
		tvRelative_Vol.setText(Integer.toString(getPlayer().cOption.relative_vol*5));
		bRelative_Vol_left.setOnClickListener(listener_OnClick);
		bRelative_Vol_right.setOnClickListener(listener_OnClick);
		
		mDialog.show();
	}
	
	private View.OnClickListener listener_OnClick = new View.OnClickListener()
	{
		@Override
		public void onClick(View v)
		{
			int	iID	= v.getId();
			
			if(iID == R.id.relative_vol_left)
			{
				getPlayer().setRelativeVol(getPlayer().cOption.relative_vol - 1);
				tvRelative_Vol.setText(Integer.toString(getPlayer().cOption.relative_vol*5));
				updateLanguage_AudioDescriptionSummary();
			}
			else if(iID == R.id.relative_vol_right)
			{
				getPlayer().setRelativeVol(getPlayer().cOption.relative_vol + 1);
				tvRelative_Vol.setText(Integer.toString(getPlayer().cOption.relative_vol*5));
				updateLanguage_AudioDescriptionSummary();
			}
		}
	};
		
	private void confirmScan()
	{
		AlertDialog.Builder	mBuilder_Scan	= new AlertDialog.Builder(mContext);
		
		mBuilder_Scan.setMessage(getResources().getString(R.string.auto_search));
		
		mBuilder_Scan.setPositiveButton(getResources().getString(R.string.ok), ListenerOnClick_okScan);
		mBuilder_Scan.setNegativeButton(getResources().getString(R.string.cancel), null);
		
		AlertDialog	mDialog_Scan	= mBuilder_Scan.create();
		mDialog_Scan.show();
	}

	@SuppressWarnings("deprecation")
	private void manualScan()
	{
		getMainActivity().showDialog(DxbView.DIALOG_MANUAL_SCAN_OPTION);
	}
	
	private DialogInterface.OnClickListener ListenerOnClick_okScan	= new DialogInterface.OnClickListener()
	{
		public void onClick(DialogInterface dialog, int which)
		{
			isbackpressed = true;
			getPlayer().eState	= DxbPlayer.STATE.OPTION_FULL_SCAN;
			((Activity) mContext).finish();
		}
	};

	private void setCountryCodeValue() {
		CharSequence[] values = mCountryCodeSelect.getEntryValues();
		for(int i=0 ; i<=values.length ; i++) {
			if(i == values.length) {
				mCountryCodeSelect.setValue(values[DxbPlayer.DEFAULT_VALUE_COUNTRYCODE].toString());
			} else if (values[i].equals(mCountryCodeSelect.getValue())) {
				break;
			}
		}
	}
	
	private void setLanguage_AudioValue()
	{
		Log.i(TAG, "setLanguage_AudioValue()");

		ArrayList<CharSequence> revisedEntries = new ArrayList<CharSequence>();
		ArrayList<CharSequence> revisedValues = new ArrayList<CharSequence>();

		if(audio_cursor != null)
		{
			audio_cursor.close();
			audio_cursor	= null;
		}
		
		audio_cursor	= getPlayer().getAudio_LanguageCode();
		if (audio_cursor != null)
			iCount_Language_Audio	= audio_cursor.getCount();
		else
			iCount_Language_Audio	= 0;
		if(iCount_Language_Audio <= 0)
		{
			revisedEntries.add(getResources().getString(R.string.no_information));
			revisedValues.add("1");
			mLanguage_AudioSelect.setValue("1");
		}
		else
		{
			String langCode, strStreamType;
			for(int i=0 ; i<iCount_Language_Audio ; i++)
			{
				int strSize = audio_cursor.getString(1).length();
				try
				{
					if(strSize >= 3)
					{
						langCode	= audio_cursor.getString(1).substring(0, 3);
					}
					else
					{
						langCode	= audio_cursor.getString(1).substring(0, strSize);
					}
				}
				catch(StringIndexOutOfBoundsException e)
				{
					langCode = "";
				}
				
				int audioType = audio_cursor.getInt(2);
				switch(audioType)
				{
					case 0x03:
					case 0x04:
						strStreamType = "[MPEG Audio]";
					break;

					case 0x0f:
					case 0x11:
						strStreamType = "[AAC Audio]";
					break;

					case 0x80:
					case 0x81:
						strStreamType = "[AC3 Audio]";
					break;

					default:
						strStreamType = "[Undefined]";
					break;
				}

				revisedEntries.add(DxbUtil.getLanguageText(langCode) + strStreamType);
				revisedValues.add(Integer.toString(i));
				if(i == getPlayer().cOption.language_audio)
				{
					mLanguage_AudioSelect.setValue(Integer.toString(i));
				}
				audio_cursor.moveToNext();
			}
		}
		
		if(audio_cursor != null)
		{
			audio_cursor.close();
			audio_cursor	= null;
		}
		
		mLanguage_AudioSelect.setEntries(revisedEntries.toArray(new CharSequence[revisedEntries.size()]));
		mLanguage_AudioSelect.setEntryValues(revisedValues.toArray(new CharSequence[revisedValues.size()]));
	}
	
	private void setLanguage_SubtitleValue() {
		Log.i(TAG, "setLanguage_SubtitleValue()");

		ArrayList<CharSequence> subtitleEntries = new ArrayList<CharSequence>();
		ArrayList<CharSequence> subtitleValues = new ArrayList<CharSequence>();
		
		//	default : 0 - Off
		{
			subtitleEntries.add(getResources().getString(R.string.off));
			subtitleValues.add("0");
	
			Log.d(TAG, "gInfo_Option.language_subtitle = " + getPlayer().cOption.language_subtitle);
			if(mLanguage_SubtitleSelect.getValue() == null)
			{
				mLanguage_SubtitleSelect.setValue(Integer.toString(DxbPlayer.DEFAULT_VALUE_LANGUAGE_SUBTITLE));
				getPlayer().cOption.language_subtitle	= DxbPlayer.DEFAULT_VALUE_LANGUAGE_SUBTITLE;
			}
/*			else
			{
				getPlayer().cOption.language_subtitle	= Integer.parseInt((String)mLanguage_SubtitleSelect.getValue());
			}*/
			Log.d(TAG, "gInfo_Option.language_subtitle = " + getPlayer().cOption.language_subtitle);
		}
		
		//	subtitle : 
		Cursor	cursorSubtitle = getPlayer().getSubtitle_LanguageCode();
		if(		(		(cursorSubtitle == null)
					||	(cursorSubtitle.getCount() <= 0)
				)
			&&	(getPlayer().getSubtitleCount(1) <= 0)
		)
		{
			subtitleEntries.add(getResources().getString(R.string.on_no_information));
			subtitleValues.add("1");
		}
		else
		{
			if(getPlayer().cOption.language_subtitle == 0)
			{
				mLanguage_SubtitleSelect.setValue(Integer.toString(0));
			}
			
			int iIndex=1;
			String	langCode;
			for(int i=0 ; i<cursorSubtitle.getCount() ; i++)
			{
				try
				{
					langCode	= cursorSubtitle.getString(1).substring(0, 3);
				}
				catch(StringIndexOutOfBoundsException e)
				{
					langCode = "";
				}
				
				subtitleEntries.add(DxbUtil.getLanguageText(langCode));
				subtitleValues.add(Integer.toString(iIndex));
				if(iIndex == getPlayer().cOption.language_subtitle)
				{
					mLanguage_SubtitleSelect.setValue(Integer.toString(iIndex));
				}
				iIndex++;
				cursorSubtitle.moveToNext();
			}
			
			for(int i=0 ; i<getPlayer().getSubtitleCount(1) ; i++)
			{
				langCode	= getPlayer().getTTX_LanguageCode(i);
				subtitleEntries.add("Teletext - " + DxbUtil.getLanguageText(langCode));
				subtitleValues.add(Integer.toString(iIndex));
				if(iIndex == getPlayer().cOption.language_subtitle)
				{
					mLanguage_SubtitleSelect.setValue(Integer.toString(iIndex));
				}
				iIndex++;
			}
		}
	
		if(cursorSubtitle != null)
		{
			cursorSubtitle.close();
		}
		
		mLanguage_SubtitleSelect.setEntries(subtitleEntries.toArray(new CharSequence[subtitleEntries.size()]));
		mLanguage_SubtitleSelect.setEntryValues(subtitleValues.toArray(new CharSequence[subtitleValues.size()]));
	}
	
	private void setAudioModeValue()
	{
		Log.i(TAG, "setAudioModeValue()");
		
		CharSequence[] values = mAudioMode_Select.getEntryValues();
		
		for(int i=0; i<=values.length; i++)
		{
			if(i == values.length)
			{
				mAudioMode_Select.setValue(values[DxbPlayer.DEFAULT_AUDIO_MODE].toString());
			}
			else if(values[i].equals(mAudioMode_Select.getValue()))
			{
				break;
			}
		}
	}
	
	private void setTimeZoneValue()
	{
		Log.i(TAG, "setTimeZoneValue()");

		CharSequence[] values	= mTimeZoneSelect.getEntryValues();
		for(int i=0 ; i<=values.length ; i++)
		{
			Log.d(TAG, "values.length=" + values.length + ", i=" + i);
			if(i == values.length)
			{
				mTimeZoneSelect.setValue(values[DxbPlayer.DEFAULT_VALUE_TIME_ZONE_TYPE].toString());
			}
			else if(values[i].equals(mTimeZoneSelect.getValue()))
			{
				Log.d(TAG, "mTimeZoneSelect == break");
				break;
			}
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

	private final String SD0_DATA_PATH	= "/data/data/com.telechips.android.dvb/databases";
	private final String DB_PATH		= "/sat_transponder.db";
	@SuppressWarnings("deprecation")
	private void copy_DBManagement(File sd, File usb, int value)
	{
		String	sFIS=null, sFOS=null;
		if(value == 0)
		{
			sFIS	= DxbStorage.USB0_PATH + DB_PATH;
			sFOS	= SD0_DATA_PATH + DB_PATH;
		}
		else if(value == 1)
		{
			sFIS	= SD0_DATA_PATH + DB_PATH;
			sFOS	= DxbStorage.USB0_PATH + DB_PATH;
		}
		
		copyDB(sFIS, sFOS);
		showDialog(DIALOG_COPY_LOAD + value);
	}
	
	private int copyDB(String sFIS, String sFOS)
	{
		int	copycount	= 0;
		File file	= new File(sFIS);
		if(file!=null && file.exists())
		{
			try
			{
				FileInputStream fis = new FileInputStream(file);
				FileOutputStream newfos = new FileOutputStream(sFOS);
				
				int	readcount	= 0;
				byte[]	buffer	= new byte[1024];
				
				while((readcount = fis.read(buffer, 0, 1024)) != -1)
				{
					copycount	= copycount + readcount;
		
					newfos.write(buffer, 0, readcount);
				}
				
				newfos.close();
				fis.close();
			}
			catch(Exception e)
			{
				Log.e(TAG, "could not persist screen output select", e);
			}
		}
	
		return copycount;
	}
	
	private ProgressDialog mDialog_Copy	= null;
	private final int DIALOG_COPY_LOAD		= 0;
//	private final int DIALOG_COPY_BACKUP	= 1;
	public Dialog onCreateDialog(final int id)
	{
		mDialog_Copy	= new ProgressDialog(this);
		mDialog_Copy.getWindow().setBackgroundDrawable(new ColorDrawable(0xff343434));

		mDialog_Copy.setTitle(getResources().getStringArray(R.array.db_management_select_entries)[id - DIALOG_COPY_LOAD]);
		mDialog_Copy.setMessage(getResources().getString(R.string.copying));
		mDialog_Copy.setProgressStyle(ProgressDialog.STYLE_SPINNER);
		mDialog_Copy.setIndeterminate(true);
		mDialog_Copy.setCancelable(false);
		
		mDialog_Copy.show();
		
		mHandler_Main.postDelayed(mRunnable_copy, 10000);
		
		return null;
	}
	private Runnable mRunnable_copy = new Runnable()
	{
		public void run()
		{
			Log.d(TAG, "mRunnable_copy --> run()");
			mDialog_Copy.dismiss();
		}
	};

	private void initIntentFilter() {
		// Scheduler
		{
			IntentFilter	commandFilter	= new IntentFilter();
			commandFilter.addAction(Alert.ACTION_ALERT_STATE);
			registerReceiver(mReceiverIntent, commandFilter);
		}
		// Storage
		{
			IntentFilter	commandFilter	= new IntentFilter();
			commandFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
			commandFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
			registerReceiver(mBroadcastReceiver, commandFilter);
		}
	}

	private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(final Context context, Intent intent) {
			final String action = intent.getAction();
			if (Intent.ACTION_MEDIA_MOUNTED.equals(action) || Intent.ACTION_MEDIA_UNMOUNTED.equals(action)) {
				setStorageValue();
				updateState(true);
			}
		}
	};

	private BroadcastReceiver mReceiverIntent = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			Log.i(TAG, "onReceive()");
			String action = intent.getAction();
			if(Alert.ACTION_ALERT_STATE.equals(action)) {
				Log.d(TAG, "if(Alert.ACTION_IS_ALERT.equals(action))");
				int iAlert_state	= intent.getIntExtra(Alert.EXTRA_TYPE_ALERT_STATE, Alert.TYPE_ALERT_STOP);
				if(iAlert_state == Alert.TYPE_ALERT_START) {
					isSchedulerStart	= true;
				}
			}
		}
	};
}
