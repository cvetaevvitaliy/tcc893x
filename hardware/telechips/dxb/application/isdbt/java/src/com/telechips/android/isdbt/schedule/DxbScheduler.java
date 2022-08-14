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
package com.telechips.android.isdbt.schedule;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;

import com.telechips.android.isdbt.R;
import com.telechips.android.isdbt.player.DxbChannelData;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.DatePickerDialog;
import android.app.Dialog;
import android.app.TimePickerDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.IntentFilter;
import android.database.Cursor;
import android.os.Bundle;
import android.os.Handler;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.KeyEvent;
import android.view.View;
import android.view.Window;
import android.view.View.OnFocusChangeListener;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.Button;
import android.widget.DatePicker;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.TimePicker;
import android.widget.AdapterView.OnItemClickListener;


public class DxbScheduler extends Activity
{
	public static DxbScheduler schedulerActivity;

	class Component
	{
		Com_Add		add		= new Com_Add();
		Com_List	list	= new Com_List();
	}
	
	class Com_Add
	{
		//	Layout
		ViewGroup	vgAdd;
		
		//	Title
		TextView	tvTitle;
		
		//	Info
		TextView	tvInfo_ChannelType;
		TextView	tvInfo_ChannelNumber;
		TextView	tvInfo_ChannelName;
		TextView	tvInfo_StartDate;
		TextView	tvInfo_StartTime;
		TextView	tvInfo_EndTime;
		TextView	tvInfo_Repeat;
		TextView	tvInfo_Mode;
		
		//	Channel Type
		TextView	tvType;
		View		vgType_left;
		View		vgType_right;
		ImageView	bType_left;
		ImageView		bType_right;
		
		//	Channel No.
		TextView	tvNo;
		View		vgNo_left;
		View		vgNo_right;
		ImageView	bNo_left;
		ImageView	bNo_right;
		
		//	Channel Name
		TextView	tvName;
		
		//	Date / Time
		TextView	tvDate;
		TextView	tvTime_Start;
		TextView	tvTime_End;
		
		//	Repeat
		TextView	tvRepeat;
		View		vgRepeat_left;
		View		vgRepeat_right;
		ImageView	bRepeat_left;
		ImageView	bRepeat_right;
		
		//	Mode
		TextView	tvMode;
		View		vgMode_left;
		View		vgMode_right;
		ImageView	bMode_left;
		ImageView	bMode_right;
		
		//	Action
		Button		bOk;
		Button		bExit;
	}
	
	class Com_List
	{
		//	Title
			TextView	tvTime;

		//	ListView
			ListView	lvSchedule;
		
		//	Action
			Button		bAdd;
			Button		bEdit;
			Button		bDel;
			Button		bExit;
	}
	
	class Information
	{
		Context	mContext	= null;
		STATE	eState		= STATE.NULL;
		
		//	Display
		int	iDisplayWidth;
		int	iDisplayHeight;
		float	fDensity;
		
		//	Time
			Date	dateBase_TOT;
			Date	dateBase_System;
			Date	dateCurrent_System;
			Date	dateStart;
			Date	dateEnd;
			
		//	Format
			SimpleDateFormat	formatDate;
			SimpleDateFormat	formatTime;
		
		Scheduler	cScheduler	= new Scheduler();
		
		ScheduleDB	dbSchedule	= null;
		Cursor		curSchedule	= null;
		int			iCount_Schedule	= 0;
		
		int	iCount_Event;
		int	iIndex_CurrentEvent	= -1;
		int	iID_CurrentEvent;
		int	iID_EditEvent;
		
		//	Editor - control
			TextView	tvCurrent_focus	= null;
		
		//	PickerDialog
			DatePickerDialog	pickerDate;
			TimePickerDialog	pickerTime_Start;
			TimePickerDialog	pickerTime_End;
		
		//	DataBase
			DxbEventData	eventData;
			ArrayList<DxbChannelData> tvList;
			ArrayList<DxbChannelData> radioList;
			
		//	IntentFilter
			IntentFilter commandFilter	= null;
	}

	// DxbScheduler State
	public enum STATE
	{
		LIST,
		ADD,
		EDIT,
		NULL
	}
	
	Handler mHandler_Main = new Handler();
	
	Component	gComponent;
	Information	gInformation;
	
	static Date	dateCurrent_TOT;

	
	//	Type - Time
		private final int TYPE_TIME_START	= 0;
		private final int TYPE_TIME_END		= 1;
	
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		DxbLog(I, "onCreate()");
		
		super.onCreate(savedInstanceState);
		
		getWindow();
		this.requestWindowFeature(Window.FEATURE_NO_TITLE);
		//getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN | 0x80000000, WindowManager.LayoutParams.FLAG_FULLSCREEN | 0x80000000); 
		//getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		
		setContentView(R.layout.dxb_schedule);
		
		setComponent();
		setInformation();
		
		setSystemData();
		setFormat();
		
		setTextSize();
		setListener();
		
		initDialog();
		
		Bundle bundle	= getIntent().getExtras();
		setBundleData(bundle);
		setDefaultValue(gInformation.eventData);
	}

	protected void onResume()
	{
		DxbLog(I, "onResume()");
		
		super.onResume();

		displayScreen();
		mHandler_Main.postDelayed(mRunnable_Signal, 1000);
	}
	
	protected void onPause()
	{
		DxbLog(I, "onPause()");
		
		super.onPause();
		
		mHandler_Main.removeCallbacks(mRunnable_Signal);
		if(gInformation.cScheduler != null)
		{
			//gInformation.cScheduler.release(this);
		}
	}
	
	private void setComponent()
	{
		DxbLog(I, "setComponent()");
		
		gComponent	= new Component();
		
		/*	Add	*/
		{
			//	Layout
			gComponent.add.vgAdd		= (ViewGroup)findViewById(R.id.schedule_add);
			
			//	Title
			gComponent.add.tvTitle		= (TextView)findViewById(R.id.schedule_add_title);
			
			//	Info
			gComponent.add.tvInfo_ChannelType	= (TextView)findViewById(R.id.schedule_add_info_type);
			gComponent.add.tvInfo_ChannelNumber	= (TextView)findViewById(R.id.schedule_add_info_no);
			gComponent.add.tvInfo_ChannelName	= (TextView)findViewById(R.id.schedule_add_info_name);
			gComponent.add.tvInfo_StartDate		= (TextView)findViewById(R.id.schedule_add_info_date);
			gComponent.add.tvInfo_StartTime		= (TextView)findViewById(R.id.schedule_add_info_start_time);
			gComponent.add.tvInfo_EndTime		= (TextView)findViewById(R.id.schedule_add_info_end_time);
			gComponent.add.tvInfo_Repeat		= (TextView)findViewById(R.id.schedule_add_info_repeat);
			gComponent.add.tvInfo_Mode			= (TextView)findViewById(R.id.schedule_add_info_mode);
			
			//	Channel Type
			gComponent.add.bType_left	= (ImageView)findViewById(R.id.schedule_add_type_left);
			gComponent.add.tvType		= (TextView)findViewById(R.id.schedule_add_type_text);
			gComponent.add.bType_right	= (ImageView)findViewById(R.id.schedule_add_type_right);
			
			//	Channel No.
			gComponent.add.bNo_left		= (ImageView)findViewById(R.id.schedule_add_no_left);
			gComponent.add.tvNo			= (TextView)findViewById(R.id.schedule_add_no_text);
			gComponent.add.bNo_right	= (ImageView)findViewById(R.id.schedule_add_no_right);
			
			//	Channel Name
			gComponent.add.tvName		= (TextView)findViewById(R.id.schedule_add_name_text);
			
			//	Date / Time
			gComponent.add.tvDate		= (TextView)findViewById(R.id.schedule_add_date);
			gComponent.add.tvTime_Start	= (TextView)findViewById(R.id.schedule_add_start_time);
			gComponent.add.tvTime_End	= (TextView)findViewById(R.id.schedule_add_end_time);
			
			//	Repeat
			gComponent.add.bRepeat_left		= (ImageView)findViewById(R.id.schedule_add_repeat_left);
			gComponent.add.tvRepeat			= (TextView)findViewById(R.id.schedule_add_repeat_text);
			gComponent.add.bRepeat_right	= (ImageView)findViewById(R.id.schedule_add_repeat_right);
			
			//	Mode
			gComponent.add.bMode_left	= (ImageView)findViewById(R.id.schedule_add_mode_left);
			gComponent.add.tvMode		= (TextView)findViewById(R.id.schedule_add_mode_text);
			gComponent.add.bMode_right	= (ImageView)findViewById(R.id.schedule_add_mode_right);
			
			//	Action
			gComponent.add.bOk			= (Button)findViewById(R.id.schedule_add_b_confirm);
			gComponent.add.bExit		= (Button)findViewById(R.id.schedule_add_b_exit);
			
			//	left/right Button
				gComponent.add.vgType_left	= (View)findViewById(R.id.schedule_add_type_left_g);
				gComponent.add.vgType_right	= (View)findViewById(R.id.schedule_add_type_right_g);
				gComponent.add.vgNo_left	= (View)findViewById(R.id.schedule_add_no_left_g);
				gComponent.add.vgNo_right	= (View)findViewById(R.id.schedule_add_no_right_g);
				gComponent.add.vgRepeat_left	= (View)findViewById(R.id.schedule_add_repeat_left_g);
				gComponent.add.vgRepeat_right	= (View)findViewById(R.id.schedule_add_repeat_right_g);
				gComponent.add.vgMode_left	= (View)findViewById(R.id.schedule_add_mode_left_g);
				gComponent.add.vgMode_right	= (View)findViewById(R.id.schedule_add_mode_right_g);
		}
		
		/*	List	*/
		{
			//	Title
			gComponent.list.tvTime		= (TextView)findViewById(R.id.schedule_list_time);

			//	List
			gComponent.list.lvSchedule	= (ListView)findViewById(R.id.schedule_list_list);
			
			//	Action
			gComponent.list.bAdd	= (Button)findViewById(R.id.schedule_list_b_add);
			gComponent.list.bEdit	= (Button)findViewById(R.id.schedule_list_b_edit);
			gComponent.list.bDel	= (Button)findViewById(R.id.schedule_list_b_del);
			gComponent.list.bExit	= (Button)findViewById(R.id.schedule_list_b_exit);
		}
	}
	
	private void setInformation()
	{
		DxbLog(I, "setInformation()");
		
		gInformation	= new Information();
		gInformation.mContext	= this;
		
		schedulerActivity	= DxbScheduler.this;
	}
	
	private void setBundleData(Bundle bundle)
	{
		DxbLog(I, "setBundleData()");
		
		gInformation.eventData	= bundle.getParcelable("eventInfo");
		gInformation.tvList		= bundle.getParcelableArrayList("tv_list");
		gInformation.radioList	= bundle.getParcelableArrayList("radio_list");
		
		gInformation.dateBase_TOT		= new Date(gInformation.eventData.lUTC_baseTOT);
		gInformation.dateStart			= new Date(gInformation.eventData.lUTC_Start);
		gInformation.dateEnd			= new Date(gInformation.eventData.lUTC_End);
		
		dateCurrent_TOT	= new Date(gInformation.eventData.lUTC_baseTOT);
		
		DxbLog(D, "gInformation.eventData.iChannelType=" + gInformation.eventData.iChannelType);
		DxbLog(D, "gInformation.eventData.iIndexService=" + gInformation.eventData.iIndexService);
	}
	
	private void setSystemData()
	{
		DxbLog(I, "setSystemData()");
		
		Display display = ((WindowManager)getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
		
		gInformation.iDisplayWidth	= display.getWidth();
		gInformation.iDisplayHeight	= display.getHeight();
		
		DisplayMetrics displayMetrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
		gInformation.fDensity	= displayMetrics.density;

		long	lSystemTime	= System.currentTimeMillis();
		gInformation.dateBase_System	= new Date(lSystemTime);
		gInformation.dateCurrent_System	= new Date(lSystemTime);
	}
	
	private void setFormat()
	{
		DxbLog(I, "setFormat()");
		
		gInformation.formatDate	= new SimpleDateFormat("MM/dd/yyyy");
		gInformation.formatTime	= new SimpleDateFormat("HH:mm");
	}
    
    private void initDialog()
    {
    	gInformation.pickerDate			= new DatePickerDialog(this, ListenerOnDateSet, 0, 0, 0);
    	gInformation.pickerTime_Start	= new TimePickerDialog(this, ListenerOnTimeSet_start, 0, 0, false);
    	gInformation.pickerTime_End		= new TimePickerDialog(this, ListenerOnTimeSet_end, 0, 0, false);
    }

	private void setDefaultValue(DxbEventData eventData)
	{
		DxbLog(I, "setDefaultValue()");
		
		if(eventData.iAction == 0)
		{
			setState(STATE.ADD);
		}
		else// if(eventData.iAction == 1)
		{
			setState(STATE.LIST);
		}
		
		update_currentTime();
	}
	
	private void resetButton()
	{
		//	Hide - left/right	Button
		gComponent.add.vgType_left.setVisibility(View.INVISIBLE);
		gComponent.add.vgType_right.setVisibility(View.INVISIBLE);
		gComponent.add.vgNo_left.setVisibility(View.INVISIBLE);
		gComponent.add.vgNo_right.setVisibility(View.INVISIBLE);
		gComponent.add.vgRepeat_left.setVisibility(View.INVISIBLE);
		gComponent.add.vgRepeat_right.setVisibility(View.INVISIBLE);
		gComponent.add.vgMode_left.setVisibility(View.INVISIBLE);
		gComponent.add.vgMode_right.setVisibility(View.INVISIBLE);
		
		//	Focus	- TextView
		gComponent.add.tvType.setFocusable(true);
		gComponent.add.tvNo.setFocusable(true);
		gComponent.add.tvRepeat.setFocusable(true);
		gComponent.add.tvMode.setFocusable(true);
		
		gInformation.tvCurrent_focus	= null;
	}
	
	private void update_currentTime()
	{
		dateCurrent_TOT.setTime(gInformation.dateBase_TOT.getTime() + getUTC_System_gap());
		
		gComponent.list.tvTime.setText(gInformation.formatTime.format(dateCurrent_TOT));
	}
	
	Runnable mRunnable_Signal = new Runnable()
	{
		public void run()
		{
			//DxbLog(I, "run()");
			
			try
			{
				update_currentTime();
				/*if( dateCurrent_TOT.getSeconds() == 0 )
				{
					mHandler_Main.postDelayed(mRunnable_Signal, 1000*60);
				}
				else*/
				{
					mHandler_Main.postDelayed(mRunnable_Signal, 1000);
				}
				gInformation.dateCurrent_System.setTime(System.currentTimeMillis());
				DxbLog(D,	"[" + dateCurrent_TOT.getTime() + "]   " + dateCurrent_TOT.getHours() + ":" + dateCurrent_TOT.getMinutes() + ":" + dateCurrent_TOT.getSeconds() + 
							", [" + gInformation.dateCurrent_System.getTime() + "]   " + gInformation.dateCurrent_System.getHours() + ":" + gInformation.dateCurrent_System.getMinutes() + ":" + gInformation.dateCurrent_System.getSeconds());
			}
			catch(IllegalStateException e)
			{
				e.printStackTrace();
			}
		}
	};
	
	private long getUTC_System_gap()
	{
		return (System.currentTimeMillis() - gInformation.dateBase_System.getTime());
	}
	
	private void setTextSize()
	{
		DxbLog(I, "setTextSize()");
		
		/*	Add	*/
		{
			//	Title
			gComponent.add.tvTitle.setTextSize(25 * gInformation.iDisplayWidth / gInformation.fDensity / 800);
			
			//	Info
			float	infoTextSize	= 18 * gInformation.iDisplayWidth / gInformation.fDensity / 800;
			gComponent.add.tvInfo_ChannelType.setTextSize(infoTextSize);
			gComponent.add.tvInfo_ChannelNumber.setTextSize(infoTextSize);
			gComponent.add.tvInfo_ChannelName.setTextSize(infoTextSize);
			gComponent.add.tvInfo_StartDate.setTextSize(infoTextSize);
			gComponent.add.tvInfo_StartTime.setTextSize(infoTextSize);
			gComponent.add.tvInfo_EndTime.setTextSize(infoTextSize);
			gComponent.add.tvInfo_Repeat.setTextSize(infoTextSize);
			gComponent.add.tvInfo_Mode.setTextSize(infoTextSize);
			
			//	Data
			gComponent.add.tvType.setTextSize(infoTextSize);
			gComponent.add.tvNo.setTextSize(infoTextSize);
			gComponent.add.tvName.setTextSize(infoTextSize);
			gComponent.add.tvDate.setTextSize(infoTextSize);
			gComponent.add.tvTime_Start.setTextSize(infoTextSize);
			gComponent.add.tvTime_End.setTextSize(infoTextSize);
			gComponent.add.tvRepeat.setTextSize(infoTextSize);
			gComponent.add.tvMode.setTextSize(infoTextSize);
			
			//	Action
			infoTextSize	= 15 * gInformation.iDisplayWidth / gInformation.fDensity / 800;
			((TextView)findViewById(R.id.schedule_add_tv_confirm)).setTextSize(infoTextSize);
			((TextView)findViewById(R.id.schedule_add_tv_exit)).setTextSize(infoTextSize);
		}
		
		/*	List	*/
		{
			//	Title
			float	infoTextSize	= 30 * gInformation.iDisplayWidth / gInformation.fDensity / 800;
			((TextView)findViewById(R.id.schedule_list_title_title)).setTextSize(infoTextSize);
			infoTextSize	= 15 * gInformation.iDisplayWidth / gInformation.fDensity / 800;
			gComponent.list.tvTime.setTextSize(infoTextSize);
			
			//	Menu
			infoTextSize	= 20 * gInformation.iDisplayWidth / gInformation.fDensity / 800;
			((TextView)findViewById(R.id.schedule_list_title_event)).setTextSize(infoTextSize);
			((TextView)findViewById(R.id.schedule_list_title_program)).setTextSize(infoTextSize);
			((TextView)findViewById(R.id.schedule_list_title_date)).setTextSize(infoTextSize);
			((TextView)findViewById(R.id.schedule_list_title_time)).setTextSize(infoTextSize);
			((TextView)findViewById(R.id.schedule_list_title_repeat)).setTextSize(infoTextSize);
			((TextView)findViewById(R.id.schedule_list_title_status)).setTextSize(infoTextSize);
			
			//	Action
			infoTextSize	= 15 * gInformation.iDisplayWidth / gInformation.fDensity / 800;
			((TextView)findViewById(R.id.schedule_list_tv_add)).setTextSize(infoTextSize);
			((TextView)findViewById(R.id.schedule_list_tv_edit)).setTextSize(infoTextSize);
			((TextView)findViewById(R.id.schedule_list_tv_del)).setTextSize(infoTextSize);
			((TextView)findViewById(R.id.schedule_list_tv_exit)).setTextSize(infoTextSize);
		}
	}
	
	private void setListener()
	{
		DxbLog(I, "setListener()");
		
		/*	Add	*/
		{
			//	setClick - left/right Button
			{
				//	Channel Type
				gComponent.add.bType_left.setOnClickListener(ListenerOnClick_Button_LeftRight);
				gComponent.add.bType_right.setOnClickListener(ListenerOnClick_Button_LeftRight);
			
				//	Channel No.
				gComponent.add.bNo_left.setOnClickListener(ListenerOnClick_Button_LeftRight);
				gComponent.add.bNo_right.setOnClickListener(ListenerOnClick_Button_LeftRight);
			
				//	Repeat
				gComponent.add.bRepeat_left.setOnClickListener(ListenerOnClick_Button_LeftRight);
				gComponent.add.bRepeat_right.setOnClickListener(ListenerOnClick_Button_LeftRight);
				
				//	Mode
				gComponent.add.bMode_left.setOnClickListener(ListenerOnClick_Button_LeftRight);
				gComponent.add.bMode_right.setOnClickListener(ListenerOnClick_Button_LeftRight);
			}
			
			//	setClick - TextView
			{
				gComponent.add.tvType.setOnClickListener(ListenerOnClick_TextView);
				gComponent.add.tvNo.setOnClickListener(ListenerOnClick_TextView);
				
				//	Date / Time
				gComponent.add.tvDate.setOnClickListener(ListenerOnClick_TextView);
				gComponent.add.tvTime_Start.setOnClickListener(ListenerOnClick_TextView);
				gComponent.add.tvTime_End.setOnClickListener(ListenerOnClick_TextView);
				
				gComponent.add.tvRepeat.setOnClickListener(ListenerOnClick_TextView);
				gComponent.add.tvMode.setOnClickListener(ListenerOnClick_TextView);
			}
			
			//	setClick - action Button
			{
				//	Action
				gComponent.add.bOk.setOnClickListener(ListenerOnClick_Button_Action);
				gComponent.add.bExit.setOnClickListener(ListenerOnClick_Button_Action);
			}
			
			//	setFocusChange - left/right Button
			{
				gComponent.add.bType_left.setOnFocusChangeListener(ListenerOnFocusChange_LeftRight);
				gComponent.add.bType_right.setOnFocusChangeListener(ListenerOnFocusChange_LeftRight);
				
				gComponent.add.bNo_left.setOnFocusChangeListener(ListenerOnFocusChange_LeftRight);
				gComponent.add.bNo_right.setOnFocusChangeListener(ListenerOnFocusChange_LeftRight);
				
				gComponent.add.bRepeat_left.setOnFocusChangeListener(ListenerOnFocusChange_LeftRight);
				gComponent.add.bRepeat_right.setOnFocusChangeListener(ListenerOnFocusChange_LeftRight);
				
				gComponent.add.bMode_left.setOnFocusChangeListener(ListenerOnFocusChange_LeftRight);
				gComponent.add.bMode_right.setOnFocusChangeListener(ListenerOnFocusChange_LeftRight);
			}
			
			//	setFocusChange - left/right Button
			{
				gComponent.add.tvType.setOnFocusChangeListener(ListenerOnFocusChange_TextView);
				gComponent.add.tvNo.setOnFocusChangeListener(ListenerOnFocusChange_TextView);
				gComponent.add.tvDate.setOnFocusChangeListener(ListenerOnFocusChange_TextView);
				gComponent.add.tvTime_Start.setOnFocusChangeListener(ListenerOnFocusChange_TextView);
				gComponent.add.tvTime_End.setOnFocusChangeListener(ListenerOnFocusChange_TextView);
				gComponent.add.tvRepeat.setOnFocusChangeListener(ListenerOnFocusChange_TextView);
				gComponent.add.tvMode.setOnFocusChangeListener(ListenerOnFocusChange_TextView);
			}
			
			//	setFocusChange - action Button
			{
				gComponent.add.bOk.setOnFocusChangeListener(ListenerOnFocusChange_Action);
				gComponent.add.bExit.setOnFocusChangeListener(ListenerOnFocusChange_Action);
			}
		}
		
		/*	List	*/
		{
			gComponent.list.lvSchedule.setOnItemClickListener(ListenerOnItemClick);
			gComponent.list.lvSchedule.setOnItemSelectedListener(ListenerOnItemSelected);
			//gComponent.list.lvSchedule.
			
			//	Action
			gComponent.list.bAdd.setOnClickListener(ListenerOnClick_Button_Action);
			gComponent.list.bEdit.setOnClickListener(ListenerOnClick_Button_Action);
			gComponent.list.bDel.setOnClickListener(ListenerOnClick_Button_Action);
			gComponent.list.bExit.setOnClickListener(ListenerOnClick_Button_Action);
			
			gComponent.list.bAdd.setOnFocusChangeListener(ListenerOnFocusChange_Action);
			gComponent.list.bEdit.setOnFocusChangeListener(ListenerOnFocusChange_Action);
			gComponent.list.bDel.setOnFocusChangeListener(ListenerOnFocusChange_Action);
			gComponent.list.bExit.setOnFocusChangeListener(ListenerOnFocusChange_Action);
		}
	}	
	
	OnClickListener ListenerOnClick_Button_LeftRight	= new View.OnClickListener()
	{
		public void onClick(View v)
		{
			int id = v.getId();
			DxbLog(I, "ListenerOnClick_Button_LeftRight - onClick(id=" + id + ")");
			
			switch(id)
			{
				case R.id.schedule_add_type_left:
				case R.id.schedule_add_type_right:
					setChannelType(gInformation.eventData.iChannelType + 1);
				break;
				
				case R.id.schedule_add_no_left:
					setChannelNumber(gInformation.eventData.iIndexService - 1);
				break;
				
				case R.id.schedule_add_no_right:
					setChannelNumber(gInformation.eventData.iIndexService + 1);
				break;
			
				case R.id.schedule_add_repeat_left:
					setRepeat(gInformation.eventData.iRepeatType - 1);
				break;
				
				case R.id.schedule_add_repeat_right:
					setRepeat(gInformation.eventData.iRepeatType + 1);
				break;
				
				case R.id.schedule_add_mode_left:
				case R.id.schedule_add_mode_right:
					setMode(gInformation.eventData.iMode + 1);
				break;
			}
		}
	};
	
	OnClickListener ListenerOnClick_TextView	= new View.OnClickListener()
	{
		public void onClick(View v)
		{
			int id = v.getId();
			DxbLog(I, "ListenerOnClick_TextView - onClick(id=" + id + ")");
			
			switch(id)
			{
				case R.id.schedule_add_type_text:
					gComponent.add.vgType_left.setVisibility(View.VISIBLE);
					gComponent.add.vgType_right.setVisibility(View.VISIBLE);
					gComponent.add.vgType_right.requestFocus();
					gComponent.add.tvType.setFocusable(false);
				break;
				
				case R.id.schedule_add_no_text:
					gComponent.add.vgNo_left.setVisibility(View.VISIBLE);
					gComponent.add.vgNo_right.setVisibility(View.VISIBLE);
					gComponent.add.vgNo_right.requestFocus();
					gComponent.add.tvNo.setFocusable(false);
				break;
				
				case R.id.schedule_add_date:
					gInformation.pickerDate.updateDate(gInformation.dateStart.getYear()+1900, gInformation.dateStart.getMonth()+1, gInformation.dateStart.getDate());
					gInformation.pickerDate.show();
				break;
				
				case R.id.schedule_add_start_time:
					gInformation.pickerTime_Start.updateTime(gInformation.dateStart.getHours(), gInformation.dateStart.getMinutes());
					gInformation.pickerTime_Start.show();
				break;
					
				case R.id.schedule_add_end_time:
					gInformation.pickerTime_End.updateTime(gInformation.dateEnd.getHours(), gInformation.dateEnd.getMinutes());
					gInformation.pickerTime_End.show();
				break;
				
				case R.id.schedule_add_repeat_text:
					gComponent.add.vgRepeat_left.setVisibility(View.VISIBLE);
					gComponent.add.vgRepeat_right.setVisibility(View.VISIBLE);
					gComponent.add.vgRepeat_right.requestFocus();
					gComponent.add.tvRepeat.setFocusable(false);
				break;
				
				case R.id.schedule_add_mode_text:
					gComponent.add.vgMode_left.setVisibility(View.VISIBLE);
					gComponent.add.vgMode_right.setVisibility(View.VISIBLE);
					gComponent.add.vgMode_right.requestFocus();
					gComponent.add.tvMode.setFocusable(false);
				break;
			}
		}
	};
	
	OnClickListener ListenerOnClick_Button_Action	= new View.OnClickListener()
	{
		public void onClick(View v)
		{
			int id = v.getId();
			DxbLog(I, "ListenerOnClick_Button_Action - onClick(id=" + id + ")");
			
			switch(id)
			{
				case R.id.schedule_add_b_confirm:
					{
						accessDB_insert();
					}
				break;
					
				case R.id.schedule_add_b_exit:
					{
						setState(STATE.LIST);
						displayScreen();
					}
				break;
				
				case R.id.schedule_list_b_add:
					{
						setEventData_reset();
						setState(STATE.ADD);
						
						displayScreen();
					}
				break;
				
				case R.id.schedule_list_b_edit:
					if(gInformation.iCount_Event > 0)
					{
						if(setEditData(gInformation.curSchedule, gInformation.iID_CurrentEvent, gInformation.eventData))
						{
							setState(STATE.EDIT);
							
							displayScreen();
						}
					}
				break;
				
				case R.id.schedule_list_b_del:
					{
						accessDB_delete(gInformation.iID_CurrentEvent);
					}
				break;
				
				case R.id.schedule_list_b_exit:
					{
						finish();
					}
				break;
			}
		}
	};
	
	OnItemClickListener ListenerOnItemClick = new OnItemClickListener()
	{
		public void onItemClick(AdapterView<?> parent, View v, int position, long id)
		{
			DxbLog(I, "ListenerOnItemClick-->onItemClick(position="+position+")");
			
			if(gInformation.curSchedule != null)
			{
				//if(gInformation.iIndex_CurrentEvent	!= position)
				{
					gInformation.curSchedule.moveToPosition(position);
					gComponent.list.lvSchedule.setSelection(position);
					
					gInformation.iIndex_CurrentEvent	= position;
					gInformation.iID_CurrentEvent		= gInformation.curSchedule.getInt(ScheduleDB.iColumnIndex_ID);
					
					Builder	mBuilder	= new AlertDialog.Builder(gInformation.mContext);
					mBuilder.setTitle(gInformation.mContext.getResources().getString(R.string.select));
					mBuilder.setSingleChoiceItems(R.array.schedule_select_select_entries, 0, ListenerOnClick_selectSelect);
					mBuilder.setPositiveButton(gInformation.mContext.getResources().getString(R.string.cancel), null);
					
					Dialog	mDialog	= mBuilder.create();
					mDialog.show();
				}
			}
		}
	};
	
	DialogInterface.OnClickListener ListenerOnClick_selectSelect	= new DialogInterface.OnClickListener()
	{
		public void onClick(DialogInterface dialog, int item)
		{
			DxbLog(I, "onClick(dialog="+dialog+", item="+item);
			
			if(item == 0)
			{
				if(gInformation.iCount_Event > 0)
				{
					if(setEditData(gInformation.curSchedule, gInformation.iID_CurrentEvent, gInformation.eventData))
					{
						setState(STATE.EDIT);
						
						displayScreen();
					}
				}
			}
			else if(item == 1)
			{
				accessDB_delete(gInformation.iID_CurrentEvent);
			}
			
			dialog.dismiss();
		}
	};
	
	OnItemSelectedListener ListenerOnItemSelected = new OnItemSelectedListener()
	{
		public void onItemSelected(AdapterView<?> parent, View v, int position, long id)
		{
			DxbLog(I, "ListenerOnItemSelect-->onItemSelected(position="+position+")");
			
			//if(gInformation.iIndex_CurrentEvent	!= position)
			{
				gInformation.curSchedule.moveToPosition(position);
				gComponent.list.lvSchedule.setSelection(position);
				
				gInformation.iIndex_CurrentEvent	= position;
				gInformation.iID_CurrentEvent		= gInformation.curSchedule.getInt(ScheduleDB.iColumnIndex_ID);
			}
		}

		public void onNothingSelected(AdapterView<?> arg0)
		{
		}
	};
	
	OnFocusChangeListener ListenerOnFocusChange_LeftRight = new OnFocusChangeListener()
	{
		public void onFocusChange(View v, boolean hasFocus)
		{
			DxbLog(I, "ListenerOnFocusChange-->onFocusChange(hasFocus="+hasFocus+")");
			
			gInformation.tvCurrent_focus	= null;

			int			iPosition	= -1;
			ImageView	ivNavi		= null;
			int			iID_image	= -1;
			
			int id = v.getId();
			switch(id)
			{
				case R.id.schedule_add_type_left:
					iPosition	= 0;
					ivNavi		= gComponent.add.bType_left;
				break;
			
				case R.id.schedule_add_type_right:
					iPosition	= 1;
					ivNavi		= gComponent.add.bType_right;
				break;
			
				case R.id.schedule_add_no_left:
					iPosition	= 0;
					ivNavi		= gComponent.add.bNo_left;
				break;
			
				case R.id.schedule_add_no_right:
					iPosition	= 1;
					ivNavi		= gComponent.add.bNo_right;
				break;
			
				case R.id.schedule_add_repeat_left:
					iPosition	= 0;
					ivNavi		= gComponent.add.bRepeat_left;
				break;
			
				case R.id.schedule_add_repeat_right:
					iPosition	= 1;
					ivNavi		= gComponent.add.bRepeat_right;
				break;
			
				case R.id.schedule_add_mode_left:
					iPosition	= 0;
					ivNavi		= gComponent.add.bMode_left;
				break;
			
				case R.id.schedule_add_mode_right:
					iPosition	= 1;
					ivNavi		= gComponent.add.bMode_right;
				break;
			
			}
			
			if(hasFocus)
			{
				if(iPosition == 0)
				{
					iID_image	= R.drawable.dxb_portable_schedule_left_btn_ics_f;
				}
				else if(iPosition == 1)
				{
					iID_image	= R.drawable.dxb_portable_schedule_right_btn_ics_f;
				}
			}
			else
			{
				if(iPosition == 0)
				{
					iID_image	= R.drawable.dxb_portable_schedule_left_btn_ics;
				}
				else if(iPosition == 1)
				{
					iID_image	= R.drawable.dxb_portable_schedule_right_btn_ics;
				}
			}

			if(		(iPosition > -1)
				&&	(ivNavi != null)
				&&	(iID_image > -1)
			)
			{
				ivNavi.setBackgroundResource(iID_image);
			}
		}
	};
	
	OnFocusChangeListener ListenerOnFocusChange_TextView = new OnFocusChangeListener()
	{
		public void onFocusChange(View v, boolean hasFocus)
		{
			DxbLog(I, "ListenerOnFocusChange-->onFocusChange(hasFocus="+hasFocus+")");
			
			TextView	tvChange_color	= null;
			
			if(hasFocus)
			{
				resetButton();
			}

			int id = v.getId();
			switch(id)
			{
				case R.id.schedule_add_type_text:
					tvChange_color	= gComponent.add.tvType;
				break;
				
				case R.id.schedule_add_no_text:
					tvChange_color	= gComponent.add.tvNo;
				break;
				
				case R.id.schedule_add_date:
					tvChange_color	= gComponent.add.tvDate;
				break;
				
				case R.id.schedule_add_start_time:
					tvChange_color	= gComponent.add.tvTime_Start;
				break;
				
				case R.id.schedule_add_end_time:
					tvChange_color	= gComponent.add.tvTime_End;
				break;
				
				case R.id.schedule_add_repeat_text:
					tvChange_color	= gComponent.add.tvRepeat;
				break;
				
				case R.id.schedule_add_mode_text:
					tvChange_color	= gComponent.add.tvMode;
				break;
			}
			
			if(tvChange_color != null)
			{
				if(hasFocus)
				{
					gInformation.tvCurrent_focus	= tvChange_color;
					tvChange_color.setTextColor(getResources().getColor(R.color.ics_focus));
				}
				else
				{
					tvChange_color.setTextColor(getResources().getColor(R.color.color_white));
				}
			}
		}
	};
	
	OnFocusChangeListener ListenerOnFocusChange_Action = new OnFocusChangeListener()
	{
		public void onFocusChange(View v, boolean hasFocus)
		{
			DxbLog(I, "ListenerOnFocusChange-->onFocusChange(hasFocus="+hasFocus+")");
			
			if(hasFocus)
			{
				resetButton();
			}
			
			Button		bChange_color	= null;
			int			iColor			= -1;

			int id = v.getId();
			switch(id)
			{
				case R.id.schedule_add_b_confirm:
					bChange_color	= gComponent.add.bOk;
					iColor			= R.color.color_red;
				break;
				
				case R.id.schedule_add_b_exit:
					bChange_color	= gComponent.add.bExit;
					iColor			= R.color.color_blue;
				break;
				
				case R.id.schedule_list_b_add:
					bChange_color	= gComponent.list.bAdd;
					iColor			= R.color.color_red;
				break;
				
				case R.id.schedule_list_b_edit:
					bChange_color	= gComponent.list.bEdit;
					iColor			= R.color.color_green;
				break;
				
				case R.id.schedule_list_b_del:
					bChange_color	= gComponent.list.bDel;
					iColor			= R.color.color_yellow;
				break;
				
				case R.id.schedule_list_b_exit:
					bChange_color	= gComponent.list.bExit;
					iColor			= R.color.color_blue;
				break;
			}
			
			if(bChange_color != null)
			{
				if(hasFocus)
				{
					bChange_color.setBackgroundResource(R.drawable.ics_button);
				}
				else
				{
					bChange_color.setBackgroundColor(getResources().getColor(iColor));
				}
			}
		}
	};
	
	private DatePickerDialog.OnDateSetListener ListenerOnDateSet = new DatePickerDialog.OnDateSetListener()
	{
		public void onDateSet(DatePicker view, int year, int monthOfYear, int dayOfMonth)
		{
			DxbLog(I, "onDateSet(year=" + year + ", " + "monthOfYear=" + monthOfYear + ", dayOfMonth=" + dayOfMonth + ")");
			
			gInformation.dateStart.setYear(year - 1900);
			gInformation.dateStart.setMonth(monthOfYear);
			gInformation.dateStart.setDate(dayOfMonth);
			
			gComponent.add.tvDate.setText(getText_Date());
		}
	}; 
	
	private TimePickerDialog.OnTimeSetListener ListenerOnTimeSet_start = new TimePickerDialog.OnTimeSetListener()
	{
		public void onTimeSet(TimePicker view, int hourOfDay, int minute)
		{
			DxbLog(I, "onTimeSet(hourOfDay=" + hourOfDay + ", minute=" + minute + ")");
			
			gInformation.dateStart.setHours(hourOfDay);
			gInformation.dateStart.setMinutes(minute);
			
			gComponent.add.tvTime_Start.setText(getText_Time(TYPE_TIME_START));
		}
	};
	
	private TimePickerDialog.OnTimeSetListener ListenerOnTimeSet_end = new TimePickerDialog.OnTimeSetListener()
	{
		public void onTimeSet(TimePicker view, int hourOfDay, int minute)
		{
			DxbLog(I, "onTimeSet(hourOfDay=" + hourOfDay + ", minute=" + minute + ")");
			
			gInformation.dateEnd.setHours(hourOfDay);
			gInformation.dateEnd.setMinutes(minute);
			
			gComponent.add.tvTime_End.setText(getText_Time(TYPE_TIME_END));
		}
	};

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		DxbLog(I, "onKeyDown(keyCode=" + keyCode + ", event=" + event + ")");
		
		if((keyCode == KeyEvent.KEYCODE_BACK) || (keyCode == KeyEvent.KEYCODE_ESCAPE))
		{
			boolean bReturn	= backState(keyCode, event, gInformation.eState);
			if(bReturn)
			{
				return bReturn;
			}
			else
			{
				return super.onKeyDown(keyCode, event);
			}
		}
		
		if(gInformation.eState == STATE.LIST)
		{
			boolean bReturn	= onKeyDown_List(keyCode, event);
			
			if(bReturn)
			{
				return bReturn;
			}
		}
		else if(	(gInformation.eState == STATE.ADD)
				||	(gInformation.eState == STATE.EDIT)
		)
		{
			boolean bReturn	= onKeyDown_Add(keyCode, event);
			
			if(bReturn)
			{
				return bReturn;
			}
		}
		
		return super.onKeyDown(keyCode, event);
	}
	
	private boolean onKeyDown_List(int keyCode, KeyEvent event)
	{
		DxbLog(I, "onKeyDown_List(keyCode=" + keyCode + ", event=" + event + ")");
		
		boolean	return_state	= false;
		
		switch(keyCode)
		{
			case KeyEvent.KEYCODE_PROG_RED:
			case KeyEvent.KEYCODE_TV:
			{
				setEventData_reset();
				setState(STATE.ADD);
				
				displayScreen();
				
				return_state	= true;
			}
			break;
			
			case KeyEvent.KEYCODE_PROG_GREEN:
			case KeyEvent.KEYCODE_GUIDE:
			{
				if(gInformation.iCount_Event > 0)
				{
					if(setEditData(gInformation.curSchedule, gInformation.iID_CurrentEvent, gInformation.eventData))
					{
						setState(STATE.EDIT);
						
						displayScreen();
					}
				}
				
				return_state	= true;
			}
			break;
			
			case KeyEvent.KEYCODE_DEL:
			case KeyEvent.KEYCODE_PROG_YELLOW:
			case KeyEvent.KEYCODE_BOOKMARK:
			{
				accessDB_delete(gInformation.iID_CurrentEvent);
				return_state	= true;
			}
			break;
			
			case KeyEvent.KEYCODE_PROG_BLUE:
			case KeyEvent.KEYCODE_MENU:
			{
				finish();
			}
			break;
		}
		
		
		return return_state;
	}
	
	private boolean onKeyDown_Add(int keyCode, KeyEvent event)
	{
		DxbLog(I, "onKeyDown_Add(keyCode=" + keyCode + ", event=" + event + ")");
		
		boolean	return_state	= false;
		
		switch(keyCode)
		{
			case KeyEvent.KEYCODE_DPAD_LEFT:
			case KeyEvent.KEYCODE_DPAD_RIGHT:
			{
				return_state	= onKeyDown_Editor(keyCode, event);
			}
			break;
			
			case KeyEvent.KEYCODE_PROG_RED:
			case KeyEvent.KEYCODE_TV:
			{
				accessDB_insert();
				
				return_state	= true;
			}
			break;
			
			case KeyEvent.KEYCODE_PROG_BLUE:
			case KeyEvent.KEYCODE_MENU:
			{
				setState(STATE.LIST);
				displayScreen();
				
				return_state	= true;
			}
			break;
		}
		
		return return_state;
	}

	private boolean onKeyDown_Editor(int keyCode, KeyEvent event)
	{
		DxbLog(I, "onKeyDown_Editor(keyCode=" + keyCode + ", event=" + event + ")");
		
		boolean	return_state	= false;

		int		iMove	= 0;
		if(keyCode == KeyEvent.KEYCODE_DPAD_LEFT)
		{
			iMove	= -1;
		}
		else if(keyCode == KeyEvent.KEYCODE_DPAD_RIGHT)
		{
			iMove	= 1;
		}
		else
		{
			return	return_state;
		}
		
		if(gInformation.tvCurrent_focus == gComponent.add.tvType)
		{
			setChannelType(gInformation.eventData.iChannelType + iMove);
			return_state	= true;
		}
		else if(gInformation.tvCurrent_focus == gComponent.add.tvNo)
		{
			setChannelNumber(gInformation.eventData.iIndexService + iMove);
			return_state	= true;
			
		}
		else if(gInformation.tvCurrent_focus == gComponent.add.tvDate)
		{
			setDate(iMove);
			return_state	= true;
			
		}
		else if(gInformation.tvCurrent_focus == gComponent.add.tvTime_Start)
		{
			setTime_Start(iMove);
			return_state	= true;
			
		}
		else if(gInformation.tvCurrent_focus == gComponent.add.tvTime_End)
		{
			setTime_End(iMove);
			return_state	= true;
			
		}
		else if(gInformation.tvCurrent_focus == gComponent.add.tvRepeat)
		{
			setRepeat(gInformation.eventData.iRepeatType + iMove);
			return_state	= true;
			
		}
		else if(gInformation.tvCurrent_focus == gComponent.add.tvMode)
		{
			setMode(gInformation.eventData.iMode + iMove);
			return_state	= true;
		}
		
		return return_state;
	}	
	
	private boolean backState(int keyCode, KeyEvent event, STATE eState)
	{
		boolean	return_state	= false;
		
		if(		(eState == STATE.ADD)
			||	(eState == STATE.EDIT)
		)
		{
			setState(STATE.LIST);
			displayScreen();
			
			return_state	= true;
		}
		
		return return_state;
	}

	private void setState(STATE state)
	{
		DxbLog(I, "setState(" + state + ")");
		
		if(state == STATE.ADD || state == STATE.EDIT)
		{
			resetButton();
		}
		
		gInformation.eState	= state;
	}

	protected boolean setEditData(Cursor curSchedule, int iID, DxbEventData data)
	{
		DxbLog(I, "setEditData(iID=" + iID + ")");
		
		boolean	return_state	= false;
		if(curSchedule != null)
		{
			int iCount	= curSchedule.getCount();
			if(iCount > 0)
			{
				gInformation.iID_EditEvent	= iID;
				if(gInformation.dbSchedule == null)
				{
					gInformation.dbSchedule	= gInformation.cScheduler.makeScheduleDB(gInformation.mContext);
				}
				
				if(gInformation.dbSchedule != null)
				{
					ScheduleData	tempData	= gInformation.cScheduler.getData(gInformation.dbSchedule, iID);
					if(tempData != null)
					{
						data.iIndexService	= tempData.iIndexService;
						
						data.iChannelType	= tempData.iChannelType;
						
						data.iRepeatType	= tempData.iRepeatType;
						data.iMode			= tempData.iAlarmType;
						
						long	lStart_Time		= (tempData.lUTC_Start/60000) * 60000;
						long	lRepeat_Time	= 0;
						
						if(tempData.iRepeatType == 1)
						{
							lRepeat_Time	= tempData.iCount * 24 * 60 * 60 * 1000;
						}
						else if(tempData.iRepeatType == 2)
						{
							lRepeat_Time	= tempData.iCount * 7 * 24 * 60 * 60 * 1000;
						}
						
						gInformation.dateStart.setTime(lStart_Time + lRepeat_Time);
						gInformation.dateEnd.setTime((tempData.lUTC_End/60000) * 60000);

						return_state	= true;
					}
				}
			}
		}
		
		return return_state;
	}

	private void setEditBox(DxbEventData data)
	{
		DxbLog(I, "setEditBox()");
		
		gComponent.add.tvType.setText(getText_Type());
		
		gComponent.add.tvNo.setText(getText_No());
		gComponent.add.tvName.setText(getText_Name());
		
		gComponent.add.tvDate.setText(getText_Date());
		gComponent.add.tvTime_Start.setText(getText_Time(TYPE_TIME_START));
		gComponent.add.tvTime_End.setText(getText_Time(TYPE_TIME_END));
		
		gComponent.add.tvRepeat.setText(getText_Repeat(data.iRepeatType));
		gComponent.add.tvMode.setText(getText_Mode(data.iMode));
	}

	private void displayScreen()
	{
		DxbLog(I, "displayScreen(gInformation.eState=" + gInformation.eState + ")");
		
		updateList();
		
		if(gInformation.eState	== STATE.ADD)
		{
			setVisibility_Add(true, getResources().getString(R.string.event_add));
			setVisibility_List(false);
		}
		else if(gInformation.eState	== STATE.EDIT)
		{
			setVisibility_Add(true, getResources().getString(R.string.event_edit));
			setVisibility_List(false);
		}
		else if(gInformation.eState	== STATE.LIST)
		{
			setVisibility_Add(false, null);
			setVisibility_List(true);
		}
	}
	
	private void setVisibility_List(boolean bState)
	{
		gComponent.list.lvSchedule.setFocusable(bState);
		
		gComponent.list.bAdd.setFocusable(bState);
		gComponent.list.bEdit.setFocusable(bState);
		gComponent.list.bDel.setFocusable(bState);
		gComponent.list.bExit.setFocusable(bState);
	}
	
	private void setVisibility_Add(boolean bState, String sTitle)
	{
		if(bState)
		{
			setEditBox(gInformation.eventData);
			
			gComponent.add.tvTitle.setText(sTitle);
			gComponent.add.vgAdd.setVisibility(View.VISIBLE);
			gComponent.add.tvTime_Start.requestFocus();
		}
		else
		{
			gComponent.add.vgAdd.setVisibility(View.GONE);
		}
	}
	
	private void updateList()
	{
		DxbLog(I, "updateList()");
		
		if(gInformation.dbSchedule == null)
		{
			gInformation.dbSchedule	= gInformation.cScheduler.makeScheduleDB(gInformation.mContext);
		}
		
		if(gInformation.dbSchedule != null)
		{
			if(gInformation.curSchedule == null)
			{
				//gInformation.cScheduler.reset(gInformation.mContext, gInformation.dbSchedule, dateCurrent_TOT.getTime());
			}
			else
			{
				gInformation.curSchedule.close();
			}
			
			gInformation.curSchedule	= gInformation.cScheduler.getList(gInformation.mContext, gInformation.dbSchedule);
			if(gInformation.curSchedule != null)
			{
				gInformation.iCount_Event	= gInformation.curSchedule.getCount();
				DxbLog(D, "gInformation.iCount_Event=" + gInformation.iCount_Event);
				
				DxbAdapter_Schedule	adapterSchedule;
				if(gInformation.iDisplayWidth > 800)
				{
					adapterSchedule	= new DxbAdapter_Schedule(this, R.layout.dxb_schedule_row_60px, gInformation.curSchedule, new String[] {}, new int[] {});
				}
				else
				{
					adapterSchedule	= new DxbAdapter_Schedule(this, R.layout.dxb_schedule_row_40px, gInformation.curSchedule, new String[] {}, new int[] {});
				}
				gComponent.list.lvSchedule.setAdapter(adapterSchedule);
				
				if(gInformation.iCount_Event > 0)
				{
					gComponent.list.lvSchedule.requestFocus();
				}
			}
			else
			{
				DxbLog(D, "gInformation.curSchedule == null");
				gInformation.iCount_Event	= 0;
			}
		}
	}
	
	private ArrayList<DxbChannelData> getVisible_List()
	{
		DxbLog(I, "getVisible_List()");
		
		if(		(gInformation.eventData.iChannelType == 0)
			&&	(gInformation.eventData.iCountTV > 0)
		)
		{
			return gInformation.tvList;
		}
		if(		(gInformation.eventData.iChannelType == 1)
			&&	(gInformation.eventData.iCountRadio > 0)
		)
		{
			return gInformation.radioList;
		}
		
		return null;
	}
	
	private int getVisible_List_Count()
	{
		DxbLog(I, "getVisible_List_Count(iChannelType=" + gInformation.eventData.iChannelType + ", iCountTV=" + gInformation.eventData.iCountTV + ", iCountRadio=" + gInformation.eventData.iCountRadio + ")");
		
		if(		(gInformation.eventData.iChannelType == 0)
			&&	(gInformation.eventData.iCountTV > 0)
		)
		{
			return gInformation.eventData.iCountTV;
		}
		if(		(gInformation.eventData.iChannelType == 1)
			&&	(gInformation.eventData.iCountRadio > 0)
		)
		{
			return gInformation.eventData.iCountRadio;
		}
		
		return	0;
	}
	
	private String getText_Type()
	{
		DxbLog(I, "getText_Type(gInformation.eventData.iChannelType =" + gInformation.eventData.iChannelType + ")");
		
		String	return_Text	= getResources().getString(R.string.tv);
		
		switch(gInformation.eventData.iChannelType)
		{
			/*case 0:
				return_Text	= getResources().getString(R.string.tv);
			break;*/
				
			case 1:
				return_Text	= getResources().getString(R.string.radio);
			break;
		}
		
		return	return_Text;
	}
	
	private String getText_No()
	{
		DxbLog(I, "getText_No()");
		
		ArrayList<DxbChannelData>	tempList	= getVisible_List();
		int							tempCount	= getVisible_List_Count();
		
		DxbLog(D, "tempCount=" + tempCount + ", gInformation.eventData.iIndexService=" + gInformation.eventData.iIndexService);
		if(tempCount > gInformation.eventData.iIndexService)
		{
			return	String.format("%03d", tempList.get(gInformation.eventData.iIndexService).iChannel_number);
		}
		
		return "";
	}
	
	private String getText_Name()
	{
		DxbLog(I, "getText_Name()");
		
		ArrayList<DxbChannelData>	tempList	= getVisible_List();
		int							tempCount	= getVisible_List_Count();
		
		if(tempCount > gInformation.eventData.iIndexService)
		{
			return	tempList.get(gInformation.eventData.iIndexService).sChannel_name;
		}
		
		return "";
	}
	
	private String getText_Date()
	{
		DxbLog(I, "getText_Date()");
		
		return gInformation.formatDate.format(gInformation.dateStart.getTime());
	}
	
	private String getText_Time(int iState)
	{
		DxbLog(I, "getText_Time(iState=" + iState + ")");
		
		switch(iState)
		{
			case TYPE_TIME_START:
				return gInformation.formatTime.format(gInformation.dateStart.getTime());
			
			case TYPE_TIME_END:
				return gInformation.formatTime.format(gInformation.dateEnd.getTime());
		}
		
		return "";
	}
	
	private String getText_Repeat(int iRepeat)
	{
		DxbLog(I, "getText_Repeat(iRepeat=" + iRepeat + ")");
		
		String	return_Text	= getResources().getString(R.string.once);
		
		switch(iRepeat)
		{
			/*case 0:
				return_Text	= getResources().getString(R.string.once);
			break;*/
				
			case 1:
				return_Text	= getResources().getString(R.string.daily);
			break;
				
			case 2:
				return_Text	= getResources().getString(R.string.weekly);
				break;
		}
		DxbLog(D, return_Text);
		
		return	return_Text;
	}
	
	private String getText_Mode(int iMode)
	{
		DxbLog(I, "getText_Mode(iMode=" + iMode + ")");
		
		String	return_Text	= getResources().getString(R.string.view);
		
		switch(iMode)
		{
			/*case 0:
				return_Text	= getResources().getString(R.string.once);
			break;*/
				
			case 1:
				return_Text	= getResources().getString(R.string.record);
			break;
		}
		
		if(iMode == 0)
		{
			gComponent.add.tvTime_End.setFocusable(false);
			gComponent.add.tvTime_End.setTextColor(getResources().getColor(R.color.invalid));
		}
		else if(iMode == 1)
		{
			gComponent.add.tvTime_End.setFocusable(true);
			gComponent.add.tvTime_End.setTextColor(getResources().getColor(R.color.color_white));
		}
		
		return	return_Text;
	}
	
	private void setChannelType(int iIndex)
	{
		DxbLog(I, "setChannelType(iIndex=" + iIndex + ")");
		
		int	iChange_index	= (iIndex + 2) % 2;
		
		if(		(gInformation.eventData.iCountTV > 0)
			&&	(gInformation.eventData.iCountRadio > 0)
			&&	(iChange_index != gInformation.eventData.iChannelType)
		)
		{
			gInformation.eventData.iChannelType	= iChange_index;
			gComponent.add.tvType.setText(getText_Type());
			
			setChannelNumber(0);
		}
	}
	
	private void setChannelNumber(int iIndex)
	{
		DxbLog(I, "setChannelNumber(iIndex=" + iIndex + ")");
		
		if(gInformation.eventData.iIndexService != iIndex)
		{
			int	tempCount	= getVisible_List_Count();
			if(iIndex < 0)
			{
				gInformation.eventData.iIndexService	= tempCount - 1;
			}
			else if(iIndex >= tempCount)
			{
				gInformation.eventData.iIndexService	= 0;
			}
			else
			{
				gInformation.eventData.iIndexService	= iIndex;
			}
			
			gComponent.add.tvNo.setText(getText_No());
			gComponent.add.tvName.setText(getText_Name());
		}
	}
	
	private void setDate(int iMove)
	{
		if(		(iMove == 1)
			||	(iMove == -1)
		)
		{
			long	moveTime	= gInformation.dateStart.getTime() + (iMove * 24*60*60*1000);
			gInformation.dateStart.setTime(moveTime);
			
			gComponent.add.tvDate.setText(getText_Date());
		}
	}
	
	private void setTime_Start(int iMove)
	{
		if(		(iMove == 1)
			||	(iMove == -1)
		)
		{
			long	moveTime	= gInformation.dateStart.getTime() + (iMove * 60*1000);
			gInformation.dateStart.setTime(moveTime);
			
			gComponent.add.tvTime_Start.setText(getText_Time(TYPE_TIME_START));
		}
	}
	
	private void setTime_End(int iMove)
	{
		if(		(iMove == 1)
			||	(iMove == -1)
		)
		{
			long	moveTime	= gInformation.dateEnd.getTime() + (iMove * 60*1000);
			gInformation.dateEnd.setTime(moveTime);
			
			gComponent.add.tvTime_End.setText(getText_Time(TYPE_TIME_END));
		}
	}
	
	private void setRepeat(int iRepeat)
	{
		DxbLog(I, "setRepeat(iRepeat=" + iRepeat + ")");
		
		if(gInformation.eventData.iRepeatType != iRepeat)
		{
			int	tempCount	= 3;
			if(iRepeat < 0)
			{
				gInformation.eventData.iRepeatType	= tempCount - 1;
			}
			else if(iRepeat >= tempCount)
			{
				gInformation.eventData.iRepeatType	= 0;
			}
			else
			{
				gInformation.eventData.iRepeatType	= iRepeat;
			}
			
			gComponent.add.tvRepeat.setText(getText_Repeat(gInformation.eventData.iRepeatType));
			DxbLog(D, "gInformation.eventData.iRepeatType=" + gInformation.eventData.iRepeatType);
		}
	}
	
	private void setMode(int iMode)
	{
		DxbLog(I, "setMode()");
		
		if(gInformation.eventData.iMode != iMode)
		{
			int	tempCount	= 2;
			if(iMode < 0)
			{
				gInformation.eventData.iMode	= tempCount - 1;
			}
			else if(iMode >= tempCount)
			{
				gInformation.eventData.iMode	= 0;
			}
			else
			{
				gInformation.eventData.iMode	= iMode;
			}
			gComponent.add.tvMode.setText(getText_Mode(gInformation.eventData.iMode));
		}
	}
	
	private void setEventData_reset()
	{
		gInformation.dateStart.setTime((dateCurrent_TOT.getTime()/60000) * 60000);
		gInformation.dateEnd.setTime((dateCurrent_TOT.getTime()/60000) * 60000);
	}
	
	private void accessDB_insert()
	{
		DxbLog(I, "accessDB_insert()");
		
		ScheduleData	scData	= new ScheduleData();
		{
			scData.iIndexService	= gInformation.eventData.iIndexService;
			
			scData.iChannelType	= gInformation.eventData.iChannelType;
			scData.sChannelName	= getText_Name();
			
			scData.lUTC_Current	= dateCurrent_TOT.getTime();
			scData.lUTC_Start	= gInformation.dateStart.getTime();
			//scData.lUTC_End		= (scData.lUTC_Start / 24*60*60000) + (gInformation.dateEnd.getTime() % (24*60*60000));
			scData.lUTC_End		= gInformation.dateEnd.getTime();
			
			scData.iRepeatType	= gInformation.eventData.iRepeatType;
			scData.iAlarmType	= gInformation.eventData.iMode;
		}
		
		if(gInformation.dbSchedule == null)
		{
			gInformation.dbSchedule	= gInformation.cScheduler.makeScheduleDB(gInformation.mContext);
		}
		
		if(gInformation.dbSchedule != null)
		{
			long state	= -1;
			if(gInformation.eState == STATE.ADD)
			{
				state	= gInformation.cScheduler.insert(gInformation.mContext, gInformation.dbSchedule, scData);
				if(state != Scheduler.STATE_SUCCESS)
				{
					String	sMsg	= "";
					if(state == Scheduler.STATE_PLAYING)
					{
						sMsg	= getResources().getString(R.string.the_schedule_is_invalid);
					}
					else if(state == Scheduler.STATE_INVALID_TIME_END)
					{
						sMsg	= getResources().getString(R.string.invalid_end_time);
					}
					else if(state == Scheduler.STATE_INVALID_TIME_OVER)
					{
						sMsg	= getResources().getString(R.string.the_schedule_is_invalid_max_recording_time_is_10_hours);
					}
					else
					{
						sMsg	= getResources().getString(R.string.this_schedule_is_conflict_with_event) + state;
						DxbLog(D, "state=" + state);
					}
					
					AlertDialog msgDialog	= new AlertDialog.Builder(gInformation.mContext)
						.setTitle(getResources().getString(R.string.reminder))
						.setMessage(sMsg)
						.setPositiveButton( "OK", new DialogInterface.OnClickListener()
						{
							public void onClick(DialogInterface dialog, int which)
							{
								dialog.dismiss();
							}
						} ).show();


					
					
					return;
				}
			}
			else if(gInformation.eState == STATE.EDIT)
			{
				state	= gInformation.cScheduler.update(gInformation.mContext, gInformation.dbSchedule, gInformation.iID_EditEvent, scData);
				DxbLog(D, "state=" + state);
			}
			
			if(state >= 0)
			{
				setState(STATE.LIST);
				displayScreen();
			}
		}
	}
	
	private void accessDB_delete(int iID)
	{
		DxbLog(I, "accessDB_delete(iID=" + iID + ")");
		
		if(gInformation.iCount_Event > 0)
		{
			Scheduler.deleteData(gInformation.mContext, gInformation.dbSchedule, gInformation.curSchedule, Scheduler.TYPE_DELETE_KEY, iID);

			displayScreen();
		}
	}
	
	public static long getCurrentTime_TOT()
	{
		return dateCurrent_TOT.getTime();
	}

	final int D = 0;
	final int E = 1;
	final int I = 2;
	final int V = 3;
	final int W = 4;
	
	private void DxbLog(int level, String mString)
	{
		/*if(DxbView.TAG == null)
		{
			return;
		}*/
		
		String TAG = "[[[DxbScheduler]]] ";
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
