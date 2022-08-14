package com.telechips.android.atsc.samples;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewParent;
import android.view.WindowManager;
import android.graphics.drawable.ColorDrawable;
import android.graphics.Color;
import android.util.Log;

import com.telechips.android.atsc.*;

public class DxbOptions_ATSC extends PreferenceActivity implements
Preference.OnPreferenceChangeListener, Preference.OnPreferenceClickListener
{	
	private static final String TAG = "[[[DxbOptions_ATSC]]]";

	private static final String KEY_CLOSEDCAPTION_SELECT = "closedcaption_select";
	private static final String KEY_CAPTIONSVCNUM_SELECT = "closedcaption_svcnum";
	private static final String KEY_AUDIO_PREF_SELECT = "audio_preference";    
	private static final String KEY_ASPECT_RATIO_SELECT = "aspect_ratio";
	private static final String KEY_INPUT_SELECT = "input_select";
	private static final String KEY_SCAN_SELECT = "scan_select";

	private ListPreference mClosedCaptionSelect;
	private ListPreference mCaptionSvcNumSelect;
	private ListPreference mAudioPrefSelect;
	private ListPreference mAspectRatioSelect;
	private ListPreference mInputSelect;
	private Preference 	mScanSelect;
	
	private boolean isbackpressed;

	static ProgressDialog mDialog = null;
	private static final int DIALOG_SCAN = 0;
	static AlertDialog	mScanDoneDialog = null;
	private static final int DIALOG_SCANDONE = 1;
	static AlertDialog	mScanCancelDialog = null;
	private static final int DIALOG_SCANCANCEL = 2;
	
	private static int nInputSelected = 0;	
	
	private static int nSearchPercent = 0;
	private static int nSearchTVChNum = 0;
	private static int nSearchRadioChNum = 0;
	public static int ScanIsPerformed = 0; // 0: scan is not performed 1: scan is complete/canceled having searched channel 2: scan is complete/canceled with no searched channel
	
	public enum DXB_STATE_OPTION
	{
		GENERAL,
		SCAN,
		SCAN_STOP,
		EXIT,
		NULL
	}
	private DXB_STATE_OPTION	eState_OptionScan		= DXB_STATE_OPTION.NULL;

	@Override
	public void onCreate(Bundle saveInstanceState)
	{
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onCreate()");
		super.onCreate(saveInstanceState);

		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,WindowManager.LayoutParams.FLAG_FULLSCREEN);
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

		addPreferencesFromResource(R.xml.options_atsc);

		ScanIsPerformed = 0;
		eState_OptionScan = DXB_STATE_OPTION.GENERAL;
		setDefault_Value();
		
		getListView().setBackgroundColor(Color.rgb(16, 16, 16));
		isbackpressed = false;
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

		if( DxbPlayer_Control.mPlayer != null )
		{
			DxbPlayer_Control.mPlayer.setOnSearchCompletionListener(searchcompletionlistener);
			DxbPlayer_Control.mPlayer.setOnSearchPercentListener(searchpercentlistener);
			DxbPlayer_Control.mPlayer.setOnSearchDoneInfoListener(searchdoneinfolistener);

			updateState(true);
		}
		else
		{
			Log.d(TAG, "~~~~~~~~~~~~~~~~> onResume() resume");
			View title = (View) findViewById(android.R.id.title);
			title.setVisibility(View.GONE);
			if ( title != null)
			{
				ViewParent parent = title.getParent();
				if ( parent != null && (parent instanceof View))
				{
					View parentView = (View)parent;
					parentView.setBackgroundColor(Color.BLACK);
				}
			}

			getPreferenceScreen().removeAll();
			onBackPressed();
		}
	}	

	@Override
	protected void onPause()
	{
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onPause()");
		super.onPause();

		if(DxbPlayer_Control.intentSubActivity != null && isbackpressed == false)
		{
			Log.d(TAG, "~~~~~~~~~~~~~~~~> onPause() suspend");
			eState_OptionScan	= DXB_STATE_OPTION.EXIT;
			if( mDialog != null && mDialog.isShowing() == true )
			{
				Log.d(TAG, "mDialog="+mDialog);
				Log.d(TAG, "mDialog.isShowing()="+mDialog.isShowing());
				removeDialog(DIALOG_SCAN);
				mDialog = null;
				DxbPlayer_Control.searchCancel();
			}
			if ( mScanDoneDialog != null && mScanDoneDialog.isShowing() == true)
			{
				Log.d(TAG, "mScanDoneDialog.isShowing()="+mScanDoneDialog.isShowing());
				Log.d(TAG, "mScanDoneDialog="+mScanDoneDialog);
				mScanDoneDialog.dismiss();
				mScanDoneDialog = null;
			}

			if ( mScanCancelDialog != null && mScanCancelDialog.isShowing() == true)
			{
				Log.d(TAG, "mScanCancelDialog="+mScanCancelDialog);
				Log.d(TAG, "mScanCancelDialog.isShowing()="+mScanCancelDialog.isShowing());
				mScanCancelDialog.dismiss();
				mScanCancelDialog = null;
			}

			if(  mClosedCaptionSelect.getDialog()!= null && mClosedCaptionSelect.getDialog().isShowing() == true)
			{
				mClosedCaptionSelect.getDialog().dismiss();
			}
			if(  mCaptionSvcNumSelect.getDialog()!= null && mCaptionSvcNumSelect.getDialog().isShowing() == true)
			{
				mCaptionSvcNumSelect.getDialog().dismiss();
			}
			if( mAudioPrefSelect.getDialog()!= null && mAudioPrefSelect.getDialog().isShowing() == true)
			{
				mAudioPrefSelect.getDialog().dismiss();
			}
			if( mAspectRatioSelect.getDialog() != null && mAspectRatioSelect.getDialog().isShowing() == true)
			{
				mAspectRatioSelect.getDialog().dismiss();
			}
			if(  mInputSelect.getDialog()!= null && mInputSelect.getDialog().isShowing() == true)
			{
				mInputSelect.getDialog().dismiss();
			}

			DxbPlayer_Control.releaseSurface();
			DxbPlayer_Control.mPlayer.stop();
			DxbPlayer_Control.mPlayer.release();
			
			DxbPlayer_Control.mPlayer	= null;
		}
	}

	@Override
	public void onStop()
	{
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onStop()");
		super.onStop();

		this.finish();
	}

	@Override
	public void onDestroy()
	{
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onDestroy()");
		if(DxbPlayer_Control.intentSubActivity != null && isbackpressed == false)
		{
			if(DxbPlayer_Control.mChannelManager != null)
			{
				DxbPlayer_Control.mChannelManager.close();
				Log.d(TAG, "~~~~~~~~~~~~~~~~> onDestroy() 0");
				DxbPlayer_Control.mChannelManager = null;
			}
			
			if(DxbPlayer_Control.mDB != null)
			{
				Log.d(TAG, "~~~~~~~~~~~~~~~~> onDestroy()1");
				DxbPlayer_Control.mDB.close();
				Log.d(TAG, "~~~~~~~~~~~~~~~~> onDestroy()2");				
				DxbPlayer_Control.mDB = null;
				Log.d(TAG, "~~~~~~~~~~~~~~~~> onDestroy()3");				
			}
		}
		super.onDestroy();
	}

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        int keyCode = event.getKeyCode();
        if (keyCode == KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE) {
            return true;
        }

        return super.dispatchKeyEvent(event);
    }

	@Override 
	public void onBackPressed()
	{
		Log.d(TAG, "BACK KEY onBackPressed Called");
		isbackpressed = true;
		super.onBackPressed();
	}

	private void updateState(boolean force) 
	{
		updateClosedCaptionSelectSummary(mClosedCaptionSelect.getValue());
		updateCaptionSVCNumSelectSummary(mCaptionSvcNumSelect.getValue());
		updateAudioPrefSelectSummary(mAudioPrefSelect.getValue());
		updateAspectRatioSelectSummary(mAspectRatioSelect.getValue());
		updateInputSelectSummary(mInputSelect.getValue());
	}    

	private void updateClosedCaptionSelectSummary(Object value) 
	{
		CharSequence[] summaries = getResources().getTextArray(R.array.closedcaption_select_summaries);
		CharSequence[] values = mClosedCaptionSelect.getEntryValues();
		for (int i=0; i<values.length; i++) 
		{
			if (values[i].equals(value)) 
			{
				mClosedCaptionSelect.setSummary(summaries[i]);
				break;
			}
		}
	}

	private void updateCaptionSVCNumSelectSummary(Object value) 
	{
		CharSequence[] summaries = getResources().getTextArray(R.array.captionnum_select_summaries);
		CharSequence[] values = mCaptionSvcNumSelect.getEntryValues();
		for (int i=0; i<values.length; i++) 
		{
			if (values[i].equals(value)) 
			{
				mCaptionSvcNumSelect.setSummary(summaries[i]);
				break;
			}
		}
	}

	private void updateAudioPrefSelectSummary(Object value) 
	{
		CharSequence[] summaries = getResources().getTextArray(R.array.audio_pref_select_summaries);
		CharSequence[] values = mAudioPrefSelect.getEntryValues();
		for (int i=0; i<values.length; i++) 
		{
			if (values[i].equals(value)) 
			{
				mAudioPrefSelect.setSummary(summaries[i]);
				break;
			}
		}
	}

	private void updateAspectRatioSelectSummary(Object value) 
	{
		CharSequence[] summaries = getResources().getTextArray(R.array.aspect_ratio_select_summaries);
		CharSequence[] values = mAspectRatioSelect.getEntryValues();
		for (int i=0; i<values.length; i++) 
		{
			if (values[i].equals(value)) 
			{
				mAspectRatioSelect.setSummary(summaries[i]);
				break;
			}
		}
	}

	private void updateInputSelectSummary(Object value) 
	{
		CharSequence[] summaries = getResources().getTextArray(R.array.input_select_summaries);
		CharSequence[] values = mInputSelect.getEntryValues();
		for (int i=0; i<values.length; i++) 
		{
			if (values[i].equals(value)) 
			{
				mInputSelect.setSummary(summaries[i]);
				nInputSelected = i;
				//Log.d(TAG, "~~~~~~~~~~~~~~~~>i"+i +"nInputSelected()"+nInputSelected);
				break;
			}
		}
	}

	ATSCPlayer.OnSearchCompletionListener searchcompletionlistener = new ATSCPlayer.OnSearchCompletionListener()
	{
		public void onSearchCompletion(ATSCPlayer player)
		{
			Log.d(TAG, "onSearchCompletion() eState_OptionScan"+eState_OptionScan);
			if(mDialog != null && mDialog.isShowing() == true)
			{
				removeDialog(DIALOG_SCAN);
				mDialog = null;
			}

			if(eState_OptionScan == DXB_STATE_OPTION.SCAN)
			{
				if( nSearchTVChNum == 0 && nSearchRadioChNum == 0)
					ScanIsPerformed = 2;
				else
					ScanIsPerformed = 1;
				Log.d(TAG, "onSearchCompletion() ScanIsPerformed"+ScanIsPerformed);
				showDialog(DIALOG_SCANDONE);
			}
			else if( eState_OptionScan == DXB_STATE_OPTION.SCAN_STOP)
			{
				if( nSearchTVChNum == 0 && nSearchRadioChNum == 0)
					ScanIsPerformed = 2;
				else
					ScanIsPerformed = 1;
				Log.d(TAG, "onSearchCompletion() ScanIsPerformed"+ScanIsPerformed);
				if( DxbPlayer_Control.intentSubActivity != null)
					showDialog(DIALOG_SCANCANCEL);
			}
			else if ( eState_OptionScan == DXB_STATE_OPTION.EXIT )
			{
				if( nSearchTVChNum == 0 && nSearchRadioChNum == 0)
					ScanIsPerformed = 2;
				else
					ScanIsPerformed = 1;
				Log.d(TAG, "onSearchCompletion() ScanIsPerformed"+ScanIsPerformed);
			}
			eState_OptionScan = DXB_STATE_OPTION.GENERAL;
		}
	};
	
	// ATSCPlayer SearchPercentListener
	ATSCPlayer.OnSearchPercentListener searchpercentlistener = new ATSCPlayer.OnSearchPercentListener()
	{
		public void onSearchPercentUpdate(ATSCPlayer player, int nPercent)
		{
			nSearchPercent = nPercent;
			mDialog.setMessage("Please wait while searching..." + "[ " + nSearchPercent + "% ]\nFound TV [ "
					+ nSearchTVChNum + " ] Radio [ "+ nSearchRadioChNum + " ]");

			Log.d(TAG, "Search " + nPercent + "%");
		}
	};
	
	// ATSCPlayer SearchDoneInfoListener 
	ATSCPlayer.OnSearchDoneInfoListener searchdoneinfolistener = new ATSCPlayer.OnSearchDoneInfoListener()
	{
		public void onSearchDoneInfoUpdate(ATSCPlayer player, int nTVChannel, int nRadioChannel)
		{
			nSearchTVChNum = nTVChannel;
			nSearchRadioChNum = nRadioChannel;
			mDialog.setMessage("Please wait while searching..." + "[ " + nSearchPercent + "% ]\nFound TV [ "
				+ nSearchTVChNum + " ] Radio [ "+ nSearchRadioChNum + " ]");

			Log.d(TAG, "[Searched TV : " + nTVChannel + " Searched Radio :" + nRadioChannel +"]");
		}
	};

	@Override
	protected Dialog onCreateDialog(int id)
	{
		Log.d(TAG, "onCreateDialog()");
		
		switch(id)
		{
			case DIALOG_SCAN:
				{
					//if(mDialog == null)
					{
						mDialog = new ProgressDialog(this);
						mDialog.getWindow().setBackgroundDrawable(new ColorDrawable(0xff343434));
					}
					mDialog.setTitle(getResources().getString(R.string.app_name));			
					mDialog.setMessage(getResources().getString(R.string.please_wait_while_searching)+"[ 0% ]\nFound TV [ 0 ] Radio [ 0 ]");
					mDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
					mDialog.setIndeterminate(true);
					mDialog.setCancelable(true);
					
					mDialog.setOnCancelListener(new DialogInterface.OnCancelListener()
					{
						public void onCancel(DialogInterface dialog)
						{
							Log.d(TAG, "Progress Dialog --- onCancel");
							eState_OptionScan	= DXB_STATE_OPTION.SCAN_STOP;
							if(mDialog != null && eState_OptionScan == DXB_STATE_OPTION.SCAN_STOP)
							{
								removeDialog(DIALOG_SCAN);
								dialog = null;
							}
							DxbPlayer_Control.searchCancel();
						}
					});	
					return mDialog;
				}
			
			case DIALOG_SCANDONE:
				{
					AlertDialog.Builder builder =  new AlertDialog.Builder(this);
					
					builder.setIcon(R.drawable.dxb_portable_atsc_launcher_icon_72);
					builder.setTitle("ATSC Scan is Completed!");
					builder.setMessage("Found TV [ "+nSearchTVChNum + " ] Radio [ "+nSearchRadioChNum + " ]");
					builder.setCancelable(false);
					builder.setPositiveButton("OK", new DialogInterface.OnClickListener() 
					{
						public void onClick(DialogInterface dialog, int which)
						{
							Log.d(TAG, "DIALOG_SCANDONE Dialog --- Positive");
							dialog.dismiss();
							dialog = null;
						}
					});
					mScanDoneDialog = builder.create();
					return mScanDoneDialog;
				}
			
			case DIALOG_SCANCANCEL:
				{
					AlertDialog.Builder builder =  new AlertDialog.Builder(this);
					
					builder.setIcon(R.drawable.dxb_portable_atsc_launcher_icon_72);
					builder.setTitle("ATSC Scan is Canceled!");
					builder.setMessage("Found TV [ "+nSearchTVChNum + " ] Radio [ "+nSearchRadioChNum + " ]");
					builder.setCancelable(false);
					builder.setPositiveButton("OK", new DialogInterface.OnClickListener()
					{
						public void onClick(DialogInterface dialog, int which)
						{
							Log.d(TAG, "DIALOG_SCANCANCEL Dialog --- Positive");
							dialog.dismiss();
							dialog = null;
						}
					});
					mScanCancelDialog = builder.create();
					return mScanCancelDialog;
				}
			
			default:
			break;
		}
		return null;
	}
	
	public boolean onPreferenceClick(Preference preference)
	{
		final String key = preference.getKey();
		
		if(KEY_SCAN_SELECT.equals(key))
		{
			ScanIsPerformed = 0;
			Log.d(TAG, "~~~~~~~~~~~~~~~~> onPreferenceClick()");
			if(eState_OptionScan != DXB_STATE_OPTION.GENERAL)
			{
				Log.d(TAG, "DxbPlayer Channel Scan. (eState_OptionScan="+eState_OptionScan+")");
				return false;
			}
			nSearchPercent = 0;
			nSearchTVChNum = 0;
			nSearchRadioChNum = 0;

			eState_OptionScan = DXB_STATE_OPTION.SCAN;
			DxbPlayer_Control.mChannelManager.deleteAllChannel();
			DxbPlayer_Control.getClearChannel();

			Log.d(TAG, "DxbPlayer_Control.options_input"+ DxbPlayer_Control.options_input);
			DxbPlayer_Control.options_input = nInputSelected;
			Log.d(TAG, "DxbPlayer_Control.options_input"+ DxbPlayer_Control.options_input);
			DxbPlayer_Control.search(DxbPlayer_Control.options_input);

			Log.d(TAG, "() ScanIsPerformed"+ScanIsPerformed);
			showDialog(DIALOG_SCAN);
		}
		return true;
	}

	public boolean onPreferenceChange(Preference preference, Object objValue)
	{
		final String key = preference.getKey();

		if (KEY_CLOSEDCAPTION_SELECT.equals(key))
		{
			//int value = Integer.parseInt((String) objValue);
			try
			{
				updateClosedCaptionSelectSummary(objValue);
			}
			catch (NumberFormatException e)
			{
				//Log.e(TAG, "could not persist screen output select", e);
			}
		}
		else if (KEY_CAPTIONSVCNUM_SELECT.equals(key))
		{
			//int value = Integer.parseInt((String) objValue);
			try
			{
				updateCaptionSVCNumSelectSummary(objValue);
			}
			catch (NumberFormatException e)
			{
				//Log.e(TAG, "could not persist screen output select", e);
			}
		}
		else if (KEY_AUDIO_PREF_SELECT.equals(key))
		{
			//int value = Integer.parseInt((String) objValue);
			try
			{
				updateAudioPrefSelectSummary(objValue);
			}
			catch (NumberFormatException e)
			{
				//Log.e(TAG, "could not persist screen output select", e);
			}
		}		
		else if (KEY_ASPECT_RATIO_SELECT.equals(key))
		{
			//int value = Integer.parseInt((String) objValue);
			try
			{
				updateAspectRatioSelectSummary(objValue);
			}
			catch (NumberFormatException e)
			{
				//Log.e(TAG, "could not persist screen output select", e);
			}
		}			
		else if (KEY_INPUT_SELECT.equals(key))
		{
			//int value = Integer.parseInt((String) objValue);
			try
			{
				updateInputSelectSummary(objValue);
			}
			catch (NumberFormatException e)
			{
				//Log.e(TAG, "could not persist screen output select", e);
			}
		}			
		return true;
	}

	private void setDefault_Value()
	{
		CharSequence[] values;

		mClosedCaptionSelect = (ListPreference) findPreference(DxbPlayer_Control.getKEY_CLOSEDCAPTION_SELECT());
		values = mClosedCaptionSelect.getEntryValues();
		for (int i=0; i<=values.length; i++)
		{
			if(i == values.length)
			{
				mClosedCaptionSelect.setValue(values[DxbPlayer_Control.DEFAULT_VALUE_CLOSEDCAPTION].toString());
			}
			else if (values[i].equals(mClosedCaptionSelect.getValue()))
			{
				break;
			}
		}
		mClosedCaptionSelect.setOnPreferenceChangeListener(this);

		mCaptionSvcNumSelect = (ListPreference) findPreference(DxbPlayer_Control.getKEY_CAPTIONSVCNUM_SELECT());
		values = mCaptionSvcNumSelect.getEntryValues();
		for (int i=0; i<=values.length; i++)
		{
			if(i == values.length)
			{
				mCaptionSvcNumSelect.setValue(values[DxbPlayer_Control.DEFAULT_VALUE_CAPTIONSVCNUM].toString());
			}
			else if (values[i].equals(mCaptionSvcNumSelect.getValue()))
			{
				break;
			}
		}
		mCaptionSvcNumSelect.setOnPreferenceChangeListener(this);

		mAudioPrefSelect = (ListPreference) findPreference(DxbPlayer_Control.getKEY_AUDIOPREF_SELECT());
		values = mAudioPrefSelect.getEntryValues();
		for (int i=0; i<=values.length; i++)
		{
			if(i == values.length)
			{
				mAudioPrefSelect.setValue(values[DxbPlayer_Control.DEFAULT_VALUE_AUDIOPREF].toString());
			}
			else if (values[i].equals(mAudioPrefSelect.getValue()))
			{
				break;
			}
		}		                
		mAudioPrefSelect.setOnPreferenceChangeListener(this);

		mAspectRatioSelect = (ListPreference) findPreference(DxbPlayer_Control.getKEY_ASPECTRATIO_SELECT());
		values = mAspectRatioSelect.getEntryValues();
		for (int i=0; i<=values.length; i++)
		{
			if(i == values.length)
			{
				mAspectRatioSelect.setValue(values[DxbPlayer_Control.DEFAULT_VALUE_ASPECTRATIO].toString());
			}
			else if (values[i].equals(mAspectRatioSelect.getValue()))
			{
				break;
			}
		}
		mAspectRatioSelect.setOnPreferenceChangeListener(this);

		mInputSelect = (ListPreference) findPreference(DxbPlayer_Control.getKEY_INPUT_SELECT());
		values = mInputSelect.getEntryValues();
		for (int i=0; i<=values.length; i++)
		{
			if(i == values.length)
			{
				mInputSelect.setValue(values[DxbPlayer_Control.DEFAULT_VALUE_INPUT].toString());
			}
			else if (values[i].equals(mInputSelect.getValue()))
			{
				break;
			}
		}
		mInputSelect.setOnPreferenceChangeListener(this);
		
		mScanSelect = (Preference) findPreference(DxbPlayer_Control.getKEY_SCAN_SELECT());
		mScanSelect.setOnPreferenceClickListener(this);
	}
}
