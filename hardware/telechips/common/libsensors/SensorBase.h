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

#ifndef ANDROID_SENSOR_BASE_H
#define ANDROID_SENSOR_BASE_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>


/*****************************************************************************/
#define CALIBRATION_DATA_FILE	"/data/misc/calibration_set.txt"
#define DELIMITER           " = "

typedef struct {	
    short x;	
    short y;
    short z;
    short resolution;
    short delay_time;
} tcc_sensor_accel_t;

struct sensors_event_t;

class SensorBase {
protected:
    const char* dev_name;
    const char* data_name;
    int         dev_fd;
    int         data_fd;

    static int openInput(const char* inputName);
    static int64_t getTimestamp();


    static int64_t timevalToNano(timeval const& t) {
        return t.tv_sec*1000000000LL + t.tv_usec*1000;
    }

    int open_device();
    int close_device();

public:
    char gsenName[256];
    char halName[256];
    char class_path[256];
            SensorBase(
                    const char* dev_name,
                    const char* data_name);

    virtual ~SensorBase();

    virtual int readEvents(sensors_event_t* data, int count) = 0;
    virtual bool hasPendingEvents() const;
    virtual int getFd() const;
    virtual int setDelay(int32_t handle, int64_t ns);
    virtual int enable(int32_t handle, int enabled) = 0;
    virtual int setCalibration(float x, float y,float z) = 0;
    virtual int startCalibration() = 0;
    virtual int sensor_get_class_path();
    virtual int get_sysfs_input_attr_str(char *class_path,	const char *attr, char *str, int len);
    virtual int set_sysfs_input_attr(char *class_path,	const char *attr, char *value, int len);
    virtual int get_sysfs_input_attr_show(char *class_path, const char *attr, int *value);
    virtual int LoadShort(void * data, char *lpKeyName, short * val);
    virtual int LoadCalibrationPara(tcc_sensor_accel_t *para);
    virtual int SetCalibDataToDriver(tcc_sensor_accel_t para);
    virtual int SaveShort(void *data, char *lpKeyName, short val);
    virtual int SaveCalibrationPara(tcc_sensor_accel_t para);	
    virtual int set_HwCalbration();
    virtual int hasTemperature();
};

/*****************************************************************************/

#endif  // ANDROID_SENSOR_BASE_H
