package com.android.server;

import android.content.ContentResolver;
import android.content.Context;
import android.os.ServiceManager;
import android.util.Log;
import android.util.Slog;

public class AfterQBSystemCallback2 implements Runnable {
    private static final String TAG = "AfterQBSystemCallback2";
    
    private ContentResolver mContentResolver;
    private Context mContext;
    
    AfterQBSystemCallback2(Context context, ContentResolver contentResolver) {
        mContentResolver = contentResolver;
        mContext = context;
    }
    
    @Override
    public void run() {
        Log.d(TAG, "Call AfterQBSystemCallback2 ");
        
        PreQBSystemCallback callback = new PreQBSystemCallback(mContext, mContentResolver);
        callback.run();
        
        Slog.i(TAG, "Telechips Display Manager");
        TelechipsDisplayManagerService displayManager = new TelechipsDisplayManagerService(mContext);
        ServiceManager.addService(Context.TELECHIPS_DISPLAY_SERVICE, displayManager);
       	//start1 = java.lang.System.currentTimeMillis();
    }

}
