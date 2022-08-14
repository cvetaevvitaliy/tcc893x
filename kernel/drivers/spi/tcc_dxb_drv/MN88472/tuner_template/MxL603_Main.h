/*******************************************************************************
 *
 * FILE NAME          : MxL603_Main.h
 * 
 * AUTHOR             : Brenndon Lee
 *
 * DATE CREATED       : 10/25/2010
 *
 * DESCRIPTION        : Debug header files 
 *
 *******************************************************************************
 *                Copyright (c) 2010, MaxLinear, Inc.
 ******************************************************************************/

#ifndef __MXL_MAIN_H__
#define __MXL_MAIN_H__

/******************************************************************************
    Include Header Files
    (No absolute paths - paths handled by make file)
******************************************************************************/

/******************************************************************************
    Macros
******************************************************************************/

int MxL603_OEM_Init(void);
int MxL603_OEM_Tune(int freqInKhz, int bandwidthInKhz);
int MxL603_OEM_DeInit(void);
int MxL603_OEM_SetSystem(int system);


#endif /* __MXL_MAIN_H__*/
