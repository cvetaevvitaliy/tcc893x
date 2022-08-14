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

package com.telechips.android.tdmb.util;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;

public class DxbStorage {

	public static final int SDCARD_0	= 0;
	public static final int SDCARD_1	= 1;
	public static final int USB_0		= 2;
	public static final int USB_1		= 3;
	public static final int DEFAULT		= SDCARD_0;

	public static final String STORAGE_PATH = "/storage";

	public static final String SDCARD0_PATH	= "/storage/sdcard0";
	public static final String SDCARD1_PATH	= "/storage/sdcard1";
	public static final String USB0_PATH	= "/storage/usb0";
	public static final String USB1_PATH	= "/storage/usb1";

	public static int miCurrentStorage = DEFAULT;

	public static File[] getVolumeList() {
		File f = new File(STORAGE_PATH);
		return f.listFiles();
	}
	
	private static String getPath(int iStorage) {
		switch (iStorage) {
		case SDCARD_0:
			return SDCARD0_PATH;
		case SDCARD_1:
			return SDCARD1_PATH;
		case USB_0:
			return USB0_PATH;
		case USB_1:
			return USB1_PATH;
		default:
			return null;
		}
	}
	
	public static boolean isMounted(int iStorage) {
		String read;
		String path = getPath(iStorage);
		if (path == null) {
			return false;
		}
		try {
			BufferedReader bp = new BufferedReader(new FileReader("/proc/mounts"));
			while((read = bp.readLine())!=null) {
				if (read.contains(path)) {
					bp.close();
					return true;
				}
			}
			bp.close();
		} catch(IOException ex) {
			return false;
		}
		return false;
	}

	public static void setStorage(int iStorage) {
		switch (iStorage) {
		case SDCARD_0:
		case SDCARD_1:
		case USB_0:
		case USB_1:
			miCurrentStorage = iStorage;
		default:
			miCurrentStorage = DEFAULT;
		}
	}

	public static void setStorage(String sPath) {
		if (SDCARD0_PATH.equalsIgnoreCase(sPath)) {
			miCurrentStorage = SDCARD_0;
		} else if (SDCARD1_PATH.equalsIgnoreCase(sPath)) {
			miCurrentStorage = SDCARD_1;
		} else if (USB0_PATH.equalsIgnoreCase(sPath)) {
			miCurrentStorage = USB_0;
		} else if (USB1_PATH.equalsIgnoreCase(sPath)) {
			miCurrentStorage = USB_1;
		}
	}

	public static String getPath_Device() {
		return getPath(miCurrentStorage);
	}
	
	public static String getPath_Player(int iTab) {
		if(iTab == 0)
			return "/dmb-";
		else
			return "/dab-";
	}
	
	public static String getPath_Extension_Capture() {
		return ".jpg";
	}
	
	public static String getPath_Extension_Record(int iTab) {
		if(iTab == 0)
			return ".ts";
		else
			return ".mp2";
	}
}
