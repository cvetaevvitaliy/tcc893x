
/*!
 ***********************************************************************
 \par Copyright
 \verbatim
  ________  _____           _____   _____           ____  ____   ____		
     /     /       /       /       /       /     /   /    /   \ /			
    /     /___    /       /___    /       /____ /   /    /____/ \___			
   /     /       /       /       /       /     /   /    /           \		
  /     /_____  /_____  /_____  /_____  /     / _ /_  _/_      _____/ 		
   																				
  Copyright (c) 2010 Telechips Inc.
  Korad Bldg, 1000-12 Daechi-dong, Kangnam-Ku, Seoul, Korea					
 ***********************************************************************
 */
/*!
 ***********************************************************************

 ***********************************************************************
 */

#ifndef _TCC_SUBTITLE_INTERFACE_H_
#define _TCC_SUBTITLE_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif


/* ======================================================================
**  INCLUDE
** ====================================================================== */


/* ======================================================================
**  DIFINITION OF CONSTANT, STRUCTURE, ENUMERATION, MACRO
** ====================================================================== */

// Return values.
typedef enum
{
    tccSubtitle_Success = 0,

    tccSubtitle_GeneralError,
    tccSubtitle_InvalidParameter,

    tccSubtitle_Error_Last
} tccSubtitle_ErrorCodes_t;

// Text Character Set */
typedef enum
{
	tccSubtitle_CharacterSet_NONE,
	tccSubtitle_CharacterSet_ANSI,
	tccSubtitle_CharacterSet_UNICODE,
	tccSubtitle_CharacterSet_UTF8,

	tccSubtitle_CharacterSet_Last
} tccSubtitle_CharacterSet;


/* ======================================================================
**  GLOBAL FUNCTION
** ====================================================================== */

/**
 * tccSubtitle_open opens an external subtitle with filename.
 * 
 * @param [in] filename - Moving picture filename included directory name
 *
 * @return tccSubtitle_ErrorCodes_t
 * @retval tccSubtitle_Success - Success.
 */
extern tccSubtitle_ErrorCodes_t tccSubtitle_open(char *filename, int bytecharactersize);


/**
 * tccSubtitle_close closes an external subtitle
 * 
 * @return tccSubtitle_ErrorCodes_t
 * @retval tccSubtitle_Success - Success.
 */
extern tccSubtitle_ErrorCodes_t tccSubtitle_close(void);


/**
 * tccSubtitle_getFrame gets a frame data
 * 
 * @return tccSubtitle_ErrorCodes_t
 * @retval tccSubtitle_Success - Success.
 */
extern tccSubtitle_ErrorCodes_t tccSubtitle_getFrame(int *pLangIdx, int *pStartPTS, int *pEndPTS, int *pLineCount, int *pTextLength, char **pText);


/**
 * tccSubtitle_seek seeks
 * 
 * @return tccSubtitle_ErrorCodes_t
 * @retval tccSubtitle_Success - Success.
 */
extern tccSubtitle_ErrorCodes_t tccSubtitle_seek(int targetTime);


/**
 * tccSubtitle_getOutputTextCharacterSet seeks
 * 
 * @return tccSubtitle_CharacterSet
 */
extern tccSubtitle_CharacterSet tccSubtitle_getOutputTextCharacterSet(void);


/**
 * tccSubtitle_getClassNums returns the number of classes
 * 
 * @return tccSubtitle_ErrorCodes_t
 * @retval tccSubtitle_Success - Success.
 */
extern tccSubtitle_ErrorCodes_t tccSubtitle_getClassNums(int *out_iClassNums);


/**
 * tccSubtitle_getClassName returns the class name
 * 
 * @param [in] in_iClassIndex - Class index
 * @param [out] out_pClassName - Class Name (by UTF8)
 * 
 * @return tccSubtitle_ErrorCodes_t
 * @retval tccSubtitle_Success - Success.
 */
extern tccSubtitle_ErrorCodes_t tccSubtitle_getClassName(int in_iClassIndex, char **out_pClassName);


/**
 * tccSubtitle_selectClass selects the classes displayed
 * 
 * @return tccSubtitle_ErrorCodes_t
 * @retval tccSubtitle_Success - Success.
 */
extern tccSubtitle_ErrorCodes_t tccSubtitle_selectClass(int in_iNums, int *in_pClassIndex);


#ifdef __cplusplus
}
#endif

#endif /* _TCC_SUBTITLE_INTERFACE_H_ */

