/*
 * Copyright (C) 2008 The Android Open Source Project
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

#include <hardware/sensors.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <math.h>

#include <poll.h>
#include <pthread.h>

#include <linux/input.h>

#include <cutils/atomic.h>
#include <cutils/log.h>

#include "nusensors.h"
#include "LightSensor.h"
#include "ProximitySensor.h"
#include "AkmSensor.h"
#include "AccelSensor.h"

/*****************************************************************************/

struct sensors_poll_context_t {
    struct sensors_poll_device_t device; // must be first

        sensors_poll_context_t();
        ~sensors_poll_context_t();
    int activate(int handle, int enabled);
    int setDelay(int handle, int64_t ns);
    int pollEvents(sensors_event_t* data, int count);
    int setCalibration(int handle, float x,float y, float z);
    int startCalibration(int handle);
    int hasHardware(int handle);
    int hasThermometer();

private:
    enum {
        akm                 = 0,
        accel               = 1,
        light                = 2,
        proximity        = 3,
        numSensorDrivers,
        numFds,
    };

    static const size_t wake = numFds - 1;
    static const char WAKE_MESSAGE = 'W';
    struct pollfd mPollFds[numFds];
    int mWritePipeFd;
    SensorBase* mSensors[numSensorDrivers];

    int handleToDriver(int handle) const {
        //ALOGE("  handleToDriver(%d)", handle);
        switch (handle) {
            case ID_A:
#ifdef SUPPORT_BMA_TEMPERATURE                
            case ID_T: // temperature
#endif
                //ALOGE("  handleToDriver case ID_A %d",mPollFds[akm].fd);
                //if(mSensors[akm]->getFd() < 0){
                if(mPollFds[akm].fd < 0){
                    //ALOGE("  return accel");
                    return accel;
                }
            case ID_M:
            case ID_O:
                //ALOGE("  return akm");
                return akm;   
            case ID_P:
                //ALOGE("  return proximity");
                return proximity;
            case ID_L:
                //ALOGE("  return light");
                return light;
        }
        //ALOGE("  return EINVAL");
        return -EINVAL;
    }
};

/*****************************************************************************/

sensors_poll_context_t::sensors_poll_context_t()
{
   //ALOGE(" sensors_poll_context_t ~~~ ");

    mSensors[akm] = new AkmSensor();
    mPollFds[akm].fd = mSensors[akm]->getFd();
    mPollFds[akm].events = POLLIN;
    mPollFds[akm].revents = 0;

    if(mSensors[akm]->getFd() < 0){
        mSensors[accel] = new AccelSensor();
        mPollFds[accel].fd = mSensors[accel]->getFd();
        mPollFds[accel].events = POLLIN;
        mPollFds[accel].revents = 0;
    }else{
        mPollFds[accel].fd = -1;
    }

    mSensors[light] = new LightSensor();
    mPollFds[light].fd = mSensors[light]->getFd();
    mPollFds[light].events = POLLIN;
    mPollFds[light].revents = 0;

    mSensors[proximity] = new ProximitySensor();
    mPollFds[proximity].fd = mSensors[proximity]->getFd();
    mPollFds[proximity].events = POLLIN;
    mPollFds[proximity].revents = 0;

    int wakeFds[2];
    int result = pipe(wakeFds);
    ALOGE_IF(result<0, "error creating wake pipe (%s)", strerror(errno));
    fcntl(wakeFds[0], F_SETFL, O_NONBLOCK);
    fcntl(wakeFds[1], F_SETFL, O_NONBLOCK);
    mWritePipeFd = wakeFds[1];

    mPollFds[wake].fd = wakeFds[0];
    mPollFds[wake].events = POLLIN;
    mPollFds[wake].revents = 0;
}

sensors_poll_context_t::~sensors_poll_context_t() {
    for (int i=0 ; i<numSensorDrivers ; i++) {
        delete mSensors[i];
    }
    close(mPollFds[wake].fd);
    close(mWritePipeFd);
}

int sensors_poll_context_t::hasHardware(int handle)
{
    int driver;
    int ret = -1;
    //ALOGE("  hasHardware(%d)", handle);
    driver = handleToDriver(handle);
    //ALOGE("  hasHardware  driver(%d)", driver);
    if(driver < 0)
        return -1;
    ret = (mSensors[driver]->getFd() >= 0)? 1: -1;
    //ALOGE("  hasHardware  check getFd (%d)",ret);
    return ret;
}

int sensors_poll_context_t::activate(int handle, int enabled) {
    int index = handleToDriver(handle);
    if (index < 0) return index;
    int err =  mSensors[index]->enable(handle, enabled);
    if (enabled && !err) {
        const char wakeMessage(WAKE_MESSAGE);
        int result = write(mWritePipeFd, &wakeMessage, 1);
        ALOGE_IF(result<0, "error sending wake message (%s)", strerror(errno));
    }
    return err;
}

int sensors_poll_context_t::setDelay(int handle, int64_t ns) {

    int index = handleToDriver(handle);
    if (index < 0) return index;
    return mSensors[index]->setDelay(handle, ns);
}

int sensors_poll_context_t::setCalibration(int handle, float x,float y, float z) {

    int index = handleToDriver(handle);
    if (index < 0) return index;
    if(handle != ID_A)
        return 0;
    return mSensors[index]->setCalibration(x,y,z);
}

int sensors_poll_context_t::startCalibration(int handle) {

    int index = handleToDriver(handle);
    if (index < 0) return index;
    if(handle != ID_A)
        return 0;
    return mSensors[index]->startCalibration();
}

int sensors_poll_context_t::pollEvents(sensors_event_t* data, int count)
{
    int nbEvents = 0;
    int n = 0;

    //ALOGE(" pollEvents() start count(%d) fd(%d)",count,mPollFds);
    do {
        // see if we have some leftover from the last poll()
        for (int i=0 ; count && i<numSensorDrivers ; i++) {
            SensorBase* const sensor(mSensors[i]);
            if(mPollFds[i].fd < 0)
                continue;
            if ((mPollFds[i].revents & POLLIN) || (sensor->hasPendingEvents())) {
                int nb = sensor->readEvents(data, count);
                if (nb < count) {
                    // no more data for this sensor
                    mPollFds[i].revents = 0;
                }
                count -= nb;
                nbEvents += nb;
                data += nb;
            }
        }

        if (count) {
            // we still have some room, so try to see if we can get
            // some events immediately or just wait if we don't have
            // anything to return
            n = poll(mPollFds, numFds, nbEvents ? 0 : -1);
            if (n<0) {
                ALOGE("poll() failed (%s)", strerror(errno));
                return -errno;
            }
            if (mPollFds[wake].revents & POLLIN) {
                char msg;
                int result = read(mPollFds[wake].fd, &msg, 1);
                ALOGE_IF(result<0, "error reading from wake pipe (%s)", strerror(errno));
                ALOGE_IF(msg != WAKE_MESSAGE, "unknown message on wake queue (0x%02x)", int(msg));
                mPollFds[wake].revents = 0;
            }
        }
        else
        {
            //ALOGE(" pollEvents() end count(%d)",count);
            return count;
        }

        // if we have events and space, go read them
    } while (n && count);

    //ALOGE(" pollEvents() end nbEvents(%d)",nbEvents);

    return nbEvents;
}

int sensors_poll_context_t::hasThermometer() {
    int ret = -1;

    if( mSensors[akm]->getFd() > 0){
        ret = mSensors[akm]->hasTemperature();
    }else if(mSensors[accel]->getFd() > 0){
        ret = mSensors[accel]->hasTemperature();
    }
    //ALOGE(" hasThermometer  ret(%d)",ret);
    return ret;
}

/*****************************************************************************/

static int poll__close(struct hw_device_t *dev)
{
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    if (ctx) {
        delete ctx;
    }
    return 0;
}

static int poll__activate(struct sensors_poll_device_t *dev,
        int handle, int enabled) {
    int res;
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    res = ctx->activate(handle, enabled);
    //ALOGE(" poll__activate() handle(%d) enabled(%d) res(%d)",handle,enabled,res);
    return res;
}

static int poll__setDelay(struct sensors_poll_device_t *dev,
        int handle, int64_t ns) {
    int res;
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    res = ctx->setDelay(handle, ns);
    //ALOGE(" ~~~poll__setDelay()  h(%d) ns(%d) res(%d)",handle,ns,res);
    return res;
}

static int poll__poll(struct sensors_poll_device_t *dev,
        sensors_event_t* data, int count) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->pollEvents(data, count);
}

static int poll__setCalibration(struct sensors_poll_device_t *dev,
        int handle, float x, float y, float z){
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->setCalibration(handle, x,y,z);
}

static int poll__startCalibration(struct sensors_poll_device_t *dev,
        int handle){
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->startCalibration(handle);
}

 sensors_poll_context_t *mDevices = NULL;

int checkHardware(struct sensors_poll_device_t *dev,int handle)
{
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    //ALOGE("   checkHardware  handle(%d)",handle);
    return ctx->hasHardware(handle);
}

/*****************************************************************************/

int init_nusensors(hw_module_t const* module, hw_device_t** device, int *enable, int cnt)
{
    int status = -EINVAL;

    //ALOGE(" >>>>>>>>>>>>>>>>>>>>>>>> init_nusensors");

    sensors_poll_context_t *dev = new sensors_poll_context_t();

    memset(&dev->device, 0, sizeof(sensors_poll_device_t));

    dev->device.common.tag = HARDWARE_DEVICE_TAG;
    dev->device.common.version  = 0;
    dev->device.common.module   = const_cast<hw_module_t*>(module);
    dev->device.common.close    = poll__close;
    dev->device.activate        = poll__activate;
    dev->device.setDelay        = poll__setDelay;
    dev->device.poll            = poll__poll;
    dev->device.setCalibration = poll__setCalibration;
    dev->device.startCalibration = poll__startCalibration;

    *device = &dev->device.common;
    status = 0;
    {
        int i;
        for( i = 0 ; i < cnt ; i++){
            #ifdef SUPPORT_BMA_TEMPERATURE
            if(i == ID_T)
            {
                if(dev->hasThermometer() >= 0)
                    enable[i] = 1;
                else
                    enable[i] = -1;
            }
            else
            #endif
            {
            if(dev->hasHardware(i) >= 0){
                enable[i] = 1;
            }else{
                enable[i] = -1;
                }
            }
            //ALOGE("    check hw(%d) in init_nusensors enalbe(%d)",i,enable[i]);
        }
    }
    return status;
}
