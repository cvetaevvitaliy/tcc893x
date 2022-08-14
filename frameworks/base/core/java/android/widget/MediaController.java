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

import android.content.Context;
import android.graphics.PixelFormat;
import android.media.AudioManager;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;
import android.widget.SeekBar.OnSeekBarChangeListener;

import com.android.internal.policy.PolicyManager;

import java.util.Formatter;
import java.util.Locale;

// VIEW_SUBTITLE
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;

/**
 * A view containing controls for a MediaPlayer. Typically contains the
 * buttons like "Play/Pause", "Rewind", "Fast Forward" and a progress
 * slider. It takes care of synchronizing the controls with the state
 * of the MediaPlayer.
 * <p>
 * The way to use this class is to instantiate it programatically.
 * The MediaController will create a default set of controls
 * and put them in a window floating above your application. Specifically,
 * the controls will float above the view specified with setAnchorView().
 * The window will disappear if left idle for three seconds and reappear
 * when the user touches the anchor view.
 * <p>
 * Functions like show() and hide() have no effect when MediaController
 * is created in an xml layout.
 * 
 * MediaController will hide and
 * show the buttons according to these rules:
 * <ul>
 * <li> The "previous" and "next" buttons are hidden until setPrevNextListeners()
 *   has been called
 * <li> The "previous" and "next" buttons are visible but disabled if
 *   setPrevNextListeners() was called with null listeners
 * <li> The "rewind" and "fastforward" buttons are shown unless requested
 *   otherwise by using the MediaController(Context, boolean) constructor
 *   with the boolean set to false
 * </ul>
 */
public class MediaController extends FrameLayout {
    private String LOG_TAG = "MediaController";

    private MediaPlayerControl  mPlayer;
    private Context             mContext;
    private View                mAnchor;
    private View                mRoot;
    private WindowManager       mWindowManager;
    private Window              mWindow;
    private View                mDecor;
    private WindowManager.LayoutParams mDecorLayoutParams;
    private ProgressBar         mProgress;
    private int                 mLatestProgress;
    private TextView            mEndTime, mCurrentTime;
    private boolean             mShowing;
    private boolean             mDragging;
    private static final int    sDefaultTimeout = 3000;
    private static final int    FADE_OUT = 1;
    private static final int    SHOW_PROGRESS = 2;
    // VIEW_SUBTITLE
    private static final int    SHOW_SUBTITLE = 3;

    private View                mVideoView;
    private int                 mRequestedSysUIModeAtHiding = -1;

    private long                mLastSeekEventTime;
    private static long         sSeekSkipTimeOffset = 250; // Fix me at some cases ..
    private boolean             mUseFastForward;
    private boolean             mStreamingMedia;
    private boolean             mFromXml;
    private boolean             mListenersSet;
    private View.OnClickListener mNextListener, mPrevListener;
    StringBuilder               mFormatBuilder;
    Formatter                   mFormatter;
    private ImageButton         mPauseButton;
    private ImageButton         mFfwdButton;
    private ImageButton         mRewButton;
    private ImageButton         mNextButton;
    private ImageButton         mPrevButton;

    // VIEW_SUBTITLE
    private static final int    SUBTITLE_MAX_DISP = 2; 
    private View                mSubtitle;
    private TextView[]          mSubtitleText = new TextView[SUBTITLE_MAX_DISP];
    private int                 mSubtitlePosition = 0;

    private static final int SUBTITLE_NONE = -1; 
    private static final int SUBTITLE_OFF = 0; 
    private static final int SUBTITLE_ON = 1; 

    private boolean mPGSCaptionEnable = false;
    private Bitmap	mBitmap;
    private Bitmap	mReSizedBitmap;
    private Bitmap	mBitmapErease;
    private Canvas	mCanvas;
    private Paint	mPaint;
    
    private int mPGSBitMap_Width;
    private int mPGSBitMap_Height;
    private ImageView PgsCaptionView;

    public MediaController(Context context, AttributeSet attrs) {
        super(context, attrs);
        mRoot = this;
        mContext = context;
        mUseFastForward = true;
        mStreamingMedia = false;
        mFromXml = true;
    }

    @Override
    public void onFinishInflate() {
        if (mRoot != null)
            initControllerView(mRoot);
    }

    /**
     * @hide
     */
	public MediaController(Context context, boolean useFastForward, boolean isStreamingMedia) {
        super(context);
        mContext = context;
        mUseFastForward = useFastForward;
        mStreamingMedia = isStreamingMedia;
        initFloatingWindowLayout();
        initFloatingWindow();
    }

    public MediaController(Context context, View videoView, int requestedSysUIModeAtHiding) {
        super(context);
        mContext = context;
        mUseFastForward = true;
        mVideoView = videoView;
        mRequestedSysUIModeAtHiding = requestedSysUIModeAtHiding;
        mStreamingMedia = false;
        initFloatingWindowLayout();
        initFloatingWindow();
    }

    public MediaController(Context context, boolean useFastForward) {
        super(context);
        mContext = context;
        mUseFastForward = useFastForward;
        mStreamingMedia = false;
        initFloatingWindowLayout();
        initFloatingWindow();
    }

    public MediaController(Context context) {
        this(context, true);
    }

    private void initFloatingWindow() {
        mWindowManager = (WindowManager)mContext.getSystemService(Context.WINDOW_SERVICE);
        mWindow = PolicyManager.makeNewWindow(mContext);
        mWindow.setWindowManager(mWindowManager, null, null);
        mWindow.requestFeature(Window.FEATURE_NO_TITLE);
        mDecor = mWindow.getDecorView();
        mDecor.setOnTouchListener(mTouchListener);
        mWindow.setContentView(this);
        mWindow.setBackgroundDrawableResource(android.R.color.transparent);

        // While the media controller is up, the volume control keys should
        // affect the media stream type
        mWindow.setVolumeControlStream(AudioManager.STREAM_MUSIC);

        setFocusable(true);
        setFocusableInTouchMode(true);
        setDescendantFocusability(ViewGroup.FOCUS_AFTER_DESCENDANTS);
        requestFocus();
    }

    // Allocate and initialize the static parts of mDecorLayoutParams. Must
    // also call updateFloatingWindowLayout() to fill in the dynamic parts
    // (y and width) before mDecorLayoutParams can be used.
    private void initFloatingWindowLayout() {
        mDecorLayoutParams = new WindowManager.LayoutParams();
        WindowManager.LayoutParams p = mDecorLayoutParams;
        p.gravity = Gravity.TOP | Gravity.LEFT;
        p.height = LayoutParams.WRAP_CONTENT;
        p.x = 0;
        p.format = PixelFormat.TRANSLUCENT;
        p.type = WindowManager.LayoutParams.TYPE_APPLICATION_PANEL;
        p.flags |= WindowManager.LayoutParams.FLAG_ALT_FOCUSABLE_IM
                | WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL
                | WindowManager.LayoutParams.FLAG_SPLIT_TOUCH;
        p.token = null;
        p.windowAnimations = 0; // android.R.style.DropDownAnimationDown;
    }

    // Update the dynamic parts of mDecorLayoutParams
    // Must be called with mAnchor != NULL.
    private void updateFloatingWindowLayout() {
        int [] anchorPos = new int[2];
        mAnchor.getLocationOnScreen(anchorPos);

        // we need to know the size of the controller so we can properly position it
        // within its space
        mDecor.measure(MeasureSpec.makeMeasureSpec(mAnchor.getWidth(), MeasureSpec.AT_MOST),
                MeasureSpec.makeMeasureSpec(mAnchor.getHeight(), MeasureSpec.AT_MOST));

        WindowManager.LayoutParams p = mDecorLayoutParams;
        p.width = mAnchor.getWidth();
        p.x = anchorPos[0] + (mAnchor.getWidth() - p.width) / 2;
        p.y = anchorPos[1] + mAnchor.getHeight() - mDecor.getMeasuredHeight();
    }

    // This is called whenever mAnchor's layout bound changes
    private OnLayoutChangeListener mLayoutChangeListener =
            new OnLayoutChangeListener() {
        public void onLayoutChange(View v, int left, int top, int right,
                int bottom, int oldLeft, int oldTop, int oldRight,
                int oldBottom) {
            updateFloatingWindowLayout();
            if (mShowing) {
                mWindowManager.updateViewLayout(mDecor, mDecorLayoutParams);
            }
        }
    };

    private OnTouchListener mTouchListener = new OnTouchListener() {
        public boolean onTouch(View v, MotionEvent event) {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                if (mShowing) {
                    hide();
                }
            }
            return false;
        }
    };
    
    public void setMediaPlayer(MediaPlayerControl player) {
        mPlayer = player;
        updatePausePlay();
    }

    /**
     * Set the view that acts as the anchor for the control view.
     * This can for example be a VideoView, or your Activity's main view.
     * When VideoView calls this method, it will use the VideoView's parent
     * as the anchor.
     * @param view The view to which to anchor the controller when it is visible.
     */
    public void setAnchorView(View view) {
        if (mAnchor != null) {
            mAnchor.removeOnLayoutChangeListener(mLayoutChangeListener);
        }
        mAnchor = view;
        if (mAnchor != null) {
            mAnchor.addOnLayoutChangeListener(mLayoutChangeListener);
        }

        FrameLayout.LayoutParams frameParams = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
        );

        removeAllViews();
        View v = makeControllerView();
        addView(v, frameParams);
    }

    // VIEW_SUBTITLE
    /**
     * Set the view that acts as the anchor for the subtitle view.
     * This can for example be a VideoView, or your Activity's subtitle view.
     *
     * @hide
     */
    public void setSubtitleView(int nFontSize) {
        if (mSubtitle == null) {
            makeSubtitleView(nFontSize);
        }
        mSubtitlePosition = 0;
    }

    /**
     * Get the subtitle number to display.
     *
     * @hide
     */
    public int getSubtitleDispNum() {
        return  SUBTITLE_MAX_DISP;
    }

    /**
     * Create the view that holds the widgets that control playback.
     * Derived classes can override this to create their own.
     * @return The controller view.
     * @hide This doesn't work as advertised
     */
    protected View makeControllerView() {
        LayoutInflater inflate = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mRoot = inflate.inflate(com.android.internal.R.layout.media_controller, null);

        initControllerView(mRoot);

        return mRoot;
    }

    private void initControllerView(View v) {
        mPauseButton = (ImageButton) v.findViewById(com.android.internal.R.id.pause);
        if (mPauseButton != null) {
            mPauseButton.requestFocus();
            mPauseButton.setOnClickListener(mPauseListener);
        }

        mFfwdButton = (ImageButton) v.findViewById(com.android.internal.R.id.ffwd);
        if (mFfwdButton != null) {
            mFfwdButton.setOnClickListener(mFfwdListener);
            if (!mFromXml) {
                mFfwdButton.setVisibility(mUseFastForward ? View.VISIBLE : View.GONE);
            }
        }

        mRewButton = (ImageButton) v.findViewById(com.android.internal.R.id.rew);
        if (mRewButton != null) {
            mRewButton.setOnClickListener(mRewListener);
            if (!mFromXml) {
                mRewButton.setVisibility(mUseFastForward ? View.VISIBLE : View.GONE);
            }
        }

        // By default these are hidden. They will be enabled when setPrevNextListeners() is called 
        mNextButton = (ImageButton) v.findViewById(com.android.internal.R.id.next);
        if (mNextButton != null && !mFromXml && !mListenersSet) {
            mNextButton.setVisibility(View.GONE);
        }
        mPrevButton = (ImageButton) v.findViewById(com.android.internal.R.id.prev);
        if (mPrevButton != null && !mFromXml && !mListenersSet) {
            mPrevButton.setVisibility(View.GONE);
        }

        mProgress = (ProgressBar) v.findViewById(com.android.internal.R.id.mediacontroller_progress);
        if (mProgress != null) {
            if (mProgress instanceof SeekBar) {
                SeekBar seeker = (SeekBar) mProgress;
                seeker.setOnSeekBarChangeListener(mSeekListener);
            }
            mProgress.setMax(1000);
        }

        mEndTime = (TextView) v.findViewById(com.android.internal.R.id.time);
        mCurrentTime = (TextView) v.findViewById(com.android.internal.R.id.time_current);
        mFormatBuilder = new StringBuilder();
        mFormatter = new Formatter(mFormatBuilder, Locale.getDefault());

        installPrevNextListeners();
    }

    // VIEW_SUBTITLE
    /**
     * Create the view that holds the widgets that control subtitle.
     * Derived classes can override this to create their own.
     * @return The subtitle view.
     * @hide This doesn't work as advertised
     */
    protected View makeSubtitleView(int nFontSize) {
        LayoutInflater inflate = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mSubtitle = inflate.inflate(com.android.internal.R.layout.subtitle_controller, null);

        initSubtitleView(mSubtitle, nFontSize);

        return mSubtitle;
    }

    // VIEW_SUBTITLE
    private void initSubtitleView(View v, int nFontSize) {
        // external subtitle
        mSubtitleText[0] = (TextView) v.findViewById(com.android.internal.R.id.subtitle_class0);
        mSubtitleText[1] = (TextView) v.findViewById(com.android.internal.R.id.subtitle_class1);
        for (int i=0; i<SUBTITLE_MAX_DISP; i++) {
            mSubtitleText[i].setTextSize(nFontSize);
            mSubtitleText[i].setText(" ");
        }

        WindowManager.LayoutParams p = new WindowManager.LayoutParams();
        p.format = PixelFormat.TRANSLUCENT;
        p.type = WindowManager.LayoutParams.TYPE_APPLICATION_PANEL;
        p.flags |= WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE | WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE;
        p.token = null;
        p.windowAnimations = 0; 

        mWindowManager.addView(v, p);
     
        // internal subtitle
        mPGSBitMap_Width = 1280;
        mPGSBitMap_Height = 720;
        
        mBitmapErease = Bitmap.createBitmap(mPGSBitMap_Width, mPGSBitMap_Height, Bitmap.Config.ARGB_8888);
        
        PgsCaptionView = new ImageView(mContext);
        PgsCaptionView.setScaleType(ImageView.ScaleType.CENTER_INSIDE);
        PgsCaptionView.setVisibility(View.VISIBLE);
        
        p.format = PixelFormat.TRANSLUCENT;
        p.type = WindowManager.LayoutParams.TYPE_APPLICATION_PANEL;
        p.flags |= WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE | WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE;
        p.token = null;
        p.windowAnimations = 0; 
        mWindowManager.addView(PgsCaptionView, p);
        mPGSCaptionEnable = true;
    }

    /**
     * @hide
     */
    public void specifyStreamingMedia() {
		mStreamingMedia = true;
    }

    // VIEW_SUBTITLE
    /**
     * @hide
     */
    public void clearSubtitle() {
        if (mSubtitle != null) {
            for (int i=0; i<SUBTITLE_MAX_DISP; i++) {
                if (mSubtitleText[i] != null) {
                    mSubtitleText[i].setText(" ");
                }
            }
        }
    }

    // VIEW_SUBTITLE
    /**
     * @hide
     */
    public void setSubtitleOnoff(boolean onoff) {
        if (mSubtitle != null) {
            for (int i=0; i<SUBTITLE_MAX_DISP; i++) {
                if (mSubtitleText[i] != null) {
                    if (onoff == true) {
                        mSubtitleText[i].setVisibility(View.VISIBLE);
                    } else {
                        mSubtitleText[i].setVisibility(View.INVISIBLE);
                    }
                }
            }
        }

        if (mPGSCaptionEnable == true) {
	        if (onoff == true)
	            PgsCaptionView.setVisibility(View.VISIBLE);
	        else 
	            PgsCaptionView.setVisibility(View.INVISIBLE);
        }
    }
	
    // VIEW_SUBTITLE
    /**
     * @hide
     */
    public void setSubtitleFontSize(int fontsize) {
        if (mSubtitle != null) {
            for (int i=0; i<SUBTITLE_MAX_DISP; i++) {
                if (mSubtitleText[i] != null) {
            		mSubtitleText[i].setTextSize(fontsize);
                }
            }
        }
    }

    /**
     * Show the controller on screen. It will go away
     * automatically after 3 seconds of inactivity.
     */
    public void show() {
		if(SystemProperties.get("show.controller.always", "0").equals("1")) {
			show(0); // Never go away
		} else {
			show(sDefaultTimeout);
		}
    }

    /**
     * Disable pause or seek buttons if the stream cannot be paused or seeked.
     * This requires the control interface to be a MediaPlayerControlExt
     */
    private void disableUnsupportedButtons() {
        try {
            if (mPauseButton != null && !mPlayer.canPause()) {
                mPauseButton.setEnabled(false);
            }
            if (mRewButton != null && !mPlayer.canSeekBackward()) {
                mRewButton.setEnabled(false);
            }
            if (mFfwdButton != null && !mPlayer.canSeekForward()) {
                mFfwdButton.setEnabled(false);
            }
        } catch (IncompatibleClassChangeError ex) {
            // We were given an old version of the interface, that doesn't have
            // the canPause/canSeekXYZ methods. This is OK, it just means we
            // assume the media can be paused and seeked, and so we don't disable
            // the buttons.
        }
    }

    /**
     * @hide
     */
    public void disableUnsupportedProgressThumb() {
        try {
            if (mProgress != null 
                    && (!mPlayer.canSeekForward() && !mPlayer.canSeekBackward())) {
                if (mProgress instanceof SeekBar) {
                    SeekBar seeker = (SeekBar) mProgress;
                    seeker.setUserSeekable(false);
                }
            }
        } catch (IncompatibleClassChangeError ex) {
            // We were given an old version of the interface, that doesn't have
            // the canPause/canSeekXYZ methods. This is OK, it just means we
            // assume the media can be paused and seeked, and so we don't disable
            // the buttons.
        }
    }
    
    /**
     * Show the controller on screen. It will go away
     * automatically after 'timeout' milliseconds of inactivity.
     * @param timeout The timeout in milliseconds. Use 0 to show
     * the controller until hide() is called.
     */
    public void show(int timeout) {
        if (!mShowing && mAnchor != null) {
            setProgress();
            if (mPauseButton != null) {
                mPauseButton.requestFocus();
            }
            disableUnsupportedButtons();
            disableUnsupportedProgressThumb();
            updateFloatingWindowLayout();
            mWindowManager.addView(mDecor, mDecorLayoutParams);
            mShowing = true;
        }
        updatePausePlay();
        
        // cause the progress bar to be updated even if mShowing
        // was already true.  This happens, for example, if we're
        // paused with the progress bar showing the user hits play.
        mHandler.sendEmptyMessage(SHOW_PROGRESS);

        Message msg = mHandler.obtainMessage(FADE_OUT);
        if (timeout != 0) {
            mHandler.removeMessages(FADE_OUT);
            mHandler.sendMessageDelayed(msg, timeout);
        }
    }
    
    public boolean isShowing() {
        return mShowing;
    }

    /**
     * Remove the controller from the screen.
     */
    public void hide() {
        if (mAnchor == null)
            return;

        if (mShowing) {
            try {
                mHandler.removeMessages(SHOW_PROGRESS);
                mWindowManager.removeView(mDecor);
                if (mRequestedSysUIModeAtHiding >= 0) {
                    mVideoView.setSystemUiVisibility(mRequestedSysUIModeAtHiding);
                }
            } catch (IllegalArgumentException ex) {
                Log.w(LOG_TAG, "already removed");
            }
            mShowing = false;

            if (mPGSCaptionEnable) {
                if (mBitmapErease != null)
                    mBitmapErease.eraseColor(0x00000000);
      
                if (PgsCaptionView != null) {
                    PgsCaptionView.setImageBitmap(mBitmapErease);
                    PgsCaptionView.requestLayout();
                }
            }
        }
    }

    /**
     * @hide
     */
    public void hideSubtitle() {
        if( mSubtitle != null ){
            mWindowManager.removeView(mSubtitle);
            mSubtitle = null;
        }
		
        if( PgsCaptionView != null){
            mWindowManager.removeView(PgsCaptionView); 
            PgsCaptionView = null;
            mPGSCaptionEnable = false;
        }	
    }

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            int pos;
            switch (msg.what) {
                case FADE_OUT:
                    hide();
                    break;
                case SHOW_PROGRESS:
                    pos = setProgress();
                    if (!mDragging && mShowing && mPlayer.isPlaying()) {
                        msg = obtainMessage(SHOW_PROGRESS);
                        sendMessageDelayed(msg, 1000 - (pos % 1000));
                    }
                    break;
            }
        }
    };

    private String stringForTime(int timeMs) {
        int totalSeconds = timeMs / 1000;

        int seconds = totalSeconds % 60;
        int minutes = (totalSeconds / 60) % 60;
        int hours   = totalSeconds / 3600;

        mFormatBuilder.setLength(0);
        if (hours > 0) {
            return mFormatter.format("%d:%02d:%02d", hours, minutes, seconds).toString();
        } else {
            return (timeMs >= 0) ? mFormatter.format("%02d:%02d", minutes, seconds).toString()
                                 : mFormatter.format("--:--").toString();
        }
    }

    private int setProgress() {
        if (mPlayer == null || mDragging) {
            return 0;
        }
        int position = mPlayer.getCurrentPosition();
        int duration = mPlayer.getDuration();

        if (duration <= 0 && !mPlayer.canSeekForward() && !mPlayer.canSeekBackward()) {
            duration = -1;
            position = -1;
        }

        if (mProgress != null) {
            if (duration > 0) {
                // use long to avoid overflow
                long pos = 1000L * position / duration;
                mProgress.setProgress( (int) pos);
            }
            int percent = mPlayer.getBufferPercentage();
            mProgress.setSecondaryProgress(percent * 10);
        }

        if (mEndTime != null)
            mEndTime.setText(stringForTime(duration));
        if (mCurrentTime != null)
            mCurrentTime.setText(stringForTime(position));

        return position;
    }

    /**
     * @hide
     */
    public int setSubtitle(int index, String subtitle)
    {
        if (mPlayer == null) {
            return 0;
        }

        if (mSubtitle != null && mSubtitleText[index] != null) {
            int iSubtitleWidth = 0;
            int iSubtitleHeight = 0;

            mWindowManager.removeView(mSubtitle);

            if (subtitle != null) {
                mSubtitleText[index].setText(subtitle);
            }
            else {
                mSubtitleText[index].setText(" ");
            }

            for (int i=0; i<SUBTITLE_MAX_DISP; i++) {
                mSubtitleText[i].measure(MeasureSpec.UNSPECIFIED, MeasureSpec.UNSPECIFIED);
                Log.w(LOG_TAG, "i:" + i + " W:" + mSubtitleText[i].getMeasuredWidth()
                        + " H:" + mSubtitleText[i].getMeasuredHeight());

                if (iSubtitleWidth < mSubtitleText[i].getMeasuredWidth()){
                    iSubtitleWidth = mSubtitleText[i].getMeasuredWidth();
                }

                if (mSubtitleText[i].getMeasuredWidth() > 0){
                    iSubtitleHeight += mSubtitleText[i].getMeasuredHeight();
                }
            }
            Log.w(LOG_TAG, "Measure W:"+iSubtitleWidth+" H:"+iSubtitleHeight);

            WindowManager.LayoutParams p = new WindowManager.LayoutParams();
            p.gravity = Gravity.TOP | Gravity.CENTER_HORIZONTAL;
            p.x = 0;
            p.y = mAnchor.getHeight() - iSubtitleHeight - mSubtitlePosition;
            //if (mShowing) {
            //    p.y -= mDecor.getMeasuredHeight();
            //}
			p.width = iSubtitleWidth;
            //p.width = mAnchor.getWidth(); 
            p.height = iSubtitleHeight; 
            Log.w(LOG_TAG, "Total X:"+p.x+" Y:"+p.y+" W:"+p.width+" H:" + p.height);

            p.format = PixelFormat.TRANSLUCENT;
            p.type = WindowManager.LayoutParams.TYPE_APPLICATION_PANEL;
            p.flags |= WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE | WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE;
            p.token = null;
            p.windowAnimations = 0; 

            mWindowManager.addView(mSubtitle, p);
        }

        return 0;
    }

    /**
     * @hide
     */
    public void setSubtitlePosition(int position)
    {
        if (mSubtitlePosition != position) {
            mSubtitlePosition = position;

            if (mSubtitle != null) {
                for (int i=0; i<SUBTITLE_MAX_DISP; i++) {
                    if (mSubtitleText[i] != null) {
                        setSubtitle(i, mSubtitleText[i].getText().toString());
                    }
                }
            }
        }
    }

    /**
     * @hide
     */
    public void setPGSCaption(int OffsetX, int OffsetY, int Srcwidth, int SrcHeight,
            int Dstwidth, int DstHeight, int StreamSize, int[] StreamData) {
       if(StreamSize == 0)
       {
          mBitmapErease.eraseColor(0x00000000);
          PgsCaptionView.setImageBitmap(mBitmapErease);
       }
       else
       {
          WindowManager.LayoutParams p = new WindowManager.LayoutParams();
          p.gravity = Gravity.TOP;
          p.width = mAnchor.getWidth();
          p.y = OffsetY;
          p.height = DstHeight;
          p.format = PixelFormat.TRANSLUCENT;
          p.type = WindowManager.LayoutParams.TYPE_APPLICATION_PANEL;
          p.flags |= WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE | WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE;
          p.token = null;
          p.windowAnimations = 0; 
          
          mWindowManager.removeView(PgsCaptionView);
          mWindowManager.addView(PgsCaptionView, p);
          
          mBitmapErease.eraseColor(0x00000000);
          PgsCaptionView.setImageBitmap(mBitmapErease);
          mBitmap = Bitmap.createBitmap(StreamData, 0, Srcwidth, Srcwidth, SrcHeight, Bitmap.Config.ARGB_8888);
          mReSizedBitmap = Bitmap.createScaledBitmap(mBitmap, Dstwidth, DstHeight, true);
          PgsCaptionView.setImageBitmap(mReSizedBitmap);
       }
       PgsCaptionView.requestLayout();
       setFocusable(false);
    }


    @Override
    public boolean onTouchEvent(MotionEvent event) {
        show(sDefaultTimeout);
        return true;
    }

    @Override
    public boolean onTrackballEvent(MotionEvent ev) {
        show(sDefaultTimeout);
        return false;
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        int keyCode = event.getKeyCode();
        final boolean uniqueDown = event.getRepeatCount() == 0
                && event.getAction() == KeyEvent.ACTION_DOWN;
        if (keyCode ==  KeyEvent.KEYCODE_HEADSETHOOK
                || keyCode == KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE
                || keyCode == KeyEvent.KEYCODE_SPACE) {
            if (uniqueDown) {
                doPauseResume();
                show(sDefaultTimeout);
                if (mPauseButton != null) {
                    mPauseButton.requestFocus();
                }
            }
            return true;
        } else if (keyCode == KeyEvent.KEYCODE_MEDIA_PLAY) {
            if (uniqueDown && !mPlayer.isPlaying()) {
                mPlayer.start();
                updatePausePlay();
                show(sDefaultTimeout);
            }
            return true;
        } else if (keyCode == KeyEvent.KEYCODE_MEDIA_STOP
                || keyCode == KeyEvent.KEYCODE_MEDIA_PAUSE) {
            if (uniqueDown && mPlayer.isPlaying()) {
                mPlayer.pause();
                updatePausePlay();
                show(sDefaultTimeout);
            }
            return true;
        } else if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN
                || keyCode == KeyEvent.KEYCODE_VOLUME_UP
                || keyCode == KeyEvent.KEYCODE_VOLUME_MUTE
                || keyCode == KeyEvent.KEYCODE_CAMERA) {
            // don't show the controls for volume adjustment
            return super.dispatchKeyEvent(event);
        } else if (keyCode == KeyEvent.KEYCODE_BACK || keyCode == KeyEvent.KEYCODE_MENU) {
            if (uniqueDown) {
                hide();
            }
            return true;
        }

        show(sDefaultTimeout);
        return super.dispatchKeyEvent(event);
    }

    private View.OnClickListener mPauseListener = new View.OnClickListener() {
        public void onClick(View v) {
            doPauseResume();
            show(sDefaultTimeout);
        }
    };

    private void updatePausePlay() {
        if (mRoot == null || mPauseButton == null)
            return;

        if (mPlayer.isPlaying()) {
            mPauseButton.setImageResource(com.android.internal.R.drawable.ic_media_pause);
        } else {
            mPauseButton.setImageResource(com.android.internal.R.drawable.ic_media_play);
        }
    }

    private void doPauseResume() {
        if (mPlayer.isPlaying()) {
            mPlayer.pause();
        } else {
            mPlayer.start();
        }
        updatePausePlay();
    }

	private void doSeek(int progress) {
		// Seek audio and/or video track.
		long duration = mPlayer.getDuration();
		long newposition = (duration * progress) / 1000L;
		Log.v(LOG_TAG, "doSeek " + (int) newposition);
		mPlayer.seekTo( (int) newposition);
		if (mCurrentTime != null)
			mCurrentTime.setText(stringForTime( (int) newposition));
		
		// VIEW_SUBTITLE
		clearSubtitle();
	}

    // There are two scenarios that can trigger the seekbar listener to trigger:
    //
    // The first is the user using the touchpad to adjust the posititon of the
    // seekbar's thumb. In this case onStartTrackingTouch is called followed by
    // a number of onProgressChanged notifications, concluded by onStopTrackingTouch.
    // We're setting the field "mDragging" to true for the duration of the dragging
    // session to avoid jumps in the position in case of ongoing playback.
    //
    // The second scenario involves the user operating the scroll ball, in this
    // case there WON'T BE onStartTrackingTouch/onStopTrackingTouch notifications,
    // we will simply apply the updated position without suspending regular updates.
    private OnSeekBarChangeListener mSeekListener = new OnSeekBarChangeListener() {
        public void onStartTrackingTouch(SeekBar bar) {
            show(3600000);

            mDragging = true;
            mLastSeekEventTime = 0;

            // By removing these pending progress messages we make sure
            // that a) we won't update the progress while the user adjusts
            // the seekbar and b) once the user is done dragging the thumb
            // we will post one of these messages to the queue again and
            // this ensures that there will be exactly one message queued up.
            mHandler.removeMessages(SHOW_PROGRESS);
        }

        public void onProgressChanged(SeekBar bar, int progress, boolean fromuser) {
            if (!fromuser) {
                // We're not interested in programmatically generated changes to
                // the progress bar's position.
                return;
            }

            if (mStreamingMedia && mDragging) {
				mLatestProgress = progress;
				Log.v(LOG_TAG, "onProgressChanged "+ mLatestProgress);
				return;
			}

            long now = SystemClock.elapsedRealtime();
            if ((now - mLastSeekEventTime) > sSeekSkipTimeOffset) {
                mLastSeekEventTime = now;
            } else {
                Log.v(LOG_TAG, "onProgressChanged skips doSeek");
                return;
            }
            if (!mStreamingMedia) {
                doSeek(progress);
            }
        }

        public void onStopTrackingTouch(SeekBar bar) {
			if (mStreamingMedia && mDragging) {
				doSeek(mLatestProgress);
			}
            mDragging = false;
            setProgress();
            updatePausePlay();
            show(sDefaultTimeout);

            // Ensure that progress is properly updated in the future,
            // the call to show() does not guarantee this because it is a
            // no-op if we are already showing.
            mHandler.sendEmptyMessage(SHOW_PROGRESS);
        }
    };

    @Override
    public void setEnabled(boolean enabled) {
        if (mPauseButton != null) {
            mPauseButton.setEnabled(enabled);
        }
        if (mFfwdButton != null) {
            mFfwdButton.setEnabled(enabled);
        }
        if (mRewButton != null) {
            mRewButton.setEnabled(enabled);
        }
        if (mNextButton != null) {
            mNextButton.setEnabled(enabled && mNextListener != null);
        }
        if (mPrevButton != null) {
            mPrevButton.setEnabled(enabled && mPrevListener != null);
        }
        if (mProgress != null) {
            mProgress.setEnabled(enabled);
            if (mProgress instanceof SeekBar) {
                SeekBar seeker = (SeekBar) mProgress;
                seeker.setUserSeekable(enabled);
            }
        }
        disableUnsupportedButtons();
        disableUnsupportedProgressThumb();
        super.setEnabled(enabled);
    }

    @Override
    public void onInitializeAccessibilityEvent(AccessibilityEvent event) {
        super.onInitializeAccessibilityEvent(event);
        event.setClassName(MediaController.class.getName());
    }

    @Override
    public void onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo info) {
        super.onInitializeAccessibilityNodeInfo(info);
        info.setClassName(MediaController.class.getName());
    }

    private View.OnClickListener mRewListener = new View.OnClickListener() {
        public void onClick(View v) {
            if (mPlayer.canSeekBackward()) {
                int pos = mPlayer.getCurrentPosition();
                pos -= 5000; // milliseconds
                mPlayer.seekTo(pos);
            }
            setProgress();

            // VIEW_SUBTITLE
            clearSubtitle();

            show(sDefaultTimeout);
        }
    };

    private View.OnClickListener mFfwdListener = new View.OnClickListener() {
        public void onClick(View v) {
            if (mPlayer.canSeekForward()) {
                int pos = mPlayer.getCurrentPosition();
                pos += 15000; // milliseconds
                mPlayer.seekTo(pos);
            }
            setProgress();

            // VIEW_SUBTITLE
            clearSubtitle();

            show(sDefaultTimeout);
        }
    };

    private void installPrevNextListeners() {
        if (mNextButton != null) {
            mNextButton.setOnClickListener(mNextListener);
            mNextButton.setEnabled(mNextListener != null);
        }

        if (mPrevButton != null) {
            mPrevButton.setOnClickListener(mPrevListener);
            mPrevButton.setEnabled(mPrevListener != null);
        }
    }

    public void setPrevNextListeners(View.OnClickListener next, View.OnClickListener prev) {
        if (mStreamingMedia == true) {
            mListenersSet = false;
            return;
        }

        mNextListener = next;
        mPrevListener = prev;
        mListenersSet = true;

        if (mRoot != null) {
            installPrevNextListeners();
            
            if (mNextButton != null && !mFromXml) {
                mNextButton.setVisibility(View.VISIBLE);
            }
            if (mPrevButton != null && !mFromXml) {
                mPrevButton.setVisibility(View.VISIBLE);
            }
        }
    }

    public interface MediaPlayerControl {
        void    start();
        void    pause();
        int     getDuration();
        int     getCurrentPosition();
        void    seekTo(int pos);
        boolean isPlaying();
        int     getBufferPercentage();
        boolean canPause();
        boolean canSeekBackward();
        boolean canSeekForward();

        /**
         * Get the audio session id for the player used by this VideoView. This can be used to
         * apply audio effects to the audio track of a video.
         * @return The audio session, or 0 if there was an error.
         */
        int     getAudioSessionId();
    }
}
