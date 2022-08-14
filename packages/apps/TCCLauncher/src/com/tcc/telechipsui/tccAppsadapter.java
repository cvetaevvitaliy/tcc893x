package com.tcc.telechipsui;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
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
import android.widget.ImageView;
import android.widget.TextView;

public class tccAppsadapter extends BaseAdapter {

	private PackageManager  m_packageManager;
	private LayoutInflater m_inflater;
	private static ArrayList<tccAppsinfo> mApplications;
	private List<ResolveInfo> m_Apps;
	Context m_context;
	
	//constructor
	public tccAppsadapter(Context context)
	{
		m_context = context;
		
		
	//	GetAllappsList();
		m_inflater = (LayoutInflater) this.m_context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		
		
		
	}
	private void GetAllappsList()
	{
		Intent intent = new Intent(Intent.ACTION_MAIN, null);
	    intent.addCategory(Intent.CATEGORY_LAUNCHER);
	 
	    
	    m_packageManager = m_context.getApplicationContext().getPackageManager();
	 
	    
	    m_Apps = m_packageManager.queryIntentActivities(intent, 0);
	      
	    Collections.sort(m_Apps, new ResolveInfo.DisplayNameComparator(m_packageManager));
	
	    if (m_Apps != null) {
            final int count = m_Apps.size();
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
            if (mApplications == null) {
                mApplications = new ArrayList<tccAppsinfo>(count);
            }
            mApplications.clear();

            for (int i = 0; i < count; i++) {
            	tccAppsinfo application = new tccAppsinfo();
                ResolveInfo info = m_Apps.get(i);
               
           
                application.title = info.loadLabel(m_packageManager);
                application.setActivity(new ComponentName(
                        info.activityInfo.applicationInfo.packageName,
                        info.activityInfo.name),
                        Intent.FLAG_ACTIVITY_NEW_TASK
                        | Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
                application.icon = info.activityInfo.loadIcon(m_packageManager);

          //      Log.e("HOLIC", String.format("packagename:%s",info.activityInfo.applicationInfo.packageName) );
                               
          //      Log.e("HOLIC", String.format("activity_name:%s",info.activityInfo.name) );
                
           //     Log.e("HOLIC", String.format("label_name:%s",application.title) );
                
                
                mApplications.add(application);
            }
        }
	    
	    
	    
	}
	public void StartApk(int position, long id)
	{
		
		final tccAppsinfo info = mApplications.get(position);
		
		
		if(info ==null)
		{
			
		}
		m_context.startActivity(info.intent);
		
	
		
	}
	
	@Override
	public int getCount() {
		// TODO Auto-generated method stub
		return m_Apps.size();
	}

	@Override
	public Object getItem(int position) {
		// TODO Auto-generated method stub
		return m_Apps.get(position);
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
		final tccAppsinfo info = mApplications.get(position);
			  
		
		if(convertView == null)
			{
				convertView = m_inflater.inflate(R.layout.grid_cell,null);
							
	
			}
			Drawable icon = info.icon;
				
	           if (!info.filtered) {
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

	       //        if (width > 0 && height > 0 && (width < iconWidth || height < iconHeight)) 
	               {
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
