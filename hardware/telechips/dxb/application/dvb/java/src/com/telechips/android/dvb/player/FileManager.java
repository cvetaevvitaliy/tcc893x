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

package com.telechips.android.dvb.player;

import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;

import java.io.File;
import java.io.FileInputStream;

import com.telechips.android.dvb.player.dvb.DB_CHANNEL_Data;

public class FileManager extends ChannelManager
{
	public FileManager(Context context)
	{
		super(context, null);

		DxbLog(D, "FileManager()");
	}

	public FileManager open() throws SQLException
	{
		DxbLog(D, "open()");

		if (mSQLiteDatabase == null)
		{
			mSQLiteDatabase = getWritableDatabase();
		}
		return this;
	}

	public void close()
	{
		DxbLog(D, "close()");

		if (mSQLiteDatabase != null)
		{
			synchronized(mSQLiteDatabase) {
				mSQLiteDatabase.close();
			}
			mSQLiteDatabase = null;
		}
	}

	private boolean isRecordedFile(String path) {
		DxbLog(D, "isRecordedFile(" + path + ")");

		int lastDot = path.lastIndexOf(".");
		if (lastDot < 0) return false;
		return "MTS".equals(path.substring(lastDot + 1).toUpperCase());
	}

	private void listUpdate(String dirPath)
	{
		DxbLog(D, "listUpdate(" + dirPath + ")");

		File f = new File(dirPath);
		File[] files = f.listFiles();

		if (files != null) {
			for (int i = 0; i < files.length; i++) {
				File file = files[i];

				if (file.isFile() && isRecordedFile(file.getName())) {
					insertChannel(file);
				}
			}
		}
	}

	@Override
	public void onDBInfoUpdate(int dbtype, DB_CHANNEL_Data dbinfo)
	{
		DxbLog(D, "onDBInfoUpdate(dbtype = " + dbtype + ")");

		// Audio Table
		deleteAllAudioPIDTable();
		for (int i = 0; i < dbinfo.dbService.uiTotalAudioPID; i++)
		{
			insertAudioPIDTable(dbinfo.dbService.uiServiceID, dbinfo.dbService.uiCurrentChannelNumber, dbinfo.dbService.uiCurrentCountryCode, dbinfo.dbAudio[i]);
		}

		// Subtitle Table
		deleteAllSubtitleTable();
		for (int i = 0; i < dbinfo.dbService.uiTotalCntSubtLang; i++)
		{
			insertSubtitleTable(dbinfo.dbService.uiServiceID, dbinfo.dbService.uiCurrentChannelNumber, dbinfo.dbService.uiCurrentCountryCode, dbinfo.dbSubtitle[i]);
		}
	}

	public int ByteToInteger(byte a, byte b, byte c, byte d)
	{
		int result = (a & 0xff);
		result = (result << 8) | (b & 0xff);
		result = (result << 8) | (c & 0xff);
		result = (result << 8) | (d & 0xff);

		return result;
	}

	public long insertChannel(File f)
	{
		DxbLog(D, "insertChannel()");

		byte[] buffer = new byte[144];
		try {
			FileInputStream in = new FileInputStream(f);
			in.skip(4);
			in.read(buffer, 0, 144);
			in.close();
		} catch (Exception e) {
			DxbLog(E, "[ERR:" + f.getName() + "]" + e);
			return -1;
		}

		SQLiteDatabase channel = mSQLiteDatabase;
		if (channel != null)
		{
			synchronized(channel) {
				if (channel.isOpen())
				{
					channel.execSQL(String.format("INSERT INTO services (uiCurrentChannelNumber, uiCurrentCountryCode, ucVersionNum, uiTStreamID, usOrig_Net_ID, usServiceID, PMT_PID, enType, fEIT_Schedule, fEIT_PresentFollowing, fCA_Mode_free, fAllowCountry, ucNameLen, strServiceName, uiAudio_PID, uiVideo_PID, ucVideo_IsScrambling, ucAudio_StreamType, ucVideo_StreamType, uiPCR_PID, uiTotalAudio_PID, ucLastTableID, uiLogicalChannel, usTotalCntSubtLang, uiTeletext_PID, ucCount_Teletext_Info, ucPMTVersionNum, ucSDTVersionNum, ucNITVersionNum) VALUES (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d,'%s', %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
							ByteToInteger(buffer[3], buffer[2], buffer[1], buffer[0]),			//uiCurrentChannelNumber
							ByteToInteger(buffer[7], buffer[6], buffer[5], buffer[4]),			//uiCurrentCountryCode
							ByteToInteger(buffer[11], buffer[10], buffer[9], buffer[8]),		//ucVersionNum
							ByteToInteger(buffer[15], buffer[14], buffer[13], buffer[12]),		//uiTStreamID
							ByteToInteger(buffer[19], buffer[18], buffer[17], buffer[16]),		//usOrig_Net_ID
							ByteToInteger(buffer[23], buffer[22], buffer[21], buffer[20]),		//usServiceID
							ByteToInteger(buffer[27], buffer[26], buffer[25], buffer[24]),		//PMT_PID
							ByteToInteger(buffer[31], buffer[30], buffer[29], buffer[28]),		//enType
							ByteToInteger(buffer[35], buffer[34], buffer[33], buffer[32]),		//fEIT_Schedule
							ByteToInteger(buffer[39], buffer[38], buffer[37], buffer[36]),		//fEIT_PresentFollowing
							ByteToInteger(buffer[43], buffer[42], buffer[41], buffer[40]),		//fCA_Mode_free
							ByteToInteger(buffer[47], buffer[46], buffer[45], buffer[44]),		//fAllowCountry
							ByteToInteger(buffer[51], buffer[50], buffer[49], buffer[48]),		//ucNameLen
							f.getName(),														//strServiceName
							ByteToInteger(buffer[87], buffer[86], buffer[85], buffer[84]),		//uiAudio_PID
							ByteToInteger(buffer[91], buffer[90], buffer[89], buffer[88]),		//uiVideo_PID
							ByteToInteger(buffer[95], buffer[94], buffer[93], buffer[92]),		//ucVideo_IsScrambling
							ByteToInteger(buffer[99], buffer[98], buffer[97], buffer[96]),		//ucAudio_StreamType
							ByteToInteger(buffer[103], buffer[102], buffer[101], buffer[100]),	//ucVideo_StreamType
							ByteToInteger(buffer[107], buffer[106], buffer[105], buffer[104]),	//uiPCR_PID
							ByteToInteger(buffer[111], buffer[110], buffer[109], buffer[108]),	//uiTotalAudio_PID
							ByteToInteger(buffer[115], buffer[114], buffer[113], buffer[112]),	//ucLastTableID
							ByteToInteger(buffer[119], buffer[118], buffer[117], buffer[116]),	//uiLogicalChannel
							ByteToInteger(buffer[123], buffer[122], buffer[121], buffer[120]),	//usTotalCntSubtLang
							ByteToInteger(buffer[127], buffer[126], buffer[125], buffer[124]),	//uiTeletext_PID
							ByteToInteger(buffer[131], buffer[130], buffer[129], buffer[128]),	//ucCount_Teletext_Info
							ByteToInteger(buffer[135], buffer[134], buffer[133], buffer[132]),	//ucPMTVersionNum
							ByteToInteger(buffer[139], buffer[138], buffer[137], buffer[136]),	//ucSDTVersionNum
							ByteToInteger(buffer[143], buffer[142], buffer[141], buffer[140])	//ucNITVersionNum
					));
				}
			}
		}
		return 0;
	}

	public Cursor getAllFiles(String dirPath)
	{
		DxbLog(D, "getAllFiles()");

		deleteAllChannel();
		listUpdate(dirPath);

		SQLiteDatabase channel = mSQLiteDatabase;
		if (channel != null)
		{
			synchronized(channel) {
				if (channel.isOpen())
				{
					return channel.query(CHANNEL_DATABASE_TABLE,
						new String[] {
										KEY_ROWID,
										KEY_CHANNEL_NUMBER,
										KEY_FREQUENCY,
										KEY_COUNTRY_CODE,
										KEY_SERVICE_TYPE,
										KEY_AUDIO_PID,
										KEY_VIDEO_PID,
										KEY_TELETEXT_PID,
										KEY_SERVICE_ID,
										KEY_CHANNEL_NAME,
										KEY_FREE_CA_MODE,
										KEY_SCRAMBLING,
										KEY_BOOKMARK,
										KEY_LOGICALCHANNEL
										},
							null,
							null, null, null, null, null);
				}
			}
		}
		return null;
	}
}

