package android.net.ethernet;

import java.util.List;

import android.annotation.SdkConstant;
import android.annotation.SdkConstant.SdkConstantType;
import android.net.wifi.IWifiManager;
import android.os.Handler;
import android.os.RemoteException;
import android.util.Log;

/**
 * @hide
 */
public class EthernetManager {
	public static final String TAG = "EthernetManager";
	public static final int ETH_DEVICE_SCAN_RESULT_READY = 0;
	public static final String ETH_STATE_CHANGED_ACTION =
	        "android.net.ethernet.ETH_STATE_CHANGED";
	public static final String NETWORK_STATE_CHANGED_ACTION =
			"android.net.ethernet.STATE_CHANGE";

   // public static final String ACTION_ETH_NETWORK = "android.net.ethernet.ETH_NET_CHG";

	public static final String EXTRA_NETWORK_INFO = "networkInfo";
	public static final String EXTRA_ETH_EVENT = "eth_event";
	public static final String EXTRA_ETH_STATE = "eth_state";

	public static final int ETH_STATE_DISABLED = 1;
	public static final int ETH_STATE_ENABLED = 2;
	public static final int ETH_STATE_ENABLING = 3;//xelloss, this is for connecting or opening state
    public static final int ETH_STATE_UNKNOWN = 0;

	IEthernetManager mService;
	Handler mHandler;

	//xelloss
	public void setCheckConnecting(int value) {
			try {
				mService.setCheckConnecting(value);
			} catch (RemoteException e) {
				Log.i(TAG,"Can not setCheckConnecting");
			}
		}
	
	public int getCheckConnecting( ) {
		try {
			 return mService.getCheckConnecting();
		} catch (RemoteException e) {
		   return 0;
		}
	}
	
	public void setStackConnected(boolean value) {
			try {
				mService.setStackConnected(value);
			} catch (RemoteException e) {
				Log.i(TAG,"Can not setStackConnected");
			}
		}
	
	public boolean getStackConnected( ) {
		try {
			 return mService.getStackConnected();
		} catch (RemoteException e) {
		   return false;
		}
	}

	public void setHWConnected(boolean value) {
			try {
				mService.setHWConnected(value);
			} catch (RemoteException e) {
				Log.i(TAG,"Can not setHWConnected");
			}
		}
	
	public boolean getHWConnected( ) {
		try {
			 return mService.getHWConnected();
		} catch (RemoteException e) {
		   return false;
		}
	}

	public boolean isEthDeviceFound( ) {
		try {
			if( mService == null )
				return false;
			
			 return mService.isEthDeviceFound();
		} catch (RemoteException e) {
		   return false;
		}
	}
	
	public boolean isEthConfigured() {

		try {
			return mService.isEthConfigured();
		} catch (RemoteException e) {
			Log.i(TAG, "Can not check eth config state");
		}
		return false;
	}

	public EthernetDevInfo getSavedEthConfig() {
		try {
			return mService.getSavedEthConfig();
		} catch (RemoteException e) {
			Log.i(TAG, "Can not get eth config");
       }
		return null;
	}
	public void UpdateEthDevInfo(EthernetDevInfo info) {
		try {
			mService.UpdateEthDevInfo(info);
		} catch (RemoteException e) {
			Log.i(TAG, "Can not update ethernet device info");
       }
	}

	public EthernetManager(IEthernetManager service, Handler handler) {
		Log.d(TAG, "Init Ethernet Manager");
		mService = service;
		mHandler = handler;
	}


	public String[] getDeviceNameList() {
		try {
			 return mService.getDeviceNameList();
		} catch (RemoteException e) {
            return null;
        }
	}

	public void setEthEnabled(boolean enable) {
		try {
			mService.setEthState(enable ? ETH_STATE_ENABLED:ETH_STATE_DISABLED);
		} catch (RemoteException e) {
			Log.i(TAG,"Can not set new state");
		}
	}
	
	public void setEthernetState(int state) {
			try {
				mService.setEthState(state);
			} catch (RemoteException e) {
				Log.i(TAG,"Can not set new state");
			}
		}

	public int getEthState( ) {
		try {
			 return mService.getEthState();
		} catch (RemoteException e) {
           return 0;
		}
	}
	

	public boolean ethConfigured() {
		try {
			 return mService.isEthConfigured();
		} catch (RemoteException e) {
          return false;
		}

	}

	public int getTotalInterface() {
		try {
			 return mService.getTotalInterface();
		} catch (RemoteException e) {
			return 0;
		}
	}

	public void ethSetDefaultConf() {
		try {
			mService.setEthMode(EthernetDevInfo.ETH_CONN_MODE_DHCP);
		} catch (RemoteException e) {
		}
	}

}
