/*
 * Copyright (C) 2008, 2009 Telechips, Inc.
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
//#define LOG_NDEBUG    0
#define LOG_TAG "lights"

#include <fcntl.h>
#include <errno.h>
#include <cutils/log.h>
#include <cutils/atomic.h>
#include <stdlib.h>
#include <pthread.h>

#include <hardware/hardware.h>
#include <hardware/lights.h>

#define LCD_FILE        "/sys/class/leds/lcd-backlight/brightness"
#define BUTTON_FILE	"/sys/class/leds/button-backlight/brightness"
#define KEYBOARD_FILE	"/sys/class/leds/keyboard-backlight/brightness"

static pthread_once_t g_init = PTHREAD_ONCE_INIT;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

struct light_context_t {
    struct light_device_t device;

    /* our private state goes below here */
    int fd;
    
};

static int light_device_open(const struct hw_module_t* module, const char* name,
                             struct hw_device_t** device);

static struct hw_module_methods_t lights_module_methods = {
    open: light_device_open
};

struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = LIGHTS_HARDWARE_MODULE_ID,
    .name = "Telechips lights module",
    .author = "Telechips, Inc.",
    .methods = &lights_module_methods,
};

void init_globals(void)
{
    pthread_mutex_init(&g_lock, NULL);
}

static int write_int(int fd, int value)
{
    static int already_warned = 0;

    if (fd >= 0) {
        char buffer[20];
        int bytes = sprintf(buffer, "%d\n", value);
        int nwritten = write(fd, buffer, bytes);
        return nwritten > 0 ? 0 : -errno;
    } else {
        if (already_warned == 0) {
            ALOGE("write failed (%s)\n", strerror(errno));
            already_warned = 1;
        }
        return -errno;
    }
}

static int rgb_to_brightness(struct light_state_t const* state)
{
    int color = state->color & 0x00ffffff;
    return ((77*((color>>16)&0x00ff))
            + (150*((color>>8)&0x00ff)) + (29*(color&0x00ff))) >> 8;
}

static int set_light_backlight(struct light_device_t* dev,
                               struct light_state_t const* state)
{
    struct light_context_t *ctx = (struct light_context_t *) dev;
    int level;

    level = rgb_to_brightness(state);

    ALOGV("%s: level=%d", __func__, level);

    return write_int(ctx->fd, level);
}

static int set_light_keyboard(struct light_device_t* dev,
                              struct light_state_t const* state)
{
    return 0;
}

static int set_light_buttons(struct light_device_t* dev,
                             struct light_state_t const* state)
{
    return 0;
}

static int set_light_battery(struct light_device_t* dev,
                             struct light_state_t const* state)
{
    return 0;
}

static int set_light_notifications(struct light_device_t* dev,
                                   struct light_state_t const* state)
{
    return 0;
}

static int set_light_attention(struct light_device_t* dev,
                               struct light_state_t const* state)
{
    return 0;
}

static int light_device_close(struct hw_device_t *dev)
{
    struct light_context_t *ctx = (struct light_context_t *) dev;
      ALOGI(" (%s) fd:%d \n",__func__, ctx->fd);

    if (ctx) {
        /* free all resources associated with this device here */
        if (ctx->fd >= 0)
            close(ctx->fd);
        free(ctx);
    }
    return 0;
}


static int light_device_open(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device)
{
    int (*set_light)(struct light_device_t *dev,
                     struct light_state_t const* staet);
    struct light_context_t *ctx;
    char *file;
    int status = -EINVAL;
    int fd;

    if (!strcmp(LIGHT_ID_BACKLIGHT, name)) {
        file = LCD_FILE;
        set_light = set_light_backlight;
    } else if (!strcmp(LIGHT_ID_KEYBOARD, name)) {
        file = 0;
        set_light = set_light_keyboard;
    } else if (!strcmp(LIGHT_ID_BUTTONS, name)) {
        file = 0;
        set_light = set_light_buttons;
    } else if (!strcmp(LIGHT_ID_BATTERY, name)) {
        file = 0;
        set_light = set_light_battery;
    } else if (!strcmp(LIGHT_ID_NOTIFICATIONS, name)) {
        file = 0;
        set_light = set_light_notifications;
    } else if (!strcmp(LIGHT_ID_ATTENTION, name)) {
        file = 0;
        set_light = set_light_attention;
    } else {
        return -EINVAL;
    }

    if (file) {
        fd = open(file, O_RDWR);
        if (fd < 0) {
            ALOGE("can't open file '%s' (%s)", file, strerror(errno));
            return -EINVAL;
        }
    } else {
        fd = -1;
    }

    pthread_once(&g_init, init_globals);

    ctx = (struct light_context_t *) malloc(sizeof(struct light_context_t));
    if (ctx == NULL) {
        ALOGE("can't allocate memory");
        return ENOMEM;
    }

    /* initialize our state here */
    memset(ctx, 0, sizeof(*ctx));

      ALOGI(" (%s) fd:%d \n",__func__, ctx->fd);
	  
    /* initialize the procs */
    ctx->device.common.tag = HARDWARE_DEVICE_TAG;
    ctx->device.common.version = 0;
    ctx->device.common.module = (struct hw_module_t*)(module);
    ctx->device.common.close = light_device_close;
    ctx->device.set_light = set_light;
    ctx->fd = fd;

    *device = &ctx->device.common;
    return 0;
}
