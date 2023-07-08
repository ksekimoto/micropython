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

#include <stdio.h>
#include <stdint.h>
#include "common.h"
#include "iodefine.h"
#include "interrupt_handlers.h"
#include "rx_gpio.h"
#include "rx_timer.h"
#include "rx_i2c.h"

#ifdef USE_DBG_PRINT
// #define DEBUG_I2C
// #define DEBUG_I2C_REG_DUMP
// #define DEBUG_I2C_INT_EEI
// #define DEBUG_I2C_INT_TMOF
// #define DEBUG_I2C_INT_AL
// #define DEBUG_I2C_INT_START
// #define DEBUG_I2C_INT_STOP
// #define DEBUG_I2C_INT_NACKF
// #define DEBUG_I2C_INT_RXI
// #define DEBUG_I2C_INT_TXI
// #define DEBUG_I2C_INT_TEI
// #define DEBUG_I2C_TIMEOUT
// #define DEBUG_I2C_ERROR
// #define DEBUG_I2C_NACK
// #define DEBUG_I2C_TX_DATA
// #define DEBUG_I2C_RX_DATA
#endif

#define RX_I2C_PRIORITY 6
#define I2C_TIMEOUT_STOP_CONDITION 10000
#define I2C_TIMEOUT_BUS_BUSY 10000

xaction_t *current_xaction;
xaction_unit_t *current_xaction_unit;
static bool last_stop;

static const rx_af_pin_t scl_pins[] = {
    #if defined(RX63N)
    { AF_RIIC, 0, P12 },
    { AF_RIIC, 1, P21 },
    { AF_RIIC, 2, P16 },
    { AF_RIIC, 3, PC0 },

    #elif defined(RX65N)

    { AF_RIIC, 0, P12 },
    { AF_RIIC, 1, P21 },
    { AF_RIIC, 2, P16 },

    #else
    #error "MCU Series is not specified."
    #endif
};
#define SCL_PINS_SIZE sizeof(scl_pins) / sizeof(rx_af_pin_t)

static const rx_af_pin_t sda_pins[] = {
    #if defined(RX63N)
    { AF_RIIC, 0, P13 },
    { AF_RIIC, 1, P20 },
    { AF_RIIC, 2, P17 },
    { AF_RIIC, 3, PC1 },

    #elif defined(RX65N)

    { AF_RIIC, 0, P13 },
    { AF_RIIC, 1, P20 },
    { AF_RIIC, 2, P17 },

    #else
    #error "MCU Series is not specified."
    #endif
};
#define SDA_PINS_SIZE sizeof(sda_pins) / sizeof(rx_af_pin_t)

riic_t *RIIC[] = {
    (riic_t *)0x88300,
    (riic_t *)0x88320,
    (riic_t *)0x88340,
    #if defined(RX63N)
    (riic_t *)0x88360
    #endif
};

#if 0
static uint8_t pclk_div[8] = {
    1, 2, 4, 8, 16, 32, 64, 128
};
#endif

void  rx_i2c_get_pins(uint32_t ch, uint8_t *scl, uint8_t *sda) {
    *scl = scl_pins[ch].pin;
    *sda = sda_pins[ch].pin;
}

bool rx_af_find_ch(rx_af_pin_t *af_pin, uint32_t size, uint32_t pin, uint8_t *ch) {
    bool find = false;
    uint32_t i;
    for (i = 0; i < size; i++) {
        if (af_pin->pin == pin) {
            find = true;
            *ch = af_pin->ch;
            break;
        }
        af_pin++;
    }
    return find;
}

bool rx_i2c_find_af_ch(uint32_t scl, uint32_t sda, uint8_t *ch) {
    bool find = false;
    uint8_t scl_ch;
    uint8_t sda_ch;
    find = rx_af_find_ch((rx_af_pin_t *)&scl_pins, SCL_PINS_SIZE, scl, &scl_ch);
    if (find) {
        find = rx_af_find_ch((rx_af_pin_t *)&sda_pins, SDA_PINS_SIZE, sda, &sda_ch);
        if (find) {
            find = (scl_ch == sda_ch);
            if (find) {
                *ch = scl_ch;
            } else {
                *ch = 0;
            }
        }
    }
    return find;
}

static void rx_i2c_clear_line(uint32_t ch) {
    riic_t *rric_reg = RIIC[ch];
    if (rric_reg->ICCR1.BIT.SDAI == 0) {
        int32_t t = 10;
        while (t-- > 0) {
            rric_reg->ICCR1.BIT.CLO = 1;
            uint32_t timeout = I2C_TIMEOUT_STOP_CONDITION;
            while (rric_reg->ICCR1.BIT.CLO == 1) {
                if (timeout-- == 0) {
                    #if defined(DEBUG_I2C_ERROR)
                    debug_printf("I2C CLR ERR\r\n");
                    #endif
                    break;
                }
            }
            if (rric_reg->ICCR1.BIT.SDAI == 1) {
                break;
            }
        }
    }
}

#if defined(DEBUG_I2C_REG_DUMP)
static void i2c_reg_dump(uint32_t ch) {
    riic_t *rric_reg = RIIC[ch];
    debug_printf("CR1=%02X CR2=%02X MR1=%02X MR2=%02X MR3=%02X\r\n",
        rric_reg->ICCR1.BYTE, rric_reg->ICCR2.BYTE, rric_reg->ICMR1.BYTE, rric_reg->ICMR3.BYTE, rric_reg->ICMR3.BYTE);
    debug_printf("FER=%02X SER=%02X IER=%02X SR1=%02X SR2=%02X\r\n",
        rric_reg->ICFER.BYTE, rric_reg->ICSER.BYTE, rric_reg->ICIER.BYTE, rric_reg->ICSR1.BYTE, rric_reg->ICSR2.BYTE);
}
#endif

static void rx_i2c_set_int_priority(uint32_t ch, uint32_t val) {
    switch (ch) {
        case 0:
            IPR(RIIC0, RXI0) = val;
            IPR(RIIC0, TXI0) = val;
            #if defined(RX63N)
            IPR(RIIC0, TEI0) = val;
            IPR(RIIC0, EEI0) = val;
            #endif
            break;
        case 1:
            IPR(RIIC1, RXI1) = val;
            IPR(RIIC1, TXI1) = val;
            #if defined(RX63N)
            IPR(RIIC1, TEI1) = val;
            IPR(RIIC1, EEI1) = val;
            #endif
            break;
        case 2:
            IPR(RIIC2, RXI2) = val;
            IPR(RIIC2, TXI2) = val;
            #if defined(RX63N)
            IPR(RIIC2, TEI2) = val;
            IPR(RIIC2, EEI2) = val;
            #endif
            break;
        #if defined(RX63N)
        case 3:
            IPR(RIIC3, RXI3) = val;
            IPR(RIIC3, TXI3) = val;
            IPR(RIIC3, TEI3) = val;
            IPR(RIIC3, EEI3) = val;
            break;
        #endif
    }
}

static void rx_i2c_set_int_req(uint32_t ch, uint32_t val) {
    switch (ch) {
        case 0:
            IR(RIIC0, RXI0) = val;
            IR(RIIC0, TXI0) = val;
            #if defined(RX63N)
            IR(RIIC0, TEI0) = val;
            IR(RIIC0, EEI0) = val;
            #endif
            break;
        case 1:
            IR(RIIC1, RXI1) = val;
            IR(RIIC1, TXI1) = val;
            #if defined(RX63N)
            IR(RIIC1, TEI1) = val;
            IR(RIIC1, EEI1) = val;
            #endif
            break;
        case 2:
            IR(RIIC2, RXI2) = val;
            IR(RIIC2, TXI2) = val;
            #if defined(RX63N)
            IR(RIIC2, TEI2) = val;
            IR(RIIC2, EEI2) = val;
            #endif
            break;
        #if defined(RX63N)
        case 3:
            IR(RIIC3, RXI3) = val;
            IR(RIIC3, TXI3) = val;
            IR(RIIC3, TEI3) = val;
            IR(RIIC3, EEI3) = val;
            break;
        #endif
    }
}

static void rx_i2c_set_int_enable_req(uint32_t ch, uint32_t val) {
    switch (ch) {
        case 0:
            IEN(RIIC0, RXI0) = val;
            IEN(RIIC0, TXI0) = val;
            #if defined(RX63N)
            IEN(RIIC0, TEI0) = val;
            IEN(RIIC0, EEI0) = val;
            #endif
            break;
        case 1:
            IEN(RIIC1, RXI1) = val;
            IEN(RIIC1, TXI1) = val;
            #if defined(RX63N)
            IEN(RIIC1, TEI1) = val;
            IEN(RIIC1, EEI1) = val;
            #endif
            break;
        case 2:
            IEN(RIIC2, RXI2) = val;
            IEN(RIIC2, TXI2) = val;
            #if defined(RX63N)
            IEN(RIIC2, TEI2) = val;
            IEN(RIIC2, EEI2) = val;
            #endif
            break;
        #if defined(RX63N)
        case 3:
            IEN(RIIC3, RXI3) = val;
            IEN(RIIC3, TXI3) = val;
            IEN(RIIC3, TEI3) = val;
            IEN(RIIC3, EEI3) = val;
            break;
        #endif
    }
}

static void rx_i2c_module_start(uint32_t ch) {
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
        #if defined(RX63N)
        case 3:
            MSTP(RIIC3) = 0;
            break;
        #endif
    }
    SYSTEM.PRCR.WORD = 0xA500;
}

static void rx_i2c_module_stop(uint32_t ch) {
    SYSTEM.PRCR.WORD = 0xA502;
    switch (ch) {
        case 0:
            MSTP(RIIC0) = 1;
            break;
        case 1:
            MSTP(RIIC1) = 1;
            break;
        case 2:
            MSTP(RIIC2) = 1;
            break;
        #if defined(RX63N)
        case 3:
            MSTP(RIIC3) = 1;
            break;
        #endif
    }
    SYSTEM.PRCR.WORD = 0xA500;
}

typedef struct _rx_clock_params {
    uint8_t cks;
    uint8_t brh;
    uint8_t brl;
} rx_clock_params_t;

#if defined(RX63N)
// PCLK=50MHz
static const rx_clock_params_t rx_clock_params[] = {
    {7, 16, 20},    // 10 kbps
    {4, 26, 31},    // 50 kbps
    {3, 24, 31},    // 100 kbps
    {2, 7, 16},     // 400 kbps
    {0, 12, 24},    // 1000 kbps
};
#elif defined(RX65N)
// PCLK=60MHz
static const rx_clock_params_t rx_clock_params[] = {
    {7, 20, 24},    // 10 kbps
    {5, 15, 18},    // 50 kbps
    {3, 2, 3},      // 100 kbps
    {2, 8, 19},     // 400 kbps
    {0, 15, 29},    // 1000 kpbs
};
#else
#error "MCU Series is not specified."
#endif

void rx_i2c_clock_calc(uint32_t baudrate, uint8_t *cks, uint8_t *brh, uint8_t *brl) {
    uint idx = 0;
    if (baudrate <= 10000) {
        idx = 0;
    } else if (baudrate <= 50000) {
        idx = 1;
    } else if (baudrate <= 100000) {
        idx = 2;
    } else if (baudrate <= 400000) {
        idx = 3;
    } else {
        idx = 4;
    }
    *cks = rx_clock_params[idx].cks;
    *brh = rx_clock_params[idx].brh;
    *brl = rx_clock_params[idx].brl;
}

void rx_i2c_set_baudrate(uint32_t ch, uint32_t baudrate) {
    riic_t *rric_reg = RIIC[ch];
    uint8_t cks;
    uint8_t brh;
    uint8_t brl;
    #if 1
    rx_i2c_clock_calc(baudrate, &cks, &brh, &brl);
    #else
    cks = 3;
    brh = 24;
    brl = 29;
    #endif
    rric_reg->ICMR1.BIT.CKS = cks;
    rric_reg->ICBRH.BIT.BRH = brh;
    rric_reg->ICBRL.BIT.BRL = brl;
}

void rx_i2c_init(uint32_t ch, uint32_t scl, uint32_t sda, uint32_t baudrate, uint32_t timeout) {
    (void)timeout;
    riic_t *rric_reg = RIIC[ch];
    #if defined(DEBUG_I2C)
    debug_printf("rx_i2c_init: ch:%d scl:%d sda:%d baud:%d\r\n", ch, scl, sda, baudrate);
    #endif

    rx_i2c_module_start(ch);
    rx_i2c_set_int_priority(ch, RX_I2C_PRIORITY);
    rx_i2c_set_int_req(ch, 0);
    rx_i2c_set_int_enable_req(ch, 0);
    rx_gpio_config(scl, GPIO_MODE_AF_OD, GPIO_NOPULL, AF_RIIC);
    rx_gpio_config(sda, GPIO_MODE_AF_OD, GPIO_NOPULL, AF_RIIC);

    rric_reg->ICCR1.BIT.ICE = 0;       // I2C disable
    rric_reg->ICCR1.BIT.IICRST = 1;    // I2C internal reset
    rric_reg->ICCR1.BIT.ICE = 1;       // I2C enable
    rx_i2c_set_baudrate(ch, baudrate);
    rric_reg->ICSER.BYTE = 0x00;       // I2C reset bus status enable register
    rric_reg->ICMR3.BIT.ACKWP = 0x01;  // I2C allow to write ACKBT (transfer acknowledge bit)
    rric_reg->ICIER.BYTE = 0xe0;
    rric_reg->ICCR1.BIT.IICRST = 0;    // I2C internal reset clear
    rx_i2c_clear_line(ch);
    return;
}

void rx_i2c_deinit(uint32_t ch) {
    riic_t *rric_reg = RIIC[ch];
    rric_reg->ICIER.BYTE = 0;           // I2C interrupt disable
    rric_reg->ICCR1.BIT.ICE = 0;        // I2C disable
    rx_i2c_module_stop(ch);
    #if defined(DEBUG_I2C)
    debug_printf("rx_i2c_deinit: ch:%d\r\n", ch);
    #endif
    return;
}

void rx_i2c_read_last_byte(uint32_t ch) {
    riic_t *rric_reg = RIIC[ch];
    xaction_t *xaction = current_xaction;
    xaction_unit_t *unit = &xaction->units[xaction->m_current];
    xaction->m_current++;
    if (xaction->m_current == xaction->m_num_of_units) {
        rric_reg->ICSR2.BIT.STOP = 0;
        rric_reg->ICCR2.BIT.SP = 1;
        rx_i2c_unit_read_byte(ch, unit);
        rric_reg->ICMR3.BIT.WAIT = 0;
        uint32_t timeout = I2C_TIMEOUT_STOP_CONDITION;
        while (rric_reg->ICSR2.BIT.STOP == 0) {
            if (timeout-- == 0) {
                #if defined(DEBUG_I2C_TIMEOUT)
                debug_printf("1:I2C SC TO\r\n");
                #endif
                break;
            }
        }
        rric_reg->ICSR2.BIT.NACKF = 0;
        rric_reg->ICSR2.BIT.STOP = 0;
        xaction->m_status = RX_I2C_STATUS_Stopped;
        if (timeout == 0) {
            xaction->m_error = RX_I2C_ERROR_TMOF;
        } else {
            xaction->m_error = RX_I2C_ERROR_OK;
        }
    }
    // else {
    //     rx_i2c_xaction_start(xaction, true);
    // }
}

void rx_i2c_stop_confition(uint32_t ch) {
    riic_t *rric_reg = RIIC[ch];
    rric_reg->ICSR2.BIT.NACKF = 0;
    rric_reg->ICSR2.BIT.STOP = 0;
    rric_reg->ICCR2.BIT.SP = 1;
    uint32_t timeout = I2C_TIMEOUT_STOP_CONDITION;
    while (rric_reg->ICSR2.BIT.STOP == 0) {
        if (timeout-- == 0) {
            #if defined(DEBUG_I2C_TIMEOUT)
            debug_printf("2:I2C SC TO\r\n");
            #endif
            break;
        }
    }
    rric_reg->ICSR2.BIT.STOP = 0;
    #ifdef DEBUG_I2C
    debug_printf("I2C SC OK\r\n");
    #endif
}

void rx_i2c_abort(uint32_t ch) {
    riic_t *rric_reg = RIIC[ch];
    rx_i2c_stop_confition(ch);
    rric_reg->ICIER.BYTE = 0x00;
    rx_i2c_set_int_req(ch, 0);
    rric_reg->ICSR2.BIT.START = 0;
    rric_reg->ICSR2.BIT.NACKF = 0;
    rric_reg->ICCR1.BIT.ICE = 0;
    rx_i2c_clear_line(ch);
    current_xaction->m_error = RX_I2C_ERROR_BUSY;
}

void rx_i2c_xaction_start(xaction_t *xaction, bool repeated_start) {
    uint32_t timeout;
    uint32_t ch = xaction->m_ch;
    uint8_t address;
    riic_t *rric_reg = RIIC[ch];

    current_xaction = xaction;
    current_xaction_unit = &xaction->units[xaction->m_current];
    xaction_unit_t *unit = current_xaction_unit;

    rric_reg->ICCR1.BIT.ICE = 0;    // I2C disable
    rx_i2c_set_baudrate(ch, xaction->m_baudrate);
    rric_reg->ICCR1.BIT.ICE = 1;    // I2C enable
    #if defined(DEBUG_I2C_REG_DUMP)
    debug_printf("=2=\r\n");
    i2c_reg_dump(ch);
    #endif
    address = 0xFE & (xaction->m_address << 1);
    address |= unit->m_fread ? 0x1 : 0x0;
    #if defined(DEBUG_I2C)
    debug_printf("rx_i2c_xaction_start: ch:%d addr: %d(%d) \r\n", ch, xaction->m_address, address);
    #endif
    if (repeated_start) {
        rric_reg->ICIER.BYTE = 0x20;
        timeout = I2C_TIMEOUT_BUS_BUSY;
        while (rric_reg->ICCR2.BIT.BBSY == 0) {
            if (timeout-- == 0) {
                #if defined(DEBUG_I2C_TIMEOUT)
                debug_printf("4:I2C BBSY not H\r\n");
                #endif
                break;
            }
        }
        rric_reg->ICSR2.BIT.START = 0;
        rric_reg->ICCR2.BIT.RS = 1; // I2C repeatedstart condition
        timeout = I2C_TIMEOUT_BUS_BUSY;
        while (rric_reg->ICSR2.BIT.START != 1) {
            if (timeout-- == 0) {
                #if defined(DEBUG_I2C_TIMEOUT)
                debug_printf("5:I2C STCon not H\r\n");
                #endif
                break;
            }
        }
    } else {
        rric_reg->ICIER.BYTE = 0xe0;
        timeout = I2C_TIMEOUT_BUS_BUSY;
        while (rric_reg->ICCR2.BIT.BBSY == 1) {
            if (timeout-- == 0) {
                #if defined(DEBUG_I2C_TIMEOUT)
                debug_printf("6:I2C BBSY not L\r\n");
                #endif
                rx_i2c_abort(ch);
                return;
            }
        }
        rric_reg->ICCR2.BIT.ST = 1; // I2C start condition
    }
    timeout = I2C_TIMEOUT_BUS_BUSY;
    while (rric_reg->ICSR2.BIT.TDRE == 0) {
        if (timeout-- == 0) {
            #if defined(DEBUG_I2C_TIMEOUT)
            debug_printf("7:I2C TDRE not H\r\n");
            #endif
            break;
        }
        if (rric_reg->ICSR2.BIT.NACKF == 1) {
            #if defined(DEBUG_I2C_NACK)
            debug_printf("1:I2C NACK H\r\n");
            #endif
            rx_i2c_abort(ch);
            break;
        }
    }
    rric_reg->ICDRT = address;      // I2C send slave address
                                    // TODO: handling 10 bit address
// #ifdef DEBUG_I2C
// debug_printf("I2C ST SLA=%02X %02X %02X\r\n", address, rric_reg->ICSR1.BYTE, rric_reg->ICSR2.BYTE);
// #endif
    if (!unit->m_fread) {
        timeout = I2C_TIMEOUT_BUS_BUSY;
        while (rric_reg->ICSR2.BIT.TDRE == 0) {
            if (timeout-- == 0) {
                #if defined(DEBUG_I2C_TIMEOUT)
                debug_printf("8:I2C TDRE not H\r\n");
                #endif
                break;
            }
            if (rric_reg->ICSR2.BIT.NACKF == 1) {
                #if defined(DEBUG_I2C_NACK)
                debug_printf("2:I2C NACK H\r\n");
                #endif
                rx_i2c_abort(ch);
                break;
            }
        }
        rric_reg->ICIER.BYTE = 0xc0; /* TIE | TEIE */
    } else {
        timeout = I2C_TIMEOUT_BUS_BUSY;
        while (rric_reg->ICSR2.BIT.RDRF == 0) {
            if (timeout-- == 0) {
                #if defined(DEBUG_I2C_TIMEOUT)
                debug_printf("9:I2C RDRF not H\r\n");
                #endif
                break;
            }
        }
        if (unit->m_bytes_transfer == 1) {
            rric_reg->ICMR3.BIT.WAIT = 1;
            rric_reg->ICMR3.BIT.ACKBT = 1;
        }
        uint8_t dummy = rric_reg->ICDRR;
        (void)dummy;
        rric_reg->ICIER.BYTE = 0x20; /* RIE */
    }
    rx_i2c_set_int_enable_req(ch, 1);
}

void rx_i2c_xaction_stop(void) {
    uint32_t ch = current_xaction->m_ch;
    riic_t *rric_reg = RIIC[ch];
    rx_i2c_set_int_req(ch, 0);
    rx_i2c_set_int_enable_req(ch, 0);
    rric_reg->ICIER.BYTE = 0x00;
    rric_reg->ICCR1.BIT.ICE = 0;
    #if defined(DEBUG_I2C)
    debug_printf("rx_i2c_xaction_stop\r\n");
    #endif
}

void rx_i2c_unit_write_byte(uint32_t ch, xaction_unit_t *unit) {
    riic_t *rric_reg = RIIC[ch];
    uint32_t timeout = I2C_TIMEOUT_BUS_BUSY;
    while (rric_reg->ICSR2.BIT.TDRE == 0) {
        if (timeout-- == 0) {
            #if defined(DEBUG_I2C_TIMEOUT)
            debug_printf("10:I2C TDRE not H\r\n");
            #endif
            break;
        }
    }
    uint8_t data = unit->buf[unit->m_bytes_transferred];
    #if defined(DEBUG_I2C_TX_DATA)
    debug_printf("w%02x ", data);
    #endif
    rric_reg->ICDRT = data;
    ++unit->m_bytes_transferred;
    --unit->m_bytes_transfer;
}

void rx_i2c_unit_read_byte(uint32_t ch, xaction_unit_t *unit) {
    riic_t *rric_reg = RIIC[ch];
    uint8_t data = rric_reg->ICDRR;
    #if defined(DEBUG_I2C_RX_DATA)
    debug_printf("r%02x ", data);
    #endif
    unit->buf[unit->m_bytes_transferred] = data;
    ++unit->m_bytes_transferred;
    --unit->m_bytes_transfer;
}

void rx_i2c_unit_init(xaction_unit_t *unit, uint8_t *buf, uint32_t size, bool fread, void *next) {
    #if defined(DEBUG_I2C_UNIT)
    debug_printf("rx_i2c_unit_init size:%d fread:%d\r\n", size, fread);
    #endif
    unit->m_bytes_transferred = 0;
    unit->m_bytes_transfer = size;
    unit->m_fread = fread;
    unit->buf = buf;
    unit->next = (void *)next;
}

void rx_i2c_xaction_init(xaction_t *xaction, xaction_unit_t *units, uint32_t size, uint32_t ch, uint32_t baudrate, uint32_t address, bool stop) {
    #if defined(DEBUG_I2C_XACTION)
    debug_printf("rx_i2c_xaction_init size:%d ch:%d addr:%d baud:%d stop:%d\r\n", size, ch, address, baudrate, stop);
    #endif
    xaction->units = units;
    xaction->m_num_of_units = size;
    xaction->m_current = 0;
    xaction->m_ch = ch;
    xaction->m_baudrate = baudrate;
    xaction->m_address = address;
    xaction->m_status = RX_I2C_STATUS_Idle;
    xaction->m_error = RX_I2C_ERROR_OK;
    xaction->m_stop = stop;
}

static void rx_i2c_iceei_isr(uint32_t ch) {
    riic_t *rric_reg = RIIC[ch];
    #ifdef DEBUG_I2C_INT_EEI
    debug_printf("I2C EEI %02X %02X\r\n", rric_reg->ICSR1.BYTE, rric_reg->ICSR2.BYTE);
    #endif
    // Check Timeout
    if ((rric_reg->ICSR2.BIT.TMOF != 0) && (rric_reg->ICIER.BIT.TMOIE != 0)) {
        #if defined(DEBUG_I2C_INT_TMOF)
        debug_printf("I2C TO\r\n");
        #endif
        rric_reg->ICSR2.BIT.TMOF = 0;
    }
    // Check Arbitration Lost
    if ((rric_reg->ICSR2.BIT.AL != 0) && (rric_reg->ICIER.BIT.ALIE != 0)) {
        #if defined(DEBUG_I2C_INT_AL)
        debug_printf("I2C AL\r\n");
        #endif
        rric_reg->ICSR2.BIT.AL = 0;
    }
    // Check stop condition detection
    if ((rric_reg->ICSR2.BIT.STOP != 0) && (rric_reg->ICIER.BIT.SPIE != 0)) {
        #if defined(DEBUG_I2C_INT_STOP)
        debug_printf("I2C SP\r\n");
        #endif
        rric_reg->ICSR2.BIT.STOP = 0;
        // ToDo:
    }
    // Check NACK reception
    if ((rric_reg->ICSR2.BIT.NACKF != 0) && (rric_reg->ICIER.BIT.NAKIE != 0)) {
        #if defined(DEBUG_I2C_INT_NACKF)
        debug_printf("I2C NK\r\n");
        #endif
        rric_reg->ICSR2.BIT.NACKF = 0;
    }
    // Check start condition detection
    if ((rric_reg->ICSR2.BIT.START != 0) && (rric_reg->ICIER.BIT.STIE != 0)) {
        #if defined(DEBUG_I2C_INT_START)
        debug_printf("I2C ST\r\n");
        #endif
        rric_reg->ICSR2.BIT.START = 0;
    }
}

static void rx_i2c_icrxi_isr(uint32_t ch) {
    riic_t *rric_reg = RIIC[ch];
    xaction_unit_t *unit = current_xaction_unit;
    #if defined(DEBUG_I2C_INT_RXI)
    debug_printf("I2C RXI\r\n");
    #endif
    if (rric_reg->ICSR2.BIT.RDRF == 0) {
        return;
    }
    if (unit->m_bytes_transfer > 2) {
        if (unit->m_bytes_transfer == 3) {
            rric_reg->ICMR3.BIT.WAIT = 1;
        }
        rx_i2c_unit_read_byte(ch, unit);
    } else if (unit->m_bytes_transfer == 2) {
        rric_reg->ICMR3.BIT.ACKBT = 1;
        rx_i2c_unit_read_byte(ch, unit);
    } else {
        rric_reg->ICIER.BIT.RIE = 0;
        rx_i2c_read_last_byte(ch);
    }
}

static void rx_i2c_ictxi_isr(uint32_t ch) {
    riic_t *rric_reg = RIIC[ch];
    if (rric_reg->ICSR2.BIT.TDRE == 0) {
        return;
    }
    xaction_unit_t *unit = current_xaction_unit;
    #if defined(DEBUG_I2C_INT_TXI)
    debug_printf("I2C TXI\r\n");
    #endif
    if (unit->m_bytes_transfer) {
        rx_i2c_unit_write_byte(ch, unit);
    } else {
        rric_reg->ICIER.BIT.TIE = 0;
    }
}

static void rx_i2c_ictei_isr(uint32_t ch) {
    riic_t *rric_reg = RIIC[ch];
    xaction_t *xaction = current_xaction;
    #if defined(DEBUG_I2C_INT_TEI)
    debug_printf("I2C TEI\r\n");
    #endif
    rric_reg->ICIER.BIT.TEIE = 0;
    rric_reg->ICSR2.BIT.TEND = 0;
    xaction->m_current++;
    if (xaction->m_current == xaction->m_num_of_units) {
        if (xaction->m_stop) {
            rx_i2c_stop_confition(ch);
            last_stop = true;
        } else {
            last_stop = false;
        }
        xaction->m_status = RX_I2C_STATUS_Stopped;
        xaction->m_error = RX_I2C_ERROR_OK;
    } else {
        rx_i2c_xaction_start(xaction, true);
    }
}

bool rx_i2c_action_execute(xaction_t *xaction, bool repeated_start, uint32_t timeout_ms) {
    #if defined(DEBUG_I2C_ACTION)
    debug_printf("rx_i2c_action_execute: rs:%d to:%d\r\n", repeated_start, timeout_ms);
    #endif
    bool flag = false;
    uint32_t start = (uint32_t)mtick();

    current_xaction = xaction;
    current_xaction_unit = xaction->units;

    rx_i2c_xaction_start(xaction, repeated_start);
    while (true) {
        if (xaction->m_status == RX_I2C_STATUS_Stopped) {
            if (xaction->m_error == RX_I2C_ERROR_OK) {
                flag = true;
            }
            break;
        }
        if ((uint32_t)mtick() - start > timeout_ms) {
            #if defined(DEBUG_I2C_TIMEOUT)
            debug_printf("\r\nrx_i2c_action_execute: timeout\r\n");
            #endif
            rx_i2c_abort(xaction->m_ch);
            break;
        }
    }
    rx_i2c_xaction_stop();
    if (last_stop == true) {
        mdelay(1);  // avoid device busy of next access.
    }
    current_xaction = (xaction_t *)NULL;
    current_xaction_unit = (xaction_unit_t *)NULL;

    return flag;
}

#ifdef __cplusplus
extern "C" {
#endif
void __attribute__ ((interrupt)) INT_Excep_RIIC0_EEI0(void) {
    rx_i2c_iceei_isr(0);
}
void __attribute__ ((interrupt)) INT_Excep_RIIC0_RXI0(void) {
    rx_i2c_icrxi_isr(0);
}
void __attribute__ ((interrupt)) INT_Excep_RIIC0_TXI0(void) {
    rx_i2c_ictxi_isr(0);
}
void __attribute__ ((interrupt)) INT_Excep_RIIC0_TEI0(void) {
    rx_i2c_ictei_isr(0);
}
void __attribute__ ((interrupt)) INT_Excep_RIIC1_EEI1(void) {
    rx_i2c_iceei_isr(1);
}
void __attribute__ ((interrupt)) INT_Excep_RIIC1_RXI1(void) {
    rx_i2c_icrxi_isr(1);
}
void __attribute__ ((interrupt)) INT_Excep_RIIC1_TXI1(void) {
    rx_i2c_ictxi_isr(1);
}
void __attribute__ ((interrupt)) INT_Excep_RIIC1_TEI1(void) {
    rx_i2c_ictei_isr(1);
}
void __attribute__ ((interrupt)) INT_Excep_RIIC2_EEI2(void) {
    rx_i2c_iceei_isr(2);
}
void __attribute__ ((interrupt)) INT_Excep_RIIC2_RXI2(void) {
    rx_i2c_icrxi_isr(2);
}
void __attribute__ ((interrupt)) INT_Excep_RIIC2_TXI2(void) {
    rx_i2c_ictxi_isr(2);
}
void __attribute__ ((interrupt)) INT_Excep_RIIC2_TEI2(void) {
    rx_i2c_ictei_isr(2);
}
void __attribute__ ((interrupt)) INT_Excep_RIIC3_EEI3(void) {
    rx_i2c_iceei_isr(3);
}
void __attribute__ ((interrupt)) INT_Excep_RIIC3_RXI3(void) {
    rx_i2c_icrxi_isr(3);
}
void __attribute__ ((interrupt)) INT_Excep_RIIC3_TXI3(void) {
    rx_i2c_ictxi_isr(3);
}
void __attribute__ ((interrupt)) INT_Excep_RIIC3_TEI3(void) {
    rx_i2c_ictei_isr(3);
}
#ifdef __cplusplus
}
#endif
