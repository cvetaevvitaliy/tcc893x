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

public class ChannelManager {
	public static final String KEY_ENSEMBLE_NAME = "ensembleName";
	public static final String KEY_ENSEMBLE_ID = "ensembleID";
	public static final String KEY_ENSEMBLE_FREQ = "ensembleFreq";
	public static final String KEY_SERVICE_NAME = "serviceName";
	public static final String KEY_SERVICE_ID = "serviceID";
	public static final String KEY_CHANNEL_NAME = "channelName";
	public static final String KEY_CHANNEL_ID = "channelID";
	public static final String KEY_TYPE = "type";
	public static final String KEY_BITRATE = "bitrate";
	public static final String KEY_REG_0 = "reg0";
	public static final String KEY_REG_1 = "reg1";
	public static final String KEY_REG_2 = "reg2";
	public static final String KEY_REG_3 = "reg3";
	public static final String KEY_REG_4 = "reg4";
	public static final String KEY_REG_5 = "reg5";
	public static final String KEY_REG_6 = "reg6";

	public static final String KEY_ROWID = "_id";

	public static final int	SERVICE_TYPE_DTV	= 0x01;
	public static final int	SERVICE_TYPE_MHDTV	= 0x11;
	public static final int	SERVICE_TYPE_ASDTV	= 0x16;
	public static final int	SERVICE_TYPE_AHDTV	= 0x19;
	
	public static final int	SERVICE_TYPE_DRADIO		= 0x02;
	public static final int	SERVICE_TYPE_FMRADIO	= 0x07;
	public static final int	SERVICE_TYPE_ADRADIO	= 0x0A;

	private ChannelDbAdapter mChannelDbAdapter;
	private SQLiteDatabase mSQLiteDatabase;

	private static final String DATABASE_NAME = "TDMB.db";
	private static final String DATABASE_TABLE = "channels";
	private static final int DATABASE_VERSION = 2;

	private static final String DATABASE_CREATE =
//		"create table channels (_id integer primary key autoincrement, "
		"create table channels (_id integer primary key, "
			+ "ensembleName text not null, ensembleID integer, ensembleFreq integer, serviceName text not null, serviceID integer, channelName text, channelID integer, type integer, bitrate integer, reg0 integer, reg1 integer, reg2 integer, reg3 integer, reg4 integer, reg5 integer, reg6 integer);";

	private final Context mContext;
	private static class ChannelDbAdapter  extends SQLiteOpenHelper {

		ChannelDbAdapter (Context context) {
			// 
			super(context, DATABASE_NAME, null, DATABASE_VERSION);
		}

		@Override
		public void onCreate(SQLiteDatabase database) {
			database.execSQL(DATABASE_CREATE);
		}

		@Override
		public void onUpgrade(SQLiteDatabase database, int oldVersion, int newVersion) {
			database.execSQL("DROP TABLE IF EXISTS channels");
			onCreate(database);
		}
	}

	public ChannelManager(Context context) {
		mContext= context;
	}

	public ChannelManager open() throws SQLException {
		mChannelDbAdapter= new ChannelDbAdapter (mContext);
		mSQLiteDatabase= mChannelDbAdapter.getWritableDatabase();
		return this;
	}

	public void close()
	{
		mChannelDbAdapter.close();
	}

	public long insertChannel(String ensembleName, int ensembleID, int ensembleFreq, String serviceName, int serviceID, String channelName, int channelID, int type, int bitrate, int reg0, int reg1, int reg2, int reg3, int reg4, int reg5, int reg6) {
		
		Log.d("      #############################################3","                               @@@@@@@@@@@@@@@@@@");
		ContentValues values= new ContentValues();
		values.put(KEY_ENSEMBLE_NAME, ensembleName);
		values.put(KEY_ENSEMBLE_ID, ensembleID);
		values.put(KEY_ENSEMBLE_FREQ, ensembleFreq);
		values.put(KEY_SERVICE_NAME, serviceName.toUpperCase());
		values.put(KEY_SERVICE_ID, serviceID);
		values.put(KEY_CHANNEL_NAME, channelName.toUpperCase());
		values.put(KEY_CHANNEL_ID, channelID);
		values.put(KEY_TYPE, type);
		values.put(KEY_BITRATE, bitrate);
		values.put(KEY_REG_0, reg0);
		values.put(KEY_REG_1, reg1);
		values.put(KEY_REG_2, reg2);
		values.put(KEY_REG_3, reg3);
		values.put(KEY_REG_4, reg4);
		values.put(KEY_REG_5, reg5);
		values.put(KEY_REG_6, reg6);

		return mSQLiteDatabase.insert(DATABASE_TABLE, null, values);
	}

	public boolean updateChannel(long rowId, String ensembleName, int ensembleID, int ensembleFreq, String serviceName, int serviceID, String channelName, int channelID, int type, int bitrate, int reg0, int reg1, int reg2, int reg3, int reg4, int reg5, int reg6) {
		ContentValues values= new ContentValues();
		values.put(KEY_ENSEMBLE_NAME, ensembleName);
		values.put(KEY_ENSEMBLE_ID, ensembleID);
		values.put(KEY_ENSEMBLE_FREQ, ensembleFreq);
		values.put(KEY_SERVICE_NAME, serviceName.toUpperCase());
		values.put(KEY_SERVICE_ID, serviceID);
		values.put(KEY_CHANNEL_NAME, channelName.toUpperCase());
		values.put(KEY_CHANNEL_ID, channelID);
		values.put(KEY_TYPE, type);
		values.put(KEY_BITRATE, bitrate);
		values.put(KEY_REG_0, reg0);
		values.put(KEY_REG_1, reg1);
		values.put(KEY_REG_2, reg2);
		values.put(KEY_REG_3, reg3);
		values.put(KEY_REG_4, reg4);
		values.put(KEY_REG_5, reg5);
		values.put(KEY_REG_6, reg6);

		return mSQLiteDatabase.update(DATABASE_TABLE, values, KEY_ROWID + "=" + rowId, null) > 0;
	}

	public boolean deleteChannel(long rowId) {
		return mSQLiteDatabase.delete(DATABASE_TABLE, KEY_ROWID + "=" + rowId, null) > 0;
	}

	public boolean deleteAllChannel() {
		return mSQLiteDatabase.delete(DATABASE_TABLE, null, null) > 0;
	}

	public Cursor getAllChannels() {
		return mSQLiteDatabase.query(DATABASE_TABLE,
			new String[] {
							KEY_ROWID,
							KEY_ENSEMBLE_NAME,
							KEY_ENSEMBLE_ID,
							KEY_ENSEMBLE_FREQ,
							KEY_SERVICE_NAME,
							KEY_SERVICE_ID,
							KEY_CHANNEL_NAME,
							KEY_CHANNEL_ID,
							KEY_TYPE,
							KEY_BITRATE,
							KEY_REG_0,
							KEY_REG_1,
							KEY_REG_2,
							KEY_REG_3,
							KEY_REG_4,
							KEY_REG_5,
							KEY_REG_6
							},
				null, null, null, null, KEY_SERVICE_NAME+" ASC");
	}

	public Cursor getTypeChannels(int nType) throws SQLException {
		return mSQLiteDatabase.query(DATABASE_TABLE,
			new String[] {
							KEY_ROWID,
							KEY_ENSEMBLE_NAME,
							KEY_ENSEMBLE_ID,
							KEY_ENSEMBLE_FREQ,
							KEY_SERVICE_NAME,
							KEY_SERVICE_ID,
							KEY_CHANNEL_NAME,
							KEY_CHANNEL_ID,
							KEY_TYPE,
							KEY_BITRATE,
							KEY_REG_0,
							KEY_REG_1,
							KEY_REG_2,
							KEY_REG_3,
							KEY_REG_4,
							KEY_REG_5,
							KEY_REG_6
							},
				KEY_TYPE + "=" + nType, null, null, null, KEY_SERVICE_NAME+" ASC");
	}

	public Cursor getChannel(long rowId) throws SQLException {
		Cursor cursor = mSQLiteDatabase.query(true, DATABASE_TABLE,
			new String[] {
							KEY_ROWID,
							KEY_ENSEMBLE_NAME,
							KEY_ENSEMBLE_ID,
							KEY_ENSEMBLE_FREQ,
							KEY_SERVICE_NAME,
							KEY_SERVICE_ID,
							KEY_CHANNEL_NAME,
							KEY_CHANNEL_ID,
							KEY_TYPE,
							KEY_BITRATE,
							KEY_REG_0,
							KEY_REG_1,
							KEY_REG_2,
							KEY_REG_3,
							KEY_REG_4,
							KEY_REG_5,
							KEY_REG_6
							},
				KEY_ROWID + "=" + rowId, null, null, null, null, null);
		if (cursor != null) {
			cursor.moveToFirst();
		}
		return cursor;
	}  
}

