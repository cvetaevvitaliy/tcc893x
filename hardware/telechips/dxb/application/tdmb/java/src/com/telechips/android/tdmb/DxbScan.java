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

import com.telechips.android.tdmb.player.*;

public class DxbScan
{
	private static final int D = 0;
	private static final int E = 1;
	private static final int I = 2;
	private static final int V = 3;
	private static final int W = 4;
	
	static TDMBPlayer.OnSearchPercentListener ListenerOnSearchPercent = new TDMBPlayer.OnSearchPercentListener()
	{
		public void onSearchPercentUpdate(TDMBPlayer player, int idx, int nPercent)
		{
			DxbLog(I, "ListenerOnSearchPercent --> onSearchPercentUpdate::IDX[" + idx + "] (" + nPercent + ")");
			if(idx == 0)
			{
				DxbView_Message.mDialog_Scan.setMessage(		"Please wait while searching...\n[ "
													+	nPercent + "% ]"
					//								+	DxbPlayer_Control.getScanCount());
													);
			}
		}
	};
	
	static TDMBPlayer.OnSearchCompletionListener ListenerOnSearchCompletion = new TDMBPlayer.OnSearchCompletionListener()
	{
		public void onSearchCompletion(TDMBPlayer player, int idx, int manual)
		{
			DxbLog(I, "onSearchCompletion() [idx:" + idx + "][manual:" + manual + "]" + "[eState:" + DxbPlayer.eState + "]");
			if(idx == player.mModuleIndex)
			{
				if(DxbPlayer.eState != DxbPlayer.STATE.UI_PAUSE)
				{
					if(DxbPlayer.eState == DxbPlayer.STATE.OPTION_MANUAL_SCAN)
					{
						DxbView_Message.removeDialog();
						DxbPlayer.eState = DxbPlayer.STATE.GENERAL;
						if(manual == 0)
						{
							DxbView.setChannel();
							DxbView_Indicator.Signal_visible();
						}
					}
					else
					{
						DxbView_Message.removeDialog();
				
						DxbPlayer.eState	= DxbPlayer.STATE.GENERAL;
					
						DxbPlayer.setDefaultValue();
						DxbView_List.updateChannelList(0);
				
						Information.COMM	cInfo	= Manager_Setting.g_Information.cCOMM;
						if(cInfo.curChannels != null)
						{
							cInfo.iCount_TV 		= cInfo.curChannels.getCount();
							cInfo.iCount_Current	= cInfo.curChannels.getCount();
						}
						else
						{
							cInfo.iCount_TV 		= 0;
							cInfo.iCount_Current	= 0;
						}
						DxbLog(D, "onSearchCompletion() count - " + cInfo.iCount_TV);
				
						if(DxbRemote.getChannel() == 1)
						{
							DxbRemote.setChannel(0);
							Manager_Setting.mContext.onPause();
							Manager_Setting.mContext.onStop();
							Manager_Setting.mContext.onDestroy();
							Manager_Setting.mContext.finish();
							android.os.Process.killProcess(android.os.Process.myPid()); 
						}
						else
						{
							if(manual == 0)
							{
								if(cInfo.curChannels != null)
								{
									cInfo.curChannels.moveToFirst();
								}
								
								DxbView.resetChannel();
								DxbView.setChannel();
								DxbView_Indicator.Signal_visible();
								
								Manager_Setting.g_Information.cCOMM.iCurrent_Radio = 0;
								DxbView.mDB.putPosition(1, 0);
							}
						}
					}
				}
			}
		}
	};
	
	static void startFull()
	{
		DxbLog(I, "startFull() DxbPlayer.eState ="+ DxbPlayer.eState);
		
		if(DxbPlayer.eState != DxbPlayer.STATE.GENERAL)
		{
			DxbView_Message.updateToast(Manager_Setting.mContext.getResources().getString(R.string.please_wait_cancel_scanning));
			return;
		}

		DxbPlayer.eState = DxbPlayer.STATE.SCAN;
		DxbView_Indicator.Signal_visible();
		
		Information.COMM	info	= Manager_Setting.g_Information.cCOMM;
		info.iCount_TV		= 0;
		info.iCount_Radio	= 0;
		info.iCount_Current	= 0;

		DxbPlayer.search();
		Manager_Setting.mContext.showDialog(DxbView_Message.DIALOG_SCAN);
	}
	
	static void startManual(int Hz_frequency)
	{
		DxbLog(I, "startManual() DxbPlayer.eState ="+ DxbPlayer.eState);
		
		if(DxbPlayer.eState != DxbPlayer.STATE.GENERAL)
		{
			DxbView_Message.updateToast(Manager_Setting.mContext.getResources().getString(R.string.please_wait_cancel_scanning));
			return;
		}

		DxbPlayer.eState = DxbPlayer.STATE.SCAN;
		DxbView_Indicator.Signal_visible();
		
		Information.COMM	info	= Manager_Setting.g_Information.cCOMM;
		info.iCount_TV		= 0;
		info.iCount_Radio	= 0;
		info.iCount_Current	= 0;

		DxbPlayer.manualScan(Hz_frequency);
		Manager_Setting.mContext.showDialog(DxbView_Message.DIALOG_SCAN);
	}
	
	public static void cancel()
	{
		DxbLog(I, "cancel()");
		
		if(DxbPlayer.mPlayer != null)
		{
			DxbPlayer.mPlayer.searchCancel();
		}
	}
	
	private static void DxbLog(int level, String mString)
	{
		if(DxbView.TAG == null)
		{
			return;
		}
		
		String TAG = "[[[DxbScan]]] ";
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
