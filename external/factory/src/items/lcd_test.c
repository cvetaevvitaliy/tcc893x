#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include "minui.h"
#include "factory_ui.h"
#include "ui.h"
#include "lcd_test.h"


#define BACKLIGHT_PATH "/sys/class/leds/lcd-backlight/brightness"

int backlight_levels[] = { 1, 32, 64, 128, 255 };
char* LCD_TEST_HEADERS[] = {
	"BACKLIGHT LEVEL TEST",
	NULL
};

char* LCD_TEST_MENUS[] = {
	"Adjust Backlight Level",
	"Image View",
	"Test Pass",
	"Test Fail",
	"Back",
	NULL
};

int adjust_brightness(int level)
{
	char buf[20];
	int ret, fd;
	fd = open(BACKLIGHT_PATH, O_RDWR);
	if( fd<0 ) {
		return -1;
	}
	ret = sprintf(buf, "%d\n", level);
	write( fd, buf, ret);

	close(fd);

	return 0;
}

int show_test_image(void)
{
	int i = 1;
	ui_clear_key_queue();
	while( 1 ) {
		if( i<=IMAGE_NONE || i>=NUM_IMAGE_ICONS )
			return 0;
		ui_draw_test_image(i);

		int key = ui_wait_key();
		int action = device_handle_key(key, 1);

		if( action < 0 ) {
			switch(action) {
				case HIGHLIGHT_UP:
					i++;
					break;
				case HIGHLIGHT_DOWN:
					i--;
					break;
			}
		}
	}

	return 0;
}

int lcd_test_command(struct ft_struct *t, int prev_selected)
{
	int initial_selected = 0;
	int level = 0;
	int level_max = (sizeof(backlight_levels)/sizeof(int));
	for (;;) {
		int chosen_item = get_menu_selection(LCD_TEST_HEADERS, LCD_TEST_MENUS, 0, 
				initial_selected);

		chosen_item = device_perform_action(chosen_item);

		switch (chosen_item) 
		{
			initial_selected = chosen_item;
			case ID_TEST_LCD_LEVEL:
			if( level >= level_max )
				level = 0;
			adjust_brightness( backlight_levels[level++] );
			break;
			case ID_TEST_LCD_IMAGE:
			show_test_image();
			ui_draw_test_image(0);
			ui_do_show_text();
			break;
			case ID_TEST_LCD_PASS:
			set_ft_status(ID_BACKLIGHT_LEVEL, STATUS_PASS);
			break;
			case ID_TEST_LCD_FAIL:
			set_ft_status(ID_BACKLIGHT_LEVEL, STATUS_FAIL);
			break;
			case ID_TEST_LCD_BACK:
			ui_draw_test_image(0);
			return prev_selected;
		}
	}

	return 0;
}

struct ft_struct lcd_service = {
	.status = 0,
	.do_command = lcd_test_command,
};

int init_lcd_test(void)
{
	add_ft_service(&lcd_service, ID_BACKLIGHT_LEVEL);

	return 0;
}
