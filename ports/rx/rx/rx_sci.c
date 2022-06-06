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

#include <stdbool.h>
#include <stdlib.h>
#include "common.h"
#include "iodefine.h"
#include "interrupt_handlers.h"
#include "pendsv.h"
#include "rx_sci.h"
#include "py/runtime.h"

typedef struct st_sci0 ST_SCI0;

static volatile ST_SCI0 *sci_regs[] = {
    (volatile ST_SCI0 *)0x8A000, /* sci 0 */
    (volatile ST_SCI0 *)0x8A020, /* sci 1 */
    (volatile ST_SCI0 *)0x8A040, /* sci 2 */
    (volatile ST_SCI0 *)0x8A060, /* sci 3 */
    (volatile ST_SCI0 *)0x8A080, /* sci 4 */
    (volatile ST_SCI0 *)0x8A0A0, /* sci 5 */
    (volatile ST_SCI0 *)0x8A0C0, /* sci 6 */
    (volatile ST_SCI0 *)0x8A0E0, /* sci 7 */
    (volatile ST_SCI0 *)0x8A100, /* sci 8 */
    (volatile ST_SCI0 *)0x8A120, /* sci 9 */
    (volatile ST_SCI0 *)0x8A140, /* sci 10 */
    (volatile ST_SCI0 *)0x8A160, /* sci 11 */
    (volatile ST_SCI0 *)0x8B300, /* sci 12 */
};

static const uint32_t ch_to_idx[] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
};

#if defined(RX65N)
static uint8_t ch_9bit[] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};
#endif

static const rx_af_pin_t rx_sci_tx_pins[] = {
    #if defined(RX63N)

    { AF_SCI1, 0, P20 },
    { AF_SCI2, 0, P32 },

    { AF_SCI1, 1, P16 },
    { AF_SCI1, 1, P26 },
    { AF_SCI1, 1, PF0 },

    { AF_SCI1, 2, P13 },
    { AF_SCI1, 2, P50 },

    { AF_SCI2, 3, P17 },
    { AF_SCI1, 3, P23 },

    { AF_SCI1, 4, PB1 },

    { AF_SCI1, 5, PA4 },
    { AF_SCI1, 5, PC3 },

    { AF_SCI1, 6, P00 },
    { AF_SCI1, 6, P32 },
    { AF_SCI2, 6, PB1 },

    { AF_SCI1, 7, P90 },

    { AF_SCI1, 8, PC7 },

    { AF_SCI1, 9, PB6 },

    { AF_SCI1, 10, P82 },

    #elif defined(RX65N)

    { AF_SCI1, 0, P20 },
    { AF_SCI2, 0, P32 },

    { AF_SCI1, 1, P16 },
    { AF_SCI1, 1, P26 },
    { AF_SCI1, 1, PF0 },

    { AF_SCI1, 2, P13 },
    { AF_SCI1, 2, P50 },

    { AF_SCI2, 3, P17 },
    { AF_SCI1, 3, P23 },

    { AF_SCI1, 4, PB1 },

    { AF_SCI1, 5, PA4 },
    { AF_SCI1, 5, PC3 },

    { AF_SCI1, 6, P00 },
    { AF_SCI1, 6, P32 },
    { AF_SCI2, 6, PB1 },

    { AF_SCI1, 7, P90 },

    { AF_SCI1, 8, PC7 },

    { AF_SCI1, 9, PB6 },

    { AF_SCI1, 10, P82 },

    #else
    #error "RX MCU Series is not specified."
    #endif
};
#define SCI_TX_PINS_SIZE sizeof(rx_sci_tx_pins) / sizeof(rx_af_pin_t)

static const rx_af_pin_t rx_sci_rx_pins[] = {
    #if defined(RX63N)

    { AF_SCI1, 0, P21 },
    { AF_SCI2, 0, P33 },

    { AF_SCI1, 1, P15 },
    { AF_SCI1, 1, P30 },
    { AF_SCI1, 1, PF2 },

    { AF_SCI1, 2, P12 },
    { AF_SCI1, 2, P52 },

    { AF_SCI2, 3, P16 },
    { AF_SCI1, 3, P25 },

    { AF_SCI1, 4, PB0 },

    { AF_SCI1, 5, PA2 },
    { AF_SCI1, 5, PA3 },
    { AF_SCI1, 5, PC2 },

    { AF_SCI1, 6, P01 },
    { AF_SCI1, 6, P33 },
    { AF_SCI2, 6, PB0 },

    { AF_SCI1, 7, P92 },

    { AF_SCI1, 8, PC6 },

    { AF_SCI1, 9, PB6 },

    { AF_SCI1, 10, P81 },

    #elif defined(RX65N)

    { AF_SCI1, 0, P21 },
    { AF_SCI2, 0, P33 },

    { AF_SCI1, 1, P15 },
    { AF_SCI1, 1, P30 },
    { AF_SCI1, 1, PF2 },

    { AF_SCI1, 2, P12 },
    { AF_SCI1, 2, P52 },

    { AF_SCI2, 3, P16 },
    { AF_SCI1, 3, P25 },

    { AF_SCI1, 4, PB0 },

    { AF_SCI1, 5, PA2 },
    { AF_SCI1, 5, PA3 },
    { AF_SCI1, 5, PC2 },

    { AF_SCI1, 6, P01 },
    { AF_SCI1, 6, P33 },
    { AF_SCI2, 6, PB0 },

    { AF_SCI1, 7, P92 },

    { AF_SCI1, 8, PC6 },

    { AF_SCI1, 9, PB6 },

    { AF_SCI1, 10, P81 },

    #else
    #error "RX MCU Series is not specified."
    #endif
};
#define SCI_RX_PINS_SIZE sizeof(rx_sci_rx_pins) / sizeof(rx_af_pin_t)

static const rx_af_pin_t rx_sci_cts_pins[] = {
    #if defined(RX63N)

    #elif defined(RX65N)

    #else
    #error "RX MCU Series is not specified."
    #endif
};
#define SCI_CTS_PINS_SIZE sizeof(rx_sci_cts_pins) / sizeof(rx_af_pin_t)

typedef struct _sci_fifo {
    volatile uint32_t tail, head, len, busy;
    uint8_t *bufp;
    uint32_t size;
} sci_fifo;

static uint32_t rx_sci_init_flag[SCI_IDX_MAX] = {
    0
};
static SCI_CB sci_cb[SCI_IDX_MAX];

static uint8_t tx_buf[SCI_IDX_MAX][SCI_TX_BUF_SIZE] __attribute__((aligned(16)));
static uint8_t rx_buf[SCI_IDX_MAX][SCI_RX_BUF_SIZE] __attribute__((aligned(16)));
static volatile sci_fifo tx_fifo[SCI_IDX_MAX];
static volatile sci_fifo rx_fifo[SCI_IDX_MAX];

static uint32_t m_cts_pin[] = {
    PIN_END,
    PIN_END,
    PIN_END,
    PIN_END,
    PIN_END,
    PIN_END,
    PIN_END,
    PIN_END,
    PIN_END,
    PIN_END,
};

static uint32_t m_rts_pin[] = {
    PIN_END,
    PIN_END,
    PIN_END,
    PIN_END,
    PIN_END,
    PIN_END,
    PIN_END,
    PIN_END,
    PIN_END,
    PIN_END,
};

static void delay_us(volatile unsigned int us) {
    us *= 60;
    while (us-- > 0) {
        ;
    }
}

bool rx_af_find_ch_af(rx_af_pin_t *af_pin, uint32_t size, uint32_t pin, uint32_t *ch, uint32_t *af) {
    bool find = false;
    uint32_t i;
    for (i = 0; i < size; i++) {
        if (af_pin->pin == pin) {
            find = true;
            *ch = af_pin->ch;
            *af = af_pin->af;
            break;
        }
        af_pin++;
    }
    return find;
}

static void rx_sci_tx_set_pin(uint32_t pin) {
    bool find = false;
    uint32_t ch;
    uint32_t af;
    find = rx_af_find_ch_af((rx_af_pin_t *)&rx_sci_tx_pins, SCI_TX_PINS_SIZE, pin, &ch, &af);
    if (find) {
        rx_gpio_config(pin, GPIO_MODE_AF_PP, 0, af);
    }
}

static void rx_sci_rx_set_pin(uint32_t pin) {
    bool find = false;
    uint32_t ch;
    uint32_t af;
    find = rx_af_find_ch_af((rx_af_pin_t *)&rx_sci_rx_pins, SCI_RX_PINS_SIZE, pin, &ch, &af);
    if (find) {
        rx_gpio_config(pin, GPIO_MODE_INPUT, 1, af);
    }
}

static void rx_sci_cts_set_pin(uint32_t pin) {
    bool find = false;
    uint32_t ch;
    uint32_t af;
    find = rx_af_find_ch_af((rx_af_pin_t *)&rx_sci_cts_pins, SCI_CTS_PINS_SIZE, pin, &ch, &af);
    if (find) {
        rx_gpio_config(pin, GPIO_MODE_INPUT, 1, af);
    }
}

void rx_sci_rx_set_callback(int ch, SCI_CB cb) {
    sci_cb[ch] = cb;
}

void rx_sci_rx_set_int(uint32_t ch, int flag) {
    int idx;
    int bit;
    #if defined(RX63N)
    idx = (214 + ch * 3) / 8;
    bit = (214 + ch * 3) & 7;
    #endif
    #if defined(RX65N)
    if (ch < 3) {
        idx = (58 + ch * 2) / 8;
        bit = (58 + ch * 2) & 7;
    } else if (ch < 7) {
        idx = (74 + ch * 2) / 8;
        bit = (74 + ch * 2) & 7;
    } else if (ch < 11) {
        idx = (84 + ch * 2) / 8;
        bit = (84 + ch * 2) & 7;
    } else {
        idx = (92 + ch * 2) / 8;
        bit = (92 + ch * 2) & 7;
    }
    #endif
    uint8_t mask = (uint8_t)(1 << bit);
    ICU.IER[idx].BYTE = (uint8_t)((ICU.IER[idx].BYTE & ~mask) | (flag << bit));
}

void rx_sci_rx_int_enable(uint32_t ch) {
    rx_sci_rx_set_int(ch, 1);
}

void rx_sci_rx_int_disable(uint32_t ch) {
    rx_sci_rx_set_int(ch, 0);
}

void rx_sci_tx_set_int(uint32_t ch, int flag) {
    int idx;
    int bit;
    #if defined(RX63N)
    idx = (215 + ch * 3) / 8;
    bit = (215 + ch * 3) & 7;
    #endif
    #if defined(RX65N)
    if (ch < 3) {
        idx = (59 + ch * 2) / 8;
        bit = (59 + ch * 2) & 7;
    } else if (ch < 7) {
        idx = (75 + ch * 2) / 8;
        bit = (75 + ch * 2) & 7;
    } else if (ch < 11) {
        idx = (85 + ch * 2) / 8;
        bit = (85 + ch * 2) & 7;
    } else {
        idx = (93 + ch * 2) / 8;
        bit = (93 + ch * 2) & 7;
    }
    #endif
    uint8_t mask = (uint8_t)(1 << bit);
    ICU.IER[idx].BYTE = (uint8_t)((ICU.IER[idx].BYTE & ~mask) | (flag << bit));
}

void rx_sci_tx_int_enable(uint32_t ch) {
    rx_sci_tx_set_int(ch, 1);
}

void rx_sci_tx_int_disable(uint32_t ch) {
    rx_sci_tx_set_int(ch, 0);
}

#if defined(RX63N)
void rx_sci_te_set_int(uint32_t ch, int flag) {
    int idx = (216 + ch * 3) / 8;
    int bit = (0 + ch * 3) & 7;
    uint8_t mask = (uint8_t)(1 << bit);
    ICU.IER[idx].BYTE = (uint8_t)((ICU.IER[idx].BYTE & ~mask) | (flag << bit));
}
#endif

#if defined(RX65N)
void rx_sci_te_set_int(uint32_t ch, int flag) {
    switch (ch) {
        case 0:
            ICU.GENBL0.BIT.EN0 = flag; // vec: 110
            break;
        case 1:
            ICU.GENBL0.BIT.EN2 = flag; // vec: 110
            break;
        case 2:
            ICU.GENBL0.BIT.EN4 = flag; // vec: 110
            break;
        case 3:
            ICU.GENBL0.BIT.EN6 = flag; // vec: 110
            break;
        case 4:
            ICU.GENBL0.BIT.EN8 = flag; // vec: 110
            break;
        case 5:
            ICU.GENBL0.BIT.EN10 = flag; // vec: 110
            break;
        case 6:
            ICU.GENBL0.BIT.EN12 = flag; // vec: 110
            break;
        case 7:
            ICU.GENBL0.BIT.EN14 = flag; // vec: 110
            break;
        case 8:
            ICU.GENBL1.BIT.EN24 = flag; // vec: 111
            break;
        case 9:
            ICU.GENBL1.BIT.EN26 = flag; // vec: 111
            break;
        case 10:
            ICU.GENAL0.BIT.EN8 = flag; // vec: 112
            break;
        case 11:
            ICU.GENAL0.BIT.EN12 = flag; // vec: 112
            break;
        case 12:
            ICU.GENBL0.BIT.EN16 = flag; // vec: 110
            break;
    }
}
#endif

void rx_sci_te_int_enable(uint32_t ch) {
    rx_sci_te_set_int(ch, 1);
}

void rx_sci_te_int_disable(uint32_t ch) {
    rx_sci_te_set_int(ch, 0);
}

#if defined(RX63N)
void rx_sci_er_set_int(uint32_t ch, int flag) {

}
#endif
#if defined(RX65N)
void rx_sci_er_set_int(uint32_t ch, int flag) {
    switch (ch) {
        case 0:
            ICU.GENBL0.BIT.EN1 = flag; // vec: 110
            break;
        case 1:
            ICU.GENBL0.BIT.EN3 = flag; // vec: 110
            break;
        case 2:
            ICU.GENBL0.BIT.EN5 = flag; // vec: 110
            break;
        case 3:
            ICU.GENBL0.BIT.EN7 = flag; // vec: 110
            break;
        case 4:
            ICU.GENBL0.BIT.EN9 = flag; // vec: 110
            break;
        case 5:
            ICU.GENBL0.BIT.EN11 = flag; // vec: 110
            break;
        case 6:
            ICU.GENBL0.BIT.EN13 = flag; // vec: 110
            break;
        case 7:
            ICU.GENBL0.BIT.EN15 = flag; // vec: 110
            break;
        case 8:
            ICU.GENBL1.BIT.EN25 = flag; // vec: 111
            break;
        case 9:
            ICU.GENBL1.BIT.EN27 = flag; // vec: 111
            break;
        case 10:
            ICU.GENBL1.BIT.EN29 = flag; // vec: 110
            break;
        case 11:
            ICU.GENAL0.BIT.EN9 = flag; // vec: 112
            break;
        case 12:
            ICU.GENAL0.BIT.EN13 = flag; // vec: 112
            break;
    }
}
#endif

void rx_sci_er_int_enable(uint32_t ch) {
    rx_sci_er_set_int(ch, 1);
}

void rx_sci_er_int_disable(uint32_t ch) {
    rx_sci_er_set_int(ch, 0);
}

void rx_sci_isr_rx(uint32_t ch) {
    uint32_t idx = ch_to_idx[ch];
    uint16_t d;
    #if defined(RX65N)
    if (ch_9bit[idx]) {
        d = (uint16_t)sci_regs[idx]->RDRHL.WORD;
    } else {
        d = (uint16_t)sci_regs[idx]->RDR;
    }
    #else
    d = (uint16_t)sci_regs[idx]->RDR;
    #endif
    if (sci_cb[idx]) {
        if ((*sci_cb[idx])(ch, (int)d)) {
        }
    }
    uint32_t size = rx_fifo[idx].size;
    sci_fifo *rxfifo = (sci_fifo *)&rx_fifo[idx];
    if (rxfifo->len < size) {
        uint32_t i = rxfifo->head;
        #if defined(RX65N)
        if (ch_9bit[idx]) {
            *(uint16_t *)(rxfifo->bufp + i) = (uint16_t)d;
            i += 2;
            rxfifo->len += 2;
        } else {
            *(rxfifo->bufp + i) = (uint8_t)d;
            i++;
            rxfifo->len++;
        }
        #else
        *(rxfifo->bufp + i) = (uint8_t)d;
        i++;
        rxfifo->len++;
        #endif
        rxfifo->head = i % size;
        if (m_rts_pin[idx] != PIN_END) {
            if (rxfifo->len > (size - rx_SCI_FLOW_START_NUM)) {
                rx_gpio_write(m_rts_pin[idx], 1);
            }
        }
    }
}

void rx_sci_isr_er(uint32_t ch) {
    uint32_t idx = ch_to_idx[ch];
    volatile ST_SCI0 *sci_reg = sci_regs[idx];
    sci_reg->RDR;
    while (0 != (sci_reg->SSR.BYTE & 0x38)) {
        sci_reg->RDR;
        sci_reg->SSR.BYTE = (uint8_t)((sci_reg->SSR.BYTE & ~0x38) | 0xc0);
        if (0 != (sci_reg->SSR.BYTE & 0x38)) {
            __asm__ __volatile__ ("nop");
        }
    }
}

void rx_sci_isr_tx(uint32_t ch) {
    uint32_t idx = ch_to_idx[ch];
    uint32_t size = tx_fifo[idx].size;
    sci_fifo *txfifo = (sci_fifo *)&tx_fifo[idx];
    if (txfifo->len != 0) {
        uint32_t i = txfifo->tail;
        #if defined(RX65N)
        if (ch_9bit[idx]) {
            sci_regs[idx]->TDRHL.WORD = *(uint16_t *)(txfifo->bufp + i);
            i += 2;
            txfifo->len -= 2;
        } else {
            sci_regs[idx]->TDR = (uint8_t)*(txfifo->bufp + i);
            i++;
            txfifo->len--;
        }
        #else
        sci_regs[idx]->TDR = (uint8_t)*(txfifo->bufp + i);
        i++;
        txfifo->len--;
        #endif
        txfifo->tail = i % size;
    } else {
        /* tx_fifo[idx].len == 0 */
        /* after transfer completed */
        uint8_t scr = sci_regs[idx]->SCR.BYTE;
        scr &= (uint8_t) ~0x80; /* TIE disable */
        scr |= (uint8_t)0x04;  /* TEIE enable */
        sci_regs[idx]->SCR.BYTE = scr;
    }
}

void rx_sci_isr_te(uint32_t ch) {
    uint32_t idx = ch_to_idx[ch];
    tx_fifo[idx].busy = 0;
    sci_regs[idx]->SCR.BYTE &= (uint8_t) ~0x84; /* TIE and TEIE disable */
}

int rx_sci_rx_ch(uint32_t ch) {
    uint16_t c;
    uint32_t idx = ch_to_idx[ch];
    uint32_t size = rx_fifo[idx].size;
    sci_fifo *rxfifo = (sci_fifo *)&rx_fifo[idx];
    if (rxfifo->len) {
        uint32_t state = rx_disable_irq();
        uint32_t i = rxfifo->tail;
        #if defined(RX65N)
        if (ch_9bit[ch]) {
            c = *(uint16_t *)(rxfifo->bufp + i);
            i += 2;
            rxfifo->len -= 2;
        } else {
            c = (uint16_t)*(rxfifo->bufp + i);
            i++;
            rxfifo->len--;
        }
        #else
        c = (uint16_t)*(rxfifo->bufp + i);
        i++;
        rxfifo->len--;
        #endif
        rxfifo->tail = i % size;
        if (m_rts_pin[idx] != PIN_END) {
            if (rxfifo->len <= (size - rx_SCI_FLOW_START_NUM)) {
                rx_gpio_write(m_rts_pin[idx], 0);
            }
        }
        rx_enable_irq(state);
    } else {
        c = 0;
    }
    return (int)c;
}

int rx_sci_rx_any(uint32_t ch) {
    uint32_t idx = ch_to_idx[ch];
    return (int)(rx_fifo[idx].len != 0);
}

void rx_sci_tx_ch(uint32_t ch, int c) {
    uint32_t idx = ch_to_idx[ch];
    uint32_t size = tx_fifo[idx].size;
    sci_fifo *txfifo = (sci_fifo *)&tx_fifo[idx];
    while (tx_fifo[idx].len == size) {
    }
    uint32_t state = rx_disable_irq();
    uint32_t i = tx_fifo[idx].head;
    #if defined(RX65N)
    if (ch_9bit[idx]) {
        *(uint16_t *)(txfifo->bufp + i) = (uint16_t)c;
        i += 2;
        txfifo->len += 2;
    } else {
        *(txfifo->bufp + i) = (uint8_t)c;
        i++;
        txfifo->len++;
    }
    #else
    *(txfifo->bufp + i) = (uint8_t)c;
    i++;
    txfifo->len++;
    #endif
    txfifo->head = i % size;
    if (!txfifo->busy) {
        txfifo->busy = 1;
        uint8_t scr = sci_regs[idx]->SCR.BYTE;
        if ((scr & 0xa0) != 0) {
            sci_regs[idx]->SCR.BYTE &= ~0xa0;
        }
        sci_regs[idx]->SCR.BYTE |= 0xa0; /* TIE and TE enable */
    }
    rx_enable_irq(state);
}

int rx_sci_tx_wait(uint32_t ch) {
    uint32_t idx = ch_to_idx[ch];
    return (int)(tx_fifo[idx].len != (tx_fifo[idx].size - 1));
}

void rx_sci_tx_break(uint32_t ch) {
    uint32_t idx = ch_to_idx[ch];
    volatile ST_SCI0 *sci_reg = sci_regs[idx];
    uint8_t scr = sci_reg->SCR.BYTE;
    uint8_t smr = sci_reg->SMR.BYTE;
    sci_reg->SCR.BYTE = 0;
    while (sci_reg->SCR.BYTE != 0) {
        ;
    }
    sci_reg->SMR.BYTE |= (uint8_t)0x08;
    sci_reg->SCR.BYTE = scr;
    sci_reg->TDR = 0;
    #if defined(RX65N)
    while ((sci_reg->SSR.BYTE & 0x80) == 0) {
        ;
    }
    #endif
    sci_reg->SMR.BYTE = smr;
    return;
}

void rx_sci_tx_str(uint32_t ch, uint8_t *p) {
    int c;
    #if defined(RX65N)
    uint32_t idx = ch_to_idx[ch];
    if (ch_9bit[idx]) {
        uint16_t *q = (uint16_t *)p;
        while ((c = *q++) != 0) {
            rx_sci_tx_ch(ch, (int)c);
        }
    } else {
        while ((c = (int)*p++) != 0) {
            rx_sci_tx_ch(ch, (int)c);
        }
    }
    #else
    while ((c = (int)*p++) != 0) {
        rx_sci_tx_ch(ch, (int)c);
    }
    #endif
}

static void rx_sci_fifo_set(sci_fifo *fifo, uint8_t *bufp, uint32_t size) {
    fifo->head = 0;
    fifo->tail = 0;
    fifo->len = 0;
    fifo->busy = 0;
    fifo->bufp = bufp;
    fifo->size = size;
}

void rx_sci_txfifo_set(uint32_t ch, uint8_t *bufp, uint32_t size) {
    uint32_t idx = ch_to_idx[ch];
    sci_fifo *fifo = (sci_fifo *)&tx_fifo[idx];
    rx_sci_fifo_set(fifo, bufp, size);
}

void rx_sci_rxfifo_set(uint32_t ch, uint8_t *bufp, uint32_t size) {
    uint32_t idx = ch_to_idx[ch];
    sci_fifo *fifo = (sci_fifo *)&rx_fifo[idx];
    rx_sci_fifo_set(fifo, bufp, size);
}

static void rx_sci_fifo_init(uint32_t ch) {
    uint32_t idx = ch_to_idx[ch];
    rx_sci_txfifo_set(ch, (uint8_t *)&tx_buf[idx][0], SCI_TX_BUF_SIZE);
    rx_sci_rxfifo_set(ch, (uint8_t *)&rx_buf[idx][0], SCI_RX_BUF_SIZE);
}

void rx_sci_int_priority(uint32_t ch, int priority) {
    switch (ch) {
        case 0:
            IPR(SCI0, RXI0) = priority;
            IPR(SCI0, TXI0) = priority;
            #if (defined(RX64M) || defined(RX65N))
            set_int_priority_groupbl0(priority);
            #endif
            break;
        case 1:
            IPR(SCI1, RXI1) = priority;
            IPR(SCI1, TXI1) = priority;
            #if (defined(RX64M) || defined(RX65N))
            set_int_priority_groupbl0(priority);
            #endif
            break;
        case 2:
            IPR(SCI2, RXI2) = priority;
            IPR(SCI2, TXI2) = priority;
            #if (defined(RX64M) || defined(RX65N))
            set_int_priority_groupbl0(priority);
            #endif
            break;
        case 3:
            IPR(SCI3, RXI3) = priority;
            IPR(SCI3, TXI3) = priority;
            #if (defined(RX64M) || defined(RX65N))
            set_int_priority_groupbl0(priority);
            #endif
            break;
        case 4:
            IPR(SCI4, RXI4) = priority;
            IPR(SCI4, TXI4) = priority;
            #if (defined(RX64M) || defined(RX65N))
            set_int_priority_groupbl0(priority);
            #endif
            break;
        case 5:
            IPR(SCI5, RXI5) = priority;
            IPR(SCI5, TXI5) = priority;
            #if (defined(RX64M) || defined(RX65N))
            set_int_priority_groupbl0(priority);
            #endif
            break;
        case 6:
            IPR(SCI6, RXI6) = priority;
            IPR(SCI6, TXI6) = priority;
            #if (defined(RX64M) || defined(RX65N))
            set_int_priority_groupbl0(priority);
            #endif
            break;
        case 7:
            IPR(SCI7, RXI7) = priority;
            IPR(SCI7, TXI7) = priority;
            #if (defined(RX64M) || defined(RX65N))
            set_int_priority_groupbl0(priority);
            #endif
            break;
        case 8:
            IPR(SCI8, RXI8) = priority;
            IPR(SCI8, TXI8) = priority;
            #if (defined(RX64M) || defined(RX65N))
            set_int_priority_groupbl1(priority);
            #endif
            break;
        case 9:
            IPR(SCI9, RXI9) = priority;
            IPR(SCI9, TXI9) = priority;
            #if (defined(RX64M) || defined(RX65N))
            set_int_priority_groupbl1(priority);
            #endif
            break;
        case 10:
            IPR(SCI10, RXI10) = priority;
            IPR(SCI10, TXI10) = priority;
            #if (defined(RX64M) || defined(RX65N))
            set_int_priority_groupal0(priority);
            #endif
            break;
        case 11:
            IPR(SCI11, RXI11) = priority;
            IPR(SCI11, TXI11) = priority;
            #if (defined(RX64M) || defined(RX65N))
            set_int_priority_groupal0(priority);
            #endif
            break;
        case 12:
            IPR(SCI12, RXI12) = priority;
            IPR(SCI12, TXI12) = priority;
            #if (defined(RX64M) || defined(RX65N))
            set_int_priority_groupbl0(priority);
            #endif
            break;
        default:
            break;
    }
}

void rx_sci_int_enable(uint32_t ch) {
    rx_sci_tx_set_int(ch, 1);
    rx_sci_rx_set_int(ch, 1);
    rx_sci_er_set_int(ch, 1);
    rx_sci_te_set_int(ch, 1);
    #if (defined(RX64M) || defined(RX65N))
    if (ch < 8 || ch == 12) {
        set_int_state_groupbl0(1);
    } else if (ch == 8 || ch == 9) {
        set_int_state_groupbl1(1);
    } else {
        set_int_state_groupal0(1);
    }
    #endif
}

void rx_sci_int_disable(uint32_t ch) {
    rx_sci_tx_set_int(ch, 0);
    rx_sci_rx_set_int(ch, 0);
    rx_sci_er_set_int(ch, 0);
    rx_sci_te_set_int(ch, 0);
    #if (defined(RX64M) || defined(RX65N))
    // if (ch < 8 || ch == 12) {
    //     set_int_state_groupbl0(0);
    // } else if (ch == 8 || ch == 9) {
    //     set_int_state_groupbl1(0);
    // } else {
    //     set_int_state_groupal0(0);
    // }
    #endif
}

void rx_sci_module(uint32_t ch, int flag) {
    switch (ch) {
        case 0:
            MSTP_SCI0 = flag;
            break;
        case 1:
            MSTP_SCI1 = flag;
            break;
        case 2:
            MSTP_SCI2 = flag;
            break;
        case 3:
            MSTP_SCI3 = flag;
            break;
        case 4:
            MSTP_SCI4 = flag;
            break;
        case 5:
            MSTP_SCI5 = flag;
            break;
        case 6:
            MSTP_SCI6 = flag;
            break;
        case 7:
            MSTP_SCI7 = flag;
            break;
        case 8:
            MSTP_SCI8 = flag;
            break;
        case 9:
            MSTP_SCI9 = flag;
            break;
        case 10:
            MSTP_SCI10 = flag;
            break;
        case 11:
            MSTP_SCI11 = flag;
            break;
        case 12:
            MSTP_SCI12 = flag;
            break;
        default:
            break;
    }
}

static void rx_sci_module_start(uint32_t ch) {
    rx_sci_module(ch, 0);
}

static void rx_sci_module_stop(uint32_t ch) {
    rx_sci_module(ch, 1);
}

void rx_sci_set_baud(uint32_t ch, uint32_t baud) {
    volatile ST_SCI0 *sci = sci_regs[ch];
    #if defined(RX65N)
    if (baud == 0) {
        sci->SMR.BYTE &= ~0x03; // PCLK/1
        sci->SEMR.BYTE |= 0x50; // BGDM and ABCS
        sci->BRR = (uint8_t)((int)PCLK / SCI_DEFAULT_BAUD / 8 - 1);
        // sci->MDDR = (uint8_t)((((int)(sci->BRR) + 1) * SCI_DEFAULT_BAUD * 32 * 256) / PCLK);
    } else if (baud > 19200) {
        sci->SMR.BYTE &= ~0x03; // PCLK/1
        sci->SEMR.BYTE |= 0x50; //  BGDM and ABCS
        sci->BRR = (uint8_t)((int)PCLK / baud / 8 - 1);
        // sci->MDDR = (uint8_t)((((int)(sci->BRR) + 1) * baud * 32 * 256) / PCLK);
    } else if (baud > 2400) {
        sci->SMR.BYTE &= ~0x03;
        sci->SMR.BYTE |= 0x02;  // PCLK/16
        sci->SEMR.BYTE |= 0x50; // BGDM and ABCS
        sci->BRR = (uint8_t)((int)PCLK / baud / 128 - 1);
        // sci->MDDR = (uint8_t)((((int)(sci->BRR) + 1) * baud * 32 * 256) / PCLK);
    } else {
        sci->SMR.BYTE &= ~0x03;
        sci->SMR.BYTE |= 0x03;  // PCLK/64
        sci->SEMR.BYTE |= 0x50; // BGDM and ABCS
        sci->BRR = (uint8_t)((int)PCLK / baud / 512 - 1);
        // sci->MDDR = (uint8_t)((((int)(sci->BRR) + 1) * baud * 32 * 256) / PCLK);
    }
    #else
    if (baud == 0) {
        sci->SMR.BYTE &= ~0x03; // PCLK/1
        sci->BRR = (uint8_t)((int)PCLK / SCI_DEFAULT_BAUD / 32 - 1);
    } else if (baud > 9600) {
        sci->SMR.BYTE &= ~0x03; // PCLK/1
        sci->BRR = (uint8_t)((int)PCLK / baud / 32 - 1);
    } else {
        sci->SMR.BYTE &= ~0x03;
        sci->SMR.BYTE |= 0x02;  // PCLK/16
        sci->BRR = (uint8_t)((int)PCLK / baud / 512 - 1);
    }
    #endif
}

/*
 * bits: 7, 8, 9
 * parity: none:0, odd:1, even:2
 */
void rx_sci_init_with_flow(uint32_t ch, uint32_t tx_pin, uint32_t rx_pin, uint32_t baud, uint32_t bits, uint32_t parity, uint32_t stop, uint32_t flow, uint32_t cts_pin, uint32_t rts_pin) {
    uint8_t smr = 0;
    uint8_t scmr = (uint8_t)0xf2;
    uint32_t idx = ch_to_idx[ch];
    volatile ST_SCI0 *sci_reg = sci_regs[idx];
    if (rx_sci_init_flag[idx] == 0) {
        rx_sci_fifo_init(idx);
        sci_cb[idx] = 0;
        rx_sci_init_flag[idx]++;
    } else {
        rx_sci_init_flag[idx]++;
        return;
    }
    SYSTEM.PRCR.WORD = 0xA502;
    MPC.PWPR.BIT.B0WI = 0; /* Enable write to PFSWE */
    MPC.PWPR.BIT.PFSWE = 1; /* Enable write to PFS */
    rx_sci_module_start(ch);
    rx_sci_tx_set_pin(tx_pin);
    rx_sci_rx_set_pin(rx_pin);
    if (flow) {
        if (cts_pin != (uint32_t)PIN_END) {
            m_cts_pin[idx] = cts_pin;
            rx_sci_cts_set_pin(cts_pin);
        }
        if (rts_pin != (uint32_t)PIN_END) {
            m_rts_pin[idx] = rts_pin;
            rx_gpio_config(rts_pin, GPIO_MODE_OUTPUT_PP, false, 0);
            rx_gpio_write(rts_pin, 0);
        }
    }
    // MPC.PWPR.BYTE = 0x80;     /* Disable write to PFSWE and PFS*/
    SYSTEM.PRCR.WORD = 0xA500;
    rx_sci_int_disable(ch);
    rx_sci_int_priority(ch, RX_PRI_UART);
    uint32_t state = rx_disable_irq();
    sci_reg->SCR.BYTE = 0;
    while (sci_reg->SCR.BYTE != 0) {
        ;
    }
    if (bits == 7) {
        smr |= (uint8_t)0x40;
    } else {
        smr &= (uint8_t) ~0x40;
    }
    if (parity != 0) {
        smr |= (uint8_t)0x20;
    } else {
        smr &= (uint8_t) ~0x20;
    }
    if (parity == 1) {
        smr |= (uint8_t)0x10;
    } else {
        smr &= (uint8_t) ~0x10;
    }
    if (stop == 2) {
        smr |= (uint8_t)0x80;
    } else {
        smr &= (uint8_t) ~0x80;
    }
    sci_reg->SMR.BYTE = smr;
    #if defined(RX65N)
    if (bits == 9) {
        scmr &= (uint8_t) ~0x10;
        ch_9bit[idx] = 1;
    } else {
        scmr |= (uint8_t)0x10;
        ch_9bit[idx] = 0;
    }
    #endif
    if (flow) {
        sci_reg->SPMR.BYTE |= 0x01;
    }
    sci_reg->SCMR.BYTE = scmr;
    rx_sci_set_baud(ch, baud);
    delay_us(10);
    sci_reg->SCR.BYTE = (uint8_t)0xd0;
    rx_sci_int_enable(ch);
    rx_enable_irq(state);
    if (!rx_sci_init_flag[idx]) {
        rx_sci_init_flag[idx] = true;
    }
}

void rx_sci_init(uint32_t ch, uint32_t tx_pin, uint32_t rx_pin, uint32_t baud, uint32_t bits, uint32_t parity, uint32_t stop, uint32_t flow) {
    rx_sci_init_with_flow(ch, tx_pin, rx_pin, baud, bits, parity, stop, flow, PIN_END, PIN_END);
}

void rx_sci_init_default(uint32_t ch, uint32_t tx_pin, uint32_t rx_pin, uint32_t baud) {
    rx_sci_init(ch, tx_pin, rx_pin, baud, 8, 0, 1, 0);
}

void rx_sci_deinit(uint32_t ch) {
    uint32_t idx = ch_to_idx[ch];
    if (rx_sci_init_flag[idx] != 0) {
        rx_sci_init_flag[idx]--;
        if (rx_sci_init_flag[idx] == 0) {
            rx_sci_int_disable(ch);
            rx_sci_module_stop(ch);
            sci_cb[idx] = 0;
        }
    }
}

/* rx interrupt */
void INT_Excep_SCI0_RXI0(void) {
    rx_sci_isr_rx(0);
}
void INT_Excep_SCI1_RXI1(void) {
    rx_sci_isr_rx(1);
}
void INT_Excep_SCI2_RXI2(void) {
    rx_sci_isr_rx(2);
}
void INT_Excep_SCI3_RXI3(void) {
    rx_sci_isr_rx(3);
}
void INT_Excep_SCI4_RXI4(void) {
    rx_sci_isr_rx(4);
}
void INT_Excep_SCI5_RXI5(void) {
    rx_sci_isr_rx(5);
}
void INT_Excep_SCI6_RXI6(void) {
    rx_sci_isr_rx(6);
}
void INT_Excep_SCI7_RXI7(void) {
    rx_sci_isr_rx(7);
}
void INT_Excep_SCI8_RXI8(void) {
    rx_sci_isr_rx(8);
}
void INT_Excep_SCI9_RXI9(void) {
    rx_sci_isr_rx(9);
}
void INT_Excep_SCI10_RXI10(void) {
    rx_sci_isr_rx(10);
}
void INT_Excep_SCI11_RXI11(void) {
    rx_sci_isr_rx(11);
}
void INT_Excep_SCI12_RXI12(void) {
    rx_sci_isr_rx(12);
}

/* tx interrupt */
void INT_Excep_SCI0_TXI0(void) {
    rx_sci_isr_tx(0);
}
void INT_Excep_SCI1_TXI1(void) {
    rx_sci_isr_tx(1);
}
void INT_Excep_SCI2_TXI2(void) {
    rx_sci_isr_tx(2);
}
void INT_Excep_SCI3_TXI3(void) {
    rx_sci_isr_tx(3);
}
void INT_Excep_SCI4_TXI4(void) {
    rx_sci_isr_tx(4);
}
void INT_Excep_SCI5_TXI5(void) {
    rx_sci_isr_tx(5);
}
void INT_Excep_SCI6_TXI6(void) {
    rx_sci_isr_tx(6);
}
void INT_Excep_SCI7_TXI7(void) {
    rx_sci_isr_tx(7);
}
void INT_Excep_SCI8_TXI8(void) {
    rx_sci_isr_tx(8);
}
void INT_Excep_SCI9_TXI9(void) {
    rx_sci_isr_tx(9);
}
void INT_Excep_SCI10_TXI10(void) {
    rx_sci_isr_tx(10);
}
void INT_Excep_SCI11_TXI11(void) {
    rx_sci_isr_tx(11);
}
void INT_Excep_SCI12_TXI12(void) {
    rx_sci_isr_tx(12);
}

#if defined(RX63N)
/* er interrupt */
void INT_Excep_SCI0_ERI0(void) {
    rx_sci_isr_er(0);
}
void INT_Excep_SCI1_ERI1(void) {
    rx_sci_isr_er(1);
}
void INT_Excep_SCI2_ERI2(void) {
    rx_sci_isr_er(2);
}
void INT_Excep_SCI3_ERI3(void) {
    rx_sci_isr_er(3);
}
void INT_Excep_SCI4_ERI4(void) {
    rx_sci_isr_er(4);
}
void INT_Excep_SCI5_ERI5(void) {
    rx_sci_isr_er(5);
}
void INT_Excep_SCI6_ERI6(void) {
    rx_sci_isr_er(6);
}
void INT_Excep_SCI0_ERI7(void) {
    rx_sci_isr_er(7);
}
void INT_Excep_SCI1_ERI8(void) {
    rx_sci_isr_er(8);
}
void INT_Excep_SCI2_ERI9(void) {
    rx_sci_isr_er(9);
}
void INT_Excep_SCI3_ERI10(void) {
    rx_sci_isr_er(10);
}
void INT_Excep_SCI4_ERI11(void) {
    rx_sci_isr_er(11);
}
void INT_Excep_SCI5_ERI2(void) {
    rx_sci_isr_er(12);
}

/* te interrupt */
void INT_Excep_SCI0_TEI0(void) {
    rx_sci_isr_te(0);
}
void INT_Excep_SCI1_TEI1(void) {
    rx_sci_isr_te(1);
}
void INT_Excep_SCI2_TEI2(void) {
    rx_sci_isr_te(2);
}
void INT_Excep_SCI3_TEI3(void) {
    rx_sci_isr_te(3);
}
void INT_Excep_SCI4_TEI4(void) {
    rx_sci_isr_te(4);
}
void INT_Excep_SCI5_TEI5(void) {
    rx_sci_isr_te(5);
}
void INT_Excep_SCI6_TEI6(void) {
    rx_sci_isr_te(6);
}
void INT_Excep_SCI7_TEI7(void) {
    rx_sci_isr_te(7);
}
void INT_Excep_SCI8_TEI8(void) {
    rx_sci_isr_te(8);
}
void INT_Excep_SCI9_TEI9(void) {
    rx_sci_isr_te(9);
}
void INT_Excep_SCI10_TEI10(void) {
    rx_sci_isr_te(10);
}
void INT_Excep_SCI11_TEI11(void) {
    rx_sci_isr_te(11);
}
void INT_Excep_SCI12_TEI12(void) {
    rx_sci_isr_te(12);
}
#endif
