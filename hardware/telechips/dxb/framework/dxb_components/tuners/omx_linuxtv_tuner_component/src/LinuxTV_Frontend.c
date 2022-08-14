/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions andlimitations under the License.

****************************************************************************/

/****************************************************************************

Revision History

****************************************************************************

****************************************************************************/
#define LOG_TAG	"LINUXTV_FRONTEND"

#include <utils/Log.h>

#include <sys/ioctl.h>
#include <fcntl.h>

#include <LinuxTV_Frontend.h>

//#define DBG_MSG
#ifdef DBG_MSG
#define DEBUG_MSG(msg...)	ALOGD(msg)
#else
#define DEBUG_MSG(msg...)
#endif

#define FRONTEND    "/dev/dvb0.frontend1"
#define MAX_CMD       32

static int gSystemType = SYS_DVBS2;
static int ghFE = -1;
static struct dtv_properties gTVPs;
struct dvb_frontend_info gInfo;

#define FE_SET_USER_COMMAND 200
#define FE_GET_USER_COMMAND 201

#define DTV_BLIND_SCAND_START		DTV_MAX_COMMAND + 1
#define DTV_BLIND_SCAND_CANCEL		DTV_MAX_COMMAND + 2
#define DTV_BLIND_SCAND_RESET		DTV_MAX_COMMAND + 3
#define DTV_BLIND_SCAND_GET_STATE	DTV_MAX_COMMAND + 4
#define DTV_BLIND_SCAND_GET_INFO	DTV_MAX_COMMAND + 5

int LinuxTV_Frontend_Open()
{
	DEBUG_MSG("[%s:%d]", __func__, ghFE);

	if (ghFE >= 0) return -1;

	ghFE = open(FRONTEND, O_RDWR);
	gTVPs.num = 0;
	gTVPs.props = malloc(sizeof(struct dtv_property) * MAX_CMD);

	memset(&gInfo, 0x0, sizeof(struct dvb_frontend_info));
	if (ioctl(ghFE, FE_GET_INFO, &gInfo) == 0)
	{
		ALOGI("Name        : %s", gInfo.name);
		ALOGI("Type        : %d", gInfo.type);
		ALOGI("Frequency   : %d ~ %d (Tolerance:%d, Step:%d)", gInfo.frequency_min, gInfo.frequency_max, gInfo.frequency_tolerance, gInfo.frequency_stepsize);
		ALOGI("Symbol Rate : %d ~ %d (Tolerance:%d)", gInfo.symbol_rate_min, gInfo.symbol_rate_max, gInfo.symbol_rate_tolerance);
		ALOGI("Noti Delay  : %d", gInfo.notifier_delay);
		ALOGI("Caps        : %d", gInfo.caps);
	}

	return 0;
}

int LinuxTV_Frontend_Close()
{
	DEBUG_MSG("[%s:%d]", __func__, ghFE);

	if (ghFE < 0) return -1;

	free(gTVPs.props);
	close(ghFE);
	ghFE = -1;

	return 0;
}

int LinuxTV_Frontend_DiSEqC_ResetOverload()
{
	DEBUG_MSG("[%s:%d]", __func__, ghFE);

	int err = -1;

	if (ghFE < 0) return err;

	err = ioctl(ghFE, FE_DISEQC_RESET_OVERLOAD, 0);

	return err;
}

int LinuxTV_Frontend_DiSEqC_SendCMD(unsigned char *msg, unsigned int len)
{
	DEBUG_MSG("[%s:%d]len = %d", __func__, ghFE, len);

	int err = -1;
	struct dvb_diseqc_master_cmd cmd;

	if (ghFE < 0) return err;

	memcpy(cmd.msg, msg, len);
	cmd.msg_len = len;

	err = ioctl(ghFE, FE_DISEQC_SEND_MASTER_CMD, &cmd);

	return err;
}

int LinuxTV_Frontend_DiSEqC_GetReply(unsigned char *msg)
{
	DEBUG_MSG("[%s:%d]", __func__, ghFE);

	int err = -1;
	struct dvb_diseqc_slave_reply reply;

	if (ghFE < 0) return err;

	err = ioctl(ghFE, FE_DISEQC_RECV_SLAVE_REPLY, &reply);
	if (err == 0)
	{
		memcpy(msg, reply.msg, reply.msg_len);		
	}

	return (err) ? err : reply.msg_len;
}

int LinuxTV_Frontend_DiSEqC_SendBurst(int arg)
{
	DEBUG_MSG("[%s:%d] arg = %d", __func__, ghFE, arg);

	int err = -1;

	DEBUG_MSG("[%s:%d] arg = %d", __func__, ghFE, arg);

	if (ghFE < 0) return err;

	err = ioctl(ghFE, FE_DISEQC_SEND_BURST, arg); // fe_sec_mini_cmd_t

	return err;
}

int LinuxTV_Frontend_SetTone(int arg)
{
	DEBUG_MSG("[%s:%d] arg = %d", __func__, ghFE, arg);

	int err = -1;

	if (ghFE < 0) return err;

	err = ioctl(ghFE, FE_SET_TONE, arg); // fe_sec_tone_mode_t

	return err;
}

int LinuxTV_Frontend_SetVoltage(int arg)
{
	DEBUG_MSG("[%s:%d] arg = %d", __func__, ghFE, arg);

	int err = -1;

	if (ghFE < 0) return err;

	err = ioctl(ghFE, FE_SET_VOLTAGE, arg); // fe_sec_voltage_t

	return err;
}

int LinuxTV_Frontend_EnableHighLNBVoltage(int arg)
{
	DEBUG_MSG("[%s:%d] arg = %d", __func__, ghFE, arg);

	int err = -1;

	if (ghFE < 0) return err;

	err = ioctl(ghFE, FE_ENABLE_HIGH_LNB_VOLTAGE, arg);

	return err;
}

int LinuxTV_Frontend_GetStatus()
{
	DEBUG_MSG("[%s:%d]", __func__, ghFE);

	int err = -1;
	fe_status_t status;

	if (ghFE < 0) return err;

	err = ioctl(ghFE, FE_READ_STATUS, &status);

	return (err) ? err : status;
}

int LinuxTV_Frontend_GetBER()
{
	DEBUG_MSG("[%s:%d]", __func__, ghFE);

	int err = -1;
	int ber;

	if (ghFE < 0) return err;

	err = ioctl(ghFE, FE_READ_BER, &ber);

	return (err) ? err : ber;
}

int LinuxTV_Frontend_GetSignalStrength()
{
	DEBUG_MSG("[%s:%d]", __func__, ghFE);

	int err = -1;
	unsigned short strength;

	if (ghFE < 0) return err;

	err = ioctl(ghFE, FE_READ_SIGNAL_STRENGTH, &strength);

	return (err) ? err : (int)strength;
}

int LinuxTV_Frontend_GetSNR()
{
	DEBUG_MSG("[%s:%d]", __func__, ghFE);

	int err = -1;
	unsigned short snr;

	if (ghFE < 0) return err;

	err = ioctl(ghFE, FE_READ_SNR, &snr);

	return (err) ? err : (int)snr;
}

int LinuxTV_Frontend_GetUncorrectedBlocks()
{
	DEBUG_MSG("[%s:%d]", __func__, ghFE);

	int err = -1;
	int uncorrected;

	if (ghFE < 0) return err;

	err = ioctl(ghFE, FE_READ_UNCORRECTED_BLOCKS, &uncorrected);

	return (err) ? err : uncorrected;
}

int LinuxTV_Frontend_SetFrontend(unsigned int uiFreqKHz, unsigned int uiBWKHz, unsigned int uiLock)
{
	DEBUG_MSG("[%s:%d] uiFreqKHz = %d, uiBWKHz = %d, uiLock = %d", __func__, ghFE, uiFreqKHz, uiBWKHz, uiLock);

	int err = -1;
	unsigned int uiTimeth, uiCounter;
	struct dvb_frontend_parameters params;

	if (ghFE < 0) return err;

	if (gSystemType == SYS_DVBS2)
	{
		params.u.qam.symbol_rate = uiBWKHz;

		//Channel lock time increase while symbol rate decrease.Give the max waiting time for different symbolrates.
		if(uiBWKHz < 5000000)
		{
			uiTimeth = 10000*2;//5000*2;       //The max waiting time is 5000ms,considering the IQ swapped status the time should be doubled.
		}
		else if(uiBWKHz < 10000000)
		{
	        uiTimeth = 1200*2;//600*2;        //The max waiting time is 600ms,considering the IQ swapped status the time should be doubled.
		}
		else
		{
	        uiTimeth = 500*2;//250*2;        //The max waiting time is 250ms,considering the IQ swapped status the time should be doubled.
		} 
	}
	else
	{
		switch(uiBWKHz)
		{
			case 5000:
				params.u.ofdm.bandwidth = BANDWIDTH_5_MHZ;
				break;
			case 6000:
				params.u.ofdm.bandwidth = BANDWIDTH_6_MHZ;
				break;
			case 7000:
				params.u.ofdm.bandwidth = BANDWIDTH_7_MHZ;
				break;
			case 8000:
				params.u.ofdm.bandwidth = BANDWIDTH_8_MHZ;
				break;
		}
		uiTimeth = 500;
	}

	params.frequency = uiFreqKHz;

	err = ioctl(ghFE, FE_SET_FRONTEND, &params);

	if (uiLock)
	{
		uiCounter = uiTimeth/10;
		do
		{
			usleep(10000);    //Wait 10ms for demod to lock the channel.
			if (LinuxTV_Frontend_GetStatus() != 32)
				break;
		}while(--uiCounter);
		if(0 == uiCounter)
		{
			err = -1;
		}
	}

	return err;
}

int LinuxTV_Frontend_GetFrontend()
{
	DEBUG_MSG("[%s:%d]", __func__, ghFE);

	int err = -1;
	struct dvb_frontend_parameters params;

	if (ghFE < 0) return err;

	err = ioctl(ghFE, FE_GET_FRONTEND, &params);

	return err;
}

int LinuxTV_Frontend_SetTuneMode(int arg)
{
	DEBUG_MSG("[%s:%d] arg = %d", __func__, ghFE, arg);

	int err = -1;

	if (ghFE < 0) return err;

	err = ioctl(ghFE, FE_SET_FRONTEND_TUNE_MODE, arg);

	return err;
}

int LinuxTV_Frontend_GetEvent()
{
	DEBUG_MSG("[%s:%d]", __func__, ghFE);

	int err = -1;
	struct dvb_frontend_event event;

	if (ghFE < 0) return err;

	err = ioctl(ghFE, FE_GET_EVENT, &event);

	return err;
}

int LinuxTV_Frontend_DishNetwork_SendLegacyCMD(int arg)
{
	DEBUG_MSG("[%s:%d] arg = %d", __func__, ghFE, arg);

	int err = -1;

	if (ghFE < 0) return err;

	err = ioctl(ghFE, FE_DISHNETWORK_SEND_LEGACY_CMD, arg);

	return err;
}

static int LinuxTV_Frontend_SetProperties()
{
	DEBUG_MSG("[%s:%d]", __func__, ghFE);

	int err = -1;

	if (ghFE >= 0 && gTVPs.num > 0)
	{
		err = ioctl(ghFE, FE_SET_PROPERTY, &gTVPs);
	}
	gTVPs.num = 0;

	return err;
}

static int LinuxTV_Frontend_GetProperties()
{
	DEBUG_MSG("[%s:%d]", __func__, ghFE);

	int err = -1;

	if (ghFE >= 0 && gTVPs.num > 0)
	{
		err = ioctl(ghFE, FE_GET_PROPERTY, &gTVPs);
	}
	gTVPs.num = 0;

	return err;
}

static int LinuxTV_Frontend_SetPropCMD(unsigned int uiCMD, unsigned int uiData)
{
	DEBUG_MSG("[%s:%d] uiCMD = %d, uiData = %d", __func__, ghFE, uiCMD, uiData);

	struct dtv_property *tvp;

	if (gTVPs.num >= MAX_CMD)
		return -1;

	tvp = gTVPs.props + gTVPs.num;
	gTVPs.num++;

	memset(tvp, 0x00, sizeof(struct dtv_property));
	tvp->cmd = uiCMD;
	tvp->u.data = uiData;

	return 0;
}

int LinuxTV_Frontend_GetSignalQuality()
{
	DEBUG_MSG("[%s:%d]", __func__, ghFE);

	int sq = LinuxTV_Frontend_GetSNR() * 4;
	if (sq > 100) sq = 100;
	return sq;
}

int LinuxTV_Frontend_SetAntennaPower(int arg)
{
	DEBUG_MSG("[%s:%d] arg = %d", __func__, ghFE, arg);
	return 0;
}

static int LinuxTV_Frontend_SetUserCommand()
{
	DEBUG_MSG("[%s:%d]", __func__, ghFE);

	int err = -1;

	if (ghFE >= 0 && gTVPs.num > 0)
	{
		err = ioctl(ghFE, FE_SET_USER_COMMAND, &gTVPs);
	}
	gTVPs.num = 0;

	return err;
}

static int LinuxTV_Frontend_GetUserCommand()
{
	DEBUG_MSG("[%s:%d]", __func__, ghFE);

	int err = -1;

	if (ghFE >= 0 && gTVPs.num > 0)
	{
		err = ioctl(ghFE, FE_GET_USER_COMMAND, &gTVPs);
	}
	gTVPs.num = 0;

	return err;
}

int LinuxTV_Frontend_BlindScan_Start()
{
	DEBUG_MSG("[%s:%d]", __func__, ghFE);

	int err;

	err = LinuxTV_Frontend_SetPropCMD(DTV_BLIND_SCAND_START, 0);
	if (err != 0)
		return err;

	return LinuxTV_Frontend_SetUserCommand();
}

int LinuxTV_Frontend_BlindScan_Cancel()
{
	DEBUG_MSG("[%s:%d]", __func__, ghFE);

	int err;

	err = LinuxTV_Frontend_SetPropCMD(DTV_BLIND_SCAND_CANCEL, 0);
	if (err != 0)
		return err;

	return LinuxTV_Frontend_SetUserCommand();
}

int LinuxTV_Frontend_BlindScan_Reset()
{
	DEBUG_MSG("[%s:%d]", __func__, ghFE);

	int err;

	err = LinuxTV_Frontend_SetPropCMD(DTV_BLIND_SCAND_RESET, 0);
	if (err != 0)
		return err;

	return LinuxTV_Frontend_SetUserCommand();
}

int LinuxTV_Frontend_BlindScan_GetState(int *state)
{
	DEBUG_MSG("[%s:%d]", __func__, ghFE);

	int err;

	err = LinuxTV_Frontend_SetPropCMD(DTV_BLIND_SCAND_GET_STATE, 0);
	if (err != 0)
		return err;
	err = LinuxTV_Frontend_GetUserCommand();
	if (err != 0)
		return err;

	*state = gTVPs.props[0].u.data;

	return 0;
}

int LinuxTV_Frontend_BlindScan_GetInfo(int *percent, int *index, int *freqMHz, int *symKHz)
{
	DEBUG_MSG("[%s:%d]", __func__, ghFE);

	int err;

	err = LinuxTV_Frontend_SetPropCMD(DTV_BLIND_SCAND_GET_INFO, 0);
	if (err != 0)
		return err;
	err = LinuxTV_Frontend_GetUserCommand();
	if (err != 0)
		return err;

	*percent = gTVPs.props[0].u.buffer.len;
	*index = gTVPs.props[0].u.buffer.reserved1[0];
	*freqMHz = gTVPs.props[0].u.buffer.reserved1[1];
	*symKHz = gTVPs.props[0].u.buffer.reserved1[2];

	return 0;
}

