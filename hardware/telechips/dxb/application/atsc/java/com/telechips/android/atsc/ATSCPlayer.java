/*
 * Copyright (C) 2009 Telechips, Inc.
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

package com.telechips.android.atsc;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.util.Log;

import java.io.IOException;

import java.lang.ref.WeakReference;

/**
 * ATSCPlayer class can be used to control playback
 * of ATSC streams.
 *
 * <p>Topics covered here are:
 * <ol>
 * <li><a href="#BlockDiagram">Block Diagram</a>
 * <li><a href="#Permissions">Permissions</a>
 * </ol>
 *
 * <a name="BlockDiagram">
 * <h3>Block Diagram</h3>
 * TODO: insert description
 *
 * <a name="Permissions"></a>
 * <h3>Permissions</h3>
 *
 * One may need to declare a corresponding WAKE_LOCK permission {@link
 * android.R.styleable#AndroidManifestUsesPermission &lt;uses-permission&gt;}
 * element.
 */
public class ATSCPlayer {
	static {
		System.loadLibrary("atsc_jni");
	}

	private static final String TAG = "ATSCPlayer";

	private static final int EVENT_NOP 				= 0;
	private static final int EVENT_PREPARE 				= 1;
	private static final int EVENT_SEARCH_COMPLETE 		= 2;
	private static final int EVENT_SEARCH_PERCENT 		= 3;
	private static final int EVENT_SEARCH_DONEINFO 		= 4;
	private static final int EVENT_VIDEO_OUTPUT 		= 5;
	private static final int EVENT_RECORDING_COMPLETE 	= 6;
	private static final int EVENT_SIGNAL_STATUS 		= 7;
	private static final int EVENT_EPGUPDATE_DONE 		= 8;
	private static final int EVENT_SCRAMBLED_DONE 		= 9;		
	private static final int EVENT_TIME_UPDATE			= 10;
	private static final int EVENT_SUBTITLE_UPDATE 		= 11;
	private static final int EVENT_ERROR 				= 12;

	private int mNativeContext; 			// accesed by native methods
	private int mListenerContext; 			// accessed by native methods

	private Surface mSurface; 							// accessed by native methods
	private SurfaceHolder mSurfaceHolder;
	private EventHandler mEventHandler;

	private PowerManager.WakeLock mWakeLock = null;
	private boolean mScreenOnWhilePlaying;
	private boolean mStayAwake;

	// Listener
	private OnPreparedListener mOnPreparedListener;
	private OnSearchCompletionListener mOnSearchCompletionListener;
	private OnSearchPercentListener mOnSearchPercentListener;
	private OnSearchDoneInfoListener mOnSearchDoneInfoListener;
	private OnClosedCaptionUpdateListener mOnClosedCaptionUpdateListener;
	private OnVideoOutputListener mOnVideoOutputListener;
	private OnRecordingCompletionListener mOnRecordingCompletionListener;
	private OnSignalStatusListener mOnSignalStatusListener;
	private OnEpgUpdateDoneListener mOnEpgUpdateDoneListener;	
	private OnScrambledStatusListener mOnScrambledStatusListener;	
	private OnTimeUpdateListener mOnTimeUpdateListener;	
	private OnErrorListener mOnErrorListener;

	public ATSCPlayer() {
		Looper looper;

		if((looper = Looper.myLooper()) != null) {
			mEventHandler = new EventHandler(this, looper);
		}
		else if((looper = Looper.getMainLooper()) != null) {
			mEventHandler = new EventHandler(this, looper);
		}
		else {
			mEventHandler = null;
		}

		native_setup(new WeakReference<ATSCPlayer>(this));
	}

	private native final void native_setup(Object weak_this);
	private native final void native_finalize();

	private native void _start(int country_code) throws IllegalStateException;
	private native void _stop() throws IllegalStateException;
	private native void _release() throws IllegalStateException;

	public native void setSurface() throws IllegalStateException;
	public native void releaseSurface() throws IllegalStateException;
	public native int useSurface(int arg) throws IllegalStateException;

	public native void prepare(String DB_Path) throws IllegalStateException;

	public native void search(int countryCode, int  modulationFlag) throws IllegalStateException;
	public native void manualSearch(int countryCode, int  modulationFlag, int frequencyKhz, int deleteDB) throws IllegalStateException;
	public native void searchCancel() throws IllegalStateException;
	public native void setChannel(int channel) throws IllegalStateException;

	public native int setCapture(String filePath) throws IllegalStateException;
	public native int setRecord(String filePath) throws IllegalStateException;
	public native int setRecStop() throws IllegalStateException;

	public native int getCountryCode() throws IllegalStateException;
	public native void setCountryCode(int countryCode) throws IllegalStateException;

	public native int getVideoWidth() throws IllegalStateException;
	public native int getVideoHeight() throws IllegalStateException;
	
	public native int getSignalStrength() throws IllegalStateException;

	public native boolean isPlaying() throws IllegalStateException;

	public native int setLCDUpdate() throws IllegalStateException;

	public native void setDisplayEnable() throws IllegalStateException;
	public native void setDisplayDisable() throws IllegalStateException;

	public native void setVolume(float leftVolume, float rightVolume) throws IllegalStateException;

	/**
	 * Subtitle On/Off
	 *
	 * @param int
	 */
	public native int playSubtitle(int OnOff);
	public native int enableCC(int EnableDisable);	
	public native int setCCServiceNum(int ServiceNum);	
	public native int setAudioLanguage(int AudioLanguage);		
	public native int setAspectRatio(int AspectRatio);

	private void updateSurfaceScreenOn() {
		if(mSurfaceHolder != null) {
			mSurfaceHolder.setKeepScreenOn(mScreenOnWhilePlaying && mStayAwake);
		}
	}

	private void stayAwake(boolean awake) {
		if(mWakeLock != null) {
			if(awake && !mWakeLock.isHeld()) {
				mWakeLock.acquire();
			}
			else if(!awake && mWakeLock.isHeld()) {
				mWakeLock.release();
			}
		}

		mStayAwake = awake;
		updateSurfaceScreenOn();
	}

	public void setWakeMode(Context context, int mode) {
		boolean washeld = false;
		if(mWakeLock != null) {
			if(mWakeLock.isHeld()) {
				washeld = true;
				mWakeLock.release();
			}
			mWakeLock = null;
		}

		PowerManager pm = (PowerManager)context.getSystemService(Context.POWER_SERVICE);
		//mWakeLock = pm.newWakeLock(mode | PowerManager.ON_AFTER_RELEASE, ATSCPlayer.class.getName());
	        mWakeLock = pm.newWakeLock(mode, context.getClass().getName());
		mWakeLock.setReferenceCounted(false);
		if(washeld) {
			mWakeLock.acquire();
		}
	}

	public void setDisplay(SurfaceHolder sh) {
		mSurfaceHolder = sh;
		mSurface = sh.getSurface();
		if(mSurface == null) {
			Log.e(TAG,"setDisplay : mSurface  is null");
		}		
		updateSurfaceScreenOn();
	}

	public void setScreenOnWhilePlaying(boolean screenOn) {
		if(mScreenOnWhilePlaying != screenOn) {
			mScreenOnWhilePlaying = screenOn;
			updateSurfaceScreenOn();
		}
	}

	public void start(int country_code) throws IllegalStateException {
		stayAwake(true);
		_start(country_code);
	}	

	public void stop() throws IllegalStateException {
		stayAwake(false);
		_stop();
	};
	
	public void release() throws IllegalStateException {
		stayAwake(false);
		updateSurfaceScreenOn();

		mOnPreparedListener 			= null;
		mOnSearchCompletionListener 	= null;
		mOnSearchPercentListener 		= null;
		mOnSearchDoneInfoListener 		= null;
		mOnClosedCaptionUpdateListener = null;
		mOnVideoOutputListener 		= null;
		mOnRecordingCompletionListener 	= null;
		mOnSignalStatusListener 		= null;
		mOnEpgUpdateDoneListener 		= null;
		mOnScrambledStatusListener 		= null;
		mOnTimeUpdateListener	 		= null;
		mOnErrorListener 				= null;

		_release();
	};
	
	public interface OnPreparedListener {
		void onPrepared(ATSCPlayer player); 
	}

	public void setOnPreparedListener(OnPreparedListener listener) {
		mOnPreparedListener = listener;
	}

	public interface OnSearchCompletionListener {
		void onSearchCompletion(ATSCPlayer player);
	}

	public void setOnSearchCompletionListener(OnSearchCompletionListener listener) {
		mOnSearchCompletionListener = listener;
	}

	public interface OnSearchPercentListener {
		void onSearchPercentUpdate(ATSCPlayer player, int nPercent);
	}

	public void setOnSearchPercentListener(OnSearchPercentListener listener) {
		mOnSearchPercentListener = listener;
	}

	public interface OnSearchDoneInfoListener {
		void onSearchDoneInfoUpdate(ATSCPlayer player, int nTVNum, int nRadioNum);
	}

	public void setOnSearchDoneInfoListener(OnSearchDoneInfoListener listener) {
		mOnSearchDoneInfoListener = listener;
	}

	public interface OnClosedCaptionUpdateListener {
		void onClosedCaptionUpdate(ATSCPlayer player, SubtitleData subt);
	}

	public void setOnClosedCaptionUpdateListener(OnClosedCaptionUpdateListener listener) {
		mOnClosedCaptionUpdateListener = listener;
	}

	public interface OnVideoOutputListener {
		void onVideoOutputUpdate(ATSCPlayer player);
	}

	public void setOnVideoOutputListener(OnVideoOutputListener listener) {
		mOnVideoOutputListener = listener;
	}

	public interface OnRecordingCompletionListener {
		void onRecordingCompletion(ATSCPlayer player, int result);
	}

	public void setOnRecordingCompletionListener(OnRecordingCompletionListener listener) {
		mOnRecordingCompletionListener = listener;
	}

	public interface OnSignalStatusListener {
		void onSignalStatusUpdate(ATSCPlayer player, int nLock, int nSignalStrength);
	}

	public void setOnSignalStatusListener(OnSignalStatusListener listener) {
		mOnSignalStatusListener = listener;
	}

	public interface OnEpgUpdateDoneListener {
		void onEpgUpdateDoneUpdate(ATSCPlayer player,int iChannelNumber, int iServiceID);
	}
	
	public void setOnEpgUpdateDoneListener(OnEpgUpdateDoneListener listener) {
		mOnEpgUpdateDoneListener = listener;
	}

	public interface OnScrambledStatusListener {
		void onScrambledStatusUpdate(ATSCPlayer player, int iScrambledState, int iChannelNumber);
	}

	public void setOnScrambledStatusListener(OnScrambledStatusListener listener) {
		mOnScrambledStatusListener = listener;
	}

	public interface OnTimeUpdateListener {
		void onTimeUpdateUpdate(ATSCPlayer player, int iUTCTime);
	}

	public void setOnTimeUpdateListener(OnTimeUpdateListener listener) {
		mOnTimeUpdateListener = listener;
	}

	public interface OnErrorListener {
		boolean onError(ATSCPlayer player, int what, int extra);
	}

	public void setOnErrorListener(OnErrorListener listener) {
		mOnErrorListener = listener;
	}

	// subtitle data object
	private SubtitleData getSubtitleDataObject()
	{
		return new SubtitleData();
	}

	private class EventHandler extends Handler {
		private ATSCPlayer mATSCPlayer;

		public EventHandler(ATSCPlayer player, Looper looper) {
			super(looper);
			mATSCPlayer = player;
		}

		@Override
		public void handleMessage(Message msg) {
			if(mATSCPlayer.mNativeContext == 0) {
				Log.w(TAG, "player went away with unhandled");
				return;
			}

			switch(msg.what) {
				case EVENT_NOP:
					break;
				case EVENT_PREPARE:
					if(mOnPreparedListener != null) {
						mOnPreparedListener.onPrepared(mATSCPlayer);
					}
					break;
				case EVENT_SEARCH_COMPLETE:
					if(mOnSearchCompletionListener != null) {
						mOnSearchCompletionListener.onSearchCompletion(mATSCPlayer);
					}
					break;
				case EVENT_SEARCH_PERCENT:
					if(mOnSearchPercentListener != null) {
						mOnSearchPercentListener.onSearchPercentUpdate(mATSCPlayer, msg.arg1);
					}
					break;
				case EVENT_SEARCH_DONEINFO:
					if(mOnSearchDoneInfoListener != null) {
						mOnSearchDoneInfoListener.onSearchDoneInfoUpdate(mATSCPlayer, msg.arg1, msg.arg2);
					}
					break;
				case EVENT_SUBTITLE_UPDATE:
					if(mOnClosedCaptionUpdateListener != null) {
						mOnClosedCaptionUpdateListener.onClosedCaptionUpdate(mATSCPlayer, (SubtitleData)msg.obj);
					}
					break;
				case EVENT_VIDEO_OUTPUT:
					if(mOnVideoOutputListener != null){
						mOnVideoOutputListener.onVideoOutputUpdate(mATSCPlayer);
					}
					break;
				case EVENT_RECORDING_COMPLETE:
					if(mOnRecordingCompletionListener != null){
						mOnRecordingCompletionListener.onRecordingCompletion(mATSCPlayer, msg.arg1);
					}
					break;
				case EVENT_SIGNAL_STATUS:
					if(mOnSignalStatusListener != null) {
						mOnSignalStatusListener.onSignalStatusUpdate(mATSCPlayer, msg.arg1, msg.arg2);
					}
					break;
				case EVENT_EPGUPDATE_DONE:
					if(mOnEpgUpdateDoneListener != null) {
						mOnEpgUpdateDoneListener.onEpgUpdateDoneUpdate(mATSCPlayer, msg.arg1, msg.arg2);
					}
					break;
				case EVENT_SCRAMBLED_DONE:
					if(mOnScrambledStatusListener != null) {
						mOnScrambledStatusListener.onScrambledStatusUpdate(mATSCPlayer, msg.arg1, msg.arg2);
					}
					break;
				case EVENT_TIME_UPDATE:
					if(mOnTimeUpdateListener != null) {
						mOnTimeUpdateListener.onTimeUpdateUpdate(mATSCPlayer, msg.arg1);
					}
					break;
				case EVENT_ERROR:
					if(mOnErrorListener != null) {
						mOnErrorListener.onError(mATSCPlayer, msg.what, msg.arg1);
					}
					break;			
				default:
					Log.e(TAG, "Unknown message type " + msg.what);
					break;
			}
		}
	}

	private static void postEventFromNative(Object player_ref, int what, int arg1, int arg2, Object obj) {
		ATSCPlayer player = (ATSCPlayer)((WeakReference) player_ref).get();
		if(player == null) {
			Log.w(TAG, "player is null");
			return;
		}
		if(player.mEventHandler == null)
		Log.w(TAG, "mEventHandler is null");
		
		if(player.mEventHandler != null) {
			Message m = player.mEventHandler.obtainMessage(what, arg1, arg2, obj);
			player.mEventHandler.sendMessage(m);
		}
	}
}
