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

/*****************************************************************************
* File Name		: usb_msc_scsi.c
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

/***********************************************************************************
System Includes
***********************************************************************************/
#include <string.h>
#include <assert.h>

//#define DEBUG_USB_MSC

/***********************************************************************************
User Includes
***********************************************************************************/
#include "usb_common.h"
#if defined(DEBUG_USE_RAMDISK)
#include "ram_disk.h"
#endif
#include "usb_msc_scsi.h"
#include "storage.h"
#include "common.h"

uint32_t storage_get_cache_block_count(void);

/***********************************************************************************
Defines
***********************************************************************************/

/*Optional CMD support (Comment out to not support)*/
#define SUPPORT_SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL_CMD
//#define SUPPORT_MODE_SENSE_CMD

#define SCSI_BUF_SIZE   0x2000
static uint8_t scsi_buf[SCSI_BUF_SIZE] __attribute__((aligned(16)));
static uint32_t scsi_blk;

/***********************************************************************************
Local Types
***********************************************************************************/

/*SCSI INQUIRY response data*/
#define SCSI_INQUIRY_RESPONSE_SIZE 36
typedef struct SCSI_INQUIRY_RESPONSE
{
	uint8_t Peripheral;
	uint8_t Removable;
	uint8_t Version;
	uint8_t Response_Data_Format;
	uint8_t AdditionalLength;
	uint8_t Byte5;
	uint8_t Byte6;
	uint8_t Byte7;
	uint8_t vendorID[8];
	uint8_t productID[16];
	uint8_t productRev[4];
}SCSI_INQUIRY_RESPONSE;

/*SCSI Fixed Format sense data*/
#define SCSI_SENSE_DATA_SIZE 18
typedef struct SCSI_SENSE_DATA
{
	uint8_t Valid 			:1;
	uint8_t ResponseCode 		:7;
	
	uint8_t Obsolete;
	
	uint8_t Filemark 			:1;
	uint8_t EOM 				:1;
	uint8_t ILI 				:1;
	uint8_t Reserved1 		:1;
	uint8_t SenseKey 			:4;
	
	uint32_t Information;
	uint8_t AddSenseLen;
	uint32_t CmdSpecificInfo;
	uint8_t ASC;
	uint8_t ASCQ;
	uint8_t FRUC;
	uint8_t SenseKeySpecific[3];
}SCSI_SENSE_DATA;

#define SCSI_READ_CAPACITY_SIZE 8
typedef struct SCSI_READ_CAPACITY
{
	uint32_t LastLBA;		/*The LBA of the last block (i.e num blocks - 1) */
	uint32_t BlockLength; /*Number of bytes in a block*/
}SCSI_READ_CAPACITY;

/*See Book p145 - this will produce a residue of 4*/
#define SCSI_MODE_SENSE_SIZE 4
typedef struct SCSI_MODE_SENSE
{
	uint8_t buffer[SCSI_MODE_SENSE_SIZE];
}SCSI_MODE_SENSE;

/*Structre to record an expected data out*/
typedef struct DATA_OUT_EXPECTED
{
	uint32_t m_NumBytes; 
	uint16_t m_CmdID;
}DATA_OUT_EXPECTED;

/***********************************************************************************
Variables
***********************************************************************************/
/*SCSI INQUIRY_RESPONSE response (Big Endian)*/
static const SCSI_INQUIRY_RESPONSE g_SCSI_INQUIRY_RESPONSE =
{
	0x00, /*Direct access block device*/
	/*If set as NOT removable Windows will not keep sending TEST_UNIT_READY cmds,
	but really should report that the device is removable.)*/
	0x80, /*Device is removable*/
	//0x00, /*Device is NOT removable*/
	0x04, /*SPC-2 compliance*/
	0x02, /*response data format*/
	0x20, /*This has 20h+4 bytes (36 in total)*/
	0x00,
	0x00,
	0x00,
	{'R','e','n','e','s','a','s',' '}, /*Vendor  (8 Characters)*/
	/*Product (16 Characters)*/
	{'M','S','C',' ','D','e','m','o','.',' ',' ',' ',' ',' ',' ',' '},
	{'0','0','0','1'}		/*Version (4 Characters)*/
};

/*SCSI READ_CAPACITY response (Big Endian)
Initialised in SCSI_Init function*/
static SCSI_READ_CAPACITY g_SCSI_READ_CAPACITY =
{
	0,
	0
};

/*SCSI_MODE_SENSE response (Big Endian)*/
/*See Jan Axelson book "USB Mass Storage" page 145*/
/*This command is optional. See g_SCSI_MODE_SENSE usage for details*/
#ifdef SUPPORT_MODE_SENSE_CMD
static const SCSI_MODE_SENSE g_SCSI_MODE_SENSE =
{
	{/*Buffer*/
		0x03, /*The number of bytes that follow*/
		0x00, /*The media type is SBC*/
		0x00, /*Not write proteted*/
		0x00  /*No mode-param block descriptors*/
	}
};
#endif /*SUPPORT_MODE_SENSE_CMD*/

/*Sense data. Initialised by function ResetSenseData*/
static volatile  SCSI_SENSE_DATA g_SCSI_SENSE_DATA;

/*Record an expected data out*/
static volatile  DATA_OUT_EXPECTED g_DataOutExpected = {0,0};


/*Buffer to hold sense data when ready to sent to host*/
static uint8_t g_SCSI_Sense_Buffer[SCSI_SENSE_DATA_SIZE];

/***********************************************************************************
Private Function Prototypes
***********************************************************************************/
static void ResetSenseData(void);
static void SetSenseBuffer(void);

/**********************************************************************
* Outline 		: SCSI_Init 
* Description 	: Initialise this module.
*				  Clears sense data.
* Argument  	: none
* Return value  : Error code.
**********************************************************************/

SCSI_ERROR SCSI_Init(void)
{
	uint32_t Value;
	uint8_t* pArray;
	
	SCSI_ERROR err = SCSI_ERROR_OK;
	
	/*Reset sense data*/
	ResetSenseData();
	
	/*Re-set expected data count*/
	g_DataOutExpected.m_NumBytes = 0;
	
	/*Set READ_CAPACITY response (Big Endian format required)*/
	/*Num Blocks */
#if defined(DEBUG_USE_RAMDISK)
    Value = (uint32_t)RamDiskGetNumBlocks()-1;
#else
	Value = (uint32_t)storage_get_block_count()-1;
#endif
	pArray = (uint8_t*)&g_SCSI_READ_CAPACITY.LastLBA;
	pArray[0] = (uint8_t)((Value >> 24) & 0xFF);
	pArray[1] = (uint8_t)((Value >> 16) & 0xFF); 
	pArray[2] = (uint8_t)((Value >> 8) & 0xFF); 
	pArray[3] = (uint8_t)((Value >> 0) & 0xFF); 
	
	/*Block Size */
#if defined(DEBUG_USE_RAMDISK)
    Value = RamDiskGetBlockSize();
#else
	Value = storage_get_block_size();
#endif
#if defined(DEBUG_USB_MSC)
	debug_printf("SCSIBLKSZ:%x\r\n", Value);
#endif
	pArray = (uint8_t*)&g_SCSI_READ_CAPACITY.BlockLength;
	pArray[0] = (uint8_t)((Value >> 24) & 0xFF);
	pArray[1] = (uint8_t)((Value >> 16) & 0xFF); 
	pArray[2] = (uint8_t)((Value >> 8) & 0xFF); 
	pArray[3] = (uint8_t)((Value >> 0) & 0xFF); 
	
	return err;
}
/***********************************************************************************
End of function SCSI_Init
***********************************************************************************/

/**********************************************************************
* Outline 		: SCSI_ProcessCmd 
* Description 	: Processes a SCSI command.
*				  If the command has a data stage IN this will supply
*				  the data.
*				  If the command has a data stage OUT this will supply
*				  a data buffer.
*				
* Argument  	: _pCDB: SCSI Command
*				  _pPhaseNext: (OUT) Set to the phase that shoul be done next.
*				  _pNumBytes: (OUT) Set to size of data stage. 
*				  _ppBuffer: (OUT) If there is a data stage then set to buffer.
*							  NOTE: If doing a data out can set this to NULL
*							  in which case the data will be read from the host
*							  but not stored anywhere.
*				
* Return value  : Error Code
**********************************************************************/

SCSI_ERROR SCSI_ProcessCmd(uint8_t(*_pCDB)[SCSI_CDB_SIZE], SCSI_PHASE* _pPhaseNext,
						 	uint32_t* _pNumBytes, uint8_t** _ppBuffer)
{
	SCSI_ERROR err = SCSI_ERROR_OK;
	uint8_t cmdID = (*_pCDB)[0];
	switch(cmdID)
	{
		case SCSI_ID_INQUIRY:
		{
			/*INQUIRY*/
			uint8_t AllocationLength;
			uint8_t Length = SCSI_INQUIRY_RESPONSE_SIZE;
			
			DEBUG_MSG_LOW(("SCSI: INQUIRY cmd received.\r\n"));
		
			/*The size of the data response required is stored within the command*/
			AllocationLength = (uint8_t)((*_pCDB)[4]);
			
			/*Normally AllocationLength will equall SCSI_INQUIRY_RESPONSE_SIZE but
			if it is less than SCSI_INQUIRY_RESPONSE_SIZE then set length to that.*/
			if(Length > AllocationLength)
			{
				Length = AllocationLength;
			}
		
			/*This has a data stage IN*/
			*_pPhaseNext = SCSI_PHASE_DATA_IN;
			/*Set data*/
			*_pNumBytes = Length;
			*_ppBuffer = (uint8_t*)&g_SCSI_INQUIRY_RESPONSE;
	
			break;
		}
		case SCSI_ID_REQUEST_SENSE:
		{
			uint8_t AllocationLength;
			uint8_t Length = SCSI_SENSE_DATA_SIZE;
			
			/*REQUEST_SENSE*/
			DEBUG_MSG_LOW( ("SCSI: REQUEST_SENSE cmd received.\r\n"));
			/*Host has requested the SCSI sense data*/
			
			/*The size of the data response required is stored within the command*/
			AllocationLength = (uint8_t)((*_pCDB)[4]);
			
			/*Normally AllocationLength will equall SCSI_SENSE_DATA_SIZE but
			if it is less than SCSI_SENSE_DATA_SIZE then set length to that.*/
			if(Length > AllocationLength)
			{
				Length = AllocationLength;
			}
			
			/*This has a data stage IN*/
			*_pPhaseNext = SCSI_PHASE_DATA_IN;
			/*Set data*/
			*_pNumBytes = Length;
			
			/*Prepare sense data in a buffer*/
			SetSenseBuffer();
			*_ppBuffer = g_SCSI_Sense_Buffer;
			
			break;
		}
		case SCSI_ID_READ_CAPACITY10:
		{
			/*READ CAPACITY(10)*/
			DEBUG_MSG_LOW(("SCSI: READ_CAPACITY10 cmd received.\r\n"));
			
			/*This has a data stage IN*/
			*_pPhaseNext = SCSI_PHASE_DATA_IN;
			/*Set data*/
			*_pNumBytes = SCSI_READ_CAPACITY_SIZE;
			*_ppBuffer = (uint8_t*)&g_SCSI_READ_CAPACITY;
						
			break;
		}
		case SCSI_ID_READ10:
		{
			/*READ(10)*/
			uint32_t LogicalBlockAddress = 0;
			uint16_t TransferLength = 0;
			uint32_t TransferBytes = 0;
			uint8_t LUN = (uint8_t)((*_pCDB)[1] >> 5);
			assert(LUN == 0); 
				
			DEBUG_MSG_LOW(("SCSI: SCSI_ID_READ10 cmd received.\r\n"));
			
			/*This is the block to start reading from */
			LogicalBlockAddress |= ((uint32_t)(*_pCDB)[2] << 24);
			LogicalBlockAddress |= ((uint32_t)(*_pCDB)[3] << 16);
			LogicalBlockAddress |= ((uint32_t)(*_pCDB)[4] << 8);
			LogicalBlockAddress |= ((uint32_t)(*_pCDB)[5] << 0);
			
			/*This is number of blocks to transfer*/
			TransferLength |= ((uint16_t)(*_pCDB)[7] << 8);
			TransferLength |= ((uint16_t)(*_pCDB)[8] << 0);
#if defined(DEBUG_USE_RAMDISK)
            TransferBytes = (uint32_t)TransferLength * (uint32_t)RamDiskGetBlockSize();
#else
			TransferBytes = (uint32_t)TransferLength * (uint32_t)storage_get_block_size();
#endif
			/*Check parameters are OK*/
			/*  1. LUN == 0 (Only suppporting a single Logical unit)
				2. Number of blocks + block to start is within RAM disk.*/
			if( (LUN != 0) ||
#if defined(DEBUG_USE_RAMDISK)
			( (LogicalBlockAddress + TransferLength) > RamDiskGetNumBlocks()) )
#else
			( (LogicalBlockAddress + TransferLength) > storage_get_block_count()) || (TransferBytes > SCSI_BUF_SIZE) )
#endif	
			{
#if defined(DEBUG_USB_MSC)
                debug_printf("SCSIRD Invalid buf:%x blk:%x bytes:%x\r\n", *_ppBuffer, LogicalBlockAddress, TransferBytes);
#endif
                /*Invalid request*/
				DEBUG_MSG_LOW(("READ(10) - Invalid parameter!"));
				
				/*Set SCSI Sense data*/
				g_SCSI_SENSE_DATA.SenseKey = 0x05; 	/*ILLEGAL REQUEST*/
				/*INVALID FIELD IN COMMAND PACKET*/
				g_SCSI_SENSE_DATA.ASC = 0x24;
			
				/*Return a failed status which will probably cause
				 the host to request the sense data (SCSI_ID_REQUEST_SENSE)*/
				*_pPhaseNext = SCSI_PHASE_STATUS;
				err = SCSI_ERROR_FAILED;
			}
			else
			{
				DEBUG_MSG_LOW(("READ(10) Length = %lu, LBA = %lu\r\n",
						 			TransferBytes, LogicalBlockAddress));
			
				/*This has a data stage IN*/
				*_pPhaseNext = SCSI_PHASE_DATA_IN;
				/*Set data*/
				*_pNumBytes = TransferBytes;
#if defined(DEBUG_USE_RAMDISK)
                *_ppBuffer = RamDiskGetBuffer(LogicalBlockAddress);
#else
                *_ppBuffer = scsi_buf;
                storage_read_blocks(*_ppBuffer, LogicalBlockAddress, TransferLength);
#endif
#if defined(DEBUG_USB_MSC)
                debug_printf("SCSIRD buf:%x blk:%x bytes:%x\r\n", *_ppBuffer, LogicalBlockAddress, TransferBytes);
#endif
			}
			break;
		}
		case SCSI_ID_TEST_UNIT_READY:
		{
			/*TEST_UNIT_READY*/
			DEBUG_MSG_MID(("SCSI: TEST_UNIT_READY cmd received.\r\n"));
			
			/*No data transport*/
			/*If not ready return an error so a command failed CSW is sent*/
			
			/*We are ready so just send a normal passed CSW*/
			*_pPhaseNext = SCSI_PHASE_STATUS;
			*_pNumBytes = 0;
			*_ppBuffer = NULL;
			break;
		}
		case SCSI_ID_WRITE10:
		{
			/*WRITE(10)*/
			uint32_t LogicalBlockAddress = 0;
			uint16_t TransferLength = 0;
			uint32_t TransferBytes = 0;
			uint8_t LUN = (uint8_t)((*_pCDB)[1] >> 5);
			assert(LUN == 0); /*Only suppporting a single Logical unit*/
			
			DEBUG_MSG_LOW(("SCSI: SCSI_ID_WRITE10 cmd received.\r\n"));
			
			/*This is the block to start reading from */
			LogicalBlockAddress |= ((uint32_t)(*_pCDB)[2] << 24);
			LogicalBlockAddress |= ((uint32_t)(*_pCDB)[3] << 16);
			LogicalBlockAddress |= ((uint32_t)(*_pCDB)[4] << 8);
			LogicalBlockAddress |= ((uint32_t)(*_pCDB)[5] << 0);
			
			/*This is number of blocks to transfer*/
			TransferLength |= ((uint16_t)(*_pCDB)[7] << 8);
			TransferLength |= ((uint16_t)(*_pCDB)[8] << 0);
#if defined(DEBUG_USE_RAMDISK)
			TransferBytes = (uint32_t)TransferLength * (uint32_t)RamDiskGetBlockSize();
#else
			TransferBytes = (uint32_t)TransferLength * (uint32_t)storage_get_block_size();
#endif
			/*Check parameters are OK*/
			/*  1. LUN == 0 (Only suppporting a single Logical unit)
				2. Number of blocks + block to start is within RAM disk.*/
			if( (LUN != 0) ||
#if defined(DEBUG_USE_RAMDISK)
			( (LogicalBlockAddress + TransferLength) > RamDiskGetNumBlocks()) )
#else
			( (LogicalBlockAddress + TransferLength) > storage_get_block_count()) || (TransferBytes > SCSI_BUF_SIZE) )
#endif
			{
#if defined(DEBUG_USB_MSC)
                debug_printf("SCSIWT Invalid buf:%x blk:%x bytes:%x\r\n", *_ppBuffer, LogicalBlockAddress, TransferBytes);
#endif
				/*Invalid request*/
				DEBUG_MSG_LOW(("WRITE(10) - Invalid parameter!"));
				
				/*Set SCSI Sense data*/
				g_SCSI_SENSE_DATA.SenseKey = 0x05; 	/*ILLEGAL REQUEST*/
				/*INVALID FIELD IN COMMAND PACKET*/
				g_SCSI_SENSE_DATA.ASC = 0x24;
			
				/*Return a failed status which will probably cause
				 the host to request the sense data (SCSI_ID_REQUEST_SENSE)*/
				*_pPhaseNext = SCSI_PHASE_STATUS;
				err = SCSI_ERROR_FAILED;
			}
			else
			{
				DEBUG_MSG_LOW(("Write(10) Length = %lu, LBA = %lu\r\n",
				 			TransferBytes, LogicalBlockAddress));
						
				/*This has a data stage OUT*/
				*_pPhaseNext = SCSI_PHASE_DATA_OUT;
				/*Set data*/
				*_pNumBytes = TransferBytes;
#if defined(DEBUG_USE_RAMDISK)
                *_ppBuffer = RamDiskGetBuffer(LogicalBlockAddress);
#else
                *_ppBuffer = scsi_buf;
                scsi_blk = LogicalBlockAddress;
#endif
#if defined(DEBUG_USB_MSC)
    debug_printf("SCSIWR buf:%x blk:%x bytes:%x\r\n", *_ppBuffer, LogicalBlockAddress, TransferBytes);
#endif
			
				/*Set the DATA_OUT_EXPECTED structure so we know what to do
				when we get the data in function SCSI_Data_OUT*/
				g_DataOutExpected.m_NumBytes = *_pNumBytes;
				g_DataOutExpected.m_CmdID = SCSI_ID_WRITE10;
			}
			break;
		}
		case SCSI_ID_VERIFY10:
		{
			uint32_t LogicalBlockAddress = 0;
			uint16_t VerifyLength = 0;
			uint8_t LUN = (uint8_t)(0x07 & ((*_pCDB)[1] >> 5));
			uint8_t ByteChk = (uint8_t)(0x01 & ((*_pCDB)[1] >> 1));
			assert(LUN == 0); /*Only suppporting a single Logical unit*/
			
			/*VERIFY(10)*/
			DEBUG_MSG_LOW(("SCSI: SCSI_ID_VERIFY10 cmd received.\r\n"));
			
			/*We only support this if ByteChk = 0 (NO Data sent from host)
			and LUN = 0*/
			if((0 == ByteChk) && (LUN == 0))
			{
				/*This is the block to start verify from */
				LogicalBlockAddress |= ((uint32_t)(*_pCDB)[2] << 24);
				LogicalBlockAddress |= ((uint32_t)(*_pCDB)[3] << 16);
				LogicalBlockAddress |= ((uint32_t)(*_pCDB)[4] << 8);
				LogicalBlockAddress |= ((uint32_t)(*_pCDB)[5] << 0);
			
				/*This is number of blocks to verify*/
				VerifyLength |= ((uint16_t)(*_pCDB)[7] << 8);
				VerifyLength |= ((uint16_t)(*_pCDB)[8] << 0);
			
				/*Don't actually need to check the area as we know it is OK*/
			
				/*No data transport - straight to status*/
				*_pPhaseNext = SCSI_PHASE_STATUS;
				*_pNumBytes = 0;
				*_ppBuffer = NULL;
			}
			else
			{
				/*Don't support */
				DEBUG_MSG_LOW(("SCSI: Not Supported (ByteChk = 1)\r\n"));
				
				/*Set SCSI Sense data*/
				g_SCSI_SENSE_DATA.SenseKey = 0x05; 	/*ILLEGAL REQUEST*/
				/*INVALID FIELD IN COMMAND PACKET*/
				g_SCSI_SENSE_DATA.ASC = 0x24;
			
				/*Return a failed status which will probably cause
				 the host to request the sense data (SCSI_ID_REQUEST_SENSE)*/
				*_pPhaseNext = SCSI_PHASE_STATUS;
				err = SCSI_ERROR_FAILED;
			}
					
			break;
		}
		/*This cmd is optional*/
		#ifdef SUPPORT_SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL_CMD
		case SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL:
		{
			/*PREVENT_ALLOW_MEDIUM_REMOVAL*/
			DEBUG_MSG_LOW(("SCSI: PREVENT_ALLOW_MEDIUM_REMOVAL cmd received.\r\n"));
			
			/*Is this a Prevent or Allow request?*/
			if(0 == ((*_pCDB)[4] & 0x01))
			{
				/*'allow' removal - then fine*/
				/*No data transport - straight to status*/
				*_pPhaseNext = SCSI_PHASE_STATUS;
				*_pNumBytes = 0;
				*_ppBuffer = NULL;
			}
			else
			{
				DEBUG_MSG_LOW(("SCSI: PREVENT is not possible.\r\n"));
				
				/* If 'prevent' removal then return failure and
				set sense key with ILLEGAL REQUEST and an
				additional sense code set to  */
				
				/*Set SCSI Sense data*/
				g_SCSI_SENSE_DATA.SenseKey = 0x05; 	/*ILLEGAL REQUEST*/
				/*INVALID FIELD IN COMMAND PACKET*/
				g_SCSI_SENSE_DATA.ASC = 0x24;
			
				/*Return a failed status which will probably cause
				 the host to request the sense data (SCSI_ID_REQUEST_SENSE)*/
				*_pPhaseNext = SCSI_PHASE_STATUS;
				err = SCSI_ERROR_FAILED;
				 
			}
			break;
		}
		#endif /*SUPPORT_SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL_CMD*/
		
		/*This cmd is optional*/
		#ifdef SUPPORT_MODE_SENSE_CMD
		case SCSI_ID_MODE_SENSE6:
		{
			/*NOTE As MODE SELECT is not supported then strictly
			this shouldn't be either.
			However, see book page 144 - can just return header.
			*/
			/*MODE_SENSE6*/
			DEBUG_MSG_LOW(("SCSI: MODE_SENSE6 cmd received.\r\n"));
			
			/*This has a data stage IN*/
			*_pPhaseNext = SCSI_PHASE_DATA_IN;
			/*Set data*/
			*_pNumBytes = SCSI_MODE_SENSE_SIZE;
			*_ppBuffer = (uint8_t*)&g_SCSI_MODE_SENSE;	
			break;
		}
		#endif /*SUPPORT_MODE_SENSE_CMD*/
		default:
		{
			/*Command not supported/recognised*/		
			DEBUG_MSG_LOW(("*** CMD Not supported ***\r\n"));
			
			#ifndef SUPPORT_SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL_CMD
			if(SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL == cmdID)
			{
				DEBUG_MSG_LOW(("SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL\r\n"));
			}
			#endif
			#ifndef SUPPORT_MODE_SENSE_CMD
			if(SCSI_ID_MODE_SENSE6 == cmdID)
			{
				DEBUG_MSG_LOW(("SCSI_ID_MODE_SENSE6\r\n"));
			}
			#endif
			if(SCSI_READ_FORMAT_CAPACITIES == cmdID)
			{
				DEBUG_MSG_LOW(("SCSI_READ_FORMAT_CAPACITIES\r\n"));
			}
			
			/*Set SCSI Sense data (BOOK page 135)*/
			g_SCSI_SENSE_DATA.SenseKey = 0x05; 	/*ILLEGAL REQUEST*/
			g_SCSI_SENSE_DATA.ASC = 0x20; 		/*INVALID COMMAND*/
			
			/*Return a failed status which will probably cause
			 the host to request the sense data (SCSI_ID_REQUEST_SENSE)*/
			*_pPhaseNext = SCSI_PHASE_STATUS;
			err = SCSI_ERROR_FAILED;	
		}
	}
	
	/*Clear the sense data*/
	if(SCSI_ERROR_OK == err)
	{
		/*Unless this is the sense data being requested then
		reset the sense data.*/
		if(cmdID != SCSI_ID_REQUEST_SENSE)
		{
			/*Only need to reset id it has an error*/
			/*Assumption that any error would always set the SenseKey*/
			if(0 != g_SCSI_SENSE_DATA.SenseKey)
			{
				ResetSenseData();
			}
		}
	}
	
	return err;
}
/**********************************************************************************
End of function SCSI_ProcessCmd
***********************************************************************************/   

/**********************************************************************
* Outline 		: SCSI_Data_OUT
* Description 	: Used to pass data to the SCSI module after
*				  SCSI_ProcessCmd function has supplied a buffer
*				  and set phase to SCSI_PHASE_DATA_OUT.
* Argument  	: none
* Return value  : none
**********************************************************************/

SCSI_ERROR SCSI_Data_OUT(uint32_t _NumBytes, uint8_t* _pBuffer)
{
	SCSI_ERROR err = SCSI_ERROR_OK;
	
	/*Was data expected*/
	if(0 != g_DataOutExpected.m_NumBytes)
	{
		assert(g_DataOutExpected.m_NumBytes == _NumBytes);
		
		if(SCSI_ID_WRITE10 == g_DataOutExpected.m_CmdID)
		{
			/*The buffer suppled by SCSI_ProcessCmd when
			this cmd was first received is infact the RAMDisk.
			So no need to do anything extra with the data as it has already
			been written to the RAMDisk.*/	
#if defined(DEBUG_USE_RAMDISK)
#if defined(DEBUG_USB_MSC)
		    debug_printf("SCSIOUT buf:%x bytes:%x\r\n", _pBuffer, _NumBytes);
#endif
#else
#if defined(DEBUG_USB_MSC)
            debug_printf("SCSIOUT buf:%x blk:%x bytes:%x\r\n", _pBuffer, scsi_blk, _NumBytes);
#endif
            storage_write_blocks(_pBuffer, scsi_blk, _NumBytes / storage_get_block_size());
#endif
		}
		else
		{
			/*This CMD doesn't have a data out*/
			assert(0); /*g_DataOutExpected should never have been set with it!*/
			err = SCSI_ERROR_FAILED;	
		}
	}
	else
	{
		/*Don't know what to do with this data - was not expecting it.*/
		err = SCSI_ERROR_FAILED;
	}
	
	return err;
}
/**********************************************************************************
End of function SCSI_Data_OUT
***********************************************************************************/   

/**********************************************************************
* Outline 		: ResetSenseData
* Description 	: Clear the sense data to default.
* Argument  	: none
* Return value  : none
**********************************************************************/

void ResetSenseData(void)
{
	memset((void*)&g_SCSI_SENSE_DATA, 0 , sizeof(SCSI_SENSE_DATA));
	/*Current Error*/
	g_SCSI_SENSE_DATA.ResponseCode = 0x70;
	/*No Extra Vendor Specific Sense Data */
	g_SCSI_SENSE_DATA.AddSenseLen = 0x0A;
}
/**********************************************************************************
End of function ResetSenseData
***********************************************************************************/   

/**********************************************************************
* Outline 		: SetSenseBuffer 
* Description 	: Packages the sense structure up into a
*				  buffer and sends to host.
*				  Note: Buffer will be in big endian.
*				
*				  This is done in a long winded way to avoid endian issues
*				  and problems with padding in the CSW structure.
* Argument  	: none
* Return value  : none
**********************************************************************/

static void SetSenseBuffer(void)
{		
	g_SCSI_Sense_Buffer[0] = (uint8_t)((((uint8_t)g_SCSI_SENSE_DATA.Valid) << 7) & 0x80);
	g_SCSI_Sense_Buffer[0] |= (uint8_t)(((uint8_t)g_SCSI_SENSE_DATA.ResponseCode) & 0xEF);
	
	g_SCSI_Sense_Buffer[1] = g_SCSI_SENSE_DATA.Obsolete;
	
	g_SCSI_Sense_Buffer[2] = (uint8_t)((((uint8_t)g_SCSI_SENSE_DATA.Filemark) << 7) & 0x80);
	g_SCSI_Sense_Buffer[2] |= (uint8_t)((((uint8_t)g_SCSI_SENSE_DATA.EOM) << 6) & 0x40); 
	g_SCSI_Sense_Buffer[2] |= (uint8_t)((((uint8_t)g_SCSI_SENSE_DATA.ILI) << 5) & 0x20);
	g_SCSI_Sense_Buffer[2] |= (uint8_t)((((uint8_t)g_SCSI_SENSE_DATA.Reserved1) << 4) & 0x10);
	g_SCSI_Sense_Buffer[2] |= (uint8_t)((((uint8_t)g_SCSI_SENSE_DATA.SenseKey) << 0) & 0x0F);
	
	g_SCSI_Sense_Buffer[3] = (uint8_t)((g_SCSI_SENSE_DATA.Information >> 24) & 0xFF);
	g_SCSI_Sense_Buffer[4] = (uint8_t)((g_SCSI_SENSE_DATA.Information >> 16) & 0xFF);
	g_SCSI_Sense_Buffer[5] = (uint8_t)((g_SCSI_SENSE_DATA.Information >> 8) & 0xFF);
	g_SCSI_Sense_Buffer[6] = (uint8_t)((g_SCSI_SENSE_DATA.Information >> 0) & 0xFF);
	
	g_SCSI_Sense_Buffer[7] =  g_SCSI_SENSE_DATA.AddSenseLen;
	
	g_SCSI_Sense_Buffer[8] = (uint8_t)((g_SCSI_SENSE_DATA.CmdSpecificInfo >> 24) & 0xFF);
	g_SCSI_Sense_Buffer[9] = (uint8_t)((g_SCSI_SENSE_DATA.CmdSpecificInfo >> 16) & 0xFF);
	g_SCSI_Sense_Buffer[10] = (uint8_t)((g_SCSI_SENSE_DATA.CmdSpecificInfo >> 8) & 0xFF); 
	g_SCSI_Sense_Buffer[11] = (uint8_t)((g_SCSI_SENSE_DATA.CmdSpecificInfo >> 0) & 0xFF);
	
	g_SCSI_Sense_Buffer[12] =  g_SCSI_SENSE_DATA.ASC;
	g_SCSI_Sense_Buffer[13] =  g_SCSI_SENSE_DATA.ASCQ;
	g_SCSI_Sense_Buffer[14] =  g_SCSI_SENSE_DATA.FRUC;
	
	g_SCSI_Sense_Buffer[15] =  g_SCSI_SENSE_DATA.SenseKeySpecific[0];
	g_SCSI_Sense_Buffer[16] =  g_SCSI_SENSE_DATA.SenseKeySpecific[1];
	g_SCSI_Sense_Buffer[17] =  g_SCSI_SENSE_DATA.SenseKeySpecific[2]; 
}
/**********************************************************************************
End of function SetSenseBuffer
***********************************************************************************/   
