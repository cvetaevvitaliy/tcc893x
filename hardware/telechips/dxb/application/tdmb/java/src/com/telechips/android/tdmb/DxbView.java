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

package com.telechips.android.tdmb;

import com.telechips.android.tdmb.player.TDMBPlayer;

import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Rect;
import android.media.AudioManager;
import android.os.Handler;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.KeyEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.AbsoluteLayout;
import android.widget.ImageView;
import android.widget.TextView;

public class DxbView
{
	static String TAG = "[[[DxbView]]] ";

	private static final int D = 0;
	private static final int E = 1;
	private static final int I = 2;
	private static final int V = 3;
	private static final int W = 4;
	
	private static Component			gComponent;
	private static Information			gInformation;
	private static Information.OPTION	gInfo_Option;

	// DxbView State
	public enum STATE
	{
		LIST,
		FULL,
		EPG,
		NULL
	}
	public static STATE	eState			= STATE.NULL;
	
	// SurfaceView
	static SurfaceView mSurfaceView;
	static SurfaceHolder mSurfaceHolder;

	/*	Display	*/
	int	Surface_width;
	int	Surface_height;

	// coordinates
	private static final int COORDINATES_X = 0x00;
	private static final int COORDINATES_Y = 0x01;	

	static Handler mHandler_Main = new Handler();

	// Setting DB
	public static DxbDB_Setting mDB;
	
	// Up/Down
	static final int UP	= 0;
	static final int DOWN = 1;
	
	//	Option
	public static Intent intentSubActivity;
	
	static AudioManager mAudioManager;
	static int mAudioMaxVolume = 0;
	static int mAudioCurrentVolume = 0;
	
	static void init()
	{
		gComponent		= Manager_Setting.g_Component;
		gInformation	= Manager_Setting.g_Information;
		gInfo_Option	= Manager_Setting.g_Information.cOption;
		
		initSystemSet();

		DxbView_Indicator.init();
		DxbView_List.init();
		DxbView_Full.init();
		DxbView_Record.init();
		DxbPlayer.initView();
		
		setDefaultValue();
		Manager_Setting.mContext.getWindow().addFlags(// lock 화면 띄우기 전에 자신의 화면을 먼저 띄운다.
													WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED
													// 키잠금 해제하기
													| WindowManager.LayoutParams.FLAG_DISMISS_KEYGUARD
													// screen on 을 유지
													| WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON
													// 화면 on
													| WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON);
	}

	private static void initSystemSet()
	{
		Log.i(TAG, "initSystemSet()");
		Display display = ((WindowManager)Manager_Setting.mContext.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
		
		gInformation.cCOMM.iDisplayWidth	= display.getWidth();
		gInformation.cCOMM.iDisplayHeight	= display.getHeight();

		mAudioManager = (AudioManager)Manager_Setting.mContext.getSystemService(Context.AUDIO_SERVICE);
		mAudioMaxVolume = mAudioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
		mAudioCurrentVolume = mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
		//Log.w(TAG, "StreamMaxVolume : " + mAudioMaxVolume);
		//Log.w(TAG, "StreamVolume    : " + mAudioCurrentVolume);
		//mAudioManager.setStreamVolume(AudioManager.STREAM_MUSIC, mAudioMaxVolume, 0);
		
		DisplayMetrics displayMetrics = new DisplayMetrics();
		Manager_Setting.mContext.getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
		gInformation.cCOMM.fDensity	= displayMetrics.density;
	}

	static boolean mIsPrepared = false;

	static TDMBPlayer.OnPreparedListener ListenerOnPrepared = new TDMBPlayer.OnPreparedListener()
	{
		public void onPrepared(TDMBPlayer player, int idx, int ret)
		{
			if(idx != player.mModuleIndex)
				return;
			
			Log.d(TAG,		"OnPreparedListener   "
						+	"     : idx : "
						+   idx
						+	"     : TV : "
						+	gInformation.cCOMM.iCurrent_TV);
			
			Log.d(TAG, "Prepare ret : " + ret);

			if(ret != 0)
			{
				try {
					Thread.sleep(1000);
				} catch(InterruptedException e) {}
				
				DxbPlayer.setPrepare();
				return;
			}
			
			if(mIsPrepared == true)
				return;
			mIsPrepared = true;

			DxbView_List.updateChannelList(0);
			if(		(gInformation.cCOMM.iCurrent_TV >= gInformation.cCOMM.iCount_TV)
				||	(gInformation.cCOMM.iCurrent_TV < 0)
			)
			{
				gInformation.cCOMM.iCurrent_TV = 0;
			}

			if(gInformation.cCOMM.iCurrent_TV >= 0)
			{
				updateScreen();
				
				gInfo_Option.language_audio	= 0;
				gInfo_Option.language_subtitle	= 0;
				if(gInformation.cCOMM.curChannels != null)
				{
					gInformation.cCOMM.curChannels.moveToPosition(gInformation.cCOMM.iCurrent_TV);
				}
				
				setChannel();
				DxbView_List.iIndex_Tab_old	= 0;
			}
			else if(gInformation.cLIST.iCount_Tab > 1)
			{
				DxbView_List.updateChannelList(1);
				if(		(gInformation.cCOMM.iCurrent_Radio >= gInformation.cCOMM.iCount_Radio)
					||	(gInformation.cCOMM.iCurrent_Radio < 0)
				)
				{
					gInformation.cCOMM.iCurrent_Radio = 0;
				}
				if(gInformation.cCOMM.iCurrent_Radio >= 0)
				{
					if(gInformation.cCOMM.curChannels != null)
					{
						Cursor c = gInformation.cCOMM.curChannels;
						c.moveToPosition(gInformation.cCOMM.iCurrent_Radio);
					}
					
					gInfo_Option.language_audio	= 0;
					gInfo_Option.language_subtitle	= 0;
					setChannel();
				}
			}

			if(DxbRemote.getChannel() == 1)
			{
				DxbScan.startFull();
			}
		}
	};
	
	static TDMBPlayer.OnAudioOutputListener ListenerOnAudioOutput = new TDMBPlayer.OnAudioOutputListener()
	{
		public void onAudioOutputUpdate(TDMBPlayer player)
		{
			DxbLog(I, "onAudioOutputUpdate()");
		}
	};
	
	static TDMBPlayer.OnVideoOutputListener ListenerOnVideoOutput = new TDMBPlayer.OnVideoOutputListener()
	{
		public void onVideoOutputUpdate(TDMBPlayer player)
		{
			DxbLog(I, "onVideoOutputUpdate()");
			
			gInformation.cCOMM.isEnable_Video	= true;
			DxbPlayer.playSubtitle(DxbPlayer._ON_);

			//DxbPlayer.setDATAPrepare();

			/*if (mType == TYPE_DAB)
			{
				setButtonDim(mRECBtn, R.drawable.dxb_portable_toolbar_record_btn_n, true);
				setButtonDim(mCaptureBtn, R.drawable.dxb_portable_toolbar_capture_btn_n, true);
			}
			else
			{
				setButtonDim(mRECBtn, R.drawable.dxb_portable_toolbar_record_btn_f, false);
				setButtonDim(mCaptureBtn, R.drawable.dxb_portable_toolbar_capture_btn_f, false);
			}*/
		}
	};
	
	static TDMBPlayer.OnErrorListener ListenerOnError = new TDMBPlayer.OnErrorListener()
	{
		public boolean onError(TDMBPlayer player, int what, int extra)
		{
			DxbLog(I, "Play Error!");
			
			return false;
		}
	};
	
	static void initSurfaceView()
	{
		DxbLog(I, "initSurfaceView()");
		
		mSurfaceView = (SurfaceView)Manager_Setting.mContext.findViewById(R.id.surface_view);
		if (mSurfaceView == null)
		{
			DxbLog(D, "SurfaceView is Null");
			return;
		}
		
		SurfaceHolder holder = mSurfaceView.getHolder();
		if (holder == null)
		{
			DxbLog(D, "++++++++++++++++++ mSurfaceHolder is Null");
			return;
		}

		holder.addCallback(mSHCallback);
		holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
		holder.setFixedSize(960, 720);
	}
	
	static SurfaceHolder.Callback mSHCallback = new SurfaceHolder.Callback()
	{
		public void surfaceChanged(SurfaceHolder holder, int format, int w, int h)
		{
			if(mSurfaceHolder == null)
			{
				return;
			}
			DxbPlayer.setLCDUpdate();
			DxbPlayer.useSurface(0);
			initSystemSet();
			if (eState == STATE.FULL)
				FullView_SetLayout();
		}

		public void surfaceCreated(SurfaceHolder holder)
		{
			DxbLog(I, "surfaceCreated(holder=" + holder + ")");

			mSurfaceHolder = holder;
			mSurfaceHolder.setFixedSize(960, 720);
			if(DxbPlayer.mPlayer != null)
			{
				DxbPlayer.mPlayer.setDisplay(mSurfaceHolder);
				DxbPlayer.setSurface();
			}
			else
			{
				DxbPlayer.setListener_Player();
				DxbPlayer.setSurface();
				DxbPlayer.start();
			}
			
			if(eState == STATE.LIST)
			{
				int	iCurrentTab	= DxbView_List.getTab();
				if(gInformation.cCOMM.iCount_Current > 0)
				{
					gComponent.listview.lvService.requestFocus();
					if(iCurrentTab == 0)
					{
						gComponent.listview.lvService.setSelection(gInformation.cCOMM.iCurrent_TV);
					}
					else
					{
						gComponent.listview.lvService.setSelection(gInformation.cCOMM.iCurrent_Radio);
					}
				}
			}
		}

		public void surfaceDestroyed(SurfaceHolder holder)
		{
			// after we return from this we can't use the surface any more
			DxbLog(I, "surfaceDestroyed(holder="+holder+")");
			
			if(DxbPlayer.eState == DxbPlayer.STATE.SCAN)
			{
				DxbPlayer.eState	= DxbPlayer.STATE.UI_PAUSE;
				DxbScan.cancel();
				DxbView_Message.removeDialog();
			}
			else if (DxbView_Record.eState == DxbView_Record.STATE.RECORDING)
			{
				if(DxbPlayer.mPlayer != null)
				{
					DxbPlayer.mPlayer.setRecStop();
				}
			}
			
			DxbView_Record.disconnect();
			mSurfaceHolder = null;
		}
	};

	private static void setDefaultValue()
	{
		DxbLog(I, "setDefaultValue()");
		
		eState			= STATE.LIST;
		DxbPlayer.eState		= DxbPlayer.STATE.GENERAL;
		
		DxbPlayer.setDefaultValue();
	}

	static void setState(boolean isRefresh, STATE eChange_state)
	{
		DxbLog(I, "Dxb_setViewState(eChange_state="+eChange_state+")");
		
		if(		(!isRefresh)
			&&	(eState == eChange_state)
		)
		{
			DxbLog(V, "Fail : Dxb_setViewState() - state error");
			
			return;
		}
		
		DxbPlayer.playSubtitle(DxbPlayer._OFF_);
		
		if(		(eState == STATE.LIST)
			&&	(eChange_state == STATE.FULL)
			&&	(gInformation.cCOMM.curChannels != null)
		)
		{
			if(gInformation.cCOMM.curChannels.getCount() <= 0)
			{
				if(gInformation.cLIST.iCount_Tab > 1)
				{
					int	iCurrentTab	= DxbView_List.getTab();
					if(		(iCurrentTab == 0)
						&&	(gInformation.cCOMM.iCount_Radio > 0)
					)
					{
						//tabHost.setCurrentTab(1);
					}
					else if(gInformation.cCOMM.iCount_TV > 0)
					{
						//tabHost.setCurrentTab(0);
					}
				}				
			}
		}		

		updateBackgroundLayout(eChange_state);
		
		if(eChange_state == STATE.FULL)
		{
			if(!DxbPlayer.isValidate_Teletext())
			{
				gInfo_Option.teletext	= 0;
			}
			
			DxbView_Full.setTitleState(false);
			DxbLog(D, "gInfo_Option.teletext"+gInfo_Option.teletext);
			if(gInfo_Option.teletext == 1)
			{
				DxbPlayer.setState_Teletext(true);
				//DxbView_Full.setGroupState(false);
			}
			else
			{
				DxbPlayer.setState_Teletext(false);
				//DxbView_Full.setGroupState(true);
			}
			DxbView_Full.setGroupState(false);
			
			if(		isRefresh
				&&	(DxbPlayer.eState == DxbPlayer.STATE.OPTION_MANUAL_SCAN)
			)
			{
				DxbPlayer.eState = DxbPlayer.STATE.GENERAL;
				DxbScan.startManual(gInfo_Option.scan_manual);
			}
		}
		else
		{
			gComponent.indicator.ivSection.setVisibility(View.VISIBLE);
			gComponent.indicator.timeIndicator.setVisibility(View.VISIBLE);
			DxbView_Indicator.Signal_visible();
		}

		int	layoutWidth, layoutHeight;
		int	startX, startY;
		
		if(eChange_state == STATE.LIST)
		{
			// screen mode Full on listview
			
			if(DxbPlayer.VISIBLE_LIST_preview_half)
			{
				layoutWidth		= getToPosition(COORDINATES_X, 390);	// (400-2=398) 4+390+4
				layoutHeight	= getToPosition(COORDINATES_Y, 220);	// (227) 3+220+4

				startX	= getToPosition(COORDINATES_X, 406);			// 400+2+4=406
				startY	= getToPosition(COORDINATES_Y, 82);				// 25+(455/8=57)=82
			}
			else
			{
				layoutWidth		= getToPosition(COORDINATES_X, 280);
				layoutHeight	= getToPosition(COORDINATES_Y, 210);

				startX	= getToPosition(COORDINATES_X, 507);
				startY	= getToPosition(COORDINATES_Y, 87);
			}

			//DxbPlayer.releaseSurface();
			View child = (View)Manager_Setting.mContext.findViewById(R.id.surface_view);
			AbsoluteLayout.LayoutParams lp = new AbsoluteLayout.LayoutParams(
                    layoutWidth,
                    layoutHeight, startX, startY);

			child.setLayoutParams(lp);

			if(gInformation.cCOMM.iCount_TV == 0)
			{
				gComponent.listview.bScan.requestFocus();
			}
		}
		else if(eChange_state == STATE.FULL)
		{
		/*
			// screen mode setting value on fullview
			layoutWidth		= getToPosition(COORDINATES_X, 800);
			layoutHeight	= getToPosition(COORDINATES_Y, 480);

			startX	= getToPosition(COORDINATES_X, 0);
			startY	= getToPosition(COORDINATES_Y, 0);

			switch(DxbView_Full.eState_ScreenSize)
			{
				case DxbView_Full.SCREENMODE_LETTERBOX:
					{
						float disp_ratio = layoutWidth / layoutHeight;
						float video_ratio = 4 / 3;
						if(disp_ratio > video_ratio)
						{
							int width = layoutHeight * 4 / 3;
							startX = (layoutWidth - width) / 2;
							layoutWidth = width;
						}
						else
						{
							int height = layoutWidth * 3 / 4;
							startX = (layoutHeight - height) / 2;
							layoutHeight = height;
						}
					}
					break;

				case DxbView_Full.SCREENMODE_PANSCAN:
					{
					}
					break;

				case DxbView_Full.SCREENMODE_FULL:
					break;
			}

			//DxbPlayer.releaseSurface();
			View child = (View)Manager_Setting.mContext.findViewById(R.id.surface_view);
			AbsoluteLayout.LayoutParams lp = new AbsoluteLayout.LayoutParams(
                    layoutWidth,
                    layoutHeight, startX, startY);

			child.setLayoutParams(lp);

			//fullButton_menu2.requestFocus();
			//gComponent.fullview.navi.menu.bMenu1.requestFocus();
			//mMainHandler.postDelayed(setHideControlRunnable, HIDETIMER);
		*/
			FullView_SetLayout();
		}
		else if(eChange_state == STATE.EPG)
		{
			// screen mode Full on epgview
			
			layoutWidth		= getToPosition(COORDINATES_X, 280);
			layoutHeight	= getToPosition(COORDINATES_Y, 200);

			startX	= getToPosition(COORDINATES_X, 507);
			startY	= getToPosition(COORDINATES_Y, 99);

			//DxbPlayer.releaseSurface();
			View child = (View)Manager_Setting.mContext.findViewById(R.id.surface_view);
			AbsoluteLayout.LayoutParams lp = new AbsoluteLayout.LayoutParams(
                    layoutWidth,
                    layoutHeight, startX, startY);

			child.setLayoutParams(lp);

			DxbPlayer.updateList_EPG();
		}

		updateScreen();
	}
	
	static boolean backState(int keyCode, KeyEvent event, STATE state)
	{
		Log.d(TAG, "backState(state="+state+")");
		
		if(state == STATE.LIST)
		{
			return false;
		}
		else if(eState == STATE.FULL)
		{
			if(DxbPlayer.isShown_Teletext())
			{
				DxbPlayer.stopTeletext();
				DxbPlayer.setState_Teletext(false);
				DxbView_Full.setGroupState(true);
			}
			else if(Manager_Setting.g_Component.fullview.navi.vgNavi.isShown())
			{
				DxbView_Full.setGroupState(false);
			}
			else
			{
				setState(false, STATE.LIST);
			}
		}
		else
		{
			setState(false, STATE.FULL);
		}
		
		return true;
	}

	static Rect FullView_SetLayout()
	{
		Rect crop_rect = new Rect();

		int	layoutWidth, layoutHeight;
		int	startX, startY;

		// screen mode setting value on fullview
		layoutWidth		= getToPosition(COORDINATES_X, 800);
		layoutHeight	= getToPosition(COORDINATES_Y, 480);

		startX	= getToPosition(COORDINATES_X, 0);
		startY	= getToPosition(COORDINATES_Y, 0);

		Log.e(TAG, "ScreenSize = " + DxbView_Full.eState_ScreenSize);

		float disp_ratio = (layoutWidth / (float)layoutHeight);
		float video_ratio = (float)(4 / 3.0);

		int width, height;

		switch(DxbView_Full.eState_ScreenSize)
		{
			case DxbView_Full.SCREENMODE_LETTERBOX:
				{
					if(disp_ratio > video_ratio)
					{
						width = (int)(layoutHeight * 4 / 3.0);
						startX = (layoutWidth - width) / 2;
						layoutWidth = width;
					}
					else if(disp_ratio < video_ratio)
					{
						height = (int)(layoutWidth * 3 / 4.0);
						startY = (layoutHeight - height) / 2;
						layoutHeight = height;
					}
				}
				break;

			case DxbView_Full.SCREENMODE_PANSCAN:
				{
					int cropX = 0;
					int cropY = 0;
					int cropWidth = 320;
					int cropHeight = 240;
					int r;

					if(disp_ratio > video_ratio)
					{
						cropHeight = (int)(240 * video_ratio / disp_ratio);
						cropY = (240 - cropHeight) / 2;
                        r = cropY%4;                    
                        cropY-=r;
                        cropHeight+=r;
                        cropHeight = (cropHeight>>2)<<2;
					}
					else if(disp_ratio < video_ratio)
					{
						cropWidth = (int)(320 * disp_ratio / video_ratio);
						cropWidth = ((cropWidth+15)>>4)<<4;
						cropX = (320 - cropWidth) / 2;
					}
					crop_rect.set(cropX, cropY, cropWidth + cropX, cropHeight + cropY);
				}
				break;

			case DxbView_Full.SCREENMODE_FULL:
				break;
		}

		Log.d(TAG, "X = " + startX);
		Log.d(TAG, "Y = " + startY);
		Log.d(TAG, "W = " + layoutWidth);
		Log.d(TAG, "H = " + layoutHeight);

		View child = (View)Manager_Setting.mContext.findViewById(R.id.surface_view);
		AbsoluteLayout.LayoutParams lp = new AbsoluteLayout.LayoutParams(
                layoutWidth,
                layoutHeight, startX, startY);

		child.setLayoutParams(lp);

		return crop_rect;
	}

	private static int getToPosition(int co, int position)
	{
		DxbLog(I, "getToPosition()");
		int	return_position	= 0;
		
		if(co == COORDINATES_X)
		{
			return_position	= position * gInformation.cCOMM.iDisplayWidth / 800;
		}
		else if(co == COORDINATES_Y)
		{
			return_position	= position * gInformation.cCOMM.iDisplayHeight / 480;
		}
		
		return return_position;
	}
	
	private static void updateBackgroundLayout(STATE eChange_state)
	{
		DxbLog(I, "Dxb_updateBackgroundLayout(eChange_state=" + eChange_state + ")");
		
		//	(All)GONE
		{
			if(eChange_state != STATE.LIST)
			{
				Manager_Setting.mContext.findViewById(R.id.layout_list).setVisibility(View.GONE);
			}
			
			if(eChange_state != STATE.FULL)
			{
				Manager_Setting.mContext.findViewById(R.id.layout_full).setVisibility(View.GONE);
			}

			if(eChange_state != STATE.EPG)
			{
				Manager_Setting.mContext.findViewById(R.id.layout_epg).setVisibility(View.GONE);
			}
		}
		
		if(eChange_state == STATE.LIST)
		{
			Manager_Setting.mContext.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
			
			//showSystemUi(true);
			Manager_Setting.mContext.findViewById(R.id.id_channel_bg).setBackgroundColor(R.color.color_2);
			Manager_Setting.mContext.findViewById(R.id.layout_list).setVisibility(View.VISIBLE);
		}
		else if(eChange_state == STATE.FULL)
		{
			Manager_Setting.mContext.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN | 0x80000000, WindowManager.LayoutParams.FLAG_FULLSCREEN | 0x80000000); 
			Manager_Setting.mContext.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

			//showSystemUi(false);
			Manager_Setting.mContext.findViewById(R.id.id_channel_bg).setBackgroundColor(0xFF000000);
			
			gComponent.fullview.navi.vgNavi.setVisibility(View.VISIBLE);
			Manager_Setting.mContext.findViewById(R.id.layout_full).setVisibility(View.VISIBLE);
		}
		else if(eChange_state == STATE.EPG)
		{
			Manager_Setting.mContext.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

			//showSystemUi(true);
			//Manager_Setting.mContext.findViewById(R.id.id_channel_bg).setBackgroundResource(DxbPlayer_Control.getRdrawable_epgBG());
			Manager_Setting.mContext.findViewById(R.id.layout_epg).setVisibility(View.VISIBLE);
		}
		
		eState			= eChange_state;
	}
	
	public static void updateScreen()
	{
		DxbLog(I, "updateScreen()");
		View			View	= null;
		ImageView		imageView = null;
		TextView	textView_info = null;
		int	image = 0, text = 0, list_text = 0, info_text = 0;
		
		final int TEXT = 111;
		
		//	Select - state
		if(gInformation.cCOMM.iCount_Current <= 0)
		{
			image	= R.drawable.dxb_portable_channel_no_channel_img;
			text	= R.string.no_channel;
			
			if(DxbView_List.getTab() == 1)
			{
				list_text	= R.string.if_you_want_to_use_radio_scan_channel;
			}
			else
			{
				list_text	= R.string.if_you_want_to_use_tv_scan_channel;
			}
			info_text	= list_text;
			
			gComponent.fullview.title.tvTitle.setText(R.string.app_name);
			gComponent.fullview.navi.tvTitle.setText(R.string.app_name);
			gComponent.listview.tvTitle.setText(R.string.no_channel);
		}
		else
		{
			gComponent.fullview.title.tvTitle.setText(DxbPlayer.getServiceName());
			gComponent.fullview.navi.tvTitle.setText(DxbPlayer.getServiceName());
			gComponent.listview.tvTitle.setText(DxbPlayer.getServiceName());
			
			if(gInformation.cINDI.iSignal_Level < 0)
			{
				image	= R.drawable.dxb_portable_channel_weak_signal_img;
				text	= R.string.weak_signal;
				info_text	= R.string.receiving_signal_strength_is_weak;
			}
			else
			{
				if(DxbPlayer.isRadio())
				{
					image	= R.drawable.dxb_portable_channel_radio_img;
				}
				text	= TEXT;
			}
		}
		
		// Select - Display area
		if(eState == STATE.LIST)
		{
			imageView	= (ImageView)Manager_Setting.mContext.findViewById(R.id.list_image);
			View		= gComponent.listview.vVideo;
		}
		else if(eState == STATE.FULL)
		{
			imageView	= (ImageView)Manager_Setting.mContext.findViewById(R.id.full_image);
			View		= gComponent.fullview.vVideo;
			
			textView_info	= (TextView)Manager_Setting.mContext.findViewById(R.id.full_text);
		}
		else if(eState == STATE.EPG)
		{
			imageView	= (ImageView)Manager_Setting.mContext.findViewById(R.id.epg_image);
			View		= gComponent.epgview.vVideo;
		}
		
		// (Video area) - display image
		if(imageView != null)
		{
			// 0-play video, !0-display image
			if(true)//image == 0)
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
		
		// 0-not display, !0-display
		if(text != 0)
		{
			// TEXT-getResources, !TEXT-fixed
			if(text == TEXT)
			{
				if(DxbView_List.getTab() == 1)
				{
					gComponent.listview.tvInformation.setText(Manager_Setting.mContext.getResources().getString(R.string.radio_channel) + " : " + gInformation.cCOMM.iCount_Current);
				}
				else
				{
					gComponent.listview.tvInformation.setText(Manager_Setting.mContext.getResources().getString(R.string.tv_channel) + " : " + gInformation.cCOMM.iCount_Current);
				}			
			}
			else
			{
				gComponent.listview.tvInformation.setText(Manager_Setting.mContext.getResources().getString(text));
			}
		}
		
		// 0-hide, !0-display text
		if(list_text == 0)
		{
			gComponent.listview.tvMessage.setVisibility(android.view.View.GONE);
		}
		else
		{
			gComponent.listview.tvMessage.setText(Manager_Setting.mContext.getResources().getString(list_text));
			gComponent.listview.tvMessage.setVisibility(android.view.View.VISIBLE);
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
				textView_info.setText(Manager_Setting.mContext.getResources().getString(info_text));
				textView_info.setVisibility(android.view.View.VISIBLE);
			}
		}
	}
	
	static void changeChannel(int state)
	{
		DxbLog(I, "changeChannel(state = " + state + ")");
		
		switch(state)
		{
			case UP:
				if (gInformation.cCOMM.iCount_Current > 0)
				{
					if(gInformation.cCOMM.iCurrent_TV == 0)
					{
						DxbView_Message.updateToast(Manager_Setting.mContext.getResources().getString(R.string.first_service));
						
						return;
					}
					else if(gInformation.cCOMM.iCurrent_TV < 0)
					{
						gInformation.cCOMM.iCurrent_TV	= 0;
					}
					else
					{
						gInformation.cCOMM.iCurrent_TV--;
					}

					if(gInformation.cCOMM.curChannels != null)
					{
						gInformation.cCOMM.curChannels.moveToPosition(gInformation.cCOMM.iCurrent_TV);
					}
				}
			break;
			
			case DOWN:
				if(gInformation.cCOMM.iCount_Current > 0)
				{
					
					if(gInformation.cCOMM.iCurrent_TV == gInformation.cCOMM.iCount_TV-1)
					{
						DxbView_Message.updateToast(Manager_Setting.mContext.getResources().getString(R.string.last_service));
						
						return;
					}
					else if(gInformation.cCOMM.iCurrent_TV >= gInformation.cCOMM.iCount_TV)
					{
						gInformation.cCOMM.iCurrent_TV	= gInformation.cCOMM.iCount_TV - 1;
					}
					else
					{
						gInformation.cCOMM.iCurrent_TV++;
					}

					if(gInformation.cCOMM.curChannels != null)
					{
						gInformation.cCOMM.curChannels.moveToPosition(gInformation.cCOMM.iCurrent_TV);
					}
				}
			break;
			
			default:
				return;
		}
		
		resetChannel();
		setChannel();
	}
	
	static void setChannel()
	{
		DxbLog(I, "setChannel()");
		
		if(DxbPlayer.eState == DxbPlayer.STATE.SCAN)
		{
			return;
		}
		
		if(		(gInformation.cCOMM.iCount_Current > 0)
			&&	(gInformation.cCOMM.curChannels != null)
		)
		{
			Log.d(TAG, "gInformation.cCOMM.curChannels.getPosition()="+gInformation.cCOMM.curChannels.getPosition());
			
			if(DxbView_List.getTab() == 1)
			{
				gInformation.cCOMM.iCurrent_Radio = gInformation.cCOMM.curChannels.getPosition();
				mDB.putPosition(1, gInformation.cCOMM.iCurrent_Radio);
			}
			else
			{
				gInformation.cCOMM.iCurrent_TV = gInformation.cCOMM.curChannels.getPosition();
				mDB.putPosition(0, gInformation.cCOMM.iCurrent_TV);
			}
			
			DxbPlayer.setChannel(gInformation.cCOMM.curChannels);
			DxbPlayer.setAudio (gInfo_Option.language_audio);
			DxbPlayer.setAudioDualMono (gInfo_Option.dual_audio);
			
			if(DxbPlayer.isValidate_Teletext())
			{
				gComponent.fullview.navi.bTeletext.setVisibility(View.VISIBLE);
			}
			else
			{
				gComponent.fullview.navi.bTeletext.setVisibility(View.INVISIBLE);
			}

			int sectionID	= DxbPlayer.getRid_Indicator_Section();
			if(sectionID > 0)
			{
				gComponent.indicator.ivSection.setImageResource(sectionID);
			}

			gComponent.fullview.title.tvTitle.setText(DxbPlayer.getServiceName());
			gComponent.fullview.navi.tvTitle.setText(DxbPlayer.getServiceName());
			gComponent.listview.tvTitle.setText(DxbPlayer.getServiceName());

			updateScreen();
			gComponent.listview.lvService.invalidateViews();
		}
		else
		{
			gComponent.fullview.navi.bTeletext.setVisibility(View.INVISIBLE);
			DxbPlayer.eState = DxbPlayer.STATE.GENERAL;
		}
	}
	
	static void resetChannel()
	{
		if(Manager_Setting.ePLAYER == Manager_Setting.PLAYER.DVBT)
		{
			gInfo_Option.language_audio	= 0;
			gInfo_Option.language_subtitle	= 0;
		}
	}
	
	private static void DxbLog(int level, String mString)
	{
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
