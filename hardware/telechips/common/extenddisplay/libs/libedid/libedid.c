/****************************************************************************
Copyright (C) 2013 Telechips Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
****************************************************************************/
 
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "edid.h"
#include "libedid.h"
#include "libddc/libddc.h"

#define EDID_DEBUG      1
#define LOG_TAG "LIBEDID"
#include <utils/Log.h>

#if EDID_DEBUG
#define DPRINTF(args...)    ALOGD(args)
#else
#define DPRINTF(args...)
#endif

#if 0
#define FPRINTF(args...)    ALOGI(args)
#else
#define FPRINTF(args...)
#endif

#define MSGPRT(cond,printf_exp) ( (cond)?(ALOGD printf_exp):(EDID_DEBUG?(ALOGD printf_exp):0))
/**
 * @var gEdidData
 * Pointer to EDID data
 */
static unsigned char* gEdidData=NULL;

/**
 * @var gExtensions
 * Number of EDID extensions
 */
static int gExtensions;

#define EDID_CHK_LENGTH		32

//! Structure for parsing video timing parameter in EDID
static const struct edid_params
{
#if (1) || defined(TELECHIPS)
	unsigned int supported;
#endif
    /** H Blank */
    unsigned int vHBlank;
    /** V Blank */
    unsigned int vVBlank;
    /**
     * H Total + V Total @n
     * For more information, refer HDMI register map
     */
    unsigned int vHVLine;
    /** CEA VIC */
    unsigned char  VIC;
    /** CEA VIC for 16:9 aspect ratio */
    unsigned char  VIC16_9;
    /** 0 if progressive, 1 if interlaced */
    unsigned char  interlaced;
    /** Pixel frequency */
    enum PixelFreq PixelClock;
} aVideoParams[] =
{
    { 1, 0xA0 , 0x16A0D, 0x32020D,  1 , 1 , 0, PIXEL_FREQ_25_200,  },  // v640x480p_60Hz
    { 1, 0x8A , 0x16A0D, 0x35A20D,  2 , 3 , 0, PIXEL_FREQ_27_027,  },  // v720x480p_60Hz
	#if defined(HDMI_V1_4) //MVC_PROCESS
	{ 1, 0x172, 0xF2EE , 0x6722EE,  4 , 4 , 0, PIXEL_FREQ_74_250,  },	    // v1280x720p_60Hz_3D
	#endif
    { 1, 0x172, 0xF2EE , 0x6722EE,  4 , 4 , 0, PIXEL_FREQ_74_250,  },  // v1280x720p_60Hz
    { 1, 0x118, 0xB232 , 0x898465,  5 , 5 , 1, PIXEL_FREQ_74_250,  },  // v1920x1080i_60Hz
    { 1, 0x114, 0xB106 , 0x6B420D,  6 , 7 , 1, PIXEL_FREQ_27_027,  },  // v720x480i_60Hz
    { 0, 0x114, 0xB106 , 0x6B4106,  8 , 9 , 0, PIXEL_FREQ_27_027,  },  // v720x240p_60Hz
    { 0, 0x228, 0xB106 , 0xD6820D,  10, 11, 1, PIXEL_FREQ_54_054,  },  // v2880x480i_60Hz
    { 0, 0x228, 0xB106 , 0x6B4106,  12, 13, 0, PIXEL_FREQ_54_054,  },  // v2880x240p_60Hz
    { 0, 0x114, 0x16A0D, 0x6B420D,  14, 15, 0, PIXEL_FREQ_54_054,  },  // v1440x480p_60Hz
    { 1, 0x118, 0x16C65, 0x898465,  16, 16, 0, PIXEL_FREQ_148_500, },  // v1920x1080p_60Hz
    { 1, 0x90 , 0x18A71, 0x360271,  17, 18, 0, PIXEL_FREQ_27,      },  // v720x576p_50Hz
    { 1, 0x2BC, 0xF2EE , 0x7BC2EE,  19, 19, 0, PIXEL_FREQ_74_250,  },  // v1280x720p_50Hz
    { 1, 0x2D0, 0xB232 , 0xA50465,  20, 20, 1, PIXEL_FREQ_74_250,  },  // v1920x1080i_50Hz
    { 1, 0x120, 0xC138 , 0x6C0271,  21, 22, 1, PIXEL_FREQ_27,      },  // v720x576i_50Hz
    { 0, 0x120, 0xC138 , 0x6C0138,  23, 24, 0, PIXEL_FREQ_27,      },  // v720x288p_50Hz
    { 0, 0x240, 0xC138 , 0xD80271,  25, 26, 1, PIXEL_FREQ_54,      },  // v2880x576i_50Hz
    { 0, 0x240, 0xC138 , 0xD80138,  27, 28, 0, PIXEL_FREQ_54,      },  // v2880x288p_50Hz
    { 0, 0x120, 0x18A71, 0x6C0271,  29, 30, 0, PIXEL_FREQ_54,      },  // v1440x576p_50Hz
    { 1, 0x2D0, 0x16C65, 0xA50465,  31, 31, 0, PIXEL_FREQ_148_500, },  // v1920x1080p_50Hz
    #if defined(HDMI_V1_4) //MVC_PROCESS
    { 0, 0x33E, 0x16C65, 0xABE465,  32, 32, 0, PIXEL_FREQ_74_250,  },  // v1920x1080p_24Hz_3D
    #endif
    { 0, 0x33E, 0x16C65, 0xABE465,  32, 32, 0, PIXEL_FREQ_74_250,  },  // v1920x1080p_24Hz
    { 0, 0x2D0, 0x16C65, 0xA50465,  33, 33, 0, PIXEL_FREQ_74_250,  },  // v1920x1080p_25Hz
    { 0, 0x2D0, 0x16C65, 0xA50465,  34, 34, 0, PIXEL_FREQ_74_250,  },  // v1920x1080p_30Hz
    { 0, 0x228, 0x16A0D, 0xD6820D,  35, 36, 0, PIXEL_FREQ_108_108, },  // v2880x480p_60Hz
    { 0, 0x240, 0x18A71, 0xD80271,  37, 38, 0, PIXEL_FREQ_108,     },  // v2880x576p_50Hz
    { 1, 0x180, 0x2AA71, 0x9004E2,  39, 39, 0, PIXEL_FREQ_72,      },  // v1920x1080i_50Hz(1250)
    { 0, 0x2D0, 0xB232 , 0xA50465,  40, 40, 1, PIXEL_FREQ_148_500, },  // v1920x1080i_100Hz
    { 0, 0x2BC, 0xF2EE , 0x7BC2EE,  41, 41, 0, PIXEL_FREQ_148_500, },  // v1280x720p_100Hz
    { 0, 0x90 , 0x18A71, 0x360271,  42, 43, 0, PIXEL_FREQ_54,      },  // v720x576p_100Hz
    { 0, 0x120, 0xC138 , 0x6C0271,  44, 45, 1, PIXEL_FREQ_54,      },  // v720x576i_100Hz
    { 0, 0x118, 0xB232 , 0x898465,  46, 46, 1, PIXEL_FREQ_148_500, },  // v1920x1080i_120Hz
    { 0, 0x172, 0xF2EE , 0x6722EE,  47, 47, 0, PIXEL_FREQ_148_500, },  // v1280x720p_120Hz
    { 0, 0x8A , 0x16A0D, 0x35A20D,  48, 49, 0, PIXEL_FREQ_54_054,  },  // v720x480p_120Hz
    { 0, 0x114, 0xB106 , 0x6B420D,  50, 51, 1, PIXEL_FREQ_54_054,  },  // v720x480i_120Hz
    { 0, 0x90 , 0x18A71, 0x360271,  52, 53, 0, PIXEL_FREQ_108,     },  // v720x576p_200Hz
    { 0, 0x120, 0xC138 , 0x6C0271,  54, 55, 1, PIXEL_FREQ_108,     },  // v720x576i_200Hz
    { 0, 0x8A , 0x16A0D, 0x35A20D,  56, 57, 0, PIXEL_FREQ_108_108, },  // v720x480p_240Hz
    { 0, 0x114, 0xB106 , 0x6B420D,  58, 59, 1, PIXEL_FREQ_108_108, },  // v720x480i_240Hz
};

#define NONDTD 0x0
#define DBCCOLLECTION 0x1
#define DTDCOLLECTION 0x2
unsigned char* pStandardResTiming[] = {
    (unsigned char*)(""),
    (unsigned char*)("01 DMT0659   4:3                640x480p @  60Hz"),
    (unsigned char*)("02 480p      4:3                720x480p @  60Hz"),
    (unsigned char*)("03 480pH    16:9                720x480p @  60Hz"),
    #if defined(HDMI_V1_4) //MVC_PROCESS
    (unsigned char*)("04 720p     16:9               1280x720p @  60Hz"),
    #endif
    (unsigned char*)("04 720p     16:9               1280x720p @  60Hz"),
    (unsigned char*)("05 1080i    16:9              1920x1080i @  60Hz"),
    (unsigned char*)("06 480i      4:3          720(1440)x480i @  60Hz"),
    (unsigned char*)("07 480iH    16:9          720(1440)x480i @  60Hz"),
    (unsigned char*)("08 240p      4:3          720(1440)x240p @  60Hz"),
    (unsigned char*)("09 240pH    16:9          720(1440)x240p @  60Hz"),
    (unsigned char*)("10 480i4x    4:3             (2880)x480i @  60Hz"),
    (unsigned char*)("11 480i4xH  16:9             (2880)x480i @  60Hz"),
    (unsigned char*)("12 240p4x    4:3             (2880)x240p @  60Hz"),
    (unsigned char*)("13 240p4xH  16:9             (2880)x240p @  60Hz"),
    (unsigned char*)("14 480p2x    4:3               1440x480p @  60Hz"),
    (unsigned char*)("15 480p2xH  16:9               1440x480p @  60Hz"),
    (unsigned char*)("16 1080p    16:9              1920x1080p @  60Hz"),
    (unsigned char*)("17 576p      4:3                720x576p @  50Hz"),
    (unsigned char*)("18 576pH    16:9                720x576p @  50Hz"),
    (unsigned char*)("19 720p50   16:9               1280x720p @  50Hz"),
    (unsigned char*)("20 1080i25  16:9              1920x1080i @  50Hz"),
    (unsigned char*)("21 576i      4:3          720(1440)x576i @  50Hz"),
    (unsigned char*)("22 576iH    16:9          720(1440)x576i @  50Hz"),
    (unsigned char*)("23 288p      4:3          720(1440)x288p @  50Hz"),
    (unsigned char*)("24 288pH    16:9          720(1440)x288p @  50Hz"),
    (unsigned char*)("25 576i4x    4:3             (2880)x576i @  50Hz"),
    (unsigned char*)("26 576i4xH  16:9             (2880)x576i @  50Hz"),
    (unsigned char*)("27 288p4x    4:3             (2880)x288p @  50Hz"),
    (unsigned char*)("28 288p4xH  16:9             (2880)x288p @  50Hz"),
    (unsigned char*)("29 576p2x    4:3               1440x576p @  50Hz"),
    (unsigned char*)("30 576p2xH  16:9               1440x576p @  50Hz"),
    (unsigned char*)("31 1080p50  16:9              1920x1080p @  50Hz"),
    #if defined(HDMI_V1_4) //MVC_PROCESS
    (unsigned char*)("32 1080p24  16:9              1920x1080p @  24Hz"),
    #endif
    (unsigned char*)("32 1080p24  16:9              1920x1080p @  24Hz"),
    (unsigned char*)("33 1080p25  16:9              1920x1080p @  25Hz"),
    (unsigned char*)("34 1080p30  16:9              1920x1080p @  30Hz"),
    (unsigned char*)("35 480p4x    4:3             (2880)x480p @  60Hz"),
    (unsigned char*)("36 480p4xH  16:9             (2880)x480p @  60Hz"),
    (unsigned char*)("37 576p4x    4:3             (2880)x576p @  50Hz"),
    (unsigned char*)("38 576p4xH  16:9             (2880)x576p @  50Hz"),
    (unsigned char*)("39 108Oi25  16:9  1920x1080i(1250 Total) @  50Hz"),
    (unsigned char*)("40 1080i50  16:9              1920x1080i @ 100Hz"),
    (unsigned char*)("41 720p100  16:9               1280x720p @ 100Hz"),
    (unsigned char*)("42 576p100   4:3                720x576p @ 100Hz"),
    (unsigned char*)("43 576p100H 16:9                720x576p @ 100Hz"),
    (unsigned char*)("44 576i50    4:3          720(1440)x576i @ 100Hz"),
    (unsigned char*)("45 576i50H  16:9          720(1440)x576i @ 100Hz"),
    (unsigned char*)("46 1080i60  16:9              1920x1080i @ 120Hz"),
    (unsigned char*)("47 720p120  16:9               1280x720p @ 120Hz"),
    (unsigned char*)("48 480p119   4:3                720x480p @ 120Hz"),
    (unsigned char*)("49 480p119H 16:9                720x480p @ 120Hz"),
    (unsigned char*)("50 480i59    4:3          720(1440)x480i @ 120Hz"),
    (unsigned char*)("51 480i59H  16:9          720(1440)x480i @ 120Hz"),
    (unsigned char*)("52 576p200   4:3                720x576p @ 200Hz"),
    (unsigned char*)("53 576p200H 16:9                720x576p @ 200Hz"),
    (unsigned char*)("54 576i100   4:3          720(1440)x576i @ 200Hz"),
    (unsigned char*)("55 576i100H 16:9          720(1440)x576i @ 200Hz"),
    (unsigned char*)("56 480p239   4:3                720x480p @ 240Hz"),
    (unsigned char*)("57 480p239H 16:9                720x480p @ 240Hz"),
    (unsigned char*)("58 480i119   4:3          720(1440)x480i @ 240Hz"),
    (unsigned char*)("59 480i119H 16:9          720(1440)x480i @ 240Hz"),
    (unsigned char*)("60 720p24   16:9               1280x720p @  24Hz"),
    (unsigned char*)("61 720p25   16:9               1280x720p @  25Hz"),
    (unsigned char*)("62 720p30   16:9               1280x720p @  30Hz"),
    (unsigned char*)("63 1080p120 16:9               1920x1080 @ 120Hz")
};

unsigned char* pAudioFormat[] = {
    (unsigned char*)"0 = Reserved", 
    (unsigned char*)"1 = Linear Pulse Code Modulation (LPCM)",
    (unsigned char*)"2 = AC-3",
    (unsigned char*)"3 = MPEG1 (Layers 1 and 2)",
    (unsigned char*)"4 = MP3",
    (unsigned char*)"5 = MPEG2",
    (unsigned char*)"6 = AAC",
    (unsigned char*)"7 = DTS",
    (unsigned char*)"8 = ATRAC",
    (unsigned char*)"9 = One-bit audio aka SACD",
    (unsigned char*)"10 = DD+",
    (unsigned char*)"11 = DTS-HD",
    (unsigned char*)"12 = MLP/Dolby TrueHD",
    (unsigned char*)"13 = DST Audio",
    (unsigned char*)"14 = Microsoft WMA Pro",
    (unsigned char*)"15 = Reserved"
};

unsigned char* pADChannel[] = {
    (unsigned char*)"1 Channel",
    (unsigned char*)"2 Channel",
    (unsigned char*)"3 Channel",
    (unsigned char*)"4 Channel",
    (unsigned char*)"5 Channel",
    (unsigned char*)"6 Channel",
    (unsigned char*)"7 Channel",
    (unsigned char*)"8 Channel"
};
    
unsigned char* pSampleFreq[] = {
    (unsigned char*)"32kHz",
    (unsigned char*)"44kHz",
    (unsigned char*)"48kHz",    
    (unsigned char*)"88kHz",
    (unsigned char*)"96kHz",
    (unsigned char*)"176kHz",
    (unsigned char*)"192kHz"
};

unsigned char* pBitrate[] = {
	(unsigned char*)"16Bit",
	(unsigned char*)"20Bit",
	(unsigned char*)"24Bit"
};

unsigned char* pEstablishedTiming[] = {
    (unsigned char*)"800x600@60Hz",
    (unsigned char*)"800x600@56Hz",
    (unsigned char*)"640x480@75Hz",
    (unsigned char*)"640x480@72Hz",
    (unsigned char*)"640x480@67Hz",
    (unsigned char*)"640x480@60Hz",
    (unsigned char*)"720x400@88Hz",
    (unsigned char*)"720x400@70Hz",
    (unsigned char*)"1280x1024@75Hz",
    (unsigned char*)"1024x678@75Hz",
    (unsigned char*)"1024x768@70Hz",
    (unsigned char*)"1024x768@60Hz",
    (unsigned char*)"1024x768@87Hz(I)",
    (unsigned char*)"832x624@75Hz",
    (unsigned char*)"800x600@75Hz",
    (unsigned char*)"800x600@72Hz"
};


unsigned char* pBlockType[]={
    (unsigned char*)"Monitor Serial Number",
    (unsigned char*)"ASCII String",
    (unsigned char*)"Monitor Range Limits",
    (unsigned char*)"Monitor Name",
    (unsigned char*)"Color Point Data",
    (unsigned char*)"Standard Timing Data",
    (unsigned char*)"Currently Undefined",
    (unsigned char*)"Defined by Manufacturer"
};
#if !defined(_LINUX_)
#pragma pack(1)
#endif
typedef union _DTV{
    unsigned char DTVInfo;
    struct
    {
        unsigned char NativeFormatNum:4;
        unsigned char YCbCr422:1;
        unsigned char YCbCr444:1;
        unsigned char basicaudio:1;
        unsigned char underscan:1;
    }B;
#if defined(_LINUX_)
}__attribute__ ((packed)) stDTV;
#else
}stDTV;
#endif

typedef union _DBC{
    unsigned char DataBlockCollection;
    struct
    {
        unsigned char ByteNumber:5;
        unsigned char BlockType:3;
    }B;

#if defined(_LINUX_)
}__attribute__ ((packed)) stDBC;
#else
}stDBC;
#endif

typedef struct _stEEDID{
    unsigned char ExtensionTag;
    unsigned char RevisionNumber;
    unsigned char DTDs;
    stDTV   DTVInfo;
    //stDBC   BlockHead;

#if defined(_LINUX_)
}__attribute__ ((packed)) stEEDID;
#else
}stEEDID;
#endif

typedef union _SVD{
    unsigned char svd;
    struct{
        unsigned char index:7;
        unsigned char native:1;
    }B;

#if defined(_LINUX_)
}__attribute__ ((packed)) SVD;
#else
}SVD;
#endif

typedef struct _stVDB{
    stDBC   BlockHead;
    //unsigned char NumOfDscrpt;
    SVD ShortVideoDscrpt[18];

#if defined(_LINUX_)
}__attribute__ ((packed)) stVDB;
#else
}stVDB;
#endif

typedef union _SAD{
    struct{
        unsigned char channelnum:3;
        unsigned char audioformat:4;
        unsigned char reserved:1;
    }sadb1;

    struct{
        unsigned char samplefreq:7;
        unsigned char reserved:1;
    }sadb2;

    struct{
        unsigned char bitrate:3;
        unsigned char reserved:5;
    }sadb3;
    unsigned char sadb3a;

#if defined(_LINUX_)
}__attribute__ ((packed)) SAD;
#else
}SAD;
#endif

typedef struct _stADB{
    stDBC BlockHead;
    SAD SADB[3];

#if defined(_LINUX_)
}__attribute__ ((packed)) stADB;
#else
}stADB;
#endif

typedef union _CRSINK{
    unsigned char crsink;
    struct {
        unsigned char DVIDual:1;
        unsigned char reserved:2;
        unsigned char ycbcr444:1;
        unsigned char dc30bpp:1;
        unsigned char dc36bpp:1;
        unsigned char dc48bpp:1;
        unsigned char AIsupport:1;
    }B;
    
#if defined(_LINUX_)
}__attribute__ ((packed)) CRSINK;
#else
}CRSINK;
#endif

typedef union _LATENCY{
    unsigned char Latency;
    struct{
        unsigned char reserved:6;
        unsigned char ilatencyfield:1;
        unsigned char latencyfield:1;
    }B;
        
#if defined(_LINUX_)
}__attribute__ ((packed)) LATENCY;
#else
}LATENCY;
#endif

typedef union _PHYADR{
    unsigned char PhysicalAddr;
    struct{
        unsigned char high:4;
        unsigned char low:4;
    }B;
    
#if defined(_LINUX_)
}__attribute__ ((packed)) PHYADR;
#else
}PHYADR;
#endif

typedef union _STRUCT_3D{
    unsigned char Struct_3D;
    struct{
		unsigned char reserved:6;
        unsigned char Image_Size:2;
        unsigned char multi_present_3d:2;
        unsigned char present_3d:1;
    }B;
        
#if defined(_LINUX_)
}__attribute__ ((packed)) STRUCT_3D;
#else
}STRUCT_3D;
#endif

typedef union _VIC_3D_LEN{
    unsigned char VIC_3D_len;
    struct{
        unsigned char hdmi_3d_len:6;
        unsigned char hdmi_vic_len:6;
    }B;
        
#if defined(_LINUX_)
}__attribute__ ((packed)) VIC_3D_LEN;
#else
}VIC_3D_LEN;
#endif

typedef struct _stVSDB{
    stDBC BlockHead;
    unsigned char ieeeregid[3];
    PHYADR cpntphysaddr[2];
    CRSINK colorsink;
    unsigned char tmdsfreq;
    LATENCY latency;
    unsigned char videolatency;
    unsigned char audiolatency;
    unsigned char interlacedvideolatency;
    unsigned char interlacedaudiolatency;
	STRUCT_3D struct_3d;
	VIC_3D_LEN vic_3d_len;

#if defined(_LINUX_)
}__attribute__ ((packed)) stVSDB;
#else
}stVSDB;
#endif

typedef union _SPKLOC{
    unsigned char spkloacation;
    struct{
        unsigned char frntLR:1;
        unsigned char LFE:1;
        unsigned char frntCtr:1;
        unsigned char rearLR:1;
        unsigned char rearCtr:1;
        unsigned char frntLCRC:1;
        unsigned char rearLCRC:1;
        unsigned char reserved:1;
    }B; 

#if defined(_LINUX_)
}__attribute__ ((packed)) SPKLOC;
#else
}SPKLOC;
#endif

typedef struct _stSPK{
    stDBC BlockHead;
    SPKLOC  SpkLocation;
    unsigned char Byte2nd;
    unsigned char Byte3rd;

#if defined(_LINUX_)
}__attribute__ ((packed)) stSPK;
#else
}stSPK;
#endif

typedef struct _stCEABlock{
    stEEDID CEAInfo;
    stVDB   VDB;
    stADB   ADB;
    stVSDB  VSDB;
    stSPK   SPK;
    unsigned char NumofDTDs;

#if defined(_LINUX_)
}__attribute__ ((packed)) stCEABlock;
#else
}stCEABlock;
#endif




typedef struct _stPDBlock{
    unsigned short  pixelclock;// 10Khz unit
    unsigned char   HActive; // in pixel
    unsigned char   HBlanking; // in pixel
    union{
        unsigned char HABhigh;
        struct{
            unsigned char HBlankinghigh:4;
            unsigned char HActivehigh:4;
        }B;
    }HHigh;
    unsigned char   VActive;
    unsigned char   VBlanking;

    union{
        unsigned char VABhigh;
        struct{
            unsigned char VBlankinghigh:4;
            unsigned char VActivehigh:4;
        }B;
    }VHigh;

    unsigned char   HSyncOffset;        //in pixel
    unsigned char   HSyncPulsewidth;   //in pixel

    union{
        unsigned char VerticalSync;
        struct{
            unsigned char VSyncPulsewidth:4;
            unsigned char VSyncOffset:4;
        }B;
    }VSync;

    union{
        unsigned char MSBit;
        struct{
            unsigned char VSyncPulsewidthHSB:2;
            unsigned char VSyncOffsetHSB:2;
            unsigned char HSyncPulsewidthHSB:2;
            unsigned char HSyncOffsetHSB:2;
        }B;
    }HSB;

    unsigned char   Himagesize; //in mm
    unsigned char   Vimagesize; //in mm

    union{
        unsigned char Imgszhsb;
        struct{
            unsigned char VImgszHSB:4;
            unsigned char HImgszHSB:4;
        }B;
    }HSBIMGSize;

    unsigned char   HBoarder;
    unsigned char   VBoarder;

    union{
        unsigned char   modebit;
        struct{
            unsigned char Stereomode:1;
            unsigned char HSyncpositive:1;
            unsigned char VSyncpositive:1;
            unsigned char SeperateSync:2;
            unsigned char Stereo:2;
            unsigned char Interlaced:1;
        }B;
    }MODE;

#if defined(_LINUX_)
}__attribute__ ((packed)) stPDBlock;
#else
}stPDBlock;
#endif

typedef struct _stNPDBlock{
    unsigned short PixelClock;
    unsigned char reserved1;
    unsigned char BlockType;
    unsigned char reserved2;
    
    union{
    unsigned char BlockContents[13];    
        

        struct{
            unsigned char MinVFreq;//59
            unsigned char MaxVFreq;//60
            unsigned char MinHFreq;//61
            unsigned char MaxHFreq;//62
            unsigned char Pxlclk;//63
            unsigned short ScndGTF;//64 65
            unsigned char StrtHFreq; //66
            unsigned char C; //67
            unsigned short M; //68 69
            unsigned char K; //70
            unsigned char J; //71
        }FD000A;


        struct{
            unsigned char reseved1;//59
            unsigned char reseved2;//60
            unsigned char reseved3;//61
            unsigned char reseved4;//62
            unsigned char reseved5;//63
            unsigned short ScndGTF;//64 65
            unsigned char StrtHFreq;//66
            unsigned char MinVFreq;//67
            unsigned char MaxVFreq;//68
            unsigned char MinHFreq;//69
            unsigned char MaxHFreq;//70
            unsigned char Pxlclk;//71
        }FD0200;

        struct{
            unsigned char Windex0;// 59
            unsigned char WB0notuse;//60
            unsigned char WB0WX; //61
            unsigned char WB0WY; //62
            unsigned char WB0gamma;//63
                        
            unsigned char Windex1;// 64
			union{
                unsigned char wlsb; //65
				struct{
					unsigned char whiteY:2;
					unsigned char whiteX:2;
					unsigned char reserved:4;
				}BO;
			}WB1lsb;

            unsigned char WB1WX; //66
            unsigned char WB1WY; //67
            unsigned char WB1gamma;//68
            unsigned char reseved[3];//69 70 71
        }FB;

        struct{
            struct{
                unsigned char Hresolution;
                struct{
                    unsigned char VFreq:6;
                    unsigned char AspectRatio:2;
                }B;
            }STDTMG[6];
            unsigned char reserved;
        }FA;
        
    }NPDB;

#if defined(_LINUX_)
}__attribute__ ((packed)) stNPDBlock;
#else
}stNPDBlock;
#endif


//EDID Data Format
typedef struct _stEDID{

    // HEADER INFOMATION
    unsigned char   HeaderInfo[8];
    union{
    unsigned short  ManufacturerID; 
        struct{
            unsigned short byte3rd:5;
            unsigned short byte2nd:5;
            unsigned short byte1st:5;
            unsigned short reserved:1;
        }B;
    }MANUID;
    
    unsigned short  ProductID;
    unsigned int    SerialNum;
    unsigned char   Week;
    unsigned char   Year;
    unsigned char   EDIDVersion;
    unsigned char   EDIDRevision;
    // BASIC DISPLAY PARAMETERS
    union{
        unsigned char videf;
        struct{
            unsigned char SerrationVsync:1;
            unsigned char SynconGreen:1;
            unsigned char CompositeSync:1;
            unsigned char SeparateSync:1;
            unsigned char Blank2BlackSetup:1;
            unsigned char VideoLevel:2;
            unsigned char VinType:1;
            
        }B;
    }VIDEF;

    unsigned char MaxHImgSize;
    unsigned char MaxVImgSize;
    unsigned char DisplayGamma;

    union{
        unsigned char pwrmgr;
        struct{
            unsigned char DefaultGTF:1;
            unsigned char PreferredTiming:1;
            unsigned char StandardColorSpace:1;
            unsigned char DisplayType:2;
            unsigned char ActiveOffLP:1;
            unsigned char Suspend:1;
            unsigned char Standby:1;
        }B;
    }PWRMGR;

    // CHROMATICITY COORDINATES
    union{
        unsigned char lsbrg;
        struct{
            unsigned char GreenY:2;
            unsigned char GreenX:2;
            unsigned char RedY:2;
            unsigned char RedX:2;
        }B;
    }LSBRG;
    union{
        unsigned char lsbbw;
        struct{
            unsigned char WhiteY:2;
            unsigned char WhiteX:2;
            unsigned char BlueY:2;
            unsigned char BlueX:2;
        }B;
    }LSBBW;

    struct{
        unsigned char RedX;
        unsigned char RedY;
        unsigned char GreenX;
        unsigned char GreenY;
        unsigned char BlueX;
        unsigned char BlueY;
        unsigned char WhiteX;
        unsigned char WhiteY;
    }HSB;

    union{
        unsigned char EstablishedTiming1;
        struct{
            unsigned char est720400a70:1;
            unsigned char est720400a88:1;
            unsigned char est640480a60:1;
            unsigned char est640480a67:1;
            unsigned char est640480a72:1;
            unsigned char est640480a75:1;
            unsigned char est800600a56:1;
            unsigned char est800600a60:1;
        }B;
    }ESTiming1;
    
    union{
        unsigned char EstablishedTiming2;
        struct{
            unsigned char est800600a72:1;
            unsigned char est800600a75:1;
            unsigned char est832624a75:1;
            unsigned char est1024768a87:1;
            unsigned char est1024768a60:1;
            unsigned char est1024768a70:1;
            unsigned char est1024768a75:1;
            unsigned char est12801024a75:1;
            
        }B;
    }ESTiming2;
    
    unsigned char ReservedTiming;

    //STANDARD TIMING IDENTIFICATION
    struct{
        unsigned char Hresolution;
        struct{
            unsigned char VFreq:6;
            unsigned char AspectRatio:2;
        }B;
    }STDTIMING[8];

    union{
        stPDBlock  PCLKDSCRTBlock;
        stNPDBlock NPCLKDSCRTBlock;
    }DescriptorBlock[4];

    unsigned char ExtensionEDID;
    unsigned char CheckSum;

#if defined(_LINUX_)
}__attribute__ ((packed)) stEDID;
#else
}stEDID;
#endif

#define BYTESIZE (8)
typedef struct _TIMINGDESCRIPTOR{
    int PixelClock;
    int HActive;
    int HBlanking;
    int VActive;
    int VBlanking;
    int HSyncOffset;
    int HSyncPulseWidth;
    int VSyncOffset;
    int VSyncPulseWidth;
    int HImageSize;
    int VImageSize;
    int HBoarder;
    int VBoarder;
    int Interlaced;
    int Stereo;
    int SeperateSync;
    int HSyncPol;
    int VSyncPol;
    int Stereomode;
#if defined(_LINUX_)
}__attribute__ ((packed)) TIMINGDESCRIPTOR;
#else
}TIMINGDESCRIPTOR;
#endif

#if !defined(_LINUX_)
#pragma pack()
#endif

static stEDID  gEDID;
static stCEABlock  gCEA;
static TIMINGDESCRIPTOR    *p1stTMG = NULL;
static TIMINGDESCRIPTOR    *pETMG = NULL;


int BlockHeadParse(stDBC* pPos, stCEABlock* pCEA, unsigned char* pHDROffset, int prtflag)
{
    int i, index;
	SVD stsvd;
    stDBC   BlkHdr;
    

    BlkHdr = *pPos;
    
    switch(BlkHdr.B.BlockType)
    {
        case 1:
            MSGPRT(prtflag,("\tBlock Type.... %s\n", (unsigned char*)"[AUDIO Data Block]"));
            MSGPRT(prtflag,("\tBlock Length.. %d\n", BlkHdr.B.ByteNumber));
            memcpy(&pCEA->ADB, pPos, sizeof(stADB));

            MSGPRT(prtflag,("\tAudio Format Code : %s\n", pAudioFormat[pCEA->ADB.SADB[0].sadb1.audioformat]));
            MSGPRT(prtflag,("\tChannel           : %s\n", pADChannel[pCEA->ADB.SADB[0].sadb1.channelnum]));
			
			MSGPRT(prtflag,("\tSampling Freq     :\n"));
			for(i=0;i<7;i++)
			{
				if( (pCEA->ADB.SADB[1].sadb2.samplefreq)&(0x1<<i) )
					MSGPRT(prtflag,("\t\t%s Supported\n", pSampleFreq[i]));
			}

			MSGPRT(prtflag,("\tBit Rate          :\n"));
            if( pCEA->ADB.SADB[0].sadb1.audioformat == 1) // LPCM
            {
                for(i=0;i<3;i++)
				{
					if( (pCEA->ADB.SADB[2].sadb3.bitrate)&(0x1<<i) )
						MSGPRT(prtflag,("\t\t%s Supported\n", pBitrate[i]));
				}
            }
            else
            {
                MSGPRT(prtflag,("\t\t%dkhz\n",pCEA->ADB.SADB[2].sadb3a*8));
            }
            
            *pHDROffset = pCEA->ADB.BlockHead.B.ByteNumber+1; // Plus Header
            break;//return (unsigned char*)"AUDIO";

        case 2:
            // parse Video Data Block
            MSGPRT(prtflag,("\tBlock Type.... %s\n", (unsigned char*)"[VIDEO Data Block]"));
            MSGPRT(prtflag,("\tBlock Length.. %d\n", BlkHdr.B.ByteNumber));
			MSGPRT(prtflag,("\tTiming Supported..\n"));

            //pCEA->VDB.BlockHead = BlkHdr;

			memcpy(&pCEA->VDB, pPos, BlkHdr.B.ByteNumber+1);// Plus Header
            
            for(i=0;i<pCEA->VDB.BlockHead.B.ByteNumber;i++)
            {
				//stsvd.svd = pEEDID[i];
                stsvd.svd = pCEA->VDB.ShortVideoDscrpt[i].svd;// = *(unsigned char*)&pPos[i];

				#if defined(HDMI_V1_4) //MVC_PROCESS
                if( 0 < (stsvd.B.index) && (stsvd.B.index) < 64+2)
                    MSGPRT(prtflag,("\t\t%s %s\n", pStandardResTiming[stsvd.B.index],(stsvd.B.native)? "[Native]":""));
                else
                    MSGPRT(prtflag,("\t\t%s\n", (unsigned char*)"Reseved"));
				#else
                if( 0 < (stsvd.B.index) && (stsvd.B.index) < 64)
                    MSGPRT(prtflag,("\t\t%s %s\n", pStandardResTiming[stsvd.B.index],(stsvd.B.native)? "[Native]":""));
                else
                    MSGPRT(prtflag,("\t\t%s\n", (unsigned char*)"Reseved"));
				#endif
            }
            *pHDROffset = pCEA->VDB.BlockHead.B.ByteNumber+1;    
            break;//return (unsigned char*)"VIDEO";

        case 3:
            MSGPRT(prtflag,("\tBlock Type.... %s\n", (unsigned char*)"[Vendor Specific Data Block]"));
            MSGPRT(prtflag,("\tBlock Length.. %d\n", BlkHdr.B.ByteNumber));
            //memcpy(&pCEA->VSDB,        pPos, BlkHdr.B.ByteNumber+1);
            memcpy(&pCEA->VSDB,        pPos, (  (BlkHdr.B.ByteNumber > sizeof(pCEA->VSDB)) ? sizeof(pCEA->VSDB)         : (BlkHdr.B.ByteNumber+1) ));
            MSGPRT(prtflag,("\tIEEE Registration Number : 0x%2X%2X%2X\n", pCEA->VSDB.ieeeregid[2],pCEA->VSDB.ieeeregid[1],pCEA->VSDB.ieeeregid[0]));
            MSGPRT(prtflag,("\tCEC Physical Address     :%d.%d.%d.%d\n", pCEA->VSDB.cpntphysaddr[0].B.high,pCEA->VSDB.cpntphysaddr[0].B.low,pCEA->VSDB.cpntphysaddr[1].B.high,pCEA->VSDB.cpntphysaddr[1].B.low));

            MSGPRT(prtflag,("\tDVI Duallink Operation   : %s\n", (pCEA->VSDB.colorsink.B.DVIDual)? "YES":"NO"));
            MSGPRT(prtflag,("\tYCbCr 4:4:4 Support      : %s\n", (pCEA->VSDB.colorsink.B.ycbcr444)? "YES":"NO"));
            MSGPRT(prtflag,("\t30bpp Support            : %s\n", (pCEA->VSDB.colorsink.B.dc30bpp)? "YES":"NO"));
            MSGPRT(prtflag,("\t36bpp Support            : %s\n", (pCEA->VSDB.colorsink.B.dc36bpp)? "YES":"NO"));
            MSGPRT(prtflag,("\t48bpp Support            : %s\n", (pCEA->VSDB.colorsink.B.dc48bpp)? "YES":"NO"));
            MSGPRT(prtflag,("\tAI(ACP/ISRC) Support     : %s\n", (pCEA->VSDB.colorsink.B.AIsupport)? "YES":"NO"));

            MSGPRT(prtflag,("\tMaxinum TMDS Clock       : %dMhz\n", pCEA->VSDB.tmdsfreq*5));
			MSGPRT(prtflag,("\tLatency Field Present    : %s\n", (pCEA->VSDB.latency.B.latencyfield)? "YES":"NO"));
            MSGPRT(prtflag,("\tILatency Field Present   : %s\n", (pCEA->VSDB.latency.B.ilatencyfield)? "YES":"NO"));
            
			MSGPRT(prtflag,("\t3D Present               : %s\n", (pCEA->VSDB.struct_3d.B.present_3d)? "YES":"NO"));
            MSGPRT(prtflag,("\t3D Multi Present         : %s\n", (pCEA->VSDB.struct_3d.B.multi_present_3d)? "YES":"NO"));
			MSGPRT(prtflag,("\tHDMI_3D_LEN              : %d\n",  pCEA->VSDB.vic_3d_len.B.hdmi_3d_len));
			MSGPRT(prtflag,("\tHDMI_VIC_LEN             : %d\n",  pCEA->VSDB.vic_3d_len.B.hdmi_vic_len));

            if(pCEA->VSDB.latency.B.latencyfield)
            {
				MSGPRT(prtflag,("\tVideo Latency : "));
				if(pCEA->VSDB.videolatency)
				{
					if(pCEA->VSDB.videolatency == 255)
						MSGPRT(prtflag,("%dNo Video is supported in this device\n"));
					else
						MSGPRT(prtflag,("%dms\n",(pCEA->VSDB.videolatency-1)*2 ));
				}
				else
					MSGPRT(prtflag,("invalid or unknown\n"));

				MSGPRT(prtflag,("\tAudio Latency : "));
				if(pCEA->VSDB.audiolatency)
				{
					if(pCEA->VSDB.audiolatency == 255)
						MSGPRT(prtflag,("%dNo Audio is supported in this device\n"));
					else
						MSGPRT(prtflag,("%dms\n",(pCEA->VSDB.audiolatency-1)*2 ));
				}
				else
					MSGPRT(prtflag,("invalid or unknown\n"));

		    }

            if(pCEA->VSDB.latency.B.ilatencyfield)
            {
				MSGPRT(prtflag,("\tIVideo Latency : "));
				if(pCEA->VSDB.interlacedvideolatency)
				{
					if(pCEA->VSDB.interlacedvideolatency == 255)
						MSGPRT(prtflag,("%dNo Video is supported in this device\n"));
					else
						MSGPRT(prtflag,("%dms\n",(pCEA->VSDB.interlacedvideolatency-1)*2 ));
				}
				else
					MSGPRT(prtflag,("invalid or unknown\n"));

				MSGPRT(prtflag,("\tIAudio Latency : "));
				if(pCEA->VSDB.interlacedaudiolatency)
				{
					if(pCEA->VSDB.interlacedaudiolatency == 255)
						MSGPRT(prtflag,("%dNo Audio is supported in this device\n"));
					else
						MSGPRT(prtflag,("%dms\n",(pCEA->VSDB.interlacedaudiolatency-1)*2 ));
				}
				else
					MSGPRT(prtflag,("invalid or unknown\n"));
            }
            *pHDROffset = pCEA->VSDB.BlockHead.B.ByteNumber+1;
            break;//return (unsigned char*)"Vendor Specific";

		case 4:
            MSGPRT(prtflag,("\tBlock Type.... %s\n", (unsigned char*)"[Speaker Allocation Data Block]"));
            MSGPRT(prtflag,("\tBlock Length.. %d\n", BlkHdr.B.ByteNumber));

            memcpy(&pCEA->SPK, pPos, 3+1);
            
            if( (pCEA->SPK.Byte2nd == 0)&&(pCEA->SPK.Byte3rd == 0) ) //OK Speaker Location block..
            {
                MSGPRT(prtflag,("\t\tFront Left/Right Present : %s\n",(pCEA->SPK.SpkLocation.B.frntLR)? "YES":"NO"));
                MSGPRT(prtflag,("\t\tLFE Present : %s\n",(pCEA->SPK.SpkLocation.B.LFE)? "YES":"NO"));    
                MSGPRT(prtflag,("\t\tFront Center Present : %s\n",(pCEA->SPK.SpkLocation.B.frntCtr)? "YES":"NO"));    
                MSGPRT(prtflag,("\t\tRear Left/Right Present : %s\n",(pCEA->SPK.SpkLocation.B.rearLR)? "YES":"NO"));    
                MSGPRT(prtflag,("\t\tRear Center Present : %s\n",(pCEA->SPK.SpkLocation.B.rearCtr)? "YES":"NO"));    
                MSGPRT(prtflag,("\t\tFront Left/Right Center Present : %s\n",(pCEA->SPK.SpkLocation.B.frntLR)? "YES":"NO"));    
                MSGPRT(prtflag,("\t\tRear Left/Right Center Present : %s\n",(pCEA->SPK.SpkLocation.B.frntLR)? "YES":"NO"));
                *pHDROffset = pCEA->SPK.BlockHead.B.ByteNumber+1;
            }
            else
            {
                memset(&pCEA->SPK, 0X00, 3+1);
                pCEA->SPK.BlockHead.B.BlockType = 4;// Speaker
                *pHDROffset = 0;
                MSGPRT(prtflag,("\t\tNo Data Block...\n"));    
            }
            break;//return (unsigned char*)"Speaker Location";

        default:
            *pHDROffset = BlkHdr.B.ByteNumber+1;
            //*pHDROffset = pCEA->VSDB.BlockHead.B.ByteNumber+1;
            break;
            
    }

	return 0;
}


//int parse_timingdescriptor(unsigned char* pPosition, stCEABlock* pCEA, unsigned char* pOffset)
int parse_timingdescriptor(unsigned char* pDTDPos,TIMINGDESCRIPTOR* pTD, int prtflag)    
{
    int i;
    stPDBlock  DTDB;
    stNPDBlock NPDTDB;
    
    int temp;
    int Hres, Vres;
    double  freq;

    temp = sizeof(stPDBlock);
    temp = sizeof(stNPDBlock);

    memcpy(&DTDB, pDTDPos, sizeof(stPDBlock));

    if(DTDB.pixelclock == 0)
    {
        //        return 0;
        memcpy(&NPDTDB, pDTDPos, sizeof(stNPDBlock));    
        
        if( NPDTDB.BlockType < 0xF8 || NPDTDB.BlockType > 0xFF)
            return -1;

        switch(NPDTDB.BlockType)
        {

            MSGPRT(prtflag, ("  Block Type : %s\n", pBlockType[0xFF-NPDTDB.BlockType]));       

            case 0xFF:// Monitor Serial Number
            case 0xFE:// ASCII String
            case 0xFC:// Monitor Name
                MSGPRT(prtflag, ("    %s\n", &NPDTDB.NPDB.BlockContents));
                break;
                
            case 0xFD: // Monitor range limit
                // Process Secondary GTF toggle..
                temp = 0;
                temp |= (NPDTDB.NPDB.FD000A.ScndGTF&0xFF00)>>8;
                temp |= (NPDTDB.NPDB.FD000A.ScndGTF&0x00FF)<<8;

                if( temp == 0x000A )
                {
                    MSGPRT(prtflag, ("    Horizontal Range    : %d - %d KHz\n", NPDTDB.NPDB.FD000A.MinHFreq, NPDTDB.NPDB.FD000A.MaxHFreq));
                    MSGPRT(prtflag, ("    Vertical   Range    : %d - %d KHz\n", NPDTDB.NPDB.FD000A.MinVFreq, NPDTDB.NPDB.FD000A.MaxVFreq));
                    MSGPRT(prtflag, ("    Pixel Clock         :      %d MHz\n", NPDTDB.NPDB.FD000A.Pxlclk*10));
                    MSGPRT(prtflag, ("    Secondary GTF Toggle: 0x%X\n",temp));
                }
                else
                {
                    MSGPRT(prtflag, ("    Horizontal Range    : %d - %d KHz\n", NPDTDB.NPDB.FD0200.MinHFreq, NPDTDB.NPDB.FD0200.MaxHFreq));
                    MSGPRT(prtflag, ("    Vertical   Range    : %d - %d KHz\n", NPDTDB.NPDB.FD0200.MinVFreq, NPDTDB.NPDB.FD0200.MaxVFreq));
                    MSGPRT(prtflag, ("    Pixel Clock         :      %d MHz\n", NPDTDB.NPDB.FD0200.Pxlclk*10));
                    MSGPRT(prtflag, ("    Secondary GTF Toggle: 0x%X\n",temp));
                }
                break;

            case 0xFB: // White Point index
                if( NPDTDB.NPDB.FB.Windex0 == 1) // following bytes are meaningful..
                {
                    MSGPRT(prtflag, ("  White point index #0 White X  : %d\n",NPDTDB.NPDB.FB.WB0WX));
                    MSGPRT(prtflag, ("  White point index #0 White Y  : %d\n",NPDTDB.NPDB.FB.WB0WY));
                    MSGPRT(prtflag, ("  White point index #0 Gamma    : %.3f\n",(double)NPDTDB.NPDB.FB.WB0gamma/100 + 1));
                }
                
                if( NPDTDB.NPDB.FB.Windex1 == 1) // following bytes are meaningful..else
                {
                    temp = 0;
                    temp = NPDTDB.NPDB.FB.WB1WX;
                    temp <<= 2;
                    temp |= NPDTDB.NPDB.FB.WB1lsb.BO.whiteX;
                    MSGPRT(prtflag, ("  White point index #1 White X  : %d\n",temp));

                    temp = 0;
                    temp = NPDTDB.NPDB.FB.WB1WY;
                    temp <<= 2;
                    temp |= NPDTDB.NPDB.FB.WB1lsb.BO.whiteY;
                    MSGPRT(prtflag, ("  White point index #1 White Y  : %d\n",temp));

                }

                break;

            case 0xFA: // Standard Timing Identification
                // Standard Timing
                //i = sizeof(NPDTDB.NPDB.FA.STDTMG);
                for(i=0;i<6;i++)
                {
                    Hres = NPDTDB.NPDB.FA.STDTMG[i].Hresolution*8+248;
                    switch( NPDTDB.NPDB.FA.STDTMG[i].B.AspectRatio )
                    {
                        case 0: // 16:10
                            Vres = (10*Hres)/16;
                            break;
                        case 1: // 4:3
                            Vres = (3*Hres)/4;
                            break;
                        case 2: // 5:4
                            Vres = (4*Hres)/5;
                            break;
                        case 3: // 16:9
                            Vres = (9*Hres)/16;
                            break;
                    }
                    MSGPRT(prtflag, ("  %dx%d@%dHz\n", Hres, Vres,NPDTDB.NPDB.FA.STDTMG[i].B.VFreq+60));
                }    
                break;
            case 0xF9:
                MSGPRT(prtflag, ("  Currently Undefined...\n"));
                break;
            case 0xF8:
                MSGPRT(prtflag, ("  Defined by Manufacture...\n"));
                break;
        }
    
    }
    else
    {
    pTD->PixelClock = DTDB.pixelclock;

    pTD->HActive = (DTDB.HHigh.B.HActivehigh);
    pTD->HActive <<= BYTESIZE;
    pTD->HActive |= DTDB.HActive;

    pTD->HBlanking = DTDB.HHigh.B.HBlankinghigh;
    pTD->HBlanking <<= BYTESIZE;
    pTD->HBlanking |= DTDB.HBlanking;

    pTD->VActive = DTDB.VHigh.B.VActivehigh;
    pTD->VActive <<= BYTESIZE;
    pTD->VActive |= DTDB.VActive;

    pTD->VBlanking = DTDB.VHigh.B.VBlankinghigh;
    pTD->VBlanking <<= BYTESIZE;
    pTD->VBlanking |= DTDB.VBlanking;

    pTD->HSyncOffset = DTDB.HSB.B.HSyncOffsetHSB;
    pTD->HSyncOffset <<= BYTESIZE;
    pTD->HSyncOffset |= DTDB.HSyncOffset;

    pTD->HSyncPulseWidth = DTDB.HSB.B.HSyncPulsewidthHSB;
    pTD->HSyncPulseWidth <<= BYTESIZE;
    pTD->HSyncPulseWidth |= DTDB.HSyncPulsewidth;

    pTD->VSyncOffset = DTDB.HSB.B.VSyncOffsetHSB;
    pTD->VSyncOffset <<= BYTESIZE/2;
    pTD->VSyncOffset |= DTDB.VSync.B.VSyncOffset;

    pTD->VSyncPulseWidth = DTDB.HSB.B.VSyncPulsewidthHSB;
    pTD->VSyncPulseWidth <<= BYTESIZE/2;
    pTD->VSyncPulseWidth |= DTDB.VSync.B.VSyncPulsewidth;

    pTD->HImageSize = DTDB.HSBIMGSize.B.HImgszHSB;
    pTD->HImageSize <<= BYTESIZE;
    pTD->HImageSize |= DTDB.Himagesize;

    pTD->VImageSize = DTDB.HSBIMGSize.B.VImgszHSB;
    pTD->VImageSize <<= BYTESIZE;
    pTD->VImageSize |= DTDB.Vimagesize;

    pTD->HBoarder = DTDB.HBoarder;
    pTD->VBoarder = DTDB.VBoarder;

    pTD->Interlaced = DTDB.MODE.B.Interlaced;
    pTD->Stereo = DTDB.MODE.B.Stereo;
    pTD->SeperateSync = DTDB.MODE.B.SeperateSync;
    pTD->HSyncPol = DTDB.MODE.B.HSyncpositive;
    pTD->VSyncPol = DTDB.MODE.B.VSyncpositive;
    pTD->Stereomode = DTDB.MODE.B.Stereomode;

    freq = (double)(pTD->PixelClock*10000)/((pTD->HActive+pTD->HBlanking)*(pTD->VActive+pTD->VBlanking));
    
    MSGPRT(prtflag,("   %dx%d%c at %3.3fMHz\n", pTD->HActive, (pTD->Interlaced)? pTD->VActive*2 : pTD->VActive,(pTD->Interlaced)? 'i':'p',freq));
    MSGPRT(prtflag,("   Label             : %dx%d\n",pTD->HActive,(pTD->Interlaced)? pTD->VActive*2 : pTD->VActive));
    MSGPRT(prtflag,("   PixelClk          : %3.3fMhz\n",(double)pTD->PixelClock/100));//10Khz
    MSGPRT(prtflag,("   HActive           : %d\n",pTD->HActive));
    MSGPRT(prtflag,("   HStart Sync Pulse : %d\n",pTD->HActive+pTD->HSyncOffset));
    MSGPRT(prtflag,("   HEnd Sync Pulse   : %d\n",pTD->HActive+pTD->HSyncOffset+pTD->HSyncPulseWidth));
    MSGPRT(prtflag,("   HEnd Blanking intv: %d\n",pTD->HActive+pTD->HBlanking));
    
    MSGPRT(prtflag,("   VActive           : %d\n",(pTD->Interlaced)? pTD->VActive*2 : pTD->VActive));
    MSGPRT(prtflag,("   VStart Sync Pulse : %d\n",(pTD->Interlaced)? (pTD->VActive+pTD->VSyncOffset)*2 : pTD->VActive+pTD->VSyncOffset));
    MSGPRT(prtflag,("   VEnd Sync Pulse   : %d\n",(pTD->Interlaced)? (pTD->VActive+pTD->VSyncOffset+pTD->VSyncPulseWidth)*2 : pTD->VActive+pTD->VSyncOffset+pTD->VSyncPulseWidth));
    MSGPRT(prtflag,("   VEnd Blanking intv: %d\n",(pTD->Interlaced)? (pTD->VActive+pTD->VBlanking)*2 : pTD->VActive+pTD->VBlanking));
    MSGPRT(prtflag,("   HSync%c, VSync%c\n",(pTD->HSyncPol)? '+':'-', (pTD->VSyncPol)? '+':'-'));
    }
	return 0;    
}


int parse_edid2( void* pEDIDB, unsigned char* pBuffer, int prtflag)
{
    int size,i;
    unsigned short temp1, temp2;
    stEDID  *pEDID =(stEDID*)pEDIDB;
    unsigned char hdrinfo[] = {0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00};

    memcpy(pEDID, pBuffer, sizeof(stEDID));
    p1stTMG = (TIMINGDESCRIPTOR*)malloc(sizeof(TIMINGDESCRIPTOR)*4);
	memset(p1stTMG,0x00,sizeof(TIMINGDESCRIPTOR)*4);

    // Check EDID Block Header Info..
    for(i=0;i<8;i++)
    {
        if(hdrinfo[i] != pEDID->HeaderInfo[i])
        {
            MSGPRT(prtflag, ("This isn't EDID Block: %s\n", pEDID->HeaderInfo));
            break;
        }
    }
/*
    if( strcmp((const char*)pEDID->HeaderInfo, (const char*)hdrinfo) == 0 )
        MSGPRT(prtflag, ("Header Infomation: %s\n", &pEDID->HeaderInfo));
    else
        MSGPRT(prtflag, ("This isn't EDID Block: %s\n", pEDID->HeaderInfo));
*/
    // Reverse Order of Manufacture ID
    temp1 = pEDID->MANUID.ManufacturerID;
    pEDID->MANUID.ManufacturerID = temp1>>8;
    pEDID->MANUID.ManufacturerID |= (temp1&0xFF)<<8;


    MSGPRT(prtflag, ("  Manufacturer ID       :%c%c%c\n", (pEDID->MANUID.B.byte1st+0x40),(pEDID->MANUID.B.byte2nd+0x40),(pEDID->MANUID.B.byte3rd+0x40)));
    MSGPRT(prtflag, ("  Product ID            :0x%X\n", pEDID->ProductID));
    MSGPRT(prtflag, ("  32bit Serial Num      :0x%X\n", pEDID->SerialNum));
    MSGPRT(prtflag, ("  Year of Manufacture   :%d\n", pEDID->Year+1990));
    MSGPRT(prtflag, ("  Week of Manufacture   :%d\n", pEDID->Week));
    MSGPRT(prtflag, ("  EDID Version.Revision :%d.%d\n", pEDID->EDIDVersion,pEDID->EDIDRevision));


    //Basic Display Parameters
    //Vedio Input Definition
    if( pEDID->VIDEF.B.VinType == 1) // Digital
    {
        MSGPRT(prtflag, ("Video Input Signal Type :Digital\n"));
        MSGPRT(prtflag, ("  DFP 1.x Compatible    :%s\n", (pEDID->VIDEF.B.SerrationVsync) ? "YES":"NO")); 
    }
    else // Analog
    {
        MSGPRT(prtflag, ("Video Input Signal Type:Analog\n"));
        MSGPRT(prtflag, ("  Video Level         :%d\n", pEDID->VIDEF.B.VideoLevel));
        MSGPRT(prtflag, ("  Blank-to-Black setup:%s\n",(pEDID->VIDEF.B.Blank2BlackSetup)? "YES":"NO"));
        MSGPRT(prtflag, ("  Separate Sync       :%s\n",(pEDID->VIDEF.B.SeparateSync)? "YES":"NO"));
        MSGPRT(prtflag, ("  Composite Sync      :%s\n",(pEDID->VIDEF.B.CompositeSync)? "YES":"NO"));
        MSGPRT(prtflag, ("  Sync on Green       :%s\n",(pEDID->VIDEF.B.SynconGreen)? "YES":"NO"));
        MSGPRT(prtflag, ("  Serration VSync     :%s\n",(pEDID->VIDEF.B.SerrationVsync)? "YES":"NO"));
    }

    MSGPRT(prtflag, ("Image Size[HxV]       :%dcm x %dcm\n",pEDID->MaxHImgSize, pEDID->MaxVImgSize));
    MSGPRT(prtflag, ("Power Management supports:%s %s %s\n",
        (pEDID->PWRMGR.B.Standby)? "Standby":"",
        (pEDID->PWRMGR.B.Suspend)? "Suspend":"",
        (pEDID->PWRMGR.B.ActiveOffLP)? "ActiveOff/LowPower":"" ));
    
    MSGPRT(prtflag, ("Display Gamma        :%2.2f\n",(double)pEDID->DisplayGamma/100 + 1));

    
    switch(pEDID->PWRMGR.B.DisplayType)
    {
        case 0:
            MSGPRT(prtflag, ("Display Type         :Monochrome\n"));
            break;
        case 1:
            MSGPRT(prtflag, ("Display Type         :RGB Color\n"));
            break;
        case 2:
            MSGPRT(prtflag, ("Display Type         :Non RGB multicolor\n"));
            break;
        case 3:
            MSGPRT(prtflag, ("Display Type         :Undefined\n"));
            break;
    }

    MSGPRT(prtflag, ("Standard Color Space : %s\n",(pEDID->PWRMGR.B.StandardColorSpace)? "YES":"NO" ));
    MSGPRT(prtflag, ("Preferred Timing Mode: %s\n",(pEDID->PWRMGR.B.PreferredTiming)?"YES":"NO" ));
    MSGPRT(prtflag, ("Default GTF Supported: %s\n",(pEDID->PWRMGR.B.DefaultGTF)? "YES":"NO" ));
    

    // Chromaticity Coordinates
    temp1 = pEDID->HSB.RedX;
    temp1 <<= 2;
    temp1 |= pEDID->LSBRG.B.RedX;
    temp2 = pEDID->HSB.RedY;
    temp2 <<= 2;
    temp2 |= pEDID->LSBRG.B.RedY;
    MSGPRT(prtflag, ("Red   Chromacity     :Rx %1.3f - Ry %1.3f\n", (double)temp1/1000, (double)temp2/1000));

    temp1 = pEDID->HSB.GreenX;
    temp1 <<= 2;
    temp1 |= pEDID->LSBRG.B.GreenX;
    temp2 = pEDID->HSB.GreenY;
    temp2 <<= 2;
    temp2 |= pEDID->LSBRG.B.GreenY;
    MSGPRT(prtflag, ("Green Chromacity     :Gx %1.3f - Gy %1.3f\n", (double)temp1/1000, (double)temp2/1000));

    temp1 = pEDID->HSB.BlueX;
    temp1 <<= 2;
    temp1 |= pEDID->LSBBW.B.BlueX;
    temp2 = pEDID->HSB.BlueY;
    temp2 <<= 2;
    temp2 |= pEDID->LSBBW.B.BlueY;
    MSGPRT(prtflag, ("Blue  Chromacity     :Bx %1.3f - By %1.3f\n", (double)temp1/1000, (double)temp2/1000));
    temp1 = pEDID->HSB.WhiteX;
    temp1 <<= 2;
    temp1 |= pEDID->LSBBW.B.WhiteX;
    temp2 = pEDID->HSB.WhiteY;
    temp2 <<= 2;
    temp2 |= pEDID->LSBBW.B.WhiteY;
    MSGPRT(prtflag, ("White Chromacity     :Wx %1.3f - Wy %1.3f\n", (double)temp1/1000, (double)temp2/1000));

    //Established Timing
    temp1 = pEDID->ESTiming2.EstablishedTiming2;
    temp1 <<= 8;
    temp1 |= pEDID->ESTiming1.EstablishedTiming1;

    MSGPRT(prtflag, ("Established Timing\n"));
    for(i=0;i<16;i++)
    {
        if( temp1&(0x1<<i) )
            MSGPRT(prtflag, ("  %s\n", pEstablishedTiming[i]));
    }

    // Reserved Timing
    if(pEDID->ReservedTiming)
    {
        MSGPRT(prtflag, ("Manufacturer's Reserved Timing\n"));
        if(pEDID->ReservedTiming&(1<<7))
            MSGPRT(prtflag, ("  1152x768@75Hz(Mac II, Apple\n"));
    }

    // Standard Timing
    MSGPRT(prtflag, ("Standard Timing Identification\n"));
    for(i=0;i<8;i++)
    {
        if( pEDID->STDTIMING[i].Hresolution != 0x1)
        {
        temp1 = pEDID->STDTIMING[i].Hresolution*8+248;
        switch( pEDID->STDTIMING[i].B.AspectRatio )
        {
            case 0: // 16:10
                temp2 = (10*temp1)/16;
                break;
            case 1: // 4:3
                temp2 = (3*temp1)/4;
                break;
            case 2: // 5:4
                temp2 = (4*temp1)/5;
                break;
            case 3: // 16:9
                temp2 = (9*temp1)/16;
                break;
        }
        MSGPRT(prtflag, ("  %dx%d@%dHz\n", temp1, temp2,pEDID->STDTIMING[i].B.VFreq+60));
    }    
    
    }    
    
    for(i=0;i<4;i++)
    {
        MSGPRT(prtflag, ("Detailed Timing Descriptor #%d\n", i));
        parse_timingdescriptor((unsigned char*)&pEDID->DescriptorBlock[i],&p1stTMG[i], prtflag);
    }
    
    return pEDID->ExtensionEDID;
}

int parse_extblockcount(void* pdata)
{
    int i,cnt=0;
    int blocktype;
    char *pElement = (char*)pdata;
    
    for(i=1;i<128;i++)
    {
        blocktype = pElement[i];
        
        switch(blocktype)
        {
            case 0x01: // LCD Timings
            case 0x02: // Additional Timing Data Type 2
            case 0x20: // EDID 2.0 Extension
            case 0x30: // Color information type 0
            case 0x40: // DVI feature data
            case 0x50: // Touch screen data
            case 0xF0: // Block Map
            case 0xFF: // Extension defined by monitor manufacturer
                cnt++;
                break;
        }
    }
    return cnt;
}

int parse_eedid(void* pCEAB, char* pEEDID, int prtflag)
{
    int DTDsCnt=0,i=0,ret=1;
    unsigned int blockparse = 0;
    unsigned char checksum = 0, HDRPos = 0, HDROffset = 0, DTDOffset=0;
    unsigned char*  pPointer;
	
	stPDBlock *pDTDs;
    stCEABlock  *CEA = (stCEABlock*)pCEAB;
	
    
    // Parse EDID Timing Extension
    memcpy(&CEA->CEAInfo, pEEDID, sizeof(stEEDID));
    //HDROffset += 5;
    
    // Check Extension Tag
    switch(CEA->CEAInfo.ExtensionTag)
    {
        case 0x02:
            MSGPRT(prtflag,("EDID Extension : CEA EDID Timing Extension...\n"));
            break;
            
        case 0xF0:
            MSGPRT(prtflag,("EDID Extension : Block Map...\n"));
            // need to parse.. But..
            return ret;
            break;

        default:
            MSGPRT(prtflag,("EDID Extension : Tag is unknown so far...\n"));
            break;
    }
    
/*
    if(CEA->CEAInfo.ExtensionTag != 0x02)
        MSGPRT(prtflag,("EDID Extension : Tag is invalid..\n"));
    else
        MSGPRT(prtflag,("EDID Extension : CEA EDID Timing Extension...\n"));
*/  
    // Check Revision Number
    MSGPRT(prtflag,("\tRevision = %d\n", CEA->CEAInfo.RevisionNumber ));

    // Check DTD Data
    if( CEA->CEAInfo.DTDs == 0x00)
    {
        blockparse = NONDTD;
        MSGPRT(prtflag,("\tThere are no DTDs and non-DTD Data..\n"));
    }
    else if( CEA->CEAInfo.DTDs == 0x04)
    {
        blockparse = DTDCOLLECTION;
        MSGPRT(prtflag,("\tNo non-DTD data is present..\n"));
	}
    else
    {
        blockparse = DBCCOLLECTION;
        MSGPRT(prtflag,("\t18Byte DTDs begins at %d \n", CEA->CEAInfo.DTDs));
    }
       

    MSGPRT(prtflag,("\tDTV underscan....\t%s Supported\n", (CEA->CEAInfo.DTVInfo.B.underscan)? "   ":"Not"));
    MSGPRT(prtflag,("\tBasicAudio.......\t%s Supported\n", (CEA->CEAInfo.DTVInfo.B.basicaudio)? "   ":"Not"));
    MSGPRT(prtflag,("\tYCbCr 4:4:4......\t%s Supported\n", (CEA->CEAInfo.DTVInfo.B.YCbCr444)? "   ":"Not"));
    MSGPRT(prtflag,("\tYCbCr 4:2:2......\t%s Supported\n", (CEA->CEAInfo.DTVInfo.B.YCbCr422)? "   ":"Not"));
    MSGPRT(prtflag,("\tNative Format....\t%3d Supported\n",CEA->CEAInfo.DTVInfo.B.NativeFormatNum));

    if( blockparse == DBCCOLLECTION)
    {
        // if 02byte is 
        MSGPRT(prtflag,("Data Block Collection Parse..\n"));
        HDRPos += 4;
        
        while(HDRPos < CEA->CEAInfo.DTDs)
        {
            BlockHeadParse((stDBC*)(pEEDID+HDRPos), CEA, &HDROffset, prtflag);
            HDRPos += HDROffset;
        }
/*        
        BlockHeadParse((stDBC*)(pEEDID+HDRPos), CEA, &HDROffset);
        HDRPos += HDROffset;
        BlockHeadParse((stDBC*)(pEEDID+HDRPos), CEA, &HDROffset);
        HDRPos += HDROffset;
        BlockHeadParse((stDBC*)(pEEDID+HDRPos), CEA, &HDROffset);
        HDRPos += HDROffset;
*/
        
        // current position + DTDCount*sizeof(DTD) < 110
		DTDOffset = HDRPos;//CEA->CEAInfo.DTDs;
		while(DTDOffset < 110)
		{
			DTDOffset+=18;
			DTDsCnt++;
		}

		pETMG = (TIMINGDESCRIPTOR*)malloc(sizeof(TIMINGDESCRIPTOR)*(DTDsCnt));

		DTDOffset = HDRPos;//CEA->CEAInfo.DTDs;
        
        // 18Byte copy DTD from position
        MSGPRT(prtflag,("[Extension Detailed Timing Descriptor Block Parse]\n"));
		while(i<DTDsCnt)
		{
            MSGPRT(prtflag,("  Detailed Timing #%d\n", i));
			parse_timingdescriptor((unsigned char*)(pEEDID + DTDOffset), &pETMG[i], prtflag);
			i++;
			DTDOffset+=18;
		}
		CEA->NumofDTDs = DTDsCnt;
        //parse_timingdescriptor((unsigned char*)(pEEDID+HDRPos),CEA,&HDROffset);
    }
	return ret;
}


int EDID_parsing(int prtflag)
{
	char * EdidData;
    int extblocks;
    int i;
	FPRINTF("%s prtflag:%d", __func__, prtflag);
	
    // check if read edid?
    if (!EDIDRead())    {
        DPRINTF("%d EDID Read Fail!!!\n",__LINE__);
        return 0;
    }    

    extblocks = parse_edid2(&gEDID, (unsigned char*)gEdidData, prtflag);
    
	if(extblocks)
	{
        if(extblocks==1)
        {
	        char temp[128];
	        memset(temp,0xFF,EDID_CHK_LENGTH);

			EdidData = (char*)(gEdidData+128);
			
	        if(memcmp(temp, EdidData, EDID_CHK_LENGTH) == 0 )
	        {
	            DPRINTF("This Block is not EDID. It's Invalid..\n");
				return 0;
	        }
			else
			{
				parse_eedid(&gCEA, EdidData, prtflag);
			}
        }
        else
        {
	        char temp[128];
	        memset(temp,0xFF,EDID_CHK_LENGTH);
            for(i=0;i<extblocks;i++)
            {
				EdidData = (char*)( gEdidData + ((i+1)*128) );

		        if(memcmp(temp, EdidData, EDID_CHK_LENGTH) == 0 )
		        {
		            DPRINTF("This Block is not EDID. It's Invalid..\n");
					return 0;
		        }
				else
				{
					parse_eedid(&gCEA, EdidData, prtflag);
				}
            }
        }
		//parse_eedid(&gCEA, (char*)(gEdidData+128), prtflag);
	}

    return 1;
}

void EDIDInfo(void)
{
	FPRINTF("%s ", __func__);
    EDID_parsing(1);
}

/**
 * Calculate a checksum.
 * @param   buffer  [in]    Pointer to data to calculate a checksum
 * @param   size    [in]    Sizes of data
 * @return  If checksum result is 0, return 1;Otherwise, return 0.
 */
static int CalcChecksum(const unsigned char* const buffer, const int size)
{
    unsigned char i,sum;
    int ret = 1;

    // check parameter
    if ( buffer == NULL ){
        DPRINTF("invalid parameter : buffer\n");
        return 0;
    }
	
    for (sum = 0, i = 0 ; i < size; i++)
    {
        sum += buffer[i];
    }

    return sum;
}

#if 1
unsigned char EDID_Sample[]=
{
	0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x38,0xA3,0x98,0x79,0x01,0x01,0x01,0x01,
	0x0F,0x12,0x01,0x03,0x80,0x00,0x00,0x78,0x2A,0x2A,0x02,0xA9,0x54,0x5D,0x9D,0x25,
	0x0E,0x52,0x5D,0xBF,0xEE,0x80,0x61,0x59,0x71,0x59,0x81,0x19,0x81,0x40,0x81,0x80,
	0x90,0x40,0x95,0x00,0x01,0x01,0x64,0x19,0x00,0x40,0x41,0x00,0x26,0x30,0x18,0x88,
	0x36,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x0E,0x1F,0x00,0x80,0x51,0x00,0x1E,0x30,
	0x40,0x80,0x37,0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0x00,0x00,0x00,0xFD,0x00,0x32,
	0x56,0x1F,0x5B,0x10,0x00,0x0A,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0xFC,
	0x00,0x56,0x54,0x38,0x30,0x30,0x0A,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x01,0xB3,
	0x02,0x03,0x28,0x76,0x52,0x02,0x03,0x11,0x12,0x84,0x05,0x13,0x14,0x10,0x1F,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x23,0x09,0x07,0x07,0x6C,0x03,0x0C,0x00,0x10,
	0x00,0x38,0x1E,0xC0,0x1A,0x00,0x27,0x00,0x8C,0x0A,0xD0,0x8A,0x20,0xE0,0x2D,0x10,
	0x10,0x3E,0x96,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x8C,0x0A,0xD0,0x90,0x20,0x40,
	0x31,0x20,0x0C,0x40,0x55,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x01,0x1D,0x00,0x72,
	0x51,0xD0,0x1E,0x20,0x6E,0x28,0x55,0x00,0x00,0x00,0x00,0x00,0x00,0x1E,0x01,0x1D,
	0x00,0xBC,0x52,0xD0,0x1E,0x20,0xB8,0x28,0x55,0x40,0x00,0x00,0x00,0x00,0x00,0x1E,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x9A
};
#else
unsigned char EDID_Sample[]=
{
    0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x4C,0x2D,0x77,0x04,0x00,0x00,0x00,0x00,
    0x17,0x12,0x01,0x03,0x80,0xA0,0x5A,0x78,0x0A,0xEF,0x9B,0xA3,0x54,0x4C,0x9E,0x26,
    0x0F,0x4A,0x4C,0xBF,0xEF,0x00,0xA9,0x40,0x95,0x0F,0x95,0x00,0x90,0x40,0x81,0x80,
    0x81,0x40,0x81,0x0F,0x71,0x4F,0x64,0x19,0x00,0x40,0x41,0x00,0x26,0x30,0x18,0x88,
    0x36,0x00,0x40,0xB0,0x64,0x00,0x00,0x18,0x01,0x1D,0x00,0x72,0x51,0xD0,0x1E,0x20,
    0x6E,0x28,0x55,0x00,0x40,0xB0,0x64,0x00,0x00,0x1E,0x00,0x00,0x00,0xFD,0x00,0x32,
    0x4B,0x1C,0x51,0x11,0x00,0x0A,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0xFC,
    0x00,0x53,0x41,0x4D,0x53,0x55,0x4E,0x47,0x0A,0x20,0x20,0x20,0x20,0x20,0x01,0x98,
    0x02,0x03,0x1A,0xF1,0x46,0x84,0x13,0x05,0x14,0x02,0x11,0x23,0x09,0x07,0x07,0x83,
    0x01,0x00,0x00,0x66,0x03,0x0C,0x00,0x10,0x00,0x80,0x01,0x1D,0x00,0xBC,0x52,0xD0,
    0x1E,0x20,0xB8,0x28,0x55,0x40,0x40,0xB0,0x64,0x00,0x00,0x1E,0x01,0x1D,0x80,0x18,
    0x71,0x1C,0x16,0x20,0x58,0x2C,0x25,0x00,0x40,0xB0,0x64,0x00,0x00,0x9E,0x01,0x1D,
    0x80,0xD0,0x72,0x1C,0x16,0x20,0x10,0x2C,0x25,0x80,0x40,0xB0,0x64,0x00,0x00,0x9E,
    0x8C,0x0A,0xD0,0x8A,0x20,0xE0,0x2D,0x10,0x10,0x3E,0x96,0x00,0x90,0x2C,0x11,0x00,
    0x00,0x18,0x8C,0x0A,0xD0,0x90,0x20,0x40,0x31,0x20,0x0C,0x40,0x55,0x00,0x90,0x2C,
    0x11,0x00,0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC7
};

unsigned char EDID_Sample[]=
{
    0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x4C,0x2D,0x33,0x05,0x01,0x00,0x00,0x00,
    0x02,0x13,0x01,0x03,0x80,0x10,0x09,0x78,0x0A,0xEE,0x91,0xA3,0x54,0x4C,0x99,0x26,
    0x0F,0x50,0x54,0xBD,0xEF,0x80,0x71,0x4F,0x81,0x00,0x81,0x40,0x81,0x80,0x95,0x00,
    0x95,0x0F,0xB3,0x00,0x01,0x01,0x02,0x3A,0x80,0x18,0x71,0x38,0x2D,0x40,0x58,0x2C,
    0x45,0x00,0xA0,0x5A,0x00,0x00,0x00,0x1E,0x66,0x21,0x50,0xB0,0x51,0x00,0x1B,0x30,
    0x40,0x70,0x36,0x00,0xA0,0x5A,0x00,0x00,0x00,0x1E,0x00,0x00,0x00,0xFD,0x00,0x18,
    0x4B,0x1A,0x51,0x17,0x00,0x0A,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0xFC,
    0x00,0x53,0x41,0x4D,0x53,0x55,0x4E,0x47,0x0A,0x20,0x20,0x20,0x20,0x20,0x01,0x72,
    0x02,0x01,0x24,0x01,0x40,0x80,0x00,0x00,0x03,0x00,0x22,0x00,0x00,0x00,0x03,0x10,
    0x07,0x00,0x82,0x00,0x00,0x00,0xE0,0x00,0x0F,0x01,0x04,0x01,0x00,0x20,0x03,0x00,
    0x00,0x00,0x00,0x10,0x00,0x00,0x01,0x00,0x02,0x01,0xC0,0x00,0x00,0x20,0x20,0x51,
    0x00,0x00,0x4A,0x00,0x00,0x00,0x02,0x00,0x0D,0x00,0x08,0x20,0x00,0x16,0x20,0x10,
    0x0C,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x08,0x0A,0x90,0x02,0x00,0xE0,0x21,
    0x00,0x00,0x2E,0x00,0x00,0xA0,0x42,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x51
};
#endif

/**
 * Read EDID Block(128 bytes)
 * @param   blockNum    [in]    Number of block to read
 * @param   outBuffer   [out]   Pointer to buffer to store EDID data
 * @return  If fail to read, return 0;Otherwise, return 1
 */
static int ReadEDIDBlock(const unsigned int blockNum, unsigned char* outBuffer)
{
    int segNum, offset, dataPtr, res, loop = 0;
	//unsigned int edid_block_num;

	FPRINTF("%s blockNum:%d", __func__,blockNum);
    // check parameter
    if (outBuffer == NULL){
        ALOGE("invalid parameter : outBuffer\n");
        return 0;
    }

    // calculate
    segNum = blockNum / 2;
    offset = (blockNum % 2) * SIZEOFEDIDBLOCK;
    dataPtr = (blockNum) * SIZEOFEDIDBLOCK;

    // read block
    if ( !EDDCRead(EDID_SEGMENT_POINTER, segNum, EDID_ADDR, offset, SIZEOFEDIDBLOCK, outBuffer) )
    {
        ALOGE("Fail to Read %dth EDID Block \n", blockNum);
        return 0;
    }

//	edid_block_num = blockNum;
//	memcpy(outBuffer, &EDID_Sample[edid_block_num*128], 128);

    // print data
#if EDID_DEBUG
    offset = 0;

    do
    {
    
        DPRINTF("0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X ", 
				outBuffer[offset], outBuffer[offset+1], outBuffer[offset+2], outBuffer[offset+3], outBuffer[offset+4], outBuffer[offset+5], outBuffer[offset+6], outBuffer[offset+7],
				outBuffer[offset+8], outBuffer[offset+9], outBuffer[offset+10], outBuffer[offset+11], outBuffer[offset+12], outBuffer[offset+13], outBuffer[offset+14], outBuffer[offset+15]
				);
		offset = offset + 16;
    } while (SIZEOFEDIDBLOCK > offset);
#endif

    res = CalcChecksum(outBuffer, SIZEOFEDIDBLOCK);
    if (res)
    {
        char temp[128];
        memset(temp,0xFF,EDID_CHK_LENGTH);
        if(memcmp(temp, outBuffer, EDID_CHK_LENGTH) == 0 )
        {
            DPRINTF("This Block is not EDID. It's Invalid..\n");
			return 0;
        }
        DPRINTF("CheckSum : 0x%2X\n, Anyway continue..", res);
        return 1;
    }
    else
        DPRINTF("%dth EDID Block CheckSum:0x%2X\n",res);

    return 1;
}

/**
 * Check if EDID data is valid or not.
 * @return if EDID data is valid, return 1;otherwise, return 0
 */
static inline int EDIDValid(void)
{
    return (gEdidData == NULL) ?  0 : 1;
}



/**
 * Search Vender Specific Data Block(VSDB) in EDID extension block.
 * @param   extension   [in] the number of EDID extension block to check
 * @return  if there is a VSDB, return the its offset from start of @n
 *          EDID extension block. if there is no VSDB, return 0.
 */
static int GetVSDBOffset(const int extension)
{
    unsigned int BlockOffset = extension*SIZEOFEDIDBLOCK;
    unsigned int offset = BlockOffset + EDID_DATA_BLOCK_START_POS;
    unsigned int tag,blockLen,DTDOffset;

	FPRINTF("%s extension:%d, gExtensions:%d", __func__,extension, gExtensions);

    if (!EDIDValid() || (extension > gExtensions) ){
        DPRINTF("EDID Data is not available\n");
        return 0;
    }

    DTDOffset = gEdidData[BlockOffset + EDID_DETAILED_TIMING_OFFSET_POS];

    // check if there is HDMI VSDB
    while ( offset < BlockOffset + DTDOffset )
    {
        // find the block tag and length
        // tag
        tag = gEdidData[offset] & EDID_TAG_CODE_MASK;
        // block len
        blockLen = (gEdidData[offset] & EDID_DATA_BLOCK_SIZE_MASK) + 1;

        // check if it is HDMI VSDB
        // if so, check identifier value, if it's hdmi vsbd - return offset
        if (tag == EDID_VSDB_TAG_VAL &&
            gEdidData[offset+1] == 0x03 &&
            gEdidData[offset+2] == 0x0C &&
            gEdidData[offset+3] == 0x0 &&
            blockLen > EDID_VSDB_MIN_LENGTH_VAL )
        {
            return offset;
        }
        // else find next block
        offset += blockLen;
    } // while()

    // return error
    return 0;
}

/**
 * Check if EDID supports the HDMI mode.
 * @return  If EDID supports HDMI mode, return 1;Otherwise, return 0.
 */
static int CheckHDMIMode(void)
{
    int index;

	FPRINTF("%s gExtensions:%d", __func__, gExtensions);

    // read EDID
    if(!EDIDRead())   {
    	DPRINTF("%d EDID Read Fail!!!\n",__LINE__);
        return 0;
    }

    // find VSDB
    for ( index = 1 ; index <= gExtensions ; index++ )
    {
        if (GetVSDBOffset(index) > 0) // if there is a VSDB, it means RX support HDMI mode
            return 1;
    }
    return 0;
}


/**
 * Check if EDID extension block is timing extension block or not.
 * @param   extension   [in] The number of EDID extension block to check
 * @return  If the block is timing extension, return 1;Otherwise, return 0
 */
static int IsTimingExtension(const int extension)
{
	int ret = 0;

	FPRINTF("%s extension:%d", __func__,extension);

	if (!EDIDValid() || (extension > gExtensions) ){
		FPRINTF("EDID Data is not available\n");
		return ret;
	}

	if (gEdidData[extension*SIZEOFEDIDBLOCK] == EDID_TIMING_EXT_TAG_VAL )
	{
		// check extension revsion number
		// revision num == 3
		if (gEdidData[extension*SIZEOFEDIDBLOCK + EDID_TIMING_EXT_REV_NUMBER_POS] == 3)
			ret = 1;
		// revison num != 3 && DVI mode 
		else if ( !CheckHDMIMode() && 
			gEdidData[extension*SIZEOFEDIDBLOCK + EDID_TIMING_EXT_REV_NUMBER_POS] != 2 )
			ret = 1;
	}

	return ret;
}



/**
 * Check if the video format is contained in - @n
 * Detailed Timing Descriptor(DTD) of EDID extension block.
 * @param   extension   [in]    Number of EDID extension block to check
 * @param   videoFormat [in]    Video format to check
 * @return  If the video format is contained in DTD of EDID extension block, -@n
 *          return 1;Otherwise, return 0.
 */
static int IsContainVideoDTD(const int extension,const enum VideoFormat videoFormat)
{
    int i=0,cnt,StartAddr,EndAddr;
#if 1
    int hblank = 0, hactive = 0, vblank = 0, vactive = 0, interlaced = 0, pixelclock = 0;
    int vHActive = 0, vVActive = 0;

    TIMINGDESCRIPTOR* pTMG;
    
	FPRINTF("%s extension:%d, videoFormat:%d", __func__,extension, videoFormat);
    if (!EDIDValid() || (extension > gEDID.ExtensionEDID) ){
        DPRINTF("EDID Data is not available\n");
        return 0;
    }

    if (extension == 0)
    {
        cnt = 4;
        pTMG = p1stTMG;
    }
    else
    {
        cnt = gCEA.NumofDTDs;
        pTMG = pETMG;
    }


    while(i<cnt)
        {
        vHActive = (aVideoParams[videoFormat].vHVLine & 0xFFF000) >> 12;
        vHActive -= aVideoParams[videoFormat].vHBlank;
        vVActive = aVideoParams[videoFormat].vVBlank&0x7FF;
        vVActive -= (aVideoParams[videoFormat].vVBlank) >> 11;
        pixelclock = aVideoParams[videoFormat].PixelClock;

        if(pTMG[i].PixelClock) // Block has pixelclock info
        {
            hactive = pTMG[i].HActive;
            vactive = pTMG[i].VActive;
            hblank = pTMG[i].HBlanking;
            vblank = pTMG[i].VBlanking;

            if (hblank == aVideoParams[videoFormat].vHBlank && 
                vblank == (aVideoParams[videoFormat].vVBlank>>11) && 
                hactive == vHActive && 
                vactive == vVActive &&
                pixelclock == pTMG[i].PixelClock)
            {
                DPRINTF("Sink Support the Video mode\n");
                return 1;
            }
        }
		i++;
    }

#else
    if (!EDIDValid() || (extension > gExtensions) ){
        DPRINTF("EDID Data is not available\n");
        return 0;
    }

    // if edid block( 0th block )
    if (extension == 0)
    {
        StartAddr = EDID_DTD_START_ADDR;
        EndAddr = StartAddr + EDID_DTD_TOTAL_LENGTH;
    }
    else // if edid extension block
    {
        StartAddr = gEdidData[extension*SIZEOFEDIDBLOCK + EDID_DETAILED_TIMING_OFFSET_POS];
        EndAddr = gEdidData[(extension+1)*SIZEOFEDIDBLOCK];
    }

    // check DTD(Detailed Timing Description)
    for (i = StartAddr; i < EndAddr; i+= EDID_DTD_BYTE_LENGTH)
    {
        int hblank = 0, hactive = 0, vblank = 0, vactive = 0, interlaced = 0, pixelclock = 0;
        int vHActive = 0, vVActive = 0;

        // get pixel clock
        pixelclock = (gEdidData[i+EDID_DTD_PIXELCLOCK_POS2] << SIZEOFBYTE);
        pixelclock |= gEdidData[i+EDID_DTD_PIXELCLOCK_POS1];

        if (!pixelclock)
        {
            continue;
        }

        // get HBLANK value in pixels
        hblank = gEdidData[i+EDID_DTD_HBLANK_POS2] & EDID_DTD_HBLANK_POS2_MASK;
        hblank <<= SIZEOFBYTE; // lower 4 bits
        hblank |= gEdidData[i+EDID_DTD_HBLANK_POS1];

        // get HACTIVE value in pixels
        hactive = gEdidData[i+EDID_DTD_HACTIVE_POS2] & EDID_DTD_HACTIVE_POS2_MASK;
        hactive <<= (SIZEOFBYTE/2); // upper 4 bits
        hactive |= gEdidData[i+EDID_DTD_HACTIVE_POS1];

        // get VBLANK value in pixels
        vblank = gEdidData[i+EDID_DTD_VBLANK_POS2] & EDID_DTD_VBLANK_POS2_MASK;
        vblank <<= SIZEOFBYTE; // lower 4 bits
        vblank |= gEdidData[i+EDID_DTD_VBLANK_POS1];

        // get VACTIVE value in pixels
        vactive = gEdidData[i+EDID_DTD_VACTIVE_POS2] & EDID_DTD_VACTIVE_POS2_MASK;
        vactive <<= (SIZEOFBYTE/2); // upper 4 bits
        vactive |= gEdidData[i+EDID_DTD_VACTIVE_POS1];

        vHActive = (aVideoParams[videoFormat].vHVLine & 0xFFF) - aVideoParams[videoFormat].vHBlank;
        vVActive = (aVideoParams[videoFormat].vHVLine & 0xFFF000) >> 12;
        vVActive -= aVideoParams[videoFormat].vVBlank;


        // get Interlaced Mode Value
        interlaced = (int)(gEdidData[i+EDID_DTD_INTERLACE_POS] & EDID_DTD_INTERLACE_MASK);
        if (interlaced) interlaced = 1;

        if (hblank == aVideoParams[videoFormat].vHBlank && vblank == aVideoParams[videoFormat].vHBlank // blank
            && hactive == vHActive && vactive == vVActive ) //line
        {
            int EDIDpixelclock = aVideoParams[videoFormat].PixelClock;
            EDIDpixelclock /= 100; pixelclock /= 100;

            if (pixelclock == EDIDpixelclock)
            {
                DPRINTF("Sink Support the Video mode\n");
                return 1;
            }
        }
    } // for
#endif
    return 0;
}

/**
 * Check if a VIC(Video Identification Code) is contained in -@n
 * EDID extension block.
 * @param   extension   [in]    Number of EDID extension block to check
 * @param   VIC         [in]    VIC to check
 * @return  If the VIC is contained in contained in EDID extension block, -@n
 *          return 1;otherwise, Return 0.
 */
static int IsContainVIC(const int extension, const int VIC)
{
#if 1
	int cnt, i=0;
	if(extension == 0)
	{
	}
	else
	{
		cnt = gCEA.VDB.BlockHead.B.ByteNumber;
	}

	FPRINTF("%s extension:%d, VIC:%d", __func__,extension, VIC);

	while(i<cnt)
	{
		DPRINTF("EDID VIC : %d\n",gCEA.VDB.ShortVideoDscrpt[i].B.index);
		
		if( VIC == gCEA.VDB.ShortVideoDscrpt[i].B.index)
		{
			DPRINTF("Sink Device supports requested video mode\n");
			return 1;
		}
		i++;
	}

#else

    unsigned int StartAddr = extension*SIZEOFEDIDBLOCK;
    unsigned int ExtAddr = StartAddr + EDID_DATA_BLOCK_START_POS;
    unsigned int tag,blockLen;
    unsigned int DTDStartAddr = gEdidData[StartAddr + EDID_DETAILED_TIMING_OFFSET_POS];

    if (!EDIDValid() || (extension > gExtensions)){
        DPRINTF("EDID Data is not available\n");
        return 0;
    }

    // while
    while ( ExtAddr < StartAddr + DTDStartAddr )
    {
        // find the block tag and length
        // tag
        tag = gEdidData[ExtAddr] & EDID_TAG_CODE_MASK;
        // block len
        blockLen = (gEdidData[ExtAddr] & EDID_DATA_BLOCK_SIZE_MASK) + 1;
        DPRINTF("tag = %d\n",tag);
        DPRINTF("blockLen = %d\n",blockLen-1);

        // check if it is short video description
        if (tag == EDID_SHORT_VID_DEC_TAG_VAL)
        {
            // if so, check SVD
            int index;
            for (index=1; index < blockLen;index++)
            {
                DPRINTF("EDIDVIC = %d\n",gEdidData[ExtAddr+index] & EDID_SVD_VIC_MASK);
                DPRINTF("VIC = %d\n",VIC);

                // check VIC with SVDB
                if (VIC == (gEdidData[ExtAddr+index] & EDID_SVD_VIC_MASK) )
                {
                    DPRINTF("Sink Device supports requested video mode\n");
                    return 1;
                }
            } // for
        } // if tag
        // else find next block
        ExtAddr += blockLen;
    } // while()
#endif//

    return 0;
}

/**
 * Check if EDID contains the video format.
 * @param   videoFormat [in]    Video format to check
 * @param   pixelRatio  [in]    Pixel aspect ratio of video format to check
 * @return  if EDID contains the video format, return 1;Otherwise, return 0.
 */
int CheckResolution(const enum VideoFormat videoFormat,
                            const enum PixelAspectRatio pixelRatio)
{
    int index,vic;

	FPRINTF("%s videoFormat:%d, pixelRatio:%d", __func__,videoFormat, pixelRatio);

    // read EDID
    if(!EDIDRead())    {
		DPRINTF("%d EDID Read Fail!!!\n",__LINE__);
        return 0;
    }

    // check ET(Established Timings) for 640x480p@60Hz
    if ( videoFormat == v640x480p_60Hz // if it's 640x480p@60Hz
        && gEDID.ESTiming1.B.est640480a60 == 1) // it support
    {
         return 1;
    }

    // check STI(Standard Timing Identification)
    // do not need

    // check DTD(Detailed Timing Description) of EDID block(0th)
    if (IsContainVideoDTD(0,videoFormat))
    {
        return 1;
    }

    // check EDID Extension
    vic = (pixelRatio == HDMI_PIXEL_RATIO_16_9) ? aVideoParams[videoFormat].VIC16_9 : aVideoParams[videoFormat].VIC;

    // find VSDB
    for ( index = 1 ; index <= gExtensions ; index++ )
    {
        if ( IsTimingExtension(index) ) // if it's timing block
        {
            if (IsContainVIC(index,vic) || IsContainVideoDTD(index,videoFormat))
                return 1;
        }
    }

    return 0;
}


/**
 * Check if EDID contains the video format.
 * @param   videoFormat [in]    Video format to check
 * @param   pixelRatio  [in]    Pixel aspect ratio of video format to check
 * @return  if EDID contains the video format, return 1;Otherwise, return 0.
 */
int CheckResolutionOfDVI(const enum VideoFormat videoFormat,
                            const enum PixelAspectRatio pixelRatio, unsigned int width, unsigned int height, unsigned frame_hz)
{
    int index,vic;

	unsigned int hres = 0, vres = 0, freq = 0;
	
	FPRINTF("%s videoFormat:%d, pixelRatio:%d, width:%d, height:%d, frame_hz:%d", __func__,videoFormat, pixelRatio, width, height, frame_hz);

    // read EDID
    if(!EDIDRead())    {
		DPRINTF("%d EDID Read Fail!!!\n",__LINE__);
        return 0;
    }

	// In DVI mode, 1920x1080i 60Hz is not supported.
	if( videoFormat == v1920x1080i_60Hz )
	{
		return 0;
	}

    // check ET(Established Timings) for 640x480p@60Hz
    if ( videoFormat == v640x480p_60Hz // if it's 640x480p@60Hz
        && gEDID.ESTiming1.B.est640480a60 == 1) // it support
    {
         return 1;
    }

    // check DTD(Detailed Timing Description) of EDID block(0th)
    if (IsContainVideoDTD(0,videoFormat))
    {
        return 1;
    }

    // check EDID Extension
    vic = (pixelRatio == HDMI_PIXEL_RATIO_16_9) ? aVideoParams[videoFormat].VIC16_9 : aVideoParams[videoFormat].VIC;

    // find VSDB
    for ( index = 1 ; index <= gExtensions ; index++ )
    {
        if ( IsTimingExtension(index) ) // if it's timing block
        {
            if (IsContainVIC(index,vic) || IsContainVideoDTD(index,videoFormat))
                return 1;
        }
    }

    // Check Standard Timing
    for(index=0;index<8;index++)
    {
		if( gEDID.STDTIMING[index].Hresolution != 0x1)
		{
			hres = gEDID.STDTIMING[index].Hresolution*8+248;
			freq = gEDID.STDTIMING[index].B.VFreq+60;

			if( pixelRatio == HDMI_PIXEL_RATIO_4_3 )
			{
				if( gEDID.STDTIMING[index].B.AspectRatio == 1 )	// 4:3
					vres = (9*hres)/16;

				FPRINTF("%s videoFormat:%d, pixelRatio:%d, hres:%d, vres:%d, freq:%d", __func__,videoFormat, pixelRatio, hres, vres, freq);

				if( width == hres && height == vres && frame_hz == freq )
					return 1;
					
			}
			else if( pixelRatio == HDMI_PIXEL_RATIO_16_9 )
			{
				if( gEDID.STDTIMING[index].B.AspectRatio == 3 )	// 16:9
					vres = (9*hres)/16;

				FPRINTF("%s videoFormat:%d, pixelRatio:%d, hres:%d, vres:%d, freq:%d", __func__,videoFormat, pixelRatio, hres, vres, freq);

				if( width == hres && height == vres && frame_hz == freq )
					return 1;
			}
			else
			{
				//Do nothing...
			}
			
		}    
    
    }
	
	//Check Detailed Timing Descriptor Block.
	for( index=0;index<4;index++)
	{
		if( p1stTMG[index].PixelClock != 0 )
		{
			if( hres < p1stTMG[index].HActive )
				hres = p1stTMG[index].HActive;
			if( vres < p1stTMG[index].VActive )
				vres = p1stTMG[index].VActive;
		}
	}

	if( hres >= width && vres >= height )
	{
		ALOGE("%s : hres = %d, vres = %d, width = %d, height = %d ", __func__, hres, vres, width, height);
		return 1;
	}
	

	return 0;
}


/**
 * Check if EDID supports the color depth.
 * @param   depth [in]    Color depth
 * @param   space [in]    Color space
 * @return  If EDID supports the color depth, return 1;Otherwise, return 0.
 */
static int CheckColorDepth(const enum ColorDepth depth,const enum ColorSpace space)
{
    int index;
    unsigned int StartAddr;
	
	FPRINTF("%s depth:%d, space:%d", __func__,depth, space);

    // if color depth == 24 bit, no need to check
    if ( depth == HDMI_CD_24 )
        return 1;

    // check EDID data is valid or not
    // read EDID
    if(!EDIDRead())    {
        DPRINTF("%d EDID Read Fail!!!\n",__LINE__);
        return 0;
    }

    // find VSDB
    for ( index = 1 ; index <= gExtensions ; index++ )
    {
        if ( IsTimingExtension(index) // if it's timing block
            && ((StartAddr = GetVSDBOffset(index)) > 0) )   // check block
        {
            int blockLength = gEdidData[StartAddr] & EDID_DATA_BLOCK_SIZE_MASK;
            if (blockLength >= EDID_DC_POS)
            {
                // get supported DC value
                int deepColor = gEdidData[StartAddr + EDID_DC_POS] & EDID_DC_MASK;
                DPRINTF("EDID deepColor = %x\n",deepColor);
                // check supported DeepColor

                // if YCBCR444
                if (space == HDMI_CS_YCBCR444)
                {
                    if ( !(deepColor & EDID_DC_YCBCR_VAL))
                        return 0;
                }

                // check colorDepth
                switch (depth)
                {
                    case HDMI_CD_36:
                    {
                        deepColor &= EDID_DC_36_VAL;
                        break;
                    }
                    case HDMI_CD_30:
                    {
                        deepColor &= EDID_DC_30_VAL;
                        break;
                    }
                    case HDMI_CD_24:
                    {
                        deepColor = 1;
                        break;
                    }
                    default :
                        deepColor = 0;
                } // switch

                if (deepColor)
                    return 1;
                else
                    return 0;
            } // if
        } // if
    } // for

    return 0;
}

/**
 * Check if EDID supports the color space.
 * @param   space [in]    Color space
 * @return  If EDID supports the color space, return 1;Otherwise, return 0.
 */
const enum ColorSpace CheckColorSpaceWithEDID(void)
{
    int index;
	enum ColorSpace  space;

    // RGB is default
    space = HDMI_CS_RGB;

    // check EDID data is valid or not
    // read EDID
    if(!EDIDRead())    {
		DPRINTF("%d EDID Read Fail!!!\n",__LINE__);
        return space;
    }

    // find VSDB
    for ( index = 1 ; index <= gExtensions ; index++ )
    {
        if (IsTimingExtension(index))  // if it's timing block
        {
            // read Color Space
            int CS = gEdidData[index*SIZEOFEDIDBLOCK + EDID_COLOR_SPACE_POS];

            if (CS & EDID_YCBCR422_CS_MASK && CS & EDID_YCBCR444_CS_MASK)
			{
				space = HDMI_CS_YCBCR444;
				DPRINTF("%s  support color space : HDMI_CS_YCBCR444 and HDMI_CS_YCBCR422", __func__);
			}
			else if(CS & EDID_YCBCR444_CS_MASK) // YCBCR444
            {
                space = HDMI_CS_YCBCR444;
				DPRINTF("%s  support color space : HDMI_CS_YCBCR444", __func__);
            } 
			else if(CS & EDID_YCBCR422_CS_MASK) // YCBCR422
            {
                space = HDMI_CS_YCBCR422;
				DPRINTF("%s  support color space : HDMI_CS_YCBCR422", __func__);
            } 
			else
			{
			    // RGB is default
			    space = HDMI_CS_RGB;
				DPRINTF("%s  support color space : HDMI_CS_RGB", __func__);
			}
        } // if
    } // for

	DPRINTF("%s  space:%d", __func__, space);

    return space;
}


/**
 * Check if EDID supports the color space.
 * @param   space [in]    Color space
 * @return  If EDID supports the color space, return 1;Otherwise, return 0.
 */
int CheckColorSpace(const enum ColorSpace space)
{
    int index;

	FPRINTF("%s  space:%d", __func__, space);

    // RGB is default
    if (space == HDMI_CS_RGB)
        return 1;

    // check EDID data is valid or not
    // read EDID
    if(!EDIDRead())    {
		DPRINTF("%d EDID Read Fail!!!\n",__LINE__);
        return 0;
    }

    // find VSDB
    for ( index = 1 ; index <= gExtensions ; index++ )
    {
        if (IsTimingExtension(index))  // if it's timing block
        {
            // read Color Space
            int CS = gEdidData[index*SIZEOFEDIDBLOCK + EDID_COLOR_SPACE_POS];

            if ( (space == HDMI_CS_YCBCR444 && (CS & EDID_YCBCR444_CS_MASK)) || // YCBCR444
                    (space == HDMI_CS_YCBCR422 && (CS & EDID_YCBCR422_CS_MASK)) ) // YCBCR422
            {
                return 1;
            }
        } // if
    } // for
    return 0;
}

/**
 * Check if EDID supports the colorimetry.
 * @param   color [in]    Colorimetry
 * @return  If EDID supports the colorimetry, return 1;Otherwise, return 0.
 */
static int CheckColorimetry(const enum HDMIColorimetry color)
{
    int index;

	FPRINTF("%s  color:%d", __func__, color);

    // do not need to parse if not extended colorimetry
    if (color == HDMI_COLORIMETRY_NO_DATA || color == HDMI_COLORIMETRY_ITU601 || color == HDMI_COLORIMETRY_ITU709)
        return 1;

    // read EDID
    if(!EDIDRead())
    {
       return 0;
    }

    // find VSDB
    for ( index = 1 ; index <= gExtensions ; index++ )
    {
        if ( IsTimingExtension(index) ) // if it's timing block
        {
            // check address
            unsigned int ExtAddr = index*SIZEOFEDIDBLOCK + EDID_DATA_BLOCK_START_POS;
            unsigned int EndAddr = index*SIZEOFEDIDBLOCK + gEdidData[index*SIZEOFEDIDBLOCK + EDID_DETAILED_TIMING_OFFSET_POS];
            unsigned int tag,blockLen;

            // while
            while ( ExtAddr < EndAddr )
            {
                // find the block tag and length
                // tag
                tag = gEdidData[ExtAddr] & EDID_TAG_CODE_MASK;
                // block len
                blockLen = (gEdidData[ExtAddr] & EDID_DATA_BLOCK_SIZE_MASK) + 1;

                // check if it is colorimetry block
                if (tag == EDID_EXTENDED_TAG_VAL && // extended tag
                    gEdidData[ExtAddr+1] == EDID_EXTENDED_COLORIMETRY_VAL && // colorimetry block
                    (blockLen-1) == EDID_EXTENDED_COLORIMETRY_BLOCK_LEN )// check length
                {
                    // get supported DC value
                    int colorimetry = (gEdidData[ExtAddr + 2]);
                    int metadata = (gEdidData[ExtAddr + 3]);

                    DPRINTF("EDID extened colorimetry = %x\n",colorimetry);
                    DPRINTF("EDID gamut metadata profile = %x\n",metadata);

                    // check colorDepth
                    switch (color)
                    {
                        case HDMI_COLORIMETRY_EXTENDED_xvYCC601:
                            if (colorimetry & EDID_XVYCC601_MASK && metadata)
                                return 1;
                            break;
                        case HDMI_COLORIMETRY_EXTENDED_xvYCC709:
                            if (colorimetry & EDID_XVYCC709_MASK && metadata)
                                return 1;
                            break;
                        default:
                            break;
                    }
                    return 0;
                } // if VSDB block
                // else find next block
                ExtAddr += blockLen;
            } // while()
        } // if
    } // for

    return 0;
}


/**
 * Initialize EDID library. This will intialize DDC library.
 * @return  If success, return 1;Otherwise, return 0.
 */
int EDIDOpen()
{
    if (gEdidData)
        free(gEdidData);
	
    gEdidData=NULL;
    // init DDC
    return DDCOpen();
}

/**
 * Finalize EDID library. This will finalize DDC library.
 * @return  If success, return 1;Otherwise, return 0.
 */
int EDIDClose()
{
    // reset EDID
    EDIDReset();

    // close EDDC
    return DDCClose();
}

int getEDID(unsigned char* outBuffer) {
    if (EDIDValid()) {
        memcpy(outBuffer, gEdidData, 4*SIZEOFEDIDBLOCK);
        return 1;
    } else {
        if (!EDIDOpen()) {
            printf("EDIDInit() failed!\n");
            outBuffer = NULL;
            return -1;
        }

        if (!EDIDRead()) {
	        printf("EDIDRead() failed!\n");
            outBuffer = NULL;
            return -1;
        } else {
            memcpy(outBuffer, gEdidData, 4*SIZEOFEDIDBLOCK);
            return 1;
        }
    }
}

/**
 * Read EDID data of Rx.
 * @return If success, return 1;Otherwise, return 0;
 */
int EDIDRead()
{
    int block,dataPtr, ret;
    unsigned char temp[SIZEOFEDIDBLOCK];
    int extblocknum;
    unsigned char*   ptempbuff;

	FPRINTF("%s ", __func__);

    // if already read??
    if (EDIDValid())
        return 1;

    // read EDID Extension Number
    // read EDID
    if (!ReadEDIDBlock(0,temp))
    {
        return 0;
    }
	
    // get extension
    gExtensions = temp[EDID_EXTENSION_NUMBER_POS];

    DPRINTF("EDID Extension blocks %d...\n", gExtensions);

    if(gExtensions>3)
    {
        DPRINTF("EDID Extension blocks can't be greater than 3...\n");
       gExtensions = 3;
    }

    // prepare buffer
    gEdidData = (unsigned char*)malloc(4*SIZEOFEDIDBLOCK);
    
    if (!gEdidData)
        return 0;

    // copy EDID Block 0
    memcpy(gEdidData,temp,SIZEOFEDIDBLOCK);

    // read EDID Extension
    for ( block = 1,dataPtr = SIZEOFEDIDBLOCK; block <= gExtensions; block++,dataPtr+=SIZEOFEDIDBLOCK )
    {
        // read extension 1~gExtensions
        if (!ReadEDIDBlock(block, gEdidData+dataPtr))
        {
            // reset buffer
            EDIDReset();
            return 0;
        }
        
        if(block == 1)
        {
            if( *(char*)(gEdidData+dataPtr)== 0xF0 )    
            {
                extblocknum = parse_extblockcount(gEdidData+dataPtr);

                //if (extblocknum == gExtensions)
                    gExtensions = extblocknum+1;
            }
            
            ptempbuff = (unsigned char*)malloc( (gExtensions+1)*SIZEOFEDIDBLOCK);
            memcpy(ptempbuff, gEdidData, (SIZEOFEDIDBLOCK *2));
            free(gEdidData);
            gEdidData = ptempbuff;
        }
    }

    // check if extension is more than 1, and first extension block is not block map.
    if (gExtensions > 1 && gEdidData[SIZEOFEDIDBLOCK] != EDID_BLOCK_MAP_EXT_TAG_VAL)
    {
        // reset buffer
        DPRINTF("EDID has more than 1 extension but, first extension block is not block map\n");
        EDIDReset();
        return 0;
    }

    ret = EDID_parsing(1);

    return ret;
}

#if defined(TELECHIPS)
int EDIDReadBlock(const unsigned int blockNum, unsigned char* const outBuffer)
{
    // if already read??
    if (EDIDValid())
        return 1;

    // read EDID Extension Number
    // read EDID
    if (!ReadEDIDBlock(blockNum,outBuffer))
    {
        return 0;
    }

    return 1;
}
#endif

int Read_EEDIDBlock(unsigned int blockNum, unsigned char* pBuf)
{
    int segNum, offset, dataPtr;
    
	if( pBuf == NULL )
	{
        DPRINTF("Read_EEDIDBlock : Inavalid Parameter\n");
		return 0; // Invalid parameter
	}
    
    // calculate
    segNum = blockNum / 2;
    offset = (blockNum % 2) * SIZEOFEDIDBLOCK;
    dataPtr = (blockNum) * SIZEOFEDIDBLOCK;
    
    // read block
    if ( !EDDCRead(EDID_SEGMENT_POINTER, segNum, EDID_ADDR, offset, SIZEOFEDIDBLOCK, pBuf) )
    {
        DPRINTF("Fail to Read %dth EDID Block\n", blockNum);
        return 0;
    }
    
    memcpy(pBuf, &EDID_Sample[blockNum*128], 128);
    
    // print data
    offset = 0;

    do
    {
    
        DPRINTF("0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X ", 
				pBuf[offset], pBuf[offset+1], pBuf[offset+2], pBuf[offset+3], pBuf[offset+4], pBuf[offset+5], pBuf[offset+6], pBuf[offset+7],
				pBuf[offset+8], pBuf[offset+9], pBuf[offset+10], pBuf[offset+11], pBuf[offset+12], pBuf[offset+13], pBuf[offset+14], pBuf[offset+15]
				);
		offset = offset + 16;
    } while (SIZEOFEDIDBLOCK > offset);

/*    
    if (!CalcChecksum(pBuf, SIZEOFEDIDBLOCK))
    {
        DPRINTF("CheckSum fail : %dth EDID Block\n", blockNum);
        return 0
    }
*/   
	return 0;
}
/**
 * Reset stored EDID data.
 */
void EDIDReset()
{
    if (gEdidData)
    {
        free(gEdidData);
        gEdidData = NULL;
        DPRINTF("                                   EDID is reset!!!\n");
    }


    memset(&gEDID, 0x0, sizeof(stEDID));
    memset(&gCEA, 0x0, sizeof(stCEABlock));

    if (p1stTMG)
   	{
    	free(p1stTMG);
    	p1stTMG = NULL;
    }
    if (pETMG)
    {
    	free(pETMG);
    	pETMG = NULL;
    }
}

/**
 * Get CEC physical address.
 * @param   outAddr [out]   CEC physical address. LSB 2 bytes is available. [0:0:AB:CD]
 * @return  If success, return 1;Otherwise, return 0.
 */
int EDIDGetCECPhysicalAddress(int* const outAddr)
{
    int index;
    unsigned int StartAddr;

    if(!EDIDRead())    {
		DPRINTF("EDID Read Fail!!!\n");
        return 0;
    }

    // find VSDB
    for ( index = 1 ; index <= gExtensions ; index++ )
    {
        if ( IsTimingExtension(index) // if it's timing block
            && (StartAddr = GetVSDBOffset(index)) > 0 )   // check block
        {
            // get supported DC value
            // int tempDC1 = (int)(gEdidData[tempAddr+EDID_DC_POS]);
            int phyAddr = gEdidData[StartAddr + EDID_CEC_PHYICAL_ADDR] << 8;
            phyAddr |= gEdidData[StartAddr + EDID_CEC_PHYICAL_ADDR+1];

            DPRINTF("phyAddr = %x\n",phyAddr);

            *outAddr = phyAddr;

            return 1;
        } // if
    } // for

    return 0;
}

/**
 * Check if Rx supports HDMI/DVI mode or not.
 * @param   video [in]   HDMI or DVI mode to check
 * @return  If Rx supports requested mode, return 1;Otherwise, return 0.
 */
int EDIDHDMIModeSupport(const struct HDMIVideoParameter * const video)
{
    // check if read edid?
    if (!EDIDRead())    {
        DPRINTF("%d EDID Read Fail!!!\n",__LINE__);
        return 0;
    }

    // check hdmi mode
    if (video->mode == HDMI)
    {
        if ( !CheckHDMIMode() )
        {
            DPRINTF("HDMI mode Not Supported\n");
            return 0;
        }
    }
    return 1;
}

/**
 * Check if Rx supports HDMI/DVI mode or not.
 * @param   video [in]   HDMI or DVI mode to check
 * @return  If Rx supports requested mode, return 1;Otherwise, return 0.
 */
int EDIDHDMIModeSupportForCTS(const struct HDMIVideoParameter * const video)
{
    // check if read edid?
    if (!EDIDRead())    {
        DPRINTF("%d EDID Read Fail!!!\n",__LINE__);
        return 0;
    }

    // check hdmi mode
	if ( !CheckHDMIMode() )
	{
		DPRINTF("HDMI mode Not Supported\n");
		return 0;
	}

    return 1;
}


/**
 * Check if Rx supports requested video parameters or not.
 * @param   video [in]   Video parameters to check
 * @return  If Rx supports video parameters, return 1;Otherwise, return 0.
 */
int EDIDVideoModeSupport(const struct HDMIVideoParameter * const video)
{
	int ret=1;
	
    // check if read edid?
    if (!EDIDRead())
    {
        DPRINTF("EDID Read Fail!!!\n");
        ret = 0;
    }

    // check resolution
    if ( !CheckResolution(video->resolution,video->pixelAspectRatio))
    {
        DPRINTF("Video Resolution Not Supported\n");
        ret = 0;
    }

    // check color space
    if ( !CheckColorSpace(video->colorSpace) )
    {
        DPRINTF("Color Space Not Supported\n");
        ret = 0;
    }

    // check color depth
    if ( !CheckColorDepth(video->colorDepth,video->colorSpace) )
    {
        DPRINTF("Color Depth Not Supported\n");
        ret = 0;
    }

    // check color depth
    if ( !CheckColorimetry(video->colorimetry) )
    {
        DPRINTF("Colorimetry Not Supported\n");
        ret = 0;
    }

    return ret;
}

/**
 * Check if Rx supports requested audio parameters or not.
 * @param   audio [in]   Audio parameters to check
 * @return  If Rx supports audio parameters, return 1;Otherwise, return 0.
 */
int EDIDAudioModeSupport(const struct HDMIAudioParameter * const audio)
{
    int index;

    // read EDID
    if(!EDIDRead())
    {
        DPRINTF("EDID Read Fail!!!\n");
        return 0;
    }

    // check EDID Extension

    // find timing block
    for ( index = 1 ; index <= gExtensions ; index++ )
    {
        if ( IsTimingExtension(index) ) // if it's timing block
        {
            // find Short Audio Description
            unsigned int StartAddr = index*SIZEOFEDIDBLOCK;
            unsigned int ExtAddr = StartAddr + EDID_DATA_BLOCK_START_POS;
            unsigned int tag,blockLen;
            unsigned int DTDStartAddr = gEdidData[StartAddr + EDID_DETAILED_TIMING_OFFSET_POS];

            // while
            while ( ExtAddr < StartAddr + DTDStartAddr )
            {
                // find the block tag and length
                // tag
                tag = gEdidData[ExtAddr] & EDID_TAG_CODE_MASK;
                // block len
                blockLen = (gEdidData[ExtAddr] & EDID_DATA_BLOCK_SIZE_MASK) + 1;

                DPRINTF("tag = %d\n",tag);
                DPRINTF("blockLen = %d\n",blockLen-1);

                // check if it is short video description
                if (tag == EDID_SHORT_AUD_DEC_TAG_VAL)
                {
                    // if so, check SAD
                    int i;
                    int audioFormat,channelNum,sampleFreq,wordLen;
                    for (i=1; i < blockLen; i+=3)
                    {
                        audioFormat = gEdidData[ExtAddr+i] & EDID_SAD_CODE_MASK;
                        channelNum = gEdidData[ExtAddr+i] & EDID_SAD_CHANNEL_MASK;
                        sampleFreq = gEdidData[ExtAddr+i+1];
                        wordLen = gEdidData[ExtAddr+i+2];

                        DPRINTF("EDIDAudioFormatCode = %d[%x]\n",audioFormat>>3, audioFormat>>3);
                        DPRINTF("EDIDChannelNumber-1= %d[%x]\n",channelNum, channelNum);
                        DPRINTF("EDIDSampleFreq= %d[%x]\n",sampleFreq, sampleFreq);
                        DPRINTF("EDIDWordLeng= %d[%x]\n",wordLen, wordLen);

                        // check parameter
                        // check audioFormat
                        if ( audioFormat == ( (audio->formatCode) << 3)     &&  // format code
                                channelNum >= ( (audio->channelNum) -1)         &&  // channel number
                                ( sampleFreq & (1<<(audio->sampleFreq)) )      ) // sample frequency
                        {
                            if (audioFormat == LPCM_FORMAT) // check wordLen
                            {
                                if ( wordLen & (1<<(audio->wordLength)) )
                                {
									return 1;
                                }
                                else
                                {
                                	return 0;
                                }
                            }
                            return 1; // if not LPCM
                        }
                    } // for
                } // if tag
                // else find next block
                ExtAddr += blockLen;
            } // while()
        } // if
    } // for

    return 0;
}
