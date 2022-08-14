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

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

import java.util.ArrayList;

import com.telechips.android.isdbt.player.isdbt.Channel;

public class ChannelManager
{
	private final static String TAG = "[[[ChannelManager]]]";
	
	//CAS Information
	public static final String KEY_FREE_CA_MODE = "fCA_Mode_free"; //Information in SDT
	public static final String KEY_SCRAMBLING = "ucVideo_IsScrambling"; //CA descriptor in PMT

    public static final String KEY_ROWID = "_id";
		
    public static final String KEY_CHANNEL_NUMBER		= "channelNumber";
    public static final String KEY_COUNTRY_CODE			= "countryCode";
    public static final String KEY_AUDIO_PID			= "audioPID";
    public static final String KEY_VIDEO_PID			= "videoPID";
    public static final String KEY_SUBTITLE_PID			= "stPID";
    public static final String KEY_SUPERIMPOSE_PID		= "siPID";
    public static final String KEY_TELETEXT_PID     	= "uiTeletext_PID";
    public static final String KEY_PMT_PID				= "PMT_PID";
    public static final String KEY_REMOCON_ID			= "remoconID";
    public static final String KEY_REGION_ID			= "regionID";
    public static final String KEY_THREE_DIGIT_NUMBER	= "threedigitNumber";
    public static final String KEY_TSTREAM_ID			= "TStreamID";
    public static final String KEY_BER_AVG				= "berAVG";
    public static final String KEY_SERVICETYPE_PID		= "serviceType";
    public static final String KEY_SERVICE_ID			= "serviceID";
    public static final String KEY_CHANNEL_NAME			= "channelName";
    public static final String KEY_SERVICE_TYPE			= "serviceType";
    public static final String KEY_PRESET				= "preset";
    public static final String KEY_NETWORKID			= "networkID";
    public static final String KEY_FREQUENCY			= "uiFrequency";
    public static final String KEY_BOOKMARK				= "uiBookmark";
    public static final String KEY_LOGICALCHANNEL		= "uiLogicalChannel";

    public static final String KEY_US_SERVICE_ID		= "usServiceID";
    public static final String KEY_STREAM_AUDIO_PID		= "uiAudio_PID";
    public static final String KEY_STREAM_TYPE			= "ucAudio_StreamType";
    public static final String KEY_AUDIO_TYPE			= "ucAudioType";
    public static final String KEY_AC_LANGCODE			= "acLangCode";
	
    public static final String KEY_EPG_TABLE_ID			= "uiTableId";
    public static final String KEY_EPG_SECTION_NUM		= "ucSection";
    public static final String KEY_EPG_EVENT_ID			= "EventID";
    public static final String KEY_EPG_START_MJD		= "Start_MJD";
    public static final String KEY_EPG_START_HOUR		= "Start_HH";
    public static final String KEY_EPG_START_MINUTE		= "Start_MM";
    public static final String KEY_EPG_START_SEC		= "Start_SS";
    public static final String KEY_EPG_DURATION_HOUR	= "Duration_HH";
    public static final String KEY_EPG_DURATION_MINUTE	= "Duration_MM";
    public static final String KEY_EPG_DURATION_SEC		= "Duration_SS";
    public static final String KEY_EPG_EVT_NAME			= "EvtName";
    public static final String KEY_EPG_EVT_TEXT			= "EvtText";
    public static final String KEY_EPG_EVT_EXTERN		= "ExtEvtItem";
    public static final String KEY_EPG_SERVICE_ID		= "usServiceID";
    public static final String KEY_EPG_CHANNEL_NUMBER	= "uiCurrentChannelNumber";
    public static final String KEY_EPG_COUNTRY_CODE		= "uiCurrentCountryCode";
    public static final String KEY_EPG_RATING			= "iRating";
    
    public static final String KEY_EPG_GENRE			= "Genre";
    public static final String KEY_EPG_AUDIO_MODE		= "AudioMode";
    public static final String KEY_EPG_AUDIO_SR			= "AudioSr";
    public static final String KEY_EPG_AUDIO_LANG1		= "AudioLang1";
    public static final String KEY_EPG_AUDIO_LANG2		= "AudioLang2";
    public static final String KEY_EPG_VIDEO_MODE		= "VideoMode";
    public static final String KEY_EPG_REF_SERVICE_ID	= "RefServiceID";
    public static final String KEY_EPG_REF_EVENT_ID		= "RefEventID";

    public static final String KEY_AUDIO_LANGUAGE_CODE  = "acLangCode";
    public static final String KEY_AUDIO_STREAM_TYPE    = "ucAudio_StreamType";
    public static final String KEY_AUDIO_SERVICE_ID     = "usServiceID";
    public static final String KEY_AUDIO_CHANNEL_NUMBER = "uiCurrentChannelNumber";

    public static final String KEY_SERVICE_LANGUAGE_CODE    = "usTotalCntSubtLang";
    public static final String KEY_SERVICE_SERVICE_ID       = "usServiceID";
    public static final String KEY_SERVICE_CHANNEL_NUMBER   = "uiCurrentChannelNumber";

    public static final String KEY_SUBTITLE_LANGUAGE_CODE   = "acSubtLangCode";
    public static final String KEY_SUBTITLE_SERVICE_ID      = "usServiceID";
    public static final String KEY_SUBTITLE_CHANNEL_NUMBER  = "uiCurrentChannelNumber";

    public static final String KEY_TTX_LANGUAGE_CODE        = "Language_Code";
    public static final String KEY_TTX_TYPE                 = "eType";
    public static final String KEY_TTX_MAGAZINE_NUMBER      = "ucMagazine_Number";
    public static final String KEY_TTX_PAGE_NUMBER          = "ucPage_Number";

    public static final int SERVICE_TYPE_DTV    = 0x01;
    public static final int SERVICE_TYPE_MHDTV  = 0x11;
    public static final int SERVICE_TYPE_ASDTV  = 0x16;
    public static final int SERVICE_TYPE_AHDTV  = 0x19;

    public static final int SERVICE_TYPE_DRADIO     = 0x02;
    public static final int SERVICE_TYPE_FMRADIO    = 0x07;
    public static final int SERVICE_TYPE_ADRADIO    = 0x0A;
    
	public static final int PRESET_MODE_MAX_COUNT	= 3;
	private ChannelDbAdapter[] mChannelDbAdapter = new ChannelDbAdapter[PRESET_MODE_MAX_COUNT];
	private SQLiteDatabase[] mPresetChannelDB = new SQLiteDatabase[PRESET_MODE_MAX_COUNT];
	private EPGDbAdapter[] mEPGDbAdapter = new EPGDbAdapter[PRESET_MODE_MAX_COUNT];
	private static SQLiteDatabase[] mSQLiteDatabaseEPG = new SQLiteDatabase[PRESET_MODE_MAX_COUNT];
    protected SQLiteDatabase mSQLiteDatabase;

	private int mPresetMode = 0;	/* index for preset mode. 0 ~ (PRESET_MODE_MAX_COUNT-1) */

    protected static final String DATABASE_NAME = "ISDBT.db";
    protected static final String CHANNEL_DATABASE_TABLE = "channels";
    protected static final String SERVICE_DATABASE_TABLE = "services";
    protected static final String AUDIOPID_DATABASE_TABLE = "audioPID";
    protected static final String SUBTITLE_DATABASE_TABLE   = "subTitle";
    protected static final String TTX_DESCRIPTOR_DATABASE_TABLE = "TTX_Descriptor";

    private static final String DATABASE_EPG_NAME = "/data/data/com.telechips.android.isdbt/databases/ISDBTEPG.db";
    private static final String EPG_PF_DATABASE_TABLE = "EPG_PF";
    private static final String EPG_SCHEDULE_DATABASE_TABLE = "EPG_Sche";

    protected static final int DATABASE_VERSION = 2;

    private static final String CHANNEL_DATABASE_CREATE =
    		"create table if not exists channels (_id integer primary key, "
    		+ "channelNumber integer, countryCode integer, audioPID integer, videoPID integer, stPID integer, siPID integer, PMT_PID integer, remoconID integer, serviceType integer, "
    		+ "serviceID integer, regionID integer, threedigitNumber integer, TStreamID integer, berAVG integer, channelName text not null, TSName text not null, preset integer, networkID integer, areaCode integer, frequency blob, affiliationID blob);";
    		
    private final Context mContext;

    private static class ChannelDbAdapter  extends SQLiteOpenHelper
    {
        ChannelDbAdapter (Context context, String name)
        {
            super(context, name, null, DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase database)
        {
            Log.d(TAG, "onCreate()");
		
            database.execSQL(CHANNEL_DATABASE_CREATE);
        }

        @Override
        public void onUpgrade(SQLiteDatabase database, int oldVersion, int newVersion)
        {
            Log.d(TAG, "onUpgrade()");
		
            database.execSQL("DROP TABLE IF EXISTS channels");
            onCreate(database);
        }
    }

    private static class EPGDbAdapter  extends SQLiteOpenHelper
    {
        EPGDbAdapter (Context context, String name)
        {
            super(context, name, null, DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase database)
        {
			Log.d(TAG, "EPGDbAdapter.onCrate - file open:" + database.getPath());
        }

        @Override
        public void onUpgrade(SQLiteDatabase database, int oldVersion, int newVersion)
        {
        }
    }

    public ChannelManager(Context context)
    {
        Log.d(TAG, "ChannelManager()");
				
        mContext= context;
    }

    public ChannelManager open() throws SQLException
    {
	Log.d(TAG, "open()");
	for (int cnt=0; cnt < PRESET_MODE_MAX_COUNT; cnt++)
	{
		if(mChannelDbAdapter[cnt] != null) mChannelDbAdapter[cnt].close();
		mChannelDbAdapter[cnt] = new ChannelDbAdapter (mContext, "ISDBT"+Integer.toString(cnt)+".db");
		
		if(mPresetChannelDB[cnt] != null) mPresetChannelDB[cnt].close();
		mPresetChannelDB[cnt] = mChannelDbAdapter[cnt].getWritableDatabase();
        }
		
        return this;
    }
		
	protected void close_epg_db ()
        {
		for (int cnt=0; cnt < PRESET_MODE_MAX_COUNT; cnt++)
		{
			if(mSQLiteDatabaseEPG[cnt] != null) {
				mSQLiteDatabaseEPG[cnt].close();
				mSQLiteDatabaseEPG[cnt] = null;
			}
			if (mEPGDbAdapter[cnt] != null) {
				mEPGDbAdapter[cnt].close();
				mEPGDbAdapter[cnt] = null;
			}
		}
	}
    public void close()
    {
        Log.d(TAG, "close()");
				
		close_epg_db();

		for (int cnt=0; cnt < PRESET_MODE_MAX_COUNT; cnt++)
        {
			if(mPresetChannelDB[cnt] != null) {
				mPresetChannelDB[cnt].close();
				mPresetChannelDB[cnt] = null;
        }
			if(mChannelDbAdapter[cnt] != null) {
				mChannelDbAdapter[cnt].close();
				mChannelDbAdapter[cnt] = null;
        }
        }
    }
	
    public boolean openEPGDB()
    {
        try
        {
		for (int cnt=0; cnt < PRESET_MODE_MAX_COUNT; cnt++)
		{
			if(mSQLiteDatabaseEPG[cnt] == null)
			{
				if(mEPGDbAdapter[cnt] != null) mEPGDbAdapter[cnt].close();
				mEPGDbAdapter[cnt] = new EPGDbAdapter (mContext, "ISDBTEPG"+Integer.toString(cnt)+".db");

				if (mSQLiteDatabaseEPG[cnt] != null) mSQLiteDatabaseEPG[cnt].close();
				mSQLiteDatabaseEPG[cnt] = mEPGDbAdapter[cnt].getWritableDatabase();
				Log.d(TAG, "EPG:" + mSQLiteDatabaseEPG[cnt].getPath() + " Open Success");
				return true;
			}
		}
		return true;
        }
        catch(Exception e)
        {
            Log.e(TAG, "Couldn't open " + DATABASE_EPG_NAME + "Exception" + e);
            close_epg_db();
            Log.e(TAG, DATABASE_EPG_NAME + "Open Fail");
            return false;
        }		
	    }
	
	private boolean checkSanityChannelDB()
	{
		if (mPresetMode < 0 || mPresetMode >= PRESET_MODE_MAX_COUNT) return false;
		if (mPresetChannelDB[mPresetMode] == null) return false;
		return true;
	}

    public long insertChannel(int channelNumber, int countryCode, int audioPID, int videoPID, int serviceID, String channelName)
    {
        Log.d(TAG, "insertChannel()");
		
        return 0;
    }
	
    public boolean updateChannel(int rowId, int channelNumber, int countryCode, int audioPID, int videoPID, int serviceID, String channelName)
    {		
        Log.d(TAG, "updateChannel()");
		
        return false;
    }
	
    public boolean updateBookmark(long rowId, int iBookmark)
    {
		return false;
    }
	
    public boolean deleteChannel(long rowId)
    {
        Log.d(TAG, "deleteChannel()");
		
	if (checkSanityChannelDB() == true)
	        return mPresetChannelDB[mPresetMode].delete(CHANNEL_DATABASE_TABLE, KEY_ROWID + "=" + rowId, null) > 0;
	else
            return false;
    }
		
    public boolean deleteAllChannel()
    {
        Log.d(TAG, "deleteAllChannel()");

	if (checkSanityChannelDB() == true)
		return mPresetChannelDB[mPresetMode].delete(CHANNEL_DATABASE_TABLE, null, null) > 0;
	else
            return false;
    }
		
    public DxbChannelData getDxbChannelData(Cursor c)
    {
        if((c == null) || (c.getCount() <= 0))
        {
            Log.e(TAG, "cursor is null");
            return null;
        }

        DxbChannelData channelData = new DxbChannelData(c.getInt(0), //ID
                                                        c.getInt(1), //channelNumber
                                                        c.getInt(2), //countryCode
                                                        c.getInt(9), //serviceType
                                                        c.getInt(3), //audioPID
                                                        c.getInt(4), //videoPID
                                                        c.getInt(7), //serviceID
                                                        c.getString(8), //channelNmae
                                                        c.getInt(5), //pmtPID
                                                        c.getInt(6), //remoconID
                                                        c.getInt(10), //regionID
                                                        c.getInt(11), //threeDigitNumber
                                                        c.getInt(12), //tstreamID
                                                        c.getInt(13), //berAvg
                                                        c.getInt(14), //preset
                                                        c.getInt(15)); //networkID
				
        return channelData;
    }
    
    public DxbChannelData getDxbChannelData(int rowId, int channelNumber) throws SQLException 
    {
        String queryService;
        queryService  =     "(("
                            +   KEY_VIDEO_PID
                            +   "!=65535"               + " AND "
                            +   KEY_VIDEO_PID
                            +   ">0"                    + ") OR ("
                            +   KEY_AUDIO_PID
                            +   "!=65535"               + " AND "
                            +   KEY_AUDIO_PID
                            +   ">0"                    + ")) ";
		
        if(rowId >= 0) {
            queryService += "AND "
                            +	KEY_ROWID
                            +	"==" + rowId 			+ " ";
        }

        if(channelNumber >= 0) {
            queryService += "AND "
                            +	KEY_CHANNEL_NUMBER
                            +	"==" + channelNumber 	+ " ";
        }

        String orderby;
        orderby =        KEY_REMOCON_ID
                        +   ", "
                        +   KEY_THREE_DIGIT_NUMBER;

		Cursor cursor;
		if(checkSanityChannelDB() == true) {
        	cursor = mPresetChannelDB[mPresetMode].query(CHANNEL_DATABASE_TABLE,
                        new String[] {
                            KEY_ROWID,
                            KEY_CHANNEL_NUMBER,
                            KEY_COUNTRY_CODE,
                            KEY_AUDIO_PID,
                            KEY_VIDEO_PID,
                            KEY_PMT_PID,
                            KEY_REMOCON_ID,
                            KEY_SERVICE_ID,
                            KEY_CHANNEL_NAME,
                            KEY_SERVICE_TYPE,
                            KEY_REGION_ID,
                            KEY_THREE_DIGIT_NUMBER,
                            KEY_TSTREAM_ID,
                            KEY_BER_AVG,
                            KEY_PRESET,
                            KEY_NETWORKID
                         },
                queryService,
                null, null, null, orderby);
		} else
			cursor = null;

        if(cursor != null)
        {
            if(cursor.getCount() > 0)
            {
                cursor.moveToFirst();
                DxbChannelData channel = getDxbChannelData(cursor);
                cursor.close();

                return channel;
            }

            cursor.close();
        }

        return null;
    }

    public ArrayList getSegChannels(int channelNumber, int remoconID, int networkID, int serviceType) throws SQLException
    {
        try {
            String queryService;
            queryService  =     "(("
                            +   KEY_VIDEO_PID
                            +   "!=65535"               + " AND "
                            +   KEY_VIDEO_PID
                            +   ">0"                    + ") OR ("
                            +   KEY_AUDIO_PID
                            +   "!=65535"               + " AND "
                            +   KEY_AUDIO_PID
                            +   ">0"                    + ")) AND ("
                            +	  KEY_CHANNEL_NUMBER
                            +	  "==" + channelNumber	+ " AND "
                            +   KEY_REMOCON_ID
                            +   "==" + remoconID        + " AND "
                            +	  KEY_NETWORKID
                            +	  "==" + networkID		+ " AND "
                            +   KEY_SERVICE_TYPE
                            +   "==" + serviceType + ")";

            String orderby;
            orderby =           KEY_REMOCON_ID
                            +   ", "
                            +   KEY_THREE_DIGIT_NUMBER;

			Cursor cursor;
			if (checkSanityChannelDB() == true) {
            	cursor = mPresetChannelDB[mPresetMode].query(CHANNEL_DATABASE_TABLE,
                            new String[] {
                                KEY_ROWID,
                                KEY_CHANNEL_NUMBER,
                                KEY_COUNTRY_CODE,
                                KEY_AUDIO_PID,
                                KEY_VIDEO_PID,
                                KEY_PMT_PID,
                                KEY_REMOCON_ID,
                                KEY_SERVICE_ID,
                                KEY_CHANNEL_NAME,
                                KEY_SERVICE_TYPE,
                                KEY_REGION_ID,
                                KEY_THREE_DIGIT_NUMBER,
                                KEY_TSTREAM_ID,
                                KEY_BER_AVG,
                                KEY_PRESET,
                                KEY_NETWORKID
	                    },
	                    queryService,
	                    null, null, null, orderby);
			} else 
				cursor = null;

            if(cursor != null)
            {
                int count = cursor.getCount();
                if(count > 0)
                {
                    ArrayList<DxbChannelData> channelList = new ArrayList<DxbChannelData>();

                    cursor.moveToFirst();
                    for(int i=0; i<count; i++)
                    {
                        channelList.add(getDxbChannelData(cursor));
                        cursor.moveToNext();
                    }
                    cursor.close();

            	    return channelList;
                }

                cursor.close();
            }

            return null;
        }
        catch (Exception e) {
            Log.e(TAG, "1seg channel table is not exist");
        }
		
        return null;
    }

    public ArrayList getSegChannels(int serviceType) throws SQLException
    {
        try {
            String queryService;
            queryService  =     "(("
                            +   KEY_VIDEO_PID
                            +   "!=65535"               + " AND "
                            +   KEY_VIDEO_PID
                            +   ">0"                    + ") OR ("
                            +   KEY_AUDIO_PID
                            +   "!=65535"               + " AND "
                            +   KEY_AUDIO_PID
                            +   ">0"                    + ")) AND ("
                            +   KEY_SERVICE_TYPE		
                            +   "==" + serviceType + ")";

            String orderby;
            orderby =           KEY_REMOCON_ID
                            +   ", "
                            +	KEY_NETWORKID
                            +   ", "
                            +   KEY_THREE_DIGIT_NUMBER;

			Cursor cursor;
			if (checkSanityChannelDB() == true) {
            	cursor = mPresetChannelDB[mPresetMode].query(CHANNEL_DATABASE_TABLE,
                            new String[] {
                                KEY_ROWID,
                                KEY_CHANNEL_NUMBER,
                                KEY_COUNTRY_CODE,
                                KEY_AUDIO_PID,
                                KEY_VIDEO_PID,
                                KEY_PMT_PID,
                                KEY_REMOCON_ID,
                                KEY_SERVICE_ID,
                                KEY_CHANNEL_NAME,
                                KEY_SERVICE_TYPE,
                                KEY_REGION_ID,
                                KEY_THREE_DIGIT_NUMBER,
                                KEY_TSTREAM_ID,
                                KEY_BER_AVG,
                                KEY_PRESET,
                                KEY_NETWORKID
	                    },
	                    queryService,
	                    null, null, null, orderby);
			} else 
				cursor = null;

            if(cursor != null)
            {
                int count = cursor.getCount();
                if(count > 0)
                {
                    ArrayList<DxbChannelData> channelList = new ArrayList<DxbChannelData>();

                    cursor.moveToFirst();
                    for(int i=0; i<count; i++)
                    {
               	        channelList.add(getDxbChannelData(cursor));
                    	cursor.moveToNext();
                    }
                    cursor.close();

                    return channelList;
                }

                cursor.close();
            }

            return null;      
        }
        catch (Exception e) {
            Log.e(TAG, "1seg channel table is not exist");
        }
		
        return null;
	}
/*
    public ArrayList getAllChannels()
    {
        String queryService;
        queryService =      "("
                            +   KEY_VIDEO_PID
                            +   "!=65535"               + " AND "
                            +   KEY_VIDEO_PID
                            +   ">0"                    + ") OR ("
                            +   KEY_AUDIO_PID
                            +   "!=65535"               + " AND "
                            +   KEY_AUDIO_PID
                            +   ">0"                    + ")";

        String groupby;
        groupby =           KEY_TSTREAM_ID;

        String having;
        having =            "MAX("
                        +   KEY_BER_AVG
                        +   ")";

        String orderby;
        orderby =           KEY_REMOCON_ID
                        +   ", "
                        +   KEY_THREE_DIGIT_NUMBER;

	Cursor cursor;
	if (checkSanityChannelDB() == true ) {
        	cursor =  mPresetChannelDB[mPresetMode].query(CHANNEL_DATABASE_TABLE,
                         new String[] {
                            KEY_ROWID,
                            KEY_CHANNEL_NUMBER,
                            KEY_COUNTRY_CODE,
                            KEY_AUDIO_PID,
                            KEY_VIDEO_PID,
                            KEY_PMT_PID,
                            KEY_REMOCON_ID,
                            KEY_SERVICE_ID,
                            KEY_CHANNEL_NAME,
                            KEY_SERVICE_TYPE,
                            KEY_REGION_ID,
                            KEY_THREE_DIGIT_NUMBER,
                            KEY_TSTREAM_ID,
                            KEY_BER_AVG,
                            KEY_PRESET,
                            KEY_NETWORKID
                        },
                        queryService,
                        null, null, null, orderby);
         } else
              cursor= null;

        if(cursor != null)
        {
            int count = cursor.getCount();
            if(count > 0) 
            {
                ArrayList<DxbChannelData> channelList = new ArrayList<DxbChannelData>();

                cursor.moveToFirst();
                for(int i=0; i<count; i++)
                {
                    channelList.add(getDxbChannelData(cursor));
                    cursor.moveToNext();
                }
                cursor.close();
 
                return channelList;
            }

            cursor.close();
        }

        return null;
    }
*/
    public ArrayList getAllChannels()
    {
        try {
            String sql =
                    "SELECT "
                        + "a." + KEY_ROWID + ", "
                        + "a." + KEY_CHANNEL_NUMBER + ", "
                        + "a." + KEY_COUNTRY_CODE + ", "
                        + "a." + KEY_AUDIO_PID + ", "
                        + "a." + KEY_VIDEO_PID + ", "
                        + "a." + KEY_PMT_PID + ", "
                        + "a." + KEY_REMOCON_ID + ", "
                        + "a." + KEY_SERVICE_ID + ", "
                        + "a." + KEY_CHANNEL_NAME + ", "
                        + "a." + KEY_SERVICE_TYPE + ", "
                        + "a." + KEY_REGION_ID + ", "
                        + "a." + KEY_THREE_DIGIT_NUMBER + ", "
                        + "a." + KEY_TSTREAM_ID + ", "
                        + "a." + KEY_BER_AVG + ", "
                        + "a." + KEY_PRESET + ", "
                        + "a." + KEY_NETWORKID + " "
                    + "FROM "
                        + "(SELECT * FROM " + CHANNEL_DATABASE_TABLE + " "
                            + "WHERE ((" + KEY_VIDEO_PID + "!=65535 AND " + KEY_VIDEO_PID + ">0) OR (" +  KEY_AUDIO_PID + "!=65535 AND " + KEY_AUDIO_PID + ">0)) AND (" + KEY_SERVICE_TYPE + "!=192) "
                            + ") a, "
                        + "(SELECT " + KEY_CHANNEL_NUMBER + ", MIN(" + KEY_SERVICE_ID + ") as \"min_ServiceID\" FROM " + CHANNEL_DATABASE_TABLE + " "
                            + "WHERE ((" + KEY_VIDEO_PID + "!=65535 AND " + KEY_VIDEO_PID + ">0) OR (" +  KEY_AUDIO_PID + "!=65535 AND " + KEY_AUDIO_PID + ">0)) AND (" + KEY_SERVICE_TYPE + "!=192) "
                            + "GROUP BY " + KEY_CHANNEL_NUMBER + ", " + KEY_AUDIO_PID + ", " + KEY_VIDEO_PID + ", " + KEY_SUBTITLE_PID + ", " + KEY_SUPERIMPOSE_PID + " "
                            + ") b "
                    + "WHERE (a." + KEY_CHANNEL_NUMBER + "==b." + KEY_CHANNEL_NUMBER + ") AND (a." + KEY_SERVICE_ID + "==b.min_ServiceID) "
                    + "UNION "
                    + "SELECT "
                        + "a." + KEY_ROWID + ", "
                        + "a." + KEY_CHANNEL_NUMBER + ", "
                        + "a." + KEY_COUNTRY_CODE + ", "
                        + "a." + KEY_AUDIO_PID + ", "
                        + "a." + KEY_VIDEO_PID + ", "
	                    + "a." + KEY_PMT_PID + ", "
                        + "a." + KEY_REMOCON_ID + ", "
                        + "a." + KEY_SERVICE_ID + ", "
                        + "a." + KEY_CHANNEL_NAME + ", "
                        + "a." + KEY_SERVICE_TYPE + ", "
                        + "a." + KEY_REGION_ID + ", "
                        + "a." + KEY_THREE_DIGIT_NUMBER + ", "
                        + "a." + KEY_TSTREAM_ID + ", "
                        + "a." + KEY_BER_AVG + ", "
                        + "a." + KEY_PRESET + ", "
                        + "a." + KEY_NETWORKID + " "
                    + "FROM "
                        + "(SELECT * FROM " + CHANNEL_DATABASE_TABLE + " "
                            + "WHERE ((" + KEY_VIDEO_PID + "!=65535 AND " + KEY_VIDEO_PID + ">0) OR (" +  KEY_AUDIO_PID + "!=65535 AND " + KEY_AUDIO_PID + ">0)) AND (" + KEY_SERVICE_TYPE + "==192) "
                            + ") a, "
                        + "(SELECT " + KEY_CHANNEL_NUMBER + " FROM " + CHANNEL_DATABASE_TABLE + " "
                            + "WHERE (" + KEY_VIDEO_PID + "!=65535 AND " + KEY_VIDEO_PID + ">0) OR (" +  KEY_AUDIO_PID + "!=65535 AND " + KEY_AUDIO_PID + ">0) "
                            + "GROUP BY " + KEY_CHANNEL_NUMBER + " "
                            + "HAVING MIN(" + KEY_SERVICE_TYPE + ")==192"
                            + ") b "
                        + "WHERE a." + KEY_CHANNEL_NUMBER + "==" + "b." + KEY_CHANNEL_NUMBER + " "
                    + "ORDER BY " + KEY_REMOCON_ID + ", " + KEY_THREE_DIGIT_NUMBER + " ";

			if (checkSanityChannelDB() == false)
				return null;

            Cursor cursor = mPresetChannelDB[mPresetMode].rawQuery(sql, null);
            if(cursor != null)
            {
                int count = cursor.getCount();
                if(count > 0)
                {
                    ArrayList<DxbChannelData> channelList = new ArrayList<DxbChannelData>();

                    cursor.moveToFirst();
                    for(int i=0; i<count; i++)
                    {
                        channelList.add(getDxbChannelData(cursor));
                        cursor.moveToNext();
                    }
                    cursor.close();

                    return channelList;
                }

                cursor.close();
            }

            return null;
        }
        catch (Exception e) {
            Log.e(TAG, "Channel Table is not exist");
        }
		
        return null;
    }

	public Cursor getTTX_Descriptor(int serviceID, int channelNumber, int countryCode, int iType)
	{
		Log.d(TAG, "getTTX_Descriptor()");
		
		Cursor cursor;
		if (checkSanityChannelDB() == true) {
			cursor = mPresetChannelDB[mPresetMode].query(true, TTX_DESCRIPTOR_DATABASE_TABLE,
				new String[] {
								KEY_ROWID,
								KEY_TTX_LANGUAGE_CODE,
								KEY_TTX_MAGAZINE_NUMBER,
								KEY_TTX_PAGE_NUMBER
								},
					KEY_SERVICE_ID+"="+serviceID+" AND "+KEY_CHANNEL_NUMBER+"="+channelNumber+" AND "+KEY_COUNTRY_CODE+"="+countryCode+" AND "+KEY_TTX_TYPE+"="+iType,
					null, null, null, null, null);
		
			if(cursor != null)
				cursor.moveToFirst();
		} else
			cursor = null;
		
		return cursor;
	}
	
    public DxbEPGData getDxbEPGData(Cursor c)
    {
        if((c == null) || (c.getCount() <= 0))
        {
            Log.e(TAG, "cursor is null");
            return null;
        }

        DxbEPGData epgData = new DxbEPGData(c.getInt(18), //tableID
                                            c.getInt(19), //sectionNumber
                                            c.getInt(20), //eventID
                                            c.getInt(1), //startMJD
                                            c.getInt(2), //startHH
                                            c.getInt(3), //startMM
                                            c.getInt(5), //durationHH
                                            c.getInt(6), //durationMM
                                            c.getString(8), //evtName
                                            c.getString(9), //evtText
                                            c.getString(10), //evtExtern
                                            c.getInt(11), //genre
                                            c.getInt(12), //audioMode
                                            c.getInt(14), //audioLang1
                                            c.getInt(15), //audioLang2
                                            c.getInt(16), //videoMode
                                            c.getInt(17)); //rating
				
        return epgData;
	}

	private boolean checkSanityEPGDB()
	{
		if (mPresetMode < 0 || mPresetMode >= PRESET_MODE_MAX_COUNT) return false;
		if (mSQLiteDatabaseEPG[mPresetMode] == null) return false;
		return true;
	}

    public ArrayList getEPG_PF(int serviceID, int channelNumber, int countryCode) throws SQLException
    {
        Cursor cursor = getEPG_PF_Information(serviceID, channelNumber, countryCode);
        if(cursor != null)
        {
            int count = cursor.getCount();
            if(count > 0)
            {
                ArrayList<DxbEPGData> epgList = new ArrayList<DxbEPGData>();

                    cursor.moveToFirst();
                    for(int i=0; i<count; i++)
                    {
               	        epgList.add(getDxbEPGData(cursor));
                    	cursor.moveToNext();
                    }
                    cursor.close();

                    return epgList;
                }

                cursor.close();
            }

            return null;      
    }

    public Cursor getEPG_PF_Information(int serviceID, int channelNumber, int countryCode) throws SQLException
    {
        try {
			if (checkSanityEPGDB() == false)
				return null;

            Cursor cursor = mSQLiteDatabaseEPG[mPresetMode].query(true, 
                EPG_PF_DATABASE_TABLE + "_" + channelNumber + "_0x" + Integer.toHexString(serviceID),
                    new String[] {
                        KEY_ROWID,
                        KEY_EPG_START_MJD,
                        KEY_EPG_START_HOUR,
                        KEY_EPG_START_MINUTE,
                        KEY_EPG_START_SEC,
                        KEY_EPG_DURATION_HOUR,
                        KEY_EPG_DURATION_MINUTE,
                        KEY_EPG_DURATION_SEC,
                        KEY_EPG_EVT_NAME,
                        KEY_EPG_EVT_TEXT,
                        KEY_EPG_EVT_EXTERN,
                        KEY_EPG_GENRE,
                        KEY_EPG_AUDIO_MODE,
                        KEY_EPG_AUDIO_SR,
                        KEY_EPG_AUDIO_LANG1,
                        KEY_EPG_AUDIO_LANG2,
                        KEY_EPG_VIDEO_MODE,
                        KEY_EPG_RATING,
                    	KEY_EPG_TABLE_ID,
                    	KEY_EPG_SECTION_NUM,
                    	KEY_EPG_EVENT_ID
                    },
                    KEY_EPG_SERVICE_ID + "=" + serviceID + " AND " + KEY_EPG_CHANNEL_NUMBER + "=" + channelNumber + " AND " + KEY_EPG_COUNTRY_CODE+ "=" + countryCode, null, null, null, KEY_EPG_TABLE_ID + "," + KEY_EPG_SECTION_NUM + "," +  KEY_EPG_START_MJD + "," + KEY_EPG_START_HOUR + " ," + KEY_EPG_START_MINUTE, null);
            if (cursor != null) {
                cursor.moveToFirst();
            }

            return cursor;
        }
        catch (Exception e) {
            Log.e(TAG, "EPG_PF Table is not exist");
        }
	
        return null;
    }

    public Cursor getEPG_PF_Information(int serviceID, int channelNumber, int countryCode, int mjd, int time, int prev_event_time) throws SQLException
    {
        try {
            int iDummy;
            int iStartMJD, iEndMJD;
            int iStartHH, iEndHH;
            String sql;
            Cursor cursor;
	
            if(prev_event_time < 0) {
                iDummy = 6;
            } else if (prev_event_time > time) {
                iDummy = (time + 24) - prev_event_time;
            } else {
                iDummy = time - prev_event_time;
            }
		
            iEndMJD = mjd;
            iEndHH = time;
			
            if(time < iDummy)
            {
                iStartMJD = mjd - 1;
                iStartHH = (time + 24) - iDummy;
                iEndMJD = mjd;
                iEndHH = time + 3;
            }
            else if ((time + 3) >= 24)
            {
                iStartMJD = mjd;
                iStartHH = time - iDummy;
                iEndMJD = mjd + 1;
                iEndHH = (time + 3) - 24;
            }
            else
            {
                iStartMJD = mjd;
                iStartHH = time - iDummy;
                iEndMJD = mjd;
                iEndHH = time + 3;
            }
			
            if(iStartMJD != iEndMJD)
            {
                sql = "SELECT "
                    + KEY_ROWID + ", "
                    + KEY_EPG_START_MJD + ", "
                    + KEY_EPG_START_HOUR + ", "
                    + KEY_EPG_START_MINUTE + ", "
                    + KEY_EPG_START_SEC + ", "
                    + KEY_EPG_DURATION_HOUR + ", "
                    + KEY_EPG_DURATION_MINUTE + ", "
                    + KEY_EPG_DURATION_SEC + ", "
                    + KEY_EPG_EVT_NAME + ", "
                    + KEY_EPG_EVT_TEXT + ", "
                    + KEY_EPG_EVT_EXTERN + ", "
                    + KEY_EPG_GENRE + ", "
                    + KEY_EPG_AUDIO_MODE + ", "
                    + KEY_EPG_AUDIO_SR + ", "
                    + KEY_EPG_AUDIO_LANG1 + ", "
                    + KEY_EPG_AUDIO_LANG2 + ", "
                    + KEY_EPG_VIDEO_MODE + ", "
                    + KEY_EPG_TABLE_ID + ", "
                    + KEY_EPG_SECTION_NUM + ", "
                    + KEY_EPG_EVENT_ID + ", "
                    + KEY_EPG_REF_SERVICE_ID + ", "
                    + KEY_EPG_REF_EVENT_ID + " "
                    + "FROM " + EPG_PF_DATABASE_TABLE + "_" + channelNumber + "_0x" + Integer.toHexString(serviceID) + " "
                    + "WHERE (" + KEY_EPG_SECTION_NUM + " == 0 OR " + KEY_EPG_SECTION_NUM + " == 1) "
                        + "OR ((" + KEY_EPG_START_MJD + " == " + iStartMJD + " AND " + KEY_EPG_START_HOUR + " >= " + iStartHH + ") "
                        + "OR ("+ KEY_EPG_START_MJD + " == " + iEndMJD + " AND " + KEY_EPG_START_HOUR + " <= " + iEndHH + ")) "
                    + "GROUP BY " +  KEY_EPG_EVENT_ID + " "
                    + "ORDER BY " + KEY_EPG_TABLE_ID + ", " + KEY_EPG_SECTION_NUM + ", " +  KEY_EPG_START_MJD + ", " + KEY_EPG_START_HOUR + ", " + KEY_EPG_START_MINUTE + " ";
            }
            else
            {
                sql = "SELECT "
                    + KEY_ROWID + ", "
                    + KEY_EPG_START_MJD + ", "
                    + KEY_EPG_START_HOUR + ", "
                    + KEY_EPG_START_MINUTE + ", "
                    + KEY_EPG_START_SEC + ", "
                    + KEY_EPG_DURATION_HOUR + ", "
                    + KEY_EPG_DURATION_MINUTE + ", "
                    + KEY_EPG_DURATION_SEC + ", "
                    + KEY_EPG_EVT_NAME + ", "
                    + KEY_EPG_EVT_TEXT + ", "
                    + KEY_EPG_EVT_EXTERN + ", "
                    + KEY_EPG_GENRE + ", "
                    + KEY_EPG_AUDIO_MODE + ", "
                    + KEY_EPG_AUDIO_SR + ", "
                    + KEY_EPG_AUDIO_LANG1 + ", "
                    + KEY_EPG_AUDIO_LANG2 + ", "
                    + KEY_EPG_VIDEO_MODE + ", "
                    + KEY_EPG_TABLE_ID + ", "
                    + KEY_EPG_SECTION_NUM + ", "
                    + KEY_EPG_EVENT_ID + ", "
                    + KEY_EPG_REF_SERVICE_ID + ", "
                    + KEY_EPG_REF_EVENT_ID + " "
                    + "FROM " + EPG_PF_DATABASE_TABLE + "_" + channelNumber + "_0x" + Integer.toHexString(serviceID) + " "
                    + "WHERE (" + KEY_EPG_SECTION_NUM + " == 0 OR " + KEY_EPG_SECTION_NUM + " == 1) "
                    	+ "OR (" + KEY_EPG_START_MJD + " == " + iStartMJD + " AND " + KEY_EPG_START_HOUR + " >= " + iStartHH + " AND " + KEY_EPG_START_HOUR + " <= " + iEndHH + ") " 
                    + "GROUP BY " +  KEY_EPG_EVENT_ID + " "
                    + "ORDER BY " + KEY_EPG_TABLE_ID + ", " + KEY_EPG_SECTION_NUM + ", " +  KEY_EPG_START_MJD + ", " + KEY_EPG_START_HOUR + ", " + KEY_EPG_START_MINUTE + " ";
            }
			
			if (checkSanityEPGDB() == false)
				return null;

            cursor = mSQLiteDatabaseEPG[mPresetMode].rawQuery(sql, null);
            if (cursor != null) {
                cursor.moveToFirst();
            }

            return cursor;
        }
        catch (Exception e) {
            Log.e(TAG, "EPG_Sche Table is not exist");
        }
		
        return null;
    }
	
    public Cursor getEPG_Sche_Information(int serviceID, int channelNumber, int countryCode) throws SQLException
    {
        try {
            int pfMJD=0, pfHH=0, pfMM=0, pfSS=0;

            String sql = 
                "SELECT "
                    + KEY_EPG_START_MJD + ", "
                    + KEY_EPG_START_HOUR + ", "
                    + KEY_EPG_START_MINUTE + ", "
                    + KEY_EPG_START_SEC + " "
                + "FROM " + EPG_PF_DATABASE_TABLE + "_" + channelNumber + "_0x" + Integer.toHexString(serviceID) + " "
                + "ORDER BY " + KEY_EPG_START_MJD + " DESC, " + KEY_EPG_START_HOUR + " DESC, " + KEY_EPG_START_MINUTE + " DESC";

			if (checkSanityEPGDB() == false)
				return null;

            Cursor cursor = mSQLiteDatabaseEPG[mPresetMode].rawQuery(sql, null);
            if((cursor != null) && (cursor.getCount() > 0)) {
                cursor.moveToFirst();

                pfMJD = cursor.getInt(0);
                pfHH = cursor.getInt(1);
                pfMM = cursor.getInt(2);
                pfSS = cursor.getInt(3);

                cursor.close();
            }

            sql = "SELECT "
                    + KEY_ROWID + ", "
                    + KEY_EPG_START_MJD + ", "
                    + KEY_EPG_START_HOUR + ", "
                    + KEY_EPG_START_MINUTE + ", "
                    + KEY_EPG_START_SEC + ", "
                    + KEY_EPG_DURATION_HOUR + ", "
                    + KEY_EPG_DURATION_MINUTE + ", "
                    + KEY_EPG_DURATION_SEC + ", "
                    + KEY_EPG_EVT_NAME + ", "
                    + KEY_EPG_EVT_TEXT + ", "
                    + KEY_EPG_EVT_EXTERN + ", "
                    + KEY_EPG_GENRE + ", "
                    + KEY_EPG_AUDIO_MODE + ", "
                    + KEY_EPG_AUDIO_SR + ", "
                    + KEY_EPG_AUDIO_LANG1 + ", "
                    + KEY_EPG_AUDIO_LANG2 + ", "
                    + KEY_EPG_VIDEO_MODE + ", "
                    + KEY_EPG_TABLE_ID + ", "
                    + KEY_EPG_SECTION_NUM + " "
                    + "FROM " + EPG_PF_DATABASE_TABLE + "_" + channelNumber + "_0x" + Integer.toHexString(serviceID) + " "
                + "UNION "
                + "SELECT "
                    + KEY_ROWID + ", "
                    + KEY_EPG_START_MJD + ", "
                    + KEY_EPG_START_HOUR + ", "
                    + KEY_EPG_START_MINUTE + ", "
                    + KEY_EPG_START_SEC + ", "
                    + KEY_EPG_DURATION_HOUR + ", "
                    + KEY_EPG_DURATION_MINUTE + ", "
                    + KEY_EPG_DURATION_SEC + ", "
                    + KEY_EPG_EVT_NAME + ", "
                    + KEY_EPG_EVT_TEXT + ", "
                    + KEY_EPG_EVT_EXTERN + ", "
                    + KEY_EPG_GENRE + ", "
                    + KEY_EPG_AUDIO_MODE + ", "
                    + KEY_EPG_AUDIO_SR + ", "
                    + KEY_EPG_AUDIO_LANG1 + ", "
                    + KEY_EPG_AUDIO_LANG2 + ", "
                    + KEY_EPG_VIDEO_MODE + ", "
                    + KEY_EPG_TABLE_ID + ", "
                    + KEY_EPG_SECTION_NUM + " "
                + "FROM " + EPG_SCHEDULE_DATABASE_TABLE + "_" + channelNumber + "_0x" + Integer.toHexString(serviceID) + " "
                + "WHERE (" + KEY_EPG_START_MJD + " == " + pfMJD + " AND " + KEY_EPG_START_HOUR + " == " + pfHH + " AND " + KEY_EPG_START_MINUTE + " > " + pfMM + ") "
                    + "OR (" + KEY_EPG_START_MJD + " == "  + pfMJD + " AND " + KEY_EPG_START_HOUR + "  > " + pfHH + ") "
                    + "OR (" + KEY_EPG_START_MJD + " > " + pfMJD + ") "
                + "ORDER BY " + KEY_EPG_TABLE_ID + ", " + KEY_EPG_SECTION_NUM + ", " +  KEY_EPG_START_MJD + ", " + KEY_EPG_START_HOUR + ", " + KEY_EPG_START_MINUTE + " ";

            cursor = mSQLiteDatabaseEPG[mPresetMode].rawQuery(sql, null);
            if (cursor != null) {
                cursor.moveToFirst();
            }

            return cursor;
        }
        catch (Exception e) {
            Log.e(TAG, "EPG_Sche Table is not exist");
        }
		
        return null;
    }

    public Cursor getEPG_Sche_Information(int serviceID, int channelNumber, int countryCode, int date) throws SQLException
    {
        try {
            int pfMJD=0, pfHH=0, pfMM=0, pfSS=0;
            int requestMJD=0;

            String sql = 
                    "SELECT "
                        + KEY_EPG_START_MJD + ", "
                        + KEY_EPG_START_HOUR + ", "
                        + KEY_EPG_START_MINUTE + ", "
                        + KEY_EPG_START_SEC + " "
                    + "FROM " + EPG_PF_DATABASE_TABLE + "_" + channelNumber + "_0x" + Integer.toHexString(serviceID) + " "
                    + "ORDER BY " + KEY_EPG_START_HOUR + " DESC, " + KEY_EPG_START_MINUTE + " DESC";

			if (checkSanityEPGDB() == false)
				return null;

            Cursor cursor = mSQLiteDatabaseEPG[mPresetMode].rawQuery(sql, null);
            if((cursor != null) && (cursor.getCount() > 0)) {
                cursor.moveToFirst();

                pfMJD = cursor.getInt(0);
                pfHH = cursor.getInt(1);
                pfMM = cursor.getInt(2);
                pfSS = cursor.getInt(3);

                requestMJD = pfMJD + date;

                cursor.close();
            }

            if(date == 0) {
                sql = "SELECT "
                        + KEY_ROWID + ", "
                        + KEY_EPG_START_MJD + ", "
                        + KEY_EPG_START_HOUR + ", "
                        + KEY_EPG_START_MINUTE + ", "
                        + KEY_EPG_START_SEC + ", "
                        + KEY_EPG_DURATION_HOUR + ", "
                        + KEY_EPG_DURATION_MINUTE + ", "
                        + KEY_EPG_DURATION_SEC + ", "
                        + KEY_EPG_EVT_NAME + ", "
                        + KEY_EPG_EVT_TEXT + ", "
                        + KEY_EPG_EVT_EXTERN + ", "
                        + KEY_EPG_GENRE + ", "
                        + KEY_EPG_AUDIO_MODE + ", "
                        + KEY_EPG_AUDIO_SR + ", "
                        + KEY_EPG_AUDIO_LANG1 + ", "
                        + KEY_EPG_AUDIO_LANG2 + ", "
                        + KEY_EPG_VIDEO_MODE + ", "
                        + KEY_EPG_TABLE_ID + ", "
                        + KEY_EPG_SECTION_NUM + " "
                        + "FROM " + EPG_PF_DATABASE_TABLE + "_" + channelNumber + "_0x" + Integer.toHexString(serviceID) + " "
                    + "UNION "
                    + "SELECT "
                        + KEY_ROWID + ", "
                        + KEY_EPG_START_MJD + ", "
                        + KEY_EPG_START_HOUR + ", "
                        + KEY_EPG_START_MINUTE + ", "
                        + KEY_EPG_START_SEC + ", "
                        + KEY_EPG_DURATION_HOUR + ", "
                        + KEY_EPG_DURATION_MINUTE + ", "
                        + KEY_EPG_DURATION_SEC + ", "
                        + KEY_EPG_EVT_NAME + ", "
                        + KEY_EPG_EVT_TEXT + ", "
                        + KEY_EPG_EVT_EXTERN + ", "
                        + KEY_EPG_GENRE + ", "
                        + KEY_EPG_AUDIO_MODE + ", "
                        + KEY_EPG_AUDIO_SR + ", "
                        + KEY_EPG_AUDIO_LANG1 + ", "
                        + KEY_EPG_AUDIO_LANG2 + ", "
                        + KEY_EPG_VIDEO_MODE + ", "
                        + KEY_EPG_TABLE_ID + ", "
                        + KEY_EPG_SECTION_NUM + " "
                    + "FROM " + EPG_SCHEDULE_DATABASE_TABLE + "_" + channelNumber + "_0x" + Integer.toHexString(serviceID) + " "
                    + "WHERE (" + KEY_EPG_START_MJD + " == " + pfMJD + " AND " + KEY_EPG_START_HOUR + " == " + pfHH + " AND " + KEY_EPG_START_MINUTE + " > " + pfMM + ") "
                        + "OR (" + KEY_EPG_START_MJD + " == "  + pfMJD + " AND " + KEY_EPG_START_HOUR + "  > " + pfHH + ") "
                    + "ORDER BY " + KEY_EPG_TABLE_ID + ", " + KEY_EPG_SECTION_NUM + ", " + KEY_EPG_START_MJD + ", " + KEY_EPG_START_HOUR + ", " + KEY_EPG_START_MINUTE + " ";
            }
            else {
                sql = "SELECT "
                        + KEY_ROWID + ", "
                        + KEY_EPG_START_MJD + ", "
                        + KEY_EPG_START_HOUR + ", "
	                    + KEY_EPG_START_MINUTE + ", "
                        + KEY_EPG_START_SEC + ", "
                        + KEY_EPG_DURATION_HOUR + ", "
                        + KEY_EPG_DURATION_MINUTE + ", "
                        + KEY_EPG_DURATION_SEC + ", "
                        + KEY_EPG_EVT_NAME + ", "
                        + KEY_EPG_EVT_TEXT + ", "
                        + KEY_EPG_EVT_EXTERN + ", "
                        + KEY_EPG_GENRE + ", "
                        + KEY_EPG_AUDIO_MODE + ", "
                        + KEY_EPG_AUDIO_SR + ", "
                        + KEY_EPG_AUDIO_LANG1 + ", "
                        + KEY_EPG_AUDIO_LANG2 + ", "
                        + KEY_EPG_VIDEO_MODE + ", "
                        + KEY_EPG_TABLE_ID + ", "
                        + KEY_EPG_SECTION_NUM + " "
                    + "FROM " + EPG_SCHEDULE_DATABASE_TABLE + "_" + channelNumber + "_0x" + Integer.toHexString(serviceID) + " "
                    + "WHERE " + KEY_EPG_START_MJD + " == " + requestMJD + " "
                    + "ORDER BY " + KEY_EPG_TABLE_ID + ", " + KEY_EPG_SECTION_NUM + ", " + KEY_EPG_START_MJD + ", " + KEY_EPG_START_HOUR + ", " + KEY_EPG_START_MINUTE + " ";
            }
			
            cursor = mSQLiteDatabaseEPG[mPresetMode].rawQuery(sql, null);
            if (cursor != null) {
                cursor.moveToFirst();
            }

            return cursor;
        }
        catch (Exception e) {
            Log.e(TAG, "EPG_Sche Table is not exist");
        }
		
        return null;
    }
	
    public Cursor getEPG_Sche_Information(int serviceID, int channelNumber, int countryCode, int mjd, int time, int prev_event_time) throws SQLException
    {
        try {
            int iDummy;
            int iStartMJD, iEndMJD;
            int iStartHH, iEndHH;
            String sql;
            Cursor cursor;
			
            if(prev_event_time < 0) {
                iDummy = 6;
            } else if (prev_event_time > time) {
                iDummy = (time + 24) - prev_event_time;
            } else {
                iDummy = time - prev_event_time;
            }
			
            iEndMJD = mjd;
            iEndHH = time;
			
            if(time < iDummy)
            {
                iStartMJD = mjd - 1;
                iStartHH = (time + 24) - iDummy;
                iEndMJD = mjd;
                iEndHH = time + 3;
            }
            else if ((time + 3) >= 24)
            {
                iStartMJD = mjd;
                iStartHH = time - iDummy;
                iEndMJD = mjd + 1;
                iEndHH = (time + 3) - 24;
            }
            else
            {
                iStartMJD = mjd;
                iStartHH = time - iDummy;
                iEndMJD = mjd;
                iEndHH = time + 3;
            }
			
            if (iStartMJD != iEndMJD)
            {
                sql = "SELECT "
                        + KEY_ROWID + ", "
                        + KEY_EPG_START_MJD + ", "
                        + KEY_EPG_START_HOUR + ", "
                        + KEY_EPG_START_MINUTE + ", "
                        + KEY_EPG_START_SEC + ", "
                        + KEY_EPG_DURATION_HOUR + ", "
                        + KEY_EPG_DURATION_MINUTE + ", "
                        + KEY_EPG_DURATION_SEC + ", "
                        + KEY_EPG_EVT_NAME + ", "
                        + KEY_EPG_EVT_TEXT + ", "
                        + KEY_EPG_EVT_EXTERN + ", "
                        + KEY_EPG_GENRE + ", "
                        + KEY_EPG_AUDIO_MODE + ", "
                        + KEY_EPG_AUDIO_SR + ", "
                        + KEY_EPG_AUDIO_LANG1 + ", "
                        + KEY_EPG_AUDIO_LANG2 + ", "
                        + KEY_EPG_VIDEO_MODE + ", "
                        + KEY_EPG_TABLE_ID + ", "
                        + KEY_EPG_SECTION_NUM + ", "
                        + KEY_EPG_EVENT_ID + ", "
                        + KEY_EPG_REF_SERVICE_ID + ", "
                        + KEY_EPG_REF_EVENT_ID + " "
                    + "FROM " + EPG_SCHEDULE_DATABASE_TABLE + "_" + channelNumber + "_0x" + Integer.toHexString(serviceID) + " "
                    + "WHERE (" + KEY_EPG_START_MJD + " == " + iStartMJD + " AND " + KEY_EPG_START_HOUR + " >= " + iStartHH + ") "
                        + "OR ("+ KEY_EPG_START_MJD + " == " + iEndMJD + " AND " + KEY_EPG_START_HOUR + " <= " + iEndHH + ") "
                    + "GROUP BY " + KEY_EPG_EVENT_ID + " "
                    + "ORDER BY " + KEY_EPG_TABLE_ID + ", " + KEY_EPG_SECTION_NUM + ", " + KEY_EPG_START_MJD + ", " + KEY_EPG_START_HOUR + ", " + KEY_EPG_START_MINUTE + " ";
            }
            else
            {
                sql = "SELECT "
                        + KEY_ROWID + ", "
                        + KEY_EPG_START_MJD + ", "
                        + KEY_EPG_START_HOUR + ", "
                        + KEY_EPG_START_MINUTE + ", "
                        + KEY_EPG_START_SEC + ", "
                        + KEY_EPG_DURATION_HOUR + ", "
                        + KEY_EPG_DURATION_MINUTE + ", "
                        + KEY_EPG_DURATION_SEC + ", "
                        + KEY_EPG_EVT_NAME + ", "                        
                        + KEY_EPG_EVT_TEXT + ", "
                        + KEY_EPG_EVT_EXTERN + ", "
                        + KEY_EPG_GENRE + ", "
                        + KEY_EPG_AUDIO_MODE + ", "
                        + KEY_EPG_AUDIO_SR + ", "
                        + KEY_EPG_AUDIO_LANG1 + ", "
                        + KEY_EPG_AUDIO_LANG2 + ", "
                        + KEY_EPG_VIDEO_MODE + ", "
                        + KEY_EPG_TABLE_ID + ", "
                        + KEY_EPG_SECTION_NUM + ", "
                        + KEY_EPG_EVENT_ID + ", "
                        + KEY_EPG_REF_SERVICE_ID + ", "
                        + KEY_EPG_REF_EVENT_ID + " "
                    + "FROM " + EPG_SCHEDULE_DATABASE_TABLE + "_" + channelNumber + "_0x" + Integer.toHexString(serviceID) + " "
                    + "WHERE (" + KEY_EPG_START_MJD + " == " + iStartMJD + " AND " + KEY_EPG_START_HOUR + " >= " + iStartHH + " AND " + KEY_EPG_START_HOUR + " <= " + iEndHH + ") " 
                    + "GROUP BY " + KEY_EPG_EVENT_ID + " "
                    + "ORDER BY " + KEY_EPG_TABLE_ID + ", " + KEY_EPG_SECTION_NUM + ", " + KEY_EPG_START_MJD + ", " + KEY_EPG_START_HOUR + ", " + KEY_EPG_START_MINUTE + " ";
            }
			
			if (checkSanityEPGDB() == false)
				return null;

            cursor = mSQLiteDatabaseEPG[mPresetMode].rawQuery(sql, null);
            if (cursor != null) {
                cursor.moveToFirst();
            }

            return cursor;
        }
        catch (Exception e) {
            Log.e(TAG, "EPG_Sche Table is not exist");
        }
		
        return null;
    }	

    public Cursor getEPG_PF_LastEvent(int serviceID, int channelNumber, int countryCode) throws SQLException
    {
        try {
            String sql;
            Cursor cursor;
			
            sql = "SELECT "
                    + KEY_ROWID + ", "
                    + KEY_EPG_START_MJD + ", "
                    + KEY_EPG_START_HOUR + ", "
                    + KEY_EPG_START_MINUTE + ", "
                    + KEY_EPG_START_SEC + ", "
                    + KEY_EPG_DURATION_HOUR + ", "
                    + KEY_EPG_DURATION_MINUTE + ", "
                    + KEY_EPG_DURATION_SEC + ", "
                    + KEY_EPG_EVT_NAME + ", "
                    + KEY_EPG_EVT_TEXT + ", "
                    + KEY_EPG_EVT_EXTERN + ", "
                    + KEY_EPG_GENRE + ", "
                    + KEY_EPG_AUDIO_MODE + ", "
                    + KEY_EPG_AUDIO_SR + ", "
                    + KEY_EPG_AUDIO_LANG1 + ", "
                    + KEY_EPG_AUDIO_LANG2 + ", "
                    + KEY_EPG_VIDEO_MODE + ", "
                    + KEY_EPG_TABLE_ID + ", "
                    + KEY_EPG_SECTION_NUM + ", "
                    + KEY_EPG_EVENT_ID + ", "
                    + KEY_EPG_REF_SERVICE_ID + ", "
                    + KEY_EPG_REF_EVENT_ID + " "
                + "FROM " + EPG_PF_DATABASE_TABLE + "_" + channelNumber + "_0x" + Integer.toHexString(serviceID) + " "
                + "ORDER BY " + KEY_EPG_START_MJD + " DESC, " + KEY_EPG_START_HOUR + " DESC, " + KEY_EPG_START_MINUTE + " DESC ";

			if (checkSanityEPGDB() == false)
				return null;

            cursor = mSQLiteDatabaseEPG[mPresetMode].rawQuery(sql, null);
            if (cursor != null) {
                cursor.moveToFirst();
            }

            return cursor;
        }
        catch (Exception e) {
            Log.e(TAG, "EPG_Sche Table is not exist");
        }
		
        return null;
    }	
	
    public Cursor getEPG_Sche_LastEvent(int serviceID, int channelNumber, int countryCode) throws SQLException
    {
        try {
            String sql;
            Cursor cursor;
			
            sql = "SELECT "
                    + KEY_ROWID + ", "
                    + KEY_EPG_START_MJD + ", "
                    + KEY_EPG_START_HOUR + ", "
                    + KEY_EPG_START_MINUTE + ", "
                    + KEY_EPG_START_SEC + ", "
                    + KEY_EPG_DURATION_HOUR + ", "
                    + KEY_EPG_DURATION_MINUTE + ", "
                    + KEY_EPG_DURATION_SEC + ", "
                    + KEY_EPG_EVT_NAME + ", "
                    + KEY_EPG_EVT_TEXT + ", "
                    + KEY_EPG_EVT_EXTERN + ", "
                    + KEY_EPG_GENRE + ", "
                    + KEY_EPG_AUDIO_MODE + ", "
                    + KEY_EPG_AUDIO_SR + ", "
                    + KEY_EPG_AUDIO_LANG1 + ", "
                    + KEY_EPG_AUDIO_LANG2 + ", "
                    + KEY_EPG_VIDEO_MODE + ", "
                    + KEY_EPG_TABLE_ID + ", "
                    + KEY_EPG_SECTION_NUM + ", "
                    + KEY_EPG_EVENT_ID + ", "
                    + KEY_EPG_REF_SERVICE_ID + ", "
                    + KEY_EPG_REF_EVENT_ID + " "
                + "FROM " + EPG_SCHEDULE_DATABASE_TABLE + "_" + channelNumber + "_0x" + Integer.toHexString(serviceID) + " "
                + "ORDER BY " + KEY_EPG_START_MJD + " DESC, " + KEY_EPG_START_HOUR + " DESC, " + KEY_EPG_START_MINUTE + " DESC ";

			if (checkSanityEPGDB() == false)
				return null;

            cursor = mSQLiteDatabaseEPG[mPresetMode].rawQuery(sql, null);
            if (cursor != null) {
                cursor.moveToFirst();
            }

            return cursor;
        }
        catch (Exception e) {
            Log.e(TAG, "EPG_Sche Table is not exist");
        }
		
        return null;
    }	
    
	public Cursor getSubtitle_LanguageCode(int ServiceID, int ChannelNumber)
	{
		Log.i(TAG, "getSubtitle_LanguageCode()");
		
		Cursor cursor;
		if (checkSanityChannelDB() == true) {
			cursor	= mPresetChannelDB[mPresetMode].query(true, SERVICE_DATABASE_TABLE,
				new String[] {
								KEY_ROWID,
								KEY_SERVICE_LANGUAGE_CODE
								},
				KEY_SERVICE_SERVICE_ID + "=" + ServiceID + " AND " + KEY_SERVICE_CHANNEL_NUMBER + "=" + ChannelNumber,
				null, null, null, null, null);
		
			if(cursor != null)
				cursor.moveToFirst();
		} else 
			cursor = null;
		
		return cursor;
		//return null;
	}

	public Cursor getAudio_LanguageCode(int ServiceID, int ChannelNumber)
	{
		Log.i(TAG, "getAudio_LanguageCode()");
		
		Cursor cursor;
		if (checkSanityChannelDB() == true) {
			cursor	= mPresetChannelDB[mPresetMode].query(true, AUDIOPID_DATABASE_TABLE,
				new String[] {
								KEY_ROWID,
								KEY_AUDIO_LANGUAGE_CODE,
			                    KEY_AUDIO_STREAM_TYPE
								},
				KEY_AUDIO_SERVICE_ID + "=" + ServiceID + " AND " + KEY_AUDIO_CHANNEL_NUMBER + "=" + ChannelNumber,
				null, null, null, null, null);
		
			if(cursor != null)
				cursor.moveToFirst();
		} else
			cursor = null;
		return cursor;
	}
		
	public void setPresetMode (int PresetMode)
	{
		Log.i(TAG, "setPresetMode:"+PresetMode);
		if (PresetMode < 0 || PresetMode >= PRESET_MODE_MAX_COUNT)
			Log.d(TAG, "setPresetMode fail:"+PresetMode);
		else
			mPresetMode = PresetMode;
	}
}
