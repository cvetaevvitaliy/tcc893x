/****************************************************************************
 *   FileName    : nand_board_v8.h
 *   Description : board specific functions for NAND Driver
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/
#ifndef _NAND_BOARD_H
#define _NAND_BOARD_H

#define NAND_PORT_STATUS_NAND_ON_CS0				0x00000001
#define NAND_PORT_STATUS_NAND_ON_CS1				0x00000002
#define NAND_PORT_STATUS_NAND_ON_CS2				0x00000004
#define NAND_PORT_STATUS_NAND_ON_CS3				0x00000008
#define NAND_PORT_STATUS_NAND_ON_CS(x)				(0x0000000F&(1<<x))
#define NAND_PORT_STATUS_DATA_WIDTH_MASK			0x00000030
#define NAND_PORT_STATUS_DATA_WIDTH_8BIT			0x00000010
#define NAND_PORT_STATUS_DATA_WIDTH_16BIT			0x00000020

unsigned int NAND_BD_Get_CSCountMax( void );
void NAND_BD_Init(unsigned int *puiPortStatus);
void NAND_BD_Disable_CS_Port(unsigned int uiMask);
void NAND_BD_Enable_WP_Port( void );
unsigned int NAND_BD_Disable_WP_Port( void );

#endif //_NAND_BOARD_H
