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

import java.util.*;

import android.content.*;
import android.database.Cursor;
import android.graphics.Color;
import android.graphics.Typeface;
import android.util.Log;
import android.view.*;
import android.widget.*;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemSelectedListener;

import com.telechips.android.isdbt.player.*;
import com.telechips.android.isdbt.util.DxbUtil;
import com.telechips.android.isdbt.player.isdbt.*;

public class DxbView_EPG extends DxbView implements View.OnFocusChangeListener, View.OnClickListener, View.OnKeyListener,
													OnItemClickListener, OnItemSelectedListener
{
	private final View mView;
	
	//	Spinner
	private Spinner		mspinner_ServiceType;
	private Spinner		mspinner_ServiceList;

	//	Navi
	private Button		mbuPrev;
	private Button		mbuNext;
	private TextView	mtvDate_Current;

	//	List
	private ListView	mlvEPG_List;
	private TextView	mtvName_Current;
	private TextView	mtvDetail_Current;
		
	//	NoData
	private ImageView	mivNoData;
	
	// Video
	private View		mvVideo;

	//	Schedule Menu
	private Button		mbuBook;
	private Button		mbuTimer;
			
	private ArrayAdapter<String>		spinnerAdapter_Type	= null;

	private int miCount;
	private int	miCurrent;
	
	private Cursor mepgCursor = null, mNextEPG = null;
	
	private int	miIndex_startMJD;
	private int	miIndex_startHH, miIndex_startMM, miIndex_durationHH, miIndex_durationMM;
	private int	miIndex_name, miIndex_event_text, miIndex_event_extern;
	private int miIndex_TableID;

	private int miEpgUpdate_day = 0;

	// Table;
	private int iFirst_TableID;
	private int iLast_TableID;

	public DxbView_EPG(Context context, View v)
	{
		super(context);
		mView = v;

		setComponent();
		setTextSize();
		setListener();
	}

	@Override
	protected void EPGUpdate(DxbPlayer player, int serviceID, int tableID)
	{
		if(tableID >= iFirst_TableID && tableID <= iLast_TableID) 
		{
			DxbLog(I, "EPG Updated(" + tableID + ")");

			updateList();
		}
	}

	private void setComponent()
	{
		DxbLog(I, "setComponent()");
		
		Typeface typeface = Typeface.createFromFile("/system/fonts/DroidSansFallback_DxB.ttf");

		//	Spinner
		mspinner_ServiceType	= (Spinner)mView.findViewById(R.id.epg_service_type_spinner);
		mspinner_ServiceList	= (Spinner)mView.findViewById(R.id.epg_service_list_spinner);
		
		//	Navi
		mbuPrev	= (Button)mView.findViewById(R.id.epg_prev);
		mbuNext	= (Button)mView.findViewById(R.id.epg_next);
		mtvDate_Current	= (TextView)mView.findViewById(R.id.epg_current_text);
		
		mtvDate_Current.setTypeface(typeface);

		//	List
		mlvEPG_List		= (ListView)mView.findViewById(R.id.epg_list);
		
		//	detail info
		mtvName_Current	= (TextView)mView.findViewById(R.id.epg_name);
		mtvDetail_Current	= (TextView)mView.findViewById(R.id.epg_detail);

		mtvName_Current.setTypeface(typeface);
		mtvDetail_Current.setTypeface(typeface);
	
		//	Nodata
		mivNoData	= (ImageView)mView.findViewById(R.id.epg_no_data);
		
		//	Video
		mvVideo	= mView.findViewById(R.id.epg_view);
		
		//	Menu
		mbuBook	= (Button)mView.findViewById(R.id.epg_b_book);
		mbuTimer	= (Button)mView.findViewById(R.id.epg_b_timer);
	}
	
	private void setTextSize()
	{
		DxbLog(I, "setTextSize()");

		float	infoTextSize;
		
		//	detail info
		infoTextSize	= getTextSize(20);
		mtvName_Current.setTextSize(infoTextSize);
		infoTextSize	= getTextSize(18);
		mtvDetail_Current.setTextSize(infoTextSize);
		
		//	Navi
		infoTextSize	= getTextSize(20);
		mtvDate_Current.setTextSize(infoTextSize);
	}
	
	private void setListener()
	{
		DxbLog(I, "setListener()");

		//	Spinner - OnItemSelectedListener
		mspinner_ServiceType.setOnItemSelectedListener(ListenerOnItemSelected_spinnerType);
		mspinner_ServiceList.setOnItemSelectedListener(ListenerOnItemSelected_spinnerList);
		
		//	OnFocusChangeListener
		mbuPrev.setOnFocusChangeListener(this);
		mbuNext.setOnFocusChangeListener(this);
		
		//	OnClickListener
		mbuPrev.setOnClickListener(this);
		mbuNext.setOnClickListener(this);
		
		//	ListView Listener
		mlvEPG_List.setOnItemSelectedListener(this);
		mlvEPG_List.setOnItemClickListener(this);
		mlvEPG_List.setOnKeyListener(this);
		
		//	Video
		mvVideo.setOnClickListener(this);
		
		//	Menu
		mbuBook.setOnClickListener(this);
		mbuTimer.setOnClickListener(this);
		mbuBook.setOnFocusChangeListener(this);
		mbuTimer.setOnFocusChangeListener(this);
	}

	public void onFocusChange(View v, boolean hasFocus)
	{
		DxbLog(I, "onFocusChange()");

		int id = v.getId();
		if(id == R.id.epg_prev) {
			if(miEpgUpdate_day > 0) {
				if(hasFocus) {
					v.setBackgroundResource(R.drawable.dxb_portable_epg_left_btn_f);
				} else {
					v.setBackgroundResource(R.drawable.dxb_portable_epg_left_btn_n);
				}
			}
		} else if(id == R.id.epg_next) {
			if(mNextEPG != null) {
				if(mNextEPG.getCount() > 0) {
					if(hasFocus) {
						v.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_f);
					} else {
						v.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_n);
					}
				}
			}
		} else if(id == R.id.epg_b_book) {
			if(hasFocus) {
				v.setBackgroundResource(R.drawable.ics_button);
			} else {
				v.setBackgroundColor(getResources().getColor(R.color.color_red));
			}
		} else if(id == R.id.epg_b_timer) {
			if(hasFocus) {
				v.setBackgroundResource(R.drawable.ics_button);
			} else {
				v.setBackgroundColor(getResources().getColor(R.color.color_green));
			}
		}
	}
	
	public void onClick(View v)
	{
		DxbLog(I, "mClickListener_epgSelect - onClick()");
	
		int id = v.getId();
		switch(id)
		{
			case R.id.epg_prev:
				if(mbuPrev.isFocusable())
				{
					if(miEpgUpdate_day <= 0)
					{
						updateToast(getResources().getString(R.string.first_data));
						return;
					}
					
					miEpgUpdate_day--;
					miCurrent		= 0;
				}
			break;

			case R.id.epg_next:
				if(mbuNext.isFocusable())
				{
					miEpgUpdate_day++;
					miCurrent		= 0;
				}
			break;

			case R.id.epg_view:
			{
				close_EPGView();
				setState(false, VIEW_MAIN);
			}
			break;

			case R.id.epg_b_book:
				startScheduler(0);
			return;

			case R.id.epg_b_timer:
				startScheduler(1);
			return;
		}
		
		updateList();
	}

	public void onItemSelected(AdapterView<?> arg0, View v, int position, long arg3)
	{
		DxbLog(I, "onItemSelected(position=" + position + ", arg3=" + arg3 + ", v=" + v + ")");

		if(v != null)
		{
			int id = v.getId();
			DxbLog(I, "ListenerOnItemSelected --> onItemSelected(id=" + id + ", position=" + position + ", arg3=" + arg3 + ")");
			
			switch(id)
			{
				case -1:	//	STB --> -1
				case R.id.epg_list:
				{
					miCurrent	= position;
					mtvName_Current.setText(getTitle_all(mepgCursor));
					mtvDetail_Current.setText(getEPG_Event(mepgCursor));
					
					setEventData(mepgCursor);
				}
				break;
			}
		}
	}
	
	private AdapterView.OnItemSelectedListener ListenerOnItemSelected_spinnerType	= new AdapterView.OnItemSelectedListener()
	{
		public void onItemSelected(AdapterView<?> arg0, View v, int position, long arg3)
		{
			int id = v.getId();
			DxbLog(I, "ListenerOnItemSelected_spinnerType --> onItemSelected(id=" + id + ", position=" + position + ", arg3=" + arg3 + ")");
			
			switch(id)
			{
				case R.id.spinner_text:
				{
					if(getServiceType() != position)
					{
						setServiceType(position);
						setChannel();
						updateSpinner_List();
						updateList();
					}
				}
				break;
			}			
		}

		public void onNothingSelected(AdapterView<?> arg0)
		{
			// TODO Auto-generated method stub
		}
	};
	
	private AdapterView.OnItemSelectedListener ListenerOnItemSelected_spinnerList	= new AdapterView.OnItemSelectedListener()
	{
		public void onItemSelected(AdapterView<?> arg0, View v, int position, long arg3)
		{
			int id = v.getId();
			DxbLog(I, "ListenerOnItemSelected_spinnerList --> onItemSelected(id=" + id + ", position=" + position + ", arg3=" + arg3 + ")");
			
			switch(id)
			{
				case R.id.spinner_text:
				{
					if(getChannelPosition() != position)
					{
						setChannel(getServiceType(), position);
//						updateList();
					}
				}
				break;
			}
		}

		public void onNothingSelected(AdapterView<?> arg0)
		{
			// TODO Auto-generated method stub
		}
	};
	
	public void onNothingSelected(AdapterView<?> arg0)
	{
		DxbLog(D, "onNothingSelected()");
		
		//arg0.setBackgroundResource(R.drawable.memob);
	} 

	public void onItemClick(AdapterView<?> parent, View v, int position, long id)
	{
		DxbLog(I, "ListenerOnItemClick --> onItemClick(position=" + position + ", id=" + id  + ")");
		
		if(isValidateCussor(mepgCursor))
		{
			mepgCursor.moveToPosition(position);
		}
		mlvEPG_List.setSelection(position);

		mtvName_Current.setText(getTitle(mepgCursor));
			
		String	epgDetail	= getEvent_Extern(mepgCursor);
		if (epgDetail == null)
		{
			epgDetail	= getEvent(mepgCursor);
		}
		else if(epgDetail.length() < 1)
		{
			epgDetail	= getEvent(mepgCursor);
		}
		if(mtvDetail_Current != null)
		{
			mtvDetail_Current.setText(epgDetail);
		}

		setEventData(mepgCursor);
			
		mlvEPG_List.invalidateViews();
	}
	
	public boolean onKey(View v, int keyCode, KeyEvent event)
	{
		if ((event.getAction() == 1) || (event.getRepeatCount() > 0))
		{
			DxbLog(I, "ListenerOnKey --> onKey(keyCode=" + keyCode + ", event=" + event + ")");
		
			switch (keyCode)
			{
				case KeyEvent.KEYCODE_ENTER:
					startScheduler(0);
				break;
			}
		}

		if ((event.getAction() == 0) || (event.getRepeatCount() > 0))
		{
			return false;
		}
		
		return true;
	}

	public void setVisible(boolean v)
	{
		DxbLog(I, "setVisible(v=" + v + ")");
		
		setIndicatorVisible(v);
		resetData();
		if (v)
		{
			if(!isBookRecord())
			{	
				mbuBook.setVisibility(View.INVISIBLE);
				mbuTimer.setVisibility(View.INVISIBLE);
			}
			
			getContext().getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN | 0x80000000, WindowManager.LayoutParams.FLAG_FULLSCREEN | 0x80000000); 
			getContext().getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

			if (getPlayer().getChannelCount() > 0)
			{
				mView.findViewById(R.id.epg_schedule_button).setVisibility(View.VISIBLE);
			}
			else
			{
				mView.findViewById(R.id.epg_schedule_button).setVisibility(View.GONE);
			}
			
			setSpinner();
			onUpdateScreen();
			updateList();
			
			RelativeLayout.LayoutParams lp = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.MATCH_PARENT);
			lp.setMargins(0, 0, 0, getPlayer().getStatusbarHeight()-2);	// set_position - STB
			mView.setLayoutParams(lp);
			
			int flag =		View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
						|	View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
						|	View.SYSTEM_UI_FLAG_LAYOUT_STABLE;

			mView.setSystemUiVisibility(flag);
		}
		
		mView.setVisibility(v ? View.VISIBLE : View.GONE);
	}
	
	private void setSpinner()
	{
		DxbLog(I, "setSpinner()");
		
		updateSpinner_Type();
		updateSpinner_List();
		
		mspinner_ServiceType.setSelection(getServiceType());
		mspinner_ServiceList.setSelection(getChannelPosition());
		
		if(getPlayer().getRecordState() != DxbPlayer.REC_STATE.STOP)
		{
			mspinner_ServiceType.setEnabled(false);
			mspinner_ServiceList.setEnabled(false);
		}
		else
		{
			mspinner_ServiceType.setEnabled(true);
			mspinner_ServiceList.setEnabled(true);
		}
	}
	
	private void updateSpinner_Type()
	{
		DxbLog(I, "updateSpinner_Type()");
		
		ArrayList<String>	Items	= new ArrayList<String>();
		if(getPlayer().getChannelCount(0) > 0)
		{
			Items.add(getResources().getString(R.string.tv));
		}
		if(getPlayer().getChannel_Count(1) > 0)
		{
			Items.add(getResources().getString(R.string.radio));
		}
		
		if(getDisplayWidth() > 800)
		{
			spinnerAdapter_Type	= new ArrayAdapter<String>(getContext(), R.layout.dxb_spinner_35px, Items);
		}
		else
		{
			spinnerAdapter_Type	= new ArrayAdapter<String>(getContext(), R.layout.dxb_spinner_18px, Items);
		}
		
		mspinner_ServiceType.setAdapter(spinnerAdapter_Type);
	}
	
	private void updateSpinner_List()
	{
		DxbLog(I, "updateSpinner_List()");
		
		ArrayAdapter<String>		spinnerAdapter_List;
		if(getDisplayWidth() > 800)
		{
			spinnerAdapter_List	= new ArrayAdapter<String>(getContext(), R.layout.dxb_spinner_35px, getPlayer().getNameList(getServiceType()));
		}
		else
		{
			spinnerAdapter_List	= new ArrayAdapter<String>(getContext(), R.layout.dxb_spinner_18px, getPlayer().getNameList(getServiceType()));
		}
		
		mspinner_ServiceList.setAdapter(spinnerAdapter_List);
	}

	private void startScheduler(int iAction)
	{
		DxbLog(I, "startScheduler(iAction=" + iAction + ")");
		
		getPlayer().setEventData(iAction);

		close_EPGView();
		setState(false, VIEW_SCHEDULE);
	}

	private boolean isValidateCussor(Cursor cursor)
	{
		return (	cursor != null
				&&	cursor.getCount() > 0
				&&	!cursor.isClosed()
				);
	}
	
	private void resetData()
	{
		getMainHandler().removeCallbacks(mSchedule_Init_Runnable);
		
		if(mepgCursor != null)
		{
			if(!mepgCursor.isClosed())
			{
				mepgCursor.close();
			}
			
			mepgCursor	= null;
		}
		
		if(mNextEPG != null)
		{
			if(!mNextEPG.isClosed())
			{
				mNextEPG.close();
			}
			mNextEPG = null;
		}
		
		miEpgUpdate_day	= 0;
		miCount	= 0;
	}
	
	private void setDisplayDate(Cursor cur)
	{
		int	MJD;
        DateTimeData datetime;
        String	date;
        
		datetime = getPlayer().getCurrentDateTime();
        MJD = DxbUtil.getMJD(cur.getInt(miIndex_startMJD), cur.getInt(miIndex_startHH), datetime.iPolarity, datetime.iHourOffset);
		date = DxbUtil.getMJD_Date(MJD);
		
		mtvDate_Current.setText(date);
	}
	
	private int mScheduleType = 0;
	private class Schedule_Init_Thread extends Thread
	{
		public Schedule_Init_Thread(Runnable runnable)
		{
			super(runnable);
			DxbLog(D, "Schedule_Init_Thread(miEpgUpdate_day=" + miEpgUpdate_day + ")");
		}
		
		@Override
		public void run()
		{
			super.run();
		
			DxbLog(D, "Schedule_Init_Thread --> run() --> mScheduleType = " + mScheduleType);
			switch(mScheduleType)
			{
				case 0:
					{
					}
					break;

				case 1:
					{
						mNextEPG = getPlayer().getEPG_Schedule();
					DxbLog(D, "mNextEPG.getCount() = " + mNextEPG.getCount());
					}
					break;
			}
		}
	}

	private class Schedule_Init_Runnable implements Runnable
	{
		public void run()
		{
			DxbLog(I, "Schedule_Init_Runnable()");
		}
	}

	private Runnable mSchedule_Init_Runnable = new Runnable()
	{
		public void run()
		{
			Schedule_Init_Runnable initRunnable = new Schedule_Init_Runnable();
			Schedule_Init_Thread initThread = new Schedule_Init_Thread(initRunnable);
			initThread.start();
		}
	};

	private Runnable mSchedule_Init_Update = new Runnable()
	{
		public void run()
		{
			DxbLog(I, "mSchedule_Init_Update(mScheduleType=" + mScheduleType + ")");
			
			switch(mScheduleType)
			{
				case 0:
					if(		mepgCursor != null
						&&	!mepgCursor.isClosed()
						&&	miCount != mepgCursor.getCount()
					)
					{
						updatePrevBt(miEpgUpdate_day);
						updateList(mepgCursor);
						updateNextBt(mepgCursor);	// EPG_updateList() --> Next (Use, gInformation.miCount)
					}
					else
					{
						getMainHandler().removeCallbacks(mSchedule_Init_Runnable);
						getMainHandler().postDelayed(mSchedule_Init_Runnable, 3000);
					}
				break;
			
				case 1:
					if(mNextEPG == null)
					{
						mbuNext.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_d);
						mbuNext.setFocusable(false);
					}
					else if(mNextEPG.getCount() <= 0)
					{
						mbuNext.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_d);
						mbuNext.setFocusable(false);
					}
					else
					{
						//Log.d(TAG, "mNextEPG.getCount() = " + mNextEPG.getCount());
						mbuNext.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_n);
						mbuNext.setFocusable(true);
					}
				break;
			}
		}
	};

	private void updateList()
	{
		//DxbLog(I, "updateList()");
		
		getMainHandler().removeCallbacks(mSchedule_Init_Runnable);
		
		getPlayer().setEPGUpdate_state(false);
		if( getPlayer().openEPGDB() == false)
		{
			DxbLog(D, "CANNOT open EPG database !!!");
			
			miEpgUpdate_day	= 0;
			mepgCursor	= null;
		}
		else
		{
			mScheduleType = 0;
			if(mepgCursor != null)
			{
				if(!mepgCursor.isClosed())
				{
					mepgCursor.close();
				}
			}
			mepgCursor = getPlayer().getEPG_Schedule(miEpgUpdate_day);
			//DxbLog(D, "mepgCursor.getCount() = " + mepgCursor.getCount());

			if(mepgCursor != null && mepgCursor.getCount() > 0)
			{
				mepgCursor.moveToFirst();
				iFirst_TableID = mepgCursor.getInt(miIndex_TableID);

				mepgCursor.moveToLast();
				iLast_TableID = mepgCursor.getInt(miIndex_TableID);
			}

			updatePrevBt(miEpgUpdate_day);
			updateList(mepgCursor);
			updateNextBt(mepgCursor);	// EPG_updateList() --> Next (Use, gInformation.miCount)
		}
		
		getMainHandler().postDelayed(mRunnable_updateList, 5000);
	}
    
	private Runnable mRunnable_updateList = new Runnable()
	{
		public void run()
		{
			//DxbLog(I, "mRunnable_updateList --> run()");
			if(getPlayer().isEPGUpdate())
			{
				updateList();
			}
			else
			{
				getMainHandler().postDelayed(mRunnable_updateList, 5000);
			}
		}
	};
		
	private void updatePrevBt(int iDay)
	{
		DxbLog(I, "EPG_updatePrevBt(iDay = " + iDay + ")");
		
		if(iDay <= 0)
		{
			mbuPrev.setBackgroundResource(R.drawable.dxb_portable_epg_left_btn_d);
			mbuPrev.setFocusable(false);
		}
		else
		{
			mbuPrev.setBackgroundResource(R.drawable.dxb_portable_epg_left_btn_n);
			mbuPrev.setFocusable(true);
		}
	}
	
	private void updateNextBt(Cursor cursor)
	{
		DxbLog(I, "EPG_updateNextBt()");
		
		if (miCount <= 0)
		{
			mbuNext.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_d);
			mbuNext.setFocusable(false);
		}
		else if(miEpgUpdate_day >= 0)
		{
			requestEPGUpdate(miEpgUpdate_day+1);
			if(mNextEPG != null)
			{
				mNextEPG.close();
				mNextEPG = null;
			}

			mNextEPG = getPlayer().getEPG_Schedule();
			DxbLog(D, "mNextEPG.getCount() = " + mNextEPG.getCount());
			//Log.d(TAG, "mNextEPG=" + mNextEPG);
			if(mNextEPG == null)
			{
				mbuNext.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_d);
				mbuNext.setFocusable(false);
			}
			else if(mNextEPG.getCount() <= 0)
			{
				mbuNext.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_d);
				mbuNext.setFocusable(false);
			}
			else
			{
				//Log.d(TAG, "mNextEPG.getCount() = " + mNextEPG.getCount());
				mbuNext.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_n);
				mbuNext.setFocusable(true);
			}
		}
	}
	
	private void updateList(Cursor cursor)
	{
		DxbLog(I, "updateList()");
		
		DxbLog(D, "miCurrent = " + miCurrent);
		if(		(cursor != null)
			&&	!cursor.isClosed()
		)
		{
			miCount = cursor.getCount();
			if(miCount <= 0)
			{
				miCurrent	= 0;
			}
			else if(miCurrent >= miCount)
			{
				miCurrent	= miCount-1;
			}
			
			Cursor c = cursor;
//			getContext().startManagingCursor(c);
			c.moveToPosition(miCurrent);

			DxbAdapter_EPG cAdapter_EPG;
			if(getDisplayWidth() > 1024)
			{
				cAdapter_EPG	= new DxbAdapter_EPG(getContext(), R.layout.dxb_epg_row_60px, c, new String[] {}, new int[] {});
			}
			else
			{
				cAdapter_EPG	= new DxbAdapter_EPG(getContext(), R.layout.dxb_epg_row_40px, c, new String[] {}, new int[] {});
			}
			
			mlvEPG_List.setAdapter(cAdapter_EPG);
			if(cAdapter_EPG.getCount() < 1)
			{
				mspinner_ServiceList.requestFocus();
				mivNoData.setVisibility(View.VISIBLE);
			}
			else
			{
				DxbLog(D, "miCurrent = " + miCurrent);
				mlvEPG_List.setSelection(miCurrent);
				mlvEPG_List.requestFocus();
				mivNoData.setVisibility(View.INVISIBLE);
			}
		}
	}

	public void setColumnIndex(Cursor cursor)
	{
		if(cursor != null)
		{
			miIndex_startMJD		= cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_START_MJD);
			
			miIndex_startHH		= cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_START_HOUR);
			miIndex_startMM		= cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_START_MINUTE);
			
			miIndex_durationHH	= cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_DURATION_HOUR);
			miIndex_durationMM	= cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_DURATION_MINUTE);
			
			miIndex_name			= cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_EVT_NAME);
			miIndex_event_text	= cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_EVT_TEXT);
			
			miIndex_TableID 	= cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_TABLE_ID);

			if (getPlayer().ePLAYER == DxbPlayer.PLAYER.ISDBT)
			{
				miIndex_event_extern	= cursor.getColumnIndexOrThrow(ChannelManager.KEY_EPG_EVT_EXTERN);
			}
		}
		else
		{
			DxbLog(D, "if(cursor == null)");
		}
	}
	
	private String getEPG_Time(Cursor cursor)
	{
		int	iStartHH	= cursor.getInt(miIndex_startHH);
		int	iStartMM	= cursor.getInt(miIndex_startMM);
		
		int iDurationHH	= cursor.getInt(miIndex_durationHH);
		int	iDurationMM	= cursor.getInt(miIndex_durationMM);
		
		int	iOverHH		= (iStartMM + iDurationMM) / 60;
				
		int	iEndHH		= (iStartHH + iDurationHH + iOverHH) % 24;
		int	iEndMM		= (iStartMM + iDurationMM) % 60;
			
		String return_Time = String.format(	"%02d:%02d~%02d:%02d"
											, iStartHH
											, iStartMM
											, iEndHH
											, iEndMM);

		return	return_Time;
	}

	private String getEPG_Event(Cursor cursor)
	{
		String	epgDetail	= "";
		
		if(isValidateCussor(cursor))
		{
			epgDetail	= getEvent(cursor);
			
			if(epgDetail.length() <= 0)
			{
				epgDetail	= getEvent_Extern(cursor);
			}
			else
			{
				epgDetail	= epgDetail + "\n" + getEvent_Extern(cursor);
			}
		}
		
		return	epgDetail;
	}

	private String getTitle(Cursor cursor)
	{
		String 	returnName	= "";
		
		if(isValidateCussor(cursor))
		{
			String	getName	= cursor.getString(miIndex_name);
			DxbLog(I, "getTitle(len : " + getName.length() + " --> " + getName);
			
			int	matLen;
			if(isSTB())
			{
				matLen	= 36;	// TV3 (2008/07/15 21:24~21:53) - STB
			}
			else
			{
				matLen	= 41;
			}
			
			if(getName.length() > matLen)
			{
				returnName	= getName.substring(0, (matLen-3)) + " ... ";
			}
			else
			{
				returnName	= getName;
			}
		}
		
		return	returnName;
	}

	private String getTitle_all(Cursor cursor)
	{
		String 	returnName	= "";
		
		if(isValidateCussor(cursor))
		{
			String	getName	= cursor.getString(miIndex_name);
			DxbLog(I, "getTitle(len : " + getName.length() + " --> " + getName);
			
			
			returnName	= getName;
		}
		
		return	returnName;
	}
	
	private String getEvent(Cursor cursor)
	{
		if(isValidateCussor(cursor))
		{
			return	cursor.getString(miIndex_event_text);
		}
		else
		{
			return "";
		}
	}
	
	private String getEvent_Extern(Cursor cursor)
	{
		DxbLog(I, "getEvent_Extern()");

		if(isValidateCussor(cursor))
		{
			if(cursor.isNull(miIndex_event_extern))
			{
				return "";
			}
			else
			{
				return cursor.getString(miIndex_event_extern);
			}
		}
		else
		{
			return "";
		}
	}
	
	private void requestEPGUpdate(int state)
	{
		DxbLog(I, "requestEPGUpdate(state = " + state + ")");

		if(getPlayer().isValid())
		{
			getPlayer().requestEPGUpdate(state);
		}
	}	
	
	private void setEventData(Cursor cursor)
	{
		if(isValidateCussor(cursor))
		{
			int MJD, startHH, startMM, durationHH, durationMM;
            DateTimeData datetime;
			long	lUTC_Start, lUTC_End;
			
			datetime = getPlayer().getCurrentDateTime();
			
			MJD			= DxbUtil.getMJD(cursor.getInt(miIndex_startMJD), cursor.getInt(miIndex_startHH), datetime.iPolarity, datetime.iHourOffset);
			startHH		= DxbUtil.getHour(cursor.getInt(miIndex_startHH), datetime.iPolarity, datetime.iHourOffset);
			startMM		= DxbUtil.getMinute(cursor.getInt(miIndex_startMM), datetime.iPolarity, datetime.iMinOffset);
			durationHH	= cursor.getInt(miIndex_durationHH);
			durationMM	= cursor.getInt(miIndex_durationMM);		
			
			Date	date	= new Date();
			date.setYear(DxbUtil.getMJD_YY(MJD) - 1900);
			date.setMonth(DxbUtil.getMJD_MM(MJD) - 1);
			date.setDate(DxbUtil.getMJD_DD(MJD));
			date.setHours(startHH);
			date.setMinutes(startMM);
			date.setSeconds(0);
			lUTC_Start	= date.getTime();
			
			date.setHours(startHH + durationHH);
			date.setMinutes(startMM + durationMM);
			lUTC_End	= date.getTime();
			
			getPlayer().setEventData_Time(lUTC_Start, lUTC_End);
		}
	}

	@Override
	public void onChannelChange()
	{
		DxbLog(I, "onChannelChange()");
		
		onUpdateEPG_PF();
		updateList();
		onUpdateScreen();
	}

	@Override
	protected void onUpdateScreen()
	{
		DxbLog(I, "onUpdateScreen()");
		
//		ImageView imageView = null;
		int	image = 0;

		if (!getPlayer().isPlaying())
		{
			return;
		}
		
		if(getPlayer().getChannelCount() <= 0)
		{
			image	= R.drawable.dxb_portable_channel_no_channel_img;
		}
		else
		{
			if(getSignalLevel() < 0 && getPlayer().getServiceType() != 2)
			{
				image	= R.drawable.dxb_portable_channel_weak_signal_img;
				getPlayer().setPlayPause(getPlayer().getServiceType(), false);
			}
			else if(	getPlayer().isParentalLock()
					&&	!getPlayer().isUnlockParental()
			)
			{
				image	= R.drawable.dxb_portable_lock_img;
				
				if(getPlayer().getRecordState() != DxbPlayer.REC_STATE.STOP)
				{
					getContext().mFullView.setRecordStartStop();
				}
				
				getPlayer().setPlayPause(getPlayer().getServiceType(), false);
			}
			else
			{
				if(getPlayer().isRadio())
				{
					//image	= R.drawable.dxb_portable_channel_radio_img;
				}
			}
		}
		
//		imageView	= (ImageView)getContext().findViewById(R.id.epg_image);
//		if (imageView != null)
		{
			if(image == 0)
			{
//				imageView.setVisibility(android.view.View.GONE);
				if(getPlayer().isVideoPlay())
				{
					mvVideo.setBackgroundColor(0x00000000);
					DxbLog(D, "mvVideo.setBackgroundColor(0x00000000)");
				}
				else
				{
					mvVideo.setBackgroundColor(0xFF101010);
					DxbLog(D, "mvVideo.setBackgroundColor(0xFF101010)");
				}
			}
			else
			{
//				imageView.setBackgroundResource(image);
//				imageView.setVisibility(android.view.View.VISIBLE);
				mvVideo.setBackgroundColor(0xFF101010);
				DxbLog(D, "mvVideo.setBackgroundColor(0xFF101010)");
			}
		}
	}


	@Override
	protected void onUpdateEPG_PF()
	{
		DxbLog(I, "onUpdateEPG_PF()");
		
//		getPlayer().setCurrentRating(0);
        
		if(getPlayer().isPlaying())
		{
			getPlayer().setEPG_PF();
/*       
			if (getPlayer().isCurrEPG())
			{
				int	iRating	= getPlayer().getCurrentEPG_Rating();
				getPlayer().setCurrentRating(iRating);
			}
*/	
			onUpdateScreen();
		}
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		DxbLog(I, "onKeyDown(keyCode=" + keyCode + ", event=" + event + ")");

		if(		(keyCode == KeyEvent.KEYCODE_BACK)
			||	(keyCode == KeyEvent.KEYCODE_ESCAPE)
		)
		{
			close_EPGView();
			setState(false, VIEW_MAIN);
			
			return true;
		}

		switch(keyCode)
		{
			case KeyEvent.KEYCODE_0:
			{
				close_EPGView();
				setState(false, VIEW_MAIN);
				
				return true;
			}
	
			case KeyEvent.KEYCODE_PROG_RED:
			case KeyEvent.KEYCODE_TV:
			{
				if(getPlayer().getChannelCount() > 0)
				{
					startScheduler(0);
				}
				
				return true;
			}
				
			case KeyEvent.KEYCODE_PROG_GREEN:
			case KeyEvent.KEYCODE_GUIDE:
			{
				if(getPlayer().getChannelCount() > 0)
				{
					startScheduler(1);
				}
				
				return true;
			}
		}
		
		return false;
	}

	@Override
	protected String getClassName() {
		return "[[[DxbView_EPG]]]";
	}

	@Override
	protected void VideoOutput(DxbPlayer player)
	{
		DxbLog(I, "VideoOutput()");

		if(!getPlayer().isVideoPlay())
		{
			DxbLog(D, "mvVideo.setBackgroundColor(0xFF101010)");
			mvVideo.setBackgroundColor(0xFF101010);
		}
		else
		{
			if(		!getPlayer().isParentalLock()
				||	getPlayer().isUnlockParental()
			)
			{
				DxbLog(D, "mvVideo.setBackgroundColor(0x00000000)");
				mvVideo.setBackgroundColor(0x00000000);
			}
		}
	}

	private class DxbAdapter_EPG extends SimpleCursorAdapter {
		private static final String TAG = "[[[DxbAdapter_EPG]]] ";
		
		private Cursor	cursorEPG;
		
		class ViewHolder {
			TextView	textTime;
			TextView	textName;
		}
		
		class QueryHandler extends AsyncQueryHandler
		{
			QueryHandler(ContentResolver res)
			{
				super(res);
			}
		}

		public DxbAdapter_EPG(Context context, int layout, Cursor c, String[] from, int[] to)
		{
			super(context, layout, c, from, to);
		
			Log.i(TAG, "DxbAdapter_EPG() - Manager_Setting.g_Information.cEPG.iCount="+miCount);
			
			cursorEPG = c;
			
			if (miCount <= 0)
			{
				mtvName_Current.setText("");
				mtvDetail_Current.setText("");
			}
			
			setColumnIndex(cursorEPG);
		}

		@Override
		public View newView(Context context, Cursor cursor, ViewGroup parent)
		{
			View v = super.newView(context, cursor, parent);
			ViewHolder hEPG = new ViewHolder();

			Typeface typeface = Typeface.createFromFile("/system/fonts/DroidSansFallback_DxB.ttf");

			hEPG.textTime = (TextView) v.findViewById(R.id.epg_row_time);
			hEPG.textName = (TextView) v.findViewById(R.id.epg_row_name);
		
			hEPG.textTime.setTypeface(typeface);
			hEPG.textName.setTypeface(typeface);

			v.setTag(hEPG);
			
			return v;
		}

		@Override
		public void bindView(View view, Context context, Cursor cursor)
		{
			int	position	= cursor.getPosition();
			//Log.i(TAG, "bindView(position=" + position + ")");
			if(position == 0)
			{
				setDisplayDate(cursor);
			}
			
			ViewHolder hEPG = (ViewHolder) view.getTag();
			String	sName	= getTitle(cursor);
			
			hEPG.textTime.setText(getEPG_Time(cursor));
			hEPG.textName.setText(sName);
			
			hEPG.textTime.setTextColor(Color.rgb(224, 224, 224));
			hEPG.textName.setTextColor(Color.rgb(224, 224, 224));
		}
	}
	
	private void close_EPGView()
	{
		getMainHandler().removeCallbacks(mRunnable_updateList);
	}
}
