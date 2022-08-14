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

package com.telechips.android.dvb.schedule;

import android.app.PendingIntent;
import android.app.PendingIntent.CanceledException;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.PowerManager.WakeLock;
import android.util.Log;

public class ScheduleReceiver extends BroadcastReceiver
{
	private Intent	intentAlert		= null;
	private Intent	intentStopRec	= new Intent(Alert.ACTION_REC_STOP);

	@Override
	public void onReceive(final Context context, final Intent intent)
	{
		Log(I, "onReceive()");
		
		final PendingResult result = goAsync();
		final WakeLock wl = AlarmAlertWakeLock.createPartialWakeLock(context);
		wl.acquire();
		
		AsyncHandler.post(new Runnable()
		{
			public void run()
			{
				handleIntent(context, intent);
				result.finish();
				wl.release();
			}
		});
	}

	private void handleIntent(Context context, Intent intent)
	{
		Log(I, "handleIntent()");

		boolean	isScreenOn=true;
		int	iChannel_Type=0, iChannel_Index=0, iID=0, iType_repeat=0;
    	
		int	iType	= intent.getIntExtra(Alert.EXTRA_TYPE_ALARM, 0);
		//Log(D, "iType=" + iType);
		switch(iType)
		{
			case Alert.TYPE_ALARM_NOTICE_VIEW:
			case Alert.TYPE_ALARM_NOTICE_REC:
			{
				isScreenOn	= AlarmAlertWakeLock.isScreenOn();
				iChannel_Type	= intent.getIntExtra(Alert.EXTRA_TYPE_CHANNEL_TYPE, 0);
				iChannel_Index	= intent.getIntExtra(Alert.EXTRA_TYPE_CHANNEL_INDEX, 0);
				iID				= intent.getIntExtra(Alert.EXTRA_TYPE_ID, 0);
				iType_repeat	= intent.getIntExtra(Alert.EXTRA_TYPE_REPEAT, 0);

				//	send alertState	- Dxb_backgroundVideo play
				{
					ScheduleAlert.sendDxb_AlertStart(context);
				}

				Scheduler.deleteData(context, null, null, Scheduler.TYPE_DELETE_MSG, iID);
				
				AlarmAlertWakeLock.acquireCpuWakeLock(context);
		        
				if(intentAlert == null)
				{
					intentAlert	= new Intent(context, ScheduleAlert.class);
				}
				if(intentAlert != null)
				{
					Log(D, "isScreenOn = " + isScreenOn);
					intentAlert.putExtra(Alert.EXTRA_TYPE_IS_SCREEN_ON,		isScreenOn);
					intentAlert.putExtra(Alert.EXTRA_TYPE_ALARM,			iType);
					intentAlert.putExtra(Alert.EXTRA_TYPE_CHANNEL_TYPE,		iChannel_Type);
					intentAlert.putExtra(Alert.EXTRA_TYPE_CHANNEL_INDEX,	iChannel_Index);
					intentAlert.putExtra(Alert.EXTRA_TYPE_ID,				iID);
					intentAlert.putExtra(Alert.EXTRA_TYPE_REPEAT,			iType_repeat);

					PendingIntent	pie	= PendingIntent.getActivity(context, 0, intentAlert, PendingIntent.FLAG_ONE_SHOT|PendingIntent.FLAG_UPDATE_CURRENT);
					try
					{
						pie.send();
					}
					catch(CanceledException e)
					{
						e.printStackTrace();
					}
				}
			}
			break;
			
			case Alert.TYPE_ALARM_REC_STOP:
				context.sendBroadcast(intentStopRec);
				AlarmAlertWakeLock.releaseCpuLock();
			break;
		}
	}
	
	private static final int D = 0;
	private static final int E = 1;
	private static final int I = 2;
	private static final int V = 3;
	private static final int W = 4;
	
	private static void Log(int level, String mString)
	{
		if(Scheduler.TAG == null)
		{
			return;
		}
		
		String TAG = "[[[ScheduleReceiver]]] ";
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
