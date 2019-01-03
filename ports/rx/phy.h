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

#ifndef PORTS_RX_PHY_H_
#define PORTS_RX_PHY_H_

/* Phy Address */
#define ICS1894_ADDR    0x10        // PHY device address for Wakamatsu FM3 LAN board
#define LAN8187_ADDR    0x06        // PHY device address for Wakamatsu ARM LAN board
#define DP83848_ADDR    0x01        // PHY device address for DP83848
#define LAN8700_ADDR    0x1F        // PHY device address for Will Electronics WX-PHY
#define LAN8720_ADDR    0x00        // PHY device address for GR-SAKURA and GR-ROSE
/* Phy ID */
#define ICS1894_ID      0x0015f450  // PHY Identifier of ICS1894
#define LAN8187_ID      0x0006C0C4  // PHY Identifier of LAN8187
#define DP83848_ID      0x20005C90  // PHY Identifier of DP83848
#define LAN8700_ID      0x0007C0C0  // PHY Identifier of LAN8700i
#define LAN8720_ID      0x0007C0F1  // PHY Identifier of LAN8720
#define LAN8720A_ID     0x0007C0C0  // PHY Identifier of LAN8720A

#define PHY_ADDR        LAN8720_ADDR    // Default Phy Address

#define PHY_REG_BMCR    0x00        // Basic Mode Control Register
#define PHY_REG_BMSR    0x01        // Basic Mode Status Register
#define PHY_REG_IDR1    0x02        // PHY Identifier 1
#define PHY_REG_IDR2    0x03        // PHY Identifier 2
#define PHY_REG_STS     0x10        // Status Register

#define BMCR_RESET      0x8000
#define BMSR_AUTO_DONE  0x0020
#define PHY_AUTO_NEG    0x3000      // Select Auto Negotiation

#define STS_LINK_ON     0x1
#define STS_10_MBIT     0x2
#define STS_FULL_DUP    0x4

#define MII_WR_TOUT     1000        // MII Write timeout count
#define MII_RD_TOUT     1000        // MII Read timeout count


/* Standard PHY Registers */
#define BASIC_MODE_CONTROL_REG      0
#define BASIC_MODE_STATUS_REG       1
#define PHY_IDENTIFIER1_REG         2
#define PHY_IDENTIFIER2_REG         3
#define AN_ADVERTISEMENT_REG        4
#define AN_LINK_PARTNER_ABILITY_REG 5
#define AN_EXPANSION_REG            6

#define PHY_AUTO_NEGOTIATON_WAIT    0x00040000L

bool phy_read(uint32_t reg_addr, uint32_t *data);
bool phy_write(uint32_t reg_addr, uint32_t data);
bool phy_set_link_speed(void);
bool phy_get_link_status(void);
bool phy_init(void);

#endif /* PORTS_RX_PHY_H_ */
