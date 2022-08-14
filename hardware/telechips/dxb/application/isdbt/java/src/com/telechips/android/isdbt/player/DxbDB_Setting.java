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
package com.telechips.android.isdbt.player;

import com.telechips.android.isdbt.*;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

public class DxbDB_Setting {

	private static final String TAG = "[[[DxbDB_Setting]]] ";

	private static final String KEY_POSITION_TV     = "position_tv";
	private static final String KEY_POSITION_RADIO  = "position_radio";
	private static final String KEY_TIME_ZONE       = "time_zone";
	private static final String KEY_PASSWORD        = "password";
	private static final String KEY_AGE             = "age";
	private static final String KEY_SCAN			= "scan";
	private static final String KEY_AREA_0			= "area_0";
	private static final String KEY_AREA_1			= "area_1";

	private static final String KEY_ROWID = "_id";

	private static final String DATABASE_NAME			= "DxbSetting.db";
	private static final String DATABASE_SETTINGTABLE	= "dxbsetting";

	private static final int DATABASE_VERSION = 1;

	private static final String DATABASE_FEEDTABLE_CREATE =		"create table if not exists "
															+	DATABASE_SETTINGTABLE
															+	" (_id integer primary key autoincrement, "
															+	KEY_POSITION_TV		+ " integer, "
															+	KEY_POSITION_RADIO	+ " integer, "
															+	KEY_TIME_ZONE		+ " integer, "
															+	KEY_PASSWORD		+ " text, "
															+	KEY_AGE				+ " integer, "
															+   KEY_SCAN            + " integer, "
															+   KEY_AREA_0          + " integer, "
	 														+   KEY_AREA_1          + " integer);";


	String[] mColumns = new String[] { KEY_ROWID, KEY_POSITION_TV, KEY_POSITION_RADIO, KEY_TIME_ZONE, KEY_PASSWORD, KEY_AGE, KEY_SCAN, KEY_AREA_0, KEY_AREA_1 };

	private final Context mCtx;
	private DatabaseHelper mDbHelper;
	private SQLiteDatabase mDb;

	private class DatabaseHelper extends SQLiteOpenHelper {
		public DatabaseHelper(Context context) {
			super(context, DATABASE_NAME, null, DATABASE_VERSION);
		}

        @Override
		public void onCreate(SQLiteDatabase db) {
			db.execSQL(DATABASE_FEEDTABLE_CREATE);
			loadSettings(db);
		}

        @Override
		public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
			db.execSQL("DROP TABLE IF EXISTS " + DATABASE_SETTINGTABLE);
			onCreate(db);
		}
	}

	public DxbDB_Setting(Context ctx) {
		this.mCtx = ctx;
	}

	public boolean open() throws SQLException {
		close();

		mDbHelper = new DatabaseHelper(mCtx);
		mDb = mDbHelper.getWritableDatabase();

		return true;
	}

	public void close() {
		if(mDb != null)
		{
			mDb.close();
			mDb = null;
		}

		if(mDbHelper != null)
		{
			mDbHelper.close();
			mDbHelper = null;	
		}
	}

	protected DxbPlayer getPlayer()
	{
		ISDBTPlayerActivity	mContext	= (ISDBTPlayerActivity)mCtx;
		
		return mContext.getPlayer();
	}
	
	public void loadSettings(SQLiteDatabase db)
	{
		Log.i(TAG, "loadSettings()");

		ContentValues cv = new ContentValues();
		
		cv.put(KEY_POSITION_TV, 0);
		cv.put(KEY_POSITION_RADIO, 0);
		cv.put(KEY_TIME_ZONE, 0);
		cv.put(KEY_PASSWORD, getPlayer().DEFAULT_VALUE_PASSWORD);
		cv.put(KEY_AGE, getPlayer().DEFAULT_VALUE_AGE);
		cv.put(KEY_SCAN, getPlayer().DEFAULT_VALUE_SCAN);		
		cv.put(KEY_AREA_0, getPlayer().DEFAULT_VALUE_AREA_0);
		cv.put(KEY_AREA_1, getPlayer().DEFAULT_VALUE_AREA_1);
		
		db.insert(DATABASE_SETTINGTABLE, null, cv);
	}
	public int getPosition(int type)
	{
		int	position;
		Cursor cursor = mDb.query(DATABASE_SETTINGTABLE, mColumns, KEY_ROWID + "=" + 1, null, null, null, null, null);
		
		if (cursor == null || cursor.getCount() <= 0)
		{
			if(cursor != null)
			{
				cursor.close();
			}
			
			return 0;
		}
		
		cursor.moveToFirst();
		switch(type)
		{
			case 1:
				position	= cursor.getInt(2);
				cursor.close();
			return position;

			case 0:
			default:
				position	= cursor.getInt(1);
				cursor.close();
			return position;
		}		
	}

	public void putPosition(int type, int position) {
		ContentValues cv = new ContentValues();
		switch(type) {
		case 1:
			cv.put(KEY_POSITION_RADIO, position);
			break;

		case 0:
		default:
			cv.put(KEY_POSITION_TV, position);
			break;
		}
		mDb.update(DATABASE_SETTINGTABLE, cv, KEY_ROWID + "=" + 1, null);
	}
	

	public int getTimeZone()
	{
		int time_zone;
		Log.i(TAG, "getTimeZone()");
		
		Cursor cursor = mDb.query(DATABASE_SETTINGTABLE, mColumns, KEY_ROWID + "=" + 1, null, null, null, null, null);
		if (cursor == null || cursor.getCount() <= 0)
		{
			if (cursor != null)
			{
				cursor.close();
			}

			return getPlayer().DEFAULT_VALUE_TIME_ZONE_INDEX;
		}

		cursor.moveToFirst();
		time_zone = cursor.getInt(3);
		cursor.close();

		return time_zone;
	}


	public void putTimeZone(int time_zone)
	{
		ContentValues cv = new ContentValues();
		cv.put(KEY_TIME_ZONE, time_zone);
		mDb.update(DATABASE_SETTINGTABLE, cv, KEY_ROWID + "=" + 1, null);
	}
	
	public String getPassword()
	{
		Log.i(TAG, "getPassword()");
	
		String	password;
		Cursor cursor = mDb.query(DATABASE_SETTINGTABLE, mColumns, KEY_ROWID + "=" + 1, null, null, null, null, null);
		if (cursor == null || cursor.getCount() <= 0)
		{
			if (cursor != null)
			{
				cursor.close();
			}
			
			return getPlayer().DEFAULT_VALUE_PASSWORD;
		}
		
		cursor.moveToFirst();
		password = cursor.getString(4);
		cursor.close();
		
		Log.d(TAG, "password = " + password);
		
		return password;
	}
	
	public void putPassword(String newPassword)
	{
		ContentValues cv = new ContentValues();
		Log.d(TAG, "newPassword = " + newPassword);
		cv.put(KEY_PASSWORD, newPassword);
		mDb.update(DATABASE_SETTINGTABLE, cv, KEY_ROWID + "=" + 1, null);
	}
	
	public int getAge() {
		Log.i(TAG, "getAge()");
		int	age;
		Cursor cursor = mDb.query(DATABASE_SETTINGTABLE, mColumns, KEY_ROWID + "=" + 1, null, null, null, null, null);
		if (cursor == null || cursor.getCount() <= 0) {
			if (cursor != null) {
				cursor.close();
			}
			return -1;
		}
		cursor.moveToFirst();
		age = cursor.getInt(5);
		cursor.close();
		return age;
	}
	
	public void putAge(int age) {
		ContentValues cv = new ContentValues();
		cv.put(KEY_AGE, age);
		mDb.update(DATABASE_SETTINGTABLE, cv, KEY_ROWID + "=" + 1, null);
	}
	
	public int getScan() {
		Log.i(TAG, "getScan()");
		int	scan;
		Cursor cursor = mDb.query(DATABASE_SETTINGTABLE, mColumns, KEY_ROWID + "=" + 1, null, null, null, null, null);
		if (cursor == null || cursor.getCount() <= 0) {
			if (cursor != null) {
				cursor.close();
			}
			return -1;
		}
		cursor.moveToFirst();
		scan = cursor.getInt(6);
		cursor.close();
		return scan;
	}
	
	public void putScan(int scan) {
		ContentValues cv = new ContentValues();
		cv.put(KEY_SCAN, scan);
		mDb.update(DATABASE_SETTINGTABLE, cv, KEY_ROWID + "=" + 1, null);
	}
	
	public int getArea_0()
	{
		int	area;
		Log.i(TAG, "getArea_0()");
		
		Cursor cursor = mDb.query(DATABASE_SETTINGTABLE, mColumns, KEY_ROWID + "=" + 1, null, null, null, null, null);
		if (cursor == null || cursor.getCount() <= 0)
		{
			if (cursor != null)
			{
				cursor.close();
			}
			
			return -1;
		}
		
		cursor.moveToFirst();
		area = cursor.getInt(7);
		cursor.close();

		return area;
	}
	
	public void putArea_0(int areaCode)
	{
		ContentValues cv = new ContentValues();
		cv.put(KEY_AREA_0, areaCode);
		mDb.update(DATABASE_SETTINGTABLE, cv, KEY_ROWID + "=" + 1, null);
	}
	
	public int getArea_1()
	{
		int	area;
		Log.i(TAG, "getArea_1()");
		
		Cursor cursor = mDb.query(DATABASE_SETTINGTABLE, mColumns, KEY_ROWID + "=" + 1, null, null, null, null, null);
		if (cursor == null || cursor.getCount() <= 0)
		{
			if (cursor != null)
			{
				cursor.close();
			}
			
			return -1;
		}
		
		cursor.moveToFirst();
		area = cursor.getInt(8);
		cursor.close();

		return area;
	}
	
	public void putArea_1(int areaCode)
	{
		ContentValues cv = new ContentValues();
		cv.put(KEY_AREA_1, areaCode);
		mDb.update(DATABASE_SETTINGTABLE, cv, KEY_ROWID + "=" + 1, null);
	}
}
