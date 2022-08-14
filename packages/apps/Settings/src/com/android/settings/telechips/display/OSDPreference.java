/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.android.settings.telechips.display;

import android.content.ContentResolver;
import android.content.Context;
import android.database.ContentObserver;
import android.os.Bundle;
import android.os.Handler;
import android.os.IPowerManager;
import android.os.Parcel;
import android.os.Parcelable;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.preference.SeekBarDialogPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.util.AttributeSet;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.SeekBar;

import com.android.settings.R;

public class OSDPreference extends SeekBarDialogPreference implements
        SeekBar.OnSeekBarChangeListener{

    private SeekBar mSeekBar_x, mSeekBar_y;
    private int mOutput;
    private int mOSD_x, mOSD_y;
    private int mOldOSD_x, mOldOSD_y;

    private boolean mRestoredOldState;

    private static final int MAXIMUM_RESIZE = 5;
   
     private ContentObserver mOSDObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            onOSDChanged();
        }
    };

    public OSDPreference(Context context, AttributeSet attrs) {
        super(context, attrs);

        setDialogLayoutResource(R.layout.preference_dialog_osd);
    }

    @Override
    protected void showDialog(Bundle state) {
        super.showDialog(state);

	mRestoredOldState = false;
   
        mOutput = getOutputMode();
        mOldOSD_x = getOSD(mOutput,0);
		mOldOSD_y = getOSD(mOutput,1);
   }

    @Override
    protected void onBindDialogView(View view) {
        super.onBindDialogView(view);
		mSeekBar_x = (SeekBar) view.findViewById(R.id.seekbar_x);
		mSeekBar_y = (SeekBar) view.findViewById(R.id.seekbar_y);

        mSeekBar_x.setMax(MAXIMUM_RESIZE);
        mSeekBar_y.setMax(MAXIMUM_RESIZE);
        mOutput = getOutputMode(); 
		mOSD_x = getOSD(mOutput,0);
        mSeekBar_x.setProgress(MAXIMUM_RESIZE - mOSD_x);
		mOSD_y = getOSD(mOutput,1);
        mSeekBar_y.setProgress(MAXIMUM_RESIZE - mOSD_y);

        mSeekBar_x.setOnSeekBarChangeListener(this);
		mSeekBar_y.setOnSeekBarChangeListener(this);
    }
    
    public void onProgressChanged(SeekBar seekBar, int progress,
            boolean fromTouch) {
          	setOSD(mOutput,true);
    }
    
    public void onStartTrackingTouch(SeekBar seekBar) {
        // NA
    }
    
    public void onStopTrackingTouch(SeekBar seekBar) {
        // NA
    }

    private int getOSD(int OutputMode, int coordinates) {
        int osd_x = 0, osd_y = 0;
		if(coordinates == 0){
			switch(OutputMode)
			{
					case 0: // LCD
					case 1: // HDMI
							osd_x = SystemProperties.getInt("persist.sys.hdmi_resize_lt", 1);
							break;
					case 2: //Composite
							osd_x= SystemProperties.getInt("persist.sys.composite_resize_lt", 1);
							break;
					case 3:
							osd_x = SystemProperties.getInt("persist.sys.component_resize_lt", 1);
							break;
			}
		} else{
			switch(OutputMode)
			{
					case 0: // LCD
					case 1: // HDMI
							osd_y = SystemProperties.getInt("persist.sys.hdmi_resize_up", 1);
							break;
					case 2: //Composite
							osd_y = SystemProperties.getInt("persist.sys.composite_resize_up", 1);
							break;
					case 3:
							osd_y = SystemProperties.getInt("persist.sys.component_resize_up", 1);
							break;
			}
		}

		if(coordinates==0)
			return osd_x;
		else
			return osd_y;
    }

    private int getOutputMode() {
        int OutputMode = SystemProperties.getInt("persist.sys.output_mode",1);
        return OutputMode;
    }

    private void onOSDChanged() {
        mSeekBar_x.setProgress(mOSD_x);
		mSeekBar_y.setProgress(mOSD_y);
    }


    @Override
    protected void onDialogClosed(boolean positiveResult) {
        super.onDialogClosed(positiveResult);

        final ContentResolver resolver = getContext().getContentResolver();

	if (positiveResult == false) {
            restoreOldState();
        }

        resolver.unregisterContentObserver(mOSDObserver);
    }

    private void restoreOldState() {
        if (mRestoredOldState) return;
	setOSD(mOutput,false);
        mRestoredOldState = true;
    }

    private void setOSD(int OutputMode, boolean state) {
        if(state){
                switch(OutputMode)
                {
                        case 0: // LCD
                        case 1: // HDMI
                                SystemProperties.set("persist.sys.hdmi_resize_lt", Integer.toString(MAXIMUM_RESIZE - mSeekBar_x.getProgress()));
                                SystemProperties.set("persist.sys.hdmi_resize_rt", Integer.toString(MAXIMUM_RESIZE - mSeekBar_x.getProgress()));
								SystemProperties.set("persist.sys.hdmi_resize_up", Integer.toString(MAXIMUM_RESIZE - mSeekBar_y.getProgress()));
 								SystemProperties.set("persist.sys.hdmi_resize_dn", Integer.toString(MAXIMUM_RESIZE - mSeekBar_y.getProgress()));
                               break;
                        case 2: //Composite
                                SystemProperties.set("persist.sys.composite_resize_lt", Integer.toString(MAXIMUM_RESIZE - mSeekBar_x.getProgress()));
                                SystemProperties.set("persist.sys.composite_resize_rt", Integer.toString(MAXIMUM_RESIZE - mSeekBar_x.getProgress()));
								SystemProperties.set("persist.sys.composite_resize_up", Integer.toString(MAXIMUM_RESIZE - mSeekBar_y.getProgress()));
 								SystemProperties.set("persist.sys.composite_resize_dn", Integer.toString(MAXIMUM_RESIZE - mSeekBar_y.getProgress()));
                                break;
                        case 3:
                                SystemProperties.set("persist.sys.component_resize_lt", Integer.toString(MAXIMUM_RESIZE - mSeekBar_x.getProgress()));
                                SystemProperties.set("persist.sys.component_resize_rt", Integer.toString(MAXIMUM_RESIZE - mSeekBar_x.getProgress()));
								SystemProperties.set("persist.sys.component_resize_up", Integer.toString(MAXIMUM_RESIZE - mSeekBar_y.getProgress()));
 								SystemProperties.set("persist.sys.component_resize_dn", Integer.toString(MAXIMUM_RESIZE - mSeekBar_y.getProgress()));
                                break;
                }
       } else {
                switch(OutputMode)
                {
                        case 0: // LCD
                        case 1: // HDMI
                                SystemProperties.set("persist.sys.hdmi_resize_lt", Integer.toString(mOldOSD_x));
                                SystemProperties.set("persist.sys.hdmi_resize_rt", Integer.toString(mOldOSD_x));
								SystemProperties.set("persist.sys.hdmi_resize_up", Integer.toString(mOldOSD_y));
								SystemProperties.set("persist.sys.hdmi_resize_dn", Integer.toString(mOldOSD_y));
                                break;
                        case 2: //Composite
                                SystemProperties.set("persist.sys.composite_resize_lt", Integer.toString(mOldOSD_x));
                                SystemProperties.set("persist.sys.composite_resize_rt", Integer.toString(mOldOSD_x));
								SystemProperties.set("persist.sys.composite_resize_up", Integer.toString(mOldOSD_y));
 								SystemProperties.set("persist.sys.composite_resize_dn", Integer.toString(mOldOSD_y));
                               break;
                        case 3:
                                SystemProperties.set("persist.sys.component_resize_lt", Integer.toString(mOldOSD_x));
                                SystemProperties.set("persist.sys.component_resize_rt", Integer.toString(mOldOSD_x));
								SystemProperties.set("persist.sys.component_resize_up", Integer.toString(mOldOSD_y));
 								SystemProperties.set("persist.sys.component_resize_dn", Integer.toString(mOldOSD_y));
                               break;
                }
        }
   }
}
