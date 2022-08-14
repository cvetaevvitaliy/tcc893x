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

package com.android.gallery3d.app;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Rect;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.ImageView.ScaleType;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.TextView;
// VIEW_SUBTITLE
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.graphics.Bitmap;

import com.android.gallery3d.R;

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

    private static final float ERROR_MESSAGE_RELATIVE_PADDING = 1.0f / 6;

    protected Listener mListener;

    protected final View mBackground;
    protected TimeBar mTimeBar;

    protected View mMainView;
    protected final LinearLayout mLoadingView;
    protected final TextView mErrorView;
    protected final ImageView mPlayPauseReplayView;

    // VIEW_SUBTITLE
    protected boolean mSubtitleEnable = false;
    protected boolean mPGSCaptionEnable = false;
 
    protected static final int SUBTITLE_MAX_NUM = 2;
    protected TextView[] subtitleView;

    protected Bitmap	mBitmap;
    protected Bitmap	mReSizedBitmap;
    protected Bitmap	mBitmapErease;

    protected int mPGSBitMap_Width;
    protected int mPGSBitMap_Height;
    protected final ImageView PgsCaptionView;

    protected State mState;

    protected boolean mCanReplay = true;

    public void setSeekable(boolean canSeek) {
        mTimeBar.setSeekable(canSeek);
    }

    public CommonControllerOverlay(Context context) {
        super(context);

        mState = State.LOADING;
        // TODO: Move the following layout code into xml file.
        LayoutParams wrapContent =
                new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
        LayoutParams matchParent =
                new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);

        mBackground = new View(context);
        mBackground.setBackgroundColor(context.getResources().getColor(R.color.darker_transparent));
        addView(mBackground, matchParent);

        // Depending on the usage, the timeBar can show a single scrubber, or
        // multiple ones for trimming.
        createTimeBar(context);
        addView(mTimeBar, wrapContent);
        mTimeBar.setContentDescription(
                context.getResources().getString(R.string.accessibility_time_bar));
        mLoadingView = new LinearLayout(context);
        mLoadingView.setOrientation(LinearLayout.VERTICAL);
        mLoadingView.setGravity(Gravity.CENTER_HORIZONTAL);
        ProgressBar spinner = new ProgressBar(context);
        spinner.setIndeterminate(true);
        mLoadingView.addView(spinner, wrapContent);
        TextView loadingText = createOverlayTextView(context);
        loadingText.setText(R.string.loading_video);
        mLoadingView.addView(loadingText, wrapContent);
        addView(mLoadingView, wrapContent);

        mPlayPauseReplayView = new ImageView(context);
        mPlayPauseReplayView.setImageResource(R.drawable.ic_vidcontrol_play);
        mPlayPauseReplayView.setContentDescription(
                context.getResources().getString(R.string.accessibility_play_video));
        mPlayPauseReplayView.setBackgroundResource(R.drawable.bg_vidcontrol);
        mPlayPauseReplayView.setScaleType(ScaleType.CENTER);
        mPlayPauseReplayView.setFocusable(true);
        mPlayPauseReplayView.setClickable(true);
        mPlayPauseReplayView.setOnClickListener(this);
        addView(mPlayPauseReplayView, wrapContent);

        mErrorView = createOverlayTextView(context);
        addView(mErrorView, matchParent);

        // VIEW_SUBTITLE
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        int fontsize = Integer.parseInt(prefs.getString("pref_video_settings_subtitle_fontsize_key", "32"));
        subtitleView =  new TextView[SUBTITLE_MAX_NUM];
        for(int i=0; i<SUBTITLE_MAX_NUM; i++) {
            subtitleView[i] = new TextView(context);
            subtitleView[i].setText(" ");

            subtitleView[i].setGravity(Gravity.CENTER);
            subtitleView[i].setBackgroundColor(0x00000000);
 
            subtitleView[i].setShadowLayer((float)1.2, (float)1.2, (float)1.2, (int)0xFFFFFF);
            subtitleView[i].setTextSize(fontsize);
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

    private TextView createOverlayTextView(Context context) {
        TextView view = new TextView(context);
        view.setGravity(Gravity.CENTER);
        view.setTextColor(0xFFFFFFFF);
        view.setPadding(0, 15, 0, 15);
        return view;
    }

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
        showMainView(mPlayPauseReplayView);
    }

    @Override
    public void showPaused() {
        mState = State.PAUSED;
        showMainView(mPlayPauseReplayView);
    }

    @Override
    public void showEnded() {
        mState = State.ENDED;
        if (mCanReplay) showMainView(mPlayPauseReplayView);
    }

    @Override
    public void showLoading() {
        mState = State.LOADING;
        showMainView(mLoadingView);
    }

    public void toggleLoadingViewVisibility(boolean isLoading) {
        if (isLoading) {
            showLoading();
        } else {
            mState = State.PLAYING;
            hide();
        }
    }

    @Override
    public void showErrorMessage(String message) {
        mState = State.ERROR;
        int padding = (int) (getMeasuredWidth() * ERROR_MESSAGE_RELATIVE_PADDING);
        mErrorView.setPadding(
                padding, mErrorView.getPaddingTop(), padding, mErrorView.getPaddingBottom());
        mErrorView.setText(message);
        showMainView(mErrorView);
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
                addView(subtitleView[i], new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT));
            }
            mSubtitleEnable = true;
        }

        subtitleView[index].setText(subtitle);
        showSubtitleView(index, subtitleView[index]);
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
        mPlayPauseReplayView.setVisibility(View.INVISIBLE);
        mLoadingView.setVisibility(View.INVISIBLE);
        mBackground.setVisibility(View.INVISIBLE);
        mTimeBar.setVisibility(View.INVISIBLE);
        setVisibility(View.INVISIBLE);
        setFocusable(true);
        requestFocus();
    }

    private void showMainView(View view) {
        mMainView = view;
        mErrorView.setVisibility(mMainView == mErrorView ? View.VISIBLE : View.INVISIBLE);
        mLoadingView.setVisibility(mMainView == mLoadingView ? View.VISIBLE : View.INVISIBLE);
        mPlayPauseReplayView.setVisibility(
                mMainView == mPlayPauseReplayView ? View.VISIBLE : View.INVISIBLE);
        show();
    }

    // VIEW_SUBTITLE
    public void showSubtitleView(int index, View view) {
        mMainView = null; 
        for(int i=0; i<SUBTITLE_MAX_NUM; i++) {
            subtitleView[i].setVisibility(View.VISIBLE);
        }
        requestLayout();
        setVisibility(View.VISIBLE);
        setFocusable(false);

        //show();
    }

    public void showPGSView(View view) {
        mMainView = null; 
        PgsCaptionView.setVisibility(View.VISIBLE);
        requestLayout();
        setVisibility(View.VISIBLE);
        setFocusable(false);
        //show();
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
            if (view == mPlayPauseReplayView) {
                if (mState == State.ENDED) {
                    if (mCanReplay) {
                        mListener.onReplay();
                    }
                } else if (mState == State.PAUSED || mState == State.PLAYING) {
                    mListener.onPlayPause();
                }
            }
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
        boolean error = mErrorView.getVisibility() == View.VISIBLE;

        int y = h - pb;
        // Put both TimeBar and Background just above the bottom system
        // component.
        // But extend the background to the width of the screen, since we don't
        // care if it will be covered by a system component and it looks better.
        mBackground.layout(0, y - mTimeBar.getBarHeight(), w, y);

        // VIEW_SUBTITLE
        if (mSubtitleEnable == true) 
        {
            int st = y;
            if (mTimeBar.getVisibility() == View.VISIBLE) {
                st -= mTimeBar.getPreferredHeight();
            }
       
            for(int i=0; i<SUBTITLE_MAX_NUM; i++) {
                if (subtitleView[i] != null) {
                    st -= subtitleView[i].getMeasuredHeight();
                    subtitleView[i].layout(0, st , w, y);
                    subtitleView[i].requestLayout();
                }
            }
        }

        if(mPGSCaptionEnable == true)
        { 
            if (mTimeBar.getVisibility() == View.VISIBLE) {
                mBitmapErease.eraseColor(0x00000000);
                PgsCaptionView.setImageBitmap(mBitmapErease);
               showPGSView(PgsCaptionView);
           }
        }

        mTimeBar.layout(pl, y - mTimeBar.getPreferredHeight(), w - pr, y);

        // Put the play/pause/next/ previous button in the center of the screen
        layoutCenteredView(mPlayPauseReplayView, 0, 0, w, h);

        if (mMainView != null) {
            layoutCenteredView(mMainView, 0, 0, w, h);
        }
    }

    private void layoutCenteredView(View view, int l, int t, int r, int b) {
        int cw = view.getMeasuredWidth();
        int ch = view.getMeasuredHeight();
        int cl = (r - l - cw) / 2;
        int ct = (b - t - ch) / 2;
        view.layout(cl, ct, cl + cw, ct + ch);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        measureChildren(widthMeasureSpec, heightMeasureSpec);
    }

    protected void updateViews() {
        if (mState == State.LOADING) {
            mBackground.setVisibility(View.INVISIBLE);
            mTimeBar.setVisibility(View.INVISIBLE);
        } else {
            mBackground.setVisibility(View.VISIBLE);
            mTimeBar.setVisibility(View.VISIBLE);
        }
        Resources resources = getContext().getResources();
        int imageResource = R.drawable.ic_vidcontrol_reload;
        String contentDescription = resources.getString(R.string.accessibility_reload_video);
        if (mState == State.PAUSED) {
            imageResource = R.drawable.ic_vidcontrol_play;
            contentDescription = resources.getString(R.string.accessibility_play_video);
        } else if (mState == State.PLAYING) {
            imageResource = R.drawable.ic_vidcontrol_pause;
            contentDescription = resources.getString(R.string.accessibility_pause_video);
        }

        mPlayPauseReplayView.setImageResource(imageResource);
        mPlayPauseReplayView.setContentDescription(contentDescription);
        mPlayPauseReplayView.setVisibility(
                (mState != State.LOADING && mState != State.ERROR &&
                !(mState == State.ENDED && !mCanReplay))
                ? View.VISIBLE : View.GONE);
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
