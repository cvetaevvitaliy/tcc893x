/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.android.systemui.statusbar;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.accessibility.AccessibilityEvent;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.os.SystemProperties;

import com.android.systemui.R;
import com.android.systemui.statusbar.policy.NetworkController;

// Intimately tied to the design of res/layout/signal_cluster_view.xml
public class SignalClusterView
        extends LinearLayout
        implements NetworkController.SignalCluster {

    static final boolean DEBUG = false;
    static final String TAG = "SignalClusterView";

    NetworkController mNC;

    private boolean mWifiVisible = false;
    private int mWifiStrengthId = 0;
    private boolean mMobileVisible = false;
    private int mMobileStrengthId = 0, mMobileTypeId = 0;
    private boolean mIsAirplaneMode = false;
    private int mAirplaneIconId = 0;
    private String mWifiDescription, mMobileDescription, mMobileTypeDescription;
    private boolean mEthernetVisible = false;
    private int mEthernetId = 0, mEthernetActivityId = 0;
    private boolean mNfsVisible = false;
    private int mNfsId = 0, mNfsActivityId = 0;
    private String mNfsDescription;

    ViewGroup mWifiGroup, mMobileGroup;
    ViewGroup mEthernetGroup;
    ImageView mEthernet;
    ImageView mWifi, mMobile, mMobileType, mAirplane;
    View mSpacer;
    ViewGroup mNfsGroup;
    ImageView mNfs;

    public SignalClusterView(Context context) {
        this(context, null);
    }

    public SignalClusterView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public SignalClusterView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    public void setNetworkController(NetworkController nc) {
        if (DEBUG) Log.d(TAG, "NetworkController=" + nc);
        mNC = nc;
    }

    @Override
    protected void onAttachedToWindow() {
        super.onAttachedToWindow();

        mWifiGroup      = (ViewGroup) findViewById(R.id.wifi_combo);
        mWifi           = (ImageView) findViewById(R.id.wifi_signal);
	if(!(SystemProperties.get("tcc.solution.mbox","0").equals("1"))){
        	mMobileGroup    = (ViewGroup) findViewById(R.id.mobile_combo);
        	mMobile         = (ImageView) findViewById(R.id.mobile_signal);
        	mMobileType     = (ImageView) findViewById(R.id.mobile_type);
	}
        mSpacer         =             findViewById(R.id.spacer);
        mAirplane       = (ImageView) findViewById(R.id.airplane);

        mEthernetGroup  = (ViewGroup) findViewById(R.id.ethernet_combo);
        mEthernet       = (ImageView) findViewById(R.id.ethernet);
        mNfsGroup       = (ViewGroup) findViewById(R.id.nfs_combo);
        mNfs            = (ImageView) findViewById(R.id.nfs);

        apply();
    }

    @Override
    protected void onDetachedFromWindow() {
        mWifiGroup      = null;
        mWifi           = null;
        mMobileGroup    = null;
        mMobile         = null;
        mMobileType     = null;
        mEthernetGroup  = null;
       	mEthernet       = null;
        mNfsGroup       = null;
        mNfs            = null;
        mSpacer         = null;
        mAirplane       = null;

        super.onDetachedFromWindow();
    }

    @Override
    public void setWifiIndicators(boolean visible, int strengthIcon, String contentDescription) {
        mWifiVisible = visible;
        mWifiStrengthId = strengthIcon;
        mWifiDescription = contentDescription;

        apply();
    }

    public void setEthernetIndicators(boolean visible, int strengthIcon, int activityIcon,
            String contentDescription) {
        mEthernetVisible = visible;
        mEthernetId = strengthIcon;
        mEthernetActivityId = activityIcon;

        apply();
    }

    public void setNfsIndicators(boolean visible, int strengthIcon, int activityIcon,
            String contentDescription) {
        mNfsVisible = visible;
        mNfsId = strengthIcon;
        mNfsActivityId = activityIcon;

        apply();
    }

    @Override
    public void setMobileDataIndicators(boolean visible, int strengthIcon,
            int typeIcon, String contentDescription, String typeContentDescription) {
        mMobileVisible = visible;
        mMobileStrengthId = strengthIcon;
        mMobileTypeId = typeIcon;
        mMobileDescription = contentDescription;
        mMobileTypeDescription = typeContentDescription;

        apply();
    }

    @Override
    public void setIsAirplaneMode(boolean is, int airplaneIconId) {
        mIsAirplaneMode = is;
        mAirplaneIconId = airplaneIconId;

        apply();
    }

    @Override
    public boolean dispatchPopulateAccessibilityEvent(AccessibilityEvent event) {
        // Standard group layout onPopulateAccessibilityEvent() implementations
        // ignore content description, so populate manually
        if (mWifiVisible && mWifiGroup != null && mWifiGroup.getContentDescription() != null)
            event.getText().add(mWifiGroup.getContentDescription());
        if (mMobileVisible && mMobileGroup != null && mMobileGroup.getContentDescription() != null)
            event.getText().add(mMobileGroup.getContentDescription());
        return super.dispatchPopulateAccessibilityEvent(event);
    }

    @Override
    public void onRtlPropertiesChanged(int layoutDirection) {
        super.onRtlPropertiesChanged(layoutDirection);

        if (mWifi != null) {
            mWifi.setImageDrawable(null);
        }

        if (mMobile != null) {
            mMobile.setImageDrawable(null);
        }

        if (mMobileType != null) {
            mMobileType.setImageDrawable(null);
        }

        if(mAirplane != null) {
            mAirplane.setImageDrawable(null);
        }

        apply();
    }

    // Run after each indicator change.
    private void apply() {
        if (mWifiGroup == null) return;

        if (mWifiVisible) {
            mWifi.setImageResource(mWifiStrengthId);

            mWifiGroup.setContentDescription(mWifiDescription);
            mWifiGroup.setVisibility(View.VISIBLE);
        } else {
            mWifiGroup.setVisibility(View.GONE);
        }

        if (DEBUG) Log.d(TAG,
                String.format("wifi: %s sig=%d",
                    (mWifiVisible ? "VISIBLE" : "GONE"),
                    mWifiStrengthId));

        if (mEthernetVisible) {
            mEthernetGroup.setVisibility(View.VISIBLE);
            mEthernet.setImageResource(mEthernetId);
            mEthernetGroup.setContentDescription(null);
        } else {
            mEthernetGroup.setVisibility(View.GONE);
        }

        if (mMobileVisible && !mIsAirplaneMode) {
            mMobile.setImageResource(mMobileStrengthId);
            mMobileType.setImageResource(mMobileTypeId);
        }

        if (mNfsVisible) {
            mNfsGroup.setVisibility(View.VISIBLE);
            mNfs.setImageResource(mNfsId);
            mNfsGroup.setContentDescription(null);
        } else {
            mNfsGroup.setVisibility(View.GONE);
        }
	
				if(!(SystemProperties.get("tcc.solution.mbox","0").equals("1"))){
      	  if (mMobileVisible && !mIsAirplaneMode) {
        	    mMobile.setImageResource(mMobileStrengthId);
            	mMobileType.setImageResource(mMobileTypeId);

	            mMobileGroup.setContentDescription(mMobileTypeDescription + " " + mMobileDescription);
 	           mMobileGroup.setVisibility(View.VISIBLE);
 	       	} else {
 	           mMobileGroup.setVisibility(View.GONE);
 					}

				}

        if (mIsAirplaneMode) {
            mAirplane.setImageResource(mAirplaneIconId);
            mAirplane.setVisibility(View.VISIBLE);
        } else {
            mAirplane.setVisibility(View.GONE);
        }

        if (mMobileVisible && mWifiVisible && mIsAirplaneMode) {
            mSpacer.setVisibility(View.INVISIBLE);
        } else {
            mSpacer.setVisibility(View.GONE);
        }

        if (DEBUG) Log.d(TAG,
                String.format("mobile: %s sig=%d typ=%d",
                    (mMobileVisible ? "VISIBLE" : "GONE"),
                    mMobileStrengthId, mMobileTypeId));

	if(!(SystemProperties.get("tcc.solution.mbox","0").equals("1"))){
        	mMobileType.setVisibility(
                	!mWifiVisible ? View.VISIBLE : View.GONE);
	}
    }
}

