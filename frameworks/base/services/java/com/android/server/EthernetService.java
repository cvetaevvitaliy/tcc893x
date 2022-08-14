/*
 * Copyright (C) 2009 The Android-x86 Open Source Project
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
 *
 * Author: Yi Sun <beyounn@gmail.com>
 */

package com.android.server;

import java.net.UnknownHostException;

import android.net.ethernet.EthernetNative;
import android.net.ethernet.IEthernetManager;
import android.net.ethernet.EthernetManager;
import android.net.ethernet.EthernetStateTracker;
import android.net.ethernet.EthernetDevInfo;
import android.os.SystemProperties;
import android.provider.Settings;
import android.util.Log;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

public class EthernetService<syncronized> extends IEthernetManager.Stub{
	private static final boolean LOCAL_LOG =  true;
	private Context mContext;
	private EthernetStateTracker mTracker;
	private String[] DevName;
	private static final String TAG = "EthernetService";
	private int isEthEnabled ;
	private int mEthState= EthernetManager.ETH_STATE_UNKNOWN;

	//xelloss
	private static int mCheckConnecting=0;
	private static boolean mStackconnected=false;
	private static boolean mHWconnected=false;
	private static boolean mFoundEthernetDevice = false;
	private static final String ETH_DEV0 = "eth0";
	private static final String ETH_DEV1 = "eth1";

	public EthernetService(Context context, EthernetStateTracker Tracker){
		mTracker = Tracker;
		mContext = context;

		if (!SystemProperties.getBoolean("ro.QB.enable", false)) {
			isEthEnabled = getPersistedState();

			getDeviceNameList();
			setEthState(isEthEnabled);

			if (LOCAL_LOG)
				Log.i(TAG, "Ethernet dev enabled " + isEthEnabled
						+ "Trigger the ethernet monitor "
						+ mFoundEthernetDevice);

			boolean mMediaBox = mContext.getResources().getBoolean(com.android.internal.R.bool.config_enable_displaymanager);
			
			if(!mMediaBox)
				mFoundEthernetDevice = false;

			// if(isEthConfigured())//xelloss
			if (mFoundEthernetDevice) {
				mTracker.StartPolling();
			}
		}
	}
	
	public void SystemReady() {
		isEthEnabled = getPersistedState();
		getDeviceNameList();
		setEthState(isEthEnabled);
		
		if ( SystemProperties.getBoolean("ro.QB.enable", false)) {
			if (mFoundEthernetDevice) {
				mTracker.StartPolling();
			}
		}
	}

	public boolean isEthConfigured() {

		final ContentResolver cr = mContext.getContentResolver();
		int x = Settings.System.getInt(cr, Settings.System.ETH_CONF,0);

		if (x == 1)
			return true;
		return false;
	}

	public synchronized EthernetDevInfo getSavedEthConfig() {

		if (isEthConfigured() ) {
			final ContentResolver cr = mContext.getContentResolver();
			EthernetDevInfo info = new EthernetDevInfo();
			info.setConnectMode(Settings.System.getString(cr, Settings.System.ETH_MODE));
			info.setIfName(Settings.System.getString(cr, Settings.System.ETH_IFNAME));
			info.setIpAddress(Settings.System.getString(cr, Settings.System.ETH_IP));
			info.setDnsAddr(Settings.System.getString(cr, Settings.System.ETH_DNS));
			info.setNetMask(Settings.System.getString(cr, Settings.System.ETH_MASK));
			info.setRouteAddr(Settings.System.getString(cr, Settings.System.ETH_ROUTE));
			
//Log.d(TAG, "getSavedEthConfig dev: " +  info.getIfName() + " IP: " +info.getIpAddress() + " DNS: " + info.getDnsAddr() + " gateway: " +  info.getRouteAddr() + " netmask: " + info.getNetMask());
			return info;
		}
		return null;
	}


	public synchronized void setEthMode(String mode) {
		final ContentResolver cr = mContext.getContentResolver();
		if (DevName != null) {
			Settings.System.putString(cr, Settings.System.ETH_IFNAME, DevName[0]);
			Settings.System.putInt(cr, Settings.System.ETH_CONF,1);
			Settings.System.putString(cr, Settings.System.ETH_MODE, mode);
		}
	}

	public synchronized void UpdateEthDevInfo(EthernetDevInfo info) {
		final ContentResolver cr = mContext.getContentResolver();
	    Settings.System.putInt(cr, Settings.System.ETH_CONF,1);
	    Settings.System.putString(cr, Settings.System.ETH_IFNAME, info.getIfName());
	    Settings.System.putString(cr, Settings.System.ETH_IP, info.getIpAddress());
	    Settings.System.putString(cr, Settings.System.ETH_MODE, info.getConnectMode());
	    Settings.System.putString(cr, Settings.System.ETH_DNS, info.getDnsAddr());
	    Settings.System.putString(cr, Settings.System.ETH_ROUTE, info.getRouteAddr());
	    Settings.System.putString(cr, Settings.System.ETH_MASK,info.getNetMask());
		
		Log.d(TAG, "UpdateEthDevInfo dev: " +  info.getIfName() + " IP: " +info.getIpAddress() + " DNS: " + info.getDnsAddr() + " gateway: " +  info.getRouteAddr() + " netmask: " + info.getNetMask());
		//xelloss debugging and todo later- block this start routine , static IP connection -> DHCP spend long time to receive error 'Nak'
	    if (mEthState == EthernetManager.ETH_STATE_ENABLED) {
			//setEthState(EthernetManager.ETH_STATE_DISABLED);
			//setEthState(EthernetManager.ETH_STATE_ENABLED);
			
		try {
				mTracker.resetInterface();
			} catch (UnknownHostException e) {
				//xelloss
				final Intent intent = new Intent(EthernetManager.ETH_STATE_CHANGED_ACTION);
				intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
		
				intent.putExtra(EthernetManager.EXTRA_ETH_EVENT, EthernetStateTracker.EVENT_INTERFACE_CONFIGURATION_FAILED);
				intent.putExtra(EthernetManager.EXTRA_ETH_STATE, EthernetManager.ETH_STATE_DISABLED);
				mContext.sendStickyBroadcast(intent);
				mTracker.sendEmptyMessage((2/*EVENT_INTERFACE_CONFIGURATION_FAILED*/));
				Log.e(TAG, "Wrong ethernet configuration");
			}

	    }
	    

	}

	public int getTotalInterface() {
		return EthernetNative.getInterfaceCnt();
	}
/*
    private void enforceAccessPermission() {
        mContext.enforceCallingOrSelfPermission(android.Manifest.permission.ACCESS_ETH_STATE,
                                                "EthernetService");
    }

    private void enforceChangePermission() {
        mContext.enforceCallingOrSelfPermission(android.Manifest.permission.CHANGE_ETH_STATE,
                                                "EthernetService");

    }
*/
	private int scanEthDevice() {
		int i = 0,j;
		String ethDevName;
		int k=0;
		
		if ((i = EthernetNative.getInterfaceCnt()) != 0) {
			//Log.i(TAG, "total found "+i+ " net devices");
			//DevName = new String[i];
			
			for (j = 0; j < i; j++) {
				//check only eth0 or eth1 device
				ethDevName = EthernetNative.getInterfaceName(j);
				if(ethDevName != null){
					if (ethDevName.equals(ETH_DEV0) || ethDevName.equals(ETH_DEV1)) {
						mFoundEthernetDevice = true;
						k++;
					}
				}
				else{
					break;
				}
			}

			if(LOCAL_LOG) Log.i(TAG, "total found "+i+ " net devices " + " k " + k);

			if(mFoundEthernetDevice)
				DevName = new String[k];
			
		}
		else
			return i;

		for (j = 0,k=0; j < i; j++) {
			
			//check only eth0 or eth1 device
			ethDevName = EthernetNative.getInterfaceName(j);
			if(ethDevName != null){
				if (ethDevName.equals(ETH_DEV0) || ethDevName.equals(ETH_DEV1)) {
					DevName[k] = ethDevName;
					if(LOCAL_LOG) Log.i(TAG,"device " + k + " name " + DevName[k]);
					k++;
					
				}
			}
			else{
				break;
		}

			//DevName[j] = EthernetNative.getInterfaceName(j);
			//if (DevName[j] == null)
			//	break;
			//Log.i(TAG,"device " + j + " name " + DevName[j]);
		}
		
		return k;
		//return i;

	}

	public String[] getDeviceNameList() {
		if (scanEthDevice() > 0 )
			return DevName;
		else
			return null;
	}

	private int getPersistedState() {
		final ContentResolver cr = mContext.getContentResolver();
		try {
			//return 1;
			return Settings.System.getInt(cr, Settings.System.ETH_ON);
		} catch (Settings.SettingNotFoundException e) {
			return EthernetManager.ETH_STATE_UNKNOWN;
		}
	}

	private synchronized void persistEthEnabled(boolean enabled) {
		if (SystemProperties.getBoolean("ro.QB.enable", false) && SystemProperties.getBoolean("persist.sys.QB.user.allow", false)) {			
			String quickboot_status = SystemProperties.get("tcc.QB.boot.with", "not_yet");
	        if( !quickboot_status.equals("snapshot") && !quickboot_status.equals("hibernate") ) {
	        	return;
	        }
		}
		
	    final ContentResolver cr = mContext.getContentResolver();
	    Settings.System.putInt(cr, Settings.System.ETH_ON,
		enabled ? EthernetManager.ETH_STATE_ENABLED : EthernetManager.ETH_STATE_DISABLED);
	}

	public synchronized void setEthState(int state) {
		if(LOCAL_LOG) Log.i(TAG, "setEthState from " + mEthState + " to "+ state);
		boolean skip_resetInterface = false;
		if (mEthState != state){
			//prevent twice resetinterface -> configureInterface for status icon
			//(getCheckConnecting()==1) means DHCP is active, don't try it again.
			if ((mEthState == EthernetManager.ETH_STATE_ENABLING && state == EthernetManager.ETH_STATE_ENABLED)
				|| (getCheckConnecting()==1) ) {
				if(LOCAL_LOG) Log.d(TAG, " already resetInterface ");
				skip_resetInterface = true;
			}
			mEthState = state;
			if (state == EthernetManager.ETH_STATE_DISABLED) {
				persistEthEnabled(false);
				mTracker.stopInterface(false);
			} else {
				persistEthEnabled(true);
				//mTracker.setCheckConnecting(1);
				if (!isEthConfigured()) {
					// If user did not configure any interfaces yet, pick the first one
					// and enable it.
					//xelloss block because of setting ETH_CONF evne not using Ethernet menu
					//if(mFoundEthernetDevice)
						//setEthMode(EthernetDevInfo.ETH_CONN_MODE_DHCP);
				}
				if(skip_resetInterface == true) {
					return;
				}
				try {
					mTracker.resetInterface();
				} catch (UnknownHostException e) {
					//xelloss
					//mTracker.setCheckConnecting(0);
					final Intent intent = new Intent(EthernetManager.ETH_STATE_CHANGED_ACTION);
					intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
			
					intent.putExtra(EthernetManager.EXTRA_ETH_EVENT, EthernetStateTracker.EVENT_INTERFACE_CONFIGURATION_FAILED);
					intent.putExtra(EthernetManager.EXTRA_ETH_STATE, EthernetManager.ETH_STATE_DISABLED);
					mContext.sendStickyBroadcast(intent);
					mTracker.sendEmptyMessage(2/*EVENT_INTERFACE_CONFIGURATION_FAILED*/);
					Log.e(TAG, "Wrong ethernet configuration");
				}
			}
		}
	}

	public int getEthState( ) {
		return mEthState;
	}

	//xelloss
	public int getCheckConnecting()
	{
		if(LOCAL_LOG)  Log.d(TAG, "getCheckConnecting:	mCheckCount " + mCheckConnecting);
		return mCheckConnecting;
	}

	public void setCheckConnecting(int value)
	{
		mCheckConnecting = value;
		if(LOCAL_LOG) Log.d(TAG, "setCheckConnecting:	mCheckConnecting " + mCheckConnecting);
	}

	//xelloss
	public boolean getStackConnected()
	{
		if(LOCAL_LOG) Log.d(TAG, "getStackConnected:	mStackconnected " + mStackconnected);
		return mStackconnected;
	}

	public void setStackConnected(boolean value)
	{
		mStackconnected = value;
		if(LOCAL_LOG) Log.d(TAG, "setStackConnected:	mStackconnected " + mStackconnected);
	}

	public boolean getHWConnected()
	{
		if(LOCAL_LOG) Log.d(TAG, "getHWConnected:  mHWconnected " + mHWconnected);
		return mHWconnected;
	}

	public void setHWConnected(boolean value)
	{
		mHWconnected = value;
		if(LOCAL_LOG) Log.d(TAG, "setHWConnected:  mHWconnected " + mHWconnected);
	}

	public boolean isEthDeviceFound() 
	{
		if(LOCAL_LOG) Log.d(TAG, "isFoundEthDevice: mFoundEthernetDevice " + mFoundEthernetDevice);
		return mFoundEthernetDevice;

}
}
