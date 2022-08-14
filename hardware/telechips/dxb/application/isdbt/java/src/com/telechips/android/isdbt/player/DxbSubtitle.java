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
	
public class DxbSubtitle extends ImageView implements ISDBTPlayer.OnSubtitleUpdateListener {

	private ISDBTPlayer mPlayer;
	private Bitmap mstBitmap;
	private int giTargetDisplayWidth = 960;
	private int giTargetDisplayHeight = 540;
	private boolean subtitle_enable = false;
	private boolean subtitle_visible = true;

	public DxbSubtitle(Context context, ISDBTPlayer player) {
		super(context);

		mstBitmap = Bitmap.createBitmap(giTargetDisplayWidth, giTargetDisplayHeight, Bitmap.Config.ARGB_8888);
		setVisibility(View.VISIBLE);
		setScaleType(ImageView.ScaleType.FIT_XY);

		mPlayer = player;
		mPlayer.setOnSubtitleUpdateListener(this);		
	}

	public void enableSubtitle(boolean visible) {
		subtitle_enable = visible;
		
		if (visible == true) {
			if(subtitle_visible == true) {
				setVisibility(View.VISIBLE);
				mPlayer.playSubtitle(1);
			}
		}
		else {
			mPlayer.playSubtitle(0);
		}
	}

	public void startSubtitle(boolean visible) {	
		if (subtitle_enable == true) {				
			if (visible == true) {
				setVisibility(View.VISIBLE);
				mPlayer.playSubtitle(1);
			} else {
				setVisibility(View.INVISIBLE);
				mPlayer.playSubtitle(0);
			}
		}
		
		subtitle_visible = visible;		
	}	

	public void stopSubtitle() {
		setVisibility(View.INVISIBLE);
		mPlayer.playSubtitle(0);
		subtitle_visible = false;
	}

	public void onSubtitleUpdate(ISDBTPlayer player, SubtitleData subt) {
		//Log.d("--->", "--->subt.subtitle_update" + subt.subtitle_update + "subtitle_visible" + subtitle_visible + "subtitle_enable" + subtitle_enable);	
		if((subtitle_enable == true) && (subtitle_visible == true) && (subt.subtitle_update ==1)) {
			mPlayer.drawSubtitle(mstBitmap);
		} else {
			mstBitmap.eraseColor(0x0);
		}
			setImageBitmap(mstBitmap);
	}	
}
