package com.telechips.tccAppsManager;



import android.app.Activity;
import android.content.Context;
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




public class tccAppsViewMgrActivity extends Activity 
implements AdapterView.OnItemSelectedListener,AdapterView.OnItemLongClickListener, AdapterView.OnItemClickListener,OnKeyListener {

	
	private GridView m_gridViewImages;
	public tccAppsViewAdapter m_gridAdapter;
	public Context m_context;
	public View m_View;
	public Activity frgactivity;
	private int m_iCurrentPos;

		
	@Override
	 public void onItemClick(AdapterView<?> parent, View v, int position, long id) 
	 {
	   	 
		/*				
			 m_gridViewImages.setSelector(R.drawable.appselect);
			 m_iCurrentPos = position;
			 
			 m_gridViewImages.smoothScrollToPosition(m_iCurrentPos);
			 			
			 SetInvalidatePos();
		*/
		 m_gridViewImages.setSelector(R.drawable.appselect);
		 m_iCurrentPos = position;
		 SetInvalidatePos();
		 m_gridAdapter.StartApps(m_iCurrentPos);
			 
    }
	public boolean onItemLongClick(AdapterView<?> parent, View v, int position, long id)
{
		
				
			 m_gridViewImages.setSelector(R.drawable.appselect);
			 m_iCurrentPos = position;
	//		 Log.e("HOLIC", String.format("onItemLongClick : %d id: %d ",position,id) );
			 SetInvalidatePos();
			 
			 String str =  m_gridAdapter.GetApkPath(m_iCurrentPos);
				String str_uniq = m_gridAdapter.GetApkUniq(m_iCurrentPos);	
				
				//없다 
				if(tccAppsMainView.SetApkPath(str, str_uniq)==true)
				{
					frgactivity.showDialog(3);
				}
				else //이미 있다.
				{
					frgactivity.showDialog(1);
				}
	//		 m_gridAdapter.StartApps(m_iCurrentPos);
			
		 
		return true;
}
	@Override
	public void onNothingSelected(AdapterView<?> arg0) {
		
			 
		m_gridViewImages.setSelector(new ColorDrawable(Color.TRANSPARENT));
		m_gridViewImages.invalidate();
		m_iCurrentPos = -1;
	 
	
	}	
	@Override
	public void onItemSelected(AdapterView<?> parentView, View selectedItemView, int position, long id) {
		
	
		m_gridViewImages.setSelector(R.drawable.appselect);
		m_iCurrentPos = position;
		SetInvalidatePos();
		
	// Log.e("HOLIC", String.format("onItemSelected : %d id:%d ",m_iCurrentPos,id) );
	
	}
	public void SetInvalidatePos()
	{
		
		m_gridViewImages.requestFocusFromTouch();
		m_gridViewImages.setSelection(m_iCurrentPos);
	
			 
	}
	public tccAppsViewMgrActivity(Context context, Activity activity)
	{
		m_context =  context;
		frgactivity = 	activity;	
		
		m_iCurrentPos = -1;
	}


	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
				
	super.onCreate(savedInstanceState);
			
	}
	
	public boolean onKey(View v, int keyCode, KeyEvent event) {
	
		
		 
		if(event.getAction() == KeyEvent.ACTION_DOWN )
		{
			
					
			if(m_iCurrentPos >=0)
			{		
			
			if(keyCode ==82) //menu key
			{
				String str =  m_gridAdapter.GetApkPath(m_iCurrentPos);
				String str_uniq = m_gridAdapter.GetApkUniq(m_iCurrentPos);	
				
				//없다 
				if(tccAppsMainView.SetApkPath(str, str_uniq)==true)
				{
					frgactivity.showDialog(3);
				}
				else //이미 있다.
				{
					frgactivity.showDialog(1);
				}
					
				return false;
		
			}
			else if(keyCode ==66) //enter key
			{
				m_gridAdapter.StartApps(m_iCurrentPos);
				return false;
			}
			
		 }		
		}
					
		return false;	
		}

	public View SetLayOut(int resource, LayoutInflater inflater,ViewGroup container)
	{
	
		m_View  = inflater.inflate(resource, container, false);
		
		m_gridViewImages = (GridView)m_View.findViewById(R.id.gridiconview);
		
		m_gridViewImages.setSelector(R.drawable.appselect);
		
		m_gridViewImages.setVerticalScrollBarEnabled(false);
		
	
	
		SetAdapter();
	
		
		return m_View;
	
		
	}
	public void AddPackageItem(String packname)
	{
		
		m_gridAdapter.AddItem(packname);
		m_gridAdapter.notifyDataSetChanged();
		m_gridViewImages.invalidate();
		
	}
	public void DeleteReceiverItems(String str)
	{
		
		
		m_gridAdapter.DeleteItems(str);
		
		m_iCurrentPos =-1;
		m_gridAdapter.notifyDataSetChanged();
		m_gridViewImages.invalidate();
	}
	public void SetAdapter()
	{
		 m_gridAdapter = new tccAppsViewAdapter(this.m_context);
		
		m_gridViewImages.setAdapter(m_gridAdapter);
		
		m_gridViewImages.setOnItemSelectedListener(this);
			
		m_gridViewImages.setOnKeyListener(this);
		
		m_gridViewImages.setOnItemClickListener(this);
		
		m_gridViewImages.setOnItemLongClickListener(this);
		
	}
	
	
	
}
