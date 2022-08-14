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
#include <string.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "libiic.h"

#include <utils/Log.h>

#define LOG_TAG				"LIBIIC: "
#if 0
#define printf(args...)    LOGI(args)
#else
#define printf(args...)
#endif
/**
 * @brief I2C device name.
 * User should change this
 */
#if defined(_TCC9300_)
#define DEV_NAME    "/dev/i2c-7"
#elif defined(_TCC8800_)
#define DEV_NAME    "/dev/i2c-5"
#elif defined(_TCC8920_) || defined(_TCC8930_)
#define DEV_NAME    "/dev/i2c-4"
#else
#define DEV_NAME    "/dev/i2c-3"
#endif//
/**
 * I2C file descriptor
 */
static int iic_fd = -1;

/**
 * Check if I2C file is already opened or not
 * @return  If I2C file is already opened, return 1;Otherwise. return 0
 */
static int IICFileAvailable()
{
    return (iic_fd < 0) ? 0 : 1;
}

/**
 * Initialze I2C library. Open I2C device.
 * @return  If succeed in opening I2C device or it is already opened, return 1;@n
 *          Otherwise, return 0
 */
int IICOpen()
{
    int ret = 1;

    // check already open??
    if (IICFileAvailable())
    {
//        printf("I2C_IIC is already open!!!!\n\n");
        return 1;
    }

    // open
    if ( (iic_fd = open(DEV_NAME,O_RDWR)) < 0 )
    {
        printf("Cannot open I2C_IIC : %s \n\n",DEV_NAME);
        ret = 0;
    }

    return ret;
}

/**
 * Finailize I2C library. Close I2C device
 * @return  If succeed in closing I2C device, return 1;@n
 *          Otherwise, return 0
 */
int IICClose()
{
    int ret = 1;
    // check if fd is available
    if (!IICFileAvailable())
    {
        printf("I2C_IIC is not available!!!!\n\n");
        return 1;
    }

    // close
    if (close(iic_fd) < 0)
    {
        printf("Cannot close I2C_IIC : %s \n\n",DEV_NAME);
        ret = 0;
    }

    iic_fd = -1;

    return ret;
}

/**
 * Read data though I2C. For more information of I2C, refer I2C Spec.
 * @param   devAddr    [in]    Device address
 * @param   offset  [in]    Byte offset
 * @param   size    [in]    Sizes of data
 * @param   buffer  [out]   Pointer to buffer to store data
 * @return  If succeed in reading, return 1;Otherwise, return 0
 */
int IICRead(unsigned char devAddr, unsigned char offset, unsigned char size, unsigned char* buffer)
{
    struct i2c_rdwr_ioctl_data msgset;
    struct i2c_msg msgs[2];
    int ret = 1;

    if (!IICFileAvailable())
    {
        printf("I2C_IIC is not available!!!\n\n");
        return 0;
    }

    // set offset
    msgs[0].addr = devAddr>>1;
    msgs[0].flags = 0;
    msgs[0].len = 1;
    msgs[0].buf = &offset;

    // read data
    msgs[1].addr = devAddr>>1;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = size;
    msgs[1].buf = buffer;

    // set rdwr ioctl data
    msgset.nmsgs = 2;
    msgset.msgs = msgs;

    if ((ret = ioctl(iic_fd, I2C_RDWR, &msgset)) < 0)
    {
        perror("iic error:");
        ret = 0;
    }

    return ret;
}

/**
 * Write data though I2C. For more information of I2C, refer I2C Spec.
 * @param   devAddr    [in]    Device address
 * @param   offset  [in]    Byte offset
 * @param   size    [in]    Size of data
 * @param   buffer  [out]   Pointer to buffer
 * @return  If succeed in writing, return 1;Otherwise, return 0
 */
int IICWrite(unsigned char devAddr, unsigned char offset, unsigned char size, unsigned char* buffer)
{
    unsigned char* temp;
    int bytes;
    int retval = 0;

//TODO: debugging only
#if 1
    printf("------------------------------------------------\n");
    printf("%s(addr=0x%02x, offset=0x%02x, size=%d, buffer)\n", __FUNCTION__, devAddr, offset, size);

    int i=0;
    do {
        if (i%8) {
            printf("0x%02x ", buffer[i]);
        } else {
//TODO: BUG!!!!!!!!!!!!!!!!!!!
//????????
            printf("\n0x%02x : 0x%02x ", i, buffer[i]);
        }
        i++;
    } while (i<size);
    printf("\n");

    printf("------------------------------------------------\n");
#endif

    // allocate temporary buffer
    temp = (unsigned char*) malloc((size+1)*sizeof(unsigned char));
    if (!temp)
    {
        printf("not enough resources at %s\n", __FUNCTION__);
        goto exit;
    }

    temp[0] = offset;
    memcpy(temp+1,buffer,size);

    if (!IICFileAvailable())
    {
        printf("I2C_IIC is not available!!!\n\n");
        goto exit;
    }

#if defined(DISPLAY_OUTPUT_STB)
    if (ioctl(iic_fd, I2C_SLAVE_FORCE, devAddr>>1) < 0)
#else
    if (ioctl(iic_fd, I2C_SLAVE, devAddr>>1) < 0)
#endif
    {
        printf("cannot set slave address 0x%02x\n\n", devAddr);
        goto exit;
    }

    // write temp buffer
    if ( (bytes = write(iic_fd,temp,size+1)) != (size+1))
    {
        printf("fail to write %d bytes, only write %d bytes\n",size,bytes);
        goto exit;
    }

    retval = 1;

exit:
    // free temp buffer
    if (temp) free(temp);

    return retval;
}
