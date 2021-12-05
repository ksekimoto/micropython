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

/*******************************************************************************
* File Name		: usb_msc_scsi.h
* Version 		: 1.00
* Device 		: Renesas Generic MCU.
* Tool Chain 	: HEW
* H/W Platform	: RSK Generic
* Description 	: Handles SCSI commands for access to a Block driver.
			This was written to support the commands that
			a USB Mass Storage device needs to support.
			This uses a simple RAM disk.
******************************************************************************/

/******************************************************************************
* History 		: 07.05.2009 Ver. 1.00 First Release
******************************************************************************/

#ifndef USB_MSC_SCSI_H
#define USB_MSC_SCSI_H

/***********************************************************************************
Defines
***********************************************************************************/
/*SCSI Command IDs */
#define SCSI_ID_INQUIRY						0x12
#define SCSI_ID_READ_CAPACITY10				0x25		
#define SCSI_ID_READ10						0x28
#define SCSI_ID_REQUEST_SENSE				0x03
#define SCSI_ID_TEST_UNIT_READY				0x00
#define SCSI_ID_WRITE10						0x2A
#define SCSI_ID_MODE_SENSE6					0x1A
#define SCSI_ID_VERIFY10					0x2F
#define SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL 	0x1E
#define SCSI_READ_FORMAT_CAPACITIES			0x23

/***********************************************************************************
Type Definitions
***********************************************************************************/
typedef enum SCSI_PHASE
{
	SCSI_PHASE_CMD,
	SCSI_PHASE_DATA_IN,
	SCSI_PHASE_DATA_OUT,
	SCSI_PHASE_STATUS,
	SCSI_PHASE_CBW_ERROR	
}SCSI_PHASE;

typedef enum SCSI_ERROR
{
	SCSI_ERROR_OK,
	SCSI_ERROR_FAILED
}SCSI_ERROR;

#define SCSI_CDB_SIZE 16

/***********************************************************************************
Function Prototypes
***********************************************************************************/
SCSI_ERROR SCSI_Init(void);

SCSI_ERROR SCSI_ProcessCmd(uint8_t(*_pCDB)[SCSI_CDB_SIZE], SCSI_PHASE* _pPhaseNext,
						 	uint32_t* _pNumBytes, uint8_t** _ppBuffer);

SCSI_ERROR SCSI_Data_OUT(uint32_t _NumBytes, uint8_t* _pBuffer);

#endif /*USB_MSC_SCSI_H*/