package com.telechips.tccapkinstaller;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnShowListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.database.DataSetObserver;
import android.graphics.Color;
import android.graphics.drawable.PaintDrawable;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.example.tccapkinstaller.R;

public class TccapkinstallActivity extends Activity
implements AdapterView.OnItemClickListener,  AdapterView.OnItemSelectedListener{

	private ListView mListView;
	private LayoutInflater m_inflater;
	private  EditText m_pathtxt;
		
	
	 static ArrayList<TccPackageInfo> mTccPackInfo;
	 static PackInfoAdapter m_Pack_adapter;
		
	
	private static final int INDEX_NONE = -1;
	
	private static final String rootMain="/storage/sdcard0";
	private static final String rootSD="/storage/sdcard1";
	private static final String rootUSB="/storage/usb0";
	private static final String rootUSB1="/storage/usb1";
	private boolean m_isSDCard1;
	private boolean m_isUsb;
	private boolean m_isUsb1;
	private int m_iCurrentStorage;
	
	public PackageManager m_PackMgr;
	 private List<String> Dlglist = new ArrayList<String>();
	 
	
	
//	private ArrayList<CharSequence> m_StrorageList;
	//private PackageInfo m_packInfo;
	 private static final boolean DEBUG = true;
	 private static final String TAG = "tccApkInstaller";
	 
	 private final BroadcastReceiver  m_Apkreceiver = new ApkinstallReceiver();
	 private final BroadcastReceiver m_Storagereceiver = new ExternalReceiver();
	
	
	@SuppressWarnings("deprecation")
	@Override
	protected void onCreate(Bundle savedInstanceState)  {
		super.onCreate(savedInstanceState);
		this.requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);
		setContentView(R.layout.activity_tccapkinstall);
		
		this.getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE, R.layout.custom_title);
		
		m_inflater = (LayoutInflater) getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		
		Dlglist.clear();
		 m_isSDCard1 = false;
		m_isUsb = false;
		m_isUsb1 = false;
		m_iCurrentStorage = -1;
		
		if(isExternalStroage(rootSD))
		{
			m_isSDCard1 =true;
		}
		if(isExternalStroage(rootUSB))
		{
			m_isUsb = true;
		}
		if(isExternalStroage(rootUSB1))
		{
			m_isUsb1 = true;
		}
		
		CheckDialog();
						
		mListView = (ListView) findViewById(R.id.apklistview);
		m_pathtxt = ( EditText)findViewById(R.id.path);
		m_pathtxt.setText("/storage");
	
		
		m_pathtxt.setOnClickListener(new View.OnClickListener() {
	        @Override
	        public void onClick(View v) {
	        	
	        	CheckDialog();
	        }

	    });
		
		 IntentFilter intentfilter;
		intentfilter =  new IntentFilter();
		intentfilter.addAction(Intent.ACTION_PACKAGE_ADDED);
		intentfilter.addAction(Intent.ACTION_PACKAGE_REMOVED);
		intentfilter.addDataScheme("package");
		
		
		registerReceiver( m_Apkreceiver,intentfilter );
		
		IntentFilter filter = new IntentFilter();
	    filter.addAction(Intent.ACTION_MEDIA_MOUNTED);
	    filter.addAction(Intent.ACTION_MEDIA_REMOVED);
	    filter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
	    filter.addAction(Intent.ACTION_MEDIA_EJECT);
	    filter.addAction(Intent.ACTION_MEDIA_BAD_REMOVAL);
	    //ACTION_MEDIA_BAD_REMOVAL
	    filter.addDataScheme("file"); 
	    
	    registerReceiver(m_Storagereceiver,filter);
	        
	  	
		mListView.setSelector(new PaintDrawable(0xff33b5e5));
		mListView.setOnItemClickListener(this);
	
		
				
		mTccPackInfo = new ArrayList<TccPackageInfo>();
	
		mTccPackInfo.clear();
	
		// fileScan(root); 
		
		m_Pack_adapter = new PackInfoAdapter(getApplicationContext(),mTccPackInfo);
	        
		mListView.setAdapter(m_Pack_adapter);
	
	 

		m_PackMgr =getPackageManager();
		if(m_PackMgr == null)
		{
			  Log.e("HOLIC", String.format("packagemanager is null") );
		}
		
	
		
	}
	void CheckDialog()
	{

		this.showDialog(0);
	
	}
	public boolean isExternalStroage(String path)
	{
		 
		
		File f = new File(path);
		 
		if(f.exists())
		{
			 File[] fileList = null;
			 fileList = f.listFiles();
			 
					 
			if(fileList == null)
			return false;
			else
				return true;
		}
		else
		{
			return false;
		}
		
	
		
	}
	@Override
	protected void onPause() {
		// TODO Auto-generated method stub
		super.onPause();
	}

	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
	}

	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		
		
		super.onDestroy();
		unregisterReceiver( m_Apkreceiver );
		unregisterReceiver( m_Storagereceiver );
				
	}

	private void SetStorage(String path)
	{
				
		
		if(path.equals(rootMain))
		{
			mTccPackInfo.clear();
			m_pathtxt.setText(rootMain);
			fileScan(rootMain);
			
		}
		else if(path.equals(rootSD))
		{
			mTccPackInfo.clear();
			m_pathtxt.setText(rootSD);
			fileScan(rootSD);
			
		}
		else if(path.equals(rootUSB))
		{
			mTccPackInfo.clear();
			m_pathtxt.setText(rootUSB);
			fileScan(rootUSB);
			
		}
		else if(path.equals(rootUSB1))
		{
			mTccPackInfo.clear();
			m_pathtxt.setText(rootUSB1);
			fileScan(rootUSB1);
			
		}
		 
		m_Pack_adapter.notifyDataSetChanged();
		
		
		
	}
	
	@Override
	@Deprecated
	protected void onPrepareDialog(int id, Dialog dialog, Bundle args) {
		// TODO Auto-generated method stub
		 Dlglist.clear();
		
		 
		 Dlglist.add( "/storage/sdcard0");
		
				 
			if(m_isSDCard1 ==true)
			{
				Dlglist.add( "/storage/sdcard1");
		
			}
			
			if(m_isUsb ==true)
			{
				Dlglist.add( "/storage/usb0");

			}
							
			if(m_isUsb1 ==true)
			{
				Dlglist.add( "/storage/usb1");
	
			}
			final CharSequence []  StorageList = Dlglist.toArray(new CharSequence[Dlglist.size()]);
			
			 Log.e("HOLIC", String.format("On Prepare dialog_%s",Dlglist.toString()) );	
		
			 ArrayAdapter<String> adapter = 
					new ArrayAdapter<String>(this, android.R.layout.select_dialog_singlechoice, android.R.id.text1, Dlglist);
				
			adapter.notifyDataSetChanged();
					
			
		((AlertDialog) dialog).getListView().invalidate();
		super.onPrepareDialog(id, dialog, args);
	}
	@Override
		protected Dialog onCreateDialog(int id) {
		// TODO Auto-generated method stub
		
		 Dialog dialog;
		 AlertDialog.Builder builder;
				
		final ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,android.R.layout.select_dialog_singlechoice,Dlglist);
				
		
		builder = new AlertDialog.Builder(this,AlertDialog.THEME_DEVICE_DEFAULT_DARK);
	     builder.setIcon(R.drawable.apk_installer_icon);
		 builder.setTitle("Choose device to Scan ApkFile");
		
	    builder.setSingleChoiceItems(adapter, -1, new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				// TODO Auto-generated method stub
		
				AlertDialog alert = (AlertDialog)dialog;
			     ListView lv = (ListView)alert.getListView();
			     			     
			     m_iCurrentStorage = lv.getCheckedItemPosition();
			     
			     String str = Dlglist.get(m_iCurrentStorage);
			     
			     if(str.equals("/storage/sdcard0"))
			     {
			    	 SetStorage(rootMain);
			    	 Log.e("HOLIC", String.format("select to string %s",str) );	
			     }
			     else if(str.equals("/storage/sdcard1"))
			     {
			    	 SetStorage(rootSD);
			    	 Log.e("HOLIC", String.format("select to string %s",str) );
			     }
			     else if(str.equals("/storage/usb0"))
			     {
			    	 SetStorage(rootUSB);
			    	 Log.e("HOLIC", String.format("select to string %s",str) );
			     }
			     else if(str.equals("/storage/usb1"))
			     {
			    	 SetStorage(rootUSB1);
			    	 Log.e("HOLIC", String.format("select to string %s",str) );
			     }


				dialog.dismiss();
			}
		});
	
	   
	     dialog = builder.create();
		  
	     dialog.setOnShowListener(new OnShowListener() {

	            @Override
	            public void onShow(DialogInterface alert) {
	                ListView listView = ((AlertDialog)alert).getListView();
	                final ListAdapter originalAdapter = listView.getAdapter();

	                listView.setAdapter(new ListAdapter()
	                {

	                    @Override
	                    public int getCount() {
	                        return originalAdapter.getCount();
	                    }

	                    @Override
	                    public Object getItem(int id) {
	                        return originalAdapter.getItem(id);
	                    }

	                    @Override
	                    public long getItemId(int id) {
	                        return originalAdapter.getItemId(id);
	                    }

	                    @Override
	                    public int getItemViewType(int id) {
	                        return originalAdapter.getItemViewType(id);
	                    }

	                    @Override
	                    public View getView(int position, View convertView, ViewGroup parent) {
	                        View view = originalAdapter.getView(position, convertView, parent);
	                        TextView textView = (TextView)view;
	                        textView.setTextColor(Color.WHITE);
	                        textView.setTextSize(22); // FIXIT - absolute size 
	                        return view;
	                    }

	                    @Override
	                    public int getViewTypeCount() {
	                        return originalAdapter.getViewTypeCount();
	                    }

	                    @Override
	                    public boolean hasStableIds() {
	                        return originalAdapter.hasStableIds();
	                    }

	                    @Override
	                    public boolean isEmpty() {
	                        return originalAdapter.isEmpty();
	                    }

	                    @Override
	                    public void registerDataSetObserver(DataSetObserver observer) {
	                        originalAdapter.registerDataSetObserver(observer);

	                    }

	                    @Override
	                    public void unregisterDataSetObserver(DataSetObserver observer) {
	                        originalAdapter.unregisterDataSetObserver(observer);

	                    }

	                    @Override
	                    public boolean areAllItemsEnabled() {
	                        return originalAdapter.areAllItemsEnabled();
	                    }

	                    @Override
	                    public boolean isEnabled(int position) {
	                        return originalAdapter.isEnabled(position);
	                    }

	                });

	            }

	        });

	     dialog.show();
	     
	     return dialog;
	     

	}


	private boolean isInstalledApplication(PackageManager pm, String packname )
	{
		
		
		try
		{
			pm.getApplicationInfo(packname, PackageManager.GET_META_DATA);
		}
		catch(NameNotFoundException e)
		{
			return false;
		}
		return true;
		
	}
	private void fileScan(String _path) {
		  
		File f = new File(_path);
		  
		  if(f.exists())
		  {
			  File[] fileList = null;
			  fileList = f.listFiles();
					  
			  if(fileList != null)
				{
					 
					  
					  for (int i = 0; i < fileList.length; i++) 
					  {
						  String path = fileList[i].getAbsolutePath();
					  
				
						  
						  if (fileList[i].isDirectory())
						   {
							   fileScan(path);
						   } 
						  else
						  {
							  if (path.endsWith(".apk")) 
							  {
								  String APKFilePath;
								  
								  TccPackageInfo tccpackinfo = new TccPackageInfo();
								  
								  tccpackinfo.m_sApkName = fileList[i].getName();
								  tccpackinfo.m_sApkPath = fileList[i].getAbsolutePath();
										  
								  PackageInfo    pi = m_PackMgr.getPackageArchiveInfo(tccpackinfo.m_sApkPath, PackageManager.GET_ACTIVITIES);				 
								
															  
								  if(pi!= null)
								  {
								
									  
									  pi.applicationInfo.sourceDir       = tccpackinfo.m_sApkPath;
								
									  pi.applicationInfo.publicSourceDir = tccpackinfo.m_sApkPath;
							
								
								  
									  try
									  {
										  m_PackMgr.getApplicationInfo(pi.packageName, PackageManager.GET_META_DATA);
										  tccpackinfo.m_bIsInstall = false;
										  						 
										  
									//	  Log.e("HOLIC", String.format("Already Installed") );
									  }
									  catch(NameNotFoundException e)
									  {
										  tccpackinfo.m_bIsInstall = true;
						  
										  
									  }
									  
									  mTccPackInfo.add(tccpackinfo); 
								  }
								  
								
								 // tccpackinfo.m_dicon =  pi.applicationInfo.loadIcon(m_PackMgr);
								//  Drawable icon = pi.applicationInfo.loadIcon(m_PackMgr);
								  
							
							 					  
							  } //if
							  
						  }
					  
					  }
				}
				
		  
		  }
		  else
		  {
			
			  return;
		  }	  
		 		  
		
		
		
		  
		  
}
		@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
//		getMenuInflater().inflate(R.menu.tccapkinstall, menu);
		return true;
	}
	private class ListRowInfo {
        View rowView;
        ImageView icon = null;
        ImageView isinstall = null;
        TextView apkname = null;
        TextView apkpath = null;
    
        ListRowInfo(View view){
            this.rowView = view;
        }

        TextView getApkNameText() {
            if (apkname == null) {
            	apkname = (TextView)rowView.findViewById(R.id.apkname_txt);
            }
            return apkname;
        }
        TextView getApkPathText() {
            if (apkpath == null) {
            	apkpath = (TextView)rowView.findViewById(R.id.apkpath_txt);
            }
            return apkpath;
        }

        ImageView getIcon() {
            if (icon == null) {
                icon = (ImageView)rowView.findViewById(R.id.icon);
            }
            return icon;
        }
        ImageView getIsInstall() {
            if (isinstall == null) {
            	isinstall = (ImageView)rowView.findViewById(R.id.isinstall_view);
            }
            return isinstall;
        }
    }
	
		private class ExternalReceiver extends BroadcastReceiver{
		
		 @Override
         public void onReceive(Context context, Intent intent) {
             
		/*	 private static final String rootMain="/storage/sdcard0";
				private static final String rootSD="/storage/sdcard1";
				private static final String rootUSB="/storage/usb0";
				private static final String rootUSB1="/storage/usb1";
			 */
			 if(intent.getAction().equals(intent.ACTION_MEDIA_MOUNTED))
            {
            	if(intent.getData().getPath().equals("/storage/usb0"))
            	{
            		m_isUsb = true; 
    
            //			mTccPackInfo.clear();
             //   		m_Pack_adapter.notifyDataSetChanged();
            	}
            	else if(intent.getData().getPath().equals("/storage/sdcard1"))
            	{
            		m_isSDCard1 = true;
       
            //			mTccPackInfo.clear();
             //   		m_Pack_adapter.notifyDataSetChanged();
            	}
            	else if(intent.getData().getPath().equals("/storage/usb1"))
            	{
            		m_isUsb1 = true;
     
           // 			mTccPackInfo.clear();
            //    		m_Pack_adapter.notifyDataSetChanged();       	
            	}
            		
            	
            }
            else if(intent.getAction().equals(intent.ACTION_MEDIA_EJECT))
            {
            	   String str = Dlglist.get(m_iCurrentStorage); 
            	
            	   Log.e("HOLIC", String.format("ACTION_MEDIA_EJECT %s",str) );
            	   
            	if(intent.getData().getPath().equals("/storage/sdcard1"))
            	{
            		         		 
            		m_isSDCard1 = false;
            		
            		if(str.equals("/storage/sdcard1"))
            		{
            			m_pathtxt.setText("/storage");  	
            			mTccPackInfo.clear();
                       	m_Pack_adapter.notifyDataSetChanged();
                                        	 
            		}
            		          		       		
                                	          		
            	}
            else if(intent.getData().getPath().equals("/storage/usb0"))
            	{
            		m_isUsb = false; 
            		
            		if(str.equals("/storage/usb0"))
            		{
            			m_pathtxt.setText("/storage");
            			mTccPackInfo.clear();
            			m_Pack_adapter.notifyDataSetChanged();
            		}
            		
            	}
            	else if(intent.getData().getPath().equals("/storage/usb1"))
            	{
            		m_isUsb1 = false;
            		
            		if(str.equals("/storage/usb1"))
            		{
            			m_pathtxt.setText("/storage");  	
            			mTccPackInfo.clear();
                        m_Pack_adapter.notifyDataSetChanged();
                        
                     }
            		            		            	
            	}
            	
            }
            else if(intent.getAction().equals(intent.ACTION_MEDIA_BAD_REMOVAL))
            {
            
            }
			 
		     
          //   UpdateExternalStorage();.
         }
		
	}
	
	private  class ApkinstallReceiver extends BroadcastReceiver {

		@Override
		public void onReceive(Context context, Intent intent) {
			
			String packageName = intent.getData().getSchemeSpecificPart();
			String action = intent.getAction();
			
			 
			 
			 if(action ==null)
			 {
					
			 }
			 
			 
			if(action.equals(Intent.ACTION_PACKAGE_ADDED))
			{
				 
				for(int i=0; i < mTccPackInfo.size(); i++)
				{
					if(packageName.equals(mTccPackInfo.get(i).m_sPackagename))
					{
						mTccPackInfo.get(i).m_bIsInstall = false;
												
						 m_Pack_adapter.notifyDataSetChanged();
						 
						   if (DEBUG) Log.d(TAG, "Apkinstall_Receiver");
						 return;
					}
				}
		
				
			}
		
		}

	}

	public class PackInfoAdapter extends ArrayAdapter<TccPackageInfo> {
	        Context mContext;
	        ListRowInfo rowInfo = null;
	        private  ArrayList<TccPackageInfo> items;
	        private LayoutInflater mInflater;	
	        public PackInfoAdapter(Context context, ArrayList<TccPackageInfo> object)
	        {
	        	
	        	
	        	super(context, R.layout.folder_list_row,object);
	        	
	        	 
	        	 this.items = object;
	            this.mContext = context;
	            mInflater = (LayoutInflater) mContext
						.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
	         
	        }
	    @Override
	    public View getView(int position, View convertView, ViewGroup parent)
	     {
	            
	        	
	        	View row = convertView;
	            
	       	
	        	if (row == null)
	        	{
	            	row = m_inflater.inflate(R.layout.folder_list_row, null);
	            	rowInfo = new ListRowInfo(row);
	                row.setTag(rowInfo);
	            } else 
	            {
	                rowInfo = (ListRowInfo)row.getTag();
	            }
	        	final 	TccPackageInfo pacinfo = this.getItem(position);
	        	
	        	
	        	 rowInfo.getApkNameText().setText(pacinfo.m_sApkName);
	        	 rowInfo.getApkPathText().setText(pacinfo.m_sApkPath);
	        	 
	           	 
	        	 rowInfo.getIcon().setImageDrawable(mTccPackInfo.get(position).m_dicon);
	        	 
	        	 if(pacinfo.m_bIsInstall ==true) //Not installed
	        	 {
	        		 rowInfo.getIsInstall().setImageResource(R.drawable.stb_video_list_apk_02_img);
	        	 }
	        	 else
	        		 rowInfo.getIsInstall().setImageResource(R.drawable.stb_video_list_apk_01_img);
	        	 
	            return row;
	        }
	    }
	    
	   public void onItemClick(AdapterView<?> parent, View view, int position, long id) 
       {
       
			
		  if( mTccPackInfo.get(position).m_bIsInstall == true)
		  {
		
			  File apkFile = new File(mTccPackInfo.get(position).m_sApkPath);
			  
			  Uri apkUri = Uri.fromFile(apkFile);
			   Intent install_Intent = new Intent(Intent.ACTION_VIEW);
			   install_Intent.setDataAndType(Uri.fromFile(apkFile), "application/vnd.android.package-archive");
			   startActivityForResult(install_Intent,1);
			  
			   
			
		  }
		  
		 	
       }
	@Override
	public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2,
			long arg3) {
		
		// TODO Auto-generated method stub
		
	}
	@Override
	public void onNothingSelected(AdapterView<?> list) {
		
		// TODO Auto-generated method stub
		
	}
	

}
