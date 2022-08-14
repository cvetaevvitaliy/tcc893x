package android.net.ethernet;

import java.net.InetAddress;
import java.net.UnknownHostException;

import android.R;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.bluetooth.BluetoothHeadset;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.net.ConnectivityManager;
import android.net.DhcpInfo;
//import android.net.NetworkStateTracker;
import android.net.BaseNetworkStateTracker;
import android.net.NetworkUtils;
import android.net.NetworkInfo.DetailedState;
import android.net.wifi.WifiManager;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.os.SystemProperties;
import android.util.*;
import android.net.wifi.SupplicantState;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiInfo;
import android.net.NetworkInfo;
import android.net.DhcpResults;
import android.net.LinkProperties;
import android.net.LinkCapabilities;
import android.os.INetworkManagementService;
import android.net.InterfaceConfiguration;
import android.net.RouteInfo;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.IBinder;
import android.os.storage.IMountService;
import android.net.networkfilesystem.NetworkFileSystemManager;
import android.net.networkfilesystem.NetworkFileSystemDevInfo;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.os.Environment;
import android.os.Messenger;

import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Track the state of Ethernet connectivity. All event handling is done here,
 * and all changes in connectivity state are initiated here.
 *
 * @hide
 */
public class EthernetStateTracker extends BaseNetworkStateTracker {
    private static final String NETWORKTYPE = "ETHERNET";
	private static final boolean LOCAL_LOG =  true;
	private static final String TAG="EthernetStateTracker";
	public static final int EVENT_DHCP_START			= 0;
	public static final int EVENT_INTERFACE_CONFIGURATION_SUCCEEDED = 1;
	public static final int EVENT_INTERFACE_CONFIGURATION_FAILED	= 2;
	public static final int EVENT_HW_CONNECTED			= 3;
	public static final int EVENT_HW_DISCONNECTED			= 4;
	public static final int EVENT_HW_PHYCONNECTED			= 5;
	private static final int NOTIFY_ID				= 6;
	private EthernetManager mEM;
	private boolean mServiceStarted;

	private boolean mStackConnected;
	private boolean mHWConnected;
	private boolean mPHYup;
	private boolean mInterfaceStopped;
	private DhcpHandler mDhcpTarget;
	private String mInterfaceName ;
	private DhcpInfo mDhcpInfo;
	private EthernetMonitor mMonitor;
	private String[] sDnsPropNames;
	private boolean mStartingDhcp;
	private NotificationManager mNotificationManager;
	private Notification mNotification;
	private WifiManager mWifiManager;
	private SupplicantState mSupplicantState;
	private String mWifiMac;
	private boolean mEthConnected;

    private AtomicBoolean mTeardownRequested = new AtomicBoolean(false);
    private AtomicBoolean mPrivateDnsRouteSet = new AtomicBoolean(false);
    private AtomicInteger mDefaultGatewayAddr = new AtomicInteger(0);
    private AtomicBoolean mDefaultRouteSet = new AtomicBoolean(false);

    private static String sIfaceMatch = "";
    private static String mIface = "";
    private static boolean mLinkUp;
	private LinkProperties mLinkProperties;
    private LinkCapabilities mLinkCapabilities;
    private NetworkInfo mNetworkInfo;
	private DhcpResults mDhcpResults;
	private Handler mMYHandler;
    private Context mContext;
	private Handler mTarget;
    private INetworkManagementService mNwService;
	private NetworkInfo mNetworkWifiInfo;

    private IMountService mMountService = null;
    private StorageManager mStorageManager;
	private NetworkFileSystemManager mNFSManager;

	public EthernetStateTracker(Context context, Handler target) {
		if(LOCAL_LOG) Log.i(TAG,"Starts...");
		mContext = context;
		mTarget = target;
		mNetworkInfo = new NetworkInfo(ConnectivityManager.TYPE_ETHERNET, 0, NETWORKTYPE, "");
		mLinkProperties = new LinkProperties();
		mLinkCapabilities = new LinkCapabilities();

        mNetworkInfo.setIsAvailable(false);
        setTeardownRequested(false);

		if(EthernetNative.initEthernetNative() != 0 )
		{
			Log.e(TAG,"Can not init ethernet device layers");
			return;
		}
		if(LOCAL_LOG) Log.i(TAG,"Successed");
		mServiceStarted = true;

		HandlerThread handlerThread = new HandlerThread("Ethernet Tracker Thread");
        handlerThread.start();
		mMYHandler = new MyHandler(handlerThread.getLooper());

		HandlerThread dhcpThread = new HandlerThread("DHCP Handler Thread");
		dhcpThread.start();
		mDhcpTarget = new DhcpHandler(dhcpThread.getLooper(), mMYHandler);

		mMonitor = new EthernetMonitor(this);
		mDhcpInfo = new DhcpInfo();
		mDhcpResults = new DhcpResults();
		setTeardownRequested(false);

        IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
        mNwService = INetworkManagementService.Stub.asInterface(b);

		IntentFilter filter = new IntentFilter();
		filter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
        mContext.registerReceiver(new checkWifiStateReceiver(), filter);

		if (mNFSManager == null) {
			mNFSManager =(NetworkFileSystemManager)mContext.getSystemService(Context.NFS_SERVICE);
		}
		if (mStorageManager == null) {
        	mStorageManager = (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);
		}

		// NFS
        IntentFilter nfs_filter = new IntentFilter();
        nfs_filter.addAction(Intent.ACTION_MEDIA_MOUNTED);
        nfs_filter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        nfs_filter.addDataScheme("file");
        mContext.registerReceiver(mNfsReceiver, nfs_filter);

        IntentFilter net_filter = new IntentFilter();
        net_filter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
        net_filter.addAction(EthernetManager.ETH_STATE_CHANGED_ACTION);
        mContext.registerReceiver(mNetworkReceiver, net_filter);
		//
	}

	// NFS
    private void UnmountAllNfs() {
        IMountService mountService = getMountService();
        StorageVolume volumes[] = mStorageManager.getVolumeList();

        int i;
        for (i = 0; i < volumes.length; i++) {
            doUnmountNfs(volumes[i].getPath());
        }
    }

    private boolean doUnmountNfs(String path) {
        NetworkFileSystemDevInfo info = mNFSManager.getNfsConfig(path);
        if (info == null) {
            return false;
        }

        IMountService mountService = getMountService();
        try {
            if (mountService.getVolumeState(path).equals(Environment.MEDIA_MOUNTED)) {
                mountService.unmountVolume(path, true, false);
            }
        } catch (RemoteException e) {
            return false;
        }
       return true;
    }

    private synchronized IMountService getMountService() {
        if (mMountService == null) {
            IBinder service = ServiceManager.getService("mount");
            if (service != null) {
                mMountService = IMountService.Stub.asInterface(service);
            } else {
                Log.e(TAG, "Can't get mount service");
            }
        }
        return mMountService;
    }

    private final BroadcastReceiver mNfsReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (LOCAL_LOG) Log.d(TAG, "Intent "+ intent);

            String action = intent.getAction();
            String path = intent.getData().getPath();

            if (mNFSManager!=null && mNFSManager.getNfsConfig(path) == null) {
                return;
            }
            if (action.equals(Intent.ACTION_MEDIA_MOUNTED)) {
                final Intent nfs_intent = new Intent(NetworkFileSystemManager.NFS_STATE_CHANGED_ACTION);
                nfs_intent.putExtra(NetworkFileSystemManager.EXTRA_NFS_STATE, NetworkFileSystemManager.EVENT_NFS_MOUNTED);
                mContext.sendStickyBroadcast(nfs_intent);
            } else if (action.equals(Intent.ACTION_MEDIA_UNMOUNTED) && mNFSManager !=null &&!mNFSManager.getNfsState(null).equals(Environment.MEDIA_MOUNTED)) {
                final Intent nfs_intent = new Intent(NetworkFileSystemManager.NFS_STATE_CHANGED_ACTION);
                nfs_intent.putExtra(NetworkFileSystemManager.EXTRA_NFS_STATE, NetworkFileSystemManager.EVENT_NFS_UNMOUNTED);
                mContext.sendStickyBroadcast(nfs_intent);
            }
        }
    };

    private final BroadcastReceiver mNetworkReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (LOCAL_LOG) Log.d(TAG, "Intent "+ intent);

            String action = intent.getAction();
            if (!action.equals(WifiManager.WIFI_STATE_CHANGED_ACTION) && !action.equals(EthernetManager.ETH_STATE_CHANGED_ACTION)) {
                return;
            }

            boolean wifiState = SystemProperties.get("dhcp.wlan0.result").equals("ok");
            boolean ethState = SystemProperties.get("dhcp.eth0.result").equals("ok");
            if (LOCAL_LOG) Log.d(TAG, " wifiState " + wifiState + " ethState " +ethState);
            if (!wifiState && !ethState) {
               	UnmountAllNfs();
            }
        }
    };
	//

	private class checkWifiStateReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
			if(action.equals(ConnectivityManager.CONNECTIVITY_ACTION)){

				NetworkInfo info = (NetworkInfo)(intent.getParcelableExtra(ConnectivityManager.EXTRA_NETWORK_INFO));

				// ethernet act like wifi but now wifi is terminated after connecting ethernet. wifi enable - ethernet connection - ethernet connected - wifi disconnect
				if (mEthConnected && info.getType() == ConnectivityManager.TYPE_WIFI && info.getDetailedState() == DetailedState.DISCONNECTED)
				{
					setWifiVirtualState(DetailedState.CONNECTED);
				}
			}
        }
	}
/*
    private static class InterfaceObserver extends INetworkManagementEventObserver.Stub {
        private EthernetStateTracker mTracker;

        InterfaceObserver(EthernetStateTracker tracker) {
            super();
            mTracker = tracker;
        }

        public void interfaceStatusChanged(String iface, boolean up) {
            Log.d(TAG, "Interface status changed: " + iface + (up ? "up" : "down"));
        }

        public void interfaceLinkStateChanged(String iface, boolean up) {
            if (mIface.equals(iface) && mLinkUp != up) {
                Log.d(TAG, "Interface " + iface + " link " + (up ? "up" : "down"));
                mLinkUp = up;

                // use DHCP
                if (up) {
                    mTracker.reconnect();
                } else {
                    NetworkUtils.stopDhcp(mIface);
                    mTracker.mNetworkInfo.setIsAvailable(false);
                    mTracker.mNetworkInfo.setDetailedState(DetailedState.DISCONNECTED,
                                                           null, null);
                }
            }
        }

        public void interfaceAdded(String iface) {
            mTracker.interfaceAdded(iface);
        }

        public void interfaceRemoved(String iface) {
            mTracker.interfaceRemoved(iface);
        }

        public void limitReached(String limitName, String iface) {
            // Ignored.
        }
    }
*/
    private void interfaceAdded(String iface) {
        if (!iface.matches(sIfaceMatch))
            return;

        Log.d(TAG, "Adding " + iface);

        synchronized(mIface) {
            if(!mIface.isEmpty())
                return;
            mIface = iface;
        }

        mNetworkInfo.setIsAvailable(true);

        Message msg = mTarget.obtainMessage(EVENT_STATE_CHANGED, mNetworkInfo);
        msg.sendToTarget();

        reconnect();
    }

    private void interfaceRemoved(String iface) {
        if (!iface.equals(mIface))
            return;

        Log.d(TAG, "Removing " + iface);

        NetworkUtils.stopDhcp(mIface);

        mLinkProperties.clear();
        mNetworkInfo.setIsAvailable(false);
        mNetworkInfo.setDetailedState(DetailedState.DISCONNECTED, null, null);

        Message msg = mTarget.obtainMessage(EVENT_STATE_CHANGED, mNetworkInfo);
        msg.sendToTarget();

        mIface = "";
    }

	public int getCheckConnecting()
	{
		int ret=0;
		if(LOCAL_LOG) Log.i(TAG, "CheckConnecting mEM "+ mEM);

		if (mEM != null)
			ret = mEM.getCheckConnecting();

		return ret;
	}

	public void setCheckConnecting(int value)
	{
		if(LOCAL_LOG) Log.i(TAG, "setCheckConnecting mEM "+ mEM);

		if (mEM != null)
			mEM.setCheckConnecting(value);

		return;
	}

	public boolean stopInterface(boolean suspend) {
		if (mEM != null) {
			EthernetDevInfo info = mEM.getSavedEthConfig();
			if (info != null && mEM.ethConfigured())
			{
				synchronized (mDhcpTarget) {
					mInterfaceStopped = true;
					if(LOCAL_LOG) Log.i(TAG, "stop dhcp and interface");
					String ifname = info.getIfName();

					mDhcpTarget.removeMessages(EVENT_DHCP_START);
					//xelloss ,not need when not using dhcpcd service
					//String ifname = info.getIfName();
					if (info.getConnectMode().equals(EthernetDevInfo.ETH_CONN_MODE_DHCP)){
					if (!NetworkUtils.stopDhcp(ifname)) {
						Log.e(TAG, "Could not stop DHCP");
					}
					}

					NetworkUtils.resetConnections(ifname,NetworkUtils.RESET_ALL_ADDRESSES);
					if (!suspend)
						NetworkUtils.disableInterface(ifname);
				}
			}
		}

		return true;
	}

	private static int stringToIpAddr(String addrString) throws UnknownHostException {
		try {
			if (addrString == null)
				return 0;
			String[] parts = addrString.split("\\.");
			if (parts.length != 4) {
				throw new UnknownHostException(addrString);
			}

			int a = Integer.parseInt(parts[0]);
			int b = Integer.parseInt(parts[1]) <<  8;
			int c = Integer.parseInt(parts[2]) << 16;
			int d = Integer.parseInt(parts[3]) << 24;

			return a | b | c | d;
		} catch (NumberFormatException ex) {
			throw new UnknownHostException(addrString);
		}
	}

	private boolean configureInterface(EthernetDevInfo info) throws UnknownHostException {

		mStackConnected = false;
		mHWConnected = false;
		mInterfaceStopped = false;
		mStartingDhcp = true;
		sDnsPropNames = new String[] {
			"dhcp." + mInterfaceName + ".dns1",
			"dhcp." + mInterfaceName + ".dns2"
		};
		if (info.getConnectMode().equals(EthernetDevInfo.ETH_CONN_MODE_DHCP)) {
			if(LOCAL_LOG) Log.i(TAG, "trigger dhcp for device " + info.getIfName());
			mDhcpTarget.sendEmptyMessage(EVENT_DHCP_START);
		} else {
			int event;
			mStartingDhcp = false;

			mDhcpInfo.ipAddress = stringToIpAddr(info.getIpAddress());
			mDhcpInfo.gateway = stringToIpAddr(info.getRouteAddr());
			//mDhcpInfo.netmask = stringToIpAddr(info.getNetMask());
			mDhcpInfo.netmask = NetworkUtils.netmaskIntToPrefixLength(stringToIpAddr(info.getNetMask()));
			mDhcpInfo.dns1 = stringToIpAddr(info.getDnsAddr());
			mDhcpInfo.dns2 = 0;

			if(LOCAL_LOG) Log.i(TAG, "set ip manually " + mDhcpInfo.toString());

			//NetworkUtils.removeDefaultRoute(info.getIfName());
			if (NetworkUtils.configureInterface(info.getIfName(), mDhcpInfo)) {
				event = EVENT_INTERFACE_CONFIGURATION_SUCCEEDED;
				if(LOCAL_LOG) Log.v(TAG, "Static IP configuration succeeded");
			} else {
				event = EVENT_INTERFACE_CONFIGURATION_FAILED;
				if(LOCAL_LOG) Log.v(TAG, "Static IP configuration failed");
			}
			mMYHandler.sendEmptyMessage(event);

			//mDhcpResults.ipAddress = info.getIpAddress();

			try {
			InetAddress inetAddress = NetworkUtils.numericToInetAddress(info.getRouteAddr());
			//mDhcpResults.addRoute(new RouteInfo(inetAddress));
	        } catch (IllegalArgumentException e) {
	            Log.e(TAG, "error : "  + e);
	        }
			//mDhcpResults.prefixLength =NetworkUtils.netmaskIntToPrefixLength(stringToIpAddr(info.getNetMask()));
			//mDhcpResults.dns1 = info.getDnsAddr();
			//mDhcpResults.dns2 = info.getDnsAddr();

			//if(LOCAL_LOG) Log.i(TAG, "set ip manually mDhcpResults " + mDhcpResults);
			/*
			InterfaceConfiguration ifcg = new InterfaceConfiguration();
			ifcg.addr = mDhcpInfoInternal.makeLinkAddress();
			ifcg.interfaceFlags = "[up]";
			try {
				mNwService.setInterfaceConfig(mInterfaceName, ifcg);
				if(LOCAL_LOG) Log.v(TAG, "Static IP configuration succeeded");
				event = EVENT_INTERFACE_CONFIGURATION_SUCCEEDED;
			} catch (RemoteException re) {
				if(LOCAL_LOG) Log.v(TAG, "Static IP configuration failed " + re);
				event = EVENT_INTERFACE_CONFIGURATION_FAILED;
			} catch (IllegalStateException e) {
				if(LOCAL_LOG) Log.v(TAG, "Static IP configuration failed " + e);
				event = EVENT_INTERFACE_CONFIGURATION_FAILED;
			}
			mMYHandler.sendEmptyMessage(event);
*/
		}
		return true;
	}


	public boolean resetInterface()  throws UnknownHostException{
		/*
		 * This will guide us to enabled the enabled device
		 */
		if (mEM != null) {
			EthernetDevInfo info = mEM.getSavedEthConfig();
			if (info != null && mEM.ethConfigured()) {
				synchronized(this) {
					mInterfaceName = info.getIfName();

					if(LOCAL_LOG) Log.i(TAG, "reset device " + mInterfaceName);
					NetworkUtils.resetConnections(mInterfaceName,NetworkUtils.RESET_ALL_ADDRESSES);

					 // Stop DHCP
					if (mDhcpTarget != null) {
						mDhcpTarget.removeMessages(EVENT_DHCP_START);
					}
					//xelloss ,not need when not using dhcpcd service
					if (info.getConnectMode().equals(EthernetDevInfo.ETH_CONN_MODE_DHCP)){
					if (!NetworkUtils.stopDhcp(mInterfaceName))
					{
						Log.e(TAG, "Could not stop DHCP");
					}
					}


					configureInterface(info);
				}
			}
		}
		return true;
	}

	public void StartPolling() {
		if(LOCAL_LOG) Log.i(TAG, "start polling");
		mMonitor.startMonitoring();
	}

	public void sendEmptyMessage(int event)
	{
		mMYHandler.sendEmptyMessage(event);
	}

	public boolean isAvailable() {
		//Only say available if we have interfaces and user did not disable us.
		return ((mEM.getTotalInterface() != 0) && (mEM.getEthState() != EthernetManager.ETH_STATE_DISABLED));
	}

	public boolean reconnect() {

		try {
			synchronized (this) {
				if (mHWConnected && mStackConnected)
					return true;
			}
			if (isTeardownRequested()) {
				Log.i(TAG, "reconnect getEthState " + mEM.getEthState() );
				setTeardownRequested(false);
			if (mEM.getEthState() != EthernetManager.ETH_STATE_DISABLED ) {
				// maybe this is the first time we run, so set it to enabled
				mEM.setEthEnabled(true);
				if (!mEM.ethConfigured()) {
					mEM.ethSetDefaultConf();
				}
				return resetInterface();
			}
			}
		} catch (UnknownHostException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return false;

	}

	public boolean teardown() {

		if (!isTeardownRequested()) {
			setTeardownRequested(true);
			return (mEM != null) ? stopInterface(false) : false;
		}else {
			return true;
		}

	}

    @Override
    public void captivePortalCheckComplete() {
        // not implemented
    }

	public boolean setRadio(boolean turnOn) {
		// TODO Auto-generated method stub
		return false;
	}

	 @Override
	 public void setUserDataEnable(boolean enabled) {
		 Log.w(TAG, "ignoring setUserDataEnable(" + enabled + ")");
	 }

	 @Override
	 public void setPolicyDataEnable(boolean enabled) {
		 Log.w(TAG, "ignoring setPolicyDataEnable(" + enabled + ")");
	 }

	 /**
	  * Check if private DNS route is set for the network
	  */
	 public boolean isPrivateDnsRouteSet() {
		 return mPrivateDnsRouteSet.get();
	 }

	 /**
	  * Set a flag indicating private DNS route is set
	  */
	 public void privateDnsRouteSet(boolean enabled) {
		 mPrivateDnsRouteSet.set(enabled);
	 }

	 /**
	  * Fetch NetworkInfo for the network
	  */
	 public synchronized NetworkInfo getNetworkInfo() {
	 	if(mEthConnected)
			 return mNetworkWifiInfo;

		 return mNetworkInfo;
	 }

	 /**
	  * Fetch LinkProperties for the network
	  */
	 public synchronized LinkProperties getLinkProperties() {
		 return new LinkProperties(mLinkProperties);
	 }

	/**
	  * A capability is an Integer/String pair, the capabilities
	  * are defined in the class LinkSocket#Key.
	  *
	  * @return a copy of this connections capabilities, may be empty but never null.
	  */
	 public LinkCapabilities getLinkCapabilities() {
		 return new LinkCapabilities(mLinkCapabilities);
	 }

	 /**
	  * Fetch default gateway address for the network
	  */
	 public int getDefaultGatewayAddr() {
		 return mDefaultGatewayAddr.get();
	 }

	 /**
	  * Check if default route is set
	  */
	 public boolean isDefaultRouteSet() {
		 return mDefaultRouteSet.get();
	 }

	 /**
	  * Set a flag indicating default route is set for the network
	  */
	 public void defaultRouteSet(boolean enabled) {
		 mDefaultRouteSet.set(enabled);
	 }
	public int startUsingNetworkFeature(String feature, int callingPid,
			int callingUid) {
		// TODO Auto-generated method stub
		return 0;
	}

	public int stopUsingNetworkFeature(String feature, int callingPid,
			int callingUid) {
		// TODO Auto-generated method stub
		return 0;
	}

    public void setTeardownRequested(boolean isRequested) {
        mTeardownRequested.set(isRequested);
    }

    public boolean isTeardownRequested() {
        return mTeardownRequested.get();
    }

	public String getTcpBufferSizesPropName() {
		// TODO Auto-generated method stub
		return "net.tcp.buffersize.default";
	}

    public void setDependencyMet(boolean met) {
        // not supported on this network
    }

    @Override
    public void addStackedLink(LinkProperties link) {
        mLinkProperties.addStackedLink(link);
    }

    @Override
    public void removeStackedLink(LinkProperties link) {
        mLinkProperties.removeStackedLink(link);
    }

    @Override
    public void supplyMessenger(Messenger messenger) {
        // not supported on this network
    }
	
	public void startMonitoring(Context context, Handler target) {
		if(LOCAL_LOG) Log.i(TAG,"start to monitor the ethernet devices");
		if (mServiceStarted )	{
			mEM = (EthernetManager)mContext.getSystemService(Context.ETH_SERVICE);
			int state = mEM.getEthState();
			if (state != mEM.ETH_STATE_DISABLED) {
				if (state == mEM.ETH_STATE_UNKNOWN){
					// maybe this is the first time we run, so set it to enabled
					//mEM.setEthEnabled(true);
					mEM.setEthEnabled(false);//xelloss set it to be disabled
				} else {
					try {
						resetInterface();
					} catch (UnknownHostException e) {
						Log.e(TAG, "Wrong ethernet configuration");
					}
				}
			}
		}
	}



	private void setWifiVirtualState(NetworkInfo.DetailedState state)
	{

		if (mWifiManager == null) {
			mWifiManager =(WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);
		}

				if(state == DetailedState.CONNECTED){
			mEthConnected = true;
		}

		Intent intent = new Intent(WifiManager.VIRTUAL_STATE_CHANGED_ACTION);
		intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);

		if (mWifiManager != null) {
			//WifiInfo Winfo = new WifiInfo();
			WifiInfo Winfo = mWifiManager.getConnectionInfo();
			if (Winfo != null)
			{
				if(state == DetailedState.CONNECTED){
					mSupplicantState = Winfo.getSupplicantState();
					Winfo.setSupplicantState(SupplicantState.COMPLETED);

					mWifiMac = Winfo.getMacAddress();
					if(mWifiMac == null){
						try {
							InterfaceConfiguration config = mNwService.getInterfaceConfig(mInterfaceName);
							
							if(config != null)
							{
								String hwAddr;
								hwAddr = config.getHardwareAddress();
								Winfo.setMacAddress(hwAddr);
								if(LOCAL_LOG) Log.i(TAG, "Interface: " + mInterfaceName +"  hwAddr : " + hwAddr );
							}
							else
							{
								Winfo.setMacAddress("00:12:34:56:78:00");
								if(LOCAL_LOG) Log.i(TAG, "Wifi Mac Setting to ");
							}
						}
						catch (RemoteException e) {
				            Log.e(TAG, "Could not get list of interfaces " + e);
				        }
					}
				}
				else if(state == DetailedState.DISCONNECTED){
					if(mEthConnected == true){
						Winfo.setSupplicantState(mSupplicantState);
						Winfo.setMacAddress(mWifiMac);
					}
				}

				Winfo = mWifiManager.getConnectionInfo();
				if(LOCAL_LOG) Log.w(TAG, "setWifiVirtualState " + "WifiInfo info : " + Winfo + " state " + state );
			}
		}

            NetworkInfo Ninfo = new NetworkInfo(ConnectivityManager.TYPE_WIFI,0,"WIFI",null);
			mNetworkWifiInfo = Ninfo;
			if (Ninfo != null) {

				Ninfo.setDetailedState(state, null, null);

				if(state == DetailedState.CONNECTED){
					Ninfo.setIsAvailable(true);
				}
				else if(state == DetailedState.DISCONNECTED){
					Ninfo.setIsAvailable(false);
				}

				intent.putExtra(WifiManager.EXTRA_NETWORK_INFO, Ninfo);

				if(LOCAL_LOG) Log.w(TAG, "setWifiVirtualState " + "Send NetworkInfo: " + Ninfo + " state " + state);

			}


		if(state == DetailedState.CONNECTED){
			mContext.sendStickyBroadcast(intent);
	}
		if(state == DetailedState.DISCONNECTED && mEthConnected == true)
		{
			mContext.sendStickyBroadcast(intent);
			mEthConnected = false;
		}
	}

	private void postNotification(int event) {
		String ns = Context.NOTIFICATION_SERVICE;
		if(LOCAL_LOG) Log.i(TAG, "post event event "+event);
		mNotificationManager = (NotificationManager)mContext.getSystemService(ns);
		final Intent intent = new Intent(EthernetManager.ETH_STATE_CHANGED_ACTION);
		intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
		//intent.putExtra(EthernetManager.EXTRA_ETH_EVENT, event);
		//intent.putExtra(EthernetManager.EXTRA_ETH_STATE, mEM.getEthState());
		//mContext.sendStickyBroadcast(intent);

		//xelloss, send proper intent. this is for dhcp ?
		mEM.setCheckConnecting(0);
		if(event == EVENT_INTERFACE_CONFIGURATION_SUCCEEDED || event == EVENT_HW_CONNECTED)
		{
			intent.putExtra(EthernetManager.EXTRA_ETH_EVENT, event);
			intent.putExtra(EthernetManager.EXTRA_ETH_STATE, EthernetManager.ETH_STATE_ENABLED);
			mContext.sendStickyBroadcast(intent);

			String resultprop="dhcp."+mInterfaceName+".result";

			SystemProperties.set(resultprop, "ok");

			mNetworkInfo.setIsAvailable(true);
			setWifiVirtualState(DetailedState.CONNECTED);
		}
		else if(event ==EVENT_INTERFACE_CONFIGURATION_FAILED || event ==EVENT_HW_DISCONNECTED)
		{
			intent.putExtra(EthernetManager.EXTRA_ETH_EVENT, event);
			intent.putExtra(EthernetManager.EXTRA_ETH_STATE, EthernetManager.ETH_STATE_DISABLED);
			mContext.sendStickyBroadcast(intent);


			String resultprop="dhcp."+mInterfaceName+".result";

			SystemProperties.set(resultprop, "failed");

			mNetworkInfo.setIsAvailable(false);
			setWifiVirtualState(DetailedState.DISCONNECTED);
		}

		if (false) {
			if (mNotificationManager != null) {
				int icon;
				CharSequence title = "Ethernet Status";
				CharSequence detail;
				if(mNotification == null) {
					mNotification = new Notification();
					mNotification.contentIntent = PendingIntent.getActivity(mContext, 0,
							new Intent(EthernetManager.NETWORK_STATE_CHANGED_ACTION), 0);
				}
				mNotification.when = System.currentTimeMillis();

				switch (event) {
					case EVENT_HW_CONNECTED:
					case EVENT_INTERFACE_CONFIGURATION_SUCCEEDED:
						//mNotification.icon=icon = com.android.internal.R.drawable.connect_established;
						String ipprop="dhcp."+mInterfaceName+".ipaddress";
						String ipaddr = SystemProperties.get(ipprop);
						detail = "Ethernet is connected. IP address: " + ipaddr ;
						break;
					case EVENT_HW_DISCONNECTED:
					case EVENT_INTERFACE_CONFIGURATION_FAILED:
						//mNotification.icon = icon = com.android.internal.R.drawable.connect_no;
						detail = "Ethernet is disconnected.";
						break;
					default:
						//mNotification.icon=icon = com.android.internal.R.drawable.connect_creating;
						detail = "Unknown event with Ethernet.";
				}
				if(LOCAL_LOG) Log.i(TAG, "post event to notification manager "+detail);
				mNotification.setLatestEventInfo(mContext, title, detail, mNotification.contentIntent );
				mNotificationManager.notify(10010, mNotification);
				//Message message = mTarget.obtainMessage(EVENT_NOTIFICATION_CHANGED,mNotification);
				//mTarget.sendMessageDelayed(message, 0);
			} else {
				Log.i(TAG, "notification manager is not up yet");
			}
		}
	}

    private class MyHandler extends Handler {
        public MyHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
		synchronized (mMYHandler) {
			switch (msg.what) {
			case EVENT_INTERFACE_CONFIGURATION_SUCCEEDED:
				if(LOCAL_LOG) Log.i(TAG, "received configured events, stack: " + mStackConnected + " HW: " + mHWConnected);
				mStackConnected = true;
				mEM.setStackConnected(mStackConnected);
				if (mHWConnected) {
					mNetworkInfo.setDetailedState(DetailedState.CONNECTED,null,null);

					mLinkProperties = mDhcpResults.linkProperties;
					mLinkProperties.setInterfaceName(mInterfaceName);
					postNotification(msg.what);
					//mTarget.sendEmptyMessage(EVENT_CONFIGURATION_CHANGED);
					Message lmsg = mTarget.obtainMessage(EVENT_STATE_CHANGED, mNetworkInfo);
          			lmsg.sendToTarget();
				}
				else{
					Log.e(TAG, "Stack connection OK but HW not connected @@@" +mHWConnected);
					mEM.setCheckConnecting(0);// free connecting status
					//postNotification(EVENT_HW_DISCONNECTED);
					//stopInterface(true);
				}
				break;

			case EVENT_INTERFACE_CONFIGURATION_FAILED:
				if(LOCAL_LOG) Log.i(TAG, "received configured EVENT_INTERFACE_CONFIGURATION_FAILED");
				mStackConnected = false;
				mEM.setStackConnected(mStackConnected);

				//connection_modification
				//fail to try connecting but don't set disable.
				//mEM.setEthEnabled(false);
				postNotification(msg.what);//xelloss
				//start to retry ?
				break;
			case EVENT_HW_CONNECTED:
				if(LOCAL_LOG) Log.i(TAG, "received connected events, stack: " + mStackConnected + " HW: " + mHWConnected);
				mHWConnected = true;
				mEM.setHWConnected(mHWConnected);
				if (mStackConnected) {
					mNetworkInfo.setDetailedState(DetailedState.CONNECTED,null,null);
					postNotification(msg.what);
					//mTarget.sendEmptyMessage(EVENT_CONFIGURATION_CHANGED);
					Message lmsg = mTarget.obtainMessage(EVENT_STATE_CHANGED, mNetworkInfo);
          			lmsg.sendToTarget();
				}
				break;
			case EVENT_HW_DISCONNECTED:
				if(LOCAL_LOG) Log.i(TAG, "received disconnected events, stack: " + mStackConnected + " HW: " + mHWConnected );
				if (mStackConnected && mHWConnected ) {//xellos send event when stack connectd , for reconnect from dchp for 9300 gmac
					postNotification(msg.what);
				}
				mHWConnected = false;
				mPHYup = false;
				mEM.setHWConnected(mHWConnected);
				mNetworkInfo.setDetailedState(DetailedState.DISCONNECTED,null,null);
				if (mStackConnected) {
				stopInterface(true);
				}
				Message lmsg = mTarget.obtainMessage(EVENT_STATE_CHANGED, mNetworkInfo);
      			lmsg.sendToTarget();
				//postNotification(msg.what);
				break;
			case EVENT_HW_PHYCONNECTED:
				mPHYup = true;
				if(LOCAL_LOG) Log.i(TAG, "interface up event, kick off connection request mHWConnected" +mHWConnected);
				if (!mStartingDhcp) {
					int state = mEM.getEthState();
					if (state != mEM.ETH_STATE_DISABLED) {
						EthernetDevInfo info = mEM.getSavedEthConfig();
						if (info != null && mEM.ethConfigured()) {
							try {
								configureInterface(info);
							} catch (UnknownHostException e) {
								 // TODO Auto-generated catch block
								 e.printStackTrace();
							}
						}
					}
				}
				//set the value here even if it was already set
				mHWConnected = true;
				mEM.setHWConnected(mHWConnected);
				break;
			}
		}
	}
    }

	private class DhcpHandler extends Handler {

		 public DhcpHandler(Looper looper, Handler target) {
				super(looper);
			}

		 public void handleMessage(Message msg) {
			 int event;

			 switch (msg.what) {
			 case EVENT_DHCP_START:
				 synchronized (mDhcpTarget) {
					 if (!mInterfaceStopped) {
						mEM.setCheckConnecting(1);//xelloss set to begin connecting
						 if(LOCAL_LOG) Log.d(TAG, "DhcpHandler: DHCP request started");
						 mPHYup = true;

						 if (!mPHYup){
						 	event = EVENT_INTERFACE_CONFIGURATION_FAILED;
						 	if(LOCAL_LOG) Log.i(TAG, "DhcpHandler: no Phy connect");
						}
						 else if (NetworkUtils.runDhcp(mInterfaceName, mDhcpResults)) {
							 event = EVENT_INTERFACE_CONFIGURATION_SUCCEEDED;
							 if(LOCAL_LOG) Log.i(TAG, "DhcpHandler: DHCP request succeeded");
						 } else {
							 event = EVENT_INTERFACE_CONFIGURATION_FAILED;
							 if(LOCAL_LOG) Log.i(TAG, "DhcpHandler: DHCP request failed: " +
									 NetworkUtils.getDhcpError());
						 }
						 mMYHandler.sendEmptyMessage(event);
					 } else {
						 mInterfaceStopped = false;
					 }
					 mStartingDhcp = false;
				 }
				 break;
			 }

		 }
	}

	public void notifyPhyConnected(String ifname) {
		if(LOCAL_LOG) Log.i(TAG, "report interface is up HWConnected " + mHWConnected+ " getCheckConnecting " + mEM.getCheckConnecting());

		//this call means that phy(hw) is connected, so set the value here 
		mHWConnected = true;
		mEM.setHWConnected(mHWConnected);

		//don't send phy up event when DHCP is acting to prevent from reconnecting.
		if(mEM.getCheckConnecting() == 1 )
			return;

		synchronized(this) {
			mMYHandler.sendEmptyMessage(EVENT_HW_PHYCONNECTED);
		}

	}
	public void notifyStateChange(String ifname,DetailedState state) {
		if(LOCAL_LOG) Log.i(TAG, "report new state " + state.toString() + " on dev " + ifname);
		if (ifname.equals(mInterfaceName)) {
			if(LOCAL_LOG) Log.i(TAG, "update network state tracker");
			synchronized(this) {
				mMYHandler.sendEmptyMessage(state.equals(DetailedState.CONNECTED)
					? EVENT_HW_CONNECTED : EVENT_HW_DISCONNECTED);
			}
		}
	}
}
