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

import java.util.Calendar;

import android.media.AudioManager;
import android.media.MediaScannerConnection;
import android.media.MediaScannerConnection.MediaScannerConnectionClient;
import android.net.Uri;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;

import com.telechips.android.tdmb.player.*;
import com.telechips.android.tdmb.util.DxbStorage;

public class DxbView_Record
{
	private static final int D = 0;
	private static final int E = 1;
	private static final int I = 2;
	private static final int V = 3;
	private static final int W = 4;
	
	private static Component.cRecord	gComponent = null;
	
	public enum STATE
	{
		STOP,
		RECORDING,
		SAVE,
		NULL
	}
	static STATE	eState	= STATE.STOP;

	public enum TYPE
	{
		CAPTURE,
		MOVIE,
		NULL
	}
	private static TYPE eType	= TYPE.MOVIE;
	
	private static final int START	= 0;
	private static final int UPDATE	= 1;

	// Capture & Record
	private static String mScanFileMimeType;
	private static String mScanFilePath;
	
	private static String 	sFullPath_recording;

	private static String sFileName_recording;

	// Record
	private static boolean mIsScanning = false;
	static private int	/*recHH,*/ recMM, recSS;
	
	private static int[] recIcon = {	R.drawable.dxb_portable_recording_icon_01,
								R.drawable.dxb_portable_recording_icon_02};
	static private int	recAni;
	
	static void init()
	{
		setComponent();
		
		setListener();
		
		mMediaScannerConnection = new MediaScannerConnection(Manager_Setting.mContext, mMediaScannerConnectionClient);
	}
	
	static void setComponent()
	{
		gComponent	= Manager_Setting.g_Component.record;
		
		gComponent.vgMenu	= (ViewGroup)Manager_Setting.mContext.findViewById(R.id.full_group_rec);
		gComponent.ivIcon	= (ImageView)Manager_Setting.mContext.findViewById(R.id.full_rec_icon);
		gComponent.bStop	= (Button)Manager_Setting.mContext.findViewById(R.id.full_rec_button);
	}
	
	private static void setListener()
	{
		gComponent.bStop.setOnClickListener(ListenerOnClick);
	}
	
	static OnClickListener ListenerOnClick = new OnClickListener()
	{
		public void onClick(View v)
		{
			DxbLog(I, "onClick()");
			if(gComponent.vgMenu.isShown())
			{
				DxbView_Record.setRecord();
			}
			else if(DxbView_Record.eState == DxbView_Record.STATE.STOP)
			{
				DxbView_Full.setGroupState(true);
			}
			else
			{
				DxbLog(D, "DxbView_Record.eState = " + DxbView_Record.eState);
				DxbView_Message.updateToast(Manager_Setting.mContext.getResources().getString(R.string.please_wait));
			}
		}
	};
	
	static TDMBPlayer.OnRecordingCompletionListener ListenerOnRecordingCompletion = new TDMBPlayer.OnRecordingCompletionListener()
	{
		public void onRecordingCompletion(TDMBPlayer player, int result)
		{
			DxbLog(I, "###################################################");
			DxbLog(I, "onRecordingCompletion() : result=" + result);
			DxbLog(I, "###################################################");
			
			if(eType == TYPE.MOVIE)
			{
				if(Manager_Setting.g_Component.fullview.navi.menu.bRecord !=null)
				{
					DxbView.mHandler_Main.removeCallbacks(mRunnable_Ani);
					gComponent.vgMenu.setVisibility(View.INVISIBLE);
				}
			}
			
			switch(result)
			{
				case 0:
				case 7:
					DxbView_Message.updateToast("'" + sFileName_recording + "'" + " " + Manager_Setting.mContext.getResources().getString(R.string.saved));
				break;
				
				case 1:
					DxbView_Message.updateToast(Manager_Setting.mContext.getResources().getString(R.string.recording_error));
				break;
				
				case 2:
					DxbView_Message.updateToast("'" + DxbStorage.getPath_Device() + "'" + " " + Manager_Setting.mContext.getResources().getString(R.string.memory_full));
				break;
				
				case 3:
					DxbView_Message.updateToast(Manager_Setting.mContext.getResources().getString(R.string.file_open_error));
					eType	= TYPE.NULL;
				break;
				
				case 4:
					DxbView_Message.updateToast("'" + DxbStorage.getPath_Device() + "'" + " " + Manager_Setting.mContext.getResources().getString(R.string.invalid_memory));
					eType	= TYPE.NULL;
				break;
				
				case 5:
				default:
					DxbView_Message.updateToast(Manager_Setting.mContext.getResources().getString(R.string.failed));
					eType	= TYPE.NULL;
				break;
			}
			
			if(eType == TYPE.MOVIE)
			{
				if(result != 3)
				{
					if(DxbView_List.getTab()==0)
						AddMediaDB(sFullPath_recording, "video/ts");
					else
						AddMediaDB(sFullPath_recording, "audio/mp2");
					
					DxbView.mHandler_Main.postDelayed(mRunnable_Recording, 5000);
				}
				else
				{
					DxbView_Record.eState = DxbView_Record.STATE.STOP;
					eState = STATE.STOP;
				}
			}
			else if(eType == TYPE.CAPTURE)
			{
				if(result == 0)
				{
					AddMediaDB(sFullPath_recording, "image/jpeg");
					DxbView.mHandler_Main.postDelayed(mRunnable_Recording, 5000);
				}
				else
				{
					DxbView_Record.eState = DxbView_Record.STATE.STOP;
					eState = STATE.STOP;
				}
			}
			else
			{
				DxbView_Record.eState = DxbView_Record.STATE.STOP;
			}
			
			if(DxbPlayer.eState == DxbPlayer.STATE.UI_PAUSE)
			{
				if(DxbView.intentSubActivity == null)
				{
					DxbPlayer.releaseSurface();
					if(DxbPlayer.mPlayer != null)
					{
						DxbPlayer.mPlayer.stop();
						DxbPlayer.mPlayer.release();
						
						DxbPlayer.mPlayer	= null;
						
						DxbView.mAudioManager.setStreamVolume(AudioManager.STREAM_MUSIC, DxbView.mAudioCurrentVolume, 0);
					}
					
					if(DxbPlayer.mChannelManager != null)
					{
						DxbPlayer.mChannelManager.close();
						DxbPlayer.mChannelManager = null;
					}
					
					if(DxbView.mDB != null)
					{
						DxbView.mDB.close();
						DxbView.mDB = null;
					}
					
					if(mIsScanning == true)
					{
						mMediaScannerConnection.disconnect();
						mIsScanning = false;
					}
				}
				
				DxbPlayer.eState	= DxbPlayer.STATE.GENERAL;
			}
		}
	};

	static void setCapture()
	{
		DxbLog(I, "setCapture");
		
		if(Manager_Setting.g_Information.cCOMM.iCount_Current <= 0)
		{
			DxbView_Message.updateToast(Manager_Setting.mContext.getResources().getString(R.string.no_channel));
			return;
		}
		
		if(Manager_Setting.g_Information.cINDI.iSignal_Level < 0)
		{
			DxbView_Message.updateToast(Manager_Setting.mContext.getResources().getString(R.string.weak_signal));
			return;
		}

		if(eState == STATE.STOP)
		{
			DxbView.mHandler_Main.removeCallbacks(mRunnable_Recording);
			
			eType = TYPE.CAPTURE;
			eState = STATE.SAVE;

			Calendar CurTime = Calendar.getInstance();
			String CurTime_Str	= String.format("%02d%02d%02d%02d",
												CurTime.get(Calendar.DATE),
												CurTime.get(Calendar.HOUR),
												CurTime.get(Calendar.MINUTE),
												CurTime.get(Calendar.SECOND)
											);
			
			sFullPath_recording	=		DxbStorage.getPath_Device()
										+	DxbStorage.getPath_Player(DxbView_List.getTab())
										+	CurTime_Str
										+	DxbStorage.getPath_Extension_Capture();

			sFileName_recording = CurTime_Str + ".jpg";
			DxbLog(D, "setCapture sFullPath_recording = " + sFullPath_recording);
			DxbPlayer.setCapture(sFullPath_recording);
		}
		else
		{
			DxbView_Message.updateToast(Manager_Setting.mContext.getResources().getString(R.string.please_wait));
		}
	}

	static void setRecord()
	{
		DxbLog(I, "setRecord");
	
		if(Manager_Setting.g_Information.cCOMM.iCount_Current <= 0)
		{
			DxbView_Message.updateToast(Manager_Setting.mContext.getResources().getString(R.string.no_channel));
			return;
		}
		
		if(Manager_Setting.g_Information.cINDI.iSignal_Level < 0)
		{
			DxbView_Message.updateToast(Manager_Setting.mContext.getResources().getString(R.string.weak_signal));
			return;
		}
	
		if(Manager_Setting.g_Component.fullview.navi.menu.bRecord != null )
		{
			if(eState == STATE.STOP)
			{
				int iTab = DxbView_List.getTab();
				DxbView.mHandler_Main.removeCallbacks(mRunnable_Recording);
				
				eType = TYPE.MOVIE;
				eState = STATE.RECORDING;

				Calendar CurTime = Calendar.getInstance();
				String CurTime_Str	= String.format(	"%02d%02d%02d%02d",
														CurTime.get(Calendar.DATE),
														CurTime.get(Calendar.HOUR),
														CurTime.get(Calendar.MINUTE),
														CurTime.get(Calendar.SECOND)
													);
			
				sFullPath_recording =		DxbStorage.getPath_Device()
										+	DxbStorage.getPath_Player(iTab)
										+	CurTime_Str
										+	DxbStorage.getPath_Extension_Record(iTab);
				sFileName_recording = CurTime_Str + DxbStorage.getPath_Extension_Record(iTab);

				DxbLog(D, "setRecord RecordingFileName = " + sFullPath_recording);
				DxbPlayer.setRecord(sFullPath_recording);
				
				updateRecording(START);
				
				DxbView.mHandler_Main.postDelayed(mRunnable_Ani, 1000);
				
				DxbView_Full.setGroupState(false);
				
				gComponent.vgMenu.setVisibility(View.VISIBLE);
				gComponent.bStop.setFocusableInTouchMode(true);
				gComponent.bStop.requestFocus();
			}
			else if(eState == STATE.RECORDING)
			{
				gComponent.vgMenu.setVisibility(View.INVISIBLE);
				eState = STATE.SAVE;

				DxbPlayer.setRecStop();
			}
			else	// STATE.SAVE
			{
				DxbView_Message.updateToast(Manager_Setting.mContext.getResources().getString(R.string.please_wait));
			}
		}
	}	
	
	private static Runnable mRunnable_Recording = new Runnable()
	{
		public void run()
		{
			if(eState != STATE.STOP)
			{
				eState = STATE.STOP;
			}
		}
	};
	
	private static Runnable mRunnable_Ani = new Runnable()
	{
		public void run()
		{
			//DxbLog(I, "mRunnable_Ani --> run()");
			
			if(		(DxbView.eState == DxbView.STATE.FULL)
				&&	(eState == STATE.RECORDING)
			)
			{
				updateRecording(UPDATE);
				DxbView.mHandler_Main.postDelayed(mRunnable_Ani, 1000);
			}
		}
	};
	
	private static void updateRecording(int state)
	{
		switch(state)
		{
			case START:
				//recHH	= 0;
				recMM	= 0;
				recSS	= 0;
				recAni	= 0;
			break;
			
			case UPDATE:
				recSS++;
				if(recSS >= 60)
				{
					recSS = recSS - 60;
					recMM++;
				}
				
				if(recMM >= 60)
				{
					recMM = recMM - 60;
					//recHH++;
				}
				
				recAni	= (recAni+1) % 2;
			break;
		}
		
		gComponent.ivIcon.setImageResource(recIcon[recAni]);
		//fullRec_Time.setText(recHH+":"+recMM+":"+recSS);
	}
	
	static void setFocus_bStop()
	{
		gComponent.bStop.setFocusableInTouchMode(true);
		gComponent.bStop.requestFocus();		
	}
	
	static void disconnect()
	{
		DxbLog(I, "disconnect()");
	
		if(mIsScanning == true)
		{
			mMediaScannerConnection.disconnect();
			mIsScanning = false;
		}
	}
	private static void AddMediaDB(final String Path, final String MimeType)
	{
		DxbLog(I, "AddMediaDB()");
		MediaScanfile(Path, MimeType);
	}
    
	private static MediaScannerConnectionClient mMediaScannerConnectionClient = new MediaScannerConnectionClient()
	{
		public void onMediaScannerConnected()
		{
			DxbLog(V, "onMediaScannerConnected(" + mScanFilePath + ", " + mScanFileMimeType + ")");

			if(mMediaScannerConnection.isConnected())
			{
				mMediaScannerConnection.scanFile(mScanFilePath, mScanFileMimeType);
				mIsScanning = true;
			}
		}
		
		public void onScanCompleted(String path, Uri uri)
		{
			DxbLog(V, "onScanCompleted(" + path + ", " + uri.toString() + ")");
			
			mMediaScannerConnection.disconnect();
			mIsScanning = false;
			
			DxbView.mHandler_Main.removeCallbacks(mRunnable_Recording);
			DxbView_Record.eState = DxbView_Record.STATE.STOP;
			
			if(		(DxbView.eState == DxbView.STATE.FULL)
				&&	(DxbPlayer.eState == DxbPlayer.STATE.GENERAL)
			)
			{
				DxbPlayer.playSubtitle(Manager_Setting.g_Information.cOption.subtitle);
			}
		}
	};
	
	private static MediaScannerConnection mMediaScannerConnection;
	
	private static void MediaScanfile(final String Path, final String MimeType)
	{
		mScanFilePath = Path;
		mScanFileMimeType = MimeType;
		DxbLog(V, "MediaScanfile() --> mScanFilePath = " + mScanFilePath + ", mScanFileMimeType = " + mScanFileMimeType);
		{
			if(mIsScanning == true)
			{
				mMediaScannerConnection.disconnect();
				mIsScanning = false;
			}
			mMediaScannerConnection.connect();
		}
	}
    
	private static void DxbLog(int level, String mString)
	{
		if(DxbView.TAG == null)
		{
			return;
		}
		
		String TAG = "[[[DxbView_Record]]] ";
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
