/*
 * Copyright (C) 2007 The Android Open Source Project
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
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <cutils/android_reboot.h>
#include "minui.h"
#include "factory_ui.h"

#include "ui.h"

#define MAX_COLS 96
#define MAX_ROWS 32

#define CHAR_WIDTH 10
#define CHAR_HEIGHT 18

#define UI_WAIT_KEY_TIMEOUT_SEC    120


static pthread_mutex_t gUpdateMutex = PTHREAD_MUTEX_INITIALIZER;
static gr_surface gImageIcon[NUM_IMAGE_ICONS];

static const struct { gr_surface* surface; const char *name; } BITMAPS[] = {
    { &gImageIcon[TEST_IMAGE_0], "test_image0" },
    { &gImageIcon[TEST_IMAGE_1], "test_image1" },
    { &gImageIcon[TEST_IMAGE_2], "test_image2" },
    { &gImageIcon[TEST_IMAGE_3], "test_image3" },
    { &gImageIcon[TEST_IMAGE_4], "test_image4" },
    { &gImageIcon[TEST_IMAGE_5], "test_image5" },
    { &gImageIcon[TEST_IMAGE_6], "test_image6" },
    { NULL, NULL },
};

static int gCurrentIcon = 0;
static int gPagesIdentical = 0;

static char text[MAX_ROWS][MAX_COLS];
static int text_cols = 0, text_rows = 0;
static int text_col = 0, text_row = 0, text_top = 0;
static int show_text = 0;
static int show_text_ever = 0;   // has show_text ever been 1?

static char menu[MAX_ROWS][MAX_COLS];
static int menu_info[MAX_ROWS] = {0};
static int show_menu = 0;
static int menu_top = 0, menu_items = 0, menu_sel = 0;

// Key event input queue
static pthread_mutex_t key_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t key_queue_cond = PTHREAD_COND_INITIALIZER;
static int key_queue[256], key_queue_len = 0;
static volatile char key_pressed[KEY_MAX + 1];

static int rel_sum = 0;

// Return the current time as a double (including fractions of a second).
static double now() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// Clear the screen and draw the currently selected background icon (if any).
// Should only be called with gUpdateMutex locked.
static void draw_image_locked(int icon)
{
    gPagesIdentical = 0;
    gr_color(0, 0, 0, 255);
    gr_fill(0, 0, gr_fb_width(), gr_fb_height());

    if (icon) {
        gr_surface surface = gImageIcon[icon];
        int iconWidth = gr_get_width(surface);
        int iconHeight = gr_get_height(surface);
        int iconX = (gr_fb_width() - iconWidth) / 2;
        int iconY = (gr_fb_height() - iconHeight) / 2;
        gr_blit(surface, 0, 0, iconWidth, iconHeight, iconX, iconY);
    }
}

static void draw_text_line(int row, const char* t) {
  if (t[0] != '\0') {
    gr_text(0, (row+1)*CHAR_HEIGHT-1, t);
  }
}

// Redraw everything on the screen.  Does not flip pages.
// Should only be called with gUpdateMutex locked.
static void draw_screen_locked(void)
{
	draw_image_locked(gCurrentIcon);

	if (show_text) {
		gr_color(0, 0, 0, 160);
		gr_fill(0, 0, gr_fb_width(), gr_fb_height());

		int i = 0;
		if (show_menu) {
			gr_color(64, 96, 255, 255);
			gr_fill(0, (menu_top+menu_sel) * CHAR_HEIGHT,
					gr_fb_width(), (menu_top+menu_sel+1)*CHAR_HEIGHT+1);

			for (; i < menu_top + menu_items; ++i) {
				if (i == menu_top + menu_sel) {
					gr_color(255, 255, 255, 255);
					draw_text_line(i, menu[i]);
					gr_color(64, 96, 255, 255);
				} else if ( i<menu_top ) { 
					gr_color(255, 255, 0, 255);
					draw_text_line(i, menu[i]);
					gr_color(64, 96, 255, 255);
				} else {
					if ( menu_info[i] == 1) {
						gr_color(255, 0, 0, 255);
						draw_text_line(i, menu[i]);
						gr_color(64, 96, 255, 255);
					} else if ( menu_info[i] == 2) {
						gr_color(128, 255, 0, 255);
						draw_text_line(i, menu[i]);
						gr_color(64, 96, 255, 255);
					} else {
						draw_text_line(i, menu[i]);
					}
				}
			}
			gr_fill(0, i*CHAR_HEIGHT+CHAR_HEIGHT/2-1,
					gr_fb_width(), i*CHAR_HEIGHT+CHAR_HEIGHT/2+1);
			++i;
		}

		gr_color(255, 255, 0, 255);

		for (; i < text_rows; ++i) {
			draw_text_line(i, text[(i+text_top) % text_rows]);
		}
	}
}

// Redraw everything on the screen and flip the screen (make it visible).
// Should only be called with gUpdateMutex locked.
static void update_screen_locked(void)
{
    draw_screen_locked();
    gr_flip();
}


static int input_callback(int fd, short revents, void *data)
{
    struct input_event ev;
    int ret;
    int fake_key = 0;

    ret = ev_get_input(fd, revents, &ev);
    if (ret)
        return -1;

    if (ev.type == EV_SYN) {
        return 0;
    } else if (ev.type == EV_REL) {
        if (ev.code == REL_Y) {
            // accumulate the up or down motion reported by
            // the trackball.  When it exceeds a threshold
            // (positive or negative), fake an up/down
            // key event.
            rel_sum += ev.value;
            if (rel_sum > 3) {
                fake_key = 1;
                ev.type = EV_KEY;
                ev.code = KEY_DOWN;
                ev.value = 1;
                rel_sum = 0;
            } else if (rel_sum < -3) {
                fake_key = 1;
                ev.type = EV_KEY;
                ev.code = KEY_UP;
                ev.value = 1;
                rel_sum = 0;
            }
        }
    } else {
        rel_sum = 0;
    }

    if (ev.type != EV_KEY || ev.code > KEY_MAX)
        return 0;

    pthread_mutex_lock(&key_queue_mutex);
    if (!fake_key) {
        // our "fake" keys only report a key-down event (no
        // key-up), so don't record them in the key_pressed
        // table.
        key_pressed[ev.code] = ev.value;
    }
    const int queue_max = sizeof(key_queue) / sizeof(key_queue[0]);
    if (ev.value > 0 && key_queue_len < queue_max) {
        key_queue[key_queue_len++] = ev.code;
        pthread_cond_signal(&key_queue_cond);
    }
    pthread_mutex_unlock(&key_queue_mutex);

    if (ev.value > 0 && device_toggle_display(key_pressed, ev.code)) {
        pthread_mutex_lock(&gUpdateMutex);
        show_text = !show_text;
        if (show_text) show_text_ever = 1;
        update_screen_locked();
        pthread_mutex_unlock(&gUpdateMutex);
    }

    if (ev.value > 0 && device_reboot_now(key_pressed, ev.code)) {
        android_reboot(ANDROID_RB_RESTART, 0, 0);
    }

    return 0;
}

// Reads input events, handles special hot keys, and adds to the key queue.
static void *input_thread(void *cookie)
{
    for (;;) {
        if (!ev_wait(-1))
            ev_dispatch();
    }
    return NULL;
}

void ui_init(void)
{
    gr_init();
    ev_init(input_callback, NULL);

    text_col = text_row = 0;
    text_rows = gr_fb_height() / CHAR_HEIGHT;
    if (text_rows > MAX_ROWS) text_rows = MAX_ROWS;
    text_top = 1;

    text_cols = gr_fb_width() / CHAR_WIDTH;
    if (text_cols > MAX_COLS - 1) text_cols = MAX_COLS - 1;

    int i;
    for (i = 0; BITMAPS[i].name != NULL; ++i) {
        int result = res_create_surface(BITMAPS[i].name, BITMAPS[i].surface);
        if (result < 0) {
            //LOGE("Missing bitmap %s\n(Code %d)\n", BITMAPS[i].name, result);
        }
    }

    pthread_t t;
    pthread_create(&t, NULL, input_thread, NULL);
}

void ui_set_background(int icon)
{
	pthread_mutex_lock(&gUpdateMutex);
	gCurrentIcon = icon;
	update_screen_locked();
	pthread_mutex_unlock(&gUpdateMutex);
}

void ui_print(const char *fmt, ...)
{
	char buf[256];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 256, fmt, ap);
	va_end(ap);

	fputs(buf, stdout);

	// This can get called before ui_init(), so be careful.
	pthread_mutex_lock(&gUpdateMutex);
	if (text_rows > 0 && text_cols > 0) {
		char *ptr;
		for (ptr = buf; *ptr != '\0'; ++ptr) {
			if (*ptr == '\n' || text_col >= text_cols) {
				text[text_row][text_col] = '\0';
				text_col = 0;
				text_row = (text_row + 1) % text_rows;
				if (text_row == text_top) text_top = (text_top + 1) % text_rows;
			}
			if (*ptr != '\n') text[text_row][text_col++] = *ptr;
		}
		text[text_row][text_col] = '\0';
		update_screen_locked();
	}
	pthread_mutex_unlock(&gUpdateMutex);
}

void ui_start_menu(char** headers, char** items, int initial_selection) {
	int i;
	pthread_mutex_lock(&gUpdateMutex);
	if (text_rows > 0 && text_cols > 0) {
		for (i = 0; i < text_rows; ++i) {
			if (headers[i] == NULL) break;
			strncpy(menu[i], headers[i], text_cols-1);
			menu[i][text_cols-1] = '\0';
		}
		menu_top = i;
		for (; i < text_rows; ++i) {
			if (items[i-menu_top] == NULL) break;
			strncpy(menu[i], items[i-menu_top], text_cols-1);
			menu[i][text_cols-1] = '\0';
		}
		menu_items = i - menu_top;
		show_menu = 1;
		menu_sel = initial_selection;
		update_screen_locked();
	}
	pthread_mutex_unlock(&gUpdateMutex);
}

int ui_menu_select(int sel) {
    int old_sel;
    pthread_mutex_lock(&gUpdateMutex);
    if (show_menu > 0) {
        old_sel = menu_sel;
        menu_sel = sel;
        if (menu_sel < 0) menu_sel = menu_items-1;
        if (menu_sel >= menu_items) menu_sel = 0;
        sel = menu_sel;
        if (menu_sel != old_sel) update_screen_locked();
    }
    pthread_mutex_unlock(&gUpdateMutex);
    return sel;
}

void ui_end_menu() {
    int i;
    pthread_mutex_lock(&gUpdateMutex);
    if (show_menu > 0 && text_rows > 0 && text_cols > 0) {
        show_menu = 0;
        update_screen_locked();
    }
    pthread_mutex_unlock(&gUpdateMutex);
}

int ui_text_visible()
{
    pthread_mutex_lock(&gUpdateMutex);
    int visible = show_text;
    pthread_mutex_unlock(&gUpdateMutex);
    return visible;
}

int ui_text_ever_visible()
{
    pthread_mutex_lock(&gUpdateMutex);
    int ever_visible = show_text_ever;
    pthread_mutex_unlock(&gUpdateMutex);
    return ever_visible;
}

void ui_show_text(int visible)
{
    pthread_mutex_lock(&gUpdateMutex);
    show_text = visible;
    if (show_text) show_text_ever = 1;
    update_screen_locked();
    pthread_mutex_unlock(&gUpdateMutex);
}

// Return true if USB is connected.
static int usb_connected() {
    int fd = open("/sys/class/android_usb/android0/state", O_RDONLY);
    if (fd < 0) {
        printf("failed to open /sys/class/android_usb/android0/state: %s\n",
               strerror(errno));
        return 0;
    }

    char buf;
    /* USB is connected if android_usb state is CONNECTED or CONFIGURED */
    int connected = (read(fd, &buf, 1) == 1) && (buf == 'C');
    if (close(fd) < 0) {
        printf("failed to close /sys/class/android_usb/android0/state: %s\n",
               strerror(errno));
    }
    return connected;
}

int ui_wait_key()
{
    pthread_mutex_lock(&key_queue_mutex);

    // Time out after UI_WAIT_KEY_TIMEOUT_SEC, unless a USB cable is
    // plugged in.
    do {
        struct timeval now;
        struct timespec timeout;
        gettimeofday(&now, NULL);
        timeout.tv_sec = now.tv_sec;
        timeout.tv_nsec = now.tv_usec * 1000;
        timeout.tv_sec += UI_WAIT_KEY_TIMEOUT_SEC;

        int rc = 0;
        while (key_queue_len == 0 && rc != ETIMEDOUT) {
            rc = pthread_cond_timedwait(&key_queue_cond, &key_queue_mutex,
                                        &timeout);
        }
    } while (usb_connected() && key_queue_len == 0);

    int key = -1;
    if (key_queue_len > 0) {
        key = key_queue[0];
        memcpy(&key_queue[0], &key_queue[1], sizeof(int) * --key_queue_len);
    }
    pthread_mutex_unlock(&key_queue_mutex);
    return key;
}

int ui_key_pressed(int key)
{
    // This is a volatile static array, don't bother locking
    return key_pressed[key];
}

void ui_clear_key_queue() {
    pthread_mutex_lock(&key_queue_mutex);
    key_queue_len = 0;
    pthread_mutex_unlock(&key_queue_mutex);
}

int ui_set_menu_info(int i, int info)
{
	pthread_mutex_lock(&key_queue_mutex);
	menu_info[ i + menu_top ] = info;
	pthread_mutex_unlock(&key_queue_mutex);
	return 0;
}

int ui_clear_menu_info(void)
{
	pthread_mutex_lock(&key_queue_mutex);
	memset(menu_info, 0, sizeof(menu_info));
	pthread_mutex_unlock(&key_queue_mutex);
	return 0;
}

void ui_draw_test_image(int icon)
{
	pthread_mutex_lock(&gUpdateMutex);
	show_text = 0;
	gCurrentIcon = icon;
	draw_image_locked(gCurrentIcon);
	gr_flip();
	pthread_mutex_unlock(&gUpdateMutex);
}

void ui_do_show_text(void)
{
	pthread_mutex_lock(&key_queue_mutex);
	show_text = 1;
	update_screen_locked();
	pthread_mutex_unlock(&key_queue_mutex);
}

void ui_clear_text_buf(void)
{
	pthread_mutex_lock(&key_queue_mutex);
	memset(text, 0, sizeof(text));
	update_screen_locked();
	pthread_mutex_unlock(&key_queue_mutex);
}
