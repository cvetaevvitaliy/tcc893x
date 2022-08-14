package com.android.server;

import android.content.ContentResolver;
import android.content.Context;
import android.database.ContentObserver;
import android.os.SystemProperties;
import android.provider.Settings;
import android.util.Log;
import android.util.Slog;

public class PreQBSystemCallback implements Runnable {
    private static final String TAG = "PreQBSystemCallback";
    
    private ContentResolver mContentResolver;
    private Context mContext;
    
    PreQBSystemCallback(Context context, ContentResolver contentResolver) {
        mContentResolver = contentResolver;
        mContext = context;
    }
    

    private class SPDIFSettingsObserver extends ContentObserver {
        public SPDIFSettingsObserver() {
            super(null);
        }
        @Override
        public void onChange(boolean selfChange) {
			// Planet 20110908 Start
			try {
			       Thread.sleep(100);
			} catch (InterruptedException e) { }
			// Planet 20110908 End
			
            int settingSPDIF;
            settingSPDIF = Settings.System.getInt(mContentResolver,
                Settings.System.SPDIF_SETTING, 0);
            SystemProperties.set("persist.sys.spdif_setting", Integer.toString(settingSPDIF));
        }
    }


    private class OutputModeObserver extends ContentObserver {
        //BluetoothService bluetooth = null;

        public OutputModeObserver() {
            super(null);
        }
        @Override
        public void onChange(boolean selfChange) {
			try {
			       Thread.sleep(100);
			} catch (InterruptedException e) { }
           int outputMode = Settings.System.getInt( mContentResolver,
           Settings.System.OUTPUT_MODE ,0) ;
           SystemProperties.set("persist.sys.output_mode", Integer.toString(outputMode));
	/*
	   if((SystemProperties.get("ro.memtype")).compareTo("ddr3") == 0)
	   	if(outputMode == 1 && bluetooth != null)
		 	bluetooth.disable();	
	  */ 

        }
    }

    private class HDMIResolutionObserver extends ContentObserver {
        public HDMIResolutionObserver() {
            super(null);
        }
        @Override
        public void onChange(boolean selfChange) {
        	try {
			       Thread.sleep(100);
			} catch (InterruptedException e) { }

            int hdmiResolution = Settings.System.getInt(mContentResolver,
                Settings.System.HDMI_RESOLUTION, 0);
            SystemProperties.set("persist.sys.hdmi_resolution", Integer.toString(hdmiResolution));
        }
    }

    private class HDMIModeObserver extends ContentObserver {
        public HDMIModeObserver() {
            super(null);
        }
        @Override
        public void onChange(boolean selfChange) {
        	try {
			       Thread.sleep(100);
			} catch (InterruptedException e) { }
            int hdmiMode = Settings.System.getInt(mContentResolver,
                Settings.System.HDMI_MODE, 0);
            SystemProperties.set("persist.sys.hdmi_mode", Integer.toString(hdmiMode));
        }
    }

    private class HDMIAspectRatioObserver extends ContentObserver {
        public HDMIAspectRatioObserver() {
            super(null);
        }
        @Override
        public void onChange(boolean selfChange) {
        	try {
			       Thread.sleep(100);
			} catch (InterruptedException e) { }
            int hdmiAspectRatio = Settings.System.getInt(mContentResolver,
                Settings.System.HDMI_ASPECT_RATIO, 0);
            SystemProperties.set("persist.sys.hdmi_aspect_ratio", Integer.toString(hdmiAspectRatio));
        }
    }

    private class HDMIColorSpaceObserver extends ContentObserver {
        public HDMIColorSpaceObserver() {
            super(null);
        }
        @Override
        public void onChange(boolean selfChange) {
        	try {
			       Thread.sleep(100);
			} catch (InterruptedException e) { }
            int hdmiColorSpace = Settings.System.getInt(mContentResolver,
                Settings.System.HDMI_COLOR_SPACE, 0);
            SystemProperties.set("persist.sys.hdmi_color_space", Integer.toString(hdmiColorSpace));
        }
    }

    private class HDMIColorDepthObserver extends ContentObserver {
        public HDMIColorDepthObserver() {
            super(null);
        }
        @Override
        public void onChange(boolean selfChange) {
        	try {
			       Thread.sleep(100);
			} catch (InterruptedException e) { }
            int hdmiColorDepth = Settings.System.getInt(mContentResolver,
                Settings.System.HDMI_COLOR_DEPTH, 0);
            SystemProperties.set("persist.sys.hdmi_color_depth", Integer.toString(hdmiColorDepth));
        }
    }

    private class CompositeModeObserver extends ContentObserver {
        public CompositeModeObserver() {
            super(null);
        }
        @Override
        public void onChange(boolean selfChange) {
            int compositeMode = Settings.System.getInt(mContentResolver,
                Settings.System.COMPOSITE_MODE, 0);
            SystemProperties.set("persist.sys.composite_mode", Integer.toString(compositeMode));
        }
    }

    private class ComponentResolutionObserver extends ContentObserver {
        public ComponentResolutionObserver() {
            super(null);
        }
        @Override
        public void onChange(boolean selfChange) {
            int componentResoution = Settings.System.getInt(mContentResolver,
                Settings.System.COMPONENT_RESOLUTION, 0);
            SystemProperties.set("persist.sys.component_mode", Integer.toString(componentResoution));
        }
    }


    private class HDMIAutoSelectObserver extends ContentObserver {
        public HDMIAutoSelectObserver() {
            super(null);
        }
        @Override
        public void onChange(boolean selfChange) {
            boolean enableAutoSelect = (Settings.System.getInt(mContentResolver,
                Settings.System.HDMI_AUTO_SELECT, 0) > 0);
            SystemProperties.set("persist.sys.hdmi_auto_select", enableAutoSelect ? "1" : "0");
        }
    }

   private class HDMICECSelectObserver extends ContentObserver {
        public HDMICECSelectObserver() {
            super(null);
        }
        @Override
        public void onChange(boolean selfChange) {
            boolean enableCECSelect = (Settings.System.getInt(mContentResolver,
                Settings.System.HDMI_CEC_SELECT, 0) > 0);
            SystemProperties.set("persist.sys.hdmi_cec", enableCECSelect ? "1" : "0");
        }
    }

	private class NativeModeObserver extends ContentObserver {
        public NativeModeObserver() {
            super(null);
        }
        @Override
        public void onChange(boolean selfChange) {
            int native_mode = Settings.System.getInt(mContentResolver,
                Settings.System.NATIVE_MODE, 0);
            SystemProperties.set("persist.sys.native_mode", Integer.toString(native_mode));
        }
    }

	private class NativeModeSelectObserver extends ContentObserver {
		 public NativeModeSelectObserver() {
			 super(null);
		 }
		 @Override
		 public void onChange(boolean selfChange) {
			 boolean enableSelect = (Settings.System.getInt(mContentResolver,
				 Settings.System.NATIVE_MODE_SELECT, 0) > 0);
			 SystemProperties.set("persist.sys.native_mode_select", enableSelect ? "1" : "0");
		 }
	 }

    private class AppRotationObserver extends ContentObserver {
        public AppRotationObserver() {
            super(null);
        }
        @Override
        public void onChange(boolean selfChange) {
            int app_rotation = Settings.System.getInt(mContentResolver,
                Settings.System.APP_ROTATION, 0);
            SystemProperties.set("persist.sys.app_rotation", Integer.toString(app_rotation));
        }
    }

    private class MouseMButtonEventObserver extends ContentObserver {
        public MouseMButtonEventObserver() {
            super(null);
        }
        @Override
        public void onChange(boolean selfChange) {
            int event = Settings.System.getInt(mContentResolver,
                Settings.System.MOUSE_M_BUTTON_EVENT, 0);
            SystemProperties.set("persist.sys.middle_mouse", Integer.toString(event));
        }
    }

    private class MouseRButtonEventObserver extends ContentObserver {
        public MouseRButtonEventObserver() {
            super(null);
        }
        @Override
        public void onChange(boolean selfChange) {
            int event = Settings.System.getInt(mContentResolver,
                Settings.System.MOUSE_R_BUTTON_EVENT, 0);
            SystemProperties.set("persist.sys.right_mouse", Integer.toString(event));
        }
    }

    
    @Override
    public void run() {
        Log.d(TAG, "Call QBPostSystemCallback ");
        
       	//start1 = java.lang.System.currentTimeMillis();

        try {
            SystemProperties.set("persist.sys.spdif_setting",
                    Integer.toString(Settings.System.getInt(mContentResolver,
                    Settings.System.SPDIF_SETTING)));
        } catch (Throwable e) {
            Slog.e(TAG, "Failed to set SPDIF output properties", e);
        }
        try {
            SystemProperties.set("persist.sys.output_mode",
                    Integer.toString(Settings.System.getInt(mContentResolver,
                            Settings.System.OUTPUT_MODE)));
        } catch (Throwable e) {
            Slog.e(TAG, "Failed to set output mode setting properties", e);
        }

        try {
            SystemProperties.set("persist.sys.hdmi_resolution",
                   Integer.toString(Settings.System.getInt(mContentResolver,
                           Settings.System.HDMI_RESOLUTION)));
        } catch (Throwable e) {
            Slog.e(TAG, "Failed to set HDMI resolution setting properties", e);
        }

        try {
            SystemProperties.set("persist.sys.hdmi_auto_select",
                    Integer.toString(Settings.System.getInt(mContentResolver,
                            Settings.System.HDMI_AUTO_SELECT)));
        } catch (Throwable e) {
            Slog.e(TAG, "Failed to set HDMI auto select setting properties", e);
        }

        try {
            SystemProperties.set("persist.sys.hdmi_cec",
                    Integer.toString(Settings.System.getInt(mContentResolver,
                            Settings.System.HDMI_CEC_SELECT)));
        } catch (Throwable e) {
            Slog.e(TAG, "Failed to set HDMI CEC select setting properties", e);
        }

        try {
            SystemProperties.set("persist.sys.hdmi_mode",
                    Integer.toString(Settings.System.getInt(mContentResolver, Settings.System.HDMI_MODE)));
        } catch (Throwable e) {
            Slog.e(TAG, "Failed to set HDMI mode setting properties", e);
        }

        try {
            SystemProperties.set("persist.sys.hdmi_aspect_ratio",
                    Integer.toString(Settings.System.getInt(mContentResolver, Settings.System.HDMI_ASPECT_RATIO)));
        } catch (Throwable e) {
            Slog.e(TAG, "Failed to set HDMI aspect ratio setting properties", e);
        }

        try {
            SystemProperties.set("persist.sys.hdmi_color_space",
                    Integer.toString(Settings.System.getInt(mContentResolver, Settings.System.HDMI_COLOR_SPACE)));
        } catch (Throwable e) {
            Slog.e(TAG, "Failed to set HDMI color space setting properties", e);
        }			

        try {
            SystemProperties.set("persist.sys.hdmi_color_depth",
                    Integer.toString(Settings.System.getInt(mContentResolver, Settings.System.HDMI_COLOR_DEPTH)));
        } catch (Throwable e) {
            Slog.e(TAG, "Failed to set HDMI color depth setting properties", e);
        }

        try {
            SystemProperties.set("persist.sys.app_rotation",
                    Integer.toString(Settings.System.getInt(mContentResolver,
                            Settings.System.APP_ROTATION)));
        } catch (Throwable e) {
            Slog.e(TAG, "Failed to set output mode setting properties", e);
        }

        try {
            SystemProperties.set("persist.sys.middle_mouse",
                    Integer.toString(Settings.System.getInt(mContentResolver,
                            Settings.System.MOUSE_M_BUTTON_EVENT)));
        } catch (Throwable e) {
            Slog.e(TAG, "Failed to set mouse middle button setting properties", e);
        }

        try {
            SystemProperties.set("persist.sys.right_mouse",
                    Integer.toString(Settings.System.getInt(mContentResolver,
                            Settings.System.MOUSE_R_BUTTON_EVENT)));
        } catch (Throwable e) {
            Slog.e(TAG, "Failed to set mouse right button setting properties", e);
        }
        
        try {
            SystemProperties.set("persist.sys.composite_mode",
                    Integer.toString(Settings.System.getInt(mContentResolver, 
                            Settings.System.COMPOSITE_MODE)));
        } catch (Throwable e) {
            Slog.e(TAG, "Failed to set composite mode setting properties", e);
        }

        try {
            SystemProperties.set("persist.sys.component_mode",
                    Integer.toString(Settings.System.getInt(mContentResolver,
                            Settings.System.COMPONENT_RESOLUTION)));
        } catch (Throwable e) {
            Slog.e(TAG, "Failed to set component resolution setting properties", e);
        }

        try {
            SystemProperties.set("persist.sys.native_mode",
                    Integer.toString(Settings.System.getInt(mContentResolver, Settings.System.NATIVE_MODE)));
        } catch (Throwable e) {
            Slog.e(TAG, "Failed to set Native Mode properties", e);
        }


        try {
            SystemProperties.set("persist.sys.native_mode_select",
                    Integer.toString(Settings.System.getInt(mContentResolver,
                            Settings.System.NATIVE_MODE_SELECT)));
        } catch (Throwable e) {
            Slog.e(TAG, "Failed to set Native Mode select setting properties", e);
        }
        mContentResolver.registerContentObserver(Settings.System.getUriFor(Settings.System.SPDIF_SETTING),
                false, new SPDIFSettingsObserver());

        mContentResolver.registerContentObserver(Settings.System.getUriFor(Settings.System.OUTPUT_MODE),
                false, new OutputModeObserver());

        mContentResolver.registerContentObserver(Settings.System.getUriFor(Settings.System.HDMI_RESOLUTION),
                false, new HDMIResolutionObserver());

        mContentResolver.registerContentObserver(Settings.System.getUriFor(Settings.System.HDMI_MODE),
                false, new HDMIModeObserver());

        mContentResolver.registerContentObserver(Settings.System.getUriFor(Settings.System.HDMI_AUTO_SELECT),
                false, new HDMIAutoSelectObserver());

        mContentResolver.registerContentObserver(Settings.System.getUriFor(Settings.System.HDMI_CEC_SELECT),
                false, new HDMICECSelectObserver());

        mContentResolver.registerContentObserver(Settings.System.getUriFor(Settings.System.HDMI_ASPECT_RATIO),
                false, new HDMIAspectRatioObserver());

        mContentResolver.registerContentObserver(Settings.System.getUriFor(Settings.System.HDMI_COLOR_SPACE),
                false, new HDMIColorSpaceObserver());

        mContentResolver.registerContentObserver(Settings.System.getUriFor(Settings.System.HDMI_COLOR_DEPTH),
                false, new HDMIColorDepthObserver());

        mContentResolver.registerContentObserver(Settings.System.getUriFor(Settings.System.COMPOSITE_MODE),
                false, new CompositeModeObserver());

        mContentResolver.registerContentObserver(Settings.System.getUriFor(Settings.System.COMPONENT_RESOLUTION),
                false, new ComponentResolutionObserver());

        mContentResolver.registerContentObserver(Settings.System.getUriFor(Settings.System.NATIVE_MODE),
                false, new NativeModeObserver());
		
        mContentResolver.registerContentObserver(Settings.System.getUriFor(Settings.System.NATIVE_MODE_SELECT),
                false, new NativeModeSelectObserver());
		
        mContentResolver.registerContentObserver(Settings.System.getUriFor(Settings.System.APP_ROTATION),
                false, new AppRotationObserver());

        mContentResolver.registerContentObserver(Settings.System.getUriFor(Settings.System.MOUSE_M_BUTTON_EVENT),
                false, new MouseMButtonEventObserver());

        mContentResolver.registerContentObserver(Settings.System.getUriFor(Settings.System.MOUSE_R_BUTTON_EVENT),
                false, new MouseRButtonEventObserver());

    }

}
