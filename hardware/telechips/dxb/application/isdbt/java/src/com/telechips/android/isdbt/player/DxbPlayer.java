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

import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.LinkedList;
import java.util.Queue;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.database.Cursor;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Display;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.RelativeLayout;
import android.widget.RelativeLayout.LayoutParams;
import android.graphics.Bitmap;
import android.graphics.Rect;
import android.graphics.Point;

import com.telechips.android.isdbt.DxbOptions;
import com.telechips.android.isdbt.DxbView;
import com.telechips.android.isdbt.R;
import com.telechips.android.isdbt.player.isdbt.*;
import com.telechips.android.isdbt.schedule.DxbEventData;
import com.telechips.android.isdbt.util.DxbStorage;
import com.telechips.android.isdbt.util.DxbUtil;

public class DxbPlayer extends SurfaceView
{
	public static final int	_OFF_	= 0;
	public static final int	_ON_	= 1;
	
	// Player
	public static enum PLAYER {
		ATSC,
		DVBT,
		ISDBT,
		TDMB,
		NULL
	}
	public PLAYER ePLAYER = PLAYER.ISDBT;

	// LocalPlay State
	public static enum LOCAL_STATE {
		STOP,
		PLAYING,
		PAUSE
	}
	private LOCAL_STATE eLocalPlayingState = LOCAL_STATE.STOP;

	// Dxb_Player State
	public static enum STATE {
		GENERAL,
		OPTION_FULL_SCAN,
		OPTION_MANUAL_SCAN,
		OPTION_MAKE_PRESET,
		SCAN,
		SCAN_STOP,
		UI_PAUSE,
		OPTION_AUTOSEARCH_UP,
		OPTION_AUTOSEARCH_DN,
		NULL
	}
	public STATE eState = STATE.GENERAL;

	public static enum REC_STATE {
		STOP,
		RECORDING,
		RECORDING_ALARM,
		NULL
	}
	private REC_STATE eRecState = REC_STATE.STOP;

	// Channel Information
	private class ChannelInfo
	{
		int	iTab		= 0;
		int	iCount		= 0;
		int	iPosition	= 0;

		int	iCount_TV		= 0;
		int iCurrent_TV;

		int iCurrent_File;

		int iRecordTab  = 0;
		int iRecordPosition = 0;
		
		Cursor	cursorFileList	= null;
	}
	private ChannelInfo cChannelInfo = new ChannelInfo();

	// Subtitle Information
	private int mSubtitleCount = 0;

	// Option
	public static int		DEFAULT_VALUE_COUNTRY           = 0;      // (0)JAPAN/(1)BRAZIL
	public static int		DEFAULT_VALUE_AUDIO             = 0;      // (0)Audio1/(1)Audio2
	public static int		DEFAULT_VALUE_DUAL_AUDIO	    = 0;      // (0)Main/(1)Sub/(2)Main+Sub
	public static int		DEFAULT_VALUE_CAPTION           = 1;      // (0)off/(1)Language1/(2)Language2
	public static int		DEFAULT_VALUE_SUPER_IMPOSE      = 0;      // (0)off/(1)Language1/(2)Language2
	public static int		DEFAULT_VALUE_AREA_0            = -1;
	public static int		DEFAULT_VALUE_AREA_1            = -1;
	public static int 		DEFAULT_VALUE_SCAN				= -1;
	public static int 		DEFAULT_VALUE_SCAN_MANUAL       = 0;
	public static int		DEFAULT_VALUE_AGE               = 5;      // (0)age of 10/(1)age of 12/(2)age of 14/(3)age of 16/(4)age of 18/(5)All
	public static String	DEFAULT_VALUE_PASSWORD          = "0000"; 
	public static int		DEFAULT_VALUE_SEAMLESS_CHANGE   = 0;      // (0)Auto/(1)Full-seg/(2)1-seg
	public static boolean	DEFAULT_VALUE_HANDOVER          = false;  // 
	public static boolean	DEFAULT_VALUE_EWS               = false;  // 
	public static int		DEFAULT_VALUE_BCAS_MODE         = 1;      // (0)Normal/(1)1-seg
	public static String	DEFAULT_VALUE_STORAGE           = "/storage/sdcard0";
	public static boolean	DEFAULT_VALUE_FREQUENCY_VHF     = false;
	public static boolean	DEFAULT_VALUE_FREQUENCY_MID     = false;
	public static boolean	DEFAULT_VALUE_FREQUENCY_SHB     = false;
	public static boolean	DEFAULT_VALUE_SIGNAL_QUALITY    = false;
	public static boolean	DEFAULT_VALUE_FIELD_LOG         = false;
	public static int       DEFAULT_VALUE_TIME_ZONE_TYPE    = 0;        // default auto(0) / manual(1)
	public int              DEFAULT_VALUE_TIME_ZONE_INDEX   = 12;       // default "0"
	public static boolean	DEFAULT_VALUE_TELETEXT          = false;
			
	public class OPTION
	{
		public int		countrycode							= DEFAULT_VALUE_COUNTRY;
		public boolean	record								= false;
		public boolean	book_record							= false;
		public boolean	timeshift							= false;
		public boolean	dual_decode							= false;
		
		public int		audio 								= DEFAULT_VALUE_AUDIO;
		public int		dual_audio							= DEFAULT_VALUE_DUAL_AUDIO;
		public int		caption								= DEFAULT_VALUE_CAPTION;
		public int		super_impose						= DEFAULT_VALUE_SUPER_IMPOSE;
		public int		area_0								= DEFAULT_VALUE_AREA_0;
		public int		area_0_temp;
		public int		area_1								= DEFAULT_VALUE_AREA_1;
		public int		area_1_temp;
		public int		scan								= DEFAULT_VALUE_SCAN;
		public int		scan_manual							= DEFAULT_VALUE_SCAN_MANUAL;
		public int		scan_manual_bandwidth;
		public int		age									= DEFAULT_VALUE_AGE;
		public String	password							= DEFAULT_VALUE_PASSWORD;
		public int		seamless_change						= DEFAULT_VALUE_SEAMLESS_CHANGE;
		public boolean	handover							= DEFAULT_VALUE_HANDOVER;
		public boolean	ews									= DEFAULT_VALUE_EWS;
		public int		bcas_mode							= DEFAULT_VALUE_BCAS_MODE;
		public String	storgae								= DEFAULT_VALUE_STORAGE;
		public boolean	frequency_vhf						= false;
		public boolean	frequency_mid						= false;
		public boolean	frequency_shb						= false;
		public boolean	signal_quality						= false;
		public boolean	field_log							= false;
		public int      time_zone_type						= DEFAULT_VALUE_TIME_ZONE_TYPE;
		public int      time_zone_index						= DEFAULT_VALUE_TIME_ZONE_INDEX;
		public boolean	teletext							= DEFAULT_VALUE_TELETEXT;
		public int		preset_mode							= -1;
	}
	public OPTION cOption = new OPTION();

	public static final int SCAN_INITSCAN = 0;
	public static final int SCAN_RESCAN = 1;
	public static final int SCAN_AREASCAN = 2;
	public static final int SCAN_MANUALSCAN = 4;
	public static final int SCAN_AUTOSEARCH = 5;

	// check playr OFF state
	public boolean isOFF	= false;

	public static final int SCREENMODE_LETTERBOX	= 0;
	public static final int SCREENMODE_PANSCAN	= 1;
	public static final int SCREENMODE_FULL	= 2;

	private int mScreenMode = SCREENMODE_FULL;

	private int mWidth = 0;
	private int mHeight = 0;

	private int mVideoWidth = 0;
	private int mVideoHeight = 0;
	private int mVideoAspectRatio = 0;
	private int mVideoFrameRate = 0;
	private int mVideoProgressive = 0;
	public boolean	bcas_message_keep = false;		//at used seamlesschange = Fullseg (for display black screen, default value is "false")
	public boolean	bcas_message_display = false;

	private SurfaceHolder mSurfaceHolder = null;

	/*	Display	*/
	int	Surface_width;
	int	Surface_height;

	// Setting DB
	private DxbDB_Setting mDB = null;
	private ChannelManager mChannelManager = null;
	private FileManager mFileManager = null;
	//private Channel mChannel = new Channel();

	private final Context mContext;
	private EventHandler mEventHandler;
	private DxbPlayer mDxbPlayer;
	private DxbSubtitle mSubtitleView;
	private DxbSuperimpose mSuperimposeView;
	private DxbPng mPngView;
	private DxbTeletext mRenderTTX;

	/*	Visible Check	*/
	public boolean VISIBLE_LIST_preview_half		= true;

	private ISDBTPlayer mPlayer	= null;

	/*	Scan	*/
	private int[] g_CountryCode;
	public int iManualScan_Frequency	= 0;

	private AutoSearchInfoData mAutoSearchInfo;

	/* -- preset mode -- */
	public int mPresetMode = -1;

	/*	ISDBT Only 	*/
	/************************************************************************************************
	ISDBT INFO SPECIFICATION
	*************************************************************************************************
	MSB 																						  LSB
	|31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10|09|08|07|06|05|04|03|02|01|00|
	|  |  |  |	|			|						|						|						|

	Bit[31] : 13Seg
	Bit[30] : 1Seg
	Bit[29] : JAPAN
	Bit[28] : BRAZIL
	Bit[27:24] : reserved for ISDBT feature
	Bit[23] : record support
	Bit[22] : book support
	Bit[21] : timeshfit support	
	Bit[20] : Dual-decoding support
	Bit[20:16] : reserved for UI
	Bit[15:05] : reserved
	Bit[05:00] : Demod Band Info
	{
		Baseband_None			=0,
		Baseband_TCC351x_CSPI_STS	=1,
		Baseband_reserved		=2,
		Baseband_TCC351x_I2C_STS	=3,
		Baesband_reserved6		=4,
		Baseband_TCC351x_I2C_SPIMS	=5, //reserved
		Baseband_reserved		=6,
		Baseband_reserved		=7,
		Baseband_TCC353X_CSPI_STS	=8,
		Baseband_TCC353X_I2C_STS	=9,
		Baseband_Max			=63,
	}
	*/

	//information about profile 
	public static final int ISDBT_SINFO_PROFILE_A				= 0x80000000; //b'31 - support full seg
	public static final int ISDBT_SINFO_PROFILE_C				= 0x40000000; //b'30 - support 1seg
		
	//information about nation
	public static final int ISDBT_SINFO_JAPAN					= 0x20000000; //b'29 - japan
	public static final int ISDBT_SINFO_BRAZIL					= 0x10000000; //b'28 - brazil
	
	//information about UI ON/OFF
	public static final int ISDBT_SINFO_RECORD					= 0x00400000; //b'23 - record
	public static final int ISDBT_SINFO_BOOKRECORD				= 0x00200000; //b'22 - book-record
	public static final int ISDBT_SINFO_TIMESHIFT				= 0x00100000; //b'21 - time-shift
	public static final int ISDBT_SINFO_DUALDECODE				= 0x00080000; //b'20 - dual-decode

	//information about baseband
	public static final int ISDBT_BASEBAND_TCC351x_CSPI_STS		= 0x00000001;	
	public static final int ISDBT_BASEBAND_TCC351x_I2C_STS		= 0x00000003;
	public static final int ISDBT_BASEBAND_TCC351x_I2C_SPIMS	= 0x00000005; //reserved
	public static final int ISDBT_BASEBAND_TCC353X_CSPI_STS		= 0x00000008;
	public static final int ISDBT_BASEBAND_TCC353X_I2C_STS		= 0x00000009;
	public static final int ISDBT_BASEBAND_MASK					= 0x0000003F;
	
	//compress expression of specification
	public static final int ISDBT_SINFO_MID_JAPAN				= (ISDBT_SINFO_JAPAN|ISDBT_SINFO_DUALDECODE);
	public static final int ISDBT_SINFO_MID_BRAZIL				= (ISDBT_SINFO_BRAZIL|ISDBT_SINFO_RECORD|ISDBT_SINFO_DUALDECODE);
	public static final int ISDBT_SINFO_STB_JAPAN				= (ISDBT_SINFO_JAPAN);
	public static final int ISDBT_SINFO_STB_BRAZIL				= (ISDBT_SINFO_BRAZIL|ISDBT_SINFO_RECORD|ISDBT_SINFO_BOOKRECORD|ISDBT_SINFO_TIMESHIFT);
	
	private static final String SZ_ISDBT_FEATURE = "dxb.isdbt.feature";
	public static int m_isdbt_feature;

	public static final int ISDBT_COUNTRY_JAPAN	= 0;
	public static final int ISDBT_COUNTRY_BRAZIL= 1;

	public static final int ISDBT_SECTION_FSEG	= 0x01;
	public static final int ISDBT_SECTION_1SEG	= 0xC0;

	//dual decode
	public static final int DUAL_DECODE_MAIN	= 0;
	public static final int DUAL_DECODE_SUB		= 1;
	public static final int DUAL_DECODE_MAX		= 2;

	private static Channel mChannel[] = new Channel[DUAL_DECODE_MAX];
	private static int iDefaultChannelIndex = DUAL_DECODE_MAIN;
	private static int iCurrentChannelIndex = DUAL_DECODE_MAIN;
	private static boolean isDual_decoding = false;
		
	//signal quality
	private static SQInfoData mSQInfo;
	
	//BCAS info
	public class BCASCardInfo
	{
		public int		number;
		public String	type;
		public String	ID_card[]	= new String[5];
		public String	ID_group[]	= new String[5];	
	}
	public BCASCardInfo cBCAScardInfo = new BCASCardInfo();
	private static int iSCStatus = SCInfoData.SC_ERR_OK;
	private boolean bSCDisplay = false;
		
	/*********************************************************************************************/
	/*	SI(system information) Data - service, epg	*/
	private ArrayList<DxbChannelData>	tvChannelData = new ArrayList<DxbChannelData>();
	private ArrayList<DxbChannelData>	radioChannelData = new ArrayList<DxbChannelData>();
	
	private ArrayList<String>	tvNameData		= new ArrayList<String>();
	private ArrayList<String>	radioNameData	= new ArrayList<String>();
	
	private ArrayList<DxbEPGData> tvEPGData = null;
	
	private DxbEventData		eventData		= new DxbEventData();
	/*********************************************************************************************/
		
	public DxbPlayer(Context context)
	{
		super(context);
		
		DxbLog(I, "DxbPlayer(context=" + context + ")");
		mContext = context;
		initVideoView();
		initPlayer();
	}

	public DxbPlayer(Context context, AttributeSet attrs)
	{
		this(context, attrs, 0);
		//initVideoView();
		DxbLog(I, "DxbPlayer(context=" + context + ", attrs=" + attrs + ")");
	}

	public DxbPlayer(Context context, AttributeSet attrs, int defStyle)
	{
		super(context, attrs, defStyle);
		
		DxbLog(I, "DxbPlayer(context=" + context + ", attrs=" + attrs + ", defStyle=" + defStyle + ")");
		mContext = context;
		initVideoView();
		initPlayer();
	}

	private void initPlayer()
	{
		DxbLog(I, "initPlayer()");
		
		mDxbPlayer = this;

		g_CountryCode = getResources().getIntArray(R.array.countrycode_select_values);

		Looper looper;
		if ((looper = Looper.myLooper()) != null)
		{
			mEventHandler = new EventHandler(looper);
		}
		else if ((looper = Looper.getMainLooper()) != null)
		{
			mEventHandler = new EventHandler(looper);
		}
		else
		{
			mEventHandler = null;
		}

		if (mChannelManager== null)
		{
			mChannelManager = new ChannelManager(mContext);
		}
		if (mFileManager == null)
		{
			mFileManager = new FileManager(mContext);
		}
		if (mDB == null)
		{
			mDB = new DxbDB_Setting(mContext);
		}
		
		for(int i=0; i<DUAL_DECODE_MAX; i++)
		{
			if(mChannel[i] == null)
			{
				mChannel[i] = new Channel();
			}
		}
	}

	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec)
	{
		DxbLog(I, "onMeasure(Surface : Width = " + MeasureSpec.getSize(widthMeasureSpec) + ", Height = " + MeasureSpec.getSize(heightMeasureSpec) + ")");
		DxbLog(I, "onMeasure(Video   : Width = " + mVideoWidth + ", Height = " + mVideoHeight + ")");

		if (mWidth > 0 && mHeight > 0)
		{
			setMeasuredDimension(mWidth, mHeight);
		}
		else if (mVideoWidth == 0 && mVideoHeight == 0)
		{
			super.onMeasure(widthMeasureSpec, heightMeasureSpec);
		}
		else
		{
			int width, height;
			int layoutWidth = MeasureSpec.getSize(widthMeasureSpec);
			int layoutHeight = MeasureSpec.getSize(heightMeasureSpec);
			float disp_ratio = (layoutWidth / (float)layoutHeight);
			float video_ratio = (mVideoWidth / (float)mVideoHeight);

			switch(mVideoAspectRatio) {
			case 0:
				video_ratio = (float)(16.0/9.0);
				width = 16;
				height = 9;
				break;

			case 1:
				video_ratio = (float)(4.0/3.0);
				width = 4;
				height = 3;
				break;

			default:
				width = mVideoWidth;
				height = mVideoHeight;
				break;
			}

			switch(mScreenMode)
			{
				case SCREENMODE_LETTERBOX:
					if(disp_ratio > video_ratio)
					{
						width = (int)(layoutHeight * width / (float)height);
						layoutWidth = width;
					}
					else if(disp_ratio < video_ratio)
					{
						height = (int)(layoutWidth * height / (float)width);
						layoutHeight = height;
					}
					DxbLog(D, "LetterBox (W = " + layoutWidth + ", H = " + layoutHeight + ")");
					setMeasuredDimension(layoutWidth, layoutHeight);
				break;

				case SCREENMODE_PANSCAN:
					if(disp_ratio > video_ratio)
					{
						height = (int)(layoutWidth * height / (float)width);
						layoutHeight = height;
					}
					else if(disp_ratio < video_ratio)
					{
						width = (int)(layoutHeight * width / (float)height);
						layoutWidth = width;
					}
					DxbLog(D, "PanScan (W = " + layoutWidth + ", H = " + layoutHeight + ")");
					setMeasuredDimension(layoutWidth, layoutHeight);

				break;

				case SCREENMODE_FULL:
					DxbLog(D, "Full (W = " + layoutWidth + ", H = " + layoutHeight + ")");
					super.onMeasure(widthMeasureSpec, heightMeasureSpec);
				break;
			}
		}
	}

	private void initVideoView()
	{
		DxbLog(I, "initVideoView()");
		mVideoWidth = 0;
		mVideoHeight = 0;
		getHolder().addCallback(mSHCallback);
		getHolder().setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
	}

	SurfaceHolder.Callback mSHCallback = new SurfaceHolder.Callback()
	{
		public void surfaceChanged(SurfaceHolder holder, int format, int w, int h)
		{
			DxbLog(I, "surfaceChanged(w=" + w + ", h=" + h + ")");
			
			Surface_width = w;
			Surface_height = h;
			
			if (mPlayer != null)
			{
				mPlayer.useSurface(0);
				if (getServiceType() != 1)
				{
					setChannel();	// error - (Unlock parental) Full --> List
				}
			}
		}

		public void surfaceCreated(SurfaceHolder holder)
		{
			DxbLog(I, "surfaceCreated(holder=" + holder + ")");
			mSurfaceHolder = holder;
			openPlayer();
			setFreqBand();
		}

		public void surfaceDestroyed(SurfaceHolder holder)
		{
			DxbLog(I, "surfaceDestroyed(holder="+holder+")");
			mSurfaceHolder = null;
			//stop();
			//releaseSurface();
		}
	};

	public LOCAL_STATE getLocalPlayState() {
		return eLocalPlayingState;
	}
	public REC_STATE getRecordState() {
		return eRecState;
	}

	public boolean isPlaying()
	{
		return (mSurfaceHolder != null);
	}

	private void attachView(View v, int index)
	{
	/*
		LayoutParams lp = new RelativeLayout.LayoutParams(
				RelativeLayout.LayoutParams.MATCH_PARENT,
				RelativeLayout.LayoutParams.MATCH_PARENT
		);
	*/	
		((RelativeLayout)getParent()).addView(v, index);
	}

	public void openPlayer()
	{
		DxbLog(I, "openPlayer()");
		
		if (mSurfaceHolder == null)
		{
			return;
		}

		if(mPlayer == null)
		{
			mPlayer = new ISDBTPlayer();
			db_open();
			setChannelList(false);
		}
		else
		{
			if (isReqPresetModeChange() == true) {
				stop();
				setPresetMode();
			}
			mPlayer.setDisplay(mSurfaceHolder);
			setSurface();
//			start();
			return;
		}

		//Subtitle
		mSubtitleView = new DxbSubtitle(mContext, mPlayer);
		attachView(mSubtitleView, 1);

		//Superimpose
		mSuperimposeView = new DxbSuperimpose(mContext, mPlayer);
		attachView(mSuperimposeView, 2);

		//PNG
		mPngView = new DxbPng(mContext, mPlayer);
		attachView(mPngView, 3);
		
		mPlayer.setDisplay(mSurfaceHolder);
		mPlayer.setOnPreparedListener(ListenerOnPrepared);
		mPlayer.setOnVideoOutputListener(ListenerOnVideoOutput);
		mPlayer.setOnErrorListener(ListenerOnError);
		mPlayer.setOnVideoDefinitionUpdateListener(ListenerOnVideoDefinitionUpdate);
		mPlayer.setOnDBInfoUpdateListener(mFileManager.ListenerOnDBInfoUpdateListener);
		
		mPlayer.setOnDebugModeListener(ListenerOnDebugMode);
		mPlayer.setOnSearchPercentListener(ListenerOnSearchPercent);
		mPlayer.setOnSearchCompletionListener(ListenerOnSearchCompletion);
		mPlayer.setOnDBInformationUpdateListener(ListenerOnDBInformationUpdate);
		mPlayer.setOnPlayingCompletionListener(ListenerOnPlayingCompletion);
		mPlayer.setOnRecordingCompletionListener(ListenerOnRecordingCompletion);
		mPlayer.setOnFilePlayTimeUpdateListener(ListenerOnTimeUpdate);
		mPlayer.setOnTeletextDataUpdateListener(ListenerOnTeletextDataUpdate);

		mPlayer.setOnEPGUpdateListener(ListenerOnEPGUpdate);
		
		//mPlayer.setWakeMode(Manager_Setting.mContext, PowerManager.SCREEN_BRIGHT_WAKE_LOCK|PowerManager.ON_AFTER_RELEASE);
		//mPlayer.prepare("/data/data/com.telechips.android.isdbt/databases/ISDBT.db");

		preparePlayer();
		setPresetMode();

		mPlayer.setOnChannelUpdateListener(ListenerOnChannelUpdate);

		if(DxbOptions.VISIBLE_ID_preset == true)
		{
			if(cOption.scan < 0)
			{	
				selectArea_0();
			}
		}
		
		if(DxbOptions.VISIBLE_ID_parental_rating == true)
		{
			mPlayer.setParentalRate(cOption.age);
		}
		
		if(DxbOptions.VISIBLE_ID_handover == true)
		{
			mPlayer.setOnHandoverChannelListener(ListenerOnHandoverChannel);
		}
		
		if(DxbOptions.VISIBLE_ID_ews == true)
		{
			mPlayer.setOnEWSListener(ListenerOnEWS);
		}
		
		if(DxbOptions.VISIBLE_ID_bcas_info == true)
		{
			mPlayer.setOnSCErrorListener(ListenerOnSCError);
			mPlayer.setOnSCInfoListener(ListenerOnSCInfo);
		}
		
		mPlayer.setOnAutoSearchInfoListener(ListenerOnAutoSearchInfo);

		setSurface();
		start();
	}

	public void preparePlayer()
	{
		DxbLog(I, "preparePlayer()");

		int ServiceType, TunerType, StrongSignalLevel;
				
		//Set ISDBT_feature	[Default: PVR Off], if you Enabling to PVR, add ISDBT_SINFO_SUPPORT_PVR flag
		//m_isdbt_feature = (ISDBT_SINFO_PROFILE_C|ISDBT_SINFO_MID_JAPAN|ISDBT_BASEBAND_TCC351x_CSPI_STS);	//for 1seg, JAPAN, bandband_TCC351x, CSPI+TSIF
		//m_isdbt_feature = (ISDBT_SINFO_PROFILE_C|ISDBT_SINFO_MID_JAPAN|ISDBT_BASEBAND_TCC351x_I2C_STS);	//for 1seg, JAPAN, bandband_TCC351x, I2C+TSIF
		//m_isdbt_feature = (ISDBT_SINFO_PROFILE_C|ISDBT_SINFO_MID_JAPAN|ISDBT_BASEBAND_TCC351x_I2C_SPIMS);	//for 1seg, JAPAN, bandband_TCC351x, I2C+SPIMS
		
		m_isdbt_feature = (ISDBT_SINFO_PROFILE_A|ISDBT_SINFO_PROFILE_C|ISDBT_SINFO_MID_JAPAN|ISDBT_BASEBAND_TCC353X_CSPI_STS); //for fullseg, JAPAN, CSPI+TSIF
		//m_isdbt_feature = (ISDBT_SINFO_PROFILE_A|ISDBT_SINFO_PROFILE_C|ISDBT_SINFO_MID_BRAZIL|ISDBT_BASEBAND_TCC353X_CSPI_STS); //for fullseg, BRAZIL, CSPI+TSIF
		//m_isdbt_feature = (ISDBT_SINFO_PROFILE_A|ISDBT_SINFO_PROFILE_C|ISDBT_SINFO_STB_JAPAN|ISDBT_BASEBAND_TCC353X_CSPI_STS); //for fullseg, JAPAN, CSPI+TSIF
		//m_isdbt_feature = (ISDBT_SINFO_PROFILE_A|ISDBT_SINFO_PROFILE_C|ISDBT_SINFO_STB_BRAZIL|ISDBT_BASEBAND_TCC353X_CSPI_STS); //for fullseg, BRAZIL, CSPI+TSIF

		//m_isdbt_feature = (ISDBT_SINFO_PROFILE_A|ISDBT_SINFO_PROFILE_C|ISDBT_SINFO_MID_JAPAN|ISDBT_BASEBAND_TCC353X_I2C_STS); //for fullseg, JAPAN, I2C+TSIF
		//m_isdbt_feature = (ISDBT_SINFO_PROFILE_A|ISDBT_SINFO_PROFILE_C|ISDBT_SINFO_MID_BRAZIL|ISDBT_BASEBAND_TCC353X_I2C_STS); //for fullseg, BRAZIL, I2C+TSIF
		//m_isdbt_feature = (ISDBT_SINFO_PROFILE_A|ISDBT_SINFO_PROFILE_C|ISDBT_SINFO_STB_JAPAN|ISDBT_BASEBAND_TCC353X_I2C_STS); //for fullseg, JAPAN, I2C+TSIF
		//m_isdbt_feature = (ISDBT_SINFO_PROFILE_A|ISDBT_SINFO_PROFILE_C|ISDBT_SINFO_STB_BRAZIL|ISDBT_BASEBAND_TCC353X_I2C_STS); //for fullseg, BRAZIL, I2C+TSIF
		
		if(mPlayer != null)
		{
			//mPlayer.setWakeMode(mContext, PowerManager.SCREEN_BRIGHT_WAKE_LOCK|PowerManager.ON_AFTER_RELEASE);
			mPlayer.feature_override = 0;
			mPlayer.prepare(m_isdbt_feature);
			if (mPlayer.feature_override != 0) {
				m_isdbt_feature = mPlayer.feature_override;
			}
		}
		
		if((m_isdbt_feature&ISDBT_SINFO_JAPAN) == ISDBT_SINFO_JAPAN)
		{
			DxbOptions.VISIBLE_ID_preset			= true;
			DxbOptions.VISIBLE_ID_parental_rating	= false;
			DxbOptions.VISIBLE_ID_seamless_change	= true;
			DxbOptions.VISIBLE_ID_handover		= true;
			DxbOptions.VISIBLE_ID_ews			= true;
			
			if((m_isdbt_feature&ISDBT_SINFO_PROFILE_A) == ISDBT_SINFO_PROFILE_A)
			{
				DxbOptions.VISIBLE_ID_bcas_mode		= true;
				DxbOptions.VISIBLE_ID_bcas_info		= true;
				DxbOptions.VISIBLE_ID_frequency_sel	= true;
			}
			else
			{
				DxbOptions.VISIBLE_ID_bcas_mode		= false;
				DxbOptions.VISIBLE_ID_bcas_info		= false;
				DxbOptions.VISIBLE_ID_frequency_sel	= false;
			}
			
			cOption.countrycode = ISDBT_COUNTRY_JAPAN;
		}
		else
		{
			DxbOptions.VISIBLE_ID_preset			= false;
			DxbOptions.VISIBLE_ID_parental_rating	= true;
			DxbOptions.VISIBLE_ID_seamless_change	= false;
			DxbOptions.VISIBLE_ID_handover			= false;
			DxbOptions.VISIBLE_ID_ews				= false;
			DxbOptions.VISIBLE_ID_bcas_mode			= false;
			DxbOptions.VISIBLE_ID_bcas_info			= false;
			DxbOptions.VISIBLE_ID_frequency_sel 	= false;

			cOption.countrycode = ISDBT_COUNTRY_BRAZIL;
		}

		if((m_isdbt_feature&ISDBT_SINFO_RECORD) == ISDBT_SINFO_RECORD)
		{
			cOption.record = true;
		}
		
		if((m_isdbt_feature&ISDBT_SINFO_BOOKRECORD) == ISDBT_SINFO_BOOKRECORD)
		{
			cOption.book_record = true;
		}

		if((m_isdbt_feature&ISDBT_SINFO_TIMESHIFT) == ISDBT_SINFO_TIMESHIFT)
		{
			cOption.timeshift = true;
		}

		if((m_isdbt_feature&ISDBT_SINFO_DUALDECODE) == ISDBT_SINFO_DUALDECODE)
		{
			cOption.dual_decode = true;
		}

		if((m_isdbt_feature&ISDBT_SINFO_PROFILE_A) == ISDBT_SINFO_PROFILE_A)
		{
			ServiceType = SQInfoData.SQINFO_FULL_SEG;
		}
		else
		{
			ServiceType = SQInfoData.SQINFO_ONE_SEG;
		}

		if((m_isdbt_feature&ISDBT_SINFO_JAPAN) == ISDBT_SINFO_JAPAN)
		{
			StrongSignalLevel = 25;		
		}
		else
		{
			StrongSignalLevel = 35;					
		}
		
		switch(m_isdbt_feature&ISDBT_BASEBAND_MASK)
		{
			case ISDBT_BASEBAND_TCC351x_CSPI_STS:	
			case ISDBT_BASEBAND_TCC351x_I2C_STS:
			case ISDBT_BASEBAND_TCC351x_I2C_SPIMS:
				TunerType = SQInfoData.SQINFO_BASEBAND_TCC351x;
			break;

			case ISDBT_BASEBAND_TCC353X_CSPI_STS:
			case ISDBT_BASEBAND_TCC353X_I2C_STS:
				TunerType = SQInfoData.SQINFO_BASEBAND_TCC353x;
			break;

			default:
				TunerType = SQInfoData.SQINFO_BASEBAND_DEFAULT;
			break;
		}

		mSQInfo = new SQInfoData(ServiceType, TunerType, StrongSignalLevel);
	}
	
	/************************************************************************************************************************
	 * 	device access -----> Start
	 ************************************************************************************************************************/
		public boolean isValid()
		{
			return (mPlayer != null);
		}
	
		public void start()
		{
			DxbLog(I, "start()");
			if(mPlayer != null)
			{
				mPlayer.start(g_CountryCode[cOption.countrycode]);
			}
		}

		public void stop()
		{
			DxbLog(I, "stop()");
			
			enableSubtitle(false);
			isVideoPlay_output = false;
			isVideoPlay_update = false;
			
			tvEPGData = null;
			
			if(mPlayer != null)
			{
				for(int i=0; i<DUAL_DECODE_MAX; i++)
				{
					if(mChannel[i] != null)
					{
						mChannel[i].clear();
					}
				}

				mPlayer.stop();
			}
		}

		public void release()
		{
			DxbLog(I, "release()");
			if(mPlayer != null)
			{
				// When mPlayer is processing a release method, some threads can call other methods of mPlayer.
				// For protecting it, mPlayer becomes null before calling the release method.
				ISDBTPlayer player	= mPlayer;
				mPlayer = null;
				player.release();

				mVideoWidth = 0;
				mVideoHeight = 0;
			}
		}

		/*	Surface	*/	
		public void setSurface()
		{
			DxbLog(I, "setSurface()");
			
			if(mPlayer != null)
			{
				mPlayer.setSurface();
			}
		}

		public void releaseSurface()
		{
			DxbLog(I, "releaseSurface()");
			if(mPlayer != null)
			{
				mPlayer.releaseSurface();
			}
		}

		public void useSurface(int arg)
		{
			DxbLog(I, "useSurface(arg = " + arg + ")");
			if(mPlayer != null) {
				mPlayer.useSurface(arg);
			}
		}
			
		public void setLCDUpdate() {
		}

		public void setScreenMode(int screenMode, int l, int t, int r, int b)
		{
			DxbLog(I, "setScreenMode(screenMode = " + screenMode + ")");
			property_set("tcc.dxb.fullscreen", "0");	
			mScreenMode = screenMode;

			if (mScreenMode == SCREENMODE_FULL)
				property_set("tcc.dxb.fullscreen", "1");
		
			setLayout(l, t, r, b);
		}

		public void setFreqBand()
		{
			if(DxbOptions.VISIBLE_ID_frequency_sel) {
				int freq_band = 0;
				if (cOption.frequency_vhf == true)
				{
					freq_band |= 1;
				}
				if (cOption.frequency_mid == true)
				{
					freq_band |= 2;
				}
				if (cOption.frequency_shb == true)
				{
					freq_band |= 4;
				}

				if(mPlayer != null) {
					mPlayer.setFreqBand(freq_band);
				}
			}
		}
		 
		public void makePresetList()
		{
			DxbLog(I, "makePresetList(gInfo_Option.area_code=" + getAreaCode() + ")");
			
			if(mPlayer != null)
			{
				cChannelInfo.iCount_TV = 0;
				cChannelInfo.iCount = 0;

				eState = STATE.SCAN;
				
				if(cOption.scan == 3)
				{
					for(int i=0; i<DUAL_DECODE_MAX; i++)
					{
						if(mChannel[i] != null)
						{
							mChannel[i].clear();
						}
					}

					mPlayer.setPreset(getAreaCode());
				}
			}
		}
			
		public void search() {
			DxbLog(I, "search(g_CountryCode[cOption.countrycode]="+g_CountryCode[cOption.countrycode]);
			if(mPlayer != null) {
				eState = STATE.SCAN;

				cChannelInfo.iCount_TV		= 0;
				cChannelInfo.iCount = 0;
				//mPlayer.setOnNITUpdateListener(DxbScan.ListenerOnNITUpdateListener);
				
				for(int i=0; i<DUAL_DECODE_MAX; i++)
				{
					if(mChannel[i] != null) {
						mChannel[i].clear();
					}
				}
				
				mPlayer.search(cOption.scan, cOption.countrycode, getAreaCode(), 0, 0);
			}
		}
	
		public void manualScan(int KHz_frequency, int Mhz_bandwidth)
		{
			DxbLog(I, "manualScan(KHz_frequency = " + KHz_frequency + " Mhz_bandwidth = " + Mhz_bandwidth + ")");
			
			if(mPlayer != null)
			{
				int channel_num = 0;
				
				eState = STATE.SCAN;

				cChannelInfo.iCount_TV		= 0;
				cChannelInfo.iCount = 0;

				for(int i=0; i<DUAL_DECODE_MAX; i++)
				{
					if(mChannel[i] != null) {
						mChannel[i].clear();
					}
				}

				if(cOption.countrycode == ISDBT_COUNTRY_JAPAN) {
					channel_num = (KHz_frequency - 473143) / (Mhz_bandwidth * 1000);
				} else {
					if(KHz_frequency < 473143) {
						channel_num = (KHz_frequency - 177143) / (Mhz_bandwidth * 1000);
					} else {
						channel_num = ((KHz_frequency - 473143) / (Mhz_bandwidth * 1000)) + 7;
					}
				}
				
				mPlayer.search(SCAN_MANUALSCAN, cOption.countrycode, getAreaCode(), channel_num, 1);
			}
		}

		//Direction 0=down
		//              1=up
		public void autosearch (int Direction)
					{
			DxbChannelData ch = null;
			int position,ch_no;

			eState = STATE.SCAN;
			if (mPlayer!= null) {
				if(mDB != null) position = mDB.getPosition(0);
				else position = 0;

				if (position != cChannelInfo.iCurrent_TV) {
					DxbLog(D, "autosearch - current ch is different " + position + "and" + cChannelInfo.iCurrent_TV);
				}
				ch = getCurChannelData();
				if (ch != null) ch_no = ch.iChannel_number;
				else ch_no = -1;
				
				DxbLog(D, "autosearch : cur_ch="+ch_no+ " direction="+Direction);
				mPlayer.search(SCAN_AUTOSEARCH, cOption.countrycode, getAreaCode(), ch_no, Direction);
			}
		}

		public int getCurrentTime()
		{
			DateTimeData date_time = new DateTimeData();
			date_time.iMJD	= 0;
			int currTime	= 0;
			if(mPlayer != null)
			{
				mPlayer.getCurrentDateTime(date_time);
				if(date_time.iMJD != 0)
				{
					currTime	= date_time.iHour*60*60 + date_time.iMinute*60 + date_time.iSecond;
					//DxbLog(D, date_time.iHour + ":" + date_time.iMinute + ":" + date_time.iSecond);
				}
			}
			
			if(date_time.iMJD == 0)
			{
				Calendar calTime = Calendar.getInstance();
				currTime	= calTime.get(Calendar.HOUR_OF_DAY)*60*60 + calTime.get(Calendar.MINUTE)*60 + calTime.get(Calendar.SECOND);
			}
			
			return currTime;
		}

		public DateTimeData getCurrentDateTime()
		{
			DateTimeData date_time	= null;
			if(mPlayer != null)
			{
				date_time = new DateTimeData();
				mPlayer.getCurrentDateTime(date_time);
			}
			return date_time;
		}
			
		public void setChannelCancel() {
		}
			
		/*	Record & Capture	*/
		public int setCapture(String name) {
			int	return_state	= 1;
			if(mPlayer != null) {
				return_state = mPlayer.setCapture(name);
			}
			DxbLog(I, "setCapture(name="+name+"(state="+return_state+")");
			return return_state;
		}

		public boolean setRecord(String name) {
			DxbLog(I, "setRecord()");
			if(mPlayer != null) {

				eRecState = REC_STATE.RECORDING;
				mPlayer.setRecord(name);

				cChannelInfo.iRecordPosition = cChannelInfo.iPosition;
				cChannelInfo.iRecordTab = cChannelInfo.iTab;

				//eLocalPlayingState = LOCAL_STATE.PLAYING;
				//mPlayer.setPlay(name);
				return true;
			}
			return false;
		}

		public boolean setPlay() {
			if (cChannelInfo.iRecordPosition != cChannelInfo.iPosition || cChannelInfo.iRecordTab != cChannelInfo.iTab) {
				return false;
			}
			if (eLocalPlayingState != LOCAL_STATE.PLAYING) {
				eLocalPlayingState = LOCAL_STATE.PLAYING;
				
				Display display = ((WindowManager)mContext.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();

				Point view_size = new Point(0,0);
				Point real_size = new Point(0,0);
				
				display.getSize(view_size);
				display.getRealSize(real_size);

				mPlayer.setPlay("", real_size.x, real_size.y, view_size.x, view_size.y);
				
				return true;
			}
			return false;
		}

		public void setRecStop() {
			DxbLog(I, "setRecStop()");
			if(mPlayer != null) {	
				mPlayer.setRecStop();
				eRecState = REC_STATE.STOP;
			}
		}

		public void setLocalPlayStop() {
			DxbLog(I, "setLocalPlayStop()");
			if(mPlayer != null) {		
				mPlayer.setPlayStop();
				eLocalPlayingState = LOCAL_STATE.STOP;
			}
		}

		public void setSeekTo(int seekTime) {
			DxbLog(I, "setSeekTo()");
			if(mPlayer != null) {
				mPlayer.setSeekTo(seekTime);
			}
		}

		public int setPlaySpeed(int iSpeed) {
			DxbLog(I, "setPlaySpeed()");
			if(mPlayer != null) {
				return mPlayer.setPlaySpeed(iSpeed);
			}
			return 0;
		}

		public void setPlayPause(int serviceType, boolean bPlayPause)
		{
			DxbLog(I, "setFilePlayPause(serviceType=" + serviceType + ", bPlayPause=" + bPlayPause + ")");
			
			if(mPlayer != null)
			{
				if(serviceType == 1)
				{
					if (bPlayPause)
					{
						eLocalPlayingState = LOCAL_STATE.PLAYING;
					}
					else
					{
						eLocalPlayingState = LOCAL_STATE.PAUSE;
					}
				}
				
				mPlayer.setPlayPause(bPlayPause);
			}
		}

		public int getDuration() {
			//DxbLog(I, "getDuration()");
			if(mPlayer != null) {
				return mPlayer.getDuration();
			}
			return 0;
		}

		public int getCurrentPosition() {
			//DxbLog(I, "getCurrentPosition()");
			if (mPlayer != null) {
				return mPlayer.getCurrentPosition();
			}
			return 0;
		}

		public int getCurrentReadPosition() {
			//DxbLog(I, "getCurrentReadPosition()");
			if (mPlayer != null) {
				return mPlayer.getCurrentReadPosition();
			}
			return 0;
		}

		public int getCurrentWritePosition() {
			//DxbLog(I, "getCurrentWritePosition()");
			if(mPlayer != null) {
				return mPlayer.getCurrentWritePosition();
			}
			return 0;
		}

		public String getDiskFSType() {
			//DxbLog(I, "getDiskFSType()");
			String fsType = null;
			if(mPlayer != null) {
				fsType = mPlayer.getDiskFSType(); 
				if(fsType.equalsIgnoreCase("UFSD") == true) {
					return "NTFS";
				}
			}
			return fsType;
		}

		public int isFilePlay()
		{
			//DxbLog(I, "isFilePlay(mChannel.filePlay=" + mChannel.filePlay + ")");
			if (mChannel[iDefaultChannelIndex] == null)
			{
				return 0;
			}

			return mChannel[iDefaultChannelIndex].filePlay;
		}

		public void setLayout(int l, int t, int r, int b)
		{
			DxbLog(I, "setLayout(" + l + ", " + t + ", " + r + ", " + b + ")");
			mWidth = r - l;
			mHeight = b - t;

			RelativeLayout.LayoutParams lp = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.MATCH_PARENT);
			lp.setMargins(l, t, 0, 0);
			
			if (mWidth == 0 && mHeight == 0)
			{
				lp.addRule(RelativeLayout.CENTER_IN_PARENT);
			}
			setLayoutParams(lp);
		}

		public void playTeletext(int onoff)
		{
			if (mPlayer != null)
			{
				if (mRenderTTX != null)
				{
					mRenderTTX.startTeletext(false);
				}
				
				mPlayer.playTeletext(onoff);
			}
		}

		public void setTeletext_UpdatePage(int pageNumber)
		{
			DxbLog(I, "setTeletext_UpdatePage(" + pageNumber + ")");
			
			if (mPlayer != null)
			{
				mPlayer.setTeletext_UpdatePage(pageNumber);
			}
		}

		public void searchCancel()
		{
			DxbLog(I, "searchCancel()");
			
			if (mPlayer != null)
			{
				mPlayer.searchCancel();
			}
		}

		public void onCtrlLastFrameFunction(int enable)
		{
			//Log.e("[TEST]", "onCtrlLastFrameFunction() = " + enable);
			if (mPlayer != null)
			{
				if(enable == 1)
					mPlayer.CtrlLastFrame_Open();
				else
					mPlayer.CtrlLastFrame_Close();
			}
		}

	/************************************************************************************************************************
	 * 	device access -----> End
	 ************************************************************************************************************************/

		
	/************************************************************************************************************************
	 * 	DB control -----> Start
	 ************************************************************************************************************************/
		/*	Service	*/
		private int	iIndex_channelNumber, iIndex_countryCode, iIndex_frequency, iIndex_serviceType;
		private int	iIndex_audioPID, iIndex_videoPID, iIndex_teletextPID, iIndex_serviceID;
		private int	iIndex_serviceName;
		private int	iIndex_freeCAmode, iIndex_scrambling, iIndex_bookmark, iIndex_logicalChannel=-1;
	    
		public void db_open()
		{
			DxbLog(I, "db_open()");
			
			if(mChannelManager != null)
			{
				mChannelManager.open();
			}
			
			if(mFileManager != null)
			{
				mFileManager.open();
			}
			
			if(mDB != null)
			{
				mDB.open();
				cChannelInfo.iCurrent_TV 	= mDB.getPosition(0);
				
				//cOption.time_zone_index	= mDB.getTimeZone();
				cOption.password			= mDB.getPassword();
				cOption.age					= mDB.getAge();
				
				cOption.scan				= mDB.getScan();
				cOption.area_0				= mDB.getArea_0();
				cOption.area_1				= mDB.getArea_1();
			}
			else
			{
				DxbLog(D, "not open DB!!!");
			}
			cChannelInfo.iCount = getChannel_Count(cChannelInfo.iTab);
		}
		
		public void onDestroy()
		{
			DxbLog(I, "onDestroy()");
			
			db_close();
		}

		private void db_close()
		{
			DxbLog(I, "db_close()");
			
			if (mChannelManager != null)
			{
				mChannelManager.close();
				mChannelManager = null;
			}
			
			if (mFileManager != null)
			{
				mFileManager.close();
				mFileManager = null;
			}
			
			if (mDB != null)
			{
				mDB.close();
				mDB = null;
			}
		}

		public void setColumnIndex_Service(Cursor cursor)
		{
			if ((cursor != null) && (iIndex_logicalChannel == -1))
			{
				iIndex_channelNumber	= cursor.getColumnIndexOrThrow(FileManager.KEY_CHANNEL_NUMBER);
				iIndex_countryCode		= cursor.getColumnIndexOrThrow(FileManager.KEY_COUNTRY_CODE);
				iIndex_frequency		= cursor.getColumnIndexOrThrow(FileManager.KEY_FREQUENCY); 
				iIndex_serviceType		= cursor.getColumnIndexOrThrow(FileManager.KEY_SERVICE_TYPE);
				
				iIndex_audioPID			= cursor.getColumnIndexOrThrow(FileManager.KEY_AUDIO_PID);
				iIndex_videoPID			= cursor.getColumnIndexOrThrow(FileManager.KEY_VIDEO_PID);
				iIndex_teletextPID		= cursor.getColumnIndexOrThrow(FileManager.KEY_TELETEXT_PID);
				iIndex_serviceID		= cursor.getColumnIndexOrThrow(FileManager.KEY_SERVICE_ID);
				
				iIndex_serviceName		= cursor.getColumnIndexOrThrow(FileManager.KEY_CHANNEL_NAME);
				
				iIndex_freeCAmode		= cursor.getColumnIndexOrThrow(FileManager.KEY_FREE_CA_MODE);
				iIndex_scrambling		= cursor.getColumnIndexOrThrow(FileManager.KEY_SCRAMBLING);
				iIndex_bookmark			= cursor.getColumnIndexOrThrow(FileManager.KEY_BOOKMARK);
				iIndex_logicalChannel	= cursor.getColumnIndexOrThrow(FileManager.KEY_LOGICALCHANNEL);
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
			if (getServiceType() == 1)
			{
				return mChannel[iDefaultChannelIndex].channelName;
			}
			else
			{
				return mChannel[iCurrentChannelIndex].channelName;
			}
		}

		public String getServiceName_addHyphen()
		{
			if (getServiceType() == 1)
			{
				return (mChannel[iDefaultChannelIndex].channelName + "-");
			}
			else
			{
				return (mChannel[iCurrentChannelIndex].channelName + "-");
			}
		}

		public String getServiceName_addLogical()
		{
			if (getServiceType() == 1)
			{
				return mChannel[iDefaultChannelIndex].channelName;
			}
			else if(mChannel[iDefaultChannelIndex].logicalChannel == 0)
			{
				return mChannel[iCurrentChannelIndex].channelName;
			}
			else
			{
				return ("CH " + String.format("%03d", mChannel[iCurrentChannelIndex].logicalChannel) + "  -  " + mChannel[iCurrentChannelIndex].channelName);
			}
		}

		public String getServiceName_addThreeDigitNumber()
		{
			if (getServiceType() == 1)
			{
				return mChannel[iDefaultChannelIndex].channelName;
			}
			else if(mChannel[iCurrentChannelIndex].threeDigitNumber <= 0)
			{
				return mChannel[iCurrentChannelIndex].channelName;
			}
			else
			{
				return String.format("[%03d] %s", mChannel[iCurrentChannelIndex].threeDigitNumber, mChannel[iCurrentChannelIndex].channelName);
			}
		}
		
		public Cursor getFileList()
		{
			DxbLog(I, "getFileList() --> cOption.storage = " + DxbStorage.getPath_Device());
			
			if(mFileManager != null)
			{
				cChannelInfo.cursorFileList	= mFileManager.getAllFiles(DxbStorage.getPath_Device());
				cChannelInfo.iCount	= cChannelInfo.cursorFileList.getCount();
				
				setColumnIndex_Service(cChannelInfo.cursorFileList);	
			}
			else
			{
				cChannelInfo.iCount	= 0;
			}
			
			return cChannelInfo.cursorFileList;
		}
	    
	    public void setChannelList(boolean scan)
	    {
	    	DxbLog(I, "setChannelList()");
	    	
	    	if(mChannelManager == null)
	    	{
	    		return;
	    	}
	    	
	    	ArrayList<DxbChannelData>	current_List	= null;
	    	ArrayList<String>			current_Name	= null;
	    	int	current_Count	= 0;
	    	
	    	current_List = mChannelManager.getAllChannels();
	    	if(current_List != null)
	    	{
	    		current_Count = current_List.size();
	    		DxbLog(D, "current_Count = " + current_Count);
	    	
		    	cChannelInfo.iCount_TV = current_Count;
	
		    	tvChannelData = current_List;
				current_Name = tvNameData;
				
				eventData.iCountTV = current_Count;
		    	
	    		current_Name.clear();
		    		
		    	for(int j=0 ; j<current_Count ; j++)
		    	{
					current_Name.add(new String(current_List.get(j).sChannel_name));
		    	}
	    	}
	    	else
	    	{
		    	tvChannelData.clear();	    		
				cChannelInfo.iCount_TV = 0;
				cChannelInfo.iCount = 0;
	    	}
	    	
	    	if(cChannelInfo.iCount_TV > 0)
	    	{
	    		cChannelInfo.iTab	= 0;
	    		cChannelInfo.iCount	= cChannelInfo.iCount_TV;
	    	}

	    	if(cChannelInfo.iCount_TV <= cChannelInfo.iCurrent_TV)
	    	{
	    		cChannelInfo.iCurrent_TV	= 0;
	    	}

			if(scan == true)
			{
		    	if(iManualScan_Frequency == -1)
		    	{
			    	cChannelInfo.iCurrent_TV	= 0;
	    		}
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
	    	if(iTab == 0)
	    	{
	    		return tvChannelData;
	    	}
	    	
	    	return radioChannelData;
	    }
	    
	    public ArrayList<String> getNameList(int iTab)
	    {
	    	if(iTab == 0)
	    	{
	    		return tvNameData;
	    	}
	    	
	    	return radioNameData;
	    }
	    
	    public void setCurrent_ChannelData(int iTab, int iPosition)
	    {
	    	DxbLog(I, "setCurrent_ChannelData(" + iTab + ", " + iPosition + ")");
	    	
	    	if(cChannelInfo.iTab != iTab)
	    	{
	    		cChannelInfo.iTab	= iTab;
	    	}
	    	
	    	if(cChannelInfo.iPosition != iPosition)
	    	{
	    		cChannelInfo.iPosition	= iPosition;
	    	}
	    	
	    	setCurrent_ChannelData();
	    }
	    
	    public void setFileList_cursor(int position)
	    {
	    	DxbLog(I, "setFileList_cursor(position=" + position + ")");
	    	
	    	cChannelInfo.cursorFileList.moveToPosition(position);
	    }
	    
	    public void setCurrent_ChannelData()
	    {
	    	DxbLog(I, "setCurrent_ChannelData()");
	    	
			if(cChannelInfo.iTab == 0)
			{
				DxbChannelData	p	= tvChannelData.get(cChannelInfo.iPosition);
				
				mChannel[iDefaultChannelIndex].ID					= p.iID;
				
				mChannel[iDefaultChannelIndex].channelNumber		= p.iChannel_number;
				mChannel[iDefaultChannelIndex].countryCode			= p.iCountry_code;
				mChannel[iDefaultChannelIndex].frequency			= p.iFrequency;
				mChannel[iDefaultChannelIndex].serviceType			= p.iService_type;

				mChannel[iDefaultChannelIndex].audioPID				= p.iAudio_PID;
				mChannel[iDefaultChannelIndex].videoPID				= p.iVideo_PID;
				mChannel[iDefaultChannelIndex].teletextPID			= p.iTeletext_PID;

				mChannel[iDefaultChannelIndex].serviceID			= p.iService_ID;
				mChannel[iDefaultChannelIndex].channelName			= p.sChannel_name;
				
				mChannel[iDefaultChannelIndex].iBookmark			= p.iBookmark;
				mChannel[iDefaultChannelIndex].logicalChannel 		= p.iLogical_channel;
			}
			else if(cChannelInfo.iTab == 1)
			{
				mChannel[iDefaultChannelIndex].ID				= cChannelInfo.cursorFileList.getInt(getColumnIndex_ID());

				mChannel[iDefaultChannelIndex].channelNumber	= cChannelInfo.cursorFileList.getInt(getColumnIndex_ChannelNumber());
				mChannel[iDefaultChannelIndex].countryCode		= cChannelInfo.cursorFileList.getInt(getColumnIndex_CountryCode());
				mChannel[iDefaultChannelIndex].frequency		= cChannelInfo.cursorFileList.getInt(getColumnIndex_Frequency());
				mChannel[iDefaultChannelIndex].serviceType		= cChannelInfo.cursorFileList.getInt(getColumnIndex_ServiceType());

				mChannel[iDefaultChannelIndex].videoPID			= cChannelInfo.cursorFileList.getInt(getColumnIndex_VideoPID());
				mChannel[iDefaultChannelIndex].teletextPID		= cChannelInfo.cursorFileList.getInt(getColumnIndex_TeletextPID());

				mChannel[iDefaultChannelIndex].serviceID		= cChannelInfo.cursorFileList.getInt(getColumnIndex_ServiceID());
				mChannel[iDefaultChannelIndex].channelName		= cChannelInfo.cursorFileList.getString(getColumnIndex_ServiceName());
				mChannel[iDefaultChannelIndex].logicalChannel	= cChannelInfo.cursorFileList.getInt(getColumnIndex_LogicalChannel());

				return;
			}
			else
			{
				mChannel[iDefaultChannelIndex].ID				= -1;
				
				mChannel[iDefaultChannelIndex].channelNumber	= 0;
				mChannel[iDefaultChannelIndex].countryCode		= 0;
				mChannel[iDefaultChannelIndex].frequency		= 0;
				mChannel[iDefaultChannelIndex].serviceType		= 0;

				mChannel[iDefaultChannelIndex].videoPID			= 0;
				mChannel[iDefaultChannelIndex].teletextPID		= 0;

				mChannel[iDefaultChannelIndex].serviceID		= 0;
				mChannel[iDefaultChannelIndex].channelName		= "";
				mChannel[iDefaultChannelIndex].logicalChannel 	= 0;
				
				return;
			}
	    }
	    
		public int getCurServiceID()
		{
			return	mChannel[iDefaultChannelIndex].serviceID;
		}
		
		public int getCurCountryCode()
		{
			return mChannel[iDefaultChannelIndex].countryCode;
		}
		
		public int getCurFrequency()
		{
			return mChannel[iDefaultChannelIndex].frequency;
		}
		
		public int getCurChannelNumber()
		{
			return mChannel[iDefaultChannelIndex].channelNumber;
		}
		
		public int getCurChannelType()
		{
			return mChannel[iDefaultChannelIndex].serviceType;
		}
		
		public String getCurChannelName()
		{
			return mChannel[iDefaultChannelIndex].channelName;
		}
		
		public Channel getCurChannel()
		{
			return mChannel[iCurrentChannelIndex];
		}

	    public DxbChannelData getCurChannelData()
	    {
	    	DxbLog(I, "getCurChannelData()");
	    	
			ArrayList<DxbChannelData>	playChannelData;
			if(cChannelInfo.iTab == 0)
			{
				playChannelData = tvChannelData;
			}
			else
			{
				return null;
			}
			
			if (playChannelData.size() == 0)	return null;
			return playChannelData.get(cChannelInfo.iPosition);
	    }
    
		public void toggleBookmark(int position)
		{
			DxbLog(I, "toggleBookmark(position=" + position + ")");
	    	
			ArrayList<DxbChannelData>	playChannelData	= null;
			if(cChannelInfo.iTab == 0)
			{
				playChannelData = tvChannelData;
			}
			
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
			ArrayList<DxbChannelData>	playChannelData	= null;
			if(cChannelInfo.iTab == 0)
			{
				playChannelData = tvChannelData;
			}

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
			
			ArrayList<DxbChannelData>	playChannelData	= null;
			if(cChannelInfo.iTab == 0)
			{
				playChannelData = tvChannelData;
			}
			
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
								setChannel(i);
								return;
							}
							
							index++;
						}
					}
				}
				
			}
		}
			
		public String getCurrent_ServiceName()
		{
			DxbLog(I, "getCurrent_ServiceName()");
			return ChannelManager.KEY_CHANNEL_NAME;
		}
		
		public String getCurrent_ServiceType()
		{
			DxbLog(I, "getCurrent_ServiceType()");
			return ChannelManager.KEY_SERVICE_TYPE;
		}

		public int getChannel_Count(int iTab)
		{
			if(iTab == 0)
			{
				return tvChannelData.size();
			}
			else if(iTab == 1)
			{
				return radioChannelData.size();
			}
			
			return 0;
		}
		
		public int getChannel_Count_DB(int iTab)
		{
			int	iCount	= 0;
			if(mChannelManager != null && iTab < 2)
			{
				ArrayList<DxbChannelData> current_List = mChannelManager.getAllChannels();
				if(current_List != null)
				{
					iCount	= current_List.size();
				}
			}
			return iCount;
		}

		public Cursor getAudio_LanguageCode()
		{
			DxbLog(I, "getAudio_LanguageCode()");

			if(		(mChannelManager != null)
				&&	(mChannel[iDefaultChannelIndex].filePlay == 0)
			)
			{
				return mChannelManager.getAudio_LanguageCode(mChannel[iDefaultChannelIndex].serviceID, mChannel[iDefaultChannelIndex].channelNumber);
			}
			else if (mFileManager != null && mChannel[iDefaultChannelIndex].filePlay != 0)
			{
				return mFileManager.getAudio_LanguageCode(mChannel[iDefaultChannelIndex].serviceID, mChannel[iDefaultChannelIndex].channelNumber);
			}
			
			return null;
		}
			
		public Cursor getSubtitle_LanguageCode()
		{
			DxbLog(I, "getSubtitle_LanguageCode()");
			
			if(		(mChannelManager != null)
				&&	(mChannel[iDefaultChannelIndex].filePlay == 0)
			)
			{
				return mChannelManager.getSubtitle_LanguageCode(mChannel[iDefaultChannelIndex].serviceID, mChannel[iDefaultChannelIndex].channelNumber);
			}
			else if (mFileManager != null && mChannel[iDefaultChannelIndex].filePlay != 0)
			{
				return mFileManager.getSubtitle_LanguageCode(mChannel[iDefaultChannelIndex].serviceID, mChannel[iDefaultChannelIndex].channelNumber);
			}
			
			return null;
		}
		
		public Cursor getTeletext_Descriptor(int iType)
		{
			DxbLog(I, "getTeletext_Descriptor()");

			/*	teletext type
			 * 	0x00	- reserved for future use
			 * 	0x01	- initial teletext page
			 * 	0x02	- teletext subtitle page
			 * 	0x03	- additional information page
			 * 	0x04	- program schedule page
			 * 	0x05~0x1F	- reserved for future use
			 */

			if(ePLAYER != PLAYER.ISDBT)
			{
				if (mChannelManager != null && mChannel[iDefaultChannelIndex].filePlay == 0)
				{
					return mChannelManager.getTTX_Descriptor(mChannel[iDefaultChannelIndex].serviceID, mChannel[iDefaultChannelIndex].channelNumber, mChannel[iDefaultChannelIndex].countryCode, iType);
				}
				else if (mFileManager != null && mChannel[iDefaultChannelIndex].filePlay != 0)
				{
					return mFileManager.getTTX_Descriptor(mChannel[iDefaultChannelIndex].serviceID, mChannel[iDefaultChannelIndex].channelNumber, mChannel[iDefaultChannelIndex].countryCode, iType);
				}
			}
			
			return null;
		}

		private final Rect mWindowInsets = new Rect();

		@Override
		protected boolean fitSystemWindows(Rect insets) {
			// We don't set the paddings of this View, otherwise,
			// the content will get cropped outside window
			mWindowInsets.set(insets);
			return true;
		}

		public int getStatusbarHeight() {
			DxbLog(I, "getStatusbarHeight : " + mWindowInsets);
			return mWindowInsets.height();
		}

	/************************************************************************************************************************
	 * 	DB control -----> End
	 ************************************************************************************************************************/

			
	/************************************************************************************************************************
	 * 	Signal -----> Start
	 ************************************************************************************************************************/
	public int getSignalStrength() {
		//DxbLog(I, "getSignalStrength()");
		int	iSignal	= 4;
		if (mPlayer != null) {
			mPlayer.getSignalStrength(mSQInfo);
			iSignal = mSQInfo.getSignalLevel();
		}
		//DxbLog(D, "mPlayer.getSignalStrength() = " + iSignal);
	
		if (iSignal > 4) {
			return 4;
		} else {
			return iSignal;
		}
	}
	
	public String getSignalMessage() {
		//DxbLog(I, "getSignalMessage()");
		String msg = "";

		if (mPlayer != null)
		{
			msg = mSQInfo.getSignalQuality();
		}
		
		return msg;
	}

	/************************************************************************************************************************
	 * 	Signal -----> End
	 ************************************************************************************************************************/
	
	/************************************************************************************************************************
	 *	(DVBT Only) teletext, RadioImage, EPG -----> Start
	 ************************************************************************************************************************/
	public boolean updateBookmark(long rowId, int iBookmark)
	{
		if (mChannelManager != null)
		{
			return mChannelManager.updateBookmark(rowId, iBookmark);
		}

		return false;
	}

	public boolean isRadio() {
		DxbLog(I, "isRadio()");
		if(mPlayer != null) {
			DxbLog(D, "mChannel.videoPID=" + mChannel[iDefaultChannelIndex].videoPID);
			if(mChannel[iDefaultChannelIndex].videoPID != 0xFFFF) {
				return false;
			}
		}
			return true;
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

	public boolean isTV(int serviceType) {
		switch(serviceType) {
		case ChannelManager.SERVICE_TYPE_DTV:
		case ChannelManager.SERVICE_TYPE_MHDTV:
		case ChannelManager.SERVICE_TYPE_ASDTV:
		case ChannelManager.SERVICE_TYPE_AHDTV:
			return true;
		}
		return false;
	}
		
	public boolean isRadio(int serviceType) {
		switch(serviceType) {
		case ChannelManager.SERVICE_TYPE_DRADIO:
		case ChannelManager.SERVICE_TYPE_FMRADIO:
		case ChannelManager.SERVICE_TYPE_ADRADIO:
			return true;
		}
		return false;
	}

	public void requestEPGUpdate(int arg) {
		if (mPlayer != null) {
			mPlayer.requestEPGUpdate(arg);
		}
	}

	public void setEPG_PF()
	{
		DxbLog(I, "setEPG_PF()");
	
		if(openEPGDB())
		{
			requestEPGUpdate(0);

			tvEPGData = getEPG_PF();
		}
	}

	public boolean isValidate_Teletext()
	{
		DxbLog(I, "mChannel.teletextPID=" + mChannel[iDefaultChannelIndex].teletextPID);
		
		if(mChannel[iDefaultChannelIndex].teletextPID == 0xFFFFFFFF)
	{
			return false;
		}

		return true;
	}
			
	public void stopTeletext() {
		if (mRenderTTX != null) {
			mRenderTTX.stopTeletext();
		}
	}

	public void enableSubtitle(boolean enable)
	{
		//DxbLog(I, "enableSubtitle(" + enable + ")");
		
		if(mPlayer != null)
		{
			if(mSubtitleView != null)
			{
				if(cOption.caption > 0){
					mSubtitleView.enableSubtitle(enable);
				}else{
					mSubtitleView.enableSubtitle(false);
				}					
			}

			if(mSuperimposeView != null)
			{
				if(cOption.super_impose > 0){
					mSuperimposeView.enableSubtitle(enable);	
				}else{
					mSuperimposeView.enableSubtitle(false);
				}					
			}

			if(mPngView != null)
			{
				if(cOption.super_impose > 0){
					mPngView.enableSubtitle(enable);	
				}else{
					mPngView.enableSubtitle(false);
				}				
			}
			
 		}
	}
	
	public void playSubtitle(int iIndex)
	{
		DxbLog(I, "playSubtitle(" + iIndex + ")");
	
		if(mPlayer != null)
		{
			if(iIndex == 1)
			{
				if(cOption.caption > 0){
					mSubtitleView.startSubtitle(true);	
				} else {
					mSubtitleView.startSubtitle(false);					
				}
				
				if(cOption.super_impose > 0){
					mSuperimposeView.startSubtitle(true);	
					mPngView.startSubtitle(true);
				}else{
					mSuperimposeView.startSubtitle(false);
					mPngView.startSubtitle(false);
				}
								
			} else
			{
				mSubtitleView.startSubtitle(false);
				mSuperimposeView.startSubtitle(false);
				mPngView.startSubtitle(false);
			}
		}
	}

	private void stopSubtitle()
	{
		DxbLog(I, "stopSubtitle()");
		
		mSubtitleView.stopSubtitle();
		mSuperimposeView.stopSubtitle();
		mPngView.stopSubtitle();
		
		if (mRenderTTX != null)
		{
			mRenderTTX.stopTeletext();
		}
		
		if(mPlayer != null)
		{
			mPlayer.playTeletext(0);	// 0-off, 1-on
		}
	}
	
	public void setTimeZone(int iMode, int iIndex)
	{
		DxbLog(I, "setTimeZone()");
		cOption.time_zone_type		= iMode;
		
		if(iMode == 0)
		{
			if(mPlayer != null)
			{
				mPlayer.setUserLocalTimeOffset(true, iIndex);
			}
		}
		else if(iMode == 1)
		{
			DxbLog(D, "gInfo_Option.time_zone_index = " + cOption.time_zone_index);
			cOption.time_zone_index	= iIndex;
			mDB.putTimeZone(cOption.time_zone_index);

			CharSequence[] csValue	= getResources().getTextArray(R.array.time_zone_manual_select_values);
			int	iValue	= Integer.parseInt((String) csValue[cOption.time_zone_index]);
			DxbLog(D, "iValue = " + iValue);

			if(mPlayer != null)
			{
				mPlayer.setUserLocalTimeOffset(false, iValue);
			}
		}
	}

	/************************************************************************************************************************
	 *	(ISDBT Only) Preset, EWS, BCAS, EPG -----> End
	 ************************************************************************************************************************/
	public boolean isRecord()
	{
		return cOption.record;		
	}
	
	public boolean isBookRecord()
	{
		return cOption.book_record;
	}
	
	public boolean isTimeShift()
	{
		return cOption.timeshift;		
	}
	
	public void setAudio(int iIndex)
	{
		DxbLog(I, "setAudio(" + iIndex + ")");

		cOption.audio  = iIndex;
		
		if(mPlayer != null)
		{
			mPlayer.setAudio(iIndex);
		}
	}

	public void setAudioDualMono(int iIndex)
	{
		//config. dual-monoral AAC
		DxbLog(I, "setAudioDualMono(" + iIndex + ")");

		cOption.dual_audio = iIndex;
		
		if(mPlayer != null)
		{
			mPlayer.setAudioDualMono(iIndex);
		}
	}

	public void setCaption(int state)
	{
		DxbLog(I, "setCaption(" + state + ")");
		
		if(cOption.caption != state)
		{
			cOption.caption	= state;
		}
		
		if((mPlayer != null) && (isVideoPlay_output == true))
		{			
			if(cOption.caption > 0) {	
				mSubtitleView.enableSubtitle(true);
			} else {
				mSubtitleView.enableSubtitle(false);	
			}
			mPlayer.setSubtitle(state);
		}
	}
	
	public void setSuperImpose(int state)
	{
		DxbLog(I, "setSuperImpose(" + state + ")");
		
		if(cOption.super_impose != state)
		{
			cOption.super_impose = state;
		}
		
		if((mPlayer != null) && (isVideoPlay_output == true))
		{
			if(cOption.super_impose > 0) {	
				mSuperimposeView.enableSubtitle(true);
				mPngView.enableSubtitle(true);
			} else {
				mSuperimposeView.enableSubtitle(false);
				mPngView.enableSubtitle(false);
			}
			mPlayer.setSuperImpose(state);
		}
	}

	public void setArea_0(int areaCode)
	{
		if(cOption.area_0 != areaCode)
		{
			cOption.area_0 = areaCode;
		}
		
		if(mDB != null)
		{
			mDB.putArea_0(areaCode);
		}
	}
	
	public void setArea_1(int areaCode)
	{
		if(cOption.area_1 != areaCode)
		{
			cOption.area_1 = areaCode;
		}
	
		if(mDB != null)
		{
			mDB.putArea_1(areaCode);
		}
	}
	
	public void setScan(int scan)
	{
		DxbLog(I, "setScan(" + scan + ")");

		if(cOption.scan != scan)
		{
			cOption.scan = scan;
		}

		if(mDB != null)
		{
			mDB.putScan(scan);		
		}
		
	}

	public int getScan ()
	{
		return cOption.scan;
	}

	public void setParentalRate(int age)
	{
		DxbLog(I, "setParentalRate(" + age + ")");

		if(cOption.age != age)
		{
			cOption.age	= age;
		}

		if(mDB != null)
		{
			mDB.putAge(age);			
		}
		
		if(mPlayer != null)
		{
			mPlayer.setParentalRate(age);
		}
	}	
	
	public void setChangePassword(String newPassword)
	{
		DxbLog(I, "setChangePassword(" + cOption.password + " --> " + newPassword + ")");
		
		if(!newPassword.equalsIgnoreCase(cOption.password))
		{
			mDB.putPassword(newPassword);
			cOption.password	= newPassword;
		}
	}

	public void setSeamlessChange(int iIndex)
	{
		//iIndex = 0(auto)/ 1(Fullseg) / 2(1seg)
		DxbLog(I, "setSeamlessChange(" + iIndex + ")");
		
		cOption.seamless_change = iIndex;

		if(mSQInfo != null)
		{
			mSQInfo.setSeamlessMode(cOption.seamless_change);
		}
	}
	
	public void setHandover(int state)
	{
		DxbLog(I, "setHandover(" + state + ")");
		
		if(state == 0)
		{
			cOption.handover	= false;
		}
		else
		{
			cOption.handover	= true;
		}
	}

	public void setEWS(int state)
	{
		DxbLog(I, "setEWS(" + state + ")");
		
		if(state == 0)
		{
			cOption.ews	= false;
		}
		else
		{
			cOption.ews	= true;
		}
	}

	public void setBCASModeChange(int state)
	{
		DxbLog(I, "setBCASModeChange(" + state + ")");
		
		cOption.bcas_mode = state;
	}

	public int requestBCASInfo(int arg)
	{
		DxbLog(I, "requestBCASInfo(" + arg + ")");
		
		if(mPlayer != null)
		{
			return mPlayer.reqSCInfo(arg);
		}
		
		return 0;
	}

	public void setSignalQuality(int state)
	{
		DxbLog(I, "setSignalQuality(" + state + ")");
		
		if(state == 0)
		{
			cOption.signal_quality = false;
		}
		else
		{
			cOption.signal_quality = true;
		}
	}
	
	public int getCountryCode()
	{
		return cOption.countrycode;
	}
	
	public void updateChannelInfo()
	{
		if(mPlayer == null)
		{
			return;
		}

		if(mChannel[iCurrentChannelIndex] == null)
		{
			return;
		}
		
		mPlayer.getChannelInfo(mChannel[iCurrentChannelIndex]);
		mPlayer.setSubtitle(cOption.caption);
		mPlayer.setSuperImpose(cOption.super_impose);
		
		DateTimeData date_time = new DateTimeData();
		mPlayer.getCurrentDateTime(date_time);

		if(mChannel[iCurrentChannelIndex].startHH < 24)
		{
			if(date_time.iPolarity == 0)
			{
				mChannel[iCurrentChannelIndex].startHH += date_time.iHourOffset;
				if((mChannel[iCurrentChannelIndex].startHH / 24) > 0) 
				{
					mChannel[iCurrentChannelIndex].startHH %= 24;
					mChannel[iCurrentChannelIndex].startMJD += 1;
				}
				mChannel[iCurrentChannelIndex].startMM += date_time.iMinOffset;
			}
			else
			{
				if(mChannel[iCurrentChannelIndex].startHH >= date_time.iHourOffset) 
				{
					mChannel[iCurrentChannelIndex].startHH -= date_time.iHourOffset;
				}
				else
				{
					mChannel[iCurrentChannelIndex].startHH = (mChannel[iCurrentChannelIndex].startHH + 24) - date_time.iHourOffset;
					mChannel[iCurrentChannelIndex].startMJD -= 1;
				}
				mChannel[iCurrentChannelIndex].startMM += date_time.iMinOffset;
			}	
		}
	}
	
	public int getRarray_Area(int iArea)
	{
		int	return_Rarray	= -1;
		switch(iArea)
		{
		case 0:
			return_Rarray	=R.array.area_hokkaidou_entries;
		break;
		
		case 1:
			return_Rarray	=R.array.area_touhoku_entries;
		break;
		
		case 2:
			return_Rarray	=R.array.area_kantou_entries;
		break;
		
		case 3:
			return_Rarray	=R.array.area_hokshinetsu_entries;
		break;
		
		case 4:
			return_Rarray	=R.array.area_toukai_entries;
		break;
		
		case 5:
			return_Rarray	=R.array.area_kinki_entries;
		break;
		
		case 6:
			return_Rarray	=R.array.area_cyuugoku_entries;
		break;
		
		case 7:
			return_Rarray	=R.array.area_shikoku_entries;
		break;
		
		case 8:
			return_Rarray	=R.array.area_kyuusyuu_entries;
		break;
		}
		
		return	return_Rarray;
	}
	
	public int getAreaCode()
	{
		int	return_AreaCode	= -1;
		switch(cOption.area_0)
		{
			case 0:
				return_AreaCode	= cOption.area_1 + 10;
			break;
		
			case 1:
				return_AreaCode	= cOption.area_1 + 17;
			break;
		
			case 2:
				if(cOption.area_1 == 7)
				{
					return_AreaCode	= 32;
				}
				else
				{
					return_AreaCode	= cOption.area_1 + 23;
				}
			break;
		
			case 3:
				switch(cOption.area_1)
				{
					case 0:
					case 1:
						return_AreaCode	= cOption.area_1 + 30;
					break;
					
					case 2:
						return_AreaCode	= 34;
					break;
					
					case 3:
						return_AreaCode	= 36;
					break;
					case 4:
						return_AreaCode	= 37;
					break;
				}
			break;
		
			case 4:
				switch(cOption.area_1)
				{
					case 0:
						return_AreaCode	= 33;
					break;
					
					case 1:
					case 2:
						return_AreaCode	= cOption.area_1 + 37;
					break;
					
					case 3:
						return_AreaCode	= 35;
					break;
				}
			break;
		
			case 5:
				return_AreaCode	= cOption.area_1 + 40;
			break;
		
			case 6:
				switch(cOption.area_1)
				{
					case 0:
						return_AreaCode	= 46;
					break;
					
					case 1:
					case 2:
					case 3:
						return_AreaCode	= cOption.area_1 + 47;
					break;
					
					case 4:
						return_AreaCode	= 47;
					break;
				}
			break;
		
			case 7:
				switch(cOption.area_1)
				{
					case 0:
						return_AreaCode	= 52;
					break;
					
					case 1:
						return_AreaCode	= 51;
					break;
					
					case 2:
					case 3:
						return_AreaCode	= cOption.area_1 + 51;
					break;
				}
			break;
		
			case 8:
				return_AreaCode	= cOption.area_1 + 55;
			break;
		}
		
		return 	return_AreaCode;
	}

	public String getVideoMode(int iVideo)
	{
		CharSequence[] getString = mContext.getResources().getTextArray(R.array.epg_video_mode);
		String VideoMode = null;

		switch(iVideo)
		{
			case 0x01 :
			case 0x02 :
			case 0x03 :
			case 0x04 :
			case 0xA1 :
			case 0xA2 :
			case 0xA3 :
			case 0xA4 :
				VideoMode = getString[1].toString();					
			break;

			case 0xB2 :
			case 0xB3 :
			case 0xB4 :
			case 0xC1 :
			case 0xC2 :
			case 0xC3 :
			case 0xC4 :
				VideoMode = getString[0].toString();					
			break;
	
			case 0xD1 :
			case 0xD2 :
			case 0xD3 :
			case 0xD4 :
			default:
				VideoMode = null;
			break;
		}

		return VideoMode;
	}

	public String getVideoMode()
	{
		if(mChannel[iCurrentChannelIndex] == null)
		{
			return "";
		}
		
		return getVideoMode(mChannel[iCurrentChannelIndex].videoMode);
	}
	
	public String getAudioMode(int iAudio)
	{
		CharSequence[] getString;
		String AudioMode = null;

		if(iAudio == 0x02)
		{
			getString = mContext.getResources().getTextArray(R.array.dual_audio_select_entries);
			AudioMode = getString[cOption.dual_audio].toString();
			
		}
		else if(iAudio > 0)
		{
			getString = mContext.getResources().getTextArray(R.array.epg_audio_mode);
			AudioMode = getString[mChannel[iCurrentChannelIndex].audioMode-1].toString();
		}

		return AudioMode;
	}

	public String getAudioMode()
	{
		if(mChannel[iCurrentChannelIndex] == null)
		{
			return "";
		}
		
		return getAudioMode(mChannel[iCurrentChannelIndex].audioMode);
	}
	
	public String getLang(int iLang)
	{
		switch(iLang)
		{
			case 0x6A706E:
				return "JPN";
			case 0x656E67:
				return "ENG";
			case 0x646575:
				return "DEU";
			case 0x667261:
				return "FRA";
			case 0x697461:
				return "ITA";
			case 0x727573:
				return "RUS";
			case 0x7A686F:
				return "ZHO";
			case 0x6B6F72:
				return "KOR";
			case 0x737061:
				return "SPA";
			case 0x627261:
				return "BRA";
			case 0x657463:
			default:
				return "etc";
		}
	}

	public String getAudioLang()
	{
		String AudioLang = null;

		if(mChannel[iCurrentChannelIndex] == null)
		{
			return AudioLang;
		}
		
		if(mChannel[iCurrentChannelIndex].audioLang1 > 0 && mChannel[iCurrentChannelIndex].audioLang2 > 0)
		{
			AudioLang = getLang(mChannel[iCurrentChannelIndex].audioLang1) + "/" + getLang(mChannel[iCurrentChannelIndex].audioLang2);
		}
		else if(mChannel[iCurrentChannelIndex].audioLang1 > 0)
		{
			AudioLang = getLang(mChannel[iCurrentChannelIndex].audioLang1);
		}
		else if(mChannel[iCurrentChannelIndex].audioLang2 > 0)
		{
			AudioLang = getLang(mChannel[iCurrentChannelIndex].audioLang2);
		}

		return AudioLang;
	}

	public String getAudioInfo()
	{
		String AudioInfo = null;

		if(mChannel[iCurrentChannelIndex] == null)
		{
			return AudioInfo;
		}

		if(mChannel[iCurrentChannelIndex].audioLang1 > 0 && mChannel[iCurrentChannelIndex].audioLang2 > 0)
		{
			AudioInfo = mContext.getResources().getString(R.string.bilingual);
		}

		if(mChannel[iCurrentChannelIndex].audioMode == 0x40 || mChannel[iCurrentChannelIndex].audioMode == 0x41)
		{
			if(AudioInfo == null)
			{
				AudioInfo = mContext.getResources().getString(R.string.descriptive_video_service);
			}
			else
			{
				AudioInfo += "\n" + mContext.getResources().getString(R.string.descriptive_video_service);
			}
		}
		
		return AudioInfo;
	}
 
	public int getSubtitleCount()
	{
		return mChannel[iCurrentChannelIndex].totalSubtitleLang;
	}

	public String getEPGEvent()
	{
		String eventName = "";

		if(mChannel[iCurrentChannelIndex] == null || mChannel[iCurrentChannelIndex].eventName == null)
		{
			return eventName;
		}

		if(mChannel[iCurrentChannelIndex].eventName.length() > 28)
		{
			eventName = mChannel[iCurrentChannelIndex].eventName.substring(0, 28) + "...";
		}
		else
		{
			eventName = mChannel[iCurrentChannelIndex].eventName;
		}

		return eventName;
	}
	
	public String getEPGDateTime()
	{
		int yDash, mDash, k, Day, Month, Year;
		int sHour, sMin, eHour, eMin, dHour, dMin, b;
		String szDateTime, szDate, szSHour, szSMin, szEHour, szEMin;
		
		szDateTime = szDate = szSHour = szSMin = szEHour = szEMin = "";

		if(mChannel[iCurrentChannelIndex] == null)
		{
			return szDateTime;
		}

		if(mChannel[iCurrentChannelIndex].startMJD > 0)
		{
			yDash = ( mChannel[iCurrentChannelIndex].startMJD * 100 - 1507820) / 36525;
			mDash = ( mChannel[iCurrentChannelIndex].startMJD * 10000 - 149561000 - (((yDash * 3652500)/10000)*10000) ) / 306001;

			Day = (mChannel[iCurrentChannelIndex].startMJD * 10000 - 149560000 - (((yDash * 3652500)/10000)*10000) - (((mDash * 306001)/10000)*10000)) / 10000;
			if(mDash == 14 || mDash == 15)
			{
				k = 1;
			}
			else
			{
				k = 0;
			}

			Year = yDash + k + 1900;
			Month = mDash - 1 - k * 12;

			szDate = Year + ". " + Month + ". " + Day + ".";
		}

		if((mChannel[iCurrentChannelIndex].startHH>=0)&&(mChannel[iCurrentChannelIndex].startMM>=0)&&(mChannel[iCurrentChannelIndex].durationHH>=0)&&(mChannel[iCurrentChannelIndex].durationMM>=0))
		{
			sHour = mChannel[iCurrentChannelIndex].startHH;
			sMin = mChannel[iCurrentChannelIndex].startMM;
			dHour = mChannel[iCurrentChannelIndex].durationHH;
			dMin = mChannel[iCurrentChannelIndex].durationMM;
			eHour = eMin = 0;

			if((sHour==0xff)&&(sMin==0xff)&&(dHour==0xff)&&(dMin==0xff))
			{
				szSHour= "**";
				szSMin = "**";
				szEHour = "**";
				szEMin = "**";
			}
			else if((dHour==0xff)&&(dMin==0xff))
			{
				szSHour= String.format("%02d", sHour);
				szSMin = String.format("%02d", sMin);
				szEHour = "**";
				szEMin = "**";
			}
			else
			{
				eMin = sMin + dMin;
				b = eMin/60;
				eMin %=60;

				eHour = (sHour + b + dHour)%24;

				szSHour = String.format("%02d", sHour);
				szSMin = String.format("%02d", sMin);
				szEHour = String.format("%02d", eHour);
				szEMin = String.format("%02d", eMin);
			}

			szDateTime = String.format("%s  %s:%s ~ %s:%s", szDate, szSHour, szSMin, szEHour, szEMin);
		}

		return szDateTime;
	}

	public int getLogoImageWidth()
	{
		if(mChannel[iCurrentChannelIndex] == null)
		{
			return 0;
		}
		
		return mChannel[iCurrentChannelIndex].logoImageWidth;
	}

	public int getLogoImageHeight()
	{
		if(mChannel[iCurrentChannelIndex] == null)
		{
			return 0;
		}

		return mChannel[iCurrentChannelIndex].logoImageHeight;
	}

	public int[] getLogoImage()
	{
		if(mChannel[iCurrentChannelIndex] == null)
		{
			return null;
		}

		return mChannel[iCurrentChannelIndex].plogoImage;
	}
	
	public int getSCStatus()
	{
		return iSCStatus;
	}
	
	public boolean getSCDisplay()
	{
		return bSCDisplay;
	}

	public void setSCDisplay(boolean value)
	{
		bSCDisplay = value;
	}

	public void setSCDisplay(int status, int type)
	{	
		boolean value = false;
		
		if(type == ISDBT_SECTION_FSEG) 
   		{
			if(status != SCInfoData.SC_ERR_OK)
			{
				value = true;
			}
   		}

		bSCDisplay = value;
	}
	
	public void setSCInfo(SCInfoData obj)
	{
		DxbLog(I, "onSCInfoUpdate()");
		
		DxbLog(D, "======================================\n");
		DxbLog(D, "scinfolistener : show warning message ()");
		DxbLog(D, "Size=" + obj.SCDataSize + "\n");
		for (int i=0 ; i < 34 /*obj.SCDataSize*/ ; i++)
		{
			DxbLog(D, "Data[" + i + "]=" + obj.SCData[i] + "\n");
		}
		DxbLog(D, "======================================\n");

		int	Number_of_card_IDs;
		
		Number_of_card_IDs	= obj.SCData[6];
		cBCAScardInfo.number	= Number_of_card_IDs;
		//Log.d(TAG, "Number_of_card_IDs = " + Number_of_card_IDs);
		
		for(int i=0 ; i<Number_of_card_IDs ; i++)
		{
			if(i == 0)
			{
				/*	get Card type	*/
					String	sManufacturer_Identifier	= new Character((char) obj.SCData[8]).toString();
					String	sVersion					= String.format("%03d", obj.SCData[9]);

					cBCAScardInfo.type	= sManufacturer_Identifier + sVersion;
					//Log.d(TAG, gBCAScard_info.type);
				
				/*	get Card ID	*/
					int	iID_Identifier	= obj.SCData[11];
					
					long	lData[]	= new long[8];
					lData[0]	=  obj.SCData[12] & 0xFF;
					lData[1]	=  obj.SCData[13] & 0xFF;
					lData[2]	=  obj.SCData[14] & 0xFF;
					lData[3]	=  obj.SCData[15] & 0xFF;
					lData[4]	=  obj.SCData[16] & 0xFF;
					lData[5]	=  obj.SCData[17] & 0xFF;
					lData[6]	=  obj.SCData[18] & 0xFF;
					lData[7]	=  obj.SCData[19] & 0xFF;
					/*Log.d(TAG, "lData[0]=" + String.format("%X",lData[0]));
					Log.d(TAG, "lData[1]=" + String.format("%X",lData[1]));
					Log.d(TAG, "lData[2]=" + String.format("%X",lData[2]));
					Log.d(TAG, "lData[3]=" + String.format("%X",lData[3]));
					Log.d(TAG, "lData[4]=" + String.format("%X",lData[4]));
					Log.d(TAG, "lData[5]=" + String.format("%X",lData[5]));
					Log.d(TAG, "lData[6]=" + String.format("%X",lData[6]));
					Log.d(TAG, "lData[7]=" + String.format("%X",lData[7]));*/
					
					long	ID	=		(lData[0] << (8*5))
									+	(lData[1] << (8*4))
									+	(lData[2] << (8*3))
									+	(lData[3] << (8*2))
									+	(lData[4] << (8*1))
									+	(lData[5] << (8*0));	//	big endian
					//Log.d(TAG, "ID=" + String.format("%X",ID));
					
					long	Check_Code	=		(lData[6] << (8*0))
											+	(lData[7] << (8*1));	//	little endian
					//Log.d(TAG, "Check_Code=" + String.format("%X",Check_Code));
					
					long	Display_ID[]	= new long[4];
					Display_ID[0]	= ID/1000/10000/10000;
					Display_ID[1]	= ID/1000/10000		% 10000;
					Display_ID[2]	= ID/1000			% 10000;
					Display_ID[3]	= ID				% 1000;
					/*Log.d(TAG, Long.toString(Display_ID[0]));
					Log.d(TAG, Long.toString(Display_ID[1]));
					Log.d(TAG, Long.toString(Display_ID[2]));
					Log.d(TAG, Long.toString(Display_ID[3]));*/
					
					long	Display_Check_Code[]	= new long[2];
					Display_Check_Code[0]	= Check_Code/10000;
					Display_Check_Code[1]	= Check_Code%10000;
					//Log.d(TAG, Long.toString(Display_Check_Code[0]));
					//Log.d(TAG, Long.toString(Display_Check_Code[1]));
					
					cBCAScardInfo.ID_card[0]	= String.format("%01d", iID_Identifier) + String.format("%03d", Display_ID[0]);
					cBCAScardInfo.ID_card[1]	= String.format("%04d", Display_ID[1]);
					cBCAScardInfo.ID_card[2]	= String.format("%04d", Display_ID[2]);
					cBCAScardInfo.ID_card[3]	= String.format("%03d", Display_ID[3]) + String.format("%01d", Display_Check_Code[0]);
					cBCAScardInfo.ID_card[4]	= String.format("%04d", Display_Check_Code[1]);
					/*Log.d(TAG, gBCAScard_info.ID_card[0]);
					Log.d(TAG, gBCAScard_info.ID_card[1]);
					Log.d(TAG, gBCAScard_info.ID_card[2]);
					Log.d(TAG, gBCAScard_info.ID_card[3]);
					Log.d(TAG, gBCAScard_info.ID_card[4]);*/
			}
			else /*if(i == 1)*/
			{
				/*	get Group ID	*/
				int	iID_Identifier	= obj.SCData[11];
				
				long	lData[]	= new long[8];
				lData[0]	=  obj.SCData[12] & 0xFF;
				lData[1]	=  obj.SCData[13] & 0xFF;
				lData[2]	=  obj.SCData[14] & 0xFF;
				lData[3]	=  obj.SCData[15] & 0xFF;
				lData[4]	=  obj.SCData[16] & 0xFF;
				lData[5]	=  obj.SCData[17] & 0xFF;
				lData[6]	=  obj.SCData[18] & 0xFF;
				lData[7]	=  obj.SCData[19] & 0xFF;
				/*Log.d(TAG, "lData[0]=" + String.format("%X",lData[0]));
				Log.d(TAG, "lData[1]=" + String.format("%X",lData[1]));
				Log.d(TAG, "lData[2]=" + String.format("%X",lData[2]));
				Log.d(TAG, "lData[3]=" + String.format("%X",lData[3]));
				Log.d(TAG, "lData[4]=" + String.format("%X",lData[4]));
				Log.d(TAG, "lData[5]=" + String.format("%X",lData[5]));
				Log.d(TAG, "lData[6]=" + String.format("%X",lData[6]));
				Log.d(TAG, "lData[7]=" + String.format("%X",lData[7]));*/
				
				long	ID	=		(lData[0] << (8*5))
								+	(lData[1] << (8*4))
								+	(lData[2] << (8*3))
								+	(lData[3] << (8*2))
								+	(lData[4] << (8*1))
								+	(lData[5] << (8*0));	//	big endian
				//Log.d(TAG, "ID=" + String.format("%X",ID));
				
				long	Check_Code	=		(lData[6] << (8*0))
										+	(lData[7] << (8*1));	//	little endian
				//Log.d(TAG, "Check_Code=" + String.format("%X",Check_Code));
				
				long	Display_ID[]	= new long[4];
				Display_ID[0]	= ID/1000/10000/10000;
				Display_ID[1]	= ID/1000/10000		% 10000;
				Display_ID[2]	= ID/1000			% 10000;
				Display_ID[3]	= ID				% 1000;
				/*Log.d(TAG, Long.toString(Display_ID[0]));
				Log.d(TAG, Long.toString(Display_ID[1]));
				Log.d(TAG, Long.toString(Display_ID[2]));
				Log.d(TAG, Long.toString(Display_ID[3]));*/
				
				long	Display_Check_Code[]	= new long[2];
				Display_Check_Code[0]	= Check_Code/10000;
				Display_Check_Code[1]	= Check_Code%10000;
				//Log.d(TAG, Long.toString(Display_Check_Code[0]));
				//Log.d(TAG, Long.toString(Display_Check_Code[1]));
				
				cBCAScardInfo.ID_group[0]	= String.format("%01d", iID_Identifier) + String.format("%03d", Display_ID[0]);
				cBCAScardInfo.ID_group[1]	= String.format("%04d", Display_ID[1]);
				cBCAScardInfo.ID_group[2]	= String.format("%04d", Display_ID[2]);
				cBCAScardInfo.ID_group[3]	= String.format("%03d", Display_ID[3]) + String.format("%01d", Display_Check_Code[0]);
				cBCAScardInfo.ID_group[4]	= String.format("%04d", Display_Check_Code[1]);
				/*Log.d(TAG, gBCAScard_info.ID_group[0]);
				Log.d(TAG, gBCAScard_info.ID_group[1]);
				Log.d(TAG, gBCAScard_info.ID_group[2]);
				Log.d(TAG, gBCAScard_info.ID_group[3]);
				Log.d(TAG, gBCAScard_info.ID_group[4]);*/
			}
		}
	}
	
	public boolean isReqPresetModeChange()
	{
		if (mPresetMode != cOption.preset_mode) {
			DxbLog(I, "preset_mode was changed"+mPresetMode+"->"+cOption.preset_mode);
			return true;
		}
		else
			return false;
	}
	public void setPresetMode ()
	{
		int rtn;
		mPresetMode = cOption.preset_mode;
		if(mChannelManager != null) mChannelManager.setPresetMode(mPresetMode);
		if(mPlayer !=null ) {
			rtn = mPlayer.setPresetMode(mPresetMode);
		}
		setChannelList(true);
	}

	public void startHandover()
	{
		DxbLog(I, "startHandover");
		
		if (mPlayer != null)
		{
			mPlayer.setHandover(0);
			
			if(isDual_decoding == true)
			{
				iCurrentChannelIndex = DUAL_DECODE_MAIN;
			}
		}
	}
	
	public void monitorSignal()
	{
		int	channelIndex = 0;

		if(isDual_decoding == false)
		{
			return;
		}

		if (mChannel[iDefaultChannelIndex].serviceType == ISDBT_SECTION_FSEG)
		{									
			channelIndex = DUAL_DECODE_MAIN;	
		}
		else
		{
			channelIndex = DUAL_DECODE_SUB;	
		}	
		
		switch (cOption.seamless_change)
		{
			case 0: //Auto
				if(mSQInfo.isStrongSignal() == false)
				{
					channelIndex = DUAL_DECODE_SUB;
				}

				if(DxbOptions.VISIBLE_ID_bcas_mode == false)
				{
					if((iSCStatus != SCInfoData.SC_ERR_OK) && (mChannel[iDefaultChannelIndex].serviceType != ISDBT_SECTION_1SEG))
					{
						channelIndex = DUAL_DECODE_SUB;
					}
				}
			break;
												
			case 1: //Full-seg
			break;

			case 2: //1-seg
				channelIndex = DUAL_DECODE_SUB;
			break;
			
			default:
			break;
		}
		
		if(DxbOptions.VISIBLE_ID_bcas_mode == true)
		{
			if((cOption.bcas_mode == 1) && (iSCStatus != SCInfoData.SC_ERR_OK) && (mChannel[iDefaultChannelIndex].serviceType != ISDBT_SECTION_1SEG))
			{
				channelIndex = DUAL_DECODE_SUB;
			}
		}

		if(isVideoPlay_update == true)
		{
			if(channelIndex != iCurrentChannelIndex)
			{
				DxbLog(D, "setDualChannel " + iCurrentChannelIndex + " -> " + channelIndex);
				
				if(mPlayer != null)
				{
					mPlayer.setDualChannel(channelIndex);
					iCurrentChannelIndex = channelIndex;
				}
			}
		}
	}

	DialogInterface.OnClickListener ListenerOnClick_selectArea0	= new DialogInterface.OnClickListener()
	{
		public void onClick(DialogInterface dialog, int item)
		{
			DxbLog(I, "onClick(dialog="+dialog+", item="+item);
			
			if(item == -1)
			{
				onDestroy();
				android.os.Process.killProcess(android.os.Process.myPid());
				
				return;
			}

			cOption.area_0_temp	= item;
			
			selectArea_1();
			
			dialog.dismiss();
		}
	};
	
	DialogInterface.OnCancelListener ListenerOnCancel_selectArea0 = new DialogInterface.OnCancelListener()
	{
		public void onCancel(DialogInterface dialog)
		{
			DxbLog(I, "onCancel()");
			
			onDestroy();
			android.os.Process.killProcess(android.os.Process.myPid()); 
		}
	};
	
	DialogInterface.OnClickListener ListenerOnClick_selectArea1	= new DialogInterface.OnClickListener()
	{
		public void onClick(DialogInterface dialog, int item)
		{
			cOption.area_1_temp	= item;
		
			dialog.dismiss();
			
			selectScan();
		}
	};

	DialogInterface.OnCancelListener ListenerOnCancel_selectArea1 = new DialogInterface.OnCancelListener()
	{
		public void onCancel(DialogInterface dialog)
		{
			DxbLog(I, "onCancel()");
			
			onDestroy();
			android.os.Process.killProcess(android.os.Process.myPid()); 
		}
	};
	
	DialogInterface.OnClickListener ListenerOnClick_selectScan	= new DialogInterface.OnClickListener()
	{
		public void onClick(DialogInterface dialog, int which)
		{
			DxbLog(I, "onClick(dialog="+dialog+", which="+which);
			
			int	item	= which * -1;
			cOption.scan	= item + 1;
			cOption.area_0	= cOption.area_0_temp;
			cOption.area_1	= cOption.area_1_temp;
			
			setScan(cOption.scan);
			setArea_0(cOption.area_0);
			setArea_1(cOption.area_1);
			makePresetList();
			
			dialog.dismiss();
			
			if (mOnAreaChangeListener != null)
			{
				mOnAreaChangeListener.onAreaChange();
			}
		}
	};
	
	private void selectArea_0()
	{
		DxbLog(I, "selectArea_0()");
		
		Builder Builder_Area0 = new AlertDialog.Builder(mContext);

		Builder_Area0.setTitle(mContext.getResources().getString(R.string.area_code));
		Builder_Area0.setSingleChoiceItems(R.array.area_entries, -1, ListenerOnClick_selectArea0);
		Builder_Area0.setOnCancelListener(ListenerOnCancel_selectArea0);

		Builder_Area0.setPositiveButton(mContext.getResources().getString(R.string.cancel), ListenerOnClick_selectArea0);
		Dialog Dialog_Area0 = Builder_Area0.create();
		Dialog_Area0.show();
	}

	private void selectArea_1()
	{
		DxbLog(I, "selectArea_1()");
		
		Builder Builder_Area1 = new AlertDialog.Builder(mContext);
 
		Builder_Area1.setTitle(mContext.getResources().getString(R.string.area_code));
		Builder_Area1.setSingleChoiceItems(getRarray_Area(cOption.area_0_temp), -1, ListenerOnClick_selectArea1);
		Builder_Area1.setOnCancelListener(ListenerOnCancel_selectArea1);

		Builder_Area1.setPositiveButton(mContext.getResources().getString(R.string.cancel), null);
		Dialog Dialog_Area1 = Builder_Area1.create();
		Builder_Area1.show();
	}

	private void selectScan()
	{
		AlertDialog.Builder	Builder_Scan = new AlertDialog.Builder(mContext);
		
		Builder_Scan.setMessage(mContext.getResources().getString(R.string.want_to_scan));
		
		Builder_Scan.setPositiveButton(mContext.getResources().getString(R.string.yes), ListenerOnClick_selectScan);
		Builder_Scan.setNegativeButton(mContext.getResources().getString(R.string.no), ListenerOnClick_selectScan);
		
		AlertDialog	Dialog_Scan	= Builder_Scan.create();
		Dialog_Scan.show();
	}
	
	/************************************************************************************************************************
	 * 	Listener -----> Start
	 ************************************************************************************************************************/
	private ISDBTPlayer.OnPreparedListener ListenerOnPrepared = new ISDBTPlayer.OnPreparedListener() {
		public void onPrepared(ISDBTPlayer player) {
			DxbLog(D,		"OnPreparedListener   " + "     :" + cChannelInfo.iCurrent_TV);
			if (mOnPreparedListener != null) {
				mOnPreparedListener.onPrepared(mDxbPlayer);
			}
//			DxbView_List.updateChannelList(0, 2);
		}
	};

		public interface OnPreparedListener {
			void onPrepared(DxbPlayer player);
		}

		public void setOnPreparedListener(OnPreparedListener listener) {
			mOnPreparedListener = listener;
		}

		private OnPreparedListener mOnPreparedListener;

	private ISDBTPlayer.OnVideoOutputListener ListenerOnVideoOutput = new ISDBTPlayer.OnVideoOutputListener()
	{
		public void onVideoOutputUpdate(ISDBTPlayer player)
		{
			DxbLog(I, "onVideoOutputUpdate() ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~>>>>>>>>>>>");
			
			isVideoPlay_output	= true;
			if(mOnVideoOutputListener != null)
			{
				mOnVideoOutputListener.onVideoOutput(mDxbPlayer);				
				
				enableSubtitle(true);				
			}
		}
	};
	
	public interface OnVideoOutputListener
	{
		void onVideoOutput(DxbPlayer player);		
	}
	
	public void setOnVideoOutputListener(OnVideoOutputListener listener)
	{
		mOnVideoOutputListener	= listener;
	}
	
	private OnVideoOutputListener mOnVideoOutputListener;
	
	private ISDBTPlayer.OnErrorListener ListenerOnError = new ISDBTPlayer.OnErrorListener() {
		public boolean onError(ISDBTPlayer player, int what, int extra) {
			DxbLog(I, "Play Error!");
			return false;
		}
	};

	private ISDBTPlayer.OnVideoDefinitionUpdateListener ListenerOnVideoDefinitionUpdate = new ISDBTPlayer.OnVideoDefinitionUpdateListener()
	{
		public void onVideoDefinitionUpdate(ISDBTPlayer player, VideoDefinitionData videoinfo)
		{
			DxbLog(I, "onVideoDefinitionUpdate() ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~>>>>>>>>>>>");

			DxbLog(W, "width : " + videoinfo.width);
			DxbLog(W, "height : " + videoinfo.height);
			
			if(isVideoPlay_update == false)
			{
				isVideoPlay_update	= true;
				isVideoPlay_output = false;
			}
			
			if(videoinfo.aspect_ratio == 0)
			{
				DxbLog(W, "aspect ratio : " + videoinfo.aspect_ratio + "[16:9]");
			}
			else
			{
				DxbLog(W, "aspect ratio : " + videoinfo.aspect_ratio + "[4:3]");
			}
			
			mVideoWidth = videoinfo.width;
			mVideoHeight = videoinfo.height;
			mVideoAspectRatio = videoinfo.aspect_ratio;
			mVideoFrameRate = videoinfo.frame_rate;
			mVideoProgressive = videoinfo.progressive;

			if(videoinfo.DisplayID == 0)
			{
				if(videoinfo.sub_DecoderID == 1)
				{
					mVideoWidth = videoinfo.sub_width;
					mVideoHeight = videoinfo.sub_height;
					mVideoAspectRatio = videoinfo.sub_aspect_ratio;
				}

				if(videoinfo.main_DecoderID == 0)
				{
					mVideoWidth = videoinfo.width;
					mVideoHeight = videoinfo.height;
					mVideoAspectRatio = videoinfo.aspect_ratio;
					mVideoFrameRate = videoinfo.frame_rate;
					
					if(mSurfaceHolder != null)
					{
						mSurfaceHolder.setFixedSize(mVideoWidth, mVideoHeight);
					}
				}
			}
			else if(videoinfo.DisplayID == 1)
			{
				mVideoWidth = videoinfo.sub_width;
				mVideoHeight = videoinfo.sub_height;
				mVideoAspectRatio = videoinfo.sub_aspect_ratio;
			}

			if (mWidth == 0 && mHeight == 0)
			{
				setScreenMode(mScreenMode, 0, 0, 0, 0);
			}
		}
	};

    private ISDBTPlayer.OnDebugModeListener ListenerOnDebugMode = new ISDBTPlayer.OnDebugModeListener() {
		public void onDebugUpdate(ISDBTPlayer player, DebugModeData data) {
			if (mOnDebugModeListener != null) {
				mOnDebugModeListener.onDebugUpdate(mDxbPlayer, data);
			}
		}
	};

		public interface OnDebugModeListener {
			void onDebugUpdate(DxbPlayer player, DebugModeData data);
		}

		public void setOnDebugModeListener(OnDebugModeListener listener) {
			mOnDebugModeListener = listener;
		}

		private OnDebugModeListener	mOnDebugModeListener;

		private ISDBTPlayer.OnSearchPercentListener ListenerOnSearchPercent = new ISDBTPlayer.OnSearchPercentListener() {
			public void onSearchPercentUpdate(ISDBTPlayer player, int nPercent, int nChannel) {
				if (mOnSearchPercentListener != null) {
					mOnSearchPercentListener.onSearchPercentUpdate(mDxbPlayer, nPercent, nChannel);
				}
			}
		};

    	public interface OnSearchPercentListener {
    		void onSearchPercentUpdate(DxbPlayer player, int nPercent, int nChannel);
    	}

    	public void setOnSearchPercentListener(OnSearchPercentListener listener) {
    		mOnSearchPercentListener = listener;
    	}

    	private OnSearchPercentListener mOnSearchPercentListener;

	private ISDBTPlayer.OnSearchCompletionListener ListenerOnSearchCompletion = new ISDBTPlayer.OnSearchCompletionListener() {
   		public void onSearchCompletion(ISDBTPlayer player) {
   			if (mOnSearchCompletionListener != null) {
   				mOnSearchCompletionListener.onSearchCompletion(mDxbPlayer);
				playSubtitle(_ON_);
   			}
   		}
   	};

   	public interface OnSearchCompletionListener {
   		void onSearchCompletion(DxbPlayer player);
   	}

   	public void setOnSearchCompletionListener(OnSearchCompletionListener listener) {
   		mOnSearchCompletionListener = listener;
   	}

   	private OnSearchCompletionListener mOnSearchCompletionListener;

	private ISDBTPlayer.OnDBInformationUpdateListener ListenerOnDBInformationUpdate = new ISDBTPlayer.OnDBInformationUpdateListener()
	{
		public void onDBInformationUpdate(ISDBTPlayer player, int type, int param)
		{
			if (mOnDBInformationUpdateListener != null)
			{
				mOnDBInformationUpdateListener.onDBInformationUpdate(mDxbPlayer, type, param);
			}
		}
	};

	public interface OnDBInformationUpdateListener {
		void onDBInformationUpdate(DxbPlayer player, int type, int param);
	}

	public void setOnDBInformationUpdateListener(OnDBInformationUpdateListener listener) {
		mOnDBInformationUpdateListener = listener;
	}

	private OnDBInformationUpdateListener mOnDBInformationUpdateListener;

	private ISDBTPlayer.OnPlayingCompletionListener ListenerOnPlayingCompletion = new ISDBTPlayer.OnPlayingCompletionListener()
	{
		public void onPlayingCompletion(ISDBTPlayer player, int type, int state)
		{
			if (type == 0)
			{
//				mChannel[iDefaultChannelIndex].filePlay = 0;
				eLocalPlayingState = LOCAL_STATE.STOP;
			}
				
			if (mOnPlayingCompletionListener != null)
			{
				mOnPlayingCompletionListener.onPlayingCompletion(mDxbPlayer, type, state);
			}
		}
	};

	public interface OnPlayingCompletionListener
	{
		void onPlayingCompletion(DxbPlayer player, int type, int state);
	}

	public void setOnPlayingCompletionListener(OnPlayingCompletionListener listener)
	{
		mOnPlayingCompletionListener = listener;
	}

	private OnPlayingCompletionListener mOnPlayingCompletionListener;

	private ISDBTPlayer.OnRecordingCompletionListener ListenerOnRecordingCompletion = new ISDBTPlayer.OnRecordingCompletionListener()
	{
		public void onRecordingCompletion(ISDBTPlayer player, int result)
		{
			if (mOnRecordingCompletionListener != null)
			{
				mOnRecordingCompletionListener.onRecordingCompletion(mDxbPlayer, result);
			}
			
			eRecState = REC_STATE.STOP;
		}
	};

	public interface OnRecordingCompletionListener {
		void onRecordingCompletion(DxbPlayer player, int result);
	}
	
	public void setOnRecordingCompletionListener(OnRecordingCompletionListener listener) {
		mOnRecordingCompletionListener = listener;
	}
	
	private OnRecordingCompletionListener mOnRecordingCompletionListener;

	private ISDBTPlayer.OnFilePlayTimeUpdateListener ListenerOnTimeUpdate = new ISDBTPlayer.OnFilePlayTimeUpdateListener()
	{
		public void onFilePlayTimeUpdate(ISDBTPlayer player, int time)
		{
			if (mOnFilePlayTimeUpdateListener != null)
			{
				mOnFilePlayTimeUpdateListener.onFilePlayTimeUpdate(mDxbPlayer, time);
			}
		}
	};

	public interface OnFilePlayTimeUpdateListener
	{
		void onFilePlayTimeUpdate(DxbPlayer player, int time);
	}
	
	public void setOnFilePlayTimeUpdateListener(OnFilePlayTimeUpdateListener listener)
	{
		mOnFilePlayTimeUpdateListener = listener;
	}
	
	private OnFilePlayTimeUpdateListener mOnFilePlayTimeUpdateListener;

	private ISDBTPlayer.OnTeletextDataUpdateListener ListenerOnTeletextDataUpdate = new ISDBTPlayer.OnTeletextDataUpdateListener()
	{
		public void onTeletextDataUpdate(ISDBTPlayer player, TeletextData teletextData)
		{
			//Log.i("DxbPlayer", "onTeletextDataUpdate()");
			if (mRenderTTX != null)
			{
				mRenderTTX.Rendering(teletextData, false, false);
			}
		}
	};

	public interface OnTeletextDataUpdateListener
	{
		void onTeletextDataUpdate(DxbPlayer player, Bitmap bitmap);
	}

	public void setOnTeletextDataUpdateListener(OnTeletextDataUpdateListener listener)
	{
		mOnTeletextDataUpdateListener = listener;
	}

		private OnTeletextDataUpdateListener mOnTeletextDataUpdateListener;

	public void updateTeletextData(Bitmap bitmap)
	{
		mOnTeletextDataUpdateListener.onTeletextDataUpdate(mDxbPlayer, bitmap);
	}
	
	public interface OnChannelChangeListener
	{
	    void onChannelChange();
	}
		
	public void setOnChannelChangeListener(OnChannelChangeListener listener)
	{
		mOnChannelChangeListener = listener;
	}

	private OnChannelChangeListener mOnChannelChangeListener;
	
	private ISDBTPlayer.OnChannelUpdateListener ListenerOnChannelUpdate = new ISDBTPlayer.OnChannelUpdateListener()
	{
		public void onChannelUpdate(ISDBTPlayer player, int request)
		{
			if (mOnChannelUpdateListener != null)
			{
				mOnChannelUpdateListener.onChannelUpdate(mDxbPlayer, request);
			}
		}
	};
	
	public interface OnChannelUpdateListener {
		void onChannelUpdate(DxbPlayer player, int request);
	}

	public void setOnChannelUpdateListener(OnChannelUpdateListener listener) {
		mOnChannelUpdateListener = listener;
	}

	private OnChannelUpdateListener mOnChannelUpdateListener;

	public interface OnAreaChangeListener {
		void onAreaChange();
	}

	public void setOnAreaChangeListener(OnAreaChangeListener listener) {
		mOnAreaChangeListener = listener;
	}

	private OnAreaChangeListener mOnAreaChangeListener;
	
	private ISDBTPlayer.OnHandoverChannelListener ListenerOnHandoverChannel = new ISDBTPlayer.OnHandoverChannelListener()
	{
		public void onHandoverChannel(ISDBTPlayer player, int affiliation, int channel)
		{
			if (mOnHandoverChannelListener != null)
			{
				mOnHandoverChannelListener.onHandoverChannel(mDxbPlayer, affiliation, channel);
			}
		}
	};
	
	public interface OnHandoverChannelListener
	{
		void onHandoverChannel(DxbPlayer player, int affiliation, int channel);
	}

	public void setOnHandoverChannelListener(OnHandoverChannelListener listener)
	{
		mOnHandoverChannelListener = listener;
	}

	private OnHandoverChannelListener mOnHandoverChannelListener;

	private ISDBTPlayer.OnEWSListener ListenerOnEWS = new ISDBTPlayer.OnEWSListener()
	{
   		public void onEWS(ISDBTPlayer player, int bStatus, EWSData obj)
   		{
   			if(cOption.ews == true)
   			{	   				
				if (mOnEWSListener != null)
				{
	   				mOnEWSListener.onEWS(mDxbPlayer, bStatus, obj);
	   			}
   			}
   			else
   			{
   				DxbLog(I, "EWS option is disabled");
   			}
   		}
   	};

   	public interface OnEWSListener {
   		void onEWS(DxbPlayer player, int bStatus, EWSData obj);
   	}

   	public void setOnEWSListener(OnEWSListener listener) {
   		mOnEWSListener = listener;
   	}

   	private OnEWSListener mOnEWSListener;
   	
	private ISDBTPlayer.OnEPGUpdateListener ListenerOnEPGUpdate = new ISDBTPlayer.OnEPGUpdateListener()
	{
		public void onEPGUpdate(ISDBTPlayer player, int serviceID, int tableID)
		{
			if (mOnEPGUpdateListener != null)
			{
				if(mChannel[iCurrentChannelIndex].serviceID == serviceID)
				{
					mOnEPGUpdateListener.onEPGUpdate(mDxbPlayer, serviceID, tableID);
				}
			}
		}
	};
	
	public interface OnEPGUpdateListener
	{
		void onEPGUpdate(DxbPlayer player, int channelNumber, int serviceID);
	}

	public void setOnEPGUpdateListener(OnEPGUpdateListener listener)
	{
		mOnEPGUpdateListener = listener;
	}

	private OnEPGUpdateListener mOnEPGUpdateListener;

	private ISDBTPlayer.OnSCErrorListener ListenerOnSCError = new ISDBTPlayer.OnSCErrorListener()
	{
   		public void onSCErrorUpdate(ISDBTPlayer player, int bStatus)
   		{
			if(iSCStatus != bStatus)
	   		{
				iSCStatus = bStatus;
		
				setSCDisplay(bStatus, mChannel[iDefaultChannelIndex].serviceType);
					
				if (mOnSCErrorListener != null)
				{
	   				mOnSCErrorListener.onSCErrorUpdate(mDxbPlayer, bStatus);
	   			}
			}
   		}
   	};

   	public interface OnSCErrorListener {
   		void onSCErrorUpdate(DxbPlayer player, int bStatus);
   	}

   	public void setOnSCErrorListener(OnSCErrorListener listener) {
   		mOnSCErrorListener = listener;
   	}

   	private OnSCErrorListener mOnSCErrorListener;
   	
	private ISDBTPlayer.OnSCInfoListener ListenerOnSCInfo = new ISDBTPlayer.OnSCInfoListener()
	{
   		public void onSCInfoUpdate(ISDBTPlayer player, SCInfoData obj)
   		{
   			setSCInfo(obj);
   			
   			if (mOnSCInfoListener != null) {
   				mOnSCInfoListener.onSCInfoUpdate(mDxbPlayer, obj);
   			}
   		}
   	};

   	public interface OnSCInfoListener {
   		void onSCInfoUpdate(DxbPlayer player, SCInfoData obj);
   	}

   	public void setOnSCInfoListener(OnSCInfoListener listener) {
   		mOnSCInfoListener = listener;
   	}

   	private OnSCInfoListener mOnSCInfoListener;

	/***** autosearch *****/
	public interface OnAutoSearchInfoListener {
		void onAutoSearchInfoUpdate (DxbPlayer player, int ext1, int ext2);
	}
	public void setOnAutoSearchInfoListener(OnAutoSearchInfoListener listener)
	{
		mOnAutoSearchInfoListener = listener;
	}
	private OnAutoSearchInfoListener mOnAutoSearchInfoListener;

	public int getAutoSearchInfoStatus () {
		return mAutoSearchInfo.mAutoSearchStatus;
	}
	public int getAutoSearchInfoFullseg() {
		return mAutoSearchInfo.mAutoSearchFullSeg;
	}
	public int getAutoSearchInfoOneseg() {
		return mAutoSearchInfo.mAutoSearchOneSeg;
	}
	public int getAutoSearchInfoTotalSvc () {
		return mAutoSearchInfo.mAutoSearchTotalSvc;
	}

	private ISDBTPlayer.OnAutoSearchInfoListener ListenerOnAutoSearchInfo = new ISDBTPlayer.OnAutoSearchInfoListener ()
	{
		public void onAutoSearchInfoUpdate (ISDBTPlayer player, int ext1, int ext2, AutoSearchInfoData obj)
		{
			mAutoSearchInfo = obj;
			if (mOnAutoSearchInfoListener != null)
				mOnAutoSearchInfoListener.onAutoSearchInfoUpdate(mDxbPlayer, ext1, ext2);
		}
	};

	/************************************************************************************************************************
	 * 	Listener -----> End
	 ************************************************************************************************************************/

	public String getVideoInfo() {
		if (mVideoProgressive == 1) {
			return String.valueOf(mVideoWidth) + "x" + String.valueOf(mVideoHeight) + " [P]";
		} else {
			return String.valueOf(mVideoWidth) + "x" + String.valueOf(mVideoHeight) + " [I]";
		}
	}

	public int getFrameRate()
	{
		return mVideoFrameRate;
	}

	private class EventHandler extends Handler {
		public static final int EVENT_SET_CHANNEL						= 0;
		public static final int EVENT_SET_LOCALPLAY						= 1;
		public static final int EVENT_COMPLETE_SETCHANNEL				= 2;

		private int mRequestChID;
		private boolean mRunningThread;

		public EventHandler(Looper looper) {
			super(looper);
			mRunningThread = false;
		}

		@Override
		public void handleMessage(Message msg)
		{
			DxbLog(I, "EventHandler --> handleMessage(msg=" + msg + ")");
			
			switch (msg.what)
			{
				case EVENT_SET_CHANNEL:
					if(getServiceType() == msg.arg1)
					{
						resetChannel();
						if (msg.arg1 == 1)
						{
							setLocalPlay((String)msg.obj);
						}
						else
						{
							setChannel(msg.arg2);
						}
						
						if (mOnChannelChangeListener != null)
						{
							mOnChannelChangeListener.onChannelChange();
						}
					}
				break;

			case EVENT_COMPLETE_SETCHANNEL:
				//resumeSubtitle();
				if (msg.arg1 == 1)
				{
					if(iSCStatus != SCInfoData.SC_ERR_OK)
			   		{
						setSCDisplay(iSCStatus, mChannel[iDefaultChannelIndex].serviceType);
							
						if (mOnSCErrorListener != null)
						{
			   				mOnSCErrorListener.onSCErrorUpdate(mDxbPlayer, iSCStatus);
			   			}
					}
				}
				break;

			case EVENT_SET_LOCALPLAY:
				if (eLocalPlayingState != LOCAL_STATE.STOP)
				{
					sendMessageDelayed(obtainMessage(EVENT_SET_LOCALPLAY, msg.arg1, msg.arg2, msg.obj), 100);
				}
				else
				{
					eLocalPlayingState = LOCAL_STATE.PLAYING;
					sendMessage(obtainMessage(EVENT_SET_CHANNEL, msg.arg1, msg.arg2, msg.obj));
				}
				break;

			default:
				DxbLog(E, "Unknown message type " + msg.what);
				break;
			}
		}

		private void setLocalPlay(String fileName)
		{
			if (mPlayer != null)
			{
				Display display = ((WindowManager)mContext.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();

				Point view_size = new Point(0,0);
				Point real_size = new Point(0,0);
				
				display.getSize(view_size);
				display.getRealSize(real_size);
				
				mPlayer.setPlay(DxbStorage.getPath_Device() + "/" + fileName, real_size.x, real_size.y, view_size.x, view_size.y);

				isVideoPlay_update = false;
				
				Cursor	subt_cursor;
				subt_cursor	= getSubtitle_LanguageCode();
				if(subt_cursor != null)
				{	
					mSubtitleCount = subt_cursor.getCount();
					subt_cursor.close();
				
					if (mRenderTTX != null)
					{
						mRenderTTX.setTeletextInformation(getTeletext_Descriptor(1), getTeletext_Descriptor(2));
					}
				}
			}
			
			postUserEvent(EVENT_COMPLETE_SETCHANNEL, 0, 0, null);
		}

		private void setChannel(int id)
		{
			DxbLog(I, "setChannel(id = " + id + ")");
			
			mRequestChID = id;
		
			if (!mRunningThread)
			{
				mRunningThread = true;
				new Thread(new Runnable()
				{
					public void run()
					{
						int channelID;	
						int ret = 0;
						do
						{
							channelID = mRequestChID;
					
							if (mPlayer != null)
							{
								DxbChannelData ch = null, main_ch = null, sub_ch = null, suspect_ch = null;
								ArrayList<DxbChannelData> suspect_list;
								int main_ch_id = 0, sub_ch_id = 0;

								isDual_decoding = false;
								
								ch = getCurChannelData();
								
								if(cOption.dual_decode == true)
								{
									//find main_ch, sub_ch
									if (ch.iService_type != ISDBT_SECTION_1SEG)
									{
										suspect_list = mChannelManager.getSegChannels(ch.iChannel_number, ch.iRemocon_ID, ch.iNetwork_ID, ISDBT_SECTION_1SEG);
									}
									else
									{
										suspect_list = mChannelManager.getSegChannels(ch.iChannel_number, ch.iRemocon_ID, ch.iNetwork_ID, ISDBT_SECTION_FSEG);
									}
	
									if ((suspect_list != null) && (suspect_list.size() > 0)) 
									{
										int count = suspect_list.size();
										for (int i=0; i<count; i++)
										{
											if ((ch.iThreeDigitNumber%10) == ((suspect_list.get(i).iThreeDigitNumber)%10))
											{
												if((ch.iService_type == ISDBT_SECTION_1SEG)
													|| ((ch.iService_type != ISDBT_SECTION_1SEG) && (suspect_list.get(i).iPMT_PID >= 0x1FC8 && suspect_list.get(i).iPMT_PID <= 0x1FD0)))
												{
													suspect_ch = suspect_list.get(i);
													isDual_decoding	= true;
													break;
												}	
											}
										}
									}
								}
								
								if (ch.iService_type != ISDBT_SECTION_1SEG)
								{
									iDefaultChannelIndex = DUAL_DECODE_MAIN;
									main_ch = ch;
									sub_ch = suspect_ch;
								}
								else
								{
									iDefaultChannelIndex = DUAL_DECODE_SUB;
									main_ch = suspect_ch;
									sub_ch = ch;
								}

								if (main_ch != null)
								{
									main_ch_id = main_ch.iID;
								}

								if (sub_ch != null)
								{
									sub_ch_id = sub_ch.iID;
								}

								if((mChannel[DUAL_DECODE_MAIN].isSameChannel(main_ch) == false)	|| mChannel[DUAL_DECODE_SUB].isSameChannel(sub_ch) == false)
								{
									enableSubtitle(false);
												
 									isVideoPlay_update = false;
									
									//get display size
									Display display = ((WindowManager)mContext.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
									
									Point view_size = new Point(0,0);
									Point real_size = new Point(0,0);
									
									display.getSize(view_size);					
									display.getRealSize(real_size);					

									iCurrentChannelIndex = iDefaultChannelIndex;
									switch (cOption.seamless_change)
									{
										case 1: //Full-seg
											if(main_ch != null) {
												iCurrentChannelIndex = DUAL_DECODE_MAIN;
											}
										break;

										case 2: //1-seg
											if(sub_ch != null) {
												iCurrentChannelIndex = DUAL_DECODE_SUB;
											}
										break;
									}

									mPlayer.setChannel(main_ch_id, sub_ch_id, cOption.audio, cOption.dual_audio, real_size.x, real_size.y, view_size.x, view_size.y, iCurrentChannelIndex);
									mPlayer.setSubtitle(cOption.caption);
									mPlayer.setSuperImpose(cOption.super_impose);

									mChannel[DUAL_DECODE_MAIN].set(main_ch);
									mChannel[DUAL_DECODE_SUB].set(sub_ch);

									if(mChannel[iDefaultChannelIndex] != null && mChannel[iDefaultChannelIndex].preset == 1)
									{
										DxbChannelData cur_ch = mChannelManager.getDxbChannelData(mChannel[iDefaultChannelIndex].ID, -1);
										if( (cur_ch != null) && (cur_ch.iPreset == 0))
										{
											mChannel[iDefaultChannelIndex].set(cur_ch);
										}
									}

									if(mSQInfo != null)
									{
										if(isDual_decoding == true)
										{
											mSQInfo.setServiceType(SQInfoData.SQINFO_DUAL_DECODE);
										}
										else if(mChannel[iDefaultChannelIndex] != null && mChannel[iDefaultChannelIndex].serviceType != ISDBT_SECTION_1SEG)
										{
											mSQInfo.setServiceType(SQInfoData.SQINFO_FULL_SEG);
										}
										else
										{
											mSQInfo.setServiceType(SQInfoData.SQINFO_ONE_SEG);
										}

										mSQInfo.setSeamlessMode(cOption.seamless_change);			
									}

									if (getServiceType() == 0)
									{
										setTimeZone(cOption.time_zone_type, cOption.time_zone_index);
									}
								
									Cursor	subt_cursor	= getSubtitle_LanguageCode();
									if(subt_cursor != null)
									{
										mSubtitleCount = subt_cursor.getCount();
										subt_cursor.close();
										if (mRenderTTX != null)
										{
											mRenderTTX.setTeletextInformation(getTeletext_Descriptor(1), getTeletext_Descriptor(2));
										}
									}
									
									ret = 1;
								}
							}
						}
						while(mRequestChID != channelID);
												
						mRunningThread = false;
						postUserEvent(EVENT_COMPLETE_SETCHANNEL, ret, 0, null);
					}
				}).start();
			}
		}
	}

	private void postUserEvent(int what, int arg1, int arg2, Object obj) {
		if (mEventHandler != null) {
			mEventHandler.sendMessage(mEventHandler.obtainMessage(what, arg1, arg2, obj));
		}
	}

	public void setServiceType(int iType)
	{
		DxbLog(I, "setServiceType(iType=" + iType + ")");
		
		if (cChannelInfo.iTab != iType)
		{
			cChannelInfo.iTab = iType;
			updateChannelList();
			setUnlockParental(false);
		}
	}

	public void updateChannelList()
	{
		DxbLog(I, "updateChannelList()");
		
		cChannelInfo.iCount = getChannel_Count(cChannelInfo.iTab);
		if (cChannelInfo.iTab == 0)
		{
			cChannelInfo.iPosition = cChannelInfo.iCurrent_TV;
		}
		else
		{
			cChannelInfo.iPosition = 0;
			cChannelInfo.iCurrent_File	= 0;
		}
	}
	
	public int getServiceType()
	{
		//DxbLog(I, "getServiceType() --> cChannelInfo.iTab = " + cChannelInfo.iTab);
		
		return cChannelInfo.iTab;
	}

	public void changeChannle_LogicalChannel(int iLogical_Channel)
	{
		DxbLog(I, "changeChannle_LogicalChannel(iLogical_Channel=" + iLogical_Channel + ")");
		
		ArrayList<DxbChannelData>	cur_Data	= null;
		 
		if(cChannelInfo.iTab == 0)
		{
			cur_Data	= tvChannelData;
		}

		if(cur_Data != null)
		{
			int iCount	= cur_Data.size();
			for(int i=0 ; i<iCount ; i++)
			{
				DxbChannelData	p	= cur_Data.get(i);
				
				if(p.iLogical_channel == iLogical_Channel)
				{
					if(cChannelInfo.iTab == 0)
					{
						cChannelInfo.iCurrent_TV	= i;
					}
					setChannel();
					
					return;
				}
			}
		}
	}

	public boolean setChannel()
	{
		if (cChannelInfo.iCount > 0)
		{
/*
			SCHEDULER	Scheduler_Info	= cSCHEDULER;
			if ((Scheduler_Info.Channel_Type >= 0) && (Scheduler_Info.Channel_Index >= 0)) {
				cChannelInfo.iTab	= Scheduler_Info.Channel_Type;
				if(cChannelInfo.iTab == 0) {
					cChannelInfo.iCurrent_TV	= Scheduler_Info.Channel_Index;
				}

				Scheduler_Info.Channel_Type		= -1;
				Scheduler_Info.Channel_Index	= -1;
			}
*/
			if (cChannelInfo.iTab == 0)
			{
				return setChannel(cChannelInfo.iCurrent_TV);
			}
			else if (cChannelInfo.iTab == 1)
			{
				return setChannel(cChannelInfo.iCurrent_File);
			}
		}
		
		return false;
	}

	public boolean setChannelUp()
	{
		if (cChannelInfo.iCount > 0)
		{
			if (cChannelInfo.iPosition > 0)
			{
				return setChannel(cChannelInfo.iPosition - 1);
			}
			else
			{
				return setChannel(cChannelInfo.iCount - 1);
			}
		}
		return false;
	}

	public boolean setChannelDown()
	{
		DxbLog(I, "setChannelDown()");
		
		if(cChannelInfo.iCount > 0)
		{
			return setChannel((cChannelInfo.iPosition + 1) % cChannelInfo.iCount);
		}
		
		return false;
	}

	public boolean setChannel(int iChannel)
	{
		DxbLog(I, "setChannel(iChannel = " + iChannel + ")");
		
		if(eState == STATE.SCAN)
		{
			return false;
		}
		
		if(cChannelInfo.iCount > 0)
		{
			if(		(eLocalPlayingState != LOCAL_STATE.STOP)
				&&	(cChannelInfo.iTab == 1)
				&&	(cChannelInfo.iCurrent_File == iChannel)
			)
			{
				return false;
			}
			
			mEventHandler.removeMessages(EventHandler.EVENT_SET_LOCALPLAY);

			if (iChannel >= 0)
			{
				if(cChannelInfo.iPosition != iChannel)
				{
					setUnlockParental(false);
				}
				cChannelInfo.iPosition = iChannel;
			}
			
			if(cChannelInfo.iTab <= 1)
			{
				if(cChannelInfo.iTab == 0)
				{
					if(cChannelInfo.iCurrent_TV != iChannel)
					{
						setUnlockParental(false);
					}
					cChannelInfo.iCurrent_TV = iChannel;

					mChannel[DUAL_DECODE_MAIN].filePlay = 0;
					mChannel[DUAL_DECODE_SUB].filePlay = 0;
					mDB.putPosition(0, cChannelInfo.iCurrent_TV);
					DxbLog(D, "setChannel : TV Start - " + iChannel);
				}
				
				setCurrent_ChannelData();

				//resetChannel();
				//setChannel(mChannel.ID);
				if (mPlayer != null)
				{
					if (cChannelInfo.iTab == 1)
					{
						cChannelInfo.iCurrent_File = iChannel;
						mChannel[DUAL_DECODE_MAIN].filePlay = 1;
						mChannel[DUAL_DECODE_SUB].filePlay = 1;
						if (eLocalPlayingState == LOCAL_STATE.STOP)
						{
							DxbLog(D, "setChannel : File Play Start - " + iChannel);
							eLocalPlayingState = LOCAL_STATE.PLAYING;
						}
						else
						{
							DxbLog(D, "setChannel : File Play Stop & Start - " + iChannel);
							mPlayer.setPlayStop();
							mEventHandler.sendMessage(
									mEventHandler.obtainMessage(
											EventHandler.EVENT_SET_LOCALPLAY,
											cChannelInfo.iTab,
											mChannel[iDefaultChannelIndex].ID,
											mChannel[iDefaultChannelIndex].channelName
									)
							);
							return true;
						}
					}
					
					mEventHandler.sendMessage(
							mEventHandler.obtainMessage(
									EventHandler.EVENT_SET_CHANNEL,
									cChannelInfo.iTab,
									mChannel[iDefaultChannelIndex].ID,
									mChannel[iDefaultChannelIndex].channelName
							)
					);

					return true;
				}
			}
		}
		else
		{
			eState = STATE.GENERAL;
		}
		
		return false;
	}

	public boolean setBackScanChannel()
	{
		if (cChannelInfo.iCount > 0)
		{
			if (cChannelInfo.iTab == 0)
			{
				ArrayList<DxbChannelData> ch_list = getChannelList(0);				
				DxbChannelData ch = getCurChannelData();

				if(	(mPlayer != null)
						&&	(ch != null)
						&&	(ch.iService_type == ISDBT_SECTION_1SEG) )
				
				{
					ArrayList<DxbChannelData> suspect_list;
					DxbChannelData suspect_ch = null;

					suspect_list = mChannelManager.getSegChannels(ch.iChannel_number, ch.iRemocon_ID, ch.iNetwork_ID, ISDBT_SECTION_FSEG);
					if ((suspect_list != null) && (suspect_list.size() > 0)) 
					{
						int count = suspect_list.size();
						for (int i=0; i<count; i++)
						{
							if ((ch.iThreeDigitNumber%10) == ((suspect_list.get(i).iThreeDigitNumber)%10))
							{
								suspect_ch = suspect_list.get(i);
								break;
							}
						}
					}
				
					if (suspect_ch != null)
					{
						int count = ch_list.size();
						for (int i=0; i<count; i++)
						{
							if (suspect_ch.iID == ch_list.get(i).iID)
							{
								cChannelInfo.iCurrent_TV = i;
								
								resetChannel();
								setChannel();
								
								return true;
							}
						}
					}
				}
			}
		}
		
		return false;
	}
	
	public boolean setAutoSearchChannel (int fullseg_row, int oneseg_row)
	{
		ArrayList<DxbChannelData> ch_list;
		int channel_count, i;
		int	search_row;

		DxbLog(D, "In setAutoSearchChannel," + fullseg_row + "," + "onesg_row");

		if (cChannelInfo.iCount > 0) {
			if (cChannelInfo.iTab == 0) {
				ch_list = getChannelList(0);
				channel_count = ch_list.size();
				if((m_isdbt_feature&ISDBT_SINFO_PROFILE_A) == ISDBT_SINFO_PROFILE_A) {
					if(fullseg_row != 0) search_row = fullseg_row;
					else search_row = 0;
				} else
					search_row = oneseg_row;
				if (search_row == 0) {
					DxbLog(D, "In setAutoSearchChannel, invalid row id (" + fullseg_row + "," + oneseg_row + ")");
					return false;
				}
				for(i=0; i < channel_count; i++) {
					if (ch_list.get(i).iID == search_row) {
						DxbLog(D,"In setAutoSearchChannel, start ch(" + i + "," + search_row + ")");
						setChannel(i);
						return true;
					}
				}
			}
		}
		return false;
	}

	public DxbChannelData getChannel(int ID, int channelNumber)
	{
		if(mChannelManager != null)
		{
			return mChannelManager.getDxbChannelData(ID, channelNumber);
		}

		return null;
	}

	public ArrayList getChannels(int serviceType)
	{
		if(mChannelManager != null)
		{
			return mChannelManager.getSegChannels(serviceType);
		}

		return null;
	}
	
	public int getChannelCount(int type) {
		if (type == 0)
			return cChannelInfo.iCount_TV;
		return 0;
	}

	public int getChannelCount()
	{
		DxbLog(I, "getChannelCount() --> iCount = " + cChannelInfo.iCount);
		
		return cChannelInfo.iCount;
	}

	public int getChannelPosition()
	{
		return cChannelInfo.iPosition;
	}

	public int getChannelPosition(int i)
	{
		if (i == 0)
		{
			return cChannelInfo.iPosition;
		}
		else if (i == 1)
		{
			return cChannelInfo.iCurrent_TV;
		}
		else
		{
			return cChannelInfo.iCurrent_File;
		}
	}

	public int getSubtitleCount(int i) {
		if (i == 0) {
			return mSubtitleCount;
		} else if (mRenderTTX != null) {
			return mRenderTTX.getSubtitleCount();
		}
		return 0;
	}

	public void resetChannelInfo()
	{
		DxbLog(I, "resetChannelInfo()");
		
		cChannelInfo.iCurrent_TV	= 0;
		cChannelInfo.iCount			= 0;
		
		if(getServiceType() == 0)
		{
			cChannelInfo.iCount_TV		= 0;
		}
	}

	public void resetChannel()
	{
		DxbLog(I, "resetChannel(ePLAYER=" + ePLAYER + ")");
		
		if (ePLAYER == PLAYER.ISDBT)
		{
			if (cOption.audio > 0)
			{
				cOption.audio	= 1;
			}
/*			
			if (cOption.caption > 0)
			{
				cOption.caption	= 1;
			}
*/			
//			DxbView_Teletext.stopSubtitle();
		}
	}

	private boolean isVideoPlay_output	= false;
	private boolean isVideoPlay_update	= false;
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

	public boolean isVideoOutput()
	{
		if(isVideoPlay_output)
		{
			return true;
		}
		
		return false;
	}

	public boolean isVideoUpdate()
	{
		if(isVideoPlay_update)
		{
			return true;
		}
		
		return false;
	}

	public String changeAudio()
	{
		String	return_audio	= null;
		Cursor audio_cursor	= getAudio_LanguageCode();

		if(audio_cursor != null)
		{
			int iCount_Language_Audio	= audio_cursor.getCount();
			if (iCount_Language_Audio > 0)
			{
				cOption.audio++;
				cOption.audio %= iCount_Language_Audio;
				audio_cursor.moveToPosition(cOption.audio);
				setAudio(cOption.audio);
				return_audio = "Audio" + cOption.audio;
			}
			
			audio_cursor.close();
			}
		
		return return_audio;
	}

	public String changeSubtitle()
	{
		String	return_Subtitle	= null;
		Cursor cursorSubtitle	= getSubtitle_LanguageCode();

		Log.e("--->","changeSubtitle");
		
		if(cursorSubtitle != null)
		{
			mSubtitleCount = cursorSubtitle.getCount();
			int iTtxSubtitle = (mRenderTTX != null) ? mRenderTTX.getSubtitleCount() : 0;
			
			if (mSubtitleCount + iTtxSubtitle > 0)
			{
				cOption.caption++;
				cOption.caption %= (mSubtitleCount + iTtxSubtitle + 1);

				playSubtitle(_ON_);
				
				if (cOption.caption == 0)
				{
				}
				else if(cOption.caption <= mSubtitleCount) // subtitle
				{
					cursorSubtitle.moveToPosition(cOption.caption-1);
					return_Subtitle	= "Subtitle" + cOption.caption;
				}
				else if (iTtxSubtitle > 0) // teletext subtitle
				{
					int i = cOption.caption - mSubtitleCount - 1;
					return_Subtitle	= mRenderTTX.getTTX_LanguageCode(i);
				}
			}
			
			cursorSubtitle.close();
		}
		
		return return_Subtitle;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	// EPG
	/////////////////////////////////////////////////////////////////////////////////////////
	public boolean openEPGDB() {
		if(mChannelManager != null) {
			return mChannelManager.openEPGDB();
		}
		return false;
	}
		
	public ArrayList<DxbEPGData> getEPG_PF()
	{
		DxbLog(I, "getEPG_PF()");
		if (mChannelManager != null)
		{
			return mChannelManager.getEPG_PF(mChannel[iDefaultChannelIndex].serviceID, mChannel[iDefaultChannelIndex].channelNumber, mChannel[iDefaultChannelIndex].countryCode);
		}
		
		return null;
	}

	public Cursor getEPG_Schedule()
	{
		return getEPG_Schedule(mChannel[iDefaultChannelIndex].serviceID, mChannel[iDefaultChannelIndex].channelNumber, mChannel[iDefaultChannelIndex].countryCode, -1);
	}

	public Cursor getEPG_Schedule(int date)
	{
		return getEPG_Schedule(mChannel[iDefaultChannelIndex].serviceID, mChannel[iDefaultChannelIndex].channelNumber, mChannel[iDefaultChannelIndex].countryCode, date);
	}
	
	public Cursor getEPG_Schedule(int serviceID, int channelNumber, int countryCode)
	{
		return getEPG_Schedule(serviceID, channelNumber, countryCode, -1);
	}	

	public Cursor getEPG_Schedule(int serviceID, int channelNumber, int countryCode, int date)
	{
		DxbLog(I, "getEPG_Schedule(" + date + ")");
		
		if(mChannelManager != null)
		{
			if(mChannel[iDefaultChannelIndex].serviceType == ISDBT_SECTION_1SEG)
			{
				return mChannelManager.getEPG_PF_Information(serviceID, channelNumber, countryCode);
			}
			else
			{
				if(date < 0)	
				{
					return mChannelManager.getEPG_Sche_Information(serviceID, channelNumber, countryCode);
				}
				else
				{
					return mChannelManager.getEPG_Sche_Information(serviceID, channelNumber, countryCode, date);
				}
			}
		}
		else
		{
			return null;
		}
	}	

	public Cursor getEPG_Schedule(int serviceID, int channelNumber, int countryCode, int serviceType, int mjd, int time, int prev_event_time)
	{
		DxbLog(I, "getEPG_Schedule(" + mjd + ", " + time + ")");
		
		if(mChannelManager != null)
		{
			if(serviceType == ISDBT_SECTION_1SEG)
			{
				return mChannelManager.getEPG_PF_Information(serviceID, channelNumber, countryCode, mjd, time, prev_event_time);
			}
			else
			{
				return mChannelManager.getEPG_Sche_Information(serviceID, channelNumber, countryCode, mjd, time, prev_event_time);
			}
		}
		else
		{
			return null;
		}
	}	
				
	public Cursor getEPG_Schedule_LastEvent(int serviceID, int channelNumber, int countryCode, int serviceType)
	{
		DxbLog(I, "getEPG_Schedule_LastEvent()");
		
		if(mChannelManager != null)
		{
			if(serviceType == ISDBT_SECTION_1SEG)
			{
				return mChannelManager.getEPG_PF_LastEvent(serviceID, channelNumber, countryCode);
			}
			else
			{
				return mChannelManager.getEPG_Sche_LastEvent(serviceID, channelNumber, countryCode);
			}
		}
		else
		{
			return null;
		}				
	}
	
	public boolean isCurrEPG()
	{
		if(tvEPGData != null && tvEPGData.size() > 0)
		{
			DxbEPGData epgData = tvEPGData.get(0);
			if(epgData.iSectionNumber == 0)
			{
				return true;
			}
		}

		return false;
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
		if (isCurrEPG())
		{
			DxbEPGData epgData = tvEPGData.get(0);
			return epgData.iStartHH * 60 + epgData.iStartMM;	
		}
		
		return 0;
	}

	public int getCurrEPG_DurationTime()
	{
		if (isCurrEPG())
		{
			DxbEPGData epgData = tvEPGData.get(0);
			return epgData.iDurationHH * 60 + epgData.iDurationMM;	
		}
		return 0;
	}

	public String getCurrEPG_Title()
	{
		String title = null;

		if (isCurrEPG())
		{
			DxbEPGData epgData = tvEPGData.get(0);
			title = epgData.sEvtName;
			if(title != null && title.length() > 41)
			{
				return title.substring(0, 36) + " ... ";
			}
		}
		return title;
	}

	public int getCurrentEPG_Rating()
	{
		int	iRating	= 0;
		
		if(ePLAYER != PLAYER.ISDBT)
		{
			if (isCurrEPG())
			{	
				DxbEPGData epgData = tvEPGData.get(0);
				iRating	= epgData.iRating;
			}
		}
		
		return iRating;
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
			//isVideoPlay_output	= false;
			if(mOnVideoOutputListener != null)
			{
				mOnVideoOutputListener.onVideoOutput(mDxbPlayer);
			}
		}
	}
	
	public boolean isUnlockParental()
	{
		return bUnlockParental_state;
	}
	
	public boolean isNextEPG()
	{
		if(tvEPGData != null && tvEPGData.size() > 1)
		{
			DxbEPGData epgData = tvEPGData.get(1);
			if(epgData.iSectionNumber == 1)
			{
				return true;
			}
		}

		return false;
	}
	
	public int getNextEPG_StartTime()
	{
		if (isNextEPG())
		{
			DxbEPGData epgData = tvEPGData.get(1);
			return epgData.iStartHH * 60 + epgData.iStartMM;	
		}

		return 0;
	}

	public int getNextEPG_DurationTime()
	{
		if (isNextEPG())
		{
			DxbEPGData epgData = tvEPGData.get(1);
			return epgData.iDurationHH * 60 + epgData.iDurationMM;	
		}
		return 0;
	}

	public String getNextEPG_Title()
	{
		String title = null;

		if (isNextEPG())
		{
			DxbEPGData epgData = tvEPGData.get(1);
			title = epgData.sEvtName;
			if(title != null && title.length() > 41)
			{
				return title.substring(0, 36) + " ... ";
			}
		}
		return title;
	}

	public int getNextEPG_Rating()
	{
		int	iRating	= 0;
		
		if(isNextEPG())
		{
			DxbEPGData epgData = tvEPGData.get(1);
			iRating	= epgData.iRating;
		}
		
		return iRating;
	}

	public int getTTX_initPage() {
		if (mRenderTTX != null) {
			return mRenderTTX.getTTX_initPage();
		}
		return 0;
	}
	
	public String getTTX_LanguageCode(int i) {
		if (mRenderTTX != null) {
			return mRenderTTX.getTTX_LanguageCode(i);
		}
		return null;
	}
	
	public static final int PROP_NAME_MAX = 31;
	public static final int PROP_VALUE_MAX = 91;

	public static String property_get(String key) {
		if (key.length() > PROP_NAME_MAX) {
			throw new IllegalArgumentException("key.length > " + PROP_NAME_MAX);
		}
		return ISDBTPlayer.property_get(key);
	}

	public static void property_set(String key, String val) {
		if (key.length() > PROP_NAME_MAX) {
			throw new IllegalArgumentException("key.length > " + PROP_NAME_MAX);
		}
		if (val != null && val.length() > PROP_VALUE_MAX) {
			throw new IllegalArgumentException("val.length > " + PROP_VALUE_MAX);
		}
		ISDBTPlayer.property_set(key, val);
	}
	public void setFilePlay(boolean v) {
		if(v == true) {
			mChannel[DUAL_DECODE_MAIN].filePlay = 1;
			mChannel[DUAL_DECODE_SUB].filePlay = 1;
		}
		else {
			mChannel[DUAL_DECODE_MAIN].filePlay = 0;
			mChannel[DUAL_DECODE_SUB].filePlay = 0;
		}
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
		String TAG = "[[[DxbPlayer]]]";
		
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
					Log.d(TAG, mString);
				break;
			}
		}
	}	
}
