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

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

class ScheduleData
{
	int		iIndexService;
	int		iChannelType;
	String	sChannelName;

	long	lUTC_Current;
	long	lUTC_Start;
	long	lUTC_End;
	
	int		iRepeatType;	// 0-once, 1-daily, 2-weekly
	int		iAlarmType;
	
	int		iCount;
}

public class ScheduleDB
{
	private final int DATABASE_VERSION = 1;

	static int	iColumnIndex_ID	= -1;
	static int	iColumnIndex_IndexService, iColumnIndex_ChannelType;
	static int	iColumnIndex_Name;
	static int	iColumnIndex_UTC_Start;
	static int	iColumnIndex_UTC_End;
	static int	iColumnIndex_Repeat;
	static int	iColumnIndex_AlarmType;
	static int	iColumnIndex_Count;

	private final String DATABASE_NAME	= "SCHEDULE.db";
	private final String DATABASE_TABLE = "schedule";
	
	private final String KEY_ROWID			= "_id";
	private final String KEY_INDEX_SERVICE	= "iIndexService";
	private final String KEY_CHANNEL_TYPE	= "iChannelType";
	private final String KEY_CHANNEL_NAME	= "sChannelName";
	private final String KEY_UTC_START		= "lUTC_Start";
	private final String KEY_UTC_END		= "lUTC_End";
	private final String KEY_REPEAT			= "iRepeatType";
	private final String KEY_ALARM_TYPE		= "iAlarmType";
	private final String KEY_COUNT			= "iCount";

	private String DATABASE_CREATE	= null;

	String[] mColumns = new String[] {	KEY_ROWID, KEY_INDEX_SERVICE, KEY_CHANNEL_TYPE, KEY_CHANNEL_NAME,
										KEY_UTC_START, KEY_UTC_END,
										KEY_REPEAT, KEY_ALARM_TYPE,
										KEY_COUNT};

	private ScheduleDBHelpe mDbHelper;
	private static SQLiteDatabase mDb	= null;

	private final Context mCtx;
	public ScheduleDB(Context ctx)
	{
		Log(I, "ScheduleDB()");
		
		this.mCtx = ctx;
	}

	public ScheduleDB open() throws SQLException
	{
		Log(I, "open()");
		
		createDataBase();
		
		mDbHelper = new ScheduleDBHelpe(mCtx);
		mDb = mDbHelper.getWritableDatabase();
		
		return this;
	}
	
	private void createDataBase()
	{
		Log(I, "createDataBase()");
		
		DATABASE_CREATE	=		"create table if not exists "
							+	DATABASE_TABLE
							+	"(_id integer primary key, "
							+	KEY_INDEX_SERVICE	+ " integer, "
							+	KEY_CHANNEL_TYPE	+ " integer, "
							+	KEY_CHANNEL_NAME	+ " text, "
							+	KEY_UTC_START		+ " long, "
							+	KEY_UTC_END			+ " long, "
							+	KEY_REPEAT			+ " integer, "
							+	KEY_ALARM_TYPE		+ " integer, "
							+	KEY_COUNT			+ " integer)";
	}

	public void close()
	{
		Log(I, "close()");
		
		if(mDb != null)
		{
			mDb.close();
			mDbHelper.close();
			
			mDb	= null;
		}
	}
	
	public long insert(ScheduleData data)
	{
		Log(I, "insert()");
		
		long	return_state	= -1;
		
		if(mDb != null)
		{
			ContentValues values	= new ContentValues();
			if(values != null)
			{
				values.put(KEY_INDEX_SERVICE, data.iIndexService);
				
				values.put(KEY_CHANNEL_TYPE, data.iChannelType);
				values.put(KEY_CHANNEL_NAME, data.sChannelName);
				
				values.put(KEY_UTC_START, data.lUTC_Start);
				values.put(KEY_UTC_END, data.lUTC_End);
				
				values.put(KEY_REPEAT, data.iRepeatType);
				values.put(KEY_ALARM_TYPE, data.iAlarmType);
				
				values.put(KEY_COUNT, 0);
			}
			return_state = mDb.insert(DATABASE_TABLE, null, values);	// Returns : the row ID of the newly inserted row, or -1 if an error occurred 
		}

		return return_state;
	}
	
	public int update(int iID, ScheduleData data)
	{
		Log(I, "update(iID=" + iID + ")");
		
		int	return_state	= -1;
		
		if(mDb != null)
		{
			ContentValues values	= new ContentValues();
			if(values != null)
			{
				values.put(KEY_INDEX_SERVICE, data.iIndexService);
				
				values.put(KEY_CHANNEL_TYPE, data.iChannelType);
				values.put(KEY_CHANNEL_NAME, data.sChannelName);
				
				values.put(KEY_UTC_START, data.lUTC_Start);
				values.put(KEY_UTC_END, data.lUTC_End);
				
				values.put(KEY_REPEAT, data.iRepeatType);
				values.put(KEY_ALARM_TYPE, data.iAlarmType);
				
				values.put(KEY_COUNT, 0);
			}
			return_state = mDb.update(DATABASE_TABLE, values, KEY_ROWID + "=" + iID, null);	// Returns : the number of rows affected 
		}

		return return_state;
	}
	
	public int updateRepeatCount(int iID, int iCount)
	{
		Log(I, "updateRepeatCount(iID=" + iID + ", iCount=" + iCount + ")");
	
		int return_state	= -1;
		if(mDb != null)
		{
			ContentValues values	= new ContentValues();
			if(values != null)
			{
				values.put(KEY_COUNT, iCount);
			}
			return_state = mDb.update(DATABASE_TABLE, values, KEY_ROWID + "=" + iID, null);
		}
		
		return return_state;
	}
	
	public ScheduleData getData(int iID)
	{
		Log(I, "getData(iID=" + iID + ")");
		
		ScheduleData	return_Data	= null;
		if(mDb != null)
		{
			Cursor cur	= mDb.query(DATABASE_TABLE, mColumns, KEY_ROWID + "=" + iID, null, null, null, null);
			if(cur != null)
			{
				setColumnIndex(cur);
				if(cur.getCount() == 1)
				{
					cur.moveToFirst();
					
					return_Data	= new ScheduleData();
					
					return_Data.iIndexService	= cur.getInt(iColumnIndex_IndexService);
					
					return_Data.iChannelType	= cur.getInt(iColumnIndex_ChannelType);
					return_Data.sChannelName	= cur.getString(iColumnIndex_Name);
					
					return_Data.lUTC_Start		= cur.getLong(iColumnIndex_UTC_Start);
					return_Data.lUTC_End		= cur.getLong(iColumnIndex_UTC_End);
					
					return_Data.iRepeatType		= cur.getInt(iColumnIndex_Repeat);
					return_Data.iAlarmType		= cur.getInt(iColumnIndex_AlarmType);
					
					return_Data.iCount			= cur.getInt(iColumnIndex_Count);
				}
				
				cur.close();
			}
		}
		
		return return_Data;
	}
	
	public long delete(int iID)
	{
		Log(I, "delete(iID=" + iID + ")");
		
		long	return_state	= -1;
		if(mDb != null)
		{
			return_state	= mDb.delete(DATABASE_TABLE, KEY_ROWID + "=" + iID, null);
		}

		return return_state;
	}
	
	public Cursor getSchedule()
	{
		Log(I, "getSchedule()");
		
		Cursor return_cursor	= null;
		
		if(mDb != null)
		{
			return_cursor	= mDb.query(DATABASE_TABLE, mColumns, null, null, null, null, 
										KEY_UTC_START + "% (24*60*60*10000)"
										);
			setColumnIndex(return_cursor);
		}
		else
		{
			Log(D, "mDB == null");
		}
		
		return return_cursor;
	}
	
	private void setColumnIndex(Cursor c)
	{
		Log(I, "setColumnIndex(c=" + c + ")");
		
		if( (c != null) && (iColumnIndex_ID == -1) )
		{
			iColumnIndex_ID				= c.getColumnIndexOrThrow(KEY_ROWID);
			
			iColumnIndex_IndexService	= c.getColumnIndexOrThrow(KEY_INDEX_SERVICE);
			iColumnIndex_ChannelType	= c.getColumnIndexOrThrow(KEY_CHANNEL_TYPE);
			iColumnIndex_Name			= c.getColumnIndexOrThrow(KEY_CHANNEL_NAME);
			
			iColumnIndex_UTC_Start		= c.getColumnIndexOrThrow(KEY_UTC_START);
			iColumnIndex_UTC_End		= c.getColumnIndexOrThrow(KEY_UTC_END);
			
			iColumnIndex_Repeat			= c.getColumnIndexOrThrow(KEY_REPEAT);
			iColumnIndex_AlarmType		= c.getColumnIndexOrThrow(KEY_ALARM_TYPE);
			
			iColumnIndex_Count			= c.getColumnIndexOrThrow(KEY_COUNT);
		}
	}
	
	private class ScheduleDBHelpe  extends SQLiteOpenHelper
	{
		ScheduleDBHelpe (Context context)
		{
			super(context, DATABASE_NAME, null, DATABASE_VERSION);
		}

		@Override
		public void onCreate(SQLiteDatabase database)
		{
			Log(I, "onCreate()");
			
			database.execSQL(DATABASE_CREATE);
		}

		@Override
		public void onUpgrade(SQLiteDatabase database, int oldVersion, int newVersion)
		{
			Log(I, "onUpgrade()");
			
			database.execSQL("DROP TABLE IF EXISTS services");
			onCreate(database);
		}
	}
	
	final int D = 0;
	final int E = 1;
	final int I = 2;
	final int V = 3;
	final int W = 4;
	
	private void Log(int level, String mString)
	{
		if(Scheduler.TAG == null)
		{
			return;
		}
		
		String TAG = "[[[ScheduleDB]]] ";
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