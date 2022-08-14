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

package com.telechips.android.dvb.option;

import com.telechips.android.dvb.DVBPlayerActivity;
import com.telechips.android.dvb.player.DxbPlayer;
import com.telechips.android.dvb.schedule.Alert;
import com.telechips.android.dvb.player.diseqc.DishSetupManager;
import com.telechips.android.dvb.player.diseqc.Dish;
import com.telechips.android.dvb.player.diseqc.Motor;
import com.telechips.android.dvb.player.diseqc.Tuner;
import com.telechips.android.dvb.R;

import android.os.Bundle;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.graphics.Color;
import android.text.InputFilter;
import android.util.Log;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnFocusChangeListener;
import android.view.LayoutInflater;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.CursorAdapter;
import android.widget.TextView;

public class DishSetupActivity extends Activity {

	protected static final String TAG = "DishSetupActivity";

	private DishSetupManager mDishManager;
	private int mSelectedSatID = 0;
	private int mSelectedTsId = 0;
	private int mListTsId = 0;
	
	private Dish	mDish;
	private Motor	mMotor;
	private Tuner mTuner[];

	private TextView mSelectedSatName;
	private ListView mSatList;
	private SatAdapter mSatAdapter;

	private ListView mLnbList;
	private LnbSetAdapter mLnbAdapter;

	private LinearLayout mTsLayout;
	private ListView mTsList;
	private TsAdapter mTsAdapter;
	
	private Cursor cSatInfo	= null;
	private Cursor cSatList	= null;
	private Cursor cTsInfo	= null;
	private Cursor cTsList	= null;

	public static boolean isbackpressed;
	//private boolean isSchedulerStart;
	
	private int switch_flag = 0;

	AlertDialog.Builder builder;	

	public static final int DISHSETUP_LNB_TYPE = 0;
	public static final int DISHSETUP_LNB_POWER = 1;
	public static final int DISHSETUP_LNB_TONE = 2;
	public static final int DISHSETUP_LNB_TONEBURST = 3;
	public static final int DISHSETUP_LNB_DISEQC_MODE = 4;
	public static final int DISHSETUP_LNB_COMMITTED_COMMAND = 5;
	public static final int DISHSETUP_LNB_UNCOMMITTED_COMMAND = 6;
	public static final int DISHSETUP_LNB_DISEQC_REPEATS = 7;
	public static final int DISHSETUP_LNB_COMMAND_ORDER = 8;
	public static final int DISHSETUP_LNB_MOTOR = 9;
	
	private final int LIST_SAT	= 0;
	private final int LIST_TS	= 1;
	private final int LIST_LNB	= 2;
	

	private int LnbSettingItemSelected=0;
	
	private DVBPlayerActivity getMainActivity()
	{
		return DVBPlayerActivity.getInstance();
	}
	
	private DxbPlayer getPlayer()
	{
		return getMainActivity().getPlayer();
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		getPlayer().isDishSetupActivityOn = true;
		getPlayer().stop();
		
		setContentView(R.layout.activity_dish_setup);

		getPlayer().resetDVBSx();
		mDishManager = new DishSetupManager(DishSetupActivity.this);
		mDishManager.open();
		
		mMotor	= mDishManager.getMotoLocation();
		setMotorAngle(mMotor);
		
		initSystemSet();
		
		setComponent();
		setListener();
		
		mLnbAdapter = new LnbSetAdapter(this);
		mLnbList.setAdapter(mLnbAdapter);

		
		

		mSatList.setOnItemSelectedListener(new OnItemSelectedListener() {
			@Override
			public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
				// TODO Auto-generated method stub
				mSelectedSatID = (int)arg3;

				mSelectedSatName.setText((String)arg1.getTag());
				mDish = mDishManager.getDishSetup(mSelectedSatID);

				if (mLnbList.getVisibility() == View.VISIBLE)
					mLnbAdapter.notifyDataSetChanged();
				if (mTsLayout.getVisibility() == View.VISIBLE) {
					mTsAdapter.notifyDataSetChanged();
					updateTsList();
				}
			}

			@Override
			public void onNothingSelected(AdapterView<?> arg0) {
				// TODO Auto-generated method stub
			}
		});

		mSatList.setOnItemClickListener(new AdapterView.OnItemClickListener() {

			@Override
			public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
				if(arg0 == null)
					return;
				
				mSelectedSatID = (int)arg3;
			}
		});
		cSatInfo	= mDishManager.querySatInfo(
				new String[] {"_id", "sat_longitude", "sat_name"}
				, null
				, null
		);
		mSatAdapter = new SatAdapter(DishSetupActivity.this, cSatInfo);
		mSatList.setAdapter(mSatAdapter);
		if(cSatInfo.getCount() > 0)
		{
			mSatList.requestFocus();
		}

		mTsLayout = (LinearLayout)findViewById(R.id.ts_list_layout);
		mTsList.setOnItemSelectedListener(new OnItemSelectedListener()
		{
			@Override
			public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3)
			{
				// TODO Auto-generated method stub
				mSelectedTsId = (int)arg3;
				mListTsId = arg2;
				
				mTuner = mDishManager.getTsList(mSelectedSatID);
				
				Log.d(TAG,"TsList ==> sat_list setOnItemSelectedListener " + mSelectedTsId);
				Log.d(TAG,"TsList ==> sat_list setOnItemSelectedListener " + mListTsId);
			}

			@Override
			public void onNothingSelected(AdapterView<?> arg0) {
				// TODO Auto-generated method stub
			}
		});
		mTsList.setOnItemClickListener(new AdapterView.OnItemClickListener()
		{
			@Override
			public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3)
			{
				if(arg0 == null)
					return;
				
				if((mSelectedTsId != (int)arg3) || (mListTsId != arg2))
				{
					mSelectedTsId = (int)arg3;
					mListTsId = arg2;
					
					mTuner = mDishManager.getTsList(mSelectedSatID);
				}
				
				ContentValues values = new ContentValues();
				if(mTuner[mListTsId].getFecInner() == 0)
				{
					mTuner[mListTsId].setFecInner(1);
					values.put("fec_inner", 1);
				}
				else
				{
					mTuner[mListTsId].setFecInner(0);
					values.put("fec_inner", 0);
				}

				mDishManager.updateTsInfo(values, mTuner[mListTsId].getId(), mDish);
				
				updateTsList();
				mTsList.requestFocus();
				mTsList.setSelection(arg2);
			}
		});
		cTsInfo	= mDishManager.queryTsInfo(
				//new String[] {"_id", "ts_id", "frequency", "symbol", "polarisation"}
				new String[] {"_id", "frequency", "symbol", "polarisation", "fec_inner"}
				//, "sat_id=" + mSelectedSatID
				, null
				, null
		);
		mTsAdapter = new TsAdapter(DishSetupActivity.this, cTsInfo);
		mTsList.setAdapter(mTsAdapter);

		isbackpressed = false;
//		isSchedulerStart = false;
		initIntentFilter();
	}
	
	
	
/*****************************************************************************************************************************************************************************
 *	View component
 *****************************************************************************************************************************************************************************/
	Button	bRed;
	Button	bGreen;
	Button	bYellow;
	Button	bBlue;
	Button	bSearch;
	
	private void setComponent()
	{
		DxbLog(I, "setComponent()");
		
		mSelectedSatName = (TextView)findViewById(R.id.selected_sat_name);
		
		mSatList	= (ListView)findViewById(R.id.sat_list);
		mTsList		= (ListView)findViewById(R.id.ts_list);
		mLnbList	= (ListView)findViewById(R.id.lnb_list);
		
		bRed	= (Button)findViewById(R.id.button_dish_setup_add);
		bGreen	= (Button)findViewById(R.id.button_dish_setup_edit);
		bYellow	= (Button)findViewById(R.id.button_dish_setup_delete);
		bBlue	= (Button)findViewById(R.id.button_dish_setup_scan);
		bSearch	= (Button)findViewById(R.id.button_dish_setup_goto);
	}
	
	private void setListener()
	{
		mSatList.setOnFocusChangeListener(listener_OnFocusChange);
		mTsList.setOnFocusChangeListener(listener_OnFocusChange);
		mLnbList.setOnFocusChangeListener(listener_OnFocusChange);
		
		
		
		
		mLnbList.setOnItemSelectedListener(listener_OnItemSelected_LnbList);
		mLnbList.setOnItemClickListener(listener_OnItemClick_LnbList);
		
	}
	
	private int iPosition_Lnb	= 0;
	AdapterView.OnItemSelectedListener	listener_OnItemSelected_LnbList = new OnItemSelectedListener()
	{
		@Override
		public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3)
		{
			// TODO Auto-generated method stub
			Log.d(TAG,"LnbList == > sat_list setOnItemSelectedListener " + arg2);
			Log.d(TAG,"LnbList == > sat_list setOnItemSelectedListener " + arg3);
			
			iPosition_Lnb	= arg2;
			
			if(mLnbList.isFocused())
			{
				if(mDish != null)
				{
					setVisibility_Button(LIST_LNB, arg2, mDish.getMotorDiseqcMode());
				}
			}
		}

		@Override
		public void onNothingSelected(AdapterView<?> arg0)
		{
			// TODO Auto-generated method stub
		}
	};
	
	AdapterView.OnItemClickListener	listener_OnItemClick_LnbList = new AdapterView.OnItemClickListener()
	{
		@Override
		public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3)
		{
			// TODO Auto-generated method stub
			if(mDish == null)
				return;
			
			Log.d(TAG,"LnbList == > sat_list setOnItemClickListener " + arg2);
			iPosition_Lnb	= arg2;
			
			switch(arg2)
			{
				case DISHSETUP_LNB_TYPE:
					showSatLnbType();
				break;
				
				case DISHSETUP_LNB_POWER:
					showLnbPower();
				break;
				
				case DISHSETUP_LNB_TONE:
					showLnbTone();
				break;
				
				case DISHSETUP_LNB_TONEBURST:
					showLnbToneBurst();
				break;
				
				case DISHSETUP_LNB_DISEQC_MODE:
					showLnbDiseqcMode();
				break;
				
				case DISHSETUP_LNB_COMMITTED_COMMAND:
					showLnbCommittedCommand();
				break;
				
				case DISHSETUP_LNB_UNCOMMITTED_COMMAND:
					showLnbUncommittedCommand();
				break;
				
				case DISHSETUP_LNB_DISEQC_REPEATS:
					DiseqcRepeats();
				break;
				
				case DISHSETUP_LNB_COMMAND_ORDER:
					showLnbCommandOrder();
				break;
				
				case DISHSETUP_LNB_MOTOR:
					showLnbMotor();
				break;
			}
		}
	};

	View.OnFocusChangeListener listener_OnFocusChange = new OnFocusChangeListener()
	{
		@Override
		public void onFocusChange(View v, boolean isFocused)
		{
			DxbLog(I, "listener_OnFocusChange --> onFocusChange(" + isFocused + ")");
			
			if(isFocused)
			{
				switch(v.getId())
				{
					case R.id.sat_list:
						setVisibility_Button(LIST_SAT);

						switch_flag = 0;
						((CursorAdapter)mSatList.getAdapter()).notifyDataSetChanged();
					break;
						
					case R.id.ts_list:
						setVisibility_Button(LIST_TS);

						switch_flag = 1;
						((CursorAdapter)mTsList.getAdapter()).notifyDataSetChanged();
					break;
					
					case R.id.lnb_list:
						if(mDish != null)
						{
							setVisibility_Button(LIST_LNB, iPosition_Lnb, mDish.getMotorDiseqcMode());
						}
					break;
				}
			}
		}
	};
/*****************************************************************************************************************************************************************************
 *****************************************************************************************************************************************************************************/
	
	private void setVisibility_Button(int iList)
	{
		setVisibility_Button(iList, 0, 0);
	}
	
/*	private void setVisibility_Button(int iList, int iIndex)
	{
		setVisibility_Button(iList, iIndex, 0);
	}*/
	
	private void setVisibility_Button(int iList, int iIndex, int motor)
	{
		DxbLog(I, "setVisibility_Button(" + iList + ", " + iIndex + ")");
		
		switch(iList)
		{
			case LIST_SAT:
				bGreen.setText(this.getResources().getString(R.string.dish_setup_conf_button_edit));
				//bYellow.setText(this.getResources().getString(R.string.dish_setup_conf_button_delete));
				
				bGreen.setVisibility(View.VISIBLE);
				bYellow.setVisibility(View.VISIBLE);
			break;
			
			case LIST_TS:
				bGreen.setText(this.getResources().getString(R.string.dish_setup_conf_button_edit));
				//bYellow.setText(this.getResources().getString(R.string.dish_setup_conf_button_delete));
				
				bGreen.setVisibility(View.VISIBLE);
				bYellow.setVisibility(View.VISIBLE);
			break;
			
			case LIST_LNB:
				if(		(iIndex == DISHSETUP_LNB_MOTOR)
					&&	(motor == Dish.LNB_DISEQC_1_3)
				)
				{
					bGreen.setText(this.getResources().getString(R.string.guide) + " : " + this.getResources().getString(R.string.set_location));
					if(!bGreen.isShown())
					{
						bGreen.setVisibility(View.VISIBLE);
					}
				}
				else
				{
					if(bGreen.isShown())
					{
						bGreen.setVisibility(View.INVISIBLE);
					}
				}
				
				if(bYellow.isShown())
				{
					bYellow.setVisibility(View.INVISIBLE);
				}
			break;
		}
	}
	
	@Override
	public void onStart()
	{
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onStart()");
		super.onStart();
	}
	
	@Override
	protected void onResume()
	{
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onResume()");
		super.onResume();
	}

	@Override
	protected void onPause()
	{
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onPause()");
		super.onPause();

		if(isbackpressed == false)
//				&&	(isSchedulerStart == false)
		{
			if(getPlayer().isValid())
			{
				//getPlayer().stop();
				getPlayer().release();
			}
		}
	}

	@Override
	public void onStop()
	{
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onStop()");
		
		super.onStop();
		this.finish();
		Log.d(TAG, "~~~~~~~~~~~~~~~~> onStop() finish");
	}

	@Override
	public void onDestroy() {
		Log.d(TAG, "onDestroy()");
		getPlayer().isDishSetupActivityOn = false;
		
		if(cSatInfo != null)
		{
			cSatInfo.close();
			cSatInfo	= null;
		}
		if(cSatList != null)
		{
			cSatList.close();
			cSatList	= null;
		}
		if(cTsInfo != null)
		{
			cTsInfo.close();
			cTsInfo	= null;
		}
		if(cTsList != null)
		{
			cTsList.close();
			cTsList	= null;
		}
		
		if (mDishManager != null)
		{
			mDishManager.close();
		}
		
		if(isbackpressed == false)
//				&&	(isSchedulerStart == false)
		{
			getPlayer().onDestroy();
			
			if(getPlayer().isValid() == false)
			{
				getPlayer().isOFF = true;
			}
		}
		
		super.onDestroy();
		this.finish();
		
		unregisterReceiver(mReceiverIntent);
	}

	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event)
	{
		switch (keyCode)
		{
			//red(add)
			case KeyEvent.KEYCODE_TV:
				addSatFrequency();
			break;
	
			//green(edit)
			case KeyEvent.KEYCODE_GUIDE:
				if(mSatList.isFocused() || mTsList.isFocused())
				{
					if(mDish != null && switch_flag == 0)
						showSatEdit();
					else if(mTuner != null && switch_flag != 0)
						showTsEdit();
				}
				else if(mLnbList.isFocused() && iPosition_Lnb==DISHSETUP_LNB_MOTOR)
				{
					showSetLocation();
				}
			break;

			//yellow(delete)
			case KeyEvent.KEYCODE_BOOKMARK:
				if(mSatList.isFocused() || mTsList.isFocused())
				{
					if(mDish != null)
						showDelete();
				}
			break;

			//info(View ts data)
			case KeyEvent.KEYCODE_INFO:
				if (mTsLayout.getVisibility() == View.VISIBLE)
				{
					mTsLayout.setVisibility(View.GONE);
					mLnbList.setVisibility(View.VISIBLE);
					mLnbList.requestFocus();
				}
				else
				{
					mTsLayout.setVisibility(View.VISIBLE);
					mLnbList.setVisibility(View.GONE);
					updateTsList();
					mTsLayout.requestFocus();
				}
			break;
			
			//blue(menu)
			case KeyEvent.KEYCODE_MENU:
				if(mDish != null)
					showScanConfigDialog();
			break;
		}
		
		return super.onKeyUp(keyCode, event);
	}

	@Override 
	public void onBackPressed()
	{
		Log.d("BACK KEY", "onBackPressed Called");
		isbackpressed = true;
		super.onBackPressed();
	}
	
	private void updateSatList() {
		if(cSatList != null)
		{
			cSatList.close();
		}
		
		cSatList	= mDishManager.querySatInfo(
				new String[] {"_id", "motor_position", "sat_longitude", "sat_name"}
				, null
				, null
		);
		mSatAdapter.changeCursor(cSatList);
		
		getPlayer().ExecuteDiSEqC(null, mDish);
	}
	
	private void updateTsList()
	{
		if(cTsList != null)
		{
			cTsList.close();
		}
		
		cTsList	= mDishManager.queryTsInfo(
				new String[] {"_id", "frequency", "symbol", "polarisation", "fec_inner"}
				, "sat_id=" + mSelectedSatID
				, null
		);
		mTsAdapter.changeCursor(cTsList);
		
		getPlayer().ExecuteDiSEqC(null, mDish);
	}
		
	@SuppressWarnings("deprecation")
	private void showSatLnbType(){
		int pos = 0;

		pos = mDish.getLnbType();

		builder = new AlertDialog.Builder(DishSetupActivity.this);	
		
		builder.setTitle(R.string.dish_setup_lnb_type);
		builder.setIcon(android.R.drawable.ic_dialog_info);
		builder.setSingleChoiceItems(new String[] {"SINGLE", "UNIVERSAL"}, pos, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				LnbSettingItemSelected = which;
				
				ContentValues values = new ContentValues();
				values.put("lnb_type", LnbSettingItemSelected);
				mDish = mDishManager.getDishSetup(mSelectedSatID);
				
				int _id = mDish.getId();

				mDish.setLnbType(LnbSettingItemSelected);
				mDishManager.updateSatInfo(values, _id, mDish);
				mLnbAdapter.notifyDataSetChanged();
				
				if(LnbSettingItemSelected == 0)
				{
					dialog.dismiss();
					showSatLnbTypeSingle();
				}
				else if(LnbSettingItemSelected == 1)
				{
					dialog.dismiss();
					showSatLnbTypeUniversal();
				}
			}
		});
		
		builder.setNegativeButton(R.string.cancel, new  DialogInterface.OnClickListener(){
			//@Override
			public void onClick(DialogInterface dialog, int which) {
			// TODO Auto-generated method stub
				dialog.dismiss();
			}
		});	
		
		AlertDialog dialog = builder.create();
		dialog.setOnShowListener(new DialogInterface.OnShowListener(){
			public void onShow(DialogInterface dialog) {
			}         
		}); 	

		 dialog.setOnDismissListener(new DialogInterface.OnDismissListener(){
				public void onDismiss(DialogInterface dialog) {
//					t.quitLoop();	
				}         
		});	
		 
		dialog.show();
		WindowManager m = getWindowManager();   
		Display d = m.getDefaultDisplay();  	
		WindowManager.LayoutParams lp=dialog.getWindow().getAttributes();
		lp.dimAmount=0.0f;
		//lp.height = (int) (d.getHeight() * 0.6);  
		lp.width = (int) (d.getWidth() * 0.50);
		dialog.getWindow().setAttributes(lp);
		dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
	}
	
	private AlertDialog.Builder lnbFrequencyBuilder;
	private View dvbs_lnb_frequency;
	
	@SuppressWarnings("deprecation")
	private void showSatLnbTypeSingle()
	{
		lnbFrequencyBuilder = new AlertDialog.Builder(DishSetupActivity.this);
		LayoutInflater layoutInflater = LayoutInflater.from(DishSetupActivity.this);
		
		dvbs_lnb_frequency = layoutInflater.inflate(R.layout.dvbs_lnb_frequency_single_dia, null);
		
		final EditText edittext_th = (EditText) dvbs_lnb_frequency.findViewById(R.id.edittext_lnb_single_threshold);

//		if (((edittext_th.getText().toString()) == null) || ((edittext_th.getText().toString()).equals("0")))
//			edittext_th.setText("10750");
//		else
			edittext_th.setText(String.valueOf(mDish.getLnbLofThreshold()));
		
		lnbFrequencyBuilder.setView(dvbs_lnb_frequency);
		lnbFrequencyBuilder.setTitle(R.string.dish_setup_conf_lnb_single);
		
		lnbFrequencyBuilder.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				ContentValues values = new ContentValues();

				if (((edittext_th.getText().toString()) == null) || ((edittext_th.getText().toString()).equals("0"))) {
					values.put("lnb_lof_lo", 0);
					values.put("lnb_lof_hi", 0);
					values.put("lnb_lof_th", 0);
				} else {
					mDish.setLnbLoLof(Integer.parseInt(edittext_th.getText().toString()));
					mDish.setLnbHiLof(Integer.parseInt(edittext_th.getText().toString()));
					mDish.setLnbLofThreshold(Integer.parseInt(edittext_th.getText().toString()));

					values.put("lnb_lof_lo", Integer.parseInt(edittext_th.getText().toString()));
					values.put("lnb_lof_hi", Integer.parseInt(edittext_th.getText().toString()));
					values.put("lnb_lof_th", Integer.parseInt(edittext_th.getText().toString()));
				}
				mDish = mDishManager.getDishSetup(mSelectedSatID);
				int _id = mDish.getId();

				mDishManager.updateSatInfo(values, _id, mDish);
				mLnbAdapter.notifyDataSetChanged();
				dialog.dismiss();
			}
		});
		
		lnbFrequencyBuilder.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int id) {
				dialog.dismiss();
			}
		});

		AlertDialog dialog = lnbFrequencyBuilder.create();
		 
		dialog.show();
		DisplayMetrics displayMetrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);

		System.out.println("H = " + displayMetrics.heightPixels);
		System.out.println("W = " + displayMetrics.widthPixels);

		WindowManager m = getWindowManager();
		Display d = m.getDefaultDisplay();
		WindowManager.LayoutParams lp = dialog.getWindow().getAttributes();
		lp.dimAmount = 0.0f;
		lp.width = (int) (d.getWidth() * 0.50);
		dialog.getWindow().setAttributes(lp);
		dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
	}
	
	@SuppressWarnings("deprecation")
	private void showSatLnbTypeUniversal()
	{
		lnbFrequencyBuilder = new AlertDialog.Builder(DishSetupActivity.this);
		LayoutInflater layoutInflater = LayoutInflater.from(DishSetupActivity.this);
		
		dvbs_lnb_frequency = layoutInflater.inflate(R.layout.dvbs_lnb_frequency_dia, null);
		
		final EditText edittext_lo = (EditText) dvbs_lnb_frequency.findViewById(R.id.edittext_lnb_low);
		final EditText edittext_hi = (EditText) dvbs_lnb_frequency.findViewById(R.id.edittext_lnb_high);
		final EditText edittext_th = (EditText) dvbs_lnb_frequency.findViewById(R.id.edittext_lnb_threshold);

		edittext_lo.setText(String.valueOf(mDish.getLnbLoLof()));
		edittext_hi.setText(String.valueOf(mDish.getLnbHiLof()));
		edittext_th.setText(String.valueOf(mDish.getLnbLofThreshold()));
		
		lnbFrequencyBuilder.setView(dvbs_lnb_frequency);
		lnbFrequencyBuilder.setTitle(R.string.dish_setup_conf_lnb_universal);
		
		lnbFrequencyBuilder.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				ContentValues values = new ContentValues();
				
				if (((edittext_lo.getText().toString()) == null) || ((edittext_lo.getText().toString()).equals("0"))) {
					values.put("lnb_lof_lo", 0);
				} else {
					mDish.setLnbLoLof(Integer.parseInt(edittext_lo.getText().toString()));
					values.put("lnb_lof_lo", Integer.parseInt(edittext_lo.getText().toString()));
				}
				
				if (((edittext_hi.getText().toString()) == null) || ((edittext_hi.getText().toString()).equals("0"))) {
					values.put("lnb_lof_hi", 0);
				} else {
					mDish.setLnbHiLof(Integer.parseInt(edittext_hi.getText().toString()));
					values.put("lnb_lof_hi", Integer.parseInt(edittext_hi.getText().toString()));
				}
				
				if (((edittext_th.getText().toString()) == null) || ((edittext_th.getText().toString()).equals("0"))) {
					values.put("lnb_lof_th", 0);
				} else {
					mDish.setLnbLofThreshold(Integer.parseInt(edittext_th.getText().toString()));
					values.put("lnb_lof_th", Integer.parseInt(edittext_th.getText().toString()));
				}

				mDish = mDishManager.getDishSetup(mSelectedSatID);
				int _id = mDish.getId();

				mDishManager.updateSatInfo(values, _id, mDish);
				mLnbAdapter.notifyDataSetChanged();
				dialog.dismiss();
			}
		});
		
		lnbFrequencyBuilder.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int id) {
				dialog.dismiss();
			}
		});

		AlertDialog dialog = lnbFrequencyBuilder.create();
		 
		dialog.show();
		DisplayMetrics displayMetrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);

		System.out.println("H = " + displayMetrics.heightPixels);
		System.out.println("W = " + displayMetrics.widthPixels);

		WindowManager m = getWindowManager();
		Display d = m.getDefaultDisplay();
		WindowManager.LayoutParams lp = dialog.getWindow().getAttributes();
		lp.dimAmount = 0.0f;
		lp.width = (int) (d.getWidth() * 0.50);
		dialog.getWindow().setAttributes(lp);
		dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
	}

	@SuppressWarnings("deprecation")
	public void showLnbPower(){
		 
		int pos = 0;
		pos = mDish.getLnbPower();

		builder = new AlertDialog.Builder(DishSetupActivity.this);	
		builder.setTitle(R.string.dish_setup_lnb_power);
		builder.setIcon( android.R.drawable.ic_dialog_info);
		builder.setSingleChoiceItems(new String[] { "OFF", "13V", "18V", "Polarisation"}, pos, new DialogInterface.OnClickListener()
		{
			public void onClick(DialogInterface dialog, int which)
			{
				LnbSettingItemSelected = which;
				ContentValues values = new ContentValues();
				values.put("lnb_power", LnbSettingItemSelected);
				
				int _id  = mDish.getId();
				mDish = mDishManager.getDishSetup(mSelectedSatID);
				
				mDish.setLnbPower(LnbSettingItemSelected);
				mDishManager.updateSatInfo(values, _id, mDish);
				
				mLnbAdapter.notifyDataSetChanged();

				dialog.dismiss();
			}
		});

		builder.setNegativeButton(R.string.cancel, new  DialogInterface.OnClickListener(){
			//@Override
			public void onClick(DialogInterface dialog, int which) {
			// TODO Auto-generated method stub
				dialog.dismiss();
			}
		});

		AlertDialog dialog = builder.create();
	
		dialog.show();
		WindowManager m = getWindowManager();
		Display d = m.getDefaultDisplay();
		WindowManager.LayoutParams lp=dialog.getWindow().getAttributes();
		lp.dimAmount=0.0f;
		//lp.height = (int) (d.getHeight() * 0.6);
		lp.width = (int) (d.getWidth() * 0.50);
		dialog.getWindow().setAttributes(lp);
		dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
	}

	@SuppressWarnings("deprecation")
	public void showLnbTone()
	{
		int pos = 0;
		pos = mDish.getLnbTone();
		
		String[] ItemData = new String[3];

		ItemData[0] = this.getResources().getString(R.string.dish_setup_tone_off);
		ItemData[1] = this.getResources().getString(R.string.dish_setup_tone_22K);
		ItemData[2] = this.getResources().getString(R.string.dish_setup_tone_auto);

		builder = new AlertDialog.Builder(this);
		builder.setTitle(R.string.dish_setup_lnb_tone);
		builder.setIcon( android.R.drawable.ic_dialog_info);
		builder.setSingleChoiceItems(ItemData, pos, new DialogInterface.OnClickListener()
		{
			public void onClick(DialogInterface dialog, int which)
			{
				LnbSettingItemSelected= which;

				ContentValues values = new ContentValues();
				values.put("lnb_tone", LnbSettingItemSelected);

				int _id  = mDish.getId();

				mDish = mDishManager.getDishSetup(mSelectedSatID);
				mDish.setLnbTone(LnbSettingItemSelected);
				mDishManager.updateSatInfo(values, _id, mDish);
				mLnbAdapter.notifyDataSetChanged();

				dialog.dismiss();
			}
		});

		builder.setNegativeButton(R.string.cancel, new  DialogInterface.OnClickListener(){
			public void onClick(DialogInterface dialog, int which) {
				// TODO Auto-generated method stub
				dialog.dismiss();
			}
		});

		AlertDialog dialog = builder.create();

		dialog.show();
		WindowManager m = getWindowManager();
		Display d = m.getDefaultDisplay();
		WindowManager.LayoutParams lp=dialog.getWindow().getAttributes();
		lp.dimAmount=0.0f;
		lp.width = (int) (d.getWidth() * 0.50);
		dialog.getWindow().setAttributes(lp);
		dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
	}

	@SuppressWarnings("deprecation")
	public void showLnbToneBurst(){
		int pos = 0;
		pos = mDish.getLnbToneburst();

		String[] ItemData = new String[3];
		ItemData[0] = this.getResources().getString(R.string.dish_setup_none);
		ItemData[1] = "Mini-A";
		ItemData[2] = "Mini-B";
		builder = new AlertDialog.Builder(this);

		builder.setTitle(R.string.dish_setup_lnb_toneburst);
		builder.setIcon( android.R.drawable.ic_dialog_info);
		builder.setSingleChoiceItems(ItemData, pos, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				LnbSettingItemSelected= which;
				
				ContentValues values = new ContentValues();
				values.put("lnb_toneburst", LnbSettingItemSelected);
				
				mDish = mDishManager.getDishSetup(mSelectedSatID);
				
				int _id  = mDish.getId();

				mDish.setLnbToneburst(LnbSettingItemSelected);
				mDishManager.updateSatInfo(values, _id, mDish);
				mLnbAdapter.notifyDataSetChanged();
				
				dialog.dismiss();
			}
		});

		builder.setNegativeButton(R.string.cancel, new  DialogInterface.OnClickListener(){
			public void onClick(DialogInterface dialog, int which) {
				// TODO Auto-generated method stub
				dialog.dismiss();
			}
		});

		AlertDialog dialog = builder.create();
	
		dialog.show();
		WindowManager m = getWindowManager();
		Display d = m.getDefaultDisplay();
		WindowManager.LayoutParams lp=dialog.getWindow().getAttributes();
		lp.dimAmount=0.0f;
		lp.width = (int) (d.getWidth() * 0.50);
		dialog.getWindow().setAttributes(lp);
		dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
	}

	@SuppressWarnings("deprecation")
	public void showLnbDiseqcMode(){

		int pos = 0;
		pos = mDish.getLnbDiseqcMode();
				
		String[] ItemData = new String[3];
		ItemData[0] = this.getResources().getString(R.string.dish_setup_none);
		ItemData[1] = this.getResources().getString(R.string.dish_setup_diseqc0);
		ItemData[2] = this.getResources().getString(R.string.dish_setup_diseqc1);

		builder = new AlertDialog.Builder(this);
		builder.setTitle(R.string.dish_setup_lnb_diseqc);
		builder.setIcon( android.R.drawable.ic_dialog_info);
		builder.setSingleChoiceItems(ItemData, pos, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				LnbSettingItemSelected= which;
				
				ContentValues values = new ContentValues();
				values.put("lnb_diseqc_mode", LnbSettingItemSelected);
				values.put("lnb_cmd_sequence", 1);
				
				if(LnbSettingItemSelected == 1)
				{
					values.put("lnb_diseqc_config11", 0);
					values.put("diseqc_repeat", 0);
					mDish.setLnbDiseqcMode(LnbSettingItemSelected);
					mDish.setDiseqcRepeat(0);
				}		

				if(LnbSettingItemSelected == 2)
					values.put("lnb_diseqc_config10", 4);

				mDish = mDishManager.getDishSetup(mSelectedSatID);

				int _id  = mDish.getId();

				mDish.setLnbDiseqcMode(LnbSettingItemSelected);
				mDishManager.updateSatInfo(values, _id, mDish);
				mLnbAdapter.notifyDataSetChanged();
				
				dialog.dismiss();
			}
		});

		builder.setNegativeButton(R.string.cancel, new  DialogInterface.OnClickListener(){
			public void onClick(DialogInterface dialog, int which) {
				// TODO Auto-generated method stub
				dialog.dismiss();
				}
		});
		
		AlertDialog dialog = builder.create();
	
		dialog.show();
		WindowManager m = getWindowManager();
		Display d = m.getDefaultDisplay();
		WindowManager.LayoutParams lp=dialog.getWindow().getAttributes();
		lp.dimAmount=0.0f;
		lp.width = (int) (d.getWidth() * 0.50);
		dialog.getWindow().setAttributes(lp);
		dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
	}

	@SuppressWarnings("deprecation")
	public void showLnbCommittedCommand(){

		String[] ItemData=null;
		int pos = 0;
		pos = mDish.getLnbDiseqcConfig10();

		ItemData = new String[5];

		ItemData[0]=this.getResources().getString(R.string.dish_setup_none);
		ItemData[1]="AA";
		ItemData[2]="AB";
		ItemData[3]="BA";
		ItemData[4]="BB"; 

		builder = new AlertDialog.Builder(this);
		builder.setTitle(R.string.dish_setup_lnb_committed_command);
		builder.setIcon( android.R.drawable.ic_dialog_info);
		builder.setSingleChoiceItems(ItemData, pos, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				LnbSettingItemSelected= which;
				
				ContentValues values = new ContentValues();
				values.put("lnb_diseqc_config10", LnbSettingItemSelected);
				
				int _id  = mDish.getId();

				mDish = mDishManager.getDishSetup(mSelectedSatID);
				mDish.setLnbDiseqcConfig10(LnbSettingItemSelected);
				mDishManager.updateSatInfo(values, _id, mDish);
				mLnbAdapter.notifyDataSetChanged();
				
				dialog.dismiss();
			}
		});

		builder.setNegativeButton(R.string.cancel, new  DialogInterface.OnClickListener(){
			public void onClick(DialogInterface dialog, int which) {
				// TODO Auto-generated method stub
				dialog.dismiss();
				}
		});

		AlertDialog dialog = builder.create();

		dialog.show();
		WindowManager m = getWindowManager();
		Display d = m.getDefaultDisplay();
		WindowManager.LayoutParams lp=dialog.getWindow().getAttributes();
		lp.dimAmount=0.0f; 
		lp.width = (int) (d.getWidth() * 0.50);
		dialog.getWindow().setAttributes(lp);
		dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
	}

	@SuppressWarnings("deprecation")
	public void showLnbUncommittedCommand(){

		String[] ItemData=null;
		int pos = 0;
		pos = mDish.getLnbDiseqcConfig11();

		ItemData = new String[17];
		ItemData[0] = this.getResources().getString(R.string.dish_setup_none);

		for(int j=1;j<17;j++){
				ItemData[j]="LNB"+String.valueOf(j);
		}

		builder = new AlertDialog.Builder(this);
		builder.setTitle(R.string.dish_setup_lnb_uncommitted_command);
		builder.setIcon( android.R.drawable.ic_dialog_info);
		builder.setSingleChoiceItems(ItemData, pos, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				LnbSettingItemSelected= which;
				
				ContentValues values = new ContentValues();
				values.put("lnb_diseqc_config11", LnbSettingItemSelected);
				mDish = mDishManager.getDishSetup(mSelectedSatID);
				
				int _id  = mDish.getId();

				mDish.setLnbDiseqcConfig11(LnbSettingItemSelected);
				mDishManager.updateSatInfo(values, _id, mDish);
				mLnbAdapter.notifyDataSetChanged();
				
				dialog.dismiss();
			}
		});

		builder.setNegativeButton(R.string.cancel, new  DialogInterface.OnClickListener(){
			public void onClick(DialogInterface dialog, int which) {
				// TODO Auto-generated method stub
				dialog.dismiss();
			}
		});	

		AlertDialog dialog = builder.create();
		
		dialog.show();
		WindowManager m = getWindowManager();   
		Display d = m.getDefaultDisplay();  	
		WindowManager.LayoutParams lp=dialog.getWindow().getAttributes();
		lp.dimAmount=0.0f; 
		lp.width = (int) (d.getWidth() * 0.50);
		dialog.getWindow().setAttributes(lp);
		dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
	}

	@SuppressWarnings("deprecation")
	public void DiseqcRepeats()
	{
		int pos = 0;
		pos = mDish.getDiseqcRepeat();
		
		String[] ItemData = new String[2];

		ItemData[0] = this.getResources().getString(R.string.off);
		ItemData[1] = this.getResources().getString(R.string.on);

		builder = new AlertDialog.Builder(this);
		builder.setTitle(R.string.dish_setup_lnb_diseqc_repeat);
		builder.setIcon( android.R.drawable.ic_dialog_info);
		builder.setSingleChoiceItems(ItemData, pos, new DialogInterface.OnClickListener()
		{
			public void onClick(DialogInterface dialog, int which)
			{
				Log.d(TAG, "which = " + which);
				LnbSettingItemSelected= which;

				ContentValues values = new ContentValues();
				values.put("diseqc_repeat", LnbSettingItemSelected);

				int _id  = mDish.getId();

				mDish = mDishManager.getDishSetup(mSelectedSatID);
				mDish.setDiseqcRepeat(LnbSettingItemSelected);
				mDishManager.updateSatInfo(values, _id, mDish);
				mLnbAdapter.notifyDataSetChanged();

				dialog.dismiss();
			}
		});

		builder.setNegativeButton(R.string.cancel, new  DialogInterface.OnClickListener(){
			public void onClick(DialogInterface dialog, int which) {
				// TODO Auto-generated method stub
				dialog.dismiss();
			}
		});

		AlertDialog dialog = builder.create();

		dialog.show();
		WindowManager m = getWindowManager();
		Display d = m.getDefaultDisplay();
		WindowManager.LayoutParams lp=dialog.getWindow().getAttributes();
		lp.dimAmount=0.0f;
		lp.width = (int) (d.getWidth() * 0.50);
		dialog.getWindow().setAttributes(lp);
		dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
	}

	@SuppressWarnings("deprecation")
	public void showLnbCommandOrder()
	{
		/*
		 * diseqc 1.0)
		 *  0) commited, toneburst
		 *  1) toneburst, committed
		 * diseqc 1.1)
		 *  2) committed, uncommitted, toneburst
		 *  3) toneburst, committed, uncommitted
		 *  4) uncommitted, committed, toneburst
		 *  5) toneburst, uncommitted, committed
		 */

		String[] ItemData = null;
		
		if (mDish.getLnbDiseqcMode() == Dish.LNB_DISEQC_1_0)
		{
			ItemData = new String[2];			
			ItemData[0] = "1  |   Committed,  Toneburst";
			ItemData[1] = "2  |   Toneburst,  Committed";
		}
		else if (mDish.getLnbDiseqcMode() == Dish.LNB_DISEQC_1_1)
		{
			ItemData = new String[4];			
			ItemData[0] = "1  |   Committed,  Uncommitted,  Toneburst";
			ItemData[1] = "2  |   Toneburst,  Committed,  Uncommitted";
			ItemData[2] = "3  |   Uncommitted,  Committed,  Toneburst";
			ItemData[3] = "4  |   Toneburst,  Uncommitted,  Committed";
		}
		
		int pos = 0;
		pos = mDish.getLnbSequence() -1;
		
		builder = new AlertDialog.Builder(this);
		builder.setTitle(R.string.dish_setup_lnb_command_order);
		builder.setIcon(android.R.drawable.ic_dialog_info);
		builder.setSingleChoiceItems(ItemData, pos, new DialogInterface.OnClickListener()
		{
			public void onClick(DialogInterface dialog, int which)
			{
				LnbSettingItemSelected = which;

				ContentValues values = new ContentValues();
				mDish = mDishManager.getDishSetup(mSelectedSatID);
				
				int _id  = mDish.getId();

				mDish.setLnbSequence(LnbSettingItemSelected+1);
				
				values.put("lnb_cmd_sequence", LnbSettingItemSelected+1);
				
				mDishManager.updateSatInfo(values, _id, mDish);
				mLnbAdapter.notifyDataSetChanged();
				
				dialog.dismiss();
			}
		});

		AlertDialog dialog = builder.create();

		dialog.show();
		WindowManager m = getWindowManager();
		Display d = m.getDefaultDisplay();
		WindowManager.LayoutParams lp = dialog.getWindow().getAttributes();
		lp.dimAmount = 0.0f;
		lp.width = (int) (d.getWidth() * 0.50);
		dialog.getWindow().setAttributes(lp);
		dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
	}

	//************************** setting motor_setup dialog **************************//

	private AlertDialog.Builder lnbMotorBuilder;
	private View dvbs_motor_setup;
	TextView motor_move;
	TextView motor_move_step;
	Button motor_move_store;
	Button motor_location_save;
	Button motor_location_reset;
	Button motor_goto_x;
	
	ImageView bt_motor_left;
	ImageView bt_motor_right;
	ImageView bt_motor_step_left;
	ImageView bt_motor_step_right;
	
	double position_goto_x = 0; 
	
	public void motor_setup_default()
	{
		motor_move		= (TextView)dvbs_motor_setup.findViewById(R.id.move_state);
		motor_move_step	= (TextView)dvbs_motor_setup.findViewById(R.id.motor_move_step);
		motor_move_store		= (Button)dvbs_motor_setup.findViewById(R.id.motor_move_store);
		motor_location_save		= (Button)dvbs_motor_setup.findViewById(R.id.motor_position_save);
		motor_location_reset	= (Button)dvbs_motor_setup.findViewById(R.id.motor_position_reset);
		motor_goto_x			= (Button)dvbs_motor_setup.findViewById(R.id.motor_goto_x);
		
		bt_motor_left = (ImageView)dvbs_motor_setup.findViewById(R.id.bt_motor_left);
		bt_motor_right = (ImageView)dvbs_motor_setup.findViewById(R.id.bt_motor_right);
		bt_motor_step_left = (ImageView)dvbs_motor_setup.findViewById(R.id.bt_motor_step_left);
		bt_motor_step_right = (ImageView)dvbs_motor_setup.findViewById(R.id.bt_motor_step_right);

		motor_move.setText(R.string.dish_setup_motor_move_stop);
		motor_move_step.setText(R.string.dish_setup_motor_move_step);
		motor_move_store.setText(R.string.dish_setup_motor_move_goto_position);
		motor_location_save.setText(R.string.dish_setup_motor_move_position_save);
		motor_location_reset.setText(R.string.dish_setup_motor_move_position_reset);
		
		bt_motor_left.setNextFocusDownId(R.id.motor_move_step);
		bt_motor_right.setNextFocusDownId(R.id.motor_move_step);
		
		bt_motor_step_left.setNextFocusRightId(R.id.bt_motor_step_right);
		bt_motor_step_right.setNextFocusLeftId(R.id.bt_motor_step_left);
		
		bt_motor_step_left.setNextFocusUpId(R.id.move_state);
		bt_motor_step_right.setNextFocusUpId(R.id.move_state);
	}

	@SuppressWarnings("deprecation")
	public void showLnbMotoSetup(int motor)
	{
		getPlayer().moveMotorStore(1);
		
		dvbs_motor_setup = View.inflate(DishSetupActivity.this, R.layout.dvbs_motor_setup, null);
		lnbMotorBuilder = new AlertDialog.Builder(DishSetupActivity.this);

		motor_setup_default();
		switch(motor)
		{
			case 2:
				dvbs_motor_setup.findViewById(R.id.gr_motor_goto_x).setVisibility(View.VISIBLE);
			break;
			
			case 1:
			default:
				dvbs_motor_setup.findViewById(R.id.gr_motor_goto_x).setVisibility(View.GONE);
			break;
		}

		lnbMotorBuilder.setView(dvbs_motor_setup);
		lnbMotorBuilder.setTitle(R.string.dish_setup_conf_lnb_universal);

		lnbMotorBuilder.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int id) {
				dialog.dismiss();
			}
		});
		
		bt_motor_left.setOnFocusChangeListener(new View.OnFocusChangeListener() {
			@Override
			public void onFocusChange(View v, boolean hasFocus) {
				Log.d(TAG, "moveMotoLeft()");

				bt_motor_left.setBackgroundResource(R.drawable.arrow_left2);
				motor_move.setText(R.string.dish_setup_motor_move_left);
				getPlayer().moveMotorEast(0);
			}
		});
		
		bt_motor_right.setOnFocusChangeListener(new View.OnFocusChangeListener() {
			@Override
			public void onFocusChange(View v, boolean hasFocus) {
				Log.d(TAG, "moveMotorRight()");

				bt_motor_right.setBackgroundResource(R.drawable.arrow_right2);
				motor_move.setText(R.string.dish_setup_motor_move_right);
				getPlayer().moveMotorWest(0);
			}
		});
		
		motor_move.setOnFocusChangeListener(new View.OnFocusChangeListener() {
			@Override
			public void onFocusChange(View v, boolean hasFocus) {
				Log.d(TAG, "moveMotorStop()");

				motor_move.setText(R.string.dish_setup_motor_move_stop);
				
				if(hasFocus)
				{
					motor_move.setTextColor(getResources().getColor(R.color.ics_focus));
					bt_motor_right.setBackgroundResource(R.drawable.arrow_right);
					bt_motor_left.setBackgroundResource(R.drawable.arrow_left);
					
					bt_motor_left.setFocusable(true);
					bt_motor_right.setFocusable(true);
				}
				else
				{
					motor_move.setTextColor(getResources().getColor(R.color.color_white));
				}
				
				getPlayer().moveMotorStop();
			}
		});
		
		motor_move_step.setOnFocusChangeListener(new View.OnFocusChangeListener() {
			@Override
			public void onFocusChange(View v, boolean hasFocus) {
				Log.d(TAG, "moveMotorStop()");

				motor_move.setText(R.string.dish_setup_motor_move_stop);
				
				if(hasFocus)
				{
					motor_move_step.setTextColor(getResources().getColor(R.color.ics_focus));
					bt_motor_step_right.setBackgroundResource(R.drawable.arrow_right);
					bt_motor_step_left.setBackgroundResource(R.drawable.arrow_left);
					bt_motor_left.setBackgroundResource(R.drawable.arrow_left);
					bt_motor_right.setBackgroundResource(R.drawable.arrow_right);
				}
				else
				{
					motor_move_step.setTextColor(getResources().getColor(R.color.color_white));
				}
				
				getPlayer().moveMotorStop();
			}
		});

		motor_move_step.setOnKeyListener(new View.OnKeyListener() {
			@Override
			public boolean onKey(View v, int keyCode, KeyEvent event) {
				switch(keyCode)
				{
					case KeyEvent.KEYCODE_DPAD_LEFT :
						if(event.getAction() == KeyEvent.ACTION_DOWN)
						{
							bt_motor_step_left.setBackgroundResource(R.drawable.arrow_left2);
							motor_move_step.setText(R.string.dish_setup_motor_move_left);

							getPlayer().moveMotorEast(-1);
							bt_motor_step_left.setBackgroundResource(R.drawable.arrow_left);
							motor_move_step.setText(R.string.dish_setup_motor_move_step);
						}
						return true;

					case KeyEvent.KEYCODE_DPAD_RIGHT :
						if(event.getAction() == KeyEvent.ACTION_DOWN)
						{
							bt_motor_step_right.setBackgroundResource(R.drawable.arrow_right2);
							motor_move_step.setText(R.string.dish_setup_motor_move_right);
						
							getPlayer().moveMotorWest(-1);
							bt_motor_step_right.setBackgroundResource(R.drawable.arrow_right);
							motor_move_step.setText(R.string.dish_setup_motor_move_step);
						}
						return true;
				}
				return false;
			}
		});
		
//		bt_direction.setOnClickListener(new View.OnClickListener(){
//			@Override
//			public void onClick(View v) {
//				if(bt_direction.getText() == "East")
//				{
//					Log.d(TAG, "direction : West");
//					bt_direction.setText("West");
//				}
//				else
//				{
//					Log.d(TAG, "direction : East");
//					bt_direction.setText("East");
//				}
//			}
//		});
		
		motor_move_store.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				Log.d(TAG, "moveMotorStore");
				getPlayer().moveMotorStore(mDish.getId());
			}
		});
		
		// motor_move_save function
		motor_location_save.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				Log.d(TAG, "storeMotorPosition");
				getPlayer().storeMotorPosition(mDish.getId());
			}
		});
		
		motor_location_reset.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				Log.d(TAG, "moveMotorReset");
				getPlayer().moveMotorStore(0);
			}
		});
		
		motor_goto_x.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				Log.d(TAG, "GotoX");
				getPlayer().moveMotorUSALS(mDish.getSatLongitude());
			}
		});

		AlertDialog dialog = lnbMotorBuilder.create();

		dialog.show();

		DisplayMetrics displayMetrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);

		WindowManager m = getWindowManager();
		Display d = m.getDefaultDisplay();
		WindowManager.LayoutParams lp = dialog.getWindow().getAttributes();
		lp.dimAmount = 0.0f;
		lp.width = (int) (d.getWidth() * 0.5);
		dialog.getWindow().setAttributes(lp);
		dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
	}

	@SuppressWarnings("deprecation")
	public void showLnbMotor()
	{
		int pos = 0;
		pos = mDish.getMotorDiseqcMode() - Dish.LNB_DISEQC_1_1;
				
		String[] ItemData = new String[3];
		ItemData[0] = this.getResources().getString(R.string.dish_setup_none);
		ItemData[1] = this.getResources().getString(R.string.dish_setup_diseqc2);
		ItemData[2] = this.getResources().getString(R.string.dish_setup_diseqc3);

		builder = new AlertDialog.Builder(this);
		builder.setTitle(R.string.dish_setup_lnb_moto_setting);
		builder.setIcon( android.R.drawable.ic_dialog_info);
		builder.setSingleChoiceItems(ItemData, pos, new DialogInterface.OnClickListener()
		{
			public void onClick(DialogInterface dialog, int which)
			{
				ContentValues values = new ContentValues();
				values.put("motor_diseqc_mode", which+Dish.LNB_DISEQC_1_1);
				
				int _id  = mDish.getId();

				mDish.setMotorDiseqcMode(which+Dish.LNB_DISEQC_1_1);
				mDishManager.updateSatInfo(values, _id, mDish);
				mLnbAdapter.notifyDataSetChanged();
				
				switch(which)
				{
					case 1:
					case 2:
						showLnbMotoSetup(which);
					break;
				}
				setVisibility_Button(LIST_LNB, iPosition_Lnb, mDish.getMotorDiseqcMode());

				dialog.dismiss();
			}
		});

		builder.setNegativeButton(R.string.cancel, new  DialogInterface.OnClickListener(){
			public void onClick(DialogInterface dialog, int which) {
				// TODO Auto-generated method stub
				dialog.dismiss();
				}
		});
		
		AlertDialog dialog = builder.create();
	
		dialog.show();
		WindowManager m = getWindowManager();
		Display d = m.getDefaultDisplay();
		WindowManager.LayoutParams lp=dialog.getWindow().getAttributes();
		lp.dimAmount=0.0f;
		lp.width = (int) (d.getWidth() * 0.50);
		dialog.getWindow().setAttributes(lp);
		dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
	}
	
	@SuppressWarnings("deprecation")
	private void addSatFrequency(){
		int pos = 0;
		builder = new AlertDialog.Builder(DishSetupActivity.this);	
		
		builder.setTitle(R.string.add);
		builder.setIcon(android.R.drawable.ic_dialog_info);
		builder.setSingleChoiceItems(new String[] {"Satellite", "Frequency"}, pos, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				
				if(mDish == null)
				{
					switch(which)
					{
					case 0 :
						showSatAdd();
						dialog.dismiss();
						break;
					}
				}
				else
				{
					switch(which)
					{
					case 0 :
						showSatAdd();
						dialog.dismiss();
						break;
					case 1 :
						showTsAdd();
						dialog.dismiss();
						break;
					}
				}
			}
		});

		builder.setNegativeButton(R.string.cancel, new  DialogInterface.OnClickListener(){
			//@Override
			public void onClick(DialogInterface dialog, int which) {
			// TODO Auto-generated method stub
				dialog.dismiss();
			}
		});	
		
		AlertDialog dialog = builder.create();
		dialog.setOnShowListener(new DialogInterface.OnShowListener(){
			public void onShow(DialogInterface dialog) {
			}         
		}); 	

		 dialog.setOnDismissListener(new DialogInterface.OnDismissListener(){
				public void onDismiss(DialogInterface dialog) {
//					t.quitLoop();	
				}         
		});	
		 
		dialog.show();
		WindowManager m = getWindowManager();   
		Display d = m.getDefaultDisplay();  	
		WindowManager.LayoutParams lp=dialog.getWindow().getAttributes();
		lp.dimAmount=0.0f;
		//lp.height = (int) (d.getHeight() * 0.6);  
		lp.width = (int) (d.getWidth() * 0.50);
		dialog.getWindow().setAttributes(lp);
		dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
	}
	
	private AlertDialog.Builder satAddBuilder;
	private View dvbs_sat_add;
	
	@SuppressWarnings("deprecation")
	private void showSatAdd()
	{
		//ContentValues values = null;
		satAddBuilder = new AlertDialog.Builder(this);
		LayoutInflater layoutInflater = LayoutInflater.from(this);

		dvbs_sat_add = layoutInflater.inflate(R.layout.dvbs_sat_add, null);
		satAddBuilder.setTitle(R.string.add_title);

		final EditText edittext_satname = (EditText) dvbs_sat_add.findViewById(R.id.edittext_sat_name);
		final Button button_sat_direction = (Button) dvbs_sat_add.findViewById(R.id.edit_direction);
		final EditText edittext_angle0 = (EditText) dvbs_sat_add.findViewById(R.id.angle0);
		
		edittext_angle0.setFilters(new InputFilter[] { new InputFilter.LengthFilter(3) });
		
		final EditText edittext_angle1 = (EditText) dvbs_sat_add.findViewById(R.id.angle1);
		edittext_angle1.setFilters(new InputFilter[] { new InputFilter.LengthFilter(1) });

		final TextView sat_no = (TextView) dvbs_sat_add.findViewById(R.id.sat_number);

		if (mSatList.getCount() != 0)
			sat_no.setText(String.valueOf(mSatList.getCount() + 1));
		else
			sat_no.setText(String.valueOf(1));

		edittext_satname.setText("");
		edittext_angle0.setText("");
		edittext_angle1.setText("");
		
		if(button_sat_direction.getText().toString() == "1")
			button_sat_direction.setText("West");
		else 
			button_sat_direction.setText("East");

		button_sat_direction.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View arg0) {
				// TODO Auto-generated method stub

				if (button_sat_direction.getText().equals("East")) {
					button_sat_direction.setText("West");
				} else {
					button_sat_direction.setText("East");
				}
			}
		});

		satAddBuilder.setView(dvbs_sat_add);

		satAddBuilder.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int id) {
				ContentValues values = new ContentValues();
				
				//values.put("sat_id", sat_no.getText().toString());
				values.put("sat_name", edittext_satname.getText().toString());
				
				if(		(edittext_angle0.getText().toString().equals(""))
					||	(edittext_angle0.getText().toString().equals(null))
					||	(edittext_angle1.getText().toString().equals(""))
					||	(edittext_angle1.getText().toString().equals(null))
				)
				{
					dialog.dismiss();
					return;
				}
				
				int sat_longitude = Integer.parseInt(edittext_angle0.getText().toString()) * 10
						+ Integer.parseInt(edittext_angle1.getText().toString());

				//East
				if (button_sat_direction.getText().equals("East"))
				{
					values.put("motor_position", 0);
				} else {
					values.put("motor_position", 1);
					sat_longitude = sat_longitude * (-1);
				}

				values.put("sat_longitude", sat_longitude);
				
				// set_default sat_lnb_info
				values.put("lnb_no", 1);
				values.put("lnb_type", 0);
				values.put("lnb_lof_lo", 10750);
				values.put("lnb_lof_hi", 10750);
				values.put("lnb_lof_th",  10750);
				values.put("lnb_power", 0);
				values.put("lnb_tone",  0);
				values.put("lnb_toneburst",  0);
				values.put("lnb_diseqc_mode", 0);
				values.put("lnb_diseqc_config10", 0);
				values.put("lnb_diseqc_config11", 0);
				values.put("lnb_cmd_sequence", 0);
				values.put("motor_diseqc_mode",  0+Dish.LNB_DISEQC_1_1);
				values.put("motor_id",  0);
				values.put("diseqc_repeat",  0);
				values.put("diseqc_fast",  0);
				
				mDishManager.insertSatInfo(values);
				
				updateSatList();
				
				dialog.dismiss();
			}
		});
		
		satAddBuilder.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int id) {
				dialog.dismiss();
			}
		});

		AlertDialog dialog = satAddBuilder.create();

		dialog.show();

		DisplayMetrics displayMetrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);

		WindowManager m = getWindowManager();
		Display d = m.getDefaultDisplay();
		WindowManager.LayoutParams lp = dialog.getWindow().getAttributes();
		lp.dimAmount = 0.0f;
		lp.width = (int) (d.getWidth() * 0.5);
		dialog.getWindow().setAttributes(lp);
		dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
	}
	
	private AlertDialog.Builder tsAddBuilder;
	private View dvbs_ts_add;
	
	@SuppressWarnings("deprecation")
	private void showTsAdd()
	{
		tsAddBuilder = new AlertDialog.Builder(this);
		LayoutInflater layoutInflater = LayoutInflater.from(this);

		dvbs_ts_add = layoutInflater.inflate(R.layout.dvbs_ts_add, null);
		tsAddBuilder.setTitle(R.string.add_title);
		
		final TextView ts_no = (TextView) dvbs_ts_add.findViewById(R.id.ts_number);
		final EditText edittext_tsFrequency = (EditText) dvbs_ts_add.findViewById(R.id.edittext_ts_frequency);
		final EditText edittext_tsSymbol = (EditText) dvbs_ts_add.findViewById(R.id.edittext_ts_symbol);
		final Button button_tsPolarity = (Button) dvbs_ts_add.findViewById(R.id.button_ts_polarity);

		if (mTsList.getCount() != 0)
			ts_no.setText(String.valueOf(mTsList.getCount() + 1));
		else
			ts_no.setText(String.valueOf(1));

		edittext_tsFrequency.setText("");
		edittext_tsSymbol.setText("");
		button_tsPolarity.setText("H");

		button_tsPolarity.setOnClickListener(new View.OnClickListener()
		{
			@Override
			public void onClick(View arg0)
			{
				// TODO Auto-generated method stub

				if (button_tsPolarity.getText().equals("V"))
				{
					button_tsPolarity.setText("H");
				}
				else
				{
					button_tsPolarity.setText("V");
				}
			}
		});

		tsAddBuilder.setView(dvbs_ts_add);

		tsAddBuilder.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener()
		{
			public void onClick(DialogInterface dialog, int id)
			{
				String	sFreq	= edittext_tsFrequency.getText().toString();
				String	sSymbol	= edittext_tsSymbol.getText().toString();
				int		iPolarisation;
				if (button_tsPolarity.getText().toString().equals("H"))
				{
					iPolarisation	= 0;
				}
				else
				{
					iPolarisation	= 1;
				}
				
				Cursor	cTs	= mDishManager.queryTsInfo(
						new String[] {"_id", "frequency", "symbol", "polarisation", "fec_inner"}
						, "sat_id=" + mSelectedSatID + " AND frequency=" + sFreq + " AND symbol=" + sSymbol + " AND polarisation=" + iPolarisation
						, null
				);
				
				int	iCount_Ts	= cTs.getCount();
				if(iCount_Ts <= 0)
				{
					Cursor	cList	= mDishManager.queryTsInfo(
							new String[] {"_id", "ts_id", "frequency", "symbol", "polarisation", "fec_inner"}
							, "sat_id=" + mSelectedSatID
							, null
							, "ts_id desc"
					);
					
					int	iID	= 0;
					if(cList.getCount() > 0)
					{
						cList.moveToFirst();
						iID	= cList.getInt(cList.getColumnIndex("ts_id")) + 1;
					}
					cList.close();
					
					ContentValues values = new ContentValues();
					
					values.put("ts_id", iID);
					values.put("frequency", sFreq);
					values.put("symbol", sSymbol);
					values.put("polarisation", iPolarisation);
					
					// set_default sat_lnb_info
					values.put("sat_id", mDish.getId());
					values.put("fec_inner", 0);
					
					mDishManager.insertTsInfo(values);

					updateTsList();
					dialog.dismiss();
				}
				else
				{
					getMainActivity().updateToast(getResources().getString(R.string.this_transporder_existed));
				}
				
				cTs.close();
			}
		});
		
		tsAddBuilder.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int id) {
				dialog.dismiss();
			}
		});

		AlertDialog dialog = tsAddBuilder.create();

		dialog.show();

		DisplayMetrics displayMetrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);

		WindowManager m = getWindowManager();
		Display d = m.getDefaultDisplay();
		WindowManager.LayoutParams lp = dialog.getWindow().getAttributes();
		lp.dimAmount = 0.0f;
		lp.width = (int) (d.getWidth() * 0.5);
		dialog.getWindow().setAttributes(lp);
		dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
		
	}
	
	private AlertDialog.Builder satEditBuilder;
	@SuppressWarnings("deprecation")
	private void showSatEdit()
	{
		//ContentValues values = null;
		satEditBuilder = new AlertDialog.Builder(this);
		LayoutInflater layoutInflater = LayoutInflater.from(this);

		dvbs_sat_add = layoutInflater.inflate(R.layout.dvbs_sat_add, null);
		satEditBuilder.setTitle(R.string.edit_title);

		final EditText edittext_satname = (EditText) dvbs_sat_add.findViewById(R.id.edittext_sat_name);
		final Button button_sat_direction = (Button) dvbs_sat_add.findViewById(R.id.edit_direction);
		final EditText edittext_angle0 = (EditText) dvbs_sat_add.findViewById(R.id.angle0);
		
		edittext_angle0.setFilters(new InputFilter[] { new InputFilter.LengthFilter(3) });
		
		final EditText edittext_angle1 = (EditText) dvbs_sat_add.findViewById(R.id.angle1);
		edittext_angle1.setFilters(new InputFilter[] { new InputFilter.LengthFilter(1) });

		final TextView sat_no = (TextView) dvbs_sat_add.findViewById(R.id.sat_number);

		sat_no.setText(String.valueOf(mDish.getId()));

		edittext_satname.setText(mDish.getSatName());
		
		double temp_satLongitude = mDish.getSatLongitude();
		
//		Log.d(TAG, "motorPosition ==== > " + mDish.getMotorPosition());
		
		if(mDish.getMotorPosition() == 1)
		{
			button_sat_direction.setText("West");
			temp_satLongitude = mDish.getSatLongitude() * (-1);
		}
		else
		{
			button_sat_direction.setText("East");
			//temp_satLongitude = mDish.getSatLongitude() * (-1);
		}
		
		edittext_angle0.setText(String.valueOf((int)temp_satLongitude / 10));
		edittext_angle1.setText(String.valueOf((int)temp_satLongitude % 10));

		button_sat_direction.setOnClickListener(new View.OnClickListener()
		{
			@Override
			public void onClick(View arg0)
			{
				if (button_sat_direction.getText().equals("East"))
				{
					button_sat_direction.setText("West");
				}
				else
				{
					button_sat_direction.setText("East");
				}
			}
		});

		satEditBuilder.setView(dvbs_sat_add);

		satEditBuilder.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int id) {
				ContentValues values = new ContentValues();
				
				//values.put("sat_id", sat_no.getText().toString());
				values.put("sat_name", edittext_satname.getText().toString());
				
				if ((edittext_angle0.getText().toString().equals(""))
						|| (edittext_angle0.getText().toString().equals(null))
						|| (edittext_angle1.getText().toString().equals(""))
						|| (edittext_angle1.getText().toString().equals(null)))
				{
					dialog.dismiss();
					return;
				}

				//East
				int sat_longitude = Integer.parseInt(edittext_angle0.getText().toString()) * 10
						+ Integer.parseInt(edittext_angle1.getText().toString());
						
				if (button_sat_direction.getText().equals("East"))
				{
					values.put("motor_position", 0);
					mDish.setMotorPosition(0);
				}
				else
				{
					values.put("motor_position", 1);
					sat_longitude = sat_longitude * (-1);
					mDish.setMotorPosition(1);
				}

				values.put("sat_longitude", sat_longitude);
				
				mDishManager.updateSatInfo(values, mDish.getId(), mDish);
				
				updateSatList();				
				dialog.dismiss();
			}
		});
		
		satEditBuilder.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int id) {
				dialog.dismiss();
			}
		});

		AlertDialog dialog = satEditBuilder.create();

		dialog.show();
		mSatAdapter.notifyDataSetChanged();

		DisplayMetrics displayMetrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);

		WindowManager m = getWindowManager();
		Display d = m.getDefaultDisplay();
		WindowManager.LayoutParams lp = dialog.getWindow().getAttributes();
		lp.dimAmount = 0.0f;
		lp.width = (int) (d.getWidth() * 0.5);
		dialog.getWindow().setAttributes(lp);
		dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
	}
	
	private AlertDialog.Builder tsEditBuilder;
	
	@SuppressWarnings("deprecation")
	private void showTsEdit()
	{
		tsEditBuilder = new AlertDialog.Builder(this);
		LayoutInflater layoutInflater = LayoutInflater.from(this);

		dvbs_ts_add = layoutInflater.inflate(R.layout.dvbs_ts_add, null);
		tsEditBuilder.setTitle(R.string.edit_title);
		
		final TextView ts_no = (TextView) dvbs_ts_add.findViewById(R.id.ts_number);
		final EditText edittext_tsFrequency = (EditText) dvbs_ts_add.findViewById(R.id.edittext_ts_frequency);
		final EditText edittext_tsSymbol = (EditText) dvbs_ts_add.findViewById(R.id.edittext_ts_symbol);
		final Button button_tsPolarity = (Button) dvbs_ts_add.findViewById(R.id.button_ts_polarity);

		ts_no.setText(String.valueOf(mTuner[mListTsId].getTsId()));
		edittext_tsFrequency.setText(String.valueOf(mTuner[mListTsId].getFrequency()));
		edittext_tsSymbol.setText(String.valueOf(mTuner[mListTsId].getSymbolRate()));
		
		if(mTuner[mListTsId].getPolarization() == 1)
			button_tsPolarity.setText("V");
		else
			button_tsPolarity.setText("H");

		button_tsPolarity.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View arg0) {
				// TODO Auto-generated method stub

				if (button_tsPolarity.getText().equals("H"))
				{
					button_tsPolarity.setText("V");
				}
				else
				{
					button_tsPolarity.setText("H");
				}
			}
		});

		tsEditBuilder.setView(dvbs_ts_add);

		tsEditBuilder.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener()
		{
			public void onClick(DialogInterface dialog, int id)
			{
				String	sFreq	= edittext_tsFrequency.getText().toString();
				String	sSymbol	= edittext_tsSymbol.getText().toString();
				int	iPolarisation;
				if (button_tsPolarity.getText().equals("H"))
				{
					iPolarisation	= 0;
				}
				else
				{
					iPolarisation	= 1;
				}
				
				Cursor	cTs	= mDishManager.queryTsInfo(
						new String[] {"_id", "frequency", "symbol", "polarisation", "fec_inner"},
						"sat_id=" + mSelectedSatID + " AND frequency=" + sFreq + " AND symbol=" + sSymbol + " AND polarisation=" + iPolarisation,
						null
				);
				
				int	iCount_Ts	= cTs.getCount();
				if(iCount_Ts <= 0)
				{
					ContentValues values = new ContentValues();
					
					values.put("frequency", sFreq);
					values.put("symbol", sSymbol);
					values.put("polarisation", iPolarisation);
					
					mDishManager.updateTsInfo(values, mTuner[mListTsId].getId(), mDish);
					
					updateTsList();
					dialog.dismiss();
				}
				else
				{
					getMainActivity().updateToast(getResources().getString(R.string.this_transporder_existed));
				}
			}
		});
		
		tsEditBuilder.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int id) {
				dialog.dismiss();
			}
		});

		AlertDialog dialog = tsEditBuilder.create();

		dialog.show();

		DisplayMetrics displayMetrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);

		WindowManager m = getWindowManager();
		Display d = m.getDefaultDisplay();
		WindowManager.LayoutParams lp = dialog.getWindow().getAttributes();
		lp.dimAmount = 0.0f;
		lp.width = (int) (d.getWidth() * 0.5);
		dialog.getWindow().setAttributes(lp);
		dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
	}

	private AlertDialog.Builder mDeleteBuilder;
	private void showDelete() {
		mDeleteBuilder = new AlertDialog.Builder(this)

		.setTitle(R.string.delete)
		.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {

			@Override
			public void onClick(DialogInterface dialog, int which) {

				if(mLnbList.getVisibility() == 8)
				{
					if(switch_flag == 0)
					{
						mDishManager.deleteSatInfo(mSelectedSatID);
					}
					else
					{
						mDishManager.deleteTsInfo(mSelectedTsId);
					}
				}
				else
				{
					mTuner = mDishManager.getTsList(mSelectedSatID);

					if(switch_flag == 0)
					{
						if(mTuner != null)
						{
							for(int i=0; i < mTuner.length ; i++)
							{
								if(mTuner == null)
									break;
							
								mDishManager.deleteTsInfo(mTuner[i].getId());
							}
						}
						mDishManager.deleteSatInfo(mSelectedSatID);
					}
					else
					{
						mDishManager.deleteTsInfo(mSelectedTsId);
					}
				}
				updateTsList();
				updateSatList();
				
				mDish = mDishManager.getDishSetup(mSelectedSatID);
				mLnbAdapter.notifyDataSetChanged();
				mLnbList.setAdapter(mLnbAdapter);
				
				dialog.dismiss();
			}
		})

		.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int id) {
				dialog.dismiss();
			}
		});

		AlertDialog alert = mDeleteBuilder.create();
		alert.show();
	}
	
	private  class cSetLocation
	{
		private TextView	tvLocation_text;
		private Button		bLocation_button;
		
		private TextView	tvLongitudeDirection_text;
		private Button		bLongitudeDirection_button;
		
		private TextView	tvLongitudeAngle_text;
		private EditText	etLongitudeAngle0;
		private TextView	tvLongitudeDirection_dot;
		private EditText	etLongitudeAngle1;
		
		private TextView	tvLatitudeDirection_text;
		private Button		bLatitudeDirection_button;
		
		private TextView	tvLatitudeAngle_text;
		private EditText	etLatitudeAngle0;
		private TextView	tvLatitudeDirection_dot;
		private EditText	etLatitudeAngle1;
		
		private AlertDialog.Builder builder_setLocation;
		private View view_setLocation;
	}
	cSetLocation cLocation = new cSetLocation();
	
	private void showSetLocation()
	{
		cLocation.builder_setLocation	= new AlertDialog.Builder(this);
		LayoutInflater	layoutInflater	= LayoutInflater.from(this);
		
		cLocation.view_setLocation	= layoutInflater.inflate(R.layout.dvbs_set_location, null);
		cLocation.builder_setLocation.setTitle(R.string.set_location);
		
		setComponent_Location();
		setFontSize_Location();
		setFilters_Location();
			
		cLocation.builder_setLocation.setView(cLocation.view_setLocation);
		
		setListener_Location();
		setValue_Location();
		
		AlertDialog dialog = cLocation.builder_setLocation.create();
		dialog.show();
	}
	
	private void setComponent_Location()
	{
		cLocation.tvLocation_text	= (TextView)cLocation.view_setLocation.findViewById(R.id.set_location_location_text);
		cLocation.bLocation_button	= (Button)cLocation.view_setLocation.findViewById(R.id.set_location_location_button);
		
		cLocation.tvLongitudeDirection_text		= (TextView)cLocation.view_setLocation.findViewById(R.id.set_location_longitude_direction_text);
		cLocation.bLongitudeDirection_button	= (Button)cLocation.view_setLocation.findViewById(R.id.set_location_longitude_direction_button);
		
		cLocation.tvLongitudeAngle_text	= (TextView)cLocation.view_setLocation.findViewById(R.id.set_location_longitude_angle_text);
		cLocation.etLongitudeAngle0		= (EditText)cLocation.view_setLocation.findViewById(R.id.set_location_longitude_angle0);
		cLocation.tvLongitudeDirection_dot			= (TextView)cLocation.view_setLocation.findViewById(R.id.set_location_longitude_dot);
		cLocation.etLongitudeAngle1		= (EditText)cLocation.view_setLocation.findViewById(R.id.set_location_longitude_angle1);
		
		cLocation.tvLatitudeDirection_text	= (TextView)cLocation.view_setLocation.findViewById(R.id.set_location_latitude_direction_text);
		cLocation.bLatitudeDirection_button	= (Button)cLocation.view_setLocation.findViewById(R.id.set_location_latitude_direction_button);
		
		cLocation.tvLatitudeAngle_text	= (TextView)cLocation.view_setLocation.findViewById(R.id.set_location_latitude_angle_text);
		cLocation.etLatitudeAngle0		= (EditText)cLocation.view_setLocation.findViewById(R.id.set_location_latitude_angle0);
		cLocation.tvLatitudeDirection_dot			= (TextView)cLocation.view_setLocation.findViewById(R.id.set_location_latitude_dot);
		cLocation.etLatitudeAngle1		= (EditText)cLocation.view_setLocation.findViewById(R.id.set_location_latitude_angle1);
	}
	
	private void setFontSize_Location()
	{
		cLocation.tvLocation_text.setTextSize(getTextSize(12));
		cLocation.bLocation_button.setTextSize(getTextSize(12));

		cLocation.tvLongitudeDirection_text.setTextSize(getTextSize(12));
		cLocation.bLongitudeDirection_button.setTextSize(getTextSize(12));

		cLocation.tvLongitudeAngle_text.setTextSize(getTextSize(12));
		cLocation.etLongitudeAngle0.setTextSize(getTextSize(12));
		cLocation.tvLongitudeDirection_dot.setTextSize(getTextSize(12));
		cLocation.etLongitudeAngle1.setTextSize(getTextSize(12));
		
		cLocation.tvLatitudeDirection_text.setTextSize(getTextSize(12));
		cLocation.bLatitudeDirection_button.setTextSize(getTextSize(12));

		cLocation.tvLatitudeAngle_text.setTextSize(getTextSize(12));
		cLocation.etLatitudeAngle0.setTextSize(getTextSize(12));
		cLocation.tvLatitudeDirection_dot.setTextSize(getTextSize(12));
		cLocation.etLatitudeAngle1.setTextSize(getTextSize(12));
	}

	private void setFilters_Location()
	{
		cLocation.etLongitudeAngle0.setFilters(new InputFilter[]{new InputFilter.LengthFilter(3)});
		cLocation.etLongitudeAngle1.setFilters(new InputFilter[]{new InputFilter.LengthFilter(1)});

		cLocation.etLatitudeAngle0.setFilters(new InputFilter[]{new InputFilter.LengthFilter(3)});
		cLocation.etLatitudeAngle1.setFilters(new InputFilter[]{new InputFilter.LengthFilter(1)});
	}
	
	private void setListener_Location()
	{
		cLocation.bLocation_button.setOnClickListener(listenerOnClick_bLocation);
		cLocation.bLocation_button.setOnKeyListener(listenerOnKey_bLocation);
		
		cLocation.bLongitudeDirection_button.setOnClickListener(listenerOnClick_bLocation);
		cLocation.bLongitudeDirection_button.setOnKeyListener(listenerOnKey_bLocation);
		
		cLocation.bLatitudeDirection_button.setOnClickListener(listenerOnClick_bLocation);
		cLocation.bLatitudeDirection_button.setOnKeyListener(listenerOnKey_bLocation);
		
		cLocation.builder_setLocation.setPositiveButton(R.string.ok, listenerOnClick_setLocation);
		cLocation.builder_setLocation.setNegativeButton(R.string.cancel, listenerOnClick_cancel);
	}
	
	private void setValue_Location()
	{
		DxbLog(I, "setValue_Location(" + mMotor.getLocation() + ", " + mMotor.getLongitudeDirection() + ", " + mMotor.getLatitudeDirection() + ")");
		
		CharSequence[] summaries = getResources().getTextArray(R.array.dvbs_location_country_entries);
		cLocation.bLocation_button.setText(summaries[mMotor.getLocation()]);

		if(mMotor.getLocation() == Motor.LOCATION_MAX)
		{
			setEnabled_Location(true);
			
			summaries = getResources().getTextArray(R.array.dvbs_location_longitude);
			cLocation.bLongitudeDirection_button.setText(summaries[mMotor.getLongitudeDirection()]);
			
			cLocation.etLongitudeAngle0.setText(Integer.toString(mMotor.getLongitudeAngle()/10));
			cLocation.etLongitudeAngle1.setText(Integer.toString(mMotor.getLongitudeAngle()%10));
			
			summaries = getResources().getTextArray(R.array.dvbs_location_latitude);
			cLocation.bLatitudeDirection_button.setText(summaries[mMotor.getLatitudeDirection()]);
			
			cLocation.etLatitudeAngle0.setText(Integer.toString(mMotor.getLatitudeAngle()/10));
			cLocation.etLatitudeAngle1.setText(Integer.toString(mMotor.getLatitudeAngle()%10));
		}
		else
		{
			setEnabled_Location(false);

			//	All East
			summaries = getResources().getTextArray(R.array.dvbs_location_longitude);
			cLocation.bLongitudeDirection_button.setText(summaries[0]);
			
			summaries = getResources().getTextArray(R.array.dvbs_location_longitude_algle);
			int	angle	= Integer.parseInt(summaries[mMotor.getLocation()].toString());
			cLocation.etLongitudeAngle0.setText(Integer.toString(angle/10));
			cLocation.etLongitudeAngle1.setText(Integer.toString(angle%10));
			
			//	All North
			summaries = getResources().getTextArray(R.array.dvbs_location_latitude);
			cLocation.bLatitudeDirection_button.setText(summaries[0]);
			
			summaries = getResources().getTextArray(R.array.dvbs_location_latitude_angle);
			angle	= Integer.parseInt(summaries[mMotor.getLocation()].toString());
			cLocation.etLatitudeAngle0.setText(Integer.toString(angle/10));
			cLocation.etLatitudeAngle1.setText(Integer.toString(angle%10));
		}
	}
	
	private void setEnabled_Location(boolean state)
	{
		//	setEnabled
		cLocation.bLongitudeDirection_button.setEnabled(state);
		cLocation.etLongitudeAngle0.setEnabled(state);
		cLocation.etLongitudeAngle1.setEnabled(state);
		cLocation.bLatitudeDirection_button.setEnabled(state);
		cLocation.etLatitudeAngle0.setEnabled(state);
		cLocation.etLatitudeAngle1.setEnabled(state);
		
		//	setFocusable
		cLocation.bLongitudeDirection_button.setFocusable(state);
		cLocation.etLongitudeAngle0.setFocusable(state);
		cLocation.etLongitudeAngle1.setFocusable(state);
		cLocation.bLatitudeDirection_button.setFocusable(state);
		cLocation.etLatitudeAngle0.setFocusable(state);
		cLocation.etLatitudeAngle1.setFocusable(state);
	}
	
	View.OnClickListener listenerOnClick_bLocation = new View.OnClickListener()
	{
		@Override
		public void onClick(View v)
		{
			DxbLog(I, "listenerOnKey_bLocation --> onKey(" + v + ")");
			
			if(v.getId() == R.id.set_location_location_button)
			{
				mMotor.setLocation((mMotor.getLocation() + 1) % (Motor.LOCATION_MAX+1));
				setValue_Location();
			}
			else if(v.getId() == R.id.set_location_longitude_direction_button)
			{
				mMotor.setLongitudeAngle((mMotor.getLongitudeAngle() + 1) % 2);
				CharSequence[] summaries = getResources().getTextArray(R.array.dvbs_location_longitude);
				cLocation.bLongitudeDirection_button.setText(summaries[mMotor.getLongitudeAngle()]);
			}
			else if(v.getId() == R.id.set_location_latitude_direction_button)
			{
				mMotor.setLatitudeAngle((mMotor.getLatitudeAngle() + 1) % 2);
				CharSequence[] summaries = getResources().getTextArray(R.array.dvbs_location_latitude);
				cLocation.bLatitudeDirection_button.setText(summaries[mMotor.getLatitudeAngle()]);
			}
		}
	};
	
	View.OnKeyListener listenerOnKey_bLocation = new View.OnKeyListener()
	{
		@Override
		public boolean onKey(View v, int keyCode, KeyEvent event)
		{
			DxbLog(I, "listenerOnKey_bLocation --> onKey(" + v + ", " + keyCode + ", " + event + ")");
			
			if(event.getAction() == KeyEvent.ACTION_DOWN)
			{
				switch(keyCode)
				{
					case KeyEvent.KEYCODE_DPAD_LEFT:
						if(v.getId() == R.id.set_location_location_button)
						{
							if(mMotor.getLocation() == Motor.LOCATION_MAX)
							{
								mMotor.setLongitudeAngle(	Integer.parseInt(cLocation.etLongitudeAngle0.getText().toString()) * 10
														+	Integer.parseInt(cLocation.etLongitudeAngle1.getText().toString()));
								mMotor.setLatitudeAngle(	Integer.parseInt(cLocation.etLatitudeAngle0.getText().toString()) * 10
														+	Integer.parseInt(cLocation.etLatitudeAngle1.getText().toString()));
							}
							
							mMotor.setLocation((mMotor.getLocation() + Motor.LOCATION_MAX) % (Motor.LOCATION_MAX+1));
							setValue_Location();
						}
						else if(v.getId() == R.id.set_location_longitude_direction_button)
						{
							mMotor.setLongitudeAngle((mMotor.getLongitudeAngle() + 1) % 2);
							CharSequence[] summaries = getResources().getTextArray(R.array.dvbs_location_longitude);
							cLocation.bLongitudeDirection_button.setText(summaries[mMotor.getLongitudeAngle()]);
						}
						else if(v.getId() == R.id.set_location_latitude_direction_button)
						{
							mMotor.setLatitudeAngle((mMotor.getLatitudeAngle() + 1) % 2);
							CharSequence[] summaries = getResources().getTextArray(R.array.dvbs_location_latitude);
							cLocation.bLatitudeDirection_button.setText(summaries[mMotor.getLatitudeAngle()]);
						}
						return true;
					
					case KeyEvent.KEYCODE_DPAD_RIGHT:
						if(v.getId() == R.id.set_location_location_button)
						{
							if(mMotor.getLocation() == Motor.LOCATION_MAX)
							{
								mMotor.setLongitudeAngle(	Integer.parseInt(cLocation.etLongitudeAngle0.getText().toString()) * 10
														+	Integer.parseInt(cLocation.etLongitudeAngle1.getText().toString()));
								mMotor.setLatitudeAngle(	Integer.parseInt(cLocation.etLatitudeAngle0.getText().toString()) * 10
														+	Integer.parseInt(cLocation.etLatitudeAngle1.getText().toString()));
							}
							
							mMotor.setLocation((mMotor.getLocation() + 1) % (Motor.LOCATION_MAX+1));
							setValue_Location();
						}
						else if(v.getId() == R.id.set_location_longitude_direction_button)
						{
							mMotor.setLongitudeAngle((mMotor.getLongitudeAngle() + 1) % 2);
							CharSequence[] summaries = getResources().getTextArray(R.array.dvbs_location_longitude);
							cLocation.bLongitudeDirection_button.setText(summaries[mMotor.getLongitudeAngle()]);
						}
						else if(v.getId() == R.id.set_location_latitude_direction_button)
						{
							mMotor.setLatitudeAngle((mMotor.getLatitudeAngle() + 1) % 2);
							CharSequence[] summaries = getResources().getTextArray(R.array.dvbs_location_latitude);
							cLocation.bLatitudeDirection_button.setText(summaries[mMotor.getLatitudeAngle()]);
						}
						return true;
				}
			}
			
			return false;
		}
	};
	
	DialogInterface.OnClickListener listenerOnClick_setLocation	= new DialogInterface.OnClickListener()
	{
		public void onClick(DialogInterface dialog, int id)
		{
			DxbLog(D, "listenerOnClick_setLocation --> onClick()");
			
			if(mMotor.getLocation() == Motor.LOCATION_MAX)
			{
				int	iLongitude	=		Integer.parseInt(cLocation.etLongitudeAngle0.getText().toString()) * 10
									+	Integer.parseInt(cLocation.etLongitudeAngle1.getText().toString());
				int	iLatitude	=		Integer.parseInt(cLocation.etLatitudeAngle0.getText().toString()) * 10
									+	Integer.parseInt(cLocation.etLatitudeAngle1.getText().toString());

				if(iLongitude==0)
				{
					getMainActivity().updateToast(getResources().getString(R.string.please_set_longitude_angle));
					return;
				}
				else if(iLatitude==0)
				{
					getMainActivity().updateToast(getResources().getString(R.string.please_set_latitude_angle));
					return;
				}

				mMotor.setAngle(mMotor.getLongitudeAngle(), mMotor.getLatitudeAngle());
			}
			else
			{
				CharSequence[] summaries = getResources().getTextArray(R.array.dvbs_location_longitude_algle);
				int	iLongitude	= Integer.parseInt(summaries[mMotor.getLocation()].toString());

				summaries = getResources().getTextArray(R.array.dvbs_location_latitude_angle);
				int	iLatitude	= Integer.parseInt(summaries[mMotor.getLocation()].toString());

				mMotor.setAngle(iLongitude, iLatitude);
			}
			
			ContentValues values = new ContentValues();
			
			values.put("location", mMotor.getLocation());
			values.put("longitude_direction", mMotor.getLongitudeDirection());
			values.put("longitude_angle", mMotor.getLongitudeAngle());
			values.put("latitude_direction", mMotor.getLatitudeDirection());
			values.put("latitude_angle", mMotor.getLatitudeAngle());
			
			mDishManager.updateMotorLocation(values);
			
			dialog.dismiss();
		}
	};
	
	DialogInterface.OnClickListener listenerOnClick_cancel = new DialogInterface.OnClickListener()
	{
		public void onClick(DialogInterface dialog, int id)
		{
			dialog.dismiss();
		}
	};
	
	private void setMotorAngle(Motor motor)
	{
		Motor setMotor	= motor;
		if(motor == null)
		{
			ContentValues values = new ContentValues();
			
			values.put("location", Motor.LOCATION_DEFAULT);
			values.put("longitude_direction", Motor.LONGITUDE_DIRECTION_DEFAULT);
			values.put("longitude_angle", Motor.LONGITUDE_ANGLE_DEFAULT);
			values.put("latitude_direction", Motor.LATITUDE_DIRECTION_DEFAULT);
			values.put("latitude_angle", Motor.LATITUDE_ANGLE_DEFAULT);

			mDishManager.insertMotorLocation(values);
			mMotor	= mDishManager.getMotoLocation();
			setMotor	= mMotor;
		}
		
		int iLocation	= setMotor.getLocation();
		if(iLocation<0 || iLocation>Motor.LOCATION_MAX)
		{
			setMotor.setLocation(Motor.LOCATION_DEFAULT);
			iLocation	= Motor.LOCATION_DEFAULT;
		}
		
		if(iLocation == Motor.LOCATION_MAX)
		{
			setMotor.setAngle(setMotor.getLongitudeAngle(), setMotor.getLatitudeAngle());
		}
		else
		{
			CharSequence[] summaries = getResources().getTextArray(R.array.dvbs_location_longitude_algle);
			int	iLongitude	= Integer.parseInt(summaries[iLocation].toString());

			summaries = getResources().getTextArray(R.array.dvbs_location_latitude_angle);
			int	iLatitude	= Integer.parseInt(summaries[iLocation].toString());

			setMotor.setAngle(iLongitude, iLatitude);
		}
	}

	private AlertDialog.Builder mScanBuilder;
	private void showScanConfigDialog()
	{
		mScanBuilder = new AlertDialog.Builder(this)
		.setTitle(R.string.scan_mode)
		.setItems(R.array.dvbs_scan_mode_entries, new DialogInterface.OnClickListener()
		{
			@Override
			public void onClick(DialogInterface dialog, int which)
			{
				DxbPlayer player = DVBPlayerActivity.getInstance().getPlayer();
				Log.d(TAG, "showScanConfigDialog() --> onClick() , which=" + which);
				switch(which)
				{
					case 0:	// Default
						player.eState = DxbPlayer.STATE.OPTION_FULL_SCAN;
						player.setSatelliteID(mSelectedSatID);
						player.setSatelliteName(mSelectedSatName.getText().toString());
					break;
					
					case 1:	// Blind Scan
						player.eState = DxbPlayer.STATE.OPTION_BLIND_SCAN;
						player.setSatelliteID(mSelectedSatID);
						player.setSatelliteName(mSelectedSatName.getText().toString());
					break;
				}

				isbackpressed = true;
				DishSetupActivity.this.finish();
				dialog.dismiss();
			}
		})
		.setNeutralButton(getResources().getString(R.string.cancel), new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				dialog.dismiss();
			}
		});

		AlertDialog alert = mScanBuilder.create();
		alert.setOnShowListener(new DialogInterface.OnShowListener() {
			public void onShow(DialogInterface dialog) {
			}
		});
		alert.setOnDismissListener(new DialogInterface.OnDismissListener() {
			public void onDismiss(DialogInterface dialog) {
			}
		});
		alert.show();
      }

	class SatAdapter extends CursorAdapter {

		@SuppressWarnings("deprecation")
		public SatAdapter(Context context, Cursor c) {
			super(context, c);
			// TODO Auto-generated constructor stub
		}

		@Override
		public void bindView(View arg0, Context arg1, Cursor arg2)
		{
			DxbLog(D, "bindView(" + arg0 + ", " + arg1 + ", " + arg2 + ")");
			
			//final ImageView image = (ImageView)arg0.findViewById(R.id.icon);
			final TextView sat_id = (TextView)arg0.findViewById(R.id.sat_id);
			final TextView direction = (TextView)arg0.findViewById(R.id.direction);
			final TextView sat_longitude = (TextView)arg0.findViewById(R.id.sat_longitude);
			final TextView sat_name = (TextView)arg0.findViewById(R.id.sat_name);

			//image.setImageResource(R.drawable.dxb_portable_dvbt_launcher_icon_36);
			//sat_id.setText(arg2.getString(arg2.getColumnIndex("sat_id")));
			
			sat_id.setText(Integer.toString(arg2.getPosition() + 1));
			
			int longitude = arg2.getInt(arg2.getColumnIndex("sat_longitude"));
			if (longitude >= 0) {
				direction.setText("East");
			} else {
				longitude *= -1;
				direction.setText("West");
			}

			sat_longitude.setText(Integer.toString(longitude/10) + "." + Integer.toString(longitude%10));
			sat_name.setText(arg2.getString(arg2.getColumnIndex("sat_name")));
			arg0.setTag(arg2.getString(arg2.getColumnIndex("sat_name")));
			
			if (!mSatList.isFocused() && mSelectedSatID == arg2.getInt(arg2.getColumnIndex("_id"))) {
				sat_id.setTextColor(Color.YELLOW);
				direction.setTextColor(Color.YELLOW);
				sat_longitude.setTextColor(Color.YELLOW);
				sat_name.setTextColor(Color.YELLOW);
			} else {
				sat_id.setTextColor(Color.WHITE);
				direction.setTextColor(Color.WHITE);
				sat_longitude.setTextColor(Color.WHITE);
				sat_name.setTextColor(Color.WHITE);
			}
		}

		@Override
		public View newView(Context arg0, Cursor arg1, ViewGroup arg2) {
			// TODO Auto-generated method stub
			LayoutInflater inflater = LayoutInflater.from(arg0);
			return inflater.inflate(R.layout.list_dvbs_sat, arg2, false);
		}
	}

	class TsAdapter extends CursorAdapter
	{
		@SuppressWarnings("deprecation")
		public TsAdapter(Context context, Cursor c)
		{
			super(context, c);
			// TODO Auto-generated constructor stub
		}

		@Override
		public void bindView(View arg0, Context arg1, Cursor arg2)
		{
			// TODO Auto-generated method stub
			//final ImageView image = (ImageView)arg0.findViewById(R.id.icon);
			final CheckBox	check_box	= (CheckBox)arg0.findViewById(R.id.ts_checkbox);
			final TextView ts_id = (TextView)arg0.findViewById(R.id.ts_id);
			final TextView frequency = (TextView)arg0.findViewById(R.id.frequency);
			final TextView symbol = (TextView)arg0.findViewById(R.id.symbol);
			final TextView polarization = (TextView)arg0.findViewById(R.id.polarisation);

			//image.setImageResource(R.drawable.dxb_portable_dvbt_launcher_icon_36);
			//ts_id.setText(Integer.toString(arg2.getInt(arg2.getColumnIndex("ts_id"))));
			ts_id.setText(Integer.toString(arg2.getPosition() + 1));
			frequency.setText(Integer.toString(arg2.getInt(arg2.getColumnIndex("frequency"))) + " MHz");
			symbol.setText(Integer.toString(arg2.getInt(arg2.getColumnIndex("symbol"))) + "KHz");
			
			int pol = arg2.getInt(arg2.getColumnIndex("polarisation"));
			if (pol == 0)
				polarization.setText("H");
			else if (pol == 1)
				polarization.setText("V");
			else
				polarization.setText("L");
			
			int	iCheck	= arg2.getInt(arg2.getColumnIndex("fec_inner"));
			if(iCheck == 1)
			{
				check_box.setChecked(true);
			}
			else
			{
				check_box.setChecked(false);
			}
		}

		@Override
		public View newView(Context arg0, Cursor arg1, ViewGroup arg2) {
			// TODO Auto-generated method stub
			LayoutInflater inflater = LayoutInflater.from(arg0);
			return inflater.inflate(R.layout.list_dvbs_ts, arg2, false);
		}
	}

	class LnbSetAdapter extends BaseAdapter
	{
        class ViewHolder
        {
            TextView text;
            TextView info;
            ImageButton iboolean;
        }

        private LayoutInflater mInflater;
		private String[] mItems;

		public LnbSetAdapter(Context context)
		{
			super();

			int i = 0;
			
			mItems = new String[10];
			mItems[i++] = context.getResources().getString(R.string.dish_setup_lnb_type);
			mItems[i++] = context.getResources().getString(R.string.dish_setup_lnb_power);
			mItems[i++] = context.getResources().getString(R.string.dish_setup_lnb_tone);
			mItems[i++] = context.getResources().getString(R.string.dish_setup_lnb_toneburst);
			mItems[i++] = context.getResources().getString(R.string.dish_setup_lnb_diseqc);
			mItems[i++] = context.getResources().getString(R.string.dish_setup_lnb_committed_command);
			mItems[i++] = context.getResources().getString(R.string.dish_setup_lnb_uncommitted_command);
			mItems[i++] = context.getResources().getString(R.string.dish_setup_lnb_diseqc_repeat);
			mItems[i++] = context.getResources().getString(R.string.dish_setup_lnb_command_order);
			mItems[i++] = context.getResources().getString(R.string.dish_setup_lnb_motor);

			mInflater = LayoutInflater.from(context);
		}
		
		@Override
		public int getCount()
		{
			// TODO Auto-generated method stub
			return mItems.length;
		}

		@Override
		public Object getItem(int arg0)
		{
			// TODO Auto-generated method stub
			return arg0;
		}

		@Override
		public long getItemId(int arg0)
		{
			// TODO Auto-generated method stub
			return arg0;
		}

		@Override
		public boolean isEnabled(int position)
		{
			DxbLog(D, "isEnabled(" + position + ")");
			
			if (mDish == null)
			{
				return false;
			}

			switch(position)
			{
				case DISHSETUP_LNB_COMMITTED_COMMAND :
					if(mDish.getLnbDiseqcMode() == Dish.LNB_DISEQC_NONE)
						return false;
					break;
				case DISHSETUP_LNB_UNCOMMITTED_COMMAND :
					if(mDish.getLnbDiseqcMode() != Dish.LNB_DISEQC_1_1)
						return false;
					break;
				case DISHSETUP_LNB_DISEQC_REPEATS :
					if(mDish.getLnbDiseqcMode() != Dish.LNB_DISEQC_1_1)
						return false;
					break;
				case DISHSETUP_LNB_COMMAND_ORDER :
					if(mDish.getLnbDiseqcMode() == Dish.LNB_DISEQC_NONE)
						return false;
					break;
			}
			
			return true;
		}

		@Override
		public View getView(int arg0, View arg1, ViewGroup arg2)
		{
			//DxbLog(I, "getView(" + arg0 + ", " + arg1 + ", " + arg2 + ")");
			
			ViewHolder holder;
			if (arg1 == null)
			{
				arg1 = mInflater.inflate(R.layout.list_dvbs_lnb_set, null);
				
				holder = new ViewHolder();
				holder.text = (TextView) arg1.findViewById(R.id.text);
				holder.info = (TextView) arg1.findViewById(R.id.info);
				holder.text.setTextSize(20);
				holder.info.setTextSize(20);
				
				arg1.setTag(holder);
				
				return arg1;
			}
			else
			{
				if(arg1.getWidth()==0 || arg1.getHeight()==0)
				{
					return arg1;
				}

				holder = (ViewHolder) arg1.getTag();
			}

			//DxbLog(D, "mItems[" + arg0 + "] = " + mItems[arg0]);
			holder.text.setText(mItems[arg0]);
			holder.text.setTextColor(Color.WHITE);

			holder.info.setVisibility(View.VISIBLE);

			if(mDish != null)
			{
				switch(arg0)
				{
					case DISHSETUP_LNB_TYPE:
						if (mDish.getLnbType() == Dish.LNB_TYPE_SINGLE)
							holder.info.setText("Single(" + mDish.getLnbLoLof() + ")");
						else if (mDish.getLnbType() == Dish.LNB_TYPE_UNIVERSAL)
							holder.info.setText("Universal(" + mDish.getLnbLoLof() + "/" + mDish.getLnbHiLof() + ")");
						holder.info.setTextColor(Color.YELLOW);
					break;
					
					case DISHSETUP_LNB_POWER:
						if (mDish.getLnbPower() == Dish.LNB_POWER_OFF)
							holder.info.setText("Off");
						else if (mDish.getLnbPower() == Dish.LNB_POWER_13V)
							holder.info.setText("13V");
						else if (mDish.getLnbPower() == Dish.LNB_POWER_18V)
							holder.info.setText("18V");
						else if (mDish.getLnbPower() == Dish.LNB_POWER_AUTO)
							holder.info.setText("Polarisation");
						holder.info.setTextColor(Color.YELLOW);
					break;
					
					case DISHSETUP_LNB_TONE:
						if (mDish.getLnbTone() == Dish.LNB_TONE_OFF)
							holder.info.setText("Off");
						else if (mDish.getLnbTone() == Dish.LNB_TONE_22K)
							holder.info.setText("On");
						else if (mDish.getLnbTone() == Dish.LNB_TONE_AUTO)
							holder.info.setText("Auto");
						holder.info.setTextColor(Color.YELLOW);
					break;
					
					case DISHSETUP_LNB_TONEBURST:
						if (mDish.getLnbToneburst() == Dish.LNB_TONEBURST_NONE)
							holder.info.setText("None");
						else if (mDish.getLnbToneburst() == Dish.LNB_TONEBURST_A)
							holder.info.setText("Mini-A");
						else if (mDish.getLnbToneburst() == Dish.LNB_TONEBURST_B)
							holder.info.setText("Mini-B");
						holder.info.setTextColor(Color.YELLOW);
					break;
					
					case DISHSETUP_LNB_DISEQC_MODE:
						if (mDish.getLnbDiseqcMode() == Dish.LNB_DISEQC_NONE)
							holder.info.setText("None");
						else if(mDish.getLnbDiseqcMode() == Dish.LNB_DISEQC_1_0)
							holder.info.setText("DisEqc 1.0");
						else if(mDish.getLnbDiseqcMode() == Dish.LNB_DISEQC_1_1)
							holder.info.setText("DisEqc 1.1");
						holder.info.setTextColor(Color.YELLOW);
					break;
					
					case DISHSETUP_LNB_COMMITTED_COMMAND:
						if (mDish.getLnbDiseqcConfig10() == 0)
							holder.info.setText("None");
						else if(mDish.getLnbDiseqcConfig10() == 1)
							holder.info.setText("LNB AA");
						else if(mDish.getLnbDiseqcConfig10() == 2)
							holder.info.setText("LNB AB");
						else if(mDish.getLnbDiseqcConfig10() == 3)
							holder.info.setText("LNB BA");
						else if(mDish.getLnbDiseqcConfig10() == 4)
							holder.info.setText("LNB BB");
							
						holder.info.setTextColor(Color.YELLOW);
	
						if(mDish.getLnbDiseqcMode() == Dish.LNB_DISEQC_NONE) {
							holder.info.setVisibility(View.INVISIBLE);
							holder.text.setTextColor(Color.DKGRAY);				
						}
					break;
					
					case DISHSETUP_LNB_UNCOMMITTED_COMMAND:
						if(mDish.getLnbDiseqcConfig11() == 0)
							holder.info.setText("None");
						else
							holder.info.setText("LNB " + mDish.getLnbDiseqcConfig11());
						
						holder.info.setTextColor(Color.YELLOW);
	
						if(mDish.getLnbDiseqcMode() != Dish.LNB_DISEQC_1_1)	{
							holder.info.setVisibility(View.INVISIBLE);
							holder.text.setTextColor(Color.DKGRAY);
						}
					break;

						case DISHSETUP_LNB_DISEQC_REPEATS:
						if(mDish.getDiseqcRepeat() == Dish.LNB_REPEAT_OFF)
							holder.info.setText("Off");
						else if(mDish.getDiseqcRepeat() == Dish.LNB_REPEAT_ON)
							holder.info.setText("On");
						holder.info.setTextColor(Color.YELLOW);
	
						if(mDish.getLnbDiseqcMode() != Dish.LNB_DISEQC_1_1)	{
							holder.info.setVisibility(View.INVISIBLE);
							holder.text.setTextColor(Color.DKGRAY);
						}					
					break;
					
					case DISHSETUP_LNB_COMMAND_ORDER:
						if(mDish.getLnbDiseqcMode() == 1)
						{
							if(mDish.getLnbSequence() == Dish.LNB_SEQUENCE_NONE)
								holder.info.setText("None");
							else if(mDish.getLnbSequence() == Dish.LNB_SEQUENCE_1)
								holder.info.setText("Committed, Toneburst");
							else if(mDish.getLnbSequence() == Dish.LNB_SEQUENCE_2)
								holder.info.setText("Toneburst, Committed");
							holder.info.setTextColor(Color.YELLOW);
						}
						else if(mDish.getLnbDiseqcMode() == 2)
						{
							if (mDish.getLnbSequence() == Dish.LNB_SEQUENCE_NONE)
								holder.info.setText("None");
							else if (mDish.getLnbSequence() == Dish.LNB_SEQUENCE_1)
								holder.info.setText("Committed, Uncommitted, Toneburst");
							else if (mDish.getLnbSequence() == Dish.LNB_SEQUENCE_2)
								holder.info.setText("Toneburst, Committed, Uncommitted");
							else if (mDish.getLnbSequence() == Dish.LNB_SEQUENCE_3)
								holder.info.setText("Uncommitted, Committed, Toneburst");
							else if (mDish.getLnbSequence() == Dish.LNB_SEQUENCE_4)
								holder.info.setText("Toneburst, Uncommitted, Committed");
							holder.info.setTextColor(Color.YELLOW);
						}
	
						if(mDish.getLnbDiseqcMode() == Dish.LNB_DISEQC_NONE) {
							holder.info.setVisibility(View.INVISIBLE);
							holder.text.setTextColor(Color.DKGRAY);
						}
					break;
						
					case DISHSETUP_LNB_MOTOR:
						Log.d(TAG, "mDish.getMotorDiseqcMode() = " + mDish.getMotorDiseqcMode());
						
						if(mDish.getMotorDiseqcMode() == (Dish.LNB_DISEQC_NONE+Dish.LNB_DISEQC_1_1))
						{
							holder.info.setText(getResources().getString(R.string.dish_setup_none));
						}
						else if(mDish.getMotorDiseqcMode() == Dish.LNB_DISEQC_1_2)
						{
							holder.info.setText(getResources().getString(R.string.dish_setup_diseqc2));
						}
						else if(mDish.getMotorDiseqcMode() == Dish.LNB_DISEQC_1_3)
						{
							holder.info.setText(getResources().getString(R.string.dish_setup_diseqc3));
						}
					break;
				}
			}
			
			return arg1;
		}
	}
	
	private void initIntentFilter()
	{
//		 Scheduler
		IntentFilter commandFilter = new IntentFilter();
		commandFilter.addAction(Alert.ACTION_ALERT_STATE);
		registerReceiver(mReceiverIntent, commandFilter);
	}
	
	private BroadcastReceiver mReceiverIntent = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			Log.i(TAG, "onReceive()");
			String action = intent.getAction();
			if(Alert.ACTION_ALERT_STATE.equals(action)) {
				Log.d(TAG, "if(Alert.ACTION_IS_ALERT.equals(action))");
				int iAlert_state	= intent.getIntExtra(Alert.EXTRA_TYPE_ALERT_STATE, Alert.TYPE_ALERT_STOP);
				if(iAlert_state == Alert.TYPE_ALERT_START) {
					//isSchedulerStart	= true;
				}
			}
		}
	};
	

	private int	iDisplayWidth;
	private int	iDisplayHeight;
	private float	fDensity;
	@SuppressWarnings("deprecation")
	private void initSystemSet()
	{
		Log.i(TAG, "initSystemSet()");
		
		Display display = ((WindowManager)getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();

		iDisplayWidth = display.getWidth();
		iDisplayHeight = display.getHeight();

		DisplayMetrics displayMetrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
		fDensity	= displayMetrics.density;
	}

	protected int getDisplayWidth() {
		return iDisplayWidth;
	}

	private int getDisplayHeight() {
		return iDisplayHeight;
	}

	private float getDisplayDensity() {
		return fDensity;
	}
	
	private float getTextSize(int iSize)
	{
		return	(iSize * getDisplayWidth() / getDisplayDensity() / 800);
	}
	
/*****************************************************************************************************************************************************************************
 *	Debug - log_message
 *****************************************************************************************************************************************************************************/

	protected static final int I = 0;
	protected static final int D = 1;
	protected static final int V = 2;
	protected static final int W = 3;
	protected static final int E = 4;

	protected void DxbLog(int level, String mString)
	{
		String TAG	= "[[[DishSetupActivity]]";
		
		if(TAG != null) {
			switch(level) {
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
