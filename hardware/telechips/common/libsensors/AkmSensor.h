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

#ifndef ANDROID_AKM_SENSOR_H
#define ANDROID_AKM_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>


#include "nusensors.h"
#include "SensorBase.h"
#include "InputEventReader.h"

/*****************************************************************************/

struct input_event;

class AkmSensor : public SensorBase {
public:
            AkmSensor();
    virtual ~AkmSensor();

    enum {
        Accelerometer   = 0,
        MagneticField   = 1,
        Orientation     = 2,
#ifdef SUPPORT_BMA_TEMPERATURE			
        Temperature = 3,// temperature
#endif           
        numSensors
    };

	typedef struct{
		short	_11;
		short	_12;
		short	_13;
		short	_21;
		short	_22;
		short	_23;
		short	_31;
		short	_32;
		short	_33;
	} LayoutMatrix;

	typedef struct{
		float 	_11;
		float 	_12;
		float 	_13;
		float 	_21;
		float 	_22;
		float 	_23;
		float 	_31;
		float 	_32;
		float 	_33;
	} LayoutMatrixF;

	LayoutMatrix eCLayout[4];
	LayoutMatrixF fECLayout;	

	virtual int setDelay(int32_t handle, int64_t ns);
	virtual int enable(int32_t handle, int enabled);
	virtual int readEvents(sensors_event_t* data, int count);
	void processEvent(int code, int value);
	int setCalibration(float x, float y,float z);
	int startCalibration();
	float dataConvertTemp(int data);
	int hasTemperature();	
private:
	int update_delay();
	int ak8975_GetMatrix(short *matrix);
	int ak8975_initLayout(void);
	uint32_t mEnabled;
	uint32_t mPendingMask;
	InputEventCircularReader mInputReader;
	uint32_t mEnabledTemperature;
	int32_t mTempSupport;
	int32_t mTempResolution;
	int32_t mTempCenter;		
	int         mTemperature_fd;
	sensors_event_t mPendingEvents[numSensors];
	sensors_event_t mDummyEvents[1];
	uint64_t mDelays[numSensors];
};

/*****************************************************************************/

#endif  // ANDROID_AKM_SENSOR_H
