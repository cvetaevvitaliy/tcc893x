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
#include <stdlib.h>
#include <sys/select.h>

#include <cutils/log.h>

#include <linux/input.h>

#include "SensorBase.h"

/*****************************************************************************/

SensorBase::SensorBase(
        const char* dev_name,
        const char* data_name)
    : dev_name(dev_name), data_name(data_name),
      dev_fd(-1), data_fd(-1)
{
    data_fd = openInput(data_name);
}

SensorBase::~SensorBase() {
    if (data_fd >= 0) {
        close(data_fd);
    }
    if (dev_fd >= 0) {
        close(dev_fd);
    }
}

int SensorBase::open_device() {
    if (dev_fd<0 && dev_name) {
        dev_fd = open(dev_name, O_RDONLY);
        //ALOGE_IF(dev_fd<0, "Couldn't open %s (%s)", dev_name, strerror(errno));
    }
    return 0;
}

int SensorBase::close_device() {
    if (dev_fd >= 0) {
        close(dev_fd);
        dev_fd = -1;
    }
    return 0;
}

int SensorBase::getFd() const {
    return data_fd;
}

int SensorBase::setDelay(int32_t handle, int64_t ns) {
    return 0;
}

bool SensorBase::hasPendingEvents() const {
    return false;
}

int64_t SensorBase::getTimestamp() {
    struct timespec t;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return int64_t(t.tv_sec)*1000000000LL + t.tv_nsec;
}

int SensorBase::openInput(const char* inputName) {
    int fd = -1;
    const char *dirname = "/dev/input";
    char devname[PATH_MAX];
    char *filename;
    DIR *dir;
    struct dirent *de;
    dir = opendir(dirname);
    if(dir == NULL)
        return -1;
    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';
    while((de = readdir(dir))) {
        if(de->d_name[0] == '.' &&
                (de->d_name[1] == '\0' ||
                        (de->d_name[1] == '.' && de->d_name[2] == '\0')))
            continue;
        strcpy(filename, de->d_name);
        fd = open(devname, O_RDONLY);
        if (fd>=0) {
            char name[80];
            if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
                name[0] = '\0';
            }
            if (!strcmp(name, inputName)) {
                break;
            } else {
                close(fd);
                fd = -1;
            }
        }
    }
    closedir(dir);
    ALOGE_IF(fd<0, "couldn't find '%s' input device", inputName);
    return fd;
}

int SensorBase::sensor_get_class_path()
{
	char *dirname = "/sys/class/input";
	char buf[256];
	int res;
	DIR *dir;
	struct dirent *de;
	int fd = -1;
	int found = 0;

	dir = opendir(dirname);
	if (dir == NULL)
		return -1;

	while((de = readdir(dir))) {
		if (strncmp(de->d_name, "input", strlen("input")) != 0) {
		    continue;
        	}

		sprintf(class_path, "%s/%s", dirname, de->d_name);
		snprintf(buf, sizeof(buf), "%s/name", class_path);

		fd = open(buf, O_RDONLY);
		if (fd < 0) {
		    continue;
		}
		if ((res = read(fd, buf, sizeof(buf))) < 0) {
		    close(fd);
		    continue;
		}
		buf[res - 1] = '\0';
		if (strcmp(buf, "tcc-accel") == 0) {
		    found = 1;
		    close(fd);
		    break;
		}

		close(fd);
		fd = -1;
	}
	closedir(dir);

	if (found) {
		return 0;
	}else {
		*class_path = '\0';
		return -1;
	}

}

int SensorBase::set_sysfs_input_attr(char *class_path,
				const char *attr, char *value, int len)
{
	char path[256];
	int fd;

	if (class_path == NULL || *class_path == '\0'
	    || attr == NULL || value == NULL || len < 1) {
		return -EINVAL;
	}
	snprintf(path, sizeof(path), "%s/%s", class_path, attr);
	path[sizeof(path) - 1] = '\0';
	fd = open(path, O_RDWR);
	if (fd < 0) {
		return -errno;
	}
	if (write(fd, value, len) < 0) {
		close(fd);
		return -errno;
	}
	close(fd);

	return 0;
}

int SensorBase::get_sysfs_input_attr_show(char *class_path,
				const char *attr, int *value)
{
	char path[256];
       char buff[22];
	int fd;
       int len = 20;
       int tmp = 0;

	if (class_path == NULL || *class_path == '\0'
	    || attr == NULL || value == NULL) {
		return -EINVAL;
	}
       memset(buff,0,sizeof(buff));
	snprintf(path, sizeof(path), "%s/%s", class_path, attr);
	path[sizeof(path) - 1] = '\0';
	fd = open(path, O_RDWR);
	if (fd < 0) {
		return -errno;
	}
	if (read(fd, buff, len) < 0) {
		close(fd);
		return -errno;
	}
       tmp = atoi(buff);
       //ALOGE("AccelSensor: readAttr buff(%s) value(%d)",buff,tmp);
       *value = tmp;
	close(fd);

	return 0;
}

int SensorBase::get_sysfs_input_attr_str(char *class_path,
				const char *attr, char *str, int len)
{
	char path[256];
       //char buff[256];
	int fd;
       int ret = 0;

	if (class_path == NULL || *class_path == '\0'
	    || attr == NULL || str == NULL || len <= 0) {
		return -EINVAL;
	}
       memset(str,0,len);
	snprintf(path, sizeof(path), "%s/%s", class_path, attr);
	path[sizeof(path) - 1] = '\0';
	fd = open(path, O_RDWR);
	if (fd < 0) {
		return -errno;
	}
       ret = read(fd, str, len);
	if (ret < 0) {
		close(fd);
		return -errno;
	}
       //ALOGE("AccelSensor:  read ret = %d",ret);
	close(fd);
       //ALOGE("AccelSensor: ret(%d) buff(%s) str(%s)",ret,buff,str);

        return 0;
}

int SensorBase::LoadShort(void * data, char *lpKeyName, short * val)
{
	int tmp = 0;
	int ret = 0;
	char buf[64] = { '\0' };
       FILE * fp = (FILE *)data;
	
	if ( fscanf(fp, "%63s" DELIMITER "%d", buf, &tmp) != 2) {
		//ALOGE( "File read error.\n");
		return 0;
	}
	if (strncmp(buf, lpKeyName, sizeof(buf)) != 0) {
		//ALOGE( "%s read error.\n", lpKeyName);
		return 0;
	}
	*val = (short) tmp;
	
	return 1;
}

int SensorBase::LoadCalibrationPara(tcc_sensor_accel_t *para)
{
	int ret;
	short temp = 0;
	FILE *fp = NULL;

	if(para == NULL)
		return 0;
	para->x = 0;
	para->y = 0;
	para->z = 0;
		
	if ((fp = fopen(CALIBRATION_DATA_FILE, "r")) == NULL) {
		//ALOGE( "%s Setting file open error.\n", __FUNCTION__);
		return -1;
	}
	
	ret = 1;
	temp = 0;
	ret = ret && LoadShort((void *)fp, "X", &temp);
	para->x = temp;
	temp = 0;
	ret = ret && LoadShort((void *)fp, "Y", &temp);
	para->y = temp;
	temp = 0;
	ret = ret && LoadShort((void *)fp, "Z", &temp);
	para->z = temp;

	if (fclose(fp) != 0) {
		//ALOGE( "%s Setting file close error.\n", __FUNCTION__);
		ret = 0;
	}

	if(ret == 0)
	{
		para->x = 0;
		para->y = 0;
		para->z = 0;
	}

	ALOGD( " %s : Calibration X = %d, Y=%d , Z=%d.\n", __FUNCTION__,para->x,para->y,para->z);
	
	return ret;
}

int SensorBase::SetCalibDataToDriver(tcc_sensor_accel_t para)
{
    char buffer[64];
    int bytes;

    //ALOGE("        SensorBase setCalibration   start (%d)(%d)(%d)",para.x,para.y,para.z);

    /*
    bytes = sprintf(buffer, "%d\n", (int)para.x);
    set_sysfs_input_attr(class_path,"calibx",buffer,bytes);
        
    bytes = sprintf(buffer, "%d\n", (int)para.y);
    set_sysfs_input_attr(class_path,"caliby",buffer,bytes);

    bytes = sprintf(buffer, "%d\n", (int)para.z);
    set_sysfs_input_attr(class_path,"calibz",buffer,bytes);
    */
    bytes = sprintf(buffer, "%d %d %d\n",(int)para.x,(int)para.y, (int)para.z);
    set_sysfs_input_attr(class_path,"calibration",buffer,bytes);

    //ALOGE("        SensorBase setCalibration  end (%d)(%d)(%d)",para.x,para.y,para.z);

    return 1;
}

int SensorBase::SaveShort(void *data, char *lpKeyName, short val)
{
       FILE * fp = (FILE *)data;
	if (fprintf(fp, "%s" DELIMITER "%d\n", lpKeyName, val) < 0) {
		//ALOGE( "%s write error.\n", lpKeyName);
		return 0;
	} else {
		return 1;
	}
}

int SensorBase::SaveCalibrationPara(tcc_sensor_accel_t para)
{
	int ret = 0;
	FILE *fp;
	
	if ((fp = fopen(CALIBRATION_DATA_FILE, "w")) == NULL) {
		//ALOGE( "%s Setting file open error.\n", __FUNCTION__);
		return 0;
	}

	ret = 1;
	ret = ret && SaveShort((void *)fp, "X", para.x);
	ret = ret && SaveShort((void *)fp, "Y", para.y);
	ret = ret && SaveShort((void *)fp, "Z", para.z);
	
	if (fclose(fp) != 0) {
		//ALOGE( "%s Setting file close error.\n", __FUNCTION__);
		ret = 0;
	}
	//ALOGD( " %s : Calibration X = %d, Y=%d , Z=%d ret = %d.\n", __FUNCTION__,para.x,para.y,para.z,ret);
	
	return ret;
}

int SensorBase::set_HwCalbration() 
{
    int value = 0;
    char buffer[20];
    int bytes = 20;

    if(get_sysfs_input_attr_show(class_path,"autocalibration", &value) < 0){
        //ALOGE( "%s cann't get attribute \n", __FUNCTION__);
        return -1;
    }

    if(value <= 0 ){
        //ALOGE( "%s Hw Calibration doesn't support.\n", __FUNCTION__);
        return -1;
    }

    if( set_sysfs_input_attr(class_path,"autocalibration",buffer,bytes) < 0){
        //ALOGE( "%s Hw Calibration failed \n", __FUNCTION__);
        return -1;
    }

    //ALOGE( "%s Hw Calibration success \n", __FUNCTION__);
    
    return 0;
}

int SensorBase::hasTemperature()
{
    return -1;
}
