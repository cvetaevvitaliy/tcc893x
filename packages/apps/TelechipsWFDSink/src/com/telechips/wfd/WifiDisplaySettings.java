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

import java.io.File;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Fragment;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.hardware.display.DisplayManager;
import android.hardware.display.WifiDisplay;
import android.hardware.display.WifiDisplayStatus;
import android.net.NetworkInfo;
import android.net.Uri;
import android.net.wifi.p2p.WifiP2pDevice;
import android.net.wifi.p2p.WifiP2pManager;
import android.net.wifi.p2p.WifiP2pWfdInfo;
import android.os.Bundle;
import android.os.Handler;
import android.os.SystemProperties;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceFragment;
import android.preference.PreferenceGroup;
import android.preference.PreferenceScreen;
import android.provider.Settings;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.ImageView;
import android.widget.Switch;
import android.widget.TextView;
import com.telechips.wfd.ProgressCategory;
import android.util.Log;
import android.net.wifi.p2p.WifiP2pGroup;
import android.net.wifi.p2p.WifiP2pGroupList;
import android.net.wifi.p2p.WifiP2pManager.GroupInfoListener;
import android.net.wifi.p2p.WifiP2pManager.PeerListListener;
import android.net.wifi.p2p.WifiP2pManager.PersistentGroupInfoListener;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.app.Dialog;

/**
 * The Settings screen for WifiDisplay configuration and connection management.
 */
public final class WifiDisplaySettings extends PreferenceFragment 
        implements PersistentGroupInfoListener {
    private static final String TAG = "WifiDisplaySettings";

    //private static final int MENU_ID_SCAN = Menu.FIRST;
    private static final int MENU_ID_INFORMATION = Menu.FIRST+1;
    private static final int MENU_ID_CREATE_GROUP = Menu.FIRST + 2;
    private static final int MENU_ID_REMOVE_GROUP = Menu.FIRST + 3;
    private static final int MENU_ID_RESET = Menu.FIRST + 4;
    private static final int MENU_ID_WPS_PBC = Menu.FIRST + 5;
    private static final int MENU_ID_SETTINGS = Menu.FIRST + 6;
    
    public DisplayManager mDisplayManager;

    private boolean mWifiDisplayOnSetting;
    public WifiDisplayStatus mWifiDisplayStatus;

    private PreferenceGroup mPairedDevicesCategory;
    private ProgressCategory mAvailableDevicesCategory;
    private PreferenceGroup mPersistentGroup;
    private WifiDisplayPersistentGroup mSelectedGroup;
    private OnClickListener mDeleteGroupListener;

    private TextView mEmptyView;

    //private Switch mActionBarSwitch;
    
    private WifiP2pDevice mThisDevice;
	private int mConnectedDeviceRtspPort;
	private static WifiDisplaySettings instance;
	public boolean bCreateGroup = false;

    private static final int DIALOG_DELETE_GROUP = 4;

    private WifiP2pGroupList mPersistentGroups;
    
    public WifiDisplaySettings() {
    }

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        Log.d(TAG,"onCreate");
        mDisplayManager = (DisplayManager)getActivity().getSystemService(Context.DISPLAY_SERVICE);
        
        addPreferencesFromResource(R.xml.wifi_display_settings);
        setHasOptionsMenu(true);
        instance = this;
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        //Log.d(TAG,"onActivityCreated");
        Activity activity = getActivity();
        //mActionBarSwitch = new Switch(activity);
        if (activity instanceof PreferenceActivity) {
            PreferenceActivity preferenceActivity = (PreferenceActivity) activity;
            /*
            if (preferenceActivity.onIsHidingHeaders() || !preferenceActivity.onIsMultiPane()) {
                final int padding = activity.getResources().getDimensionPixelSize(
                        R.dimen.action_bar_switch_padding);
                mActionBarSwitch.setPadding(0, 0, padding, 0);
                activity.getActionBar().setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM,
                        ActionBar.DISPLAY_SHOW_CUSTOM);
                activity.getActionBar().setCustomView(mActionBarSwitch,
                        new ActionBar.LayoutParams(
                                ActionBar.LayoutParams.WRAP_CONTENT,
                                ActionBar.LayoutParams.WRAP_CONTENT,
                                Gravity.CENTER_VERTICAL | Gravity.END));
            }
            */
        }

        //delete persistent group dialog listener
        mDeleteGroupListener = new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                if (which == DialogInterface.BUTTON_POSITIVE) {
                    if (TelechipsWFDSink.getInstance().mWifiP2pManager != null) {
                    	TelechipsWFDSink.getInstance().mWifiP2pManager.deletePersistentGroup(TelechipsWFDSink.getInstance().mChannel,
                                mSelectedGroup.getNetworkId(),
                                new WifiP2pManager.ActionListener() {
                            public void onSuccess() {
                                Log.d(TAG, " delete group success");
                            }
                            public void onFailure(int reason) {
                                Log.d(TAG, " delete group fail " + reason);
                            }
                        });
                    }
                }
            }
        }; 

        mEmptyView = (TextView) getView().findViewById(android.R.id.empty);
        getListView().setEmptyView(mEmptyView);

        if( SystemProperties.get("tcc.wifi.p2p.persistent").equals("1")) {
        	mPersistentGroup = new PreferenceCategory(getActivity());
        	mPersistentGroup.setTitle("Paired Devices");
        }

        update();

        if (mWifiDisplayStatus.getFeatureState() == WifiDisplayStatus.FEATURE_STATE_UNAVAILABLE) {
        	Log.e(TAG,"WifiDisplayStatus.FEATURE_STATE_UNAVAILABLE");
            activity.finish();
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.d(TAG,"onResume");
        
        if(WFDPlay.getWFDPlayStatus() == WFDPlay.WFDPLAY_STATUS_SHUTDOWN) {
        	return;
        }	
        getActivity().getContentResolver().registerContentObserver(Settings.Secure.getUriFor(
                Settings.Global.WIFI_DISPLAY_ON), false, mSettingsObserver);

        Context context = getActivity();
        IntentFilter filter = new IntentFilter();
        filter.addAction(DisplayManager.ACTION_WIFI_DISPLAY_STATUS_CHANGED);
        filter.addAction(WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION);
        filter.addAction(WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION);
        filter.addAction(WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION);
        filter.addAction(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION);
        filter.addAction(WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION);
        filter.addAction(WifiP2pManager.WIFI_P2P_DISCOVERY_CHANGED_ACTION);
        filter.addAction(WifiP2pManager.WIFI_P2P_PERSISTENT_GROUPS_CHANGED_ACTION);
        context.registerReceiver(mReceiver, filter);
        
        update();
    }

    void unregisterReceiver() {
		try{
			getInstance().getActivity().unregisterReceiver(mReceiver);
		}catch(IllegalArgumentException ex){
			
		}
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(DisplayManager.ACTION_WIFI_DISPLAY_STATUS_CHANGED)) {
                //Log.d(TAG,"				ACTION_WIFI_DISPLAY_STATUS_CHANGED");
                
                WifiDisplayStatus status = (WifiDisplayStatus)intent.getParcelableExtra(
                        DisplayManager.EXTRA_WIFI_DISPLAY_STATUS);
                mWifiDisplayStatus = status;
                //Log.d(TAG,"status = " + status);
                applyState();
                
            } else  if (WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION.equals(action)) {
            	Log.d(TAG,"				WIFI_P2P_THIS_DEVICE_CHANGED_ACTION");
                mThisDevice = (WifiP2pDevice) intent.getParcelableExtra(
                        WifiP2pManager.EXTRA_WIFI_P2P_DEVICE);
                //mDisplayManager.scanWifiDisplays();
                
            }else if (WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION.equals(action)) {
            	Log.d(TAG,"				WIFI_P2P_STATE_CHANGED_ACTION");
            	TelechipsWFDSink.getInstance().mWifiP2pEnabled = intent.getIntExtra(WifiP2pManager.EXTRA_WIFI_STATE,
                        WifiP2pManager.WIFI_P2P_STATE_DISABLED) == WifiP2pManager.WIFI_P2P_STATE_ENABLED;
            	//Log.d(TAG,"mWifiP2pEnabled = " + TelechipsWFDSink.getInstance().mWifiP2pEnabled );
                if(TelechipsWFDSink.getInstance().mWifiP2pEnabled==true) {	
                	//Log.d(TAG,"scanWifiDisplays");
                	//mDisplayManager.scanWifiDisplays();
                }
            } else if (WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION.equals(action)) {
            	//Log.d(TAG,"				WIFI_P2P_PEERS_CHANGED_ACTION");
                if (TelechipsWFDSink.getInstance().mWifiP2pManager != null) {
                  	TelechipsWFDSink.getInstance().mWifiP2pManager.requestPeers(TelechipsWFDSink.getInstance().mChannel, TelechipsWFDSink.getInstance());
                }
                
            } else if (WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION.equals(action)) {
            	Log.d(TAG,"				WIFI_P2P_CONNECTION_CHANGED_ACTION");
                if (TelechipsWFDSink.getInstance().mWifiP2pManager == null) {
                	return;
                }
                NetworkInfo networkInfo = (NetworkInfo) intent.getParcelableExtra(WifiP2pManager.EXTRA_NETWORK_INFO);
                if (TelechipsWFDSink.getInstance().mWifiP2pManager != null) {
                   	TelechipsWFDSink.getInstance().mWifiP2pManager.requestGroupInfo(TelechipsWFDSink.getInstance().mChannel, TelechipsWFDSink.getInstance());
                }
                if (networkInfo.isConnected()) {
	                	//if(WFDPlay.mWFDPlayError==false) {
                		if(bCreateGroup==true) {
                			Log.e(TAG, "-------bCreateGroup is true-------------- ");
                			bCreateGroup = false;
                			return;
                		}
                		if(WFDPlay.getWFDPlayStatus() == WFDPlay.WFDPLAY_STATUS_NONE) {
		                    Log.d(TAG, "---------------------- Connected");
		                    //TelechipsWFDSink.getInstance().sendMessage(TelechipsWFDSink.getInstance().MESSAGE_CONNECTION_DIALOG_DISMISS);
		                    TelechipsWFDSink.getInstance().mWifiP2pManager.requestConnectionInfo(TelechipsWFDSink.getInstance().mChannel, TelechipsWFDSink.getInstance());
                		}
                } else {
                	Log.d(TAG, "---------------------- Disonnected");
                    TelechipsWFDSink.getInstance().sendMessage(TelechipsWFDSink.getInstance().MESSAGE_CONNECTION_DIALOG_DISMISS);
                }
            } else if (WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION.equals(action)) {
            	//Log.d(TAG,"				WIFI_P2P_THIS_DEVICE_CHANGED_ACTION");
                mThisDevice = (WifiP2pDevice) intent.getParcelableExtra(WifiP2pManager.EXTRA_WIFI_P2P_DEVICE);
                //Log.d(TAG, "Update device info: " + mThisDevice);
                //updateDevicePref();
            } else if (WifiP2pManager.WIFI_P2P_PERSISTENT_GROUPS_CHANGED_ACTION.equals(action)) {
                if (TelechipsWFDSink.getInstance().mWifiP2pManager != null) {
                	TelechipsWFDSink.getInstance().mWifiP2pManager.requestPersistentGroupInfo(TelechipsWFDSink.getInstance().mChannel, WifiDisplaySettings.this);
                }
            }
        }
    };
    
    @Override
    public void onPause() {
        super.onPause();

        Context context = getActivity();
		try{
			context.unregisterReceiver(mReceiver);
		}catch(IllegalArgumentException ex){
			
		}
        
        getActivity().getContentResolver().unregisterContentObserver(mSettingsObserver);
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        /*
        MenuItem item = menu.add(Menu.NONE, MENU_ID_SCAN, 0,
                mWifiDisplayStatus.getScanState() == WifiDisplayStatus.SCAN_STATE_SCANNING ?
                        R.string.wifi_display_searching_for_devices :
                                R.string.wifi_display_search_for_devices);
        item.setIcon(R.drawable.search);
        item.setEnabled(mWifiDisplayStatus.getFeatureState() == WifiDisplayStatus.FEATURE_STATE_ON
                && mWifiDisplayStatus.getScanState() == WifiDisplayStatus.SCAN_STATE_NOT_SCANNING);
        item.setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS|MenuItem.SHOW_AS_ACTION_WITH_TEXT);
        */
        if(SystemProperties.get("tcc.wifi.display.miracast").equals("1")) 
        {
	        menu.add(Menu.NONE, MENU_ID_CREATE_GROUP, 0, "Create group")
	        	.setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
	        menu.add(Menu.NONE, MENU_ID_REMOVE_GROUP, 0, "Remove group")
	        	.setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
	        menu.add(Menu.NONE, MENU_ID_WPS_PBC, 0, "WPS_PBC")
	    		.setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
        }
        menu.add(Menu.NONE, MENU_ID_RESET, 0, "RESET")
    	.setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
        
        /*
        menu.add(Menu.NONE, MENU_ID_INFORMATION, 0, "Info")
        	.setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
        */
        
        if(SystemProperties.get("ro.build.type").equals("eng")) {
        	menu.add(Menu.NONE, MENU_ID_SETTINGS, 0, "Settings")
        	.setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
        }

        super.onCreateOptionsMenu(menu, inflater);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            /*
            case MENU_ID_SCAN:
                if (mWifiDisplayStatus.getFeatureState() == WifiDisplayStatus.FEATURE_STATE_ON) {
                	Log.d(TAG,"scanWifiDisplays");
                    mDisplayManager.startWifiDisplayScan();
                }
                return true;
            */           
            case MENU_ID_RESET:
            	AlertDialog.Builder alertDlg = new AlertDialog.Builder(getActivity());
    			alertDlg.setTitle("Reset WFDSink");
    			alertDlg.setMessage("Remove paired group and reset Wi-Fi\nAre you sure?");
    			alertDlg.setPositiveButton("ok",
    					new DialogInterface.OnClickListener() {
    						public void onClick(DialogInterface dialog,	int whichButton) {
    							
    							if( SystemProperties.get("tcc.wifi.p2p.persistent").equals("1")) {
	    					        for (WifiP2pGroup group: mPersistentGroups.getGroupList()) {
	    					        	if (TelechipsWFDSink.getInstance().mWifiP2pManager != null) {
	    			                    	TelechipsWFDSink.getInstance().mWifiP2pManager.deletePersistentGroup(TelechipsWFDSink.getInstance().mChannel,
	    			                    			group.getNetworkId(),
	    			                                new WifiP2pManager.ActionListener() {
	    			                            public void onSuccess() {
	    			                                Log.d(TAG, " delete group success");
	    			                            }
	    			                            public void onFailure(int reason) {
	    			                                Log.d(TAG, " delete group fail " + reason);
	    			                            }
	    			                        });
	    			                    }
	    					        }
    							}
    					        
    							//TelechipsWFDSink.getInstance().startP2p("Reset","waitting for initialization",true);
    							TelechipsWFDSink.getInstance().sendMessage(TelechipsWFDSink.getInstance().MESSAGE_RESET_P2P);
    							
    							
    						}
    					});
    			alertDlg.setNegativeButton("cancel",
    					new DialogInterface.OnClickListener() {
    						public void onClick(DialogInterface dialog, int whichButton) {
    						}
    					});
    			alertDlg.show();   
                return true;

            case MENU_ID_INFORMATION:
            	String strInfo=TelechipsWFDSink.getVersionName(TelechipsWFDSink.getInstance());
            	/*
            	strInfo+="\n\n";
            	if(TelechipsWFDSink.getInstance().mDebug==true)
            		strInfo += "Enabled debug mode\n";
            	if( SystemProperties.get("tcc.wifi.p2p.persistent").equals("1"))
            		strInfo += "Enabled Persistent GO\n";
            	if( SystemProperties.get("tcc.wifi.p2p.auto_pbc").equals("1"))
            		strInfo+="Enabled Automatic PBC\n";
            	if( SystemProperties.get("tcc.wfd.filedump").equals("1"))
            		strInfo+="Enabled TS dump\n";
            	*/
            	AlertDialog.Builder alertDlg2 = new AlertDialog.Builder(getActivity());
    			alertDlg2.setTitle("Version");
    			alertDlg2.setMessage(strInfo);
    			alertDlg2.setPositiveButton("ok",
    					new DialogInterface.OnClickListener() {
    						public void onClick(DialogInterface dialog,
    								int whichButton) {
    						}
    					});
    			alertDlg2.show();            
    			return true;
            case MENU_ID_CREATE_GROUP:
                Log.d(TAG, "MENU_ID_CREATE_GROUP");
                if (TelechipsWFDSink.getInstance().mWifiP2pManager != null) {
                	TelechipsWFDSink.getInstance().mWifiP2pManager.createGroup(TelechipsWFDSink.getInstance().mChannel, new WifiP2pManager.ActionListener() {
	                    public void onSuccess() {
	                    	Log.d(TAG, " create group success");
	                    	bCreateGroup = true;
	                    	//TelechipsWFDSink.getInstance().stopP2PDispeerTimeoutThread();
	                    	WifiDisplaySettings.getInstance().mDisplayManager.stopWifiDisplayScan();
	                                //mWifiP2pManager.requestWpsPbc(mChannel);
	                    }
	                    public void onFailure(int reason) {
	                    	Log.e(TAG, " create group fail " + reason);
	                    }
                	});
                }
                return true;
		case MENU_ID_REMOVE_GROUP:
			if (TelechipsWFDSink.getInstance().mWifiP2pManager != null) {
				TelechipsWFDSink.getInstance().mWifiP2pManager.removeGroup(TelechipsWFDSink.getInstance().mChannel,	new WifiP2pManager.ActionListener() {
					public void onSuccess() {
						Log.d(TAG, " remove group success");
						bCreateGroup = false;
					}
					public void onFailure(int reason) {
						Log.d(TAG, " remove group fail " + reason);						}
				});
			}
			return true;
        case MENU_ID_WPS_PBC:
            if (TelechipsWFDSink.getInstance().mWifiP2pManager != null) {
            	TelechipsWFDSink.getInstance().mWifiP2pManager.requestWpsPbc(TelechipsWFDSink.getInstance().mChannel);
            }
            return true;
        case MENU_ID_SETTINGS:
            if (getActivity() instanceof PreferenceActivity) {
            	/*
                ((PreferenceActivity) getActivity()).startPreferencePanel(
                        WirelessDisplayAdvancedSettings.class.getCanonicalName(),
                        null,
                        R.string.wireless_display_setting_titlebar, null,
                        this, 0);
                */
            	TelechipsWFDSink.getInstance().intentAdvancedSettings = new Intent(TelechipsWFDSink.getInstance(), WifiDisplayAdvancedSettings.class);
                startActivityForResult(TelechipsWFDSink.getInstance().intentAdvancedSettings, 0);
            } else {
        		//Intent intent = new Intent(TelechipsWFDSink.getInstance(), WirelessDisplayAdvancedSettings.class);
                //startActivityForResult(intent, 0);
                
                //startFragment(this, WirelessDisplayAdvancedSettings.class.getCanonicalName(), -1, null);
            }
        	
        	return true;
        }
        return super.onOptionsItemSelected(item);
    }

    public boolean startFragment(
            Fragment caller, String fragmentClass, int requestCode, Bundle extras) {
        if (getActivity() instanceof PreferenceActivity) {
            PreferenceActivity preferenceActivity = (PreferenceActivity)getActivity();
            preferenceActivity.startPreferencePanel(fragmentClass, extras,
                    R.string.lock_settings_picker_title, null, caller, requestCode);
            return true;
        } else {
            Log.w(TAG, "Parent isn't PreferenceActivity, thus there's no way to launch the "
                    + "given Fragment (name: " + fragmentClass + ", requestCode: " + requestCode
                    + ")");
            return false;
        }
    }
    
    
    
    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {
        Log.d(TAG,"onPreferenceTreeClick");
		if (SystemProperties.get(WifiDisplayAdvancedSettings.PROPERTY_MIRACAST_CERT).equals("1")) {
			if (preference instanceof WifiDisplayPreference) {
				WifiDisplayPreference p = (WifiDisplayPreference) preference;
				WifiDisplay display = p.getDisplay();

				if (display.equals(mWifiDisplayStatus.getActiveDisplay())) {
					showDisconnectDialog(display);
				} else {
					Log.d(TAG,
							"connectWifiDisplay port = "
									+ display.getRtspPort());
					mDisplayManager.connectWifiDisplay(display.getDeviceAddress());
					setConnectedDeviceRtspPort(display.getRtspPort());
				}
			} else if (preference instanceof WifiDisplayPersistentGroup) {
				mSelectedGroup = (WifiDisplayPersistentGroup) preference;
				onCreateDialog(DIALOG_DELETE_GROUP);
			}
		}
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    public Dialog onCreateDialog(int id) {
        if (id == DIALOG_DELETE_GROUP) {
        	Log.d(TAG,"onCreateDialog: DIALOG_DELETE_GROUP");
            AlertDialog dialog = new AlertDialog.Builder(getActivity())
                .setMessage("Forget this group?")
                .setPositiveButton("ok", mDeleteGroupListener)
                .setNegativeButton("cancel", null)
                .create();
            dialog.show();
            return dialog;
        }
        return null;
    }

    public int getConnectedDeviceRtspPort(){
    	return mConnectedDeviceRtspPort;
    }
    
    public void setConnectedDeviceRtspPort(int port){
    	mConnectedDeviceRtspPort = port;
    }
    
	public static final WifiDisplaySettings getInstance() {
		if (instance != null)
			return instance;
		return null;
	}
	
    private void update() {
        mWifiDisplayOnSetting = Settings.Global.getInt(getActivity().getContentResolver(),
                Settings.Global.WIFI_DISPLAY_ON, 0) != 0;
        mWifiDisplayStatus = mDisplayManager.getWifiDisplayStatus();
        //Log.d(TAG,"update: mWifiDisplayOnSetting ="+mWifiDisplayOnSetting);
        //Log.d(TAG,"update: mWifiDisplayStatus ="+mWifiDisplayStatus);

        applyState();
    }

    private void applyState() {
        //Log.d(TAG,"applyState");
        
        final int featureState = mWifiDisplayStatus.getFeatureState();
        //Log.d(TAG,"featureState = " + featureState);
        //mActionBarSwitch.setEnabled(featureState != WifiDisplayStatus.FEATURE_STATE_DISABLED);
        //mActionBarSwitch.setChecked(mWifiDisplayOnSetting);

        final PreferenceScreen preferenceScreen = getPreferenceScreen();
        preferenceScreen.removeAll();

        if (featureState == WifiDisplayStatus.FEATURE_STATE_ON) {
            //final WifiDisplay[] pairedDisplays = mWifiDisplayStatus.getRememberedDisplays();
            final WifiDisplay[] availableDisplays = mWifiDisplayStatus.getDisplays();

            if (mPairedDevicesCategory == null) {
                mPairedDevicesCategory = new PreferenceCategory(getActivity());
                mPairedDevicesCategory.setTitle(R.string.wifi_display_paired_devices);
            } else {
                mPairedDevicesCategory.removeAll();
            }
            preferenceScreen.addPreference(mPairedDevicesCategory);

            //for (WifiDisplay d : pairedDisplays) {
            //    mPairedDevicesCategory.addPreference(createWifiDisplayPreference(d, true));
            //}
            if (mPairedDevicesCategory.getPreferenceCount() == 0) {
                preferenceScreen.removePreference(mPairedDevicesCategory);
            }

            if (mAvailableDevicesCategory == null) {
                mAvailableDevicesCategory = new ProgressCategory(getActivity(), null,
                        R.string.wifi_display_no_devices_found);
                mAvailableDevicesCategory.setTitle(R.string.wifi_display_available_devices);
            } else {
                mAvailableDevicesCategory.removeAll();
            }
            preferenceScreen.addPreference(mAvailableDevicesCategory);

            for (WifiDisplay d : availableDisplays) {
                	//Log.d(TAG,"#### add device type:" +d.getDeviceType() + "deviceAddress: " + d.getDeviceAddress() + " rtsp port: " + d.getRtspPort());
                	if( (d.getDeviceType() == WifiP2pWfdInfo.WFD_SOURCE) || (d.getDeviceType() == WifiP2pWfdInfo.SOURCE_OR_PRIMARY_SINK))
                		mAvailableDevicesCategory.addPreference(createWifiDisplayPreference(d, false));
            }
            if (mWifiDisplayStatus.getScanState() == WifiDisplayStatus.SCAN_STATE_SCANNING) {
                mAvailableDevicesCategory.setProgress(true);
            } else {
                mAvailableDevicesCategory.setProgress(false);
            }

            if( SystemProperties.get("tcc.wifi.p2p.persistent").equals("1")) {
            	mPersistentGroup.setEnabled(true);
            	preferenceScreen.addPreference(mPersistentGroup);
            }

        } else {
            mEmptyView.setText(featureState == WifiDisplayStatus.FEATURE_STATE_OFF ?
                    R.string.wifi_display_settings_empty_list_wifi_display_off :
                            R.string.wifi_display_settings_empty_list_wifi_display_disabled);
        }

        getActivity().invalidateOptionsMenu();
    }

    private Preference createWifiDisplayPreference(final WifiDisplay d, boolean paired) {
        //Log.d(TAG,"createWifiDisplayPreference");
        
        WifiDisplayPreference p = new WifiDisplayPreference(getActivity(), d);
        if (d.equals(mWifiDisplayStatus.getActiveDisplay())) {
            switch (mWifiDisplayStatus.getActiveDisplayState()) {
                case WifiDisplayStatus.DISPLAY_STATE_CONNECTED:
                    p.setSummary(R.string.wifi_display_status_connected);
                    break;
                case WifiDisplayStatus.DISPLAY_STATE_CONNECTING:
                    p.setSummary(R.string.wifi_display_status_connecting);
                    break;
            }
        } else if (paired && contains(mWifiDisplayStatus.getDisplays(),
                d.getDeviceAddress())) {
            p.setSummary(R.string.wifi_display_status_available);
        }
        if (paired) {
            p.setWidgetLayoutResource(R.layout.wifi_display_preference);
        }
        return p;
    }

    private void showDisconnectDialog(final WifiDisplay display) {
        DialogInterface.OnClickListener ok = new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                if (display.equals(mWifiDisplayStatus.getActiveDisplay())) {
                    mDisplayManager.disconnectWifiDisplay();
                }
            }
        };

        AlertDialog dialog = new AlertDialog.Builder(getActivity())
                .setCancelable(true)
                .setTitle(R.string.wifi_display_disconnect_title)
                .setMessage("Disconnect?")
                .setPositiveButton(android.R.string.ok, ok)
                .setNegativeButton(android.R.string.cancel, null)
                .create();
        dialog.show();
    }

    private void showOptionsDialog(final WifiDisplay display) {
       
    }

    private static boolean contains(WifiDisplay[] displays, String address) {
        //Log.d(TAG,"contains");
        
        for (WifiDisplay d : displays) {
            if (d.getDeviceAddress().equals(address)) {
                return true;
            }
        }
        return false;
    }

    private final ContentObserver mSettingsObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange, Uri uri) {
            update();
        }
    };
    
    public void onPersistentGroupInfoAvailable(WifiP2pGroupList groups) {
        if( SystemProperties.get("tcc.wifi.p2p.persistent").equals("1")) {
	        mPersistentGroup.removeAll();
	        
	        for (WifiP2pGroup group: groups.getGroupList()) {
	            //Log.d(TAG, " group " + group);
	            mPersistentGroup.addPreference(new WifiDisplayPersistentGroup(getActivity(), group));
	        }
	        mPersistentGroups =  groups;
        }
    }

/*
    public void onGroupInfoAvailable(WifiP2pGroup group) {
        Log.d(TAG, " group " + group);
        mConnectedGroup = group;
        updateDevicePref();
    }
*/
    private final class WifiDisplayPreference extends Preference
            implements View.OnClickListener {
        private final WifiDisplay mDisplay;

        public WifiDisplayPreference(Context context, WifiDisplay display) {
            super(context);

            mDisplay = display;
            setTitle(display.getFriendlyDisplayName());
        }

        public WifiDisplay getDisplay() {
            return mDisplay;
        }

        @Override
        protected void onBindView(View view) {
            super.onBindView(view);

            ImageView deviceDetails = (ImageView) view.findViewById(R.id.deviceDetails);
            if (deviceDetails != null) {
                deviceDetails.setOnClickListener(this);

                if (!isEnabled()) {
                    TypedValue value = new TypedValue();
                    getContext().getTheme().resolveAttribute(android.R.attr.disabledAlpha,
                            value, true);
                    deviceDetails.setImageAlpha((int)(value.getFloat() * 255));
                }
            }
        }

        @Override
        public void onClick(View v) {
            showOptionsDialog(mDisplay);
        }
    }
}
