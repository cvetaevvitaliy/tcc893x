package com.telechips.tccAppsManager;

import java.util.ArrayList;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
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
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;


public class tccFavoriteViewAdapter extends BaseAdapter {
	
	public Context m_context;
	private LayoutInflater m_inflater;
	private static ArrayList<MyApplicationInfo> mFavoriteItems;
	private PackageManager m_PackMgr;
	private PackageInfo m_packInfo;
	//constructor
	public tccFavoriteViewAdapter(Context context)
	{
		this.m_context = context;
		m_inflater = (LayoutInflater) this.m_context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
	
		m_PackMgr = this.m_context.getApplicationContext().getPackageManager();
		
		mFavoriteItems = new ArrayList<MyApplicationInfo>();
		
	
	}
		
	public void Removeitem(int pos)
	{
		mFavoriteItems.remove(pos);
	}
	public void DeleteReceiveItem(String str)
	{
		for(int i=0; i < mFavoriteItems.size(); i++)
		{
			final MyApplicationInfo info = mFavoriteItems.get(i);
			if(info.PackageName.equals(str))
			{
				Removeitem(i);
							
			}
			
		}
		
	}
	public void StartFavoriteApk(int pos)
	{
		final MyApplicationInfo info = mFavoriteItems.get(pos);
		
		
		if(info ==null)
			Log.e("HOLIC", String.format("StartAPK_%d",pos) );
		
		m_context.startActivity(info.intent);
	}
	public String GetApkPath(int position)
	{
		return mFavoriteItems.get(position).sourceDir;
	}
	public boolean GetApkinfo(String path)
	{
	
		m_packInfo = m_PackMgr.getPackageArchiveInfo(path, 0);
		
		MyApplicationInfo myapplication = new MyApplicationInfo();
		 
		Intent i = m_PackMgr.getLaunchIntentForPackage(m_packInfo.applicationInfo.packageName);
		
		ResolveInfo ri = m_PackMgr.resolveActivity(i,0);
		
		if(ri == null)
		{
			Log.e("HOLIC", String.format("ResolveInfo NULL:%s",m_packInfo.applicationInfo.packageName) );
			return false;
			
		}	
			
		myapplication.title = ri.loadLabel(m_PackMgr);
				
		myapplication.setActivity(new ComponentName(
				   ri.activityInfo.applicationInfo.packageName,
				   ri.activityInfo.name),
                   Intent.FLAG_ACTIVITY_NEW_TASK
                   | Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
		
		myapplication.icon = ri.activityInfo.loadIcon(m_PackMgr);
		myapplication.sourceDir = ri.activityInfo.applicationInfo.sourceDir;
		myapplication.PackageName =  ri.activityInfo.applicationInfo.packageName;
		
		if(mFavoriteItems == null)
		{
			
			Log.e("HOLIC", String.format("mFavoriteItems is null") );
			return false;
		}
		mFavoriteItems.add(myapplication);
		
		return true;
		
	}
	@Override
	public int getCount() {
		// TODO Auto-generated method stub
		
		
		if(mFavoriteItems ==null)
			return 0;
	
		
		return mFavoriteItems.size();
			
	}

	@Override
	public Object getItem(int position) {
		// TODO Auto-generated method stub
		
		return mFavoriteItems.get(position);
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
		 MyApplicationInfo info;
		  
		 if(mFavoriteItems == null)
			 return convertView;
			 
		info  = mFavoriteItems.get(position);
		
		
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
	
	
}
