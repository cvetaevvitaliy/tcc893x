/*
 * Copyright (C) 2009 Telechips, Inc.
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
package android.media;

import android.util.Log;
import java.util.Locale;
import java.io.UnsupportedEncodingException;

/**
 * {@hide}
 */
public class MediaSubtitle {
	private final static String TAG = "MediaSubtitle";

	private static final int CHARACTERSET_NONE    = 0;
	private static final int CHARACTERSET_ANSI    = 1;
	private static final int CHARACTERSET_UNICODE = 2;
	private static final int CHARACTERSET_UTF8    = 3;

	public Locale mLocale;
	public String mLocaleString;

	public int mSubtitleClassIdx;
	public int mSubtitleStartPTS;
	public int mSubtitleEndPTS;
	public int mSubtitleCharSet;
	public String mSubtitleString;

	public int mPGSStartTm;
	public int mSrcPGSWidth;
	public int mSrcPGSHeigth;
	public int mDstPGSWidth;
	public int mDstPGSHeigth;
	public int mPGSOffsetX;
	public int mPGSOffsetY;
	public int mPGSStreamLen;
	public int[] mPGSStreamArray;

	public byte[] mSubtitleArray;
	public int mSubtitleArrayLen;

	public String[] mSubtitleClass;
	public int mSubtitleClassNum;

	public MediaSubtitle() {

	 	mSubtitleClassIdx = 0;
		mSubtitleStartPTS = 0;
		mSubtitleEndPTS = 0;
		mSubtitleCharSet = CHARACTERSET_NONE;
		mSubtitleString = new String();

		//mSubtitleArray = null;
		mSubtitleArrayLen = 0;

		mSubtitleClass = new String[50];
		mSubtitleClassNum = 0;

		mLocale = Locale.getDefault();
		Log.d(TAG, "Local is " + mLocale.toString());
		if ((mLocale.toString()).equals("zh_CN")){
			mLocaleString = new String("GB2312");
		} else if ((mLocale.toString()).equals("zh_TW")) {
			mLocaleString = new String("BIG5");
		} else if ((mLocale.toString()).equals("ar_EG") || (mLocale.toString()).equals("ar_IL")) {
			mLocaleString = new String("Windows-1256");
		} else if ((mLocale.toString()).equals("ko_KR")) {
			mLocaleString = new String("KSC5601");
		} else if ((mLocale.toString()).equals("af_ZA") //Afrikans
			|| (mLocale.toString()).equals("ca_ES") //Catalan
			|| (mLocale.toString()).equals("da_DK") //Danish
			|| (mLocale.toString()).equals("nl_BE") || (mLocale.toString()).equals("nl_NL") //Dutch
			|| (mLocale.toString()).equals("en_AU") || (mLocale.toString()).equals("en_CA") || (mLocale.toString()).equals("en_GB") //English
			|| (mLocale.toString()).equals("en_NZ") || (mLocale.toString()).equals("en_SG") /*|| (mLocale.toString()).equals("en_US"))*/
			|| (mLocale.toString()).equals("fi_FI") //Finnish
			|| (mLocale.toString()).equals("fr_BE") || (mLocale.toString()).equals("fr_CA") || (mLocale.toString()).equals("fr_CH") || (mLocale.toString()).equals("fr_FR") //French
			|| (mLocale.toString()).equals("de_AT") || (mLocale.toString()).equals("de_CH") || (mLocale.toString()).equals("de_DE") || (mLocale.toString()).equals("de_LI") //German
			|| (mLocale.toString()).equals("it_IT") || (mLocale.toString()).equals("it_CH") //Italian
			|| (mLocale.toString()).equals("nb_NO") //Norwegian
			|| (mLocale.toString()).equals("pt_BR") || (mLocale.toString()).equals("pt_PT") //Portuguese
			|| (mLocale.toString()).equals("es_ES") || (mLocale.toString()).equals("es_US") //Spanish
			|| (mLocale.toString()).equals("sv_SE")) { //Swendish
			mLocaleString = new String("Windows-1252");
		} else {
			//mLocaleString = new String("Windows-1252");
			mLocaleString = new String("KSC5601");
		}
	}

	public String getSubtitleString() {
		if (CHARACTERSET_ANSI == mSubtitleCharSet) {
			try {
				if (mSubtitleArrayLen > 0) {
					String ANSIString = new String(mSubtitleArray, 0, mSubtitleArrayLen, mLocaleString);
					return ANSIString;
				}
			} catch (UnsupportedEncodingException e) {
				// ignore
			}
		}
		else if (CHARACTERSET_UNICODE == mSubtitleCharSet || CHARACTERSET_UTF8 == mSubtitleCharSet) {
			return mSubtitleString;
	}

		return null;
	}

	public String getSubtitleClassName(int index) {
		if(index < mSubtitleClassNum && mSubtitleClass != null)
		{
			return mSubtitleClass[index];
		}

		return null;
	}

	public int getSubtitleClassDefaultIndex() {
		if ((mLocale.toString()).equals("zh_CN") || mLocale.toString().equals("zh_CN")) {
            if (mSubtitleClass != null && mSubtitleClass[1] != null && mSubtitleClass[1].equals("ZHCC")) {
				return 1;
			} else if (mSubtitleClass != null && mSubtitleClass[2] != null && mSubtitleClass[2].equals("ZHCC")) {
				return 2;
			}
		} else {
			if (mSubtitleClass != null && mSubtitleClass[1] != null && mSubtitleClass[1].equals("KRCC")) {
				return 1;
			} else if (mSubtitleClass != null && mSubtitleClass[2] != null && mSubtitleClass[2].equals("KRCC")) {
				return 2;
			}
		}

		return 0;
	}

	public void clearSubtitle() {
		mSubtitleClassIdx = 0;
		mSubtitleStartPTS = 0;
		mSubtitleEndPTS = 0;
		mSubtitleString = " ";
	}

	public void clearPGS() {
		mPGSStartTm = 0;
		mSrcPGSWidth = 0;
		mSrcPGSHeigth = 0;
		mDstPGSWidth = 0;
		mDstPGSHeigth = 0;
		mPGSOffsetX = 0;
		mPGSOffsetY = 0;
		mPGSStreamLen = 0;
	}
}

