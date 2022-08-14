package com.android.settings.telechips.ethernet;

import java.util.List;

import com.android.settings.R;

import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.NetworkInfo;
import android.net.ethernet.EthernetManager;
import android.net.ethernet.EthernetDevInfo;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.TextView;
import android.util.Log;
import android.view.KeyEvent;

public class EthernetConfigDialog extends AlertDialog implements DialogInterface.OnClickListener,
AdapterView.OnItemSelectedListener, View.OnClickListener{
	private final String TAG = "EtherenetSettings";
	private EthernetEnabler mEthEnabler;
	private View mView;
	private Spinner mDevList;
	private TextView mDevs;
	private RadioButton mConTypeDhcp;
	private RadioButton mConTypeManual;
	private EditText mIpaddr;
	private EditText mDns;
	private EditText mGw;
	private EditText mMask;

	private EthernetLayer mEthLayer;
	private EthernetManager mEthManager;
	private EthernetDevInfo mEthInfo;
	private boolean mEnablePending;

	public EthernetConfigDialog(Context context, EthernetEnabler Enabler) {
		super(context);
		mEthLayer = new EthernetLayer(this);
		mEthEnabler = Enabler;
		mEthManager=Enabler.getManager();
		buildDialogContent(context);

	}

	public int buildDialogContent(Context context) {
		this.setTitle(R.string.eth_config_title);
		this.setView(mView = getLayoutInflater().inflate(R.layout.eth_configure, null));
		mDevs = (TextView) mView.findViewById(R.id.eth_dev_list_text);
        mDevList = (Spinner) mView.findViewById(R.id.eth_dev_spinner);
        mConTypeDhcp = (RadioButton) mView.findViewById(R.id.dhcp_radio);
        mConTypeManual = (RadioButton) mView.findViewById(R.id.manual_radio);
        mIpaddr = (EditText)mView.findViewById(R.id.ipaddr_edit);
        mMask = (EditText)mView.findViewById(R.id.netmask_edit);
        mDns = (EditText)mView.findViewById(R.id.eth_dns_edit);
        mGw = (EditText)mView.findViewById(R.id.eth_gw_edit);

	    mConTypeDhcp.setChecked(true);
		mConTypeManual.setChecked(false);
        mIpaddr.setEnabled(false);
        mMask.setEnabled(false);
		mDns.setEnabled(false);
		mGw.setEnabled(false);
        mConTypeManual.setOnClickListener(new RadioButton.OnClickListener() {
			public void onClick(View v) {
				mIpaddr.setEnabled(true);
		        mDns.setEnabled(true);
		        mGw.setEnabled(true);
		        mMask.setEnabled(true);
			}
        });

        mConTypeDhcp.setOnClickListener(new RadioButton.OnClickListener() {
			public void onClick(View v) {
				mIpaddr.setEnabled(false);
		        mDns.setEnabled(false);
		        mGw.setEnabled(false);
		        mMask.setEnabled(false);
			}
        });

		this.setInverseBackgroundForced(true);
		this.setButton(BUTTON_POSITIVE, context.getText(R.string.menu_save), this);
		this.setButton(BUTTON_NEGATIVE, context.getText(R.string.menu_cancel), this);
		String[] Devs = mEthEnabler.getManager().getDeviceNameList();
		if (Devs != null) {
			Log.i(TAG,"found device: " + Devs[0]);
			updateDevNameList(Devs);
			if (mEthManager.isEthConfigured()) {
				mEthInfo = mEthManager.getSavedEthConfig();
				for (int i = 0 ; i < Devs.length; i++) {
					if (Devs[i].equals(mEthInfo.getIfName())) {
						mDevList.setSelection(i);
						break;
					}
				}

				mIpaddr.setText(mEthInfo.getIpAddress());
				mGw.setText(mEthInfo.getRouteAddr());
				mDns.setText(mEthInfo.getDnsAddr());
				mMask.setText(mEthInfo.getNetMask());
				if (mEthInfo.getConnectMode().equals(EthernetDevInfo.ETH_CONN_MODE_DHCP))
				{
					mIpaddr.setEnabled(false);
					mDns.setEnabled(false);
					mGw.setEnabled(false);
					mMask.setEnabled(false);
				} else {
					mConTypeDhcp.setChecked(false);
					mConTypeManual.setChecked(true);
					mIpaddr.setEnabled(true);
					mDns.setEnabled(true);
					mGw.setEnabled(true);
					mMask.setEnabled(true);
				}
			}
		}


		return 0;
	}


	private void handle_saveconf() {
		EthernetDevInfo info = new EthernetDevInfo();
		
		//xelloss for abnormal exit
		if(mDevList == null ||mDevList.getSelectedItem()== null )
		{
			//mEthEnabler.setEthEnabled(false);
			mEthEnabler.setCheckBox(mEthManager.ETH_STATE_DISABLED);
			return;
		}
		
		info.setIfName(mDevList.getSelectedItem().toString());
		Log.i(TAG, "Config device for " + mDevList.getSelectedItem().toString());
		if (mConTypeDhcp.isChecked()) {
			Log.i(TAG,"mode dhcp");
			info.setConnectMode(EthernetDevInfo.ETH_CONN_MODE_DHCP);
			info.setIpAddress(null);
			info.setRouteAddr(null);
			info.setDnsAddr(null);
			info.setNetMask(null);
		} else {
			Log.i(TAG,"mode manual");
			info.setConnectMode(EthernetDevInfo.ETH_CONN_MODE_MANUAL);
			info.setIpAddress(mIpaddr.getText().toString());
			info.setRouteAddr(mGw.getText().toString());
			info.setDnsAddr(mDns.getText().toString());
			info.setNetMask(mMask.getText().toString());
		}
		mEthManager.UpdateEthDevInfo(info);
		if (mEnablePending) {
			//xelloss update checkbox
			mEthEnabler.setCheckBox(mEthManager.ETH_STATE_ENABLING);
			mEthManager.setEthernetState(mEthManager.ETH_STATE_ENABLING);
			mEnablePending = false;
		}
	}

	public void onClick(DialogInterface dialog, int which) {
		switch(which) {
		case BUTTON_POSITIVE:
			handle_saveconf();
			break;
		case BUTTON_NEGATIVE:
			//xelloss update checkbox
			mEthEnabler.setCheckBox(mEthManager.ETH_STATE_DISABLED);
			//Don't need to do anything
			break;
		default:
			Log.e(TAG,"Unknow button");
		}

	}

	public void onItemSelected(AdapterView<?> parent, View view, int position,
			long id) {
		// TODO Auto-generated method stub

	}

	public void onNothingSelected(AdapterView<?> parent) {
		// TODO Auto-generated method stub

	}

	public void onClick(View v) {
		// TODO Auto-generated method stub

	}
	//xelloss for back key
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		if (keyCode == KeyEvent.KEYCODE_BACK)
		{
			mEthEnabler.setCheckBox(mEthManager.ETH_STATE_DISABLED);
		}

		return super.onKeyDown(keyCode, event);
	}

	public void updateDevNameList(String[] DevList) {
		if (DevList != null) {
            ArrayAdapter<CharSequence> adapter = new ArrayAdapter<CharSequence>(
                    getContext(), android.R.layout.simple_spinner_item, DevList);
            adapter.setDropDownViewResource(
                    android.R.layout.simple_spinner_dropdown_item);
            mDevList.setAdapter(adapter);
        }

	}

	public void enableAfterConfig() {
		// TODO Auto-generated method stub
		mEnablePending = true;
	}

}
