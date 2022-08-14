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

public class DxbStorage
{
	static String getPath_Device()
	{
		return Manager_Setting.g_Information.cOption.storage;
	}
	
	static String getPath_Player()
	{
		if(Manager_Setting.ePLAYER == Manager_Setting.PLAYER.ATSC)
		{
			return "/atsc-";
		}
		else if(Manager_Setting.ePLAYER == Manager_Setting.PLAYER.DVBT)
		{
			return "/dvbt-";
		}
		else if(Manager_Setting.ePLAYER == Manager_Setting.PLAYER.ISDBT)
		{
			return "/isdbt-";
		}
		else if(Manager_Setting.ePLAYER == Manager_Setting.PLAYER.TDMB)
		{
			return "/tdmb-";
		}
		else
		{
			return "/test-";
		}
	}
	
	static String getPath_Extension_Capture()
	{
		return ".jpg";
	}
	
	static String getPath_Extension_Record()
	{
		if(Manager_Setting.ePLAYER == Manager_Setting.PLAYER.TDMB)
		{
			return ".mp4";
		}
		else
		{
			return ".ts";
		}		
	}
}
