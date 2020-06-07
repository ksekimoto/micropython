/**
 * This file defines the Micorpython API to Renesas RZ
 * It is used as input to gen_mpy.py to create a micropython module
 **/ 
#if 0
#if __has_include("esp_idf_version.h")
#   include "esp_idf_version.h"
#endif

// Disable some macros and includes that make pycparser choke

#ifdef PYCPARSER
#define __attribute__(x)
#define _SOC_IO_MUX_REG_H_
#define _SYS_REENT_H_
#define PORTMACRO_H
#define PORTABLE_H
#define INC_FREERTOS_H
#define QUEUE_H
#define SEMAPHORE_H
#define XTENSA_HAL_H
#define _SOC_I2S_STRUCT_H_
#define XTRUNTIME_H
#define _SOC_SPI_STRUCT_H_
#define _SOC_RTC_CNTL_STRUCT_H_
#define __XTENSA_API_H__

// Exclude SOC just because it contains large structs that don't interest the user
#define _SOC_SPI_PERIPH_H_
typedef void *spi_dev_t;

// TODO: Check why lldesc_t causes inifinite recursion on gen_mpy.py 
#define _ROM_LLDESC_H_
typedef void *lldesc_t; 

// FreeRTOS portmaco is excluded, but we still need TickType_t
#include <stdint.h>
typedef uint32_t TickType_t;

// Micropython specific types
typedef void *mp_obj_t;

static inline void get_ccount(int *ccount);

#else // PYCPARSER

// Helper function to measure CPU cycles
//
static inline void get_ccount(int *ccount)
{
	asm volatile("rsr.ccount %0" : "=a"(*ccount));
}

#endif //PYCPARSER

// The following includes are the source of the esp-idf micropython module.
// All included files are API we want to include in the module

#if defined(ESP_IDF_VERSION_MAJOR) && ESP_IDF_VERSION_MAJOR >= 4
#   include "esp32/clk.h"
#else
#   include "esp_clk.h"
#endif

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "driver/adc.h"
#include "driver/i2s.h"
#include "driver/pcnt.h"

// We don't want the whole FreeRTOS, only selected functions

void task_delay_ms(int ms);

// The binding only publishes structs that are used in some function. We need spi_transaction_ext_t
// TOOD: Find some way to mark structs for binding export instead of new function.
static inline void set_spi_transaction_ext(
        spi_transaction_ext_t *ext_trans,
        spi_transaction_t *trans,
        uint8_t command_bits,
        uint8_t address_bits){
    ext_trans->base = *trans;
    ext_trans->command_bits = command_bits;
    ext_trans->address_bits = address_bits;
}

// Wrapper for safe ISR callbacks from micropython
// Need to call both spi_transaction_set_cb and set spi_pre/post_cb_isr!

// Use this to set pre/post callbacks for spi transaction.
// pre_cb/post_cb should be either a callable object or "None".
// Result should be set to spi_transaction_t user field.
// Allocates RAM.

void *spi_transaction_set_cb(mp_obj_t pre_cb, mp_obj_t post_cb);

// These functions can be set into pre_cb/post_cb of spi_device_interface_config_t

void spi_pre_cb_isr(spi_transaction_t *trans);
void spi_post_cb_isr(spi_transaction_t *trans);

// Useful constants

#define EXPORT_CONST_INT(int_value) enum {ENUM_##int_value = int_value}

#if defined(ESP_IDF_VERSION_MAJOR) && ESP_IDF_VERSION_MAJOR >= 4
// SPI HOST enum was changed to macros on v4
enum {
    ENUM_SPI_HOST = SPI_HOST,
    ENUM_HSPI_HOST = HSPI_HOST,
    ENUM_VSPI_HOST = VSPI_HOST,
};
#endif

enum {
    ENUM_portMAX_DELAY = portMAX_DELAY
};

enum {
    ENUM_I2S_PIN_NO_CHANGE = I2S_PIN_NO_CHANGE
};

enum {
    ENUM_SPI_DEVICE_TXBIT_LSBFIRST = SPI_DEVICE_TXBIT_LSBFIRST,
    ENUM_SPI_DEVICE_RXBIT_LSBFIRST = SPI_DEVICE_RXBIT_LSBFIRST,
    ENUM_SPI_DEVICE_BIT_LSBFIRST = SPI_DEVICE_BIT_LSBFIRST,
    ENUM_SPI_DEVICE_3WIRE = SPI_DEVICE_3WIRE,
    ENUM_SPI_DEVICE_POSITIVE_CS = SPI_DEVICE_POSITIVE_CS,
    ENUM_SPI_DEVICE_HALFDUPLEX = SPI_DEVICE_HALFDUPLEX,
    ENUM_SPI_DEVICE_CLK_AS_CS = SPI_DEVICE_CLK_AS_CS,
};

enum {
    ENUM_SPI_TRANS_MODE_DIO = SPI_TRANS_MODE_DIO,
    ENUM_SPI_TRANS_MODE_QIO = SPI_TRANS_MODE_QIO,
    ENUM_SPI_TRANS_MODE_DIOQIO_ADDR = SPI_TRANS_MODE_DIOQIO_ADDR,
    ENUM_SPI_TRANS_USE_RXDATA = SPI_TRANS_USE_RXDATA,
    ENUM_SPI_TRANS_USE_TXDATA = SPI_TRANS_USE_TXDATA,
    ENUM_SPI_TRANS_VARIABLE_CMD = SPI_TRANS_VARIABLE_CMD,
    ENUM_SPI_TRANS_VARIABLE_ADDR = SPI_TRANS_VARIABLE_ADDR,
};

enum {
    ENUM_MALLOC_CAP_DMA = MALLOC_CAP_DMA,
    ENUM_MALLOC_CAP_INTERNAL = MALLOC_CAP_INTERNAL,
    ENUM_MALLOC_CAP_SPIRAM = MALLOC_CAP_SPIRAM,
};

/////////////////////////////////////////////////////////////////////////////////////////////
// ili9341 flush and ISR in C
//
// disp_drv->user_data should be a dict that contains dc and spi, setup by micropython.
// like this: "self.disp_drv.user_data = {'dc': self.dc, 'spi': self.spi}"


void ili9341_post_cb_isr(spi_transaction_t *trans);
void ili9341_flush(void *disp_drv, const void *area, void *color_p);

#endif

void ili9341_flush(void *disp_drv, const void *area, void *color_p);

