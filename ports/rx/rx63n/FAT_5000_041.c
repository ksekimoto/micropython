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

/************************************************************************
* File Name		: FAT_5000_041.c
* Version 		: 1.00
* Device 		: Renesas Generic MCU.
* Tool Chain 	: HEW
* H/W Platform	: RSK Generic
* Description 	: This is part of an example FAT image.
*                 Offset h'5000
*                 Length h'041
******************************************************************************/

/******************************************************************************
* History 		: 30.10.2008 Ver. 1.00 First Release
**********************************************************************/

/******************************************************************************
System Includes
******************************************************************************/
#include "FAT_Image_Arrays.h"

const unsigned char FAT_Image_Array_5000_041[0x41] =
{0x52, 0x65, 0x6E, 0x65, 0x73, 0x61, 0x73, 0x0D, 
0x0A, 0x0D, 0x0A, 0x55, 0x53, 0x42, 0x20, 0x4D, 
0x61, 0x73, 0x73, 0x20, 0x53, 0x74, 0x6F, 0x72, 
0x61, 0x67, 0x65, 0x20, 0x43, 0x6C, 0x61, 0x73, 
0x73, 0x20, 0x44, 0x65, 0x6D, 0x6F, 0x6E, 0x73, 
0x74, 0x72, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x2E, 
0x0D, 0x0A, 0x0D, 0x0A, 0x45, 0x78, 0x61, 0x6D, 
0x70, 0x6C, 0x65, 0x20, 0x66, 0x69, 0x6C, 0x65, 
0x2E};
