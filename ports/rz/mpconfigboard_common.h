/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Damien P. George
 * Copyright (c) 2018 Kentaro Sekimoto
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

//#include RZ_HAL_H

/*****************************************************************************/
// Feature settings with defaults

// Whether to include the rz module, with peripheral register constants
#ifndef MICROPY_PY_RZ
#define MICROPY_PY_RZ (1)
#endif

// Whether to include legacy functions and classes in the pyb module
#ifndef MICROPY_PY_PYB_LEGACY
#define MICROPY_PY_PYB_LEGACY (1)
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

// SD/MMC interface bus width (defaults to 4 bits)
#ifndef MICROPY_HW_SDMMC_BUS_WIDTH
#define MICROPY_HW_SDMMC_BUS_WIDTH (4)
#endif

// Whether to automatically mount (and boot from) the SD card if it's present
#ifndef MICROPY_HW_SDCARD_MOUNT_AT_BOOT
#define MICROPY_HW_SDCARD_MOUNT_AT_BOOT (MICROPY_HW_ENABLE_SDCARD)
#endif

// Whether to enable the MMA7660 driver, exposed as pyb.Accel
#ifndef MICROPY_HW_HAS_MMA7660
#define MICROPY_HW_HAS_MMA7660 (0)
#endif

// Whether to enable the LCD32MK driver, exposed as pyb.LCD
#ifndef MICROPY_HW_HAS_LCD
#define MICROPY_HW_HAS_LCD (0)
#endif

// The volume label used when creating the flash filesystem
#ifndef MICROPY_HW_FLASH_FS_LABEL
//#define MICROPY_HW_FLASH_FS_LABEL "pybflash"
#define MICROPY_HW_FLASH_FS_LABEL "rzbflash"
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
// General configuration

// Heap start / end definitions
#ifndef MICROPY_HEAP_START
#define MICROPY_HEAP_START &_heap_start
#endif
#ifndef MICROPY_HEAP_END
#define MICROPY_HEAP_END &_heap_end
#endif

// Configuration for RZA2M series

//#define MP_HAL_UNIQUE_ID_ADDRESS (0x1ffff7ac)
#define PYB_EXTI_NUM_VECTORS (16)
#define MICROPY_HW_MAX_TIMER (17)
#define MICROPY_HW_MAX_UART (13)

//#if defined(RZA2M)
//#define MP_HAL_UNIQUE_ID_ADDRESS (0xFEFFFAC0)
//#else
//#error "MP_HAL_UNIQUE_ID_ADDRESS should be defined."
//#endif

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
#if defined(MICROPY_HW_CAN1_TX) || defined(MICROPY_HW_CAN2_TX)
#define MICROPY_HW_ENABLE_CAN (1)
#else
#define MICROPY_HW_ENABLE_CAN (0)
#endif

// Pin definition header file
#define MICROPY_PIN_DEFS_PORT_H "pin_defs_rz.h"

