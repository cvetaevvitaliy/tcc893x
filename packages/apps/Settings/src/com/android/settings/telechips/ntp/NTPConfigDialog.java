/*
 *  * Copyright (C) 2010 Telechips, Inc.
 *   *
 *   * Licensed under the Apache License, Version 2.0 (the "License");
 *   * you may not use this file except in compliance with the License.
 *   * You may obtain a copy of the License at
 *   *
 *   *      http://www.apache.org/licenses/LICENSE-2.0
 *   *
 *   * Unless required by applicable law or agreed to in writing, software
 *   * distributed under the License is distributed on an "AS IS" BASIS,
 *   * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   * See the License for the specific language governing permissions and
 *   * limitations under the License.
 *   */

package com.android.settings.telechips.ntp;

import com.android.settings.R;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.TextView;
import android.util.Log;
import android.provider.Settings;
import android.content.ContentResolver;
import android.content.res.Resources;
import android.view.KeyEvent;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import java.util.List;

public class NTPConfigDialog extends AlertDialog implements DialogInterface.OnClickListener,
AdapterView.OnItemSelectedListener, View.OnClickListener{
	private final String TAG = "NTPConfiguration";
	
	private final String NTP_SERVER = "pool.ntp.org";//"kr.pool.ntp.org" "gps.bora.net"
	private static final boolean DEBUG = true;
	private View mView;
	private Context mContext;
	private EditText ntp_server;

	public NTPConfigDialog(Context context) {
		super(context);
		mContext = context;

		buildDialogContent(context);
	}

	public int buildDialogContent(Context context) {
		if (DEBUG) Log.d(TAG, "buildDialogContent");

		this.setTitle(R.string.ntp_config_title);
		this.setView(mView = getLayoutInflater().inflate(R.layout.ntp_configure, null));
        ntp_server = (EditText)mView.findViewById(R.id.ntp_server_edit);

		ntp_server.setEnabled(true);

		this.setInverseBackgroundForced(true);
		this.setButton(BUTTON_POSITIVE, context.getText(R.string.menu_save), this);
		this.setButton(BUTTON_NEGATIVE, context.getText(R.string.menu_cancel), this);
		
		if (isNTPConfigured()) {
			
			final ContentResolver cr = mContext.getContentResolver();

			ntp_server.setText(Settings.Global.getString(cr, Settings.Global.NTP_SERVER));

		}
		else{
			final Resources res = context.getResources();
            final ContentResolver resolver = context.getContentResolver();
            final String defaultServer = res.getString(
                    com.android.internal.R.string.config_ntpServer);
			
			ntp_server.setText(defaultServer);
			
			if(DEBUG) Log.d(TAG, "defaultServer = " + defaultServer);
		}
		
		ntp_server.setEnabled(true);
		return 0;
	}

	
	public boolean isNTPConfigured() {
	
		final ContentResolver cr = mContext.getContentResolver();
		int c = Settings.System.getInt(cr, Settings.System.NTP_CONF,0);

		if(DEBUG) Log.d(TAG, "isNTPConfigured = " + c);
		
		if (c == 1){
			return true;
		}
		
		return false;
	}
	
	private void handle_saveconf() {
		if (DEBUG) Log.d(TAG, "handle_saveconf");

		if( ntp_server.getText().toString().length() == 0)
		{
			if (DEBUG) Log.d(TAG, "handle_saveconf no input return");
			return;
		}
		
		final ContentResolver cr = mContext.getContentResolver();
	    Settings.System.putInt(cr, Settings.System.NTP_CONF,1);
		//Settings.System.putInt(cr, Settings.System.NTP_UPDATE,1);
		
		Settings.Global.putString(cr, Settings.Global.NTP_SERVER, ntp_server.getText().toString());

		//fake setting to trigger EVENT_AUTO_TIME_CHANGED for NetworkTimeUpdateService when NTP Server is changed without modification source of original sdk
		int auto_time;
		auto_time = Settings.Global.getInt(cr, Settings.Global.AUTO_TIME,0);
		Settings.Global.putInt(cr, Settings.Global.AUTO_TIME,(auto_time!=0) ? 0:1);
		Settings.Global.putInt(cr, Settings.Global.AUTO_TIME,auto_time);
				
	}

	public void onClick(DialogInterface dialog, int which) {
		if (DEBUG) Log.d(TAG, "onClick");

		switch(which) {
		case BUTTON_POSITIVE:
			handle_saveconf();
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
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		if (DEBUG) Log.d(TAG, "onKeyDown");
	
		// nothing
		return super.onKeyDown(keyCode, event);
	}
}
