/*******************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only
 * intended for use with Renesas products. No other uses are authorized.
 * This software is owned by Renesas Electronics Corporation and is  protected
 * under all applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES
 * REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY,
 * INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR  A
 * PARTICULAR PURPOSE AND NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE  EXPRESSLY
 * DISCLAIMED.
 * TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
 * ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE  LIABLE
 * FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES
 * FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS
 * AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this
 * software and to discontinue the availability of this software.
 * By using this software, you agree to the additional terms and
 * conditions found by accessing the following link:
 * http://www.renesas.com/disclaimer
 ******************************************************************************/
 /* Copyright (C) 2010 Renesas Electronics Corporation. All rights reserved.*/

/*””FILE COMMENT””******************************* Technical reference data ****
* File Name		: ram_disk.h
* Version 		: 1.00
* Device 		: Renesas Generic MCU.
* Tool Chain 	: HEW
* H/W Platform	: RSK Generic
* Description 	: Simple RAM Disk.
			Accesses RAM like a block driver.
			Provides access directly to the RAM.
			Initialised with a FAT file system as stored
			in section FATExample.
******************************************************************************/

/******************************************************************************
* History 		: 07.05.2009 Ver. 1.00 First Release
*””FILE COMMENT END””*********************************************************/

/***********************************************************************************
Revision History
DD.MM.YYYY OSO-UID Description
08.11.2007 RTE-PIN First Release
***********************************************************************************/
#ifndef RAM_DISK_H
#define RAM_DISK_H

/***********************************************************************************
Sysyem Includes
***********************************************************************************/
#include <stdint.h>

/***********************************************************************************
Function Prototypes
***********************************************************************************/
void		RamDiskInit(void);
uint16_t	RamDiskGetBlockSize(void);
uint16_t	RamDiskGetNumBlocks(void);
uint8_t*	RamDiskGetBuffer(uint16_t _LBA);

#endif /*RAM_DISK_H*/