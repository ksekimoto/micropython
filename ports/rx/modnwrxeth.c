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

/*
 * refer to https://github.com/boochow/micropython/blob/stm32-ethernet/ports/stm32/network_ethernet.c
 */

#include <stdio.h>
#include <string.h>
#include "py/runtime.h"
#include "py/objstr.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "lib/netutils/netutils.h"

#if MICROPY_HW_HAS_ETHERNET && MICROPY_PY_LWIP

#include "lwip/opt.h"
#include "lwip_inc/lwipopts.h"
#include "lwip/init.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "lwip/igmp.h"
#include "lwip/dhcp.h"
#include "lwip/timeouts.h"
#include "lwip/prot/dhcp.h"
#include "netif/etharp.h"
#include "ethernetif.h"

struct netif *g_netif;

typedef struct {
    mp_obj_base_t base;
    void (*poll_callback)(void*);
    struct netif netif;
    struct dhcp dhcp_struct;
    mp_uint_t last_arp;
} rx_ether_obj_t;

const mp_obj_type_t mod_network_nic_type_rx_ether;

STATIC rx_ether_obj_t rx_ether_obj;

//u32_t sys_now(void) {
//    return (u32_t)mp_hal_ticks_ms();
//}

STATIC void rx_ether_lwip_poll(void *self_in) {
    rx_ether_obj_t *self = self_in;
    mp_uint_t t;

    ethernetif_input(&self->netif);
    t = mp_hal_ticks_ms();
    if ((t - self->last_arp) > ARP_TMR_INTERVAL) {
      etharp_tmr();
      self->last_arp = t;
    }
}

/*
 * lwip initialize
 */
STATIC void rx_ether_lwip_init(rx_ether_obj_t *self) {
    for (struct netif *netif = netif_list; netif != NULL; netif = netif->next) {
        if (netif == &self->netif) {
            netif_remove(netif);
            break;
        }
    }
    ip_addr_t ipconfig[4];
    ipconfig[0].addr = 0;
    ipconfig[1].addr = 0;
    ipconfig[2].addr = 0;
    ipconfig[3].addr = 0;

    g_netif = &self->netif;
    memset(&self->netif, 0, sizeof(struct netif));
    self-> netif.linkoutput = low_level_output;

    netif_add(&self->netif, &ipconfig[0], &ipconfig[1], &ipconfig[2], self, ethernetif_init, ethernet_input);
    netif_set_default(&self->netif);
    dns_setserver(0, &ipconfig[3]);
}

STATIC mp_obj_t rx_ether_lwip_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    // init the ethernet object
    rx_ether_obj.base.type = &mod_network_nic_type_rx_ether;
    rx_ether_obj.poll_callback = rx_ether_lwip_poll;

    // Hardware init
    rx_ether_lwip_init(&rx_ether_obj);

    return &rx_ether_obj;
}

STATIC mp_obj_t rx_ether_lwip_isconnected(mp_obj_t self_in) {
    rx_ether_obj_t *self = self_in;
    ethernetif_set_link(&self->netif);
    return mp_obj_new_bool(self->netif.flags & NETIF_FLAG_LINK_UP);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rx_ether_lwip_isconnected_obj, rx_ether_lwip_isconnected);

STATIC mp_obj_t rx_ether_lwip_active(size_t n_args, const mp_obj_t *args) {
    rx_ether_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    if (n_args == 1) {
        return mp_obj_new_bool(self->netif.flags & NETIF_FLAG_UP);
    } else {
        if (mp_obj_is_true(args[1])) {
            netif_set_up(&self->netif);
            netif_set_link_up(&self->netif);
        } else {
            netif_set_link_down(&self->netif);
            netif_set_down(&self->netif);
        }
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rx_ether_lwip_active_obj, 1, 2, rx_ether_lwip_active);

STATIC mp_obj_t rx_ether_lwip_ifconfig(size_t n_args, const mp_obj_t *args) {
    rx_ether_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    if (n_args == 1) {
        // get
        const ip_addr_t *dns = dns_getserver(0);
        mp_obj_t tuple[4] = {
            netutils_format_ipv4_addr((uint8_t*)&self->netif.ip_addr, NETUTILS_BIG),
            netutils_format_ipv4_addr((uint8_t*)&self->netif.netmask, NETUTILS_BIG),
            netutils_format_ipv4_addr((uint8_t*)&self->netif.gw, NETUTILS_BIG),
            netutils_format_ipv4_addr((uint8_t*)dns, NETUTILS_BIG),
        };
        return mp_obj_new_tuple(4, tuple);
    } else if (args[1] == MP_OBJ_NEW_QSTR(MP_QSTR_dhcp)) {
        dhcp_set_struct(&self->netif, &self->dhcp_struct);
        dhcp_start(&self->netif);

        uint32_t start = mp_hal_ticks_ms();
        while ((self->netif.ip_addr.addr == 0) && (mp_hal_ticks_ms() - start < 12000)) {
            sys_check_timeouts();
        }
        if (self->netif.ip_addr.addr == 0) {
            mp_raise_msg(&mp_type_OSError, "timeout waiting for DHCP to get IP address");
        }
        return mp_const_none;
    } else {
        // set
        mp_obj_t *items;
        mp_obj_get_array_fixed_n(args[1], 4, &items);
        netutils_parse_ipv4_addr(items[0], (uint8_t*)&self->netif.ip_addr, NETUTILS_BIG);
        netutils_parse_ipv4_addr(items[1], (uint8_t*)&self->netif.netmask, NETUTILS_BIG);
        netutils_parse_ipv4_addr(items[2], (uint8_t*)&self->netif.gw, NETUTILS_BIG);
        ip_addr_t dns;
        netutils_parse_ipv4_addr(items[3], (uint8_t*)&dns, NETUTILS_BIG);
        dns_setserver(0, &dns);
        ethernetif_update_config(&self->netif);
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rx_ether_lwip_ifconfig_obj, 1, 2, rx_ether_lwip_ifconfig);

STATIC const mp_rom_map_elem_t rx_ether_lwip_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_isconnected), MP_ROM_PTR(&rx_ether_lwip_isconnected_obj) },
    { MP_ROM_QSTR(MP_QSTR_active), MP_ROM_PTR(&rx_ether_lwip_active_obj) },
    { MP_ROM_QSTR(MP_QSTR_ifconfig), MP_ROM_PTR(&rx_ether_lwip_ifconfig_obj) },

};
STATIC MP_DEFINE_CONST_DICT(rx_ether_lwip_locals_dict, rx_ether_lwip_locals_dict_table);

const mp_obj_type_t mod_network_nic_type_rx_ether = {
    { &mp_type_type },
    .name = MP_QSTR_Ethernet,
    .make_new = rx_ether_lwip_make_new,
    .locals_dict = (mp_obj_dict_t*)&rx_ether_lwip_locals_dict,
};
#endif
