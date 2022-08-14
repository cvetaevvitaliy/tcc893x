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

package com.telechips.android.dvb.schedule;

public class Alert
{
	/*
	 * Action Text
	 */
	public final static String ACTION_ALERT_STATE		= "com.telechips.android.schedule.alert.state";
	public final static String ACTION_CHANGE_CHANNEL	= "com.telechips.android.schedule.change.channel";
	public final static String ACTION_REC_STOP			= "com.telechips.android.schedule.rec.stop";
	public final static String ACTION_DXB_PLAYING_REQ	= "com.telechips.android.dxb.playing.req";
	public final static String ACTION_DXB_PLAYING_RES	= "com.telechips.android.dxb.playing.res";
	
	/*
	 * Extra Text
	 */
	public final static String EXTRA_TYPE_IS_SCREEN_ON	= "extra.type.isscreenon";
	public final static String EXTRA_TYPE_ALARM			= "extra.type.alarm";
	//{
		public final static int	MAX_ALARM					= 2;
		
		//	View	: 0-notice_view
		//	Rec		: 1-notice_rec, 2-rec_stop
			public final static int	TYPE_ALARM_NOTICE_VIEW		= 0;
			public final static int TYPE_ALARM_NOTICE_REC		= 1;
			public final static int TYPE_ALARM_REC_STOP			= 2;
	//}
	
	public final static String EXTRA_TYPE_ALERT_STATE	= "extra.type.alert.state";
	//{
		//	0-Stop, 1-Start, 2-Pause
			public final static int TYPE_ALERT_STOP		= 0;
			public final static int TYPE_ALERT_START	= 1;
			public final static int TYPE_ALERT_PAUSE	= 2;
	//}
			
	public final static String EXTRA_TYPE_ID			= "extra.type.id";
	public final static String EXTRA_TYPE_TABLE			= "extra.type.table";
	public final static String EXTRA_TYPE_CHANNEL_TYPE	= "extra.type.channel.type";	//	0-tv, 1-radio
	public final static String EXTRA_TYPE_CHANNEL_INDEX	= "extra.type.index";
	public final static String EXTRA_TYPE_REPEAT		= "extra.type.repeat";			//	0-once, 1-daily, 2-weekly
	public final static String EXTRA_TYPE_TIME			= "extra.type.time";
}
