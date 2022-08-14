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

package com.telechips.android.dvb;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.database.Cursor;
import android.graphics.drawable.ColorDrawable;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnFocusChangeListener;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;

import com.telechips.android.dvb.option.DxbOptions;
import com.telechips.android.dvb.player.*;
import com.telechips.android.dvb.player.diseqc.DishSetupManager;
import com.telechips.android.dvb.player.dvb.TuneSpace;

public class DxbView_Scan extends DxbView
{
	private ProgressDialog mDialog_Scan;
	
	public DxbView_Scan(Context context)
	{
		super(context);
		
		getPlayer().setOnSearchPercentListener(ListenerOnSearchPercent);
		getPlayer().setOnSearchCompletionListener(ListenerOnSearchCompletion);
		getPlayer().setOnBlindScanPercentListener(ListenerOnBlindScanPercent);
		getPlayer().setOnBlindScanCompletionListener(ListenerOnBlindScanCompletion);
		getPlayer().setOnDBInformationUpdateListener(ListenerOnDBInformationUpdateListener);
	}

	private DxbPlayer.OnSearchPercentListener ListenerOnSearchPercent = new DxbPlayer.OnSearchPercentListener()
	{
		public void onSearchPercentUpdate(DxbPlayer player, int nPercent)
		{
			//DxbLog(I, "ListenerOnSearchPercent --> onSearchPercentUpdate(" + nPercent + ")");
			
			String	return_Count = "  TV : " + getPlayer().getChannel_Count_DB(0) + ", Radio : " + getPlayer().getChannel_Count_DB(1);

				mDialog_Scan.setMessage("Please wait while searching..."
											+ "( " + (getPlayer().getSearchFreqKhz() / 1000) + " Mhz )\n"
											+ "[ " + nPercent + "% ]"
											+ return_Count
										);
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
				getPlayer().setChannelList();
				getContext().updateChannelList();
			}
		}
	};
	
	String sScan_TransPonder[]	= new String[10];
	int	iScan_Index	= 0;
	int	iScan_Count	= 0;
	final int iScan_Max	= 10;
	private DxbPlayer.OnBlindScanPercentListener ListenerOnBlindScanPercent = new DxbPlayer.OnBlindScanPercentListener()
	{
		public void onBlindScanPercentUpdate(DxbPlayer player, int nPercent, int index, int freqMHz, int symbolKHz)
		{
			String	msg	=	"Please wait while searching..." + "     ( " + nPercent + " % )\n\n";
							
			int		iPolarisation	= getPlayer().getPolarization(index);
			if( (freqMHz>0) && (symbolKHz>0) )
			{
				Cursor	cTs	= mDishManager.queryTsInfo(
						new String[] {"_id", "frequency", "symbol", "polarisation", "fec_inner"}
						, "sat_id=" + getPlayer().getSatelliteID() + " AND frequency=" + freqMHz + " AND symbol=" + symbolKHz + " AND polarisation=" + iPolarisation
						, null
				);
				
				int	iCount_Ts	= cTs.getCount();
				if(iCount_Ts <= 0)
				{
					Cursor	cList	= mDishManager.queryTsInfo(
							new String[] {"_id", "ts_id", "frequency", "symbol", "polarisation", "fec_inner"}
							, "sat_id=" + getPlayer().getSatelliteID()
							, null
							, "ts_id desc"
					);
					
					int	iID	= 0;
					if(cList.getCount() > 0)
					{
						cList.moveToFirst();
						iID	= cList.getInt(cList.getColumnIndex("ts_id")) + 1;
					}
					cList.close();
					
					ContentValues values = new ContentValues();
					
					values.put("ts_id", iID);
					values.put("frequency", Integer.toString(freqMHz));
					values.put("symbol", Integer.toString(symbolKHz));
					values.put("polarisation", iPolarisation);
					
					// set_default sat_lnb_info
					values.put("sat_id", getPlayer().getSatelliteID());
					values.put("fec_inner", 0);
					
					mDishManager.insertTsInfo(values);
				}
				cTs.close();
				
				sScan_TransPonder[iScan_Index]	=	String.format("%03d   ", (iScan_Count+1)) +
													freqMHz + " MHz   " +
													symbolKHz + " KHz   " +
													((getPlayer().getPolarization(index)==0)?"H":"V");
				
				iScan_Count++;
				iScan_Index = (iScan_Index+1) % iScan_Max;
			}
												
			for(int j=0 ; j<iScan_Max ; j++)
			{
				if(j >= iScan_Count)
				{
					break;
				}
				
				int	iIndex_display;
				if(iScan_Count < iScan_Max)
				{
					iIndex_display	= j;
				}
				else
				{
					iIndex_display	= (iScan_Index + j) % iScan_Max;
				}
				
				msg	=	msg + sScan_TransPonder[iIndex_display] + "\n";
			}
			
			mDialog_Scan.setMessage(msg);
		}
	};
	
	private DxbPlayer.OnBlindScanCompletionListener ListenerOnBlindScanCompletion = new DxbPlayer.OnBlindScanCompletionListener()
	{
		public void onBlindScanCompletion(DxbPlayer player)
		{
			DxbLog(I, "onBlindScanhCompletion()");
			if(getPlayer().eState != DxbPlayer.STATE.UI_PAUSE)
			{
				if (mDishManager != null)
				{
					mDishManager.close();
					mDishManager	= null;
				}

				if(!isCancel_Scan)
				{
					mDialog_Scan.setTitle(	getResources().getString(R.string.channel_search) + " - " + getPlayer().getSatelliteName());
					getPlayer().search();
				}
				else
				{
					getContext().setIndicatorReset();
					getPlayer().eState	= DxbPlayer.STATE.GENERAL;
				}
			}
		}
	};
	
	private DxbPlayer.OnDBInformationUpdateListener ListenerOnDBInformationUpdateListener = new DxbPlayer.OnDBInformationUpdateListener()
	{
		public void onDBInformationUpdate(DxbPlayer player, int type, int param)
		{
			//Log.e("TAG", "onDBInformationUpdate Playing cursorPosition ==> " + Manager_Setting.g_Information.cCOMM.iCurrent_TV);
			
			if(getPlayer().getServiceType() == 2)
			{
				return;
			}
			
			switch(param)
			{
				case 0x55:	// (85)parental_rating_descriptor
					if((type & 1) == 0)	// type : 0-PF, 1-schedule
					{
						int epg_type = (type & 2) >> 1;	// epg_type	: 0-p, 1-f
						int rating = (type >> 8) & 0xFF;
						//DxbLog(D, "¡Ù¡Ù¡ÙListenerOnDBInformationUpdateListener --> epg_type=" + epg_type + ", rating=" + rating  + ", param=" + param + ")");
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

	public void startFull()
	{
		DxbLog(I, "startFull() DxbPlayer.eState ="+ getPlayer().eState);
		
		if(getPlayer().eState != DxbPlayer.STATE.GENERAL)
		{
			updateToast(getResources().getString(R.string.please_wait_cancel_scanning));
			return;
		}

		if (getPlayer().search())
		{
			getContext().showDialog(DIALOG_SCAN_DEFAULT);
		}
	}
	
	public void startManual(int KHz_frequency, int Mhz_bandwidth)
	{
		DxbLog(I, "startManual() DxbPlayer.eState ="+ getPlayer().eState);
		
		if(getPlayer().eState != DxbPlayer.STATE.GENERAL)
		{
			updateToast(getResources().getString(R.string.please_wait_cancel_scanning));
			return;
		}

		if (getPlayer().manualScan(KHz_frequency, Mhz_bandwidth))
		{
			getContext().showDialog(DIALOG_SCAN_DEFAULT);
		}
	}
	
	private DishSetupManager mDishManager = null;;
	public void startBlindScan()
	{
		DxbLog(I, "startBlindScan() DxbPlayer.eState ="+ getPlayer().eState);
		
		if(getPlayer().eState != DxbPlayer.STATE.GENERAL)
		{
			updateToast(getResources().getString(R.string.please_wait_cancel_scanning));
			return;
		}

		if (getPlayer().blindScan()) {
			mDishManager = new DishSetupManager(getContext());
			mDishManager.open();
			iScan_Index	= 0;
			iScan_Count	= 0;
			
			getContext().showDialog(DIALOG_SCAN_BLIND);
		}
	}
	
	public void cancel() {
		DxbLog(I, "cancel()");
		if(getPlayer().isValid()) {
			getPlayer().searchCancel();
		}
	}

	private void removeDialog()
	{
		if(mDialog_Scan!=null && mDialog_Scan.isShowing())
		{
			DVBPlayerActivity.getInstance().removeDialog(DIALOG_SCAN);
		}
	}
	
	Dialog dlg = null;
	View textEntryView;
	TextView	tvChannel;
	ImageView tvChannelLeft;
	ImageView tvChannelRight;

	int	iIndex_Channel;
	EditText	editFrequency;
	RadioGroup	rgBandwidth;
	TextView	tvRF;
	TextView	tvQuality;
	
	private static Context mScanDialog;
	public static int DIALOG_SCAN;
	public static int DIALOG_MANUAL;
	private boolean isCancel_Scan	= false;
	public Dialog onCreateDialog(final int id)
	{
		if(	(id == DIALOG_SCAN_DEFAULT) || (id == DIALOG_SCAN_BLIND) )
		{
			isCancel_Scan	= false;
			DIALOG_SCAN	= id;
			
			//if(mDialog == null)
			{
				mDialog_Scan = new ProgressDialog(getContext());
				mDialog_Scan.getWindow().setBackgroundDrawable(new ColorDrawable(0xff343434));
			}
			
			switch(id)
			{
				case DIALOG_SCAN_DEFAULT:
					if(getPlayer().isDVB_S2())
					{
						mDialog_Scan.setTitle(	getResources().getString(R.string.channel_search) + " - " +
								getPlayer().getSatelliteName());
					}
					else
					{
						mDialog_Scan.setTitle(	getResources().getString(R.string.channel_search));
					}
				break;
				
				case DIALOG_SCAN_BLIND:
					iScan_Index	= 0;
					mDialog_Scan.setTitle(	getResources().getString(R.string.channel_search) + " - " +
											getPlayer().getSatelliteName() + " - " +
											getResources().getString(R.string.trans_ponder));
				break;
			}
			
			mDialog_Scan.setMessage(getResources().getString(R.string.please_wait_while_searching));
			mDialog_Scan.setProgressStyle(ProgressDialog.STYLE_SPINNER);
			mDialog_Scan.setIndeterminate(true);
			mDialog_Scan.setCancelable(true);
			
			mDialog_Scan.setOnCancelListener(new DialogInterface.OnCancelListener()
			{
				public void onCancel(DialogInterface dialog)
				{
					Log.d("Log", "Progress Dialog --- onCancel");
					
					updateToast(getResources().getString(R.string.please_wait_cancel_scanning));
					getPlayer().searchCancel();
					isCancel_Scan	= true;
					getContext().removeDialog(DIALOG_SCAN);
				}
			});
			ListenerOnSearchPercent.onSearchPercentUpdate(getPlayer(), 0);

			return mDialog_Scan;
		}
		else if(id == DIALOG_SCAN_MANUAL || id == DIALOG_MANUAL_SCAN_OPTION)
		{
			getPlayer().stop();

			// choice manual scan in opotion or dxbView
			switch(id)
			{
				case DIALOG_SCAN_MANUAL :
					DIALOG_MANUAL = DIALOG_SCAN_MANUAL;
					mScanDialog = getContext();
				break;

				case DIALOG_MANUAL_SCAN_OPTION :
					DIALOG_MANUAL = DIALOG_MANUAL_SCAN_OPTION;
					mScanDialog = DxbOptions.getInstance();
				break;
			}
			CharSequence[] summaries = getResources().getTextArray(R.array.countrycode_select_summaries);
			
			// This example shows how to add a custom layout to an AlertDialog
			//LayoutInflater factory = LayoutInflater.from(DxbOptions.getInstance());
			//textEntryView = factory.inflate(R.layout.dxb_manual_scan_dialog, null);
			textEntryView = View.inflate(mScanDialog, R.layout.dxb_manual_scan_dialog, null);
			dlg =  new AlertDialog.Builder(mScanDialog)
			.setIconAttribute(android.R.attr.alertDialogIcon)
			.setTitle(getResources().getString(R.string.manual_scan) + "  -  " + summaries[getPlayer().cOption.countrycode])
			.setView(textEntryView)
			.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener()
			{
				public void onClick(DialogInterface dialog, int whichButton)
				{
					/* User clicked OK so do some stuff */
					EditText frequencyText = (EditText)dlg.findViewById(R.id.frequency_edit);
					RadioGroup bandwidthRG = (RadioGroup)dlg.findViewById(R.id.bandwidth_rg);
					if(frequencyText.getText().toString().length() > 0 && bandwidthRG != null)
					{
						int id_bandwidth = bandwidthRG.getCheckedRadioButtonId();
						RadioButton rd = (RadioButton)dlg.findViewById(id_bandwidth);
						if(rd != null)
						{
							int fFrequency	= Integer.parseInt(frequencyText.getText().toString());
							getPlayer().cOption.scan_manual	= fFrequency;
							switch(id_bandwidth)
							{
							case R.id.six_mhz:
								getPlayer().cOption.scan_manual_bandwidth = 6;
								break;
								
							case R.id.seven_mhz:
								getPlayer().cOption.scan_manual_bandwidth = 7;
								break;
								
							case R.id.eight_mhz:
								getPlayer().cOption.scan_manual_bandwidth = 8;
								break;
							}

							getPlayer().eState = DxbPlayer.STATE.OPTION_MANUAL_SCAN;
							//in order to parent stop/release player
							DxbOptions.isbackpressed = true;
							if(id == DIALOG_SCAN_MANUAL)
							{
								getContext().setState(true, DxbView.VIEW_NULL);
							}
							else
							{
								((Activity) mScanDialog).finish();
							}
						}
					}
					dlg = null;
					getContext().removeDialog(DIALOG_MANUAL);
				}
			})
			.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener()
			{
				public void onClick(DialogInterface dialog, int whichButton)
				{
					/* User clicked cancel so do some stuff */
					dlg = null;
					getContext().removeDialog(DIALOG_MANUAL);
					getPlayer().setChannel();
				}
			})
			.setOnCancelListener(new DialogInterface.OnCancelListener()
			{
				public void onCancel(DialogInterface dialog)
				{
					dlg = null;
					getContext().removeDialog(DIALOG_MANUAL);
					getPlayer().setChannel();
				}
			})
			.setOnKeyListener(new DialogInterface.OnKeyListener()
			{
				public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event)
				{
					DxbLog(D, "keyCode = " + keyCode);
					DxbLog(D, "event.getAction() = " + event.getAction());
					DxbLog(D, "tvChannel.isFocused() = " + tvChannel.isFocused());
					DxbLog(D, "editFrequency.isFocused() = " + editFrequency.isFocused());
					
					if(		!editFrequency.isFocused()
							&&	(event.getAction() == KeyEvent.ACTION_UP)
							&&	(		(keyCode == KeyEvent.KEYCODE_DPAD_UP)
									||	(keyCode == KeyEvent.KEYCODE_DPAD_DOWN)
									||	(keyCode == KeyEvent.KEYCODE_ENTER)
									)
							)
					{
						EditText	editText	= (EditText)dlg.findViewById(R.id.frequency_edit);
						InputMethodManager	imm	= (InputMethodManager)getContext().getSystemService(getContext().INPUT_METHOD_SERVICE);
						imm.hideSoftInputFromWindow(editText.getWindowToken(), 0);
					}
					else if(	(event.getAction() == KeyEvent.ACTION_UP)
							&&	(		(keyCode == KeyEvent.KEYCODE_DPAD_UP)
									||	(keyCode == KeyEvent.KEYCODE_DPAD_DOWN)
									)
							)
					{
						if(editFrequency.isFocused())
						{
							EditText	editText	= (EditText)dlg.findViewById(R.id.frequency_edit);
							InputMethodManager	imm	= (InputMethodManager)getContext().getSystemService(getContext().INPUT_METHOD_SERVICE);
							imm.showSoftInput(editText, InputMethodManager.SHOW_FORCED);
						}
						else
						{
							EditText	editText	= (EditText)dlg.findViewById(R.id.frequency_edit);
							InputMethodManager	imm	= (InputMethodManager)getContext().getSystemService(getContext().INPUT_METHOD_SERVICE);
							imm.hideSoftInputFromWindow(editText.getWindowToken(), 0);
						}
					}
					else if(tvChannel.isFocused() && event.getAction() == KeyEvent.ACTION_DOWN)
					{
						if(keyCode == KeyEvent.KEYCODE_DPAD_LEFT)
						{
							setManualScan_Channel(-1);
							return true;
						}
						else if(keyCode == KeyEvent.KEYCODE_DPAD_RIGHT)
						{
							setManualScan_Channel(1);
							return true;
						}
					}
					
					return false;
				}
			})
			.create();

			iIndex_Channel	= 0;
			tvChannel	= (TextView)textEntryView.findViewById(R.id.manual_scan_channel_index);
			editFrequency	= (EditText)textEntryView.findViewById(R.id.frequency_edit);
			rgBandwidth		= (RadioGroup)textEntryView.findViewById(R.id.bandwidth_rg);
			
			tvChannelLeft = (ImageView)textEntryView.findViewById(R.id.manual_scan_channel_left);
			tvChannelRight = (ImageView)textEntryView.findViewById(R.id.manual_scan_channel_right);
			
			tvRF		= (TextView)textEntryView.findViewById(R.id.rf_value);
			tvQuality	= (TextView)textEntryView.findViewById(R.id.quality_value);
			
			tvChannelLeft.setOnClickListener(new ImageView.OnClickListener() {
				@Override
				public void onClick(View v) {
					setManualScan_Channel(-1);
				}
			});

			tvChannelRight.setOnClickListener(new ImageView.OnClickListener() {
				@Override
				public void onClick(View v) {
					setManualScan_Channel(1);
				}
			});

			setManualScan_Channel(0);

			tvChannel.setOnFocusChangeListener(ListenerOnFocusChange);
			
			return dlg;
		}
		return null;
	}
	
	private void setManualScan_Channel(int iOperation)
	{
		DxbLog(I, "setManualScan_Channel(" + iOperation + ")");
		getMainHandler().removeCallbacks(mRunnable_checkSignal);
		
		TuneSpace	tune	= getPlayer().getTuneSpace();
		
		if(iOperation != 0)
		{
			iIndex_Channel	= (iIndex_Channel + tune.numbers + iOperation) % tune.numbers;
		}
		
		while(tune.channels[iIndex_Channel].frequency1 <= 0)
		{
			if(iOperation < 0)
			{
				iIndex_Channel	= (iIndex_Channel + tune.numbers - 1) % tune.numbers;
			}
			else
			{
				iIndex_Channel	= (iIndex_Channel + 1) % tune.numbers;
			}
		}
		
		tvChannel.setText(tune.channels[iIndex_Channel].name);
		editFrequency.setText(Integer.toString(tune.channels[iIndex_Channel].frequency1));
		
		int	iID = 0;
		int bw = (tune.channels[iIndex_Channel].bandwidth1>0)? tune.channels[iIndex_Channel].bandwidth1 : tune.typicalBandwidth;
		switch(bw)
		{
			case 6:
				iID	= R.id.six_mhz;
			break;
			
			case 7:
				iID	= R.id.seven_mhz;
			break;
			
			case 8:
				iID	= R.id.eight_mhz;
			break;
		}
		rgBandwidth.check(iID);		
		
		getPlayer().setFrequency(getPlayer().cOption.countrycode, iIndex_Channel);

		set_RF_strenght_quality(0, 0);
		getMainHandler().postDelayed(mRunnable_checkSignal, 1000);
	}
	
	private void set_RF_strenght_quality(int dBm, int percentage)
	{
		if(dBm==0 || percentage<=0)
		{
			tvRF.setText(R.string.default_dBm);
			tvQuality.setText( R.string.default_percentage);
		}
		else
		{
			tvRF.setText(dBm + getResources().getString(R.string.dBm));
			tvQuality.setText( percentage + getResources().getString(R.string.percentage));
		}
	}
	
	private Runnable mRunnable_checkSignal = new Runnable()
	{
		public void run()
		{
//			DxbLog(I, "mRunnable_checkSignal --> run()");
			
			getMainHandler().removeCallbacks(mRunnable_checkSignal);
			if(dlg == null)
			{
				return;
			}
			
			if(dlg.isShowing())
			{
				getPlayer().getSignalLevel();
				set_RF_strenght_quality(getPlayer().getSignalStrength(), getPlayer().getSignalQuality());
				
				getMainHandler().postDelayed(mRunnable_checkSignal, 1000);
			}
		}
	};
	
	OnFocusChangeListener ListenerOnFocusChange = new OnFocusChangeListener()
	{
		public void onFocusChange(View v, boolean hasFocus)
		{
			DxbLog(I, "ListenerOnFocusChange-->onFocusChange(hasFocus="+hasFocus+")");
			
			int id = v.getId();
			switch(id)
			{
				case R.id.manual_scan_channel_index:
				{
					if(hasFocus)
					{
						tvChannel.setTextColor(getResources().getColor(R.color.ics_focus));
					}
					else
					{
						tvChannel.setTextColor(getResources().getColor(R.color.color_white));
					}
					
				}
			}
		}
	};

	@Override
	protected String getClassName() {
		return "[[[DxbView_Scan]]]";
	}
}
