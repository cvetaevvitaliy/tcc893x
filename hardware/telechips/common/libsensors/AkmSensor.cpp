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

//#include <linux/akm8975.h>

#include <cutils/log.h>
#include <cutils/properties.h>

#include "AkmSensor.h"

#include <mach/akm8975.h>
#include <mach/sensor_ioctl.h>

/*****************************************************************************/

AkmSensor::AkmSensor()
: SensorBase(AKM_DEVICE_NAME, "compass"),
      mEnabled(0),
      mEnabledTemperature(0),
      mPendingMask(0),
      mInputReader(32),
      mTempSupport(0),mTempResolution(-1),mTempCenter(-1)
{
    //ALOGE(" new AkmSensor");
    mTemperature_fd = -1;
    sensor_get_class_path();
    
    memset(mPendingEvents, 0, sizeof(mPendingEvents));

    mPendingEvents[Accelerometer].version = sizeof(sensors_event_t);
    mPendingEvents[Accelerometer].sensor = ID_A;
    mPendingEvents[Accelerometer].type = SENSOR_TYPE_ACCELEROMETER;
    mPendingEvents[Accelerometer].acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;

    mPendingEvents[MagneticField].version = sizeof(sensors_event_t);
    mPendingEvents[MagneticField].sensor = ID_M;
    mPendingEvents[MagneticField].type = SENSOR_TYPE_MAGNETIC_FIELD;
    mPendingEvents[MagneticField].magnetic.status = SENSOR_STATUS_ACCURACY_HIGH;

    mPendingEvents[Orientation  ].version = sizeof(sensors_event_t);
    mPendingEvents[Orientation  ].sensor = ID_O;
    mPendingEvents[Orientation  ].type = SENSOR_TYPE_ORIENTATION;
    mPendingEvents[Orientation  ].orientation.status = SENSOR_STATUS_ACCURACY_HIGH;

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
        mDelays[i] = 200000000; // 200 ms by default

    // read the actual value of all sensors if they're enabled already
    struct input_absinfo absinfo;
    short flags = 0;

    ak8975_initLayout();
    open_device();
    if(dev_fd >= 0){
        property_set("hw.sensor.ak8975", "1");
        //ALOGI(" property_set hw.sensor.ak8975 1");        
    }else{
        property_set("hw.sensor.ak8975", "0");
        //ALOGI(" property_set hw.sensor.ak8975 0");       
    }

    if (!ioctl(dev_fd, ECS_IOCTL_APP_GET_AFLAG, &flags)) {
        if (flags)  {
            mEnabled |= 1<<Accelerometer;
            if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_ACCEL_X), &absinfo)) {
                mPendingEvents[Accelerometer].acceleration.x = absinfo.value * CONVERT_A_X;
            }
            if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_ACCEL_Y), &absinfo)) {
                mPendingEvents[Accelerometer].acceleration.y = absinfo.value * CONVERT_A_Y;
            }
            if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_ACCEL_Z), &absinfo)) {
                mPendingEvents[Accelerometer].acceleration.z = absinfo.value * CONVERT_A_Z;
            }
        }
    }
    if (!ioctl(dev_fd, ECS_IOCTL_APP_GET_MVFLAG, &flags)) {
        if (flags)  {
            mEnabled |= 1<<MagneticField;
            if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_MAGV_X), &absinfo)) {
                mPendingEvents[MagneticField].magnetic.x = absinfo.value * CONVERT_M_X;
            }
            if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_MAGV_Y), &absinfo)) {
                mPendingEvents[MagneticField].magnetic.y = absinfo.value * CONVERT_M_Y;
            }
            if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_MAGV_Z), &absinfo)) {
                mPendingEvents[MagneticField].magnetic.z = absinfo.value * CONVERT_M_Z;
            }
        }
    }
    if (!ioctl(dev_fd, ECS_IOCTL_APP_GET_MFLAG, &flags)) {
        if (flags)  {
            mEnabled |= 1<<Orientation;
            if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_YAW), &absinfo)) {
                mPendingEvents[Orientation].orientation.azimuth = absinfo.value;
            }
            if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_PITCH), &absinfo)) {
                mPendingEvents[Orientation].orientation.pitch = absinfo.value;
            }
            if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_ROLL), &absinfo)) {
                mPendingEvents[Orientation].orientation.roll = -absinfo.value;
            }
            if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_ORIENT_STATUS), &absinfo)) {
                mPendingEvents[Orientation].orientation.status = uint8_t(absinfo.value & SENSOR_STATE_MASK);
            }
        }
    }

    // disable temperature sensor, since it is not reported
    flags = 0;
    ioctl(dev_fd, ECS_IOCTL_APP_SET_TFLAG, &flags);

    #if 1
    if(dev_fd >= 0)
    {
        unsigned int value;
        unsigned int data[3];
        int a_file = -1;

	if ((a_file = open(ACCEL_DEVICE_NAME, O_RDWR)) < 0) {
		ALOGE("open error.\n");
	}        

        if (a_file >= 0) {
            // set input device
            value = 0;
            if (ioctl(a_file, IOCTL_SENSOR_SET_INPUTDEVICE, &value) < 0) {
                ALOGE("AKD_GetSuspendStatus ioctl error.\n");
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

            close(a_file);

        }else{
            ALOGE("Device file is not opened.\n");        
        }
    }
    #endif    
    
    if (!mEnabled) {
        close_device();
    }
}

AkmSensor::~AkmSensor() {
}

int AkmSensor::enable(int32_t handle, int en)
{
    int what = -1;
    switch (handle) {
        case ID_A: what = Accelerometer; break;
        case ID_M: what = MagneticField; break;
        case ID_O: what = Orientation;   break;
#ifdef SUPPORT_BMA_TEMPERATURE	        
        case ID_T: what = Temperature; break;
#endif            
    }

    if (uint32_t(what) >= numSensors)
        return -EINVAL;

    int newState  = en ? 1 : 0;
    int err = 0;

#ifdef SUPPORT_BMA_TEMPERATURE	
    if(what == Temperature)
    {
        if(mTempSupport == 0){
            //ALOGE("AkmSensor: enable  mTemperature not support ");
            return -1;
        }
        if ((uint32_t(newState)<<what) != (mEnabledTemperature& (1<<what))) {

            if (!mEnabledTemperature) {
                mTemperature_fd = openInput("tcc-accel");
                //ALOGE("AkmSensor: enable open mTemperature_fd(%d)",mTemperature_fd);
            }

            int cmd;

            short flags = newState ;

            char buffer[20];
            int bytes;            
            bytes = sprintf(buffer, "%d\n", flags| 0x02);
            set_sysfs_input_attr(class_path,"enable",buffer,bytes);

            err = err<0 ? -errno : 0;
            ALOGE_IF(err, "ECS_IOCTL_APP_SET_XXX cmd=0x%x failed (%s) ", cmd,strerror(-err));
            if (!err) {
                mEnabledTemperature &= ~(1<<what);
                mEnabledTemperature |= (uint32_t(flags)<<what);
                //ALOGE("AkmSensor: mEnabled(%d) flags(%d) what(%d)",mEnabledTemperature,flags,what);
                //update_delay();
            }
            if (!mEnabledTemperature) {
                ALOGE("AkmSensor: enable close mTemperature_fd(%d)",mTemperature_fd);
                if (mTemperature_fd >= 0) {
                    close(mTemperature_fd);
                    mTemperature_fd = -1;
                }
            }
        }    
        return err;
    }
#endif 
     //ALOGE("AkmSensor: enable dev_fd(%d) newState(%d) what(%d) mEnabled(%d)",dev_fd,newState,what,mEnabled);

    if ((uint32_t(newState)<<what) != (mEnabled & (1<<what))) {
        if (!mEnabled) {
            open_device();
            //ALOGE("AkmSensor: open_device dev_fd(%d)",dev_fd);
        }
        int cmd;
        switch (what) {
            case Accelerometer: cmd = ECS_IOCTL_APP_SET_AFLAG;  break;
            case MagneticField: cmd = ECS_IOCTL_APP_SET_MVFLAG; break;
            case Orientation:   cmd = ECS_IOCTL_APP_SET_MFLAG;  break;
        }
        short flags = newState;
        if(what == Accelerometer){
            char buffer[20];
            int bytes;            
            bytes = sprintf(buffer, "%d\n", flags);
            set_sysfs_input_attr(class_path,"enable",buffer,bytes);

            bytes = sprintf(buffer, "%d\n", 1);

            set_sysfs_input_attr(class_path,"inputdevice",buffer,bytes);
        }
        err = ioctl(dev_fd, cmd, &flags);
        err = err<0 ? -errno : 0;
        ALOGE_IF(err, "ECS_IOCTL_APP_SET_XXX cmd=0x%x failed (%s) ", cmd,strerror(-err));
        if (!err) {
            mEnabled &= ~(1<<what);
            mEnabled |= (uint32_t(flags)<<what);
            //ALOGE("AkmSensor: mEnabled(%d) flags(%d) what(%d)",mEnabled,flags,what);
            update_delay();
        }
        if (!mEnabled) {
            //ALOGE("AkmSensor: enable mEnabled(%d)",mEnabled);
            close_device();
        }
    }
    return err;
}

int AkmSensor::setDelay(int32_t handle, int64_t ns)
{
#ifdef ECS_IOCTL_APP_SET_DELAY
    int what = -1;
    switch (handle) {
        case ID_A: what = Accelerometer; break;
        case ID_M: what = MagneticField; break;
        case ID_O: what = Orientation;   break;
        #ifdef SUPPORT_BMA_TEMPERATURE	        
        case ID_T: 
            what = Temperature; 
            return 0;
        #endif
    }

    

    if (uint32_t(what) >= numSensors)
        return -EINVAL;

    if (ns < 0)
        return -EINVAL;

    mDelays[what] = ns;
    return update_delay();
#else
    return -1;
#endif
}

int AkmSensor::update_delay()
{
    if (mEnabled) {
        uint64_t wanted = -1LLU;
        for (int i=0 ; i<numSensors ; i++) {
            if (mEnabled & (1<<i)) {
                uint64_t ns = mDelays[i];
                wanted = wanted < ns ? wanted : ns;
            }
        }
        short delay = int64_t(wanted) / 1000000;
        if (ioctl(dev_fd, ECS_IOCTL_APP_SET_DELAY, &delay)) {
            return -errno;
        }
    }
    return 0;
}

float AkmSensor::dataConvertTemp(int data)
{
    float fData = (float)data;

    fData = (fData/(float)mTempResolution) + (float)mTempCenter;
    return fData;
}

int AkmSensor::readEvents(sensors_event_t* data, int count)
{
    if (count < 1)
        return -EINVAL;

     #ifdef SUPPORT_BMA_TEMPERATURE
     //ALOGE("AkmSensor: readEvents count = %d ",count);
    if(mEnabled == 0 && mEnabledTemperature == 0)
    {
        //ALOGE("AkmSensor: readEvents mEnabled(%d)",mEnabled);
        *data++ = mDummyEvents[0];
        usleep(20*1000);
        return 1;
    }
     #else
    if(mEnabled == 0)
    {
        //ALOGE("AkmSensor: readEvents mEnabled(%d)",mEnabled);
        *data++ = mDummyEvents[0];
        usleep(20*1000);
        return 1;
    }
    #endif

    ssize_t n = mInputReader.fill(data_fd);
    #ifdef SUPPORT_BMA_TEMPERATURE
    
    if(mTemperature_fd >= 0){
       ssize_t nn= mInputReader.fill(mTemperature_fd);
        n += nn;
        //ALOGE("AkmSensor: readEvents nn(%d)",nn);
    }
    #endif
    if (n < 0){
        //ALOGE("AkmSensor: readEvents mInputReader.fill(%d)",n);
        *data++ = mDummyEvents[0];
        usleep(20*1000);
        return 1;
    }

    int numEventReceived = 0;
    input_event const* event;

    //ALOGE("AkmSensor: readEvents data_fd(%d) count(%d) n(%d)",data_fd,count,n);

    while (count && mInputReader.readEvent(&event)) {
        int type = event->type;
        //ALOGE("AkmSensor: processEvent type (%d) value (%d) mPendingMask(%d)",type,event->value,mPendingMask);
        if (type == EV_ABS) {
            processEvent(event->code, event->value);
            mInputReader.next();
        } else if (type == EV_SYN) {
            int64_t time = timevalToNano(event->time);
            for (int j=0 ; count && mPendingMask && j<numSensors ; j++) {
                if (mPendingMask & (1<<j)) {
                    mPendingMask &= ~(1<<j);
                    mPendingEvents[j].timestamp = time;
                    
                    #ifdef SUPPORT_BMA_TEMPERATURE
                    if ((mEnabled & (1<<j)) || (mEnabledTemperature&(1<<j))) {
                        //ALOGE("AkmSensor: readEvents [%f %f %f]",mPendingEvents[Accelerometer].acceleration.x,mPendingEvents[Accelerometer].acceleration.y,mPendingEvents[Accelerometer].acceleration.z);
                        *data++ = mPendingEvents[j];
                        count--;
                        numEventReceived++;
                    }
                    #else
                    if (mEnabled & (1<<j) ) {
                        //ALOGE("AkmSensor: readEvents [%f %f %f]",mPendingEvents[Accelerometer].acceleration.x,mPendingEvents[Accelerometer].acceleration.y,mPendingEvents[Accelerometer].acceleration.z);
                        *data++ = mPendingEvents[j];
                        count--;
                        numEventReceived++;
                    }
                    #endif
                }
            }
            if (!mPendingMask) {
                mInputReader.next();
            }
        } else {
            //ALOGE("AkmSensor: unknown event (type=%d, code=%d)",
            //        type, event->code);
            mInputReader.next();
        }
    }

    //ALOGE("AkmSensor: readEvents numEventReceivedl (%d)",numEventReceived);

    return numEventReceived;
}

void AkmSensor::processEvent(int code, int value)
{
    switch (code) {
        case EVENT_TYPE_ACCEL_Y:
            mPendingMask |= 1<<Accelerometer;
            mPendingEvents[Accelerometer].acceleration.x = value * CONVERT_A_X;
            break;
        case EVENT_TYPE_ACCEL_X:
            mPendingMask |= 1<<Accelerometer;
            mPendingEvents[Accelerometer].acceleration.y = value * CONVERT_A_Y;
            break;
        case EVENT_TYPE_ACCEL_Z:
            mPendingMask |= 1<<Accelerometer;
            mPendingEvents[Accelerometer].acceleration.z = value * CONVERT_A_Z;
            break;

        case EVENT_TYPE_MAGV_X:
            mPendingMask |= 1<<MagneticField;
            if(fECLayout._11 != 0)
                mPendingEvents[MagneticField].magnetic.y = value * fECLayout._11;
            else
                mPendingEvents[MagneticField].magnetic.x = -(value  * fECLayout._21);
            break;
        case EVENT_TYPE_MAGV_Y:
            mPendingMask |= 1<<MagneticField;
            if(fECLayout._22 != 0)
                mPendingEvents[MagneticField].magnetic.x = -(value * fECLayout._22);
            else
                mPendingEvents[MagneticField].magnetic.y = value * fECLayout._12;
            break;
        case EVENT_TYPE_MAGV_Z:
            mPendingMask |= 1<<MagneticField;
            mPendingEvents[MagneticField].magnetic.z = value * fECLayout._33;
            break;

        case EVENT_TYPE_YAW:
            mPendingMask |= 1<<Orientation;
            mPendingEvents[Orientation].orientation.azimuth = value * CONVERT_O_Y;
            break;
        case EVENT_TYPE_PITCH:
            mPendingMask |= 1<<Orientation;
            mPendingEvents[Orientation].orientation.pitch = value * CONVERT_O_P;
            break;
        case EVENT_TYPE_ROLL:
            mPendingMask |= 1<<Orientation;
            mPendingEvents[Orientation].orientation.roll = value * CONVERT_O_R;
            break;
        case EVENT_TYPE_ORIENT_STATUS:
            mPendingMask |= 1<<Orientation;
            {            	 
                uint8_t data = uint8_t(value & SENSOR_STATE_MASK);
                ALOGD_IF(mPendingEvents[Orientation].orientation.status != data,
                        "M-Sensor status %d", data); 
            }        
            mPendingEvents[Orientation].orientation.status =
                    uint8_t(value & SENSOR_STATE_MASK);
            break;
#ifdef SUPPORT_BMA_TEMPERATURE	            
        case EVENT_TYPE_TEMPERATURE: // temperature
            mPendingMask |= 1<<Temperature;
            mPendingEvents[Temperature].temperature = dataConvertTemp(value);
            //ALOGE("AkmSensor: temperature %d %f",value, mPendingEvents[Temperature].temperature);
            break;
#endif              
    }
}

int AkmSensor::setCalibration(float x, float y,float z)
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

int AkmSensor::startCalibration()
{
	//ALOGE(" startCalibration start");
       //ALOGE(" startCalibration end");
	return 0;
}

int AkmSensor::ak8975_GetMatrix(short *matrix)
{
	int m_file = -1;
	if(matrix == NULL)
		return -1;

	if ((m_file = open(MSENSOR_DEVICE, O_RDWR)) < 0) {
		ALOGE("open error.\n");
		return -1;
	}
	
	if (m_file < 0) {
		ALOGE("Device file is not opened.\n");
		return -1;
	}
	
	if (ioctl(m_file, ECS_IOCTL_GET_MATRIX, matrix) < 0) {
		ALOGE("AKD_GetSuspendStatus ioctl error.\n");
		return -1;
	}

	close(m_file);

	return 0;	
}

int AkmSensor::ak8975_initLayout(void)
{
	memset(&eCLayout[0],0,sizeof(eCLayout));
	if(ak8975_GetMatrix((short *)&eCLayout[0]) < 0)
	{
		eCLayout[0]._11 = 1;
		eCLayout[0]._22 = 1;
		eCLayout[0]._33 = -1;
	}
	fECLayout._11 = eCLayout[0]._11 * CONVERT_M;
	fECLayout._12 = eCLayout[0]._12 * CONVERT_M;
	fECLayout._13 = eCLayout[0]._13 * CONVERT_M;
	fECLayout._21 = eCLayout[0]._21 * CONVERT_M;
	fECLayout._22 = eCLayout[0]._22 * CONVERT_M;
	fECLayout._23 = eCLayout[0]._23 * CONVERT_M;
	fECLayout._31 = eCLayout[0]._31 * CONVERT_M;
	fECLayout._32 = eCLayout[0]._32 * CONVERT_M;
	fECLayout._33 = -(eCLayout[0]._33 * CONVERT_M);	
	//ALOGE("   [%5f][%5f][%5f] \n",fECLayout._11,fECLayout._12,fECLayout._13);	
	//ALOGE("   [%5f][%5f][%5f] \n",fECLayout._21,fECLayout._22,fECLayout._23);	
	//ALOGE("   [%5f][%5f][%5f] \n",fECLayout._31,fECLayout._32,fECLayout._33);	
	return 0;
}

int AkmSensor::hasTemperature()
{
    //ALOGE("AkmSensor::hasTemperature = %d.\n",mTempSupport);
	return (mTempSupport == 0)? -1:1;
}

