/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Kentaro Sekimoto
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

#ifndef PORTS_RZ_MBED_MBED_SPI_H_
#define PORTS_RZ_MBED_MBED_SPI_H_

#ifdef __cplusplus
extern "C" {
#endif

void mbed_spi_set_bits(uint32_t ch, uint32_t bits);
void mbed_spi_set_clk(uint32_t ch, uint32_t spi_clk);
void mbed_spi_set_firstbit(uint32_t ch, uint32_t firstbit);
void mbed_spi_set_spi_ch(uint32_t ch, uint32_t polarity, uint32_t phase);
void mbed_spi_transfer(uint32_t ch, uint32_t bits, uint8_t *dst, uint8_t *src, uint32_t count, uint32_t timeout);
void mbed_spi_init(uint32_t ch, uint32_t cs, uint32_t baud, uint32_t bits, uint32_t mode);
void mbed_spi_deinit(uint32_t ch, uint32_t cs);

#ifdef __cplusplus
};
#endif

#endif /* PORTS_RZ_MBED_MBED_SPI_H_ */
