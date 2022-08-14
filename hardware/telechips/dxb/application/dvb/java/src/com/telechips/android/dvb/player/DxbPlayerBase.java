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

import com.telechips.android.dvb.player.diseqc.*;
import com.telechips.android.dvb.player.DxbPVR.*;
import com.telechips.android.dvb.player.dvb.*;

import android.annotation.SuppressLint;
import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.Rect;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.FrameLayout;

public abstract class DxbPlayerBase extends FrameLayout implements 
		DVBTPlayer.OnVideoDefinitionUpdateListener, DVBTPlayer.OnPreparedListener,
		DVBTPlayer.OnVideoOutputListener, DVBTPlayer.OnErrorListener,
		DVBTPlayer.OnDebugModeListener, DVBTPlayer.OnDBInformationUpdateListener,
		DVBTPlayer.OnDBInfoUpdateListener, DVBTPlayer.OnScrambledStatusListener
{
	private static enum PLAYER {
		ATSC,
		DVBT,
		DVBS2,
		ISDBT,
		TDMB,
		NULL
	}
	private PLAYER ePLAYER;

	public static final int SCREENMODE_LETTERBOX    = 0;
	public static final int SCREENMODE_PANSCAN      = 1;
	public static final int SCREENMODE_FULL         = 2;

	private int mScreenMode = SCREENMODE_FULL;

	private final Rect          mWindowInsets  = new Rect();
	private SurfaceHolder       mSurfaceHolder = null;
	private VideoDefinitionData mVideoInfo     = null;
	private DVBTPlayer          mPlayer        = null;

	private SurfaceView         mSurfaceView;
	private DxbSubtitle         mDxbSubtitle;
	private DxbTeletext         mDxbTeletext;
	private DxbChannel          mDxbChannel;
	private DxbScan             mDxbScan;
	private DxbPVR              mDxbPVR;
	private DiSEqCDev           mDxbDiSEqC;

	private boolean isSTB	= false;

	// Debug Time
	private long mStartTime;

	public DxbPlayerBase(Context context, AttributeSet attrs)
	{
		super(context, attrs);

		isSTB = "1".equals(property_get("tcc.solution.mbox"));

		if ("1".equals(property_get("tcc.dxb.dvbt.baseband")))
		{
			ePLAYER = PLAYER.DVBS2;
		}
		else
		{
			ePLAYER = PLAYER.DVBT;
		}
		initVideoView(context);
		initSubtitleView(context);
		initTeletextView(context);
		initPlayer();

		DxbLog(I, "DxbPlayerBase(" + ePLAYER + ")");
	}

	@Override
	protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
		DxbLog(D, "onLayout(changed=" + changed
				+ ", l=" + left + ", t=" + top + ", r=" + right + ", b=" + bottom + ")");
		onVideoLayout(left, top, right, bottom);
		onSubtitleLayout(left, top, right, bottom);
		onTeletextLayout(left, top, right, bottom);
	}

	@Override
	protected boolean fitSystemWindows(Rect insets) {
		DxbLog(D, "fitSystemWindows(Rect = " + insets + ")");
		mWindowInsets.set(insets);
		return true;
	}

/*****************************************************************************************************************************************************************************
 *	DxB Standard
 *****************************************************************************************************************************************************************************/
	public boolean isDVB() {
		return (isDVB_T() || isDVB_S2());
	}

	public boolean isDVB_T() {
		return (ePLAYER == PLAYER.DVBT);
	}

	public boolean isDVB_S2() {
		return (ePLAYER == PLAYER.DVBS2);
	}

/*****************************************************************************************************************************************************************************
 *	Player
 *****************************************************************************************************************************************************************************/
	private void initPlayer() {
		mDxbScan = new DxbScan(this);
		mDxbChannel = new DxbChannel(this);
		mDxbPVR = new DxbPVR(this);
		mDxbDiSEqC = new DiSEqCDev();
	}

	public void openPlayer() {
		DxbLog(I, "openPlayer()");
		new Thread(new Runnable() {
			public void run() {
				onCreate();
			}
		}).start();
	}

	public void closePlayer() {
		DxbLog(I, "closePlayer()");
		new Thread(new Runnable() {
			public void run() {
				onDestroy();
			}
		}).start();
	}

/*****************************************************************************************************************************************************************************
 *	Video
 *****************************************************************************************************************************************************************************/
	@SuppressWarnings("deprecation")
	private void initVideoView(Context context) {
		mSurfaceView = new SurfaceView(context);
		mSurfaceView.getHolder().addCallback(mSHCallback);
		mSurfaceView.getHolder().setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
		addView(mSurfaceView);
	}

	private SurfaceHolder.Callback mSHCallback = new SurfaceHolder.Callback() {
		@Override
		public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
			DxbLog(I, "surfaceChanged()");
		}

		@Override
		public void surfaceCreated(SurfaceHolder holder) {
			DxbLog(I, "surfaceCreated()");
			mSurfaceHolder = holder;
			if (mSurfaceHolder != null) {
				openPlayer();
			}
		}

		@Override
		public void surfaceDestroyed(SurfaceHolder holder) {
			DxbLog(I, "surfaceDestroyed()");
			mSurfaceHolder = null;
		}
	};

	private void onVideoLayout(int left, int top, int right, int bottom) {
		int layoutWidth, layoutHeight, videoWidth, videoHeight;
		float layoutRatio, videoRatio;
		VideoDefinitionData videoinfo = mVideoInfo;
		int mode = mScreenMode;

		if (mode != SCREENMODE_FULL && videoinfo != null) {
			switch (videoinfo.aspect_ratio) {
			case 0:
				videoWidth = 16;
				videoHeight = 9;
				break;
			case 1:
				videoWidth = 4;
				videoHeight = 3;
				break;
			default:
				videoWidth = videoinfo.width;
				videoHeight = videoinfo.height;
				break;
			}
			videoRatio = videoWidth / (float)videoHeight;

			layoutWidth = right - left;
			layoutHeight = bottom - top;
			layoutRatio = layoutWidth / (float)layoutHeight;

			DxbLog(D, "onLayout (layout w=" + layoutWidth + ", h=" + layoutHeight + ", ratio=" + layoutRatio + ")");
			DxbLog(D, "onLayout (video  w=" + videoWidth + ", h=" + videoHeight + ", ratio=" + videoRatio + ")");

			if (layoutRatio != videoRatio) {
				if (	(mode == SCREENMODE_LETTERBOX && layoutRatio < videoRatio)
					||	(mode == SCREENMODE_PANSCAN && layoutRatio > videoRatio)
				) {
					videoHeight = (int)(layoutWidth * videoHeight / (float)videoWidth);
					videoWidth = layoutWidth;
				} else {
					videoWidth = (int)(layoutHeight * videoWidth / (float)videoHeight);
					videoHeight = layoutHeight;
				}
				left += (layoutWidth - videoWidth) / 2;
				right -= (layoutWidth - videoWidth) / 2;
				top += (layoutHeight - videoHeight) / 2;
				bottom -= (layoutHeight - videoHeight) / 2;
			}
		}

		mSurfaceView.layout(left, top, right, bottom);
		DxbLog(I, "onLayout (Mode=" + mode
				+ ", l=" + left + ", t=" + top + ", r=" + right + ", b=" + bottom + ")");
	}

/*****************************************************************************************************************************************************************************
 *	Subtitle
 *****************************************************************************************************************************************************************************/
	private void initSubtitleView(Context context) {
		mDxbSubtitle = new DxbSubtitle(context, this);
		addView(mDxbSubtitle);
	}

	private void onSubtitleLayout(int left, int top, int right, int bottom) {
		mDxbSubtitle.layout(left, top, right, bottom);
	}

	protected int getSubtitleCount() {
		return mDxbSubtitle.getCount();
	}

	protected void startSubtitle(int index) {
		mDxbSubtitle.startSubtitle(index);
		mHandler.sendMessage(mHandler.obtainMessage(SET_SUBTITLE_VISIBILITY, 1, 0, 0));
	}

	protected void stopSubtitle() {
		mHandler.sendMessage(mHandler.obtainMessage(SET_SUBTITLE_VISIBILITY, 0, 0, 0));
		mDxbSubtitle.stopSubtitle();
	}

	protected void setSubtitleInformation(Cursor c) {
		mDxbSubtitle.setInformation(c);
	}

	protected String getSubtitle_LanguageCode(int index) {
		return mDxbSubtitle.getLanguageCode(index);
	}

	public void resetSubtitleInformation()
	{
		mDxbSubtitle.resetInformation();
	}

/*****************************************************************************************************************************************************************************
 *	Teletext
 *****************************************************************************************************************************************************************************/
	private void initTeletextView(Context context) {
		LayoutParams lp = new LayoutParams(
				LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);

		mDxbTeletext = new DxbTeletext(context, this);
		addView(mDxbTeletext, lp);
	}

	private void onTeletextLayout(int left, int top, int right, int bottom) {
		mDxbTeletext.layout(left, top, right, bottom);
	}

	protected int getTtxSubtitleCount() {
		return mDxbTeletext.getSubtitleCount();
	}

	protected void startTtxSubtitle(int index) {
		mDxbTeletext.startSubtitle(index);
		mHandler.sendMessage(mHandler.obtainMessage(SET_TTX_SUBTITLE_VISIBILITY, 1, 0, 0));
	}

	protected void stopTtxSubtitle() {
		mHandler.sendMessage(mHandler.obtainMessage(SET_TTX_SUBTITLE_VISIBILITY, 0, 0, 0));
		mDxbTeletext.stopSubtitle();
	}

	protected void setTeletextInformation(Cursor init_cursor, Cursor subtitle_cursor) {
		mDxbTeletext.setTeletextInformation(init_cursor, subtitle_cursor);
	}

	public String getTTX_LanguageCode(int index) {
		return mDxbTeletext.getTTX_LanguageCode(index);
	}

	public boolean isPlayTTX() {
		return mDxbTeletext.isPlayTTX();
	}

	public void setTeletext_UpdatePage(int pageNumber) {
		mDxbTeletext.setTeletext_UpdatePage(pageNumber);
	}

	public void playTeletext() {
		mDxbTeletext.startTeletext();
	}

	public void stopTeletext() {
		mDxbTeletext.stopTeletext();
	}

	public int getTTX_initPage() {
		return mDxbTeletext.getTTX_initPage();
	}

	public void onTeletextDataUpdate(DVBTPlayer player, Bitmap bitmap) {
		onTeletextDataUpdate(bitmap);
	}

	public void resetTeletextInformation()
	{
		mDxbTeletext.resetTeletextInformation();
	}

/*****************************************************************************************************************************************************************************
 *	Channel
 *****************************************************************************************************************************************************************************/
	public void setChannel(int channel_group, int channel_number, int channel_index, int audio, int subtitle) {
		mStartTime = SystemClock.uptimeMillis();
		mDxbChannel.setChannel(channel_group, channel_number, channel_index, audio, subtitle);
	}

	public void setChannelCancel() {
	}

	public void resetChannelPosition() {
		mDxbChannel.reset();
	}

	public void setFrequency(int channel_group, int channel_number) { // this sets only frequency without pid filter.
		mDxbChannel.setFrequency(channel_group, channel_number);
	}

	public RFInfoData getRFInformation(){
		return mDxbChannel.getRFInformation();
	}

	public void onSetChannelCompletion(DVBTPlayer player) {
		DxbLog(I, "onSetChannelCompletion(), +" + (SystemClock.uptimeMillis() - mStartTime) + " milliseconds");
		onSetChannelCompletion();
	}

/*****************************************************************************************************************************************************************************
 *	PVR
 *****************************************************************************************************************************************************************************/
	public LOCAL_STATE getLocalPlayState() {
		return mDxbPVR.getPlayState();
	}

	public REC_STATE getRecordState() {
		return mDxbPVR.getRecordState();
	}

	public int setCapture(String name) {
		return mDxbPVR.setCapture(name);
	}

	public boolean setRecord(String name) {
		return mDxbPVR.setRecord(name);
	}

	protected boolean setPlay(String path) {
		return mDxbPVR.setPlay(path);
	}

	protected void setPlayResume() {
		mDxbPVR.resume();
	}

	protected void setPlayPause() {
		mDxbPVR.pause();
	}

	public void setRecStop() {
		mDxbPVR.setRecStop();
	}

	public void setLocalPlayStop() {
		mDxbPVR.setPlayStop();
	}

	public void setSeekTo(int seekTime) {
		mDxbPVR.setSeekTo(seekTime);
	}

	public int setPlaySpeed(int iSpeed) {
		return mDxbPVR.setSpeed(iSpeed);
	}

	public int getDuration() {
		return mDxbPVR.getDuration();
	}

	public int getCurrentPosition() {
		return mDxbPVR.getCurrentPosition();
	}

	public int getCurrentReadPosition() {
		return mDxbPVR.getCurrentReadPosition();
	}

	public int getCurrentWritePosition() {
		return mDxbPVR.getCurrentWritePosition();
	}

	public String getDiskFSType() {
		return mDxbPVR.getDiskFSType();
	}

	public void onPlayingCompletion(DVBTPlayer player, int type, int state) {
		onPlayingCompletion(type, state);
	}

	public void onFilePlayTimeUpdate(DVBTPlayer player, int time) {
		onFilePlayTimeUpdate(time);
	}

	public void onRecordingCompletion(DVBTPlayer player, int result) {
		onRecordingCompletion(result);
	}

/*****************************************************************************************************************************************************************************
 *	Search
 *****************************************************************************************************************************************************************************/
	public boolean search(TuneSpace tuneSpace) {
		return mDxbScan.AllChannelScan(tuneSpace);
	}

	public boolean search(TuneSpace tuneSpace, int[] index_list) {
		return mDxbScan.MultiChannelScan(tuneSpace, index_list);
	}

	public boolean search(int KHz_frequency, int Mhz_bandwidth) {
		return mDxbScan.OneChannelScan(KHz_frequency, Mhz_bandwidth);
	}

	public boolean blindscan(TuneSpace tuneSpace) {
		return mDxbScan.BlindScan(tuneSpace);
	}

	public int getSearchFreqKhz() {
		return mDxbScan.getSearchFreqKHz();
	}

	public void searchCancel() {
		mDxbScan.Cancel();
	}

	public void onSearchPercentUpdate(DVBTPlayer player, int nPercent) {
		onSearchPercentUpdate(nPercent);
	}

	public void onSearchCompletion(DVBTPlayer player) {
		onSearchCompletion();
	}

	public void onBlindScanUpdate(DVBTPlayer player, int nPercent, int nIndex, int freqMHz, int symbolKHz) {
		onBlindScanUpdate(nPercent, nIndex, freqMHz, symbolKHz);
	}

	public void onBlindScanCompletion(DVBTPlayer player) {
		onBlindScanCompletion();
	}

/*****************************************************************************************************************************************************************************
 *	DiSEqC
 *****************************************************************************************************************************************************************************/
	public void ExecuteDiSEqC(Tuner tune, Dish dish) {
		mDxbDiSEqC.Execute(tune, dish);
	}

	public void moveMotorUSALS(int sat_long) { // longitude * 10
		mDxbDiSEqC.ExecuteMotorUSALS(sat_long);
	}

	public void moveMotorWest(int steps) {
		mDxbDiSEqC.ExecuteMotorDriveWest(steps);
	}

	public void moveMotorEast(int steps) {
		mDxbDiSEqC.ExecuteMotorDriveEast(steps);
	}

	public void moveMotorStop() {
		mDxbDiSEqC.ExecuteMotorDriveStop();
	}

	public void storeMotorPosition(int index) {
		mDxbDiSEqC.ExecuteMotorStoreNN(index);
	}

	public void moveMotorStore(int index) {
		mDxbDiSEqC.ExecuteMotorGotoNN(index);
	}

/*****************************************************************************************************************************************************************************
 *	Device Access
 *****************************************************************************************************************************************************************************/
	protected void prepare(String db)
	{
		DxbLog(I, "prepare(" + db + ")");
		
		mStartTime = SystemClock.uptimeMillis();
		synchronized(this) {
			if(mPlayer != null) {
				DxbLog(E, "[prepare] mPlayer is not null");
				return;
			}
			mPlayer = new DVBTPlayer();
			mPlayer.setOnPreparedListener(this);
			mPlayer.setOnVideoOutputListener(this);
			mPlayer.setOnErrorListener(this);
			mPlayer.setOnVideoDefinitionUpdateListener(this);
			mPlayer.setOnDBInfoUpdateListener(this);
			mPlayer.setOnDebugModeListener(this);
			mPlayer.setOnDBInformationUpdateListener(this);
			mPlayer.setOnScrambledStatusListener(this);
			mPlayer.prepare(db);
		}
	}

	public void release() {
		DxbLog(I, "release()");
		onReleased();
		synchronized(this) {
			if(mPlayer == null) {
				DxbLog(E, "[release] mPlayer is null");
				return;
			}
			mPlayer.release();
			mPlayer = null;
		}
	}

	protected void start() {
		DxbLog(I, "start()");
		synchronized(this) {
			if(mPlayer == null) {
				DxbLog(E, "[start] mPlayer is null");
				return;
			}
			mPlayer.start(0);
		}
	}

	public void stop() {
		DxbLog(I, "stop()");
		synchronized(this) {
			if(mPlayer == null) {
				DxbLog(E, "[stop] mPlayer is null");
				return;
			}
			mPlayer.stop();
		}
		mDxbChannel.reset();
	}

	protected void pause() {
		DxbLog(I, "pause()");
		synchronized(this) {
			if(mPlayer == null) {
				DxbLog(E, "[pause] mPlayer is null");
				return;
			}
			mPlayer.setPlayPause(false);
		}
	}

	protected void resume() {
		DxbLog(I, "resume(");
		synchronized(this) {
			if(mPlayer == null) {
				DxbLog(E, "[resume] mPlayer is null");
				return;
			}
			mPlayer.setPlayPause(true);
		}
	}

	public void setSurface() {
		DxbLog(I, "setSurface()");
		synchronized(this) {
			if(mPlayer == null) {
				DxbLog(E, "[setSurface] mPlayer is null");
				return;
			}
			mPlayer.setDisplay(mSurfaceHolder);
			mPlayer.setSurface();
		}
	}

	public void releaseSurface() {
		DxbLog(I, "releaseSurface()");
		synchronized(this) {
			if(mPlayer == null) {
				DxbLog(E, "[releaseSurface] mPlayer is null");
				return;
			}
			mPlayer.releaseSurface();
		}
	}

	public void useSurface(int arg) {
		DxbLog(I, "useSurface(arg = " + arg + ")");
		synchronized(this) {
			if(mPlayer == null) {
				DxbLog(E, "[useSurface] mPlayer is null");
				return;
			}
			mPlayer.useSurface(arg);
		}
	}

	public void goToSleep() {
		DxbLog(I, "goToSleep()");
		synchronized(this) {
			if(mPlayer == null) {
				DxbLog(E, "[goToSleep] mPlayer is null");
				return;
			}
			mPlayer.goToSleep();
		}
	}
	
	public void setTuneSpace(TuneSpace space)
	{
		DxbLog(I, "setTuneSpace(" + space + ")");
		synchronized(this) {
			if(mPlayer == null) {
				DxbLog(E, "[setTuneSpace] mPlayer is null");
				return;
			}
			mPlayer.setTuneSpace(space);
		}
	}
	
	public void start(int country_code)
	{
		DxbLog(I, "start(" + country_code + ")");
		synchronized(this) {
			if(mPlayer == null) {
				DxbLog(E, "[start] mPlayer is null");
				return;
			}
			mPlayer.start(country_code);
		}
	}

	public void setAntennaPower(int iIndex) {
		DxbLog(I, "setAntennaPower(" + iIndex + ")");
		synchronized(this) {
			if(mPlayer == null) {
				DxbLog(E, "[setAntennaPower] mPlayer is null");
				return;
			}
			mPlayer.setAntennaPower(iIndex);
		}
	}

	public void requestEPGUpdate(int arg) {
		DxbLog(I, "requestEPGUpdate(" + arg + ")");
		synchronized(this) {
			if(mPlayer == null) {
				DxbLog(E, "[requestEPGUpdate] mPlayer is null");
				return;
			}
			mPlayer.requestEPGUpdate(arg);
		}
	}

	public void setAudio(int iIndex) {
		DxbLog(I, "setAudio(" + iIndex + ")");
		synchronized(this) {
			if(mPlayer == null) {
				DxbLog(E, "[setAudio] mPlayer is null");
				return;
			}
			mPlayer.setAudio(iIndex);
		}
	}

	public void setStereo(int iMode) {
		DxbLog(I, "setStereo(" + iMode + ")");
		synchronized(this) {
			if(mPlayer == null) {
				DxbLog(E, "[setStereo] mPlayer is null");
				return;
			}
			mPlayer.setStereo(iMode);
		}
	}

	public void setVolume(int audioID, int audioVolume) {
		DxbLog(I, "setVolume(" + audioID + ", " + audioVolume + ")");
		synchronized(this) {
			if(mPlayer == null) {
				DxbLog(E, "[setVolume] mPlayer is null");
				return;
			}
			mPlayer.setVolume(audioID, audioVolume);
		}
	}

	public void onOutputAudioDescription(boolean isOnAudioDescription) {
		DxbLog(I, "onOutputAudioDescription(" + isOnAudioDescription + ")");
		synchronized(this) {
			if(mPlayer == null) {
				DxbLog(E, "[onOutputAudioDescription] mPlayer is null");
				return;
			}
			mPlayer.onOutputAudioDescription(isOnAudioDescription);
		}
	}

	public DateTimeData getCurrentDateTime() {
		DxbLog(D, "getCurrentDateTime()");
		synchronized(this) {
			if(mPlayer == null) {
				DxbLog(E, "[getCurrentDateTime] mPlayer is null");
				return null;
			}
			DateTimeData date_time	= new DateTimeData();
			mPlayer.getCurrentDateTime(date_time);
			return date_time;
		}
	}

	protected DateTimeData getTime() {
		DxbLog(D, "getTime()");
		synchronized(this) {
			if(mPlayer == null) {
				DxbLog(E, "[getTime] mPlayer is null");
				return null;
			}
			DateTimeData date_time = new DateTimeData();
			date_time.iMJD	= 0;
			mPlayer.getCurrentDateTime(date_time);
			return date_time;
		}
	}

	protected SignalState getSignalState() {
//		DxbLog(D, "getSignalState()");
		synchronized(this) {
			if(mPlayer == null) {
				DxbLog(E, "[getSignalState] mPlayer is null");
				return null;
			}
			SignalState state = new SignalState();
			mPlayer.getSignalState(state);
			return state;
		}
	}

	protected void setUserLocalTimeOffset(boolean bAuto, int iOffset) {
		DxbLog(I, "setTimeOffset()");
		synchronized(this) {
			if(mPlayer == null) {
				DxbLog(E, "[setTimeOffset] mPlayer is null");
				return;
			}
			mPlayer.setUserLocalTimeOffset(bAuto, iOffset);
		}
	}

/*****************************************************************************************************************************************************************************
 *	Listener
 *****************************************************************************************************************************************************************************/
	private void onReleased() { // release() -> onRelease(), this is not listener
		DxbLog(I, "onReleased()");
		mDxbDiSEqC.setPlayer(null);
		mDxbSubtitle.setPlayer(null);
		mDxbTeletext.setPlayer(null);
		mDxbScan.setPlayer(null);
		mDxbChannel.setPlayer(null);
		mDxbPVR.setPlayer(null);
	}

	@Override
	public void onPrepared(DVBTPlayer player) {
		DxbLog(I, "onPrepared(), +" + (SystemClock.uptimeMillis() - mStartTime) + " milliseconds");
		mDxbDiSEqC.setPlayer(player);
		mDxbSubtitle.setPlayer(player);
		mDxbTeletext.setPlayer(player);
		mDxbScan.setPlayer(player);
		mDxbChannel.setPlayer(player);
		mDxbPVR.setPlayer(player);
		onPrepared();
	}

	@Override
	public void onVideoDefinitionUpdate(DVBTPlayer player, VideoDefinitionData videoinfo) {
		DxbLog(I, "onVideoDefinitionUpdate(), +" + (SystemClock.uptimeMillis() - mStartTime) + " milliseconds");
		mVideoInfo = videoinfo;
		if (mSurfaceHolder != null) {
			mSurfaceHolder.setFixedSize(mVideoInfo.width, mVideoInfo.height);
		}
		requestLayout();
		onVideoDefinitionUpdate(videoinfo);
	}

	@Override
	public void onDBInfoUpdate(DVBTPlayer player, int fromfile, int dbtype, DB_CHANNEL_Data dbinfo) {
		DxbLog(D, "onDBInfoUpdate()");
		onDBInfoUpdate(fromfile, dbtype, dbinfo);
	}

	@Override
	public void onVideoOutputUpdate(DVBTPlayer player) {
		DxbLog(I, "onVideoOutputUpdate(), +" + (SystemClock.uptimeMillis() - mStartTime) + " milliseconds");
		onVideoOutputUpdate();
	}

	@Override
	public boolean onError(DVBTPlayer player, int what, int extra) {
		DxbLog(E, "onError()");
		onError(what, extra);
		return false;
	}

	@Override
	public void onDebugUpdate(DVBTPlayer player, DebugModeData data) {
		DxbLog(D, "onDebugUpdate()");
		onDebugUpdate(data);
	}

	@Override
	public void onDBInformationUpdate(DVBTPlayer player, int type, int param) {
//		DxbLog(D, "onDBInformationUpdate()");
		onDBInformationUpdate(type, param);
	}

	@Override
	public void onScrambledStatus(DVBTPlayer player, int pid, int scrambled) {
		DxbLog(D, "onScrambled()");
		onScrambledStatus(pid, scrambled);
	}

/*****************************************************************************************************************************************************************************
 *	Handler
 *****************************************************************************************************************************************************************************/
	private static final int SET_SUBTITLE_VISIBILITY = 1;
	private static final int SET_TTX_SUBTITLE_VISIBILITY = 2;

	@SuppressLint("HandlerLeak")
	private Handler mHandler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case SET_SUBTITLE_VISIBILITY:
				if (msg.arg1 == 1)
					mDxbSubtitle.setVisibility(View.VISIBLE);
				else
					mDxbSubtitle.setVisibility(View.GONE);
				break;
			case SET_TTX_SUBTITLE_VISIBILITY:
				if (msg.arg1 == 1)
					mDxbTeletext.setVisibility(View.VISIBLE);
				else
					mDxbTeletext.setVisibility(View.GONE);
				break;
			}
		}
	};

/*****************************************************************************************************************************************************************************
 *	Property
 *****************************************************************************************************************************************************************************/
	private static final int PROP_NAME_MAX = 31;
	private static final int PROP_VALUE_MAX = 91;

	public static String property_get(String key) {
		if (key.length() > PROP_NAME_MAX) {
			throw new IllegalArgumentException("key.length > " + PROP_NAME_MAX);
		}
		return DVBTPlayer.property_get(key);
	}

	public static void property_set(String key, String val) {
		if (key.length() > PROP_NAME_MAX) {
			throw new IllegalArgumentException("key.length > " + PROP_NAME_MAX);
		}
		if (val != null && val.length() > PROP_VALUE_MAX) {
			throw new IllegalArgumentException("val.length > " + PROP_VALUE_MAX);
		}
		DVBTPlayer.property_set(key, val);
	}

/*****************************************************************************************************************************************************************************
 *	ETC
 *****************************************************************************************************************************************************************************/
	public boolean isValid() {
		return (mPlayer != null);
	}

	public boolean isPlaying() {
		return (mSurfaceHolder != null);
	}

	public boolean isSTB()
	{
		return isSTB;
	}

	public void setScreenMode(int screenMode, int l, int t, int r, int b) {
		mScreenMode = screenMode;
		requestLayout();
	}

	public int getStatusbarHeight() {
		return mWindowInsets.height();
	}

	protected VideoDefinitionData getVideoInformation() {
		return mVideoInfo;
	}

/*****************************************************************************************************************************************************************************
 *	abstract
 *****************************************************************************************************************************************************************************/
	// Main Thread Callback
	abstract protected void onCreate();
	abstract protected void onDestroy();
	abstract protected void onPrepared();
	abstract protected void onVideoDefinitionUpdate(VideoDefinitionData videoinfo);
	abstract protected void onDBInfoUpdate(int fromfile, int dbtype, DB_CHANNEL_Data dbinfo);
	abstract protected void onVideoOutputUpdate();
	abstract protected void onError(int what, int extra);
	abstract protected void onDebugUpdate(DebugModeData data);
	abstract protected void onDBInformationUpdate(int type, int param);
	abstract protected void onPlayingCompletion(int type, int state);
	abstract protected void onFilePlayTimeUpdate(int time);
	abstract protected void onRecordingCompletion(int result);
	abstract protected void onSearchPercentUpdate(int nPercent);
	abstract protected void onSearchCompletion();
	abstract protected void onBlindScanUpdate(int nPercent, int nIndex, int freqMHz, int symbolKHz);
	abstract protected void onBlindScanCompletion();
	abstract protected void onTeletextDataUpdate(Bitmap bitmap);
	abstract protected void onScrambledStatus(int pid, int scrambled);

	// Sub Thread Callback (UI Control is impossible)
	abstract protected void onSetChannelCompletion();
	abstract protected void onDishSetup(int index);

	abstract protected TuneSpace getTuneSpace(int group_id);

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
