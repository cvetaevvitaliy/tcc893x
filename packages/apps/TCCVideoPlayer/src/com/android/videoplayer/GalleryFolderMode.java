/*
 * Copyright (c) 2009, Moo Productions
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 * - Redistributions of source code must retain the above copyright notice, this list of 
 *   conditions and the following disclaimer.
 * 
 * - Redistributions in binary form must reproduce the above copyright notice, this list 
 *   of conditions and the following disclaimer in the documentation and/or other materials 
 *   provided with the distribution.
 * 
 * - Neither the name of the Moo Productions nor the names of its contributors may be used 
 *   to endorse or promote products derived from this software without specific prior written 
 *   permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS 
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER 
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package com.android.videoplayer;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Instrumentation;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.graphics.drawable.PaintDrawable;
import android.net.Uri;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.provider.MediaStore;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MenuItem.OnMenuItemClickListener;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.android.videoplayer.util.ImageManager;

public class GalleryFolderMode extends Activity
        implements AdapterView.OnItemClickListener, AdapterView.OnItemLongClickListener, AdapterView.OnItemSelectedListener {
    private static final String TAG = "GalleryFolderMode";
    private static final boolean DEBUG = false;
    private static final int INDEX_NONE = -1;
    private static final String STATE_SELECTED_INDEX = "first_index";
    private static final String STATE_SELECTED_PATH = "current_path";

    private static final String root="/storage";
    private List<String> mItem = new ArrayList<String>();
    private List<String> mPath = new ArrayList<String>();

    private TextView mCurrentPathView;

    private SharedPreferences mPrefs;

    private ListView mListView;
    private View mListLayout;

    private String mCurrentPath;

    private VideoPreviewControl mPreview;

    private int mCurrentPosition;
    private static boolean mShowApkFile = false;

    private OnClickListener mButtonClick = new OnClickListener() {
		public void onClick(View v) {
	
		switch(v.getId())
		{
			case R.id.btnVideoSet:
			{
				Log.e("HOLIC", String.format("SettingVideo") );
				
				Intent preferences = new Intent();
                 preferences.setClass(GalleryFolderMode.this, GallerySettings.class);
                 startActivity(preferences);
    
			}
			break;
		}
		
	}
		
    };
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (DEBUG) Log.d(TAG, "onCreate()");
        
        this.requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);
        setContentView(R.layout.gallery_folder_mode_main);
        this.getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE, R.layout.custom_title);
       
        mPrefs = PreferenceManager.getDefaultSharedPreferences(this);

        mCurrentPathView = (TextView)findViewById(R.id.path);
        mListLayout = findViewById(R.id.listlayout);
        mListView = (ListView) findViewById(R.id.listview);
        mListView.setOnItemClickListener(this);
        mListView.setOnItemLongClickListener(this);
        mListView.setOnItemSelectedListener(this);
        mListView.setSelector(new PaintDrawable(0xff33b5e5));
        mListView.setDivider(getResources().getDrawable(R.drawable.stb_video_list_line_img));
        
        
        
        mPreview = new VideoPreviewControl(findViewById(R.id.viewlayout), this);
        mCurrentPosition = INDEX_NONE;
        mCurrentPath = root;
        
        Button btn = (Button)this.findViewById(R.id.btnVideoSet);
        btn.setOnClickListener(mButtonClick);
    }

    @Override
    public void onStart() {
        super.onStart();
        if (DEBUG) Log.d(TAG, "onStart()");
        // install an intent filter to receive SD card related events.
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_EJECT);
        intentFilter.addDataScheme("file");
        registerReceiver(mReceiver, intentFilter);
    }

    @Override
    public void onStop() {
        super.onStop();
        if (DEBUG) Log.d(TAG, "onStop()" );
        unregisterReceiver(mReceiver);
        mPreview.stopCurrentVideo();
    }

    @Override
    public void onResume() {
        super.onResume();
        if (DEBUG) Log.d(TAG, "onResume()");
        mShowApkFile = mPrefs.getBoolean("pref_gallery_display_apk_file_key", false);
        getDir(mCurrentPath);
        mListLayout.requestFocus();
        if(mPath.size() <= mCurrentPosition) {
            mCurrentPosition = INDEX_NONE;
        }
        if(mCurrentPosition != INDEX_NONE){
            mListView.setSelection(mCurrentPosition);
            focusedPreview(mCurrentPosition);
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        if (DEBUG) Log.d(TAG, "onPause()");
        mPreview.stopCurrentVideo();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (DEBUG) Log.d(TAG, "onDestroy()");
    }

    @Override
    protected void onSaveInstanceState(Bundle state) {
        super.onSaveInstanceState(state);
        if (DEBUG) Log.d(TAG, "onSaveInstanceState(mCurrentPosition=" + mCurrentPosition + " mCurrentPath=" + mCurrentPath + ")");
        state.putInt(STATE_SELECTED_INDEX, mCurrentPosition);
        state.putString(STATE_SELECTED_PATH, mCurrentPath);
    }

    @Override
    protected void onRestoreInstanceState(Bundle state) {
        super.onRestoreInstanceState(state);
        mCurrentPosition = state.getInt(STATE_SELECTED_INDEX, 0);
        mCurrentPath = state.getString(STATE_SELECTED_PATH);
        if (DEBUG) Log.d(TAG, "onRestoreInstanceState(mCurrentPosition=" + mCurrentPosition + " mCurrentPath=" + mCurrentPath + ")");
    }

    BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (DEBUG) Log.d(TAG, "onReceive(action=" + intent.getAction() + ")");
            if (mCurrentPath.equals(root) || mCurrentPath.contains(intent.getData().getPath()))
                rebake();
        }
    };

    private void rebake() {
        mCurrentPosition = INDEX_NONE;
        mPreview.stopCurrentVideo();
        getDir(root);
        mListLayout.requestFocus();
    }

    @Override
    public void onContextMenuClosed(Menu menu) {
        super.onOptionsMenuClosed(menu);
        if (DEBUG) Log.d(TAG, "onOptionsMenuClosed()");
        if(mCurrentPosition != INDEX_NONE){
            focusedPreview(mCurrentPosition);
        }
        return;
    }

    @Override
    public boolean onCreateOptionsMenu (Menu menu) {
        super.onCreateOptionsMenu(menu);
        if (DEBUG) Log.d(TAG, "onCreateOptionsMenu()");
        menu.add(Menu.NONE, Menu.NONE, Menu.NONE, R.string.pref_video_category)
                .setOnMenuItemClickListener(new OnMenuItemClickListener() {
                    public boolean onMenuItemClick(MenuItem item) {
                        Intent preferences = new Intent();
                        preferences.setClass(GalleryFolderMode.this, GallerySettings.class);
                        startActivity(preferences);
                        return true;
                    }
                })
                .setAlphabeticShortcut('p')
                .setIcon(android.R.drawable.ic_menu_preferences);
        return true;
    }

    private ImageManager.ImageListParam getParam(int inclusion , String dirPath) {
        return ImageManager.getImageListParam(
                inclusion,
                ImageManager.SORT_DESCENDING,
                (dirPath != null ) ? dirPath : null);
    }

    private boolean checkContents(String path) {
        File f = new File(path);
        File[] files = f.listFiles();
        if (files == null) return false;
        for(int i=0; i < files.length; i++){
            File file = files[i];
            if (file.isDirectory()) {
                if (checkContents(file.getPath())) {
                    return true;
                }
            } else {
                if(ImageManager.isVideoFile(file.getPath()) || (mShowApkFile && ImageManager.isApkFile(file.getPath()))) {
                    return true;
                }
            }
        }
        return false;
    }

    private void getDir(String dirPath) {
        List<String> folderItem = new ArrayList<String>();
        List<String> fileItem = new ArrayList<String>();
        List<String> folderPath = new ArrayList<String>();
        List<String> filePath = new ArrayList<String>();

        mItem.clear();
        mPath.clear();
        mCurrentPathView.setText(dirPath);

        File f = new File(dirPath);
        File[] files = f.listFiles();

        if (files != null) {
            if (DEBUG) Log.d(TAG,"getDir  dirPath :" + dirPath + " files.length : " + files.length + " root :" + root);
            if (DEBUG) Log.d(TAG, " -------------------------------------------");
            for (int i = 0; i < files.length; i++) {
                File file = files[i];

                if(DEBUG) Log.d(TAG," file.getPath():" + file.getPath() + " file.getName() :"  + file.getName() + " Uri: " + Uri.fromFile(file));
                if (file.isDirectory()) {
                    if (file.canExecute() && checkContents(file.getPath())) {
                        if (!file.getName().equals("emulated") || !dirPath.equals(root)) {
                            folderPath.add(file.getPath());
                            folderItem.add(file.getName() + "/");
                        }
                    }
                } else {
                    if(ImageManager.isVideoFile(file.getName()) || ImageManager.isCaptionFile(file.getName()) ||
                            (mShowApkFile && ImageManager.isApkFile(file.getPath()))) {
                        filePath.add(file.getPath());
                        fileItem.add(file.getName());
                    }
                }
            }
        } else {
        	mCurrentPosition = INDEX_NONE;
        }

        // Add Default Folder
        if (!dirPath.equals(root)) {
            if (DEBUG) Log.d(TAG," f.getParent():" + f.getParent());

            folderItem.add(0,root);
            folderPath.add(0,root);
            folderItem.add(1,"../");
            folderPath.add(1,f.getParent());
        }

        if(DEBUG) Log.d(TAG,"getDir  folderItem.size() :" + folderItem.size() + " mPath.size() " + mPath.size());

        // Make all List
        mPath.addAll(folderPath);
        mPath.addAll(filePath);
        mItem.addAll(folderItem);
        mItem.addAll(fileItem);

        if (mPath.size() ==0) {
            mCurrentPathView.setText(R.string.image_gallery_NoImageView_text);
        }

        // Make and Set List Adapter
        ListArrayAdapter fileList = new ListArrayAdapter(this);
        mListView.setAdapter(fileList);
        return;
    }

    private class ListRowInfo {
        View rowView;
        ImageView icon = null;
        TextView text = null;

        ListRowInfo(View view){
            this.rowView = view;
        }

        TextView getText() {
            if (text == null) {
                text = (TextView)rowView.findViewById(R.id.rowtext);
            }
            return text;
        }

        ImageView getIcon() {
            if (icon == null) {
                icon = (ImageView)rowView.findViewById(R.id.icon);
            }
            return icon;
        }
    }

    private class ListArrayAdapter extends ArrayAdapter {
        Context mContext;
        ListRowInfo rowInfo = null;

        ListArrayAdapter(Context context){
            super(context, R.layout.folder_list_row, mItem);
            this.mContext = context;
        }

        public View getView(int position, View convertView, ViewGroup parent) {
            View row = convertView;
            if (row == null) {
                row = ((Activity)mContext).getLayoutInflater().inflate(R.layout.folder_list_row, null);
                rowInfo = new ListRowInfo(row);
                row.setTag(rowInfo);
            } else {
                rowInfo = (ListRowInfo)row.getTag();
            }

            File file = new File(mPath.get(position));
            rowInfo.getText().setText(mItem.get(position));
            if (file.isDirectory() && file.canRead()) {
                rowInfo.getIcon().setImageResource(R.drawable.files_list_directory);
            } else {
                if (ImageManager.isVideoFile(file.getName())) {
                    rowInfo.getIcon().setImageResource(R.drawable.files_list_video);
                } else if (ImageManager.isApkFile(file.getName())) {
                    rowInfo.getIcon().setImageResource(R.drawable.files_list_apk);
                } else {
                    rowInfo.getIcon().setImageResource(R.drawable.files_list_other);
                }
            }
            return row;
        }
    }

    private void focusedPreview(int position) {
        File file = new File(mPath.get(position));
        if(DEBUG) Log.d(TAG,"focusedPreview(file.getPath()=" + file.getPath() +" position=" + position + ")");

        mPreview.stopCurrentVideo();
        mPreview.displayInfo(file);
        if (!file.isDirectory()) {
            if (ImageManager.isVideoFile(file.getName()) && file.length() > 0) {
                mPreview.preprocessPreview(file);
            }
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (DEBUG) Log.d(TAG, "onKeyDown keyCode " + keyCode + " this " + this);
        switch(keyCode) {
            case KeyEvent.KEYCODE_DPAD_RIGHT:
                return true;
            case KeyEvent.KEYCODE_BACK:
                if(!mCurrentPath.equals(root)){
                	if (mPath.size() > 1) {
                    	mCurrentPath = mPath.get(1);
                    	getDir(mCurrentPath);
                    } else {
                    	mCurrentPath = root;
                    	getDir(mCurrentPath);
                    }

                    mCurrentPosition = 0;

                    mListView.setSelection(mCurrentPosition);
                    focusedPreview(mCurrentPosition);
                    return true;
                }
                break;
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        File file = new File(mPath.get(position));
        mPreview.stopCurrentVideo();
        if (file.isDirectory()) {
            if (DEBUG) Log.d(TAG, "onItemClick(id=" + id + " position=" + position+")");
            if (file.canRead()) {
                mCurrentPath = mPath.get(position);
                mCurrentPosition = position;
                getDir(mCurrentPath);
            } else {
                new AlertDialog.Builder(this)
                //.setIcon(R.drawable.icon)
                .setTitle("[" + file.getName() + "] folder can't be read!")
                .setPositiveButton("OK",
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            // TODO Auto-generated method stub
                        }
                }).show();
            }
        } else {
            if (DEBUG) Log.d(TAG,"onItemClick(mCurrentPath=" + mCurrentPath + " Uri.fromFile(file)=" + Uri.fromFile(file) + ")");
            if (ImageManager.isVideoFile(file.getName())) {
                if (file.length() == 0) {
                    new AlertDialog.Builder(this)
                    //.setIcon(R.drawable.icon)
                    .setTitle("[" + file.getName() + "] file can't be read!")
                    .setPositiveButton("OK",
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                // TODO Auto-generated method stub
                            }
                    }).show();
                    return;
                }
                ImageManager.ImageListParam param = getParam(ImageManager.INCLUDE_VIDEO, mCurrentPath);
                Intent intent = new Intent(Intent.ACTION_VIEW);
                intent.setDataAndType(Uri.fromFile(file), "video/*");
                intent.addCategory(Intent.CATEGORY_ALTERNATIVE);
                intent.putExtra(MediaStore.EXTRA_SCREEN_ORIENTATION,ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
                intent.putExtra(MovieActivity.KEY_IMAGE_LIST, param);
                mCurrentPosition = position;
                startActivity(intent);
            } else if (mShowApkFile && ImageManager.isApkFile(file.getPath())) {
                // APK Install
                Intent intent = new Intent(Intent.ACTION_VIEW);
                intent.setDataAndType(Uri.fromFile(file), "application/vnd.android.package-archive");
                startActivity(intent);
            }
        }
    }
 
    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        if (DEBUG) Log.d(TAG, "onItemSelected(id=" + id + " position=" + position+")");
        mCurrentPosition = position;
        focusedPreview(position);
    }

    @Override
    public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
        if (DEBUG) Log.d(TAG, "onItemLongClick(id=" + id + " position=" + position+")");
        return false;
    }

    @Override
    public void onNothingSelected(AdapterView<?> list) {
        if (DEBUG) Log.d(TAG, "onNothingSelected()");
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent event) {
        switch (event.getButtonState()) {
            case MotionEvent.BUTTON_SECONDARY:
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    new Thread(new Runnable() {
                        public void run() {
                            new Instrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
                        }
                    }).start();
                }
                return true;
            case MotionEvent.BUTTON_TERTIARY:
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    new Thread(new Runnable() {
                        public void run() {
                            new Instrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_MENU);
                        }
                    }).start();
                }
                return true;
        }
        return super.dispatchTouchEvent(event);
    }
/*
    @Override
    public boolean onGenericMotionEvent(MotionEvent event) {
        if ((event.getSource() & InputDevice.SOURCE_CLASS_POINTER) != 0) {
            switch (event.getAction()) {
                case MotionEvent.ACTION_SCROLL: {
                    // Handle mouse (or ext. device) by shifting the page depending on the scroll
                    final float vscroll;
                    final float hscroll;
                    if ((event.getMetaState() & KeyEvent.META_SHIFT_ON) != 0) {
                        vscroll = 0;
                        hscroll = event.getAxisValue(MotionEvent.AXIS_VSCROLL);
                    } else {
                        vscroll = -event.getAxisValue(MotionEvent.AXIS_VSCROLL);
                        hscroll = event.getAxisValue(MotionEvent.AXIS_HSCROLL);
                    }
                    if (hscroll != 0 || vscroll != 0) {
                        AudioManager audioManager = (AudioManager)getSystemService(Context.AUDIO_SERVICE);
                        if (hscroll > 0 || vscroll > 0) {
                            audioManager.adjustStreamVolume(AudioManager.STREAM_MUSIC, audioManager.ADJUST_LOWER, 1);
                        } else {
                            audioManager.adjustStreamVolume(AudioManager.STREAM_MUSIC, audioManager.ADJUST_RAISE, 1);
						}
                        return true;
                    }
                }
            }
        }
        return super.onGenericMotionEvent(event);
    }
*/
}
