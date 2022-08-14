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
package com.telechips.android.isdbt.player;

import android.os.Parcel;
import android.os.Parcelable;

public class DxbEPGData
{
	public int iTableID;
	public int iSectionNumber;
	public int iEventID;

	public int iStartMJD;
	public int iStartHH;
	public int iStartMM;
	public int iDurationHH;
	public int iDurationMM;
	//public int iEndHH;
	//public int iEndMM;

	public String sEvtName;
	public String sEvtText;
	public String sEvtExtern;

	public int iGenre;
	public int iAudioMode;
	public int iAudioLang1;
	public int iAudioLang2;
	public int iVideoMode;
	public int iRating;

	public DxbEPGData()
	{
		
	}
	
	public DxbEPGData( int tableID,
                       int sectionNumber,
                       int eventID,
                       int startMJD,
                       int startHH,
                       int startMM,
                       int durationHH,
                       int durationMM,
                       //int endHH,
                       //int endMM,
                       String evtName,
                       String evtText,
                       String evtExtern,
                       int genre,
                       int audioMode,
                       int audioLang1,
                       int audioLang2,
                       int videoMode,
                       int rating
	)
	{
		this.iTableID			= tableID;
		this.iSectionNumber		= sectionNumber;
		this.iEventID			= eventID;

		this.iStartMJD			= startMJD;
		this.iStartHH			= startHH;
		this.iStartMM			= startMM;
		this.iDurationHH		= durationHH;
		this.iDurationMM		= durationMM;
		//this.iEndHH				= endHH;
		//this.iEndMM				= endMM;
		
		this.sEvtName			= evtName;
		this.sEvtText			= evtText;
		this.sEvtExtern			= evtExtern;
		
		this.iGenre				= genre;
		this.iAudioMode			= audioMode;
		this.iAudioLang1		= audioLang1;
		this.iAudioLang2		= audioLang2;
		this.iVideoMode			= videoMode;
		this.iRating			= rating;
	}	
}
