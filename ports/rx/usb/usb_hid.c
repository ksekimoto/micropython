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
******************************************************************************/
/* Copyright (C) 2009. Renesas Electronics Europe, All Rights Reserved */
/**********************************************************************
* File Name		: usb_hid.c
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

/***********************************************************************************
System Includes
***********************************************************************************/
#include <assert.h>

/***********************************************************************************
User Includes
***********************************************************************************/
#include "usb_common.h"

#if defined(USB_HID)
#include "usb_hal.h"
#include "usb_core.h"
#include "usbdescriptors.h"
#include "usb_hid.h"

/***********************************************************************************
Defines
***********************************************************************************/
/* HID Descriptor Types */
#define HID_DESCRIPTOR 			0x21
#define HID_REPORT_DESCRIPTOR 	0x22

/* HID Class Request Codes */
#define GET_REPORT 0x01
#define SET_REPORT 0x09

/***********************************************************************************
Local Types
***********************************************************************************/

/***********************************************************************************
Variables
***********************************************************************************/
/*OUT Report buffer*/ 
static uint8_t g_OutputReport[OUTPUT_REPORT_SIZE];
/*IN Report buffer*/
//static uint8_t g_InputReport[INPUT_REPORT_SIZE];
static uint8_t g_InputReport[INPUT_REPORT_DEF_SIZE];
/*Callback when a Report OUT is received */
CB_REPORT_OUT g_CBReportOUT;

/***********************************************************************************
Private Function Prototypes
***********************************************************************************/
static USB_ERR ProcessStandardSetupPacket(SetupPacket* _pSetupPacket,
											uint16_t* _pNumBytes,
										 	uint8_t** _ppBuffer);
											
static USB_ERR ProcessGetDescriptor(SetupPacket* _pSetupPacket,
										uint16_t* _pNumBytes,
										 const uint8_t** _ppBuffer);
											
static USB_ERR ProcessClassSetupPacket(SetupPacket* _pSetupPacket,
										uint16_t* _pNumBytes,
										uint8_t** _ppBuffer);

/*Callbacks required by USB_CORE*/
USB_ERR CBUnhandledSetupPacketHID(SetupPacket* _pSetupPacket,
										uint16_t* _pNumBytes,
										uint8_t** _ppBuffer);
										
void CBDoneControlOutHID(USB_ERR err, uint32_t NumBytes);
void CBCableHID(bool _bConnected);
void CBErrorHID(USB_ERR _err);
void CBDoneInterruptINHID(USB_ERR _err);

static void CopyInputReport(uint8_t *report, int size);
//void CopyInputReportHID();

//void CopyInputReportHID(void) {
//    CopyInputReport(&g_InputReport);
//}

/**********************************************************************
* Outline 		: USBHID_Init
* Description 	: Initialises this module.
* Argument  	: _ReportIN - A report to send to the host when it asks
*				  for it.
*				  _cb - A Callback function to be called when a OUT
*				  report is received.
* Return value  : Error Code.
**********************************************************************/

USB_ERR USBHID_Init(uint8_t *report, int size,
					CB_REPORT_OUT _cb)
{
	USB_ERR err;	
	
	/*Initialise the USB core*/
	err = USBCORE_Init(gStringDescriptorManufacturer.pucData,
					   gStringDescriptorManufacturer.length,
					   gStringDescriptorProduct.pucData,
					   gStringDescriptorProduct.length,
					   gStringDescriptorSerialNum.pucData,
					   gStringDescriptorSerialNum.length,
					   gDeviceDescriptor.pucData,
					   gDeviceDescriptor.length,
					   gConfigurationDescriptor.pucData,
					   gConfigurationDescriptor.length,
					   (CB_SETUP_PACKET)CBUnhandledSetupPacketHID,
					   (CB_DONE_OUT)CBDoneControlOutHID,
					   (CB_CABLE)CBCableHID,
					   (CB_ERROR)CBErrorHID);
	
	/*Store call back*/				   
	g_CBReportOUT = _cb;
	
	/*Initilise the IN Report*/
	CopyInputReport(report, size);
	
	return err;						   
}
/***********************************************************************************
End of function USBHID_Init
***********************************************************************************/

/**********************************************************************
* Outline 		: USBHID_ReportIN 
* Description 	: Send IN Report to host.
*				  Uses Interrupt IN.
* Argument  	: _ReportIN: The report to send to the host.
* Return value  : Error code
**********************************************************************/

USB_ERR USBHID_ReportIN(uint8_t *report, int size)
{
	USB_ERR err = USB_ERR_OK;
	
	CopyInputReport(report, size);
	
	/*Send Report using INTERRUPT IN*/
	USBHAL_Interrupt_IN_HID(size, g_InputReport, CBDoneInterruptINHID);
	
	return err;
}
/***********************************************************************************
End of function USBHID_ReportIN
***********************************************************************************/

/**********************************************************************
* Outline 		: CBUnhandledSetupPacket
* Description 	: Called from USB core when it can't deal with a setup
* 				  packet.
*				  This is a function of type CB_SETUP_PACKET.
*				
* Argument  	: _pSetupPacket - Setup packet.
*				  _pNumBytes - (OUT)Buffer size.
*				  _ppBuffer - (OUT)Buffer.
*				
* Return value  : Error code
**********************************************************************/

USB_ERR CBUnhandledSetupPacketHID(SetupPacket* _pSetupPacket,
									uint16_t* _pNumBytes,
									uint8_t** _ppBuffer)
{
	USB_ERR err;
	
	switch(EXTRACT_bmRequest_TYPE(_pSetupPacket->bmRequest))
	{
		case REQUEST_STANDARD:
		{
			/* Standard Type */
			err = ProcessStandardSetupPacket(_pSetupPacket,
											 _pNumBytes, _ppBuffer);
			break;
		}
		case REQUEST_CLASS:
		{
			/*Class Type */
			err = ProcessClassSetupPacket(_pSetupPacket, _pNumBytes, _ppBuffer);
			break;
		}
		case REQUEST_VENDOR:
		default:
		{
			DEBUG_MSG_LOW(("USBHID: Unsupported Request type\r\n"));
			err = USB_ERR_UNKNOWN_REQUEST;
		}
	}
	
	return err;
}
/**********************************************************************************
End of function CBUnhandledSetupPacket
***********************************************************************************/   

/**********************************************************************
* Outline 		: ProcessStandardSetupPacket
* Description 	: Process a Standard Setup Packet that the lower layers couldn't.
*				  This function does not return.
* Argument  	: _pSetupPacket: Setup Packet
*				  _pNumBytes: (OUT)If this  can handle this then
*				 			  this will be set with the size of the data.
*				 			  (If there is no data stage then this will be set to zero)
*				  _ppBuffer: (OUT)If this  can handle this then
*				 			  this will be set to point to the data(IN) or a buffer(OUT).
*				 			  (If there is a data stage for this packet).
* Return value  : Error Code
**********************************************************************/

static USB_ERR ProcessStandardSetupPacket(SetupPacket* _pSetupPacket,
											uint16_t* _pNumBytes,
										 	uint8_t** _ppBuffer)
{
	USB_ERR err;
		
	switch(_pSetupPacket->bRequest)
	{
		case GET_DESCRIPTOR:
		{
			err = ProcessGetDescriptor(_pSetupPacket, _pNumBytes,
									(const uint8_t**)_ppBuffer);
			break;
			
		}
		default:
		{
			DEBUG_MSG_LOW(("USBHID: Unsupported Standard Setup request %d\r\n",
						 _pSetupPacket->bRequest));
			err = USB_ERR_UNKNOWN_REQUEST;	
		}
	}

	return err;
}
/**********************************************************************************
End of function ProcessStandardSetupPacket
***********************************************************************************/   

/**********************************************************************
* Outline 		: ProcessGetDescriptor
* Description 	: Process a Get Descriptor request that the lower layers couldn't.
* Argument  	: _pSetupPacket: Setup Packet
*				  _pNumBytes: (OUT)If this  can handle this then
*				 			  this will be set with the size of the data.
*				 			  (If there is no data stage then this will be set to zero)
*				  _ppBuffer: (OUT)If this  can handle this then
*				 			  this will be set to point to the data(IN)(Descriptor).
*				 			  (If there is a data stage for this packet).
* Return value  : Error Code
**********************************************************************/

static USB_ERR ProcessGetDescriptor(SetupPacket* _pSetupPacket,
										uint16_t* _pNumBytes,
										 const uint8_t** _ppBuffer)
{
	USB_ERR err = USB_ERR_OK;
	
	/*USB Core handles normal Get Descriptor requests,
	so only need to support HID specific here.*/
	uint8_t DescriptorType = (uint8_t)((_pSetupPacket->wValue >> 8) & 0x00FF);
	switch(DescriptorType)
	{
		/* Hid Descriptor */
		case HID_DESCRIPTOR:
		{
			DEBUG_MSG_LOW(("USBHID: GET_DESCIPTOR - HID_DESCRIPTOR\r\n"));
#ifdef USB_DEBUG_DESCRIPTOR
    debug_printf("HID DESCRIPTTOR: SIZE=%d\r\n", HID_DESCRIPTOR_SIZE);
#endif
			/* The HID Descriptor is stored within the configuration descriptor */
			/*Data IN response */
			*_pNumBytes = HID_DESCRIPTOR_SIZE;
			*_ppBuffer = &gConfigurationDescriptor.
							pucData[START_INDEX_OF_HID_WITHIN_CONFIG_DESC];
			break;
		}

		/* HID Report Descriptor */
		case HID_REPORT_DESCRIPTOR:
		{
#ifdef USB_DEBUG_DESCRIPTOR
    debug_printf("HID_REPORT: SIZE=%d\r\n", gHIDReportDescriptor.length);
#endif
            DEBUG_MSG_LOW(("USBHID: GET_DESCIPTOR - HID_REPORT_DESCRIPTOR\r\n"));
			/*Data IN response */
			*_pNumBytes = gHIDReportDescriptor.length;
			*_ppBuffer = gHIDReportDescriptor.pucData;

			break;
		}
		default:
		{
			DEBUG_MSG_LOW(("USBHID: Unsupported GetDescriptor type %d\r\n",
						 DescriptorType));
			err = USB_ERR_UNKNOWN_REQUEST;	
		}
	}
	
	return err;
}
/**********************************************************************************
End of function ProcessGetDescriptor
***********************************************************************************/   

/**********************************************************************
* Outline 		: ProcessClassSetupPacket
* Description 	: Process HID Class specific requests.
* Argument  	: _pSetupPacket: Setup Packet
*				  _pNumBytes: (OUT)If this  can handle this then
*				 			  this will be set with the size of the data.
*				 			  (If there is no data stage then this will be set to zero)
*				  _ppBuffer: (OUT)If this  can handle this then
*				 			  this will be set to point to the data(IN) or a buffer(OUT).
*				 			  (If there is a data stage for this packet).
* Return value  : Error Code
**********************************************************************/

static USB_ERR ProcessClassSetupPacket(SetupPacket* _pSetupPacket,
										uint16_t* _pNumBytes,
										 uint8_t** _ppBuffer)
{
	USB_ERR err = USB_ERR_OK;

	switch(_pSetupPacket->bRequest)
	{
		case SET_REPORT:
		{
			DEBUG_MSG_LOW(("USBHID: SET_REPORT\r\n"));
#ifdef USB_DEBUG_SPECIAL
    debug_printf("SET_REPORT\r\n");
#endif			/*HOST is going to send the output report*/
			/*Data OUT*/
			*_pNumBytes = OUTPUT_REPORT_SIZE;
			*_ppBuffer = g_OutputReport;
			break;
		}
		case GET_REPORT:
		{
			DEBUG_MSG_LOW(("USBHID: GET_REORT\r\n"));
#ifdef USB_DEBUG_SPECIAL
    debug_printf("GET_REPORT\r\n");
#endif
			/*HOST has requested the input report*/
			/*Data IN*/
			*_pNumBytes = INPUT_REPORT_DEF_SIZE;
			*_ppBuffer = g_InputReport;
			break;
		}
		default:
		{
			DEBUG_MSG_LOW(("USBHID: Unsupported Class Setup request %d\r\n",
						 _pSetupPacket->bRequest));
			err = USB_ERR_UNKNOWN_REQUEST;
		}
	}
	
	return err;
}
/***********************************************************************************
End of function ProcessClassSetupPacket
***********************************************************************************/

/**********************************************************************
* Outline 		: CBDoneControlOut
* Description 	: A Control Out has completed in response to a
*				  setup packet handled in CBUnhandledSetupPacket.
* Argument  	: _err: Error Code
*				  _NumBytes: Number of bytes received.
* Return value  : none
**********************************************************************/

void CBDoneControlOutHID(USB_ERR _err, uint32_t _NumBytes)
{
	DEBUG_MSG_LOW(("USBHID: CBDoneControlOut. Bytes =  %ld\r\n", _NumBytes));
						 
	/*Assume this is a SET_REPORT data stage*/
	assert(USB_ERR_OK == _err);
	assert(OUTPUT_REPORT_SIZE == _NumBytes);
	
	/*Call registered callback*/
	g_CBReportOUT((uint8_t(*)[OUTPUT_REPORT_SIZE])g_OutputReport);
}

/***********************************************************************************
End of function CBDoneControlOut
***********************************************************************************/   

/**********************************************************************
* Outline 		: CBDoneInterruptIN 
* Description 	: A Control IN has completed in response to a
*				  setup packet handled in CBUnhandledSetupPacket.
* Argument  	: _err: Error Code
* Return value  : none
**********************************************************************/

void CBDoneInterruptINHID(USB_ERR _err)
{
	assert(USB_ERR_OK == _err);
	/*Nothing to do*/
} 
/**********************************************************************************
End of function CBDoneInterruptIN
***********************************************************************************/   

/**********************************************************************
* Outline 		: CBCable 
* Description 	: Callback function called when the USB cable is
*				  Connected/Disconnected.
*				  Sets connected flag.
*				
* Argument  	: _bConnected: true = Connected, false = Disconnected.
* Return value  : none
**********************************************************************/

void CBCableHID(bool _bConnected)
{	
	if(true == _bConnected)
	{
		DEBUG_MSG_LOW(("USBHID: Cable Connected\r\n"));
	}
	else
	{
		DEBUG_MSG_LOW(("USBHID: Cable Disconnected\r\n"));
	}
}
/**********************************************************************************
End of function CBCable
***********************************************************************************/   

/**********************************************************************
* Outline 		: CBError 
* Description 	: One of the lower layers has reported an error.
*				  Not expecting this but try resetting HAL.
* Argument  	: none
* Return value  : none
**********************************************************************/

void CBErrorHID(USB_ERR _err)
{
	DEBUG_MSG_LOW(("USBHID: ***CBError***\r\n"));
	assert(0);
	
	/*Reset HAL*/
	USBHAL_Reset();
}
/**********************************************************************************
End of function CBError
***********************************************************************************/   

/**********************************************************************
* Outline 		: CopyInputReport
* Description 	: Copy supplied input report to input report buffer.
* Argument  	: _ReportIN - Input report
* Return value  : none
**********************************************************************/

static void CopyInputReport(uint8_t *report, int size)
{
	uint16_t index;
		
	/*Copy INPUT Report - so can send it whenever a GET_REPORT
	request is received.*/
	for(index = 0; index<size; index++)
	{
		g_InputReport[index] = report[index];
	}
}
/**********************************************************************************
End of function CopyInputReport
***********************************************************************************/   

#endif
