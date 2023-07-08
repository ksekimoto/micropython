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
#include <string.h>
#include "common.h"
#include "iodefine.h"
// #include "pendsv.h"
#include "rz_sci.h"
#include "rz_gpio.h"
#include "rz_utils.h"
#include "rz_sci.h"

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

#define SCI_TX_BUF_SIZE 256
#define SCI_RX_BUF_SIZE 1024

#define SCI_PCLK    66000000
#define SCI_CH_MAX 5
#define SCI_BUF_SIZE 4096
#define SCI_DEFAULT_PRIORITY 4
#define SCI_DEFAULT_BAUD    115200

static volatile struct st_scifa *SCI[SCI_CH_MAX] = {
    (volatile struct st_scifa *)0xE8007000, /* sci 0 */
    (volatile struct st_scifa *)0xE8007800, /* sci 1 */
    (volatile struct st_scifa *)0xE8008000, /* sci 2 */
    (volatile struct st_scifa *)0xE8008800, /* sci 3 */
    (volatile struct st_scifa *)0xE8009000, /* sci 4 */
};

static const uint8_t sci_tx_pins[SCI_CH_MAX] = {
    0x42,   /* ch 0 P42 */
    0x73,   /* ch 1 P73 */
    0xe2,   /* ch 2 PE2 */
    0x63,   /* ch 3 P63 */
    0x90    /* ch 4 P90 */
};

static const uint8_t sci_rx_pins[SCI_CH_MAX] = {
    0x41,   /* ch 0 P41 */
    0x71,   /* ch 1 P71 */
    0xe1,   /* ch 2 PE1 */
    0x62,   /* ch 3 P62 */
    0x91    /* ch 4 P91*/
};

static const IRQn_Type sci_irqn[SCI_CH_MAX][4] = {
    {RXI0_IRQn, TXI0_IRQn, ERI0_IRQn, TEI0_IRQn},
    {RXI1_IRQn, TXI1_IRQn, ERI1_IRQn, TEI1_IRQn},
    {RXI2_IRQn, TXI2_IRQn, ERI2_IRQn, TEI2_IRQn},
    {RXI3_IRQn, TXI3_IRQn, ERI3_IRQn, TEI3_IRQn},
    {RXI4_IRQn, TXI4_IRQn, ERI4_IRQn, TEI4_IRQn},
};

typedef void (*SCI_ISR)(void);

static const SCI_ISR sci_isr[SCI_CH_MAX][4] = {
    {sci_isr_rx0, sci_isr_tx0, sci_isr_er0, sci_isr_te0},
    {sci_isr_rx1, sci_isr_tx1, sci_isr_er1, sci_isr_te1},
    {sci_isr_rx2, sci_isr_tx2, sci_isr_er2, sci_isr_te2},
    {sci_isr_rx3, sci_isr_tx3, sci_isr_er3, sci_isr_te3},
    {sci_isr_rx4, sci_isr_tx4, sci_isr_er4, sci_isr_te4},
};


// struct SCI_FIFO {
//     int tail, head, len, busy;
//     uint8_t buff[SCI_BUF_SIZE];
// };

// static bool sci_init_flag[SCI_CH_MAX] = {
//     false, false, false, false, false,
// };
// static sci_cb sci_cb[SCI_CH_MAX] = {
//     0, 0, 0, 0, 0,
// };

// static volatile struct SCI_FIFO tx_fifo[SCI_CH_MAX];
// static volatile struct SCI_FIFO rx_fifo[SCI_CH_MAX];

typedef struct _sci_fifo {
    volatile uint32_t tail, head, len, busy;
    uint8_t *bufp;
    uint32_t size;
} sci_fifo;

static uint32_t rz_sci_init_flag[] = {
    0,
    0,
    0,
    0,
    0,
};
static SCI_CB sci_cb[] = {
    (SCI_CB)0,
    (SCI_CB)0,
    (SCI_CB)0,
    (SCI_CB)0,
    (SCI_CB)0,
};
// 9bit transfer flag is defined although RZA2M scifa doesn't support 9bit transfer.
static uint8_t ch_9bit[] = {
    0,
    0,
    0,
    0,
    0,
};

static uint8_t tx_buf[SCI_CH_MAX][SCI_TX_BUF_SIZE];
static uint8_t rx_buf[SCI_CH_MAX][SCI_RX_BUF_SIZE];
static volatile sci_fifo tx_fifo[SCI_CH_MAX];
static volatile sci_fifo rx_fifo[SCI_CH_MAX];

static void delay_us(volatile unsigned int us) {
    us *= 60;
    while (us-- > 0) {
        ;
    }
}

void rz_sci_module(int ch, int flag) {
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

static void rz_sci_module_start(uint32_t ch) {
    rz_sci_module(ch, 0);
}

static void rz_sci_module_stop(uint32_t ch) {
    rz_sci_module(ch, 1);
}

void rz_sci_rx_set_callback(int ch, SCI_CB cb) {
    sci_cb[ch] = cb;
}

void rz_sci_tx_enable(int ch) {
    volatile struct st_scifa *sci = SCI[ch];
    sci->SCR.WORD |= 0xa0;  /* TIE and TE set */
}

void rz_sci_tx_disable(int ch) {
    volatile struct st_scifa *sci = SCI[ch];
    sci->SCR.WORD &= ~0xa0; /* TIE and TE clear */
}

static void rz_sci_isr_rx(uint32_t ch) {
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
    uint16_t d;
    if (ch_9bit[ch]) {
        d = (uint16_t)sci->FRDR.BYTE;
    } else {
        d = (uint16_t)sci->FRDR.BYTE;
    }
    sci->FSR.BIT.RDF = 0;
    ;
    if (sci_cb[ch]) {
        if ((*sci_cb[ch])(ch, (int)d)) {
            // return;
        }
    }
    uint32_t size = rx_fifo[ch].size;
    sci_fifo *rxfifo = (sci_fifo *)&rx_fifo[ch];
    if (rxfifo->len < size) {
        uint32_t i = rxfifo->head;
        if (ch_9bit[ch]) {
            *(uint16_t *)(rxfifo->bufp + i) = (uint16_t)d;
            i += 2;
            rxfifo->len += 2;
        } else {
            *(rxfifo->bufp + i) = (uint8_t)d;
            i++;
            rxfifo->len++;
        }
        rxfifo->head = i % size;
        // if (m_rts_pin[idx] != PIN_END) {
        //     if (rxfifo->len > (size - rz_sci_FLOW_START_NUM)) {
        //         ra_gpio_write(m_rts_pin[idx], 1);
        //     }
        // }
    }
}

static void rz_sci_isr_er(uint32_t ch) {
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

static void rz_sci_isr_tx(uint32_t ch) {
    volatile struct st_scifa *sci = SCI[ch];
    uint32_t size = tx_fifo[ch].size;
    sci_fifo *txfifo = (sci_fifo *)&tx_fifo[ch];
    if (txfifo->len != 0) {
        uint32_t i = txfifo->tail;
        if (ch_9bit[ch]) {
            sci->FTDR.BYTE = (uint8_t)*(txfifo->bufp + i);
            i += 2;
            txfifo->len -= 2;
        } else {
            sci->FTDR.BYTE = (uint8_t)*(txfifo->bufp + i);
            i++;
            txfifo->len--;
        }
        txfifo->tail = i % size;
    } else {
        /* tx_fifo[idx].len == 0 */
        /* after transfer completed */
        uint16_t scr = sci->SCR.WORD;
        scr &= (uint16_t) ~0x0080; /* TIE disable */
        scr |= (uint16_t)0x0004;  /* TEIE enable */
        sci->SCR.WORD = scr;
    }
}

void rz_sci_isr_te(uint32_t ch) {
    volatile struct st_scifa *sci = SCI[ch];
    tx_fifo[ch].busy = 0;
    sci->SCR.WORD &= (uint16_t) ~0x0084;    /* TIE and TEIE disable */
}

int rz_sci_rx_ch(uint32_t ch) {
    uint16_t c;
    uint32_t size = rx_fifo[ch].size;
    sci_fifo *rxfifo = (sci_fifo *)&rx_fifo[ch];
    if (rxfifo->len) {
        // uint32_t state = rz_disable_irq();
        uint32_t i = rxfifo->tail;
        if (ch_9bit[ch]) {
            c = *(uint16_t *)(rxfifo->bufp + i);
            i += 2;
            rxfifo->len -= 2;
        } else {
            c = (uint16_t)*(rxfifo->bufp + i);
            i++;
            rxfifo->len--;
        }
        rxfifo->tail = i % size;
        // if (m_rts_pin[idx] != PIN_END) {
        //     if (rxfifo->len <= (size - rz_sci_FLOW_START_NUM)) {
        //         ra_gpio_write(m_rts_pin[idx], 0);
        //     }
        // }
        // rz_enable_irq(state);
    } else {
        c = 0;
    }
    return (int)c;
}

int rz_sci_rx_any(uint32_t ch) {
    return (int)(rx_fifo[ch].len != 0);
}

void rz_sci_tx_ch(uint32_t ch, int c) {
    volatile struct st_scifa *sci = SCI[ch];
    uint32_t size = tx_fifo[ch].size;
    sci_fifo *txfifo = (sci_fifo *)&tx_fifo[ch];
    while (tx_fifo[ch].len == size) {
    }
    uint32_t state = rz_disable_irq();
    uint32_t i = tx_fifo[ch].head;
    if (ch_9bit[ch]) {
        *(uint16_t *)(txfifo->bufp + i) = (uint16_t)c;
        i += 2;
        txfifo->len += 2;
    } else {
        *(txfifo->bufp + i) = (uint8_t)c;
        i++;
        txfifo->len++;
    }
    txfifo->head = i % size;
    if (!txfifo->busy) {
        txfifo->busy = 1;
        uint16_t scr = sci->SCR.WORD;
        if ((scr & 0x00a0) != 0) {
            sci->SCR.WORD &= ~0x00a0;
        }
        sci->SCR.WORD |= 0x00a0;    /* TIE and TE enable */
    }
    rz_enable_irq(state);
}

int rz_sci_tx_wait(uint32_t ch) {
    return (int)(tx_fifo[ch].len != (tx_fifo[ch].size - 1));
}

void rz_sci_tx_break(uint32_t ch) {
    #if RZ_TODO
    volatile struct st_scifa *sci = SCI[ch];
    uint16_t scr = sci->SCR.WORD;
    uint16_t smr = sci->SMR.WORD;
    sci->SCR.WORD = 0;
    while (sci->SCR.WORD != 0) {
        ;
    }
    sci->SMR.WORD |= 0x0008;    /* stop bit length = 2 */
    sci->SCR.WORD = scr;
    sci->FRDR.BYTE = 0;
    while (sci->FSR.BIT.TEND == 0) {
        ;
    }
    sci->SMR.WORD = smr;
    #endif
    return;
}

void rz_sci_tx_str(uint32_t ch, uint8_t *p) {
    int c;
    if (ch_9bit[ch]) {
        uint16_t *q = (uint16_t *)p;
        while ((c = *q++) != 0) {
            rz_sci_tx_ch(ch, (int)c);
        }
    } else {
        while ((c = (int)*p++) != 0) {
            rz_sci_tx_ch(ch, (int)c);
        }
    }
}

static void rz_sci_fifo_set(sci_fifo *fifo, uint8_t *bufp, uint32_t size) {
    fifo->head = 0;
    fifo->tail = 0;
    fifo->len = 0;
    fifo->busy = 0;
    fifo->bufp = bufp;
    fifo->size = size;
    // memset((void *)bufp, 0, (size_t)size);
}

void rz_sci_txfifo_set(uint32_t ch, uint8_t *bufp, uint32_t size) {
    sci_fifo *fifo = (sci_fifo *)&tx_fifo[ch];
    rz_sci_fifo_set(fifo, bufp, size);
}

void rz_sci_rxfifo_set(uint32_t ch, uint8_t *bufp, uint32_t size) {
    sci_fifo *fifo = (sci_fifo *)&rx_fifo[ch];
    rz_sci_fifo_set(fifo, bufp, size);
}

static void rz_sci_fifo_init(uint32_t ch) {
    rz_sci_txfifo_set(ch, (uint8_t *)&tx_buf[ch][0], SCI_TX_BUF_SIZE);
    rz_sci_rxfifo_set(ch, (uint8_t *)&rx_buf[ch][0], SCI_RX_BUF_SIZE);
}

void rz_sci_set_baud(uint32_t ch, uint32_t baud) {
    volatile struct st_scifa *sci = SCI[ch];
    uint32_t idx;
    uint32_t brr;
    uint32_t mddr;
    uint32_t wk_data = 8;
    float wk_error;
    if (baud < 0) {
        return;
    }
    for (idx = 0; idx <= 4; idx++) {
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
                sci->SEMR.BIT.BGDM = 0x1;      /* Baud rate generator double-speed mode */
                sci->SEMR.BIT.ABCS0 = 0x1;     /* 8 times the transfer rate as the base clock */
                sci->SMR.BIT.CKS = idx;        /* Clock Select  0(1/1), 1(1/4), 2(1/16), 3(1/64) */
            } else {
                sci->SEMR.BIT.BGDM = 0x0;      /* Baud rate generator normal mode */
                sci->SEMR.BIT.ABCS0 = 0x0;     /* 16 times the transfer rate as the base clock */
                sci->SMR.BIT.CKS = idx - 1;    /* Clock Select  0(1/1), 1(1/4), 2(1/16), 3(1/64) */
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
void rz_sci_init_with_flow(uint32_t ch, uint32_t tx_pin, uint32_t rx_pin, uint32_t baud, uint32_t bits, uint32_t parity, uint32_t stop, uint32_t flow, uint32_t cts_pin, uint32_t rts_pin) {
    uint16_t smr = 0;
    volatile struct st_scifa *sci = SCI[ch];
    if (rz_sci_init_flag[ch] == 0) {
        rz_sci_fifo_init(ch);
        sci_cb[ch] = 0;
        rz_sci_init_flag[ch]++;
    } else {
        rz_sci_init_flag[ch]++;
        return;
    }
    uint32_t state = rz_disable_irq();
    rz_sci_module_start(ch);
    rz_gpio_mode_input(rx_pin);
    switch (ch) {
        case 0:
            rz_gpio_mode_af(tx_pin, 1);
            rz_gpio_mode_af(rx_pin, 1);
            break;
        case 1:
            rz_gpio_mode_af(tx_pin, 4);
            rz_gpio_mode_af(rx_pin, 4);
            break;
        case 2:
            rz_gpio_mode_af(tx_pin, 3);
            rz_gpio_mode_af(rx_pin, 3);
            break;
        case 3:
            rz_gpio_mode_af(tx_pin, 3);
            rz_gpio_mode_af(rx_pin, 3);
            break;
        case 4:
            rz_gpio_mode_af(tx_pin, 4);
            rz_gpio_mode_af(rx_pin, 4);
            break;
        default:
            break;
    }
    sci->SCR.WORD = 0;
    while (sci->SCR.WORD != 0) {
        ;
    }
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
    #if RZ_TODO
    if (flow) {
    }
    #endif
    sci->SMR.WORD = smr;
    rz_sci_set_baud(ch, baud);
    sci->FCR.WORD = 0x0030;
    delay_us(10);
    sci->SCR.WORD = 0x50;
    InterruptHandlerRegister(sci_irqn[ch][0], sci_isr[ch][0]);
    InterruptHandlerRegister(sci_irqn[ch][1], sci_isr[ch][1]);
    InterruptHandlerRegister(sci_irqn[ch][2], sci_isr[ch][2]);
    InterruptHandlerRegister(sci_irqn[ch][3], sci_isr[ch][3]);
    GIC_SetPriority(sci_irqn[ch][0], SCI_DEFAULT_PRIORITY);
    GIC_SetPriority(sci_irqn[ch][1], SCI_DEFAULT_PRIORITY);
    GIC_SetPriority(sci_irqn[ch][2], SCI_DEFAULT_PRIORITY);
    GIC_SetPriority(sci_irqn[ch][3], SCI_DEFAULT_PRIORITY);
    GIC_EnableIRQ(sci_irqn[ch][0]);
    GIC_EnableIRQ(sci_irqn[ch][1]);
    GIC_EnableIRQ(sci_irqn[ch][2]);
    GIC_EnableIRQ(sci_irqn[ch][3]);
    rz_enable_irq(state);
}

void rz_sci_init(uint32_t ch, uint32_t tx_pin, uint32_t rx_pin, uint32_t baud, uint32_t bits, uint32_t parity, uint32_t stop, uint32_t flow) {
    rz_sci_init_with_flow(ch, tx_pin, rx_pin, baud, bits, parity, stop, flow, PIN_END, PIN_END);
}

void rz_sci_init_default(uint32_t ch, uint32_t baud) {
    rz_sci_init(ch, sci_tx_pins[ch], sci_rx_pins[ch], baud, 8, 0, 1, 0);
}

void rz_sci_deinit(uint32_t ch) {
    uint32_t state = rz_disable_irq();
    if (rz_sci_init_flag[ch] != 0) {
        rz_sci_init_flag[ch]--;
        if (rz_sci_init_flag[ch] == 0) {
            rz_sci_module_stop(ch);
            GIC_DisableIRQ(sci_irqn[ch][0]);
            GIC_DisableIRQ(sci_irqn[ch][1]);
            GIC_DisableIRQ(sci_irqn[ch][2]);
            GIC_DisableIRQ(sci_irqn[ch][3]);
            sci_cb[ch] = 0;
        }
    }
    rz_enable_irq(state);
}

/* rx interrupt */
void sci_isr_rx0(void) {
    rz_sci_isr_rx(0);
}
void sci_isr_rx1(void) {
    rz_sci_isr_rx(1);
}
void sci_isr_rx2(void) {
    rz_sci_isr_rx(2);
}
void sci_isr_rx3(void) {
    rz_sci_isr_rx(3);
}
void sci_isr_rx4(void) {
    rz_sci_isr_rx(4);
}

/* tx interrupt */
void sci_isr_tx0(void) {
    rz_sci_isr_tx(0);
}
void sci_isr_tx1(void) {
    rz_sci_isr_tx(1);
}
void sci_isr_tx2(void) {
    rz_sci_isr_tx(2);
}
void sci_isr_tx3(void) {
    rz_sci_isr_tx(3);
}
void sci_isr_tx4(void) {
    rz_sci_isr_tx(4);
}

/* er interrupt */
void sci_isr_er0(void) {
    rz_sci_isr_er(0);
}
void sci_isr_er1(void) {
    rz_sci_isr_er(1);
}
void sci_isr_er2(void) {
    rz_sci_isr_er(2);
}
void sci_isr_er3(void) {
    rz_sci_isr_er(3);
}
void sci_isr_er4(void) {
    rz_sci_isr_er(4);
}

/* te interrupt */
void sci_isr_te0(void) {
    rz_sci_isr_te(0);
}
void sci_isr_te1(void) {
    rz_sci_isr_te(1);
}
void sci_isr_te2(void) {
    rz_sci_isr_te(2);
}
void sci_isr_te3(void) {
    rz_sci_isr_te(3);
}
void sci_isr_te4(void) {
    rz_sci_isr_te(4);
}
