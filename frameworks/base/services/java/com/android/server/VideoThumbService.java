package com.android.server;

import java.io.FileDescriptor;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import android.content.Context;
import android.graphics.Bitmap;
import android.media.MediaMetadataRetriever;
import android.os.RemoteException;
import android.provider.MediaStore;
import android.service.videothumb.IVideoThumbService;
import android.util.Log;

class VideoThumbService extends IVideoThumbService.Stub {
    private Semaphore mSemaphore;
    final String TAG = "VideoThumbService";
    private boolean mPauseFlag = false;
    private Thread mPauseReleaseThread = null;
    
    public VideoThumbService(Context context) {
        mSemaphore = new Semaphore(1);
    }
    
    @Override
    public Bitmap createVideoThumbnail(FileDescriptor fd) throws RemoteException {
        if( mPauseFlag ) {
            Log.w(TAG, "status of video thumbnail sevice is pause! skip fd::" + fd.toString());
            return null;
        }
        
        try {
            mSemaphore.tryAcquire(10, TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            Log.w(TAG, "semaphore acquire timeout for extracting video thumbnail! fd::" + fd.toString());
            return null;
        }

        if( MediaStore.check_vpu_running() == 1 ) {
            Log.w(TAG, "vpu is running! skip extracting thumbnail! fd::" + fd.toString());
            mSemaphore.release();
            return null;
        }

        Bitmap bitmap = null;
        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        try {
            retriever.setDataSource(fd);
            bitmap = retriever.getFrameAtTime(-1);
        } catch (IllegalArgumentException ex) {
            // Assume this is a corrupt video file
        } catch (RuntimeException ex) {
            // Assume this is a corrupt video file.
        } finally {
            try {
                retriever.release();
            } catch (RuntimeException ex) {
                // Ignore failures while cleaning up.
            }
        }
        
        mSemaphore.release();        
        return bitmap;
    }

    @Override
    public int pauseVideoThumbnail(int releaseMillis) throws RemoteException {
       
        if( mPauseReleaseThread != null )
            mPauseReleaseThread.interrupt();
        
        mPauseReleaseThread = null;

        if( releaseMillis == 0 ) {
            mPauseFlag = false;
            return 0;
        }
        
        final int releaseTimeout = releaseMillis;
        
        try {
            mSemaphore.tryAcquire(10, TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            Log.e(TAG, "semaphore acquire timeout to pause video thumbnail!");
            return -1;
        }
        
        mPauseFlag = true;

        Runnable r = new Runnable() {
            public void run() {
                  try {
                      if( releaseTimeout != 0)
                          Thread.sleep(releaseTimeout);
                    mPauseFlag = false;
                } catch (InterruptedException e) {
                    Log.d(TAG, "occured InterruptedException");                    
                }
            }
          };

          mPauseReleaseThread = new Thread(r);
          mPauseReleaseThread.start();

          mSemaphore.release();
          return 0;
    }
}
