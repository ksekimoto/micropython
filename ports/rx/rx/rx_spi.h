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

#ifndef RX_RX_SPI_H_
#define RX_RX_SPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "rx_gpio.h"

#undef CH_COM
#undef CH_LCD
#undef CH_BOTH
#define CH_COM  1
#define CH_LCD  2
#define CH_BOTH (CH_COM | CH_LCD)

#ifdef SPI_DEBUG
#define SPI_ENTER(ch)               _debug_printf(ch, "%s ENTER\n", __FUNCTION__)
#define SPI_EXIT(ch)                _debug_printf(ch, "%s EXIT \n", __FUNCTION__)
#define SPI_TRACE1(ch, a)           _debug_printf(ch, (const char *)a)
#define SPI_TRACE2(ch, a, b)        _debug_printf(ch, (const char *)a, b)
#define SPI_TRACE3(ch, a, b, c)     _debug_printf(ch, (const char *)a, b, c)
#else
#define SPI_ENTER(ch)
#define SPI_EXIT(ch)
#define SPI_TRACE1(ch, a)
#define SPI_TRACE2(ch, a, b)
#define SPI_TRACE3(ch, a, b, c)
#endif

#define BCLK    60000000    /* 48MHz: SPI input clock default */
// #define CLK_FAST    24000000    /* 24MHz */
// #define CLK_SLOW    400000      /* 400KHz */

struct st_ir {
    unsigned char IR : 1;
    unsigned char : 7;
};
typedef volatile struct st_ir *vp_ir;
struct st_ier {
    unsigned char BYTE;
};
typedef volatile struct st_ier *vp_ier;
struct st_ipr {
    unsigned char IPR : 4;
    unsigned char : 4;
};
typedef volatile struct st_ipr *vp_ipr;
typedef volatile struct st_rspi *vp_rspi;

void rx_spi_set_bits(uint32_t ch, uint32_t bits);
void rx_spi_set_clk(uint32_t ch, uint32_t spi_clk);
void rx_spi_set_firstbit(uint32_t ch, uint32_t firstbit);
void rx_spi_set_spi_ch(uint32_t ch, uint32_t polarity, uint32_t phase);
uint8_t rx_spi_write_byte(uint32_t ch, uint8_t b);
void rx_spi_write_bytes32(uint32_t ch, uint32_t *buf, uint32_t count);
void rx_spi_write_bytes16(uint32_t ch, uint16_t *buf, uint32_t count);
void rx_spi_write_bytes8(uint32_t ch, uint8_t *buf, uint32_t count);
void rx_spi_write_bytes(uint32_t ch, uint32_t bits, uint8_t *buf, uint32_t count);
void rx_spi_transfer32(uint32_t ch, uint32_t *dst, uint32_t *src, uint32_t count);
void rx_spi_transfer16(uint32_t ch, uint16_t *dst, uint16_t *src, uint32_t count);
void rx_spi_transfer8(uint32_t ch, uint8_t *dst, uint8_t *src, uint32_t count);
void rx_spi_transfer(uint32_t ch, uint32_t bits, uint8_t *dst, uint8_t *src, uint32_t count, uint32_t timeout);
void rx_spi_init(uint32_t ch, uint32_t cs, uint32_t speed, uint32_t bits, uint32_t mode);
void rx_spi_init_with_pin(uint32_t ch, uint32_t mosi, uint32_t miso, uint32_t clk, uint32_t cs, uint32_t baud, uint32_t bits, uint32_t mode);
void rx_spi_deinit(uint32_t ch, uint32_t cs);
void rx_spi_start_xfer(uint32_t ch, uint16_t spcmd, uint8_t spbr);
void rx_spi_end_xfer(uint32_t ch);
void rx_spi_get_conf(uint32_t ch, uint16_t *spcmd, uint8_t *spbr);

#ifdef __cplusplus
}
#endif

#endif /* RX_RX_SPI_H_ */
