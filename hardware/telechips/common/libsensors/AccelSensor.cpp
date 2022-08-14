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

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>

#include <linux/akm8973.h>

#include <cutils/log.h>

#include "AccelSensor.h"

#include <mach/sensor_ioctl.h>

/*****************************************************************************/

AccelSensor::AccelSensor()
: SensorBase(ACCEL_DEVICE_NAME, "tcc-accel"),
      mEnabled(0),
      mPendingMask(0),
      mInputReader(32),
      mResolution(-1),mTempSupport(0),mTempResolution(-1),mTempCenter(-1)
{   
    sensor_get_class_path();
    
    memset(mPendingEvents, 0, sizeof(mPendingEvents));

    mPendingEvents[Accelerometer].version = sizeof(sensors_event_t);
    mPendingEvents[Accelerometer].sensor = ID_A;
    mPendingEvents[Accelerometer].type = SENSOR_TYPE_ACCELEROMETER;
    mPendingEvents[Accelerometer].acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;

#ifdef SUPPORT_BMA_TEMPERATURE
    mPendingEvents[Temperature].version = sizeof(sensors_event_t);
    mPendingEvents[Temperature].sensor = ID_T;
    mPendingEvents[Temperature].type = SENSOR_TYPE_TEMPERATURE;
#endif    
    
    mDummyEvents[0].version = sizeof(sensors_event_t);
    mDummyEvents[0].sensor = ID_A;
    mDummyEvents[0].type = SENSOR_TYPE_ACCELEROMETER;
    mDummyEvents[0].acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
    mDummyEvents[0].acceleration.x = 0;
    mDummyEvents[0].acceleration.y = 0;
    mDummyEvents[0].acceleration.z = 9.8f;    
    
    for (int i=0 ; i<numSensors ; i++)
        mDelays[i] = 20; // 200 ms by default

    // read the actual value of all sensors if they're enabled already
    struct input_absinfo absinfo;
    short flags = 0;

    open_device();

    //if(dev_fd >= 0)
    //    mEnabled |= 1<<Accelerometer;

    #if 1
    {
        unsigned int value;
        unsigned int data[3];
        int a_file = dev_fd;

        if (a_file >= 0) {
            // set input device
            value = 1;
            if (ioctl(a_file, IOCTL_SENSOR_SET_INPUTDEVICE, &value) < 0) {
                ALOGE("AKD_GetSuspendStatus ioctl error.\n");
            }

            // get the resolution of g-sensor
            value = -1;
            mResolution = 256;
            if (ioctl(a_file, IOCTL_SENSOR_GET_RESOLUTION, &value) < 0) {
                ALOGE("AKD_GetSuspendStatus ioctl error.\n");
            }else{
                if(value > 0)
                    mResolution = value;
            }
            //  get the temperature information about the support
            data[0] = data[1] = data[2] = 0;
            if (ioctl(a_file, IOCTL_SENSOR_GET_TEMP_INFO, &data[0]) < 0) {
                ALOGE("IOCTL_SENSOR_GET_TEMP_INFO ioctl error.\n");
                mTempSupport = 0;
            }else{
                mTempSupport = data[0];
                mTempCenter = data[1];
                mTempResolution = data[2];
            }

        }else{
            ALOGE("Device file is not opened.\n");        
        }
    }
    #endif

    if (!mEnabled) {
        close_device();
    }

}

AccelSensor::~AccelSensor() {
}

int AccelSensor::enable(int32_t handle, int en)
{
    int what = -1;
    char buffer[20];
    int bytes;
        
    switch (handle) {
        case ID_A: what = Accelerometer; break;
#ifdef SUPPORT_BMA_TEMPERATURE	        
        case ID_T: what = Temperature; break;
#endif        
    }

    if (uint32_t(what) >= numSensors)
        return -EINVAL;

    int newState  = en ? 1 : 0;
    int err = 0;

    //ALOGE("AccelSensor: enable dev_fd(%d) newState(%d) what(%d) mEnabled(%d)",dev_fd,newState,what,mEnabled);

    if ((uint32_t(newState)<<what) != (mEnabled & (1<<what))) {
        if (!mEnabled) {
            tcc_sensor_accel_t oldData;	
            open_device();
            //ALOGE("AccelSensor: open_device dev_fd(%d)",dev_fd);
            oldData.x = 0;
            oldData.y = 0;
            oldData.z = 0;
            LoadCalibrationPara(&oldData);
            SetCalibDataToDriver(oldData);
        }

        short flags = newState;

        if(what != Accelerometer)
            bytes = sprintf(buffer, "%d\n", flags | (1<<what));
        else
            bytes = sprintf(buffer, "%d\n", flags);
        //ALOGE("        buffer(%s) flags(%d)",buffer,flags);
        set_sysfs_input_attr(class_path,"enable",buffer,bytes);

        //ALOGE("        mResolution(%d) newState(%d)",mResolution,newState);


        if (!err) {
            mEnabled &= ~(1<<what);
            mEnabled |= (uint32_t(flags)<<what);
            //ALOGE("AccelSensor: mEnabled(%d) flags(%d) what(%d)",mEnabled,flags,what);
            update_delay();
        }
        if (!mEnabled) {
            //ALOGE("AccelSensor: enable mEnabled(%d)",mEnabled);
            close_device();
        }
    }
    return err;
}

int AccelSensor::setDelay(int32_t handle, int64_t ms)
{
    int what = -1;
    switch (handle) {
        case ID_A: what = Accelerometer; break;
#ifdef SUPPORT_BMA_TEMPERATURE	        
        case ID_T: what = Temperature; break;
#endif               
    }

    if (uint32_t(what) >= numSensors)
        return -EINVAL;

    if (ms < 0)
        return -EINVAL;

    if(ms == 0)
        ms = 1;

    mDelays[what] = ms;
    return update_delay();
}

int AccelSensor::update_delay()
{
    if (mEnabled) {
        char buffer[20];
        int bytes;
        uint64_t wanted = -1LLU;
        for (int i=0 ; i<numSensors ; i++) {
            if (mEnabled & (1<<i)) {
                uint64_t ns = mDelays[i];
                wanted = wanted < ns ? wanted : ns;
            }
        }
        unsigned int delay = (unsigned int )(wanted )/ 1000000;

        bytes = sprintf(buffer, "%d\n", delay);
        //ALOGE("        update_delay buffer(%s) delay(%d)",buffer,delay);
        set_sysfs_input_attr(class_path,"delay",buffer,bytes);
    }
    return 0;
}

float AccelSensor::dataConvert(int data)
{    
    float fTemp;
    float dot_shift = (1000.0f);
    float  m_GRAVI = 9.8f;
    int iTemp = 0;
    int resolution = (int)mResolution;
    fTemp = (float)((((float)data)/((float)resolution))*m_GRAVI);
    fTemp = (float)((float)(fTemp)*dot_shift);
    iTemp = (int)fTemp;
    fTemp = ((float)iTemp);
    fTemp = (float)(fTemp/dot_shift);
    return fTemp;
}

float AccelSensor::dataConvertTemp(int data)
{
    float fData = (float)data;

    fData = (fData/(float)mTempResolution) + (float)mTempCenter;
    return fData;
}
int AccelSensor::readEvents(sensors_event_t* data, int count)
{
    //ALOGE("AccelSensor: readEvents count(%d)",count);

    if (count < 1)
        return -EINVAL;

    if(mEnabled == 0)
    {
        //ALOGE("AkmSensor: readEvents mEnabled(%d)",mEnabled);
        *data++ = mDummyEvents[0];
        usleep(20*1000);
        return 1;        
//        usleep(100*1000);
//        ALOGE("AccelSensor: readEvents mEnabled(%d)",mEnabled);
//        return 0;
    }

    ssize_t n = mInputReader.fill(data_fd);
    if (n < 0)
    {
        ALOGE("AccelSensor: readEvents mInputReader.fill (%d) ",(int)n);
        return n;
    }

    int numEventReceived = 0;
    input_event const* event;

    //ALOGE("AccelSensor: readEvents data_fd(%d) count(%d) n(%d)",data_fd,count,n);

    while (count && mInputReader.readEvent(&event)) {
        int type = event->type;
        //ALOGE("AccelSensor: processEvent type (%d) value (%d) mPendingMask(%d)",type,event->value,mPendingMask);
        if (type == EV_ABS) {
            processEvent(event->code, event->value);
            mInputReader.next();
        } else if (type == EV_SYN) {
            int64_t time = timevalToNano(event->time);
            for (int j=0 ; count && mPendingMask && j<numSensors ; j++) {
                if (mPendingMask & (1<<j)) {
                    mPendingMask &= ~(1<<j);
                    mPendingEvents[j].timestamp = time;
                    if (mEnabled & (1<<j)) {
                        *data++ = mPendingEvents[j];
                        count--;
                        numEventReceived++;
                    }
                }
            }
            if (!mPendingMask) {
                mInputReader.next();
            }
        } else {
            //ALOGE("AccelSensor: unknown event (type=%d, code=%d)",
            //        type, event->code);
            mInputReader.next();
        }
    }

    //if(numEventReceived)
        //ALOGE("AccelSensor: readEvents (%f)(%f)(%f)",mPendingEvents[0].acceleration.x,mPendingEvents[0].acceleration.y,mPendingEvents[0].acceleration.z);

    return numEventReceived;
}

void AccelSensor::processEvent(int code, int value)
{
    //ALOGE("AccelSensor: processEvent code (%d) value (%d) resolution(%d)",code,value,mResolution);
    switch (code) {
        case EVENT_TYPE_ACCEL_X:
            mPendingMask |= 1<<Accelerometer;
            mPendingEvents[Accelerometer].acceleration.x = dataConvert(value);//*CONVERT_A_X;
            break;
        case EVENT_TYPE_ACCEL_Y:
            mPendingMask |= 1<<Accelerometer;
            mPendingEvents[Accelerometer].acceleration.y = dataConvert(value);//* CONVERT_A_Y;
            break;
        case EVENT_TYPE_ACCEL_Z:
            mPendingMask |= 1<<Accelerometer;
            mPendingEvents[Accelerometer].acceleration.z = dataConvert(value);//* CONVERT_A_Z;
            break;
#ifdef SUPPORT_BMA_TEMPERATURE	            
        case EVENT_TYPE_TEMPERATURE: // temperature
            mPendingMask |= 1<<Temperature;
            mPendingEvents[Temperature].temperature = dataConvertTemp(value);
            //ALOGE("AccelSensor: temperature %d %f",value, mPendingEvents[Temperature].temperature);
            break;
#endif            
    }
}

int AccelSensor::setCalibration(float x, float y,float z)
{
	tcc_sensor_accel_t temp;
       tcc_sensor_accel_t oldData;	
       tcc_sensor_accel_t newData;	
       int iData[3];
	float fPercent=0;
       float  m_GRAVI = 9.8f;

	//ALOGE(" set calibration float x=%f y=%f z =%f",x,y,z);
       if(set_HwCalbration() >= 0){
           return 0;
       }

	fPercent = (float)(((float)(x/m_GRAVI))*100);
	iData[0] = (int)(fPercent);
	fPercent = (float)(((float)(y/m_GRAVI))*100);
	iData[1] = (int)(fPercent);
	iData[2] = 0;

	temp.x = iData[0];
	temp.y = iData[1];
	temp.z = 0;

	oldData.x = 0;
	oldData.y = 0;
	oldData.z = 0;

	LoadCalibrationPara(&oldData);

	newData.x = oldData.x + temp.x;
	newData.y = oldData.y + temp.y;
	newData.z = oldData.z + temp.z;
	
	//ALOGE(" set calibration percent x=%d y=%d z =%d",temp.x,temp.y,temp.z);
	if(SaveCalibrationPara(newData) > 0)
       {
           SetCalibDataToDriver(newData);
           return 0;
       }
	return -1;
}

int AccelSensor::startCalibration()
{
	return 0;
}

int AccelSensor::hasTemperature()
{
    //ALOGE("AccelSensor::hasTemperature = %d.\n",mTempSupport);
	return (mTempSupport == 0)? -1:1;
}
