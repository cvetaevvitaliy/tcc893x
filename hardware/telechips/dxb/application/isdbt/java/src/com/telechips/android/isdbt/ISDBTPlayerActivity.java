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

import com.telechips.android.isdbt.player.DxbPlayer;
import com.telechips.android.isdbt.player.isdbt.EWSData;
import com.telechips.android.isdbt.schedule.Alert;
import com.telechips.android.isdbt.schedule.DxbScheduler;
import com.telechips.android.isdbt.util.DxbStorage;
import com.telechips.android.isdbt.resource.*;

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
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Toast;
import android.widget.LinearLayout;
import android.widget.TextView;

public class ISDBTPlayerActivity extends Activity implements DxbPlayer.OnChannelChangeListener, DxbPlayer.OnAreaChangeListener, DxbPlayer.OnPreparedListener {

	private class SCHEDULER {
		public int Alarm_Type;
		
		public int Channel_Type;
		public int Channel_Index;
		
		public int Repeat_Type;
		public int Alarm_ID;
	}
	private SCHEDULER Scheduler_Info = new SCHEDULER();

	class Information_ {
		int	iAlert_state	= Alert.TYPE_ALERT_STOP;
	}
	Information_	gInformation	= new Information_();
	
	private static final String TAG = "[[[ISDBTPlayerActivity]]] ";

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
	//private DxbView_EPG mEPGView;
	private DxbView_EPG_3List mEPGView;
	private DxbView_Debug mDebugView;

	private DxbView_Scan mScan;
	private Toast mToast = null;
	private Toast mSQInfoToast = null;
	
	private DxbPlayer mPlayer;
	private DxbRemote mRemote;

	public Intent intentOptionActivity;
	public Intent intentScheduleActivity;

	private static ISDBTPlayerActivity mContext;

	public static ISDBTPlayerActivity getInstance() {
		return mContext;
	}

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		Log.i(TAG, "================> onCreate 2013.04.04.");

		super.onCreate(savedInstanceState);
		eCycle_Life = DXB_LIFE_CYCLE.ON_CREATE;

		getWindow().addFlags(WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED
							| WindowManager.LayoutParams.FLAG_DISMISS_KEYGUARD
							| WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON
							| WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON);
		requestWindowFeature(Window.FEATURE_NO_TITLE);

		setContentView(R.layout.main);

		initSystemSet();
		
		mPlayer = (DxbPlayer)findViewById(R.id.surface_view);
		mPlayer.setOnChannelChangeListener(this);
		mPlayer.setOnAreaChangeListener(this);
		mPlayer.setOnPreparedListener(this);

		mRemote = new DxbRemote(this);

		mIndicatorView = new DxbView_Indicator(this, findViewById(R.id.layout_indicator), findViewById(R.id.indicatorBottomView));
		mListView = new DxbView_List(this, findViewById(R.id.layout_list));
		mFullView = new DxbView_Full(this, findViewById(R.id.layout_full));
		mTeletextView = new DxbView_Teletext(this, findViewById(R.id.layout_teletext));
		//mEPGView = new DxbView_EPG(this, findViewById(R.id.layout_epg));
		mEPGView = new DxbView_EPG_3List(this, findViewById(R.id.layout_epg_3list));
		mDebugView = new DxbView_Debug(this, findViewById(R.id.debug));
		mScan = new DxbView_Scan(this);

		setState(true, DxbView.VIEW_MAIN);

		initIntentFilter();

		Bundle bundle	= getIntent().getExtras();
		setBundleData(bundle);
		
		mContext = this;
		
		getView().thread_Start();

		getPlayer().setOnPlayingCompletionListener(ListenerOnPlayingCompletion);
		getPlayer().setOnFilePlayTimeUpdateListener(ListenerOnTimeUpdate);
		getPlayer().setOnVideoOutputListener(ListenerOnVideoOutput);
		
		getPlayer().setOnHandoverChannelListener(ListenerOnHandoverChannel);
		getPlayer().setOnEWSListener(ListenerOnEWS);
		getPlayer().setOnSCErrorListener(ListenerOnSCError);
		getPlayer().setOnEPGUpdateListener(ListenerOnEPGUpdate);
	}

	private void setResumeOptionValue()
	{
		Log.i(TAG, "setResumeOptionValue()");

		SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(this);
		String list_value;

		list_value= pref.getString(DxbOptions.KEY_AUDIO_SELECT, "");
		if(list_value != "")
		{
			getPlayer().cOption.audio = Integer.parseInt(list_value);
		}		
		
		list_value= pref.getString(DxbOptions.KEY_DUAL_AUDIO_SELECT, "");
		if(list_value != "")
		{
			getPlayer().cOption.dual_audio = Integer.parseInt(list_value);
		}
		
		list_value= pref.getString(DxbOptions.KEY_CAPTION_SELECT, "");
		if(list_value != "")
		{
			getPlayer().cOption.caption = Integer.parseInt(list_value);
		}
			
		list_value= pref.getString(DxbOptions.KEY_SUPER_IMPOSE_SELECT, "");
		if(list_value != "")
		{
			getPlayer().cOption.super_impose = Integer.parseInt(list_value);
		}
			
		getPlayer().cOption.frequency_vhf = pref.getBoolean(DxbOptions.KEY_FREQ_VHF, false);
		getPlayer().cOption.frequency_mid = pref.getBoolean(DxbOptions.KEY_FREQ_MID, false);
		getPlayer().cOption.frequency_shb = pref.getBoolean(DxbOptions.KEY_FREQ_SHB, false);
		
		list_value = pref.getString(DxbOptions.KEY_SEAMLESS_CHANGE_SELECT,"");
		if (list_value !="") {
			getPlayer().cOption.seamless_change = Integer.parseInt(list_value);
		}
		
		list_value = pref.getString(DxbOptions.KEY_HANDOVER_SELECT, "");
		if(list_value != "")
		{
			int	iHandover = Integer.parseInt(list_value);
			if(iHandover == 0)
			{
				getPlayer().cOption.handover = false;
			}
			else
			{
				getPlayer().cOption.handover = true;
			}
		}

		list_value = pref.getString(DxbOptions.KEY_EWS_SELECT, "");
		if(list_value != "")
		{
			int	iEWS	= Integer.parseInt(list_value);
			if(iEWS == 0)
			{
				getPlayer().cOption.ews	= false;
			}
			else
			{
				getPlayer().cOption.ews	= true;
			}
		}

		list_value = pref.getString(DxbOptions.KEY_BCASMODE_SELECT,"");
		if (list_value != "")
		{
			getPlayer().cOption.bcas_mode = Integer.parseInt(list_value);
		}

		list_value = pref.getString(DxbOptions.KEY_STORAGE_SELECT, "");
		if(list_value != "")
		{
			DxbStorage.setStorage(list_value);
		}

		list_value = pref.getString(DxbOptions.KEY_SIGNAL_QUALITY_SELECT, "");
		if(list_value != "")
		{
			int	iSignalQuality = Integer.parseInt(list_value);
			if(iSignalQuality == 0)
			{
				getPlayer().cOption.signal_quality = false;
			}
			else
			{
				getPlayer().cOption.signal_quality = true;
			}
		}
	
		getPlayer().cOption.field_log = pref.getBoolean(DxbOptions.KEY_FIELD_LOG, false);

		/* preset mode */
		if (DxbOptions.VISIBLE_ID_preset_mode == true) { 
			list_value = pref.getString(DxbOptions.KEY_PRESET_MODE, "");
			if (list_value != "")
				getPlayer().cOption.preset_mode = Integer.parseInt(list_value);
			else
				getPlayer().cOption.preset_mode = 0;
		} else
			getPlayer().cOption.preset_mode = 0;
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
	}

	private void setBundleData(Bundle bundle)
	{
		Log.i(TAG, "setBundleData(bundle = " + bundle + ")");
		
		Scheduler_Info.Alarm_Type		= -1;
		Scheduler_Info.Channel_Type		= -1;
		Scheduler_Info.Channel_Index	= -1;
		Scheduler_Info.Alarm_ID			= -1;
		Scheduler_Info.Repeat_Type		= -1;

		if(bundle != null)
		{
			Scheduler_Info.Alarm_Type		= bundle.getInt(Alert.EXTRA_TYPE_ALARM, 0);
			
			Scheduler_Info.Channel_Type		= bundle.getInt(Alert.EXTRA_TYPE_CHANNEL_TYPE, 0);
			Scheduler_Info.Channel_Index	= bundle.getInt(Alert.EXTRA_TYPE_CHANNEL_INDEX, 0);
			
			Scheduler_Info.Alarm_ID			= bundle.getInt(Alert.EXTRA_TYPE_ID, 0);
			Scheduler_Info.Repeat_Type		= bundle.getInt(Alert.EXTRA_TYPE_REPEAT, 0);
			
			Log.d(TAG, "iNotice_Type = " + Scheduler_Info.Alarm_Type);
			Log.d(TAG, "iChannel_Type = " + Scheduler_Info.Channel_Type);
			Log.d(TAG, "iChannel_Index = " + Scheduler_Info.Channel_Index);
			Log.d(TAG, "iAlarm_ID = " + Scheduler_Info.Alarm_ID);
			Log.d(TAG, "iRepeat_Type = " + Scheduler_Info.Repeat_Type);
		}
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
			onStop();
			onDestroy();
			finish();
			android.os.Process.killProcess(android.os.Process.myPid());
			
			return;
		}
		
		if ((intentOptionActivity == null) && (intentScheduleActivity == null))
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

		getPlayer().onCtrlLastFrameFunction(1);
		
		eCycle_Life = DXB_LIFE_CYCLE.ON_RESUME;
 		Intent i = new Intent(SERVICECMD);
		i.putExtra(CMDNAME, CMDPAUSE);
		sendBroadcast(i);
		setResumeOptionValue();

		intentScheduleActivity	= null;
		intentOptionActivity = null;
		getPlayer().iManualScan_Frequency = 0;
		if(!getPlayer().isValid() && getPlayer().isPlaying())
		{
			getPlayer().openPlayer();
			getPlayer().setFreqBand();
			getPlayer().setChannel();
			getPlayer().useSurface(0);
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
		if(getPlayer().getRecordState() == DxbPlayer.REC_STATE.RECORDING)
		{
			getPlayer().eState = DxbPlayer.STATE.UI_PAUSE;
			mFullView.setRecordStartStop();
			getPlayer().setRecStop();
		}
		if(getPlayer().getLocalPlayState() == DxbPlayer.LOCAL_STATE.PLAYING)
		{
			getPlayer().eState = DxbPlayer.STATE.UI_PAUSE;
			getPlayer().setLocalPlayStop();
		}
		
		if(getAlertState() != Alert.TYPE_ALERT_START)
		{
			getPlayer().releaseSurface();
			getPlayer().onCtrlLastFrameFunction(0);
			
			if ((intentOptionActivity == null) && (intentScheduleActivity == null))
			{
				if(getPlayer().isValid())
				{
					getPlayer().playSubtitle(DxbPlayer._OFF_);
					getPlayer().stop();
					getPlayer().release();
				}
			}
		}
	}

	@Override
	protected void onStop() {
		Log.d(TAG, "================> onStop");
		getPlayer().property_set("tcc.dxb.fullscreen", "0");
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
		getPlayer().property_set("tcc.dxb.fullscreen", "0");
		
		eCycle_Life = DXB_LIFE_CYCLE.ON_DESTROY;
		if(mReceiverIntent_Schedule != null)
		{
			unregisterReceiver(mReceiverIntent_Schedule);
			mReceiverIntent_Schedule	= null;
		}

		getPlayer().onDestroy();
		getView().onDestroy();
		
		//DxbView_Message.onDestroy();
		
		if (mToast != null)
		{
			mToast.cancel();
			mToast = null;
		}
	
		if (mSQInfoToast != null)
		{
			mSQInfoToast.cancel();
			mSQInfoToast = null;
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

		if (mSQInfoToast != null)
		{
			mSQInfoToast.cancel();
			mSQInfoToast = null;
		}

		getPlayer().playSubtitle(DxbPlayer._OFF_);
		mIndicatorView.releaseAll();

		findViewById(R.id.full_view).setBackgroundColor(0xFF101010);
		
		showDialog(DxbDialog.APK_FINISH_PROGRESS);
		
		new Thread()
		{
			public void run()
			{
				getPlayer().releaseSurface();
				if(getPlayer().isValid())
				{
					getPlayer().playSubtitle(DxbPlayer._OFF_);
					getPlayer().stop();
					getPlayer().release();
				}
				
				if(mReceiverIntent_Schedule != null)
				{
					unregisterReceiver(mReceiverIntent_Schedule);
					mReceiverIntent_Schedule	= null;
				}

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

	public DxbPlayer.OnChannelUpdateListener ListenerOnChannelUpdate = new DxbPlayer.OnChannelUpdateListener()
	{	
		public void onChannelUpdate(DxbPlayer player, int request)
		{
			Log.i(TAG, "onChannelUpdate()");
			
			getView().onChannelUpdate(player, request);			
		}
	};
	
	public DxbPlayer.OnHandoverChannelListener ListenerOnHandoverChannel = new DxbPlayer.OnHandoverChannelListener()
	{	
		public void onHandoverChannel(DxbPlayer player, int affiliation, int channel)
		{
			Log.i(TAG, "onHandoverChannel()");
			
			getView().HandoverChannel(player, affiliation, channel);			
		}
	};
	
	public DxbPlayer.OnEWSListener ListenerOnEWS = new DxbPlayer.OnEWSListener()
	{	
		public void onEWS(DxbPlayer player, int bStatus, EWSData obj)
		{
			Log.i(TAG, "onEWS()");
			
			String message = "";

			if(bStatus != 0)
			{
				CharSequence[] values;
				CharSequence[] entries;
				String signaltype = "";
				String localcode = "";
	
				values = mContext.getResources().getTextArray(R.array.signal_type_values);
				entries = mContext.getResources().getTextArray(R.array.signal_type_entries);
				for(int i=0; i<values.length; i++)
				{
					if(Integer.parseInt(values[i].toString()) == obj.SignalType)
					{
						signaltype = entries[i].toString();
						break;
					}
				}

				message = mContext.getResources().getString(R.string.ews_running_start) + "\n" + signaltype + "\n" + mContext.getResources().getString(R.string.local_code_entries_tag);
				values = mContext.getResources().getTextArray(R.array.local_code_values);
				entries = mContext.getResources().getTextArray(R.array.local_code_entries);
				for(int j=0; j < obj.LocalCodeLength; j++)
				{
					for(int i=0; i<values.length; i++)
					{
						if(Integer.parseInt(values[i].toString()) == obj.LocalCode[j])
						{
							localcode = entries[i].toString();
							message = message + " " + localcode;
							break;
						}
					}
				}

				updateToast(message);
				mContext.findViewById(R.id.indicator_ews).setVisibility(View.VISIBLE);
			}
			else
			{
				message = mContext.getResources().getString(R.string.ews_running_end);
				updateToast(message);
				mContext.findViewById(R.id.indicator_ews).setVisibility(View.INVISIBLE);
			}

			getView().EWS(player, bStatus, obj);			
		}
	};
	
	public DxbPlayer.OnSCErrorListener ListenerOnSCError = new DxbPlayer.OnSCErrorListener()
	{	
		public void onSCErrorUpdate(DxbPlayer player, int bStatus)
		{
			Log.i(TAG, "onSCErrorUpdate()");

			getPlayer().requestBCASInfo(0);
			
			getView().SCErrorUpdate(player, bStatus);			
		}
	};
	
	public DxbPlayer.OnEPGUpdateListener ListenerOnEPGUpdate = new DxbPlayer.OnEPGUpdateListener()
	{	
		public void onEPGUpdate(DxbPlayer player, int serviceID, int tableID)
		{
			Log.i(TAG, "onEPGUpdate()");
			
			getView().EPGUpdate(player, serviceID, tableID);			
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
		
		if (getView() != null)
		{
			if (getView().onKeyDown(keyCode, event))
			{
				return true;
			}
		}
		
		return super.onKeyDown(keyCode, event);
	}

	public void updateScreen()
	{
		Log.i(TAG, "updateScreen()");
		
		getView().onUpdateScreen();
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

			case DxbView.VIEW_LIST:
			case DxbView.VIEW_EPG:
				iCurrView = iChangeView;
				getView().setVisible(true);
				if(getPlayer().isValid()) {
					getPlayer().playSubtitle(DxbPlayer._OFF_);
				}			
				break;


			case DxbView.VIEW_NULL:
				break;
	
			default: // change view
				iCurrView = iChangeView;
				//getPlayer().releaseSurface();            // destroy renderer
				//getPlayer().setVisibility(View.GONE);
				getView().setVisible(true);
				//getPlayer().setVisibility(View.VISIBLE); // create renderer
		}

		if(isRefresh)
		{ // scan popup
			if (getPlayer().eState == DxbPlayer.STATE.OPTION_MANUAL_SCAN)
			{
				getPlayer().eState = DxbPlayer.STATE.GENERAL;
				mScan.startManual(getPlayer().cOption.scan_manual, getPlayer().cOption.scan_manual_bandwidth);
			}
			else if(getPlayer().eState == DxbPlayer.STATE.OPTION_FULL_SCAN)
			{
				getPlayer().eState = DxbPlayer.STATE.GENERAL;
				mScan.startFull();
			} else if (getPlayer().eState == DxbPlayer.STATE.OPTION_AUTOSEARCH_DN) {
				getPlayer().eState = DxbPlayer.STATE.GENERAL;
				mScan.startAutoSearch (0);
			} else if (getPlayer().eState == DxbPlayer.STATE.OPTION_AUTOSEARCH_UP) {
				getPlayer().eState = DxbPlayer.STATE.GENERAL;
				mScan.startAutoSearch (1);
			}
		}
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

	public int getSCStatus()
	{
		return getPlayer().getSCStatus();
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
			ad.setMessage(this.getResources().getString(R.string.are_you_sure_to_exit_ISDBT));
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
			pd.setMessage(this.getString(R.string.please_wait_ISDBT_is_shutting_off));
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
					Scheduler_Info.Alarm_Type		= iAlarm_Type;
					Scheduler_Info.Channel_Type		= -1;
					Scheduler_Info.Channel_Index	= -1;
					Scheduler_Info.Alarm_ID			= iID;
					Scheduler_Info.Repeat_Type		= iRepeat_Type;

					setState(true, DxbView.VIEW_FULL);
					getPlayer().setServiceType(iChannel_Type);
					getPlayer().setChannel(iChannel_Index);
				}
			}
			else if(Alert.ACTION_REC_STOP.equals(action))
			{
				Log.d(TAG, "if(Alert.ACTION_STOP_REC.equals(action))");
				
				if(getPlayer().getRecordState() == DxbPlayer.REC_STATE.RECORDING) {
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
			if(		(getPlayer().getRecordState() == DxbPlayer.REC_STATE.RECORDING)
				||	(getPlayer().getRecordState() == DxbPlayer.REC_STATE.RECORDING_ALARM)
			)
			{
				getPlayer().eState = DxbPlayer.STATE.UI_PAUSE;
				getPlayer().setRecStop();
			}
			
			if (getPlayer().getLocalPlayState() != DxbPlayer.LOCAL_STATE.STOP)
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
				getPlayer().playSubtitle(DxbPlayer._OFF_);
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
	
		if(		(Scheduler_Info.Alarm_Type == Alert.TYPE_ALARM_NOTICE_REC)
			&&	(Scheduler_Info.Repeat_Type >= 0)
			&&	(Scheduler_Info.Alarm_ID >= 0)
		)
		{
			mFullView.startRecord(Scheduler_Info.Alarm_ID, Scheduler_Info.Repeat_Type);
			
			Scheduler_Info.Alarm_Type	= -1;
			Scheduler_Info.Repeat_Type	= -1;
			Scheduler_Info.Alarm_ID		= -1;
		}
	}

	public void onAreaChange()
	{
		getView().onAreaChange();		
	}
	
	public void updateChannelList()
	{
		Log.i(TAG, "updateChannelList()");
		
		getPlayer().updateChannelList();
		getView().setChannel();
		mListView.updateChannelList();
		mFullView.onUpdateScreen();
	}

	public void updateBackScanChannelList()
	{
		Log.i(TAG, "updateBackScanChannelList()");
		
		getPlayer().updateChannelList();
		getView().setBackScanChannel();
		mListView.updateChannelList();
		mFullView.onUpdateScreen();		
	}
	
	public void updateChannelAutoSearch (int fullseg_row, int oneseg_row)
	{
		Log.i(TAG,"updateChannelautoSearch()");
		getView().setAutoSearchChannel(fullseg_row, oneseg_row);
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

	public void updateSignalQualityToast(String message) {
//		DxbLog(I, "updateSignalQualityToast(" + message + ")");
		if(mSQInfoToast == null) {
			mSQInfoToast = Toast.makeText(this, message, 2000);
			mSQInfoToast.setGravity(Gravity.TOP|Gravity.RIGHT, 0, 30);
			
			LinearLayout linearLayout = (LinearLayout)mSQInfoToast.getView();
			View child = linearLayout.getChildAt(0);
			TextView messageTextView = (TextView)child;
		} else {
			mSQInfoToast.show();
			mSQInfoToast.setText(message);
		}
		mSQInfoToast.show();
	}
	
	public void onPrepared(DxbPlayer player) {
		getView().onUpdateScreen();
	}
	
	public DxbView getEPGView()
	{
		return mEPGView;
	}
}
