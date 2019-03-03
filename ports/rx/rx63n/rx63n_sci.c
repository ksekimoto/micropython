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

#include <stdbool.h>
#include "common.h"
#include "iodefine.h"
#include "interrupt_handlers.h"
#include "pendsv.h"
#include "rx63n_sci.h"

#if defined(GRCITRUS)
#define SCI_CH_NUM 7
#define SCI_BUF_SIZE 1024
#elif defined(GRSAKURA)
#define SCI_CH_NUM 4
#define SCI_BUF_SIZE 512
#else
#define SCI_CH_NUM 4
#define SCI_BUF_SIZE 512
#endif
#define SCI_DEFAULT_PRIORITY 3
#define SCI_DEFAULT_BAUD    115200

static volatile struct st_sci0 *SCI[] = {
    (volatile struct st_sci0 *)0x8A000, /* sci 0 */
    (volatile struct st_sci0 *)0x8A020, /* sci 1 */
    (volatile struct st_sci0 *)0x8A040, /* sci 2 */
    (volatile struct st_sci0 *)0x8A060, /* sci 3 */
    (volatile struct st_sci0 *)0x8A080, /* sci 4 */
    (volatile struct st_sci0 *)0x8A0A0, /* sci 5 */
    (volatile struct st_sci0 *)0x8A0C0, /* sci 6 */
    (volatile struct st_sci0 *)0x8A0E0, /* sci 7 */
    (volatile struct st_sci0 *)0x8A100, /* sci 8 */
    (volatile struct st_sci0 *)0x8A120, /* sci 9 */
    (volatile struct st_sci0 *)0x8A140, /* sci 10 */
    (volatile struct st_sci0 *)0x8A160, /* sci 11 */
    (volatile struct st_sci0 *)0x8B300, /* sci 12 */
};

static const uint8_t sci_tx_pins[] = {
    P20,    /* ch 0 P20 */
    P16,    /* ch 1 P16 */
    P50,    /* ch 2 P50 */
    P23,    /* ch 3 P23 */
    0xff,   /* ch 4 */
    PC3,    /* ch 5 PC3 */
    P32 ,   /* ch 6 P32 */
    0xff,   /* ch 7 */
    0xff,   /* ch 8 */
    0xff,   /* ch 9 */
    0xff,   /* ch 10 */
    0xff,   /* ch 11 */
    0xff,   /* ch 12 */
};

static const uint8_t sci_rx_pins[] = {
    P21,    /* ch 0 P21 */
    P15,    /* ch 1 P15 */
    P52,    /* ch 2 P52 */
    P25,    /* ch 3 P25 */
    0xff,   /* ch 4 */
    PC2,    /* ch 5 PC2 */
    P33,    /* ch 6 P33 */
    0xff,   /* ch 7 */
    0xff,   /* ch 8 */
    0xff,   /* ch 9 */
    0xff,   /* ch 10 */
    0xff,   /* ch 11 */
    0xff,   /* ch 12 */
};

struct SCI_FIFO {
    int tail, head, len, busy;
    uint8_t buff[SCI_BUF_SIZE];
};

static bool sci_init_flag[SCI_CH_NUM] = {false};
static SCI_CALLBACK sci_callback[SCI_CH_NUM] = {};
static volatile struct SCI_FIFO tx_fifo[SCI_CH_NUM];
static volatile struct SCI_FIFO rx_fifo[SCI_CH_NUM];

static void delay_us(volatile unsigned int us) {
    us *= 60;
    while (us-- > 0)
        ;
}

void sci_rx_set_callback(int ch, SCI_CALLBACK callback)
{
    sci_callback[ch] = callback;
}

void sci_rx_set_int(int ch, int flag) {
    int idx = (214 + ch * 3) / 8;
    int bit = (6 +  ch * 3) & 7;
    uint8_t mask = (1 << bit);
    ICU.IER[idx].BYTE = (ICU.IER[idx].BYTE & ~mask) | (flag << bit);
}

void sci_rx_int_enable(int ch) {
    sci_rx_set_int(ch, 1);
}

void sci_rx_int_disable(int ch) {
    sci_rx_set_int(ch, 0);
}

void sci_tx_set_int(int ch, int flag) {
    int idx = (215 + ch * 3) / 8;
    int bit = (7 +  ch * 3) & 7;
    uint8_t mask = (1 << bit);
    ICU.IER[idx].BYTE = (ICU.IER[idx].BYTE & ~mask) | (flag << bit);
}

void sci_tx_int_enable(int ch) {
    sci_tx_set_int(ch, 1);
}

void sci_tx_int_disable(int ch) {
    sci_tx_set_int(ch, 0);
}

void sci_te_set_int(int ch, int flag) {
    int idx = (216 + ch * 3) / 8;
    int bit = (0 +  ch * 3) & 7;
    uint8_t mask = (1 << bit);
    ICU.IER[idx].BYTE = (ICU.IER[idx].BYTE & ~mask) | (flag << bit);
}

void sci_te_int_enable(int ch) {
    sci_te_set_int(ch, 1);
}

void sci_te_int_disable(int ch) {
    sci_te_set_int(ch, 0);
}

#if defined(RX63N_SCI_ADDITIONAL)
void sci_tx_enable(int ch) {
    volatile struct st_sci0 *sci = SCI[ch];
    sci->SCR.BYTE |= 0xa0;  /* TIE and TE set */
}

void sci_tx_disable(int ch) {
    volatile struct st_sci0 *sci = SCI[ch];
    sci->SCR.BYTE &= ~0xa0; /* TIE and TE clear */
}
#endif

static void sci_isr_rx(int ch) {
    int i;
    uint8_t d;
    volatile struct st_sci0 *sci = SCI[ch];
    d = sci->RDR;
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
    volatile struct st_sci0 *sci = SCI[ch];
    sci->RDR;
    //sci->SSR.BYTE = 0x84;
    while (0 != (sci->SSR.BYTE & 0x38)) {
        sci->RDR;
        sci->SSR.BYTE = (sci->SSR.BYTE & ~0x38) | 0xc0;
        if (0 != (sci->SSR.BYTE & 0x38)) {
            __asm__ __volatile__("nop");
        }
    }
}

static void sci_isr_tx(int ch) {
    int i;
    volatile struct st_sci0 *sci = SCI[ch];
    rx_disable_irq();
    if (!tx_fifo[ch].busy) {
        //sci->SCR.BYTE &= ~0xa0; /* TIE and TE reset */
        goto sci_isr_tx_exit;
    }
    if (tx_fifo[ch].len != 0) {
        i = tx_fifo[ch].tail;
        sci->TDR = tx_fifo[ch].buff[i++];
        tx_fifo[ch].len--;
        tx_fifo[ch].tail = i % SCI_BUF_SIZE;
    }
    if (tx_fifo[ch].len == 0) {
        sci->SCR.BYTE |= 0x04; /* TEI set */
    }
sci_isr_tx_exit:
    rx_enable_irq();
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
    volatile struct st_sci0 *sci = SCI[ch];
    rx_disable_irq();
    //if (!tx_fifo[ch].busy)
    //    goto sci_isr_te_exit;
    sci->SCR.BYTE &= ~0xa4; /* TIE, TE and TEI reset */
    if (tx_fifo[ch].len == 0) {
        tx_fifo[ch].busy = 0;
    } else {
        sci->SCR.BYTE |= 0xa0;  /* TIE and TE set */
    }
//sci_isr_te_exit:
    rx_enable_irq();
}

uint8_t sci_rx_ch(int ch) {
    uint8_t c;
    int i;
    if (rx_fifo[ch].len) {
        sci_rx_int_disable(ch);
        i = rx_fifo[ch].tail;
        c = rx_fifo[ch].buff[i++];
        rx_fifo[ch].tail = i % SCI_BUF_SIZE;
        rx_fifo[ch].len--;
        sci_rx_int_enable(ch);
    } else {
        c = 0;
    }
    return c;
}

int sci_rx_any(int ch) {
    return (int)(rx_fifo[ch].head != rx_fifo[ch].tail);
}

void sci_tx_ch(int ch, uint8_t c) {
    int i;
    volatile struct st_sci0 *sci = SCI[ch];
    while (tx_fifo[ch].len == SCI_BUF_SIZE) {
        rx_disable_irq();
        i = tx_fifo[ch].tail;
        sci->TDR = tx_fifo[ch].buff[i++];
        tx_fifo[ch].len--;
        tx_fifo[ch].tail = i % SCI_BUF_SIZE;
        rx_enable_irq();
    }
    rx_disable_irq();
    if (!tx_fifo[ch].busy) {
        tx_fifo[ch].busy = 1;
        sci->SCR.BYTE |= 0xa0;  /* TIE and TE set */
    }
    i = tx_fifo[ch].head;
    tx_fifo[ch].buff[i++] = c;
    tx_fifo[ch].head = i % SCI_BUF_SIZE;
    tx_fifo[ch].len++;
    rx_enable_irq();
#if 0
    while (tx_fifo[ch].len == SCI_BUF_SIZE) {
        while ((sci->SSR.BYTE & 0x04) == 0) ;
        i = tx_fifo[ch].tail;
        sci->TDR = tx_fifo[ch].buff[i++];
        tx_fifo[ch].len--;
        tx_fifo[ch].tail = i % SCI_BUF_SIZE;
    }
    if (tx_fifo[ch].busy) {
        sci_tx_int_disable(ch);
        i = tx_fifo[ch].head;
        tx_fifo[ch].buff[i++] = c;
        tx_fifo[ch].head = i % SCI_BUF_SIZE;
        tx_fifo[ch].len++;
        sci_tx_int_enable(ch);
    } else {
        sci->TDR = c;
        tx_fifo[ch].busy = 1;
    }
#endif
}

int sci_tx_wait(int ch) {
    return (int)(tx_fifo[ch].head != tx_fifo[ch].tail);
}

void sci_tx_str(int ch, uint8_t *p) {
    uint8_t c;
    while ((c = *p++) != 0) {
        sci_tx_ch(ch, c);
    }
}

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

void sci_int_priority(int ch, int priority) {
    switch (ch) {
    case 0:
        IPR(SCI0, RXI0) = priority;
        //IPR(SCI0, TXI0) = priority;
        break;
    case 1:
        IPR(SCI1, RXI1) = priority;
        //IPR(SCI1, TXI1) = priority;
        break;
    case 2:
        IPR(SCI2, RXI2) = priority;
        //IPR(SCI2, TXI2) = priority;
        break;
    case 3:
        IPR(SCI3, RXI3) = priority;
        //IPR(SCI3, TXI3) = tx_priority;
        break;
    case 4:
        IPR(SCI4, RXI4) = priority;
        //IPR(SCI4, TXI4) = priority;
        break;
    case 5:
        IPR(SCI5, RXI5) = priority;
        //IPR(SCI5, TXI5) = priority;
        break;
    case 6:
        IPR(SCI6, RXI6) = priority;
        //IPR(SCI6, TXI6) = tx_priority;
        break;
    case 7:
        IPR(SCI7, RXI7) = priority;
        //IPR(SCI7, TXI7) = priority;
        break;
    case 8:
        IPR(SCI8, RXI8) = priority;
        //IPR(SCI8, TXI8) = priority;
        break;
    case 9:
        IPR(SCI9, RXI9) = priority;
        //IPR(SCI9, TXI9) = priority;
        break;
    case 10:
        IPR(SCI10, RXI10) = priority;
        //IPR(SCI10, TXI10) = priority;
        break;
    case 11:
        IPR(SCI11, RXI11) = priority;
        //IPR(SCI11, TXI11) = priority;
        break;
    case 12:
        IPR(SCI12, RXI12) = priority;
        //IPR(SCI12, TXI12) = priority;
        break;
    default:
        break;
    }
}

void sci_int_enable(int ch) {
    sci_tx_set_int(ch, 1);
    sci_te_set_int(ch, 1);
    sci_rx_set_int(ch, 1);
}

void sci_int_disable(int ch) {
    sci_tx_set_int(ch, 0);
    sci_te_set_int(ch, 1);
    sci_rx_set_int(ch, 0);
}

void sci_module(int ch, int flag) {
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

void sci_module_start(int ch) {
    sci_module(ch, 0);
}

void sci_module_stop(int ch) {
    sci_module(ch, 1);
}

void sci_set_baud(int ch, int baud) {
    volatile struct st_sci0 *sci = SCI[ch];
    if (baud != 0)
        sci->BRR = (uint8_t)((int)PCLK / baud / 32 - 1);
    else
        sci->BRR = (uint8_t)((int)PCLK / SCI_DEFAULT_BAUD / 32 - 1);
}

void sci_init_with_pins(int ch, int tx_pin, int rx_pin, int baud) {
    volatile struct st_sci0 *sci = SCI[ch];

    if (!sci_init_flag[ch]) {
        sci_fifo_init(ch);
        sci_callback[ch] = 0;
        rx_disable_irq();
        SYSTEM.PRCR.WORD = 0xA502;
        MPC.PWPR.BIT.B0WI = 0; /* Enable write to PFSWE */
        MPC.PWPR.BIT.PFSWE = 1; /* Enable write to PFS */
        sci_module_start(ch);
        uint8_t tx_port = GPIO_PORT(tx_pin);
        uint8_t tx_mask = GPIO_MASK(tx_pin);
        uint8_t rx_port = GPIO_PORT(rx_pin);
        uint8_t rx_mask = GPIO_MASK(rx_pin);
        _PMR(tx_port) &= ~tx_mask;
        _PMR(rx_port) &= ~rx_mask;
        _PDR(tx_port) |= tx_mask;
        _PDR(rx_port) &= ~rx_mask;
        _PXXPFS(tx_port, tx_pin & 7) = 0x0a;
        _PXXPFS(rx_port, rx_pin & 7) = 0x0a;
        _PMR(tx_port) |= tx_mask;
        _PMR(rx_port) |= rx_mask;
        //MPC.PWPR.BYTE = 0x80;     /* Disable write to PFSWE and PFS*/
        SYSTEM.PRCR.WORD = 0xA500;
        sci->SCR.BYTE = 0;
        sci->SMR.BYTE = 0x00;
        sci_set_baud(ch, baud);
        delay_us(10);
        sci->SCR.BYTE = 0xd0;
        sci_int_priority(ch, SCI_DEFAULT_PRIORITY);
        sci_int_enable(ch);
        rx_enable_irq();
        sci_init_flag[ch] = true;
    }
}

void sci_init(int ch, int baud) {
    int tx_pin = (int)sci_tx_pins[ch];
    int rx_pin = (int)sci_rx_pins[ch];
    if ((tx_pin != 0xff) && (rx_pin != 0xff)) {
        sci_init_with_pins(ch, tx_pin, rx_pin, baud);
    }
}

void sci_deinit(int ch) {
    if (sci_init_flag[ch]) {
        sci_init_flag[ch] = false;
        sci_int_disable(ch);
        SYSTEM.PRCR.WORD = 0xA502;
        MPC.PWPR.BIT.B0WI = 0; /* Enable write to PFSWE */
        MPC.PWPR.BIT.PFSWE = 1; /* Enable write to PFS */
        sci_module_stop(ch);
        //MPC.PWPR.BYTE = 0x80;     /* Disable write to PFSWE and PFS*/
        SYSTEM.PRCR.WORD = 0xA500;
        sci_callback[ch] = 0;
    }
}

/* rx interrupt */
void INT_Excep_SCI0_RXI0(void) {
    sci_isr_rx(0);
}
void INT_Excep_SCI1_RXI1(void) {
    sci_isr_rx(1);
}
void INT_Excep_SCI2_RXI2(void) {
    sci_isr_rx(2);
}
void INT_Excep_SCI3_RXI3(void) {
    sci_isr_rx(3);
}
void INT_Excep_SCI4_RXI4(void) {
    sci_isr_rx(4);
}
void INT_Excep_SCI5_RXI5(void) {
    sci_isr_rx(5);
}
void INT_Excep_SCI6_RXI6(void) {
    sci_isr_rx(6);
}
void INT_Excep_SCI7_RXI7(void) {
    sci_isr_rx(7);
}
void INT_Excep_SCI8_RXI8(void) {
    sci_isr_rx(8);
}
void INT_Excep_SCI9_RXI9(void) {
    sci_isr_rx(9);
}
void INT_Excep_SCI10_RXI10(void) {
    sci_isr_rx(10);
}
void INT_Excep_SCI11_RXI11(void) {
    sci_isr_rx(11);
}
void INT_Excep_SCI12_RXI12(void) {
    sci_isr_rx(12);
}
/* er interrupt */
void INT_Excep_SCI0_ERI0(void) {
    sci_isr_er(0);
}
void INT_Excep_SCI1_ERI1(void) {
    sci_isr_er(1);
}
void INT_Excep_SCI2_ERI2(void) {
    sci_isr_er(2);
}
void INT_Excep_SCI3_ERI3(void) {
    sci_isr_er(3);
}
void INT_Excep_SCI4_ERI4(void) {
    sci_isr_er(4);
}
void INT_Excep_SCI5_ERI5(void) {
    sci_isr_er(5);
}
void INT_Excep_SCI6_ERI6(void) {
    sci_isr_er(6);
}

/* tx interrupt */
void INT_Excep_SCI0_TXI0(void) {
    sci_isr_tx(0);
}
void INT_Excep_SCI1_TXI1(void) {
    sci_isr_tx(1);
}
void INT_Excep_SCI2_TXI2(void) {
    sci_isr_tx(2);
}
void INT_Excep_SCI3_TXI3(void) {
    sci_isr_tx(3);
}
void INT_Excep_SCI4_TXI4(void) {
    sci_isr_tx(4);
}
void INT_Excep_SCI5_TXI5(void) {
    sci_isr_tx(5);
}
void INT_Excep_SCI6_TXI6(void) {
    sci_isr_tx(6);
}
void INT_Excep_SCI7_TXI7(void) {
    sci_isr_tx(7);
}
void INT_Excep_SCI8_TXI8(void) {
    sci_isr_tx(8);
}
void INT_Excep_SCI9_TXI9(void) {
    sci_isr_tx(9);
}
void INT_Excep_SCI10_TXI10(void) {
    sci_isr_tx(10);
}
void INT_Excep_SCI11_TXI11(void) {
    sci_isr_tx(11);
}
void INT_Excep_SCI12_TXI12(void) {
    sci_isr_tx(12);
}

/* tx interrupt */
void INT_Excep_SCI0_TEI0(void) {
    sci_isr_te(0);
}
void INT_Excep_SCI1_TEI1(void) {
    sci_isr_te(1);
}
void INT_Excep_SCI2_TEI2(void) {
    sci_isr_te(2);
}
void INT_Excep_SCI3_TEI3(void) {
    sci_isr_te(3);
}
void INT_Excep_SCI4_TEI4(void) {
    sci_isr_te(4);
}
void INT_Excep_SCI5_TEI5(void) {
    sci_isr_te(5);
}
void INT_Excep_SCI6_TEI6(void) {
    sci_isr_te(6);
}
void INT_Excep_SCI7_TEI7(void) {
    sci_isr_te(7);
}
void INT_Excep_SCI8_TEI8(void) {
    sci_isr_te(8);
}
void INT_Excep_SCI9_TEI9(void) {
    sci_isr_te(9);
}
void INT_Excep_SCI10_TEI10(void) {
    sci_isr_te(10);
}
void INT_Excep_SCI11_TEI11(void) {
    sci_isr_te(11);
}
void INT_Excep_SCI12_TEI12(void) {
    sci_isr_te(12);
}
