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

package com.telechips.android.iptv;

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
 * IPTVPlayer class can be used to control playback
 * of IPTV streams.
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
public class IPTVPlayer {
    static {
        System.loadLibrary("iptv_jni");
    }

    private final static String TAG = "IPTVPlayer";
    private int mNativeContext; // accesed by native methods
    private int mListenerContext; // accessed by native methods
    private Surface mSurface; // accessed by native methods
    private SurfaceHolder mSurfaceHolder;
    private EventHandler mEventHandler;
    private PowerManager.WakeLock mWakeLock = null;
    private boolean mScreenOnWhilePlaying;
    private boolean mStayAwake;

    /**
     * Default constructor.
     * <p>When done with the IPTVPlayer, you should call {@link #release()},
     * to free the resources. If not released, too many IPTVPlayer instances may
     * result in an exception.</p>
     */
    public IPTVPlayer() {
        Looper looper;
        if ((looper = Looper.myLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else if ((looper = Looper.getMainLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else {
            mEventHandler = null;
        }

        native_setup(new WeakReference<IPTVPlayer>(this));
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
    public native void prepare() throws IllegalStateException;

   public native void setSurface() throws IllegalStateException;
  

   /**
     * Start playback.
     *
     * @throws IllegalStateException if it is called in an invalid state
     */
    public void start() throws IllegalStateException {
        stayAwake(true);
        _start();
    };
   
    private native void _start() throws IllegalStateException;

    /**
     * Stops playback.
     *
     * @throws IllegalStateException if it is called in an invalid state
     */
    public void stop() throws IllegalStateException {
        stayAwake(false);
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
        //mWakeLock = pm.newWakeLock(mode|PowerManager.ON_AFTER_RELEASE, IPTVPlayer.class.getName());
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
     * Returns the width of the video.
     *
     * @return the width of the video, or 0 if there is no video,
     * no display surface was set, or prepare()
     * have not completed yet
     */
    public native int getVideoWidth() throws IllegalStateException;

    /**
     * Returns the height of the video.
     *
     * @return the height of the video, or 0 if there is no video,
     * no display surface was set, or prepare()
     * has not completed yet
     */
    public native int getVideoHeight() throws IllegalStateException;
	 /**
     * Checks whether the IPTVPlayer is playing.
     *
     * @return true if currently playing, false otherwise
     */
    public native boolean isPlaying() throws IllegalStateException;

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
     * Release Surface.
     */
    public native void useSurface() throws IllegalStateException;

	/**
     * Use Surface.
     */
    
	public native void releaseSurface() throws IllegalStateException;

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

    /**
     * Sets the volume on this player.
     *
     * @param leftVolume left volume scaler
     * @param rightVolume right volume scaler
     */
    public native void setVolume(float leftVolume, float rightVolume) throws IllegalStateException;

    /**
     * Sets the Display Position on this player.
     *
     * @param x horizontal offset
     * @param y vertical offset
     * @param width horizontal width
     * @param height vertical height
     * @param rotate rotate on/off
     */
    public native void setDisplayPosition(int x, int y, int width, int height, int rotate);


	public native int setPIDInfo(int audio_pid, int video_pid, int pcr_pid, int audio_type, int video_type) throws IllegalStateException;
	public native int setIPInfo(int port, String ip, int protocol) throws IllegalStateException;

	public native int setSocketInit() throws IllegalStateException;
	public native int setSocketStart() throws IllegalStateException;
	public native int setSocketStop() throws IllegalStateException;
	public native int setSocketCommand(int cmd) throws IllegalStateException;

	public native int execute() throws IllegalStateException;
	public native int setActiveMode(int activemode) throws IllegalStateException;
		
    /**
     * Inteface definition for a callback to be invoked when the player
     * is ready for a playback.
     */
    public interface OnPreparedListener {
        /**
         * Called when the player is ready for a playback.
         *
         * @param player the IPTVPlayer that is ready for playback
         */
        void onPrepared(IPTVPlayer player); 
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
        void onSearchCompletion(IPTVPlayer player);
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
        void onVideoOutputUpdate(IPTVPlayer player);
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
        void onRecordingCompletion(IPTVPlayer player);
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
        void onChannelUpdate(IPTVPlayer player, Channel channel);
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
        void onTunningCompletion(IPTVPlayer player, int bStatus);
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
        void onSearchPercentUpdate(IPTVPlayer player, int nPercent);
    }

    public void setOnSearchPercentListener(OnSearchPercentListener listener) {
        mOnSearchPercentListener = listener;
    }
	private OnSearchPercentListener mOnSearchPercentListener;


	public interface OnDVBParsingCompletionListener  {
		/**		  * Called when the stop of recording		  */	
		void onDVBParsingCompletion (IPTVPlayer player, IPTVSIData data); 
	}

	public void    setOnDVBParsingCompletionListener (OnDVBParsingCompletionListener	listener){
		mOnDVBParsingCompletionListener = listener;
	};

	private OnDVBParsingCompletionListener mOnDVBParsingCompletionListener;

    /**
     * Interface definitions of a callback to be invoked when there
     * has been an error during an asynchronous operation (other errors
     * will throw exceptions at method call time).
     */
    public interface OnErrorListener {
        /**
         * Called to indicate an error.
         *
         * @param player the IPTVPlay the error pertains to
         * @param what the type of error that has occurred
         * @param extra an extra code, specific to the error.
         * @return True if the method handled the error, false if it didn't.
         * Returning false, or not having an OnErrorListener at all, will
         * cause the OnCompletionListener to be called.
         */
        boolean onError(IPTVPlayer player, int what, int extra);
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

    private IPTVSIData getIPTVSIDataObject() {
        return new IPTVSIData();
    }

    private OnErrorListener mOnErrorListener;

    private static final int EVENT_NOP = 0;
    private static final int EVENT_PREPARED = 1;
    private static final int EVENT_SEARCH_COMPLETE = 2;
    private static final int EVENT_CHANNEL_UPDATE = 3;
    private static final int EVENT_TUNNING_COMPLETE = 4;
    private static final int EVENT_SEARCH_PERCENT = 5;
    private static final int EVENT_VIDEO_OUTPUT = 6;
    private static final int EVENT_RECORDING_COMPLETE = 7;
    private static final int EVENT_DVB_PARSING_COMPLETE = 8;
    private static final int EVENT_ERROR = 9;

    private class EventHandler extends Handler {
        private IPTVPlayer mIPTVPlayer;

        public EventHandler(IPTVPlayer player, Looper looper) {
            super(looper);
            mIPTVPlayer = player;
        }

        @Override
        public void handleMessage(Message msg) {
            if (mIPTVPlayer.mNativeContext == 0) {
                Log.w(TAG, "player went away with unhandled ");
                return;
            }

            switch (msg.what) {
            case EVENT_PREPARED:
                if (mOnPreparedListener != null)
                    mOnPreparedListener.onPrepared(mIPTVPlayer);
                return;

            case EVENT_SEARCH_COMPLETE:
                if (mOnSearchCompletionListener != null)
                    mOnSearchCompletionListener.onSearchCompletion(mIPTVPlayer);
                return;

            case EVENT_CHANNEL_UPDATE:
                if (mOnChannelUpdateListener != null)
                mOnChannelUpdateListener.onChannelUpdate(mIPTVPlayer, (Channel) msg.obj);
                return;

 			case EVENT_TUNNING_COMPLETE:
                if (mOnTunningCompletionListener != null)
                    mOnTunningCompletionListener.onTunningCompletion(mIPTVPlayer, msg.arg2);
                return;

            case EVENT_SEARCH_PERCENT:
                if (mOnSearchPercentListener != null)
                    mOnSearchPercentListener.onSearchPercentUpdate(mIPTVPlayer, msg.arg2);
                return;

            case EVENT_VIDEO_OUTPUT:
                if (mOnVideoOutputListener != null)
                    mOnVideoOutputListener.onVideoOutputUpdate(mIPTVPlayer);
                return;

            case EVENT_RECORDING_COMPLETE:
                if (mOnRecordingCompletionListener != null)
                    mOnRecordingCompletionListener.onRecordingCompletion(mIPTVPlayer);
                return;

		case EVENT_DVB_PARSING_COMPLETE:
			if (mOnDVBParsingCompletionListener != null)
			mOnDVBParsingCompletionListener.onDVBParsingCompletion(mIPTVPlayer, (IPTVSIData)msg.obj);

		  return;

            case EVENT_ERROR:
                if (mOnErrorListener != null)
                    mOnErrorListener.onError(mIPTVPlayer, msg.what, msg.arg1);
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
        IPTVPlayer player = (IPTVPlayer) ((WeakReference) player_ref).get();
        if (player == null) {
            return;
        }

        if (player.mEventHandler != null) {
            Message m = player.mEventHandler.obtainMessage(what, arg1, arg2, obj);
            player.mEventHandler.sendMessage(m);
        }
    }
}
