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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "libddc.h"

#include <utils/Log.h>
#define LOG_TAG "LIBDDC"

#define LIBDDC_DEBUG 0
#if LIBDDC_DEBUG
#define DPRINTF(args...)    ALOGD(args)
#else
#define DPRINTF(args...)
#endif

/**
 * @brief DDC device name.
 * User should change this.
 */
 
#if defined(_TCC8800_) || defined(_TCC8920_) || defined(_TCC8930_)
#define DEV_NAME    "/dev/i2c-3"
#else
#define DEV_NAME    "/dev/i2c-0"
#endif//
/**
 * DDC file descriptor
 */
static int ddc_fd = -1;

/**
 * Reference count of DDC file descriptor
 */
static unsigned int ref_cnt = 0;

/**
 * Check if DDC file is already opened or not
 * @return  If DDC file is already opened, return 1;Otherwise, return 0.
 */
static int DDCFileAvailable()
{
    return (ddc_fd < 0) ? 0 : 1;
}

/**
 * Initialze DDC library. Open DDC device
 * @return  If succeed in opening DDC device or it is already opened, return 1;@n
 *         Otherwise, return 0
 */
int DDCOpen()
{
    int ret = 1;

    // check already open??
    if (ref_cnt > 0)
    {
//        DPRINTF("I2C_DDC is already open!!!!\n\n");
        ref_cnt++;
        return 1;
    }

    // open
    if ( (ddc_fd = open(DEV_NAME, O_RDWR)) < 0 )
    {
        ALOGE("Cannot open I2C_DDC : %s \n\n",DEV_NAME);
        ret = 0;
    }

    ref_cnt++;
    return ret;
}

/**
 * Finalize DDC library. Close DDC device
 * @return  If succeed in closing DDC device or it is being used yet, return 1;@n
 *          Otherwise, return 0
 */
int DDCClose()
{
    int ret = 1;
    // check if fd is available
    if (ref_cnt == 0)
    {
        ALOGE("I2C_DDC is not available!!!!\n\n");
        return 1;
    }

    // close
    if (ref_cnt > 1)
    {
        ref_cnt--;
        return 1;
    }

    if (close(ddc_fd) < 0)
    {
        ALOGE("Cannot close I2C_DDC : %s \n\n",DEV_NAME);
        ret = 0;
    }

    ref_cnt--;
    ddc_fd = -1;

    return ret;
}

/**
 * Read data though DDC. For more information of DDC, refer DDC Spec.
 * @param   addr    [in]    Device address
 * @param   offset  [in]    Byte offset
 * @param   size    [in]    Sizes of data
 * @param   buffer  [out]   Pointer to buffer to store data
 * @return  If succeed in reading, return 1;Otherwise, return 0
 */
int DDCRead(unsigned char addr, unsigned char offset,
            unsigned int size, unsigned char* buffer)
{
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data msgset;

    int ret = 1;
	int i;

    if (!DDCFileAvailable())
    {
        ALOGE("I2C_DDC is not available!!!!\n\n");
        return 0;
    }

#if 0//defined(TELECHIPS)
	// set offset
	if(ioctl(ddc_fd, I2C_SLAVE_NONAK, addr>>1) < 0) {
		ALOGE("ddc error:");
		ret = 0;
	}
	if(ioctl(ddc_fd, I2C_WR_RD, addr>>1) < 0) {
		ALOGE("ioctl(I2C_SLAVE) failed!!!\n");
		ret = 0;
	}

	if(write(ddc_fd, &offset, 1) !=1) {
		ALOGE("ddc error:");
		ret = 0;
	}

	// read
  #if 0
	i=0;
	for(i=0; i<size; i++)
	{
		if(read(ddc_fd, &buffer[i], 1) != 1) {
			perror("ddc error:");
			ret = 0;
		}
	}
  #else
	if(read(ddc_fd, &buffer[0], size) != size) {
		perror("ddc error:");
		ret = 0;
	}
  #endif
#else
    // set offset
    msgs[0].addr = addr>>1;
    msgs[0].flags = 0;
    msgs[0].len = 1;
    msgs[0].buf = &offset;

    // read data
    msgs[1].addr = addr>>1;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = size;
    msgs[1].buf = buffer;

    // set rdwr ioctl data
    msgset.nmsgs = 2;
    msgset.msgs = msgs;

    // i2c fast read
    if ((ret = ioctl(ddc_fd, I2C_RDWR, &msgset)) < 0)
    {
        ALOGE("ddc error:");
        ret = 0;
    }
#endif

    return ret;
}

/**
 * Read data though E-DDC. For more information of E-DDC, refer E-DDC Spec.
 * @param   segpointer  [in]    Segment pointer
 * @param   segment     [in]    Segment number
 * @param   addr        [in]    Device address
 * @param   offset      [in]    Byte offset
 * @param   size        [in]    Sizes of data
 * @param   buffer      [out]   Pointer to buffer to store data
 * @return  If succeed in reading, return 1;Otherwise, return 0
 */

int EDDCRead(unsigned char segpointer, unsigned char segment, unsigned char addr,
  unsigned char offset, unsigned int size, unsigned char* buffer)
{
    struct i2c_rdwr_ioctl_data msgset;
    struct i2c_msg msgs[3];
    int ret = 1;
	int i;

    if (!DDCFileAvailable())
    {
        DPRINTF("I2C_DDC is not available!!!!\n\n");
        return 0;
    }

	DPRINTF("EDID READ START segpointer:0x%x segment:0x%x add:0x%x offset:0x%x size:%d!!\n", segpointer, segment, addr, offset, size);

#if 0//defined(TELECHIPS)
	// set segment pointer
	if(ioctl(ddc_fd, I2C_SLAVE, segpointer>>1) < 0) {
		DPRINTF("ioctl(I2C_SLAVE) failed!!!\n");
		ret = 0;
	}
	
	if(write(ddc_fd, &segment, 1) !=1) {
		DPRINTF("write failed!!!\n");
		ret = 0;
	}

	// set offset
	if(ioctl(ddc_fd, I2C_SLAVE_NONAK, addr>>1) < 0) {
		DPRINTF("ioctl(I2C_SLAVE) failed!!!\n");
		ret = 0;
	}

	if(ioctl(ddc_fd, I2C_WR_RD, addr>>1) < 0) {
		printf("ioctl(I2C_SLAVE) failed!!!\n");
		ret = 0;
	}

	if(write(ddc_fd, &offset, 1) !=1) {
		DPRINTF("write failed!!!\n");
		ret = 0;
	}

	// eddc read
  #if 0	
	i=0;
	for(i=0; i<size; i++)
	{
		if(read(ddc_fd, &buffer[i], 1) != 1) {
			DPRINTF("read failed!!!\n");
			ret = 0;
		}
	}
  #else
	if(read(ddc_fd, &buffer[0], size) != size) {
		printf("read failed!!!\n");
		ret = 0;
	}
  #endif
#else
    // set segment pointer
    msgs[0].addr  = segpointer>>1;
    msgs[0].flags = 0;
    msgs[0].len   = 1;
    msgs[0].buf   = &segment;

    // set offset
    msgs[1].addr  = addr>>1;
    msgs[1].flags = 0;
    msgs[1].len   = 1;
    msgs[1].buf   = &offset;

    // read data
    msgs[2].addr  = addr>>1;
    msgs[2].flags = I2C_M_RD;
    msgs[2].len   = size;
    msgs[2].buf   = buffer;

    msgset.nmsgs = 3;
    msgset.msgs  = msgs;
	

    // eddc read
    if (ioctl(ddc_fd, I2C_RDWR, &msgset) < 0)
    {
        DPRINTF("ioctl(I2C_RDWR) failed!!!\n");
        ret = 0;
    }

#endif
	DPRINTF("EDID READ end ret: %d !!\n",ret);

    return ret;
}

/**
 * Write data though DDC. For more information of DDC, refer DDC Spec.
 * @param   addr    [in]    Device address
 * @param   offset  [in]    Byte offset
 * @param   size    [in]    Sizes of data
 * @param   buffer  [out]   Pointer to buffer to write
 * @return  If succeed in writing, return 1;Otherwise, return 0
 */
int DDCWrite(unsigned char addr, unsigned char offset, unsigned int size, unsigned char* buffer)
{
    unsigned char* temp;
    int bytes;
    int retval = 0;

    // allocate temporary buffer
    temp = (unsigned char*) malloc((size+1)*sizeof(unsigned char));
    if (!temp)
    {
        DPRINTF("not enough resources at %s\n", __FUNCTION__);
        goto exit;
    }

    temp[0] = offset;
    memcpy(temp+1,buffer,size);

    if (!DDCFileAvailable())
    {
        DPRINTF("I2C_DDC is not available!!!!\n\n");
        goto exit;
    }

    if (ioctl(ddc_fd, I2C_SLAVE, addr>>1) < 0)
    {
        DPRINTF("cannot set slave address 0x%02x\n\n", addr);
        goto exit;
    }

    // write temp buffer
    if ( (bytes = write(ddc_fd,temp,size+1)) != (size+1))
    {
        DPRINTF("fail to write %d bytes, only write %d bytes\n",size,bytes);
        goto exit;
    }

    retval = 1;

exit:
    // free temp buffer
    if (temp) free(temp);

    return retval;
}
