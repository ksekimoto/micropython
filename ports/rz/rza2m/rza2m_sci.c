/*
 * Copyright (c) 2020, Kentaro Sekimoto
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

#include <stdbool.h>
#include "common.h"
#include "iodefine.h"
//#include "pendsv.h"
#include "rza2m_sci.h"
#include "rza2m_gpio.h"

#define RZA2M_SCI_INT_ENABLE

#if defined(RZA2M_SCI_INT_ENABLE)
/* rx interrupt */
void sci_isr_rx0(void);
void sci_isr_rx1(void);
void sci_isr_rx2(void);
void sci_isr_rx3(void);
void sci_isr_rx4(void);
/* tx interrupt */
void sci_isr_tx0(void);
void sci_isr_tx1(void);
void sci_isr_tx2(void);
void sci_isr_tx3(void);
void sci_isr_tx4(void);
/* er interrupt */
void sci_isr_er0(void);
void sci_isr_er1(void);
void sci_isr_er2(void);
void sci_isr_er3(void);
void sci_isr_er4(void);
/* te interrupt */
void sci_isr_te0(void);
void sci_isr_te1(void);
void sci_isr_te2(void);
void sci_isr_te3(void);
void sci_isr_te4(void);
#endif

#define SCI_PCLK    66000000
#define SCI_CH_NUM 5
#define SCI_BUF_SIZE 512
#define SCI_DEFAULT_PRIORITY 4
#define SCI_DEFAULT_BAUD    115200

static volatile struct st_scifa *SCI[SCI_CH_NUM] = {
    (volatile struct st_scifa *)0xE8007000, /* sci 0 */
    (volatile struct st_scifa *)0xE8007800, /* sci 1 */
    (volatile struct st_scifa *)0xE8008000, /* sci 2 */
    (volatile struct st_scifa *)0xE8008800, /* sci 3 */
    (volatile struct st_scifa *)0xE8009000, /* sci 4 */
};

static const uint8_t sci_tx_pins[SCI_CH_NUM] = {
    0x42,   /* ch 0 P42 */
    0x73,   /* ch 1 P73 */
    0xe2,   /* ch 2 PE2 */
    0x63,   /* ch 3 P63 */
    0x90    /* ch 4 P90 */
};

static const uint8_t sci_rx_pins[SCI_CH_NUM] = {
    0x41,   /* ch 0 P41 */
    0x71,   /* ch 1 P71 */
    0xe1,   /* ch 2 PE1 */
    0x62,   /* ch 3 P62 */
    0x91    /* ch 4 P91*/
};

#if defined(RZA2M_SCI_INT_ENABLE)
static const IRQn_Type sci_irqn[SCI_CH_NUM][4] = {
    {RXI0_IRQn, TXI0_IRQn, ERI0_IRQn, TEI0_IRQn},
    {RXI1_IRQn, TXI1_IRQn, ERI1_IRQn, TEI1_IRQn},
    {RXI2_IRQn, TXI2_IRQn, ERI2_IRQn, TEI2_IRQn},
    {RXI3_IRQn, TXI3_IRQn, ERI3_IRQn, TEI3_IRQn},
    {RXI4_IRQn, TXI4_IRQn, ERI4_IRQn, TEI4_IRQn},
};

typedef void (*SCI_ISR)(void);

static const SCI_ISR sci_isr[SCI_CH_NUM][4] = {
    {sci_isr_rx0, sci_isr_tx0, sci_isr_er0, sci_isr_te0},
    {sci_isr_rx1, sci_isr_tx1, sci_isr_er1, sci_isr_te1},
    {sci_isr_rx2, sci_isr_tx2, sci_isr_er2, sci_isr_te2},
    {sci_isr_rx3, sci_isr_tx3, sci_isr_er3, sci_isr_te3},
    {sci_isr_rx4, sci_isr_tx4, sci_isr_er4, sci_isr_te4},
};
#endif

struct SCI_FIFO {
    int tail, head, len, busy;
    uint8_t buff[SCI_BUF_SIZE];
};

static bool sci_init_flag[SCI_CH_NUM] = {
    false, false, false, false, false,
};
static SCI_CALLBACK sci_callback[SCI_CH_NUM] = {
    0, 0, 0, 0, 0,
};
#if defined(RZA2M_SCI_INT_ENABLE)
static volatile struct SCI_FIFO tx_fifo[SCI_CH_NUM];
static volatile struct SCI_FIFO rx_fifo[SCI_CH_NUM];
#endif

static void delay_us(volatile unsigned int us) {
    us *= 60;
    while (us-- > 0) {
        ;
    }
}

void sci_rx_set_callback(int ch, SCI_CALLBACK callback)
{
    sci_callback[ch] = callback;
}

void sci_tx_enable(int ch) {
    volatile struct st_scifa *sci = SCI[ch];
    sci->SCR.WORD |= 0xa0;  /* TIE and TE set */
}

void sci_tx_disable(int ch) {
    volatile struct st_scifa *sci = SCI[ch];
    sci->SCR.WORD &= ~0xa0; /* TIE and TE clear */
}

#if defined(RZA2M_SCI_INT_ENABLE)
static void sci_isr_rx(int ch) {
    int i;
    uint8_t d;
    volatile struct st_scifa *sci = SCI[ch];
    if (((sci->FSR.WORD & 0x9c) != 0) || (sci->LSR.BIT.ORER == 1)) {
        sci->SCR.BIT.RE = 0;
        sci->FCR.BIT.RFRST = 1;
        sci->FCR.BIT.RFRST = 0;
        sci->FSR.WORD &= ~0x9c;
        sci->LSR.BIT.ORER = 0;
        sci->SCR.BIT.RE = 1;
        return;
    }
    while (sci->FSR.BIT.RDF == 0) {
        ;
    }
    d = sci->FRDR.BYTE;
    sci->FSR.BIT.RDF = 0;;
    if (sci_callback[ch]) {
        if ((*sci_callback[ch])(d)) {
            return;
        }
    }
    if (rx_fifo[ch].len < SCI_BUF_SIZE) {
        i = rx_fifo[ch].head;
        rx_fifo[ch].buff[i++] = d;
        rx_fifo[ch].head = i % SCI_BUF_SIZE;
        rx_fifo[ch].len++;
    }
}

void sci_isr_er(int ch) {
    volatile struct st_scifa *sci = SCI[ch];
    if (((sci->FSR.WORD & 0x9c) != 0) || (sci->LSR.BIT.ORER == 1)) {
        sci->SCR.BIT.RE = 0;
        sci->FCR.BIT.RFRST = 1;
        sci->FCR.BIT.RFRST = 0;
        sci->FSR.WORD &= ~0x9c;
        sci->LSR.BIT.ORER = 0;
        sci->SCR.BIT.RE = 1;
    }
}

static void sci_isr_tx(int ch) {
    int i;
    volatile struct st_scifa *sci = SCI[ch];
    rz_disable_irq();
    if (!tx_fifo[ch].busy) {
        sci->SCR.WORD &= ~0x00a0;
        goto sci_isr_tx_exit;
    }
    if (tx_fifo[ch].len != 0) {
        i = tx_fifo[ch].tail;
        while (sci->FSR.BIT.TDFE == 0) {
            /* Wait */
        }
        sci->FTDR.BYTE = tx_fifo[ch].buff[i++];
        sci->FSR.WORD &= ~0x0060;
        tx_fifo[ch].len--;
        tx_fifo[ch].tail = i % SCI_BUF_SIZE;
    } else {
        if (sci->FSR.BIT.TEND) {
            sci->SCR.WORD &= ~0x00a0;
            tx_fifo[ch].busy = 0;
        }
    }
sci_isr_tx_exit:
    rz_enable_irq();
#if 0
    if (tx_fifo[ch].len != 0) {
        i = tx_fifo[ch].tail;
        sci->TDR = tx_fifo[ch].buff[i++];
        tx_fifo[ch].len--;
        tx_fifo[ch].tail = i % SCI_BUF_SIZE;
    } else {
        tx_fifo[ch].busy = 0;
    }
#endif
}

void sci_isr_te(int ch) {
    volatile struct st_scifa *sci = SCI[ch];
    rz_disable_irq();
    //if (!tx_fifo[ch].busy)
    //    goto sci_isr_te_exit;
    sci->SCR.WORD &= ~0xa4; /* TIE, TE and TEI reset */
    if (tx_fifo[ch].len == 0) {
        tx_fifo[ch].busy = 0;
    } else {
        sci->SCR.WORD |= 0xa0;  /* TIE and TE set */
    }
//sci_isr_te_exit:
    rz_enable_irq();
}
#endif

uint8_t sci_rx_ch(int ch) {
    uint8_t c;
    int i;
#if defined(RZA2M_SCI_INT_ENABLE)
    if (rx_fifo[ch].len) {
        //GIC_DisableIRQ(sci_irqn[ch][0]);
        rz_disable_irq();
        i = rx_fifo[ch].tail;
        c = rx_fifo[ch].buff[i++];
        rx_fifo[ch].tail = i % SCI_BUF_SIZE;
        rx_fifo[ch].len--;
        rz_enable_irq();
        //GIC_EnableIRQ(sci_irqn[ch][0]);
    } else {
        c = 0;
    }
#else
    volatile struct st_scifa *sci = SCI[ch];
    if (((sci->FSR.WORD & 0x9c) != 0) || (sci->LSR.BIT.ORER == 1)) {
        sci->SCR.BIT.RE = 0;
        sci->FCR.BIT.RFRST = 1;
        sci->FCR.BIT.RFRST = 0;
        sci->FSR.WORD &= ~0x9c;
        sci->LSR.BIT.ORER = 0;
        sci->SCR.BIT.RE = 1;
        return 0;
    }
    while (sci->FSR.BIT.RDF == 0) {
        ;
    }
    c = sci->FRDR.BYTE;
    sci->FSR.BIT.RDF;
#endif
    return c;
}

int sci_rx_any(int ch) {
#if defined(RZA2M_SCI_INT_ENABLE)
    return (int)(rx_fifo[ch].head != rx_fifo[ch].tail);
#else
    volatile struct st_scifa *sci = SCI[ch];
    if (sci->FSR.BIT.RDF == 0) {
        return 0;
    } else {
        return 1;
    }
#endif
}

void sci_tx_ch(int ch, uint8_t c) {
    int i;
    volatile struct st_scifa *sci = SCI[ch];
#if defined(RZA2M_SCI_INT_ENABLE)
#if 0
    while (tx_fifo[ch].len == SCI_BUF_SIZE) {
        rz_disable_irq();
        i = tx_fifo[ch].tail;
        sci->FTDR.BYTE = tx_fifo[ch].buff[i++];
        tx_fifo[ch].len--;
        tx_fifo[ch].tail = i % SCI_BUF_SIZE;
        rz_enable_irq();
    }
#endif
    rz_disable_irq();
    if (!tx_fifo[ch].busy) {
        tx_fifo[ch].busy = 1;
        sci->SCR.WORD |= 0x00a0;    /* TIE and TE set */
    }
    i = tx_fifo[ch].head;
    tx_fifo[ch].buff[i++] = c;
    tx_fifo[ch].head = i % SCI_BUF_SIZE;
    tx_fifo[ch].len++;
    rz_enable_irq();
#else
    while (sci->FSR.BIT.TDFE == 0) {
        /* Wait */
    }
    sci->FTDR.BYTE = c;
    sci->FSR.WORD &= ~0x0060u;
#endif
}

int sci_tx_wait(int ch) {
    volatile struct st_scifa *sci = SCI[ch];
    /* TDFE - TDRE(RX) */
    return (sci->FSR.WORD & 0x10)? 1:0;
}

void sci_tx_str(int ch, uint8_t *p) {
    uint8_t c;
    while ((c = *p++) != 0) {
        sci_tx_ch(ch, c);
    }
}

#if defined(RZA2M_SCI_INT_ENABLE)
static void sci_fifo_init(int ch) {
    tx_fifo[ch].head = 0;
    tx_fifo[ch].tail = 0;
    tx_fifo[ch].len = 0;
    tx_fifo[ch].busy = 0;
    rx_fifo[ch].head = 0;
    rx_fifo[ch].tail = 0;
    rx_fifo[ch].len = 0;
    rx_fifo[ch].busy = 0;
}
#endif

void sci_module(int ch, int flag) {
    switch (ch) {
    case 0:
        CPG.STBCR4.BIT.MSTP47 = flag;
        break;
    case 1:
        CPG.STBCR4.BIT.MSTP46 = flag;
        break;
    case 2:
        CPG.STBCR4.BIT.MSTP45 = flag;
        break;
    case 3:
        CPG.STBCR4.BIT.MSTP44 = flag;
        break;
    case 4:
        CPG.STBCR4.BIT.MSTP43 = flag;
        break;
    default:
        break;
    }
}

void sci_module_start(int ch) {
    sci_module(ch, 0);
}

void sci_module_stop(int ch) {
    sci_module(ch, 1);
}

void sci_set_baud(int ch, int baud) {
    volatile struct st_scifa *sci = SCI[ch];
    uint32_t idx;
    uint32_t brr;
    uint32_t mddr;
    uint32_t wk_data = 8;
    float wk_error;
    if (baud < 0) {
        return;
    }
    for (idx = 0; idx <= 4; idx++ ) {
        brr = SCI_PCLK / (wk_data * baud);
        if ((brr <= 255) || (idx == 4)) {
            if (brr > 255) {
                brr = 255;
            }
            wk_error = (float)SCI_PCLK / (float)(wk_data * baud * (brr + 1));
            mddr = (uint32_t)((256.0f / wk_error) + 0.5f);
            if (mddr < 128) {
                mddr = 128; /* MDDR >= 128 only when idx is 4*/
            }
            /* idx= 0: BGDM=1 ABCS0=1 CKS=0 */
            /*      1: BGDM=0 ABCS0=0 CKS=0 */
            /*      2: BGDM=0 ABCS0=0 CKS=1 */
            /*      3: BGDM=1 ABCS0=1 CKS=3 (Do not use CKS=2) */
            /*      4: BGDM=0 ABCS0=0 CKS=3 */
            if ((idx == 0) || (idx == 3)) {
                sci->SEMR.BIT.BGDM  = 0x1;     /* Baud rate generator double-speed mode */
                sci->SEMR.BIT.ABCS0 = 0x1;     /* 8 times the transfer rate as the base clock */
                sci->SMR.BIT.CKS    = idx;     /* Clock Select  0(1/1), 1(1/4), 2(1/16), 3(1/64) */
            } else {
                sci->SEMR.BIT.BGDM  = 0x0;     /* Baud rate generator normal mode */
                sci->SEMR.BIT.ABCS0 = 0x0;     /* 16 times the transfer rate as the base clock */
                sci->SMR.BIT.CKS    = idx - 1; /* Clock Select  0(1/1), 1(1/4), 2(1/16), 3(1/64) */
            }
            sci->SEMR.BIT.MDDRS = 0x0;                   /* Select BRR register */
            sci->BRR_MDDR.BRR.BYTE = (uint8_t)brr;       /* Bit Rate */
            sci->SEMR.BIT.BRME = 0x0;                    /* Bit rate modulation is disabled */
            if (mddr <= 255) {
                sci->SEMR.BIT.MDDRS = 0x1;               /* Select MDDR register */
                sci->BRR_MDDR.MDDR.BYTE = (uint8_t)mddr; /* Modulation Duty */
                sci->SEMR.BIT.BRME = 0x1;                /* Bit rate modulation is enabled */
            }
            break;
        }
    }

//    if (baud == 0) {
//        sci->SMR.WORD &= ~0x03; // PCLK/1
//        sci->SEMR.BYTE |= 0x50; // BGDM and ABCS
//        sci->BRR_MDDR.BRR.BYTE = (uint8_t)((int)SCI_PCLK / SCI_DEFAULT_BAUD / 32 - 1);
//        //sci->MDDR = (uint8_t)((((int)(sci->BRR) + 1) * SCI_DEFAULT_BAUD * 32 * 256) / PCLK);
//    } else if (baud > 19200) {
//        sci->SMR.WORD &= ~0x03; // PCLK/1
//        sci->SEMR.BYTE |= 0x50; //  BGDM and ABCS
//        sci->BRR_MDDR.BRR.BYTE = (uint8_t)((int)SCI_PCLK / baud / 8 - 1);
//        //sci->MDDR = (uint8_t)((((int)(sci->BRR) + 1) * baud * 32 * 256) / PCLK);
//    } else if (baud > 2400) {
//        sci->SMR.WORD &= ~0x03;
//        sci->SMR.WORD |= 0x02;  // PCLK/16
//        sci->SEMR.BYTE |= 0x50; // BGDM and ABCS
//        sci->BRR_MDDR.BRR.BYTE = (uint8_t)((int)SCI_PCLK / baud / 128 - 1);
//        //sci->MDDR = (uint8_t)((((int)(sci->BRR) + 1) * baud * 32 * 256) / PCLK);
//    } else {
//        sci->SMR.WORD &= ~0x03;
//        sci->SMR.WORD |= 0x03;  // PCLK/64
//        sci->SEMR.BYTE |= 0x50; // BGDM and ABCS
//        sci->BRR_MDDR.BRR.BYTE = (uint8_t)((int)SCI_PCLK / baud / 512 - 1);
//        //sci->MDDR = (uint8_t)((((int)(sci->BRR) + 1) * baud * 32 * 256) / PCLK);
//    }
}

/*
 * bits: 7, 8
 * parity: none:0, odd:1, even:2
 */
void sci_init_with_pins(int ch, int tx_pin, int rx_pin, int baud, int bits, int parity, int stop, int flow) {
    uint16_t smr = 0;
    volatile struct st_scifa *sci = SCI[ch];

    if (!sci_init_flag[ch]) {
#if defined(RZA2M_SCI_INT_ENABLE)
        sci_fifo_init(ch);
        sci_callback[ch] = 0;
#endif
    }
    rz_disable_irq();
    sci_module_start(ch);
    _gpio_mode_input(rx_pin);
    _gpio_mode_af(tx_pin, 4);
    _gpio_mode_af(rx_pin, 4);
    sci->SCR.WORD = 0;
    if (bits == 7) {
        smr |= 0x40;
    } else {
        smr &= ~0x40;
    }
    if (parity != 0) {
        smr |= 0x20;
    } else {
        smr &= ~0x20;
    }
    if (parity == 1) {
        smr |= 0x10;
    } else {
        smr &= ~0x10;
    }
    if (stop == 2) {
        smr |= 0x80;
    } else {
        smr &= ~0x80;
    }
    sci->SMR.WORD = smr;
    sci_set_baud(ch, baud);
    sci->FCR.WORD = 0x0030;
    delay_us(10);
#if defined(RZA2M_SCI_INT_ENABLE)
    sci->SCR.WORD = 0xd0;
#else
    sci->SCR.WORD = 0x30;
#endif
#if defined(RZA2M_SCI_INT_ENABLE)
    InterruptHandlerRegister(sci_irqn[ch][0], sci_isr[ch][0]);
    InterruptHandlerRegister(sci_irqn[ch][1], sci_isr[ch][1]);
    InterruptHandlerRegister(sci_irqn[ch][2], sci_isr[ch][2]);
    //InterruptHandlerRegister(sci_irqn[ch][3], sci_isr[ch][3]);
    GIC_SetPriority(sci_irqn[ch][0], SCI_DEFAULT_PRIORITY);
    GIC_SetPriority(sci_irqn[ch][1], SCI_DEFAULT_PRIORITY);
    GIC_SetPriority(sci_irqn[ch][2], SCI_DEFAULT_PRIORITY);
    //GIC_SetPriority(sci_irqn[ch][3], SCI_DEFAULT_PRIORITY);
    GIC_EnableIRQ(sci_irqn[ch][0]);
    GIC_EnableIRQ(sci_irqn[ch][1]);
    GIC_EnableIRQ(sci_irqn[ch][2]);
    //GIC_EnableIRQ(sci_irqn[ch][3]);
#endif
    rz_enable_irq();
    if (!sci_init_flag[ch]) {
        sci_init_flag[ch] = true;
    }
}

void sci_init(int ch, int baud, int bits, int parity, int stop, int flow) {
    int tx_pin = (int)sci_tx_pins[ch];
    int rx_pin = (int)sci_rx_pins[ch];
    if ((tx_pin != 0xff) && (rx_pin != 0xff)) {
        sci_init_with_pins(ch, tx_pin, rx_pin, baud, bits, parity, stop, flow);
    }
}

void sci_init_default(int ch, int baud) {
    sci_init(ch, baud, 8, 0, 1, 0);
}

void sci_deinit(int ch) {
    rz_disable_irq();
    sci_init_flag[ch] = false;
    sci_module_stop(ch);
#if defined(RZA2M_SCI_INT_ENABLE)
    GIC_DisableIRQ(sci_irqn[ch][0]);
    GIC_DisableIRQ(sci_irqn[ch][1]);
    GIC_DisableIRQ(sci_irqn[ch][2]);
    //GIC_DisableIRQ(sci_irqn[ch][3]);
#endif
    sci_callback[ch] = 0;
    rz_enable_irq();
}

#if defined(RZA2M_SCI_INT_ENABLE)
/* rx interrupt */
void sci_isr_rx0(void) {
    sci_isr_rx(0);
}
void sci_isr_rx1(void) {
    sci_isr_rx(1);
}
void sci_isr_rx2(void) {
    sci_isr_rx(2);
}
void sci_isr_rx3(void) {
    sci_isr_rx(3);
}
void sci_isr_rx4(void) {
    sci_isr_rx(4);
}

/* tx interrupt */
void sci_isr_tx0(void) {
    sci_isr_tx(0);
}
void sci_isr_tx1(void) {
    sci_isr_tx(1);
}
void sci_isr_tx2(void) {
    sci_isr_tx(2);
}
void sci_isr_tx3(void) {
    sci_isr_tx(3);
}
void sci_isr_tx4(void) {
    sci_isr_tx(4);
}

/* er interrupt */
void sci_isr_er0(void) {
    sci_isr_er(0);
}
void sci_isr_er1(void) {
    sci_isr_er(1);
}
void sci_isr_er2(void) {
    sci_isr_er(2);
}
void sci_isr_er3(void) {
    sci_isr_er(3);
}
void sci_isr_er4(void) {
    sci_isr_er(4);
}

/* te interrupt */
void sci_isr_te0(void) {
    sci_isr_te(0);
}
void sci_isr_te1(void) {
    sci_isr_te(1);
}
void sci_isr_te2(void) {
    sci_isr_te(2);
}
void sci_isr_te3(void) {
    sci_isr_te(3);
}
void sci_isr_te4(void) {
    sci_isr_te(4);
}
#endif
