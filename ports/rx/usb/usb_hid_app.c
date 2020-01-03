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
* File Name : usb_hid_app.c
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

/***********************************************************************************
System Includes
***********************************************************************************/
//#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* IO Port RPDL function defintions */
#include "r_pdl_io_port.h"
/* ADC RPDL function definitions */
#include "r_pdl_adc_10.h"
/* General RPDL function definitions */
#include "r_pdl_definitions.h"

/***********************************************************************************
User Includes
***********************************************************************************/
#include "hwsetup.h"
#include "switch.h"
#include "lcd.h"
#include "usb_hid.h"
#include "usb_hid_app.h"

/***********************************************************************************
Defines
***********************************************************************************/
#define LED0 	PDL_IO_PORT_0_2
#define LED_ON	0
#define LED_OFF	1

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* FORMAT OF DATA TRANSFERED BETWEEN HOST AND DEVICE*/
/*
OUT
The host sends (OUT transfer) 17 bytes.
The first byte is a cmd byte the following 16 are the 16
ASCII LCD characters values.
The bits in the cmd byte are defined below:
*/
#define OUT_BIT_LED 0x01 /*Toggle LED*/
#define OUT_BIT_ADC 0x02 /*Read ADC*/
#define OUT_BIT_LCD 0x04 /*Update LCD*/
/*
IN
The device sends (IN transfer) 5 bytes.
The first byte contains information as defined by the bits below.
The remaining 4 bytes (may) hold a 32 bit ADC value in little endian. 
*/
#define IN_BIT_LED	0x01 /*LED state (1 = ON, 0 = OFF)*/
#define IN_BIT_ADC	0x02 /*ADC data is valid*/
#define IN_BIT_SW	0x04 /*A switch has been pressed.*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/***********************************************************************************
Variables
***********************************************************************************/
/*In Report buffer*/
static uint8_t g_INReport[INPUT_REPORT_SIZE];


/***********************************************************************************
Private Function Prototypes
***********************************************************************************/
static void CBReportOut(uint8_t(*_ReportOut)[OUTPUT_REPORT_SIZE]);
static void CreateINReport(bool _bADC, bool _bSWPressed, uint32_t _ADCValue);
static void SendINReport(bool _bADC, bool _bSWPressed, uint32_t _ADCValue);
static void SwitchHandler(void);

/**********************************************************************
* Outline 		: USB_HID_APP_Main
* Description 	: Start the HID USB sample application.
*				  This function does not return.
* Argument  	: none
* Return value  : none
**********************************************************************/

void USB_HID_APP_Main(void)
{
	volatile bool bRPDLReturn;
	
	/*LCD*/
	InitialiseLCD();
	DisplayLCD(LCD_LINE1,"Renesas ");
	DisplayLCD(LCD_LINE2,"USB HID ");
	
	DEBUG_MSG_LOW( ("\r\n*** HID App Starting ***\r\n"));
	
	/* Configure the ADC channel that is connected to the potentiometer. AN0 */
    bRPDLReturn = R_ADC_10_Create(0, PDL_ADC_10_CHANNELS_OPTION_1 | PDL_ADC_10_MODE_SINGLE,
					(uint32_t)48E6, 6E-7, PDL_NO_FUNC, 0);
	assert(true == bRPDLReturn);
	
	/*Initialise USB IN Report */
	CreateINReport(false,false,0);
	
	/*Initialise the USB HID Class*/
	USBHID_Init((uint8_t(*)[INPUT_REPORT_SIZE])g_INReport, CBReportOut);
	
	/*Register a callback for when an RSK switch is pressed*/
	SetSwitchPressCallback(SwitchHandler);
	
	/*The rest is interrupt driven.*/
	while(1);
}
/***********************************************************************************
End of function USB_HID_APP_Main
***********************************************************************************/

/**********************************************************************
* Outline 		: CBReportOut
* Description 	: Host has sent an OUT report.
*				  Decipher it.
* Argument  	: _ReportOut: Output report
* Return value  : none
**********************************************************************/
static void CBReportOut(uint8_t(*_ReportOut)[OUTPUT_REPORT_SIZE])
{
	uint16_t ADCValue;
	volatile bool bRPDLReturn;
	
	/*Toggle LED?*/
	if(0 != ((*_ReportOut)[0] & OUT_BIT_LED))
	{
		R_IO_PORT_Modify(LED0, PDL_IO_PORT_XOR, 1);
	}
	
	/*Read ADC?*/
	if(0 != ((*_ReportOut)[0] & OUT_BIT_ADC))
	{
		/* Start AD conversion */
		bRPDLReturn = R_ADC_10_Control(PDL_ADC_10_0_ON);
		assert(true == bRPDLReturn);
		
		/* Fetch ADC value */
		bRPDLReturn = R_ADC_10_Read(0, &ADCValue);
		assert(true == bRPDLReturn);
		
		DEBUG_MSG_LOW( ("HIDApp: ADC Value = h'%04x\r\n", ADCValue));
		
		/*Send response*/
		SendINReport(true, false, (uint32_t)ADCValue);
	}
	
	/*Update LCD*/
	if(0 != ((*_ReportOut[0]) & OUT_BIT_LCD))
	{
		char szLine1[NUMB_CHARS_PER_LINE+1];
		char szLine2[NUMB_CHARS_PER_LINE+1];
		szLine1[NUMB_CHARS_PER_LINE] = '\0';
		szLine2[NUMB_CHARS_PER_LINE] = '\0';
		memcpy(szLine1, &(*_ReportOut)[1], NUMB_CHARS_PER_LINE);
		memcpy(szLine2, &(*_ReportOut)[1+NUMB_CHARS_PER_LINE],
					 NUMB_CHARS_PER_LINE);
		
		DisplayLCD(LCD_LINE1, (int8_t*)szLine1);
		DisplayLCD(LCD_LINE2, (int8_t*)szLine2);
	}
}
/***********************************************************************************
End of function CBReportOut
***********************************************************************************/

/**********************************************************************
* Outline 		: CreateINReport
* Description 	: Create the IN report based on supplied params.
* Argument  	: _bADC:		true if providing new ADC value.
*				  _bSWPressed:true if switch pressed.
*				  _ADCValue:  ADC Value.
* Return value  : none
**********************************************************************/

static void CreateINReport(bool _bADC, bool _bSWPressed, uint32_t _ADCValue)
{
	uint8_t value;
	
	/*Clear to start*/
	g_INReport[0] = 0;
	
	/*Set LED Bit in report after reading IO port to see if LED is On or Off*/
	R_IO_PORT_Read(LED0, &value);
	if(LED_ON == value)
	{
		g_INReport[0] |= IN_BIT_LED;
	}

	/*Set new ADC value bit?*/
	if(0 != _bADC)
	{
		g_INReport[0] |= IN_BIT_ADC;
	}
	
	/*Set switch toggled  bit?*/
	if(0 != _bSWPressed)
	{
		g_INReport[0] |= IN_BIT_SW;
	}
	
	/*Set ADC Value (even if not setting new ADC value bit)*/
	/*Little Endian*/
	g_INReport[1] = (uint8_t)(_ADCValue & 0xFF);
	g_INReport[2] = (uint8_t)((_ADCValue >> 8) & 0xFF);
	g_INReport[3] = (uint8_t)((_ADCValue >> 12) & 0xFF);
	g_INReport[4] = (uint8_t)((_ADCValue >> 16) & 0xFF);
}
/***********************************************************************************
End of function CreateINReport
***********************************************************************************/

/**********************************************************************
* Outline 		: SendINReport
* Description 	: Create and then send IN report
* Argument  	: none
* Return value  : none
**********************************************************************/

static void SendINReport(bool _bADC, bool _bSWPressed, uint32_t _ADCValue)
{
	/*Create report*/
	CreateINReport(_bADC, _bSWPressed, _ADCValue);
	
	/*Send to host*/
	USBHID_ReportIN((uint8_t(*)[INPUT_REPORT_SIZE])g_INReport);
}
/***********************************************************************************
End of function SendINReport
***********************************************************************************/

/**********************************************************************
* Outline 		: SwitchHandler	
* Description 	: Called when a RSK switch is pressed.
* Argument  	: none
* Return value  : none
**********************************************************************/
void SwitchHandler(void)
{
	/*Send IN report to host */
	SendINReport(false, true, 0);
} 
/***********************************************************************************
End of function SwitchHandler
***********************************************************************************/

