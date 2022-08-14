package com.telechips.android.atsc.samples;

import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.graphics.Color;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.SimpleCursorAdapter;
import android.widget.TextView;

public class DxbAdapter_EPG extends SimpleCursorAdapter 
{
	private static final String TAG = "[[[DxbAdapter_EPG]]]";
	
	private Cursor cEPG;
	
	static class ViewHolder 
	{
		TextView textTime;
		TextView textName;
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
		Log.d(TAG, "DxbAdapter_EPG() - DxbPlayer_Control.mTotalEPGCount=" + DxbPlayer_Control.mTotalEPGCount);
		
		cEPG = c;
		
		if(DxbPlayer_Control.mTotalEPGCount <= 0) 
		{
			DxbPlayer_Control.epgText_name.setText("");
			DxbPlayer_Control.epgText_detail.setText("");

			SampleATSCPlayer.epgTextView_Current.setText("");
	
			SampleATSCPlayer.epgButton_Prev.setBackgroundResource(R.drawable.dxb_portable_epg_left_btn_d);
			SampleATSCPlayer.epgButton_Prev.setFocusable(false);
			SampleATSCPlayer.epgButton_Next.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_d);
			SampleATSCPlayer.epgButton_Next.setFocusable(false);
		}
		DxbPlayer_Control.setEPG_ColumnIndex(cEPG);
	}

	@Override
	public View newView(Context context, Cursor cursor, ViewGroup parent)
	{
		View v = super.newView(context, cursor, parent);
		ViewHolder hEPG = new ViewHolder();

		hEPG.textTime = (TextView)v.findViewById(R.id.epg_row_time);
		hEPG.textName = (TextView)v.findViewById(R.id.epg_row_name);

		v.setTag(hEPG);
		
		return v;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see android.widget.SimpleCursorAdapter#bindView(android.view.View,
	 * android.content.Context, android.database.Cursor)
	 */
	@Override
	public void bindView(View view, Context context, Cursor cursor) 
	{
		int	position	= cursor.getPosition();
		if(position == 0)
		{
			if(DxbPlayer_Control.mTotalEPGCount > 0) 
			{
				//Log.d(TAG, "bindView() - ");
				DxbPlayer_Control.setEPG_DisplayDate(cursor);

				if( SampleATSCPlayer.mepgFirstday == DxbPlayer_Control.getEPG_StartDay(cursor))
				{
					DxbPlayer_Control.epgFirst = 1;
					SampleATSCPlayer.epgButton_Prev.setBackgroundResource(R.drawable.dxb_portable_epg_left_btn_d);
					SampleATSCPlayer.epgButton_Prev.setFocusable(false);
					if( SampleATSCPlayer.mepgFirstday == SampleATSCPlayer.mepgLastday )
					{
						DxbPlayer_Control.epgLast = 1;
						SampleATSCPlayer.epgButton_Next.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_d);
						SampleATSCPlayer.epgButton_Next.setFocusable(false);
					}
					else
					{
						DxbPlayer_Control.epgLast = 0;
						SampleATSCPlayer.epgButton_Next.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_f);
						SampleATSCPlayer.epgButton_Next.setFocusable(true);
					}
				}
				else if( SampleATSCPlayer.mepgLastday == DxbPlayer_Control.getEPG_StartDay(cursor))
				{
					DxbPlayer_Control.epgFirst = 0;
					DxbPlayer_Control.epgLast = 1;
					SampleATSCPlayer.epgButton_Prev.setBackgroundResource(R.drawable.dxb_portable_epg_left_btn_f);
					SampleATSCPlayer.epgButton_Prev.setFocusable(true);
					SampleATSCPlayer.epgButton_Next.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_d);
					SampleATSCPlayer.epgButton_Next.setFocusable(false);
				}
				else
				{
					DxbPlayer_Control.epgFirst = 0;
					DxbPlayer_Control.epgLast = 0;
					SampleATSCPlayer.epgButton_Prev.setBackgroundResource(R.drawable.dxb_portable_epg_left_btn_f);
					SampleATSCPlayer.epgButton_Prev.setFocusable(true);
					SampleATSCPlayer.epgButton_Next.setBackgroundResource(R.drawable.dxb_portable_epg_right_btn_f);
					SampleATSCPlayer.epgButton_Next.setFocusable(true);
				}
			}
		}

		ViewHolder hEPG = (ViewHolder)view.getTag();
		String sName = DxbPlayer_Control.getEPG_Name(cursor);
		
		hEPG.textTime.setText(DxbPlayer_Control.getEPG_Time(cursor));
		if ( sName.length() > 18 )
		{
			hEPG.textName.setText(sName.substring( 0, 18) +"...");
			Log.d(TAG, "DxbAdapter_EPG() - sName.length() =" + sName.length() );
		}
		else
		{
			hEPG.textName.setText(sName);
	 	}
		
		hEPG.textTime.setTextColor(Color.rgb(224, 224, 224));
		hEPG.textName.setTextColor(Color.rgb(224, 224, 224));
	}
}
