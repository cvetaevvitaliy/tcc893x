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

import java.util.LinkedList;
import java.util.Queue;

import com.telechips.android.isdbt.player.DxbPlayer;
import com.telechips.android.isdbt.player.DxbPlayer.LOCAL_STATE;
import com.telechips.android.isdbt.player.isdbt.EWSData;

import android.app.Dialog;
import android.content.Context;
import android.content.res.Resources;
import android.media.AudioManager;
import android.os.Handler;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.widget.TextView;

public class DxbView {

	//	Dialog
	protected static final int DIALOG_SCAN = 0;
	protected static final int DIALOG_FAVOURITE_LIST = 1;
	protected static final int DIALOG_MANUAL_SCAN = 2;

	// ViewgetCurChannelType
	public static final int VIEW_NULL     = 0;
	public static final int VIEW_LIST     = 1;
	public static final int VIEW_FULL     = 2;
	public static final int VIEW_EPG      = 3;
	public static final int VIEW_TELETEXT = 4;
	public static final int VIEW_OPTION   = 5;
	public static final int VIEW_SCHEDULE = 6;
	public static final int VIEW_MAIN     = VIEW_FULL;

	// coordinates
	protected static final int COORDINATES_X = 0x00;
	protected static final int COORDINATES_Y = 0x01;	

	private final ISDBTPlayerActivity mContext;

	protected static final int NORMAL_SPEED = 5;
	private int miPlaySpeed = NORMAL_SPEED;

	//	Logical channel - remocon control
	int			iInputChannel	= -1;
	TextView	tvInputChannel	= null;


	public DxbView(Context context) {
		mContext = (ISDBTPlayerActivity)context;
	}

	protected ISDBTPlayerActivity getContext() {
		return mContext;
	}

	public void setVisible(boolean v) {
		
	}

	protected void setState(boolean isRefresh, int iChange_view)
	{
		setVisible(false);
		getContext().setState(isRefresh, iChange_view);
	}

	protected void setScan() {
		getPlayer().eState = DxbPlayer.STATE.OPTION_FULL_SCAN;
		getContext().setState(true, VIEW_NULL);
	}

	protected void setAutoSearch (int Direction) {
		if (Direction==0)
			getPlayer().eState = DxbPlayer.STATE.OPTION_AUTOSEARCH_DN;
		else
			getPlayer().eState = DxbPlayer.STATE.OPTION_AUTOSEARCH_UP;
		getContext().setState(true, VIEW_FULL);
	}
	protected DxbPlayer getPlayer() {
		return getContext().getPlayer();
	}

	protected Handler getMainHandler() {
		return mContext.mHandler_Main;
	}

	protected int getDisplayWidth() {
		return getContext().cCOMM.iDisplayWidth;
	}

	protected int getDisplayHeight() {
		return getContext().cCOMM.iDisplayHeight;
	}

	protected float getDisplayDensity() {
		return getContext().cCOMM.fDensity;
	}
	
	protected float getTextSize(int iSize)
	{
		return	(iSize * getDisplayWidth() / getDisplayDensity() / 800);
	}

	protected boolean isSTB()
	{
		if(getContext().cCOMM.iDisplayWidth > 1024)
		{
			return true;
		}
		
		else return false;
	}

	protected boolean isRecord()
	{
		return getPlayer().isRecord();
	}

	protected boolean isBookRecord()
	{
		return getPlayer().isBookRecord();
	}

	protected boolean isTimeShift()
	{
		return getPlayer().isTimeShift();
	}

	protected void setIndicatorVisible(boolean v) {
		getContext().setIndicatorVisible(v);
	}

	protected int getSignalLevel() {
		return getContext().getSignalLevel();
	}

	protected int getSCStatus() {
		return getContext().getSCStatus();
	}
	
	protected DxbRemote getRemote() {
		return new DxbRemote(getContext());
	}

	protected Resources getResources() {
		return getContext().getResources();
	}

	protected void onUpdateScreen() {
		
	}

	protected void onUpdateEPG_PF()	{
		DxbLog(I, "onUpdateEPG_PF()");
	}

	protected void volumeDown() {
		AudioManager audioManager = (AudioManager)getContext().getSystemService(Context.AUDIO_SERVICE);
		audioManager.adjustStreamVolume(AudioManager.STREAM_MUSIC, AudioManager.ADJUST_LOWER, 1);
	}

	protected void volumeUp() {
		AudioManager audioManager = (AudioManager)getContext().getSystemService(Context.AUDIO_SERVICE);
		audioManager.adjustStreamVolume(AudioManager.STREAM_MUSIC, AudioManager.ADJUST_RAISE, 1);
	}

	protected int getToPosition(int co, int position) {
		DxbLog(I, "getToPosition(co="+co + ", position=" + position + ")");
		
		int	return_position	= 0;
		if (co == COORDINATES_X) {
			return_position	= position * getContext().cCOMM.iDisplayWidth / 800;
		} else if(co == COORDINATES_Y) {
			return_position	= position * getContext().cCOMM.iDisplayHeight / 480;
		}
		return return_position;
	}
	
	protected void updateToast(String message) {
		getContext().updateToast(message);
	}
	
	protected void updateSignalQualityToast(String message) {
		getContext().updateSignalQualityToast(message);
	}
	
	public int getServiceType() {
		return	getPlayer().getServiceType();
	}

	public void setServiceType(int i) {
	    getPlayer().setServiceType(i);
	}
	
	public void setChannel()
	{
		if (getPlayer().getLocalPlayState() == LOCAL_STATE.STOP || getPlayer().getServiceType() == 1)
		{
			getPlayer().resetChannel();
			getPlayer().setChannel();
		}
		else
		{
			updateToast(getContext().getResources().getString(R.string.now_playing));
		}
	}

	public void setChannel(int iCh)
	{
		if (getPlayer().getLocalPlayState() == LOCAL_STATE.STOP || getPlayer().getServiceType() == 1)
		{
			getPlayer().setChannel(iCh);
		}
		else
		{
			updateToast(getContext().getResources().getString(R.string.now_playing));
		}
	}

	public void setChannel(int iType, int iCh)
	{
		if (getPlayer().getLocalPlayState() == LOCAL_STATE.STOP || getPlayer().getServiceType() == 1)
		{
			setServiceType(iType);
			setChannel(iCh);
		}
		else
		{
			updateToast(getContext().getResources().getString(R.string.now_playing));
		}
	}
	
	public void setBackScanChannel()
	{
		if (getPlayer().getLocalPlayState() == LOCAL_STATE.STOP || getPlayer().getServiceType() == 1)
		{
			getPlayer().setBackScanChannel();
		}
	}
	
	public void setAutoSearchChannel (int fullseg_row, int oneseg_row)
	{
		if (getPlayer().getLocalPlayState() == LOCAL_STATE.STOP || getPlayer().getServiceType() == 1) {
			getPlayer().setAutoSearchChannel(fullseg_row, oneseg_row);
		}
	}

	public int getChannelPosition()
	{
		return getPlayer().getChannelPosition();
	}

	protected void setChannelUp()
	{
		if (getPlayer().getLocalPlayState() == LOCAL_STATE.STOP || getPlayer().getServiceType() == 1)
		{
			getPlayer().setChannelUp();
		}
		else
		{
			updateToast(getContext().getResources().getString(R.string.now_playing));
		}
	}

	protected void setChannelDown()
	{
		if (getPlayer().getLocalPlayState() == LOCAL_STATE.STOP || getPlayer().getServiceType() == 1)
		{
			getPlayer().setChannelDown();
		}
		else
		{
			updateToast(getContext().getResources().getString(R.string.now_playing));
		}
	}

	public boolean onGenericMotionEvent(MotionEvent event) {
		return false;
	}

	public boolean onKeyDown(int keyCode, KeyEvent event) {
		return false;
	}

	protected boolean press_InputChannel(int keyCode)
	{
		DxbLog(I, "press_InputChannel(keyCode=" + keyCode + ")");
		
		if(tvInputChannel == null)
		{
			tvInputChannel	= (TextView)getContext().findViewById(R.id.input_channel);
			if (getDisplayWidth() > 800)
			{
				tvInputChannel.setTextSize(30 / getDisplayDensity());
			}
			else
			{
				tvInputChannel.setTextSize(20 / getDisplayDensity());
			}
		}
		
		if(		(keyCode >= KeyEvent.KEYCODE_0)
			&&	(keyCode <= KeyEvent.KEYCODE_9)
		)
		{
			getMainHandler().removeCallbacks(mRunnable_InputChannel);
			update_InputChannel(keyCode);
			tvInputChannel.requestFocus();
			
			return true;
		}
		else if(keyCode == KeyEvent.KEYCODE_ENTER)
		{
			if(confirm_InputChannel())
			{
				return true;
			}
		}
		else if(keyCode == KeyEvent.KEYCODE_BACK)
		{
			if(back_InputChannel())
			{
				return true;
			}
		}
		
		return	false;
	}
	
	
	private void update_InputChannel(int keyCode)
	{
		DxbLog(I, "updateInputChannel(" + keyCode + ")");
		DxbLog(D, "iInputChannel = " + iInputChannel);
		
		if(iInputChannel < 0)
		{
			tvInputChannel.setVisibility(View.VISIBLE);
			iInputChannel	= keyCode - KeyEvent.KEYCODE_0;
			
			tvInputChannel.setText(String.format("%03d", iInputChannel));
			getMainHandler().postDelayed(mRunnable_InputChannel, 3000);
		}
		else if(iInputChannel < 100)
		{
			iInputChannel	= iInputChannel * 10 + (keyCode - KeyEvent.KEYCODE_0);
			tvInputChannel.setText(String.format("%03d", iInputChannel));
			
			if(iInputChannel > 99)
			{
				getPlayer().changeChannle_LogicalChannel(iInputChannel);
//				changeChannel_InputChannel(iInputChannel);
				
				tvInputChannel.setVisibility(View.GONE);
				iInputChannel	= -1;
			}
			else
			{
				getMainHandler().postDelayed(mRunnable_InputChannel, 3000);
			}
		}
	}
	
	private boolean confirm_InputChannel()
	{
		DxbLog(I, "confirm_InputChannel()");
		
		int	iChannel	= iInputChannel;

		tvInputChannel.setVisibility(View.GONE);
		iInputChannel	= -1;
		
		if(		(iChannel > 0)
			&&	(iChannel < 10000)
		)
		{
			getPlayer().changeChannle_LogicalChannel(iChannel);
//			changeChannel_InputChannel(iChannel);
			
			return true;
		}
		
		return false;
	}
	
	private boolean back_InputChannel()
	{
		if(iInputChannel >= 0)
		{
			tvInputChannel.setVisibility(View.GONE);
			iInputChannel	= -1;
			
			return true;
		}
		
		return false;
	}
	
/*	private void changeChannel_InputChannel(int iInputChannel)
	{
		ArrayList<DxbChannelData>	tempList;
		int	iCount;

		if(Manager_Setting.g_Information.cCURRENT.iTab == 1)
		{
			tempList	= DxbView_EPG.gInformation2.radioChannelData;
			iCount		= Manager_Setting.g_Information.cCOMM.iCount_Radio;
		}
		else
		{
			tempList	= DxbView_EPG.gInformation2.tvChannelData;
			iCount		= Manager_Setting.g_Information.cCOMM.iCount_TV;
		}
		
		for(int i=0 ; i<iCount ; i++)
		{
			if(tempList.get(i).iChannelNumber == iInputChannel)
			{
				changeChannel(Manager_Setting.g_Information.cCURRENT.iTab, i);
				
				if(eState == STATE.FULL)
				{
					DxbView_Full.setTitleState(true);
				}
				
				return;
			}
		}
	}*/
	
	private Runnable mRunnable_InputChannel = new Runnable()
	{
		public void run()
		{
			int	iChannel	= iInputChannel;

			tvInputChannel.setVisibility(View.GONE);
			iInputChannel	= -1;
			
			if(		(iChannel > 0)
				&&	(iChannel < 10000)
			)
			{
				getPlayer().changeChannle_LogicalChannel(iChannel);
//				changeChannel_InputChannel(iChannel);
			}
		}
	};

	public void onChannelChange() {

	}

	public void onAreaChange() {
		
	}
	
	protected Dialog onCreateDialog(int id) {
		return null; 
	}
	
	protected String getClassName() {
		return null;
	}
	
	protected void send_Event(MSG_VIEW msg)
	{
	}
	
	public void onDestroy()
	{
		thread_Stop();
	}
	
	protected int getPlaySpeed()
	{
		return miPlaySpeed;
	}
	
	protected void setPlaySpeed(int iSpeed)
	{
		miPlaySpeed	= iSpeed;
	}
	
	protected void PlayingCompletion(DxbPlayer player, int type, int state)
	{
	}
	
	protected void FilePlayTimeUpdate(DxbPlayer player, int time)
	{
	}
	
	protected void VideoOutput(DxbPlayer player)
	{
	}
		
	protected void onChannelUpdate(DxbPlayer player, int request)
	{
	}

	protected void HandoverChannel(DxbPlayer player, int affiliation, int channel)
	{
	}
	
	protected void EWS(DxbPlayer player, int status, EWSData obj)
	{
	}
	
	protected void SCErrorUpdate(DxbPlayer player, int state)
	{
	}
	
	protected void EPGUpdate(DxbPlayer player, int channelNumber, int serviceID)
	{
	}

/*****************************************************************************************************************************************************************************
 *	Thread - DxbView
 *****************************************************************************************************************************************************************************/
	protected enum EVENT
	{
		SCAN_PERCENT,
		NULL
	}
	
	class MSG_VIEW
	{
		EVENT	event;
		int		iVal;
		byte	bVal[];
	}
	
//	private Thread	t;
	private Queue<MSG_VIEW>msgQueue	= new LinkedList<MSG_VIEW>();
	
	protected void thread_Start()
	{
		DxbLog(I, "thread_Start()");
		
/*		Thread_View	r	= new Thread_View();
		getContext().t	= new Thread(r);
		getContext().t.start();*/
	}
	
	protected void thread_Stop()
	{
		/*		if(		(getContext().t != null)
			&&	(getContext().t.isAlive())
		)
		{
			getContext().t.interrupt();
		}*/
	}
	
	protected int thread_sendMSG(EVENT event, int iVal, byte[] bVal)
	{
		MSG_VIEW	msg	= new MSG_VIEW();
		msg.event	= event;
		msg.iVal	= iVal;
		msg.bVal	= bVal;
		
		msgQueue.offer(msg);
		
		return 0;
	}
	
	protected class Thread_View implements Runnable
	{
		public void run()
		{
			try
			{
				while(!Thread.currentThread().isInterrupted())
				{
					if(msgQueue.peek() != null)
					{
						MSG_VIEW msg	= (MSG_VIEW)msgQueue.poll();
						if(msg.event == EVENT.SCAN_PERCENT)
						{
						}
						else
						{
							send_Event(msg);
						}
					}
					else
					{
					}
					
					Thread.sleep(100);
				}
			}
			catch(InterruptedException e)
			{
				e.printStackTrace();
			}
			finally
			{
				DxbLog(I, "Thread is dead..");
			}
		}
	}


/*****************************************************************************************************************************************************************************
 *	Debug - log_message
 *****************************************************************************************************************************************************************************/

	protected static final int I = 0;
	protected static final int D = 1;
	protected static final int V = 2;
	protected static final int W = 3;
	protected static final int E = 4;

	protected void DxbLog(int level, String mString) {
		if(getClassName() != null) {
			switch(level) {
    		case E:
    			Log.e(getClassName(), mString);
    			break;

    		case I:
    			Log.i(getClassName(), mString);
    			break;
    		
    		case V:
    			Log.v(getClassName(), mString);
    			break;
    		
    		case W:
    			Log.w(getClassName(), mString);
    			break;
    		
    		case D:
    		default:
    			Log.d(getClassName(), mString);
    			break;
			}
		}
	}
}
