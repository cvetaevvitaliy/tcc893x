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

package com.telechips.android.tdmb.player;

import java.lang.ref.WeakReference;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;

/**
 * TDMBPlayer class can be used to control playback
 * of TDMB streams.
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
public class TDMBPlayer {
    static {
        System.loadLibrary("tdmb_jni");
    }

    private final static String TAG = "TDMBPlayer";
    private int mNativeContext; // accesed by native methods
    private int mListenerContext; // accessed by native methods
    private Surface mSurface; // accessed by native methods
    private SurfaceHolder mSurfaceHolder;
    private EventHandler mEventHandler;
    private PowerManager.WakeLock mWakeLock = null;
    private boolean mScreenOnWhilePlaying;
    private boolean mStayAwake;
    public int mModuleIndex;

    /**
     * Default constructor.
     * <p>When done with the TDMBPlayer, you should call {@link #release()},
     * to free the resources. If not released, too many TDMBPlayer instances may
     * result in an exception.</p>
     */
    public TDMBPlayer() {
        Looper looper;
        if ((looper = Looper.myLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else if ((looper = Looper.getMainLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else {
            mEventHandler = null;
        }

        native_setup(new WeakReference<TDMBPlayer>(this));
    }

    private native final void native_setup(Object weak_this);
    private native final void native_finalize();

    public native int setBBModuleIndex(int bbidx, int moduleidx) throws IllegalStateException;;

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
    public native void prepare(int basebandType, String dbPath) throws IllegalStateException;

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
        //mWakeLock = pm.newWakeLock(mode|PowerManager.ON_AFTER_RELEASE, context.getClass().getName());
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
                Log.w("stayAwake", "acquire");
                mWakeLock.acquire();
            } else if (!awake && mWakeLock.isHeld()) {
                Log.w("stayAwake", "release");
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
     * Returns the signal strength of the tuner.
     */
    public native int[] getSignalStrength() throws IllegalStateException;

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
    public native void search(int countryCode, int[] freqList, int freqListCount) throws IllegalStateException;

    /**
     * Stop search channels.
     */
	public native void searchCancel() throws IllegalStateException;

    /**
     * Manual Search channels.
     *
     * @param frequency
     */
    public native void manual_search(int freq) throws IllegalStateException;

	public native void manual_setChannel(int[] channelInfo) throws IllegalStateException;

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
     * @param channel channel information to set
     */
    public native void setChannelIndex(int channelIndex, int type) throws IllegalStateException;

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
     * Sets LCDUpdate
     *
     * @param void
     */
	public native int setLCDUpdate() throws IllegalStateException;

    public native void setDisplayEnable() throws IllegalStateException;
    public native void setDisplayDisable() throws IllegalStateException;

    public native void setAudioMute(boolean isMute) throws IllegalStateException;

    /**
     * Inteface definition for a callback to be invoked when the player
     * is ready for a playback.
     */
    public interface OnPreparedListener {
        /**
         * Called when the player is ready for a playback.
         *
         * @param player the TDMBPlayer that is ready for playback
         */
        void onPrepared(TDMBPlayer player, int idx, int ret); 
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
        void onSearchCompletion(TDMBPlayer player, int idx, int manual);
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
     * Interface definitions for a callback to be invoked when audio output
     * has started.
     */
    public interface OnAudioOutputListener {
        /**
         * Called when the start of audio output
         */
        void onAudioOutputUpdate(TDMBPlayer player);
    }

    /**
     * Register a callback to be invoked when audio output
     * has started
     *
     * @param listener the callback that will be run
     */
    public void setOnAudioOutputListener(OnAudioOutputListener listener) {
        mOnAudioOutputListener = listener;
    }

    private OnAudioOutputListener mOnAudioOutputListener;

    /**
     * Interface definitions for a callback to be invoked when video output
     * has started.
     */
    public interface OnVideoOutputListener {
        /**
         * Called when the start of video output
         */
        void onVideoOutputUpdate(TDMBPlayer player);
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
        void onRecordingCompletion(TDMBPlayer player, int nResult);
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
        void onChannelUpdate(TDMBPlayer player, int moduleidx, Channel channel);
    }

    /**
     * Register a callback to be invoked when searched channel.
     */
    public void setOnChannelUpdateListener(OnChannelUpdateListener listener) {
        mOnChannelUpdateListener = listener;
    }

    private OnChannelUpdateListener mOnChannelUpdateListener;

    /**
     * Interface definitions for a callback to be invoked when
     * update dabdlsdata information.
     */
    public interface OnDABDLSDataUpdateListener {
        /**
         * Called when the dabdlsdata information updated.
         */
        void onDABDLSDataUpdate(TDMBPlayer player, DABDLSData channel);
    }

    /**
     * Register a callback to be invoked when searched dabdlsdata.
     */
    public void setOnDABDLSDataUpdateListener(OnDABDLSDataUpdateListener listener) {
        mOnDABDLSDataUpdateListener = listener;
    }

    private OnDABDLSDataUpdateListener mOnDABDLSDataUpdateListener;

    /**
     * Interface definitions for a callback to be invoked when
     * update ews information.
     */
    public interface OnEWSDataUpdateListener {
        /**
         * Called when the ews information updated.
         */
        void onEWSDataUpdate(TDMBPlayer player, EWSData ewsData);
    }

    /**
     * Register a callback to be invoked when searched ews data.
     */
    public void setOnEWSDataUpdateListener(OnEWSDataUpdateListener listener) {
        mOnEWSDataUpdateListener = listener;
    }

    private OnEWSDataUpdateListener mOnEWSDataUpdateListener;

    /**
     * Interface definitions for a callback to be invoked when
     * update data service information.
     */
    public interface OnDataSVCUpdateListener {
        /**
         * Called when the ews information updated.
         */
        void onDataSVCUpdate(TDMBPlayer player);
    }

    /**
     * Register a callback to be invoked when searched data service.
     */
    public void setOnDataSVCUpdateListener(OnDataSVCUpdateListener listener) {
        mOnDataSVCUpdateListener = listener;
    }

    private OnDataSVCUpdateListener mOnDataSVCUpdateListener;

    /**
     * Interface definitions for a callback to be invoked when channel tunning
     * has completed.
     */
    public interface OnTunningCompletionListener {
        /**
         * Called when the end of channel search is reached during tunning.
         */
        void onTunningCompletion(TDMBPlayer player, int idx, int bStatus);
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
        void onSearchPercentUpdate(TDMBPlayer player, int idx, int nPercent);
    }

    public void setOnSearchPercentListener(OnSearchPercentListener listener) {
        mOnSearchPercentListener = listener;
    }
	private OnSearchPercentListener mOnSearchPercentListener;

    /**
     * Interface definitions of a callback to be invoked when there
     * has been an error during an asynchronous operation (other errors
     * will throw exceptions at method call time).
     */
    public interface OnErrorListener {
        /**
         * Called to indicate an error.
         *
         * @param player the TDMBPlay the error pertains to
         * @param what the type of error that has occurred
         * @param extra an extra code, specific to the error.
         * @return True if the method handled the error, false if it didn't.
         * Returning false, or not having an OnErrorListener at all, will
         * cause the OnCompletionListener to be called.
         */
        boolean onError(TDMBPlayer player, int what, int extra);
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

    // XXX
    private Channel getChannelObject() {
        return new Channel();
    }

    // XXX
    private DABDLSData getDABDLSDataObject() {
        return new DABDLSData();
    }

    // XXX
    private EWSData getEWSDataObject() {
        return new EWSData();
    }

    private OnErrorListener mOnErrorListener;

    private static final int EVENT_NOP = 0;
    private static final int EVENT_PREPARED = 1;
    private static final int EVENT_SEARCH_COMPLETE = 2;
    private static final int EVENT_CHANNEL_UPDATE = 3;
    private static final int EVENT_TUNNING_COMPLETE = 4;
    private static final int EVENT_SEARCH_PERCENT = 5;	
    private static final int EVENT_AUDIO_OUTPUT = 6;
    private static final int EVENT_VIDEO_OUTPUT = 7;
    private static final int EVENT_RECORDING_COMPLETE = 8;
    private static final int EVENT_PARENTLOCK_COMPLETE = 9;
    private static final int EVENT_DLSDATA_UPDATE = 10;
    private static final int EVENT_EWS_UPDATE= 11;
    private static final int EVENT_DATA_UPDATE = 12;
    private static final int EVENT_ERROR = 13;
    private static final int EVENT_SERVICE_DIED = 99;

    private class EventHandler extends Handler {
        private TDMBPlayer mTDMBPlayer;

        public EventHandler(TDMBPlayer player, Looper looper) {
            super(looper);
            mTDMBPlayer = player;
        }

        @Override
        public void handleMessage(Message msg) {
            if (mTDMBPlayer.mNativeContext == 0) {
                Log.w(TAG, "player went away with unhandled ");
                return;
            }

            switch (msg.what) {
            case EVENT_PREPARED:
                if (mOnPreparedListener != null)
                    mOnPreparedListener.onPrepared(mTDMBPlayer, msg.arg1, msg.arg2);
                return;

            case EVENT_SEARCH_COMPLETE:
                if (mOnSearchCompletionListener != null)
                    mOnSearchCompletionListener.onSearchCompletion(mTDMBPlayer, msg.arg1, msg.arg2);
                return;

            case EVENT_CHANNEL_UPDATE:
                if (mOnChannelUpdateListener != null)
                	mOnChannelUpdateListener.onChannelUpdate(mTDMBPlayer, msg.arg1, (Channel) msg.obj);
                return;

            case EVENT_DLSDATA_UPDATE:
                if (mOnDABDLSDataUpdateListener != null)
                mOnDABDLSDataUpdateListener.onDABDLSDataUpdate(mTDMBPlayer, (DABDLSData) msg.obj);
                return;

            case EVENT_TUNNING_COMPLETE:
                if (mOnTunningCompletionListener != null)
                    mOnTunningCompletionListener.onTunningCompletion(mTDMBPlayer, msg.arg1, msg.arg2);
                return;

            case EVENT_SEARCH_PERCENT:
                if (mOnSearchPercentListener != null)
                    mOnSearchPercentListener.onSearchPercentUpdate(mTDMBPlayer, msg.arg1, msg.arg2);
                return;

            case EVENT_AUDIO_OUTPUT:
                if (mOnAudioOutputListener != null)
                    mOnAudioOutputListener.onAudioOutputUpdate(mTDMBPlayer);
                return;

            case EVENT_VIDEO_OUTPUT:
                if (mOnVideoOutputListener != null)
                    mOnVideoOutputListener.onVideoOutputUpdate(mTDMBPlayer);
                return;

            case EVENT_RECORDING_COMPLETE:
                if (mOnRecordingCompletionListener != null)
                    mOnRecordingCompletionListener.onRecordingCompletion(mTDMBPlayer, msg.arg1);
                return;

            case EVENT_EWS_UPDATE:
                if (mOnEWSDataUpdateListener != null)
                    mOnEWSDataUpdateListener.onEWSDataUpdate(mTDMBPlayer, (EWSData) msg.obj);
                return;

            case EVENT_DATA_UPDATE:
                if (mOnDataSVCUpdateListener != null)
                    mOnDataSVCUpdateListener.onDataSVCUpdate(mTDMBPlayer);
                return;

            case EVENT_ERROR:
                if (mOnErrorListener != null)
                    mOnErrorListener.onError(mTDMBPlayer, msg.what, msg.arg1);
                return;
           
            case EVENT_SERVICE_DIED:
                Log.e(TAG, "EVENT_SERVICE_DIED receive");
                return;
           
            case EVENT_NOP:
                break;

            default:
                Log.e(TAG, "Unknown message type " + msg.what);
                return;
            }
        }
    }

    private static void postEventFromNative(Object player_ref,
                                            int what, int arg1, int arg2, Object obj) {
        TDMBPlayer player = (TDMBPlayer) ((WeakReference) player_ref).get();
        if (player == null) {
            return;
        }

        if (player.mEventHandler != null) {
            Message m = player.mEventHandler.obtainMessage(what, arg1, arg2, obj);
            player.mEventHandler.sendMessage(m);
        }
    }
}
