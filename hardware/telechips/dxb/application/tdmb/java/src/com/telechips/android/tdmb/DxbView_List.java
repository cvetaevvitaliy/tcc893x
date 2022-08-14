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

import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnFocusChangeListener;
import android.view.View.OnKeyListener;
import android.widget.*;
import android.widget.AdapterView.OnItemClickListener;

public class DxbView_List
{
	private static final int D = 0;
	private static final int E = 1;
	private static final int I = 2;
	private static final int V = 3;
	private static final int W = 4;
	
	private static Component.cListView	gComponent	= null;
	
	static int	iIndex_Tab_current	= 0;
	static int	iIndex_Tab_old		= 0;

	static DxbAdapter_Service adapter;

	static public void init()
	{
		DxbLog(I, "init()");
		
		setComponent();
		
		TextView	tab_text1, tab_text2;
		if(Manager_Setting.g_Information.cLIST.iCount_Tab == 1)
		{
			Manager_Setting.mContext.findViewById(R.id.list_tab1).setVisibility(View.VISIBLE);
			
			tab_text1	= (TextView)Manager_Setting.mContext.findViewById(R.id.list_tab1_text);
		}
		else	//if(Manager_Setting.g_Information.cLIST.iCount_Tab == 2)
		{
			Manager_Setting.mContext.findViewById(R.id.list_tab2).setVisibility(View.VISIBLE);
			
			tab_text1	= (TextView)Manager_Setting.mContext.findViewById(R.id.list_tab2_1_text);
		}
		tab_text2	= (TextView)Manager_Setting.mContext.findViewById(R.id.list_tab2_2_text);
		
		if(Manager_Setting.g_Information.cCOMM.iDisplayWidth > 800)
		{
			tab_text1.setTextSize(28 / Manager_Setting.g_Information.cCOMM.fDensity);
			tab_text2.setTextSize(28 / Manager_Setting.g_Information.cCOMM.fDensity);
			
			//	Video
			gComponent.tvMessage.setTextSize(25 / Manager_Setting.g_Information.cCOMM.fDensity);
			
			//	Preview
			gComponent.tvTitle.setTextSize(30 / Manager_Setting.g_Information.cCOMM.fDensity);
			gComponent.tvInformation.setTextSize(25 / Manager_Setting.g_Information.cCOMM.fDensity);
			
			//	Scan
			gComponent.bScan.setTextSize(35 / Manager_Setting.g_Information.cCOMM.fDensity);
		}
		else	//	if(DxbPlayer_Control.eResolution == DxbPlayer_Control.DISPLAY_RESOLUTION.p480)
		{
			tab_text1.setTextSize(18 / Manager_Setting.g_Information.cCOMM.fDensity);
			tab_text2.setTextSize(18 / Manager_Setting.g_Information.cCOMM.fDensity);
			
			//	Video
			gComponent.tvMessage.setTextSize(18 / Manager_Setting.g_Information.cCOMM.fDensity);
			
			//	Preview
			gComponent.tvTitle.setTextSize(20 / Manager_Setting.g_Information.cCOMM.fDensity);
			gComponent.tvInformation.setTextSize(18 / Manager_Setting.g_Information.cCOMM.fDensity);
			
			//	Scan
			gComponent.bScan.setTextSize(20 / Manager_Setting.g_Information.cCOMM.fDensity);
		}
		
		setListener();
	}
	
	private static void setComponent()
	{
		DxbLog(I, "setComponent()");

		gComponent	= Manager_Setting.g_Component.listview;
		
		//	Tab
			gComponent.vTab1	= Manager_Setting.mContext.findViewById(R.id.list_tab2_1);
			gComponent.vTab2	= Manager_Setting.mContext.findViewById(R.id.list_tab2_2);
			
			gComponent.ivTab1	= (ImageView)Manager_Setting.mContext.findViewById(R.id.list_tab2_1_image);
			gComponent.ivTab2	= (ImageView)Manager_Setting.mContext.findViewById(R.id.list_tab2_2_image);
			
			gComponent.tvTab1	= (TextView)Manager_Setting.mContext.findViewById(R.id.list_tab2_1_text);
			gComponent.tvTab2	= (TextView)Manager_Setting.mContext.findViewById(R.id.list_tab2_2_text);
			
		//	List
			gComponent.lvService	= (ListView)Manager_Setting.mContext.findViewById(R.id.list_service);
		
		//	Video
			gComponent.vVideo		= Manager_Setting.mContext.findViewById(R.id.list_view);
			gComponent.tvMessage	= (TextView)Manager_Setting.mContext.findViewById(R.id.list_message);

		//	Preview
			gComponent.tvTitle		= (TextView)Manager_Setting.mContext.findViewById(R.id.list_title);
			gComponent.tvInformation	= (TextView)Manager_Setting.mContext.findViewById(R.id.list_count);
		
		//	Scan
			gComponent.bScan			= (Button)Manager_Setting.mContext.findViewById(R.id.bu_scan);
		
		if(Manager_Setting.g_Information.cCOMM.iDisplayWidth > 800)
		{
			gComponent.ID_row	= R.layout.dxb_list_row_60px;
		}
		else
		{
			gComponent.ID_row	= R.layout.dxb_list_row_40px;
		}
	}
	
	private static void setListener()
	{
		//	List
		gComponent.lvService.setOnItemClickListener(ListenerOnItemClick);
		gComponent.lvService.setOnKeyListener(ListenerOnKey);
		
		//	OnClickListener
		gComponent.vTab1.setOnClickListener(ListenerOnClick);
		gComponent.vTab2.setOnClickListener(ListenerOnClick);
		gComponent.vVideo.setOnClickListener(ListenerOnClick);
		gComponent.bScan.setOnClickListener(ListenerOnClick);
		
		//	OnFocusChangeListener
		gComponent.bScan.setOnFocusChangeListener(OnFocusChangeListenerOnFocusChange);
	}
	
	static OnItemClickListener ListenerOnItemClick = new OnItemClickListener()
	{
		public void onItemClick(AdapterView<?> parent, View v, int position, long id)
		{
			DxbLog(I, "ListenerOnItemClick-->onItemClick(position="+position+")");
			
			if(DxbPlayer.eState == DxbPlayer.STATE.SCAN_STOP)
			{
				DxbView_Message.updateToast(Manager_Setting.mContext.getResources().getString(R.string.please_wait_cancel_scanning));
				return;
			}

			if(		(		getTab() == 0 
						&&	(Manager_Setting.g_Information.cCOMM.iCurrent_TV != position)
					)
				||	(		getTab() == 1 
						&&	(Manager_Setting.g_Information.cCOMM.iCurrent_Radio != position)
					)
			)
			{
				if(Manager_Setting.g_Information.cCOMM.curChannels != null)
				{
					Manager_Setting.g_Information.cCOMM.curChannels.moveToPosition(position);
					gComponent.lvService.setSelection(position);
				}

				DxbView.resetChannel();
				DxbView.setChannel();
			}
		/*
			else
			{
				DxbLog(E, "DxbPlayer.mChannelManager.deleteChannel : " + Manager_Setting.g_Information.cCOMM.curChannels.getInt(0));
				DxbPlayer.mChannelManager.deleteChannel(Manager_Setting.g_Information.cCOMM.curChannels.getInt(0));
				updateChannelList(getTab());
				if(Manager_Setting.g_Information.cCOMM.curChannels != null)
				{
					Manager_Setting.g_Information.cCOMM.curChannels.moveToPosition(position);
					gComponent.lvService.setSelection(position);
				}

				DxbView.resetChannel();
				DxbView.setChannel();
			}
		*/
		}
	};
	
	static OnKeyListener ListenerOnKey = new OnKeyListener()
	{
		public boolean onKey(View v, int keyCode, KeyEvent event)
		{
			if(DxbPlayer.eState == DxbPlayer.STATE.SCAN_STOP)
			{
				DxbView_Message.updateToast(Manager_Setting.mContext.getResources().getString(R.string.please_wait_cancel_scanning));
				return true;
			}

			if(		(event.getAction() == 1)
				||	(event.getRepeatCount() > 0)
			)
			{
				DxbLog(I, "ListenerOnKey-->onKey(keyCode=" + keyCode + ", event=" + event + ")");

				int	iCurrentTab	= 0;
				if(Manager_Setting.g_Information.cLIST.iCount_Tab > 1)
				{
					iCurrentTab	= getTab();
				}
				
				switch(keyCode)
				{
					case KeyEvent.KEYCODE_DPAD_UP:
						if(		(		(iCurrentTab==0)
									&&	(Manager_Setting.g_Information.cCOMM.iCount_TV>1)
								)
							||	(		(iCurrentTab==1)
									&&	(Manager_Setting.g_Information.cCOMM.iCount_Radio>1)
								)
						)
						{
							setCurrentService(true);
						}
					break;
						
					case KeyEvent.KEYCODE_DPAD_DOWN:
						if(		(		(iCurrentTab==0)
									&&	(Manager_Setting.g_Information.cCOMM.iCount_TV>1)
								)
							||	(		(iCurrentTab==1)
									&&	(Manager_Setting.g_Information.cCOMM.iCount_Radio>1)
								)
						)
						{
							setCurrentService(true);
						}
					break;
				}
			}
			
			return false;
		}
	};
	
	static OnClickListener ListenerOnClick = new OnClickListener()
	{
		public void onClick(View v)
		{
			DxbLog(I, "ListenerOnClick - onClick()");

			if(DxbPlayer.eState == DxbPlayer.STATE.SCAN_STOP)
			{
				DxbView_Message.updateToast(Manager_Setting.mContext.getResources().getString(R.string.please_wait_cancel_scanning));
				return;
			}
			else if(DxbPlayer.eState == DxbPlayer.STATE.SCAN)
			{
				return;
			}

			int id = v.getId();
			switch(id)
			{
				case R.id.list_tab2_1:
					changeTab(0);
				break;
			
				case R.id.list_tab2_2:
					changeTab(1);
				break;
				
				case R.id.list_view:
					DxbView.setState(false, DxbView.STATE.FULL);	// (full view) TV only.
				break;
				
				case R.id.bu_scan:
					if(DxbPlayer.eState == DxbPlayer.STATE.GENERAL)
					{
						DxbScan.startFull();
					}
				break;
			}
		}
	};
	
	static OnFocusChangeListener OnFocusChangeListenerOnFocusChange	= new View.OnFocusChangeListener()
	{
		public void onFocusChange(View v, boolean hasFocus)
		{
			DxbLog(I, "onFocusChange()");
			
			int id = v.getId();
			if(id == R.id.bu_scan)
			{
				if(hasFocus == true)
				{
					gComponent.bScan.setBackgroundResource(R.drawable.dxb_portable_toolbar_btn_bg_f);
				}
				else
				{
					gComponent.bScan.setBackgroundResource(R.drawable.dxb_portable_toolbar_btn_bg_n);
				}
			}
		}
	};
	
	static Runnable mRunnable_KeyEvent = new Runnable()
	{
		public void run()
		{
			DxbLog(I, "mRunnable_KeyEvent --> run()");
			
			if(DxbView.eState==DxbView.STATE.LIST)
			{
				setCurrentService(false);
			}
		}
	};

	public static boolean onKeyDown(int keyCode, KeyEvent event)
	{
		DxbLog(I, "onKeyDown()");
		
		switch(keyCode)
		{
			case KeyEvent.KEYCODE_0:
				DxbView.setState(false, DxbView.STATE.FULL);
			break;
			
			case KeyEvent.KEYCODE_9:
				if(DxbView.eState == DxbView.STATE.LIST)
				{
					DxbScan.startFull();
				}
			break;
	
			case KeyEvent.KEYCODE_PAGE_UP:
				DxbView.changeChannel(DxbView.UP);
				break;
			case KeyEvent.KEYCODE_PAGE_DOWN:
				DxbView.changeChannel(DxbView.DOWN);
			break;
			
			case KeyEvent.KEYCODE_MENU:
				nextTab();
			break;

			default:
				return false;
		}
		
		return true;
	}
	
	private static void setCurrentService(boolean state)
	{
		DxbLog(I, "setCurrentService(state=" + state + ")");
		if(state)
		{
			DxbView.mHandler_Main.removeCallbacks(mRunnable_KeyEvent);
			DxbView.mHandler_Main.postDelayed(mRunnable_KeyEvent, 1000);
		}
		else
		{
			DxbView.mHandler_Main.removeCallbacks(mRunnable_KeyEvent);

			if(Manager_Setting.g_Information.cCOMM.curChannels != null)
			{
				if(		(		getTab() == 0 
							&&	(Manager_Setting.g_Information.cCOMM.iCurrent_TV != Manager_Setting.g_Information.cCOMM.curChannels.getPosition())
						)
					||	(		getTab() == 1 
							&&	(Manager_Setting.g_Information.cCOMM.iCurrent_Radio != Manager_Setting.g_Information.cCOMM.curChannels.getPosition())
						)
				)
				{
					DxbView.resetChannel();
					DxbView.setChannel();
				}
			}
		}			
	}			
	
	public static int getTab()
	{
		return	iIndex_Tab_current;
	}
	
	public static void nextTab()
	{
		changeTab((iIndex_Tab_current + 1) % Manager_Setting.g_Information.cLIST.iCount_Tab);
	}
	
	public static void changeTab(int iIndex)
	{
		DxbLog(I, "changeTab() : iIndex_Tab_current=" + iIndex_Tab_current + " --> iIndex=" + iIndex);
		
		if(		(iIndex == iIndex_Tab_current)
			||	(Manager_Setting.g_Information.cLIST.iCount_Tab < 2)
			||	(iIndex >= Manager_Setting.g_Information.cLIST.iCount_Tab)
		)
		{
			return;
		}

		if(DxbPlayer.eState == DxbPlayer.STATE.SCAN_STOP)
		{
			DxbView_Message.updateToast(Manager_Setting.mContext.getResources().getString(R.string.please_wait_cancel_scanning));
			return;
		}
		
		iIndex_Tab_current	= iIndex;
		if(iIndex_Tab_current == 0)
		{
			gComponent.vTab1.setBackgroundResource(R.drawable.dxb_portable_tab_bg_f);
			gComponent.vTab2.setBackgroundResource(R.drawable.dxb_portable_tab_bg_n);
			
			gComponent.ivTab1.setImageResource(R.drawable.dxb_portable_tab_tv_btn_f);
			gComponent.ivTab2.setImageResource(R.drawable.dxb_portable_tab_radio_btn_n);
			
			gComponent.tvTab1.setTextColor(R.color.color_4);
			gComponent.tvTab2.setTextColor(R.color.color_9);
		}
		else if(iIndex_Tab_current == 1)
		{
			gComponent.vTab1.setBackgroundResource(R.drawable.dxb_portable_tab_bg_n);
			gComponent.vTab2.setBackgroundResource(R.drawable.dxb_portable_tab_bg_f);
			
			gComponent.ivTab1.setImageResource(R.drawable.dxb_portable_tab_tv_btn_n);
			gComponent.ivTab2.setImageResource(R.drawable.dxb_portable_tab_radio_btn_f);
			
			gComponent.tvTab1.setTextColor(R.color.color_9);
			gComponent.tvTab2.setTextColor(R.color.color_4);
		}
		updateChannelList(getTab());
		
		DxbView.mHandler_Main.removeCallbacks(mRunnable_TabEvent);
		DxbView.mHandler_Main.postDelayed(mRunnable_TabEvent, 1000);
	}
	
	private static Runnable mRunnable_TabEvent = new Runnable()
	{
		public void run()
		{
			DxbLog(I, "mRunnable_KeyEvent --> run()");
			
			if(DxbView.eState==DxbView.STATE.LIST)
			{
				if(Manager_Setting.g_Information.cCOMM.iCount_Current > 0)
				{
					int iPosition;
					int	iCurrentTab	= getTab();
					if(iIndex_Tab_old != iCurrentTab)
					{
						if(iCurrentTab == 0)
						{
							iPosition	= Manager_Setting.g_Information.cCOMM.iCurrent_TV;
						}
						else
						{
							iPosition	= Manager_Setting.g_Information.cCOMM.iCurrent_Radio;
						}
						gComponent.lvService.setSelection(iPosition);
						gComponent.lvService.requestFocus();
						
						if(Manager_Setting.g_Information.cCOMM.curChannels != null)
						{
							Manager_Setting.g_Information.cCOMM.curChannels.moveToPosition(iPosition);
						}
						DxbView.resetChannel();
						DxbView.setChannel();

						iIndex_Tab_old = iCurrentTab;
					}
				}
			}
		}
	};
		
	static void updateChannelList(int iTabNumber)
	{
		DxbLog(I, "updateChannelList(iTabNumber="+iTabNumber+")");
		int	cursorPosition;

		if(Manager_Setting.g_Information.cCOMM.curChannels != null)
		{
			Manager_Setting.mContext.stopManagingCursor(Manager_Setting.g_Information.cCOMM.curChannels);
		}

		Manager_Setting.g_Information.cCOMM.curChannels = DxbPlayer.getChannels(iTabNumber);
		if(Manager_Setting.g_Information.cCOMM.curChannels != null)
		{
			if(iTabNumber == 0)
			{
				Manager_Setting.g_Information.cCOMM.iCount_TV = Manager_Setting.g_Information.cCOMM.curChannels.getCount();
				cursorPosition			= Manager_Setting.g_Information.cCOMM.iCurrent_TV;
			}
			else
			{
				Manager_Setting.g_Information.cCOMM.iCount_Radio	= Manager_Setting.g_Information.cCOMM.curChannels.getCount();
				cursorPosition				= Manager_Setting.g_Information.cCOMM.iCurrent_Radio;
			}
			Manager_Setting.g_Information.cCOMM.curChannels.moveToPosition(cursorPosition);
			Manager_Setting.g_Information.cCOMM.iCount_Current	=  Manager_Setting.g_Information.cCOMM.curChannels.getCount();

			DxbLog(D, "Manager_Setting.g_Information.cCOMM.iCount_Current="+Manager_Setting.g_Information.cCOMM.iCount_Current);
			
			Manager_Setting.mContext.startManagingCursor(Manager_Setting.g_Information.cCOMM.curChannels);
			
			if(adapter == null)
			{
				adapter = new DxbAdapter_Service(Manager_Setting.mContext, gComponent.ID_row, Manager_Setting.g_Information.cCOMM.curChannels, new String[] {}, new int[] {});
			}
			else
			{
				adapter.changeCursor(Manager_Setting.g_Information.cCOMM.curChannels);
			}
			
			gComponent.lvService.setAdapter(adapter);
			
			updateChannelCount(iTabNumber, Manager_Setting.g_Information.cCOMM.iCount_Current);
		}
		else
		{
			Manager_Setting.g_Information.cCOMM.iCurrent_TV	= 0;
			Manager_Setting.g_Information.cCOMM.iCurrent_Radio	= 0;
			Manager_Setting.g_Information.cCOMM.iCount_Current	= 0;

			if(iTabNumber == 0)
			{
				Manager_Setting.g_Information.cCOMM.iCount_TV	= 0;
				cursorPosition			= 0;
			}
			else
			{
				Manager_Setting.g_Information.cCOMM.iCount_Radio	= 0;
				cursorPosition			= 0;
			}

			updateChannelCount(iTabNumber, 0);
		}		
	}
	
	private static void updateChannelCount(int iTab, int iCount)
	{
		DxbLog(I, "updateChannelCount(iTab=" + iTab + ", iCount=" + iCount + ")");
		
		String strCount = "";
		
		if(iTab == 0)
		{
			strCount = Manager_Setting.mContext.getResources().getString(R.string.tv_channel) + " : " + iCount;
		}
		else
		{
			strCount = Manager_Setting.mContext.getResources().getString(R.string.radio_channel) + " : " + iCount;
		}
		
		gComponent.tvInformation.setText(strCount);
		
		DxbView.updateScreen();
	}
	
	private static void DxbLog(int level, String mString)
	{
		if(DxbView.TAG == null)
		{
			return;
		}
		
		String TAG = "[[[DxbView_List]]] ";
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
