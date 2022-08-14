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

package com.telechips.android.dvb;

import com.telechips.android.dvb.util.DxbUtil;
import com.telechips.android.dvb.util.DxbStorage;

import android.content.Context;
import android.view.KeyEvent;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.os.StatFs;

public class DxbView_Full_Information extends DxbView {

	private View mView;

	private TextView[] mtvTitleView;
	private TextView[] mtvItemView;

	private final int LIST_COUNT = 10;
	private int mListCount;

	public DxbView_Full_Information(Context context, View v) {
		super(context);
		mView = v;

		setComponent();
	}

	public void setComponent() {
		LinearLayout titleList = (LinearLayout)mView.findViewById(R.id.title_list);
		LinearLayout itemList = (LinearLayout)mView.findViewById(R.id.item_list);
		LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);

		mtvTitleView = new TextView[LIST_COUNT];
		mtvItemView = new TextView[LIST_COUNT];
		for(int i = 0; i < LIST_COUNT; i++) {
			mtvTitleView[i] = new TextView(getContext());
			mtvTitleView[i].setTextSize(18 / getDisplayDensity());
			titleList.addView(mtvTitleView[i], lp);
			mtvItemView[i] = new TextView(getContext());
			mtvItemView[i].setTextSize(18 / getDisplayDensity());
			itemList.addView(mtvItemView[i], lp);
		}
		setVisible(0);
	}

	@Override
	public void setVisible(boolean v) {
		if (v == false) {
			setVisible(0);
		}
		mView.setVisibility(v ? View.VISIBLE : View.GONE);
	}

	public boolean isShown() {
		return mView.isShown();
	}

	private void setVisible(int num) {
		int i = 0;
		if (mListCount == num) {
			return;
		}
		while(i < LIST_COUNT) {
			if (i < num) {
				mtvTitleView[i].setVisibility(View.VISIBLE);
				mtvItemView[i].setVisibility(View.VISIBLE);
			} else {
				mtvTitleView[i].setVisibility(View.GONE);
				mtvItemView[i].setVisibility(View.GONE);
			}
			i++;
		}
		mListCount = num;
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		if (keyCode == KeyEvent.KEYCODE_BACK || keyCode == KeyEvent.KEYCODE_ESCAPE) {
			setVisible(false);
			return true;
		}
		return false;
	}

	public void UpdateInfoForRecording(int curTime, int durTime, String sFileName) {
		StatFs mStatFs = new StatFs(DxbStorage.getPath_Device());

		int i = 0, BytePerSec = 0;
		int blockSize = mStatFs.getBlockSize() / 1024;
		int freeSize = mStatFs.getAvailableBlocks() * blockSize;
		int totalSize = mStatFs.getBlockCount() * blockSize;
		int curRecSize = getPlayer().getCurrentWritePosition();
		int maxRecSize = curRecSize + freeSize;

		mtvTitleView[i].setText("Disk Total Space");
		mtvItemView[i++].setText(DxbUtil.formatFileSize(totalSize));

		mtvTitleView[i].setText("Disk Free Space");
		mtvItemView[i++].setText(DxbUtil.formatFileSize(freeSize));

		mtvTitleView[i].setText("File System");
		mtvItemView[i++].setText(getPlayer().getDiskFSType());

		mtvTitleView[i].setText("Cluster Size");
		mtvItemView[i++].setText(DxbUtil.formatFileSize(blockSize));

		mtvTitleView[i].setText("File Name");
		mtvItemView[i++].setText(sFileName);

		mtvTitleView[i].setText("Max Record Size");
		mtvItemView[i++].setText(DxbUtil.formatFileSize(maxRecSize));

		mtvTitleView[i].setText("Max Record Time");
		if (durTime > 0) {
			BytePerSec = curRecSize/durTime;
			if (BytePerSec > 0) {
				mtvItemView[i++].setText(DxbUtil.stringForTime(maxRecSize/BytePerSec*1000));
			} else {
				mtvItemView[i++].setText("--");
			}
		} else {
			mtvItemView[i++].setText("--");
		}

		mtvTitleView[i].setText("Record Bitrate");
		if (durTime > 0) {
			mtvItemView[i++].setText(DxbUtil.formatFileSize(BytePerSec)+"/S");
		} else {
			mtvItemView[i++].setText("--");
		}

		mtvTitleView[i].setText("Cur Play Size");
		mtvItemView[i++].setText(DxbUtil.formatFileSize(getPlayer().getCurrentReadPosition()));

		mtvTitleView[i].setText("Cur Record Size");
		mtvItemView[i++].setText(DxbUtil.formatFileSize(curRecSize));

		setVisible(i);
	}

	public void UpdateInfoForPlay(int curTime, int durTime) {
		int i = 0;

		mtvTitleView[i].setText("Time");
		mtvItemView[i++].setText(DxbUtil.stringForTime(curTime*1000) + " / " + DxbUtil.stringForTime(durTime*1000));

		mtvTitleView[i].setText("File Name");
		mtvItemView[i++].setText(getPlayer().getServiceName_addHyphen());

		mtvTitleView[i].setText("File Size");
		mtvItemView[i++].setText(DxbUtil.formatFileSize(getPlayer().getCurrentWritePosition()));

		mtvTitleView[i].setText("Resolution");
		mtvItemView[i++].setText(getPlayer().getVideoInfo());

		mtvTitleView[i].setText("Frame Rate");
		mtvItemView[i++].setText(String.valueOf(getPlayer().getFrameRate()) + " Hz");

		setVisible(i);
	}

}
