/*
 * Copyright 2011 Google Inc.
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

package com.telechips.wfd;

import android.os.Build;
import android.os.Handler;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.util.Log;

public class SystemUiHider {
    private static final int HIDE_DELAY_MILLIS = 2000;

    private Handler mHandler;
    private View mView;
	private final String TAG = "SystemUiHider";
	
    public SystemUiHider(View view) {
        mView = view;
    }

    public void setup(Window window) {
    	Log.d(TAG,"setup()");
    	mView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);

        // Pre-Jellybean just hide the status bar
        
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN) {
            window.addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        }
        

        mHandler = new Handler();
        
        
        mView.setOnSystemUiVisibilityChangeListener(new View.OnSystemUiVisibilityChangeListener() {
            @Override
            public void onSystemUiVisibilityChange(int visibility) {
                if ((visibility & View.SYSTEM_UI_FLAG_HIDE_NAVIGATION) == 0) {
                    delay();
                }
            }
        });
        
    }

    public void hideSystemUi() {
    	Log.d(TAG,"hideSystemUi()");
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
            // On Jellybean we can use new System UI flags to allow showing titlebar/systembar
            // only upon touching the screen, while still having the content laid out in
            // the entire screen.

            //if(WFDPlay.getWFDPlayStatus()==WFDPlay.WFDPLAY_STATUS_PLAY) 
        	{
        	    if(WFDPlay.getInstance()!=null)
                	WFDPlay.getInstance().hideMenu();
            }
            mView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);
        }
    }

    private Runnable mHideRunnable = new Runnable() {
        public void run() {
        	//if(WFDPlay.getWFDPlayStatus() != WFDPlay.WFDPLAY_STATUS_PAUSE) 
        	{
        		hideSystemUi();
        	}
        }
    };

    public void delay() {
    	Log.d(TAG,"delay()");
        mHandler.removeCallbacks(mHideRunnable);
        mHandler.postDelayed(mHideRunnable, HIDE_DELAY_MILLIS);
    }
    
    public void hideMenu() {
    	Log.d(TAG,"delay()");
        mHandler.removeCallbacks(mHideRunnable);
        mHandler.postDelayed(mHideRunnable, 0);
    }
    
    public void cancelDelay() {
    	mHandler.removeCallbacks(mHideRunnable);
    }
}
