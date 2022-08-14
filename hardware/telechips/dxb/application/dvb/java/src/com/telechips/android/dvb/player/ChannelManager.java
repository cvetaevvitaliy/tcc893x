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

package com.telechips.android.dvb.player;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.nio.channels.FileChannel;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

import com.telechips.android.dvb.player.dvb.DB_CHANNEL_Data;
import com.telechips.android.dvb.player.dvb.DB_SERVICE_Data;
import com.telechips.android.dvb.player.dvb.DB_AUDIO_Data;
import com.telechips.android.dvb.player.dvb.DB_SUBTITLE_Data;

public class ChannelManager extends SQLiteOpenHelper
{
	//CAS Information
	public static final String KEY_FREE_CA_MODE = "fCA_Mode_free"; //Information in SDT
	public static final String KEY_SCRAMBLING = "ucVideo_IsScrambling"; //CA descriptor in PMT
///////////////////////////////

	public static final String KEY_ROWID = "_id";

	public static final String KEY_CHANNEL_NUMBER	= "uiCurrentChannelNumber";
	public static final String KEY_COUNTRY_CODE		= "uiCurrentCountryCode";
	public static final String KEY_FREQUENCY		= "uiFrequency";
	public static final String KEY_SERVICE_TYPE		= "enType";
	public static final String KEY_AUDIO_PID		= "uiAudio_PID";
	public static final String KEY_VIDEO_PID		= "uiVideo_PID";
	public static final String KEY_VIDEO_STREAM_TYPE = "ucVideo_StreamType";
	public static final String KEY_TELETEXT_PID		= "uiTeletext_PID";
	public static final String KEY_SERVICE_ID		= "usServiceID";
	public static final String KEY_CHANNEL_NAME		= "strServiceName";
	public static final String KEY_BOOKMARK			= "uiBookmark";
	public static final String KEY_LOGICALCHANNEL	= "uiLogicalChannel";
	public static final String KEY_PCR_PID			= "uiPCR_PID";
	public static final String KEY_TOTAL_AUDIO_PID	= "uiTotalAudio_PID";
	public static final String KEY_TOTAL_SUBTITLE	= "usTotalCntSubtLang";
	public static final String KEY_PMT_VERSION_NUM	= "ucPMTVersionNum";
	public static final String KEY_CHANNEL_SNR		= "uiSnr";

	public static final String KEY_US_SERVICE_ID	= "usServiceID";
	public static final String KEY_STREAM_AUDIO_PID	= "uiAudio_PID";
	public static final String KEY_STREAM_TYPE		= "ucAudio_StreamType";
	public static final String KEY_AUDIO_TYPE		= "ucAudioType";
	public static final String KEY_AC_LANGCODE		= "acLangCode";
	
	public static final String KEY_EPG_START_MJD	= "Start_MJD";
	public static final String KEY_EPG_START_HOUR	= "Start_HH";
	public static final String KEY_EPG_START_MINUTE	= "Start_MM";
	public static final String KEY_EPG_DURATION_HOUR	= "Duration_HH";
	public static final String KEY_EPG_DURATION_MINUTE	= "Duration_MM";	
	public static final String KEY_EPG_EVT_NAME		= "EvtName";
	public static final String KEY_EPG_EVT_TEXT		= "EvtText";
	public static final String KEY_EPG_EVT_EXTERN	= "EvtText_extn";
	public static final String KEY_EPG_SERVICE_ID	= "usServiceID";
	public static final String KEY_EPG_CHANNEL_NUMBER	= "uiCurrentChannelNumber";
	public static final String KEY_EPG_COUNTRY_CODE	= "uiCurrentCountryCode";
	public static final String KEY_EPG_RATING		= "iRating";
	
	public static final String KEY_AUDIO_LANGUAGE_CODE	= "acLangCode";
	public static final String KEY_AUDIO_STREAM_TYPE	= "ucAudio_StreamType";
	public static final String KEY_AUDIO_SERVICE_ID		= "usServiceID";
	public static final String KEY_AUDIO_CHANNEL_NUMBER	= "uiCurrentChannelNumber";
	
	public static final String KEY_SUBTITLE_LANGUAGE_CODE	= "acSubtLangCode";
	public static final String KEY_SUBTITLE_SERVICE_ID		= "usServiceID";
	public static final String KEY_SUBTITLE_CHANNEL_NUMBER	= "uiCurrentChannelNumber";
	
	public static final String KEY_TTX_LANGUAGE_CODE		= "Language_Code";
	public static final String KEY_TTX_TYPE					= "eType";
	public static final String KEY_TTX_MAGAZINE_NUMBER		= "ucMagazine_Number";
	public static final String KEY_TTX_PAGE_NUMBER			= "ucPage_Number";
	
	public static final int	SERVICE_TYPE_DTV	= 0x01;
	public static final int	SERVICE_TYPE_MHDTV	= 0x11;
	public static final int	SERVICE_TYPE_ASDTV	= 0x16;
	public static final int	SERVICE_TYPE_AHDTV	= 0x19;
	
	public static final int	SERVICE_TYPE_DRADIO		= 0x02;
	public static final int	SERVICE_TYPE_FMRADIO	= 0x07;
	public static final int	SERVICE_TYPE_ADRADIO	= 0x0A;

	protected SQLiteDatabase mSQLiteDatabase	= null;
	private SQLiteDatabase mSQLiteDatabaseEPG	= null;

	protected static String DATABASE_NAME = "DVBT.db";
	protected static String DATABASE_NAME_BACK = "DVBT_Bak.db";
	protected static final String CHANNEL_DATABASE_TABLE = "services";
	protected static final String AUDIOPID_DATABASE_TABLE = "audioPID";
	protected static final String SUBTITLE_DATABASE_TABLE	= "subTitle";
	protected static final String TTX_DESCRIPTOR_DATABASE_TABLE = "TTX_Descriptor";
	
	private static final String DATABASE_EPG_NAME = "DVBTEPG.db";
	private static final String EPG_PF_DATABASE_TABLE = "EPG_PF";
	private static final String EPG_SCHEDULE_DATABASE_TABLE = "EPG_Schedule";

	protected static final int DATABASE_VERSION = 2;

	public class EpgData {
		int iStartHH;
		int iStartMM;
		int iDurationHH;
		int iDurationMM;
		int iRating;
		String sName;
	}

	protected static final String CHANNEL_DATABASE_CREATE =
		"create table if not exists services (_id integer primary key, uiCurrentChannelNumber integer, uiCurrentCountryCode integer, uiDVBSystem integer, uiFrequency integer, uiDVBPLPId integer, ucVersionNum integer, uiTStreamID integer, usOrig_Net_ID integer, usServiceID integer, PMT_PID integer, enType integer, fEIT_Schedule integer, fEIT_PresentFollowing integer, fCA_Mode_free integer, fAllowCountry integer, ucNameLen integer, strServiceName text, uiAudio_PID integer, uiVideo_PID integer, ucVideo_IsScrambling integer, ucAudio_StreamType integer, ucVideo_StreamType integer, uiPCR_PID integer, uiTotalAudio_PID integer, ucLastTableID integer, uiLogicalChannel integer, usTotalCntSubtLang integer, uiTeletext_PID integer, ucCount_Teletext_Info integer, ucPMTVersionNum integer, ucSDTVersionNum integer, ucNITVersionNum integer, uiBookmark integer, uiSnr integer);";

	protected static final String AUDIO_TABLE_DATABASE_CREATE =
		"create table if not exists audioPID (_id integer primary key, usServiceID integer, uiCurrentChannelNumber integer, uiCurrentCountryCode integer, uiDVBPLPId integer, uiAudio_PID integer, ucAudio_IsScrambling integer, ucAudio_StreamType integer, ucAudioType integer, acLangCode text);";

	protected static final String SUBTITLE_TABLE_DATABASE_CREATE =
		"create table if not exists subTitle (_id integer primary key, usServiceID integer, uiCurrentChannelNumber integer, uiCurrentCountryCode integer, uiDVBPLPId integer, ucSubtitle_PID integer, acSubtLangCode text, ucSubtType integer, usCompositionPageId integer, usAncillaryPageId integer);";

	protected static final String TTX_DESCRIPTOR_DATABASE_CREATE =
		"create table if not exists TTX_Descriptor(_id integer primary key, usServiceID integer, uiCurrentChannelNumber integer, uiCurrentCountryCode integer, uiDVBPLPId integer, Language_Code text, eType integer, ucMagazine_Number integer, ucPage_Number integer);";

	//	Logical Channel
		private final int	ASSIGN_LOGICAL_CHANNEL_NO	= 800;

	protected final Context mContext;

	public ChannelManager(Context context)
	{
		this(context, DATABASE_NAME);
	}

	public ChannelManager(Context context, String name)
	{
		super(context, name, null, DATABASE_VERSION);

		DxbLog(D, "ChannelManager(" + name + ")");

		mContext= context;
	}

	@Override
	public void onCreate(SQLiteDatabase database)
	{
		DxbLog(D, "onCreate()");
		
		database.execSQL(CHANNEL_DATABASE_CREATE);
		database.execSQL(AUDIO_TABLE_DATABASE_CREATE);
		database.execSQL(SUBTITLE_TABLE_DATABASE_CREATE);
		database.execSQL(TTX_DESCRIPTOR_DATABASE_CREATE);
	}

	@Override
	public void onUpgrade(SQLiteDatabase database, int oldVersion, int newVersion)
	{
		DxbLog(D, "onUpgrade()");

		database.execSQL("DROP TABLE IF EXISTS services");
		onCreate(database);
	}

	public ChannelManager open() throws SQLException
	{
		DxbLog(D, "open()");

		close();

		mSQLiteDatabase = getWritableDatabase();
		mSQLiteDatabaseEPG = SQLiteDatabase.openOrCreateDatabase(getEpgFile(), null);

		return this;
	}

	public void close()
	{
		DxbLog(D, "close()");
		
		if(mSQLiteDatabaseEPG != null)
		{
			synchronized(mSQLiteDatabaseEPG)
			{
				mSQLiteDatabaseEPG.close();
			}
			mSQLiteDatabaseEPG	= null;
		}
		
		if(mSQLiteDatabase != null)
		{
			synchronized(mSQLiteDatabase)
			{
				mSQLiteDatabase.close();
			}
			mSQLiteDatabase	= null;
		}
	}

	public File getFile()
	{
		return mContext.getDatabasePath(DATABASE_NAME);
	}

	public File getEpgFile()
	{
		return mContext.getDatabasePath(DATABASE_EPG_NAME);
	}

	public void backup()
	{
		DxbLog(D, "backup()");

		try {
			FileInputStream in = new FileInputStream(mContext.getDatabasePath(DATABASE_NAME));
			FileOutputStream out = new FileOutputStream(mContext.getDatabasePath(DATABASE_NAME_BACK));
			FileChannel src = in.getChannel();
			FileChannel dst = out.getChannel();
			dst.transferFrom(src, 0, src.size());
			src.close();
			dst.close();
			in.close();
			out.close();
		} catch (Exception e) {
		}
	}

	public void restore()
	{
		DxbLog(D, "restore()");

		try {
			FileInputStream in = new FileInputStream(mContext.getDatabasePath(DATABASE_NAME_BACK));
			FileOutputStream out = new FileOutputStream(mContext.getDatabasePath(DATABASE_NAME));
			FileChannel src = in.getChannel();
			FileChannel dst = out.getChannel();
			dst.transferFrom(src, 0, src.size());
			src.close();
			dst.close();
			in.close();
			out.close();
		} catch (Exception e) {
		}
	}

	public boolean openEPGDB()
	{
		DxbLog(D, "openEPGDB()");
		
		if(mSQLiteDatabaseEPG == null)
		{
			try
			{
				mSQLiteDatabaseEPG = SQLiteDatabase.openOrCreateDatabase(getEpgFile(), null);
				DxbLog(D, DATABASE_EPG_NAME + "Open Success");
				return true;
			}
			catch(Exception e)
			{
				DxbLog(E, "Couldn't open " + DATABASE_EPG_NAME + "Exception" + e);
				mSQLiteDatabaseEPG = null;
				return false;
			}
		}
		
		return true;
		
	}

	public void onDBInfoUpdate(int dbtype, DB_CHANNEL_Data dbinfo)
	{
		DxbLog(D, "onDBInfoUpdate(dbtype=" + dbtype + ", dbinfo=" + dbinfo + ")");

		// Service Table
		updateChannel(dbinfo.dbService);

		// Audio Table
		deleteAudioPIDTable(dbinfo.dbService.uiServiceID, dbinfo.dbService.uiCurrentChannelNumber, dbinfo.dbService.uiCurrentCountryCode);
		for (int i = 0; i < dbinfo.dbService.uiTotalAudioPID; i++)
		{
			insertAudioPIDTable(dbinfo.dbService.uiServiceID, dbinfo.dbService.uiCurrentChannelNumber, dbinfo.dbService.uiCurrentCountryCode, dbinfo.dbAudio[i]);
		}

		// Subtitle Table
		deleteSubtitleTable(dbinfo.dbService.uiServiceID, dbinfo.dbService.uiCurrentChannelNumber, dbinfo.dbService.uiCurrentCountryCode);
		for (int i = 0; i < dbinfo.dbService.uiTotalCntSubtLang; i++)
		{
			insertSubtitleTable(dbinfo.dbService.uiServiceID, dbinfo.dbService.uiCurrentChannelNumber, dbinfo.dbService.uiCurrentCountryCode, dbinfo.dbSubtitle[i]);
		}
	}

	public long insertChannel(int channelNumber, int countryCode, int serviceType, int audioPID, int videoPID, int serviceID, String channelName)
	{
		DxbLog(D, "insertChannel()");
		return 0;
	}

	public long insertAudioPIDTable(int uiServiceID, int uiCurrentChannelNumber, int uiCurrentCountryCode, DB_AUDIO_Data audioDB)
	{
		DxbLog(D, "insertAudioPIDTable(uiServiceID=" + uiServiceID + ", uiCurrentChannelNumber=" + uiCurrentChannelNumber + ", uiCurrentCountryCode=" + uiCurrentCountryCode + ", audioDB=" + audioDB + ")");

		SQLiteDatabase channel = mSQLiteDatabase;
		if (channel != null && audioDB != null)
		{
			synchronized(channel)
			{
				if (channel.isOpen())
				{
					channel.execSQL(String.format("INSERT INTO audioPID (usServiceID, uiCurrentChannelNumber, uiCurrentCountryCode, uiDVBPLPId, uiAudio_PID, ucAudio_IsScrambling, ucAudio_StreamType, ucAudioType, acLangCode) VALUES (%d, %d, %d, %d, %d, %d, %d, %d, '%s')",
									uiServiceID,
									uiCurrentChannelNumber,
									uiCurrentCountryCode,
									0,
									audioDB.uiAudioPID,
									audioDB.uiAudioIsScrambling,
									audioDB.uiAudioStreamType,
									audioDB.uiAudioType,
									audioDB.stracLangCode
					));
				}
			}
		}
		
		return 0;
	}

	public long insertSubtitleTable(int uiServiceID, int uiCurrentChannelNumber, int uiCurrentCountryCode, DB_SUBTITLE_Data subtitleDB)
	{
		DxbLog(D, "insertSubtitleTable()");

		SQLiteDatabase channel = mSQLiteDatabase;
		if (channel != null && subtitleDB != null)
		{
			synchronized(channel)
			{
				if (channel.isOpen())
				{
					channel.execSQL(String.format("INSERT INTO subTitle (usServiceID, uiCurrentChannelNumber, uiCurrentCountryCode, uiDVBPLPId, ucSubtitle_PID, acSubtLangCode, ucSubtType, usCompositionPageId, usAncillaryPageId) VALUES (%d, %d, %d, %d, %d, '%s', %d, %d, %d)",
									uiServiceID,
									uiCurrentChannelNumber,
									uiCurrentCountryCode,
									0,
									subtitleDB.uiSubtitlePID,
									subtitleDB.stracSubtLangCode,
									subtitleDB.uiSubtType,
									subtitleDB.uiCompositionPageID,
									subtitleDB.uiAncillaryPageID
					));
				}
			}
		}
		
		return 0;
	}

	public long insertTeleTextDescriptor(int usServiceID, int uiCurrentChannelNumber, int uiCurrentCountryCode, String Language_Code, int eType, int ucMagazine_Number, int ucPage_Number)
	{
		DxbLog(D, "insertTeleTextDescriptor()");

		SQLiteDatabase channel = mSQLiteDatabase;
		if (channel != null)
		{
			synchronized(channel)
			{
				if (channel.isOpen())
				{
					channel.execSQL(String.format("INSERT INTO TTX_Descriptor (usServiceID, uiCurrentChannelNumber, uiCurrentCountryCode, Language_Code, eType, ucMagazine_Number, ucPage_Number) VALUES (%d, %d, %d, '%s', %d, %d, %d)",
									usServiceID,
									uiCurrentChannelNumber,
									uiCurrentCountryCode,
									Language_Code,
									eType,
									ucMagazine_Number,
									ucPage_Number
					));
				}
			}
		}
		
		return 0;
	}

	public boolean updateChannel(DB_SERVICE_Data serviceDB)
	{
		DxbLog(D, "updateChannel(" + serviceDB + ")");

		SQLiteDatabase channel = mSQLiteDatabase;
		if(channel != null)
		{
			ContentValues values= new ContentValues();
			values.put(KEY_AUDIO_PID, serviceDB.uiAudioPID);
			values.put(KEY_VIDEO_PID, serviceDB.uiVideoPID);
			values.put(KEY_SCRAMBLING, serviceDB.uiVideoIsScrambling);
			values.put(KEY_AUDIO_STREAM_TYPE, serviceDB.uiAudioStreamType);
			values.put(KEY_VIDEO_STREAM_TYPE, serviceDB.uiVideoStreamType);
			values.put(KEY_PCR_PID, serviceDB.uiPCRPID);
			values.put(KEY_TOTAL_AUDIO_PID, serviceDB.uiTotalAudioPID);
			values.put(KEY_TOTAL_SUBTITLE, serviceDB.uiTotalCntSubtLang);
			values.put(KEY_PMT_VERSION_NUM, serviceDB.uiPMTVersionNum);
			synchronized(channel)
			{
				if (channel.isOpen())
				{
					return channel.update(	CHANNEL_DATABASE_TABLE,
											values,
												KEY_SERVICE_ID + "=" + serviceDB.uiServiceID + " AND " +
												KEY_CHANNEL_NUMBER + "=" + serviceDB.uiCurrentChannelNumber + " AND " +
												KEY_COUNTRY_CODE + "=" + serviceDB.uiCurrentCountryCode,
											null
								) > 0;
				}
			}
		}
		
		return false;
	}

	public boolean updateBookmark(long rowId, int iBookmark)
	{
		DxbLog(D, "updateBookmark()");

		SQLiteDatabase channel = mSQLiteDatabase;
		if(channel != null)
		{
			ContentValues values= new ContentValues();
			values.put(KEY_BOOKMARK, iBookmark);
			synchronized(channel) {
				if (channel.isOpen())
					return channel.update(CHANNEL_DATABASE_TABLE, values, KEY_ROWID + "=" + rowId, null) > 0;
			}
		}
		return false;
	}

	public boolean deleteChannel(long rowId)
	{
		DxbLog(D, "deleteChannel()");

		SQLiteDatabase channel = mSQLiteDatabase;
		if(channel != null)
		{
			synchronized(channel) {
				if (channel.isOpen())
					return channel.delete(CHANNEL_DATABASE_TABLE, KEY_ROWID + "=" + rowId, null) > 0;
			}
		}
		return false;
	}

	public boolean deleteAllChannel()
	{
		DxbLog(D, "deleteAllChannel()");
		SQLiteDatabase channel = mSQLiteDatabase;
		if(channel != null)
		{
			synchronized(channel) {
				if (channel.isOpen())
					return channel.delete(CHANNEL_DATABASE_TABLE, null, null) > 0;
			}
		}
		return false;
	}

	public boolean deleteAudioPIDTable(long rowId)
	{
		DxbLog(D, "deleteAudioPIDTable()");

		SQLiteDatabase channel = mSQLiteDatabase;
		if (channel != null)
		{
			synchronized(channel)
			{
				if (channel.isOpen())
				{
					return channel.delete(AUDIOPID_DATABASE_TABLE, KEY_ROWID + "=" + rowId, null) > 0;
				}
			}
		}
		
		return false;
	}

	public boolean deleteAudioPIDTable(int uiServiceID, int uiCurrentChannelNumber, int uiCurrentCountryCode)
	{
		SQLiteDatabase channel = mSQLiteDatabase;
		if (channel != null)
		{
			synchronized(channel)
			{
				if (channel.isOpen())
				{
					return channel.delete(	AUDIOPID_DATABASE_TABLE,
											KEY_SERVICE_ID + "=" + uiServiceID + " AND " +
											KEY_CHANNEL_NUMBER + "=" + uiCurrentChannelNumber + " AND " +
											KEY_COUNTRY_CODE + "=" + uiCurrentCountryCode,
											null
								) > 0;
				}
			}
		}
		return false;
	}

	public boolean deleteAllAudioPIDTable()
	{
		DxbLog(D, "deleteAllAudioPIDTable()");

		SQLiteDatabase channel = mSQLiteDatabase;
		if (channel != null)
		{
			synchronized(channel)
			{
				if (channel.isOpen())
				{
					return channel.delete(AUDIOPID_DATABASE_TABLE, null, null) > 0;
				}
			}
		}
		return false;
	}

	public boolean deleteSubtitleTable(long rowId)
	{
		DxbLog(D, "deleteSubtitleTable()");

		SQLiteDatabase channel = mSQLiteDatabase;
		if (channel != null)
		{
			synchronized(channel)
			{
				if (channel.isOpen())
				{
					return channel.delete(SUBTITLE_DATABASE_TABLE, KEY_ROWID + "=" + rowId, null) > 0;
				}
			}
		}
		return false;
	}

	public boolean deleteSubtitleTable(int uiServiceID, int uiCurrentChannelNumber, int uiCurrentCountryCode)
	{
		SQLiteDatabase channel = mSQLiteDatabase;
		if (channel != null)
		{
			synchronized(channel)
			{
				if (channel.isOpen())
				{
					return channel.delete(	SUBTITLE_DATABASE_TABLE,
											KEY_SERVICE_ID + "=" + uiServiceID + " AND " +
											KEY_CHANNEL_NUMBER + "=" + uiCurrentChannelNumber + " AND " +
											KEY_COUNTRY_CODE + "=" + uiCurrentCountryCode,
											null
								) > 0;
				}
			}
		}
		
		return false;
	}

	public boolean deleteAllSubtitleTable()
	{
		DxbLog(D, "deleteAllSubtitleTable()");

		SQLiteDatabase channel = mSQLiteDatabase;
		if (channel != null)
		{
			synchronized(channel)
			{
				if (channel.isOpen())
				{
					return channel.delete(SUBTITLE_DATABASE_TABLE, null, null) > 0;
				}
			}
		}
		
		return false;
	}

	public boolean deleteTeleTextDescriptor(long rowId)
	{
		DxbLog(D, "deleteTeleTextDescriptor()");

		SQLiteDatabase channel = mSQLiteDatabase;
		if (channel != null)
		{
			synchronized(channel)
			{
				if (channel.isOpen())
				{
					return channel.delete(TTX_DESCRIPTOR_DATABASE_TABLE, KEY_ROWID + "=" + rowId, null) > 0;
				}
			}
		}
		
		return false;
	}

	public boolean deleteAllTeleTextDescriptor()
	{
		DxbLog(D, "deleteAllTeleTextDescriptor()");

		SQLiteDatabase channel = mSQLiteDatabase;
		if (channel != null)
		{
			synchronized(channel)
			{
				if (channel.isOpen())
				{
					return channel.delete(TTX_DESCRIPTOR_DATABASE_TABLE, null, null) > 0;
				}
			}
		}
		
		return false;
	}

	public Cursor getAllChannels(int ServiceType)
	{
		DxbLog(D, "getAllChannels()");

		SQLiteDatabase channel = mSQLiteDatabase;
		if(channel != null)
		{
			String	queryService;
			if(ServiceType == 1)
			{
				/*	TV - ETSI EN 300 468 V1.11.1(2010-04)
				 * 			0x01(01)	: digital television service (see note 1)
				 * 			0x16(22)	: advanced codec SD digital television service
				 * 			0x19(25)	: advanced codec HD digital television service
				 * 	*/
						//	(vidoePID!=0xff) OR (audioPID!=0xff)
				queryService	=		"("
									+	KEY_VIDEO_PID
									+	"!=65535"					+ " OR "
									+	KEY_AUDIO_PID
									+	"!=65535"					+ ") AND ("
									+	KEY_SERVICE_TYPE
									+	"=" + SERVICE_TYPE_DTV		+ " OR "
									+	KEY_SERVICE_TYPE
									+	"=" + SERVICE_TYPE_MHDTV	+ " OR "
									+	KEY_SERVICE_TYPE
									+	"=" + SERVICE_TYPE_ASDTV	+ " OR "
									+	KEY_SERVICE_TYPE
									+	"=" + SERVICE_TYPE_AHDTV	+ ")";
			}
			else
			{
				queryService	=		KEY_AUDIO_PID
									+	"!=65535"					+ " AND "
									+	KEY_SERVICE_TYPE
									+	"=" + SERVICE_TYPE_DRADIO	+ " OR "
									+	KEY_SERVICE_TYPE
									+	"=" + SERVICE_TYPE_FMRADIO	+ " OR "
									+	KEY_SERVICE_TYPE
									+	"=" + SERVICE_TYPE_ADRADIO;
			}

			synchronized(channel) {
				if (channel.isOpen())
				{
					return channel.query(CHANNEL_DATABASE_TABLE,
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
							queryService,
							null, null, null, KEY_LOGICALCHANNEL, null);
				}
			}
		}
		return null;
	}

	public Cursor getAudioPIDQuery(int serviceID, int channelNumber, int countryCode) throws SQLException
	{
		DxbLog(D, "getAudioPIDQuery()");

		SQLiteDatabase channel = mSQLiteDatabase;
		if(channel != null)
		{
			synchronized(channel) {
				if (channel.isOpen())
				{
					Cursor cursor = channel.query(true, AUDIOPID_DATABASE_TABLE,
						new String[] {
										KEY_ROWID,
										KEY_STREAM_AUDIO_PID,
										KEY_STREAM_TYPE,
										KEY_AUDIO_TYPE,
										KEY_AC_LANGCODE
										},
						KEY_US_SERVICE_ID + "=" + serviceID + " AND " + KEY_EPG_CHANNEL_NUMBER + "=" + channelNumber + " AND " +KEY_EPG_COUNTRY_CODE+ "=" + countryCode, null, null, null, null, null);

					if (cursor != null)
					{
						cursor.moveToFirst();
					}
					return cursor;
				}
			}
		}
		return null;
	}

	public Cursor getTTX_Descriptor(int serviceID, int channelNumber, int countryCode, int iType)
	{
		DxbLog(D, "getTTX_Descriptor()");

		SQLiteDatabase channel = mSQLiteDatabase;
		if(channel != null)
		{
			synchronized(channel) {
				if (channel.isOpen())
				{
					Cursor cursor = channel.query(true, TTX_DESCRIPTOR_DATABASE_TABLE,
							new String[] {
											KEY_ROWID,
											KEY_TTX_LANGUAGE_CODE,
											KEY_TTX_MAGAZINE_NUMBER,
											KEY_TTX_PAGE_NUMBER
											},
								KEY_SERVICE_ID+"="+serviceID+" AND "+KEY_CHANNEL_NUMBER+"="+channelNumber+" AND "+KEY_COUNTRY_CODE+"="+countryCode+" AND "+KEY_TTX_TYPE+"="+iType,
								null, null, null, null, null);

					if(cursor != null)
					{
						cursor.moveToFirst();
					}
					return cursor;
				}
			}
		}
		return null;
	}

	public EpgData[] getEPG_PF_Information(int serviceID, int channelNumber, int countryCode) throws SQLException
	{
		DxbLog(D, "getEPG_PF_Information()");

		SQLiteDatabase epg = mSQLiteDatabaseEPG;
		if(epg == null)
		{
			return null;
		}
		
		Cursor cursor	= null;
		EpgData[] data = new EpgData[2];
		synchronized(epg) {
			if(epg.isOpen())
			{
				cursor = epg.query(true, EPG_PF_DATABASE_TABLE,
					new String[] {
						KEY_ROWID,
						KEY_EPG_START_MJD,
						KEY_EPG_START_HOUR,
						KEY_EPG_START_MINUTE,
						KEY_EPG_DURATION_HOUR,
						KEY_EPG_DURATION_MINUTE,
						KEY_EPG_EVT_NAME,
						KEY_EPG_EVT_TEXT,
						KEY_EPG_EVT_EXTERN,
						KEY_EPG_RATING
						},
					KEY_EPG_SERVICE_ID + "=" + serviceID + " AND " + KEY_EPG_CHANNEL_NUMBER + "=" + channelNumber + " AND " + KEY_EPG_COUNTRY_CODE+ "=" + countryCode, null, null, null,
					KEY_EPG_START_MJD + ", " + KEY_EPG_START_HOUR + ", " + KEY_EPG_START_MINUTE,
					null);

				if (cursor != null)
				{
					if (cursor.moveToFirst())
					{
						for (int i = 0; i < 2; i++)
						{
							data[i] = new EpgData();
							data[i].iStartHH = cursor.getInt(cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_START_HOUR));
							data[i].iStartMM = cursor.getInt(cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_START_MINUTE));
							data[i].iDurationHH	= cursor.getInt(cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_DURATION_HOUR));
							data[i].iDurationMM	= cursor.getInt(cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_DURATION_MINUTE));
							data[i].sName = cursor.getString(cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_EVT_NAME));
							data[i].iRating	= cursor.getInt(cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_RATING));
							if (!cursor.moveToNext())
								break;
						}
					}
					cursor.close();
				}
			}
		}
		
		return data;
	}

	public Cursor getEPG_SCHEDULE_Information(int serviceID, int channelNumber, int countryCode) throws SQLException
	{
		DxbLog(D, "getEPG_SCHEDULE_Information()");

		SQLiteDatabase epg = mSQLiteDatabaseEPG;
		if(epg == null)
		{
			return null;
		}

		Cursor cursor	= null;
		synchronized(epg) {
			if(epg.isOpen())
			{
				cursor = epg.query(true, EPG_SCHEDULE_DATABASE_TABLE,
						new String[] {
										KEY_ROWID,
										KEY_EPG_START_MJD,
										KEY_EPG_START_HOUR,
										KEY_EPG_START_MINUTE,
										KEY_EPG_DURATION_HOUR,
										KEY_EPG_DURATION_MINUTE,
										KEY_EPG_EVT_NAME,
										KEY_EPG_EVT_TEXT,
										KEY_EPG_EVT_EXTERN,
										KEY_EPG_RATING
									},
							KEY_EPG_SERVICE_ID + "=" + serviceID + " AND " + KEY_EPG_CHANNEL_NUMBER + "=" + channelNumber + " AND " + KEY_EPG_COUNTRY_CODE+ "=" + countryCode, null, null, null,
							KEY_EPG_START_MJD + ", " + KEY_EPG_START_HOUR + ", " + KEY_EPG_START_MINUTE,
							null);

				if (cursor != null)
				{
					cursor.moveToFirst();
				}
			}
		}
		
		return cursor;
	}
	
	public Cursor getSubtitle_LanguageCode(int ServiceID, int ChannelNumber)
	{
		DxbLog(D, "getSubtitle_LanguageCode()");

		SQLiteDatabase channel = mSQLiteDatabase;
		if(channel != null)
		{
			synchronized(channel) {
				if (channel.isOpen())
				{
					Cursor cursor	= channel.query(true, SUBTITLE_DATABASE_TABLE,
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
				}
			}
		}
		return null;
	}

	public Cursor getAudio_LanguageCode(int ServiceID, int ChannelNumber)
	{
		DxbLog(D, "getAudio_LanguageCode()");

		SQLiteDatabase channel = mSQLiteDatabase;
		if(channel != null)
		{
			synchronized(channel) {
				if (channel.isOpen())
				{
					Cursor cursor	= channel.query(true, AUDIOPID_DATABASE_TABLE,
							new String[] {
											KEY_ROWID,
											KEY_AUDIO_LANGUAGE_CODE,
											KEY_AUDIO_STREAM_TYPE
											},
							KEY_AUDIO_SERVICE_ID + "=" + ServiceID + " AND " + KEY_AUDIO_CHANNEL_NUMBER + "=" + ChannelNumber,
							null, null, null, null, null);

					if(cursor != null)
					{
						cursor.moveToFirst();
					}
					return cursor;
				}
			}
		}
		return null;
	}

	public boolean eliminateChannel()
	{

		Log.i(TAG, "eliminateChannel()");

		int iIndex_logicalChannel, logicalChannel;
		int iIndex_channelSnr, channelSnr;
		int iIndex_ServiceName;
		int iIndex_id, id;
		String serviceName;
		int total, i,j;
		Cursor cursor=null;

		SQLiteDatabase channel = mSQLiteDatabase;
		if(channel != null)
		{
			synchronized(channel)
			{
				if (channel.isOpen())
				{
					cursor	= channel.query(true,
											CHANNEL_DATABASE_TABLE,
											new String[] {	KEY_ROWID,
															KEY_CHANNEL_NAME,
															KEY_LOGICALCHANNEL,
															KEY_CHANNEL_SNR	},
											null, null, null, null, KEY_LOGICALCHANNEL, null);
				}
			}
		}

		if(cursor != null)
		{
			total = cursor.getCount();
			iIndex_ServiceName	= cursor.getColumnIndexOrThrow(ChannelManager.KEY_CHANNEL_NAME);
			iIndex_logicalChannel	= cursor.getColumnIndexOrThrow(ChannelManager.KEY_LOGICALCHANNEL);
			iIndex_channelSnr		= cursor.getColumnIndexOrThrow(ChannelManager.KEY_CHANNEL_SNR);
			iIndex_id				= cursor.getColumnIndexOrThrow(ChannelManager.KEY_ROWID);

			for(i=0; i <total ; ++i)
			{
				cursor.moveToPosition(i);

				logicalChannel=cursor.getInt(iIndex_logicalChannel);
				serviceName=cursor.getString(iIndex_ServiceName);
				channelSnr = cursor.getInt(iIndex_channelSnr);
				id= cursor.getInt(iIndex_id);

				while(cursor.moveToNext())
				{
					if(		logicalChannel==cursor.getInt(iIndex_logicalChannel)
						&&	serviceName.equals(cursor.getString(iIndex_ServiceName))
					)
					{
						if(channelSnr < cursor.getInt(iIndex_channelSnr))
						{
							j=id;
							channelSnr = cursor.getInt(iIndex_channelSnr);
							id = cursor.getInt(iIndex_id);
						}
						else
						{
							j=cursor.getInt(iIndex_id);
						}
						
						Log.i(TAG, "eliminate rowid=" + j + "; channel=" + logicalChannel + "; name=" + serviceName + "; snr=" + channelSnr);
						deleteChannel(j);
						continue;
					}
					
					break;
				}
			}
			
			cursor.close();
			return true;
		}
		
		return false;
	}

	public void check_logicalChannel()
	{
		Log.i(TAG, "check_logicalChannel()");

		int iIndex_logicalChannel, iIndex_id;
		int	assign_channel_no	= ASSIGN_LOGICAL_CHANNEL_NO;
		int	iCount_Service;

		Cursor cursor=null;

		SQLiteDatabase channel = mSQLiteDatabase;
		if(channel != null)
		{
			synchronized(channel)
			{
				if (channel.isOpen())
				{
					 cursor	= channel.query(true,
							 				CHANNEL_DATABASE_TABLE,
											new String[] {	KEY_ROWID,
															KEY_FREQUENCY,
															KEY_LOGICALCHANNEL	},
											null, null, null, null, KEY_LOGICALCHANNEL + " desc", null);
				}
			}
		}

		if(cursor != null)
		{
			iCount_Service = cursor.getCount();
			if(iCount_Service > 0)
			{
				iIndex_logicalChannel	= cursor.getColumnIndexOrThrow(ChannelManager.KEY_LOGICALCHANNEL);
				iIndex_id				= cursor.getColumnIndexOrThrow(ChannelManager.KEY_ROWID);

				cursor.moveToFirst();
				int	iMax	= cursor.getInt(iIndex_logicalChannel);
				if(iMax >= ASSIGN_LOGICAL_CHANNEL_NO)
				{
					assign_channel_no	= iMax + 1;
				}

				do
				{
					if( cursor.getInt(iIndex_logicalChannel)==0)
					{
						setLogicalChannel(cursor.getInt(iIndex_id), assign_channel_no);
						++assign_channel_no;
					}
					if(cursor.isLast())
					{
						break;
					}
					
					cursor.moveToNext();
				}
				while(true);
			}
			
			cursor.close();
		}
	}
	
	private boolean setLogicalChannel(long rowId, int ilogicalchannel)
	{
		if(mSQLiteDatabase == null)
		{
			return false;
		}

		ContentValues values= new ContentValues();
		values.put(KEY_LOGICALCHANNEL , ilogicalchannel);
		
		return mSQLiteDatabase.update(CHANNEL_DATABASE_TABLE, values, KEY_ROWID + "=" + rowId, null) > 0;
	}

/*****************************************************************************************************************************************************************************
 *	Debug - log_message
 *****************************************************************************************************************************************************************************/
	protected static final int D = 0;
	protected static final int E = 1;
	protected static final int I = 2;
	protected static final int V = 3;
	protected static final int W = 4;

	protected final String TAG = "[[[" + getClass().getSimpleName() + "]]]";

	protected void DxbLog(int level, String mString) {
		switch(level) {
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

