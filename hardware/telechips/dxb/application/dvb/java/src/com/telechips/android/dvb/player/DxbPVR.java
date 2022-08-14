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

import android.util.Log;
import android.os.Handler;
import android.os.Message;

public class DxbPVR extends Handler implements DVBTPlayer.OnPlayingCompletionListener,
		DVBTPlayer.OnFilePlayTimeUpdateListener, DVBTPlayer.OnRecordingCompletionListener
{
	private static final int NONE        = 0;
	private static final int SET_REC     = 1 << 0;
	private static final int STOP_REC    = 1 << 1;
	private static final int SET_PLAY    = 1 << 2;
	private static final int PAUSE       = 1 << 3;
	private static final int STOP_PLAY   = 1 << 4;
	private static final int SET_SPEED   = 1 << 5;

	private static final int NO_WAIT     = 0;
	private static final int WAIT        = 1;

	private final DxbPlayerBase mParent;
	private DVBTPlayer mPlayer;

	private int mState = NONE;

	public static enum LOCAL_STATE {
		STOP,
		PLAYING,
		PAUSE
	}
	private LOCAL_STATE mPlayState = LOCAL_STATE.STOP;

	public static enum REC_STATE {
		STOP,
		RECORDING,
		RECORDING_ALARM,
		SAVE,
		NULL
	}
	private REC_STATE mRecState = REC_STATE.STOP;

	public DxbPVR(DxbPlayerBase parent) {
		mParent = parent;
	}

	public void setPlayer(DVBTPlayer player) {
		synchronized(mParent) {
			mPlayer = player;
			if (mPlayer != null) {
				mPlayer.setOnPlayingCompletionListener(this);
				mPlayer.setOnFilePlayTimeUpdateListener(this);
				mPlayer.setOnRecordingCompletionListener(this);
			}
		}
	}

	public LOCAL_STATE getPlayState() {
		return mPlayState;
	}

	public REC_STATE getRecordState() {
		return mRecState;
	}

	public int setCapture(String name) {
		synchronized(mParent) {
			if (mPlayer == null) {
				DxbLog(E, "[setCapture] mPlayer is null");
				return 1;
			}
			return mPlayer.setCapture(name);
		}
	}

	public boolean setRecord(String name) {
		if (mPlayer == null) {
			DxbLog(E, "[setRecord] mPlayer is null");
			return false;
		}
		mRecState = REC_STATE.RECORDING;
		sendMessage(obtainMessage(SET_REC, 0, 0, name));
		return true;
	}

	public void setRecStop() {
		if (mPlayer == null) {
			DxbLog(E, "[setRecStop] mPlayer is null");
			return;
		}
		mRecState = REC_STATE.SAVE;
		sendMessage(obtainMessage(STOP_REC, 0, 0, null));
	}

	public boolean setPlay(String name) {
		if (mPlayer == null) {
			DxbLog(E, "[setPlay] mPlayer is null");
			return false;
		}
		if (mPlayState != LOCAL_STATE.STOP) {
			if (name == null) { // time-shift
				DxbLog(W, "[setPlay] playing");
				return false;
			}
			sendMessage(obtainMessage(STOP_PLAY, SET_PLAY, 0, name)); // stop & start
		} else {
			mPlayState = LOCAL_STATE.PLAYING;
			sendMessage(obtainMessage(SET_PLAY, NO_WAIT, 0, name)); // start
		}
		return true;
	}

	public void setPlayStop() {
		if (mPlayer == null) {
			DxbLog(E, "[setPlayStop] mPlayer is null");
			return;
		}
		sendMessage(obtainMessage(STOP_PLAY, 0, 0, null));
	}

	public void pause() {
		if (mPlayer == null) {
			DxbLog(E, "[pause] mPlayer is null");
			return;
		}
		mPlayState = LOCAL_STATE.PAUSE;
		sendMessage(obtainMessage(PAUSE, 0, 0, null));
	}

	public void resume() {
		synchronized(mParent) {
			if (mPlayer == null) {
				DxbLog(E, "[resume] mPlayer is null");
				return;
			}
			mPlayState = LOCAL_STATE.PLAYING;
			mPlayer.setPlayPause(true);
		}
	}

	public void setSeekTo(int seekTime) {
		synchronized(mParent) {
			if (mPlayer == null) {
				DxbLog(E, "[setSeekTo] mPlayer is null");
				return;
			}
			mPlayer.setSeekTo(seekTime);
		}
	}

	public int setSpeed(int iSpeed) {
		if (mPlayer == null) {
			DxbLog(E, "[setSpeed] mPlayer is null");
			return -1;
		}
		sendMessage(obtainMessage(SET_SPEED, iSpeed, 0, null));
		return 0;
	}

	public int getDuration() {
		synchronized(mParent) {
			if (mPlayer == null) {
				DxbLog(E, "[getDuration] mPlayer is null");
				return 0;
			}
			return mPlayer.getDuration();
		}
	}

	public int getCurrentPosition() {
		synchronized(mParent) {
			if (mPlayer == null) {
				DxbLog(E, "[getCurrentPosition] mPlayer is null");
				return 0;
			}
			return mPlayer.getCurrentPosition();
		}
	}

	public int getCurrentReadPosition() {
		synchronized(mParent) {
			if (mPlayer == null) {
				DxbLog(E, "[getCurrentReadPosition] mPlayer is null");
				return 0;
			}
			return mPlayer.getCurrentReadPosition();
		}
	}

	public int getCurrentWritePosition() {
		synchronized(mParent) {
			if (mPlayer == null) {
				DxbLog(E, "[getCurrentWritePosition] mPlayer is null");
				return 0;
			}
			return mPlayer.getCurrentWritePosition();
		}
	}

	public String getDiskFSType() {
		String fsType = null;

		synchronized(mParent) {
			if (mPlayer == null) {
				DxbLog(E, "[getDiskFSType] mPlayer is null");
				return null;
			}
			fsType = mPlayer.getDiskFSType();
		}
		if(fsType.equalsIgnoreCase("UFSD") == true) {
			return "NTFS";
		}
		return fsType;
	}

	@Override
	public void handleMessage(Message msg) {
		switch (msg.what) {
		case SET_REC:
			final String recName = (String)msg.obj;
			mState |= SET_REC;
			new Thread(new Runnable() {
				@Override
				public void run() {
					mState &= ~SET_REC;
					synchronized(mParent) {
						DxbLog(D, "[handleMessage] SET_REC (name = " + recName + ")");
						if (mPlayer != null) {
							mPlayer.setRecord(recName);
						}
					}
				}
			}).start();
			break;
		case STOP_REC:
			mState |= STOP_REC;
			new Thread(new Runnable() {
				@Override
				public void run() {
					while ((mState & SET_REC) != 0);
					mState &= ~STOP_REC;
					synchronized(mParent) {
						DxbLog(D, "[handleMessage] STOP_REC");
						if (mPlayer != null) {
							mPlayer.setRecStop();
						}
					}
				}
			}).start();
			break;
		case SET_PLAY:
			if (msg.arg1 == WAIT) {
				if (mPlayState == LOCAL_STATE.STOP) {
					mPlayState = LOCAL_STATE.PLAYING;
				} else {
					DxbLog(D, "[handleMessage] SET_PLAY (Wait)");
					sendMessageDelayed(msg, 100);
					break;
				}
			}

			final String playName = (String)msg.obj;
			final int state = msg.arg1;
			mState |= SET_PLAY;
			new Thread(new Runnable() {
				@Override
				public void run() {
					if (playName == null)
						while ((mState & SET_REC) != 0);
					mState &= ~SET_PLAY;
					synchronized(mParent) {
						DxbLog(D, "[handleMessage] SET_PLAY (name = " + playName + ")");
						if (mPlayer != null) {
							if (playName == null) {
								mPlayer.setPlay("");
							} else {
								mPlayer.setPlay(playName);
								mParent.onSetChannelCompletion();
							}
							if (state == PAUSE) {
								DxbLog(D, "[handleMessage] SET_PLAY (PAUSE)");
								mPlayer.setPlayPause(false);
							}
						}
					}
				}
			}).start();
			break;
		case PAUSE:
			mState |= PAUSE;
			new Thread(new Runnable() {
				@Override
				public void run() {
					mState &= ~PAUSE;
					while ((mState & SET_PLAY) != 0);
					synchronized(mParent) {
						DxbLog(D, "[handleMessage] PAUSE");
						if (mPlayer != null) {
							mPlayer.setPlayPause(false);
						}
					}
				}
			}).start();
			break;
		case STOP_PLAY:
			mState |= STOP_PLAY;
			new Thread(new Runnable() {
				@Override
				public void run() {
					mState &= ~STOP_PLAY;
					while ((mState & SET_PLAY) != 0);
					synchronized(mParent) {
						DxbLog(D, "[handleMessage] STOP_PLAY");
						if (mPlayer != null) {
							mPlayer.setPlayStop();
						}
					}
				}
			}).start();
			if (msg.arg1 == SET_PLAY) {
				sendMessageDelayed(obtainMessage(SET_PLAY, WAIT, 0, msg.obj), 100);
			}
			break;
		case SET_SPEED:
			final int speed = msg.arg1;
			mState |= SET_SPEED;
			new Thread(new Runnable() {
				@Override
				public void run() {
					mState &= ~SET_SPEED;
					while ((mState & SET_PLAY) != 0);
					synchronized(mParent) {
						DxbLog(D, "[handleMessage] SET_SPEED");
						if (mPlayer != null) {
							mPlayer.setPlaySpeed(speed);
						}
					}
				}
			}).start();
			break;
		}
	}

	@Override
	public void onPlayingCompletion(DVBTPlayer player, int type, int state) {
		if (type == 0) {
			mPlayState = LOCAL_STATE.STOP;
		}
		mParent.onPlayingCompletion(player, type, state);
	}

	@Override
	public void onFilePlayTimeUpdate(DVBTPlayer player, int time) {
		mParent.onFilePlayTimeUpdate(player, time);
	}

	@Override
	public void onRecordingCompletion(DVBTPlayer player, int result) {
		mParent.onRecordingCompletion(player, result);
		mRecState = REC_STATE.STOP;
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
