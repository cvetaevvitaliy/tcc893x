package com.telechips.android.atsc.samples;

import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;


public class ChannelManager {
	private final static String TAG = "ChannelManager";

	public static final String KEY_COUNTRY_CODE 	= "CountryCode";
	public static final String KEY_CHANNEL_NUMBER 	= "ChannelNumber";
	public static final String KEY_MODULATION 		= "Modulation";
	public static final String KEY_SERVICE_TYPE 	= "ServiceType";
	public static final String KEY_SERVICE_ID 		= "Service_ID";
	public static final String KEY_CHMAJORNUM		= "LogicalCHNumber";
	public static final String KEY_CHMINORNUM 		= "LogicalCHNumberMinor";
	public static final String KEY_SERVICE_NAME 	= "ServiceName";
	public static final String KEY_VER_PAT 			= "Ver_PAT";
	public static final String KEY_PMT_PID 			= "PMT_PID";

	public static final String KEY_ROWID = "_id";

	public static final String KEY_EPG_START_YEAR 		= "Start_YEAR";
	public static final String KEY_EPG_START_MONTH 		= "Start_MON";
	public static final String KEY_EPG_START_DAY 		= "Start_DAY";
	public static final String KEY_EPG_START_WDAY 		= "Start_WDAY";
	public static final String KEY_EPG_START_HOUR 		= "Start_HH";
	public static final String KEY_EPG_START_MINUTE 	= "Start_MM";
	public static final String KEY_EPG_DURATION_HOUR 	= "Duration_HH";
	public static final String KEY_EPG_DURATION_MINUTE 	= "Duration_MM";	
	public static final String KEY_EPG_EVT_NAME 		= "EvtName";
	public static final String KEY_EPG_EVT_EXTERN 		= "EvtText_extn";
	public static final String KEY_EPG_SERVICE_ID 		= "Service_ID";
	public static final String KEY_EPG_CHANNEL_NUMBER 	= "ChannelNumber";
	public static final String KEY_EPG_COUNTRYCODE 		= "CountryCode";

	public static final int	SERVICE_TYPE_DTV	= 0x01;
	public static final int	SERVICE_TYPE_DRADIO = 0x02;

	private ChannelDbAdapter m_clChannelDbAdapter;	
	private SQLiteDatabase m_clSQLiteDatabase;
	private SQLiteDatabase m_clSQLiteDatabase_EPG;

	private static final String DATABASE_ATSC 			= "ATSC.db";
	private static final String DATABASE_CHANNEL_TABLE 	= "ChannelTable";
	
	private static final String DATABASE_ATSC_EPG 		= "/data/data/com.telechips.android.atsc.samples/databases/ATSC_EPG_DB.db";
	private static final String DATABASE_EPG_PF_TABLE 	= "EPG_PF";

	private static final int DATABASE_VERSION = 2;

	private static final String DATABASE_CHANNEL_TABLE_CREATE =
		"create table if not exists ChannelTable (_id integer primary key, " + 
			"CountryCode integer, ChannelNumber integer, Modulation integer, " + 
			"ServiceType integer, Service_ID integer, "+
			"LogicalCHNumber integer, LogicalCHNumberMinor integer, "+
			"ServiceName text not null, " + 
			"Ver_PAT integer, PMT_PID integer, Scrambled integer);";


	private final Context m_clContext;
	
	private static class ChannelDbAdapter extends SQLiteOpenHelper {
		ChannelDbAdapter(Context context) {			
			super(context, DATABASE_ATSC, null, DATABASE_VERSION);

			Log.d(TAG, "[ChannelDbAdapter] : ChannelDbAdapter()");
		}

		@Override
		public void onCreate(SQLiteDatabase database) {
			Log.d(TAG, "[ChannelDbAdapter] : onCreate()");
			
			database.execSQL(DATABASE_CHANNEL_TABLE_CREATE);
		}

		@Override
		public void onUpgrade(SQLiteDatabase database, int oldVersion, int newVersion) {
			Log.d(TAG, "[ChannelDbAdapter] : onUpgrade()");
			
			database.execSQL("DROP TABLE IF EXISTS ChannelTable");
			onCreate(database);
		}		
	}
	
	public ChannelManager(Context context) {
		Log.d(TAG, "ChannelManager()");
		
		m_clContext = context;
	}

	public ChannelManager open() throws SQLException {
		Log.d(TAG, "open()");
		
		m_clChannelDbAdapter = new ChannelDbAdapter(m_clContext);
		m_clSQLiteDatabase 		= m_clChannelDbAdapter.getWritableDatabase();
		m_clSQLiteDatabase_EPG 	= null;
		
		return this;
	}

	public void close() {
		Log.d(TAG, "close()");
		closeEPGDB();
		m_clChannelDbAdapter.close();
	}
	
	public void closeEPGDB() {
		if(m_clSQLiteDatabase_EPG != null) {
			m_clSQLiteDatabase_EPG.close();
			m_clSQLiteDatabase_EPG = null;
		}
	}
	
	public boolean openEPGDB() {
		SQLiteDatabase db = null;
		if(m_clSQLiteDatabase_EPG != null)
			closeEPGDB();
		
		try {
			//db = SQLiteDatabase.openDatabase(DATABASE_ATSC_EPG, null, SQLiteDatabase.OPEN_READONLY);
			//db = SQLiteDatabase.openDatabase(DATABASE_ATSC_EPG, null, SQLiteDatabase.OPEN_READWRITE);	     
			db = SQLiteDatabase.openOrCreateDatabase(DATABASE_ATSC_EPG, null);
			m_clChannelDbAdapter.onOpen(db);	
			m_clSQLiteDatabase_EPG = db;
			Log.d(TAG, DATABASE_ATSC_EPG + " " + "Open Success");
			return true;
		} catch(Exception e) {
			Log.e(TAG, "Couldn't open " + DATABASE_ATSC_EPG + "Exception" + e);
			m_clSQLiteDatabase_EPG = null;
			Log.e(TAG, DATABASE_ATSC_EPG + "Open Fail");
			return false;
		}		
		
	}

	public boolean deleteChannel(long rowId) {
		Log.d(TAG, "deleteChannel()");
		
		return (m_clSQLiteDatabase.delete(DATABASE_CHANNEL_TABLE, KEY_ROWID + "=" + rowId, null) > 0);
	}

	public boolean deleteAllChannel() {
		Log.d(TAG, "deleteAllChannel()");
		
		return (m_clSQLiteDatabase.delete(DATABASE_CHANNEL_TABLE, null, null) > 0);
	}

	public Cursor getAllChannels(int iServiceType) 
	{
		Log.d(TAG, "getAllChannels()  "+ iServiceType);
		
		String szQueryService;

		if(iServiceType == 1)
		{
			szQueryService = KEY_SERVICE_TYPE +	"=" + SERVICE_TYPE_DTV;
		}
		else
		{
			szQueryService = KEY_SERVICE_TYPE +	"=" + SERVICE_TYPE_DRADIO;
		}

		return m_clSQLiteDatabase.query(true, DATABASE_CHANNEL_TABLE, 
			new String[] {
							KEY_ROWID,
							KEY_COUNTRY_CODE, 
							KEY_CHANNEL_NUMBER, 
							KEY_MODULATION, 
							KEY_SERVICE_TYPE, 
							KEY_SERVICE_ID, 
							KEY_CHMAJORNUM,
							KEY_CHMINORNUM,							
							KEY_SERVICE_NAME, 
							KEY_VER_PAT, 
							KEY_PMT_PID
						 },
				szQueryService,
				null, null, null, KEY_MODULATION+"," + KEY_CHMAJORNUM, null);
	}

	public Cursor getChannel(long rowId) throws SQLException {
		Log.d(TAG, "getChannel()");
		
		Cursor cursor = m_clSQLiteDatabase.query(true, DATABASE_CHANNEL_TABLE,
			new String[] { 
							KEY_ROWID, 
							KEY_COUNTRY_CODE, 
							KEY_CHANNEL_NUMBER, 
							KEY_MODULATION, 
							KEY_SERVICE_TYPE, 
							KEY_SERVICE_ID, 
							KEY_CHMAJORNUM,
							KEY_CHMINORNUM,							
							KEY_SERVICE_NAME, 
							KEY_VER_PAT, 
							KEY_PMT_PID 
						 }, 
			KEY_ROWID + "=" + rowId, 
			null, null, null, null, null);
		
		if(cursor != null) {
			cursor.moveToFirst();
		}
		return cursor;
	}
	
	public Cursor getEPGInfoFirstIdx(int iService_ID, int iChannelNumber, int iCountryCode) throws SQLException {
		Log.d(TAG, "getEPGInfoFirstIdx()");
		Log.d(TAG, "iService_ID:" + iService_ID);
		Log.d(TAG, "iChannelNumber:" + iChannelNumber);
		Log.d(TAG, "iCountryCode:" + iCountryCode);
		
		Cursor cursor = m_clSQLiteDatabase_EPG.query(true, DATABASE_EPG_PF_TABLE,
			new String[] { 
							KEY_ROWID,
							KEY_EPG_START_YEAR,
							KEY_EPG_START_MONTH,
							KEY_EPG_START_DAY,
							KEY_EPG_START_WDAY,
							KEY_EPG_START_HOUR,
							KEY_EPG_START_MINUTE,
							KEY_EPG_DURATION_HOUR,
							KEY_EPG_DURATION_MINUTE,
							KEY_EPG_EVT_NAME,
							KEY_EPG_EVT_EXTERN
						}, 
			KEY_EPG_SERVICE_ID + "=" + iService_ID + " AND " + KEY_EPG_CHANNEL_NUMBER + "=" + iChannelNumber + " AND " + KEY_EPG_COUNTRYCODE+ "=" + iCountryCode, 
			null, null, null, KEY_EPG_START_YEAR+"," +KEY_EPG_START_MONTH+ " ,"+ KEY_EPG_START_DAY+","+KEY_EPG_START_HOUR + " ," + KEY_EPG_START_MINUTE, null);
		
		if(cursor != null) {
			cursor.moveToFirst();
		}
		return cursor;
	}

	public Cursor getEPGInfoLastIdx(int iService_ID, int iChannelNumber, int iCountryCode) throws SQLException {
		Log.d(TAG, "getEPGInfoLastIdx()");
		Log.d(TAG, "iService_ID:" + iService_ID);
		Log.d(TAG, "iChannelNumber:" + iChannelNumber);
		Log.d(TAG, "iCountryCode:" + iCountryCode);
		
		Cursor cursor = m_clSQLiteDatabase_EPG.query(true, DATABASE_EPG_PF_TABLE,
			new String[] { 
							KEY_ROWID,
							KEY_EPG_START_YEAR,
							KEY_EPG_START_MONTH,
							KEY_EPG_START_DAY,
							KEY_EPG_START_WDAY,
							KEY_EPG_START_HOUR,
							KEY_EPG_START_MINUTE,
							KEY_EPG_DURATION_HOUR,
							KEY_EPG_DURATION_MINUTE,
							KEY_EPG_EVT_NAME,
							KEY_EPG_EVT_EXTERN
						}, 
			KEY_EPG_SERVICE_ID + "=" + iService_ID + " AND " + KEY_EPG_CHANNEL_NUMBER + "=" + iChannelNumber + " AND " + KEY_EPG_COUNTRYCODE+ "=" + iCountryCode, 
			null, null, null, KEY_EPG_START_YEAR+"," +KEY_EPG_START_MONTH+ " ,"+ KEY_EPG_START_DAY+","+KEY_EPG_START_HOUR + " ," + KEY_EPG_START_MINUTE, null);
		
		if(cursor != null) {
			cursor.moveToLast();
		}
		return cursor;
	}

	public Cursor getEPGInfo(int iService_ID, int iChannelNumber, int iCountryCode, long rowId) throws SQLException {
		Log.d(TAG, "getEPGInfo()"+rowId);
		Log.d(TAG, "iService_ID:" + iService_ID);
		Log.d(TAG, "iChannelNumber:" + iChannelNumber);
		Log.d(TAG, "iCountryCode:" + iCountryCode);
		
		Cursor cursor = m_clSQLiteDatabase_EPG.query(true, DATABASE_EPG_PF_TABLE,
			new String[] { 
							KEY_ROWID,
							KEY_EPG_START_YEAR,
							KEY_EPG_START_MONTH,
							KEY_EPG_START_DAY,
							KEY_EPG_START_WDAY,
							KEY_EPG_START_HOUR,
							KEY_EPG_START_MINUTE,
							KEY_EPG_DURATION_HOUR,
							KEY_EPG_DURATION_MINUTE,
							KEY_EPG_EVT_NAME,
							KEY_EPG_EVT_EXTERN
						}, 
			KEY_ROWID + "=" + rowId+ " AND "+KEY_EPG_SERVICE_ID + "=" + iService_ID + " AND " + KEY_EPG_CHANNEL_NUMBER + "=" + iChannelNumber + " AND " + KEY_EPG_COUNTRYCODE+ "=" + iCountryCode, 
			null, null, null, KEY_EPG_START_YEAR+"," +KEY_EPG_START_MONTH+ " ,"+ KEY_EPG_START_DAY+","+KEY_EPG_START_HOUR + " ," + KEY_EPG_START_MINUTE, null);
		
		if(cursor != null) {
			cursor.moveToFirst();
		}
		return cursor;
	}

	public Cursor getEPG_PF_Information(int iService_ID, int iChannelNumber, int iCountryCode) throws SQLException {
		Log.d(TAG, "getEPG_PF_Information()");
		Log.d(TAG, "iService_ID:" + iService_ID);
		Log.d(TAG, "iChannelNumber:" + iChannelNumber);
		Log.d(TAG, "iCountryCode:" + iCountryCode);
		
		Cursor cursor = m_clSQLiteDatabase_EPG.query(true, DATABASE_EPG_PF_TABLE,
			new String[] {
							KEY_ROWID,
							KEY_EPG_START_YEAR,
							KEY_EPG_START_MONTH,
							KEY_EPG_START_DAY,
							KEY_EPG_START_WDAY,
							KEY_EPG_START_HOUR,
							KEY_EPG_START_MINUTE,
							KEY_EPG_DURATION_HOUR,
							KEY_EPG_DURATION_MINUTE,
							KEY_EPG_EVT_NAME,
							KEY_EPG_EVT_EXTERN
							},
				KEY_EPG_SERVICE_ID + "=" + iService_ID + " AND " + KEY_EPG_CHANNEL_NUMBER + "=" + iChannelNumber + " AND " + KEY_EPG_COUNTRYCODE+ "=" + iCountryCode,
				null, null, null, KEY_EPG_START_YEAR+"," +KEY_EPG_START_MONTH+ " ,"+ KEY_EPG_START_DAY+","+KEY_EPG_START_HOUR + " ," + KEY_EPG_START_MINUTE, null);

		if (cursor != null) {
			cursor.moveToFirst();
		}
		return cursor;
	}

	public Cursor getEPG_PF_Information_byDay(int iService_ID, int iChannelNumber, int iCountryCode, int Day) throws SQLException {
		Log.d(TAG, "getEPG_PF_Information()");
		Log.d(TAG, "iService_ID:" + iService_ID);
		Log.d(TAG, "iChannelNumber:" + iChannelNumber);
		Log.d(TAG, "iCountryCode:" + iCountryCode);
		
		Cursor cursor = m_clSQLiteDatabase_EPG.query(true, DATABASE_EPG_PF_TABLE,
			new String[] {
							KEY_ROWID,
							KEY_EPG_START_YEAR,
							KEY_EPG_START_MONTH,
							KEY_EPG_START_DAY,
							KEY_EPG_START_WDAY,
							KEY_EPG_START_HOUR,
							KEY_EPG_START_MINUTE,
							KEY_EPG_DURATION_HOUR,
							KEY_EPG_DURATION_MINUTE,
							KEY_EPG_EVT_NAME,
							KEY_EPG_EVT_EXTERN
							},
				KEY_EPG_SERVICE_ID + "=" + iService_ID + " AND " + KEY_EPG_CHANNEL_NUMBER + "=" + iChannelNumber + " AND " + KEY_EPG_COUNTRYCODE+ "=" + iCountryCode +" AND " +KEY_EPG_START_DAY+"="+Day,
				null, null, null, KEY_EPG_START_YEAR+"," +KEY_EPG_START_MONTH+ " ,"+ KEY_EPG_START_DAY+","+KEY_EPG_START_HOUR + " ," + KEY_EPG_START_MINUTE, null);

		if (cursor != null) {
			cursor.moveToFirst();
		}
		return cursor;
	}

}

