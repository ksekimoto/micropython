//
// Copyright (c) 2017, Kentaro Sekimoto
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  -Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
//  -Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include "common.h"
#include "iodefine.h"
#include "interrupt_handlers.h"

#define SCI_BUF_SIZE 4096

volatile struct st_sci0 *SCI[] = { (volatile struct st_sci0 *)0x8A000, /* sci 0 */
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
(volatile struct st_sci0 *)0x8B300 /* sci 12 */
};

static volatile struct SCI_FIFO {
    int ri, wi, ct, run;
    unsigned char buff[SCI_BUF_SIZE];
} TxFifo, RxFifo;

static volatile struct SCI_FIFO TxFifo;
static volatile struct SCI_FIFO RxFifo;

static void delay_ms(volatile unsigned int ms)
{
    ms *= 1000;
    while (ms-- > 0)
        ;
}

static void sci_isr_rx(int ch)
{
    int i;
    unsigned char d;
    volatile struct st_sci0 *sci = SCI[ch];
    d = sci->RDR;
    if (RxFifo.ct < SCI_BUF_SIZE) {
        i = RxFifo.wi;
        RxFifo.buff[i++] = d;
        RxFifo.wi = i % SCI_BUF_SIZE;
        RxFifo.ct++;
    }
}

static void sci_isr_er(int ch)
{
    volatile struct st_sci0 *sci = SCI[ch];
    sci->RDR;
    sci->SSR.BYTE = 0x84;
}

static void sci_isr_tx(int ch)
{
    int i;
    volatile struct st_sci0 *sci = SCI[ch];
    if (TxFifo.ct) {
        i = TxFifo.ri;
        sci->TDR = TxFifo.buff[i++];
        TxFifo.ri = i % SCI_BUF_SIZE;
        TxFifo.ct--;
    } else {
        TxFifo.run = 0;
    }
}

void INT_Excep_SCI0_RXI0(void)
{
    sci_isr_rx(0);
}
void INT_Excep_SCI1_RXI1(void)
{
    sci_isr_rx(1);
}
void INT_Excep_SCI2_RXI2(void)
{
    sci_isr_rx(2);
}
void INT_Excep_SCI3_RXI3(void)
{
    sci_isr_rx(3);
}
void INT_Excep_SCI5_RXI5(void)
{
    sci_isr_rx(5);
}
void INT_Excep_SCI6_RXI6(void)
{
    sci_isr_rx(6);
}

void INT_Excep_SCI0_ERI0(void)
{
    sci_isr_er(0);
}
void INT_Excep_SCI1_ERI1(void)
{
    sci_isr_er(1);
}
void INT_Excep_SCI2_ERI2(void)
{
    sci_isr_er(2);
}
void INT_Excep_SCI3_ERI3(void)
{
    sci_isr_er(3);
}
void INT_Excep_SCI5_ERI5(void)
{
    sci_isr_er(5);
}
void INT_Excep_SCI6_ERI6(void)
{
    sci_isr_er(6);
}

void INT_Excep_SCI0_TXI0(void)
{
    sci_isr_tx(0);
}
void INT_Excep_SCI1_TXI1(void)
{
    sci_isr_tx(1);
}
void INT_Excep_SCI2_TXI2(void)
{
    sci_isr_tx(2);
}
void INT_Excep_SCI3_TXI3(void)
{
    sci_isr_tx(3);
}
void INT_Excep_SCI4_TXI5(void)
{
    sci_isr_tx(5);
}
void INT_Excep_SCI6_TXI6(void)
{
    sci_isr_tx(6);
}

void SCI_RxEnable(int ch)
{
    switch(ch) {
    case 0:
        IEN(SCI0, RXI0)= 1;
        break;
        default:
        break;
    }
}

void SCI_RxDisable(int ch)
{
    switch(ch) {
    case 0:
        IEN(SCI0, RXI0)= 0;
        break;
        default:
        break;
    }
}

void SCI_TxEnable(int ch)
{
    switch(ch) {
    case 0:
        IEN(SCI0, TXI0)= 1;
        break;
        default:
        break;
    }
}

void SCI_TxDisable(int ch)
{
    switch(ch) {
    case 0:
        IEN(SCI0, TXI0)= 0;
        break;
        default:
        break;
    }
}

unsigned char SCI_Rx(int ch)
{
    unsigned char c;
    int i;
    //while (!RxFifo.ct) ;
    if (RxFifo.ct) {
        SCI_RxDisable(ch);
        i = RxFifo.ri;
        c = RxFifo.buff[i++];
        RxFifo.ri = i % SCI_BUF_SIZE;
        RxFifo.ct--;
        SCI_RxEnable(ch);
    } else
        c = 0;
    return c;
}

void SCI_Tx(int ch, unsigned char c)
{
    int i;
    volatile struct st_sci0 *sci = SCI[ch];
    while (TxFifo.ct >= SCI_BUF_SIZE)
        ;
    SCI_TxDisable(ch);
    if (TxFifo.run) {
        i = TxFifo.wi;
        TxFifo.buff[i++] = c;
        TxFifo.wi = i % SCI_BUF_SIZE;
        TxFifo.ct++;
    } else {
        sci->TDR = c;
        TxFifo.run = 1;
    }
    SCI_TxEnable(ch);
}

void SCI_TxStr(int ch, unsigned char *p)
{
    unsigned char c;
    while ((c = *p++) != 0) {
        if (c == '\n')
            SCI_Tx(ch, '\r');
        SCI_Tx(ch, c);
    }
}

static void sci_fifo_init()
{
    TxFifo.ri = 0;
    TxFifo.wi = 0;
    TxFifo.ct = 0;
    TxFifo.run = 0;
    RxFifo.ri = 0;
    RxFifo.wi = 0;
    RxFifo.ct = 0;
}

void sci_int_enable(int ch)
{
    switch(ch) {
    case 0:
        IPR(SCI0, RXI0)= 3;
        IEN(SCI0, RXI0) = 1;
        IPR(SCI0, TXI0) = 2;
        IEN(SCI0, TXI0) = 1;
        break;
        case 1:
        IPR(SCI1, RXI1) = 3;
        IEN(SCI1, RXI1) = 1;
        IPR(SCI1, TXI1) = 2;
        IEN(SCI1, TXI1) = 1;
        break;
        case 2:
        IPR(SCI2, RXI2) = 3;
        IEN(SCI2, RXI2) = 1;
        IPR(SCI2, TXI2) = 2;
        IEN(SCI2, TXI2) = 1;
        break;
        case 3:
        IPR(SCI3, RXI3) = 3;
        IEN(SCI3, RXI3) = 1;
        IPR(SCI3, TXI3) = 2;
        IEN(SCI3, TXI3) = 1;
        break;
        default:
        break;
    }
}

void SCI_Init(int ch, int baud)
{
    volatile struct st_sci0 *sci = SCI[ch];

    sci_fifo_init();
    SYSTEM.PRCR.WORD = 0xA502;
    switch(ch) {
    case 0:
        MSTP_SCI0 = 0;
        //PORT2.PMR.BIT.B0 = 0;
        //PORT2.PMR.BIT.B1= 0;
        MPC.PWPR.BIT.B0WI = 0; /* Enable write to PFSWE */
        MPC.PWPR.BIT.PFSWE = 1; /* Enable write to PFS */
        MPC.P20PFS.BYTE = 0x0A;
        MPC.P21PFS.BYTE = 0x0A;
        //MPC.PWPR.BYTE = 0x80;       /* Disable write to PFSWE and PFS*/
        PORT2.PMR.BIT.B0 = 1;
        PORT2.PMR.BIT.B1 = 1;
        break;
    case 1:
        MSTP_SCI1 = 0;
        PORT3.PMR.BIT.B0 = 1;
        break;
    case 2:
        MSTP_SCI2 = 0;
        PORT1.PMR.BIT.B2 = 1;
        break;
    case 3:
        MSTP_SCI3 = 0;
        PORT1.PMR.BIT.B6 = 1;
        break;
    default:
        break;
    }
    SYSTEM.PRCR.WORD = 0xA500;
    sci->SCR.BYTE = 0;
    sci->SMR.BYTE = 0x00;
    if (baud != 0)
        sci->BRR = (unsigned char)((int)PCLK / baud / 32 - 1);
    else
        sci->BRR = (unsigned char)((int)PCLK / 115200 / 32 - 1);
    delay_ms(1);
    sci->SCR.BYTE = 0xF0;
    sci_int_enable(ch);
}
