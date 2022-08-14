/* linux/arch/arm/mach-tcc88xx/include/mach/tcc_fb.h
 *
 * Copyright (C) 2010 Telechips, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
*/

#ifndef __TCC_FB_H
#define __TCC_FB_H

#define ATAG_TCC_PANEL	0x54434364 /* TCCd */

enum {
	PANEL_ID_LMS350DF01,
	PANEL_ID_LMS480KF01,
	PANEL_ID_DX08D11VM0AAA,
	PANEL_ID_LB070WV6,
	PANEL_ID_CLAA104XA01CW,
	PANEL_ID_HT121WX2,
	PANEL_ID_TD043MGEB1,
	PANEL_ID_AT070TN93,
	PANEL_ID_TD070RDH11,
	PANEL_ID_N101L6,
	PANEL_ID_TW8816,
	PANEL_ID_CLAA102NA0DCW,
	PANEL_ID_ED090NA,	
	PANEL_ID_KR080PA2S,	
	PANEL_ID_CLAA070NP01,
	PANEL_ID_HV070WSA,
	PANEL_ID_FLD0800,
	PANEL_ID_CLAA070WP03,
	PANEL_ID_LMS700KF23,
	PANEL_ID_EJ070NA,
	PANEL_ID_LP101WX1,
	PANEL_ID_HDMI	
};

struct lcd_panel;

typedef enum{
	TCC_PWR_INIT,
	TCC_PWR_ON,		
	TCC_PWR_OFF
}tcc_db_power_s;

struct tcc_db_platform_data{
	/* destroy output.  db clocks are not on at this point */
	int (*set_power)(struct lcd_panel *db, tcc_db_power_s pwr);
};
extern struct tcc_db_platform_data *get_tcc_db_platform_data(void);

// tcc display output block operation 
struct tcc_db_out_ops {
	/* initialize output.  db clocks are not on at this point */
	int (*init)(struct lcd_panel *db);
	/* destroy output.  db clocks are not on at this point */
	void (*destroy)(struct lcd_panel *db);
	/* detect connected display.  can sleep.*/
	unsigned (*detect)(struct lcd_panel *db);
	/* enable output.  db clocks are on at this point */
	void (*enable)(struct lcd_panel *db);
	/* disable output.  db clocks are on at this point */
	void (*disable)(struct lcd_panel *db);
	/* suspend output.  db clocks are on at this point */
	void (*suspend)(struct lcd_panel *db);
	/* resume output.  db clocks are on at this point */
	void (*resume)(struct lcd_panel *db);
};


enum{
	TCC_DB_OUT_RGB,
	TCC_DB_OUT_LVDS,
	TCC_DB_OUT_DSI,
	TCC_DB_OUT_HDMI,
};

struct lcd_platform_data {
	unsigned power_on;
	unsigned display_on;
	unsigned bl_on;
	unsigned reset;
	unsigned iod_en;
	unsigned lcdc_num;
};

struct lcd_panel {

	struct device *dev;

	int						db_type;
	void						*db_out_data;
	struct tcc_db_out_ops		*db_out_ops;

	const char *name;
	const char *manufacturer;

	int id;			/* panel ID */
	int xres;		/* x resolution in pixels */
	int yres;		/* y resolution in pixels */
	int width;		/* display width in mm */
	int height;		/* display height in mm */
	int bpp;		/* bits per pixels */

	int clk_freq;
	int clk_div;
	int bus_width;
	int lpw;		/* line pulse width */
	int lpc;		/* line pulse count */
	int lswc;		/* line start wait clock */
	int lewc;		/* line end wait clock */
	int vdb;		/* back porch vsync delay */
	int vdf;		/* front porch vsync delay */
	int fpw1;		/* frame pulse width 1 */
	int flc1;		/* frame line count 1 */
	int fswc1;		/* frame start wait cycle 1 */
	int fewc1;		/* frame end wait cycle 1 */
	int fpw2;		/* frame pulse width 2 */
	int flc2;		/* frame line count 2 */
	int fswc2;		/* frame start wait cycle 2 */
	int fewc2;		/* frame end wait cycle 2 */
	int sync_invert;

	int (*init)(struct lcd_panel *panel);
	int (*set_power)(struct lcd_panel *panel, int on, unsigned int lcd_num);
	int (*set_backlight_level)(struct lcd_panel *panel, int level);

	int state; //current state 0 off , 0: on
	int bl_level; //current backlight level
};

int tccfb_register_panel(struct lcd_panel *panel);
struct lcd_panel *tccfb_get_panel(void);

static inline void tcc_db_set_outdata(struct lcd_panel *db, void *data)
{
	db->db_out_data = data;
}

static inline void *tcc_db_get_outdata(struct lcd_panel *db)
{
	return db->db_out_data;
}

extern void tcc_db_set_output(struct lcd_panel *db);

#ifdef CONFIG_TCC_MIPI
extern struct tcc_db_out_ops tcc_db_dsi_ops;
#endif//

#endif //__TCC_FB_H
