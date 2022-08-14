/*
 * Copyright (C) 2009 The Android Open Source Project
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

package com.android.videoplayer;

import android.annotation.TargetApi;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.SystemProperties;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
// VIEW_SUBTITLE
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.widget.Subtitle;

import com.android.videoplayer.util.BlobCache;
import com.android.videoplayer.util.CacheManager;
import com.android.videoplayer.util.Util;
import com.android.videoplayer.util.Image;
import com.android.videoplayer.util.ImageList;
import com.android.videoplayer.util.ImageManager;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;

public class MoviePlayer implements
        MediaPlayer.OnErrorListener, MediaPlayer.OnCompletionListener,
        MediaPlayer.OnPreparedListener,
        ControllerOverlay.Listener, Subtitle.OnSubtitleListener {
    @SuppressWarnings("unused")
    private static final String TAG = "MoviePlayer";

    private static final String KEY_VIDEO_POSITION = "video-position";
    private static final String KEY_RESUMEABLE_TIME = "resumeable-timeout";

    // Copied from MediaPlaybackService in the Music Player app.
    private static final String SERVICECMD = "com.android.music.musicservicecommand";
    private static final String CMDNAME = "command";
    private static final String CMDPAUSE = "pause";

    private static final long BLACK_TIMEOUT = 500;

    // If we resume the acitivty with in RESUMEABLE_TIMEOUT, we will keep playing.
    // Otherwise, we pause the player.
    private static final long RESUMEABLE_TIMEOUT = 3 * 60 * 1000; // 3 mins

    private Context mContext;
    private final View mRootView;
    private final TCCVideoView mVideoView;
    private final Bookmarker mBookmarker;
    private Uri mUri;
    private final Handler mHandler = new Handler();
    private final AudioBecomingNoisyReceiver mAudioBecomingNoisyReceiver;
    private final TCCMediaController mController;
    private final TCCMediaMenu mMediaMenu;

    private long mResumeableTime = Long.MAX_VALUE;
    private int mVideoPosition = 0;
    private boolean mHasPaused = false;
    private int mLastSystemUiVis = 0;

    // If the time bar is being dragged.
    private boolean mDragging;

    // If the time bar is visible.
    private boolean mShowing;

    private Image mImage;
    private ImageList mVideoList = null;
    private int mCurrentPosition = 0;

    private int mPlayMode = 0;
    private boolean mSubtitleEnable = true;
    private boolean mDeinterlaceEnable = false;
    private int mDisplayMode = 0;
    private boolean mBinderDied = false;
    private boolean mErrorDeid = false;

    private Listener mTimeListener;

    private final Runnable mPlayingChecker = new Runnable() {
        @Override
        public void run() {
            if (mVideoView.isPlaying()) {
                mController.showPlaying();
            } else {
                mHandler.postDelayed(mPlayingChecker, 250);
            }
        }
    };

    private final Runnable mProgressChecker = new Runnable() {
        @Override
        public void run() {
            int pos = setProgress();
            mHandler.postDelayed(mProgressChecker, 1000 - (pos % 1000));
        }
    };

    private final Runnable mStopChecker = new Runnable() {
        @Override
        public void run() {
            if (mVideoView.isPlaying()) {
                mHandler.postDelayed(mStopChecker, 250);
            } else {
                if (mVideoList != null) {
                    mImage = mVideoList.getImageAt(mCurrentPosition);
                    mUri = mImage.fullSizeImageUri();
                }
                mVideoView.setVideoURI(mUri);
                startVideo();
                mHandler.post(mProgressChecker);
            }
        }
    };

    public MoviePlayer(View rootView, final Context context,
            Uri videoUri, ImageManager.ImageListParam param, Bundle savedInstance, boolean canReplay) {
        mContext = context;
        mRootView = rootView;
        mVideoView = (TCCVideoView) rootView.findViewById(R.id.surface_view);
        mVideoView.setFullViewMode();
        mBookmarker = new Bookmarker(context);
        mUri = videoUri;

        mController = new TCCMediaController(mContext);
        ((ViewGroup)rootView).addView(mController.getView());
        mController.setListener(this);
        mController.setCanReplay(canReplay);

        mVideoView.setOnPreparedListener(this);
        mVideoView.setOnErrorListener(this);
        mVideoView.setOnCompletionListener(this);
        // VIEW_SUBTITLE
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
        mSubtitleEnable = prefs.getBoolean("pref_gallery_video_subtitle_key", true);
        mVideoView.setOnSubtitleListener(this);
        if (mSubtitleEnable == false) {
            mController.setSubtitleCount(0);
        }
        // Replay
        mPlayMode = Integer.parseInt(prefs.getString("pref_gallery_video_repeat_key", "0"));
        setPlayList(param);
        // Deinterlace
        mDeinterlaceEnable = prefs.getBoolean("pref_gallery_video_always_deinterlace", false);
        mVideoView.setDeinterlace(mDeinterlaceEnable);
        // 3D UI Mode
        mDisplayMode = Integer.parseInt(prefs.getString("pref_gallery_video_3D_UI_key", "0"));
        setDisplayMode(mDisplayMode);
        // Menu
        mMediaMenu = (TCCMediaMenu) rootView.findViewById(R.id.media_menu);
        mMediaMenu.setVideoView(mVideoView);
		mMediaMenu.setMediaController(mController);
        TextView notification = (TextView) rootView.findViewById(R.id.notification_view);
        mMediaMenu.setNotificationView(notification);
        mVideoView.setVideoURI(mUri);
        mVideoView.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (mMediaMenu == null || !mMediaMenu.isShown())
                    mController.show();
                return true;
            }
        });

        // The SurfaceView is transparent before drawing the first frame.
        // This makes the UI flashing when open a video. (black -> old screen
        // -> video) However, we have no way to know the timing of the first
        // frame. So, we hide the VideoView for a while to make sure the
        // video has been drawn on it.
        mVideoView.postDelayed(new Runnable() {
            @Override
            public void run() {
                mVideoView.setVisibility(View.VISIBLE);
            }
        }, BLACK_TIMEOUT);

        setOnSystemUiVisibilityChangeListener();
        // Hide system UI by default
        mVideoView.showSystemUi(false);

        mAudioBecomingNoisyReceiver = new AudioBecomingNoisyReceiver();
        mAudioBecomingNoisyReceiver.register();

        Intent i = new Intent(SERVICECMD);
        i.putExtra(CMDNAME, CMDPAUSE);
        context.sendBroadcast(i);

        if (savedInstance != null) { // this is a resumed activity
            mVideoPosition = savedInstance.getInt(KEY_VIDEO_POSITION, 0);
            mResumeableTime = savedInstance.getLong(KEY_RESUMEABLE_TIME, Long.MAX_VALUE);
            mVideoView.start();
            mVideoView.suspend();
            mHasPaused = true;
        } else {
            final Integer bookmark = mBookmarker.getBookmark(mUri);
            if (bookmark != null) {
                showResumeDialog(context, bookmark);
            } else {
                startVideo();
            }
        }
    }

    public MoviePlayer(View rootView, final Context context,
            Uri videoUri, Bundle savedInstance, boolean canReplay) {
        mContext = context;
        mRootView = rootView;
        mVideoView = (TCCVideoView) rootView.findViewById(R.id.surface_view);
        mVideoView.setPreviewMode();
        mBookmarker = null;
        mUri = videoUri;

        // Replay
        mPlayMode = 1;
        setPlayList(null);

        // Controller
        mController = null;
        mShowing = true;

        // 3D UI Mode
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
        mDisplayMode = Integer.parseInt(prefs.getString("pref_gallery_video_3D_UI_key", "0"));
        setDisplayMode(mDisplayMode);

        // Menu
        mMediaMenu = null;

        mVideoView.setOnErrorListener(this);
        mVideoView.setOnCompletionListener(this);
        mVideoView.setOnPreparedListener(this);
        mVideoView.setVideoURI(mUri);

        mAudioBecomingNoisyReceiver = new AudioBecomingNoisyReceiver();
        mAudioBecomingNoisyReceiver.register();

        Intent i = new Intent(SERVICECMD);
        i.putExtra(CMDNAME, CMDPAUSE);
        context.sendBroadcast(i);

        startVideo();
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    private void setOnSystemUiVisibilityChangeListener() {
        // When the user touches the screen or uses some hard key, the framework
        // will change system ui visibility from invisible to visible. We show
        // the media control and enable system UI (e.g. ActionBar) to be visible at this point
        mVideoView.setOnSystemUiVisibilityChangeListener(
                new View.OnSystemUiVisibilityChangeListener() {
            @Override
            public void onSystemUiVisibilityChange(int visibility) {
                int diff = mLastSystemUiVis ^ visibility;
                mLastSystemUiVis = visibility;
                if ((diff & View.SYSTEM_UI_FLAG_HIDE_NAVIGATION) != 0
                        && (visibility & View.SYSTEM_UI_FLAG_HIDE_NAVIGATION) == 0) {
                    if (mMediaMenu == null || !mMediaMenu.isShown())
                        mController.show();
                }
            }
        });
    }

    public void onSaveInstanceState(Bundle outState) {
        outState.putInt(KEY_VIDEO_POSITION, mVideoPosition);
        outState.putLong(KEY_RESUMEABLE_TIME, mResumeableTime);
    }

    private void showResumeDialog(Context context, final int bookmark) {
        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        builder.setTitle(R.string.resume_playing_title);
        builder.setMessage(String.format(
                context.getString(R.string.resume_playing_message),
                Util.formatDuration(context, bookmark / 1000)));
        builder.setOnCancelListener(new OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialog) {
                onCompletion();
            }
        });
        builder.setPositiveButton(
                R.string.resume_playing_resume, new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                mVideoView.seekTo(bookmark);
                startVideo();
            }
        });
        builder.setNegativeButton(
                R.string.resume_playing_restart, new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                startVideo();
            }
        });
        builder.show();
    }

    public void onPause() {
        mHasPaused = true;
        mHandler.removeCallbacksAndMessages(null);
        mVideoPosition = mVideoView.getCurrentPosition();
        if (mBookmarker != null)
            mBookmarker.setBookmark(mUri, mVideoPosition, mVideoView.getDuration());
        mVideoView.pause(); //mVideoView.suspend();
        mResumeableTime = System.currentTimeMillis() + RESUMEABLE_TIMEOUT;
    }

    public void onResume() {
        if (mHasPaused) {
            mVideoView.setVideoURI(mUri);
            mVideoView.seekTo(mVideoPosition);
            mVideoView.resume();
            mHasPaused = false;

            // If we have slept for too long, pause the play
            if (System.currentTimeMillis() > mResumeableTime) {
                //pauseVideo();
            }
        }
        mHandler.post(mProgressChecker);
    }

    public void onDestroy() {
        mVideoView.stopPlayback();
        mVideoView.setDeinterlace(false);
        mAudioBecomingNoisyReceiver.unregister();
    }

    // This updates the time bar display (if necessary). It is called every
    // second by mProgressChecker and also from places where the time bar needs
    // to be updated immediately.
    private int setProgress() {
        if (mDragging || !mShowing) {
            return 0;
        }
        int position = mVideoView.getCurrentPosition();
        int duration = mVideoView.getDuration();
        if (mTimeListener != null) {
            mTimeListener.setTimes(position, duration);
        }
        if (mController != null) {
            mController.setTimes(position, duration, 0, 0);
        }
        return position;
    }

    private void startVideo() {
        // For streams that we expect to be slow to start up, show a
        // progress spinner until playback starts.
        String scheme = mUri.getScheme();
        if ("http".equalsIgnoreCase(scheme) || "rtsp".equalsIgnoreCase(scheme)) {
            mPlayMode = 0;
            mController.showLoading();
            mHandler.removeCallbacks(mPlayingChecker);
            mHandler.postDelayed(mPlayingChecker, 250);
        } else if (mController != null) {
            if (mImage != null)
                mController.setTitle(mImage.getTitle());
            mController.showPlaying();
            mController.hide();
        }

        mVideoView.start();
        setProgress();
    }

    private void playVideo() {
        mVideoView.start();
        mController.showPlaying();
        setProgress();
    }

    private void pauseVideo() {
        mVideoView.pause();
        mController.showPaused();
    }

    public void stopVideo() {
        if (mController != null) {
            mController.hide();
            mController.clearSubtitle();
        }
        mHandler.removeCallbacksAndMessages(null);
        mVideoView.stopPlayback();
    }

    // Below are notifications from VideoView
    @Override
    public boolean onError(MediaPlayer player, int arg1, int arg2) {
        mHandler.removeCallbacksAndMessages(null);
        // VideoView will show an error dialog if we return false, so no need
        // to show more message.
        if (mController != null)
            mController.showErrorMessage("");
        if(arg1 == MediaPlayer.MEDIA_ERROR_SERVER_DIED) {
            mBinderDied = true;
        }
        mErrorDeid = true;
        onError();
        return false;
    }

    public void onError() {
    }

    public boolean checkApplicationFinish() {
        return mBinderDied;
    }

    @Override
    public void onCompletion(MediaPlayer mp) {
        if (mController != null) {
            mController.showEnded();
        }
        if(!mErrorDeid) {
            if(mPlayMode == 1) {
                onReplay();
                return;
            } else if (mPlayMode == 2 && mVideoList != null) {
                onNextPlay();
                return;
            }
        }
        onCompletion();
        mErrorDeid = false;
    }

    public void onCompletion() {
    }

    @Override
    public void onPrepared(MediaPlayer mp) {
        mVideoView.setMediaPlayer(mp, mController);
        if (mMediaMenu != null) {
            mMediaMenu.onPrepared();
        }
        onPrepared();
        return;
    }

    public void onPrepared() {
    }
	
    // Below are notifications from ControllerOverlay
    @Override
    public void onPlayPause() {
        if (mVideoView.isPlaying()) {
            pauseVideo();
        } else {
            playVideo();
        }
    }

    @Override
    public void onPrevPlay() {
        stopVideo();
        if(mCurrentPosition == 0) {
            mCurrentPosition = mVideoList.getCount();
        }
        mCurrentPosition = (mCurrentPosition - 1) % mVideoList.getCount();
        mHandler.postDelayed(mStopChecker, 250);
    }

    @Override
    public void onNextPlay() {
        stopVideo();
        mCurrentPosition = (mCurrentPosition + 1) % mVideoList.getCount();
        mHandler.postDelayed(mStopChecker, 250);
    }

    @Override
    public void onFastForward() {
        if (mVideoView.canSeekForward()) {
            int pos = mVideoView.getCurrentPosition();
            pos += 15000; // milliseconds
            mVideoView.seekTo(pos);
        }
        setProgress();
    }

    @Override
    public void onRewind() {
        if (mVideoView.canSeekBackward()) {
            int pos = mVideoView.getCurrentPosition();
            pos -= 5000; // milliseconds
            mVideoView.seekTo(pos);
        }
        setProgress();
    }
    
    // VIEW_SUBTITLE
    // Below are notifications from Subtitle
    @Override
    public void onSubtitle(int index, String subtitle) {
        if (mController != null) {
            Log.i(TAG, "onSubtitle " + index + ", " + subtitle);
            mController.setSubtitle(index, subtitle);
        }
    }

    @Override
    public void onPGSCaption(int OffsetX, int OffsetY, int Srcwidth, int SrcHeight, int Dstwidth, int DstHeight, int StreamSize, int[] StreamData) {
       if (mController != null) {
          Log.i(TAG, "onPGSCaption()");
          mController.setPGSCaption(OffsetX, OffsetY, Srcwidth, SrcHeight, Dstwidth, DstHeight, StreamSize, StreamData);
       }
    }

    @Override
    public void onSeekStart() {
        mDragging = true;
    }

    @Override
    public void onSeekMove(int time) {
        mVideoView.seekTo(time);
    }

    @Override
    public void onSeekEnd(int time, int start, int end) {
        mDragging = false;
        mVideoView.seekTo(time);
        setProgress();
    }

    @Override
    public void onShown() {
        mShowing = true;
        setProgress();
        mVideoView.showSystemUi(true);
    }

    @Override
    public void onHidden() {
        mShowing = false;
        mVideoView.showSystemUi(false);
    }

    @Override
    public void onReplay() {
        stopVideo();
        mHandler.postDelayed(mStopChecker, 250);
    }

    // Below are key events passed from MovieActivity.
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (mMediaMenu != null && mMediaMenu.isShown()) {
            return mMediaMenu.dispatchKeyEvent(event);
        }

        // Some headsets will fire off 7-10 events on a single click
        if (event.getRepeatCount() > 0) {
            return isMediaKey(keyCode);
        }

        switch (keyCode) {
            case KeyEvent.KEYCODE_HEADSETHOOK:
            case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
                if (mVideoView.isPlaying()) {
                    pauseVideo();
                } else {
                    playVideo();
                }
                return true;
            case KeyEvent.KEYCODE_MEDIA_PAUSE:
                if (mVideoView.isPlaying()) {
                    pauseVideo();
                }
                return true;
            case KeyEvent.KEYCODE_MEDIA_PLAY:
                if (!mVideoView.isPlaying()) {
                    playVideo();
                }
                return true;
            case KeyEvent.KEYCODE_MEDIA_PREVIOUS:
            case KeyEvent.KEYCODE_PAGE_UP:
                onPrevPlay();
                return true;
            case KeyEvent.KEYCODE_MEDIA_NEXT:
            case KeyEvent.KEYCODE_PAGE_DOWN:
                onNextPlay();
                return true;
            case KeyEvent.KEYCODE_BACK:
                if (mController.isShowing() && event.getDeviceId() >= 0) {
                    mController.hide();
                } else {
                    onCompletion();
                }
                return true;
            case KeyEvent.KEYCODE_MENU:
                mController.hide();
                if (mMediaMenu != null) {
                    mMediaMenu.show();
                }
                return true;
            case KeyEvent.KEYCODE_MEDIA_STOP:
                if (mMediaMenu != null) {
                    if (mMediaMenu.getPlaySpeed() == 5)
                        onCompletion();
                    else
                        mMediaMenu.setPlaySpeed(5);
                }
                return true;
            case KeyEvent.KEYCODE_MEDIA_REWIND:
                if (mMediaMenu != null) {
                    int speed = mMediaMenu.getPlaySpeed();
                    if (speed > 5) speed = 5;
                    else if (speed > 0) speed--;
                    mMediaMenu.setPlaySpeed(speed);
                }
                return true;
            case KeyEvent.KEYCODE_MEDIA_FAST_FORWARD:
                if (mMediaMenu != null) {
                    int speed = mMediaMenu.getPlaySpeed();
                    if (speed < 5) speed = 5;
                    else if (speed < 10) speed++;
                    mMediaMenu.setPlaySpeed(speed);
                }
                return true;
        }
        mController.show();
        return false;
    }

    public boolean onKeyUp(int keyCode, KeyEvent event) {
        return isMediaKey(keyCode);
    }

    private static boolean isMediaKey(int keyCode) {
        return keyCode == KeyEvent.KEYCODE_HEADSETHOOK
                || keyCode == KeyEvent.KEYCODE_MEDIA_PREVIOUS
                || keyCode == KeyEvent.KEYCODE_MEDIA_NEXT
                || keyCode == KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE
                || keyCode == KeyEvent.KEYCODE_MEDIA_PLAY
                || keyCode == KeyEvent.KEYCODE_MEDIA_PAUSE
                || keyCode == KeyEvent.KEYCODE_MEDIA_STOP
                || keyCode == KeyEvent.KEYCODE_MEDIA_REWIND
                || keyCode == KeyEvent.KEYCODE_MEDIA_FAST_FORWARD
                || keyCode == KeyEvent.KEYCODE_PAGE_UP
                || keyCode == KeyEvent.KEYCODE_PAGE_DOWN
                || keyCode == KeyEvent.KEYCODE_BACK
                || keyCode == KeyEvent.KEYCODE_MENU;
    }

    // We want to pause when the headset is unplugged.
    private class AudioBecomingNoisyReceiver extends BroadcastReceiver {

        public void register() {
            mContext.registerReceiver(this,
                    new IntentFilter(AudioManager.ACTION_AUDIO_BECOMING_NOISY));
        }

        public void unregister() {
            mContext.unregisterReceiver(this);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            if (mVideoView.isPlaying()) pauseVideo();
        }
    }

    private void setPlayList(ImageManager.ImageListParam param) {
        if(param != null ) {
            if(!param.mIsEmptyImageList){
                Log.d(TAG, "mInclusion: " + param.mInclusion + " getPath " + mUri.getPath() + " BucketId :" + param.mBucketId);
                if ( param.mInclusion == ImageManager.INCLUDE_VIDEO && mUri.getScheme().equals("file")) {
                    mVideoList = ImageManager.makeImageList(param, param.mBucketId);
                } else {
                    mVideoList = ImageManager.makeImageList(param, null);
                }
            } else
                mVideoList = null;
        }
        if (mVideoList != null) {
            mImage = mVideoList.getImageForUri(mUri);
            Log.d(TAG, "mVideoList.getCount(): " + mVideoList.getCount());
            if (mImage != null) {
                mCurrentPosition = mVideoList.getImageIndex(mImage);
                Log.d(TAG, "mCurrentPosition : " + mCurrentPosition + " getTitle() :" + mImage.getTitle());
            } else {
                Log.d(TAG, "mImage NULL ");
            }
        } else {
            Log.d(TAG, "mVideoList NULL ");
        }
    }

    public void onReceive(Intent intent) {
        if (mUri.getPath().contains(intent.getData().getPath())) {
            stopVideo();
            onCompletion();
        }
    }

    public void setTimeListener(Listener listener) {
        mTimeListener = listener;
    }

    public interface Listener {
        void setTimes(int currentTime, int totalTime);
    }

    public void setDisplayMode(int mode) {
		if(mode == 1) {
		        SystemProperties.set("tcc.3d.ui.mode", "1");
			SystemProperties.set("tcc.video.mvc.support", "0");
		} else if(mode == 2) {
		        SystemProperties.set("tcc.3d.ui.mode", "2");
			SystemProperties.set("tcc.video.mvc.support", "0");
		} else if(mode == 3) {
			SystemProperties.set("tcc.3d.ui.mode", "0");
			SystemProperties.set("tcc.video.mvc.support", "1");
		} else {
		        SystemProperties.set("tcc.3d.ui.mode", "0");
			SystemProperties.set("tcc.video.mvc.support", "0");
		}
    }
}

class Bookmarker {
    private static final String TAG = "Bookmarker";

    private static final String BOOKMARK_CACHE_FILE = "bookmark";
    private static final int BOOKMARK_CACHE_MAX_ENTRIES = 100;
    private static final int BOOKMARK_CACHE_MAX_BYTES = 10 * 1024;
    private static final int BOOKMARK_CACHE_VERSION = 1;

    private static final int HALF_MINUTE = 30 * 1000;
    private static final int TWO_MINUTES = 4 * HALF_MINUTE;

    private final Context mContext;

    public Bookmarker(Context context) {
        mContext = context;
    }

    public void setBookmark(Uri uri, int bookmark, int duration) {
        try {
            BlobCache cache = CacheManager.getCache(mContext,
                    BOOKMARK_CACHE_FILE, BOOKMARK_CACHE_MAX_ENTRIES,
                    BOOKMARK_CACHE_MAX_BYTES, BOOKMARK_CACHE_VERSION);

            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            DataOutputStream dos = new DataOutputStream(bos);
            dos.writeUTF(uri.toString());
            dos.writeInt(bookmark);
            dos.writeInt(duration);
            dos.flush();
            cache.insert(uri.hashCode(), bos.toByteArray());
        } catch (Throwable t) {
            Log.w(TAG, "setBookmark failed", t);
        }
    }

    public Integer getBookmark(Uri uri) {
        try {
            BlobCache cache = CacheManager.getCache(mContext,
                    BOOKMARK_CACHE_FILE, BOOKMARK_CACHE_MAX_ENTRIES,
                    BOOKMARK_CACHE_MAX_BYTES, BOOKMARK_CACHE_VERSION);

            byte[] data = cache.lookup(uri.hashCode());
            if (data == null) return null;

            DataInputStream dis = new DataInputStream(
                    new ByteArrayInputStream(data));

            String uriString = DataInputStream.readUTF(dis);
            int bookmark = dis.readInt();
            int duration = dis.readInt();

            if (!uriString.equals(uri.toString())) {
                return null;
            }

            if ((bookmark < HALF_MINUTE) || (duration < TWO_MINUTES)
                    || (bookmark > (duration - HALF_MINUTE))) {
                return null;
            }
            return Integer.valueOf(bookmark);
        } catch (Throwable t) {
            Log.w(TAG, "getBookmark failed", t);
        }
        return null;
    }
}
