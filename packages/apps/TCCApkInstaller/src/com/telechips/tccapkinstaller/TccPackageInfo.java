package com.telechips.tccapkinstaller;

import android.graphics.drawable.Drawable;

public class TccPackageInfo {

	TccPackageInfo()
	{
		m_sApkPath = null;
		m_sApkName = null;
		m_sPackagename  =null;
	}
	
	 CharSequence m_sTitle;
	 Drawable m_dicon;
	 boolean m_bIsInstall;
	 
	 String m_sApkPath;
	 String m_sApkName;
	 String m_sPackagename;
		 
}
