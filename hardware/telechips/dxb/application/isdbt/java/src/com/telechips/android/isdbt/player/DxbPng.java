/*
 * Copyright (C) 2013 Telechips, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.telechips.android.isdbt.player;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.graphics.*;

import com.telechips.android.isdbt.player.isdbt.ISDBTPlayer;
import com.telechips.android.isdbt.player.isdbt.SubtitleData;

public class DxbPng extends ImageView implements ISDBTPlayer.OnPngUpdateListener {

	private ISDBTPlayer mPlayer;
	private Bitmap mpngBitmap;
	private int giTargetDisplayWidth = 960;
	private int giTargetDisplayHeight = 540;
	private boolean png_visible = true;
	private boolean png_enable = false;
	
	public DxbPng(Context context, ISDBTPlayer player) {
		super(context);
		
		mpngBitmap = Bitmap.createBitmap(giTargetDisplayWidth, giTargetDisplayHeight, Bitmap.Config.ARGB_8888);
		
		setVisibility(View.VISIBLE);
		setScaleType(ImageView.ScaleType.FIT_XY);
		
		mPlayer = player;
		mPlayer.setOnPngUpdateListener(this);
	}

	public void enableSubtitle(boolean visible) {
		png_enable = visible;

		if (visible == true) {
			if(png_visible == true) {
				setVisibility(View.VISIBLE);
				mPlayer.playPng(1);
			}
		}
		else {
			mPlayer.playPng(0);
		}
	}

	public void startSubtitle(boolean visible) {	
		if (png_enable == true) {				
			if (visible == true) {
				setVisibility(View.VISIBLE);
				mPlayer.playPng(1);
			} else {
				setVisibility(View.INVISIBLE);
				mPlayer.playPng(0);
			}
		}
		
		png_visible = visible;
	}	

	public void stopSubtitle() {
		setVisibility(View.INVISIBLE);
		mPlayer.playPng(0);
		png_visible = false;
	}

	public void onPngUpdate(ISDBTPlayer player, SubtitleData png) {
		//Log.d("--->", "png.png_update" + png.png_update + "png_visible" + png_visible + "png_enable" + png_enable);
		if((png_enable == true) && (png_visible == true) && (png.png_update ==1)) {
			mPlayer.drawPng(mpngBitmap);
		} else {
			mpngBitmap.eraseColor(0x0);
		}
			setImageBitmap(mpngBitmap);		
	}
}
