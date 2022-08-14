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

package com.telechips.android.isdbt.player.isdbt;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.util.Log;
import android.graphics.*;

import java.lang.ref.WeakReference;

/**
 * ISDBTPlayer class can be used to control playback
 * of ISDBT streams.
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
public class ISDBTPlayer {

    static {
        System.loadLibrary("isdbt_jni");
    }

	public static final int JAPAN	= 0;
	public static final int BRAZIL	= 1;
	
    private final static String TAG = "ISDBTPlayer";
    private int mNativeContext; // accesed by native methods
    private int mListenerContext; // accessed by native methods
    private Surface mSurface; // accessed by native methods
    private SurfaceHolder mSurfaceHolder;
    private EventHandler mEventHandler;
    private PowerManager.WakeLock mWakeLock = null;
    private boolean mScreenOnWhilePlaying;
    private boolean mStayAwake;

    public int feature_override;
    
    /**
     * Default constructor.
     * <p>When done with the ISDBTPlayer, you should call {@link #release()},
     * to free the resources. If not released, too many ISDBTPlayer instances may
     * result in an exception.</p>
     */
    public ISDBTPlayer() {
        Looper looper;
        if ((looper = Looper.myLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else if ((looper = Looper.getMainLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else {
            mEventHandler = null;
        }

        native_setup(new WeakReference<ISDBTPlayer>(this));
    }

    private native final void native_setup(Object weak_this);
    private native final void native_finalize();
    public static native void drawSubtitle(Bitmap stbitmap) throws IllegalStateException;
    public static native void drawSuperimpose(Bitmap sibitmap) throws IllegalStateException;
    public static native void drawPng(Bitmap pngbitmap) throws IllegalStateException;	


    /**
     * Sets the SurfaceHolder to use for displaying the video portion of the media.
     * This call is optional. Not calling it when playing back a video will
     * result i nonly the audio track being played.
     *
     * @param sh the SurfaceHolder to use for video display
     */
    public void setDisplay(SurfaceHolder sh) {
		mSurfaceHolder = sh;
		mSurface = sh.getSurface();
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
    //public native void prepare(String dbPath) throws IllegalStateException;
    public native void prepare(int specific_info) throws IllegalStateException;

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
        //mWakeLock = pm.newWakeLock(mode|PowerManager.ON_AFTER_RELEASE, ISDBTPlayer.class.getName());
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

    /**
     * Returns current Singal Strenth
     */
    public native void getSignalStrength(SQInfoData sqinfo) throws IllegalStateException;

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
 	 * start Test mode
     *
     */
    public native int startTestMode(int index) throws IllegalStateException;

    /**
     * Search channels.
     *
     * @param scan_type 0=initial scan, 1=rescan, 2=area scan, 4=manual scan, 5=autosearch
     * @param country_code
     * @param area_code
     * @param channel_num
     * @param options it's valid only for manual scan.
     *		1=delete channel DB
     * param  in autosearch
     *   channel_num - current channel number. -1 means no channel was selected.
     *   options - direction. 0=down (forwarding to lower UHF channel no.), 1=up (increasing UHF channel no)
     */
    public native void search(int scan_type, int country_code, int area_code, int channel_num, int options) throws IllegalStateException;
    
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
     * @param mainChannel
     * @param subChannel
     * @param audioIndex
     * @param audioMode
     * @param raw_w
     * @param raw_h
     * @param view_w
     * @param view_h
     * @param ch_index
     */
    public native void setChannel(int mainChannel, int subChannel, int audioIndex, int audioMode, int raw_w, int raw_h, int view_w, int view_h, int ch_index) throws IllegalStateException;

    /**
     * Sets dual channel.
     *
     * @param channelIndex  channel index for dual-decode
    */
    public native void setDualChannel(int channelIndex) throws IllegalStateException;

    /**
     * Sets Audio.
     *
     * @param audioIndex audio index (value start from 0)
     */
    public native int setAudio(int audioIndex) throws IllegalStateException;
    
    /**
     * Sets MultiAudio
     * 
     * @ param index select a playback mode of dual monoral audio
     */
    public native int setAudioDualMono (int index);


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
     * @param filePath
     * @param raw_w
     * @param raw_h
     * @param view_w
     * @param view_h
     */
    public native int setPlay(String filePath, int raw_w, int raw_h, int view_w, int view_h) throws IllegalStateException;

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
     * Gets channel infomation.
     *
     * @param channel channel information to get but -1 is invalid value. 
     */
    public native void getChannelInfo(Channel channel);
	
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
     */
    public native int playSubtitle(int onoff) throws IllegalStateException;

    /**
     * On/Off superimpose play
     *
     * @param onoff 0:Stop Superimpose 1:Play superimpose
     */
    public native int playSuperimpose(int onoff) throws IllegalStateException;

    /**
     * On/Off Png play
     *
     * @param onoff 0:Stop Png 1:Play Png
     */
    public native int playPng(int onoff) throws IllegalStateException;

    /**
     * select subtitle
     *
     * @param index subtitle index (value start from 0)
     */
    public native int setSubtitle(int index) throws IllegalStateException;
    
    /**
     * select superimpose
     *
     * @param index superimpose index (value start from 0)
     */
    public native int setSuperImpose(int index) throws IllegalStateException;

    /**
     * set age of parental rating
     *
     * @param age limite age (value start from 0)
     */
    public native int setParentalRate(int age) throws IllegalStateException;
	
	  /**
     * set area code in japan
     *
     * @param area_code area code in japan
     */
    public native void setArea(int area_code) throws IllegalStateException;

	  /**
     * set preset
     *
     * @param area_code area code in japan
     */
    public native void setPreset(int area_code) throws IllegalStateException;

	  /**
     * On/Off handover
     *
     * @param country_code 
     */
    public native void setHandover(int country_code) throws IllegalStateException;
	
    /**
     * Sets the volume on this player.
     *
     * @param leftVolume left volume scaler
     * @param rightVolume right volume scaler
     */
    public native void setVolume(float leftVolume, float rightVolume) throws IllegalStateException;

    /**
     * Sets frequency band.
     */
	public native void setFreqBand (int freq_band) throws IllegalStateException;

    /**
     * Sets field log.
     */
	public native void setFieldLog (String sPath, int fOn_Off) throws IllegalStateException;


    /**
      * request IC Card info
      *
      * @param arg not defined yet
      */
      public native int reqSCInfo(int arg);
	
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

    public native int CtrlLastFrame_Open() throws IllegalStateException;	
	public native int CtrlLastFrame_Close() throws IllegalStateException;

    /**
     * property_get.
     */
    public static native String property_get(String key) throws IllegalStateException;

    /**
     * property_set.
     */
    public static native void property_set(String key, String val) throws IllegalStateException;

	/*
	  * setPresetMode
	  * return 0 = fail, 1 = success
	  */
	public native int setPresetMode (int preset_mode) throws IllegalStateException;

    /**
     * Inteface definition for a callback to be invoked when the player
     * is ready for a playback.
     */
    public interface OnPreparedListener {
        /**
         * Called when the player is ready for a playback.
         *
         * @param player the ISDBTPlayer that is ready for playback
         */
        void onPrepared(ISDBTPlayer player); 
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
        void onSearchCompletion(ISDBTPlayer player);
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
        void onVideoOutputUpdate(ISDBTPlayer player);
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
        void onRecordingCompletion(ISDBTPlayer player, int result);
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
     * Interface definitions for a callback to be invoked when
     * update channel information.
     */
    public interface OnChannelUpdateListener {
        /**
         * Called when the channel information updated.
         */
        void onChannelUpdate(ISDBTPlayer player, int request);
    }

    /**
     * Register a callback to be invoked when searched channel.
     */
    public void setOnChannelUpdateListener(OnChannelUpdateListener listener) {
        mOnChannelUpdateListener = listener;
    }

    private OnChannelUpdateListener mOnChannelUpdateListener;


    /**
     * Interface definitions for a callback to be invoked when channel tunning
     * has completed.
     */
    public interface OnTunningCompletionListener {
        /**
         * Called when the end of channel search is reached during tunning.
         */
        void onTunningCompletion(ISDBTPlayer player, int bStatus);
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
        void onSearchPercentUpdate(ISDBTPlayer player, int nPercent, int nChannel);
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
        void onTeletextDataUpdate(ISDBTPlayer player, TeletextData channel);
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
        void onSubtitleUpdate(ISDBTPlayer player, SubtitleData subt);
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
     * update superimpose information.
     */
    public interface OnSuperimposeUpdateListener {
        /**
         * Called when the superimposedata information updated.
         */
        void onSuperimposeUpdate(ISDBTPlayer player, SubtitleData superim);
    }

    /**
     * Register a callback to be invoked when searched superimpose.
     */
    public void setOnSuperimposeUpdateListener(OnSuperimposeUpdateListener listener) {
        mOnSuperimposeUpdateListener = listener;
    }

    private OnSuperimposeUpdateListener mOnSuperimposeUpdateListener;


    /**
     * Interface definitions for a callback to be invoked when
     * update png information.
     */
    public interface OnPngUpdateListener {
        /**
         * Called when the png information updated.
         */
        void onPngUpdate(ISDBTPlayer player, SubtitleData png);
    }

    /**
     * Register a callback to be invoked when searched png.
     */
    public void setOnPngUpdateListener(OnPngUpdateListener listener) {
        mOnPngUpdateListener = listener;
    }

    private OnPngUpdateListener mOnPngUpdateListener;


    /**
     * Interface definitions for a callback to be invoked when
     * update file play information.
     */
    public interface OnFilePlayTimeUpdateListener {
        /**
         * Called when the teletextdata information updated.
         */
        void onFilePlayTimeUpdate(ISDBTPlayer player, int time);
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
        void onPlayingCompletion(ISDBTPlayer player, int type, int state);
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
        void onVideoDefinitionUpdate(ISDBTPlayer player, VideoDefinitionData videoinfo);
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
        void onDBInfoUpdate(ISDBTPlayer player, int dbtype, Object dbinfo);
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
        void onDBInformationUpdate(ISDBTPlayer player, int type, int param);
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
		void onDebugUpdate(ISDBTPlayer player, DebugModeData data);
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
         * @param player the ISDBTPlay the error pertains to
         * @param what the type of error that has occurred
         * @param extra an extra code, specific to the error.
         * @return True if the method handled the error, false if it didn't.
         * Returning false, or not having an OnErrorListener at all, will
         * cause the OnCompletionListener to be called.
         */
        boolean onError(ISDBTPlayer player, int what, int extra);
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

    /********** EPG Update *****************/
    public interface OnEPGUpdateListener
    {
        void onEPGUpdate(ISDBTPlayer player, int serviceID, int tableID);
    }
    
    public void setOnEPGUpdateListener(OnEPGUpdateListener listener)
    {
        mOnEPGUpdateListener = listener;
    }

    private OnEPGUpdateListener mOnEPGUpdateListener;
    
    /********** Parent Lock *****************/
    public interface OnParentLockListener
    {
        void onParentLock(ISDBTPlayer player, int bStatus);
    }

    public void setOnParentLockListener(OnParentLockListener listener)
    {
        mOnParentLockListener = listener;
    }

    private OnParentLockListener mOnParentLockListener;
	
    /*************** Handover ****************/
    public interface OnHandoverChannelListener 
    {
        void onHandoverChannel(ISDBTPlayer player, int affiliation, int channel);
    }

    public void setOnHandoverChannelListener(OnHandoverChannelListener listener)
    {
        mOnHandoverChannelListener = listener;
    }

    private OnHandoverChannelListener mOnHandoverChannelListener;
	
    /*************** EWS ****************/
    public interface OnEWSListener
    {
        void onEWS(ISDBTPlayer player, int bStatus, EWSData obj);
    }
    
    public void setOnEWSListener(OnEWSListener listener)
    {
        mOnEWSListener = listener;
    }

    private OnEWSListener mOnEWSListener;

    /************ SC Error ****************/
    public interface OnSCErrorListener
    {
        void onSCErrorUpdate(ISDBTPlayer player, int bStatus);
    }
    
    public void setOnSCErrorListener(OnSCErrorListener listener)
    {
        mOnSCErrorListener = listener;
    }

    private OnSCErrorListener mOnSCErrorListener;	

    /************ SC Info ****************/
    public interface OnSCInfoListener
    {
        void onSCInfoUpdate(ISDBTPlayer player, SCInfoData obj);
    }
    
    public void setOnSCInfoListener(OnSCInfoListener listener)
    {
        mOnSCInfoListener = listener;
    }

    private OnSCInfoListener mOnSCInfoListener;
	

	/********** AutoSearch Info ************/
	public interface OnAutoSearchInfoListener {
		void onAutoSearchInfoUpdate(ISDBTPlayer player, int ext1, int ext2, AutoSearchInfoData obj);
	}
	public void setOnAutoSearchInfoListener(OnAutoSearchInfoListener listener)
	{
		mOnAutoSearchInfoListener = listener;
	}
	private OnAutoSearchInfoListener mOnAutoSearchInfoListener;
	
    // XXX
    private Channel getChannelObject() {
        return new Channel();
    }

    private TeletextData getTeletextDataObject()
    {
        return new TeletextData();
    }

    // subtitle data object
    private SubtitleData getSubtitleDataObject()
    {
        return new SubtitleData();
    }

    // VideoDefinition data object
    private VideoDefinitionData getVideoDefinitionDataObject()
    {
        return new VideoDefinitionData();
    }

    private DB_CHANNEL_Data getDBChannelDataObject()
    {
        return new DB_CHANNEL_Data();
    }

    private DB_SERVICE_Data getDBServiceDataObject()
    {
        return new DB_SERVICE_Data();
    }

    private DB_AUDIO_Data getDBAudioDataObject()
    {
        return new DB_AUDIO_Data();
    }

    private DB_SUBTITLE_Data getDBSubtitleDataObject()
    {
        return new DB_SUBTITLE_Data();
    }

    // DebugMode data object
    private DebugModeData getDebugModeDataObject()
    {
        return new DebugModeData();
    }

    private EWSData getEWSDataObject()
    {
        return new EWSData();
    }
	
    private SCInfoData getSCInfoDataObject()
    {
        return new SCInfoData();
    }

	private AutoSearchInfoData getAutoSearchInfoDataObject()
	{
		return new AutoSearchInfoData();
	}

    private OnErrorListener mOnErrorListener;

    private static final int EVENT_NOP                      = 0;
    private static final int EVENT_PREPARED                 = 1;
    private static final int EVENT_SEARCH_COMPLETE          = 2;
    private static final int EVENT_CHANNEL_UPDATE           = 3;
    private static final int EVENT_TUNNING_COMPLETE         = 4;
    private static final int EVENT_SEARCH_PERCENT           = 5;
    private static final int EVENT_VIDEO_OUTPUT             = 6;
    private static final int EVENT_SUPERIMPOSE_UPDATE          = 7;
    private static final int EVENT_RECORDING_COMPLETE       = 8;
    private static final int EVENT_SUBTITLE_UPDATE          = 9;
    private static final int EVENT_VIDEODEFINITION_UPDATE   = 10;
    private static final int EVENT_DEBUGMODE_UPDATE         = 11;
    private static final int EVENT_PLAYING_COMPLETE         = 12;
    private static final int EVENT_PLAYING_CURRENT_TIME     = 13;
    private static final int EVENT_DBINFO_UPDATE            = 14;
    private static final int EVENT_DBINFORMATION_UPDATE     = 15;
    private static final int EVENT_ERROR                    = 16;
    private static final int EVENT_PNG_UPDATE          = 17;	
    private static final int EVENT_TELETEXT_UPDATE          = 20;
    private static final int EVENT_EPG_UPDATE               = 30;
    private static final int EVENT_PARENTLOCK_COMPLETE      = 31;
    private static final int EVENT_HANDOVER_CHANNEL         = 32;
    private static final int EVENT_EMERGENCY_INFO           = 33;
    private static final int EVENT_SMARTCARD_ERROR          = 34;
    private static final int EVENT_SMARTCARDINFO_UPDATE     = 35;
	private static final int EVENT_AUTOSEARCH_UPDATE 		= 36;
	private static final int EVENT_AUTOSEARCH_DONE			= 37; 
	
	private class EventHandler extends Handler
	{
		private ISDBTPlayer mISDBTPlayer;

		public EventHandler(ISDBTPlayer player, Looper looper)
		{
			super(looper);
			mISDBTPlayer = player;
		}

		@Override
		public void handleMessage(Message msg)
		{
			if (mISDBTPlayer.mNativeContext == 0)
			{
				Log.w(TAG, "player went away with unhandled ");
				return;
			}

			switch (msg.what)
			{
				case EVENT_PREPARED:
					if (mOnPreparedListener != null)
						mOnPreparedListener.onPrepared(mISDBTPlayer);
					return;

				case EVENT_SEARCH_COMPLETE:
					if (mOnSearchCompletionListener != null)
						mOnSearchCompletionListener.onSearchCompletion(mISDBTPlayer);
					return;

				case EVENT_CHANNEL_UPDATE:
					if (mOnChannelUpdateListener != null)
						mOnChannelUpdateListener.onChannelUpdate(mISDBTPlayer, msg.arg1);
					return;

				case EVENT_TUNNING_COMPLETE:
					if (mOnTunningCompletionListener != null)
						mOnTunningCompletionListener.onTunningCompletion(mISDBTPlayer, msg.arg2);
					return;

				case EVENT_SEARCH_PERCENT:
					if (mOnSearchPercentListener != null)
						mOnSearchPercentListener.onSearchPercentUpdate(mISDBTPlayer, msg.arg1, msg.arg2);
					return;

				case EVENT_VIDEO_OUTPUT:
					if (mOnVideoOutputListener != null)
						mOnVideoOutputListener.onVideoOutputUpdate(mISDBTPlayer);
					return;

				case EVENT_RECORDING_COMPLETE:
					if (mOnRecordingCompletionListener != null)
						mOnRecordingCompletionListener.onRecordingCompletion(mISDBTPlayer, msg.arg1);
					return;

				case EVENT_SUBTITLE_UPDATE:
					if (mOnSubtitleUpdateListener != null)
						mOnSubtitleUpdateListener.onSubtitleUpdate(mISDBTPlayer, (SubtitleData)msg.obj);
					return;

				case EVENT_SUPERIMPOSE_UPDATE:
					if (mOnSuperimposeUpdateListener != null)
						mOnSuperimposeUpdateListener.onSuperimposeUpdate(mISDBTPlayer, (SubtitleData)msg.obj);
					return;
					
				case EVENT_PNG_UPDATE:
					if (mOnPngUpdateListener != null)
						mOnPngUpdateListener.onPngUpdate(mISDBTPlayer, (SubtitleData)msg.obj);
					return;
					
				case EVENT_VIDEODEFINITION_UPDATE:
					if (mOnVideoDefinitionUpdateListener != null)
						mOnVideoDefinitionUpdateListener.onVideoDefinitionUpdate(mISDBTPlayer, (VideoDefinitionData)msg.obj);
					break;

				case EVENT_DBINFO_UPDATE:
					if (mOnDBInfoUpdateListener != null)
						mOnDBInfoUpdateListener.onDBInfoUpdate(mISDBTPlayer, msg.arg1, msg.obj);
					break;

				case EVENT_DBINFORMATION_UPDATE:
					if (mOnDBInformationUpdateListener != null)
						mOnDBInformationUpdateListener.onDBInformationUpdate(mISDBTPlayer, msg.arg1, msg.arg2);
					break;

				case EVENT_PLAYING_COMPLETE:
					if (mOnPlayingCompletionListener != null)
						mOnPlayingCompletionListener.onPlayingCompletion(mISDBTPlayer, msg.arg1, msg.arg2);
					return;

				case EVENT_PLAYING_CURRENT_TIME:
					if (mOnFilePlayTimeUpdateListener != null) {
						mOnFilePlayTimeUpdateListener.onFilePlayTimeUpdate(mISDBTPlayer, msg.arg1);
					}
					break;
					
				case EVENT_TELETEXT_UPDATE:
					if (mOnTeletextDataUpdateListener != null)
						mOnTeletextDataUpdateListener.onTeletextDataUpdate(mISDBTPlayer, (TeletextData) msg.obj);
					break;

				case EVENT_DEBUGMODE_UPDATE:
					Log.e(TAG, "EVENT_DEBUGMODE_UPDATE:");
					if (mOnDebugModeListener != null)
						mOnDebugModeListener.onDebugUpdate(mISDBTPlayer, (DebugModeData)msg.obj);
					break;
					
				case EVENT_ERROR:
					if (mOnErrorListener != null)
						mOnErrorListener.onError(mISDBTPlayer, msg.what, msg.arg1);
					return;

				case EVENT_EPG_UPDATE:
					if (mOnEPGUpdateListener != null)
						mOnEPGUpdateListener.onEPGUpdate(mISDBTPlayer, msg.arg1, msg.arg2);
					break;

				case EVENT_PARENTLOCK_COMPLETE:
					if (mOnParentLockListener != null)
						mOnParentLockListener.onParentLock(mISDBTPlayer, msg.arg1);
					break;

				case EVENT_HANDOVER_CHANNEL:
					if (mOnHandoverChannelListener != null)
						mOnHandoverChannelListener.onHandoverChannel(mISDBTPlayer, msg.arg1, msg.arg2);
					break;

				case EVENT_EMERGENCY_INFO:
					if(mOnEWSListener != null)
						mOnEWSListener.onEWS(mISDBTPlayer, msg.arg1, (EWSData)msg.obj);				
					break;

				case EVENT_SMARTCARD_ERROR:
					if(mOnSCErrorListener != null)
						mOnSCErrorListener.onSCErrorUpdate(mISDBTPlayer, msg.arg1);				
					break;

				case EVENT_SMARTCARDINFO_UPDATE:
					if(mOnSCInfoListener != null)
						mOnSCInfoListener.onSCInfoUpdate(mISDBTPlayer, (SCInfoData)msg.obj);				
					break;
						
				case EVENT_AUTOSEARCH_UPDATE:
					break;
				case EVENT_AUTOSEARCH_DONE:
					if (mOnAutoSearchInfoListener != null)
						mOnAutoSearchInfoListener.onAutoSearchInfoUpdate(mISDBTPlayer, msg.arg1, msg.arg2, (AutoSearchInfoData)msg.obj);
					break;
				case EVENT_NOP:
				break;

				default:
					Log.e(TAG, "Unknown message type " + msg.what);
				return;
			}
		}
	}

	private static void postEventFromNative(Object player_ref, int what, int arg1, int arg2, Object obj)
	{
		ISDBTPlayer player = (ISDBTPlayer) ((WeakReference) player_ref).get();
		
		if (player == null)
		{
			return;
		}

        if (what == EVENT_PREPARED)
        {
             player.feature_override = arg1;
        }

		if (player.mEventHandler != null)
		{
			Message m = player.mEventHandler.obtainMessage(what, arg1, arg2, obj);
			player.mEventHandler.sendMessage(m);
		}
	}
}
