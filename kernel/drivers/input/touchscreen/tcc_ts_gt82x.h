/*
 * 
 * Copyright (C) 2011 Goodix, Inc.
 * 
 * Author: Scott
 * Date: 2011.11.08
 */

#ifndef _LINUX_GOODIX_TOUCH_H
#define _LINUX_GOODIX_TOUCH_H

#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <asm/uaccess.h>
#include <linux/fs.h>


#define fail    0
#define success 1

#define false   0
#define true    1

#if 1
#define DEBUG_MSG(fmt, arg...) printk("<--GT msg-->"fmt, ##arg)
#else
#define DEBUG_MSG(fmt, arg...)
#endif

#if 1
#define DEBUG_UPDATE(fmt, arg...) printk("<--GT update-->"fmt, ##arg)
#else
#define DEBUG_UPDATE(fmt, arg...)
#endif 

#if 0
#define DEBUG_COOR(fmt, arg...) printk(fmt, ##arg)
#else
#define DEBUG_COOR(fmt, arg...)
#define DEBUG_COORD
#endif

#if 0
#define DEBUG_ARRAY(array, num)   do{\
                                   int i; \
                                   for (i = 0; i < (num); i++)\
                                   {\
                                       printk("%02x   ", (array)[i]);\
                                       if ((i + 1 ) %10 == 0)\
                                       {\
                                           printk("\n");\
                                       }\
                                   }\
                                   printk("\n");\
                                  }while(0)
#else
#define DEBUG_ARRAY(array, num) 
#endif 


//-------------------------GPIO REDEFINE START----------------------------//
#define GPIO_DIRECTION_INPUT(port)          gpio_direction_input(port)
#define GPIO_DIRECTION_OUTPUT(port, val)    gpio_direction_output(port, val)
#define GPIO_SET_VALUE(port, val)           gpio_set_value(port, val)
#define GPIO_FREE(port)                     gpio_free(port)
#define GPIO_REQUEST(port, name)            gpio_request(port, name)
//-------------------------GPIO REDEFINE END------------------------------//


#define FW_HEAD_LENGTH   30
#define FILE_HEAD_LENGTH 100
#define IGNORE_LENGTH    100
#define FW_MSG_LENGTH    7
#define ADDR_LENGTH      2
#define UPDATE_DATA_LENGTH  5000

#define READ_TOUCH_ADDR_H   0x0F
#define READ_TOUCH_ADDR_L   0x40
#define READ_KEY_ADDR_H     0x0F
#define READ_KEY_ADDR_L     0x41
#define READ_COOR_ADDR_H    0x0F
#define READ_COOR_ADDR_L    0x42
#define READ_ID_ADDR_H      0x00
#define READ_ID_ADDR_L      0xff
#define READ_FW_MSG_ADDR_H    0x0F
#define READ_FW_MSG_ADDR_L    0x7C
#define UPDATE_FW_MSG_ADDR_H  0x40
#define UPDATE_FW_MSG_ADDR_L  0x50
#define READ_MSK_VER_ADDR_H   0xC0
#define READ_MSK_VER_ADDR_L   0x09

//*************************TouchScreen Work Part*****************************
#define GOODIX_I2C_NAME "tcc-ts"

#if 0
#define TOUCH_MAX_HEIGHT     480
#define TOUCH_MAX_WIDTH      800
#else
#define RESOLUTION_LOC      71
u16 TOUCH_MAX_HEIGHT;
u16 TOUCH_MAX_WIDTH;
//#define TOUCH_MAX_HEIGHT 480
//#define TOUCH_MAX_WIDTH  800
#endif
//mhao:20120604
//#define RESET_PORT      TCC_GPC(17)         //RESET port
//#define INT_PORT        TCC_GPC(26)         //Int IO port
#define RESET_PORT      TCC_GPEXT3(10)         //RESET port
#if defined(CONFIG_ARCH_TCC893X)
#if defined(CONFIG_CHIP_TCC8930)
#define INT_PORT        TCC_GPB(11)         //Int IO port
#elif defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8933) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
#define INT_PORT        TCC_GPE(13)         //Int IO port
#endif
#endif


//#define INT_TRIGGER     IRQ_TYPE_EDGE_FALLING       // 1=rising 0=falling
#define INT_TRIGGER     IRQ_TYPE_EDGE_RISING       // 1=rising 0=falling
#define POLL_TIME       10        //actual query spacing interval:POLL_TIME+6

#define GOODIX_MULTI_TOUCH
#ifdef GOODIX_MULTI_TOUCH
    #define MAX_FINGER_NUM    5
#else
    #define MAX_FINGER_NUM    1
#endif

#define swap(x, y) do { typeof(x) z = x; x = y; y = z; } while (0)

//****************************升级模块参数******************************************
//#define AUTO_UPDATE_GUITAR             //如果定义了则上电会自动判断是否需要升级
#ifdef AUTO_UPDATE_GUITAR
#define SEARCH_FILE_TIMES    100
#define UPDATE_FILE_PATH_2   "/data/goodix/_goodix_update_.bin"
#define UPDATE_FILE_PATH_1   "/sdcard/goodix/_goodix_update_.bin"
#endif

//#define CREATE_WR_NODE
#ifdef CREATE_WR_NODE
static int goodix_update_write(struct file *filp, const char __user *buff, unsigned long len, void *data);
static int goodix_update_read( char *page, char **start, off_t off, int count, int *eof, void *data );
#endif 

#define  PACK_SIZE              512                    //update file package size

#define  BIT_NVRAM_STROE        0
#define  BIT_NVRAM_RECALL       1
#define  BIT_NVRAM_LOCK         2
#define  REG_NVRCS_H            0X12
#define  REG_NVRCS_L            0X01

#pragma pack(1)
typedef struct 
{
    u8  type;          //产品类型//
    u16 version;       //FW版本号//
    u8  msk_ver[4];    //MASK版本//
    u8  st_addr[2];    //烧录的起始地址//
    u16 lenth;         //FW长度//
    u8  chk_sum[3];
    u8  force_update[6];//强制升级标志,为"GOODIX"则强制升级//
}st_fw_head;
#pragma pack()

//******************************************************************************
struct goodix_ts_data {
    uint8_t bad_data;
    uint8_t irq_flag;
    uint16_t addr;
    int use_reset;        //use RESET flag
    int use_irq;          //use EINT flag
    unsigned int version;
    int (*power)(struct goodix_ts_data * ts, int on);
    struct i2c_client *client;
    struct input_dev *input_dev;
    struct hrtimer timer;
    struct work_struct work;
    struct early_suspend early_suspend;
    char phys[32];
    
#ifdef AUTO_UPDATE_GUITAR
    uint8_t force_update;
    s32 gt_loc;
    st_fw_head  ic_fw_msg;
    struct file *file; 
    mm_segment_t old_fs;
#endif
};

//*************************Touchkey Surpport Part*****************************
//mhao:20120604
//#define HAVE_TOUCH_KEY
#ifdef HAVE_TOUCH_KEY
    const uint16_t touch_key_array[]={
          KEY_SEARCH,           //SEARCH
          KEY_BACK,             //BACK
          KEY_HOME,             //HOME  
          KEY_MENU,             //MENU
    }; 
    #define MAX_KEY_NUM     (sizeof(touch_key_array)/sizeof(touch_key_array[0]))
#endif

struct goodix_i2c_rmi_platform_data {
    uint32_t version;    /* Use this entry for panels with */
    int (*init_irq)(void);
    uint16_t x_min;	
    uint16_t x_max;
    uint16_t y_min;	
    uint16_t y_max;
};


#endif /* _LINUX_GOODIX_TOUCH_H */
