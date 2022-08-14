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
package com.telechips.android.isdbt;

import android.content.Context;

import android.view.View;
import android.widget.TextView;

import com.telechips.android.isdbt.player.*;
import com.telechips.android.isdbt.player.isdbt.DebugModeData;

public class DxbView_Debug extends DxbView implements DxbPlayer.OnDebugModeListener {

	private final View mView;

	private TextView mtvLine1;
	private TextView mtvLine2;
	private TextView mtvLine3;

	public DxbView_Debug(Context context, View v) {
		super(context);
		mView = v;
		setComponent();
		setTextSize();
		getPlayer().setOnDebugModeListener(this);
	}	
	
	public void setComponent()
	{
		mtvLine1	= (TextView)mView.findViewById(R.id.debug_line1);
		mtvLine2	= (TextView)mView.findViewById(R.id.debug_line2);
		mtvLine3	= (TextView)mView.findViewById(R.id.debug_line3);
		
		clear();
	}
	
	private void setTextSize()
	{
		float	fFont	= getDisplayWidth() * 18 / 800 / getDisplayDensity();
		
		mtvLine1.setTextSize(fFont);
		mtvLine2.setTextSize(fFont);
		mtvLine3.setTextSize(fFont);

		mtvLine1.setText("");
		mtvLine2.setText("");
		mtvLine3.setText("");
	}

	/*static private String toString_(byte[] data)
	{
		char Display_char;
		String Display_str	= "";

		for(int i=0 ; i<50 ; i++)
		{
			Display_char	= (char) data[i];

			String	sTemp	= new Character(Display_char).toString();
			Display_str	= Display_str + sTemp;
		}

		return Display_str;
	}*/
	
	public void clear()
	{
		mtvLine1.setText("");
		mtvLine2.setText("");
		mtvLine3.setText("");		
		
		mtvLine1.setVisibility(View.GONE);
		mtvLine2.setVisibility(View.GONE);
		mtvLine3.setVisibility(View.GONE);
	}

	public void display(int index, String msg)
	{
		TextView	tv;
		switch(index)
		{
			case 0:
				tv	= mtvLine1;
			break;

			case 1:
				tv	= mtvLine2;
			break;

			case 2:
			default:
				tv	= mtvLine3;
		}

		tv.setText(msg);
	}
	
	public void display(String sLine1)
	{
		mtvLine1.setVisibility(View.VISIBLE);

		mtvLine1.setText(sLine1);
	}
	
	public void display(String sLine1, String sLine2)
	{
		mtvLine1.setVisibility(View.VISIBLE);
		mtvLine2.setVisibility(View.VISIBLE);

		mtvLine1.setText(sLine1);
		mtvLine2.setText(sLine2);
	}
	
	public void display(String sLine1, String sLine2, String sLine3)
	{
		mtvLine1.setVisibility(View.VISIBLE);
		mtvLine2.setVisibility(View.VISIBLE);
		mtvLine3.setVisibility(View.VISIBLE);

		mtvLine1.setText(sLine1);
		mtvLine2.setText(sLine2);
		mtvLine3.setText(sLine3);
	}

	@Override
	protected String getClassName() {
		return "[[[DxbView_Debug]]]";
	}

	public void onDebugUpdate(DxbPlayer player, DebugModeData obj) {
		DxbLog(I, "onDebugUpdate()");

		String	str1	= (String)mtvLine1.getText();
		String	str2	= (String)mtvLine2.getText();
		String	str3	= (String)mtvLine3.getText();

		if(obj.iIndex == 0)
		{
			if(obj.iLength == 0)
			{
				str1 = "";
			}
			else
			{
				str1 = new String(obj.msg, 0, obj.iLength);
			}
			DxbLog(D, obj.iLength + "  ----->  " + str1);
		}
		else if(obj.iIndex ==1)
		{
			if(obj.iLength == 0)
			{
				str2 = "";
			}
			else
			{
				str2 = new String(obj.msg, 0, obj.iLength);
			}
			DxbLog(D, obj.iLength + "  ----->  " + str2);
		}
		else if(obj.iIndex ==2)
		{
			if(obj.iLength == 0)
			{
				str3 = "";
			}
			else
			{
				str3 = new String(obj.msg, 0, obj.iLength);
			}
			DxbLog(D, obj.iLength + "  ----->  " + str3);
		}

		if(		(str1 == "")
			&&	(str2 == "")
			&&	(str3 == "")
		)
		{
			clear();
		}
		else
		{
			DxbLog(D, str1);
			DxbLog(D, str2);
			DxbLog(D, str3);
			
			mtvLine1.setVisibility(View.VISIBLE);
			mtvLine2.setVisibility(View.VISIBLE);
			mtvLine3.setVisibility(View.VISIBLE);
			
			display(0, str1);
			display(1, str2);
			display(2, str3);
		}
	}
}
