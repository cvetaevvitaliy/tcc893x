package com.android.server;

import android.content.ContentResolver;
import android.content.Context;
import android.os.ServiceManager;
import android.util.Log;
import android.util.Slog;

public class BeforeQBSystemCallback1 implements Runnable {
    private static final String TAG = "BeforeQBSystemCallback1";
    
    private ContentResolver mContentResolver;
    private Context mContext;
    
    BeforeQBSystemCallback1(Context context, ContentResolver contentResolver) {
        mContentResolver = contentResolver;
        mContext = context;
    }
    
    @Override
    public void run() {
        Log.d(TAG, "Call BeforeQBSystemCallback1 ");
    }
}
