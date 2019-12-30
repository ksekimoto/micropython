/*******************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only
* intended for use with Renesas products. No other uses are authorized. This
* software is owned by Renesas Electronics Corporation and is protected under
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
* AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
* ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software
* and to discontinue the availability of this software. By using this software,
* you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*******************************************************************************/
/* Copyright (C) 2010 Renesas Electronics Corporation. All rights reserved. */
/**********************************************************************
* File Name : usb_hid_app.h
* Version : 1.00
* Device : R5F562N8
* Tool Chain : RX Family C Compiler
* H/W Platform : RSK+RX62N
* Description : A USB HID Application.
			This USB HID application allows a host to perform the following operations
            by sending an OUTPUT report:-
					1. Toggle a LED.
					2. Read the ADC.
					3. Write to the LCD.
			The Device will tell the host, via an INPUT report, when a user
			has pressed a switch.
			See below "FORMAT OF DATA TRANSFERED BETWEEN HOST AND DEVICE".
			
			For more details see 'USB Sample Code User's Manual'.
**********************************************************************/
/**********************************************************************
* History : 27.07.2010 Ver. 1.00 First Release
**********************************************************************/

#ifndef USB_HID_APP_H
#define USB_HID_APP_H

/***********************************************************************************
User Includes
***********************************************************************************/

#include "usb_common.h"

/***********************************************************************************
Function Prototypes
***********************************************************************************/
void USB_HID_APP_Main(void);

#endif