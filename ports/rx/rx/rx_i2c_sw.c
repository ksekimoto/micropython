/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
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

#include <stdio.h>
#include <stdint.h>
#include "common.h"
#include "iodefine.h"
#include "interrupt_handlers.h"
#include "rx_gpio.h"
#include "rx_i2c_sw.h"

static uint16_t clock_delay = 10;
static uint16_t clock_delay2 = 10;

// if delay = 1, cycle is about 1MHz.
static void _delay_loop_2(uint16_t delay) {
    int count = (int)delay * (int)18;
    for (int i = 0; i < count; i++) {
        __asm__ __volatile__ ("nop");
    }
}

static void i2c_io_set_sda(uint32_t scl, uint32_t sda, uint8_t hi) {
    (void)scl;
    if (hi) {
        rx_gpio_config((uint8_t)sda, GPIO_MODE_INPUT, GPIO_PULLUP, 0);
    } else {
        rx_gpio_mode_output((uint8_t)sda);
        rx_gpio_write((uint8_t)sda, 0);
    }
}

static uint8_t i2c_io_get_sda(uint32_t scl, uint32_t sda) {
    (void)scl;
    return (uint8_t)rx_gpio_read((uint8_t)sda);
}

static void i2c_io_set_scl(uint32_t scl, uint32_t sda, uint8_t hi) {
    (void)sda;
    #ifdef ENABLE_SCL_EXPAND
    _delay_loop_2(clock_delay2);
    if (hi) {
        rx_gpio_config((uint8_t)scl, GPIO_MODE_INPUT, GPIO_PULLUP, 0);
        // wait while pin is pulled low by client
        while (!(rx_gpio_read((uint8_t)scl))) {
            ;
        }
    } else {
        rx_gpio_mode_output((uint8_t)sda);
        rx_gpio_write((uint8_t)sda, 0);
    }
    _delay_loop_2(clock_delay);
    #else
    _delay_loop_2(clock_delay2);
    if (hi) {
        rx_gpio_write((uint8_t)scl, 1);
    } else {
        rx_gpio_write((uint8_t)scl, 0);
    }
    _delay_loop_2(clock_delay);
    #endif
}

/* clock HI, delay, then LO */
static void i2c_scl_toggle(uint32_t scl, uint32_t sda) {
    i2c_io_set_scl(scl, sda, 1);
    i2c_io_set_scl(scl, sda, 0);
}

/* i2c start condition */
void rx_i2c_sw_start(uint32_t scl, uint32_t sda) {
    i2c_io_set_sda(scl, sda, 0);
    i2c_io_set_scl(scl, sda, 0);
}

/* i2c repeated start condition */
void rx_i2c_sw_repstart(uint32_t scl, uint32_t sda) {
    /* scl, sda may not be high */
    i2c_io_set_sda(scl, sda, 1);
    i2c_io_set_scl(scl, sda, 1);

    i2c_io_set_sda(scl, sda, 0);
    i2c_io_set_scl(scl, sda, 0);
}

/* i2c stop condition */
void rx_i2c_sw_stop(uint32_t scl, uint32_t sda) {
    i2c_io_set_sda(scl, sda, 0);
    i2c_io_set_scl(scl, sda, 1);
    i2c_io_set_sda(scl, sda, 1);
}

uint8_t rx_i2c_sw_write_byte(uint32_t scl, uint32_t sda, uint8_t b) {
    uint8_t i;
    for (i = 0; i < 8; i++) {
        if (b & (1 << (7 - i))) {
            i2c_io_set_sda(scl, sda, 1);
        } else {
            i2c_io_set_sda(scl, sda, 0);
        }
        i2c_scl_toggle(scl, sda);       // clock HI, delay, then LO
    }
    i2c_io_set_sda(scl, sda, 1);        // leave SDL HI
    i2c_io_set_scl(scl, sda, 1);        // clock back up
    b = i2c_io_get_sda(scl, sda);       // get the ACK bit
    i2c_io_set_scl(scl, sda, 0);        // not really ??
    return b == 0;                      // return ACK value
}

uint8_t rx_i2c_sw_read_byte(uint32_t scl, uint32_t sda, uint8_t last) {
    uint8_t i;
    uint8_t c, b = 0;
    i2c_io_set_sda(scl, sda, 1);        // make sure pullups are activated
    i2c_io_set_scl(scl, sda, 0);        // clock LOW

    for (i = 0; i < 8; i++) {
        i2c_io_set_scl(scl, sda, 1);    // clock HI
        c = i2c_io_get_sda(scl, sda);
        b <<= 1;
        if (c) {
            b |= 1;
        }
        i2c_io_set_scl(scl, sda, 0);    // clock LO
    }
    if (last) {
        i2c_io_set_sda(scl, sda, 1);    // set NAK
    } else {
        i2c_io_set_sda(scl, sda, 0);    // set ACK
    }
    i2c_scl_toggle(scl, sda);           // clock pulse
    i2c_io_set_sda(scl, sda, 1);        // leave with SDL HI
    return b;                           // return received byte
}

void rx_i2c_sw_scan(uint32_t scl, uint32_t sda) {
    uint8_t i = 0;
    for (i = 0; i < 127; i++) {
        rx_i2c_sw_start(scl, sda);                      // do start transition
        if (rx_i2c_sw_write_byte(scl, sda, i << 1)) {
            // DEBUGF("I2C device at address 0x%x\n", i);
        }
        rx_i2c_sw_stop(scl, sda);
    }
}

void rx_i2c_sw_init(uint32_t ch, uint32_t scl, uint32_t sda, uint32_t baudrate, uint32_t timeout) {
    (void)ch;
    (void)scl;
    (void)baudrate;
    (void)timeout;
    rx_gpio_config((uint8_t)sda, GPIO_MODE_INPUT, GPIO_PULLUP, 0);
    #ifdef ENABLE_SCL_EXPAND
    rx_gpio_config((uint8_t)scl, GPIO_MODE_INPUT, GPIO_PULLUP, 0);
    #else
    rx_gpio_mode_output((uint8_t)sda);
    #endif
}
