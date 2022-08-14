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

import com.telechips.android.dvb.player.DxbPlayer;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.RelativeLayout;

public class DxbView_Teletext extends DxbView implements View.OnClickListener, View.OnFocusChangeListener
{
	private static final int	RESET	= 0xFFFF;	

	private final View	mView;

	private ImageView	mivDisplay;

	private Button		mbuPage;
	private Button		mbuStart;
		
	private Button		mbuRed;
	private Button		mbuGreen;
	private Button		mbuYellow;
	private Button		mbuCyan;
		
	private Button		mbuPrev;
	private Button		mbuNext;
	private Button		mbuFPrev;
	private Button		mbuFNext;
		
	private Button		mbuOff;

	private int miCurrentPage;
	private int miChangePage;

	private int miRedPage;
	private int miGreenPage;
	private int	miYellowPage;
	private int	miCyanPage;
	
	public DxbView_Teletext(Context context, View v)
	{
		super(context);
		mView = v;

		setComponent();
		setListener();
	}

	public void setVisible(boolean v)
	{
		DxbLog(I, "setVisible(" + v + ")");
		
		if(v)
		{
			miCurrentPage = 0;
			getPlayer().playSubtitle(DxbPlayer._OFF_);
			playTeletext();
			mbuNext.requestFocus();
			mivDisplay.setScaleType(ImageView.ScaleType.FIT_XY);
			showSystemUi(false);
		}
		else
		{
			stopTeletext();
		}
		
		mView.setVisibility(v ? View.VISIBLE : View.GONE);
	}

	private void showSystemUi(boolean visible)
	{
		DxbLog(I, "showSystemUi(visible=" + visible + ")");
		
		int flag =		View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
					|	View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
					|	View.SYSTEM_UI_FLAG_LAYOUT_STABLE;
		
		if (!visible)
		{
			// We used the deprecated "STATUS_BAR_HIDDEN" for unbundling
			flag	|=	View.STATUS_BAR_HIDDEN
					|	View.SYSTEM_UI_FLAG_FULLSCREEN
					|	View.SYSTEM_UI_FLAG_HIDE_NAVIGATION;
		}

		RelativeLayout.LayoutParams lp = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.MATCH_PARENT);

		if(visible)
		{
			if(getPlayer().isSTB())
			{
				lp.setMargins(0, 0, 0, 30);
			}
			else
			{
				lp.setMargins(0, 0, 0, 48);
			}
		}
		else
		{
			lp.setMargins(0, 0, 0, 0);
		}
		
		mView.setLayoutParams(lp);
		mView.setSystemUiVisibility(flag);
	}
	
	private void setComponent()
	{
		DxbLog(I, "setComponent()");		
		
		mivDisplay	= (ImageView)mView.findViewById(R.id.ttx_display);		
		mbuPage		= (Button)mView.findViewById(R.id.ttx_page);
		mbuStart		= (Button)mView.findViewById(R.id.ttx_start);		
		mbuRed		= (Button)mView.findViewById(R.id.ttx_red);
		mbuGreen		= (Button)mView.findViewById(R.id.ttx_green);
		mbuYellow	= (Button)mView.findViewById(R.id.ttx_yellow);
		mbuCyan		= (Button)mView.findViewById(R.id.ttx_cyan);
		
		mbuNext		= (Button)mView.findViewById(R.id.ttx_next);
		mbuPrev		= (Button)mView.findViewById(R.id.ttx_prev);
		mbuFNext		= (Button)mView.findViewById(R.id.ttx_fnext);
		mbuFPrev		= (Button)mView.findViewById(R.id.ttx_fprev);
		
		mbuOff		= (Button)mView.findViewById(R.id.ttx_off);

		if (getPlayer().isSTB())
		{
			mView.findViewById(R.id.ttx_margin_right).setVisibility(View.GONE);
			mView.findViewById(R.id.ttx_margin_left).setVisibility(View.GONE);
			mView.findViewById(R.id.ttx_margin_top).setVisibility(View.GONE);
			mView.findViewById(R.id.ttx_margin_bottom).setVisibility(View.GONE);
			mView.findViewById(R.id.ttx_margin_menu).setVisibility(View.GONE);
		}
		
		mbuRed.setVisibility(View.GONE);
		mbuGreen.setVisibility(View.GONE);
		mbuYellow.setVisibility(View.GONE);
		mbuCyan.setVisibility(View.GONE);
	}
	
	private void setListener()
	{
		mbuPage.setOnFocusChangeListener(this);
		mbuPage.setOnClickListener(this);
		
		mbuStart.setOnClickListener(this);

		mbuRed.setOnClickListener(this);
		mbuGreen.setOnClickListener(this);
		mbuYellow.setOnClickListener(this);
		mbuCyan.setOnClickListener(this);		

		mbuPrev.setOnClickListener(this);
		mbuNext.setOnClickListener(this);
		mbuFPrev.setOnClickListener(this);
		mbuFNext.setOnClickListener(this);

		mbuOff.setOnClickListener(this);

		getPlayer().setOnTeletextDataUpdateListener(ListenerOnTeletextDataUpdate);
	}

	private DxbPlayer.OnTeletextDataUpdateListener ListenerOnTeletextDataUpdate = new DxbPlayer.OnTeletextDataUpdateListener()
	{
		public void onTeletextDataUpdate(DxbPlayer player, Bitmap bitmap)
		{
			//DxbLog(I, "onTeletextDataUpdate()");
			
			mivDisplay.setImageBitmap(bitmap);
			
			//DxbLog(D, "mView.getSystemUiVisibility() = " + mView.getSystemUiVisibility());
			if((mView.getSystemUiVisibility() & View.STATUS_BAR_HIDDEN) == 0)
			{
				showSystemUi(false);
			}
		}
	};

	public void onClick(View v)
	{
		DxbLog(I, "ListenerOnClick_TTXButton - onClick()");
		
		int id = v.getId();
		switch(id)
		{
			case R.id.ttx_page:
				//DxbLog(D, "miCurrentPage=" + miCurrentPage + ", miChangePage=" + miChangePage);
				if(miChangePage == RESET)
				{
					getPageEdit();
				}
				else
				{
					changePage(miChangePage);
				}
			break;
			
			case R.id.ttx_start:
				changePage(getPlayer().getTTX_initPage());
			break;
			
			case R.id.ttx_red:
				changePage(miRedPage);
			break;
			
			case R.id.ttx_green:
				changePage(miGreenPage);
			break;
			
			case R.id.ttx_yellow:
				changePage(miYellowPage);
			break;
			
			case R.id.ttx_cyan:
				changePage(miCyanPage);
			break;
			
			case R.id.ttx_prev:
				miChangePage	= (miCurrentPage+899) % 900;
				changePage(miChangePage);
			break;
			
			case R.id.ttx_next:
				miChangePage	= (miCurrentPage+1) % 900;
				changePage(miChangePage);
			break;
			
			case R.id.ttx_fprev:
				miChangePage	= (miCurrentPage+800) % 900;
				changePage(miChangePage);
			break;
			
			case R.id.ttx_fnext:
				miChangePage	= (miCurrentPage+100) % 900;
				changePage(miChangePage);
			break;
			
			case R.id.ttx_off:
				setState(false, VIEW_MAIN);
			break;
		}
	}
	
	View		layout_input_page;
	AlertDialog	ad_input_page	= null;
	private void getPageEdit()
	{
		DxbLog(D, "getPageEdit()");
		
		LayoutInflater	inflater_input_page	= (LayoutInflater)getContext().getSystemService(getContext().LAYOUT_INFLATER_SERVICE);
		layout_input_page	= inflater_input_page.inflate(R.layout.dxb_dialog_ttx_input_page, (ViewGroup)getContext().findViewById(R.id.etTTX_inputPage));
		
		AlertDialog.Builder	aDialog_input_page	= new AlertDialog.Builder(getContext());
		aDialog_input_page.setTitle(R.string.please_enter_page);
		aDialog_input_page.setView(layout_input_page);
		
		aDialog_input_page.setPositiveButton(R.string.enter, new DialogInterface.OnClickListener()
		{
			public void onClick(DialogInterface dialog, int which)
			{
				EditText	etInputPage	= (EditText)layout_input_page.findViewById(R.id.etTTX_inputPage);
				miChangePage	= Integer.parseInt(etInputPage.getText().toString());
				changePage(miChangePage);
			}
		});
		aDialog_input_page.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener()
		{
			public void onClick(DialogInterface dialog, int which)
			{
				ad_input_page.dismiss();
			}
		});
		aDialog_input_page.setOnKeyListener(new DialogInterface.OnKeyListener()
		{
			public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event)
			{
				if(		(keyCode == KeyEvent.KEYCODE_DPAD_DOWN)
					||	(keyCode == KeyEvent.KEYCODE_ENTER)
				)
				{
					EditText	editText	= (EditText)layout_input_page.findViewById(R.id.etTTX_inputPage);
					InputMethodManager	imm	= (InputMethodManager)getContext().getSystemService(getContext().INPUT_METHOD_SERVICE);
					imm.hideSoftInputFromWindow(editText.getWindowToken(), 0);
				}
				else if(keyCode == KeyEvent.KEYCODE_DPAD_UP)
				{
					EditText	editText	= (EditText)layout_input_page.findViewById(R.id.etTTX_inputPage);
					InputMethodManager	imm	= (InputMethodManager)getContext().getSystemService(getContext().INPUT_METHOD_SERVICE);
					imm.showSoftInput(editText, InputMethodManager.SHOW_FORCED);
				}
				
				return false;
			}
		});
		
		ad_input_page	= aDialog_input_page.create();
		ad_input_page.show();
	}

	public void onFocusChange(View v, boolean hasFocus)
	{
		DxbLog(I, "onFocusChange()");

		int id = v.getId();
		switch(id)
		{
			case R.id.ttx_page:
				if(hasFocus == true)
				{
					mbuPage.setBackgroundResource(R.drawable.ics_button);
				}
				else
				{
					mbuPage.setBackgroundResource(R.drawable.dxb_portable_ttx_input_box);
				}
			break;
		}
	}
	
	private Runnable mRunnable_Flash = new Runnable() {
		public void run() {
		}
	};
	
	private void playTeletext()
	{
		DxbLog(I, "playTeletext()");
	
		if (getPlayer().isValid())
		{
			getPlayer().playTeletext();
			getMainHandler().removeCallbacks(mRunnable_Flash);		
			changePage(getPlayer().getTTX_initPage());		
		}
	}
	
	private void stopTeletext() {
		DxbLog(I, "stopTeletext()");
		getMainHandler().removeCallbacks(mRunnable_Flash);
		getPlayer().stopTeletext();
	}
		
	private void resetInformation()
	{
		miChangePage	= RESET;
		miRedPage		= RESET;
		miGreenPage		= RESET;
		miYellowPage	= RESET;
		miCyanPage		= RESET;
	}
	
	private void changePage(int page)
	{
		DxbLog(D, "changePage(page=" + page + ")");
		
		if(page == RESET)
		{
			return;
		}
		
		if(page == 999)// Debugging mode
		{
			getPlayer().setTeletext_UpdatePage(page);
		}
		else if(page < 900)
		{	
			if(miCurrentPage != (page%800))
			{
				getMainHandler().removeCallbacks(mRunnable_Flash);
				miCurrentPage	= page;
				resetInformation();
				
				if (getPlayer().isValid())
				{
					if ((page >= 0) && (page < 900))
					{
						getPlayer().setTeletext_UpdatePage(page%800);
						String DisplayPage_Str	= String.format("P%03d", miCurrentPage);
						mbuPage.setTextColor(Color.RED);
						mbuPage.setText(DisplayPage_Str);		
					}
				}
			}			
		}
	}	
		
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		DxbLog(I, "onKeyDown_TTX(keyCode="+keyCode+", event="+event+")");

		if (keyCode == KeyEvent.KEYCODE_BACK || keyCode == KeyEvent.KEYCODE_ESCAPE) {
			setState(false, VIEW_MAIN);
			return true;
		}

		switch(keyCode) {
		case KeyEvent.KEYCODE_0:
		case KeyEvent.KEYCODE_1:
		case KeyEvent.KEYCODE_2:
		case KeyEvent.KEYCODE_3:
		case KeyEvent.KEYCODE_4:
		case KeyEvent.KEYCODE_5:
		case KeyEvent.KEYCODE_6:
		case KeyEvent.KEYCODE_7:
		case KeyEvent.KEYCODE_8:
		case KeyEvent.KEYCODE_9:
			mbuPage.requestFocus();
			if(miChangePage != RESET) {
				miChangePage	= (miChangePage*10+(keyCode-7)) % 1000;
			} else {
				miChangePage	= keyCode-7;
			}
			String DisplayPage_Str	= String.format("P%03d", miChangePage);
			mbuPage.setTextColor(Color.RED);
			mbuPage.setText(DisplayPage_Str);
			break;

		default:
			return false;
		}
		return true;
	}
	
	@Override
	protected String getClassName() {
		return "[[[DxbView_Teletext]]]";
	}
}
