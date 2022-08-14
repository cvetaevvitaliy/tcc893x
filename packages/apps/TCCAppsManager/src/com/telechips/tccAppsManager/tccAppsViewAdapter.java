package com.telechips.tccAppsManager;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.ResolveInfo;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PaintFlagsDrawFilter;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.PaintDrawable;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.TextView;

public class tccAppsViewAdapter extends BaseAdapter {

	private PackageManager  m_packageManager;
	
	private LayoutInflater m_inflater;
	private static ArrayList<MyApplicationInfo> mApplications;
	private List<ResolveInfo> m_Apps;
	Context m_context;
	
	public tccAppsViewAdapter(Context context)
	{
		this.m_context = context;
		GetAllappsList();
		m_inflater = (LayoutInflater) this.m_context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
	}
	public void Removeitem(int pos)
	{
		mApplications.remove(pos);
		
		
	}
	public void AddItem(String packname)
	{
		
		
		  m_packageManager = this.m_context.getApplicationContext().getPackageManager();
		  
		PackageInfo m_packInfo;
	
		
		
		try{
			m_packInfo = m_packageManager.getPackageInfo(packname, 0);
			
		}
		catch(NameNotFoundException e){
			Log.e("HOLIC", String.format("AddItem_NameNotFoundException") );
			return;
		}
		
		for(int i=0; i < mApplications.size(); i++)
		{
			if(packname.equals(mApplications.get(i).PackageName))
			{
				return;
			}
			
		}
		
		
		MyApplicationInfo myapplication = new MyApplicationInfo();
		
		Intent i = m_packageManager.getLaunchIntentForPackage(m_packInfo.applicationInfo.packageName);
		
		ResolveInfo ri = m_packageManager.resolveActivity(i,0);
		
		if(ri == null)
		{
			Log.e("HOLIC", String.format("RI IS null") );
		}
		
		myapplication.title = ri.loadLabel(m_packageManager);
		
		myapplication.setActivity(new ComponentName(
				   ri.activityInfo.applicationInfo.packageName,
				   ri.activityInfo.name),
                   Intent.FLAG_ACTIVITY_NEW_TASK
                   | Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
		
		myapplication.icon = ri.activityInfo.loadIcon(m_packageManager);
		myapplication.sourceDir = ri.activityInfo.applicationInfo.sourceDir;
		myapplication.PackageName =  ri.activityInfo.applicationInfo.packageName;
	
		mApplications.add(myapplication);
		
		
		
	}
	public void StartApps(int position)
	{
	
		final MyApplicationInfo info = mApplications.get(position);
		
		
		if(info ==null)
			Log.e("HOLIC", String.format("StartAPK_%d",position) );
		
		m_context.startActivity(info.intent);
	
				
		
	}
	public boolean DeleteItems(String strPath)
	{
		for(int i=0; i < mApplications.size(); i++)
		{
			final MyApplicationInfo info = mApplications.get(i);
			
			if(info.PackageName.equals(strPath))
			{
				Removeitem(i);
				return true;
				
			}
			
		}
		return false;
		
	}
	private void GetAllappsList()
	{
		Intent intent = new Intent(Intent.ACTION_MAIN, null);
		
	    intent.addCategory(Intent.CATEGORY_LAUNCHER);
	 
	    m_packageManager = this.m_context.getApplicationContext().getPackageManager();
	 
	    m_Apps = m_packageManager.queryIntentActivities(intent, 0);
	      
	   
	    Collections.sort(m_Apps, new ResolveInfo.DisplayNameComparator(m_packageManager));
	
	    if (m_Apps != null) {
            final int count = m_Apps.size();
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
            if (mApplications == null) {
                mApplications = new ArrayList<MyApplicationInfo>(count);
            }
            mApplications.clear();

            for (int i = 0; i < count; i++) {
            	MyApplicationInfo application = new MyApplicationInfo();
                ResolveInfo info = m_Apps.get(i);
               
           
                application.title = info.loadLabel(m_packageManager);
               
                application.setActivity(new ComponentName(
                        info.activityInfo.applicationInfo.packageName,
                        info.activityInfo.name),
                        Intent.FLAG_ACTIVITY_NEW_TASK
                        | Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
                
                application.icon = info.activityInfo.loadIcon(m_packageManager);
                application.sourceDir = info.activityInfo.applicationInfo.sourceDir; 
                application.PackageName = info.activityInfo.applicationInfo.packageName;
                
                         
                mApplications.add(application);
            }
        }
	    
	    
	    
	}
	public String GetApkPath(int position)
	{
		return mApplications.get(position).sourceDir;
	}
	public String GetApkUniq(int position)
	{
		return mApplications.get(position).PackageName;
	}
	@Override
	public int getCount() {
		// TODO Auto-generated method stub
		return mApplications.size();
	}

	@Override
	public Object getItem(int position) {
		// TODO Auto-generated method stub
		return mApplications.get(position);
	}

	@Override
	public long getItemId(int position) {
		// TODO Auto-generated method stub
		return position;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		// TODO Auto-generated method stub
		 final Rect mOldBounds = new Rect(); 
		final MyApplicationInfo info = mApplications.get(position);
		  
		if(convertView == null)
		{
			convertView = m_inflater.inflate(R.layout.grid_cell,null);
		}
		   Drawable icon = info.icon;

           if (!info.filtered)
           {
               //final Resources resources = getContext().getResources();
               int width = 72;//(int) resources.getDimension(android.R.dimen.app_icon_size);
               int height = 72;//(int) resources.getDimension(android.R.dimen.app_icon_size);

               final int iconWidth = icon.getIntrinsicWidth();
               final int iconHeight = icon.getIntrinsicHeight();

               if (icon instanceof PaintDrawable) {
                   PaintDrawable painter = (PaintDrawable) icon;
                   painter.setIntrinsicWidth(width);
                   painter.setIntrinsicHeight(height);
               }

               if (width > 0 && height > 0 && (width < iconWidth || height < iconHeight)) {
                   final float ratio = (float) iconWidth / iconHeight;

                   if (iconWidth > iconHeight) {
                       height = (int) (width / ratio);
                   } else if (iconHeight > iconWidth) {
                       width = (int) (height * ratio);
                   }

                   final Bitmap.Config c =
                           icon.getOpacity() != PixelFormat.OPAQUE ?
                               Bitmap.Config.ARGB_8888 : Bitmap.Config.RGB_565;
                   final Bitmap thumb = Bitmap.createBitmap(width, height, c);
                   final Canvas canvas = new Canvas(thumb);
                   canvas.setDrawFilter(new PaintFlagsDrawFilter(Paint.DITHER_FLAG, 0));
                   // Copy the old bounds to restore them later
                   // If we were to do oldBounds = icon.getBounds(),
                   // the call to setBounds() that follows would
                   // change the same instance and we would lose the
                   // old bounds
                   mOldBounds.set(icon.getBounds());
                   icon.setBounds(0, 0, width, height);
                   icon.draw(canvas);
                   icon.setBounds(mOldBounds);
                   icon = info.icon = new BitmapDrawable(thumb);
                   info.filtered = true;
               }
           }

           final TextView textView = (TextView)convertView.findViewById(R.id.txt_cell);
           final ImageView imgv = (ImageView)convertView.findViewById(R.id.img_cell);
           imgv.setImageDrawable(icon);
           textView.setText(info.title);
           	
           
		return convertView;
	}
	@Override
	public void notifyDataSetChanged() {
		// TODO Auto-generated method stub
		super.notifyDataSetChanged();
	}
	

}
