package com.android.server;

import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.database.ContentObserver;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.util.Log;
import android.util.Slog;

public class StartSystemUIQBCallback implements Runnable {
    private static final String TAG = "StartSystemUIQBCallback";
    
    private ContentResolver mContentResolver;
    private Context mContext;
    
    StartSystemUIQBCallback(Context context, ContentResolver contentResolver) {
        mContentResolver = contentResolver;
        mContext = context;
    }
    
    @Override
    public void run() {
        Log.d(TAG, "Call StartSystemUIQBCallback ");
        
        Intent intent = new Intent();
        intent.setComponent(new ComponentName("com.android.systemui",
                "com.android.systemui.SystemUIService"));
        Slog.d(TAG, "Starting service: " + intent);
        mContext.startServiceAsUser(intent, UserHandle.OWNER);
    }

}
