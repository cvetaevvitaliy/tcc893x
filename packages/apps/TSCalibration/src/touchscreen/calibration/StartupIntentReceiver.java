/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Code Aurora nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

package touchscreen.calibration;

import java.io.FileReader;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import touchscreen.calibration.TouchCalibration;

// Added to allow for automatic startup
public class StartupIntentReceiver extends BroadcastReceiver {

    protected class MyException extends Exception {
        protected MyException(String msg){
            super(msg);
        }
    }

         @Override
         public void onReceive(Context context, Intent intent) {

         try{
             String savedHeight = "";
             String savedWidth = "";
             String firstChar ="";
             boolean lineTwo = false;
             int count, i = 0;
             char[] buf = new char[100];
             FileReader rd = new FileReader(TouchCalibration.filePath);

             count = rd.read(buf, 0, 99);
             buf[count] = '\n';
             String currentPointerCalValues = new String(buf, 0, count);

             //if ((currentPointerCalValues.compareTo(TouchCalibration.defaultPointercalValues)) == 0)
             //   throw new MyException("Default Pointercal Detected, Launching TSLIB");

             int height = context.getResources().getDisplayMetrics().heightPixels;
             int width = context.getResources().getDisplayMetrics().widthPixels;

             Log.i("Calibration Start Up Receiver", "Calibration File Exists Skipping Launch Of TS Cal");
         }
         catch(Exception e){
            Log.e("Calibration Start Up Receiver: EXCEPTION ", e.toString());
            Log.i("Calibration Start Up Receiver", "Calibration File Does Not Exist, Launching TS Cal");

            Intent starterIntent = new Intent(context, TouchCalibration.class);
            //Set launch flags pertaining to M6 release
            starterIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(starterIntent);
         };
    }
}
