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

/***********************************************************************
* File Name		: ram_disk.c
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
**********************************************************************/


/***********************************************************************************
System Includes
***********************************************************************************/
#include <string.h>

/***********************************************************************************
User Includes
***********************************************************************************/
#include "FAT_Image_Arrays.h"
#include "ram_disk.h"


/***********************************************************************************
Defines
***********************************************************************************/
/*	Note - This RAM disk must be larger than h'28 * h'200
	otherwise Windows will not format it*/
/*  Note: If changing the size of the RAM disk then the example FAT Image
may not be suitable. In which case make sure 'FORMAT_WITH_FAT_Example'
is not defined.*/
#define NUM_BLOCKS 0x00000060 /*Number of logical blocks*/
#define BLOCK_SIZE 0x00000200 /*512 BYTES within a block*/

/*	If FORMAT_WITH_FAT_Example is defined then the RAM disk will
	be initialised with the data in the FATExample section.
	This can be a FAT image.
	If FORMAT_WITH_FAT_Example is not defined then the RAM disk
	will be unformatted. This will cause Windows to ask if you want
	to format it.*/
//#define FORMAT_WITH_FAT_Example

/***********************************************************************************
Variables
***********************************************************************************/
/*The RAM storage*/	
/*NOTE1: When using this as an example on other hardware where external RAM may be
avialable you can create the disk in the external space. To do this locate the
g_RAM_DATA array in external memory. In this case the size of the RAM disk can
be increased using the "NUM_BLOCKS" define above. Please also note the
limitation below.*/	
static uint8_t g_RAM_DATA[NUM_BLOCKS*(BLOCK_SIZE)];

/**********************************************************************
* Outline 		: RamDiskInit
* Description 	: Initialise this RAM disk.
*				  Copies the contents in section FATExample to it.
* Argument  	: none
* Return value  : none
**********************************************************************/

void RamDiskInit(void)
{
	#ifdef FORMAT_WITH_FAT_Example
	
			/* In order to conserve ROM the whole FAT image has not been stored:
			The parts of the FAT image that contain non zero data have been
			seperated out into 5 seperate sections.*/
			
			size_t SectionSize;
			
			/*Start by setting whole whole RAMDisk data to zero and then
			can copy in the sections that contain non-zero data */
			memset(g_RAM_DATA, 0, (size_t)NUM_BLOCKS*BLOCK_SIZE);
			
			/*1st section at offset h'0 and length h'200*/
			SectionSize = (size_t)0x200;		
			memcpy((void*)(g_RAM_DATA + 0), (void*)FAT_Image_Array_0000_200, SectionSize);
			
			/*2nd section at offset h'C00*/
			SectionSize = 0x05;	
			memcpy((void*)(g_RAM_DATA + 0xC00), (void*)FAT_Image_Array_0C00_005, SectionSize);
				 
			/*3rd section at offset h'E00*/
			SectionSize = 0x05;	
			memcpy((void*)(g_RAM_DATA + 0xE00), (void*)FAT_Image_Array_0E00_005, SectionSize);
	
			/*4th section at offset h'1000*/
			SectionSize = 0x100;	
			memcpy((void*)(g_RAM_DATA + 0x1000), (void*)FAT_Image_Array_1000_100, SectionSize);
	
			/*5th section at offset h'5000*/
			SectionSize = 0x41;	
			memcpy((void*)(g_RAM_DATA + 0x5000), (void*)FAT_Image_Array_5000_041, SectionSize);
		
	#else /*FORMAT_WITH_FAT_Example*/
	
		/*Zero all, then Windows will ask for it to be formatted*/
		memset(g_RAM_DATA, 0, (size_t)NUM_BLOCKS*BLOCK_SIZE);	
	#endif
}
/**********************************************************************************
End of function RamDiskInit
***********************************************************************************/   

/**********************************************************************
* Outline 		: RamDiskGetBlockSize
* Description 	: The RAM is split into blocks.
*				  This returns the size of a block.
* Argument  	: none
* Return value  : Block size in bytes.
**********************************************************************/

uint16_t RamDiskGetBlockSize(void)
{
	return BLOCK_SIZE;
}
/**********************************************************************************
End of function RamDiskGetBlockSize
***********************************************************************************/   

/**********************************************************************
* Outline 		: RamDiskGetNumBlocks
* Description 	: The RAM is split into blocks.
*				  This returns the number of blocks.
* Argument  	: none
* Return value  : Number of blocks. 
**********************************************************************/

uint16_t RamDiskGetNumBlocks(void)
{
	return NUM_BLOCKS;
}
/**********************************************************************************
End of function RamDiskGetNumBlocks
***********************************************************************************/   

/**********************************************************************
* Outline 		: RamDiskGetBuffer
* Description 	: Provides direct access to the RAM at a specified LBA.
* Argument  	: _LBA: Logical Block Address
* Return value  : Pointer to the actual RAM.
**********************************************************************/

uint8_t* RamDiskGetBuffer(uint32_t _LBA)
{
	return &g_RAM_DATA[_LBA*BLOCK_SIZE];
}
/**********************************************************************************
End of function RamDiskGetBuffer
***********************************************************************************/
