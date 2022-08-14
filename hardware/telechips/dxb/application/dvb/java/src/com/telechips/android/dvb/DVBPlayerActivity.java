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

import com.telechips.android.dvb.option.DxbOptions;
import com.telechips.android.dvb.player.DxbPlayer;
import com.telechips.android.dvb.player.DxbPVR.LOCAL_STATE;
import com.telechips.android.dvb.player.DxbPVR.REC_STATE;
import com.telechips.android.dvb.schedule.Alert;
import com.telechips.android.dvb.schedule.DxbScheduler;
import com.telechips.android.dvb.util.DxbStorage;
import com.telechips.android.dvb.resource.*;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.Instrumentation;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;
import android.view.Display;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.Toast;
import android.widget.LinearLayout;
import android.widget.TextView;
import java.util.Locale;

public class DVBPlayerActivity extends Activity implements DxbPlayer.OnChannelChangeListener, DxbPlayer.OnPreparedListener
{
	class Information_
	{
		int	iAlert_state	= Alert.TYPE_ALERT_STOP;
	}
	Information_	gInformation	= new Information_();
	
	private static final String TAG = "[[[DVBPlayerActivity]]] ";

	/*******************************************************************************
	 * Check State
	 *******************************************************************************/
		// Dxb_Player Life Cycle
		public enum DXB_LIFE_CYCLE
		{
			ON_CREATE,
			ON_START,
			ON_RESUME,
			ON_PAUSE,
			ON_STOP,
			ON_DESTROY,
			NULL
		}
		static private DXB_LIFE_CYCLE	eCycle_Life	= DXB_LIFE_CYCLE.ON_DESTROY;
	/*******************************************************************************/

		
	/*******************************************************************************
	 * Copied from MediaPlaybackService in the Music Player app. Should be
	 * public, but isn't.
	 *******************************************************************************/
		private static final String SERVICECMD = "com.android.music.musicservicecommand";
		private static final String CMDNAME    = "command";
		private static final String CMDPAUSE   = "pause";
	/*******************************************************************************/

	public Handler mHandler_Main = new Handler();

	// DxbView State
	public int iCurrView = DxbView.VIEW_NULL;

	public class COMM {
		//	Display
		public int	iDisplayWidth;
		public int	iDisplayHeight;
		public float	fDensity;

		public boolean	isEnable_Video	= false;
	}
	public COMM	cCOMM		= new COMM();

	private DxbView_Indicator mIndicatorView;
	private DxbView_List mListView;
	public DxbView_Full mFullView;
	private DxbView_Teletext mTeletextView;
	private DxbView_EPG mEPGView;
	private DxbView_Debug mDebugView;

	private DxbView_Scan mScan;
	private Toast mToast = null;
	
	private DxbPlayer mPlayer;
	private DxbRemote mRemote;

	public Intent intentOptionActivity;
	public Intent intentScheduleActivity;
	public boolean dishSetupDialogFlag = false;

	private static DVBPlayerActivity mContext;

	//	Audio description
	View	vAD	= null;
	ImageView	ivAD	= null;
	TextView	tvAD	= null;

	public static DVBPlayerActivity getInstance()
	{
		return mContext;
	}

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		Log.i(TAG, "================> onCreate 2014.03.27.");

		super.onCreate(savedInstanceState);
		eCycle_Life = DXB_LIFE_CYCLE.ON_CREATE;
//Set Locale
        Locale locale = new Locale("en");
        Locale.setDefault(locale);

		getWindow().addFlags(// lock 화면 띄우기 전에 자신의 화면을 먼저 띄운다.
							WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED
							// 키잠금 해제하기
							| WindowManager.LayoutParams.FLAG_DISMISS_KEYGUARD
							// 화면 on
							| WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON);
		requestWindowFeature(Window.FEATURE_NO_TITLE);

		setContentView(R.layout.main);

		initSystemSet();
		
		mPlayer = (DxbPlayer)findViewById(R.id.surface_view);
		mPlayer.setOnChannelChangeListener(this);
		mPlayer.setOnPreparedListener(this);

		mRemote = new DxbRemote(this);

		mIndicatorView = new DxbView_Indicator(this, findViewById(R.id.layout_indicator), findViewById(R.id.indicatorBottomView));
		mListView = new DxbView_List(this, findViewById(R.id.layout_list));
		mFullView = new DxbView_Full(this, findViewById(R.id.layout_full));
		mTeletextView = new DxbView_Teletext(this, findViewById(R.id.layout_teletext));
		mEPGView = new DxbView_EPG(this, findViewById(R.id.layout_epg));
		mDebugView = new DxbView_Debug(this, findViewById(R.id.debug));
		mScan = new DxbView_Scan(this);

		setState(true, DxbView.VIEW_MAIN);

		initIntentFilter();

		Bundle bundle	= getIntent().getExtras();
		mPlayer.setBundleData(bundle);
		
		mContext = this;

		getPlayer().isDVBPlayerActivityON = true;
		
		getView().thread_Start();

		getPlayer().setOnPlayingCompletionListener(ListenerOnPlayingCompletion);
		getPlayer().setOnFilePlayTimeUpdateListener(ListenerOnTimeUpdate);
		getPlayer().setOnVideoOutputListener(ListenerOnVideoOutput);
		getPlayer().setOnScrambledStatusListener(ListenerOnScrambled);
	}

	private void setResumeOptionValue()
	{
		Log.i(TAG, "setResumeValue()");

		SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(this);
		String list_value;

		list_value	= pref.getString(DxbOptions.KEY_LANGUAGE_SUBTITLE_SELECT, "");
		if(list_value != "") {
			getPlayer().cOption.language_subtitle	= Integer.parseInt(list_value);
		}

		list_value= pref.getString(DxbOptions.KEY_COUNTRYCODE_SELECT, "");
		if(list_value != "") {
			getPlayer().cOption.countrycode= Integer.parseInt(list_value);
		}

		list_value	= pref.getString(getResources().getString(R.string.key_time_zone_select), "");
		if(list_value != "")
		{
			getPlayer().cOption.time_zone_type	= Integer.parseInt(list_value);
		}

		list_value	= pref.getString(getResources().getString(R.string.key_antenna_power_select), "");
		if(list_value != "")
		{
			getPlayer().cOption.antenna_power	= Integer.parseInt(list_value);
		}

		list_value = pref.getString(DxbOptions.KEY_STORAGE_SELECT, "");
		if(list_value != "") {
			DxbStorage.setStorage(list_value);
		}
	}

	private void initSystemSet()
	{
		Log.i(TAG, "initSystemSet()");
		
		Display display = ((WindowManager)getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();

		cCOMM.iDisplayWidth = display.getWidth();
		cCOMM.iDisplayHeight = display.getHeight();

		DisplayMetrics displayMetrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
		cCOMM.fDensity	= displayMetrics.density;
		
		vAD		= (View)findViewById(R.id.audio_description_button);
		ivAD	= (ImageView)findViewById(R.id.audio_description_icon);
		tvAD	= (TextView)findViewById(R.id.audio_description_ttx);
		
		tvAD.setTextSize(25 * cCOMM.iDisplayWidth / cCOMM.fDensity / 800);
	}

	public DxbPlayer getPlayer() {
		return mPlayer;
	}
	
	@Override
	public void onStart()
	{
		Log.d(TAG, "================> onStart");
		super.onStart();
		eCycle_Life = DXB_LIFE_CYCLE.ON_START;

		if (getPlayer().isOFF)
		{
			getPlayer().isOFF = false;
			
			return;
		}
		
		if ((intentOptionActivity == null) && (intentScheduleActivity == null) && (dishSetupDialogFlag == false))
		{
			int	iCount	= 0;
			while (getPlayer().isValid())
			{
				try
				{
					if(iCount <= 20)
					{
						iCount++;
						Thread.sleep(100);
					}
					else
					{
						Log.e(TAG, "000000 please check ---> DxbPlayer.mPlayer state! 000000");
						getPlayer().stop();
						getPlayer().release();
					}
				}
				catch(InterruptedException e)
				{
					Log.d(TAG, "sleep fail!");
				}
			}
		}
		
		if(		(getPlayer().eState != DxbPlayer.STATE.OPTION_FULL_SCAN)
			&&	(getPlayer().eState != DxbPlayer.STATE.OPTION_MANUAL_SCAN)
			&&	(getPlayer().eState != DxbPlayer.STATE.OPTION_BLIND_SCAN)
			&&	(getPlayer().eState != DxbPlayer.STATE.OPTION_MAKE_PRESET)
		)
		{
			getPlayer().eState	= DxbPlayer.STATE.GENERAL;
		}
	}
	
	@Override
	protected void onResume()
	{
		Log.d(TAG, "================> onResume");
		super.onResume();
		
		if(getPlayer().isFINISH)
		{
			this.finish();
			return;
		}
		
		getPlayer().isDVBPlayerActivityON = true;
		// screen on 을 유지
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		
		eCycle_Life = DXB_LIFE_CYCLE.ON_RESUME;
 		Intent i = new Intent(SERVICECMD);
		i.putExtra(CMDNAME, CMDPAUSE);
		sendBroadcast(i);
		setResumeOptionValue();

		intentScheduleActivity	= null;
		intentOptionActivity = null;
		dishSetupDialogFlag = false;
		
		if(!getPlayer().isValid() && getPlayer().isPlaying())
		{
			getPlayer().openPlayer();
		}
		setIndicatorReset();
		setState(true, iCurrView);
		Log.d(TAG, "==================> onResume End");
	}
	
	@Override
	protected void onPause()
	{
		Log.d(TAG, "================> onPause() --> DxbPlayer.eState="+getPlayer().eState);
		super.onPause();
		eCycle_Life = DXB_LIFE_CYCLE.ON_PAUSE;
		if(getPlayer().eState == DxbPlayer.STATE.SCAN)
		{
			android.os.Process.killProcess(android.os.Process.myPid() );
			return;
		}
		else if(getPlayer().getRecordState() == REC_STATE.RECORDING)
		{
			getPlayer().eState = DxbPlayer.STATE.UI_PAUSE;
			getPlayer().setRecStop();
			return;
		}
		
		if(getAlertState() != Alert.TYPE_ALERT_START)
		{
			getPlayer().releaseSurface();
			if ((intentOptionActivity == null) && (intentScheduleActivity == null) && (dishSetupDialogFlag == false))
			{
		        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

				if(getPlayer().isValid())
				{
					getPlayer().playSubtitle(0);
					getPlayer().stop();
					getPlayer().release();
				}
			}
		}
	}

	@Override
	protected void onStop() {
		Log.d(TAG, "================> onStop");
		getPlayer().isDVBPlayerActivityON = false;
		super.onStop();
		if(eCycle_Life == DXB_LIFE_CYCLE.ON_PAUSE) {
			Log.d(TAG, "================> DXB_LIFE_CYCLE.ON_PAUSE");
			eCycle_Life = DXB_LIFE_CYCLE.ON_STOP;
		}
		getPlayer().eState	= DxbPlayer.STATE.NULL;
	}
	
	@Override
	public void onDestroy()
	{
		Log.d(TAG, "================> onDestroy()");
		getPlayer().isDVBPlayerActivityON = false;
		eCycle_Life = DXB_LIFE_CYCLE.ON_DESTROY;
		if(mReceiverIntent_Schedule != null)
		{
			unregisterReceiver(mReceiverIntent_Schedule);
			mReceiverIntent_Schedule	= null;
		}

		mDebugView.onDestroy();
		getPlayer().onDestroy();
		getView().onDestroy();
		
		//DxbView_Message.onDestroy();
		
		if (mToast != null)
		{
			mToast.cancel();
			mToast = null;
		}
		
		System.gc();
		
		super.onDestroy();
	}
	
	public void finish_APK_message()
	{
		showDialog(DxbDialog.APK_FINISH_ALERT);
	}
	
	private void close_APK_resource()
	{
		if (mToast != null)
		{
			mToast.cancel();
			mToast = null;
		}

		getPlayer().playSubtitle(DxbPlayer._OFF_);
		mIndicatorView.releaseAll();

		findViewById(R.id.full_view).setBackgroundColor(0xFF101010);
		
		showDialog(DxbDialog.APK_FINISH_PROGRESS);
		
		getPlayer().releaseSurface();

		if(getPlayer().isValid())
		{
			getPlayer().playSubtitle(0);
			getPlayer().stop();
			getPlayer().release();
		}

		new Thread()
		{
			public void run()
			{
				if(mReceiverIntent_Schedule != null)
				{
					unregisterReceiver(mReceiverIntent_Schedule);
					mReceiverIntent_Schedule	= null;
				}

				mDebugView.onDestroy();
				getPlayer().onDestroy();
				getView().onDestroy();
				
				try
				{
					Thread.sleep(2000);
				}

				catch(InterruptedException e)
				{
					Log.d(TAG, "sleep fail!");
				}
				
				removeDialog(DxbDialog.APK_FINISH_PROGRESS);
				
				finish();
			}
		}.start();
	}
	
	public DxbPlayer.OnPlayingCompletionListener ListenerOnPlayingCompletion = new DxbPlayer.OnPlayingCompletionListener()
	{
		public void onPlayingCompletion(DxbPlayer player, int type, int state)
		{
			Log.i(TAG, "onPlayingCompletion(type = " + type + ", state = " + state + ")");
			
			getView().PlayingCompletion(player, type, state);
		}
	};
	
	public DxbPlayer.OnFilePlayTimeUpdateListener ListenerOnTimeUpdate = new DxbPlayer.OnFilePlayTimeUpdateListener()
	{
		public void onFilePlayTimeUpdate(DxbPlayer player, int time)
		{
			Log.i(TAG, "onFilePlayTimeUpdate(time=" + time + ")");
			
			if(time == 16)
				DxbView_Full.mRecordView.setVisible(false);

			getView().FilePlayTimeUpdate(player, time);
		}
	};

	public DxbPlayer.OnVideoOutputListener ListenerOnVideoOutput = new DxbPlayer.OnVideoOutputListener()
	{
		public void onVideoOutput(DxbPlayer player)
		{
			Log.i(TAG, "OnVideoOutputListener()");
			
			getView().VideoOutput(player);
		}
	};

	public DxbPlayer.OnScrambledStatusListener ListenerOnScrambled = new DxbPlayer.OnScrambledStatusListener() {
		@Override
		public void onScrambledStatus(DxbPlayer player)
		{
			Log.e(TAG, "channel is scrambled(DVBPlayerActivity)");
			updateScreen();
		}
	};

	@Override
	public boolean dispatchTouchEvent(MotionEvent event)
	{
		if (event.getButtonState() == MotionEvent.BUTTON_SECONDARY)
		{
			if(event.getAction() == MotionEvent.ACTION_DOWN)
			{
				new Thread(new Runnable()
				{
					public void run()
					{
						new Instrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
					}
				}).start();
				
				return true;
			}
		}
		return super.dispatchTouchEvent(event);
	}

	@Override
	public boolean onGenericMotionEvent(MotionEvent event)
	{
		if (getView() != null)
		{
			if (getView().onGenericMotionEvent(event))
				return true;
		}
	    return super.onGenericMotionEvent(event);
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		Log.d(TAG, "onKeyDown(keyCode=" + keyCode + ", event=" + event + ")");
		
		mHandler_Main.removeCallbacks(mListView.mRunnable_KeyEvent);
		if(getPlayer().eState == DxbPlayer.STATE.SCAN_STOP)
		{
			updateToast(getResources().getString(R.string.please_wait_cancel_scanning));
			return true;
		}

		if(event.getAction() == KeyEvent.ACTION_DOWN)
		{
			if(keyCode == KeyEvent.KEYCODE_SEARCH)
			{
				mHandler_Main.removeCallbacks(mRunnable_AD);
				
				getPlayer().setAudioDescription(!getPlayer().cOption.audio_description_on);
				
				if(getPlayer().cOption.audio_description_on)
				{
					ivAD.setVisibility(View.GONE);
				}
				else
				{
					ivAD.setVisibility(View.VISIBLE);
				}
				vAD.setVisibility(View.VISIBLE);
				mHandler_Main.postDelayed(mRunnable_AD, 7000);
				
				return true;
			}
		}
		
		if (getView() != null)
		{
			if (getView().onKeyDown(keyCode, event))
			{
				return true;
			}
		}
		
		return super.onKeyDown(keyCode, event);
	}

	private Runnable mRunnable_AD = new Runnable()
	{
		public void run()
		{
			vAD.setVisibility(View.INVISIBLE);
		}
	};

	public void updateScreen()
	{
		Log.i(TAG, "updateScreen()");
		
		getView().onUpdateScreen();
	}
	
	public void updateSignalStrength(int iStrength)
	{
		getView().onUpdateSignalStrength(iStrength);
	}
	
	public void updateSignalQuality(int iQuality)
	{
		getView().onUpdateSignalQuality(iQuality);
	}
	
	public void updateSignalMessage(String sMessage)
	{
		if(sMessage == null)
		{
			mDebugView.clear();
		}
		else
		{
			mDebugView.display(sMessage);
		}
	}
	
	public void updateEPG_PF()
	{
		Log.i(TAG, "updateEPG_PF()");
		getView().onUpdateEPG_PF();
	}

	public void setState(boolean isRefresh, int iChangeView)
	{
		Log.i(TAG, "setState(eChange_state=" + iChangeView + ", DxbPlayer.eState=" + getPlayer().eState + ")");
		
		if ((!isRefresh) && (iCurrView == iChangeView))
		{
			Log.i(TAG, "Fail : Dxb_setViewState() - state error");
			return;
		}

		switch (iChangeView)
		{
			case DxbView.VIEW_OPTION: //change activity
				getPlayer().releaseSurface();
				intentOptionActivity = new Intent(this, DxbOptions.class);
				startActivity(intentOptionActivity);
				return;
	
			case DxbView.VIEW_SCHEDULE: // change activity
				getPlayer().releaseSurface();
				intentScheduleActivity = new Intent(this, DxbScheduler.class);
				intentScheduleActivity.putExtra("eventInfo", getPlayer().getEventData());
				intentScheduleActivity.putParcelableArrayListExtra("tv_list", getPlayer().getChannelList(0));
				intentScheduleActivity.putParcelableArrayListExtra("radio_list", getPlayer().getChannelList(1));
				startActivity(intentScheduleActivity);
				return;

			case DxbView.VIEW_NULL:
				if(getPlayer().eState == DxbPlayer.STATE.OPTION_MANUAL_SCAN)
				{
					getPlayer().eState = DxbPlayer.STATE.GENERAL;
					mScan.startManual(getPlayer().cOption.scan_manual, getPlayer().cOption.scan_manual_bandwidth);
					return;
				}
				break;
	
			default: // change view
				if(		(iCurrView != DxbView.VIEW_NULL)
					&&	(iCurrView != iChangeView)
				)
				{
					getView().setVisible(false);
				}
				
				iCurrView = iChangeView;
				getView().setVisible(true);
		}
	}


	//scan menu popup
	public void showScanMenu()
	{
		mFullView.showScanDialog();
	}

	private DxbView getView()
	{
		switch (iCurrView)
		{
			case DxbView.VIEW_LIST:
				return mListView;
			case DxbView.VIEW_FULL:
				return mFullView;
			case DxbView.VIEW_TELETEXT:
				return mTeletextView;
			case DxbView.VIEW_EPG:
				return mEPGView;
		}
		
		return null;
	}
	
	DxbView_Scan getScanView()
	{
		return mScan;
	}

	public int getSignalLevel()
	{
		return mIndicatorView.getSignalLevel();
	}

	public void setIndicatorVisible(boolean v)
	{
		mIndicatorView.setVisible(v);
	}
	
	public void setIndicatorReset()
	{
		mIndicatorView.reset();
	}

	Dialog dlg = null;
	protected Dialog onCreateDialog(int id)
	{
		//DxbLog(I, "onCreateDialog()");

		if(id == DxbDialog.APK_FINISH_ALERT)
		{
			AlertDialog.Builder	ad	= new AlertDialog.Builder(this);
			ad.setTitle(this.getResources().getString(R.string.app_name));
			ad.setMessage(this.getResources().getString(R.string.are_you_sure_to_exit_DVB));
			ad.setPositiveButton(this.getResources().getString(R.string.yes), new OnClickListener()
				{
					public void onClick(DialogInterface arg0, int arg1)
					{
						close_APK_resource();
					}
				});
			ad.setNegativeButton(this.getResources().getString(R.string.no), new OnClickListener()
				{
					public void onClick(DialogInterface arg0, int arg1)
					{
						removeDialog(DxbDialog.APK_FINISH_ALERT);
					}
				});
			
			return ad.create();
		}
		else if(id == DxbDialog.APK_FINISH_PROGRESS)
		{
			ProgressDialog	pd	= new ProgressDialog(this);
			pd.setTitle(this.getResources().getString(R.string.app_name));
			pd.setMessage(this.getString(R.string.please_wait_DVB_is_shutting_off));
			pd.setIndeterminate(true);
			pd.setCancelable(false);
			
			return pd;
		}

		dlg = mScan.onCreateDialog(id);
		if (dlg != null)
		{
			return dlg;
		}
		dlg = getView().onCreateDialog(id);
		if (dlg != null)
		{
			return dlg;
		}
		return null;
	}

	private void initIntentFilter()
	{
		Log.i(TAG, "initIntentFilter()");
		//	Scheduler
		{
			IntentFilter	commandFilter	= new IntentFilter();

			commandFilter.addAction(Alert.ACTION_ALERT_STATE);
			commandFilter.addAction(Alert.ACTION_CHANGE_CHANNEL);
			commandFilter.addAction(Alert.ACTION_REC_STOP);
			commandFilter.addAction(Alert.ACTION_DXB_PLAYING_REQ);
			
			registerReceiver(mReceiverIntent_Schedule, commandFilter);
		}
	}

	private BroadcastReceiver mReceiverIntent_Schedule = new BroadcastReceiver()
	{
		@Override
		public void onReceive(Context context, Intent intent)
		{
			Log.i(TAG, "onReceive()");

			String action = intent.getAction();
			if(Alert.ACTION_ALERT_STATE.equals(action))
			{
				Log.d(TAG, "if(Alert.ACTION_IS_ALERT.equals(action))");
				
				int iAlert_state	= intent.getIntExtra(Alert.EXTRA_TYPE_ALERT_STATE, Alert.TYPE_ALERT_STOP);
				setAlertState(iAlert_state);
			}
			else if(Alert.ACTION_CHANGE_CHANNEL.equals(action))
			{
				Log.d(TAG, "if(Alert.ACTION_CHANGE_CHANNEL.equals(action))");
				
				int	iAlarm_Type		= intent.getIntExtra(Alert.EXTRA_TYPE_ALARM, 0);
	
				int	iChannel_Type	= intent.getIntExtra(Alert.EXTRA_TYPE_CHANNEL_TYPE, 0);
				int	iChannel_Index	= intent.getIntExtra(Alert.EXTRA_TYPE_CHANNEL_INDEX, 0);
				
				int	iID				= intent.getIntExtra(Alert.EXTRA_TYPE_ID, 0);
				int	iRepeat_Type	= intent.getIntExtra(Alert.EXTRA_TYPE_REPEAT, 0);

				{
					mPlayer.setSchedulerInfo_AlarmType(iAlarm_Type);
					mPlayer.setSchedulerInfo_ChannelType(-1);
					mPlayer.setSchedulerInfo_ChannelIndex(-1);
					mPlayer.setSchedulerInfo_AlarmID(iID);
					mPlayer.setSchedulerInfo_RepeatType(iRepeat_Type);

					setState(true, DxbView.VIEW_FULL);
					getPlayer().setServiceType(iChannel_Type);
					getPlayer().setChannel(iChannel_Index);
				}
			}
			else if(Alert.ACTION_REC_STOP.equals(action))
			{
				Log.d(TAG, "if(Alert.ACTION_STOP_REC.equals(action))");
				
				if(getPlayer().getRecordState() == REC_STATE.RECORDING) {
					mFullView.startRecord(-1, -1);
				}
			}
			else if(Alert.ACTION_DXB_PLAYING_REQ.equals(action))
			{
				Log.d(TAG, "else if(Alert.ACTION_DXB_PLAYING_REQ.equals(action))");
				
				Intent	intentReq	= new Intent(Alert.ACTION_DXB_PLAYING_RES);
				sendBroadcast(intentReq);
			}
		}
	};
	
	private void setAlertState(int state)
	{
		Log.i(TAG, "setAlertState(state=" + state + ")");
		Log.d(TAG, "gInformation.iAlert_state=" + gInformation.iAlert_state);
		
		if(state == Alert.TYPE_ALERT_START)
		{
			if(		(getPlayer().getRecordState() == REC_STATE.RECORDING)
				||	(getPlayer().getRecordState() == REC_STATE.RECORDING_ALARM)
			)
			{
				getPlayer().eState = DxbPlayer.STATE.UI_PAUSE;
				getPlayer().setRecStop();
			}
			
			if (getPlayer().getLocalPlayState() != LOCAL_STATE.STOP)
			{
				getPlayer().stop();
			}
		}
		
		if(		(state == Alert.TYPE_ALERT_PAUSE)
			&&	(gInformation.iAlert_state == Alert.TYPE_ALERT_START)
		)
		{
			getPlayer().releaseSurface();

			if(getPlayer().isValid())
			{
				getPlayer().playSubtitle(0);
				getPlayer().stop();
				getPlayer().release();
			}
		}
		
		gInformation.iAlert_state	= state;
	}
	
	private int getAlertState()
	{
		Log.i(TAG, "isAlert() --> gInformation.iAlert_state=" + gInformation.iAlert_state);
		
		return gInformation.iAlert_state;
	}

	public void onChannelChange()
	{
		getView().onChannelChange();
	
		if(		(mPlayer.getSchedulerInfo_AlarmType() == Alert.TYPE_ALARM_NOTICE_REC)
			&&	(mPlayer.getSchedulerInfo_RepeatType() >= 0)
			&&	(mPlayer.getSchedulerInfo_AlarmID() >= 0)
		)
		{
			mFullView.startRecord(mPlayer.getSchedulerInfo_AlarmID(), mPlayer.getSchedulerInfo_RepeatType());
			
			mPlayer.setSchedulerInfo_AlarmType(-1);
			mPlayer.setSchedulerInfo_RepeatType(-1);
			mPlayer.setSchedulerInfo_AlarmID(-1);
		}
	}

	public void updateChannelList()
	{
		Log.i(TAG, "updateChannelList()");
		
		getView().setChannel();
		mListView.updateChannelList();
		mFullView.onUpdateScreen();
	}

	public void updateToast(String message) {
//		DxbLog(I, "updateToast(" + message + ")");
		if(mToast == null) {
			mToast = Toast.makeText(this, message, 2000);

			LinearLayout linearLayout = (LinearLayout)mToast.getView();
			View child = linearLayout.getChildAt(0);
			TextView messageTextView = (TextView)child;
			messageTextView.setTextSize(25);
		} else {
			mToast.show();
			mToast.setText(message);
		}
		mToast.show();
	}

	public void onPrepared(DxbPlayer player) {
		if (player.eState == DxbPlayer.STATE.OPTION_MANUAL_SCAN)
		{
			player.eState = DxbPlayer.STATE.GENERAL;
			mScan.startManual(player.cOption.scan_manual, player.cOption.scan_manual_bandwidth);
			return;
		}
		else if(player.eState == DxbPlayer.STATE.OPTION_BLIND_SCAN)
		{
			player.eState = DxbPlayer.STATE.GENERAL;
			mScan.startBlindScan();
			return;
		}
		else if(player.eState == DxbPlayer.STATE.OPTION_FULL_SCAN)
		{
			player.eState = DxbPlayer.STATE.GENERAL;
			mScan.startFull();
			return;
		}
		if (player.getServiceType() != 2)
		{
			if (player.setChannel())
				return;
		}
		getView().onUpdateScreen();
	}
	
	public DxbView_EPG getEPGView()
	{
		return mEPGView;
	}
}
