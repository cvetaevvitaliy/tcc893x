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

import android.content.Context;
import android.view.View;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.widget.ImageButton;
import android.widget.TextView;
import android.os.Handler;
import android.view.animation.Animation;
import android.view.animation.Animation.AnimationListener;
import android.view.animation.AnimationUtils;


/**
 * The playback controller for the Movie Player.
 */
public class TCCMediaController extends CommonControllerOverlay implements
        AnimationListener {

    private ImageButton         mPauseButton;
    private ImageButton         mFfwdButton;
    private ImageButton         mRewButton;
    private ImageButton         mNextButton;
    private ImageButton         mPrevButton;

    private boolean             mUseFastForward;

    private String mTitle;
    private TextView mTitleView;

    private boolean hidden;

    private final Handler handler;
    private final Runnable startHidingRunnable;
    private final Animation hideAnimation;

    public TCCMediaController(Context context) {
        super(context);

        handler = new Handler();
        startHidingRunnable = new Runnable() {
            @Override
            public void run() {
                startHiding();
            }
        };

        hideAnimation = AnimationUtils.loadAnimation(context, R.anim.player_out);
        hideAnimation.setAnimationListener(this);

        hide();
    }

    protected void createTimeBar(Context context) {
        mTimeBar = new TimeBar(context, this);

        mUseFastForward = true;

        mPauseButton = (ImageButton) mTimeBar.getView().findViewById(R.id.pause);
        if (mPauseButton != null) {
            mPauseButton.requestFocus();
            mPauseButton.setOnClickListener(this);
        }

        mFfwdButton = (ImageButton) mTimeBar.getView().findViewById(R.id.ffwd);
        if (mFfwdButton != null) {
            mFfwdButton.setOnClickListener(this);
            mFfwdButton.setVisibility(mUseFastForward ? View.VISIBLE : View.GONE);
        }

        mRewButton = (ImageButton) mTimeBar.getView().findViewById(R.id.rew);
        if (mRewButton != null) {
            mRewButton.setOnClickListener(this);
            mRewButton.setVisibility(mUseFastForward ? View.VISIBLE : View.GONE);
        }

        mNextButton = (ImageButton) mTimeBar.getView().findViewById(R.id.next);
        if (mNextButton != null) {
            mNextButton.setOnClickListener(this);
        }

        mPrevButton = (ImageButton) mTimeBar.getView().findViewById(R.id.prev);
        if (mPrevButton != null) {
            mPrevButton.setOnClickListener(this);
        }

        mTitleView = (TextView) mTimeBar.getView().findViewById(R.id.title_current);
    }

    @Override
    public void hide() {
        boolean wasHidden = hidden;
        hidden = true;
        super.hide();
        if (mListener != null && wasHidden != hidden) {
            mListener.onHidden();
        }
    }

    @Override
    public void show() {
        boolean wasHidden = hidden;
        hidden = false;
        super.show();
        if (mListener != null && wasHidden != hidden) {
            mPauseButton.requestFocus();
            mListener.onShown();
        }
        maybeStartHiding();
    }

    public boolean isShowing() {
        return !hidden;
    }

    public void Lock() {
    }

    public void Unlock() {
    }
    
    private void maybeStartHiding() {
        cancelHiding();
        if (mState == State.PLAYING) {
            handler.postDelayed(startHidingRunnable, 2500);
        }
    }

    private void startHiding() {
        startHideAnimation(mTimeBar.getView());
    }

    private void startHideAnimation(View view) {
        if (view.getVisibility() == View.VISIBLE) {
            view.startAnimation(hideAnimation);
        }
    }

    private void cancelHiding() {
        handler.removeCallbacks(startHidingRunnable);
        mTimeBar.getView().setAnimation(null);
    }

    @Override
    public void onAnimationStart(Animation animation) {
        // Do nothing.
    }

    @Override
    public void onAnimationRepeat(Animation animation) {
        // Do nothing.
    }

    @Override
    public void onAnimationEnd(Animation animation) {
        hide();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (hidden) {
            show();
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (super.onTouchEvent(event)) {
            return true;
        }
/*
        if (event.getAction() == MotionEvent.ACTION_UP) {
            if (hidden) {
                show();
            } else {
                hide();
            }
        }
        return true;
*/
        return false;
    }

    @Override
    protected void updateViews() {
        if (hidden) {
            return;
        }
        super.updateViews();
    }

    public void setTitle(String str) {
        if (str == null) {
            mTitleView.setText("");
        } else {
            if (str.length() > 40) {
                mTitle = str.substring(0, 37) + "...";
            } else {
                mTitle = str;
            }
            mTitleView.setText(mTitle);
        }
    }
    
    // TimeBar listener

    @Override
    public void onScrubbingStart() {
        cancelHiding();
        super.onScrubbingStart();
    }

    @Override
    public void onScrubbingMove(int time) {
        cancelHiding();
        super.onScrubbingMove(time);
    }

    @Override
    public void onScrubbingEnd(int time, int trimStartTime, int trimEndTime) {
        maybeStartHiding();
        super.onScrubbingEnd(time, trimStartTime, trimEndTime);
    }
    
    // Controller
    
    private void updatePausePlay() {
        if (mPauseButton == null)
            return;

        if (mState == State.PLAYING) {
            mPauseButton.setImageResource(R.drawable.ic_media_pause);
        } else {
            mPauseButton.setImageResource(R.drawable.ic_media_play);
        }
    }

    @Override
    public void onClick(View view) {
        switch(view.getId()) {
        case R.id.pause:
            if (mListener != null)
                mListener.onPlayPause();
            updatePausePlay();
            break;

        case R.id.rew:
            if (mListener != null)
                mListener.onRewind();
            break;

        case R.id.ffwd:
            if (mListener != null)
                mListener.onFastForward();
            break;

        case R.id.prev:
            if (mListener != null)
                mListener.onPrevPlay();
            break;

        case R.id.next:
            if (mListener != null)
                mListener.onNextPlay();
            break;
        }
        maybeStartHiding();
    }
}
