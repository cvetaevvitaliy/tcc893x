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

package com.telechips.android.tdmb;

import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

public class Component
{
	cIndicator	indicator	= new cIndicator();
	cListView	listview	= new cListView();
	cFullView	fullview	= new cFullView();
	cRecord		record		= new cRecord();
	cEPGView	epgview		= new cEPGView();

	// Subtitle
	ImageView	subtitle_Display;

	Teletext	teletext	= new Teletext();
	
	/*
	 * Indicator
	 */
	class cIndicator
	{
		ImageView	ivSection;
		ImageView	ivSignal;
		TextView	timeIndicator;
	}

	/*
	 * List View
	 */
	class cListView
	{
		//	Tab
			View	vTab1;
			View	vTab2;
			
			ImageView	ivTab1;
			ImageView	ivTab2;
			
			TextView	tvTab1;
			TextView	tvTab2;
			
		//	List
			ListView	lvService;
		
		// Video
			View		vVideo;
			TextView	tvMessage;
		
		// Title
		TextView	tvTitle;
		TextView	tvInformation;
		
		Button		bScan;
	
		int	ID_row;
	}
	
	/*
	 * Full View
	 */
	class cFullView
	{
		cNavi	navi	= new cNavi();
		cTitle	title	= new cTitle();
		
		// Video
		View		vVideo;

		/*
		 * Navi
		 */
		class cNavi
		{
			ViewGroup	vgNavi;
			
			TextView	tvTitle;
			Button		bTeletext;
			
			//	(Channel) Up/Down
			Button		bUp;
			Button		bDown;
			
			cMenu	menu	= new cMenu();
		}
			//	Menu
			class cMenu
			{
				View	vLayout;
				
				//	View
				View	vMenu5;
				View	vMenu6;
				
				//	Button
				Button	bMenu1;
				Button	bMenu2;
				Button	bMenu3;
				Button	bMenu4;
				Button	bMenu5;
				Button	bMenu6;
				
				//	(ImageView)Icon
				ImageView	ivMenu1;
				ImageView	ivMenu2;
				ImageView	ivMenu3;
				ImageView	ivMenu4;
				ImageView	ivMenu5;
				ImageView	ivMenu6;
				
				//	Text
				TextView	tvMenu1;
				TextView	tvMenu2;
				TextView	tvMenu3;
				TextView	tvMenu4;
				TextView	tvMenu5;
				TextView	tvMenu6;
				
				//	Specific
				Button	bCapture;
				Button	bRecord;
			}
		
		/*
		 * Title
		 */
		class cTitle
		{
			TextView	tvTitle;
		}
	}
	
	/*
	 * Record
	 */
	class cRecord
	{
		ViewGroup	vgMenu;
		
		ImageView	ivIcon;
		Button		bStop;
	}
	
	/*
	 * EPG
	 */
	class cEPGView
	{
		//	Navi
		Button		bPrev;
		Button		bNext;
		TextView	tvDate_Current;

		//	List
		ListView	lvEPG_List;
		TextView	tvName_Current;
		TextView	tvDetail_Current;
		
		//	NoData
		ImageView	ivNoData;
		
		// Video
		View		vVideo;
	}
	
	/*
	 * Teletext
	 */
	class Teletext
	{
		ImageView	subtitle;
		
		ViewGroup fullViewGroup_TTX;

		// ttx display area
		ImageView	ivDisplay;
		
		TextView	tvHeader;
		Button		buPage;
		Button		buStart;
		
		Button		buRed;
		Button		buGreen;
		Button		buYellow;
		Button		buCyan;
		
		
		Button		buPrev;
		Button		buNext;
		Button		buFPrev;
		Button		buFNext;
		
		Button		buOff;
	}
}
