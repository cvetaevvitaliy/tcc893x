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

import com.telechips.android.isdbt.player.DxbPlayer;

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
	private static final int MENU_SCAN			= 7;
	private static final int MENU_MAX			= MENU_SCAN+1;
	
	private View mView;
	private View mBgView;
	private int mBgViewStallCnt;
	
	public class cMenu
	{
		View vMenu;
		ImageView ivMenu;
		TextView tvMenu;
	}
	public cMenu[] mcMenu = new cMenu[MENU_MAX];

	public DxbView_Full_Menu(Context context, View v)
	{
		super(context);
		mView = v;
	
		for (int i = 0; i < MENU_MAX; i++)
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
		int i;
		DxbLog(I, "setVisible(" + v + ")");
		
		if(v)
		{
			if(isRecord())
			{
				if(getPlayer().getRecordState() != DxbPlayer.REC_STATE.STOP)
				{
					mcMenu[MENU_CHANNEL].vMenu.setVisibility(View.GONE);
					mcMenu[MENU_EPG].vMenu.setVisibility(View.VISIBLE);
					mcMenu[MENU_CAPTURE].vMenu.setVisibility(View.GONE);
					mcMenu[MENU_RECORD].vMenu.setVisibility(View.VISIBLE);
					mcMenu[MENU_OPTION].vMenu.setVisibility(View.GONE);
					mcMenu[MENU_INFO].vMenu.setVisibility(View.VISIBLE);
					mcMenu[MENU_SCAN].vMenu.setVisibility(View.GONE);
				}
				else if(getPlayer().isFilePlay() == 0)
				{
					mcMenu[MENU_CHANNEL].vMenu.setVisibility(View.VISIBLE);
					mcMenu[MENU_EPG].vMenu.setVisibility(View.VISIBLE);
					mcMenu[MENU_CAPTURE].vMenu.setVisibility(View.VISIBLE);
					mcMenu[MENU_RECORD].vMenu.setVisibility(View.VISIBLE);
					mcMenu[MENU_OPTION].vMenu.setVisibility(View.VISIBLE);
					mcMenu[MENU_INFO].vMenu.setVisibility(View.GONE);
					//mcMenu[MENU_SCAN].vMenu.setVisibility(View.VISIBLE);
					mcMenu[MENU_SCAN].vMenu.setVisibility(View.GONE);
				}
				else
				{
					mcMenu[MENU_CHANNEL].vMenu.setVisibility(View.VISIBLE);
					mcMenu[MENU_EPG].vMenu.setVisibility(View.GONE);
					mcMenu[MENU_CAPTURE].vMenu.setVisibility(View.GONE);
					mcMenu[MENU_RECORD].vMenu.setVisibility(View.GONE);
					mcMenu[MENU_OPTION].vMenu.setVisibility(View.VISIBLE);
					mcMenu[MENU_INFO].vMenu.setVisibility(View.VISIBLE);
					mcMenu[MENU_SCAN].vMenu.setVisibility(View.GONE);
				}
			}
			else
			{
				mcMenu[MENU_CHANNEL].vMenu.setVisibility(View.VISIBLE);
				mcMenu[MENU_EPG].vMenu.setVisibility(View.VISIBLE);
				mcMenu[MENU_CAPTURE].vMenu.setVisibility(View.VISIBLE);
				mcMenu[MENU_RECORD].vMenu.setVisibility(View.GONE);
				mcMenu[MENU_OPTION].vMenu.setVisibility(View.VISIBLE);
				mcMenu[MENU_INFO].vMenu.setVisibility(View.GONE);
				//mcMenu[MENU_SCAN].vMenu.setVisibility(View.VISIBLE);
				mcMenu[MENU_SCAN].vMenu.setVisibility(View.GONE);
			}

			mBgViewStallCnt = 0;
			for (i=MENU_CHANNEL; i < MENU_MAX; i++) {
				if (mcMenu[i].vMenu.getVisibility() != View.GONE)
					mBgViewStallCnt++;
			}
			if(mBgViewStallCnt==3) mBgView.setBackgroundResource(R.drawable.dxb_portable_toolbar_3bg);
			else if(mBgViewStallCnt==4) mBgView.setBackgroundResource(R.drawable.dxb_portable_toolbar_4bg);
			else if(mBgViewStallCnt==5) mBgView.setBackgroundResource(R.drawable.dxb_portable_toolbar_5bg);
			else if(mBgViewStallCnt==6) mBgView.setBackgroundResource(R.drawable.dxb_portable_toolbar_6bg);
			else if(mBgViewStallCnt==7) mBgView.setBackgroundResource(R.drawable.dxb_portable_toolbar_7bg);
			else mBgView.setBackgroundResource(R.drawable.dxb_portable_toolbar_6bg);
			
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
		mcMenu[7].vMenu = mView.findViewById(R.id.full_menu8_layout);

		//	Menu Icon(ImageView)
		mcMenu[0].ivMenu	= (ImageView)mView.findViewById(R.id.full_menu1_icon);
		mcMenu[1].ivMenu	= (ImageView)mView.findViewById(R.id.full_menu2_icon);
		mcMenu[2].ivMenu	= (ImageView)mView.findViewById(R.id.full_menu3_icon);
		mcMenu[3].ivMenu	= (ImageView)mView.findViewById(R.id.full_menu4_icon);
		mcMenu[4].ivMenu	= (ImageView)mView.findViewById(R.id.full_menu5_icon);
		mcMenu[5].ivMenu	= (ImageView)mView.findViewById(R.id.full_menu6_icon);
		mcMenu[6].ivMenu	= (ImageView)mView.findViewById(R.id.full_menu7_icon);
		mcMenu[7].ivMenu	= (ImageView)mView.findViewById(R.id.full_menu8_icon);
			
		//	Menu Text
		mcMenu[0].tvMenu	= (TextView)mView.findViewById(R.id.full_menu1_text);
		mcMenu[1].tvMenu	= (TextView)mView.findViewById(R.id.full_menu2_text);
		mcMenu[2].tvMenu	= (TextView)mView.findViewById(R.id.full_menu3_text);
		mcMenu[3].tvMenu	= (TextView)mView.findViewById(R.id.full_menu4_text);
		mcMenu[4].tvMenu	= (TextView)mView.findViewById(R.id.full_menu5_text);
		mcMenu[5].tvMenu	= (TextView)mView.findViewById(R.id.full_menu6_text);
		mcMenu[6].tvMenu	= (TextView)mView.findViewById(R.id.full_menu7_text);
		mcMenu[7].tvMenu	= (TextView)mView.findViewById(R.id.full_menu8_text);
	}

	private void setTextSize()
	{
		if (getDisplayWidth() > 800)
		{
			int size = 20;

			if (getDisplayWidth() > 1024)
			{
				size = 25;
			}

			for (int i = 0; i < MENU_MAX; i++)
			{
				mcMenu[i].tvMenu.setTextSize(size / getDisplayDensity());
			}
		}
		else //	if(DxbPlayer_Control.eResolution == DxbPlayer_Control.DISPLAY_RESOLUTION.p480)
		{
			for (int i = 0; i < MENU_MAX; i++)
			{
				mcMenu[i].tvMenu.setTextSize(15 / getDisplayDensity());
			}
		}
	}
	
	private void setMenu()
	{
		for (int i = 0; i < MENU_MAX; i++)
		{
			mcMenu[i].ivMenu.setImageResource(getRdrawable_MenuIcon(i, false));
			mcMenu[i].tvMenu.setText(getRstring_MenuText(i));
		}
	}

	public int getRdrawable_MenuIcon(int index, boolean state)
	{
		int	return_Resource = -1;
		if(index == MENU_CHANNEL) {
			if(state == true) {
				return_Resource	= R.drawable.dxb_portable_toolbar_channel_btn_f;
			} else {
				return_Resource	= R.drawable.dxb_portable_toolbar_channel_btn_n;
			}
		} else if(index == MENU_SCREEN) {
			if(state == true) {
				return_Resource	= R.drawable.dxb_portable_toolbar_screen_btn_f;
			} else {
				return_Resource	= R.drawable.dxb_portable_toolbar_screen_btn_n;
			}
		} else if(index == MENU_EPG) {
			if(state == true) {
				return_Resource	= R.drawable.dxb_portable_toolbar_epg_btn_f;
			} else {
				return_Resource	= R.drawable.dxb_portable_toolbar_epg_btn_n;
			}
		} else if(index == MENU_CAPTURE) {
			if(state == true) {
				return_Resource	= R.drawable.dxb_portable_toolbar_capture_btn_f;
			} else {
				return_Resource	= R.drawable.dxb_portable_toolbar_capture_btn_n;
			}
		} else if(index == MENU_RECORD) {
			if(state == true) {
				return_Resource	= R.drawable.dxb_portable_toolbar_record_btn_f;
			} else {
				return_Resource	= R.drawable.dxb_portable_toolbar_record_btn_n;
			}
		} else if(index == MENU_OPTION) {
			if(state == true) {
				return_Resource	= R.drawable.dxb_portable_toolbar_option_btn_f;
			} else {
				return_Resource	= R.drawable.dxb_portable_toolbar_option_btn_n;
			}
		} else if(index == MENU_INFO) {
			if(state == true) {
				return_Resource	= R.drawable.dxb_portable_toolbar_epg_btn_f;
			} else {
				return_Resource	= R.drawable.dxb_portable_toolbar_epg_btn_n;
			}
		} else if (index == MENU_SCAN) {
			if (state == true)	return_Resource = R.drawable.dxb_portable_toolbar_search_btn_f;
			else 				return_Resource = R.drawable.dxb_portable_toolbar_search_btn_n;
		}
		
		return return_Resource;
	}

	public String getRstring_MenuText(int index)
	{
		if (index == MENU_CHANNEL)	return getResources().getString(R.string.channel);
		else if(index == MENU_SCREEN)	return getResources().getString(R.string.screen);
		else if(index == MENU_EPG)	return getResources().getString(R.string.epg);
		else if(index == MENU_CAPTURE)	return getResources().getString(R.string.capture);
		else if(index == MENU_RECORD)	return getResources().getString(R.string.record);
		else if(index == MENU_OPTION)	return getResources().getString(R.string.option);
		else if(index == MENU_INFO)	return getResources().getString(R.string.info);
		else if(index == MENU_SCAN)	return getResources().getString(R.string.scan);
		
		return null;
	}

	public int getRedrawable_ToolbarStall ()
	{
		int	return_Resource = -1;
		if (mBgViewStallCnt==3)	return_Resource = R.drawable.dxb_portable_toolbar_3bg_01_f;
		else if (mBgViewStallCnt==4) return_Resource = R.drawable.dxb_portable_toolbar_4bg_01_f;
		else if (mBgViewStallCnt==5) return_Resource = R.drawable.dxb_portable_toolbar_5bg_01_f;
		else if (mBgViewStallCnt==6) return_Resource = R.drawable.dxb_portable_toolbar_6bg_01_f;
		else if (mBgViewStallCnt==7) return_Resource = R.drawable.dxb_portable_toolbar_7bg_01_f;
		return return_Resource;
	}
	private void setListener()
	{
		for (int i = 0; i < MENU_MAX; i++)
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
			case R.id.full_menu8_layout:
				return MENU_SCAN;
			default:
				return MENU_NULL;
		}
	}

	public void onFocusChange(View v, boolean hasFocus)
	{
		int id = v.getId();
		int	menu_index	= getIndex_Menu(id);
		int	toolbar_stall;
		if (menu_index != MENU_NULL)
		{
			mcMenu[menu_index].ivMenu.setImageResource(getRdrawable_MenuIcon(menu_index, hasFocus));
			if(hasFocus == true)
			{
				toolbar_stall = getRedrawable_ToolbarStall();
				mcMenu[menu_index].vMenu.setBackgroundResource(toolbar_stall);
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

				case MENU_SCAN:
					mOnEventListener.onEvent(DxbView_Full.EVENT_SCAN);
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
