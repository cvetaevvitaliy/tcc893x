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

import java.io.BufferedReader;

import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.net.wifi.p2p.WifiP2pConfig;
import android.net.wifi.p2p.WifiP2pDevice;
import android.net.wifi.p2p.WifiP2pDeviceList;
import android.net.wifi.p2p.WifiP2pGroup;
import android.net.wifi.p2p.WifiP2pInfo;
import android.net.wifi.p2p.WifiP2pManager;
import android.net.wifi.p2p.WifiP2pManager.ActionListener;
import android.net.wifi.p2p.WifiP2pManager.Channel;
import android.net.wifi.p2p.WifiP2pManager.ConnectionInfoListener;
import android.net.wifi.p2p.WifiP2pManager.DialogListener;
import android.net.wifi.p2p.WifiP2pManager.GroupInfoListener;
import android.net.wifi.p2p.WifiP2pManager.PeerListListener;
import android.net.wifi.p2p.WifiP2pWfdInfo;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;
import android.util.TypedValue;
import android.view.WindowManager;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.LinearLayout.LayoutParams;
import android.widget.TextView;
import android.widget.Toast;
import android.os.SystemProperties;
import android.preference.PreferenceActivity;

@SuppressLint("HandlerLeak")
public class TelechipsWFDSink extends PreferenceActivity implements PeerListListener, GroupInfoListener, ConnectionInfoListener, DialogListener//implements ChannelListener//, DeviceActionListener
{
	private final String TAG = "TelechipsWFDSink";
	
	private static final int P2P_DISCOVER_TIMEOUT = 120; // sec
	private static final int P2P_DISCOVER_TIMEOUT_THREAD_TYPE = 1;
    private static final int DEFAULT_CONTROL_PORT = 7236;
    private static final int MAX_THROUGHPUT = 50;
    private static final int CONNECTION_TIMEOUT = 40;
    public final int POPUP_TIMEOUT = 5000;
    public final int MESSAGE_APP_EXIT = 1;
    public final int MESSAGE_CONNECTION_DIALOG_DISMISS = 2;
    public final int MESSAGE_TIMEOUT = 3;
    public final int MESSAGE_CONNECTION_DIALOG_MSG_UPDATE = 4;
    public final int MESSAGE_RESET_P2P = 6;    
    public final int MESSAGE_POPUP_TIMEOUT = 7;
    public final int MESSAGE_WFDPLAY_ERROR = 8;
    public final int MESSAGE_IP_ASSIGN = 9;
    public final int MESSAGE_IP_GET = 10;
    public final int MESSAGE_RTSP = 11;
    public final int MESSAGE_WFDPLAY_CONNECTION_RETRY = 12;
    
    static public int ONINFO_PAUSE = 1000;
	static public int ONINFO_RESUME = 1001;
	static public int ONINFO_AUDIO_FRAME_SKIP = 1002;
	static public int ONINFO_VIDEO_FRAME_SKIP = 1003;
	static public int ONINFO_WFD_ERROR = 1004;
	static public int ONINFO_WFD_STANDBY  = 1005;
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
	static public int ONINFO_WFD_M7 = 1100;

    public Channel mChannel;
	private IntentFilter mIntentFilter = new IntentFilter();
	public Resources mRes;
	
	TextView myDevice;
	TextView mTVVersion;
    private WifiP2pDevice mThisDevice;
    ImageView translateImageView;
	Animation ani;
	public WifiManager mWifiManager;
	public WifiP2pManager mWifiP2pManager;
    public boolean mWifiP2pEnabled;
	private static TelechipsWFDSink instance;
	
	private TimeoutThread mP2PDispeerThread;
	
	DialogListener dialogListener;
	private ProgressDialog pd;
	
    LinearLayout layout;
    LinearLayout.LayoutParams lp;
    
    public boolean WFDPlayError = false;

    Dialog showPinDialog;
    Dialog notifyDialog;
    public Intent intentAdvancedSettings; 
    private String connectionStatus;
    AlertDialog.Builder alertDlg;

    String mConnectingMessage;
    WifiP2pInfo mConnectP2PInfo;

    static final public int CONNECTION_PROGRESS_INIT = 0;
    static final public int CONNECTION_PROGRESS_P2P_GO_NEG_SUCCESS = 20;
    static final public int CONNECTION_PROGRESS_P2P_GROUP_FORMATION_SUCCESS = 40;
    static final public int CONNECTION_PROGRESS_PERSISTENT = 50;
    static final public int CONNECTION_PROGRESS_GROUP_START = 60;
    static final public int CONNECTION_PROGRESS_IP_PROCESS = 65;
    static final public int CONNECTION_PROGRESS_RTSP_NEGO = 80;
    static final public int CONNECTION_PROGRESS_COMPLETE = 100;
    static final public int CONNECTION_PROGRESS_INCREMENT = 1;

    public int mConnectionStatus;
    static final public int CONNECTION_STATUS_INIT = 0;
    static final public int CONNECTION_STATUS_P2P_GO_NEGO_SUCCESS = 1;
    static final public int CONNECTION_STATUS_P2P_GROUP_FORMATION_SUCCESS = 2;
    static final public int CONNECTION_STATUS_PERSISTENT = 3;
    static final public int CONNECTION_STATUS_P2P_GROUP_START = 4;
    static final public int CONNECTION_STATUS_IP_PROCESS = 5;
    static final public int CONNECTION_STATUS_RTSP_NEGO = 6;
    static final public int CONNECTION_STATUS_COMPLETE = 7;

    static final public int CONNECTION_TIMEOUT_P2P_GO_NEGO_SUCCESS = 20;
    static final public int CONNECTION_TIMEOUT_P2P_GROUP_FORMATION_SUCCESS = 20;
    static final public int CONNECTION_TIMEOUTS_PERSISTENT = 30;
    static final public int CONNECTION_TIMEOUT_IP_PROCESS = 30;
    static final public int CONNECTION_TIMEOUT_RTSP_NEGO = 15;
    static final public int CONNECTION_TIMEOUT_COMPLETE = 30;

	///////////////////////////////////////
	// Member Methods
	///////////////////////////////////////
	
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
		instance = this;
		Log.d(TAG, "onCreate");
		
		final float scale = getResources().getDisplayMetrics().density;
       	Log.d(TAG,"lcd scale = " + scale);
       	
       	int mWidth = getResources().getDisplayMetrics().widthPixels;
       	int mHeight = getResources().getDisplayMetrics().heightPixels;
       	
       	Log.d(TAG,"screen width = " + mWidth);
       	Log.d(TAG,"screen height = " + mHeight);
       	
		LinearLayout fragment1 = (LinearLayout) findViewById(R.id.fragment1);
		LinearLayout.LayoutParams lparam = new LinearLayout.LayoutParams(
	             LayoutParams.WRAP_CONTENT, LayoutParams.FILL_PARENT);
		
        myDevice = (TextView)findViewById(R.id.my_device_describe);
        
		if(mWidth>1000) {
			lparam.leftMargin = 55;
			lparam.rightMargin = 55;
        	myDevice.setTextSize(TypedValue.COMPLEX_UNIT_DIP, 25);
		} else {
			lparam.leftMargin = 40;
			lparam.rightMargin = 40;
			myDevice.setTextSize(TypedValue.COMPLEX_UNIT_DIP, 20);
		}
		
		if(mHeight>500) {
			lparam.topMargin = 50;
		} else {
			lparam.topMargin = 20;
		}
		
		fragment1.setLayoutParams(lparam);
		
		mTVVersion = (TextView)findViewById(R.id.tv_version);

		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		
		if(!SystemProperties.get("tcc.wifi.display.connect").equals("0")) {
			alertDlg = new AlertDialog.Builder(this);
			alertDlg.setTitle("Error");
			alertDlg.setMessage("Wi-Fi Display is already connected");
			alertDlg.setPositiveButton("ok",
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog,
								int whichButton) {
							finish();
						}
					});
			alertDlg.show();
			return;	
		}
		
        mRes = getResources();
        mWifiManager = (WifiManager) getSystemService(Context.WIFI_SERVICE);
		mWifiP2pManager = (WifiP2pManager)getSystemService(Context.WIFI_P2P_SERVICE);

		if (mWifiP2pManager != null) {
			mChannel = mWifiP2pManager.initialize(this,	getMainLooper(), null);

			if (mChannel == null) {
				// Failure to set up connection
				Log.e(TAG, "Failed to set up connection with wifi p2p service");
				mWifiP2pManager = null;
			}
		} else {
			Log.e(TAG, "mWifiP2pManager is null!");
			alertDlg = new AlertDialog.Builder(this);
			alertDlg.setTitle("Error");
			alertDlg.setMessage("don't initialize Wi-Fi P2P service, Please reset Wi-Fi");
			alertDlg.setPositiveButton("ok",
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog,
								int whichButton) {
							finish();
						}
					});
			alertDlg.show();
			handler.sendEmptyMessageDelayed(MESSAGE_POPUP_TIMEOUT,POPUP_TIMEOUT);
			return;	
		}
		
		
       /*
       PackageInfo packageInfo = null;
		try {
			packageInfo = this.getPackageManager().getPackageInfo(this.getPackageName(), 0);
		} catch (NameNotFoundException e) {

		}
		
        int flags = packageInfo.applicationInfo.flags;
        mDebug = (flags & ApplicationInfo.FLAG_DEBUGGABLE) != 0;
        */
        SystemProperties.set("tcc.dhcpclient.ipaddress","");
        SystemProperties.set("tcc.wifi.wfd.sinkapp","1");
       	startP2p("Starting","waitting for initialization",false);

       	// check wifi connected
		//checkWifiConnected();
       	/*
       	if(SystemProperties.get("ro.build.type").equals("eng")) {
       		SystemProperties.set(WifiDisplayAdvancedSettings.PROPERTY_DEBUG_MODE,"1");
       	}
		*/

        /* don't use from kitkat
        if(!SystemProperties.get("tcc.solution.mbox").equals("1"))
	       	checkWifiConnected();
       	*/
        //mWifiManager.tempCancelScan();
    }

    private void checkWifiConnected() {
    	ConnectivityManager cManager; 
    	NetworkInfo wifi; 
    	 
    	cManager=(ConnectivityManager)getSystemService(Context.CONNECTIVITY_SERVICE); 
    	wifi = cManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI); 
    	 
    	if(wifi.isConnected()) {
			alertDlg = new AlertDialog.Builder(this);
			alertDlg.setTitle("Info");
			alertDlg.setMessage("This device is already connected to AP.\nBecause AP connection makes poor Wi-Fi Display connection.\nWe recommand disconnect AP for Wi-Fi Display.\nAre you agree?");
			alertDlg.setPositiveButton("Yes",
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog,
								int whichButton) {
							mWifiP2pManager.requestTempDisconnectWifi(mChannel);
						}
					});
			alertDlg.setNegativeButton("No",
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog,
								int whichButton) {
							// nothing to do
						}
					});
			alertDlg.show();
			//handler.sendEmptyMessageDelayed(MESSAGE_POPUP_TIMEOUT,POPUP_TIMEOUT);
			return;	
    	}
    }
    
	public static final TelechipsWFDSink getInstance() {
		if (instance != null)
			return instance;
		return null;
	}
	
	private void updateWfdEnableState() {
		Log.d(TAG, "updateWfdEnableState");
		WifiP2pWfdInfo wfdInfo = new WifiP2pWfdInfo();
		wfdInfo.setWfdEnabled(true);
		wfdInfo.setDeviceType(WifiP2pWfdInfo.PRIMARY_SINK);
		wfdInfo.setSessionAvailable(true);
		wfdInfo.setControlPort(DEFAULT_CONTROL_PORT);
		wfdInfo.setMaxThroughput(MAX_THROUGHPUT);
		if(SystemProperties.get("tcc.hdcp2.enable").equals("1") || SystemProperties.get("tcc.hdcp2.enable").equals("2")){
			Log.d(TAG,"HDCP2 ENABLE!!!");
			wfdInfo.setContentProtection(true);
		}else
			wfdInfo.setContentProtection(false);
        
		if(TelechipsWFDSink.getInstance().mWifiP2pManager==null)
			TelechipsWFDSink.getInstance().mWifiP2pManager = (WifiP2pManager)getSystemService(Context.WIFI_P2P_SERVICE);

		if (TelechipsWFDSink.getInstance().mWifiP2pManager != null) { 
			if(TelechipsWFDSink.getInstance().mChannel ==null)
				TelechipsWFDSink.getInstance().mChannel = TelechipsWFDSink.getInstance().mWifiP2pManager.initialize(this,	getMainLooper(), null);
		}else {
			Log.e(TAG, "mWifiP2pManager is null");
			return;
		}

		TelechipsWFDSink.getInstance().mWifiP2pManager.setWFDInfo(TelechipsWFDSink.getInstance().mChannel, wfdInfo, new ActionListener() {
			@Override
			public void onSuccess() {
				//Log.d(TAG, "updateWfdEnableState");
			}

			@Override
			public void onFailure(int reason) {
				Log.d(TAG, "Failed to set WFD info with reason " + reason + ".");
				// FIXME:
				//startP2p("Please wait","System needs to reset", true);
			}
		});
	}

    
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION.equals(action)) {
                mThisDevice = (WifiP2pDevice) intent.getParcelableExtra(
                        WifiP2pManager.EXTRA_WIFI_P2P_DEVICE);
                //Log.d(TAG, "Update device info: " + mThisDevice);
                updateMyDevicePref();
        		//updateWfdEnableState();
            }else if (WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION.equals(action)) {
                mWifiP2pEnabled = intent.getIntExtra(WifiP2pManager.EXTRA_WIFI_STATE,
                        WifiP2pManager.WIFI_P2P_STATE_DISABLED) == WifiP2pManager.WIFI_P2P_STATE_ENABLED;
                //updateWfdEnableState();
            }
        }
    };
    
    private void updateMyDevicePref() {
        if (mThisDevice != null) {
        	myDevice.setTextColor(0xFF000000);

            if (TextUtils.isEmpty(mThisDevice.deviceName)) {
            	myDevice.setText("Please connect to"+mThisDevice.deviceAddress);
            } else {
            	myDevice.setText("Please connect to "+mThisDevice.deviceName);
            }

            translateImageView = (ImageView)findViewById(R.id.imageView2);
			ani = AnimationUtils.loadAnimation(this, R.anim.animation);
			translateImageView.startAnimation(ani);
        }
    }
    
    public void onResume()
    {
    	super.onResume();
    	Log.d(TAG,"onResume");
    	
    	if(WFDPlay.getWFDPlayStatus() == WFDPlay.WFDPLAY_STATUS_SHUTDOWN) {
			Log.e(TAG,"shutdown wfd sink");
			onBackPressed();
			WFDPlay.setWFDPlayStatus(WFDPlay.WFDPLAY_STATUS_NONE);
			return;
		}

       	if(SystemProperties.get("ro.build.type").equals("eng")) {
			mTVVersion.setText(TelechipsWFDSink.getVersionName(instance)+"  ");
		}else {
			mTVVersion.setText("");
		}
    	    	
		if(mWifiP2pManager!=null) {
	        mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION);
	        mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION);
	        mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION);
	        mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION);
	        
	        registerReceiver(mReceiver, mIntentFilter);
	        
	        //startP2PDispeerTimeoutThread(P2P_DISCOVER_TIMEOUT);
            
			dialogListener = new TelechipsWFDSink();

			mWifiP2pManager.setDialogListener(mChannel, dialogListener);
			
			if(WFDPlay.getWFDPlayStatus() == WFDPlay.WFDPLAY_STATUS_PLAYERROR) {
				Log.e(TAG,"reStartP2p");
				startP2p("Play error","Waitting for re-initialization", false);
			}else if(WFDPlay.getWFDPlayStatus() == WFDPlay.WFDPLAY_STATUS_PLAYEND) {
				Log.d(TAG,"onResume scanWifiDisplays");
			}else if(WFDPlay.getWFDPlayStatus() == WFDPlay.WFDPLAY_STATUS_PLAYBACKKEY) {
				Log.e(TAG,"reStartP2p");
				startP2p("Re-initialize","Waitting for re-initialization",false);
			}

            Log.d(TAG,"startWifiDisplayScan");
            WifiDisplaySettings.getInstance().mDisplayManager.startWifiDisplayScan();

		}
    }
    
    public void onPause()
    {
        super.onPause();
        Log.d(TAG,"onPause");
		try{
			unregisterReceiver(mReceiver);
		}catch(IllegalArgumentException ex){
			
		}

        if(TelechipsWFDSink.getInstance().mWifiP2pManager != null)
        	TelechipsWFDSink.getInstance().mWifiP2pManager.stopPeerDiscovery(TelechipsWFDSink.getInstance().mChannel, null);
		
		//stopP2PDispeerTimeoutThread();
        Log.d(TAG,"stopWifiDisplayScan");

		WifiDisplaySettings.getInstance().mDisplayManager.stopWifiDisplayScan();
    }
    
    public void onDestroy()
    {
    	super.onDestroy();
    	Log.e(TAG, "onDestroy");
    	if (mWifiP2pManager != null) {
    		mWifiP2pManager.requestTempReconnectWifi(mChannel);
            mWifiP2pManager.stopPeerDiscovery(mChannel, null);

            mWifiP2pManager.removeGroup(mChannel, new WifiP2pManager.ActionListener() {
                public void onSuccess() {
                }
                public void onFailure(int reason) {
                }
            });

    	}
    	
    	if(TelechipsWFDSink.getInstance().pd!=null){
			if(TelechipsWFDSink.getInstance().pd.isShowing()==true)
				TelechipsWFDSink.getInstance().pd.dismiss();
		}
    }
    
    public void onBackPressed() {
    	Log.e(TAG, "onBackPressed");
    	
		if(SystemProperties.get("tcc.solution.mbox").equals("1"))
			mWifiManager.setWifiEnabled(true);
		clearProperty();
        finish();

    }
    public void clearProperty() {
		
		SystemProperties.set("tcc.wifi.display.sink.connect","0");
		SystemProperties.set("tcc.wifi.wfd.sinkapp","0");    	
    }
    
    @Override
    public void onConfigurationChanged(Configuration config) {
	     super.onConfigurationChanged(config);

	     switch(config.orientation) {
	        case Configuration.ORIENTATION_PORTRAIT:   
	   	     Log.e(TAG, "ORIENTATION_PORTRAIT");
	   	     //setContentView(R.layout.main_port);
	   	     //lp = new LinearLayout.LayoutParams(200, LayoutParams.WRAP_CONTENT);
	   	     //layout.setLayoutParams(lp);
	   	     
	   	     //lp.setMargins(0, 50, 0, 0);
	   	     //if(layout==null)
	   	     //	Log.e(TAG, "layout is null");
	   	     //else 
	   	     //	 layout.setLayoutParams(lp);
	         break;
	     case Configuration.ORIENTATION_LANDSCAPE:  
		   	 Log.e(TAG, "ORIENTATION_LANDSCAPE");
	         break;
	    }


    }
    
	public void showToast(int id, int showTime) {
		Toast.makeText(TelechipsWFDSink.getInstance(), mRes.getString(id), showTime).show();
	}

	public void showToast(String msg, int showTime) {
		Toast.makeText(TelechipsWFDSink.getInstance(), msg, showTime).show();
	}

	@Override
	public void onPeersAvailable(WifiP2pDeviceList arg0) {
        //Log.d(TAG, "onPeersAvailable");
	}

	@Override
	public void onGroupInfoAvailable(WifiP2pGroup arg0) {
        Log.d(TAG, "onGroupInfoAvailable");
	}
	
	public void startPlay(String sourceIP, int rtspPort) {
		Log.d(TAG, "startPlay sourceIP=" + sourceIP + " rtspPort = " + rtspPort);

    	if(rtspPort>0) saveRtspPort(rtspPort);
        
		if( sourceIP == null) {
			Log.e(TAG,"startPlay error");
			TelechipsWFDSink.getInstance().pd.dismiss();
			TelechipsWFDSink.getInstance().notifyDialog = new Dialog(TelechipsWFDSink.getInstance());
			TelechipsWFDSink.getInstance().notifyDialog.setContentView(R.layout.customdialog);
            if( TelechipsWFDSink.getInstance().mConnectP2PInfo.isGroupOwner ) 
			    TelechipsWFDSink.getInstance().notifyDialog.setTitle("IP assign fail");
            else
			    TelechipsWFDSink.getInstance().notifyDialog.setTitle("IP get fail");

			TextView text = (TextView) TelechipsWFDSink.getInstance().notifyDialog.findViewById(R.id.text);
			text.setText("Please try again.");
			TelechipsWFDSink.getInstance().notifyDialog.show();
			handler.sendEmptyMessageDelayed(MESSAGE_POPUP_TIMEOUT,POPUP_TIMEOUT);
            
			return;
		}else if(rtspPort<=0) {
			rtspPort = getRtspPort();
			Log.d(TAG,"getRtspPort: rtspPort = " + rtspPort);
		}
		try{
			unregisterReceiver(mReceiver);
		}catch(IllegalArgumentException ex){
			
		}

        TelechipsWFDSink.getInstance().connectionStatus = mRes.getText(R.string.connection_rtsp_wait_dialog_message).toString();
        setConnectionStatus(CONNECTION_STATUS_RTSP_NEGO);
        //setConnectionStatus(CONNECTION_STATUS_COMPLETE);

        SystemProperties.set("tcc.wifi.display.sink.connect","1");
        
		Intent intent = new Intent(TelechipsWFDSink.this, WFDPlay.class);
		intent.putExtra("IP", sourceIP);
        intent.putExtra("PORT", rtspPort);
        startActivityForResult(intent, 0);
	}
	
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		Log.d(TAG,"onActivityResult");
		if (requestCode == 0) {
	        if (resultCode == RESULT_OK) {
	        	Log.d(TAG,"onActivityResult ok");
	        	String result = data.getStringExtra("WFDPlay_result");
	        	if(result.contains("ok")) {
	        		Log.d(TAG,"onActivityResult ok");
	        	}else if(result.contains("onBackPressed")) {
	        		Log.d(TAG,"WFDPlay onBackPressed");
	        	}else{
	        		Log.d(TAG,"WFDPlay fail");
	        		// re-start p2p
	        		WFDPlay.setWFDPlayStatus(WFDPlay.WFDPLAY_STATUS_NONE);
	        		//reStartP2p();
	        		
	        	}
	        }
	    }
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
				
				if(mTimeout==0 && type == P2P_DISCOVER_TIMEOUT_THREAD_TYPE) {
					mTimeout = P2P_DISCOVER_TIMEOUT;

					Log.d(TAG,"startWifiDisplayScan");
					WifiDisplaySettings.getInstance().mDisplayManager.startWifiDisplayScan();
				}
				//Log.d(TAG,"TimeoutThread run() type = " + type + "mTimeout = " + mTimeout);
			}
		
			Log.d(TAG,"TimeoutThread out run() type = " + type);
			
		}
	}
	
	public Handler handler = new Handler() {
		public void handleMessage(Message msg) {
			switch (msg.what) {
				case MESSAGE_APP_EXIT:
					showToast(msg.getData().getString("MESSAGE"), Toast.LENGTH_SHORT);
                    Log.e(TAG, "stopWifiDisplayScan");
					WifiDisplaySettings.getInstance().mDisplayManager.stopWifiDisplayScan();
					break;
				case MESSAGE_CONNECTION_DIALOG_DISMISS:
					Log.d(TAG,"Connection progress dialog dismiss");
					if(TelechipsWFDSink.getInstance().pd==null)  
						Log.e(TAG,"pd is null");
					else
						if(TelechipsWFDSink.getInstance().pd.isShowing()==true) {
							TelechipsWFDSink.getInstance().pd.dismiss();
                            TelechipsWFDSink.getInstance().pd = null;
                            Log.e(TAG,"startWifiDisplayScan");
                            WifiDisplaySettings.getInstance().mDisplayManager.startWifiDisplayScan();
                        }
					break;
				case MESSAGE_TIMEOUT:
					Log.d(TAG,"TIMEOUT");
					if(TelechipsWFDSink.getInstance().pd!=null && TelechipsWFDSink.getInstance().pd.isShowing()==true)
						TelechipsWFDSink.getInstance().pd.dismiss();

                    if(TelechipsWFDSink.getInstance().showPinDialog!=null && TelechipsWFDSink.getInstance().showPinDialog.isShowing()==true)
						TelechipsWFDSink.getInstance().showPinDialog.dismiss();
                    
					showToast("Connection timeout, try again", Toast.LENGTH_SHORT);
					break;
				case MESSAGE_CONNECTION_DIALOG_MSG_UPDATE:
                    //Log.d(TAG,"MESSAGE_CONNECTION_DIALOG_MSG_UPDATE : " + TelechipsWFDSink.getInstance().connectionStatus);
                    if( (TelechipsWFDSink.getInstance().pd != null) && (TelechipsWFDSink.getInstance().pd.isShowing()==true) )
    					TelechipsWFDSink.getInstance().pd.setMessage(TelechipsWFDSink.getInstance().connectionStatus + " ... " + msg.getData().getString("MESSAGE"));
					break;
				case MESSAGE_RESET_P2P:
                    Log.e(TAG,"MESSAGE_RESET_P2P");
					TelechipsWFDSink.getInstance().startP2p("Reset","waiting for initialization",true);

                    // new scan after wifi reset
                    Log.e(TAG,"startWifiDisplayScan");
                    WifiDisplaySettings.getInstance().mDisplayManager.startWifiDisplayScan();

					break;
				case MESSAGE_WFDPLAY_CONNECTION_RETRY:
					Log.d(TAG,"MESSAGE_WFDPLAY_CONNECTION_RETRY");
					if(TelechipsWFDSink.getInstance().notifyDialog!=null)
						TelechipsWFDSink.getInstance().notifyDialog.dismiss();
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
		data.putString("MESSAGE", Integer.toString(message));
		msg.what = what;
		msg.setData(data);
		handler.sendMessage(msg);
	}
			
	@Override
	public void onConnectionInfoAvailable(WifiP2pInfo info) {

        Log.d(TAG, "onConnectionInfoAvailable");

        setConnectionStatus(CONNECTION_STATUS_IP_PROCESS);
        
		if(WifiDisplaySettings.getInstance().bCreateGroup==true)
			return;

        TelechipsWFDSink.getInstance().mConnectP2PInfo = info;
        
        if( TelechipsWFDSink.getInstance().mConnectP2PInfo.isGroupOwner ) 
            TelechipsWFDSink.getInstance().connectionStatus = mRes.getText(R.string.connection_ip_assign_wait_dialog_message).toString();
	    else
            TelechipsWFDSink.getInstance().connectionStatus = mRes.getText(R.string.connection_ip_get_wait_dialog_message).toString();
        
		if(WFDPlay.getWFDPlayStatus() != WFDPlay.WFDPLAY_STATUS_PLAYERROR) {
    		Thread thread = new Thread(new Runnable() {
    			int i;
                String sourceIP = null;
                int rtspPort = TelechipsWFDSink.getInstance().mConnectP2PInfo.mWfdRtspPort;
                
    			public void run() {
       			if(rtspPort<=0)
	    			rtspPort = WifiDisplaySettings.getInstance().getConnectedDeviceRtspPort();

    			if (TelechipsWFDSink.getInstance().mConnectP2PInfo.isGroupOwner != true) {
    				for(i=0; i<CONNECTION_TIMEOUT_RTSP_NEGO; i++) {
    					if( TelechipsWFDSink.getInstance().mConnectP2PInfo.groupOwnerAddress==null ) {
                            if(TelechipsWFDSink.getInstance().pd!=null && TelechipsWFDSink.getInstance().pd.isShowing()==false)
                                return;
    						Log.d(TAG, "groupOwnerAddress==null");
    						sleep(1);
    					}else{
    						Log.d(TAG,"Source device is "+ TelechipsWFDSink.getInstance().mConnectP2PInfo.groupOwnerAddress.getHostAddress());
    						sourceIP = TelechipsWFDSink.getInstance().mConnectP2PInfo.groupOwnerAddress.getHostAddress();
                            startPlay(sourceIP, rtspPort);
    						break;
    					}
    				}
    			} else {
    				// wait ip message from source
    				Log.d(TAG, "I'm grouop owner, let find source ip");
    				String ipRegex = "(\\d{1,3}\\.){3}\\d{1,3}";
    				Pattern patttern = Pattern.compile(ipRegex);
    				Matcher match;
    				
    				for(i=0; i<CONNECTION_TIMEOUT_RTSP_NEGO; i++) {
    					String str = SystemProperties.get("tcc.dhcpclient.ipaddress");
    					match = patttern.matcher(str);
    					if (match.find()) {
    						sourceIP = match.group();
    						Log.d(TAG, "Source device is " + sourceIP);
                            startPlay(sourceIP, rtspPort);
    						break;
    					} 
    					
    					Log.d(TAG, "Wait until find source ip");
    					if( TelechipsWFDSink.getInstance().pd!=null && TelechipsWFDSink.getInstance().pd.isShowing()==false)
                            return;

    					sleep(1);
    				}
    			}

    			}
    		});
    		thread.start();
		}else {
            Log.e(TAG,"Invalid status : " + WFDPlay.getWFDPlayStatus());
        }
        
	}
	
	void saveRtspPort(int port) {
		Log.d(TAG, "saveRtspPort : " + port);
		File f = new File(getInstance().getFilesDir() + "/rtspPort"); 
		if(f.exists())
			f.delete();
		try {
			 f.createNewFile();
			 FileWriter fw = new FileWriter(f);
			 Log.d(TAG,String.valueOf(port));
			 fw.write(String.valueOf(port));
			 fw.close();
		} catch (IOException e1) {
			 // TODO Auto-generated catch block
			 e1.printStackTrace();
		}
	}
	
	int getRtspPort() {
		File f = new File(getInstance().getFilesDir() + "/rtspPort"); 
		String port = null;
		try {
			 FileReader fr = new FileReader(f);
			 BufferedReader br = new BufferedReader(fr);
			 port = br.readLine();
			 br.close();
			 fr.close();
		} catch (IOException e1) {
			 // TODO Auto-generated catch block
			 e1.printStackTrace();
		}
		if(port!=null)
			return Integer.parseInt(port);
		else
			return 7236;	// default 
	}
	
	void startP2p(String title, String message, boolean reset) {
		Log.d(TAG, "startP2p reset = " + reset);
        int i=0;

		if(TelechipsWFDSink.getInstance().pd!=null){
			if(TelechipsWFDSink.getInstance().pd.isShowing()==true)
				TelechipsWFDSink.getInstance().pd.dismiss();
		}
		
		if(reset==true) {
            Log.d(TAG, "wifi off");
			TelechipsWFDSink.getInstance().mWifiManager.setWifiEnabled(false);
            
            for(i=0; i<10; i++) {
                if(TelechipsWFDSink.getInstance().mWifiManager.isWifiEnabled()==false)
                    break;
                Log.d(TAG, "wifi off wait");
                sleep(1);
            }
        }

		TelechipsWFDSink.getInstance().pd = null;
		TelechipsWFDSink.getInstance().pd = ProgressDialog.show(TelechipsWFDSink.getInstance(), title, message, true, false);
		Thread thread = new Thread(new Runnable() {
			public void run() {
                int i=0;
				if(TelechipsWFDSink.getInstance().mWifiManager.isWifiEnabled()==false) {
					TelechipsWFDSink.getInstance().mWifiManager.setWifiEnabled(true);
                    for(i=0; i<10; i++) {
                        if(TelechipsWFDSink.getInstance().mWifiManager.isWifiEnabled()==true)
                            break;
                        Log.d(TAG, "wifi on wait");
                        sleep(1);
                    }
                }

				if (TelechipsWFDSink.getInstance().pd != null) {
					if (TelechipsWFDSink.getInstance().pd.isShowing() == true)
						TelechipsWFDSink.getInstance().pd.dismiss();
				}
				
				WFDPlay.setWFDPlayStatus(WFDPlay.WFDPLAY_STATUS_NONE);
				updateWfdEnableState();
			}
		});
		thread.start();
	}
	
	void resetP2p(String title, String message, boolean reset) {
		Log.d(TAG, "resetP2p");
		if(TelechipsWFDSink.getInstance().pd!=null){
			if(TelechipsWFDSink.getInstance().pd.isShowing()==true)
				TelechipsWFDSink.getInstance().pd.dismiss();
		}
		
		if(reset==true) {
			TelechipsWFDSink.getInstance().mWifiManager.setWifiEnabled(false);
		}
		
		TelechipsWFDSink.getInstance().pd = null;
		TelechipsWFDSink.getInstance().pd = ProgressDialog.show(TelechipsWFDSink.getInstance(), title, message, true, false);
		Thread thread = new Thread(new Runnable() {
			public void run() {
				
				if(TelechipsWFDSink.getInstance().mWifiManager.isWifiEnabled()==false)
					TelechipsWFDSink.getInstance().mWifiManager.setWifiEnabled(true);
				
				if (TelechipsWFDSink.getInstance().pd != null) {
					if (TelechipsWFDSink.getInstance().pd.isShowing() == true)
						TelechipsWFDSink.getInstance().pd.dismiss();
				}
				
				WFDPlay.setWFDPlayStatus(WFDPlay.WFDPLAY_STATUS_NONE);
				updateWfdEnableState();
                Log.d(TAG,"startWifiDisplayScan");
				WifiDisplaySettings.getInstance().mDisplayManager.startWifiDisplayScan();
			}
		});
		thread.start();
	}
	
	static void sleep(int secs){
		try {
			Thread.sleep( secs * 1000);
		}catch(InterruptedException e) {
			e.printStackTrace();
		}
	}

    void connectCancel() {
        Log.e(TAG,"connectCancel");
        TelechipsWFDSink.getInstance().mWifiP2pManager.cancelConnect(TelechipsWFDSink.getInstance().mChannel, new ActionListener() {
            @Override
            public void onSuccess() {
                Log.d(TAG, "Canceled connection to Wifi display");
                TelechipsWFDSink.getInstance().pd.dismiss();
            }

            @Override
            public void onFailure(int reason) {
                Log.d(TAG, "Failed to cancel connection to Wifi display");
                        
            }
        });

         TelechipsWFDSink.getInstance().mWifiP2pManager.removeGroup( TelechipsWFDSink.getInstance().mChannel, new WifiP2pManager.ActionListener() {
            public void onSuccess() {
            }
            public void onFailure(int reason) {
            }
        });

        TelechipsWFDSink.getInstance().mWifiP2pManager.requestP2pFlush( TelechipsWFDSink.getInstance().mChannel);

    }

    void setConnectionStatus(int status){
        Log.d(TAG,"setConnectionStatus : " + status);
        TelechipsWFDSink.getInstance().mConnectionStatus = status;
        if(status == CONNECTION_STATUS_P2P_GROUP_START) {
            Log.e(TAG, "stopWifiDisplayScan");
            WifiDisplaySettings.getInstance().mDisplayManager.stopWifiDisplayScan();
        }
    }

    int getConnectionStatus() {
        return TelechipsWFDSink.getInstance().mConnectionStatus;
    }

    int getConnectionTimeout(int status) {
        int timeout=0;
        switch(status) {
            case CONNECTION_STATUS_INIT:
                timeout = CONNECTION_TIMEOUT_P2P_GO_NEGO_SUCCESS;
                break;
            case CONNECTION_STATUS_P2P_GO_NEGO_SUCCESS:
                timeout = CONNECTION_TIMEOUT_P2P_GROUP_FORMATION_SUCCESS;
                break;
            case CONNECTION_STATUS_P2P_GROUP_FORMATION_SUCCESS:
                timeout = CONNECTION_TIMEOUT_IP_PROCESS;
                break;
            case CONNECTION_STATUS_PERSISTENT:
                timeout = CONNECTION_TIMEOUTS_PERSISTENT;
                break;
            case CONNECTION_STATUS_IP_PROCESS:
                timeout = CONNECTION_TIMEOUT_RTSP_NEGO;
                break;
            case CONNECTION_STATUS_RTSP_NEGO:
                timeout = CONNECTION_TIMEOUT_COMPLETE;
                break;
            case CONNECTION_STATUS_P2P_GROUP_START:
                timeout = CONNECTION_TIMEOUT_COMPLETE;
                break;
            default:
                Log.e(TAG,"Unknown status = " + status);
                timeout = 10; //
                break;
        }
        return timeout;
    }
    
	void createConnectionProgressDialog(boolean persistent) {
		if (TelechipsWFDSink.getInstance().pd != null && TelechipsWFDSink.getInstance().pd.isShowing() == true)	// block twice pop-up, 20130418
			return;
	    Log.d(TAG,"createConnectionProgressDialog");	

		TelechipsWFDSink.getInstance().connectionStatus = TelechipsWFDSink.getInstance().mRes.getText(R.string.connection_p2p_wait_dialog_message).toString();
        TelechipsWFDSink.getInstance().pd = new ProgressDialog(TelechipsWFDSink.getInstance());
        TelechipsWFDSink.getInstance().pd.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
            
		if(persistent==true) {
            setConnectionStatus(CONNECTION_STATUS_PERSISTENT);
            TelechipsWFDSink.getInstance().pd.incrementProgressBy(CONNECTION_PROGRESS_PERSISTENT-TelechipsWFDSink.getInstance().pd.getProgress());
            TelechipsWFDSink.getInstance().pd.setTitle(TelechipsWFDSink.getInstance().mRes.getText(R.string.connection_persistent_dialog_title));
		}else{
            TelechipsWFDSink.getInstance().pd.setTitle(TelechipsWFDSink.getInstance().mRes.getText(R.string.connection_dialog_title));

		}
        setConnectionStatus(CONNECTION_STATUS_INIT);
        TelechipsWFDSink.getInstance().pd.setMessage(TelechipsWFDSink.getInstance().connectionStatus);
        TelechipsWFDSink.getInstance().pd.setButton("cancel", new DialogInterface.OnClickListener() 
        {
            public void onClick(DialogInterface dialog, int which) 
            {
                Log.d(TAG, "Connect cancel");
                connectCancel();
                Log.d(TAG,"startWifiDisplayScan");
                WifiDisplaySettings.getInstance().mDisplayManager.startWifiDisplayScan();
                return;
            }
        });
        TelechipsWFDSink.getInstance().pd.show();

		Thread thread = new Thread(new Runnable() {
			int i;
            int saveStatus = CONNECTION_STATUS_INIT;
			public void run() {
				for (i = 0; i < CONNECTION_TIMEOUT; i++) 
                {
					sleep(1);
					if (TelechipsWFDSink.getInstance().pd == null || TelechipsWFDSink.getInstance().pd.isShowing() == false) {
						break;
                    }

                    // update progress
                    if(saveStatus != getConnectionStatus()) {
                        if(getConnectionStatus() == CONNECTION_STATUS_P2P_GO_NEGO_SUCCESS)
                            TelechipsWFDSink.getInstance().pd.incrementProgressBy(CONNECTION_PROGRESS_P2P_GO_NEG_SUCCESS-TelechipsWFDSink.getInstance().pd.getProgress());
                        else if(getConnectionStatus() == CONNECTION_STATUS_P2P_GROUP_FORMATION_SUCCESS)
                            TelechipsWFDSink.getInstance().pd.incrementProgressBy(CONNECTION_PROGRESS_P2P_GROUP_FORMATION_SUCCESS-TelechipsWFDSink.getInstance().pd.getProgress());
                        else if(getConnectionStatus() == CONNECTION_STATUS_IP_PROCESS)
                            TelechipsWFDSink.getInstance().pd.incrementProgressBy(CONNECTION_PROGRESS_IP_PROCESS-TelechipsWFDSink.getInstance().pd.getProgress());
                        else if(getConnectionStatus() == CONNECTION_STATUS_RTSP_NEGO)
                            return;
                            //TelechipsWFDSink.getInstance().pd.incrementProgressBy(CONNECTION_PROGRESS_RTSP_NEGO-TelechipsWFDSink.getInstance().pd.getProgress());
                        else if(getConnectionStatus() == CONNECTION_STATUS_P2P_GROUP_START)
                            TelechipsWFDSink.getInstance().pd.incrementProgressBy(CONNECTION_PROGRESS_GROUP_START-TelechipsWFDSink.getInstance().pd.getProgress());
                        else if(getConnectionStatus() == CONNECTION_STATUS_COMPLETE)
                            return;
                        i=0;
                    }
                    
                    saveStatus = getConnectionStatus();
                    
					sendMessage(MESSAGE_CONNECTION_DIALOG_MSG_UPDATE, i);
                    
                    //Log.d(TAG,"i = " + i + " connection progress = " + TelechipsWFDSink.getInstance().pd.getProgress() + " status = " + saveStatus);

                    TelechipsWFDSink.getInstance().pd.incrementProgressBy(CONNECTION_PROGRESS_INCREMENT);                     
                    
                    // check timeout each status
                    if(i > getConnectionTimeout(getConnectionStatus())) {
                        Log.d(TAG,"status = " + getConnectionStatus() + " timeout");
                        break;
                    }
				}

                // timeout
				if (TelechipsWFDSink.getInstance().pd != null && TelechipsWFDSink.getInstance().pd.isShowing() == true) {
					Log.e(TAG, "Connection timeout");
					TelechipsWFDSink.getInstance().connectCancel();
					
					sendMessage(MESSAGE_TIMEOUT);
                    Log.d(TAG,"startWifiDisplayScan");
					WifiDisplaySettings.getInstance().mDisplayManager.startWifiDisplayScan();
				}
			}
		});
		thread.start();
	}

	/* DialogListener */
	@Override
	public void onAttached() {
		Log.d(TAG, "onAttached");
	}

	@Override
	public void onConnectionRequested(WifiP2pDevice arg0, WifiP2pConfig arg1) {
		Log.d(TAG, "------------------------------------- onConnectionRequested");
		boolean persistent=false;
		
		if(TelechipsWFDSink.getInstance().showPinDialog!=null) {
			TelechipsWFDSink.getInstance().showPinDialog.dismiss();
		}

		if(WifiDisplaySettings.getInstance().bCreateGroup==true)
			WifiDisplaySettings.getInstance().bCreateGroup=false;
		
        if(SystemProperties.get("tcc.wifi.p2p.persistent").equals("1")) {
			if((arg0==null) && (arg1==null)) {
				persistent = true;
				Log.d(TAG, "persistent");
			}else{
				persistent = false;
			}
        }
        				
		//Log.e(TAG, "stopWifiDisplayScan");
        //WifiDisplaySettings.getInstance().mDisplayManager.stopWifiDisplayScan();
        
		createConnectionProgressDialog(persistent);
	}

	@Override
	public void onMessageRequested(WifiP2pDevice arg0, WifiP2pConfig arg1) {
		Log.d(TAG, "onMessageRequested msg = " + arg1.message);
		boolean persistent=false;
		if(TelechipsWFDSink.getInstance().showPinDialog!=null) {
			TelechipsWFDSink.getInstance().showPinDialog.dismiss();
		}
		
		if(WifiDisplaySettings.getInstance().bCreateGroup==true)
			WifiDisplaySettings.getInstance().bCreateGroup=false;
		
		/* P2P Group Formation Timeout */
		if( TelechipsWFDSink.getInstance().pd != null && TelechipsWFDSink.getInstance().pd.isShowing() == true) {
			if( arg1.message == WifiP2pManager.P2P_GROUP_FORMATION_FAIL) {
			
				TelechipsWFDSink.getInstance().pd.dismiss();
				TelechipsWFDSink.getInstance().notifyDialog = new Dialog(TelechipsWFDSink.getInstance());
				TelechipsWFDSink.getInstance().notifyDialog.setContentView(R.layout.customdialog);
				TelechipsWFDSink.getInstance().notifyDialog.setTitle("P2P Group Formation Timeout");
				TextView text = (TextView) TelechipsWFDSink.getInstance().notifyDialog.findViewById(R.id.text);
				text.setText("Please try again.");
				TelechipsWFDSink.getInstance().notifyDialog.show();
				handler.sendEmptyMessageDelayed(MESSAGE_POPUP_TIMEOUT,POPUP_TIMEOUT);
				
				return;
			}else if(arg1.message == WifiP2pManager.P2P_GO_NEG_FAIL) {
				TelechipsWFDSink.getInstance().pd.dismiss();
				TelechipsWFDSink.getInstance().notifyDialog = new Dialog(TelechipsWFDSink.getInstance());
				TelechipsWFDSink.getInstance().notifyDialog.setContentView(R.layout.customdialog);
				TelechipsWFDSink.getInstance().notifyDialog.setTitle("P2P Go Nego fail");
				TextView text = (TextView) TelechipsWFDSink.getInstance().notifyDialog.findViewById(R.id.text);
				text.setText("Please try again.");
				TelechipsWFDSink.getInstance().notifyDialog.show();
				handler.sendEmptyMessageDelayed(MESSAGE_POPUP_TIMEOUT,POPUP_TIMEOUT);
			}else if(arg1.message == WifiP2pManager.P2P_GO_NEG_SUCCESS) {
			    TelechipsWFDSink.getInstance().connectionStatus = TelechipsWFDSink.getInstance().mRes.getText(R.string.connection_p2p_go_neg_success_dialog_message).toString();
                setConnectionStatus(CONNECTION_STATUS_P2P_GO_NEGO_SUCCESS);
                return;
            }else if(arg1.message == WifiP2pManager.P2P_GROUP_FORMATION_SUCCESS) {
                TelechipsWFDSink.getInstance().connectionStatus = TelechipsWFDSink.getInstance().mRes.getText(R.string.connection_p2p_gr_for_success_dialog_message).toString();
                setConnectionStatus(CONNECTION_STATUS_P2P_GROUP_FORMATION_SUCCESS);
                return;
            }else if(arg1.message == WifiP2pManager.P2P_GROUP_STARTED) {
                TelechipsWFDSink.getInstance().connectionStatus = TelechipsWFDSink.getInstance().mRes.getText(R.string.connection_p2p_gr_start_dialog_message).toString();
                setConnectionStatus(CONNECTION_STATUS_P2P_GROUP_START);
                return;
            }

		}
		
		createConnectionProgressDialog(persistent);
	}
	
	@Override
	public void onDetached(int arg0) {
		Log.d(TAG, "onDetached");
	}

	@Override
	public void onShowPinRequested(String pin) {
		Log.d(TAG, "onShowPinRequested pin="+pin);

		//Log.d(TAG, "stopWifiDisplayScan");
        //WifiDisplaySettings.getInstance().mDisplayManager.stopWifiDisplayScan();

		if(TelechipsWFDSink.getInstance().showPinDialog!=null && TelechipsWFDSink.getInstance().showPinDialog.isShowing()==true) {
			return;
		} else {
			TelechipsWFDSink.getInstance().showPinDialog = new Dialog(TelechipsWFDSink.getInstance());
			TelechipsWFDSink.getInstance().showPinDialog.setContentView(R.layout.customdialog);
			TelechipsWFDSink.getInstance().showPinDialog.setTitle("Show PIN - please input pin to source device");
			TextView text = (TextView) TelechipsWFDSink.getInstance().showPinDialog.findViewById(R.id.text);
			text.setText("PIN : " + pin);
	
			TelechipsWFDSink.getInstance().showPinDialog.show();
		}

        Thread thread = new Thread(new Runnable() {
			int i;
			public void run() {
				for (i = 0; i < CONNECTION_TIMEOUT; i++) 
                {
					sleep(1);
                    if(TelechipsWFDSink.getInstance().showPinDialog != null && TelechipsWFDSink.getInstance().showPinDialog.isShowing() == false)
                        break;
                }

                // timeout
				if (TelechipsWFDSink.getInstance().showPinDialog != null && TelechipsWFDSink.getInstance().showPinDialog.isShowing() == true) {
					Log.e(TAG, "pin show timeout");
					TelechipsWFDSink.getInstance().connectCancel();
					
					sendMessage(MESSAGE_TIMEOUT);
                    Log.d(TAG,"startWifiDisplayScan");
					WifiDisplaySettings.getInstance().mDisplayManager.startWifiDisplayScan();
				}
			}
		});
		thread.start();

	}
	
	 public static String getVersionName(Context context)
	 {
	     try {
	         PackageInfo pi= context.getPackageManager().getPackageInfo(context.getPackageName(), 0);
	         return "Version: "+pi.versionName;//+\nBuild time: +mTime;
	     } catch (NameNotFoundException e) {
	         return null;
	     }
	 }

}
