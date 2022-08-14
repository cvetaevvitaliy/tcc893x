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
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "libhpd.h"
#include <utils/Log.h>

#define LOG_TAG				"-LIBHPD-"
#define printf(args...)    ALOGI(args)

/**
 * @def HPD_DEVICE_NAME
 * Defines simbolic name of HPD device.
 */
#define HPD_DEVICE_NAME         "/dev/hpd"

/**
 * @var _handler
 * Pointer to callback function.
 */
static HPDCallback *_handler = NULL;

/**
 * @var running
 * HPD thread running flag.
 */
static int running = 0;

/**
 * @var _pthread
 * Keep ID of the HPD thread.
 */
static pthread_t _pthread;

/**
 * @var fd
 * HPD device file descriptor.
 */
static int fd = -1;

/**
 * Detect changes of the HPD signal.
 *
 * @param    arg    [in]    Input argument. Not in use.
 * @return    NULL
 */
static void *HPDThread(void *arg)
{
    int hpd;
    fd_set rfds;
    struct timeval tv;

    while (running) {
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);

        tv.tv_sec = 0;
        tv.tv_usec = 100000; // 100 ms

        int retval = select(fd + 1, &rfds, NULL, NULL, &tv);

        if (retval == -1) {
            perror("select()");
			if (_handler)
            	_handler(hpd);
			
        } else if (retval) {
            read(fd, &hpd, sizeof(int));
            if (_handler)
                _handler(hpd);
         } else {
            if (_handler)
                _handler(hpd);
        }
    }

    return 0;
}

/**
 * Open device driver and assign HPD file descriptor.
 *
 * @return    If success to assign HPD file descriptor, return 1;otherwise, return 0.
 */
int HPDOpen()
{
    int res = 1;

    if (fd != -1)
        HPDClose();

    printf("opening %s...\n", HPD_DEVICE_NAME);
    if ((fd = open(HPD_DEVICE_NAME, O_RDWR)) < 0) {
        printf("Can't open %s!\n", HPD_DEVICE_NAME);
        res = 0;
    }

    return res;
}

/**
 * Close HPD file descriptor.
 *
 * @return    If success to close HPD file descriptor, return 1;otherwise, return 0.
 */
int HPDClose()
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
 * Create HPD thread.
 * If success, _pthread variable holds thread ID.
 *
 * @return    If success to creat thread, return 1;otherwise, return 0.
 */
int HPDStart()
{
    if (fd == -1) {
        printf("open device first!\n");
        return 0;
    }

    if (ioctl(fd, HPD_IOC_START, NULL)) {
        printf("ioctl(HPD_IOC_START) failed!\n");
        return 0;
    }


    if (!_handler)
        return 0;

    running = 1;
#if 0   
 if (pthread_create(&_pthread, NULL, &HPDThread, NULL)) {
        running = 0;
        printf("pthread_create() failed!\n");
        return 0;
    }
#endif//
    return 1;
}

int HPDCheck()
{
	int hpd;
	if(fd == -1){
		return 0;
	}
   
        read(fd, &hpd, sizeof(int));

	return hpd;	
}
/**
 * Wait and terminate HPD thread.
 *
 * @return    If success to terminate thread, return 1;otherwise, return 0.
 */
int HPDStop()
{
    if (fd == -1) {
        printf("open device first!\n");
        return 0;
    }

    if (!_handler)
        return 0;

    if (!running)
        return 0;

    running = 0;

    if (ioctl(fd, HPD_IOC_STOP, NULL)) {
        printf("ioctl(HPD_IOC_STOP) failed!\n");
        return 0;
    }

    #if 0
    if (pthread_join(_pthread, NULL)) {
        printf("pthread_join() failed!\n");
    }
   #endif//
    return 1;
}

/**
 * Set HPD callback function.
 *
 * @param    handler    [in]    The pointer to handler.
 * @return    If success to set CallBack handler, return 1;otherwise, return 0.
 */
int HPDSetCallback(HPDCallback *handler)
{
    if (!handler)
        return 0;

    _handler = handler;
    return 1;
}
