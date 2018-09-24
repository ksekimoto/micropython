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

/***********************************************************
* File Name		: USB_MSC.c
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
**********************************************************/

/***********************************************************************************
System Includes
***********************************************************************************/
#include <string.h>
#include <assert.h>

/***********************************************************************************
User Includes
***********************************************************************************/
//#include "lcd.h"
#include "usb_common.h"
#include "usb_hal.h"
#include "usb_core.h"
#include "usbdescriptors.h"
//#include "ram_disk.h"
#include "usb_msc_scsi.h"
#include "usb_msc.h"
#include "common.h"

/***********************************************************************************
Local Types
***********************************************************************************/
	
/***********************************************************************************
Variables
***********************************************************************************/
/*SCSI related data (Initialised by usb_msc_InitialiseGlobals function)*/
volatile SCSI g_SCSI;
/*Buffer to read a CBW into*/
uint8_t g_CBW_Buffer[CBW_REQUEST_SIZE];
/*Buffer to send a CSW from*/
uint8_t g_CSW_Buffer[CSW_SIZE];

/*Response to GET_MAX_LUN - Max Logical unit number = 0 */
const uint8_t g_GET_MAX_LUN_Data = LUN_VALUE;


/***********************************************************************************
Private Function Prototypes
***********************************************************************************/
USB_ERR CBUnhandledSetupPacketMSC(SetupPacket* _pSetupPacket,
										uint16_t* _pNumBytes,
										uint8_t** _ppBuffer);
										
void CBDoneControlOutMSC(USB_ERR _err, uint32_t _NumBytes);
void CBDoneBulkOutMSC(USB_ERR _err, uint32_t _NumBytes);
static void CBDoneDiscardBulkOut(USB_ERR _err, uint32_t _NumBytes);
static void CBDoneBulkIn(USB_ERR _err);
void CBCableMSC(bool _bConnected);
void CBErrorMSC(USB_ERR _err);

static void PopulateCBW(volatile CBW* _pCBW, const uint8_t (*_Buffer)[CBW_SIZE]);
static bool CheckCBWValid(void);
						 
static USB_ERR ProcessClassSetupPacket(SetupPacket* _pSetupPacket,
										uint16_t* _pNumBytes,
										uint8_t** _ppBuffer);
static void ProcessCBW(void);
										
static STATUS GetStatus(SCSI_PHASE _DPhase, uint32_t _DNumBytes,
							SCSI_PHASE _HPhase, uint32_t _HNumBytes);

static void NextAction(void);
static void SendCSW(void);
void InitialiseGlobals(void);
void SetAllowStallClear(bool _bEnable);
									
/**********************************************************************
* Outline 		: USBMSC_Init
* Description 	: Initialise this module.
*				  Sets up the RAM disk.
*				  Initialises the USB Core (and therefore USB HAL)
*				  Configures HAL.
* Argument  	: none
* Return value  : none
**********************************************************************/

USB_ERR USBMSC_Init(void)
{
	USB_ERR err;
	
	USBHAL_CONFIG oHALConfig;
	
	/*LCD*/
	//InitialiseLCD();
	//DisplayLCD(LCD_LINE1,"Renesas ");
	//DisplayLCD(LCD_LINE2,"USB MSC ");
	
	DEBUG_MSG_LOW(("\r\n*** MSC App Starting ***\r\n"));
	
	/*RAM Disk*/
#if defined(DEBUG_USE_RAMDISK)
	RamDiskInit();
#endif
	
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
					   (CB_SETUP_PACKET)CBUnhandledSetupPacketMSC,
					   (CB_DONE_OUT)CBDoneControlOutMSC,
					   (CB_CABLE)CBCableMSC,
					   (CB_ERROR)CBErrorMSC);
	
	if(USB_ERR_OK == err)
	{				   
		/*Config HAL*/
		/*Get default/current config*/
		oHALConfig = *USBHAL_Config_Get();
		/*MSC is a special case as it mustn't send a zero length packet
		to end a BULK IN that doesn't automatically end with a short packet*/
		oHALConfig.m_bBULK_IN_No_Short_Packet = true;
		err = USBHAL_Config_Set(&oHALConfig);
	}
	
	return err;					   						   
}
/***********************************************************************************
End of function USBMSC_Init
***********************************************************************************/

/**********************************************************************
* Outline 		: CBCable 
* Description 	: Callback function called when the USB cable is
*				  Connected/Disconnected.
*				  Resets this module and initilases SCSI module.
*				
* Argument  	: _bConnected: true = Connected, false = Disconnected.
* Return value  : none
**********************************************************************/

void CBCableMSC(bool _bConnected)
{	
	if(true == _bConnected)
	{
		DEBUG_MSG_LOW(("USBMSC: Cable Connected\r\n"));
		
		/* Initialise globals */
		InitialiseGlobals();
	
		/*Initialise/Reset SCSI module */
		SCSI_Init();
		
		/*Ready to recieve a scsi command */
		g_SCSI.Phase = SCSI_PHASE_CMD;
		/*Setup BULK OUT ready to receive a CBW*/
		USBHAL_Bulk_OUT_MSC(CBW_REQUEST_SIZE, g_CBW_Buffer, (CB_DONE_OUT)CBDoneBulkOutMSC);
	}
	else
	{
		DEBUG_MSG_LOW(("USBMSC: Cable Disconnected\r\n"));
	}
}
/***********************************************************************************
End of function CBCable
***********************************************************************************/

/**********************************************************************
* Outline 		: CBError 
* Description 	: One of the lower layers has reported an error.
*				  Not expecting this but try stalling BULK IN and OUT
*				  which may force the host to issue a
*				  BULK_ONLY_MASS_STORAGE_RESET.
* Argument  	: none
* Return value  : none
**********************************************************************/

void CBErrorMSC(USB_ERR _err)
{
	DEBUG_MSG_LOW(("USBMSC: Error CallBack %d\r\n", _err));
	
	/*Don't expect this.*/
	assert(0);	
	
	/*Reset HAL*/
	USBHAL_Reset();
	
	/*Stall BULK IN and OUT until get a BULK_ONLY_MASS_STORAGE_RESET*/
	SetAllowStallClear(false);
	USBHAL_Bulk_OUT_MSC_Stall();
	USBHAL_Bulk_IN_MSC_Stall();
}
/**********************************************************************************
End of function CBError
***********************************************************************************/   

/**********************************************************************
* Outline 		: CBDoneControlOut
* Description 	: A Control Out has completed in response to a
*				  setup packet handled in CBUnhandledSetupPacket.
*				
* Argument  	: _err: error code
*				  _NumBytes: Number of bytes read into buffer
*				  supplied during CBUnhandledSetupPacket.
* Return value  : none
**********************************************************************/

void CBDoneControlOutMSC(USB_ERR _err, uint32_t _NumBytes)
{
	/*Never use control out*/
	assert(0);
}
/**********************************************************************************
End of function CBDoneControlOut
***********************************************************************************/   

/**********************************************************************
* Outline 		: usb_msc_CBDoneBulkOut
* Description 	: A USBHAL_Bulk_OUT request has completed.
*				  This will either be a CBW or the data phase of
*				  a SCSI command.
*
* Argument  	: _err: error code
*				  _NumBytes: Number of bytes read into buffer
*				  supplied during CBUnhandledSetupPacket.
* Return value  : none
**********************************************************************/

void CBDoneBulkOutMSC(USB_ERR err, uint32_t NumBytes)
{	
	if(USB_ERR_OK == err)
	{
		/*What type of data are we expecting?*/
		switch(g_SCSI.Phase)
		{
			case SCSI_PHASE_CMD:
			{
				DEBUG_MSG_MID(("USBMSC: CBDoneBulkOut - SCSI_PHASE_CMD\r\n"));
				
				/*Check  CBW_SIZE == NumBytes*/
				//assert(CBW_SIZE == NumBytes);
				if(CBW_SIZE != NumBytes)
				{
					g_SCSI.Phase = SCSI_PHASE_CBW_ERROR;
				}
				else
				{
					/*At this point g_CBW_Buffer will be filled with a new CBW*/
					ProcessCBW();
				}
				
				/*At this point g_SCSI will be set, this can then perform
				 the next required action*/
				NextAction();
					
				break;
			}	
			case SCSI_PHASE_DATA_OUT:
			{
				DEBUG_MSG_MID(("USBMSC: CBDoneBulkOut - SCSI_PHASE_DATA_OUT\r\n"));
				
				/*Pass Data to SCSI module*/
				SCSI_Data_OUT(g_SCSI.m_DataBuff.NumBytes, g_SCSI.m_DataBuff.pucBuf);
				
				/*Onto the status stage*/
				g_SCSI.Phase = SCSI_PHASE_STATUS;
		
				/*This will send the CSW*/
				NextAction();
		
				break;
			}
			default:
			{
				/*TODO - handle better?*/
				/*Unexpected data received...*/
				assert(0);
			}
		}
		
	}
	else
	{
			/*TODO - handle better?*/
		/*NOTE: Can only currently get this if request is cancelled
		because cable is removed.*/
		/*Error reported from lower layer performing this bulk out.*/
	}
}
/**********************************************************************************
End of function CBDoneBulkOut
***********************************************************************************/   

/**********************************************************************
* Outline 		: CBDoneDiscardBulkOut
* Description 	: A USBHAL_Bulk_OUT request has completed where we asked
*				  for the data to be discarded.
*				  This will be the data stage of a SCSI command.
*				  Move onto sending a CSW.
*				
* Argument  	: _err: Error code
*				  _NumBytes: Number of bytes read.
* Return value  : none
**********************************************************************/

static void CBDoneDiscardBulkOut(USB_ERR err, uint32_t NumBytes)
{
	assert(USB_ERR_OK == err);
	
	/*Onto the status stage*/
	g_SCSI.Phase = SCSI_PHASE_STATUS;

	/*This will send the CSW*/
	NextAction();
}
/**********************************************************************************
End of function CBDoneDiscardBulkOut
***********************************************************************************/   

/**********************************************************************
* Outline 		: CBDoneBulkIn
* Description 	: A Bulk IN request has completed.
*				  If this was the data stage of a SCSI command then:
*					  If we couldn't send as much as the host was expecting
*					  then stall the Endpoint.
*					  Move onto sending a CSW.
*				  If this was a CSW sent then move on to being able
*				  to receive a CBW. 
* Argument  	: _err: error code.
* Return value  : none
**********************************************************************/

static void CBDoneBulkIn(USB_ERR err)
{
	assert(USB_ERR_OK == err);
	
	if(SCSI_PHASE_DATA_IN == g_SCSI.Phase)
	{	
		DEBUG_MSG_HIGH(("USBMSC: BulkINDone - SCSI_PHASE_DATA_IN\r\n"));
		
		/* Have just sent data stage of BULK IN */
		/*If dCSWDataResidue is not zero then this is because host was expecting
		more than we have just sent it.
		In this case we have either sent dummy data or padded the data.*/
		if(0 != g_SCSI.oCSW.dCSWDataResidue)
		{
			DEBUG_MSG_MID(("USBMSC: dCSWDataResidue = %lu\r\n",
							g_SCSI.oCSW.dCSWDataResidue));
			
			/*If g_SCSI.m_DataBuff.pucBuf is NULL then this means that we will have
			sent dummy data of the correct length to the host.
			In this case a stall is not required (a residue in CSW is required)
			and we can now move onto the status stage.*/
			if(NULL == g_SCSI.m_DataBuff.pucBuf)
			{
				/*Onto the status stage*/
				g_SCSI.Phase = SCSI_PHASE_STATUS;
			}
			else
			{
				/*Correct data + padding data will have been sent*/
				/*Onto the status stage*/
				g_SCSI.Phase = SCSI_PHASE_STATUS;
			}
			
		}
		else
		{
			/*Onto the status stage*/
			g_SCSI.Phase = SCSI_PHASE_STATUS;
		}
		
		/*This will send the CSW*/
		NextAction();
	}
	else if(SCSI_PHASE_STATUS == g_SCSI.Phase)
	{
		/*Have just sent CSW*/
		DEBUG_MSG_HIGH(("USBMSC: BulkINDone - SCSI_PHASE_STATUS\r\n"));
		
		/*Ready to receive another CBW*/
		g_SCSI.Phase = SCSI_PHASE_CMD;
		
		/*Setup BULK OUT ready to receive a CBW*/
		USBHAL_Bulk_OUT_MSC(CBW_REQUEST_SIZE, g_CBW_Buffer, (CB_DONE_OUT)CBDoneBulkOutMSC);
	}
}
/***********************************************************************************
End of function CBDoneBulkIn
***********************************************************************************/

/**********************************************************************
* Outline 		: CBUnhandledSetupPacket 
* Description 	: Called from USB core when it can't deal with a setup
*				  packet.
*				  Expect the core to deal with Standard requests so
*				  this should be a Class or Vendor request.
*				  This is a function of type CB_SETUP_PACKET.
*				
* Argument  	: _pSetupPacket - Setup packet.
*				  _pNumBytes - (OUT)Buffer size.
*				  _ppBuffer - (OUT)Buffer.
*				
* Return value  : Error code. If not USB_ERR_OK then USB core will
*				  issue a stall.
**********************************************************************/

USB_ERR CBUnhandledSetupPacketMSC(SetupPacket* _pSetupPacket,
									uint16_t* _pNumBytes,
									uint8_t** _ppBuffer)
{
	USB_ERR err;
	
	switch(EXTRACT_bmRequest_RECIPIENT(_pSetupPacket->bmRequest))
	{
		case REQUEST_CLASS:
		{
			err = ProcessClassSetupPacket(_pSetupPacket, _pNumBytes, _ppBuffer);
			break;
		}
		case REQUEST_VENDOR:
		default:
			err = USB_ERR_UNKNOWN_REQUEST;
	}
	
	return err;
}
/***********************************************************************************
End of function CBUnhandledSetupPacket
***********************************************************************************/

/**********************************************************************
* Outline 		: NextAction
* Description 	: Based on the settings in g_SCSI this will perform the
*				  next required stage.
* Argument  	: none
* Return value  : none
**********************************************************************/

static void NextAction(void)
{	
	if(SCSI_PHASE_STATUS == g_SCSI.Phase)
	{
		DEBUG_MSG_MID(("USBMSC: Phase Status - %d\r\n",
						 g_SCSI.oCSW.bCSWStatus));
		
		/*Send CSW*/
		SendCSW();
	}
	else if(SCSI_PHASE_DATA_IN == g_SCSI.Phase)
	{
		DEBUG_MSG_MID(("USBMSC: Phase Data IN\r\n"));
		
		/*Send data to host*/
		USBHAL_Bulk_IN_MSC(g_SCSI.m_DataBuff.NumBytes, g_SCSI.m_DataBuff.pucBuf,
						(CB_DONE)CBDoneBulkIn);
	}
	else if(SCSI_PHASE_DATA_OUT == g_SCSI.Phase)
	{
		DEBUG_MSG_MID(("USBMSC: Phase Data OUT\r\n"));
		/*Read data from host*/
	
		/*Note g_SCSI.m_DataBuff.pucBuf may be NULL (this is not an error)
		in which case the data will be discarded*/
		if(NULL != g_SCSI.m_DataBuff.pucBuf)
		{
			USBHAL_Bulk_OUT_MSC(g_SCSI.m_DataBuff.NumBytes, g_SCSI.m_DataBuff.pucBuf,
									 (CB_DONE_OUT)CBDoneBulkOutMSC);
		}
		else
		{
			USBHAL_Bulk_OUT_MSC(g_SCSI.m_DataBuff.NumBytes, g_SCSI.m_DataBuff.pucBuf,
									 (CB_DONE_OUT)CBDoneDiscardBulkOut);
		}
	}
	else if(SCSI_PHASE_CBW_ERROR == g_SCSI.Phase)
	{
		DEBUG_MSG_LOW(("USBMSC: Phase CBW_ERROR\r\n"));
		
		/*An invalid or not meaningfull CBW has been recieved*/
		/*Stall BULK IN and OUT until get a BULK_ONLY_MASS_STORAGE_RESET*/
		SetAllowStallClear(false);
		USBHAL_Bulk_OUT_MSC_Stall();
		USBHAL_Bulk_IN_MSC_Stall();
		
		
		/*NOTE: The host should respond to this by doing a
		BULK_ONLY_MASS_STORAGE_RESET. Untill then we don't try and recieve
		another CBW.*/
	}
}
/**********************************************************************************
End of function NextAction
***********************************************************************************/   

/**********************************************************************
* Outline 		: ProcessClassSetupPacket 
* Description 	: Processes a MSC class setup packet.
*				  Provides a buffer if there is a data stage.
*				
* Argument  	: _pSetupPacket - Setup packet.
*				  _pNumBytes - (OUT)Buffer size.
*				  _ppBuffer - (OUT)Buffer.
*				
* Return value  : Error code
**********************************************************************/

static USB_ERR ProcessClassSetupPacket(SetupPacket* _pSetupPacket,
										uint16_t* _pNumBytes,
										uint8_t** _ppBuffer)
{
	USB_ERR err = USB_ERR_OK;
	
	switch(_pSetupPacket->bRequest)
	{
		case BULK_ONLY_MASS_STORAGE_RESET:
		{
			DEBUG_MSG_MID(("USBMSC: BULK_ONLY_MASS_STORAGE_RESET\r\n"));
			
			/*
			This request is used to reset the mass storage device
			and its associated interface.
			This class-specific request shall ready the device for
			the next CBW from the host.
			*/
			
			/*Check paramaters are OK*/
			/*Check wIndex equalls the only interface number we support*/
			if(_pSetupPacket->wIndex != INTERFACE_NUMBER)
			{
				DEBUG_MSG_LOW(
						("USBMSC: Error BULK_ONLY_MASS_STORAGE_RESET wIndex invalid.\r\n"));
				err = USB_ERR_INVALID_REQUEST;
			}
			/*Check wValue equall 0*/
			else if(_pSetupPacket->wValue != 0)
			{
				DEBUG_MSG_LOW(
						("USBMSC: Error BULK_ONLY_MASS_STORAGE_RESET wValue invalid.\r\n"));
				err = USB_ERR_INVALID_REQUEST;
			}
			/*Check wLength equalls 0*/
			else if(_pSetupPacket->wLength != 0)
			{
				DEBUG_MSG_LOW(
						("USBMSC: Error BULK_ONLY_MASS_STORAGE_RESET wLength invalid.\r\n"));
				err = USB_ERR_INVALID_REQUEST;
			}
			else
			{
				/*Accept command*/
					
				/*Set auto stall clear - as this may have been turned off before this.*/
				SetAllowStallClear(true);
				
				/*Make sure we are ready to recieve SCSI cmds*/
				g_SCSI.Phase = SCSI_PHASE_CMD;				
				
				/*Setup BULK OUT ready to receive a CBW*/
				USBHAL_Bulk_OUT_MSC(CBW_REQUEST_SIZE, g_CBW_Buffer, (CB_DONE_OUT)CBDoneBulkOutMSC);
				
				/*No data response */
				*_pNumBytes = 0;
			}
			break;
		}
		case GET_MAX_LUN:
		{
			/*Get the highest logical unit.
			The device shall return one byte of data that contains
			the maximum LUN supported by the device.*/
			
			DEBUG_MSG_MID(("USBMSC: GET_MAX_LUN\r\n"));
			
			/*Check paramaters are OK*/
			/*Check wIndex equalls the only interface number we support*/
			if(_pSetupPacket->wIndex != INTERFACE_NUMBER)
			{
				DEBUG_MSG_LOW(
						("USBMSC: Error GET_MAX_LUN wIndex invalid.\r\n"));
				err = USB_ERR_INVALID_REQUEST;
			}
			/*Check wValue equall 0*/
			else if(_pSetupPacket->wValue != 0)
			{
				DEBUG_MSG_LOW(
						("USBMSC: Error GET_MAX_LUN wValue invalid.\r\n"));
				err = USB_ERR_INVALID_REQUEST;
			}
			/*Check wLength equalls 1*/
			else if(_pSetupPacket->wLength != 1)
			{
				DEBUG_MSG_LOW(
						("USBMSC: Error GET_MAX_LUN wLength invalid.\r\n"));
				err = USB_ERR_INVALID_REQUEST;
			}
			else
			{
				/*We only support 1 unit so return 0
				(Note: could do a stall instead.)*/
				*_ppBuffer = (uint8_t*)&g_GET_MAX_LUN_Data;
				*_pNumBytes = 1;
			}
			break;
		}
		default:
		{
			DEBUG_MSG_MID(("USBMSC: Unsupported Class request\r\n"));
			err = USB_ERR_UNKNOWN_REQUEST;	
		}
	}

	return err;
}
/**********************************************************************************
End of function ProcessClassSetupPacket
***********************************************************************************/   

/**********************************************************************
* Outline 		: ProcessCBW 
* Description 	: Called when the CBW Buffer is filled with a new CBW.
*				  This will process the the CBW as far as
*				  setting up g_SCSI goes so that the caller of this
*				  will know the next action to do and have access to
*				  any data required.
* Argument  	: none
* Return value  : none
**********************************************************************/

static void ProcessCBW(void)
{
	SCSI_PHASE DevicePhaseExpected;
	SCSI_PHASE HostPhaseExpected;
	SCSI_ERROR scsi_err;
	uint32_t NumBytes = 0;
	uint8_t* pBuffer = NULL;
	
	/*Setup CBW structure from received data */
	PopulateCBW(&g_SCSI.oCBW, (const uint8_t(*)[CBW_SIZE])g_CBW_Buffer);
	
	/*Check CBW is 'valid and meaningful'*/
	if(false == CheckCBWValid())
	{
		DEBUG_MSG_LOW(("\r\nUSBMSC: Invalid CBW!"));
		g_SCSI.Phase = SCSI_PHASE_CBW_ERROR;		 
	}
	else
	{
		DEBUG_MSG_MID(("\r\nUSBMSC: SCSI cmd received = %#2.2X, Length = %lu\r\n",
			g_SCSI.oCBW.CBWCB[0], g_SCSI.oCBW.dCBWDataTransferLength));
		
		/*Set CSW to passed to start with*/
		g_SCSI.oCSW.bCSWStatus = (uint8_t)CSW_STATUS_PASSED;
		/*Copy tag */
		g_SCSI.oCSW.dCSWTag = g_SCSI.oCBW.dCBWTag;
		/*Set residue to 0 to start with*/
		g_SCSI.oCSW.dCSWDataResidue = 0;
	
		/*Looking at CBW, set phase that host expects next*/	
		if(0 != g_SCSI.oCBW.dCBWDataTransferLength)
		{
			if(0 == (g_SCSI.oCBW.bmCBWFlags & 0x80))
			{
				/*OUT*/
				HostPhaseExpected = SCSI_PHASE_DATA_OUT;
			}
			else
			{
				/*IN*/
				HostPhaseExpected = SCSI_PHASE_DATA_IN;
			}
		}
		else
		{
			/*No Data phase so straight to Status phase */
			HostPhaseExpected = SCSI_PHASE_STATUS;
		}
	
		/*Process the CBWCB*/
		scsi_err = SCSI_ProcessCmd((uint8_t(*)[SCSI_CDB_SIZE])g_SCSI.oCBW.CBWCB,
									&DevicePhaseExpected, &NumBytes, &pBuffer);
		/*If SCSI_ProcessCmd failed then ensure PhaseNext is set to 
		SCSI_PHASE_STATUS as a failed command won't have a data stage.*/
		if(SCSI_ERROR_OK != scsi_err)
		{
			DevicePhaseExpected = SCSI_PHASE_STATUS;
			/*Set bCSWStatus to fail but this may be upgraded to a phase error later*/
			g_SCSI.oCSW.bCSWStatus = (uint8_t)CSW_STATUS_FAILED;
		}
		
		/*The SCSI_ProcessCmd function will with commands such as READ where there
		would normally be a DATA IN stage will set DevicePhaseExpected as SCSI_PHASE_DATA_IN
		even if the numBytes == 0.
		(Hence Case 4 Command Verifier test becomes a Hi>Di rather than Hi>Dn.)
		So if NumBytes == 0 force DevicePhaseExpected to SCSI_PHASE_STATUS.*/
		if((DevicePhaseExpected != SCSI_PHASE_STATUS) && (0 == NumBytes))
		{
			DEBUG_MSG_LOW(("USBMSC: Forcing SCSI_PHASE_STATUS as no data length.\r\n"));
			DevicePhaseExpected = SCSI_PHASE_STATUS;
			/*Set bCSWStatus to fail but this may be upgraded to a phase error later*/
			g_SCSI.oCSW.bCSWStatus = (uint8_t)CSW_STATUS_FAILED;
		}
	
		/*Does the SCSI response fit with what the USB host expects?
		The answer will be 1 of the 13 cases in enum STATUS */
		g_SCSI.m_Status = GetStatus(DevicePhaseExpected, NumBytes,
								HostPhaseExpected, g_SCSI.oCBW.dCBWDataTransferLength);
	
		/*Work out what to do next based on status*/
		/*At end of this CSW and g_SCSI.Phase will be set and if there
		is a data stage so will the the data stage buffer (m_DataBuff)*/
		switch(g_SCSI.m_Status)
		{
			case STATUS_Hn_Dn:
			{
				DEBUG_MSG_MID(("USBMSC: STATUS_Hn_Dn\r\n"));
				/*Normal - No Data Transport phase */
				g_SCSI.Phase = SCSI_PHASE_STATUS;
				break;
			}
			case STATUS_Hn_Di:
			{
				DEBUG_MSG_LOW(("USBMSC: STATUS_Hn_Di\r\n"));
			
				/*Phase error as host not expecting data phase but device is*/
				g_SCSI.Phase = SCSI_PHASE_STATUS;
				g_SCSI.oCSW.bCSWStatus = (uint8_t)CSW_STATUS_PHASE_ERROR;
				break;
			}
			case STATUS_Hn_Do:
			{
				DEBUG_MSG_LOW(("USBMSC: STATUS_Hn_Do\r\n"));
			
				/*Phase error as host not expecting data phase but device is*/
				g_SCSI.Phase = SCSI_PHASE_STATUS;
				g_SCSI.oCSW.bCSWStatus = (uint8_t)CSW_STATUS_PHASE_ERROR;
				break;
			}
			case STATUS_Hi_Dn:
			{
				DEBUG_MSG_LOW(("USBMSC: STATUS_Hi_Dn\r\n"));
				
				/* Rather than stalling BULK IN send dummy data */	
				g_SCSI.Phase = SCSI_PHASE_DATA_IN;
				g_SCSI.m_DataBuff.NumBytes = g_SCSI.oCBW.dCBWDataTransferLength;
				/*Set to null so HAL will know to just write dummy data IN*/
				g_SCSI.m_DataBuff.pucBuf = NULL; 
				g_SCSI.oCSW.dCSWDataResidue = g_SCSI.oCBW.dCBWDataTransferLength - 0;
			
				break;	
			}
			case STATUS_Hi_More_Di:
			{
				DEBUG_MSG_LOW(("USBMSC: STATUS_Hi_More_Di\r\n"));
				
				/*NOTE USB spec says set bCSWStatus to 00h or 01h*/
				
				/* Rather than stalling BULK IN send data + padding data*/	
				g_SCSI.Phase = SCSI_PHASE_DATA_IN;
				g_SCSI.m_DataBuff.NumBytes = g_SCSI.oCBW.dCBWDataTransferLength;
				/*NOTE: Paddign data will just be got from reading beyond buffer*/
				g_SCSI.m_DataBuff.pucBuf = pBuffer;
				g_SCSI.oCSW.dCSWDataResidue = g_SCSI.oCBW.dCBWDataTransferLength - NumBytes;
				
				break;
			}
			case STATUS_Hi_Equall_Di:
			{
				DEBUG_MSG_HIGH(("USBMSC: STATUS_Hi_Equall_Di\r\n"));
				/*NORMAL DATA IN*/
				g_SCSI.Phase = SCSI_PHASE_DATA_IN;
				g_SCSI.m_DataBuff.NumBytes = NumBytes;
				g_SCSI.m_DataBuff.pucBuf = pBuffer;
				break;
			}
			case STATUS_Hi_Less_Di:
			{
				DEBUG_MSG_LOW(("USBMSC: STATUS_Hi_Less_Di\r\n"));
				/*Send only as much as host is expecting and then return CSW
				 with a phase error*/
				g_SCSI.Phase = SCSI_PHASE_DATA_IN;
				g_SCSI.m_DataBuff.NumBytes = g_SCSI.oCBW.dCBWDataTransferLength;
				g_SCSI.m_DataBuff.pucBuf = pBuffer;
				g_SCSI.oCSW.bCSWStatus = (uint8_t)CSW_STATUS_PHASE_ERROR; 
				break;
			}
			case STATUS_Hi_Do:
			{
				DEBUG_MSG_LOW(("USBMSC: STATUS_Hi_Do\r\n"));
				/*Phase error CSW*/
				
				/* Rather than stalling BULK IN send dummy data */		
				g_SCSI.Phase = SCSI_PHASE_DATA_IN;
				g_SCSI.m_DataBuff.NumBytes = g_SCSI.oCBW.dCBWDataTransferLength;	
				/*Set to null so HAL will know to just write dummy data IN*/
				g_SCSI.m_DataBuff.pucBuf = NULL;			
				/*Set phase error*/
				g_SCSI.oCSW.bCSWStatus = (uint8_t)CSW_STATUS_PHASE_ERROR;
				
				break;
			}
			case STATUS_Ho_Dn: /* Case 9 */
			{
				DEBUG_MSG_LOW(("USBMSC: STATUS_Ho_Dn\r\n"));
				/*Read data from host but discard as device not expecting any*/
				g_SCSI.Phase = SCSI_PHASE_DATA_OUT;
				g_SCSI.m_DataBuff.NumBytes = g_SCSI.oCBW.dCBWDataTransferLength;
				g_SCSI.m_DataBuff.pucBuf = NULL; /*So data will be discarded*/
				/*Residue is all as not processign any of it.*/
				g_SCSI.oCSW.dCSWDataResidue = g_SCSI.oCBW.dCBWDataTransferLength;
			 	break;
			}
			case STATUS_Ho_Di: /* Case 10 */
			{
				DEBUG_MSG_LOW(("USBMSC: STATUS_Ho_Di\r\n"));
				/*Read data from host but discard as device not expecting any
				(infact device expecting to send data!)*/
				g_SCSI.Phase = SCSI_PHASE_DATA_OUT;
				g_SCSI.m_DataBuff.NumBytes = g_SCSI.oCBW.dCBWDataTransferLength;
				g_SCSI.m_DataBuff.pucBuf = NULL; /*So data will be discarded*/
				/*Then report phase error */
				g_SCSI.oCSW.bCSWStatus = (uint8_t)CSW_STATUS_PHASE_ERROR;
				break;
			}
			case STATUS_Ho_More_Do: /* Case 11 */
			{
				DEBUG_MSG_LOW(("USBMSC: STATUS_Ho_More_Do\r\n"));
				/*NOTE - could use data sent from host but have decided to discard
				as probably safest as there is a discrepancy. */
				/*Read data from host but discard as device not expecting any*/
				g_SCSI.Phase = SCSI_PHASE_DATA_OUT;
				g_SCSI.m_DataBuff.NumBytes = g_SCSI.oCBW.dCBWDataTransferLength;
				g_SCSI.m_DataBuff.pucBuf = NULL; /*So data will be discarded*/
				/*Residue is all as not processign any of it.*/
				g_SCSI.oCSW.dCSWDataResidue = g_SCSI.oCBW.dCBWDataTransferLength;
				break;
			}
			case STATUS_Ho_Equall_Do: /* Case 12 */
			{
				DEBUG_MSG_MID(("USBMSC: STATUS_Ho_Equall_Do\r\n"));
				/*NORMAL DATA OUT*/
				/*Read data from host*/
				g_SCSI.Phase = SCSI_PHASE_DATA_OUT;
				g_SCSI.m_DataBuff.NumBytes = NumBytes;
				g_SCSI.m_DataBuff.pucBuf = pBuffer;
				break;
			}
			case STATUS_Ho_Less_Do: /* Case 13 */
			{
				DEBUG_MSG_LOW(("USBMSC: STATUS_Ho_Less_Do\r\n"));
				/*Read data from host but discard as device not expecting so much*/
				g_SCSI.Phase = SCSI_PHASE_DATA_OUT;
				g_SCSI.m_DataBuff.NumBytes = g_SCSI.oCBW.dCBWDataTransferLength;
				g_SCSI.m_DataBuff.pucBuf = NULL; /*So data will be discarded*/
				/*Residue is all as not processign any of it.*/
				g_SCSI.oCSW.dCSWDataResidue = g_SCSI.oCBW.dCBWDataTransferLength;
				/*Then report phase error */
				g_SCSI.oCSW.bCSWStatus = (uint8_t)CSW_STATUS_PHASE_ERROR;
				break;
			}
		}
	}
}
/**********************************************************************************
End of function ProcessCBW
***********************************************************************************/   

/**********************************************************************
* Outline 		: CheckCBWValid 
* Description 	: Checks the CBW in g_SCSI.oCBW is valid and meaningful.
* Argument  	: none
* Return value  : true if valid, false if not.
**********************************************************************/

static bool CheckCBWValid(void)
{
	bool bValid = true;
	
	/*Check for a valid signature*/
	if(CBW_SIGNATURE != g_SCSI.oCBW.dCBWSignature)
	{
		bValid = false;
	}
	
	/*Check LUN is correct*/
	if(LUN_VALUE != g_SCSI.oCBW.bCBWLUN)
	{
		bValid = false;
	}
	
	return bValid;
}
/***********************************************************************************
End of function CheckCBWValid
***********************************************************************************/

/**********************************************************************
* Outline 		: SendCSW 
* Description 	: Packages the g_SCSI.oCSW structure up into a
*				  buffer and sends to host.
*				  Note: Buffer will be in little endian.
*				
*				  This is done in a long winded way to avoid endian issues
*				  and problems with padding in the CSW structure.
* Argument  	: none
* Return value  : none
**********************************************************************/

static void SendCSW(void)
{
	g_CSW_Buffer[0] = (uint8_t)((g_SCSI.oCSW.dCSWSignature >> 0) & 0xFF);
	g_CSW_Buffer[1] = (uint8_t)((g_SCSI.oCSW.dCSWSignature >> 8) & 0xFF);
	g_CSW_Buffer[2] = (uint8_t)((g_SCSI.oCSW.dCSWSignature >> 16) & 0xFF);
	g_CSW_Buffer[3] = (uint8_t)((g_SCSI.oCSW.dCSWSignature >> 24) & 0xFF);
	
	g_CSW_Buffer[4] = (uint8_t)((g_SCSI.oCSW.dCSWTag >> 0) & 0xFF);
	g_CSW_Buffer[5] = (uint8_t)((g_SCSI.oCSW.dCSWTag >> 8) & 0xFF);
	g_CSW_Buffer[6] = (uint8_t)((g_SCSI.oCSW.dCSWTag >> 16) & 0xFF);
	g_CSW_Buffer[7] = (uint8_t)((g_SCSI.oCSW.dCSWTag >> 24) & 0xFF);
	
	g_CSW_Buffer[8] = (uint8_t)((g_SCSI.oCSW.dCSWDataResidue >> 0) & 0xFF);
	g_CSW_Buffer[9] = (uint8_t)((g_SCSI.oCSW.dCSWDataResidue >> 8) & 0xFF);
	g_CSW_Buffer[10] = (uint8_t)((g_SCSI.oCSW.dCSWDataResidue >> 16) & 0xFF);
	g_CSW_Buffer[11] = (uint8_t)((g_SCSI.oCSW.dCSWDataResidue >> 24) & 0xFF);
	
	g_CSW_Buffer[12] = g_SCSI.oCSW.bCSWStatus;
	
	USBHAL_Bulk_IN_MSC(CSW_SIZE, g_CSW_Buffer, (CB_DONE)CBDoneBulkIn);
}

/***********************************************************************************
End of function SendCSW
***********************************************************************************/   

/**********************************************************************
* Outline 		: PopulateCBW 
* Description 	: Populate the CBW structure from the supplied
*				  buffer.
*				  The buffer contains little endian data.
*				  This is done in long winded way to avoid endian issues
*				  and problems with padding in the CBW structure.
* Argument  	: _pCBW: CBW structure to be populated
*				  _Buffer: Buffer containing CBW(little endian)
* Return value  : none
**********************************************************************/

static void PopulateCBW(volatile CBW* _pCBW, const uint8_t (*_Buffer)[CBW_SIZE])
{
	_pCBW->dCBWSignature = (((uint32_t)(*_Buffer)[0]) << 0) &  0x000000FF;
	_pCBW->dCBWSignature |= (((uint32_t)(*_Buffer)[1]) << 8) &  0x0000FF00;
	_pCBW->dCBWSignature |= (((uint32_t)(*_Buffer)[2]) << 16) & 0x00FF00FF;
	_pCBW->dCBWSignature |= (((uint32_t)(*_Buffer)[3]) << 24) & 0xFF000000;
	
	_pCBW->dCBWTag = (((uint32_t)(*_Buffer)[4]) << 0) &  0x000000FF;
	_pCBW->dCBWTag |= (((uint32_t)(*_Buffer)[5]) << 8) &  0x0000FF00;
	_pCBW->dCBWTag |= (((uint32_t)(*_Buffer)[6]) << 16) & 0x00FF00FF;
	_pCBW->dCBWTag |= (((uint32_t)(*_Buffer)[7]) << 24) & 0xFF000000;
	
	_pCBW->dCBWDataTransferLength = (((uint32_t)(*_Buffer)[8]) << 0) &  0x000000FF;
	_pCBW->dCBWDataTransferLength |= (((uint32_t)(*_Buffer)[9]) << 8) &  0x0000FF00;
	_pCBW->dCBWDataTransferLength |= (((uint32_t)(*_Buffer)[10]) << 16) & 0x00FF00FF;
	_pCBW->dCBWDataTransferLength |= (((uint32_t)(*_Buffer)[11]) << 24) & 0xFF000000;
	
	_pCBW->bmCBWFlags = (*_Buffer)[12];
	
	_pCBW->Reserved1 = (uint8_t)(((*_Buffer)[13] >> 4) & 0x0F);
	_pCBW->bCBWLUN = (uint8_t)(((*_Buffer)[13]) & 0x0F);
	
	_pCBW->Reserved2 = (uint8_t)(((*_Buffer)[14] >> 5) & 0x07);
	_pCBW->bCBWCBLength = (uint8_t)(((*_Buffer)[14]) & 0x1F);
	
	memcpy((void*)_pCBW->CBWCB, &(*_Buffer)[15], CBWCB_SIZE);
}

/**********************************************************************************
End of function PopulateCBW
***********************************************************************************/   

/**********************************************************************
* Outline 		: InitialiseGlobals
* Description 	: Sets this modules data to default values.
* Argument  	: none
* Return value  : none
**********************************************************************/

void InitialiseGlobals(void)
{
	memset((void*)&g_SCSI, 0, sizeof(SCSI));
	g_SCSI.Phase = SCSI_PHASE_CMD;
	g_SCSI.oCSW.dCSWSignature = CSW_SIGNATURE;
}
/**********************************************************************************
End of function InitialiseGlobals
***********************************************************************************/   

/**********************************************************************
* Outline 		: GetStatus 
* Description 	: Given what the device expects "D" to do next and what
*				the host "H" expects next, calculate which of the
*				13 possible status values we are at.
* Argument  	: _DPhase: Phase Device thinks we should do next.
*				  _DNumBytes: Size of Data Phase (if applicable) that
*							  the Device expects. 
*				  _HPhase: Phase Host thinks we should do next.
*				  _HNumBytes: Size of Data Phase (if applicable) that
*							  the Host expects.
* Return value  : STATUS value, this will determine next action.
**********************************************************************/

static STATUS GetStatus(SCSI_PHASE _DPhase, uint32_t _DNumBytes,
							SCSI_PHASE _HPhase, uint32_t _HNumBytes)
{
	STATUS Status;
				
	if(SCSI_PHASE_DATA_IN == _DPhase)
	{
		/*Di*/
		if(SCSI_PHASE_DATA_IN == _HPhase)
		{
			/*Hi*/
			if(_HNumBytes == _DNumBytes)
			{
				Status = STATUS_Hi_Equall_Di; /*Normal*/
			}
			else if(_HNumBytes > _DNumBytes)
			{
				Status = STATUS_Hi_More_Di;
			}
			else /*(SCSI_PHASE_STATUS == _HPhase)*/
			{
				Status = STATUS_Hi_Less_Di;
			}
		}
		else if(SCSI_PHASE_DATA_OUT == _HPhase)
		{
			/*Ho*/
			Status = STATUS_Ho_Di;
		}
		else /*(SCSI_PHASE_STATUS == _HPhase)*/
		{
			/*Hn*/
			Status = STATUS_Hn_Di;
		}
	}
	else if(SCSI_PHASE_DATA_OUT == _DPhase)
	{
		/*Do*/
		if(SCSI_PHASE_DATA_IN == _HPhase)
		{
			/*Hi*/
			Status = STATUS_Hi_Do;
		}
		else if(SCSI_PHASE_DATA_OUT == _HPhase)
		{
			/*Ho*/
			if(_HNumBytes == _DNumBytes)
			{
				Status = STATUS_Ho_Equall_Do; /*Normal*/
			}
			else if(_HNumBytes > _DNumBytes)
			{
				Status = STATUS_Ho_More_Do;
			}
			else
			{
				Status = STATUS_Ho_Less_Do;
			}
		}
		else /*(SCSI_PHASE_STATUS == _HPhase)*/
		{
			/*Hn*/
			Status = STATUS_Hn_Do;
		}
	}
	else /*(SCSI_PHASE_STATUS == _DPhase)*/
	{
		/*Dn*/
		if(SCSI_PHASE_DATA_IN == _HPhase)
		{
			/*Hi*/
			Status = STATUS_Hi_Dn;
		}
		else if(SCSI_PHASE_DATA_OUT == _HPhase)
		{
			/*Ho*/
			Status = STATUS_Ho_Dn;
		}
		else /*(SCSI_PHASE_STATUS == _HPhase)*/
		{
			/*Hn*/
			Status = STATUS_Hn_Dn; /*Normal*/
		}
	}
	
	return Status;		
}
/**********************************************************************************
End of function GetStatus
***********************************************************************************/   

/**********************************************************************
* Outline 		: SetAllowStallClear
* Description 	: Configure the HIDs stall clear option
* Argument  	: _bEnable: true to enable stalls to be cleared by a Clear Feature.
*					 		false to stop stalls being cleared by a Clear Feature.
* Return value  : none
**********************************************************************/
void SetAllowStallClear(bool _bEnable)
{
	USBHAL_CONFIG oHALConfig;
	oHALConfig = *USBHAL_Config_Get();
	oHALConfig.m_bAllowStallClear = _bEnable;
	USBHAL_Config_Set(&oHALConfig);
}
/**********************************************************************************
End of function SetAllowStallClear
***********************************************************************************/   
