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

import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;

import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Parcel;
import android.os.Parcelable;
import android.util.AttributeSet;

import com.telechips.android.dvb.R;
import com.telechips.android.dvb.schedule.Alert;
import com.telechips.android.dvb.schedule.DxbEventData;
import com.telechips.android.dvb.util.*;
import com.telechips.android.dvb.player.diseqc.*;
import com.telechips.android.dvb.player.dvb.RFInfoData;
import com.telechips.android.dvb.player.dvb.DateTimeData;
import com.telechips.android.dvb.player.dvb.DebugModeData;
import com.telechips.android.dvb.player.dvb.SignalState;
import com.telechips.android.dvb.player.dvb.TuneSpace;
import com.telechips.android.dvb.player.dvb.VideoDefinitionData;
import com.telechips.android.dvb.player.dvb.DB_CHANNEL_Data;
import com.telechips.android.dvb.player.ChannelManager.EpgData;

public class DxbPlayer extends DxbPlayerBase
{
	public static final int	_OFF_	= 0;
	public static final int	_ON_	= 1;

	// Dxb_Player State
	public static enum STATE
	{
		GENERAL,
		OPTION_FULL_SCAN,
		OPTION_MANUAL_SCAN,
		OPTION_BLIND_SCAN,
		OPTION_MAKE_PRESET,
		SCAN,
		SCAN_STOP,
		UI_PAUSE,
		NULL
	}
	public STATE eState = STATE.GENERAL;

	// Option
	public static int		DEFAULT_VALUE_LANGUAGE_SUBTITLE	= 0;
	public static int		DEFAULT_VALUE_LANGUAGE_AUDIO	= 0;
	public boolean			DEFAULT_VALUE_AD				= false;
	public static int		DEFAULT_AUDIO_MODE				= 0;
	public int				DEFAULT_VALUE_RELATIVE_VOL		= 0;		// 0~100
	public static int		DEFAULT_VALUE_COUNTRYCODE		= 17;		//11(886); //Tiwan
	public static int		DEFAULT_VALUE_TIME_ZONE_TYPE	= 0;		// default auto(0) / manual(1)
	public static int		DEFAULT_VALUE_TIME_ZONE_INDEX	= 12;		// default "0"
	public static String	DEFAULT_VALUE_PASSWORD			= "0000";
	public static int		DEFAULT_VALUE_ANTENNA_POWER		= 2;		// 0-off, 1-on, 2-auto
	public static int		DEFAULT_VALUE_AGE				= 0;		// 0-all, (1~F)+7

	public class OPTION
	{
		public int		scan_manual;
		public int		scan_manual_bandwidth;
		public int		countrycode				= DEFAULT_VALUE_COUNTRYCODE;
		public int		language_audio			= DEFAULT_VALUE_LANGUAGE_AUDIO;
		public int		language_subtitle		= DEFAULT_VALUE_LANGUAGE_SUBTITLE;
		public int		time_zone_type			= DEFAULT_VALUE_TIME_ZONE_TYPE;
		public int		time_zone_index			= DEFAULT_VALUE_TIME_ZONE_INDEX;
		public String	password				= DEFAULT_VALUE_PASSWORD;
		public int		antenna_power			= DEFAULT_VALUE_ANTENNA_POWER;
		public int		age						= DEFAULT_VALUE_AGE;
		public int		audio_mode				= DEFAULT_AUDIO_MODE;
		public boolean	audio_description_on	= DEFAULT_VALUE_AD;
		public int		relative_vol			= DEFAULT_VALUE_RELATIVE_VOL;
	}
	public OPTION cOption = new OPTION();

	private class SCHEDULER
	{
		public boolean	isScreenOn;
		public boolean	isPlayerOn;
		
		public int Alarm_Type;
		
		public int Channel_Type;
		public int Channel_Index;
		
		public int Repeat_Type;
		public int Alarm_ID;
	}
	private SCHEDULER Scheduler_Info = new SCHEDULER();
	
	// check playr OFF state
	public boolean isOFF	= false;
	public boolean isFINISH	= false;

	public boolean isDVBPlayerActivityON = false;
	public boolean isDxbScheduleActivityON = false;
	public boolean isDxbOptionActivityON = false;
	public boolean isDishSetupActivityOn = false;

	// Databases
	private DxbDB_Setting    mSettingDB = null;
	private ChannelManager   mChannelManager = null;
	private FileManager      mFileManager = null;

	public static final int DTV_TV    = 0;
	public static final int DTV_RADIO = 1;
	public static final int DTV_PVR   = 2;

	private int mTab;
	private int[] mCurrPosition = new int[3];
	private int[] mPrevPosition = new int[3];

	private int mFileCount = 0;
	private Cursor mCursorFileList = null;
	private int mRecordTab  = 0;
	private int mRecordPosition = 0;

	private int[]            mCountryCode;
	private TuneSpace[]      mTuneSpace;
	private String			sSatellite_Name;
	
	/*********************************************************************************************/
	/*	SI(system information) Data - service, epg	*/
	private ArrayList<DxbChannelData>	tvChannelData		= new ArrayList<DxbChannelData>();
	private ArrayList<DxbChannelData>	radioChannelData	= new ArrayList<DxbChannelData>();
	
	private ArrayList<String>	tvNameData		= new ArrayList<String>();
	private ArrayList<String>	radioNameData	= new ArrayList<String>();
	
	private DxbEventData		eventData		= new DxbEventData();

	private DxbChannelData   mChannelData;
	private EpgData[]        mPF_EPG;
	/*********************************************************************************************/

	public DxbPlayer(Context context, AttributeSet attrs)
	{
		super(context, attrs);
		initPlayer(context);
	}

	private void initPlayer(Context context)
	{
		mChannelManager = new ChannelManager(context);
		mFileManager = new FileManager(context);
		mSettingDB = new DxbDB_Setting(context);

		mCountryCode = getResources().getIntArray(R.array.countrycode);

		if (isDVB_S2())
		{
			initDVBSx(context);
		}
		else
		{
			mTuneSpace = DxbSpace.get(getResources().getXml(R.xml.freq_list));
		}
	}

	@Override
	public void openPlayer()
	{
		if(!isValid())
		{
			super.openPlayer();
		}
		else
		{
			onPrepared();
		}
	}

	public TuneSpace getTuneSpace()
	{
		return getTuneSpace(mCountryCode[cOption.countrycode]);
	}

	@Override
	public TuneSpace getTuneSpace(int channel_group)
	{
		TuneSpace tuneSpace = null;
		if (isDVB_S2())
		{
			tuneSpace = getTuneSpaceForDVBSx(channel_group);
		}
		else if (mTuneSpace != null)
		{
			if (mTuneSpace.length > cOption.countrycode)
			{
				tuneSpace = mTuneSpace[cOption.countrycode];
			}
		}
		if (tuneSpace == null)
		{
			tuneSpace = new TuneSpace(0);
		}
		return tuneSpace;
	}

	public int getCurrentTime()
	{
		DateTimeData date_time = getTime();
		int currTime = 0;
		if (date_time != null)
		{
			if(date_time.iMJD != 0)
			{
				currTime	= date_time.iHour*60*60 + date_time.iMinute*60 + date_time.iSecond;
				//DxbLog(D, date_time.iHour + ":" + date_time.iMinute + ":" + date_time.iSecond);
			}
			if(date_time.iMJD == 0)
			{
				Calendar calTime = Calendar.getInstance();
				currTime	= calTime.get(Calendar.HOUR_OF_DAY)*60*60 + calTime.get(Calendar.MINUTE)*60 + calTime.get(Calendar.SECOND);
			}
		}
		return currTime;
	}

	//	public Parcel getRFInfo(){
	//		DxbLog(I, "getRFInfo");
	//		return getRFInformation();
	//	}
		
		private int DVBSystem;
		private int Frequency;
		private int Bandwidth;
		private int Rotation;
		private int Constellation;
		private int FecLength;
		private int GuardInterval;
		private int CodeRate;
		private int Fft;
		private int Plp;
		private int PilotPp;

		public void setProgramInfo(){
			if(getRFInformation() != null)
			{
				RFInfoData RFInfo = getRFInformation();
				DxbLog(I, "after RFInfo");
		//		RFInfo.;

				DVBSystem = RFInfo.iDVBSystem;
				Frequency = RFInfo.iFecLength;
				Bandwidth = RFInfo.iBandwidth;
				Rotation = RFInfo.iRotation;
				Constellation = RFInfo.iConstellation;
				FecLength = RFInfo.iFecLength;
				GuardInterval = RFInfo.iGuardInterval;
				CodeRate = RFInfo.iCodeRate;
				Fft = RFInfo.iFft;
				Plp = RFInfo.iPlp;
				PilotPp = RFInfo.iPilotPp;
				
	/*			DVBSystem = RFInfo.readInt();
				Frequency = RFInfo.readInt();
				Bandwidth = RFInfo.readInt();
				Rotation = RFInfo.readInt();
				Constellation = RFInfo.readInt();
				FecLength = RFInfo.readInt();
				GuardInterval = RFInfo.readInt();
				CodeRate = RFInfo.readInt();
				Fft = RFInfo.readInt();
				Plp = RFInfo.readInt();
				PilotPp = RFInfo.readInt();
	*/			//int  = mPlayer.getRFInformation().readInt();
			}
			else
			{
				DVBSystem = -1;
				Frequency = -1;
				Bandwidth = -1;
				Rotation = -1;
				Constellation = -1;
				FecLength = -1;
				GuardInterval = -1;
				CodeRate = -1;
				Fft = -1;
				Plp = -1;
				PilotPp = -1;
			}
		}
	/************************************************************************************************************************
	 * 	DB control -----> Start
	 ************************************************************************************************************************/
		/*	Service	*/
		private int	iIndex_channelNumber, iIndex_countryCode, iIndex_frequency, iIndex_serviceType;
		private int	iIndex_audioPID, iIndex_videoPID, iIndex_teletextPID, iIndex_serviceID;
		private int	iIndex_serviceName;
		private int	iIndex_freeCAmode, iIndex_scrambling, iIndex_bookmark, iIndex_logicalChannel=-1;

		private void db_open()
		{
			DxbLog(I, "db_open()");

			if(mChannelManager == null)
			{
				return;
			}
			mChannelManager.open();
			
			if(mFileManager == null)
			{
				return;
			}
			mFileManager.open();
			
			if(mSettingDB == null)
			{
				return;
			}
			mSettingDB.open();
			
			if (isDVB_S2())
			{
				startDVBSx();
			}

			setChannelPosition(DTV_TV, mSettingDB.getPosition(0));
			setChannelPosition(DTV_RADIO, mSettingDB.getPosition(1));
			cOption.time_zone_index	= mSettingDB.getTimeZone();
			cOption.password		= mSettingDB.getPassword();
			cOption.age				= mSettingDB.getAge();
			
			int	iVol	= mSettingDB.getRelativeVol();
			if(((iVol+AD_VOL_MAX)/1000) > 0)
			{
				cOption.audio_description_on	= true;
			}
			else
			{
				cOption.audio_description_on	= false;
			}
			
			cOption.relative_vol	= (iVol+AD_VOL_MAX)%1000 - AD_VOL_MAX;

			setChannelList();
		}

		private void db_close()
		{
			DxbLog(I, "db_close()");

			mChannelManager.close();
			mFileManager.close();
			mSettingDB.close();
			if (isDVB_S2())
			{
				stopDVBSx();
			}
		}

		public void setColumnIndex_Service(Cursor cursor)
		{
			if ((cursor != null) && (iIndex_logicalChannel == -1))
			{
				iIndex_channelNumber	= cursor.getColumnIndexOrThrow(ChannelManager.KEY_CHANNEL_NUMBER);
				iIndex_countryCode		= cursor.getColumnIndexOrThrow(ChannelManager.KEY_COUNTRY_CODE);
				iIndex_frequency		= cursor.getColumnIndexOrThrow(ChannelManager.KEY_FREQUENCY); 
				iIndex_serviceType		= cursor.getColumnIndexOrThrow(ChannelManager.KEY_SERVICE_TYPE);
				
				iIndex_audioPID			= cursor.getColumnIndexOrThrow(ChannelManager.KEY_AUDIO_PID);
				iIndex_videoPID			= cursor.getColumnIndexOrThrow(ChannelManager.KEY_VIDEO_PID);
				iIndex_teletextPID		= cursor.getColumnIndexOrThrow(ChannelManager.KEY_TELETEXT_PID);
				iIndex_serviceID		= cursor.getColumnIndexOrThrow(ChannelManager.KEY_SERVICE_ID);
				
				iIndex_serviceName		= cursor.getColumnIndexOrThrow(ChannelManager.KEY_CHANNEL_NAME);
				
				iIndex_freeCAmode		= cursor.getColumnIndexOrThrow(ChannelManager.KEY_FREE_CA_MODE);
				iIndex_scrambling		= cursor.getColumnIndexOrThrow(ChannelManager.KEY_SCRAMBLING);
				iIndex_bookmark			= cursor.getColumnIndexOrThrow(ChannelManager.KEY_BOOKMARK);
				iIndex_logicalChannel	= cursor.getColumnIndexOrThrow(ChannelManager.KEY_LOGICALCHANNEL);
			}
		}

		public int	getColumnIndex_ID() {
			return 0;
		}

		public int getColumnIndex_ChannelNumber() {
			return iIndex_channelNumber;
		}

		public int getColumnIndex_CountryCode() {
			return iIndex_countryCode;
		}

		public int getColumnIndex_Frequency() {
			return iIndex_frequency;
		}

		public int getColumnIndex_ServiceType() {
			return iIndex_serviceType;
		}

		public int getColumnIndex_AudioPID() {
			return iIndex_audioPID;
		}

		public int getColumnIndex_VideoPID() {
			return iIndex_videoPID;
		}

		public int getColumnIndex_TeletextPID() {
			return iIndex_teletextPID;
		}

		public int getColumnIndex_ServiceID() {
			return iIndex_serviceID;
		}

		public int getColumnIndex_ServiceName() {
			return iIndex_serviceName;
		}

		public int getColumnIndex_FreeCAmode() {
			return iIndex_freeCAmode;
		}

		public int getColumnIndex_Scrambling() {
			return iIndex_scrambling;
		}

		public int getColumnIndex_Bookmark() {
			return iIndex_bookmark;
		}

		public int getColumnIndex_LogicalChannel()
		{
			return iIndex_logicalChannel;
		}

		public String getServiceName()
		{
			DxbChannelData channeldata = mChannelData;;

			if (channeldata == null)
				return null;

			return channeldata.sChannel_name;
		}

		public String getServiceName_addHyphen()
		{
			DxbChannelData channeldata = mChannelData;;

			if (channeldata == null)
				return null;

			return (channeldata.sChannel_name + "-");
		}

		public String getServiceName_addLogical()
		{
			DxbChannelData channeldata = mChannelData;;

			if (channeldata == null)
				return null;

			if (getServiceType() == DTV_PVR)
			{
				return channeldata.sChannel_name;
			}
			else if(channeldata.iLogical_channel == 0)
			{
				return channeldata.sChannel_name;
			}
			else
			{
				return ("CH " + String.format("%03d", channeldata.iLogical_channel) + "  -  " + channeldata.sChannel_name);
			}
		}

		public Cursor getFileList()
		{
			DxbLog(I, "getFileList() --> cOption.storage = " + DxbStorage.getPath_Device());
			
			mCursorFileList	= mFileManager.getAllFiles(DxbStorage.getPath_Device());
			mFileCount = mCursorFileList.getCount();
			return mCursorFileList;
		}

		public void setChannelList()
		{
			DxbLog(I, "setChannelList()");

			Cursor	current_Cursor	= null;
			ArrayList<DxbChannelData>	current_List	= null;
			ArrayList<String>			current_Name	= null;
			int current_Count;
			for(int i=0 ; i<2 ; i++)
			{
				DxbLog(D, "i=" + i);

				current_Cursor	= mChannelManager.getAllChannels(i+1);
				if(		(current_Cursor == null)
					||	current_Cursor.isClosed()
				)
				{
					continue;
				}
				current_Count	= current_Cursor.getCount();
				DxbLog(D, "current_Count = " + current_Count);

				if(i == 0)
				{
					current_List	= tvChannelData;
					current_Name	= tvNameData;
					setColumnIndex_Service(current_Cursor);
					
					eventData.iCountTV	= current_Count;
				}
				else if(i == 1)
				{
					current_List	= radioChannelData;
					current_Name	= radioNameData;
					
					eventData.iCountRadio	= current_Count;
				}

				current_List.clear();
				current_Name.clear();
				current_Cursor.moveToFirst();

				for(int j=0 ; j<current_Count ; j++)
				{
					DxbLog(D, "ID = " + current_Cursor.getInt(0));
					DxbLog(D,	"ChannelNumber="	+	current_Cursor.getInt(getColumnIndex_ChannelNumber())	+ ", " +
								"CountryCode="		+ 	current_Cursor.getInt(getColumnIndex_CountryCode())		+ ", " +
								"Frequency="		+	current_Cursor.getInt(getColumnIndex_Frequency())		+ ", " +
								"ServiceType="		+ 	current_Cursor.getInt(getColumnIndex_ServiceType())		+ ", " +

								"AudioPID="			+ 	current_Cursor.getInt(getColumnIndex_AudioPID())		+ ", " +
								"VideoPID="			+	current_Cursor.getInt(getColumnIndex_VideoPID())		+ ", " +
								"TeletextPID="		+	current_Cursor.getInt(getColumnIndex_TeletextPID())		+ ", " +
								"ServiceID="		+ 	current_Cursor.getInt(getColumnIndex_ServiceID())		+ ", " +

								"ServiceName="		+	current_Cursor.getString(getColumnIndex_ServiceName())	+ ", " +

								"FreeCAmode="		+ 	current_Cursor.getInt(getColumnIndex_FreeCAmode())		+ ", " +
								"Scrambling="		+	current_Cursor.getInt(getColumnIndex_Scrambling())		+ ", " +
								"Bookmark="			+	current_Cursor.getInt(getColumnIndex_Bookmark())		+ ", " +
								"LogicalChannel="	+	current_Cursor.getInt(getColumnIndex_LogicalChannel())
							);
					current_List.add(CursorToChannelData(current_Cursor));
					current_Name.add(new String(current_Cursor.getString(getColumnIndex_ServiceName())));
					current_Cursor.moveToNext();
				}

				if (getChannelPosition(i) >= current_Count)
				{
					restoreChannelPosition(i);
				}

				current_Cursor.close();
			}
			if(getChannelCount(DTV_TV) > 0)
			{
				setServiceType(DTV_TV);
			}
			else
			{
				setServiceType(DTV_RADIO);
			}
		}

		public void setEventData(int iAction)
		{
			eventData.iAction		= iAction;
			
			eventData.iChannelType	= getChannelType(getCurChannelType());
			if(eventData.iChannelType == 1)
			{
				eventData.iIndexService	= getChannelPosition(1);
			}
			else //if(getPlayer().eventData.iChannelType == 0)
			{
				eventData.iIndexService	= getChannelPosition(0);
			}
			
			eventData.iMode			= 0;

			{
				DateTimeData	currentDateTime	= getCurrentDateTime();
				Date	dateData	= new Date(0);
				if(currentDateTime != null)
				{
					dateData.setYear(DxbUtil.getMJD_YY(currentDateTime.iMJD) - 1900);
					dateData.setMonth(DxbUtil.getMJD_MM(currentDateTime.iMJD) - 1);
					dateData.setDate(DxbUtil.getMJD_DD(currentDateTime.iMJD));
					dateData.setHours(currentDateTime.iHour);
					dateData.setMinutes(currentDateTime.iMinute);
					dateData.setSeconds(currentDateTime.iSecond);
				}
				eventData.lUTC_baseTOT	= dateData.getTime();
			}
		}

		public void setEventData_Time(long lUTC_Start, long lUTC_End)
		{
			eventData.lUTC_Start	= lUTC_Start;
			eventData.lUTC_End	= lUTC_End;
		}

		public DxbEventData getEventData()
		{
			return eventData;
		}

		public ArrayList<DxbChannelData> getChannelList(int iTab)
		{
			switch (iTab)
			{
			case DTV_TV:
				return tvChannelData;
			case DTV_RADIO:
				return radioChannelData;
			}
			return null;
		}

		public ArrayList<String> getNameList(int iTab)
		{
			if(iTab == 0)
			{
				return tvNameData;
			}
			return radioNameData;
		}

		public void setFileList_cursor(int position)
		{
			DxbLog(I, "setFileList_cursor(position=" + position + ")");
			mCursorFileList.moveToPosition(position);
		}

		private DxbChannelData CursorToChannelData(Cursor c)
		{
			if (c == null || c.getCount() == 0)
			{
				return null;
			}
			return new DxbChannelData(	c.getInt(getColumnIndex_ID()),
										c.getInt(getColumnIndex_ChannelNumber()),
										c.getInt(getColumnIndex_CountryCode()),
										c.getInt(getColumnIndex_Frequency()),
										c.getInt(getColumnIndex_ServiceType()),
										c.getInt(getColumnIndex_AudioPID()),
										c.getInt(getColumnIndex_VideoPID()),
										c.getInt(getColumnIndex_TeletextPID()),
										c.getInt(getColumnIndex_ServiceID()),
										c.getString(getColumnIndex_ServiceName()),
										c.getInt(getColumnIndex_FreeCAmode()),
										c.getInt(getColumnIndex_Scrambling()),
										c.getInt(getColumnIndex_Bookmark()),
										c.getInt(getColumnIndex_LogicalChannel())
									);
		}

		public void setCurrent_ChannelData()
		{
			DxbLog(I, "setCurrent_ChannelData()");
			DxbChannelData channeldata = null;
			
			switch (getServiceType())
			{
				case DTV_PVR:
				{
					setFileList_cursor(getChannelPosition());
					channeldata = CursorToChannelData(mCursorFileList);
				}
				break;
				
				case DTV_TV:
				{
					if(tvChannelData.size() <= getChannelPosition())
					{
						setChannelPosition(DTV_TV, 0);
					}
					channeldata = tvChannelData.get(getChannelPosition());
				}
				break;
				
				case DTV_RADIO:
				{
					if(radioChannelData.size() <= getChannelPosition())
					{
						setChannelPosition(DTV_RADIO, 0);
					}
					channeldata	= radioChannelData.get(getChannelPosition());
				}
				break;
				
				default:
				break;
			}
			
			mChannelData = channeldata;
		}
		
		public int getCurDVBSystem()
	    {
			return DVBSystem; 
		}

		public int getCurServiceID()
		{
			DxbChannelData channeldata = mChannelData;

			if (channeldata == null)
				return 0;

			return	channeldata.iID;
		}
		
		public int getCurProgramID()
		{
			DxbChannelData channeldata = mChannelData;

			if (channeldata == null)
				return 0;

			return channeldata.iLogical_channel;
		}

		public int getCurCountryCode()
		{
			DxbChannelData channeldata = mChannelData;

			if (channeldata == null)
				return 0;

			return channeldata.iCountry_code;
		}

		public int getCurFrequency()
		{
			DxbChannelData channeldata = mChannelData;

			if (channeldata == null)
				return 0;

			if (isDVB_S2())
				return getFrequencyDVBSx(channeldata.iChannel_number);
			return channeldata.iFrequency;
		}

		public int getCurChannelNumber()
		{
			DxbChannelData channeldata = mChannelData;

			if (channeldata == null)
				return 0;

			return channeldata.iChannel_number;
		}

		public int getCurChannelType()
		{
			DxbChannelData channeldata = mChannelData;

			if (channeldata == null)
				return 0;

			return channeldata.iService_type;
		}

		public String getCurChannelName()
		{
			DxbChannelData channeldata = mChannelData;

			if (channeldata == null)
				return "";

			return channeldata.sChannel_name;
		}

		public int getCurVideoPid()
		{
			DxbChannelData channeldata = mChannelData;
			
			if (channeldata == null)
				return 0;

			return channeldata.iVideo_PID;
		}
		
		public int getCurAudioPid()
		{
			DxbChannelData channeldata = mChannelData;

			if (channeldata == null)
				return 0;
						
			return channeldata.iAudio_PID;
		}
		
		public int getCurBandwidth()
		{
			return Bandwidth;
		}
		
		public int getCurRotation()
		{
			return Rotation;
		}
		
		public int getCurConstellation()
		{
			return Constellation;
		}
		
		public int getCurFecLength()
		{
			return FecLength;
		}
		
		public int getCurGuardInterval()
		{
			return GuardInterval;
		}
		
		public int getCurCodeRate()
		{
			return CodeRate;
		}
		
		public int getCurFft()
		{
			return Fft;
		}
		
		public int getCurPlp()
		{
			return Plp;
		}
		
		public int getCurPilotPp()
		{
			return PilotPp;
		}
		
		public void toggleBookmark(int position)
		{
			DxbLog(I, "toggleBookmark(position=" + position + ")");

			ArrayList<DxbChannelData>	playChannelData	= getChannelList(getServiceType());

			if(playChannelData != null)
			{
				DxbChannelData	p	= playChannelData.get(position);
				
				if(p.iBookmark == 1)
				{
					p.iBookmark	= 0;
					updateBookmark(p.iID, 0);
				}
				else
				{
					p.iBookmark	= 1;
					updateBookmark(p.iID, 1);
				}
			}
		}

		public String isBookmark(int position)
		{
			ArrayList<DxbChannelData>	playChannelData	= getChannelList(getServiceType());

			if(playChannelData != null)
			{
				DxbChannelData	p	= playChannelData.get(position);
				
				if(p != null)
				{
					if(p.iBookmark == 1)
					{
						return p.sChannel_name;
					}
				}
			}

			return null;
		}

		public void setChannel_Bookmark(int position)
		{
			DxbLog(I, "setChannel_Bookmark(position=" + position + ")");
			
			ArrayList<DxbChannelData>	playChannelData	= getChannelList(getServiceType());

			if(playChannelData != null)
			{
				int	iCount	= playChannelData.size();
				int	index	= 0;
				for(int i=0 ; i<iCount ; i++)
				{
					DxbChannelData	p	= playChannelData.get(i);
					
					if(p != null)
					{
						if(p.iBookmark == 1)
						{
							if(index == position)
							{
								resetChannel();
								setChannel(i);
								return;
							}
							
							index++;
						}
					}
				}
				
			}
		}

		public int getChannel_Count_DB(int iTab)
		{
			int	iCount	= 0;
			if(iTab < 2)
			{
				Cursor	cursor	= mChannelManager.getAllChannels(iTab+1);
				if(cursor != null)
				{
					iCount	= cursor.getCount();
					cursor.close();
				}
			}
			return iCount;
		}

	/************************************************************************************************************************
	 * 	DB control -----> End
	 ************************************************************************************************************************/

			
	/************************************************************************************************************************
	 * 	Signal -----> Start
	 ************************************************************************************************************************/
	private int	iStrength	= 0;
	private int	iQuality	= 0;
	private String	sMessage	= null;

	public int getSignalLevel()
	{
		return getSignalLevel(0, 0);
	}
	
	public int getSignalLevel(int frequency, int bandwidth)
	{
		//DxbLog(I, "getSignalLevel(" + frequency + ", " + bandwidth + ")");
		SignalState state = getSignalState();
		int	iSignal	= 4;
		
		if (state != null)
		{
			if(frequency!=0 && bandwidth!=0)
			{
				state.Strength	= frequency;
				state.Quality	= bandwidth;
			}
			
			iStrength	= state.Strength;
			iQuality	= state.Quality;
			sMessage	= state.Message;
				

			iSignal = iStrength/20;
			if(iSignal==1 && iQuality==0)
			{
				iSignal=0;
			}
				
			if (state.Message != null)
			{
				DxbLog(E, "[Signal]" + state.Message);
			}
		}
		if (iSignal > 4)
		{
			return 4;
		}
		else 
		{
			return iSignal;
		}
	}

	public int getSignalStrength()
	{
		return iStrength;
	}

	public int getSignalQuality()
	{
		return iQuality;
	}

	public String getSignalMessage()
	{
		return sMessage;
	}
	/************************************************************************************************************************
	 * 	Signal -----> End
	 ************************************************************************************************************************/

	/************************************************************************************************************************
	 *	(DVB Only) teletext, RadioImage, EPG -----> Start
	 ************************************************************************************************************************/
	public boolean updateBookmark(long rowId, int iBookmark)
	{
		return mChannelManager.updateBookmark(rowId, iBookmark);
	}

	public boolean isRadio() {
		DxbLog(I, "isRadio()");
		DxbChannelData channeldata = mChannelData;

		if (channeldata != null)
		{
			DxbLog(D, "mChannel.iVideo_PID=" + channeldata.iVideo_PID);
			if(channeldata.iVideo_PID == 0xFFFF)
			{
				return true;
			}
		}
		return false;
	}

	private int iTS_Scrambled = 0;
	public boolean isScrambled()
	{
		/*
		DxbChannelData channeldata = mChannelData;

		if (channeldata != null)
		{
			DxbLog(I, "isScrambled() --> mChannelData.iFree_CA_mode=" + channeldata.iFree_CA_mode + ", mChannelData.iScrambling=" + channeldata.iScrambling);
			if(		(channeldata.iFree_CA_mode == 1)
				||	(channeldata.iScrambling == 1)
			)
			{
				return true;
			}
		}
		return false;
		*/
		DxbLog(I, "isScrambled() --> iTS_Scrambled=" + iTS_Scrambled);
		return (iTS_Scrambled != 0);
	}

	private int getChannelType(int iType)
	{
		int	return_Type	= -1;
		switch(iType)
		{
			case ChannelManager.SERVICE_TYPE_DTV:
			case ChannelManager.SERVICE_TYPE_MHDTV:
			case ChannelManager.SERVICE_TYPE_ASDTV:
			case ChannelManager.SERVICE_TYPE_AHDTV:
				return_Type	= 0;
			break;
			
			case ChannelManager.SERVICE_TYPE_DRADIO:
			case ChannelManager.SERVICE_TYPE_FMRADIO:
			case ChannelManager.SERVICE_TYPE_ADRADIO:
				return_Type	= 1;
			break;
		}

		return return_Type;
	}

	public boolean isValidate_Teletext()
	{
		DxbLog(I, "isValidate_Teletext()");
		DxbChannelData channeldata = mChannelData;
		if (channeldata != null)
		{
			DxbLog(I, "mChannelData.iTeletext_PID=" + channeldata.iTeletext_PID);
			if(channeldata.iTeletext_PID == 0xFFFFFFFF)
			{
				return false;
			}
		}
		return true;
	}

	private final int AD_VOL_MIN	= -5;
	private final int AD_VOL_MAX	= 5;
	public void setRelativeVol(int vol)
	{
		int	setVol;
		if(vol < AD_VOL_MIN)
		{
			setVol	= AD_VOL_MIN;
		}
		else if(vol > AD_VOL_MAX)
		{
			setVol	= AD_VOL_MAX;
		}
		else
		{
			setVol	= vol;
		}
		
		if(cOption.relative_vol == setVol)
		{
			return;
		}
		
		cOption.relative_vol	= setVol;
		if(cOption.audio_description_on)
		{
			mSettingDB.putRelativeVol(1000 + cOption.relative_vol);
		}
		else
		{
			mSettingDB.putRelativeVol(cOption.relative_vol);
		}
		
		int	iVol	= (cOption.relative_vol+AD_VOL_MAX) * 10;
		DxbLog(D, "setRelativeVol(vol = " + vol + ")" + ", setVol = " + setVol + ", iVol=" + iVol);
		setVolume(1, iVol);
	}
	
	public void setAudioDescription(boolean bAD)
	{
		cOption.audio_description_on	= bAD;
		if(cOption.audio_description_on)
		{
			mSettingDB.putRelativeVol(1000 + cOption.relative_vol);
		}
		else
		{
			mSettingDB.putRelativeVol(cOption.relative_vol);
		}
		
		onOutputAudioDescription(cOption.audio_description_on);
	}
	
	public void setTimeZone(int iMode, int iIndex)
	{
		DxbLog(I, "setTimeZone()");
		cOption.time_zone_type		= iMode;
		
		if(iMode == 0)
		{
			setUserLocalTimeOffset(true, iIndex);
		}
		else if(iMode == 1)
		{
			DxbLog(D, "gInfo_Option.time_zone_index = " + cOption.time_zone_index);
			cOption.time_zone_index	= iIndex;
			mSettingDB.putTimeZone(cOption.time_zone_index);

			CharSequence[] csValue	= getResources().getTextArray(R.array.time_zone_manual_select_values);
			int	iValue	= Integer.parseInt((String) csValue[cOption.time_zone_index]);
			DxbLog(D, "iValue = " + iValue);
			setUserLocalTimeOffset(false, iValue);
		}
	}

	public void setParentalRate(int age)
	{
		DxbLog(I, "setParentalRate(age=" + age + ")");
		
		mSettingDB.putAge(age);
		
		if(cOption.age != age)
		{
			cOption.age	= age;
		}
		
		if(isParentalLock())
		{
			pause();
		}
		else
		{
			resume();
		}
	}

	public void setChangePassword(String newPassword)
	{
		DxbLog(I, "setChangePassword(" + cOption.password + " --> " + newPassword + ")");
		
		if(!newPassword.equalsIgnoreCase(cOption.password))
		{
			mSettingDB.putPassword(newPassword);
			cOption.password	= newPassword;
		}
	}
	/************************************************************************************************************************
	 *	(DVB Only) teletext, RadioImage, EPG -----> End
	 ************************************************************************************************************************/


	public void setBundleData(Bundle bundle)
	{
		DxbLog(I, "setBundleData(bundle = " + bundle + ")");
		
		Scheduler_Info.isScreenOn		= true;
		Scheduler_Info.isPlayerOn		= true;
		
		Scheduler_Info.Alarm_Type		= -1;
		Scheduler_Info.Channel_Type		= -1;
		Scheduler_Info.Channel_Index	= -1;
		Scheduler_Info.Alarm_ID			= -1;
		Scheduler_Info.Repeat_Type		= -1;

		if(bundle != null)
		{
			Scheduler_Info.isScreenOn		= bundle.getBoolean(Alert.EXTRA_TYPE_IS_SCREEN_ON);
			Scheduler_Info.isPlayerOn		= false;
			
			Scheduler_Info.Alarm_Type		= bundle.getInt(Alert.EXTRA_TYPE_ALARM, 0);
			
			Scheduler_Info.Channel_Type		= bundle.getInt(Alert.EXTRA_TYPE_CHANNEL_TYPE, 0);
			Scheduler_Info.Channel_Index	= bundle.getInt(Alert.EXTRA_TYPE_CHANNEL_INDEX, 0);
			
			Scheduler_Info.Alarm_ID			= bundle.getInt(Alert.EXTRA_TYPE_ID, 0);
			Scheduler_Info.Repeat_Type		= bundle.getInt(Alert.EXTRA_TYPE_REPEAT, 0);
			
			DxbLog(D, "isScreenOn = " + Scheduler_Info.isScreenOn);
			DxbLog(D, "isPlayerOn = " + Scheduler_Info.isPlayerOn);
			DxbLog(D, "iNotice_Type = " + Scheduler_Info.Alarm_Type);
			DxbLog(D, "iChannel_Type = " + Scheduler_Info.Channel_Type);
			DxbLog(D, "iChannel_Index = " + Scheduler_Info.Channel_Index);
			DxbLog(D, "iAlarm_ID = " + Scheduler_Info.Alarm_ID);
			DxbLog(D, "iRepeat_Type = " + Scheduler_Info.Repeat_Type);
		}
	}

	public void setSchedulerInfo_AlarmType(int iAlarm_Type)
	{
		Scheduler_Info.Alarm_Type		= iAlarm_Type;
	}

	public int getSchedulerInfo_AlarmType()
	{
		return Scheduler_Info.Alarm_Type;
	}

	public void setSchedulerInfo_ChannelType(int type)
	{
		Scheduler_Info.Channel_Type		= type;
	}

	public void setSchedulerInfo_ChannelIndex(int index)
	{
		Scheduler_Info.Channel_Index	= index;
	}

	public void setSchedulerInfo_AlarmID(int iID)
	{
		Scheduler_Info.Alarm_ID			= iID;
	}

	public int getSchedulerInfo_AlarmID()
	{
		return Scheduler_Info.Alarm_ID;
	}

	public void setSchedulerInfo_RepeatType(int iRepeat_Type)
	{
		Scheduler_Info.Repeat_Type		= iRepeat_Type;
	}

	public int getSchedulerInfo_RepeatType()
	{
		return Scheduler_Info.Repeat_Type;
	}

	public boolean isScreenOn()
	{
		return	Scheduler_Info.isScreenOn;
	}

	public void setScreenOn(boolean state)
	{
		Scheduler_Info.isScreenOn	= state;
	}

	public boolean isPlayerOn()
	{
		return	Scheduler_Info.isPlayerOn;
	}

	public void setPlayerOn(boolean state)
	{
		Scheduler_Info.isPlayerOn	= state;
	}


	/************************************************************************************************************************
	 * 	Abstract -----> Start
	 ************************************************************************************************************************/
	@Override
	protected void onCreate()
	{
		DxbLog(I, "onCreate()");
		db_open();
		prepare(mChannelManager.getFile().getPath());
	}

	@Override
	public void onDestroy()
	{
		DxbLog(I, "onDestroy()");
		db_close();
	}

	@Override
	protected void onPrepared() {
		setSurface();
		useSurface(0);
		
//		setTuneSpace(tuneSpace[cOption.countrycode]);
//		start(g_CountryCode[cOption.countrycode]);
		setAntennaPower(cOption.antenna_power);
		
		if (mOnPreparedListener != null) {
			mOnPreparedListener.onPrepared(DxbPlayer.this);
		}
		
		int	iVol	= (cOption.relative_vol+AD_VOL_MAX) * 10;
		setVolume(1, iVol);
		onOutputAudioDescription(cOption.audio_description_on);
	}

	@Override
	protected void onVideoDefinitionUpdate(VideoDefinitionData videoinfo) {
		isVideoPlay_output	= false;
		isVideoPlay_update	= true;
	}

	@Override
	protected void onDBInfoUpdate(int fromfile, int dbtype, DB_CHANNEL_Data dbinfo)
	{
		if (fromfile == 1)
		{
			mFileManager.onDBInfoUpdate(dbtype, dbinfo);
		}
		else
		{
			DxbChannelData channeldata = mChannelData;
			if (channeldata == null)
			{
				return;
			}
			
			if(		channeldata.iChannel_number == dbinfo.dbService.uiCurrentChannelNumber
				&&	channeldata.iCountry_code == dbinfo.dbService.uiCurrentCountryCode
				&&	channeldata.iService_ID == dbinfo.dbService.uiServiceID
			)
			{
				DxbLog(I, "onDBInfoUpdate() - update service db");

				stop();

				channeldata.iAudio_PID = dbinfo.dbService.uiAudioPID;
				channeldata.iVideo_PID = dbinfo.dbService.uiVideoPID;
				channeldata.iTeletext_PID = dbinfo.dbService.uiTeletextPID;
				channeldata.iScrambling = dbinfo.dbService.uiVideoIsScrambling;

				mChannelManager.onDBInfoUpdate(dbtype, dbinfo);

				setChannel();
			}
		}
	}

	@Override
	protected void onVideoOutputUpdate() {
		isVideoPlay_output	= true;
		if(mOnVideoOutputListener != null) {
			mOnVideoOutputListener.onVideoOutput(DxbPlayer.this);
		}
	}

	@Override
	protected void onError(int what, int extra) {
	}

	@Override
	protected void onDebugUpdate(DebugModeData data) {
		if (mOnDebugModeListener != null) {
			mOnDebugModeListener.onDebugUpdate(DxbPlayer.this, data);
		}
	}

	@Override
	protected void onDBInformationUpdate(int type, int param) {
		if (mOnDBInformationUpdateListener != null) {
			mOnDBInformationUpdateListener.onDBInformationUpdate(DxbPlayer.this, type, param);
		}
	}

	@Override
	protected void onPlayingCompletion(int type, int state) {
		if (mOnPlayingCompletionListener != null) {
			mOnPlayingCompletionListener.onPlayingCompletion(DxbPlayer.this, type, state);
		}
	}

	@Override
	protected void onFilePlayTimeUpdate(int time) {
		if (mOnFilePlayTimeUpdateListener != null) {
			mOnFilePlayTimeUpdateListener.onFilePlayTimeUpdate(DxbPlayer.this, time);
		}
	}

	@Override
	protected void onRecordingCompletion(int result) {
		if (mOnRecordingCompletionListener != null) {
			mOnRecordingCompletionListener.onRecordingCompletion(DxbPlayer.this, result);
		}
	}

	@Override
	protected void onSearchPercentUpdate(int nPercent) {
		if (mOnSearchPercentListener != null) {
			mOnSearchPercentListener.onSearchPercentUpdate(DxbPlayer.this, nPercent);
		}
	}

	@Override
	protected void onSearchCompletion() {
		int count = 0;
		Cursor c;
		c = mChannelManager.getAllChannels(1);
		if (c != null)
		{
			count += c.getCount();
			c.close();
		}
		c = mChannelManager.getAllChannels(2);
		if (c != null)
		{
			count += c.getCount();
			c.close();
		}
		if (count == 0)
		{
			mChannelManager.restore();
			restoreChannelPosition(DTV_TV);
			restoreChannelPosition(DTV_RADIO);
		}
		
		if (mOnSearchCompletionListener != null)
		{
			if(mChannelManager!=null)
			{
				mChannelManager.eliminateChannel();
				mChannelManager.check_logicalChannel();
			}
			else
			{
				DxbLog(W, "ChannelManager not ready ");
			}

			mOnSearchCompletionListener.onSearchCompletion(DxbPlayer.this);
		}
	}

	@Override
	protected void onBlindScanUpdate(int nPercent, int index, int freqMHz, int symbolKHz) {
		DxbLog(I, "onBlindScanUpdate(" + nPercent + "%, POL: " + mTsList[index].getPolarization() + ", Freq: " + freqMHz + " MHz, Symbol: " + symbolKHz + " KHz)");
		if (mOnBlindScanPercentListener != null) {
			mOnBlindScanPercentListener.onBlindScanPercentUpdate(DxbPlayer.this, nPercent, index, freqMHz, symbolKHz);
		}
	}

	@Override
	protected void onBlindScanCompletion() {
		DxbLog(I, "onBlindScanCompletion()");
		
		if (mOnBlindScanCompletionListener != null) {
			mOnBlindScanCompletionListener.onBlindScanCompletion(DxbPlayer.this);
		}
	}

	@Override
	protected void onTeletextDataUpdate(Bitmap bitmap) {
		if (mOnTeletextDataUpdateListener != null) {
			mOnTeletextDataUpdateListener.onTeletextDataUpdate(DxbPlayer.this, bitmap);
		}
	}

	@Override
	protected void onSetChannelCompletion() {
		int iTab = getServiceType();
		if (iTab != DTV_PVR) {
			mSettingDB.putPosition(iTab, getChannelPosition(iTab));
			setTimeZone(cOption.time_zone_type, cOption.time_zone_index);
		}
		setSubtitleInformation(getSubtitle_LanguageCode());
		setTeletextInformation(getTeletext_Descriptor(1), getTeletext_Descriptor(2));
		resumeSubtitle();
	}

	@Override
	protected void onScrambledStatus(int pid, int scrambled) {
		DxbChannelData channeldata = mChannelData;
		if (channeldata == null) {
			return;
		}
		if (channeldata.iVideo_PID == pid) {
			iTS_Scrambled = scrambled;
			if (mOnScrambledStatusListener != null) {
				mOnScrambledStatusListener.onScrambledStatus(DxbPlayer.this);
			}
		}
	}

	/************************************************************************************************************************
	 * 	Abstract -----> End
	 ************************************************************************************************************************/

	/************************************************************************************************************************
	 * 	Listener -----> Start
	 ************************************************************************************************************************/
	public interface OnPreparedListener {
		void onPrepared(DxbPlayer player);
	}

	public void setOnPreparedListener(OnPreparedListener listener) {
		mOnPreparedListener = listener;
	}

	private OnPreparedListener mOnPreparedListener;

	public interface OnVideoOutputListener {
		void onVideoOutput(DxbPlayer player);
	}
	
	public void setOnVideoOutputListener(OnVideoOutputListener listener) {
		mOnVideoOutputListener	= listener;
	}
	
	private OnVideoOutputListener mOnVideoOutputListener;

	public interface OnDebugModeListener {
		void onDebugUpdate(DxbPlayer player, DebugModeData data);
	}

	public void setOnDebugModeListener(OnDebugModeListener listener) {
		mOnDebugModeListener = listener;
	}

	private OnDebugModeListener	mOnDebugModeListener;

	public interface OnDBInformationUpdateListener {
		void onDBInformationUpdate(DxbPlayer player, int type, int param);
	}

	public void setOnDBInformationUpdateListener(OnDBInformationUpdateListener listener) {
		mOnDBInformationUpdateListener = listener;
	}

	private OnDBInformationUpdateListener mOnDBInformationUpdateListener;

	public interface OnPlayingCompletionListener {
		void onPlayingCompletion(DxbPlayer player, int type, int state);
	}

	public void setOnPlayingCompletionListener(OnPlayingCompletionListener listener) {
		mOnPlayingCompletionListener = listener;
	}

	private OnPlayingCompletionListener mOnPlayingCompletionListener;

	public interface OnFilePlayTimeUpdateListener {
		void onFilePlayTimeUpdate(DxbPlayer player, int time);
	}

	public void setOnFilePlayTimeUpdateListener(OnFilePlayTimeUpdateListener listener) {
		mOnFilePlayTimeUpdateListener = listener;
	}

	private OnFilePlayTimeUpdateListener mOnFilePlayTimeUpdateListener;

	public interface OnRecordingCompletionListener {
		void onRecordingCompletion(DxbPlayer player, int result);
	}

	public void setOnRecordingCompletionListener(OnRecordingCompletionListener listener) {
		mOnRecordingCompletionListener = listener;
	}

	private OnRecordingCompletionListener mOnRecordingCompletionListener;

	public interface OnSearchPercentListener {
		void onSearchPercentUpdate(DxbPlayer player, int nPercent);
	}

	public void setOnSearchPercentListener(OnSearchPercentListener listener) {
		mOnSearchPercentListener = listener;
	}

	private OnSearchPercentListener mOnSearchPercentListener;

	public interface OnSearchCompletionListener {
		void onSearchCompletion(DxbPlayer player);
	}

	public void setOnSearchCompletionListener(OnSearchCompletionListener listener) {
		mOnSearchCompletionListener = listener;
	}

	private OnSearchCompletionListener mOnSearchCompletionListener;

	public interface OnBlindScanPercentListener {
		void onBlindScanPercentUpdate(DxbPlayer player, int nPercent, int index, int freqMHz, int symbolKHz);
	}

	public void setOnBlindScanPercentListener(OnBlindScanPercentListener listener) {
		mOnBlindScanPercentListener = listener;
	}

	private OnBlindScanPercentListener mOnBlindScanPercentListener;

	public interface OnBlindScanCompletionListener {
		void onBlindScanCompletion(DxbPlayer player);
	}

	public void setOnBlindScanCompletionListener(OnBlindScanCompletionListener listener) {
		mOnBlindScanCompletionListener = listener;
	}

	private OnBlindScanCompletionListener mOnBlindScanCompletionListener;

	public interface OnTeletextDataUpdateListener {
		void onTeletextDataUpdate(DxbPlayer player, Bitmap bitmap);
	}

	public void setOnTeletextDataUpdateListener(OnTeletextDataUpdateListener listener) {
		mOnTeletextDataUpdateListener = listener;
	}

	private OnTeletextDataUpdateListener mOnTeletextDataUpdateListener;

	public interface OnScrambledStatusListener {
		void onScrambledStatus(DxbPlayer player);
	}

	public void setOnScrambledStatusListener(OnScrambledStatusListener listener) {
		mOnScrambledStatusListener = listener;
	}

	private OnScrambledStatusListener mOnScrambledStatusListener;

	/************************************************************************************************************************
	 * 	Listener -----> End
	 ************************************************************************************************************************/

	public String getVideoInfo() {
		VideoDefinitionData videoinfo = getVideoInformation();
		if (videoinfo == null)
			return "";
		return videoinfo.toString();
	}

	public int getFrameRate()
	{
		VideoDefinitionData videoinfo = getVideoInformation();
		if (videoinfo == null)
			return 0;
		return videoinfo.frame_rate;
	}

	private boolean isVideoPlay_output	= false;
	private boolean isVideoPlay_update	= true;
	public boolean isVideoPlay()
	{
		if(		isVideoPlay_output
			&&	isVideoPlay_update
		)
		{
			return true;
		}
		
		return false;
	}


/*****************************************************************************************************************************************************************************
 *	PVR
 *****************************************************************************************************************************************************************************/
	public boolean isFilePlay() {
		return (getRecordState() == DxbPVR.REC_STATE.STOP
			&& getLocalPlayState() == DxbPVR.LOCAL_STATE.PLAYING);
	}

	public boolean setPlay() {
		if (getServiceType() != DTV_PVR) {
			if(		(mRecordPosition == getChannelPosition())
				&&	(mRecordTab == getServiceType()))
			{
				return setPlay(null);
			}
		}
		return false;
	}

	@Override
	public int setCapture(String name) {
		if (getServiceType() != DTV_PVR) {
			return super.setCapture(name);
		}
		return 1;
	}

	@Override
	public boolean setRecord(String name) {
		if (getServiceType() != DTV_PVR) {
			mRecordTab = getServiceType();
			mRecordPosition = getChannelPosition();
			return super.setRecord(name);
		}
		return false;
	}

	public void setPlayPause(int serviceType, boolean bPlayPause)
	{
		DxbLog(I, "setFilePlayPause(serviceType=" + serviceType + ", bPlayPause=" + bPlayPause + ")");
		if (serviceType == DTV_PVR)
		{
			if (bPlayPause)
				setPlayResume();
			else
				setPlayPause();
		}
		else
		{
			if (bPlayPause)
				resume();
			else
				pause();
		}
	}


/*****************************************************************************************************************************************************************************
 *	Channel
 *****************************************************************************************************************************************************************************/
	@Override
	public void stop()
	{
		super.stop();
		setUnlockParental(false);
	}

	@Override
	public void resetChannelPosition()
	{
		super.resetChannelPosition();
		for (int tab = 0; tab < mCurrPosition.length; tab++)
		{
			setChannelPosition(tab, 0);
		}
	}

	public void setChannelPosition(int tab, int index)
	{
		if (tab < mCurrPosition.length)
		{
			mPrevPosition[tab] = mCurrPosition[tab];
			mCurrPosition[tab] = index;
		}
	}

	public int getChannelPosition(int tab)
	{
		if (tab < mCurrPosition.length)
			return mCurrPosition[tab];
		return -1;
	}

	public void restoreChannelPosition(int tab)
	{
		if (tab < mCurrPosition.length)
			mCurrPosition[tab] = mPrevPosition[tab];
	}

	public void changeChannle_LogicalChannel(int iLogical_Channel)
	{
		DxbLog(I, "changeChannle_LogicalChannel(iLogical_Channel=" + iLogical_Channel + ")");
		
		ArrayList<DxbChannelData>	cur_Data	= getChannelList(getServiceType());
		 
		if(cur_Data != null)
		{
			int iCount	= cur_Data.size();
			for(int i=0 ; i<iCount ; i++)
			{
				DxbChannelData	p	= cur_Data.get(i);
				if(p.iLogical_channel == iLogical_Channel)
				{
					setChannel(i);
					return;
				}
			}
		}
	}

	public boolean setChannel()
	{
		if(		(Scheduler_Info.Channel_Type >= 0)
			&&	(Scheduler_Info.Channel_Index >= 0)
		)
		{
			mTab = Scheduler_Info.Channel_Type;
			setChannelPosition(Scheduler_Info.Channel_Type, Scheduler_Info.Channel_Index);

			Scheduler_Info.Channel_Type		= -1;
			Scheduler_Info.Channel_Index	= -1;
		}
		return setChannel(getChannelPosition());
	}

	public boolean setChannelUp()
	{
		DxbLog(D, "setChannelUp()");
		resetChannel();
		return setChannel(getChannelPosition() - 1);
	}

	public boolean setChannelDown()
	{
		DxbLog(D, "setChannelDown()");
		resetChannel();
		return setChannel(getChannelPosition() + 1);
	}

	public boolean setChannel(int iChannel)
	{
		int iChannelCount = getChannelCount();
		int iType = getServiceType();
		int iPrevChannel = getChannelPosition();

		if (iChannelCount <= 0)
		{
			eState = STATE.GENERAL;
			DxbLog(E, "[setChannel] No Channel");
			return false;
		}

		if(eState == STATE.SCAN)
		{
			DxbLog(E, "[setChannel] Search");
			return false;
		}

		while(iChannel < 0)
			iChannel += iChannelCount;
		while(iChannel >= iChannelCount)
			iChannel -= iChannelCount;

		DxbLog(I, "setChannel : Start - Type: " + iType  + ", Ch: " + iChannel);

		setChannelPosition(iType, iChannel);
		setCurrent_ChannelData();

		if (iType != DTV_PVR)
		{
			if(getChannelPosition() != iPrevChannel)
			{
				resetChannel();
				setUnlockParental(false);
			}
		}

		DxbChannelData channeldata = mChannelData;

		if (channeldata == null)
		{
			return false;
		}

		if (iType == DTV_PVR)
		{
			setPlay(DxbStorage.getPath_Device() + "/" + channeldata.sChannel_name);
		}
		else
		{
			setSatelliteID(channeldata.iCountry_code);
			setChannel(channeldata.iCountry_code, channeldata.iChannel_number, channeldata.iID, cOption.language_audio, cOption.language_subtitle);
		}

		if (mOnChannelChangeListener != null)
		{
			mOnChannelChangeListener.onChannelChange();
		}

		return true;
	}

	public int getServiceType()
	{
		return mTab;
	}

	public void setServiceType(int iType)
	{
		DxbLog(I, "setServiceType(iType=" + iType + ")");
		mTab = iType;
	}

	public int getChannelCount()
	{
		return getChannelCount(getServiceType());
	}

	public int getChannelCount(int iTab)
	{
		switch (iTab)
		{
		case DTV_TV:
			return tvChannelData.size();
		case DTV_RADIO:
			return radioChannelData.size();
		}
		return mFileCount;
	}

	public int getChannelPosition()
	{
		return getChannelPosition(getServiceType());
	}

	public void resetChannel()
	{
		DxbLog(I, "resetChannel()");
		mPF_EPG = null;
		cOption.language_audio	= 0;
		if (cOption.language_subtitle > 0)
		{
			cOption.language_subtitle	= 1;
		}
		stopTtxSubtitle();
	}

	public interface OnChannelChangeListener
	{
		void onChannelChange();
	}

	private OnChannelChangeListener mOnChannelChangeListener;		

	public void setOnChannelChangeListener(OnChannelChangeListener listener)
	{
		mOnChannelChangeListener = listener;
	}


/*****************************************************************************************************************************************************************************
 *	Scan
 *****************************************************************************************************************************************************************************/
	@Override
	public void searchCancel()
	{
		eState  = STATE.SCAN_STOP;
		super.searchCancel();
	}

	public boolean search()
	{
		eState = STATE.SCAN;

		mChannelManager.backup();
		mChannelManager.deleteAllChannel();

		resetChannelPosition();

		return search(getTuneSpace());
	}

	public boolean search(int[] index_list)
	{
		eState = STATE.SCAN;

		mChannelManager.backup();
		mChannelManager.deleteAllChannel();

		resetChannelPosition();

		return search(getTuneSpace(), index_list);
	}

	public boolean manualScan(int KHz_frequency, int Mhz_bandwidth)
	{
		eState = STATE.SCAN;

		setChannelPosition(DTV_TV, tvChannelData.size());
		setChannelPosition(DTV_RADIO, radioChannelData.size());

		return search(KHz_frequency, Mhz_bandwidth);
	}

	public boolean blindScan()
	{
		eState = STATE.SCAN;

		TuneSpace tuneSpace = null;

		mDish = mDishSetupManager.getDishSetup(mCountryCode[cOption.countrycode]);
		mTsList = mDishSetupManager.getTsList(mDish);
		if (mTsList != null && mTsList.length != 0)
		{
			tuneSpace = new TuneSpace(mTsList.length);
			tuneSpace.countryCode = mCountryCode[cOption.countrycode];
			tuneSpace.space = 2;
			tuneSpace.minChannel = 0;
			tuneSpace.maxChannel = mTsList.length;
			for (int i = 0; i < mTsList.length; i++)
			{
				tuneSpace.channels[i].frequency1 = mDish.getLnbLoLof();
				tuneSpace.channels[i].frequency2 = mTsList[i].getFrequency()*1000;
				tuneSpace.channels[i].bandwidth1 = mTsList[i].getSymbolRate();
			}
		}

		return blindscan(tuneSpace);
	}


/*****************************************************************************************************************************************************************************
 *	Audio
 *****************************************************************************************************************************************************************************/

	public String changeAudio(int value)
	{
		String	return_audio	= null;
		Cursor audio_cursor	= getAudio_LanguageCode();
		
		if(audio_cursor != null)
		{
			int iCount_Language_Audio	= audio_cursor.getCount();
			if (iCount_Language_Audio > 0)
			{
				if(value == 1)
				{
					cOption.language_audio++;
				}
				else if(value == -1)
				{
					cOption.language_audio--;
				}

				if(cOption.language_audio < 0)
					cOption.language_audio = audio_cursor.getCount() - 1;

				cOption.language_audio %= iCount_Language_Audio;
				audio_cursor.moveToPosition(cOption.language_audio);
				int strSize = audio_cursor.getString(1).length();

				if(value != 0)
					setAudio(cOption.language_audio);

				try
				{
					if(strSize >= 3)
					{
						return_audio	= audio_cursor.getString(1).substring(0, 3);
					}
					else
					{
						return_audio	= audio_cursor.getString(1).substring(0, strSize);
					}
				}
				catch(StringIndexOutOfBoundsException e)
				{
					return_audio = "";
				}
			}
			
			audio_cursor.close();
		}
		
		return return_audio;
	}

	public Cursor getAudio_LanguageCode()
	{
		DxbLog(I, "getAudio_LanguageCode()");
		DxbChannelData channeldata = mChannelData;
		if (channeldata != null)
		{
			if (isFilePlay())
			{
				return mFileManager.getAudio_LanguageCode(channeldata.iService_ID, channeldata.iChannel_number);
			}
			else
			{
				return mChannelManager.getAudio_LanguageCode(channeldata.iService_ID, channeldata.iChannel_number);
			}
		}
		return null;
	}

/*****************************************************************************************************************************************************************************
 *	Subtitle
 *****************************************************************************************************************************************************************************/

	public int getSubtitleCount(int i)
	{
		if (i == 0)
		{
			return getSubtitleCount();
		}
		else
		{
			return getTtxSubtitleCount();
		}
	}

	public void resumeSubtitle()
	{
		DxbLog(I, "resumeSubtitle()");
		playSubtitle(cOption.language_subtitle);
	}

	private boolean flag_subtitle_enable	= false;
	public void setSubtitleEnable(boolean state)
	{
		flag_subtitle_enable	= state;
	}
	
	public boolean isSubtitleEnable()
	{
		return	flag_subtitle_enable;
	}
	
	public void playSubtitle(int iIndex)
	{
		DxbLog(I, "playSubtitle(" + iIndex + ")");

		if(!isPlayTTX())
		{
			int iSubtitle = getSubtitleCount();
			int iTtxSubtitle = getTtxSubtitleCount();
			if(		(iIndex > 0)
				&&	isSubtitleEnable()
			)
			{
				if(cOption.language_subtitle <= iSubtitle)
				{
					startSubtitle(cOption.language_subtitle - 1);
				}
				else if(iTtxSubtitle > 0 && cOption.language_subtitle <= iSubtitle + iTtxSubtitle)
				{
					startTtxSubtitle(cOption.language_subtitle - 1 - iSubtitle);
				}
			}
			else
			{
				stopSubtitle();
				stopTtxSubtitle();
			}
		}
	}

	public String changeSubtitle(int value)
	{
		String	return_Subtitle	= null;
		Cursor cursorSubtitle	= getSubtitle_LanguageCode();

		if(cursorSubtitle != null)
		{
			if (getSubtitleCount() + getTtxSubtitleCount() > 0)
			{
				cOption.language_subtitle = value;
				cOption.language_subtitle %= (getSubtitleCount() + getTtxSubtitleCount() + 1);

				playSubtitle(cOption.language_subtitle);

				if (cOption.language_subtitle == 0)
				{
				}
				else if(cOption.language_subtitle <= getSubtitleCount()) // subtitle
				{
					cursorSubtitle.moveToPosition(cOption.language_subtitle-1);
					try
					{
						return_Subtitle	= cursorSubtitle.getString(1).substring(0, 3);
					}
					catch(StringIndexOutOfBoundsException e)
					{
						return_Subtitle = "";
					}
				}
				else if (getTtxSubtitleCount() > 0) // teletext subtitle
				{
					int i = cOption.language_subtitle - getSubtitleCount() - 1;
					return_Subtitle	= getTTX_LanguageCode(i);
				}
			}
			cursorSubtitle.close();
		}
		return return_Subtitle;
	}

	public String[] getEntrySubtitle() {
		Cursor cursorSubtitle = getSubtitle_LanguageCode();
		ArrayList<CharSequence> Entries = new ArrayList<CharSequence>();
		String langCode;

		Entries.add(getResources().getString(R.string.off));

		if(((cursorSubtitle == null)
			|| (cursorSubtitle.getCount() <= 0))
			&&	(getSubtitleCount(1) <= 0))
		{
			Entries.add(getResources().getString(R.string.on_no_information));
		}
		else
		{
			for(int i=1; i<=getSubtitleCount(); i++)
			{
				try
				{
					DxbLog(E, "getEntrySubtitle() ==> StringIndexOutOfBoundsException");
					langCode = cursorSubtitle.getString(1).substring(0, 3);
				}
				catch(StringIndexOutOfBoundsException e)
				{
					langCode = "";
				}

				Entries.add(DxbUtil.getLanguageText(langCode));
				cursorSubtitle.moveToNext();
			}

			for(int i=1 ; i<= getSubtitleCount(1) ; i++)
			{
				langCode = getTTX_LanguageCode(i-1);
				Entries.add("Teletext - " + DxbUtil.getLanguageText(langCode));
			}
			cursorSubtitle.close();
		}
		String[] Entry = Entries.toArray(new String[Entries.size()]);

		return Entry;
	}
	
	public String changeStereoMode(int value)
	{
		String	return_stereo	= null;
		
		if(value == 1)
		{
			cOption.audio_mode++;
			
			if(cOption.audio_mode > 3)
				cOption.audio_mode = 0;
		}
		else if(value == -1)
		{
			cOption.audio_mode--;
			
			if(cOption.audio_mode < 0)
				cOption.audio_mode = 3;
		}
		
		//if(value != 0)
			setStereo(cOption.audio_mode);
		
		DxbLog(E, "value ====> " + value);
		switch(cOption.audio_mode)
		{
			case 0:
				return_stereo = "Stereo";
				break;
			case 1:
				return_stereo = "Stereo LL";
				break;
			case 2:
				return_stereo = "Stereo RR";
				break;
			case 3:
				return_stereo = "Stereo LR";
				break;
		}
		
		return return_stereo;
	}

	public Cursor getSubtitle_LanguageCode()
	{
		DxbLog(I, "getSubtitle_LanguageCode()");
		DxbChannelData channeldata = mChannelData;
		if (channeldata != null)
		{
			if (isFilePlay())
			{
				return mFileManager.getSubtitle_LanguageCode(channeldata.iService_ID, channeldata.iChannel_number);
			}
			else
			{
				return mChannelManager.getSubtitle_LanguageCode(channeldata.iService_ID, channeldata.iChannel_number);
			}
		}
		return null;
	}


/*****************************************************************************************************************************************************************************
 *	EPG
 *****************************************************************************************************************************************************************************/
	public void setEPG_PF()
	{
		DxbLog(I, "setEPG_PF()");
		DxbChannelData channeldata = mChannelData;

		if (channeldata != null)
		{
			requestEPGUpdate(0);
			mPF_EPG = mChannelManager.getEPG_PF_Information(channeldata.iService_ID, channeldata.iChannel_number, channeldata.iCountry_code);
		}
	}

	public boolean isCurrEPG()
	{
		return (mPF_EPG != null
			&&	mPF_EPG[0] != null);
	}

	private boolean isEPGUpdate_state	= false;
	public void setEPGUpdate_state(boolean state)
	{
		isEPGUpdate_state	= state;
	}
	
	public boolean isEPGUpdate()
	{
		return isEPGUpdate_state;
	}

	public int getCurrEPG_StartTime()
	{
		EpgData[] epg = mPF_EPG;
		if (epg != null && epg[0] != null)
			return epg[0].iStartHH * 60 + epg[0].iStartMM;
		return 0;
	}

	public int getCurrEPG_DurationTime()
	{
		EpgData[] epg = mPF_EPG;
		if (epg != null && epg[0] != null)
			return epg[0].iDurationHH * 60 + epg[0].iDurationMM;
		return 0;
	}

	public String getCurrEPG_Title()
	{
		EpgData[] epg = mPF_EPG;
		if (epg != null && epg[0] != null)
		{
			if (epg[0].sName != null)
			{
				if (epg[0].sName.length() > 41)
				{
					return epg[0].sName.substring(0, 36) + " ... ";
				}
				else
				{
					return epg[0].sName;
				}
			}
		}
		return null;
	}
	
	public int getCurrentEPG_Rating()
	{
		EpgData[] epg = mPF_EPG;
		if (epg != null && epg[0] != null)
			return epg[0].iRating;
		return 0;
	}
	
	public boolean isNextEPG()
	{
		return (mPF_EPG != null
			&&	mPF_EPG[1] != null);
	}

	public int getNextEPG_StartTime()
	{
		EpgData[] epg = mPF_EPG;
		if (epg != null && epg[1] != null)
			return epg[1].iStartHH * 60 + epg[1].iStartMM;
		return 0;
	}

	public int getNextEPG_DurationTime()
	{
		EpgData[] epg = mPF_EPG;
		if (epg != null && epg[1] != null)
			return epg[1].iDurationHH * 60 + epg[1].iDurationMM;
		return 0;
	}

	public String getNextEPG_Title()
	{
		EpgData[] epg = mPF_EPG;
		if (epg != null && epg[1] != null)
		{
			if (epg[1].sName != null)
			{
				if (epg[1].sName.length() > 41)
				{
					return epg[1].sName.substring(0, 36) + " ... ";
				}
				else
				{
					return epg[1].sName;
				}
			}
		}
		return null;
	}

	public int getNextEPG_Rating()
	{
		EpgData[] epg = mPF_EPG;
		if (epg != null && epg[1] != null)
			return epg[1].iRating;
		return 0;
	}

	private int iCurrentRating	= 0;
	public void setCurrentRating(int iRating)
	{
		DxbLog(I, "setCurrentRating(iRating=" + iRating + ")");
		iCurrentRating	= iRating;
	}
	
	public int getCurrentRating()
	{
		DxbLog(I, "getCurrentRating() - iCurrentRating = " + iCurrentRating);
		return iCurrentRating;
	}
	
	public boolean isParentalLock()
	{
		DxbLog(I, "isParentalLock()");
		
		if(		(getCurrentRating() != 0)
			&&	(cOption.age != 0)
			&&	(getCurrentRating() > cOption.age)
		)
		{
			DxbLog(D, "return true");
			return true;
		}
		
		DxbLog(D, "return false");
		return false;
	}
	
	private boolean bUnlockParental_state	= false;
	public void setUnlockParental(boolean state)
	{
		DxbLog(I, "setUnlockParental(" + state + ")");
		
		bUnlockParental_state	= state;
		
		if(!state)
		{
			iTS_Scrambled = 0;
			isVideoPlay_output	= false;
			if(mOnVideoOutputListener != null)
			{
				mOnVideoOutputListener.onVideoOutput(DxbPlayer.this);
			}
		}
	}
	
	public boolean isUnlockParental()
	{
		return bUnlockParental_state;
	}


/*****************************************************************************************************************************************************************************
 *	Teletext
 *****************************************************************************************************************************************************************************/
	public Cursor getTeletext_Descriptor(int iType)
	{
		DxbLog(I, "getTeletext_Descriptor(iType=" + iType +")");

		/*	teletext type
		 * 	0x00	- reserved for future use
		 * 	0x01	- initial teletext page
		 * 	0x02	- teletext subtitle page
		 * 	0x03	- additional information page
		 * 	0x04	- program schedule page
		 * 	0x05~0x1F	- reserved for future use
		 */

		DxbChannelData channeldata = mChannelData;
		if (channeldata != null)
		{
			if (isFilePlay())
			{
				return mFileManager.getTTX_Descriptor(channeldata.iService_ID, channeldata.iChannel_number, channeldata.iCountry_code, iType);
			}
			else
			{
				return mChannelManager.getTTX_Descriptor(channeldata.iService_ID, channeldata.iChannel_number, channeldata.iCountry_code, iType);
			}
		}
		return null;
	}

	public Cursor getEPG_Schedule()
	{
		DxbLog(I, "getEPG_Schedule()");
		DxbChannelData channeldata = mChannelData;
		if (channeldata != null)
		{
			return mChannelManager.getEPG_SCHEDULE_Information(channeldata.iService_ID, channeldata.iChannel_number, channeldata.iCountry_code);
		}
		return null;
	}

/*****************************************************************************************************************************************************************************
 *	DVBSx
 *****************************************************************************************************************************************************************************/
	private DishSetupManager mDishSetupManager = null;
	private Tuner[]          mTsList;
	private Dish             mDish;

	private void initDVBSx(Context context)
	{
		mDishSetupManager = new DishSetupManager(context);
		setSatelliteID(0);
	}

	private void startDVBSx()
	{
		mDishSetupManager.open();
	}

	private void stopDVBSx()
	{
		mDishSetupManager.close();
	}

	public void resetDVBSx()
	{
		stopDVBSx();
		initDVBSx(getContext());
		startDVBSx();
	}

	private int getFrequencyDVBSx(int channel_number)
	{
		Tuner[] tsList = mTsList;
		if (tsList != null && tsList.length > channel_number)
		{
			return tsList[channel_number].getFrequency()*1000;
		}
		return 0;
	}

	public int getPolarization(int index)
	{
		return mTsList[index].getPolarization();
	}

	private TuneSpace getTuneSpaceForDVBSx(int group_id)
	{
		TuneSpace tuneSpace = null;
		mDish = mDishSetupManager.getDishSetup(group_id);
		mTsList = mDishSetupManager.getTsList(group_id);
		if (mTsList != null && mTsList.length != 0)
		{
			tuneSpace = new TuneSpace(mTsList.length);
			tuneSpace.countryCode = mCountryCode[cOption.countrycode];
			tuneSpace.space = 2;
			tuneSpace.minChannel = 0;
			tuneSpace.maxChannel = mTsList.length;
			for (int i = 0; i < mTsList.length; i++)
			{
				tuneSpace.channels[i].frequency1 = DiSEqCDev.getIntermediateFrequency(mTsList[i], mDish)*1000;
				tuneSpace.channels[i].frequency2 = mTsList[i].getFrequency()*1000;
				tuneSpace.channels[i].bandwidth1 = mTsList[i].getSymbolRate();
				tuneSpace.channels[i].fec_inner	= mTsList[i].getFecInner();
			}
		}
		
		return tuneSpace;
	}

	public void setSatelliteID(int i)
	{
		if (isDVB_S2())
		{
			mCountryCode[cOption.countrycode] = i;
		}
	}

	public int getSatelliteID()
	{
		return mCountryCode[cOption.countrycode];
	}

	public void setSatelliteName(String name)
	{
		if (isDVB_S2())
		{
			sSatellite_Name	= name;
		}
	}
	
	public String getSatelliteName()
	{
		return sSatellite_Name;
	}

	@Override
	public void onDishSetup(int index)
	{
		if (isDVB_S2())
		{
			Dish dish = mDish;
			Tuner[] tsList = mTsList;
			if (dish == null)
			{
				DxbLog(E, "[DishSetup] mTsList is null");
				return;
			}
			if (tsList == null || tsList.length <= index)
			{
				DxbLog(E, "[DishSetup] mTsList is null");
				return;
			}
			ExecuteDiSEqC(tsList[index], dish);
		}
	}
}
