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

package com.telechips.android.dvb.player;

import com.telechips.android.dvb.player.dvb.DVBTPlayer;
import com.telechips.android.dvb.player.dvb.SubtitleData;

import android.view.View;
import android.widget.ImageView;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.database.Cursor;
import android.util.Log;

public class DxbSubtitle extends ImageView implements DVBTPlayer.OnSubtitleUpdateListener {

	private DVBTPlayer mPlayer;
	private int mSubtitleCount;
	private String[] mSubtitleLang;

	private Bitmap mBitmap;
	private Canvas mCanvas;
	private Paint mPaint;

	static int giTargetDisplayWidth = 720;
	static int giTargetDisplayHeight = 576;

	private final DxbPlayerBase mParent;

	public DxbSubtitle(Context context) {
		this(context, null);
	}

	public DxbSubtitle(Context context, DxbPlayerBase parent) {
		super(context);

		mParent = parent;

		Rect mRect = new Rect();
		mBitmap = Bitmap.createBitmap(giTargetDisplayWidth, giTargetDisplayHeight, Bitmap.Config.ARGB_8888);
		mCanvas	= new Canvas(mBitmap);
		mPaint	= new Paint(Paint.ANTI_ALIAS_FLAG);

		mRect.set(0,0,giTargetDisplayWidth-1,giTargetDisplayHeight-1);
		mCanvas.clipRect(mRect);
		mCanvas.translate(3, 3);
		mCanvas.drawColor(0x00000000);
		mCanvas.save();

		setScaleType(ImageView.ScaleType.FIT_XY);
		setVisibility(View.INVISIBLE);
	}

	public void setPlayer(DVBTPlayer player)
	{
		synchronized(mParent) {
			mPlayer = player;
			if (mPlayer == null)
				return;
			mPlayer.setOnSubtitleUpdateListener(this);
		}
	}

	public void onSubtitleUpdate(DVBTPlayer player, SubtitleData subt) {
		if(subt.region_id == -1) { //discard all drawings
			mBitmap.eraseColor(0x00000000);
		} else if(subt.region_id == -2) { //update all drawings
			setImageBitmap(mBitmap);
		} else {
			mCanvas.drawBitmap(subt.data, 0, subt.width, subt.x, subt.y, subt.width, subt.height, true, mPaint);
		}
	}

	public void startSubtitle(int i)
	{
		DxbLog(I, "startSubtitle(" + i + ")");
		synchronized(mParent) {
			if (mPlayer == null) {
				DxbLog(E, "[startSubtitle] mPlayer is null");
				return;
			}
			mPlayer.playSubtitle(1, i);
		}
	}

	public void stopSubtitle()
	{
		DxbLog(I, "stopSubtitle()");
		synchronized(mParent) {
			if (mPlayer == null) {
				DxbLog(E, "[stopSubtitle] mPlayer is null");
				return;
			}
			mPlayer.playSubtitle(0, 0);	//	0-off, 1-on
		}
	}

	public int getCount()
	{
		return mSubtitleCount;
	}

	public String getLanguageCode(int i)
	{
		if (mSubtitleLang == null || mSubtitleLang.length <= i)
			return null;
		return mSubtitleLang[i].substring(0, 3);
	}

	public void setInformation(Cursor c)
	{
		mSubtitleCount = 0;
		mSubtitleLang = null;

		if (c == null)
			return;

		mSubtitleCount = c.getCount();
		if (mSubtitleCount > 0)
		{
			mSubtitleLang = new String[mSubtitleCount];

			c.moveToFirst();
			for (int i = 0; i < mSubtitleCount; i++)
			{
				mSubtitleLang[i] = c.getString(1);
				c.moveToNext();
			}
		}
		c.close();
	}

	public void resetInformation()
	{
		mSubtitleCount = 0;
		mSubtitleLang = null;
	}

/*****************************************************************************************************************************************************************************
 *	Debug - log_message
 *****************************************************************************************************************************************************************************/
	private static final int D = 0;
	private static final int E = 1;
	private static final int I = 2;
	private static final int V = 3;
	private static final int W = 4;

	private final String TAG = "[[[" + getClass().getSimpleName() + "]]]";

	private void DxbLog(int level, String mString) {
		switch(level) {
		case E:
			Log.e(TAG, mString);
			break;
		case I:
			Log.i(TAG, mString);
			break;
		case V:
			Log.v(TAG, mString);
			break;
		case W:
			Log.w(TAG, mString);
			break;
		case D:
		default:
			//Log.d(TAG, mString);
			break;
		}
	}
}
