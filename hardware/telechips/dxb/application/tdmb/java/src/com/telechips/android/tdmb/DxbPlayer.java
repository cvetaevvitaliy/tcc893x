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

import java.io.UnsupportedEncodingException;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Color;
import android.graphics.Rect;
import android.os.storage.StorageManager;
import android.util.Log;
import android.view.KeyEvent;

import com.telechips.android.tdmb.DxbAdapter_Service.ViewHolder;
import com.telechips.android.tdmb.player.Channel;
import com.telechips.android.tdmb.player.DABDLSData;
import com.telechips.android.tdmb.player.EWSData;
import com.telechips.android.tdmb.player.TDMBPlayer;

public class DxbPlayer
{
	static String TAG = "[[[DxbPlayer]]] ";

	private static final int D = 0;
	private static final int E = 1;
	private static final int I = 2;
	private static final int V = 3;
	private static final int W = 4;
	
	private static Component	gComponent;
	private static Information	gInformation;
	private static Information.OPTION	gInfo_Option;

	/*
	 * check playr OFF state
	 */
	static boolean isOFF	= false;
	
	// Dxb_Player State
	public enum STATE
	{
		GENERAL,
		OPTION_MANUAL_SCAN,
		OPTION_MAKE_PRESET,
		SCAN,
		SCAN_STOP,
		UI_PAUSE,
		NULL
	}
	public static STATE	eState	= STATE.NULL;
	
	public static ChannelManager mChannelManager = null;
	private static Channel mChannel;
	
	static final int	_OFF_	= 0;
	static final int	_ON_	= 1;
	
	/*	Visible Check	*/
	public static boolean VISIBLE_LIST_preview_half		= true;


	/*	TDMB Only -->	*/
		public static TDMBPlayer mPlayer	= null;
		public static TDMBPlayer mDATASVC	= null;

		private static final int KOREA_BAND	= 0x01;
		private static final int BAND_III = 0x02;
		private static final int L_BAND = 0x03;
		private static final int CANADA_BAND = 0x04;
		private static final int CHINA_BAND_III = 0x05;
		private static final int KOREA_BAND_ONLYSEOUL = 0x06;
		
		private static int[] freqList = { 181280, 183008, 184736, 205280, 207008, 208736, 211280 };
		
		private static int iCountryCode = KOREA_BAND; //886; //Tiwan

		private static final int TYPE_DAB = 1;
		private static final int TYPE_TDMB = 2;
		private static final int TYPE_DABP = 4;

		// Copied from MediaPlaybackService in the Music Player app. Should be
		// public, but isn't.
		private static final String SERVICECMD = "com.android.music.musicservicecommand";
		private static final String CMDNAME    = "command";
		private static final String CMDPAUSE   = "pause";
	
		private static EWSParser mEWSParser;	
	/*	TDMB Only -->	*/
	
	static void init()
	{
		DxbLog(I, "init()");
		
		gComponent		= Manager_Setting.g_Component;
		gInformation	= Manager_Setting.g_Information;
		gInfo_Option	= Manager_Setting.g_Information.cOption;
		
		if( mChannelManager == null )
		{
			mChannelManager = new ChannelManager(Manager_Setting.mContext);
			mChannelManager.open();
		}
		
		if(DxbView.mDB != null)
		{
			DxbView.mDB.close();
			DxbView.mDB = null;
		}
		
		DxbView.mDB = new DxbDB_Setting(Manager_Setting.mContext).open();
		if(DxbView.mDB != null)
		{
			Manager_Setting.g_Information.cCOMM.iCurrent_TV		= DxbView.mDB.getPosition(0);
			Manager_Setting.g_Information.cCOMM.iCurrent_Radio	= DxbView.mDB.getPosition(1);
			
			gInfo_Option.preset			= DxbView.mDB.getPreset();
			gInfo_Option.area_code			= DxbView.mDB.getArea();
			gInfo_Option.parental_rating	= DxbView.mDB.getAge();
		}
 		else
		{
			DxbLog(D, "not open DB!!!");
		}		
	}
	
	static void setListener_Player()
	{
		DxbLog(I, "setListener_Player()");
		
		/*	TDMB Only -->	*/
			if(mPlayer == null)
			{
				mPlayer = new TDMBPlayer();
				DxbView.mIsPrepared = false;
			}
			if(mDATASVC == null)
			{
				//mDATASVC = new TDMBPlayer();
				mIsDATASVCPrepared	= false;
			}
			setListener_TDMB();
		/*	TDMB Only -->	*/

		if (mChannel == null)
		{
			mChannel = new Channel();
		}
		
		mPlayer.setDisplay(DxbView.mSurfaceHolder);
		
		//	Common
		mPlayer.setOnPreparedListener(DxbView.ListenerOnPrepared);
		mPlayer.setOnAudioOutputListener(DxbView.ListenerOnAudioOutput);
		mPlayer.setOnVideoOutputListener(DxbView.ListenerOnVideoOutput);
		mPlayer.setOnErrorListener(DxbView.ListenerOnError);
		
		//	Scan
		mPlayer.setOnSearchPercentListener(DxbScan.ListenerOnSearchPercent);
		mPlayer.setOnSearchCompletionListener(DxbScan.ListenerOnSearchCompletion);
		
		//	Recording
		mPlayer.setOnRecordingCompletionListener(DxbView_Record.ListenerOnRecordingCompletion);
		
		setPrepare();
		//setDATAPrepare();
	}
	
	public static void setPrepare()
	{
		DxbLog(I, "setPrepare()");
		
		if(mPlayer != null)
		{
			//mPlayer.setWakeMode(Manager_Setting.mContext, PowerManager.SCREEN_BRIGHT_WAKE_LOCK|PowerManager.ON_AFTER_RELEASE);
			int bbtype = 4;
			int moduleidx = 0;
			mPlayer.mModuleIndex = moduleidx;
			mPlayer.setBBModuleIndex(bbtype, moduleidx);
			mPlayer.prepare(bbtype, "/data/data/com.telechips.android.tdmb/databases/TDMB.db"); // 1:TCC3510 CSPI+SPIMS 2:TCC3510 CSPI+STS 3:TCC3510 or TCC3161 I2C+STS 4:TCC3171 I2C+STS
		}
	}
	

	static boolean mIsDATASVCPrepared = false;

	static TDMBPlayer.OnPreparedListener ListenerOnPrepared = new TDMBPlayer.OnPreparedListener()
	{
		public void onPrepared(TDMBPlayer player, int idx, int ret)
		{
			if(idx != player.mModuleIndex)
				return;
			
			Log.d(TAG,		"DATASVC OnPreparedListener   "
						+	"     : idx : "
						+   idx);

			if(mIsDATASVCPrepared == true)
				return;
			mIsDATASVCPrepared = true;

			if(mDATASVC != null && mIsDATASVCPrepared == true)
			{
				Cursor AllChannels = mChannelManager.getAllChannels();
				int ch_num = AllChannels.getCount();
				
				Channel dataCh = new Channel();
				for(int i = 0; i < ch_num; i++)
				{
					AllChannels.moveToPosition(i);
					
					dataCh.ensembleName	= AllChannels.getString(1);
					dataCh.ensembleID	= AllChannels.getInt(2);
					dataCh.ensembleFreq	= AllChannels.getInt(3);
					dataCh.serviceName	= AllChannels.getString(4);
					dataCh.serviceID		= AllChannels.getInt(5);
					dataCh.channelName	= AllChannels.getString(6);
					dataCh.channelID		= AllChannels.getInt(7);
					dataCh.type			= AllChannels.getInt(8);
					dataCh.bitrate			= AllChannels.getInt(9);
					for(int regIndex=0; regIndex<7; regIndex++)
					{
						dataCh.reg[regIndex] = AllChannels.getInt(regIndex+10);
					}

					if(dataCh.type == 3)
					{
						DxbLog(I, "Select Data Channel : " + dataCh.ensembleFreq + "," + dataCh.serviceName);
						manual_setChannel(mDATASVC, dataCh);
						break;
					}
				}
			}
		}
	};

	static void setDATAPrepare()
	{
		DxbLog(I, "setDATAPrepare()");

		if(mDATASVC != null && mIsDATASVCPrepared == false)
		{
			try
			{
				Thread.sleep(5000);
			}
			catch(InterruptedException e)
			{
				Log.d(TAG, "sleep fail!");
			}

			int bbtype = 4;
			int moduleidx = 1;
			mDATASVC.mModuleIndex = moduleidx;
			mDATASVC.setOnPreparedListener(ListenerOnPrepared);
			mDATASVC.setBBModuleIndex(bbtype, moduleidx);
			mDATASVC.prepare(bbtype, null); // 1,2,3:not support 4:TCC3171 I2C+STS
		}
	}
	

	/************************************************************************************************************************
	 * 	setListener  -----> Start
	 ************************************************************************************************************************/
		private static void setListener_TDMB()
		{
			//	TDMB Only
			if (mEWSParser == null)
			{
				mEWSParser = new EWSParser();
			}

			mPlayer.setOnChannelUpdateListener(ListenerOnChannelUpdate);
			mPlayer.setOnDABDLSDataUpdateListener(ListenerOnDABDLSDataUpdate);
			mPlayer.setOnEWSDataUpdateListener(ListenerOnEWSDataUpdate);
			mPlayer.setOnTunningCompletionListener(ListenerTunningCompletion);
		}
	
		static TDMBPlayer.OnChannelUpdateListener ListenerOnChannelUpdate = new TDMBPlayer.OnChannelUpdateListener()
		{
			public void onChannelUpdate(TDMBPlayer player, int moduleidx, Channel channel)
			{
				Log.i(TAG, "Please wait while searching... moduleidx : " + moduleidx);
				Log.i(TAG, channel.ensembleName + " :: Ensemble ID : " + channel.ensembleID + " :: Ensemble FREQ : " + channel.ensembleFreq);
				Log.i(TAG, channel.serviceName + " :: Service ID : " + channel.serviceID);
				Log.i(TAG, channel.channelName + " :: Channel ID : " + channel.channelID + " Channel TYPE : " + channel.type + " Channel BITRATE : " + channel.bitrate);
				Log.i(TAG, channel.reg[0] + " " + channel.reg[1] + " " + channel.reg[2] + " " + channel.reg[3] + " " + channel.reg[4] + " " + channel.reg[5] + " " + channel.reg[6]);
			}
		};
		
		static TDMBPlayer.OnDABDLSDataUpdateListener ListenerOnDABDLSDataUpdate = new TDMBPlayer.OnDABDLSDataUpdateListener()
		{
			public void onDABDLSDataUpdate(TDMBPlayer player, DABDLSData dabdlsData)
			{
				if( dabdlsData.dlsDataType == 4 )
				{
					if( dabdlsData.dlsDataSize > 0 )
					{
						//mDialogText.setText(dabdlsData.strDLSData);
					}
				}
				else
				{
					try
					{
						if(dabdlsData.dlsDataSize > 0)
						{
							dabdlsData.strDLSData = new String(dabdlsData.dlsData, 0, dabdlsData.dlsDataSize, "KSC5601");
							//mDialogText.setText(dabdlsData.strDLSData);
						}
					}
					catch (UnsupportedEncodingException e)
					{
						e.printStackTrace();
					}
				}
			}
		};
		
		static TDMBPlayer.OnEWSDataUpdateListener ListenerOnEWSDataUpdate = new TDMBPlayer.OnEWSDataUpdateListener()
		{
			public void onEWSDataUpdate(TDMBPlayer player, EWSData ewsData)
			{
				int i, j, h, cnt;
				int srcIndex = 0, checkVariable = 0;
				String strEWSMessage;
	
				if(ewsData.EWSMessageLen < 8)
				{
					Log.e(TAG, "EWS message too short\r\n");
					return;
				}
	
				if(ewsData.EWSMessage == null)
				{
					Log.e(TAG, "EWS message is null\r\n");
					return;
				}
	
				mEWSParser.id = ewsData.EWSMessageID;
	
				// 3 bytes
				for(i=0; i<3; i++)
				{
					mEWSParser.type[i] = ewsData.EWSMessage[srcIndex++];
				}
	
				//String strEWSType = new String(mEWSParser.type);
				String strEWSType = String.format("%c%c%c", mEWSParser.type[0], mEWSParser.type[1], mEWSParser.type[2]);
	
				for(i=0; i<mEWSParser.ews_type.length; i++)
				{
					if(mEWSParser.ews_type[i][0].equals(strEWSType))
					{
						strEWSType = mEWSParser.ews_type[i][1];
						break;
					}
				}
	
				if(i == mEWSParser.ews_type.length)
				{
					Log.e(TAG, "Unknown EWS type\r\n");
					return;
				}
	
				strEWSMessage = "[" + mEWSParser.ews_type[i][1] + "]\r\n";
	
				// 2 bits
				if(ewsData.EWSMessage[srcIndex] < 0)
				{
					checkVariable = ((int)ewsData.EWSMessage[srcIndex]) + 256;
					mEWSParser.priority = (char)(checkVariable >> 6);
				}
				else
				{
					mEWSParser.priority = (char)(((int)ewsData.EWSMessage[srcIndex]) >> 6);
				}
	
				if(mEWSParser.priority >= mEWSParser.ews_priority.length)
				{
					Log.e(TAG, "Unknown EWS priority\r\n");
					return;
				}
	
				strEWSMessage += "[" + mEWSParser.ews_priority[mEWSParser.priority] + "]\r\n";
	
				// 28 bits
				if(ewsData.EWSMessage[srcIndex] < 0)
				{
					checkVariable = ((int)ewsData.EWSMessage[srcIndex]) + 256;
					mEWSParser.time = (checkVariable & 0x3F) << 22;
				}
				else
				{
					mEWSParser.time = (ewsData.EWSMessage[srcIndex] & 0x3F) << 22;
				}
				srcIndex++;
	
				if(ewsData.EWSMessage[srcIndex] < 0)
				{
					checkVariable = ((int)ewsData.EWSMessage[srcIndex]) + 256;
					mEWSParser.time |= (checkVariable << 14);
				}
				else
				{
					mEWSParser.time |= (ewsData.EWSMessage[srcIndex] << 14);
				}
				srcIndex++;	
	
				if(ewsData.EWSMessage[srcIndex] < 0)
				{
					checkVariable = ((int)ewsData.EWSMessage[srcIndex]) + 256;
					mEWSParser.time |= (checkVariable << 6);
				}
				else
				{
					mEWSParser.time |= (ewsData.EWSMessage[srcIndex] << 6);
				}
				srcIndex++;
				
				if(ewsData.EWSMessage[srcIndex] < 0)
				{
					checkVariable = ((int)ewsData.EWSMessage[srcIndex]) + 256;
					mEWSParser.time |= (checkVariable >> 2);
				}
				else
				{
					mEWSParser.time |= (ewsData.EWSMessage[srcIndex] >> 2);
				}
	
				int ewsTime = mEWSParser.time >> 11;
				int y = (int)((ewsTime - 15078.2) / 365.25);
				int m = (int)((ewsTime - 14956.1 - (int)(y * 365.25)) / 30.6001);
				int d = ewsTime - 14956 - (int)(y * 365.25) - (int)(m * 30.6001);
				int k = (m == 14 || m == 15) ? 1 : 0;
				int Year = 1900 + y + k;
				int Month = m - 1 - k * 12;
				int Day = d;
				int Hour = (mEWSParser.time >> 6) & 0x1F;
				int Minute = (mEWSParser.time & 0x3F);
	
				String ewsMSGStr = String.format("[%04d\uB144 %02d\uC6D4 %02d\uC77C %02d\uC2DC %02d\uBD84]\r\n", Year, Month, Day, Hour, Minute);
				strEWSMessage += ewsMSGStr;
	
				// 3 bits
				if(ewsData.EWSMessage[srcIndex] < 0)
				{
					checkVariable = ((int)ewsData.EWSMessage[srcIndex]) + 256;
				}
				else
				{
					checkVariable = ewsData.EWSMessage[srcIndex];
				}
				mEWSParser.local_type = (char)((checkVariable & 0x03) << 1);
				srcIndex++;
				if(ewsData.EWSMessage[srcIndex] < 0)
				{
					checkVariable = ((int)ewsData.EWSMessage[srcIndex]) + 256;
				}
				else
				{
					checkVariable = ewsData.EWSMessage[srcIndex];
				}
				mEWSParser.local_type |= (char)(checkVariable >> 7);
	
				// 4 bits
				mEWSParser.local_count = (char)(((checkVariable & 0x78) >> 3) + 1);
	
				// 3 bits reserved
				srcIndex++;
	
				if(mEWSParser.local_type == 0)	// 전국
				{	
					mEWSParser.local_count = 0;
				}
				else if(mEWSParser.local_type == 1)	// 대한민국 정부 지정
				{		
				}
				else if(mEWSParser.local_type == 2)	// 행정동 표기
				{
				}
				else	// Rfu
				{
					return;
				}
	
				if(ewsData.EWSMessage[srcIndex] < 0)
				{
					checkVariable = ((int)ewsData.EWSMessage[srcIndex]) + 256;
				}
				else
				{
					checkVariable = ewsData.EWSMessage[srcIndex];
				}
				for(i=0; i<mEWSParser.local_count; i++)
				{
					mEWSParser.local_code_main[i] = 0;
					mEWSParser.local_code_sub[i] = 0;
					for(j=0; j<10; j++)
					{
						if(j < 2)
						{
							mEWSParser.local_code_main[i] <<= 4;
							mEWSParser.local_code_main[i] |= (checkVariable - 0x30);
							srcIndex++;
						}
						else
						{
							mEWSParser.local_code_sub[i] <<= 4;
							mEWSParser.local_code_sub[i] |= (checkVariable - 0x30);
							srcIndex++;
						}
						if(ewsData.EWSMessage[srcIndex] < 0)
						{
							checkVariable = ((int)ewsData.EWSMessage[srcIndex]) + 256;
						}
						else
						{
							checkVariable = ewsData.EWSMessage[srcIndex];
						}
					}
				}
	
				if(mEWSParser.local_type == 0)
					cnt = 1;
				else
					cnt = mEWSParser.local_count;
	
				for(i = 0; i < cnt; i++)
				{
					if(mEWSParser.local_type == 0)	// 전국
					{	
						ewsMSGStr = mEWSParser.ews_Local_code[0];
					}
					else
					{
						for(j=0; j<mEWSParser.ews_Local_code.length; j++)
						{
							if(mEWSParser.local_code_main[i] == mEWSParser.ews_Local_code_index[j])
							{
								ewsMSGStr = mEWSParser.ews_Local_code[j];
								break;
							}
						}
					}
	
					if(ewsMSGStr != null)
					{
						strEWSMessage += "[" + ewsMSGStr + "]";
					/*
						for(j=0; j<mEWSParser.ews_Local_code.length; j++)
						{
							if(mEWSParser.local_code_main[i] == mEWSParser.ews_Local_code_index[j])
							{
								ewsMSGStr = mEWSParser.GetLocalSubCode(j, i);
								break;
							}
						}
					*/
						for(h=0; h<mEWSParser.ews_Local_code.length; h++)
						{
							if(mEWSParser.local_code_main[i] == mEWSParser.ews_Local_code_index[h])
							{
								for(j=0; j<mEWSParser.GetLocalSubCodeCount(h); j++)
								{
									ewsMSGStr = mEWSParser.GetLocalSubCode(h, mEWSParser.local_code_sub[i]);
									break;
								}
								break;
							}
						}
	
						if(ewsMSGStr != null)
						{
							strEWSMessage += "[" + ewsMSGStr + "]";
						}
					}
				}
	
				strEWSMessage += "\r\n";
	
				int remain_length = ewsData.EWSMessageLen - srcIndex;
				String ewsCommentStr = null;
				if(remain_length>0)
				{
					char chMsg;
					mEWSParser.message = new byte[remain_length];
	
					for(i=0; i<remain_length; i++)
					{
						mEWSParser.message[i] = ewsData.EWSMessage[srcIndex];
						srcIndex++;
					}
	
					try {
						ewsCommentStr = new String(mEWSParser.message, 0, remain_length, "UTF-16LE");
					}
					catch (UnsupportedEncodingException e) {
						e.printStackTrace();
					}
				}
				else
					ewsCommentStr = null;
	
				strEWSMessage += "[" + ewsCommentStr + "]";
				
				Log.w("#########################", strEWSMessage);
			}
		};
	
		static TDMBPlayer.OnTunningCompletionListener ListenerTunningCompletion = new TDMBPlayer.OnTunningCompletionListener()
		{
			public void onTunningCompletion(TDMBPlayer player, int idx, int bStatus)
			{
				DxbLog(I, "onTunningCompletion() [idx:" + idx + "]");

				if(idx != player.mModuleIndex)
					return;
				
				/*updateSignalTimer();
	
				if (mCheckCurrent_Player_State != UI_PAUSE_STATE)
				{
					mCurrent_Player_State = GENERAL_STATE;
				}
	
				if (bStatus == 1)
				{
					mCurrent_Player_State = GENERAL_STATE;
				}
				else
				{
					if (mCurrent_Player_State != UI_PAUSE_STATE)
					{
						mCurrent_Player_State = GENERAL_STATE;
					}
	
					if (mCurrent_Player_State == UI_PAUSE_STATE)
					{
						mPlayer.release();
					}
				}
				mCurrent_Player_State = GENERAL_STATE;*/
				
				/*
				 * 음악이 재생 중이면 재생을 멈추도록 Command를 전달한다.
				 * 음악 재생 정지 Command는 원하는 곳으로 이동 가능함.
				 */
				if(Manager_Setting.mContext != null)
				{
					Intent i = new Intent(SERVICECMD);
					i.putExtra(CMDNAME, CMDPAUSE);
					Manager_Setting.mContext.sendBroadcast(i);
				}
				useSurface(0);
			}
		};
	/************************************************************************************************************************
	 * 	setListener -----> End
	 ************************************************************************************************************************/

		
	/************************************************************************************************************************
	 * 	device access -----> Start
	 ************************************************************************************************************************/
		public static void start()
		{
			DxbLog(I, "start() iCountryCode="+iCountryCode);
			
			if(mPlayer != null)
			{
				mPlayer.start(iCountryCode);
			}
		}
		
		/*	Surface	*/	
			public static void setSurface()
			{
				DxbLog(I, "setSurface()");
				if(mPlayer != null)
				{
					mPlayer.setSurface();
					mPlayer.setDisplayEnable();
				}
			}

			public static void releaseSurface()
			{
				DxbLog(I, "releaseSurface()");
				if(mPlayer != null)
				{
					mPlayer.setDisplayDisable();
					mPlayer.releaseSurface();
				}
			}
			
			public static void useSurface(int arg)
			{
				DxbLog(I, "useSurface(arg = " + arg + ")");
				
				if(mPlayer != null)
				{
					mPlayer.useSurface(arg);
				}
			}
			
			public static void setLCDUpdate()
			{
				if(mPlayer != null)
				{
					mPlayer.setLCDUpdate();
				}
			}

			public static void setScreenMode(int screenMode)
			{
				DxbLog(I, "setScreenMode(screenMode = " + screenMode + ")");

				Rect crop_rect = DxbView.FullView_SetLayout();
			}

		/*	Scan	*/
			public static void search()
			{
				DxbLog(I, "search(freqList=" + ")");
				
				if(mPlayer != null)
				{
					mPlayer.search(KOREA_BAND, freqList, 0);
					//mPlayer.search(KOREA_BAND, freqList, freqList.length);
				}
			}
	
			public static void manual_setChannel(TDMBPlayer player, Channel channel)
			{
				start();
				//DxbPlayer.eState = DxbPlayer.STATE.OPTION_MANUAL_SCAN;

				int[] chInfo = {player.mModuleIndex, channel.ensembleFreq, channel.ensembleID, channel.serviceID, channel.channelID, channel.type, channel.bitrate,
								channel.reg[0], channel.reg[1], channel.reg[2], channel.reg[3], channel.reg[4], channel.reg[5], channel.reg[6]};

				player.manual_setChannel(chInfo);
			}
	
			public static void manualScan(int Hz_frequency)
			{
				DxbPlayer.eState = DxbPlayer.STATE.OPTION_MANUAL_SCAN;
				mPlayer.manual_search(Hz_frequency);
			}
				
			private static void makePresetList()
			{
			}
			
			public static String getScanCount()
			{
				Cursor	cursorChannel;
				int	iCountTV;
				String	return_Count;
				
				cursorChannel	= DxbPlayer.getChannels(0);
				iCountTV		= cursorChannel.getCount();
				
				return_Count	= "  TV : " + iCountTV;
				
				DxbLog(I, return_Count);

				cursorChannel.close();
				
				return 	return_Count;
			}

		/*	select Channel	*/
			public static void setChannel(Cursor mChannelsCursor)
			{
				DxbLog(I, "setChannel()");
				
				if(		(mChannelsCursor != null)
					&&	(mChannel != null)
					&&	(mPlayer != null)
				)
				{
					Cursor c = mChannelsCursor;
					mChannel.ensembleName	= c.getString(1);
					mChannel.ensembleID		= c.getInt(2);
					mChannel.ensembleFreq	= c.getInt(3);
					mChannel.serviceName	= c.getString(4);
					mChannel.serviceID		= c.getInt(5);
					mChannel.channelName	= c.getString(6);
					mChannel.channelID		= c.getInt(7);
					mChannel.type			= c.getInt(8);
					mChannel.bitrate		= c.getInt(9);
					
					for(int regIndex=0; regIndex<7; regIndex++)
					{
						mChannel.reg[regIndex] = c.getInt(regIndex+10);
					}
					
					DxbPlayer.start();
					mPlayer.setChannelIndex(c.getInt(0), c.getInt(8));
				}
			}
			
		/*	Record & Capture	*/
			public static int setCapture(String name)
			{
				int	return_state	= 1;
				
				if(mPlayer != null)
				{
					return_state = mPlayer.setCapture(name);
				}
				DxbLog(I, "setCapture(name="+name+"(state="+return_state+")");
				
				return return_state;
			}
			
			public static void setRecord(String name)
			{
				DxbLog(I, "setRecord()");
				if(mPlayer != null)
				{
					mPlayer.setRecord(name);
				}
			}
				
				public static void setRecStop()
				{
					DxbLog(I, "setRecStop()");
					if(mPlayer != null)
					{		
						mPlayer.setRecStop();
					}
				}
	/************************************************************************************************************************
	 * 	device access -----> End
	 ************************************************************************************************************************/

		
	/************************************************************************************************************************
	 * 	DB control -----> Start
	 ************************************************************************************************************************/
		/*	Service	*/
			public static String getServiceName()
			{
				if(mChannel != null && mChannel.serviceName != null)
				{
					return mChannel.serviceName;
				}
				return null;
			}
			
			public static Cursor getChannels(int iTab)
			{
				//DxbLog(I, "getChannels(iTab="+iTab+")");
				int	service_type;
				
				if(iTab == 0)
				{
					service_type	= TYPE_TDMB;
				}
				else
				{
					service_type	= TYPE_DAB;
					//service_type	= TYPE_DABP;
				}
				
				return mChannelManager.getTypeChannels(service_type);
				//return mChannelManager.getAllChannels();
			}
			
			public static String getCurrent_ServiceName()
			{
				DxbLog(I, "getCurrent_ServiceName()");
				return ChannelManager.KEY_SERVICE_NAME;
			}
			
			public static String getCurrent_ServiceType()
			{
				DxbLog(I, "getCurrent_ServiceType()");
				return ChannelManager.KEY_TYPE;
			}
			
		/*	EPG	*/
			public static boolean openEPGDB()
			{
				return false;
			}
			
			public static Cursor getEPG_Schedule()
			{
				return null;
			}
	/************************************************************************************************************************
	 * 	DB control -----> End
	 ************************************************************************************************************************/

			
	/************************************************************************************************************************
	 * 	Signal -----> Start
	 ************************************************************************************************************************/
		public static int getSignalStrength()
		{
			//DxbLog(I, "getSignalStrength()");
			
			int[] dSignal = mPlayer.getSignalStrength();
		/*
			dSignal[0] : Signal Strength
			dSignal[1] : SNR
			dSignal[2] : PCBer
			dSignal[3] : RSSI
		
			dSignal[4] : ViterBer
			dSignal[5] : FICBer
			dSignal[6] : MSCBer
			dSignal[7] : TSPer
		
			dSignal[8] : RFLoopGain
			dSignal[9] : BBLoopGain
		
			dSignal[10] : OFDM_Lock
		
			dSignal[11] : DSPVer
			dSignal[12] : DSPBuildDate
			dSignal[13] : DDVer
		
			for(int i=0; i<14; i++)
			{
				if(i==11)
				{
					DxbLog(I, "#" + i + " =====> DSP Ver (" + ((dSignal[i]>>24)&0xFF) + "." + ((dSignal[i]>>16)&0xFF) + "." + ((dSignal[i]>>8)&0xFF) + ")");
				}
				else if(i==12)
				{
					DxbLog(I, "#" + i + " =====> BuildDate " + ((dSignal[i]>>20)&0xFFF) + "y-" + ((dSignal[i]>>16)&0xF) +  "m-" + ((dSignal[i]>>8)&0xFF) + "d " + ((dSignal[i])&0xFF) + "h");
				}
				else if(i==13)
				{
					DxbLog(I, "#" + i + " =====> DD Ver (" + ((dSignal[i]>>16)&0xFF) + "." + ((dSignal[i]>>8)&0xFF) + "." + ((dSignal[i]>>0)&0xFF) + ")");
				}
				else
				{
					DxbLog(I, "#" + i + " =====> " + dSignal[i]);
				}
			}
		*/
			if(dSignal[0] > 4)
			{
				dSignal[0] = 4;
			}
			
			return dSignal[0];
		}
		
		static void monitoringSignal(int iLevel)
		{
		}
	/************************************************************************************************************************
	 * 	Signal -----> End
	 ************************************************************************************************************************/

			
	/************************************************************************************************************************
	 * 	Service -----> Start
	 ************************************************************************************************************************/
		private static int	iIndex_serviceType, iIndex_serviceName, iIndex_remoconID, iIndex_pmtPID;

		public static void setService_ColumnIndex(Cursor cursor)
		{
			DxbLog(I, "getColumnIndices()");
			if (cursor != null)
			{
				iIndex_serviceName	= cursor.getColumnIndexOrThrow(ChannelManager.KEY_SERVICE_NAME);
			}
		}

		public static void bindView(Cursor cursor, ViewHolder vh)
		{
			//DxbLog(I, "bindView()");
			String	serviceName	= cursor.getString(iIndex_serviceName);
			
			int	position	= cursor.getPosition();
			if(		(		DxbView_List.getTab() == 0 
					&&	(gInformation.cCOMM.iCurrent_TV != position)
				)
			||	(		DxbView_List.getTab() == 1 
					&&	(gInformation.cCOMM.iCurrent_Radio != position)
				)
			)
			{
				vh.name.setTextColor(Color.rgb(204, 204, 204));
			}
			else
			{
				vh.name.setTextColor(Color.rgb(248, 196, 0));
			}
			
			if(DxbView_List.getTab() == 0)
			{
				vh.icon_text.setBackgroundResource(R.drawable.dxb_portable_list_tv_icon_n);
			}
			else
			{
				vh.icon_text.setBackgroundResource(R.drawable.dxb_portable_list_radio_icon_n);
			}
			
			vh.name.setText(serviceName);
		}
	/************************************************************************************************************************
	 * 	Service -----> End
	 ************************************************************************************************************************/

		
	/************************************************************************************************************************
	 * 	setting -----> Start
	 ************************************************************************************************************************/
		@SuppressWarnings("rawtypes")
		public static Class getClass_Option()
		{
			DxbLog(I, "getClass_Option()");
			return DxbOptions.class;
		}
		
		static void setOptionValue()
		{
			DxbLog(I, "setOptionValue()");
			
			DxbOptions.setResumeValue();
		}
		
	/************************************************************************************************************************
	 * 	setting -----> End
	 ************************************************************************************************************************/

		
	/************************************************************************************************************************
	 *	(DVBT Only) teletext, RadioImage, EPG -----> Start
	 ************************************************************************************************************************/
		static void initView()
		{
		}
		
		static boolean isRadio()
		{
			if(DxbView_List.getTab() == 1)
				return true;
			return false;
		}
		
		static void updateList_EPG()
		{
		}
		
		static boolean onKeyDown_EPG(int keyCode, KeyEvent event)
		{
			return false;
		}
		
		static boolean onKeyDown_Teletext(int keyCode, KeyEvent event)
		{
			return false;
		}
		
		static boolean isShown_Teletext()
		{
			return false;
		}
		
		static boolean isValidate_Teletext()
		{
			return false;
		}
		
		static void setState_Teletext(boolean state)
		{
			//DxbView_Teletext.setState(state);
		}
		
		static int playTeletext(boolean state)
		{
			return 0;
		}

		static void stopTeletext()
		{
		}
		
		static void playSubtitle(int state)
		{
		}
	/************************************************************************************************************************
	 *	(DVBT Only) teletext, RadioImage, EPG -----> End
	 ************************************************************************************************************************/
				
				
	/************************************************************************************************************************
	 * 	(ISDBT Only) ParentalRating, Preset, AreaCode, monitoringSignal, Section -----> Start
	 ************************************************************************************************************************/
		public static void setDefaultValue()	// set section
		{
		}
		
		public static boolean isVisible_AreaCode()
		{
			return false;
		}
		
		public static boolean isVisible_ParentalRating()
		{
			return false;
		}

		public static int getRid_Indicator_Section()
		{
			return -1;
		}
		
		public static void setPresetOpt(int preset)
		{
		}
		
		public static void setArea(int areaCode)
		{
		}
		
		public static void setAudio(int iIndex)
		{
		}

		public static void setAudioDualMono(int iIndex)
		{
		}	

		public static void setParentalRate(int age)
		{
		}

		public static int reqSCInfo (int arg)
		{
			return 0;
		}
		
		static void changeServiceList()
		{
		}
		
		static DialogInterface.OnClickListener ListenerOnClick_selectAreaCode	= new DialogInterface.OnClickListener()
		{
			public void onClick(DialogInterface dialog, int item)
			{
			}
		};
		
		static DialogInterface.OnCancelListener ListenerOnCancel_selectAreaCode	= new DialogInterface.OnCancelListener()
		{
			public void onCancel(DialogInterface dialog)
			{
			}
		};
	/************************************************************************************************************************
	 * 	(ISDBT Only) ParentalRating, Preset, AreaCode, monitoringSignal, Section -----> End
	 ************************************************************************************************************************/
		
		
	private static void DxbLog(int level, String mString)
	{
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
