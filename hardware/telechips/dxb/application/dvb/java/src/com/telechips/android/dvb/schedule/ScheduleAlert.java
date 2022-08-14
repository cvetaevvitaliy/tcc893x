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

package com.telechips.android.dvb.schedule;

import com.telechips.android.dvb.option.DxbOptions;
import com.telechips.android.dvb.R;
import com.telechips.android.dvb.player.DxbPlayer;
import com.telechips.android.dvb.DVBPlayerActivity;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

public class ScheduleAlert extends Activity
{
	class Component
	{
		TextView	tvMessage;
		
		Button		bCancel;
		Button		bOk;
	}
	private Component	gComponent	= new Component();
	
	class Data_Bundle
	{
		boolean	isScreenOn;
		
		int	iAlarm_Type;
		
		int	iChannel_Type;
		int	iChannel_Index;
		
		int	iID;
		int	iType_Repeat;
	}
	
	class Information
	{
		Context	mContext	= null;
		boolean	isPlayng	= false;

		//	Display
		int	iDisplayWidth;
		int	iDisplayHeight;
		float	fDensity;
		
		//	Message
		int	iCountdown;

		//	Bundle Data
		Data_Bundle	dataBundle	= new Data_Bundle();
		
		//	Class
		DxbScheduler schedulerActivity	= (DxbScheduler)DxbScheduler.schedulerActivity;
		DxbOptions	optionActivity	= DxbOptions.getInstance();
		
		//	Dxb
		boolean	isDxbPlaying	= false;
	}
	private Information	gInformation	= new Information();
	
	Handler mHandler_Main = new Handler();

    protected static final String SCREEN_OFF = "screen_off";

	private DVBPlayerActivity getMainActivity() {
		return DVBPlayerActivity.getInstance();
	}

	private DxbPlayer getPlayer() {
		return getMainActivity().getPlayer();
	}

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		Log(I, "onCreate()");
		
		super.onCreate(savedInstanceState);
		

        final Window win = getWindow();
        win.addFlags(WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED
                | WindowManager.LayoutParams.FLAG_DISMISS_KEYGUARD);
        
        // Turn on the screen unless we are being launched from the AlarmAlert
        // subclass as a result of the screen turning off.
        if (!getIntent().getBooleanExtra(SCREEN_OFF, false)) {
            win.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON
                    | WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON
                    | WindowManager.LayoutParams.FLAG_ALLOW_LOCK_WHILE_SCREEN_ON);
        }
		
        LayoutInflater inflater = LayoutInflater.from(this);
        setContentView(inflater.inflate(R.layout.dxb_schedule_alert, null));
		
		setSystemData(this);
		
		setComponent();
		setTextSize();

		setListener();
		
		Bundle bundle	= getIntent().getExtras();
		setBundleData(bundle);

		init();
	}
	
	protected void onResume()
	{
		Log(I, "onResume()");
		
		super.onResume();
		
		IntentFilter	commandFilter	= new IntentFilter();
		commandFilter.addAction(Alert.ACTION_DXB_PLAYING_RES);
		registerReceiver(mReceiverIntent, commandFilter);
	}
	
	protected void onPause()
	{
		Log(I, "onPause()");
		
		super.onPause();
		
		unregisterReceiver(mReceiverIntent);
		sendDxb_AlertPause();
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		Log(I, "onKeyDown(keyCode=" + keyCode + ", event=" + event + ")");
		
		switch(keyCode)
		{
			case KeyEvent.KEYCODE_BACK:
				scheduler_Cancel();
			break;
		}
		
		return super.onKeyDown(keyCode, event);
	}
	
	@SuppressWarnings("deprecation")
	private void setSystemData(Context context)
	{
		Log(I, "setSystemData()");
		
		Display display = ((WindowManager)getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
		
		gInformation.iDisplayWidth	= display.getWidth();
		gInformation.iDisplayHeight	= display.getHeight();
		
		DisplayMetrics displayMetrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
		gInformation.fDensity	= displayMetrics.density;
	}
	
	private void setComponent()
	{
		Log(I, "setComponent()");
		
		//	Message
			gComponent.tvMessage	= (TextView)findViewById(R.id.alert_meg);
			
		//	Button
			gComponent.bCancel	= (Button)findViewById(R.id.alert_cancel);
			gComponent.bOk		= (Button)findViewById(R.id.alert_ok);
	}
	
	private void setTextSize()
	{
		Log(I, "setTextSize()");
		
		float	infoTextSize;
		
		infoTextSize	= 15 * gInformation.iDisplayWidth / gInformation.fDensity / 800;
		gComponent.tvMessage.setTextSize(infoTextSize);
	}
	
	private void setListener()
	{
		Log(I, "setListener()");
		
		gComponent.bCancel.setOnClickListener(ListenerOnClick);
		gComponent.bOk.setOnClickListener(ListenerOnClick);
	}
	
	OnClickListener ListenerOnClick	= new View.OnClickListener()
	{
		public void onClick(View v)
		{
			int id = v.getId();
			Log(I, "ListenerOnClick - onClick(id=" + id + ")");
			
			switch(id)
			{
				case R.id.alert_cancel:
					scheduler_Cancel();
				break;
					
				case R.id.alert_ok:
					scheduler_Start();
				break;
			}
		}
	};
	
	private void setBundleData(Bundle bundle)
	{
		Log(I, "setBundleData()");
		
		gInformation.dataBundle.isScreenOn		= bundle.getBoolean(Alert.EXTRA_TYPE_IS_SCREEN_ON);
		
		gInformation.dataBundle.iAlarm_Type		= bundle.getInt(Alert.EXTRA_TYPE_ALARM);
		
		gInformation.dataBundle.iChannel_Type	= bundle.getInt(Alert.EXTRA_TYPE_CHANNEL_TYPE);
		gInformation.dataBundle.iChannel_Index	= bundle.getInt(Alert.EXTRA_TYPE_CHANNEL_INDEX);
		
		gInformation.dataBundle.iID				= bundle.getInt(Alert.EXTRA_TYPE_ID);
		gInformation.dataBundle.iType_Repeat	= bundle.getInt(Alert.EXTRA_TYPE_REPEAT);

		Log(D, "isScreenOn=" + gInformation.dataBundle.isScreenOn + 
				", iAlarm_Type=" + gInformation.dataBundle.iAlarm_Type + 
				", iID=" + gInformation.dataBundle.iID + 
				", iType_Repeat=" + gInformation.dataBundle.iType_Repeat);
	}
	
	private void init()
	{
		Log(I, "init()");
		
		gInformation.mContext	= this;
		/*gInformation.isPlayng	= true;*/	// sleep_wakeup - error
		gInformation.iCountdown	= 10;
		
		updateMessage();
		mHandler_Main.postDelayed(mRunnable_Notice, 1000);
	}
    
	Runnable mRunnable_Notice = new Runnable()
	{
		public void run()
		{
			Log(I, "run() - mRunnable_Notice - (getCountdown()=" + getCountdown() + ")");
			
			try
			{
				if(getCountdown() <= 0)
				{
					scheduler_Start();
				}
				else
				{
					if(getCountdown() == 10)
					{
						sendDxb_Playing_req();
					}

					updateMessage();
					updateCountdown();
					mHandler_Main.postDelayed(mRunnable_Notice, 1000);
				}
			}
			catch(IllegalStateException e)
			{
				e.printStackTrace();
			}
		}
	};
	
	private void updateMessage()
	{
		Log(I, "updateMessage()");
		
		if(gInformation.dataBundle.iAlarm_Type == 0)
		{
			gComponent.tvMessage.setText("[View] " + getResources().getString(R.string.book_event_time_up) + " : " + getCountdown() + "s");
		}
		else if(gInformation.dataBundle.iAlarm_Type == 1)
		{
			gComponent.tvMessage.setText("[Record] " + getResources().getString(R.string.book_event_time_up) + " : " + getCountdown() + "s");
		}
	}
	
	private int getCountdown()
	{
		return gInformation.iCountdown;
	}
	
	private void updateCountdown()
	{
		gInformation.iCountdown--;
	}
	
	private BroadcastReceiver mReceiverIntent = new BroadcastReceiver()
	{
		@Override
		public void onReceive(Context context, Intent intent)
		{
			Log(I, "onReceive()");

			String action = intent.getAction();

			if(Alert.ACTION_DXB_PLAYING_RES.equals(action))
			{
				setDxbPlay_State(getPlayer().isOFF);
			}
		}
	};
	
	private void setDxbPlay_State(boolean state)
	{
		Log(I, "setDxbPlay_State(state=" + state + ")");
		gInformation.isDxbPlaying	= state;
	}
	
	private boolean isDxbPlaying()
	{
		Log(I, "isDxbPlaying() = " + gInformation.isDxbPlaying);
		return gInformation.isDxbPlaying;
	}
	
	private void scheduler_Start()
	{
		Log(I, "scheduler_Start()");
		Log(I, "isDVBPlayerActivityON = " + getPlayer().isDVBPlayerActivityON + ", isDxbScheduleActivityON = " + getPlayer().isDxbScheduleActivityON + ", isDxbOptionActivity : " + getPlayer().isDxbOptionActivityON);

		if(getPlayer().isDVBPlayerActivityON || getPlayer().isDxbScheduleActivityON || getPlayer().isDxbOptionActivityON)
		{
			Log(D, "isPlaying() == true");

			Intent	intentChangeChannel	= new Intent(Alert.ACTION_CHANGE_CHANNEL);

			putExtra_ChangeChannel(intentChangeChannel, gInformation.dataBundle);

			gInformation.mContext.sendBroadcast(intentChangeChannel);
		}
		else
		{
			Log(D, "isPlaying() == false");

			Intent	intent	= new Intent();
			intent.setClassName("com.telechips.android.dvb", "com.telechips.android.dvb.DVBPlayerActivity");
			intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
			putExtra_ChangeChannel(intent, gInformation.dataBundle);
			
			startActivity(intent);
		}

		scheduler_finishActivity();
	}
	
	private void putExtra_ChangeChannel(Intent intent, Data_Bundle	data_bundle)
	{
		intent.putExtra(Alert.EXTRA_TYPE_IS_SCREEN_ON, data_bundle.isScreenOn);
		
		intent.putExtra(Alert.EXTRA_TYPE_ALARM, data_bundle.iAlarm_Type);
		
		intent.putExtra(Alert.EXTRA_TYPE_CHANNEL_TYPE, data_bundle.iChannel_Type);
		intent.putExtra(Alert.EXTRA_TYPE_CHANNEL_INDEX, data_bundle.iChannel_Index);
		
		intent.putExtra(Alert.EXTRA_TYPE_ID, data_bundle.iID);
		intent.putExtra(Alert.EXTRA_TYPE_REPEAT, data_bundle.iType_Repeat);
	}
	
	private void scheduler_Cancel()
	{
		Log(I, "scheduler_Cancel()");
		
		sendDxb_AlertStop();

		mHandler_Main.removeCallbacks(mRunnable_Notice);
		Alarm.release(gInformation.mContext, null, gInformation.dataBundle.iID, gInformation.dataBundle.iType_Repeat);
		
		finish();
	}
	
	private void scheduler_finishActivity()
	{
		Log(I, "scheduler_finishActivity()");
		
		sendDxb_AlertStop();

		gInformation.schedulerActivity.finish();
		if(gInformation.optionActivity != null)
		{
			gInformation.optionActivity.finish();
		}
		
		mHandler_Main.removeCallbacks(mRunnable_Notice);
		
		finish();
	}
	
	public static void sendDxb_AlertStart(Context context)
	{
		Log.i("ScheduleAlert", "sendDxb_AlertStart()");
		
		Intent	intentNotice	= new Intent(Alert.ACTION_ALERT_STATE);
		intentNotice.putExtra(Alert.EXTRA_TYPE_ALERT_STATE, Alert.TYPE_ALERT_START);
		context.sendBroadcast(intentNotice);
	}
	
	private void sendDxb_Playing_req()
	{
		Log(I, "send_DxbPlaying_req()");
		
		Intent	intentNotice	= new Intent(Alert.ACTION_DXB_PLAYING_REQ);
		sendBroadcast(intentNotice);
	}
	
	private void sendDxb_AlertPause()
	{
		Log(I, "sendDxb_AlertPause(gInformation.isPlayng=" + gInformation.isPlayng + ")");
		
		if(gInformation.isPlayng)
		{
			scheduler_Cancel();
			
			Intent	intentNotice	= new Intent(Alert.ACTION_ALERT_STATE);
			intentNotice.putExtra(Alert.EXTRA_TYPE_ALERT_STATE, Alert.TYPE_ALERT_PAUSE);
			sendBroadcast(intentNotice);
		}
	}
	
	private void sendDxb_AlertStop()
	{
		Log(I, "sendDxb_AlertStop()");
		
		gInformation.isPlayng	= false;
		
		Intent	intentNotice	= new Intent(Alert.ACTION_ALERT_STATE);
		intentNotice.putExtra(Alert.EXTRA_TYPE_ALERT_STATE, Alert.TYPE_ALERT_STOP);
		sendBroadcast(intentNotice);
	}

	final int D = 0;
	final int E = 1;
	final int I = 2;
	final int V = 3;
	final int W = 4;
	
	private void Log(int level, String mString)
	{
		if(Scheduler.TAG == null)
		{
			return;
		}
		
		String TAG = "[[[ScheduleAlert]]] ";
		if(TAG != null)
		{
			switch(level)
			{
	    		case E:
	    			Log.e(TAG, mString);
	    		break;
	    		
	    		case I:
	    			Log.i(TAG, mString);
	    		break;
	    		
	    		case V:
	    			Log.v(TAG, mString);
	    		break;
	    		
	    		case W:
	    			Log.w(TAG, mString);
	    		break;
	    		
	    		case D:
	    		default:
	    			Log.d(TAG, mString);
	    		break;
			}
		}
	}
}
