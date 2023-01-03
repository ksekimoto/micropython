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
#include "iodefine.h"
#include "interrupt_handlers.h"
#include "rx_utils.h"
#include "rx_ether.h"
#include "phy.h"

#if MICROPY_HW_ETH_MDC && MICROPY_PY_LWIP

typedef struct st_etherc etherc_t;
typedef struct st_edmac edmac_t;

volatile etherc_t *ether_reg = (volatile etherc_t *)(0xC0100);
volatile edmac_t *edmac_reg = (volatile edmac_t *)(0xC0000);

static ethfifo RX_DESC_SECTION rxdesc[ETH_BUF_NUM] __attribute__((aligned(32)));
static ethfifo TX_DESC_SECTION txdesc[ETH_BUF_NUM] __attribute__((aligned(32)));
static uint8_t ETH_BUF_SECTION rxbuf[ETH_BUF_NUM][ALIGNED_BUFSIZE] __attribute__((aligned(32)));
static uint8_t ETH_BUF_SECTION txbuf[ETH_BUF_NUM][ALIGNED_BUFSIZE] __attribute__((aligned(32)));

static RX_ETHER_INPUT_CB rx_ether_input_cb = (RX_ETHER_INPUT_CB)0;

/**
 * Ethernet device driver control structure initialization
 */
struct ei_device le0 =
{
    (uint8_t *)"eth0",  /* device name */
    0,      /* open */
    0,      /* Tx_act */
    0,      /* Rx_act */
    0,      /* txing */
    0,      /* irq lock */
    0,      /* dmaing */
    0,      /* current receive discripter */
    0,      /* current transmit discripter */
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

static void rx_phy_mii_write_1(void) {
    volatile uint32_t j;
    for (j = MDC_WAIT; j > 0; j--) {
        ether_reg->PIR.LONG = 0x00000006;
    }
    for (j = MDC_WAIT; j > 0; j--) {
        ether_reg->PIR.LONG = 0x00000007;
    }
    for (j = MDC_WAIT; j > 0; j--) {
        ether_reg->PIR.LONG = 0x00000007;
    }
    for (j = MDC_WAIT; j > 0; j--) {
        ether_reg->PIR.LONG = 0x00000006;
    }
}

static void rx_phy_mii_write_0(void) {
    volatile uint32_t j;
    for (j = MDC_WAIT; j > 0; j--) {
        ether_reg->PIR.LONG = 0x00000002;
    }
    for (j = MDC_WAIT; j > 0; j--) {
        ether_reg->PIR.LONG = 0x00000003;
    }
    for (j = MDC_WAIT; j > 0; j--) {
        ether_reg->PIR.LONG = 0x00000003;
    }
    for (j = MDC_WAIT; j > 0; j--) {
        ether_reg->PIR.LONG = 0x00000002;
    }
}

static void rx_phy_ta_z0(void) {
    volatile uint32_t j;
    for (j = MDC_WAIT; j > 0; j--) {
        ether_reg->PIR.LONG = 0x00000000;
    }
    for (j = MDC_WAIT; j > 0; j--) {
        ether_reg->PIR.LONG = 0x00000001;
    }
    for (j = MDC_WAIT; j > 0; j--) {
        ether_reg->PIR.LONG = 0x00000001;
    }
    for (j = MDC_WAIT; j > 0; j--) {
        ether_reg->PIR.LONG = 0x00000000;
    }
}

static void rx_phy_ta_10(void) {
    rx_phy_mii_write_1();
    rx_phy_mii_write_0();
}

static void rx_phy_preamble(void) {
    volatile uint16_t i = 32;
    while (i-- > 0) {
        rx_phy_mii_write_1();
    }
}

static void rx_phy_reg_set(uint16_t phy_addr, uint16_t reg_addr, int32_t option) {
    volatile uint32_t i;
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
            rx_phy_mii_write_0();
        } else {
            rx_phy_mii_write_1();
        }
        data <<= 1;
    }
}

static void rx_phy_reg_read(uint16_t *data) {
    volatile uint32_t i, j;
    uint16_t reg_data;
    reg_data = 0;
    i = 16;
    while (i-- > 0) {
        for (j = MDC_WAIT; j > 0; j--) {
            ether_reg->PIR.LONG = 0x00000000;
        }
        for (j = MDC_WAIT; j > 0; j--) {
            ether_reg->PIR.LONG = 0x00000001;
        }
        reg_data <<= 1;
        reg_data |= (uint16_t)((ether_reg->PIR.LONG & 0x00000008) >> 3); /* MDI read  */
        for (j = MDC_WAIT; j > 0; j--) {
            ether_reg->PIR.LONG = 0x00000001;
        }
        for (j = MDC_WAIT; j > 0; j--) {
            ether_reg->PIR.LONG = 0x00000000;
        }
    }
    *data = reg_data;
}

static void rx_phy_reg_write(uint16_t data) {
    volatile uint32_t i;
    i = 16;
    while (i-- > 0) {
        if ((data & 0x8000) == 0) {
            rx_phy_mii_write_0();
        } else {
            rx_phy_mii_write_1();
        }
        data <<= 1;
    }
}

static uint16_t rx_phy_read(uint16_t phy_addr, uint16_t reg_addr) {
    uint16_t data;
    uint32_t state = rx_disable_irq();
    rx_phy_preamble();
    rx_phy_reg_set(phy_addr, reg_addr, PHY_READ);
    rx_phy_ta_z0();
    rx_phy_reg_read(&data);
    rx_phy_ta_z0();
    rx_enable_irq(state);
    return data;
}

static void rx_phy_write(uint16_t phy_addr, uint16_t reg_addr, uint16_t data) {
    uint32_t state = rx_disable_irq();
    rx_phy_preamble();
    rx_phy_reg_set(phy_addr, reg_addr, PHY_WRITE);
    rx_phy_ta_10();
    rx_phy_reg_write(data);
    rx_phy_ta_z0();
    rx_enable_irq(state);
}

#if 0
static int16_t rx_phy_init(uint16_t phy_addr) {
    uint16_t reg;
    uint32_t count;
    rx_phy_write(phy_addr, BASIC_MODE_CONTROL_REG, 0x8000);
    count = 0;
    do {
        reg = rx_phy_read(phy_addr, BASIC_MODE_CONTROL_REG);
        count++;
    } while ((reg & 0x8000) && (count < PHY_RESET_WAIT));
    if (count >= PHY_RESET_WAIT) {
        return R_PHY_ERROR;
    } else {
        return R_PHY_OK;
    }
}

static void phy_set_100full(uint16_t phy_addr) {
    rx_phy_write(phy_addr, BASIC_MODE_CONTROL_REG, 0x2100);
}

static void phy_set_10half(uint16_t phy_addr) {
    rx_phy_write(phy_addr, BASIC_MODE_CONTROL_REG, 0x0000);
}

static int16_t phy_set_autonegotiate(uint16_t phy_addr) {
    uint16_t reg;
    volatile uint32_t count;
    rx_phy_write(phy_addr, AN_ADVERTISEMENT_REG, 0x01E1);
    rx_phy_write(phy_addr, BASIC_MODE_CONTROL_REG, 0x1200);
    count = 0;
    do {
        reg = rx_phy_read(phy_addr, BASIC_MODE_STATUS_REG);
        reg = rx_phy_read(phy_addr, BASIC_MODE_STATUS_REG);
        count++;
    } while (!(reg & 0x0020) && count < PHY_AUTO_NEGOTIATON_WAIT);

    if (count >= PHY_AUTO_NEGOTIATON_WAIT) {
        return R_PHY_ERROR;
    } else {
        return (int16_t)rx_phy_read(phy_addr, AN_LINK_PARTNER_ABILITY_REG);
    }
}
#endif

////////////////////////////////////////////////////////////////////////////
/// fifo operations
////////////////////////////////////////////////////////////////////////////

void rx_ether_fifo_init(ethfifo p[], uint32_t status) {
    ethfifo *current = 0;
    uint32_t i, j;

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

int32_t rx_ether_fifo_write(ethfifo *p, uint8_t buf[], uint32_t size) {
    uint32_t i;
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
    return (int32_t)i;
}

int32_t rx_ether_fifo_read(ethfifo *p, uint8_t buf[]) {
    uint32_t i, temp_size;
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
        return (int32_t)temp_size;
    }
}

////////////////////////////////////////////////////////////////////////////
/// rx_etherc operations
////////////////////////////////////////////////////////////////////////////

bool rx_ether_phy_write(uint32_t phy_addr, uint32_t reg_addr, uint32_t data, uint32_t retry) {
    rx_phy_write((uint16_t)phy_addr, (uint16_t)reg_addr, (uint16_t)data);
    return true;
}

bool rx_ether_phy_read(uint32_t phy_addr, uint32_t reg_addr, uint32_t *data, uint32_t retry) {
    *data = (uint32_t)rx_phy_read((uint16_t)phy_addr, (uint16_t)reg_addr);
    return true;
}

void rx_ether_set_link_speed(bool speed, bool fullduplex) {
    if (speed) {
        ether_reg->ECMR.BIT.RTM = 1;        // 100Mbps
    } else {
        ether_reg->ECMR.BIT.RTM = 0;        // 10Mbps
    }
    if (fullduplex) {
        ether_reg->ECMR.BIT.DM = 1;         // full duplex
    } else {
        ether_reg->ECMR.BIT.DM = 0;         // half duplex
    }
}

void rx_ether_init(uint8_t *hwaddr) {
    le0.open = 1;
    rx_ether_fifo_init(rxdesc, (uint32_t)ACT);
    rx_ether_fifo_init(txdesc, (uint32_t)0);
    le0.rxcurrent = &rxdesc[0];
    le0.txcurrent = &txdesc[0];
    le0.mac_addr[0] = hwaddr[0];
    le0.mac_addr[1] = hwaddr[1];
    le0.mac_addr[2] = hwaddr[2];
    le0.mac_addr[3] = hwaddr[3];
    le0.mac_addr[4] = hwaddr[4];
    le0.mac_addr[5] = hwaddr[5];

    edmac_reg->EDMR.BIT.SWR = 1;                // reset EDMAC and ETHERC0
    for (int i = 0; i < 100; i++) {
        __asm__ __volatile__ ("nop");
    }
    ether_reg->ECSR.LONG = 0x00000037;         // clear all ETHERC0 status BFR, PSRTO, LCHNG, MPD, ICD
    ether_reg->ECSIPR.LONG = 0x00000020;       // disable ETHERC0 status change interrupt
    ether_reg->RFLR.LONG = 1518;               // ether payload is 1500+ CRC
    ether_reg->IPGR.LONG = 0x00000014;         // Intergap is 96-bit time

    uint32_t mac_h, mac_l;
    mac_h = ((uint32_t)hwaddr[0] << 24) | ((uint32_t)hwaddr[1] << 16) | ((uint32_t)hwaddr[2] << 8) | (uint32_t)hwaddr[3];
    mac_l = ((uint32_t)hwaddr[4] << 8) | (uint32_t)hwaddr[5];
    if (mac_h == 0 && mac_l == 0) {
        // Do nothing
    } else {
        ether_reg->MAHR = mac_h;
        ether_reg->MALR.LONG = mac_l;
    }

    /* EDMAC */
    edmac_reg->EESR.LONG = 0x47FF0F9F;          // clear all EDMAC status bits
    edmac_reg->RDLAR = (void *)le0.rxcurrent;   // initialize Rx Descriptor List Address
    edmac_reg->TDLAR = (void *)le0.txcurrent;   // initialize Tx Descriptor List Address
    edmac_reg->TRSCER.LONG = 0x00000000;        // copy-back status is RFE & TFE only
    edmac_reg->TFTR.LONG = 0x00000000;          // threshold of Tx_FIFO
    edmac_reg->FDR.LONG = 0x00000707;           // transmit fifo & receive fifo is 2048 bytes
    // Configure receiving method
    // b0        RNR - Receive Request Bit Reset - Continuous reception of multiple frames is possible.
    // b1        RNC - Receive Request Bit Non-Reset Mode - The RR bit is automatically reset.
    // b31:b2    Reserved set to 0
    edmac_reg->RMCR.LONG = 0x00000001;
    edmac_reg->EDMR.BIT.DE = 1;

    // Initialize PHY
    phy_init();
}

void rx_ether_start(void) {
    #if defined(HW_INTERRUPT)
    // Enable interrupt
    // Sets up interrupt when you use interrupt
    edmac_reg->EESIPR.LONG = 0x00040000;
    #if defined(RX63N)
    // ICU.IER[4].BIT.IEN0 = 1;
    // ICU.IPR[8].BYTE = 4;    // Set priority level
    IPR(ETHER, EINT) = 4;
    IEN(ETHER, EINT) = 1;
    #elif defined(RX65N)
    // ICU.IER[4].BIT.IEN0 = 1;
    // ICU.IPR[8].BYTE = PRI_ETH;        // Set priority level
    IPR(EXDMAC, EXDMAC0I) = RX_PRI_ETH;  // IPR_EXDMAC_EXDMAC0I=126,
    IEN(EXDMAC, EXDMAC0I) = 1;           // IER_EXDMAC_EXDMAC0I=0x0F IEN_EXDMAC_EXDMAC0I IEN6
    /* Enable interrupts of interest only. */
    // ether_reg->ECSIPR.BIT.LCHNGIP = 1;
    // edmac_reg->EESIPR.BIT.ECIIP    = 1;
    /* Set Ethernet interrupt level and enable */
    ICU.IPR[IPR_ICU_GROUPAL1].BIT.IPR = 12;
    ICU.IER[IER_ICU_GROUPAL1].BIT.IEN1 = 1;
    ICU.GENAL1.BIT.EN4 = 1;
    #else
    #error "RX MCU Series is not specified."
    #endif
    #endif
    // Enable receive and transmit
    ether_reg->ECMR.BIT.RE = 1;
    ether_reg->ECMR.BIT.TE = 1;
    // Enable EDMAC receive
    edmac_reg->EDRRR.LONG = 0x00000001;
    for (int i = 0; i < 100; i++) {
        __asm__ __volatile__ ("nop");
    }
    return;
}

void rx_ether_deinit(void) {
    le0.open = 0;
    ether_reg->ECMR.LONG = 0x00000000;        // disable TE and RE
    le0.irqlock = 1;
    rx_ether_input_cb = (RX_ETHER_INPUT_CB)0;
}

void rx_ether_input_set_callback(RX_ETHER_INPUT_CB func) {
    rx_ether_input_cb = func;
}

void rx_ether_input_callback(void) {
    if (rx_ether_input_cb) {
        (*rx_ether_input_cb)();
    }
}

#endif // ICROPY_HW_HAS_ETHERNET && MICROPY_PY_LWIP
