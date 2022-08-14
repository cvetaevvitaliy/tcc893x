package com.telechips.android.atsc.samples;

import java.util.Vector;

import android.content.Context;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.util.Log;

public class DxbPlayer_Popup extends Dxbcommon_Popup
{
	private static final String TAG = "[[[DxbPlayer_Popup]]]";
	
	private final Context context;
	private final LayoutInflater inflater;
	private final View root;
	private ViewGroup mTrack;
	private Vector<DxbPlayer_Item> itemVector = new Vector<DxbPlayer_Item>();
	
	public DxbPlayer_Popup( View anchor)
	{
		super(anchor);
		Log.d(TAG, "DxbPlayer_Popup");
		context = anchor.getContext();
		inflater = (LayoutInflater)context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		root = (ViewGroup)inflater.inflate(R.layout.dxb_popup, null);
		setContentView(root);
		mTrack = (ViewGroup)root.findViewById(R.id.popupview);
	}
	
	public void show()
	{
		Log.d(TAG, "show");
		preShow();
		createList();
		int[] location = new int[2];
		anchor.getLocationOnScreen(location);
		root.setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
		root.measure(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
		window.showAtLocation(this.anchor, Gravity.CENTER, 0, 0);
	}
	
	public void update(int state)
	{
		Log.d(TAG, "update"+ state);
		switch(state)
		{
			case 1:
				window.update(380, -50, -1, -1, true);
				break;
			case 2:
				window.update(0, 0, -1, -1, true);
				break;
			case 3:
				window.update(380, -50, -1, -1, true);
				break;				
		}	
	}
	
	public void additem(DxbPlayer_Item action)
	{
		itemVector.add(action);
	}
	
	private void createList()
	{
		View view;
		String title;
		int res;
		int	index = 0;

		for(int i=0; i<itemVector.size();i++)
		{
			title = itemVector.get(i).getTitle();
			res = itemVector.get(i).getRes();
			view = getItem(title, res);
			view.setFocusable(true);
			view.setClickable(false);
			mTrack.addView(view, index);
			index++;
		}
	}
	
	private View getItem(String title, int res)
	{
		LinearLayout container = (LinearLayout)inflater.inflate(R.layout.dxb_popupview_row, null);
		TextView	text = (TextView)container.findViewById(R.id.popuptitle);
		ImageView 	image = (ImageView)container.findViewById(R.id.popupimage);
		
		if( title != null)
			text.setText(title);
		else
			text.setVisibility(View.GONE);
		image.setImageResource(res);
		container.setPadding(0, 3, 0, 3);
		return container;
	}
}


