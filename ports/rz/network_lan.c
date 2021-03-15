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

#include <stdio.h>
#include <string.h>
#include "py/runtime.h"
#include "py/mphal.h"

#if defined(MICROPY_HW_ETH_RZ)

#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip_inc/lwipopts.h"
#include "lwip/init.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "lwip/igmp.h"
#include "lwip/dhcp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/timeouts.h"
#include "lwip/prot/dhcp.h"
#include "ethernetif.h"
#include "sntp_client.h"
#include "modnetwork.h"
#include "eth.h"

typedef struct _network_lan_obj_t {
    mp_obj_base_t base;
    eth_t *eth;
} network_lan_obj_t;

STATIC const network_lan_obj_t network_lan_eth0 = { { &network_lan_type }, &eth_instance };

STATIC void network_lan_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    network_lan_obj_t *self = MP_OBJ_TO_PTR(self_in);
    struct netif *netif = eth_netif(self->eth);
    int status = eth_link_status(self->eth);
    mp_printf(print, "<ETH %u %u.%u.%u.%u>",
        status,
        netif->ip_addr.addr & 0xff,
        netif->ip_addr.addr >> 8 & 0xff,
        netif->ip_addr.addr >> 16 & 0xff,
        netif->ip_addr.addr >> 24
        );
}

err_t eth_netif_output(struct netif *netif, struct pbuf *p);
err_t ethernetif_init(struct netif *netif);
void ethernetif_input(struct netif *netif);

/*
 * lwip initialize
 */
STATIC void network_lan_init(eth_t *self) {
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

    // g_netif = &self->netif;
    memset(&self->netif, 0, sizeof(struct netif));
    self->netif.linkoutput = eth_netif_output;

    netif_add(&self->netif, &ipconfig[0], &ipconfig[1], &ipconfig[2], self, ethernetif_init, ethernet_input);
    netif_set_default(&self->netif);
    dns_setserver(0, &ipconfig[3]);
}

STATIC mp_obj_t network_lan_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    const network_lan_obj_t *self = &network_lan_eth0;
    // rx_ether_obj.poll_callback = network_lan_poll;
    // Hardware init
    network_lan_init(&eth_instance);
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t network_lan_active(size_t n_args, const mp_obj_t *args) {
    network_lan_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    if (n_args == 1) {
        return mp_obj_new_bool(self->eth->netif.flags & NETIF_FLAG_UP);
    } else {
        if (mp_obj_is_true(args[1])) {
            netif_set_up(&self->eth->netif);
            netif_set_link_up(&self->eth->netif);
        } else {
            netif_set_link_down(&self->eth->netif);
            netif_set_down(&self->eth->netif);
        }
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(network_lan_active_obj, 1, 2, network_lan_active);

STATIC mp_obj_t network_lan_isconnected(mp_obj_t self_in) {
    network_lan_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_bool(eth_link_status(self->eth) == 3);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(network_lan_isconnected_obj, network_lan_isconnected);

STATIC mp_obj_t network_lan_ifconfig(size_t n_args, const mp_obj_t *args) {
    network_lan_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    return mod_network_nic_ifconfig(eth_netif(self->eth), n_args - 1, args + 1);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(network_lan_ifconfig_obj, 1, 2, network_lan_ifconfig);

STATIC mp_obj_t network_lan_status(size_t n_args, const mp_obj_t *args) {
    network_lan_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    (void)self;

    if (n_args == 1) {
        // No arguments: return link status
        return MP_OBJ_NEW_SMALL_INT(eth_link_status(self->eth));
    }

    mp_raise_ValueError(MP_ERROR_TEXT("unknown status param"));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(network_lan_status_obj, 1, 2, network_lan_status);

STATIC mp_obj_t network_lan_config(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    network_lan_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    if (kwargs->used == 0) {
        // Get config value
        if (n_args != 2) {
            mp_raise_TypeError(MP_ERROR_TEXT("must query one param"));
        }

        switch (mp_obj_str_get_qstr(args[1])) {
            case MP_QSTR_mac: {
                return mp_obj_new_bytes(&eth_netif(self->eth)->hwaddr[0], 6);
            }
            default:
                mp_raise_ValueError(MP_ERROR_TEXT("unknown config param"));
        }
    } else {
        // Set config value(s)
        if (n_args != 1) {
            mp_raise_TypeError(MP_ERROR_TEXT("can't specify pos and kw args"));
        }

        for (size_t i = 0; i < kwargs->alloc; ++i) {
            if (MP_MAP_SLOT_IS_FILLED(kwargs, i)) {
                mp_map_elem_t *e = &kwargs->table[i];
                switch (mp_obj_str_get_qstr(e->key)) {
                    case MP_QSTR_trace: {
                        eth_set_trace(self->eth, mp_obj_get_int(e->value));
                        break;
                    }
                    default:
                        mp_raise_ValueError(MP_ERROR_TEXT("unknown config param"));
                }
            }
        }

        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(network_lan_config_obj, 1, network_lan_config);

STATIC const mp_rom_map_elem_t network_lan_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_active), MP_ROM_PTR(&network_lan_active_obj) },
    { MP_ROM_QSTR(MP_QSTR_isconnected), MP_ROM_PTR(&network_lan_isconnected_obj) },
    { MP_ROM_QSTR(MP_QSTR_ifconfig), MP_ROM_PTR(&network_lan_ifconfig_obj) },
    { MP_ROM_QSTR(MP_QSTR_status), MP_ROM_PTR(&network_lan_status_obj) },
    { MP_ROM_QSTR(MP_QSTR_config), MP_ROM_PTR(&network_lan_config_obj) },
};
STATIC MP_DEFINE_CONST_DICT(network_lan_locals_dict, network_lan_locals_dict_table);

const mp_obj_type_t network_lan_type = {
    { &mp_type_type },
    .name = MP_QSTR_LAN,
    .print = network_lan_print,
    .make_new = network_lan_make_new,
    .locals_dict = (mp_obj_dict_t *)&network_lan_locals_dict,
};

#endif // defined(MICROPY_HW_ETH_RZ)
