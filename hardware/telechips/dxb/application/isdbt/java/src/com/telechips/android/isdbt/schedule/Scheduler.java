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

import java.util.Date;

import android.content.Context;
import android.database.Cursor;
import android.util.Log;

public class Scheduler
{
	static String TAG = "[[[Scheduler]]] ";
	//static String TAG = null;
	
	public final static int TYPE_DELETE_KEY			= 0;
	public final static int TYPE_DELETE_MSG			= 1;
	
	public final static long STATE_SUCCESS			= 0;
	public final static long STATE_EMPTY_DATA		= -1;
	public final static long STATE_INVALID_DATA		= -2;
	public final static long STATE_INVALID_TIME_END	= -3;
	public final static long STATE_INVALID_TIME_OVER	= -4;
	public final static long STATE_PLAYING			= -5;
	
	public final int TYPE_ALARM_ADD	= 0;
	public final int TYPE_ALARM_UPDATE	= 1;
	
//	IntentFilter commandFilter	= null;
	
//	Information	gInformation	= new Information();
	
	ScheduleDB makeScheduleDB(Context mContext)
	{
		Log(I, "makeScheduleDB()");

		return new ScheduleDB(mContext).open();
	}
	
	public void release(Context mContext, ScheduleDB scheduleDB)
	{
		Log(I, "release()");
		
		if(scheduleDB != null)
		{
			scheduleDB.close();
		}
	}
	
	public Cursor getList(Context mContext, ScheduleDB scheduleDB)
	{
		Log(I, "getList()");
		
		Cursor	return_cursor	= null;
		
//		if(gInformation == null)
//		{
//			gInformation	= new Information();
//		}
		
		if(scheduleDB != null)
		{
			return_cursor	= scheduleDB.getSchedule();
		}
		else
		{
			Log(D, "scheduleDB == null");
		}
		
		return 	return_cursor;
	}
	
	public ScheduleData getData(ScheduleDB scheduleDB, int iID)
	{
		Log(I, "getData(iID=" + iID + ")");
		
		ScheduleData	return_data	= null;
		
		if(scheduleDB != null)
		{
			return_data	= scheduleDB.getData(iID);
		}
		
		return	return_data;
	}
	
	public void reset(Context mContext, ScheduleDB scheduleDB, long currentTime)
	{
		Log(I, "reset()");
		
		if(scheduleDB != null)
		{
			releaseAlarm_All(mContext, scheduleDB);

			Cursor	curDB	= scheduleDB.getSchedule();
			int iCount	= curDB.getCount();
			if(iCount > 0)
			{
				curDB.moveToFirst();
				for(int i=0 ; i<iCount ; i++)
				{
					if(curDB.getInt(ScheduleDB.iColumnIndex_Repeat) == 0)
					{
						long	curT	= currentTime/60000 * 60000;
						long	startT	= curDB.getInt(ScheduleDB.iColumnIndex_UTC_Start)/60000 * 60000;
						Log(D, "curT=" + curT + ", startT=" + startT);
						if(curT <= startT)
						{
							scheduleDB.delete(curDB.getInt(ScheduleDB.iColumnIndex_ID));
						}
					}
				}
				
				if(curDB.getCount() > 0)
				{
					resetAlarm_All(mContext, scheduleDB);
				}
			}
		
		}
	}
	
	public long insert(Context mContext, ScheduleDB scheduleDB, ScheduleData data)
	{
		Log(I, "insert()");
		
		long	return_state	= STATE_SUCCESS;
		
		if(scheduleDB != null)
		{
			return_state	= isValid(scheduleDB, TYPE_ALARM_ADD, 0, data);
			if(return_state == STATE_SUCCESS)
			{
				int iID	= (int)scheduleDB.insert(data);
				if(iID > 0)
				{
					Alarm.set(mContext, iID, data);
				}
			}
		}
		
		Log(D, "return_state=" + return_state);
		return	return_state;
	}
	
	public long update(Context mContext, ScheduleDB scheduleDB, int iID, ScheduleData data)
	{
		Log(I, "update(iID=" + iID + ")");
		
		long	return_state	= STATE_SUCCESS;
		
		if(scheduleDB != null)
		{
			return_state	= isValid(scheduleDB, TYPE_ALARM_UPDATE, iID, data);
			if(return_state == STATE_SUCCESS)
			{
				return_state	= scheduleDB.update(iID, data);
				if(return_state > STATE_SUCCESS)
				{
					Alarm.set(mContext, iID, data);
				}
			}
		}
		
		Log(D, "return_state=" + return_state);
		return	return_state;
	}
	
	public long delete(Context context, boolean isResetAlarm, ScheduleDB scheduleDB, int iID)
	{
		Log(I, "delete(isResetAlarm=" + isResetAlarm + ", iID=" + iID + ")");
		
		long	return_state	= STATE_EMPTY_DATA;
		
		if(scheduleDB != null)
		{
			if(isResetAlarm)
			{
				releaseAlarm_All(context, scheduleDB);
			}
			else
			{
				Alarm.release(context, scheduleDB, iID, 3);	// 0-once, 1-daily, 2-weekly, (3)other-delete
			}

			return_state	= scheduleDB.delete(iID);
			
			if(isResetAlarm)
			{
				resetAlarm_All(context, scheduleDB);
			}
		}
		
		return return_state;
	}
	
	private void releaseAlarm_All(Context context, ScheduleDB dbSchedule)
	{
		Log(I, "releaseAlarm_All()");
		
		Cursor cursor	= dbSchedule.getSchedule();
		if(cursor != null)
		{
			int iCount	= cursor.getCount();
			Log(D, "iCount=" + iCount);
			if(iCount > 0)
			{
				cursor.moveToFirst();
				for(int i=0 ; i<iCount ; i++)
				{
					Alarm.release(context, dbSchedule, cursor.getInt(ScheduleDB.iColumnIndex_ID), 3);	//	iType_repeat --> 0-once, 1-daily, 2-weekly, (3)other-delete
					cursor.moveToNext();
				}
			}
		}
	}
	
	private void resetAlarm_All(Context context, ScheduleDB scheduleDB)
	{
		if(scheduleDB != null)
		{
			Cursor cursor	= scheduleDB.getSchedule();
			if(cursor != null)
			{
				int iCount	= cursor.getCount();
				if(iCount > 0)
				{
					cursor.moveToFirst();
					for(int i=0 ; i<iCount ; i++)
					{
						int	iID	= cursor.getInt(ScheduleDB.iColumnIndex_ID);
						if(iID > 0)
						{
							ScheduleData data	= getData(scheduleDB, iID);
							Alarm.set(context, iID, data);
						}
						cursor.moveToNext();
					}
				}
			}
		}
	}
	
	private long isValid(ScheduleDB scheduleDB, int iType, int iID, ScheduleData data)
	{
		Log(I, "isValid(iType=" + iType + ", iID=" + iID + ")");
		
		long return_state	= STATE_SUCCESS;
		
		long	pCurrent	= (data.lUTC_Current/60000) * 60000;
		long	pStart		= (data.lUTC_Start/60000) * 60000;
		long	pEnd		= (data.lUTC_End/60000) * 60000;
		
		if(pCurrent >= pStart)
		{
			return STATE_PLAYING;
		}
		
		if(data.iAlarmType == Alert.TYPE_ALARM_NOTICE_REC)
		{
			Log(D, "pStart = " + pStart + ", pEnd = " + pEnd);
			if(pStart >= pEnd)
			{
				return STATE_INVALID_TIME_END;
			}
			else if( (pStart+10*60*60000) < pEnd )
			{
				return STATE_INVALID_TIME_OVER;
			}
		}
		
		//if(iType == TYPE_ALARM_ADD)
		if(scheduleDB != null)
		{
			Cursor curSchedule	= scheduleDB.getSchedule();
			if(curSchedule != null)
			{
				int iCount	= curSchedule.getCount();
				if(iCount > 0)
				{
					curSchedule.moveToFirst();
					
					switch(data.iRepeatType)
					{
						case 0:	//	once
							return_state	= isValid_Once(iType, iID, data, curSchedule, iCount);
						break;
						
						case 1:	// daily
							return_state	= isValid_Daily(iType, iID, data, curSchedule, iCount);
						break;
						
						case 2:	// weekly
							return_state	= isValid_Weekly(iType, iID, data, curSchedule, iCount);
						break;
					}
				}
			}
		}
		
		Log(I, "return_state = " + return_state);
		return return_state;
	}
	
	private long isValid_Once(int iType, int iID, ScheduleData data, Cursor curSchedule, int iCount)
	{
		Log(I, "isValid_Once(data.lUTC_Start = " + data.lUTC_Start + ", data.lUTC_End = " + data.lUTC_End +")");
		
		long return_state	= STATE_SUCCESS;
		
		for(int i=0 ; i<iCount ; i++)
		{
			if(iType == TYPE_ALARM_UPDATE)
			{
				if(iID == curSchedule.getInt(ScheduleDB.iColumnIndex_ID))
				{
					continue;
				}
			}
			
			long	dStart	= curSchedule.getLong(ScheduleDB.iColumnIndex_UTC_Start);
			long	dEnd	= curSchedule.getLong(ScheduleDB.iColumnIndex_UTC_End);
			int		dRepeat	= curSchedule.getInt(ScheduleDB.iColumnIndex_Repeat);
			Log(D, "dStart=" + dStart + ", dEnd=" + dEnd);
			
			switch(dRepeat)
			{
				case 0:	// Once
				{
					if(		(data.lUTC_Start == dStart)
						||	(data.lUTC_End == dEnd)
					)
					{
						return curSchedule.getInt(ScheduleDB.iColumnIndex_ID);
					}
					if(		(data.lUTC_Start < dStart)
						&&	(data.lUTC_End > dStart)
					)
					{
						return curSchedule.getInt(ScheduleDB.iColumnIndex_ID);
					}
					if(		(data.lUTC_Start < dEnd)
						&&	(data.lUTC_End > dEnd)
					)
					{
						return curSchedule.getInt(ScheduleDB.iColumnIndex_ID);
					}
					if(		(dStart < data.lUTC_Start)
						&&	(dEnd > data.lUTC_Start)
					)
					{
						return curSchedule.getInt(ScheduleDB.iColumnIndex_ID);
					}
					if(		(dStart < data.lUTC_End)
						&&	(dEnd > data.lUTC_End)
					)
					{
						return curSchedule.getInt(ScheduleDB.iColumnIndex_ID);
					}
				}
				break;
				
				case 1:	// daily
				{
					long	ppStart	= data.lUTC_Start	% (24*60*60*1000);
					long	ppEnd	= data.lUTC_End		% (24*60*60*1000);
					long	ddStart	= dStart	% (24*60*60*1000);
					long	ddEnd	= dEnd		% (24*60*60*1000);
					
					if(		(ppStart == ddStart)
						||	(ppEnd == ddEnd)
					)
					{
						return curSchedule.getInt(ScheduleDB.iColumnIndex_ID);
					}
					
					if(		(ppStart < ddStart)
						&&	(ppEnd > ddStart)
					)
					{
						return curSchedule.getInt(ScheduleDB.iColumnIndex_ID);
					}
					
					if(		(ppStart < ddEnd)
						&&	(ppEnd > ddEnd)
					)
					{
						return curSchedule.getInt(ScheduleDB.iColumnIndex_ID);
					}
				}
				break;
					
				case 2:	// weekly
				{
					Date pDate	= new Date(data.lUTC_Start);
					Date dDate	= new Date(dStart);
					
					int	pDay	= pDate.getDay();
					int	dDay	= dDate.getDay();
					
					if(pDay == dDay)
					{
						long	ppStart	= data.lUTC_Start	% (24*60*60*1000);
						long	ppEnd	= data.lUTC_End		% (24*60*60*1000);
						long	ddStart	= dStart	% (24*60*60*1000);
						long	ddEnd	= dEnd		% (24*60*60*1000);
						
						if(		(ppStart == ddStart)
							||	(ppEnd == ddEnd)
						)
						{
							return curSchedule.getInt(ScheduleDB.iColumnIndex_ID);
						}
						
						if(		(ppStart < ddStart)
							&&	(ppEnd > ddStart)
						)
						{
							return curSchedule.getInt(ScheduleDB.iColumnIndex_ID);
						}
						
						if(		(ppStart < ddEnd)
							&&	(ppEnd > ddEnd)
						)
						{
							return curSchedule.getInt(ScheduleDB.iColumnIndex_ID);
						}						
					}
				}
				break;
			}
		}
		
		Log(D, "return return_state;");
		return return_state;
	}

	private long isValid_Daily(int iType, int iID, ScheduleData data, Cursor curSchedule, int iCount)
	{
		Log(I, "isValid_Daily()");
		
		long return_state	= STATE_SUCCESS;
		
		long	pStart	= ((data.lUTC_Start/60000) * 60000)	% (24*60*60*1000);
		long	pEnd;
		{
			long	tempStart	= pStart % (24*60*60*1000);
			long	tempEnd		= ((data.lUTC_End/60000) * 60000) % (24*60*60*1000);
			long	tempDuration	= (tempEnd + (24*60*60*1000) - tempStart) % (24*60*60*1000);
			
			if(tempDuration == 0)
			{
				pEnd	= pStart + (24*60*60*1000);
			}
			else
			{
				pEnd	= pStart + tempDuration;
			}
		}
		Log(D, "pStart=" + pStart + ", pEnd=" + pEnd);
		
		for(int i=0 ; i<iCount ; i++)
		{
			if(iType == TYPE_ALARM_UPDATE)
			{
				if(iID == curSchedule.getInt(ScheduleDB.iColumnIndex_ID))
				{
					continue;
				}
			}
			
			long	dStart	= ((curSchedule.getLong(ScheduleDB.iColumnIndex_UTC_Start)/60000) * 60000)	% (24*60*60*1000);
			long	dEnd	= ((curSchedule.getLong(ScheduleDB.iColumnIndex_UTC_End)/60000) * 60000)	% (24*60*60*1000);
			
			if(		(pStart == dStart)
				||	(pEnd == dEnd)
			)
			{
				return curSchedule.getInt(ScheduleDB.iColumnIndex_ID);
			}
			
			if(		(pStart < dStart)
				&&	(pEnd > dStart)
			)
			{
				return curSchedule.getInt(ScheduleDB.iColumnIndex_ID);
			}
			
			if(		(pStart < dEnd)
				&&	(pEnd > dEnd)
			)
			{
				return curSchedule.getInt(ScheduleDB.iColumnIndex_ID);
			}
		}
		
		return return_state;
	}
	
	private long isValid_Weekly(int iType, int iID, ScheduleData data, Cursor curSchedule, int iCount)
	{
		Log(I, "isValid_Weekly()");
		
		long return_state	= STATE_SUCCESS;
		
		long	pStart	= (data.lUTC_Start/60000) * 60000;
		long	pEnd;
		{
			long	tempStart	= pStart % (24*60*60*1000);
			long	tempEnd		= ((data.lUTC_End/60000) * 60000) % (24*60*60*1000);
			long	tempDuration	= (tempEnd + (24*60*60*1000) - tempStart) % (24*60*60*1000);
			
			if(tempDuration == 0)
			{
				pEnd	= pStart + (24*60*60*1000);
			}
			else
			{
				pEnd	= pStart + tempDuration;
			}
		}
		Log(D, "pStart=" + pStart + ", pEnd=" + pEnd);
		
		for(int i=0 ; i<iCount ; i++)
		{
			if(iType == TYPE_ALARM_UPDATE)
			{
				if(iID == curSchedule.getInt(ScheduleDB.iColumnIndex_ID))
				{
					continue;
				}
			}
			
			long	dStart	= (curSchedule.getLong(ScheduleDB.iColumnIndex_UTC_Start)/60000) * 60000;
			long	dEnd	= (curSchedule.getLong(ScheduleDB.iColumnIndex_UTC_End)/60000) * 60000;
			int		dRepeat	= curSchedule.getInt(ScheduleDB.iColumnIndex_Repeat);
			
			switch(dRepeat)
			{
				case 1:	// daily
				{
					long	ppStart	= pStart	% (24*60*60*1000);
					long	ppEnd	= pEnd		% (24*60*60*1000);
					long	ddStart	= dStart	% (24*60*60*1000);
					long	ddEnd	= dEnd		% (24*60*60*1000);
					
					if(		(ppStart == ddStart)
						||	(ppEnd == ddEnd)
					)
					{
						return curSchedule.getInt(ScheduleDB.iColumnIndex_ID);
					}
					
					if(		(ppStart < ddStart)
						&&	(ppEnd > ddStart)
					)
					{
						return curSchedule.getInt(ScheduleDB.iColumnIndex_ID);
					}
					
					if(		(ppStart < ddEnd)
						&&	(ppEnd > ddEnd)
					)
					{
						return curSchedule.getInt(ScheduleDB.iColumnIndex_ID);
					}
				}
				break;
				
				case 0:	// Once
				case 2:	// weekly
				{
					Date pDate	= new Date(pStart);
					Date dDate	= new Date(dStart);
					
					int	pDay	= pDate.getDay();
					int	dDay	= dDate.getDay();
					
					if(pDay == dDay)
					{
						long	ppStart	= pStart	% (24*60*60*1000);
						long	ppEnd	= pEnd		% (24*60*60*1000);
						long	ddStart	= dStart	% (24*60*60*1000);
						long	ddEnd	= dEnd		% (24*60*60*1000);
						
						if(		(ppStart == ddStart)
							||	(ppEnd == ddEnd)
						)
						{
							return curSchedule.getInt(ScheduleDB.iColumnIndex_ID);
						}
						
						if(		(ppStart < ddStart)
							&&	(ppEnd > ddStart)
						)
						{
							return curSchedule.getInt(ScheduleDB.iColumnIndex_ID);
						}
						
						if(		(ppStart < ddEnd)
							&&	(ppEnd > ddEnd)
						)
						{
							return curSchedule.getInt(ScheduleDB.iColumnIndex_ID);
						}						
					}
				}
				break;
			}
		}
		
		return return_state;
	}
	
	public static void deleteData(Context context, ScheduleDB scheduleDB, Cursor cursorDB, int iType, int iID)
	{
		Log(I, "deleteData(iType=" + iType + ", lID=" + iID + ")");
		Log(D, "scheduleDB=" + scheduleDB + ", cursorDB=" + cursorDB);
		
		ScheduleDB	sDB	= null;
		Cursor	curDB;
		if(		(scheduleDB == null)
			||	(cursorDB == null)
		)
		{
			sDB	= new ScheduleDB(context).open();
			curDB	= sDB.getSchedule();
		}
		else
		{
			sDB		= scheduleDB;
			curDB	= cursorDB;
		}
		
		if(curDB != null)
		{
			int	iCount	= curDB.getCount();
			Log(D, "iCount=" + iCount);
			if(iCount > 0)
			{
				int	iCurrent_RepeatType	= -1;
				int	iCurrent_Count		= -1;
				curDB.moveToFirst();
				for(int i=0 ; i<iCount ; i++)
				{
					int	curID	= curDB.getInt(ScheduleDB.iColumnIndex_ID);
					Log(D, "curID=" + curID);
					if(		(iCount==2)
						&&	(iType == TYPE_DELETE_MSG)
						&&	(iID != curID)
					)
					{
						Alarm.release(context, sDB, curID, 3);	//	iType_repeat --> 0-once, 1-daily, 2-weekly, (3)other-delete
					}
					else if(	(iCount == 2)
							&&	(iType == TYPE_DELETE_KEY)
					)
					{
						Alarm.release(context, sDB, curID, 3);	//	iType_repeat --> 0-once, 1-daily, 2-weekly, (3)other-delete
					}
					else if(	(iID == curID)
							&&	(iType == TYPE_DELETE_KEY)
					)
					{
						Alarm.release(context, sDB, curID, 3);	//	iType_repeat --> 0-once, 1-daily, 2-weekly, (3)other-delete
					}
					
					if(iID == curID)
					{
						iCurrent_RepeatType	= curDB.getInt(ScheduleDB.iColumnIndex_Repeat);
						iCurrent_Count		= curDB.getInt(ScheduleDB.iColumnIndex_Count) + 1;
					}
				}
				
				Log(D, "iType=" + iType);
				if(		(iType == TYPE_DELETE_MSG)
					&&	(iCurrent_RepeatType > 0)
				)
				{
					sDB.updateRepeatCount((int)iID, iCurrent_Count++);
				}
				else
				{
					sDB.delete(iID);
				}
			}
		}
		
		if(		(sDB != null)
			&&	(scheduleDB == null)
		)
		{
			curDB.close();
			sDB.close();
		}
	}
	
	private static final int D = 0;
	private static final int E = 1;
	private static final int I = 2;
	private static final int V = 3;
	private static final int W = 4;
	
	private static void Log(int level, String mString)
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
