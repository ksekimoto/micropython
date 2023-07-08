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

/******************************************************************************
* DISCLAIMER

* This software is supplied by Renesas Technology Corp. and is only
* intended for use with Renesas products. No other uses are authorized.

* This software is owned by Renesas Technology Corp. and is protected under
* all applicable laws, including copyright laws.

* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES
* REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY,
* INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE AND NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY
* DISCLAIMED.

* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
* TECHNOLOGY CORP. NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES
* FOR ANY REASON RELATED TO THE THIS SOFTWARE, EVEN IF RENESAS OR ITS
* AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

* Renesas reserves the right, without notice, to make changes to this
* software and to discontinue the availability of this software.
* By using this software, you agree to the additional terms and
* conditions found by accessing the following link:
* http://www.renesas.com/disclaimer
******************************************************************************
* Copyright (C) 2008. Renesas Technology Corp., All Rights Reserved.
*******************************************************************************
* File Name    : r_ether.c
* Version      : 1.03
* Description  : Ethernet module device driver
******************************************************************************
* History : DD.MM.YYYY Version Description
*         : 15.02.2010 1.00    First Release
*         : 03.03.2010 1.01    Buffer size is aligned on the 32-byte boundary.
*         : 08.03.2010 1.02    Modification of receiving method
*         : 06.04.2010 1.03    RX62N chnages
******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "py/runtime.h"
#include "common.h"
#include "RZ_A2M.h"
#include "iodefine.h"
#include "edmac_iodefine.h"
#include "rz_utils.h"
#include "rz_ether.h"
#include "phy.h"
#if defined(MBED_OS_EMAC)
#include "r_ether_rza2_if.h"
#endif

#if MICROPY_HW_ETH_MDC && MICROPY_PY_LWIP

#define ETH_NUM 2

static ethfifo rxdesc[ETH_BUF_NUM] __attribute((section("NC_BSS"), aligned(32)));
static ethfifo txdesc[ETH_BUF_NUM] __attribute((section("NC_BSS"), aligned(32)));
static int8_t rxbuf[ETH_BUF_NUM][ALIGNED_BUFSIZE] __attribute((section("NC_BSS"), aligned(32)));
static int8_t txbuf[ETH_BUF_NUM][ALIGNED_BUFSIZE] __attribute((section("NC_BSS"), aligned(32)));

typedef volatile struct st_etherc *st_etherc_p;
static st_etherc_p ETHERCP[ETH_NUM] = {
    (st_etherc_p)0xE8204100,
    (st_etherc_p)0xE8204300
};
typedef volatile struct st_edmac *st_edmac_p;
static st_edmac_p EDMACP[ETH_NUM] = {
    (st_edmac_p)0xE8204000,
    (st_edmac_p)0xE8204200
};

static RZ_ETHER_INPUT_CB rz_ether_input_cb[ETH_NUM] = {
    (RZ_ETHER_INPUT_CB)0,
    (RZ_ETHER_INPUT_CB)0
};
static IRQn_Type rz_ether_irqn[ETH_NUM] = {
    EINT0_IRQn,
    EINT1_IRQn
};

void rz_ether_input_set_callback(uint32_t ch, RZ_ETHER_INPUT_CB func) {
    rz_ether_input_cb[ch] = func;
}

void rz_ether_input_callback(uint32_t ch) {
    volatile st_etherc_p ethercp = ETHERCP[ch];
    volatile st_edmac_p edmacp = EDMACP[ch];
    uint32_t status_ecsr;
    uint32_t status_eesr;
    status_ecsr = ethercp->ECSR.LONG;
    status_eesr = edmacp->EESR.LONG;
    if (rz_ether_input_cb[ch]) {
        (*rz_ether_input_cb[ch])();
    }
    if (status_eesr & (1UL << 22)) {
        ethercp->ECSR.LONG = status_ecsr;
    }
    edmacp->EESR.LONG = status_eesr;
}

void rz_ether_input_callback0(void) {
    rz_ether_input_callback(0);
}

void rz_ether_input_callback1(void) {
    rz_ether_input_callback(1);
}

static void (*rz_ether_callback[ETH_NUM])() = {
    rz_ether_input_callback0,
    rz_ether_input_callback1
};

/**
 * Ethernet device driver control structure initialization
 */
struct ei_device le0 =
{
    (int8_t *)"eth0",   /* device name */
    0,      /* open */
    0,      /* Tx_act */
    0,      /* Rx_act */
    0,      /* txing */
    0,      /* irq lock */
    0,      /* dmaing */
    0,      /* current receive descripter */
    0,      /* current transmit descripter */
    0,      /* save irq */
    {
        0,  /* rx packets */
        0,  /* tx packets */
        0,  /* rx errors */
        0,  /* tx errors */
        0,  /* rx dropped */
        0,  /* tx dropped */
        0,  /* multicast */
        0,  /* collisions */

        0,  /* rx length errors */
        0,  /* rx over errors */
        0,  /* rx CRC errors */
        0,  /* rx frame errors */
        0,  /* rx fifo errors */
        0,  /* rx missed errors */

        0,  /* tx aborted errors */
        0,  /* tx carrier errors */
        0,  /* tx fifo errors */
        0,  /* tx heartbeat errors */
        0   /* tx window errors */
    },
    {
        0,  /* MAC 0 */
        0,  /* MAC 1 */
        0,  /* MAC 2 */
        0,  /* MAC 3 */
        0,  /* MAC 4 */
        0   /* MAC 5 */
    }
};

////////////////////////////////////////////////////////////////////////////
/// routines for phy operations
////////////////////////////////////////////////////////////////////////////

static void rz_phy_mii_write_1(uint32_t ch) {
    volatile int32_t j;
    volatile st_etherc_p ethercp = ETHERCP[ch];
    uint32_t *pir = (uint32_t *)&(ethercp->PIR.LONG);

    for (j = MDC_WAIT; j > 0; j--) {
        (*pir) = 0x00000006;
    }
    for (j = MDC_WAIT; j > 0; j--) {
        (*pir) = 0x00000007;
    }
    for (j = MDC_WAIT; j > 0; j--) {
        (*pir) = 0x00000007;
    }
    for (j = MDC_WAIT; j > 0; j--) {
        (*pir) = 0x00000006;
    }
}

static void rz_phy_mii_write_0(uint32_t ch) {
    volatile int32_t j;
    volatile st_etherc_p ethercp = ETHERCP[ch];
    uint32_t *pir = (uint32_t *)&(ethercp->PIR.LONG);

    for (j = MDC_WAIT; j > 0; j--) {
        (*pir) = 0x00000002;
    }
    for (j = MDC_WAIT; j > 0; j--) {
        (*pir) = 0x00000003;
    }
    for (j = MDC_WAIT; j > 0; j--) {
        (*pir) = 0x00000003;
    }
    for (j = MDC_WAIT; j > 0; j--) {
        (*pir) = 0x00000002;
    }
}

static void rz_phy_ta_z0(uint32_t ch) {
    volatile int32_t j;
    volatile st_etherc_p ethercp = ETHERCP[ch];
    uint32_t *pir = (uint32_t *)&(ethercp->PIR.LONG);

    for (j = MDC_WAIT; j > 0; j--) {
        (*pir) = 0x00000000;
    }
    for (j = MDC_WAIT; j > 0; j--) {
        (*pir) = 0x00000001;
    }
    for (j = MDC_WAIT; j > 0; j--) {
        (*pir) = 0x00000001;
    }
    for (j = MDC_WAIT; j > 0; j--) {
        (*pir) = 0x00000000;
    }
}

static void rz_phy_ta_10(uint32_t ch) {
    rz_phy_mii_write_1(ch);
    rz_phy_mii_write_0(ch);
}

static void rz_phy_preamble(uint32_t ch) {
    volatile int16_t i = 32;
    while (i-- > 0) {
        rz_phy_mii_write_1(ch);
    }
}

static void rz_phy_reg_set(uint32_t ch, uint16_t phy_addr, uint16_t reg_addr, int32_t option) {
    volatile int32_t i;
    uint16_t data = (PHY_ST << 14); /* ST code */
    if (option == PHY_READ) {
        data |= (PHY_READ << 12);   /* OP code(RD) */
    } else {
        data |= (PHY_WRITE << 12);  /* OP code(WT) */
    }
    data |= (phy_addr << 7); /* PHY Address  */
    data |= (reg_addr << 2); /* Reg Address  */
    i = 14;
    while (i-- > 0) {
        if ((data & 0x8000) == 0) {
            rz_phy_mii_write_0(ch);
        } else {
            rz_phy_mii_write_1(ch);
        }
        data <<= 1;
    }
}

static void rz_phy_reg_read(uint32_t ch, uint16_t *data) {
    volatile int32_t i, j;
    uint16_t reg_data;
    volatile st_etherc_p ethercp = ETHERCP[ch];
    uint32_t *pir = (uint32_t *)&(ethercp->PIR.LONG);
    reg_data = 0;
    i = 16;
    while (i-- > 0) {
        for (j = MDC_WAIT; j > 0; j--) {
            (*pir) = 0x00000000;
        }
        for (j = MDC_WAIT; j > 0; j--) {
            (*pir) = 0x00000001;
        }
        reg_data <<= 1;
        reg_data |= (uint16_t)(((*pir) & 0x00000008) >> 3); /* MDI read  */
        for (j = MDC_WAIT; j > 0; j--) {
            (*pir) = 0x00000001;
        }
        for (j = MDC_WAIT; j > 0; j--) {
            (*pir) = 0x00000000;
        }
    }
    *data = reg_data;
}

static void rz_phy_reg_write(uint32_t ch, uint16_t data) {
    volatile int32_t i;
    i = 16;
    while (i-- > 0) {
        if ((data & 0x8000) == 0) {
            rz_phy_mii_write_0(ch);
        } else {
            rz_phy_mii_write_1(ch);
        }
        data <<= 1;
    }
}

static uint16_t rz_phy_read(uint32_t ch, uint16_t phy_addr, uint16_t reg_addr) {
    uint16_t data;
//    uint32_t state = rz_disable_irq();
    rz_phy_preamble(ch);
    rz_phy_reg_set(ch, phy_addr, reg_addr, PHY_READ);
    rz_phy_ta_z0(ch);
    rz_phy_reg_read(ch, &data);
    rz_phy_ta_z0(ch);
//    rz_enable_irq(state);
    return data;
}

static void rz_phy_write(uint32_t ch, uint16_t phy_addr, uint16_t reg_addr, uint16_t data) {
//    uint32_t state = rz_disable_irq();
    rz_phy_preamble(ch);
    rz_phy_reg_set(ch, phy_addr, reg_addr, PHY_WRITE);
    rz_phy_ta_10(ch);
    rz_phy_reg_write(ch, data);
    rz_phy_ta_z0(ch);
//    rz_enable_irq(state);
}

////////////////////////////////////////////////////////////////////////////
/// fifo operations
////////////////////////////////////////////////////////////////////////////

void rz_ether_fifo_init(ethfifo p[], uint32_t status) {
    ethfifo *current = 0;
    int32_t i, j;

    for (i = 0; i < ETH_BUF_NUM; i++) {
        current = &p[i];
        if (status == 0) {
            current->buf_p = (uint8_t *)&txbuf[i][0];
        } else {
            current->buf_p = (uint8_t *)&rxbuf[i][0];
        }
        for (j = 0; j < ALIGNED_BUFSIZE; j++) {
            current->buf_p[j] = 0;
        }
        current->bufsize = ALIGNED_BUFSIZE;
        current->size = 0;
        current->status = status;
        current->next = &p[i + 1];
    }
    current->status |= DL;
    current->next = &p[0];
}

int32_t rz_ether_fifo_write(uint32_t ch, ethfifo *p, int8_t buf[], int32_t size) {
    int32_t i;
    ethfifo *current = p;

    if ((current->status & ACT) != 0) {
        /**
         * Current descriptor is active and ready to transmit or transmitting.
         **/
        return -1;
    }
    for (i = 0; i < size; i++) {
        if (i >= ALIGNED_BUFSIZE) {
            break;
        } else {
            /* transfer packet data */
            current->buf_p[i] = buf[i];
        }
    }
    current->bufsize = (uint16_t)i;
    return i;
}

int32_t rz_ether_fifo_read(uint32_t ch, ethfifo *p, int8_t buf[]) {
    int32_t i, temp_size;
    ethfifo *current = p;

    if ((current->status & ACT) != 0) {
        /**
         * Current descriptor is active and ready to receive or receiving.
         * This is not an error.
         **/
        return -1;
    } else if ((current->status & FE) != 0) {
        /**
         * Frame error.
         * Must move to new descriptor as E-DMAC now points to next one.
         **/
        return -2;
    } else {
        if ((current->status & FP0) == FP0) {
            /* This is the last descriptor.  Complete frame is received.   */
            temp_size = current->size;
            while (temp_size > ALIGNED_BUFSIZE) {
                temp_size -= ALIGNED_BUFSIZE;
            }
        } else {
            /**
             * This is either a start or continuos descriptor.
             * Complete frame is NOT received.
             **/
            temp_size = ALIGNED_BUFSIZE;
        }
        /* Copy received data from receive buffer to user buffer */
        for (i = 0; i < temp_size; i++) {
            buf[i] = current->buf_p[i];
        }
        /* Return data size received */
        return temp_size;
    }
}

////////////////////////////////////////////////////////////////////////////
/// rz_etherc operations
////////////////////////////////////////////////////////////////////////////

bool rz_ether_phy_write(uint32_t ch, uint32_t phy_addr, uint32_t reg_addr, uint32_t data, uint32_t retry) {
    rz_phy_write(ch, (uint16_t)phy_addr, (uint16_t)reg_addr, (uint16_t)data);
    return true;
}

bool rz_ether_phy_read(uint32_t ch, uint32_t phy_addr, uint32_t reg_addr, uint32_t *data, uint32_t retry) {
    *data = (uint32_t)rz_phy_read(ch, (uint16_t)phy_addr, (uint16_t)reg_addr);
    return true;
}

void rz_ether_set_link_speed(uint32_t ch, bool speed, bool fullduplex) {
    volatile st_etherc_p ethercp = ETHERCP[ch];
    if (speed) {
        ethercp->ECMR.BIT.RTM = 1;        // 100Mbps
    } else {
        ethercp->ECMR.BIT.RTM = 0;        // 10Mbps
    }
    if (fullduplex) {
        ethercp->ECMR.BIT.DM = 1;         // full duplex
    } else {
        ethercp->ECMR.BIT.DM = 0;         // half duplex
    }
}

void rz_ether_init(uint32_t ch, uint8_t *hwaddr) {
    volatile st_etherc_p ethercp = ETHERCP[ch];
    volatile st_edmac_p edmacp = EDMACP[ch];

    le0.open = 1;
    rz_ether_fifo_init(rxdesc, (uint32_t)ACT);
    rz_ether_fifo_init(txdesc, (uint32_t)0);
    le0.rxcurrent = &rxdesc[0];
    le0.txcurrent = &txdesc[0];
    le0.mac_addr[0] = hwaddr[0];
    le0.mac_addr[1] = hwaddr[1];
    le0.mac_addr[2] = hwaddr[2];
    le0.mac_addr[3] = hwaddr[3];
    le0.mac_addr[4] = hwaddr[4];
    le0.mac_addr[5] = hwaddr[5];

    edmacp->EDMR.BIT.SWR = 1;                // reset EDMAC and ETHERC0
    for (int i = 0; i < 100; i++) {
        __asm__ __volatile__ ("nop");
    }
    ethercp->ECSR.LONG = 0x00000037;         // clear all ETHERC0 status BFR, PSRTO, LCHNG, MPD, ICD
    ethercp->ECSIPR.LONG = 0x00000020;       // disable ETHERC0 status change interrupt
    ethercp->RFLR.LONG = 1518;               // ether payload is 1500+ CRC
    ethercp->IPGR.LONG = 0x00000014;         // Intergap is 96-bit time

    uint32_t mac_h, mac_l;
    mac_h = ((uint32_t)hwaddr[0] << 24) | ((uint32_t)hwaddr[1] << 16) | ((uint32_t)hwaddr[2] << 8) | (uint32_t)hwaddr[3];
    mac_l = ((uint32_t)hwaddr[4] << 8) | (uint32_t)hwaddr[5];
    if (mac_h == 0 && mac_l == 0) {
        // Do nothing
    } else {
        ethercp->MAHR.LONG = mac_h;
        ethercp->MALR.LONG = mac_l;
    }

    /* EDMAC */
    edmacp->EESR.LONG = 0x47FF0F9F;          // clear all EDMAC status bits
    edmacp->RDLAR.LONG = (unsigned long)le0.rxcurrent;   // initialize Rx Descriptor List Address
    edmacp->TDLAR.LONG = (unsigned long)le0.txcurrent;   // initialize Tx Descriptor List Address
    edmacp->TRSCER.LONG = 0x00000000;        // copy-back status is RFE & TFE only
    edmacp->TFTR.LONG = 0x00000000;          // threshold of Tx_FIFO
    edmacp->FDR.LONG = 0x0000070F;           // transmit fifo & receive fifo is 2048 bytes
    // Configure receiving method
    // b0        RNR - Receive Request Bit Reset - Continuous reception of multiple frames is possible.
    // b31:b1    Reserved set to 0
    edmacp->RMCR.LONG = 0x00000001;
    edmacp->EDMR.BIT.DE = 1;
    // Initialize PHY
    phy_init(ch);
    // phy_start_autonegotiate (ch, ETHER_FLAG_ON);
}

void rz_ether_tx_req(uint32_t ch) {
    volatile st_edmac_p edmacp = EDMACP[ch];
    if (edmacp->EDTRR.LONG == 0x00000000) {
        edmacp->EDTRR.LONG = 0x00000001;
    }
}

void rz_ether_rx_frame(uint32_t ch) {
    volatile st_edmac_p edmacp = EDMACP[ch];
    if ((edmacp->EESR.LONG & 0x00040000) != 0) {
        edmacp->EESR.LONG |= 0x00040000;
    }
}

void rz_ether_rx_req(uint32_t ch) {
    volatile st_edmac_p edmacp = EDMACP[ch];
    if (edmacp->EDRRR.LONG == 0x00000000L) {
        edmacp->EDRRR.LONG = 0x00000001L; /* Restart if stopped */
    }
}

void rz_ether_start(uint32_t ch) {
    volatile st_etherc_p ethercp = ETHERCP[ch];
    volatile st_edmac_p edmacp = EDMACP[ch];
    #if defined(HW_INTERRUPT)
    // Enable interrupt
    // ethercp->ECSIPR.BIT.LCHNGIP = 1;    /* Enable interrupts of interest only. */
    // edmacp->EESIPR.BIT.ECIIP = 1;
    edmacp->EESIPR.BIT.FRIP = 1;
    InterruptHandlerRegister(rz_ether_irqn[ch], rz_ether_callback[ch]);
    GIC_SetPriority(rz_ether_irqn[ch], 0x80);
    GIC_EnableIRQ(rz_ether_irqn[ch]);
    #endif
    // Enable receive and transmit
    ethercp->ECMR.BIT.RE = 1;
    ethercp->ECMR.BIT.TE = 1;
    // Enable EDMAC receive
    edmacp->EDRRR.LONG = 0x00000001;
    for (int i = 0; i < 100; i++) {
        __asm__ __volatile__ ("nop");
    }
    return;
}

void rz_ether_deinit(uint32_t ch) {
    volatile st_etherc_p ethercp = ETHERCP[ch];
    GIC_DisableIRQ(rz_ether_irqn[ch]);
    le0.open = 0;
    ethercp->ECMR.LONG = 0x00000000;        // disable TE and RE
    le0.irqlock = 1;
    rz_ether_input_cb[ch] = (RZ_ETHER_INPUT_CB)0;
}


#endif // ICROPY_HW_HAS_ETHERNET && MICROPY_PY_LWIP
