/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
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

#include <stdio.h>
#include <string.h>

#include "py/mphal.h"
#include "py/runtime.h"
#include "mpconfigboard.h"

#if MICROPY_HW_ENABLE_LCD_CONSOLE

#include "font.h"
#include "lcdspi_info.h"
#include "lcdspi.h"
#include "lcd.h"
#include "extmod/machine_spi.h"

extern font_t MisakiFont6x12;

#if defined(MICROPY_HW_LCDSPI_CON_CH)
#define DEF_SPI_ID      MICROPY_HW_LCDSPI_CON_CH
#else
#define DEF_SPI_ID      -1
#endif
#define DEF_BAUDRATE    20000000

static lcdspi_screen_t m_screen = {
    .font = (font_t *)&MisakiFont6x12,
    .cx = 0,
    .cy = 0,
    .fcol = 0xffff,
    .bcol = 0x0000,
    .unit_wx = 6,
    .unit_wy = 12,
};
static lcdspi_pins_t m_pins;
static uint32_t m_lcdspi_id = MICROPY_HW_LCDSPI_CON_ID;
static uint32_t m_spi_ch = 0;
static lcdspi_t m_lcdspi = {
    .screen = &m_screen,
};

typedef struct _machine_pin_obj_t {
    mp_obj_base_t base;
    uint32_t id;
} machine_pin_obj_t;

#if 0
static void rz_gpio_mode_input(uint32_t pin) {
    mp_hal_pin_input((mp_hal_pin_obj_t)pin);
}

static void rz_gpio_mode_output(uint32_t pin) {
    mp_hal_pin_output((mp_hal_pin_obj_t)pin);
}

static void rz_gpio_write(uint32_t pin, bool level) {
    mp_hal_pin_write((mp_hal_pin_obj_t)pin, (int)level);
}

static bool rz_gpio_read(uint32_t pin) {
    return (bool)mp_hal_pin_read((mp_hal_pin_obj_t)pin);
}
#endif

#if MICROPY_HW_LCDSPI_CON_CH != -1
extern const mp_obj_type_t machine_pin_type;
extern const mp_obj_type_t machine_spi_type;
static mp_obj_t m_spi_obj;
static mp_machine_spi_p_t *machine_spi_p;

static machine_pin_obj_t pin_mosi = {{&pin_type}, 0};
static machine_pin_obj_t pin_miso = {{&pin_type}, 0};
static machine_pin_obj_t pin_sck = {{&pin_type}, 0};
static mp_obj_t m_args[] = {
    MP_OBJ_NEW_SMALL_INT(DEF_SPI_ID),
    MP_ROM_QSTR(MP_QSTR_baudrate),
    MP_OBJ_NEW_SMALL_INT(DEF_BAUDRATE),
    MP_ROM_QSTR(MP_QSTR_mosi),
    MP_OBJ_FROM_PTR(&pin_mosi),
    MP_ROM_QSTR(MP_QSTR_miso),
    MP_OBJ_FROM_PTR(&pin_miso),
    MP_ROM_QSTR(MP_QSTR_sck),
    MP_OBJ_FROM_PTR(&pin_sck),
};

static void m_spi_init_helper(void) {
    m_spi_obj = machine_spi_type.make_new(&machine_spi_type, 1, 4, (const mp_obj_t *)m_args);
    machine_spi_p = (mp_machine_spi_p_t *)machine_spi_type.protocol;
}

static void m_spi_transfer_helper(size_t len, const uint8_t *src, uint8_t *dest) {
    machine_spi_p->transfer((mp_obj_base_t *)m_spi_obj, (size_t)len, (const uint8_t *)src, (uint8_t *)dest);
}
#else
void m_spi_init_helper(void) {
    lcdspi_spi_init(&m_lcdspi, false);
}

static void m_spi_transfer_helper(size_t len, const uint8_t *src, uint8_t *dest) {
    lcdspi_spisw_transfer(dest, src, (uint32_t)len);
}
#endif

static volatile uint8_t m_dir = MICROPY_HW_LCDSPI_CON_DIR;

void lcd_init(void) {
    #if defined(MICROPY_HW_LCDSPI_CON_CLK)
    m_pins.pin_clk = (uint32_t)(MICROPY_HW_LCDSPI_CON_CLK)->id;
    #else
    m_pins.pin_clk = (uint32_t)PIN_NONE;
    #endif
    #if defined(MICROPY_HW_LCDSPI_CON_MOSI)
    m_pins.pin_dout = (uint32_t)(MICROPY_HW_LCDSPI_CON_MOSI)->id;
    #else
    m_pins.pin_dout = (uint32_t)PIN_NONE;
    #endif
    #if defined(MICROPY_HW_LCDSPI_CON_MISO)
    m_pins.pin_din = (uint32_t)(MICROPY_HW_LCDSPI_CON_MISO)->id;
    #else
    m_pins.pin_din = (uint32_t)PIN_NONE;
    #endif
    #if defined(MICROPY_HW_LCDSPI_CON_CS)
    m_pins.pin_cs = (uint32_t)(MICROPY_HW_LCDSPI_CON_CS)->id;
    #else
    m_pins.pin_cs = (uint32_t)PIN_NONE;
    #endif
    #if defined(MICROPY_HW_LCDSPI_CON_RESET)
    m_pins.pin_reset = (uint32_t)(MICROPY_HW_LCDSPI_CON_RESET)->id;
    #else
    m_pins.pin_reset = (uint32_t)PIN_NONE;
    #endif
    #if defined(MICROPY_HW_LCDSPI_CON_RS)
    m_pins.pin_rs = (uint32_t)(MICROPY_HW_LCDSPI_CON_RS)->id;
    #else
    m_pins.pin_rs = (uint32_t)PIN_NONE;
    #endif
    #if defined(MICROPY_HW_LCDSPI_CON_BL)
    m_pins.pin_bl = (uint32_t)(MICROPY_HW_LCDSPI_CON_BL)->id;
    #else
    m_pins.pin_bl = (uint32_t)PIN_NONE;
    #endif
    #if MICROPY_HW_LCDSPI_CON_CH != -1
    #if defined(MICROPY_HW_LCDSPI_CON_MOSI)
    pin_mosi.id = (MICROPY_HW_LCDSPI_CON_MOSI)->id;
    #endif
    #if defined(MICROPY_HW_LCDSPI_CON_MISO)
    pin_miso.id = (MICROPY_HW_LCDSPI_CON_MISO)->id;
    #endif
    #if defined(MICROPY_HW_LCDSPI_CON_CLK)
    pin_sck.id = (MICROPY_HW_LCDSPI_CON_CLK)->id;
    #endif
    m_args[0] = MP_OBJ_NEW_SMALL_INT(DEF_SPI_ID);
    m_args[1] = MP_ROM_QSTR(MP_QSTR_baudrate);
    m_args[2] = MP_OBJ_NEW_SMALL_INT(DEF_BAUDRATE);
    m_args[3] = MP_ROM_QSTR(MP_QSTR_mosi);
    m_args[4] = MP_OBJ_FROM_PTR(&pin_mosi);
    m_args[5] = MP_ROM_QSTR(MP_QSTR_miso);
    m_args[6] = MP_OBJ_FROM_PTR(&pin_miso);
    m_args[7] = MP_ROM_QSTR(MP_QSTR_sck);
    m_args[8] = MP_OBJ_FROM_PTR(&pin_sck);
    #endif
    m_lcdspi.gpio_output = (lcdspi_gpio_output_t)rz_gpio_mode_output;
    m_lcdspi.gpio_input = (lcdspi_gpio_input_t)rz_gpio_mode_input;
    m_lcdspi.gpio_write = (lcdspi_gpio_write_t)rz_gpio_write;
    m_lcdspi.gpio_read = (lcdspi_gpio_read_t)rz_gpio_read;
    m_lcdspi.spi_init = (lcdspi_spi_init_t)m_spi_init_helper;
    m_lcdspi.spi_transfer = (lcdspi_spi_transfer_t)m_spi_transfer_helper;
    lcdspi_init(&m_lcdspi, &m_screen, &m_pins, m_lcdspi_id, m_spi_ch);
    lcdspi_set_screen_dir(&m_lcdspi, m_dir);
}

void lcd_print_str(const char *str) {
    (void)str;
}

void lcd_print_strn(const char *str, unsigned int len) {
    while (len != 0) {
        lcdspi_write_formatted_char(&m_lcdspi, *str);
        str++;
        len--;
    }
}

#endif // MICROPY_HW_ENABLE_LCD_CONSOLE
