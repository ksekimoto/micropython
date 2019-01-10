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

#ifndef RX65N_ETHER_H_
#define RX65N_ETHER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define TX_DESC_SECTION   __attribute__((section ("_TX_DEDC")))
#define RX_DESC_SECTION   __attribute__((section ("_RX_DESC")))

struct eth_descriptor {
    // little endian
    uint32_t status;
    uint16_t size;
    uint16_t bufsize;
    uint8_t *buf_p;
    struct eth_descriptor *next;
};

typedef struct eth_descriptor ethfifo;

struct enet_stats {
    uint32_t rx_packets;    /* total packets received    */
    uint32_t tx_packets;    /* total packets transmitted  */
    uint32_t rx_errors;     /* bad packets received      */
    uint32_t tx_errors;     /* packet transmit problems    */
    uint32_t rx_dropped;    /* no space in buffers      */
    uint32_t tx_dropped;    /* no space available      */
    uint32_t multicast;     /* multicast packets received  */
    uint32_t collisions;

    /* detailed rx_errors: */
    uint32_t rx_length_errors;
    uint32_t rx_over_errors;    /* receiver ring buffer overflow  */
    uint32_t rx_crc_errors;     /* recved pkt with crc error  */
    uint32_t rx_frame_errors;   /* recv'd frame alignment error  */
    uint32_t rx_fifo_errors;    /* recv'r fifo overrun      */
    uint32_t rx_missed_errors;  /* receiver missed packet    */

    /* detailed tx_errors */
    uint32_t tx_aborted_errors;
    uint32_t tx_carrier_errors;
    uint32_t tx_fifo_errors;
    uint32_t tx_heartbeat_errors;
    uint32_t tx_window_errors;
};

struct ei_device {
    const int8_t *name;
    uint8_t open;
    uint8_t Tx_act;
    uint8_t Rx_act;
    uint8_t txing;          /* Transmit Active */
    uint8_t irqlock;        /* EDMAC's interrupt disabled when '1'. */
    uint8_t dmaing;         /* EDMAC Active */
    ethfifo *rxcurrent;     /* current receive discripter */
    ethfifo *txcurrent;     /* current transmit discripter */
    uint8_t save_irq;       /* Original dev->irq value. */
    struct enet_stats stat;
    uint8_t mac_addr[6];
};

#define HW_INTERRUPT
#define ETH_RMII_MODE   0
#define ETH_MII_MODE    1
#define ETH_MODE_SEL    ETH_RMII_MODE
#define ETH_BUF_SIZE    1536
#define ETH_BUF_NUM     4

#define ACT     0x80000000
#define DL      0x40000000
#define FP1     0x20000000
#define FP0     0x10000000
#define FE      0x08000000

#define RFOVER  0x00000200
#define RAD     0x00000100
#define RMAF    0x00000080
#define RRF     0x00000010
#define RTLF    0x00000008
#define RTSF    0x00000004
#define PRE     0x00000002
#define CERF    0x00000001

#define TAD     0x00000100
#define CND     0x00000008
#define DLC     0x00000004
#define CD      0x00000002
#define TRO     0x00000001

/* Standard PHY Registers */
#define BASIC_MODE_CONTROL_REG      0
#define BASIC_MODE_STATUS_REG       1
#define PHY_IDENTIFIER1_REG         2
#define PHY_IDENTIFIER2_REG         3
#define AN_ADVERTISEMENT_REG        4
#define AN_LINK_PARTNER_ABILITY_REG 5
#define AN_EXPANSION_REG            6

/* Media Independent Interface */
#define PHY_ST      1
#define PHY_READ    2
#define PHY_WRITE   1
//#define  PHY_ADDR  0x1F

#define MDC_WAIT    2

/* PHY return definitions */
#define R_PHY_OK    0
#define R_PHY_ERROR -1

/* Auto-Negotiation Link Partner Status */
#define PHY_AN_LINK_PARTNER_100BASE 0x0180
#define PHY_AN_LINK_PARTNER_FULL    0x0140

#define PHY_AN_LINK_PARTNER_100FULL 0x0100
#define PHY_AN_LINK_PARTNER_100HALF 0x0080
#define PHY_AN_LINK_PARTNER_10FULL  0x0040
#define PHY_AN_LINK_PARTNER_10HALF  0x0020

//#define PHY_RESET_WAIT            0x00020000L
//#define PHY_AUTO_NEGOTIATON_WAIT  0x00800000L
#define PHY_RESET_WAIT              0x00002000L
#define PHY_AUTO_NEGOTIATON_WAIT    0x00040000L

void rx_ether_int(void);
void rx_ether_init(uint8_t *hwaddr);
void rx_ether_start(void);
void rx_ether_deinit(void);
bool rx_ether_phy_write(uint32_t phy_addr, uint32_t reg_addr, uint32_t data, uint32_t retry);
bool rx_ether_phy_read(uint32_t phy_addr, uint32_t reg_addr, uint32_t *data, uint32_t retry);
void rx_ether_set_link_speed(bool speed, bool fullduplex);
int32_t rx_ether_fifo_write(ethfifo *p, int8_t buf[], int32_t size);
int32_t rx_ether_fifo_read(ethfifo *p, int8_t buf[]);

typedef void (*RX_ETHER_INPUT_CB)(void);
void rx_ether_input_set_callback(RX_ETHER_INPUT_CB func);
void rx_ether_input_callback(void);

#ifdef __cplusplus
}
#endif

#endif /* RX65N_ETHER_H_ */
