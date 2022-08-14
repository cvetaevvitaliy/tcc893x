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

import com.telechips.android.isdbt.player.ChannelManager;
import com.telechips.android.isdbt.player.DxbPlayer;

import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.graphics.Color;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.SimpleCursorAdapter;
import android.widget.TextView;

public class DxbAdapter_Cursor extends SimpleCursorAdapter
{
	private static final String TAG = "[[[DxbAdapter_Cursor]]] ";
	
	private Context	mContext;
	
	private AsyncQueryHandler mQueryHandler;
	private String mConstraint = null;
	private boolean mConstraintIsValid = false;

	private Cursor mCursor;
	private boolean mUseIndicator = true;

	private int	iIndex_serviceType;
	private int	iIndex_serviceName;
	private int iBookmark;

	public class ViewHolder {
		TextView	name;
		CheckBox	indicator;
	}

	class QueryHandler extends AsyncQueryHandler
	{
		QueryHandler(ContentResolver res)
		{
			super(res);
			Log.i(TAG, "QueryHandler()");
		}

		@Override
		protected void onQueryComplete(int token, Object cookie, Cursor cursor)
		{
			Log.i(TAG, "onQueryComplete()");
		}
	}

	public DxbAdapter_Cursor(Context context, int layout, Cursor c, String[] from, int[] to)
	{
		super(context, layout, c, from, to);
		Log.i(TAG, "DxbAdapter_Cursor()");
		
		mContext	= context;
		mCursor		= c;
		mQueryHandler = new QueryHandler(context.getContentResolver());
		
		setService_ColumnIndex(c);
	}

	public AsyncQueryHandler getQueryHandler()
	{
		Log.i(TAG, "AsyncQueryHandler()");
		return mQueryHandler;
	}

	public void setUseIndicator(boolean bShow)
	{
		mUseIndicator = bShow;
	}
	
	private Context getContext()
	{
		return mContext;
	}
	
	protected DxbPlayer getPlayer()
	{
		ISDBTPlayerActivity	activity	= (ISDBTPlayerActivity)getContext();
		
		return activity.getPlayer();
	}

	private void setService_ColumnIndex(Cursor cursor)
	{
		DxbLog(I, "setService_ColumnIndex()");
	
		if(cursor != null)
		{
			iIndex_serviceType	= getPlayer().getColumnIndex_ServiceType();
			iIndex_serviceName	= getPlayer().getColumnIndex_ServiceName();
			iBookmark			= getPlayer().getColumnIndex_Bookmark();
		}
	}

	@Override
	public View newView(Context context, Cursor cursor, ViewGroup parent)
	{
		Log.i(TAG, "newView()");
		View v = super.newView(context, cursor, parent);
		ViewHolder vh = new ViewHolder();
		
		vh.name			= (TextView)	v.findViewById(R.id.list_row_name);
		vh.indicator	= (CheckBox)	v.findViewById(R.id.list_row_indicator);

		if (mUseIndicator == false)
			vh.indicator.setVisibility(View.GONE);

		v.setTag(vh);
		
		return v;
	}

	@Override
	public void bindView(View view, Context context, Cursor cursor)
	{
		DxbLog(I, "bindView()");

		ViewHolder vh = (ViewHolder) view.getTag();
		int		serviceType	= cursor.getInt(iIndex_serviceType);
		String	serviceName	= cursor.getString(iIndex_serviceName);
		int		bookmark	= cursor.getInt(iBookmark);
		int	    position	= cursor.getPosition();

		if (getPlayer().getChannelPosition() != position)
		{
			vh.name.setTextColor(Color.rgb(204, 204, 204));
		}
		else
		{
			vh.name.setTextColor(Color.rgb(248, 196, 0));
		}
			
		int scarmbling_info1 = cursor.getColumnIndexOrThrow(ChannelManager.KEY_FREE_CA_MODE); //free_ca_mode in sdt
		int scarmbling_info2 = cursor.getColumnIndexOrThrow(ChannelManager.KEY_SCRAMBLING); //cas descriptor in pmt
	    scarmbling_info1	= cursor.getInt(scarmbling_info1);
	    scarmbling_info2	= cursor.getInt(scarmbling_info2);
	   
	    Log.d(TAG, serviceName + " : Scramble Infromation " + scarmbling_info1 +" " + scarmbling_info2);

	    if(scarmbling_info1 == 1 || scarmbling_info2 == 1)
	   		vh.name.setText(serviceName + " [Scrambled]");
	    else    
			vh.name.setText(serviceName);

	    Log.d(TAG, "bookmark = " + bookmark);
		if(bookmark == 1)
			vh.indicator.setChecked(true);
		else
			vh.indicator.setChecked(false);
	}

	@Override
	public void changeCursor(Cursor cursor) {
		super.changeCursor(cursor);
		//Log.i(TAG, "changeCursor()");
	}

	@Override
	public Cursor runQueryOnBackgroundThread(CharSequence constraint) {
		//Log.i(TAG, "runQueryOnBackgroundThread()");
		String s = constraint.toString();
		if(mConstraintIsValid && ((s == null && mConstraint == null) || (s != null && s.equals(mConstraint)))) {
			return getCursor();
		}
		Cursor c = mCursor;
		mConstraint = s;
		mConstraintIsValid = true;
		return c;
	}
	
	private static final int D = 0;
	private static final int E = 1;
	private static final int I = 2;
	private static final int V = 3;
	private static final int W = 4;
	
	private static void DxbLog(int level, String mString)
	{
		String TAG	= "[[DxbAdapter_Cursor]]";
		
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
