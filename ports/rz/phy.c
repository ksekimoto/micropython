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

#include <stdio.h>
#include <stdbool.h>
#include "py/objlist.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "py/mperrno.h"
#include "py/mphal.h"

#if MICROPY_HW_ETH_MDC && MICROPY_PY_LWIP

#include "common.h"
#include "phy.h"

// typedef __uint32_t uint32_t;

#if defined(USE_DBG_PRINT)
#define DEBUG_PHY
#endif

// PHY Address
static char PhyAddr[] = {
    LAN8720_ADDR,
    ICS1894_ADDR,
    LAN8187_ADDR,
    DP83848_ADDR,
    LAN8700_ADDR
};

static uint32_t s_phy_addr = PHY_ADDR;
#define PHY_MAX (sizeof(PhyAddr) / sizeof(char))

bool phy_read(uint32_t ch, uint32_t reg_addr, uint32_t *data) {
    return rz_ether_phy_read(ch, s_phy_addr, reg_addr, data, MII_RD_TOUT);
}

bool phy_write(uint32_t ch, uint32_t reg_addr, uint32_t data) {
    return rz_ether_phy_write(ch, s_phy_addr, reg_addr, data, MII_WR_TOUT);
}

static bool _phy_reset(uint32_t ch, uint32_t phy_addr) {
    uint32_t i;
    uint32_t data;
    if (!rz_ether_phy_write(ch, phy_addr, PHY_REG_BMCR, BMCR_RESET, MII_WR_TOUT)) {
        return false;
    }
    for (i = 0; i < MII_WR_TOUT; i++) {
        if (!rz_ether_phy_read(ch, phy_addr, PHY_REG_BMCR, &data, MII_RD_TOUT)) {
            return false;
        }
        if (!(data & BMCR_RESET)) {
            return true;
        }
    }
    return false;
}

static bool phy_reset(uint32_t ch) {
    return _phy_reset(ch, s_phy_addr);
}

static bool phy_verify_id(uint32_t ch) {
    uint32_t id1, id2;

    if (!phy_read(ch, PHY_REG_IDR1, &id1)) {
        return false;
    }
    if (!phy_read(ch, PHY_REG_IDR2, &id2)) {
        return false;
    }
    #if defined(DEBUG_PHY)
    debug_printf("PHY(%d) ID: %2x%2x\r\n", ch, id1, id2);
    #endif
    return true;
}

static bool phy_set_auto_negotiate(uint32_t ch) {
    uint32_t reg;
    volatile uint32_t count;
    phy_write(ch, AN_ADVERTISEMENT_REG, 0x01E1);
    phy_write(ch, BASIC_MODE_CONTROL_REG, 0x1200);
    count = 0;
    do {
        phy_read(ch, BASIC_MODE_STATUS_REG, &reg);
        phy_read(ch, BASIC_MODE_STATUS_REG, &reg);
        count++;
    } while (!(reg & 0x0020) && count < PHY_AUTO_NEGOTIATON_WAIT);

    if (count >= PHY_AUTO_NEGOTIATON_WAIT) {
        return false;
    } else {
        return true;
    }
}

bool phy_set_link_speed(uint32_t ch) {
    uint32_t data;
    bool full_duplex, mbit_100;

    if (!phy_set_auto_negotiate(ch)) {
        #if defined(DEBUG_PHY)
        debug_printf("PHY(%d) AN NG\r\n", ch);
        #endif
        mbit_100 = true;
        full_duplex = true;
    } else {
        if (!phy_read(ch, PHY_REG_BMSR, &data)) {
            return false;
        }
        if (data & STS_FULL_DUP) {
            full_duplex = true;
        } else {
            full_duplex = false;
        }
        if (data & STS_10_MBIT) {
            mbit_100 = false;
        } else {
            mbit_100 = true;
        }
        #if defined(DEBUG_PHY)
        debug_printf("PHY(%d) AN FD:%d 100M:%d\r\n", ch, full_duplex, mbit_100);
        #endif
    }
    rz_ether_set_link_speed(ch, mbit_100, full_duplex);
    return true;
}

static bool phy_find_id(uint32_t ch, uint32_t *addr) {
    uint32_t i;
    for (i = 0; i < PHY_MAX; i++) {
        if (_phy_reset(ch, (uint32_t)PhyAddr[i])) {
            *addr = (uint32_t)PhyAddr[i];
            return true;
        }
    }
    return false;
}

bool phy_init(uint32_t ch) {
    s_phy_addr = PHY_ADDR;
    if (!phy_find_id(ch, &s_phy_addr)) {
        return false;
    }
    if (!phy_reset(ch)) {
        return false;
    }
    if (!phy_verify_id(ch)) {
        return false;
    }
    if (!phy_set_link_speed(ch)) {
        return false;
    }
    return true;
}

bool phy_get_link_status(uint32_t ch) {
    uint32_t data = 0;
    bool link_status = false;

    if (phy_read(ch, PHY_REG_BMSR, &data)) {
        link_status = ((data & 0x0004) == 0x0004);
    }
    return link_status;
}

#endif
