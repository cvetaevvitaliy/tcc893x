package com.telechips.android.atsc.samples;

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
	public static final String KEY_AREA				= "area";
	public static final String KEY_AGE				= "age";
	public static final String KEY_ASPECTRATIO		= "aspectratio";

	public static final String KEY_ROWID = "_id";

	private DatabaseHelper mDbHelper;
	private static SQLiteDatabase mDb;

	private static final String DATABASE_NAME = "DxbSetting.db";
	private static final String DATABASE_SETTINGTABLE = "dxbsetting";
	private static final int DATABASE_VERSION = 1;

	private static final String DATABASE_FEEDTABLE_CREATE =		"create table "
															+	DATABASE_SETTINGTABLE
															+	" (_id integer primary key autoincrement," 
															+	KEY_POSITION_TV		+ " integer, "
															+	KEY_POSITION_RADIO	+ " integer, "
															+	KEY_AREA			+ " integer, "
															+	KEY_AGE				+ " integer, "
															+	KEY_ASPECTRATIO		+ " integer)";

	String[] mColumns = new String[] { KEY_ROWID, KEY_POSITION_TV, KEY_POSITION_RADIO, KEY_AREA, KEY_AGE, KEY_ASPECTRATIO};

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
		Log.d(TAG, "loadSettings()");
		ContentValues cv = new ContentValues();
		
		cv.put(KEY_POSITION_TV, 0);
		cv.put(KEY_POSITION_RADIO, 0);
		cv.put(KEY_AREA, DxbPlayer_Control.DEFAULT_VALUE_AREA_CODE);
		cv.put(KEY_AGE, DxbPlayer_Control.DEFAULT_VALUE_PARENTAL_RATING);
		cv.put(KEY_ASPECTRATIO, 3);
		
		db.insert(DATABASE_SETTINGTABLE, null, cv);
	}
	
	public int getPosition(int type)
	{
		int	position = 0;
		Cursor cursor = mDb.query(DATABASE_SETTINGTABLE, mColumns, KEY_ROWID + "=" + 1, null, null, null, null, null);
		if (cursor == null || cursor.getCount() <= 0)
		{
			return 0;
		}
		
		cursor.moveToFirst();
		
		switch(type)
		{
			case 1: //radio
				position = cursor.getInt(2);
			
			case 0: // tv
			default:
				position = cursor.getInt(1);
		}
		cursor.close();

		return position;
	}

	public void putPosition(int type, int position)
	{
		ContentValues cv = new ContentValues();

		switch(type)
		{
			case 1: //radio
				cv.put(KEY_POSITION_RADIO, position);
			break;
			
			case 0: // tv
			default:
				cv.put(KEY_POSITION_TV, position);
			break;
		}

		mDb.update(DATABASE_SETTINGTABLE, cv, KEY_ROWID + "=" + 1, null);
	}
	
	public int getArea()
	{
		Log.d(TAG, "getArea()");
		int	AreaCode = 0;
		Cursor cursor = mDb.query(DATABASE_SETTINGTABLE, mColumns, KEY_ROWID + "=" + 1, null, null, null, null, null);
		if (cursor == null || cursor.getCount() <= 0)
		{
			return -1;
		}
		
		cursor.moveToFirst();
		
		AreaCode = cursor.getInt(3);
		cursor.close();

		return AreaCode;
	}
	
	public void putArea(int areaCode)
	{
		ContentValues cv = new ContentValues();
		cv.put(KEY_AREA, areaCode);
		mDb.update(DATABASE_SETTINGTABLE, cv, KEY_ROWID + "=" + 1, null);
	}

	public int getAge()
	{
		Log.d(TAG, "getAge()");
		int	Age = 0;
		Cursor cursor = mDb.query(DATABASE_SETTINGTABLE, mColumns, KEY_ROWID + "=" + 1, null, null, null, null, null);
		if (cursor == null || cursor.getCount() <= 0)
		{
			return -1;
		}
		
		cursor.moveToFirst();
		Age = cursor.getInt(4);
		cursor.close();
		
		return Age;
	}
	
	public static void putAge(int age)
	{
		ContentValues cv = new ContentValues();
		cv.put(KEY_AGE, age);
		mDb.update(DATABASE_SETTINGTABLE, cv, KEY_ROWID + "=" + 1, null);
	}

	public int getAspectRatio()
	{
		Log.d(TAG, "getAspectRatio()");
		int AspectRatio =0;
		
		Cursor cursor = mDb.query(DATABASE_SETTINGTABLE, mColumns, KEY_ROWID + "=" + 1, null, null, null, null, null);
		if (cursor == null || cursor.getCount() <= 0)
		{
			return -1;
		}
		
		cursor.moveToFirst();
		AspectRatio = cursor.getInt(5);
		cursor.close();

		return AspectRatio;
	}

	public void putAspectRatio(int aspectRatio)
	{
		ContentValues cv = new ContentValues();
		cv.put(KEY_ASPECTRATIO, aspectRatio);
		mDb.update(DATABASE_SETTINGTABLE, cv, KEY_ROWID + "=" + 1, null);
	}
}
