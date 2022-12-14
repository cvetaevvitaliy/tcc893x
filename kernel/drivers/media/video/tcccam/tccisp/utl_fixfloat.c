/****************************************************************************
One line to give the program's name and a brief idea of what it does.
Copyright (C) 2013 Telechips Inc.

This program is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA 02111-1307 USA
****************************************************************************/


#include "stdinc.h"

// Exactly correct for formats with fractional part would be:
// A range limit (MAX) equal to the value in the comments and a comparison "< MAX"
// instead "<= MAX" in the asserts below. But binary representations of real numbers
// are inaccurate anyway. So we take a slightly lower value to avoid potential problems.
#define UTL_FIX_MAX_U0208          3.998f //exactly this would be < 4 - 0.5/256
#define UTL_FIX_MIN_U0208          0.0f
#define UTL_FIX_PRECISION_U0208  256.0f
#define UTL_FIX_MASK_U0208       0x3ff

#define UTL_FIX_MAX_U0408          15.998f //exactly this would be < 16 - 0.5/256
#define UTL_FIX_MIN_U0408          0.0f
#define UTL_FIX_PRECISION_U0408  256.0f
#define UTL_FIX_MASK_U0408       0xfff

#define UTL_FIX_MAX_U0800        255.499f //exactly this would be < 256 - 0.5
#define UTL_FIX_MIN_U0800          0.0f
#define UTL_FIX_MASK_U0800       0x0ff

#define UTL_FIX_MAX_U1000       1023.499f //exactly this would be < 1024 - 0.5
#define UTL_FIX_MIN_U1000          0.0f
#define UTL_FIX_MASK_U1000       0x3ff

#define UTL_FIX_MAX_U1200       4095.499f //exactly this would be < 4096 - 0.5
#define UTL_FIX_MIN_U1200          0.0f
#define UTL_FIX_MASK_U1200       0xfff

#define UTL_FIX_MAX_S0207          1.996f //exactly this would be < 2 - 0.5/128
#define UTL_FIX_MIN_S0207         -2.0f
#define UTL_FIX_PRECISION_S0207  128.0f
#define UTL_FIX_MASK_S0207       0x01ff
#define UTL_FIX_SIGN_S0207       0x0100

#define UTL_FIX_MAX_S0307          3.996f //exactly this would be < 4 - 0.5/128
#define UTL_FIX_MIN_S0307         -4.0f
#define UTL_FIX_PRECISION_S0307  128.0f
#define UTL_FIX_MASK_S0307       0x03ff
#define UTL_FIX_SIGN_S0307       0x0200

#define UTL_FIX_MAX_S0407          7.996f //exactly this would be < 8 - 0.5/128
#define UTL_FIX_MIN_S0407         -8.0f
#define UTL_FIX_PRECISION_S0407  128.0f
#define UTL_FIX_MASK_S0407       0x07ff
#define UTL_FIX_SIGN_S0407       0x0400

#define UTL_FIX_MAX_S0808        127.998f //exactly this would be < 128 - 0.5/256
#define UTL_FIX_MIN_S0808       -128.0f
#define UTL_FIX_PRECISION_S0808  256.0f
#define UTL_FIX_MASK_S0808       0xffff
#define UTL_FIX_SIGN_S0808       0x8000

#define UTL_FIX_MAX_S1200       2047.499f //exactly this would be < 2048 - 0.5
#define UTL_FIX_MIN_S1200      -2048.0f
#define UTL_FIX_MASK_S1200       0x0fff
#define UTL_FIX_SIGN_S1200       0x0800


/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_U0408 \n
 *  \RETURNVALUE unsigned fixed point value in UINT32 container \n
 *  \PARAMETERS  FLOAT value \n
 *  \DESCRIPTION Converts float values to unsigned fixed point values with \n
 *               4 bit integer and 8 bit fractional part to program the \n
 *               marvin registers. \n
 */
/*****************************************************************************/

// unsigned fixed point 4 bit integer / 8 bit fractional part
// 0x3ff = 4095/256 = 15.99609375
// 0x100 = 1
// 0x000 = 0

UINT32 UtlFloatToFix_U0408( FLOAT fFloat )
{
  UINT32 ulFix = 0;

  ASSERT( fFloat <= UTL_FIX_MAX_U0408 );
  ASSERT( fFloat >= UTL_FIX_MIN_U0408 );

  fFloat *= UTL_FIX_PRECISION_U0408;

  // round
  // no handling of negative values required
  ulFix = (UINT32)(fFloat + 0.5f);

  //no masking of upper bits required

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_U0408 \n
 *  \RETURNVALUE FLOAT value \n
 *  \PARAMETERS  unsigned fixed point value in UINT32 container \n
 *  \DESCRIPTION Converts unsigned fixed point values with 4 bit integer and \n
 *               8 bit fractional part (marvin register) to float values. \n
 */
/*****************************************************************************/
FLOAT UtlFixToFloat_U0408( UINT32 ulFix )
{
  FLOAT fFloat = 0;

  // any value from 0x0000 to 0x0fff will do as input
  ASSERT( (ulFix & ~UTL_FIX_MASK_U0408) == 0 );

  // precision is not cut away here, so no rounding is necessary
  // no handling of negative values required
  fFloat = (FLOAT)ulFix;

  fFloat /= UTL_FIX_PRECISION_U0408;

  return fFloat;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_U0208 \n
 *  \RETURNVALUE unsigned fixed point value in UINT32 container \n
 *  \PARAMETERS  FLOAT value \n
 *  \DESCRIPTION Converts float values to unsigned fixed point values with \n
 *               2 bit integer and 8 bit fractional part to program the \n
 *               marvin registers. \n
 */
/*****************************************************************************/

// unsigned fixed point 2 bit integer / 8 bit fractional part
// 0x3ff = 1023/256 = 3.99609375
// 0x100 = 1
// 0x000 = 0

UINT32 UtlFloatToFix_U0208( FLOAT fFloat )
{
  UINT32 ulFix = 0;

  ASSERT( fFloat <= UTL_FIX_MAX_U0208 );
  ASSERT( fFloat >= UTL_FIX_MIN_U0208 );

  fFloat *= UTL_FIX_PRECISION_U0208;

  // round
  // no handling of negative values required
  ulFix = (UINT32)(fFloat + 0.5f);

  //no masking of upper bits required

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_U0208 \n
 *  \RETURNVALUE FLOAT value \n
 *  \PARAMETERS  unsigned fixed point value in UINT32 container \n
 *  \DESCRIPTION Converts unsigned fixed point values with 2 bit integer and \n
 *               8 bit fractional part (marvin register) to float values. \n
 */
/*****************************************************************************/
FLOAT UtlFixToFloat_U0208( UINT32 ulFix )
{
  FLOAT fFloat = 0;

  // any value from 0x0000 to 0x03ff will do as input
  ASSERT( (ulFix & ~UTL_FIX_MASK_U0208) == 0 );

  // precision is not cut away here, so no rounding is necessary
  // no handling of negative values required
  fFloat = (FLOAT)ulFix;

  fFloat /= UTL_FIX_PRECISION_U0208;

  return fFloat;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_U1000 \n
 *  \RETURNVALUE unsigned integer value in UINT32 container \n
 *  \PARAMETERS  FLOAT value \n
 *  \DESCRIPTION Converts float values to unsigned integer values with \n
 *               10 bit integer and no fractional part to program the \n
 *               marvin registers. \n
 */
/*****************************************************************************/

// unsigned 10 bit integer, no fractional part
// 0x3ff = 1023
// 0x001 = 1
// 0x000 = 0

UINT32 UtlFloatToFix_U1000( FLOAT fFloat )
{
  UINT32 ulFix = 0;

  ASSERT( fFloat <= UTL_FIX_MAX_U1000 );
  ASSERT( fFloat >= UTL_FIX_MIN_U1000 );

  // precision is 1, thus no multiplication is required

  // round
  // no handling of negative values required
  ulFix = (UINT32)(fFloat + 0.5f);

  //no masking of upper bits required

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_U1000 \n
 *  \RETURNVALUE FLOAT value \n
 *  \PARAMETERS  unsigned integer value in UINT32 container \n
 *  \DESCRIPTION Converts unsigned integer values with 10 bit integer and \n
 *               no fractional part (marvin register) to float values. \n
 */
/*****************************************************************************/
FLOAT UtlFixToFloat_U1000( UINT32 ulFix )
{
  FLOAT fFloat = 0;

  // any value from 0x0000 to 0x03ff will do as input
  ASSERT( (ulFix & ~UTL_FIX_MASK_U1000) == 0 );

  // precision is not cut away here, so no rounding is necessary
  // no handling of negative values required
  fFloat = (FLOAT)ulFix;

  // precision is 1, thus no division is required.

  return fFloat;
}
/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_U0800 \n
 *  \RETURNVALUE unsigned integer value in UINT32 container \n
 *  \PARAMETERS  FLOAT value \n
 *  \DESCRIPTION Converts float values to unsigned integer values with \n
 *               8 bit integer and no fractional part to program the \n
 *               marvin registers. \n
 */
/*****************************************************************************/

// unsigned 8 bit integer, no fractional part
// 0x0ff = 255
// 0x001 = 1
// 0x000 = 0

UINT32 UtlFloatToFix_U0800( FLOAT fFloat )
{
  UINT32 ulFix = 0;

  ASSERT( fFloat <= UTL_FIX_MAX_U0800 );
  ASSERT( fFloat >= UTL_FIX_MIN_U0800 );

  // precision is 1, thus no multiplication is required

  // round
  // no handling of negative values required
  ulFix = (UINT32)(fFloat + 0.5f);

  //no masking of upper bits required

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_U0800 \n
 *  \RETURNVALUE FLOAT value \n
 *  \PARAMETERS  unsigned integer value in UINT32 container \n
 *  \DESCRIPTION Converts unsigned integer values with 10 bit integer and \n
 *               no fractional part (marvin register) to float values. \n
 */
/*****************************************************************************/
FLOAT UtlFixToFloat_U0800( UINT32 ulFix )
{
  FLOAT fFloat = 0;

  // any value from 0x0000 to 0x00ff will do as input
  ASSERT( (ulFix & ~UTL_FIX_MASK_U0800) == 0 );

  // precision is not cut away here, so no rounding is necessary
  // no handling of negative values required
  fFloat = (FLOAT)ulFix;

  // precision is 1, thus no division is required.

  return fFloat;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_U1200 \n
 *  \RETURNVALUE unsigned integer value in UINT32 container \n
 *  \PARAMETERS  FLOAT value \n
 *  \DESCRIPTION Converts float values to unsigned integer values with \n
 *               12 bit integer and no fractional part to program the \n
 *               marvin registers. \n
 */
/*****************************************************************************/

// unsigned 12 bit integer, no fractional part
// 0xfff = 4095
// 0x001 = 1
// 0x000 = 0

UINT32 UtlFloatToFix_U1200( FLOAT fFloat )
{
  UINT32 ulFix = 0;

  ASSERT( fFloat <= UTL_FIX_MAX_U1200 );
  ASSERT( fFloat >= UTL_FIX_MIN_U1200 );

  // precision is 1, thus no multiplication is required

  // round
  // no handling of negative values required
  ulFix = (UINT32)(fFloat + 0.5f);

  //no masking of upper bits required

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_U1200 \n
 *  \RETURNVALUE FLOAT value \n
 *  \PARAMETERS  unsigned integer value in UINT32 container \n
 *  \DESCRIPTION Converts unsigned integer values with 12 bit integer and \n
 *               no fractional part (marvin register) to float values. \n
 */
/*****************************************************************************/
FLOAT UtlFixToFloat_U1200( UINT32 ulFix )
{
  FLOAT fFloat = 0;

  // any value from 0x0000 to 0x0fff will do as input
  ASSERT( (ulFix & ~UTL_FIX_MASK_U1200) == 0 );

  // precision is not cut away here, so no rounding is necessary
  // no handling of negative values required
  fFloat = (FLOAT)ulFix;

  // precision is 1, thus no division is required.

  return fFloat;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_S0207 \n
 *  \RETURNVALUE signed fixed point value in UINT32 container, leading 1's \n
 *               in the upper bits, not used by the format, are suppressed \n
 *  \PARAMETERS  FLOAT value \n
 *  \DESCRIPTION Converts float values to signed fixed point values (two's
 *               complement) with 2 bit integer and 7 bit fractional part to
 *               program the marvin registers. \n
 */
/*****************************************************************************/

// signed (two's complement) fixed point 2 bit integer / 7 bit fractional part
// 0x0ff = 255/128 = 1.9921875
// 0x080 = 1
// 0x000 = 0
// 0x1ff = -1/128 = -0.0078125
// 0x100 = -256/128 = -2

UINT32 UtlFloatToFix_S0207( FLOAT fFloat )
{
  UINT32 ulFix = 0;

  ASSERT( fFloat <= UTL_FIX_MAX_S0207 );
  ASSERT( fFloat >= UTL_FIX_MIN_S0207 );

  fFloat *= UTL_FIX_PRECISION_S0207;

  // round, two's complement if negative
  if( fFloat > 0.0f ){
    ulFix = (UINT32)(fFloat + 0.5f);
  }
  else
  {
    fFloat = -fFloat;
    ulFix  = (UINT32)(fFloat + 0.5f);
    ulFix  = ~ulFix;
    ulFix++;
  }

  // set upper (unused) bits to 0
  ulFix &= UTL_FIX_MASK_S0207;

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_S0207 \n
 *  \RETURNVALUE FLOAT value \n
 *  \PARAMETERS  signed fixed point value in UINT32 container, leading \n
 *               1's in the upper bits, not used by the format, must be 0 \n
 *  \DESCRIPTION Converts signed fixed point values (two's complement) with \n
 *               2 bit integer and 7 bit fractional part (marvin register) to \n
 *               float values. \n
 */
/*****************************************************************************/
FLOAT UtlFixToFloat_S0207( UINT32 ulFix )
{
  FLOAT fFloat = 0;

  // any value from 0x0000 to 0x01ff will do as input
  ASSERT( (ulFix & ~UTL_FIX_MASK_S0207) == 0 );

  // sign extension and two's complement if negative
  // (precision is not cut away here, so no rounding is necessary)
  if( (ulFix & UTL_FIX_SIGN_S0207) == 0 )
  {
    fFloat = (FLOAT)ulFix;
  }
  else
  {
    ulFix |= ~UTL_FIX_MASK_S0207;
    ulFix--;
    ulFix = ~ulFix;
    fFloat = (FLOAT)ulFix;
    fFloat = -fFloat;
  }

  fFloat /= UTL_FIX_PRECISION_S0207;

  return fFloat;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_S0307 \n
 *  \RETURNVALUE signed fixed point value in UINT32 container, leading 1's \n
 *               in the upper bits, not used by the format, are suppressed \n
 *  \PARAMETERS  FLOAT value \n
 *  \DESCRIPTION Converts float values to signed fixed point values (two's
 *               complement) with 3 bit integer and 7 bit fractional part to
 *               program the marvin registers. \n
 */
/*****************************************************************************/

// signed (two's complement) fixed point 3 bit integer / 7 bit fractional part
// 0x1ff = 511/128 = 3.9921875
// 0x080 = 1
// 0x000 = 0
// 0x3ff = -1/128 = -0.0078125
// 0x200 = -512/128 = -4

UINT32 UtlFloatToFix_S0307( FLOAT fFloat )
{
  UINT32 ulFix = 0;

  ASSERT( fFloat <= UTL_FIX_MAX_S0307 );
  ASSERT( fFloat >= UTL_FIX_MIN_S0307 );

  fFloat *= UTL_FIX_PRECISION_S0307;

  // round, two's complement if negative
  if( fFloat > 0.0f ){
    ulFix = (UINT32)(fFloat + 0.5f);
  }
  else
  {
    fFloat = -fFloat;
    ulFix  = (UINT32)(fFloat + 0.5f);
    ulFix  = ~ulFix;
    ulFix++;
  }

  // set upper (unused) bits to 0
  ulFix &= UTL_FIX_MASK_S0307;

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_S0307 \n
 *  \RETURNVALUE FLOAT value \n
 *  \PARAMETERS  signed fixed point value in UINT32 container, leading \n
 *               1's in the upper bits, not used by the format, must be 0 \n
 *  \DESCRIPTION Converts signed fixed point values (two's complement) with \n
 *               3 bit integer and 7 bit fractional part (marvin register) to \n
 *               float values. \n
 */
/*****************************************************************************/
FLOAT UtlFixToFloat_S0307( UINT32 ulFix )
{
  FLOAT fFloat = 0;

  // any value from 0x0000 to 0x03ff will do as input
  ASSERT( (ulFix & ~UTL_FIX_MASK_S0307) == 0 );

  // sign extension and two's complement if negative
  // (precision is not cut away here, so no rounding is necessary)
  if( (ulFix & UTL_FIX_SIGN_S0307) == 0 )
  {
    fFloat = (FLOAT)ulFix;
  }
  else
  {
    ulFix |= ~UTL_FIX_MASK_S0307;
    ulFix--;
    ulFix = ~ulFix;
    fFloat = (FLOAT)ulFix;
    fFloat = -fFloat;
  }

  fFloat /= UTL_FIX_PRECISION_S0307;

  return fFloat;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_S0407 \n
 *  \RETURNVALUE signed fixed point value in UINT32 container, leading 1's \n
 *               in the upper bits, not used by the format, are suppressed \n
 *  \PARAMETERS  FLOAT value \n
 *  \DESCRIPTION Converts float values to signed fixed point values (two's
 *               complement) with 4 bit integer and 7 bit fractional part to
 *               program the marvin registers. \n
 */
/*****************************************************************************/

// signed (two's complement) fixed point 4 bit integer / 7 bit fractional part
// 0x3ff = 1023/128 = 7.9921875
// 0x080 = 1
// 0x000 = 0
// 0x7ff = -1/128 = -0.0078125
// 0x400 = -1024/128 = -8

UINT32 UtlFloatToFix_S0407( FLOAT fFloat )
{
  UINT32 ulFix = 0;

  ASSERT( fFloat <= UTL_FIX_MAX_S0407 );
  ASSERT( fFloat >= UTL_FIX_MIN_S0407 );

  fFloat *= UTL_FIX_PRECISION_S0407;

  // round, two's complement if negative
  if( fFloat > 0.0f ){
    ulFix = (UINT32)(fFloat + 0.5f);
  }
  else
  {
    fFloat = -fFloat;
    ulFix  = (UINT32)(fFloat + 0.5f);
    ulFix  = ~ulFix;
    ulFix++;
  }

  // set upper (unused) bits to 0
  ulFix &= UTL_FIX_MASK_S0407;

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_S0407 \n
 *  \RETURNVALUE FLOAT value \n
 *  \PARAMETERS  signed fixed point value in UINT32 container, leading \n
 *               1's in the upper bits, not used by the format, must be 0 \n
 *  \DESCRIPTION Converts signed fixed point values (two's complement) with \n
 *               4 bit integer and 7 bit fractional part (marvin register) to \n
 *               float values. \n
 */
/*****************************************************************************/
FLOAT UtlFixToFloat_S0407( UINT32 ulFix )
{
  FLOAT fFloat = 0;

  // any value from 0x0000 to 0x07ff will do as input
  ASSERT( (ulFix & ~UTL_FIX_MASK_S0407) == 0 );

  // sign extension and two's complement if negative
  // (precision is not cut away here, so no rounding is necessary)
  if( (ulFix & UTL_FIX_SIGN_S0407) == 0 )
  {
    fFloat = (FLOAT)ulFix;
  }
  else
  {
    ulFix |= ~UTL_FIX_MASK_S0407;
    ulFix--;
    ulFix = ~ulFix;
    fFloat = (FLOAT)ulFix;
    fFloat = -fFloat;
  }

  fFloat /= UTL_FIX_PRECISION_S0407;

  return fFloat;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_S0808 \n
 *  \RETURNVALUE signed fixed point value in UINT32 container, leading 1's \n
 *               in the upper bits, not used by the format, are suppressed \n
 *  \PARAMETERS  FLOAT value \n
 *  \DESCRIPTION Converts float values to signed fixed point values (two's
 *               complement) with 8 bit integer and 8 bit fractional part to
 *               program the marvin registers. \n
 */
/*****************************************************************************/

// signed (two's complement) fixed point 8 bit integer / 8 bit fractional part
// 0x7fff = 32767/256 = 127.99609375
// 0x0100 = 1
// 0x0000 = 0
// 0xffff = -1/256 = -0.00390625
// 0x8000 = -32768/256 = -128

UINT32 UtlFloatToFix_S0808( FLOAT fFloat )
{
  UINT32 ulFix = 0;

  ASSERT( fFloat <= UTL_FIX_MAX_S0808 );
  ASSERT( fFloat >= UTL_FIX_MIN_S0808 );

  fFloat *= UTL_FIX_PRECISION_S0808;

  // round, two's complement if negative
  if( fFloat > 0.0f ){
    ulFix = (UINT32)(fFloat + 0.5f);
  }
  else
  {
    fFloat = -fFloat;
    ulFix  = (UINT32)(fFloat + 0.5f);
    ulFix  = ~ulFix;
    ulFix++;
  }

  // set upper (unused) bits to 0
  ulFix &= UTL_FIX_MASK_S0808;

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_S0808 \n
 *  \RETURNVALUE FLOAT value \n
 *  \PARAMETERS  signed fixed point value in UINT32 container, leading \n
 *               1's in the upper bits, not used by the format, must be 0 \n
 *  \DESCRIPTION Converts signed fixed point values (two's complement) with \n
 *               8 bit integer and 8 bit fractional part (marvin register) to \n
 *               float values. \n
 */
/*****************************************************************************/
FLOAT UtlFixToFloat_S0808( UINT32 ulFix )
{
  FLOAT fFloat = 0;

  // any value from 0x0000 to 0xffff will do as input
  ASSERT( (ulFix & ~UTL_FIX_MASK_S0808) == 0 );

  // sign extension and two's complement if negative
  // (precision is not cut away here, so no rounding is necessary)
  if( (ulFix & UTL_FIX_SIGN_S0808) == 0 )
  {
    fFloat = (FLOAT)ulFix;
  }
  else
  {
    ulFix |= ~UTL_FIX_MASK_S0808;
    ulFix--;
    ulFix = ~ulFix;
    fFloat = (FLOAT)ulFix;
    fFloat = -fFloat;
  }

  fFloat /= UTL_FIX_PRECISION_S0808;

  return fFloat;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_S1200 \n
 *  \RETURNVALUE signed integer value in UINT32 container, leading 1's \n
 *               in the upper bits, not used by the format, are suppressed \n
 *  \PARAMETERS  FLOAT value \n
 *  \DESCRIPTION Converts float values to signed integer values (two's
 *               complement) with 12 bit integer and no fractional part to
 *               program the marvin registers. \n
 */
/*****************************************************************************/

// signed (two's complement) 12 bit integer, no fractional part
// 0x7ff = 2047
// 0x001 = 1
// 0x000 = 0
// 0xfff = -1
// 0x800 = -2048

UINT32 UtlFloatToFix_S1200( FLOAT fFloat )
{
  UINT32 ulFix = 0;

  ASSERT( fFloat <= UTL_FIX_MAX_S1200 );
  ASSERT( fFloat >= UTL_FIX_MIN_S1200 );

  // precision is 1, thus no multiplication is required

  // round, two's complement if negative
  if( fFloat > 0.0f ){
    ulFix = (UINT32)(fFloat + 0.5f);
  }
  else
  {
    fFloat = -fFloat;
    ulFix  = (UINT32)(fFloat + 0.5f);
    ulFix  = ~ulFix;
    ulFix++;
  }

  // set upper (unused) bits to 0
  ulFix &= UTL_FIX_MASK_S1200;

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_S1200 \n
 *  \RETURNVALUE FLOAT value \n
 *  \PARAMETERS  signed integer value in UINT32 container, leading \n
 *               1's in the upper bits, not used by the format, must be 0 \n
 *  \DESCRIPTION Converts signed integer values (two's complement) with \n
 *               12 bit integer and no fractional part (marvin register) to \n
 *               float values. \n
 */
/*****************************************************************************/
FLOAT UtlFixToFloat_S1200( UINT32 ulFix )
{
  FLOAT fFloat = 0;

  // any value from 0x0000 to 0x0fff will do as input
  ASSERT( (ulFix & ~UTL_FIX_MASK_S1200) == 0 );

  // sign extension and two's complement if negative
  // (precision is not cut away here, so no rounding is necessary)
  if( (ulFix & UTL_FIX_SIGN_S1200) == 0 )
  {
    fFloat = (FLOAT)ulFix;
  }
  else
  {
    ulFix |= ~UTL_FIX_MASK_S1200;
    ulFix--;
    ulFix = ~ulFix;
    fFloat = (FLOAT)ulFix;
    fFloat = -fFloat;
  }

  // precision is 1, thus no division is required.

  return fFloat;
}
