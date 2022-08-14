package com.telechips.android.dtvdump;

import java.util.Calendar;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnGenericMotionListener;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.TextView.OnEditorActionListener;

public class dtvDumpActivity extends Activity implements	View.OnClickListener, View.OnKeyListener, View.OnFocusChangeListener,
															RadioGroup.OnCheckedChangeListener, TextWatcher
{
	private final int DTV_NULL	= -1;
	private final int DTV_DVBT	= 0;
	private final int DTV_ISDBT	= 1;
	private final int DTV_DVBS	= 2;
	private final int DTV_ATSC	= 3;
	private final int DTV_IPTV	= 4;
	
	private COMPONENT	comp	= new COMPONENT();
	private class COMPONENT
	{
		GROUP	group	= new GROUP();
		TITLE 	title	= new TITLE();
		SETTING	setting	= new SETTING();
		MEMORY	memory	= new MEMORY();
		BUTTON	button	= new BUTTON();
		MESSAGE	msg		= new MESSAGE();
		
		class GROUP
		{
			View	vBandwidth;
			View	vBandwidth_Edit;
			View	vPOL;
			View	vModulation;
			
			RadioGroup	rgPlayer;
			RadioGroup	rgBandwidth;
			RadioGroup	rgPOL;
			RadioGroup	rgModulation;			
			RadioGroup	rgStorage;
		}
		
		class TITLE
		{
			TextView	tvPlayer;
			TextView	tvChannel;
			TextView	tvBandwidth;
			TextView	tvPOL;
			TextView	tvModulation;			
			TextView	tvStorage;
			TextView	tvName;
		}
		
		class SETTING
		{
			RadioButton	rbPlayer_ISDBT;
			RadioButton	rbPlayer_DVBT;
			RadioButton	rbPlayer_DVBS;
			RadioButton	rbPlayer_ATSC;
			RadioButton rbPlayer_IPTV;
			
			EditText	etChannel;
			TextView	tvChannelKHz;
			
			RadioButton	rbBandwidth_6;
			RadioButton	rbBandwidth_7;
			RadioButton	rbBandwidth_8;
			
			EditText	etBandwidth;
			TextView	tvBandwidthKHz;
			
			RadioButton	rbPOL_V;
			RadioButton	rbPOL_H;
			
			RadioButton	rbModulation_QAM_AUTO;
			RadioButton	rbModulation_VSB_8;
			RadioButton	rbModulation_VSB_16;
			RadioButton	rbModulation_QAM_32;
			RadioButton	rbModulation_QAM_64;
			RadioButton	rbModulation_QAM_128;			
			RadioButton	rbModulation_QAM_256;
			
			RadioButton	rbStorage_usb0;
			RadioButton	rbStorage_usb1;
			RadioButton	rbStorage_sdcard0;
			RadioButton	rbStorage_sdcard1;
			
			EditText	etName;
		}
		
		class MEMORY
		{
			SeekBar		sbState;
			TextView	tvState;
		}
		
		class BUTTON
		{
			Button	bSearch;
			Button	bDump;
		}
		
		class MESSAGE
		{
			TextView	tvInformation_Search1;
			TextView	tvInformation_Search2;
			TextView	tvInformation_Search3;
			TextView	tvInformation_Search4;
			
			TextView	tvSearch;
			TextView	tvDump;
		}
	}
	
/*	private SETTING	set_info	= new SETTING();
	private class SETTING
	{
		int		iPlayer;
		int		iChannel;
		int		iBandwidth;
		int		iStorage;
		String	sName;
	}*/
	
	private DISPLAY	display_info	= new DISPLAY();
	private class DISPLAY
	{
		int	iWidth;
//		int	iHeight;
		
		float	fDensity;
	}
	
	private CURRENT	current_info	= new CURRENT();
	private class CURRENT
	{
		boolean	isSuccess;
		boolean isRecording;
		
		String	sName_Temp;
		
		int	iFreeSize;
	}
	
	private enum MSG
	{
		SEARCH,
		DUMP
	}
	
	private enum MSG_TYPE
	{
		PLAYER,
		CHANNEL,
		BANDWIDTH,
		POL,
		MODULATION,
		STORAGE,
		NAME,
		SEARCH,
		DUMP,
		IP,
		PORT,
	}
	
	private enum STATE_TYPE
	{
		RECORDING,
		REC_STOP
	}
	
	String sStorage[]	= {
		"/storage/usb0",
		"/storage/usb1",
		"/storage/sdcard0",
		"/storage/sdcard1"
	};
	
	private Handler mHandler_Main = new Handler();
	
	
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		DumpLog(I, "onCreate() --> 2013. 02. 27.");
		
		super.onCreate(savedInstanceState);

		getWindow().addFlags(	WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON
							|	WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON);
		setContentView(R.layout.main);
		
		setDisplayInfo();
		
		setComponent();
		setTextSize();
		setListener();
		
		setDefaultValue();
		checkSetting();
	}
	
	public void onPause()
	{
		DumpLog(I, "onPause()");
		
		super.onPause();

		if(current_info.isRecording)
		{
			stopDump();
		}
		DtvDump.deinit();
	}
	
	private void setDefaultValue()
	{
		current_info.isSuccess		= false;
		current_info.isRecording	= false;
		
		current_info.sName_Temp	= "";
	}
	
	@SuppressWarnings("deprecation")
	private void setDisplayInfo()
	{
		DumpLog(I, "setDisplayInfo()");
		
		Display display = ((WindowManager)getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
		display_info.iWidth	= display.getWidth();
//		display_info.iHeight	= display.getHeight();
		
		DisplayMetrics displayMetrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
		display_info.fDensity	= displayMetrics.density;
	}
	
	private void setComponent()
	{
		DumpLog(I, "setComponent()");
		
		comp.group.vBandwidth	= findViewById(R.id.layout_bandwidth);
		comp.group.vBandwidth_Edit	= findViewById(R.id.group_bandwidth_edit);
		comp.group.vPOL				= findViewById(R.id.layout_pol);
		comp.group.vModulation		= findViewById(R.id.layout_modulation);
		
		comp.group.rgPlayer		= (RadioGroup)findViewById(R.id.group_player);
		comp.group.rgBandwidth	= (RadioGroup)findViewById(R.id.group_bandwidth_radio);
		comp.group.rgPOL		= (RadioGroup)findViewById(R.id.group_pol);
		comp.group.rgModulation	= (RadioGroup)findViewById(R.id.group_modulation);
		comp.group.rgStorage	= (RadioGroup)findViewById(R.id.group_storage);
		
		comp.title.tvPlayer		= (TextView)findViewById(R.id.title_player);
		comp.title.tvChannel	= (TextView)findViewById(R.id.title_channel);
		comp.title.tvBandwidth	= (TextView)findViewById(R.id.title_bandwidth);
		comp.title.tvPOL		= (TextView)findViewById(R.id.title_pol);
		comp.title.tvModulation	= (TextView)findViewById(R.id.title_modulation);
		comp.title.tvStorage	= (TextView)findViewById(R.id.title_storage);
		comp.title.tvName		= (TextView)findViewById(R.id.title_name);
		
		comp.setting.rbPlayer_ISDBT		= (RadioButton)findViewById(R.id.setting_player_isdbt);
		comp.setting.rbPlayer_DVBT		= (RadioButton)findViewById(R.id.setting_player_dvbt);
		comp.setting.rbPlayer_DVBS		= (RadioButton)findViewById(R.id.setting_player_dvbs);
		comp.setting.rbPlayer_ATSC			= (RadioButton)findViewById(R.id.setting_player_atsc);
		comp.setting.rbPlayer_IPTV			= (RadioButton)findViewById(R.id.setting_player_iptv);
		comp.setting.etChannel			= (EditText)findViewById(R.id.setting_channel);
		comp.setting.tvChannelKHz		= (TextView)findViewById(R.id.setting_channel_khz);
		comp.setting.rbBandwidth_6		= (RadioButton)findViewById(R.id.setting_bandwidth_6);
		comp.setting.rbBandwidth_7		= (RadioButton)findViewById(R.id.setting_bandwidth_7);
		comp.setting.rbBandwidth_8		= (RadioButton)findViewById(R.id.setting_bandwidth_8);
		comp.setting.etBandwidth		= (EditText)findViewById(R.id.setting_bandwidth_edit);
		comp.setting.tvBandwidthKHz	= (TextView)findViewById(R.id.setting_bandwidth_khz);
		comp.setting.rbPOL_V			= (RadioButton)findViewById(R.id.setting_pol_vertical);
		comp.setting.rbPOL_H			= (RadioButton)findViewById(R.id.setting_pol_horizontal);
		comp.setting.rbModulation_VSB_8		= (RadioButton)findViewById(R.id.setting_modulation_VSB_8);
		comp.setting.rbModulation_VSB_16	= (RadioButton)findViewById(R.id.setting_modulation_VSB_16);
		comp.setting.rbModulation_QAM_32	= (RadioButton)findViewById(R.id.setting_modulation_QAM_32);
		comp.setting.rbModulation_QAM_64	= (RadioButton)findViewById(R.id.setting_modulation_QAM_64);
		comp.setting.rbModulation_QAM_128	= (RadioButton)findViewById(R.id.setting_modulation_QAM_128);
		comp.setting.rbModulation_QAM_256	= (RadioButton)findViewById(R.id.setting_modulation_QAM_256);		
		comp.setting.rbStorage_usb0		= (RadioButton)findViewById(R.id.setting_storage_usb0);
		comp.setting.rbStorage_usb1		= (RadioButton)findViewById(R.id.setting_storage_usb1);
		comp.setting.rbStorage_sdcard0	= (RadioButton)findViewById(R.id.setting_storage_sdcard0);
		comp.setting.rbStorage_sdcard1	= (RadioButton)findViewById(R.id.setting_storage_sdcard1);
		comp.setting.etName				= (EditText)findViewById(R.id.setting_name);

		comp.memory.sbState		= (SeekBar)findViewById(R.id.memory_seekbar);
		comp.memory.tvState		= (TextView)findViewById(R.id.memory_text);
		
		comp.button.bSearch	= (Button)findViewById(R.id.button_search);
		comp.button.bDump	= (Button)findViewById(R.id.button_dump);
		
		comp.msg.tvInformation_Search1	= (TextView)findViewById(R.id.information_search1);
		comp.msg.tvInformation_Search2	= (TextView)findViewById(R.id.information_search2);
		comp.msg.tvInformation_Search3	= (TextView)findViewById(R.id.information_search3);
		comp.msg.tvInformation_Search4	= (TextView)findViewById(R.id.information_search4);
		
		comp.msg.tvSearch	= (TextView)findViewById(R.id.msg_search);
		comp.msg.tvDump		= (TextView)findViewById(R.id.msg_dump);
	}
	
	private void setComponent_Enable(STATE_TYPE type)
	{
		if(type == STATE_TYPE.RECORDING)
		{
			comp.setting.rbPlayer_ISDBT.setEnabled(false);
			comp.setting.rbPlayer_DVBT.setEnabled(false);
			comp.setting.rbPlayer_DVBS.setEnabled(false);
			comp.setting.rbPlayer_ATSC.setEnabled(false);
			comp.setting.rbPlayer_IPTV.setEnabled(false);
			comp.setting.etChannel.setEnabled(false);
			comp.setting.rbBandwidth_6.setEnabled(false);
			comp.setting.rbBandwidth_7.setEnabled(false);
			comp.setting.rbBandwidth_8.setEnabled(false);
			comp.setting.rbPOL_V.setEnabled(false);
			comp.setting.rbPOL_H.setEnabled(false);
			comp.setting.etBandwidth.setEnabled(false);
			comp.setting.rbModulation_VSB_8.setEnabled(false);
			comp.setting.rbModulation_VSB_16.setEnabled(false);
			comp.setting.rbModulation_QAM_32.setEnabled(false);
			comp.setting.rbModulation_QAM_64.setEnabled(false);
			comp.setting.rbModulation_QAM_128.setEnabled(false);
			comp.setting.rbModulation_QAM_256.setEnabled(false);
			comp.setting.rbStorage_usb0.setEnabled(false);
			comp.setting.rbStorage_usb1.setEnabled(false);
			comp.setting.rbStorage_sdcard0.setEnabled(false);
			comp.setting.rbStorage_sdcard1.setEnabled(false);
			comp.setting.etName.setEnabled(false);
			
			comp.button.bSearch.setEnabled(false);
		}
		else if(type == STATE_TYPE.REC_STOP)
		{
			comp.setting.rbPlayer_ISDBT.setEnabled(true);
			comp.setting.rbPlayer_DVBT.setEnabled(true);
			comp.setting.rbPlayer_DVBS.setEnabled(true);
			comp.setting.rbPlayer_ATSC.setEnabled(true);
			comp.setting.rbPlayer_IPTV.setEnabled(true);
			comp.setting.etChannel.setEnabled(true);
			comp.setting.rbBandwidth_6.setEnabled(true);
			comp.setting.rbBandwidth_7.setEnabled(true);
			comp.setting.rbBandwidth_8.setEnabled(true);
			comp.setting.etBandwidth.setEnabled(true);
			comp.setting.rbPOL_V.setEnabled(true);
			comp.setting.rbPOL_H.setEnabled(true);
			comp.setting.rbModulation_VSB_8.setEnabled(true);
			comp.setting.rbModulation_VSB_16.setEnabled(true);
			comp.setting.rbModulation_QAM_32.setEnabled(true);
			comp.setting.rbModulation_QAM_64.setEnabled(true);
			comp.setting.rbModulation_QAM_128.setEnabled(true);
			comp.setting.rbModulation_QAM_256.setEnabled(true);
			comp.setting.rbStorage_usb0.setEnabled(true);
			comp.setting.rbStorage_usb1.setEnabled(true);
			comp.setting.rbStorage_sdcard0.setEnabled(true);
			comp.setting.rbStorage_sdcard1.setEnabled(true);
			comp.setting.etName.setEnabled(true);
			if(getPlayer() == DTV_IPTV)
				comp.button.bSearch.setEnabled(false);
			else
			comp.button.bSearch.setEnabled(true);
		}
	}
	
	private void setTextSize()
	{
		DumpLog(I, "setTextSize()");
		
		int	text_size	= (int)((18*display_info.iWidth/800) / display_info.fDensity);
		int	text_size3	= (int)((12*display_info.iWidth/800) / display_info.fDensity);
		
		comp.title.tvPlayer.setTextSize(text_size);
		comp.title.tvChannel.setTextSize(text_size);
		comp.title.tvBandwidth.setTextSize(text_size);
		comp.title.tvPOL.setTextSize(text_size);
		comp.title.tvModulation.setTextSize(text_size);
		comp.title.tvStorage.setTextSize(text_size);
		comp.title.tvName.setTextSize(text_size);
		
		comp.setting.rbPlayer_ISDBT.setTextSize(text_size);
		comp.setting.rbPlayer_DVBT.setTextSize(text_size);
		comp.setting.rbPlayer_DVBS.setTextSize(text_size);
		comp.setting.rbPlayer_ATSC.setTextSize(text_size);
		comp.setting.rbPlayer_IPTV.setTextSize(text_size);
		comp.setting.etChannel.setTextSize(text_size);
		comp.setting.tvChannelKHz.setTextSize(text_size);
		comp.setting.rbBandwidth_6.setTextSize(text_size);
		comp.setting.rbBandwidth_7.setTextSize(text_size);
		comp.setting.rbBandwidth_8.setTextSize(text_size);
		comp.setting.etBandwidth.setTextSize(text_size);
		comp.setting.tvBandwidthKHz.setTextSize(text_size);
		comp.setting.rbPOL_V.setTextSize(text_size);
		comp.setting.rbPOL_H.setTextSize(text_size);
		comp.setting.rbModulation_VSB_8.setTextSize(text_size3);
		comp.setting.rbModulation_VSB_16.setTextSize(text_size3);
		comp.setting.rbModulation_QAM_32.setTextSize(text_size3);
		comp.setting.rbModulation_QAM_64.setTextSize(text_size3);
		comp.setting.rbModulation_QAM_128.setTextSize(text_size3);		
		comp.setting.rbModulation_QAM_256.setTextSize(text_size3);
		comp.setting.rbStorage_usb0.setTextSize(text_size);
		comp.setting.rbStorage_usb1.setTextSize(text_size);
		comp.setting.rbStorage_sdcard0.setTextSize(text_size);
		comp.setting.rbStorage_sdcard1.setTextSize(text_size);
		comp.setting.etName.setTextSize(text_size);

		comp.memory.tvState.setTextSize(text_size);
		
		comp.button.bSearch.setTextSize(text_size);
		comp.button.bDump.setTextSize(text_size);
		
		comp.msg.tvInformation_Search1.setTextSize(text_size);
		comp.msg.tvInformation_Search2.setTextSize(text_size);
		comp.msg.tvInformation_Search3.setTextSize(text_size);
		comp.msg.tvInformation_Search4.setTextSize(text_size);
		
		comp.msg.tvSearch.setTextSize(text_size);
		comp.msg.tvDump.setTextSize(text_size);
		
		/*	dummy Text	*/
		((TextView)findViewById(R.id.title_dummy1)).setTextSize(text_size);
		((TextView)findViewById(R.id.title_dummy2)).setTextSize(text_size);
		((TextView)findViewById(R.id.title_dummy3)).setTextSize(text_size);
		((TextView)findViewById(R.id.title_dummy4)).setTextSize(text_size);
		((TextView)findViewById(R.id.title_dummy5)).setTextSize(text_size);
	}
	
	private void setListener()
	{
		DumpLog(I, "setListener()");
		
		comp.group.rgPlayer.setOnCheckedChangeListener(this);
		comp.group.rgBandwidth.setOnCheckedChangeListener(this);
		comp.group.rgPOL.setOnCheckedChangeListener(this);
		comp.group.rgModulation.setOnCheckedChangeListener(this);		
		comp.group.rgStorage.setOnCheckedChangeListener(this);
		
		comp.setting.etChannel.setOnFocusChangeListener(this);
		comp.setting.etBandwidth.setOnFocusChangeListener(this);
		comp.setting.etName.setOnFocusChangeListener(this);
		
		comp.setting.etChannel.setOnKeyListener(this);
		comp.setting.etBandwidth.setOnKeyListener(this);
		comp.setting.etName.addTextChangedListener(this);
		
		comp.button.bSearch.setOnClickListener(this);
		comp.button.bDump.setOnClickListener(this);
	}
	
	private int	getPlayer()
	{
		DumpLog(I, "getPlayer()");
		
		int	iIndex;
		switch(comp.group.rgPlayer.getCheckedRadioButtonId())
		{
			case R.id.setting_player_isdbt:
				iIndex	= DTV_ISDBT;
			break;
			
			case R.id.setting_player_dvbt:
				iIndex	= DTV_DVBT;
			break;
			
			case R.id.setting_player_dvbs:
				iIndex	= DTV_DVBS;
			break;
			
			case R.id.setting_player_atsc:
				iIndex	= DTV_ATSC;
			break;

			case R.id.setting_player_iptv:
				iIndex	= DTV_IPTV;
			break;
			
			default:
				iIndex	= DTV_NULL;
			break;
		}

		return iIndex;
	}
	
	private int getChannel()
	{
		DumpLog(I, "getChannel()");
		
		String	sChannel	= comp.setting.etChannel.getText().toString();
		int		iChannel	= 0;
		if(sChannel.length() > 0)
		{
			iChannel	= Integer.parseInt(sChannel);
		}
		
		return iChannel;
	}
	
	private int getBandwidth(int iPlayer)
	{
		DumpLog(I, "getBandwidth(iPlayer = " + iPlayer + ")");
		
		int	iBandwidth	= -1;
		
		if(iPlayer == DTV_DVBT)	// 1-dvbt
		{
		switch(comp.group.rgBandwidth.getCheckedRadioButtonId())
		{
			case R.id.setting_bandwidth_6:
					iBandwidth	= 0;
			break;
			
			case R.id.setting_bandwidth_7:
					iBandwidth	= 1;
			break;
			
			case R.id.setting_bandwidth_8:
					iBandwidth	= 2;
				break;
			}
		}
		else if(iPlayer == DTV_DVBS)	
		{
			String	sBandwidth	= comp.setting.etBandwidth.getText().toString();
			if(sBandwidth.length() > 0)
			{
				iBandwidth	= Integer.parseInt(sBandwidth);
			}
		}

		return iBandwidth;
	}
	
	private int getPOL()
	{
		DumpLog(I, "getPOL()");
		
		int	iIndex	= -1;
		switch(comp.group.rgPOL.getCheckedRadioButtonId())
		{
			case R.id.setting_pol_vertical:
				iIndex	= 0;
			break;
			
			case R.id.setting_pol_horizontal:
				iIndex	= 1;
			break;
		}

		return iIndex;
	}
	
	private int getModulation()
	{
		DumpLog(I, "getModulation()");
		
		int	iIndex	= -1;
		switch(comp.group.rgModulation.getCheckedRadioButtonId())
		{
			case R.id.setting_modulation_VSB_8:
				iIndex	= 0;
				DumpLog(I, "iIndex	= 0");
			break;

			case R.id.setting_modulation_VSB_16:
				iIndex	= 1;
				DumpLog(I, "iIndex	= 1");
			break;

			case R.id.setting_modulation_QAM_32:
				iIndex	= 2;
				DumpLog(I, "iIndex	= 2");
			break;
			
			case R.id.setting_modulation_QAM_64:
				iIndex	= 3;
				DumpLog(I, "iIndex	= 3");
			break;
			
			case R.id.setting_modulation_QAM_128:
				iIndex	= 4;
				DumpLog(I, "iIndex	= 4");
			break;
			
			case R.id.setting_modulation_QAM_256:
				iIndex	= 5;
				DumpLog(I, "iIndex	= 5");
			break;
		}
		
		return iIndex;
	}
	
	private int getStorage()
	{
		DumpLog(I, "getStorage()");
		
		int	iIndex	= -1;
		switch(comp.group.rgStorage.getCheckedRadioButtonId())
		{
			case R.id.setting_storage_usb0:
				iIndex	= 0;
			break;
			
			case R.id.setting_storage_usb1:
				iIndex	= 1;
			break;
			
			case R.id.setting_storage_sdcard0:
				iIndex	= 2;
			break;
			
			case R.id.setting_storage_sdcard1:
				iIndex	= 3;
			break;
		}

		return iIndex;
	}
	
	private String getName()
	{
		String	sName	= comp.setting.etName.getText().toString();
		DumpLog(I, "getName() --> sName=" + sName);
		
		if(sName.length() > 0)
		{
			return	sName;
		}
		
		return	null;
	}
	
	private String getIp()
	{
		DumpLog(I, "getIp()");
		
		String	sIp	= comp.setting.etChannel.getText().toString();
		String	iIp	= "";
		
		if(sIp.length() > 0)
		{
			iIp	= sIp;
		}
		
		return iIp;
	}

	private int getPort()
	{
		DumpLog(I, "getPort()");
		
		int	iPort	= -1;
		
		String	sPort	= comp.setting.etBandwidth.getText().toString();
		if(sPort.length() > 0)
		{
			iPort	= Integer.parseInt(sPort);
		}
		return iPort;
	}

	@SuppressLint("DefaultLocale")
	private void setName(int iPlayer)	// 0-isdbt, 1-dvbt, 2-dvbs
	{
		DumpLog(I, "iPlayer=" + iPlayer);
		String	changeName	= "";
		String	channelName	= "";
		
		DumpLog(D, "comp.setting.etName.getText().toString()=" + comp.setting.etName.getText().toString() + ", current_info.sName_Temp=" + current_info.sName_Temp);
		if(comp.setting.etName.getText().toString().equals(current_info.sName_Temp))
		{
			if(iPlayer == DTV_ISDBT)
			{
				changeName	= getResources().getString(R.string.isdbt);
			}
			else if(iPlayer == DTV_DVBT)
			{
				changeName	= getResources().getString(R.string.dvbt);
			}
			else if(iPlayer == DTV_DVBS)
			{
				changeName	= getResources().getString(R.string.dvbs);
			}
			else if(iPlayer == DTV_ATSC)
			{
				changeName	= getResources().getString(R.string.atsc);
			}
			else if(iPlayer == DTV_IPTV)
			{
				changeName	= getResources().getString(R.string.iptv);
			}
			else
			{
				changeName	= "";
			}

			channelName = comp.setting.etChannel.getText().toString();
			if(iPlayer != DTV_IPTV)
				channelName += comp.setting.tvChannelKHz.getText().toString();
			
			Calendar CurTime = Calendar.getInstance();
			String CurTime_Str	= String.format("%04d%02d%02d_%02d%02d%02d",
												CurTime.get(Calendar.YEAR),
												CurTime.get(Calendar.MONTH),
												CurTime.get(Calendar.DATE),
												
												CurTime.get(Calendar.HOUR),
												CurTime.get(Calendar.MINUTE),
												CurTime.get(Calendar.SECOND)
											);
			
			changeName	= changeName + "_" + channelName + "_" + CurTime_Str;
			
			current_info.sName_Temp	= changeName;
			comp.setting.etName.setText(changeName);
		}
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		switch (keyCode) {
		case KeyEvent.KEYCODE_BACK:
			if (current_info.isRecording)
			{
				Toast a = Toast.makeText(getApplicationContext(), "Now is recording.... Please Stop Dumping !!!!!", Toast.LENGTH_SHORT);
				a.setGravity(Gravity.CENTER, 0, 0);
				a.show();

				return true;
			}	
			break;	
		}
		
		return super.onKeyDown(keyCode, event);
	}

	@Override
	public void onClick(View v)
	{
		int	iID	= v.getId();
		DumpLog(I, "onClick() --> iID = " + iID);
		switch(iID)
		{
			case R.id.button_search:
				startSearch();
			break;
			
			case R.id.button_dump:
				if(current_info.isRecording)
				{
					stopDump();
				}
				else
				{
					if(getPlayer() == DTV_IPTV)
						startSocketDump();
					else
					startDump();
				}
			break;
		}
	}

	@Override
	public boolean onKey(View v, int keyCode, KeyEvent event)
	{
		int	iID	= v.getId();
		DumpLog(I, "onKey() --> iID = " + iID + ", event=" + event);
		
		if(event.getAction() == KeyEvent.ACTION_UP)
		{
			switch(iID)
			{
				case R.id.setting_channel:
					checkSetting_Search();
				break;
				
				case R.id.setting_name:
					checkSetting_Dump();
			}
		}
		
		return false;
	}

	@Override
	public void onFocusChange(View v, boolean hasFocus)
	{
		int	iID	= v.getId();
		DumpLog(I, "onFocusChange() --> iID=" + iID + ", hasFocus=" + hasFocus);
		
		switch(iID)
		{
			case R.id.setting_channel:
				checkSetting_Search();
				if(hasFocus)
				{
					InputMethodManager	imm	= (InputMethodManager)getSystemService(INPUT_METHOD_SERVICE);
					imm.showSoftInput(comp.setting.etChannel, InputMethodManager.SHOW_FORCED);
				}
				else
				{
					InputMethodManager	imm	= (InputMethodManager)getSystemService(INPUT_METHOD_SERVICE);
					imm.hideSoftInputFromWindow(comp.setting.etChannel.getWindowToken(), 0);
				}
			break;
			
			case R.id.setting_bandwidth_edit:
				checkSetting_Search();
				if(hasFocus)
				{
					InputMethodManager	imm	= (InputMethodManager)getSystemService(INPUT_METHOD_SERVICE);
					imm.showSoftInput(comp.setting.etBandwidth, InputMethodManager.SHOW_FORCED);
				}
				else
				{
					InputMethodManager	imm	= (InputMethodManager)getSystemService(INPUT_METHOD_SERVICE);
					imm.hideSoftInputFromWindow(comp.setting.etBandwidth.getWindowToken(), 0);
				}
			break;
			
			case R.id.setting_name:
				checkSetting_Dump();
				if(hasFocus)
				{
					InputMethodManager	imm	= (InputMethodManager)getSystemService(INPUT_METHOD_SERVICE);
					imm.showSoftInput(comp.setting.etName, InputMethodManager.SHOW_FORCED);
					getSystemService(INPUT_METHOD_SERVICE);
					comp.setting.etName.setImeOptions(EditorInfo.IME_ACTION_SEND);
				}
				else
				{
					InputMethodManager	imm	= (InputMethodManager)getSystemService(INPUT_METHOD_SERVICE);
					imm.hideSoftInputFromWindow(comp.setting.etName.getWindowToken(), 0);
				}
			break;
		}
	}

	@Override
	public void onCheckedChanged(RadioGroup group, int checkedId)
	{
		DumpLog(I, "onCheckedChanged() --> checkedId = " + checkedId);
		
		switch(checkedId)
		{
			case R.id.setting_player_isdbt:
			case R.id.setting_player_dvbt:
			case R.id.setting_player_dvbs:
			case R.id.setting_player_atsc:
			case R.id.setting_player_iptv:
			case R.id.setting_bandwidth_6:
			case R.id.setting_bandwidth_7:
			case R.id.setting_bandwidth_8:
			case R.id.setting_pol_vertical:
			case R.id.setting_pol_horizontal:
			case R.id.setting_modulation_VSB_8:
			case R.id.setting_modulation_VSB_16:
			case R.id.setting_modulation_QAM_32:
			case R.id.setting_modulation_QAM_64:
			case R.id.setting_modulation_QAM_128:
			case R.id.setting_modulation_QAM_256:				
			{
				comp.setting.etChannel.setText("");
				comp.setting.etBandwidth.setText("");

				InputMethodManager	imm	= (InputMethodManager)getSystemService(INPUT_METHOD_SERVICE);
				imm.hideSoftInputFromWindow(comp.setting.etChannel.getWindowToken(), 0);
				imm.hideSoftInputFromWindow(comp.setting.etName.getWindowToken(), 0);
			}
			break;
			
			case R.id.setting_storage_usb0:
			case R.id.setting_storage_usb1:
			case R.id.setting_storage_sdcard0:
			case R.id.setting_storage_sdcard1:
			{
				InputMethodManager	imm	= (InputMethodManager)getSystemService(INPUT_METHOD_SERVICE);
				imm.hideSoftInputFromWindow(comp.setting.etChannel.getWindowToken(), 0);
				imm.hideSoftInputFromWindow(comp.setting.etName.getWindowToken(), 0);
				
				updateMemoryInfo();
			}
			break;
		}
		
		switch(checkedId)
		{
			case R.id.setting_player_isdbt:
				comp.title.tvChannel.setText(R.string.channel);
				comp.title.tvBandwidth.setText(R.string.bandwidth);
				comp.setting.etChannel.setInputType(0x00000002);
				comp.setting.tvBandwidthKHz.setText("KHz");
				comp.setting.tvChannelKHz.setText("KHz");
				comp.group.vBandwidth.setVisibility(View.INVISIBLE);
				comp.group.vPOL.setVisibility(View.INVISIBLE);
				comp.group.vModulation.setVisibility(View.GONE);
			break;

			case R.id.setting_player_dvbt:
				comp.title.tvChannel.setText(R.string.channel);
				comp.title.tvBandwidth.setText(R.string.bandwidth);
				comp.setting.etChannel.setInputType(0x00000002);
				comp.setting.tvChannelKHz.setText("KHz");
				comp.setting.tvBandwidthKHz.setText("KHz");
				comp.group.vBandwidth.setVisibility(View.VISIBLE);
				comp.group.rgBandwidth.setVisibility(View.VISIBLE);
				comp.group.vBandwidth_Edit.setVisibility(View.GONE);
				comp.group.vPOL.setVisibility(View.INVISIBLE);
				comp.group.vModulation.setVisibility(View.GONE);				
			break;
		
			case R.id.setting_player_dvbs:
				comp.title.tvChannel.setText(R.string.channel);
				comp.title.tvBandwidth.setText(R.string.bandwidth);
				comp.setting.etChannel.setInputType(0x00000002);
				comp.setting.tvChannelKHz.setText("KHz");
				comp.setting.tvBandwidthKHz.setText("KHz");
				comp.group.vBandwidth.setVisibility(View.VISIBLE);
				comp.group.rgBandwidth.setVisibility(View.GONE);
				comp.group.vBandwidth_Edit.setVisibility(View.VISIBLE);
				comp.group.vPOL.setVisibility(View.VISIBLE);
				comp.group.vModulation.setVisibility(View.GONE);
			break;

			case R.id.setting_player_atsc:
				comp.title.tvChannel.setText(R.string.channel);
				comp.title.tvBandwidth.setText(R.string.bandwidth);
				comp.setting.etChannel.setInputType(0x00000002);
				comp.setting.tvChannelKHz.setText("MHz");
				comp.setting.tvBandwidthKHz.setText("KHz");
				comp.group.vModulation.setVisibility(View.VISIBLE);
				comp.group.vBandwidth.setVisibility(View.GONE);
				comp.group.vBandwidth_Edit.setVisibility(View.GONE);
				comp.group.rgBandwidth.setVisibility(View.GONE);
				comp.group.vPOL.setVisibility(View.GONE);
			break;

			case R.id.setting_player_iptv:
				comp.title.tvChannel.setText(R.string.ip);
				comp.title.tvBandwidth.setText(R.string.port);
				comp.setting.etChannel.setInputType(0x00000001);
				comp.setting.tvChannelKHz.setText("");
				comp.setting.tvBandwidthKHz.setText("");
				comp.group.vModulation.setVisibility(View.GONE);
				comp.group.vBandwidth.setVisibility(View.VISIBLE);
				comp.group.vBandwidth_Edit.setVisibility(View.VISIBLE);
				comp.group.rgBandwidth.setVisibility(View.GONE);
				comp.group.vPOL.setVisibility(View.GONE);
			break;
		}

		checkSetting();
		setName(getPlayer());
	}

	@Override
	public void afterTextChanged(Editable arg0)
	{
		DumpLog(I, "afterTextChanged() --> arg0=" + arg0);
		
		checkSetting_Dump();
	}

	@Override
	public void beforeTextChanged(CharSequence arg0, int arg1, int arg2, int arg3)
	{
		DumpLog(I, "beforeTextChanged() --> arg0=" + arg0 + ", arg1=" + arg1 + ", arg2=" + arg2 + ", arg3=" + arg3);
	}

	@Override
	public void onTextChanged(CharSequence arg0, int arg1, int arg2, int arg3)
	{
		DumpLog(I, "beforeTextChanged() --> arg0=" + arg0 + ", arg1=" + arg1 + ", arg2=" + arg2 + ", arg3=" + arg3);
	}
	
	private Runnable mRunnable_Recording = new Runnable()
	{
		public void run()
		{
			DumpLog(I, "mRunnable_Recording --> run()");
			
			String	sMSG	= comp.msg.tvInformation_Search4.getText().toString();
			if(sMSG.length() <= 9)
			{
				comp.msg.tvInformation_Search4.setText(getResources().getString(R.string.Recording) + " .");
				comp.msg.tvDump.setText(getResources().getString(R.string.Recording) + " .");
			}
			else if(sMSG.length() <= 11)
			{
				comp.msg.tvInformation_Search4.setText(getResources().getString(R.string.Recording) + " . .");
				comp.msg.tvDump.setText(getResources().getString(R.string.Recording) + " . .");
			}
			else if(sMSG.length() <= 13)
			{
				comp.msg.tvInformation_Search4.setText(getResources().getString(R.string.Recording) + " . . .");
				comp.msg.tvDump.setText(getResources().getString(R.string.Recording) + " . . .");
			}
			else if(sMSG.length() >= 15)
			{
				comp.msg.tvInformation_Search4.setText(getResources().getString(R.string.Recording));
				comp.msg.tvDump.setText(getResources().getString(R.string.Recording));
			}
			
			if (updateMemoryInfo() < 0)
			{
				comp.msg.tvInformation_Search3.setText(getResources().getString(R.string.Recording) + "Fail to record !!!!!!!!!!!");
				comp.msg.tvDump.setText(getResources().getString(R.string.Recording) + "Fail to record !!!!!!!!!!!");
			
				stopDump();
			}
			else
			{
				mHandler_Main.postDelayed(mRunnable_Recording, 1000);
			}
		}
	};
	
	private void checkSetting()
	{
		checkSetting_Search();
		checkSetting_Dump();
	}
	
	private void checkSetting_Search()
	{
		DumpLog(I, "checkSetting_Search()");
		
		int	iPlayer	= getPlayer();
		if(iPlayer == DTV_NULL)
		{
			updateMSG(MSG.SEARCH, MSG_TYPE.PLAYER);
			if(comp.button.bSearch.isEnabled())
			{
				comp.button.bSearch.setEnabled(false);
			}
		}
		else if (iPlayer == DTV_IPTV)
		{
			if(comp.button.bSearch.isEnabled())
			{
				comp.button.bSearch.setEnabled(false);
			}
		}
		else if(getChannel() <= 0)
		{
			updateMSG(MSG.SEARCH, MSG_TYPE.CHANNEL);
			if(comp.button.bSearch.isEnabled())
			{
				comp.button.bSearch.setEnabled(false);
			}
		}
		else if (((iPlayer == DTV_DVBT) || (iPlayer == DTV_DVBS)) && (getBandwidth(iPlayer) < 0))
		{
			updateMSG(MSG.SEARCH, MSG_TYPE.BANDWIDTH);
			if(comp.button.bSearch.isEnabled())
			{
				comp.button.bSearch.setEnabled(false);
			}
		}
		else if ((iPlayer == DTV_DVBS) && (getPOL() < 0))
		{
			updateMSG(MSG.SEARCH, MSG_TYPE.POL);
			if(comp.button.bSearch.isEnabled())
			{
				comp.button.bSearch.setEnabled(false);
			}
		}
		else if ((iPlayer == DTV_ATSC) && (getModulation() < 0))
		{
			updateMSG(MSG.SEARCH, MSG_TYPE.MODULATION);
			if (comp.button.bSearch.isEnabled())
			{
				comp.button.bSearch.setEnabled(false);
			}
		}
		else
		{
			updateMSG(MSG.SEARCH, MSG_TYPE.SEARCH);
			if(!comp.button.bSearch.isEnabled())
			{
				comp.button.bSearch.setEnabled(true);
			}
		}
	}
	
	private void checkSetting_Dump()
	{
		DumpLog(I, "checkSetting_Dump()");
		
		if(getStorage() < 0)
		{
			updateMSG(MSG.DUMP, MSG_TYPE.STORAGE);
			if(comp.button.bDump.isEnabled())
			{
				comp.button.bDump.setEnabled(false);
			}
		}
		else if(getName() == null)
		{
			updateMSG(MSG.DUMP, MSG_TYPE.NAME);
			if(comp.button.bDump.isEnabled())
			{
				comp.button.bDump.setEnabled(false);
			}
		}
		else
		{
			if(current_info.isSuccess)
			{
				updateMSG(MSG.DUMP, MSG_TYPE.DUMP);
				if(!comp.button.bDump.isEnabled())
				{
					comp.button.bDump.setEnabled(true);
				}
			}
			else
			{
				if(getPlayer() == DTV_IPTV)
				{
					updateMSG(MSG.DUMP, MSG_TYPE.DUMP);
					if(!validateIPAddress(getIp()))
					{
						updateMSG(MSG.DUMP, MSG_TYPE.IP);
						if(comp.button.bDump.isEnabled())
						{
							comp.button.bDump.setEnabled(false);
						}
					}
					else if(getPort() < 0 || getPort() > 65535)
					{
						updateMSG(MSG.DUMP, MSG_TYPE.PORT);
						if(comp.button.bDump.isEnabled())
						{
							comp.button.bDump.setEnabled(false);
						}
					}
					else if(!comp.button.bDump.isEnabled())
					{
						comp.button.bDump.setEnabled(true);
					}
				}
				else
				{
					updateMSG(MSG.DUMP, MSG_TYPE.SEARCH);
					if(comp.button.bDump.isEnabled())
					{
						comp.button.bDump.setEnabled(false);
					}
				}
			}
		}
	}
	
	private void updateMSG(MSG msg, MSG_TYPE state)
	{
		DumpLog(I, "updateMSG() --> msg=" + msg + ", state=" + state);
		
		updateMSG(msg, state, -1, null);
	}
	
	private void updateMSG(MSG msg, MSG_TYPE state, int iVal)
	{
		DumpLog(I, "updateMSG() --> msg=" + msg + ", state=" + state + ", iVal=" + iVal);
		
		updateMSG(msg, state, iVal, null);
	}
	
	private void updateMSG(MSG msg, MSG_TYPE state, String sVal)
	{
		DumpLog(I, "updateMSG() --> msg=" + msg + ", state=" + state + " sVal=" + sVal);
		
		updateMSG(msg, state, -1, sVal);
	}
	
	private void updateMSG(MSG msg, MSG_TYPE state, int iVal, String sVal)
	{
		DumpLog(I, "updateMSG() --> msg=" + msg + ", state=" + state + ", iVal=" + iVal + " sVal=" + sVal);
		
		String sMSG	= null;
		
		if(msg == MSG.SEARCH)
		{
			if(state == MSG_TYPE.PLAYER)
			{
				sMSG	= getResources().getString(R.string.please_set_player);
			}
			else if(state == MSG_TYPE.CHANNEL)
			{
				sMSG	= getResources().getString(R.string.please_set_channel);
			}
			else if(state == MSG_TYPE.BANDWIDTH)
			{
				sMSG	= getResources().getString(R.string.please_set_bandwidth);
			}
			else if(state == MSG_TYPE.POL)
			{
				sMSG	= getResources().getString(R.string.please_set_pol);
			}
			else if(state == MSG_TYPE.MODULATION)
			{
				sMSG	= getResources().getString(R.string.please_set_modulation);
			}
			else if(state == MSG_TYPE.SEARCH)
			{
				sMSG	= getResources().getString(R.string.please_start_search);
			}
			
			if(sMSG != null)
			{
				comp.msg.tvSearch.setText(sMSG);
			}
		}
		else if(msg == MSG.DUMP)
		{
			if(state == MSG_TYPE.STORAGE)
			{
				sMSG	= getResources().getString(R.string.please_set_storage);
			}
			else if(state == MSG_TYPE.NAME)
			{
				sMSG	= getResources().getString(R.string.please_set_name);
			}
			else if(state == MSG_TYPE.SEARCH)
			{
				sMSG	= getResources().getString(R.string.please_start_search);
			}
			else if(state == MSG_TYPE.DUMP)
			{
				sMSG	= getResources().getString(R.string.please_start_dump);
			}
			else if(state == MSG_TYPE.IP)
			{
				sMSG	= getResources().getString(R.string.please_set_ip);
			}
			else if(state == MSG_TYPE.PORT)
			{
				sMSG	= getResources().getString(R.string.please_set_port);
			}
			
			if(sMSG != null)
			{
				comp.msg.tvDump.setText(sMSG);
			}
		}
	}
	
	private int updateMemoryInfo()
	{
		int	iStorage	= getStorage();
		DumpLog(I, "iStorage = " + iStorage);
		if(iStorage >= 0 && iStorage <= 3)
		{
			current_info.iFreeSize	= DtvDump.getFreeSpace(sStorage[iStorage]);
		}
		
		String	sFreeSize	= current_info.iFreeSize + "KB";
		if(current_info.iFreeSize >= 1024)
		{
			float	changeFreeSize	= (float)current_info.iFreeSize / 1024;
			if(changeFreeSize >= 1024)
			{
				changeFreeSize	= changeFreeSize / 1024;
				
				sFreeSize	= String.format("%.2f", changeFreeSize) + "GB";
			}
			else
			{
				sFreeSize	= String.format("%.2f", changeFreeSize) + "MB";
			}
		}
		
		if(current_info.isRecording)
		{
			int	iUsed	= DtvDump.getSize();
			String	sUsed	= iUsed + "KB";
			if(iUsed >= 1024)
			{
				float	changeUsed	= (float)iUsed / 1024;
				if(changeUsed >= 1024)
				{
					changeUsed	= changeUsed / 1024;
					sUsed	= String.format("%.2f", changeUsed) + "GB";
				}
				else
				{
					sUsed	= String.format("%.2f", changeUsed) + "MB";
				}
			}
			comp.memory.tvState.setText(sUsed + " / " + sFreeSize);
			
			comp.memory.sbState.setProgress(iUsed);
		}
		else
		{
			comp.memory.tvState.setText("0KB / " + sFreeSize);
		}
		
		if (current_info.iFreeSize > 0)
			return 0;

		return -1;
	}

	public boolean validateIPAddress(String iIp) {
		String[] tokens = iIp.split("\\.");

		if(tokens.length != 4)
			return false;

		for(String str : tokens) {
			int tempNum = Integer.parseInt(str);
			if(tempNum > 255 || tempNum < 0)
				return false;
		}
	
		return true;
	}
	
	/*****************************************************************************************************************************************************************************
	 *	call - JNI
	 *****************************************************************************************************************************************************************************/
		private void startSearch()
		{
			DumpLog(I, "startSearch()");
			
			int	RESET	= 0xFF;
			
			int	iState	= RESET;
			String	sInfo1	= "", sInfo2 = "";

			/* Clear Text */
			comp.msg.tvInformation_Search1.setText("");
			comp.msg.tvInformation_Search2.setText("");
			comp.msg.tvInformation_Search3.setText("");
			comp.msg.tvInformation_Search4.setText("");
			comp.msg.tvDump.setText("");

			/* Get Player */
			int	iPlayer		= getPlayer();
			if(iPlayer == DTV_ISDBT)	// 0-isdbt, 1-dvbt, 2-dvbs
			{
				sInfo1	= getResources().getString(R.string.isdbt);
			}
			else if(iPlayer == DTV_DVBT)
			{
				sInfo1	= getResources().getString(R.string.dvbt);
			}
			else if(iPlayer == DTV_DVBS)
			{
				sInfo1	= getResources().getString(R.string.dvbs);
			}
			else if (iPlayer == DTV_ATSC)
			{
				sInfo1	= getResources().getString(R.string.atsc);
			}
			else
			{
				comp.msg.tvInformation_Search1.setText(getResources().getString(R.string.please_set_player));
				comp.msg.tvInformation_Search2.setText("");
				comp.msg.tvInformation_Search3.setText("");
				comp.msg.tvInformation_Search4.setText("");
				
				checkSetting_Search();
				
				return;
			}
			
			/* Get Channel */			
			int	iChannel	= getChannel();
			
			if (iChannel > 0 && (iPlayer == DTV_ISDBT || iPlayer == DTV_DVBT || iPlayer == DTV_DVBS))
			{
				sInfo2	= iChannel + "KHz";
			}
			else if (iChannel > 0 && iPlayer == DTV_ATSC)
			{
				sInfo1	= iChannel + "MHz";
			}
			else
			{
				comp.msg.tvInformation_Search1.setText(getResources().getString(R.string.please_set_channel));
				comp.msg.tvInformation_Search2.setText("");
				comp.msg.tvInformation_Search3.setText("");
				comp.msg.tvInformation_Search4.setText("");
				
				checkSetting_Search();
				
				return;
			}
			
			/* Get BandWidth */
			int	iBandwidth	= getBandwidth(iPlayer);
			
			if ((iPlayer == DTV_DVBT) && (iBandwidth >= 0 && iBandwidth <= 2))
			{
				sInfo2	= sInfo2 + "  /  " + (iBandwidth + 6) + getResources().getString(R.string.mhz);
			}
			else if ((iPlayer == DTV_DVBS) && (iBandwidth > 0))
			{
				sInfo2	= sInfo2 + "  /  " + iBandwidth + getResources().getString(R.string.khz);
			}
			else if (iPlayer != DTV_ISDBT && iPlayer != DTV_ATSC)
			{
				comp.msg.tvInformation_Search1.setText(getResources().getString(R.string.please_set_bandwidth));
				comp.msg.tvInformation_Search2.setText("");
				comp.msg.tvInformation_Search3.setText("");
				comp.msg.tvInformation_Search4.setText("");
				
				checkSetting_Search();
				
				return;
			}
			comp.msg.tvInformation_Search2.setText(sInfo2);
			
			/* Get POL */
			int iPOL	= getPOL();
			
			if ((iPlayer == DTV_DVBS) && (iPOL > -1))
			{
				if(iPOL == 0)
				{
					sInfo1	= sInfo1 + "  :  " + getResources().getString(R.string.v);
				}
				else if(iPOL == 1)
				{
					sInfo1	= sInfo1 + "  :  " + getResources().getString(R.string.h);
				}
			}
			else if ((iPlayer != DTV_ISDBT) && (iPlayer != DTV_DVBT) && (iPlayer != DTV_ATSC))
			{
				comp.msg.tvInformation_Search1.setText(getResources().getString(R.string.please_set_pol));
				comp.msg.tvInformation_Search2.setText("");
				comp.msg.tvInformation_Search3.setText("");
				comp.msg.tvInformation_Search4.setText("");
				
				checkSetting_Search();
				
				return;
			}
			comp.msg.tvInformation_Search1.setText(sInfo1);

			/* Get Modulation */
			int	iModulation = getModulation();
			int iModulation_input = -1;
			
			if (iPlayer == DTV_ATSC)
			{
				if (iModulation >= 0)
				{
					/* Modulation input parameter
					 * 0:	QAM_AUTO
					 * 8:	VSB_8
					 * 16:	VSB_16
					 * 32:	QAM_32
					 * 64:	QAM_64
					 * 128:	QAM_128
					 * 256:	QAM_256
					 */

					if (iModulation == 0)
					{
						sInfo1 = sInfo1 + "  /  " + "VSB_8";
						iModulation_input = 8;
					}
					else if (iModulation == 1)
					{
						sInfo1 = sInfo1 + "  /  " + "VSB_16";
						iModulation_input = 16;
					}
					else if (iModulation == 2)
					{
						sInfo1 = sInfo1 + "  /  " + "QAM_32";
						iModulation_input = 32;
					}
					else if (iModulation == 3)
					{
						sInfo1 = sInfo1 + "  /  " + "QAM_64";
						iModulation_input = 64;
					}

					else if (iModulation == 4)
					{
						sInfo1 = sInfo1 + "  /  " + "QAM_128";
						iModulation_input = 128;
					}

					else if (iModulation == 5)
					{
						sInfo1 = sInfo1 + "  /  " + "QAM_256";
						iModulation_input = 256;
					}
				}
				else
				{
					comp.msg.tvInformation_Search1.setText(getResources().getString(R.string.please_set_modulation));
					comp.msg.tvInformation_Search2.setText("");
					comp.msg.tvInformation_Search3.setText("");
					comp.msg.tvInformation_Search4.setText("");
					
					checkSetting_Search();
				
					return;
				}

				comp.msg.tvInformation_Search1.setText(sInfo1);
			}

			/* Init DtvDump */
			DtvDump.deinit();
			if(iPlayer == DTV_ISDBT)
			{
				iState	= DtvDump.init(iPlayer, iChannel, 0, 1, 0);
			}
			else if(iPlayer == DTV_DVBT)
			{
				iState	= DtvDump.init(iPlayer, iChannel, (iBandwidth + 6)*1000, 1, 0);
			}
			else if(iPlayer == DTV_DVBS)
			{
				iState	= DtvDump.init(iPlayer, iChannel, iBandwidth, iPOL, 0);
			}
			else if (iPlayer == DTV_ATSC)
			{
				iState	= DtvDump.init(iPlayer, iChannel*1000000, 0, 0, iModulation_input);
			}
			
			if(iState == RESET)
			{
				comp.msg.tvInformation_Search3.setText(getResources().getString(R.string.please_check_your_settings));
				
				current_info.isSuccess	= false;
				checkSetting_Dump();
			}
			else if(iState == 0)
			{
				comp.msg.tvInformation_Search3.setText(getResources().getString(R.string.search_success));
				comp.msg.tvSearch.setText(getResources().getString(R.string.search_success));
				
				current_info.isSuccess	= true;
				checkSetting_Dump();
			}
			else
			{
				comp.msg.tvInformation_Search3.setText(getResources().getString(R.string.search_fail));
				comp.msg.tvSearch.setText(getResources().getString(R.string.search_fail));
				
				current_info.isSuccess	= false;
				checkSetting_Dump();
			}
			
			setName(getPlayer());
		}
		
		private void startDump()
		{
			DumpLog(I, "startDump()");
			current_info.isRecording	= true;
			comp.button.bDump.setText(getResources().getString(R.string.stop_dump));
			setComponent_Enable(STATE_TYPE.RECORDING);
			
			setName(getPlayer());
			int	iTable[]	= {
					0x2000
			};
			
			String	sPath	= sStorage[getStorage()] + "/" + comp.setting.etName.getText().toString() + ".ts";
			DumpLog(D, "sPath = " + sPath);

			int result = DtvDump.start(iTable, sPath, current_info.iFreeSize);
			
			if (result >= 0)
			{
				current_info.isRecording = true;
				comp.button.bDump.setText(getResources().getString(R.string.stop_dump));
				setComponent_Enable(STATE_TYPE.RECORDING);

			    comp.msg.tvInformation_Search3.setText(getResources().getString(R.string.Recording));
			    comp.msg.tvDump.setText(getResources().getString(R.string.Recording));
			    
			    mHandler_Main.postDelayed(mRunnable_Recording, 1000);
			    comp.memory.sbState.setMax(current_info.iFreeSize);
			}
			else
			{
				DumpLog(E, "Failed startDump()");

				comp.msg.tvInformation_Search3.setText(getResources().getString(R.string.fail_dump_start));
				comp.msg.tvDump.setText(getResources().getString(R.string.fail_dump_start));

				Toast a = Toast.makeText(getApplicationContext(), "Oops Fail to start dumping.. Please Check Storage or Something !!! ", Toast.LENGTH_SHORT);
				a.setGravity(Gravity.CENTER, 0, 0);
				a.show();
			}
		}

		private void startSocketDump()
		{
			DumpLog(I, "startSocketDump()");
			current_info.isRecording	= true;
			comp.button.bDump.setText(getResources().getString(R.string.stop_dump));
			setComponent_Enable(STATE_TYPE.RECORDING);
			
			setName(getPlayer());
			String 	iIp		= getIp();
			int		iPort	= getPort();
			String	sPath	= sStorage[getStorage()] + "/" + comp.setting.etName.getText().toString() + ".ts";
			DumpLog(D, "sPath = " + sPath);

			int result = DtvDump.sockstart(iIp, iPort, sPath, current_info.iFreeSize);
			
			if (result >= 0)
			{
				current_info.isRecording = true;
				comp.button.bDump.setText(getResources().getString(R.string.stop_dump));
				setComponent_Enable(STATE_TYPE.RECORDING);

				comp.msg.tvInformation_Search1.setText(R.string.iptv);
				comp.msg.tvInformation_Search2.setText(getIp());;
				comp.msg.tvInformation_Search3.setText(getResources().getString(R.string.Recording));
				comp.msg.tvDump.setText(getResources().getString(R.string.Recording));
			
				mHandler_Main.postDelayed(mRunnable_Recording, 1000);
				comp.memory.sbState.setMax(current_info.iFreeSize);
			}
			else
			{
				DumpLog(E, "Failed startDump()");

				comp.msg.tvInformation_Search3.setText(getResources().getString(R.string.fail_dump_start));
				comp.msg.tvDump.setText(getResources().getString(R.string.fail_dump_start));

				Toast a = Toast.makeText(getApplicationContext(), "Oops Fail to start dumping.. Please Check Storage or Something !!! ", Toast.LENGTH_SHORT);
				a.setGravity(Gravity.CENTER, 0, 0);
				a.show();
			}
		}
		
		private void stopDump()
		{
			DumpLog(I, "stopDump()");
			
			mHandler_Main.removeCallbacks(mRunnable_Recording);
			current_info.isRecording	= false;
			comp.button.bDump.setText(getResources().getString(R.string.start_dump));
			
			int	iUsed	= DtvDump.getSize();
			String	sUsed	= iUsed + "KB";
			if(iUsed >= 1024)
			{
				float	changeUsed	= (float)iUsed / 1024;
				if(changeUsed >= 1024)
				{
					changeUsed	= changeUsed / 1024;
					sUsed	= String.format("%.2f", changeUsed) + "GB";
				}
				else
				{
					sUsed	= String.format("%.2f", changeUsed) + "MB";
				}
			}

			DtvDump.stop();
			
			comp.msg.tvDump.setText(getResources().getString(R.string.please_start_dump));
			comp.msg.tvInformation_Search4.setText(getResources().getString(R.string.stopped_record) + " / " + sUsed + "   " + comp.setting.etName.getText().toString() + ".ts");

			setComponent_Enable(STATE_TYPE.REC_STOP);
			setName(getPlayer());
			updateMemoryInfo();
		}
	/*****************************************************************************************************************************************************************************
	 *****************************************************************************************************************************************************************************/

	
	/*****************************************************************************************************************************************************************************
	 *	Debug - log_message
	 *****************************************************************************************************************************************************************************/
		private final int I = 0;
		private final int D = 1;
		private final int V = 2;
		private final int W = 3;
		private final int E = 4;
		
		private String TAG	= "[[[dtvDump]]]";
//		private String TAG	= null;

		protected void DumpLog(int level, String mString)
		{
			if(TAG == null)
			{
				return;
			}
			
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
	/*****************************************************************************************************************************************************************************
	 *****************************************************************************************************************************************************************************/
}