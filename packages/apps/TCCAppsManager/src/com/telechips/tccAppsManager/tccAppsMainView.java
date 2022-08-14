package com.telechips.tccAppsManager;


import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;
import android.database.Cursor;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnKeyListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.GridView;



public class tccAppsMainView extends Activity 
implements AdapterView.OnItemSelectedListener,AdapterView.OnItemLongClickListener,AdapterView.OnItemClickListener, OnKeyListener {

	private static Context m_context;
	public View m_Mainview;
	public static tccFavoriteViewAdapter m_FavoriteAdapter;
	private int m_iCurrentPos;
	private static GridView m_gridFavoriteImg;
	private static tccDatabase m_tccDatabase;
	private static Cursor mCursor;
	public Activity m_frgactivity;
	private static String m_sSelectItemString;
	private static String m_sUniqItemString;
	
	@Override
	 public void onItemClick(AdapterView<?> parent, View v, int position, long id) 
	 {
	
		/* m_gridFavoriteImg.setSelector(R.drawable.appselect);
		 m_iCurrentPos = position;
		
		 m_gridFavoriteImg.smoothScrollToPosition(m_iCurrentPos);
		 
		 SetInvalidatePos();*/
		 m_gridFavoriteImg.setSelector(R.drawable.appselect);
	 	 m_iCurrentPos = position;
	 	
	 	 SetInvalidatePos();
	 	
	 	 m_FavoriteAdapter.StartFavoriteApk(m_iCurrentPos);
	 }
	 public boolean onItemLongClick(AdapterView<?> parent, View v, int position, long id)
	 {
	 	 				
		 m_gridFavoriteImg.setSelector(R.drawable.appselect);
	 	 m_iCurrentPos = position;
	 	
	 	 SetInvalidatePos();
	 		
		 m_frgactivity.showDialog(2);
		 /*
		 m_gridFavoriteImg.setSelector(R.drawable.appselect);
	 	 m_iCurrentPos = position;
	 	
	 	 SetInvalidatePos();
	 	
	 	 m_FavoriteAdapter.StartFavoriteApk(m_iCurrentPos);
	 		*/ 		 
	 		return true;
	 }
	 public void onItemSelected(AdapterView<?> parentView, View selectedItemView, int position, long id)
	 {
			
		 m_gridFavoriteImg.setSelector(R.drawable.appselect);
			m_iCurrentPos = position;
			SetInvalidatePos();
		
	}

	public void onNothingSelected(AdapterView<?> arg0) {
		
		m_gridFavoriteImg.setSelector(new ColorDrawable(Color.TRANSPARENT));
		m_gridFavoriteImg.invalidate();
		m_iCurrentPos = -1;
			// Empty
	}

	
	public void SetInvalidatePos()
	{
		
		m_gridFavoriteImg.requestFocusFromTouch();
		m_gridFavoriteImg.setSelection(m_iCurrentPos);
		 
	}
	
	public void DeleteReceiverItems(String strPath)
	{
		
				
		mCursor = m_tccDatabase.getMatchUniqName(strPath);
		mCursor.moveToFirst();
		
		if(mCursor != null && mCursor.getCount() != 0)
		{
			//	mCursor.geti
			int id = mCursor.getInt(mCursor.getColumnIndex("_id"));
				
			m_tccDatabase.deleteColumn(id);
			mCursor.close();
	
			m_FavoriteAdapter.DeleteReceiveItem(strPath);
			
			m_iCurrentPos = -1;
			m_FavoriteAdapter.notifyDataSetChanged();
			m_gridFavoriteImg.invalidate();
				
			
		}
	}
	public void DeleteItems()
	{
		String strPath;
			
		strPath = m_FavoriteAdapter.GetApkPath(m_iCurrentPos);
		mCursor = m_tccDatabase.getMatchName(strPath);
		mCursor.moveToFirst();
			
		if(mCursor != null && mCursor.getCount() != 0)
		{
			//	mCursor.geti
			int id = mCursor.getInt(mCursor.getColumnIndex("_id"));
				
				
			m_tccDatabase.deleteColumn(id);
			mCursor.close();
			//adapter 
			m_FavoriteAdapter.Removeitem(m_iCurrentPos);
			
			m_iCurrentPos = -1;
			m_FavoriteAdapter.notifyDataSetChanged();
			m_gridFavoriteImg.invalidate();
				
			
		}
		
		
		
	}
		
	public static void AddFavoriteItems()
	{
		mCursor = m_tccDatabase.getMatchName(m_sSelectItemString);
		mCursor.moveToFirst();
		m_tccDatabase.insertColumn(m_sSelectItemString,m_sUniqItemString);
		m_FavoriteAdapter.GetApkinfo(m_sSelectItemString);
		m_FavoriteAdapter.notifyDataSetChanged();
		m_gridFavoriteImg.invalidate();
	
		
	}
	public static boolean SetApkPath(String str, String str_uniq)
	{
			
		//	
		mCursor = m_tccDatabase.getMatchName(str);
		mCursor.moveToFirst();
			
		if(mCursor != null && mCursor.getCount() != 0)
		{
			String stz = mCursor.getString(mCursor.getColumnIndex("Apkname"));
			
			//이미 있는 경우 
			return false;
		}
		else
		{
			m_sSelectItemString = str;
			m_sUniqItemString = str_uniq;
			
		/*	m_tccDatabase.insertColumn(str,str_uniq);
			m_FavoriteAdapter.GetApkinfo(str);
			m_FavoriteAdapter.notifyDataSetChanged();
			m_gridFavoriteImg.invalidate();
			
		
			*/
			return true;
		}	
		
	}
	public void initDB()
	{
		m_tccDatabase = new tccDatabase(m_context);
		m_tccDatabase.open();
		
		String szApk;
		mCursor = null;
		mCursor = m_tccDatabase.getAllColumns();
		mCursor.getCount();
		
		 
		 //DB에 입력 
		 while(mCursor.moveToNext())
		 {
			 szApk =  mCursor.getString(mCursor.getColumnIndex("Apkname"));
			 m_FavoriteAdapter.GetApkinfo(szApk);
			 
		 }
		 
		 mCursor.close();
		 	
		 m_FavoriteAdapter.notifyDataSetChanged();
		 m_gridFavoriteImg.invalidate();
		 
		 
		 
	}
	public tccAppsMainView(Context context, Activity frg_activity)  //constructor
	{
		this.m_context = context;
		m_frgactivity = frg_activity;
		
		m_iCurrentPos = -1;
	}
	public void SetAdapter()
	{
		m_FavoriteAdapter = new tccFavoriteViewAdapter(this.m_context);
		m_gridFavoriteImg.setAdapter(m_FavoriteAdapter);
		
		DBopen();
		initDB();
		
		
	}
	public void DBopen()
	{
		
		String dir = "/data/data/com.telechips.tccAppsManager/databases/";
		String fname = "cometelechipsfavorite.db";
		  File folder =new File(dir);
		  if(folder.exists()){
			  
				   
		  }else{
		   folder.mkdirs();
	
		  }
		  
		  File dbfile =  new File(dir+fname);
		  if(dbfile.exists())
		  {
			  
			  return;
			  
		  }
		  else
		  {
			
			  AssetManager aman = m_context.getResources().getAssets();
		
			  InputStream in = null;
			  FileOutputStream out = null;
			  long filesize=0;
			  
			  try{
			   in = aman.open("cometelechipsfavorite.db",AssetManager.ACCESS_BUFFER);
			   filesize = in.available();
			   
			   
			    byte[] tmpbyte = new byte[(int)filesize];
			    in.read(tmpbyte);
			    in.close();
			    dbfile.createNewFile();
			    out = new FileOutputStream(dbfile);
			    out.write(tmpbyte);
			    out.close();
			  
			  }
			  catch(IOException e)
			  {
			   System.out.println("DBCreate Error ["+e+"]");
			  }			
		
		  }
	}
	
	public View SetLayOut(int resource, LayoutInflater inflater,ViewGroup container)
	{
		
		m_Mainview = inflater.inflate(resource, container, false);
		
		
		m_gridFavoriteImg = (GridView)m_Mainview.findViewById(R.id.gridifavorite);
	
		if(m_gridFavoriteImg ==null)
		{
			Log.e("HOLIC", String.format("m_gridFavoriteImg_NULL") );
			
		}
		
		m_gridFavoriteImg.setSelector(R.drawable.appselect);
		
		
		SetAdapter();
		m_gridFavoriteImg.setOnItemSelectedListener(this);
		m_gridFavoriteImg.setOnKeyListener(this);
		
		m_gridFavoriteImg.setOnItemClickListener(this);
		
		m_gridFavoriteImg.setOnItemLongClickListener(this);
				
		//onNothingSelected
	
		return m_Mainview;
	}
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		
		
	}

	@Override
	public void onContentChanged() {
		// TODO Auto-generated method stub
		
		super.onContentChanged();
	}
	
   

public boolean onKey(View v, int keyCode, KeyEvent event) {
	
		if(event.getAction() == KeyEvent.ACTION_DOWN )
		{
			if(m_iCurrentPos >=0)
			{	
			
				if(keyCode ==82) //menu key
				{
				
				m_frgactivity.showDialog(2);
					
	
				return false;
		
				}
				else if(keyCode ==66)
				{
				m_FavoriteAdapter.StartFavoriteApk(m_iCurrentPos);
				return false;
				}
			}	
		}
					
		return false;
	}
@Override
protected void onDestroy() {
	// TODO Auto-generated method stub
		m_tccDatabase.close();
	
	super.onDestroy();
}
 
	
}
