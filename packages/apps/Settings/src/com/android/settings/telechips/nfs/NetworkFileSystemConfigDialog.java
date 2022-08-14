/*
 * Copyright (C) 2010 Telechips, Inc.
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

package com.android.settings.telechips.nfs;

import android.content.Context;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.net.networkfilesystem.NetworkFileSystemManager;
import android.net.networkfilesystem.NetworkFileSystemDevInfo;
import android.view.View;
import android.widget.AdapterView;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.Spinner;
import android.widget.TextView;
import android.util.Log;

import com.android.settings.R;

public class NetworkFileSystemConfigDialog extends AlertDialog implements DialogInterface.OnClickListener,
		AdapterView.OnItemSelectedListener, View.OnClickListener{
	private final String TAG = "NetworkFileSystemSettings";
	private static final boolean DEBUG = true;
	private NetworkFileSystemEnabler mNFSEnabler;
	private View mView;
	private Spinner mDevList;
	private TextView mDevs;
	private RadioButton mConTypeNfs;
	private RadioButton mConTypeCifs;
	private EditText mIpaddr;
	private EditText mPath;
	private EditText mUser;
	private EditText mPass;

	private NetworkFileSystemManager mNFSManager;
	private NetworkFileSystemDevInfo mNFSInfo;
	private boolean mEnablePending;

	public NetworkFileSystemConfigDialog(Context context, NetworkFileSystemEnabler Enabler) {
		super(context);
		mNFSEnabler = Enabler;
		mNFSManager=Enabler.getManager();
		buildDialogContent(context);
	}

	public int buildDialogContent(Context context) {
		if (DEBUG) Log.d(TAG, "buildDialogContent");

		this.setTitle(R.string.nfs_config_title);
		this.setView(mView = getLayoutInflater().inflate(R.layout.nfs_configure, null));
        mConTypeNfs = (RadioButton) mView.findViewById(R.id.nfs_radio);
        mConTypeCifs = (RadioButton) mView.findViewById(R.id.cifs_radio);
        mIpaddr = (EditText)mView.findViewById(R.id.ipaddr_edit);
        mPath = (EditText)mView.findViewById(R.id.nfs_path_edit);
        mUser = (EditText)mView.findViewById(R.id.nfs_user_edit);
        mPass = (EditText)mView.findViewById(R.id.nfs_password_edit);

		mConTypeNfs.setChecked(true);
		mConTypeCifs.setChecked(false);
        mIpaddr.setEnabled(true);
		mPath.setEnabled(true);
		mUser.setEnabled(false);
		mPass.setEnabled(false);

        mConTypeCifs.setOnClickListener(new RadioButton.OnClickListener() {
			public void onClick(View v) {
			mIpaddr.setEnabled(true);
		    mPath.setEnabled(true);
			mUser.setEnabled(true);
			mPass.setEnabled(true);
			}
        });

        mConTypeNfs.setOnClickListener(new RadioButton.OnClickListener() {
			public void onClick(View v) {
			mIpaddr.setEnabled(true);
		    mPath.setEnabled(true);
			mUser.setEnabled(false);
			mPass.setEnabled(false);
			}
        });

		this.setInverseBackgroundForced(true);
		this.setButton(BUTTON_POSITIVE, context.getText(R.string.menu_save), this);
		this.setButton(BUTTON_NEGATIVE, context.getText(R.string.menu_cancel), this);
		mNFSInfo = mNFSManager.getNfsConfig(mNFSEnabler.NFS_NAME);
		if (mNFSInfo != null) {
			mIpaddr.setText(mNFSInfo.getIpAddress());
			mPath.setText(mNFSInfo.getPath());
			mUser.setText(mNFSInfo.getUser());
			mPass.setText(mNFSInfo.getPass());

			if (mNFSInfo.getConnectMode().equals(NetworkFileSystemDevInfo.NFS_CONN_MODE_NFS))
			{
				mIpaddr.setEnabled(true);
				mPath.setEnabled(true);
				mUser.setEnabled(false);
				mPass.setEnabled(false);
			} else {
				mConTypeNfs.setChecked(false);
				mConTypeCifs.setChecked(true);
				mIpaddr.setEnabled(true);
				mPath.setEnabled(true);
				mUser.setEnabled(true);
				mPass.setEnabled(true);
			}
		}
		return 0;
	}


	private void handle_saveconf() {
		if (DEBUG) Log.d(TAG, "handle_saveconf");

		NetworkFileSystemDevInfo info = new NetworkFileSystemDevInfo();
				
		if (mConTypeNfs.isChecked()) {
			Log.i(TAG,"mode nfs");
			info.setLabel(mNFSEnabler.NFS_NAME);
			info.setConnectMode(NetworkFileSystemDevInfo.NFS_CONN_MODE_NFS);
			info.setIpAddress(mIpaddr.getText().toString());
			info.setPath(mPath.getText().toString());
			info.setUser(mUser.getText().toString());
			info.setPass(mPass.getText().toString());
		} else {
			Log.i(TAG,"mode cifs");
			info.setLabel(mNFSEnabler.NFS_NAME);
			info.setConnectMode(NetworkFileSystemDevInfo.NFS_CONN_MODE_CIFS);
			info.setIpAddress(mIpaddr.getText().toString());
			info.setPath(mPath.getText().toString());
			info.setUser(mUser.getText().toString());
			info.setPass(mPass.getText().toString());
		}
		mNFSManager.setNfsConfig(info);
	}

	public void onClick(DialogInterface dialog, int which) {
		if (DEBUG) Log.d(TAG, "onClick");

		switch(which) {
		case BUTTON_POSITIVE:
			handle_saveconf();
			mNFSEnabler.setNfsEnabled(true);
			break;
		case BUTTON_NEGATIVE:
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
}
