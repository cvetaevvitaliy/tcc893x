package com.android.videoplayer;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.TextView;

public class TimeBar implements
        SeekBar.OnSeekBarChangeListener {

    public interface Listener {
        void onScrubbingStart();

        void onScrubbingMove(int time);

        void onScrubbingEnd(int time, int start, int end);
    }

    protected Listener mListener;

    private ProgressBar mProgress;

    private boolean mScrubbing;

    private int mScrubbingTime;
    private int mCurrentTime;
    private int mTotalTime;

    private View mRoot;

    private TextView mTextEndTime;
    private TextView mTextCurrTime;

    public TimeBar(Context context, Listener listener) {
        LayoutInflater inflate = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mRoot =  inflate.inflate(R.layout.tcc_media_controller, null);

        mListener = listener;

        mProgress = (ProgressBar) mRoot.findViewById(R.id.mediacontroller_progress);
        if (mProgress != null) {
            if (mProgress instanceof SeekBar) {
                SeekBar seeker = (SeekBar) mProgress;
                seeker.setOnSeekBarChangeListener(this);
            }
            mProgress.setMax(1000);
        }

        mTextEndTime = (TextView) mRoot.findViewById(R.id.time);
        mTextCurrTime = (TextView) mRoot.findViewById(R.id.time_current);
    }

    public View getView() {
    	return mRoot;
    }
    
    public int getPreferredHeight() {
        return mRoot.getMeasuredHeight();
    }

    public void setTime(int currentTime, int totalTime, int trimStartTime,
            int trimEndTime) {
        if (mCurrentTime == currentTime && mTotalTime == totalTime) {
            return;
        }
        mCurrentTime = currentTime;
        mTotalTime = totalTime;
        update();
    }

    private void update() {
        if (mProgress != null) {
            if (mTotalTime > 0 && !mScrubbing) {
                // use long to avoid overflow
                long pos = 1000L * mCurrentTime / mTotalTime;
                mProgress.setProgress( (int) pos);
            }
        }

        if (mTextEndTime != null)
            mTextEndTime.setText(stringForTime(mTotalTime));
        if (mTextCurrTime != null)
        	mTextCurrTime.setText(stringForTime(mCurrentTime));
        mRoot.requestLayout();
    }

    private String stringForTime(long timeMs) {
        int totalSeconds = (int) timeMs / 1000;

        int seconds = totalSeconds % 60;
        int minutes = (totalSeconds / 60) % 60;
        int hours   = totalSeconds / 3600;
        if (hours > 0) {
            return String.format("%d:%02d:%02d", hours, minutes, seconds).toString();
        } else {
            return String.format("%02d:%02d", minutes, seconds).toString();
        }
    }

    public void onStartTrackingTouch(SeekBar bar) {
    	mScrubbing = true;
    	mListener.onScrubbingStart();
    }

    public void onProgressChanged(SeekBar bar, int progress, boolean fromuser) {
        if (!fromuser) {
            // We're not interested in programmatically generated changes to
            // the progress bar's position.
            return;
        }
        long duration = mTotalTime;
        mScrubbingTime = (int)((duration * progress) / 1000L);
        if (mScrubbing)
	        mListener.onScrubbingMove(mScrubbingTime);
        else
            mListener.onScrubbingEnd(mScrubbingTime, 0, 0);
        if (mTextCurrTime != null)
            mTextCurrTime.setText(stringForTime(mScrubbingTime));
        mRoot.requestLayout();
    }

    public void onStopTrackingTouch(SeekBar bar) {
        mListener.onScrubbingEnd(mScrubbingTime, 0, 0);
        mScrubbing = false;
    }
}
