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

#include <stdint.h>
#include <stdbool.h>
#include "common.h"
#include "rx65n_spi.h"

#define SPI_DEFAULT_POLARITY 0
#define SPI_DEFAULT_PHASE 0

static vp_ir g_spri[] = {
    (vp_ir)(0x87000 + IR_RSPI0_SPRI0),
    (vp_ir)(0x87000 + IR_RSPI1_SPRI1),
    (vp_ir)(0x87000 + IR_RSPI2_SPRI2)
};

static vp_ier g_ier[] = {
    (vp_ier)(0x87200 + IER_RSPI0_SPRI0),
    (vp_ier)(0x87200 + IER_RSPI1_SPRI1),
    (vp_ier)(0x87200 + IER_RSPI2_SPRI2)
};

static uint8_t g_ier_bit[] = {
    7,      // IEN7
    2,      // IEN2
    5,      // IEN5
};

static vp_ipr g_ipr[] = {
    (vp_ipr)(0x87300 + IPR_RSPI0_SPRI0),
    (vp_ipr)(0x87300 + IPR_RSPI1_SPRI1),
    (vp_ipr)(0x87300 + IPR_RSPI2_SPRI2)
};

static vp_rspi g_rspi[] = {
#if defined(RX63N)
    (vp_rspi)0x88380,   // RSPI0
    (vp_rspi)0x883A0,   // RSPI1
    (vp_rspi)0x883C0    // RSPI2
#endif
#if defined(RX65N) || defined(RX64M)
    (vp_rspi)0xD0100,   // RSPI0
    (vp_rspi)0xD0140,   // RSPI1
    (vp_rspi)0xD0300    // RSPI2
#endif
};

static uint8_t SPI_PINS[] = {
    PC5,    // PC5: RSPCKA
    PC6,    // PC6: MOSIA
    PC7,    // PC7: MISOA
    PE5,    // PE5: RSPCKB-B
    PE6,    // PE6: MOSIB-B
    PE7,    // PE7: MISOB-B
    PD3,    // PD3: RSPCKC
    PD1,    // PD1: MOSIC
    PD2     // PD2: MISOC
};

void rx_spi_get_pins(uint32_t ch, uint8_t *mosi, uint8_t *miso, uint8_t *clk) {
    *mosi = SPI_PINS[ch * 3 + 1];
    *miso = SPI_PINS[ch * 3 + 2];
    *clk = SPI_PINS[ch * 3];
}

static void rx_spi_set_MSTP(uint32_t ch, int bit) {
    switch (ch) {
    case 0:
        SYSTEM.MSTPCRB.BIT.MSTPB17 = bit;
        break;
    case 1:
        SYSTEM.MSTPCRB.BIT.MSTPB16 = bit;
        break;
    default:
        SYSTEM.MSTPCRC.BIT.MSTPC22 = bit;
        break;
    }
}

static void rx_spi_set_PMR(uint8_t pin, uint32_t value) {
    uint8_t port = pin >> 3;
    uint8_t bit = pin & 0x7;
    if (value)
        _PMR(port) |= (1 << bit);
    else
        _PMR(port) &= ~(1 << bit);
}

inline static void rx_spi_set_ir(uint32_t ch, int bit) {
    g_spri[ch]->IR = bit;
}

inline static bool rx_spi_chk_ir(uint32_t ch, int bit) {
    return (g_spri[ch]->IR == bit);
}

void rx_spi_set_bits(uint32_t ch, uint32_t bits) {
    vp_rspi prspi = g_rspi[ch];
    if (bits == 8) {
        prspi->SPCMD0.WORD = (prspi->SPCMD0.WORD & ~0x0f00) | 0x0700; // Command Reg: SPI mode: 8bit
    } else if (bits == 16) {
        prspi->SPCMD0.WORD = (prspi->SPCMD0.WORD & ~0x0f00) | 0x0f00; // Command Reg: SPI mode: 16bit
    } else if (bits == 32) {
        prspi->SPCMD0.WORD = (prspi->SPCMD0.WORD & ~0x0f00) | 0x0300; // Command Reg: SPI mode: 32bit
    }
}

void rx_spi_set_clk(uint32_t ch, uint32_t baud) {
    if (baud == 0)
        return;
    vp_rspi prspi = g_rspi[ch];
    prspi->SPCR.BIT.SPE = 0;
    prspi->SPBR = BCLK / 2 / baud - 1;
    prspi->SPCR.BIT.SPE = 1;
}

void rx_spi_set_firstbit(uint32_t ch, uint32_t firstbit) {
    vp_rspi prspi = g_rspi[ch];
    if (firstbit)
        prspi->SPCMD0.WORD |= 0x1000;
    else
        prspi->SPCMD0.WORD &= ~0x1000;
}

void rx_spi_set_mode(uint32_t ch, uint32_t polarity, uint32_t phase) {
    vp_rspi prspi = g_rspi[ch];
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

void rx_spi_select_spi_pin(uint32_t ch) {
    uint8_t pinCLK = SPI_PINS[ch * 3 + 0];
    uint8_t pinMOSI = SPI_PINS[ch * 3 + 1];
    uint8_t pinMISO = SPI_PINS[ch * 3 + 2];
    rx_spi_set_PMR(pinCLK, 0);
    rx_spi_set_PMR(pinMOSI, 0);
    rx_spi_set_PMR(pinMISO, 0);
    gpio_mode_output(pinCLK);
    gpio_mode_output(pinMOSI);
    gpio_mode_input(pinMISO);
    MPC.PWPR.BIT.B0WI = 0;
    MPC.PWPR.BIT.PFSWE = 1;
    _MPC(pinCLK) = 13;
    _MPC(pinMOSI) = 13;
    _MPC(pinMISO) = 13;
    MPC.PWPR.BIT.PFSWE = 0;
    rx_spi_set_PMR(pinCLK, 1);
    rx_spi_set_PMR(pinMOSI, 1);
    rx_spi_set_PMR(pinMISO, 1);
}

void rx_spi_reset_spi_pin(uint32_t ch) {
    uint8_t pinCLK = SPI_PINS[ch * 3 + 0];
    uint8_t pinMOSI = SPI_PINS[ch * 3 + 1];
    uint8_t pinMISO = SPI_PINS[ch * 3 + 2];
    rx_spi_set_PMR(pinCLK, 0);
    rx_spi_set_PMR(pinMOSI, 0);
    rx_spi_set_PMR(pinMISO, 0);
}

void rx_spi_set_spi_ch(uint32_t ch, uint32_t polarity, uint32_t phase) {
    vp_rspi prspi = g_rspi[ch];
    SYSTEM.PRCR.WORD = 0xA502;
    rx_spi_set_MSTP(ch, 0);
    SYSTEM.PRCR.WORD = 0xA500;

    rx_spi_select_spi_pin(ch);
    g_ipr[ch]->IPR = 0;
    g_ier[ch]->BYTE &= ~(1 << g_ier_bit[ch]);
    prspi->SPCR.BYTE = 0;       /* stop SPI */
    prspi->SPSR.BYTE = 0xa0;
    prspi->SPPCR.BYTE = 0;      /* fixed idle value, disable loop-back mode */
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
    prspi->SPCR.BYTE = 0xC8;    /* Start SPI in master mode */
    prspi->SPCR2.BYTE = 0;
}

void rx_spi_reset_spi_ch(uint32_t ch) {
    SYSTEM.PRCR.WORD = 0xA502;
    rx_spi_set_MSTP(ch, 1);
    rx_spi_reset_spi_pin(ch);
    SYSTEM.PRCR.WORD = 0xA500;
}

uint8_t rx_spi_write_byte(uint32_t ch, uint8_t b) {
    vp_rspi prspi = g_rspi[ch];
    rx_spi_set_ir(ch, 0);
    prspi->SPDR.LONG = (uint32_t)(b);
    while (rx_spi_chk_ir(ch, 0))
            ;
    return (uint8_t)(prspi->SPDR.LONG);
}

void rx_spi_write_bytes8(uint32_t ch, uint8_t *buf, uint32_t count) {
    vp_rspi prspi = g_rspi[ch];
    rx_spi_set_bits(ch, 8);
    while (count--) {
        rx_spi_set_ir(ch, 0);
        prspi->SPDR.LONG = (uint32_t)(*buf++);
        while (rx_spi_chk_ir(ch, 0))
            ;
        prspi->SPDR.LONG;
    }
}

void rx_spi_write_bytes16(uint32_t ch, uint16_t *buf, uint32_t count) {
    vp_rspi prspi = g_rspi[ch];
    rx_spi_set_bits(ch, 16);
    while (count--) {
        rx_spi_set_ir(ch, 0);
        prspi->SPDR.LONG = (uint32_t)(*buf++);
        while (rx_spi_chk_ir(ch, 0))
            ;
        prspi->SPDR.LONG;
    }
    rx_spi_set_bits(ch, 8);
}

void rx_spi_write_bytes32(uint32_t ch, uint32_t *buf, uint32_t count) {
    vp_rspi prspi = g_rspi[ch];
    rx_spi_set_bits(ch, 32);
    while (count--) {
        rx_spi_set_ir(ch, 0);
        prspi->SPDR.LONG = (uint32_t)(*buf++);
        while (rx_spi_chk_ir(ch, 0))
            ;
        prspi->SPDR.LONG;
    }
    rx_spi_set_bits(ch, 8);
}

void rx_spi_write_bytes(uint32_t ch, uint32_t bits, uint8_t *buf, uint32_t count) {
    if (bits == 8) {
        rx_spi_write_bytes8(ch, buf, count);
    } else if (bits == 16) {
        rx_spi_write_bytes16(ch, (uint16_t *)buf, count >> 1);
    } else if (bits == 32) {
        rx_spi_write_bytes32(ch, (uint32_t *)buf, count >> 2);
    }
}

void rx_spi_transfer8(uint32_t ch, uint8_t *dst, uint8_t *src, uint32_t count) {
    vp_rspi prspi = g_rspi[ch];
    rx_spi_set_bits(ch, 8);
    while (count--) {
        rx_spi_set_ir(ch, 0);
        prspi->SPDR.LONG = (uint32_t)(*src);
        while (rx_spi_chk_ir(ch, 0))
            ;
        *dst = (uint8_t)(prspi->SPDR.LONG);
        src++;
        dst++;
    }
}

void rx_spi_transfer16(uint32_t ch, uint16_t *dst, uint16_t *src, uint32_t count) {
    vp_rspi prspi = g_rspi[ch];
    rx_spi_set_bits(ch, 16);
    while (count--) {
        rx_spi_set_ir(ch, 0);
        prspi->SPDR.LONG = (uint32_t)(*src);
        while (rx_spi_chk_ir(ch, 0))
            ;
        *dst = (uint16_t)(prspi->SPDR.LONG);
        src++;
        dst++;
    }
    rx_spi_set_bits(ch, 8);
}

void rx_spi_transfer32(uint32_t ch, uint32_t *dst, uint32_t *src, uint32_t count) {
    vp_rspi prspi = g_rspi[ch];
    rx_spi_set_bits(ch, 32);
    while (count--) {
        rx_spi_set_ir(ch, 0);
        prspi->SPDR.LONG = (uint32_t)(*src);
        while (rx_spi_chk_ir(ch, 0))
            ;
        *dst = (uint32_t)(prspi->SPDR.LONG);
        src++;
        dst++;
    }
    rx_spi_set_bits(ch, 8);
}

void rx_spi_transfer(uint32_t ch, uint32_t bits, uint8_t *dst, uint8_t *src, uint32_t count, uint32_t timeout) {
    if (bits == 8) {
        rx_spi_transfer8(ch, dst, src, count);
    } else if (bits == 16) {
        rx_spi_transfer16(ch, (uint16_t *)dst, (uint16_t *)src, count >> 1);
    } else if (bits == 32) {
        rx_spi_transfer32(ch, (uint32_t *)dst, (uint32_t *)src, count >> 2);
    }
}

void rx_spi_start_xfer(uint32_t ch, uint16_t spcmd, uint8_t spbr) {
    vp_rspi prspi = g_rspi[ch];
    prspi->SPCR.BIT.SPE = 0; //Stop SPI
    prspi->SPCMD0.WORD = spcmd;
    prspi->SPBR = spbr;
    prspi->SPCR.BIT.SPE = 1; //Start SPI
}

void rx_spi_end_xfer(uint32_t ch) {
}

void rx_spi_get_conf(uint32_t ch, uint16_t *spcmd, uint8_t *spbr) {
    vp_rspi prspi = g_rspi[ch];
    *spcmd = prspi->SPCMD0.WORD;
    *spbr = prspi->SPBR;
}

void rx_spi_init(uint32_t ch, uint32_t cs, uint32_t baud, uint32_t bits, uint32_t mode) {
    uint32_t polarity = mode & 1;
    uint32_t phase = (mode & 2) >> 1;
    rx_spi_set_spi_ch(ch, polarity, phase);
    rx_spi_set_clk(ch, baud);
    rx_spi_set_bits(ch, bits);
    gpio_mode_output(cs);
    gpio_write(cs, 1);
    return;
}

void rx_spi_deinit(uint32_t ch, uint32_t cs) {
    rx_spi_reset_spi_ch(ch);
    gpio_write(cs, 1);
}
