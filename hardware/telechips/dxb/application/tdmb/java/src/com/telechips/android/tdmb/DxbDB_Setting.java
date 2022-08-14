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

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

public class DxbDB_Setting
{
	private static final String TAG = "[[[DxbDB_Setting]]] ";

	public static final String KEY_POSITION_TV		= "position_tv";
	public static final String KEY_POSITION_RADIO	= "position_radio";
	public static final String KEY_PRESET			= "preset";
	public static final String KEY_AREA				= "area";
	public static final String KEY_AGE				= "age";
	
	public static final String KEY_ROWID = "_id";

	private DatabaseHelper mDbHelper;
	private static SQLiteDatabase mDb;

	private static final String DATABASE_NAME = "DxbSetting.db";
	private static final String DATABASE_SETTINGTABLE = "dxbsetting";
	private static final int DATABASE_VERSION = 1;

	private static final String DATABASE_FEEDTABLE_CREATE =		"create table "
															+	DATABASE_SETTINGTABLE
															+	" (_id integer primary key autoincrement, "
															+	KEY_POSITION_TV		+ " integer, "
															+	KEY_POSITION_RADIO	+ " integer, "
															+	KEY_PRESET			+ " integer, "
															+	KEY_AREA			+ " integer, "
															+	KEY_AGE				+ " integer)";

	String[] mColumns = new String[] { KEY_ROWID, KEY_POSITION_TV, KEY_POSITION_RADIO, KEY_PRESET, KEY_AREA, KEY_AGE};

	private final Context mCtx;

	private class DatabaseHelper extends SQLiteOpenHelper
	{
		public DatabaseHelper(Context context)
		{
			super(context, DATABASE_NAME, null, DATABASE_VERSION);
		}

		public void onCreate(SQLiteDatabase db)
		{
			db.execSQL(DATABASE_FEEDTABLE_CREATE);
			loadSettings(db);
		}

		public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion)
		{
			db.execSQL("DROP TABLE IF EXISTS " + DATABASE_SETTINGTABLE);
			onCreate(db);
		}
	}

	public DxbDB_Setting(Context ctx)
	{
		this.mCtx = ctx;
	}

	public DxbDB_Setting open() throws SQLException
	{
		mDbHelper = new DatabaseHelper(mCtx);
		mDb = mDbHelper.getWritableDatabase();
		
		return this;
	}

	public void close()
	{
		mDb.close();
		mDbHelper.close();
	}
	
	public void loadSettings(SQLiteDatabase db)
	{
		Log.i(TAG, "loadSettings()");
		ContentValues cv = new ContentValues();
		
		cv.put(KEY_POSITION_TV, 0);
		cv.put(KEY_POSITION_RADIO, 0);
		cv.put(KEY_PRESET, Manager_Setting.DEFAULT_VALUE_PRESET);
		cv.put(KEY_AREA, Manager_Setting.DEFAULT_VALUE_AREA_CODE);
		cv.put(KEY_AGE, Manager_Setting.DEFAULT_VALUE_PARENTAL_RATING);
		
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

	public void putPosition(int type, int position)
	{
		ContentValues cv = new ContentValues();

		switch(type)
		{
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
	
	public int getPreset()
	{
		int	preset;
		Log.i(TAG, "getPreset()");
		
		Cursor cursor = mDb.query(DATABASE_SETTINGTABLE, mColumns, KEY_ROWID + "=" + 1, null, null, null, null, null);
		if (cursor == null || cursor.getCount() <= 0)
		{
			if (cursor != null)
			{
				cursor.close();
			}
			
			return Manager_Setting.DEFAULT_VALUE_PRESET;
		}
		
		cursor.moveToFirst();
		preset = cursor.getInt(3);
		cursor.close();

		return preset;
	}
	
	public void putPreset(int preset)
	{
		ContentValues cv = new ContentValues();
		cv.put(KEY_PRESET, preset);
		mDb.update(DATABASE_SETTINGTABLE, cv, KEY_ROWID + "=" + 1, null);
	}
	
	public int getArea()
	{
		int	area;
		Log.i(TAG, "getArea()");
		
		Cursor cursor = mDb.query(DATABASE_SETTINGTABLE, mColumns, KEY_ROWID + "=" + 1, null, null, null, null, null);
		if (cursor == null || cursor.getCount() <= 0)
		{
			if (cursor != null)
			{
				cursor.close();
			}
			
			return Manager_Setting.DEFAULT_VALUE_AREA_CODE;
		}
		
		cursor.moveToFirst();
		area = cursor.getInt(4);
		cursor.close();

		return area;
	}
	
	public void putArea(int areaCode)
	{
		ContentValues cv = new ContentValues();
		cv.put(KEY_AREA, areaCode);
		mDb.update(DATABASE_SETTINGTABLE, cv, KEY_ROWID + "=" + 1, null);
	}
	
	public int getAge()
	{
		int	age;
		Log.i(TAG, "getAge()");
		
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
		
		age = cursor.getInt(5);
		cursor.close();
		
		return age;
	}
	
	public static void putAge(int age)
	{
		ContentValues cv = new ContentValues();
		cv.put(KEY_AGE, age);
		mDb.update(DATABASE_SETTINGTABLE, cv, KEY_ROWID + "=" + 1, null);
	}
}
