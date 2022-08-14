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

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>

#include "ui.h"
#include "minui/minui.h"
#include "roots.h"
#include "factory_ui.h"
#include "item_reference.h"
#include "lcd_test.h"

#include <cutils/properties.h>
#include <cutils/android_reboot.h>
#include <sys/reboot.h>

struct ft_struct *ft_head[ID_ITEM_TEST_NUM] = {0};

int full_test(void)
{
	int i;
	for(i=0; i<ID_ITEM_TEST_NUM; i++)
	{
		if( ft_head[i] != NULL ) {
			ft_head[i]->do_command( ft_head[i], i);
			ui_do_show_text();
		}
	}

	return 0;
}

int item_test(int prev_selected)
{
	int initial_selected = 0;
	for (;;) {
		int chosen_item = get_menu_selection(ITEM_TEST_HEADERS, ITEM_TEST_MENUS, 0, 
				initial_selected);

		chosen_item = device_perform_action(chosen_item);

		int status;
		int wipe_cache;
		switch (chosen_item) 
		{
			case ID_EXIT_ITEM:
				return prev_selected;
			default:
				initial_selected = ft_head[chosen_item]->do_command(ft_head[chosen_item], 
						chosen_item);
				ui_do_show_text();
		}
	}
	return 0;
}

char** get_test_report_menu(char **item_test_menus)
{
	int i;
	char** new_menus = (char**)malloc(sizeof(char*)*20);

	for(i=0; i<ID_ITEM_TEST_NUM+1; i++)
	{
		new_menus[i] = (char*)malloc(sizeof(char)*32);
	}
	for(i=0; i<ID_ITEM_TEST_NUM; i++)
	{
		char *buf = (ft_head[i] == NULL) ? 
			item_status_string[0] : item_status_string[ft_head[i]->status];
		sprintf(new_menus[i], "[%s] %s",
				buf, item_test_menus[i]);
		if( ft_head[i] != NULL )
			ui_set_menu_info(i, ft_head[i]->status);
	}
	strcpy( new_menus[ID_ITEM_TEST_NUM-1], 
			item_test_menus[ID_ITEM_TEST_NUM-1] );
	new_menus[ID_ITEM_TEST_NUM] = NULL;

	return new_menus;
}

int test_report(int prev_selected)
{
	int initial_selected = 0;
	char **new_menus = get_test_report_menu(ITEM_TEST_MENUS);

	for (;;) {
		int chosen_item = get_menu_selection(TEST_REPORT_HEADERS, new_menus, 0, 
				initial_selected);

		chosen_item = device_perform_action(chosen_item);
		initial_selected = chosen_item;
		switch (chosen_item) 
		{
			case ID_EXIT_ITEM:
				return prev_selected;
			default:
				break;
		}
	}
	return 0;
}

int clear_flash(int prev_selected)
{
	int initial_selected = 0;
	for (;;) {

		int chosen_item = get_menu_selection(CLEAR_FLASH_HEADERS,
				CLEAR_FLASH_MENUS, 0, initial_selected);

		chosen_item = device_perform_action(chosen_item);
		initial_selected = chosen_item;
		switch (chosen_item) 
		{
			case ID_RUN_CLEAR_FLASH:
				ui_print("\n-- Wiping data...\n");
				erase_volume("/data");
				erase_volume("/cache");
				ui_print("Data wipe complete.\n");
				break;
			case ID_EXIT_CLEAR_FLASH:
				ui_clear_text_buf();
				return prev_selected;
		}
	}

	return 0;
}

int print_version(int prev_selected)
{
	int initial_selected = 0;
	int fd = open("/proc/version", O_RDONLY);
	char buf[6][128];
	//char *model, *device, *date, *sdk, *android_ver;

	property_get("ro.product.model", buf[0], "Unknown");
	property_get("ro.product.device", buf[1], "Unknown");
	property_get("ro.build.date", buf[2], "Unknown");
	property_get("ro.build.version.sdk", buf[3], "Unknown");
	property_get("ro.build.version.release", buf[4], "Unknown");
	read(fd, buf[5], sizeof(buf[5]));
	close(fd);

	for (;;) {

		int chosen_item = get_menu_selection(PRINT_VERSION_HEADERS,
				PRINT_VERSION_MENUS, 0, initial_selected);

		chosen_item = device_perform_action(chosen_item);
		initial_selected = chosen_item;
		switch (chosen_item) 
		{
			case ID_SHOW_VERSION:
				ui_print("Device Model : %s\n", buf[0]);
				ui_print("Device Name : %s\n", buf[1]);
				ui_print("Build Date : %s\n", buf[2]);
				ui_print("SDK Version : %s\n", buf[3]);
				ui_print("Android Version : %s\n", buf[4]);
				ui_print("Kernel Version : %s\n", buf[5]);
				break;
			case ID_EXIT_PRINT:
				return prev_selected;
		}
	}
	return 0;
}

static int
prompt_and_wait() {
	int initial_selected = 0;
	for (;;) {
		int chosen_item = get_menu_selection(ROOT_HEADERS, ROOT_MENUS, 0, initial_selected);
		chosen_item = device_perform_action(chosen_item);

		switch (chosen_item) 
		{
			case ID_FULL_TEST:
				initial_selected = full_test();
				break;
			case ID_ITEM_TEST:
				initial_selected = item_test(chosen_item);
				break;
			case ID_TEST_REPORT:
				initial_selected = test_report(chosen_item);
				ui_clear_menu_info();
				break;
			case ID_CLEAR_FLASH:
				initial_selected = clear_flash(chosen_item);
				break;
			case ID_PRINT_VERSION:
				initial_selected = print_version(chosen_item);
				ui_clear_text_buf();
				break;
			case ID_REBOOT:
				return 0;
		}
	}
	return 0;
}

int dev_init(void)
{
	init_item_reference();
	init_lcd_test();
	return 0;
}

int main(int argc, char **argv) 
{
	ui_init();
	load_volume_table();
	dev_init();
	ui_do_show_text();
	prompt_and_wait();

	ui_print("Rebooting...\n");
	android_reboot(ANDROID_RB_RESTART, 0, 0);
	return EXIT_SUCCESS;
}
