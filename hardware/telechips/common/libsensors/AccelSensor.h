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

#ifndef ANDROID_ACCEL_SENSOR_H
#define ANDROID_ACCEL_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>


#include "nusensors.h"
#include "SensorBase.h"
#include "InputEventReader.h"

/*****************************************************************************/
struct input_event;

class AccelSensor : public SensorBase {
public:
            AccelSensor();
    virtual ~AccelSensor();

    enum {
        Accelerometer   = 0,
#ifdef SUPPORT_BMA_TEMPERATURE			
        Temperature = 1,// temperature
#endif        
        numSensors
    };

    virtual int setDelay(int32_t handle, int64_t ns);
    virtual int setCalibration(float x,float y,float z);
    virtual int startCalibration();
    virtual int enable(int32_t handle, int enabled);
    virtual int readEvents(sensors_event_t* data, int count);
    void processEvent(int code, int value);
    float dataConvert(int data);	
    float dataConvertTemp(int data);
    int hasTemperature();
private:
    int update_delay();
    uint32_t mEnabled;
    uint32_t mPendingMask;
    InputEventCircularReader mInputReader;
    int32_t mResolution;
    int32_t mTempSupport;
    int32_t mTempResolution;
    int32_t mTempCenter;	
    sensors_event_t mPendingEvents[numSensors];
    sensors_event_t mDummyEvents[numSensors];
    uint64_t mDelays[numSensors];
};

/*****************************************************************************/

#endif  // ANDROID_ACCEL_SENSOR_H
