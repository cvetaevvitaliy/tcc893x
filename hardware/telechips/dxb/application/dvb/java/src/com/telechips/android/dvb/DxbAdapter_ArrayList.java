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

import java.util.ArrayList;

import android.content.Context;
import android.graphics.Color;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.TextView;

import com.telechips.android.dvb.player.DxbChannelData;
import com.telechips.android.dvb.player.DxbPlayer;

public class DxbAdapter_ArrayList extends ArrayAdapter<DxbChannelData>
{
	private ArrayList<DxbChannelData> items;
	
	public class ViewHolder {
		TextView	name;
		CheckBox	indicator;
	}

	public DxbAdapter_ArrayList(Context context, int resource, ArrayList<DxbChannelData> items)
	{
		super(context, resource, items);

		DxbLog(I, "DxbAdapter_ArrayList()");
		this.items	= items;
	}
	
	protected DxbPlayer getPlayer()
	{
		DVBPlayerActivity	activity	= (DVBPlayerActivity)getContext();
		
		return activity.getPlayer();
	}
	
	@Override
	public View getView(int position, View convertView, ViewGroup parent)
	{
		DxbLog(I, "getView(position=" + position + ")");
		ViewHolder vh = new ViewHolder();
		String	serviceName;
		
		View v	= convertView;
		if(v == null)
		{
			LayoutInflater vi	= (LayoutInflater)getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			DVBPlayerActivity	activity	= (DVBPlayerActivity)getContext();
			if(getPlayer().isSTB())
			{
				v	= vi.inflate(R.layout.dxb_list_row_60px_stb, null);
			}
			else
			{
				if(activity.cCOMM.iDisplayWidth >= 1024)
			{
				v	= vi.inflate(R.layout.dxb_list_row_60px, null);
			}
			else
			{
				v	= vi.inflate(R.layout.dxb_list_row_40px, null);
			}
		}
		}
		
		DxbChannelData	p	= items.get(position);
		if(p != null)
		{
			vh.name			= (TextView)v.findViewById(R.id.list_row_name);
			vh.indicator	= (CheckBox)v.findViewById(R.id.list_row_indicator);
			
/*				if (mUseIndicator == false)
				vh.indicator.setVisibility(View.GONE);*/
			
			if(p.iLogical_channel == 0)
			{
				serviceName	= p.sChannel_name;
			}
			else
			{
				serviceName	= String.format("%03d", p.iLogical_channel) + " - " + p.sChannel_name;
			}
			
			if(getPlayer().getChannelPosition() != position)
			{
				vh.name.setTextColor(Color.rgb(204, 204, 204));
			}
			else
			{
				vh.name.setTextColor(Color.rgb(248, 196, 0));
			}
			
		    if(p.iFree_CA_mode == 1 || p.iScrambling == 1)
		    {
		   		vh.name.setText(serviceName + " [Scrambled]");
		    }
		    else
		    {
				vh.name.setText(serviceName);
		    }
		    
		    if(p.iBookmark == 1)
		    {
		    	vh.indicator.setChecked(true);
		    }
		    else
		    {
		    	vh.indicator.setChecked(false);
		    }
		}
		
		return v;
	}
	
	private static final int D = 0;
	private static final int E = 1;
	private static final int I = 2;
	private static final int V = 3;
	private static final int W = 4;
	
	private static void DxbLog(int level, String mString)
	{
		String TAG	= "[[DxbAdapter_ArrayList]]";
		
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