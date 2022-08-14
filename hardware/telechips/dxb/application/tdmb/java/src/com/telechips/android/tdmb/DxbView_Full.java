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

import android.content.Intent;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.view.View.OnFocusChangeListener;
import android.view.View.OnTouchListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

public class DxbView_Full
{
	private static final int D = 0;
	private static final int E = 1;
	private static final int I = 2;
	private static final int V = 3;
	private static final int W = 4;
	
	private static Component.cFullView	gComponent	= null;
	private static Information.FULL		gInformation	= null;
	private static Information.OPTION	gInfo_Option	= null;
	
	public static final int	MENU_NULL		= -1;
	public static final int	MENU_CHANNEL	= 1;
	public static final int	MENU_SCREEN		= 2;
	public static final int	MENU_EPG		= 3;
	public static final int	MENU_CAPTURE	= 4;
	public static final int	MENU_RECORD		= 5;
	public static final int	MENU_OPTION		= 6;
	
	// Screen size
	public static final int SCREENMODE_LETTERBOX	= 0;
	public static final int SCREENMODE_PANSCAN	= 1;
	public static final int SCREENMODE_FULL	= 2;
	
	static int	 eState_ScreenSize	= SCREENMODE_FULL;

	static public void init()
	{
		DxbLog(I, "init()");
		
		gComponent		= Manager_Setting.g_Component.fullview;
		gInformation	= Manager_Setting.g_Information.cFULL;
		gInfo_Option	= Manager_Setting.g_Information.cOption;
		
		setComponent();
		setVisibility();
		
		setMenu();
		setTextSize();
		
		setListener();
	}
	
	static private void setComponent()
	{
		gComponent.navi.menu.vLayout	= Manager_Setting.mContext.findViewById(R.id.layout_menu);
		
		/*	Video	*/
			gComponent.vVideo	= Manager_Setting.mContext.findViewById(R.id.full_view);
		
		/*	Navi	*/
			//	ViewGroup
			gComponent.navi.vgNavi = (ViewGroup) Manager_Setting.mContext.findViewById(R.id.layout_full_group);

			//	Title
			gComponent.navi.tvTitle		= (TextView)Manager_Setting.mContext.findViewById(R.id.full_title);		
			gComponent.navi.bTeletext	= (Button)Manager_Setting.mContext.findViewById(R.id.full_title_ttx);
			
			//	(Channel) Up/Down
			gComponent.navi.bUp			= (Button)Manager_Setting.mContext.findViewById(R.id.full_ch_up);
			gComponent.navi.bDown		= (Button)Manager_Setting.mContext.findViewById(R.id.full_ch_down);
		
			//	Menu View
			gComponent.navi.menu.vMenu5	= Manager_Setting.mContext.findViewById(R.id.full_menu5_layout);
			gComponent.navi.menu.vMenu6	= Manager_Setting.mContext.findViewById(R.id.full_menu6_layout);
			
			//	Menu Button
			gComponent.navi.menu.bMenu1	= (Button)Manager_Setting.mContext.findViewById(R.id.full_menu1);
			gComponent.navi.menu.bMenu2	= (Button)Manager_Setting.mContext.findViewById(R.id.full_menu2);
			gComponent.navi.menu.bMenu3	= (Button)Manager_Setting.mContext.findViewById(R.id.full_menu3);
			gComponent.navi.menu.bMenu4	= (Button)Manager_Setting.mContext.findViewById(R.id.full_menu4);
			gComponent.navi.menu.bMenu5	= (Button)Manager_Setting.mContext.findViewById(R.id.full_menu5);
			gComponent.navi.menu.bMenu6	= (Button)Manager_Setting.mContext.findViewById(R.id.full_menu6);
			
			//	Menu Icon(ImageView)
			gComponent.navi.menu.ivMenu1	= (ImageView)Manager_Setting.mContext.findViewById(R.id.full_menu1_icon);
			gComponent.navi.menu.ivMenu2	= (ImageView)Manager_Setting.mContext.findViewById(R.id.full_menu2_icon);
			gComponent.navi.menu.ivMenu3	= (ImageView)Manager_Setting.mContext.findViewById(R.id.full_menu3_icon);
			gComponent.navi.menu.ivMenu4	= (ImageView)Manager_Setting.mContext.findViewById(R.id.full_menu4_icon);
			gComponent.navi.menu.ivMenu5	= (ImageView)Manager_Setting.mContext.findViewById(R.id.full_menu5_icon);
			gComponent.navi.menu.ivMenu6	= (ImageView)Manager_Setting.mContext.findViewById(R.id.full_menu6_icon);
			
			//	Menu Text
			gComponent.navi.menu.tvMenu1	= (TextView)Manager_Setting.mContext.findViewById(R.id.full_menu1_text);
			gComponent.navi.menu.tvMenu2	= (TextView)Manager_Setting.mContext.findViewById(R.id.full_menu2_text);
			gComponent.navi.menu.tvMenu3	= (TextView)Manager_Setting.mContext.findViewById(R.id.full_menu3_text);
			gComponent.navi.menu.tvMenu4	= (TextView)Manager_Setting.mContext.findViewById(R.id.full_menu4_text);
			gComponent.navi.menu.tvMenu5	= (TextView)Manager_Setting.mContext.findViewById(R.id.full_menu5_text);
			gComponent.navi.menu.tvMenu6	= (TextView)Manager_Setting.mContext.findViewById(R.id.full_menu6_text);
			
			//	Menu Specific
			if(Manager_Setting.ePLAYER == Manager_Setting.PLAYER.ATSC)
			{
				gComponent.navi.menu.bCapture	= null;
				gComponent.navi.menu.bRecord	= null;
			}
			else if(Manager_Setting.ePLAYER == Manager_Setting.PLAYER.TDMB)
			{
				gComponent.navi.menu.bCapture	= gComponent.navi.menu.bMenu3;
				gComponent.navi.menu.bRecord	= gComponent.navi.menu.bMenu4;
			}
			else
			{
				gComponent.navi.menu.bCapture	= gComponent.navi.menu.bMenu4;
				gComponent.navi.menu.bRecord	= gComponent.navi.menu.bMenu5;
			}
			
		/*	Title Only	*/
			gComponent.title.tvTitle		= (TextView)Manager_Setting.mContext.findViewById(R.id.full_title_only);
	}
	
	static private void setVisibility()
	{
		if(gInformation.iCount_Menu == 4)
		{
			gComponent.navi.menu.vLayout.setBackgroundResource(R.drawable.dxb_portable_toolbar_4bg);
			gComponent.navi.menu.vMenu5.setVisibility(View.GONE);
			gComponent.navi.menu.vMenu6.setVisibility(View.GONE);
		}
		else if(gInformation.iCount_Menu == 5)
		{
			gComponent.navi.menu.vLayout.setBackgroundResource(R.drawable.dxb_portable_toolbar_5bg);
			gComponent.navi.menu.vMenu6.setVisibility(View.GONE);
		}
		else	// if(gInformation.iCount_Menu == 6)
		{
			gComponent.navi.menu.vLayout.setBackgroundResource(R.drawable.dxb_portable_toolbar_6bg);
		}
	}
	
	static private void setMenu()
	{
		gComponent.navi.menu.ivMenu1.setImageResource(getRdrawable_MenuIcon(1, false));
		gComponent.navi.menu.ivMenu2.setImageResource(getRdrawable_MenuIcon(2, false));
		gComponent.navi.menu.ivMenu3.setImageResource(getRdrawable_MenuIcon(3, false));
		gComponent.navi.menu.ivMenu4.setImageResource(getRdrawable_MenuIcon(4, false));
		gComponent.navi.menu.ivMenu5.setImageResource(getRdrawable_MenuIcon(5, false));
		gComponent.navi.menu.ivMenu6.setImageResource(getRdrawable_MenuIcon(6, false));
		
		gComponent.navi.menu.tvMenu1.setText(Manager_Setting.mContext.getResources().getString(getRstring_MenuText(1)));
		gComponent.navi.menu.tvMenu2.setText(Manager_Setting.mContext.getResources().getString(getRstring_MenuText(2)));
		gComponent.navi.menu.tvMenu3.setText(Manager_Setting.mContext.getResources().getString(getRstring_MenuText(3)));
		gComponent.navi.menu.tvMenu4.setText(Manager_Setting.mContext.getResources().getString(getRstring_MenuText(4)));
		gComponent.navi.menu.tvMenu5.setText(Manager_Setting.mContext.getResources().getString(getRstring_MenuText(5)));
		gComponent.navi.menu.tvMenu6.setText(Manager_Setting.mContext.getResources().getString(getRstring_MenuText(6)));
	}
	
	static private void setTextSize()
	{
		TextView tvMessage	= (TextView)Manager_Setting.mContext.findViewById(R.id.full_text);		
		if(Manager_Setting.g_Information.cCOMM.iDisplayWidth > 800)
		{
			//	Title
			gComponent.navi.tvTitle.setTextSize(30 / Manager_Setting.g_Information.cCOMM.fDensity);
			gComponent.title.tvTitle.setTextSize(30 / Manager_Setting.g_Information.cCOMM.fDensity);
			
			//	Preview
			tvMessage.setTextSize(40 / Manager_Setting.g_Information.cCOMM.fDensity);
			
			//	Menu Text
			gComponent.navi.menu.tvMenu1.setTextSize(25 / Manager_Setting.g_Information.cCOMM.fDensity);
			gComponent.navi.menu.tvMenu2.setTextSize(25 / Manager_Setting.g_Information.cCOMM.fDensity);
			gComponent.navi.menu.tvMenu3.setTextSize(25 / Manager_Setting.g_Information.cCOMM.fDensity);
			gComponent.navi.menu.tvMenu4.setTextSize(25 / Manager_Setting.g_Information.cCOMM.fDensity);
			gComponent.navi.menu.tvMenu5.setTextSize(25 / Manager_Setting.g_Information.cCOMM.fDensity);
			gComponent.navi.menu.tvMenu6.setTextSize(25 / Manager_Setting.g_Information.cCOMM.fDensity);
		}
		else	//	if(DxbPlayer_Control.eResolution == DxbPlayer_Control.DISPLAY_RESOLUTION.p480)
		{
			//	Title
			gComponent.navi.tvTitle.setTextSize(18 / Manager_Setting.g_Information.cCOMM.fDensity);
			gComponent.title.tvTitle.setTextSize(18 / Manager_Setting.g_Information.cCOMM.fDensity);
			
			//	Preview
			tvMessage.setTextSize(25 / Manager_Setting.g_Information.cCOMM.fDensity);
			
			//	Menu Text
			gComponent.navi.menu.tvMenu1.setTextSize(18 / Manager_Setting.g_Information.cCOMM.fDensity);
			gComponent.navi.menu.tvMenu2.setTextSize(18 / Manager_Setting.g_Information.cCOMM.fDensity);
			gComponent.navi.menu.tvMenu3.setTextSize(18 / Manager_Setting.g_Information.cCOMM.fDensity);
			gComponent.navi.menu.tvMenu4.setTextSize(18 / Manager_Setting.g_Information.cCOMM.fDensity);
			gComponent.navi.menu.tvMenu5.setTextSize(18 / Manager_Setting.g_Information.cCOMM.fDensity);
			gComponent.navi.menu.tvMenu6.setTextSize(18 / Manager_Setting.g_Information.cCOMM.fDensity);
		}		
	}
	
	private static void setListener()
	{
		//	OnClickListener
			//	video
				gComponent.vVideo.setOnClickListener(ListenerOnClick);
		
			//	navi
				gComponent.navi.bTeletext.setOnClickListener(ListenerOnClick);
			
				gComponent.navi.bUp.setOnClickListener(ListenerOnClick);
				gComponent.navi.bDown.setOnClickListener(ListenerOnClick);
	
				gComponent.navi.menu.bMenu1.setOnClickListener(ListenerOnClick);
				gComponent.navi.menu.bMenu2.setOnClickListener(ListenerOnClick);
				gComponent.navi.menu.bMenu3.setOnClickListener(ListenerOnClick);
				gComponent.navi.menu.bMenu4.setOnClickListener(ListenerOnClick);
				gComponent.navi.menu.bMenu5.setOnClickListener(ListenerOnClick);
				gComponent.navi.menu.bMenu6.setOnClickListener(ListenerOnClick);
		
		//	OnFocusChange
			gComponent.navi.bTeletext.setOnFocusChangeListener(ListenerOnFocusChange);
			
			gComponent.navi.bUp.setOnFocusChangeListener(ListenerOnFocusChange);
			gComponent.navi.bDown.setOnFocusChangeListener(ListenerOnFocusChange);
	
			gComponent.navi.menu.bMenu1.setOnFocusChangeListener(ListenerOnFocusChange);
			gComponent.navi.menu.bMenu2.setOnFocusChangeListener(ListenerOnFocusChange);
			gComponent.navi.menu.bMenu3.setOnFocusChangeListener(ListenerOnFocusChange);
			gComponent.navi.menu.bMenu4.setOnFocusChangeListener(ListenerOnFocusChange);
			gComponent.navi.menu.bMenu5.setOnFocusChangeListener(ListenerOnFocusChange);
			gComponent.navi.menu.bMenu6.setOnFocusChangeListener(ListenerOnFocusChange);
		
		//	OnTouchListener
			//	Title
				gComponent.navi.bTeletext.setOnTouchListener(ListenerOnTouch);
			
			//	Navi
				gComponent.navi.bUp.setOnTouchListener(ListenerOnTouch);
				gComponent.navi.bDown.setOnTouchListener(ListenerOnTouch);
	
				gComponent.navi.menu.bMenu1.setOnTouchListener(ListenerOnTouch);
				gComponent.navi.menu.bMenu2.setOnTouchListener(ListenerOnTouch);
				gComponent.navi.menu.bMenu3.setOnTouchListener(ListenerOnTouch);
				gComponent.navi.menu.bMenu4.setOnTouchListener(ListenerOnTouch);
				gComponent.navi.menu.bMenu5.setOnTouchListener(ListenerOnTouch);
				gComponent.navi.menu.bMenu6.setOnTouchListener(ListenerOnTouch);
	}
	
	static OnClickListener ListenerOnClick = new OnClickListener()
	{
		public void onClick(View v)
		{
			DxbLog(I, "ListenerOnClick - onClick()");

			if(DxbView_Record.eState != DxbView_Record.STATE.STOP)
			{
				DxbView_Message.updateToast(Manager_Setting.mContext.getResources().getString(R.string.please_wait));
				return;
			}
			
			int id = v.getId();
			if(id == R.id.full_view)
			{
				if(DxbPlayer.isShown_Teletext())
				{
					return;
				}
				
				setGroupState(!gComponent.navi.vgNavi.isShown());
				
				return;
			}

			setGroupState(true);			
			switch(id)
			{
				case R.id.full_title_ttx:
					if(!DxbPlayer.isValidate_Teletext())
					{
						gInfo_Option.teletext	= 0;
					}
					else
					{
						gInfo_Option.teletext	= 1;
					}
					
					setTitleState(false);
					DxbLog(D, "gInfo_Option.teletext = "+gInfo_Option.teletext);
					if(gInfo_Option.teletext == 1)
					{
						setGroupState(false);
						
						DxbPlayer.setState_Teletext(true);
					}
					else
					{
						DxbPlayer.setState_Teletext(false);

						setGroupState(true);
					}
				break;
			
				case R.id.full_ch_up:
					DxbView.changeChannel(DxbView.UP);
				break;
				
				case R.id.full_ch_down:
					DxbView.changeChannel(DxbView.DOWN);
				break;
				
				case R.id.full_menu1:
				case R.id.full_menu2:
				case R.id.full_menu3:
				case R.id.full_menu4:
				case R.id.full_menu5:
				case R.id.full_menu6:
					setEvent(getMenuState(getIndex_Menu(id)));
				break;
			}
		}
	};
	
	static OnFocusChangeListener ListenerOnFocusChange	= new View.OnFocusChangeListener()
	{
		public void onFocusChange(View v, boolean hasFocus)
		{
			DxbLog(I, "onFocusChange()");
	
			int id = v.getId();
			int	menu_index	= getIndex_Menu(id);
			
			TextView	tvMenu	= null;
			Button		bMenu	= null;
			
			switch(id)
			{
				case R.id.full_title_ttx:
					if(hasFocus == true)
					{
						gComponent.navi.bTeletext.setBackgroundResource(R.drawable.dxb_portable_ttx_main_btn_f);
					}
					else
					{
						gComponent.navi.bTeletext.setBackgroundResource(R.drawable.dxb_portable_ttx_main_btn_n);
					}
				break;
			
				case R.id.full_ch_up:
					if(hasFocus == true)
					{
						gComponent.navi.bUp.setBackgroundResource(R.drawable.dxb_portable_channel_up_btn_f);
					}
					else
					{
						gComponent.navi.bUp.setBackgroundResource(R.drawable.dxb_portable_channel_up_btn_n);
					}					
				break;
				
				case R.id.full_ch_down:
					if(hasFocus == true)
					{
						gComponent.navi.bDown.setBackgroundResource(R.drawable.dxb_portable_channel_down_btn_f);
					}
					else
					{
						gComponent.navi.bDown.setBackgroundResource(R.drawable.dxb_portable_channel_down_btn_n);
					}
				break;
				
				case R.id.full_menu1:
					gComponent.navi.menu.ivMenu1.setImageResource(getRdrawable_MenuIcon(menu_index, hasFocus));
					tvMenu	= gComponent.navi.menu.tvMenu1;
					bMenu	= gComponent.navi.menu.bMenu1;
				break;
					
				case R.id.full_menu2:
					gComponent.navi.menu.ivMenu2.setImageResource(getRdrawable_MenuIcon(menu_index, hasFocus));
					tvMenu	= gComponent.navi.menu.tvMenu2;
					bMenu	= gComponent.navi.menu.bMenu2;
				break;
					
				case R.id.full_menu3:
					gComponent.navi.menu.ivMenu3.setImageResource(getRdrawable_MenuIcon(menu_index, hasFocus));
					tvMenu	= gComponent.navi.menu.tvMenu3;
					bMenu	= gComponent.navi.menu.bMenu3;
				break;
					
				case R.id.full_menu4:
					gComponent.navi.menu.ivMenu4.setImageResource(getRdrawable_MenuIcon(menu_index, hasFocus));
					tvMenu	= gComponent.navi.menu.tvMenu4;
					bMenu	= gComponent.navi.menu.bMenu4;
				break;
					
				case R.id.full_menu5:
					gComponent.navi.menu.ivMenu5.setImageResource(getRdrawable_MenuIcon(menu_index, hasFocus));
					tvMenu	= gComponent.navi.menu.tvMenu5;
					bMenu	= gComponent.navi.menu.bMenu5;
				break;
					
				case R.id.full_menu6:
					gComponent.navi.menu.ivMenu6.setImageResource(getRdrawable_MenuIcon(menu_index, hasFocus));
					tvMenu	= gComponent.navi.menu.tvMenu6;
					bMenu	= gComponent.navi.menu.bMenu6;
				break;
			}
			
			if(		(tvMenu != null)
				&&	(bMenu != null)
			)
			{
				if(hasFocus == true)
				{
					bMenu.setBackgroundResource(R.drawable.dxb_portable_toolbar_6bg_01_f);
					tvMenu.setTextColor(Manager_Setting.mContext.getResources().getColor(R.color.color_4));
				}
				else
				{
					bMenu.setBackgroundColor(Manager_Setting.mContext.getResources().getColor(R.color.color_0));
					tvMenu.setTextColor(Manager_Setting.mContext.getResources().getColor(R.color.color_9));
				}
			}
		}
	};

	static OnTouchListener ListenerOnTouch	= new View.OnTouchListener()
	{
		public boolean onTouch(View v, MotionEvent mEvent)
		{
			DxbLog(I, "onTouch(v=" + v +", mEvent=" + mEvent + ")");
			
			if(mEvent.getAction() == MotionEvent.ACTION_UP)
			{
				int	id = v.getId();
				
				if(DxbView_Record.eState != DxbView_Record.STATE.STOP)
				{
					DxbView_Message.updateToast(Manager_Setting.mContext.getResources().getString(R.string.please_wait));
					return false;
				}
				
				setGroupState(true);			
				if(		(id == R.id.full_title_ttx)
					&&	(!gComponent.navi.bTeletext.isFocused())
				)
				{
					if(!DxbPlayer.isValidate_Teletext())
					{
						gInfo_Option.teletext	= 0;
					}
					else
					{
						gInfo_Option.teletext	= 1;
					}
					
					setTitleState(false);
					DxbLog(D, "gInfo_Option.teletext = "+gInfo_Option.teletext);
					if(gInfo_Option.teletext == 1)
					{
						setGroupState(false);
						
						DxbPlayer.setState_Teletext(true);
					}
					else
					{
						DxbPlayer.setState_Teletext(false);
						
						setGroupState(true);
					}
				}
				else if(	(id == R.id.full_ch_up)
						&&	(!gComponent.navi.bUp.isFocused())
				)
				{
					DxbView.changeChannel(DxbView.UP);
				}
				else if(	(id == R.id.full_ch_down)
						&&	(!gComponent.navi.bDown.isFocused())
				)
				{
					DxbView.changeChannel(DxbView.DOWN);
				}
				else if(	id == R.id.full_menu1
						||	id == R.id.full_menu2
						||	id == R.id.full_menu3
						||	id == R.id.full_menu4
						||	id == R.id.full_menu5
						||	id == R.id.full_menu6
				)
				{
					setEvent(getMenuState(getIndex_Menu(id)));
				}
			}
			
			return true;
		}
	};
	
	public static boolean onKeyDown(int keyCode, KeyEvent event)
	{
		DxbLog(I, "onKeyDown()");
		
		if(DxbPlayer.isShown_Teletext())
		{
			return DxbPlayer.onKeyDown_Teletext(keyCode, event);
		}
		else if(Manager_Setting.g_Component.fullview.navi.vgNavi.isShown())
		{
			setGroupState(true);
		}
		else if(keyCode == KeyEvent.KEYCODE_PAGE_UP)
		{
			DxbView.changeChannel(DxbView.UP);
			setTitleState(true);
			return true;
		}
		else if(keyCode == KeyEvent.KEYCODE_PAGE_DOWN)
		{
			DxbView.changeChannel(DxbView.DOWN);
			setTitleState(true);
			return true;
		}
		else
		{
			setGroupState(true);
			return true;
		}

		int	menu_index	= getIndex_Menu(keyCode);
		switch(keyCode)
		{
			case KeyEvent.KEYCODE_1:
				setEvent(getMenuState(menu_index));
			break;
				
			case KeyEvent.KEYCODE_2:
				setEvent(getMenuState(menu_index));
			break;
			
			case KeyEvent.KEYCODE_3:
				setEvent(getMenuState(menu_index));
			break;
			
			case KeyEvent.KEYCODE_4:
				setEvent(getMenuState(menu_index));
			break;
			
			case KeyEvent.KEYCODE_5:
				setEvent(getMenuState(menu_index));
			break;
			
			case KeyEvent.KEYCODE_6:
				setEvent(getMenuState(menu_index));
			break;

			case KeyEvent.KEYCODE_PAGE_UP:
				gComponent.navi.bUp.requestFocus();
				DxbView.changeChannel(DxbView.UP);
			break;

			case KeyEvent.KEYCODE_PAGE_DOWN:
				gComponent.navi.bDown.requestFocus();
				DxbView.changeChannel(DxbView.DOWN);
			break;

			default:
				return false;
		}
		
		return true;
	}
	
	private static int getIndex_Menu(int id)
	{
		int	return_Index	= -1;
		
		if(		(id == R.id.full_menu1)
			||	(id == KeyEvent.KEYCODE_1)
		)
		{
			return_Index	= 1;
		}
		else if(	(id == R.id.full_menu2)
				||	(id == KeyEvent.KEYCODE_2)
		)
		{
			return_Index	= 2;
		}
		else if(	(id == R.id.full_menu3)
				||	(id == KeyEvent.KEYCODE_3)
		)
		{
			return_Index	= 3;
		}
		else if(	(id == R.id.full_menu4)
				||	(id == KeyEvent.KEYCODE_4)
		)
		{
			return_Index	= 4;
		}
		else if(	(id == R.id.full_menu5)
				||	(id == KeyEvent.KEYCODE_5)
		)
		{
			return_Index	= 5;
		}
		else if(	(id == R.id.full_menu6)
				||	(id == KeyEvent.KEYCODE_6)
		)
		{
			return_Index	= 6;
		}
		
		DxbLog(I, "getIndex_Menu(id =" + id + ") --> return_Index=" + return_Index);
		
		return	return_Index;
	}
	
	public static int getMenuState(int menu)
	{
		int	return_State	= Manager_Setting.MENU_NULL;
		
		if(menu == Manager_Setting.MENU_CHANNEL)
		{
			return_State	= MENU_CHANNEL;
		}
		else if(menu == Manager_Setting.MENU_SCREEN)
		{
			return_State	= MENU_SCREEN;
		}
		else if(menu == Manager_Setting.MENU_EPG)
		{
			return_State	= MENU_EPG;
		}
		else if(menu == Manager_Setting.MENU_CAPTURE)
		{
			return_State	= MENU_CAPTURE;
		}
		else if(menu == Manager_Setting.MENU_RECORD)
		{
			return_State	= MENU_RECORD;
		}
		else if(menu == Manager_Setting.MENU_OPTION)
		{
			return_State	= MENU_OPTION;
		}
		
		DxbLog(D, "getMenuState(menu="+menu+ ") --> return_State = " + return_State);
		return return_State;
	}
	
	public static int getRdrawable_MenuIcon(int index, boolean state)
	{
		int	return_Resource;
		
		if(index == Manager_Setting.MENU_CHANNEL)
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
		else if(index == Manager_Setting.MENU_SCREEN)
		{
			if(state == true)
			{
				return_Resource	= R.drawable.dxb_portable_toolbar_screen_btn_f;
			}
			else
			{
				return_Resource	= R.drawable.dxb_portable_toolbar_screen_btn_n;
			}
		}
		else if(index == Manager_Setting.MENU_EPG)
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
		else if(index == Manager_Setting.MENU_CAPTURE)
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
		else if(index == Manager_Setting.MENU_RECORD)
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
		else /*if(index == Manager_Setting.MENU_OPTION)*/
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

		return return_Resource;
	}

	public static int getRstring_MenuText(int index)
	{
		int	return_Resource;
		
		if(index == Manager_Setting.MENU_CHANNEL)
		{
			return_Resource = R.string.channel;
		}
		else if(index == Manager_Setting.MENU_SCREEN)
		{
			return_Resource	= R.string.screen;
		}
		else if(index == Manager_Setting.MENU_EPG)
		{
			return_Resource	= R.string.epg;
		}
		else if(index == Manager_Setting.MENU_CAPTURE)
		{
			return_Resource = R.string.capture;
		}
		else if(index == Manager_Setting.MENU_RECORD)
		{
			return_Resource = R.string.record;
		}
		else	/* if(index == Manager_Setting.MENU_OPTION)*/
		{
			return_Resource = R.string.option;
		}

		return return_Resource;
	}
	
	private static void setEvent(int event)
	{
		DxbLog(I, "setEvent(event = " + event + ")");
		
		if(event == MENU_CHANNEL)
		{
			DxbView.setState(false, DxbView.STATE.LIST);
		}
		else if(event == MENU_SCREEN)
		{
			if(eState_ScreenSize == SCREENMODE_LETTERBOX)
			{
				eState_ScreenSize	= SCREENMODE_PANSCAN;
			}
			else if(eState_ScreenSize == SCREENMODE_PANSCAN)
			{
				eState_ScreenSize	= SCREENMODE_FULL;
			}
			else if(eState_ScreenSize == SCREENMODE_FULL)
			{
				eState_ScreenSize	= SCREENMODE_LETTERBOX;
			}			
			
			gComponent.navi.menu.bMenu2.requestFocus();
			DxbPlayer.setScreenMode(eState_ScreenSize);
		}
		else if(event == MENU_EPG)
		{
			/*	TDMB not support -->
				DxbView_EPG.resetData();
				TDMB not support -->	*/
			DxbView.setState(false, DxbView.STATE.EPG);
		}
		else if(event == MENU_CAPTURE)
		{
			gComponent.navi.menu.bCapture.requestFocus();
			DxbView_Record.setCapture();			
		}
		else if(event == MENU_RECORD)
		{
			gComponent.navi.menu.bRecord.requestFocus();
			DxbView_Record.setRecord();			
		}
		else if(event == MENU_OPTION)
		{
			@SuppressWarnings("rawtypes")
			Class	class_Option	= DxbPlayer.getClass_Option();
			if(class_Option != null)
			{
				DxbPlayer.releaseSurface();
				DxbView.intentSubActivity = new Intent(Manager_Setting.mContext, class_Option);
				Manager_Setting.mContext.startActivity( DxbView.intentSubActivity );
			}
		}
		else
		{
				DxbLog(E, "(Menu) - Error!");
		}
	}
		
	static boolean isFullDisplay()
	{
		if(		(DxbView_Record.eState == DxbView_Record.STATE.STOP)
			&&	(gInfo_Option.teletext != 1)
			&&	(DxbView.eState == DxbView.STATE.FULL)
			&&	(!gComponent.navi.vgNavi.isShown())
		)
		{
			return true;
		}
			
		return false;		
	}
	
		
	static void setGroupState(boolean state)
	{
		DxbLog(I, "setGroupState(state=" + state + ")");
		if(state)
		{
			DxbView.mHandler_Main.removeCallbacks(mRunnable_Navigation);
			
			DxbView_Indicator.Signal_visible();
			
			if(!gComponent.navi.vgNavi.isShown())
			{
				DxbPlayer.playSubtitle(DxbPlayer._OFF_);
					
				gComponent.navi.vgNavi.setVisibility(View.VISIBLE);
				gComponent.navi.menu.bMenu1.requestFocus();
				Manager_Setting.g_Component.indicator.ivSection.setVisibility(View.VISIBLE);
				Manager_Setting.g_Component.indicator.timeIndicator.setVisibility(View.VISIBLE);
			}
			
			DxbView.mHandler_Main.postDelayed(mRunnable_Navigation, 7000);
		}
		else
		{
			DxbView.mHandler_Main.removeCallbacks(mRunnable_Navigation);
			
			if(		(DxbView_Record.eState == DxbView_Record.STATE.STOP)
				&&	(gInfo_Option.teletext != 1)
			)
			{
				DxbPlayer.playSubtitle(gInfo_Option.subtitle);
			}
			else
			{
				DxbPlayer.playSubtitle(DxbPlayer._OFF_);
			}
			gComponent.navi.vgNavi.setVisibility(View.INVISIBLE);
			Manager_Setting.g_Component.indicator.ivSection.setVisibility(View.INVISIBLE);
			Manager_Setting.g_Component.indicator.timeIndicator.setVisibility(View.INVISIBLE);
			
			DxbView_Indicator.Signal_invisible();
		}
	}
	
	private static Runnable mRunnable_Navigation = new Runnable()
	{
		public void run()
		{
			//Log.i(TAG, "mRunnable_Navigation --> run()");
			
			if(		(DxbView.eState==DxbView.STATE.FULL)
				&&	(gComponent.navi.vgNavi.isShown())
			)
			{
				setGroupState(false);
			}
		}
	};
	
	static void setTitleState(boolean state)
	{
		DxbLog(I, "setTitleState(" + state + ")");
		
		DxbView.mHandler_Main.removeCallbacks(mRunnable_TitleOnly);
		if(state)
		{
			DxbView.mHandler_Main.postDelayed(mRunnable_TitleOnly, 5000);
			gComponent.title.tvTitle.setVisibility(View.VISIBLE);
		}
		else
		{
			gComponent.title.tvTitle.setVisibility(View.INVISIBLE);
		}
	}
	
	private static Runnable mRunnable_TitleOnly = new Runnable()
	{
		public void run()
		{
			DxbLog(I, "mRunnable_TitleOnly --> run()");
			
			setTitleState(false);
		}
	};

	private static void DxbLog(int level, String mString)
	{
		if(DxbView.TAG == null)
		{
			return;
		}
		
		String TAG = "[[[DxbView_Full]]] ";
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
