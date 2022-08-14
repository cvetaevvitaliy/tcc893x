/****************************************************************************
Copyright (C) 2013 Telechips Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
****************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>

//#include <modules/cec/cec.h>

#include "libcec.h"

#include <utils/Log.h>

#define CEC_DEBUG 1
#define LOG_TAG 		"LIBCEC"

#if CEC_DEBUG
#define printf(args...)    ALOGE(args)
#else
#define printf(args...)
#endif


/**
 * @def CEC_DEVICE_NAME
 * Defines simbolic name of the CEC device.
 */
#define CEC_DEVICE_NAME         "/dev/CEC"

static struct {
    enum CECDeviceType devtype;
    unsigned char laddr;
} laddresses[] = {
    { CEC_DEVICE_RECODER, 1  },
    { CEC_DEVICE_RECODER, 2  },
    { CEC_DEVICE_TUNER,   3  },
    { CEC_DEVICE_PLAYER,  4  },
    { CEC_DEVICE_AUDIO,   5  },
    { CEC_DEVICE_TUNER,   6  },
    { CEC_DEVICE_TUNER,   7  },
    { CEC_DEVICE_PLAYER,  8  },
    { CEC_DEVICE_RECODER, 9  },
    { CEC_DEVICE_TUNER,   10 },
    { CEC_DEVICE_PLAYER,  11 },
};

#if CEC_DEBUG
inline static void CECPrintFrame(unsigned char *buffer, unsigned int size);
#endif

static int fd = -1;


/**
 * Open device driver and assign CEC file descriptor.
 *
 * @return  If success to assign CEC file descriptor, return 1; otherwise, return 0.
 */
int CECOpen(void)
{
    int res = 1;

    printf("opening %s...\n", CEC_DEVICE_NAME);
    if (fd == -1 && (fd = open(CEC_DEVICE_NAME, O_RDWR)) < 0) {
        printf("Can't open %s!\n", CEC_DEVICE_NAME);
        res = 0;
    }

    return res;
}

/**
 * Close CEC file descriptor.
 *
 * @return  If success to close CEC file descriptor, return 1; otherwise, return 0.
 */
int CECClose(void)
{
    int res = 1;

    if (fd != -1) {
        printf("closing...\n");
        if (close(fd) != 0) {
            printf("close() failed!\n");
            res = 0;
        }
        fd = -1;
    }

    return res;
}

/**
 * Start CEC
 *
 * @return  If success to start CEC, return 1; otherwise, return 0.
 */
int CECStart(void)
{
    if (ioctl(fd, CEC_IOC_START, NULL)) {
        printf("ioctl(CEC_IOC_START) failed!\n");
        return 0;
    }

    return 1;
}

/**
 * Stop CEC
 *
 * @return  If success to stop CEC, return 1; otherwise, return 0.
 */
int CECStop(void)
{
    if (ioctl(fd, CEC_IOC_STOP, NULL)) {
        printf("ioctl(CEC_IOC_STOP) failed!\n");
        return 0;
    }

    return 1;
}

int CECGetReumeStatus(void)
{
	unsigned int resume_status;
	if( ioctl(fd, CEC_IOC_GETRESUMESTATUS, &resume_status)) {
		printf("ioctl(CEC_IOC_GETRESUMESTATUS) failed!\n");
		return 0;
	}

	return resume_status;
}

int CECClrResuemStatus(void)
{
	unsigned resume_status = 0;
	if( ioctl(fd, CEC_IOC_CLRRESUMESTATUS, &resume_status)) {
		printf("ioctl(CEC_IOC_GETRESUMESTATUS) failed!\n");
		return 0;
	}

	return 1;
}


/**
 * Allocate logical address.
 *
 * @param paddr   [in] CEC device physical address.
 * @param devtype [in] CEC device type.
 *
 * @return new logical address, or 0 if an arror occured.
 */
int CECAllocLogicalAddress(int paddr, enum CECDeviceType devtype)
{
    unsigned char laddr = CEC_LADDR_UNREGISTERED;
    int i = 0;

    if (fd == -1) {
        printf("open device first!\n");
        return 0;
    }

    if (CECSetLogicalAddr(laddr) < 0) {
        printf("CECSetLogicalAddr() failed!\n");
        return 0;
    }

    if (paddr == CEC_NOT_VALID_PHYSICAL_ADDRESS) {
        return CEC_LADDR_UNREGISTERED;
    }

    /* send "Polling Message" */
    while (i < sizeof(laddresses)/sizeof(laddresses[0])) {
        if (laddresses[i].devtype == devtype) {
            unsigned char _laddr = laddresses[i].laddr;
            unsigned char message = ((_laddr << 4) | _laddr);
            if (CECSendMessage(&message, 1) != 1) {
                laddr = _laddr;
                break;
            }
        }
        i++;
    }

    if (laddr == CEC_LADDR_UNREGISTERED) {
        printf("All LA addresses in use!!!\n");
        return CEC_LADDR_UNREGISTERED;
    }

    if (CECSetLogicalAddr(laddr) < 0) {
        printf("CECSetLogicalAddr() failed!\n");
        return 0;
    }

    printf("Logical address = %d\n", laddr);

    /* broadcast "Report Physical Address" */
    unsigned char buffer[5];
    buffer[0] = (laddr << 4) | CEC_MSG_BROADCAST;
    buffer[1] = CEC_OPCODE_REPORT_PHYSICAL_ADDRESS;
    buffer[2] = (paddr >> 8) & 0xFF;
    buffer[3] = paddr & 0xFF;
    buffer[4] = devtype;

    if (CECSendMessage(buffer, 5) != 5) {
        printf("CECSendMessage() failed!\n");
        return 0;
    }

    return laddr;
}

/**
 * Send CEC message.
 *
 * @param *buffer   [in] pointer to buffer address where message located.
 * @param size      [in] message size.
 *
 * @return number of bytes written, or 0 if an arror occured.
 */
int CECSendMessage(unsigned char *buffer, int size)
{
	int ret;
	
    if (fd == -1) {
        printf("open device first!\n");
        return 0;
    }

    if (size > CEC_MAX_FRAME_SIZE) {
        printf("size should not exceed %d\n", CEC_MAX_FRAME_SIZE);
        return 0;
    }

#if CEC_DEBUG
    printf("CECSendMessage() : ");
    CECPrintFrame(buffer, size);
#endif

	ret = write(fd, buffer, size);

//#if CEC_DEBUG
//	if (ret != size) {
//		printf("                    - failed! (RET=%d)\n", ret);
//	}
//#endif

    return ret;

}

/**
 * Receive CEC message.
 *
 * @param *buffer   [in] pointer to buffer address where message will be stored.
 * @param size      [in] buffer size.
 * @param timeout   [in] timeout in microseconds.
 *
 * @return number of bytes received, or 0 if an arror occured.
 */
int CECReceiveMessage(unsigned char *buffer, int size, long timeout)
{
    int bytes = 0;
    fd_set rfds;
    struct timeval tv;

    if (fd == -1) {
        printf("open device first!\n");
        return 0;
    }

#if 0
	// Remove Wait...
    tv.tv_sec = 0;
    tv.tv_usec = timeout;

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    retval = select(fd + 1, &rfds, NULL, NULL, &tv);
#endif /* 0 */

	bytes = read(fd, buffer, size);
#if CEC_DEBUG
	if( bytes > 0)
	{
	    printf("CECReceiveMessage() : ");
	    CECPrintFrame(buffer, bytes);
	}

	if(bytes < 0)
	{
		printf("CECReceiveMessage() Error is occurred!!!!!! %d", bytes);
	}
#endif

    return bytes;
}

/**
 * Set CEC logical address.
 *
 * @return 1 if success, otherwise, return 0.
 */
int CECSetLogicalAddr(unsigned int laddr)
{
    if (ioctl(fd, CEC_IOC_SETLADDR, &laddr)) {
        printf("ioctl(CEC_IOC_SETLA) failed!\n");
        return 0;
    }

    return 1;
}

#if CEC_DEBUG
/**
 * Print CEC frame.
 */
void CECPrintFrame(unsigned char *buffer, unsigned int size)
{
    if (size > 0) {
        int i;
        printf("fsize: %d ", size);
        printf("frame: ");
        for (i = 0; i < size; i++) {
            printf("0x%02x ", buffer[i]);
        }
        printf("\n");
    }
}
#endif

/**
 * Check CEC message.
 *
 * @param opcode   [in] pointer to buffer address where message will be stored.
 * @param lsrc     [in] buffer size.
 *
 * @return 1 if message should be ignored, otherwise, return 0.
 */
//TODO: not finished
int CECIgnoreMessage(unsigned char opcode, unsigned char lsrc)
{
    int retval = 0;

    /* if a message coming from address 15 (unregistered) */
    if (lsrc == CEC_LADDR_UNREGISTERED) {
        switch (opcode) {
            case CEC_OPCODE_DECK_CONTROL:
            case CEC_OPCODE_PLAY:
                retval = 1;
                break;
            default:
                break;
        }
    }

    return retval;
}

/**
 * Check CEC message.
 *
 * @param opcode   [in] pointer to buffer address where message will be stored.
 * @param size     [in] message size.
 *
 * @return 0 if message should be ignored, otherwise, return 1.
 */
//TODO: not finished
int CECCheckMessageSize(unsigned char opcode, int size)
{
    int retval = 1;

    switch (opcode) {
		case CEC_OPCODE_SET_OSD_NAME:
		case CEC_OPCODE_REPORT_POWER_STATUS:
        case CEC_OPCODE_PLAY:
        case CEC_OPCODE_DECK_CONTROL:
		case CEC_OPCODE_USER_CONTROL_PRESSED:
        case CEC_OPCODE_SET_MENU_LANGUAGE:
            if (size != 3) retval = 0;
            break;
		case CEC_OPCODE_USER_CONTROL_RELEASED:
            if (size != 2) retval = 0;
            break;
        case CEC_OPCODE_SET_STREAM_PATH:
        case CEC_OPCODE_FEATURE_ABORT:
            if (size != 4) retval = 0;
            break;
        default:
            break;
    }

    return retval;
}

/**
 * Check CEC message.
 *
 * @param opcode    [in] pointer to buffer address where message will be stored.
 * @param broadcast [in] broadcast/direct message.
 *
 * @return 0 if message should be ignored, otherwise, return 1.
 */
//TODO: not finished
int CECCheckMessageMode(unsigned char opcode, int broadcast)
{
    int retval = 1;

    switch (opcode) {
        case CEC_OPCODE_REQUEST_ACTIVE_SOURCE:
        case CEC_OPCODE_SET_MENU_LANGUAGE:
		case CEC_OPCODE_SET_STREAM_PATH:
		case CEC_OPCODE_ROUTING_INFORMATION:
            if (!broadcast) retval = 0;
            break;
        case CEC_OPCODE_GIVE_PHYSICAL_ADDRESS:
        case CEC_OPCODE_DECK_CONTROL:
        case CEC_OPCODE_PLAY:
		case CEC_OPCODE_USER_CONTROL_PRESSED:
		case CEC_OPCODE_USER_CONTROL_RELEASED:
        case CEC_OPCODE_FEATURE_ABORT:
        case CEC_OPCODE_ABORT:
		case CEC_OPCODE_SET_OSD_NAME:
		case CEC_OPCODE_GIVE_OSD_NAME:
		case CEC_OPCODE_GIVE_DEVICE_POWER_STATUS:
		case CEC_OPCODE_REPORT_POWER_STATUS:
            if (broadcast) retval = 0;
            break;
        default:
            break;
    }

    return retval;
}


/**
 * Send Receive Message to Android platform
 *
 * @param uiKeyData [in] Send Message.
 *
 * @return 0 if message should be ignored, otherwise, return 1.
 */
int CECSendReceiveMessage(unsigned int uiData)
{
	printf("CECSendReceiveMessage : %d\n", uiData);

    if (ioctl(fd, CEC_IOC_SENDDATA, &uiData)) {
        printf("ioctl(CEC_IOC_SETLA) failed!\n");
        return 0;
    }
	
	return 1;
}

int CECCheckPhysicalAddress(int paddr )
{
	int paddr_edid;

    if (!EDIDGetCECPhysicalAddress(&paddr_edid)) {
        printf("Error: EDIDGetCECPhysicalAddress() failed.\n");
    }

	ALOGE("%s, paddr_edid = %d, paddr = %d\n", __func__, paddr_edid, paddr);

	if( paddr_edid == paddr )
		return 1;

	return 0;
}

