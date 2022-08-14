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

package com.telechips.android.dvb;

import java.util.ArrayList;
import java.util.List;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.content.*;
import android.os.Handler;
import android.text.format.Time;
import android.util.DisplayMetrics;
import android.view.*;
import android.view.inputmethod.InputMethodManager;
import android.widget.*;

import com.telechips.android.dvb.DxbView_Full_Record.TYPE;
import com.telechips.android.dvb.option.DishSetupActivity;
import com.telechips.android.dvb.player.*;
import com.telechips.android.dvb.player.DxbPVR.LOCAL_STATE;
import com.telechips.android.dvb.player.DxbPVR.REC_STATE;
import com.telechips.android.dvb.util.DxbUtil;

public class DxbView_Full extends DxbView implements View.OnClickListener, View.OnFocusChangeListener, View.OnTouchListener,
														SeekBar.OnSeekBarChangeListener, DxbView_Full_Menu.OnEventListener
{
	public static final int EVENT_NULL         = -1;
	public static final int EVENT_CHANNEL      = 0;
	public static final int EVENT_SCREENMODE   = 1;
	public static final int EVENT_EPG          = 2;
	public static final int EVENT_CAPTURE      = 3;
	public static final int EVENT_RECORD       = 4;
	public static final int EVENT_OPTION       = 5;
	public static final int EVENT_INFORMATION  = 6;
	public static final int EVENT_TTX          = 7;
	public static final int EVENT_FAVORITE     = 8;
	public static final int EVENT_CHANNEL_UP   = 9;
	public static final int EVENT_CHANNEL_DOWN = 10;
	public static final int EVENT_FAST_FORWARD = 11;
	public static final int EVENT_REWIND       = 12;
	public static final int EVENT_PLAY_PAUSE   = 13;
	public static final int EVENT_PLAY         = 14;
	public static final int EVENT_PAUSE        = 15;
	public static final int EVENT_STOP         = 16;
	public static final int EVENT_AUDIO_CHANGE = 17;
	public static final int EVENT_SUBT_CHANGE  = 18;
	public static final int EVENT_UI_SHOW      = 19;
	public static final int EVENT_MENU_SHOW    = 20;
	
	private TextView	tvBackMessage;
	private ImageView	ivBackImage;
	private View		vBackVideo;
	
	private class Navi
	{
		public View		vgNavi;

		public TextView	tvTitle;
		public Button	bTeletext;
			
		//	(Channel) Up/Down
		public Button	bUp;
		public Button	bDown;
		
		//	EPG
		public TextView	tvEPG_P;
		public TextView	tvEPG_F;
		public SeekBar	tvProgress;
		
		//	Strength / Quality
		public TextView	tvStrength_txt;
		public TextView	tvStrength_val;
		public TextView	tvQuality_txt;
		public TextView	tvQuality_val;
		
		// Media Button
		public ImageButton ibuRew;
		public ImageButton ibuPlayPause;
		public ImageButton ibuStop;
		public ImageButton ibuFfwd;
	}
	Navi cNavi = new Navi();
		
	// Title
	private class Title
	{
		public TextView	tvTitle;
	}
	Title	cTitle	= new Title();
	
	View mStateView;

	private class LocalPlay
	{
		public View vPlayInfo;

		public TextView tvCurrTime;
		public TextView tvEndTime;
		public TextView tvPlayMode;
		public SeekBar sbTimeBar;
	}
	LocalPlay cLocalPlay = new LocalPlay();

	private static final int[] SpeedList = {-32, -16, -8, -4, -2, 1, 2, 4, 8, 16, 32};

	private int eState_ScreenSize = DxbPlayer.SCREENMODE_FULL;

	private int mCurrentTime = 0;

	private int mIsSeekTo = 0;
	private int mIsTracking = 0;

	private int mLastSystemUiVis = 0;
	private final View mView;
	static DxbView_Full_Record mRecordView;
	private DxbView_Full_Information mInfoView;
	private DxbView_Full_Menu mMenuView;
	
	View		layout_password;
	AlertDialog	ad_password	= null;

	public DxbView_Full(Context context, View v)
	{
		super(context);
		mView = v;

		mRecordView = new DxbView_Full_Record(context, getContext().findViewById(R.id.full_group_rec));
		mInfoView = new DxbView_Full_Information(context, getContext().findViewById(R.id.layout_file_play));
		mMenuView = new DxbView_Full_Menu(context, getContext().findViewById(R.id.layout_menu));

		setComponent();
		setTextSize();
		setListener();
		setOnSystemUiVisibilityChangeListener();

/*		if(getPlayer().isSTB())
		{
			cNavi.bTeletext.setFocusable(false);
			
			cNavi.bUp.setVisibility(View.GONE);
			cNavi.bDown.setVisibility(View.GONE);
		}
		else*/
		{
			mView.findViewById(R.id.pvr_button_group).setVisibility(View.INVISIBLE);
		}
	}

	@Override
	public void setVisible(boolean v)
	{
		DxbLog(I, "setVisible(v=" + v + ")");
		
		if(v)
		{
			getPlayer().setSubtitleEnable(true);
			
			getContext().getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN | 0x80000000, WindowManager.LayoutParams.FLAG_FULLSCREEN | 0x80000000); 
			getContext().getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

			getContext().findViewById(R.id.main).setBackgroundColor(0xFF000000);
			onUpdateScreen();
			setTitleState(false);
			setVideoLayout();
			setGroupState(false);
			getMainHandler().removeCallbacks(mRunnable_hiddenSystemUi);
			getMainHandler().postDelayed(mRunnable_hiddenSystemUi, 2000);
			mStateView.setVisibility(View.VISIBLE);
		}
		else
		{
			getPlayer().playSubtitle(DxbPlayer._OFF_);
			mStateView.setVisibility(View.GONE);
			vBackVideo.setBackgroundColor(0x00000000);
		}
		
		mView.setVisibility(v ? View.VISIBLE : View.GONE);
	}

	private void setVideoLayout()
	{
		DxbLog(I, "setVideoLayout(eState_ScreenSize=" + eState_ScreenSize + ")");
		getPlayer().setScreenMode(eState_ScreenSize, 0, 0, 0, 0);
	}

	private void setOnSystemUiVisibilityChangeListener()
	{
		// When the user touches the screen or uses some hard key, the framework
		// will change system ui visibility from invisible to visible. We show
		// the media control and enable system UI (e.g. ActionBar) to be visible at this point
		mView.setOnSystemUiVisibilityChangeListener
		(
			new View.OnSystemUiVisibilityChangeListener()
			{
				@Override
				public void onSystemUiVisibilityChange(int visibility)
				{
					DxbLog(I, "setOnSystemUiVisibilityChangeListener() --> onSystemUiVisibilityChange(visibility=" + visibility + ")");
					int diff = mLastSystemUiVis ^ visibility;
					mLastSystemUiVis = visibility;

					if(		(diff & View.SYSTEM_UI_FLAG_HIDE_NAVIGATION) != 0
						&&	(visibility & View.SYSTEM_UI_FLAG_HIDE_NAVIGATION) == 0
					)
					{
						showSystemUi(true);

						if(		(getPlayer().getServiceType() == 2)
							&&	(getPlayer().getLocalPlayState() == LOCAL_STATE.STOP)
						)
						{
							getMainHandler().postDelayed(mRunnable_hiddenSystemUi, 2000);
							getPlayer().setFileList_cursor(getPlayer().getChannelPosition(2));
							setChannel();
						}
						else if(	(!mMenuView.isShown())
								&&	(dialog_Favorite != null)
								&&	(!dialog_Favorite.isShowing())
						)
						{
							if(		getPlayer().isParentalLock()
								&&	!getPlayer().isUnlockParental()
							)
							{
								updateUnlockPassword();
							}
							else
							{
								setGroupState(true);
							}
						}
						else if(!mMenuView.isShown())
						{
							setGroupState(true);
						}
					}
				}
			}
		);
	}

	private void showSystemUi(boolean visible)
	{
		DxbLog(I, "showSystemUi(visible=" + visible + ")");
		getMainHandler().removeCallbacks(mRunnable_hiddenSystemUi);
		
		int flag =		View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
					|	View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
					|	View.SYSTEM_UI_FLAG_LAYOUT_STABLE;
		
		if (!visible)
		{
			// We used the deprecated "STATUS_BAR_HIDDEN" for unbundling
			flag	|=	View.STATUS_BAR_HIDDEN
					|	View.SYSTEM_UI_FLAG_FULLSCREEN
					|	View.SYSTEM_UI_FLAG_HIDE_NAVIGATION;
		}

		RelativeLayout.LayoutParams lp = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.MATCH_PARENT);

		if(visible)
		{
			lp.setMargins(0, 0, 0, getPlayer().getStatusbarHeight());
		}
		else
		{
			lp.setMargins(0, 0, 0, 0);
		}
		
		mView.setLayoutParams(lp);
		mView.setSystemUiVisibility(flag);
	}

	private void showRecordUi(boolean visible)
	{
		DxbLog(I, "showRecordUi(visible=" + visible + ")");
		
		mRecordView.setVisible(visible);
	}

	@Override
	protected void FilePlayTimeUpdate(DxbPlayer player, int time)
	{
		//DxbLog(D, "onFilePlayTimeUpdate(t = " + time + ")");

		if (mIsSeekTo == 1 || mIsTracking == 1)
			return;

		if(		getPlayer().getRecordState() == REC_STATE.RECORDING
			&&	mRecordView.getRecordType() == TYPE.TIME_SHIFT 
			&&	getPlayer().getLocalPlayState() == LOCAL_STATE.PAUSE
			&&	time < 0
		)
		{
			cNavi.ibuPlayPause.setImageResource(R.drawable.dxb_ic_media_pause);
			cLocalPlay.vPlayInfo.setVisibility(View.GONE);
			getPlayer().setPlayPause(2, true);
			time = 0;
		}
		
		if (time < 0)
		{
			time = 0;
		}
		
		mCurrentTime = time*1000;
		cLocalPlay.tvCurrTime.setText(DxbUtil.stringForTime(mCurrentTime));
		cLocalPlay.tvEndTime.setText(DxbUtil.stringForTime(getPlayer().getDuration()));
		
		int sec = getPlayer().getDuration() / 1000;
		cLocalPlay.sbTimeBar.setMax(sec); // 24 ~ 40 px
		cLocalPlay.sbTimeBar.setKeyProgressIncrement(1);
		cLocalPlay.sbTimeBar.setProgress(time);

		if (getPlayer().getRecordState() == REC_STATE.RECORDING)
		{
			if (sec >= 46800) // 13 hours * 60 * 60
			{
				setEvent(EVENT_RECORD);
				sec = 46800;
			}
			
			mInfoView.UpdateInfoForRecording(time, sec, mRecordView.getFileName());
		}
		else
		{
			mInfoView.UpdateInfoForPlay(time, sec);
		}
	}

	@Override
	protected void VideoOutput(DxbPlayer player)
	{
		DxbLog(I, "VideoOutput()");
		if(!getPlayer().isVideoPlay())
		{
			vBackVideo.setBackgroundColor(0xFF101010);
		}
		else
		{
			if(		!getPlayer().isParentalLock()
				||	getPlayer().isUnlockParental()
			)
			{
				vBackVideo.setBackgroundColor(0x00000000);
			}
		}
	}

	private boolean isPVR_UpDown	= false;
	@Override
	protected void PlayingCompletion(DxbPlayer player, int type, int state)
	{
		DxbLog(D, "PlayingCompletion(type = " + type + ", state = " + state + ")");
		
		//	type : 0-normal, 1-speed, 2-seek
		if(type == 0)
		{
			mInfoView.setVisible(false);
			if (getPlayer().getServiceType() == 2)
			{
				synchronized (cLocalPlay)
				{
					mIsSeekTo = 0;
					mIsTracking = 0;
				}

				getPlayer().stop();
				
				if(!isPVR_UpDown)
				{
					if (mView.isShown())
					{
						setEvent(EVENT_CHANNEL);
					}
				}
			}
			
			if(isPVR_UpDown)
			{
				isPVR_UpDown	= false;
			}
			
			setPlaySpeed(5);
			cLocalPlay.vPlayInfo.setVisibility(View.GONE);
		}
		else if (type == 1)
		{
			setPlaySpeed(5);
			cLocalPlay.vPlayInfo.setVisibility(View.GONE);
		}
		else if (type == 2)
		{
			synchronized (cLocalPlay)
			{
				if (mIsTracking == 0)
				{
					getMainHandler().removeCallbacks(mRunnable_SeekTo);
					getMainHandler().postDelayed(mRunnable_SeekTo, 500);
				}
			}
		}
	}

	private void setComponent()
	{
		ivBackImage		= (ImageView)getContext().findViewById(R.id.full_image);
		tvBackMessage	= (TextView)getContext().findViewById(R.id.full_text);
		
		cNavi.vgNavi = mView.findViewById(R.id.full_view_navi);

		//	Title
		cNavi.tvTitle		= (TextView)mView.findViewById(R.id.full_title);		
		cNavi.bTeletext	= (Button)mView.findViewById(R.id.full_title_ttx);
		
		//	(Channel) Up/Down
		cNavi.bUp		= (Button)mView.findViewById(R.id.full_ch_up);
		cNavi.bDown		= (Button)mView.findViewById(R.id.full_ch_down);

		//	EPG
		cNavi.tvProgress	= (SeekBar)mView.findViewById(R.id.full_epg_progress);
		cNavi.tvEPG_P		= (TextView)mView.findViewById(R.id.full_epg_p);
		cNavi.tvEPG_F		= (TextView)mView.findViewById(R.id.full_epg_f);
		
		//	Strength / Quality
		cNavi.tvStrength_txt	= (TextView)mView.findViewById(R.id.full_strength_txt);
		cNavi.tvStrength_val	= (TextView)mView.findViewById(R.id.full_strength_val);
		cNavi.tvQuality_txt		= (TextView)mView.findViewById(R.id.full_quality_txt);
		cNavi.tvQuality_val		= (TextView)mView.findViewById(R.id.full_quality_val);

		//	File Play
		cLocalPlay.tvCurrTime = (TextView)mView.findViewById(R.id.time_current);
		cLocalPlay.tvEndTime = (TextView)mView.findViewById(R.id.time);
		cLocalPlay.sbTimeBar = (SeekBar)mView.findViewById(R.id.mediacontroller_progress);
		cLocalPlay.vPlayInfo = (View)getContext().findViewById(R.id.full_group_play_mode);
		cLocalPlay.tvPlayMode = (TextView)getContext().findViewById(R.id.full_play_mode_text);

		/*	Title Only	*/
		cTitle.tvTitle = (TextView)getContext().findViewById(R.id.full_title_only);

		// Media Button
		cNavi.ibuRew = (ImageButton)mView.findViewById(R.id.rew);
		cNavi.ibuPlayPause = (ImageButton)mView.findViewById(R.id.pause);
		cNavi.ibuStop = (ImageButton)mView.findViewById(R.id.stop);
		cNavi.ibuFfwd = (ImageButton)mView.findViewById(R.id.ffwd);
		
		mStateView = getContext().findViewById(R.id.full_display_state);
		vBackVideo	= getContext().findViewById(R.id.full_view);
	}

	private void setTextSize()
	{
			//	Title
		cNavi.tvTitle.setTextSize(getTextSize(18));
		cTitle.tvTitle.setTextSize(getTextSize(18));
			
			//	Preview
		tvBackMessage.setTextSize(getTextSize(25));
			
			//	File play
		cLocalPlay.tvCurrTime.setTextSize(getTextSize(18));
		cLocalPlay.tvEndTime.setTextSize(getTextSize(18));
		cLocalPlay.tvPlayMode.setTextSize(getTextSize(30));
		
		//	EPG
		cNavi.tvEPG_P.setTextSize(getTextSize(18));
		cNavi.tvEPG_F.setTextSize(getTextSize(18));
		
		//	Strength / Quality
		cNavi.tvStrength_txt.setTextSize(getTextSize(18));
		cNavi.tvStrength_val.setTextSize(getTextSize(18));
		cNavi.tvQuality_txt.setTextSize(getTextSize(18));
		cNavi.tvQuality_val.setTextSize(getTextSize(18));
	}
	
	private void setListener()
	{
		//	OnClickListener
		mView.setOnClickListener(this);
		ivBackImage.setOnClickListener(this);

		// Media Button
		cNavi.ibuRew.setOnClickListener(this);
		cNavi.ibuPlayPause.setOnClickListener(this);
		cNavi.ibuStop.setOnClickListener(this);
		cNavi.ibuFfwd.setOnClickListener(this);

		//	navi
		cNavi.bTeletext.setOnClickListener(this);
		cNavi.bUp.setOnClickListener(this);
		cNavi.bDown.setOnClickListener(this);

		//	OnFocusChange
		cNavi.bTeletext.setOnFocusChangeListener(this);
		cNavi.bUp.setOnFocusChangeListener(this);
		cNavi.bDown.setOnFocusChangeListener(this);

		//	OnTouchListener
		//	Title
		cNavi.bTeletext.setOnTouchListener(this);
			
		//	Navi
		cNavi.bUp.setOnTouchListener(this);
		cNavi.bDown.setOnTouchListener(this);

		//	OnSeekBarChangeListener
		cLocalPlay.sbTimeBar.setOnSeekBarChangeListener(this);
		
		mMenuView.setOnEventListener(this);
	}

	public void onFocusChange(View v, boolean hasFocus)
	{
		DxbLog(I, "onFocusChange()");

		int id = v.getId();
		switch(id)
		{
			case R.id.full_title_ttx:
				if(hasFocus == true)
				{
					cNavi.bTeletext.setBackgroundResource(R.drawable.dxb_portable_ttx_main_btn_f);
				}
				else
				{
					cNavi.bTeletext.setBackgroundResource(R.drawable.dxb_portable_ttx_main_btn_n);
				}
			break;
		
			case R.id.full_ch_up:
				if(hasFocus == true)
				{
					cNavi.bUp.setBackgroundResource(R.drawable.dxb_portable_channel_up_btn_f);
				}
				else
				{
					cNavi.bUp.setBackgroundResource(R.drawable.dxb_portable_channel_up_btn_n);
				}					
			break;
			
			case R.id.full_ch_down:
				if(hasFocus == true)
				{
					cNavi.bDown.setBackgroundResource(R.drawable.dxb_portable_channel_down_btn_f);
				}
				else
				{
					cNavi.bDown.setBackgroundResource(R.drawable.dxb_portable_channel_down_btn_n);
				}
			break;

			case R.id.mediacontroller_progress:
				if(hasFocus == true)
				{
					cLocalPlay.sbTimeBar.setBackgroundColor(getResources().getColor(R.color.color_3));
				}
				else
				{
					cLocalPlay.sbTimeBar.setBackgroundColor(getResources().getColor(R.color.trans_background));
				}
			break;
			
			default:
		}
	}

	private void setGroupState(boolean state)
	{
		DxbLog(I, "setGroupState(state=" + state + ")");
		
		if(state)
		{
			getPlayer().setSubtitleEnable(false);

			getMainHandler().removeCallbacks(mRunnable_Navigation);
			getMainHandler().removeCallbacks(mRunnable_hiddenSystemUi);
			setTitleState(false);
			
			if(!cNavi.vgNavi.isShown())
			{
				setMenuState(false);
				getMainHandler().removeCallbacks(mRunnable_EPG_PF_DB_Update);
				getPlayer().playSubtitle(DxbPlayer._OFF_);
				
				if (getPlayer().isValidate_Teletext())
				{
					cNavi.bTeletext.setVisibility(View.VISIBLE);
				}
				else
				{
					cNavi.bTeletext.setVisibility(View.INVISIBLE);
				}

				reflashUI();

				cNavi.vgNavi.setVisibility(View.VISIBLE);

//				if(!getPlayer().isSTB())
				{
					cNavi.bUp.requestFocus();
				}
				
				setIndicatorVisible(true);
				mInfoView.setVisible(false);
				//getMainHandler().postDelayed(mRunnable_EPG_PF_DB_Update, 1000);
				
				onUpdateEPG_PF();
			}

			showSystemUi(state);

			getMainHandler().postDelayed(mRunnable_Navigation, 7000);
		}
		else
		{
			getPlayer().setSubtitleEnable(true);

			getMainHandler().removeCallbacks(mRunnable_EPG_PF_DB_Update);
			getMainHandler().removeCallbacks(mRunnable_Navigation);

			cNavi.vgNavi.setVisibility(View.GONE);
			setIndicatorVisible(false);
			if (getPlayer().eState != DxbPlayer.STATE.SCAN)
			{
				getPlayer().playSubtitle(getPlayer().cOption.language_subtitle);
			}
		}
	}

	private void setMenuState(boolean state)
	{
		DxbLog(I, "setMenuState(state=" + state + ")");
		
		getMainHandler().removeCallbacks(mRunnable_hiddenMenu);
		getMainHandler().removeCallbacks(mRunnable_hiddenSystemUi);

		if(state)
		{
			showSystemUi(state);
			getMainHandler().postDelayed(mRunnable_hiddenMenu, 7000);
			
			if(!mMenuView.isShown())
			{
				getPlayer().playSubtitle(DxbPlayer._OFF_);
			}
		}
		else
		{
			if (getPlayer().eState != DxbPlayer.STATE.SCAN)
			{
				getPlayer().playSubtitle(getPlayer().cOption.language_subtitle);
			}

			getMainHandler().postDelayed(mRunnable_hiddenSystemUi, 2000);
		}

		mMenuView.setVisible(state);
	}

	private void reflashUI()
	{
		if (getPlayer().isFilePlay() || getPlayer().getRecordState() != REC_STATE.STOP)
		{
			mView.findViewById(R.id.file_play).setVisibility(View.VISIBLE);
			mView.findViewById(R.id.air_play).setVisibility(View.GONE);
			
			if(!getPlayer().isSTB())
			{
				cNavi.bUp.setVisibility(View.GONE);
				cNavi.bDown.setVisibility(View.GONE);
			}
		}
		else
		{
			mView.findViewById(R.id.file_play).setVisibility(View.GONE);
			mView.findViewById(R.id.air_play).setVisibility(View.VISIBLE);
			
			if(!getPlayer().isSTB())
			{
				cNavi.bUp.setVisibility(View.VISIBLE);
				cNavi.bDown.setVisibility(View.VISIBLE);
			}
		}
	}

	private Runnable mRunnable_SeekTo	= new Runnable()
	{
		public void run()
		{
			DxbLog(I, "mRunnable_SeekTo --> run()");
			synchronized (cLocalPlay)
			{
				if (mIsSeekTo == 1)
				{
					mIsSeekTo = 0;
					getPlayer().setSeekTo(-1);
				}
			}
		}
	};

	public void onStartTrackingTouch(SeekBar seekBar) {
		synchronized (cLocalPlay)
		{
			getMainHandler().removeCallbacks(mRunnable_SeekTo);
			mIsTracking = 1;
		}
	}

	public void onStopTrackingTouch(SeekBar seekBar) {
		synchronized (cLocalPlay)
		{
			getMainHandler().removeCallbacks(mRunnable_SeekTo);
			mIsTracking = 0;
			if (mIsSeekTo == 1)
			{
				getMainHandler().postDelayed(mRunnable_SeekTo, 500);
			}
		}
	}

	public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
		if (fromUser == true) {
			if (getPlayer().getLocalPlayState() == LOCAL_STATE.STOP) {
				if (getPlayer().getRecordState() == REC_STATE.STOP)
					return;
				if (getPlayer().setPlay() == false)
					updateToast(getResources().getString(R.string.recording_other_channel));
			}
			synchronized (cLocalPlay)
			{
				getMainHandler().removeCallbacks(mRunnable_SeekTo);
				mIsSeekTo = 1;
				getPlayer().setSeekTo(progress*1000);
			}
		}
	}

	@Override
	public boolean onGenericMotionEvent(MotionEvent event) {
		if ((event.getSource() & InputDevice.SOURCE_CLASS_POINTER) != 0) {
			switch (event.getAction()) {
			case MotionEvent.ACTION_SCROLL:
				final float vscroll;
				final float hscroll;
				if ((event.getMetaState() & KeyEvent.META_SHIFT_ON) != 0) {
					vscroll = 0;
					hscroll = event.getAxisValue(MotionEvent.AXIS_VSCROLL);
				} else {
					vscroll = -event.getAxisValue(MotionEvent.AXIS_VSCROLL);
					hscroll = event.getAxisValue(MotionEvent.AXIS_HSCROLL);
				}
				if (hscroll != 0 || vscroll != 0) {
					if (hscroll > 0 || vscroll > 0) {
						volumeDown();
					} else {
						volumeUp();
					}
					return true;
				}
			}
		}
		return false;
	}

	public boolean onTouch(View v, MotionEvent event)
	{
		DxbLog(I, "onTouch(v=" + v +", event=" + event + ")");
		
		if(event.getAction() == MotionEvent.ACTION_UP)
		{
			onClick(v);
		}
		else if (event.getAction() == MotionEvent.ACTION_DOWN)
		{
			v.requestFocus();
		}
		
		return true;
	}

	public void onClick(View v)
	{
		DxbLog(I, "ListenerOnClick - onClick(v.getId() = " + v.getId() + ")");
		
		int id = v.getId();
		if(id != R.id.full_title_ttx)
		{
			if (cNavi.vgNavi.isShown())
			{
				setGroupState(true);
			}
			else
			{
				if(		getPlayer().isParentalLock()
					&&	!getPlayer().isUnlockParental()
				)
				{
					updateUnlockPassword();
				}
				else
				{
					setGroupState(true);
				}

				return;
			}
		}
		
		if(		(id == R.id.full_image)
			&&	(getPlayer().getServiceType() == 2)
		)
		{
			getPlayer().setFileList_cursor(getPlayer().getChannelPosition(2));
			setChannel();
		}
		else
		{
			setEvent(getIndex_Menu(id));
		}
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		DxbLog(I, "onKeyDown(keyCode=" + keyCode + ", event=" + event + ")");

		if(press_InputChannel(keyCode))
		{
			return true;
		}

		if(keyCode == KeyEvent.KEYCODE_BOOKMARK)
		{
			showFavorite();
			return false;
		}

		if(keyCode == KeyEvent.KEYCODE_POUND)
		{
			getPlayer().setServiceType(2); // pvr list
			setEvent(EVENT_CHANNEL);
			return true;
		}

		if (mMenuView.isShown())
		{
			if(keyCode == KeyEvent.KEYCODE_DPAD_LEFT || keyCode == KeyEvent.KEYCODE_DPAD_RIGHT)
			{
				getMainHandler().removeCallbacks(mRunnable_hiddenMenu);
				getMainHandler().removeCallbacks(mRunnable_hiddenSystemUi);
				getMainHandler().postDelayed(mRunnable_hiddenMenu, 7000);
			}
			else if (mMenuView.onKeyDown(keyCode, event))
			{
				getMainHandler().removeCallbacks(mRunnable_hiddenSystemUi);
				getMainHandler().postDelayed(mRunnable_hiddenSystemUi, 2000);
				
				return true;
			}
		}
		else if(mInfoView.isShown())
		{
			if (mInfoView.onKeyDown(keyCode, event))
			{
				return true;
			}
		}
		else if (cNavi.vgNavi.isShown())
		{
			if (keyCode == KeyEvent.KEYCODE_BACK || keyCode == KeyEvent.KEYCODE_ESCAPE)
			{
				setGroupState(false);
				getMainHandler().removeCallbacks(mRunnable_hiddenSystemUi);
				getMainHandler().postDelayed(mRunnable_hiddenSystemUi, 2000);
				
				return true;
			}
			
			if(keyCode != KeyEvent.KEYCODE_MEDIA_STOP)
			{
				setGroupState(true);
			}
		}
		else
		{
			switch(keyCode)
			{
				case KeyEvent.KEYCODE_DPAD_UP:
					setEvent(EVENT_CHANNEL_UP);
					return true;

				case KeyEvent.KEYCODE_DPAD_DOWN:
					setEvent(EVENT_CHANNEL_DOWN);
					return true;

				case KeyEvent.KEYCODE_DPAD_LEFT:
					volumeDown();
					return true;
	
				case KeyEvent.KEYCODE_DPAD_RIGHT:
					volumeUp();
					return true;

				case KeyEvent.KEYCODE_BACK:
				case KeyEvent.KEYCODE_ESCAPE:
					getContext().finish_APK_message();
					/*if (getPlayer().getRecordState() != REC_STATE.STOP)
					{
						return true;
					}*/
					return false;
			}
		}
		
		if (setEvent(getIndex_Menu(keyCode)))
		{
			return true;
		}
		return false;
	}

	private int getIndex_Menu(int id)
	{
		DxbLog(I, "getIndex_Menu(id=" + id + ")");
		
		switch (id)
		{
			case R.id.full_menu1_layout:
			case KeyEvent.KEYCODE_1:
				return EVENT_CHANNEL;
	
			case R.id.full_menu2_layout:
			case KeyEvent.KEYCODE_2:
				return EVENT_SCREENMODE;
	
			case R.id.full_menu3_layout:
			case KeyEvent.KEYCODE_3:
			case KeyEvent.KEYCODE_GUIDE:
			case KeyEvent.KEYCODE_F9:
				return EVENT_EPG;
	
			case R.id.full_menu4_layout:
			case KeyEvent.KEYCODE_4:
				return EVENT_CAPTURE;
	
			case R.id.full_menu5_layout:
			case KeyEvent.KEYCODE_5:
	 		case KeyEvent.KEYCODE_MEDIA_RECORD:
				return EVENT_RECORD;
	
			case R.id.full_menu6_layout:
			case KeyEvent.KEYCODE_6:
	 		case KeyEvent.KEYCODE_SETTINGS:
				return EVENT_OPTION;
	
			case R.id.full_menu7_layout:
			case KeyEvent.KEYCODE_7:
	 		case KeyEvent.KEYCODE_INFO:
				return EVENT_INFORMATION;
	
	 		case R.id.full_title_ttx:
	 		case KeyEvent.KEYCODE_8:
	 		case KeyEvent.KEYCODE_WINDOW:
	 			return EVENT_TTX;
	
	 		case KeyEvent.KEYCODE_BOOKMARK:
				return EVENT_FAVORITE;
	
	 		case R.id.full_ch_up:
	 		case KeyEvent.KEYCODE_PAGE_UP:
	 		case KeyEvent.KEYCODE_MEDIA_PREVIOUS:
	 			return EVENT_CHANNEL_UP;
	
	 		case R.id.full_ch_down:
	 		case KeyEvent.KEYCODE_PAGE_DOWN:
	 		case KeyEvent.KEYCODE_MEDIA_NEXT:
	 			return EVENT_CHANNEL_DOWN;
	
	 		case R.id.ffwd:
	 		case KeyEvent.KEYCODE_MEDIA_FAST_FORWARD:
	 			return EVENT_FAST_FORWARD;
	
	 		case R.id.rew:
	 		case KeyEvent.KEYCODE_MEDIA_REWIND:
	 			return EVENT_REWIND;
	
	 		case R.id.pause:
	 		case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
	 			return EVENT_PLAY_PAUSE;
	
	 		case KeyEvent.KEYCODE_MEDIA_PLAY:
	 			return EVENT_PLAY;
	
	 		case KeyEvent.KEYCODE_MEDIA_PAUSE:
	 			return EVENT_PAUSE;
	
	 		case R.id.stop:
	 		case KeyEvent.KEYCODE_MEDIA_STOP:
	 			return EVENT_STOP;
	
	 		case KeyEvent.KEYCODE_BUTTON_1:
	 			return EVENT_AUDIO_CHANGE;
	
	 		case KeyEvent.KEYCODE_CAPTIONS:
	 			return EVENT_SUBT_CHANGE;
	
			case R.id.layout_full:
			case KeyEvent.KEYCODE_ENTER:
	 			return EVENT_UI_SHOW;
	 			
	 		case KeyEvent.KEYCODE_MENU:
	 			return EVENT_MENU_SHOW;
	
			default:
				return EVENT_NULL;
		}
	}

	private boolean setEvent(int event)
	{
		DxbLog(I, "setEvent(event=" + event + ")");
		switch (event)
		{
			case EVENT_CHANNEL:
				setState(false, VIEW_LIST);
			break;
			
			case EVENT_SCREENMODE:
				changeScreenMode();
			break;
			
			case EVENT_EPG:
				if(getPlayer().getServiceType() < 2)
				{
					setState(false, VIEW_EPG);
				}
			break;
			
			case EVENT_CAPTURE:
				mRecordView.setCapture();
			break;
			
			case EVENT_RECORD:
				if(mRecordView.getRecordType() == TYPE.TIME_SHIFT && getPlayer().getRecordState() == REC_STATE.RECORDING)
				{
					setRecordStartStop();
				}
				else
				{
					showRecordingTime();
				}
			break;
			
			case EVENT_OPTION:
				if (getPlayer().getRecordState() == REC_STATE.STOP)
				{
					setState(false, VIEW_OPTION);
				}
			break;
			
			case EVENT_INFORMATION:
				showProgramInfo();
				//showInformation();
			break;
			
			case EVENT_TTX:
				if (getPlayer().isValidate_Teletext())
				{
					setIndicatorVisible(false);
					setState(false, VIEW_TELETEXT);
				}
			break;
			
			case EVENT_CHANNEL_UP:
			{
				isPVR_UpDown	= true;
				setChannelUp();
				setTitleState(true);
			}
			break;
			
			case EVENT_CHANNEL_DOWN:
			{
				isPVR_UpDown	= true;
				setChannelDown();
				setTitleState(true);
			}
			break;
			
			case EVENT_FAST_FORWARD:
				setFastForward();
			break;
			
			case EVENT_REWIND:
				setRewind();
			break;
			
			case EVENT_PLAY_PAUSE:
				setLocalPlayStartPause();
			break;
			
			case EVENT_PLAY:
				setLocalPlayStart();
			break;
			
			case EVENT_PAUSE:
				setLocalPlayPause();
			break;
			
			case EVENT_STOP:
				setLocalPlayStop();
			break;
			
			case EVENT_AUDIO_CHANGE:
				setChangeAudio();
			break;
			
			case EVENT_SUBT_CHANGE:
				setChangeSubtitle();
			break;
			
			case EVENT_UI_SHOW:
				setMenuState(false);
				if(cNavi.vgNavi.isShown())
				{
					getMainHandler().removeCallbacks(mRunnable_hiddenSystemUi);
					getMainHandler().postDelayed(mRunnable_hiddenSystemUi, 2000);
				}

				setGroupState(!cNavi.vgNavi.isShown());
			break;
			
			case EVENT_MENU_SHOW:
				setGroupState(false);
				if(mMenuView.isShown())
				{
					getMainHandler().removeCallbacks(mRunnable_hiddenSystemUi);
					getMainHandler().postDelayed(mRunnable_hiddenSystemUi, 2000);
				}
				
				setMenuState(!mMenuView.isShown());
			break;
			
			default:
				return false;
		}
		return true;
	}

	private void changeScreenMode()
	{
		DxbLog(I, "changeScreenMode()");
		
		if(eState_ScreenSize == DxbPlayer.SCREENMODE_LETTERBOX)
		{
			eState_ScreenSize	= DxbPlayer.SCREENMODE_PANSCAN;
		}
		else if(eState_ScreenSize == DxbPlayer.SCREENMODE_PANSCAN)
		{
			eState_ScreenSize	= DxbPlayer.SCREENMODE_FULL;
		}
		else if(eState_ScreenSize == DxbPlayer.SCREENMODE_FULL)
		{
			eState_ScreenSize	= DxbPlayer.SCREENMODE_LETTERBOX;
		}
		
		setVideoLayout();
	}

	public void setRecordStartStop()
	{
		if(mRecordView.setRecord(TYPE.MOVIE))
		{
			if(mMenuView.isShown())
			{
				setMenuState(false);
			}
		}
		else if(cNavi.vgNavi.isShown())
		{
			setGroupState(false);
		}
	}

	//record time check thread
	final Time rec_Time = new Time();

	long cur_Time_t;
	long rec_Time_t;

	int rec_Time_Comp;
	int fix_Time = 0;
	int remind_Time = 0;
	int remind_Time_t = 180;
	int cal_remind_Time = 0;
	int set_remind_Time = 0;
	int recordUi_Time = 0;

	//remind record time calc
	public void rec_Time()
	{
		cur_Time_t = System.currentTimeMillis();
		rec_Time_Comp = (int)cur_Time_t + set_remind_Time;
		remind_Time_t = set_remind_Time;
	}

	public boolean isPlay = true;
	private Handler recordTime_handler = new Handler();

	private Runnable RecordTimeThread = new Runnable() {
		public void run() {

				cur_Time_t = System.currentTimeMillis();
				cal_remind_Time = (int)(rec_Time_Comp - cur_Time_t - fix_Time);
				remind_Time = remind_Time + 1000;
				recordUi_Time++;

				DxbLog(I, "remind_time : " + cal_remind_Time/1000);

				if(recordUi_Time == 15)
					showRecordUi(false);

				if(getPlayer().getRecordState() == REC_STATE.STOP)
				{
					recordTime_handler.removeCallbacks(RecordTimeThread);
				}

				if(cal_remind_Time <= 0)
				{
					isPlay = false;
					setRecordStartStop();
					recordTime_handler.removeCallbacks(RecordTimeThread);
				}

				if(cal_remind_Time > 0 && getPlayer().getRecordState() == REC_STATE.RECORDING)
					recordTime_handler.postDelayed(this, 1000);
		}
	};

	//record time set dialog
	private void showRecordingTime()
	{
		DxbLog(I, "showRecordingTime()");
		remind_Time = 0;

		View dialog = View.inflate(getContext(), R.layout.dxb_view_record_time, null);
		final AlertDialog dialog_recording_time = new AlertDialog.Builder(getContext()).setView(dialog).create();

		dialog_recording_time.setTitle(R.string.recording_time);
		dialog_recording_time.setMessage(getResources().getString(R.string.recording_time_message));
		dialog_recording_time.setButton(DialogInterface.BUTTON_POSITIVE, getResources().getString(R.string.record), new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				if(getPlayer().getRecordState() == REC_STATE.RECORDING)
				{
					fix_Time = remind_Time;
					remind_Time = 0;
					rec_Time();
					showRecordUi(true);
					recordUi_Time = 0;
				}
				else if(getPlayer().getRecordState() == REC_STATE.STOP)
				{
					cal_remind_Time = set_remind_Time;
					rec_Time();
					isPlay = true;
					recordTime_handler.removeCallbacks(RecordTimeThread);
					recordTime_handler.postDelayed(RecordTimeThread,  0);
					setRecordStartStop();
					dialog_recording_time.dismiss();
					showRecordUi(true);
				}
			}
		});

		if(getPlayer().getRecordState() == REC_STATE.RECORDING) {
			dialog_recording_time.setButton(DialogInterface.BUTTON_NEUTRAL, getResources().getString(R.string.rec_stop), new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which) {
					setRecordStartStop();
					recordTime_handler.removeCallbacks(RecordTimeThread);
					isPlay = false;
					showRecordUi(false);
				}
			});
		}

		dialog_recording_time.setButton(DialogInterface.BUTTON_NEGATIVE, getResources().getString(R.string.cancel), new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				remind_Time = 0;
			}
		});

		dialog_recording_time.setOnCancelListener(new DialogInterface.OnCancelListener() {
			@Override
			public void onCancel(DialogInterface dialog) {
				remind_Time = 0;
			}
		});

		dialog_recording_time.show();

		final TextView tx_time = (TextView)dialog.findViewById(R.id.record_time_text);

		if(getPlayer().getRecordState() == REC_STATE.STOP)
			set_remind_Time = 10800000;
		else if(getPlayer().getRecordState() == REC_STATE.RECORDING)
			set_remind_Time = cal_remind_Time;

		tx_time.setText("" + set_remind_Time/60000 + "min");

		Button bt_minus = (Button)dialog.findViewById(R.id.bt_record_time_minus);
	    bt_minus.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				if(set_remind_Time > 600000) {
					set_remind_Time = set_remind_Time - 600000;
						tx_time.setText("" + set_remind_Time/60000 + "min");
				}
				else
					tx_time.setText("" + set_remind_Time/60000 + "min");
			}
	    });

		Button bt_plus = (Button)dialog.findViewById(R.id.bt_record_time_plus);
	    bt_plus.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				if(set_remind_Time < 10800000)
				{
					set_remind_Time = set_remind_Time + 60000;
					tx_time.setText("" + set_remind_Time/60000 + "min");
				}
				else
					tx_time.setText("" + set_remind_Time/60000 + "min");
			}
		});
	}

	public void startRecord(int iID_Alarm, int iType_repeat)
	{
		if (mRecordView.setRecord(TYPE.ALARM, iID_Alarm, iType_repeat))
		{
			setMenuState(false);
		}
	}

	private void showInformation() {
		mInfoView.setVisible(true);
		setMenuState(false);
	}

	AlertDialog	dialog_Favorite	= null;
	private void showFavorite()
	{
		DxbLog(I, "showFavorite()");
		
		String	sName_bookmark	= null;
		List<String> listArrays = new ArrayList<String>();
		int	iCount_service	= getPlayer().getChannelCount();
		for(int index=0 ; index<iCount_service ; index++)
		{
			sName_bookmark	= getPlayer().isBookmark(index);
			if(sName_bookmark != null)
			{
				listArrays.add(sName_bookmark);
			}
		}
		
		String[] favouriteArrays = listArrays.toArray(new String[listArrays.size()]);
		listArrays.clear();

		dialog_Favorite	= new AlertDialog.Builder(getContext())
			.setTitle(R.string.FAVOURITE)
			.setItems(favouriteArrays, new DialogInterface.OnClickListener()
				{
					public void onClick(DialogInterface dialog, int which)
					{
						DxbLog(I, "onClick(which = " + which + ")");
						getPlayer().setChannel_Bookmark(which);

						dialog_Favorite.dismiss();
					}
				})
			.setOnCancelListener(new DialogInterface.OnCancelListener()
				{
					public void onCancel(DialogInterface dialog)
					{
						dialog_Favorite.dismiss();
					}
				})
			.create();
		
		dialog_Favorite.show();
		
	}

	private void setFastForward() {
		setPlaySpeed_mode(getPlaySpeed() + 1);
	}

	private void setRewind() {
		setPlaySpeed_mode(getPlaySpeed() - 1);
	}

	private void setPlaySpeed_mode(int iSpeed) {
		if (iSpeed < 0 || 10 < iSpeed) {
			iSpeed = NORMAL_SPEED;
		}
		if (iSpeed == getPlaySpeed()) {
			return;
		}
		if (getPlayer().getLocalPlayState() == LOCAL_STATE.STOP) {
			if (getPlayer().getRecordState() == REC_STATE.STOP)
				return;
			if (iSpeed < NORMAL_SPEED) {
				if (getPlayer().setPlay() == false)
					updateToast(getResources().getString(R.string.recording_other_channel));
			}
		}
		if (getPlayer().getLocalPlayState() == LOCAL_STATE.STOP || getPlayer().setPlaySpeed(SpeedList[iSpeed]) != 0) {
			setPlaySpeed(NORMAL_SPEED);
		} else {
			setPlaySpeed(iSpeed);
		}
		if (getPlaySpeed() == NORMAL_SPEED) {
			cLocalPlay.vPlayInfo.setVisibility(View.GONE);
		} else {
			cLocalPlay.vPlayInfo.setVisibility(View.VISIBLE);
			cLocalPlay.tvPlayMode.setText(String.valueOf(SpeedList[getPlaySpeed()]) + "X");
		}
	}

	private void setLocalPlayStartPause() {
		if(getPlayer().getLocalPlayState() == LOCAL_STATE.PAUSE) {
			setLocalPlayStart();
		} else {
			setLocalPlayPause();
		}
	}

	private void setLocalPlayStart() {
		if (getPlaySpeed() != NORMAL_SPEED) {
			setPlaySpeed_mode(NORMAL_SPEED);
		} else if(getPlayer().getLocalPlayState() == LOCAL_STATE.PAUSE) {
			cNavi.ibuPlayPause.setImageResource(R.drawable.dxb_ic_media_pause);
			cLocalPlay.vPlayInfo.setVisibility(View.GONE);
			getPlayer().setPlayPause(2, true);
		} else if(getPlayer().getLocalPlayState() == LOCAL_STATE.PLAYING) {
			int TimeDiff = getPlayer().getDuration() - mCurrentTime;
			if(TimeDiff > 5000)
				getPlayer().setSeekTo(getPlayer().getDuration());
		}
	}

	private void setLocalPlayPause()
	{
		if (getPlaySpeed() != NORMAL_SPEED)
		{
			setPlaySpeed_mode(NORMAL_SPEED);
		}
		
		if(getPlayer().getLocalPlayState() == LOCAL_STATE.PAUSE)
		{
			return;
		}
		else if(getPlayer().getLocalPlayState() == LOCAL_STATE.PLAYING)
		{
			cNavi.ibuPlayPause.setImageResource(R.drawable.dxb_ic_media_play);
			cLocalPlay.vPlayInfo.setVisibility(View.VISIBLE);
			cLocalPlay.tvPlayMode.setText("PAUSE");
			getPlayer().setPlayPause(2, false);
		}
		else if(getPlayer().getRecordState() == REC_STATE.STOP)
		{
			//cNavi.menu[MENU_RECORD].vMenu.requestFocus();
			mRecordView.setRecord(TYPE.TIME_SHIFT);
			getPlayer().setPlay();
			//setGroupState(false);
			cNavi.ibuPlayPause.setImageResource(R.drawable.dxb_ic_media_play);
			cLocalPlay.vPlayInfo.setVisibility(View.VISIBLE);
			cLocalPlay.tvPlayMode.setText("PAUSE");
			getPlayer().setPlayPause(2, false);
			reflashUI();
		}
		else if(getPlayer().getLocalPlayState() == LOCAL_STATE.STOP)
		{
			getPlayer().setPlay();
			cNavi.ibuPlayPause.setImageResource(R.drawable.dxb_ic_media_play);
			cLocalPlay.vPlayInfo.setVisibility(View.VISIBLE);
			cLocalPlay.tvPlayMode.setText("PAUSE");
			getPlayer().setPlayPause(2,  false);
		}
		else if (getPlayer().setPlay() == false)
		{
			updateToast(getResources().getString(R.string.recording_other_channel));
		}
	}

	private void setLocalPlayStop()
	{
		if (getPlaySpeed() != NORMAL_SPEED)
		{
			setPlaySpeed_mode(NORMAL_SPEED);
		}
		else
		{
			cNavi.ibuPlayPause.setImageResource(R.drawable.dxb_ic_media_pause);
			cLocalPlay.vPlayInfo.setVisibility(View.GONE);
			if(getPlayer().getLocalPlayState() != LOCAL_STATE.STOP)
			{
				getPlayer().setLocalPlayStop();
			}
			else
			{
				if (getPlayer().getRecordState() == REC_STATE.RECORDING)
				{
					setRecordStartStop();
				}
			}
			reflashUI();
		}
	}

	private AlertDialog.Builder audioChangeBuilder;
	private View dxb_view_audio_channel;
	private void setChangeAudio()
	{
		audioChangeBuilder = new AlertDialog.Builder(getContext());
		LayoutInflater layoutInflater = LayoutInflater.from(getContext());
		dxb_view_audio_channel = layoutInflater.inflate(R.layout.dxb_view_audio_channel, null);

		final TextView channel = (TextView)dxb_view_audio_channel.findViewById(R.id.audio_channel);
		final TextView stereo = (TextView)dxb_view_audio_channel.findViewById(R.id.audio_mode);
		final ImageView channel_left = (ImageView)dxb_view_audio_channel.findViewById(R.id.bt_channel_left);
		final ImageView channel_right = (ImageView)dxb_view_audio_channel.findViewById(R.id.bt_channel_right);
		final ImageView stereo_mode_left = (ImageView)dxb_view_audio_channel.findViewById(R.id.bt_mode_left);
		final ImageView stereo_mode_right = (ImageView)dxb_view_audio_channel.findViewById(R.id.bt_mode_right);

		//**** focus change ****//
		channel_left.setNextFocusRightId(R.id.bt_channel_right);
		channel_right.setNextFocusLeftId(R.id.bt_channel_left);

		String langCode = getPlayer().changeAudio(0);
		String stereo_mode = getPlayer().changeStereoMode(0);

		if (langCode != null)
			channel.setText(DxbUtil.getLanguageText(langCode));
		
		if(stereo_mode != null)
			stereo.setText(stereo_mode);

		audioChangeBuilder.setTitle(R.string.audio_channel);
		audioChangeBuilder.setView(dxb_view_audio_channel);

		channel_left.setOnClickListener(new ImageView.OnClickListener() {
			@Override
			public void onClick(View arg0) {
				String langCode = getPlayer().changeAudio(-1);
				if (langCode != null)
					channel.setText(DxbUtil.getLanguageText(langCode));
			}
		});

		channel_right.setOnClickListener(new ImageView.OnClickListener() {
			@Override
			public void onClick(View arg0) {
				String langCode = getPlayer().changeAudio(1);
				if (langCode != null)
					channel.setText(DxbUtil.getLanguageText(langCode));
			}
		});

		channel_left.setOnFocusChangeListener(new ImageView.OnFocusChangeListener() {
			@Override
			public void onFocusChange(View v, boolean hasFocus) {
				channel.setTextColor(getResources().getColor(R.color.ics_focus));
				stereo.setTextColor(getResources().getColor(R.color.color_white));
			}
		});

		channel_right.setOnFocusChangeListener(new ImageView.OnFocusChangeListener() {
			@Override
			public void onFocusChange(View v, boolean hasFocus) {
				channel.setTextColor(getResources().getColor(R.color.ics_focus));
				stereo.setTextColor(getResources().getColor(R.color.color_white));
			}
		});

		stereo_mode_left.setOnClickListener(new ImageView.OnClickListener() {
			@Override
			public void onClick(View arg0) {
				String stereo_mode_change = getPlayer().changeStereoMode(-1);
				if (stereo_mode_change != null)
					stereo.setText(stereo_mode_change);
			}
		});

		stereo_mode_right.setOnClickListener(new ImageView.OnClickListener() {
			@Override
			public void onClick(View arg0) {
				String stereo_mode_change = getPlayer().changeStereoMode(1);
				if (stereo_mode_change != null)
					stereo.setText(stereo_mode_change);
			}
		});

		stereo_mode_left.setOnFocusChangeListener(new ImageView.OnFocusChangeListener() {
			@Override
			public void onFocusChange(View v, boolean hasFocus) {
				stereo.setTextColor(getResources().getColor(R.color.ics_focus));
				channel.setTextColor(getResources().getColor(R.color.color_white));
			}
		});

		stereo_mode_right.setOnFocusChangeListener(new ImageView.OnFocusChangeListener() {
			@Override
			public void onFocusChange(View v, boolean hasFocus) {
				stereo.setTextColor(getResources().getColor(R.color.ics_focus));
				channel.setTextColor(getResources().getColor(R.color.color_white));
			}
		});

		audioChangeBuilder.setOnKeyListener(new DialogInterface.OnKeyListener() {
			@Override
			public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
				switch(keyCode)
				{
					case KeyEvent.KEYCODE_INFO :
						if(event.getAction() == MotionEvent.ACTION_DOWN)
							dialog.dismiss();
					break;
				}
				return false;
			}
		});
		
		AlertDialog dialog = audioChangeBuilder.create();
		
		dialog.show();
		
		DisplayMetrics displayMetrics = new DisplayMetrics();
		((WindowManager)getContext().getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay().getMetrics(displayMetrics);
		
		WindowManager m = dialog.getWindow().getWindowManager();
		Display d = m.getDefaultDisplay();
		WindowManager.LayoutParams lp = dialog.getWindow().getAttributes();
		lp.dimAmount = 0.0f;
		lp.alpha = 0.7f;
		lp.width = (int) (d.getWidth() * 0.5);
		dialog.getWindow().setAttributes(lp);
		dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
	}

	private AlertDialog.Builder subTitleChangeBuilder;

	private void setChangeSubtitle() {
		subTitleChangeBuilder = new AlertDialog.Builder(getContext());
		subTitleChangeBuilder.setTitle(R.string.subtitle);

		CharSequence[] Entry = getPlayer().getEntrySubtitle();

		int pos = getPlayer().cOption.language_subtitle;

		subTitleChangeBuilder.setSingleChoiceItems(Entry, pos, new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				getPlayer().changeSubtitle(which);
				dialog.dismiss();
			}
		});

		subTitleChangeBuilder.setOnKeyListener(new DialogInterface.OnKeyListener() {
			@Override
			public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
				switch(keyCode)
				{
					case KeyEvent.KEYCODE_INFO :
						if(event.getAction() == MotionEvent.ACTION_DOWN)
							dialog.dismiss();
					break;
				}
				return false;
			}
		});

		AlertDialog dialog = subTitleChangeBuilder.create();

		dialog.show();

		DisplayMetrics displayMetrics = new DisplayMetrics();
		((WindowManager)getContext().getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay().getMetrics(displayMetrics);

		WindowManager m = dialog.getWindow().getWindowManager();
		Display d = m.getDefaultDisplay();
		WindowManager.LayoutParams lp = dialog.getWindow().getAttributes();
		lp.dimAmount = 0.0f;
		lp.alpha = 0.7f;
		lp.width = (int) (d.getWidth() * 0.5);
		dialog.getWindow().setAttributes(lp);
		dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
	}
	
	private void updateUnlockPassword()
	{
		if(ad_password != null)
		{
			if(ad_password.isShowing())
			{
				return;
			}
		}
		
		LayoutInflater	inflater_password	= (LayoutInflater)getContext().getSystemService(getContext().LAYOUT_INFLATER_SERVICE);
		layout_password	= inflater_password.inflate(R.layout.dxb_dialog_password, (ViewGroup)getContext().findViewById(R.id.etPassword));
		
		AlertDialog.Builder	aDialog_password	= new AlertDialog.Builder(getContext());
		aDialog_password.setTitle(R.string.please_enter_password);
		aDialog_password.setView(layout_password);
		
		aDialog_password.setPositiveButton(R.string.enter, new DialogInterface.OnClickListener()
		{
			public void onClick(DialogInterface dialog, int which)
			{
				EditText	etPassword	= (EditText)layout_password.findViewById(R.id.etPassword);
				String		sPassword	= etPassword.getText().toString();
				if(sPassword.equals(getPlayer().cOption.password))
				{
					getPlayer().setPlayPause(getPlayer().getServiceType(), true);
					getPlayer().setUnlockParental(true);
					onUpdateScreen();
					ad_password.dismiss();
				}
				else
				{
					Builder mBuilder	= new AlertDialog.Builder(getPlayer().getContext());
					mBuilder.setTitle(getResources().getString(R.string.change_password));
					mBuilder.setMessage(getResources().getString(R.string.please_enter_the_correct_password));

					mBuilder.setPositiveButton(	getResources().getString(R.string.ok), null);
					
					mBuilder.show();
				}
			}
		});
		aDialog_password.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener()
		{
			public void onClick(DialogInterface dialog, int which)
			{
				ad_password.dismiss();
			}
		});
		aDialog_password.setOnKeyListener(new DialogInterface.OnKeyListener()
		{
			public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event)
			{
				if(		(keyCode == KeyEvent.KEYCODE_DPAD_DOWN)
					||	(keyCode == KeyEvent.KEYCODE_ENTER)
				)
				{
					EditText	editText	= (EditText)layout_password.findViewById(R.id.etPassword);
					InputMethodManager	imm	= (InputMethodManager)getContext().getSystemService(getContext().INPUT_METHOD_SERVICE);
					imm.hideSoftInputFromWindow(editText.getWindowToken(), 0);
				}
				else if(keyCode == KeyEvent.KEYCODE_DPAD_UP)
				{
					EditText	editText	= (EditText)layout_password.findViewById(R.id.etPassword);
					InputMethodManager	imm	= (InputMethodManager)getContext().getSystemService(getContext().INPUT_METHOD_SERVICE);
					imm.showSoftInput(editText, InputMethodManager.SHOW_FORCED);
				}
				
				return false;
			}
		});
		
		ad_password	= aDialog_password.create();
		ad_password.show();
	}

    private Runnable mRunnable_EPG_PF_DB_Update = new Runnable() {
        public void run() {
            new Thread(new Runnable() {
                public void run() {
                    getPlayer().setEPG_PF();
                    getMainHandler().post(mRunnable_EPG_PF_UI_Update);
                }
            }).start();
        }
    };

    private Runnable mRunnable_EPG_PF_UI_Update = new Runnable()
    {
        public void run()
        {
			DxbLog(I, "mRunnable_EPG_PF_UI_Update()");
			cNavi.tvEPG_P.setText("");
			cNavi.tvEPG_F.setText("");
			getPlayer().setCurrentRating(0);
			
			if (getPlayer().isCurrEPG())
			{
				int iStart = getPlayer().getCurrEPG_StartTime();
				int iDuration = getPlayer().getCurrEPG_DurationTime();
				
				int	iRating	= getPlayer().getCurrentEPG_Rating();
				if(iRating == 0)
				{
					cNavi.tvEPG_P.setText(DxbUtil.stringForTime(iStart, iDuration) + "	 (All) " + getPlayer().getCurrEPG_Title());
				}
				else
				{
					cNavi.tvEPG_P.setText(DxbUtil.stringForTime(iStart, iDuration) + "	 (" + (iRating+3) + ") " + getPlayer().getCurrEPG_Title());
				}
				getPlayer().setCurrentRating(iRating);
				
				cNavi.tvProgress.setMax(iDuration);
				cNavi.tvProgress.setKeyProgressIncrement(1);
				cNavi.tvProgress.setProgress(getPlayer().getCurrentTime() / 60 - iStart);
			}
			
			if (getPlayer().isNextEPG())
			{
				int iStart = getPlayer().getNextEPG_StartTime();
				int iDuration = getPlayer().getNextEPG_DurationTime();
				
				int	iRating	= getPlayer().getNextEPG_Rating();
				if(iRating == 0)
				{
					cNavi.tvEPG_F.setText(DxbUtil.stringForTime(iStart, iDuration) + "	 (All) " + getPlayer().getNextEPG_Title());
				}
				else
				{
					cNavi.tvEPG_F.setText(DxbUtil.stringForTime(iStart, iDuration) + "	 (" + (iRating+3) + ") " + getPlayer().getNextEPG_Title());
				}
			}
			
			onUpdateScreen();
        }
    };
    
	private Runnable mRunnable_Navigation = new Runnable()
	{
		public void run()
		{
			//Log.i(TAG, "mRunnable_Navigation --> run()");
			if (mView.isShown())
			{
				setGroupState(false);
				getMainHandler().removeCallbacks(mRunnable_hiddenSystemUi);
				getMainHandler().postDelayed(mRunnable_hiddenSystemUi, 2000);
			}
		}
	};

	private Runnable mRunnable_PlayMode = new Runnable() {
		public void run() {
			//Log.i(TAG, "mRunnable_PlayMode --> run()");
			cLocalPlay.vPlayInfo.setVisibility(View.GONE);
		}
	};
	
	private Runnable mRunnable_hiddenSystemUi	= new Runnable()
	{
		public void run()
		{
			showSystemUi(false);
		}
	};

	private Runnable mRunnable_hiddenMenu	= new Runnable()
	{
		public void run()
		{
			DxbLog(I, "mRunnable_hiddenMenu --> run()");
			mMenuView.setVisible(false);
			if (getPlayer().eState != DxbPlayer.STATE.SCAN)
			{
				getPlayer().playSubtitle(getPlayer().cOption.language_subtitle);
			}
			
			getMainHandler().removeCallbacks(mRunnable_hiddenSystemUi);
			getMainHandler().postDelayed(mRunnable_hiddenSystemUi, 2000);
		}
	};

	public void setTitleState(boolean state)
	{
		DxbLog(I, "setTitleState(" + state + ")");
		
		getMainHandler().removeCallbacks(mRunnable_TitleOnly);
		if(cNavi.vgNavi.isShown())
		{
			cTitle.tvTitle.setVisibility(View.INVISIBLE);
		}
		else if(state)
		{
			getMainHandler().postDelayed(mRunnable_TitleOnly, 5000);
			cTitle.tvTitle.setVisibility(View.VISIBLE);
		}
		else
		{
			cTitle.tvTitle.setVisibility(View.INVISIBLE);
		}
	}
	
	private Runnable mRunnable_TitleOnly = new Runnable()
	{
		public void run()
		{
			DxbLog(I, "mRunnable_TitleOnly --> run()");
			setTitleState(false);
		}
	};

	@Override
	public void onChannelChange()
	{
		DxbLog(I, "onChannelChange()");
		if(!getPlayer().isUnlockParental())
		{
			mView.findViewById(R.id.pvr_controller_group).setVisibility(View.INVISIBLE);
		}
		
		onUpdateEPG_PF();
		onUpdateScreen();
		
		if(getPlayer().isValidate_Teletext())
		{
			cNavi.bTeletext.setVisibility(View.VISIBLE);
		}
		else
		{
			cNavi.bTeletext.setVisibility(View.INVISIBLE);
		}
	}

	@Override
	protected void onUpdateScreen()
	{
		DxbLog(I, "onUpdateScreen()");

		if(		(getPlayer().eState == DxbPlayer.STATE.OPTION_FULL_SCAN)
			||	(getPlayer().eState == DxbPlayer.STATE.OPTION_MANUAL_SCAN)
			||	(getPlayer().eState == DxbPlayer.STATE.OPTION_BLIND_SCAN)
			||	(getPlayer().eState == DxbPlayer.STATE.OPTION_MAKE_PRESET)
		)
		{
			ivBackImage.setVisibility(View.GONE);
			vBackVideo.setBackgroundColor(0xFF101010);
			tvBackMessage.setVisibility(View.GONE);

			return;
		}

		int	image = 0, text = 0;

		if(		getPlayer().isPlaying()
			||	(getContext().intentOptionActivity != null)	// error : option --> back --> not_display(no_channel)
		)
		{
			if(getPlayer().getChannelCount() <= 0)
			{
				cTitle.tvTitle.setText(R.string.app_name);
				cNavi.tvTitle.setText(R.string.app_name);
				image	= R.drawable.dxb_portable_channel_no_channel_img;
				
				if(getPlayer().getServiceType() == 2)
				{
					text	= R.string.no_pvr_file_found;
				}
				else
				{
					text	= R.string.if_you_want_to_use_radio_scan_channel;
				}

				// show scan dialog
				if(exe_dialog)
				{
					scan_dialog();
					confirm_Scan();
				}
			}
			else
			{
				cTitle.tvTitle.setText(getPlayer().getServiceName_addLogical());
				cNavi.tvTitle.setText(getPlayer().getServiceName_addLogical() + " ( " + (float)getPlayer().getCurFrequency() / 1000 + "MHz )");
				
				if(getPlayer().getServiceType() == 2)
				{
					if(getPlayer().getLocalPlayState() == LOCAL_STATE.STOP)
					{
						image = R.drawable.ic_control_play;
					}
				}
				//	else(getPlayer().getServiceType() <= 2){}	--> check!!!
				else if (getSignalLevel() < 0)
				{
					image	= R.drawable.dxb_portable_channel_weak_signal_img;
					text	= R.string.receiving_signal_strength_is_weak;
				}
				else if(	getPlayer().isParentalLock()
						&&	!getPlayer().isUnlockParental()
				)
				{
					image	= R.drawable.dxb_portable_lock_img;
					text	= R.string.the_service_is_locked_the_rating_is;
					
					if(getPlayer().getRecordState() != REC_STATE.STOP)
					{
						setRecordStartStop();
					}
					
					getPlayer().setPlayPause(getPlayer().getServiceType(), false);
				}
				else if(getPlayer().isScrambled())
				{
					image	= R.drawable.dxb_portable_channel_no_play_img;
					text	= R.string.scrambled;
				}
				else if (getPlayer().isRadio())
				{
					image	= R.drawable.dxb_portable_channel_radio_img;
				}
			}
			
			if (image == 0)
			{
				ivBackImage.setVisibility(View.GONE);

				if(getPlayer().isVideoPlay())
				{
					vBackVideo.setBackgroundColor(0x00000000);
				}
				else
				{
					vBackVideo.setBackgroundColor(0xFF101010);
				}
			}
			else
			{
				ivBackImage.setBackgroundResource(image);
				ivBackImage.setVisibility(View.VISIBLE);

				vBackVideo.setBackgroundColor(0xFF101010);
			}
			
			if(text == 0)
			{
				tvBackMessage.setVisibility(View.GONE);
			}
			else
			{
				mInfoView.setVisible(false);
				
				if(text == R.string.the_service_is_locked_the_rating_is)
				{
					tvBackMessage.setText(getResources().getString(text) + " " + (getPlayer().cOption.age+3));
				}
				else
				{
					tvBackMessage.setText(getResources().getString(text));
				}

				tvBackMessage.setVisibility(View.VISIBLE);
			}
		}
	}

	@Override
	protected void onUpdateSignalStrength(int iStrength)
	{
		//DxbLog(I, "onUpdateSignalStrength(" + iStrength + ")");
		
		String	sStrength	= iStrength + "%";
		cNavi.tvStrength_val.setText(sStrength);
	}

	@Override
	protected void onUpdateSignalQuality(int iQuality)
	{
		//DxbLog(I, "onUpdateSignalQuality(" + iQuality + ")");
		
		String	sQuality	= iQuality + "%";
		cNavi.tvQuality_val.setText(sQuality);
	}
	
	@Override
	protected void onUpdateEPG_PF()
	{
		DxbLog(I, "onUpdateEPG_PF()");
		
		if(		(getPlayer().eState == DxbPlayer.STATE.OPTION_FULL_SCAN)
			||	(getPlayer().eState == DxbPlayer.STATE.OPTION_MANUAL_SCAN)
			||	(getPlayer().eState == DxbPlayer.STATE.OPTION_BLIND_SCAN)
			||	(getPlayer().eState == DxbPlayer.STATE.OPTION_MAKE_PRESET)
		)
		{
			return;
		}
		
		if(getPlayer().getServiceType() == 2)
		{
			mView.findViewById(R.id.pvr_controller_group).setVisibility(View.VISIBLE);
			return;
		}
		
		cNavi.tvEPG_P.setText("00:00~00:00   No EPG information");
		cNavi.tvEPG_F.setText("00:00~00:00   No EPG information");
		getPlayer().setCurrentRating(0);
		
		if(getPlayer().isValid())
		{
			getPlayer().setEPG_PF();
	        
			if (getPlayer().isCurrEPG())
			{
				int iStart = getPlayer().getCurrEPG_StartTime();
				int iDuration = getPlayer().getCurrEPG_DurationTime();
				
				int	iRating	= getPlayer().getCurrentEPG_Rating();
				if(iRating == 0)
				{
					cNavi.tvEPG_P.setText(DxbUtil.stringForTime(iStart, iDuration) + "   (All) " + getPlayer().getCurrEPG_Title());
				}
				else
				{
					cNavi.tvEPG_P.setText(DxbUtil.stringForTime(iStart, iDuration) + "   (" + (iRating+3) + ") " + getPlayer().getCurrEPG_Title());
				}
				getPlayer().setCurrentRating(iRating);
				
				cNavi.tvProgress.setMax(iDuration);
				cNavi.tvProgress.setKeyProgressIncrement(1);
				cNavi.tvProgress.setProgress(getPlayer().getCurrentTime() / 60 - iStart);
				
				mView.findViewById(R.id.pvr_controller_group).setVisibility(View.VISIBLE);
			}
			else if(getPlayer().getRecordState() == REC_STATE.RECORDING)
			{
				mView.findViewById(R.id.pvr_controller_group).setVisibility(View.VISIBLE);
			}
			else
			{
				mView.findViewById(R.id.pvr_controller_group).setVisibility(View.VISIBLE);
			}
			
			if (getPlayer().isNextEPG())
			{
				int iStart = getPlayer().getNextEPG_StartTime();
				int iDuration = getPlayer().getNextEPG_DurationTime();
				
				int	iRating	= getPlayer().getNextEPG_Rating();
				if(iRating == 0)
				{
					cNavi.tvEPG_F.setText(DxbUtil.stringForTime(iStart, iDuration) + "   (All) " + getPlayer().getNextEPG_Title());
				}
				else
				{
					cNavi.tvEPG_F.setText(DxbUtil.stringForTime(iStart, iDuration) + "   (" + (iRating+3) + ") " + getPlayer().getNextEPG_Title());
				}
			}
			
			onUpdateScreen();
		}
	}

	@Override
	protected Dialog onCreateDialog(int id)
	{
		DxbLog(I, "onCreateDialog(id=" + id + ")");
		
		if (id == DIALOG_FAVOURITE_LIST)
		{
		}
		
		return null;
	}
	
	@Override
	protected String getClassName() {
		return "[[[DxbView_Full]]]";
	}

	public void onEvent(int event)
	{
		DxbLog(I, "onEvent(event=" + event + ")");
		
		if (event != EVENT_SCREENMODE)
		{
			setMenuState(false);
			getMainHandler().removeCallbacks(mRunnable_hiddenSystemUi);
			getMainHandler().postDelayed(mRunnable_hiddenSystemUi, 2000);
		}
		
		setEvent(event);
	}
	
	public void confirm_Scan()
	{
		AlertDialog	confirmScanDialog = new AlertDialog.Builder(getContext())

		.setMessage(getResources().getString(R.string.no_scan_data))
		.setPositiveButton(getResources().getString(R.string.ok), new DialogInterface.OnClickListener() {

			@Override
			public void onClick(DialogInterface dialog, int which) {
				showScanDialog();
			}
		})

		.setNegativeButton(getResources().getString(R.string.no), null)
		.create();

		confirmScanDialog.show();
	}

	// Confirm popup scan_dialog when DVB APP first start
	boolean exe_dialog=true;

	public void scan_dialog()
	{
		exe_dialog = !exe_dialog;
	}

	//scan dialog when DVB APP first start
	public void showScanDialog()
	{
		DxbLog(I, "showScanDialog()");

		final Builder ScanDialog = new AlertDialog.Builder(getContext());

		if(getPlayer().isDVB_S2())
		{
			ScanDialog.setTitle(getResources().getString(R.string.scan));
			ScanDialog.setItems(R.array.no_channel_search_scan_dvbs, new DialogInterface.OnClickListener()
			{
				@Override
				public void onClick(DialogInterface dialog, int which) {
					switch(which)
					{
						//Dish setup
						case 0 :
							getContext().dishSetupDialogFlag = true;
							Intent	intent_dish_setup =  new Intent(getContext(), DishSetupActivity.class);
							getContext().startActivity(intent_dish_setup);
					}
				}
			});
		}
		else
		{
			CharSequence[] summaries = getResources().getTextArray(R.array.countrycode_select_summaries);

			ScanDialog.setTitle(getResources().getString(R.string.scan) + "  -  " + summaries[getPlayer().cOption.countrycode]);
			ScanDialog.setItems(R.array.no_channel_search_scan, new DialogInterface.OnClickListener()
			{
			@Override
				public void onClick(DialogInterface dialog, int which)
				{
				switch(which)
				{
					//Country Code
					case 0 :
						country_Code();
						break;

					//Auto Search
					case 1 :
						autoScan();
						break;

					//Manual Search
					case 2 :
						manualScan();
						break;
				}
			}
			});
		}
		
		ScanDialog.setNeutralButton(getResources().getString(R.string.cancel), new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				dialog.dismiss();
			}
		}).create();

		ScanDialog.show();
	}

	public void showScanDialog_dvbs()
	{
		DxbLog(I, "showScanDialog_dvbs()");
		
		final AlertDialog ScanDialog = new AlertDialog.Builder(getContext())
		
		.create();
		
		ScanDialog.show();
	
		
	}

	public void country_Code()
	{
		DxbLog(I, "Country_code_dialog");

		final AlertDialog country_code_dialog = new AlertDialog.Builder(getContext())

		.setTitle(R.string.countrycode)
		.setSingleChoiceItems(R.array.countrycode_select_summaries, getPlayer().cOption.countrycode, new DialogInterface.OnClickListener() {

			@Override
			public void onClick(DialogInterface dialog, int which) {
				getPlayer().cOption.countrycode = which;
				dialog.dismiss();
				showScanDialog();
			}
		})
		.setNeutralButton(R.string.cancel, new DialogInterface.OnClickListener() {

			@Override
			public void onClick(DialogInterface dialog, int which) {
				showScanDialog();
			}
		})
		.setOnCancelListener(new DialogInterface.OnCancelListener() {

			@Override
			public void onCancel(DialogInterface dialog) {
				showScanDialog();
			}
		})
		.create();

		country_code_dialog.show();
	}

	public void autoScan()
	{
		DxbLog(I, "Auto_Search");
		getPlayer().search();
		getContext().showDialog(DIALOG_SCAN_DEFAULT);
	}

	public void manualScan()
	{
		DxbLog(I, "Manual Search");
		getContext().showDialog(DIALOG_SCAN_MANUAL);
	}

	@Override
	protected void send_Event(MSG_VIEW msg)
	{
		DxbLog(I, "send_Event(msg.evetn=" + msg.event + ")");
		
		if(msg.event == EVENT.SCAN_PERCENT)
		{
//			String	return_Count = "  TV : " + getPlayer().getChannel_Count(0) + ", Radio : " + getPlayer().getChannel_Count(1);

/*			mDialog_Scan.setMessage(		"Please wait while searching...\n[ "
														+	msg.iVal + "% ]"
														+	return_Count
												);
	*/		
		}
	}
	private void showSystemUiInfo(boolean visible)
	{
		int flag =		View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
					|	View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
					|	View.SYSTEM_UI_FLAG_LAYOUT_STABLE;
		
		if (!visible)
		{
			// We used the deprecated "STATUS_BAR_HIDDEN" for unbundling
			flag	|=	View.STATUS_BAR_HIDDEN
					|	View.SYSTEM_UI_FLAG_FULLSCREEN
					|	View.SYSTEM_UI_FLAG_HIDE_NAVIGATION;
		}

		RelativeLayout.LayoutParams lp = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.MATCH_PARENT);

		if(visible)
		{
			lp.setMargins(0, 0, 0, getPlayer().getStatusbarHeight());
		}
		else
		{
			//lp.setMargins(0, 0, 0, 0);
		}
		
		mView.setLayoutParams(lp);
		mView.setSystemUiVisibility(flag);
	}
	
	private AlertDialog.Builder programInfoBuilder;
	private View dxb_view_program_info;
	
	public void showProgramInfo()
	{
		getPlayer().setProgramInfo();
		
		programInfoBuilder = new AlertDialog.Builder(getContext());
		LayoutInflater layoutInflater = LayoutInflater.from(getContext());
		dxb_view_program_info = layoutInflater.inflate(R.layout.dxb_view_program_info, null);

		programInfoBuilder.setTitle(R.string.program_info);

		final TextView program_no = (TextView) dxb_view_program_info.findViewById(R.id.program_no);
		final TextView program_name = (TextView) dxb_view_program_info.findViewById(R.id.program_name);
		final TextView channel_no = (TextView) dxb_view_program_info.findViewById(R.id.channel_num);
		final TextView video_pid = (TextView) dxb_view_program_info.findViewById(R.id.video_pid);
		final TextView frequency = (TextView) dxb_view_program_info.findViewById(R.id.frequency);
		final TextView audio_pid = (TextView) dxb_view_program_info.findViewById(R.id.audio_pid);
		final TextView bandwidth = (TextView) dxb_view_program_info.findViewById(R.id.bandwidth);
		final TextView rotation = (TextView) dxb_view_program_info.findViewById(R.id.rotation);
		final TextView constellation = (TextView) dxb_view_program_info.findViewById(R.id.constellation);
		final TextView fec_length = (TextView) dxb_view_program_info.findViewById(R.id.fec_length);
		final TextView guard_interval = (TextView) dxb_view_program_info.findViewById(R.id.guard_interval);
		final TextView code_rate = (TextView) dxb_view_program_info.findViewById(R.id.code_rate);
		final TextView fft = (TextView) dxb_view_program_info.findViewById(R.id.fft);
		final TextView multiple_plp = (TextView) dxb_view_program_info.findViewById(R.id.multiple_plp);
		final TextView pilot_pp = (TextView) dxb_view_program_info.findViewById(R.id.pilot_pp);
		final TextView single_plp = (TextView) dxb_view_program_info.findViewById(R.id.single_plp);

		program_no.setText(Integer.toString(getPlayer().getCurProgramID()));
		program_name.setText(getPlayer().getServiceName());
		channel_no.setText("CH-" + Integer.toString(getPlayer().getColumnIndex_LogicalChannel()));
		video_pid.setText(Integer.toString(getPlayer().getCurVideoPid()));
		frequency.setText(Integer.toString(getPlayer().getCurFrequency()));
		audio_pid.setText(Integer.toString(getPlayer().getCurAudioPid()));
		bandwidth.setText(Integer.toString(getPlayer().getCurBandwidth() / 1000) + "MHz");
		
		if(getPlayer().getCurDVBSystem() == 0)
		{
			//		if(getPlayer().getCurRotation() == -1)
				rotation.setText("N/A");
		
			if(getPlayer().getCurConstellation() == 0)
				constellation.setText("QPSK");
			else if(getPlayer().getCurConstellation() == 1)
				constellation.setText("16QAM");
			else if(getPlayer().getCurConstellation() == 2)
				constellation.setText("64QAM");
			else
				constellation.setText("N/A");
		
		//	if(getPlayer().getCurFecLength() == -1)
			fec_length.setText("N/A");
		
			if(getPlayer().getCurGuardInterval() == 0)
				guard_interval.setText("1/32");
			else if(getPlayer().getCurGuardInterval() == 1)
				guard_interval.setText("1/16");
			else
				guard_interval.setText("N/A");
		
			if(getPlayer().getCurCodeRate() == 0)
				code_rate.setText("1/2");
			else if(getPlayer().getCurCodeRate() == 1)
				code_rate.setText("2/3");
			else if(getPlayer().getCurCodeRate() == 2)
				code_rate.setText("3/4");
			else if(getPlayer().getCurCodeRate() == 3)
				code_rate.setText("5/6");
			else
				code_rate.setText("N/A");
		
			if(getPlayer().getCurFft() == 0)
				fft.setText("2K");
			else if(getPlayer().getCurFft() == 1)
				fft.setText("8K");
			else if(getPlayer().getCurFft() == 2)
				fft.setText("4K");
			else
				fft.setText("N/A");
		
			single_plp.setText("N/A");
			multiple_plp.setText("N/A");
		
			pilot_pp.setText("N/A");
		}
		else if(getPlayer().getCurDVBSystem() == 1)
		{
			if(getPlayer().getCurRotation() == 1)
				rotation.setText("YES");
			else if(getPlayer().getCurRotation() == 0)
				rotation.setText("NO");
			else
				rotation.setText("N/A");
				
			if(getPlayer().getCurConstellation() == 0)
				constellation.setText("QPSK");
			else if(getPlayer().getCurConstellation() == 1)
				constellation.setText("16QAM");
			else if(getPlayer().getCurConstellation() == 2)
				constellation.setText("64QAM");
			else if(getPlayer().getCurConstellation() == 3)
				constellation.setText("256QAM");
			else
				constellation.setText("N/A");
			
			if(getPlayer().getCurFecLength() == 0)
				fec_length.setText("16K LDPC");
			else if(getPlayer().getCurFecLength() == 1)
				fec_length.setText("64K LDPC");
			else
				fec_length.setText("N/A");
			
			if(getPlayer().getCurGuardInterval() == 0)
				guard_interval.setText("1/32");
			else if(getPlayer().getCurGuardInterval() == 1)
				guard_interval.setText("1/16");
			else if(getPlayer().getCurGuardInterval() == 2)
				guard_interval.setText("1/8");
			else if(getPlayer().getCurGuardInterval() == 3)
				guard_interval.setText("1/4");
			else if(getPlayer().getCurGuardInterval() == 4)
				guard_interval.setText("1/128");
			else if(getPlayer().getCurGuardInterval() == 1)
				guard_interval.setText("19/128");
			else if(getPlayer().getCurGuardInterval() == 1)
				guard_interval.setText("19/256");
			else				
				guard_interval.setText("N/A");
			
			if(getPlayer().getCurCodeRate() == 0)
				code_rate.setText("1/2");
			else if(getPlayer().getCurCodeRate() == 1)
				code_rate.setText("3/5");
			else if(getPlayer().getCurCodeRate() == 2)
				code_rate.setText("2/3");
			else if(getPlayer().getCurCodeRate() == 3)
				code_rate.setText("3/4");
			else if(getPlayer().getCurCodeRate() == 4)
				code_rate.setText("4/5");
			else if(getPlayer().getCurCodeRate() == 5)
				code_rate.setText("5/6");
			else
				code_rate.setText("N/A");
			
			if(getPlayer().getCurFft() == 0)
				fft.setText("1K");
			else if(getPlayer().getCurFft() == 1)
				fft.setText("2K");
			else if(getPlayer().getCurFft() == 2)
				fft.setText("4K");
			else if(getPlayer().getCurFft() == 3)
				fft.setText("8K");
			else if(getPlayer().getCurFft() == 4)
				fft.setText("16K");
			else if(getPlayer().getCurFft() == 5)
				fft.setText("32K");
			else
				fft.setText("N/A");
			
			if(getPlayer().getCurPlp() == -1)
			{
				single_plp.setText("YES");
				multiple_plp.setText("NO");
			}
			else
			{
				single_plp.setText("NO");
				multiple_plp.setText("Yes");
			}
			
			if(getPlayer().getCurPilotPp() == 0)
				pilot_pp.setText("PP1");
			else if(getPlayer().getCurPilotPp() == 1)
				pilot_pp.setText("PP2");
			else if(getPlayer().getCurPilotPp() == 2)
				pilot_pp.setText("PP3");
			else if(getPlayer().getCurPilotPp() == 3)
				pilot_pp.setText("PP4");
			else if(getPlayer().getCurPilotPp() == 4)
				pilot_pp.setText("PP5");
			else if(getPlayer().getCurPilotPp() == 5)
				pilot_pp.setText("PP6");
			else if(getPlayer().getCurPilotPp() == 6)
				pilot_pp.setText("PP7");
			else if(getPlayer().getCurPilotPp() == 7)
				pilot_pp.setText("PP8");
			else
				pilot_pp.setText("N/A");
		}

		programInfoBuilder.setView(dxb_view_program_info);
		programInfoBuilder.setOnKeyListener(new DialogInterface.OnKeyListener() {
			@Override
			public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
				switch(keyCode)
				{
					case KeyEvent.KEYCODE_INFO :
						if(event.getAction() == MotionEvent.ACTION_DOWN)
							dialog.dismiss();
					break;
				}
				return false;
			}
		});

		AlertDialog dialog = programInfoBuilder.create();

		dialog.show();

		DisplayMetrics displayMetrics = new DisplayMetrics();
		((WindowManager)getContext().getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay().getMetrics(displayMetrics);
		
		//WindowManager m = getWindowManager();
		//Display d = m.getDefaultDisplay();
		WindowManager.LayoutParams lp = dialog.getWindow().getAttributes();
		lp.dimAmount = 0.0f;
		lp.alpha = 0.9f;
		dialog.getWindow().setAttributes(lp);
		dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
	}
}
