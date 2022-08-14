/****************************************************************************
 * Copyright (C) 2013 Telechips Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions
 * andlimitations under the License.
 * ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <utils/Log.h>
#include <cutils/log.h>

#include <binder/ProcessState.h>

#include <semaphore.h>
#include <sqlite3.h>

#include <TDMBPlayer.h>
#include <DMBPlayerClient.h>

#define LOG_TAG "TPEG_MAIN"
#define TDMB_DATA_CHANNEL_TYPE	3

#define MANUAL_TUNE

using namespace android;

const unsigned int BAND_III_TABLE[] =
{
	174928,176640,178352,180064,181936,183648,185360,187072, 
	188928,190640,192352,194064,195936,197648,199360,201072, 
	202928,204640,206352,208064,209936,211648,213360,215072, 
	216928,218640,220352,222064,223936,225648,227360,229072, 
	230784,232496,234208,235776,237488,239200 
};

const unsigned int KOREA_BAND_TABLE[] = 
{
	175280,177008,178736,
	181280,183008,184736,
	187280,189008,190736,
	193280,195008,196736,
	199280,201008,202736,
	205280,207008,208736,
	211280,213008,214736
};

static sem_t g_status_sem;

class TPEGListener : public DxbPlayerClientListener
{
public:
	TPEGListener(int module_index)
	{
		m_module_index = module_index;
		m_p_tune_result = NULL;
		m_data_receive = 0;

		reset_ch_info();

		m_fp = NULL;
	};

	~TPEGListener()
	{
		set_dump_file(NULL);
	};

	bool set_dump_file(char *pFileName)
	{
		if(m_fp) {
			fclose(m_fp);
		}
		m_fp = NULL;

		if(pFileName) {
			m_fp = fopen(pFileName, "wb");
			if(m_fp == NULL) {
				printf("Fail to open %s\n", pFileName);
				return false;
			}
		}

		return true;
	}

	void set_tune_result(int *result)
	{
		m_p_tune_result = result;
	}

	void reset_ch_info()
	{
		m_ch_count = 0;
		memset(m_ch_info, 0x00, sizeof(m_ch_info));
	}

	int get_ch_count()
	{
		return m_ch_count;
	}

	TDMB_INFO* get_ch_info(int sel)
	{
		if(sel < 0 || sel >= m_ch_count) {
			printf("Invalid channel number(%d)\n", sel);
			return NULL;
		}

		return &m_ch_info[sel];
	}

	virtual void notify(int msg, int ext1, int ext2, const Parcel *obj = NULL)
	{
		//ALOGV("%s msg : %d", __FUNCTION__, msg);
		//printf("%s msg : %d\n", __FUNCTION__, msg);

		if(msg == 99)
		{
			ALOGE("EVENT_SERVICE_DIED receive");
			printf("EVENT_SERVICE_DIED receive\n");
			return;
		}

		if(ext1 != m_module_index) {
			//ALOGD("unhandled notify(%d-%d)", msg, ext1);
			return;
		}

		switch(msg)
		{
			case TDMBPlayer::EVENT_PREPARED:
				{
					//ALOGD("EVENT_PREPARED");
					sem_post(&g_status_sem);
				}
				break;
			case TDMBPlayer::EVENT_CHANNEL_UPDATE:
				{
					//ALOGD("EVENT_CHANNEL_UPDATE");
					TDMB_INFO *p_ch_info = (TDMB_INFO*)obj->data();
					memcpy(&m_ch_info[m_ch_count++], p_ch_info, sizeof(m_ch_info[0]));
				}
				break;
			case TDMBPlayer::EVENT_TUNNING_COMPLETE:
				{
					ALOGD("EVENT_TUNNING_COMPLETE");
					if(m_p_tune_result)
						*m_p_tune_result = ext2;
					sem_post(&g_status_sem);
				}
				break;
			case TDMBPlayer::EVENT_SEARCH_COMPLETE:
				{
					ALOGD("EVNET_SEARCH_COMPLETE");
						sem_post(&g_status_sem);
				}
				break;
			case TDMBPlayer::EVENT_DATA_UPDATE:
				{
					//ALOGD("EVENT_DATA_UPDATEA");
					
					int *pDataArg = (int*)obj->data();
					int length = *(pDataArg++);
					unsigned char *pdata = (unsigned char*)pDataArg;

					if(m_fp && length > 0 && pdata) {
						fwrite(pdata, 1, length, m_fp);
					}

					m_data_receive += length;
					printf("data receive : %ld\r", m_data_receive);
				}
				break;
			default:
				break;
		}
	};

protected:
	FILE *m_fp;
	int m_module_index;
	int *m_p_tune_result;
	unsigned long m_data_receive;
	int m_ch_count;
	TDMB_INFO m_ch_info[64];
};

char getInput_char(char *pPrintStr)
{
	char input = 0;

	if(pPrintStr) {
		printf("%s", pPrintStr);
	}

	while(1)
	{
		char ch = getchar();
		if(ch == '\n') {
			break;
		}
		else {
			input = ch;
		}
	}
	return input;
}

int getInput_int(char *pPrintStr, unsigned int min, unsigned int max)
{
	while(1)
	{
		int sel;
		unsigned int ch_num;
		char ch_buf[10];

		ch_num = 0;
		memset(ch_buf, 0, sizeof(ch_buf));

		printf("%s", pPrintStr);

		while(1)
		{
			char ch = getchar();
			if(ch == '\n') {
				break;
			}
			else if(ch < 0x30 || ch > 0x39) {
				ch_num = 0xFFFF;
			}
			else {
				if(ch_num < sizeof(ch_buf))
					ch_buf[ch_num++] = ch;
			}
		}

		if(ch_num == 0 || ch_num == 0xFFFF)
			sel = -1;
		else
			sel = atoi((const char*)ch_buf);

		//printf("sel : %d, min : %d, max : %d\n", sel, min, max);

		if(sel >= min && sel <= max)
			return sel;
		else
			printf("Invalid input\n");
	}

	return -1;
}

int readChannelDB(void *pChannelDB, int ChannelCount)
{
	int DataChCount = 0;
	int DataChDBList[64] = {0,};

#ifdef MANUAL_TUNE
	if(pChannelDB == NULL) {
		printf("Invalid ChannelDB\n");
		return -1;
	}

	printf("Data Channel list\n");		
	for(int i = 0; i < ChannelCount; i++)
	{
		TDMB_INFO *pInfo = &(((TDMB_INFO*)pChannelDB)[i]);
		if(pInfo && pInfo->Channel_Type == TDMB_DATA_CHANNEL_TYPE) {
			printf("\tch(%d)-", i);
			//printf("ensembleBand(%d)", pInfo->Ensemble_Band);
			//printf("ensembleFreq(%d)", pInfo->Ensemble_Freq);
			//printf("ensembleId(%d)", pInfo->Ensemble_Id);
			//printf("serviceId(%d)", pInfo->Service_Id);
			printf("serviceLabel(%s)", pInfo->Service_Label);
			//printf("channelId(%d)", pInfo->Channel_Id);
			//printf("channelLabel(%s)", pInfo->Channel_Label);
			//printf("channelType(%d)", pInfo->Channel_Type);
			printf("\n");

			DataChDBList[DataChCount++] = i;
		}
	}
#else
	sqlite3 *handle = NULL;
	int err = sqlite3_open((char*)pChannelDB, &handle);
	if(err != SQLITE_OK) {
		printf("Fail to open Channel DB(%d)\n", err);
		return -1;
	}
	
	sqlite3_stmt *stmt;
	int rc;
	int count = 1;
	char szSQL[1024] = { 0 };

	sprintf(szSQL, "SELECT ensembleFreq, serviceName, serviceID, channelID, type, bitrate FROM (SELECT * FROM channels)");
	sqlite3_prepare(handle, szSQL, strlen(szSQL), &stmt, NULL);

	printf("Data Channel list\n");
	do {
		rc = sqlite3_step(stmt);
		if(rc == SQLITE_ROW) {
			if(sqlite3_column_int(stmt, 3) == TDMB_DATA_CHANNEL_TYPE) { // display data channel only
				printf("\tch(%d)-", count);
				printf("ensembleFreq(%d)", sqlite3_column_int(stmt, 0));
				printf("serviceName(%s)", sqlite3_column_text(stmt, 1));
				//printf("serviceID(%d)", sqlite3_column_int(stmt, 2));
				//printf("channelID(%d)", sqlite3_column_int(stmt, 3));
				//printf("type(%d)", sqlite3_column_int(stmt, 4));
				//printf("bitrate(%d)", sqlite3_column_int(stmt, 5));
				printf("\n");
				
				DataChDBList[DataChCount++] = count;
			}
			count++;
		}
	} while(rc == SQLITE_ROW);
		
	sqlite3_finalize(stmt);
	sqlite3_close(handle);
#endif

	if(DataChCount <= 0)
	{
		printf("Data channel not found\n");
	}
	else
	{
		while(1)
		{
			int cnt = 0;
			int select = getInput_int("select data channel index : ", DataChDBList[0], DataChDBList[DataChCount-1]);
			for(cnt = 0; cnt < DataChCount; cnt++)
			{
				if(DataChDBList[cnt] == select) {
					break;
				}
			}

			if(cnt == DataChCount) {
				printf("Invalid data channel index(%d)\n", select);
			}
			else {
				return select;
			}
		}
	}

	return -1;
}

int readArgs(int argc, char *argv[], int *pModuleIndex, int *pBBType, int *pBand)
{
	if(argc >= 2 && strcmp(argv[1],"menu") == 0) {
		*pModuleIndex = getInput_int("select module index(0 ~ 1) : ", 0, 1);
		*pBBType = getInput_int("select base band(1 ~ 4)\n\t1=TCC3150\n\t2=TCC351X CSPI+STS\n\t3=TCC351X I2C+STS\n\t4=TCC3171 I2C+STS\n : ", 1, 4);
		*pBand = getInput_int("select band (1 ~ 5)\n\t1=Korea Band\n\t2=Band III\n\t3=L Band\n\t4=Canada Band\n\t5=China Band III : ", 1, 5);
	}

	printf("--------------------------------------\n");
	if(*pModuleIndex >= 2) {
		printf("Invalid module index(%d)\n", *pModuleIndex);
		return -1;
	}
	else {
		printf("Module Index = %d\n", *pModuleIndex);
	}

	switch(*pBBType)
	{
		case 1:
			printf("Base Band = TCC3150\n");
			break;
		case 2:
			printf("Base Band = TCC351X CSPI+STS\n");
			break;
		case 3:
			printf("Base Band = TCC351X I2C+STS\n");
			break;
		case 4:
			printf("Base Band = TCC3171 I2C+STS\n");
			break;
		default:
			printf("Invalid base band(%d)\n", *pBBType);
			return -1;
	}

	switch(*pBand)
	{
		case 1:
			printf("Band = Korea Band\n");
			break;
		case 2:
			printf("Band = Band III\n");
			break;
		case 3:
			printf("Band = L Band\n");
			break;
		case 4:
			printf("Band = Canada Band\n");
			break;
		case 5:
			printf("Band = China Band III\n");
			break;
		default:
			printf("Invalid band(%d)\n", *pBand);
			return -1;
	}

	printf("--------------------------------------\n");
	return 0;
}

int main(int argc, char *argv[])
{
	char ChDBPath[] = "/data/data/com.telechips.android.tdmb/databases/TDMB2.db";
	char DumpPath[] ="/storage/sdcard0/";
	int ChIndex = 0;

	// manual tune variables
	unsigned int *pFreqTable = NULL;
	unsigned int FreqTableSize = 0;
	unsigned int Frequency;

	// input argument variables
	int ArgModuleIndex = 1;
	int ArgBBType = 4;
	int ArgBand = 1;

	sp<DMBPlayerClient> player = NULL;
	sp<TPEGListener> listener = NULL;

	int cnt = 0;

	//ALOGD("Start of tpegmain\n");
	
	if(readArgs(argc, argv, &ArgModuleIndex, &ArgBBType, &ArgBand) < 0) {
		return -1;
	}

	sem_init(&g_status_sem, 0, 0);

	ProcessState::self()->startThreadPool();

	player = new DMBPlayerClient();
	if(player == NULL)
	{
		printf("Fail to create DMBPlayerClient instance\n");
		goto _error_return_;
	}
	
	listener = new TPEGListener(ArgModuleIndex);
	if(listener == NULL) {
		printf("Fail to create TPEGListener instance\n");
		goto _error_return_;
	}

	player->setListener(listener);
	
	// 0 = none
	// 1 = TCC3150
	// 2 = TCC351X CSPI+STS
	// 3 = TCC351X I2C+STS
	// 4 = TCC317X I2C+STS
	if(player->setBBModuleIndex(ArgBBType, ArgModuleIndex) != (status_t)OK)
	{
		printf("TDMBPlayer setBBModuleIndex fail\n");
		goto _error_return_;
	}

	if(player->playprepare(ArgBBType, NULL) != (status_t)OK)
	{
		printf("TDMBPlayer playprepare fail\n");
		goto _error_return_;
	}

	// wait TDMBPlayer::EVENT_PREPARED
	sem_wait(&g_status_sem);

	// 1 = Korea Band
	// 2 = Band III
	// 3 = L Band
	// 4 = Canada Band
	// 5 = China Band III
	// 6 = Korea Band only Seoul
	if(player->start(ArgBand) != (status_t)OK)
	{
		printf("TDMBPlayer start fail\n");
		goto _error_return_;
	}

	sleep(1);

#ifdef MANUAL_TUNE
	switch(ArgBand)
	{
		case 0x01:
			pFreqTable = (unsigned int*)KOREA_BAND_TABLE;
			FreqTableSize = sizeof(KOREA_BAND_TABLE)/sizeof(KOREA_BAND_TABLE[0]);
			break;
		case 0x02:
			pFreqTable = (unsigned int*)BAND_III_TABLE;
			FreqTableSize = sizeof(BAND_III_TABLE)/sizeof(BAND_III_TABLE[0]);
			break;
		default:
			printf("Not defined Band\n");
			printf("Please insert frequency table\n");

	}

	if(pFreqTable == NULL || FreqTableSize == 0)
		goto _error_return_;

	while(1)
	{
		unsigned int i;

		printf("Frequency Table");
		for(i = 0; i < FreqTableSize; i++)
		{
			if((i % 3) == 0)
				printf("\n");
			printf("\t%d", pFreqTable[i]);
		}
		printf("\n");

		Frequency = getInput_int("type frequency : ", pFreqTable[0], pFreqTable[FreqTableSize-1]);
		for(i = 0; i < FreqTableSize; i++)
		{
			if(pFreqTable[i] == Frequency)
				break;
		}

		if(i == FreqTableSize)	// input frequency is not available
		{
			printf("Invalid frequency : %d\n", Frequency);
		}
		else
		{
			listener->reset_ch_info();
			player->manual_search(Frequency);

			// wait TDMBPlayer::EVENT_SEARCH_COMPLETE
			sem_wait(&g_status_sem);

			int ch_count = listener->get_ch_count();
			if(ch_count > 0) {
				printf("manual search success\n");

				ChIndex = readChannelDB(listener->get_ch_info(0), ch_count);
				printf("channel index : %d\n", ChIndex);
				if(ChIndex > 0) {
					TDMB_INFO *pInfo = listener->get_ch_info(ChIndex);
					if(pInfo) {
						int count = 0;
						int result = 0;
						listener->set_tune_result(&result);

						printf("%s searching... \n", pInfo->Service_Label);
						while(count < 10)
						{
							count++;
							if(player->setManualChannel((unsigned char*)pInfo, sizeof(TDMB_INFO)) != (status_t)OK) {
								//printf("TDMBPlayer setMannualChannel fail\n", pInfo->Ensemble_Freq, pInfo->Service_Id, pInfo->Channel_Id);
								continue;
							}
								
							// wait TDMBPlayer::EVENT_TUNNING_COMPLETE
							sem_wait(&g_status_sem);
							
							if(result > 0) {
								break;
							}
						}
						
						if(result > 0)
							printf("OK\n");
						else
							printf("Fail\n");

						if(result > 0) {
							char filename[PATH_MAX];
							sprintf(filename, DumpPath, strlen(DumpPath));
							sprintf(filename + strlen(DumpPath), pInfo->Service_Label, strlen(pInfo->Service_Label));
							sprintf(filename + strlen(DumpPath) + strlen(pInfo->Service_Label), ".bin", strlen(".bin"));
							printf("dump file name : %s\n", filename);
							listener->set_dump_file(filename);
							break;
						}

						listener->set_tune_result(NULL);
					}
					//break;
				}
			}
			else {
				printf("manual search fail\n");
			}
		}

		char ch = getInput_char("continue?(Y/N) : ");
		if(ch == 'N' || ch == 'n') {
			goto _error_return_;
		}
	}
#else
	ChIndex = readChannelDB(ChDBPath, 0);
	if(ChIndex <= 0) {
		goto _error_return_;
	}
	else {
		char filename[PATH_MAX];
		sprintf(filename, DumpPath, strlen(DumpPath));
		sprintf(filename + strlen(DumpPath), "TDMBData.bin", strlen("TDMBData.bin"));
		printf("dump file name : %s\n", filename);
		listener->set_dump_file(filename);
	}

	if(player->setChannelIndex(ChIndex, TDMB_DATA_CHANNEL_TYPE, 0) != (status_t)OK) {
		printf("TDMBplayer setChannel fail\n");
		goto _error_return_;
	}
#endif
	
	printf("channel start..  press ENTER key to stop!!!\n");
	while(1)
	{
#if 1
		struct timeval tv;
		fd_set fds;

		FD_ZERO(&fds);
		FD_SET(STDIN_FILENO, &fds);
		
		tv.tv_sec = 0;
		tv.tv_usec = 100000;	// 100ms
		
		select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
		if(FD_ISSET(STDIN_FILENO, &fds) != 0) {
			break;
		}
		else {
			cnt++;
			if(cnt >= 10)	// 1s
			{
				int iSignalStrength[14]= { 0 };
				if(player->getSignalStrength(iSignalStrength) != 0) {
					 ALOGE("getSignalStrength failed");
				}
				else {
					ALOGD("[ANTENNA(%d), SNR(%d), PCBER(%d)]", iSignalStrength[0], iSignalStrength[1], iSignalStrength[2]);
				}
				
				cnt = 0;
			}
		}
#else
		if(getchar())
			break;
#endif
	}

	player->playstop();
	player->setListener(0);
	player->disconnect();

	sem_destroy(&g_status_sem);
	
	//ALOGD("End of tepgmain");
	return 0;

_error_return_:
	//ALOGE("_error_return_");
	if(player != 0 )
	{
		player->playstop();
		player->setListener(0);
		player->disconnect();
	}

	sem_destroy(&g_status_sem);
	return 0;
}
