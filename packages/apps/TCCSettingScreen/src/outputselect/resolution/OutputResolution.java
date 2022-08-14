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

package outputselect.resolution;

import java.util.Timer;
import java.util.TimerTask;
import java.util.HashSet;
import java.util.Set;

import android.app.Activity;   
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.database.ContentObserver;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.view.LayoutInflater;
import android.view.View;
import android.view.Window;
import android.widget.TextView;
import android.widget.ImageView;
import android.widget.Button;
import android.graphics.Color;
import android.util.Log;

public class OutputResolution extends Activity implements View.OnClickListener{
    private final static String TAG = "Output_Resoltion_Select";

    private Button mSelectButton;
    private TextView mBanner;
    private TextView mMessage;
    private ImageView mIcon;

    private final static int UID_UPDATE = 0;
    private final static int COUNT_UPDATE = 1;
    private final static int INTERRUPT = 2;

    private static final String AUTO_RESOLUTION = "1";
    private static final String OUTPUT_MODE = "output_mode";
    private static final String HDMI_RESOLUTION = "hdmi_resolution";
    private static final String HDMI_MODE = "hdmi_mode";
    private static final String COMPOSITE_MODE = "composite_mode";
    private static final String COMPONENT_RESOLUTION = "component_resolution";

    private boolean mHDMI_active;
    private boolean mComposite_active;
    private boolean mComponent_active;
	
    private static int output_mode = 1;
    private static int prev_output_mode = 1;
    private static int hdmi_resolution = 125;
    private static int hdmi_mode = 0;
    private static int composite_mode = 0;
    private static int component_resolution = 1;
    private static int output_count = 1;
    private static int display_mode = 0;

    private static String key_output_mode = "HDMI";
    private static String key_resolution = "Auto Detect";

    private int message_count = 11;

     /*
     * File Creation for storing the resolution file
     */
    private Handler handler = new Handler() {   
	@Override  
	public void handleMessage(Message msg) {   
		switch (msg.what) {   
			case UID_UPDATE:
				if(display_mode == 1) {
					updateAutoDetect();
				} else if(display_mode == 2 || display_mode == 3) {
					updateAttachMode();
				} else {
					updateState();
				}
				mMessage.setText(key_output_mode + "  " + key_resolution);
				handler.sendEmptyMessageDelayed(COUNT_UPDATE,1000);   
				break;
			case COUNT_UPDATE:
				message_count--;
				mBanner.setText("This setting will be reset after " + message_count + " seconds." + "\n\n" + "Press Ok button if you can see this message.");

				if(SystemProperties.getInt("persist.sys.auto_resolution",1) == 1){
					handler.sendEmptyMessageDelayed(INTERRUPT,0);
				} else if(message_count == 0){
					message_count = 12;
					handler.sendEmptyMessageDelayed(UID_UPDATE,0);
				} else{
					handler.sendEmptyMessageDelayed(COUNT_UPDATE,1000);
				}
				break;
			case INTERRUPT:
				Log.e(TAG, "Finishing the Application");
				System.exit(0);
				break;
			default:
				break;
		}
		super.handleMessage(msg);
	}
   };

   @Override
   protected void onCreate(Bundle savedInstanceState) {
	super.onCreate(savedInstanceState);

	ContentResolver resolver = getContentResolver();

	setContentView(R.layout.output_activity);

	mIcon = (ImageView) findViewById(R.id.icon);
	mBanner = (TextView) findViewById(R.id.banner);
	mBanner.setText("This setting will be reset after " + message_count + " seconds." + "\n\n" + "Press Ok button if you can see this message.\n\n\n");
	mMessage = (TextView) findViewById(R.id.message);
	mMessage.setTextColor(Color.RED);
	mMessage.setText(key_output_mode + "   " + key_resolution);

	mSelectButton = (Button) findViewById(R.id.ok_button);
	mSelectButton.setOnClickListener(this);

	mHDMI_active = "true".equals(SystemProperties.get("ro.system.hdmi_active"));

       mComposite_active = "true".equals(SystemProperties.get("ro.system.composite_active"));

       mComponent_active = "true".equals(SystemProperties.get("ro.system.component_active"));

	int display_type = SystemProperties.getInt("tcc.output.display.type", 1);

	display_mode = SystemProperties.getInt("persist.sys.display.mode", 1);

	if(display_type == 2){
		mComposite_active = false;
		mComponent_active = false;
	} else if(display_type == 1){
		mComponent_active = false;
	}

	Settings.System.putInt(getContentResolver(), OUTPUT_MODE, output_mode);
	Settings.System.putInt(getContentResolver(), HDMI_MODE, hdmi_mode);
	Settings.System.putInt(getContentResolver(), HDMI_RESOLUTION, hdmi_resolution);

	handler.sendEmptyMessageDelayed(COUNT_UPDATE,0);
   }

   @Override
   protected void onDestroy() {
	super.onDestroy();
	handler.removeCallbacksAndMessages(null);
   }

    @Override
    public void onBackPressed(){}

   public void onClick(View v) {
              SystemProperties.set("persist.sys.auto_resolution", AUTO_RESOLUTION);
    }

    private void updateState() {
		if(output_mode == 1){
			if(output_count == 0){
				output_count = 1;
				hdmi_mode = 0;
				hdmi_resolution = 125;
				Settings.System.putInt(getContentResolver(), OUTPUT_MODE, output_mode);
				Settings.System.putInt(getContentResolver(), HDMI_MODE, hdmi_mode);
				Settings.System.putInt(getContentResolver(), HDMI_RESOLUTION, hdmi_resolution);
				Settings.System.putInt(getContentResolver(), COMPONENT_RESOLUTION, 0);
				updateSummary(output_mode,hdmi_resolution);
			} else if(output_count == 1){
				output_count = 2;
				hdmi_resolution = 8;
				Settings.System.putInt(getContentResolver(), HDMI_RESOLUTION, hdmi_resolution);
				updateSummary(output_mode,hdmi_resolution);
			}else if(output_count == 2){
				output_count = 0;
				hdmi_mode = 1;
				Settings.System.putInt(getContentResolver(), HDMI_MODE, hdmi_mode);
				updateSummary(output_mode,hdmi_resolution);
				if(mComposite_active)
					output_mode++;
				else
					output_mode = 1;
			}
		} else if(output_mode == 2){
			if(output_count == 0){
				output_count = 1;
				composite_mode = 0;
				Settings.System.putInt(getContentResolver(), OUTPUT_MODE, output_mode);
				Settings.System.putInt(getContentResolver(), COMPOSITE_MODE, composite_mode);
				Settings.System.putInt(getContentResolver(), HDMI_RESOLUTION, 125);
				Settings.System.putInt(getContentResolver(), HDMI_MODE, 0);
				updateSummary(output_mode,composite_mode);
			}else if(output_count == 1){
				output_count = 0;
				composite_mode = 1;
				Settings.System.putInt(getContentResolver(), OUTPUT_MODE, output_mode);
				Settings.System.putInt(getContentResolver(), COMPOSITE_MODE, composite_mode);
				updateSummary(output_mode,composite_mode);

				if(mComponent_active)
					output_mode++;
				else
					output_mode = 1;
			}
		} else if(output_mode == 3){
			component_resolution = 1;
			Settings.System.putInt(getContentResolver(), OUTPUT_MODE, output_mode);
			Settings.System.putInt(getContentResolver(), COMPONENT_RESOLUTION, component_resolution);
			Settings.System.putInt(getContentResolver(), COMPOSITE_MODE, 0);
			updateSummary(output_mode,component_resolution);
			output_mode = 1;
		}
    }

    private void updateAutoDetect() {
		output_mode = SystemProperties.getInt("persist.sys.output_mode", 1);
		if(output_mode != prev_output_mode)	{
			prev_output_mode = output_mode;
			output_count = 0;
		}

		if(output_mode == 1){
			if(output_count == 0){
				output_count = 1;
				hdmi_mode = 0;
				hdmi_resolution = 125;
				Settings.System.putInt(getContentResolver(), OUTPUT_MODE, output_mode);
				Settings.System.putInt(getContentResolver(), HDMI_MODE, hdmi_mode);
				Settings.System.putInt(getContentResolver(), HDMI_RESOLUTION, hdmi_resolution);
				Settings.System.putInt(getContentResolver(), COMPONENT_RESOLUTION, 0);
				updateSummary(output_mode,hdmi_resolution);
			} else if(output_count == 1){
				output_count = 2;
				hdmi_resolution = 8;
				Settings.System.putInt(getContentResolver(), HDMI_RESOLUTION, hdmi_resolution);
				updateSummary(output_mode,hdmi_resolution);
			}else if(output_count == 2){
				output_count = 0;
				hdmi_mode = 1;
				Settings.System.putInt(getContentResolver(), HDMI_MODE, hdmi_mode);
				updateSummary(output_mode,hdmi_resolution);
			}
		} else if(output_mode == 2){
			if(output_count == 0){
				output_count = 1;
				composite_mode = 0;
				Settings.System.putInt(getContentResolver(), OUTPUT_MODE, output_mode);
				Settings.System.putInt(getContentResolver(), COMPOSITE_MODE, composite_mode);
				Settings.System.putInt(getContentResolver(), HDMI_RESOLUTION, 125);
				Settings.System.putInt(getContentResolver(), HDMI_MODE, 0);
				updateSummary(output_mode,composite_mode);
			}else if(output_count == 1){
				output_count = 0;
				composite_mode = 1;
				Settings.System.putInt(getContentResolver(), OUTPUT_MODE, output_mode);
				Settings.System.putInt(getContentResolver(), COMPOSITE_MODE, composite_mode);
				updateSummary(output_mode,composite_mode);
			}
		}
    }

    private void updateAttachMode() {
		output_mode = SystemProperties.getInt("persist.sys.output_mode", 1);
		if(output_mode != prev_output_mode)	{
			prev_output_mode = output_mode;
			output_count = 0;
		}

		if(display_mode == 3)
		{
			if(output_mode == 1) {
				if(output_count == 0) {
					output_count++;
					hdmi_mode = 0;
					hdmi_resolution = 125;
					Settings.System.putInt(getContentResolver(), OUTPUT_MODE, output_mode);
					Settings.System.putInt(getContentResolver(), HDMI_MODE, hdmi_mode);
					Settings.System.putInt(getContentResolver(), HDMI_RESOLUTION, hdmi_resolution);
					Settings.System.putInt(getContentResolver(), COMPONENT_RESOLUTION, 0);
					updateSummary(output_mode,hdmi_resolution);
				} else if(output_count == 1) {
					output_count++;
					hdmi_resolution = 8;
					Settings.System.putInt(getContentResolver(), HDMI_RESOLUTION, hdmi_resolution);
					updateSummary(output_mode,hdmi_resolution);
				} else if(output_count == 2) {
					output_count++;
					hdmi_mode = 1;
					Settings.System.putInt(getContentResolver(), HDMI_MODE, hdmi_mode);
					updateSummary(output_mode,hdmi_resolution);
				} else if(output_count == 3) {
					output_count++;
					composite_mode = 0;
					//Settings.System.putInt(getContentResolver(), OUTPUT_MODE, 2);
					Settings.System.putInt(getContentResolver(), COMPOSITE_MODE, composite_mode);
					updateSummary(2,composite_mode);
				} else if(output_count == 4) {
					output_count = 0;
					composite_mode = 1;
					//Settings.System.putInt(getContentResolver(), OUTPUT_MODE, 2);
					Settings.System.putInt(getContentResolver(), COMPOSITE_MODE, composite_mode);
					updateSummary(2,composite_mode);
				}
			} else if(output_mode == 3) {
				if(output_count == 0) {
					output_count++;
					component_resolution = 1;
					Settings.System.putInt(getContentResolver(), OUTPUT_MODE, output_mode);
					Settings.System.putInt(getContentResolver(), COMPONENT_RESOLUTION, component_resolution);
					Settings.System.putInt(getContentResolver(), COMPOSITE_MODE, 0);
					updateSummary(output_mode,component_resolution);
				} else if(output_count == 1) {
					output_count++;
					composite_mode = 0;
					//Settings.System.putInt(getContentResolver(), OUTPUT_MODE, 2);
					Settings.System.putInt(getContentResolver(), COMPOSITE_MODE, composite_mode);
					updateSummary(2,composite_mode);
				} else if(output_count == 2) {
					output_count = 0;
					composite_mode = 1;
					//Settings.System.putInt(getContentResolver(), OUTPUT_MODE, 2);
					Settings.System.putInt(getContentResolver(), COMPOSITE_MODE, composite_mode);
					updateSummary(2,composite_mode);
				}
			}
		}else if(display_mode == 2) {
			if(output_mode == 1) {
				if(output_count == 0) {
					output_count++;
					hdmi_mode = 0;
					hdmi_resolution = 125;
					Settings.System.putInt(getContentResolver(), OUTPUT_MODE, output_mode);
					Settings.System.putInt(getContentResolver(), HDMI_MODE, hdmi_mode);
					Settings.System.putInt(getContentResolver(), HDMI_RESOLUTION, hdmi_resolution);
					Settings.System.putInt(getContentResolver(), COMPONENT_RESOLUTION, 0);
					updateSummary(output_mode,hdmi_resolution);
				} else if(output_count == 1) {
					output_count++;
					hdmi_resolution = 8;
					Settings.System.putInt(getContentResolver(), HDMI_RESOLUTION, hdmi_resolution);
					updateSummary(output_mode,hdmi_resolution);
				} else if(output_count == 2) {
					output_count++;
					hdmi_mode = 1;
					Settings.System.putInt(getContentResolver(), HDMI_MODE, hdmi_mode);
					updateSummary(output_mode,hdmi_resolution);
				} else if(output_count == 3) {
					output_count++;
					composite_mode = 0;
					//Settings.System.putInt(getContentResolver(), OUTPUT_MODE, 2);
					Settings.System.putInt(getContentResolver(), COMPOSITE_MODE, composite_mode);
					updateSummary(2,composite_mode);
				} else if(output_count == 4) {
					output_count = 0;
					composite_mode = 1;
					//Settings.System.putInt(getContentResolver(), OUTPUT_MODE, 2);
					Settings.System.putInt(getContentResolver(), COMPOSITE_MODE, composite_mode);
					updateSummary(2,composite_mode);
				}
			} else if(output_mode == 2) {
				if(output_count == 0) {
					output_count++;
					composite_mode = 0;
					Settings.System.putInt(getContentResolver(), OUTPUT_MODE, output_mode);
					Settings.System.putInt(getContentResolver(), COMPOSITE_MODE, composite_mode);
					updateSummary(output_mode,composite_mode);
				} else if(output_count == 1) {
					output_count = 0;
					composite_mode = 1;
					Settings.System.putInt(getContentResolver(), OUTPUT_MODE, output_mode);
					Settings.System.putInt(getContentResolver(), COMPOSITE_MODE, composite_mode);
					updateSummary(output_mode,composite_mode);
				}
			}
		}
    }

   private void updateSummary(int output, int resolution) {
	if(output == 1){
		CharSequence[] HDMI_Mode_Summaries = getResources().getTextArray(R.array.hdmi_mode_summaries);
		CharSequence[] HDMI_Summaries = getResources().getTextArray(R.array.hdmi_resolution_summaries);
		key_output_mode = (String)HDMI_Mode_Summaries[hdmi_mode];
		if(resolution == 125){
			key_resolution = (String)HDMI_Summaries[9];
		}
		else{
			key_resolution = (String)HDMI_Summaries[resolution];
		}
	} else if(output == 2){
		CharSequence[] Output_Summaries = getResources().getTextArray(R.array.output_mode_summaries);
		CharSequence[] Composite_Summaries = getResources().getTextArray(R.array.composite_mode_summaries);
		key_output_mode = (String)Output_Summaries[output];
		key_resolution = (String)Composite_Summaries[resolution];
	} else if(output == 3){
		CharSequence[] Output_Summaries = getResources().getTextArray(R.array.output_mode_summaries);
		CharSequence[] Component_Summaries = getResources().getTextArray(R.array.component_resolution_summaries);
		key_output_mode = (String)Output_Summaries[output];
		key_resolution = (String)Component_Summaries[resolution];
	}
   }
}
