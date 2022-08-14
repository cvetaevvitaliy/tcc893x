package com.android.server;

import com.android.server.power.PowerManagerService;
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

public class QBPopup extends Activity {
    private Context mContext;
    AlertDialog.Builder mYesNoPopup = null;
    int mWaitTime = 5;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if(SystemProperties.getInt("tcc.solution.mbox",1) == 1){
            if(SystemProperties.getInt("persist.sys.auto_resolution",1) != 1){
                finish();
            }	
        }
		
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_BLUR_BEHIND,
                WindowManager.LayoutParams.FLAG_BLUR_BEHIND);
        
        mContext = this;
        
        mYesNoPopup = new AlertDialog.Builder(mContext);

        mYesNoPopup.setMessage(mContext.getString(com.android.internal.R.string.quickboot_warning_message_for_making));
        mYesNoPopup.setTitle(mContext.getString(com.android.internal.R.string.quickboot_warning_title_for_making));
        mYesNoPopup.setIcon(android.R.drawable.ic_dialog_alert);
        
        mYesNoPopup.setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int item) {
                SystemProperties.set("persist.sys.QB.user.allow", "true");
                
                PowerManagerService pms = (PowerManagerService) ServiceManager.getService("power");
                pms.reboot(false, "make quickboot image", false);
            }
        });

        mYesNoPopup.setNegativeButton(android.R.string.no, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int item) {
                SystemProperties.set("persist.sys.QB.user.allow", "false");
                finish();
            }
        });

        mYesNoPopup.setOnCancelListener(new DialogInterface.OnCancelListener() {
            public void onCancel(DialogInterface dialog) {
                finish();
            }
        });
        
        mYesNoPopup.show();
    }
}
