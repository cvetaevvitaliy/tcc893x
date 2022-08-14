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

#include "TC3X_IO_HPI.h"
#if defined (USE_IF_HPI)
#include "TC3X_IO.h"

#include "TC3X_IO_SRAMLIKE.h"

#ifdef TCC78X
#include "TCC78x.h"
#else
#include "TCC79x.h"
#endif

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_HPI_Init
//    Description : Initialize HPI
//    Input : 
//      moduleidx : module index
//    Output : 
//      
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_HPI_Init (int moduleidx)
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

    if (moduleidx != 0)
    {
        PRINTF_LV_0 ("[ERROR] TC3X_IO_HPI_Init - Please insert code\n");
        return;
    }

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
//        TC3X_IO_HPI_ReadStream
//    Description : Read stream
//    Input : 
//      moduleidx : module index
//      data : data pointer
//      iSize : size
//    Output : 
//      
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_HPI_ReadStream (int moduleidx, unsigned char *data, int iSize)
{
    unsigned char *tmp;

    if (moduleidx != 0)
    {
        PRINTF_LV_0 ("[ERROR] TC3X_IO_HPI_ReadStream - Please insert code\n");
        return;
    }

    tmp = (unsigned char *) data;
    while (iSize)
    {
        *tmp++ = TC3X_HwEBIDATA;
        *tmp++ = TC3X_HwEBIDATA;
        *tmp++ = TC3X_HwEBIDATA;
        *tmp++ = TC3X_HwEBIDATA;
        iSize -= 4;
    }
}
#endif // USE_IF_HPI
