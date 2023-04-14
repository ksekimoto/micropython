/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Damien P. George
 * Portion Copyright (c) 2021 Kentaro Sekimoto
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
#include "shared/netutils/netutils.h"
#include "extmod/modnetwork.h"
#include "eth.h"

#if MICROPY_HW_ETH_MDC

#include "rx_ether.h"
#include "phy.h"

#include "lwip/etharp.h"
#include "lwip/dns.h"
#include "lwip/dhcp.h"
#include "netif/ethernet.h"

// #define DEBUG_ETH_FUNC_TRACE
#define DEBUG_ETH_PACKET_READ
#define DEBUG_ETH_PACKET_WRITE
#define DEBUG_ETH_FATAL_ERROR

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

typedef struct _eth_t {
    uint32_t trace_flags;
    struct netif netif;
    struct dhcp dhcp_struct;
} eth_t;

void ethernetif_input_cb(void);
extern struct ei_device le0;

eth_t eth_instance;

STATIC void eth_process_frame(eth_t *self, struct pbuf *p);

#if 0
STATIC void eth_phy_write(uint32_t reg, uint32_t val) {
    phy_write(reg, val);
}
#endif

STATIC uint32_t eth_phy_read(uint32_t reg) {
    uint32_t data;
    phy_read(reg, &data);
    return data;
}

void eth_init(eth_t *self, int mac_idx) {
    #if defined(DEBUG_ETH_FUNC_TRACE)
    printf("eth_init\r\n");
    #endif
    mp_hal_get_mac(mac_idx, &self->netif.hwaddr[0]);
    self->netif.hwaddr_len = 6;
}

void eth_set_trace(eth_t *self, uint32_t value) {
    self->trace_flags = value;
}

STATIC int eth_mac_init(eth_t *self) {
    #if defined(DEBUG_ETH_FUNC_TRACE)
    printf("eth_mac_init\r\n");
    #endif
    rx_ether_init((uint8_t *)&self->netif.hwaddr);
    rx_ether_input_set_callback((RX_ETHER_INPUT_CB)ethernetif_input_cb);
    rx_ether_start();
    return 0;
}

STATIC void eth_mac_deinit(eth_t *self) {
    #if defined(DEBUG_ETH_FUNC_TRACE)
    printf("eth_mac_deinit\r\n");
    #endif
    rx_ether_deinit();
}

STATIC int eth_tx_buf_get(size_t len, uint8_t **buf) {
    *buf = le0.txcurrent->buf_p;
    le0.txcurrent->bufsize = (uint16_t)len;
    return 0;
}

STATIC int eth_tx_buf_send(void) {
    int32_t flag = FP1;
    flag |= FP0;
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
    return 0;
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

STATIC err_t eth_netif_output(struct netif *netif, struct pbuf *p) {
    // This function should always be called from a context where PendSV-level IRQs are disabled

    LINK_STATS_INC(link.xmit);
    eth_trace(netif->state, (size_t)-1, p, NETUTILS_TRACE_IS_TX | NETUTILS_TRACE_NEWLINE);

    uint8_t *buf;
    int ret = eth_tx_buf_get(p->tot_len, &buf);
    if (ret == 0) {
        pbuf_copy_partial(p, buf, p->tot_len, 0);
        ret = eth_tx_buf_send();
    }
    if (ret != 0) {
        #if defined(DEBUG_ETH_PACKET_WRITE)
        printf("ETX:Err\r\n");
        #endif
        return ERR_BUF;
    } else {
        #if defined(DEBUG_ETH_PACKET_WRITE)
        printf("ETX:%d\r\n", p->tot_len);
        #endif
        return ERR_OK;
    }
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
    struct pbuf *p = NULL;
    struct pbuf *q = NULL;
    bool flag = true;
    int32_t recvd = 0;
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
//     uint32_t state = rx_disable_irq();
    while (flag) {
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
                #if defined(DEBUG_ETH_FATAL_ERROR)
                printf("ERX: Alloc Err\r\n");
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
//     rx_enable_irq(state);
    #if defined(DEBUG_ETH_PACKET_READ)
    if (recvdsize > 0) {
        printf("ERX:%ld\r\n", recvdsize);
    }
    #endif
    return p0;
}

void ethernetif_input_cb(void) {
    struct pbuf *p = low_level_input(&eth_instance.netif);
    eth_process_frame(&eth_instance, p);
}

STATIC err_t eth_netif_init(struct netif *netif) {
    #if defined(DEBUG_ETH_FUNC_TRACE)
    printf("eth_netif_init\r\n");
    #endif
    #if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "lwip";
    #endif /* LWIP_NETIF_HOSTNAME */

    netif->linkoutput = eth_netif_output;
    netif->output = etharp_output;
    netif->mtu = 1500;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET | NETIF_FLAG_IGMP;
    return ERR_OK;
}

STATIC void eth_lwip_init(eth_t *self) {
    #if defined(DEBUG_ETH_FUNC_TRACE)
    printf("eth_lwip_init\r\n");
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
    netif_add(n, &ipconfig[0], &ipconfig[1], &ipconfig[2], self, eth_netif_init, ethernet_input);
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
    #if defined(DEBUG_ETH_FUNC_TRACE)
    printf("eth_lwip_deinit\r\n");
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

STATIC void eth_process_frame(eth_t *self, struct pbuf *p) {
    eth_trace(self, p->len, p->payload, NETUTILS_TRACE_NEWLINE);

    struct netif *netif = &self->netif;
    if (netif->flags & NETIF_FLAG_LINK_UP) {
        if (p != NULL) {
            if (netif->input(p, netif) != ERR_OK) {
                LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
                pbuf_free(p);
            }
        }
    }
}

struct netif *eth_netif(eth_t *self) {
    return &self->netif;
}

int eth_link_status(eth_t *self) {
    #if defined(DEBUG_ETH_FUNC_TRACE)
    printf("eth_link_status\r\n");
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
        uint32_t s = eth_phy_read(PHY_BCR) | (eth_phy_read(PHY_SR) << 16);
        if (s == 0) {
            return 0; // link down
        } else {
            return 1; // link join
        }
    }
}

int eth_start(eth_t *self) {
    #if defined(DEBUG_ETH_FUNC_TRACE)
    printf("eth_start\r\n");
    #endif
    eth_lwip_deinit(self);

    // Make sure Eth is Not in low power mode.
    eth_low_power_mode(self, false);

    int ret = eth_mac_init(self);
    if (ret < 0) {
        return ret;
    }
    eth_lwip_init(self);
    return 0;
}

int eth_stop(eth_t *self) {
    #if defined(DEBUG_ETH_FUNC_TRACE)
    printf("eth_stop\r\n");
    #endif
    eth_lwip_deinit(self);
    eth_mac_deinit(self);
    return 0;
}

void eth_low_power_mode(eth_t *self, bool enable) {
    (void)self;
    // ToDo: implement
}

#endif // defined(MICROPY_HW_ETH_MDC)
