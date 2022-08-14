package com.android.settings.telechips.ethernet;

import static android.net.ethernet.EthernetManager.ETH_STATE_DISABLED;
import static android.net.ethernet.EthernetManager.ETH_STATE_ENABLED;
import static android.net.ethernet.EthernetManager.ETH_STATE_UNKNOWN;
import static android.net.ethernet.EthernetManager.ETH_STATE_ENABLING;//xelloss

import static android.net.ethernet.EthernetStateTracker.EVENT_INTERFACE_CONFIGURATION_FAILED;
import static android.net.ethernet.EthernetStateTracker.EVENT_HW_DISCONNECTED;

import com.android.settings.R;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.NetworkInfo;
import android.net.ethernet.EthernetManager;
import android.net.ethernet.EthernetDevInfo;
import android.preference.Preference;
import android.preference.CheckBoxPreference;
import android.text.TextUtils;
import android.util.Config;
import android.util.Log;
import android.os.SystemProperties;

public class EthernetEnabler implements Preference.OnPreferenceChangeListener{
		private static final boolean LOCAL_LOGD = true;
		private static final String TAG = "EthernetSetting";
		//private final IntentFilter mEthStateFilter;
	    private Context mContext;
	    private EthernetManager mEthManager;
	    private CheckBoxPreference mEthCheckBoxPref;
		private Preference mEthConfigPref;
	    private final CharSequence mOriginalSummary;
	    private EthernetConfigDialog mEthConfigDialog;
		private final IntentFilter mIntentFilter;
		
		//this flag is for checking disconnect event when sleep and wake up
		private int mStartMode = START_FROM_NONE;
		public static final int START_FROM_MENU = 1;
		public static final int START_FROM_RESUME = 2;
		public static final int START_FROM_NONE = 3;

		private final BroadcastReceiver mEthStateReceiver = new BroadcastReceiver() {

	        @Override
	        public void onReceive(Context context, Intent intent) {
	        	Log.d(TAG,"intent.getAction() " + intent.getAction());
	            if (intent.getAction().equals(EthernetManager.ETH_STATE_CHANGED_ACTION)) {
	                handleEthStateChanged(
	                        intent.getIntExtra(EthernetManager.EXTRA_ETH_EVENT,
									EVENT_INTERFACE_CONFIGURATION_FAILED),
	                        intent.getIntExtra(EthernetManager.EXTRA_ETH_STATE,
								EthernetManager.ETH_STATE_UNKNOWN));
	            } else if (intent.getAction().equals(EthernetManager.NETWORK_STATE_CHANGED_ACTION)) {
	                handleNetworkStateChanged(
	                        (NetworkInfo) intent.getParcelableExtra(EthernetManager.EXTRA_NETWORK_INFO));
	            }
	        }
	    };

	    public void setConfigDialog (EthernetConfigDialog Dialog) {
		 mEthConfigDialog = Dialog;
	    }

	    public EthernetEnabler(Context context,
			EthernetManager ethernetManager,
	            CheckBoxPreference ethernetCheckBoxPreference,
	            Preference ethernetConfigPreference) {
	        mContext = context;
	        mEthCheckBoxPref = ethernetCheckBoxPreference;
			mEthConfigPref = ethernetConfigPreference;
	        mEthManager = ethernetManager;

	        mOriginalSummary = ethernetCheckBoxPreference.getSummary();
	        ethernetCheckBoxPreference.setPersistent(false);
			
//			Log.d(TAG, "EthernetEnabler mEthManager.getEthState() " + mEthManager.getEthState()  + "Statckconnected " + mEthManager.getStackConnected() + "HWconnected " + mEthManager.getHWConnected() + "connecting" +  mEthManager.getCheckConnecting());
	        if(mEthManager.getEthState() == ETH_STATE_ENABLED) {
				mEthCheckBoxPref.setChecked(true);
				//mEthCheckBoxPref.setSummary(R.string.eth_toggle_summary_off);

				EthernetDevInfo info = mEthManager.getSavedEthConfig();
				if (info != null ){
					String ipaddr = null;
					if(info.getConnectMode().equals(EthernetDevInfo.ETH_CONN_MODE_DHCP)){
				String ipprop="dhcp."+"eth0"+".ipaddress";
						ipaddr = SystemProperties.get(ipprop);
					}
					else{
						ipaddr = info.getIpAddress();
					}
					
				String detail = mContext.getString(R.string.eth_toggle_summary_off) + "<" +mContext.getString(R.string.eth_connected_ipaddr) + ipaddr + ">" ;
				mEthCheckBoxPref.setSummary(detail);
				}
				
				if(mEthManager.getStackConnected() ==false || mEthManager.getHWConnected()==false){
					//this checking is for connected dhcp -> unplug cable -> plug cable for Static iP
					if(mEthManager.getCheckConnecting() == 1)
					{
						mEthCheckBoxPref.setEnabled(false);
						mEthCheckBoxPref.setSummary( R.string.connect_starting);

						mEthManager.setEthernetState(mEthManager.ETH_STATE_ENABLING);
					}
					else{
						//mEthCheckBoxPref.setChecked(false);
						mEthCheckBoxPref.setSummary(R.string.eth_disconnected);
					}
				}
			
	        }
			else if(mEthManager.getEthState() == ETH_STATE_ENABLING)
			{
				mEthCheckBoxPref.setEnabled(false);
				mEthCheckBoxPref.setSummary( R.string.connect_starting);
			}
			//xelloss set intentfilter and regist broadcast receive fot checking eth connecting status
			/*
	        IntentFilter intentFilter = new IntentFilter();
			intentFilter.addAction(EthernetManager.ETH_STATE_CHANGED_ACTION);
			mContext.registerReceiver(mEthStateReceiver, intentFilter);
			*/
			mIntentFilter = new IntentFilter();
			mIntentFilter.addAction(EthernetManager.ETH_STATE_CHANGED_ACTION);

			mStartMode = START_FROM_MENU;

	    }



		public EthernetManager getManager() {
		return mEthManager;
	    }

	    public void resume() {
			if(mStartMode != START_FROM_MENU)
			{
				mStartMode = START_FROM_RESUME;
			}
			mContext.registerReceiver(mEthStateReceiver, mIntentFilter);
	        mEthCheckBoxPref.setOnPreferenceChangeListener(this);
	    }

	    public void pause() {
			mStartMode = START_FROM_NONE;
	        mContext.unregisterReceiver(mEthStateReceiver);
	        mEthCheckBoxPref.setOnPreferenceChangeListener(null);
	    }

		public boolean onPreferenceChange(Preference preference, Object newValue) {

			setEthEnabled((Boolean)newValue);
			return false;
		}

	    //private void setEthEnabled(final boolean enable) {
	    public void setEthEnabled(final boolean enable) {

	        int state = mEthManager.getEthState();

			Log.i(TAG,"setEthEnabled enable " + enable + " state " + state);
			
			// Disable button
	        mEthCheckBoxPref.setEnabled(false);

	        if (state != ETH_STATE_ENABLED && state != ETH_STATE_ENABLING && enable) {
				if (mEthManager.ethConfigured() != true) {
					// Now, kick off the setting dialog to get the configurations
					mEthConfigDialog.enableAfterConfig();
					mEthConfigDialog.show();
					return; //xelloss
				} else {
					//xelloss add connecting state 
					//mEthManager.setEthEnabled(enable);
					mEthManager.setEthernetState(mEthManager.ETH_STATE_ENABLING);
					mEthCheckBoxPref.setSummary( R.string.connect_starting);
				}
	        } else {
				mEthManager.setEthEnabled(enable);
	        }
			
			state = mEthManager.getEthState();//xelloss get eth state again 
			
			if(state == ETH_STATE_ENABLED || state == ETH_STATE_DISABLED)//xelloss check eth state for checkbox finally
			{
				mEthCheckBoxPref.setChecked(enable);
				mEthCheckBoxPref.setSummary( enable?R.string.eth_toggle_summary_off:R.string.eth_toggle_summary_on);
				mEthCheckBoxPref.setEnabled(true);
				
				if(enable){
					EthernetDevInfo info = mEthManager.getSavedEthConfig();
					if (info != null ){
						String ipaddr = null;
						if(info.getConnectMode().equals(EthernetDevInfo.ETH_CONN_MODE_DHCP)){
					String ipprop="dhcp."+"eth0"+".ipaddress";
							ipaddr = SystemProperties.get(ipprop);
						}
						else{
							ipaddr = info.getIpAddress();
						}
						
					String detail = mContext.getString(R.string.eth_toggle_summary_off) + "<" +mContext.getString(R.string.eth_connected_ipaddr) + ipaddr + ">" ;
					mEthCheckBoxPref.setSummary(detail);
				}
			}
			}
	        
	    }
		
		public void setCheckBox(int state) 
		{
			Log.i(TAG,"setCheckBox  req state " + state + "now state " +  mEthManager.getEthState());
			
			if(state == ETH_STATE_ENABLING )
			{
				// Disable button
				mEthCheckBoxPref.setEnabled(false);
				
				mEthCheckBoxPref.setSummary( R.string.connect_starting);
			}
			else
			{
				int eth_state = mEthManager.getEthState();

				if(eth_state != ETH_STATE_ENABLING)
				{
				mEthCheckBoxPref.setEnabled(true);
			}
		}

		}

	    private void handleEthStateChanged(int ethEvent, int ethState) {
			 int state = mEthManager.getEthState();
			 Log.d(TAG, "handleEthStateChanged event "
					 + ethEvent + " to state "
					 + ethState + " from current state "+state + "getCheckConnecting " + mEthManager.getCheckConnecting());

			if( (mStartMode != START_FROM_MENU) || mEthManager.getCheckConnecting() == 1)
			{
				mStartMode = START_FROM_MENU;
				Log.d(TAG,"previous connecting has not done. just return");
				return;
			}
			 
			if (state == ETH_STATE_ENABLING && (  ethState == ETH_STATE_ENABLED)) {
				setEthEnabled(ethState == ETH_STATE_ENABLED);
			}
			else if(state == ETH_STATE_ENABLING && ethState == ETH_STATE_DISABLED)
			{
				//connection_modification
				//setEthEnabled(ethState == ETH_STATE_ENABLED);
				//mEthCheckBoxPref.setSummary(R.string.connect_error);
				// Menu - fail to try connecting - display "disconected and checked"
				mEthCheckBoxPref.setChecked(true);
				mEthCheckBoxPref.setSummary(R.string.eth_disconnected);
				mEthCheckBoxPref.setEnabled(true);
			}
			else{
				//connection_modification
				if( ethState == ETH_STATE_ENABLED)//success to connect
				{
					setEthEnabled(ethState == ETH_STATE_ENABLED);
				}
				// Menu - cable out - display "disconected and checked"
				else if( ethEvent == EVENT_HW_DISCONNECTED && (ethState == ETH_STATE_DISABLED && state != ethState))
				{
					mEthCheckBoxPref.setChecked(true);
					mEthCheckBoxPref.setSummary(R.string.eth_disconnected);
					mEthCheckBoxPref.setEnabled(true);
				}
			}
			
	    }

	    private void handleNetworkStateChanged(NetworkInfo networkInfo) {

	        if (LOCAL_LOGD) {
	            Log.d(TAG, "Received network state changed to " + networkInfo);
	        }
	        /*
	        if (mEthernetManager.isEthEnabled()) {
	            String summary = ethStatus.getStatus(mContext,
	                    mEthManager.getConnectionInfo().getSSID(), networkInfo.getDetailedState());
	            mEthCheckBoxPref.setSummary(summary);
	        }
	        */
	    }

	    private boolean isEnabledByDependency() {
	        Preference dep = getDependencyPreference();
	        if (dep == null) {
	            return true;
	        }

	        return !dep.shouldDisableDependents();
	    }

	    private Preference getDependencyPreference() {
	        String depKey = mEthCheckBoxPref.getDependency();
	        if (TextUtils.isEmpty(depKey)) {
	            return null;
	        }

	        return mEthCheckBoxPref.getPreferenceManager().findPreference(depKey);
	    }

	    private static String getHumanReadableEthState(int ethState) {

	        switch (ethState) {
	            case ETH_STATE_DISABLED:
	                return "Disabled";
	            case ETH_STATE_ENABLED:
	                return "Enabled";
	            case ETH_STATE_UNKNOWN:
	                return "Unknown";
	            default:
	                return "Some other state!";
	        }

	    }
}
