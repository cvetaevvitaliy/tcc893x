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

#include <cutils/log.h>

#include "nusensors.h"

/*****************************************************************************/

/*
 * The SENSORS Module
 */

/*
 * the AK8973 has a 8-bit ADC but the firmware seems to average 16 samples,
 * or at least makes its calibration on 12-bits values. This increases the
 * resolution by 4 bits.
 */

static const struct sensor_t sSensorList_enum[] = {
        { "BMA150 3-axis Accelerometer",
                "Bosh",
                1, SENSORS_HANDLE_BASE+ID_A,
                SENSOR_TYPE_ACCELEROMETER, 4.0f*9.81f, (4.0f*9.81f)/256.0f, 0.2f, 0, 0 , 0, { } },
        { "AK8975 3-axis Magnetic field sensor",
                "Asahi Kasei",
                1, SENSORS_HANDLE_BASE+ID_M,
                SENSOR_TYPE_MAGNETIC_FIELD, 2000.0f, 1.0f/16.0f, 6.8f, 0, 0 , 0, { } },
        { "AK8975 Orientation sensor",
                "Asahi Kasei",
                1, SENSORS_HANDLE_BASE+ID_O,
                SENSOR_TYPE_ORIENTATION, 360.0f, 1.0f, 7.0f, 0, 0 , 0, { } },
        { "CM3602 Proximity sensor",
                "Capella Microsystems",
                1, SENSORS_HANDLE_BASE+ID_P,
                SENSOR_TYPE_PROXIMITY,
                PROXIMITY_THRESHOLD_CM, PROXIMITY_THRESHOLD_CM,
                0.5f, 0, 0 , 0, { } },
        { "CM3602 Light sensor",
                "Capella Microsystems",
                1, SENSORS_HANDLE_BASE+ID_L,
                SENSOR_TYPE_LIGHT, 10240.0f, 1.0f, 0.5f, 0, 0 , 0, { } },        
#ifdef SUPPORT_BMA_TEMPERATURE                
        { "BMA150 Temperature sensor",
                "Bosh",
                1, SENSORS_HANDLE_BASE+ID_T,
                SENSOR_TYPE_TEMPERATURE, (87.5f -(-40.0f)), 0.5f, 0.5f, 0, 0 , 0, { } },                      
#endif
};

enum {
	SENSOR_HANDLE_1 = ID_A,
	SENSOR_HANDLE_2 = ID_M,
	SENSOR_HANDLE_3 = ID_O,
	SENSOR_HANDLE_4 = ID_P,
	SENSOR_HANDLE_5 = ID_L,
#ifdef SUPPORT_BMA_TEMPERATURE	
	SENSOR_HANDLE_6 = ID_T,
#endif
	SENSOR_HANDLE_MAX,
	MAX_NUM_SENSORS = SENSOR_HANDLE_MAX,
};

struct sensor_enum_t{
	const char*     name;
	int             	handle;
};

static const struct sensor_enum_t sHwSensorList[] = {
		{"Accelerometer" , SENSOR_HANDLE_1 },
		{"Magnetic" , SENSOR_HANDLE_2 },
		{"Orientation" , SENSOR_HANDLE_3 },
		{"Proximity" , SENSOR_HANDLE_4 },
		{"Light" , SENSOR_HANDLE_5 },
#ifdef SUPPORT_BMA_TEMPERATURE		
		{"Temperature" , SENSOR_HANDLE_6 },
#endif
};

static struct sensor_t sSensorList[MAX_NUM_SENSORS];
static int mSensorListCnt = 0;


int open_sensors(const struct hw_module_t* module, const char* name,
        hw_device_t** device);

static int sensors__get_sensors_list(struct sensors_module_t* module,
                                     struct sensor_t const** list)
{
	int i;
	//LOGE("  ~~~ sensors__get_sensors_list  ~~~ (%d)",mSensorListCnt);

	*list = sSensorList;
	return mSensorListCnt;
}

static struct hw_module_methods_t sensors_module_methods = {
	.open = open_sensors
};

struct sensors_module_t HAL_MODULE_INFO_SYM = {
        common: {
                tag: HARDWARE_MODULE_TAG,
                version_major: 1,
                version_minor: 0,
                id: SENSORS_HARDWARE_MODULE_ID,
                name: "Sensors TCC Module",
                author: "Telechips, Inc.",
                methods: &sensors_module_methods,
                dso: 0,
                reserved: {},
        },
        get_sensors_list: sensors__get_sensors_list,
};

/*****************************************************************************/

int open_sensors(const struct hw_module_t* module, const char* name,
        hw_device_t** device)
{	
	int ret;
	int handleList[SENSOR_HANDLE_MAX];
	//LOGE("  ~~~ open_sensors  ~~~ ");
	ret = init_nusensors(module, device,&handleList[0],SENSOR_HANDLE_MAX);
	{
		int i;
		int support_cnt = 0;
		for(i = 0 ; i < MAX_NUM_SENSORS ; i++)
		{
			if(handleList[i] >= 0){
				//LOGE("  ~~~ hasHardware handle(%s)  (%d)  ~~~ ",sHwSensorList[i].name,i);
				memcpy(&sSensorList[support_cnt++], &sSensorList_enum[i], sizeof(struct sensor_t));
			}
		}
		//LOGE("  ~~~ open_sensors  support_cnt(%d)~~~ ", support_cnt);		
		mSensorListCnt = support_cnt;
	}
	return ret;
}
