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

#include <stdio.h>
#include <stdint.h>
#include "common.h"
#include "iodefine.h"
#include "interrupt_handlers.h"
#include "rx63n_i2c.h"

//#define DEBUG_I2C
//#define DEBUG_I2C_REG_DUMP
//#define DEBUG_I2C_CLK
//#define DEBUG_I2C_INT
//#define DEBUG_I2C_ERR
//#define DEBUG_I2C_NACK
//#define DEBUG_I2C_DETAIL
//#define DEBUG_I2C_TX_DATA
//#define DEBUG_I2C_RX_DATA

#define RX_I2C_PRIORITY 4
#define I2C_TIMEOUT_STOP_CONDITION 100000
#define I2C_TIMEOUT_BUS_BUSY 100000

xaction_t *current_xaction;
xaction_unit_t *current_xaction_unit;

static const uint8_t i2c_scl_pins[] = {
    P12,      /* P12 */
    P21,      /* P21 */
    P16,      /* P16 */
    PC0,      /* PC0 */
};

static const uint8_t i2c_sda_pins[] = {
    P13,      /* P13 */
    P20,      /* P20 */
    P17,      /* P17 */
    PC1,      /* PC1 */
};

riic_t *RIIC[] = {
    (riic_t *)0x88300,
    (riic_t *)0x88320,
    (riic_t *)0x88340,
    (riic_t *)0x88360
};

static uint8_t pclk_div[8] = {
    1, 2, 4, 8, 16, 32, 64, 128
};

void  rx_i2c_get_pins(uint32_t ch, uint8_t *scl, uint8_t *sda) {
    *scl = i2c_scl_pins[ch];
    *sda = i2c_sda_pins[ch];
}

static void delay(int32_t t) {
    while (t-- > 0) {
        __asm__ __volatile__("nop");
    }
}

static void rx_i2c_clear_line(uint32_t ch) {
    riic_t *riicp = RIIC[ch];
    if (riicp->ICCR1.BIT.SDAI == 0) {
        int32_t t = 10;
        while (t-- > 0) {
            riicp->ICCR1.BIT.CLO = 1;
            while (riicp->ICCR1.BIT.CLO == 1)
                ;
            if (riicp->ICCR1.BIT.SDAI == 1)
                break;
        }
    }
}

#if defined(DEBUG_I2C_REG_DUMP)
static void i2c_reg_dump(uint32_t ch) {
    riic_t *riicp = RIIC[ch];
    debug_printf("CR1=%02X CR2=%02X MR1=%02X MR2=%02X MR3=%02X\r\n",
        riicp->ICCR1.BYTE, riicp->ICCR2.BYTE, riicp->ICMR1.BYTE, riicp->ICMR3.BYTE, riicp->ICMR3.BYTE);
    debug_printf("FER=%02X SER=%02X IER=%02X SR1=%02X SR2=%02X\r\n",
        riicp->ICFER.BYTE, riicp->ICSER.BYTE, riicp->ICIER.BYTE, riicp->ICSR1.BYTE, riicp->ICSR2.BYTE);
}
#endif

static void rx_i2c_ipr(uint32_t ch, uint32_t val) {
    switch (ch) {
    case 0:
        IPR(RIIC0,EEI0)= val;
        IPR(RIIC0,RXI0) = val;
        IPR(RIIC0,TXI0) = val;
        IPR(RIIC0,TEI0) = val;
        break;
        case 1:
        IPR(RIIC1,EEI1) = val;
        IPR(RIIC1,RXI1) = val;
        IPR(RIIC1,TXI1) = val;
        IPR(RIIC1,TEI1) = val;
        break;
        case 2:
        IPR(RIIC2,EEI2) = val;
        IPR(RIIC2,RXI2) = val;
        IPR(RIIC2,TXI2) = val;
        IPR(RIIC2,TEI2) = val;
        break;
        case 3:
        IPR(RIIC3,EEI3) = val;
        IPR(RIIC3,RXI3) = val;
        IPR(RIIC3,TXI3) = val;
        IPR(RIIC3,TEI3) = val;
        break;
    }
}

static void rx_i2c_ir(uint32_t ch, uint32_t val) {
    switch (ch) {
    case 0:
        IR(RIIC0,EEI0)= val;
        IR(RIIC0,RXI0) = val;
        IR(RIIC0,TXI0) = val;
        IR(RIIC0,TEI0) = val;
        break;
        case 1:
        IR(RIIC1,EEI1) = val;
        IR(RIIC1,RXI1) = val;
        IR(RIIC1,TXI1) = val;
        IR(RIIC1,TEI1) = val;
        break;
        case 2:
        IR(RIIC2,EEI2) = val;
        IR(RIIC2,RXI2) = val;
        IR(RIIC2,TXI2) = val;
        IR(RIIC2,TEI2) = val;
        break;
        case 3:
        IR(RIIC3,EEI3) = val;
        IR(RIIC3,RXI3) = val;
        IR(RIIC3,TXI3) = val;
        IR(RIIC3,TEI3) = val;
        break;
    }
}

static void rx_i2c_ien(uint32_t ch, uint32_t val) {
    switch (ch) {
    case 0:
        IEN(RIIC0,EEI0)= val;
        IEN(RIIC0,RXI0) = val;
        IEN(RIIC0,TXI0) = val;
        IEN(RIIC0,TEI0) = val;
        break;
        case 1:
        IEN(RIIC1,EEI1) = val;
        IEN(RIIC1,RXI1) = val;
        IEN(RIIC1,TXI1) = val;
        IEN(RIIC1,TEI1) = val;
        break;
        case 2:
        IEN(RIIC2,EEI2) = val;
        IEN(RIIC2,RXI2) = val;
        IEN(RIIC2,TXI2) = val;
        IEN(RIIC2,TEI2) = val;
        break;
        case 3:
        IEN(RIIC3,EEI3) = val;
        IEN(RIIC3,RXI3) = val;
        IEN(RIIC3,TXI3) = val;
        IEN(RIIC3,TEI3) = val;
        break;
    }
}

static void rx_i2c_pin(uint32_t ch) {
    switch (ch) {
    case 0:
        PORT1.PCR.BIT.B2 = 0;
        PORT1.PCR.BIT.B3 = 0;
        PORT1.PDR.BIT.B2 = 0;
        PORT1.PDR.BIT.B3 = 0;
        PORT1.PMR.BIT.B2 = 0;
        PORT1.PMR.BIT.B3 = 0;
        PORT1.ODR0.BIT.B4 = 1;
        PORT1.ODR0.BIT.B6 = 1;
        MPC.PWPR.BYTE = 0x40;
        MPC.P12PFS.BIT.PSEL = 0x0F;
        MPC.P13PFS.BIT.PSEL = 0x0F;
        //MPC.PWPR.BIT.PFSWE = 0;
        //MPC.PWPR.BIT.B0WI  = 1;
        PORT1.PMR.BIT.B2 = 1;
        PORT1.PMR.BIT.B3 = 1;
        break;
    case 1:
        PORT1.PMR.BIT.B2 = 0;
        PORT1.PMR.BIT.B3 = 0;
        MPC.PWPR.BIT.B0WI = 0;
        MPC.PWPR.BIT.PFSWE = 1;
        MPC.P12PFS.BYTE = 0x0F;
        MPC.P13PFS.BYTE = 0x0F;
        MPC.PWPR.BIT.PFSWE = 0;
        MPC.PWPR.BIT.B0WI = 1;
        PORT1.PMR.BIT.B2 = 1;
        PORT1.PMR.BIT.B3 = 1;
        PORT1.ODR0.BIT.B4 = 1;
        PORT1.ODR0.BIT.B6 = 1;
        break;
    case 2:
        PORT1.PMR.BIT.B2 = 0;
        PORT1.PMR.BIT.B3 = 0;
        MPC.PWPR.BIT.B0WI = 0;
        MPC.PWPR.BIT.PFSWE = 1;
        MPC.P12PFS.BYTE = 0x0F;
        MPC.P13PFS.BYTE = 0x0F;
        MPC.PWPR.BIT.PFSWE = 0;
        MPC.PWPR.BIT.B0WI = 1;
        PORT1.PMR.BIT.B2 = 1;
        PORT1.PMR.BIT.B3 = 1;
        PORT1.ODR0.BIT.B4 = 1;
        PORT1.ODR0.BIT.B6 = 1;
        break;
    case 3:
        PORT1.PMR.BIT.B2 = 0;
        PORT1.PMR.BIT.B3 = 0;
        MPC.PWPR.BIT.B0WI = 0;
        MPC.PWPR.BIT.PFSWE = 1;
        MPC.P12PFS.BYTE = 0x0F;
        MPC.P13PFS.BYTE = 0x0F;
        MPC.PWPR.BIT.PFSWE = 0;
        MPC.PWPR.BIT.B0WI = 1;
        PORT1.PMR.BIT.B2 = 1;
        PORT1.PMR.BIT.B3 = 1;
        PORT1.ODR0.BIT.B4 = 1;
        PORT1.ODR0.BIT.B6 = 1;
        break;
    }
}

static void rx_i2c_power(uint32_t ch) {
    SYSTEM.PRCR.WORD = 0xA502;
    switch (ch) {
    case 0:
        MSTP(RIIC0) = 0;
        break;
    case 1:
        MSTP(RIIC1) = 0;
        break;
    case 2:
        MSTP(RIIC2) = 0;
        break;
    case 3:
        MSTP(RIIC3) = 0;
        break;
    }
    SYSTEM.PRCR.WORD = 0xA500;
}

void rx_i2c_init(uint32_t ch) {
    riic_t *riicp = RIIC[ch];

    rx_i2c_power(ch);
    rx_i2c_ipr(ch, RX_I2C_PRIORITY);
    rx_i2c_ir(ch, 0);
    rx_i2c_ien(ch, 0);
    rx_i2c_pin(ch);

    riicp->ICCR1.BIT.ICE = 0;       // I2C disable
    riicp->ICCR1.BIT.IICRST = 1;    // I2C internal reset
    riicp->ICCR1.BIT.ICE = 1;       // I2C enable
    // Standard Mode
    riicp->ICMR1.BIT.CKS = 0x03;    // I2C CKS=3 then n=8, PCLK/n = 6000000
    riicp->ICBRH.BIT.BRH = 24;      // Set High width of SCL
    riicp->ICBRL.BIT.BRL = 29;      // Set Low width of SCL
    //riicp->ICFER.BYTE = 0x72;       // Default
    //riicp->ICMR2.BIT.TMOS = 0x01;
    //riicp->ICMR2.BIT.TMOL = 0x01;
    //riicp->ICMR2.BIT.TMOH = 0x01;
    //riicp->ICMR3.BIT.WAIT = 0x01;
    //riicp->ICMR3.BIT.NF = 0x02;
    //riicp->ICSR1.BYTE = 0x00;
    //riicp->ICSR2.BYTE = 0x00;
    riicp->ICSER.BYTE = 0x00;       // I2C reset bus status enable register
    riicp->ICMR3.BIT.ACKWP = 0x01;  // I2C allow to write ACKBT (transfer acknowledge bit)
    riicp->ICIER.BYTE = 0xF0;       // b0: Disable Time Out interrupt */
                                    // b1: Disable Arbitration Lost interrupt */
                                    // b2: Disable Start Condition Detection Interrupt */
                                    // b3: Disable Stop condition detection interrupt */
                                    // b4: Disable NACK reception interrupt */
                                    // b5: Enable Receive Data Full Interrupt */
                                    // b6: Enable Transmit End Interrupt */
                                    // b7: Enable Transmit Data Empty Interrupt */
    riicp->ICCR1.BIT.IICRST = 0;    // I2C internal reset
    rx_i2c_clear_line(ch);
#if defined(DEBUG_I2C)
    debug_printf("rx_i2c_init\r\n");
#endif
    return;
}

void rx_i2c_deinit(uint32_t ch) {
    riic_t *riicp = RIIC[ch];

    riicp->ICIER.BYTE = 0;           // I2C interrupt disable
    riicp->ICCR1.BIT.ICE = 0;        // I2C disable
#if defined(DEBUG_I2C)
    debug_printf("rx_i2c_deinit\r\n");
#endif
    return;
}

void rx_i2c_read_last_byte(uint32_t ch) {
    riic_t *riicp = RIIC[ch];
    xaction_t *xaction = current_xaction;
    xaction_unit_t *unit = &xaction->units[xaction->m_current];
    while ((riicp->ICSR2.BIT.RDRF) == 0);
    if(xaction->m_current == xaction->m_num_of_units) {
        riicp->ICSR2.BIT.STOP = 0;
        riicp->ICCR2.BIT.SP = 1;
        rx_i2c_unit_read_byte(ch, unit);
        riicp->ICMR3.BIT.WAIT = 0;
        uint32_t timeout = I2C_TIMEOUT_STOP_CONDITION;
        while (timeout-- > 0) {
            if (riicp->ICSR2.BIT.STOP != 0)
                break;
        }
        riicp->ICSR2.BIT.NACKF = 0;
        riicp->ICSR2.BIT.STOP = 0;
        //xAction->Signal(I2C_HAL_XACTION::c_Status_Completed);//Complete
    } else {
        rx_i2c_unit_read_byte(ch, unit);
        rx_i2c_xaction_start(ch, xaction, true);
    }
}

void rx_i2c_stop_confition(uint32_t ch) {
    riic_t *riicp = RIIC[ch];
    riicp->ICSR2.BIT.NACKF = 0;
    riicp->ICSR2.BIT.STOP = 0;
    riicp->ICCR2.BIT.SP= 1;
    uint32_t timeout = I2C_TIMEOUT_STOP_CONDITION;
    while (riicp->ICSR2.BIT.STOP == 0) {
        if (timeout-- == 0) {
#ifdef DEBUG_I2C
            debug_printf("I2C SC TO\r\n");
#endif
            break;
        }
    }
    riicp->ICSR2.BIT.STOP = 0;
#ifdef DEBUG_I2C
    debug_printf("I2C SC OK\r\n");
#endif
}

void rx_i2c_abort(uint32_t ch) {
    uint8_t dummy;
    riic_t *riicp = RIIC[ch];
    xaction_t *xaction = current_xaction;
    rx_i2c_stop_confition(ch);
    riicp->ICIER.BYTE = 0x00;
    while (0x00 != riicp->ICIER.BYTE) ;
    rx_i2c_ir(ch, 0);
    dummy = riicp->ICDRR;
    riicp->ICCR1.BIT.ICE = 0;
    //xAction->Signal(I2C_HAL_XACTION::c_Status_rx_i2c_aborted);
}

void rx_i2c_xaction_start(uint32_t ch, xaction_t *xaction, bool repeated_start) {
    uint8_t address;
    uint32_t timeout;
    riic_t *riicp = RIIC[ch];

    current_xaction = xaction;
    current_xaction_unit = &xaction->units[xaction->m_current];
    xaction_unit_t *unit = current_xaction_unit;

    riicp->ICCR1.BIT.ICE = 0;    // I2C disable
    riicp->ICMR1.BYTE = xaction->m_clock;
    riicp->ICBRH.BIT.BRH = xaction->m_clock;
    riicp->ICBRL.BIT.BRL = xaction->m_clock;
    riicp->ICIER.BYTE = 0xF0;
    riicp->ICCR1.BIT.ICE = 1;    // I2C enable
#ifdef DEBUG_I2C_REG_DUMP
        i2c_reg_dump(ch);
#endif

    address = 0xFE & (xaction->m_address << 1);
    address |= unit->m_fread ? 0x1 : 0x0;

    riicp->ICCR2.BIT.MST = 1;    // I2C master
    riicp->ICCR2.BIT.TRS = 1;    // I2C transmit
    if (repeated_start) {
        while (riicp->ICCR2.BIT.BBSY == 0)
            ;
#ifdef DEBUG_I2C
        debug_printf("I2C RS SLA=%02X %02X %02X\r\n", address, riicp->ICSR1.BYTE, riicp->ICSR2.BYTE);
        //debug_printf("I2C RS SLA=%02X icmr1(cks)=%02X brh,brl=%02X\r\n", address, riicp->ICMR1.BYTE, riicp->ICBRH.BIT.BRH);
#endif
        riicp->ICCR2.BIT.RS = 1; // I2C repeatedstart condition
        while (riicp->ICCR2.BIT.RS == 1)
            ;
        //while (riicp->ICSR2.BIT.TDRE == 0) ;
    } else {
        timeout = I2C_TIMEOUT_BUS_BUSY;
        while (riicp->ICCR2.BIT.BBSY) {
            if (timeout-- == 0) {
#ifdef DEBUG_I2C
                debug_printf("I2C BBSY\r\n");
#endif
                rx_i2c_abort(ch);
                return;
            }
        }
#ifdef DEBUG_I2C
        debug_printf("I2C ST SLA=%02X %02X %02X\r\n", address, riicp->ICSR1.BYTE, riicp->ICSR2.BYTE);
        //debug_printf("I2C ST SLA=%02X icmr1(cks)=%02X brh,brl=%02X\r\n", address, riicp->ICMR1.BYTE, riicp->ICBRH.BIT.BRH);
#endif
        riicp->ICCR2.BIT.ST = 1; // I2C start condition
        timeout = I2C_TIMEOUT_BUS_BUSY;
        while (1) {
            if ((riicp->ICSR2.BIT.NACKF == 1) || (timeout-- == 0)) {
#ifdef DEBUG_I2C
                debug_printf("I2C NK %02X TO=%d\r\n", riicp->ICSR2.BYTE, timeout);
#endif
                rx_i2c_abort(ch);
                return;
            }
            if (riicp->ICSR2.BIT.TDRE != 0)
                break;
        }
    }
    riicp->ICDRT = address;      // I2C send slave address
                                 // TODO: handling 10 bit address
    if (!unit->m_fread) {
        while (riicp->ICSR2.BIT.TDRE == 0)
            ;
        rx_i2c_ien(ch, 1);
        riicp->ICIER.BYTE |= 0xc0; /* TIE | TEIE */
    } else {
        while (riicp->ICSR2.BIT.RDRF == 0)
            ;
        rx_i2c_ien(ch, 1);
        riicp->ICIER.BYTE |= 0x38; /* RIE | NAKIE | SPIE */
        if (unit->m_bytes_transfer == 1) {
            riicp->ICMR3.BIT.WAIT = 1;
            riicp->ICMR3.BIT.ACKBT = 1;
        }
        uint8_t dummy = riicp->ICDRR;
    }
}

void rx_i2c_xaction_stop(uint32_t ch) {
    riic_t *riicp = RIIC[ch];

    rx_i2c_ir(ch, 0);
    rx_i2c_ien(ch, 0);
    riicp->ICIER.BYTE = 0x00;
    riicp->ICCR1.BIT.ICE = 0;
    current_xaction = (xaction_t *)NULL;
    current_xaction_unit = (xaction_unit_t *)NULL;
#ifdef DEBUG_I2C
    debug_printf("I2C Stop\r\n");
#endif
}

void rx_i2c_get_clock(uint32_t rateKhz, uint8_t *clockRate, uint8_t *clockRate2) {
    uint32_t bgr;
    uint8_t div_idx = 3;
    if (rateKhz > 400)
        rateKhz = 400;
    if (rateKhz == 0)
        rateKhz = 1;
    rateKhz *= 1000;
    bgr = PCLK / rateKhz;
    if (bgr <= 480) {
        div_idx = 3;
    } else if (bgr <= 4800) {
        div_idx = 6;
    } else {
        div_idx = 7;
    }
    clockRate2 = (uint8_t)div_idx;
    clockRate = (uint8_t)(((bgr / pclk_div[div_idx]) & 0xff) / 2);
#ifdef DEBUG_I2C_CLK
    debug_printf("I2C CLK=%d %d\r\n", clockRate, clockRate2);
#endif
}

void rx_i2c_unit_write_byte(uint32_t ch, xaction_unit_t *unit) {
    riic_t *riicp = RIIC[ch];
    while (riicp->ICSR2.BIT.TDRE == 0)
        ;
    riicp->ICDRT = unit->buf[unit->m_bytes_transferred];
    ++unit->m_bytes_transferred;
    --unit->m_bytes_transfer;
}

void rx_i2c_unit_read_byte(uint32_t ch, xaction_unit_t *unit) {
    riic_t *riicp = RIIC[ch];
    uint8_t data = riicp->ICDRR;
#ifdef DEBUG_I2C_RX_DATA
    debug_printf("r%02x", data);
    //debug_printf("r");
#endif
    unit->buf[unit->m_bytes_transferred] = data;
    ++unit->m_bytes_transferred;
    --unit->m_bytes_transfer;
}

static void rx_i2c_iceei_isr(uint32_t ch) {
    riic_t *riicp = RIIC[ch];
#ifdef DEBUG_I2C_INT
    debug_printf("I2C EEI %02X %02X\r\n", riicp->ICSR1.BYTE, riicp->ICSR2.BYTE);
#endif
    // Check Timeout
    if ((riicp->ICSR2.BIT.TMOF != 0) && (riicp->ICIER.BIT.TMOIE != 0)) {
#ifdef DEBUG_I2C_INT
        debug_printf("I2C TO\r\n");
#endif
        riicp->ICSR2.BIT.TMOF = 0;
    }
    // Check Arbitration Lost
    if ((riicp->ICSR2.BIT.AL != 0) && (riicp->ICIER.BIT.ALIE != 0)) {
#ifdef DEBUG_I2C_INT
        debug_printf("I2C AL\r\n");
#endif
        riicp->ICSR2.BIT.AL = 0;
    }
    // Check stop condition detection
    if ((riicp->ICSR2.BIT.STOP != 0) && (riicp->ICIER.BIT.SPIE != 0)) {
#ifdef DEBUG_I2C_INT
        debug_printf("I2C SC\r\n");
#endif
        riicp->ICSR2.BIT.STOP = 0;
        // ToDo:
    }
    // Check NACK reception
    if ((riicp->ICSR2.BIT.NACKF != 0) && (riicp->ICIER.BIT.NAKIE != 0)) {
        uint8_t dummy;
#ifdef DEBUG_I2C_INT
        debug_printf("I2C NK\r\n");
#endif
        riicp->ICSR2.BIT.NACKF = 0;
        //g_RX63N_I2C_Driver.rx_i2c_abort(ch);
    }
    // Check start condition detection
    if ((riicp->ICSR2.BIT.START != 0) && (riicp->ICIER.BIT.STIE != 0)) {
#ifdef DEBUG_I2C_INT
        debug_printf("I2C SC\r\n");
#endif
        riicp->ICSR2.BIT.START = 0;
    }
}

static void rx_i2c_icrxi_isr(uint32_t ch) {
    riic_t *riicp = RIIC[ch];
    xaction_unit_t *unit = current_xaction_unit;
    xaction_t *xaction = current_xaction;
#ifdef DEBUG_I2C_INT
    debug_printf("I2C RXI\r\n");
#endif
    while (riicp->ICSR2.BIT.RDRF == 0)
        ;
    if (unit->m_bytes_transfer > 2) {
        if (unit->m_bytes_transfer == 3)
            riicp->ICMR3.BIT.WAIT = 1;
        rx_i2c_unit_read_byte(ch, unit);
    } else if (unit->m_bytes_transfer == 2) {
        riicp->ICMR3.BIT.ACKBT = 1;
        rx_i2c_unit_read_byte(ch, unit);
    } else {
        rx_i2c_read_last_byte(ch);
    }
}

static void rx_i2c_ictxi_isr(uint32_t ch) {
    riic_t *riicp = RIIC[ch];
    xaction_unit_t *unit = current_xaction_unit;
    //GLOBAL_LOCK(irq);
#ifdef DEBUG_I2C_INT
    debug_printf("I2C TXI\r\n");
#endif
    if (unit->m_bytes_transfer) {
        rx_i2c_unit_write_byte(ch, unit);
    } else {
        riicp->ICIER.BIT.TIE = 0;
    }
}

static void rx_i2c_ictei_isr(uint32_t ch) {
    riic_t *riicp = RIIC[ch];
    xaction_t *xaction = current_xaction;
    xaction_unit_t *unit = current_xaction_unit;
#ifdef DEBUG_I2C_INT
    debug_printf("I2C TEI\r\n");
#endif
    riicp->ICIER.BIT.TEIE = 0;
    if (xaction->m_current == xaction->m_num_of_units) {
        rx_i2c_stop_confition(ch);
        //xAction->Signal(I2C_HAL_XACTION::c_Status_Completed );
    } else {
        rx_i2c_xaction_start(ch, xaction, true);
    }
}

#ifdef __cplusplus
extern "C" {
#endif
void __attribute__ ((interrupt)) INT_Excep_RIIC0_EEI0(void) { rx_i2c_iceei_isr(0); }
void __attribute__ ((interrupt)) INT_Excep_RIIC0_RXI0(void) { rx_i2c_icrxi_isr(0); }
void __attribute__ ((interrupt)) INT_Excep_RIIC0_TXI0(void) { rx_i2c_ictxi_isr(0); }
void __attribute__ ((interrupt)) INT_Excep_RIIC0_TEI0(void) { rx_i2c_ictei_isr(0); }
void __attribute__ ((interrupt)) INT_Excep_RIIC1_EEI1(void) { rx_i2c_iceei_isr(1); }
void __attribute__ ((interrupt)) INT_Excep_RIIC1_RXI1(void) { rx_i2c_icrxi_isr(1); }
void __attribute__ ((interrupt)) INT_Excep_RIIC1_TXI1(void) { rx_i2c_ictxi_isr(1); }
void __attribute__ ((interrupt)) INT_Excep_RIIC1_TEI1(void) { rx_i2c_ictei_isr(1); }
void __attribute__ ((interrupt)) INT_Excep_RIIC2_EEI2(void) { rx_i2c_iceei_isr(2); }
void __attribute__ ((interrupt)) INT_Excep_RIIC2_RXI2(void) { rx_i2c_icrxi_isr(2); }
void __attribute__ ((interrupt)) INT_Excep_RIIC2_TXI2(void) { rx_i2c_ictxi_isr(2); }
void __attribute__ ((interrupt)) INT_Excep_RIIC2_TEI2(void) { rx_i2c_ictei_isr(2); }
void __attribute__ ((interrupt)) INT_Excep_RIIC3_EEI3(void) { rx_i2c_iceei_isr(3); }
void __attribute__ ((interrupt)) INT_Excep_RIIC3_RXI3(void) { rx_i2c_icrxi_isr(3); }
void __attribute__ ((interrupt)) INT_Excep_RIIC3_TXI3(void) { rx_i2c_ictxi_isr(3); }
void __attribute__ ((interrupt)) INT_Excep_RIIC3_TEI3(void) { rx_i2c_ictei_isr(3); }
#ifdef __cplusplus
}
#endif
