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
package com.telechips.android.isdbt;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.preference.CheckBoxPreference;
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
import java.util.ArrayList;

import com.telechips.android.isdbt.player.DxbPlayer;
import com.telechips.android.isdbt.schedule.Alert;
import com.telechips.android.isdbt.util.DxbStorage;
import com.telechips.android.isdbt.util.DxbUtil;

public class DxbOptions extends PreferenceActivity implements Preference.OnPreferenceChangeListener, Preference.OnPreferenceClickListener
{
	private static final String TAG = "[[[DxbOptions]]] ";

	private static Context mContext;

	/*	Visible Check	*/
	public static boolean	VISIBLE_ID_frequency_sel            = false;
	public static boolean	VISIBLE_ID_preset                   = false;
	public static boolean	VISIBLE_ID_parental_rating          = false;
	public static boolean	VISIBLE_ID_seamless_change          = false;
	public static boolean	VISIBLE_ID_handover                 = false;
	public static boolean	VISIBLE_ID_ews                      = false;
	public static boolean	VISIBLE_ID_bcas_mode                = false;
	public static boolean	VISIBLE_ID_bcas_info                = false;
	public static boolean	VISIBLE_ID_signal_quality           = true;
	public static boolean	VISIBLE_ID_field_log                = false;

	public static boolean	VISIBLE_ID_preset_mode				= false;

	/*	select key	*/
	public static final String KEY_AUDIO_SELECT               = "key_audio_select";
	public static final String KEY_DUAL_AUDIO_SELECT          = "key_dual_audio_select";
	public static final String KEY_CAPTION_SELECT             = "key_caption_select";
	public static final String KEY_SUPER_IMPOSE_SELECT        = "key_super_impose_select";
	public static final String KEY_FREQ_VHF                   = "key_frequency_vhf";
	public static final String KEY_FREQ_MID                   = "key_frequency_mid";
	public static final String KEY_FREQ_SHB                   = "key_frequency_shb";
	public static final String KEY_CHANGE_AREA_SELECT         = "key_change_area_select";
	public static final String KEY_AUTO_SCAN_SELECT           = "key_auto_scan";
	public static final String KEY_RESCAN_SELECT              = "key_rescan";
	public static final String KEY_MANUAL_SCAN_SELECT         = "key_manual_scan";
	public static final String KEY_PARENTAL_RATING_SELECT     = "key_parental_rating_select";
	public static final String KEY_CHANGE_PASSWORD_SELECT     = "key_change_password_select";
	public static final String KEY_SEAMLESS_CHANGE_SELECT     = "key_seamless_change_select";
	public static final String KEY_HANDOVER_SELECT            = "key_handover_select";
	public static final String KEY_EWS_SELECT                 = "key_ews_select";
	public static final String KEY_BCASMODE_SELECT            = "key_bcas_mode_select";
	public static final String KEY_BCASCARD_INFO_SELECT       = "key_bcas_card_information";
	public static final String KEY_BCASCARD_CARD_ID           = "key_card_identifier";
	public static final String KEY_BCASCARD_INDIVIDUAL_ID     = "key_individual_id";
	public static final String KEY_BCASCARD_GROUP_ID          = "key_group_id";
	public static final String KEY_STORAGE_SELECT             = "key_storage_select";
	public static final String KEY_SIGNAL_QUALITY_SELECT      = "key_signal_quality_select";
	public static final String KEY_FIELD_LOG			      = "key_field_log";
	public static final String KEY_PRESET_MODE				  = "key_preset_mode_select";

	/*	perference value	*/
	private ListPreference mAudioSelect;
	private ListPreference mDualAudioSelect;
	private ListPreference mCaptionSelect;
	private ListPreference mSuperImposeSelect;
	private CheckBoxPreference mFreqSel_VHF;
	private CheckBoxPreference mFreqSel_MID;
	private CheckBoxPreference mFreqSel_SHB;
	private ListPreference mAreaSelect;
	private PreferenceScreen mAutoScanSelect;
	private PreferenceScreen mRescanSelect;
	private PreferenceScreen mManualScanSelect;
	private EditTextPreference mParentalRatingSelect;
	private EditTextPreference mChangePasswordSelect;
	private ListPreference mSeamlessChangeSelect;
	private ListPreference mHandoverSelect;
	private ListPreference mEWSSelect;
	private ListPreference mStorageSelect;
	private ListPreference mBCASModeSelect;		/* how to do when there is B-CAS card error */
	private PreferenceScreen mBCAScardSelect;
	private Preference mCardIdentifierSelect;
	private Preference mIndividualIDSelect;
	private Preference mGroupIDSelect;
	private ListPreference mSignalQualitySelect;
	private CheckBoxPreference mFieldLog = null;
	private ListPreference mPresetMode;
			
	/*	dialog value	*/
	Builder	mBuilder;
	static Dialog	mDialog;

	private int	iCount_Language_Audio;
	private Cursor	audio_cursor	= null;

	public static boolean isbackpressed;
	public static boolean isSchedulerStart;

	private Handler mHandler_Main = new Handler();

	public static DxbOptions getInstance() {
		return (DxbOptions)mContext;
	}

	private ISDBTPlayerActivity getMainActivity() {
		return ISDBTPlayerActivity.getInstance();
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

	@Override
	public void onCreate(Bundle saveInstanceState)
	{
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onCreate()");
		
		super.onCreate(saveInstanceState);
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

		if (getPlayer().getCountryCode() == DxbPlayer.ISDBT_COUNTRY_JAPAN)		
		{
			addPreferencesFromResource(R.xml.options_isdbt_japan);
		}
		else
		{
			addPreferencesFromResource(R.xml.options_isdbt_brazil);
		}

		if (getPlayer().isBookRecord())
		{
			isSchedulerStart = false;
		}
		else
		{
			isSchedulerStart = true;
		}
		
		if (getPlayer().getLocalPlayState() != DxbPlayer.LOCAL_STATE.STOP)
			getPreferenceScreen().removePreference(findPreference("scan_category"));

		getListView().setBackgroundColor(Color.rgb(16, 16, 16));
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		isbackpressed = false;
		isSchedulerStart = false;
		mContext = this;

		initIntentFilter();
	}

	@Override
	public void onStart() {
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onStart()");
		super.onStart();
	}

	@Override
	protected void onResume() {
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onResume()");
		super.onResume();
		
		mHandler_Main.post(mDefault_Config_Runnable);
	}

	@Override
	protected void onPause()
	{
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onPause()");
		super.onPause();
		
		if(		(getMainActivity().intentOptionActivity != null)
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
			
			getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
			this.finish();
			android.os.Process.killProcess(android.os.Process.myPid());
		}
	}

	@Override
	public void onStop()
	{
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onStop()");
		super.onStop();
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onStop() finish");
	}

	@Override
	public void onDestroy()
	{
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onDestroy()");

		if(		(getMainActivity().intentOptionActivity != null)
			&&	(isbackpressed == false)
			&&	(isSchedulerStart == false)
		)
		{
			getPlayer().onDestroy();
			
			if(getPlayer().isValid() == false)
			{
				getPlayer().isOFF = true;
			}
		}

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
				if(mParentalRatingSelect != null)	
				{
					mParentalRatingSelect.setText(null);
				}
				if(mChangePasswordSelect != null)
				{
					mChangePasswordSelect.setText(null);	
				}
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

	private void updateState(boolean force)
	{
		Log.i(TAG, "updateState()");

		updateAudioSelectSummary(mAudioSelect.getValue());
		updateDualAudioSelectSummary(mDualAudioSelect.getValue());		
		updateCaptionSelectSummary(mCaptionSelect.getValue());
		updateSuperImposeSelectSummary(mSuperImposeSelect.getValue());

		if(VISIBLE_ID_parental_rating)
		{
			updateParentalRatingSelectSummary(getPlayer().cOption.age);
		}

		if(VISIBLE_ID_seamless_change)
		{
			updateSeamlessChangeSelectSummary(mSeamlessChangeSelect.getValue());
		}
		
		if(VISIBLE_ID_handover)
		{
			updateHandoverSelectSummary(mHandoverSelect.getValue());
		}
		
		if(VISIBLE_ID_ews)
		{
			updateEWSSelectSummary(mEWSSelect.getValue());
		}

		if(VISIBLE_ID_bcas_mode)
		{
			updateBCASModeSelectSummary(mBCASModeSelect.getValue());
		}

		updateStorageSelectSummary();
		
		if(VISIBLE_ID_signal_quality)
		{
			updateSignalQualitySelectSummary(mSignalQualitySelect.getValue());
		}
		
		if(VISIBLE_ID_preset_mode==true)
			updatePresetModeSummary (mPresetMode.getValue());
	}

	private void updateAudioSelectSummary(Object value)
	{
		Log.i(TAG, "updateAudioSelectSummary()");

		getPlayer().setAudio(Integer.parseInt((String) value));
		
		CharSequence[] summaries = getResources().getTextArray(R.array.audio_select_summaries);
		CharSequence[] values = mAudioSelect.getEntryValues();
		for (int i=0; i<values.length; i++)
		{
			if (values[i].equals(value))
			{
				mAudioSelect.setSummary(summaries[i]);
				break;
			}
		}
	}

	private void updateDualAudioSelectSummary(Object value)
	{
		Log.i(TAG, "updateDualAudioSelectSummary()");

		getPlayer().setAudioDualMono(Integer.parseInt((String) value));
		
		CharSequence[] summaries = getResources().getTextArray(R.array.dual_audio_select_summaries);
		CharSequence[] values = mDualAudioSelect.getEntryValues();
		for (int i=0; i<values.length; i++)
		{
			if (values[i].equals(value))
			{
				mDualAudioSelect.setSummary(summaries[i]);
				break;
			}
		}
	}
			
	private void updateCaptionSelectSummary(Object value)
	{
		Log.i(TAG, "updateCaptionSelectSummary()");
		
		getPlayer().setCaption(Integer.parseInt((String) value));
		
		CharSequence[] summaries = getResources().getTextArray(R.array.caption_select_summaries);
		CharSequence[] values = mCaptionSelect.getEntryValues();
		for (int i=0; i<values.length; i++)
		{
			if (values[i].equals(value))
			{
				getPlayer().setCaption(i);
				mCaptionSelect.setSummary(summaries[i]);
				break;
			}
		}
	}
	
	private void updateSuperImposeSelectSummary(Object value)
	{
		Log.i(TAG, "updateSuperImposeSelectSummary()");

		getPlayer().setSuperImpose(Integer.parseInt((String) value));
		
		CharSequence[] summaries = getResources().getTextArray(R.array.super_impose_select_summaries);
		CharSequence[] values = mSuperImposeSelect.getEntryValues();
		for (int i=0; i<values.length; i++)
		{
			if (values[i].equals(value))
			{
				
				mSuperImposeSelect.setSummary(summaries[i]);
				break;
			}
		}
	}	

	private void updateParentalRatingSelectSummary(int value)
	{
		Log.i(TAG, "updateParentalRatingSelectSummary(value=" + value + ")");

		if(value >= 0)
		{
			getPlayer().setParentalRate(value);
			
			CharSequence[] summaries = getResources().getTextArray(R.array.parental_rating_select_summaries);
			mParentalRatingSelect.setSummary(summaries[value]);
		}
	}

	private void updateSeamlessChangeSelectSummary(Object value)
	{
		Log.i(TAG, "updateSeamlessChangeSelectSummary()");
		
		getPlayer().setSeamlessChange(Integer.parseInt((String) value));
		
		CharSequence[] summaries = getResources().getTextArray(R.array.seamless_change_select_summaries);
		CharSequence[] values = mSeamlessChangeSelect.getEntryValues();
		for (int i=0; i<values.length; i++)
		{
			if (values[i].equals(value))
			{
				mSeamlessChangeSelect.setSummary(summaries[i]);
				break;
			}
		}
	}

	private void updateHandoverSelectSummary(Object value)
	{
		Log.i(TAG, "updateHandoverSelectSummary()");

		getPlayer().setHandover(Integer.parseInt((String) value));
		
		CharSequence[] summaries = getResources().getTextArray(R.array.handover_select_summaries);
		CharSequence[] values = mHandoverSelect.getEntryValues();
		for (int i=0; i<values.length; i++)
		{
			if (values[i].equals(value))
			{
				mHandoverSelect.setSummary(summaries[i]);
				break;
			}
		}
	}

	private void updateEWSSelectSummary(Object value)
	{
		Log.i(TAG, "updateEWSSelectSummary()");

		getPlayer().setEWS(Integer.parseInt((String) value));
		
		CharSequence[] summaries = getResources().getTextArray(R.array.ews_select_summaries);
		CharSequence[] values = mEWSSelect.getEntryValues();
		for (int i=0; i<values.length; i++)
		{
			if (values[i].equals(value))
			{
				mEWSSelect.setSummary(summaries[i]);
				break;
			}
		}
	}

	private void updateBCASModeSelectSummary(Object value)
	{
		Log.i(TAG, "updateBCASCardModeSelectSummary()");

		CharSequence[] summaries = getResources().getTextArray(R.array.bcas_mode_select_summaries);
		CharSequence[] values = mBCASModeSelect.getEntryValues();
		for (int i=0; i<values.length; i++)
		{
			if (values[i].equals(value))
			{
				mBCASModeSelect.setSummary(summaries[i]);
				break;
			}
		}
	}
	
	private void updateBCASCardInfoSelectSummary()
	{
		Log.i(TAG, "updateBCASCardInfoSelectSummary()");
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

	private void updateSignalQualitySelectSummary(Object value)
	{
		Log.i(TAG, "updateSignalQualitySelectSummary()");

		getPlayer().setSignalQuality(Integer.parseInt((String) value));
		
		CharSequence[] summaries = getResources().getTextArray(R.array.signal_quality_select_summaries);
		CharSequence[] values = mSignalQualitySelect.getEntryValues();
		for (int i=0; i<values.length; i++)
		{
			if (values[i].equals(value))
			{
				mSignalQualitySelect.setSummary(summaries[i]);
				break;
			}
		}
	}
		
	private void updatePresetModeSummary (Object value)
	{
		Log.i(TAG, "updatePresetModeSummary");

		if (VISIBLE_ID_preset_mode == true) {
			CharSequence[] summaries = getResources().getTextArray(R.array.preset_mode_select_summaries);
			CharSequence[] values = mPresetMode.getEntryValues();
			for (int i=0; i<values.length; i++)
			{
				if (values[i].equals(value))
				{
					mPresetMode.setSummary(summaries[i]);
					break;
				}
			}
		}
	}

	public boolean onPreferenceClick(Preference preference)
	{
		final String key = preference.getKey();
		
		Log.i(TAG, "OnPreferenceClick() - " + key);
		
		if(KEY_AUTO_SCAN_SELECT.equals(key))
		{
			getPlayer().setScan(0); // 0-initial scan, 1-rescan, 2-area scan, 3-preset, 4-manual scan
			
			confirmScan();
		}
		else if(KEY_RESCAN_SELECT.equals(key))
		{
			getPlayer().setScan(1); // 0-initial scan, 1-rescan, 2-area scan, 3-preset, 4-manual scan

			confirmScan();
		}
		else if(KEY_MANUAL_SCAN_SELECT.equals(key))
		{
			getPlayer().setScan(4); // 0-initial scan, 1-rescan, 2-area scan, 3-preset, 4-manual scan

			manualScan();
		}
		else if(KEY_BCASCARD_INFO_SELECT.equals(key))
		{
			setBCAScardValue();
		}
			
		return false;
	}
	
	public boolean onPreferenceChange(Preference preference, Object objValue)
	{
		final String key = preference.getKey();
		int value;
		
		Log.i(TAG, "onPreferenceChange() - " + key);		

		if (KEY_AUDIO_SELECT.equals(key))
		{
			try
			{
				updateAudioSelectSummary(objValue);
			}
			catch (NumberFormatException e)
			{
				//Log.e(TAG, "", e);
			}
		}
		else if (KEY_DUAL_AUDIO_SELECT.equals(key))
		{
			try
			{
				updateDualAudioSelectSummary(objValue);
			}
			catch (NumberFormatException e)
			{
				//Log.e(TAG, "", e);
			}
		}
		else if (KEY_CAPTION_SELECT.equals(key))
		{
			try
			{
				updateCaptionSelectSummary(objValue);
			}
			catch (NumberFormatException e)
			{
				//Log.e(TAG, "", e);
			}
		}
		else if (KEY_SUPER_IMPOSE_SELECT.equals(key))
		{
			try
			{
				updateSuperImposeSelectSummary(objValue);
			}
			catch (NumberFormatException e)
			{
				//Log.e(TAG, "", e);
			}
		}
		else if(KEY_CHANGE_AREA_SELECT.equals(key))
		{
			value = Integer.parseInt((String) objValue);
			try
			{
				getPlayer().cOption.area_0_temp	= value;
				
				mBuilder = new AlertDialog.Builder(this);
				
				mBuilder.setTitle(getResources().getString(R.string.area_code));
				mBuilder.setSingleChoiceItems(getPlayer().getRarray_Area(getPlayer().cOption.area_0_temp), -1, new DialogInterface.OnClickListener()
				{
					public void onClick(DialogInterface dialog, int item)
					{
						Log.i(TAG, "onClick(dialog=" + dialog + ", item=" + item);

						getPlayer().cOption.area_1_temp = item;
						
						selectScan();
					}
				});

				mBuilder.setPositiveButton(getResources().getString(R.string.cancel), null);
				mDialog = mBuilder.create();
				mDialog.show();
			}
			catch(NumberFormatException e)
			{
				//Log.e(TAG, "", e);
			}
		}
		else if (KEY_PARENTAL_RATING_SELECT.equals(key))
		{
			//value = Integer.parseInt((String) objValue);
			try
			{
				Editable	text = mParentalRatingSelect.getEditText().getText();
				if(text.toString().equalsIgnoreCase(getPlayer().cOption.password))
				{
					mParentalRatingSelect.setDialogMessage("");
					mParentalRatingSelect.setText(null);

					//Log.d(TAG, "success!!!  - " + mParentalRatingSelect.getText());
					
					mBuilder = new AlertDialog.Builder(this);

					mBuilder.setTitle(getResources().getString(R.string.age));
					mBuilder.setSingleChoiceItems(R.array.parental_rating_select_entries, getPlayer().cOption.age, new DialogInterface.OnClickListener()
					{
						public void onClick(DialogInterface dialog, int item)
						{
							dialog.dismiss();
							
							updateParentalRatingSelectSummary(item);
						}
					});

					mBuilder.setPositiveButton(getResources().getString(R.string.cancel), null);
					mDialog = mBuilder.create();
					mDialog.show();
				}
				else
				{
					mParentalRatingSelect.setDialogMessage(R.string.Please_enter_the_correct_password);
					mParentalRatingSelect.setText(null);

					//Log.d(TAG, "error!!!  - " + mParentalRatingSelect.getText());
				}
				//mParentalRatingSelect.getEditText().clearComposingText();
			}
			catch (NumberFormatException e)
			{
				//Log.e(TAG, "", e);
			}
		}
		else if(KEY_CHANGE_PASSWORD_SELECT.equals(key))
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
					mBuilder    = new AlertDialog.Builder(this);
					mBuilder.setTitle(getResources().getString(R.string.change_password));
					mBuilder.setMessage(getResources().getString(R.string.please_enter_the_correct_password));

					mBuilder.setPositiveButton( getResources().getString(R.string.ok), null);

					mBuilder.show();
				}
			}
			catch(NumberFormatException e)
			{
				//Log.e(TAG, "could not persist screen output select", e);
			}
		}
		else if (KEY_SEAMLESS_CHANGE_SELECT.equals(key)) //seamless change
		{
			try
			{
				updateSeamlessChangeSelectSummary(objValue);
			}
			catch (NumberFormatException e)
			{
				//Log.e(TAG, "", e);
			}
		}
		else if(KEY_HANDOVER_SELECT.equals(key))
		{
			try
			{
				updateHandoverSelectSummary(objValue);
			}
			catch (NumberFormatException e)
			{
				//Log.e(TAG, "", e);
			}
		}
		else if(KEY_EWS_SELECT.equals(key))
		{
			try
			{
				updateEWSSelectSummary(objValue);
			}
			catch (NumberFormatException e)
			{
				//Log.e(TAG, "", e);
			}
		}			
		else if(KEY_BCASMODE_SELECT.equals(key))
		{
			value	= Integer.parseInt((String) objValue);
			try
			{
				if((value == 0) || (value == 1)) // 0=normal, 1=1seg
				{
					updateBCASModeSelectSummary(objValue);
				}
			}
			catch(NumberFormatException e)
			{
				//Log.e(TAG, "", e);
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
				//Log.e(TAG, "", e);
			}
		}						
		else if(KEY_SIGNAL_QUALITY_SELECT.equals(key))
		{
			try
			{
				updateSignalQualitySelectSummary(objValue);
			}
			catch (NumberFormatException e)
			{
				//Log.e(TAG, "", e);
			}
		}				
		else if (KEY_PRESET_MODE.equals(key)) {
			try {
				updatePresetModeSummary(objValue);
			}
			catch (NumberFormatException e) {
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
	
	private void setDefault_Value()
	{
		Log.i(TAG, "setDefault_Value()");

		CharSequence[] values;

		mAudioSelect = (ListPreference) findPreference(KEY_AUDIO_SELECT);
		values = mAudioSelect.getEntryValues();
		for (int i=0; i<=values.length; i++)
		{
			if(i == values.length)
		 	{
				mAudioSelect.setValue(values[DxbPlayer.DEFAULT_VALUE_AUDIO].toString());
			}
			else if (values[i].equals(mAudioSelect.getValue()))
			{
				break;
			}
		}		
		mAudioSelect.setOnPreferenceChangeListener(this);
		
		mDualAudioSelect = (ListPreference) findPreference(KEY_DUAL_AUDIO_SELECT);
		values = mDualAudioSelect.getEntryValues();
		for (int i=0; i<=values.length; i++)
		{
			if(i == values.length)
			{
				mDualAudioSelect.setValue(values[DxbPlayer.DEFAULT_VALUE_DUAL_AUDIO].toString());
			}
			else if (values[i].equals(mDualAudioSelect.getValue()))
			{
				break;
			}
		}		
		mDualAudioSelect.setOnPreferenceChangeListener(this);
		
		mCaptionSelect = (ListPreference) findPreference(KEY_CAPTION_SELECT);
		values = mCaptionSelect.getEntryValues();
		for (int i=0; i<=values.length; i++)
		{
			if(i == values.length)
			{
				mCaptionSelect.setValue(values[DxbPlayer.DEFAULT_VALUE_CAPTION].toString());
			}
			else if (values[i].equals(mCaptionSelect.getValue()))
			{
				break;
			}
		}		
		mCaptionSelect.setOnPreferenceChangeListener(this);
		
		mSuperImposeSelect = (ListPreference) findPreference(KEY_SUPER_IMPOSE_SELECT);
		values = mSuperImposeSelect.getEntryValues();
		for (int i=0; i<=values.length; i++)
		{
			if(i == values.length)
			{
				mSuperImposeSelect.setValue(values[DxbPlayer.DEFAULT_VALUE_SUPER_IMPOSE].toString());
			}
			else if (values[i].equals(mSuperImposeSelect.getValue()))
			{
				break;
			}
		}
		mSuperImposeSelect.setOnPreferenceChangeListener(this);

		if(VISIBLE_ID_preset)
		{
			mAreaSelect	= (ListPreference) findPreference(KEY_CHANGE_AREA_SELECT);
			if (mAreaSelect != null)
				mAreaSelect.setOnPreferenceChangeListener(this);
		}

 		mAutoScanSelect   = (PreferenceScreen)findPreference(KEY_AUTO_SCAN_SELECT);
		if (mAutoScanSelect != null)
 			mAutoScanSelect.setOnPreferenceClickListener(this);

 		mRescanSelect   = (PreferenceScreen)findPreference(KEY_RESCAN_SELECT);
		if (mRescanSelect != null)
 			mRescanSelect.setOnPreferenceClickListener(this);
 
 		mManualScanSelect  = (PreferenceScreen)findPreference(KEY_MANUAL_SCAN_SELECT);
		if (mManualScanSelect != null)
			mManualScanSelect.setOnPreferenceClickListener(this);
		
		if(VISIBLE_ID_parental_rating)
		{
			mParentalRatingSelect = (EditTextPreference) findPreference(KEY_PARENTAL_RATING_SELECT);
			if(mParentalRatingSelect.getText() != null)
			{
				mParentalRatingSelect.setText(null);
			}
			mParentalRatingSelect.setOnPreferenceChangeListener(this);

			mChangePasswordSelect	= (EditTextPreference)findPreference(KEY_CHANGE_PASSWORD_SELECT);
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
		}

		if(VISIBLE_ID_seamless_change)
		{
			mSeamlessChangeSelect = (ListPreference) findPreference(KEY_SEAMLESS_CHANGE_SELECT);
			values = mSeamlessChangeSelect.getEntryValues();
			for (int i=0; i<=values.length; i++)
			{
				if(i == values.length)
				{	
					mSeamlessChangeSelect.setValue(values[DxbPlayer.DEFAULT_VALUE_SEAMLESS_CHANGE].toString());
				}
				else if (values[i].equals(mSeamlessChangeSelect.getValue()))
				{
					break;
				}
			}		
			mSeamlessChangeSelect.setOnPreferenceChangeListener(this);
		}

		if(VISIBLE_ID_handover)
		{
			mHandoverSelect = (ListPreference) findPreference(KEY_HANDOVER_SELECT);
			values = mHandoverSelect.getEntryValues();
			for (int i=0; i<=values.length; i++)
			{
				if(i == values.length)
				{
					if(DxbPlayer.DEFAULT_VALUE_HANDOVER == false)
					{
				    	mHandoverSelect.setValue(values[0].toString());
					}
					else
					{
				    	mHandoverSelect.setValue(values[1].toString());
					}
				}
				else if (values[i].equals(mHandoverSelect.getValue()))
				{
					break;
				}
			}		
			mHandoverSelect.setOnPreferenceChangeListener(this);
		}
		
		if(VISIBLE_ID_ews)
		{
			mEWSSelect = (ListPreference) findPreference(KEY_EWS_SELECT);
			values = mEWSSelect.getEntryValues();
			for (int i=0; i<=values.length; i++)
			{
				if(i == values.length)
			    {
					if(DxbPlayer.DEFAULT_VALUE_EWS == false)
					{
				    	mEWSSelect.setValue(values[0].toString());
					}
					else
					{
				    	mEWSSelect.setValue(values[1].toString());
					}
			    }
				else if (values[i].equals(mEWSSelect.getValue()))
			    {
			        break;
			    }
			}		
			mEWSSelect.setOnPreferenceChangeListener(this);
		}
						
		if(VISIBLE_ID_bcas_mode)
		{
			mBCASModeSelect = (ListPreference)findPreference(KEY_BCASMODE_SELECT);
			values = mBCASModeSelect.getEntryValues();
			for (int i=0; i<=values.length; i++)
			{
				if(i == values.length)
				{	
					mBCASModeSelect.setValue(values[DxbPlayer.DEFAULT_VALUE_BCAS_MODE].toString());
				}
				else if (values[i].equals(mBCASModeSelect.getValue()))
				{
					break;
				}
			}
			mBCASModeSelect.setOnPreferenceChangeListener(this);
		}

		if(VISIBLE_ID_bcas_info)
		{
			mBCAScardSelect	= (PreferenceScreen)findPreference(KEY_BCASCARD_INFO_SELECT);
			mBCAScardSelect.setOnPreferenceClickListener(this);
			
			mCardIdentifierSelect = (Preference)findPreference(KEY_BCASCARD_CARD_ID);
			mIndividualIDSelect = (Preference)findPreference(KEY_BCASCARD_INDIVIDUAL_ID);
			mGroupIDSelect = (Preference)findPreference(KEY_BCASCARD_GROUP_ID);
			
			getPlayer().requestBCASInfo(0);
		}
				
		if(VISIBLE_ID_signal_quality)
		{
			mSignalQualitySelect = (ListPreference) findPreference(KEY_SIGNAL_QUALITY_SELECT);
			values = mSignalQualitySelect.getEntryValues();
			for (int i=0; i<=values.length; i++)
			{
				if(i == values.length)
			    {
					if(DxbPlayer.DEFAULT_VALUE_SIGNAL_QUALITY == false)
					{
				    	mSignalQualitySelect.setValue(values[0].toString());
					}
					else
					{
				    	mSignalQualitySelect.setValue(values[1].toString());
					}
			    }
				else if (values[i].equals(mSignalQualitySelect.getValue()))
			    {
			        break;
			    }
			}		
			mSignalQualitySelect.setOnPreferenceChangeListener(this);
		}


		if (VISIBLE_ID_frequency_sel) {
			mFreqSel_VHF = (CheckBoxPreference) findPreference(KEY_FREQ_VHF);
			mFreqSel_MID = (CheckBoxPreference) findPreference(KEY_FREQ_MID);
			mFreqSel_SHB = (CheckBoxPreference) findPreference(KEY_FREQ_MID);
		}

		mStorageSelect = (ListPreference) findPreference(KEY_STORAGE_SELECT);
		if(mStorageSelect !=null) { 
			//setStorageValue(1, null);
			setStorageValue();
			mStorageSelect.setOnPreferenceChangeListener(this);
		}

		if (VISIBLE_ID_field_log) {
			mFieldLog = (CheckBoxPreference) findPreference("key_field_log");
		}

		/* ----- preset mode-----*/
		if (VISIBLE_ID_preset_mode == true) {
			mPresetMode = (ListPreference) findPreference(KEY_PRESET_MODE);
			values = mPresetMode.getEntryValues();
			for(int i=0; i <= values.length;i++)
			{
				if (i==values.length) mPresetMode.setValue(values[0].toString());
				else if (values[i].equals(mPresetMode.getValue())) break;
			}
			mPresetMode.setOnPreferenceChangeListener(this);
		}
	}
	
	private void confirmScan()
	{
		AlertDialog.Builder	mBuilder_Scan	= new AlertDialog.Builder(mContext);
		int scan_type = getPlayer().getScan();
		
		if (scan_type==0)		//initial scan
			mBuilder_Scan.setMessage(getResources().getString(R.string.auto_search));
		else if (scan_type==1)
			mBuilder_Scan.setMessage(getResources().getString(R.string.rescan));
		else
			Log.i(TAG, "confirmScan="+scan_type+" error");
		
		mBuilder_Scan.setPositiveButton(getResources().getString(R.string.ok), ListenerOnClick_okScan);
		mBuilder_Scan.setNegativeButton(getResources().getString(R.string.cancel), null);
		
		AlertDialog	mDialog_Scan	= mBuilder_Scan.create();
		mDialog_Scan.show();
	}

	private void manualScan()
	{
		getMainActivity().showDialog(DxbView.DIALOG_MANUAL_SCAN);
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

	private void selectScan()
	{
		AlertDialog.Builder mBuilder_Scan   = new AlertDialog.Builder(mContext);

		mBuilder_Scan.setMessage(mContext.getResources().getString(R.string.want_to_scan));

		mBuilder_Scan.setPositiveButton(mContext.getResources().getString(R.string.yes), ListenerOnClick_selectScan);
		mBuilder_Scan.setNegativeButton(mContext.getResources().getString(R.string.no), ListenerOnClick_selectScan);

		AlertDialog mDialog_Scan    = mBuilder_Scan.create();
		mDialog_Scan.show();
	}

	private DialogInterface.OnClickListener ListenerOnClick_selectScan   = new DialogInterface.OnClickListener()
	{
		public void onClick(DialogInterface dialog, int which)
		{
			Log.i(TAG, "onClick(dialog="+dialog+", which="+which);

			int item    = which * -1;
			getPlayer().cOption.scan   = item + 1;
			getPlayer().cOption.area_0 = getPlayer().cOption.area_0_temp;
			getPlayer().cOption.area_1 = getPlayer().cOption.area_1_temp;

			getPlayer().setArea_0(getPlayer().cOption.area_0);
			getPlayer().setArea_1(getPlayer().cOption.area_1);
			getPlayer().makePresetList();
			
			dialog.dismiss();
			mDialog.dismiss();
			
			isbackpressed = true;
			getPlayer().eState	= DxbPlayer.STATE.OPTION_FULL_SCAN;
			((Activity) mContext).finish();
		}
	};
	
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
				if(storageVolume.getName().equalsIgnoreCase("emulated")) {
					continue;
				} else if (storageVolume.canWrite()) {
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

	private void setBCAScardValue()
	{
		Log.i(TAG, "setBCAScardValue()");

		String type = "";
		String cardID = "";
		String groupID = "";
		
		getPlayer().requestBCASInfo(0);
		
		if(getPlayer().cBCAScardInfo.number >= 1)
		{
			type = getPlayer().cBCAScardInfo.type;
			
			cardID = getPlayer().cBCAScardInfo.ID_card[0] + " " +
						getPlayer().cBCAScardInfo.ID_card[1] + " " +
						getPlayer().cBCAScardInfo.ID_card[2] + " " +
						getPlayer().cBCAScardInfo.ID_card[3];
			
			if(getPlayer().cBCAScardInfo.number > 1)
			{
				groupID	= getPlayer().cBCAScardInfo.ID_group[0] + " " +
							getPlayer().cBCAScardInfo.ID_group[1] + " " +
							getPlayer().cBCAScardInfo.ID_group[2] + " " +
							getPlayer().cBCAScardInfo.ID_group[3];
			}
		}
		
		mCardIdentifierSelect.setSummary(type);
		mIndividualIDSelect.setSummary(cardID);
		mGroupIDSelect.setSummary(groupID);
	}

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
