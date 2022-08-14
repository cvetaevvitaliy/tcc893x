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

import android.app.Dialog;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.database.Cursor;
import android.graphics.Color;
import android.graphics.Typeface;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.style.*;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.view.View.OnFocusChangeListener;
import android.view.ViewGroup.LayoutParams;
import android.widget.AbsoluteLayout;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemSelectedListener;

import com.telechips.android.isdbt.player.*;
import com.telechips.android.isdbt.player.isdbt.*;
import com.telechips.android.isdbt.util.DxbUtil;
import com.telechips.android.isdbt.DxbView_EPG;

import java.util.ArrayList;
import java.util.Date;

//  Component
class cEPGEvent
{
	public int iID;
	public TextView tvEPGEvent;
	public AbsoluteLayout.LayoutParams	lpEPGEvent;

	public cEPGEvent(int id, View tv)
	{
		Typeface typeface = Typeface.createFromFile("/system/fonts/DroidSansFallback_DxB.ttf");

		iID = id;
		tvEPGEvent = (TextView)tv;
		
		tvEPGEvent.setTypeface(typeface);

		lpEPGEvent = (AbsoluteLayout.LayoutParams)tvEPGEvent.getLayoutParams();
	}
}

//  Information
class EPGEvent
{
	public static final int TYPE_BLANK			= 0;
	public static final int TYPE_PRESENT		= 1;
	public static final int TYPE_FOLLOWING		= 2;
	public static final int TYPE_AFTER			= 3;
	public static final int TYPE_SCHE_BASIC		= 4;
	public static final int TYPE_SCHE_EXTEND	= 5;

	public int iChannelNumber;
	public int iServiceID;
	public String sChannelName;
	
	public int iType;

	public int iMJD;
	public int iStartHH, iStartMM;
	public int iDurationHH, iDurationMM;
	public int iEndHH, iEndMM;
	public String sEvtName;
	public String sEvtText;
	public String sEvtExtern;
	
	public int iGenre;
	public int iAudioMode;
	public int iAudioLang1;
	public int iAudioLang2;
	public int iVideoMode;
	
	public int iEventID;
	public int iRefServiceID;
	public int iRefEventID;

	public int iIndex_RefList;
	public int iIndex_RefEPGEvent;	

	public int iWeight;
	
	private void ApplyLocalTime(DateTimeData datetime)
	{
		if(iStartHH == 0xFF)
		{
			return;
		}

		if(datetime != null)
		{
			if(datetime.iPolarity == 0)
			{
				iStartHH += datetime.iHourOffset;
				if((iStartHH / 24) > 0)
				{
					iStartHH %= 24;
					iMJD += 1;
				}
				iStartMM += datetime.iMinOffset;
			}
			else
			{
				if(iStartHH >= datetime.iHourOffset)
				{
					iStartHH -= datetime.iHourOffset;
				}
				else
				{
					iStartHH = (iStartHH + 24) - datetime.iHourOffset;
					iMJD -= 1;
				}
				iStartMM -= datetime.iMinOffset;
			}
		}
	}
	
	public EPGEvent(int MJD, int startHH, int index_reflist, int index_refepgevent, DateTimeData datetime)
	{
		iChannelNumber = -1;
		iServiceID = -1;
		sChannelName = "";
		
		iType = TYPE_BLANK;
		iMJD = MJD;
		iStartHH = startHH;
		iStartMM = 0;
		iDurationHH = 1;
		iDurationMM = 0;
		iEndHH = iStartHH + iDurationHH;
		iEndMM = iStartMM + iDurationMM;

		iGenre = -1;
		iAudioMode = -1;
		iAudioLang1 = 0;
		iAudioLang2 = 0;
		iVideoMode = -1;
		
		sEvtName = "";
		sEvtText = "";
		sEvtExtern = "";	
		
		iEventID = -1;
		iRefServiceID = -1;
		iRefEventID = -1;

		iIndex_RefList = index_reflist;
		iIndex_RefEPGEvent = index_refepgevent;
		
		iWeight = 1;
	}

	public EPGEvent(DxbChannelData channel, Cursor cursor, int index_reflist, int index_refepgevent, DateTimeData datetime)
	{
		iChannelNumber = channel.iChannel_number;
		iServiceID = channel.iService_ID;
		sChannelName = channel.sChannel_name;
		
		int tableID = cursor.getInt(17);
		if(tableID == 0x4E)
		{
			int sectionNumber = cursor.getInt(18);
			if(sectionNumber == 0)
			{
				iType = TYPE_PRESENT;
			}
			else if(sectionNumber == 1)
			{
				iType = TYPE_FOLLOWING;
			}
			else
			{
				iType = TYPE_AFTER;
			}
		}
		else if(tableID >= 50 && tableID <= 57)
		{
			iType = TYPE_SCHE_BASIC;
		}
		else 
		{
			iType = TYPE_SCHE_EXTEND;
		}

		iMJD = cursor.getInt(1);
		iStartHH = cursor.getInt(2);
		iStartMM = cursor.getInt(3);

		ApplyLocalTime(datetime);

		iDurationHH = cursor.getInt(5);
		iDurationMM = cursor.getInt(6);

		if(iStartHH == 0xFF || iDurationHH == 0xFF)
		{
			iEndHH = 0xFF;
		}
		else
		{
			iEndHH = (iStartHH + iDurationHH) + ((iStartMM + iDurationMM) / 60);
		}
		if(iStartMM == 0xFF || iDurationMM == 0xFF) 
		{
			iEndMM = 0xFF;
		}
		else
		{
			iEndMM = (iStartMM + iDurationMM) % 60;
		}
		
		sEvtName = cursor.getString(8);
		sEvtText = cursor.getString(9);
		sEvtExtern = cursor.getString(10);
		
		iGenre		= cursor.getInt(11);
		iAudioMode	= cursor.getInt(12);
		iAudioLang1	= cursor.getInt(14);
		iAudioLang2	= cursor.getInt(15);
		iVideoMode	= cursor.getInt(16);
		
		iEventID = cursor.getInt(19);
		iRefServiceID = cursor.getInt(20);
		iRefEventID = cursor.getInt(21);
		
		iIndex_RefList = index_reflist;
		iIndex_RefEPGEvent = index_refepgevent;
		
		iWeight = 1;
	}
}	

public class DxbView_EPG_3List extends DxbView implements View.OnFocusChangeListener, View.OnClickListener, View.OnKeyListener
{
	private final View mView;
	
	//	Title
	private TextView	tvTitle_Service;
	private TextView	tvTitle_EPG;
	private TextView	tvTitle_Date;
	private Button		bPrev;
	private Button		bNext;
	
	//	Service
	private TextView	tvService[];
	private AbsoluteLayout.LayoutParams	lpService[];
		
	//	Time
	private TextView	tvTime[];
	private AbsoluteLayout.LayoutParams	lpTime[];
		
	//	EPGEvent
	private ArrayList<cEPGEvent> alcEPGEvent[];
		
	//	Button
	private Button	bLeft;
	private Button	bRight;
	private Button	bUp;
	private Button	bDown;
	private Button	bDetail;
	
	//  DetailView
	private LinearLayout	llParent;
	private View 			vDetail;
	private PopupWindow		wDetail;
	private TextView		tvDetail_Title;
	private TextView		tvDetail_Time;
	private TextView 		tvDetail_Text;
	private Button 			bDetail_Exit;
	
	private Button		bBook;
	private Button		bTimer;

	//	Service
	private boolean	isEPGopen;
	private ArrayList<DxbChannelData> alChannel;
	private int iCount_Channel;
	private int	iIndex_Channel;
	
	//	Time
	private int iFirst_MJD;
	private int iFirst_Time;
	private int iLast_MJD;
	private int iLast_Time;
	private int iCurrent_MJD;
	private int iCurrent_Time;
	
	// Table;
	private int iFirst_TableID;
	private int iLast_TableID;
	
	//	EPGEvent
	private ArrayList<EPGEvent> alEPGEvent[];
	private int iIndex_List;
	private int iIndex_EPGEvent;

	private static final int EPG_3LIST_LIST_COUNT			= 3;
	private static final int EPG_3LIST_TIME_COUNT			= 4;
	private static final int EPG_3LIST_EPGEVENT_COUNT		= 30;

	private int LIST_POS_X, LIST_POS_Y;
	private int LIST_BOX_WIDTH, LIST_BOX_HEIGHT;
	private int LIST_GAP_WIDTH, LIST_GAP_HEIGHT;
	private int LIST_TEXT_HEIGHT;
	private int LIST_MIN_HEIGHT;

	private int TIME_BOX_WIDTH, TIME_BOX_HEIGHT;
	private int TIME_GAP_HEIGHT;
	
	private cEPGEvent mEPGEventExtend;

	private String BlankBox;

	public DxbView_EPG_3List(Context context, View v)
	{
		super(context);
		mView = v;
		
		setInformation();
		setComponent();
		setPosition();
		setListener();	
	}

	@Override
	protected void EPGUpdate(DxbPlayer player, int serviceID, int tableID)
	{
		if(tableID >= iFirst_TableID && tableID <= iLast_TableID) 
		{
			DxbLog(I, "EPG Updated(" + tableID + ")");
			updateView();
		}
	}

	private void setInformation()
	{
		DxbLog(I, "setInformation()");
		
		isEPGopen = false;
		
		//	service information
		alChannel = null;
		iCount_Channel = 0;
		iIndex_Channel =0;
		
		//	time information
		iCurrent_MJD = 0;
		iCurrent_Time = 0;

		//  table information
		iFirst_TableID = 0x4E;
		iLast_TableID = 0x5F;

		alEPGEvent = new ArrayList[EPG_3LIST_LIST_COUNT];
		
		for(int i=0; i<EPG_3LIST_LIST_COUNT; i++)
		{
			alEPGEvent[i] = new ArrayList<EPGEvent>();
		}
	}
	
	private void setComponent()
	{
		DxbLog(I, "setComponent()");

		Typeface typeface = Typeface.createFromFile("/system/fonts/DroidSansFallback_DxB.ttf");
		
		//	bind title components
		tvTitle_Service	= (TextView)mView.findViewById(R.id.epg_title_service);
		tvTitle_Service.setTypeface(typeface);
		tvTitle_EPG		= (TextView)mView.findViewById(R.id.epg_title_event);
		tvTitle_EPG.setTypeface(typeface);
		tvTitle_Date		= (TextView)mView.findViewById(R.id.epg_title_date);
		tvTitle_Date.setTypeface(typeface);
		bPrev			= (Button)mView.findViewById(R.id.epg_3list_button_prev);
		bNext			= (Button)mView.findViewById(R.id.epg_3list_button_next);
		
		//	bind service components
		tvService = new TextView[EPG_3LIST_LIST_COUNT];
		
		tvService[0]	= (TextView)mView.findViewById(R.id.epg_service0);
		tvService[0].setTypeface(typeface);
		tvService[1]	= (TextView)mView.findViewById(R.id.epg_service1);
		tvService[1].setTypeface(typeface);
		tvService[2]	= (TextView)mView.findViewById(R.id.epg_service2);
		tvService[2].setTypeface(typeface);

		lpService = new AbsoluteLayout.LayoutParams[3];
		
		lpService[0]	= (AbsoluteLayout.LayoutParams)tvService[0].getLayoutParams();
		lpService[1]	= (AbsoluteLayout.LayoutParams)tvService[1].getLayoutParams();
		lpService[2]	= (AbsoluteLayout.LayoutParams)tvService[2].getLayoutParams();

		//	bind time components
		tvTime = new TextView[EPG_3LIST_TIME_COUNT];
		
		tvTime[0]	= (TextView)mView.findViewById(R.id.epg_time0);
		tvTime[1]	= (TextView)mView.findViewById(R.id.epg_time1);
		tvTime[2]	= (TextView)mView.findViewById(R.id.epg_time2);
		tvTime[3]	= (TextView)mView.findViewById(R.id.epg_time3);

		lpTime = new AbsoluteLayout.LayoutParams[4];
		
		lpTime[0]	= (AbsoluteLayout.LayoutParams)tvTime[0].getLayoutParams();
		lpTime[1]	= (AbsoluteLayout.LayoutParams)tvTime[1].getLayoutParams();
		lpTime[2]	= (AbsoluteLayout.LayoutParams)tvTime[2].getLayoutParams();
		lpTime[3]	= (AbsoluteLayout.LayoutParams)tvTime[3].getLayoutParams();

		//	bind EPG event components
		alcEPGEvent = new ArrayList[EPG_3LIST_LIST_COUNT];
	
		alcEPGEvent[0] = new ArrayList<cEPGEvent>();
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_0, mView.findViewById(R.id.epg_service0_0)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_1, mView.findViewById(R.id.epg_service0_1)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_2, mView.findViewById(R.id.epg_service0_2)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_3, mView.findViewById(R.id.epg_service0_3)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_4, mView.findViewById(R.id.epg_service0_4)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_5, mView.findViewById(R.id.epg_service0_5)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_6, mView.findViewById(R.id.epg_service0_6)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_7, mView.findViewById(R.id.epg_service0_7)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_8, mView.findViewById(R.id.epg_service0_8)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_9, mView.findViewById(R.id.epg_service0_9)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_10, mView.findViewById(R.id.epg_service0_10)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_11, mView.findViewById(R.id.epg_service0_11)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_12, mView.findViewById(R.id.epg_service0_12)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_13, mView.findViewById(R.id.epg_service0_13)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_14, mView.findViewById(R.id.epg_service0_14)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_10, mView.findViewById(R.id.epg_service0_15)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_11, mView.findViewById(R.id.epg_service0_16)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_12, mView.findViewById(R.id.epg_service0_17)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_13, mView.findViewById(R.id.epg_service0_18)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_14, mView.findViewById(R.id.epg_service0_19)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_10, mView.findViewById(R.id.epg_service0_20)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_11, mView.findViewById(R.id.epg_service0_21)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_12, mView.findViewById(R.id.epg_service0_22)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_13, mView.findViewById(R.id.epg_service0_23)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_14, mView.findViewById(R.id.epg_service0_24)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_10, mView.findViewById(R.id.epg_service0_25)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_11, mView.findViewById(R.id.epg_service0_26)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_12, mView.findViewById(R.id.epg_service0_27)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_13, mView.findViewById(R.id.epg_service0_28)));
		alcEPGEvent[0].add(new cEPGEvent(R.id.epg_service0_14, mView.findViewById(R.id.epg_service0_29)));

		alcEPGEvent[1] = new ArrayList<cEPGEvent>();
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_0, mView.findViewById(R.id.epg_service1_0)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_1, mView.findViewById(R.id.epg_service1_1)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_2, mView.findViewById(R.id.epg_service1_2)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_3, mView.findViewById(R.id.epg_service1_3)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_4, mView.findViewById(R.id.epg_service1_4)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_5, mView.findViewById(R.id.epg_service1_5)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_6, mView.findViewById(R.id.epg_service1_6)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_7, mView.findViewById(R.id.epg_service1_7)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_8, mView.findViewById(R.id.epg_service1_8)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_9, mView.findViewById(R.id.epg_service1_9)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_10, mView.findViewById(R.id.epg_service1_10)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_11, mView.findViewById(R.id.epg_service1_11)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_12, mView.findViewById(R.id.epg_service1_12)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_13, mView.findViewById(R.id.epg_service1_13)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_14, mView.findViewById(R.id.epg_service1_14)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_10, mView.findViewById(R.id.epg_service1_15)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_11, mView.findViewById(R.id.epg_service1_16)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_12, mView.findViewById(R.id.epg_service1_17)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_13, mView.findViewById(R.id.epg_service1_18)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_14, mView.findViewById(R.id.epg_service1_19)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_10, mView.findViewById(R.id.epg_service1_20)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_11, mView.findViewById(R.id.epg_service1_21)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_12, mView.findViewById(R.id.epg_service1_22)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_13, mView.findViewById(R.id.epg_service1_23)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_14, mView.findViewById(R.id.epg_service1_24)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_10, mView.findViewById(R.id.epg_service1_25)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_11, mView.findViewById(R.id.epg_service1_26)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_12, mView.findViewById(R.id.epg_service1_27)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_13, mView.findViewById(R.id.epg_service1_28)));
		alcEPGEvent[1].add(new cEPGEvent(R.id.epg_service1_14, mView.findViewById(R.id.epg_service1_29)));

		alcEPGEvent[2] = new ArrayList<cEPGEvent>();
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_0, mView.findViewById(R.id.epg_service2_0)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_1, mView.findViewById(R.id.epg_service2_1)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_2, mView.findViewById(R.id.epg_service2_2)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_3, mView.findViewById(R.id.epg_service2_3)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_4, mView.findViewById(R.id.epg_service2_4)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_5, mView.findViewById(R.id.epg_service2_5)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_6, mView.findViewById(R.id.epg_service2_6)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_7, mView.findViewById(R.id.epg_service2_7)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_8, mView.findViewById(R.id.epg_service2_8)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_9, mView.findViewById(R.id.epg_service2_9)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_10, mView.findViewById(R.id.epg_service2_10)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_11, mView.findViewById(R.id.epg_service2_11)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_12, mView.findViewById(R.id.epg_service2_12)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_13, mView.findViewById(R.id.epg_service2_13)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_14, mView.findViewById(R.id.epg_service2_14)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_10, mView.findViewById(R.id.epg_service2_15)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_11, mView.findViewById(R.id.epg_service2_16)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_12, mView.findViewById(R.id.epg_service2_17)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_13, mView.findViewById(R.id.epg_service2_18)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_14, mView.findViewById(R.id.epg_service2_19)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_10, mView.findViewById(R.id.epg_service2_20)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_11, mView.findViewById(R.id.epg_service2_21)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_12, mView.findViewById(R.id.epg_service2_22)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_13, mView.findViewById(R.id.epg_service2_23)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_14, mView.findViewById(R.id.epg_service2_24)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_10, mView.findViewById(R.id.epg_service2_25)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_11, mView.findViewById(R.id.epg_service2_26)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_12, mView.findViewById(R.id.epg_service2_27)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_13, mView.findViewById(R.id.epg_service2_28)));
		alcEPGEvent[2].add(new cEPGEvent(R.id.epg_service2_14, mView.findViewById(R.id.epg_service2_29)));

		mEPGEventExtend = new cEPGEvent(R.id.epg_service_extend, mView.findViewById(R.id.epg_service_extend));
		
		//	bind button components
		bLeft	= (Button)mView.findViewById(R.id.epg_3list_button_left);

		mEPGEventExtend = new cEPGEvent(R.id.epg_service_extend, mView.findViewById(R.id.epg_service_extend));
		
		//	bind button components
		bLeft	= (Button)mView.findViewById(R.id.epg_3list_button_left);
		bRight	= (Button)mView.findViewById(R.id.epg_3list_button_up);
		bUp		= (Button)mView.findViewById(R.id.epg_3list_button_down);
		bDown	= (Button)mView.findViewById(R.id.epg_3list_button_right);
		bDetail	= (Button)mView.findViewById(R.id.epg_3list_button_detail);
		
		//	bind detail components		
		llParent			= (LinearLayout)mView.findViewById(R.id.epg_3list_button);
		vDetail				= View.inflate(getContext(), R.layout.dxb_view_epg_3list_detail, null);
		tvDetail_Title		= (TextView)vDetail.findViewById(R.id.epg_detail_title);
		tvDetail_Title.setTypeface(typeface);
		tvDetail_Time		= (TextView)vDetail.findViewById(R.id.epg_detail_time);
		tvDetail_Time.setTypeface(typeface);
		tvDetail_Text		= (TextView)vDetail.findViewById(R.id.epg_detail_text);
		tvDetail_Text.setTypeface(typeface);
		bDetail_Exit		= (Button)vDetail.findViewById(R.id.epg_detail_button_exit); 

		bBook	= (Button)mView.findViewById(R.id.epg_3list_b_book);
		bTimer	= (Button)mView.findViewById(R.id.epg_3list_b_timer);
	}
	
	private void setPosition()
	{
		DxbLog(I, "setPosition()");

		TextView	tvTitle0	= (TextView)mView.findViewById(R.id.epg_title_title0);
		TextView	tvTitle1	= (TextView)mView.findViewById(R.id.epg_title_title1);
		float infoTextSize;	
		int displayWidth;
		
		displayWidth = getDisplayWidth();
		if(displayWidth > 1280)
		{
			LIST_POS_X			= 60;
			LIST_POS_Y			= 36 + 1;

			LIST_BOX_WIDTH		= 598;
			LIST_BOX_HEIGHT		= 160;
			
			LIST_GAP_WIDTH		= 1;
			LIST_GAP_HEIGHT		= 1;

			LIST_MIN_HEIGHT		= 36;

			LIST_TEXT_HEIGHT	= 36;
			
			TIME_BOX_WIDTH		= 60;
			TIME_BOX_HEIGHT		= 160;
			TIME_GAP_HEIGHT		= 1;
				
			tvTitle0.setTextSize(40);
			tvTitle1.setTextSize(25);
				
			tvTitle_Service.setTextSize(35);
			tvTitle_EPG.setTextSize(25);
			tvTitle_Date.setTextSize(25);
				
			wDetail	= new PopupWindow(vDetail, 800, 400, true);

			tvDetail_Title.setTextSize(30);
			tvDetail_Time.setTextSize(20);
			tvDetail_Text.setTextSize(20);

			// Schedule
			infoTextSize	= 11 * getDisplayWidth() / getDisplayDensity() / 800;
			bBook.setTextSize(infoTextSize);
			bTimer.setTextSize(infoTextSize);

			BlankBox = "                ";
		}
		else if(displayWidth > 1024)
		{
			LIST_POS_X			= 50;
			LIST_POS_Y			= 30 + 1;

			LIST_BOX_WIDTH		= 390;
			LIST_BOX_HEIGHT		= 102;
			
			LIST_GAP_WIDTH		= 1;
			LIST_GAP_HEIGHT		= 1;

			LIST_MIN_HEIGHT		= 30;

			LIST_TEXT_HEIGHT	= 30;
			
			TIME_BOX_WIDTH		= 50;
			TIME_BOX_HEIGHT		= 102;
			TIME_GAP_HEIGHT		= 1;
				
			tvTitle0.setTextSize(40);
			tvTitle1.setTextSize(25);
				
			tvTitle_Service.setTextSize(35);
			tvTitle_EPG.setTextSize(25);
			tvTitle_Date.setTextSize(25);
				
			wDetail	= new PopupWindow(vDetail, 800, 400, true);

			tvDetail_Title.setTextSize(30);
			tvDetail_Time.setTextSize(20);
			tvDetail_Text.setTextSize(20);

			// Schedule
			infoTextSize	= 11 * getDisplayWidth() / getDisplayDensity() / 800;
			bBook.setTextSize(infoTextSize);
			bTimer.setTextSize(infoTextSize);

			BlankBox = "                      ";
		}
		else if(displayWidth > 800)
		{
			LIST_POS_X		= 40;
			LIST_POS_Y		= 25 + 1;

			LIST_BOX_WIDTH	= 310;
			LIST_BOX_HEIGHT	= 90;
							
			LIST_GAP_WIDTH	= 1;
			LIST_GAP_HEIGHT	= 1;

			LIST_MIN_HEIGHT = 25;
				
			LIST_TEXT_HEIGHT = 25;
				
			TIME_BOX_WIDTH = 40;
			TIME_BOX_HEIGHT = 90;
			TIME_GAP_HEIGHT = 1;

			tvTitle0.setTextSize(35);
			tvTitle1.setTextSize(22);
				
			tvTitle_Service.setTextSize(30);
			tvTitle_EPG.setTextSize(20);
			tvTitle_Date.setTextSize(20);

			wDetail	= new PopupWindow(vDetail, 720, 340, true);
				
			tvDetail_Title.setTextSize(25);
			tvDetail_Time.setTextSize(20);
			tvDetail_Text.setTextSize(20);
				
			mView.findViewById(R.id.epg_3list_margin_top).setVisibility(View.GONE);
			mView.findViewById(R.id.epg_3list_margin_bottom).setVisibility(View.GONE);

			BlankBox = "                ";
		}
		else
		{
			LIST_POS_X		= 30;
			LIST_POS_Y		= 20 + 1;

			LIST_BOX_WIDTH	= 240;
			LIST_BOX_HEIGHT	= 70;
						
			LIST_GAP_WIDTH	= 1;
			LIST_GAP_HEIGHT	= 1;

			LIST_MIN_HEIGHT = 20;
			
			LIST_TEXT_HEIGHT = 20;
			
			TIME_BOX_WIDTH = 30;
			TIME_BOX_HEIGHT = 70;
			TIME_GAP_HEIGHT = 1;

			tvTitle0.setTextSize(30);
			tvTitle1.setTextSize(20);
			
			tvTitle_Service.setTextSize(25);
			tvTitle_EPG.setTextSize(20);
			tvTitle_Date.setTextSize(20);

			wDetail	= new PopupWindow(vDetail, 500, 280, true);
			
			tvDetail_Title.setTextSize(25);
			tvDetail_Time.setTextSize(20);
			tvDetail_Text.setTextSize(20);
			
			mView.findViewById(R.id.epg_3list_margin_top).setVisibility(View.GONE);
			mView.findViewById(R.id.epg_3list_margin_bottom).setVisibility(View.GONE);

			BlankBox = "                ";
		}

		//	set service layout
		for(int i=0; i<EPG_3LIST_LIST_COUNT; i++)
		{
			lpService[i].x		= LIST_POS_X + ((LIST_BOX_WIDTH + LIST_GAP_WIDTH) * i);
			lpService[i].width	= LIST_BOX_WIDTH;
			lpService[i].height	= LIST_TEXT_HEIGHT;
		}
		
		//	set time layout
		for(int i=0; i<EPG_3LIST_TIME_COUNT; i++)
		{
			lpTime[i].y		= LIST_POS_Y + ((TIME_BOX_HEIGHT + TIME_GAP_HEIGHT) * i);
			lpTime[i].width	= TIME_BOX_WIDTH;
			lpTime[i].height	= TIME_BOX_HEIGHT;
		}

		//	set EPG event layout
		for(int i=0; i<EPG_3LIST_LIST_COUNT; i++)
		{
			int	x =  LIST_POS_X + ((LIST_BOX_WIDTH + LIST_GAP_WIDTH) * i);
			
			for(int j=0; j<EPG_3LIST_EPGEVENT_COUNT; j++)
			{
				cEPGEvent epgevent = (cEPGEvent)alcEPGEvent[i].get(j);
				
				epgevent.lpEPGEvent.x		= x;
				epgevent.lpEPGEvent.width	= LIST_BOX_WIDTH;
			}
		}
	}
    
	private void setListener()
	{
		DxbLog(I, "setListener()");

		//	set title listener
		bPrev.setOnClickListener(this);
		bNext.setOnClickListener(this);
		bPrev.setOnTouchListener(new OnTouchListener() {          
			public boolean onTouch(View v, MotionEvent event) { 
				if(bPrev.isFocusable() == true)
				{
					if(event.getAction() == MotionEvent.ACTION_DOWN)
					{
						bPrev.setBackgroundResource(R.drawable.dxb_portable_epg_left_btn_f);
					}	
					if(event.getAction() == MotionEvent.ACTION_UP){   
						bPrev.setBackgroundResource(R.drawable.dxb_portable_epg_left_btn_n);  
					}
				}
		        return false; 
		    } 
        });
		bNext.setOnTouchListener(new OnTouchListener() {          
            public boolean onTouch(View v, MotionEvent event) { 
				if(bNext.isFocusable() == true)
				{
					if(event.getAction() == MotionEvent.ACTION_DOWN)
					{
						bNext.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_f);
					}
                	if(event.getAction() == MotionEvent.ACTION_UP)
                	{   
                		bNext.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_n);
                	}
                } 
                return false; 
            } 
        });
	    
		//	set EPG event listener
		for(int i=0; i<EPG_3LIST_LIST_COUNT; i++)
		{
			for(int j=0; j<EPG_3LIST_EPGEVENT_COUNT; j++)
			{
				cEPGEvent epgevent = (cEPGEvent)alcEPGEvent[i].get(j);
				
				//epgevent.tvEPGEvent.setOnClickListener(ListenerOnClickEPGEvent);
			}
		}
		
		//	set button listener
		bLeft.setOnClickListener(this);
		bRight.setOnClickListener(this);
		bUp.setOnClickListener(this);
		bDown.setOnClickListener(this);
		bDetail.setOnClickListener(this);
		
		//  set detail listener
		bDetail_Exit.setOnClickListener(this);
		bDetail_Exit.setOnFocusChangeListener(this);

		//	PVR listener
		bBook.setOnClickListener(this);
		bTimer.setOnClickListener(this);
		bBook.setOnFocusChangeListener(this);
		bTimer.setOnFocusChangeListener(this);			
	}

	private int getIndex_Channel(int index_list)
	{
		int index_channel = 0;

		if(index_list >= iCount_Channel)
		{
			DxbLog(D, "iIndex_List is smaller than gInformation.iCount_Channel");
			return -1;
		}
		
		index_channel = iIndex_Channel + index_list;
		if(index_channel >= iCount_Channel)
		{
			index_channel -= iCount_Channel;
		}

		return index_channel;
	}
	
	private String getChannelName(int index_list)
	{
		int index_channel = 0;
		String msg = "";
		
		if((alChannel != null) && (alChannel.size() > 0))
		{
			index_channel = getIndex_Channel(index_list);
			if(index_channel >= 0)
			{
				msg = alChannel.get(index_channel).sChannel_name;
			}
		}
		
		return msg;
	}
	
	private String getDate(int iMJD)
	{
		int yDash, mDash, k, Day, Month, Year, Week;
		String Date;
		
		Date = "";
		
		if(iMJD > 0)
		{
			CharSequence[] arrayWeek = getResources().getTextArray(R.array.week_select_entries);

			yDash = (iMJD * 100 - 1507820) / 36525;
			mDash = (iMJD * 10000 - 149561000 - (((yDash * 3652500)/10000)*10000)) / 306001;

			Day = (iMJD * 10000 - 149560000 - (((yDash * 3652500)/10000)*10000) - (((mDash * 306001)/10000)*10000)) / 10000;
			if(mDash == 14 || mDash == 15)
			{
				k = 1;
			}
			else
			{
				k = 0;
			}

			Year = yDash + k + 1900;
			Month = mDash - 1 - k * 12;
			Week = (iMJD + 3) % 7;

			Date = Year + ". " + Month + ". " + Day +". (" + arrayWeek[Week] + ")";
		}
		
		return Date;
	}
	
	private String getDateTime(EPGEvent Event)
	{
		int sHour, sMin, eHour, eMin, dHour, dMin, b;
		String Date, Time, SHour, SMin, EHour, EMin;

		sHour = Event.iStartHH;
		sMin = Event.iStartMM;
		dHour = Event.iDurationHH;
		dMin = Event.iDurationMM;
		eHour = eMin = 0;
		Date = Time = SHour = SMin = EHour = EMin = "";

		Date = getDate(Event.iMJD);
		
		if((sHour>=0)&&(sMin>=0)&&(dHour>=0)&&(dMin>=0))
		{

			if((sHour==0xff)&&(sMin==0xff)&&(dHour==0xff)&&(dMin==0xff))
			{
				SHour= "**";
				SMin = "**";
				EHour = "**";
				EMin = "**";
			}
			else if((dHour==0xff)&&(dMin==0xff))
			{
				SHour= String.format("%02d", sHour);
				SMin = String.format("%02d", sMin);
				EHour = "**";
				EMin = "**";
			}
			else
			{
				eMin = sMin + dMin;
				b = eMin/60;
				eMin %=60;

				eHour = (sHour + b + dHour)%24;

				SHour = String.format("%02d", sHour);
				SMin = String.format("%02d", sMin);
				EHour = String.format("%02d", eHour);
				EMin = String.format("%02d", eMin);
			}

			Time = String.format(" %s:%s ~ %s:%s", SHour, SMin, EHour, EMin);
		}
		
		return Date+Time;
	}

	private void updateTitle()
	{
		EPGEvent info;
		String msg;

		//  update channel name
		tvTitle_Service.setText(getChannelName(iIndex_List));
		
		//  update EPG event
		tvTitle_EPG.setText("");

		if(alEPGEvent[iIndex_List].size() > 0)
		{
			info = alEPGEvent[iIndex_List].get(iIndex_EPGEvent);
			if(info != null)
			{
				msg = getDateTime(info);
				if(info.sEvtName != null)
				{
					msg += "  " + info.sEvtName;					
				}
				
				tvTitle_EPG.setText(msg);
			}
		}

		//  update Date
		msg = getDate(iCurrent_MJD);
		tvTitle_Date.setText(msg);
	}

	private void updateService()
	{
		String msg;
		
		for(int i=0; i<EPG_3LIST_LIST_COUNT; i++)
		{
			msg = getChannelName(i);
			tvService[i].setText(msg);
		}		
	}

	private void updateTime()
	{
		String msg;
		
		for(int i=0; i<EPG_3LIST_TIME_COUNT; i++)
		{
			msg = String.format("%02d", (iCurrent_Time + i) % 24);
			tvTime[i].setText(msg);
		}
	}

	void updateDate()
	{
		//  change button image
		if(iCurrent_MJD <= iFirst_MJD)
		{
			bPrev.setBackgroundResource(R.drawable.dxb_portable_epg_left_btn_d);
			bPrev.setFocusable(false);
		}
		else
		{
			bPrev.setBackgroundResource(R.drawable.dxb_portable_epg_left_btn_n);
			bPrev.setFocusable(true);
		}

		if(iCurrent_MJD >= iLast_MJD)
		{
			bNext.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_d);
			bNext.setFocusable(false);
		}
		else
		{
			bNext.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_n);						
			bNext.setFocusable(true);
		}
	}
	
	private void updateInfo()
	{
		updateTitle();
		
		updateService();
		
		updateTime();
		
		updateDate();
	}
	
	private void decorateEPGEvent(EPGEvent info, cEPGEvent comp)
	{
		String time, msg;
		int margin, margin_top;

		if(info.iChannelNumber < 0)
		{
			DxbLog(D, "info.iChannelNumber is smaller than 0");
			comp.tvEPGEvent.setText("");
			
			return;
		}
		
		margin = (int)getResources().getDimension(R.dimen.epg_margin_text);
		margin_top = (int)getResources().getDimension(R.dimen.epg_margin_text_top);
		
		comp.tvEPGEvent.setBackgroundColor(0xFFDDDDDD);
		
		if(comp.lpEPGEvent.height >= LIST_MIN_HEIGHT)
		{
			if(info.iStartHH == 0xFF)
			{
				time = "** : **";
			}
			else
			{
				if((info.iMJD < iCurrent_MJD) || (info.iMJD == iCurrent_MJD && info.iStartHH < iCurrent_Time))
				{	
					time = String.format("%02d : %02d", info.iStartHH, info.iStartMM);
				}
				else
				{
					time = String.format("%02d", info.iStartMM);
				}
			}
			
			msg = time + "  ";
			if(info.sEvtName != null)
			{
				msg += info.sEvtName + "\n  ";
			}
			if(info.sEvtText != null)
			{
				msg += info.sEvtText + "\n ";
			}
			if(info.sEvtExtern != null)
			{
				msg += info.sEvtExtern;
			}
	
			final SpannableStringBuilder sps = new SpannableStringBuilder(msg);
	
			sps.setSpan(new ForegroundColorSpan(0xFFFFFFFF), 0, time.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
			sps.setSpan(new BackgroundColorSpan(0xFF3A3A3A), 0, time.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

			if(info.sEvtName != null)
			{
				msg = time + "  ";
				sps.setSpan(new StyleSpan(Typeface.BOLD), msg.length(), msg.length() + info.sEvtName.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
				sps.setSpan(new ForegroundColorSpan(0xFF000000), msg.length(), msg.length() + info.sEvtName.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
	
				msg += info.sEvtName;
			}

			sps.setSpan(new RelativeSizeSpan((float)1.3), 0, msg.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
				
			comp.tvEPGEvent.setPadding(margin, margin_top, margin, 0);
			comp.tvEPGEvent.setTextSize(15);
			comp.tvEPGEvent.setText(sps);
		}
		else
		{
			final SpannableStringBuilder sps = new SpannableStringBuilder(BlankBox);

			sps.setSpan(new ForegroundColorSpan(0xFF3A3A3A), 0, BlankBox.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
			sps.setSpan(new BackgroundColorSpan(0xFF3A3A3A), 0, BlankBox.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

			if(comp.lpEPGEvent.height > 5)
			{
				comp.tvEPGEvent.setPadding(margin, margin_top, margin, 0);
			}
			else
			{
				comp.tvEPGEvent.setPadding(margin, 0, margin, 0);
			}
		
			comp.tvEPGEvent.setTextSize(5);
			comp.tvEPGEvent.setText(sps);
		}
	}

	private void updateLastTime()
	{
		EPGEvent info;
		Cursor cursor;
		DxbChannelData channel;
		DateTimeData datetime;
		int index_channel;
		
		DxbLog(I, "updateLastTime()");
		
		if((alChannel != null) && (alChannel.size() > 0))
		{
			index_channel = getIndex_Channel(iIndex_List);
			if(index_channel >= 0)
			{
				channel = alChannel.get(index_channel);
				cursor = getPlayer().getEPG_Schedule_LastEvent(channel.iService_ID, channel.iChannel_number, channel.iCountry_code, channel.iService_type);
				datetime = getPlayer().getCurrentDateTime();

				if(cursor != null && cursor.getCount() > 0)
				{
					cursor.moveToFirst();		
					
					info = new EPGEvent(channel, cursor, 0, 0, datetime);
					if(info.iStartHH != 0xFF)
					{
						if((info.iMJD > iLast_MJD)
							|| ((info.iMJD == iLast_MJD) && (info.iStartHH > iLast_Time)))
						{
							if(info.iStartHH < 3)
							{
								iLast_MJD = info.iMJD - 1;
								iLast_Time = (info.iStartHH + 24) - 3;					
							}
							else
							{
								iLast_MJD = info.iMJD;
								iLast_Time = info.iStartHH - 3;
							}
						}
					}
				}

				if(cursor != null)
				{
					cursor.close();
				}
			}
		}
	}
	
	private void updateEPGEventInfo(boolean isMoveDown)
	{
		EPGEvent prev_info, cur_info, blank_info, ref_info;
		int i, j, k, l;
		int count, ref_count;
		Cursor cursor;
		DxbChannelData channel;
		DateTimeData datetime;
		int current_mjd, current_time;
		int[] prev_event_time = new int[EPG_3LIST_LIST_COUNT];
		
		DxbLog(I, "updateEPGEventInfo(" + isMoveDown + ")");
		
		datetime = getPlayer().getCurrentDateTime();
		current_mjd = iCurrent_MJD;
		current_time = iCurrent_Time;

		for(i=0; i<EPG_3LIST_LIST_COUNT; i++)
		{
			prev_event_time[i] = -1;
		}

		if(datetime != null)
		{
			if(datetime.iPolarity == 0)
			{
				if(current_time >= datetime.iHourOffset)
				{
					current_time -= datetime.iHourOffset;
				}
				else
				{
					current_time = (current_time + 24) - datetime.iHourOffset;
					current_mjd -= 1;
				}
			}
			else
			{
				current_time += datetime.iHourOffset;
				if((current_time / 24) > 0)
				{
					current_time %= 24;
					current_mjd += 1;
				}
			}
		}
		
		//  clear information of EPG event
		for(i=0; i<EPG_3LIST_LIST_COUNT; i++)
		{
			if(isMoveDown == true && alEPGEvent[i].size() > 0)
			{
				cur_info = alEPGEvent[i].get(0);
				prev_event_time[i] = cur_info.iStartHH;
			}
			else
			{
				prev_event_time[i] = -1;
			}

			alEPGEvent[i].removeAll(alEPGEvent[i]);
		}
		
		//  make information of EPG event
		for(i=0; i<EPG_3LIST_LIST_COUNT; i++)
		{
			if((alChannel != null) && (alChannel.size() > 0))
			{
				int iIndex_Channel = getIndex_Channel(i);
				if(iIndex_Channel >= 0)
				{
					channel = alChannel.get(iIndex_Channel);
					cursor = getPlayer().getEPG_Schedule(channel.iService_ID, channel.iChannel_number, channel.iCountry_code, channel.iService_type, current_mjd, current_time, prev_event_time[i]);
					if(cursor != null && cursor.getCount() > 0)
					{
						cursor.moveToFirst();
						count = cursor.getCount();
						
						for(j=0; j<count; j++)
						{
							cur_info = new EPGEvent(channel, cursor, i, j, datetime);
							if(cur_info != null)
							{
								int cur_mjd = cur_info.iMJD;
								int cur_end = cur_info.iEndHH;
								if(cur_end > 23)
								{
									cur_mjd += 1;
									cur_end -= 24;									
								}

								if(cur_info.iStartHH == 0xff || cur_info.iStartMM == 0xff) {
									count = count - j;
									break;
								}

								if(((iCurrent_MJD + 1) == cur_mjd)
									||((iCurrent_MJD == cur_mjd) && (iCurrent_Time < cur_end))
									|| ((iCurrent_MJD == cur_mjd) && (iCurrent_Time == cur_end && cur_info.iEndMM > 0))
									|| ((iCurrent_MJD == cur_info.iMJD) && (iCurrent_Time <= cur_info.iStartHH)))
								{
									count = count - j;
									break;
								}
							}
							
							if(cursor.isLast() == true)
							{
								count = 0;
								break;
							}
							
							cursor.moveToNext();
						}
	
						prev_info = null;
						cur_info = null;
						blank_info = null;
						ref_info = null;
						
						iFirst_TableID = cursor.getInt(17);

						for(j=0; j<count; j++)
						{
							cur_info = new EPGEvent(channel, cursor, i, j, datetime);
							if(j == 0)
							{
								if(((iCurrent_MJD == cur_info.iMJD) && ((iCurrent_Time < cur_info.iStartHH)
									|| (iCurrent_Time == cur_info.iStartHH && 0 < cur_info.iStartMM))) 
									|| (iCurrent_MJD < cur_info.iMJD))
								{
									blank_info = new EPGEvent(channel, cursor, i, j, datetime);
	
									blank_info.iStartHH = iCurrent_Time;
									blank_info.iStartMM = 0;
	
									if(iCurrent_MJD == cur_info.iMJD)
									{
										blank_info.iDurationHH = cur_info.iStartHH - iCurrent_Time;
									}
									else
									{
										blank_info.iDurationHH = (cur_info.iStartHH + 24) - iCurrent_Time;
									}
									blank_info.iDurationMM = cur_info.iStartMM;
	
									blank_info.iEndHH = blank_info.iStartHH + blank_info.iDurationHH + ((blank_info.iStartMM + blank_info.iDurationMM) / 60);
									blank_info.iEndMM = (blank_info.iStartMM + blank_info.iDurationMM) % 60;
									
									blank_info.sEvtName = "";
									blank_info.sEvtText = "";
									blank_info.sEvtExtern = "";
									
									blank_info.iEventID -= 1;
									blank_info.iRefEventID -= 1;
				
									alEPGEvent[i].add(blank_info);
									prev_info = blank_info;
									
									j++;
									count++;
									
									cur_info = new EPGEvent(channel, cursor, i, j, datetime);
								}
							}
							else if(prev_info.iEndHH != 0xFF && cur_info.iStartHH != 0xFF)
							{
								int prev_end = (prev_info.iEndHH * 60) + prev_info.iEndMM;
								int cur_start = (((cur_info.iMJD - prev_info.iMJD) * 24) * 60) + (cur_info.iStartHH * 60) + cur_info.iStartMM;
								
								if(cur_start - prev_end > 2)
								{
									blank_info = new EPGEvent(channel, cursor, i, j, datetime);
									
									blank_info.iMJD = prev_info.iMJD;
									blank_info.iStartHH = prev_info.iEndHH;
									if(blank_info.iStartHH > 23)
									{
										blank_info.iMJD += 1;
										blank_info.iStartHH -= 24;									
									}
									blank_info.iStartMM =  prev_info.iEndMM;
									
									blank_info.iDurationHH = (cur_start - prev_end) / 60;
									blank_info.iDurationMM = (cur_start - prev_end) % 60;
	
									blank_info.iEndHH = cur_info.iStartHH;
									blank_info.iEndMM = cur_info.iStartMM;
	
									blank_info.sEvtName = "";
									blank_info.sEvtText = "";
									blank_info.sEvtExtern = "";
	
									blank_info.iEventID -= 1;
									blank_info.iRefEventID -= 1;
				
									alEPGEvent[i].add(blank_info);
									prev_info = blank_info;
									
									j++;
									count++;
									
									cur_info = new EPGEvent(channel, cursor, i, j, datetime);
								}
							}	
							
							alEPGEvent[i].add(cur_info);
							prev_info = cur_info;
							
							if(j+1 >= count && prev_info.iEndHH != 0xFF)
							{
								int prev_end = (prev_info.iEndHH * 60) + prev_info.iEndMM + 1;
								int cur_start = (iCurrent_Time + EPG_3LIST_TIME_COUNT) * 60;
								
								if(prev_info.iMJD > iCurrent_MJD)
								{
									cur_start -= (24 * 60);
								}
	
								if(prev_end < cur_start)
								{
									blank_info = new EPGEvent(channel, cursor, i, j+1, datetime);
									
									blank_info.iMJD = prev_info.iMJD;
									blank_info.iStartHH = prev_info.iEndHH;
									if(blank_info.iStartHH > 23)
									{
										blank_info.iMJD += 1;
										blank_info.iStartHH -= 24;									
									}
									blank_info.iStartMM =  prev_info.iEndMM;
									
									blank_info.iDurationHH = (cur_start - prev_end) / 60;
									blank_info.iDurationMM = (cur_start - prev_end) % 60;
	
									blank_info.iEndHH = iCurrent_Time + EPG_3LIST_TIME_COUNT - 1;
									blank_info.iEndMM = 59;
	
									blank_info.sEvtName = "";
									blank_info.sEvtText = "";
									blank_info.sEvtExtern = "";
				
									blank_info.iEventID += 1;
									blank_info.iRefEventID += 1;
	
									alEPGEvent[i].add(blank_info);
									prev_info = blank_info;
								}
							}
	
							iLast_TableID = cursor.getInt(17);
							cursor.moveToNext();
						}
					}

					if(cursor != null)
					{
						cursor.close();
					}
				}
			}
			
			//  make blank information
			if(alEPGEvent[i].size() == 0)
			{
				int iMJD, iTime;

				for(j=0; j<EPG_3LIST_TIME_COUNT; j++)
				{
					iMJD = iCurrent_MJD;
					iTime = iCurrent_Time + j;
					if(iTime > 23)
					{
						iMJD += 1;
						iTime -= 24;
					}
					
					blank_info = new EPGEvent(iMJD, iTime, i, j, datetime);
					alEPGEvent[i].add(blank_info);
				}
			}
		}

		//  merge information of EPG event
		for(i=0; i<EPG_3LIST_LIST_COUNT; i++)
		{
			count = alEPGEvent[i].size();
			for(j=0; j<count; j++)
			{
				cur_info = alEPGEvent[i].get(j);
				if(cur_info.iWeight > 0)
				{
					for(k=i+1; k<EPG_3LIST_LIST_COUNT; k++)
					{	
						ref_count = alEPGEvent[k].size();
						for(l=0; l<ref_count; l++)
						{
							ref_info = alEPGEvent[k].get(l);
							if((ref_info.iChannelNumber < 0) || (cur_info.iChannelNumber != ref_info.iChannelNumber)) 
							{ 
								break;
							}
							
							if((cur_info.iServiceID == ref_info.iRefServiceID) && (cur_info.iEventID == ref_info.iRefEventID)
								&& (cur_info.iStartHH == ref_info.iStartHH) && (cur_info.iStartMM == ref_info.iStartMM) 
								&& (cur_info.iEndHH == ref_info.iEndHH) && (cur_info.iEndMM == ref_info.iEndMM)) 
							{
								cur_info.iWeight += ref_info.iWeight;
								
								ref_info.iIndex_RefList = cur_info.iIndex_RefList;
								ref_info.iIndex_RefEPGEvent = cur_info.iIndex_RefEPGEvent;
								ref_info.iWeight = 0;
							}
						}
					}
				}
			}
		}
	}
	
	private void updateEPGEventComp()
	{
		EPGEvent info, temp;
		cEPGEvent blank_comp, prev_comp, cur_comp, ref_comp;
		int y, y_index, y_hour, y_min;
		int i, j, k;
		int count, num;
		int height_limit;
		float height, height_per_min, duration;

		DxbLog(I, "updateEPGEventComp()");

		height = LIST_BOX_HEIGHT;
		height_per_min = height / 60;
		height_limit = LIST_POS_Y + ((TIME_BOX_HEIGHT + TIME_GAP_HEIGHT) * EPG_3LIST_TIME_COUNT);
				
		//  clear EPG event component
		for(i=0; i<EPG_3LIST_LIST_COUNT; i++)
		{
			int	x = LIST_POS_X + ((LIST_BOX_WIDTH + LIST_GAP_WIDTH) * i);
		
			for(j=0; j<EPG_3LIST_EPGEVENT_COUNT; j++)
			{
				blank_comp = (cEPGEvent)alcEPGEvent[i].get(j);

				blank_comp.lpEPGEvent.x		= x;
				blank_comp.lpEPGEvent.y		= LIST_POS_Y;
				blank_comp.lpEPGEvent.width	= LIST_BOX_WIDTH;
				blank_comp.lpEPGEvent.height	= LIST_BOX_HEIGHT;
				
				blank_comp.tvEPGEvent.setText("");
				blank_comp.tvEPGEvent.setBackgroundColor(0xFFDDDDDD);
				blank_comp.tvEPGEvent.setVisibility(View.GONE);
			}
		}		
		
		//  arrange EPG event component
		for(i=0; i<EPG_3LIST_LIST_COUNT; i++)
		{
			y =  LIST_POS_Y;
			
			count = alEPGEvent[i].size();
			if(count > EPG_3LIST_EPGEVENT_COUNT)
			{
				DxbLog(E, "the number of EPG evnet is " + count);
				count = EPG_3LIST_EPGEVENT_COUNT;
			}

			for(j=0; j<count; j++)
			{
				info = (EPGEvent)alEPGEvent[i].get(j);
				cur_comp = (cEPGEvent)alcEPGEvent[i].get(j);

				if((y > LIST_POS_Y) && (info.iStartMM == 0))
				{
					y_index = (y - LIST_POS_Y) / (LIST_BOX_HEIGHT + LIST_GAP_HEIGHT);
					y_hour = iCurrent_Time + y_index;
					if(y_hour > 23)
					{
						y_hour -= 24;
					}

					if(info.iStartHH == y_hour)
					{
						y_min = (y - LIST_POS_Y) % (LIST_BOX_HEIGHT + LIST_GAP_HEIGHT);

						for(k=j-1; k>0; k--)
						{
							prev_comp = (cEPGEvent)alcEPGEvent[i].get(k);
							if(prev_comp.lpEPGEvent.height > y_min)
							{
								prev_comp.lpEPGEvent.height -= y_min;
								y -=  y_min;
								
								break;
							}
							else
							{
								prev_comp.lpEPGEvent.y -= y_min;									
							}
						}						
					}
					else
					{
						y_min = LIST_BOX_HEIGHT - ((y - LIST_POS_Y) % (LIST_BOX_HEIGHT + LIST_GAP_HEIGHT));

						if(j > 0)
						{
							prev_comp = (cEPGEvent)alcEPGEvent[i].get(j-1);
							prev_comp.lpEPGEvent.height += y_min;
							y +=  y_min;
						}
					}
				}

				if((y > LIST_POS_Y) && (info.iWeight == 0))
				{
					ref_comp = (cEPGEvent)alcEPGEvent[info.iIndex_RefList].get(info.iIndex_RefEPGEvent);
					
					if(ref_comp.lpEPGEvent.y < y)
					{
						y_min = y - ref_comp.lpEPGEvent.y;

						for(k=j-1; k>0; k--)
						{
							prev_comp = (cEPGEvent)alcEPGEvent[i].get(k);
							if(prev_comp.lpEPGEvent.height > y_min)
							{
								prev_comp.lpEPGEvent.height -= y_min;
								y -=  y_min;
								
								break;
							}
							else
							{
								prev_comp.lpEPGEvent.y -= y_min;									
							}
						}						
					}
					else
					{
						y_min = ref_comp.lpEPGEvent.y - y;

						if(j > 0)
						{
							prev_comp = (cEPGEvent)alcEPGEvent[i].get(j-1);
							prev_comp.lpEPGEvent.height += y_min;
							y +=  y_min;
						}
					}
				}
				
				cur_comp.lpEPGEvent.y = y;
				
				if(info.iWeight > 0)
				{
					cur_comp.lpEPGEvent.width = (info.iWeight * LIST_BOX_WIDTH) + ((info.iWeight - 1) * LIST_GAP_WIDTH);
				}
				else
				{
					cur_comp.lpEPGEvent.width = 0;
				}

				if((info.iType == EPGEvent.TYPE_PRESENT && info.iEndHH == 0xFF)
					|| (info.iType == EPGEvent.TYPE_FOLLOWING && info.iEndHH == 0xFF))
				{
					if(j+1 >= count) 
					{
						duration = (height_limit - y) / height_per_min;
					}
					else
					{
						duration = 60;
					}

					for(num=1, k=j+1; k<count; num++, k++)
					{
						temp = (EPGEvent)alEPGEvent[i].get(k);
						if(temp.iStartHH != 0xFF)
						{
							duration = (((temp.iMJD * 24 * 60) + (temp.iStartHH * 60) + temp.iStartMM) - ((info.iMJD * 24 * 60) + (info.iStartHH * 60) + info.iStartMM)) / num;
							break;
						}		
						else if(temp.iEndHH != 0xFF)
						{
							duration = (((temp.iMJD * 24 * 60) + (temp.iEndHH * 60) + temp.iEndMM) - ((info.iMJD * 24 * 60) + (info.iStartHH * 60) + info.iStartMM))  / (num + 1);
							break;
						}						
					}
				}
				else if(info.iType == EPGEvent.TYPE_FOLLOWING && info.iStartHH == 0xFF)
				{
					duration = 60;
	
					for(num=1, k=j-1; k>=0; num++, k--)
					{
						temp = (EPGEvent)alEPGEvent[i].get(k);
						if(temp.iEndHH != 0xFF)
						{
							duration = (((info.iMJD * 24 * 60) + (info.iEndHH * 60) + info.iEndMM) - ((temp.iMJD * 24 * 60) + (temp.iEndHH * 60) + temp.iEndMM)) /num;
							break;
						}		
						else if(temp.iStartHH != 0xFF)
						{
							duration = (((info.iMJD * 24 * 60) + (info.iEndHH * 60) + info.iEndMM) - ((temp.iMJD * 24 * 60) + (temp.iStartHH * 60) + temp.iStartMM)) / (num + 1);
							break;
						}		
					}
				}
				else
				{
					if((info.iMJD < iCurrent_MJD)
						|| ((info.iMJD == iCurrent_MJD) && (info.iStartHH < iCurrent_Time)))
					{
						duration = ((info.iEndHH * 60) + info.iEndMM) - ((((iCurrent_MJD - info.iMJD) * 24) + iCurrent_Time) * 60);
					}
					else
					{
						duration = ((info.iDurationHH * 60) + info.iDurationMM);
					}
				}
				
				cur_comp.lpEPGEvent.height = (int)(Math.round(((duration * height_per_min)*10)/10.0));
				if((y+cur_comp.lpEPGEvent.height) > height_limit)
				{
					cur_comp.lpEPGEvent.height = height_limit - y;
				}

				y += cur_comp.lpEPGEvent.height + LIST_GAP_HEIGHT;
			}
		}

		//  display EPG event component
		for(i=0; i<EPG_3LIST_LIST_COUNT; i++)
		{
			count = alEPGEvent[i].size();
			for(j=0; j<count; j++)
			{
				info = (EPGEvent)alEPGEvent[i].get(j);
				cur_comp = (cEPGEvent)alcEPGEvent[i].get(j);
				
				decorateEPGEvent(info, cur_comp);
				
				cur_comp.tvEPGEvent.setVisibility(View.VISIBLE);
			}
		}
	}
	
	void updateEPGEvent(boolean isMoveDown)
	{
		updateEPGEventInfo(isMoveDown);
		
		updateEPGEventComp();
	}
	
	void setFocus()
	{
		EPGEvent info;
		cEPGEvent comp;
		
		info = alEPGEvent[iIndex_List].get(iIndex_EPGEvent);
		comp = alcEPGEvent[info.iIndex_RefList].get(info.iIndex_RefEPGEvent);
		
		comp.tvEPGEvent.setBackgroundColor(0xFF226886);
		comp.tvEPGEvent.requestFocus();
		
		if(comp.lpEPGEvent.height < LIST_MIN_HEIGHT)
		{
			int limit = LIST_POS_Y + ((TIME_BOX_HEIGHT + TIME_GAP_HEIGHT) * EPG_3LIST_TIME_COUNT);
			
			mEPGEventExtend.lpEPGEvent.x = comp.lpEPGEvent.x;
			mEPGEventExtend.lpEPGEvent.y = comp.lpEPGEvent.y;
			mEPGEventExtend.lpEPGEvent.width = comp.lpEPGEvent.width;
			mEPGEventExtend.lpEPGEvent.height = LIST_MIN_HEIGHT;
			if(limit < mEPGEventExtend.lpEPGEvent.y + mEPGEventExtend.lpEPGEvent.height)
			{
				mEPGEventExtend.lpEPGEvent.y = limit - mEPGEventExtend.lpEPGEvent.height;
			}
			
			decorateEPGEvent(info, mEPGEventExtend);
			
			mEPGEventExtend.tvEPGEvent.setBackgroundColor(0xFF226886);
			mEPGEventExtend.tvEPGEvent.setVisibility(View.VISIBLE);
		}
		else
		{
			mEPGEventExtend.tvEPGEvent.setVisibility(View.GONE);
		}
		
		setEventData(info);
	}

	void clearFocus()
	{
		EPGEvent info;
		cEPGEvent comp;

		mEPGEventExtend.tvEPGEvent.setVisibility(View.GONE);
		
		info = alEPGEvent[iIndex_List].get(iIndex_EPGEvent);
		comp = alcEPGEvent[info.iIndex_RefList].get(info.iIndex_RefEPGEvent);
		
		comp.tvEPGEvent.setBackgroundColor(0xFFDDDDDD);
	}
	
	void updateList()
	{
		Channel channel;
		DateTimeData datetime;
		int i;
		
		DxbLog(I, "start()");
		
		//  open EPG DB
		if(isEPGopen == false)
		{
			if(getPlayer().openEPGDB() == false)
			{
				DxbLog(D, "Cann't open EPG database !!!");
				
				return;
			}
		}

		isEPGopen	= true;

		iCount_Channel	= 0;
		iIndex_Channel	= 0;
		
		iFirst_MJD		= 0;
		iFirst_Time		= 0;
		iLast_MJD		= 0;
		iLast_Time		= 0;
		iCurrent_MJD	= 0;
		iCurrent_Time	= 0;
		
		iIndex_List		= 0;
		iIndex_EPGEvent	= 0;

		//  set channel info
		getPlayer().updateChannelInfo();
		channel = getPlayer().getCurChannel();
		
		alChannel = getPlayer().getChannels(channel.serviceType);
		if((alChannel != null) && (alChannel.size() > 0))
		{
			iCount_Channel	= alChannel.size();
		}

		for(i=0; i<iCount_Channel; i++)
		{
			DxbChannelData temp = (DxbChannelData)alChannel.get(i);
			if(channel.ID == temp.iID)
			{
				iIndex_Channel = i;

				datetime = getPlayer().getCurrentDateTime();
				if((datetime != null) && (datetime.iMJD > 0 && datetime.iHour >= 0))
				{
					iFirst_MJD		= datetime.iMJD;
					iFirst_Time		= datetime.iHour;
					iLast_MJD		= datetime.iMJD;
					iLast_Time		= datetime.iHour;
					iCurrent_MJD	= datetime.iMJD;
					iCurrent_Time	= datetime.iHour;

					break;
				}
				else if(channel.startMJD > 0 && channel.startHH >= 0)
				{
					iFirst_MJD		= channel.startMJD;
					iFirst_Time		= channel.startHH;
					iLast_MJD		= channel.startMJD;
					iLast_Time		= channel.startHH;
					iCurrent_MJD	= channel.startMJD;
					iCurrent_Time	= channel.startHH;

					break;
				}
			}
		}

		updateLastTime();

		updateEPGEvent(false);
		
		updateInfo();
		
		setFocus();
	}

	void shiftEPGEvent(EPGEvent prev_info, int offset)
	{
		EPGEvent cur_info;
		int prev_start_time, cur_start_time, cur_end_time; 
		int count;
		int i;

		prev_start_time = (prev_info.iMJD * 24 * 60) + (prev_info.iStartHH * 60) + prev_info.iStartMM;
		
		count = alEPGEvent[iIndex_List].size();
		for(i=0; i<count; i++)
		{
			cur_info = alEPGEvent[iIndex_List].get(i);
			
			cur_start_time = (cur_info.iMJD * 24 * 60)  + (cur_info.iStartHH * 60) + cur_info.iStartMM;
			cur_end_time = cur_start_time + (cur_info.iDurationHH * 60) + cur_info.iDurationMM;

			
			if(((i == 0) && (prev_start_time < cur_start_time))
				|| (prev_start_time >= cur_start_time) && (prev_start_time < cur_end_time))
			{
				if((i + offset) >= 0 && (i + offset) < count)
				{
					iIndex_EPGEvent = i + offset;				
				}
				else
				{
					iIndex_EPGEvent = i;				
				}
			
				return;
			}
		}
		
		if(count > 0)
		{
			iIndex_EPGEvent = count - 1;
		}
		else
		{
			iIndex_EPGEvent = 0;				
		}
	}
	
	void moveLeft()
	{
		EPGEvent prev_info, cur_info;
		int i;
		
		prev_info =  alEPGEvent[iIndex_List].get(iIndex_EPGEvent);

		clearFocus();
		
		if(iIndex_List == 0)
		{
			if(iCount_Channel < EPG_3LIST_LIST_COUNT)
			{
				DxbLog(D, "gInformation.iCount_Channel is smaller than EPG_3LIST_LIST_COUNT");
				setFocus();
				return;
			}
			
			if(iIndex_Channel == 0)
			{
				iIndex_Channel = iCount_Channel - 1; 
			}
			else
			{
				iIndex_Channel--; 							
			}
			
			updateEPGEvent(false);
			
			shiftEPGEvent(prev_info, 0);
			
			updateInfo();
		}
		else
		{
			for(i=iIndex_List-1; i>=0; i--)
			{
				cur_info = alEPGEvent[i].get(0);
				if(cur_info.iWeight != 0)
				{
					iIndex_List = i;
					
					break;
				}
			}
			
			shiftEPGEvent(prev_info, 0);
			
			updateTitle();
		}
		
		setFocus();
	}
	
	void moveRight()
	{
		EPGEvent prev_info, cur_info;
		int i;
		
		prev_info =  alEPGEvent[iIndex_List].get(iIndex_EPGEvent);
		
		clearFocus();

		if((iIndex_List + prev_info.iWeight) > 2)
		{
			if(iCount_Channel < EPG_3LIST_LIST_COUNT)
			{
				DxbLog(D, "gInformation.iCount_Channel is smaller than EPG_3LIST_LIST_COUNT");
				setFocus();
				
				return;
			}

			iIndex_Channel++;
			if(iIndex_Channel >= iCount_Channel)
			{
				iIndex_Channel = 0; 
			}

			updateEPGEvent(false);
			
			for(i=EPG_3LIST_LIST_COUNT-1; i>=0; i--)
			{
				cur_info = alEPGEvent[i].get(0);
				if(cur_info.iWeight > 0)
				{
					iIndex_List = i;
					
					break;
				}
			}
			
			shiftEPGEvent(prev_info, 0);
			
			updateInfo();
		}
		else
		{
			iIndex_List += prev_info.iWeight;
			
			shiftEPGEvent(prev_info, 0);
			
			updateTitle();
		}

		setFocus();
	}
	
	void moveUp()
	{
		EPGEvent prev_info;
		
		prev_info =  alEPGEvent[iIndex_List].get(iIndex_EPGEvent);
		
		clearFocus();
		
		if(iIndex_EPGEvent == 0)
		{
			if((iCurrent_MJD <= iFirst_MJD) && (iCurrent_Time <= iFirst_Time))
			{
				updateToast("This is first program!");
				setFocus();
				
				return;
			}
			
			if(iCurrent_Time == 0)
			{
				iCurrent_MJD -= 1;
				iCurrent_Time = 23;
			}
			else
			{
				iCurrent_Time--;
			}
			
			updateEPGEvent(false);
			
			shiftEPGEvent(prev_info, -1);
			
			updateInfo();
		}
		else
		{
			iIndex_EPGEvent--;
			
			updateTitle();
		}

		setFocus();
	}
	
	void moveDown()
	{
		EPGEvent prev_info;
		
		prev_info =  alEPGEvent[iIndex_List].get(iIndex_EPGEvent);
		
		clearFocus();

		if((iIndex_EPGEvent + 1) >= alEPGEvent[iIndex_List].size())
		{
			if((iCurrent_MJD >= iLast_MJD) && (iCurrent_Time >= iLast_Time))
			{
				updateToast("This is last program!");
				setFocus();
				
				return;
			}
			
			if(iCurrent_Time == 23)
			{
				iCurrent_MJD++;
				iCurrent_Time = 0;
			}
			else
			{
				iCurrent_Time++;
			}
			
			updateEPGEvent(true);
			
			shiftEPGEvent(prev_info, 1);
			
			updateInfo();				
		}
		else
		{
			iIndex_EPGEvent++;
			
			updateTitle();
		}
		
		setFocus();
	}
	
	void changeDate(int offset)
	{
		int mjd;
		
		DxbLog(I, "changeDate(" + offset + ")");
		
		mjd = iCurrent_MJD + offset;

		if(mjd >= iFirst_MJD && mjd <= iLast_MJD)
		{
			//  change EPG event
			clearFocus();

			iCurrent_MJD = mjd;
			
			if(mjd == iFirst_MJD)
			{
				iCurrent_Time = iFirst_Time;				
			}
			else
			{
				iCurrent_Time = 0;
			}
			
			iIndex_EPGEvent = 0;
			
			updateEPGEvent(false);
			
			updateInfo();
			
			setFocus();
		}

		updateDate();
	}

	void updateView()
	{
		DateTimeData datetime;

		clearFocus();
		
		iIndex_EPGEvent = 0;
	
		datetime = getPlayer().getCurrentDateTime();
		if(	(datetime != null)
			&& (datetime.iMJD > 0 && datetime.iHour >= 0)
			&& ((iFirst_MJD < datetime.iMJD) || (iFirst_MJD == datetime.iMJD && iFirst_Time < datetime.iHour)))
		{
			iFirst_MJD		= datetime.iMJD;
			iFirst_Time		= datetime.iHour;

			if((iCurrent_MJD < datetime.iMJD) || (iCurrent_MJD == datetime.iMJD && iCurrent_Time < datetime.iHour))
			{
				iCurrent_MJD	= datetime.iMJD;
				iCurrent_Time	= datetime.iHour;
			}

			updateLastTime();
		}

		updateEPGEvent(false);
			
		updateInfo();				
		
		setFocus();
	}

	void showDetails()
	{
		EPGEvent info;
		CharSequence[]	charsequence;
		String	time, text;

		DxbLog(I, "showDetails()");

		info = alEPGEvent[iIndex_List].get(iIndex_EPGEvent);
		
		//  set event name
		tvDetail_Title.setText(info.sEvtName);

		//  set event time
		time = "  " + getDateTime(info);
		tvDetail_Time.setText(time);

		//  set event text, extern and info
		text = new String();
		
		if(info.sEvtText != null)
		{
			text += info.sEvtText + "\n\n";
		}
		
		if(info.sEvtExtern != null)
		{
			text += info.sEvtExtern + "\n\n";
		}

		if((info.iGenre >= 0 ) && (info.iGenre <= 15))
		{
			charsequence= getResources().getTextArray(R.array.epg_genre);
			text += " Genre : " + charsequence[info.iGenre] + "\n";
		}

		if((info.iAudioMode >= 1) && (info.iAudioMode <= 10))
		{
			charsequence	= getResources().getTextArray(R.array.epg_audio_mode);
			if((info.iAudioLang1 == 0) && (info.iAudioLang2 == 0))
			{
				text += " Audio : " + charsequence[info.iAudioMode-1] + "\n";
			}
			else if(info.iAudioLang2 == 0)
			{
				text += " Audio : " + charsequence[info.iAudioMode-1] + ", " + getPlayer().getLang(info.iAudioLang1) + "\n";
			}
			else
			{
				text += " Audio : " + charsequence[info.iAudioMode-1] + ", " + getPlayer().getLang(info.iAudioLang1) + " + " + getPlayer().getLang(info.iAudioLang2) + "\n";
			}
		}		
		
		if(info.iVideoMode > 0)
		{
			text += " Video : " + getPlayer().getVideoMode(info.iVideoMode);
		}
		
		tvDetail_Text.setText(text);

		bDetail_Exit.setTextColor(0xFF226886);
		bDetail_Exit.requestFocus();

		wDetail.showAtLocation(llParent, Gravity.CENTER_HORIZONTAL, 0, 20);
		wDetail.setAnimationStyle(-1);
		wDetail.showAsDropDown(bDetail);
	}

	public void onClick(View v)
	{
		DxbLog(I, "ListenerOnClickEPGEvent - onClick()");
		
		int id = v.getId();

		switch(id)
		{
			case R.id.epg_3list_button_left:
				moveLeft();
			break;
		
			case R.id.epg_3list_button_up:
				moveUp();
			break;

			case R.id.epg_3list_button_down:
				moveDown();
			break;
		
			case R.id.epg_3list_button_right:
				moveRight();
			break;
		
			case R.id.epg_3list_button_detail:
				showDetails();
			break;
			
			case R.id.epg_detail_button_exit:
				wDetail.dismiss();
			break;
		
			case R.id.epg_3list_button_prev:
				if(bPrev.isFocusable() == true)
				{
					changeDate(-1);
				}
			break;
			
			case R.id.epg_3list_button_next:
				if(bNext.isFocusable() == true)
				{
					changeDate(1);
				}
			break;

			case R.id.epg_3list_b_book:
					startScheduler(0);
			return;
									
			case R.id.epg_3list_b_timer:
					startScheduler(1);
			return;
						
			default:
				break;
		}
		
		for(int i=0; i<EPG_3LIST_LIST_COUNT; i++)
		{
			for(int j=0; j<EPG_3LIST_EPGEVENT_COUNT; j++)
			{
				cEPGEvent comp = (cEPGEvent)alcEPGEvent[i].get(j);
				if(comp.iID == id)
				{
					clearFocus();
					
					iIndex_List = i;
					iIndex_EPGEvent = j;
					
					setFocus();
					return;
				}
			}
		}
	}
	
	public void onFocusChange(View v, boolean hasFocus)
	{
		DxbLog(I, "onFocusChange()");
		
		int id = v.getId();
		
		if(id == R.id.epg_3list_b_book)
		{
			if(hasFocus)
			{
				bBook.setBackgroundResource(R.drawable.ics_button);
			}
			else
			{
				bBook.setBackgroundColor(getResources().getColor(R.color.color_red));
			}

		}
		else if(id == R.id.epg_3list_b_timer)
		{
			if(hasFocus)
			{
				bTimer.setBackgroundResource(R.drawable.ics_button);
			}
			else
			{
				bTimer.setBackgroundColor(getResources().getColor(R.color.color_green));
			}

		}
		else if(id == R.id.epg_detail_button_exit)
		{
			if(hasFocus)
			{
				bDetail_Exit.setTextColor(0xFF226886);
			}
			else
			{
				bDetail_Exit.setTextColor(0xFFDDDDDD);
			}
		}
	}

	public boolean onKey(View v, int keyCode, KeyEvent event)
	{
		DxbLog(I, "onKeyDown()");
		
		switch(keyCode)
		{
			case KeyEvent.KEYCODE_DPAD_UP:
				moveUp();
			break;
	
			case KeyEvent.KEYCODE_DPAD_DOWN:
				moveDown();
			break;

			case KeyEvent.KEYCODE_DPAD_LEFT:
				moveLeft();
			break;

			case KeyEvent.KEYCODE_DPAD_RIGHT:
				moveRight();
			break;

			case KeyEvent.KEYCODE_ENTER:
				showDetails();
			break;

			case KeyEvent.KEYCODE_MEDIA_FAST_FORWARD:
				if(bNext.isFocusable() == true)
				{
					changeDate(1);
				}
			break;

			case KeyEvent.KEYCODE_MEDIA_REWIND:
				if(bPrev.isFocusable() == true)
				{
					changeDate(-1);
				}
			break;

			case KeyEvent.KEYCODE_BACK:
				if(wDetail.isShowing())
				{
					wDetail.dismiss();	
				}
			break;

			default:
				return false;
		}
		
		return true;
	}	

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		DxbLog(I, "onKeyDown(keyCode=" + keyCode + ", event=" + event + ")");

		if(		(keyCode == KeyEvent.KEYCODE_BACK)
			||	(keyCode == KeyEvent.KEYCODE_ESCAPE)
		)
		{
			setState(false, VIEW_MAIN);
			
			return true;
		}

		switch(keyCode)
		{
			case KeyEvent.KEYCODE_0:
			{
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

			case KeyEvent.KEYCODE_DPAD_UP:
				moveUp();
				return true;
	
			case KeyEvent.KEYCODE_DPAD_DOWN:
				moveDown();
				return true;

			case KeyEvent.KEYCODE_DPAD_LEFT:
				moveLeft();
				return true;

			case KeyEvent.KEYCODE_DPAD_RIGHT:
				moveRight();
				return true;

			case KeyEvent.KEYCODE_ENTER:
				showDetails();
				return true;

			case KeyEvent.KEYCODE_MEDIA_FAST_FORWARD:
				if(bNext.isFocusable() == true)
				{
					changeDate(1);
				}
				return true;

			case KeyEvent.KEYCODE_MEDIA_REWIND:
				if(bPrev.isFocusable() == true)
				{
					changeDate(-1);
				}
				return true;

			case KeyEvent.KEYCODE_BACK:
				if(wDetail.isShowing())
				{
					wDetail.dismiss();	
				}
				return true;
		}
		
		return false;
	}
	
	public void setVisible(boolean v)
	{
		DxbLog(I, "setVisible(v=" + v + ")");

		setIndicatorVisible(v);
		if (v)
		{
			if(!isBookRecord())
			{	
				bBook.setVisibility(View.INVISIBLE);
				bTimer.setVisibility(View.INVISIBLE);
			}
			
			getContext().getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN | 0x80000000, WindowManager.LayoutParams.FLAG_FULLSCREEN | 0x80000000); 
			getContext().getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
			
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

	private void setEventData(EPGEvent epgevent)
	{
		long	lUTC_Start, lUTC_End;
		
		Date	date	= new Date();
		date.setYear(DxbUtil.getMJD_YY(epgevent.iMJD) - 1900);
		date.setMonth(DxbUtil.getMJD_MM(epgevent.iMJD) - 1);
		date.setDate(DxbUtil.getMJD_DD(epgevent.iMJD));
		date.setHours(epgevent.iStartHH);
		date.setMinutes(epgevent.iStartMM);
		date.setSeconds(0);
		lUTC_Start	= date.getTime();
		
		date.setHours(epgevent.iStartHH + epgevent.iDurationHH);
		date.setMinutes(epgevent.iStartMM + epgevent.iDurationMM);
		lUTC_End	= date.getTime();
		
		getPlayer().setEventData_Time(lUTC_Start, lUTC_End);
	}
	private void startScheduler(int iAction)
	{
		DxbLog(I, "startScheduler(iAction=" + iAction + ")");
		
		getPlayer().setEventData(iAction);

		setState(false, VIEW_SCHEDULE);
	}
}
