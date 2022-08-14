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

import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

import java.io.File;
import java.nio.ByteOrder;
import java.nio.ByteBuffer;
import java.io.FileInputStream;

import com.telechips.android.isdbt.player.isdbt.DB_AUDIO_Data;
import com.telechips.android.isdbt.player.isdbt.DB_SERVICE_Data;
import com.telechips.android.isdbt.player.isdbt.DB_SUBTITLE_Data;
import com.telechips.android.isdbt.player.isdbt.ISDBTPlayer;

public class FileManager extends ChannelManager {
    
	private final static String TAG = "[[[FileManager]]]";

	private FileDbAdapter mFileDbAdapter;

	//Override
	public static final String KEY_CHANNEL_NUMBER	= "uiCurrentChannelNumber";
	public static final String KEY_COUNTRY_CODE		= "uiCurrentCountryCode";
	public static final String KEY_FREQUENCY		= "uiFrequency";
	public static final String KEY_SERVICE_TYPE		= "enType";
	public static final String KEY_AUDIO_PID		= "uiAudio_PID";
	public static final String KEY_VIDEO_PID		= "uiVideo_PID";
	public static final String KEY_TELETEXT_PID		= "uiTeletext_PID";
	public static final String KEY_SERVICE_ID		= "usServiceID";
	public static final String KEY_CHANNEL_NAME		= "strServiceName";
	public static final String KEY_BOOKMARK			= "uiBookmark";
	public static final String KEY_LOGICALCHANNEL	= "uiLogicalChannel";
	
    protected static final String CHANNEL_DATABASE_TABLE = "services";
    
    protected static final String CHANNEL_DATABASE_CREATE =
            "create table if not exists services (_id integer primary key, "
        	+ "uiCurrentChannelNumber integer, uiCurrentCountryCode integer, uiFrequency integer, ucVersionNum integer, usNetworkId integer, uiTStreamID integer, usOrig_Net_ID integer, "
            + "usServiceID integer, PMT_PID integer, remocon_ID integer, enType integer, fEIT_Schedule integer, fEIT_PresentFollowing integer, fCA_Mode_free integer, "
        	+ "fAllowCountry integer, ucNameLen integer, strServiceName text, uiAudio_PID integer, uiVideo_PID integer, ucVideo_IsScrambling integer, ucAudio_StreamType integer, ucVideo_StreamType integer, "
            + "uiPCR_PID integer, uiTotalAudio_PID integer, ucLastTableID integer, uiLogicalChannel integer, usTotalCntSubtLang integer, uiTeletext_PID integer, "
        	+ "ucCount_Teletext_Info integer, ucPMTVersionNum integer, ucSDTVersionNum integer, ucNITVersionNum integer,  uiBookmark integer, "
            + "preset integer, DWNL_ID, LOGO_ID);";
    
	private static final String AUDIO_TABLE_DATABASE_CREATE =
		"create table if not exists audioPID (_id integer primary key, usServiceID integer, uiCurrentChannelNumber integer, uiCurrentCountryCode integer, uiAudio_PID integer, ucAudio_IsScrambling integer, ucAudio_StreamType integer, ucAudioType integer, acLangCode text);";

	private static final String SUBTITLE_TABLE_DATABASE_CREATE =
		"create table if not exists subTitle (_id integer primary key, usServiceID integer, uiCurrentChannelNumber integer, uiCurrentCountryCode integer, ucSubtitle_PID integer, acSubtLangCode text, ucSubtType integer, usCompositionPageId integer, usAncillaryPageId integer);";

	private static final String TTX_DESCRIPTOR_DATABASE_CREATE =
		"create table if not exists TTX_Descriptor(_id integer primary key, usServiceID integer, uiCurrentChannelNumber integer, uiCurrentCountryCode integer, Language_Code text, eType integer, ucMagazine_Number integer, ucPage_Number integer);";

	private final Context mContext;
	private class FileDbAdapter  extends SQLiteOpenHelper
	{
		FileDbAdapter (Context context)
		{
			super(context, null, null, DATABASE_VERSION);
		}

		@Override
		public void onCreate(SQLiteDatabase database)
		{
			Log.d(TAG, "onCreate()");
			
			database.execSQL(CHANNEL_DATABASE_CREATE);
			database.execSQL(AUDIO_TABLE_DATABASE_CREATE);
			database.execSQL(SUBTITLE_TABLE_DATABASE_CREATE);
			database.execSQL(TTX_DESCRIPTOR_DATABASE_CREATE);
		}

		@Override
		public void onUpgrade(SQLiteDatabase database, int oldVersion, int newVersion)
		{
			Log.d(TAG, "onUpgrade()");
			
			database.execSQL("DROP TABLE IF EXISTS services");
			onCreate(database);
		}		
	}
	
	public FileManager(Context context) {
		super(context);
		Log.d(TAG, "FileManager()");
		mContext = context;
	}

	public FileManager open() throws SQLException
	{
		Log.d(TAG, "open()");
		
		mFileDbAdapter = new FileDbAdapter (mContext);
		mSQLiteDatabase = mFileDbAdapter.getWritableDatabase();

		return this;
	}

	public void close()
	{
		Log.d(TAG, "close()");
		
		mSQLiteDatabase.close();
		mFileDbAdapter.close();
	}

    public static boolean isRecordedFile(String path) {
        int lastDot = path.lastIndexOf(".");
        if (lastDot < 0) return false;
        return "MTS".equals(path.substring(lastDot + 1).toUpperCase());
    }

	public void listUpdate(String dirPath)
	{
		File f = new File(dirPath);
		File[] files = f.listFiles();

		if (files != null) {
			for (int i = 0; i < files.length; i++) {
				File file = files[i];

				if (file.isFile() && isRecordedFile(file.getName())) {
					insertChannel(file);
				}
			}
		}
	}

	ISDBTPlayer.OnDBInfoUpdateListener ListenerOnDBInfoUpdateListener = new ISDBTPlayer.OnDBInfoUpdateListener()
	{
		public void onDBInfoUpdate(ISDBTPlayer player, int dbtype, Object dbinfo)
		{
			Log.w(TAG, "onDBInfoUpdate");
			switch(dbtype) {
				case 1: // Service DB
					{
						DB_SERVICE_Data serviceDB = (DB_SERVICE_Data)dbinfo;
					}
					break;

				case 2: // Audio DB
					{
						DB_AUDIO_Data audioDB = (DB_AUDIO_Data)dbinfo;
						insertAudioPIDTable(audioDB);
					}
					break;

				case 3: // Subtitle DB
					{
						DB_SUBTITLE_Data subtitleDB = (DB_SUBTITLE_Data)dbinfo;
						insertSubtitleTable(subtitleDB);
					}
					break;
			}
		}
	};

	public int ByteToInteger(byte a, byte b, byte c, byte d)
	{
		int result = (a & 0xff);
		result = (result << 8) | (b & 0xff);
		result = (result << 8) | (c & 0xff);
		result = (result << 8) | (d & 0xff);

		return result;
	}

	public long insertChannel(File f)
	{
		Log.d(TAG, "insertChannel()");

		byte[] buffer = new byte[144];
		try {
			FileInputStream in = new FileInputStream(f);
			in.skip(4);
			in.read(buffer, 0, 144);
			in.close();
		} catch (Exception e) {
			Log.e(TAG, "[ERR:" + f.getName() + "]" + e);
			return -1;
		}

		ByteBuffer buff = ByteBuffer.allocate(buffer.length);       
		buff = ByteBuffer.wrap(buffer);
		buff.order(ByteOrder.LITTLE_ENDIAN);

		int uiCurrentChannelNumber = buff.getInt();
		int uiCurrentCountryCode = buff.getInt();
		int uiAudio_PID = buff.getInt();
		int uiVideo_PID = buff.getInt();
		int stPID = buff.getInt();
		int siPID = buff.getInt();
		int PMT_PID = buff.getInt();
		int remoconID = buff.getInt();
		int enType = buff.getInt();
		int usServiceID = buff.getInt();
		int regionID = buff.getInt();
		int threedigitNumber = buff.getInt();
		int uiTStreamID = buff.getInt();
		int berAVG = buff.getInt();
		int preset = buff.getInt();
		int usOrig_Net_ID = buff.getInt();
		int areaCode = buff.getInt();
		int frequency = buff.getInt();
		int affiliationID = buff.getInt();
				
		String string = String.format("INSERT INTO services (uiCurrentChannelNumber, uiCurrentCountryCode, ucVersionNum, uiTStreamID, usOrig_Net_ID, usServiceID, PMT_PID, enType, fEIT_Schedule, fEIT_PresentFollowing, fCA_Mode_free, fAllowCountry, ucNameLen, strServiceName, uiAudio_PID, uiVideo_PID, ucVideo_IsScrambling, ucAudio_StreamType, ucVideo_StreamType, uiPCR_PID, uiTotalAudio_PID, ucLastTableID, uiLogicalChannel, usTotalCntSubtLang, uiTeletext_PID, ucCount_Teletext_Info, ucPMTVersionNum, ucSDTVersionNum, ucNITVersionNum) VALUES (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d,'%s', %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
				uiCurrentChannelNumber,	//uiCurrentChannelNumber
				uiCurrentCountryCode,	//uiCurrentCountryCode
				0,						//ucVersionNum
				uiTStreamID,			//uiTStreamID
				usOrig_Net_ID,			//usOrig_Net_ID
				usServiceID,			//usServiceID
				PMT_PID,				//PMT_PID
				enType,					//enType
				0,						//fEIT_Schedule
				0,						//fEIT_PresentFollowing
				0,						//fCA_Mode_free
				0,						//fAllowCountry
				0,						//ucNameLen
				f.getName(),			//strServiceName
				uiAudio_PID,			//uiAudio_PID
				uiVideo_PID,			//uiVideo_PID
				0,						//ucVideo_IsScrambling
				0,						//ucAudio_StreamType
				0,						//ucVideo_StreamType
				0,						//uiPCR_PID
				0,						//uiTotalAudio_PID
				0,						//ucLastTableID
				0,						//uiLogicalChannel
				0,						//usTotalCntSubtLang
				0,						//uiTeletext_PID
				0,						//ucCount_Teletext_Info
				0,						//ucPMTVersionNum
				0,						//ucSDTVersionNum
				0						//ucNITVersionNum
		);
		
		mSQLiteDatabase.execSQL(string);
		return 0;
	}

	public boolean deleteChannel(long rowId)
	{
		Log.d(TAG, "deleteChannel()");

		return mSQLiteDatabase.delete(CHANNEL_DATABASE_TABLE, KEY_ROWID + "=" + rowId, null) > 0;
	}

	public boolean deleteAllChannel()
	{
		Log.d(TAG, "deleteAllChannel()");

		return mSQLiteDatabase.delete(CHANNEL_DATABASE_TABLE, null, null) > 0;
	}

	public long insertAudioPIDTable(DB_AUDIO_Data audioDB)
	{
		Log.d(TAG, "insertAudioPIDTable()");
		String langCode = new String(audioDB.ucacLangCode);
		mSQLiteDatabase.execSQL(String.format("DELETE FROM audioPID WHERE usServiceID=%d AND uiAudio_PID=%d", audioDB.uiServiceID, audioDB.uiAudioPID));
		mSQLiteDatabase.execSQL(String.format("INSERT INTO audioPID (usServiceID, uiCurrentChannelNumber, uiCurrentCountryCode, uiAudio_PID, ucAudio_IsScrambling, ucAudio_StreamType, ucAudioType, acLangCode) VALUES (%d, %d, %d, %d, %d, %d, %d, '%s')",
				audioDB.uiServiceID,
				audioDB.uiCurrentChannelNumber,
				audioDB.uiCurrentCountryCode,
				audioDB.uiAudioPID,
				audioDB.uiAudioIsScrambling,
				audioDB.uiAudioStreamType,
				audioDB.uiAudioType,
				langCode
		));
		return 0;
	}

	public boolean deleteAudioPIDTable(long rowId)
	{
		Log.d(TAG, "deleteAudioPIDTable()");

		return mSQLiteDatabase.delete(AUDIOPID_DATABASE_TABLE, KEY_ROWID + "=" + rowId, null) > 0;
	}

	public boolean deleteAllAudioPIDTable()
	{
		Log.d(TAG, "deleteAllAudioPIDTable()");

		return mSQLiteDatabase.delete(AUDIOPID_DATABASE_TABLE, null, null) > 0;
	}

	public long insertSubtitleTable(DB_SUBTITLE_Data subtitleDB)
	{
		Log.d(TAG, "insertSubtitleTable()");
		String langCode = new String(subtitleDB.ucacSubtLangCode);
		mSQLiteDatabase.execSQL(String.format("DELETE FROM subTitle WHERE usServiceID=%d AND ucSubtitle_PID=%d", subtitleDB.uiServiceID, subtitleDB.uiSubtitlePID));
		mSQLiteDatabase.execSQL(String.format("INSERT INTO subTitle (usServiceID, uiCurrentChannelNumber, uiCurrentCountryCode, ucSubtitle_PID, acSubtLangCode, ucSubtType, usCompositionPageId, usAncillaryPageId) VALUES (%d, %d, %d, %d, '%s', %d, %d, %d)",
				subtitleDB.uiServiceID,
				subtitleDB.uiCurrentChannelNumber,
				subtitleDB.uiCurrentCountryCode,
				subtitleDB.uiSubtitlePID,
				langCode,
				subtitleDB.uiSubtType,
				subtitleDB.uiCompositionPageID,
				subtitleDB.uiAncillaryPageID
		));
		return 0;
	}

	public boolean deleteSubtitleTable(long rowId)
	{
		Log.d(TAG, "deleteSubtitleTable()");

		return mSQLiteDatabase.delete(SUBTITLE_DATABASE_TABLE, KEY_ROWID + "=" + rowId, null) > 0;
	}

	public boolean deleteAllSubtitleTable()
	{
		Log.d(TAG, "deleteAllSubtitleTable()");

		return mSQLiteDatabase.delete(SUBTITLE_DATABASE_TABLE, null, null) > 0;
	}

	public long insertTeleTextDescriptor(int usServiceID, int uiCurrentChannelNumber, int uiCurrentCountryCode, String Language_Code, int eType, int ucMagazine_Number, int ucPage_Number)
	{
		Log.d(TAG, "insertTeleTextDescriptor()");

		mSQLiteDatabase.execSQL(String.format("INSERT INTO TTX_Descriptor (usServiceID, uiCurrentChannelNumber, uiCurrentCountryCode, Language_Code, eType, ucMagazine_Number, ucPage_Number) VALUES (%d, %d, %d, '%s', %d, %d, %d)",
				usServiceID,
				uiCurrentChannelNumber,
				uiCurrentCountryCode,
				Language_Code,
				eType,
				ucMagazine_Number,
				ucPage_Number
		));
		return 0;
	}

	public boolean deleteTeleTextDescriptor(long rowId)
	{
		Log.d(TAG, "deleteTeleTextDescriptor()");

		return mSQLiteDatabase.delete(TTX_DESCRIPTOR_DATABASE_TABLE, KEY_ROWID + "=" + rowId, null) > 0;
	}

	public boolean deleteAllTeleTextDescriptor()
	{
		Log.d(TAG, "deleteAllTeleTextDescriptor()");

		return mSQLiteDatabase.delete(TTX_DESCRIPTOR_DATABASE_TABLE, null, null) > 0;
	}

	public Cursor getAllFiles(String dirPath)
	{
		Log.d(TAG, "getAllFiles()");

		deleteAllChannel();
		listUpdate(dirPath);

		return mSQLiteDatabase.query(CHANNEL_DATABASE_TABLE,
			new String[] {
							KEY_ROWID,
							KEY_CHANNEL_NUMBER,
							KEY_FREQUENCY,
							KEY_COUNTRY_CODE,
							KEY_SERVICE_TYPE,
							KEY_AUDIO_PID,
							KEY_VIDEO_PID,
							KEY_TELETEXT_PID,
							KEY_SERVICE_ID,
							KEY_CHANNEL_NAME,
							KEY_FREE_CA_MODE,
							KEY_SCRAMBLING,
							KEY_BOOKMARK,
							KEY_LOGICALCHANNEL
							},
				null,
				null, null, null, null, null);
	}
	
	@Override
	public Cursor getSubtitle_LanguageCode(int ServiceID, int ChannelNumber)
	{
		Log.i(TAG, "getSubtitle_LanguageCode()");
	
		if(mSQLiteDatabase == null)
		{
			return null;
		}
/*		
		Cursor cursor	= mSQLiteDatabase.query(true, SUBTITLE_TABLE_DATABASE_CREATE,
				new String[] {
								KEY_ROWID,
								KEY_SUBTITLE_LANGUAGE_CODE
								},
				KEY_SUBTITLE_SERVICE_ID + "=" + ServiceID + " AND " + KEY_SUBTITLE_CHANNEL_NUMBER + "=" + ChannelNumber,
				null, null, null, null, null);
		
		if(cursor != null)
		{
			cursor.moveToFirst();
		}
		
		return cursor;
*/
		return null;
	}
}

