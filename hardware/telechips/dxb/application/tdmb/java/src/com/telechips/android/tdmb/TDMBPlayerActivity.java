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

package com.telechips.android.tdmb;

import android.app.Activity;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.drawable.ColorDrawable;
import android.os.Bundle;
import android.util.Log;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Window;
import android.view.View;
import android.app.Instrumentation;

import android.media.AudioManager;


public class TDMBPlayerActivity extends Activity 
{
	private static final String TAG = "[[[TDMBPlayerActivity]]] ";

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
		static private DXB_LIFE_CYCLE	eCycle_Life = DXB_LIFE_CYCLE.ON_DESTROY;
	/*******************************************************************************/

		
	/*******************************************************************************
	 * Copied from MediaPlaybackService in the Music Player app. Should be
	 * public, but isn't.
	 *******************************************************************************/
		private static final String SERVICECMD = "com.android.music.musicservicecommand";
		private static final String CMDNAME    = "command";
		private static final String CMDPAUSE   = "pause";
	/*******************************************************************************/
	

	@Override
	public void onCreate(Bundle savedInstanceState)
    {
		Log.i(TAG, "================> onCreate 2012.02.03.");

		super.onCreate(savedInstanceState);
		eCycle_Life = DXB_LIFE_CYCLE.ON_CREATE;
		
		getWindow();
		this.requestWindowFeature(Window.FEATURE_NO_TITLE);
		
		setContentView(R.layout.main);
		
		Manager_Setting.init(this);
		DxbPlayer.init();
		DxbView.init();
		DxbRemote.init();
	}

	@Override
	public void onStart()
	{
		Log.d(TAG, "================> onStart");
		super.onStart();
		eCycle_Life = DXB_LIFE_CYCLE.ON_START;

		if(DxbPlayer.isOFF)
		{
			onStop();
			onDestroy();
			finish();
			android.os.Process.killProcess(android.os.Process.myPid());
			
			return;
		}
	/*
		if(DxbView.intentSubActivity == null)
		{
			int	iCount	= 0;
			while(DxbPlayer.mPlayer != null)
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
						DxbPlayer.mPlayer.stop();
						DxbPlayer.mPlayer.release();
						
						DxbPlayer.mPlayer	= null;
					}
				}
				catch(InterruptedException e)
				{
					Log.d(TAG, "sleep fail!");
				}
			}
		}
	*/
		if((DxbPlayer.eState != DxbPlayer.STATE.OPTION_MANUAL_SCAN) && (DxbPlayer.eState != DxbPlayer.STATE.OPTION_MAKE_PRESET))
		{
			DxbPlayer.eState	= DxbPlayer.STATE.GENERAL;
		}
	}
	
	@Override
	protected void onResume()
	{
		Log.d(TAG, "================> onResume");

		super.onResume();
		eCycle_Life = DXB_LIFE_CYCLE.ON_RESUME;

		DxbView.initSurfaceView();

 		Intent i = new Intent(SERVICECMD);
		i.putExtra(CMDNAME, CMDPAUSE);
		sendBroadcast(i);

		if( DxbPlayer.mChannelManager == null )
		{
			DxbPlayer.mChannelManager = new ChannelManager(this);
			DxbPlayer.mChannelManager.open();
		}
		
		if(DxbView.mDB == null)
		{
			DxbView.mDB = new DxbDB_Setting(this).open();
		}

		DxbPlayer.setOptionValue();

		DxbView.intentSubActivity = null;

		if(		(DxbPlayer.mPlayer == null)
				&&	(DxbView.mSurfaceHolder != null)
			)
		{
			DxbPlayer.setListener_Player();
			DxbPlayer.setSurface();
			DxbPlayer.start();
		}

		if(DxbPlayer.mPlayer != null)
		{
			DxbPlayer.mPlayer.setAudioMute(false);
		}

		DxbView.mHandler_Main.postDelayed(DxbView_Indicator.mRunnable_Signal, 1000);

		DxbView.mSurfaceView.setVisibility(View.VISIBLE);

		DxbPlayer.useSurface(0);

		DxbView.setState(true, DxbView.eState);
	}
	
	public boolean isActivityFinish = true;

	@Override
	protected void onUserLeaveHint ()
	{
		Log.d(TAG, "================> onUserLeaveHint() --> HOME KEY OPERATION");
		isActivityFinish = false;
	}
	
	@Override
	protected void onPause()
	{
		Log.d(TAG, "================> onPause() --> DxbPlayer.eState="+DxbPlayer.eState);
		
		super.onPause();
		eCycle_Life = DXB_LIFE_CYCLE.ON_PAUSE;
		
		if(DxbPlayer.eState == DxbPlayer.STATE.SCAN)
		{
			android.os.Process.killProcess(android.os.Process.myPid() );
			return;
		}
		else if(DxbView_Record.eState == DxbView_Record.STATE.RECORDING)
		{
			DxbPlayer.eState = DxbPlayer.STATE.UI_PAUSE;
			DxbView_Record.eState = DxbView_Record.STATE.SAVE;
			if(DxbPlayer.mPlayer != null)
			{
				DxbPlayer.mPlayer.setRecStop();
			}
			return;
		}
				
		DxbView.mSurfaceView.setVisibility(View.GONE);

		DxbView_Message.onPause();
		
		DxbPlayer.releaseSurface();

		if(isActivityFinish == true)
		{
			if(DxbView.intentSubActivity == null)
			{
				if(DxbPlayer.mPlayer != null)
				{
					Log.e(TAG, "================> ActivityFinish");
					DxbPlayer.mPlayer.stop();
					DxbPlayer.mPlayer.release();
					
					DxbPlayer.mPlayer	= null;
			
					//DxbView.mAudioManager.setStreamVolume(AudioManager.STREAM_MUSIC, DxbView.mAudioCurrentVolume, 0);
				}
			}
		}
		else
		{
			if(DxbPlayer.mPlayer != null)
			{
				DxbPlayer.mPlayer.setAudioMute(true);
			}
		}

		isActivityFinish = true;
	}

	@Override
	protected void onStop()
	{
		Log.d(TAG, "================> onStop");
		super.onStop();

		if(eCycle_Life == DXB_LIFE_CYCLE.ON_PAUSE)
		{
			Log.d(TAG, "================> DXB_LIFE_CYCLE.ON_PAUSE");

			eCycle_Life = DXB_LIFE_CYCLE.ON_STOP;
		}
		
		DxbPlayer.eState	= DxbPlayer.STATE.NULL;
	}
	
	@Override
	public void onDestroy()
	{
		Log.d(TAG, "================> onDestroy()");
		eCycle_Life = DXB_LIFE_CYCLE.ON_DESTROY;

		if(DxbView.intentSubActivity == null)
		{
			if(DxbPlayer.mChannelManager != null)
			{
				DxbPlayer.mChannelManager.close();
				DxbPlayer.mChannelManager = null;
			}
			
			if(DxbView.mDB != null)
			{
				DxbView.mDB.close();
				DxbView.mDB = null;
			}
		}

		DxbView_Message.onDestroy();

		super.onDestroy();
	}
	
    @Override
    public boolean dispatchTouchEvent(MotionEvent event) {
    	if (event.getButtonState() == MotionEvent.BUTTON_SECONDARY) {
	    	if(event.getAction() == MotionEvent.ACTION_DOWN) {
	    		new Thread(new Runnable() {
	    			public void run() {
	    				new Instrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
	    			}
	    		}).start();
    		    return true;
    		}
    	}
        return super.dispatchTouchEvent(event);
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

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		Log.d(TAG, "onKeyDown(keyCode=" + keyCode + ", event=" + event + ")");
		
		DxbView.mHandler_Main.removeCallbacks(DxbView_List.mRunnable_KeyEvent);
		
		if(DxbPlayer.eState == DxbPlayer.STATE.SCAN_STOP)
		{
			DxbView_Message.updateToast(getResources().getString(R.string.please_wait_cancel_scanning));
			return true;
		}
		
		if(DxbView_Record.eState != DxbView_Record.STATE.STOP)
		{
			DxbView_Record.setFocus_bStop();
			
			if(		(keyCode == KeyEvent.KEYCODE_VOLUME_UP)
				||	(keyCode == KeyEvent.KEYCODE_VOLUME_DOWN)
			)
			{
				return super.onKeyDown(keyCode, event);
			}
			else
			{
				return true;
			}
		}
		
		if((keyCode == KeyEvent.KEYCODE_BACK) || (keyCode == KeyEvent.KEYCODE_ESCAPE))
		{
			boolean bReturn	= DxbView.backState(keyCode, event, DxbView.eState);
			
			if(bReturn)
			{
				return bReturn;
			}
			else
			{
				return super.onKeyDown(keyCode, event);
			}
		}
		
		if(DxbView.eState == DxbView.STATE.FULL)
		{
			boolean bReturn	= DxbView_Full.onKeyDown(keyCode, event);
			
			if(bReturn)
			{
				return bReturn;
			}
			else
			{
				return super.onKeyDown(keyCode, event);
			}
		}
		else if(DxbView.eState == DxbView.STATE.LIST)
		{
			boolean bReturn	= DxbView_List.onKeyDown(keyCode, event);
			
			if(bReturn)
			{
				return bReturn;
			}
			else
			{
				return super.onKeyDown(keyCode, event);
			}
		}
		else if(DxbView.eState == DxbView.STATE.EPG)
		{
			boolean bReturn	= DxbPlayer.onKeyDown_EPG(keyCode, event);
			
			if(bReturn)
			{
				return bReturn;
			}
			else
			{
				return super.onKeyDown(keyCode, event);
			}
		}
		else
		{
			switch( keyCode )
			{
				case KeyEvent.KEYCODE_HOME:
				break;
				
				case KeyEvent.KEYCODE_8:			// spd
				case KeyEvent.KEYCODE_7:			// A-B
				case KeyEvent.KEYCODE_CAMERA:
				case KeyEvent.KEYCODE_ENTER:		// EQ|MODE
				case KeyEvent.KEYCODE_MENU: 		// menu
				case KeyEvent.KEYCODE_DPAD_UP:		// ^
				case KeyEvent.KEYCODE_DPAD_DOWN:	// v
				case KeyEvent.KEYCODE_DPAD_LEFT:			// <<
				case KeyEvent.KEYCODE_DPAD_RIGHT:			// >>
				case KeyEvent.KEYCODE_SOFT_LEFT:
				case KeyEvent.KEYCODE_SOFT_RIGHT:
				break;					
			}
		}
	
		return super.onKeyDown(keyCode, event);
	}

	/* (non-Javadoc)
	 * @see android.app.Activity#onKeyMultiple(int, int, android.view.KeyEvent)
	 */
	@Override
	public boolean onKeyMultiple(int keyCode, int repeatCount, KeyEvent event)
	{
		return super.onKeyMultiple(keyCode, repeatCount, event);
	}

	/* (non-Javadoc)
	* @see android.app.Activity#onKeyUp(int, android.view.KeyEvent)
	*/
	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event)
	{
		//Log.i(TAG, "onKeyUp" + keyCode);
		
		if (keyCode == KeyEvent.KEYCODE_BACK)
		{//&& event.isTracking() && !event.isCanceled()) {
			// *** DO ACTION HERE ***
			// return true;
		}
		return super.onKeyUp(keyCode, event);
	}

	protected Dialog onCreateDialog(int id)
	{
		//DxbLog(I, "onCreateDialog()");
		
		if(id == DxbView_Message.DIALOG_SCAN)
		{
			//if(mDialog == null)
			{
				DxbView_Message.mDialog_Scan = new ProgressDialog(this);
				DxbView_Message.mDialog_Scan.getWindow().setBackgroundDrawable(new ColorDrawable(0xff343434));
			}
			DxbView_Message.mDialog_Scan.setTitle(getResources().getString(R.string.app_name));
			DxbView_Message.mDialog_Scan.setMessage(getResources().getString(R.string.please_wait_while_searching));
			DxbView_Message.mDialog_Scan.setProgressStyle(ProgressDialog.STYLE_SPINNER);
			DxbView_Message.mDialog_Scan.setIndeterminate(true);
			DxbView_Message.mDialog_Scan.setCancelable(true);
			
			DxbView_Message.mDialog_Scan.setOnCancelListener(new DialogInterface.OnCancelListener()
			{
				public void onCancel(DialogInterface dialog)
				{
					Log.d("Log", "Progress Dialog --- onCancel");
					
					DxbPlayer.eState	= DxbPlayer.STATE.SCAN_STOP;
					DxbView_Message.updateToast(getResources().getString(R.string.please_wait_cancel_scanning));
					DxbScan.cancel();
					removeDialog(DxbView_Message.DIALOG_SCAN);
				}
			});
			
			return DxbView_Message.mDialog_Scan;
		}
		
		return null;
	}

    /*private void showSystemUi(boolean visible) {
        int flag = visible ? 0 : View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
                View.SYSTEM_UI_FLAG_LOW_PROFILE;
		DxbView.mSurfaceView.setSystemUiVisibility(flag);
    }*/
}
