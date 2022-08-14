/*
**
** Copyright 2009, Telechips, Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*
* Company: Telechips.co.Ltd
* Author: lukas
*/


#define LOG_TAG "TCC_CAMERA_MODULE"

#include "TelechipsCameraHardware.h"
#include <cutils/properties.h>


static int DEBUG 	= 0;
#define LOG_D(msg...) 	if(DEBUG) { ALOGD(" " msg); }
#define LOG_I(msg...) 	if(DEBUG) { ALOGI(" " msg); }
#define FUNCTION_IN 	LOG_I("%d:  %s() - In", __LINE__, __FUNCTION__);
#define FUNCTION_OUT 	LOG_I("%d:  %s() - Out", __LINE__, __FUNCTION__);

#if 0

#if defined(DUAL_CAMERA_SUPPORT)
static camera_info sCameraInfo[] = {
	{ CAMERA_FACING_BACK,  0 /* orientation  :   0, 90, 180, 270 */ },
	{ CAMERA_FACING_FRONT, 0 /* orientation  :   0, 90, 180, 270 */ }
};
#else
static camera_info sCameraInfo[] = {
	{ CAMERA_FACING_FRONT, 0 /* orientation  :   0, 90, 180, 270 */ }
};
#endif

#endif

static android::TelechipsCameraHardware* 	gTCCCameraHals[MAX_CAMERAS_SUPPORTED];
static unsigned int 						gTCCCamerasOpen = 0;
static android::Mutex 						gTCCCameraHalDeviceLock;

static int tcc_camera_device_open(const hw_module_t* module, const char* name, hw_device_t** device);
static int tcc_camera_device_close(hw_device_t* device);
static int tcc_camera_get_number_of_cameras(void);
static int tcc_camera_get_camera_info(int camera_id, struct camera_info *info);
static int tcc_camera_open_flag=0;

static struct hw_module_methods_t tcc_camera_module_methods = {
	open: tcc_camera_device_open
};

camera_module_t HAL_MODULE_INFO_SYM = {
	common: {
		tag: HARDWARE_MODULE_TAG,
		version_major: 1,
		version_minor: 0,
		id: CAMERA_HARDWARE_MODULE_ID,
		name: "Telechips Camera Hal Module",
		author: "Telechips",
		methods: &tcc_camera_module_methods,
		dso: NULL, 		/* remove compilation warnings */
		reserved: {0}, 	/* remove compilation warnings */
	},
	get_number_of_cameras: tcc_camera_get_number_of_cameras,
	get_camera_info: tcc_camera_get_camera_info,
};

typedef struct tcc_camera_device {
    camera_device_t base;
    int cameraid;
} tcc_camera_device_t;


/*******************************************************************
 * implementation of camera_device_ops functions
 *******************************************************************/

int tcc_camera_set_preview_window(struct camera_device * device, struct preview_stream_ops *window)
{
    int rv = -EINVAL;
    tcc_camera_device_t* tcc_dev = NULL;

    FUNCTION_IN

    if(!device) return rv;
    tcc_dev = (tcc_camera_device_t*) device;
    rv = gTCCCameraHals[tcc_dev->cameraid]->setPreviewWindow(window);

	FUNCTION_OUT

    return rv;
}

void tcc_camera_set_callbacks(struct camera_device * device, camera_notify_callback notify_cb,
									camera_data_callback data_cb, camera_data_timestamp_callback data_cb_timestamp,
									camera_request_memory get_memory, void *user)
{
    tcc_camera_device_t* tcc_dev = NULL;

    FUNCTION_IN

    if(!device) return;
    tcc_dev = (tcc_camera_device_t*) device;
    gTCCCameraHals[tcc_dev->cameraid]->setCallbacks(notify_cb, data_cb, data_cb_timestamp, get_memory, user);

	FUNCTION_OUT
}

void tcc_camera_enable_msg_type(struct camera_device * device, int32_t msg_type)
{
    tcc_camera_device_t* tcc_dev = NULL;

    FUNCTION_IN

    if(!device) return;
    tcc_dev = (tcc_camera_device_t*) device;
    gTCCCameraHals[tcc_dev->cameraid]->enableMsgType(msg_type);

	FUNCTION_OUT
}

void tcc_camera_disable_msg_type(struct camera_device * device, int32_t msg_type)
{
    tcc_camera_device_t* tcc_dev = NULL;

    FUNCTION_IN

    if(!device) return;
    tcc_dev = (tcc_camera_device_t*) device;
    gTCCCameraHals[tcc_dev->cameraid]->disableMsgType(msg_type);

	FUNCTION_OUT
}

int tcc_camera_msg_type_enabled(struct camera_device * device, int32_t msg_type)
{
	int rv = -EINVAL;
    tcc_camera_device_t* tcc_dev = NULL;

    FUNCTION_IN

	if(!device) return 0;
    tcc_dev = (tcc_camera_device_t*) device;
    rv = gTCCCameraHals[tcc_dev->cameraid]->msgTypeEnabled(msg_type);

	FUNCTION_OUT

	return rv;
}

int tcc_camera_start_preview(struct camera_device * device)
{
    int rv = -EINVAL;
    tcc_camera_device_t* tcc_dev = NULL;

    FUNCTION_IN

    if(!device) return rv;
    tcc_dev = (tcc_camera_device_t*) device;
    rv = gTCCCameraHals[tcc_dev->cameraid]->startPreview();

	FUNCTION_OUT

    return rv;
}

void tcc_camera_stop_preview(struct camera_device * device)
{
    tcc_camera_device_t* tcc_dev = NULL;

    FUNCTION_IN

    if(!device) return;
    tcc_dev = (tcc_camera_device_t*) device;
    gTCCCameraHals[tcc_dev->cameraid]->stopPreview();

	FUNCTION_OUT
}

int tcc_camera_preview_enabled(struct camera_device * device)
{
    int rv = -EINVAL;
    tcc_camera_device_t* tcc_dev = NULL;

    FUNCTION_IN

    if(!device) return rv;
    tcc_dev = (tcc_camera_device_t*) device;
    rv = gTCCCameraHals[tcc_dev->cameraid]->previewEnabled();

	FUNCTION_OUT

    return rv;
}

int tcc_camera_store_meta_data_in_buffers(struct camera_device * device, int enable)
{
    int rv = -EINVAL;
    tcc_camera_device_t* tcc_dev = NULL;

    FUNCTION_IN

    if(!device) return rv;
    tcc_dev = (tcc_camera_device_t*) device;
    rv = gTCCCameraHals[tcc_dev->cameraid]->storeMetaDataInBuffers(enable);

	FUNCTION_OUT

    return rv;
}

int tcc_camera_start_recording(struct camera_device * device)
{
    int rv = -EINVAL;
    tcc_camera_device_t* tcc_dev = NULL;

    FUNCTION_IN

    if(!device) return rv;
    tcc_dev = (tcc_camera_device_t*) device;
    rv = gTCCCameraHals[tcc_dev->cameraid]->startRecording();

	FUNCTION_OUT

    return rv;
}

void tcc_camera_stop_recording(struct camera_device * device)
{
    tcc_camera_device_t* tcc_dev = NULL;

    FUNCTION_IN

    if(!device) return;
    tcc_dev = (tcc_camera_device_t*) device;
    gTCCCameraHals[tcc_dev->cameraid]->stopRecording();

	FUNCTION_OUT
}

int tcc_camera_recording_enabled(struct camera_device * device)
{
    int rv = -EINVAL;
    tcc_camera_device_t* tcc_dev = NULL;

    FUNCTION_IN

    if(!device) return rv;
    tcc_dev = (tcc_camera_device_t*) device;
    rv = gTCCCameraHals[tcc_dev->cameraid]->recordingEnabled();

	FUNCTION_OUT

    return rv;
}

void tcc_camera_release_recording_frame(struct camera_device * device, const void *opaque)
{
    tcc_camera_device_t* tcc_dev = NULL;

    FUNCTION_IN

    if(!device) return;
    tcc_dev = (tcc_camera_device_t*) device;
    gTCCCameraHals[tcc_dev->cameraid]->releaseRecordingFrame(opaque);

	FUNCTION_OUT
}

int tcc_camera_auto_focus(struct camera_device * device)
{
    int rv = -EINVAL;
    tcc_camera_device_t* tcc_dev = NULL;

    FUNCTION_IN

    if(!device) return rv;
    tcc_dev = (tcc_camera_device_t*) device;
    rv = gTCCCameraHals[tcc_dev->cameraid]->autoFocus();

    FUNCTION_OUT

    return rv;
}

int tcc_camera_cancel_auto_focus(struct camera_device * device)
{
    int rv = -EINVAL;
    tcc_camera_device_t* tcc_dev = NULL;

        FUNCTION_IN

    if(!device) return rv;
    tcc_dev = (tcc_camera_device_t*) device;
    rv = gTCCCameraHals[tcc_dev->cameraid]->cancelAutoFocus();

    FUNCTION_OUT

    return rv;
}

int tcc_camera_take_picture(struct camera_device * device)
{
    int rv = -EINVAL;
    tcc_camera_device_t* tcc_dev = NULL;

        FUNCTION_IN

    if(!device) return rv;
    tcc_dev = (tcc_camera_device_t*) device;
    rv = gTCCCameraHals[tcc_dev->cameraid]->takePicture();

    FUNCTION_OUT

    return rv;
}

int tcc_camera_cancel_picture(struct camera_device * device)
{
    int rv = -EINVAL;
    tcc_camera_device_t* tcc_dev = NULL;

        FUNCTION_IN

    if(!device) return rv;
    tcc_dev = (tcc_camera_device_t*) device;
    rv = gTCCCameraHals[tcc_dev->cameraid]->cancelPicture();

    FUNCTION_OUT

    return rv;
}

int tcc_camera_set_parameters(struct camera_device * device, const char *params)
{
    int rv = -EINVAL;
    tcc_camera_device_t* tcc_dev = NULL;

    FUNCTION_IN

    if(!device) return rv;
    tcc_dev = (tcc_camera_device_t*) device;
    rv = gTCCCameraHals[tcc_dev->cameraid]->setParameters(params);

    FUNCTION_OUT

    return rv;
}

char* tcc_camera_get_parameters(struct camera_device * device)
{
    char* param = NULL;
    tcc_camera_device_t* tcc_dev = NULL;

    FUNCTION_IN

    if(!device) return NULL;
    tcc_dev = (tcc_camera_device_t*) device;
    param = gTCCCameraHals[tcc_dev->cameraid]->getParameters();

    FUNCTION_OUT

    return param;
}

static void tcc_camera_put_parameters(struct camera_device *device, char *parms)
{
    tcc_camera_device_t* tcc_dev = NULL;

    FUNCTION_IN

    if(!device) return;
    tcc_dev = (tcc_camera_device_t*) device;
    gTCCCameraHals[tcc_dev->cameraid]->putParameters(parms);

    FUNCTION_OUT
}

int tcc_camera_send_command(struct camera_device * device, int32_t cmd, int32_t arg1, int32_t arg2)
{
    int rv = -EINVAL;
    tcc_camera_device_t* tcc_dev = NULL;

    FUNCTION_IN

    if(!device) return rv;
    tcc_dev = (tcc_camera_device_t*) device;
    rv = gTCCCameraHals[tcc_dev->cameraid]->sendCommand(cmd, arg1, arg2);

    FUNCTION_OUT

    return rv;
}

void tcc_camera_release(struct camera_device * device)
{
    tcc_camera_device_t* tcc_dev = NULL;

    FUNCTION_IN


    if(!device) return;
    tcc_dev = (tcc_camera_device_t*) device;
    gTCCCameraHals[tcc_dev->cameraid]->release();
	ALOGE("camera %d release !!!", tcc_dev->cameraid);
	tcc_camera_open_flag = 0;
    FUNCTION_OUT
}

int tcc_camera_dump(struct camera_device * device, int fd)
{
    int rv = -EINVAL;
    tcc_camera_device_t* tcc_dev = NULL;

    FUNCTION_IN

    if(!device) return rv;
    tcc_dev = (tcc_camera_device_t*) device;
    rv = gTCCCameraHals[tcc_dev->cameraid]->dump(fd);

    FUNCTION_OUT

    return rv;
}

extern "C" void tcc_heaptracker_free_leaked_memory(void);
int tcc_camera_device_close(hw_device_t* device)
{
    int ret = 0;
    tcc_camera_device_t* tcc_dev = NULL;

    FUNCTION_IN

    android::Mutex::Autolock lock(gTCCCameraHalDeviceLock);
    if(!device) {
		ALOGE("camera close error.");
        ret = -EINVAL;
        goto done;
    }

    tcc_dev = (tcc_camera_device_t*) device;
    if(tcc_dev) {
        if(gTCCCameraHals[tcc_dev->cameraid]) {
            delete gTCCCameraHals[tcc_dev->cameraid];
            gTCCCameraHals[tcc_dev->cameraid] = NULL;
            gTCCCamerasOpen--;
        }

        if(tcc_dev->base.ops) {
            free(tcc_dev->base.ops);
        }
        free(tcc_dev);
    }

done:
	#ifdef HEAPTRACKER
    heaptracker_free_leaked_memory();
	#endif

	FUNCTION_OUT

    return ret;
}

/*******************************************************************
 * implementation of camera_module functions
 *******************************************************************/

/* open device handle to one of the cameras
 *
 * assume camera service will keep singleton of each camera
 * so this function will always only be called once per camera instance
 */

int tcc_camera_device_open(const hw_module_t* module, const char* name, hw_device_t** device)
{
    int rv = 0;
    int num_cameras = 0;
    int cameraid;
    //tcc_camera_device_t* camera_device = NULL;
    //camera_device_ops_t* camera_ops = NULL;
    //android::TelechipsCameraHardware* camera = NULL;

    android::Mutex::Autolock lock(gTCCCameraHalDeviceLock);

    FUNCTION_IN
	if(tcc_camera_open_flag)
	{
		ALOGE("camera %d open error.", cameraid);
		rv = -EINVAL;
		return rv;
	}
    tcc_camera_device_t* camera_device = NULL;
    camera_device_ops_t* camera_ops = NULL;
    android::TelechipsCameraHardware* camera = NULL;

    if(name != NULL) {
        cameraid = atoi(name);
        num_cameras = MAX_CAMERAS_SUPPORTED;
		ALOGE("camera %d open !!!", cameraid);
        if(cameraid > num_cameras) {
            ALOGE("camera service provided cameraid out of bounds, "
                    "cameraid = %d, num supported = %d",
                    cameraid, num_cameras);
            rv = -EINVAL;
            goto fail;
        }

        if(gTCCCamerasOpen > MAX_CAMERAS_SUPPORTED) {
            ALOGE("maximum number of cameras already open");
            rv = -ENOMEM;
            goto fail;
        }

        camera_device = (tcc_camera_device_t*)malloc(sizeof(*camera_device));
        if(!camera_device) {
            ALOGE("camera_device allocation fail");
            rv = -ENOMEM;
            goto fail;
        }

        camera_ops = (camera_device_ops_t*)malloc(sizeof(*camera_ops));
        if(!camera_ops) {
            ALOGE("camera_ops allocation fail");
            rv = -ENOMEM;
            goto fail;
        }

        memset(camera_device, 0, sizeof(*camera_device));
        memset(camera_ops, 0, sizeof(*camera_ops));

        camera_device->base.common.tag 			= HARDWARE_DEVICE_TAG;
        camera_device->base.common.version 		= 0;
        camera_device->base.common.module 		= (hw_module_t *)(module);
        camera_device->base.ops 				= camera_ops;

        camera_device->base.common.close 		= tcc_camera_device_close;
        camera_ops->set_preview_window 			= tcc_camera_set_preview_window;
        camera_ops->set_callbacks 				= tcc_camera_set_callbacks;
        camera_ops->enable_msg_type 			= tcc_camera_enable_msg_type;
        camera_ops->disable_msg_type 			= tcc_camera_disable_msg_type;
        camera_ops->msg_type_enabled 			= tcc_camera_msg_type_enabled;
        camera_ops->start_preview 				= tcc_camera_start_preview;
        camera_ops->stop_preview 				= tcc_camera_stop_preview;
        camera_ops->preview_enabled 			= tcc_camera_preview_enabled;
        camera_ops->store_meta_data_in_buffers 	= tcc_camera_store_meta_data_in_buffers;
        camera_ops->start_recording 			= tcc_camera_start_recording;
        camera_ops->stop_recording 				= tcc_camera_stop_recording;
        camera_ops->recording_enabled 			= tcc_camera_recording_enabled;
        camera_ops->release_recording_frame 	= tcc_camera_release_recording_frame;
        camera_ops->auto_focus 					= tcc_camera_auto_focus;
        camera_ops->cancel_auto_focus 			= tcc_camera_cancel_auto_focus;
        camera_ops->take_picture 				= tcc_camera_take_picture;
        camera_ops->cancel_picture 				= tcc_camera_cancel_picture;
        camera_ops->set_parameters 				= tcc_camera_set_parameters;
        camera_ops->get_parameters 				= tcc_camera_get_parameters;
        camera_ops->put_parameters 				= tcc_camera_put_parameters;
        camera_ops->send_command 				= tcc_camera_send_command;
        camera_ops->release 					= tcc_camera_release;
        camera_ops->dump 						= tcc_camera_dump;

        *device = &camera_device->base.common;
        camera_device->cameraid = cameraid;

		camera = new android::TelechipsCameraHardware(cameraid);
        if(!camera) {
            ALOGE("Couldn't create instance of TelechipsCameraHardware class");
            rv = -ENOMEM;
            goto fail;
        }

        gTCCCameraHals[cameraid] = camera;
        gTCCCamerasOpen++;
    }
	tcc_camera_open_flag = 1;
	FUNCTION_OUT

    return rv;

fail:
    if(camera_device) {
        free(camera_device);
        camera_device = NULL;
    }
    if(camera_ops) {
        free(camera_ops);
        camera_ops = NULL;
    }
    if(camera) {
        delete camera;
        camera = NULL;
    }
    *device = NULL;
	tcc_camera_open_flag = 0;
    return rv;
}

int tcc_camera_get_number_of_cameras(void)
{
#if 0 //swhwang 20140206 fixed to get camera info 

	return sizeof(sCameraInfo) / sizeof(sCameraInfo[0]);
	
#else

	#if defined(DUAL_CAMERA_SUPPORT)
		return 2;
	#else
		return 1;
	#endif

#endif

}

int tcc_camera_get_camera_info(int camera_id, struct camera_info *info)
{
#if 0 //swhwang 20140206 fixed to get camera info 
	memcpy(info, &sCameraInfo[camera_id], sizeof(camera_info));
	return 0;
#else

	struct camera_info cam_info;

	// set camera facing
	#if defined(DUAL_CAMERA_SUPPORT)
		if(camera_id)	cam_info.facing = CAMERA_FACING_FRONT;
		else			cam_info.facing = CAMERA_FACING_BACK;
	#else
		cam_info.facing = CAMERA_FACING_FRONT;
	#endif
	
	//set orientation
	char orientation[PROPERTY_VALUE_MAX];
        property_get("persist.sys.display_orientation", orientation, "");
	cam_info.orientation = atoi(orientation);

	// copy camera information
	memcpy(info, &cam_info, sizeof(camera_info));	

	return 0;
	
#endif		
}





