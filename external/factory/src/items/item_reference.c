#include <stdio.h>
#include "minui.h"
#include "factory_ui.h"
#include "ui.h"
#include "item_reference.h"


char* ITEM_REFERENCE_HEADERS[] = {
	"ITEM REFERENCE",
	NULL
};

char* ITEM_REFERENCE_MENUS[] = {
	"Test Run",
	"Test Pass",
	"Test Fail",
	"Back",
	NULL
};

int print_result(void)
{
	ui_print("Success\n");
	return 0;
}

int do_item_reference_command(struct ft_struct *t, int prev_selected)
{
	int initial_selected = 0;
	for (;;) {
		int chosen_item = get_menu_selection(ITEM_REFERENCE_HEADERS, 
				ITEM_REFERENCE_MENUS, 0, 
				initial_selected);

		chosen_item = device_perform_action(chosen_item);

		switch (chosen_item) 
		{
			initial_selected = chosen_item;
			case ID_ITEM_REFERENCE_RUN:
				print_result();
				break;
			case ID_ITEM_REFERENCE_PASS:
				set_ft_status(ID_ITEM_REFERENCE, STATUS_PASS);
				break;
			case ID_ITEM_REFERENCE_FAIL:
				set_ft_status(ID_ITEM_REFERENCE, STATUS_FAIL);
				break;
			case ID_ITEM_REFERENCE_BACK:
				ui_clear_text_buf();
				return prev_selected;

		}
	}

	return 0;
}

struct ft_struct item_reference = {
	.status = STATUS_DEFAULT,
	.do_command = do_item_reference_command,
};

int init_item_reference(void)
{
	add_ft_service(&item_reference, ID_ITEM_REFERENCE);

	return 0;
}
