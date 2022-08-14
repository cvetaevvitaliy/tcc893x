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
import com.telechips.android.dvb.player.dvb.BlindScanInfo;

import android.util.Log;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.os.Parcel;

public class DxbScan extends Handler implements DVBTPlayer.OnSearchPercentListener,
		DVBTPlayer.OnSearchCompletionListener
{
	private static final int NONE                = 0;
	private static final int ONE_CHANNEL_SCAN    = 1;
	private static final int MULTI_CHANNEL_SCAN  = 2;
	private static final int BLIND_SCAN          = 3;

	private static final int SET_START           = 0;
	private static final int SET_DISH            = 1;
	private static final int SET_RESET           = 2;
	private static final int SET_SEARCH          = 3;
	private static final int NOTIFY_PERCENT      = 4;
	private static final int NOTIFY_COMPLETION   = 5;

	private final DxbPlayerBase mParent;
	private DVBTPlayer mPlayer;
	private TuneSpace mTuneSpace;
	private BlindScanInfo mBlindScanInfo;
	private boolean mRequestCancel = false;

	private int mState = NONE;
	private int[] mIndexList;
	private int mChannelIndex;
	private int mFrequency;
	private int mBandwidth;

	public DxbScan(DxbPlayerBase parent) {
		mParent = parent;
	}

	public void setPlayer(DVBTPlayer player) {
		synchronized(mParent) {
			mPlayer = player;
			if (mPlayer != null) {
				mPlayer.setOnSearchPercentListener(this);
				mPlayer.setOnSearchCompletionListener(this);
			}
		}
		ExecuteCMD();
	}

	public boolean OneChannelScan(int KHz_frequency, int Mhz_bandwidth) {
		if (mState != NONE) {
			DxbLog(E, "[OneChannelScan] fail, mState is not NONE");
			mRequestCancel = true;
			onSearchCompletion(mPlayer);
			return false;
		}

		mTuneSpace = null;
		mFrequency = KHz_frequency;
		mBandwidth = Mhz_bandwidth;

		mRequestCancel = false;
		mState = ONE_CHANNEL_SCAN;
		mIndexList = null;
		mChannelIndex = 0;

		ExecuteCMD();
		return true;
	}

	public boolean MultiChannelScan(TuneSpace tuneSpace, int[] index_list)
	{
		if (mState != NONE)
		{
			DxbLog(E, "[MultiChannelScan] fail, mState is not NONE");
			mRequestCancel = true;
			onSearchCompletion(mPlayer);
			return false;
		}

		mTuneSpace = tuneSpace;
		if (mTuneSpace == null || mTuneSpace.numbers == 0)
		{
			DxbLog(E, "[MultiChannelScan] fail, mTuneSpace is null or empty");
			mRequestCancel = true;
			onSearchCompletion(mPlayer);
			return false;
		}

		mRequestCancel = false;
		mState = MULTI_CHANNEL_SCAN;
		mIndexList = index_list;
		mChannelIndex = 0;

		ExecuteCMD();
		return true;
	}

	public boolean AllChannelScan(TuneSpace tuneSpace)
	{
		int	iCount_fec_inner	= 0;
		int[] index_list = new int[tuneSpace.numbers];
		for (int i = 0; i < tuneSpace.numbers; i++)
		{
			index_list[i] = i;
			if(tuneSpace.channels[i].fec_inner == 1)
			{
				iCount_fec_inner++;
			}
		}
		
		if(mParent.isDVB_S2())
		{
			if( (iCount_fec_inner!=0) && (tuneSpace.numbers!=iCount_fec_inner) )
			{
				int	iIndex	= 0;
				int[] fec_inner_list = new int[iCount_fec_inner];
				for (int i = 0; i < tuneSpace.numbers; i++)
				{
					if(tuneSpace.channels[i].fec_inner == 1)
					{
						fec_inner_list[iIndex++] = i;
					}
				}
				
				return MultiChannelScan(tuneSpace, fec_inner_list);
			}
		}
		
		return MultiChannelScan(tuneSpace, index_list);
	}

	public boolean BlindScan(TuneSpace tuneSpace) {
		if (mState != NONE) {
			DxbLog(E, "[BlindScan] fail, mState is not NONE");
			mRequestCancel = true;
			onBlindScanCompletion(mPlayer);
			return false;
		}

		mTuneSpace = tuneSpace;
		if (mTuneSpace == null || mTuneSpace.numbers == 0) {
			DxbLog(E, "[BlindScan] fail, mTuneSpace is null or empty");
			mRequestCancel = true;
			onBlindScanCompletion(mPlayer);
			return false;
		}

		mRequestCancel = false;
		mState = BLIND_SCAN;
		mIndexList = null;
		mChannelIndex = 0;

		ExecuteCMD();
		return true;
	}

	public void Cancel() {
		mRequestCancel = true;
	}

	public int getSearchFreqKHz() {
		switch (mState) {
		case ONE_CHANNEL_SCAN:
			return mFrequency;
		case MULTI_CHANNEL_SCAN:
			if (mTuneSpace != null && mTuneSpace.numbers > 0) {
				int index;
				if (mIndexList != null) {
					int i = (mChannelIndex < mIndexList.length) ? mChannelIndex : mIndexList.length - 1;
					index = (mIndexList[i] < mTuneSpace.numbers) ? mIndexList[i] : mTuneSpace.numbers - 1;
				} else {
					index = (mChannelIndex < mTuneSpace.numbers) ? mChannelIndex : mTuneSpace.numbers - 1;
				}

				if (mParent.isDVB_S2())
					return mTuneSpace.channels[index].frequency2;
				else
					return mTuneSpace.channels[index].frequency1;
			}
			break;
		}
		return 0;
	}

	private void ExecuteCMD() {
		if (mPlayer == null) {
			DxbLog(W, "[ExecuteCMD] mPlayer is null (pending cmd: " + mState + ")");
			return;
		}
		this.sendMessage(this.obtainMessage(mState, SET_START, 0, null));
	}

	@Override
	public void handleMessage(Message msg)
	{
		if (mState != msg.what)
		{
			DxbLog(E, "[handleMessage] mState is NONE");
			return;
		}
		
		switch (msg.what)
		{
			case ONE_CHANNEL_SCAN:
				if (mRequestCancel)
				{
					onSearchCompletion(mPlayer);
				}
				else
				{
					ExecuteOneChannelScan(msg.arg1);
				}
			break;
			
			case MULTI_CHANNEL_SCAN:
				if (mRequestCancel)
				{
					onSearchCompletion(mPlayer);
				}
				else
				{
					ExecuteMultiChannelScan(msg.arg1);
				}
			break;
			
			case BLIND_SCAN:
				if (mRequestCancel)
				{
					onBlindScanCompletion(mPlayer);
				}
				else
				{
					ExecuteBlindScan(msg.arg1);
				}
			break;
		}
	}

	private void ExecuteOneChannelScan(int index) {
		new Thread(new Runnable() {
				@Override
				public void run() {
					synchronized(mParent) {
						if (mPlayer != null) {
							mPlayer.manualSearch(mFrequency, mBandwidth);
						} else {
							DxbLog(E, "[ExecuteOneChannelScan] mPlayer is null");
							mRequestCancel = true;
							sendMessage(obtainMessage(ONE_CHANNEL_SCAN, 0, 0, null));
						}
					}
				}
			}).start();
	}

	private void ExecuteMultiChannelScan(int index) {
		switch (index) {
		case SET_START:
			new Thread(new Runnable() {
					@Override
					public void run() {
						synchronized(mParent) {
							if (mPlayer != null) {
								mPlayer.setTuneSpace(mTuneSpace);
							} else {
								DxbLog(E, "[ExecuteMultiChannelScan] mPlayer is null");
								mRequestCancel = true;
							}
						}
						sendMessage(obtainMessage(MULTI_CHANNEL_SCAN, SET_DISH, 0, null));
					}
				}).start();
			break;
		case SET_DISH:
			if (mParent.isDVB_S2()) {
				new Thread(new Runnable() {
						@Override
						public void run() {
							mParent.onDishSetup(mIndexList[mChannelIndex]);
							sendMessage(obtainMessage(MULTI_CHANNEL_SCAN, SET_SEARCH, 0, null));
						}
					}).start();
				break;
			}
		case SET_SEARCH:
			new Thread(new Runnable() {
					@Override
					public void run() {
						synchronized(mParent) {
							if (mPlayer != null) {
								mPlayer.manualSearch(mIndexList[mChannelIndex], 0);
							} else {
								DxbLog(E, "[ExecuteMultiChannelScan] mPlayer is null");
								mRequestCancel = true;
								sendMessage(obtainMessage(MULTI_CHANNEL_SCAN, 0, 0, null));
							}
						}
					}
				}).start();
			break;
		}
	}

	private void ExecuteBlindScan(int index) {
		switch (index) {
		case SET_START:
		case SET_DISH:
			new Thread(new Runnable() {
					@Override
					public void run() {
						mParent.onDishSetup(mChannelIndex);
						sendMessage(obtainMessage(BLIND_SCAN, SET_RESET, 0, null));
					}
				}).start();
			break;
		case SET_RESET:
			new Thread(new Runnable() {
					@Override
					public void run() {
						synchronized(mParent) {
							if (mPlayer != null) {
								mPlayer.BlindScanReset();
							} else {
								DxbLog(E, "[ExecuteBlindScan] mPlayer is null");
								mRequestCancel = true;
							}
						}
						sendMessage(obtainMessage(BLIND_SCAN, SET_SEARCH, 0, null));
					}
				}).start();
			break;
		case SET_SEARCH:
			new Thread(new Runnable() {
					@Override
					public void run() {
						int ret;
						mBlindScanInfo = null;
						synchronized(mParent) {
							if (mPlayer != null) {
								ret = mPlayer.BlindScanStart();
							} else {
								ret = 2;
							}
						}
						while (ret == 0 && mRequestCancel == false) {
							SystemClock.sleep(500);
							synchronized(mParent) {
								if (mPlayer != null) {
									ret = mPlayer.BlindScanGetState();
								} else {
									ret = 2;
								}
							}
						}
						if (ret == 1) {
							synchronized(mParent) {
								if (mPlayer != null) {
									Parcel parcel = mPlayer.BlindScanGetInfo();
									mBlindScanInfo = null;
									if (parcel != null) {
										mBlindScanInfo = new BlindScanInfo(parcel);
										parcel.recycle();
									}
								}
							}
						}
						if (mBlindScanInfo != null) {
							sendMessage(obtainMessage(BLIND_SCAN, NOTIFY_PERCENT, 0, null));
						} else {
							synchronized(mParent) {
								if (mPlayer != null) {
									mPlayer.BlindScanCancel();
								} else {
									DxbLog(E, "[ExecuteBlindScan] mPlayer is null");
									mRequestCancel = true;
								}
							}
							sendMessage(obtainMessage(BLIND_SCAN, NOTIFY_COMPLETION, 0, null));
						}
					}
				}).start();
			break;
		case NOTIFY_PERCENT:
			if (mBlindScanInfo != null) {
				onBlindScanUpdate(mPlayer, mBlindScanInfo.mPercent, mBlindScanInfo.mFreqMHz, mBlindScanInfo.mSymbolKHz);
				if (mBlindScanInfo.mPercent == 100) {
					mChannelIndex++;
					if (mChannelIndex < mTuneSpace.numbers) {
						new Thread(new Runnable() {
								@Override
								public void run() {
									synchronized(mParent) {
										if (mPlayer != null) {
											mPlayer.BlindScanCancel();
										} else {
											DxbLog(E, "[ExecuteBlindScan] mPlayer is null");
											mRequestCancel = true;
										}
									}
									sendMessage(obtainMessage(BLIND_SCAN, SET_DISH, 0, null));
								}
							}).start();
					} else {
						new Thread(new Runnable() {
								@Override
								public void run() {
									synchronized(mParent) {
										if (mPlayer != null) {
											mPlayer.BlindScanCancel();
										} else {
											DxbLog(E, "[ExecuteBlindScan] mPlayer is null");
											mRequestCancel = true;
										}
									}
									sendMessage(obtainMessage(BLIND_SCAN, NOTIFY_COMPLETION, 0, null));
								}
							}).start();
					}
				} else {
					sendMessage(obtainMessage(BLIND_SCAN, SET_SEARCH, 0, null));
				}
			}
			break;
		case NOTIFY_COMPLETION:
			onBlindScanCompletion(mPlayer);
			break;
		}
	}

	@Override
	public void onSearchPercentUpdate(DVBTPlayer player, int nPercent) {
		switch (mState) {
		case NONE:
			break;
		case MULTI_CHANNEL_SCAN:
			if (nPercent == 100) {
				mChannelIndex++;
				nPercent = (int)(100.0 * (mChannelIndex) / mIndexList.length);
				mParent.onSearchPercentUpdate(player, nPercent);
				if (mChannelIndex < mIndexList.length) {
					this.sendMessage(this.obtainMessage(MULTI_CHANNEL_SCAN, SET_DISH, 0, null));
				}
			} else {
				nPercent = (int)((100.0 * mChannelIndex + nPercent) / mIndexList.length);
				mParent.onSearchPercentUpdate(player, nPercent);
			}
			break;
		case ONE_CHANNEL_SCAN:
			break;
		}
	}

	@Override
	public void onSearchCompletion(DVBTPlayer player) {
		if (!mRequestCancel) {
			if (mState == MULTI_CHANNEL_SCAN && (mChannelIndex+1) < mIndexList.length)
				return;
		}
		mRequestCancel = false;
		mState = NONE;
		mParent.onSearchCompletion(player);
	}

	public void onBlindScanUpdate(DVBTPlayer player, int nPercent, int nFreqMHz, int nSymbolKHz) {
		nPercent = (int)((100.0 * mChannelIndex + nPercent) / mTuneSpace.numbers);
		if (nFreqMHz > 0)
			nFreqMHz += mTuneSpace.channels[mChannelIndex].frequency1;
		mParent.onBlindScanUpdate(player, nPercent, mChannelIndex, nFreqMHz, nSymbolKHz);
	}

	public void onBlindScanCompletion(DVBTPlayer player) {
		mRequestCancel = false;
		mState = NONE;
		mParent.onBlindScanCompletion(player);
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
