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

#ifndef RX_RX_I2C_SW_H_
#define RX_RX_I2C_SW_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void rx_i2c_sw_start(uint32_t scl, uint32_t sda);
void rx_i2c_sw_repstart(uint32_t scl, uint32_t sda);
void rx_i2c_sw_stop(uint32_t scl, uint32_t sda);
uint8_t rx_i2c_sw_write_byte(uint32_t scl, uint32_t sda, uint8_t b);
uint8_t rx_i2c_sw_read_byte(uint32_t scl, uint32_t sda, uint8_t last);
void rx_i2c_sw_scan(uint32_t scl, uint32_t sda);
void rx_i2c_sw_init(uint32_t ch, uint32_t scl, uint32_t sda, uint32_t baudrate, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* RX_RX_I2C_SW_H_ */
