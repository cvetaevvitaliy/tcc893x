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

package com.telechips.android.dvb.player.dvb;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.Parcel;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.util.Log;

import java.lang.ref.WeakReference;

/**
 * DVBTPlayer class can be used to control playback
 * of DVBT streams.
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
public class DVBTPlayer {

    static {
        System.loadLibrary("dvrdvbt_jni");
    }

    private final static String TAG = "DVBTPlayer";
    private int mNativeContext; // accesed by native methods
    private Surface mSurface; // accessed by native methods
    private SurfaceHolder mSurfaceHolder;
    private EventHandler mEventHandler;
    private PowerManager.WakeLock mWakeLock = null;
    private boolean mScreenOnWhilePlaying;
    private boolean mStayAwake;

    /**
     * Default constructor.
     * <p>When done with the DVBTPlayer, you should call {@link #release()},
     * to free the resources. If not released, too many DVBTPlayer instances may
     * result in an exception.</p>
     */
    public DVBTPlayer() {
        Looper looper;
        if ((looper = Looper.myLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else if ((looper = Looper.getMainLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else {
            mEventHandler = null;
        }

        native_setup(new WeakReference<DVBTPlayer>(this));
    }

    private native final void native_setup(Object weak_this);
    private native final void native_finalize();

    /**
     * Sets the SurfaceHolder to use for displaying the video portion of the media.
     * This call is optional. Not calling it when playing back a video will
     * result i nonly the audio track being played.
     *
     * @param sh the SurfaceHolder to use for video display
     */
    public void setDisplay(SurfaceHolder sh) {
		mSurfaceHolder = sh;
		mSurface = (sh != null) ? sh.getSurface() : null;
		if( mSurface == null)
			Log.e(TAG,"setDisplay : mSurface  is null");
		updateSurfaceScreenOn();
    }

    /**
     * Prepares the player for playback.
     *
     * After setting the display surface, you need to call prepare().
     * @throws IllegalStateException if it is called in an invalid state
     */
    public native void prepare(String dbPath) throws IllegalStateException;

   /**
     * Start playback.
     *
     * @throws IllegalStateException if it is called in an invalid state
     */
    public void start(int country_code) throws IllegalStateException {
        stayAwake(true);
        _start(country_code);
    };
   
    private native void _start(int country_code) throws IllegalStateException;

    /**
     * Stops playback.
     *
     * @throws IllegalStateException if it is called in an invalid state
     */
    public void stop() throws IllegalStateException {
        //stayAwake(false);
        _stop();
    };

    private native void _stop() throws IllegalStateException;

    public void setWakeMode(Context context, int mode) {
        boolean washeld = false;
        if (mWakeLock != null) {
            if (mWakeLock.isHeld()) {
                washeld = true;
                mWakeLock.release();
            }
            mWakeLock = null;
        }

        PowerManager pm = (PowerManager)context.getSystemService(Context.POWER_SERVICE);
        //mWakeLock = pm.newWakeLock(mode|PowerManager.ON_AFTER_RELEASE, DVBTPlayer.class.getName());
        mWakeLock = pm.newWakeLock(mode, context.getClass().getName());
        mWakeLock.setReferenceCounted(false);
        if (washeld) {
            mWakeLock.acquire();
        }
    }

    /**
     * Control whether we should the attached SurfaceHolder to keep the
     * screen on while video playback is occurring.
     *
     * @param screenOn Supply true to keep the screen on, false to allow it
     * to turn off.
     */
    public void setScreenOnWhilePlaying(boolean screenOn) {
        if (mScreenOnWhilePlaying != screenOn) {
            mScreenOnWhilePlaying = screenOn;
            updateSurfaceScreenOn();
        }
    }

    private void stayAwake(boolean awake) {
        if (mWakeLock != null) {
            if (awake && !mWakeLock.isHeld()) {
                mWakeLock.acquire();
            } else if (!awake && mWakeLock.isHeld()) {
                mWakeLock.release();
            }
        }
        mStayAwake = awake;
        updateSurfaceScreenOn();
    }

    private void updateSurfaceScreenOn() {
        if (mSurfaceHolder != null) {
            mSurfaceHolder.setKeepScreenOn(mScreenOnWhilePlaying && mStayAwake);
        }
    }

    public native int setTuneSpace(TuneSpace space) throws IllegalStateException;

	 /**
     * Returns current Singal Strenth
     */
	public native int getSignalStrength() throws IllegalStateException;

	public native int getSignalState(SignalState state) throws IllegalStateException;

	/**
	 * Control the power of antenna
	 *
	 * @parem arg : 0 - off, 1 - on, 2 - auto
	 */
	public native void setAntennaPower(int arg) throws IllegalStateException;

	/**
	 * [DVBSx] Control lnb voltage
	 *
	 * @parem arg Voltage of lnb(0: 13v, 1: 18v, 2: off)
	 */
	public native int SetLnbVoltage(int arg) throws IllegalStateException;

	/**
	 * [DVBSx] Control tone
	 *
	 * @parem arg Set to 1 if tone is enable
	 */
	public native int SetTone(int arg) throws IllegalStateException;

	/**
	 * [DVBSx] Control mini diseqc
	 *
	 * @parem arg Port of miniDiSEqC(0: A, 1: B)
	 */
	public native int DiseqcSendBurst(int arg) throws IllegalStateException;

	/**
	 * [DVBSx] Send diseqc command
	 *
	 * @parem arg DiSEqC Command
	 */
	public native int DiseqcSendCMD(byte[] arg) throws IllegalStateException;

	/**
	 * [DVBSx] Reset Blind Scan
	 *
	 * @return : error state
	 */
	public native int BlindScanReset() throws IllegalStateException;

	/**
	 * [DVBSx] Start Blind Scan
	 *
	 * @return : error state
	 */
	public native int BlindScanStart() throws IllegalStateException;

	/**
	 * [DVBSx] Cancel Blind Scan
	 *
	 * @return : error state
	 */
	public native int BlindScanCancel() throws IllegalStateException;

	/**
	 * [DVBSx] Get State of Blind Scan
	 *
	 * @return : 0 = wait, 1 = OK
	 */
	public native int BlindScanGetState() throws IllegalStateException;

	/**
	 * [DVBSx] Get Information of Blind Scan
	 */
	public native Parcel BlindScanGetInfo() throws IllegalStateException;

    public void release() {
        stayAwake(false);
        updateSurfaceScreenOn();
        mOnPreparedListener = null;
        //mOnCompletionListener = null;
        mOnErrorListener = null;
        _release();
    }

    private native void _release() throws IllegalStateException;

    /**
     * Search channels.
     *
     * @param countryCode
     */
    public native void search(int countryCode) throws IllegalStateException;
    /**
     * Manual search channels.
     *
     * @param frequencyKhz frequency to search : unit Khz
     */
    public native void manualSearch(int frequencyKhz, int bandwidth) throws IllegalStateException;
	/**
     * Stop search channels.
     */
	public native void searchCancel() throws IllegalStateException;

    /**
     * Release Surface.
     */
	public native void releaseSurface() throws IllegalStateException;
	
	/**
     * Set Surface.
     */
	public native void setSurface() throws IllegalStateException;
	
    /**
     * use previous registred surface, oppsite is releaseSurface
     *
     * @param arg arguments(currently doesn't use)
     */
    public native int useSurface(int arg) throws IllegalStateException;

    /**
     * Sets current channel.
     *
     * @param channelId channel row id
     * @param audioIndex audio index (value start from 0)
     * @param subtitleIndex subtitle index (value start from 0)
     */
    public native void setChannel(int channelId, int audioIndex, int subtitleIndex) throws IllegalStateException;

    /**
	 * Sets Audio.
	 *
     * @param audioIndex audio index (value start from 0)
	 */
	public native int setAudio(int audioIndex);

    /**
     * Sets Audio Volume.
     *
     * @param audio volume (value start from 0 to 100)
     */
    public native int setVolume(int audioID, int audioVolume);

    /**
     * Sets Audio.
     *
     * @param EnableAudioDescription (on/off)
     */
    public native int onOutputAudioDescription(boolean isOnAudioDescription);

    /**
     * Sets Stereo.
     *
     * @param mode stereo mode (0: Stereo, 1: Left, 2: Right, 3: Mix)
     */
    public native int setStereo(int mode);

    /**
     * Sets country code.
     *
     * @param countryCode country code
     */
    public native void setCountryCode(int countryCode) throws IllegalStateException;

    /**
     * Sets Capture
     *
     * @param Capture
     */
    public native int setCapture(String filePath) throws IllegalStateException;

    /**
     * Sets Record
     *
     * @param Record
     */
    public native int setRecord(String filePath) throws IllegalStateException;

    /**
     * Sets RecStop
     *
     * @param void
     */
	public native int setRecStop() throws IllegalStateException;

    /**
     * Sets Play
     *
     * @param Record
     */
    public native int setPlay(String filePath) throws IllegalStateException;

    /**
     * Sets PlayStop
     *
     * @param void
     */
	public native int setPlayStop() throws IllegalStateException;

    /**
     * Sets SeekTo
     *
     * @param void
     */
	public native int setSeekTo(int seekTime) throws IllegalStateException;

    /**
     * Sets PlaySpeed
     *
     * @param void
     */
	public native int setPlaySpeed(int iSpeed) throws IllegalStateException;

    /**
     * Sets PlaySpeed
     *
     * @param void
     */
	public native void setPlayPause(boolean bPlayPause) throws IllegalStateException;

    /**
     * Sets UserLocalTimeOffset
     *
     * @param void
     */
	public native void setUserLocalTimeOffset(boolean bAutoMode, int iUserLocalTimeOffset) throws IllegalStateException;

    /**
     * Gets Duration
     *
     * @param void
     */
	public native int getDuration() throws IllegalStateException;

    /**
     * Gets CurrentPosition
     *
     * @param void
     */
	public native int getCurrentPosition() throws IllegalStateException;

    /**
     * Gets CurrentReadPosition
     *
     * @param void
     */
	public native int getCurrentReadPosition() throws IllegalStateException;

    /**
     * Gets CurrentWritePosition
     *
     * @param void
     */
	public native int getCurrentWritePosition() throws IllegalStateException;

    /**
     * Gets File system
     *
     * @param void
     */
	public native String getDiskFSType() throws IllegalStateException;

	 /**
     * On/Off teletext play
     *
     * @param onoff 0:Stop Teletext 1:Play Teletext
     */
	public native int playTeletext(int onoff) throws IllegalStateException;

	 /**
	    * set teletext Updatepage
     *
     */
	public native int setTeletext_UpdatePage(int pageNumber) throws IllegalStateException;

	/**
	    * set teletext CachePage_Insert
	    *
	    */
	public native int setTeletext_CachePage_Insert(int pageNumber) throws IllegalStateException;

	/**
	    * set teletext CachePage_Delete
	    *
	    */
	public native int setTeletext_CachePage_Delete(int pageNumber) throws IllegalStateException;

	 /**
     * On/Off subtitle play
     *
     * @param onoff 0:Stop Subtitle 1:Play subtitle
     * @param subtitleIndex subtitle index (value start from 0)
     */
	public native int playSubtitle(int onoff, int subtitleIndex) throws IllegalStateException;

    /**
     * request to update epg data.
     *
     * @param arg if arg==-1, whole epg request otherwise arg is dayoffset
     */
    public native int requestEPGUpdate(int arg) throws IllegalStateException;

    /**
     * get current date & time.
     */
    public native int getCurrentDateTime(DateTimeData date_time) throws IllegalStateException;

    /**
     * go to sleep.
     */
    public native int goToSleep() throws IllegalStateException;

    /**
     * get rf information
     */
    public native Parcel getRFInformation() throws IllegalStateException;

    /**
     * set frequency.
     */
    public native int setFrequency(int channel) throws IllegalStateException;

    /**
     * property_get.
     */
    public static native String property_get(String key) throws IllegalStateException;

    /**
     * property_set.
     */
    public static native void property_set(String key, String val) throws IllegalStateException;

    /**
     * Inteface definition for a callback to be invoked when the player
     * is ready for a playback.
     */
    public interface OnPreparedListener {
        /**
         * Called when the player is ready for a playback.
         *
         * @param player the DVBTPlayer that is ready for playback
         */
        void onPrepared(DVBTPlayer player); 
    }

    private OnPreparedListener mOnPreparedListener;

    /**
     * Register a callback to be invoked when the player is ready
     * for playback.
     *
     * @param listener the callback that will be run
     */
    public void setOnPreparedListener(OnPreparedListener listener)
    {
        mOnPreparedListener = listener;
    }

    /**
     * Interface definitions for a callback to be invoked when channel search
     * has completed.
     */
    public interface OnSearchCompletionListener {
        /**
         * Called when the end of channel search is reached during search.
         */
        void onSearchCompletion(DVBTPlayer player);
    }

    /**
     * Register a callback to be invoked when the end of channel search
     * is reached during search.
     *
     * @param listener the callback that will be run
     */
    public void setOnSearchCompletionListener(OnSearchCompletionListener listener) {
        mOnSearchCompletionListener = listener;
    }

    private OnSearchCompletionListener mOnSearchCompletionListener;

    /**
     * Interface definitions for a callback to be invoked when video output
     * has started.
     */
    public interface OnVideoOutputListener {
        /**
         * Called when the start of video output
         */
        void onVideoOutputUpdate(DVBTPlayer player);
    }

    /**
     * Register a callback to be invoked when the end of channel search
     * is reached during search.
     *
     * @param listener the callback that will be run
     */
    public void setOnVideoOutputListener(OnVideoOutputListener listener) {
        mOnVideoOutputListener = listener;
    }

    private OnVideoOutputListener mOnVideoOutputListener;

    /**
     * Interface definitions for a callback to be invoked when recording
     * has stoped.
     */
    public interface OnRecordingCompletionListener {
        /**
         * Called when the stop of recording
         */
        void onRecordingCompletion(DVBTPlayer player, int result);
    }

    /**
     * Register a callback to be invoked when the end of channel search
     * is reached during search.
     *
     * @param listener the callback that will be run
     */
    public void setOnRecordingCompletionListener(OnRecordingCompletionListener listener) {
        mOnRecordingCompletionListener = listener;
    }

    private OnRecordingCompletionListener mOnRecordingCompletionListener;

    /**
     * Interface definitions for a callback to be invoked when channel tunning
     * has completed.
     */
    public interface OnTunningCompletionListener {
        /**
         * Called when the end of channel search is reached during tunning.
         */
        void onTunningCompletion(DVBTPlayer player, int bStatus);
    }

    /**
     * Register a callback to be invoked when the end of channel tunning
     * is reached during tunning.
     *
     * @param listener the callback that will be run
     */
    public void setOnTunningCompletionListener(OnTunningCompletionListener listener) {
        mOnTunningCompletionListener = listener;
    }

    private OnTunningCompletionListener mOnTunningCompletionListener;

    public interface OnSearchPercentListener {
        /**
         * Called when the end of channel search is reached during tunning.
         */
        void onSearchPercentUpdate(DVBTPlayer player, int nPercent);
    }

    public void setOnSearchPercentListener(OnSearchPercentListener listener) {
        mOnSearchPercentListener = listener;
    }
	private OnSearchPercentListener mOnSearchPercentListener;


    /**
     * Interface definitions for a callback to be invoked when
     * update teletextdata information.
     */
    public interface OnTeletextDataUpdateListener {
        /**
         * Called when the teletextdata information updated.
         */
        void onTeletextDataUpdate(DVBTPlayer player, TeletextData channel);
    }

    /**
     * Register a callback to be invoked when searched teletextdata.
     */
    public void setOnTeletextDataUpdateListener(OnTeletextDataUpdateListener listener) {
        mOnTeletextDataUpdateListener = listener;
    }

    private OnTeletextDataUpdateListener mOnTeletextDataUpdateListener;


    /**
     * Interface definitions for a callback to be invoked when
     * update subtitle information.
     */
    public interface OnSubtitleUpdateListener {
        /**
         * Called when the teletextdata information updated.
         */
        void onSubtitleUpdate(DVBTPlayer player, SubtitleData subt);
    }

    /**
     * Register a callback to be invoked when searched subtitle.
     */
    public void setOnSubtitleUpdateListener(OnSubtitleUpdateListener listener) {
        mOnSubtitleUpdateListener = listener;
    }

    private OnSubtitleUpdateListener mOnSubtitleUpdateListener;

    /**
     * Interface definitions for a callback to be invoked when
     * update file play information.
     */
    public interface OnFilePlayTimeUpdateListener {
        /**
         * Called when the teletextdata information updated.
         */
        void onFilePlayTimeUpdate(DVBTPlayer player, int time);
    }

    /**
     * Register a callback to be invoked when searched subtitle.
     */
    public void setOnFilePlayTimeUpdateListener(OnFilePlayTimeUpdateListener listener) {
        mOnFilePlayTimeUpdateListener = listener;
    }

    private OnFilePlayTimeUpdateListener mOnFilePlayTimeUpdateListener;

    /**
     * Interface definitions for a callback to be invoked when
     * update file play complete.
     */
    public interface OnPlayingCompletionListener {
        /**
         * Called when the teletextdata information updated.
         */
        void onPlayingCompletion(DVBTPlayer player, int type, int state);
    }

    /**
     * Register a callback to be invoked when searched subtitle.
     */
    public void setOnPlayingCompletionListener(OnPlayingCompletionListener listener) {
        mOnPlayingCompletionListener = listener;
    }

    private OnPlayingCompletionListener mOnPlayingCompletionListener;

    /**
     * Interface definitions for a callback to be invoked when
     * update video information.
     */
    public interface OnVideoDefinitionUpdateListener {
        /**
         * Called when the video data information updated.
         */
        void onVideoDefinitionUpdate(DVBTPlayer player, VideoDefinitionData videoinfo);
    }

    /**
     * Register a callback to be invoked when searched video definition.
     */
    public void setOnVideoDefinitionUpdateListener(OnVideoDefinitionUpdateListener listener) {
        mOnVideoDefinitionUpdateListener = listener;
    }

    private OnVideoDefinitionUpdateListener mOnVideoDefinitionUpdateListener;

    /**
     * Interface definitions for a callback to be invoked when
     * update db information.
     */
    public interface OnDBInfoUpdateListener {
        /**
         * Called when the db information updated.
         */
        void onDBInfoUpdate(DVBTPlayer player, int fromfile, int dbtype, DB_CHANNEL_Data dbinfo);
    }

    /**
     * Register a callback to be invoked when searched db information.
     */
    public void setOnDBInfoUpdateListener(OnDBInfoUpdateListener listener) {
        mOnDBInfoUpdateListener = listener;
    }

    private OnDBInfoUpdateListener mOnDBInfoUpdateListener;

    /**
     * Interface definitions for a callback to be invoked when
     * update db information.
     */
    public interface OnDBInformationUpdateListener {
        /**
         * Called when the db information updated.
         */
        void onDBInformationUpdate(DVBTPlayer player, int type, int param);
    }

    /**
     * Register a callback to be invoked when updated db information.
     */
    public void setOnDBInformationUpdateListener(OnDBInformationUpdateListener listener) {
        mOnDBInformationUpdateListener = listener;
    }

	private OnDBInformationUpdateListener mOnDBInformationUpdateListener;


	/**
	  * Interface definitions for a callback to be invoked when
	  * update video information.
	  */
	public interface OnDebugModeListener
	{
		void onDebugUpdate(DVBTPlayer player, DebugModeData data);
	}
     
	/**
	  * Register a callback to be invoked when searched debugmode.
	  */
	public void setOnDebugModeListener(OnDebugModeListener listener)
	{
		mOnDebugModeListener	= listener;
	}

	private OnDebugModeListener	mOnDebugModeListener;

    /**
     * Interface definitions of a callback to be invoked when there
     * has been an error during an asynchronous operation (other errors
     * will throw exceptions at method call time).
     */
    public interface OnErrorListener {
        /**
         * Called to indicate an error.
         *
         * @param player the DVBTPlay the error pertains to
         * @param what the type of error that has occurred
         * @param extra an extra code, specific to the error.
         * @return True if the method handled the error, false if it didn't.
         * Returning false, or not having an OnErrorListener at all, will
         * cause the OnCompletionListener to be called.
         */
        boolean onError(DVBTPlayer player, int what, int extra);
    }

    /**
     * Register a callback to be invoked when an error has happened
     * during an asynchronous operation.
     *
     * @param listener the callback that will be run
     */
    public void setOnErrorListener(OnErrorListener listener) {
        mOnErrorListener = listener;
    }

	private OnErrorListener mOnErrorListener;

    public interface OnScrambledStatusListener {
        /**
         * Called when the channel scrambled, notify send.
         */
    	void onScrambledStatus(DVBTPlayer player, int pid, int scrambled);
    }

    public void setOnScrambledStatusListener(OnScrambledStatusListener listener) {
    	mOnScrambledStatusListener = listener;
    }

    private OnScrambledStatusListener mOnScrambledStatusListener;

	private static final int EVENT_NOP                                      = 0;
	private static final int EVENT_PREPARED                                 = 1;
	private static final int EVENT_SEARCH_COMPLETE                          = 2;
	private static final int EVENT_TUNNING_COMPLETE                         = 3;
	private static final int EVENT_SEARCH_PERCENT                           = 4;
	private static final int EVENT_VIDEO_OUTPUT                             = 5;
	private static final int EVENT_RECORDING_COMPLETE                       = 6;
	private static final int EVENT_SUBTITLE_UPDATE                          = 7;
	private static final int EVENT_VIDEODEFINITION_UPDATE                   = 8;
	private static final int EVENT_DEBUGMODE_UPDATE                         = 9;
	private static final int EVENT_PLAYING_COMPLETE                         = 10;
	private static final int EVENT_PLAYING_CURRENT_TIME                     = 11;
	private static final int EVENT_DBINFO_UPDATE                            = 12;
	private static final int EVENT_DBINFORMATION_UPDATE                     = 13;
	private static final int EVENT_SCRAMBLED_DONE                           = 14;
	private static final int EVENT_TELETEXT_UPDATE                          = 15;

	private static final int EVENT_ERROR                                    = 20;

	private class EventHandler extends Handler
	{
		private DVBTPlayer mDVBTPlayer;

		public EventHandler(DVBTPlayer player, Looper looper)
		{
			super(looper);
			mDVBTPlayer = player;
		}

		@Override
		public void handleMessage(Message msg)
		{
			if (mDVBTPlayer.mNativeContext == 0)
			{
				Log.w(TAG, "player went away with unhandled ");
				return;
			}

			switch (msg.what)
			{
				case EVENT_NOP:
					return;

				case EVENT_PREPARED:
					if (mOnPreparedListener != null)
						mOnPreparedListener.onPrepared(mDVBTPlayer);
					return;

				case EVENT_SEARCH_COMPLETE:
					if (mOnSearchCompletionListener != null)
						mOnSearchCompletionListener.onSearchCompletion(mDVBTPlayer);
					return;

				case EVENT_TUNNING_COMPLETE:
					if (mOnTunningCompletionListener != null)
						mOnTunningCompletionListener.onTunningCompletion(mDVBTPlayer, msg.arg2);
					return;

				case EVENT_SEARCH_PERCENT:
					if (mOnSearchPercentListener != null)
						mOnSearchPercentListener.onSearchPercentUpdate(mDVBTPlayer, msg.arg2);
					return;

				case EVENT_VIDEO_OUTPUT:
					if (mOnVideoOutputListener != null)
						mOnVideoOutputListener.onVideoOutputUpdate(mDVBTPlayer);
					return;

				case EVENT_RECORDING_COMPLETE:
					if (mOnRecordingCompletionListener != null)
						mOnRecordingCompletionListener.onRecordingCompletion(mDVBTPlayer, msg.arg1);
					return;

				case EVENT_SUBTITLE_UPDATE:
					if (mOnSubtitleUpdateListener == null)
						return;
					if (msg.obj == null) {
						return;
					} else {
						if (msg.obj instanceof Parcel) {
							Parcel parcel = (Parcel)msg.obj;
							SubtitleData data = new SubtitleData(parcel);
							parcel.recycle();
							mOnSubtitleUpdateListener.onSubtitleUpdate(mDVBTPlayer, data);
						}
					}
					return;

				case EVENT_VIDEODEFINITION_UPDATE:
					if (mOnVideoDefinitionUpdateListener == null)
						return;
					if (msg.obj == null) {
						return;
					} else {
						if (msg.obj instanceof Parcel) {
							Parcel parcel = (Parcel)msg.obj;
							VideoDefinitionData data = new VideoDefinitionData(parcel);
							parcel.recycle();
							mOnVideoDefinitionUpdateListener.onVideoDefinitionUpdate(mDVBTPlayer, data);
						}
					}
					return;

				case EVENT_DBINFO_UPDATE:
					if (mOnDBInfoUpdateListener == null)
						return;
					if (msg.obj == null) {
						return;
					} else {
						if (msg.obj instanceof Parcel) {
							Parcel parcel = (Parcel)msg.obj;
							DB_CHANNEL_Data data = new DB_CHANNEL_Data(parcel);
							parcel.recycle();
							mOnDBInfoUpdateListener.onDBInfoUpdate(mDVBTPlayer, msg.arg1, msg.arg2, data);
						}
					}
					return;

				case EVENT_DBINFORMATION_UPDATE:
					if (mOnDBInformationUpdateListener != null)
						mOnDBInformationUpdateListener.onDBInformationUpdate(mDVBTPlayer, msg.arg1, msg.arg2);
					return;

				case EVENT_PLAYING_COMPLETE:
					if (mOnPlayingCompletionListener != null)
						mOnPlayingCompletionListener.onPlayingCompletion(mDVBTPlayer, msg.arg1, msg.arg2);
					return;

				case EVENT_PLAYING_CURRENT_TIME:
					if (mOnFilePlayTimeUpdateListener != null) {
						mOnFilePlayTimeUpdateListener.onFilePlayTimeUpdate(mDVBTPlayer, msg.arg1);
					}
					return;
					
				case EVENT_TELETEXT_UPDATE:
					if (mOnTeletextDataUpdateListener == null)
						return;
					if (msg.obj == null) {
						return;
					} else {
						if (msg.obj instanceof Parcel) {
							Parcel parcel = (Parcel)msg.obj;
							TeletextData data = new TeletextData(parcel);
							parcel.recycle();
							mOnTeletextDataUpdateListener.onTeletextDataUpdate(mDVBTPlayer, data);
						}
					}
					return;

				case EVENT_DEBUGMODE_UPDATE:
					Log.e(TAG, "EVENT_DEBUGMODE_UPDATE:");
					if (mOnDebugModeListener == null)
						return;
					if (msg.obj == null) {
						return;
					} else {
						if (msg.obj instanceof Parcel) {
							Parcel parcel = (Parcel)msg.obj;
							DebugModeData data = new DebugModeData(parcel);
							parcel.recycle();
							mOnDebugModeListener.onDebugUpdate(mDVBTPlayer, data);
						}
					}
					return;

				case EVENT_SCRAMBLED_DONE:
					Log.e(TAG, "EVENT_SCRAMBLED_DONE:");
					if(mOnScrambledStatusListener != null)
					{
						mOnScrambledStatusListener.onScrambledStatus(mDVBTPlayer, msg.arg1, msg.arg2);
					}
					return;
					
				case EVENT_ERROR:
					if (mOnErrorListener != null)
						mOnErrorListener.onError(mDVBTPlayer, msg.what, msg.arg1);
					return;

				default:
					Log.e(TAG, "Unknown message type " + msg.what);
					return;
			}
		}
	}

	private static void postEventFromNative(Object player_ref, int what, int arg1, int arg2, Object obj)
	{
		DVBTPlayer player = (DVBTPlayer) ((WeakReference) player_ref).get();
		
		if (player == null)
		{
			return;
		}

		if (player.mEventHandler != null)
		{
			Message m = player.mEventHandler.obtainMessage(what, arg1, arg2, obj);
			player.mEventHandler.sendMessage(m);
		}
	}
}
