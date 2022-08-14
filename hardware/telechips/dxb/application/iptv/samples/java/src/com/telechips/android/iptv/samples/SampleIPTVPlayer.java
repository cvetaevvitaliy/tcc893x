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
package com.telechips.android.iptv.samples;

import android.app.Activity;
import android.os.Bundle;
import android.content.Intent;
import android.util.Log;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;
import android.app.Instrumentation;

import com.telechips.android.iptv.*;

public class SampleIPTVPlayer extends Activity
{
	private static final String TAG = "[[[SampleIPTVPlayer]]] ";
	private static final boolean isDebug	= true;
	
	/*******************************************************************************
	 * Check State
	 *******************************************************************************/
		// Dxb_View State
		public enum DXB_STATE_VIEW
		{
			LIST,
			FULL,
			NULL
		}
		private DXB_STATE_VIEW	eState_View			= DXB_STATE_VIEW.NULL;

	/*******************************************************************************
	 * General Value
	 *******************************************************************************/
		// Setting DB
		//DxbDB_Setting mDB;
        	private static final int IPTV_ACTIVEMODE_STANDBY = 0;
	        private static final int IPTV_ACTIVEMODE_PLAY = 1;
		private static final int IPTV_ACTIVEMODE_TRICKPLAY = 2;
		private static final int IPTV_ACTIVEMODE_PAUSE = 3;
		private static final int IPTV_ACTIVEMODE_SEEK = 4;

		private IPTVPlayer mPlayer;

		
	/*******************************************************************************
	 * ListView Value
	 *******************************************************************************/
		// List View
		private static String[] arIPlist;
		private static ListView mListView;
		private static TextView mListComment1, mListComment2;
		private static Button	mButton;
		
		private static int		iListNumber	= -1;
		private static int		iOld_ListNumber	= -1;

	/*******************************************************************************
	 * FullView Value
	 *******************************************************************************/
		// SurfaceView
		private SurfaceView mSurfaceView;
		private SurfaceHolder mSurfaceHolder;
		
	/*******************************************************************************
	 * Temp Value
	 *******************************************************************************/
		private boolean bStart_Flag	= true;

	/*******************************************************************************
	 * Copied from MediaPlaybackService in the Music Player app. Should be
	 * public, but isn't.
	 *******************************************************************************/
	private static final String SERVICECMD = "com.android.music.musicservicecommand";
	private static final String CMDNAME    = "command";
	private static final String CMDPAUSE   = "pause";
		
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		Log.i(TAG, "onCreate()");
		super.onCreate(savedInstanceState);
		
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,WindowManager.LayoutParams.FLAG_FULLSCREEN);
		getWindow().addFlags(// lock 화면 띄우기 전에 자신의 화면을 먼저 띄운다.
							WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED
							// 키잠금 해제하기
							| WindowManager.LayoutParams.FLAG_DISMISS_KEYGUARD
							// screen on 을 유지
							| WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON
							// 화면 on
							| WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON);
		this.requestWindowFeature(getWindow().FEATURE_NO_TITLE);
		setContentView(R.layout.main);
		
		initView();
		Dxb_setDefaultValue();
	}
	
	@Override
	protected void onResume()
	{
		Log.i(TAG, "================> onResume");		
		super.onResume();
		initSerfaceView();

		createIPTVPlayer();
		mPlayer.start();

		Dxb_setViewState(DXB_STATE_VIEW.LIST);

		Intent i = new Intent(SERVICECMD);
		i.putExtra(CMDNAME, CMDPAUSE);
		sendBroadcast(i);

//		if(iListNumber == -1)
//			Dxb_setViewState(DXB_STATE_VIEW.LIST);
//		else
//		{
//			Dxb_setViewState(DXB_STATE_VIEW.FULL);
//		}
	}
	
	@Override
	protected void onPause()
	{
		Log.i(TAG, "================> onPause");
		super.onPause();
		setIPTVSocketStop();
		mPlayer.releaseSurface();
		mPlayer.stop();
                mPlayer.release();
		eState_View = DXB_STATE_VIEW.NULL;
		iOld_ListNumber =-1;
	}

	@Override
	public void onDestroy()
	{
		Log.d(TAG, "onDestroy()");
	
		super.onDestroy();
	}
	
    private void showSystemUi(boolean visible) {
        int flag = visible ? 0 : View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
                View.SYSTEM_UI_FLAG_LOW_PROFILE;
        mSurfaceView.setSystemUiVisibility(flag);
    }

	private void initSerfaceView()
	{
		Log.i(TAG, "initSerfaceView()");
		
		mSurfaceView = (SurfaceView) findViewById(R.id.surface_view);

		if (mSurfaceView == null)
		{
			Log.d(TAG, "SerfaceView is Null");
			return;
		}
		SurfaceHolder holder = mSurfaceView.getHolder();
		if (holder == null)
		{
			Log.d(TAG, "mSurfaceHolder is Null");
			return;
		}

		holder.addCallback(mSHCallback);
		holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
	}
	
    SurfaceHolder.Callback mSHCallback = new SurfaceHolder.Callback()
    {
        public void surfaceChanged(SurfaceHolder holder, int format,int w, int h)
        {
            Log.i(TAG, "surfaceChanged");
            Log.i(TAG, "width : " +  w);
            Log.i(TAG, "height : " + h);
        }

        public void surfaceCreated(SurfaceHolder holder)
        {
            Log.i(TAG, "surfaceCreated");

            mSurfaceHolder = holder;

            mPlayer.setDisplay(mSurfaceHolder);
            //mPlayer.prepare();
            mPlayer.setScreenOnWhilePlaying(true);

            mPlayer.setSurface();
        }

        public void surfaceDestroyed(SurfaceHolder holder)
        {
            Log.i(TAG, "surfaceDestroyed");

            mSurfaceHolder = null;
        }
    };
    
	private void initView()
	{
		Log.i(TAG, "initView()");
		
		initListView();
	}
	
	private void initListView()
	{
		Log.i(TAG, "initListView()");
		
		arIPlist	= getResources().getStringArray(R.array.ip_list);
		mListView	= (ListView)findViewById(R.id.service_list);
		mListComment1	= (TextView)findViewById(R.id.comment1);
		mListComment2	= (TextView)findViewById(R.id.comment2);
		mButton			= (Button)findViewById(R.id.broadcast);
		
		mListView.setAdapter(new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, arIPlist));
		mListView.setTextFilterEnabled(true);
		
		mListView.setOnItemClickListener(ListenerOnItemClick_serviceListView);
		mButton.setOnClickListener(ListenerOnClick_broadcast);;
	}
	
	OnItemClickListener ListenerOnItemClick_serviceListView = new OnItemClickListener()
	{
		public void onItemClick(AdapterView<?> parent, View v, int position, long id)
		{
			Log.i(TAG, "ListenerOnItemClick_serviceListView-->onItemClick(position="+position+")");
			
			iListNumber	= position;
			Dxb_setComment(iListNumber);
		}
	};
	
	OnClickListener ListenerOnClick_broadcast = new OnClickListener()
	{
		public void onClick(View v)
		{
			switch(iListNumber)
			{
				case 0:
					Dxb_setViewState(DXB_STATE_VIEW.FULL);
 				break;
				case 1:
					Dxb_setViewState(DXB_STATE_VIEW.FULL);
 				break;
				
				default:
					Dxb_updateToast(getResources().getString(R.string.do_not_support_IP));
				break;
			}
		}
	};
	
    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        int keyCode = event.getKeyCode();
        if (keyCode == KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE) {
            return true;
        }

        return super.dispatchKeyEvent(event);
    }

    @Override
    public boolean onGenericMotionEvent(MotionEvent event) {
        if ((event.getSource() & InputDevice.SOURCE_CLASS_POINTER) != 0) {
            switch (event.getAction()) {
                case MotionEvent.ACTION_SCROLL: {
                    // Handle mouse (or ext. device) by shifting the page depending on the scroll
                    final float vscroll;
                    final float hscroll;
                    if ((event.getMetaState() & KeyEvent.META_SHIFT_ON) != 0) {
                        vscroll = 0;
                        hscroll = event.getAxisValue(MotionEvent.AXIS_VSCROLL);
                    } else {
                        vscroll = -event.getAxisValue(MotionEvent.AXIS_VSCROLL);
                        hscroll = event.getAxisValue(MotionEvent.AXIS_HSCROLL);
                    }
                    if (hscroll != 0 || vscroll != 0) {
                        if (hscroll > 0 || vscroll > 0) {
                            new Thread(new Runnable() {
                                public void run() {
                                    new Instrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_VOLUME_DOWN);
                                }
                            }).start();
                        } else {
				            new Thread(new Runnable() {
				    			public void run() {
				    				new Instrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_VOLUME_UP);
				    			}
				    		}).start();
						}
                        return true;
                    }
                }
            }
        }
        return super.onGenericMotionEvent(event);
    }

	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		Log.d(TAG, "onKeyDown(keyCode=" + keyCode + ", event=" + event + ")");
		
		if(keyCode == 29) //keyboard "a"
		{
			setIPTVActiveMode(IPTV_ACTIVEMODE_PLAY);
			mPlayer.getVideoWidth();
			mPlayer.getVideoHeight();
		}
		else if(keyCode == 47) //keyboard "s"
			setIPTVActiveMode(IPTV_ACTIVEMODE_STANDBY);
		else if(keyCode == 32) //keyboard "d"
			setIPTVActiveMode(IPTV_ACTIVEMODE_TRICKPLAY);
		else if(keyCode == 44) //keyboard "p"
			setIPTVActiveMode(IPTV_ACTIVEMODE_PAUSE);
		else if(keyCode == 39) //keyboard "k"
			setIPTVActiveMode(IPTV_ACTIVEMODE_SEEK);
			
		
		if(keyCode == KeyEvent.KEYCODE_BACK)
		{
			return Dxb_backView(keyCode, event, eState_View);
		}
		
		if(eState_View == DXB_STATE_VIEW.FULL)
		{
			return onKeyDown_FullView(keyCode, event);
		}
		else
		{
			switch( keyCode )
			{
				case KeyEvent.KEYCODE_HOME:
				break;
				
				case KeyEvent.KEYCODE_0:
					Dxb_setViewState(DXB_STATE_VIEW.FULL);
				break;
				
				case KeyEvent.KEYCODE_9:
				break;
					
				case KeyEvent.KEYCODE_8:			// spd
				break;
				
				case KeyEvent.KEYCODE_7:			// A-B
				break;
					
				case KeyEvent.KEYCODE_CAMERA:
				break;
				
				case KeyEvent.KEYCODE_ENTER:		// EQ|MODE
				break;
				
				case KeyEvent.KEYCODE_MENU: 		// menu
				break;
				
				case KeyEvent.KEYCODE_DPAD_UP:		// ^
				break;
				
				case KeyEvent.KEYCODE_DPAD_DOWN:	// v
				break;
				
				case KeyEvent.KEYCODE_DPAD_LEFT:			// <<
				break;
				
				case KeyEvent.KEYCODE_DPAD_RIGHT:			// >>
				break;
				
				case KeyEvent.KEYCODE_SOFT_LEFT:
				break;
				
				case KeyEvent.KEYCODE_SOFT_RIGHT:
				break;					
			}
		}
	
		return super.onKeyDown(keyCode, event);
	}	

	
	public boolean onKeyDown_FullView(int keyCode, KeyEvent event)
	{
		Log.i(TAG, "onKeyDown_FullView()");

		switch(keyCode)
		{
			//case KeyEvent.KEYCODE_6:
			case KeyEvent.KEYCODE_PAGE_UP:
				Dxb_changeChannel(1);
			break;

			//case KeyEvent.KEYCODE_9:
			case KeyEvent.KEYCODE_PAGE_DOWN:
				Dxb_changeChannel(0);
			break;
			
			default:
				return super.onKeyDown(keyCode, event);
		}
		
		return true;
	}
	
	private boolean Dxb_backView(int keyCode, KeyEvent event, DXB_STATE_VIEW state)
	{
		Log.d(TAG, "DxbBack_View(state="+state+")");
		
		if(state == DXB_STATE_VIEW.LIST)
		{
			return super.onKeyDown(keyCode, event);
		}
		else if (eState_View == DXB_STATE_VIEW.FULL)
		{
			setIPTVSocketStop();
			mPlayer.releaseSurface();
			mPlayer.stop();
			Dxb_setViewState(DXB_STATE_VIEW.LIST);
			iOld_ListNumber =-1;
		}
		
		return true;
	}	
	
	private void Dxb_setDefaultValue()
	{
		Log.d(TAG, "Dxb_setDefaultValue()");
	/*	
		if(mDB == null)
		{
			mDB = new DxbDB_Setting(this).open();
		}
		iListNumber	= mDB.getIPNumber();
	*/	
		Dxb_setComment(iListNumber);
	}
	
	private void Dxb_setComment(int number)
	{
		switch(number)
		{
			case 0:
				mListComment1.setText(arIPlist[0]);
				mListComment2.setText(getResources().getString(R.string.to_support_IP));
			break;
			
			case 1:
				mListComment1.setText(arIPlist[1]);
				mListComment2.setText(getResources().getString(R.string.to_support_IP));
			break;
			
			default:
				mListComment1.setText(getResources().getString(R.string.error));
				mListComment2.setText(getResources().getString(R.string.error));
			break;
		}		
	}
	
	private void Dxb_setViewState(DXB_STATE_VIEW eChange_state)
	{
		Log.i(TAG, "Dxb_setViewState(eChange_state="+eChange_state+")");
		
		if(eState_View == eChange_state)
		{
			Log.v(TAG, "Fail : Dxb_setViewState() - state error");
			
			return;
		}
		
		Dxb_updateBackgroundLayout(eChange_state);
		
		if(eChange_state == DXB_STATE_VIEW.FULL)
		{
			Dxb_changeChannel(iListNumber);
			mPlayer.setSurface();
			mPlayer.useSurface();
		}
		else if(eChange_state == DXB_STATE_VIEW.LIST)
		{
			mPlayer.releaseSurface();
			mListView.requestFocus();
		}
	}
	
	private void Dxb_updateBackgroundLayout(DXB_STATE_VIEW eChange_state)
	{
		Log.i(TAG, "Dxb_updateBackgroundLayout(eChange_state=" + eChange_state + ")");
		
		//	(All)GONE
		{
			if(eChange_state != DXB_STATE_VIEW.LIST)
			{
				findViewById(R.id.layout_list).setVisibility(View.GONE);
			}
			
			if(eChange_state != DXB_STATE_VIEW.FULL)
			{
				findViewById(R.id.layout_full).setVisibility(View.GONE);
			}
		}
		
		if(eChange_state == DXB_STATE_VIEW.LIST)
		{
			findViewById(R.id.layout_list).setVisibility(View.VISIBLE);
		}
		else if(eChange_state == DXB_STATE_VIEW.FULL)
		{
			findViewById(R.id.layout_full).setVisibility(View.VISIBLE);
		}
		
		eState_View			= eChange_state;
	}	
	
	private void Dxb_updateToast(String message)
	{
		Toast.makeText(this, message, 2000).show();
	}
	
	private void Dxb_changeChannel(int number)
	{
		if(iOld_ListNumber == number)
		{
			switch(iOld_ListNumber)
			{
				case 0:	
					number = 1;	
				break;
				case 1:	
					number = 0;	
				break;

				default	:
				break;
			}
		}
		iOld_ListNumber = number;	
		
		switch(number)
		{
			case 0:
				setIPTVPlay(number);
			break;

			case 1:
				setIPTVPlay(number);
			break;
			
			default:
				Dxb_updateToast(getResources().getString(R.string.error));
			break;
		}
	}

	private void createIPTVPlayer()
	{
		Log.d(TAG, "createIPTVPlayer");
		
		// create a new IPTVPlayer and set the listeners
		mPlayer = new IPTVPlayer();
		mPlayer.setOnErrorListener(errorlistener);
		mPlayer.setOnDVBParsingCompletionListener(DVBParsingCompletionListener);

		mPlayer.prepare();
		if(mSurfaceHolder != null)
		{
			mPlayer.setDisplay(mSurfaceHolder);
			mPlayer.setScreenOnWhilePlaying(true);
		}
	}
	
	IPTVPlayer.OnErrorListener errorlistener = new IPTVPlayer.OnErrorListener()
	{
		public boolean onError(IPTVPlayer player, int what, int extra)
		{
			Log.d(TAG, "onError");
			
			return false;
		}
	};
	
	IPTVPlayer.OnDVBParsingCompletionListener DVBParsingCompletionListener = new IPTVPlayer.OnDVBParsingCompletionListener()
	{
		public void onDVBParsingCompletion(IPTVPlayer player, IPTVSIData data)
		{  
		        Log.i(TAG, "siDataType : " +  data.siDataType);
		        Log.i(TAG, "siDataSize : " +  data.siDataSize);
		        Log.i(TAG, "SIData[0] : " +  data.SIData[0]);
		}
	};
	
	void setIPTVPlay(int listnum) 
	{			
//		setIPTVPIDSet(listnum);
 		mPlayer.execute();
		setIPTVSocketIpSet(listnum);
		setIPTVSocketInit();
		setIPTVSocketStart();
		setIPTVActiveMode(IPTV_ACTIVEMODE_PLAY);

//		iListNumber = listnum;

 	}

	void setIPTVActiveMode(int activemode)
	{			
		Log.i(TAG, "setIPTVActiveMode(activemode="+activemode+")");

		mPlayer.setActiveMode(activemode);

		if(activemode == IPTV_ACTIVEMODE_STANDBY)
			Dxb_updateToast(getResources().getString(R.string.set_standby_mode));
		else if(activemode == IPTV_ACTIVEMODE_PLAY)
			Dxb_updateToast(getResources().getString(R.string.set_play_mode));
		else if(activemode == IPTV_ACTIVEMODE_TRICKPLAY)
			Dxb_updateToast(getResources().getString(R.string.set_trick_play_mode));

	}
	
	void setIPTVSocketInit() 
	{			
		mPlayer.setSocketInit();
	}
	
	void setIPTVPIDSet(int listnum) 
	{			
		switch(listnum)
		{
			case 0:
				setIPTVPID(0x4c0, 0x4bf, 0x4bf, 0x81, 0x1b);		//for test   videotype: h.264, audio_type:ac3
			break;

			case 1:
				setIPTVPID(0, 0, 0, 0, 0);		//for test   videotype: h.264, audio_type:ac3
			break;

			default:
				setIPTVPID(0, 0, 0, 0, 0);		//for test   videotype: h.264, audio_type:ac3
			break;

		}
	}
	
	void setIPTVSocketIpSet(int listnum) 
	{			
		switch(listnum)
		{
			case 0:
				mPlayer.setIPInfo(3990, "239.1.1.1", 0);
			break;

			case 1:
				mPlayer.setIPInfo(3990, "239.1.1.2", 0);
			break;

			default:
				mPlayer.setIPInfo(3990, "239.1.1.1", 0);
			break;

		}
	}
	
	void setIPTVPID(int audio_pid, int video_pid, int pcr_pid, int audio_type, int video_type) 
	{			
		mPlayer.setPIDInfo(audio_pid, video_pid, pcr_pid, audio_type, video_type);
	}
	
	void setIPTVSocketStart() 
	{			
		mPlayer.setSocketStart();
		mSurfaceView.setVisibility(View.VISIBLE);
	}
	
	void setIPTVSocketStop() 
	{			
		mPlayer.setSocketStop();
		mSurfaceView.setVisibility(View.INVISIBLE);  
	}
	
	void setIPTVSocketCommand(int cmd) 
	{			
		mPlayer.setSocketCommand(cmd);
	}	
}
