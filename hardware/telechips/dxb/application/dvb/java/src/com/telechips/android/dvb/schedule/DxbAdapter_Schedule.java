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

package com.telechips.android.dvb.schedule;

import java.text.SimpleDateFormat;
import java.util.Date;

import com.telechips.android.dvb.R;

import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.SimpleCursorAdapter;
import android.widget.TextView;

public class DxbAdapter_Schedule extends SimpleCursorAdapter
{
	SimpleDateFormat	formatDate	= new SimpleDateFormat("MM/dd");
	SimpleDateFormat	formatTime	= new SimpleDateFormat("HH:mm");
	
	Date	dateStart	= new Date(0);
	Date	dateEnd		= new Date(0);
	
	class ViewHolder
	{
		TextView	tvIndex;
		TextView	tvName;
		
		TextView	tvDate;
		TextView	tvTime;
		
		TextView	tvRepeat;
		ImageView	ivStatus;
	}
	
	class QueryHandler extends AsyncQueryHandler
	{
		QueryHandler(ContentResolver res)
		{
			super(res);
		}
	}

	public DxbAdapter_Schedule(Context context, int layout, Cursor c, String[] from, int[] to)
	{
		super(context, layout, c, from, to);
		
		DxbLog(I, "DxbAdapter_Schedule()");
	}
	
	public View newView(Context context, Cursor cursor, ViewGroup parent)
	{
		View v = super.newView(context, cursor, parent);
		ViewHolder vh = new ViewHolder();

		vh.tvIndex	= (TextView)v.findViewById(R.id.schedule_row_index);
		vh.tvName	= (TextView)v.findViewById(R.id.schedule_row_name);
		
		vh.tvDate	= (TextView)v.findViewById(R.id.schedule_row_date);
		vh.tvTime	= (TextView)v.findViewById(R.id.schedule_row_time);
		
		vh.tvRepeat	= (TextView)v.findViewById(R.id.schedule_row_repeat);
		vh.ivStatus	= (ImageView)v.findViewById(R.id.schedule_row_status);

		v.setTag(vh);

		return v;
	}
	
	public void bindView(View view, Context context, Cursor cursor)
	{
		DxbLog(I, "bindView(positon=" + cursor.getPosition() + ")");
		
		ViewHolder	vh	= (ViewHolder)view.getTag();
		
		int	iRepeat = -1, iCount = -1;
		if(cursor != null)
		{
			if(cursor.getCount() > 0)
			{
				iRepeat	= cursor.getInt(ScheduleDB.iColumnIndex_Repeat);
				iCount	= cursor.getInt(ScheduleDB.iColumnIndex_Count);
			}
		}
		
		vh.tvIndex.setText(getText_Index(cursor));
		vh.tvName.setText(getText_Name(cursor));
		
		vh.tvDate.setText(getText_Date(cursor, iRepeat, iCount));
		vh.tvTime.setText(getText_Time(cursor));
		
		vh.tvRepeat.setText(getText_Repeat(context, iRepeat));
		
		int	iID	= getId_Status(cursor);
		if(iID >= 0)
		{
			vh.ivStatus.setBackgroundResource(iID);
		}
	}
	
	private String getText_Index(Cursor cursor)
	{
		String	return_Text	= "";
		
		if(cursor != null)
		{
			if(cursor.getCount() > 0)
			{
				return_Text	= String.format("%02d", cursor.getPosition() + 1);
			}
		}
		
		return	return_Text;
	}
	
	private String getText_Name(Cursor cursor)
	{
		String	return_Text	= "";
		
		if(cursor != null)
		{
			if(cursor.getCount() > 0)
			{
				return_Text	= cursor.getString(ScheduleDB.iColumnIndex_Name);
			}
		}
		
		return	return_Text;
	}
	
	private String getText_Date(Cursor cursor, int iRepeat, int iCount)
	{
		String return_Text	= "00/00";
		
		if(cursor != null)
		{
			if(cursor.getCount() > 0)
			{
				if(iRepeat == 1)
				{
					long	curTime	= cursor.getLong(ScheduleDB.iColumnIndex_UTC_Start) + (24*60*60*1000)*iCount;	//	1-daily : 24H:60M:60S:1000m
					dateStart.setTime(curTime);
				}
				else if(iRepeat == 2)
				{
					long	curTime	= cursor.getLong(ScheduleDB.iColumnIndex_UTC_Start) + (7*24*60*60*1000)*iCount;	//	2-weekly : 7D:24H:60M:60S:1000m
					dateStart.setTime(curTime);
				}
				else
				{
					dateStart.setTime(cursor.getLong(ScheduleDB.iColumnIndex_UTC_Start));
				}
				
				return_Text	= formatDate.format(dateStart.getTime());
			}
		}
		
		return return_Text;
	}
	
	private String getText_Time(Cursor cursor)
	{
		String return_Text	= "00:00 ~ 00:00";
		
		if(cursor != null)
		{
			if(cursor.getCount() > 0)
			{
				int	iMode	= cursor.getInt(ScheduleDB.iColumnIndex_AlarmType);

				dateStart.setTime(cursor.getLong(ScheduleDB.iColumnIndex_UTC_Start));
				dateEnd.setTime(cursor.getLong(ScheduleDB.iColumnIndex_UTC_End));
				
				if(iMode == 0)
				{
					return_Text		= formatTime.format(dateStart.getTime()) + "~ --.--";
				}
				else
				{
					return_Text		= formatTime.format(dateStart.getTime()) +     "~"     + formatTime.format(dateEnd.getTime());
				}
			}
		}

		return return_Text;
	}
	
	private String getText_Repeat(Context context, int iRepeat)
	{
		String return_Text	= "";
		
		//if(cursor != null)
		{
			//if(cursor.getCount() > 0)
			{
				//int	iRepeat	= cursor.getInt(ScheduleDB.iColumnIndex_Repeat);
				if(iRepeat == 0)
				{
					return_Text	= context.getResources().getString(R.string.once);
				}
				else if(iRepeat == 1)
				{
					return_Text	= context.getResources().getString(R.string.daily);
				}
				else if(iRepeat == 2)
				{
					return_Text	= context.getResources().getString(R.string.weekly);
				}
			}
		}
		
		return return_Text;
	}
	
	private int getId_Status(Cursor cursor)
	{
		DxbLog(I, "getId_Status()");
		
		int	return_id	= -1;
		if(cursor != null)
		{
			if(cursor.getCount() > 0)
			{
				int	iAlarmType	= cursor.getInt(ScheduleDB.iColumnIndex_AlarmType);
				DxbLog(D, "AlarmType=" + iAlarmType);
				if(iAlarmType == 0)
				{
					return_id	= R.drawable.dxb_portable_alarm_02;
				}
				else if(iAlarmType == 1)
				{
					return_id	= R.drawable.dxb_portable_recording_icon_01;
				}
			}
		}
		
		return return_id;
	}

	final int D = 0;
	final int E = 1;
	final int I = 2;
	final int V = 3;
	final int W = 4;
	
	private void DxbLog(int level, String mString)
	{
		String TAG = "[[[DxbAdapter_Schedule]]] ";
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
