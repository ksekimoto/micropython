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

/******************************************************************************
* File Name		: usb_msc.h
* Version 		: 1.00
* Device 		: Renesas Generic MCU.
* Tool Chain 	: HEW
* H/W Platform	: RSK Generic
* Description 	: USB Mass Storage Class.
			A windows PC will mount this as a drive.
			This includes a simple RAM disk so that data can be
			read and written to the drive from Windows.

			After USB enumeration this will handle SCSI commands wrapped
			up in Command Block Wrappers (CBWs).
			A Command Status Wrapper (CSW) will end all SCSI commands. 
******************************************************************************/

/******************************************************************************
* History 		: 26.05.2010 Ver. 1.00 First Release
******************************************************************************/

#ifndef USB_MSC_H
#define USB_MSC_H

/***********************************************************************************
User Includes
***********************************************************************************/
#include "usb_common.h"
#include "usb_msc_scsi.h"

/***********************************************************************************
Defines
***********************************************************************************/
/* Mass Storage Class Requests*/
#define BULK_ONLY_MASS_STORAGE_RESET    0xFF
#define GET_MAX_LUN                     0xFE

/*Support a single interface*/
#define INTERFACE_NUMBER    0
/*Support a single LUN*/
#define LUN_VALUE 0

/*SCSI*/
/*Size of Command Block Wrapper */
#define CBW_SIZE 31
/*Ask for more than CBW so know if host sends too much*/
#define CBW_REQUEST_SIZE (CBW_SIZE+1)

/*Size of Command Block within CBW*/
#define CBWCB_SIZE SCSI_CDB_SIZE
/*Command Status Wrapper */
#define CSW_SIZE 13
/*CBW signature value*/
#define CBW_SIGNATURE 0x43425355
/*CSW signature value*/
#define CSW_SIGNATURE 0x53425355

/*Command Block Wrapper (CBW)
Sent from host in little endian order*/
typedef struct CBW
{
    uint32_t dCBWSignature;
    uint32_t dCBWTag;
    uint32_t dCBWDataTransferLength;
    uint8_t bmCBWFlags;

    uint8_t Reserved1       :4;
    uint8_t bCBWLUN         :4;

    uint8_t Reserved2       :3;
    uint8_t bCBWCBLength  :5;

    uint8_t CBWCB[CBWCB_SIZE]; /*This holds the Command Data Block*/
}CBW;

/*Command Status Wrapper (CSW)
(This must be sent to host in little endian)*/
typedef struct CSW
{
    uint32_t dCSWSignature;
    uint32_t dCSWTag;
    uint32_t dCSWDataResidue;
    uint8_t bCSWStatus;
}CSW;

/*Value to write in the bCSWStatus field of the CSW */
typedef enum CSW_STATUS
{
    CSW_STATUS_PASSED       = 0,
    CSW_STATUS_FAILED       = 1,
    CSW_STATUS_PHASE_ERROR  = 2
}CSW_STATUS;

/*The "13 conditions" that this can find itself in after being sent a CBW*/
typedef enum STATUS
{
    STATUS_Hn_Dn, /*Host expects no data, Device expects no data*/
    STATUS_Hn_Di, /*Host expects no data, Device expects data IN*/
    STATUS_Hn_Do, /*Host expects no data, Device expects data OUT*/
    STATUS_Hi_Dn,
    STATUS_Hi_More_Di,
    STATUS_Hi_Equall_Di,
    STATUS_Hi_Less_Di,
    STATUS_Hi_Do,
    STATUS_Ho_Dn,
    STATUS_Ho_Di,
    STATUS_Ho_More_Do,
    STATUS_Ho_Equall_Do,
    STATUS_Ho_Less_Do
}STATUS;

/*General purpose data buffer */
typedef struct DataBuff
{
    uint8_t* pucBuf;
    uint32_t NumBytes;
}DataBuff;

/*Structure of data relating to SCSI commands*/
typedef struct SCSI
{
    STATUS m_Status;
    SCSI_PHASE Phase;
    CBW oCBW;
    CSW oCSW;
    DataBuff m_DataBuff;
}SCSI;

/***********************************************************************************
Function Prototypes
***********************************************************************************/
USB_ERR USBMSC_Init(void);

#endif /*USB_MSC_H*/
