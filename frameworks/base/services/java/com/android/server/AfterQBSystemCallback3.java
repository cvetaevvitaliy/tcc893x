package com.android.server;

import android.content.ContentResolver;
import android.content.Context;
import android.database.ContentObserver;
import android.os.SystemProperties;
import android.util.Log;

public class AfterQBSystemCallback3 implements Runnable {
    private static final String TAG = "AfterQBSystemCallback3";
    
    private ContentResolver mContentResolver;
    private Context mContext;
    
    AfterQBSystemCallback3(Context context, ContentResolver contentResolver) {
        mContentResolver = contentResolver;
        mContext = context;
    }
    
    @Override
    public void run() {
        Log.d(TAG, "Call AfterQBSystemCallback3 ");
        
       	//start1 = java.lang.System.currentTimeMillis();

    }

}
