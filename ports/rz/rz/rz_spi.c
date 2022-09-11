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

#include <stdint.h>
#include <stdbool.h>
#include "common.h"
#include "rspi_iodefine.h"
#include "rz_gpio.h"
#include "rz_spi.h"

// typedef unsigned int uint32_t;

#define SPI_DEFAULT_POLARITY 0
#define SPI_DEFAULT_PHASE 0

#define SPI_CH_NUM  3

static rspip RSPIP[SPI_CH_NUM] = {

    (rspip)0xE800C800,   // RSPI0
    (rspip)0xE800D000,   // RSPI1
    (rspip)0xE800D800    // RSPI2
};

static const uint32_t MOSI_PINS[SPI_CH_NUM] = {
    0x0086,   /* ch 0 P86 */
    0x0105,   /* ch 1 PF1 -> PG5 */
    0x00c1,   /* ch 2 PC1 */
};

static const uint32_t MISO_PINS[SPI_CH_NUM] = {
    0x0085,   /* ch 0 P85 */
    0x0106,   /* ch 1 PF2 -> PG6 */
    0x00c2,   /* ch 2 PC2 */
};

static const uint32_t CLK_PINS[SPI_CH_NUM] = {
    0x0087,   /* ch 0 P87 */
    0x0104,   /* ch 1 PF0 -> PG4 */
    0x00c0,   /* ch 2 PC0 */
};

void rz_spi_get_pins(uint32_t ch, uint32_t *mosi, uint32_t *miso, uint32_t *clk) {
    *mosi = MOSI_PINS[ch];
    *miso = MISO_PINS[ch];
    *clk = CLK_PINS[ch];
}

static void rz_spi_set_MSTP(uint32_t ch, int bit) {
    switch (ch) {
        case 0:
            CPG.STBCR9.BIT.MSTP97 = bit;
            break;
        case 1:
            CPG.STBCR9.BIT.MSTP96 = bit;
            break;
        default:
            CPG.STBCR9.BIT.MSTP95 = bit;
            break;
    }
}

void rz_spi_set_bits(uint32_t ch, uint32_t bits) {
    rspip prspi = RSPIP[ch];
    if (bits == 8) {
        prspi->SPCMD0.WORD = (prspi->SPCMD0.WORD & ~0x0f00) \
            | 0x0700; // Command Reg: SPI mode: 8bit
        prspi->SPDCR.BYTE = (prspi->SPDCR.BYTE & ~0x60) | 0x20;
    } else if (bits == 16) {
        prspi->SPCMD0.WORD = (prspi->SPCMD0.WORD & ~0x0f00) \
            | 0x0f00; // Command Reg: SPI mode: 16bit
        prspi->SPDCR.BYTE = (prspi->SPDCR.BYTE & ~0x60) | 0x40;
    } else if (bits == 32) {
        prspi->SPCMD0.WORD = (prspi->SPCMD0.WORD & ~0x0f00) \
            | 0x0300; // Command Reg: SPI mode: 32bit
        prspi->SPDCR.BYTE = (prspi->SPDCR.BYTE & ~0x60) | 0x60;
    }
}

void rz_spi_set_clk(uint32_t ch, uint32_t baud) {
    if (baud == 0) {
        return;
    }
    rspip prspi = RSPIP[ch];
    prspi->SPCR.BIT.SPE = 0;
    prspi->SPBR.BYTE = (uint8_t)((BCLK / 2 / baud) - 1);
    prspi->SPCR.BIT.SPE = 1;
}

void rz_spi_set_firstbit(uint32_t ch, uint32_t firstbit) {
    rspip prspi = RSPIP[ch];
    if (firstbit) {
        prspi->SPCMD0.WORD |= 0x1000;
    } else {
        prspi->SPCMD0.WORD &= ~0x1000;
    }
}

void rz_spi_set_mode(uint32_t ch, uint32_t polarity, uint32_t phase) {
    rspip prspi = RSPIP[ch];
    if (polarity != 0) {
        /* CPOL(Clock Polarity) */
        prspi->SPCMD0.BIT.CPOL = 1;
    } else {
        prspi->SPCMD0.BIT.CPOL = 0;
    }
    if (phase != 0) {
        /* CPHA(Clock Phase) */
        prspi->SPCMD0.BIT.CPHA = 1;
    } else {
        prspi->SPCMD0.BIT.CPHA = 0;
    }
}

void rz_spi_select_spi_pin(uint32_t ch) {
    uint32_t clk_pin = CLK_PINS[ch];
    uint32_t mosi_pin = MOSI_PINS[ch];
    uint32_t miso_pin = MISO_PINS[ch];
    rz_gpio_mode_output(clk_pin);
    rz_gpio_mode_output(mosi_pin);
    rz_gpio_mode_input(miso_pin);
    switch (ch) {
        case 0:
            rz_gpio_mode_af(clk_pin, 4);
            rz_gpio_mode_af(mosi_pin, 4);
            rz_gpio_mode_af(miso_pin, 4);
            break;
        case 1:
            rz_gpio_mode_af(clk_pin, 3);
            rz_gpio_mode_af(mosi_pin, 3);
            rz_gpio_mode_af(miso_pin, 3);
            break;
        case 2:
            rz_gpio_mode_af(clk_pin, 4);
            rz_gpio_mode_af(mosi_pin, 4);
            rz_gpio_mode_af(miso_pin, 4);
            break;
        default:
            break;
    }
}

void rz_spi_reset_spi_pin(uint32_t ch) {
    uint32_t clk_pin = CLK_PINS[ch];
    uint32_t mosi_pin = MOSI_PINS[ch];
    uint32_t miso_pin = MISO_PINS[ch];
    rz_gpio_mode_gpio(clk_pin);
    rz_gpio_mode_gpio(mosi_pin);
    rz_gpio_mode_gpio(miso_pin);
}

void rz_spi_set_spi_ch(uint32_t ch, uint32_t polarity, uint32_t phase) {
    rspip prspi = RSPIP[ch];
    rz_spi_set_MSTP(ch, 0);

    rz_spi_select_spi_pin(ch);
    prspi->SPCR.BYTE = 0;       /* stop SPI */
    prspi->SPSR.BYTE = 0x60;
    prspi->SPPCR.BYTE = 0x20;   /* fixed idle value, disable loop-back mode */
    prspi->SPSCR.BYTE = 0;      /* Disable sequence control */
    prspi->SPDCR.BYTE = 0x20;   /* SPLW=1 long access */
    prspi->SPCMD0.WORD = 0x0700;/* LSBF=0, SPB=7, BRDV=0, CPOL=0, CPHA=0 */
    if (polarity == 0) {
        prspi->SPCMD0.BIT.CPOL = 0; /* CPOL(Clock Polarity) */
    } else {
        prspi->SPCMD0.BIT.CPOL = 1; /* CPOL(Clock Polarity) */
    }
    if (phase == 0) {
        prspi->SPCMD0.BIT.CPHA = 0; /* CPHA(Clock Phase) */
    } else {
        prspi->SPCMD0.BIT.CPHA = 1; /* CPHA(Clock Phase) */
    }
    prspi->SPCR.BYTE = 0x48;    /* Start SPI in master mode */
}

void rz_spi_reset_spi_ch(uint32_t ch) {
    rz_spi_set_MSTP(ch, 1);
    rz_spi_reset_spi_pin(ch);
}

uint8_t rz_spi_write_byte(uint32_t ch, uint8_t b) {
    rspip prspi = RSPIP[ch];
    prspi->SPDR.BYTE.LL = (uint8_t)b;
    while (prspi->SPSR.BIT.TEND == 0) {
        ;
    }
    return (uint8_t)prspi->SPDR.BYTE.LL;
}

void rz_spi_transfer8(uint32_t ch, uint8_t *dst, uint8_t *src, uint32_t count) {
    uint8_t dummy;
    rspip prspi = RSPIP[ch];
    rz_spi_set_bits(ch, 8);
    while (count--) {
        if (src != 0) {
            prspi->SPDR.BYTE.LL = (uint8_t)(*src);
            src++;
        } else {
            prspi->SPDR.BYTE.LL = (uint8_t)0x00;
        }
        while (prspi->SPSR.BIT.TEND == 0) {
            ;
        }
        if (dst != 0) {
            *dst = (uint8_t)(prspi->SPDR.BYTE.LL);
            dst++;
        } else {
            dummy = (uint8_t)prspi->SPDR.BYTE.LL;
            (void)dummy;    // to suppress gcc warning
        }
    }
}

void rz_spi_transfer16(uint32_t ch, uint16_t *dst, uint16_t *src, uint32_t count) {
    uint16_t dummy;
    rspip prspi = RSPIP[ch];
    rz_spi_set_bits(ch, 16);
    while (count--) {
        if (src != 0) {
            prspi->SPDR.WORD.L = (uint16_t)(*src);
            src++;
        } else {
            prspi->SPDR.WORD.L = (uint16_t)0x00;
        }
        while (prspi->SPSR.BIT.TEND == 0) {
            ;
        }
        if (dst != 0) {
            *dst = (uint16_t)(prspi->SPDR.WORD.L);
            dst++;
        } else {
            dummy = (uint16_t)prspi->SPDR.WORD.L;
            (void)dummy;    // to suppress gcc warning
        }
    }
    rz_spi_set_bits(ch, 8);
}

void rz_spi_transfer32(uint32_t ch, uint32_t *dst, uint32_t *src, uint32_t count) {
    uint32_t dummy;
    rspip prspi = RSPIP[ch];
    rz_spi_set_bits(ch, 32);
    while (count--) {
        uint32_t state = rz_disable_irq();
        if (src != 0) {
            prspi->SPDR.LONG = (uint32_t)(*src);
            src++;
        } else {
            prspi->SPDR.LONG = (uint32_t)0x00;
        }
        while (prspi->SPSR.BIT.TEND == 0) {
            ;
        }
        if (dst != 0) {
            *dst = (uint32_t)(prspi->SPDR.LONG);
            dst++;
        } else {
            dummy = (uint32_t)prspi->SPDR.LONG;
            (void)dummy;    // to suppress gcc warning
        }
        rz_enable_irq(state);
    }
    rz_spi_set_bits(ch, 8);
}

void rz_spi_transfer(uint32_t ch, uint32_t bits, uint8_t *dst, uint8_t *src, uint32_t count, uint32_t timeout) {
    if (bits == 8) {
        rz_spi_transfer8(ch, dst, src, count);
    } else if (bits == 16) {
        rz_spi_transfer16(ch, (uint16_t *)dst, (uint16_t *)src, count >> 1);
    } else if (bits == 32) {
        rz_spi_transfer32(ch, (uint32_t *)dst, (uint32_t *)src, count >> 2);
    }
}

void rz_spi_start_xfer(uint32_t ch, uint16_t spcmd, uint8_t spbr) {
    rspip prspi = RSPIP[ch];
    prspi->SPCR.BIT.SPE = 0; // Stop SPI
    prspi->SPCMD0.WORD = spcmd;
    prspi->SPBR.BYTE = spbr;
    prspi->SPCR.BIT.SPE = 1; // Start SPI
}

void rz_spi_end_xfer(uint32_t ch) {
}

void rz_spi_get_conf(uint32_t ch, uint16_t *spcmd, uint8_t *spbr) {
    rspip prspi = RSPIP[ch];
    *spcmd = prspi->SPCMD0.WORD;
    *spbr = prspi->SPBR.BYTE;
}

void rz_spi_init(uint32_t ch, uint32_t cs, uint32_t baud, uint32_t bits, uint32_t mode) {
    uint32_t polarity = mode & 1;
    uint32_t phase = (mode & 2) >> 1;
    rz_spi_set_spi_ch(ch, polarity, phase);
    rz_spi_set_clk(ch, baud);
    rz_spi_set_bits(ch, bits);
    rz_gpio_mode_output(cs);
    rz_gpio_write(cs, 1);
    return;
}

void rz_spi_deinit(uint32_t ch, uint32_t cs) {
    rz_spi_reset_spi_ch(ch);
    rz_gpio_write(cs, 1);
}
