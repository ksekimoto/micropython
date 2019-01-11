/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include <stdio.h>
#include <string.h>
#include "py/runtime.h"
#include "py/objstr.h"
#include "py/mperrno.h"
#include "py/mphal.h"

#if MICROPY_HW_HAS_ETHERNET && MICROPY_PY_LWIP

#include "lwip/opt.h"
#include "lwip_inc/lwipopts.h"

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/ethip6.h"
#include "lwip/etharp.h"
#include "netif/ppp/pppoe.h"

#include "common.h"
#include "phy.h"
#include "modmachine.h"
#include "ethernetif.h"

//#define DEBUG_ETHERNETIF

extern struct netif *g_netif;
extern struct ei_device le0;
extern int8_t tmpbuf[];

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};

/* Forward declarations. */
void  ethernetif_input(struct netif *netif);

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

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void
low_level_init(struct netif *netif)
{
#if defined(DEBUG_ETHERNETIF)
    debug_printf("low_level_init\r\n");
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
    uint32_t tick = utick();
    macaddress[0] = 0;
    macaddress[1] = 0;
    macaddress[2] = (uint8_t)(tick >> 24);
    macaddress[3] = (uint8_t)(tick >> 16);
    macaddress[4] = (uint8_t)(tick >> 8);
    macaddress[5] = (uint8_t)(tick);
#endif
    netif->hwaddr_len = ETHARP_HWADDR_LEN;
    memcpy(&netif->hwaddr, &macaddress, ETHARP_HWADDR_LEN);
    netif->mtu = 1500;
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP | NETIF_FLAG_ETHERNET;
    rx_ether_init((uint8_t *)&macaddress);
    rx_ether_input_set_callback((RX_ETHER_INPUT_CB)ethernetif_input_cb);
    rx_ether_start();
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become available since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
  int32_t xmit;
  int32_t flag = FP1;
  int8_t *data;

  while (p) {
    // ToDo - Can't work when a packet is fragmented.
    int32_t len = p->len;
    data = (int8_t *)p->payload;
    for (xmit = 0; len > 0; len -= xmit) {
       while ((xmit = rx_ether_fifo_write(le0.txcurrent, data, (int32_t)len)) < 0) {
         ;
       }
       if (xmit == len) {
         flag |= FP0;
       }
       /* Clear previous settings */
       le0.txcurrent->status &= ~(FP1 | FP0);
       le0.txcurrent->status |= (flag | ACT);
       flag = 0;
       le0.txcurrent = le0.txcurrent->next;
       data += xmit;
    }
#if defined(DEBUG_ETHERNETIF)
    debug_printf("ETHTX:%d\r\n", p->len);
#endif
    p = p->next;
  }
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
  return ERR_OK;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *
low_level_input(struct netif *netif)
{
  struct pbuf *p = NULL;
  bool flag = true;
  int32_t recvd;
  int32_t readcount = 0;
  int32_t receivesize = 0;

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
  while (flag) {
    recvd = rx_ether_fifo_read(le0.rxcurrent, (int8_t *)&tmpbuf);
    readcount++;
    if (readcount >= 2 && receivesize == 0)
      break;
    if (recvd == -1) {
      /* No descriptor to process */
    } else if (recvd == -2) {
      /* Frame error.  Point to next frame.  Clear this descriptor. */
      le0.stat.rx_errors++;
      receivesize = 0;
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
        receivesize = 0;
      }
      if ((le0.rxcurrent->status & FP0) == FP0) {
        /* Frame is complete */
        le0.stat.rx_packets++;
        flag = false;
      }
      p = pbuf_alloc(PBUF_RAW, recvd, PBUF_RAM);
      if (p == NULL) {
        //debug_printf("ENETRX NG Alloc\r\n");
         break;
      }
      memcpy(p->payload, &tmpbuf, recvd);
      p->len = recvd;
      p->next = NULL;
      receivesize += recvd;
      le0.rxcurrent->status &= ~(FP1 | FP0);
      le0.rxcurrent->status |= ACT;
      le0.rxcurrent = le0.rxcurrent->next;
#if defined(RX63N)
      if (EDMAC.EDRRR.LONG == 0x00000000L) {
        /* Restart if stopped */
        EDMAC.EDRRR.LONG = 0x00000001L;
#endif
#if defined(RX64M) || defined(RX65N)
      if (EDMAC0.EDRRR.LONG == 0x00000000L) {
        /* Restart if stopped */
        EDMAC0.EDRRR.LONG = 0x00000001L;
#endif
      }
    }
  }
  rx_enable_irq();
#if defined(DEBUG_ETHERNETIF)
  if (recvd > 0) {
      debug_printf("ETHRX:%d\r\n", recvd);
  }
#endif
  return p;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
void
ethernetif_input(struct netif *netif)
{
  //struct ethernetif *ethernetif;
  //struct eth_hdr *ethhdr;
  struct pbuf *p;

  //ethernetif = netif->state;

  /* move received packet into a new pbuf */
  p = low_level_input(netif);
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
    if (g_netif != NULL) {
        ethernetif_input(g_netif);
    }
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t
ethernetif_init(struct netif *netif)
{
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
  netif->linkoutput = low_level_output;

  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}

void
ethernetif_set_link(struct netif *netif)
{
#if defined(DEBUG_ETHERNETIF)
  debug_printf("ethernetif_set_link\r\n");
#endif
  uint32_t regvalue = 0;
  /* Read PHY_MISR*/
   phy_read(PHY_MISR, &regvalue);

  /* Check whether the link interrupt has occurred or not */
  if((regvalue & PHY_LINK_INTERRUPT) != (uint16_t)RESET)
  {
    /* Read PHY_SR*/
    phy_read(PHY_SR, &regvalue);

    /* Check whether the link is up or down*/
    if((regvalue & PHY_LINK_STATUS)!= (uint16_t)RESET)
    {
      netif_set_link_up(netif);
#if defined(DEBUG_ETHERNETIF)
      debug_printf("link up\r\n");
#endif
    }
    else
    {
      netif_set_link_down(netif);
#if defined(DEBUG_ETHERNETIF)
      debug_printf("link down\r\n");
#endif
    }
  }
}

err_t
ethernetif_update_config(struct netif *netif) {
#if defined(DEBUG_ETHERNETIF)
  debug_printf("ethernetif_update_config\r\n");
#endif
  if(netif_is_link_up(netif))
  {
    /* Auto-Negotiation */
    phy_set_link_speed();
    /* Restart MAC interface */
    rx_ether_start();
  }
  else
  {
    /* Stop MAC interface */
    rx_ether_deinit();
#if defined(DEBUG_ETHERNETIF)
    debug_printf("ETH Stop\r\n");
#endif
  }
  return ERR_OK;
}

#endif
