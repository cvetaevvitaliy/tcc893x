/*
 * Copyright (C) 2006 The Android Open Source Project
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
import android.content.Context;
import android.view.View;
import android.widget.VideoView;
import android.widget.RelativeLayout;
import android.media.MediaPlayer.TrackInfo;
import android.media.MediaSubtitle;
import android.media.MediaPlayer;
import android.media.MediaParameterKeys;
import android.os.SystemProperties;
import android.os.Build;
import android.util.AttributeSet;
import android.util.Log;

public class TCCVideoView extends VideoView {
    private static final String TAG = "TCCVideoView";
    private static final boolean DEBUG = false;

    private static Context mContext;
    private static int mViewMode;
    private MediaPlayer mPlayer;
    private TCCMediaController mController;
    private int mPlayRate;

    public TCCVideoView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
        mViewMode = 0;
    }

    @SuppressWarnings("deprecation")
    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    public void showSystemUi(boolean visible) {
        int flag = View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE;
        if (!visible) {
            // We used the deprecated "STATUS_BAR_HIDDEN" for unbundling
            flag |= View.STATUS_BAR_HIDDEN | View.SYSTEM_UI_FLAG_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION;
        }
        setSystemUiVisibility(flag);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        if (DEBUG) Log.i(TAG, "onMeasure(widthMeasureSpec=" + widthMeasureSpec + " heightMeasureSpec=" + heightMeasureSpec + "mViewMode=" + mViewMode + ")");
        if (mPlayer != null && isPlaying()) {
            int videoWidth = mPlayer.getVideoWidth();
            int videoHeight = mPlayer.getVideoHeight();
            int layoutWidth = getDefaultSize(videoWidth, widthMeasureSpec);
            int layoutHeight = getDefaultSize(videoHeight, heightMeasureSpec);
            float disp_ratio = (layoutWidth / (float)layoutHeight);
            float video_ratio = (videoWidth / (float)videoHeight);
			int r;

            if (mViewMode == 1) {
                if(disp_ratio > video_ratio) {
                    heightMeasureSpec = (int)(layoutWidth * videoHeight / (float)videoWidth);
                    widthMeasureSpec = layoutWidth;
                } else if(disp_ratio < video_ratio) {
                    widthMeasureSpec = (int)(layoutHeight * videoWidth / (float)videoHeight);
                    heightMeasureSpec = layoutHeight;
                }
            }
        }

        if (mViewMode == 0) {
            super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        } else {
            setMeasuredDimension(widthMeasureSpec, heightMeasureSpec);
        }
    }

    public void setPreviewMode() {
        mViewMode = 0;
        SystemProperties.set("tcc.solution.preview","1");
    }

    public void setBoxViewMode() {
        mViewMode = 0;
        SystemProperties.set("tcc.solution.preview","0");

        RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.MATCH_PARENT);
        params.addRule(RelativeLayout.CENTER_IN_PARENT);
        setLayoutParams(params);
    }

    public void setPanScanViewMode() {
        mViewMode = 1;
        SystemProperties.set("tcc.solution.preview","0");

        RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.MATCH_PARENT);
        params.addRule(RelativeLayout.CENTER_IN_PARENT);
        setLayoutParams(params);
    }

    public void setFullViewMode() {
        mViewMode = 2;
        SystemProperties.set("tcc.solution.preview","0");

        RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.MATCH_PARENT);
        params.addRule(RelativeLayout.CENTER_IN_PARENT);
        setLayoutParams(params);
    }

    public void setMediaPlayer(MediaPlayer mp, TCCMediaController mc) {
        if (DEBUG) Log.i(TAG, "setMediaPlayer");
        mPlayer = mp;
        mController = mc;
        mPlayRate = 5;
    }

    public void setPlaySpeed(int cmd) {
        int[] playRateTbl = {-3200000, -1600000, -800000, -400000, -200000, 100000,
                200000, 400000, 800000, 1600000, 3200000};
        if (mPlayer != null && canSeekForward() && canSeekBackward()) {
            if (mPlayRate == cmd || cmd < 0 || cmd > 10) {
                return;
            }
            mPlayRate = cmd;
            mPlayer.pause();
            do {
                try {
                    Thread.sleep(100);
                } catch(InterruptedException e) {
                    Log.d(TAG, "sleep fail!");
                }
            } while(isPlaying());
	    mPlayer.setPlayRate(playRateTbl[mPlayRate]);
            if (mPlayRate == 5) {
                seekTo(getCurrentPosition());
            } else if (mController != null) {
                mController.clearSubtitle();
            }
            mPlayer.start();
        }
    }

    @Override
    public void seekTo(int msec) {
        Log.e(TAG, "seekTo(" + msec + ")" + " mPlayRate= " + mPlayRate);
        if (mPlayRate == 5 && canSeekForward() && canSeekBackward())
            super.seekTo(msec);
    }

    @Override
    public void start() {
		setVisibility(View.VISIBLE);
		super.start();
    }

    @Override
    public void stopPlayback() {
        super.stopPlayback();
        mPlayer = null;
        // VIEW_SUBTITLE
        if (mController != null)
            mController.clearSubtitle();
        setVisibility(View.INVISIBLE);
    }

    public int getSubtitleType() {
        return super.getSubtitleType();
    }

    public void changeSubtitle(int i) {
        int[] classindex = {i, 0};
        if (i < 0) {
            mController.setSubtitleCount(0);
        } else {
            mController.setSubtitleCount(1);
            super.changeSubtitleClass(1, classindex);
        }
    }

    public void setTimeShiftPTS(int i) {
        super.setSubtitleTimeShiftPTS(i);
    }

    public void setSubtitleSize(int size) {
        mController.setSubtitleSize(size);
    }

    public void setSubtitlePosition(int position) {
        mController.setSubtitlePosition(position);
    }

    public int getTotalAudioTrackCount() {
        int count = 0;
        if (mPlayer != null) {
            TrackInfo[] info =  mPlayer.getTrackInfo();
            if (info == null) {
                Log.e(TAG, "Failed to get track info");
                return 0;
            }
            for (int i=0; i < info.length; i++) {
                if (info[i].getTrackType() == TrackInfo.MEDIA_TRACK_TYPE_AUDIO) {
                    count++;
                }
            }
        }
        Log.i(TAG, "Total Audio Track : " + count);
        return count;
    }

    public void getSubtitleClass(MediaSubtitle mediaSubtitle) {
        if (mPlayer != null) {
            mPlayer.getSubtitleClass(mediaSubtitle);
        }
    }

    public boolean setAudioTrack(int i) {
        int count = 0, num;
        if (mPlayer != null) {
            TrackInfo[] info =  mPlayer.getTrackInfo();
            if (info == null) {
                return false;
            }
            for (num=0; num < info.length; num++) {
                if (info[num].getTrackType() == TrackInfo.MEDIA_TRACK_TYPE_AUDIO) {
                    if (i == count) {
                        mPlayer.selectTrack(num);
                        return true;
                    }
                    count++;
                }
            }
        }
        return false;
    }

    public void setDeinterlace(boolean enable) {
        if (enable) {
            SystemProperties.set("tcc.video.deinterlace.force", "1");
        } else {
            SystemProperties.set("tcc.video.deinterlace.force", "0");
        }
    }
}
