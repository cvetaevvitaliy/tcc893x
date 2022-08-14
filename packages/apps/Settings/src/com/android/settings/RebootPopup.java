package com.android.settings;

import android.os.ServiceManager;
import android.os.PowerManager;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.Window;
import android.view.WindowManager;

import com.android.settings.R;

public class RebootPopup extends Activity {
    static final String TAG = "Reboot";
    private Context mContext;
    AlertDialog.Builder mYesNoPopup = null;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

       requestWindowFeature(Window.FEATURE_NO_TITLE);
       getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
       getWindow().setFlags(WindowManager.LayoutParams.FLAG_BLUR_BEHIND,
                WindowManager.LayoutParams.FLAG_BLUR_BEHIND);
        
        mContext = this;
        
        mYesNoPopup = new AlertDialog.Builder(mContext);

        mYesNoPopup.setMessage(this.getResources().getString(R.string.system_reboot_warning_message));
        mYesNoPopup.setTitle(R.string.system_reboot_warning_title);
        mYesNoPopup.setIcon(android.R.drawable.ic_dialog_alert);
        
        mYesNoPopup.setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int item) {
			PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
			pm.reboot(null);

            }
        });

        mYesNoPopup.setNegativeButton(android.R.string.no, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int item) {
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
