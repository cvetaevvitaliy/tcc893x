package com.telechips.android.atsc.samples;

import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Color;
import android.widget.TextView;
import android.os.PowerManager;
import android.util.Log;

import com.telechips.android.atsc.*;
import com.telechips.android.atsc.samples.DxbAdapter_Service.ViewHolder;

public class DxbPlayer_Control
{
	private static final String TAG = "[[[DxbPlayer_Control]]]";
	
	/*	Visible Check	*/
	public static final boolean	VISIBLE_ID_list_tab	= true;
	
	private final static int MENU_CHANNEL	= 1;
	private final static int MENU_EPG		= 2;
	private final static int MENU_SCREEN	= 3;
	private final static int MENU_OPTION	= 4;

	private final static int KOREA_TERESTRIAL		= 1;
	private final static int KOREA_CABLE 			= 2;
	private final static int KOREA_TERESTRIAL_TEST	= 3;
	private final static int KOREA_CABLE_TEST		= 4;


	public static final int		DEFAULT_VALUE_AREA_CODE			= -1;
	public static final String	DEFAULT_VALUE_PASSWORD			= "0000";	// default
	public static final int 	DEFAULT_VALUE_PARENTAL_RATING	= -1;		// default
	public static final int 	DEFAULT_VALUE_VIDEO				= -1;		// default
	public static final int 	DEFAULT_VALUE_AUDIO				= -1;		// default
	public static final int 	DEFAULT_VALUE_DUAL_AUDIO		= -1;		// default
	public static final int 	DEFAULT_VALUE_SUBTITLE			= 1;		// default off(0)/on(1)
	public static final int		DEFAULT_VALUE_SUBTITLE_LANGUAGE	= -1;		// default off(0)/on(1)
	public static final int		DEFAULT_VALUE_CLOSEDCAPTION		= 0;		// default off(0)/on(1)
	public static final int		DEFAULT_VALUE_CAPTIONSVCNUM		= 0;		// servicenumber 1(1), 2(2),3(3),4(4),5(5),6(6)
	public static final int 	DEFAULT_VALUE_INPUT				= 0;		// default AIR(0)/CABLE(1)
	public static final int 	DEFAULT_VALUE_AUDIOPREF			= 1;		// default ENG(0)/KOR(1)	
	public static final int 	DEFAULT_VALUE_ASPECTRATIO		= 2;		// default LETTERBOX(0)/PANSCAN(1)/FULL(2)
	public static final int		DEFAULT_VALUE_CAPTION			= -1;		// default off(0)/on(1)
	public static final int 	DEFAULT_VALUE_SUPER_IMPOSE		= -1;		// default

	public static int		options_area_code			= DEFAULT_VALUE_AREA_CODE;
	public static String	options_password		= DEFAULT_VALUE_PASSWORD;
	public static int		options_parental_rating	= DEFAULT_VALUE_PARENTAL_RATING;
	public static int		options_video			= DEFAULT_VALUE_VIDEO;
	public static int		options_audio			= DEFAULT_VALUE_AUDIO;
	public static int		options_dual_audio		= DEFAULT_VALUE_DUAL_AUDIO;
	public static int		options_subtitle		= DEFAULT_VALUE_SUBTITLE;
	public static int		options_subtitle_language	= DEFAULT_VALUE_SUBTITLE_LANGUAGE;	
	public static int		options_closedcaption	= DEFAULT_VALUE_CLOSEDCAPTION;
	public static int		options_captionsvcnum	= DEFAULT_VALUE_CAPTIONSVCNUM;	
	public static int		options_input			= DEFAULT_VALUE_INPUT;
	public static int		options_audiopref			= DEFAULT_VALUE_AUDIOPREF;	
	public static int		options_aspect_ratio	= DEFAULT_VALUE_ASPECTRATIO;
	public static int		options_caption			= DEFAULT_VALUE_CAPTION;
	public static int		options_super_impose	= DEFAULT_VALUE_SUPER_IMPOSE;

	/*	Player	*/
	public static ATSCPlayer mPlayer;
	public static ChannelManager mChannelManager = null;
	private static Channel mChannel;

	/*	Scan	*/
	private static int iCountryCode = 1;

	/*	Service	*/
	public static final int	SERVICE_TYPE_TV		= 0x01;
	public static final int SERVICE_TYPE_RADIO	= 0x02;

	public static int tvPosition 		= 0;
	public static int radioPosition 	= 0;

	/*	EPG	*/
	public static int epgPosition = 0;
	public static int mTotalEPGCount = 0;
	public static int epgFirst = 0, epgLast = 0;

	public static TextView epgText_name;
	public static TextView epgText_detail;

	private static int iIndex_serviceType, iIndex_serviceName, iIndex_chmajornum, iIndex_chminornum, iIndex_modulation;

	private static int iIndex_startYY, iIndex_startMON, iIndex_startDD, iIndex_startWD;
	private static int iIndex_startHH, iIndex_startMM, iIndex_durationHH, iIndex_durationMM, iIndex_name, iIndex_text;

	// Setting DB
	public static DxbDB_Setting mDB;
	public static Intent intentSubActivity;

	private static final int MODULATION_VSB = 8;
	private static final int MODULATION_QAM = 256;

	private static final int INPUT_AIR 			= 0x1;
	private static final int INPUT_CABLE		= 0x3;
	private static final int INPUT_TEST_AIR		= 0x4;
	private static final int INPUT_TEST_CABLE	= 0xc;

	private static PowerManager.WakeLock mWakeLock = null;

	public static void createDxbPlayer() 
	{
		Log.d(TAG, "createDxbPlayer");

		if(mPlayer == null) 
		{
			mPlayer = new ATSCPlayer();
		}

		if(mChannel == null) 
		{
			mChannel = new Channel();
		}
	}

	public static void start() 
	{
		Log.d(TAG, "start()");

		if(mPlayer != null) 
		{
			Log.d(TAG, "start() iCountryCode"+ iCountryCode);
			Log.d(TAG, "start() options_input"+ options_input);
			if ( options_input == 0 )
				iCountryCode = 1;// air
			else if ( options_input == 1 )
				iCountryCode = 2; // cable
			else if ( options_input == 2 )
				iCountryCode = 3; // air_test
			else if ( options_input == 3 )
				iCountryCode = 4; // cable_test

			mPlayer.start(iCountryCode);
		}
	}

	static SampleATSCPlayer mContext;
	public static void prepare(SampleATSCPlayer context)
	{
		Log.d(TAG, "prepare()");
		if(mPlayer != null)
		{
			mPlayer.setWakeMode(context, PowerManager.SCREEN_BRIGHT_WAKE_LOCK);
			mPlayer.prepare("/data/data/com.telechips.android.atsc.samples/databases/");
		}
		mContext = context;
	}

	public static void onFinish()
	{
		mContext.finish();
	}

	public static void setSurface() 
	{
		Log.d(TAG, "setSurface()");

		if(mPlayer != null) 
		{
			mPlayer.setSurface();
		}
	}

	public static void releaseSurface()
	{
		Log.d(TAG, "releaseSurface()");
		if(mPlayer != null)
		{
			mPlayer.playSubtitle(0);
			mPlayer.releaseSurface();
		}
	}

	public static void setLCDUpdate()
	{
		if(mPlayer != null)
		{
			//mPlayer.setLCDUpdate();
		}
	}

	public static void useSurface(int arg)
	{
		Log.i(TAG, "useSurface(arg = " + arg + ")");

		if(mPlayer != null)
		{
			mPlayer.useSurface(arg);
		}
	}

	public static void initWakeLock(Context context)
	{
		Log.d(TAG, "initWakeLock()"+mWakeLock);
		if( mWakeLock == null)
		{
			PowerManager powerManager = (PowerManager)context.getSystemService(Context.POWER_SERVICE);
			mWakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, context.getClass().getName());
			mWakeLock.setReferenceCounted(false);
		}
	}

	public static void destroyWakeLock()
	{
		Log.d(TAG, "destroyWakeLock()"+mWakeLock);
		if(mWakeLock != null)
		{
			if( mWakeLock.isHeld())
			{
				mWakeLock.release();
			}
			mWakeLock = null;
		}
	}

	public static void acquireWakeLock()
	{
		Log.d(TAG, "acquireWakeLock()"+mWakeLock);
		if(mWakeLock != null )
		{
			if( mWakeLock.isHeld() == false)
				mWakeLock.acquire();
		}
	}

	public static void releaseWakeLock()
	{
		Log.d(TAG, "releaseWakeLock()"+mWakeLock);
		if( mWakeLock != null )
		{
			if( mWakeLock.isHeld())
				mWakeLock.release();
		}
	}
	
	public static void search(int inputSource) 
	{
		Log.d(TAG, "search()");

		if(mPlayer != null) 
		{
			Log.d(TAG, "search.1");
			if ( inputSource == 0 )
				mPlayer.search(KOREA_TERESTRIAL, INPUT_AIR);
			else if ( inputSource == 1 )
				mPlayer.search(KOREA_CABLE, INPUT_CABLE);
			else if ( inputSource == 2 ) // TEST_AIR
				mPlayer.search(KOREA_TERESTRIAL_TEST, INPUT_TEST_AIR);
			else if ( inputSource == 3 ) // TEST_CABLE
				mPlayer.search(KOREA_CABLE_TEST, INPUT_TEST_CABLE);
			Log.d(TAG, "search.2");
		}
	}

	public static void searchCancel() 
	{
		Log.d(TAG, "searchCancel()");

		if(mPlayer != null) 
		{
			mPlayer.searchCancel();
		}
	}

	public static void setChannel(Cursor mChannelsCursor) 
	{
		Log.d(TAG, "setChannel()");

		if(		(mChannelsCursor != null)
			&&	(mChannel != null)
			&&	(mPlayer != null)
		)
		{
			Cursor c = mChannelsCursor;

			mChannel.m_iCountryCode		= c.getInt(1);
			mChannel.m_iChannelNumber	= c.getInt(2);			
			mChannel.m_iModulation		= c.getInt(3);
			mChannel.m_iServiceType 	= c.getInt(4);
			mChannel.m_iService_ID		= c.getInt(5);
			mChannel.m_iCHMajorNum		= c.getInt(6);
			mChannel.m_iCHMinorNum		= c.getInt(7);
			mChannel.m_szServiceName	= c.getString(8);

			Log.d(TAG, "m_iServiceType=" + mChannel.m_iServiceType);
			Log.d(TAG, "m_iChannelNumber=" + mChannel.m_iChannelNumber);
			Log.d(TAG, "m_iModulation=" + mChannel.m_iModulation);
			Log.d(TAG, "m_iServiceType=" + mChannel.m_iServiceType);
			Log.d(TAG, "m_iService_ID=" + mChannel.m_iService_ID);
			Log.d(TAG, "m_iCHMajorNum=" + mChannel.m_iCHMajorNum);
			Log.d(TAG, "m_iCHMinorNum=" + mChannel.m_iCHMinorNum);

			mPlayer.setChannel(c.getInt(0));
		}
	}

	public static int getClearChannel()
	{
		Log.d(TAG, "getClearChannel()");

		mChannel.m_iCountryCode		= 0;
		mChannel.m_iChannelNumber	= 0;			
		mChannel.m_iModulation		= 0;
		mChannel.m_iServiceType 	= 0;
		mChannel.m_iService_ID		= 0;
		mChannel.m_iCHMajorNum		= 0;
		mChannel.m_iCHMinorNum		= 0;
		mChannel.m_szServiceName	= null;

		return 0;
	}

	public static int getChannelNumber()
	{
		return mChannel.m_iChannelNumber;
	}

	public static int getLogicalCHNumber() 
	{
		return mChannel.m_iCHMajorNum;
	}

	public static int getModulation() 
	{
		return mChannel.m_iModulation;
	}

	public static int getLogicalCHMinorNumber() 
	{
		return mChannel.m_iCHMinorNum;
	}

	public static String getServiceName() 
	{
		return mChannel.m_szServiceName;
	}

	public static int getplayServiceType()
	{
		return mChannel.m_iServiceType;
	}

	public static boolean isVisible_Tab() 
	{
		return VISIBLE_ID_list_tab;
	}

	public static int getTab() 
	{
		return	SampleATSCPlayer.tabHost.getCurrentTab();
	}

	public static Cursor getChannels(int iTab) 
	{
		Log.d(TAG, "getChannels(iTab="+iTab+")");
		if(mChannelManager != null)
		{
			return mChannelManager.getAllChannels(iTab+1);
		}
		else
		{
			return null;
		}
	}

	public static int getServiceType(int id) 
	{
		int	return_type = -1;

		switch(id) 
		{
			case ChannelManager.SERVICE_TYPE_DTV:
				return_type	= SERVICE_TYPE_TV;
				break;

			case ChannelManager.SERVICE_TYPE_DRADIO:
				return_type	= SERVICE_TYPE_RADIO;
				break;

			default:
				break;
		}

		return return_type;
	}

	public static boolean openEPGDB() 
	{
		if(mChannelManager != null)
		{
			return mChannelManager.openEPGDB();
		}
		else
		{
			return false;
		}
	}

	public static void closeEPGDB() 
	{
		if(mChannelManager != null)
		{
			mChannelManager.closeEPGDB();
		}
	}

	public static Cursor getEPG_Schedule() 
	{
		if(mChannelManager != null)
		{
			return mChannelManager.getEPG_PF_Information(mChannel.m_iService_ID, mChannel.m_iChannelNumber, mChannel.m_iCountryCode);
		}
		else
		{
			return null;
		}
	}

	public static Cursor getEPG_Schedule_byDay(int day) 
	{
		if(mChannelManager != null)
		{
			return mChannelManager.getEPG_PF_Information_byDay(mChannel.m_iService_ID, mChannel.m_iChannelNumber, mChannel.m_iCountryCode, day);
		}
		else
		{
			return null;
		}
	}

	public static int getMenuState(int menu) 
	{
		int	return_State = 0xFF;

		switch(menu)
		{
			case MENU_CHANNEL:
				return_State = SampleATSCPlayer.MENU_CHANNEL;
				break;

			case MENU_EPG:
				return_State = SampleATSCPlayer.MENU_EPG;
				break;

			case MENU_SCREEN:
				return_State = SampleATSCPlayer.MENU_SCREEN;
				break;

			case MENU_OPTION:
				return_State = SampleATSCPlayer.MENU_OPTION;
				break;
		}

		return return_State;
	}

	public static void setService_ColumnIndex(Cursor cursor) 
	{
		Log.d(TAG, "getColumnIndices()");
		if(cursor != null) 
		{
			iIndex_serviceType	= cursor.getColumnIndexOrThrow(ChannelManager.KEY_SERVICE_TYPE);
			iIndex_serviceName	= cursor.getColumnIndexOrThrow(ChannelManager.KEY_SERVICE_NAME);
			iIndex_chmajornum	= cursor.getColumnIndexOrThrow(ChannelManager.KEY_CHMAJORNUM);
			iIndex_chminornum	= cursor.getColumnIndexOrThrow(ChannelManager.KEY_CHMINORNUM);
			iIndex_modulation = cursor.getColumnIndexOrThrow(ChannelManager.KEY_MODULATION);
		}
	}

	public static void bindView(Cursor cursor, ViewHolder vh) 
	{
		//Log.d(TAG, "bindView()");
		String channelnum = "";
		int serviceType = cursor.getInt(iIndex_serviceType);
		String serviceName = cursor.getString(iIndex_serviceName);
		int	CHMajorNum = cursor.getInt(iIndex_chmajornum);
		int	CHMinorNum = cursor.getInt(iIndex_chminornum);
		int	iModulation = cursor.getInt(iIndex_modulation);

		int	position = cursor.getPosition();
		if(		(		getTab() == 0 
					&&	(tvPosition != position)
				)
			||	(		getTab() == 1 
					&&	(radioPosition != position)
				)
		)
		{
			vh.name.setTextColor(Color.rgb(204, 204, 204));
			vh.num.setTextColor(Color.rgb(204, 204, 204));

			switch(serviceType) 
			{
				case SERVICE_TYPE_TV:
					vh.icon_text.setBackgroundResource(R.drawable.dxb_portable_list_tv_icon_n);
					break;

				case SERVICE_TYPE_RADIO:
					vh.icon_text.setBackgroundResource(R.drawable.dxb_portable_list_radio_icon_n);
					break;

				default:
					break;
			}
		}
		else
		{
			vh.name.setTextColor(Color.rgb(255,255,255));
			vh.num.setTextColor(Color.rgb(255,255,255));

			switch(serviceType) 
			{
				case SERVICE_TYPE_TV:
					vh.icon_text.setBackgroundResource(R.drawable.dxb_portable_list_tv_icon_f);
					break;

				case SERVICE_TYPE_RADIO:
					vh.icon_text.setBackgroundResource(R.drawable.dxb_portable_list_radio_icon_f);
					break;

				default:
					break;
			}
		}
		if( iModulation == MODULATION_VSB )
			channelnum = String.format("%3d-%1d", CHMajorNum, CHMinorNum);
		else if( iModulation == MODULATION_QAM)
			channelnum = String.format("%3d", CHMajorNum);

		vh.num.setText(channelnum);
		vh.name.setText(serviceName);
	}

	public static void setEPG_ColumnIndex(Cursor cursor) 
	{
		if(cursor != null)
		{
			iIndex_startYY 		= cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_START_YEAR);
			iIndex_startMON		= cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_START_MONTH);
			iIndex_startDD 		= cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_START_DAY);
			iIndex_startWD		= cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_START_WDAY);
			iIndex_startHH		= cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_START_HOUR);
			iIndex_startMM		= cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_START_MINUTE);
			iIndex_durationHH	= cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_DURATION_HOUR);
			iIndex_durationMM	= cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_DURATION_MINUTE);

			iIndex_name			= cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_EVT_NAME);
			iIndex_text			= cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_EVT_EXTERN);
		}
		else 
		{
			Log.d(TAG, "if(cursor == null)");
		}
	}

	public static String getEPG_Time(Cursor cursor) 
	{
		int	iStartHH	= cursor.getInt(iIndex_startHH);
		int	iStartMM	= cursor.getInt(iIndex_startMM);
		int	idurationHH	= cursor.getInt(iIndex_durationHH);
		int	idurationMM	= cursor.getInt(iIndex_durationMM);
		int	iEndHH		= (iStartHH + idurationHH + (iStartMM + idurationMM)/60) % 24;
		int	iEndMM		= (iStartMM + idurationMM) % 60;

		return String.format("%02d:%02d ~ %02d:%02d", iStartHH, iStartMM, iEndHH, iEndMM);
	}

	public static String getEPG_Time_Detail(Cursor cursor) 
	{
		int	iStartYY	= cursor.getInt(iIndex_startYY);
		int	iStartMON	= cursor.getInt(iIndex_startMON);
		int	iStartDD	= cursor.getInt(iIndex_startDD);		
		int	iStartHH	= cursor.getInt(iIndex_startHH);
		int	iStartMM	= cursor.getInt(iIndex_startMM);
		int	idurationHH	= cursor.getInt(iIndex_durationHH);
		int	idurationMM	= cursor.getInt(iIndex_durationMM);
		int	iEndHH		= (iStartHH + idurationHH + (iStartMM + idurationMM)/60) % 24;
		int	iEndMM		= (iStartMM + idurationMM) % 60;

		String sStartWD	= cursor.getString(iIndex_startWD);

		String sName = cursor.getString(iIndex_name);

		return String.format("%d.%d.%d(%s) %02d:%02d ~ %02d:%02d %s", 
				iStartYY, iStartMON, iStartDD, sStartWD, iStartHH, iStartMM, iEndHH, iEndMM, sName);
	}

	public static String getEPG_Name(Cursor cursor) 
	{
		return cursor.getString(iIndex_name);
	}

	public static void setEPG_DisplayDate(Cursor cursor)
	{
		int	iStartYY	= cursor.getInt(iIndex_startYY);
		int	iStartMON	= cursor.getInt(iIndex_startMON);
		int	iStartDD	= cursor.getInt(iIndex_startDD);

		String	date	= String.format("%d.%d.%d", iStartYY, iStartMON, iStartDD);

		SampleATSCPlayer.epgTextView_Current.setText(date);
	}

	public static void getEPG_DisplayDate(Cursor cursor)
	{
		if(cursor != null)
		{
			int	iStartYY	= cursor.getInt(iIndex_startYY);
			int	iStartMON	= cursor.getInt(iIndex_startMON);
			int	iStartDD	= cursor.getInt(iIndex_startDD);

			String	date	= String.format("%4d.%02d.%02d", iStartYY, iStartMON, iStartDD);
			Log.i(TAG, "getEPG_DisplayDate"+date);
		}
		return;
	}

	public static int getEPG_StartUTCTime(Cursor cursor)
	{
		if(cursor != null)
		{
			int	iStartMON	= cursor.getInt(iIndex_startMON);
			int	iStartDD	= cursor.getInt(iIndex_startDD);

			String	date	= String.format("%02d%02d", iStartMON, iStartDD);
			Log.i(TAG, "getEPG_StartUTCTime"+date);
			int	iStartUTC = Integer.parseInt(date);

			return iStartUTC;
		}
		else
			return 0;
	}

	public static int getEPG_StartDay(Cursor cursor)
	{
		if(cursor != null)
		{
			int	iStartDD	= cursor.getInt(iIndex_startDD);

			return iStartDD;
		}
		else
			return 0;
	}

	public static int getEPG_StartYEAR(Cursor cursor)
	{
		if(cursor != null)
		{
			int	iStartYY	= cursor.getInt(iIndex_startYY);

			return iStartYY;
		}
		else
			return 0;
	}
	public static Cursor getEPGInfo(int iService_ID, int iChannelNumber, int index)
	{
		return mChannelManager.getEPGInfo(iService_ID, iChannelNumber, mChannel.m_iCountryCode,index);
	}

	public static int getEPGFirstIndex(int iService_ID, int iChannelNumber)
	{
		Cursor cursor;
		int	iEPG_rowid = 0;
		cursor = mChannelManager.getEPGInfoFirstIdx(iService_ID, iChannelNumber, mChannel.m_iCountryCode);

		if( cursor != null )
			iEPG_rowid	= cursor.getInt(0);
		cursor.close();

		return iEPG_rowid;
	}

	public static int	 getEPGLastIndex(int iService_ID, int iChannelNumber)
	{
		Cursor cursor;
		int	iEPG_rowid = 0;
		cursor = mChannelManager.getEPGInfoLastIdx(iService_ID, iChannelNumber, mChannel.m_iCountryCode);

		if( cursor != null )
			iEPG_rowid	= cursor.getInt(0);
		cursor.close();

		return iEPG_rowid;
	}

	public static String getEPG_Text(Cursor cursor) 
	{
		return cursor.getString(iIndex_text);
	}
	
	public static void setAudio(int iIndex) 
	{
		Log.d(TAG, "setAudio()");
	}

	public static void setAudioDualMono(int iIndex) 
	{
		//config. dual-monoral AAC
		Log.d(TAG, "setMultiAudio");
		//mPlayer.setAudioDualMono(iIndex);
	}	

	public static void enableCC(int iCCEnable) 
	{
		Log.d(TAG, "enableCC() : "+iCCEnable);
		if(mPlayer != null)
		{
			mPlayer.enableCC(iCCEnable);
		}
	}

	public static void setCCServiceNum(int iCCSrvNum) 
	{
		Log.d(TAG, "setCCServiceNum() : "+iCCSrvNum);
		if(mPlayer != null)
		{
			mPlayer.setCCServiceNum(iCCSrvNum);
		}
	}

	public static void setAudioLanguage(int iAudioLang) 
	{
		Log.d(TAG, "setAudioLanguage() : "+iAudioLang);
		if(mPlayer != null)
		{
			mPlayer.setAudioLanguage(iAudioLang);
		}
	}

	public static void setAspectRatio(int iaspectRatio) 
	{
		Log.d(TAG, "setAspectRatio() : "+iaspectRatio);
		if(mPlayer != null)
		{
			mPlayer.setAspectRatio(iaspectRatio);
		}
	}

	public static void setParentalRate(int age)
	{
	}

	public static void playSubtitle(int state) 
	{
		Log.d(TAG, "playSubtitle(state="+state+")");
		if(mPlayer != null)
		{
			mPlayer.playSubtitle(state);
		}
	}

	public static void setCapture(String name) 
	{
		Log.d(TAG, "setCapture()");
		mPlayer.setCapture(name);
	}

	public static void setRecord(String name) 
	{
		Log.d(TAG, "setRecord()");
		mPlayer.setRecord(name);
	}

	public static void setRecStop()
	{
		Log.d(TAG, "setRecStop()");
		mPlayer.setRecStop();
	}

	public static String getCurrent_ServiceName() 
	{
		Log.d(TAG, "getCurrent_ServiceName()");
		return ChannelManager.KEY_SERVICE_NAME;
	}

	public static String getCurrent_ServiceType() 
	{
		Log.d(TAG, "getCurrent_ServiceType()");
		return ChannelManager.KEY_SERVICE_TYPE;
	}

	public static int getRid_Indicator_Section() 
	{
		return -1;
	}

	public static int getRid_LayoutList() 
	{
		return R.id.layout_list;
	}

	public static int getRid_List_ServiceList() 
	{
		return R.id.service_list;
	}

	public static int getRid_ListMessage() 
	{
		return R.id.list_message;
	}

	public static int getRid_ListView()
	{
		return R.id.list_view;
	}

	public static int getRid_ListImage()
	{
		return R.id.list_image;
	}

	public static int getRid_ListTitleImage()
	{
		return R.id.list_title_image;
	}

	public static int getRid_ListCount()
	{
		return R.id.list_count;
	}
	
	public static int getRid_ListTitle()
	{
		return R.id.list_atsc_title;
	}

	public static int getRid_LayoutFull()
	{
		return R.id.layout_full_4menu;
	}

	public static int getRid_FullView()
	{
		return R.id.full_4menu_view;
	}

	public static int getRid_FullImage()
	{
		return R.id.full_4menu_image;
	}

	public static int getRid_FullText()
	{
		return R.id.full_4menu_text;
	}

	public static int getRid_FullGroup()
	{
		return R.id.full_4menu_group;
	}
	
	public static int getRid_FullTitle()
	{
		return R.id.full_4menu_title;
	}
	
	public static int getRid_FullCHup()
	{
		return R.id.full_4menu_ch_up;
	}
	
	public static int getRid_FullCHdown()
	{
		return R.id.full_4menu_ch_down;
	}

	public static int getRid_FullMenu_bg()
	{
		return R.drawable.dxb_portable_toolbar_4bg_01_f;
	}

	public static int getRid_FullMenu1()
	{
		return R.id.full_4menu1;
	}

	public static int getRid_FullMenu2()
	{
		return R.id.full_4menu2;
	}

	public static int getRid_FullMenu3()
	{
		return R.id.full_4menu3;
	}

	public static int getRid_FullMenu4()
	{
		return R.id.full_4menu4;
	}

	public static int getRid_FullMenu1_Icon()
	{
		return R.id.full_4menu1_icon;
	}

	public static int getRid_FullMenu2_Icon()
	{
		return R.id.full_4menu2_icon;
	}

	public static int getRid_FullMenu3_Icon()
	{
		return R.id.full_4menu3_icon;
	}

	public static int getRid_FullMenu4_Icon()
	{
		return R.id.full_4menu4_icon;
	}

	public static int getRid_FullMenu1_Text()
	{
		return R.id.full_4menu1_text;
	}

	public static int getRid_FullMenu2_Text()
	{
		return R.id.full_4menu2_text;
	}

	public static int getRid_FullMenu3_Text()
	{
		return R.id.full_4menu3_text;
	}

	public static int getRid_FullMenu4_Text()
	{
		return R.id.full_4menu4_text;
	}

	public static int getRid_EPGImage()
	{
		return R.id.epg_image;
	}

	public static int getRdrawable_ListBG()
	{
		Log.d(TAG, "getRdrawable_ListBG()");
		return R.drawable.dxb_portable_channel_bg;
	}

	public static int getRdrawable_ListImage_Video()
	{
		Log.d(TAG, "getResource_ListVideoImage() - mChannel.serviceType=(" + mChannel.m_iServiceType + ")");
		int	return_Resource	= -1;
		
		switch(mChannel.m_iServiceType)
		{
			case ChannelManager.SERVICE_TYPE_DTV:
				break;
			
			case ChannelManager.SERVICE_TYPE_DRADIO:
				return_Resource	= R.drawable.dxb_portable_channel_radio_img;
				break;

			default:
				return_Resource	= R.drawable.dxb_portable_channel_no_play_img;
				break;
		}
		
		return return_Resource;
	}

	public static int getRdrawable_FullMenuIcon(int index, boolean state)
	{
		int	return_Resource	= -1;

		switch(index)
		{
			case MENU_CHANNEL:
				if(state == true)
				{
					return_Resource	= R.drawable.dxb_portable_toolbar_channel_btn_f;
				}
				else
				{
					return_Resource	= R.drawable.dxb_portable_toolbar_channel_btn_n;
				}
			break;

			case MENU_SCREEN:
				if(state == true)
				{
					return_Resource	= R.drawable.dxb_portable_toolbar_screen_btn_f;
				}
				else
				{
					return_Resource	= R.drawable.dxb_portable_toolbar_screen_btn_n;
				}
			break;

			case MENU_EPG:
				if(state == true)
				{
					return_Resource	= R.drawable.dxb_portable_toolbar_epg_btn_f;
				}
				else
				{
					return_Resource	= R.drawable.dxb_portable_toolbar_epg_btn_n;
				}
			break;

			case MENU_OPTION:
				if(state == true)
				{
					return_Resource	= R.drawable.dxb_portable_toolbar_option_btn_f;
				}
				else
				{
					return_Resource	= R.drawable.dxb_portable_toolbar_option_btn_n;
				}
			break;
			
			default:
				break;
		}
		return return_Resource;
	}

	public static int getRdrawable_epgBG()
	{
		Log.d(TAG, "getRdrawable_epgBG()");
		return R.drawable.dxb_portable_epg_bg;
	}

	public static int getRstring_FullMenuText(int index)
	{
		int	return_Resource	= -1;
		
		switch(index)
		{
			case MENU_CHANNEL:
				return_Resource = R.string.channel;
				break;

			case MENU_EPG:
				return_Resource	= R.string.epg;
				break;

			case MENU_SCREEN:
				return_Resource = R.string.screen;
				break;

			case MENU_OPTION:
				return_Resource	= R.string.option;
				break;

			default:
				break;
		}
		return return_Resource;
	}

	public static Class getClass_Option()
	{
		Log.d(TAG, "getClass_Option()");
		return DxbOptions_ATSC.class;
	}

	public static int getMask_Options()
	{
		Log.d(TAG, "getMask_Options()");
		int	return_Mask	= 		SampleATSCPlayer.MASK_OPTIONS_CLOSEDCAPTION
								|SampleATSCPlayer.MASK_OPTIONS_INPUT
								|SampleATSCPlayer.MASK_OPTIONS_AUDIOPREF
								|SampleATSCPlayer.MASK_OPTIONS_ASPECTRATIO;
		
		return	return_Mask;
	}

	public static String getKEY_AREA_CODE_SELECT()
	{
		return SampleATSCPlayer.KEY_AREA_CODE_SELECT;
	}

	public static String getKEY_PARENTAL_RATING_SELECT()
	{
		return SampleATSCPlayer.KEY_PARENTAL_RATING_SELECT;
	}

	public static String getKEY_VIDEO_SELECT()
	{
		return SampleATSCPlayer.KEY_VIDEO_SELECT;
	}

	public static String getKEY_AUDIO_SELECT()
	{
		return SampleATSCPlayer.KEY_AUDIO_SELECT;
	}

	public static String getKEY_DUAL_AUDIO_SELECT()
	{
		return SampleATSCPlayer.KEY_DUAL_AUDIO_SELECT;
	}

	public static String getKEY_SUBTITLE_SELECT()
	{
		return SampleATSCPlayer.KEY_SUBTITLE_SELECT;
	}

	public static String getKEY_CLOSEDCAPTION_SELECT()
	{
		return SampleATSCPlayer.KEY_CLOSEDCAPTION_SELECT;
	}

	public static String getKEY_CAPTIONSVCNUM_SELECT()
	{
		return SampleATSCPlayer.KEY_CAPTIONSVCNUM_SELECT;
	}

	public static String getKEY_AUDIOPREF_SELECT()
	{
		return SampleATSCPlayer.KEY_AUDIO_PREF_SELECT;
	}

	public static String getKEY_ASPECTRATIO_SELECT()
	{
		return SampleATSCPlayer.KEY_ASPECT_RATIO_SELECT;
	}

	public static String getKEY_INPUT_SELECT() 
	{
		return SampleATSCPlayer.KEY_INPUT_SELECT;
	}

	public static String getKEY_SCAN_SELECT() 
	{
		return SampleATSCPlayer.KEY_SCAN_SELECT;
	}

	public static String getKEY_CAPTION_SELECT()
	{
		return SampleATSCPlayer.KEY_CAPTION_SELECT;
	}

	public static String getKEY_SUPER_IMPOSE_SELECT()
	{
		return SampleATSCPlayer.KEY_SUPER_IMPOSE_SELECT;
	}

	public static Cursor getCursor_EPG()
	{
		Log.d(TAG, "getCursor_EPG()");
		// DVB-T is not action
		return null;
	}
}
