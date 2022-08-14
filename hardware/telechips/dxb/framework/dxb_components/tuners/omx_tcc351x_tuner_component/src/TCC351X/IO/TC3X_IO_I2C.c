/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions andlimitations under the License.

****************************************************************************/

#include "TC3X_IO_I2C.h"
#include "TC3X_IO.h"
#include "../TC3X_Common.h"

#if defined (USE_IF_I2C)
#define Bit7        0x00000080
#define MAX_I2C_BURST 512
#define I2C_SLAVE   0x0703  /* Change slave address         */

unsigned int gI2CHanleInit0 = 0;
unsigned int gI2CHanleInit1 = 0;
unsigned int gI2CHanleInited = 0;
unsigned char gI2CChipAddr[4];

unsigned char TempBuff[MAX_I2C_BURST+4];

int ghI2C[MAX_TCC3X] = {0,};

extern tSTDInfo g_tSTDInfo[MAX_TCC3X];

void SetupI2C (int moduleidx)
{
    if(moduleidx >= MAX_TCC3X)
    {
        PRINTF_LV_0 ("Not supported, moduleidx=%d\n", moduleidx);
        return;
    }
    
    ghI2C[moduleidx] = open("/dev/i2c-1",O_RDWR);
    if (ghI2C[moduleidx] < 0)
    {
        PRINTF_LV_0 ("INVALID_HANDLE_VALUE==ghSPI");
    }
}

void TC3X_IO_I2C_Variables_Init (int moduleidx)
{
    // none
}


//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_I2C_Init
//    Description : Initialize I2C
//    Input : 
//      moduleidx : module index
//    Output : 
//      
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_I2C_Init (int moduleidx)
{
    if(moduleidx==0)
        gI2CHanleInit0 = 1;
    else
        gI2CHanleInit1 = 1;

    if(gI2CHanleInited != 0)
    {
        return;
    }
    
    gI2CHanleInited = 1;

    TC3X_IO_memset(&gI2CChipAddr[moduleidx], 0x00, 4);

    // command : 0 5(opt.) 52 (1byte) 53 (4bytes)
    SetupI2C (moduleidx);
    TC3X_IO_PW_ON (moduleidx);
    TC3X_IO_PW_RESET (moduleidx);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_I2C_Close
//    Description : Close I2C
//    Input : 
//      moduleidx : module index
//    Output : 
//      TC3X_IO_SUCCESS
//    Remark : 
//--------------------------------------------------------------------------
TC3X_IO_Err TC3X_IO_I2C_Close(int moduleidx)
{
    if(moduleidx==0)
        gI2CHanleInit0 = 0;
    else
        gI2CHanleInit1 = 0;

    if(gI2CHanleInit0 == 0 && gI2CHanleInit1 == 0)
    {
        gI2CHanleInited = 0;
        
        PRINTF_LV_1("[%d] TC3X_IO_I2C_Close \n", moduleidx);
        close(ghI2C[0]);
    }

    return TC3X_IO_SUCCESS;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_I2C_Reg_Write
//    Description : Reg write
//    Input : 
//      moduleidx : module index
//      ChipAddr : chip address
//      RegAddr : Register Address
//      data : data
//    Output : 
//      TC3X_IO_SUCCESS
//    Remark : 
//--------------------------------------------------------------------------
TC3X_IO_Err TC3X_IO_I2C_Reg_Write (int moduleidx, int ChipAddr, int RegAddr, unsigned int data)
{
    int ret;
    unsigned char buf[2];
    
    buf[0] = (unsigned char)(RegAddr);
    buf[1] = (unsigned char)(data);

    TC3X_IO_IF_LOCK (0);

    if(gI2CChipAddr[moduleidx]!=ChipAddr)
    {
        gI2CChipAddr[moduleidx] = ChipAddr;
        ioctl(ghI2C[moduleidx], I2C_SLAVE, ChipAddr >> 1);
    }

    ret = write(ghI2C[moduleidx], buf, 2);
    if (ret !=sizeof(buf))
    {
        PRINTF_LV_0("\n i2c write err \n");
        return TC3X_IO_FAIL;        
    }
    
    TC3X_IO_IF_UnLOCK (0);

    if(!ret)
        return TC3X_IO_FAIL;
        
    return TC3X_IO_SUCCESS;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_I2C_Reg_Write_InnerSem
//    Description : Reg write (inside semaphore)
//      moduleidx : module index
//      ChipAddr : chip address
//      RegAddr : Register Address
//      data : data
//    Output : 
//      TC3X_IO_SUCCESS
///    Remark : 
//--------------------------------------------------------------------------
TC3X_IO_Err TC3X_IO_I2C_Reg_Write_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned int data)
{
    int ret;
    unsigned char buf[2];
    
    buf[0] = (unsigned char)(RegAddr);
    buf[1] = (unsigned char)(data);

    if(gI2CChipAddr[moduleidx] != ChipAddr)
    {
        gI2CChipAddr[moduleidx] = ChipAddr;
        ioctl(ghI2C[moduleidx], I2C_SLAVE, ChipAddr >> 1);
    }

    ret = write(ghI2C[moduleidx], &buf[0], 2);
    if (ret != sizeof(buf))
    {
        PRINTF_LV_0("\n i2c write err \n");
        return TC3X_IO_FAIL;        
    }
    
    if(!ret)
        return TC3X_IO_FAIL;
        
    return TC3X_IO_SUCCESS;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_I2C_Reg_Read
//    Description : Reg read
//    Input : 
//      moduleidx : module index
//      ChipAddr : chip address
//      RegAddr : Register Address
//      pError : Error flag
//    Output : 
//      TC3X_IO_SUCCESS///    Remark : 
//--------------------------------------------------------------------------
unsigned int TC3X_IO_I2C_Reg_Read (int moduleidx, int ChipAddr, int RegAddr, TC3X_IO_Err * pError)
{
    int data;
    unsigned int ret;
    int err;
    unsigned char buf[2];
    char addr = (ChipAddr&0xFF);
    data = 0;

    TC3X_IO_IF_LOCK (0);
    
    if(gI2CChipAddr[moduleidx] != ChipAddr)
    {
        gI2CChipAddr[moduleidx] = ChipAddr;
        ioctl(ghI2C[moduleidx], I2C_SLAVE, ChipAddr >> 1);    
    }

    buf[0] = (unsigned char)(RegAddr&0xff);
    buf[1] = 0x00;
    
    err = write(ghI2C[moduleidx], &buf[0], 1);
    if(err < 1)
    {
        PRINTF_LV_0("I2C Write Addr Error!!\n");
    }   
    err = read(ghI2C[moduleidx], &buf[0], 1);
    
    TC3X_IO_IF_UnLOCK (0);
    if (err<0)
    {   
        /* ERROR HANDLING: i2c transaction failed */ 
        PRINTF_LV_0("I2C_Read error");
        return TC3X_IO_FAIL;        
    }
    
    ret = (unsigned int)(buf[0]);
    return ret;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_I2C_Reg_Read_InnerSem
//    Description : Reg read (inside semaphore)
//    Input : 
//      moduleidx : module index
//      ChipAddr : chip address
//      RegAddr : Register Address
//      pError : Error flag
//    Output : 
//      TC3X_IO_SUCCESS
//    Remark : 
//--------------------------------------------------------------------------
unsigned int TC3X_IO_I2C_Reg_Read_InnerSem (int moduleidx, int ChipAddr, int RegAddr, TC3X_IO_Err * pError)
{
    int err;
    int data = 0;
    char buf[2];
    char addr = (ChipAddr&0xFF);
    unsigned int ret;
    
    if(gI2CChipAddr[moduleidx] != ChipAddr)
    {
        gI2CChipAddr[moduleidx] = ChipAddr;
        ioctl(ghI2C[moduleidx], I2C_SLAVE, ChipAddr>>1);        
    }

    buf[0] = (unsigned char)(RegAddr&0xff);
    buf[1] = 0x00;
    
    err = write(ghI2C[moduleidx], &buf[0], 1);
    if(err < 1)
    {
        PRINTF_LV_0("I2C Write Addr Error!!\n");
    }

    err = read(ghI2C[moduleidx], &buf[0], 1);
    if (err<0)
    {   
        /* ERROR HANDLING: i2c transaction failed */ 
        PRINTF_LV_0("I2C_Read error");
        return TC3X_IO_FAIL;        
    }
    
    ret = (unsigned int)(buf[0]);
    return ret;
}
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_I2C_Reg_WriteEx
//    Description : Register multi-write
//    Input : 
//      moduleidx : module index
//      ChipAddr : chip address
//      RegAddr : Register Address
//      pCh : data pointer
//      iSize : size
//    Output : 
//      TC3X_IO_SUCCESS
///    Remark : 
//--------------------------------------------------------------------------
TC3X_IO_Err TC3X_IO_I2C_Reg_WriteEx (int moduleidx, int ChipAddr, int RegAddr, unsigned char *cData, int iSize)
{
    char buf[2];
    int err, berr;
    int cMax, remain;
    int i;

    TC3X_IO_IF_LOCK (0);

    berr = 0;
    cMax = iSize/MAX_I2C_BURST;
    remain = iSize%MAX_I2C_BURST;

    if(gI2CChipAddr[moduleidx]!=ChipAddr)
    {
        gI2CChipAddr[moduleidx] = ChipAddr;
        ioctl(ghI2C[moduleidx], I2C_SLAVE, ChipAddr>>1);
    }
    
    TempBuff[0] = (unsigned char)(RegAddr)| Bit7;
    for(i=0; i<cMax; i++)
    {
        TC3X_IO_memcpy(&TempBuff[1], &cData[i*MAX_I2C_BURST], MAX_I2C_BURST);
        err = write(ghI2C[moduleidx], &TempBuff[0], MAX_I2C_BURST+1);

        if(err < 1)
        {
            berr = 1;
            PRINTF_LV_0("I2C Multi write 8 Error!!\n");
            break;
        }
    }

    if(remain && !berr)
    {
        TC3X_IO_memcpy(&TempBuff[1], &cData[cMax*MAX_I2C_BURST], remain);
        err = write(ghI2C[moduleidx], &TempBuff[0], remain+1);

        if(err < 1)
        {
            PRINTF_LV_0("I2C Multi write 8 Error!!\n");
        }
    }
    
    TC3X_IO_IF_UnLOCK (0);
    
    if(berr)
        return TC3X_IO_FAIL;
    return TC3X_IO_SUCCESS;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_I2C_Reg_WriteEx_InnerSem
//    Description : Reg write (inside semaphore)
//    Input : 
//      moduleidx : module index
//      ChipAddr : chip address
//      RegAddr : Register Address
//      pCh : data
//      iSize : size
//    Output : 
//      TC3X_IO_SUCCESS
///    Remark : 
//--------------------------------------------------------------------------
TC3X_IO_Err TC3X_IO_I2C_Reg_WriteEx_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned char *cData, int iSize)
{
    char buf[2];
    int err, berr;
    int cMax, remain;
    int i;

    berr = 0;
    cMax = iSize/MAX_I2C_BURST;
    remain = iSize%MAX_I2C_BURST;

    if(gI2CChipAddr[moduleidx]!=ChipAddr)
    {
        gI2CChipAddr[moduleidx] = ChipAddr;
        ioctl(ghI2C[moduleidx], I2C_SLAVE, ChipAddr>>1);
    }
    
    TempBuff[0] = (unsigned char)(RegAddr)| Bit7;
    for(i=0; i<cMax; i++)
    {
        TC3X_IO_memcpy(&TempBuff[1], &cData[i*MAX_I2C_BURST], MAX_I2C_BURST);
        err = write(ghI2C[moduleidx], &TempBuff[0], MAX_I2C_BURST+1);

        if(err < 1)
        {
            berr = 1;
            PRINTF_LV_0("I2C Multi write 8 Error!!\n");
            break;
        }
    }
    
    if(remain && !berr)
    {
        TC3X_IO_memcpy(&TempBuff[1], &cData[cMax*MAX_I2C_BURST], remain);
        err = write(ghI2C[moduleidx], &TempBuff[0], remain+1);

        if(err < 1)
        {
            PRINTF_LV_0("I2C Multi write 8 Error!!\n");
        }
    }

    if(berr)
        return TC3X_IO_FAIL;
    return TC3X_IO_SUCCESS;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_I2C_Reg_Write_InnerSem
//    Description : Reg multi-read
//    Input : 
//      moduleidx : module index
//      ChipAddr : chip address
//      RegAddr : Register Address
//      data : data pointer
//      iSize : size
//    Output : 
//      TC3X_IO_SUCCESS
///    Remark : 
//--------------------------------------------------------------------------
TC3X_IO_Err TC3X_IO_I2C_Reg_ReadEx (int moduleidx, int ChipAddr, int RegAddr, unsigned char *cData, int iSize)
{
    int cMax, remain;
    int berr, err;
    char buf[2];
    int i;
    
    berr = 0;
    cMax = iSize/MAX_I2C_BURST;
    remain = iSize%MAX_I2C_BURST;

    TC3X_IO_IF_LOCK (0);

    if(gI2CChipAddr[moduleidx]!=ChipAddr)
    {
        gI2CChipAddr[moduleidx] = ChipAddr;
        ioctl(ghI2C[moduleidx], I2C_SLAVE, ChipAddr>>1);
    }

    for(i=0; i<cMax; i++)
    {
        buf[0] = RegAddr | Bit7;

        err= write(ghI2C[moduleidx], &buf[0], 1);

        if(err < 1)
        {
            PRINTF_LV_0("I2C Multi read 8 Addr Error!!\n");
            berr = 1;
            break;
        }

        err= read(ghI2C[moduleidx],&cData[i*MAX_I2C_BURST], MAX_I2C_BURST);

        if (err < 0)
        {   
            /* ERROR HANDLING: i2c transaction failed */ 
            PRINTF_LV_0("I2C Multi read 8 data Error!!\n");
            berr = 1;
            break;
        }
    }

    if(remain && !berr)
    {
        buf[0] = RegAddr | Bit7;

        err= write(ghI2C[moduleidx], &buf[0], 1);

        if(err < 1)
        {
            PRINTF_LV_0("I2C Multi read 8 Addr Error!!\n");
            berr = 1;
        }
        err= read(ghI2C[moduleidx],&cData[cMax*MAX_I2C_BURST], remain);

        if (err < 0)
        {   
            /* ERROR HANDLING: i2c transaction failed */ 
            PRINTF_LV_0("I2C Multi read 8 data Error!!\n");
            berr = 1;
        }
    }

    TC3X_IO_IF_UnLOCK (0);

    if(berr == 1)
        return TC3X_IO_FAIL;
        
    return TC3X_IO_SUCCESS;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_I2C_Reg_ReadEx_InnerSem
//    Description : Reg multi-read (inside semaphore)
//    Input : 
//      moduleidx : module index
//      ChipAddr : chip address
//      RegAddr : Register Address
//      data : data pointer
//      iSize : size
//    Output : 
//      TC3X_IO_SUCCESS
///    Remark : 
//--------------------------------------------------------------------------
TC3X_IO_Err TC3X_IO_I2C_Reg_ReadEx_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned char *cData, int iSize)
{
    int cMax, remain;
    int berr, err;
    char buf[2];
    int i;

    berr = 0;
    cMax = iSize/MAX_I2C_BURST;
    remain = iSize%MAX_I2C_BURST;

    if(gI2CChipAddr[moduleidx]!=ChipAddr)
    {
        gI2CChipAddr[moduleidx] = ChipAddr;
        ioctl(ghI2C[moduleidx], I2C_SLAVE, ChipAddr>>1);
    }

    for(i=0; i<cMax; i++)
    {
        buf[0] = RegAddr | Bit7;

        err= write(ghI2C[moduleidx], &buf[0], 1);

        if(err < 1)
        {
            PRINTF_LV_0("I2C Multi read 8 Addr Error!!\n");
            berr = 1;
            break;
        }

        err= read(ghI2C[moduleidx],&cData[i*MAX_I2C_BURST], MAX_I2C_BURST);

        if (err < 0)
        {   
            /* ERROR HANDLING: i2c transaction failed */ 
            PRINTF_LV_0("I2C Multi read 8 data Error!!\n");
            berr = 1;
            break;
        }
    }

    if(remain && !berr)
    {
        buf[0] = RegAddr | Bit7;
        err= write(ghI2C[moduleidx], &buf[0], 1);

        if(err < 1)
        {
            PRINTF_LV_0("I2C Multi read 8 Addr Error!!\n");
            berr = 1;
        }

        err= read(ghI2C[moduleidx],&cData[cMax*MAX_I2C_BURST], remain);

        if (err < 0)
        {   
            /* ERROR HANDLING: i2c transaction failed */ 
            PRINTF_LV_0("I2C Multi read 8 data Error!!\n");
            berr = 1;
        }
    }

    if(berr==1)
        return TC3X_IO_FAIL;
        
    return TC3X_IO_SUCCESS;
}

void TC3X_IO_I2C_Reg_Read_Wrap(int iHandleNum, unsigned int Addr, unsigned char *data, unsigned char bytesize)
{
    int i;
    
		if(iHandleNum==1 && gI2CHanleInit1==0)
			return;

    for(i=0; i<bytesize; i++)
    { 
        data[i] = (unsigned char)(TC3X_IO_I2C_Reg_Read (iHandleNum, gI2CChipAddr[iHandleNum], Addr+i, NULL));
    }
}

void TC3X_IO_I2C_Reg_Write_Wrap(int iHandleNum, unsigned int Addr, unsigned char data)
{
		if(iHandleNum==1 && gI2CHanleInit1==0)
			return;

    TC3X_IO_I2C_Reg_Write (iHandleNum, gI2CChipAddr[iHandleNum], Addr, data);
}
#endif // USE_IF_I2C
