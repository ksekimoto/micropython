/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Damien P. George
 * Copyright (c) 2021 Kentaro Sekimoto
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// Common settings and defaults for board configuration.
// The defaults here should be overridden in mpconfigboard.h.

// #include RX_HAL_H

/*****************************************************************************/
// Feature settings with defaults

// Whether to include the rx module, with peripheral register constants
#ifndef MICROPY_PY_RX
#define MICROPY_PY_RX (1)
#endif

#ifndef MICROPY_PY_RXREG
#define MICROPY_PY_RXREG (0)    // can't be 1 because not implemented yet.
#endif

// Whether to include the pyb module
#ifndef MICROPY_PY_PYB
#define MICROPY_PY_PYB (1)
#endif

// Whether to include legacy functions and classes in the pyb module
#ifndef MICROPY_PY_PYB_LEGACY
#define MICROPY_PY_PYB_LEGACY (1)
#endif

// Whether machine.bootloader() will enter the bootloader via reset, or direct jump.
#ifndef MICROPY_HW_ENTER_BOOTLOADER_VIA_RESET
#define MICROPY_HW_ENTER_BOOTLOADER_VIA_RESET (1)
#endif

// Whether to enable storage on the internal flash of the MCU
#ifndef MICROPY_HW_ENABLE_INTERNAL_FLASH_STORAGE
#define MICROPY_HW_ENABLE_INTERNAL_FLASH_STORAGE (1)
#endif

// Whether to enable the RTC, exposed as pyb.RTC
#ifndef MICROPY_HW_ENABLE_RTC
#define MICROPY_HW_ENABLE_RTC (0)
#endif

// Whether to enable the hardware RNG peripheral, exposed as pyb.rng()
#ifndef MICROPY_HW_ENABLE_RNG
#define MICROPY_HW_ENABLE_RNG (0)
#endif

// Whether to enable the ADC peripheral, exposed as pyb.ADC and pyb.ADCAll
#ifndef MICROPY_HW_ENABLE_ADC
#define MICROPY_HW_ENABLE_ADC (1)
#endif

// Whether to enable the DAC peripheral, exposed as pyb.DAC
#ifndef MICROPY_HW_ENABLE_DAC
#define MICROPY_HW_ENABLE_DAC (0)
#endif

// Whether to enable the DCMI peripheral
#ifndef MICROPY_HW_ENABLE_DCMI
#define MICROPY_HW_ENABLE_DCMI (0)
#endif

// Whether to enable USB support
#ifndef MICROPY_HW_ENABLE_USB
#define MICROPY_HW_ENABLE_USB (0)
#endif

// Whether to enable the PA0-PA3 servo driver, exposed as pyb.Servo
#ifndef MICROPY_HW_ENABLE_SERVO
#define MICROPY_HW_ENABLE_SERVO (1)
#endif

// Whether to enable a USR switch, exposed as pyb.Switch
#ifndef MICROPY_HW_HAS_SWITCH
#define MICROPY_HW_HAS_SWITCH (0)
#endif

// Whether to expose internal flash storage as pyb.Flash
#ifndef MICROPY_HW_HAS_FLASH
#define MICROPY_HW_HAS_FLASH (0)
#endif

// Whether to enable the SD card interface, exposed as pyb.SDCard
#ifndef MICROPY_HW_ENABLE_SDCARD
#define MICROPY_HW_ENABLE_SDCARD (0)
#endif

// Whether to enable the MMC interface, exposed as pyb.MMCard
#ifndef MICROPY_HW_ENABLE_MMCARD
#define MICROPY_HW_ENABLE_MMCARD (0)
#endif

// Which SDMMC peripheral to use for the SD/MMC card driver (1 or 2)
#ifndef MICROPY_HW_SDCARD_SDMMC
#define MICROPY_HW_SDCARD_SDMMC (1)
#endif

// SD/MMC card driver interface bus width (defaults to 4 bits)
#ifndef MICROPY_HW_SDCARD_BUS_WIDTH
#define MICROPY_HW_SDCARD_BUS_WIDTH (4)
#endif

// Whether to automatically mount (and boot from) the SD card if it's present
#ifndef MICROPY_HW_SDCARD_MOUNT_AT_BOOT
#define MICROPY_HW_SDCARD_MOUNT_AT_BOOT (MICROPY_HW_ENABLE_SDCARD)
#endif

// Which SDMMC peripheral to use for the SDIO driver (1 or 2)
#ifndef MICROPY_HW_SDIO_SDMMC
#define MICROPY_HW_SDIO_SDMMC (1)
#endif

// Whether to enable the MMA7660 driver, exposed as pyb.Accel
#ifndef MICROPY_HW_HAS_MMA7660
#define MICROPY_HW_HAS_MMA7660 (0)
#endif

// Whether to enable the LCDSPI driver, exposed as pyb.LCDSPI
#ifndef MICROPY_HW_ENABLE_LCDSPI
#define MICROPY_HW_ENABLE_LCDSPI (1)
#endif

#ifndef MICROPY_PY_PYB_LCDSPI
#if MICROPY_HW_ENABLE_LCDSPI
#define MICROPY_PY_PYB_LCDSPI 0
#else
#define MICROPY_PY_PYB_LCDSPI 1
#endif
#endif

// Whether to automatically mount (and boot from) the flash filesystem
#ifndef MICROPY_HW_FLASH_MOUNT_AT_BOOT
#define MICROPY_HW_FLASH_MOUNT_AT_BOOT (MICROPY_HW_ENABLE_STORAGE)
#endif

// The volume label used when creating the flash filesystem
#ifndef MICROPY_HW_FLASH_FS_LABEL
// #define MICROPY_HW_FLASH_FS_LABEL "pybflash"
#define MICROPY_HW_FLASH_FS_LABEL "rxbflash"
#endif

// Function to determine if the given can_id is reserved for system use or not.
#ifndef MICROPY_HW_CAN_IS_RESERVED
#define MICROPY_HW_CAN_IS_RESERVED(can_id) (false)
#endif

// Function to determine if the given i2c_id is reserved for system use or not.
#ifndef MICROPY_HW_I2C_IS_RESERVED
#define MICROPY_HW_I2C_IS_RESERVED(i2c_id) (false)
#endif

// Function to determine if the given spi_id is reserved for system use or not.
#ifndef MICROPY_HW_SPI_IS_RESERVED
#define MICROPY_HW_SPI_IS_RESERVED(spi_id) (false)
#endif

// Function to determine if the given tim_id is reserved for system use or not.
#ifndef MICROPY_HW_TIM_IS_RESERVED
#define MICROPY_HW_TIM_IS_RESERVED(tim_id) (false)
#endif

// Function to determine if the given uart_id is reserved for system use or not.
#ifndef MICROPY_HW_UART_IS_RESERVED
#define MICROPY_HW_UART_IS_RESERVED(uart_id) (false)
#endif

/*****************************************************************************/
// USB configuration

// The USBD_xxx macros have been renamed to MICROPY_HW_USB_xxx.
#if defined(USBD_VID) \
    || defined(USBD_LANGID_STRING) \
    || defined(USBD_MANUFACTURER_STRING) \
    || defined(USBD_PRODUCT_HS_STRING) \
    || defined(USBD_PRODUCT_FS_STRING) \
    || defined(USBD_CONFIGURATION_HS_STRING) \
    || defined(USBD_INTERFACE_HS_STRING) \
    || defined(USBD_CONFIGURATION_FS_STRING) \
    || defined(USBD_INTERFACE_FS_STRING) \
    || defined(USBD_CDC_RX_DATA_SIZE) \
    || defined(USBD_CDC_TX_DATA_SIZE)
#error "Old USBD_xxx configuration option used, renamed to MICROPY_HW_USB_xxx"
#endif

// Default VID and PID values to use for the USB device.  If MICROPY_HW_USB_VID
// is defined by a board then all needed PID options must also be defined.  The
// VID and PID can also be set dynamically in pyb.usb_mode().
// Windows needs a different PID to distinguish different device configurations.
// #ifndef MICROPY_HW_USB_VID
// #define MICROPY_HW_USB_VID              (0xf055)
// #define MICROPY_HW_USB_PID_CDC_MSC      (0x9800)
// #define MICROPY_HW_USB_PID_CDC_HID      (0x9801)
// #define MICROPY_HW_USB_PID_CDC          (0x9802)
// #define MICROPY_HW_USB_PID_MSC          (0x9803)
// #define MICROPY_HW_USB_PID_CDC2_MSC     (0x9804)
// #define MICROPY_HW_USB_PID_CDC2         (0x9805)
// #define MICROPY_HW_USB_PID_CDC3         (0x9806)
// #define MICROPY_HW_USB_PID_CDC3_MSC     (0x9807)
// #define MICROPY_HW_USB_PID_CDC_MSC_HID  (0x9808)
// #define MICROPY_HW_USB_PID_CDC2_MSC_HID (0x9809)
// #define MICROPY_HW_USB_PID_CDC3_MSC_HID (0x980a)
// #endif

// Windows needs a different PID to distinguish different device configurations
#ifdef GRSAKURA
#define MICROPY_HW_USB_VID            (0x045B)
#define MICROPY_HW_USB_PID_CDC_MSC    (0x0234)
#define MICROPY_HW_USB_PID_CDC_HID    (0x0234)
#define MICROPY_HW_USB_PID_CDC        (0x0234)
#define MICROPY_HW_USB_PID_MSC        (0x0234)
#define MICROPY_HW_USB_PID_CDC2_MSC   (0x0234)
#elif defined(GRCITRUS)
#define MICROPY_HW_USB_VID            (0x2A50)
#define MICROPY_HW_USB_PID_CDC_MSC    (0x0277)
#define MICROPY_HW_USB_PID_CDC_HID    (0x0277)
#define MICROPY_HW_USB_PID_CDC        (0x0277)
#define MICROPY_HW_USB_PID_MSC        (0x0277)
#define MICROPY_HW_USB_PID_CDC2_MSC   (0x0277)
#elif defined(GRROSE)
#define MICROPY_HW_USB_VID            (0x045B)
#define MICROPY_HW_USB_PID_CDC_MSC    (0x025A)
#define MICROPY_HW_USB_PID_CDC_HID    (0x025A)
#define MICROPY_HW_USB_PID_CDC        (0x025A)
#define MICROPY_HW_USB_PID_MSC        (0x025A)
#define MICROPY_HW_USB_PID_CDC2_MSC   (0x025A)
#endif

#ifndef MICROPY_HW_USB_LANGID_STRING
#define MICROPY_HW_USB_LANGID_STRING            0x409
#endif

#ifndef MICROPY_HW_USB_MANUFACTURER_STRING
#define MICROPY_HW_USB_MANUFACTURER_STRING      "MicroPython"
#endif

#ifndef MICROPY_HW_USB_PRODUCT_HS_STRING
#define MICROPY_HW_USB_PRODUCT_HS_STRING        "Pyboard Virtual Comm Port in HS Mode"
#endif

#ifndef MICROPY_HW_USB_PRODUCT_FS_STRING
#define MICROPY_HW_USB_PRODUCT_FS_STRING        "Pyboard Virtual Comm Port in FS Mode"
#endif

#ifndef MICROPY_HW_USB_CONFIGURATION_HS_STRING
#define MICROPY_HW_USB_CONFIGURATION_HS_STRING  "Pyboard Config"
#endif

#ifndef MICROPY_HW_USB_INTERFACE_HS_STRING
#define MICROPY_HW_USB_INTERFACE_HS_STRING      "Pyboard Interface"
#endif

#ifndef MICROPY_HW_USB_CONFIGURATION_FS_STRING
#define MICROPY_HW_USB_CONFIGURATION_FS_STRING  "Pyboard Config"
#endif

#ifndef MICROPY_HW_USB_INTERFACE_FS_STRING
#define MICROPY_HW_USB_INTERFACE_FS_STRING      "Pyboard Interface"
#endif

// Amount of incoming buffer space for each CDC instance.
// This must be 2 or greater, and a power of 2.
#ifndef MICROPY_HW_USB_CDC_RX_DATA_SIZE
#define MICROPY_HW_USB_CDC_RX_DATA_SIZE (1024)
#endif

// Amount of outgoing buffer space for each CDC instance.
// This must be a power of 2 and no greater than 16384.
#ifndef MICROPY_HW_USB_CDC_TX_DATA_SIZE
#define MICROPY_HW_USB_CDC_TX_DATA_SIZE (1024)
#endif

/*****************************************************************************/
// General configuration

// Heap start / end definitions
#ifndef MICROPY_HEAP_START
#define MICROPY_HEAP_START &_heap_start
#endif
#ifndef MICROPY_HEAP_END
#define MICROPY_HEAP_END &_heap_end
#endif

// Configuration for RX63N series

// #define MP_HAL_UNIQUE_ID_ADDRESS (0x1ffff7ac)
#define PYB_EXTI_NUM_VECTORS (16)
#define MICROPY_HW_MAX_TIMER (17)
#define MICROPY_HW_MAX_UART (13)

#if defined(RX63N)
#define MP_HAL_UNIQUE_ID_ADDRESS (0xFEFFFAC0)
#elif defined(RX65N)
#define MP_HAL_UNIQUE_ID_ADDRESS (0xFE7F7D90)
#else
#error "MP_HAL_UNIQUE_ID_ADDRESS should be defined."
#endif

// Configure HSE for bypass or oscillator
#if MICROPY_HW_CLK_USE_BYPASS
#define MICROPY_HW_CLK_HSE_STATE (RCC_HSE_BYPASS)
#else
#define MICROPY_HW_CLK_HSE_STATE (RCC_HSE_ON)
#endif

#if MICROPY_HW_ENABLE_INTERNAL_FLASH_STORAGE
// Provide block device macros if internal flash storage is enabled
#define MICROPY_HW_BDEV_IOCTL flash_bdev_ioctl
#define MICROPY_HW_BDEV_READBLOCK flash_bdev_readblock
#define MICROPY_HW_BDEV_WRITEBLOCK flash_bdev_writeblock
#endif

// Whether to enable caching for external SPI flash, to allow block writes that are
// smaller than the native page-erase size of the SPI flash, eg when FAT FS is used.
// Enabling this enables spi_bdev_readblocks() and spi_bdev_writeblocks() functions,
// and requires a valid mp_spiflash_config_t.cache pointer.
#ifndef MICROPY_HW_SPIFLASH_ENABLE_CACHE
#define MICROPY_HW_SPIFLASH_ENABLE_CACHE (0)
#endif

// Enable the storage sub-system if a block device is defined
#if defined(MICROPY_HW_BDEV_IOCTL)
#define MICROPY_HW_ENABLE_STORAGE (1)
#else
#define MICROPY_HW_ENABLE_STORAGE (0)
#endif

// Enable hardware I2C if there are any peripherals defined
#if defined(MICROPY_HW_I2C1_SCL) || defined(MICROPY_HW_I2C2_SCL) \
    || defined(MICROPY_HW_I2C3_SCL) || defined(MICROPY_HW_I2C4_SCL)
#define MICROPY_HW_ENABLE_HW_I2C (1)
#else
#define MICROPY_HW_ENABLE_HW_I2C (0)
#endif

// Enable CAN if there are any peripherals defined
#if defined(MICROPY_HW_CAN1_TX) || defined(MICROPY_HW_CAN2_TX) || defined(MICROPY_HW_CAN3_TX)
#define MICROPY_HW_ENABLE_CAN (1)
#else
#define MICROPY_HW_ENABLE_CAN (0)
#define MICROPY_HW_MAX_CAN (0)
#endif
#if defined(MICROPY_HW_CAN3_TX)
#define MICROPY_HW_MAX_CAN (3)
#elif defined(MICROPY_HW_CAN2_TX)
#define MICROPY_HW_MAX_CAN (2)
#elif defined(MICROPY_HW_CAN1_TX)
#define MICROPY_HW_MAX_CAN (1)
#endif

// Enable I2S if there are any peripherals defined
#if defined(MICROPY_HW_I2S1) || defined(MICROPY_HW_I2S2)
#define MICROPY_HW_ENABLE_I2S (1)
#define MICROPY_HW_MAX_I2S (2)
#else
#define MICROPY_HW_ENABLE_I2S (0)
#define MICROPY_HW_MAX_I2S (0)
#endif

// Define MICROPY_HW_SDMMCx_CK values if that peripheral is used, so that make-pins.py
// generates the relevant AF constants.
#if MICROPY_HW_SDCARD_SDMMC == 1 || MICROPY_HW_SDIO_SDMMC == 1
#define MICROPY_HW_SDMMC1_CK (1)
#endif
#if MICROPY_HW_SDCARD_SDMMC == 2 || MICROPY_HW_SDIO_SDMMC == 2
#define MICROPY_HW_SDMMC2_CK (1)
#endif

#ifndef MICROPY_HW_HAS_ESP8266
#define MICROPY_HW_HAS_ESP8266 (0)
#endif

#ifndef MICROPY_HW_HAS_TWITTER
#define MICROPY_HW_HAS_TWITTER (0)
#endif

#ifndef MICROPY_HW_HAS_FONT
#define MICROPY_HW_HAS_FONT (1)
#endif

#ifndef MICROPY_HW_ENABLE_LCDSPI
#define MICROPY_HW_ENABLE_LCDSPI (1)
#endif

// Pin definition header file
#define MICROPY_PIN_DEFS_PORT_H "pin_defs_rx.h"
