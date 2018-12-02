/*
 * Copyright (c) 2018, Kentaro Sekimoto
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

#ifndef RX63N_SPI_H_
#define RX63N_SPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" int _debug_printf(int ch, const char* format, ...);
#else
int _debug_printf(int ch, const char* format, ...);
#endif

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

#define BCLK        48000000    /* 48MHz: SPI input clock default */
//#define CLK_FAST    24000000    /* 24MHz */
//#define CLK_SLOW    400000      /* 400KHz */

struct st_ir {
    unsigned char IR:1;
    unsigned char :7;
};
typedef volatile struct st_ir *vp_ir;
struct st_ier {
    unsigned char BYTE;
};
typedef volatile struct st_ier *vp_ier;
struct st_ipr {
    unsigned char IPR:4;
    unsigned char :4;
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
void rx_spi_write_bytes(uint32_t ch, uint8_t *buf, uint32_t count);
void rx_spi_transfer32(uint32_t ch, uint32_t *dst, uint32_t *src, uint32_t count);
void rx_spi_transfer16(uint32_t ch, uint16_t *dst, uint16_t *src, uint32_t count);
void rx_spi_transfer8(uint32_t ch, uint8_t *dst, uint8_t *src, uint32_t count);
void rx_spi_transfer(uint32_t ch, uint8_t *dst, uint8_t *src, uint32_t count, uint32_t timeout);
void rx_spi_init(uint32_t ch, uint32_t cs, uint32_t speed, uint32_t bits, uint32_t mode);
void rx_spi_deinit(uint32_t ch, uint32_t cs);
void rx_spi_start_xfer(uint32_t ch, uint16_t spcmd, uint8_t spbr);
void rx_spi_end_xfer(uint32_t ch);
void rx_spi_get_conf(uint32_t ch, uint16_t *spcmd, uint8_t *spbr);

#ifdef __cplusplus
}
#endif

#endif /* RX63N_SPI_H_ */
