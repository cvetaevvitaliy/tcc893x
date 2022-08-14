/*
 * Copyright (C) 2013 Telechips, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.telechips.android.tdmb;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.util.Log;
import android.widget.Toast;

public class DxbView_Message
{
	private static final int D = 0;
	private static final int E = 1;
	private static final int I = 2;
	private static final int V = 3;
	private static final int W = 4;
	
	//	Toast
	static Toast mToast = null;
	
	//	Dialog - Scan
	static ProgressDialog mDialog_Scan;
	static final int DIALOG_SCAN = 0;

	//	Dialog	- Preset
	static Builder	mBuilder_Preset;
	static Dialog	mDialog_Preset;

	static void onPause()
	{
		if(		(mDialog_Preset!=null)
			&&	mDialog_Preset.isShowing()
		)
		{
			mDialog_Preset.dismiss();
		}
	}

	static void onDestroy()
	{
		if(mToast != null)
		{
			mToast.cancel();
			mToast = null;
		}
	}
	
	static void removeDialog()
	{
		if(mDialog_Scan!=null && mDialog_Scan.isShowing())
		{
			Manager_Setting.mContext.removeDialog(DIALOG_SCAN);
		}
	}
	
	static void updateToast(String message)
	{
		DxbLog(I, "updateToast(" + message + ")");
		
		if(mToast == null)
		{
			mToast = Toast.makeText(Manager_Setting.mContext, message, 2000);
		}
		else
		{
			mToast.show();
			mToast.setText(message);
		}

		mToast.show();
	}
	
	static void selectAreaCode()
	{
		mBuilder_Preset = new AlertDialog.Builder(Manager_Setting.mContext);

		mBuilder_Preset.setTitle(Manager_Setting.mContext.getResources().getString(R.string.area_code));
		mBuilder_Preset.setSingleChoiceItems(R.array.area_code_select_entries, Manager_Setting.g_Information.cOption.area_code, DxbPlayer.ListenerOnClick_selectAreaCode);
		mBuilder_Preset.setOnCancelListener(DxbPlayer.ListenerOnCancel_selectAreaCode);

		mBuilder_Preset.setPositiveButton(Manager_Setting.mContext.getResources().getString(R.string.cancel), DxbPlayer.ListenerOnClick_selectAreaCode);
		mDialog_Preset = mBuilder_Preset.create();
		mDialog_Preset.show();
	}

	private static void DxbLog(int level, String mString)
	{
		if(DxbView.TAG == null)
		{
			return;
		}
		
		String TAG = "[[[DxbView_Message]]] ";
		if(TAG != null)
		{
			switch(level)
			{
	    		case E:
	    			Log.e(TAG, mString);
	    		break;
	    		
	    		case I:
	    			Log.i(TAG, mString);
	    		break;
	    		
	    		case V:
	    			Log.v(TAG, mString);
	    		break;
	    		
	    		case W:
	    			Log.w(TAG, mString);
	    		break;
	    		
	    		case D:
	    		default:
	    			Log.d(TAG, mString);
	    		break;
			}
		}
	}	
}
