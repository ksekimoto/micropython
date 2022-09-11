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

#include "PinNames.h"
#include "spi_api.h"
#include "dma_api.h"
#include "mbed_spi.h"

#define SPI_CH_NUM  3

static spi_t MBED_SPI[SPI_CH_NUM];

static const uint32_t spi_mosi_pins[SPI_CH_NUM] = {
    0x86,   /* ch 0 P86 */
    0xf1,   /* ch 1 PF1 */
    0xc1,   /* ch 2 PC1 */
};

static const uint32_t spi_miso_pins[SPI_CH_NUM] = {
    0x85,   /* ch 0 P85 */
    0xf2,   /* ch 1 PF2 */
    0xc2,   /* ch 2 PC2 */
};

static const uint32_t spi_clk_pins[SPI_CH_NUM] = {
    0x87,   /* ch 0 P87 */
    0xf0,   /* ch 1 PF0 */
    0xc0,   /* ch 2 PC0 */
};

void mbed_spi_set_bits(uint32_t ch, uint32_t bits) {

}

void mbed_spi_set_clk(uint32_t ch, uint32_t spi_clk) {
    spi_t *pspi = &MBED_SPI[ch];
    spi_frequency(pspi, spi_clk);
}

void mbed_spi_set_firstbit(uint32_t ch, uint32_t firstbit) {

}

void mbed_spi_set_spi_ch(uint32_t ch, uint32_t polarity, uint32_t phase) {

}

void mbed_spi_transfer(uint32_t ch, uint32_t bits, uint8_t *dst, uint8_t *src, uint32_t count, uint32_t timeout) {
    spi_t *pspi = &MBED_SPI[ch];
    uint32_t src_count = count;
    uint32_t dst_count = count;
    if (src == NULL) {
        src_count = 0;
    }
    if (dst == NULL) {
        dst_count = 0;
    }
    spi_master_block_write(pspi, (const char *)src, (int)src_count, (char *)dst, (int)dst_count, (char)0xff);
    // spi_master_transfer(pspi, (const void *)src, (size_t)src_count, (void *)dst, (size_t)dst_count, 8, 0, 0, DMA_USAGE_NEVER);
}

void mbed_spi_init(uint32_t ch, uint32_t cs, uint32_t baud, uint32_t bits, uint32_t mode) {
    spi_t *pspi = &MBED_SPI[ch];
    PinName mosi = (PinName)spi_mosi_pins[ch];
    PinName miso = (PinName)spi_miso_pins[ch];
    PinName sclk = (PinName)spi_clk_pins[ch];
    PinName ssel = (PinName)NC;
    spi_init(pspi, mosi, miso, sclk, ssel);
    spi_format(pspi, bits, mode, 0);
}

void mbed_spi_deinit(uint32_t ch, uint32_t cs) {
    spi_t *pspi = &MBED_SPI[ch];
    spi_free(pspi);
}
