/*
 * Copyright (C) 2010 Telechips, Inc.
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

package com.android.server;

import android.app.TelechipsDisplayManager;
import android.app.ITelechipsDisplayManager;
import android.provider.Settings;
import android.util.Log;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.SystemProperties;
import android.app.AlarmManager;

import android.content.Intent;
import android.content.IntentFilter;
import android.text.TextUtils;
import android.net.SntpClient;
import android.os.SystemClock;
import android.text.format.Time;

import java.util.Date;
import java.util.TimeZone;
import java.util.Calendar;

public class TelechipsDisplayManagerService<syncronized> extends ITelechipsDisplayManager.Stub{
    private static final boolean DEBUG =  false;
    private static final boolean NTP_DEBUG =  false;
    private Context mContext;
    private static final String TAG = "TelechipsDisplayManagerService";
    private int mDisplayMode = TelechipsDisplayManager.DISPLAY_OUTPUT_LCD;
    private int mPreviousDisplayMode = TelechipsDisplayManager.DISPLAY_OUTPUT_LCD;
    public static final String OUTPUT_MODE = "output_mode";
    public static final String HDMI_RESOLUTION = "hdmi_resolution";
    private int mDisplayResolution = TelechipsDisplayManager.DISPLAY_DEFAULT_HDMI_RESOLUTION;
    private int mPreviousDisplayResolution = TelechipsDisplayManager.DISPLAY_DEFAULT_HDMI_RESOLUTION;
    public static final String HDMI_MODE = "hdmi_mode";
    private int mHDMIMode = TelechipsDisplayManager.DISPLAY_DEFAULT_HDMI_MODE;
    private int mPreviousHDMIMode = TelechipsDisplayManager.DISPLAY_DEFAULT_HDMI_MODE;
    private static final String TIMEZONE_PROPERTY = "persist.sys.timezone";

    private boolean mMediaBox;

    private final String NTP_SERVER = "pool.ntp.org";//"kr.pool.ntp.org" "gps.bora.net"
    private String mNtpServer;

    public TelechipsDisplayManagerService(Context context){
        mContext = context;

        mMediaBox = mContext.getResources().getBoolean(com.android.internal.R.bool.config_enable_displaymanager);

        //if(mMediaBox == true) {
            mContext.registerReceiver(
                new BroadcastReceiver() {
                    @Override
                    public void onReceive(Context context, Intent intent) {

                        if (intent.getAction().equals(Intent.ACTION_BOOT_COMPLETED)) {
							//Android SDK support NetworkTimeUpdateService, so this code is not needed anymore, just use setTimeZone
                           //mNtpServer = getNTPServer();
                           setTimeZone();
                        }
                }
                },
                new IntentFilter(Intent.ACTION_BOOT_COMPLETED));

            startMonitoring();
        //}

    }

    public void startMonitoring() {
        if(DEBUG) Log.i(TAG, "TelechipsDisplayManager startMonitoring ");
        new MonitorThread().start();
    }

    class MonitorThread extends Thread {

        public MonitorThread() {
            super("DisplayMonitor");
        }

        public void run() {
            for (;;) {
                if(mMediaBox){
                mDisplayMode =  Integer.valueOf(SystemProperties.get("persist.sys.output_mode","0"));//Integer.valueOf(SystemProperties.get("tcc.display.output_mode","0"));//

                //if(DEBUG) Log.i(TAG, "TelechipsDisplayManager monitor mDisplayMode = " + mDisplayMode + " mPreviousDisplayMode = " + mPreviousDisplayMode);

                if( mPreviousDisplayMode != mDisplayMode )
                {
                    if(DEBUG) Log.i(TAG,"mPreviousDisplayMode " + mPreviousDisplayMode + " mDisplayMode " + mDisplayMode);
                    setOutputMode(mDisplayMode);

                    // Broadcast
                    final Intent intent = new Intent(TelechipsDisplayManager.DISPLAY_MODE_CHANGED_ACTION);
                    intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
                    intent.putExtra(TelechipsDisplayManager.EXTRA_DISPLAY_MODE, mDisplayMode);
                    intent.putExtra(TelechipsDisplayManager.EXTRA_PREVIOUS_DISPLAY_MODE, mPreviousDisplayMode);
                    mContext.sendStickyBroadcast(intent);

                }

                mPreviousDisplayMode = mDisplayMode;

                mDisplayResolution =    Integer.valueOf(SystemProperties.get("persist.sys.hdmi_resolution","0"));
                if( mPreviousDisplayResolution != mDisplayResolution )
                {
                    if(DEBUG) Log.i(TAG,"mPreviousDisplayResolution " + mPreviousDisplayResolution + " mDisplayResolution " + mDisplayResolution);
                    setOutputResolution(mDisplayResolution);

                    final Intent intent = new Intent(TelechipsDisplayManager.DISPLAY_RESOLUTION_CHANGED_ACTION);
                    intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
                    intent.putExtra(TelechipsDisplayManager.EXTRA_DISPLAY_RESOLUTION, mDisplayResolution);
                    intent.putExtra(TelechipsDisplayManager.EXTRA_PREVIOUS_DISPLAY_RESOLUTION, mPreviousDisplayResolution);
                    mContext.sendStickyBroadcast(intent);

                }
                mPreviousDisplayResolution = mDisplayResolution;

                mHDMIMode = Integer.valueOf(SystemProperties.get("persist.sys.hdmi_mode","0"));
                if( mPreviousHDMIMode != mHDMIMode )
                {
                    if(DEBUG) Log.i(TAG,"mPreviousHDMIMode " + mPreviousHDMIMode + " mHDMIMode " + mHDMIMode);
                    setHDMIMode(mHDMIMode);

                    final Intent intent = new Intent(TelechipsDisplayManager.HDMI_MODE_CHANGED_ACTION);
                    intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
                    intent.putExtra(TelechipsDisplayManager.EXTRA_HDMI_MODE, mHDMIMode);
                    intent.putExtra(TelechipsDisplayManager.EXTRA_PREVIOUS_HDMI_MODE, mPreviousHDMIMode);
                    mContext.sendStickyBroadcast(intent);
                }
                mPreviousHDMIMode = mHDMIMode;
                }

				//Android SDK support NetworkTimeUpdateService, so this code is not needed anymore
                //getNPTTime();
                
                if("1".equals(SystemProperties.get("persist.sys.auto_resolution")) 
                    && (SystemProperties.getBoolean("ro.QB.enable", false))
                    && (SystemProperties.get("tcc.QB.boot.with", "not yet").equals("normal"))
                    && (SystemProperties.get("persist.sys.QB.user.allow", "empty").equals("empty"))) 
                {
                    Intent clsIntent = new Intent(mContext, QBPopup.class );
                    clsIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    mContext.startActivity(clsIntent);
                }
 

                try {
                    Thread.sleep(2000);
                } catch (InterruptedException iex) {
                    Log.e(TAG, "Interrupted while waiting for media", iex);
                }

            }
        }
    }

    private int getPersistedDisplayOutputMode() {
        if(DEBUG)  Log.d(TAG, "getPersistedDisplayOutputMode");

        final ContentResolver cr = mContext.getContentResolver();
        try {
            return Settings.System.getInt(cr, OUTPUT_MODE);
        } catch (Settings.SettingNotFoundException e) {
            return TelechipsDisplayManager.DISPLAY_OUTPUT_LCD;
        }
    }

    private synchronized void persistDisplayOutputMode(int mode)  {
        if(DEBUG)  Log.d(TAG, "persistDisplayOutputMode = " + mode);

        final ContentResolver cr = mContext.getContentResolver();
        Settings.System.putInt(cr,OUTPUT_MODE,mode);
    }

    public int getOutputMode( ) {
        return mDisplayMode;
    }

    public void setOutputMode(int mode) {
        if(DEBUG)  Log.d(TAG, "setOutputMode = " + mode);
        mDisplayMode = mode;
        persistDisplayOutputMode(mDisplayMode);
    }

    private int getPersistedDisplayResolution() {
        if(DEBUG)  Log.d(TAG, "getPersistedDisplayResolution");

        final ContentResolver cr = mContext.getContentResolver();
        try {
            return Settings.System.getInt(cr, HDMI_RESOLUTION);
        } catch (Settings.SettingNotFoundException e) {
            return TelechipsDisplayManager.DISPLAY_DEFAULT_HDMI_RESOLUTION;
        }
    }

    private synchronized void persistDisplayResolution(int resolution)  {
        if(DEBUG)  Log.d(TAG, "persistDisplayResolution = " + resolution);

        final ContentResolver cr = mContext.getContentResolver();
        Settings.System.putInt(cr,HDMI_RESOLUTION,resolution);
    }

    public int getOutputResolution( ) {
        return mDisplayResolution;
    }

    public void setOutputResolution(int resolution) {
        if(DEBUG)  Log.d(TAG, "setOutputResolution = " + resolution);
        mDisplayResolution = resolution;
        persistDisplayResolution(mDisplayResolution);
    }

    private synchronized void persistDisplayHDMIMode(int mode)  {
        if(DEBUG)  Log.d(TAG, "persistDisplayHDMIMode = " + mode);

        final ContentResolver cr = mContext.getContentResolver();
        Settings.System.putInt(cr,HDMI_MODE,mode);
    }

    public int getHDMIMode( ) {
        return mHDMIMode;
    }

    public void setHDMIMode(int mode) {
        if(DEBUG)  Log.d(TAG, "setHDMIMode = " + mode);
        mHDMIMode = mode;
        persistDisplayHDMIMode(mHDMIMode);
    }

    public int getSetMode( ) {
        return mDisplayMode;
    }

    public void setSetMode(int mode) {
        if(DEBUG)  Log.d(TAG, "setSetMode = " + mode);
        //mDisplayMode = mode;
    }

    public int getSetHDMode( ) {
        return mDisplayMode;
    }

    public void setSetHDMode(int mode) {
        if(DEBUG)  Log.d(TAG, "setSetHDMode = " + mode);
        //mDisplayMode = mode;
    }

    ///// below functions is used for NTP
	private void setTimeZone()
    {
        //timezone default setting
        AlarmManager alarm = (AlarmManager) mContext.getSystemService(Context.ALARM_SERVICE);

        String dault_timezone = SystemProperties.get("tcc.default.timezone");

        String tz = SystemProperties.get(TIMEZONE_PROPERTY);

        if(NTP_DEBUG) Log.i(TAG, "dault_timezone : " + dault_timezone + " tz : " + tz );

        if (tz != null && !TextUtils.isEmpty(tz)) {
            alarm.setTimeZone(tz);
        }
        else{
            if(!TextUtils.isEmpty(dault_timezone)){
                alarm.setTimeZone(dault_timezone);
            }
        }

    }
	/*
    private void getNPTTime() {

        SntpClient client = new SntpClient();
        boolean wifiState = SystemProperties.get("dhcp.wlan0.result").equals("ok");
        boolean ethState = SystemProperties.get("dhcp.eth0.result").equals("ok");

        //if(NTP_DEBUG) Log.i(TAG, "getNPTTime wifiState: " + wifiState + " ethState " + ethState);

        if (( wifiState || ethState) && getAutoTime() && getNTPUpdate() && client.requestTime(mNtpServer, 10*1000)) {
            //if(getAutoTimeZone())
                setTimeZone();

            long now = client.getNtpTime() + SystemClock.elapsedRealtime() - client.getNtpTimeReference();

            if(NTP_DEBUG) Log.i(TAG, "getNPTTime setCurrentTimeMillis: now" + now + " current " + System.currentTimeMillis());
            SystemClock.setCurrentTimeMillis(now);

            final ContentResolver cr = mContext.getContentResolver();
            Settings.System.putInt(cr, Settings.System.NTP_UPDATE,0);

        }

        return;

    }

    private boolean getNTPUpdate() {

        final ContentResolver cr = mContext.getContentResolver();

        //if(NTP_DEBUG) Log.i(TAG, "getNTPUpdate " + Settings.System.getInt(cr, Settings.System.NTP_UPDATE,0));

        return Settings.System.getInt(cr, Settings.System.NTP_UPDATE,0) > 0;
    }

    private boolean getAutoTime() {

        final ContentResolver cr = mContext.getContentResolver();

        //if(NTP_DEBUG) Log.i(TAG, "getAutoTime " + Settings.Global.getInt(cr, Settings.Global.AUTO_TIME,0) );

        return Settings.Global.getInt(cr, Settings.Global.AUTO_TIME,0) > 0;
    }


    private boolean getAutoTimeZone() {

        final ContentResolver cr = mContext.getContentResolver();

        //if(NTP_DEBUG) Log.i(TAG, "getAutoTimeZone " + Settings.System.getInt(cr, Settings.System.AUTO_TIME_ZONE,0) );

        return Settings.Global.getInt(cr, Settings.Global.AUTO_TIME_ZONE,0) > 0;
    }

    private static char[] formatOffset(int off) {
        off = off / 1000 / 60;

        char[] buf = new char[9];
        buf[0] = 'G';
        buf[1] = 'M';
        buf[2] = 'T';

        if (off < 0) {
            buf[3] = '-';
            off = -off;
        } else {
            buf[3] = '+';
        }

        int hours = off / 60;
        int minutes = off % 60;

        buf[4] = (char) ('0' + hours / 10);
        buf[5] = (char) ('0' + hours % 10);

        buf[6] = ':';

        buf[7] = (char) ('0' + minutes / 10);
        buf[8] = (char) ('0' + minutes % 10);

        return buf;
    }

    private String getTimeZoneText(TimeZone tz) {
        boolean daylight = tz.inDaylightTime(new Date());
        StringBuilder sb = new StringBuilder();

        sb.append(formatOffset(tz.getRawOffset() +
                               (daylight ? tz.getDSTSavings() : 0))).
            append(", ").
            append(tz.getDisplayName(daylight, TimeZone.LONG));

        return sb.toString();
    }

    private String getNTPServer() {

        final ContentResolver cr = mContext.getContentResolver();

        // this code is for initial connection to ntp when ntp never be set
        //if(Settings.System.getInt(cr, Settings.System.NTP_CONF,0)==0){
            Settings.Global.putString(cr, Settings.Global.NTP_SERVER, NTP_SERVER);
            Settings.System.putInt(cr, Settings.System.NTP_UPDATE,1);
            Settings.System.putInt(cr, Settings.System.NTP_CONF,1);

        //}
        setTimeZone();

        if(NTP_DEBUG) Log.d(TAG, "getNTPServer NTP_SERVER " + Settings.Global.getString(cr, Settings.Global.NTP_SERVER) + " NTP_CONF " + Settings.System.getInt(cr, Settings.System.NTP_CONF,0));


        return Settings.Global.getString(cr, Settings.Global.NTP_SERVER);

    }
	*/
}
