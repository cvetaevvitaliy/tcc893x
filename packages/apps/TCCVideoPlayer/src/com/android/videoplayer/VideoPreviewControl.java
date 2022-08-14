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

import android.app.Activity;
import android.content.Context;
import android.net.Uri;
import android.os.Message;
import android.os.Handler;
import android.view.View;
import android.widget.TextView;
import android.text.format.Formatter;
import android.util.Log;

import java.io.File;

public class VideoPreviewControl implements MoviePlayer.Listener {
    private static final String TAG = "VideoPreviewControl";
	
	private static final int	SHOW_PREVIEW = 1;
	private static final int	VISIBLE_PREVIEW = 2;
	private final int ONE_SECOND = 1000;
	private final int NOT_PLAYABLE_VIDEO_DURATION = 3*ONE_SECOND;
	private final int VISIBLE_DELAY = 50;

    private MoviePlayer mPlayer = null;
	private boolean mFinishOnCompletion;
	
	private final Context mContext;
	private final View mPreView;
	private final View mCannotPlayView;
	private final TextView mTitleText;
	private final TextView mSizeText;
    private final TextView mTimeInfo;
	
    public VideoPreviewControl(View rootView, Context context) {
		mContext = context;
		mPreView = (View) rootView.findViewById(R.id.preview);
        mTimeInfo = (TextView) mPreView.findViewById(R.id.time_info);
		mCannotPlayView = rootView.findViewById(R.id.cannot_play_movie);
		mTitleText = (TextView) mCannotPlayView.findViewById(R.id.title_view);
		mSizeText = (TextView) mCannotPlayView.findViewById(R.id.size_view);
    }

	private void startPreview(File file) {
        Uri videoUri = Uri.fromFile(file);
        mFinishOnCompletion = false;
		mPlayer = new MoviePlayer(mPreView, mContext, videoUri, null, true) {
			@Override
			public void onCompletion() {
				if(mFinishOnCompletion){
					((Activity)mContext).finish();
				}
			}
		
			@Override
			public void onError() {
				Log.i(TAG, "onError - mPlayer " + mPlayer );
				mPreviewHandler.removeMessages(SHOW_PREVIEW);
				if(mPlayer != null) {
					mPlayer.stopVideo();
					mFinishOnCompletion = mPlayer.checkApplicationFinish();
					mPlayer = null;
				}
			}

            @Override
            public void onPrepared() {
                Message msg = mPreviewHandler.obtainMessage(VISIBLE_PREVIEW);
                mPreviewHandler.removeMessages(VISIBLE_PREVIEW);
                mPreviewHandler.sendMessageDelayed(msg, VISIBLE_DELAY);
				mPlayer.onResume();
            }
		};
		mPlayer.setTimeListener(this);
    }

	private Handler mPreviewHandler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
				case SHOW_PREVIEW: 
					mCannotPlayView.setVisibility(View.GONE);
					startPreview((File)msg.obj);
					break;

				case VISIBLE_PREVIEW:
					mPreView.setVisibility(View.VISIBLE);
					break;

				default:
					break;
			}
		}
	};

    @Override
    public void setTimes(int currentTime, int totalTime) {
		mTimeInfo.setText(stringForTime(currentTime) + " / " + stringForTime(totalTime));
    }

    private String stringForTime(int timeMs) {
        int totalSeconds = timeMs / 1000;
        int seconds = totalSeconds % 60;
        int minutes = (totalSeconds / 60) % 60;
        int hours   = totalSeconds / 3600;
        if (hours > 0) {
            return String.format("%d:%02d:%02d", hours, minutes, seconds);
        } else {
            return String.format("%02d:%02d", minutes, seconds);
        }
    }

    public void preprocessPreview(File file) {
		if(mPlayer != null) {
            stopCurrentVideo();
		}
        Message msg = mPreviewHandler.obtainMessage(SHOW_PREVIEW,(Object)file);
        mPreviewHandler.removeMessages(SHOW_PREVIEW);
        mPreviewHandler.sendMessageDelayed(msg, NOT_PLAYABLE_VIDEO_DURATION);
	}
	
	public void stopCurrentVideo() {
		mPreView.setVisibility(View.INVISIBLE);
		mPreviewHandler.removeMessages(SHOW_PREVIEW);
        if(mPlayer != null) {
			mPlayer.stopVideo();
			mPlayer = null;
			
		}
    }
	
	public void displayInfo(File file) {
        if (file.isDirectory()) {
            //String size_str = Formatter.formatFileSize(mContext, getDirSize(file.getPath()));
            mTitleText.setText("./" + file.getName());
            //mSizeText.setText( mContext.getString(R.string.details_folder_size)  + size_str  );
            mSizeText.setText("");
        } else {
            String size_str = Formatter.formatFileSize(mContext, file.length());
            mTitleText.setText(file.getName());
            mSizeText.setText(mContext.getString(R.string.details_file_size) + size_str);
        }
		mCannotPlayView.setVisibility(View.VISIBLE);
		return ;
	}

	private long getDirSize(String path) {
		long result = 0;
		File f = new File(path);
		File[] files = f.listFiles();
        if (files == null) {
            return 0;
        }
		for(int i=0; i < files.length; i++) {
			File file = files[i];
			if(file.isDirectory()){
				result += getDirSize(file.getPath());
			} else {
				result += file.length();
			}
		}
		return result;
	}
}
