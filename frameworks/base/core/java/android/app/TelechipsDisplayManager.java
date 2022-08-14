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

package android.app;

import android.content.Context;
import android.content.Intent;
import android.os.RemoteException;
import android.os.ServiceManager;

/**
 * @hide
 */
public class TelechipsDisplayManager
{
    private static final boolean DEBUG =  false;
    public static final String TAG = "TelechipsDisplayManager";
    public static final String DISPLAY_MODE_CHANGED_ACTION ="android.app.DISPLAY_MODE_CHANGED";
    public static final String DISPLAY_RESOLUTION_CHANGED_ACTION ="android.app.DISPLAY_RESOLUTION_CHANGED";
    public static final String HDMI_MODE_CHANGED_ACTION ="android.app.HDMI_MODE_CHANGED";

    public static final String EXTRA_DISPLAY_MODE = "Display_mode";
    public static final String EXTRA_PREVIOUS_DISPLAY_MODE = "previous_Display_mode";
    public static final String EXTRA_DISPLAY_RESOLUTION = "Display_resolution";
    public static final String EXTRA_PREVIOUS_DISPLAY_RESOLUTION = "previous_Display_resolution";
    public static final String EXTRA_HDMI_MODE = "HDMI_mode";
    public static final String EXTRA_PREVIOUS_HDMI_MODE = "previous_HDMI_mode";


    public static final int DISPLAY_OUTPUT_LCD = 0;
    public static final int DISPLAY_OUTPUT_HDMI = 1;
    public static final int DISPLAY_OUTPUT_COMPOSITE = 2;
    public static final int DISPLAY_OUTPUT_COMPONENT = 3;

    public static final int DISPLAY_DEFAULT_HDMI_RESOLUTION = 125;

    public static final int DISPLAY_DEFAULT_HDMI_MODE = 0;

    private final ITelechipsDisplayManager mService;

    /**
     * package private on purpose
     */
    TelechipsDisplayManager(ITelechipsDisplayManager service) {
        mService = service;
    }

    public int getOutputMode( ) {
        try {
             return mService.getOutputMode();
        } catch (RemoteException e) {
           return 0;
        }
    }

    public void setOutputMode(int mode) {
        try {
            mService.setOutputMode(mode);
        } catch (RemoteException ex) {
        }
    }

    public int getOutputResolution( ) {
        try {
             return mService.getOutputResolution();
        } catch (RemoteException e) {
           return 0;
        }
    }

    public void setOutputResolution(int resolution) {
        try {
            mService.setOutputResolution(resolution);
        } catch (RemoteException ex) {
        }
    }

    public int getHDMIMode( ) {
        try {
             return mService.getHDMIMode();
        } catch (RemoteException e) {
           return 0;
        }
    }

    public void setHDMIMode(int mode) {
        try {
            mService.setHDMIMode(mode);
        } catch (RemoteException ex) {
        }
    }

    public int getSetMode( ) {
        try {
             return mService.getSetMode();
        } catch (RemoteException e) {
           return 0;
        }
    }

    public void setSetMode(int mode) {
        try {
            mService.setSetMode(mode);
        } catch (RemoteException ex) {
        }
    }

    public int getSetHDMode( ) {
        try {
             return mService.getSetHDMode();
        } catch (RemoteException e) {
           return 0;
        }
    }

    public void setSetHDMode(int mode) {
        try {
            mService.setSetHDMode(mode);
        } catch (RemoteException ex) {
        }
    }


}
