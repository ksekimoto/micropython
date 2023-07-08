/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Kentaro Sekimoto
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

#ifndef DRIVERS_ESP_MPY_UART_H_
#define DRIVERS_ESP_MPY_UART_H_

void esp_serial_begin(int uart_id, int baud);
bool esp_serial_available(void);
uint8_t esp_serial_read_ch(void);
uint32_t esp_serial_read_str(uint8_t *buf, size_t size);
void esp_serial_write_byte(uint8_t c);
uint32_t esp_serial_write_bytes(uint8_t *buf, size_t size);
void esp_serial_str(const char *s);
void esp_serial_str_int(int i);
void esp_serial_strln(const char *s);
void esp_serial_str_intln(int i);
#if esp_serial_printf
#include "vsnprintf.h"
int esp_serial_printf(const void *format, ...);
#endif
void mpy_uart_debug_write(bool flag);
void mpy_uart_debug_read(bool flag);

#endif /* DRIVERS_ESP_MPY_UART_H_ */
