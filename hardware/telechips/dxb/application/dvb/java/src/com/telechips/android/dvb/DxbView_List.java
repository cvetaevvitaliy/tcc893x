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

import java.io.File;

import com.telechips.android.dvb.player.DxbPlayer;
import com.telechips.android.dvb.player.DxbPVR.LOCAL_STATE;
import com.telechips.android.dvb.util.DxbStorage;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.TextView;

public class DxbView_List extends DxbView implements	AdapterView.OnItemClickListener, AdapterView.OnItemSelectedListener, AdapterView.OnItemLongClickListener,
														View.OnClickListener
{
	private final View mView;
	
	//	List
	private ImageView	mivPrev, mivNext;
	private int[]		iString_Title	= {
											R.string.tv,
											R.string.radio,
											R.string.pvr_channel
										};
	private TextView	tvTitle;
	private ListView	mlvService;

	// Video
	private ImageView	mivImage;
	private View		mvVideo;
	private TextView	mtvMessage;

	private int			mID_row;

	private DxbAdapter_ArrayList	mAdapter_ArrayList	= null;
	private DxbAdapter_Cursor		mAdapter_Cursor		= null;

	private static final int miCount_Tab = 3;
	private int	miIndex_Tab_old = 0;
	private boolean mBookmarkFlag = false;
	private	int	mBookmarkIndex	= 0;

	public DxbView_List(Context context, View v)
	{
		super(context);
		mView = v;
		
		setComponent();
		setFontSize();
		setListener();
	}

	@Override
	public void PlayingCompletion(DxbPlayer player, int type, int state)
	{
		DxbLog(D, "onPlayingCompletion(type = " + type + ", state = " + state + ")");

		//	type : 0-normal, 1-speed, 2-seek
		if (type == 0)
		{
			if(getPlayer().getServiceType() == 2)
			{
				getPlayer().stop();
			}
			
			setPlaySpeed(5);
			onUpdateScreen();
		}
		else if (type == 1)
		{
			setPlaySpeed(5);
		}
	}
	
	@Override
	protected void FilePlayTimeUpdate(DxbPlayer player, int time)
	{
		/*DxbLog(I, "FilePlayTimeUpdate(time=" + time + ") --> mivImage.isShown()=" + mivImage.isShown());
		if(mivImage.getVisibility() == View.VISIBLE)
		{
			mivImage.setVisibility(View.GONE);	//	error : recording --> list_view --> hidden play_icon
		}*/
	}

	@Override
	protected void VideoOutput(DxbPlayer player)
	{
		DxbLog(I, "VideoOutput()");
		if(!getPlayer().isVideoPlay())
		{
			mvVideo.setBackgroundColor(0xFF101010);
		}
		else if(	!getPlayer().isParentalLock()
				||	getPlayer().isUnlockParental()
		)
		{
			if(getPlayer().getServiceType() == 2)
			{
				mivImage.setVisibility(View.GONE);
			}
			
			mvVideo.setBackgroundColor(0x00000000);
		}
	}

	public void setVisible(boolean v)
	{
		setIndicatorVisible(v);
		mView.setVisibility(v ? View.VISIBLE : View.GONE);
		if(v)
		{
			getPlayer().setSubtitleEnable(false);
			
			getContext().getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN | 0x80000000, WindowManager.LayoutParams.FLAG_FULLSCREEN | 0x80000000); 
			getContext().getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

			onUpdateScreen();
			updateChannelList();
			
			RelativeLayout.LayoutParams lp = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.MATCH_PARENT);
			lp.setMargins(0, 0, 0, getPlayer().getStatusbarHeight()-2);	// set_position - STB
			mView.setLayoutParams(lp);
			
			int flag =		View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
						|	View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
						|	View.SYSTEM_UI_FLAG_LAYOUT_STABLE;

			mView.setSystemUiVisibility(flag);
		}
	}
	
	private void setComponent()
	{
		DxbLog(I, "setComponent()");
			
		//	List
		mivPrev		= (ImageView)mView.findViewById(R.id.list_prev);
		mivNext		= (ImageView)mView.findViewById(R.id.list_next);
		tvTitle		= (TextView)mView.findViewById(R.id.list_title);
		mlvService	= (ListView)mView.findViewById(R.id.list_service);
		
		//	Video
		mivImage	= (ImageView)getContext().findViewById(R.id.list_image);
		mvVideo		= mView.findViewById(R.id.list_view);
		mtvMessage	= (TextView)mView.findViewById(R.id.list_message);

		if(getPlayer().isSTB())
		{
			mID_row	= R.layout.dxb_list_row_60px_stb;
		}
		else
		{
			if(getDisplayWidth() >= 1024)
			{
				mID_row	= R.layout.dxb_list_row_60px;
			}
			else
			{
				mID_row	= R.layout.dxb_list_row_40px;
			}
		}
	}

	private void setFontSize()
	{
		//	Video
		mtvMessage.setTextSize(getTextSize(18));
		
		//	List
		tvTitle.setTextSize(getTextSize(25));
	}

	private void setListener()
	{
		//	List
		mivPrev.setOnClickListener(this);
		mivNext.setOnClickListener(this);
		mlvService.setOnItemClickListener(this);
		mlvService.setOnItemLongClickListener(this);
		mlvService.setOnItemSelectedListener(this);

		//	OnClickListener
		mivImage.setOnClickListener(this);
		mvVideo.setOnClickListener(this);
	}

	public void onItemClick(AdapterView<?> parent, View v, int position, long id)
	{
		DxbLog(D, "ListenerOnItemClick-->onItemClick(position=" + position + ")");
			
		if(getPlayer().eState == DxbPlayer.STATE.SCAN_STOP)
		{
			updateToast(getResources().getString(R.string.please_wait_cancel_scanning));
			return;
		}
		
		if(		(position == getPlayer().getChannelPosition())
			&&	(		(getServiceType() == 2 && getPlayer().getLocalPlayState() != LOCAL_STATE.STOP)
					||	(getServiceType() != 2)
				)
		)
		{
			setState(false, DxbView.VIEW_MAIN);
		}
		else
		{
			resetChannel();
			setChannel(position);
			mlvService.setSelection(position);
		}
	}

    public void onItemSelected(AdapterView<?> parent, View view, int position, long id)
    {
        DxbLog(D, "onItemSelected(id=" + id + " position=" + position + ")");
/*
		if ((getServiceType() < 2) && (getPlayer().getChannelCount()>1))
		{
			setCurrentService(true);
		}
*/
    }

    public void onNothingSelected(AdapterView<?> list)
    {
    	DxbLog(D, "onNothingSelected()");
    }

    public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id)
    {
    	DxbLog(D, "onItemLongClick(id=" + id + " position=" + position + ")");
    	
		if (getServiceType() == 2)
		{
			getPlayer().setLocalPlayStop();
			
			final String fileName = mAdapter_Cursor.getCursor().getString(getPlayer().getColumnIndex_ServiceName());
			AlertDialog.Builder builder = new AlertDialog.Builder(getContext());
			builder.setTitle("Delete");
			builder.setMessage("Delete  '" + fileName + "'  from storage");
			builder.setOnCancelListener(
					new DialogInterface.OnCancelListener() {
						public void onCancel(DialogInterface dialog) {
						}
					});
			builder.setPositiveButton(
					"Ok", new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int which) {
							File f = new File(DxbStorage.getPath_Device() + "/" + fileName);
							DxbLog(D, "Delete " + DxbStorage.getPath_Device() + "/" + fileName);
							if (f != null && f.isFile()) {
								if (f.delete()) {
									updateChannelList();
								} else {
									updateToast("Failed to delete " + fileName);
								}
							}
						}
					});
			builder.setNegativeButton(
					"Cancel", new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int which) {
						}
					});
			builder.show();
			return true;
		}
		
        return false;
    }

	public void onClick(View v)
	{
		DxbLog(I, "ListenerOnClick - onClick()");
	
		if(getPlayer().eState == DxbPlayer.STATE.SCAN_STOP)
		{
			updateToast(getResources().getString(R.string.please_wait_cancel_scanning));
			return;
		}
		else if(getPlayer().eState == DxbPlayer.STATE.SCAN)
		{
			return;
		}

		int id = v.getId();
		switch(id)
		{
			case R.id.list_prev:
				previousTab();
			break;

			case R.id.list_next:
				nextTab();
			break;

			case R.id.list_view:
				setState(false, VIEW_MAIN);	// (full view) TV only.
			break;
			
			case R.id.list_image:
				if(getPlayer().getServiceType() == 2)
				{
					getPlayer().setFileList_cursor(getPlayer().getChannelPosition(2));
					setChannel();
				}
			break;
		}
	}

	public Runnable mRunnable_KeyEvent = new Runnable() {
		public void run() {
			DxbLog(I, "mRunnable_KeyEvent --> run()");
			setCurrentService(false);
		}
	};

	private void setCurrentService(boolean state)
	{
		DxbLog(I, "setCurrentService(state=" + state + ")");
		if(state)
		{
			getMainHandler().removeCallbacks(mRunnable_KeyEvent);
			getMainHandler().postDelayed(mRunnable_KeyEvent, 1000);
		}
		else
		{
			getMainHandler().removeCallbacks(mRunnable_KeyEvent);
		
			if (getPlayer().getChannelCount() > 0)
			{
				resetChannel();
				setChannel(-1);
			}
		}
	}
	
	public void previousTab() {
		int iPrevTab = (getServiceType() + miCount_Tab - 1) % miCount_Tab;
		changeTab(iPrevTab);
	}
	
	public void nextTab() {
		int iNextTab = (getServiceType() + 1) % miCount_Tab;
		changeTab(iNextTab);
	}

	public void changeTab(int iIndex)
	{
		DxbLog(I, "changeTab() : iIndex_Tab_current=" + getServiceType() + " --> iIndex=" + iIndex);
	
		if ((iIndex == getServiceType()) || (iIndex < 0) || (iIndex >= miCount_Tab))
		{
			return;
		}
		
		if(getPlayer().eState == DxbPlayer.STATE.SCAN_STOP)
		{
			updateToast(getResources().getString(R.string.please_wait_cancel_scanning));
			return;
		}

		if (getServiceType() == 2)
		{
			getPlayer().setLocalPlayStop();
		}
		else
		{
			getPlayer().stop();
		}

		tvTitle.setText(getResources().getString(iString_Title[iIndex]));

		setServiceType(iIndex);
		updateChannelList();
	}

	private class Bookmark_Thread extends Thread {
		public Bookmark_Thread(Runnable runnable) {
			super(runnable);
		}
		
		@Override
		public void run() {
			super.run();
			getMainHandler().post(mBookmark_Update);
		}
	}

	private class Bookmark_Runnable implements Runnable
	{
		public void run()
		{
			DxbLog(D, "Bookmark_Runnable --> run()");
			getPlayer().toggleBookmark(mlvService.getSelectedItemPosition());
		}
	}

	private Runnable mBookmark_Runnable = new Runnable()
	{
		public void run()
		{
			Bookmark_Runnable bookmarkRunnable = new Bookmark_Runnable();
			Bookmark_Thread bookmarkThread = new Bookmark_Thread(bookmarkRunnable);
			bookmarkThread.start();
		}
	};

	private Runnable mBookmark_Update = new Runnable()
	{
		public void run()
		{
			DxbLog(I, "mBookmark_Update()");
			getMainHandler().post(mUpdateList_Runnable);
		}
	};

	private Runnable mRunnable_TabEvent = new Runnable()
	{
		public void run()
		{
			DxbLog(I, "mRunnable_TabEvent --> run()");
			int	iCurrentTab	= getServiceType();
			if(getPlayer().getChannelCount() > 0)
			{
				int iPosition;
				if(miIndex_Tab_old != iCurrentTab)
				{
					if(iCurrentTab < 2)
					{
						iPosition	= getPlayer().getChannelPosition();
						mlvService.setSelection(iPosition);
						mlvService.requestFocus();
						resetChannel();
						setChannel(iPosition);
					}
					else if(iCurrentTab == 2)
					{
						getPlayer().setFileList_cursor(getPlayer().getChannelPosition(2));
						getPlayer().setCurrent_ChannelData();
						onUpdateScreen();
					}
				}
				else if(iCurrentTab < 2)
				{
					setChannel();
				}
			}
			else
			{
				if(miIndex_Tab_old != iCurrentTab)
				{
					getPlayer().resetSubtitleInformation();
					getPlayer().resetTeletextInformation();
				}
			}
			
			miIndex_Tab_old = iCurrentTab;
		}
	};

	public void updateChannelList()
	{
		DxbLog(I, "updateChannelList()");
		getMainHandler().removeCallbacks(mUpdateList_Runnable);
		getMainHandler().post(mUpdateList_Runnable);
	}

    private Runnable mUpdateList_Runnable = new Runnable()
    {
        public void run()
        {
			DxbLog(I, "mUpdateList_Update()");
		
			if(getServiceType() == 2)
			{
				//Cursor	curFile	= getPlayer().getFileList();
				if(mAdapter_Cursor == null)
				{
					mAdapter_Cursor = new DxbAdapter_Cursor(getContext(), mID_row, getPlayer().getFileList(), new String[] {}, new int[] {});
				}
				else
				{
					mAdapter_Cursor.changeCursor(getPlayer().getFileList());
				}
				
				mAdapter_Cursor.setUseIndicator(false);
				mlvService.setAdapter(mAdapter_Cursor);
			}
			else
			{
				mAdapter_ArrayList = new DxbAdapter_ArrayList(getContext(), mID_row, getPlayer().getChannelList(getServiceType()));
				mlvService.setAdapter(mAdapter_ArrayList);
			}

			if(mBookmarkFlag == true)
			{
				mBookmarkFlag = false;
				mlvService.invalidateViews();
				mlvService.setSelection(mBookmarkIndex);
			}
			else
			{
				getMainHandler().removeCallbacks(mRunnable_TabEvent);
				getMainHandler().postDelayed(mRunnable_TabEvent, 1000);
			}
			
			onUpdateScreen();
        }
    };

	@Override
	public void onChannelChange()
	{
		DxbLog(I, "onChannelChange()");
	
		onUpdateScreen();
		
		mlvService.invalidateViews();
	}

	@Override
	protected void onUpdateScreen()
	{
		DxbLog(I, "onUpdateScreen()");

		int	image = 0, text = 0;

		if (getPlayer().isPlaying())
		{
			tvTitle.setText(getResources().getString(iString_Title[getPlayer().getServiceType()]));
			if (getPlayer().getChannelCount() <= 0)
			{ // no channel
				image	= R.drawable.dxb_portable_channel_no_channel_img;
				
				if(getPlayer().getServiceType() == 2)
				{
					text = R.string.no_pvr_file_found;
				}
				else
				{
					text = R.string.if_you_want_to_use_radio_scan_channel;
				}
			}
			//	else(getPlayer().getChannelCount() > 0){}
			else if(getPlayer().getServiceType() == 2)
			{
				if(getPlayer().getLocalPlayState() == LOCAL_STATE.STOP)
				{
					image = R.drawable.ic_control_play;
				}
			}
			//	else(getPlayer().getServiceType() <= 2){}
			else if(getSignalLevel() < 0)	// weak signal
			{
				image = R.drawable.dxb_portable_channel_weak_signal_img;
			}
			else if(	getPlayer().isParentalLock()
					&&	!getPlayer().isUnlockParental()
			)
			{
				image	= R.drawable.dxb_portable_lock_img;
				getPlayer().setPlayPause(getPlayer().getServiceType(), false);
			}
			else if(getPlayer().isRadio())
			{
				image	= R.drawable.dxb_portable_channel_radio_img;
			}
			else if(getPlayer().isScrambled())
			{
				image	= R.drawable.dxb_portable_channel_no_play_img;
			}
			
			if (image == 0)
			{
				mivImage.setVisibility(View.GONE);
				if(getPlayer().isVideoPlay())
				{
					mvVideo.setBackgroundColor(0x00000000);
				}
				else
				{
					mvVideo.setBackgroundColor(0xFF101010);
				}
			}
			else
			{
				mivImage.setBackgroundResource(image);
				mivImage.setVisibility(View.VISIBLE);
				mvVideo.setBackgroundColor(0xFF101010);
			}
			
			if (text == 0)
			{
				mtvMessage.setVisibility(View.GONE);
			}
			else
			{
				mtvMessage.setText(getResources().getString(text));
				mtvMessage.setVisibility(View.VISIBLE);
			}
		}
	}
	
	@Override
	protected void onUpdateEPG_PF()
	{
		DxbLog(I, "onUpdateEPG_PF()");
		
		if(getPlayer().getServiceType() == 2)
		{
			return;
		}
		
		if(getPlayer().isPlaying())
		{
			getPlayer().setCurrentRating(0);
			
			onUpdateScreen();
		}
	}

	@Override
	public boolean onGenericMotionEvent(MotionEvent event) {
		if ((event.getSource() & InputDevice.SOURCE_CLASS_POINTER) != 0) {
			switch (event.getAction()) {
			case MotionEvent.ACTION_SCROLL:
				final float vscroll;
				final float hscroll;
				if ((event.getMetaState() & KeyEvent.META_SHIFT_ON) != 0) {
					vscroll = 0;
					hscroll = event.getAxisValue(MotionEvent.AXIS_VSCROLL);
				} else {
					vscroll = -event.getAxisValue(MotionEvent.AXIS_VSCROLL);
					hscroll = event.getAxisValue(MotionEvent.AXIS_HSCROLL);
				}
				if (hscroll != 0 || vscroll != 0) {
					if (hscroll > 0 || vscroll > 0) {
						volumeDown();
					} else {
						volumeUp();
					}
					return true;
				}
			}
		}
		return false;
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		DxbLog(I, "onKeyDown(keyCode=" + keyCode + ", event=" + event + ")");

		if (keyCode == KeyEvent.KEYCODE_BACK || keyCode == KeyEvent.KEYCODE_ESCAPE)
		{
			setState(false, VIEW_MAIN);
			return true;
		}

		if(press_InputChannel(keyCode))
		{
			return true;
		}

		switch(keyCode)
		{
			case KeyEvent.KEYCODE_PAGE_UP:
				setChannelUp();
			break;

			case KeyEvent.KEYCODE_PAGE_DOWN:
				setChannelDown();
			break;
			
			case KeyEvent.KEYCODE_DPAD_DOWN:
				mlvService.requestFocus();
			break;

			case KeyEvent.KEYCODE_DPAD_LEFT:
				previousTab();
			break;
			
			case KeyEvent.KEYCODE_DPAD_RIGHT:
				nextTab();
			break;

			case KeyEvent.KEYCODE_BOOKMARK:
				if ((getServiceType() < 2) && (getPlayer().getChannelCount()>=1))	//?? >1
				{
					mBookmarkFlag = true;
					mBookmarkIndex	= mlvService.getSelectedItemPosition();
					getMainHandler().post(mBookmark_Runnable);
				}
			break;

			default:
				return false;
		}
		return true;
	}

	@Override
	protected String getClassName() {
		return "[[[DxbView_List]]]";
	}
}
