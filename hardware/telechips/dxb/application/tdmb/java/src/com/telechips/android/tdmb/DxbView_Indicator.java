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

import java.text.SimpleDateFormat;
import java.util.Calendar;

import android.util.Log;
import android.view.View;
import android.widget.TextView;
import android.widget.ImageView;;

public class DxbView_Indicator
{
	private static final int D = 0;
	private static final int E = 1;
	private static final int I = 2;
	private static final int V = 3;
	private static final int W = 4;
	
	private static Component.cIndicator	gComponent	= null;
	
	private static int[] rssIcon = {	R.drawable.dxb_portable_indicator_rssi_icon_00,
								R.drawable.dxb_portable_indicator_rssi_icon_01,
								R.drawable.dxb_portable_indicator_rssi_icon_02,
								R.drawable.dxb_portable_indicator_rssi_icon_03,
								R.drawable.dxb_portable_indicator_rssi_icon_04 };

	static public void init()
	{
		DxbLog(I, "init()");
		
		setComponent();
		
		setFontSize();
	}
	
	private static void setComponent()
	{
		DxbLog(I, "setComponent()");
		
		gComponent	= Manager_Setting.g_Component.indicator;
		
		gComponent.ivSection		= (ImageView)Manager_Setting.mContext.findViewById(R.id.indicator_section);
		gComponent.ivSignal			= (ImageView)Manager_Setting.mContext.findViewById(R.id.indicator_signal);
		gComponent.timeIndicator	= (TextView)Manager_Setting.mContext.findViewById(R.id.indicator_time);
	}
	
	private static void setFontSize()
	{
		DxbLog(I, "setFontSize()");
		
		if(Manager_Setting.g_Information.cCOMM.iDisplayWidth > 800)
		{
			gComponent.timeIndicator.setTextSize(25 / Manager_Setting.g_Information.cCOMM.fDensity);
		}
		else	//	if(DxbPlayer_Control.eResolution == DxbPlayer_Control.DISPLAY_RESOLUTION.p480)
		{
			gComponent.timeIndicator.setTextSize(18 / Manager_Setting.g_Information.cCOMM.fDensity);
		}		
	}
	
	static Runnable mRunnable_Signal = new Runnable()
	{
		public void run()
		{
			//DxbLog(I, "mRunnable_Signal --> run()");
			
			if(		(DxbPlayer.mPlayer == null)
				||	(DxbView.mSurfaceView == null)
				||	(DxbPlayer.eState == DxbPlayer.STATE.UI_PAUSE)
			)
			{
				return;
			}
			
			Calendar CurTime = Calendar.getInstance();
			int hour	= CurTime.get(Calendar.HOUR_OF_DAY);
			SimpleDateFormat formatter = new SimpleDateFormat ("mm");
			String min = formatter.format(CurTime.getTime());
			
			Manager_Setting.g_Component.indicator.timeIndicator.setText(hour+":"+min);

			//DxbPlayer.mPlayer.startTestMode(0);
			try
			{
				if(		(Manager_Setting.g_Information.cCOMM.iCount_TV <= 0)
					&&	(Manager_Setting.g_Information.cCOMM.iCount_Radio <= 0)
				)
				{
					Signal_invisible();
				}
				else
				{
					int	iLevel	= DxbPlayer.getSignalStrength();
					if(iLevel <= 0)
					{
						Manager_Setting.g_Information.cINDI.iSignal_Count++;
					}
					else
					{
						Manager_Setting.g_Information.cINDI.iSignal_Count = 0;
					}
					
					if(		(Manager_Setting.g_Information.cINDI.iSignal_Level >= 0)
						&&	(Manager_Setting.g_Information.cINDI.iSignal_Count >= 5)
						&&	(DxbView_Record.eState != DxbView_Record.STATE.RECORDING)
					)
					{
						Manager_Setting.g_Information.cINDI.iSignal_Level = -1;
						DxbView.updateScreen();
						if(DxbPlayer.mPlayer != null)
						{
							DxbPlayer.playSubtitle(DxbPlayer._OFF_);
							DxbPlayer.mPlayer.stop();
						}
					}
					else if(	(Manager_Setting.g_Information.cINDI.iSignal_Level < 0)
								&&	(iLevel > 0)
					)
					{
						DxbPlayer.start();
						Manager_Setting.g_Information.cINDI.iSignal_Level	= iLevel;
						DxbView.updateScreen();

						if(Manager_Setting.g_Information.cCOMM.curChannels != null)
						{
							if(DxbView_List.getTab() == 0)
								Manager_Setting.g_Information.cCOMM.curChannels.moveToPosition(Manager_Setting.g_Information.cCOMM.iCurrent_TV);
							else
								Manager_Setting.g_Information.cCOMM.curChannels.moveToPosition(Manager_Setting.g_Information.cCOMM.iCurrent_Radio);
						}

						DxbView.setChannel();

						if(DxbView.eState == DxbView.STATE.FULL)
						{
							if(		!Manager_Setting.g_Component.fullview.navi.vgNavi.isShown()
								&&	(DxbView_Record.eState == DxbView_Record.STATE.STOP)
							)
							{
								DxbPlayer.playSubtitle(Manager_Setting.g_Information.cOption.subtitle);
							}
						}
					}
					else
					{
						DxbPlayer.monitoringSignal(iLevel);
					}
					
					Signal_changeLevel(iLevel);
					if(		(DxbView.eState==DxbView.STATE.FULL)
						&&	!Signal_isVisible()
					)
					{
						DxbView.mHandler_Main.postDelayed(mRunnable_Signal, 1000);
					}
					else
					{
						Signal_visible();
					}
				}
			}
			catch(IllegalStateException e)
			{
				e.printStackTrace();
			}
		}
	};
	
	public static boolean Signal_isVisible()
	{
		//DxbLog(I, "Signal_isVisible()");
		
		return gComponent.ivSignal.isShown();
	}
	
	public static void Signal_visible()
	{
		//DxbLog(I, "Signal_visible()");
		
		DxbView.mHandler_Main.removeCallbacks(mRunnable_Signal);
		
			gComponent.ivSignal.setVisibility(View.VISIBLE);
		
		DxbView.mHandler_Main.postDelayed(mRunnable_Signal, 1000);
	}
	
	public static void Signal_invisible()
	{
		//DxbLog(I, "Signal_invisible()");
		
		gComponent.ivSignal.setVisibility(View.INVISIBLE);
	}
	
	public static void Signal_changeLevel(int iLevel)
	{
		//DxbLog(I, "Signal_changeLevel()");
		
		gComponent.ivSignal.setImageResource(rssIcon[iLevel]);
	}
	
	private static void DxbLog(int level, String mString)
	{
		if(DxbView.TAG == null)
		{
			return;
		}
		
		String TAG = "[[[DxbView_Indicator]]] ";
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