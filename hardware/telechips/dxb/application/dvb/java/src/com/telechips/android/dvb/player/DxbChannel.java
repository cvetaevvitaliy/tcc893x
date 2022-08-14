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
import com.telechips.android.dvb.player.dvb.TuneSpace;
import com.telechips.android.dvb.player.dvb.RFInfoData;

import android.util.Log;
import android.os.Handler;
import android.os.Message;
import android.os.Parcel;

public class DxbChannel extends Handler {

	private static final int SET_CHANNEL   = 1;
	private static final int SET_DISH      = 2;
	private static final int SET_TUNE      = 3;
	private static final int SET_FREQUENCY = 4;

	private class Channel {
		int channel_group;
		int channel_num;
		int index;
		int audio;
		int subt;
	}
	private Channel mInfo;

	private final DxbPlayerBase mParent;
	private DVBTPlayer mPlayer;
	private int mChannelIndex;
	private TuneSpace mTuneSpace;

	public DxbChannel(DxbPlayerBase parent) {
		mParent = parent;
	}

	public void setPlayer(DVBTPlayer player) {
		synchronized(mParent) {
			mPlayer = player;
			if (mPlayer == null) {
				reset();
			}
		}
		if (mInfo != null) {
			setChannel();
		}
	}

	public void reset() {
		mTuneSpace = null;
	}

	public void setFrequency(int ch_group, int ch_num) {
		DxbLog(I, "setFrequency()");
		sendMessage(obtainMessage(SET_FREQUENCY, ch_group, ch_num, null));
	}

	public RFInfoData getRFInformation(){
		RFInfoData data = null;
		Parcel parcel = null;
		synchronized(mParent) {
			if (mPlayer != null) {
				parcel = mPlayer.getRFInformation();
			}
		}
		if (parcel != null) {
			data = new RFInfoData(parcel);
			parcel.recycle();
		}
		return data;
	}

	public void setChannel(int ch_group, int ch_num, int index, int audio, int subt) {
		mInfo = new Channel();
		mInfo.channel_group = ch_group;
		mInfo.channel_num = ch_num;
		mInfo.index = index;
		mInfo.audio = audio;
		mInfo.subt = subt;
		setChannel();
	}

	private void setChannel() {
		if (mPlayer == null) {
			DxbLog(W, "[setChannel] mPlayer is null (pending)");
			return;
		}
		mChannelIndex = mInfo.index;
		removeMessages(SET_TUNE);
		sendMessage(obtainMessage(SET_TUNE, mInfo.index, 0, mInfo));
		mInfo = null;
	}

	@Override
	public void handleMessage(Message msg) {
		switch (msg.what) {
		case SET_CHANNEL:
			if (mChannelIndex == msg.arg1) {
				final Channel info = (Channel)msg.obj;
				new Thread(new Runnable() {
					@Override
					public void run() {
						synchronized(mParent) {
							DxbLog(D, "[handleMessage] SET_CHANNEL: index = " + info.index);
							if (mPlayer != null) {
								mPlayer.setChannel(info.index, info.audio, info.subt);
							}
						}
						mParent.onSetChannelCompletion(mPlayer);
					}
				}).start();
			}
			break;
		case SET_DISH:
			if (mChannelIndex == msg.arg1) {
				final Channel info = (Channel)msg.obj;
				new Thread(new Runnable() {
					@Override
					public void run() {
						DxbLog(D, "[handleMessage] SET_DISH: index = " + info.index);
						mParent.onDishSetup(info.channel_num);
						sendMessage(obtainMessage(SET_CHANNEL, info.index, 0, info));
					}
				}).start();
			}
			break;
		case SET_TUNE:
			if (mChannelIndex == msg.arg1) {
				final Channel info = (Channel)msg.obj;
				new Thread(new Runnable() {
					@Override
					public void run() {
						synchronized(mParent) {
							DxbLog(D, "[handleMessage] SET_TUNE: index = " + info.index);
							if (mPlayer != null) {
								if (mTuneSpace == null || mTuneSpace.countryCode != info.channel_group) {
									mTuneSpace = mParent.getTuneSpace(info.channel_group);
									if (mTuneSpace != null) {
										mPlayer.setTuneSpace(mTuneSpace);
									}
								}
							}
						}
						sendMessage(obtainMessage(SET_DISH, info.index, 0, info));
					}
				}).start();
			}
			break;
		case SET_FREQUENCY:
			final int channel_group = msg.arg1;
			final int channel_number = msg.arg2;
			new Thread(new Runnable() {
				@Override
				public void run() {
					synchronized(this) {
						DxbLog(D, "[handleMessage] SET_FREQUENCY: index = " + channel_number);
						if(mPlayer == null) {
							DxbLog(E, "[setChannel] mPlayer is null");
							return;
						}
						mTuneSpace = mParent.getTuneSpace(channel_group);
						if (mTuneSpace == null || mTuneSpace.numbers <= channel_number) {
							return;
						}
						mPlayer.setTuneSpace(mTuneSpace);
						mPlayer.setFrequency(channel_number);
					}
					mParent.onDishSetup(channel_number);
				}
			}).start();
		}
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
			Log.d(TAG, mString);
			break;
		}
	}
}
