/* 
 * drivers/input/touchscreen/ft5x0x_ts.c
 *
 * FocalTech ft5x0x TouchScreen driver. 
 *
 * Copyright (c) 2010  Focal tech Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * VERSION          DATE            AUTHOR        Note
 *    1.0          2010-01-05            WenFS    only support mulititouch    Wenfs 2010-10-01
 *    2.0          2011-09-05                   Duxx      Add touch key, and project setting update, auto CLB command
 *     
 *
 */

#include <linux/i2c.h>
#include <linux/input.h>
#include "ft5x06_ts.h"
#include <linux/earlysuspend.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <mach/bsp.h>
#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <mach/gpio.h>
//#include <asm/jzsoc.h>

static struct i2c_client *this_client;

#define  CONFIG_FT5X0X_MULTITOUCH 1
#define   upgrade_Firmware_CTP    0//调试完驱动后在调试升级功能，要有升级FIRMWARE文件的i文件

struct ts_event {
    u16 au16_x[CFG_MAX_TOUCH_POINTS];              //x coordinate
    u16 au16_y[CFG_MAX_TOUCH_POINTS];              //y coordinate
    u8  au8_touch_event[CFG_MAX_TOUCH_POINTS];     //touch event:  0 -- down; 1-- contact; 2 -- contact
    u8  au8_finger_id[CFG_MAX_TOUCH_POINTS];       //touch ID
    u16    pressure;
    u8  touch_point;
};


struct ft5x0x_ts_data {
    struct input_dev    *input_dev;
    struct ts_event        event;
    struct work_struct     pen_event_work;
    struct workqueue_struct *ts_workqueue;
    struct early_suspend    early_suspend;
};


#if CFG_SUPPORT_TOUCH_KEY
int tsp_keycodes[CFG_NUMOFKEYS] ={

        KEY_MENU,
        KEY_HOME,
        KEY_BACK,
        KEY_SEARCH
};

char *tsp_keyname[CFG_NUMOFKEYS] ={

        "Menu",
        "Home",
        "Back",
        "Search"
};

static bool tsp_keystatus[CFG_NUMOFKEYS];
#endif
/***********************************************************************************************
Name    :    ft5x0x_i2c_rxdata 

Input    :    *rxdata
                     *length

Output    :    ret

function    :    

***********************************************************************************************/
static int ft5x0x_i2c_rxdata(char *rxdata, int length)
{
    int ret;

    struct i2c_msg msgs[] = {
        {
            .addr    = this_client->addr,
            .flags    = 0,
            .len    = 1,
            .buf    = rxdata,
        },
        {
            .addr    = this_client->addr,
            .flags    = I2C_M_RD,
            .len    = length,
            .buf    = rxdata,
        },
    };

    //msleep(1);
    ret = i2c_transfer(this_client->adapter, msgs, 2);
    if (ret < 0)
        pr_err("msg %s i2c read error: %d\n", __func__, ret);
    
    return ret;
}
/***********************************************************************************************
Name    :     

Input    :    
                     

Output    :    

function    :    

***********************************************************************************************/
static int ft5x0x_i2c_txdata(char *txdata, int length)
{
    int ret;

    struct i2c_msg msg[] = {
        {
            .addr    = this_client->addr,
            .flags    = 0,
            .len    = length,
            .buf    = txdata,
        },
    };

       //msleep(1);
    ret = i2c_transfer(this_client->adapter, msg, 1);
    if (ret < 0)
        pr_err("%s i2c write error: %d\n", __func__, ret);

    return ret;
}
/***********************************************************************************************
Name    :     ft5x0x_write_reg

Input    :    addr -- address
                     para -- parameter

Output    :    

function    :    write register of ft5x0x

***********************************************************************************************/
static int ft5x0x_write_reg(u8 addr, u8 para)
{
    u8 buf[3];
    int ret = -1;

    buf[0] = addr;
    buf[1] = para;
    ret = ft5x0x_i2c_txdata(buf, 2);
    if (ret < 0) {
        pr_err("write reg failed! %#x ret: %d", buf[0], ret);
        return -1;
    }
    
    return 0;
}


/***********************************************************************************************
Name    :    ft5x0x_read_reg 

Input    :    addr
                     pdata

Output    :    

function    :    read register of ft5x0x

***********************************************************************************************/
static int ft5x0x_read_reg(u8 addr, u8 *pdata)
{
    int ret;
    u8 buf[2];
    struct i2c_msg msgs[2];

    //
    buf[0] = addr;    //register address
    
    msgs[0].addr = this_client->addr;
    msgs[0].flags = 0;
    msgs[0].len = 1;
    msgs[0].buf = buf;
    msgs[1].addr = this_client->addr;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = 1;
    msgs[1].buf = buf;

    ret = i2c_transfer(this_client->adapter, msgs, 2);
    if (ret < 0)
        pr_err("msg %s i2c read error: %d\n", __func__, ret);

    *pdata = buf[0];
    return ret;
  
}


/***********************************************************************************************
Name    :     ft5x0x_read_fw_ver

Input    :     void
                     

Output    :     firmware version     

function    :     read TP firmware version

***********************************************************************************************/
static unsigned char ft5x0x_read_fw_ver(void)
{
    unsigned char ver;
    ft5x0x_read_reg(FT5X0X_REG_FIRMID, &ver);
    return(ver);
}

#if upgrade_Firmware_CTP  //upgrade related
typedef enum
{
    ERR_OK,
    ERR_MODE,
    ERR_READID,
    ERR_ERASE,
    ERR_STATUS,
    ERR_ECC,
    ERR_DL_ERASE_FAIL,
    ERR_DL_PROGRAM_FAIL,
    ERR_DL_VERIFY_FAIL
}E_UPGRADE_ERR_TYPE;

typedef unsigned char         FTS_BYTE;     //8 bit
typedef unsigned short        FTS_WORD;    //16 bit
typedef unsigned int          FTS_DWRD;    //16 bit
typedef unsigned char         FTS_BOOL;    //8 bit

typedef struct _FTS_CTP_PROJECT_SETTING_T
{
    unsigned char uc_i2C_addr;             //I2C slave address (8 bit address)
    unsigned char uc_io_voltage;           //IO Voltage 0---3.3v;    1----1.8v
    unsigned char uc_panel_factory_id;     //TP panel factory ID
}FTS_CTP_PROJECT_SETTING_T;

#define FTS_NULL                0x0
#define FTS_TRUE                0x01
#define FTS_FALSE              0x0

#define I2C_CTPM_ADDRESS       (0x70>>1)//android 要右移一位。


void delay_qt_ms(unsigned long  w_ms)
{
    unsigned long i;
    unsigned long j;

    for (i = 0; i < w_ms; i++)
    {
        for (j = 0; j < 1000; j++)
        {
            udelay(1);
        }
    }
}


/*
[function]: 
    callback: read data from ctpm by i2c interface,implemented by special user;
[parameters]:
    bt_ctpm_addr[in]    :the address of the ctpm;
    pbt_buf[out]        :data buffer;
    dw_lenth[in]        :the length of the data buffer;
[return]:
    FTS_TRUE     :success;
    FTS_FALSE    :fail;
*/
FTS_BOOL i2c_read_interface(FTS_BYTE bt_ctpm_addr, FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    int ret;
    
    ret=i2c_master_recv(this_client, pbt_buf, dw_lenth);

    if(ret<=0)
    {
        printk("[FTS]i2c_read_interface error\n");
        return FTS_FALSE;
    }
  
    return FTS_TRUE;
}

/*
[function]: 
    callback: write data to ctpm by i2c interface,implemented by special user;
[parameters]:
    bt_ctpm_addr[in]    :the address of the ctpm;
    pbt_buf[in]        :data buffer;
    dw_lenth[in]        :the length of the data buffer;
[return]:
    FTS_TRUE     :success;
    FTS_FALSE    :fail;
*/
FTS_BOOL i2c_write_interface(FTS_BYTE bt_ctpm_addr, FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    int ret;
    ret=i2c_master_send(this_client, pbt_buf, dw_lenth);
    if(ret<=0)
    {
        printk("[FTS]i2c_write_interface error line = %d, ret = %d\n", __LINE__, ret);
        return FTS_FALSE;
    }

    return FTS_TRUE;
}

/*
[function]: 
    send a command to ctpm.
[parameters]:
    btcmd[in]        :command code;
    btPara1[in]    :parameter 1;    
    btPara2[in]    :parameter 2;    
    btPara3[in]    :parameter 3;    
    num[in]        :the valid input parameter numbers, if only command code needed and no parameters followed,then the num is 1;    
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
FTS_BOOL cmd_write(FTS_BYTE btcmd,FTS_BYTE btPara1,FTS_BYTE btPara2,FTS_BYTE btPara3,FTS_BYTE num)
{
    FTS_BYTE write_cmd[4] = {0};

    write_cmd[0] = btcmd;
    write_cmd[1] = btPara1;
    write_cmd[2] = btPara2;
    write_cmd[3] = btPara3;
    return i2c_write_interface(I2C_CTPM_ADDRESS, write_cmd, num);
}

/*
[function]: 
    write data to ctpm , the destination address is 0.
[parameters]:
    pbt_buf[in]    :point to data buffer;
    bt_len[in]        :the data numbers;    
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
FTS_BOOL byte_write(FTS_BYTE* pbt_buf, FTS_DWRD dw_len)
{
    
    return i2c_write_interface(I2C_CTPM_ADDRESS, pbt_buf, dw_len);
}

/*
[function]: 
    read out data from ctpm,the destination address is 0.
[parameters]:
    pbt_buf[out]    :point to data buffer;
    bt_len[in]        :the data numbers;    
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
FTS_BOOL byte_read(FTS_BYTE* pbt_buf, FTS_BYTE bt_len)
{
    return i2c_read_interface(I2C_CTPM_ADDRESS, pbt_buf, bt_len);
}


/*
[function]: 
    burn the FW to ctpm.
[parameters]:(ref. SPEC)
    pbt_buf[in]    :point to Head+FW ;
    dw_lenth[in]:the length of the FW + 6(the Head length);    
    bt_ecc[in]    :the ECC of the FW
[return]:
    ERR_OK        :no error;
    ERR_MODE    :fail to switch to UPDATE mode;
    ERR_READID    :read id fail;
    ERR_ERASE    :erase chip fail;
    ERR_STATUS    :status error;
    ERR_ECC        :ecc error.
*/


#define    FTS_PACKET_LENGTH        128

static unsigned char CTPM_FW[]=
{
#include "ft_app.i"
};

E_UPGRADE_ERR_TYPE  fts_ctpm_fw_upgrade(FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    FTS_BYTE reg_val[2] = {0};
    FTS_DWRD i = 0;

    FTS_DWRD  packet_number;
    FTS_DWRD  j;
    FTS_DWRD  temp;
    FTS_DWRD  lenght;
    FTS_BYTE  packet_buf[FTS_PACKET_LENGTH + 6];
    FTS_BYTE  auc_i2c_write_buf[10];
    FTS_BYTE bt_ecc;
    int      i_ret;

    /*********Step 1:Reset  CTPM *****/
    /*write 0xaa to register 0xfc*/
    ft5x0x_write_reg(0xfc,0xaa);
    delay_qt_ms(50);
     /*write 0x55 to register 0xfc*/
    ft5x0x_write_reg(0xfc,0x55);
    printk("[FTS] Step 1: Reset CTPM test\n");
   
    delay_qt_ms(30);   


    /*********Step 2:Enter upgrade mode *****/
    auc_i2c_write_buf[0] = 0x55;
    auc_i2c_write_buf[1] = 0xaa;
    do
    {
        i ++;
        i_ret = ft5x0x_i2c_txdata(auc_i2c_write_buf, 2);
        delay_qt_ms(5);
    }while(i_ret <= 0 && i < 5 );

    /*********Step 3:check READ-ID***********************/        
    cmd_write(0x90,0x00,0x00,0x00,4);
    byte_read(reg_val,2);
    if (reg_val[0] == 0x79 && reg_val[1] == 0x3)
    {
        printk("[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
    }
    else
    {
        return ERR_READID;
        //i_is_new_protocol = 1;
    }

    cmd_write(0xcd,0x0,0x00,0x00,1);
    byte_read(reg_val,1);
    printk("[FTS] bootloader version = 0x%x\n", reg_val[0]);

     /*********Step 4:erase app and panel paramenter area ********************/
    cmd_write(0x61,0x00,0x00,0x00,1);  //erase app area
    delay_qt_ms(1500); 
    cmd_write(0x63,0x00,0x00,0x00,1);  //erase panel parameter area
    delay_qt_ms(100);
    printk("[FTS] Step 4: erase. \n");

    /*********Step 5:write firmware(FW) to ctpm flash*********/
    bt_ecc = 0;
    printk("[FTS] Step 5: start upgrade. \n");
    dw_lenth = dw_lenth - 8;
    packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
    packet_buf[0] = 0xbf;
    packet_buf[1] = 0x00;
    for (j=0;j<packet_number;j++)
    {
        temp = j * FTS_PACKET_LENGTH;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;
        lenght = FTS_PACKET_LENGTH;
        packet_buf[4] = (FTS_BYTE)(lenght>>8);
        packet_buf[5] = (FTS_BYTE)lenght;

        for (i=0;i<FTS_PACKET_LENGTH;i++)
        {
            packet_buf[6+i] = pbt_buf[j*FTS_PACKET_LENGTH + i]; 
            bt_ecc ^= packet_buf[6+i];
        }
        
        byte_write(&packet_buf[0],FTS_PACKET_LENGTH + 6);
        delay_qt_ms(FTS_PACKET_LENGTH/6 + 1);
        if ((j * FTS_PACKET_LENGTH % 1024) == 0)
        {
              printk("[FTS] upgrade the 0x%x th byte.\n", ((unsigned int)j) * FTS_PACKET_LENGTH);
        }
    }

    if ((dw_lenth) % FTS_PACKET_LENGTH > 0)
    {
        temp = packet_number * FTS_PACKET_LENGTH;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;

        temp = (dw_lenth) % FTS_PACKET_LENGTH;
        packet_buf[4] = (FTS_BYTE)(temp>>8);
        packet_buf[5] = (FTS_BYTE)temp;

        for (i=0;i<temp;i++)
        {
            packet_buf[6+i] = pbt_buf[ packet_number*FTS_PACKET_LENGTH + i]; 
            bt_ecc ^= packet_buf[6+i];
        }

        byte_write(&packet_buf[0],temp+6);    
        delay_qt_ms(20);
    }

    //send the last six byte
    for (i = 0; i<6; i++)
    {
        temp = 0x6ffa + i;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;
        temp =1;
        packet_buf[4] = (FTS_BYTE)(temp>>8);
        packet_buf[5] = (FTS_BYTE)temp;
        packet_buf[6] = pbt_buf[ dw_lenth + i]; 
        bt_ecc ^= packet_buf[6];

        byte_write(&packet_buf[0],7);  
        delay_qt_ms(20);
    }


    /*********Step 6: read out checksum***********************/
    /*send the opration head*/
    cmd_write(0xcc,0x00,0x00,0x00,1);
    byte_read(reg_val,1);
    printk("[FTS] Step 6:  ecc read 0x%x, new firmware 0x%x. \n", reg_val[0], bt_ecc);
    if(reg_val[0] != bt_ecc)
    {
        return ERR_ECC;
    }

    /*********Step 7: reset the new FW***********************/
    cmd_write(0x07,0x00,0x00,0x00,1);

    msleep(300);  //make sure CTP startup normally
    
    return ERR_OK;
}

int fts_ctpm_auto_clb(void)
{
    unsigned char uc_temp;
    unsigned char i ;

    printk("[FTS] start auto CLB.\n");
    msleep(200);
    ft5x0x_write_reg(0, 0x40);  
    delay_qt_ms(100);   //make sure already enter factory mode
    ft5x0x_write_reg(2, 0x4);  //write command to start calibration
    delay_qt_ms(300);
    for(i=0;i<100;i++)
    {
        ft5x0x_read_reg(0,&uc_temp);
        if ( ((uc_temp&0x70)>>4) == 0x0)  //return to normal mode, calibration finish
        {
            break;
        }
        delay_qt_ms(200);
        printk("[FTS] waiting calibration %d\n",i);
        
    }
    printk("[FTS] calibration OK.\n");
    
    msleep(300);
    ft5x0x_write_reg(0, 0x40);  //goto factory mode
    delay_qt_ms(100);   //make sure already enter factory mode
    ft5x0x_write_reg(2, 0x5);  //store CLB result
    delay_qt_ms(300);
    ft5x0x_write_reg(0, 0x0); //return to normal mode 
    msleep(300);
    printk("[FTS] store CLB result OK.\n");
    return 0;
}

int fts_ctpm_fw_upgrade_with_i_file(void)
{
   FTS_BYTE*     pbt_buf = FTS_NULL;
   int i_ret;
    
    //=========FW upgrade========================*/
   pbt_buf = CTPM_FW;
   /*call the upgrade function*/
   i_ret =  fts_ctpm_fw_upgrade(pbt_buf,sizeof(CTPM_FW));
   if (i_ret != 0)
   {
       printk("[FTS] upgrade failed i_ret = %d.\n", i_ret);
       //error handling ...
       //TBD
   }
   else
   {
       printk("[FTS] upgrade successfully.\n");
       fts_ctpm_auto_clb();  //start auto CLB
   }

   return i_ret;
}

unsigned char fts_ctpm_get_i_file_ver(void)
{
    unsigned int ui_sz;
    ui_sz = sizeof(CTPM_FW);
    if (ui_sz > 2)
    {
        return CTPM_FW[ui_sz - 2];
    }
    else
    {
        //TBD, error handling?
        return 0xff; //default value
    }
}

#define    FTS_SETTING_BUF_LEN        128

//update project setting
//only update these settings for COB project, or for some special case
int fts_ctpm_update_project_setting(void)
{
    unsigned char uc_i2c_addr;             //I2C slave address (8 bit address)
    unsigned char uc_io_voltage;           //IO Voltage 0---3.3v;    1----1.8v
    unsigned char uc_panel_factory_id;     //TP panel factory ID

    unsigned char buf[FTS_SETTING_BUF_LEN];
    FTS_BYTE reg_val[2] = {0};
    FTS_BYTE  auc_i2c_write_buf[10];
    FTS_BYTE  packet_buf[FTS_SETTING_BUF_LEN + 6];
    FTS_DWRD i = 0;
    int      i_ret;

    uc_i2c_addr = 0x0;
    uc_io_voltage = 0x0;
    uc_panel_factory_id = 0x5a;

    /*********Step 1:Reset  CTPM *****/
    /*write 0xaa to register 0xfc*/
    ft5x0x_write_reg(0xfc,0xaa);
    delay_qt_ms(50);
     /*write 0x55 to register 0xfc*/
    ft5x0x_write_reg(0xfc,0x55);
    printk("[FTS] Step 1: Reset CTPM test\n");
   
    delay_qt_ms(30);   

    /*********Step 2:Enter upgrade mode *****/
    auc_i2c_write_buf[0] = 0x55;
    auc_i2c_write_buf[1] = 0xaa;
    do
    {
        i ++;
        i_ret = ft5x0x_i2c_txdata(auc_i2c_write_buf, 2);
        delay_qt_ms(5);
    }while(i_ret <= 0 && i < 5 );

    /*********Step 3:check READ-ID***********************/        
    cmd_write(0x90,0x00,0x00,0x00,4);
    byte_read(reg_val,2);
    if (reg_val[0] == 0x79 && reg_val[1] == 0x3)
    {
        printk("[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
    }
    else
    {
        return ERR_READID;
    }

    cmd_write(0xcd,0x0,0x00,0x00,1);
    byte_read(reg_val,1);
    printk("bootloader version = 0x%x\n", reg_val[0]);


    /* --------- read current project setting  ---------- */
    //set read start address
    buf[0] = 0x3;
    buf[1] = 0x0;
    buf[2] = 0x78;
    buf[3] = 0x0;
    byte_write(buf, 4);
    byte_read(buf, FTS_SETTING_BUF_LEN);
    
    printk("[FTS] old setting: uc_i2c_addr = 0x%x, uc_io_voltage = %d, uc_panel_factory_id = 0x%x\n",
        buf[0],  buf[2], buf[4]);
    for (i = 0; i < FTS_SETTING_BUF_LEN; i++)
    {
        if (i % 16 == 0)     printk("\n");
        printk("0x%x, ", buf[i]);
        
    }
    printk("\n");

     /*--------- Step 4:erase project setting --------------*/
    cmd_write(0x62,0x00,0x00,0x00,1);
    delay_qt_ms(100);
   
    /*----------  Set new settings ---------------*/
    buf[0] = uc_i2c_addr;
    buf[1] = ~uc_i2c_addr;
    buf[2] = uc_io_voltage;
    buf[3] = ~uc_io_voltage;
    buf[4] = uc_panel_factory_id;
    buf[5] = ~uc_panel_factory_id;
    packet_buf[0] = 0xbf;
    packet_buf[1] = 0x00;
    packet_buf[2] = 0x78;
    packet_buf[3] = 0x0;
    packet_buf[4] = 0;
    packet_buf[5] = FTS_SETTING_BUF_LEN;
    for (i = 0; i < FTS_SETTING_BUF_LEN; i++)
    {
        packet_buf[6 + i] = buf[i];
        if (i % 16 == 0)     printk("\n");
        printk("0x%x, ", buf[i]);
    }
    printk("\n");
    byte_write(&packet_buf[0],FTS_SETTING_BUF_LEN + 6);
    delay_qt_ms(100);

    /********* reset the new FW***********************/
    cmd_write(0x07,0x00,0x00,0x00,1);

    msleep(200);

    return 0;
    
}



int fts_ctpm_read_project_setting(void)
{
    unsigned char uc_i2c_addr;             //I2C slave address (8 bit address)
    unsigned char uc_io_voltage;           //IO Voltage 0---3.3v;    1----1.8v
    unsigned char uc_panel_factory_id;     //TP panel factory ID

    unsigned char buf[FTS_SETTING_BUF_LEN];
    FTS_BYTE reg_val[2] = {0};
    FTS_BYTE  auc_i2c_write_buf[10];
    FTS_DWRD i = 0;
    int      i_ret;

    uc_i2c_addr = 0x70;
    uc_io_voltage = 0x0;
    uc_panel_factory_id = 0x5a;

    /*********Step 1:Reset  CTPM *****/
    /*write 0xaa to register 0xfc*/
    ft5x0x_write_reg(0xfc,0xaa);
    delay_qt_ms(50);
     /*write 0x55 to register 0xfc*/
    ft5x0x_write_reg(0xfc,0x55);
    printk("[FTS] Step 1: Reset CTPM test\n");
   
    delay_qt_ms(30);   

    /*********Step 2:Enter upgrade mode *****/
    auc_i2c_write_buf[0] = 0x55;
    auc_i2c_write_buf[1] = 0xaa;
    do
    {
        i ++;
        i_ret = ft5x0x_i2c_txdata(auc_i2c_write_buf, 2);
        delay_qt_ms(5);
    }while(i_ret <= 0 && i < 5 );

    /*********Step 3:check READ-ID***********************/        
    cmd_write(0x90,0x00,0x00,0x00,4);
    byte_read(reg_val,2);
    if (reg_val[0] == 0x79 && reg_val[1] == 0x3)
    {
        printk("[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
    }
    else
    {
        return ERR_READID;
    }

    cmd_write(0xcd,0x0,0x00,0x00,1);
    byte_read(reg_val,1);
    printk("bootloader version = 0x%x\n", reg_val[0]);


    /* --------- read current project setting  ---------- */
    //set read start address
    buf[0] = 0x3;
    buf[1] = 0x0;
    buf[2] = 0x78;
    buf[3] = 0x0;
    byte_write(buf, 4);
    byte_read(buf, FTS_SETTING_BUF_LEN);
    
    printk("[FTS] old setting: uc_i2c_addr = 0x%x, uc_io_voltage = %d, uc_panel_factory_id = 0x%x\n",
        buf[0],  buf[2], buf[4]);
    for (i = 0; i < FTS_SETTING_BUF_LEN; i++)
    {
        if (i % 16 == 0)     printk("\n");
        printk("0x%x, ", buf[i]);
        
    }
    printk("\n");

    /********* reset the new FW***********************/
    cmd_write(0x07,0x00,0x00,0x00,1);

    msleep(200);

    return 0;
    
}



#if CFG_SUPPORT_AUTO_UPG

int fts_ctpm_auto_upg(void)
{
    unsigned char uc_host_fm_ver;
    unsigned char uc_tp_fm_ver;
    int           i_ret;

    uc_tp_fm_ver = ft5x0x_read_fw_ver();
    uc_host_fm_ver = fts_ctpm_get_i_file_ver();
    if ( uc_tp_fm_ver == 0xa6  ||   //the firmware in touch panel maybe corrupted
         uc_tp_fm_ver < uc_host_fm_ver //the firmware in host flash is new, need upgrade
        )
    {
        msleep(100);
        printk("[FTS] uc_tp_fm_ver = 0x%x, uc_host_fm_ver = 0x%x\n",
            uc_tp_fm_ver, uc_host_fm_ver);
        i_ret = fts_ctpm_fw_upgrade_with_i_file();    
        if (i_ret == 0)
        {
            msleep(300);
            uc_host_fm_ver = ft5x0x_read_fw_ver();
            printk("[FTS] upgrade to new version 0x%x\n", uc_host_fm_ver);
        }
        else
        {
            printk("[FTS] upgrade failed ret=%d.\n", i_ret);
        }
    }

    return 0;
}

#endif

#endif




#if CFG_DBG_RAW_DATA_READ
#define FTS_TX_NUM 28
#define FTS_RX_NUM 16
static unsigned char _suc_raw_data[FTS_TX_NUM][FTS_RX_NUM * 2];


int fts_read_line(unsigned char uc_row, unsigned char uc_buf[], unsigned char uc_rx_num)
{
    int        i_ret;

    //set the row address
    i_ret = ft5x0x_write_reg(0x1, uc_row);
    if (i_ret < 0 )
    {
        return -1;
    }

    uc_buf[0] = 0x10;
    i_ret = ft5x0x_i2c_rxdata(uc_buf, uc_rx_num * 2);
    if (i_ret < 0)
    {
        return -2;
    }

    return 0;
    
}

int fts_raw_data_test(void)
{
    int i;
    int j;
    int i_test;
    unsigned short uc_data;

    msleep(200);  //make sure the CTP already start up normally
    
    ft5x0x_write_reg(0, 0x40); //switch to factory mode 

    msleep(100);  //make sure already enter factory mode

    printk("[TSP] raw data test start ----------------------- \n");

    for (i_test = 0; i_test < 10; i_test ++)
    {
        printk("frame %d *********\n", i_test + 1);
        ft5x0x_write_reg(0, 0xC0); //trigger start raw data scan bit
        msleep(10);
        for ( i = 0; i < FTS_TX_NUM; i ++)
        {
            msleep(1);
            fts_read_line(i, _suc_raw_data[i], FTS_RX_NUM);
            for ( j = 0; j<FTS_RX_NUM; j++)
            {
                uc_data = _suc_raw_data[i][2*j];
                uc_data <<= 8;
                uc_data |= _suc_raw_data[i][2*j + 1];
                printk("%5d ", uc_data);
            }
            printk("\n");
        }
        msleep(30);
        
    }
    printk("[TSP] raw data test end ----------------------- \n");

    return 0;
}

#endif

/***********************************************************************************************
Name    :     

Input    :    
                     

Output    :    

function    :    

***********************************************************************************************/
static void ft5x0x_ts_release(void)
{
    struct ft5x0x_ts_data *data = i2c_get_clientdata(this_client);
    input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
    input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 0);
	input_report_key(data->input_dev,BTN_TOUCH,0);
    input_sync(data->input_dev);
}


//read touch point information
static int ft5x0x_read_data(void)
{
    struct ft5x0x_ts_data *data = i2c_get_clientdata(this_client);
    struct ts_event *event = &data->event;
    u8 buf[CFG_POINT_READ_BUF] = {0};
    int ret = -1;
    int i;
    //printk("%s\n", __func__);


    ret = ft5x0x_i2c_rxdata(buf, CFG_POINT_READ_BUF);
    if (ret < 0) {
        printk("%s read_data i2c_rxdata failed: %d\n", __func__, ret);
        return ret;
    }
    memset(event, 0, sizeof(struct ts_event));
    event->touch_point = buf[2] & 0x07; 
    //event->touch_point = buf[2] >> 4; 
	//printk("buf[2] = 0x%x\n", buf[2]);


    if (event->touch_point > CFG_MAX_TOUCH_POINTS)
    {
        event->touch_point = CFG_MAX_TOUCH_POINTS;
    }

    for (i = 0; i < event->touch_point; i++)
    {
        event->au16_x[i] = SCREEN_MAX_X - ((s16)(buf[3 + 6*i] & 0x0F)<<8 | (s16)buf[4 + 6*i]);
        event->au16_y[i] = SCREEN_MAX_Y - ((s16)(buf[5 + 6*i] & 0x0F)<<8 | (s16)buf[6 + 6*i]);
        event->au8_touch_event[i] = buf[0x3 + 6*i] >> 6;
        event->au8_finger_id[i] = (buf[5 + 6*i])>>4;
		if (event->au16_x[i] >= SCREEN_MAX_LIMIT_X)
			event->au16_x[i] = SCREEN_MAX_LIMIT_X;
		if (event->au16_x[i] <= SCREEN_MIN_LIMIT_X)
			event->au16_x[i] = SCREEN_MIN_LIMIT_X;
		if (event->au16_y[i] >= SCREEN_MAX_LIMIT_Y)
			event->au16_y[i] = SCREEN_MAX_LIMIT_Y;
		if (event->au16_y[i] <= SCREEN_MIN_LIMIT_Y)
			event->au16_y[i] = SCREEN_MIN_LIMIT_Y;
        //printk("%d, %d\n", event->au16_x[i], event->au16_y[i]);
    }

    event->pressure = 200;

#if CFG_SUPPORT_READ_LEFT_DATA
    return buf[1];  //the left touch data in the frames
#else
    return 0;
#endif
    
}

/***********************************************************************************************
Name    :     

Input    :    
                     

Output    :    

function    :    

***********************************************************************************************/


#if CFG_SUPPORT_TOUCH_KEY
int ft5x0x_touch_key_process(struct input_dev *dev, int x, int y, int touch_event)
{
    int i;
    int key_id;
    
    if ( y < 894&&y > 834)
    {
        key_id = 1;
    }
    else if ( y < 638&&y > 578)
    {
        key_id = 0;
    }
    
    else if ( y < 382&&y > 322)
    {
        key_id = 2;
    }  
    else if (y < 126&&y > 36)
    {
        key_id = 3;
    }
    else
    {
        key_id = 0xf;
    }
    
    for(i = 0; i <CFG_NUMOFKEYS; i++ )
    {
        if(tsp_keystatus[i])
        {
            input_report_key(dev, tsp_keycodes[i], 0);
      
            printk("[FTS] %s key is release. Keycode : %d\n", tsp_keyname[i], tsp_keycodes[i]);

            tsp_keystatus[i] = KEY_RELEASE;
        }
        else if( key_id == i )
        {
            if( touch_event == 0)                                  // detect
            {
                input_report_key(dev, tsp_keycodes[i], 1);
                printk( "[FTS] %s key is pressed. Keycode : %d\n", tsp_keyname[i], tsp_keycodes[i]);
                tsp_keystatus[i] = KEY_PRESS;
            }
        }
    }
    return 0;
    
}    
#endif

static void ft5x0x_report_value(void)
{
    struct ft5x0x_ts_data *data = i2c_get_clientdata(this_client);
    struct ts_event *event = &data->event;
    int i;
	//printk("%s\n", __func__);

    for (i  = 0; i < event->touch_point; i++)
    {
        if (event->au16_x[i] < SCREEN_MAX_X && event->au16_y[i] < SCREEN_MAX_Y)
        // LCD view area
        {
            input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->au16_x[i]);
            input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->au16_y[i]);
            //input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 1);
            input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, event->au8_finger_id[i]);
            if (event->au8_touch_event[i]== 0 || event->au8_touch_event[i] == 2)
            {
                input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
				//input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 1);
				input_report_key(data->input_dev,BTN_TOUCH,1);
            }
            else
            {
                input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
				input_report_key(data->input_dev,BTN_TOUCH,0);
            }
        }
        else //maybe the touch key area
        {
#if CFG_SUPPORT_TOUCH_KEY
            if (event->au16_x[i] >= SCREEN_MAX_X)
            {
                //printk("key x = %d\n", event->au16_y[i]);
                ft5x0x_touch_key_process(data->input_dev, event->au16_x[i], event->au16_y[i], event->au8_touch_event[i]);
            }
#endif
        }
        
            
        input_mt_sync(data->input_dev);
    }
    input_sync(data->input_dev);

    if (event->touch_point == 0) {
        ft5x0x_ts_release();
        return ; 
    }


}    /*end ft5x0x_report_value*/


/***********************************************************************************************
Name    :     

Input    :    
                     

Output    :    

function    :    

***********************************************************************************************/
static void ft5x0x_ts_pen_irq_work(struct work_struct *work)
{
    int ret = -1;
	//printk("%s\n", __func__);

#if CFG_SUPPORT_READ_LEFT_DATA
    do
    {
        ret = ft5x0x_read_data();    
        if (ret >= 0) {    
            ft5x0x_report_value();
        }
    }while (ret > 0);

#else
    ret = ft5x0x_read_data();    
    if (ret == 0) {    
        ft5x0x_report_value();
    }
#endif    
    enable_irq(INT_IRQ);
}
/***********************************************************************************************
Name    :     

Input    :    
                     

Output    :    

function    :    

***********************************************************************************************/
static irqreturn_t ft5x0x_ts_interrupt(int irq, void *dev_id)
{
    struct ft5x0x_ts_data *ft5x0x_ts = dev_id;

	//printk("%s\n", __func__);
    disable_irq_nosync(INT_IRQ);
    if (!work_pending(&ft5x0x_ts->pen_event_work)) {
        queue_work(ft5x0x_ts->ts_workqueue, &ft5x0x_ts->pen_event_work);
    }

    return IRQ_HANDLED;
}
#ifdef CONFIG_HAS_EARLYSUSPEND
/***********************************************************************************************
Name    :     

Input    :    
                     

Output    :    

function    :    

***********************************************************************************************/
static void ft5x0x_ts_suspend(struct early_suspend *handler)
{
//    struct ft5x0x_ts_data *ts;
//    ts =  container_of(handler, struct ft5x0x_ts_data, early_suspend);

    printk("==ft5x0x_ts_suspend=\n");
//    disable_irq(this_client->irq);
//    disable_irq(INT_IRQ);
//    cancel_work_sync(&ts->pen_event_work);
//    flush_workqueue(ts->ts_workqueue);
    // ==set mode ==, 
//        ft5x0x_set_reg(FT5X0X_REG_PMODE, PMODE_HIBERNATE);
}
/***********************************************************************************************
Name    :     

Input    :    
                     

Output    :    

function    :    

***********************************************************************************************/
static void ft5x0x_ts_resume(struct early_suspend *handler)
{
    printk("==ft5x0x_ts_resume=\n");
    // wake the mode
//    __gpio_as_output(GPIO_FT5X0X_WAKE);        
//    __gpio_clear_pin(GPIO_FT5X0X_WAKE);        //set wake = 0,base on system
//     msleep(100);
//    __gpio_set_pin(GPIO_FT5X0X_WAKE);            //set wake = 1,base on system
//    msleep(100);
//    enable_irq(this_client->irq);
//    enable_irq(INT_IRQ);
}
#endif  //CONFIG_HAS_EARLYSUSPEND
/***********************************************************************************************
Name    :     

Input    :    
                     

Output    :    

function    :    

***********************************************************************************************/
static int 
ft5x0x_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct ft5x0x_ts_data *ft5x0x_ts;
    struct input_dev *input_dev;
    int err = 0;
	int ret;
    unsigned char uc_reg_value; 
#if CFG_SUPPORT_TOUCH_KEY
    int i;
#endif

    
    printk("[FTS] ft5x0x_ts_probe, driver version is %s.\n", CFG_FTS_CTP_DRIVER_VERSION);
    
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
        err = -ENODEV;
        goto exit_check_functionality_failed;
    }

    ft5x0x_ts = kzalloc(sizeof(*ft5x0x_ts), GFP_KERNEL);
    if (!ft5x0x_ts)    {
        err = -ENOMEM;
        goto exit_alloc_data_failed;
    }


    this_client = client;
    i2c_set_clientdata(client, ft5x0x_ts);


    INIT_WORK(&ft5x0x_ts->pen_event_work, ft5x0x_ts_pen_irq_work);

    ft5x0x_ts->ts_workqueue = create_singlethread_workqueue(dev_name(&client->dev));
    if (!ft5x0x_ts->ts_workqueue) {
        err = -ESRCH;
        goto exit_create_singlethread;
    }

	ret = gpio_request(INT_GPIO, "TS_INT");	//Request IO
	if (ret < 0) 
	{
		printk("gpio_request INT_GPIO fail\n");
	}

	ret = gpio_request(RST_GPIO, "TS_RST");	//Request IO
	if (ret < 0) 
	{
		printk("gpio_request RST_GPIO fail\n");
	}

	//
	tcc_gpio_config(INT_GPIO, GPIO_FN(0)|GPIO_PULLUP);
	gpio_direction_input(INT_GPIO);
	tcc_gpio_config_ext_intr(INT_IRQ, EXTINT_GPIOD_14);

	tcc_gpio_config(RST_GPIO, GPIO_FN(0)|GPIO_PULL_DISABLE);
	gpio_direction_output(RST_GPIO, 1);
	msleep(200);
	gpio_direction_output(RST_GPIO, 0);
	msleep(200);
	gpio_direction_output(RST_GPIO, 1);

    err = request_irq(INT_IRQ, ft5x0x_ts_interrupt, IRQF_TRIGGER_FALLING, "ft5x0x_ts", ft5x0x_ts);
    if (err < 0) {
        dev_err(&client->dev, "ft5x0x_probe: request irq failed\n");
        goto exit_irq_request_failed;
    }


    disable_irq(INT_IRQ);

    input_dev = input_allocate_device();
    if (!input_dev) {
        err = -ENOMEM;
        dev_err(&client->dev, "failed to allocate input device\n");
        goto exit_input_dev_alloc_failed;
    }
    
    ft5x0x_ts->input_dev = input_dev;

	input_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS) ;
	input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);

	input_dev->absbit[0] = BIT_MASK(ABS_MT_TRACKING_ID) |
	BIT_MASK(ABS_MT_TOUCH_MAJOR)| BIT_MASK(ABS_MT_WIDTH_MAJOR) |
  	BIT_MASK(ABS_MT_POSITION_X) | BIT_MASK(ABS_MT_POSITION_Y);

	//sprintf(ts->phys, "input/goodix-ts");
	input_dev->name = FT5X0X_NAME;
	input_dev->phys = "input/goodix-ts";
	input_dev->id.bustype = BUS_I2C;
	input_dev->id.vendor = 0xDEAD;
	input_dev->id.product = 0xBEEF;
	input_dev->id.version = 0x1105;	

	input_set_abs_params(input_dev, ABS_X, 0, SCREEN_MAX_X, 0, 0);
	input_set_abs_params(input_dev, ABS_Y, 0, SCREEN_MAX_Y, 0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE, 0, 255, 0, 0);

    input_set_abs_params(input_dev,
                 ABS_MT_POSITION_X, 0, SCREEN_MAX_X, 0, 0);
    input_set_abs_params(input_dev,
                 ABS_MT_POSITION_Y, 0, SCREEN_MAX_Y, 0, 0);

#if CFG_SUPPORT_TOUCH_KEY
    //setup key code area
    set_bit(EV_SYN, input_dev->evbit);
    set_bit(BTN_TOUCH, input_dev->keybit);
    input_dev->keycode = tsp_keycodes;
    for(i = 0; i < CFG_NUMOFKEYS; i++)
    {
        input_set_capability(input_dev, EV_KEY, ((int*)input_dev->keycode)[i]);
        tsp_keystatus[i] = KEY_RELEASE;
    }
#endif

    //input_dev->name        = FT5X0X_NAME;        //dev_name(&client->dev)
    err = input_register_device(input_dev);
    if (err) {
        dev_err(&client->dev,
        "ft5x0x_ts_probe: failed to register input device: %s\n",
        dev_name(&client->dev));
        goto exit_input_register_device_failed;
    }

#ifdef CONFIG_HAS_EARLYSUSPEND
    printk("==register_early_suspend =\n");
    ft5x0x_ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
    ft5x0x_ts->early_suspend.suspend = ft5x0x_ts_suspend;
    ft5x0x_ts->early_suspend.resume    = ft5x0x_ts_resume;
    register_early_suspend(&ft5x0x_ts->early_suspend);
#endif

    msleep(150);  //make sure CTP already finish startup process

 
    //get some register information
    uc_reg_value = ft5x0x_read_fw_ver();
    printk("[FTS] Firmware version = 0x%x\n", uc_reg_value);
    ft5x0x_read_reg(FT5X0X_REG_PERIODACTIVE, &uc_reg_value);
    printk("[FTS] report rate is %dHz.\n", uc_reg_value * 10);
    ft5x0x_read_reg(FT5X0X_REG_THGROUP, &uc_reg_value);
    printk("[FTS] touch threshold is %d.\n", uc_reg_value * 4);


#if CFG_SUPPORT_AUTO_UPG
        fts_ctpm_auto_upg();
#endif    


#if CFG_SUPPORT_UPDATE_PROJECT_SETTING
    fts_ctpm_update_project_setting();
#endif

#if CFG_DBG_RAW_DATA_READ
    fts_raw_data_test();
#endif

//    msleep(300);
//    fts_ctpm_read_project_setting();

    enable_irq(INT_IRQ);

    printk("[FTS] ==probe over =\n");
    return 0;

exit_input_register_device_failed:
    input_free_device(input_dev);
exit_input_dev_alloc_failed:
//    free_irq(client->irq, ft5x0x_ts);
    free_irq(INT_IRQ, ft5x0x_ts);
exit_irq_request_failed:
//exit_platform_data_null:
    cancel_work_sync(&ft5x0x_ts->pen_event_work);
    destroy_workqueue(ft5x0x_ts->ts_workqueue);
exit_create_singlethread:
    printk("==singlethread error =\n");
    i2c_set_clientdata(client, NULL);
    kfree(ft5x0x_ts);
exit_alloc_data_failed:
exit_check_functionality_failed:
    return err;
}
/***********************************************************************************************
Name    :     

Input    :    
                     

Output    :    

function    :    

***********************************************************************************************/
static int __devexit ft5x0x_ts_remove(struct i2c_client *client)
{
    struct ft5x0x_ts_data *ft5x0x_ts;
    printk("==ft5x0x_ts_remove=\n");
    ft5x0x_ts = i2c_get_clientdata(client);
    unregister_early_suspend(&ft5x0x_ts->early_suspend);
//    free_irq(client->irq, ft5x0x_ts);
    free_irq(INT_IRQ, ft5x0x_ts);
    input_unregister_device(ft5x0x_ts->input_dev);
    kfree(ft5x0x_ts);
    cancel_work_sync(&ft5x0x_ts->pen_event_work);
    destroy_workqueue(ft5x0x_ts->ts_workqueue);
    i2c_set_clientdata(client, NULL);
    return 0;
}

static const struct i2c_device_id ft5x0x_ts_id[] = {
    { "Goodix-TS"/*FT5X0X_NAME*/, 0 },{ }
};


MODULE_DEVICE_TABLE(i2c, ft5x0x_ts_id);

static struct i2c_driver ft5x0x_ts_driver = {
    .probe        = ft5x0x_ts_probe,
    .remove        = __devexit_p(ft5x0x_ts_remove),
    .id_table    = ft5x0x_ts_id,
    .driver    = {
        .name    = "Goodix-TS",//FT5X0X_NAME,
        .owner    = THIS_MODULE,
    },
};

/***********************************************************************************************
Name    :     

Input    :    
                     

Output    :    

function    :    

***********************************************************************************************/
static int __init ft5x0x_ts_init(void)
{
    int ret;
    printk("==ft5x0x_ts_init==\n");
    ret = i2c_add_driver(&ft5x0x_ts_driver);
    printk("ret=%d\n",ret);
    return ret;
//    return i2c_add_driver(&ft5x0x_ts_driver);
}

/***********************************************************************************************
Name    :     

Input    :    
                     

Output    :    

function    :    

***********************************************************************************************/
static void __exit ft5x0x_ts_exit(void)
{
    printk("==ft5x0x_ts_exit==\n");
    i2c_del_driver(&ft5x0x_ts_driver);
}

module_init(ft5x0x_ts_init);
module_exit(ft5x0x_ts_exit);

MODULE_AUTHOR("<wenfs@Focaltech-systems.com>");
MODULE_DESCRIPTION("FocalTech ft5x0x TouchScreen driver");
MODULE_LICENSE("GPL");
