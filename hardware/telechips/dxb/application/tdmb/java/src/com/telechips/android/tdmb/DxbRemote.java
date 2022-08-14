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

package com.telechips.android.tdmb;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class DxbRemote
{
	static String TAG = "[[[DxbRemote]]] ";

	private static final int D = 0;
	private static final int E = 1;
	private static final int I = 2;
	private static final int V = 3;
	private static final int W = 4;
	
	private static final String CHANNEL_ACTION = "com.telechips.android.dxb.remoteservice.channel.command";
	private static final String CHANNEL_SCAN_ACTION = "com.telechips.android.dxb.remoteservice.channel.scan.command";
	private static final String CHANNEL_UP_ACTION = "com.telechips.android.dxb.remoteservice.channel.up.command";
	private static final String CHANNEL_DOWN_ACTION = "com.telechips.android.dxb.remoteservice.channel.down.command";

	private static int mRemoteChannel_SCAN = 0;

	static void init()
	{
		Intent intent = Manager_Setting.mContext.getIntent();
		int channel_scan = intent.getIntExtra("channel_scan", 0);
		Log.d("#########################", "scan = " + channel_scan);
   
		if(channel_scan == 1)
		{
			setChannel(channel_scan);
		}
	   
//	   IntentFilter commandFilter = new IntentFilter();
//	   commandFilter.addAction(CHANNEL_ACTION);
//	   commandFilter.addAction(CHANNEL_SCAN_ACTION);
//	   commandFilter.addAction(CHANNEL_UP_ACTION);
//	   commandFilter.addAction(CHANNEL_DOWN_ACTION);
//	   registerReceiver(mIntentReceiver, commandFilter);
	}
	
	static void setChannel(int iChannel)
	{
		mRemoteChannel_SCAN	= iChannel;
	}
	
	static int getChannel()
	{
		return mRemoteChannel_SCAN;
	}

    private BroadcastReceiver mIntentReceiver = new BroadcastReceiver()
    {
        @Override
        public void onReceive(Context context, Intent intent)
        {
            String action = intent.getAction();
			if(CHANNEL_ACTION.equals(action))
			{
				int channel_id = intent.getIntExtra("channel_id", 1);
				Log.d("#########################", "id = " + channel_id);
				Manager_Setting.g_Information.cCOMM.iCurrent_TV = channel_id;
				Manager_Setting.g_Information.cCOMM.curChannels.moveToPosition(Manager_Setting.g_Information.cCOMM.iCurrent_TV);
				DxbView.setChannel();
			}
			else if(CHANNEL_SCAN_ACTION.equals(action))
			{
				DxbScan.startFull();
			}
			else if(CHANNEL_UP_ACTION.equals(action))
			{
				DxbView.changeChannel(DxbView.UP);
			}
			else if(CHANNEL_DOWN_ACTION.equals(action))
			{
				DxbView.changeChannel(DxbView.DOWN);
			}
        }
    };

    private static void DxbLog(int level, String mString)
	{
		if(TAG != null)
		{
			switch(level)
			{
	    		case E:
	    			Log.e(TAG, mString);
	    		break;
	    		
	    		case I:
	    			Log.i(TAG, mString);
	    		break;
	    		
	    		case V:
	    			Log.v(TAG, mString);
	    		break;
	    		
	    		case W:
	    			Log.w(TAG, mString);
	    		break;
	    		
	    		case D:
	    		default:
	    			Log.d(TAG, mString);
	    		break;
			}
		}
	}	
}
