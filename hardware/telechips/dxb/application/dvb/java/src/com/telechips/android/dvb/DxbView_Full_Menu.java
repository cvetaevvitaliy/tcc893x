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

package com.telechips.android.dvb;

import com.telechips.android.dvb.player.DxbPlayer;
import com.telechips.android.dvb.player.DxbPVR.REC_STATE;

import android.content.Context;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

public class DxbView_Full_Menu extends DxbView implements View.OnClickListener, View.OnFocusChangeListener, View.OnTouchListener
{
	private static final int MENU_NULL			= -1;
	private static final int MENU_CHANNEL		= 0;
	private static final int MENU_SCREEN		= 1;
	private static final int MENU_EPG			= 2;
	private static final int MENU_CAPTURE		= 3;
	private static final int MENU_RECORD		= 4;
	private static final int MENU_OPTION		= 5;
	private static final int MENU_INFO			= 6;
	
	private View mView;
	private View mBgView;
	
	public class cMenu
	{
		View vMenu;
		ImageView ivMenu;
		TextView tvMenu;
	}
	public cMenu[] mcMenu = new cMenu[7];

	public DxbView_Full_Menu(Context context, View v)
	{
		super(context);
		mView = v;
		
		for (int i = 0; i < 7; i++)
		{
			mcMenu[i] = new cMenu();
		}
		
		setComponent();
		setMenu();
		setTextSize();
		setListener();
	}

	public void setVisible(boolean v)
	{
		DxbLog(I, "setVisible(" + v + ")");
		
		if(v)
		{
			if(getPlayer().getRecordState() != REC_STATE.STOP)
			{
				mBgView.setBackgroundResource(R.drawable.dxb_portable_toolbar_5bg);
				//mcMenu[MENU_CHANNEL].vMenu.setVisibility(View.GONE);
				mcMenu[MENU_EPG].vMenu.setVisibility(View.VISIBLE);
				mcMenu[MENU_CAPTURE].vMenu.setVisibility(View.GONE);
				mcMenu[MENU_RECORD].vMenu.setVisibility(View.VISIBLE);
				mcMenu[MENU_OPTION].vMenu.setVisibility(View.GONE);
				mcMenu[MENU_INFO].vMenu.setVisibility(View.VISIBLE);
			}
			else if(getPlayer().isFilePlay() == false)
			{
				mBgView.setBackgroundResource(R.drawable.dxb_portable_toolbar_6bg);
				//mcMenu[MENU_CHANNEL].vMenu.setVisibility(View.VISIBLE);
				mcMenu[MENU_EPG].vMenu.setVisibility(View.VISIBLE);
				mcMenu[MENU_CAPTURE].vMenu.setVisibility(View.VISIBLE);
				mcMenu[MENU_RECORD].vMenu.setVisibility(View.VISIBLE);
				mcMenu[MENU_OPTION].vMenu.setVisibility(View.VISIBLE);
				mcMenu[MENU_INFO].vMenu.setVisibility(View.GONE);
			}
			else
			{
				mBgView.setBackgroundResource(R.drawable.dxb_portable_toolbar_4bg);
				//mcMenu[MENU_CHANNEL].vMenu.setVisibility(View.VISIBLE);
				mcMenu[MENU_EPG].vMenu.setVisibility(View.GONE);
				mcMenu[MENU_CAPTURE].vMenu.setVisibility(View.GONE);
				mcMenu[MENU_RECORD].vMenu.setVisibility(View.GONE);
				mcMenu[MENU_OPTION].vMenu.setVisibility(View.VISIBLE);
				mcMenu[MENU_INFO].vMenu.setVisibility(View.VISIBLE);
			}
			
			mView.setVisibility(View.VISIBLE);
			if(mcMenu[0].vMenu.getVisibility() == View.VISIBLE)
			{
				mcMenu[0].vMenu.requestFocus();
			}
			else
			{
				mcMenu[1].vMenu.requestFocus();
			}
		}
		else
		{
			mView.setVisibility(View.GONE);
		}
	}

	public boolean isShown()
	{
		return mView.getVisibility() == View.VISIBLE;
	}

	private void setComponent()
	{
		mBgView = mView.findViewById(R.id.full_menu_layout);
		
		//	Menu View
		mcMenu[0].vMenu = mView.findViewById(R.id.full_menu1_layout);
		mcMenu[1].vMenu = mView.findViewById(R.id.full_menu2_layout);
		mcMenu[2].vMenu = mView.findViewById(R.id.full_menu3_layout);
		mcMenu[3].vMenu = mView.findViewById(R.id.full_menu4_layout);
		mcMenu[4].vMenu = mView.findViewById(R.id.full_menu5_layout);
		mcMenu[5].vMenu = mView.findViewById(R.id.full_menu6_layout);
		mcMenu[6].vMenu = mView.findViewById(R.id.full_menu7_layout);

		//	Menu Icon(ImageView)
		mcMenu[0].ivMenu	= (ImageView)mView.findViewById(R.id.full_menu1_icon);
		mcMenu[1].ivMenu	= (ImageView)mView.findViewById(R.id.full_menu2_icon);
		mcMenu[2].ivMenu	= (ImageView)mView.findViewById(R.id.full_menu3_icon);
		mcMenu[3].ivMenu	= (ImageView)mView.findViewById(R.id.full_menu4_icon);
		mcMenu[4].ivMenu	= (ImageView)mView.findViewById(R.id.full_menu5_icon);
		mcMenu[5].ivMenu	= (ImageView)mView.findViewById(R.id.full_menu6_icon);
		mcMenu[6].ivMenu	= (ImageView)mView.findViewById(R.id.full_menu7_icon);
			
		//	Menu Text
		mcMenu[0].tvMenu	= (TextView)mView.findViewById(R.id.full_menu1_text);
		mcMenu[1].tvMenu	= (TextView)mView.findViewById(R.id.full_menu2_text);
		mcMenu[2].tvMenu	= (TextView)mView.findViewById(R.id.full_menu3_text);
		mcMenu[3].tvMenu	= (TextView)mView.findViewById(R.id.full_menu4_text);
		mcMenu[4].tvMenu	= (TextView)mView.findViewById(R.id.full_menu5_text);
		mcMenu[5].tvMenu	= (TextView)mView.findViewById(R.id.full_menu6_text);
		mcMenu[6].tvMenu	= (TextView)mView.findViewById(R.id.full_menu7_text);
	}

	private void setTextSize()
	{
		for (int i = 0; i < 7; i++)
		{
			mcMenu[i].tvMenu.setTextSize(getTextSize(15));
		}
	}
	
	private void setMenu()
	{
		for (int i = 0; i < 7; i++)
		{
			mcMenu[i].ivMenu.setImageResource(getRdrawable_MenuIcon(i, false));
			mcMenu[i].tvMenu.setText(getRstring_MenuText(i));
		}
	}

	public int getRdrawable_MenuIcon(int index, boolean state)
	{
		int	return_Resource = -1;
		if(index == MENU_CHANNEL)
		{
			if(state == true)
			{
				return_Resource	= R.drawable.dxb_portable_toolbar_channel_btn_f;
			}
			else
			{
				return_Resource	= R.drawable.dxb_portable_toolbar_channel_btn_n;
			}
		}
		else if(index == MENU_SCREEN)
		{
			if(state == true)
			{
				return_Resource	= R.drawable.dxb_portable_toolbar_screen_btn_f;
			}
			else
			{
				return_Resource	= R.drawable.dxb_portable_toolbar_screen_btn_n;
			}
		} else if(index == MENU_EPG) {
			if(state == true) {
				return_Resource	= R.drawable.dxb_portable_toolbar_epg_btn_f;
			} else {
				return_Resource	= R.drawable.dxb_portable_toolbar_epg_btn_n;
			}
		}
		else if(index == MENU_CAPTURE)
		{
			if(state == true)
			{
				return_Resource	= R.drawable.dxb_portable_toolbar_capture_btn_f;
			}
			else
			{
				return_Resource	= R.drawable.dxb_portable_toolbar_capture_btn_n;
			}
		}
		else if(index == MENU_RECORD)
		{
			if(state == true)
			{
				return_Resource	= R.drawable.dxb_portable_toolbar_record_btn_f;
			}
			else
			{
				return_Resource	= R.drawable.dxb_portable_toolbar_record_btn_n;
			}
		}
		else if(index == MENU_OPTION)
		{
			if(state == true)
			{
				return_Resource	= R.drawable.dxb_portable_toolbar_option_btn_f;
			}
			else
			{
				return_Resource	= R.drawable.dxb_portable_toolbar_option_btn_n;
			}
		}
		else if(index == MENU_INFO)
		{
			if(state == true)
			{
				return_Resource	= R.drawable.dxb_portable_toolbar_epg_btn_f;
			}
			else
			{
				return_Resource	= R.drawable.dxb_portable_toolbar_epg_btn_n;
			}
		}
		
		return return_Resource;
	}

	public String getRstring_MenuText(int index)
	{
		if (index == MENU_CHANNEL)
		{
			return getResources().getString(R.string.channel);
		}
		else if(index == MENU_SCREEN)
		{
			return getResources().getString(R.string.screen);
		}
		else if(index == MENU_EPG)
		{
			return getResources().getString(R.string.epg);
		}
		else if(index == MENU_CAPTURE)
		{
			return getResources().getString(R.string.capture);
		}
		else if(index == MENU_RECORD)
		{
			return getResources().getString(R.string.record);
		}
		else if(index == MENU_OPTION)
		{
			return getResources().getString(R.string.option);
		}
		else if(index == MENU_INFO)
		{
			return getResources().getString(R.string.info);
		}
		
		return null;
	}

	private void setListener()
	{
		for (int i = 0; i < 7; i++)
		{
			mcMenu[i].vMenu.setOnClickListener(this);
			mcMenu[i].vMenu.setOnFocusChangeListener(this);
			mcMenu[i].vMenu.setOnTouchListener(this);
		}
	}

	private int getIndex_Menu(int id)
	{
		DxbLog(I, "getIndex_Menu(id=" + id + ")");
		
		switch (id)
		{
			case R.id.full_menu1_layout:
				return MENU_CHANNEL;
	
			case R.id.full_menu2_layout:
				return MENU_SCREEN;
	
			case R.id.full_menu3_layout:
				return MENU_EPG;
	
			case R.id.full_menu4_layout:
				return MENU_CAPTURE;
	
			case R.id.full_menu5_layout:
				return MENU_RECORD;
	
			case R.id.full_menu6_layout:
				return MENU_OPTION;
	
			case R.id.full_menu7_layout:
				return MENU_INFO;
	
			default:
				return MENU_NULL;
		}
	}

	public void onFocusChange(View v, boolean hasFocus)
	{
		int id = v.getId();
		int	menu_index	= getIndex_Menu(id);
		if (menu_index != MENU_NULL)
		{
			mcMenu[menu_index].ivMenu.setImageResource(getRdrawable_MenuIcon(menu_index, hasFocus));
			if(hasFocus == true)
			{
				mcMenu[menu_index].vMenu.setBackgroundResource(R.drawable.dxb_portable_toolbar_6bg_01_f);
				mcMenu[menu_index].tvMenu.setTextColor(getResources().getColor(R.color.color_4));
			}
			else
			{
				mcMenu[menu_index].vMenu.setBackgroundColor(getResources().getColor(R.color.color_0));
				mcMenu[menu_index].tvMenu.setTextColor(getResources().getColor(R.color.color_9));
			}
		}
	}

	public void onClick(View v)
	{
		int menu_index = getIndex_Menu(v.getId());
		if (mOnEventListener != null)
		{
			switch (menu_index)
			{
				case MENU_CHANNEL:
					mOnEventListener.onEvent(DxbView_Full.EVENT_CHANNEL);
				break;
				
				case MENU_SCREEN:
					mOnEventListener.onEvent(DxbView_Full.EVENT_SCREENMODE);
				break;
				
				case MENU_EPG:
					mOnEventListener.onEvent(DxbView_Full.EVENT_EPG);
				break;
				
				case MENU_CAPTURE:
					mOnEventListener.onEvent(DxbView_Full.EVENT_CAPTURE);
				break;
				
				case MENU_RECORD:
					mOnEventListener.onEvent(DxbView_Full.EVENT_RECORD);
				break;
				
				case MENU_OPTION:
					mOnEventListener.onEvent(DxbView_Full.EVENT_OPTION);
				break;
				
				case MENU_INFO:
					mOnEventListener.onEvent(DxbView_Full.EVENT_INFORMATION);
				break;
				
				default:
					mOnEventListener.onEvent(DxbView_Full.EVENT_NULL);
				break;
			}
		}
	}

	public boolean onTouch(View v, MotionEvent event) {
		if(event.getAction() == MotionEvent.ACTION_UP) {
			onClick(v);
		} else if (event.getAction() == MotionEvent.ACTION_DOWN) {
			v.requestFocus();
		}
		return true;
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		if (keyCode == KeyEvent.KEYCODE_BACK || keyCode == KeyEvent.KEYCODE_ESCAPE)
		{
			setVisible(false);
			return true;
		}
		return false;
	}

    private OnEventListener mOnEventListener;
    public interface OnEventListener {
        void onEvent(int event);
    }

    public void setOnEventListener(OnEventListener listener) {
    	mOnEventListener = listener;
    }
}
