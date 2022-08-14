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

package com.android.settings;

import android.content.Context;
import android.preference.Preference;
import android.util.AttributeSet;
import android.view.View;
import android.widget.TextView;

public class ProgressCategory extends ProgressCategoryBase {

    private final int mEmptyTextRes;
    private boolean mProgress = false;
    private Preference mNoDeviceFoundPreference;
    private boolean mNoDeviceFoundAdded;
    private TextView scanning;

    public ProgressCategory(Context context, AttributeSet attrs) {
        super(context, attrs);
        setLayoutResource(R.layout.preference_progress_category_stb);
        mEmptyTextRes = R.string.bluetooth_empty_list_bluetooth_off;
    }

    public ProgressCategory(Context context, AttributeSet attrs,
            int emptyTextRes) {
        super(context, attrs);
        setLayoutResource(R.layout.preference_progress_category);
        mEmptyTextRes = emptyTextRes;
    }

    @Override
    public void onBindView(View view) {
        super.onBindView(view);

        //final View progressBar = view.findViewById(R.id.scanning_progress);
        if(mEmptyTextRes == R.string.bluetooth_empty_list_bluetooth_off) {
		scanning = (TextView) view.findViewById(R.id.scanning);
        }
	 if(mEmptyTextRes == R.string.bluetooth_empty_list_bluetooth_off) {
		scanning.setText(mProgress ? R.string.progress_scanning : R.string.progress_tap_to_pair);
	 }

	 boolean noDeviceFound = (getPreferenceCount() == 0 ||
                (getPreferenceCount() == 1 && getPreference(0) == mNoDeviceFoundPreference));
        if(mEmptyTextRes == R.string.bluetooth_empty_list_bluetooth_off) {
		scanning.setVisibility(noDeviceFound ? View.GONE : View.VISIBLE);
        }
        //progressBar.setVisibility(mProgress ? View.VISIBLE : View.GONE);

        if(mEmptyTextRes == R.string.bluetooth_empty_list_bluetooth_off);
        else {
           if (mProgress || !noDeviceFound) {
		if (mNoDeviceFoundAdded) {
		     removePreference(mNoDeviceFoundPreference);
		     mNoDeviceFoundAdded = false;
		}
	    } else {
		if (!mNoDeviceFoundAdded) {
		    if (mNoDeviceFoundPreference == null) {
			mNoDeviceFoundPreference = new Preference(getContext());
			mNoDeviceFoundPreference.setLayoutResource(R.layout.preference_empty_list);
			mNoDeviceFoundPreference.setSelectable(false);
		    }
		    addPreference(mNoDeviceFoundPreference);
		    mNoDeviceFoundAdded = true;
		}
	    }
	}
    }

    @Override
    public void setProgress(boolean progressOn) {
        mProgress = progressOn;
        notifyChanged();
    }
}
