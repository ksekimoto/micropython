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
/* Copyright (C) 2012 Renesas Electronics Corporation. All rights reserved.   */
/*******************************************************************************
* File Name     : usbdescriptors.c
* Version       : 1.00
* Device        : R5F563NB
* Tool-Chain    : Renesas RX Standard 1.2.0.0
* H/W Platform  : RSK+RX63N
* Description   : Descriptors required to enumerate a device as a
*                  Communication Device Class - Abstract Class Model(CDC ACM).
*
*                 NOTE: This will need to be modified for a particular
*                  product as it includes company/product specific data including
*                  string descriptors specifying
*                  Manufacturer, Product and Serial Number.
***********************************************************************************/
/***********************************************************************************
* History       : 13 Aug. 2012  Ver. 1.00 First Release
***********************************************************************************/
/*
 *  Modified 12 May 2014 by Yuuki Okamiya, modify descriptor for GR-SAKURA lib2.xx
 */

/***********************************************************************************
User Includes
***********************************************************************************/
#include "usb_hal.h"
#include "usb_core.h"
#include "usbdescriptors.h"
#include <string.h>

#if defined(USB_HID)
//#define USB_HID_SPECIAL
#define USB_HID_MOUSE
//#define USB_HID_KEYBOARD

#define HID_SPECIAL_REPORT_DESCRIPTOR_SIZE  34
#define HID_SPECIAL_ENDPOINT_MAX_SIZE       64

#define HID_MOUSE_REPORT_DESCRIPTOR_SIZE    52
//#define HID_MOUSE_REPORT_DESCRIPTOR_SIZE    74
#define HID_MOUSE_ENDPOINT_MAX_SIZE         8

#define HID_KEYBOARD_REPORT_DESCRIPTOR_SIZE 63
#define HID_KEYBOARD_ENDPOINT_MAX_SIZE      8

/* As specified in the Report Descriptor */
#define HID_SPECIAL_OUTPUT_REPORT_SIZE  17
/* As specified in the Report Descriptor */
#define HID_SPECIAL_INPUT_REPORT_SIZE   5
#endif

/***********************************************************************************
Defines
***********************************************************************************/
/*Vendor and Product ID*/
/*NOTE Please use your company Vendor ID when developing a new product.*/
/*Default PID/VID comes from Gadget Renesas compiler configuration.*/
/*Other VIDs should be properly allocated.*/
/*The following VIDs are not formally allocated.*/
#if defined(GRSAKURA)
#define VID 0x045B
//#define PID 0x0234
#if defined(USB_HID_SPECIAL)
#define PID 0x0236
#endif
#if defined(USB_HID_MOUSE)
#define PID 0x0234
#endif
#if defined(USB_HID_KEYBOARD)
#define PID 0x0235
#endif
#elif defined(GRCITRUS)
#define VID 0x2A50
//#define PID 0x0277
#if defined(USB_HID_SPECIAL)
#define PID 0x0278
#endif
#if defined(USB_HID_MOUSE)
#define PID 0x0277
#endif
#if defined(USB_HID_KEYBOARD)
#define PID 0x0279
#endif
#elif defined(GRROSE)
#define VID 0x045B
//#define PID 0x025A
#if defined(USB_HID_SPECIAL)
#define PID 0x025B
#endif
#if defined(USB_HID_MOUSE)
#define PID 0x025A
#endif
#if defined(USB_HID_KEYBOARD)
#define PID 0x025C
#endif
#endif
/***********************************************************************************
Variables
***********************************************************************************/
/*    Device Descriptor    */
#define DEVICE_DESCRIPTOR_SIZE    18
static const uint8_t gDeviceDescriptorData[DEVICE_DESCRIPTOR_SIZE] =
{
    /*Size of this descriptor*/
    DEVICE_DESCRIPTOR_SIZE,
    /*Device Descriptor*/
    0x01,
    /*USB Version 2.0*/
    0x00,0x02,
#if defined(USB_CDC_MSC)
    /*Class Code - Misc*/
    0xef,
    /*Subclass Code*/
    0x02,
    /*Protocol Code*/
    0x01,
#elif defined(USB_CDC)
    2,                              /*  4:bFunctionClass */
    2,                              /*  5:bFunctionSubClass */
    1,                              /*  6:bFunctionProtocol */
#elif defined(USB_MSC)
    8,                              /*  4:bFunctionClass */
    6,                              /*  5:bFunctionSubClass */
    1,                              /*  6:bFunctionProtocol */
#endif
    /*Max Packet Size for endpoint 0*/
    CONTROL_IN_PACKET_SIZE,
    (uint8_t)(VID & 0xFF),    /*Vendor ID LSB*/
    (uint8_t)((VID>>8)& 0xFF),/*Vendor ID MSB*/
    (uint8_t)(PID & 0xFF),    /*Product ID LSB*/
    (uint8_t)((PID>>8)& 0xFF),/*Product ID MSB*/
    /*Device Release Number*/
    0x00,0x01,
    /*Manufacturer String Descriptor*/
    STRING_iMANUFACTURER,
    /*Product String Descriptor*/
    STRING_iPRODUCT,
    /*Serial Number String Descriptor*/
    STRING_iSERIAL,
    /*Number of Configurations supported*/
    0x01
};

const DESCRIPTOR gDeviceDescriptor =
{
    DEVICE_DESCRIPTOR_SIZE, (uint8_t *)gDeviceDescriptorData
};

/*Configuration Descriptor*/
#if defined(USB_CDC_MSC)
#if defined(USB_HID)
// CDC_MSC_HID
#define CONFIG_DESCRIPTOR_SIZE (9 + 23 + 8 + 35 + 23 + HID_INTERFACE_DESCRIPTOR_SIZE)
// Configuration Descriptor = 9
// Interface 0 (MSC) = 23
// IAP = 8
// Interface 1 (Com) = 35
// Interface 2 (Data) = 23
// Interface 3 (HID) = 25
#define HID_INTERFACE_DESCRIPTOR_OFS    (9 + 23 + 8 + 35 + 23)
#else
// CDC_MSC
#define CONFIG_DESCRIPTOR_SIZE (9 + 23 + 8 + 35 + 23)
#endif
#elif defined(USB_CDC)
#define CONFIG_DESCRIPTOR_SIZE (9 + 23 + 35)
#elif defined(USB_MSC)
#define CONFIG_DESCRIPTOR_SIZE (9 + 23)
#endif
static uint8_t gConfigurationDescriptorData[CONFIG_DESCRIPTOR_SIZE] =
{
    /*Size of this descriptor (Just the configuration part)*/
    0x09,
    /*Configuration Descriptor*/
    0x02,
    /*Combined length of all descriptors (little endian)*/
    CONFIG_DESCRIPTOR_SIZE, 0x00,
    /*Number of interfaces*/
#if defined(USB_CDC_MSC)
#if defined(USB_HID)
    0x04,   // CDC_MSC_HID
#else
    0x03,   // CDC_MSC
#endif
#elif defined(USB_CDC)
    0x02,
#elif defined(USB_MSC)
    0x01,
#endif
    /*This Interface Value*/
    0x01,
    /*No String Descriptor for this configuration*/
    0x00,
    /*bmAttributes - Self Powered(USB bus powered), No Remote Wakeup*/
    /* 0xC0, */
    /*bmAttributes - Not USB Bus powered, No Remote Wakeup*/
    0x80,
    /*bMaxPower (2mA units) 100mA (A unit load is defined as 100mA)*/
    50,

#if defined(USB_MSC) | defined(USB_CDC_MSC)
    /* Interface Descriptor 0 */
    /*Size of this descriptor*/
    0x09,
    /*INTERFACE Descriptor*/
    0x04,
    /*Index of Interface*/
#if defined(USB_CDC_MSC)
    0x00,
#else
    0x00,
#endif
    /*bAlternateSetting*/
    0x00,
    /*Number of Endpoints - BULK IN and BULK OUT*/
    0x02,
    /*Class code = Mass Storage*/
    0x08,
    /*Subclass = SCSI*/
    0x06,
    /*Bulk only Protocol*/
    0x50,
    /*No String Descriptor for this interface*/
    0x00,

    /* Endpoint Descriptor 1 */
    /* Endpoint Bulk OUT */
    /*Size of this descriptor*/
    0x07,
    /*ENDPOINT Descriptor*/
    0x05,
    /*bEndpointAddress - OUT endpoint, endpoint number = 1*/
    0x04,
    /*Endpoint Type is BULK*/
    0x02,
    /*Max Packet Size*/
    BULK_OUT_PACKET_SIZE, 0x00,
    /*Polling Interval in mS - IGNORED FOR BULK (except high speed out)*/
    0x00,

    /* Endpoint Descriptor 2 */
    /* Endpoint Bulk IN */
    /*Size of this descriptor*/
    0x07,
    /*ENDPOINT Descriptor*/
    0x05,
    /*bEndpointAddress - IN endpoint, endpoint number = 2*/
    0x85,
    /*Endpoint Type is BULK*/
    0x02,
    /*Max Packet Size*/
    BULK_IN_PACKET_SIZE, 0x00,
    /*Polling Interval in mS - IGNORED FOR BULK*/
    0x00,
#endif

#if defined(USB_CDC_MSC)
    /* IAD */
        8,                              /*  0:bLength */
        0x0B,                           /*  1:bDescriptorType*/
        1,                              /*  2:bFirstInterface */
        2,                              /*  3:bInterfaceCount */
        2,                              /*  4:bFunctionClass */
        2,                              /*  5:bFunctionSubClass */
        1,                              /*  6:bFunctionProtocol */
        0,                              /*  7:iInterface */
#endif

#if defined(USB_CDC) | defined(USB_CDC_MSC)
    /* Interface Descriptor 1 */
    /* Communication Class Interface Descriptor */
    /*Size of this descriptor*/
    0x09,   // CDC_MSC
    /*INTERFACE Descriptor*/
    0x04,   // CDC_MSC
    /*Index of Interface*/
#if defined(USB_CDC_MSC)
    0x01,
#else
    0x00,
#endif
    /*bAlternateSetting*/
    0x00,
    /*Number of Endpoints*/
    0x01,
    /*Class code = Communication*/
    0x02,
    /*Subclass = Abstract Control Model*/
    0x02,
    /*No Protocol*/
    0x00,
    /*No String Descriptor for this interface*/
    0x00,

/*Header Functional Descriptor*/
    /*bFunctionalLength*/
    0x05,
    /*bDescriptorType = CS_INTERFACE*/
    0x24,
    /*bDescriptor Subtype = Header*/
    0x00,
    /*bcdCDC 1.1*/
    0x10,0x01,

/* ACM Functional Descriptor */
    /*bFunctionalLength*/
    0x04,
    /*bDescriptorType = CS_INTERFACE*/
    0x24,
    /*bDescriptor Subtype = Abstract Control Management*/
    0x02,
    /*bmCapabilities GET_LINE_CODING etc supported*/
    0x02,

/* Union Functional Descriptor */
    /*bFunctionalLength*/
    0x05,
    /*bDescriptorType = CS_INTERFACE*/
    0x24,
    /*bDescriptor Subtype = Union*/
    0x06,
    /*bMasterInterface = Communication Class Interface*/
#if defined(USB_CDC_MSC)
    0x01,
#else
    0x00,
#endif
    /*bSlaveInterface = Data Class Interface*/
#if defined(USB_CDC_MSC)
    0x02,
#else
    0x01,
#endif

/* Call Management Functional Descriptor */
    /*bFunctionalLength*/
    0x05,
    /*bDescriptorType = CS_INTERFACE*/
    0x24,
    /*bDescriptor Subtype = Call Management*/
    0x01,
    /*bmCapabilities*/
    0x00,
    /*bDataInterface: Data Class Interface = 1*/
#if defined(USB_CDC_MSC)
    0x02,
#else
    0x01,
#endif

/* Interrupt Endpoint */
    /*Size of this descriptor*/
    0x07,
    /*ENDPOINT Descriptor*/
    0x05,
    /*bEndpointAddress - IN endpoint, endpoint number = 3*/
    0x83,
    /*Endpoint Type is Interrupt*/
    0x03,
    /*Max Packet Size*/
    0x08,0x00,
    /*Polling Interval in mS*/
    0xFF,

/* DATA Class Interface Descriptor */
    /*Size of this descriptor*/
    0x09,
    /*INTERFACE Descriptor*/
    0x04,
    /*Index of Interface*/
#if defined(USB_CDC_MSC)
    0x02,
#else
    0x01,
#endif
    /*bAlternateSetting*/
    0x00,
    /*Number of Endpoints*/
    0x02,
    /*Class code = Data Interface*/
    0x0A,
    /*Subclass = Not Used, set as 0*/
    0x00,
    /*No Protocol*/
    0x00,
    /*No String Descriptor for this interface*/
    0x00,

/*Endpoint Bulk OUT */
    /*Size of this descriptor*/
    0x07,
    /*ENDPOINT Descriptor*/
    0x05,
    /*bEndpointAddress - OUT endpoint, endpoint number = 1*/
    0x01,
    /*Endpoint Type is BULK*/
    0x02,
    /*Max Packet Size*/
    64,0x00,
    /*Polling Interval in mS - IGNORED FOR BULK*/
    0x00,

/* Endpoint Bulk IN */
    /*Size of this descriptor*/
    0x07,
    /*ENDPOINT Descriptor*/
    0x05,
    /*bEndpointAddress - IN endpoint, endpoint number = 2*/
    0x82,
    /*Endpoint Type is BULK*/
    0x02,
    /*Max Packet Size*/
    64,0x00,
    /*Polling Interval in mS - IGNORED FOR BULK*/
    0x00,

#if defined(USB_HID)
#if defined(USB_HID_SPECIAL)
    /*Size of this descriptor*/
    0x09,
    /*INTERFACE Descriptor*/
    0x04,
    /*Index of Interface*/
    0x03,
    /*bAlternateSetting*/
    0x00,
    /*Number of Endpoints (Interrupt IN)*/
    0x01,
    /*HID Class code*/
    0x03,
    /*No Subclass*/
    0x00,
    /*No Protocol*/
    0x00,
    /*No String Descriptor for this interface*/
    0x00,

    /*Size of this descriptor*/
    0x09,
    /*HID Descriptor*/
    0x21,
    /*HID Class Specification Release Number 1.11*/
    0x11,0x01,
    /*No Target Country*/
    0x00,
    /*Number of HID Descriptors*/
    0x01,
    /*Type of HID Descriptor = "Report"*/
    0x22,
    /*Length Of HID Report Descriptor*/
    HID_SPECIAL_REPORT_DESCRIPTOR_SIZE,00,

    /*Size of this descriptor*/
    0x07,
    /*ENDPOINT Descriptor*/
    0x05,
    /*bEndpointAddress - IN endpoint, endpoint number = 3*/
    0x86,
    /*Endpoint Type is Interrupt*/
    0x03,
    /*Max Packet Size*/
    INTERRUPT_IN_PACKET_SIZE,0x00,
    /*Polling Interval in mS*/
    0x0A
#endif

#if defined(USB_HID_MOUSE)
    /*Size of this descriptor*/
    0x09,
    /*INTERFACE Descriptor*/
    0x04,
    /*Index of Interface*/
    0x03,
    /*bAlternateSetting*/
    0x00,
    /*Number of Endpoints (Interrupt IN)*/
    0x01,
    /*HID Class code*/
    0x03,
    /*Boot*/
    0x01,
    /*Mouse*/
    0x02,
    /*No String Descriptor for this interface*/
    0x00,

    /*Size of this descriptor*/
    0x09,
    /*HID Descriptor*/
    0x21,
    /*HID Class Specification Release Number 1.11*/
    0x11,0x01,
    /*No Target Country*/
    0x00,
    /*Number of HID Descriptors*/
    0x01,
    /*Type of HID Descriptor = "Report"*/
    0x22,
    /*Length Of HID Report Descriptor*/
    HID_MOUSE_REPORT_DESCRIPTOR_SIZE,00,

    /*Size of this descriptor*/
    0x07,
    /*ENDPOINT Descriptor*/
    0x05,
    /*bEndpointAddress - IN endpoint, endpoint number = 3*/
    0x86,
    /*Endpoint Type is Interrupt*/
    0x03,
    /*Max Packet Size*/
    HID_MOUSE_ENDPOINT_MAX_SIZE,0x00,
    /*Polling Interval in mS*/
    0x0A
#endif

#if defined(USB_HID_KEYBOARD)
    /*Size of this descriptor*/
    0x09,
    /*INTERFACE Descriptor*/
    0x04,
    /*Index of Interface*/
    0x03,
    /*bAlternateSetting*/
    0x00,
    /*Number of Endpoints (Interrupt IN)*/
    0x01,
    /*HID Class code*/
    0x03,
    /*Boot*/
    0x01,
    /*Keyboard*/
    0x01,
    /*No String Descriptor for this interface*/
    0x00,

    /*Size of this descriptor*/
    0x09,
    /*HID Descriptor*/
    0x21,
    /*HID Class Specification Release Number 1.11*/
    0x11,0x01,
    /*No Target Country*/
    0x00,
    /*Number of HID Descriptors*/
    0x01,
    /*Type of HID Descriptor = "Report"*/
    0x22,
    /*Length Of HID Report Descriptor*/
    HID_KEYBOARD_REPORT_DESCRIPTOR_SIZE,00,

    /*Size of this descriptor*/
    0x07,
    /*ENDPOINT Descriptor*/
    0x05,
    /*bEndpointAddress - IN endpoint, endpoint number = 3*/
    0x86,
    /*Endpoint Type is Interrupt*/
    0x03,
    /*Max Packet Size*/
    HID_KEYBOARD_ENDPOINT_MAX_SIZE,0x00,
    /*Polling Interval in mS*/
    0x0A
#endif
#endif
#endif
};

static const uint8_t HID_SPECIAL_InterfaceDescriptorData[HID_INTERFACE_DESCRIPTOR_SIZE + (HID_INTERFACE_DESCRIPTOR_SIZE % 2)] = {
    /*Size of this descriptor*/
    0x09,
    /*INTERFACE Descriptor*/
    0x04,
    /*Index of Interface*/
    0x03,
    /*bAlternateSetting*/
    0x00,
    /*Number of Endpoints (Interrupt IN)*/
    0x01,
    /*HID Class code*/
    0x03,
    /*No Subclass*/
    0x00,
    /*No Protocol*/
    0x00,
    /*No String Descriptor for this interface*/
    0x00,

    /*Size of this descriptor*/
    0x09,
    /*HID Descriptor*/
    0x21,
    /*HID Class Specification Release Number 1.11*/
    0x11,0x01,
    /*No Target Country*/
    0x00,
    /*Number of HID Descriptors*/
    0x01,
    /*Type of HID Descriptor = "Report"*/
    0x22,
    /*Length Of HID Report Descriptor*/
    HID_SPECIAL_REPORT_DESCRIPTOR_SIZE,00,

    /*Size of this descriptor*/
    0x07,
    /*ENDPOINT Descriptor*/
    0x05,
    /*bEndpointAddress - IN endpoint, endpoint number = 3*/
    0x86,
    /*Endpoint Type is Interrupt*/
    0x03,
    /*Max Packet Size*/
    INTERRUPT_IN_PACKET_SIZE,0x00,
    /*Polling Interval in mS*/
    0x0A
};

static const uint8_t HID_MOUSE_InterfaceDescriptorData[HID_INTERFACE_DESCRIPTOR_SIZE + (HID_INTERFACE_DESCRIPTOR_SIZE % 2)] = {
    /*Size of this descriptor*/
    0x09,
    /*INTERFACE Descriptor*/
    0x04,
    /*Index of Interface*/
    0x03,
    /*bAlternateSetting*/
    0x00,
    /*Number of Endpoints (Interrupt IN)*/
    0x01,
    /*HID Class code*/
    0x03,
    /*Boot*/
    0x01,
    /*Mouse*/
    0x02,
    /*No String Descriptor for this interface*/
    0x00,

    /*Size of this descriptor*/
    0x09,
    /*HID Descriptor*/
    0x21,
    /*HID Class Specification Release Number 1.11*/
    0x11,0x01,
    /*No Target Country*/
    0x00,
    /*Number of HID Descriptors*/
    0x01,
    /*Type of HID Descriptor = "Report"*/
    0x22,
    /*Length Of HID Report Descriptor*/
    HID_MOUSE_REPORT_DESCRIPTOR_SIZE,00,

    /*Size of this descriptor*/
    0x07,
    /*ENDPOINT Descriptor*/
    0x05,
    /*bEndpointAddress - IN endpoint, endpoint number = 3*/
    0x86,
    /*Endpoint Type is Interrupt*/
    0x03,
    /*Max Packet Size*/
    HID_MOUSE_ENDPOINT_MAX_SIZE,0x00,
    /*Polling Interval in mS*/
    0x0A
};

static const uint8_t HID_KEYBOARD_InterfaceDescriptorData[HID_INTERFACE_DESCRIPTOR_SIZE + (HID_INTERFACE_DESCRIPTOR_SIZE % 2)] = {
    /*Size of this descriptor*/
    0x09,
    /*INTERFACE Descriptor*/
    0x04,
    /*Index of Interface*/
    0x03,
    /*bAlternateSetting*/
    0x00,
    /*Number of Endpoints (Interrupt IN)*/
    0x01,
    /*HID Class code*/
    0x03,
    /*Boot*/
    0x01,
    /*Keyboard*/
    0x01,
    /*No String Descriptor for this interface*/
    0x00,

    /*Size of this descriptor*/
    0x09,
    /*HID Descriptor*/
    0x21,
    /*HID Class Specification Release Number 1.11*/
    0x11,0x01,
    /*No Target Country*/
    0x00,
    /*Number of HID Descriptors*/
    0x01,
    /*Type of HID Descriptor = "Report"*/
    0x22,
    /*Length Of HID Report Descriptor*/
    HID_KEYBOARD_REPORT_DESCRIPTOR_SIZE,00,

    /*Size of this descriptor*/
    0x07,
    /*ENDPOINT Descriptor*/
    0x05,
    /*bEndpointAddress - IN endpoint, endpoint number = 3*/
    0x86,
    /*Endpoint Type is Interrupt*/
    0x03,
    /*Max Packet Size*/
    HID_KEYBOARD_ENDPOINT_MAX_SIZE,0x00,
    /*Polling Interval in mS*/
    0x0A
};

const DESCRIPTOR gConfigurationDescriptor =
{
    CONFIG_DESCRIPTOR_SIZE, (uint8_t* )gConfigurationDescriptorData
};

/*String Descriptors*/
    /*Note Language ID is in USB Core */

/*Manufacturer string*/
#define STRING_MANUFACTURER_SIZE 16
/* "Renesas" */
static const uint8_t gStringDescriptorManufacturerData[STRING_MANUFACTURER_SIZE] =
{
    /* Length of this descriptor*/
    STRING_MANUFACTURER_SIZE,
    /* Descriptor Type = STRING */
    0x03,
    /* Descriptor Text (unicode) */
    'R', 0x00, 'E', 0x00, 'N', 0x00, 'E', 0x00,
    'S', 0x00, 'A', 0x00, 'S', 0x00
};

const DESCRIPTOR  gStringDescriptorManufacturer =
{
    STRING_MANUFACTURER_SIZE,
    (uint8_t *)gStringDescriptorManufacturerData
};

/*Product string*/
#if defined(GRSAKURA)
#define STRING_PRODUCT_SIZE 44
#elif defined(GRCITRUS)
#define STRING_PRODUCT_SIZE 44
#elif defined(GRROSE)
#define STRING_PRODUCT_SIZE 42
#endif
/* "CDC USB Demonstration" */
static const uint8_t gStringDescriptorProductData[STRING_PRODUCT_SIZE] =
{
    /* Length of this descriptor*/
    STRING_PRODUCT_SIZE,
    /* Descriptor Type = STRING */
    0x03,
    /* Descriptor Text (unicode) */
#ifdef GRSAKURA
    'G', 0x00, 'a', 0x00, 'd', 0x00, 'g', 0x00,
    'e', 0x00, 't', 0x00, ' ', 0x00, 'R', 0x00,
    'e', 0x00, 'n', 0x00, 'e', 0x00, 's', 0x00,
    'a', 0x00, 's', 0x00, ' ', 0x00, 'S', 0x00,
    'A', 0x00, 'K', 0x00, 'U', 0x00, 'R', 0x00,
    'A', 0x00
#elif defined GRCITRUS
    'G', 0x00, 'a', 0x00, 'd', 0x00, 'g', 0x00,
    'e', 0x00, 't', 0x00, ' ', 0x00, 'R', 0x00,
    'e', 0x00, 'n', 0x00, 'e', 0x00, 's', 0x00,
    'a', 0x00, 's', 0x00, ' ', 0x00, 'C', 0x00,
    'I', 0x00, 'T', 0x00, 'R', 0x00, 'U', 0x00,
    'S', 0x00
#elif defined GRROSE
    'G', 0x00, 'a', 0x00, 'd', 0x00, 'g', 0x00,
    'e', 0x00, 't', 0x00, ' ', 0x00, 'R', 0x00,
    'e', 0x00, 'n', 0x00, 'e', 0x00, 's', 0x00,
    'a', 0x00, 's', 0x00, ' ', 0x00, 'R', 0x00,
    'O', 0x00, 'S', 0x00
#endif
};

const DESCRIPTOR gStringDescriptorProduct =
{
    STRING_PRODUCT_SIZE,
    (uint8_t *)gStringDescriptorProductData
};

/*Serial number string "1.1"*/
#define STRING_SERIAL_NUM_SIZE 8
static const uint8_t gStringDescriptorSerialNumData[STRING_SERIAL_NUM_SIZE] =
{
    /* Length of this descriptor*/
    STRING_SERIAL_NUM_SIZE,
    /* Descriptor Type = STRING */
    0x03,
    /* Descriptor Text (unicode) */
    '1', 0x00, '.', 0x00, '1', 0x00
};

const DESCRIPTOR gStringDescriptorSerialNum =
{
    STRING_SERIAL_NUM_SIZE,
    (uint8_t *)gStringDescriptorSerialNumData
};

/*
Report Descriptor
NOTE The size of this must be HID_REPORT_DESCRIPTOR_SIZE
*/
static const uint8_t gHIDSpecialReportDescriptorData[HID_SPECIAL_REPORT_DESCRIPTOR_SIZE + (HID_SPECIAL_REPORT_DESCRIPTOR_SIZE % 2)] =
{
    /* Usage Page - Vendor defined*/
    0x06, 0xA0, 0xFF,
    /* Usage ID within this page (Vendor defined)*/
    0x09, 0x00,
    /* Collection App (Windows requires an Application Collection) */
    0xA1, 0x01,

    /* *** The INPUT REPORT *** */
    /* Usage ID within this page*/
    0x09, 0x00,
    /*Logical Min 0 */
    0x15, 0x00,
    /*Logical Max 255 */
    0x26, 0xFF, 0x00,
    /* Size 8 Bits (Each Field will be 8bits) */
    0x75, 0x08,
    /* Count (Number of fields(bytes) in INPUT report) */
    0x95, HID_SPECIAL_INPUT_REPORT_SIZE,
    /* Input Report - type variable data */
    0x81, 0x02,

    /* *** The OUTPUR REPORT *** */
    /* Usage ID within this page (Vendor defined)*/
    0x09, 0x00,
    /*Logical Min 0 */
    0x15, 0x00,
    /*Logical Max 255 */
    0x26, 0xFF, 0x00,
    /* Size 8 Bits (Each Field will be 8bits) */
    0x75, 0x08,
    /* Count (Number of fields(bytes) in INPUT report) */
    0x95, HID_SPECIAL_OUTPUT_REPORT_SIZE,
    /* Output Report - type variable data */
    0x91, 0x02,

    /* End collection */
    0xC0
};

static const uint8_t gHIDMouseReportDescriptorData[HID_MOUSE_REPORT_DESCRIPTOR_SIZE + (HID_MOUSE_REPORT_DESCRIPTOR_SIZE % 2)] =
{
    0x05, 0x01,  // Usage Page (Generic Desktop)
    0x09, 0x02,  // Usage (Mouse)
    0xA1, 0x01,  // Collection (Application)
    0x09, 0x01,  //   Usage (Pointer)
    0xA1, 0x00,  //   Collection (Physical)

    0x05, 0x09,  //     Usage Page (Button)
    0x19, 0x01,  //     Usage Minimum (0x01)
    0x29, 0x03,  //     Usage Maximum (0x05)
    0x15, 0x00,  //     Logical Minimum (0)
    0x25, 0x01,  //     Logical Maximum (1)

    0x95, 0x03,  //     Report Count (3)
    0x75, 0x01,  //     Report Size (1)
    0x81, 0x02,  //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01,  //     Report Count (1)
    0x75, 0x05,  //     Report Size (3)

    0x81, 0x03,  //     Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,  //     Usage Page (Generic Desktop Ctrls)
    0x09, 0x30,  //     Usage (X)
    0x09, 0x31,  //     Usage (Y)
    0x09, 0x38,  //     Usage (Wheel),

    0x15, 0x81,  //     Logical Minimum (-127)
    0x25, 0x7F,  //     Logical Maximum (127)
    0x75, 0x08,  //     Report Size (8)
    0x95, 0x03,  //     Report Count (3)
    0x81, 0x06,  //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)

    0xC0,        //   End Collection
    0xC0         // End Collection
};

static const uint8_t gHIDKeyboardReportDescriptorData[HID_KEYBOARD_REPORT_DESCRIPTOR_SIZE + (HID_KEYBOARD_REPORT_DESCRIPTOR_SIZE % 2)] =
{
    0x05, 0x01,  // Usage Page (Generic Desktop)
    0x09, 0x06,  // Usage (Keyboard)
    0xA1, 0x01,  // Collection (Application)
    0x05, 0x07,  //   Usage (Key Code)
    0x19, 0xe0,  //     Usage Minimum (224)
    0x29, 0xe7,  //     Usage Maximum (231)
    0x15, 0x00,  //     Logical Minimum (0)
    0x25, 0x01,  //     Logical Maximum (1)
    0x75, 0x01,  //     Report Size (1)
    0x95, 0x08,  //     Report Count (8)
    0x81, 0x02,  //     Input (Data, Variable, Absolute)
    0x95, 0x01,  //     Report Count (1)
    0x75, 0x08,  //     Report Size (8)
    0x81, 0x01,  //     Input (Constant)
    0x95, 0x05,  //     Report Count (5)
    0x75, 0x01,  //     Report Size (1)
    0x05, 0x08,  //     Usage Page (LED)
    0x19, 0x01,  //     Usage Minimum (1), Num Lock, Caps Lock, Scroll Lock, Compose, Kana
    0x29, 0x05,  //     Usage Maximum (5)
    0x91, 0x02,  //     Output (Data, Variable, Absolute)
    0x95, 0x01,  //     Report Count (1)
    0x75, 0x03,  //     Report Size (3)
    0x91, 0x01,  //     Output (Constant)
    0x95, 0x06,  //     Report Count (6)
    0x75, 0x08,  //     Report Size (8)
    0x15, 0x00,  //     Logical Minimum (0)
    0x25, 0x65,  //     Logical Maximum (101)
    0x05, 0x07,  //   Usage (Key Code)
    0x19, 0x00,  //     Usage Minimum (0)
    0x29, 0x65,  //     Usage Maximum (101)
    0x81, 0x00,  //     Input (Data, Array)
    0xC0         // End Collection
};

DESCRIPTOR gHIDReportDescriptor =
{
    /* Must match value specified in HID descriptor! */
#if defined(HID_SPECIAL)
    HID_SPECIAL_REPORT_DESCRIPTOR_SIZE,
    (uint8_t *)gHIDSpecialReportDescriptorData
#endif
#if defined(HID_MOUSE)
    HID_MOUSE_REPORT_DESCRIPTOR_SIZE,
    (uint8_t *)gHIDMouseReportDescriptorData
#endif
#if defined(HID_KEYBOARD)
    HID_KEYBOARD_REPORT_DESCRIPTOR_SIZE,
    (uint8_t *)gHIDMouseReportDescriptorData
#endif
};

#if defined(USB_HID)
void SetHIDReportDescriptor(char *desc, int size) {
    gHIDReportDescriptor.pucData = (uint8_t *)desc;
    gHIDReportDescriptor.length = (uint16_t)size;
}

void SetPIDVID(uint8_t pid, uint8_t vid) {
    gConfigurationDescriptorData[8] = (uint8_t)(pid & 0xFF);
    gConfigurationDescriptorData[9] = (uint8_t)(pid >> 8);
    gConfigurationDescriptorData[10] = (uint8_t)(vid & 0xFF);
    gConfigurationDescriptorData[11] = (uint8_t)(vid >> 8);
}

void SetHIDInterfaceDescriptor(char *desc) {
    memcpy((uint8_t *)&gConfigurationDescriptorData[HID_INTERFACE_DESCRIPTOR_OFS], (const uint8_t *)desc, (size_t)HID_INTERFACE_DESCRIPTOR_SIZE);
}

void SetHIDMode(int hid_mode) {
    switch(hid_mode) {
        case HID_SPECIAL_MODE:
            SetHIDInterfaceDescriptor((char *)HID_SPECIAL_InterfaceDescriptorData);
            break;
        case HID_MOUSE_MODE:
            SetHIDInterfaceDescriptor((char *)HID_MOUSE_InterfaceDescriptorData);
            break;
        case HID_KEYBOARD_MODE:
            SetHIDInterfaceDescriptor((char *)HID_KEYBOARD_InterfaceDescriptorData);
            break;
    }
}

void SetDefaultHIDReportDescriptor(int hid_mode) {
    switch(hid_mode) {
        case HID_SPECIAL_MODE:
            SetHIDReportDescriptor((char *)gHIDSpecialReportDescriptorData, (int)sizeof(gHIDSpecialReportDescriptorData));
            break;
        case HID_MOUSE_MODE:
            SetHIDReportDescriptor((char *)gHIDSpecialReportDescriptorData, (int)sizeof(gHIDMouseReportDescriptorData));
            break;
        case HID_KEYBOARD_MODE:
            SetHIDReportDescriptor((char *)gHIDSpecialReportDescriptorData, (int)sizeof(gHIDKeyboardReportDescriptorData));
            break;
    }
}
#endif
