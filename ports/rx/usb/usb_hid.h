/******************************************************************************
* DISCLAIMER:
* The software supplied by Renesas Technology Europe Ltd is
* intended and supplied for use on Renesas Technology products.
* This software is owned by Renesas Technology Europe, Ltd. Or
* Renesas Technology Corporation and is protected under applicable
* copyright laws. All rights are reserved.
*
* THIS SOFTWARE IS PROVIDED "AS IS". NO WARRANTIES, WHETHER EXPRESS,
* IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* APPLY TO THIS SOFTWARE. RENESAS TECHNOLOGY AMERICA, INC. AND
* AND RENESAS TECHNOLOGY CORPORATION RESERVE THE RIGHT, WITHOUT
* NOTICE, TO MAKE CHANGES TO THIS SOFTWARE. NEITHER RENESAS
* TECHNOLOGY AMERICA, INC. NOR RENESAS TECHNOLOGY CORPORATION SHALL,
* IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
* CONSEQUENTIAL DAMAGES FOR ANY REASON WHATSOEVER ARISING OUT OF THE
* USE OR APPLICATION OF THIS SOFTWARE.
******************************************************************************/

/* Copyright (C) 2009. Renesas Technology Europe, All Rights Reserved */

/**********************************************************************
* File Name		: usb_hid.h
* Version 		: 1.00
* Device 		: Renesas Generic MCU.
* Tool Chain 	: HEW
* H/W Platform	: RSK Generic
* Description 	: Human Interface Device (HID) USB Class.
				Supports an IN and an OUT HID Report.

				NOTE: This module does not have any knowledge of the
				contents of the HID reports.	  
******************************************************************************/

/******************************************************************************
* History 		: 07.05.2009 Ver. 1.00 First Release
**********************************************************************/

/*��FILE COMMENT��******************************* Technical reference data ****
* File Name		: usb_hid.h
* Version 		: 1.00
* Device 		: Renesas Generic MCU.
* Tool Chain 	: HEW
* H/W Platform	: RSK Generic
* Description 	: Human Interface Device (HID) USB Class.
				Supports an IN and an OUT HID Report.

				NOTE: This module does not have any knowledge of the
				contents of the reports.	  
******************************************************************************/

/******************************************************************************
* History 		: 07.05.2009 Ver. 1.00 First Release
*��FILE COMMENT END��*********************************************************/

#ifndef USB_HID_H
#define USB_HID_H

/***********************************************************************************
User Includes
***********************************************************************************/
#include "usb_common.h"
#include "usbdescriptors.h"

#if defined(USB_HID)
/***********************************************************************************
Type Definitions
***********************************************************************************/
typedef void(*CB_REPORT_OUT)(uint8_t(*)[OUTPUT_REPORT_SIZE]);

/***********************************************************************************
Function Prototypes
***********************************************************************************/
USB_ERR USBHID_Init(uint8_t *report, int size,
					CB_REPORT_OUT _cb);
					
USB_ERR USBHID_ReportIN(uint8_t *report, int size);
#endif

#endif /*USB_HID_H*/
