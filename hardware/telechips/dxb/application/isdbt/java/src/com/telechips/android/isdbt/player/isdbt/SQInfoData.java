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

package com.telechips.android.isdbt.player.isdbt;

import android.util.Log;

public class SQInfoData
{
	private final static String TAG = "SQInfoData";

	public static final int SQINFO_ONE_SEG = 0; 	
	public static final int SQINFO_FULL_SEG = 1; 	
	public static final int SQINFO_DUAL_DECODE = 2;

	public static final int SQINFO_BASEBAND_TCC351x = 1;
	public static final int SQINFO_BASEBAND_TCC353x = 2;
	public static final int SQINFO_BASEBAND_DEFAULT = 4;

	public static final int SQINFO_JAPAN = 0;
	public static final int SQINFO_BRAZIL = 1;

	public int SQInfoArraySize;
	public int [] SQInfoArray;

	public int [] signal_length = new int[3];	// [0] - 1seg, [1] - fullseg
	public int antennaPercent[] = new int[3];
	public int tmcctransParamm[] = new int[3];
	public int tmccLock;
	public int [] mer = new int[3];
	public int [] vBer = new int[3];
	public int [] pcber = new int[3];
	public int [] tsper = new int[3];
	public int rssi;
	public int bbLoopGain;
	public int rfLoopGain;
	public int snr;
	public int frequency;

	private int mServiceType;
	private int mSeamlessMode;
	private int mTunerType;
	private int miStrongSignalLevel;
	private int mPrevSignalStrength;
	private int mCount;

	public SQInfoData(int iServiceType, int iTunerType, int iChangeLevel) 
	{
		mServiceType = iServiceType;
		mSeamlessMode = iServiceType;
		mTunerType = iTunerType;			
		miStrongSignalLevel = iChangeLevel;
		mPrevSignalStrength = 0;
		mCount = 0;
		SQInfoArraySize = 0;
	}

	private int getAntenaPercent(int vBer)
	{
		int iAntenaPercent = 0;

		if (360 < vBer) {			// VBER_ANTENNA_10
			iAntenaPercent = 0;
		}
		else if (100 < vBer) {		// VBER_ANTENNA_25
			iAntenaPercent = 25;
		}
		else if (60 < vBer) {		// VBER_ANTENNA_35
			iAntenaPercent = 35; 
		}
		else if(6 < vBer) {			// VBER_ANTENNA_55
			iAntenaPercent = 55;
		}
		else {
			iAntenaPercent = 100;
		}

		return iAntenaPercent;
	}

	private boolean setSQInfoValue()
	{
		if(SQInfoArray == null) {
			return false;
		}

		if(mServiceType == this.SQINFO_DUAL_DECODE) {
			mPrevSignalStrength = signal_length[this.SQINFO_FULL_SEG];
		}
		else {
			mPrevSignalStrength = signal_length[mServiceType];
		}

		switch(mTunerType)
		{
			case SQINFO_BASEBAND_TCC353x:
				if (SQInfoArraySize <= 8) {
					signal_length[0] = SQInfoArray[0];
					signal_length[1] = SQInfoArray[1];
				} else {

					antennaPercent[0] = SQInfoArray[0];
					antennaPercent[1] = SQInfoArray[1];
					antennaPercent[2] = SQInfoArray[2];
					tmcctransParamm[0] = SQInfoArray[3] & 0x000F;
					tmcctransParamm[1] = SQInfoArray[4] & 0x000F;
					tmcctransParamm[2] = SQInfoArray[5] & 0x000F;
					tmccLock = SQInfoArray[6];
					mer[0] = SQInfoArray[7];
					mer[1] = SQInfoArray[8];
					mer[2] = SQInfoArray[9];
					vBer[0] = SQInfoArray[10];
					vBer[1] = SQInfoArray[11];
					vBer[2] = SQInfoArray[12];
					pcber[0] = SQInfoArray[13];
					pcber[1] = SQInfoArray[14];
					pcber[2] = SQInfoArray[15];
					tsper[0] = SQInfoArray[16];
					tsper[1] = SQInfoArray[17];
					tsper[2] = SQInfoArray[18];
					rssi = SQInfoArray[19];
					bbLoopGain = SQInfoArray[20];
					rfLoopGain = SQInfoArray[21];
					snr = SQInfoArray[22];
					frequency = SQInfoArray[23];

					if (tmccLock != 0) {
						if (mCount < 5) {
							antennaPercent[0] = getAntenaPercent(vBer[0]);
							antennaPercent[1] = getAntenaPercent(vBer[1]);
							antennaPercent[2] = getAntenaPercent(vBer[2]);
						}

						if (tmcctransParamm[0] == 1) {
							signal_length[0] = antennaPercent[0];
						} else if (tmcctransParamm[1] == 1) {
							signal_length[0] = antennaPercent[1];
						} else if (tmcctransParamm[2] == 1) {
							signal_length[0] = antennaPercent[2];
						} else {
							signal_length[0] = 0;
						}

						if ((tmcctransParamm[0] > 1) && (tmcctransParamm[0] < 14)) {
							signal_length[1] = antennaPercent[0];
						} else if ((tmcctransParamm[1] > 1) && (tmcctransParamm[1] < 14)) {
							signal_length[1] = antennaPercent[1];
						} else if ((tmcctransParamm[2] > 1) && (tmcctransParamm[2] < 14)) {
							signal_length[1] = antennaPercent[2];
						} else {
							signal_length[1] = 0;
						}
					} else {
						signal_length[0] = antennaPercent[0];
						signal_length[1] = antennaPercent[1];
					}
				}
			break;

			case SQINFO_BASEBAND_TCC351x:
				signal_length[0] = SQInfoArray[0];
				signal_length[1] = SQInfoArray[1];
			break;

			default:
				signal_length[0] = 0;
				signal_length[1] = 0;
			break;
		}
	
		return true;
	}

	private int getSignalStrength()
	{
		int iSignalStrength = 0;

		if (SQInfoArray != null) {
			if(mServiceType == this.SQINFO_DUAL_DECODE)
				iSignalStrength = signal_length[this.SQINFO_FULL_SEG];
			else
				iSignalStrength = signal_length[mServiceType];
		}

		return iSignalStrength;
	}

	public void setServiceType(int iServiceType)
	{
		mServiceType = iServiceType;
		mPrevSignalStrength = 0;
		mCount = 0;
	}

	public void setSeamlessMode(int iSeamlessMode)
	{
		if(iSeamlessMode == 0) {
			mSeamlessMode = this.SQINFO_DUAL_DECODE;
		} else if(iSeamlessMode == 1) {
			mSeamlessMode = this.SQINFO_FULL_SEG;
		} else {
			mSeamlessMode = this.SQINFO_ONE_SEG;
		}
	}

	public void resetCount()
	{
		mCount = 0;
	}

	public String getSignalQuality()
	{
		String msg = "No Signal";
		boolean ret;

		ret = setSQInfoValue();
		if(ret == true) {
			switch(mTunerType)
			{
				case SQINFO_BASEBAND_TCC353x:
					msg = "A signal length[" + signal_length[0] + "]\n";
					msg += "B signal length[" + signal_length[1] + "]";
					if (SQInfoArraySize > 2) {
						msg += "\n";
						msg += "[Master] Lock[" +tmccLock + "] RSSI[" + rssi + "] RF[" + rfLoopGain + "] BB[" + bbLoopGain + "]\n";
						msg += "A VBER[" + String.format("%03d", vBer[0]) + "] TSPER[" + String.format("%05d", tsper[0]);
						msg += "] MER[" + String.format("%02d", mer[0]) + "] PCBER[" + String.format("%06d", pcber[0]) + "]\n"; 
						msg += "B VBER[" + String.format("%03d", vBer[1]) + "] TSPER[" + String.format("%05d", tsper[1]);
						msg += "] MER[" + String.format("%02d", mer[1]) + "] PCBER[" + String.format("%06d", pcber[1]) + "]\n"; 
						msg += "C VBER[" + String.format("%03d", vBer[2]) + "] TSPER[" + String.format("%05d", tsper[2]);
						msg += "] MER[" + String.format("%02d", mer[2]) + "] PCBER[" + String.format("%06d", pcber[2]) + "]"; 
						//msg += "B VBER[" + String.format("%03d", vBer[1]) + "] TSPER[" + String.format("%05d", tsper[1]) + "] MER[" + mer[1] + "] PCBER[" + pcber[1] + "]\n"; 
						//msg += "C VBER[" + string.format("%03d", vBer[2]) + "] TSPER[" + tsper[2] + "] MER[" + mer[2] + "] PCBER[" + pcber[2] + "]"; 
					}
				break;

				case SQINFO_BASEBAND_TCC351x:
					msg = "A signal length[" + signal_length[0] + "]\n";
					msg += "B signal length[" + signal_length[1] + "]";
				default:
				break;
			}
		}

		return msg;
	}

	public int getSignalLevel() 
	{
		int iCurSignalStrength;
		int iSignalLevel = 0;
		boolean ret;

		ret = setSQInfoValue();
		if(ret == true) {
			iCurSignalStrength = this.getSignalStrength();
			switch(mTunerType)
			{
				case SQINFO_BASEBAND_TCC353x:
				case SQINFO_BASEBAND_TCC351x:
					if(iCurSignalStrength > 70) {
						iSignalLevel =  4;
					} else if(iCurSignalStrength > 50) {
						iSignalLevel = 3;
					} else if(iCurSignalStrength > 30) {
						iSignalLevel = 2;
					} else if(iCurSignalStrength >= 10) {
						iSignalLevel = 1;
					} else {
						if((mServiceType == this.SQINFO_DUAL_DECODE) && (signal_length[this.SQINFO_ONE_SEG] > 10)) {
							if((mSeamlessMode == this.SQINFO_DUAL_DECODE) || (mSeamlessMode == this.SQINFO_ONE_SEG)) {
								iSignalLevel = 1;
							} else {
								iSignalLevel = 0;
							}
						} else {
							iSignalLevel = 0;
						}
					}
				break;

				default:
				break;
			}

			if(iSignalLevel > 4) {
				iSignalLevel = 4;
			}
		}
	
		mCount++;

		return iSignalLevel;
	}

	public boolean isStrongSignal()
	{
		int iCurSignalStrength;
		boolean ret;

		ret = setSQInfoValue();
		if(ret == true) {
			iCurSignalStrength =  this.getSignalStrength();

			switch(mTunerType)
			{
				case SQINFO_BASEBAND_TCC353x:
				case SQINFO_BASEBAND_TCC351x:
				{
					if(mPrevSignalStrength < miStrongSignalLevel && iCurSignalStrength < miStrongSignalLevel) {
						ret = false;
					}
				}
				break;

				default:
					if(mPrevSignalStrength < 26 && iCurSignalStrength < 26) {
						ret = false;
					}
				break;
			}
		}

		return ret;
	}
}

