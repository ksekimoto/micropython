/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Damien P. George
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

#include <string.h>
#include "py/mphal.h"
#include "py/mperrno.h"
#include "lib/netutils/netutils.h"
#include "modnetwork.h"

#if MICROPY_HW_ETH_RX && MICROPY_PY_LWIP

#include "lwip/opt.h"
#include "lwip_inc/lwipopts.h"

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "netif/etharp.h"

#include "common.h"
#include "phy.h"
#include "modmachine.h"

#include "lwip/etharp.h"
#include "lwip/dns.h"
#include "lwip/dhcp.h"
#include "netif/ethernet.h"
#include "eth.h"

#if defined(USE_DBG_PRINT)
//#define DEBUG_ETHERNETIF
#endif

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'
/* phy registers */
#define PHY_BCR     0x00
#define PHY_BSR     0x01
#define PHY_MISR    0x12
#define PHY_SR      0x10

/* phy status bit */
#define PHY_LINK_STATUS         0x0001
#define PHY_SPEED_STATUS        0x0002
#define PHY_DUPLEX_STATUS       0x0004
#define PHY_AUTONEGO_COMPLETE   0x0020
#define PHY_AUTONEGOTIATION     0x1000
#define PHY_LINK_INTERRUPT      0x2000
#define RESET   1

eth_t eth_instance;

extern struct ei_device le0;
//static int8_t rx_buf[ALIGNED_BUFSIZE] __attribute__((aligned(32)));

void eth_init(eth_t *self, int mac_idx) {
    mp_hal_get_mac(mac_idx, &self->netif.hwaddr[0]);
    self->netif.hwaddr_len = 6;
}

void eth_set_trace(eth_t *self, uint32_t value) {
    self->trace_flags = value;
}

STATIC int eth_mac_init(eth_t *self) {
#if defined(DEBUG_ETHERNETIF)
    debug_printf("eth_mac_init\r\n");
#endif
    uint8_t macaddress[6];
#if defined(RX65N)
    uint8_t id[16];
    get_unique_id((uint8_t *)&id);
    macaddress[0] = 0;
    macaddress[1] = id[11];
    macaddress[2] = id[12];
    macaddress[3] = id[13];
    macaddress[4] = id[14];
    macaddress[5] = id[15];
#else
#if defined(MICROPY_HW_ETH_MAC_ADDRESS_0)
    macaddress[0] = MICROPY_HW_ETH_MAC_ADDRESS_0;
    macaddress[1] = MICROPY_HW_ETH_MAC_ADDRESS_1;
    macaddress[2] = MICROPY_HW_ETH_MAC_ADDRESS_2;
    macaddress[3] = MICROPY_HW_ETH_MAC_ADDRESS_3;
    macaddress[4] = MICROPY_HW_ETH_MAC_ADDRESS_4;
    macaddress[5] = MICROPY_HW_ETH_MAC_ADDRESS_5;
#else
    uint32_t tick = utick();
    macaddress[0] = 0;
    macaddress[1] = 0;
    macaddress[2] = (uint8_t)(tick >> 24);
    macaddress[3] = (uint8_t)(tick >> 16);
    macaddress[4] = (uint8_t)(tick >> 8);
    macaddress[5] = (uint8_t)(tick);
#endif
#endif
    self->netif.hwaddr_len = ETHARP_HWADDR_LEN;
    memcpy(&self->netif.hwaddr, &macaddress, ETHARP_HWADDR_LEN);
    self->netif.mtu = 1500;
    self->netif.flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP | NETIF_FLAG_ETHERNET;
    rx_ether_init((uint8_t *)&macaddress);
    rx_ether_input_set_callback((RX_ETHER_INPUT_CB)ethernetif_input_cb);
    rx_ether_start();
    return 0;
}

STATIC void eth_mac_deinit(eth_t *self) {
#if defined(DEBUG_ETHERNETIF)
    debug_printf("eth_mac_deinit\r\n");
#endif
    rx_ether_deinit();
}

/*******************************************************************************/
// ETH-LwIP bindings

#define TRACE_ASYNC_EV (0x0001)
#define TRACE_ETH_TX (0x0002)
#define TRACE_ETH_RX (0x0004)
#define TRACE_ETH_FULL (0x0008)

STATIC void eth_trace(eth_t *self, size_t len, const void *data, unsigned int flags) {
    if (((flags & NETUTILS_TRACE_IS_TX) && (self->trace_flags & TRACE_ETH_TX))
        || (!(flags & NETUTILS_TRACE_IS_TX) && (self->trace_flags & TRACE_ETH_RX))) {
        const uint8_t *buf;
        if (len == (size_t)-1) {
            // data is a pbuf
            const struct pbuf *pbuf = data;
            buf = pbuf->payload;
            len = pbuf->len; // restricted to print only the first chunk of the pbuf
        } else {
            // data is actual data buffer
            buf = data;
        }
        if (self->trace_flags & TRACE_ETH_FULL) {
            flags |= NETUTILS_TRACE_PAYLOAD;
        }
        netutils_ethernet_trace(MP_PYTHON_PRINTER, len, buf, flags);
    }
}

err_t eth_netif_output(struct netif *netif, struct pbuf *p) {
    // This function should always be called from a context where PendSV-level IRQs are disabled
    LINK_STATS_INC(link.xmit);
    eth_trace(netif->state, (size_t)-1, p, NETUTILS_TRACE_IS_TX | NETUTILS_TRACE_NEWLINE);

    //int tot_len = p->tot_len;
    int len = 0;
    uint8_t *buf = le0.txcurrent->buf_p;
    int32_t flag = FP1;
    for (struct pbuf *q = p; q != NULL; q = q->next) {
        /* Send the data from the pbuf to the interface, one pbuf at a
         time. The size of the data in each pbuf is kept in the ->len
         variable. */
        memcpy((void *)(buf + len), (const void *)(q->payload), (size_t)(q->len));
        len += (int)q->len;
    }
    flag |= FP0;
    le0.txcurrent->bufsize = (uint16_t)len;
    /* Clear previous settings */
    le0.txcurrent->status &= ~(FP1 | FP0);
    le0.txcurrent->status |= (flag | ACT);
    le0.txcurrent = le0.txcurrent->next;
    le0.stat.tx_packets++;
#if defined(RX63N)
    if (EDMAC.EDTRR.LONG == 0x00000000) {
        EDMAC.EDTRR.LONG = 0x00000001;
    }
#endif
#if defined(RX64M) || defined(RX65N)
    if (EDMAC0.EDTRR.LONG == 0x00000000) {
        EDMAC0.EDTRR.LONG = 0x00000001;
    }
#endif
#if defined(DEBUG_ETHERNETIF)
    if (len != tot_len) {
        debug_printf("ETX:Err\r\n");
    }
    debug_printf("ETX:%d\r\n", len);
#endif
    return ERR_OK;
}

#if 0
STATIC err_t eth_netif_init(struct netif *netif) {
#if defined(DEBUG_ETHERNETIF)
    debug_printf("eth_netif_init\r\n");
#endif
    netif->linkoutput = eth_netif_output;
    netif->output = etharp_output;
    netif->mtu = 1500;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET | NETIF_FLAG_IGMP;
    // Checksums only need to be checked on incoming frames, not computed on outgoing frames
    NETIF_SET_CHECKSUM_CTRL(netif,
        NETIF_CHECKSUM_CHECK_IP
        | NETIF_CHECKSUM_CHECK_UDP
        | NETIF_CHECKSUM_CHECK_TCP
        | NETIF_CHECKSUM_CHECK_ICMP
        | NETIF_CHECKSUM_CHECK_ICMP6);
    return ERR_OK;
}
#endif

static struct pbuf *low_level_input(struct netif *netif);

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
void ethernetif_input(struct netif *netif) {
    /* move received packet into a new pbuf */
    struct pbuf *p = low_level_input(netif);
    /* if no packet could be read, silently ignore this */
    if (p != NULL) {
        /* pass all packets to ethernet_input, which decides what packets it supports */
        if (netif->input(p, netif) != ERR_OK) {
            LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
            pbuf_free(p);
            p = NULL;
        }
    }
}

void ethernetif_input_cb(void) {
    ethernetif_input(&eth_instance.netif);
}

err_t ethernetif_init(struct netif *netif) {
#if defined(DEBUG_ETHERNETIF)
    debug_printf("ethernetif_init\r\n");
#endif
#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;

    /* We directly use etharp_output() here to save a function call.
     * You can instead declare your own function an call etharp_output()
     * from it if you have to do some checks before sending (e.g. if link
     * is available...) */
    netif->output = etharp_output;
    netif->linkoutput = eth_netif_output;

    /* initialize the hardware */
    eth_mac_init(&eth_instance);

    return ERR_OK;
}

STATIC void eth_lwip_init(eth_t *self) {
#if defined(DEBUG_ETHERNETIF)
    debug_printf("eth_lwip_init\r\n");
#endif
    ip_addr_t ipconfig[4];
    IP4_ADDR(&ipconfig[0], 0, 0, 0, 0);
    IP4_ADDR(&ipconfig[2], 192, 168, 0, 1);
    IP4_ADDR(&ipconfig[1], 255, 255, 255, 0);
    IP4_ADDR(&ipconfig[3], 8, 8, 8, 8);

    MICROPY_PY_LWIP_ENTER

    struct netif *n = &self->netif;
    n->name[0] = 'e';
    n->name[1] = '0';
    netif_add(n, &ipconfig[0], &ipconfig[1], &ipconfig[2], self, ethernetif_init, ethernet_input);
    netif_set_hostname(n, "MPY");
    netif_set_default(n);
    netif_set_up(n);

    dns_setserver(0, &ipconfig[3]);
    dhcp_set_struct(n, &self->dhcp_struct);
    dhcp_start(n);

    netif_set_link_up(n);

    MICROPY_PY_LWIP_EXIT
}

STATIC void eth_lwip_deinit(eth_t *self) {
#if defined(DEBUG_ETHERNETIF)
    debug_printf("eth_lwip_deinit\r\n");
#endif
    MICROPY_PY_LWIP_ENTER
    for (struct netif *netif = netif_list; netif != NULL; netif = netif->next) {
        if (netif == &self->netif) {
            netif_remove(netif);
            netif->ip_addr.addr = 0;
            netif->flags = 0;
        }
    }
    MICROPY_PY_LWIP_EXIT
}

struct netif *eth_netif(eth_t *self) {
    return &self->netif;
}

int eth_link_status(eth_t *self) {
#if defined(DEBUG_ETHERNETIF)
    debug_printf("eth_link_status\r\n");
#endif
    struct netif *netif = &self->netif;
    if ((netif->flags & (NETIF_FLAG_UP | NETIF_FLAG_LINK_UP))
        == (NETIF_FLAG_UP | NETIF_FLAG_LINK_UP)) {
        if (netif->ip_addr.addr != 0) {
            return 3; // link up
        } else {
            return 2; // link no-ip;
        }
    } else {
        //int s = eth_phy_read(0) | eth_phy_read(0x10) << 16;
        int s;
        uint32_t regvalue;
        phy_read(PHY_BCR, &regvalue);
        s = regvalue;
        phy_read(PHY_SR, &regvalue);
        s |= (regvalue << 16);
        if (s == 0) {
            return 0; // link down
        } else {
            return 1; // link join
        }
    }
}

int eth_start(eth_t *self) {
#if defined(DEBUG_ETHERNETIF)
    debug_printf("eth_start\r\n");
#endif
    eth_lwip_deinit(self);
    int ret = eth_mac_init(self);
    if (ret < 0) {
        return ret;
    }
    eth_lwip_init(self);
    return 0;
}

int eth_stop(eth_t *self) {
#if defined(DEBUG_ETHERNETIF)
    debug_printf("eth_stop\r\n");
#endif
    eth_lwip_deinit(self);
    eth_mac_deinit(self);
    return 0;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *low_level_input(struct netif *netif) {
    struct pbuf *p0 = NULL;
    struct pbuf *p;
    struct pbuf *q;
    bool flag = true;
    int32_t recvd;
    int32_t readcount = 0;
    int32_t recvdsize = 0;

#if defined(RX63N)
    if ((EDMAC.EESR.LONG & 0x00040000) != 0) {
        EDMAC.EESR.LONG |= 0x00040000;
    }
#endif
#if defined(RX64M) || defined(RX65N)
    if ((EDMAC0.EESR.LONG & 0x00040000) != 0) {
        EDMAC0.EESR.LONG |= 0x00040000;
    }
#endif
    rx_disable_irq();
    while (flag) {
        //recvd = rx_ether_fifo_read(le0.rxcurrent, (int8_t *)&rx_buf);
        readcount++;
        if (readcount >= 2 && recvdsize == 0) {
            break;
        }
        if ((le0.rxcurrent->status & ACT) != 0) {
            /* No descriptor to process */
        } else if ((le0.rxcurrent->status & FE) != 0) {
            /* Frame error.  Point to next frame.  Clear this descriptor. */
            le0.stat.rx_errors++;
            recvdsize = 0;
            le0.rxcurrent->status &= ~(FP1 | FP0 | FE);
            le0.rxcurrent->status &= ~(RFOVER | RAD | RMAF | RRF | RTLF | RTSF | PRE | CERF);
            le0.rxcurrent->status |= ACT;
            le0.rxcurrent = le0.rxcurrent->next;
#if defined(RX63N)
            if (EDMAC.EDRRR.LONG == 0x00000000L) {
                /* Restart if stopped */
                EDMAC.EDRRR.LONG = 0x00000001L;
            }
#endif
#if defined(RX64M) || defined(RX65N)
            if (EDMAC0.EDRRR.LONG == 0x00000000L) {
                /* Restart if stopped */
                EDMAC0.EDRRR.LONG = 0x00000001L;
            }
#endif
        } else {
            /* We have a good buffer. */
            if ((le0.rxcurrent->status & FP1) == FP1) {
                /* Beginning of a frame */
                recvdsize = 0;
            }
            if ((le0.rxcurrent->status & FP0) == FP0) {
                /* Frame is complete */
                le0.stat.rx_packets++;
                flag = false;
                /* This is the last descriptor.  Complete frame is received.   */
                recvd = le0.rxcurrent->size;
                while (recvd > ALIGNED_BUFSIZE) {
                    recvd -= ALIGNED_BUFSIZE;
                }
            } else {
                /**
                 * This is either a start or continuous le0.rxcurrentriptor.
                 * Complete frame is NOT received.
                 **/
                recvd = ALIGNED_BUFSIZE;
            }
            p = pbuf_alloc(PBUF_RAW, recvd, PBUF_RAM);
            if (p == NULL) {
#if defined(DEBUG_ETHERNETIF)
                debug_printf("ERX: Alloc Err\r\n");
#endif
                break;
            }
            memcpy(p->payload, (const void *)(le0.rxcurrent->buf_p), (size_t)recvd);
            p->len = recvd;
            recvdsize += recvd;
            if (p0 == NULL) {
                p0 = p;
            } else {
                q->next = p;
            }
            p->next = NULL;
            q = p;
            le0.rxcurrent->status &= ~(FP1 | FP0);
            le0.rxcurrent->status |= ACT;
            le0.rxcurrent = le0.rxcurrent->next;
#if defined(RX63N)
            if (EDMAC.EDRRR.LONG == 0x00000000L) {
                /* Restart if stopped */
                EDMAC.EDRRR.LONG = 0x00000001L;
            }
#endif
#if defined(RX64M) || defined(RX65N)
            if (EDMAC0.EDRRR.LONG == 0x00000000L) {
                /* Restart if stopped */
                EDMAC0.EDRRR.LONG = 0x00000001L;
            }
#endif
        }
    }
    rx_enable_irq();
#if defined(DEBUG_ETHERNETIF)
    if (recvdsize > 0) {
        debug_printf("ERX:%d\r\n", recvdsize);
    }
#endif
    return p0;
}

#endif // defined(MICROPY_HW_ETH_RX)
