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

import android.util.Log;

public class Manager_Setting
{
	private static final int D = 0;
	private static final int E = 1;
	private static final int I = 2;
	private static final int V = 3;
	private static final int W = 4;
	
	static int	MENU_NULL		= -1;
	static int	MENU_CHANNEL	= 1;
	static int	MENU_SCREEN		= 2;
	static int	MENU_EPG		= -1;
	static int	MENU_CAPTURE	= -1;
	static int	MENU_RECORD		= -1;
	static int	MENU_OPTION		= -1;

	/*******************************************************************************
	 * Option Value
	 *******************************************************************************/
		public static int		DEFAULT_VALUE_PRESET			= -1;
		public static int		DEFAULT_VALUE_AREA_CODE			= -1;
		public static int		DEFAULT_VALUE_PARENTAL_RATING	= 5;		// default
		public static String	DEFAULT_VALUE_STORAGE			= "/mnt/sdcard";	// default
	/*******************************************************************************/
	

	enum PLAYER
	{
		ATSC,
		DVBT,
		ISDBT,
		TDMB,
		NULL
	}
	static PLAYER	ePLAYER;
	
	static TDMBPlayerActivity mContext;
	
	/*
	 * Componet
	 */
	static Component	g_Component		= new Component();
	
	/*
	 * Information
	 */
	static Information	g_Information	= new Information();
	
	public static void init(TDMBPlayerActivity context)
	{
		DxbLog(I, "init()");
		
		/*	Set Player	*/
		ePLAYER	= PLAYER.TDMB;
		mContext	= context;
		
		/*	Set Viewer	*/
		if(ePLAYER != PLAYER.TDMB)
		{
			g_Information.cEPG.VISIBLE	= true;
		}
		if(		(ePLAYER == PLAYER.ATSC)
			||	(ePLAYER == PLAYER.DVBT)
		)
		{
			g_Information.cSUBT.VISIBLE	= true;
		}
		if(ePLAYER == PLAYER.DVBT)
		{
			g_Information.cTTX.VISIBLE	= true;
		}
		
		/*	Set ListView	*/
		if(ePLAYER == PLAYER.ISDBT)
		{
			g_Information.cLIST.iCount_Tab	= 1;
		}
		
		/*	Set FullView	*/
		if(ePLAYER == PLAYER.TDMB)
		{
			g_Information.cFULL.iCount_Menu	= 5;
			
			MENU_CAPTURE	= 3;
			MENU_RECORD		= 4;
			MENU_OPTION		= 5;
		}
		else if(ePLAYER == PLAYER.ATSC)
		{
			g_Information.cFULL.iCount_Menu	= 4;
			
			MENU_EPG		= 3;
			MENU_OPTION		= 4;
		}
		else
		{
			MENU_EPG		= 3;
			MENU_CAPTURE	= 4;
			MENU_RECORD		= 5;
			MENU_OPTION		= 6;
		}
		
		/*	set Option	*/
		Information.OPTION	mOption	= g_Information.cOption;
		mOption.storage				= DEFAULT_VALUE_STORAGE;
	}
	
	private static void DxbLog(int level, String mString)
	{
		if(DxbPlayer.TAG == null)
		{
			return;
		}
		
		String TAG = "[[[Manager_Setting]]] ";
		if(TAG != null)
		{
			switch(level)
			{
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
}
