package com.telechips.android.atsc.samples;

import android.text.format.Time;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.os.Bundle;
import android.os.Handler;
import android.os.SystemProperties;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.Display;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.view.View.OnClickListener;
import android.view.View.OnFocusChangeListener;
import android.view.View.OnTouchListener;
import android.view.View.OnKeyListener;
import android.widget.AdapterView;
import android.widget.Button;
import android.graphics.*;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TabHost;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.TabHost.OnTabChangeListener;
import android.widget.AbsoluteLayout;
import android.view.Window;
import android.content.Context;
import android.widget.ArrayAdapter;

import com.telechips.android.atsc.*;

@SuppressWarnings("deprecation")
public class SampleATSCPlayer extends Activity
{
	private static final String TAG = "[[[SampleATSCPlayer]]]";

	/*******************************************************************************
	 * Check State
	 *******************************************************************************/
		// Dxb_View State
		public enum DXB_STATE_VIEW
		{
			LIST,
			FULL,
			EPG,
			NULL
		}
		private DXB_STATE_VIEW	eState_View			= DXB_STATE_VIEW.NULL;
		
		// Dxb_Player State
		public enum DXB_STATE_PLAYER
		{
			GENERAL,
			UI_PAUSE,
			NULL
		}
		public static DXB_STATE_PLAYER	eState_Player		= DXB_STATE_PLAYER.NULL;

		// Dxb_Player Life Cycle
		public enum DXB_LIFE_CYCLE
		{
			ON_CREATE,
			ON_START,
			ON_RESUME,
			ON_PAUSE,
			ON_STOP,
			ON_DESTROY,
			NULL
		}
		private static DXB_LIFE_CYCLE	eCycle_Life	= DXB_LIFE_CYCLE.ON_DESTROY;
	/*******************************************************************************/
		
	/*******************************************************************************
	 * Global Value
	 *******************************************************************************/
		// System value
		Handler mHandler_Main = new Handler();
		
		private int	mDisplayWidth;
		private int mDisplayHeight;
		private boolean mUseScreenMode;
		
		// Signal
		WindowManager mWindowManager;
		WindowManager.LayoutParams mWindowParams;
		ImageView	mImage_Signal;
		private int mSignalGauge = 0;

		boolean isShown_Signal	= false;

		int	level_Signal		= 4;
		int	count_Signal		= 0;

		
		private int[] rssIcon = {	R.drawable.dxb_portable_indicator_rssi_icon_00,
									R.drawable.dxb_portable_indicator_rssi_icon_01,
									R.drawable.dxb_portable_indicator_rssi_icon_02,
									R.drawable.dxb_portable_indicator_rssi_icon_03,
									R.drawable.dxb_portable_indicator_rssi_icon_04 };
		// Section
		int sectionID	= -1;
		ImageView imageIndicator;
		
		TextView timeIndicator;
		
		//closed caption
		private static ImageView	closedcaption_Display;
		private static Rect mRect;
		private static Bitmap	mBitmap;
		private static Canvas	mCanvas;
		private static Paint	mPaint;

		private static boolean closedcaptioncleardone = false;

		static int giTargetDisplayWidth = 1280;
		static int giTargetDisplayHeight = 200; //720;
		
		// coordinates
		private static final int COORDINATES_X = 0x00;
		private static final int COORDINATES_Y = 0x01;

		private static final int MODULATION_VSB = 8;
		private static final int MODULATION_QAM = 256;

		private static final int LISTCENTER_Y = 240;
	
		// SurfaceView
		private SurfaceView mSurfaceView;
		private SurfaceHolder mSurfaceHolder;
	
		// Service value
		private Cursor mChannelsCursor = null;
		private int mTotalChannel_TvCount = 0;
		private int mTotalChannel_RadioCount = 0;
		private int mTotalChannel_CurrentCount = 0;
		private int current_cursor = 0;
		
		// Up/Down
		private static final int CH_UP	= 0;
		private static final int CH_DOWN = 1;
	/**********************************************************************************/
		// Toast
		Toast mToast = null;
		
	/*******************************************************************************
	 * ListView Value
	 *******************************************************************************/
		// List View
		public static TabHost	tabHost;
		public static ListView mListView;
		public static View	View_List;
		DxbAdapter_Service madapter;
		
	/*******************************************************************************
	 * FulltView Value
	 *******************************************************************************/
		// Full View
		ViewGroup fullViewGroup;
		public static View	View_Full;
		
		// Title
		TextView fullTitle;
		
		// Screen size
		private static final int SCREENMODE_LETTERBOX	= 0;
		private static final int SCREENMODE_PANSCAN	= 1;
		private static final int SCREENMODE_FULL	= 2;
		
		int	 eState_ScreenSize	= SCREENMODE_FULL;
		
		// Change Channel
		Button mChannelUpBtn;
		Button mChannelDownBtn;
		
		// Menu
		int		iState_Menu	= 1;
		
		public static final int	MENU_NULL		= -1;
		public static final int	MENU_CHANNEL	= 1;
		public static final int	MENU_EPG		= 2;
		public static final int	MENU_SCREEN	= 3;
		public static final int	MENU_OPTION	= 4;
		
		Button	fullButton_menu1;
		Button	fullButton_menu2;
		Button	fullButton_menu3;
		Button	fullButton_menu4;
		
		ImageView	fullIcon1;
		ImageView	fullIcon2;
		ImageView	fullIcon3;
		ImageView	fullIcon4;

		TextView	fullText1;
		TextView	fullText2;
		TextView	fullText3;
		TextView	fullText4;

		private static AlertDialog mDialog;
		private final int ALERT_DIALOG_EXIT_ID = 0;
	/**********************************************************************************/

	/*******************************************************************************
	 * EPG Value
	 *******************************************************************************/
		static TextView	epgTextView_Current;
		static Button	epgButton_Prev, epgButton_Next;

		public static int	epgUpdate_day = 0;
		public static int	mepgFirstday= 0, mepgLastday = 0;
		private static boolean 	mMatchCase = false;
		private static boolean epgupdatedone = false;

		static ListView	mepgListView;

		public static View	View_EPG;
		private Cursor mepgCursor = null, nextEPG =null;
		private  int	currentFocus	= -1;	// 0-prev, 1-next, 2-list
	/**********************************************************************************/
	
	/*******************************************************************************
	 * Option Value
	 *******************************************************************************/
		public static final int	MASK_OPTIONS_AREA_CODE			= 0x0001;
		public static final int	MASK_OPTIONS_PARENTAL_RATING	= 0x0002;
		public static final int MASK_OPTIONS_VIDEO			= 0x0004;
		public static final int MASK_OPTIONS_AUDIO			= 0x0008;
		public static final int MASK_OPTIONS_DUAL_AUDIO		= 0x0010;
		public static final int MASK_OPTIONS_SUBTITLE			= 0x0020;
		public static final int MASK_OPTIONS_CLOSEDCAPTION	= 0x0030;
		public static final int MASK_OPTIONS_CAPTIONSVCNUM	= 0x0040;
		public static final int MASK_OPTIONS_INPUT			= 0x0050;
		public static final int MASK_OPTIONS_AUDIOPREF		= 0x0060;
		public static final int MASK_OPTIONS_ASPECTRATIO		= 0x0080;
		public static final int MASK_OPTIONS_CAPTION			= 0x0090;
		public static final int MASK_OPTIONS_SUPER_IMPOSE	= 0x0100;

		public static final String KEY_AREA_CODE_SELECT			= "area_code_select";
		public static final String KEY_PARENTAL_RATING_SELECT = "parental_rating_select";
		public static final String KEY_VIDEO_SELECT = "video_select";
		public static final String KEY_AUDIO_SELECT = "audio_select";
		public static final String KEY_DUAL_AUDIO_SELECT = "dual_audio_select";
		public static final String KEY_SUBTITLE_SELECT = "subtitle_select";
		public static final String KEY_CLOSEDCAPTION_SELECT = "closedcaption_select";
		public static final String  KEY_CAPTIONSVCNUM_SELECT = "closedcaption_svcnum";
		public static final String KEY_INPUT_SELECT = "input_select";
		public static final String KEY_SCAN_SELECT = "scan_select";
		public static final String KEY_AUDIO_PREF_SELECT = "audio_preference";
		public static final String KEY_ASPECT_RATIO_SELECT = "aspect_ratio";
		public static final String KEY_CAPTION_SELECT = "caption_select";		
		public static final String KEY_SUPER_IMPOSE_SELECT = "super_impose_select";
		
		public static final String KEY_SHAREDPREF_NAME = "com.telechips.android.atsc.samples_preferences";

		// Copied from MediaPlaybackService in the Music Player app. Should be
		// public, but isn't.
		private static final String SERVICECMD = "com.android.music.musicservicecommand";
		private static final String CMDNAME    = "command";
		private static final String CMDPAUSE   = "pause";
	/**********************************************************************************/

	/*******************************************************************************
	 * POPUP
	 *******************************************************************************/
		DxbPlayer_Popup mNoSignalPopup = null;
		public static int NoSignalEnable = 0;

		DxbPlayer_Popup mNoServicePopup = null;
		public static int NoServiceEnable = 0;

		DxbPlayer_Popup mScrambledPopup = null;
		public static int ScrambledEnable = 0;
		
		private static boolean mChannelChanging = false;
		//FullView AudioPreference
		public int mSelectedAP = -1;
		public int selectedButton = -1;

		//FullView AspectRatio
		public int mSelectedARatio = -1;

		public static boolean GloballyPopupEnabled = false;
	/**********************************************************************************/

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		Log.d(TAG, "================> onCreate");
		
		super.onCreate(savedInstanceState);
		eCycle_Life = DXB_LIFE_CYCLE.ON_CREATE;

		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN | 0x80000000, WindowManager.LayoutParams.FLAG_FULLSCREEN | 0x80000000);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		
		setContentView(R.layout.main);

		initSystemSet();
		initView();
		initSignal();
		DxbOptions_ATSC.ScanIsPerformed = 0;

		if(DxbPlayer_Control.mChannelManager == null)
		{
			DxbPlayer_Control.mChannelManager = new ChannelManager(this);
			DxbPlayer_Control.mChannelManager.open();
		}
		
		Dxb_setDefaultValue();
		
		if(DxbPlayer_Control.mDB == null)
		{
			DxbPlayer_Control.mDB = new DxbDB_Setting(this).open();
		}
		if(DxbPlayer_Control.mDB != null)
		{
			DxbPlayer_Control.tvPosition = DxbPlayer_Control.mDB.getPosition(0);
			DxbPlayer_Control.radioPosition = DxbPlayer_Control.mDB.getPosition(1);
			DxbPlayer_Control.options_aspect_ratio = DxbPlayer_Control.mDB.getAspectRatio();
			eState_ScreenSize = DxbPlayer_Control.options_aspect_ratio;
		}
		else
		{
			Log.d(TAG, "not open DB!!!");
		}
	}
	
	@Override
	public void onStart()
	{
		Log.d(TAG, "================> onStart");
		super.onStart();
		eCycle_Life = DXB_LIFE_CYCLE.ON_START;

		if(DxbPlayer_Control.intentSubActivity == null)
		{
			if(DxbPlayer_Control.mPlayer == null)
			{
				int nCount = 0;
				while(SystemProperties.get("tcc.dxb.running", "0").equals("1") == true)
				{
					try
					{
						nCount++;
						Thread.sleep(1000);
					}
					catch(InterruptedException e) 
					{
						Log.d(TAG, "sleep fail!");
					}
					if(nCount >= 5)
						break;
				}
			}
			else
			{
				while(DxbPlayer_Control.mPlayer != null)
				{
					try
					{
						Thread.sleep(100);
					}
					catch(InterruptedException e) 
					{
						Log.d(TAG, "sleep fail!");
					}
				}
			}
		}
	}
	
	@Override
	protected void onResume()
	{
		Log.d(TAG, "================> onResume");
		
		super.onResume();
		eCycle_Life = DXB_LIFE_CYCLE.ON_RESUME;
		NoSignalEnable = 0;
		NoServiceEnable = 0;
		ScrambledEnable = 0;

		mUseScreenMode = SystemProperties.get("tcc.solution.use.screenmode", "0").equals("1");
		/*In case broadcasting, always use screenmode.
		 */
		SystemProperties.set("tcc.solution.use.screenmode", "1");

		if(SystemProperties.getBoolean("tcc.video.vsync.support", false) == true)
		{
			// deinterlace property setting is temporary.
			SystemProperties.set("tcc.video.deinterlace.support", "1");
			SystemProperties.set("tcc.video.vsync.enable", "1");
		}

		if(mSurfaceHolder == null)
		{
			initSurfaceView();
		}

 		Intent i = new Intent(SERVICECMD);
		i.putExtra(CMDNAME, CMDPAUSE);
		sendBroadcast(i);
		
		if( DxbPlayer_Control.mChannelManager == null )
		{
			DxbPlayer_Control.mChannelManager = new ChannelManager(this);
			DxbPlayer_Control.mChannelManager.open();
		}
		
		if(DxbPlayer_Control.mDB == null)
		{
			DxbPlayer_Control.mDB = new DxbDB_Setting(this).open();
		}

		int	Mask_Options	= DxbPlayer_Control.getMask_Options();
		if(Mask_Options > 0)
		{
			SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(this);
			String list_value;
			
			if((Mask_Options & MASK_OPTIONS_PARENTAL_RATING) > 0)
			{
				list_value = pref.getString(KEY_PARENTAL_RATING_SELECT, "");
				if(list_value != "")
				{
					DxbPlayer_Control.options_password = list_value;
				}
			}
			
			if((Mask_Options & MASK_OPTIONS_VIDEO) > 0)
			{
				list_value= pref.getString(KEY_VIDEO_SELECT, "");
				if(list_value != "")
				{
					DxbPlayer_Control.options_video	= Integer.parseInt(list_value);
				}
			}
			
			if((Mask_Options & MASK_OPTIONS_AUDIO) > 0)
			{
				list_value= pref.getString(KEY_AUDIO_SELECT, "");
				if(list_value != "")
				{
					DxbPlayer_Control.options_audio	= Integer.parseInt(list_value);
				}
			}
			
			if((Mask_Options & MASK_OPTIONS_DUAL_AUDIO) > 0)
			{
				list_value= pref.getString(KEY_DUAL_AUDIO_SELECT, "");
				if(list_value != "")
				{
					DxbPlayer_Control.options_dual_audio = Integer.parseInt(list_value);
				}
			}
			
			if((Mask_Options & MASK_OPTIONS_SUBTITLE) > 0)
			{
				list_value	= pref.getString(KEY_SUBTITLE_SELECT, "");
				if(list_value != "")
				{
					DxbPlayer_Control.options_subtitle = Integer.parseInt(list_value);
				}
			}
			
			if((Mask_Options & MASK_OPTIONS_CLOSEDCAPTION) > 0)
			{
				list_value	= pref.getString(KEY_CLOSEDCAPTION_SELECT, "");
				if(list_value != "")
				{
					if(DxbPlayer_Control.options_closedcaption != Integer.parseInt(list_value))
					{
					DxbPlayer_Control.options_closedcaption = Integer.parseInt(list_value);
						DxbPlayer_Control.enableCC(DxbPlayer_Control.options_closedcaption);	//off(0)
					}
				}
			}
			
			if((Mask_Options & MASK_OPTIONS_CAPTIONSVCNUM) > 0)
			{
				list_value	= pref.getString(KEY_CAPTIONSVCNUM_SELECT, "");
				if(list_value != "")
				{
					if(DxbPlayer_Control.options_captionsvcnum != Integer.parseInt(list_value))
					{
						DxbPlayer_Control.options_captionsvcnum = Integer.parseInt(list_value);
						DxbPlayer_Control.setCCServiceNum(DxbPlayer_Control.options_captionsvcnum);	
					}
				}
			}
			
			if((Mask_Options & MASK_OPTIONS_INPUT) > 0)
			{
				list_value	= pref.getString(KEY_INPUT_SELECT, "");
				if(list_value != "")
				{
					DxbPlayer_Control.options_input = Integer.parseInt(list_value);
				}
			}
			
			if((Mask_Options & MASK_OPTIONS_AUDIOPREF) > 0)
			{
				list_value	= pref.getString(KEY_AUDIO_PREF_SELECT, "");
				if(list_value != "")
				{
					if(DxbPlayer_Control.options_audiopref != Integer.parseInt(list_value))
					{
					DxbPlayer_Control.options_audiopref = Integer.parseInt(list_value);
						DxbPlayer_Control.setAudioLanguage(DxbPlayer_Control.options_audiopref);	//off(0)
					}
				}
			}
			
			if((Mask_Options & MASK_OPTIONS_ASPECTRATIO) > 0)
			{
				list_value	= pref.getString(KEY_ASPECT_RATIO_SELECT, "");
				if(list_value != "")
				{
					if(DxbPlayer_Control.options_aspect_ratio != Integer.parseInt(list_value))
					{
						String strScreenMode;

						DxbPlayer_Control.options_aspect_ratio = Integer.parseInt(list_value);
						eState_ScreenSize = DxbPlayer_Control.options_aspect_ratio;
						strScreenMode = String.format("%d", eState_ScreenSize);

						SystemProperties.set("tcc.solution.screenmode.index", strScreenMode);
//						DxbPlayer_Control.setAspectRatio(DxbPlayer_Control.options_aspect_ratio);

						DxbPlayer_Control.mDB.putAspectRatio(eState_ScreenSize);
						Log.d(TAG, "DxbPlayer_Control.mDB.aspectRatio +()"+eState_ScreenSize);
					}
				}
			}

			if((Mask_Options & MASK_OPTIONS_CAPTION) > 0)
			{
				list_value= pref.getString(KEY_CAPTION_SELECT, "");
				if(list_value != "")
				{
					DxbPlayer_Control.options_subtitle_language	= Integer.parseInt(list_value);
				}
			}
			
			if((Mask_Options & MASK_OPTIONS_SUPER_IMPOSE) > 0)
			{
				list_value= pref.getString(KEY_SUPER_IMPOSE_SELECT, "");
				if(list_value != "")
				{
					DxbPlayer_Control.options_super_impose = Integer.parseInt(list_value);
				}
			}
		}

		DxbPlayer_Control.intentSubActivity = null;

		if(		(DxbPlayer_Control.mPlayer == null)
			&& (mSurfaceHolder != null)
		)
		{
			DxbPlayer_Control.createDxbPlayer();
			createDxbPlayer_common();
			DxbPlayer_Control.start();
			DxbPlayer_Control.mPlayer.setDisplay(mSurfaceHolder);
			DxbPlayer_Control.setSurface();
		}
		Dxb_Signal_create();

		if(eState_View==DXB_STATE_VIEW.FULL)
		{
			if( DxbOptions_ATSC.ScanIsPerformed == 2)
			{
				mTotalChannel_TvCount = 0;
				mTotalChannel_RadioCount = 0;
				mTotalChannel_CurrentCount	= 0;
			}
			else if( DxbOptions_ATSC.ScanIsPerformed == 1)
			{
				ListView_updateChannelListView(0);
				
				mTotalChannel_TvCount = mChannelsCursor.getCount();
				Log.d(TAG, "ScanIsPerformed - " + mTotalChannel_TvCount);
				if(DxbPlayer_Control.isVisible_Tab())
				{
					tabHost.setCurrentTab(1);
					if(mTotalChannel_TvCount > 0)
					{
						tabHost.setCurrentTab(0);
					}
					else
					{
						ListView_updateChannelListView(1);
						if(mTotalChannel_TvCount <= 0)
						{
							tabHost.setCurrentTab(0);
							Dxb_updateScreen();
						}
						else
						{
							tabHost.setCurrentTab(1);
						}
					}
				}
				mChannelsCursor.moveToFirst();
				setChannel();
				mListView.setSelectionFromTop(mChannelsCursor.getPosition(), LISTCENTER_Y);
			}
			FullView_setGroupState(true);
		}
		Dxb_setViewState(true, eState_View);
	}
	
	@Override
	protected void onPause()
	{
		Log.d(TAG, "================> onPause() --> eState_Player="+eState_Player);
		super.onPause();
		eCycle_Life = DXB_LIFE_CYCLE.ON_PAUSE;
		
		if(mNoSignalPopup!= null && mNoSignalPopup.isshown())
		{
			mNoSignalPopup.dismiss();
			mNoSignalPopup = null;
			NoSignalEnable = 0;
		}
		if(mScrambledPopup!= null && mScrambledPopup.isshown())
		{
			mScrambledPopup.dismiss();
			mScrambledPopup = null;
			ScrambledEnable =1;
			Log.e(TAG, "ScrambledEnable"+ScrambledEnable);
		}
		if(mNoServicePopup!= null && mNoServicePopup.isshown())
		{
			mNoServicePopup.dismiss();
			mNoServicePopup = null;
			NoServiceEnable =1;
		}

		eState_Player	= DXB_STATE_PLAYER.NULL;		
		DxbPlayer_Control.releaseSurface();
		Dxb_Signal_release();

		if(DxbPlayer_Control.intentSubActivity == null) 
		{
			if(mNoSignalPopup!= null && mNoSignalPopup.isshown())
			{
				mNoSignalPopup.dismiss();
				mNoSignalPopup = null;
				NoSignalEnable = 0;
			}
			if(mScrambledPopup!= null && mScrambledPopup.isshown())
			{
				mScrambledPopup.dismiss();
				mScrambledPopup = null;
				ScrambledEnable =0;
			}
			if(mNoServicePopup!= null && mNoServicePopup.isshown())
			{
				mNoServicePopup.dismiss();
				mNoServicePopup = null;
				NoServiceEnable =0;
			}

			Log.d(TAG, "DxbPlayer_Control.intentSubActivity == null");
			if( DxbPlayer_Control.mPlayer != null)
			{
				DxbPlayer_Control.mPlayer.stop();
				DxbPlayer_Control.mPlayer.release();

				DxbPlayer_Control.mPlayer	= null;

				SystemProperties.set("tcc.solution.preview", "0");
			}
		}

		if(SystemProperties.getBoolean("tcc.video.vsync.support", true) == true)
		{
			// deinterlace property setting is temporary.
			SystemProperties.set("tcc.video.deinterlace.support", "0");
			SystemProperties.set("tcc.video.vsync.enable", "0");
		}

		/* Off screen mode, if system doesn't use screen mode originally.
		 */
		if(mUseScreenMode == false)
		{
			SystemProperties.set("tcc.solution.use.screenmode", "1");
		}
	}

	@Override
	protected void onStop()
	{
		Log.d(TAG, "================> onStop");
		super.onStop();

		if(eCycle_Life == DXB_LIFE_CYCLE.ON_PAUSE)
		{
			Log.d(TAG, "================> DXB_LIFE_CYCLE.ON_PAUSE");

			eCycle_Life = DXB_LIFE_CYCLE.ON_STOP;
		}

		eState_Player	= DXB_STATE_PLAYER.NULL;
	}

	@Override
	public void onDestroy()
	{
		Log.d(TAG, "================> onDestroy()");
		eCycle_Life = DXB_LIFE_CYCLE.ON_DESTROY;

		if(mToast != null)
		{
			mToast.cancel();
			mToast = null;
		}
		if(DxbPlayer_Control.intentSubActivity == null)
		{
			if( mepgCursor != null )
			{
				mepgCursor.close();
				mepgCursor = null;
			}
			if( nextEPG != null )
			{
				nextEPG.close();
				nextEPG = null;
			}
			if( mChannelsCursor != null )
			{
				mChannelsCursor.close();
				mChannelsCursor = null;
			}
			if(DxbPlayer_Control.mChannelManager != null) 
			{
				DxbPlayer_Control.mChannelManager.close();
				DxbPlayer_Control.mChannelManager = null;
			}
		
			if(DxbPlayer_Control.mDB != null)
			{
				DxbPlayer_Control.mDB.close();
				DxbPlayer_Control.mDB = null;
			}
		}
		super.onDestroy();
	}

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        int keyCode = event.getKeyCode();
        if (keyCode == KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE) {
            return true;
        }

        return super.dispatchKeyEvent(event);
    }

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		Log.d(TAG, "onKeyDown(keyCode=" + keyCode + ", event=" + event + ")");

		mHandler_Main.removeCallbacks(mRunnable_KeyEvent);
		if(keyCode == KeyEvent.KEYCODE_BACK)
		{
			return Dxb_backView(keyCode, event, eState_View);
		}
		
		if( keyCode == KeyEvent.KEYCODE_PAGE_UP || keyCode == KeyEvent.KEYCODE_PAGE_DOWN )
		{
			if(eState_View == DXB_STATE_VIEW.FULL )
			{
				return onKeyDown_FullView(keyCode, event);
			}
		}
		
		if(eState_View == DXB_STATE_VIEW.FULL)
		{
			return onKeyDown_FullView(keyCode, event);
		}
		else if(eState_View == DXB_STATE_VIEW.EPG)
		{
			return onKeyDown_EPGView(keyCode, event);
		}
		else
		{
			switch(keyCode)
			{
				case KeyEvent.KEYCODE_HOME:
					break;
					
				case KeyEvent.KEYCODE_0:
					break;
				
				case KeyEvent.KEYCODE_9:
					break;
					
				case KeyEvent.KEYCODE_8:			// spd
					break;
						
				case KeyEvent.KEYCODE_7:			// A-B
					break;
							
				case KeyEvent.KEYCODE_CAMERA:
					break;
						
				case KeyEvent.KEYCODE_ENTER:		// EQ|MODE
					break;
						
				case KeyEvent.KEYCODE_MENU: 		// menu
					{
						if(tabHost.getCurrentTab()== 0)
							tabHost.setCurrentTab(1);
						else if(tabHost.getCurrentTab() == 1)
							tabHost.setCurrentTab(0);

						current_cursor = mChannelsCursor.getPosition();
						Log.d(TAG, "current_cursor()="+current_cursor);
						mListView.requestFocus();

						if( mChannelsCursor != null )
							mListView.setSelectionFromTop(mChannelsCursor.getPosition(), LISTCENTER_Y);
					}
					break;

				case KeyEvent.KEYCODE_DPAD_UP:		// ^
					{
						Log.d(TAG, "current_cursor="+current_cursor+"event.getRepeatCount()="+event.getRepeatCount());
						if(event.getAction() == KeyEvent.ACTION_DOWN 
								&& event.getRepeatCount()>= 0 && current_cursor == 0)
						{
							return true;
						}
					}
					break;
						
				case KeyEvent.KEYCODE_DPAD_DOWN:	// v
					 break;

				case KeyEvent.KEYCODE_DPAD_LEFT:			// <<
					return true;

				case KeyEvent.KEYCODE_DPAD_RIGHT:			// >>
					return true;

				case KeyEvent.KEYCODE_SOFT_LEFT:
					break;

				case KeyEvent.KEYCODE_SOFT_RIGHT:
					break;
			}
		}
	
		return super.onKeyDown(keyCode, event);
	}

	/* (non-Javadoc)
	 * @see android.app.Activity#onKeyMultiple(int, int, android.view.KeyEvent)
	 */
	@Override
	public boolean onKeyMultiple(int keyCode, int repeatCount, KeyEvent event)
	{
		return super.onKeyMultiple(keyCode, repeatCount, event);
	}

	/* (non-Javadoc)
	* @see android.app.Activity#onKeyUp(int, android.view.KeyEvent)
	*/
	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event)
	{
		//Log.d(TAG, "onKeyUp" + keyCode);
		
		if(keyCode == KeyEvent.KEYCODE_BACK)
		{//&& event.isTracking() && !event.isCanceled()) {
			// *** DO ACTION HERE ***
			// return true;
		}
		return super.onKeyUp(keyCode, event);
	}

	private void initSurfaceView()
	{
		Log.d(TAG, "initSurfaceView()");
		
		mSurfaceView = (SurfaceView) findViewById(R.id.surface_view);

		if(mSurfaceView == null)
		{
			Log.d(TAG, "mSurfaceView is Null");
			return;
		}
		SurfaceHolder holder = mSurfaceView.getHolder();
		if(holder == null)
		{
			Log.d(TAG, "++++++++++++++++++ mSurfaceHolder is Null");
			return;
		}

		holder.addCallback(mSHCallback);
		holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
	}
	
	// SurfaceView
	SurfaceHolder.Callback mSHCallback = new SurfaceHolder.Callback()
	{
		public void surfaceChanged(SurfaceHolder holder, int format, int w, int h)
		{
			if(mSurfaceHolder == null)
			{
				return;
			}
			DxbPlayer_Control.setLCDUpdate();
			DxbPlayer_Control.useSurface(0);
		}

		public void surfaceCreated(SurfaceHolder holder)
		{
			Log.d(TAG, "surfaceCreated(holder=" + holder + ")");

			mSurfaceHolder = holder;
			if(DxbPlayer_Control.mPlayer != null) 
			{
				DxbPlayer_Control.mPlayer.setDisplay(mSurfaceHolder);
				DxbPlayer_Control.setSurface();
			}
			else 
			{
				Log.d(TAG, "!!!!!!!!!!!!!!!!!!!!createDxbPlayer()");
				DxbPlayer_Control.createDxbPlayer();
				createDxbPlayer_common();
				DxbPlayer_Control.start();
				DxbPlayer_Control.mPlayer.setDisplay(mSurfaceHolder);
				DxbPlayer_Control.setSurface();
			}

			if(eState_View == DXB_STATE_VIEW.LIST)
			{
				int	iCurrentTab	= tabHost.getCurrentTab();
				if(mTotalChannel_CurrentCount > 0)
				{
					mListView.requestFocus();
					if(iCurrentTab == 0)
					{
						mListView.setSelection(DxbPlayer_Control.tvPosition);
					}
					else
					{
						mListView.setSelection(DxbPlayer_Control.radioPosition);
					}
				}
			}
		}

		public void surfaceDestroyed(SurfaceHolder holder)
		{
			Log.d(TAG, "surfaceDestroyed(holder="+holder+")");
			
			mSurfaceHolder = null;
		}
	};

	private void createDxbPlayer_common()
	{
		Log.d(TAG, "createDxbPlayer_common()");
		
		if(DxbPlayer_Control.mPlayer != null)
		{			
			DxbPlayer_Control.mPlayer.setOnErrorListener(errorlistener);
			DxbPlayer_Control.mPlayer.setOnPreparedListener(preparedlistener);
			DxbPlayer_Control.mPlayer.setOnSignalStatusListener(signalstatuslistener);
			DxbPlayer_Control.mPlayer.setOnScrambledStatusListener(scrambledstatuslistener);			
			DxbPlayer_Control.mPlayer.setOnEpgUpdateDoneListener(epgupdatedonelistener);
			DxbPlayer_Control.mPlayer.setOnTimeUpdateListener(timeupdatelistener);
			DxbPlayer_Control.mPlayer.setOnClosedCaptionUpdateListener(closedcaptionupdatelistner);
	
			//DxbPlayer_Control.mPlayer.prepare();
			DxbPlayer_Control.prepare(this);
	
			DxbPlayer_Control.mPlayer.setScreenOnWhilePlaying(true);
		}
	}
	
	ATSCPlayer.OnErrorListener errorlistener = new ATSCPlayer.OnErrorListener()
	{
		public boolean onError(ATSCPlayer player, int what, int extra)
		{
			Log.d(TAG, "Play Error!");
			return false;
		}
	};

	ATSCPlayer.OnPreparedListener preparedlistener = new ATSCPlayer.OnPreparedListener()
	{
		public void onPrepared(ATSCPlayer player)
		{
			Log.d(TAG,		"OnPreparedListener   "
						+	"     :"
						+	DxbPlayer_Control.tvPosition);
			
			ListView_updateChannelListView(0);
			
			//DxbPlayer_Control.setAspectRatio(eState_ScreenSize);
			DxbPlayer_Control.enableCC(DxbPlayer_Control.options_closedcaption);	//off(0)
			DxbPlayer_Control.setAudioLanguage(DxbPlayer_Control.options_audiopref);	//off(0)
			DxbPlayer_Control.setCCServiceNum(DxbPlayer_Control.options_captionsvcnum);
			
			if(		(DxbPlayer_Control.tvPosition >= mTotalChannel_TvCount)
				||	(DxbPlayer_Control.tvPosition < 0)
			)
			{
				DxbPlayer_Control.tvPosition = 0;
			}
			if(DxbPlayer_Control.tvPosition >= 0)
			{
				//Cursor c = mChannelsCursor;
				mChannelsCursor.moveToPosition(DxbPlayer_Control.tvPosition);
				//current_cursor = mChannelsCursor.getPosition();
				if(DxbPlayer_Control.isVisible_Tab())
				{
					tabHost.setCurrentTab(1);
					tabHost.setCurrentTab(0);
				}
				else
				{
					Dxb_updateScreen();
				}
				setChannel();
			}
			else if(DxbPlayer_Control.isVisible_Tab())
			{
				ListView_updateChannelListView(1);
				if(		(DxbPlayer_Control.radioPosition >= mTotalChannel_RadioCount)
					||	(DxbPlayer_Control.radioPosition < 0)
				)
				{
					DxbPlayer_Control.radioPosition = 0;
				}
				if(DxbPlayer_Control.radioPosition >= 0)
				{
					//Cursor c = mChannelsCursor;
					mChannelsCursor.moveToPosition(DxbPlayer_Control.radioPosition);
					//current_cursor = mChannelsCursor.getPosition();
					setChannel();
				}
			}
			FullView_setGroupState(true);
		}
	};
	
	ATSCPlayer.OnSignalStatusListener signalstatuslistener = new ATSCPlayer.OnSignalStatusListener()
	{
		public void onSignalStatusUpdate(ATSCPlayer player, int nLock, int nSignalStrength)
		{
			int iSignalGauge = 0;

			//Log.d(TAG, "nSignalStrength" + nSignalStrength+"nLock"+nLock);
			//Log.d(TAG,"NoSignalEnable"+NoSignalEnable+"NoServiceEnable"+NoServiceEnable+"ScrambledEnable"+ScrambledEnable);
			if(nLock == 1) 
			{
				if(nSignalStrength < 62)
					iSignalGauge = 1;
				else if(nSignalStrength < 128)
					iSignalGauge = 2;
				else if(nSignalStrength < 192)
					iSignalGauge = 3;
				else if(nSignalStrength < 256)
					iSignalGauge = 4;

				if( DxbPlayer_Control.intentSubActivity == null )
				{
					if( NoSignalEnable > 6 )
					{
						Log.d(TAG, "mNoSignalPopup.isshown" + mNoSignalPopup.isshown());
						if(mNoSignalPopup!=null && mNoSignalPopup.isshown())
						{
							mNoSignalPopup.dismiss();
							NoSignalEnable = 0;
							mNoSignalPopup = null;
							//DxbPlayer_Control.setSurface();
							DxbPlayer_Control.playSubtitle(DxbPlayer_Control.options_closedcaption);
							if( DxbPlayer_Control.options_closedcaption == 1)
								closedcaption_Display.setVisibility(View.VISIBLE);
						}
						if( NoServiceEnable == 1 )
						{
							mNoServicePopup = new DxbPlayer_Popup(mListView);
							mNoServicePopup.additem(setItem("      No Service"));
							mNoServicePopup.show();							
							if(eState_View == DXB_STATE_VIEW.LIST)
								mNoServicePopup.update(1);
							else if(eState_View == DXB_STATE_VIEW.FULL)
								mNoServicePopup.update(2);
							else if(eState_View == DXB_STATE_VIEW.EPG)
								mNoServicePopup.update(3);
							NoServiceEnable = 2;
						}
						else if( ScrambledEnable == 1 )
						{
							mScrambledPopup = new DxbPlayer_Popup(mListView);
							mScrambledPopup.additem(setItem("Scrambled Channel"));
							mScrambledPopup.show();
							if(eState_View == DXB_STATE_VIEW.LIST)
								mScrambledPopup.update(1);
							else if(eState_View == DXB_STATE_VIEW.FULL)
								mScrambledPopup.update(2);
							else if(eState_View == DXB_STATE_VIEW.EPG)
								mScrambledPopup.update(3);
							ScrambledEnable = 2;
						}
					}
				}
			}
			else
			{
				if( DxbPlayer_Control.intentSubActivity == null
						&& mChannelChanging == false
						&& mTotalChannel_CurrentCount > 0
				)
				{
					if(NoSignalEnable == 6 )
					{
						if( ScrambledEnable >= 2 && mScrambledPopup!=null && mScrambledPopup.isshown())
						{
							mScrambledPopup.dismiss();
							mScrambledPopup = null;
							ScrambledEnable = 1;
						}
						else if( NoServiceEnable == 2 && mNoServicePopup!= null && mNoServicePopup.isshown())
						{
							mNoServicePopup.dismiss();
							mNoServicePopup = null;
							NoServiceEnable = 1;
						}
					
						mNoSignalPopup = new DxbPlayer_Popup(mListView);
						mNoSignalPopup.additem(setItem("      No Signal"));
						mNoSignalPopup.show();					
						if(eState_View == DXB_STATE_VIEW.LIST)
							mNoSignalPopup.update(1);
						else if(eState_View == DXB_STATE_VIEW.FULL)
							mNoSignalPopup.update(2);
						else if(eState_View == DXB_STATE_VIEW.EPG)
							mNoSignalPopup.update(3);
						Log.d(TAG, "mNoSignalPopup 3");
						DxbPlayer_Control.playSubtitle(0);
						closedcaption_Display.setVisibility(View.INVISIBLE);
						// ReverseJ
//						DxbPlayer_Control.releaseSurface();
					}
					NoSignalEnable++;
					//Log.d(TAG, "NoSignalEnable: " + NoSignalEnable);
				}
			}
			mSignalGauge = iSignalGauge;
			//Log.d(TAG, "mSignalGauge."+mSignalGauge );
		}
	};

	ATSCPlayer.OnScrambledStatusListener scrambledstatuslistener = new ATSCPlayer.OnScrambledStatusListener()
	{
		public void onScrambledStatusUpdate(ATSCPlayer player, int iScrambledState, int iChannelNumber)
		{
			Log.d(TAG, "scrambledstatuslistener() iScrambledState :"+iScrambledState+"iChannelNumber :" + iChannelNumber);
			Log.d(TAG, "ScrambledEnable"+ScrambledEnable);
			Log.d(TAG, "getChannelNumber="+DxbPlayer_Control.getChannelNumber());

			if( DxbPlayer_Control.intentSubActivity == null )//&& iChannelNumber == DxbPlayer_Control.getChannelNumber())
			{
				if( iScrambledState == 0 )// free channel
				{
					if( ScrambledEnable == 2 && NoServiceEnable == 0 &&  NoSignalEnable == 0 )
					{
						if(mScrambledPopup!= null && mScrambledPopup.isshown())
						{
							mScrambledPopup.dismiss();
							mScrambledPopup = null;
							ScrambledEnable = 0;
						}
					}
					else if( ScrambledEnable > 0 && NoServiceEnable == 0 &&  NoSignalEnable == 0)
						ScrambledEnable =0;
				}
				else if( iScrambledState == 1 )// scrambled channel && iChannelNumber == DxbPlayer_Control.getChannelNumber()
				{
					if( ScrambledEnable == 0 )
					{
						mScrambledPopup = new DxbPlayer_Popup(mListView);
						mScrambledPopup.additem(setItem("Scrambled Channel"));
						mScrambledPopup.show();
						if(eState_View == DXB_STATE_VIEW.LIST)
							mScrambledPopup.update(1);
						else if(eState_View == DXB_STATE_VIEW.FULL)
							mScrambledPopup.update(2);
						else if(eState_View == DXB_STATE_VIEW.EPG)
							mScrambledPopup.update(3);
						ScrambledEnable = 2;
						Log.d(TAG, "mScrambledToast");
					}
				}
				else if( iScrambledState == 2 ) // No service
				{
					if( NoSignalEnable >= 7 )
					{
						NoServiceEnable = 1;
					}
					else if( ScrambledEnable == 2 && mScrambledPopup != null && mScrambledPopup.isshown())
					{
						mScrambledPopup.dismiss();
						mScrambledPopup = null;
						ScrambledEnable = 1;
						mNoServicePopup = new DxbPlayer_Popup(mListView);
						mNoServicePopup.additem(setItem("      No Service"));
						mNoServicePopup.show();
						if(eState_View == DXB_STATE_VIEW.LIST)
							mNoServicePopup.update(1);
						else if(eState_View == DXB_STATE_VIEW.FULL)
							mNoServicePopup.update(2);
						else if(eState_View == DXB_STATE_VIEW.EPG)
							mNoServicePopup.update(3);
						NoServiceEnable = 2;
					}
					else if( ScrambledEnable == 1 || NoServiceEnable == 0 )
					{
						mNoServicePopup = new DxbPlayer_Popup(mListView);
						mNoServicePopup.additem(setItem("      No Service"));
						mNoServicePopup.show();
						if(eState_View == DXB_STATE_VIEW.LIST)
							mNoServicePopup.update(1);
						else if(eState_View == DXB_STATE_VIEW.FULL)
							mNoServicePopup.update(2);
						else if(eState_View == DXB_STATE_VIEW.EPG)
							mNoServicePopup.update(3);
						NoServiceEnable = 2;
					}
				}
				else if(iScrambledState == 3)//Service available
				{
					if( NoServiceEnable == 2 && mNoServicePopup != null && mNoServicePopup.isshown())
					{
						mNoServicePopup.dismiss();
						mNoServicePopup = null;
						NoServiceEnable = 0;
						if( ScrambledEnable == 1 )
						{
							mScrambledPopup = new DxbPlayer_Popup(mListView);
							mScrambledPopup.additem(setItem("Scrambled Channel"));
							mScrambledPopup.show();
							if(eState_View == DXB_STATE_VIEW.LIST)
								mScrambledPopup.update(1);
							else if(eState_View == DXB_STATE_VIEW.FULL)
								mScrambledPopup.update(2);
							else if(eState_View == DXB_STATE_VIEW.EPG)
								mScrambledPopup.update(3);
							ScrambledEnable = 2;
						}
					}
					else if( NoServiceEnable == 1 )
					{
						NoServiceEnable = 0;
					}
				}
			}
		}
	};

	ATSCPlayer.OnTimeUpdateListener timeupdatelistener = new ATSCPlayer.OnTimeUpdateListener()
	{
		public void onTimeUpdateUpdate(ATSCPlayer player, int iUTCTime)
		{
			//Log.d(TAG, "timeupdatelistener() iUTCTime :"+iUTCTime);
			if( DxbPlayer_Control.intentSubActivity == null)
			{
				Time t = new Time("KST");
				t.set((long)iUTCTime*1000);
				t.normalize(true);
				String current_time = String.format("%02d:%02d", t.hour, t.minute);
				timeIndicator.setText(current_time);
				//Log.d(TAG, "iUTCTime :"+t);
			}
		}
	};

	ATSCPlayer.OnEpgUpdateDoneListener epgupdatedonelistener = new ATSCPlayer.OnEpgUpdateDoneListener()
	{
		public void onEpgUpdateDoneUpdate(ATSCPlayer player, int nChannelNumber, int nServiceID)
		{
			Log.d(TAG, "epgupdatedonelistener() nChannelNumber :"+nChannelNumber+" nServiceID :"+ nServiceID +"\n");
			DxbPlayer_Control.epgPosition = 0;

			EPG_getFirstLastDate(nServiceID, nChannelNumber );
			epgupdatedone = true;
			
			if(eState_View == DXB_STATE_VIEW.EPG)
			{
				EPG_updateEPGList();
				setEPGProgramName(DxbPlayer_Control.getModulation(), DxbPlayer_Control.getLogicalCHNumber(),DxbPlayer_Control.getLogicalCHMinorNumber(),DxbPlayer_Control.getServiceName());
				Dxb_updateToast("EPG UPDATED");
				mepgListView.invalidateViews();
			}
		}
	};

	static ATSCPlayer.OnClosedCaptionUpdateListener closedcaptionupdatelistner = new ATSCPlayer.OnClosedCaptionUpdateListener()
	{
		public void onClosedCaptionUpdate(ATSCPlayer player, SubtitleData subt) {
			//Log.d(TAG, "closedcaptionupdatelistner()\n");
			if(mBitmap != null)
				mBitmap.eraseColor(0x0);

			if ( subt.region_id < 0 && closedcaptioncleardone == false) { // clear 
				closedcaption_Display.setVisibility(View.INVISIBLE);
				closedcaptioncleardone = true;
			}
			else if( subt.region_id >= 0 ){ // draw
				if(mBitmap == null) {
					mBitmap = Bitmap.createBitmap(subt.width, subt.height, Bitmap.Config.ARGB_8888);
					mCanvas.setBitmap(mBitmap);
				}
				if( closedcaptioncleardone == true) {
					closedcaption_Display.setVisibility(View.VISIBLE);
					closedcaptioncleardone = false;
				}
				
				mRect.set(subt.x, subt.y-520, subt.width, subt.height);

				mCanvas.save();
				mCanvas.clipRect(mRect);
				mCanvas.translate(3, 3);
				mCanvas.drawColor(0x00000000);
				mCanvas.restore();

				//Log.d(TAG, " region_id:"+subt.region_id+"subt.x:"+subt.x+"subt.y:"+subt.y+"subt.width:"+subt.width+"subt.height"+subt.height);
				mCanvas.drawBitmap(subt.data, 0, subt.width, subt.x, 0, subt.width, subt.height, true, mPaint);

				closedcaption_Display.setImageBitmap(mBitmap);
				closedcaption_Display.invalidate();
			}
		}
	};
	
	private static boolean EPG_GetMatchSpecialCase( String CompareDate)
	{
		Log.e(TAG, "CompareDate"+CompareDate);

		if( CompareDate.equals("01310201"))			return true;
		else if( CompareDate.equals("02280301"))	return true;
		else if( CompareDate.equals("02290301"))	return true;
		else if( CompareDate.equals("03310401"))	return true;
		else if( CompareDate.equals("04300501"))	return true;
		else if( CompareDate.equals("05310601"))	return true;
		else if( CompareDate.equals("06300701"))	return true;
		else if( CompareDate.equals("07310801"))	return true;
		else if( CompareDate.equals("08310901"))	return true;
		else if( CompareDate.equals("09301001"))	return true;
		else if( CompareDate.equals("10311101"))	return true;
		else if( CompareDate.equals("11301201"))	return true;
		else if( CompareDate.equals("12310101"))	return true;
		else
			return false;
	}

	private void EPG_getFirstLastDate(int nServiceID, int nChannelNumber)
	{
		int	epgFirstIdx= 0, epgLastIdx = 0;
		int	epgFirstdate= 0, epgLastdate = 0;
		Cursor epgFirstCursor, epgLastCursor = null;

		if(DxbPlayer_Control.openEPGDB() == false)
		{
			Log.e(TAG, "CANNOT open EPG database !!!");
			return;
		}

		epgFirstIdx = DxbPlayer_Control.getEPGFirstIndex(nServiceID, nChannelNumber );
		epgLastIdx = DxbPlayer_Control.getEPGLastIndex(nServiceID, nChannelNumber );
		Log.d(TAG, "epgFirstIdx:"+epgFirstIdx+" epgLastIdx :"+ epgLastIdx +"\n");

		epgFirstCursor = DxbPlayer_Control.getEPGInfo(nServiceID, nChannelNumber , epgFirstIdx);
		epgLastCursor = DxbPlayer_Control.getEPGInfo(nServiceID, nChannelNumber , epgLastIdx);
		DxbPlayer_Control.setEPG_ColumnIndex(epgFirstCursor);

		DxbPlayer_Control.getEPG_DisplayDate(epgFirstCursor);
		DxbPlayer_Control.getEPG_DisplayDate(epgLastCursor);

		epgFirstdate = DxbPlayer_Control.getEPG_StartUTCTime(epgFirstCursor);
		epgLastdate = DxbPlayer_Control.getEPG_StartUTCTime(epgLastCursor);

		mepgFirstday  = DxbPlayer_Control.getEPG_StartDay(epgFirstCursor);
		mepgLastday = DxbPlayer_Control.getEPG_StartDay(epgLastCursor);

		if( epgFirstCursor != null) {
			epgFirstCursor.close();
			epgFirstCursor = null;
		}
		if( epgLastCursor != null) {
			epgLastCursor.close();
			epgLastCursor = null;
		}		
		DxbPlayer_Control.closeEPGDB();

		mMatchCase = EPG_GetMatchSpecialCase(String.format("%04d%04d", epgFirstdate, epgLastdate));
	}
	
	private DxbPlayer_Item setItem(String title)
	{
		DxbPlayer_Item item = new DxbPlayer_Item();
		item.setTitle(title);
		item.setRes(R.drawable.dxb_portable_popup_icon);
		return item;
	}

	private void initSystemSet()
	{
		Log.d(TAG, "initSystemSet()");
		Display display = ((WindowManager)this.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
		mDisplayWidth = display.getWidth();
		mDisplayHeight = display.getHeight();
	}
	
	private void initSignal()
	{
		mWindowParams = new WindowManager.LayoutParams();
		mImage_Signal = new ImageView(this);
		mImage_Signal.setImageResource(R.drawable.dxb_portable_indicator_rssi_icon_00);
		
		if(mDisplayWidth > 800)
		{
			mWindowParams.x = 1140;
			mWindowParams.y = 5;
			mWindowParams.gravity = Gravity.TOP + Gravity.LEFT;
			mWindowParams.height = 30;
			mWindowParams.width = 30;
		}
		else
		{
			mWindowParams.x = 715;
			mWindowParams.y = 2;
			mWindowParams.gravity = Gravity.TOP + Gravity.LEFT;
			mWindowParams.height = android.view.ViewGroup.LayoutParams.WRAP_CONTENT;
			mWindowParams.width = android.view.ViewGroup.LayoutParams.WRAP_CONTENT;
		}
		
		mWindowParams.flags =		WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
								|	WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON
								|	WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN;
		mWindowParams.format = PixelFormat.TRANSLUCENT;
		mWindowParams.windowAnimations = 0;

		mWindowManager = (WindowManager) this.getSystemService("window");

		// Section
		imageIndicator	= (ImageView)findViewById(R.id.indicator_section);
		
		// Time
		timeIndicator	= (TextView)findViewById(R.id.indicator_time);
	}

	private Runnable mRunnable_Signal = new Runnable()
	{
		public void run()
		{
			//Log.d(TAG, "mRunnable_Signal --> run() mSignalGauge"+mSignalGauge);
			if(		(DxbPlayer_Control.mPlayer == null)
				||	(mSurfaceView == null)
				||	(eState_Player == DXB_STATE_PLAYER.UI_PAUSE)
			)
			{
				return;
			}
			
			try
			{
				if(		(mTotalChannel_TvCount <= 0)
					&&	(mTotalChannel_RadioCount <= 0)
				)
				{
					Dxb_Signal_hide();
				}
				else
				{
					int	iLevel	= mSignalGauge;
					if(iLevel <= 0)
					{
						count_Signal++;
					}
					else
					{
						count_Signal = 0;
					}
			
					if(		(level_Signal >= 0)
						&&	(count_Signal >= 5)
					)
					{
						level_Signal = -1;
						Dxb_updateScreen();
						if(DxbPlayer_Control.mPlayer != null)
						{
							DxbPlayer_Control.mPlayer.stop();
						}
					}
					else if(		(level_Signal < 0)
								&&	(iLevel > 0)
					)
					{
						level_Signal	= iLevel;
						Dxb_updateScreen();
						mChannelsCursor.moveToPosition(DxbPlayer_Control.tvPosition);
						setChannel();
					}
					/*
					mImage_Signal.setImageResource(rssIcon[iLevel]);
					*/
					mImage_Signal.setImageResource(rssIcon[mSignalGauge]);
					if(		(eState_View==DXB_STATE_VIEW.FULL)
						&&	!Dxb_isShown_Signal()
					)
					{
						mHandler_Main.postDelayed(mRunnable_Signal, 500);
					}
					else
					{
						Dxb_Signal_display();
					}
				}
			}
			catch(IllegalStateException e)
			{
				e.printStackTrace();
			}
		}
	};
	
	private void initView()
	{
		Log.d(TAG, "initView()");
		initListView();
		initFullView();
		initEPG();
		initSubtitleView();
	}
	
	private void initListView()
	{
		Log.d(TAG, "initListView()");

		mListView = (ListView) findViewById(DxbPlayer_Control.getRid_List_ServiceList());
		mListView.setOnItemClickListener(ListenerOnItemClick_serviceListView);
		mListView.setOnKeyListener(ListenerOnKey_serviceListView);
		mListView.setEmptyView(findViewById(R.id.ch_list_empty));
		
		initTab();

		View_List	= findViewById(DxbPlayer_Control.getRid_ListView());
		View_List.setOnClickListener(new OnClickListener()
		{
			public void onClick(View v)
			{
				Dxb_setViewState(false, DXB_STATE_VIEW.FULL);	// (full view) TV only.
			}
		});
	}
	
	private void initSubtitleView()
	{
		Log.d(TAG, "initSubtitleView()");
		
		if(mRect == null)
			mRect = new Rect();

		if(mCanvas == null)
			mCanvas	= new Canvas();

		if( mPaint == null)
			mPaint	= new Paint(Paint.ANTI_ALIAS_FLAG);

		closedcaption_Display = (ImageView)findViewById(R.id.full_closedcaption_display);
		closedcaption_Display.setScaleType(ImageView.ScaleType.FIT_XY);
		closedcaption_Display.setVisibility(View.INVISIBLE);
	}
	
	OnFocusChangeListener mFocusChangeListener	= new View.OnFocusChangeListener()
	{
		public void onFocusChange(View v, boolean hasFocus)
		{
			//Log.d(TAG, "onFocusChange()");
			
			int id = v.getId();
			int	menu_index	= getIndex_Menu(id);

			if( id == DxbPlayer_Control.getRid_FullCHup())
			{
				if(hasFocus == true)
				{
					mChannelUpBtn.setBackgroundResource(R.drawable.dxb_portable_channel_up_btn_f);
				}
				else
				{
					mChannelUpBtn.setBackgroundResource(R.drawable.dxb_portable_channel_up_btn_n);
				}
			}
			else if( id == DxbPlayer_Control.getRid_FullCHdown())
			{
				if(hasFocus == true)
				{
					mChannelDownBtn.setBackgroundResource(R.drawable.dxb_portable_channel_down_btn_f);
				}
				else
				{
					mChannelDownBtn.setBackgroundResource(R.drawable.dxb_portable_channel_down_btn_n);
				}
			}
			else if( id == DxbPlayer_Control.getRid_FullMenu1())
			{
				fullIcon1.setImageResource(DxbPlayer_Control.getRdrawable_FullMenuIcon(menu_index, hasFocus));
				if(hasFocus == true)
				{
					fullButton_menu1.setBackgroundResource(DxbPlayer_Control.getRid_FullMenu_bg());
					fullText1.setTextColor(getResources().getColor(R.color.color_6));
				}
				else
				{
					fullButton_menu1.setBackgroundColor(getResources().getColor(R.color.color_0));
					fullText1.setTextColor(getResources().getColor(R.color.color_4));
				}
			}
			else if( id == DxbPlayer_Control.getRid_FullMenu2())
			{
				fullIcon2.setImageResource(DxbPlayer_Control.getRdrawable_FullMenuIcon(menu_index, hasFocus));
				if(hasFocus == true)
				{
					fullButton_menu2.setBackgroundResource(DxbPlayer_Control.getRid_FullMenu_bg());
					fullText2.setTextColor(getResources().getColor(R.color.color_6));
				}
				else
				{
					fullButton_menu2.setBackgroundColor(getResources().getColor(R.color.color_0));
					fullText2.setTextColor(getResources().getColor(R.color.color_4));
				}
			}
			else if( id == DxbPlayer_Control.getRid_FullMenu3())
			{
				fullIcon3.setImageResource(DxbPlayer_Control.getRdrawable_FullMenuIcon(menu_index, hasFocus));
				if(hasFocus == true)
				{
					fullButton_menu3.setBackgroundResource(DxbPlayer_Control.getRid_FullMenu_bg());
					fullText3.setTextColor(getResources().getColor(R.color.color_6));
				}
				else
				{
					fullButton_menu3.setBackgroundColor(getResources().getColor(R.color.color_0));
					fullText3.setTextColor(getResources().getColor(R.color.color_4));
				}
			}
			else if( id == DxbPlayer_Control.getRid_FullMenu4())
			{
				fullIcon4.setImageResource(DxbPlayer_Control.getRdrawable_FullMenuIcon(menu_index, hasFocus));
				if(hasFocus == true)
				{
					fullButton_menu4.setBackgroundResource(DxbPlayer_Control.getRid_FullMenu_bg());
					fullText4.setTextColor(getResources().getColor(R.color.color_6));
				}
				else
				{
					fullButton_menu4.setBackgroundColor(getResources().getColor(R.color.color_0));
					fullText4.setTextColor(getResources().getColor(R.color.color_4));
				}
			}
			else if( id == R.id.epg_prev)
			{
				if(hasFocus == true)
				{
					epgButton_Prev.setBackgroundResource(R.drawable.dxb_portable_epg_left_btn_f);
				}
				else
				{
					if(DxbPlayer_Control.epgFirst == 0)
						epgButton_Prev.setBackgroundResource(R.drawable.dxb_portable_epg_left_btn_n);
				}
			}
			else if( id == R.id.epg_next)
			{
				if(hasFocus == true)
				{
					epgButton_Next.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_f);
				}
				else
				{
					if(DxbPlayer_Control.epgLast == 0)
						epgButton_Next.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_n);
				}
			}
		}
	};
	
	OnTouchListener mTouchListener	= new View.OnTouchListener()
	{
		public boolean onTouch(View v, MotionEvent mEvent)
		{
			//Log.d(TAG, "onTouch(v=" + v +", mEvent=" + mEvent + ")");
			int	id = v.getId();
			if(mEvent.getAction() == MotionEvent.ACTION_DOWN)
			{
				if(		(id == DxbPlayer_Control.getRid_FullMenu1())
							&&	(!fullButton_menu1.isFocused())
				)
				{
					fullButton_menu1.requestFocus();
				}
				else if(		(id == DxbPlayer_Control.getRid_FullMenu2())
							&&	(!fullButton_menu2.isFocused())
				)
				{
					fullButton_menu2.requestFocus();
				}
				else if(		(id == DxbPlayer_Control.getRid_FullMenu3())
							&&	(!fullButton_menu3.isFocused())
				)
				{
					fullButton_menu3.requestFocus();
				}
				else if(		(id == DxbPlayer_Control.getRid_FullMenu4())
							&&	(!fullButton_menu4.isFocused())
				)
				{
					fullButton_menu4.requestFocus();
				}
				else if(		(id == DxbPlayer_Control.getRid_FullCHdown())
							&&	(!mChannelDownBtn.isFocused())
				)
				{
					mChannelDownBtn.requestFocus();
				}
				else if(		(id == DxbPlayer_Control.getRid_FullCHup())
						&&	(!mChannelUpBtn.isFocused())
				)
				{
					mChannelUpBtn.requestFocus();
				}
			}
			else if(mEvent.getAction() == MotionEvent.ACTION_UP)
			{
				if(		(		(id == DxbPlayer_Control.getRid_FullMenu1())
							&&	(!fullButton_menu1.isFocused())
						)
					||	(		(id == DxbPlayer_Control.getRid_FullMenu2())
							&&	(!fullButton_menu2.isFocused())
						)
					||	(		(id == DxbPlayer_Control.getRid_FullMenu3())
							&&	(!fullButton_menu3.isFocused())
						)
					||	(		(id == DxbPlayer_Control.getRid_FullMenu4())
							&&	(!fullButton_menu4.isFocused())
						)
				)
				{
					int	menu_index	= getIndex_Menu(id);
					FullView_setEvent(DxbPlayer_Control.getMenuState(menu_index));
				}
			}
			return false;
		}
	};

	OnItemClickListener ListenerOnItemClick_serviceListView = new OnItemClickListener()
	{
		public void onItemClick(AdapterView<?> parent, View v, int position, long id)
		{
			Log.d(TAG, "ListenerOnItemClick_serviceListView-->onItemClick(position="+position+")");
			Log.d(TAG, "DxbPlayer_Control.tvPosition="+DxbPlayer_Control.tvPosition+")");
			Log.d(TAG, "DxbPlayer_Control.radioPosition="+DxbPlayer_Control.radioPosition+")");

			if(	(		DxbPlayer_Control.getTab() == 0 
					&&	(DxbPlayer_Control.tvPosition != position)
				)
			||	(		DxbPlayer_Control.getTab() == 1 
					&&	(DxbPlayer_Control.radioPosition != position)
				)
			)
			{
				mChannelsCursor.moveToPosition(position);
				mListView.setSelection(position);

				setChannel();
			}
			else if( position == 0 )
			{
				mChannelsCursor.moveToPosition(position);
				mListView.setSelection(position);

				setChannel();
			}
		}			
	};

	OnKeyListener ListenerOnKey_serviceListView = new OnKeyListener()
	{
		public boolean onKey(View v, int keyCode, KeyEvent event)
		{
			if(		(event.getAction() == KeyEvent.ACTION_UP)
				||	(event.getRepeatCount() > 0)
			)
			{
				Log.d(TAG, "ListenerOnKey_serviceListView-->onKey(keyCode=" + keyCode + ", event=" + event + ")");
				
				if(NoSignalEnable > 6 && mNoSignalPopup!=null && mNoSignalPopup.isshown())
				{
					mNoSignalPopup.dismiss();
					mNoSignalPopup = null;
					NoSignalEnable = 0;
				}
				if(keyCode==KeyEvent.KEYCODE_DPAD_RIGHT||keyCode==KeyEvent.KEYCODE_DPAD_LEFT)
				{
					return false;
				}

				int	iCurrentTab	= 0;
				if(DxbPlayer_Control.isVisible_Tab())
				{
					iCurrentTab	= tabHost.getCurrentTab();
				}
				
				switch(keyCode)
				{
					case KeyEvent.KEYCODE_DPAD_UP:
						if(		(		(iCurrentTab==0)
									&&	(mTotalChannel_TvCount>1)
								)
							||	(		(iCurrentTab==1)
									&&	(mTotalChannel_RadioCount>1)
								)
						)
						{
							if( current_cursor > 0)
								current_cursor--;

							OnKeyEvent_setCurrentService(true);
						}
						break;
						
					case KeyEvent.KEYCODE_DPAD_DOWN:
						if(		(		(iCurrentTab==0)
									&&	(mTotalChannel_TvCount>1)
								)
							||	(		(iCurrentTab==1)
									&&	(mTotalChannel_RadioCount>1)
								)
						)
						{
							if( mChannelsCursor.getPosition() == 0 )
								current_cursor = 0;
							else
								current_cursor++;

							OnKeyEvent_setCurrentService(true);
						}
						break;
				}
			}
			
			return false;
		}
	};
	
	private void OnKeyEvent_setCurrentService(boolean state)
	{
		Log.d(TAG, "OnKeyEvent_setCurrentService(state=" + state + ")");
		if(state)
		{
			mHandler_Main.removeCallbacks(mRunnable_KeyEvent);
			mHandler_Main.postDelayed(mRunnable_KeyEvent, 600);
		}
		else
		{
			mHandler_Main.removeCallbacks(mRunnable_KeyEvent);
			Log.d(TAG, "mChannelsCursor.getPosition()="+mChannelsCursor.getPosition());
			Log.d(TAG, "mChannelsCursor.current_cursor()="+current_cursor);
			if( mChannelsCursor.getPosition() >= 0 )
			{
				if(  (DxbPlayer_Control.getTab() == 0
						&& (DxbPlayer_Control.tvPosition != mChannelsCursor.getPosition()) 
					)
					||	(		DxbPlayer_Control.getTab() == 1 
						&& (DxbPlayer_Control.radioPosition != mChannelsCursor.getPosition())
					)
				)
				{
					if( current_cursor >= mTotalChannel_TvCount && (DxbPlayer_Control.getTab()==0))
						current_cursor = mTotalChannel_TvCount-1;
					else if( current_cursor >= mTotalChannel_RadioCount && (DxbPlayer_Control.getTab()==1))
						current_cursor = mTotalChannel_RadioCount-1;

					if( mChannelsCursor.getPosition() != current_cursor )
						mChannelsCursor.moveToPosition(current_cursor);
					
					setChannel();
				}
				else
					current_cursor = mChannelsCursor.getPosition();
			}
			Log.d(TAG, "mChannelsCursor.getPosition()="+mChannelsCursor.getPosition()+", current_cursor="+ current_cursor);
		}			
	}			
	
	private void initTab()
	{
		Log.d(TAG, "initTab()");
		
		tabHost	= (TabHost)findViewById(R.id.list_tab);
		tabHost.setup();
		
		TabHost.TabSpec	spec;
		
		//Tab 01  setting & register
		spec = tabHost.newTabSpec("Tab 00");	// Tab Builder - create object
		spec.setIndicator(	getResources().getString(R.string.tv_channel),
							getResources().getDrawable(R.drawable.tab_tv_icon));	// Tab title
		spec.setContent(R.id.service_list);			// Tab description
		tabHost.addTab(spec);					// Tab register
		
		//Tab 02 setting & register
		spec = tabHost.newTabSpec("Tab 01");	// Tab Builder - create object
		spec.setIndicator(	getResources().getString(R.string.radio_channel),
							getResources().getDrawable(R.drawable.tab_radio_icon));	// Tab title

		spec.setContent(R.id.service_list);			// Tab description
		tabHost.addTab(spec);					// Tab register
		if(mDisplayWidth <= 800)
		{
			for ( int i = 0; i<tabHost.getTabWidget().getChildCount();i++)
			{
				tabHost.getTabWidget().getChildAt(i).getLayoutParams().height = 85;
			}
		}
		tabHost.setOnTabChangedListener(new OnTabChangeListener()
		{
			public void onTabChanged(String tabId)
			{
				Log.d(TAG, "onTabChanged() tabId =" + tabId);
				
				int	iCurrentTab	= tabHost.getCurrentTab();
				Log.d(TAG, "onTabChanged() iCurrentTab =" + iCurrentTab);
				for ( int i = 0; i<tabHost.getTabWidget().getChildCount();i++)
				{
					tabHost.getTabWidget().getChildAt(i).setBackgroundResource(R.drawable.dxb_portable_tab_bg_01_n);
					TextView tv = (TextView) tabHost.getTabWidget().getChildAt(i).findViewById(android.R.id.title);
					tv.setTextColor(getResources().getColorStateList(R.drawable.custom_textcolor));
				}
				tabHost.getTabWidget().getChildAt(iCurrentTab).setBackgroundResource(R.drawable.dxb_portable_tab_bg_01_f);
				
				ListView_updateChannelListView(iCurrentTab);
				
				Dxb_updateScreen();
				if(mTotalChannel_CurrentCount > 0)
				{
					mListView.requestFocus();
					if(iCurrentTab == 0)
					{
						mListView.setSelection(DxbPlayer_Control.tvPosition);
					}
					else
					{
						mListView.setSelection(DxbPlayer_Control.radioPosition);
					}
				}
			}
		});
	}
	
	private void initFullView()
	{
		Log.d(TAG, "initFullView()");
		View_Full	= findViewById(DxbPlayer_Control.getRid_FullView());
		View_Full.setOnClickListener(new OnClickListener()
		{
			public void onClick(View v)
			{
				FullView_setGroupState(!fullViewGroup.isShown());
			}
		});
		
		fullViewGroup = (ViewGroup) findViewById(DxbPlayer_Control.getRid_FullGroup());
		
		fullTitle	= (TextView)findViewById(DxbPlayer_Control.getRid_FullTitle());
		
		mChannelUpBtn	= (Button) findViewById(DxbPlayer_Control.getRid_FullCHup());
		mChannelDownBtn	= (Button) findViewById(DxbPlayer_Control.getRid_FullCHdown());

		fullButton_menu1	= (Button)findViewById(DxbPlayer_Control.getRid_FullMenu1());
		fullButton_menu2	= (Button)findViewById(DxbPlayer_Control.getRid_FullMenu2());
		fullButton_menu3	= (Button)findViewById(DxbPlayer_Control.getRid_FullMenu3());
		fullButton_menu4	= (Button)findViewById(DxbPlayer_Control.getRid_FullMenu4());

		fullIcon1	= (ImageView)findViewById(DxbPlayer_Control.getRid_FullMenu1_Icon());
		fullIcon2	= (ImageView)findViewById(DxbPlayer_Control.getRid_FullMenu2_Icon());
		fullIcon3	= (ImageView)findViewById(DxbPlayer_Control.getRid_FullMenu3_Icon());
		fullIcon4	= (ImageView)findViewById(DxbPlayer_Control.getRid_FullMenu4_Icon());

		fullIcon1.setImageResource(DxbPlayer_Control.getRdrawable_FullMenuIcon(1, false));
		fullIcon2.setImageResource(DxbPlayer_Control.getRdrawable_FullMenuIcon(2, false));
		fullIcon3.setImageResource(DxbPlayer_Control.getRdrawable_FullMenuIcon(3, false));
		fullIcon4.setImageResource(DxbPlayer_Control.getRdrawable_FullMenuIcon(4, false));

		fullText1	= (TextView)findViewById(DxbPlayer_Control.getRid_FullMenu1_Text());
		fullText2	= (TextView)findViewById(DxbPlayer_Control.getRid_FullMenu2_Text());
		fullText3	= (TextView)findViewById(DxbPlayer_Control.getRid_FullMenu3_Text());
		fullText4	= (TextView)findViewById(DxbPlayer_Control.getRid_FullMenu4_Text());

		// specific setup
		fullText1.setText(getResources().getString(DxbPlayer_Control.getRstring_FullMenuText(1)));
		fullText2.setText(getResources().getString(DxbPlayer_Control.getRstring_FullMenuText(2)));
		fullText3.setText(getResources().getString(DxbPlayer_Control.getRstring_FullMenuText(3)));

		fullButton_menu1.setOnTouchListener(mTouchListener);
		fullButton_menu2.setOnTouchListener(mTouchListener);
		fullButton_menu3.setOnTouchListener(mTouchListener);

		if(DxbPlayer_Control.getMenuState(4) < 0)
		{
			fullButton_menu4.setFocusable(false);
		}
		else
		{
			fullText4.setText(getResources().getString(DxbPlayer_Control.getRstring_FullMenuText(4)));
			fullButton_menu4.setOnTouchListener(mTouchListener);
		}
		mChannelUpBtn.setOnTouchListener(mTouchListener);
		mChannelDownBtn.setOnTouchListener(mTouchListener);

		// set ClickListener
		mChannelUpBtn.setOnClickListener(mChannelChangeListener);
		mChannelDownBtn.setOnClickListener(mChannelChangeListener);

		fullButton_menu1.setOnClickListener(mChannelChangeListener);
		fullButton_menu2.setOnClickListener(mChannelChangeListener);
		fullButton_menu3.setOnClickListener(mChannelChangeListener);
		fullButton_menu4.setOnClickListener(mChannelChangeListener);

		// set FocusChangeListener
		mChannelUpBtn.setOnFocusChangeListener(mFocusChangeListener);
		mChannelDownBtn.setOnFocusChangeListener(mFocusChangeListener);

		fullButton_menu1.setOnFocusChangeListener(mFocusChangeListener);
		fullButton_menu2.setOnFocusChangeListener(mFocusChangeListener);
		fullButton_menu3.setOnFocusChangeListener(mFocusChangeListener);
		fullButton_menu4.setOnFocusChangeListener(mFocusChangeListener);
	}

	OnClickListener mChannelChangeListener = new OnClickListener()
	{
		public void onClick(View v)
		{
			Log.d(TAG, "mChannelChangeListener - onClick()");

			int id = v.getId();
			int	menu_index	= getIndex_Menu(id);

			FullView_setGroupState(true);

			if(		(id == DxbPlayer_Control.getRid_FullMenu1())
				||	(id == DxbPlayer_Control.getRid_FullMenu2())
				||	(id == DxbPlayer_Control.getRid_FullMenu3())
				||	(id == DxbPlayer_Control.getRid_FullMenu4())
			)
			{
				FullView_setEvent(DxbPlayer_Control.getMenuState(menu_index));
			}
			else if( id == DxbPlayer_Control.getRid_FullCHup())
			{
				Dxb_changeChannel(CH_UP);
				mChannelUpBtn.requestFocus();
			}
			else if( id == DxbPlayer_Control.getRid_FullCHdown())
			{
				Dxb_changeChannel(CH_DOWN);
				mChannelDownBtn.requestFocus();
			}
		}
	};

	private Runnable mRunnable_KeyEvent = new Runnable()
	{
		public void run()
		{
			Log.d(TAG, "mRunnable_KeyEvent --> run()");

			if(eState_View==DXB_STATE_VIEW.LIST)
			{
				OnKeyEvent_setCurrentService(false);
			}
		}
	};

	private Runnable mRunnable_FullViewGroup = new Runnable()
	{
		public void run()
		{
			//Log.d(TAG, "mRunnable_FullViewGroup --> run()");
			
			if(		(eState_View==DXB_STATE_VIEW.FULL)
				&&	(fullViewGroup.isShown())
			)
			{
				FullView_setGroupState(false);
			}
		}
	};

	private Runnable mRunnable_enableCC = new Runnable()
	{
		public void run()
		{
			Log.e(TAG, "mRunnable_enableCC --> run()");
			if( eState_View == DXB_STATE_VIEW.FULL)
			{
				Log.e(TAG, "options_closedcaption="+DxbPlayer_Control.options_closedcaption);
				if( !fullViewGroup.isShown()){
					DxbPlayer_Control.playSubtitle(DxbPlayer_Control.options_closedcaption);
					if( DxbPlayer_Control.options_closedcaption == 1){
						if(mBitmap != null )
							closedcaption_Display.setVisibility(View.VISIBLE);
					}
					else
						closedcaption_Display.setVisibility(View.INVISIBLE);
				}
			}
		}
	};

	private Runnable mRunnable_noSignal = new Runnable()
	{
		public void run()
		{
			//Log.e(TAG, "mRunnable_noSignal --> run()"+mChannelChanging);
			if(mChannelChanging == true)
				mChannelChanging = false;
		}
	};

	public boolean onKeyDown_FullView(int keyCode, KeyEvent event)
	{
		//Log.d(TAG, "onKeyDown_FullView()");
		if(fullViewGroup.isShown())
		{
			FullView_setGroupState(true);
		}
		else
		{
			FullView_setGroupState(true);

			switch(keyCode)
			{
				case KeyEvent.KEYCODE_PAGE_UP:
					Dxb_changeChannel(CH_UP);
					mChannelUpBtn.requestFocus();
					break;

				case KeyEvent.KEYCODE_PAGE_DOWN:
					Dxb_changeChannel(CH_DOWN);
					mChannelDownBtn.requestFocus();
					break;
				
				case KeyEvent.KEYCODE_PERIOD:
					CustomAudioDialog CAudiodialog = new CustomAudioDialog(SampleATSCPlayer.this);
					CAudiodialog.show();
					GloballyPopupEnabled = true;
					break;
					
				case KeyEvent.KEYCODE_MINUS:
					CustomAsepctRatioDialog CAsepctratioDialog = new CustomAsepctRatioDialog(SampleATSCPlayer.this);
					CAsepctratioDialog.show();
					GloballyPopupEnabled = true;
					break;
			}
			return true;
		}

		int	menu_index	= getIndex_Menu(keyCode);
		switch(keyCode)
		{
			case KeyEvent.KEYCODE_1:
				fullButton_menu1.requestFocus();
				FullView_setEvent(DxbPlayer_Control.getMenuState(menu_index));
				break;

			case KeyEvent.KEYCODE_2:
				fullButton_menu2.requestFocus();
				FullView_setEvent(DxbPlayer_Control.getMenuState(menu_index));
				break;

			case KeyEvent.KEYCODE_3:
				fullButton_menu3.requestFocus();
				FullView_setEvent(DxbPlayer_Control.getMenuState(menu_index));
				break;

			case KeyEvent.KEYCODE_4:
				fullButton_menu4.requestFocus();
				FullView_setEvent(DxbPlayer_Control.getMenuState(menu_index));
				break;
			
			case KeyEvent.KEYCODE_PAGE_UP:
				mChannelUpBtn.requestFocus();
				Dxb_changeChannel(CH_UP);
				break;

			case KeyEvent.KEYCODE_PAGE_DOWN:
				mChannelDownBtn.requestFocus();
				Dxb_changeChannel(CH_DOWN);
				break;

			case KeyEvent.KEYCODE_PERIOD:
				CustomAudioDialog CAudiodialog = new CustomAudioDialog(SampleATSCPlayer.this);
				CAudiodialog.show();
				GloballyPopupEnabled = true;
				break;
				
			case KeyEvent.KEYCODE_MINUS:
				CustomAsepctRatioDialog CAsepctratioDialog = new CustomAsepctRatioDialog(SampleATSCPlayer.this);
				CAsepctratioDialog.show();
				GloballyPopupEnabled = true;
				break;				
				
			default:
				return super.onKeyDown(keyCode, event);
		}
		
		return true;
	}
	
	private int getIndex_Menu(int id)
	{
		//Log.d(TAG, "getIndex_Menu(id =" + id + ")");
		int	return_Index	= -1;
		
		switch(id)
		{
			case KeyEvent.KEYCODE_1:
			case R.id.full_4menu1:
			case R.id.full_5menu1:
			case R.id.full_6menu1:
				return_Index	= 1;
				break;

			case KeyEvent.KEYCODE_2:
			case R.id.full_4menu2:
			case R.id.full_5menu2:
			case R.id.full_6menu2:
				return_Index	= 2;
				break;

			case KeyEvent.KEYCODE_3:
			case R.id.full_4menu3:
			case R.id.full_5menu3:
			case R.id.full_6menu3:
				return_Index	= 3;
				break;

			case KeyEvent.KEYCODE_4:
			case R.id.full_4menu4:
			case R.id.full_5menu4:
			case R.id.full_6menu4:
				return_Index	= 4;
				break;

			case KeyEvent.KEYCODE_5:
			case R.id.full_5menu5:
			case R.id.full_6menu5:
				return_Index	= 5;
				break;

			case KeyEvent.KEYCODE_6:
			case R.id.full_6menu6:
				return_Index	= 6;
				break;

			default:
			break;
		}
		
		return	return_Index;
	}
	
	private void initEPG()
	{
		Log.d(TAG, "initEPG()");
		
		epgTextView_Current	= (TextView)findViewById(R.id.epg_current_text);
		epgButton_Prev		= (Button)findViewById(R.id.epg_prev);
		epgButton_Next		= (Button)findViewById(R.id.epg_next);
		epgButton_Prev.setOnFocusChangeListener(mFocusChangeListener);
		epgButton_Next.setOnFocusChangeListener(mFocusChangeListener);
		epgButton_Prev.setOnClickListener(mClickListener_epgSelect);
		epgButton_Next.setOnClickListener(mClickListener_epgSelect);
		
		mepgListView	= (ListView)findViewById(R.id.epg_list);
		mepgListView.setOnItemClickListener(ListenerOnItemClick_epgListView);
		mepgListView.setOnKeyListener(ListenerOnKey_epgListView);
		mepgListView.setOnItemSelectedListener(ListenerOnItemSelected_epgListView);
		mepgListView.setEmptyView(findViewById(R.id.epg_list_empty));

		DxbPlayer_Control.epgText_name	= (TextView)findViewById(R.id.epg_name);
		DxbPlayer_Control.epgText_detail	= (TextView)findViewById(R.id.epg_detail);

		View_EPG	= findViewById(R.id.epg_view);
		View_EPG.setOnClickListener(new OnClickListener()
		{
			public void onClick(View v)
			{
				Dxb_setViewState(false, DXB_STATE_VIEW.FULL);
			}
		});
	}

	OnClickListener mClickListener_epgSelect	= new View.OnClickListener()
	{
		public void onClick(View v)
		{
			Log.i(TAG, "mClickListener_epgSelect - onClick()");
			int id = v.getId();
			if( epgupdatedone == true)
			{
				switch(id)
				{
					case R.id.epg_prev:
						if( mMatchCase == false)
						{
							if(epgUpdate_day <= 0)
								return;
							else
								epgUpdate_day--;
						}
						else
						{
							if( epgUpdate_day+mepgFirstday <= mepgFirstday)
								return;
							else
								epgUpdate_day--;
						}
						currentFocus = 0;
					break;
					
					case R.id.epg_next:
						if( mMatchCase == false)
						{
							if(epgUpdate_day+mepgFirstday >= mepgLastday)
								return;
							else
								epgUpdate_day++;
						}
						else
						{
							if( epgUpdate_day+mepgLastday >= mepgLastday)
								return;
							epgUpdate_day++;
						}
						currentFocus = 1;
					break;
				}
				EPG_updateEPGList();
			}
		}
	};

	OnItemSelectedListener ListenerOnItemSelected_epgListView = new OnItemSelectedListener()
	{
		public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3)
		{
			Log.d(TAG, "onItemSelected()");
			DxbPlayer_Control.epgText_name.setText(DxbPlayer_Control.getEPG_Time_Detail(mepgCursor));
			DxbPlayer_Control.epgText_detail.setText(DxbPlayer_Control.getEPG_Text(mepgCursor));
		}
		
		public void onNothingSelected(AdapterView<?> arg0)
		{ 
			Log.d(TAG, "onNothingSelected()");
			// TODO Auto-generated method stub 
			//arg0.setBackgroundResource(R.drawable.memob);
		} 
	};

	OnItemClickListener ListenerOnItemClick_epgListView = new OnItemClickListener()
	{
		public void onItemClick(AdapterView<?> parent, View v, int position, long id)
		{
			//Log.d(TAG, "ListenerOnItemClick_epgListView()");
			mepgCursor.moveToPosition(position);
			mepgListView.setSelection(position);

			DxbPlayer_Control.epgText_name.setText(DxbPlayer_Control.getEPG_Time_Detail(mepgCursor));
			DxbPlayer_Control.epgText_detail.setText(DxbPlayer_Control.getEPG_Text(mepgCursor));
		}
	};
	
	OnKeyListener ListenerOnKey_epgListView = new OnKeyListener()
	{
		public boolean onKey(View v, int keyCode, KeyEvent event)
		{
			if(		(event.getAction() == KeyEvent.ACTION_UP)
				||	(event.getRepeatCount() > 0)
			)
			{
				//Log.d(TAG, "ListenerOnKey_epgListView-->onKey(keyCode=" + keyCode + ", event=" + event + ")");
				
				switch(keyCode)
				{
					case KeyEvent.KEYCODE_DPAD_UP:
					case KeyEvent.KEYCODE_DPAD_DOWN:
						break;
				}
			}
				
			if(		(event.getAction() == KeyEvent.ACTION_DOWN)
				||	(event.getRepeatCount() > 0)
			)
			{
				return false;
			}
			else
			{
				return true;
			}
		}
	};

	public boolean onKeyDown_EPGView(int keyCode, KeyEvent event)
	{
		Log.d(TAG, "onKeyDown_EPGView()");

		switch(keyCode)
		{
			case KeyEvent.KEYCODE_MENU:
				break;

			default:
				return super.onKeyDown(keyCode, event);
		}
		return true;
	}

	private class CustomAudioDialog extends Dialog implements OnClickListener
	{
		Button OkButton;
		Button CancelButton;
		Context mContext;

		ArrayAdapter<String> adapter;
		SharedPreferences pref;
		
		public CustomAudioDialog (Context context)
		{
			super(context);
			mContext = context;
		}

		@Override
		public void onCreate(Bundle savedInstanceState)
		{
			super.onCreate(savedInstanceState);

			requestWindowFeature(Window.FEATURE_LEFT_ICON);
			setContentView(R.layout.custom_dialog_new);
			getWindow().setFeatureDrawableResource(Window.FEATURE_LEFT_ICON, android.R.drawable.ic_dialog_alert);
			getWindow().setBackgroundDrawableResource(R.color.color_2);
			setTitle(getResources().getString(R.string.audio_pref_select));
			setCancelable(true);
			
			setOnCancelListener(new DialogInterface.OnCancelListener() 
			{
				public void onCancel(DialogInterface dialog)
				{
					Log.d("Log", "CustomAudioDialog --- onCancel");
					dialog.dismiss();
					GloballyPopupEnabled = false;
					mHandler_Main.removeCallbacks(mRunnable_enableCC);
					mHandler_Main.postDelayed(mRunnable_enableCC, 200);
				}
			});

			String[] audio = getResources().getStringArray(R.array.audio_pref_select_entries);
			adapter = new ArrayAdapter<String>(mContext,R.layout.custom_list_item_single_choice,audio);
			pref = getSharedPreferences(KEY_SHAREDPREF_NAME, Activity.MODE_PRIVATE);
			String list_value;
			list_value	= pref.getString(KEY_AUDIO_PREF_SELECT, "");
			if(list_value != "")
				mSelectedAP = Integer.parseInt(list_value);
			else
				mSelectedAP = DxbPlayer_Control.options_audiopref;
			
			Log.d(TAG, "mSelectedAP="+mSelectedAP);
			Log.d(TAG, "DxbPlayer_Control.options_audiopref ="+DxbPlayer_Control.options_audiopref );
		
			ListView listView = (ListView) findViewById(R.id.dialog_listview);
			listView.setAdapter(adapter);
			listView.setOnItemClickListener(dialogClickListener);
			listView.setChoiceMode(ListView.CHOICE_MODE_SINGLE);
			listView.setItemChecked(mSelectedAP, true);
			listView.setSelection(mSelectedAP);
			listView.setBackgroundColor(Color.WHITE);
			
			CancelButton = (Button)findViewById(R.id.dialog_button1);
			OkButton = (Button)findViewById(R.id.dialog_button2);
			
			CancelButton.setOnClickListener(this);
			OkButton.setOnClickListener(this);
		}

		OnItemClickListener dialogClickListener = new OnItemClickListener()
		{
			public void onItemClick(AdapterView<?> parent, View v, int position, long id)
			{
				Log.d(TAG,"dialogClickListener --- position"+position);
				selectedButton = position;
			}
		};

		public void onClick(View v)
		{
			if(v == OkButton)
			{
				Log.d(TAG,"CustomAudioDialog --- onOK");
				SharedPreferences.Editor editor = pref.edit();

				if(selectedButton == 0)
				{
					editor.putString(KEY_AUDIO_PREF_SELECT, "0");
					editor.commit();
				}
				else if(selectedButton == 1)
				{
					editor.putString(KEY_AUDIO_PREF_SELECT, "1");
					editor.commit();
				}				
				dismiss();
				GloballyPopupEnabled = false;
				mHandler_Main.removeCallbacks(mRunnable_enableCC);
				mHandler_Main.postDelayed(mRunnable_enableCC, 200);
			}
			else if(v == CancelButton)
			{
				Log.d(TAG,"CustomAudioDialog --- onCancelButton");
				cancel();
			}
		}
	}

	private class CustomAsepctRatioDialog extends Dialog implements OnClickListener
	{
		Button OkButton;
		Button CancelButton;
		Context mContext;

		ArrayAdapter<String> adapter;
		SharedPreferences pref;

		public CustomAsepctRatioDialog (Context context)
		{
			super(context);
			mContext = context;
		}

		@Override
		public void onCreate(Bundle savedInstanceState)
		{
			super.onCreate(savedInstanceState);
			Log.d("Log", "CustomAsepctRatioDialog --- onCreate");

			requestWindowFeature(Window.FEATURE_LEFT_ICON);
			setContentView(R.layout.custom_dialog_new);
			getWindow().setFeatureDrawableResource(Window.FEATURE_LEFT_ICON, android.R.drawable.ic_dialog_alert);
			getWindow().setBackgroundDrawableResource(R.color.color_2);
			setTitle(getResources().getString(R.string.screen_mode_select));
			setCancelable(true);

			setOnCancelListener(new DialogInterface.OnCancelListener() 
			{
				public void onCancel(DialogInterface dialog)
				{
					Log.d("Log", "CustomAsepctRatioDialog --- onCancel");
					dialog.dismiss();
					GloballyPopupEnabled = false;
					mHandler_Main.removeCallbacks(mRunnable_enableCC);
					mHandler_Main.postDelayed(mRunnable_enableCC, 200);
				}
			});
			String[] aspectratio = getResources().getStringArray(R.array.aspect_ratio_select_entries);
			adapter = new ArrayAdapter<String>(mContext,R.layout.custom_list_item_single_choice,aspectratio);
			pref = getSharedPreferences(KEY_SHAREDPREF_NAME, Activity.MODE_PRIVATE);
			String list_value;
			list_value	= pref.getString(KEY_ASPECT_RATIO_SELECT, "");
			if(list_value != "")
				mSelectedARatio = Integer.parseInt(list_value);
			else
				mSelectedARatio = DxbPlayer_Control.options_aspect_ratio;
			Log.d(TAG, "mSelectedARatio="+mSelectedARatio);
			Log.d(TAG, "DxbPlayer_Control.options_aspect_ratio ="+DxbPlayer_Control.options_aspect_ratio );
		
			ListView listView = (ListView) findViewById(R.id.dialog_listview);
			listView.setAdapter(adapter);
			listView.setOnItemClickListener(dialogClickListener);
			listView.setChoiceMode(ListView.CHOICE_MODE_SINGLE);
			listView.setItemChecked(mSelectedARatio, true);
			listView.setSelection(mSelectedARatio);
			listView.setBackgroundColor(Color.WHITE);
			
			CancelButton = (Button)findViewById(R.id.dialog_button1);
			OkButton = (Button)findViewById(R.id.dialog_button2);
			
			CancelButton.setOnClickListener(this);
			OkButton.setOnClickListener(this);
		}

		OnItemClickListener dialogClickListener = new OnItemClickListener()
		{
			public void onItemClick(AdapterView<?> parent, View v, int position, long id)
			{
				Log.d(TAG,"CustomAsepctRatioDialog dialogClickListener --- position"+position);
				selectedButton = position;
			}
		};

		public void onClick(View v)
		{
			if(v == OkButton)
			{
				String strScreenMode;

				Log.d(TAG,"CustomAsepctRatioDialog --- onOK");
				SharedPreferences.Editor editor = pref.edit();

				eState_ScreenSize = selectedButton;
				strScreenMode = String.format("%d", eState_ScreenSize);

				SystemProperties.set("tcc.solution.screenmode.index", strScreenMode);
				editor.putString(KEY_ASPECT_RATIO_SELECT, strScreenMode);
				editor.commit();

				DxbPlayer_Control.mDB.putAspectRatio(eState_ScreenSize);
				dismiss();
				GloballyPopupEnabled = false;
				mHandler_Main.removeCallbacks(mRunnable_enableCC);
				mHandler_Main.postDelayed(mRunnable_enableCC, 200);
			}
			else if(v == CancelButton)
			{
				Log.d(TAG,"CustomAsepctRatioDialog --- onCancelButton");
				cancel();
			}
		}
	}

	protected Dialog onCreateDialog(int id)
	{
		switch(id)
		{
			case ALERT_DIALOG_EXIT_ID:
				AlertDialog.Builder builder = new AlertDialog.Builder(this);
				
				builder.setTitle("Exit "+getResources().getString(R.string.app_name));
				builder.setIcon(getResources().getDrawable(android.R.drawable.ic_dialog_alert));
				builder.setMessage("Are you really want to quit?");
				builder.setPositiveButton("YES", new DialogInterface.OnClickListener() 
				{
					public void onClick(DialogInterface dialog, int which) 
					{
						Log.d("Log", "AlertDialog --- onOK"+which);
						dialog.dismiss();
						GloballyPopupEnabled = false;
						SampleATSCPlayer.this.finish();
					}
				});
				builder.setNegativeButton("NO", new DialogInterface.OnClickListener() 
				{
					public void onClick(DialogInterface dialog, int which) 
					{
						Log.d("Log", "AlertDialog --- onNO"+which);
						dialog.dismiss();
						if( NoServiceEnable == 1 )
						{
							mNoServicePopup = new DxbPlayer_Popup(mListView);
							mNoServicePopup.additem(setItem("      No Service"));
							mNoServicePopup.show();
							if(eState_View == DXB_STATE_VIEW.LIST)
								mNoServicePopup.update(1);
							else if(eState_View == DXB_STATE_VIEW.FULL)
								mNoServicePopup.update(2);
							else if(eState_View == DXB_STATE_VIEW.EPG)
								mNoServicePopup.update(3);
							NoServiceEnable = 2;
						}
						if( ScrambledEnable == 1 )
						{
							mScrambledPopup = new DxbPlayer_Popup(mListView);
							mScrambledPopup.additem(setItem("Scrambled Channel"));
							mScrambledPopup.show();
							if(eState_View == DXB_STATE_VIEW.LIST)
								mScrambledPopup.update(1);
							else if(eState_View == DXB_STATE_VIEW.FULL)
								mScrambledPopup.update(2);
							else if(eState_View == DXB_STATE_VIEW.EPG)
								mScrambledPopup.update(3);
							ScrambledEnable = 2;
						}
						GloballyPopupEnabled = false;
						mHandler_Main.removeCallbacks(mRunnable_enableCC);
						mHandler_Main.postDelayed(mRunnable_enableCC, 200);
					}
				});
				builder.setOnCancelListener(new DialogInterface.OnCancelListener()
				{
					public void onCancel(DialogInterface dialog)
					{
						Log.d("Log", "AlertDialog --- onCancel");
						dialog.dismiss();
						if( NoServiceEnable == 1 )
						{
							mNoServicePopup = new DxbPlayer_Popup(mListView);
							mNoServicePopup.additem(setItem("      No Service"));
							mNoServicePopup.show();
							if(eState_View == DXB_STATE_VIEW.LIST)
								mNoServicePopup.update(1);
							else if(eState_View == DXB_STATE_VIEW.FULL)
								mNoServicePopup.update(2);
							else if(eState_View == DXB_STATE_VIEW.EPG)
								mNoServicePopup.update(3);
							NoServiceEnable = 2;
						}
						if( ScrambledEnable == 1 )
						{
							mScrambledPopup = new DxbPlayer_Popup(mListView);
							mScrambledPopup.additem(setItem("Scrambled Channel"));
							mScrambledPopup.show();
							if(eState_View == DXB_STATE_VIEW.LIST)
								mScrambledPopup.update(1);
							else if(eState_View == DXB_STATE_VIEW.FULL)
								mScrambledPopup.update(2);
							else if(eState_View == DXB_STATE_VIEW.EPG)
								mScrambledPopup.update(3);
							ScrambledEnable = 2;
						}
						GloballyPopupEnabled = false;
						mHandler_Main.removeCallbacks(mRunnable_enableCC);
						mHandler_Main.postDelayed(mRunnable_enableCC, 200);
					}
				});
				mDialog = builder.create();
				mDialog.getWindow().setBackgroundDrawableResource(R.color.color_6);
				mDialog.show();
				break;
				
			default:
				break;
		}
		return super.onCreateDialog(id);
	}

	private void Dxb_setDefaultValue()
	{
		Log.d(TAG, "Dxb_setDefaultValue()");
		
		eState_View		= DXB_STATE_VIEW.FULL;
		eState_Player		= DXB_STATE_PLAYER.GENERAL;
	}
	
	private void Dxb_setViewState(boolean isRefresh, DXB_STATE_VIEW eChange_state)
	{
		Log.d(TAG, "Dxb_setViewState(eChange_state="+eChange_state+")");
		
		if( (!isRefresh)
			&&	(eState_View == eChange_state)
		)
		{
			Log.v(TAG, "Fail : Dxb_setViewState() - state error");
			return;
		}
		
		DxbPlayer_Control.playSubtitle(0);	//off(0)
		closedcaption_Display.setVisibility(View.INVISIBLE);
		
		if(	(eState_View == DXB_STATE_VIEW.LIST)
			&&	(eChange_state == DXB_STATE_VIEW.FULL)
		)
		{
			if(mChannelsCursor.getCount() <= 0)
			{
				if(DxbPlayer_Control.isVisible_Tab())
				{
					int	iCurrentTab	= tabHost.getCurrentTab();
					if(		(iCurrentTab == 0)
						&&	(mTotalChannel_RadioCount > 0)
					)
					{
						tabHost.setCurrentTab(1);
					}
					else if(mTotalChannel_TvCount > 0)
					{
						tabHost.setCurrentTab(0);
					}
				}
			}
		}
		
		Dxb_updateBackgroundLayout(eChange_state);
		
		if(eChange_state == DXB_STATE_VIEW.FULL)
		{
			FullView_setGroupState(true);
		}
		else
		{
			imageIndicator.setVisibility(View.VISIBLE);
			timeIndicator.setVisibility(View.VISIBLE);
			Dxb_Signal_display();
		}

		int	layoutWidth, layoutHeight;
		int	startX, startY;
		String strScreenMode;
		if(eChange_state == DXB_STATE_VIEW.LIST)
		{
			if(NoSignalEnable > 6 && mNoSignalPopup!= null && mNoSignalPopup.isshown())
			{
				mNoSignalPopup.dismiss();
				mNoSignalPopup = null;
				NoSignalEnable = 5;
			}
			
			if( ScrambledEnable == 2 && mScrambledPopup!= null && mScrambledPopup.isshown())
			{
				mScrambledPopup.update(1);
			}

			if( NoServiceEnable == 2 && mNoServicePopup!= null&& mNoServicePopup.isshown())
			{
				mNoServicePopup.update(1);
			}
			// screen mode Full on listview
//			SystemProperties.set("tcc.solution.screenmode.index", "2");

			layoutWidth		= Dxb_getToPosition(COORDINATES_X, 280);
			layoutHeight	= Dxb_getToPosition(COORDINATES_Y, 210);

			startX	= Dxb_getToPosition(COORDINATES_X, 507);
			startY	= Dxb_getToPosition(COORDINATES_Y, 87);

			SystemProperties.set("tcc.solution.preview", "2");
			DxbPlayer_Control.releaseSurface();

			View child = (View)findViewById(R.id.surface_view);
			AbsoluteLayout.LayoutParams lp = new AbsoluteLayout.LayoutParams(
					layoutWidth,
					layoutHeight, startX, startY);

			child.setLayoutParams(lp);

			mListView.requestFocus();

			if( mChannelsCursor != null )
			{
				Log.d(TAG, "mChannelsCursor.getPosition()="+mChannelsCursor.getPosition());
				mListView.setSelectionFromTop(mChannelsCursor.getPosition(), LISTCENTER_Y);
			}
		}
		else if(eChange_state == DXB_STATE_VIEW.FULL)
		{
			if(NoSignalEnable > 6 && mNoSignalPopup!= null && mNoSignalPopup.isshown())
			{
				mNoSignalPopup.dismiss();
				mNoSignalPopup = null;
				NoSignalEnable = 5;
			}
			Log.e(TAG, "ScrambledEnable"+ScrambledEnable);
			if( ScrambledEnable == 2 && mScrambledPopup!= null && mScrambledPopup.isshown())
			{
				mScrambledPopup.update(2);
			}
			else if( ScrambledEnable == 1 && mScrambledPopup == null )
			{
				mScrambledPopup = new DxbPlayer_Popup(mListView);
				mScrambledPopup.additem(setItem("Scrambled Channel"));
				mScrambledPopup.show();
				ScrambledEnable = 2;
				Log.e(TAG, "ScrambledEnable"+ScrambledEnable);
			}

			if( NoServiceEnable == 2 && mNoServicePopup!= null && mNoServicePopup.isshown())
			{
				mNoServicePopup.update(2);
			}
			else if( NoServiceEnable == 1 && mNoServicePopup == null)
			{
				mNoServicePopup = new DxbPlayer_Popup(mListView);
				mNoServicePopup.additem(setItem("      No Service"));
				mNoServicePopup.show();
				NoServiceEnable = 2;
			}
			// screen mode setting value on fullview
//			strScreenMode = String.format("%d", eState_ScreenSize);

//			SystemProperties.set("tcc.solution.screenmode.index", strScreenMode);
			
			layoutWidth		= Dxb_getToPosition(COORDINATES_X, 800);
			layoutHeight	= Dxb_getToPosition(COORDINATES_Y, 480);

			startX	= Dxb_getToPosition(COORDINATES_X, 0);
			startY	= Dxb_getToPosition(COORDINATES_Y, 0);

			SystemProperties.set("tcc.solution.preview", "0");
			DxbPlayer_Control.releaseSurface();

			View child = (View)findViewById(R.id.surface_view);
			AbsoluteLayout.LayoutParams lp = new AbsoluteLayout.LayoutParams(
					layoutWidth,
					layoutHeight, startX, startY);

			child.setLayoutParams(lp);

			fullButton_menu1.requestFocus();
		}
		else if(eChange_state == DXB_STATE_VIEW.EPG)
		{
			if(NoSignalEnable > 6 && mNoSignalPopup!= null && mNoSignalPopup.isshown())
			{
				mNoSignalPopup.dismiss();
				mNoSignalPopup = null;
				NoSignalEnable = 5;
			}

			if( ScrambledEnable == 2 && mScrambledPopup!= null && mScrambledPopup.isshown())
			{
				mScrambledPopup.update(3);
			}

			if( NoServiceEnable == 2 && mNoServicePopup!= null && mNoServicePopup.isshown())
			{
				mNoServicePopup.update(3);
			}
			// screen mode Full on epgview
//			SystemProperties.set("tcc.solution.screenmode.index", "2");
			
			layoutWidth		= Dxb_getToPosition(COORDINATES_X, 280);
			layoutHeight	= Dxb_getToPosition(COORDINATES_Y, 200);

			startX	= Dxb_getToPosition(COORDINATES_X, 507);
			startY	= Dxb_getToPosition(COORDINATES_Y, 99);

			SystemProperties.set("tcc.solution.preview", "2");
			DxbPlayer_Control.releaseSurface();

			View child = (View)findViewById(R.id.surface_view);
			AbsoluteLayout.LayoutParams lp = new AbsoluteLayout.LayoutParams(
					layoutWidth,
					layoutHeight, startX, startY);

			child.setLayoutParams(lp);

			DxbPlayer_Control.epgPosition = 0;
			epgUpdate_day	= 0;

			if( DxbPlayer_Control.epgFirst == 1 && DxbPlayer_Control.epgLast == 0)
				currentFocus = 1;
			else if( DxbPlayer_Control.epgFirst == 0 && DxbPlayer_Control.epgLast == 1 )
				currentFocus = 0;
			else if( DxbPlayer_Control.epgFirst == 0 && DxbPlayer_Control.epgLast == 0 ) 
				currentFocus = 1;

			setEPGProgramName(DxbPlayer_Control.getModulation(), DxbPlayer_Control.getLogicalCHNumber(),DxbPlayer_Control.getLogicalCHMinorNumber(),DxbPlayer_Control.getServiceName());
			EPG_updateEPGList();
		}
		Dxb_updateScreen();
	}
	
	private boolean Dxb_backView(int keyCode, KeyEvent event, DXB_STATE_VIEW state)
	{
		//Log.d(TAG, "DxbBack_View(state="+state+")");
		
		if(state == DXB_STATE_VIEW.FULL)
		{
			if(fullViewGroup.isShown())
			{
				FullView_setGroupState(false);
			}
			else
			{
				mHandler_Main.removeCallbacks(mRunnable_enableCC);
				DxbPlayer_Control.playSubtitle(0);
				closedcaption_Display.setVisibility(View.INVISIBLE);

				if(mNoSignalPopup!= null && mNoSignalPopup.isshown())
				{
					mNoSignalPopup.dismiss();
					mNoSignalPopup = null;
					NoSignalEnable = 1;
				}
				if(mScrambledPopup!= null && mScrambledPopup.isshown())
				{
					mScrambledPopup.dismiss();
					mScrambledPopup = null;
					ScrambledEnable = 1;
				}
				if(mNoServicePopup!= null && mNoServicePopup.isshown())
				{
					mNoServicePopup.dismiss();
					mNoServicePopup = null;
					NoServiceEnable = 1;
				}
				showDialog(ALERT_DIALOG_EXIT_ID);
			}
		}
		else if(eState_View == DXB_STATE_VIEW.LIST || eState_View == DXB_STATE_VIEW.EPG)
		{
			Dxb_setViewState(false, DXB_STATE_VIEW.FULL);
		}
		
		return true;
	}
	
	private void Dxb_updateScreen()
	{
		Log.d(TAG, "Dxb_updateScreen() mTotalChannel_CurrentCount="+mTotalChannel_CurrentCount);
		View			View	= null;
		ImageView		imageView = null;
		TextView	textView_list = null, textView_count = null, textView_info = null, textView = null;
		int	image = 0, text = 0, list_text = 0, info_text = 0;
		String strServiceName = "";
		final int TEXT = 111;

		if(mTotalChannel_CurrentCount <= 0)
		{
			image	= R.drawable.dxb_portable_channel_no_channel_img;
			text	= R.string.no_channel;
				
			if(DxbPlayer_Control.getTab() == 1)
			{
				list_text	= R.string.if_you_want_to_use_radio_scan_channel;
			}
			else
			{
				list_text	= R.string.if_you_want_to_use_tv_scan_channel;
			}
			info_text	= list_text;
				
			fullTitle.setText(R.string.app_name);
			findViewById(DxbPlayer_Control.getRid_ListTitleImage()).setVisibility(View.INVISIBLE);

			textView = (TextView)findViewById(DxbPlayer_Control.getRid_ListTitle());
			textView.setText("");
		}
		else
		{
			if( DxbPlayer_Control.getModulation() == MODULATION_VSB)
				strServiceName = DxbPlayer_Control.getLogicalCHNumber()+"-"+DxbPlayer_Control.getLogicalCHMinorNumber()+"       "+DxbPlayer_Control.getServiceName();
			else
				strServiceName = DxbPlayer_Control.getLogicalCHNumber()+"       "+DxbPlayer_Control.getServiceName();
			fullTitle.setText(strServiceName);

			ListView_updateServiceName(DxbPlayer_Control.getplayServiceType()-1);
			text	= TEXT;
			/*
			if(level_Signal < 0)
			{
				image	= R.drawable.dxb_portable_channel_weak_signal_img;
				text	= R.string.weak_signal;
				info_text	= R.string.receiving_signal_strength_is_weak;
			}
			else
			{
				if(DxbPlayer_Control.isRadio())
				{
					image	= R.drawable.dxb_portable_channel_radio_img;
				}
				text	= TEXT;
			}
			*/
		}
		
		// Select - Display area
		if(eState_View == DXB_STATE_VIEW.LIST)
		{
			imageView	= (ImageView)findViewById(DxbPlayer_Control.getRid_ListImage());
			View		= View_List;
			
			textView_list	= (TextView)findViewById(DxbPlayer_Control.getRid_ListMessage());
			textView_count	= (TextView)findViewById(DxbPlayer_Control.getRid_ListCount());
		}
		else if(eState_View == DXB_STATE_VIEW.FULL)
		{
			imageView	= (ImageView)findViewById(DxbPlayer_Control.getRid_FullImage());
			View		= View_Full;
			
			textView_info	= (TextView)findViewById(DxbPlayer_Control.getRid_FullText());
		}
		else if(eState_View == DXB_STATE_VIEW.EPG)
		{
			imageView	= (ImageView)findViewById(DxbPlayer_Control.getRid_EPGImage());
			View		= View_EPG;
		}

		// (Video area) - display image
		if(imageView != null)
		{
			// 0-play video, !0-display image
			if(image == 0)
			{
				imageView.setVisibility(android.view.View.GONE);
				
				View.setBackgroundColor(0x00000000);
			}
			else
			{
				imageView.setBackgroundResource(image);
				imageView.setVisibility(android.view.View.VISIBLE);
					
				View.setBackgroundColor(0xFF101010);
			}
		}
		
		// (bottom of video area) - display list_count
		if(textView_count != null)
		{
			// 0-not display, !0-display
			if(text != 0)
			{
				// TEXT-getResources, !TEXT-fixed
				if(text == TEXT)
				{
					if(DxbPlayer_Control.getTab() == 1)
					{
						textView_count.setText(getResources().getString(R.string.radio_channel) + " : " + mTotalChannel_CurrentCount);
					}
					else
					{
						textView_count.setText(getResources().getString(R.string.tv_channel) + " : " + mTotalChannel_CurrentCount);
					}
				}
				else
				{
					textView_count.setText(getResources().getString(text));
				}
			}
		}
				
		// (channel list area) - info display (scan)
		if(textView_list != null)
		{
			// 0-hide, !0-display text
			if(list_text == 0)
			{
				textView_list.setVisibility(android.view.View.GONE);
			}
			else
			{
				textView_list.setText(getResources().getString(list_text));
				textView_list.setVisibility(android.view.View.VISIBLE);
			}
		}

		// (full View area) - info display (scan, weak signal, ...)
		if(textView_info != null)
		{
			if(info_text == 0)
			{
				textView_info.setVisibility(android.view.View.GONE);
			}
			else
			{
				textView_info.setText(getResources().getString(info_text));
				textView_info.setVisibility(android.view.View.VISIBLE);
			}
		}
	}
	
	private boolean Dxb_isShown_Signal()
	{
		return	isShown_Signal;
	}

	private void Dxb_Signal_create()	//check --> eva
	{
		mHandler_Main.postDelayed(mRunnable_Signal, 500);
	}

	private void Dxb_Signal_display()
	{
		//Log.d(TAG, "Dxb_Signal_display()"+Dxb_isShown_Signal());

		mHandler_Main.removeCallbacks(mRunnable_Signal);
		
		if(		(mTotalChannel_TvCount > 0)
			||	(mTotalChannel_RadioCount > 0)
		)
		{
			if(!Dxb_isShown_Signal())
			{
				mWindowManager.addView(mImage_Signal, mWindowParams);
				isShown_Signal	= true;
			}
			
			mWindowManager.updateViewLayout(mImage_Signal, mWindowParams);	
		}
		mHandler_Main.postDelayed(mRunnable_Signal, 500);
	}
	
	private void Dxb_Signal_hide()
	{
		//Log.d(TAG, "Dxb_Signal_hide()"+Dxb_isShown_Signal());
		Dxb_Signal_release();
		
		mHandler_Main.postDelayed(mRunnable_Signal, 500);
	}

	private void Dxb_Signal_release()
	{
		//Log.d(TAG, "Dxb_Signal_release()"+Dxb_isShown_Signal());
		mHandler_Main.removeCallbacks(mRunnable_Signal);
		
		if(Dxb_isShown_Signal())
		{
			mWindowManager.removeView(mImage_Signal);
			isShown_Signal	= false;
		}
	}
	
	private void Dxb_updateBackgroundLayout(DXB_STATE_VIEW eChange_state)
	{
		Log.d(TAG, "Dxb_updateBackgroundLayout(eChange_state=" + eChange_state + ")");
		
		//	(All)GONE
		{
			if(eChange_state != DXB_STATE_VIEW.LIST)
			{
				findViewById(DxbPlayer_Control.getRid_LayoutList()).setVisibility(View.GONE);
			}
			
			if(eChange_state != DXB_STATE_VIEW.FULL)
			{
				findViewById(DxbPlayer_Control.getRid_LayoutFull()).setVisibility(View.GONE);
			}

			if(eChange_state != DXB_STATE_VIEW.EPG)
			{
				findViewById(R.id.layout_epg).setVisibility(View.GONE);
			}
		}
		
		if(eChange_state == DXB_STATE_VIEW.LIST)
		{
			findViewById(R.id.id_channel_bg).setBackgroundResource(DxbPlayer_Control.getRdrawable_ListBG());
			findViewById(DxbPlayer_Control.getRid_LayoutList()).setVisibility(View.VISIBLE);
		}
		else if(eChange_state == DXB_STATE_VIEW.FULL)
		{
			findViewById(R.id.id_channel_bg).setBackgroundColor(0xFF000000);
			
			fullViewGroup.setVisibility(View.VISIBLE);
			findViewById(DxbPlayer_Control.getRid_LayoutFull()).setVisibility(View.VISIBLE);
		}
		else if(eChange_state == DXB_STATE_VIEW.EPG)
		{
			findViewById(R.id.id_channel_bg).setBackgroundResource(DxbPlayer_Control.getRdrawable_epgBG());
			findViewById(R.id.layout_epg).setVisibility(View.VISIBLE);
		}
		
		eState_View			= eChange_state;
	}
	
	private void Dxb_updateToast(String message)
	{
		if(mToast == null)
		{
			mToast = Toast.makeText(this, message, 400);
		}
		else
		{
			mToast.show();
			mToast.setText(message);
		}

		mToast.show();
	}
	
	private void Dxb_changeChannel(int state)
	{
		Log.d(TAG, "Dxb_changeChannel()");
		if( mBitmap != null )
		{
			mBitmap.recycle();
			mBitmap = null;
		}
		if(mChannelChanging == true)
			mChannelChanging = false;
		switch(state)
		{
			case CH_UP:
				if(mTotalChannel_CurrentCount > 0)
				{
					if(DxbPlayer_Control.tvPosition == 0)
					{
						DxbPlayer_Control.tvPosition = mTotalChannel_TvCount-1;
					}
					else
					{
						DxbPlayer_Control.tvPosition--;
					}				

					mChannelsCursor.moveToPosition(DxbPlayer_Control.tvPosition);
				}
				break;
			
			case CH_DOWN:
				if(mTotalChannel_CurrentCount > 0)
				{
					if(DxbPlayer_Control.tvPosition == mTotalChannel_TvCount-1)
					{
						DxbPlayer_Control.tvPosition = 0;
					}
					else
					{
						DxbPlayer_Control.tvPosition++;
					}
					
					mChannelsCursor.moveToPosition(DxbPlayer_Control.tvPosition);
				}
				break;
			
			default:
				return;
		}
		setChannel();
		mListView.setSelectionFromTop(mChannelsCursor.getPosition(), LISTCENTER_Y);
	}

	private int Dxb_getToPosition(int co, int position)
	{
		//Log.d(TAG, "Dxb_getToPosition()");
		int	return_position	= 0;
		
		if(co == COORDINATES_X)
		{
			return_position	= position * mDisplayWidth / 800;
		}
		else if(co == COORDINATES_Y)
		{
			return_position	= position * mDisplayHeight / 480;
		}
		
		return return_position;
	}
	
	private void ListView_updateChannelListView(int iTabNumber)
	{
		Log.d(TAG, "ListView_updateChannelListView()");

		mChannelsCursor = DxbPlayer_Control.getChannels(iTabNumber);
		if(mChannelsCursor != null )
		{
			if(iTabNumber == 0)
			{
				mTotalChannel_TvCount = mChannelsCursor.getCount();
			}
			else if(iTabNumber == 1)
			{
				mTotalChannel_RadioCount	= mChannelsCursor.getCount();
			}
			mTotalChannel_CurrentCount	=  mChannelsCursor.getCount();

			if(madapter == null)
			{
				madapter = new DxbAdapter_Service(this, R.layout.dxb_list_row, mChannelsCursor, new String[] {}, new int[] {});
			}
			else
			{
				madapter.changeCursor(mChannelsCursor);
			}
			
			mListView.setAdapter(madapter);
			
			if(iTabNumber == 0)
			{
				mChannelsCursor.moveToPosition(DxbPlayer_Control.tvPosition);
			}
			else if(iTabNumber == 1)
			{
				mChannelsCursor.moveToPosition(DxbPlayer_Control.radioPosition);
			}

			ListView_updateChannelCount(iTabNumber, mTotalChannel_CurrentCount);
		}
	}

	private void ListView_updateChannelCount(int iTab, int iCount)
	{
		Log.i(TAG, "ListView_updateChannelCount(iTab=" + iTab + ", iCount=" + ")");

		String strCount = "";
		TextView	textView;

		if(iTab == 0)
		{
			strCount = getResources().getString(R.string.tv_channel) + " : " + iCount;
		}
		else
		{
			strCount = getResources().getString(R.string.radio_channel) + " : " + iCount;
		}

		textView = (TextView)findViewById(DxbPlayer_Control.getRid_ListCount());
		textView.setText(strCount);
	}

	private void ListView_updateServiceName(int iTab)
	{
		Log.d(TAG, "ListView_updateServiceName() : iTab = "+iTab);
		String strServiceNum = "";
		String strServiceName = "";
		TextView	textView;
		
		Log.d(TAG, "serviceName="+DxbPlayer_Control.getServiceName());
		
		if(iTab == 0)
		{
			if( DxbPlayer_Control.getModulation() == MODULATION_VSB)
				strServiceNum = DxbPlayer_Control.getLogicalCHNumber()+" - "+DxbPlayer_Control.getLogicalCHMinorNumber();

			else
				strServiceNum = DxbPlayer_Control.getLogicalCHNumber()+"";

			strServiceName = DxbPlayer_Control.getServiceName();
			findViewById(DxbPlayer_Control.getRid_ListTitleImage()).setVisibility(View.VISIBLE);
			findViewById(DxbPlayer_Control.getRid_ListTitleImage()).setBackgroundResource(R.drawable.dxb_portable_playing_tv_icon);			
		}
		else if(iTab == 1)
		{
			strServiceNum = DxbPlayer_Control.getLogicalCHNumber()+"";
			strServiceName = DxbPlayer_Control.getServiceName();
			findViewById(DxbPlayer_Control.getRid_ListTitleImage()).setVisibility(View.VISIBLE);
			findViewById(DxbPlayer_Control.getRid_ListTitleImage()).setBackgroundResource(R.drawable.dxb_portable_playing_radio_icon);			
		}
		
		textView = (TextView)findViewById(DxbPlayer_Control.getRid_ListTitle());
		textView.setText(strServiceNum+" "+strServiceName);
	}

	private void FullView_setGroupState(boolean state)
	{
		Log.d(TAG, "FullView_setGroupState(state=" + state + ")"+fullViewGroup.isShown());
		if(state)
		{
			mHandler_Main.removeCallbacks(mRunnable_FullViewGroup);
			
			Dxb_Signal_display();
			
			if(!fullViewGroup.isShown())
			{
				DxbPlayer_Control.playSubtitle(0);
				closedcaption_Display.setVisibility(View.INVISIBLE);
				fullViewGroup.setVisibility(View.VISIBLE);
				fullButton_menu1.requestFocus();
				imageIndicator.setVisibility(View.VISIBLE);
				timeIndicator.setVisibility(View.VISIBLE);
			}
			
			mHandler_Main.postDelayed(mRunnable_FullViewGroup, 5000);
		}
		else
		{
			mHandler_Main.removeCallbacks(mRunnable_FullViewGroup);
			//Log.d(TAG, "FullView_setGroupState(state=" + state + ")"+fullViewGroup.isShown());			
			//closedcaption_Display.setVisibility(View.VISIBLE);

			fullViewGroup.setVisibility(View.INVISIBLE);
			imageIndicator.setVisibility(View.INVISIBLE);
			timeIndicator.setVisibility(View.INVISIBLE);

			Dxb_Signal_hide();

			if( GloballyPopupEnabled == false)
			{
				mHandler_Main.removeCallbacks(mRunnable_enableCC);
				mHandler_Main.postDelayed(mRunnable_enableCC, 200);
			}
		}
	}

	private void FullView_setEvent(int event)
	{
		Log.d(TAG, "FullView_setEvent()");
		
		switch(event)
		{
			case MENU_CHANNEL:
				Dxb_setViewState(false, DXB_STATE_VIEW.LIST);
				break;

			case MENU_EPG:
				Dxb_setViewState(false, DXB_STATE_VIEW.EPG);
				break;

			case MENU_SCREEN:
				{
					String strScreenMode;

					SharedPreferences pref = getSharedPreferences(KEY_SHAREDPREF_NAME, Activity.MODE_PRIVATE);
					SharedPreferences.Editor editor = pref.edit();

					String list_value;
					list_value	= pref.getString(KEY_ASPECT_RATIO_SELECT, "");
					if(list_value != "")
						mSelectedARatio = Integer.parseInt(list_value);
					else
						mSelectedARatio = DxbPlayer_Control.options_aspect_ratio;
					Log.d(TAG, "mSelectedARatio="+mSelectedARatio);
					Log.d(TAG, "DxbPlayer_Control.options_aspect_ratio ="+DxbPlayer_Control.options_aspect_ratio );
					eState_ScreenSize = mSelectedARatio;

					eState_ScreenSize = (eState_ScreenSize+1)%3;
					strScreenMode = String.format("%d", eState_ScreenSize);

					SystemProperties.set("tcc.solution.screenmode.index", strScreenMode);
					editor.putString(KEY_ASPECT_RATIO_SELECT, strScreenMode);
					editor.commit();

					DxbPlayer_Control.mDB.putAspectRatio(eState_ScreenSize);
					Log.i(TAG, "mDB.aspectRatio +()"+eState_ScreenSize);
				}
				break;

			case MENU_OPTION:
				{
					@SuppressWarnings("rawtypes")
					Class	class_Option	= DxbPlayer_Control.getClass_Option();
					if(class_Option != null)
					{
						DxbPlayer_Control.releaseSurface();
						DxbPlayer_Control.intentSubActivity = new Intent( this, class_Option);
						startActivity( DxbPlayer_Control.intentSubActivity );
					}
				}
				break;

			default:
				Log.d(TAG, "(Menu) - Error!");
				break;
		}
	}
	
	private void setEPGProgramName(int Modulation, int CHMajorNum, int CHMinorNum, String szName)
	{
		if( Modulation > 0 )
		{
			if(findViewById(R.id.epg_pnumber_text) != null)
			{
				if(Modulation == MODULATION_VSB)
				{
					if( CHMajorNum == 0 && CHMinorNum == 0 )
						((TextView)findViewById(R.id.epg_pnumber_text)).setText("");	
					else
						((TextView)findViewById(R.id.epg_pnumber_text)).setText(CHMajorNum+" - "+CHMinorNum);
				}
				else if( Modulation == MODULATION_QAM)
					((TextView)findViewById(R.id.epg_pnumber_text)).setText(CHMajorNum+"");
			}

			if(findViewById(R.id.epg_pname_text) != null)
			{
				((TextView)findViewById(R.id.epg_pname_text)).setText(szName);
			}
		}
		else
		{
			((TextView)findViewById(R.id.epg_pnumber_text)).setText("");			
			((TextView)findViewById(R.id.epg_pname_text)).setText("");
		}
	}
	
	private void EPG_updatePrevBt(int iDay)
	{
		Log.i(TAG, "EPG_updatePrevBt(iDay = " + iDay + ")");

		if(iDay <= 0)
		{
			epgButton_Prev.setBackgroundResource(R.drawable.dxb_portable_epg_left_btn_d);
			epgButton_Prev.setFocusable(false);

			currentFocus	= 1;	// 1-next
		}
		else
		{
			epgButton_Prev.setBackgroundResource(R.drawable.dxb_portable_epg_left_btn_n);
			epgButton_Prev.setFocusable(true);
		}
	}

	private void EPG_updateNextBt(Cursor cursor)
	{
		Log.i(TAG, "EPG_updateNextBt()");

		if(DxbPlayer_Control.mTotalEPGCount <= 0)
		{
			epgButton_Next.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_d);
			epgButton_Next.setFocusable(false);
		}
		else if(epgUpdate_day >= 0)
		{
			int selected_day = 0;

			if( mMatchCase == false )
				selected_day = mepgFirstday;
			else
			{
				if( epgUpdate_day > 0 )
					selected_day = mepgLastday;
				else
					selected_day = mepgFirstday;
			}
			if(nextEPG != null)
			{
				nextEPG.close();
				nextEPG = null;
			}

			nextEPG = DxbPlayer_Control.getEPG_Schedule_byDay(selected_day + epgUpdate_day);
			if(nextEPG == null)
			{
				epgButton_Next.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_d);
				epgButton_Next.setFocusable(false);

				currentFocus	= 0;	// 0-prev
			}
			else if(nextEPG.getCount() <= 0)
			{
				epgButton_Next.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_d);
				epgButton_Next.setFocusable(false);

				currentFocus	= 0;	// 0-prev
			}
			else
			{
				epgButton_Next.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_n);
				epgButton_Next.setFocusable(true);
			}
		}
	}

	private void EPG_updateFocus()
	{
		//Log.d(TAG, "EPG_updateFocus="+currentFocus+"!"+epgUpdate_day+"!"+DxbPlayer_Control.mTotalEPGCount);
		if(DxbPlayer_Control.mTotalEPGCount > 0)
		{
			switch(currentFocus)
			{
				case 0:	// Prev
					if(epgUpdate_day > 0)
					{
						epgButton_Prev.requestFocus();
						epgButton_Prev.setBackgroundResource(R.drawable.dxb_portable_epg_left_btn_f);
					}
				break;

				case 1:	// Next
					epgButton_Next.requestFocus();
					epgButton_Next.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_f);
				break;

				default:
				break;
			}
		}
	}

	private void EPG_updateList(Cursor cursor)
	{
		Log.i(TAG, "EPG_updateList()"+eState_View);
		
		if(cursor != null)
		{
			DxbPlayer_Control.mTotalEPGCount = mepgCursor.getCount();
			if(DxbPlayer_Control.mTotalEPGCount <= 0)
			{
				DxbPlayer_Control.epgPosition	= 0;
			}
			else if(DxbPlayer_Control.epgPosition >= DxbPlayer_Control.mTotalEPGCount)
			{
				DxbPlayer_Control.epgPosition	= DxbPlayer_Control.mTotalEPGCount-1;
			}
			
			Cursor c = mepgCursor;
			c.moveToPosition(DxbPlayer_Control.epgPosition);

			DxbAdapter_EPG cAdapter_EPG	= new DxbAdapter_EPG(this, R.layout.dxb_epg_row, c, new String[] {}, new int[] {});
			mepgListView.setAdapter(cAdapter_EPG);

			//epgListView.requestFocus();
			//epgListView.setSelection(DxbPlayer_Control.epgPosition);
		}
	}

	private void EPG_updateEPGList()
	{
		Log.d(TAG, "EPG_updateEPGList()");
		int selected_day = 0;

		if(DxbPlayer_Control.openEPGDB() == false)
		{
			Log.e(TAG, "CANNOT open EPG database !!!");
			epgUpdate_day = 0;
			if( mepgCursor != null)
			{
				mepgCursor.close();
				mepgCursor = null;
			}
			//return;
		}
		else
		{
			if( mMatchCase == false )
				selected_day = mepgFirstday;
			else
			{
				if( epgUpdate_day > 0 )
					selected_day = mepgLastday;
				else
					selected_day = mepgFirstday;
			}

			mepgCursor = DxbPlayer_Control.getEPG_Schedule_byDay(selected_day + epgUpdate_day);
			//mepgCursor = DxbPlayer_Control.getEPG_Schedule();
		}

		EPG_updatePrevBt(epgUpdate_day);
		EPG_updateList(mepgCursor);
		EPG_updateNextBt(mepgCursor);	// EPG_updateList() --> Next (Use, DxbPlayer_Control.mTotalEPGCount)
		EPG_updateFocus();

		//DxbPlayer_Control.closeEPGDB();
	}
	
	void setChannel()
	{
		Log.d(TAG, "setChannel()");

		mHandler_Main.removeCallbacks(mRunnable_noSignal);
		mHandler_Main.postDelayed(mRunnable_noSignal, 5000);

		mChannelChanging = true;

		if(NoSignalEnable > 6 && mNoSignalPopup!= null && mNoSignalPopup.isshown())
		{
			mNoSignalPopup.dismiss();
			mNoSignalPopup = null;
			NoSignalEnable = 0;
		}
		if(NoServiceEnable == 2 &&  mNoServicePopup!= null && mNoServicePopup.isshown())
		{
			mNoServicePopup.dismiss();
			mNoServicePopup = null;
			NoServiceEnable = 0;
		}
		if(ScrambledEnable == 2 &&  mScrambledPopup!= null && mScrambledPopup.isshown())
		{
			mScrambledPopup.dismiss();
			mScrambledPopup = null;
			ScrambledEnable = 0;
		}
		
		if(mTotalChannel_CurrentCount > 0)
		{
			Log.d(TAG, "mChannelsCursor.getPosition()="+mChannelsCursor.getPosition()+", current_cursor="+ current_cursor);
			
			if(DxbPlayer_Control.getTab() == 1)
			{
				DxbPlayer_Control.radioPosition = mChannelsCursor.getPosition();
				Log.d(TAG, "radioPosition="+DxbPlayer_Control.radioPosition);
				DxbPlayer_Control.mDB.putPosition(1, DxbPlayer_Control.radioPosition);
			}
			else
			{
				DxbPlayer_Control.tvPosition = mChannelsCursor.getPosition();
				Log.d(TAG, "tvPosition="+DxbPlayer_Control.tvPosition);
				DxbPlayer_Control.mDB.putPosition(0, DxbPlayer_Control.tvPosition);
			}
			epgupdatedone = false;
			
			DxbPlayer_Control.setChannel(mChannelsCursor);
			current_cursor = mChannelsCursor.getPosition();
			DxbPlayer_Control.setAudio(DxbPlayer_Control.options_audio);
			DxbPlayer_Control.setAudioDualMono(DxbPlayer_Control.options_dual_audio);
			DxbPlayer_Control.setCCServiceNum(DxbPlayer_Control.options_captionsvcnum);
			DxbPlayer_Control.enableCC(DxbPlayer_Control.options_closedcaption);	//off(0)
			
			sectionID	= DxbPlayer_Control.getRid_Indicator_Section();
			if(sectionID > 0)
			{
				imageIndicator.setImageResource(sectionID);
			}
			
			Dxb_updateScreen();
			SampleATSCPlayer.mListView.invalidateViews();
		}
		else
		{
			eState_Player = DXB_STATE_PLAYER.GENERAL;
		}
	}
}
