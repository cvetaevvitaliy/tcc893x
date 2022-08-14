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

package com.telechips.android.dvb.player.diseqc;

import com.telechips.android.dvb.DVBPlayerActivity;
import com.telechips.android.dvb.player.DxbPlayer;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

public class DishSetupManager extends SQLiteOpenHelper {

	private DVBPlayerActivity getMainActivity() {
		return DVBPlayerActivity.getInstance();
	}
	
	private DxbPlayer getPlayer() {
		return getMainActivity().getPlayer();
	}
	
	private static final String DATABASE_NAME = "sat_transponder.db";
	private static final int DATABASE_VERSION = 2;

	public static final String DB_TABLE_SAT_INFO = "sat_info";
	public static final String DB_TABLE_TS_INFO  = "ts_info";
	public static final String DB_TABLE_MOTO_POSITION	= "moto_position";
	private static final String CREATE_SAT_TABLE = "CREATE TABLE IF NOT EXISTS "
			+ DB_TABLE_SAT_INFO
			+ "( _id integer primary key"
//			+ ", sat_id integer"
			+ ", sat_name text"
			+ ", sat_longitude integer"
			+ ", lnb_no integer"
			+ ", lnb_type integer"
			+ ", lnb_lof_lo integer"
			+ ", lnb_lof_hi integer"
			+ ", lnb_lof_th integer"
			+ ", lnb_power integer"
			+ ", lnb_tone integer"
			+ ", lnb_toneburst integer"
			+ ", lnb_diseqc_mode integer"
			+ ", lnb_diseqc_config10 integer"
			+ ", lnb_diseqc_config11 integer"
			+ ", lnb_cmd_sequence integer"
			+ ", motor_diseqc_mode integer"
			+ ", motor_id integer"
			+ ", motor_position integer"
			+ ", diseqc_repeat integer"
			+ ", diseqc_fast integer"
			+ ")";

	private static final String CREATE_TS_TABLE = "CREATE TABLE IF NOT EXISTS "
			+ DB_TABLE_TS_INFO
			+ "( _id integer primary key"
			+ ", ts_id integer"
			+ ", sat_id integer"
			+ ", frequency integer"
			+ ", symbol integer"
			+ ", polarisation integer"
			+ ", fec_inner integer"
			+ ")";
	
	private static final String CREATE_MOTO_POSITION_TABLE	= "CREATE TABLE IF NOT EXISTS "
			+ DB_TABLE_MOTO_POSITION
			+ "( _id integer primary key"
			+ ", location integer"
			+ ", longitude_direction integer"
			+ ", longitude_angle integer"
			+ ", latitude_direction integer"
			+ ", latitude_angle integer"
			+ ")";

	private SQLiteDatabase mDB;
	private Tuner mTsH;
	private Tuner mTsV;

	public DishSetupManager(Context context) {
		super(context, DATABASE_NAME, null, DATABASE_VERSION);

		mTsH = new Tuner();
		mTsH.setId(1);
		//mTsH.setTsId(0);
		mTsH.setSatId(0);
		mTsH.setFrequency(0);
		mTsH.setSymbolRate(0);
		mTsH.setPolarization(Tuner.POL_H);
		mTsH.setFecInner(0);

		mTsV = new Tuner();
		mTsV.setId(2);
		//mTsV.setTsId(0);
		mTsV.setSatId(0);
		mTsV.setFrequency(0);
		mTsV.setSymbolRate(0);
		mTsV.setPolarization(Tuner.POL_V);
		mTsV.setFecInner(0);

		DxbLog(D, "DishSetupManager()");
	}

	@Override
	public void onCreate(SQLiteDatabase arg0) {
		DxbLog(D, "onCreate()");

		arg0.execSQL(CREATE_SAT_TABLE);
		arg0.execSQL(CREATE_TS_TABLE);
		arg0.execSQL(CREATE_MOTO_POSITION_TABLE);
	}

	@Override
	public void onUpgrade(SQLiteDatabase arg0, int arg1, int arg2) {
		DxbLog(D, "onUpgrade()");
	}

	public void open() {
		DxbLog(D, "open()");

		if (mDB != null)
			return;

		mDB = getWritableDatabase();
	}

	public void close() {
		DxbLog(D, "close()");

		if (mDB == null)
			return;

		synchronized(mDB) {
			mDB.close();
		}
		mDB = null;
	}

	public void insertSatInfo(ContentValues values) {
		DxbLog(D, "insertSatInfo()");

		SQLiteDatabase db = mDB;
		if (db != null) {
			synchronized(db) {
				if (db.isOpen()) {
					db.insert(DB_TABLE_SAT_INFO, null, values);
				}
			}
		}
	}
	
	public void insertTsInfo(ContentValues values) {
		DxbLog(D, "insertTsInfo()");

		SQLiteDatabase db = mDB;
		if (db != null) {
			synchronized(db) {
				if (db.isOpen()) {
					db.insert(DB_TABLE_TS_INFO, null, values);
				}
			}
		}
	}
	
	public void insertMotorLocation(ContentValues values)
	{
		DxbLog(D, "insertMotorLocation()");

		SQLiteDatabase db = mDB;
		if (db != null)
		{
			synchronized(db)
			{
				if (db.isOpen())
				{
					db.insert(DB_TABLE_MOTO_POSITION, null, values);
				}
			}
		}
	}
	
	public Cursor querySatInfo(String[] columns, String selection, String[] selectionArgs) {
		DxbLog(D, "querySatInfo()");

		SQLiteDatabase db = mDB;
		if (db != null) {
			synchronized(db) {
				if (db.isOpen()) {
					return db.query(DB_TABLE_SAT_INFO, columns, selection, selectionArgs, null, null, "_id asc");
				}
			}
		}
		return null;
	}

	public Cursor queryTsInfo(String[] columns, String selection, String[] selectionArgs) {
		DxbLog(D, "queryTsInfo()");

		SQLiteDatabase db = mDB;
		if (db != null) {
			synchronized(db) {
				if (db.isOpen()) {
					return db.query(DB_TABLE_TS_INFO, columns, selection, selectionArgs, null, null, "_id asc");
				}
			}
		}
		return null;
	}

	public Cursor queryTsInfo(String[] columns, String selection, String[] selectionArgs, String asc) {
		DxbLog(D, "queryTsInfo()");

		SQLiteDatabase db = mDB;
		if (db != null) {
			synchronized(db) {
				if (db.isOpen()) {
					return db.query(DB_TABLE_TS_INFO, columns, selection, selectionArgs, null, null, asc);
				}
			}
		}
		return null;
	}
	
	public Cursor queryMotorLocation(String[] columns)
	{
		DxbLog(D, "queryMotorLocation(" + columns + ")");
		
		SQLiteDatabase db	= mDB;
		if(db != null)
		{
			synchronized(db)
			{
				if(db.isOpen())
				{
					return db.query(DB_TABLE_MOTO_POSITION,  columns,  null, null, null, null, null);
				}
			}
		}
		
		return null;
	}

	public void deleteSatInfo(int _id) {
		DxbLog(D, "deleteSatInfo()");
		SQLiteDatabase db = mDB;
		
		db.delete(DB_TABLE_SAT_INFO, "_id=" + _id, null);
	}
	
	public void deleteTsInfo(int _id) {
		DxbLog(D, "deleteTsInfo()");
		SQLiteDatabase db = mDB;
		
		db.delete(DB_TABLE_TS_INFO, "_id=" + _id, null);
	}
	
	public void updateSatInfo(ContentValues values, int _id, Dish dish) {
		DxbLog(D, "updateSatInfo(" + _id + ")");

		SQLiteDatabase db = mDB;
		if (db != null) {
			synchronized(db) {
				if (db.isOpen()) {
					db.update(DB_TABLE_SAT_INFO, values, "_id=?", new String[]{String.valueOf(_id)});
				}
			}
		}
		
		getPlayer().ExecuteDiSEqC(null, dish);
	}
	
	public void updateTsInfo(ContentValues values, int _id, Dish dish) {
		DxbLog(D, "updateTsInfo(" + _id + ")");
		
		SQLiteDatabase db = mDB;
		if (db != null) {
			synchronized(db) {
				if (db.isOpen()) {
					db.update(DB_TABLE_TS_INFO, values, "_id=?", new String[]{String.valueOf(_id)});
				}
			}
		}
		
		getPlayer().ExecuteDiSEqC(null, dish);
	}
	
	public void updateMotorLocation(ContentValues values)
	{
		DxbLog(D, "updateMotorLocation()");
		
		SQLiteDatabase db	= mDB;
		if(db != null)
		{
			synchronized(db)
			{
				if(db.isOpen())
				{
					db.update(DB_TABLE_MOTO_POSITION, values, "_id=1", null);
				}
			}
		}
	}

	public Dish getDishSetup(int _id) {
		DxbLog(D, "getDishSetup(" + _id + ")");

		Cursor c = querySatInfo(
				new String[] {"_id"
//							, "sat_id"
							, "sat_name"
							, "sat_longitude"
							, "lnb_no"
							, "lnb_type"
							, "lnb_lof_lo"
							, "lnb_lof_hi"
							, "lnb_lof_th"
							, "lnb_power"
							, "lnb_tone"
							, "lnb_toneburst"
							, "lnb_diseqc_mode"
							, "lnb_diseqc_config10"
							, "lnb_diseqc_config11"
							, "lnb_cmd_sequence"
							, "motor_diseqc_mode"
							, "motor_id"
							, "motor_position"
							, "diseqc_repeat"
							, "diseqc_fast"}
				, "_id=" + _id, null);
		if (c == null)
		{
			return null;
		}

		SQLiteDatabase db = mDB;
		Dish dish = null;
		if (db != null) {
			synchronized(db) {
				if (db.isOpen()) {
					if (c.getCount() > 0) {
						dish = new Dish();
						c.moveToFirst();
						dish.setId(c.getInt(0));
//						dish.setSatId(c.getInt(0));
						dish.setSatName(c.getString(1));
						dish.setSatLongitude(c.getInt(2));
						dish.setLnbNo(c.getInt(3));
						dish.setLnbType(c.getInt(4));
						dish.setLnbLoLof(c.getInt(5));
						dish.setLnbHiLof(c.getInt(6));
						dish.setLnbLofThreshold(c.getInt(7));
						dish.setLnbPower(c.getInt(8));
						dish.setLnbTone(c.getInt(9));
						dish.setLnbToneburst(c.getInt(10));
						dish.setLnbDiseqcMode(c.getInt(11));
						dish.setLnbDiseqcConfig10(c.getInt(12));
						dish.setLnbDiseqcConfig11(c.getInt(13));
						dish.setLnbSequence(c.getInt(14));
						dish.setMotorDiseqcMode(c.getInt(15));
						dish.setMotorId(c.getInt(16));
						dish.setMotorPosition(c.getInt(17));
						dish.setDiseqcRepeat(c.getInt(18));
						dish.setFastDiseqc(c.getInt(19));
					}
				}
			}
			c.close();
		}
		return dish;
	}

	public Tuner[] getTsList(int _id) {
		DxbLog(D, "getTsList(" + _id + ")");

		Cursor c = queryTsInfo(
				//new String[] {"_id", "ts_id", "sat_id", "frequency", "symbol", "polarisation", "fec_inner"}
				new String[] {"_id", "sat_id", "frequency", "symbol", "polarisation", "fec_inner"}
				, "sat_id=" + _id
				, null
		);
		if (c == null)
		{
			return null;
		}

		SQLiteDatabase db = mDB;
		Tuner[] tsList = null;
		if (db != null) {
			synchronized(db) {
				if (db.isOpen()) {
					if (c.getCount() > 0) {
						c.moveToFirst();
						tsList = new Tuner[c.getCount()];
						for (int i = 0; i < c.getCount(); i++) {
							tsList[i] = new Tuner();
							tsList[i].setId(c.getInt(0));
							//tsList[i].setTsId(c.getInt(1));
							tsList[i].setSatId(c.getInt(1));
							tsList[i].setFrequency(c.getInt(2));
							tsList[i].setSymbolRate(c.getInt(3));
							tsList[i].setPolarization(c.getInt(4));
							tsList[i].setFecInner(c.getInt(5));
							c.moveToNext();
						}
					}
				}
			}
			c.close();
		}
		return tsList;
	}

	public Tuner[] getTsList(Dish dish) {
		DxbLog(D, "getTsList()");

		if (dish == null)
		{
			return null;
		}

		Tuner[] tsList;
		if (dish.getLnbPower() == Dish.LNB_POWER_13V) {
			tsList = new Tuner[1];
			tsList[0] = mTsV;
			return tsList;
		}
		if (dish.getLnbPower() == Dish.LNB_POWER_18V || dish.getLnbPower() == Dish.LNB_POWER_OFF) {
			tsList = new Tuner[1];
			tsList[0] = mTsH;
			return tsList;
		}
		tsList = new Tuner[2];
		tsList[0] = mTsH;
		tsList[1] = mTsV;
		return tsList;
	}
	
	public Motor getMotoLocation()
	{
		DxbLog(D, "getMotoLocation()");
		
		Cursor c = queryMotorLocation(
				new String[] {"_id"
							, "location"
							, "longitude_direction"
							, "longitude_angle"
							, "latitude_direction"
							, "latitude_angle"});
		if (c == null)
		{
			return null;
		}

		SQLiteDatabase db = mDB;
		Motor motor = null;
		if (db != null)
		{
			synchronized(db)
			{
				if (db.isOpen())
				{
					if (c.getCount() > 0)
					{		
						c.moveToFirst();

						motor	= new Motor();
						motor.setId(c.getInt(0));
						motor.setLocation(c.getInt(1));
						motor.setLongitudeDirection(c.getInt(2));
						motor.setLongitudeAngle(c.getInt(3));
						motor.setLatitudeDirection(c.getInt(4));
						motor.setLatitudeAngle(c.getInt(5));
					}
				}	
			}	
		}
		
		return motor;
	}


/*****************************************************************************************************************************************************************************
 *	Debug - log_message
 *****************************************************************************************************************************************************************************/

	private static final int D = 0;
	private static final int E = 1;
	private static final int I = 2;
	private static final int V = 3;
	private static final int W = 4;

	private void DxbLog(int level, String mString)
	{
		String TAG = "[[[DishSetupManager]]]";

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
					//Log.d(TAG, mString);
				break;
			}
		}
	}
}
