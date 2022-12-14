/****************************************************************************
 *	 TCC Version 0.6
 *	 Copyright (c) telechips, Inc.
 *	 ALL RIGHTS RESERVED
 *
****************************************************************************/

#ifndef _TCC_FB_IOCTL_H
#define _TCC_FB_IOCTL_H

#include <linux/types.h>

#define PRESENTATION_LIMIT_RESOLUTION (1024*600)

#define TCCFB_IOCTL_MAGIC	'f'

#define TCC_LCD_FB_IOCTL		0x46FF

typedef enum{
	TCC_HDMI_SUSEPNED,
	TCC_HDMI_RESUME,	
	TCC_HDMI_VIDEO_START,
	TCC_HDMI_VIDEO_END,
}TCC_HDMI_M;


#define TCC_LCDC_HDMI_START				0x0010
#define TCC_LCDC_HDMI_TIMING			0x0011
#define TCC_LCDC_HDMI_DISPLAY			0x0012
#define TCC_LCDC_HDMI_END				0x0013
#define TCC_LCDC_HDMI_CHECK				0x0014
#define TCC_LCDC_HDMI_MODE_SET			0x0015
#define TCC_LCDC_HDMI_SET_SIZE			0x0016
#define TCC_LCDC_HDMI_GET_SIZE			0x0017

#define TCC_LCDC_TVOUT_MODE_SET			0x0018
#define TCC_LCDC_HDMI_STATUS_SET		0x0019

#define TCC_LCDC_HDMI_UI_UPDATE			0x001C

#define TCC_LCDC_COMPOSITE_CHECK		0x0020
#define TCC_LCDC_COMPOSITE_MODE_SET		0x0021

#define TCC_LCDC_COMPONENT_CHECK		0x0030
#define TCC_LCDC_COMPONENT_MODE_SET		0x0031

#define TCC_LCDC_OUTPUT_MODE_CHECK		0x0040
#define TCC_LCDC_OUTPUT_MODE_SET		0x0041

#define TCC_LCDC_SET_OUTPUT_RESIZE_MODE	0x0043

#define TCC_LCDC_SET_WMIXER_OVP			0x0045

#define TCC_LCDC_SET_ENABLE				0x0050
#define TCC_LCDC_SET_DISABLE			0x0051
#define TCC_LCDC_LAYER_ENABLE			0x0052
#define TCC_LCDC_LAYER_DISABLE			0x0053
#define TCC_LCDC_SET_BGCOLOR			0x0054
#define TCC_LCDC_SET_ALPHA				0x0055
#define TCC_LCDC_GET_IMAGE_INFO			0x0056
#define TCC_LCDC_SET_CHROMAKEY			0x0057
#define TCC_LCDC_SET_CHROMA				0x0058
#define TCC_LCDC_SET_COLORENHANCE		0x0059
#define TCC_LCDC_SET_GAMMA				0x005A

#define TCC_LCDC_3D_UI_ENABLE			0x005B
#define TCC_LCDC_3D_UI_DISABLE			0x005C

#define TCC_LCDC_GET_NUM				0x005D
#define TCC_LCDC_GET_ACT_DISP_NUM	0x005E
#define TCC_LCDC_SET_LAYER_ORDER		0x005F

#if defined(CONFIG_TCC_VIDEO_DISPLAY_BY_VSYNC_INT) || defined(TCC_VIDEO_DISPLAY_BY_VSYNC_INT)
#define	TCC_LCDC_VIDEO_START_VSYNC		0x0060
#define	TCC_LCDC_VIDEO_END_VSYNC		0x0061
#define	TCC_LCDC_VIDEO_PUSH_VSYNC		0x0062
#define	TCC_LCDC_VIDEO_GET_DISPLAYED	0x0063
#define	TCC_LCDC_VIDEO_CLEAR_FRAME		0x0064
#define	TCC_LCDC_VIDEO_SKIP_FRAME_START	0x0067
#define	TCC_LCDC_VIDEO_SKIP_FRAME_END	0x0068
#define	TCC_LCDC_VIDEO_DEINTERLACE_MODE	0x0069
#define	TCC_LCDC_VIDEO_SET_SIZE_CHANGE	0x006A
#define	TCC_LCDC_VIDEO_SET_FRAMERATE	0x006B
#define	TCC_LCDC_VIDEO_SKIP_ONE_FRAME	0x006C
#define TCC_LCDC_VIDEO_DEINTERLACE_SET	0x006D		// ronald.lee
#define	TCC_LCDC_VIDEO_GET_VALID_COUNT	0x006E
#endif
#define	TCC_LCDC_VIDEO_SET_MVC_STATUS	0x006F

#define	TCC_LCDC_EXCLUSIVE_UI_SET_PARAM			0x0070
#define	TCC_LCDC_EXCLUSIVE_UI_GET_PARAM			0x0071
#define	TCC_LCDC_EXCLUSIVE_UI_SET_SIZE			0x0072
#define	TCC_LCDC_EXCLUSIVE_UI_SET_INTERLACE		0x0073
#define	TCC_LCDC_EXCLUSIVE_UI_GET_ADDR			0x0074
#define	TCC_LCDC_EXCLUSIVE_UI_ENABLE			0x0075
#define	TCC_LCDC_EXCLUSIVE_UI_DISABLE			0x0076
#define	TCC_LCDC_EXCLUSIVE_UI_UPDATE			0x0077
#define	TCC_LCDC_EXCLUSIVE_UI_UPDATE_FORCE		0x0078
#define	TCC_LCDC_EXCLUSIVE_UI_CLEAR				0x0079
#define	TCC_LCDC_EXCLUSIVE_UI_PROCESS_HDMI		0x007A
#define	TCC_LCDC_EXCLUSIVE_UI_SET_AR			0x007B
#define	TCC_LCDC_EXCLUSIVE_UI_SET_ARATIO		0x007C
#define	TCC_LCDC_EXCLUSIVE_UI_DISPLAY_ON		0x007D
#define	TCC_LCDC_EXCLUSIVE_UI_DISPLAY_OFF		0x007E
#define	TCC_LCDC_EXCLUSIVE_UI_3D_ENABLE			0x007F
#define	TCC_LCDC_EXCLUSIVE_UI_3D_DISABLE		0x0080

#define	TCC_LCD_FBIOPUT_VSCREENINFO	0x008F
#define TCC_LCDC_UI_UPDATE_CTRL		0x0090
#define TCC_LCDC_HDMI_LASTFRAME		0x0091

#define TCC_LCDC_REFER_VSYNC_ENABLE 	0x0092
#define TCC_LCDC_REFER_VSYNC_DISABLE 	0x0093
#define TCC_LCDC_GET_LAST_FRAME_STATUS  0x0094
#define TCC_HDMI_FBIOPUT_VSCREENINFO	0x0095

#define TCC_LCDC_DISPLAY_START		0x0096
#define TCC_LCDC_DISPLAY_UPDATE		0x0097
#define TCC_LCDC_DISPLAY_END		0x0098
#define TCC_LCDC_VIDEO_DISPLAYED_IDX	0x0099
#define TCC_LCDC_VIDEO_SWAP_VPU_FRAME	0x009A

#define TCC_LCD_BL_SET					0x0100
#define TCC_LCDC_SET_LUT_BRIGHT         0x0101
#define TCC_LCDC_CTRL_LAST_FRAME		0x0102
#define TCC_LCDC_SET_LUT_DVI			0x0103

#define TCC_LCDC_MOUSE_SHOW				0x0200
#define TCC_LCDC_MOUSE_MOVE				0x0201
#define TCC_LCDC_MOUSE_ICON				0x0202
#define TCC_LCDC_FBCHANNEL_ONOFF				0x0203

#define TCC_LCDC_ATTACH_SET_STATE		0x0300
#define TCC_LCDC_ATTACH_GET_STATE		0x0301

#define TCC_LCDC_SET_DISPLAY_TYPE		0x0400
#define TCC_LCDC_GET_DISPLAY_TYPE		0x0401

#define TCC_LCD_HDMI_PORT_CHANGE_ON	0x0501
#define TCC_LCD_HDMI_PORT_CHANGE_GET_STATE 0x0502

#define TCC_FB_FLUSH					0x1000

#define TCC_GPU_CLOCK					0x2000

#define TCCFB_BLIT	_IOW(TCCFB_IOCTL_MAGIC, 0x1100, unsigned int)

typedef enum{
	LCDC_TVOUT_NOE_MODE,
	LCDC_TVOUT_UI_MODE,
	LCDC_TVOUT_VIDEO_ONLY_MODE,
	LCDC_TVOUT_OVERALY_MODE,
	LCDC_TVOUT_MODE_MAX
}LCDC_TVOUT_MODE;

typedef enum{
	LCDC_COMPOSITE_NONE_MODE,
	LCDC_COMPOSITE_UI_MODE,
	LCDC_COMPOSITE_VIDEO_ONLY_MODE,
	LCDC_COMPOSITE_OVERALY_MODE,
	LCDC_COMPOSITE_MODE_MAX
}LCDC_COMPOSITE_MODE;

typedef enum{
	LCDC_COMPONENT_NONE_MODE,
	LCDC_COMPONENT_UI_MODE,
	LCDC_COMPONENT_VIDEO_ONLY_MODE,
	LCDC_COMPONENT_OVERALY_MODE,
	LCDC_COMPONENT_MODE_MAX
}LCDC_COMPONENT_MODE;

typedef enum
{
	OUTPUT_NONE,
	OUTPUT_HDMI,
	OUTPUT_COMPOSITE,
	OUTPUT_COMPONENT,
	OUTPUT_MAX
}OUTPUT_SELECT;

typedef enum
{
	OUTPUT_COMPOSITE_NTSC,
	OUTPUT_COMPOSITE_PAL,
	OUTPUT_COMPOSITE_MAX
}OUTPUT_COMPOSITE_MODE;

typedef enum
{
	OUTPUT_COMPONENT_720P,
	OUTPUT_COMPONENT_1080I,
	OUTPUT_COMPONENT_MAX
}OUTPUT_COMPONENT_MODE;

typedef enum
{
	OUTPUT_SELECT_NONE,
	OUTPUT_SELECT_HDMI_ENABLE,
	OUTPUT_SELECT_HDMI_TIMING_SET,
	OUTPUT_SELECT_COMPONENT_ENABLE,
	OUTPUT_SELECT_COMPOSITE_ENABLE,
	OUTPUT_SELECT_MAX
}OUTPUT_SELECT_MODE;

struct display_platform_data 
{
	uint32_t resolution;
	uint32_t output;
	uint32_t hdmi_resolution;
	uint32_t composite_resolution;
	uint32_t component_resolution;
	uint32_t hdmi_mode;
}; 

#define TCCFB_ROT_90	0x4
#define TCCFB_ROT_180	0x8
#define TCCFB_ROT_270	0x10

typedef struct 
{
	int width;
	int height;
	int frame_hz;
}tcc_display_size;

typedef struct
{
	int resize_up;
	int resize_down;
	int resize_left;
	int resize_right;
}tcc_display_resize;

typedef struct 
{
	int x;
	int y;
}tcc_mouse;

typedef struct
{
    unsigned int width;
    unsigned int height;
    unsigned int option;
    unsigned char *buf;
}tcc_mouse_icon;

struct tcc_rect {
	uint32_t x;
	uint32_t y;
	uint32_t w;
	uint32_t h;
};

struct tcc_img {
	uint32_t width;
	uint32_t height;
	uint32_t format;
	uint32_t offset;
	int fd;
	void *addr;
};

struct tcc_blit_req {
	struct tcc_img src;
	struct tcc_img dst;
	struct tcc_rect src_rect;
	struct tcc_rect dst_rect;
	uint32_t alpha;
    uint32_t transform;
};

struct tcc_blit_req_list {
	uint32_t count;
	struct tcc_blit_req req[];
};


typedef enum{
	LCDC_LAYER_0, 
	LCDC_LAYER_1, 
	LCDC_LAYER_2, 
	LCDC_LAYER_3, 
	LCDC_LAYER_MAX
}LCD_IMG_LAYER_TYPE;


typedef enum{
	TCC_LCDC_IMG_FMT_1BPP,
	TCC_LCDC_IMG_FMT_2BPP,
	TCC_LCDC_IMG_FMT_4BPP,
	TCC_LCDC_IMG_FMT_8BPP,
	TCC_LCDC_IMG_FMT_RGB332 = 8,
	TCC_LCDC_IMG_FMT_RGB444 = 9,
	TCC_LCDC_IMG_FMT_RGB565 = 10,
	TCC_LCDC_IMG_FMT_RGB555 = 11,
	TCC_LCDC_IMG_FMT_RGB888 = 12,
	TCC_LCDC_IMG_FMT_RGB666 = 13,
	TCC_LCDC_IMG_FMT_RGB888_3	= 14,		/* RGB888 - 3bytes aligned - B1[31:24],R0[23:16],G0[15:8],B0[7:0] newly supported : 3 bytes format*/
	TCC_LCDC_IMG_FMT_ARGB6666_3 = 15,		/* ARGB6666 - 3bytes aligned - A[23:18],R[17:12],G[11:6],B[5:0]newly supported : 3 bytes format */
	TCC_LCDC_IMG_FMT_COMP = 16,				// 4bytes
	TCC_LCDC_IMG_FMT_DECOMP	= (TCC_LCDC_IMG_FMT_COMP),
	TCC_LCDC_IMG_FMT_444SEP = 21,
	TCC_LCDC_IMG_FMT_UYVY = 22,		/* 2bytes	: YCbCr 4:2:2 Sequential format LSB [Y/U/Y/V] MSB : newly supported : 2 bytes format*/
	TCC_LCDC_IMG_FMT_VYUY = 23,		/* 2bytes	: YCbCr 4:2:2 Sequential format LSB [Y/V/Y/U] MSB : newly supported : 2 bytes format*/

	TCC_LCDC_IMG_FMT_YUV420SP = 24,	
	TCC_LCDC_IMG_FMT_YUV422SP = 25, 
	TCC_LCDC_IMG_FMT_YUYV = 26, 
	TCC_LCDC_IMG_FMT_YVYU = 27,
	
	TCC_LCDC_IMG_FMT_YUV420ITL0 = 28, 
	TCC_LCDC_IMG_FMT_YUV420ITL1 = 29, 
	TCC_LCDC_IMG_FMT_YUV422ITL0 = 30, 
	TCC_LCDC_IMG_FMT_YUV422ITL1 = 31, 
	TCC_LCDC_IMG_FMT_MAX
}TCC_LCDC_IMG_FMT_TYPE;

struct tcc_lcdc_image_update
{
	unsigned int Lcdc_layer;
	unsigned int enable;
	unsigned int Frame_width;
	unsigned int Frame_height;

	unsigned int Image_width;
	unsigned int Image_height;
	unsigned int offset_x; //position
	unsigned int offset_y; 	//position
	unsigned int addr0;
	unsigned int addr1;
	unsigned int addr2;
	unsigned int fmt;	//TCC_LCDC_IMG_FMT_TYPE
	unsigned int on_the_fly; // 0: not use , 1 : scaler0 ,  2 :scaler1	

// +----------------+----
// | src            |  
// |   +------+     |---- crop_top
// |   | dest |     |
// |   |      |     |
// |   +------+     |---- crop_bottom (crop_top + width of dest)
// |                |  
// +----------------+-----
// |<->|  crop_left    
// 		
// |<-------->|  crop_right (crop_top + height of dest)
//
	int crop_top; 
	int crop_bottom;
	int crop_left;
	int crop_right;
	
#if defined(CONFIG_TCC_VIDEO_DISPLAY_BY_VSYNC_INT) || defined(TCC_VIDEO_DISPLAY_BY_VSYNC_INT)
	int time_stamp;
	int sync_time;
	int first_frame_after_seek;
	unsigned int buffer_unique_id;
	unsigned int overlay_used_flag;
	OUTPUT_SELECT outputMode;
	int output_path;

	int deinterlace_mode;
	int odd_first_flag;
	int m2m_mode;
	int output_toMemory;
	int frameInfo_interlace;
#endif
	int one_field_only_interlace;

	int MVCframeView;
	unsigned int MVC_Base_addr0;
	unsigned int MVC_Base_addr1;
	unsigned int MVC_Base_addr2;

	unsigned int dst_addr0;
	unsigned int dst_addr1;
	unsigned int dst_addr2;

	int max_buffer;
	int ex_output;

	unsigned int codec_id;
};

struct tcc_lcdc_get_image_info
{
	unsigned int lcdc_num;
	unsigned int layer;
	unsigned int enable;
	unsigned int lcd_width;
	unsigned int lcd_height;
	unsigned int image_width;
	unsigned int image_height;
	unsigned int offset_x; //position
	unsigned int offset_y; 	//position
	unsigned int addr0;
	unsigned int addr1;
	unsigned int addr2;
	unsigned int fmt;	//TCC_LCDC_IMG_FMT_TYPE
};


typedef enum{
	LCDC_HDMI_NONE_MODE,
	LCDC_HDMI_UI_MODE,
	LCDC_HDMI_VIDEO_ONLY_MODE,
	LCDC_HDMI_OVERALY_MODE,
	LCDC_HDMI_MODE_MAX
}LCDC_HDMI_MODE;

struct lcdc_timimg_parms_t {

	unsigned int id;
	unsigned int iv;
	unsigned int ih;
	unsigned int ip;
	unsigned int dp;
	unsigned int ni;
	unsigned int tv;
	unsigned int tft;
	unsigned int stn;

	//LHTIME1
	unsigned int lpw;
	unsigned int lpc;
	//LHTIME2
	unsigned int lswc;
	unsigned int lewc;
	//LVTIME1
	unsigned int vdb;
	unsigned int vdf;
	unsigned int fpw;
	unsigned int flc;
	//LVTIME2
	unsigned int fswc;
	unsigned int fewc;
	//LVTIME3
	unsigned int fpw2;
	unsigned int flc2;
	//LVTIME4
	unsigned int fswc2;
	unsigned int fewc2;

};

typedef struct  
{
	unsigned lcdc_num;
	unsigned layer_num;
}lcdc_layerctrl_params;

typedef struct  
{
	unsigned lcdc_num;
	unsigned red;
	unsigned green;
	unsigned blue;
}lcdc_bgcolor_params;

typedef struct  
{
	unsigned lcdc_num;
	unsigned layer_num;
	unsigned layer_alpha;
}lcdc_alpha_params;

// chromaRGB 0~7 bit is chorma key color 
// chromaRGB 16~23 bit is chorma key mask value
typedef struct  
{
	unsigned lcdc_num;
	unsigned layer_num;
	unsigned enable;
	unsigned chromaR; // 0~7 key color , 16~ 24 mask 
	unsigned chromaG; // 0~7 key color , 16~ 24 mask 
	unsigned chromaB; // 0~7 key color , 16~ 24 mask 
}lcdc_chromakey_params;

typedef struct  
{
	unsigned enable;
	unsigned color;
}lcdc_chroma_params;

typedef struct  
{
	unsigned lcdc_num;
	unsigned hue; 
	unsigned brightness; 
	unsigned contrast; 
}lcdc_colorenhance_params;

typedef struct  
{
	unsigned lcdc_num;
	unsigned onoff;
	unsigned GammaBGR[256];
}lcdc_gamma_params;

typedef struct  
{
	unsigned 	lcdc;
	unsigned 	wd;
	unsigned 	ht;
	unsigned 	index;
	struct tcc_lcdc_image_update image;
}exclusive_ui_update;

typedef struct  
{
	unsigned 	width;
	unsigned 	height;
	unsigned 	conversion;
}exclusive_ui_size;

typedef struct  
{
	unsigned 	index;
	unsigned 	buf_wd;
	unsigned 	buf_ht;
	unsigned 	src_x;
	unsigned 	src_y;
	unsigned 	src_wd;
	unsigned 	src_ht;
	unsigned 	dst_x;
	unsigned 	dst_y;
	void 		*pBaseAddr;
}exclusive_ui_frame;

typedef struct  
{
	unsigned 	ratio_x;
	unsigned 	ratio_y;
}exclusive_ui_ratio;

typedef struct  
{
	unsigned 	enable;
	unsigned 	bd;
	unsigned 	plane_width;
	unsigned 	plane_height;
	unsigned 	base_addr_dma;
	unsigned 	base_addr_cpu;
	unsigned 	interlace;
	unsigned 	process;
	unsigned 	update;
	unsigned 	clear;
	struct tcc_lcdc_image_update update_img;
}exclusive_ui_params;

typedef struct  
{
	unsigned 	enable;
	unsigned 	ratio;
	unsigned 	ratio_x;
	unsigned 	ratio_y;
	unsigned 	ar_wd;
	unsigned 	ar_ht;
	unsigned 	config;
}exclusive_ui_ar_params;

typedef struct{
	unsigned int num;		// wmixer number
	unsigned int ovp;		// wmixer overlay priority
}tccfb_set_wmix_order_type;

typedef struct
{
	unsigned int 	width;
	unsigned int 	height;
	unsigned int 	bits_per_pixel;
	unsigned int 	offset;
	unsigned int 	fence_fd;
}external_fbioput_vscreeninfo;

typedef struct
{
	unsigned int	onoff;
	unsigned int	brightness;
}lut_ctrl_params;

#define LASTFRAME_FOR_RESOLUTION_CHANGE	0x1
#define LASTFRAME_FOR_CODEC_CHANGE	0x2
#endif
