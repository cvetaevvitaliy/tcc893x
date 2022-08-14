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
import android.view.View;
import android.widget.TextView;
import android.widget.ImageView;;

public class DxbView_Indicator extends DxbView {

	private final View mView;
	private final View mBottomView;

	private ImageView mivSection;
	private ImageView mivSignal;
	private TextView mtvTime;

	private int	miSignal_Level = 4;
	private int miSignal_Count = 0;
	private int	miSignal_Strength	= 0;
	private int miSignal_Quality	= 0;
	private String	msSignal_Message	= null;

	private static final int[] mRssIcon = {	R.drawable.dxb_portable_indicator_rssi_icon_00,
											R.drawable.dxb_portable_indicator_rssi_icon_01,
											R.drawable.dxb_portable_indicator_rssi_icon_02,
											R.drawable.dxb_portable_indicator_rssi_icon_03,
											R.drawable.dxb_portable_indicator_rssi_icon_04 };

	public DxbView_Indicator(Context context, View v1, View v2) {
		super(context);
		mView = v1;
		mBottomView = v2;

		DxbLog(I, "DxbView_Indicator()");
		setComponent();
		setFontSize();
	}

	public void releaseAll()
	{
		getMainHandler().removeCallbacks(mRunnable_Signal);
	}
	
	public void reset()
	{
		getMainHandler().removeCallbacks(mRunnable_Signal);
		miSignal_Level	= 0;
		miSignal_Count	= 0;
		getMainHandler().postDelayed(mRunnable_Signal, 1000);
	}

	@Override
	public void setVisible(boolean v)
	{
		DxbLog(I, "setVisible(" + v + ")");
		mivSection.setVisibility(View.VISIBLE);
		mtvTime.setVisibility(View.VISIBLE);
		Signal_visible();
		mView.setVisibility(v ? View.VISIBLE : View.GONE);
		mBottomView.setVisibility(v ? View.VISIBLE : View.GONE);
	}

	private void setComponent() {
		DxbLog(I, "setComponent()");
		mivSection	= (ImageView)mView.findViewById(R.id.indicator_section);
		mivSignal	= (ImageView)mView.findViewById(R.id.indicator_signal);
		mtvTime		= (TextView)mView.findViewById(R.id.indicator_time);
	}
	
	private void setFontSize() {
		DxbLog(I, "setFontSize()");
		mtvTime.setTextSize(getTextSize(18));
	}
	
//	int	iMessage_Test_count	= 0;
	private Runnable mRunnable_Signal = new Runnable()
	{
		public void run()
		{
			DxbLog(I, "mRunnable_Signal --> run()");
			
			if(		(getPlayer().isValid() == false)
				||	(getPlayer().eState == DxbPlayer.STATE.UI_PAUSE)
				||	(getPlayer().isFilePlay())
				||	(!getPlayer().isDVBPlayerActivityON)
			)
			{
				return;
			}
			
			int iCurTime	= getPlayer().getCurrentTime();
			String	sHH		= String.format("%02d", iCurTime/60/60);
			String	sMM		= String.format("%02d", (iCurTime/60) % 60);
			mtvTime.setText(sHH+":"+sMM);

			//getPlayer().mPlayer.startTestMode(0);
			try
			{
				if ((getPlayer().getChannelCount(0) <= 0) && (getPlayer().getChannelCount(1) <= 0))
				{
					Signal_invisible();
				}
				else
				{
					//	Display - Level
					{
						int	iLevel	= getPlayer().getSignalLevel();
						if (iLevel <= 0)
						{
							miSignal_Count++;
						}
						else
						{
							miSignal_Count = 0;
						}
						
						if ((miSignal_Level >= 0) && (miSignal_Count >= 5) && (getPlayer().getRecordState() != REC_STATE.RECORDING))
						{
							miSignal_Level = -1;
							getContext().updateScreen();
							if(getPlayer().isValid())
							{
								getPlayer().playSubtitle(DxbPlayer._OFF_);
								getPlayer().stop();
							}
						}
						else if ((miSignal_Level < 0) && (iLevel > 0))
						{
							miSignal_Level	= iLevel;
							getContext().updateScreen();
							setChannel();
						}
							
						Signal_changeLevel(iLevel);
						if(!Signal_isVisible())
						{
							getMainHandler().postDelayed(mRunnable_Signal, 1000);
						}
						else
						{
							Signal_visible();
						}
					}
					
					//	Display	- Strength
					{
						int	iStrength	= getPlayer().getSignalStrength();
						if(iStrength != miSignal_Strength)
						{
							getContext().updateSignalStrength(iStrength);
							miSignal_Strength	= iStrength;
						}
					}
					
					//	Display	- Quality
					{
						int	iQuality	= getPlayer().getSignalQuality();
						if(iQuality != miSignal_Quality)
						{
							getContext().updateSignalQuality(iQuality);
							miSignal_Quality	= iQuality;
						}
					}
					
					//	Display - Message
					{
						String	sMessage	= getPlayer().getSignalMessage();
						/*if(iMessage_Test_count == 0)
						{
							sMessage	= null;
						}
						else if(iMessage_Test_count == 1)
						{
							sMessage	= "TestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTest";
						}
						else if(iMessage_Test_count >= 2)
						{
							sMessage	= "MessageMessageMessageMessageMessageMessageMessage";
							iMessage_Test_count = -1;
						}
						iMessage_Test_count ++;*/
						
						if(sMessage != msSignal_Message)
						{
							getContext().updateSignalMessage(sMessage);
							msSignal_Message	= sMessage;
						}
					}
				}
			}
			catch(IllegalStateException e)
			{
				e.printStackTrace();
			}
		}
	};

	public int getSignalLevel()
	{
		DxbLog(I, "getSignalLevel(miSignal_Level=" + miSignal_Level + ")");
		return miSignal_Level;
	}
	
	private boolean Signal_isVisible()
	{
		DxbLog(I, "Signal_isVisible()");
		return mivSignal.isShown();
	}
	
	private void Signal_visible()
	{
		DxbLog(I, "Signal_visible()");
		
		getMainHandler().removeCallbacks(mRunnable_Signal);
		if(!mivSignal.isShown())
		{
			mivSignal.setVisibility(View.VISIBLE);
		}
		getMainHandler().postDelayed(mRunnable_Signal, 1000);
	}
	
	private void Signal_invisible() {
		DxbLog(I, "Signal_invisible()");
		mivSignal.setVisibility(View.INVISIBLE);
	}
	
	private void Signal_changeLevel(int iLevel) {
		DxbLog(I, "Signal_changeLevel()");
		mivSignal.setImageResource(mRssIcon[iLevel]);
	}
	
	@Override
	protected String getClassName() {
		//return "[[[DxbView_Indicator]]]";
		return null;
	}
}
