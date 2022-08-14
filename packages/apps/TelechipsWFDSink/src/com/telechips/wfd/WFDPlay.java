/*
# Copyright (C) 2013 Telechips, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
*/

package com.telechips.wfd;

import java.sql.Date;
import java.text.SimpleDateFormat;
import java.util.Locale;
import android.app.Activity;
import android.app.Dialog;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.graphics.PorterDuff.Mode;
import android.net.NetworkInfo;
import android.net.wifi.p2p.WifiP2pManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Parcel;
import android.os.SystemProperties;
import android.view.GestureDetector;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import android.util.Log;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnErrorListener;
import android.media.MediaPlayer.OnInfoListener;
import android.media.MediaPlayer.OnPreparedListener;
import android.media.MediaPlayer.OnVideoSizeChangedListener;

public class WFDPlay extends Activity implements OnCompletionListener, OnPreparedListener, OnVideoSizeChangedListener, OnErrorListener, OnInfoListener {

	private final String TAG = "WFDPlay";
	public static WFDPlay instance;
	private SurfaceHolder mSHolder;
	MediaPlayer mPlayer;
	String WFDUri = null;
	private IntentFilter mIntentFilter = new IntentFilter();
	int surfaceClick=0;
	static public int mWfdPlayStatus;
	static public int WFDPLAY_STATUS_NONE = 0;
	static public int WFDPLAY_STATUS_PLAYERROR = 1;
	static public int WFDPLAY_STATUS_PLAYEND = 2;
	static public int WFDPLAY_STATUS_PLAYBACKKEY = 3;
	static public int WFDPLAY_STATUS_PAUSE = 4;
	static public int WFDPLAY_STATUS_PLAY = 5;
	static public int WFDPLAY_STATUS_SHUTDOWN = 6;

	static public int ONINFO_PAUSE = 1000;
	static public int ONINFO_RESUME = 1001;
	static public int ONINFO_AUDIO_FRAME_SKIP = 1002;
	static public int ONINFO_VIDEO_FRAME_SKIP = 1003;
	static public int ONINFO_WFD_ERROR = 1004;
	static public int ONINFO_WFD_STANDBY  = 1005;
	static public int ONINFO_WFD_HDCP_RETRY  = 1006;
	static public int ONINFO_WFD_UIBC  = 1101;
	static public int ONINFO_WFD_ERROR_CONNECT_FAIL = -1;
	static public int ONINFO_WFD_ERROR_NEGO_FAIL_M1 = 1;
	static public int ONINFO_WFD_ERROR_NEGO_FAIL_M2 = 2;
	static public int ONINFO_WFD_ERROR_NEGO_FAIL_M3 = 3;
	static public int ONINFO_WFD_ERROR_NEGO_FAIL_M4 = 4;
	static public int ONINFO_WFD_ERROR_NEGO_FAIL_M5 = 5;
	static public int ONINFO_WFD_ERROR_NEGO_FAIL_M6 = 6;
	static public int ONINFO_WFD_ERROR_NEGO_FAIL_M7 = 7;
	static public int ONINFO_WFD_ERROR_NEGO_FAIL_M8 = 8;
	static public int ONINFO_WFD_ERROR_NEGO_FAIL_M13 = 13;
	static public int ONINFO_WFD_ERROR_NEGO_FAIL_M16 = 16;
	static public int ONINFO_WFD_ERROR_CONNECT_ABORT = 20;
	static public int ONINFO_WFD_ERROR_HDCP_FAIL_RETRY = 99;
	
	static public int ONINFO_PACKETLOSS = 1099;
	
	static public int ENABLE_UIBC = 7000;
	static public int DISABLE_UIBC = 7001;
	static public int SET_PARAMETER_UIBC_DOWN = 7002;
	static public int SET_PARAMETER_UIBC_UP = 7003;
	static public int SET_PARAMETER_UIBC_MOVE = 7004;
	
    public final int HANDLER_PLAYTIME = 1;
    public final int HANDLER_UI_FRAME_SKIP_UPDATE = 2;
    public final int MESSAGE_WFDPLAY_ERROR = 3;
    public final int MESSAGE_WFDPLAY_CONNECTION_RETRY = 4;
    
	int mWidth;
	int mHeight;
	
	SurfaceView sfView;
	
	public ImageView imgPause;
	public ImageView imgResume;
	public ImageView imgShutdown;
	public ImageView imgUibc;
	
	private SystemUiHider mSystemUiHider;
	
    private boolean mBackPressed = false;
    static TextView mvTvPacketloss;
    TextView mvTvPlayTime;
    static TextView mvTvTotalPacketLossNum;
    static TextView mvTvAudioSkipNum;
    static TextView mvTvVideoSkipNum;
    static TextView mvTvPacketlossBarTop;
    static TextView mvTvPacketlossBarBottom;
    static TextView mvTvPacketlossBarLeft;
    static TextView mvTvPacketlossBarRight;
    static TextView mvTvStanby;
    
    String mPacketLossInfo;
    int mTotalPacketLossNum=0;
    int mTotalAudioFrameSkipNum=0;
    int mTotalVideoFrameSkipNum=0;
    public boolean mUibcEnabled = false;
    private GestureDetector mGd;
    public Dialog mUibcDlg;
    private boolean mSupportUIBC= false;
    
    private PlayTimeThread mPlayTimeThread;
    //private FrameSkipAdjustThread mFrameSkipAdjustThread;
    
    private Toast mPacketLossToast;
    private Toast mAudioSkipToast;
    private TimeoutThread mPacketLossToastTimeoutThread;
    private TimeoutThread mAudioSkipToastTimeoutThread;
    
    static int DEFAULT_FRAME_SKIP_TIME = 300;
    static boolean bPlayed = false;
    static boolean bErrorMesg = false;
    private boolean bPauseResumeIng = false;
    
    // frame skip
	//public String strCurrentFrameSkipTime;
	public int iCurrentFrameSkipTime;
    
	Dialog errorDialog;
	Dialog errorDialog2;
	
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.videoview);
		Log.d(TAG, "onCreate");
		instance = this;
		Intent intent = getIntent();
		mvTvPacketloss = (TextView)findViewById(R.id.packetloss);
		mvTvPlayTime = (TextView)findViewById(R.id.playtime);
		mvTvTotalPacketLossNum = (TextView)findViewById(R.id.totalpacketlossnumber);
		mvTvAudioSkipNum = (TextView)findViewById(R.id.audioskipnumber);
		mvTvVideoSkipNum = (TextView)findViewById(R.id.videoskipnumber);
		mvTvStanby = (TextView)findViewById(R.id.stanby);
		/*
		if(SystemProperties.get(WifiDisplayAdvancedSettings.PROPERTY_DEBUG_MODE).equals("1") 
				&& (SystemProperties.get(WifiDisplayAdvancedSettings.PROPERTY_FRAME_SKIP).equals("0") 
				|| SystemProperties.get(WifiDisplayAdvancedSettings.PROPERTY_FRAME_SKIP).equals("")) ) {
			mvTvAudioSkipNum.setText("");
			mvTvVideoSkipNum.setText("");
		}
		*/
		mvTvPacketlossBarTop = (TextView)findViewById(R.id.packetloss_bar_top);
		mvTvPacketlossBarBottom = (TextView)findViewById(R.id.packetloss_bar_bottom);
		mvTvPacketlossBarLeft = (TextView)findViewById(R.id.packetloss_bar_left);
		mvTvPacketlossBarRight = (TextView)findViewById(R.id.packetloss_bar_right);
		
		mTotalPacketLossNum = 0;
		mTotalAudioFrameSkipNum = mTotalVideoFrameSkipNum = 0;
		mPacketLossToast = null;
		mAudioSkipToast = null;
		//pd = null;
		//pd = ProgressDialog.show(TelechipsWFDSink.getInstance(), "Preparing", "Please wait for preparing", true, false);	
		
        String sourceIP = intent.getStringExtra("IP");
        int rtspPort = intent.getIntExtra("PORT", 7236);
		
		WFDUri = "rtsp://" + sourceIP + ":"+rtspPort+"/wfd1.0";
		//WFDUri = "rtsp://" + sourceIP + ":1234/wfd1.0"; // for test
		Log.d(TAG, "------------------ WFDUri = " + WFDUri);
		
		//if(SystemProperties.get("tcc.wifi.display.uibc").equals("1"))
		//	mSupportUIBC = true;

		imgPause = (ImageView) findViewById(R.id.pause);
		imgResume = (ImageView) findViewById(R.id.resume);
		imgShutdown = (ImageView) findViewById(R.id.shutdown);
		imgUibc = (ImageView) findViewById(R.id.uibc);
		
		imgPause.setOnTouchListener(pauseTouchListener);
		imgResume.setOnTouchListener(resumeTouchListener);
		imgShutdown.setOnTouchListener(shutdownTouchListener);
//		if(mSupportUIBC)
//			imgUibc.setOnTouchListener(uibcTouchListener);
		
		imgPause.setVisibility(View.GONE);
		imgResume.setVisibility(View.GONE);
		imgShutdown.setVisibility(View.GONE);
		imgUibc.setVisibility(View.GONE);
		imgPause.invalidate();
		imgResume.invalidate();
		imgShutdown.invalidate();
		imgUibc.invalidate();
		
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		//createConnectionProgressDialog();
		//pb = (ProgressBar)findViewById(R.id.progress);
		//pb.setVisibility( View.VISIBLE );
		
        /*
		if(SystemProperties.get(WifiDisplayAdvancedSettings.PROPERTY_FRAME_SKIP_TIME,"-1").contains("-")){
			iCurrentFrameSkipTime = DEFAULT_FRAME_SKIP_TIME;
		}
		
		Log.d(TAG,"iCurrentFrameSkipTime="+iCurrentFrameSkipTime);
		*/
		readyMediaPlayer();
		readySurfaceview();
		//TelechipsWFDSink.sleep(1);
		bPlayed = false;
		bErrorMesg = false;
	}

	private void prepareMP() {
		try {
			Log.d(TAG, "setDataSource()");		
			mPlayer.setDataSource(WFDUri);

			Log.d(TAG, "prepareAsync()");
			mPlayer.prepareAsync();

		} catch (Exception e) {
			playError(e.getMessage()+" 1");
		}
	}

	private void playError(String errMsg) {
		Log.e(TAG, "playError: " + errMsg);
		
		Intent intent = new Intent();
		intent.putExtra("WFDPlay_result", "fail");
		this.setResult(RESULT_OK, intent);
		setWFDPlayStatus(WFDPLAY_STATUS_PLAYERROR);

		try {
			this.unregisterReceiver(mReceiver);
		} catch (IllegalArgumentException ex) {
		}
		//finish();
	}
	private void readyMediaPlayer() {
		mPlayer = new MediaPlayer();
		mPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
		mPlayer.setOnCompletionListener(this);
		mPlayer.setOnPreparedListener(this);
		mPlayer.setOnVideoSizeChangedListener(this);
		mPlayer.setOnErrorListener(this);
		mPlayer.setOnInfoListener(this);
		mPlayer.setScreenOnWhilePlaying(true);
		mPlayer.setVolume(0.1f, 0.1f);
	}

	public void hideMenu() {
		Log.d(TAG, "hideMenu()");
		//surfaceClick = 0;
		
		/*
		imgPause.setVisibility(View.GONE);
		imgResume.setVisibility(View.GONE);
		imgShutdown.setVisibility(View.GONE);
		imgUibc.setVisibility(View.GONE);
		*/
		
		imgPause.setVisibility(View.INVISIBLE);
		imgResume.setVisibility(View.INVISIBLE);
		imgShutdown.setVisibility(View.INVISIBLE);
		
		if(mSupportUIBC)
			imgUibc.setVisibility(View.INVISIBLE);
		
		imgPause.invalidate();
		imgResume.invalidate();
		imgShutdown.invalidate();
			
		if(mSupportUIBC)
			imgUibc.invalidate();
		
		sfView.invalidate();
	}
	
	public boolean onTouchEvent(MotionEvent event) {
		// Log.d(TAG,"onTouch");
		if (mGd != null && mSupportUIBC) {
			return mGd.onTouchEvent(event);
		}

		int action = event.getAction();
		switch (action) {
		case MotionEvent.ACTION_DOWN:
			break;

		case MotionEvent.ACTION_UP:
			Log.d(TAG, "SHOW MENU");

			// surfaceClick = 1;

			// if(getUIBCState()==false)
			{
				if (getWFDPlayStatus() == WFDPLAY_STATUS_PLAY) {
					Log.d(TAG, "WFDPLAY_STATUS_PLAY");
					imgPause.setVisibility(View.VISIBLE);
					imgResume.setVisibility(View.INVISIBLE);
				} else if (getWFDPlayStatus() == WFDPLAY_STATUS_PAUSE) {
					Log.d(TAG, "WFDPLAY_STATUS_PAUSE");
					imgPause.setVisibility(View.INVISIBLE);
					imgResume.setVisibility(View.VISIBLE);
				}

				imgShutdown.setVisibility(View.VISIBLE);
				if (mSupportUIBC)
					imgUibc.setVisibility(View.VISIBLE);

				imgPause.invalidate();
				imgResume.invalidate();
				imgShutdown.invalidate();
				if (mSupportUIBC)
					imgUibc.invalidate();
				sfView.invalidate();

				if (mSystemUiHider != null) {
					mSystemUiHider.delay();
				}
			}

			break;
		}

		return true;

	};
	
	private void readySurfaceview() {
		sfView = (SurfaceView) findViewById(R.id.videoView);
		
		mSystemUiHider = new SystemUiHider(sfView);
        mSystemUiHider.setup(getWindow());
        //sfView.setOnTouchListener(mTouchListener);
		
		mSHolder = sfView.getHolder();
		//mSHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);

		mSHolder.addCallback(new SurfaceHolder.Callback() {
		
			public void surfaceDestroyed(SurfaceHolder holder) {

				Log.d(TAG, "surfaceDestroyed()");
			}

			public void surfaceCreated(SurfaceHolder holder) {
				Log.d(TAG, "surfaceCreated()");
				mPlayer.setDisplay(holder);
				prepareMP();
			}

			public void surfaceChanged(SurfaceHolder holder, int format,
					int width, int height) {
				Log.d(TAG, "surfaceChanged()");
				// TODO Auto-generated method stub

			}
		});
	}

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION.equals(action)) {
                
            }else if (WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION.equals(action)) {

            }else if (WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION.equals(action)) {
            	Log.d(TAG,"WIFI_P2P_CONNECTION_CHANGED_ACTION");
                if (TelechipsWFDSink.getInstance().mWifiP2pManager == null) 
                	return;
                NetworkInfo networkInfo = (NetworkInfo) intent.getParcelableExtra(WifiP2pManager.EXTRA_NETWORK_INFO);
                if (TelechipsWFDSink.getInstance().mWifiP2pManager != null) {
                   	TelechipsWFDSink.getInstance().mWifiP2pManager.requestGroupInfo(TelechipsWFDSink.getInstance().mChannel, TelechipsWFDSink.getInstance());
                }
                if (networkInfo.isConnected()) {
                    Log.d(TAG, "---------------------- Connected");
                } else {
                	if(bPrepared==true) {
	                	Log.d(TAG, "---------------------- Disconnected");
	                    //start a search when we are disconnected
	                	Log.d(TAG,"disconnected: scanWifiDisplays");
	                	//mWFDPlayEnd = true;
	                	setWFDPlayStatus(WFDPLAY_STATUS_PLAYEND);
	                	MediaPlayerClose();
	                	onBackPressed();
	                	setWFDPlayStatus(WFDPLAY_STATUS_SHUTDOWN);
                	}
                }

            }
        }
    };
    
	public void onResume() {
		super.onResume();
		Log.d(TAG, "onResume");
		//mWFDPlayError = false;
		setWFDPlayStatus(WFDPLAY_STATUS_PLAYERROR);
        mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION);
        mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION);
        mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION);
        mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION);
        registerReceiver(mReceiver, mIntentFilter);
	}

	public void onPause() {
		super.onPause();
		Log.d(TAG, "onPause");			
		
		if(mPlayTimeThread!=null)
			mPlayTimeThread.setRunningState(false);

		/*
		if(mFrameSkipAdjustThread!=null)
			mFrameSkipAdjustThread.setRunningState(false);
		*/
		
		if(mBackPressed==false)
			onBackPressed();
		mBackPressed=false;
		
		TelechipsWFDSink.getInstance().onBackPressed();
		setWFDPlayStatus(WFDPLAY_STATUS_NONE);
	}
    
    
    public void onDestroy()
    {
    	Log.e(TAG, "onDestroy");
    	super.onDestroy();
    	
    	MediaPlayerClose();
    	
		try{
			unregisterReceiver(mReceiver);
		}catch(IllegalArgumentException ex){
			
		}
    }
    
    public void onBackPressed() {
    	super.onBackPressed();
    	mBackPressed = true;
    	Log.e(TAG, "onBackPressed");
    	
    	MediaPlayerClose();
    	
    	try{
			unregisterReceiver(mReceiver);
		}catch(IllegalArgumentException ex){
			
		}
		Intent intent = new Intent();
		intent.putExtra("WFDPlay_result", "onBackPressed");
		this.setResult(RESULT_OK, intent);
		setWFDPlayStatus(WFDPLAY_STATUS_PLAYBACKKEY);
		if(SystemProperties.get("tcc.solution.mbox").equals("1"))
			TelechipsWFDSink.getInstance().mWifiManager.setWifiEnabled(true);
		
		TelechipsWFDSink.getInstance().clearProperty();
    	finish();
    }

	public void onVideoSizeChanged(MediaPlayer mp, int width, int height) {
		Log.d(TAG, "onVideoSizeChanged() Width: " + width
				+ " Height: " + height);
		bVideo = true;
		mWidth = width;
		mHeight = height;
		playMP();
	}

	public void onPrepared(MediaPlayer mp) {
		Log.d(TAG, "onPrepared()");
		//pb.setVisibility( View.INVISIBLE );
		//Toast.makeText(this, "Start play", Toast.LENGTH_SHORT).show();
		bPrepared = true;
		playMP();
	}

	private boolean bVideo = false;
	private boolean bPrepared = false;

	private void playMP() {
		Log.d(TAG, "playMP()");
		if (bVideo == true && bPrepared == true) {
			mSHolder.setFixedSize(mWidth, mWidth);
			try {
				mPlayer.start();
				Intent intent = new Intent();
				intent.putExtra("WFDPlay_result", "ok");
				this.setResult(RESULT_OK, intent);
				setWFDPlayStatus(WFDPLAY_STATUS_PLAY);
				
				//if(SystemProperties.get(WifiDisplayAdvancedSettings.PROPERTY_DEBUG_MODE).equals("1")) 
				{
					mPlayTimeThread = new PlayTimeThread();
					mPlayTimeThread.start();
					
				}

				/*
				if(SystemProperties.get(WifiDisplayAdvancedSettings.PROPERTY_DYNAMIC_FRAME_SKIP).equals("1")) {
					mFrameSkipAdjustThread = new FrameSkipAdjustThread();
					mFrameSkipAdjustThread.start();
				}
				*/

				//dismissConnectionProgressDialog();
				/*
				if(WFDPlay.getInstance().pd!=null){
					if(WFDPlay.getInstance().pd.isShowing()==true)
						WFDPlay.getInstance().pd.dismiss();
				}
				*/
				bPlayed = true;
			} catch (Exception e) {
				playError(e.getMessage()+" 2");
			}
		}

	}

	public void onCompletion(MediaPlayer mp) {
		Log.d(TAG, "onCompletion()");
		
		//if(!bOnError)
		//	Toast.makeText(this, "Play completed", Toast.LENGTH_LONG).show();
		
		MediaPlayerClose();
		
		onBackPressed();
	}

	public void onBufferingUpdate(MediaPlayer mp, int percent) {
		Log.d(TAG, "onBufferingUpdate() percent: " + percent);
	}

	public boolean onInfo(MediaPlayer arg0, int arg1, int arg2) {
		//Log.d(TAG, "onInfo: arg1:" + arg1 + " arg2: " + arg2);
		if(arg1 == ONINFO_PAUSE) {
			Log.d(TAG,"pause");
			if(mPlayer!=null){
				mPlayer.pause();
				Log.d(TAG,"pause end");
				setWFDPlayStatus(WFDPLAY_STATUS_PAUSE);
			}

		} else if(arg1 == ONINFO_RESUME) {
			Log.d(TAG,"resume");
			if(mPlayer!=null) {
				mPlayer.start();
				Log.d(TAG,"resume end");
				mvTvStanby.setText("");
				mvTvStanby.setBackgroundColor(Color.TRANSPARENT);
				setWFDPlayStatus(WFDPLAY_STATUS_PLAY);
			}
		} else if (arg1 == ONINFO_WFD_STANDBY) {
			mvTvStanby.setText("@ WFDSink @\nStanby mode");
			mvTvStanby.setBackgroundColor(Color.BLACK);
			if(mPlayer!=null){
				mPlayer.pause();
				Log.d(TAG,"pause end");
				setWFDPlayStatus(WFDPLAY_STATUS_PAUSE);
			}
		} else if (arg1 ==  ONINFO_WFD_UIBC) {
			mSupportUIBC = true;
			imgUibc.setOnTouchListener(uibcTouchListener);

		} else if(arg1 == ONINFO_PACKETLOSS) {
			if(mPlayTimeThread!=null && mPlayTimeThread.getPlayTime() > 5) {
				//Log.d(TAG,"packet loss = " + arg2);
				if(arg2 == 0)
					return true;
				
				/* */
				if(SystemProperties.get(WifiDisplayAdvancedSettings.PROPERTY_DEBUG_MODE).equals("1")) {
			    	SimpleDateFormat mSimpleDateFormat = new SimpleDateFormat ( "HH:mm:ss", Locale.KOREA );
			    	Date currentTime = new Date(System.currentTimeMillis());
			    	String mTime = mSimpleDateFormat.format ( currentTime );
				
			    	if(countLines(mPacketLossInfo)==20) {
			    		mPacketLossInfo = "["+mTime+"]" + " packet loss = " + arg2+"\n";
			    	} else {
			    		if(mPacketLossInfo==null)
			    			mPacketLossInfo = "["+mTime+"]" + " packet loss = " + arg2+"\n";
			    		else
			    			mPacketLossInfo += "["+mTime+"]" + " packet loss = " + arg2+"\n";
			    	}
				
					mvTvPacketloss.setText(mPacketLossInfo);
					mTotalPacketLossNum += arg2;
					mvTvTotalPacketLossNum.setBackgroundColor(Color.RED);
					mvTvTotalPacketLossNum.setText("The number of Packet Loss = " + mTotalPacketLossNum);
					
					/*
					mvTvPacketlossBarTop.setBackgroundColor(Color.RED);
					mvTvPacketlossBarTop.setText(" ");
					mvTvPacketlossBarBottom.setBackgroundColor(Color.RED);
					mvTvPacketlossBarBottom.setText(" ");
					mvTvPacketlossBarLeft.setBackgroundColor(Color.RED);
					mvTvPacketlossBarLeft.setText("      ");
					mvTvPacketlossBarRight.setBackgroundColor(Color.RED);
					mvTvPacketlossBarRight.setText("      ");
					*/										
					//TelechipsWFDSink.sleep(1);
					//mvTvPacketlossBar.setText("");
					//mvTvPacketlossBar.setBackgroundColor(Color.TRANSPARENT);
				
				}
			}
			/* */
		} else if(arg1 == ONINFO_AUDIO_FRAME_SKIP || arg1 == ONINFO_VIDEO_FRAME_SKIP) {
			if(SystemProperties.get(WifiDisplayAdvancedSettings.PROPERTY_DEBUG_MODE).equals("1")) {
				//Log.d(TAG,"frame skip");
				if( mPlayTimeThread.pt < 3 )
					return false;
				
				if(arg1 == ONINFO_AUDIO_FRAME_SKIP) {
					mvTvAudioSkipNum.setBackgroundColor(Color.BLUE);
					mvTvAudioSkipNum.setText("Number of Audio drop = " + ++mTotalAudioFrameSkipNum);
					
					/*
					mvTvPacketlossBarTop.setBackgroundColor(Color.BLUE);
					mvTvPacketlossBarTop.setText(" ");
					mvTvPacketlossBarBottom.setBackgroundColor(Color.BLUE);
					mvTvPacketlossBarBottom.setText(" ");
					mvTvPacketlossBarLeft.setBackgroundColor(Color.BLUE);
					mvTvPacketlossBarLeft.setText("      ");
					mvTvPacketlossBarRight.setBackgroundColor(Color.BLUE);
					mvTvPacketlossBarRight.setText("      ");
					*/
				} else if(arg1 == ONINFO_VIDEO_FRAME_SKIP) {
					mvTvVideoSkipNum.setBackgroundColor(Color.BLUE);
					mvTvVideoSkipNum.setText("Number of Video drop = " + ++mTotalVideoFrameSkipNum);
					
					/*
					mvTvPacketlossBarTop.setBackgroundColor(Color.BLUE);
					mvTvPacketlossBarTop.setText(" ");
					mvTvPacketlossBarBottom.setBackgroundColor(Color.BLUE);
					mvTvPacketlossBarBottom.setText(" ");
					mvTvPacketlossBarLeft.setBackgroundColor(Color.BLUE);
					mvTvPacketlossBarLeft.setText("      ");
					mvTvPacketlossBarRight.setBackgroundColor(Color.BLUE);
					mvTvPacketlossBarRight.setText("      ");
					*/
				}
				/*
				if(mAudioSkipToast==null) {
					mAudioSkipToast = Toast.makeText(this, "Audio Skip", Toast.LENGTH_LONG);
					mAudioSkipToast.show();
					mPacketLossToastTimeoutThread = new TimeoutThread(1,3);
					mPacketLossToastTimeoutThread.start();
				}
				*/
			}
		} else if (arg1 == ONINFO_WFD_HDCP_RETRY) {
			if (arg2 == ONINFO_WFD_ERROR_HDCP_FAIL_RETRY) {
			/*
				AlertDialog.Builder alertDlg2 = new AlertDialog.Builder(
						getInstance());
				alertDlg2.setTitle("WFD Sink");
				alertDlg2.setMessage("HDCP Connect failed, retry no HDCP connection");
				alertDlg2.setPositiveButton("ok",
						new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog,
									int whichButton) {
							}
						});
				alertDlg2.show();
				*/
				WFDPlay.getInstance().errorDialog = new Dialog(WFDPlay.getInstance());
				WFDPlay.getInstance().errorDialog.setContentView(R.layout.customdialog);
				WFDPlay.getInstance().errorDialog.setTitle("WFD Sink");
				TextView text = (TextView) WFDPlay.getInstance().errorDialog.findViewById(R.id.text);
				text.setText("HDCP Connect failed, retry no HDCP connection");
				WFDPlay.getInstance().errorDialog.show();
				WFDPlay.getInstance().handler.sendEmptyMessageDelayed(WFDPlay.getInstance().MESSAGE_WFDPLAY_CONNECTION_RETRY, 1000);
			}
		} else if(arg1 == ONINFO_WFD_ERROR) {
			if (bPlayed == false && bErrorMesg == false) {
				if (arg2 == ONINFO_WFD_ERROR_CONNECT_FAIL) {
					// dismissConnectionProgressDialog();
					
					/*
					AlertDialog.Builder alertDlg2 = new AlertDialog.Builder(
							getInstance());
					alertDlg2.setTitle("WFD Sink");
					alertDlg2.setMessage("Connection fail");
					alertDlg2.setPositiveButton("ok",
							new DialogInterface.OnClickListener() {
								public void onClick(DialogInterface dialog,
										int whichButton) {
									playError("Play error 1");
									setWFDPlayStatus(WFDPLAY_STATUS_PLAYERROR);
									onBackPressed();
								}
							});
					alertDlg2.show();
					*/
					WFDPlay.getInstance().errorDialog = new Dialog(WFDPlay.getInstance());
					WFDPlay.getInstance().errorDialog.setContentView(R.layout.customdialog);
					WFDPlay.getInstance().errorDialog.setTitle("WFD Sink");
					TextView text = (TextView) WFDPlay.getInstance().errorDialog.findViewById(R.id.text);
					text.setText("Connection fail");
					WFDPlay.getInstance().errorDialog.show();
					WFDPlay.getInstance().handler.sendEmptyMessageDelayed(WFDPlay.getInstance().MESSAGE_WFDPLAY_ERROR,TelechipsWFDSink.getInstance().POPUP_TIMEOUT);
				} else if (arg2 >= ONINFO_WFD_ERROR_NEGO_FAIL_M1
						&& arg2 <= ONINFO_WFD_ERROR_CONNECT_ABORT) {
					// dismissConnectionProgressDialog();
					/*
					AlertDialog.Builder alertDlg2 = new AlertDialog.Builder(
							getInstance());
					alertDlg2.setTitle("WFD Sink");
					if (arg2 == ONINFO_WFD_ERROR_CONNECT_ABORT)
						alertDlg2.setMessage("Negotiation fail: " + arg2);
					else
						alertDlg2.setMessage("Negotiation fail: " + arg2);
					alertDlg2.setPositiveButton("ok",
							new DialogInterface.OnClickListener() {
								public void onClick(DialogInterface dialog,
										int whichButton) {
									playError("Play error 2");
									setWFDPlayStatus(WFDPLAY_STATUS_PLAYERROR);
									onBackPressed();
								}
							});
					alertDlg2.show();
					*/
					WFDPlay.getInstance().errorDialog2 = new Dialog(WFDPlay.getInstance());
					WFDPlay.getInstance().errorDialog2.setContentView(R.layout.customdialog);
					WFDPlay.getInstance().errorDialog2.setTitle("WFD Sink");
					TextView text = (TextView) WFDPlay.getInstance().errorDialog2.findViewById(R.id.text);
					text.setText("Negotiation fail: " + arg2);
					WFDPlay.getInstance().errorDialog2.show();
					WFDPlay.getInstance().handler.sendEmptyMessageDelayed(WFDPlay.getInstance().MESSAGE_WFDPLAY_ERROR,TelechipsWFDSink.getInstance().POPUP_TIMEOUT);
				}
			}
			bErrorMesg = true;
		}
		// TODO Auto-generated method stub
		return true;
	}
	
	private boolean bOnError=false;
	public boolean onError(MediaPlayer arg0, int arg1, int arg2) {
		Log.e(TAG, "onError arg1 = " + arg1 + " arg2 = " + arg2);
		//if(bPrepared==true) 
		{
			bOnError = true;
			/*
			AlertDialog.Builder alertDlg2 = new AlertDialog.Builder(getInstance());
			alertDlg2.setTitle("WFD Sink");
			alertDlg2.setMessage("Play error");
			alertDlg2.setPositiveButton("ok",
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog,
								int whichButton) {
							playError("Play error 3");    
							setWFDPlayStatus(WFDPLAY_STATUS_PLAYERROR);
							onBackPressed();
						}
					});
			alertDlg2.show();
			*/
			WFDPlay.getInstance().errorDialog = new Dialog(WFDPlay.getInstance());
			WFDPlay.getInstance().errorDialog.setContentView(R.layout.customdialog);
			WFDPlay.getInstance().errorDialog.setTitle("WFD Sink");
			TextView text = (TextView) WFDPlay.getInstance().errorDialog.findViewById(R.id.text);
			text.setText("Play error");

			if(arg2 == ONINFO_WFD_ERROR_CONNECT_ABORT){
				text.setText("Connection error");
			} else {
				text.setText("Play error");
			}

			WFDPlay.getInstance().errorDialog.show();
			WFDPlay.getInstance().handler.sendEmptyMessageDelayed(WFDPlay.getInstance().MESSAGE_WFDPLAY_ERROR,TelechipsWFDSink.getInstance().POPUP_TIMEOUT);
		}
		return true;
	}
	
	private OnTouchListener pauseTouchListener = new OnTouchListener(){
		public boolean onTouch(View v, MotionEvent event){

				ImageView view = (ImageView)v;
				if(event.getAction() == MotionEvent.ACTION_DOWN){
				
		            if (mSystemUiHider != null) {
		                mSystemUiHider.cancelDelay();
		            }
					view.setPadding(2,2,2,2);
					view.setColorFilter(0xaa111111, Mode.SRC_OVER);
				}else if(event.getAction() == MotionEvent.ACTION_UP){
					imgPause.setVisibility(View.INVISIBLE);
					imgResume.setVisibility(View.VISIBLE);
					sfView.invalidate();
					imgPause.invalidate();
					imgResume.invalidate();
					view.setPadding(0,0,0,0);
					view.setColorFilter(0x00000000, Mode.SRC_OVER);
					setWFDPlayStatus(WFDPLAY_STATUS_PAUSE);
					if(!getPauseResumeProcessing()) {
						setPauseResumeProcessing(true);
						Log.d(TAG,"pause");				
						if(mPlayer!=null)
							mPlayer.pause();
						Log.d(TAG,"pause end");
						setPauseResumeProcessing(false);
					}
				}
			return true;
		}
	};
	
	private OnTouchListener resumeTouchListener = new OnTouchListener(){
		public boolean onTouch(View v, MotionEvent event){
				ImageView view = (ImageView)v;
	
				if(event.getAction() == MotionEvent.ACTION_DOWN){
					
		            if (mSystemUiHider != null) {
		                mSystemUiHider.cancelDelay();
		            }
					view.setPadding(2,2,2,2);
					view.setColorFilter(0xaa111111, Mode.SRC_OVER);
				}else if(event.getAction() == MotionEvent.ACTION_UP){
					imgPause.setVisibility(View.VISIBLE);
					imgResume.setVisibility(View.INVISIBLE);
					sfView.invalidate();
					imgPause.invalidate();
					imgResume.invalidate();
					view.setPadding(0,0,0,0);
					view.setColorFilter(0x00000000, Mode.SRC_OVER);
					setWFDPlayStatus(WFDPLAY_STATUS_PLAY);
					//surfaceClick = 0;
					if(!getPauseResumeProcessing()) {
						setPauseResumeProcessing(true);
						Log.d(TAG,"resume");
						if(mPlayer!=null)
							mPlayer.start();
						setPauseResumeProcessing(false);
					hideMenu();
					}
				}
			return true;
		}
	};
	
	private OnTouchListener shutdownTouchListener = new OnTouchListener(){
		public boolean onTouch(View v, MotionEvent event){
				ImageView view = (ImageView)v;
	
				if(event.getAction() == MotionEvent.ACTION_DOWN){
					view.setPadding(2,2,2,2);
					view.setColorFilter(0xaa111111, Mode.SRC_OVER);
				}else if(event.getAction() == MotionEvent.ACTION_UP){				
					view.setPadding(0,0,0,0);
					view.setColorFilter(0x00000000, Mode.SRC_OVER);
					
					MediaPlayerClose();
	
					onBackPressed();
					setWFDPlayStatus(WFDPLAY_STATUS_SHUTDOWN);
				}
			return true;
		}
	};
	
	
	private OnTouchListener uibcTouchListener = new OnTouchListener(){
		public boolean onTouch(View v, MotionEvent event){
			ImageView view = (ImageView)v;
			
			if(event.getAction() == MotionEvent.ACTION_DOWN){
				view.setPadding(2,2,2,2);
				view.setColorFilter(0xaa111111, Mode.SRC_OVER);
			}else if(event.getAction() == MotionEvent.ACTION_UP){				
				view.setPadding(0,0,0,0);
				view.setColorFilter(0x00000000, Mode.SRC_OVER);
				imgPause.setVisibility(View.VISIBLE);
				imgResume.setVisibility(View.VISIBLE);
				if(mSupportUIBC)
					imgUibc.setVisibility(View.VISIBLE);
				sfView.invalidate();
				imgPause.invalidate();
				imgResume.invalidate();
				if(mSupportUIBC)
					imgUibc.invalidate();
				//imgPause.performClick();
				//setWFDPlayStatus(WFDPLAY_STATUS_PAUSE);

				//WFDPlay.getInstance().mUibcEnabled = true;
				AlertDialog.Builder alertDlg = new AlertDialog.Builder(
						WFDPlay.getInstance());
				alertDlg.setTitle("User Input Back Channel");
				alertDlg.setMessage("Do you want to control source device?\nIf so, click ok.\nAnd if you want to back to normal mode, please double touch on screen rapidly.");
				alertDlg.setPositiveButton("ok",
						new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog,
									int whichButton) {

								WFDPlay.getInstance().mUibcEnabled = true;
								//if (mSystemUiHider != null) {
								//	mSystemUiHider.delay();
								//}
								initUIBC();
								//goneMenu();
								//
							}
						});
				alertDlg.setNegativeButton("Cancel",
						new DialogInterface.OnClickListener() {

							public void onClick(DialogInterface dialog,
									int which) {
								dialog.cancel();
								// imgPause.performClick();
								// setWFDPlayStatus(WFDPLAY_STATUS_PLAY);
								//if (mSystemUiHider != null) {
								//	mSystemUiHider.delay();
								//}
								WFDPlay.getInstance().mUibcEnabled = false;
							}
						});

				alertDlg.show();
				
			}
			return true;
		}
	};
	
	private void setPauseResumeProcessing(boolean set){
		Log.d("WFDPlay","setPauseResumeProcessing " + set);
		bPauseResumeIng = set;
	}

	private boolean getPauseResumeProcessing(){
		return bPauseResumeIng;
	}

	void initUIBC() {
		setUIBCState(true);
		mGd = new GestureDetector(this, new onGestureListener());
		Parcel parcel = Parcel.obtain();

		if(mPlayer!=null)
			mPlayer.setUIBCParameter(ENABLE_UIBC,parcel);
	}
	
	public static final WFDPlay getInstance() {
		if (instance != null)
			return instance;
		Log.e("WFDPlay","instance is null");
		return null;
	}
	
	public static void setWFDPlayStatus(int status){
		mWfdPlayStatus = status;
	}
	
	public static int getWFDPlayStatus(){
		return mWfdPlayStatus;
	}
	
	void MediaPlayerClose() {
		Log.d(TAG, "MediaPlayerClose");
		if(WFDPlay.getInstance().mPlayer!=null) {
			Log.d(TAG, "stop");
			WFDPlay.getInstance().mPlayer.stop();
			Log.d(TAG, "release");
			WFDPlay.getInstance().mPlayer.release();
			Log.d(TAG, "release end");
		}
		WFDPlay.getInstance().mPlayer = null;
	}

	void setUIBCState(boolean state) {
		mUibcEnabled = state;
	}
	
	boolean getUIBCState() {
		return mUibcEnabled;
	}

    
	private final class onGestureListener extends GestureDetector.SimpleOnGestureListener
    {
		private float x,x2;
		private float y,y2;
		private Parcel parcel;

		@Override
		public boolean onDoubleTap(MotionEvent e) {
			// TODO Auto-generated method stub
			x = e.getX();
			y = e.getY();
			
			Log.d(TAG, "[UIBC] onDoubleTap x = " + x + " y = " + y);
			
			AlertDialog.Builder alertDlg = new AlertDialog.Builder(
					WFDPlay.getInstance());
			alertDlg.setTitle("Cancel User Input Back Channel");
			alertDlg.setMessage("Back to the normal mode.");
			alertDlg.setPositiveButton("ok",
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog,
								int whichButton) {
							mGd = null;
							setUIBCState(false);
							Parcel parcel = Parcel.obtain();

							if(mPlayer!=null)
								mPlayer.setUIBCParameter(DISABLE_UIBC,parcel);
						}
					});
			alertDlg.show();

			return super.onDoubleTap(e);
		}

		@Override
		public boolean onDown(MotionEvent e) {
			x = e.getX();
			y = e.getY();
			Log.d(TAG, "[UIBC] onDown x = " + x + " y = " + y);
			parcel = Parcel.obtain();
			parcel.writeFloat(x);
			parcel.writeFloat(y);

			if(mPlayer!=null)
				mPlayer.setUIBCParameter(SET_PARAMETER_UIBC_DOWN,parcel);
			
			return true;
		}

		@Override
		public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
			x = e1.getX();
			y = e1.getY();
			x2 = e2.getX();
			y2 = e2.getY();

			Log.d(TAG, "[UIBC] onFling  x = " + x + " y = " + y +  " x2 = " + x2 + " y2 = " + y2 + " velocityX = " + velocityX + " velocityY = " + velocityY);

            parcel = Parcel.obtain();
            parcel.writeFloat(x2);
            parcel.writeFloat(y2);

            if(mPlayer!=null)
                mPlayer.setUIBCParameter(SET_PARAMETER_UIBC_UP,parcel);			
			return true;
		}

		@Override
		public void onLongPress(MotionEvent e) {
			x = e.getX();
			y = e.getY();
			Log.d(TAG, "[UIBC] onLongPress x = " + x + " y = " + y);

		}

		public void onShowPress(MotionEvent e) {
			x = e.getX();
			y = e.getY();
			Log.d(TAG, "[UIBC] onShowPress x = " + x + " y = " + y);

		}

		@Override
		public boolean onScroll(MotionEvent e1, MotionEvent e2,	float distanceX, float distanceY) {
			x = e1.getX();
			y = e1.getY();
			x2 = e2.getX();
			y2 = e2.getY();
			
			parcel = Parcel.obtain();
			parcel.writeFloat(x2);
			parcel.writeFloat(y2);

			if(mPlayer!=null)
				mPlayer.setUIBCParameter(SET_PARAMETER_UIBC_MOVE,parcel);
			Log.d(TAG, "[UIBC] onScroll  x = " + x + " y = " + y +  " x2 = " + x2 + " y2 = " + y2 + " distanceX = " + distanceX + " distanceY = " + distanceY);
			return true;
		}

		@Override
		public boolean onSingleTapUp(MotionEvent e) {
			x = e.getX();
			y = e.getY();
			
			parcel = Parcel.obtain();
			parcel.writeFloat(x);
			parcel.writeFloat(y);

			if(mPlayer!=null)
				mPlayer.setUIBCParameter(SET_PARAMETER_UIBC_UP,parcel);
				
			Log.d(TAG, "[UIBC] onSingleTapUp x = " + x + " y = " + y);
			return true;
		}
	}
	
	private static int countLines(String str){
		if(str == null)
			return 0;
		
		String[] lines = str.split("\r\n|\r|\n");
		return  lines.length;
	}
	
	public class PlayTimeThread extends Thread {
		public int pt=0;
		private boolean isRunning = true;
		
		public PlayTimeThread() {
			super();
		}
		
		public int getPlayTime() {
			return pt;
		}
		public void setRunningState(boolean state) {
			isRunning = state;
		}		
		public void run() {
			while (isRunning==true) {
				sendMessage(HANDLER_PLAYTIME,pt++);
				TelechipsWFDSink.sleep(1);
			}
		}
	}
	
	
	public Handler handler = new Handler() {
		public void handleMessage(Message msg) {
			switch (msg.what) {
				case HANDLER_PLAYTIME: {
					if(SystemProperties.get(WifiDisplayAdvancedSettings.PROPERTY_DEBUG_MODE).equals("1")) 
						WFDPlay.getInstance().mvTvPlayTime.setText("Play Time : " + msg.getData().getInt("MESSAGE") + " secs");
					/*
					mvTvPacketlossBarTop.setText("");
					mvTvPacketlossBarTop.setBackgroundColor(Color.TRANSPARENT);
					mvTvPacketlossBarBottom.setText("");
					mvTvPacketlossBarBottom.setBackgroundColor(Color.TRANSPARENT);
					mvTvPacketlossBarLeft.setText("");
					mvTvPacketlossBarLeft.setBackgroundColor(Color.TRANSPARENT);
					mvTvPacketlossBarRight.setText("");
					mvTvPacketlossBarRight.setBackgroundColor(Color.TRANSPARENT);
					*/
					mvTvAudioSkipNum.setBackgroundColor(Color.TRANSPARENT);
					mvTvVideoSkipNum.setBackgroundColor(Color.TRANSPARENT);
					mvTvTotalPacketLossNum.setBackgroundColor(Color.TRANSPARENT);
					break;
				}
				/*
				case HANDLER_UI_FRAME_SKIP_UPDATE:
					mvTvAudioSkipNum.setText("The number of Frame Skip("+ Integer.toString(WFDPlay.getInstance().iCurrentFrameSkipTime) + "ms) = " + ++mTotalFrameSkipNum);
					break;
				*/
				case MESSAGE_WFDPLAY_ERROR:
					Log.d(TAG,"MESSAGE_WFDPLAY_ERROR");
					if(WFDPlay.getInstance().errorDialog!=null)
						WFDPlay.getInstance().errorDialog.dismiss();
					WFDPlay.getInstance().onBackPressed();
					break;		
				case MESSAGE_WFDPLAY_CONNECTION_RETRY:
					Log.d(TAG,"MESSAGE_WFDPLAY_CONNECTION_RETRY");
					if(WFDPlay.getInstance().errorDialog!=null)
						WFDPlay.getInstance().errorDialog.dismiss();
					break;
			}
		}
	};

	public void sendMessage(int what) {
		Message msg = new Message();
		msg.what = what;
		handler.sendMessage(msg);
	}

	public void sendMessage(int what, int message) {
		Message msg = new Message();
		Bundle data = new Bundle();
		data.putInt("MESSAGE", message);
		msg.what = what;
		msg.setData(data);
		handler.sendMessage(msg);
	}
	
	private class TimeoutThread extends Thread {
		private int mTimeout; // sec
		private boolean isRunning = true;

		private int type;
		public TimeoutThread(int t, int timeout) {
			super();
			type = t;
			mTimeout = timeout;
		}
		
		public void setRunningState(boolean state) {
			isRunning = state;
			Log.d(TAG,"setRunningState = " + isRunning);
		}
		
		public void run() {

			while (mTimeout > 0 && isRunning==true) {
				mTimeout--;
				TelechipsWFDSink.sleep(1);
			}
			
			if(type==0)
				WFDPlay.getInstance().mPacketLossToast = null;
			else if(type==1)
				WFDPlay.getInstance().mAudioSkipToast = null;
		}
	}
	/*
	void createConnectionProgressDialog() {
		Log.d(TAG,"createConnectionProgressDialog");
		if (WFDPlay.getInstance().pd != null && WFDPlay.getInstance().pd.isShowing() == true)	// block twice pop-up, 20130418
			return;
		
		WFDPlay.getInstance().pd = ProgressDialog.show(WFDPlay.getInstance(), 
			TelechipsWFDSink.getInstance().mRes.getText(R.string.wfdplay_connection_dialog_title), 
			TelechipsWFDSink.getInstance().mRes.getText(R.string.wfdplay_connection_dialog_message), true, false);
		
		
		Thread thread = new Thread(new Runnable() {
			int i;
			public void run() {
				for (i = 0; i < CONNECTION_TIMEOUT; i++) {
					TelechipsWFDSink.sleep(1);
					Log.d(TAG, "Connection timeout thread = " + i);
					if (WFDPlay.getInstance().pd != null && WFDPlay.getInstance().pd.isShowing() == false)
						break;
				}

				if (WFDPlay.getInstance().pd != null && WFDPlay.getInstance().pd.isShowing() == true) {
					Log.e(TAG, "preparing timeout");

					//dismissConnectionProgressDialog();
					
				}
			}
		});
		thread.start();
	}
	
	void dismissConnectionProgressDialog() {
		Log.d(TAG,"dismissConnectionProgressDialog");
		if(WFDPlay.getInstance().pd.isShowing()==true)
			WFDPlay.getInstance().pd.dismiss();
	}
	 */
	static public int FRAME_SKIP_DIFFERENCE_NUMBER = 5;
	static public int FRAME_SKIP_ADJUST_NUMBER = 10;
	static public int FRAME_SKIP_DROP_COUNT = 25;
	static public int FRAME_SKIP_DROP_NUMBER = 25;
	static public int FRAME_SKIP_MAX = 600;
	static public int FRAME_SKIP_MIN = 300;
	
	/*
	private class FrameSkipAdjustThread extends Thread {
		
		private int savedFrameSkipNum=0;

		private boolean isRunning = true;
		int frameSkipDiff=0;
		int frameSkipOkNum=0;
		
		public FrameSkipAdjustThread() {
			super();
		}
		         
		public void setRunningState(boolean state) {
			isRunning = state;
		}
		
		public void run() {
			while(isRunning) {
				frameSkipDiff = mTotalFrameSkipNum - savedFrameSkipNum;
				//Log.d(TAG,"current = " + Integer.toString(WFDPlay.getInstance().iCurrentFrameSkipTime) + "frameSkipDiff = " +frameSkipDiff +" frameSkipOkNum = " +frameSkipOkNum);
				if(frameSkipDiff > FRAME_SKIP_DIFFERENCE_NUMBER && WFDPlay.getInstance().iCurrentFrameSkipTime < FRAME_SKIP_MAX) {
					WFDPlay.getInstance().iCurrentFrameSkipTime += frameSkipDiff*FRAME_SKIP_ADJUST_NUMBER;
					SystemProperties.set(WifiDisplayAdvancedSettings.PROPERTY_FRAME_SKIP_TIME,	Integer.toString(WFDPlay.getInstance().iCurrentFrameSkipTime));
					Log.d(TAG,"Frame Skip Time increase: " + Integer.toString(WFDPlay.getInstance().iCurrentFrameSkipTime));
					sendMessage(HANDLER_UI_FRAME_SKIP_UPDATE);
					frameSkipOkNum = 0;
				}else {
					frameSkipOkNum++;
					if(frameSkipOkNum == FRAME_SKIP_DROP_COUNT && WFDPlay.getInstance().iCurrentFrameSkipTime > FRAME_SKIP_MIN) {
						WFDPlay.getInstance().iCurrentFrameSkipTime -= FRAME_SKIP_DROP_NUMBER;
						
						SystemProperties.set(WifiDisplayAdvancedSettings.PROPERTY_FRAME_SKIP_TIME,	Integer.toString(WFDPlay.getInstance().iCurrentFrameSkipTime));
						Log.d(TAG,"Frame Skip Time decrease: " + Integer.toString(WFDPlay.getInstance().iCurrentFrameSkipTime));
						sendMessage(HANDLER_UI_FRAME_SKIP_UPDATE);
						frameSkipOkNum = 0;
					}
				}
				savedFrameSkipNum = mTotalFrameSkipNum;
				
				TelechipsWFDSink.sleep(1);
			}
		}
	}
	*/
	
}
