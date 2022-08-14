package com.telechips.tccAppsManager;

import android.provider.BaseColumns;

public class tccDatabaseItem {

	public static final class CreateDB implements BaseColumns{
		public static final String NAME = "Apkname";
		public static final String UNIQ = "UniqApk";
		public static final String _TABLENAME = "FavoriteItem";
		public static final String _CREATE = 
			"create table "+_TABLENAME+"(" 
					+_ID+" integer primary key autoincrement, " 	
					+NAME+" text not null,"
					+UNIQ+" text not null);";
	}
}
