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
enum {
	IMAGE_NONE,
	TEST_IMAGE_0,
	TEST_IMAGE_1,
	TEST_IMAGE_2,
	TEST_IMAGE_3,
	TEST_IMAGE_4,
	TEST_IMAGE_5,
	TEST_IMAGE_6,
  NUM_IMAGE_ICONS
};

static double now();
static void draw_image_locked(int icon);
static void draw_text_line(int row, const char* t);
static void draw_screen_locked(void);
static void update_screen_locked(void);
static int input_callback(int fd, short revents, void *data);
static void *input_thread(void *cookie);
void ui_init(void);
void ui_set_background(int icon);
void ui_print(const char *fmt, ...);
void ui_start_menu(char** headers, char** items, int initial_selection);
int ui_menu_select(int sel); 
void ui_end_menu(); 
int ui_text_visible();
int ui_text_ever_visible();
void ui_show_text(int visible);
static int usb_connected(); 
int ui_wait_key();
int ui_key_pressed(int key);
void ui_clear_key_queue();
int ui_set_menu_info(int i, int info);
int ui_clear_menu_info(void);
void ui_draw_test_image(int icon);
void ui_do_show_text(void);
void ui_clear_text_buf(void);
