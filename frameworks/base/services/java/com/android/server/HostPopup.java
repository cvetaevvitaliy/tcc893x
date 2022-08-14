package com.android.server;

import com.android.server.power.PowerManagerService;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.view.Window;
import android.view.WindowManager;

import com.android.internal.R;

public class HostPopup extends Activity {
    private Context mContext;
    AlertDialog.Builder mYesNoPopup = null;
    int mWaitTime = 5;


    private BroadcastReceiver HostPopupReceiver = new BroadcastReceiver() {
       @Override
          public void onReceive(Context context, Intent intent) {
            finish(); 
          }
    };


    @Override
       protected void onCreate(Bundle savedInstanceState) {
          super.onCreate(savedInstanceState);

          requestWindowFeature(Window.FEATURE_NO_TITLE);
          getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
          getWindow().setFlags(WindowManager.LayoutParams.FLAG_BLUR_BEHIND,
                WindowManager.LayoutParams.FLAG_BLUR_BEHIND);
        
        mContext = this;
        
        mYesNoPopup = new AlertDialog.Builder(mContext);

        mYesNoPopup.setMessage(mContext.getString(com.android.internal.R.string.drd_host_warning_message_for_connection));
        mYesNoPopup.setTitle(mContext.getString(com.android.internal.R.string.drd_host_warning_title_for_connection));
        mYesNoPopup.setIcon(android.R.drawable.ic_dialog_alert);
        
        mYesNoPopup.setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int item) {
                SystemProperties.set("sys.usb.mode", "usb_host");
                finish();
            }
        });

        mYesNoPopup.setNegativeButton(android.R.string.no, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int item) {
                SystemProperties.set("sys.usb.mode", "usb_device");
                finish();
            }
        });

        mYesNoPopup.setOnCancelListener(new DialogInterface.OnCancelListener() {
            public void onCancel(DialogInterface dialog) {
                finish();
            }
        });
        
        mYesNoPopup.show();

        IntentFilter filter = new IntentFilter("android.intent.action.POPUP_CLOSE");
        registerReceiver(HostPopupReceiver, filter);
    }
}
