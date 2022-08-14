extern char* LCD_TEST_HEADERS[];
extern char* LCD_TEST_MENUS[];

enum ID_LCD_TEST {
	ID_TEST_LCD_LEVEL	,
	ID_TEST_LCD_IMAGE	,
	ID_TEST_LCD_PASS	,
	ID_TEST_LCD_FAIL	,
	ID_TEST_LCD_BACK	,
	ID_TEST_LCD_NUM
};

int init_lcd_test(void);
