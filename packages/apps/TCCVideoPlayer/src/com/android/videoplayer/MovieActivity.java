/*
 * Copyright (C) 2007 The Android Open Source Project
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

import com.android.videoplayer.util.ImageManager;

import android.app.Instrumentation;
import android.app.Activity;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.media.AudioManager;
import android.os.Bundle;
import android.provider.MediaStore;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.InputDevice;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

/**
 * This activity plays a video from a specified URI.
 */
public class MovieActivity extends Activity {
    @SuppressWarnings("unused")
    private static final String TAG = "MovieActivity";

    private MoviePlayer mPlayer;
    private boolean mFinishOnCompletion;
    private ImageManager.ImageListParam mImageList;

    public static final String KEY_IMAGE_LIST = "image_list";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.movie_view);
        View rootView = findViewById(R.id.root);
        Intent intent = getIntent();
        mFinishOnCompletion = intent.getBooleanExtra(
                MediaStore.EXTRA_FINISH_ON_COMPLETION, true);
        mImageList = intent.getParcelableExtra(KEY_IMAGE_LIST);
        mPlayer = new MoviePlayer(rootView, this, intent.getData(), mImageList, savedInstanceState,
                !mFinishOnCompletion) {
            @Override
            public void onCompletion() {
                if (mFinishOnCompletion) {
                    finish();
                }
            }
        };
        if (intent.hasExtra(MediaStore.EXTRA_SCREEN_ORIENTATION)) {
            int orientation = intent.getIntExtra(
                    MediaStore.EXTRA_SCREEN_ORIENTATION,
                    ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
            if (orientation != getRequestedOrientation()) {
                setRequestedOrientation(orientation);
            }
        }
        Window win = getWindow();
        WindowManager.LayoutParams winParams = win.getAttributes();
        winParams.buttonBrightness = WindowManager.LayoutParams.BRIGHTNESS_OVERRIDE_OFF;
        winParams.flags |= WindowManager.LayoutParams.FLAG_FULLSCREEN;
        win.setAttributes(winParams);
    }

    @Override
    public void onStart() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_EJECT);
        intentFilter.addDataScheme("file");
        registerReceiver(mReceiver, intentFilter);
        ((AudioManager) getSystemService(AUDIO_SERVICE))
                .requestAudioFocus(null, AudioManager.STREAM_MUSIC,
                AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);
        super.onStart();
    }

    @Override
    protected void onStop() {
        unregisterReceiver(mReceiver);
        ((AudioManager) getSystemService(AUDIO_SERVICE))
                .abandonAudioFocus(null);
        super.onStop();
    }

    @Override
    public void onPause() {
        mPlayer.onPause();
        super.onPause();
    }

    @Override
    public void onResume() {
        mPlayer.onResume();
        super.onResume();
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        mPlayer.onSaveInstanceState(outState);
    }

    @Override
    public void onDestroy() {
        mPlayer.setDisplayMode(0);
        mPlayer.onDestroy();
        super.onDestroy();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        return mPlayer.onKeyDown(keyCode, event)
                || super.onKeyDown(keyCode, event);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        return mPlayer.onKeyUp(keyCode, event)
                || super.onKeyUp(keyCode, event);
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        int keyCode = event.getKeyCode();
        final boolean uniqueDown = event.getRepeatCount() == 0
                && event.getAction() == KeyEvent.ACTION_DOWN;
        switch(keyCode)
        {
            case KeyEvent.KEYCODE_F1:
                if (uniqueDown) {
                    new Thread(new Runnable() {
                        public void run() {
                            new Instrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_MENU);
                        }
                    }).start();
                }
                return true;
            case KeyEvent.KEYCODE_ESCAPE:
                if (uniqueDown) {
                    new Thread(new Runnable() {
                        public void run() {
                            new Instrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
                        }
                    }).start();
                }
                return true;
        }
        return super.dispatchKeyEvent(event);
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent event) {
        switch (event.getButtonState()) {
            case MotionEvent.BUTTON_SECONDARY:
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    new Thread(new Runnable() {
                        public void run() {
                            new Instrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
                        }
                    }).start();
                }
                return true;
            case MotionEvent.BUTTON_TERTIARY:
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    new Thread(new Runnable() {
                        public void run() {
                            new Instrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_MENU);
                        }
                    }).start();
                }
                return true;
        }
        return super.dispatchTouchEvent(event);
    }

    @Override
    public boolean onGenericMotionEvent(MotionEvent event) {
        if ((event.getSource() & InputDevice.SOURCE_CLASS_POINTER) != 0) {
            switch (event.getAction()) {
                case MotionEvent.ACTION_SCROLL: {
                    // Handle mouse (or ext. device) by shifting the page depending on the scroll
                    final float vscroll;
                    final float hscroll;
                    if ((event.getMetaState() & KeyEvent.META_SHIFT_ON) != 0) {
                        vscroll = 0;
                        hscroll = event.getAxisValue(MotionEvent.AXIS_VSCROLL);
                    } else {
                        vscroll = -event.getAxisValue(MotionEvent.AXIS_VSCROLL);
                        hscroll = event.getAxisValue(MotionEvent.AXIS_HSCROLL);
                    }
                    if (hscroll != 0 || vscroll != 0) {
                        AudioManager audioManager = (AudioManager)getSystemService(Context.AUDIO_SERVICE);
                        if (hscroll > 0 || vscroll > 0) {
                            audioManager.adjustStreamVolume(AudioManager.STREAM_MUSIC, AudioManager.ADJUST_LOWER, 1);
                        } else {
                            audioManager.adjustStreamVolume(AudioManager.STREAM_MUSIC, AudioManager.ADJUST_RAISE, 1);
						}
                        return true;
                    }
                }
            }
        }
        return super.onGenericMotionEvent(event);
    }

    BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            mPlayer.onReceive(intent);
        }
    };
}
