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
package com.telechips.android.isdbt.schedule;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.SystemClock;
import android.util.Log;

class Info_CurAlarm
{
	AlarmManager	managerAlarm[]	= null;
	
	Intent			intentReceiver[]	= null;
	PendingIntent	pendingIntent[]		= null;
}

public class Alarm
{
	static Info_CurAlarm	gInfo	= new Info_CurAlarm();	
	
	public static void set(Context mContext, int iID, ScheduleData data)
	{
		Log(I, "set(iID=" + iID + ")");
		
		if(gInfo.intentReceiver == null)
		{
			gInfo.intentReceiver	= new	Intent[Alert.MAX_ALARM];
			for(int i=0 ; i<Alert.MAX_ALARM ; i++)
			{
				gInfo.intentReceiver[i]	= new	Intent(mContext, ScheduleReceiver.class);
			}
			gInfo.managerAlarm	= new	AlarmManager[Alert.MAX_ALARM];
			gInfo.pendingIntent	= new	PendingIntent[Alert.MAX_ALARM];
		}
		
		if(		(gInfo.intentReceiver == null)
			||	(gInfo.pendingIntent == null)
		)
		{
			return;
		}
		
		long	interval=0, addTime=0;
		if(data.iRepeatType == 1)
		{
			interval	= 24*60*60*1000;
			//interval	= 30*1000;	// - test value
		}
		else if(data.iRepeatType == 2)
		{
			interval	= 7*24*60*60*1000;
			//interval	= 10*60*1000;	// - test value
		}
		addTime	= data.iCount * interval;

		long	firstTime	= SystemClock.elapsedRealtime() + getAddTime(data.lUTC_Start) + addTime;
		long	endTime		= SystemClock.elapsedRealtime() + getEndTime(data.lUTC_End) + addTime;

		setChangeChannel(gInfo, mContext, iID, data, firstTime, interval);
		if(data.iAlarmType == Alert.TYPE_ALARM_NOTICE_REC)
		{
			setStopRecord(gInfo, mContext, iID, data, endTime, interval);
		}
	}
	
	private static void setChangeChannel(Info_CurAlarm info, Context mContext, int iID, ScheduleData data, long firstTime, long interval)
	{
		Log(I, "setChangeChannel(iID=" + iID + ", Alert.EXTRA_TYPE_ALARM = " + data.iAlarmType + ")");

		info.intentReceiver[0].putExtra(Alert.EXTRA_TYPE_ALARM, data.iAlarmType);
		info.intentReceiver[0].putExtra(Alert.EXTRA_TYPE_CHANNEL_TYPE, data.iChannelType);
		info.intentReceiver[0].putExtra(Alert.EXTRA_TYPE_CHANNEL_INDEX, data.iIndexService);
		info.intentReceiver[0].putExtra(Alert.EXTRA_TYPE_ID, iID);
		info.intentReceiver[0].putExtra(Alert.EXTRA_TYPE_REPEAT, data.iRepeatType);
		
		info.pendingIntent[0]	= PendingIntent.getBroadcast(mContext, (iID-1)*Alert.MAX_ALARM, info.intentReceiver[0], PendingIntent.FLAG_UPDATE_CURRENT);
		info.managerAlarm[0]	= (AlarmManager)mContext.getSystemService(Context.ALARM_SERVICE);
	
		switch(data.iRepeatType)
		{
			case 0:
				info.managerAlarm[0].set(AlarmManager.ELAPSED_REALTIME_WAKEUP, firstTime, info.pendingIntent[0]);
			break;
		
			case 1:
			case 2:
				info.managerAlarm[0].setRepeating(AlarmManager.ELAPSED_REALTIME_WAKEUP, firstTime, interval, info.pendingIntent[0]);
			break;
		}
	}
	
	private static void setStopRecord(Info_CurAlarm info, Context mContext, int iID, ScheduleData data, long endTime, long interval)
	{
		Log(I, "setStopRecord(iID=" + iID + ", Alert.EXTRA_TYPE_ALARM = " + data.iAlarmType + ")");

		info.intentReceiver[1].putExtra(Alert.EXTRA_TYPE_ALARM, Alert.TYPE_ALARM_REC_STOP);
		
		info.pendingIntent[1]	= PendingIntent.getBroadcast(mContext, (iID-1)*Alert.MAX_ALARM+1, info.intentReceiver[1], PendingIntent.FLAG_UPDATE_CURRENT);
		info.managerAlarm[1]	= (AlarmManager)mContext.getSystemService(Context.ALARM_SERVICE);

		switch(data.iRepeatType)
		{
			case 0:
				info.managerAlarm[1].set(AlarmManager.ELAPSED_REALTIME_WAKEUP, endTime, info.pendingIntent[1]);
			break;
		
			case 1:
			case 2:
				info.managerAlarm[1].setRepeating(AlarmManager.ELAPSED_REALTIME_WAKEUP, endTime, interval, info.pendingIntent[1]);
			break;
		}
	}
	
	private static long getAddTime(long lStart)
	{
		Log(I, "getAddTime()");

		return (lStart - DxbScheduler.getCurrentTime_TOT());
	}
	
	private static long getEndTime(long lEnd)
	{
		Log(I, "getEndTime()");
		
		return (lEnd - DxbScheduler.getCurrentTime_TOT());
	}
	
	public static void release(Context context, ScheduleDB scheduleDB, int iID, int iType_repeat)
	{
		Log(I, "release(lID=" + iID + ", iType_repeat=" + iType_repeat + ")");
		//	iType_repeat --> 0-once, 1-daily, 2-weekly, (3)other-delete
		
		Intent			intentReceiver;
		PendingIntent	pendingIntent;
		AlarmManager	managerAlarm;
		
		intentReceiver	= new	Intent(context, ScheduleReceiver.class);

		for(int i=0 ; i<Alert.MAX_ALARM ; i++)
		{
			pendingIntent	= PendingIntent.getBroadcast(context, (iID-1)*Alert.MAX_ALARM + i, intentReceiver, 0);
			managerAlarm	= (AlarmManager)context.getSystemService(Context.ALARM_SERVICE);
			managerAlarm.cancel(pendingIntent);
		}

		if(		(iType_repeat == 1)
			||	(iType_repeat == 2)
		)
		{
			ScheduleDB	sDB	= null;
			if(scheduleDB == null)
			{
				sDB	= new ScheduleDB(context).open();
			}
			else
			{
				sDB	= scheduleDB;
			}
			
			ScheduleData	sData	= sDB.getData(iID);
			if(sData != null)
			{
				set(context, iID, sData);
			}
/*			if(		(scheduleDB == null)
				&&	(sDB != null)
			)
			{
				sDB.close();
			}*/
		}
	}
	
	static final int D = 0;
	static final int E = 1;
	static final int I = 2;
	static final int V = 3;
	static final int W = 4;
	
	private static void Log(int level, String mString)
	{
		if(Scheduler.TAG == null)
		{
			return;
		}
		
		String TAG = "[[[Alarm]]] ";
		if(TAG != null)
		{
			switch(level)
			{
	    		case E:
	    			Log.e(TAG, "Alarm." + mString);
	    		break;
	    		
	    		case I:
	    			Log.i(TAG, "Alarm." + mString);
	    		break;
	    		
	    		case V:
	    			Log.v(TAG, "Alarm." + mString);
	    		break;
	    		
	    		case W:
	    			Log.w(TAG, "Alarm." + mString);
	    		break;
	    		
	    		case D:
	    		default:
	    			Log.d(TAG, "Alarm." + mString);
	    		break;
			}
		}
	}
}
