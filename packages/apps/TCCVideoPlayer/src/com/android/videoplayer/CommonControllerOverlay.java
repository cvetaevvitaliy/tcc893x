/*
 * Copyright (C) 2012 The Android Open Source Project
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

import android.content.Context;
import android.graphics.Rect;
import android.graphics.Paint;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;
// VIEW_SUBTITLE
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.graphics.Bitmap;

/**
 * The common playback controller for the Movie Player or Video Trimming.
 */
public abstract class CommonControllerOverlay extends FrameLayout implements
        ControllerOverlay,
        OnClickListener,
        TimeBar.Listener {

    protected enum State {
        PLAYING,
        PAUSED,
        ENDED,
        ERROR,
        LOADING
    }
    
    protected Listener mListener;

    protected TimeBar mTimeBar;

    // VIEW_SUBTITLE
    protected boolean mSubtitleEnable = false;
    protected boolean mPGSCaptionEnable = false;
 
    protected static final int SUBTITLE_MAX_NUM = 2;
    protected TextView[] mSubtitleView;
    protected int mShowSubtitle = 1;
    protected int mSubtitlePosition = 48;

    protected Bitmap	mBitmap;
    protected Bitmap	mReSizedBitmap;
    protected Bitmap	mBitmapErease;

    protected int mPGSBitMap_Width;
    protected int mPGSBitMap_Height;
    protected final ImageView PgsCaptionView;

    protected State mState;

    protected boolean mCanReplay = true;

    public CommonControllerOverlay(Context context) {
        super(context);

        mState = State.LOADING;
        // TODO: Move the following layout code into xml file.
        LayoutParams wrapContent =
                new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT);

        // Depending on the usage, the timeBar can show a single scrubber, or
        // multiple ones for trimming.
        createTimeBar(context);
        addView(mTimeBar.getView(), wrapContent);

        // VIEW_SUBTITLE
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        int fontsize = Integer.parseInt(prefs.getString("pref_gallery_video_fontsize_key", "40"));
        mSubtitleView =  new TextView[SUBTITLE_MAX_NUM];
        for(int i=0; i<SUBTITLE_MAX_NUM; i++) {
            mSubtitleView[i] = new TextView(context);
            mSubtitleView[i].setText(" ");

            mSubtitleView[i].setGravity(Gravity.CENTER);
            mSubtitleView[i].setBackgroundColor(0x00000000);
 
            mSubtitleView[i].setShadowLayer((float)1.2, (float)1.2, (float)1.2, (int)0xFFFFFF);
            mSubtitleView[i].setTextSize(fontsize);
            mSubtitleView[i].setPaintFlags(mSubtitleView[i].getPaintFlags() | Paint.FAKE_BOLD_TEXT_FLAG);
        }

        mPGSBitMap_Width = 800;
        mPGSBitMap_Height = 444;
        mBitmapErease = Bitmap.createBitmap(mPGSBitMap_Width, mPGSBitMap_Height, Bitmap.Config.ARGB_8888);
    
        PgsCaptionView = new ImageView(context);
        PgsCaptionView.setScaleType(ImageView.ScaleType.CENTER_INSIDE);
        PgsCaptionView.setVisibility(View.VISIBLE);
    
        mSubtitleEnable = false;
        mPGSCaptionEnable = false;

        RelativeLayout.LayoutParams params =
                new RelativeLayout.LayoutParams(
                        LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
        setLayoutParams(params);
        hide();
    }

    abstract protected void createTimeBar(Context context);

    @Override
    public void setListener(Listener listener) {
        this.mListener = listener;
    }

    @Override
    public void setCanReplay(boolean canReplay) {
        this.mCanReplay = canReplay;
    }

    @Override
    public View getView() {
        return this;
    }

    @Override
    public void showPlaying() {
        mState = State.PLAYING;
    }

    @Override
    public void showPaused() {
        mState = State.PAUSED;
    }

    @Override
    public void showEnded() {
        mState = State.ENDED;
    }

    @Override
    public void showLoading() {
        mState = State.LOADING;
    }

    @Override
    public void showErrorMessage(String message) {
        mState = State.ERROR;
    }

    @Override
    public void setTimes(int currentTime, int totalTime,
            int trimStartTime, int trimEndTime) {
        mTimeBar.setTime(currentTime, totalTime, trimStartTime, trimEndTime);
    }

  // VIEW_SUBTITLE
    public void setSubtitle(int index, String subtitle) {
        if (mSubtitleEnable == false) {
            for(int i=0; i<SUBTITLE_MAX_NUM; i++) {
                addView(mSubtitleView[i], new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT));
            }
            mSubtitleEnable = true;
        }

        mSubtitleView[index].setText(subtitle);
        showSubtitleView(index, mSubtitleView[index]);
    }

    public void clearSubtitle() {
        if (mSubtitleEnable == true) {
            for (int i=0; i<SUBTITLE_MAX_NUM; i++) {
                if (mSubtitleView[i] != null) {
                    mSubtitleView[i].setText(" ");
                }
            }
        }
    }

    public void setSubtitleCount(int num) {
        mShowSubtitle = num;

        // VIEW_SUBTITLE
        if (mSubtitleEnable == true) {
            for(int i=0; i<SUBTITLE_MAX_NUM; i++) {
                showSubtitleView(i, mSubtitleView[i]);
            }
        }
        if(mPGSCaptionEnable == true)
            showPGSView(PgsCaptionView);
    }

    public void setSubtitleSize(int size) {
        for(int i=0; i<SUBTITLE_MAX_NUM; i++) {
            mSubtitleView[i].setTextSize(size);
        }
    }

    public void setSubtitlePosition(int position) {
        mSubtitlePosition = position;
        requestLayout();
    }

    public void setPGSCaption(int OffsetX, int OffsetY, int Srcwidth, int SrcHeight, int Dstwidth, int DstHeight, int StreamSize, int[] StreamData) {
        if (mPGSCaptionEnable == false) {
            addView(PgsCaptionView, new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
            mPGSCaptionEnable = true;
        }
     
        if(StreamSize == 0)
        {
            mBitmapErease.eraseColor(0x00000000);
            PgsCaptionView.setImageBitmap(mBitmapErease);
        }
        else
        {
            mBitmapErease.eraseColor(0x00000000);
            PgsCaptionView.setImageBitmap(mBitmapErease);
            mBitmap = Bitmap.createBitmap(StreamData, 0, Srcwidth, Srcwidth, SrcHeight, Bitmap.Config.ARGB_8888);
            mReSizedBitmap = Bitmap.createScaledBitmap(mBitmap, Dstwidth, DstHeight, true);
        
            PgsCaptionView.setImageBitmap(mReSizedBitmap);
            PgsCaptionView.layout(OffsetX, OffsetY-30, OffsetX+Dstwidth, OffsetY+DstHeight-30);
            PgsCaptionView.requestLayout();
         }
         showPGSView(PgsCaptionView);
    }

    public void hide() {
        mTimeBar.getView().setVisibility(View.INVISIBLE);
        setVisibility(View.INVISIBLE);
        setFocusable(true);
        requestFocus();

        // VIEW_SUBTITLE
        if (mSubtitleEnable == true) {
            for(int i=0; i<SUBTITLE_MAX_NUM; i++) {
                showSubtitleView(i, mSubtitleView[i]);
            }
        }

        if(mPGSCaptionEnable == true)
            showPGSView(PgsCaptionView);
    }

    // VIEW_SUBTITLE
    public void showSubtitleView(int index, View view) {
        for(int i=0; i<SUBTITLE_MAX_NUM; i++) {
            if (i < mShowSubtitle && mSubtitleEnable) {
                mSubtitleView[i].setVisibility(View.VISIBLE);
            } else {
                mSubtitleView[i].setVisibility(View.GONE);
            }
        }
        requestLayout();
        setVisibility(View.VISIBLE);
        setFocusable(false);
    }

    public void showPGSView(View view) {
        if (0 < mShowSubtitle && mPGSCaptionEnable) {
            PgsCaptionView.setVisibility(View.VISIBLE);
        } else {
            PgsCaptionView.setVisibility(View.GONE);
        }
        requestLayout();
        setVisibility(View.VISIBLE);
        setFocusable(false);
    }

    @Override
    public void show() {
        updateViews();
        setVisibility(View.VISIBLE);
        setFocusable(false);
    }

    @Override
    public void onClick(View view) {
        if (mListener != null) {
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (super.onTouchEvent(event)) {
            return true;
        }
        return false;
    }

    // The paddings of 4 sides which covered by system components. E.g.
    // +-----------------+\
    // | Action Bar | insets.top
    // +-----------------+/
    // | |
    // | Content Area | insets.right = insets.left = 0
    // | |
    // +-----------------+\
    // | Navigation Bar | insets.bottom
    // +-----------------+/
    // Please see View.fitSystemWindows() for more details.
    private final Rect mWindowInsets = new Rect();

    @Override
    protected boolean fitSystemWindows(Rect insets) {
        // We don't set the paddings of this View, otherwise,
        // the content will get cropped outside window
        mWindowInsets.set(insets);
        return true;
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        Rect insets = mWindowInsets;
        int pl = insets.left; // the left paddings
        int pr = insets.right;
        int pt = insets.top;
        int pb = insets.bottom;

        int h = bottom - top;
        int w = right - left;

        int y = h - pb;

        // VIEW_SUBTITLE
        if (mSubtitleEnable == true) 
        {
            int st = h;
            if (mTimeBar.getView().getVisibility() == View.VISIBLE && mTimeBar.getPreferredHeight() + pb > mSubtitlePosition) {
                st -= mTimeBar.getPreferredHeight() + pb;
            } else {
                st -= mSubtitlePosition;
            }
       
            for(int i=0; i<SUBTITLE_MAX_NUM; i++) {
                if (mSubtitleView[i] != null) {
                    st -= mSubtitleView[i].getMeasuredHeight();
                    mSubtitleView[i].layout(0, st , w, h);
                    mSubtitleView[i].requestLayout();
                }
            }
        }

        if(mPGSCaptionEnable == true)
        { 
            if (mTimeBar.getView().getVisibility() == View.VISIBLE) {
                mBitmapErease.eraseColor(0x00000000);
                PgsCaptionView.setImageBitmap(mBitmapErease);
               showPGSView(PgsCaptionView);
           }
        }

        mTimeBar.getView().layout(pl, y - mTimeBar.getPreferredHeight(), w - pr, y);

        // Needed, otherwise the framework will not re-layout in case only the
        // padding is changed
        //mTimeBar.getView().requestLayout();
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        measureChildren(widthMeasureSpec, heightMeasureSpec);
    }

    protected void updateViews() {
        mTimeBar.getView().setVisibility(View.VISIBLE);
        requestLayout();
    }

    // TimeBar listener

    @Override
    public void onScrubbingStart() {
        mListener.onSeekStart();
    }

    @Override
    public void onScrubbingMove(int time) {
        mListener.onSeekMove(time);
    }

    @Override
    public void onScrubbingEnd(int time, int trimStartTime, int trimEndTime) {
        mListener.onSeekEnd(time, trimStartTime, trimEndTime);
    }
}
