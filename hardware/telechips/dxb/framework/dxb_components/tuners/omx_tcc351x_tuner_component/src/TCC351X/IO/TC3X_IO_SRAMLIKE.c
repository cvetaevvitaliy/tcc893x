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

#include "TC3X_IO_SRAMLIKE.h"
#if defined (USE_IF_SRAMLIKE)
#include "TC3X_IO.h"
#ifdef TCC78X
#include "TCC78X.h"
#else
#include "TCC79X.h"
#endif

#define	Bit31		0x80000000
#define	Bit30		0x40000000
#define	Bit29		0x20000000
#define	Bit28		0x10000000
#define	Bit27		0x08000000
#define	Bit26		0x04000000
#define	Bit25		0x02000000
#define	Bit24		0x01000000
#define	Bit23		0x00800000
#define	Bit22		0x00400000
#define	Bit21		0x00200000
#define	Bit20		0x00100000
#define	Bit19		0x00080000
#define	Bit18		0x00040000
#define	Bit17		0x00020000
#define	Bit16		0x00010000
#define	Bit15		0x00008000
#define	Bit14		0x00004000
#define	Bit13		0x00002000
#define	Bit12		0x00001000
#define	Bit11		0x00000800
#define	Bit10		0x00000400
#define	Bit9		0x00000200
#define	Bit8		0x00000100
#define	Bit7		0x00000080
#define	Bit6		0x00000040
#define	Bit5		0x00000020
#define	Bit4		0x00000010
#define	Bit3		0x00000008
#define	Bit2		0x00000004
#define	Bit1		0x00000002
#define	Bit0		0x00000001
#define	BitNONE 	0x00000000

// 현재 Bus Clock이 120M 이며, Core는 240M
// 120M = 8.3ns
// 8.3*7(WRITE_PW+1)=58.1 ns (write)
// 8.3*5(READ_PW+1)=41.5 ns (read)
// 8.3*(STP+HLD) = 33.2ns

// TC3X Spec이 low period 52.08ns (Write)                       17.211M
// TC3X Spec이 low period 17.36*2 = 34.72ns (Read)      28.801M
// TC3X Spec이 high period 17.36ns                                      57.603M

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_SRAMLIKE_Init
//    Description : Initialize SRAMLIKE
//    Input : 
//      moduleidx : module index
//    Output : 
//      TC3X_IO_SUCCESS
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_SRAMLIKE_Init (int moduleidx)
{
    unsigned int BusClkMHz = 120;

    unsigned int CSBASE = 4;
    unsigned int OD = 0;
    unsigned int WD = 0;
    unsigned int SIZE = 3;
    unsigned int TYPE = 2;
    unsigned int URDY = 0;
    unsigned int RDY = 0;
    unsigned int AM = 1;
    unsigned int PSIZE = 1;
    unsigned int LA = 3;

    unsigned int STP;
    unsigned int HLD;

    unsigned int STP_HLD;
    unsigned int READ_PW;
    unsigned int WRITE_PW;

    float     highperiod_ns = 17.36;

    float     bus_ns;
    float     need_write_ns;
    float     need_read_ns;

    bus_ns = 1000.0 / BusClkMHz;
    need_write_ns = highperiod_ns * 3;
    need_read_ns = highperiod_ns * 2;

    WRITE_PW = ((need_write_ns + bus_ns) / bus_ns) - 1;
    READ_PW = ((need_read_ns + bus_ns) / bus_ns) - 1;
    STP_HLD = (((highperiod_ns + bus_ns) / bus_ns) + 1) / 2;
    STP = STP_HLD;
    HLD = STP_HLD;

    //External memory controller
    HwCSCFG0 = (CSBASE << 28) | (OD << 27) | (WD << 26) | (SIZE << 24) | (TYPE << 22) | (URDY << 21) | (RDY << 20) |
        (AM << 19) | (PSIZE << 17) | (LA << 14) | (STP << 11) | (WRITE_PW << 3) | HLD;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_SRAMLIKE_Close
//    Description : Close SRAMLIKE
//    Input : 
//      moduleidx : module index
//    Output : 
//      TC3X_IO_SUCCESS
//    Remark : 
//--------------------------------------------------------------------------
TC3X_IO_Err TC3X_IO_SRAMLIKE_Close(int moduleidx)
{
#if defined TCC79X
#endif
    return TC3X_IO_SUCCESS;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_SRAMLIKE_Reg_Write
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
TC3X_IO_Err TC3X_IO_SRAMLIKE_Reg_Write (int moduleidx, int ChipAddr, int RegAddr, unsigned int data)
{
    int       semret;
    semret = TC3X_IO_IF_LOCK (moduleidx);

    TC3X_HwEBIADDR = (unsigned short) ((ChipAddr << 8 | RegAddr) & 0xffff);
    TC3X_HwEBIDATA = (unsigned char) (data & 0xff);

    TC3X_IO_IF_UnLOCK (moduleidx);
    return TC3X_IO_SUCCESS;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_SRAMLIKE_Reg_WriteEx
//    Description : Register multi-write
//    Input : 
//      moduleidx : module index
//      ChipAddr : chip address
//      RegAddr : Register Address
//      pCh : data pointer
//      iSize : size
//    Output : 
//      TC3X_IO_SUCCESS
//    Remark : 
//--------------------------------------------------------------------------
TC3X_IO_Err TC3X_IO_SRAMLIKE_Reg_WriteEx (int moduleidx, int ChipAddr, int RegAddr, unsigned char *pCh, int iSize)
{
    int       semret;
    int       i;

    semret = TC3X_IO_IF_LOCK (moduleidx);

    TC3X_HwEBIADDR = (ChipAddr << 8 | RegAddr) | Bit7;

    if (iSize <= 0)
    {
        TC3X_IO_IF_UnLOCK (moduleidx);
        return TC3X_IO_FAIL;
    }

    for (i = 0; i < iSize; i++)
    {
        TC3X_HwEBIDATA = pCh[i];
    }

    TC3X_IO_IF_UnLOCK (moduleidx);
    return TC3X_IO_SUCCESS;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_SRAMLIKE_Reg_Read
//    Description : Reg read
//    Input : 
//      moduleidx : module index
//      ChipAddr : chip address
//      RegAddr : Register Address
//      pError : Error flag
//    Output : 
//      TC3X_IO_SUCCESS
//    Remark : 
//--------------------------------------------------------------------------
unsigned int TC3X_IO_SRAMLIKE_Reg_Read (int moduleidx, int ChipAddr, int RegAddr, TC3X_IO_Err * pError)
{
    unsigned int data;
    int       semret;
    semret = TC3X_IO_IF_LOCK (moduleidx);

    TC3X_HwEBIADDR = (unsigned short) ((ChipAddr << 8 | RegAddr) & 0xffff);
    data = TC3X_HwEBIDATA;
    TC3X_IO_IF_UnLOCK (moduleidx);
    return data;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_SRAMLIKE_Reg_ReadEx
//    Description : Reg multi-read
//    Input : 
//      moduleidx : module index
//      ChipAddr : chip address
//      RegAddr : Register Address
//      data : data pointer
//      iSize : size
//    Output : 
//      TC3X_IO_SUCCESS
//    Remark : 
//--------------------------------------------------------------------------
TC3X_IO_Err TC3X_IO_SRAMLIKE_Reg_ReadEx (int moduleidx, int ChipAddr, int RegAddr, unsigned char *data, int iSize)
{
    unsigned char *tmp;
    int       semret;
    semret = TC3X_IO_IF_LOCK (moduleidx);

    TC3X_HwEBIADDR = (ChipAddr << 8 | RegAddr) | Bit7;

    tmp = (unsigned char *) data;

    while (iSize)
    {
        *tmp++ = TC3X_HwEBIDATA;
        iSize--;
    }

    TC3X_IO_IF_UnLOCK (moduleidx);
    return TC3X_IO_SUCCESS;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_SRAMLIKE_Reg_Write_InnerSem
//    Description : Reg write (inside semaphore)
//    Input : 
//      moduleidx : module index
//      ChipAddr : chip address
//      RegAddr : Register Address
//      pCh : data
//      iSize : size
//    Output : 
//      TC3X_IO_SUCCESS
//    Remark : 
//--------------------------------------------------------------------------
TC3X_IO_Err TC3X_IO_SRAMLIKE_Reg_Write_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned int data)
{
    TC3X_HwEBIADDR = (ChipAddr << 8 | RegAddr);
    TC3X_HwEBIDATA = (unsigned char) (data & 0xff);

    return TC3X_IO_SUCCESS;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_SRAMLIKE_Reg_WriteEx_InnerSem
//    Description : Reg write (inside semaphore)
//    Input : 
//      moduleidx : module index
//      ChipAddr : chip address
//      RegAddr : Register Address
//      pCh : data
//      iSize : size
//    Output : 
//      TC3X_IO_SUCCESS
//    Remark : 
//--------------------------------------------------------------------------
TC3X_IO_Err TC3X_IO_SRAMLIKE_Reg_WriteEx_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned char *pCh, int iSize)
{
    int       i;

    TC3X_HwEBIADDR = (ChipAddr << 8 | RegAddr) | Bit7;

    if (iSize <= 0)
    {
        return TC3X_IO_FAIL;
    }

    for (i = 0; i < iSize; i++)
    {
        TC3X_HwEBIDATA = pCh[i];
    }

    return TC3X_IO_SUCCESS;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_SRAMLIKE_Reg_Read_InnerSem
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
unsigned int TC3X_IO_SRAMLIKE_Reg_Read_InnerSem (int moduleidx, int ChipAddr, int RegAddr, TC3X_IO_Err * pError)
{
    unsigned int data;

    TC3X_HwEBIADDR = (ChipAddr << 8 | RegAddr);
    data = TC3X_HwEBIDATA;
    return data;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_SRAMLIKE_Reg_ReadEx_InnerSem
//    Description : Reg multi-read (inside semaphore)
//    Input : 
//      moduleidx : module index
//      ChipAddr : chip address
//      RegAddr : Register Address
//      data : data pointer
//      iSize : size
//    Output : 
//      TC3X_IO_SUCCESS
//    Remark : 
//--------------------------------------------------------------------------
TC3X_IO_Err TC3X_IO_SRAMLIKE_Reg_ReadEx_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned char *data, int iSize)
{
    unsigned char *tmp;

    TC3X_HwEBIADDR = (ChipAddr << 8 | RegAddr) | Bit7;

    tmp = (unsigned char *) data;

    while (iSize)
    {
        *tmp++ = TC3X_HwEBIDATA;
        iSize--;
    }

    return TC3X_IO_SUCCESS;
}
#endif // USE_IF_SRAMLIKE
