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

import java.util.Calendar;

import android.content.Context;
import android.media.MediaScannerConnection;
import android.media.MediaScannerConnection.MediaScannerConnectionClient;
import android.net.Uri;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import com.telechips.android.isdbt.player.DxbPlayer;
import com.telechips.android.isdbt.schedule.Alarm;
import com.telechips.android.isdbt.util.DxbStorage;
import com.telechips.android.isdbt.util.DxbUtil;

public class DxbView_Full_Record extends DxbView {

	private View		mView;
	private ImageView	mivIcon;
	private TextView	mtvIcon;
	private Button		mbuStop;

	private static final int START	= 0;
	private static final int UPDATE	= 1;

	public static enum TYPE {
		CAPTURE,
		MOVIE,
		ALARM,
		TIME_SHIFT,
		NULL
	}
	private TYPE eRecType = TYPE.MOVIE;

	private boolean mIsFileSaving = false;

	private int	miID_Alarm;
	private int	miType_repeat;
	
	// Capture & Record
	private String mScanFileMimeType;
	private String mScanFilePath;
	private String sFullPath_recording;
	private String sFileName_recording;

	// Record
	private boolean mIsScanning = false;
	private int	/*miRecHH,*/ miRecMM, miRecSS;

	private static final int[] recIcon = {	R.drawable.dxb_portable_recording_icon_01,
											R.drawable.dxb_portable_recording_icon_02};
	private int	muRecAni;

	public DxbView_Full_Record(Context context, View v) {
		super(context);
		mView = v;

		setComponent();
		mMediaScannerConnection = new MediaScannerConnection(getContext(), mMediaScannerConnectionClient);
		getPlayer().setOnRecordingCompletionListener(ListenerOnRecordingCompletion);
	}

	private void setComponent() {
		mivIcon	= (ImageView)mView.findViewById(R.id.full_rec_icon);
		mtvIcon	= (TextView)mView.findViewById(R.id.full_rec_text);
		mbuStop	= (Button)mView.findViewById(R.id.full_rec_button);
		
		mtvIcon.setTextSize(getTextSize(15));
		mbuStop.setTextSize(getTextSize(18));
	}

	public String getFileName() {
		return sFileName_recording;
	}
	
	public TYPE getRecordType() {
		return eRecType;
	}
	
	public void setCapture()
	{
		DxbLog(I, "setCapture");
		
		if(getPlayer().getChannelCount() <= 0)
		{
			updateToast(getResources().getString(R.string.no_channel));
			return;
		}
		
		if(getSignalLevel() < 0)
		{
			updateToast(getResources().getString(R.string.weak_signal));
			return;
		}
		
		if(		getPlayer().isParentalLock()
			&&	!getPlayer().isUnlockParental()
		)
		{
			updateToast(getResources().getString(R.string.the_service_is_locked));
			return;
		}
			
		if(mIsFileSaving)
		{
			updateToast(getResources().getString(R.string.please_wait));
		}
		
		if(getPlayer().getRecordState() == DxbPlayer.REC_STATE.STOP)
		{
			eRecType = TYPE.CAPTURE;
			mIsFileSaving = true;
			Calendar CurTime = Calendar.getInstance();
			String CurTime_Str	= String.format("%02d%02d%02d%02d",
												CurTime.get(Calendar.DATE),
												CurTime.get(Calendar.HOUR),
												CurTime.get(Calendar.MINUTE),
												CurTime.get(Calendar.SECOND)
											);

			sFullPath_recording	=		DxbStorage.getPath_Device()
										+	DxbStorage.getPath_Player()
										+	CurTime_Str
										+	DxbStorage.getPath_Extension_Capture();

			sFileName_recording = CurTime_Str + ".jpg";
			DxbLog(D, "setCapture sFullPath_recording = " + sFullPath_recording);
			getPlayer().setCapture(sFullPath_recording);
		}
	}

	public boolean setRecord(TYPE recType, int iID_Alarm, int iType_repeat) {
		miID_Alarm		= iID_Alarm;
		miType_repeat	= iType_repeat;
		return setRecord(recType);
	}

	public boolean setRecord(TYPE recType)
	{
		DxbLog(I, "setRecord");
		//		/	 "	 `	;  \   (  )  < > ? *

		char[] oldChar;
		oldChar = new char[]{' ', '/', '"', '`', ';', '\\', '(', ')', '<', '>', '?', '*'}; 
		char newChar = '_';
		String strServiceName = getPlayer().getServiceName_addHyphen();
		for(int i=0; i<oldChar.length; i++)
		{
			strServiceName = strServiceName.replace(oldChar[i], newChar);
		}
		
		if (getPlayer().getChannelCount() <= 0)
		{
			updateToast(getResources().getString(R.string.no_channel));
			return false;
		}
		
		if (getSignalLevel() < 0)
		{
			updateToast(getResources().getString(R.string.weak_signal));
			return false;
		}
		
		if(		getPlayer().isParentalLock()
			&&	!getPlayer().isUnlockParental()
		)
		{
			updateToast(getResources().getString(R.string.the_service_is_locked));
			return false;
		}
		
		if (mIsFileSaving)
		{
			updateToast(getResources().getString(R.string.please_wait));
		}

//		if(cNavi.menu[MENU_RECORD].vMenu != null )
//		{
			if(getPlayer().getRecordState() == DxbPlayer.REC_STATE.STOP)
			{
				if (getPlayer().getLocalPlayState() == DxbPlayer.LOCAL_STATE.STOP)
				{
					eRecType = recType;
					Calendar CurTime = Calendar.getInstance();
					String CurTime_Str	= DxbUtil.stringForTime(	CurTime.get(Calendar.MONTH+1),
															CurTime.get(Calendar.DATE),
															CurTime.get(Calendar.YEAR),
															CurTime.get(Calendar.HOUR_OF_DAY),
															CurTime.get(Calendar.MINUTE)
														);
					
					if (eRecType == TYPE.TIME_SHIFT)
					{
						sFullPath_recording = DxbStorage.getPath_Device() + "/ISDBT_TimeShift";
						sFileName_recording = DxbStorage.getPath_Device() + "/ISDBT_TimeShift";
					}
					else 
					{
						sFullPath_recording = DxbStorage.getPath_Device() + "/" + strServiceName + CurTime_Str;
						sFileName_recording = strServiceName + CurTime_Str;
						
						sFullPath_recording += DxbStorage.getPath_Extension_PVR();
						sFileName_recording += DxbStorage.getPath_Extension_PVR();
					}
					
					DxbLog(D, "setRecord RecordingFileName = " + sFullPath_recording);
					mbuStop.setText(getPlayer().getServiceName());
					getPlayer().setRecord(sFullPath_recording);
					
					if (eRecType != TYPE.TIME_SHIFT)
					{
						updateRecording(START);
						getMainHandler().postDelayed(mRunnable_Ani, 1000);
						mView.setVisibility(View.VISIBLE);
//						mbuStop.setFocusableInTouchMode(true);
//						mbuStop.requestFocus();
					}
					
					return true;
				}
				else
				{ // Local Playback
					updateToast(getResources().getString(R.string.please_wait));
				}
			}
			else if(getPlayer().getRecordState() == DxbPlayer.REC_STATE.RECORDING)
			{
				mView.setVisibility(View.INVISIBLE);
				getPlayer().setRecStop();
			}
			
			return false;
//		}
	}

	private DxbPlayer.OnRecordingCompletionListener ListenerOnRecordingCompletion = new DxbPlayer.OnRecordingCompletionListener() {
		public void onRecordingCompletion(DxbPlayer player, int result) {
			DxbLog(I, "###################################################");
			DxbLog(I, "onRecordingCompletion() : result=" + result);
			DxbLog(I, "###################################################");

			if ((eRecType == TYPE.MOVIE) || (eRecType == TYPE.ALARM)) {
//				if (cNavi.menu[MENU_RECORD].vMenu != null) {
					getMainHandler().removeCallbacks(mRunnable_Ani);
					mView.setVisibility(View.INVISIBLE);
//				}
			}
			switch (result) {
				case 0:
					if (eRecType != TYPE.TIME_SHIFT)
						updateToast("'" + sFileName_recording + "'" + " " + getResources().getString(R.string.saved));
				break;
				
				case 1:
					updateToast(getResources().getString(R.string.recording_error));
				break;
				
				case 2:
					updateToast("'" + DxbStorage.getPath_Device() + "'" + " " + getResources().getString(R.string.memory_full));
				break;
				
				case 3:
					updateToast(getResources().getString(R.string.file_open_error));
					eRecType	= TYPE.NULL;
				break;
				
				case 4:
					updateToast("'" + DxbStorage.getPath_Device() + "'" + " " + getResources().getString(R.string.invalid_memory));
					eRecType	= TYPE.NULL;
				break;
				
				case 5: // DxB_EVENT_ERR_INVALIDOPERATION
					updateToast(getResources().getString(R.string.failed));
					eRecType	= TYPE.NULL;
				break;

				case 6: // DxB_EVENT_FAIL_NOFREESPACE
					updateToast(getResources().getString(R.string.failed_no_free_space));
					eRecType	= TYPE.NULL;
				break;

				case 7: // DxB_EVENT_FAIL_INVALIDSTORAGE
					updateToast(getResources().getString(R.string.failed_invalid_storage));
					eRecType	= TYPE.NULL;
				break;

				default:
					updateToast(getResources().getString(R.string.failed));
					eRecType	= TYPE.NULL;
				break;
			}
			
			DxbLog(D, "eType=" + eRecType);
			if ((eRecType == TYPE.CAPTURE) && (result == 0)) {
				AddMediaDB(sFullPath_recording, "image/jpeg");
			} else if ((eRecType == TYPE.ALARM) && (result != 0)) {
				Alarm.release(getContext(), null, miID_Alarm, miType_repeat);
			}
			
			DxbLog(D, "DxbPlayer.eState=" + getPlayer().eState);
			if(getPlayer().eState == DxbPlayer.STATE.UI_PAUSE)
			{
//				if(DxbView.intentSubActivity == null)
				{
					getPlayer().releaseSurface();
					if (getPlayer().isValid())
					{
						getPlayer().stop();
						getPlayer().release();
					}
					
					getPlayer().onDestroy();
					if(mIsScanning == true)
					{
						mMediaScannerConnection.disconnect();
						mIsScanning = false;
					}
				}
				
				getPlayer().eState	= DxbPlayer.STATE.GENERAL;
			}
		}
	};

	private Runnable mRunnable_Ani = new Runnable() {
		public void run() {
			//DxbLog(I, "mRunnable_Ani --> run()");
			if(getPlayer().getRecordState() == DxbPlayer.REC_STATE.RECORDING) {
				updateRecording(UPDATE);
				getMainHandler().postDelayed(mRunnable_Ani, 1000);
			}
		}
	};
	
	private void updateRecording(int state) {
		switch(state) {
			case START:
				//miRecHH	= 0;
				miRecMM	= 0;
				miRecSS	= 0;
				muRecAni	= 0;
			break;
			
			case UPDATE:
				miRecSS++;
				if (miRecSS >= 60) {
					miRecSS = miRecSS - 60;
					miRecMM++;
				}
				if (miRecMM >= 60) {
					miRecMM = miRecMM - 60;
					//miRecHH++;
				}
				muRecAni	= (muRecAni+1) % 2;
			break;
		}
		
		mivIcon.setImageResource(recIcon[muRecAni]);
		if(muRecAni == 0)
		{
			mtvIcon.setTextColor(getResources().getColor(R.color.color_white));
		}
		else
		{
			mtvIcon.setTextColor(getResources().getColor(R.color.color_red));
		}
		//fullRec_Time.setText(miRecHH+":"+miRecMM+":"+miRecSS);
	}

	private void AddMediaDB(final String Path, final String MimeType) {
		DxbLog(I, "AddMediaDB()");
		MediaScanfile(Path, MimeType);
	}
    
	private MediaScannerConnectionClient mMediaScannerConnectionClient = new MediaScannerConnectionClient() {
		public void onMediaScannerConnected() {
			DxbLog(V, "onMediaScannerConnected(" + mScanFilePath + ", " + mScanFileMimeType + ")");
			if(mMediaScannerConnection.isConnected()) {
				mMediaScannerConnection.scanFile(mScanFilePath, mScanFileMimeType);
				mIsScanning = true;
			}
		}
		
		public void onScanCompleted(String path, Uri uri) {
			DxbLog(V, "onScanCompleted(" + path + ", " + uri.toString() + ")");
			mMediaScannerConnection.disconnect();
			mIsScanning = false;
			mIsFileSaving = false;
			if(getPlayer().eState == DxbPlayer.STATE.GENERAL) {
				getPlayer().playSubtitle(DxbPlayer._ON_);
			}
		}
	};
	
	private MediaScannerConnection mMediaScannerConnection;

	private void MediaScanfile(final String Path, final String MimeType) {
		DxbLog(V, "MediaScanfile() --> mScanFilePath = " + Path + ", mScanFileMimeType = " + MimeType);
		mScanFilePath = Path;
		mScanFileMimeType = MimeType;
		if (mIsScanning == true) {
			mMediaScannerConnection.disconnect();
			mIsScanning = false;
		}
		mMediaScannerConnection.connect();
	}
}
