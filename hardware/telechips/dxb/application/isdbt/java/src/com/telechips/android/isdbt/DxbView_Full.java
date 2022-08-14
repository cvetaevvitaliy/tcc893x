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
package com.telechips.android.isdbt;

import java.nio.IntBuffer;
import java.util.ArrayList;
import java.util.List;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.content.*;
import android.database.Cursor;
import android.graphics.*;
import android.util.Log;
import android.view.*;
import android.view.inputmethod.InputMethodManager;
import android.widget.*;

import com.telechips.android.isdbt.DxbView_Full_Record.TYPE;
import com.telechips.android.isdbt.player.*;
import com.telechips.android.isdbt.player.DxbPlayer.LOCAL_STATE;
import com.telechips.android.isdbt.player.isdbt.*;
import com.telechips.android.isdbt.util.DxbUtil;

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
	public static final int EVENT_SCAN = 21;
	public static final int EVENT_AUTOSEARCH_DN = 22;
	public static final int EVENT_AUTOSEARCH_UP = 23;
	
	private TextView	tvBackMessage;
	private ImageView	ivBackImage;
	private View		vBackVideo;
	private ProgressBar mProgress;
	
	private class Navi
	{
		public View		vgNavi;

		public TextView tvTitleVideo;
		public TextView tvTitleAudio;
		public TextView tvTitleLang;
		public TextView tvTitleInfo;
		public TextView	tvTitle;
		public Button	bTeletext;
		public ImageView ivLogo;
		public ImageView ivSubtitle;
			
		//	(Channel) Up/Down
		public Button	bUp;
		public Button	bDown;
			
		//	Unlock parental
		//public Button	bUnlock;
		
		public TextView	tvEPG_P;
		public TextView	tvEPG_F;
		public SeekBar	tvProgress;
		
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

	private int mLastSystemUiVis = 0;
	private final View mView;
	private DxbView_Full_Record mRecordView;
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

		cNavi.bTeletext.setFocusable(false);

/*		if(isSTB())
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
		
		if(getPlayer().getSCDisplay() == true)
		{
			getPlayer().setSCDisplay(false);
		}
		
		if(v)
		{
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

						if(		(getPlayer().getServiceType() == 1)
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
		cNavi.bUp.setBackgroundResource(R.drawable.dxb_portable_channel_up_btn_n);
		cNavi.bDown.setBackgroundResource(R.drawable.dxb_portable_channel_down_btn_n);
	}

	@Override
	protected void FilePlayTimeUpdate(DxbPlayer player, int time)
	{
		//DxbLog(D, "onFilePlayTimeUpdate(t = " + time + ")");
		
		if(		getPlayer().getRecordState() == DxbPlayer.REC_STATE.RECORDING
			&&	mRecordView.getRecordType() == TYPE.TIME_SHIFT 
			&&	getPlayer().getLocalPlayState() == DxbPlayer.LOCAL_STATE.PAUSE
			&&	time < 0
		)
		{
			cNavi.ibuPlayPause.setImageResource(R.drawable.dxb_ic_media_pause);
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

		if (getPlayer().getRecordState() == DxbPlayer.REC_STATE.RECORDING)
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
			vBackVideo.setVisibility(View.VISIBLE);
			vBackVideo.invalidate();
		}
		else
		{
			if(		!getPlayer().isParentalLock()
				||	getPlayer().isUnlockParental()
			)
			{
				if(getPlayer().getSCDisplay() == false)
				{	
					mProgress.setVisibility(View.GONE);
					vBackVideo.setBackgroundColor(0x00000000);
					vBackVideo.invalidate();
					getPlayer().playSubtitle(DxbPlayer._ON_);
				}
			}
		}
	}

	@Override
	protected void PlayingCompletion(DxbPlayer player, int type, int state)
	{
		DxbLog(D, "PlayingCompletion(type = " + type + ", state = " + state + ")");
		
		if(type == 0)
		{
			mInfoView.setVisible(false);
			if (getPlayer().getServiceType() == 1)
			{
				getPlayer().stop();
				if (mView.isShown())
				{
					setEvent(EVENT_CHANNEL);
				}
			}
			
			setPlaySpeed(5);
			cLocalPlay.vPlayInfo.setVisibility(View.GONE);
		}
		else if (type == 1)
		{
			setPlaySpeed(5);
			cLocalPlay.vPlayInfo.setVisibility(View.GONE);
		}
	}

	@Override
	protected void HandoverChannel(DxbPlayer player, int affiliation, int channel)
	{
		DxbLog(I, "HandoverChannel()");

		String msg = getResources().getString(R.string.receiving_signal_strength_is_weak) + "\n";
		
		if(affiliation == 0)
		{
			msg += getResources().getString(R.string.handover_relay) + " : " + (channel+13) + " CH";
		}
		else
		{
			msg += getResources().getString(R.string.handover_affiliate) + " : " + (channel+13) + " CH";
		}

		tvBackMessage.setText(msg);
	}
	
	@Override
	protected void SCErrorUpdate(DxbPlayer player, int state)
	{
		DxbLog(I, "SCErrorUpdate()");
		
		if(state != SCInfoData.SC_ERR_OK)
		{
			getMainHandler().removeCallbacks(mRunnable_SCDisplay);
			getMainHandler().postDelayed(mRunnable_SCDisplay, 5000);
			getPlayer().bcas_message_display = true;
			getPlayer().playSubtitle(0);
		}
		else
		{
			getPlayer().bcas_message_display = false;
			if(getPlayer().bcas_message_keep== true)
				showSystemUi(false);
		}
		
		onUpdateScreen();
	}
	
	private void setComponent()
	{
		Typeface typeface = Typeface.createFromFile("/system/fonts/DroidSansFallback_DxB.ttf");

		ivBackImage		= (ImageView)getContext().findViewById(R.id.full_image);
		tvBackMessage	= (TextView)getContext().findViewById(R.id.full_text);

		tvBackMessage.setTypeface(typeface);

		//      Loading image
		mProgress = (ProgressBar)getContext().findViewById(R.id.progress_bar);
		
		cNavi.vgNavi = mView.findViewById(R.id.full_view_navi);

		//	Title
		cNavi.tvTitleVideo	= (TextView)mView.findViewById(R.id.full_title_video);
		cNavi.tvTitleAudio	= (TextView)mView.findViewById(R.id.full_title_audio);
		cNavi.tvTitleLang	= (TextView)mView.findViewById(R.id.full_title_lang);
		cNavi.tvTitleInfo	= (TextView)mView.findViewById(R.id.full_title_info);
		cNavi.tvTitle		= (TextView)mView.findViewById(R.id.full_title);
		cNavi.bTeletext		= (Button)mView.findViewById(R.id.full_title_ttx);
		cNavi.ivLogo		= (ImageView)mView.findViewById(R.id.full_title_logo);
		cNavi.ivSubtitle	= (ImageView)mView.findViewById(R.id.full_title_subtitle);
		
		cNavi.tvTitleVideo.setTypeface(typeface);
		cNavi.tvTitleAudio.setTypeface(typeface);
		cNavi.tvTitleLang.setTypeface(typeface);
		cNavi.tvTitleInfo.setTypeface(typeface);
		cNavi.tvTitle.setTypeface(typeface);

		//	(Channel) Up/Down
		cNavi.bUp		= (Button)mView.findViewById(R.id.full_ch_up);
		cNavi.bDown		= (Button)mView.findViewById(R.id.full_ch_down);

		//	Unlock parental
		//cNavi.bUnlock	= (Button)mView.findViewById(R.id.full_unlock);

		//	EPG
		cNavi.tvProgress	= (SeekBar)mView.findViewById(R.id.full_epg_progress);
		cNavi.tvEPG_P		= (TextView)mView.findViewById(R.id.full_epg_p);
		cNavi.tvEPG_F		= (TextView)mView.findViewById(R.id.full_epg_f);

		cNavi.tvEPG_P.setTypeface(typeface);
		cNavi.tvEPG_F.setTypeface(typeface);

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
		int displayWidth = getDisplayWidth();

		if(displayWidth > 800)
		{
			if(displayWidth > 1280)
			{
				cNavi.tvTitleVideo.setTextSize(30 / getDisplayDensity());
				cNavi.tvTitleAudio.setTextSize(30 / getDisplayDensity());
				cNavi.tvTitleLang.setTextSize(28 / getDisplayDensity());
				cNavi.tvTitleInfo.setTextSize(30 / getDisplayDensity());
				cNavi.tvTitle.setTextSize(40 / getDisplayDensity());
			}
			else if(displayWidth > 1024)
			{
				cNavi.tvTitleVideo.setTextSize(20 / getDisplayDensity());
				cNavi.tvTitleAudio.setTextSize(20 / getDisplayDensity());
				cNavi.tvTitleLang.setTextSize(28 / getDisplayDensity());
				cNavi.tvTitleInfo.setTextSize(25 / getDisplayDensity());
				cNavi.tvTitle.setTextSize(25 / getDisplayDensity());
			}
			else
			{
				cNavi.tvTitleVideo.setTextSize(18 / getDisplayDensity());
				cNavi.tvTitleAudio.setTextSize(18 / getDisplayDensity());
				cNavi.tvTitleLang.setTextSize(16 / getDisplayDensity());
				cNavi.tvTitleInfo.setTextSize(20 / getDisplayDensity());
				cNavi.tvTitle.setTextSize(24 / getDisplayDensity());
			}
			
			cTitle.tvTitle.setTextSize(30 / getDisplayDensity());
			
			//	Preview
			tvBackMessage.setTextSize(32 / getDisplayDensity());
			
			//	EPG
			cNavi.tvEPG_P.setTextSize(25 / getDisplayDensity());
			cNavi.tvEPG_F.setTextSize(25 / getDisplayDensity());
			
			//	File Play
			cLocalPlay.tvCurrTime.setTextSize(25 / getDisplayDensity());
			cLocalPlay.tvEndTime.setTextSize(25 / getDisplayDensity());
			cLocalPlay.tvPlayMode.setTextSize(45 / getDisplayDensity());
		}
		else //	if(DxbPlayer_Control.eResolution == DxbPlayer_Control.DISPLAY_RESOLUTION.p480)
		{
			//	Title
			cNavi.tvTitleVideo.setTextSize(14/ getDisplayDensity());
			cNavi.tvTitleAudio.setTextSize(14 / getDisplayDensity());
			cNavi.tvTitleLang.setTextSize(12 / getDisplayDensity());
			cNavi.tvTitleInfo.setTextSize(16 / getDisplayDensity());
			cNavi.tvTitle.setTextSize(18 / getDisplayDensity());
			cTitle.tvTitle.setTextSize(18 / getDisplayDensity());
			
			//	Preview
			tvBackMessage.setTextSize(25 / getDisplayDensity());
			
			//	EPG
			cNavi.tvEPG_P.setTextSize(18 / getDisplayDensity());
			cNavi.tvEPG_F.setTextSize(18 / getDisplayDensity());
			
			//	File play
			cLocalPlay.tvCurrTime.setTextSize(18 / getDisplayDensity());
			cLocalPlay.tvEndTime.setTextSize(18 / getDisplayDensity());
			cLocalPlay.tvPlayMode.setTextSize(30 / getDisplayDensity());
		}		
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
//		cNavi.bUnlock.setOnClickListener(this);

		//	navi
		cNavi.bTeletext.setOnClickListener(this);
		cNavi.bUp.setOnClickListener(this);
		cNavi.bDown.setOnClickListener(this);

		//	OnFocusChange
		cNavi.bTeletext.setOnFocusChangeListener(this);
		cNavi.bUp.setOnFocusChangeListener(this);
		cNavi.bDown.setOnFocusChangeListener(this);
//		cNavi.bUnlock.setOnFocusChangeListener(this);

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
//					cNavi.bUp.setBackgroundResource(R.drawable.dxb_portable_channel_up_btn_f);
				}
				else
				{
//					cNavi.bUp.setBackgroundResource(R.drawable.dxb_portable_channel_up_btn_n);
				}					
			break;
			
			case R.id.full_ch_down:
				if(hasFocus == true)
				{
//					cNavi.bDown.setBackgroundResource(R.drawable.dxb_portable_channel_down_btn_f);
				}
				else
				{
//					cNavi.bDown.setBackgroundResource(R.drawable.dxb_portable_channel_down_btn_n);
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
			getMainHandler().removeCallbacks(mRunnable_Navigation);
			getMainHandler().removeCallbacks(mRunnable_hiddenSystemUi);
			setTitleState(false);
			
			if(!cNavi.vgNavi.isShown())
			{
				setMenuState(false);
				getMainHandler().removeCallbacks(mRunnable_EPG_PF_DB_Update);
				
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

//				if(!isSTB())
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
			getMainHandler().removeCallbacks(mRunnable_EPG_PF_DB_Update);
			getMainHandler().removeCallbacks(mRunnable_Navigation);

			cNavi.vgNavi.setVisibility(View.GONE);
			setIndicatorVisible(false);
			if ((getPlayer().eState != DxbPlayer.STATE.SCAN) &&
				(getPlayer().eState != DxbPlayer.STATE.OPTION_FULL_SCAN) &&
				(getPlayer().eState != DxbPlayer.STATE.OPTION_MANUAL_SCAN))
			{
				getPlayer().playSubtitle(DxbPlayer._ON_);
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
				getPlayer().playSubtitle(DxbPlayer._ON_);
			}

			getMainHandler().postDelayed(mRunnable_hiddenSystemUi, 2000);
		}

		mMenuView.setVisible(state);
	}

	private void reflashUI()
	{
		if(!isTimeShift())
		{
			mView.findViewById(R.id.file_play).setVisibility(View.INVISIBLE);
			mView.findViewById(R.id.air_play).setVisibility(View.INVISIBLE);
			
			cNavi.tvProgress.setVisibility(View.INVISIBLE);
			cNavi.tvEPG_P.setVisibility(View.INVISIBLE);
			cNavi.tvEPG_F.setVisibility(View.INVISIBLE);
		}
			
		if (getPlayer().isFilePlay() == 1 || getPlayer().getRecordState() != DxbPlayer.REC_STATE.STOP)
		{
			if(isTimeShift())
			{
				mView.findViewById(R.id.file_play).setVisibility(View.VISIBLE);
				mView.findViewById(R.id.air_play).setVisibility(View.GONE);
			}
			
			if(!isSTB())
			{
				cNavi.bUp.setVisibility(View.GONE);
				cNavi.bDown.setVisibility(View.GONE);
			}
		}
		else
		{
			if(!isTimeShift())
			{
				mView.findViewById(R.id.file_play).setVisibility(View.INVISIBLE);
				mView.findViewById(R.id.air_play).setVisibility(View.INVISIBLE);
		
				cNavi.tvProgress.setVisibility(View.INVISIBLE);
				cNavi.tvEPG_P.setVisibility(View.INVISIBLE);
				cNavi.tvEPG_F.setVisibility(View.INVISIBLE);
			}
			else
			{
				mView.findViewById(R.id.file_play).setVisibility(View.GONE);
				mView.findViewById(R.id.air_play).setVisibility(View.VISIBLE);
			}
			
			if(!isSTB())
			{
				cNavi.bUp.setVisibility(View.VISIBLE);
				cNavi.bDown.setVisibility(View.VISIBLE);
			}
		}
	}
	
	public void onStartTrackingTouch(SeekBar seekBar) {
	}

	public void onStopTrackingTouch(SeekBar seekBar) {
	}

	public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
		if (fromUser == true) {
			if (getPlayer().getLocalPlayState() == DxbPlayer.LOCAL_STATE.STOP) {
				if (getPlayer().getRecordState() == DxbPlayer.REC_STATE.STOP)
					return;
				if (getPlayer().setPlay() == false)
					updateToast(getResources().getString(R.string.recording_other_channel));
			}
			getPlayer().setSeekTo(progress*1000);
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

		int id = v.getId();
		
		if(event.getAction() == MotionEvent.ACTION_UP)
		{
			onClick(v);
		}
		else if (event.getAction() == MotionEvent.ACTION_DOWN)
		{
			if(id == R.id.full_ch_up) {
					cNavi.bUp.setBackgroundResource(R.drawable.dxb_portable_channel_up_btn_f);
					cNavi.bDown.setBackgroundResource(R.drawable.dxb_portable_channel_down_btn_n);
			}
			else if(id == R.id.full_ch_down) {
					cNavi.bUp.setBackgroundResource(R.drawable.dxb_portable_channel_up_btn_n);
					cNavi.bDown.setBackgroundResource(R.drawable.dxb_portable_channel_down_btn_f);
			}
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
			&&	(getPlayer().getServiceType() == 1)
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

		if(!isSTB())
		{
		 	if(keyCode == KeyEvent.KEYCODE_BACK)
			{
				getContext().finish_APK_message();
				return false;
			}
		}

		if(press_InputChannel(keyCode))
		{
			return true;
		}

		if(keyCode == KeyEvent.KEYCODE_BOOKMARK)
		{
			showFavorite();
			return false;
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
			if(keyCode == KeyEvent.KEYCODE_DPAD_UP) {
				cNavi.bUp.setBackgroundResource(R.drawable.dxb_portable_channel_up_btn_f);
				cNavi.bDown.setBackgroundResource(R.drawable.dxb_portable_channel_down_btn_n);
				cNavi.bUp.requestFocus();
			}
			else if(keyCode == KeyEvent.KEYCODE_DPAD_DOWN) {
				cNavi.bUp.setBackgroundResource(R.drawable.dxb_portable_channel_up_btn_n);
				cNavi.bDown.setBackgroundResource(R.drawable.dxb_portable_channel_down_btn_f);
				cNavi.bDown.requestFocus();
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
					/*if (getPlayer().getRecordState() != DxbPlayer.REC_STATE.STOP)
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

		if(!isRecord())
		{
			switch (event)
			{	
				case EVENT_RECORD:
					return false;
				
				default:
					break;
			}				
		}
	
		if(!isTimeShift())
		{
			switch (event)
			{	
				case EVENT_INFORMATION:
					showInformation();
				case EVENT_FAST_FORWARD:
				case EVENT_REWIND:
				case EVENT_PLAY_PAUSE:
				case EVENT_PLAY:
				case EVENT_PAUSE:
				case EVENT_STOP:
					return false;
					
				default:
					break;
			}
		}
		
		switch (event)
		{
			case EVENT_CHANNEL:
				setState(false, VIEW_LIST);
			break;
			
			case EVENT_SCREENMODE:
				changeScreenMode();
			break;
			
			case EVENT_EPG:
				if(getPlayer().getServiceType() < 1)
				{
					setState(false, VIEW_EPG);
				}
			break;
			
			case EVENT_CAPTURE:
				mRecordView.setCapture();
			break;
			
			case EVENT_RECORD:
				setRecordStartStop();
			break;
			
			case EVENT_OPTION:
				if (getPlayer().getRecordState() == DxbPlayer.REC_STATE.STOP)
				{
					setState(false, VIEW_OPTION);
				}
			break;
			
			case EVENT_INFORMATION:
				showInformation();
			break;
			
			case EVENT_SCAN:
				getPlayer().stop();
				getPlayer().playSubtitle(DxbPlayer._OFF_);
				getPlayer().setScan(0); // 0-initial scan, 1-rescan, 2-area scan, 3-preset, 4-manual scan
				setScan();
				break;
			case EVENT_AUTOSEARCH_DN:
				getPlayer().stop();
				getPlayer().playSubtitle(DxbPlayer._OFF_);
				setAutoSearch(0);
				break;
			case EVENT_AUTOSEARCH_UP:
				getPlayer().stop();
				getPlayer().playSubtitle(DxbPlayer._OFF_);
				setAutoSearch(1);
				break;
			case EVENT_TTX:
				if (getPlayer().isValidate_Teletext())
				{
					setIndicatorVisible(false);
					setState(false, VIEW_TELETEXT);
				}
			break;
			
			case EVENT_CHANNEL_UP:
				setChannelUp();
				setTitleState(true);
			break;
			
			case EVENT_CHANNEL_DOWN:
				setChannelDown();
				setTitleState(true);
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

	public void startRecord(int iID_Alarm, int iType_repeat)
	{
		if (mRecordView.setRecord(TYPE.ALARM, iID_Alarm, iType_repeat))
		{
			setMenuState(false);
		}
	}

	private void showInformation() {
		if(!mInfoView.isShown())
			mInfoView.setVisible(true);
		else
			mInfoView.setVisible(false);
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
		if (getPlayer().getLocalPlayState() == DxbPlayer.LOCAL_STATE.STOP) {
			if (getPlayer().getRecordState() == DxbPlayer.REC_STATE.STOP)
				return;
			if (iSpeed < NORMAL_SPEED) {
				if (getPlayer().setPlay() == false)
					updateToast(getResources().getString(R.string.recording_other_channel));
			}
		}
		if (getPlayer().getLocalPlayState() == DxbPlayer.LOCAL_STATE.STOP || getPlayer().setPlaySpeed(SpeedList[iSpeed]) != 0) {
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
		if(getPlayer().getLocalPlayState() == DxbPlayer.LOCAL_STATE.PAUSE) {
			setLocalPlayStart();
		} else {
			setLocalPlayPause();
		}
	}

	private void setLocalPlayStart() {
		if (getPlaySpeed() != NORMAL_SPEED) {
			setPlaySpeed_mode(NORMAL_SPEED);
		} else if(getPlayer().getLocalPlayState() == DxbPlayer.LOCAL_STATE.PAUSE) {
			cNavi.ibuPlayPause.setImageResource(R.drawable.dxb_ic_media_pause);
			getPlayer().setPlayPause(2, true);
		} else if(getPlayer().getLocalPlayState() == DxbPlayer.LOCAL_STATE.PLAYING) {
			int TimeDiff = getPlayer().getDuration() - mCurrentTime;
			if(TimeDiff > 5000)
				getPlayer().setSeekTo(getPlayer().getDuration());
		}
	}

	private void setLocalPlayPause() {
		if (getPlaySpeed() != NORMAL_SPEED) {
			setPlaySpeed_mode(NORMAL_SPEED);
		}
		if(getPlayer().getLocalPlayState() == DxbPlayer.LOCAL_STATE.PAUSE) {
			return;
		} else if(getPlayer().getLocalPlayState() == DxbPlayer.LOCAL_STATE.PLAYING) {
			cNavi.ibuPlayPause.setImageResource(R.drawable.dxb_ic_media_play);
			getPlayer().setPlayPause(2, false);
		} else if(getPlayer().getRecordState() == DxbPlayer.REC_STATE.STOP) {
			//cNavi.menu[MENU_RECORD].vMenu.requestFocus();
			mRecordView.setRecord(TYPE.TIME_SHIFT);
			getPlayer().setPlay();
			//setGroupState(false);
			cNavi.ibuPlayPause.setImageResource(R.drawable.dxb_ic_media_play);
			getPlayer().setPlayPause(2, false);
			reflashUI();
		} else {
			if (getPlayer().setPlay() == false)
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
			if(getPlayer().getLocalPlayState() != DxbPlayer.LOCAL_STATE.STOP)
			{
				getPlayer().setLocalPlayStop();
			}
			else
			{
				if (getPlayer().getRecordState() == DxbPlayer.REC_STATE.RECORDING)
				{
					setEvent(EVENT_RECORD);
				}
			}
			reflashUI();
		}
	}

	private void setChangeAudio() {
		getMainHandler().removeCallbacks(mRunnable_PlayMode);
		String langCode = getPlayer().changeAudio();
		if (langCode != null) {
			cLocalPlay.tvPlayMode.setText(DxbUtil.getLanguageText(langCode));
			cLocalPlay.vPlayInfo.setVisibility(View.VISIBLE);
			getMainHandler().postDelayed(mRunnable_PlayMode, 5000);
		}
	}

	private void setChangeSubtitle() {
		getMainHandler().removeCallbacks(mRunnable_PlayMode);
		String langCode = getPlayer().changeSubtitle();
		if (langCode != null) {
			if (getPlayer().cOption.caption <= getPlayer().getSubtitleCount(0)) {
				cLocalPlay.tvPlayMode.setText(DxbUtil.getLanguageText(langCode));
			} else {
				cLocalPlay.tvPlayMode.setText("Teletext - " + DxbUtil.getLanguageText(langCode));
			}
		} else {
			cLocalPlay.tvPlayMode.setText("Off");
		}
		cLocalPlay.vPlayInfo.setVisibility(View.VISIBLE);
		getMainHandler().postDelayed(mRunnable_PlayMode, 5000);
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

				cNavi.tvEPG_P.setText(DxbUtil.stringForTime(iStart, iDuration) + "		" + getPlayer().getCurrEPG_Title());

				cNavi.tvProgress.setMax(iDuration);
				cNavi.tvProgress.setKeyProgressIncrement(1);
				cNavi.tvProgress.setProgress(getPlayer().getCurrentTime() / 60 - iStart);
			}
			
			if (getPlayer().isNextEPG())
			{
				int iStart = getPlayer().getNextEPG_StartTime();
				int iDuration = getPlayer().getNextEPG_DurationTime();
				
				cNavi.tvEPG_F.setText(DxbUtil.stringForTime(iStart, iDuration) + "		" + getPlayer().getNextEPG_Title());
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
				getPlayer().playSubtitle(DxbPlayer._ON_);
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

	public Runnable mRunnable_SCDisplay = new Runnable()
	{
		public void run()
		{
			DxbLog(I, "mRunnable_SCDisplay --> run()");
			
			getPlayer().setSCDisplay(false);
			getPlayer().playSubtitle(1);
			//DxbView.clearSurfaceView();
			onUpdateScreen();
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
	public void onAreaChange()
	{
		DxbLog(I, "onAreaChange()");

		setScan();
	}
	
	@Override
	protected void onUpdateScreen()
	{
		DxbLog(I, "onUpdateScreen()");

		if(		(getPlayer().eState == DxbPlayer.STATE.OPTION_FULL_SCAN)
			||	(getPlayer().eState == DxbPlayer.STATE.OPTION_MANUAL_SCAN)
			||	(getPlayer().eState == DxbPlayer.STATE.OPTION_MAKE_PRESET)
		)
		{
			ivBackImage.setVisibility(View.GONE);
			mProgress.setVisibility(View.GONE);	
			vBackVideo.setBackgroundColor(0xFF101010);
			vBackVideo.setVisibility(View.VISIBLE);
			tvBackMessage.setVisibility(View.GONE);

			return;
		}

		int	image = 0, text = 0;
		String info = "";
		
		if(		getPlayer().isPlaying()
			||	(getContext().intentOptionActivity != null)	// / : option --> back --> not_display(no_channel)
		)
		{
			if(getPlayer().getChannelCount() <= 0)
			{
				cTitle.tvTitle.setText(R.string.app_name);
				cNavi.tvTitleVideo.setVisibility(View.INVISIBLE);
				cNavi.tvTitleAudio.setVisibility(View.INVISIBLE);
				cNavi.tvTitleLang.setVisibility(View.INVISIBLE);
				cNavi.tvTitleInfo.setText("");
				cNavi.tvTitle.setText(R.string.app_name);
				cNavi.ivLogo.setVisibility(View.INVISIBLE);
				
				image	= R.drawable.dxb_portable_channel_no_channel_img;
				
				if(getPlayer().getServiceType() == 1)
				{
					text	= R.string.no_pvr_file_found;
				}
				else
				{
					text	= R.string.if_you_want_to_use_tv_scan_channel;
				}
			}
			else
			{
				String VideoMode, AudioMode, AudioLang, AudioInfo;
				int iLogoWidth, iLogoHeight, SubtitleCount;
				
				getPlayer().updateChannelInfo();
				
				cTitle.tvTitle.setText(getPlayer().getServiceName_addThreeDigitNumber());
				
				cNavi.tvTitle.setText(getPlayer().getServiceName_addThreeDigitNumber() + "\n" + getPlayer().getEPGEvent() + "\n" + getPlayer().getEPGDateTime());
				
				VideoMode =getPlayer().getVideoMode();
				if(VideoMode != null)
				{
					cNavi.tvTitleVideo.setText(VideoMode);
					cNavi.tvTitleVideo.setVisibility(View.VISIBLE);
				}
				else
				{
					cNavi.tvTitleVideo.setVisibility(View.INVISIBLE);
				}

				AudioMode = getPlayer().getAudioMode();
				if(AudioMode != null)
				{
					cNavi.tvTitleAudio.setText(AudioMode.toUpperCase());
					cNavi.tvTitleAudio.setVisibility(View.VISIBLE);
				}
				else
				{
					cNavi.tvTitleAudio.setVisibility(View.INVISIBLE);
				}

				AudioLang = getPlayer().getAudioLang();
				if(AudioLang != null)
				{
					cNavi.tvTitleLang.setText(AudioLang);
					cNavi.tvTitleLang.setVisibility(View.VISIBLE);
				}
				else
				{
					cNavi.tvTitleLang.setVisibility(View.INVISIBLE);
				}
/*
				AudioInfo = getPlayer().getAudioInfo();
				if(AudioInfo != null)
				{
					cNavi.tvTitleInfo.setText(AudioInfo);
				}
				else
				{
					cNavi.tvTitleInfo.setText("");
				}
*/
				iLogoWidth = getPlayer().getLogoImageWidth();
				iLogoHeight = getPlayer().getLogoImageHeight();
				if(iLogoWidth > 0 && iLogoHeight > 0)
				{
					Bitmap bmLogo = Bitmap.createBitmap(iLogoWidth, iLogoHeight, Bitmap.Config.ARGB_8888);
					bmLogo.copyPixelsFromBuffer(IntBuffer.wrap(getPlayer().getLogoImage()));

					cNavi.ivLogo.setAdjustViewBounds(true);
					cNavi.ivLogo.setScaleType(ImageView.ScaleType.FIT_XY);
					cNavi.ivLogo.setImageBitmap(bmLogo);
					cNavi.ivLogo.setVisibility(View.VISIBLE);
				}
				else
				{
					cNavi.ivLogo.setVisibility(View.INVISIBLE);
				}

				SubtitleCount = getPlayer().getSubtitleCount();
				if(SubtitleCount > 0)
				{
				if (getPlayer().getCountryCode() == DxbPlayer.ISDBT_COUNTRY_JAPAN)		
				{
					cNavi.ivSubtitle.setBackgroundResource(R.drawable.dxb_portable_title_subtitle_japan);
					}
					else
					{
						cNavi.ivSubtitle.setBackgroundResource(R.drawable.dxb_portable_title_subtitle_brazil);
					}
					cNavi.ivSubtitle.setVisibility(View.VISIBLE);
				}
				else
				{
					cNavi.ivSubtitle.setVisibility(View.INVISIBLE);
				}

				if(getPlayer().getServiceType() == 1)
				{
					if(getPlayer().getLocalPlayState() == LOCAL_STATE.STOP)
					{
						image = R.drawable.ic_control_play;
					}
				}
				//	else(getPlayer().getServiceType() <= 2){}	--> check!!!
				else if (getPlayer().getSCDisplay() == true)
				{
					int iSCStatus = getPlayer().getSCStatus();
					
					image	= R.drawable.dxb_portable_channel_no_play_img;
					text	= R.string.please_check_bcas_card;
					info	= getResources().getString(R.string.bcas_card_error);
					
					switch(iSCStatus)
					{
						case SCInfoData.SC_ERR_CARD_DETECT:
							info = getResources().getString(R.string.bcas_card_error);
						break;
						
						case SCInfoData.SC_ERR_INVALID_NAD:
							info += " [INVALID_NAD] !!!";
						break;
						
						case SCInfoData.SC_ERR_RECEIVE_EDC:
							info += " [RECEIVE_EDC] !!!";
						break;
						
						case SCInfoData.SC_ERR_RECEIVE_OTHER:
							info += " [RECEIVE_OTHER] !!!";
						break;
						
						case SCInfoData.SC_ERR_PROTOCOL_TYPE:
							info += " [PROTOCOL_TYPE] !!!";
						break;
						
						case SCInfoData.SC_ERR_CMD_FAIL:
							info += " [CMD_FAIL] !!!";	
						break;
						
						case SCInfoData.SC_ERR_OPEN:
							info += " [OPEN]  !!!";	
						break;
						
						case SCInfoData.SC_ERR_CLOSE:
							info += " [CLOSE] !!!";	
						break;
						
						case SCInfoData.SC_ERR_RESET:
							info += " [RESET] !!!";			
						break;
						
						case SCInfoData.SC_ERR_GET_ATR:
							info += " [GET_ATR] !!!";
						break;
						
						default:
							info = getResources().getString(R.string.invalid_bcas_card_please_check_bcas_card);
						break;
					}
				}
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
					
					if(getPlayer().getRecordState() != DxbPlayer.REC_STATE.STOP)
					{
						setRecordStartStop();
					}
					
					getPlayer().setPlayPause(getPlayer().getServiceType(), false);
				}
				else if (getPlayer().isRadio())
				{
					//image	= R.drawable.dxb_portable_channel_radio_img;
				}
			}
			
			if(image == 0 && getPlayer().bcas_message_keep == true && getPlayer().bcas_message_display== true && getPlayer().cOption.seamless_change == 1)
			{
				ivBackImage.setVisibility(View.GONE);
				mProgress.setVisibility(View.GONE);	
				vBackVideo.setBackgroundColor(0xFF101010);
				vBackVideo.setVisibility(View.VISIBLE);
				tvBackMessage.setVisibility(View.GONE);
			}
			else if (image == 0)
			{
				ivBackImage.setVisibility(View.GONE);

				if(getPlayer().isVideoPlay())
				{
					mProgress.setVisibility(View.GONE);
					vBackVideo.setBackgroundColor(0x00000000);
				}
				else if(!getPlayer().isVideoOutput())
				{
					if(!getPlayer().isVideoUpdate())
					{
						vBackVideo.setVisibility(View.GONE);					
					}
				}
				else if((getPlayer().isVideoOutput()) && (!getPlayer().isVideoUpdate()))
				{
					mProgress.setVisibility(View.VISIBLE);
					vBackVideo.setBackgroundColor(0xFF101010);
					vBackVideo.setVisibility(View.VISIBLE);
				}
			}
			else
			{
				mProgress.setVisibility(View.GONE);
				ivBackImage.setBackgroundResource(image);
				ivBackImage.setVisibility(View.VISIBLE);				
				vBackVideo.setBackgroundColor(0xFF101010);
				vBackVideo.setVisibility(View.VISIBLE);
			}
			
			if(text == 0)
			{
				tvBackMessage.setVisibility(View.GONE);
				
//				cNavi.bUnlock.setVisibility(View.INVISIBLE);
			}
			else
			{
				mInfoView.setVisible(false);
				
				if(text == R.string.the_service_is_locked_the_rating_is)
				{
					tvBackMessage.setText(getResources().getString(text) + " " + (getPlayer().cOption.age+3));
//					cNavi.bUnlock.setVisibility(View.VISIBLE);
				}
				else if(text == R.string.please_check_bcas_card)
				{
					tvBackMessage.setText(info + "\n" + getResources().getString(text));
				}
				else
				{
					tvBackMessage.setText(getResources().getString(text));
//					cNavi.bUnlock.setVisibility(View.INVISIBLE);
				}

				tvBackMessage.setVisibility(View.VISIBLE);
			}
		}
	}

	@Override
	protected void onUpdateEPG_PF()
	{
		DxbLog(I, "onUpdateEPG_PF()");
		
		if(		(getPlayer().eState == DxbPlayer.STATE.OPTION_FULL_SCAN)
			||	(getPlayer().eState == DxbPlayer.STATE.OPTION_MANUAL_SCAN)
			||	(getPlayer().eState == DxbPlayer.STATE.OPTION_MAKE_PRESET)
		)
		{
			return;
		}
		
		if(getPlayer().getServiceType() == 1)
		{
			mView.findViewById(R.id.pvr_controller_group).setVisibility(View.VISIBLE);
			return;
		}
		
		cNavi.tvEPG_P.setText("");
		cNavi.tvEPG_F.setText("");
		getPlayer().setCurrentRating(0);
		
		if(getPlayer().isPlaying())
		{
			getPlayer().setEPG_PF();
	        
			if (getPlayer().isCurrEPG())
			{
				int iStart = getPlayer().getCurrEPG_StartTime();
				int iDuration = getPlayer().getCurrEPG_DurationTime();
				
				cNavi.tvEPG_P.setText(DxbUtil.stringForTime(iStart, iDuration) + "		" + getPlayer().getCurrEPG_Title());
				
				cNavi.tvProgress.setKeyProgressIncrement(1);
				cNavi.tvProgress.setProgress(getPlayer().getCurrentTime() / 60 - iStart);
				
				mView.findViewById(R.id.pvr_controller_group).setVisibility(View.VISIBLE);
			}
			else if(getPlayer().getRecordState() == DxbPlayer.REC_STATE.RECORDING)
			{
				mView.findViewById(R.id.pvr_controller_group).setVisibility(View.VISIBLE);
			}
			else
			{
				mView.findViewById(R.id.pvr_controller_group).setVisibility(View.INVISIBLE);
			}
			
			if (getPlayer().isNextEPG())
			{
				int iStart = getPlayer().getNextEPG_StartTime();
				int iDuration = getPlayer().getNextEPG_DurationTime();
				
				cNavi.tvEPG_F.setText(DxbUtil.stringForTime(iStart, iDuration) + "	  " + getPlayer().getNextEPG_Title());
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
	
	@Override
	protected void send_Event(MSG_VIEW msg)
	{
		DxbLog(I, "send_Event(msg.evetn=" + msg.event + ")");
		
		if(msg.event == EVENT.SCAN_PERCENT)
		{
			String	return_Count = "  TV : " + getPlayer().getChannel_Count(0) + ", Radio : " + getPlayer().getChannel_Count(1);

/*			mDialog_Scan.setMessage(		"Please wait while searching...\n[ "
														+	msg.iVal + "% ]"
														+	return_Count
												);
	*/		
		}
	}
}
