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

import java.util.regex.Pattern;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.drawable.ColorDrawable;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;

import com.telechips.android.isdbt.player.*;
import com.telechips.android.isdbt.player.isdbt.Channel;
import com.telechips.android.isdbt.player.isdbt.ISDBTPlayer;

public class DxbView_Scan extends DxbView
{
	private class InputValidator implements TextWatcher
	{
		private EditText mEditText;

		private InputValidator(EditText editText) {
			this.mEditText = editText;
		}

		@Override
		public void afterTextChanged(Editable s) {
		}
		
		@Override
		public void beforeTextChanged(CharSequence s, int start, int count, int after) {
		}

		@Override
		public void onTextChanged(CharSequence s, int start, int before, int count) {
			if(s.length() > 0) {
				if(!Pattern.matches("^[0-9]{6}$", s)) {
					mEditText.setError(getResources().getString(R.string.manual_scan_warning));
				}
			}	
		}
	}

	private ProgressDialog mDialog_Scan;
	
	public DxbView_Scan(Context context)
	{
		super(context);

		getPlayer().setOnSearchPercentListener(ListenerOnSearchPercent);
		getPlayer().setOnSearchCompletionListener(ListenerOnSearchCompletion);
		getPlayer().setOnChannelUpdateListener(ListenerOnChannelUpdate);
		getPlayer().setOnDBInformationUpdateListener(ListenerOnDBInformationUpdateListener);
		getPlayer().setOnAutoSearchInfoListener (ListenerOnAutoSearchInfoUpdate);
	}

	private DxbPlayer.OnSearchPercentListener ListenerOnSearchPercent = new DxbPlayer.OnSearchPercentListener()
	{
		String message = "";

		public void onSearchPercentUpdate(DxbPlayer player, int nPercent, int nChannel)
		{
			//DxbLog(I, "ListenerOnSearchPercent --> onSearchPercentUpdate(" + nPercent + ")(" + nChannel + ")");

			if(nPercent <= 1)
			{
				message = "";
			}

			if(nChannel >= 0)
			{
				DxbChannelData channel = getPlayer().getChannel(-1, nChannel);
				if(channel != null)
				{
					int threeDigitNumber  = channel.iThreeDigitNumber;
					String channelName = channel.sChannel_name;

					if(threeDigitNumber <= 0)
					{
						message = channelName;
					}
					else if(threeDigitNumber < 10)
					{
						message = "[00" + threeDigitNumber + "] " + channelName;
					}
					else if(threeDigitNumber <100)
					{
						message = "[0" + threeDigitNumber + "] " + channelName;
					}
					else
					{
						message = "[" + threeDigitNumber + "] " + channelName;
					}
				}
			}

			mDialog_Scan.setMessage("Please wait while searching...  [ " + nPercent + "% ]\n" + message);
		}
	};
	
	private DxbPlayer.OnSearchCompletionListener ListenerOnSearchCompletion = new DxbPlayer.OnSearchCompletionListener()
	{
		public void onSearchCompletion(DxbPlayer player)
		{
			DxbLog(I, "onSearchCompletion()");
			if(getPlayer().eState != DxbPlayer.STATE.UI_PAUSE)
			{
				removeDialog();
				getContext().setIndicatorReset();
				getPlayer().eState	= DxbPlayer.STATE.GENERAL;
				getPlayer().setChannelList(true);
				getContext().updateChannelList();
			}
		}
	};

	private DxbPlayer.OnChannelUpdateListener ListenerOnChannelUpdate = new DxbPlayer.OnChannelUpdateListener()
	{
		public void onChannelUpdate(DxbPlayer player, int request)
		{
			//DxbLog(I, "OnChannelUpdateListener()");
			
			Channel channel;
		
			if(getPlayer().eState != DxbPlayer.STATE.UI_PAUSE)
			{
				if(getPlayer().getServiceType() == 0)
				{
					getContext().setIndicatorReset();
					getPlayer().eState	= DxbPlayer.STATE.GENERAL;
					getPlayer().setChannelList(false);
					if(request == 1)
					{
						getContext().updateBackScanChannelList();
					}
					else
					{
						getContext().updateChannelList();
					}
				}
			}
		}
	};
	
	private DxbPlayer.OnDBInformationUpdateListener ListenerOnDBInformationUpdateListener = new DxbPlayer.OnDBInformationUpdateListener()
	{
		public void onDBInformationUpdate(DxbPlayer player, int type, int param)
		{
			//Log.e("TAG", "onDBInformationUpdate Playing cursorPosition ==> " + Manager_Setting.g_Information.cCOMM.iCurrent_TV);
			
			if(getPlayer().getServiceType() == 2) {
				return;
			}
			
			switch(param)
			{
				case 0x55:	// (85)parental_rating_descriptor
					if((type & 1) == 0)	// type : 0-PF, 1-schedule
					{
						int epg_type = (type & 2) >> 1;	// epg_type	: 0-p, 1-f
						int rating = (type >> 8) & 0xFF;
						//DxbLog(D, "ListenerOnDBInformationUpdateListener --> epg_type=" + epg_type + ", rating=" + rating  + ", param=" + param + ")");
						if(		(epg_type == 0)
							&&	(rating != getPlayer().getCurrentRating())
						)
						{
							getPlayer().setCurrentRating(rating);
							getContext().updateScreen();
						}
					}
				break;

				case 0x4D:	// (77)short_event_descriptor
				case 0x4E:	// (78)extended_event_descriptor
					if((type & 1) == 0)	// type : 0-PF, 1-schedule
					{
						getContext().updateEPG_PF();
					}
					else if((type & 1) == 1)
					{
						//DxbLog(D, "ListenerOnDBInformationUpdateListener --> param=" + param + ")");
						getPlayer().setEPGUpdate_state(true);
					}
				break;

				//case 0x54:	// (84)content_descriptor
				//break;

				case 0x83:
				getContext().updateChannelList();
				break;

			default:
				break;
			}
			
			//DxbPlayer.mPlayer.setOnDBInformationUpdateListener(null);
		}
	};
	
	private DxbPlayer.OnAutoSearchInfoListener ListenerOnAutoSearchInfoUpdate = new DxbPlayer.OnAutoSearchInfoListener()
	{
		public void onAutoSearchInfoUpdate (DxbPlayer player, int ext1, int ext2) 
		{
			int search_status = 0;
			int fullseg_row, oneseg_row;
			
			search_status = getPlayer().getAutoSearchInfoStatus();
			fullseg_row = getPlayer().getAutoSearchInfoFullseg();
			oneseg_row = getPlayer().getAutoSearchInfoOneseg();

			DxbLog(I, "OnAutoSearchInfoListener: status =" + search_status + "full=" + fullseg_row + "one=" + oneseg_row);

			if(getPlayer().eState != DxbPlayer.STATE.UI_PAUSE) {
				removeDialog();
				getContext().setIndicatorReset();
				getPlayer().eState	= DxbPlayer.STATE.GENERAL;
				getPlayer().setChannelList(false);
				if (search_status==1)	
					getContext().updateChannelAutoSearch(fullseg_row, oneseg_row);	
				else
					getContext().updateChannelList();		
			}
		}
	};
	
	public void startFull()
	{
		DxbLog(I, "startFull() DxbPlayer.eState ="+ getPlayer().eState);
		getPlayer().iManualScan_Frequency	= -1;
		
		if(getPlayer().eState != DxbPlayer.STATE.GENERAL)
		{
			updateToast(getResources().getString(R.string.please_wait_cancel_scanning));
			return;
		}
		getPlayer().search();
		getContext().showDialog(DIALOG_SCAN);
	}
	
	public void startManual(int KHz_frequency, int Mhz_bandwidth)
	{
		DxbLog(I, "startManual() DxbPlayer.eState ="+ getPlayer().eState);
		getPlayer().iManualScan_Frequency	= KHz_frequency;
		
		if(getPlayer().eState != DxbPlayer.STATE.GENERAL)
		{
			updateToast(getResources().getString(R.string.please_wait_cancel_scanning));
			return;
		}
		getPlayer().manualScan(KHz_frequency, Mhz_bandwidth);
		getContext().showDialog(DIALOG_SCAN);
	}

	public void startAutoSearch(int Direction)
	{
		DxbLog(I,"startAutoSearch("+Direction+") DxbPlayer.eState="+getPlayer().eState);
		getPlayer().iManualScan_Frequency = -1;
		if (getPlayer().eState != DxbPlayer.STATE.GENERAL) {
			updateToast(getResources().getString(R.string.please_wait_cancel_scanning));
			return;
		}
		getPlayer().autosearch(Direction);
		getContext().showDialog(DIALOG_SCAN);
	}
	
	public void cancel() {
		DxbLog(I, "cancel()");
		if(getPlayer().isValid()) {
			getPlayer().searchCancel();
		}
	}

	private boolean isValidFreq(String frequency) {
		try {
			Integer.parseInt(frequency);
			return true;
		} catch(NumberFormatException e) {
			return false;
		}
	}

	private void removeDialog()
	{
		if(mDialog_Scan!=null && mDialog_Scan.isShowing())
		{
			ISDBTPlayerActivity.getInstance().removeDialog(DIALOG_SCAN);
		}
	}
	
	Dialog dlg = null;
	protected Dialog onCreateDialog(int id)
	{
		if(id == DIALOG_SCAN)
		{
			//if(mDialog == null)
			{
				mDialog_Scan = new ProgressDialog(getContext());
				mDialog_Scan.getWindow().setBackgroundDrawable(new ColorDrawable(0xff343434));
			}
			mDialog_Scan.setTitle(getResources().getString(R.string.app_name));
			mDialog_Scan.setMessage(getResources().getString(R.string.please_wait_while_searching));
			mDialog_Scan.setProgressStyle(ProgressDialog.STYLE_SPINNER);
			mDialog_Scan.setIndeterminate(true);
			mDialog_Scan.setCancelable(true);
			
			mDialog_Scan.setOnCancelListener(new DialogInterface.OnCancelListener()
			{
				public void onCancel(DialogInterface dialog)
				{
					Log.d("Log", "Progress Dialog --- onCancel");
					
					getPlayer().eState	= DxbPlayer.STATE.SCAN_STOP;
					updateToast(getResources().getString(R.string.please_wait_cancel_scanning));
					getPlayer().searchCancel();
					getContext().removeDialog(DIALOG_SCAN);
				}
			});
			
			return mDialog_Scan;
		}
		else if(id == DIALOG_MANUAL_SCAN)
		{
			// This example shows how to add a custom layout to an AlertDialog
			LayoutInflater factory = LayoutInflater.from(DxbOptions.getInstance());
			final View textEntryView = factory.inflate(R.layout.dxb_manual_scan_dialog, null);
			dlg =  new AlertDialog.Builder(DxbOptions.getInstance())
				.setIconAttribute(android.R.attr.alertDialogIcon)
				.setTitle(R.string.manual_scan)
				.setView(textEntryView)
				.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int whichButton) {
						/* User clicked OK so do some stuff */
						EditText frequencyText = (EditText)dlg.findViewById(R.id.frequency_edit);
						int fFrequency = 0;
						if( isValidFreq(frequencyText.getText().toString()) == true ) {
							fFrequency = Integer.parseInt(frequencyText.getText().toString());
							getPlayer().cOption.scan_manual = fFrequency;
							getPlayer().cOption.scan_manual_bandwidth = 6;
							getPlayer().eState = DxbPlayer.STATE.OPTION_MANUAL_SCAN;
							DxbOptions.isbackpressed = true;
							DxbOptions.getInstance().finish();
						}
						else {
							AlertDialog err = new AlertDialog.Builder(DxbOptions.getInstance())
								.setTitle(R.string.manual_scan)
								.setMessage(getResources().getString(R.string.manual_scan_error))
								.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
									public void onClick(final DialogInterface dialog, final int which) {
										
									}
								}).create();

							err.show();
						}
						
						getContext().removeDialog(DIALOG_MANUAL_SCAN);
					}
				})
				.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int whichButton) {
			
						/* User clicked cancel so do some stuff */
						dlg = null;
						getContext().removeDialog(DIALOG_MANUAL_SCAN);
					}
				})
				.setOnCancelListener(new DialogInterface.OnCancelListener() {
					public void onCancel(DialogInterface dialog)
					{
						dlg = null;
						getContext().removeDialog(DIALOG_MANUAL_SCAN);
					}
				})
				.setOnKeyListener(new DialogInterface.OnKeyListener() {
					public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event)
					{
						if(		(keyCode == KeyEvent.KEYCODE_DPAD_DOWN)
							||	(keyCode == KeyEvent.KEYCODE_DPAD_RIGHT)
							||	(keyCode == KeyEvent.KEYCODE_ENTER)
						)
						{
							EditText	editText	= (EditText)dlg.findViewById(R.id.frequency_edit);
							InputMethodManager	imm	= (InputMethodManager)getContext().getSystemService(getContext().INPUT_METHOD_SERVICE);
							imm.hideSoftInputFromWindow(editText.getWindowToken(), 0);
						}
						else if(keyCode == KeyEvent.KEYCODE_DPAD_UP)
						{
							EditText	editText	= (EditText)dlg.findViewById(R.id.frequency_edit);
							InputMethodManager	imm	= (InputMethodManager)getContext().getSystemService(getContext().INPUT_METHOD_SERVICE);
							imm.showSoftInput(editText, InputMethodManager.SHOW_FORCED);
						}
						
						return false;
					}
				})
				.create();

				dlg.setOnShowListener(new DialogInterface.OnShowListener() {
					public void onShow(DialogInterface dialog)
					{
						EditText frequencyText = (EditText)dlg.findViewById(R.id.frequency_edit);
						frequencyText.addTextChangedListener(new InputValidator(frequencyText));
					}
				});
			return dlg;
		}
		return null;
	}

	@Override
	protected String getClassName() {
		return "[[[DxbView_Scan]]]";
	}	
}
