package com.telechips.android.atsc.samples;

import android.content.Context;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.WindowManager;
import android.widget.PopupWindow;
import android.util.Log;

public class Dxbcommon_Popup 
{
	private static final String TAG = "[[[Dxbcommon_Popup]]]";
	
	protected final View anchor;
	protected final PopupWindow window;
	private View root;
	private Drawable background = null;
	protected final WindowManager windowManager;
	
	public Dxbcommon_Popup (View anchor)
	{
		Log.d(TAG, "Dxbcommon_Popup");
		this.anchor = anchor;
		this.window = new PopupWindow(anchor.getContext());
		
		window.setTouchInterceptor(new OnTouchListener()
		{
			//@Override
			public boolean onTouch(View v, MotionEvent event)
			{
				Log.d(TAG, "Dxbcommon_Popup onTouch ");
				Log.d(TAG, "event.getAction()="+event.getAction());
				if(event.getAction() == MotionEvent.ACTION_OUTSIDE)
				{
					Dxbcommon_Popup.this.window.dismiss();
					if( SampleATSCPlayer.NoSignalEnable > 6)
						SampleATSCPlayer.NoSignalEnable = 0;
					if( SampleATSCPlayer.NoServiceEnable >= 2)
						SampleATSCPlayer.NoServiceEnable = 0;
					if( SampleATSCPlayer.ScrambledEnable >= 2)
						SampleATSCPlayer.ScrambledEnable = 0;
					return true;
				}
				return false;
			}
		});
		windowManager = (WindowManager)anchor.getContext().getSystemService(Context.WINDOW_SERVICE);
		onCreate();
	}
	
	protected void onCreate() {}
	protected void onShow() {}
	protected void preShow()
	{
		Log.d(TAG, "preShow");
		if( root == null )
		{
			throw new IllegalStateException("IllegalStateException preShow");
		}
		onShow();
		
		if(background == null)
		{
			window.setBackgroundDrawable(new BitmapDrawable());
		}
		else
		{
			window.setBackgroundDrawable(background);
		}
		
		window.setWidth(WindowManager.LayoutParams.WRAP_CONTENT);
		window.setHeight(WindowManager.LayoutParams.WRAP_CONTENT);
		window.setTouchable(true);
		window.setFocusable(false);
		window.setOutsideTouchable(true);
		window.setContentView(root);
	}
	
	public void setBackgroundDrawable(Drawable backgroud)
	{
		this.background = background;
	}
	
	public void setContentView(View root)
	{
		this.root = root;
		window.setContentView(root);
	}
	
	public void setContentView(int layoutResID)
	{
		LayoutInflater inflater = (LayoutInflater)anchor.getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		setContentView(inflater.inflate(layoutResID, null));
	}
	
	public void setOnDismissListener(PopupWindow.OnDismissListener listener)
	{
		window.setOnDismissListener(listener);
	}
	
	public void dismiss()
	{
		window.dismiss();
	}
	
	public boolean isshown()
	{
		return window.isShowing();
	}

}
