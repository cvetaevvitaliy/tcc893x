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
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.view.KeyEvent;
import android.view.View;
import android.view.LayoutInflater;
import android.widget.NumberPicker;
import android.widget.ImageButton;
import android.widget.FrameLayout;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.SeekBar;
import android.media.MediaSubtitle;
import android.os.Handler;
import android.os.Message;
import android.app.AlertDialog;
import android.util.AttributeSet;
import android.util.Log;

import java.util.ArrayList;

public class TCCMediaMenu extends FrameLayout {
    private static final String TAG = "TCCMediaMenu";
    private static final boolean DEBUG = false;

    private Context mContext;
    private TCCVideoView mVideoView;
	private TCCMediaController mController;
    private MediaSubtitle mMediaSubtitle;
    private View mRoot;
    private ImageButton mAudioButton;
    private ImageButton mSubtitleButton;
    private ImageButton mSubtitleSettingButton;
    private ImageButton mPlaySpeedButton;
    private ImageButton mScreenLayoutButton;
    private CharSequence[] mAudioList;
    private CharSequence[] mSubtitleList;
    private CharSequence[] mPlaySpeedList;
    private CharSequence[] mScreenLayoutList;
    private int mCurrentAudio;
    private int mCurrentSubtitle;
    private int mCurrentSubtitleSize;
    private int mCurrentSubtitleSync;
    private int mCurrentSubtitlePosition;
    private int mCurrentPlaySpeed;
    private int mCurrentScreenLayout;
    private TextView mNotification;
    private View mSubtitleSettingView;
	private View mSubtitleSizeView;
    private SeekBar mSubtitleSizeBar;
    private TextView mSubtitleSizeText;
    private View mTimeShiftView;
    private SeekBar mTimeShiftBar;
    private TextView mTimeShiftText;
    private View mPositionView;
    private SeekBar mPositionBar;
    private TextView mPositionText;
    private View mLastFocusView;

    private final int NOTI_MSG_HIDE = 1;
    private final int MENU_HIDE = 2;

    public TCCMediaMenu(Context context, AttributeSet attrs)  {
        super(context, attrs);
        mContext = context;
        addView(makeMenuView());
    }

    protected View makeMenuView() {
        LayoutInflater inflate = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mRoot = inflate.inflate(R.layout.tcc_media_menu, null);

        initMenu(mRoot);

        return mRoot;
    }

    SeekBar.OnFocusChangeListener mSubtitleSettingFocusChanged = new SeekBar.OnFocusChangeListener() {
        @Override
        public void onFocusChange(View v, boolean hasFocus) {
            SeekBar view = (SeekBar)v;
            if (view == mSubtitleSizeBar) {
                Log.e(TAG, "mSubtitleSizeBar " + hasFocus);
                if (hasFocus) mSubtitleSizeView.setBackgroundColor(0x5510aacc);
                else mSubtitleSizeView.setBackgroundColor(0x00000000);
            } else if (view == mTimeShiftBar) {
                if (hasFocus) mTimeShiftView.setBackgroundColor(0x5510aacc);
                else mTimeShiftView.setBackgroundColor(0x00000000);
            } else if (view == mPositionBar) {
                if (hasFocus) mPositionView.setBackgroundColor(0x5510aacc);
                else mPositionView.setBackgroundColor(0x00000000);
    	    }
        }
    };

    SeekBar.OnSeekBarChangeListener mSubtitleSizeListener = new SeekBar.OnSeekBarChangeListener() {
		@Override
		public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
		    String str = String.format("%d px", progress + 24);
            mSubtitleSizeText.setText(str);
		}
		@Override
		public void onStartTrackingTouch(SeekBar seekBar) {
		}
		@Override
		public void onStopTrackingTouch(SeekBar seekBar) {
		}
    };

    SeekBar.OnSeekBarChangeListener mTimeShiftListener = new SeekBar.OnSeekBarChangeListener() {
		@Override
		public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
		    String str = String.format("%.1f s", (float)(progress-600000)/1000);
            mTimeShiftText.setText(str);
		}
		@Override
		public void onStartTrackingTouch(SeekBar seekBar) {
		}
		@Override
		public void onStopTrackingTouch(SeekBar seekBar) {
		}
    };

    SeekBar.OnSeekBarChangeListener mPositionListener = new SeekBar.OnSeekBarChangeListener() {
		@Override
		public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
		    String str = String.format("%d px", progress);
            mPositionText.setText(str);
		}
		@Override
		public void onStartTrackingTouch(SeekBar seekBar) {
		}
		@Override
		public void onStopTrackingTouch(SeekBar seekBar) {
		}
    };

    public void setVideoView(TCCVideoView vv) {
        if (DEBUG) Log.i(TAG, "setVideoView");
        mVideoView = vv;
    }

    public void setMediaController(TCCMediaController mc) {
        if (DEBUG) Log.i(TAG, "setVideoView");
        mController = mc;
    }

    public void onPrepared() {
        if (DEBUG) Log.i(TAG, "setMediaPlayer");
        initValue();
        setEnable(true);
        setVisibility(View.GONE);
    }

    public void setNotificationView(TextView v) {
        mNotification = v;
    }

    private void setNotificationMessage(String text) {
        if (mNotification != null) {
            mNotification.setText(text);
            mNotification.setVisibility(View.VISIBLE);

            Message msg = mHandler.obtainMessage(NOTI_MSG_HIDE);
            mHandler.removeMessages(NOTI_MSG_HIDE);
            mHandler.sendMessageDelayed(msg, 3000);
        }
    }

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case NOTI_MSG_HIDE:
                    mNotification.setText("");
                    mNotification.setVisibility(View.GONE);
                    break;
                case MENU_HIDE:
                    hide();
                    break;
            }
        }
    };

    private void initMenu(View v) {
        if (DEBUG) Log.i(TAG, "initMenu");
        mAudioButton = (ImageButton) v.findViewById(R.id.audio_track);
        mAudioButton.setOnClickListener(mAudioListener);
        mSubtitleButton = (ImageButton) v.findViewById(R.id.subtitle);
        mSubtitleButton.setOnClickListener(mSubtitleListener);
        mSubtitleSettingButton = (ImageButton) v.findViewById(R.id.subtitle_setting);
        mSubtitleSettingButton.setOnClickListener(mSubtitleSettingListener);
        mPlaySpeedButton = (ImageButton) v.findViewById(R.id.play_speed);
        mPlaySpeedButton.setOnClickListener(mPlaySpeedListener);
        mScreenLayoutButton = (ImageButton) v.findViewById(R.id.screen_layout);
        mScreenLayoutButton.setOnClickListener(mScreenLayoutListener);
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        int keyCode = event.getKeyCode();
        final boolean uniqueDown = event.getRepeatCount() == 0
                && event.getAction() == KeyEvent.ACTION_DOWN;
        if (keyCode ==  KeyEvent.KEYCODE_MENU
			    || keyCode == KeyEvent.KEYCODE_BACK) {
            if (uniqueDown) {
                hide();
            }
            return true;
        }

        show();
        return super.dispatchKeyEvent(event);
    }

    public void toggle() {
        if (DEBUG) Log.i(TAG, "toggle");
        if (getVisibility() == View.VISIBLE) {
            mHandler.removeMessages(MENU_HIDE);
            hide();
        } else {
            show();
        }
    }

    public void show() {
        show(5000);
        if (mLastFocusView != null)
            mLastFocusView.requestFocus();
        mLastFocusView = null;
    }

    private void show(int time) {
        if (DEBUG) Log.i(TAG, "show");
        Message msg = mHandler.obtainMessage(MENU_HIDE);
        mHandler.removeMessages(MENU_HIDE);
        if (time != 0) {
            mHandler.sendMessageDelayed(msg, time);
        }
        if (mController != null) mController.Lock();
        if (getVisibility() == View.VISIBLE) {
            return;
        }
        setVisibility(View.VISIBLE);
        if (mAudioButton.isEnabled() == true) {
            mAudioButton.requestFocus();
        } else if (mSubtitleButton.isEnabled() == true) {
            mSubtitleButton.requestFocus();
        } else if (mSubtitleSettingButton.isEnabled() == true) {
            mSubtitleSettingButton.requestFocus();
        } else if (mPlaySpeedButton.isEnabled() == true) {
            mPlaySpeedButton.requestFocus();
        } else if (mScreenLayoutButton.isEnabled() == true) {
            mScreenLayoutButton.requestFocus();
        }
        if (mVideoView != null)
            mVideoView.showSystemUi(true);
    }

    public void hide() {
        if (DEBUG) Log.i(TAG, "hide");
        if (mController != null) setVisibility(View.INVISIBLE);
        mController.Unlock();
        if (mVideoView != null)
            mVideoView.showSystemUi(false);
    }

    private void initValue() {
        if (DEBUG) Log.i(TAG, "initValue");
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
        mCurrentAudio = 0;
        mCurrentSubtitle = prefs.getBoolean("pref_gallery_video_subtitle_key", true) ? 1 : 0;
        mCurrentSubtitleSize = Integer.parseInt(prefs.getString("pref_gallery_video_fontsize_key", "40"));
        mCurrentSubtitleSync = 0;
        mCurrentSubtitlePosition = 48;
        mCurrentPlaySpeed = 5;
        mCurrentScreenLayout = 2;

        mAudioList = null;
        mSubtitleList = null;

        getAudioTrackList();
        getSubtitleList();
        getPlaySpeedList();
        getScreenLayoutList();
    }

    private void setEnable(boolean enabled) {
        if (DEBUG) Log.i(TAG, "setEnable(enabled=" + enabled + ")");
        if (enabled && mVideoView == null) {
            return;
        }

        if (enabled && mVideoView.getTotalAudioTrackCount() > 1) {
            if (DEBUG) Log.i(TAG, "Audio Button is Enabled");
            mAudioButton.setEnabled(true);
        } else {
            mAudioButton.setEnabled(false);
        }
        if (enabled && mVideoView.getSubtitleType() > 0) {
            if (DEBUG) Log.i(TAG, "Subtitle Button is Enabled");
            mSubtitleButton.setEnabled(true);
            if (mVideoView.getSubtitleType() == 1)
                mSubtitleSettingButton.setEnabled(true);
            else
            mSubtitleSettingButton.setEnabled(false);
        } else {
            mSubtitleButton.setEnabled(false);
            mSubtitleSettingButton.setEnabled(false);
        }
        if (enabled && (mVideoView.canSeekForward() || mVideoView.canSeekBackward())) {
            if (DEBUG) Log.i(TAG, "PlaySpeed Button is Enabled");
            mPlaySpeedButton.setEnabled(true);
        } else {
            mPlaySpeedButton.setEnabled(false);
        }
        if (enabled) {
            if (DEBUG) Log.i(TAG, "ScreenMode Button is Enabled");
            mScreenLayoutButton.setEnabled(true);
        } else {
            mScreenLayoutButton.setEnabled(false);
        }
    }

    private DialogInterface.OnCancelListener mCancelListener = new DialogInterface.OnCancelListener() {
        @Override
        public void onCancel(DialogInterface dialog) {
            show();
        }
    };

    private DialogInterface.OnClickListener mCancelClickListener = new DialogInterface.OnClickListener() {
        @Override
        public void onClick(DialogInterface dialog, int whichButton) {
            show();
        }
    };

    private View.OnClickListener mAudioListener = new View.OnClickListener() {
        public void onClick(View v) {
            show(0);
            mLastFocusView = v;
            new AlertDialog.Builder(mContext)
                    .setTitle("AudioTrack")
                    .setPositiveButton("Cancel", mCancelClickListener)
                    .setSingleChoiceItems(getAudioTrackList(), getAudioTrack(),
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int whichButton) {
                                    dialog.cancel();
                                    setAudioTrack(whichButton);
                                    show();
                                }
                            })
                    .setOnCancelListener(mCancelListener)
                    .show();
//                    .getWindow()
//                    .setLayout(300, 350);
        }
    };

    private View.OnClickListener mSubtitleListener = new View.OnClickListener() {
        public void onClick(View v) {
            show(0);
            mLastFocusView = v;
            new AlertDialog.Builder(mContext)
                    .setTitle("Subtitle")
                    .setPositiveButton("Cancel", mCancelClickListener)
                    .setSingleChoiceItems(getSubtitleList(), getSubtitle(),
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int whichButton) {
                                    dialog.cancel();
                                    setSubtitle(whichButton);
                                    show();
                                }
                            })
                    .setOnCancelListener(mCancelListener)
                    .show();
//                    .getWindow()
//                    .setLayout(300, 350);
        }
    };

    private View.OnClickListener mSubtitleSettingListener = new View.OnClickListener() {
        public void onClick(View v) {
            show(0);
            mLastFocusView = v;
            LayoutInflater inflate = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            mSubtitleSettingView = inflate.inflate(R.layout.subtitle_setting, null);

            mSubtitleSizeView = mSubtitleSettingView.findViewById(R.id.size_view);
            mSubtitleSizeText = (TextView)mSubtitleSettingView.findViewById(R.id.size_value);
            mSubtitleSizeBar = (SeekBar)mSubtitleSettingView.findViewById(R.id.size_bar);
            mSubtitleSizeBar.setOnFocusChangeListener(mSubtitleSettingFocusChanged);
            mSubtitleSizeBar.setOnSeekBarChangeListener(mSubtitleSizeListener);
            mSubtitleSizeBar.setMax(56); // 24 ~ 40 px
            mSubtitleSizeBar.setKeyProgressIncrement(4);
            mSubtitleSizeBar.setProgress(getSubtitleSize() - 24);

            mTimeShiftView = mSubtitleSettingView.findViewById(R.id.sync_view);
            mTimeShiftText = (TextView)mSubtitleSettingView.findViewById(R.id.sync_value);		
            mTimeShiftBar = (SeekBar)mSubtitleSettingView.findViewById(R.id.sync_bar);
            mTimeShiftBar.setOnFocusChangeListener(mSubtitleSettingFocusChanged);
            mTimeShiftBar.setOnSeekBarChangeListener(mTimeShiftListener);
            mTimeShiftBar.setMax(1200000); // -600.0 ~ 600.0 s
            mTimeShiftBar.setKeyProgressIncrement(100);
            mTimeShiftBar.setProgress(getSubtitleSync() + 600000);

            mPositionView = mSubtitleSettingView.findViewById(R.id.position_view);
            mPositionText = (TextView)mSubtitleSettingView.findViewById(R.id.position_value);
            mPositionBar = (SeekBar)mSubtitleSettingView.findViewById(R.id.position_bar);
            mPositionBar.setOnFocusChangeListener(mSubtitleSettingFocusChanged);
            mPositionBar.setOnSeekBarChangeListener(mPositionListener);
            mPositionBar.setMax(300); // 0 ~ 300
            mPositionBar.setKeyProgressIncrement(1);
            mPositionBar.setProgress(getSubtitlePosition());

            new AlertDialog.Builder(mContext)
                    .setTitle("Subtitle Settings")
                    .setView(mSubtitleSettingView)
                    .setPositiveButton("Cancel", mCancelClickListener)
                    .setNeutralButton("OK",
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int whichButton) {
                                    dialog.cancel();
                                    setSubtitleSize(mSubtitleSizeBar.getProgress() + 24);
                                    setSubtitleSync(mTimeShiftBar.getProgress() - 600000);
                                    setSubtitlePosition(mPositionBar.getProgress());
                                    show();
                                }
                            })
                    .setOnCancelListener(mCancelListener)
                    .show();
//                    .getWindow()
//                    .setLayout(500, 350);
        }
    };

    private View.OnClickListener mPlaySpeedListener = new View.OnClickListener() {
        public void onClick(View v) {
            show(0);
            mLastFocusView = v;
            new AlertDialog.Builder(mContext)
                    .setTitle("Play Speed")
                    .setPositiveButton("Cancel", mCancelClickListener)
                    .setSingleChoiceItems(getPlaySpeedList(), getPlaySpeed(),
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int whichButton) {
                                    dialog.cancel();
                                    setPlaySpeed(whichButton);
                                    show();
                                }
                            })
                    .setOnCancelListener(mCancelListener)
                    .show();
//                    .getWindow()
//                    .setLayout(300, 350);
        }
    };

    private View.OnClickListener mScreenLayoutListener = new View.OnClickListener() {
        public void onClick(View v) {
            show(0);
            mLastFocusView = v;
            new AlertDialog.Builder(mContext)
                    .setTitle("Screen Mode")
                    .setPositiveButton("Cancel", mCancelClickListener)
                    .setSingleChoiceItems(getScreenLayoutList(), getScreenLayout(),
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int whichButton) {
                                    dialog.cancel();
                                    setScreenLayout(whichButton);
                                    show();
                                }
                            })
                    .setOnCancelListener(mCancelListener)
                    .show();
//                    .getWindow()
//                    .setLayout(300, 350);
        }
    };

    private CharSequence[] getAudioTrackList() {
        if (DEBUG) Log.i(TAG, "getAudioTrackList");
        if (mAudioList == null) {
            int totalAudioTrack = mVideoView.getTotalAudioTrackCount();
            ArrayList<CharSequence> audioList = new ArrayList<CharSequence>();
            for (int i = 1; i <= totalAudioTrack; i++) {
                audioList.add("AudioTrack #" + i);
            }
            mAudioList = audioList.toArray(new CharSequence[audioList.size()]);
            audioList.clear();
        }
        return mAudioList;
    }

    private CharSequence[] getSubtitleList() {
        if (DEBUG) Log.i(TAG, "getSubtitleList");
        if (mSubtitleList == null) {
			mMediaSubtitle = new MediaSubtitle();
			mVideoView.getSubtitleClass(mMediaSubtitle);
			int iSubtitleClassNum = mMediaSubtitle.mSubtitleClassNum;
            ArrayList<CharSequence> subtitleList = new ArrayList<CharSequence>();
            subtitleList.add("Off");
            if (iSubtitleClassNum == 0) {
                subtitleList.add("On");
            }
            for (int i = 1; i <= iSubtitleClassNum; i++) {
                subtitleList.add("Subtitle #" + i + "(" + mMediaSubtitle.getSubtitleClassName(i-1) + ")");
            }
            mSubtitleList = subtitleList.toArray(new CharSequence[subtitleList.size()]);
            subtitleList.clear();
        }
        return mSubtitleList;
    }

    private CharSequence[] getPlaySpeedList() {
        if (DEBUG) Log.i(TAG, "getPlaySpeedList");
        if (mPlaySpeedList == null) {
            mPlaySpeedList = new CharSequence[] {"-32x", "-16x", "-8x", "-4x", "-2x", "1x", "2x", "4x", "8x", "16x", "32x"};
        }
        return mPlaySpeedList;
    }

    private CharSequence[] getScreenLayoutList() {
        if (DEBUG) Log.i(TAG, "getScreenLayoutList");
        if (mScreenLayoutList == null) {
            mScreenLayoutList = new CharSequence[] {"Box", "Pan/Scan", "Full Screen"};
        }
        return mScreenLayoutList;
    }

    public int getAudioTrack() {
        if (DEBUG) Log.i(TAG, "getAudioTrack");
        return mCurrentAudio;
    }

    public int getSubtitle() {
        if (DEBUG) Log.i(TAG, "getSubtitle");
        return mCurrentSubtitle;
    }

    public int getSubtitleSize() {
        if (DEBUG) Log.i(TAG, "getSubtitleSize");
        return mCurrentSubtitleSize;
    }

    public int getSubtitleSync() {
        if (DEBUG) Log.i(TAG, "getSubtitleSync");
        return mCurrentSubtitleSync;
    }

    public int getSubtitlePosition() {
        if (DEBUG) Log.i(TAG, "getSubtitlePosition");
        return mCurrentSubtitlePosition;
    }

    public int getPlaySpeed() {
        if (DEBUG) Log.i(TAG, "getPlaySpeed");
        return mCurrentPlaySpeed;
    }

    public int getScreenLayout() {
        if (DEBUG) Log.i(TAG, "getScreenLayout");
        return mCurrentScreenLayout;
    }

    public void setAudioTrack(int which) {
        if (DEBUG) Log.d(TAG, "setAudioTrack(" + which + ")");
        if (which != getAudioTrack()) {
            boolean ret = mVideoView.setAudioTrack(which);
            mCurrentAudio = which;
            if (ret) {
                setNotificationMessage("Audio Track #" + (which + 1));
            }
        }
    }

    public void setSubtitle(int which) {
        if (DEBUG) Log.d(TAG, "setSubtitle(" + which + ")");
        if (which != mCurrentSubtitle) {
            if (which == 0) {
                mVideoView.changeSubtitle(which - 1);
                setNotificationMessage("Subtitle OFF");
            } else {
                mVideoView.changeSubtitle(which - 1);
                if (mMediaSubtitle.mSubtitleClassNum > 1) {
                    setNotificationMessage("Subtitle #" + (which - 1));
                } else {
                    setNotificationMessage("Subtitle ON");
                }
            }
            mCurrentSubtitle = which;
        }
    }

    public void setSubtitleSize(int which) {
        if (DEBUG) Log.d(TAG, "setSubtitleSize(" + which + ")");
        if (which != mCurrentSubtitleSize) {
            mVideoView.setSubtitleSize(which);
            setNotificationMessage("Subtitle Size #" + which);
            mCurrentSubtitleSize = which;
        }
    }

    public void setSubtitleSync(int which) {
        if (DEBUG) Log.d(TAG, "setSubtitleSync(" + which + ")");
        if (which != mCurrentSubtitleSync) {
            mVideoView.setTimeShiftPTS(which - mCurrentSubtitleSync);
            mCurrentSubtitleSync = which;
        }
    }

    public void setSubtitlePosition(int which) {
        if (DEBUG) Log.d(TAG, "setSubtitlePosition(" + which + ")");
        if (which != mCurrentSubtitlePosition) {
            mVideoView.setSubtitlePosition(which);
            mCurrentSubtitlePosition = which;
        }
    }

    public void setPlaySpeed(int which) {
        if (DEBUG) Log.d(TAG, "setPlaySpeed(" + which + ")");
        if (which != mCurrentPlaySpeed) {
            mVideoView.setPlaySpeed(which);
            setNotificationMessage(getPlaySpeedList()[which].toString());
            mCurrentPlaySpeed = which;
        }
    }

    public void setScreenLayout(int which) {
        if (DEBUG) Log.d(TAG, "setScreenLayout(" + which + ")");
        if (which != mCurrentScreenLayout) {
            switch(which) {
            case 0:
                mVideoView.setBoxViewMode();
                break;
            case 1:
                mVideoView.setPanScanViewMode();
                break;
            case 2:
                mVideoView.setFullViewMode();
                break;
            }
            setNotificationMessage(getScreenLayoutList()[which].toString());
            mCurrentScreenLayout = which;
        }
    }
}
