#define NO_ACTION           -1
#define HIGHLIGHT_UP        -2
#define HIGHLIGHT_DOWN      -3
#define SELECT_ITEM         -4

extern char* ROOT_HEADERS[];
extern char* ROOT_MENUS[]; 
extern char* ITEM_TEST_HEADERS[]; 
extern char* ITEM_TEST_MENUS[]; 
extern char* TEST_REPORT_HEADERS[];
extern char* PRINT_VERSION_HEADERS[];
extern char* PRINT_VERSION_MENUS[];
extern char* CLEAR_FLASH_HEADERS[];
extern char* CLEAR_FLASH_MENUS[];
extern char *item_status_string[];

enum ROOT_MENU_ID { 
	ID_FULL_TEST			, 
	ID_ITEM_TEST			, 
	ID_TEST_REPORT		,
	ID_CLEAR_FLASH		, 
	ID_PRINT_VERSION	,
	ID_REBOOT	
};
enum ITEM_TEST_ID {
	ID_KEYS								,
	ID_OFN								,
	ID_TOUCH_PANEL				,
	ID_BACKLIGHT_LEVEL 		,
	ID_NAND_FLASH					,
	ID_MEMORY_CARD				,
	ID_VIBRATOR						,
	ID_LED								,
	ID_RTC								,
	ID_LOOPBACK						,
	ID_RINGTONE						,
	ID_RECEIVER						,
	ID_HEADSET						,
	ID_MAIN_CAMERA				,
	ID_SUB_CAMERA					,
	ID_BATTERY_N_CHARGER	,
	ID_IDLE_CURRENT				,
	ID_SIGNAL_QUALITY			,
	ID_ITEM_REFERENCE			,				// by B120040 ksjung@telechips.com
	ID_EXIT_ITEM					,
	ID_ITEM_TEST_NUM
};
enum PRINT_VERSION_ID {
	ID_SHOW_VERSION				,
	ID_EXIT_PRINT
};
enum CLEAR_FALSH_ID {
	ID_RUN_CLEAR_FLASH				,
	ID_EXIT_CLEAR_FLASH
};

enum ITEM_STATUS {
	STATUS_DEFAULT				,
	STATUS_FAIL						,
	STATUS_PASS
};

struct ft_struct {
	char *name;
	int status; //0:init, 1:test fail, 2:test pass
	int (*do_command)(struct ft_struct *t, int prev_selected);
};

int get_menu_selection(char** headers, char** items, int menu_only, int initial_selection);
int erase_volume(const char *volume);
int device_toggle_display(volatile char* key_pressed, int key_code);
int device_reboot_now(volatile char* key_pressed, int key_code);
int device_handle_key(int key, int visible);
int device_perform_action(int which);
int set_ft_status(enum ITEM_TEST_ID id, enum ITEM_STATUS status);
int add_ft_service(struct ft_struct *ft_item, enum ITEM_TEST_ID id);
