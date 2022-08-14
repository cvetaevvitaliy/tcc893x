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
import com.telechips.android.isdbt.player.DxbPlayer.REC_STATE;
import com.telechips.android.isdbt.player.isdbt.Channel;
import com.telechips.android.isdbt.player.isdbt.EWSData;
import com.telechips.android.isdbt.player.isdbt.SCInfoData;

import android.content.Context;
import android.view.View;
import android.widget.TextView;
import android.widget.ImageView;;

public class DxbView_Indicator extends DxbView {

	private final View mView;
	private final View mBottomView;

	private ImageView mivEWS;
	private ImageView mivSection;
	private ImageView mivSignal;
	private TextView mtvTime;

	private int	miSignal_Level = 4;
	private int miSignal_Count = 0;

	private static final int[] mSectionIcon = {	R.drawable.dxb_portable_indicator_fseg_icon,
												R.drawable.dxb_portable_indicator_1seg_icon };

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
	protected void EWS(DxbPlayer player, int state, EWSData obj)
	{
		DxbLog(I, "EWS()");
		
		String message = "";

		if(state != 0)
		{
			CharSequence[] values;
			CharSequence[] entries;
			String signaltype = "";
			String localcode = "";
	
			values = getResources().getTextArray(R.array.signal_type_values);
			entries = getResources().getTextArray(R.array.signal_type_entries);
			for(int i=0; i<values.length; i++)
			{
				if(Integer.parseInt(values[i].toString()) == obj.SignalType)
				{
					signaltype = entries[i].toString();
					break;
				}
			}
	
			message = getResources().getString(R.string.ews_running_start) + "\n" + signaltype + "\n" + getResources().getString(R.string.local_code_entries_tag);
			values = getResources().getTextArray(R.array.local_code_values);
			entries = getResources().getTextArray(R.array.local_code_entries);
			for(int j=0; j<obj.LocalCodeLength; j++)
			{
				for(int i=0; i<values.length; i++)
				{
					if(Integer.parseInt(values[i].toString()) == obj.LocalCode[j])
					{
						localcode = entries[i].toString();
						message = message + " " + localcode;
						break;
					}
				}
			}

			updateToast(message);

			mivEWS.setVisibility(View.VISIBLE);
		}
		else
		{
			message = getResources().getString(R.string.ews_running_end);
			updateToast(message);
				
			mivEWS.setVisibility(View.INVISIBLE);
		}
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
		mivEWS			= (ImageView)mView.findViewById(R.id.indicator_ews);
		mivSection		= (ImageView)mView.findViewById(R.id.indicator_section);
		mivSignal		= (ImageView)mView.findViewById(R.id.indicator_signal);
		mtvTime	= (TextView)mView.findViewById(R.id.indicator_time);
	}
	
	private void setFontSize() {
		DxbLog(I, "setFontSize()");
		if(getDisplayWidth() > 800) {
			mtvTime.setTextSize(25 / getDisplayDensity());
		} else {// if(DxbPlayer_Control.eResolution == DxbPlayer_Control.DISPLAY_RESOLUTION.p480)
			mtvTime.setTextSize(18 / getDisplayDensity());
		}		
	}
	
	private Runnable mRunnable_Signal = new Runnable()
	{
		public void run()
		{
			DxbLog(I, "mRunnable_Signal --> run()");
			
			if((getPlayer().isValid() == false) || (getPlayer().eState == DxbPlayer.STATE.UI_PAUSE || getPlayer().isFilePlay() == 1))
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
				int	iLevel	= getPlayer().getSignalStrength();

				if (getPlayer().getChannelCount(0) <= 0)
				{
					Signal_invisible();
				}
				else
				{
					if (iLevel <= 0)
					{
						miSignal_Count++;
					}
					else
					{
						miSignal_Count = 0;
					}
					
					if ((miSignal_Level >= 0) && (miSignal_Count >= 2) && (getPlayer().getRecordState() != DxbPlayer.REC_STATE.RECORDING))
					{
						miSignal_Level = -1;
						getContext().updateScreen();
						if(getPlayer().isValid())
						{
							getPlayer().playSubtitle(DxbPlayer._OFF_);
							getPlayer().stop();
						}
					}
					else if ((miSignal_Level < 0) && (miSignal_Count == 10) && (iLevel == 0)  && (getPlayer().getRecordState() != DxbPlayer.REC_STATE.RECORDING))
					{
						if(getPlayer().isValid() && (getPlayer().getRecordState() == DxbPlayer.REC_STATE.STOP))
						{
							if(getPlayer().cOption.handover == true)
							{
								getPlayer().startHandover();
								miSignal_Level = -2;
							}
						}
					}
					else if ((miSignal_Level < 0) && (iLevel > 0))
					{
						if(miSignal_Level == -1)
						{
							setChannel();
						}
						
						miSignal_Level	= iLevel;
						getContext().updateScreen();
						
					}
					else if(getPlayer().getRecordState() != DxbPlayer.REC_STATE.RECORDING)
					{
						if(getPlayer().isValid() && (getPlayer().getRecordState() == DxbPlayer.REC_STATE.STOP))
						{
							getPlayer().monitorSignal();
						}
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
				
				if(getPlayer().cOption.signal_quality == true)
				{
					updateSignalQualityToast(getPlayer().getSignalMessage());
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

		if(!mivSection.isShown())
		{
			mivSection.setVisibility(View.VISIBLE);
		}
		
		getMainHandler().removeCallbacks(mRunnable_Signal);
		if(!mivSignal.isShown())
		{
			mivSignal.setVisibility(View.VISIBLE);
		}
		getMainHandler().postDelayed(mRunnable_Signal, 1000);
	}
	
	private void Signal_invisible() {
		DxbLog(I, "Signal_invisible()");
		
		mivSection.setVisibility(View.INVISIBLE);
		mivSignal.setVisibility(View.INVISIBLE);
	}
	
	private void Signal_changeLevel(int iLevel) {
		DxbLog(I, "Signal_changeLevel()");
		
		Channel channel = getPlayer().getCurChannel();
		if(channel.serviceType == getPlayer().ISDBT_SECTION_FSEG)
		{
			mivSection.setImageResource(mSectionIcon[0]);
		}
		else
		{
			mivSection.setImageResource(mSectionIcon[1]);
		}

		mivSignal.setImageResource(mRssIcon[iLevel]);		
	}
	
	@Override
	protected String getClassName() {
		//return "[[[DxbView_Indicator]]]";
		return null;
	}
}
