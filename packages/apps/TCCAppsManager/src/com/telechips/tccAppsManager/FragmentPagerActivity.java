package com.telechips.tccAppsManager;


import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.support.v4.view.ViewPager.OnPageChangeListener;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.ImageView;
import android.view.KeyEvent;



//inheritance FragmentActivity in support package  

public class FragmentPagerActivity extends FragmentActivity {
	private final int COUNT=2;
	private ViewPager mPager;	//View pager
	public static tccAppsViewMgrActivity m_AppsViewMgr;
	public static tccAppsMainView m_AppsMainView;
	
	private final BroadcastReceiver mPackageReceiver = new PackageReceiver();
	private static PackageManager m_PackageManager;	
	private ImageView  m_Favoritebtn;
	private ImageView m_Appsbtn;
	private IntentFilter m_IntentFilter;
	public  FragmentPagerActivity()
	{
		
		
		
	}
	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
				
		super.onCreate(savedInstanceState);
	// Custom title create!!	
		this.requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);
		
		setContentView(R.layout.activity_fragment_pager);
		
		
		this.getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE, R.layout.custom_title);
		
				
		mPager = (ViewPager)findViewById(R.id.pager);
		mPager.setAdapter(new BkFragmentAdapter(getSupportFragmentManager()));
		//아이템이 변경되면, gallery나 listview의 onItemSelectedListener와 비슷
		m_Favoritebtn =  (ImageView)findViewById(R.id.img_favorite);
		m_Favoritebtn.setBackgroundResource(R.drawable.tab_favorite_f);
		m_Favoritebtn.setClickable(true);
		
		m_Appsbtn = (ImageView)findViewById(R.id.img_apps);
		m_Appsbtn.setBackgroundResource(R.drawable.tab_application_n);
		m_Appsbtn.setClickable(true);
		
		m_Favoritebtn.setOnClickListener(new View.OnClickListener() {
	        public void onClick(View v) {
	        
	        	mPager.setCurrentItem(0);
	        }
	    
		});
		
		m_Appsbtn.setOnClickListener(new View.OnClickListener() {
	        public void onClick(View v) {
	        	mPager.setCurrentItem(1);
	        }
	    
		});
		

		mPager.setOnPageChangeListener(new OnPageChangeListener()
		{	
			//pager 이동시 event
			@Override public void onPageSelected(int position) 
			{				 
			
		//		Log.e("HOLIC", String.format("setOnPageChangeListener pos:%d ",position) );
				
				switch(position)
				{
				case 0:
					m_Favoritebtn.setImageResource(R.drawable.tab_favorite_f);
			
					m_Appsbtn.setImageResource(R.drawable.tab_application_n);
					
					m_AppsMainView.SetInvalidatePos();
					break;
				case 1:
					m_Favoritebtn.setImageResource(R.drawable.tab_favorite_n);
			
					m_Appsbtn.setImageResource(R.drawable.tab_application_f);
					
					m_AppsViewMgr.SetInvalidatePos();
		
					break;
			
				}
				
				
			}
			@Override public void onPageScrolled(int position, float positionOffest, int positionOffsetPixels) {}
			@Override public void onPageScrollStateChanged(int state) {}
		});
		
		m_IntentFilter =  new IntentFilter();
		m_IntentFilter.addAction(Intent.ACTION_PACKAGE_ADDED);
		m_IntentFilter.addAction(Intent.ACTION_PACKAGE_REMOVED);
		m_IntentFilter.addAction(Intent.ACTION_PACKAGE_CHANGED);
		m_IntentFilter.addDataScheme("package");
	
        registerReceiver(mPackageReceiver, m_IntentFilter);
		m_AppsViewMgr = new tccAppsViewMgrActivity(getApplicationContext(),FragmentPagerActivity.this);
						
		m_AppsMainView = new tccAppsMainView(getApplicationContext(),FragmentPagerActivity.this);
		
		m_PackageManager = this.getApplicationContext().getPackageManager();
	
		
	}

	//FragmentPager 구현
	public class BkFragmentAdapter extends FragmentPagerAdapter{
		//생성자
		public BkFragmentAdapter(FragmentManager fm) 
		{
			
			super(fm);
		}
		/**
		 * 실제 뷰페이저에서 보여질 fragment를 반환.
		 * 일반 아답터(갤러리, 리스트뷰 등)의 getView와 같은 역할
		 * @param position - 뷰페이저에서 보여저야할 페이지 값( 0부터 )
		 * @return 보여질 fragment
		 */
		@Override public Fragment getItem(int position) 
		{
			
			return ArrayFragment.newInstance(position);
		}

		//뷰페이저에서 보여질 총 페이지 수
		@Override public int getCount() { return COUNT; }
	}

	//뷰 페이저의 페이지에 맞는 fragment를 생성하는 객체
	public static class ArrayFragment extends Fragment {
		int mPosition;	//뷰 페이저의 페이지 값
		
		public ArrayFragment()
		{
			//default constructor
		}
		
		//fragment 생성하는 static 메소드 뷰페이저의 position을 값을 받는다.
		public static ArrayFragment newInstance(int position) {
			ArrayFragment f = new ArrayFragment();	//객체 생성
			Bundle args = new Bundle();					//해당 fragment에서 사용될 정보 담을 번들 객체
			args.putInt("position", position);				//포지션 값을 저장
			f.setArguments(args);							//fragment에 정보 전달.
			
		
			return f;											//fragment 반환
		}
		
		//fragment가 만들어질 때
		@Override
		public void onCreate(Bundle savedInstanceState) 
		{
			super.onCreate(savedInstanceState);
			
			mPosition = getArguments() != null ? getArguments().getInt("position") : 0;	// 뷰페이저의 position값을  넘겨 받음
		
	
		
		}
		//fragment의 UI 생성
		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) 
		{
						
			switch(mPosition)
			{
			case 0:
				View a =  m_AppsMainView.SetLayOut(R.layout.favoriteadd, inflater, container);
				return a;	
			case 1:
				View b  =  m_AppsViewMgr.SetLayOut(R.layout.layout2, inflater, container);
				return b;
			}	
			
			return null;
			
					
		}
	}
	//Brodcasting receiver inner class
	public  class PackageReceiver extends BroadcastReceiver {

		@Override
		public void onReceive(Context context, Intent intent) {
			// TODO Auto-generated method stub
			String packageName = intent.getData().getSchemeSpecificPart();
			String action = intent.getAction();
			
			if(action ==null)
			{
			
				return;
			}
							
			
			if(action.equals(Intent.ACTION_PACKAGE_ADDED))
			{
					
				m_AppsViewMgr.AddPackageItem(packageName);
		
			}
			else if(action.equals(Intent.ACTION_PACKAGE_REMOVED))
			{
				
			
				if(packageName.equals("com.telechips.tccAppsManager"))
				{
					
				}
				else
				{					
					m_AppsMainView.DeleteReceiverItems(packageName);
					
					m_AppsViewMgr.DeleteReceiverItems(packageName);
							
							
				}
	
							
			}
		}

	}
	//end inner class Brodcasting receiver
	//
	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
        unregisterReceiver(mPackageReceiver);
    		
	}
	
	
	@Override
	protected void onPause() {
		// TODO Auto-generated method stub
		
			super.onPause();
		
	}


	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
	//	registerReceiver( mPackageReceiver,m_intentfilter );
		
		super.onResume();
		
	}


	@Override
	protected Dialog onCreateDialog(int id) {
		// TODO Auto-generated method stub
		 Dialog dialog;
		 AlertDialog.Builder builder;
					
		 switch(id) {
	     case 1:
	    	 builder = new AlertDialog.Builder(FragmentPagerActivity.this,AlertDialog.THEME_DEVICE_DEFAULT_DARK);
	    	 builder.setMessage(" It has already been registered a favorite item")
	         .setCancelable(false)
	         .setPositiveButton("OK", new DialogInterface.OnClickListener() {
	             public void onClick(DialogInterface dialog, int id) {
	                 //Do something here
	            	 dialog.dismiss();
	             }
	         });
	         
	         dialog = builder.create();
	         dialog.show();
	         break;
	     case 2:
	         builder = new AlertDialog.Builder(FragmentPagerActivity.this,AlertDialog.THEME_DEVICE_DEFAULT_DARK);
	         builder.setMessage("Do you want to delete this item?")
	         .setCancelable(false)
	         .setPositiveButton("OK", new DialogInterface.OnClickListener() {
	             public void onClick(DialogInterface dialog, int id) {
	                 //Do something here
	            	 m_AppsMainView.DeleteItems();
	             }
	         })
	         .setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
	             public void onClick(DialogInterface dialog, int id) {
	                 //Do something here
	            	 dialog.dismiss();
	             }
	         });

	         dialog = builder.create();
	         dialog.show();
	         break;
	     case 3:
	         builder = new AlertDialog.Builder(FragmentPagerActivity.this,AlertDialog.THEME_DEVICE_DEFAULT_DARK);
	         builder.setMessage("Do you want to add a favorite item?")
	         .setCancelable(false)
	         .setPositiveButton("OK", new DialogInterface.OnClickListener() {
	             public void onClick(DialogInterface dialog, int id) {
	                 //Do something here
	            //	 m_AppsMainView.DeleteItems();
	            	 m_AppsMainView.AddFavoriteItems();
	             }
	         })
	         .setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
	             public void onClick(DialogInterface dialog, int id) {
	                 //Do something here
	            	 dialog.dismiss();
	             }
	         });

	         dialog = builder.create();
	         dialog.show();
	         break;   

	     default:
	         dialog = null;
	     }
	     return dialog;

	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if(event.getAction() == KeyEvent.ACTION_DOWN )
		{
			if (keyCode == KeyEvent.KEYCODE_TV)
			{
				Intent i = new Intent();
				i.setAction("android.intent.action.MAIN");
				i.addCategory("android.intent.category.LAUNCHER");
				i.setClassName("com.telechips.android.dvb", "com.telechips.android.dvb.DVBPlayerActivity");
				i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_PREVIOUS_IS_TOP);
				startActivity(i);
				return true;
			}
			else if (keyCode == KeyEvent.KEYCODE_SEARCH)
			{
				Intent i = new Intent();
				i.setClassName("com.android.quicksearchbox", "com.android.quicksearchbox.SearchActivity");
				startActivity(i);
				return true;
			}
		}
		return super.onKeyDown(keyCode, event);
	}	
}
