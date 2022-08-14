/*
# Copyright (C) 2013 Telechips, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
*/


package com.telechips.wfd;

import android.content.Context;
import android.net.wifi.p2p.WifiP2pGroup;
import android.preference.Preference;
import android.view.View;

public class WifiDisplayPersistentGroup extends Preference {

    public WifiP2pGroup mGroup;

    public WifiDisplayPersistentGroup(Context context, WifiP2pGroup group) {
        super(context);
        mGroup = group;
    }

    @Override
    protected void onBindView(View view) {
        if(mGroup.getNetworkName4User()!=null)
            setTitle(mGroup.getNetworkName4User());
        else
            setTitle(mGroup.getNetworkName());
        super.onBindView(view);
    }

    int getNetworkId() {
        return mGroup.getNetworkId();
    }

    String getGroupName() {
        return mGroup.getNetworkName();
    }
    
    String getGroupName4User() {
        return mGroup.getNetworkName4User();
    }

}
