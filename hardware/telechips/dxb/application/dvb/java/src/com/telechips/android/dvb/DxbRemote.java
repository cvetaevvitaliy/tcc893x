/*
 * Copyright (C) 2013 Telechips, Inc.
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

package com.telechips.android.dvb;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class DxbRemote {

	private static String TAG = "[[[DxbRemote]]] ";

	private static final String CHANNEL_ACTION = "com.telechips.android.dxb.remoteservice.channel.command";
	private static final String CHANNEL_SCAN_ACTION = "com.telechips.android.dxb.remoteservice.channel.scan.command";
	private static final String CHANNEL_UP_ACTION = "com.telechips.android.dxb.remoteservice.channel.up.command";
	private static final String CHANNEL_DOWN_ACTION = "com.telechips.android.dxb.remoteservice.channel.down.command";

	private int mRemoteChannel_SCAN = 0;
	private DVBPlayerActivity mContext;

	public DxbRemote(DVBPlayerActivity context) {
		mContext = context;
		Intent intent = mContext.getIntent();
		int channel_scan = intent.getIntExtra("channel_scan", 0);
		Log.d("#########################", "scan = " + channel_scan);
		if(channel_scan == 1) {
			setChannel(channel_scan);
		}
//	   IntentFilter commandFilter = new IntentFilter();
//	   commandFilter.addAction(CHANNEL_ACTION);
//	   commandFilter.addAction(CHANNEL_SCAN_ACTION);
//	   commandFilter.addAction(CHANNEL_UP_ACTION);
//	   commandFilter.addAction(CHANNEL_DOWN_ACTION);
//	   registerReceiver(mIntentReceiver, commandFilter);
	}
	
	public void setChannel(int iChannel) {
		mRemoteChannel_SCAN	= iChannel;
	}
	
	public int getChannel() {
		return mRemoteChannel_SCAN;
	}

    private BroadcastReceiver mIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
			if (CHANNEL_ACTION.equals(action)) {
				int channel_id = intent.getIntExtra("channel_id", 1);
				Log.d("#########################", "id = " + channel_id);
				mContext.getPlayer().setChannel(channel_id);
			} else if(CHANNEL_SCAN_ACTION.equals(action)) {
//				DxbScan.startFull();
			} else if(CHANNEL_UP_ACTION.equals(action)) {
				mContext.getPlayer().setChannelUp();
			} else if(CHANNEL_DOWN_ACTION.equals(action)) {
				mContext.getPlayer().setChannelDown();
			}
        }
    };
}
