/*
 * Copyright (C) 2006 The Android Open Source Project
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

package android.widget;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnErrorListener;
import android.media.TimedText;
import android.media.MediaSubtitle;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import java.io.IOException;
import java.util.Map;
import java.util.Vector;

/**
 * Parses a subtitle.  The Subtitle class
 * can load subtitle from various sources (such as external or internal files)
 */
public class Subtitle extends Activity {
	private String TAG = "Subtitle";

	private static final int CLASS_MAX_NUM    = 2;

	public static final int TYPE_NONE        = 0;
	public static final int TYPE_EXTERNAL    = 1;
	public static final int TYPE_INTERNAL    = 2;
	public static final int TYPE_INTERNAL_PGS = 3;
	
	public static final int DISPLAY_VIEW     = 0;
	public static final int DISPLAY_CCFB     = 1;
	
	private static final int STATE_UPDATED    = 0;
	private static final int STATE_DISPLAYED  = 1;
	private static final int STATE_CLEARED    = 2;

	private MediaPlayer      mMediaPlayer;
	private MediaSubtitle    mMediaSubtitle;
	private int              mSubtitleType;
	private String           mPath;
	private int[]            mSelectedClassIdx;
	private int              mTimeShiftPTS;
	private int              mDisplayType;

	private int[]            mState;
	private int[]            mStartPTS;
	private int[]            mEndPTS;
	private int              mPreviousPGSPTS;
	private String[]         mString;

	private static final int MSG_DISPLAY     = 1;
	private static final int MSG_CLEAR       = 2;
	private static final int MSG_STOP        = 3;

	private int              mPGS_STATE;
	
	private Vector<Integer>  mTimedTextTrackIndex = new Vector<Integer>();
	
	private Handler mHandler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			if (mMediaPlayer == null) {
				Log.e(TAG, "mMediaPlayer is null");
				return;
			}

			switch(msg.what) {
			case MSG_DISPLAY:
				if ((TYPE_EXTERNAL == mSubtitleType) || (TYPE_INTERNAL == mSubtitleType)) {
					int position = mMediaPlayer.getCurrentPosition() + mTimeShiftPTS;
					if (position < 0) {
						position = 0;
					}
					
					boolean changed = false;
					for (int i=0; i<CLASS_MAX_NUM; i++) {
						if (mEndPTS[mMediaSubtitle.mSubtitleClassIdx] < position) {
							if (mMediaSubtitle.mSubtitleStartPTS == mMediaSubtitle.mSubtitleEndPTS) {
								mStartPTS[mMediaSubtitle.mSubtitleClassIdx] = mEndPTS[mMediaSubtitle.mSubtitleClassIdx];
								mEndPTS[mMediaSubtitle.mSubtitleClassIdx] = mMediaSubtitle.mSubtitleEndPTS;
								mString[mMediaSubtitle.mSubtitleClassIdx] = " ";	
							} else {
								mStartPTS[mMediaSubtitle.mSubtitleClassIdx] = mMediaSubtitle.mSubtitleStartPTS;
								mEndPTS[mMediaSubtitle.mSubtitleClassIdx] = mMediaSubtitle.mSubtitleEndPTS;
								mString[mMediaSubtitle.mSubtitleClassIdx] = mMediaSubtitle.getSubtitleString();	
							}

							if (position < mEndPTS[mMediaSubtitle.mSubtitleClassIdx])
								mState[mMediaSubtitle.mSubtitleClassIdx] = STATE_UPDATED;

							mMediaPlayer.getSubtitle(mMediaSubtitle);
						}
						//Log.e(TAG, "Test "+i+","+mState[i]+","+mStartPTS[i]+","+mEndPTS[i]+","+mString[i]);

						if (mStartPTS[i] < position && mEndPTS[i] > position) {
							if (mState[i] == STATE_UPDATED) {
								if (mDisplayType == DISPLAY_VIEW) {
									if (mOnSubtitleListener != null) {
										mOnSubtitleListener.onSubtitle(i, mString[i]);
									}
								}
								mState[i] = STATE_DISPLAYED;
								changed = true;
							}
						} else {
							if (mState[i] == STATE_DISPLAYED) {
								if (mDisplayType == DISPLAY_VIEW) {
									if (mOnSubtitleListener != null) {
										mOnSubtitleListener.onSubtitle(i, " ");
									}
								}
								mState[i] = STATE_CLEARED;
								changed = true;
							}
						}
					}
/*
					if (mDisplayType == DISPLAY_CCFB) {
						if (changed == true) {
							String merge = null;
							for (int i=CLASS_MAX_NUM-1; i>=0; i--) {
								if (mState[i] == STATE_DISPLAYED) {
									if (merge == null) {
										if (mString[i] != null && !mString[i].isEmpty()) {
											merge = mString[i];
										}
									} else {
										merge = merge.concat("\n");
										if (mString[i] != null && !mString[i].isEmpty()) {
											merge = merge.concat(mString[i]);
										}
									}
								}
							}
							
							if (merge == null) {
								merge = " ";
							}

							mMediaPlayer.displaySubtitleString(merge);
						}
					}
*/
					msg = mHandler.obtainMessage(MSG_DISPLAY);
					mHandler.sendMessageDelayed(msg, 200);
				}
				else if(TYPE_INTERNAL_PGS == mSubtitleType)
				{
					int position = mMediaPlayer.getCurrentPosition() + mTimeShiftPTS;

					if (position < 0) {
						position = 0;
					}

					if(mPGS_STATE == STATE_DISPLAYED)
						mMediaPlayer.getSubtitle(mMediaSubtitle);

					int StartTm = mMediaSubtitle.mPGSStartTm;

					if(mPreviousPGSPTS != StartTm && StartTm != 0 )
					{
						mPGS_STATE = STATE_UPDATED;

						if(StartTm <= position)
						{
							mOnSubtitleListener.onPGSCaption(mMediaSubtitle.mPGSOffsetX, 
                                                             mMediaSubtitle.mPGSOffsetY, 
                                                             mMediaSubtitle.mSrcPGSWidth, 
                                                             mMediaSubtitle.mSrcPGSHeigth,
                                                             mMediaSubtitle.mDstPGSWidth,
                                                             mMediaSubtitle.mDstPGSHeigth,
                                                             mMediaSubtitle.mPGSStreamLen, 
                                                             mMediaSubtitle.mPGSStreamArray);
							mPreviousPGSPTS = StartTm;	
							mPGS_STATE = STATE_DISPLAYED;
						}
					}
					msg = mHandler.obtainMessage(MSG_DISPLAY);
					mHandler.sendMessageDelayed(msg, 50);
				}
				return;

			case MSG_CLEAR:
				if ((TYPE_EXTERNAL == mSubtitleType) || (TYPE_INTERNAL == mSubtitleType)) {
					for (int i=0; i<CLASS_MAX_NUM; i++) {
						if (mOnSubtitleListener != null) {
							mOnSubtitleListener.onSubtitle(i, " ");
						}
					}
					clear();

					mHandler.sendEmptyMessage(MSG_DISPLAY);
				}
				return;

			case MSG_STOP:
				mHandler.removeMessages(MSG_DISPLAY);
				return;

			default:
				Log.e(TAG, "Unknown message type " + msg.what);
				return;
			}
		}
	};

	public interface OnSubtitleListener
	{
		/**
		 * Called when the subtitle will display
		 */
		void onSubtitle(int index, String subtitle);
		void onPGSCaption(int OffsetX, int OffsetY, int Srcwidth, int SrcHeight, int Dstwidth, int DstHeight, int StreamSize, int[] StreamData);
	}

	/**
	 * Register a callback to be display
	 *
	 * @param listener the callback that will be run
	 */
	public void setOnSubtitleListener(OnSubtitleListener listener)
	{
		mOnSubtitleListener = listener;
	}

	private OnSubtitleListener mOnSubtitleListener;

	public Subtitle() {
		mMediaSubtitle = new MediaSubtitle();

		mSelectedClassIdx = new int[CLASS_MAX_NUM];
		mState = new int[CLASS_MAX_NUM];
		mStartPTS = new int[CLASS_MAX_NUM];
		mEndPTS = new int[CLASS_MAX_NUM];
		mString = new String[CLASS_MAX_NUM];

		for (int i=0; i<CLASS_MAX_NUM; i++) {
			mState[i] = STATE_CLEARED;
			mStartPTS[i] = 0;
			mEndPTS[i] = 0;
		}
	}

	public void start(MediaPlayer mp, String path, int fontsize, int displaytype) {
		if (mSubtitleType != TYPE_NONE) {
            stop();
		}

		mMediaPlayer = mp;
		mSelectedClassIdx[0] = 0;
		mSelectedClassIdx[1] = 0;
		mPath = path;
		mTimeShiftPTS = 0;
		mDisplayType = displaytype;

		mTimedTextTrackIndex.clear();

		mSubtitleType =	mMediaPlayer.openSubtitle(mPath, mDisplayType);
		if (mSubtitleType == TYPE_EXTERNAL ) {
			mMediaPlayer.getSubtitleClass(mMediaSubtitle);
			int iSubtitleClassNum = mMediaSubtitle.mSubtitleClassNum;
			if (CLASS_MAX_NUM < iSubtitleClassNum) {
				int[] piSubtitleClassSequence = {0, 1, 2, 3, 4};
				mMediaPlayer.setSubtitleClass(CLASS_MAX_NUM, piSubtitleClassSequence);
			}
			mHandler.sendEmptyMessage(MSG_DISPLAY);
		}
		else if (mSubtitleType == TYPE_INTERNAL) {
			MediaPlayer.TrackInfo[] trackInfos = mMediaPlayer.getTrackInfo();
			if (trackInfos != null && trackInfos.length > 0) {
				for (int i=0; i < trackInfos.length; ++i) {
					if ((trackInfos[i] != null) && (trackInfos[i].getTrackType() == MediaPlayer.TrackInfo.MEDIA_TRACK_TYPE_TIMEDTEXT)) {
						mTimedTextTrackIndex.add(i);
						mSubtitleType = TYPE_INTERNAL;
					}
				}

				if (mSubtitleType == TYPE_INTERNAL) {
					mMediaPlayer.setOnTimedTextListener(new MediaPlayer.OnTimedTextListener() {
						@Override
						public void onTimedText(MediaPlayer mp, TimedText text) {
						}
					});

					mSelectedClassIdx[0] = mTimedTextTrackIndex.get(mSelectedClassIdx[0]);
					mMediaPlayer.setSubtitleClass(1, mSelectedClassIdx);
				}
			}
			mHandler.sendEmptyMessage(MSG_DISPLAY);
		}
		else if(mSubtitleType == TYPE_INTERNAL_PGS)
		{
			mPreviousPGSPTS = 0;
			mPGS_STATE = STATE_DISPLAYED;
			mHandler.sendEmptyMessage(MSG_DISPLAY);
		}
		else
			return;
	}

	public void stop() {
		if (mMediaPlayer == null) {
			return;
		}

		if (mSubtitleType != TYPE_NONE) {
			if (mSubtitleType == TYPE_INTERNAL) {
				mTimedTextTrackIndex.clear();
			}

			Message msg = mHandler.obtainMessage(MSG_STOP);
			mHandler.sendMessageAtFrontOfQueue(msg);
			mMediaPlayer.closeSubtitle();
			mMediaPlayer = null;
			mSelectedClassIdx[0] = 0;
			mSelectedClassIdx[1] = 0;
			mPath = " ";
			mSubtitleType = TYPE_NONE ;
		}

		clear();
	}

	public void seek(int position) {
		if (mMediaPlayer == null) {
			return;
		}

		if ((mSubtitleType == TYPE_EXTERNAL)||(mSubtitleType == TYPE_INTERNAL)) {
			mHandler.removeMessages(MSG_DISPLAY);
			if (position < 5000) {
				position = 0;
			} else  {
				position -= 5000;
			} 
			mMediaPlayer.seekSubtitle(position); 

			Message msg = mHandler.obtainMessage(MSG_CLEAR);
			mHandler.sendMessageAtFrontOfQueue(msg);
		}
		
 		if(mSubtitleType == TYPE_INTERNAL_PGS)
		{
			int[] Null = new int[1];
			mOnSubtitleListener.onPGSCaption(0, 0, 0, 0, 0, 0, 0, Null);

			if (mMediaSubtitle != null) 
			    mMediaSubtitle.clearPGS();
		   
			mPreviousPGSPTS = 0;
			mPGS_STATE = STATE_DISPLAYED;
			mHandler.sendEmptyMessage(MSG_DISPLAY);
		}
	}

	public void clear() {
		for (int i=0; i<CLASS_MAX_NUM; i++) {
			mState[i] = STATE_CLEARED;
			mStartPTS[i] = 0;
			mEndPTS[i] = 0;
			mString[i] = " ";
		}

		if (mMediaSubtitle != null) {
			mMediaSubtitle.clearSubtitle();
		}
	}

	public int getSubtitleType() {
		return mSubtitleType;
	}

	public void changeSubtitle(int classcount, int[] classindex) {
		if (mMediaPlayer == null) {
			Log.e(TAG, "mMediaPlayer is null");
			return;
		}

		if (mSubtitleType == TYPE_EXTERNAL) {
			mMediaPlayer.getSubtitleClass(mMediaSubtitle);

			int iSubtitleClassNum = mMediaSubtitle.mSubtitleClassNum;
			if (iSubtitleClassNum > 1) {
				if (classcount > 2) {
					classcount = 2;
				}

				mMediaPlayer.setSubtitleClass(classcount, classindex);

				int position = mMediaPlayer.getCurrentPosition();
				seek( position);
			} 
		}
		else if (mSubtitleType == TYPE_INTERNAL) {
			if (mTimedTextTrackIndex.size() > 1) {
				mSelectedClassIdx[0] = mTimedTextTrackIndex.get(classindex[0]);
				mMediaPlayer.setSubtitleClass(1, mSelectedClassIdx);

				clear();
			}
		}
		else if (mSubtitleType == TYPE_INTERNAL_PGS) {
			// Todo
		}
	}

	public int setTimeShiftPTS(int iPTS) {
		int position;

		mTimeShiftPTS += iPTS;

		if (mMediaPlayer == null) {
			Log.e(TAG, "mMediaPlayer is null");
			return mTimeShiftPTS;
		}

		position = mMediaPlayer.getCurrentPosition() + mTimeShiftPTS;
		if (position < 0) {
			position = 0;
		}
					
		if ((mSubtitleType == TYPE_EXTERNAL)||(mSubtitleType == TYPE_INTERNAL)) {
			if ((mStartPTS[0] < position && mEndPTS[0] > position) == false) {
				seek(position);
			}
		} else if (mSubtitleType == TYPE_INTERNAL_PGS) {
		 	if (mMediaSubtitle.mPGSStartTm > position) {
				seek(position);
			}
		}

		return mTimeShiftPTS;
	}
}
