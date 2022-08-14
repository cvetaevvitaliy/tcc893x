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
package com.telechips.android.iptv.samples;

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

	public static final String KEY_IP_NUMBER	= "ip_number";
	public static final String KEY_TEMP1		= "temp1";
	public static final String KEY_TEMP2		= "temp2";
	public static final String KEY_TEMP3		= "temp3";
	
	public static final String KEY_ROWID = "_id";

	private DatabaseHelper mDbHelper;
	private static SQLiteDatabase mDb;

	private static final String DATABASE_NAME = "DxbSetting.db";
	private static final String DATABASE_SETTINGTABLE = "dxbsetting";
	private static final int DATABASE_VERSION = 1;

	private static final String DATABASE_FEEDTABLE_CREATE =		"create table "
															+	DATABASE_SETTINGTABLE
															+	" (_id integer primary key autoincrement, "
															+	KEY_IP_NUMBER		+ " integer, "
															+	KEY_TEMP1	+ " integer, "
															+	KEY_TEMP2			+ " integer, "
															+	KEY_TEMP3				+ " integer)";

	String[] mColumns = new String[] { KEY_ROWID, KEY_IP_NUMBER, KEY_TEMP1, KEY_TEMP2, KEY_TEMP3};

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
		mDbHelper.close();
	}
	
	public void loadSettings(SQLiteDatabase db)
	{
		Log.i(TAG, "loadSettings()");
		ContentValues cv = new ContentValues();
		
		cv.put(KEY_IP_NUMBER, 0);
		cv.put(KEY_TEMP1, 0);
		cv.put(KEY_TEMP2, 0);
		cv.put(KEY_TEMP3, 0);
		
		db.insert(DATABASE_SETTINGTABLE, null, cv);
	}
	
	public int getIPNumber()
	{
		Log.i(TAG, "getIPNumber()");
		
		Cursor cursor = mDb.query(DATABASE_SETTINGTABLE, mColumns, KEY_ROWID + "=" + 1, null, null, null, null, null);
		if (cursor == null || cursor.getCount() <= 0)
		{
			return 0;
		}
		
		cursor.moveToFirst();
		
		return cursor.getInt(1);
	}

	public void putIPNumber(int number)
	{
		Log.i(TAG, "putIPNumber(number"+number+")");
		
		ContentValues cv = new ContentValues();

		cv.put(KEY_IP_NUMBER, number);
		
		mDb.update(DATABASE_SETTINGTABLE, cv, KEY_ROWID + "=" + 1, null);
	}
}
