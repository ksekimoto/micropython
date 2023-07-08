/*
 * Copyright (c) 2022, Kentaro Sekimoto
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  -Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *  -Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "py/runtime.h"
#include "py/mphal.h"
// #include "mphalport.h"
#include "extmod/machine_spi.h"

// #if MICROPY_HW_ENABLE_LCDSPI

#include "ILI9340.h"
#include "font.h"
#include "jpeg.h"
#include "lcdspi_info.h"
#include "lcdspi.h"
// #include "common.h"

#if READ_LCD_ID
#define SPISW_READ_REGISTER   1
#else
#define SPISW_READ_REGISTER   0
#endif

#if defined(USE_DBG_PRINT)
#include "debug_printf.h"
#define DEBUG_LCDSPI
#endif

#ifdef HIGH
#undef HIGH
#endif
#define HIGH 1
#ifdef LOW
#undef LOW
#endif
#define LOW 0

static void lcdspi_spihw_set_pin_mode(void);
static void lcdspi_spisw_set_pin_mode(void);
static void lcdspi_addrset(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

#if defined(HW_LCDSPI_CH)
#define LCDSPI_CH HW_LCDSPI_CH
#else
#define LCDSPI_CH SW_LCDSPI_CH
#endif

#ifdef PIN_NONE
#undef PIN_NONE
#endif
#define PIN_NONE 0xffff

#ifndef LCDSPI_DEF_ID
#define LCDSPI_DEF_ID KMRTM24024SPI
#endif

#define LCDSPI_DEF_BAUD 4000000 // 4MHz

static bool m_spi_hw = true;
static bool m_reset_pin_used = false;

static lcdspi_gpio_output_t m_gpio_output;
static lcdspi_gpio_input_t m_gpio_input;
static lcdspi_gpio_write_t m_gpio_write;
static lcdspi_gpio_read_t m_gpio_read;
static lcdspi_spi_init_t m_spi_init;
static lcdspi_spi_transfer_t m_spi_transfer;

#if defined(RZA2M) || defined(RX63N) || defined(RX65N)
static const lcdspi_pins_t lcdspi_pins_def = {
    #if defined(HW_LCDSPI_CLK)
    (uint32_t)HW_LCDSPI_CLK->id,
    #else
    (uint32_t)PIN_NONE,
    #endif
    #if defined(HW_LCDSPI_MOSI)
    (uint32_t)HW_LCDSPI_MOSI->id,
    #else
    (uint32_t)PIN_NONE,
    #endif
    #if defined(HW_LCDSPI_MISO)
    (uint32_t)HW_LCDSPI_MISO->id,
    #else
    (uint32_t)PIN_NONE,
    #endif
    #if defined(HW_LCDSPI_CS)
    (uint32_t)HW_LCDSPI_CS->id,
    #else
    (uint32_t)PIN_NONE,
    #endif
    #if defined(HW_LCDSPI_RESET)
    (uint32_t)HW_LCDSPI_RESET->id,
    #else
    (uint32_t)PIN_NONE,
    #endif
    #if defined(HW_LCDSPI_RS)
    (uint32_t)HW_LCDSPI_RS->id,
    #else
    (uint32_t)PIN_NONE,
    #endif
    #if defined(HW_LCDSPI_BL)
    (uint32_t)HW_LCDSPI_BL->id,
    #else
    (uint32_t)PIN_NONE,
    #endif
};
#elif defined(PICO_BOARD)
static const lcdspi_pins_t lcdspi_pins_def = {
    #if defined(HW_LCDSPI_CLK)
    (uint32_t)HW_LCDSPI_CLK,
    #else
    (uint32_t)PIN_NONE,
    #endif
    #if defined(HW_LCDSPI_MOSI)
    (uint32_t)HW_LCDSPI_MOSI,
    #else
    (uint32_t)PIN_NONE,
    #endif
    #if defined(HW_LCDSPI_MISO)
    (uint32_t)HW_LCDSPI_MISO,
    #else
    (uint32_t)PIN_NONE,
    #endif
    #if defined(HW_LCDSPI_CS)
    (uint32_t)HW_LCDSPI_CS,
    #else
    (uint32_t)PIN_NONE,
    #endif
    #if defined(HW_LCDSPI_RESET)
    (uint32_t)HW_LCDSPI_RESET,
    #else
    (uint32_t)PIN_NONE,
    #endif
    #if defined(HW_LCDSPI_RS)
    (uint32_t)HW_LCDSPI_RS,
    #else
    (uint32_t)PIN_NONE,
    #endif
    #if defined(HW_LCDSPI_BL)
    (uint32_t)HW_LCDSPI_BL,
    #else
    (uint32_t)PIN_NONE,
    #endif
};
#else
#endif
static lcdspi_pins_t *m_lcdspi_pins;
static uint32_t m_lcdspi_ch = LCDSPI_CH;
static lcdspi_screen_t m_screen;

/* ********************************************************************* */
/* GPIO                                                                  */
/* ********************************************************************* */
static inline void lcdspi_gpio_set_input(uint32_t pin) {
    if (pin != PIN_NONE) {
        (*m_gpio_input)(pin);
    }
}

static inline void lcdspi_gpio_set_output(uint32_t pin) {
    if (pin != PIN_NONE) {
        (*m_gpio_output)(pin);
    }
}

static inline void lcdspi_gpio_write(uint32_t pin, bool level) {
    if (pin != PIN_NONE) {
        (*m_gpio_write)(pin, level);
    }
}

static inline bool lcdspi_gpio_read(uint32_t pin) {
    if (pin != PIN_NONE) {
        return (*m_gpio_read)(pin);
    } else {
        return false;
    }
}

void lcdspi_reset_high(void) {
    lcdspi_gpio_write(m_lcdspi_pins->pin_reset, true);
}

void lcdspi_reset_low(void) {
    lcdspi_gpio_write(m_lcdspi_pins->pin_reset, false);
}

void lcdspi_backlight_on(void) {
    if (m_lcdspi_pins->pin_bl != PIN_NONE) {
        lcdspi_gpio_set_output(m_lcdspi_pins->pin_bl);
        lcdspi_gpio_write(m_lcdspi_pins->pin_bl, true);
    }
}

void lcdspi_backlight_off(void) {
    if (m_lcdspi_pins->pin_bl != PIN_NONE) {
        lcdspi_gpio_set_output(m_lcdspi_pins->pin_bl);
        lcdspi_gpio_write(m_lcdspi_pins->pin_bl, false);
    }
}

#define MHZ_COUNT   30

static void delay_us(volatile uint32_t n) {
    while (n-- > 0) {
        for (int i = 0; i < MHZ_COUNT; i++) {
            __asm__ __volatile__ ("nop");
        }
    }
}

#if READ_LCD_ID
static void delay_ms(volatile uint32_t n) {
    // mp_hal_delay_ms(n);
    while (n-- > 0) {
        for (int i = 0; i < MHZ_COUNT * 1000; i++) {
            __asm__ __volatile__ ("nop");
        }
    }
}
#endif

/* ********************************************************************* */
/* SPI                                                                   */
/* ********************************************************************* */

static void lcdspi_spihw_transfer(uint8_t *dst, const uint8_t *src, uint32_t count) {
    m_spi_transfer((size_t)count, (const uint8_t *)src, (uint8_t *)dst);
    return;
}

static void lcdspi_spihw_write(uint8_t b) {
    uint8_t v;
    m_spi_transfer((size_t)1, (const uint8_t *)&b, (uint8_t *)&v);
    return;
}

static uint8_t lcdspi_spihw_xfer(uint8_t b) {
    uint8_t v;
    m_spi_transfer((size_t)1, (const uint8_t *)&b, (uint8_t *)&v);
    return v;
}

static void lcdspi_spihw_set_pin_mode(void) {
    // ToDo: set peripheral mode for spi pins
}

static void lcdspi_spisw_set_pin_mode(void) {
    // set gpio mode for spi pins
    if (m_lcdspi_pins->pin_clk != PIN_NONE) {
        lcdspi_gpio_set_output(m_lcdspi_pins->pin_clk);
    }
    if (m_lcdspi_pins->pin_dout != PIN_NONE) {
        lcdspi_gpio_set_output(m_lcdspi_pins->pin_dout);
    }
    if (m_lcdspi_pins->pin_din != PIN_NONE) {
        lcdspi_gpio_set_input(m_lcdspi_pins->pin_din);
    }
}

void lcdspi_spisw_init(void) {
    lcdspi_spisw_set_pin_mode();
}

static void lcdspi_set_pin_mode(void) {
    if (m_lcdspi_pins->pin_cs != PIN_NONE) {
        lcdspi_gpio_set_output(m_lcdspi_pins->pin_cs);
    }
    if (m_lcdspi_pins->pin_reset != PIN_NONE) {
        m_reset_pin_used = true;
        lcdspi_gpio_set_output(m_lcdspi_pins->pin_reset);
    }
    if (m_lcdspi_pins->pin_rs != PIN_NONE) {
        lcdspi_gpio_set_output(m_lcdspi_pins->pin_rs);
    }
}

static inline bool is_lcdspi_reset_pin_used(void) {
    return m_reset_pin_used;
}

void lcdspi_spisw_write(uint8_t dat) {
    uint8_t i = 8;
    while (i-- > 0) {
        lcdspi_gpio_write(m_lcdspi_pins->pin_dout, (dat & 0x80) ? 1 : 0);
        lcdspi_gpio_write(m_lcdspi_pins->pin_clk, LOW);
        dat = (uint8_t)(dat << 1);
        lcdspi_gpio_write(m_lcdspi_pins->pin_clk, HIGH);
    }
}

uint8_t lcdspi_spisw_read(bool miso) {
    uint8_t i = 8;
    uint8_t value = 0;
    while (i-- > 0) {
        lcdspi_gpio_write(m_lcdspi_pins->pin_clk, LOW);
        __asm__ __volatile__ ("nop");
        __asm__ __volatile__ ("nop");
        lcdspi_gpio_write(m_lcdspi_pins->pin_clk, HIGH);
        __asm__ __volatile__ ("nop");
        value <<= 1;
        if (!miso) {
            if (lcdspi_gpio_read(m_lcdspi_pins->pin_dout)) {
                value |= 1;
            }
        } else {
            if (lcdspi_gpio_read(m_lcdspi_pins->pin_din)) {
                value |= 1;
            }
        }
    }
    return value;
}

uint8_t lcdspi_spisw_xfer(uint8_t dat) {
    uint8_t i = 8;
    uint8_t value = 0;
    while (i-- > 0) {
        lcdspi_gpio_write(m_lcdspi_pins->pin_dout, (dat & 0x80) ? 1 : 0);
        lcdspi_gpio_write(m_lcdspi_pins->pin_clk, LOW);
        __asm__ __volatile__ ("nop");
        dat = (uint8_t)(dat << 1);
        lcdspi_gpio_write(m_lcdspi_pins->pin_clk, HIGH);
        __asm__ __volatile__ ("nop");
        value <<= 1;
        if (lcdspi_gpio_read(m_lcdspi_pins->pin_din)) {
            value |= 1;
        }
    }
    return value;
}

static void lcdspi_spi_write_sub(uint8_t dat, bool spi_hw) {
    if (spi_hw) {
        lcdspi_spihw_write(dat);
    } else {
        lcdspi_spisw_write(dat);
    }
}

void lcdspi_spi_write(uint8_t dat) {
    lcdspi_spi_write_sub(dat, m_spi_hw);
}

static uint8_t lcdspi_spi_xfer_sub(uint8_t dat, bool spi_hw) {
    uint8_t v = 0;
    if (m_spi_hw) {
        v = lcdspi_spihw_xfer(dat);
    } else {
        v = lcdspi_spisw_xfer(dat);
    }
    return v;
}

uint8_t lcdspi_spi_xfer(uint8_t dat) {
    return lcdspi_spi_xfer_sub(dat, m_spi_hw);
}

void lcdspi_spisw_transfer(uint8_t *dst, const uint8_t *src, uint32_t count) {
    while (count-- > 0) {
        *dst++ = lcdspi_spisw_xfer(*src++);
    }
}

static void lcdspi_spi_transfer_sub(uint8_t *dst, const uint8_t *src, uint32_t count, bool spi_hw) {
    if (m_spi_hw) {
        lcdspi_spihw_transfer((uint8_t *)dst, (const uint8_t *)src, (size_t)count);
    } else {
        lcdspi_spisw_transfer((uint8_t *)dst, (const uint8_t *)src, (size_t)count);
    }
    return;
}

void lcdspi_spi_transfer(uint8_t *dst, const uint8_t *src, uint32_t count) {
    lcdspi_spi_transfer_sub(dst, src, count, m_spi_hw);
}

void lcdspi_spi_write_cmd9(uint8_t cmd) {
    // Enter command mode: SDATA=LOW at rising edge of 1st SCLK
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, LOW);
    lcdspi_gpio_write(m_lcdspi_pins->pin_dout, LOW);
    lcdspi_gpio_write(m_lcdspi_pins->pin_clk, LOW);
    lcdspi_gpio_write(m_lcdspi_pins->pin_clk, HIGH);
    lcdspi_spisw_write(cmd);
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, HIGH);
}

void lcdspi_spi_write_dat9(uint8_t dat) {
    // Enter data mode: SDATA=HIGH at rising edge of 1st SCLK
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, LOW);
    lcdspi_gpio_write(m_lcdspi_pins->pin_dout, HIGH);
    lcdspi_gpio_write(m_lcdspi_pins->pin_clk, LOW);
    lcdspi_gpio_write(m_lcdspi_pins->pin_clk, HIGH);
    lcdspi_spisw_write(dat);
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, HIGH);
}

void lcdspi_spi_write_cmd8(uint8_t cmd) {
    // Enter command mode: RS=LOW at rising edge of 1st SCLK
    lcdspi_gpio_write(m_lcdspi_pins->pin_rs, LOW);
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, LOW);
    lcdspi_spi_write(cmd);
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, HIGH);
}

void lcdspi_spi_write_dat8(uint8_t dat) {
    // Enter data mode: RS=HIGH at rising edge of 1st SCLK
    lcdspi_gpio_write(m_lcdspi_pins->pin_rs, HIGH);
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, LOW);
    lcdspi_spi_write(dat);
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, HIGH);
}

void lcdspi_spi_write_dat8_2(uint16_t dat) {
    // Enter data mode: RS=HIGH at rising edge of 1st SCLK
    lcdspi_spi_write_dat8((uint8_t)((dat >> 8) & 0xff));
    lcdspi_spi_write_dat8((uint8_t)(dat & 0xff));
}

void lcdspi_spi_write_dat16(uint16_t dat) {
    // Enter data mode: RS=HIGH at rising edge of 1st SCLK
    lcdspi_gpio_write(m_lcdspi_pins->pin_rs, HIGH);
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, LOW);
    lcdspi_spi_write((uint8_t)((dat >> 8) & 0xff));
    lcdspi_spi_write((uint8_t)(dat & 0xff));
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, HIGH);
}

#define SPISW_READ_REGISTER   0
#if SPISW_READ_REGISTER
static void SPISW_write_cmd8(uint8_t dat) {
    // Enter command mode: RS=LOW at rising edge of 1st SCLK
    lcdspi_gpio_write(m_lcdspi_pins->pin_rs, LOW);
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, LOW);
    lcdspi_spisw_write(dat);
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, HIGH);
}

static void SPISW_write_dat8(uint8_t dat) {
    // Enter data mode: RS=HIGH at rising edge of 1st SCLK
    lcdspi_gpio_write(m_lcdspi_pins->pin_rs, HIGH);
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, LOW);
    lcdspi_spisw_write(dat);
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, HIGH);
}

static void SPIHW_write_cmd8(uint8_t dat) {
    // Enter command mode: RS=LOW at rising edge of 1st SCLK
    lcdspi_gpio_write(m_lcdspi_pins->pin_rs, LOW);
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, LOW);
    lcdspi_spihw_write(dat);
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, HIGH);
}

static void SPIHW_write_dat8(uint8_t dat) {
    // Enter data mode: RS=HIGH at rising edge of 1st SCLK
    lcdspi_gpio_write(m_lcdspi_pins->pin_rs, HIGH);
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, LOW);
    lcdspi_spihw_write(dat);
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, HIGH);
}
#endif

void lcdspi_spi_write_dat16_n(uint32_t size, uint16_t v) {
    // Enter data mode: RS=HIGH at rising edge of 1st SCLK
#define DTC_UNIT_SIZE0  (0x100)
    uint8_t src[DTC_UNIT_SIZE0];
    uint8_t dst[DTC_UNIT_SIZE0];
    for (int i = 0; i < DTC_UNIT_SIZE0; i += 2) {
        src[i] = (uint8_t)((v >> 8) & 0xff);
        src[i + 1] = (uint8_t)(v & 0xff);
    }
    lcdspi_gpio_write(m_lcdspi_pins->pin_rs, HIGH);
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, LOW);
    while (size > 0) {
        uint32_t count = DTC_UNIT_SIZE0;
        if (size < DTC_UNIT_SIZE0) {
            count = size;
        }
        lcdspi_spi_transfer((uint8_t *)dst, (uint8_t *)src, count);
        size -= count;
    }
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, HIGH);
}

void lcdspi_spi_write_buf_n(uint8_t *buf, uint32_t size) {
    // Enter data mode: RS=HIGH at rising edge of 1st SCLK
#define DTC_UNIT_SIZE   (0x100)
    uint8_t dst[DTC_UNIT_SIZE];
    memset((void *)dst, 0, (size_t)DTC_UNIT_SIZE);
    lcdspi_gpio_write(m_lcdspi_pins->pin_rs, HIGH);
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, LOW);
    while (size > 0) {
        uint32_t count = DTC_UNIT_SIZE;
        if (size < DTC_UNIT_SIZE) {
            count = size;
        }
        lcdspi_spi_transfer((uint8_t *)dst, (uint8_t *)buf, count);
        buf += count;
        size -= count;
    }
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, HIGH);
}

static uint8_t lcdspi_spisw_dat_read(bool miso) {
    uint8_t val = 0;
    for (int i = 0; i < 8; i++) {  // read results
        __asm__ __volatile__ ("nop");
        __asm__ __volatile__ ("nop");
        lcdspi_gpio_write(m_lcdspi_pins->pin_clk, HIGH);
        __asm__ __volatile__ ("nop");
        __asm__ __volatile__ ("nop");
        val <<= 1;
        if (!miso) {
            if (lcdspi_gpio_read(m_lcdspi_pins->pin_dout)) {
                val |= 1;
            }
        } else {
            if (lcdspi_gpio_read(m_lcdspi_pins->pin_din)) {
                val |= 1;
            }
        }
        lcdspi_gpio_write(m_lcdspi_pins->pin_clk, LOW);
    }
    return val;
}

static uint32_t lcdspi_spisw_cmd_writeread_sub(uint8_t cmd, uint8_t bits, uint8_t dummy, bool miso) {
    uint32_t ret = 0;
    uint8_t val = cmd;
    uint8_t count = bits / 8;
    lcdspi_spisw_set_pin_mode();
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, LOW);
    lcdspi_gpio_write(m_lcdspi_pins->pin_rs, LOW);
    for (int i = 0; i < 8; i++) {   // send command
        lcdspi_gpio_write(m_lcdspi_pins->pin_dout, (val & 0x80) != 0);
        lcdspi_gpio_write(m_lcdspi_pins->pin_clk, HIGH);
        val <<= 1;
        lcdspi_gpio_write(m_lcdspi_pins->pin_clk, LOW);
    }
    lcdspi_gpio_write(m_lcdspi_pins->pin_rs, HIGH);
    if (bits == 0) {
        lcdspi_gpio_write(m_lcdspi_pins->pin_cs, HIGH);
        return 0;
    }
    delay_us(1);
    if (!miso) {
        lcdspi_gpio_set_input(m_lcdspi_pins->pin_dout);
    } else {
        lcdspi_gpio_set_input(m_lcdspi_pins->pin_din);
        lcdspi_gpio_write(m_lcdspi_pins->pin_dout, LOW);
    }
    for (int i = 0; i < dummy; i++) {  // any dummy clocks
        lcdspi_gpio_write(m_lcdspi_pins->pin_clk, HIGH);
        lcdspi_gpio_write(m_lcdspi_pins->pin_clk, LOW);
    }
    while (count-- > 0) {
        ret <<= 8;
        val = lcdspi_spisw_dat_read(miso);
        ret |= (uint32_t)val;
    }
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, HIGH);
    if (!miso) {
        lcdspi_gpio_set_output(m_lcdspi_pins->pin_dout);
    }
    return ret;
}

uint8_t lcdspi_spisw_read_reg(uint8_t addr, uint8_t idx) {
    return (uint8_t)lcdspi_spisw_cmd_writeread_sub(addr, 8 * (idx + 1), 0, false);
}

uint8_t ILI93xx_spisw_read_reg(uint8_t addr, uint8_t idx) {
    uint8_t val;
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, LOW);
    lcdspi_gpio_write(m_lcdspi_pins->pin_rs, LOW);
    lcdspi_spisw_write(0xd9);
    delay_us(1);
    lcdspi_gpio_write(m_lcdspi_pins->pin_rs, HIGH);
    lcdspi_spisw_write(0x10 + idx);
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, HIGH);
    val = (uint8_t)lcdspi_spisw_cmd_writeread_sub(addr, 8, 0, true);
    return val;
}

static uint8_t lcdspi_spihw_cmd_writeread_sub(uint8_t cmd, uint8_t bits, uint8_t dummy) {
    uint32_t count = bits / 8;
    uint8_t src[4] = {0};
    uint8_t dst = 0;
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, LOW);
    lcdspi_gpio_write(m_lcdspi_pins->pin_rs, LOW);
    lcdspi_spihw_write(cmd);
    lcdspi_gpio_write(m_lcdspi_pins->pin_rs, HIGH);
    if (bits == 0) {
        lcdspi_gpio_write(m_lcdspi_pins->pin_cs, HIGH);
        return 0;
    }
    delay_us(1);
    for (int i = 0; i < dummy; i++) {  // any dummy clocks
        lcdspi_gpio_write(m_lcdspi_pins->pin_clk, HIGH);
        lcdspi_gpio_write(m_lcdspi_pins->pin_clk, LOW);
    }
    int i = 0;
    while (count-- > 0) {
        dst = lcdspi_spihw_xfer(src[i++]);
    }
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, HIGH);
    return dst;
}

uint8_t lcdspi_spihw_read_reg(uint8_t addr, uint8_t idx) {
    return (uint8_t)lcdspi_spihw_cmd_writeread_sub(addr, 8 * (idx + 1), 0);
}

uint8_t ILI93xx_spihw_read_reg(uint8_t addr, uint8_t idx) {
    uint8_t t = 0;
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, LOW);
    lcdspi_gpio_write(m_lcdspi_pins->pin_rs, LOW);
    lcdspi_spihw_write(0xd9);
    delay_us(1);
    lcdspi_gpio_write(m_lcdspi_pins->pin_rs, HIGH);
    lcdspi_spihw_write(0x10 + idx);
    lcdspi_gpio_write(m_lcdspi_pins->pin_cs, HIGH);
    t = (uint8_t)lcdspi_spihw_cmd_writeread_sub(addr, 8, 0);
    return t;
}

#if READ_LCD_ID
static uint32_t ILI93xx_spisw_read_reg_n(uint8_t addr, uint8_t num) {
    uint32_t val = 0;
    uint8_t t = 0;
    for (uint8_t i = 0; i < num; i++) {
        t = ILI93xx_spisw_read_reg(addr, i);
        val <<= 8;
        val |= (uint32_t)t;
    }
    return val;
}

static uint32_t ILI93xx_spihw_read_reg_n(uint8_t addr, uint8_t num) {
    uint32_t val = 0;
    uint8_t t = 0;
    for (uint8_t i = 0; i < num; i++) {
        t = ILI93xx_spihw_read_reg(addr, i);
        val <<= 8;
        val |= (uint32_t)t;
    }
    return val;
}
#endif

/* ********************************************************************* */
/* LCDSPI                                                                */
/* ********************************************************************* */

uint8_t get_rotate_param(lcdspi_t *lcdspi, uint8_t dir) {
    uint8_t d = 0;
    switch (lcdspi->lcd->lcd_info_id) {
        case ST7735R_G128x160:
        case ST7735R_R128x160:
        case ST7735R_G128x128:
        case ST7735R_G160x80:
        case KMRTM24024SPI:
            switch (dir) {
                case LCDSPI_ROTATE_90:
                    d = DDD_X_Y_EX;
                    break;
                case LCDSPI_ROTATE_180:
                    d = DDD_Y_MIRROR;
                    break;
                case LCDSPI_ROTATE_270:
                    d = DDD_X_Y_EX_X_Y_MIRROR;
                    break;
                default:
                    d = DDD_X_MIRROR;
                    break;
            }
            break;
        default:
            switch (dir) {
                case LCDSPI_ROTATE_90:
                    d = DDD_X_Y_EX_X_MIRROR;
                    break;
                case LCDSPI_ROTATE_180:
                    d = DDD_X_Y_MIRROR;
                    break;
                case LCDSPI_ROTATE_270:
                    d = DDD_X_Y_EX_Y_MIRROR;
                    break;
                default:
                    d = DDD_NORMAL;
                    break;
            }
    }
    return d;
}

void lcdspi_set_screen_dir(lcdspi_t *lcdspi, uint8_t dir) {
    lcdspi->screen_dir = dir;
    if (dir == LCDSPI_ROTATE_0) {
        lcdspi->screen->hw_scroll = true;
    } else {
        lcdspi->screen->hw_scroll = false;
    }
}

void lcdspi_set_spi_ch(lcdspi_t *lcdspi, uint32_t spi_ch) {
    lcdspi->spi_ch = spi_ch;
    m_lcdspi_ch = spi_ch;
}

void lcdspi_set_pins(lcdspi_t *lcdspi, lcdspi_pins_t *pins) {
    if (pins == NULL) {
        lcdspi->pins = (lcdspi_pins_t *)&lcdspi_pins_def;
    } else {
        lcdspi->pins = pins;
    }
    m_lcdspi_pins = lcdspi->pins;
}

void lcdspi_set_lcd(lcdspi_t *lcdspi, uint32_t lcd_id) {
    if (lcd_id < lcdspi_info_size) {
        lcdspi->lcd = (lcdspi_lcd_t *)lcdspi_info[lcd_id];
    } else {
        lcdspi->lcd = (lcdspi_lcd_t *)lcdspi_info[LCDSPI_DEF_ID];
    }
    lcdspi->baud = lcdspi->lcd->def_baud;
}

void lcdspi_screen_init(lcdspi_screen_t *screen) {
    screen->cx = 0;
    screen->cy = 0;
    screen->dy = 0;
    screen->fcol = (uint16_t)0xFFFFFF;
    screen->bcol = (uint16_t)0x000000;
    screen->unit_wx = 4;
    screen->unit_wy = 8;
}

void lcdspi_set_screen(lcdspi_t *lcdspi, lcdspi_screen_t *screen) {
    if (screen == NULL) {
        lcdspi->screen = &m_screen;
        lcdspi_screen_init(lcdspi->screen);
    } else {
        lcdspi->screen = screen;
    }
}

#if READ_LCD_ID
static void lcdspi_spisw_swreset(lcdspi_t *lcdspi) {
    lcdspi_spisw_cmd_writeread_sub(0x01, 0, 0, false);
    delay_ms(150);
}

static uint32_t spisw_id_mosi(lcdspi_t *lcdspi, uint8_t cmd, uint8_t bit, uint8_t dummy) {
    return lcdspi_spisw_cmd_writeread_sub(cmd, bit, dummy, false);
}

static uint32_t spisw_id_miso(lcdspi_t *lcdspi, uint8_t cmd, uint8_t bit, uint8_t dummy) {
    return lcdspi_spisw_cmd_writeread_sub(cmd, bit, dummy, true);
}

static void ILI93xx_spisw_read_ids(lcdspi_t *lcdspi) {
    lcdspi->did1 = spisw_id_mosi(lcdspi, 0x04, 24, 1);
    lcdspi->id1_1 = (uint8_t)spisw_id_mosi(lcdspi, 0xda, 8, 0);
    lcdspi->id2_1 = (uint8_t)spisw_id_mosi(lcdspi, 0xdb, 8, 0);
    lcdspi->id3_1 = (uint8_t)spisw_id_mosi(lcdspi, 0xdc, 8, 0);
    lcdspi->did2 = spisw_id_miso(lcdspi, 0x04, 24, 1);
    lcdspi->id1_2 = (uint8_t)ILI93xx_spisw_read_reg(0xda, 1);
    lcdspi->id2_2 = (uint8_t)ILI93xx_spisw_read_reg(0xdb, 1);
    lcdspi->id3_2 = (uint8_t)ILI93xx_spisw_read_reg(0xdc, 1);
    lcdspi->ili93xx_id_spisw = (uint32_t)ILI93xx_spisw_read_reg_n(0xd3, 4);
}

static void ILI93xx_spihw_read_ids(lcdspi_t *lcdspi) {
    lcdspi->id1_3 = (uint8_t)ILI93xx_spihw_read_reg(0xda, 1);
    lcdspi->id2_3 = (uint8_t)ILI93xx_spihw_read_reg(0xdb, 1);
    lcdspi->id3_3 = (uint8_t)ILI93xx_spihw_read_reg(0xdc, 1);
    lcdspi->ili93xx_id_spisw = (uint32_t)ILI93xx_spihw_read_reg_n(0xd3, 4);
}
#endif

void lcdspi_spi_init(lcdspi_t *lcdspi, bool spi_hw_mode) {
    if (spi_hw_mode) {
        m_spi_hw = true;
        lcdspi_spihw_set_pin_mode();
        #if defined(RZA2M)
        m_spi_init();
        #elif defined(RX63N) || defined(RX65N)
        m_spi_init();
        #elif defined(PICO_BOARD)
        m_spi_init(
            (uint32_t)lcdspi->spi_ch,
            (uint32_t)lcdspi->pins->pin_dout,
            (uint32_t)lcdspi->pins->pin_din,
            (uint32_t)lcdspi->pins->pin_clk,
            (uint32_t)lcdspi->pins->pin_cs,
            (uint32_t)lcdspi->baud,
            (uint32_t)8,
            (uint32_t)lcdspi->lcd->mode);
        #else
        #endif
    } else {
        m_spi_hw = false;
        lcdspi_spisw_init();
    }
}

static void lcdspi_spi_start_xfer(lcdspi_t *lcdspi) {
    if (!m_spi_hw) {
        lcdspi_spisw_set_pin_mode();
    }
}

static void lcdspi_spi_end_xfer(lcdspi_t *lcdspi) {
    if (!m_spi_hw) {
        lcdspi_spihw_set_pin_mode();
    }
}

/*
 * common functions
 */

static void lcdspi_addrset(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    lcdspi_spi_write_cmd8(ILI9340_CASET);
    lcdspi_spi_write_dat8_2(x1);
    lcdspi_spi_write_dat8_2(x2);
    lcdspi_spi_write_cmd8(ILI9340_PASET);
    lcdspi_spi_write_dat8_2(y1);
    lcdspi_spi_write_dat8_2(y2);
    lcdspi_spi_write_cmd8(ILI9340_RAMWR);
}

static void lcdspi_addrset_SSD1331(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    lcdspi_spi_write_cmd8(0x15);
    lcdspi_spi_write_cmd8(x1);
    lcdspi_spi_write_cmd8(x2);
    lcdspi_spi_write_cmd8(0x75);
    lcdspi_spi_write_cmd8(y1);
    lcdspi_spi_write_cmd8(y2);
}

static void swap_uint16(uint16_t *i, uint16_t *j) {
    uint16_t t;
    t = *j;
    *j = *i;
    *i = t;
}

static void swap_uint32(uint32_t *i, uint32_t *j) {
    uint32_t t;
    t = *j;
    *j = *i;
    *i = t;
}

static void update_dir(lcdspi_t *lcdspi) {
    uint8_t dir = get_rotate_param(lcdspi, lcdspi->screen_dir);
    uint8_t madctl = (lcdspi->lcd->madctl & 0x1f) | (dir << 5);
    uint32_t lcd_ctrl_id = (uint8_t)lcdspi->lcd->ctrl_info->id;
    if (lcd_ctrl_id == SSD1331) {
    } else if (lcd_ctrl_id == PCF8833 || lcd_ctrl_id == S1D15G10) {
        lcdspi_spi_write_cmd9(0x36);
        lcdspi_spi_write_dat9(madctl);
    } else {
        lcdspi_spi_write_cmd8(0x36);
        lcdspi_spi_write_dat8(madctl);
    }
}

static void update_axis16(lcdspi_t *lcdspi, uint16_t *x, uint16_t *y) {
    if ((lcdspi->screen_dir == LCDSPI_ROTATE_90) || (lcdspi->screen_dir == LCDSPI_ROTATE_270)) {
        swap_uint16(x, y);
    }
    #if 0
    switch (lcdspi->screen_dir) {
        // virtical
        case DDD_X_MIRROR:      // position 0
        case DDD_Y_MIRROR:      // position 2
        case DDD_NORMAL:
        case DDD_X_Y_MIRROR:
            break;
        // horizontal
        case DDD_X_Y_EX:        // position 1
        case DDD_X_Y_EX_X_Y_MIRROR: // position 3
        case DDD_X_Y_EX_Y_MIRROR:
        case DDD_X_Y_EX_X_MIRROR:
            swap_uint16(x, y);
            break;
    }
    #endif
}

#if 0
static void update_axis32(lcdspi_t *lcdspi, uint32_t *x, uint32_t *y) {
    if ((lcdspi->screen_dir == LCDSPI_ROTATE_90) || (lcdspi->screen_dir == LCDSPI_ROTATE_270)) {
        swap_uint32(x, y);
    }
    #if 0
    switch (lcdspi->screen_dir) {
        // virtical
        case DDD_X_MIRROR:      // position 0
        case DDD_Y_MIRROR:      // position 2
        case DDD_NORMAL:
        case DDD_X_Y_MIRROR:
            break;
        // horizontal
        case DDD_X_Y_EX:        // position 1
        case DDD_X_Y_EX_X_Y_MIRROR: // position 3
        case DDD_X_Y_EX_Y_MIRROR:
        case DDD_X_Y_EX_X_MIRROR:
            swap_uint32(x, y);
            break;
    }
    #endif
}
#endif

void lcdspi_box_fill(lcdspi_t *lcdspi, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint16_t col) {
    uint8_t PASET = lcdspi->lcd->ctrl_info->PASET;
    uint8_t CASET = lcdspi->lcd->ctrl_info->CASET;
    uint8_t RAMWR = lcdspi->lcd->ctrl_info->RAMWR;
    uint16_t width = lcdspi->lcd->width;
    uint16_t height = lcdspi->lcd->height;
    uint16_t sx = lcdspi->lcd->sx;
    uint16_t sy = lcdspi->lcd->sy;
    uint32_t lcd_ctrl_id = lcdspi->lcd->ctrl_info->id;
    update_dir(lcdspi);
    update_axis16(lcdspi, &sx, &sy);
    update_axis16(lcdspi, &width, &height);
    if (x1 > x2) {
        swap_uint32(&x1, &x2);
    }
    if (y1 > y2) {
        swap_uint32(&y1, &y2);
    }
    lcdspi_spi_start_xfer(lcdspi);
    if (lcd_ctrl_id == PCF8833 || lcd_ctrl_id == S1D15G10) {
        uint8_t v1 = (uint8_t)col & 0xff;
        uint8_t v2 = (uint8_t)((col >> 8) & 0xff);
        uint8_t b1 = R4G4(v1);
        uint8_t b2 = B4R4(v1, v2);
        uint8_t b3 = G4B4(v2);
        lcdspi_spi_write_cmd9(PASET);
        lcdspi_spi_write_dat9((uint8_t)(sx + x1));
        lcdspi_spi_write_dat9((uint8_t)(sx + x2));
        lcdspi_spi_write_cmd9(CASET);
        lcdspi_spi_write_dat9((uint8_t)(sy + y1));
        lcdspi_spi_write_dat9((uint8_t)(sy + y2));
        lcdspi_spi_write_cmd9(RAMWR);
        for (int x = 0; x < ((width * height) / 2); x++) {
            lcdspi_spi_write_dat9(b1);
            lcdspi_spi_write_dat9(b2);
            lcdspi_spi_write_dat9(b3);
        }
    } else if (lcd_ctrl_id == SSD1331) {
        lcdspi_addrset_SSD1331((uint16_t)(sx + x1), (uint16_t)(sy + y1), (uint16_t)(sx + x2), (uint16_t)(sy + y2));
        lcdspi_spi_write_dat16_n(width * height * 2, col);
    } else {
        lcdspi_addrset((uint16_t)(sx + x1), (uint16_t)(sy + y1), (uint16_t)(sx + x2), (uint16_t)(sy + y2));
        lcdspi_spi_write_dat16_n(width * height * 2, col);
    }
    lcdspi_spi_end_xfer(lcdspi);
    return;
}


void lcdspi_scroll(lcdspi_t *lcdspi, uint32_t dy) {
    uint32_t lcd_ctrl_id = lcdspi->lcd->ctrl_info->id;
    if (lcd_ctrl_id == ILI9341 || lcd_ctrl_id == ST7735) {
        lcdspi_spi_write_cmd8(0x37);
        lcdspi_spi_write_dat8((uint8_t)(dy >> 8));
        lcdspi_spi_write_dat8((uint8_t)(dy & 0xff));
    }
}

void lcdspi_scroll_range(lcdspi_t *lcdspi, uint32_t sy, uint32_t wy, uint32_t ey) {
    #if 0
    (void)lcdspi;
    (void)sy;
    (void)wy;
    (void)ey;
    #else
    uint32_t lcd_ctrl_id = lcdspi->lcd->ctrl_info->id;
    if (lcd_ctrl_id == ILI9341 || lcd_ctrl_id == ST7735) {
        lcdspi_spi_write_cmd8(0x33);
        lcdspi_spi_write_dat8((uint8_t)(sy >> 8));
        lcdspi_spi_write_dat8((uint8_t)(sy & 0xff));
        lcdspi_spi_write_dat8((uint8_t)(wy >> 8));
        lcdspi_spi_write_dat8((uint8_t)(wy & 0xff));
        lcdspi_spi_write_dat8((uint8_t)(ey >> 8));
        lcdspi_spi_write_dat8((uint8_t)(ey & 0xff));
    }
    #endif
}

void lcdspi_clear(lcdspi_t *lcdspi, uint16_t col) {
    uint8_t PASET = lcdspi->lcd->ctrl_info->PASET;
    uint8_t CASET = lcdspi->lcd->ctrl_info->CASET;
    uint8_t RAMWR = lcdspi->lcd->ctrl_info->RAMWR;
    uint16_t width = lcdspi->lcd->width;
    uint16_t height = lcdspi->lcd->height;
    uint16_t sx = lcdspi->lcd->sx;
    uint16_t ex = lcdspi->lcd->ex;
    uint16_t sy = lcdspi->lcd->sy;
    uint16_t ey = lcdspi->lcd->ey;
    uint32_t lcd_ctrl_id = lcdspi->lcd->ctrl_info->id;
    update_dir(lcdspi);
    update_axis16(lcdspi, &sx, &sy);
    update_axis16(lcdspi, &ex, &ey);
    lcdspi_spi_start_xfer(lcdspi);
    if (lcd_ctrl_id == PCF8833 || lcd_ctrl_id == S1D15G10) {
        uint8_t v1 = (uint8_t)col & 0xff;
        uint8_t v2 = (uint8_t)((col >> 8) & 0xff);
        uint8_t b1 = R4G4(v1);
        uint8_t b2 = B4R4(v1, v2);
        uint8_t b3 = G4B4(v2);
        lcdspi_spi_write_cmd9(PASET);
        lcdspi_spi_write_dat9((uint8_t)sx);
        lcdspi_spi_write_dat9((uint8_t)ex);
        lcdspi_spi_write_cmd9(CASET);
        lcdspi_spi_write_dat9((uint8_t)sy);
        lcdspi_spi_write_dat9((uint8_t)ey);
        lcdspi_spi_write_cmd9(RAMWR);
        for (int x = 0; x < (width * height / 2); x++) {
            lcdspi_spi_write_dat9(b1);
            lcdspi_spi_write_dat9(b2);
            lcdspi_spi_write_dat9(b3);
        }
    } else if (lcd_ctrl_id == SSD1331) {
        lcdspi_addrset_SSD1331((uint16_t)sx, (uint16_t)sy, (uint16_t)ex, (uint16_t)ey);
        lcdspi_spi_write_dat16_n(width * height * 2, col);
    } else {
        // if (lcd_ctrl_id == ILI9340 || lcd_ctrl_id == ST7735 || lcd_ctrl_id == ST7789) {
        lcdspi_addrset((uint16_t)sx, (uint16_t)sy, (uint16_t)ex, (uint16_t)ey);
        lcdspi_spi_write_dat16_n(width * height * 2, col);
    }
    lcdspi_spi_end_xfer(lcdspi);
    lcdspi->screen->cx = 0;
    lcdspi->screen->cy = 0;
    lcdspi->screen->dy = 0;
    if (lcdspi->screen_dir == LCDSPI_ROTATE_0) {
        lcdspi->screen->hw_scroll = true;
    } else {
        lcdspi->screen->hw_scroll = false;
    }
    return;
}

void lcdspi_init(lcdspi_t *lcdspi, lcdspi_screen_t *screen, lcdspi_pins_t *pins, uint32_t lcd_id, uint32_t spi_ch) {
    // if (lcdspi == NULL) {
    //     lcdspi = &m_lcdspi;
    // }

    m_gpio_output = lcdspi->gpio_output;
    m_gpio_input = lcdspi->gpio_input;
    m_gpio_write = lcdspi->gpio_write;
    m_gpio_read = lcdspi->gpio_read;
    m_spi_init = lcdspi->spi_init;
    m_spi_transfer = lcdspi->spi_transfer;

    lcdspi_set_lcd(lcdspi, lcd_id);
    lcdspi_set_spi_ch(lcdspi, spi_ch);
    lcdspi_set_screen(lcdspi, screen);
    lcdspi_set_pins(lcdspi, pins);
    lcdspi_set_pin_mode();    // after m_lcdspi_pins is configured
    if (is_lcdspi_reset_pin_used()) {
        lcdspi->lcd->lcdspi_reset();
    }
    #if READ_LCD_ID
    lcdspi_spi_init(lcdspi, false);
    lcdspi_spisw_swreset(lcdspi);
    ILI93xx_spisw_read_ids(lcdspi);
    #endif
    lcdspi_spi_init(lcdspi, (m_lcdspi_ch != SW_LCDSPI_CH));
    #if READ_LCD_ID
    if (m_spi_hw) {
        ILI93xx_spihw_read_ids(lcdspi);
    }
    #endif
    lcdspi_backlight_on();
    lcdspi->lcd->lcdspi_init();
    lcdspi_clear(lcdspi, 0);
    if (lcdspi->screen->hw_scroll) {
        uint16_t uy = (uint16_t)font_fontUnitY(lcdspi->screen->font);
        uint16_t wy = (lcdspi->lcd->ey - lcdspi->lcd->sy + 1) / uy * uy;
        lcdspi_scroll_range(lcdspi,
            (uint32_t)lcdspi->lcd->sy,
            (uint32_t)wy,
            (uint32_t)(lcdspi->lcd->ey - lcdspi->lcd->sy + 1 - wy));

    }
}

void lcdspi_deinit(lcdspi_t *lcdspi) {
    if (m_spi_hw) {
        // ToDo
        // delete pspi;
    }
}

void lcdspi_bitbltex565(lcdspi_t *lcdspi, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint16_t *data) {
    uint8_t PASET = lcdspi->lcd->ctrl_info->PASET;  // @suppress("Type cannot be resolved")
    uint8_t CASET = lcdspi->lcd->ctrl_info->CASET;
    uint8_t RAMWR = lcdspi->lcd->ctrl_info->RAMWR;
    uint32_t lcd_ctrl_id = lcdspi->lcd->ctrl_info->id;
    uint16_t sx = lcdspi->lcd->sx;
    uint16_t sy = lcdspi->lcd->sy;
    uint16_t width = (uint16_t)w;
    uint16_t height = (uint16_t)h;
    uint16_t i, j;
    uint16_t v1, v2;
    uint16_t *pdata = (uint16_t *)data;
    update_dir(lcdspi);
    update_axis16(lcdspi, &sx, &sy);
    lcdspi_spi_start_xfer(lcdspi);
    if (lcd_ctrl_id == PCF8833 || lcd_ctrl_id == S1D15G10) {
        for (j = 0; j < height; j++) {
            lcdspi_spi_write_cmd9(PASET);
            lcdspi_spi_write_dat9((uint8_t)(sy + y + j));
            lcdspi_spi_write_dat9((uint8_t)(sy + y + j + 1));
            for (i = 0; i < width; i += 2) {
                lcdspi_spi_write_cmd9(CASET);
                lcdspi_spi_write_dat9((uint8_t)(sx + x + i));
                lcdspi_spi_write_dat9((uint8_t)(sx + x + i + 1));
                v1 = *pdata++;
                v2 = *pdata++;
                lcdspi_spi_write_cmd9(RAMWR);
                lcdspi_spi_write_dat9(R4G4(v1));
                lcdspi_spi_write_dat9(B4R4(v1, v2));
                lcdspi_spi_write_dat9(G4B4(v2));
            }
        }
    } else if (lcd_ctrl_id == SSD1331) {
        lcdspi_addrset_SSD1331((uint16_t)(sx + x), (uint16_t)(sy + y), (uint16_t)(sx + x + width - 1), (uint16_t)(sy + y + height - 1));
        lcdspi_spi_write_buf_n((uint8_t *)pdata, width * height * 2);
    } else {
        lcdspi_addrset((uint16_t)(sx + x), (uint16_t)(sy + y), (uint16_t)(sx + x + width - 1), (uint16_t)(sy + y + height - 1));
        lcdspi_spi_write_buf_n((uint8_t *)pdata, width * height * 2);
    }
    lcdspi_spi_end_xfer(lcdspi);
}

void lcdspi_bitbltex(lcdspi_t *lcdspi, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint16_t *data) {
    uint32_t ex = lcdspi->lcd->ex;
    uint16_t *pdata = (uint16_t *)data;
    pdata += (y * ex + x);
    lcdspi_bitbltex565(lcdspi, x, y, width, height, pdata);
}

void lcdspi_pset(lcdspi_t *lcdspi, uint32_t x, uint32_t y, uint16_t col) {
    uint8_t PASET = lcdspi->lcd->ctrl_info->PASET;
    uint8_t CASET = lcdspi->lcd->ctrl_info->CASET;
    uint8_t RAMWR = lcdspi->lcd->ctrl_info->RAMWR;
    uint32_t lcd_ctrl_id = lcdspi->lcd->ctrl_info->id;
    uint16_t sx = lcdspi->lcd->sx;
    uint16_t sy = lcdspi->lcd->sy;
    uint16_t v1, v2;
    update_dir(lcdspi);
    update_axis16(lcdspi, &sx, &sy);
    lcdspi_spi_start_xfer(lcdspi);
    if (lcd_ctrl_id == PCF8833 || lcd_ctrl_id == S1D15G10) {
        lcdspi_spi_write_cmd9(PASET);
        lcdspi_spi_write_dat9((uint8_t)(sy + y));
        lcdspi_spi_write_dat9((uint8_t)(sy + y + 1));
        lcdspi_spi_write_cmd9(CASET);
        lcdspi_spi_write_dat9((uint8_t)(sx + x));
        lcdspi_spi_write_dat9((uint8_t)(sx + x + 1));
        v1 = (uint8_t)(col & 0xff);
        v2 = (uint8_t)((col >> 8) & 0xff);
        lcdspi_spi_write_cmd9(RAMWR);
        lcdspi_spi_write_dat9(R4G4(v1));
        lcdspi_spi_write_dat9(B4R4(v1, v2));
        lcdspi_spi_write_dat9(G4B4(v2));
    } else if (lcd_ctrl_id == SSD1331) {
        lcdspi_addrset_SSD1331((uint16_t)(sx + x), (uint16_t)(sy + y), (uint16_t)(sx + x + 1), (uint16_t)(sy + y + 1));
        lcdspi_spi_write_dat16(col);
    } else {
        lcdspi_addrset((uint16_t)(sx + x), (uint16_t)(sy + y), (uint16_t)(sx + x + 1), (uint16_t)(sy + y + 1));
        if (lcd_ctrl_id == ILI9488) {
            lcdspi_spi_write_dat16(col);
        } else {
            lcdspi_spi_write_dat8((uint8_t)(col >> 8));
            lcdspi_spi_write_dat8((uint8_t)col);
        }
    }
    lcdspi_spi_end_xfer(lcdspi);
}

static void lcdspi_simple_line(lcdspi_t *lcdspi, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint16_t col) {
    uint32_t z1, z2;
    uint8_t PASET = lcdspi->lcd->ctrl_info->PASET;
    uint8_t CASET = lcdspi->lcd->ctrl_info->CASET;
    uint8_t RAMWR = lcdspi->lcd->ctrl_info->RAMWR;
    uint32_t lcd_ctrl_id = lcdspi->lcd->ctrl_info->id;
    uint16_t sx = lcdspi->lcd->sx;
    uint16_t sy = lcdspi->lcd->sy;
    uint16_t v1, v2;
    lcdspi_spi_start_xfer(lcdspi);
    update_dir(lcdspi);
    update_axis16(lcdspi, &sx, &sy);
    if (x1 > x2) {
        swap_uint32(&x1, &x2);
    }
    if (y1 > y2) {
        swap_uint32(&y1, &y2);
    }
    if (x1 == x2) {
        z1 = y1;
        z2 = y2;
    } else {
        z1 = x1;
        z2 = x2;
    }
    if (lcd_ctrl_id == PCF8833 || lcd_ctrl_id == S1D15G10) {
        lcdspi_spi_write_cmd9(PASET);
        lcdspi_spi_write_dat9((uint8_t)(sy + y1));
        lcdspi_spi_write_dat9((uint8_t)(sy + y2 + 1));
        lcdspi_spi_write_cmd9(CASET);
        lcdspi_spi_write_dat9((uint8_t)(sx + x1));
        lcdspi_spi_write_dat9((uint8_t)(sx + x2 + 1));
        v1 = (uint8_t)(col & 0xff);
        v2 = (uint8_t)((col >> 8) & 0xff);
        lcdspi_spi_write_cmd9(RAMWR);
        for (uint32_t i = z1; i <= z2; i += 2) {
            lcdspi_spi_write_dat9(R4G4(v1));
            lcdspi_spi_write_dat9(B4R4(v1, v2));
            lcdspi_spi_write_dat9(G4B4(v2));
        }
    } else if (lcd_ctrl_id == SSD1331) {
        lcdspi_addrset_SSD1331((uint16_t)(sx + x1), (uint16_t)(sy + y1), (uint16_t)(sx + x2), (uint16_t)(sy + y2));
        for (uint32_t i = z1; i <= z2; i += 1) {
            lcdspi_spi_write_dat16(col);
        }
    } else {
        lcdspi_addrset((uint16_t)(sx + x1), (uint16_t)(sy + y1), (uint16_t)(sx + x2), (uint16_t)(sy + y2));
        if (lcd_ctrl_id == ILI9488) {
            for (uint32_t i = z1; i <= z2; i += 1) {
                lcdspi_spi_write_dat16(col);
            }
        } else {
            for (uint32_t i = z1; i <= z2; i += 1) {
                lcdspi_spi_write_dat8((uint8_t)(col >> 8));
                lcdspi_spi_write_dat8((uint8_t)col);
            }
        }
    }
    lcdspi_spi_end_xfer(lcdspi);
}

void lcdspi_hline(lcdspi_t *lcdspi, uint32_t x1, uint32_t y1, uint32_t x2, uint16_t col) {
    lcdspi_simple_line(lcdspi, x1, y1, x2, y1, col);
}

void lcdspi_vline(lcdspi_t *lcdspi, uint32_t x1, uint32_t y1, uint32_t y2, uint16_t col) {
    lcdspi_simple_line(lcdspi, x1, y1, x1, y2, col);
}

void lcdspi_box(lcdspi_t *lcdspi, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint16_t col) {
    lcdspi_hline(lcdspi, x1, y1, x2, col);
    lcdspi_hline(lcdspi, x1, y2, x2, col);
    lcdspi_vline(lcdspi, x1, y1, y2, col);
    lcdspi_vline(lcdspi, x2, y1, y2, col);
}

static inline int32_t _abs(int32_t x) {
    if (x < 0) {
        return -x;
    } else {
        return x;
    }
}

static inline int32_t sign(int32_t x) {
    if (x < 0) {
        return -1;
    } else {
        return 1;
    }
}

void lcdspi_line(lcdspi_t *lcdspi, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint16_t col) {
    int32_t x;
    int32_t y;
    int32_t dx;
    int32_t dy;
    int32_t s1;
    int32_t s2;
    int32_t temp;
    int32_t interchange;
    int32_t e;
    int32_t i;

    x = x1;
    y = y1;
    dx = _abs((int32_t)x2 - (int32_t)x1);
    dy = _abs((int32_t)y2 - (int32_t)y1);
    s1 = sign((int32_t)x2 - (int32_t)x1);
    s2 = sign((int32_t)y2 - (int32_t)y1);

    if (dy > dx) {
        temp = dx;
        dx = dy;
        dy = temp;
        interchange = 1;
    } else {
        interchange = 0;
    }
    e = 2 * dy - dx;
    for (i = 0; i <= dx; i++) {
        lcdspi_pset(lcdspi, (uint32_t)x, (uint32_t)y, col);
        while (e >= 0) {
            if (interchange == 1) {
                x += s1;
            } else {
                y += s2;
            }
            e -= 2 * dx;
        }
        if (interchange == 1) {
            y += s2;
        } else {
            x += s1;
        }
        e += 2 * dy;
    }
}

static void lcdspi_circle_sub(lcdspi_t *lcdspi, uint32_t x, uint32_t y, uint32_t r, uint16_t col, bool fill) {
    int32_t xi = 0;
    int32_t yi = (int32_t)r;
    int32_t di = 2 * (1 - (int32_t)r);
    int32_t limit = 0;
    int32_t delta1;
    int32_t delta2;

    while (1) {
        if (fill) {
            lcdspi_hline(lcdspi, (int32_t)x - xi, (int32_t)y - yi, (int32_t)x + xi, col);
            lcdspi_hline(lcdspi, (int32_t)x - xi, (int32_t)y + yi, (int32_t)x + xi, col);
        } else {
            lcdspi_pset(lcdspi, (int32_t)x + xi, (int32_t)y + yi, col);
            lcdspi_pset(lcdspi, (int32_t)x - xi, (int32_t)y + yi, col);
            lcdspi_pset(lcdspi, (int32_t)x + xi, (int32_t)y - yi, col);
            lcdspi_pset(lcdspi, (int32_t)x - xi, (int32_t)y - yi, col);
        }
        if (yi <= limit) {
            goto circle4;
        }
        if (di < 0) {
            goto circle2;
        }
        if (di > 0) {
            goto circle3;
        }
        // if (di == 0)
        goto circle20;
    circle2:
        delta1 = 2 * di + 2 * yi - 1;
        if (delta1 <= 0) {
            goto circle10;
        }
        // if (delta > 0)
        goto circle20;
    circle3:
        delta2 = 2 * di - 2 * xi - 1;
        if (delta2 <= 0) {
            goto circle20;
        }
        // if (delta2 > 0)
        goto circle30;
    circle10:
        xi += 1;
        di += (2 * xi + 1);
        continue;   // goto circle1;
    circle20:
        xi += 1;
        yi -= 1;
        di += (2 * xi - 2 * yi + 2);
        continue;   // goto circle1;
    circle30:
        yi -= 1;
        di -= (2 * yi - 1);
        continue;   // goto circle1;
    }
circle4:
    return;
}

void lcdspi_circle(lcdspi_t *lcdspi, uint32_t x, uint32_t y, uint32_t r, uint16_t col) {
    lcdspi_circle_sub(lcdspi, x, y, r, col, false);
}

void lcdspi_circle_fill(lcdspi_t *lcdspi, uint32_t x, uint32_t y, uint32_t r, uint16_t col) {
    lcdspi_circle_sub(lcdspi, x, y, r, col, true);
}

void lcdspi_write_font_color_xy(lcdspi_t *lcdspi, unsigned short u, uint32_t x, uint32_t y, uint16_t fgcol, uint16_t bgcol) {
    uint16_t i, j;
    uint16_t wx, wy;
    uint16_t off;
    uint16_t col0, col1;
    uint8_t PASET = lcdspi->lcd->ctrl_info->PASET;
    uint8_t CASET = lcdspi->lcd->ctrl_info->CASET;
    uint8_t RAMWR = lcdspi->lcd->ctrl_info->RAMWR;
    uint32_t lcd_ctrl_id = lcdspi->lcd->ctrl_info->id;
    uint16_t sx = lcdspi->lcd->sx;
    uint16_t sy = lcdspi->lcd->sy;
    uint16_t width = lcdspi->lcd->width;
    uint16_t height = lcdspi->lcd->height;
    font_t *font = lcdspi->screen->font;
    uint8_t *data;

    if (font == (font_t *)NULL) {
        return;
    }
    data = (unsigned char *)font_fontData(font, (int)u);
    wx = (uint16_t)font_fontWidth(font, (int)u);
    wy = (uint16_t)font_fontHeight(font, (int)u);
    update_dir(lcdspi);
    update_axis16(lcdspi, &sx, &sy);
    update_axis16(lcdspi, &width, &height);
    lcdspi_spi_start_xfer(lcdspi);
    off = 0;
    if (lcd_ctrl_id == PCF8833 || lcd_ctrl_id == S1D15G10) {
        for (j = 0; j < wy; j++) {
            lcdspi_spi_write_cmd9(CASET);
            lcdspi_spi_write_dat9((uint8_t)(sx + x));
            lcdspi_spi_write_dat9((uint8_t)(sx + width - 1));
            lcdspi_spi_write_cmd9(PASET);  // y set
            lcdspi_spi_write_dat9((uint8_t)(sy + y + j));
            lcdspi_spi_write_dat9((uint8_t)(sy + height - 1));
            lcdspi_spi_write_cmd9(RAMWR);
            for (i = 0; i < wx; i += 2) {
                if (i == 8) {
                    off++;
                }
                if (data[off] & (0x80 >> (i & 0x7))) {
                    col0 = fgcol;
                } else {
                    col0 = bgcol;
                }
                if (data[off] & (0x40 >> (i & 0x7))) {
                    col1 = fgcol;
                } else {
                    col1 = bgcol;
                }
                lcdspi_spi_write_dat9((0xff & (uint8_t)(col0 >> 4)));
                lcdspi_spi_write_dat9((0xf0 & (uint8_t)(col0 << 4)) | (0x0f & ((uint8_t)(col1 >> 8))));
                lcdspi_spi_write_dat9((uint8_t)(0xff & col1));
            }
            off++;
        }
    } else if (lcd_ctrl_id == SSD1331) {
        for (j = 0; j < wy; j++) {
            lcdspi_addrset_SSD1331((uint16_t)(sx + x),
                (uint16_t)(sy + y + j),
                (uint16_t)(sx + width - 1),
                (uint16_t)(sy + height - 1));
            for (i = 0; i < wx; i++) {
                if (i == 8) {
                    off++;
                }
                if (data[off] & (0x80 >> (i & 0x7))) {
                    col0 = fgcol;
                } else {
                    col0 = bgcol;
                }
                lcdspi_spi_write_dat16(col0);
            }
            off++;
        }

    } else {
        for (j = 0; j < wy; j++) {
            lcdspi_addrset((uint16_t)(sx + x),
                (uint16_t)(sy + y + j),
                (uint16_t)(sx + width - 1),
                (uint16_t)(sy + height - 1));
            for (i = 0; i < wx; i++) {
                if (i == 8) {
                    off++;
                }
                if (data[off] & (0x80 >> (i & 0x7))) {
                    col0 = fgcol;
                } else {
                    col0 = bgcol;
                }
                if (lcd_ctrl_id == ILI9488) {
                    lcdspi_spi_write_dat16(col0);
                } else {
                    lcdspi_spi_write_dat8((uint8_t)(col0 >> 8));
                    lcdspi_spi_write_dat8((uint8_t)col0);
                }
            }
            off++;
        }
    }
    lcdspi_spi_end_xfer(lcdspi);
}

void lcdspi_write_char_color_xy(lcdspi_t *lcdspi, unsigned char c, uint32_t x, uint32_t y, uint16_t fgcol, uint16_t bgcol) {
    lcdspi_write_font_color_xy(lcdspi, (unsigned short)c, x, y, fgcol, bgcol);
}

void lcdspi_write_char_color(lcdspi_t *lcdspi, unsigned char c, uint32_t cx, uint32_t cy, uint16_t fgcol, uint16_t bgcol) {
    font_t *font = lcdspi->screen->font;
    if (font == (font_t *)NULL) {
        return;
    }
    uint16_t ux = (uint16_t)font_fontUnitX(font);
    uint16_t uy = (uint16_t)font_fontUnitY(font);
    lcdspi_write_char_color_xy(lcdspi, c, cx * ux, cy * uy, fgcol, bgcol);
}

void lcdspi_write_unicode_color_xy(lcdspi_t *lcdspi, unsigned short u, uint32_t x, uint32_t y, uint16_t fgcol, uint16_t bgcol) {
    lcdspi_write_font_color_xy(lcdspi, (unsigned short)u, x, y, fgcol, bgcol);
}

void lcdspi_write_unicode_color(lcdspi_t *lcdspi, unsigned short u, uint32_t cx, uint32_t cy, uint16_t fgcol, uint16_t bgcol) {
    font_t *font = lcdspi->screen->font;
    if (font == (font_t *)NULL) {
        return;
    }
    uint16_t ux = (uint16_t)font_fontUnitX(font);
    uint16_t uy = (uint16_t)font_fontUnitY(font);
    lcdspi_write_unicode_color_xy(lcdspi, u, cx * ux, cy * uy, fgcol, bgcol);
}

void lcdspi_write_char_xy(lcdspi_t *lcdspi, unsigned char c, uint32_t x, uint32_t y) {
    lcdspi_write_char_color_xy(lcdspi, c, x, y, lcdspi->screen->fcol, lcdspi->screen->bcol);
}

void lcdspi_write_char(lcdspi_t *lcdspi, unsigned char c, uint32_t row, uint32_t col) {
    lcdspi_write_char_color(lcdspi, c, row, col, lcdspi->screen->fcol, lcdspi->screen->bcol);
}

void lcdspi_write_unicode_xy(lcdspi_t *lcdspi, unsigned short u, uint32_t x, uint32_t y) {
    lcdspi_write_unicode_color_xy(lcdspi, u, x, y, lcdspi->screen->fcol, lcdspi->screen->bcol);
}

void lcdspi_write_unicode(lcdspi_t *lcdspi, unsigned short u, uint32_t row, uint32_t col) {
    lcdspi_write_unicode_color(lcdspi, u, row, col, lcdspi->screen->fcol, lcdspi->screen->bcol);
}

void lcdspi_write_formatted_font(lcdspi_t *lcdspi, unsigned short u) {
    lcdspi_screen_t *screen = lcdspi->screen;
    uint16_t cx = screen->cx;
    uint16_t cy = screen->cy;
    uint16_t dy = screen->dy;
    uint16_t unit_x = (uint32_t)font_fontUnitX(screen->font);
    uint16_t unit_y = (uint32_t)font_fontUnitY(screen->font);
    uint16_t width = lcdspi->lcd->width;
    uint16_t height = lcdspi->lcd->height;
    update_axis16(lcdspi, &width, &height);
    if ((char)u == 0xc) {
        lcdspi_clear(lcdspi, 0);
        cx = 0;
        cy = 0;
        dy = 0;
    } else if ((char)u == '\n') {
        if (cy == height / unit_y - 1) {
            screen->scroll = true;
            cy = 0;
        } else {
            cy++;
        }
        if (screen->scroll) {
            dy += unit_y;
            if (dy >= (height / unit_y * unit_y)) {
                dy = 0;
            }
            if (screen->hw_scroll) {
                lcdspi_scroll(lcdspi, dy);
            }
            for (uint16_t i = 0; i < (width / unit_x); i++) {
                lcdspi_write_unicode(lcdspi, 0x20, i, cy);
            }
        }
    } else if ((char)u == '\r') {
        cx = 0;
    } else {
        lcdspi_write_unicode(lcdspi, u, cx, cy);
        if (u < 0x100) {
            cx++;
        } else {
            cx += 2;
        }
        if (cx >= width / unit_x) {
            cx = 0;
            if (cy == height / unit_y - 1) {
                screen->scroll = true;
                cy = 0;
            } else {
                cy++;
            }
            if (screen->scroll) {
                dy += unit_y;
                if (dy >= (height / unit_y * unit_y)) {
                    dy = 0;
                }
                if (screen->hw_scroll) {
                    lcdspi_scroll(lcdspi, dy);
                }
                for (uint16_t i = 0; i < (width / unit_x); i++) {
                    lcdspi_write_unicode(lcdspi, 0x20, i, cy);
                }
            }
        }
    }
    screen->cx = (uint16_t)cx;
    screen->cy = (uint16_t)cy;
    screen->dy = (uint16_t)dy;
}

void lcdspi_write_formatted_char(lcdspi_t *lcdspi, unsigned char ch) {
    lcdspi_write_formatted_font(lcdspi, (unsigned short)ch);
}

void lcdspi_write_formatted_unicode(lcdspi_t *lcdspi, unsigned short u) {
    lcdspi_write_formatted_font(lcdspi, (unsigned short)u);
}

void lcdspi_set_font(lcdspi_t *lcdspi, font_t *font) {
    lcdspi->screen->font = font;
}

font_t *lcdspi_get_font(lcdspi_t *lcdspi) {
    return lcdspi->screen->font;
}

unsigned short cnvUtf8ToUnicode(unsigned char *str, uint32_t *size) {
    unsigned int u = 0;
    unsigned char c = *str++;
    int len = 0;
    if ((c & 0x80) == 0) {
        u = c & 0x7F;
        len = 0;
    } else if ((c & 0xE0) == 0xC0) {
        u = c & 0x1F;
        len = 1;
    } else if ((c & 0xF0) == 0xE0) {
        u = c & 0x0F;
        len = 2;
    } else if ((c & 0xF8) == 0xF0) {
        u = c & 0x07;
        len = 3;
    } else if ((c & 0xFC) == 0xF8) {
        u = c & 0x03;
        len = 4;
    } else if ((c & 0xFE) == 0xFC) {
        u = c & 0x01;
        len = 5;
    }
    *size = (uint32_t)(len + 1);
    while (len-- > 0 && ((c = *str) & 0xC0) == 0x80) {
        u = (u << 6) | (unsigned int)(c & 0x3F);
        str++;
    }
    return (unsigned short)u;
}

// #endif
