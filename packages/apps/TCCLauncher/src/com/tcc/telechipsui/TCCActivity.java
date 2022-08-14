package com.tcc.telechipsui;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnFocusChangeListener;
import android.view.View.OnClickListener;
import android.view.Window;
import android.widget.Button;
import android.os.SystemProperties;

public class TCCActivity extends Activity {

    
	private FocusListenerClass FocusListener;
	private HoverListenerClass HoverListener;
	private Button m_Videobtn;
	private Button m_Musicbtn;
	private Button m_Imagebtn;
	private Button m_Favoritebtn;
	private Button m_browserbtn;
	private Button m_setbtn;
	
	private OnClickListener mButtonClick = new OnClickListener() {
		public void onClick(View v) {
		
			switch(v.getId())
			{
			case R.id.btnvideo:
				{
					Intent imovie = new Intent();
					
					
					imovie.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
					imovie.setClassName("com.android.videoplayer","com.android.videoplayer.GalleryFolderMode" );
					
					startActivity(imovie);
				}
				break;
			case R.id.btnmusic:
				{
					Intent imusic = new Intent();
					imusic.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
					imusic.setClassName("com.android.music","com.android.music.MusicBrowserActivity" );
					startActivity(imusic);
			
				}
				break;
			case R.id.btnimage:
				{
					Intent i = new Intent();
					i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
					i.setClassName("com.android.gallery", "com.android.camera.GalleryPicker");
					startActivity(i);
			
				}
				break;
			case R.id.btnapps:
				{
			/*		Intent iapps = new Intent(TCCActivity.this, activity_tccapp.class);
					startActivity(iapps);
					overridePendingTransition(R.anim.fade, R.anim.hold);
				*/
					Intent i = new Intent();
					i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
					i.setClassName("com.telechips.tccAppsManager", "com.telechips.tccAppsManager.FragmentPagerActivity");
					startActivity(i);
				}
				break;
			case R.id.btnbrowser:
				{
					Intent internet = new Intent();
					internet.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
					internet.setClassName("com.android.browser","com.android.browser.BrowserActivity");
					startActivity(internet);


				}
				break;
			case R.id.btnsetting:
				{
					Intent iset = new Intent();
					
					iset.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
					iset.setClassName("com.android.settings","com.android.settings.Settings" );
					startActivity(iset);

				}
				break;
			
			}
		
		}
		
	};
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if(event.getAction() == KeyEvent.ACTION_DOWN )
		{
			if(keyCode ==4) //cancle key
			{
				
				return false;
			}
			else if (keyCode == KeyEvent.KEYCODE_TV)
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
	public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

	if(SystemProperties.getInt("persist.sys.auto_resolution",1) != 1)
	{
		Intent i = new Intent();
		i.setClassName("outputselect.resolution","outputselect.resolution.OutputResolution");
		startActivity(i);
	}

        this.requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);
        setContentView(R.layout.activity_tcc);
        
        this.getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE, R.layout.custom_title);
        
          
        FocusListener =  new FocusListenerClass();
        
        HoverListener = new HoverListenerClass();
        
         m_Videobtn= (Button)this.findViewById(R.id.btnvideo);
    	 m_Videobtn.setOnFocusChangeListener(FocusListener);
    	 m_Videobtn.setOnHoverListener(HoverListener);
    	 m_Videobtn.setOnClickListener(mButtonClick);
    	 
    	 m_Videobtn.setHeight(296);
    	 m_Videobtn.setWidth(212);
    	 m_Videobtn.setBackgroundResource(R.drawable.video_f);
    	 
    	 m_Videobtn.requestFocus();
               
       
    	 m_Musicbtn = (Button)this.findViewById(R.id.btnmusic);
    	 m_Musicbtn.setOnFocusChangeListener(FocusListener);
    	 m_Musicbtn.setOnHoverListener(HoverListener);
    	 m_Musicbtn.setOnClickListener(mButtonClick);
        
    	 m_Imagebtn = (Button)this.findViewById(R.id.btnimage);
    	 m_Imagebtn.setOnFocusChangeListener(FocusListener);
    	 m_Imagebtn.setOnHoverListener(HoverListener);
    	 m_Imagebtn.setOnClickListener(mButtonClick);
       
    	 m_Favoritebtn = (Button)this.findViewById(R.id.btnapps);
    	 m_Favoritebtn.setOnFocusChangeListener(FocusListener);
    	 m_Favoritebtn.setOnHoverListener(HoverListener);
    	 m_Favoritebtn.setOnClickListener(mButtonClick);
       
    	 m_browserbtn = (Button)this.findViewById(R.id.btnbrowser);
    	 m_browserbtn.setOnFocusChangeListener(FocusListener);
    	 m_browserbtn.setOnHoverListener(HoverListener);
    	 m_browserbtn.setOnClickListener(mButtonClick);
        
    	 m_setbtn = (Button)this.findViewById(R.id.btnsetting);
    	 m_setbtn.setOnFocusChangeListener(FocusListener);
    	 m_setbtn.setOnHoverListener(HoverListener);
    	 m_setbtn.setOnClickListener(mButtonClick);
        
  }
       
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
   //     getMenuInflater().inflate(R.menu.activity_tcc, menu);
        return true;
    }
    class HoverListenerClass implements View.OnHoverListener{
    	
    	public boolean onHover(View v, MotionEvent event)
    	{
    		switch(v.getId())
    		{
    		case R.id.btnvideo:
    			 m_Videobtn.setBackgroundResource(R.drawable.video_f);
    			 m_Musicbtn.setBackgroundResource(R.drawable.music_n);
    			 m_Imagebtn.setBackgroundResource(R.drawable.image_n);
    			 m_Favoritebtn.setBackgroundResource(R.drawable.favorite_n);
    			 m_browserbtn.setBackgroundResource(R.drawable.browser_n);
    			 m_setbtn.setBackgroundResource(R.drawable.setting_n);
    	    			
    			m_Videobtn.setHeight(296); m_Videobtn.setWidth(212);
    			 m_Musicbtn.setHeight(250);m_Musicbtn.setWidth(172);
    			 m_Imagebtn.setHeight(250);m_Imagebtn.setWidth(172);
    			 m_Favoritebtn.setHeight(250);m_Favoritebtn.setWidth(172);
    			 m_browserbtn.setHeight(250);m_browserbtn.setWidth(172);
    			 m_setbtn.setHeight(250);m_setbtn.setWidth(172);
    			 
    			break;
    		case R.id.btnmusic:
    			 m_Videobtn.setBackgroundResource(R.drawable.video_n);
    			 m_Musicbtn.setBackgroundResource(R.drawable.music_f);
    			 m_Imagebtn.setBackgroundResource(R.drawable.image_n);
    			 m_Favoritebtn.setBackgroundResource(R.drawable.favorite_n);
    			 m_browserbtn.setBackgroundResource(R.drawable.browser_n);
    			 m_setbtn.setBackgroundResource(R.drawable.setting_n);
    	
    			 m_Videobtn.setHeight(250); m_Videobtn.setWidth(172);
    			 m_Musicbtn.setHeight(296);m_Musicbtn.setWidth(212);
    			 m_Imagebtn.setHeight(250);m_Imagebtn.setWidth(172);
    			 m_Favoritebtn.setHeight(250);m_Favoritebtn.setWidth(172);
    			 m_browserbtn.setHeight(250);m_browserbtn.setWidth(172);
    			 m_setbtn.setHeight(250);m_setbtn.setWidth(172);
    			 
    	
    			 break;
    		case R.id.btnimage:
    			 m_Videobtn.setBackgroundResource(R.drawable.video_n);
    			 m_Musicbtn.setBackgroundResource(R.drawable.music_n);
    			 m_Imagebtn.setBackgroundResource(R.drawable.image_f);
    			 m_Favoritebtn.setBackgroundResource(R.drawable.favorite_n);
    			 m_browserbtn.setBackgroundResource(R.drawable.browser_n);
    			 m_setbtn.setBackgroundResource(R.drawable.setting_n);
       			
    			m_Videobtn.setHeight(250); m_Videobtn.setWidth(172);
    			 m_Musicbtn.setHeight(250);m_Musicbtn.setWidth(172);
    			 m_Imagebtn.setHeight(296);m_Imagebtn.setWidth(212);
    			 m_Favoritebtn.setHeight(250);m_Favoritebtn.setWidth(172);
    			 m_browserbtn.setHeight(250);m_browserbtn.setWidth(172);
    			 m_setbtn.setHeight(250);m_setbtn.setWidth(172);
    			 
    				 
    			 break;
    		case R.id.btnapps:
    			 m_Videobtn.setBackgroundResource(R.drawable.video_n);
    			 m_Musicbtn.setBackgroundResource(R.drawable.music_n);
    			 m_Imagebtn.setBackgroundResource(R.drawable.image_n);
    			 m_Favoritebtn.setBackgroundResource(R.drawable.favorite_f);
    			 m_browserbtn.setBackgroundResource(R.drawable.browser_n);
    			 m_setbtn.setBackgroundResource(R.drawable.setting_n);
    
       			m_Videobtn.setHeight(250); m_Videobtn.setWidth(172);
    			 m_Musicbtn.setHeight(250);m_Musicbtn.setWidth(172);
    			 m_Imagebtn.setHeight(250);m_Imagebtn.setWidth(172);
    			 m_Favoritebtn.setHeight(296);m_Favoritebtn.setWidth(212);
    			 m_browserbtn.setHeight(250);m_browserbtn.setWidth(172);
    			 m_setbtn.setHeight(250);m_setbtn.setWidth(172);
    			 
    				break;
    		case R.id.btnbrowser:
    			 m_Videobtn.setBackgroundResource(R.drawable.video_n);
    			 m_Musicbtn.setBackgroundResource(R.drawable.music_n);
    			 m_Imagebtn.setBackgroundResource(R.drawable.image_n);
    			 m_Favoritebtn.setBackgroundResource(R.drawable.favorite_n);
    			 m_browserbtn.setBackgroundResource(R.drawable.browser_f);
    			 m_setbtn.setBackgroundResource(R.drawable.setting_n);
       			
    			m_Videobtn.setHeight(250); m_Videobtn.setWidth(172);
    			 m_Musicbtn.setHeight(250);m_Musicbtn.setWidth(172);
    			 m_Imagebtn.setHeight(250);m_Imagebtn.setWidth(172);
    			 m_Favoritebtn.setHeight(250);m_Favoritebtn.setWidth(172);
    			 m_browserbtn.setHeight(296);m_browserbtn.setWidth(212);
    			 m_setbtn.setHeight(250);m_setbtn.setWidth(172);
        
    		break;
    		case R.id.btnsetting:
    			 m_Videobtn.setBackgroundResource(R.drawable.video_n);
    			 m_Musicbtn.setBackgroundResource(R.drawable.music_n);
    			 m_Imagebtn.setBackgroundResource(R.drawable.image_n);
    			 m_Favoritebtn.setBackgroundResource(R.drawable.favorite_n);
    			 m_browserbtn.setBackgroundResource(R.drawable.browser_n);
    			 m_setbtn.setBackgroundResource(R.drawable.setting_f);
       			
    			m_Videobtn.setHeight(250); m_Videobtn.setWidth(172);
    			 m_Musicbtn.setHeight(250);m_Musicbtn.setWidth(172);
    			 m_Imagebtn.setHeight(250);m_Imagebtn.setWidth(172);
    			 m_Favoritebtn.setHeight(250);m_Favoritebtn.setWidth(172);
    			 m_browserbtn.setHeight(250);m_browserbtn.setWidth(172);
    			 m_setbtn.setHeight(296);m_setbtn.setWidth(212);
    			 
    		
    		break;
    			
    		}
    		
    		
    		return true;
    	}
    	
    }
    class FocusListenerClass implements View.OnFocusChangeListener{
    	
    	public void onFocusChange(View v, boolean hasFocus) {
			// TODO Auto-generated method stub
      		
    		switch(v.getId())
    		{
    		case R.id.btnvideo:
    			{
    			    			
    				if(hasFocus ==true)
    				{
    				
    				 m_Videobtn.setBackgroundResource(R.drawable.video_f);
    				 m_Musicbtn.setBackgroundResource(R.drawable.music_n);
    	    		 m_Imagebtn.setBackgroundResource(R.drawable.image_n);
    	    		 m_Favoritebtn.setBackgroundResource(R.drawable.favorite_n);
    	    		 m_browserbtn.setBackgroundResource(R.drawable.browser_n);
    	    		 m_setbtn.setBackgroundResource(R.drawable.setting_n);
    	    	      	    			    	    			
    	    		m_Videobtn.setHeight(296); m_Videobtn.setWidth(212);	 
    				 m_Musicbtn.setHeight(250);m_Musicbtn.setWidth(172);
    		    	 m_Imagebtn.setHeight(250);m_Imagebtn.setWidth(172);
    		    	 m_Favoritebtn.setHeight(250);m_Favoritebtn.setWidth(172);
    		    	 m_browserbtn.setHeight(250);m_browserbtn.setWidth(172);
    		    	 m_setbtn.setHeight(250);m_setbtn.setWidth(172);
    		    	
    				}
    					
    			}	
    			break;
    		case R.id.btnmusic:
    			{
    				if(hasFocus ==true)
    				{
    				
    					 m_Videobtn.setBackgroundResource(R.drawable.video_n);
    	    			 m_Musicbtn.setBackgroundResource(R.drawable.music_f);
    	    			 m_Imagebtn.setBackgroundResource(R.drawable.image_n);
    	    			 m_Favoritebtn.setBackgroundResource(R.drawable.favorite_n);
    	    			 m_browserbtn.setBackgroundResource(R.drawable.browser_n);
    	    			 m_setbtn.setBackgroundResource(R.drawable.setting_n);
    	    	    			
    	    			m_Videobtn.setHeight(250); m_Videobtn.setWidth(172);	 
    	    			 m_Musicbtn.setHeight(296);m_Musicbtn.setWidth(212);
    	    		     m_Imagebtn.setHeight(250);m_Imagebtn.setWidth(172);
    	    		     m_Favoritebtn.setHeight(250);m_Favoritebtn.setWidth(172);
    	    		     m_browserbtn.setHeight(250);m_browserbtn.setWidth(172);
    	    		     m_setbtn.setHeight(250);m_setbtn.setWidth(172);
    	    		    	
    	    	
    				
    				}
    			}
    			break;
    		case R.id.btnimage:
    			{
    				if(hasFocus ==true)
    				{
    					
    					
    					 m_Videobtn.setBackgroundResource(R.drawable.video_n);
    	    			 m_Musicbtn.setBackgroundResource(R.drawable.music_n);
    	    			 m_Imagebtn.setBackgroundResource(R.drawable.image_f);
    	    			 m_Favoritebtn.setBackgroundResource(R.drawable.favorite_n);
    	    			 m_browserbtn.setBackgroundResource(R.drawable.browser_n);
    	    			 m_setbtn.setBackgroundResource(R.drawable.setting_n);
    	    	    			
    	    			m_Videobtn.setHeight(250); m_Videobtn.setWidth(172);	 
    	    			 m_Musicbtn.setHeight(250);m_Musicbtn.setWidth(172);
    	    		     m_Imagebtn.setHeight(296);m_Imagebtn.setWidth(212);
    	    		     m_Favoritebtn.setHeight(250);m_Favoritebtn.setWidth(172);
    	    		     m_browserbtn.setHeight(250);m_browserbtn.setWidth(172);
    	    		     m_setbtn.setHeight(250);m_setbtn.setWidth(172);
    	    	
   	    
    				
    				}
    			}
    			break;
    		case R.id.btnapps:
    			{
    				if(hasFocus ==true)
    				{
    					 m_Videobtn.setBackgroundResource(R.drawable.video_n);
    	    			 m_Musicbtn.setBackgroundResource(R.drawable.music_n);
    	    			 m_Imagebtn.setBackgroundResource(R.drawable.image_n);
    	    			 m_Favoritebtn.setBackgroundResource(R.drawable.favorite_f);
    	    			 m_browserbtn.setBackgroundResource(R.drawable.browser_n);
    	    			 m_setbtn.setBackgroundResource(R.drawable.setting_n);
    	    	    			
    	    			m_Videobtn.setHeight(250); m_Videobtn.setWidth(172);	 
    	    			 m_Musicbtn.setHeight(250);m_Musicbtn.setWidth(172);
    	    		     m_Imagebtn.setHeight(250);m_Imagebtn.setWidth(172);
    	    		     m_Favoritebtn.setHeight(296);m_Favoritebtn.setWidth(212);
    	    		     m_browserbtn.setHeight(250);m_browserbtn.setWidth(172);
    	    		     m_setbtn.setHeight(250);m_setbtn.setWidth(172);
    	
    				}
    			}
    			break;
    		case R.id.btnbrowser:
    			{
    				if(hasFocus ==true)
    				{
    					 m_Videobtn.setBackgroundResource(R.drawable.video_n);
    	    			 m_Musicbtn.setBackgroundResource(R.drawable.music_n);
    	    			 m_Imagebtn.setBackgroundResource(R.drawable.image_n);
    	    			 m_Favoritebtn.setBackgroundResource(R.drawable.favorite_n);
    	    			 m_browserbtn.setBackgroundResource(R.drawable.browser_f);
    	    			 m_setbtn.setBackgroundResource(R.drawable.setting_n);
    	    	    			
    	    			m_Videobtn.setHeight(250); m_Videobtn.setWidth(172);	 
    	    			 m_Musicbtn.setHeight(250);m_Musicbtn.setWidth(172);
    	    		     m_Imagebtn.setHeight(250);m_Imagebtn.setWidth(172);
    	    		     m_Favoritebtn.setHeight(250);m_Favoritebtn.setWidth(172);
    	    		     m_browserbtn.setHeight(296);m_browserbtn.setWidth(212);
    	    		     m_setbtn.setHeight(250);m_setbtn.setWidth(172);
    	
    				}
    			}
    			break;
    		case R.id.btnsetting:
    			{
    				if(hasFocus ==true)
    				{
    					
    					 m_Videobtn.setBackgroundResource(R.drawable.video_n);
    	    			 m_Musicbtn.setBackgroundResource(R.drawable.music_n);
    	    			 m_Imagebtn.setBackgroundResource(R.drawable.image_n);
    	    			 m_Favoritebtn.setBackgroundResource(R.drawable.favorite_n);
    	    			 m_browserbtn.setBackgroundResource(R.drawable.browser_n);
    	    			 m_setbtn.setBackgroundResource(R.drawable.setting_f);
    	    	    			
    	    			m_Videobtn.setHeight(250); m_Videobtn.setWidth(172);	 
    	    			 m_Musicbtn.setHeight(250);m_Musicbtn.setWidth(172);
    	    		     m_Imagebtn.setHeight(250);m_Imagebtn.setWidth(172);
    	    		     m_Favoritebtn.setHeight(250);m_Favoritebtn.setWidth(172);
    	    		     m_browserbtn.setHeight(250);m_browserbtn.setWidth(172);
    	    		     m_setbtn.setHeight(296);m_setbtn.setWidth(212);
    				}
    			}
    			break;
    		}
    		
    		
		}
    }

    
}
