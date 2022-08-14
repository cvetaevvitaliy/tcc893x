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
#include <stdlib.h>
#include <string.h>
#include "libedid/libedid.h"

#define EDID_DEBUG      0

#if EDID_DEBUG
#define DPRINTF(args...)    printf(args)
#else
#define DPRINTF(args...)
#endif


typedef unsigned char byte;
/* byte must be 8 bits */

/* int must be at least 16 bits */

/* long must be at least 32 bits */



#define DIE_MSG( x ) \
        { MSG( x ); exit( 1 ); }


#define UPPER_NIBBLE( x ) \
        (((128|64|32|16) & (x)) >> 4)

#define LOWER_NIBBLE( x ) \
        ((1|2|4|8) & (x))

#define COMBINE_HI_8LO( hi, lo ) \
        ( (((unsigned)hi) << 8) | (unsigned)lo )

#define COMBINE_HI_4LO( hi, lo ) \
        ( (((unsigned)hi) << 4) | (unsigned)lo )

const byte edid_v1_header[] = { 0x00, 0xff, 0xff, 0xff,
                                0xff, 0xff, 0xff, 0x00 };

const byte edid_v1_descriptor_flag[] = { 0x00, 0x00 };


#define EDID_LENGTH                             0x80

#define EDID_HEADER                             0x00
#define EDID_HEADER_END                         0x07

#define ID_MANUFACTURER_NAME                    0x08
#define ID_MANUFACTURER_NAME_END                0x09
#define ID_MODEL				0x0a

#define ID_SERIAL_NUMBER			0x0c

#define MANUFACTURE_WEEK			0x10
#define MANUFACTURE_YEAR			0x11

#define EDID_STRUCT_VERSION                     0x12
#define EDID_STRUCT_REVISION                    0x13

#define DPMS_FLAGS				0x18
#define ESTABLISHED_TIMING_1                    0x23
#define ESTABLISHED_TIMING_2                    0x24
#define MANUFACTURERS_TIMINGS                   0x25

#define DETAILED_TIMING_DESCRIPTIONS_START      0x36
#define DETAILED_TIMING_DESCRIPTION_SIZE        18
#define NO_DETAILED_TIMING_DESCRIPTIONS         4



#define DETAILED_TIMING_DESCRIPTION_1           0x36
#define DETAILED_TIMING_DESCRIPTION_2           0x48
#define DETAILED_TIMING_DESCRIPTION_3           0x5a
#define DETAILED_TIMING_DESCRIPTION_4           0x6c



#define PIXEL_CLOCK_LO     (unsigned)dtd[ 0 ]
#define PIXEL_CLOCK_HI     (unsigned)dtd[ 1 ]
#define PIXEL_CLOCK        (COMBINE_HI_8LO( PIXEL_CLOCK_HI,PIXEL_CLOCK_LO )*10000)

#define H_ACTIVE_LO        (unsigned)dtd[ 2 ]

#define H_BLANKING_LO      (unsigned)dtd[ 3 ]

#define H_ACTIVE_HI        UPPER_NIBBLE( (unsigned)dtd[ 4 ] )

#define H_ACTIVE           COMBINE_HI_8LO( H_ACTIVE_HI, H_ACTIVE_LO )

#define H_BLANKING_HI      LOWER_NIBBLE( (unsigned)dtd[ 4 ] )

#define H_BLANKING         COMBINE_HI_8LO( H_BLANKING_HI, H_BLANKING_LO )




#define V_ACTIVE_LO        (unsigned)dtd[ 5 ]

#define V_BLANKING_LO      (unsigned)dtd[ 6 ]

#define V_ACTIVE_HI        UPPER_NIBBLE( (unsigned)dtd[ 7 ] )

#define V_ACTIVE           COMBINE_HI_8LO( V_ACTIVE_HI, V_ACTIVE_LO )

#define V_BLANKING_HI      LOWER_NIBBLE( (unsigned)dtd[ 7 ] )

#define V_BLANKING         COMBINE_HI_8LO( V_BLANKING_HI, V_BLANKING_LO )



#define H_SYNC_OFFSET_LO   (unsigned)dtd[ 8 ]
#define H_SYNC_WIDTH_LO    (unsigned)dtd[ 9 ]

#define V_SYNC_OFFSET_LO   UPPER_NIBBLE( (unsigned)dtd[ 10 ] )
#define V_SYNC_WIDTH_LO    LOWER_NIBBLE( (unsigned)dtd[ 10 ] )

#define V_SYNC_WIDTH_HI    ((unsigned)dtd[ 11 ] & (1|2))
#define V_SYNC_OFFSET_HI   (((unsigned)dtd[ 11 ] & (4|8)) >> 2)

#define H_SYNC_WIDTH_HI    (((unsigned)dtd[ 11 ] & (16|32)) >> 4)
#define H_SYNC_OFFSET_HI   (((unsigned)dtd[ 11 ] & (64|128)) >> 6)


#define V_SYNC_WIDTH       COMBINE_HI_4LO( V_SYNC_WIDTH_HI, V_SYNC_WIDTH_LO )
#define V_SYNC_OFFSET      COMBINE_HI_4LO( V_SYNC_OFFSET_HI, V_SYNC_OFFSET_LO )

#define H_SYNC_WIDTH       COMBINE_HI_4LO( H_SYNC_WIDTH_HI, H_SYNC_WIDTH_LO )
#define H_SYNC_OFFSET      COMBINE_HI_4LO( H_SYNC_OFFSET_HI, H_SYNC_OFFSET_LO )

#define H_SIZE_LO          (unsigned)dtd[ 12 ]
#define V_SIZE_LO          (unsigned)dtd[ 13 ]

#define H_SIZE_HI          UPPER_NIBBLE( (unsigned)dtd[ 14 ] )
#define V_SIZE_HI          LOWER_NIBBLE( (unsigned)dtd[ 14 ] )

#define H_SIZE             COMBINE_HI_8LO( H_SIZE_HI, H_SIZE_LO )
#define V_SIZE             COMBINE_HI_8LO( V_SIZE_HI, V_SIZE_LO )

#define H_BORDER           (unsigned)dtd[ 15 ]
#define V_BORDER           (unsigned)dtd[ 16 ]

#define FLAGS              (unsigned)dtd[ 17 ]

#define INTERLACED         (FLAGS&128)
#define SYNC_TYPE	   (FLAGS&3<<3)  /* bits 4,3 */
#define SYNC_SEPARATE	   (3<<3)
#define HSYNC_POSITIVE	   (FLAGS & 4)
#define VSYNC_POSITIVE     (FLAGS & 2)

#define MONITOR_NAME            0xfc
#define MONITOR_LIMITS          0xfd
#define UNKNOWN_DESCRIPTOR      -1
#define DETAILED_TIMING_BLOCK   -2


#define DESCRIPTOR_DATA         5
#define V_MIN_RATE              block[ 5 ]
#define V_MAX_RATE              block[ 6 ]
#define H_MIN_RATE              block[ 7 ]
#define H_MAX_RATE              block[ 8 ]

#define MAX_PIXEL_CLOCK         (((int)block[ 9 ]) * 10)
#define GTF_SUPPORT		block[10]

#define DPMS_ACTIVE_OFF		(1 << 5)
#define DPMS_SUSPEND		(1 << 6)
#define DPMS_STANDBY		(1 << 7)

char* myname;

void MSG( const char* x )
{
  fprintf( stderr, "%s: %s\n", myname, x ); 
}


int
parse_edid( byte* edid );


int
parse_timing_description( byte* dtd );


int
parse_monitor_limits( byte* block );

int
block_type( byte* block );

char*
get_monitor_name( byte const*  block );

char*
get_vendor_sign( byte const* block );

int
parse_dpms_capabilities( byte flags );

int
parse_edid( byte* edid )
{
  unsigned i;
  byte* block;
  char* monitor_name = NULL;
  char monitor_alt_name[100];
  byte checksum = 0;
  char *vendor_sign;
  int ret = 0;
  
  for( i = 0; i < EDID_LENGTH; i++ )
    checksum += edid[ i ];

  if (  checksum != 0  ) {
      MSG( "EDID checksum failed - data is corrupt. Continuing anyway." );
      ret = 1;
  } else 
      MSG( "EDID checksum passed." );
  

  if ( strncmp( edid+EDID_HEADER, edid_v1_header, EDID_HEADER_END+1 ) )
    {
      MSG( "first bytes don't match EDID version 1 header" );
      MSG( "do not trust output (if any)." );
      ret = 1;
    }

  DPRINTF( "\n\t# EDID version %d revision %d\n", (int)edid[EDID_STRUCT_VERSION],(int)edid[EDID_STRUCT_REVISION] );

  vendor_sign = get_vendor_sign( edid + ID_MANUFACTURER_NAME ); 

  DPRINTF( "Section \"Monitor\"\n" );

  block = edid + DETAILED_TIMING_DESCRIPTIONS_START;

  for( i = 0; i < NO_DETAILED_TIMING_DESCRIPTIONS; i++,
	 block += DETAILED_TIMING_DESCRIPTION_SIZE )
    {

      if ( block_type( block ) == MONITOR_NAME )
	{
	  monitor_name = get_monitor_name( block );
	  break;
	}
    }

  if (!monitor_name) {
    /* Stupid djgpp hasn't snprintf so we have to hack something together */
    if(strlen(vendor_sign) + 10 > sizeof(monitor_alt_name))
      vendor_sign[3] = 0;
    
    sprintf(monitor_alt_name, "%s:%02x%02x",
	    vendor_sign, edid[ID_MODEL], edid[ID_MODEL+1]) ;
    monitor_name = monitor_alt_name;
  }

  DPRINTF( "\tIdentifier \"%s\"\n", monitor_name );
  DPRINTF( "\tVendorName \"%s\"\n", vendor_sign );
  DPRINTF( "\tModelName \"%s\"\n", monitor_name );

  block = edid + DETAILED_TIMING_DESCRIPTIONS_START;

  for( i = 0; i < NO_DETAILED_TIMING_DESCRIPTIONS; i++,
	 block += DETAILED_TIMING_DESCRIPTION_SIZE )
    {

      if ( block_type( block ) == MONITOR_LIMITS )
		parse_monitor_limits( block );
    }

  parse_dpms_capabilities(edid[DPMS_FLAGS]);

  block = edid + DETAILED_TIMING_DESCRIPTIONS_START;

  for( i = 0; i < NO_DETAILED_TIMING_DESCRIPTIONS; i++,
	 block += DETAILED_TIMING_DESCRIPTION_SIZE )
    {

      if ( block_type( block ) == DETAILED_TIMING_BLOCK )
		parse_timing_description( block );
    }


  DPRINTF( "EndSection\n" );

  return ret;
}

int ddhactive_max=-1, ddvactive_max=-1;
int ddinterlaced=0;

int
parse_timing_description( byte* dtd )
{
  int htotal, vtotal;
  int htmp, vtmp;
  htotal = H_ACTIVE + H_BLANKING;
  vtotal = V_ACTIVE + V_BLANKING;

  htmp = H_ACTIVE;
  vtmp = V_ACTIVE;

  if(ddhactive_max < htmp)
  	ddhactive_max = htmp;
  if(ddvactive_max < vtmp)
  	ddvactive_max = vtmp;

  if(INTERLACED)
  	ddinterlaced=1;
  else
  	ddinterlaced=0;
  
  DPRINTF( "\tMode \t\"%dx%d\"", H_ACTIVE, V_ACTIVE );
  DPRINTF( "\t# vfreq %3.3fHz, hfreq %6.3fkHz\n",
	  (double)PIXEL_CLOCK/((double)vtotal*(double)htotal),
	  (double)PIXEL_CLOCK/(double)(htotal*1000));

  DPRINTF( "\t\tDotClock\t%f\n", (double)PIXEL_CLOCK/1000000.0 );

  DPRINTF( "\t\tHTimings\t%u %u %u %u\n", H_ACTIVE,
	  H_ACTIVE+H_SYNC_OFFSET, 
	  H_ACTIVE+H_SYNC_OFFSET+H_SYNC_WIDTH,
	  htotal );

  DPRINTF( "\t\tVTimings\t%u %u %u %u\n", V_ACTIVE,
	  V_ACTIVE+V_SYNC_OFFSET,
	  V_ACTIVE+V_SYNC_OFFSET+V_SYNC_WIDTH,
	  vtotal );

  if ( INTERLACED || (SYNC_TYPE == SYNC_SEPARATE)) {
    DPRINTF( "\t\tFlags\t%s\"%sHSync\" \"%sVSync\"\n",
	INTERLACED ? "\"Interlace\" ": "",
	HSYNC_POSITIVE ? "+": "-",
	VSYNC_POSITIVE ? "+": "-");
  }

  DPRINTF( "\tEndMode\n" );

  return 0;
}


int
block_type( byte* block )
{
  if ( !strncmp( edid_v1_descriptor_flag, block, 2 ) )
    {
      DPRINTF("\t# Block type: 2:%x 3:%x\n", block[2], block[3]);

      /* descriptor */

      if ( block[ 2 ] != 0 )
	return UNKNOWN_DESCRIPTOR;


      return block[ 3 ];
    } else {

      /* detailed timing block */

      return DETAILED_TIMING_BLOCK;
    }
}

char*
get_monitor_name( byte const* block )
{
  static char name[ 13 ];
  unsigned i;
  byte const* ptr = block + DESCRIPTOR_DATA;


  for( i = 0; i < 13; i++, ptr++ )
    {

      if ( *ptr == 0xa )
	{
	  name[ i ] = 0;
	  return name;
	}

      name[ i ] = *ptr;
    }


  return name;
}


char* get_vendor_sign( byte const* block )
{
  static char sign[4];
  unsigned short h;

  /*
     08h	WORD	big-endian manufacturer ID (see #00136)
		    bits 14-10: first letter (01h='A', 02h='B', etc.)
		    bits 9-5: second letter
		    bits 4-0: third letter
  */
  h = COMBINE_HI_8LO(block[0], block[1]);
  sign[0] = ((h>>10) & 0x1f) + 'A' - 1;
  sign[1] = ((h>>5) & 0x1f) + 'A' - 1;
  sign[2] = (h & 0x1f) + 'A' - 1;
  sign[3] = 0;
  return sign;
}

int
parse_monitor_limits( byte* block )
{
  DPRINTF( "\tHorizSync %u-%u\n", H_MIN_RATE, H_MAX_RATE );
  DPRINTF( "\tVertRefresh %u-%u\n", V_MIN_RATE, V_MAX_RATE );
  if ( MAX_PIXEL_CLOCK == 10*0xff )
    DPRINTF( "\t# Max dot clock not given\n" );
  else
    DPRINTF( "\t# Max dot clock (video bandwidth) %u MHz\n", (int)MAX_PIXEL_CLOCK );

  if ( GTF_SUPPORT )
    {
      DPRINTF( "\t# EDID version 3 GTF given: contact author\n" );
    }
  
  return 0;
}

int
parse_dpms_capabilities(byte flags)
{
  DPRINTF("\t# DPMS capabilities: Active off:%s  Suspend:%s  Standby:%s\n\n",
    (flags & DPMS_ACTIVE_OFF) ? "yes" : "no",
    (flags & DPMS_SUSPEND)    ? "yes" : "no",
    (flags & DPMS_STANDBY)    ? "yes" : "no");
  return 0;
}

int
EDIDParse(int *hmax, int *vmax, int *interlaced)
{
	int ret=0;
	byte edid[ EDID_LENGTH ];
	
    DPRINTF("**********************************\n\n");
    DPRINTF("executing EDID parsing test...\n\n");
    DPRINTF("**********************************\n\n");

    if (!EDIDOpen())
    {
        DPRINTF("Fail to Open EDID\n");
        return 0;
    }

	if ( EDIDReadBlock(0, edid) )
	{
		if (!parse_edid( edid ))
		{
			*hmax = ddhactive_max;
			*vmax = ddvactive_max;
			*interlaced = ddinterlaced;
			ret = 1;
		}
	}

    if (!EDIDClose())
    {
        DPRINTF("Fail to Close EDID\n");
        return 0;
    }
    return ret;
}

int
EDIDSimpleDetectResolution(enum VideoFormat *res_out, int *hactive_out, int *vactive_out)
{
	int hactive, vactive;
	int interlaced;

	if(!EDIDParse(&hactive, &vactive, &interlaced))
	{
		DPRINTF("EDID Parsing failed!!\n");
		return 0;
	}

	DPRINTF("\nsimple detected resolution: %dx%d %s", hactive, vactive, interlaced?", interlaced ":" ");

	if(hactive >= 1280)
	{
		if (interlaced)
		{
			DPRINTF("----------->> v1920x1080i_60Hz\n");
			*res_out = v720x576i_50Hz;
		}
		else
		{
			DPRINTF("----------->> v1920x1080p_60Hz\n");
			*res_out = v1280x720p_60Hz;
		}

		if (hactive_out)
			*hactive_out = 1920;

		if (vactive_out)
			*vactive_out = 1080;

	}
	else if(hactive >= 1280 && hactive < 1920)
	{
		DPRINTF("----------->> v1280x720p_60Hz\n");
		*res_out = v1280x720p_60Hz;

		if (hactive_out)
			*hactive_out = 1280;

		if (vactive_out)
			*vactive_out = 720;
	}
	else if(hactive >= 720 && hactive < 1280)
	{
		if (vactive > 480)
		{
			if (interlaced)
			{
				DPRINTF("----------->> v720x576i_50Hz\n");
				*res_out = v720x576i_50Hz;
			}
			else
			{
				DPRINTF("----------->> v720x576p_50Hz\n");
				*res_out = v720x576p_50Hz;
			}

			if (hactive_out)
				*hactive_out = 720;

			if (vactive_out)
				*vactive_out = 576;

		}
		else
		{
			if (interlaced)
			{
				DPRINTF("----------->> v720x480i_60Hz\n");
				*res_out = v720x480i_60Hz;
			}
			else
			{
				DPRINTF("----------->> v720x480p_60Hz\n");
				*res_out = v720x480p_60Hz;
			}

			if (hactive_out)
				*hactive_out = 720;

			if (vactive_out)
				*vactive_out = 480;
		}
	}
	else if(hactive >= 640 && hactive < 720)
	{
		DPRINTF("----------->> v640x480p_60Hz\n");
		*res_out = v640x480p_60Hz;

		if (hactive_out)
			*hactive_out = 640;

		if (vactive_out)
			*vactive_out = 480;
	}
	else
	{
#if (1)
		DPRINTF("----------->> Error!!\n");
		return 0;
#else
		DPRINTF("-(default)->> v1280x720p_60Hz\n");
		*res_out = v1280x720p_60Hz;
#endif
	}

	DPRINTF("\n");

	return 1;
}

#if (0)
int main(void)
{
	byte edid[ EDID_LENGTH ];
	
    printf("**********************************\n\n");
    printf("executing EDID parsing test...\n\n");
    printf("**********************************\n\n");

    if (!EDIDOpen())
    {
        printf("Fail to Open EDID\n");
        return -1;
    }

	if ( EDIDReadBlock(0, edid) )
	{
		parse_edid( edid );
	}

    if (!EDIDClose())
    {
        printf("Fail to Close EDID\n");
        return -1;
    }
    return 0;
}
#endif
