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

public class DxbSuperimpose extends ImageView implements ISDBTPlayer.OnSuperimposeUpdateListener {

	private ISDBTPlayer mPlayer;
	private Bitmap msiBitmap;
	private int giTargetDisplayWidth = 960;
	private int giTargetDisplayHeight = 540;
	private boolean superimpose_visible = true;
	private boolean superimpose_enable = false;
	
	public DxbSuperimpose(Context context, ISDBTPlayer player) {
		super(context);
		
		msiBitmap = Bitmap.createBitmap(giTargetDisplayWidth, giTargetDisplayHeight, Bitmap.Config.ARGB_8888);
		
		setVisibility(View.VISIBLE);
		setScaleType(ImageView.ScaleType.FIT_XY);
		
		mPlayer = player;
		mPlayer.setOnSuperimposeUpdateListener(this);
	}

	public void enableSubtitle(boolean visible) {
		superimpose_enable = visible;

		if (visible == true) {
			if(superimpose_visible == true) {
				setVisibility(View.VISIBLE);
				mPlayer.playSuperimpose(1);
			}
		}
		else {
			mPlayer.playSuperimpose(0);
		}
	}

	public void startSubtitle(boolean visible) {	
		if (superimpose_enable == true) {				
			if (visible == true) {
				setVisibility(View.VISIBLE);
				mPlayer.playSuperimpose(1);
			} else {
				setVisibility(View.INVISIBLE);
				mPlayer.playSuperimpose(0);
			}
		}
		
		superimpose_visible = visible;
	}	

	public void stopSubtitle() {
		setVisibility(View.INVISIBLE);
		mPlayer.playSuperimpose(0);
		superimpose_visible = false;
	}

	public void onSuperimposeUpdate(ISDBTPlayer player, SubtitleData superim) {
		//Log.d("--->", "superim.superimpose_update" + superim.superimpose_update + "superimpose_visible" + superimpose_visible + "superimpose_enable" + superimpose_enable);
		if((superimpose_enable == true) && (superimpose_visible == true) && (superim.superimpose_update ==1)) {
			mPlayer.drawSuperimpose(msiBitmap);
		} else {
			msiBitmap.eraseColor(0x0);
		}
			setImageBitmap(msiBitmap);		
	}
}
