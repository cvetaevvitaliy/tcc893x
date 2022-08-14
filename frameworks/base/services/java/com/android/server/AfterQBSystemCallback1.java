package com.android.server;

import android.content.ContentResolver;
import android.content.Context;
import android.database.ContentObserver;
import android.os.SystemProperties;
import android.util.Log;

public class AfterQBSystemCallback1 implements Runnable {
    private static final String TAG = "AfterQBSystemCallback1";
    
    private ContentResolver mContentResolver;
    private Context mContext;
    
    AfterQBSystemCallback1(Context context, ContentResolver contentResolver) {
        mContentResolver = contentResolver;
        mContext = context;
    }
    
    @Override
    public void run() {
        Log.d(TAG, "Call AfterQBSystemCallback1 ");
        
       	//start1 = java.lang.System.currentTimeMillis();

    }

}
