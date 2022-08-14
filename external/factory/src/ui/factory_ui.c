/*
 * Copyright (C) 2011 The Android Open Source Project
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

#include <linux/input.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include "factory_ui.h"
#include "ui.h"

extern struct ft_struct *ft_head[ID_ITEM_TEST_NUM];
char* ROOT_HEADERS[] = { 
	"FACTORY MODE",
	NULL	
};
char* ROOT_MENUS[] = { 
	"Full Test"		,
	"Item Test"		,
	"Test Report"	,
	"Clear Flash"	,
	"Version"			,
	"Reboot"			,
	NULL	
};

char* ITEM_TEST_HEADERS[] = { 
	"ITEM TEST"	,
	NULL	
};
char* ITEM_TEST_MENUS[] = { 
	"Keys"							,
	"OFN"								,
	"Touch Panel"				,
	"Backlight Level"		,
	"Nand Flash"				,
	"Memory Card"				,
	"Vibrator"					,
	"LED"								,	 
	"RTC"								,
	"Loopback"					,
	"Ringtone"					,
	"Receiver"					,
	"Headset"						,
	"Main Camera"				,
	"Sub Camera"				,
	"Battery & Charger"	,
	"Idle Current"			,
	"Signal Quality"		,
	"Item Reference"		,
	"Back"							,
	NULL	
};

char* TEST_REPORT_HEADERS[] = { 
	"TEST REPORT"	,
	NULL	
};


char* PRINT_VERSION_HEADERS[] = { 
	"VERSION INFORMATION"	,
	NULL	
};
char* PRINT_VERSION_MENUS[] = {
	"Print Version" ,
	"Back" 					,
	NULL
};

char* CLEAR_FLASH_HEADERS[] = { 
	"CLEAR FLASH"	,
	NULL	
};
char* CLEAR_FLASH_MENUS[] = {
	"Clear Flash" ,
	"Back" 				,
	NULL
};

char *item_status_string[] = {
	" "			,
	"FAIL"	,
	"PASS"	,
	NULL
};



int device_toggle_display(volatile char* key_pressed, int key_code) 
{
	return key_pressed[KEY_MENU];
}

int device_reboot_now(volatile char* key_pressed, int key_code) 
{
	static int presses = 0;
	if (key_code == KEY_POWER) { 
		++presses;
		return presses == 100;
	} else {
		presses = 0;
		return 0;
	}
}

int device_handle_key(int key_code, int visible) 
{
	if (visible) {
		switch (key_code) {
			case KEY_DOWN:
			case KEY_VOLUMEDOWN:
				return HIGHLIGHT_DOWN;

			case KEY_UP:
			case KEY_VOLUMEUP:
				return HIGHLIGHT_UP;

			case KEY_ENTER:
			case KEY_POWER:
				return SELECT_ITEM;
		}
	}

	return NO_ACTION;
}

int device_perform_action(int which) 
{
	return which;
}

int erase_volume(const char *volume) 
{
	ui_print("Formatting %s...\n", volume);

	ensure_path_unmounted(volume);

	return format_volume(volume);
}

int get_menu_selection(char** headers, char** items, int menu_only,
		int initial_selection) 
{
	ui_clear_key_queue();

	ui_start_menu(headers, items, initial_selection);
	int selected = initial_selection;
	int chosen_item = -1;

	while (chosen_item < 0) {
		int key = ui_wait_key();
		int visible = ui_text_visible();

		if (key == -1) { 
			if (ui_text_ever_visible()) {
				continue;
			} else {
				ui_end_menu();
				return ID_REBOOT;
			}
		}

		int action = device_handle_key(key, visible);

		if (action < 0) {
			switch (action) {
				case HIGHLIGHT_UP:
					--selected;
					selected = ui_menu_select(selected);
					break;
				case HIGHLIGHT_DOWN:
					++selected;
					selected = ui_menu_select(selected);
					break;
				case SELECT_ITEM:
					chosen_item = selected;
					break;
				case NO_ACTION:
					break;
			}
		} else if (!menu_only) {
			chosen_item = action;
		}
	}

	ui_end_menu();
	return chosen_item;
}

int add_ft_service(struct ft_struct *ft_item, enum ITEM_TEST_ID id)
{
	ft_head[id] = ft_item;
	return 0;
}

int set_ft_status(enum ITEM_TEST_ID id, enum ITEM_STATUS status)
{
	ft_head[id]->status = status;
	return 0;
}
