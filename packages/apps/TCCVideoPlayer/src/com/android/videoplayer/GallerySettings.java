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

import android.app.Instrumentation;
import android.graphics.drawable.PaintDrawable;
import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Window;
import android.os.SystemProperties;
import android.preference.ListPreference;
import java.util.ArrayList;

// GallerySettings
//
// This is the setting screen for Gallery.
// It reads the available settings from an XML resource.
public class GallerySettings extends PreferenceActivity {
    @Override
    public void onCreate(Bundle icicle) {
       
        this.requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);
       
        super.onCreate(icicle); 
        addPreferencesFromResource(R.xml.gallery_preferences);
        this.getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE, R.layout.set_title);
        this.getListView().setSelector(new PaintDrawable(0xff33b5e5));


	ListPreference m3DMode = (ListPreference) findPreference("pref_gallery_video_3D_UI_key");

	boolean m_3d_active = SystemProperties.get("tcc.3d.ui.enable","0").equals("1") ;
	boolean mvc_active  = SystemProperties.get("tcc.video.mvc.enable","0").equals("1");

	if(m_3d_active || mvc_active) {
		final CharSequence[] entries = m3DMode.getEntries();
		final CharSequence[] values = m3DMode.getEntryValues();

		ArrayList<CharSequence> revisedEntries = new ArrayList<CharSequence>();
		ArrayList<CharSequence> revisedValues = new ArrayList<CharSequence>();

		revisedEntries.add(entries[0]);
		revisedValues.add(values[0]);

		if(m_3d_active)
		{
			revisedEntries.add(entries[1]);
			revisedValues.add(values[1]);

			revisedEntries.add(entries[2]);
			revisedValues.add(values[2]);
		}

		if(mvc_active)
		{
			revisedEntries.add(entries[3]);
			revisedValues.add(values[3]);
		}

	        m3DMode.setEntries(revisedEntries.toArray(new CharSequence[revisedEntries.size()]));
	        m3DMode.setEntryValues(revisedValues.toArray(new CharSequence[revisedValues.size()]));
       } else {
		getPreferenceScreen().removePreference(m3DMode);
       }
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
}
