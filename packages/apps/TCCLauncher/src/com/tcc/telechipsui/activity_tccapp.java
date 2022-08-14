package com.tcc.telechipsui;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.GridView;
import android.widget.Toast;

public class activity_tccapp extends Activity {

	/** Called when the activity is first created. */
//	private GridView m_gridViewImages;
//	public tccAppsadapter m_gridAdapter;
	private Context m_context;
	private static final boolean DEBUG = true;
	private static final String TAG = "tccStbLauncher";
		
	@Override
	public void onCreate(Bundle savedInstanceState) {
	    super.onCreate(savedInstanceState);
	
	    this.requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);
	    setContentView(R.layout.activity_tccapps);
	    
	    this.getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE, R.layout.home_title);
	    // TODO Auto-generated method stub
//	    m_gridViewImages = (GridView)findViewById(R.id.gridiconview);
//	    m_gridViewImages.setSelector(R.drawable.select);
	    m_context =  this.getApplicationContext();
	    
//	    SetAdapter();
		
	}
	

}
