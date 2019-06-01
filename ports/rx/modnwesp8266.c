/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Damien P. George
 * Copyright (c) 2019 Kentaro Sekimoto
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
#include <stdarg.h>
#ifdef __linux__
#include <sys/select.h>
#endif

// ESP8266 defines its own ENOBUFS (different to standard one!)
#undef ENOBUFS

#include "py/objtuple.h"
#include "py/objlist.h"
#include "py/stream.h"
#include "py/runtime.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "lib/netutils/netutils.h"
#include "modnetwork.h"
#include "pin.h"
#include "spi.h"

#include "esp8266_driver.h"

//#define DEBUG_MODNWESP8266

#define ESP8266_EXPORT(name) esp8266_ ## name

//#define MAX_ADDRSTRLEN      (128)
//#define MAX_RX_PACKET       (ESP8266_RX_BUFFER_SIZE-ESP8266_MINIMAL_RX_SIZE-1)
//#define MAX_TX_PACKET       (ESP8266_TX_BUFFER_SIZE-ESP8266_MINIMAL_TX_SIZE-1)

#define MAKE_SOCKADDR(addr, ip, port) \
    sockaddr addr; \
    addr.sa_family = AF_INET; \
    addr.sa_data[0] = port >> 8; \
    addr.sa_data[1] = port; \
    addr.sa_data[2] = ip[0]; \
    addr.sa_data[3] = ip[1]; \
    addr.sa_data[4] = ip[2]; \
    addr.sa_data[5] = ip[3];

#define UNPACK_SOCKADDR(addr, ip, port) \
    port = (addr.sa_data[0] << 8) | addr.sa_data[1]; \
    ip[0] = addr.sa_data[2]; \
    ip[1] = addr.sa_data[3]; \
    ip[2] = addr.sa_data[4]; \
    ip[3] = addr.sa_data[5];

STATIC int mod_esp8266_socket_ioctl(mod_network_socket_obj_t *socket, mp_uint_t request, mp_uint_t arg, int *_errno);

//int ESP8266_EXPORT(errno); // for esp8266 driver

STATIC volatile uint32_t fd_closed_state = 0;
STATIC volatile bool wlan_connected = false;
STATIC volatile bool ip_obtained = false;

STATIC int esp8266_get_fd_closed_state(int fd) {
    return fd_closed_state & (1 << fd);
}
#pragma GCC diagnostic pop

STATIC void esp8266_set_fd_closed_state(int fd) {
    fd_closed_state |= 1 << fd;
}

STATIC void esp8266_reset_fd_closed_state(int fd) {
    fd_closed_state &= ~(1 << fd);
}

STATIC int mod_esp8266_gethostbyname(mp_obj_t nic, const char *name, mp_uint_t len, uint8_t *out_ip) {
#if defined(DEBUG_MODNWESP8266)
    debug_printf("mod_esp8266_gethostbyname\r\n");
#endif
    uint32_t ip = 0;
    bool ret = esp8266_gethostbyname(name, out_ip);
    ip += ((uint32_t)out_ip[0] << 24);
    ip += ((uint32_t)out_ip[1] << 16);
    ip += ((uint32_t)out_ip[2] << 8);
    ip += ((uint32_t)out_ip[3]);
    if (ret == false || ip == 0) {
        // unknown host
        return -2;
    }
    return 0;
}

STATIC int mod_esp8266_socket_socket(mod_network_socket_obj_t *socket, int *_errno) {
#if defined(DEBUG_MODNWESP8266)
    debug_printf("mod_esp8266_socket_socket\r\n");
#endif
    //if (socket->u_param.domain != MOD_NETWORK_AF_INET) {
    //    *_errno = MP_EAFNOSUPPORT;
    //    return -1;
    //}
    //mp_uint_t type;
    //switch (socket->u_param.type) {
    //    case MOD_NETWORK_SOCK_STREAM: type = SOCK_STREAM; break;
    //    case MOD_NETWORK_SOCK_DGRAM: type = SOCK_DGRAM; break;
    //    case MOD_NETWORK_SOCK_RAW: type = SOCK_RAW; break;
    //    default: *_errno = MP_EINVAL; return -1;
    //}
    // open socket
    void *handle;
    int errno = esp8266_socket_open(&handle, ESP8266_TCP);
    //int fd = ESP8266_EXPORT(socket_open)(AF_INET, type, 0);
    //if (fd < 0) {
    //    *_errno = ESP8266_EXPORT(errno);
    //    return -1;
    //}

    // clear socket state
    //esp8266_reset_fd_closed_state(fd);

    // store state of this socket
    //socket->handle = fd;
    socket->handle = (mp_uint_t )handle;

    // make accept blocking by default
    int optval = SOCK_OFF;
    unsigned optlen = sizeof(optval);
    //ESP8266_EXPORT(setsockopt)(socket->handle, ESP8266_SOCKET, ESP8266_ACCEPT_NONBLOCK, &optval, optlen);
    esp8266_setsockopt(socket->handle, ESP8266_SOCKET, ESP8266_ACCEPT_NONBLOCK, (const void *)&optval, (unsigned)optlen);
    return 0;
}

STATIC void mod_esp8266_socket_close(mod_network_socket_obj_t *socket) {
#if defined(DEBUG_MODNWESP8266)
    debug_printf("mod_esp8266_socket_close\r\n");
#endif
    esp8266_socket_close((void *)socket->handle);
}

STATIC int mod_esp8266_socket_bind(mod_network_socket_obj_t *socket, byte *ip, mp_uint_t port, int *_errno) {
#if defined(DEBUG_MODNWESP8266)
    debug_printf("mod_esp8266_socket_bind\r\n");
#endif
    //MAKE_SOCKADDR(addr, ip, port)
    esp8266_socket_address_t socket_addr;
    // ToDo
    //socket_addr._addr;
    socket_addr._ip_address = (char *)ip;
    socket_addr._port = (uint16_t)port;
    int ret = esp8266_socket_bind((void *)socket->handle, (const esp8266_socket_address_t *)&socket_addr);
    if (ret != 0) {
        *_errno = ret;
        return -1;
    }
    return 0;
}

STATIC int mod_esp8266_socket_listen(mod_network_socket_obj_t *socket, mp_int_t backlog, int *_errno) {
#if defined(DEBUG_MODNWESP8266)
    debug_printf("mod_esp8266_socket_listen\r\n");
#endif
    bool ret = esp8266_socket_listen((void *)socket->handle, (int)backlog);
    if (!ret) {
        *_errno = ret;
        return -1;
    }
    return 0;
}

STATIC int mod_esp8266_socket_accept(mod_network_socket_obj_t *socket, mod_network_socket_obj_t *socket2, byte *ip, mp_uint_t *port, int *_errno) {
#if defined(DEBUG_MODNWESP8266)
    debug_printf("mod_esp8266_socket_accept\r\n");
#endif
    // accept incoming connection
    bool ret;
    void *sock;
    esp8266_socket_address_t socket_addr;
    //sockaddr addr;
    //socklen_t addr_len = sizeof(addr);
    ret = esp8266_socket_accept((void *)socket->handle, &sock, &socket_addr);
    //if ((fd = ESP8266_EXPORT(accept)(socket->handle, &addr, &addr_len)) < 0) {
    //    if (fd == SOC_IN_PROGRESS) {
    //        *_errno = MP_EAGAIN;
    //    } else {
    //        *_errno = -fd;
    //    }
    //    return -1;
    //}

    // clear socket state
    //esp8266_reset_fd_closed_state(fd);

    // store state in new socket object
    socket2->handle = (mp_uint_t)sock;

    // return ip and port
    // it seems ESP8266 returns little endian for accept??
    //UNPACK_SOCKADDR(addr, ip, *port);
    //*port = (addr.sa_data[1] << 8) | addr.sa_data[0];
    //ip[3] = addr.sa_data[2];
    //ip[2] = addr.sa_data[3];
    //ip[1] = addr.sa_data[4];
    //ip[0] = addr.sa_data[5];
    return (ret)? 0:1;
}

STATIC int mod_esp8266_socket_connect(mod_network_socket_obj_t *socket, byte *ip, mp_uint_t port, int *_errno) {
#if defined(DEBUG_MODNWESP8266)
    debug_printf("mod_esp8266_socket_connect\r\n");
#endif
    esp8266_socket_address_t socket_addr;
    socket_addr._ip_address = (char *)ip;
    socket_addr._port = port;
    bool ret = esp8266_socket_connect((void *)socket->handle, (const esp8266_socket_address_t *)&socket_addr);
    if (ret) {
        *_errno = 0;
    } else {
        *_errno = -1;
    }
    return ret? 0 : -1;
}

#define MAX_TX_PACKET 2048
#define MAX_RX_PACKET 2048

STATIC mp_uint_t mod_esp8266_socket_send(mod_network_socket_obj_t *socket, const byte *buf, mp_uint_t len, int *_errno) {
#if defined(DEBUG_MODNWESP8266)
    debug_printf("mod_esp8266_socket_send\r\n");
#endif
    mp_int_t bytes = 0;
    while (bytes < len) {
        int n = MIN((len - bytes), MAX_TX_PACKET);
        //n = ESP8266_EXPORT(send)(socket->handle, (uint8_t*)buf + bytes, n, 0);
        n = esp8266_socket_send((void *)socket->handle, (const void *)(buf + bytes), (unsigned)n);
        if (n <= 0) {
            //*_errno = ESP8266_EXPORT(errno);
            return -1;
        }
        bytes += n;
    }
    return bytes;
}

STATIC mp_uint_t mod_esp8266_socket_sendall(mod_network_socket_obj_t *socket, const byte *buf, mp_uint_t len, int *_errno) {
#if defined(DEBUG_MODNWESP8266)
    debug_printf("mod_esp8266_socket_sendall\r\n");
#endif
    mp_int_t bytes = 0;
    while (bytes < len) {
        int n = MIN((len - bytes), MAX_TX_PACKET);
        n = esp8266_socket_send((void *)socket->handle, (const void *)(buf + bytes), (unsigned)n);
        if (n <= 0) {
            //*_errno = ESP8266_EXPORT(errno);
            return -1;
        }
        bytes += n;
    }
    return bytes;
}

STATIC mp_uint_t mod_esp8266_socket_recv(mod_network_socket_obj_t *socket, byte *buf, mp_uint_t len, int *_errno) {
    // check the socket is open
    //if (esp8266_get_fd_closed_state(socket->handle)) {
    //    // socket is closed, but ESP8266 may have some data remaining in buffer, so check
    //    fd_set rfds;
    //    FD_ZERO(&rfds);
    //    FD_SET(socket->handle, &rfds);
    //    esp8266_timeval tv;
    //    tv.tv_sec = 0;
    //    tv.tv_usec = 1;
    //    int nfds = ESP8266_EXPORT(select)(socket->handle + 1, &rfds, NULL, NULL, &tv);
    //    if (nfds == -1 || !FD_ISSET(socket->handle, &rfds)) {
    //        // no data waiting, so close socket and return 0 data
    //        ESP8266_EXPORT(closesocket)(socket->handle);
    //        return 0;
    //    }
    //}
    // cap length at MAX_RX_PACKET
    len = MIN(len, MAX_RX_PACKET);

    // do the recv
    //int ret = ESP8266_EXPORT(recv)(socket->handle, buf, len, 0);
    int ret = esp8266_socket_recv((void *)socket->handle, (void *)buf, (unsigned)len);
    if (ret < 0) {
        //*_errno = ESP8266_EXPORT(errno);
        return -1;
    }

    return ret;
}

STATIC mp_uint_t mod_esp8266_socket_sendto(mod_network_socket_obj_t *socket, const byte *buf, mp_uint_t len, byte *ip, mp_uint_t port, int *_errno) {
#if defined(DEBUG_MODNWESP8266)
    debug_printf("mod_esp8266_socket_sendto\r\n");
#endif
    // ToDo:
    esp8266_socket_address_t socket_addr;
    //MAKE_SOCKADDR(addr, ip, port)
    //int ret = ESP8266_EXPORT(sendto)(socket->handle, (byte*)buf, len, 0, (sockaddr*)&addr, sizeof(addr));
    int ret = esp8266_socket_sendto((void *)socket->handle, (const esp8266_socket_address_t *)&socket_addr, (const void *)buf, (unsigned)len);
    if (ret < 0) {
        //*_errno = ESP8266_EXPORT(errno);
        return -1;
    }
    return ret;
}

STATIC mp_uint_t mod_esp8266_socket_recvfrom(mod_network_socket_obj_t *socket, byte *buf, mp_uint_t len, byte *ip, mp_uint_t *port, int *_errno) {
#if defined(DEBUG_MODNWESP8266)
    debug_printf("mod_esp8266_socket_recvfrom\r\n");
#endif
    // ToDo:
    esp8266_socket_address_t socket_addr;
    //sockaddr addr;
    //socklen_t addr_len = sizeof(addr);
    //mp_int_t ret = ESP8266_EXPORT(recvfrom)(socket->handle, buf, len, 0, &addr, &addr_len);
    mp_int_t ret = esp8266_socket_recvfrom((void *)socket->handle, &socket_addr, (void *)buf, (unsigned)len);
    if (ret < 0) {
        //*_errno = ESP8266_EXPORT(errno);
        return -1;
    }
    //UNPACK_SOCKADDR(addr, ip, *port);
    return ret;
}

STATIC int mod_esp8266_socket_setsockopt(mod_network_socket_obj_t *socket, mp_uint_t level, mp_uint_t opt, const void *optval, mp_uint_t optlen, int *_errno) {
#if defined(DEBUG_MODNWESP8266)
    debug_printf("mod_esp8266_socket_setsockopt\r\n");
#endif
    //int ret = ESP8266_EXPORT(setsockopt)(socket->handle, level, opt, optval, optlen);
    int ret = esp8266_setsockopt((int)socket->handle, (int)level, (int)opt, (const void *)optval, (unsigned )optlen);
    if (ret < 0) {
        //*_errno = ESP8266_EXPORT(errno);
        return -1;
    }
    return 0;
}

STATIC int mod_esp8266_socket_settimeout(mod_network_socket_obj_t *socket, mp_uint_t timeout_ms, int *_errno) {
#if defined(DEBUG_MODNWESP8266)
    debug_printf("mod_esp8266_socket_settimeout\r\n");
#endif
    int ret;
    if (timeout_ms == 0 || timeout_ms == -1) {
        int optval;
        unsigned optlen = sizeof(optval);
        if (timeout_ms == 0) {
            // set non-blocking mode
            optval = SOCK_ON;
        } else {
            // set blocking mode
            optval = SOCK_OFF;
        }
        //ret = ESP8266_EXPORT(setsockopt)(socket->handle, ESP8266_SOCKET, ESP8266_RECV_NONBLOCK, &optval, optlen);
        ret = esp8266_setsockopt(socket->handle, ESP8266_SOCKET, ESP8266_RECV_NONBLOCK, (const void *)&optval, (unsigned)optlen);
        if (ret == 0) {
            //ret = ESP8266_EXPORT(setsockopt)(socket->handle, ESP8266_SOCKET, ESP8266_ACCEPT_NONBLOCK, &optval, optlen);
            ret = esp8266_setsockopt(socket->handle, ESP8266_SOCKET, ESP8266_ACCEPT_NONBLOCK, (const void *)&optval, (unsigned)optlen);
        }
    } else {
        unsigned optlen = sizeof(timeout_ms);
        ret = ESP8266_EXPORT(setsockopt)(socket->handle, ESP8266_SOCKET, ESP8266_RECV_TIMEOUT, &timeout_ms, optlen);
    }
    if (ret != 0) {
        //*_errno = ESP8266_EXPORT(errno);
        return -1;
    }
    return 0;
}

STATIC int mod_esp8266_socket_ioctl(mod_network_socket_obj_t *socket, mp_uint_t request, mp_uint_t arg, int *_errno) {
#if defined(DEBUG_MODNWESP8266)
    debug_printf("mod_esp8266_socket_ioctl\r\n");
#endif
    mp_uint_t ret = 0;
    if (request == MP_STREAM_POLL) {
        mp_uint_t flags = arg;
        ret = 0;
        int fd = socket->handle;

        // init fds
        fd_set rfds, wfds, xfds;
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_ZERO(&xfds);

        // set fds if needed
        if (flags & MP_STREAM_POLL_RD) {
            FD_SET(fd, &rfds);

            // A socked that just closed is available for reading.  A call to
            // recv() returns 0 which is consistent with BSD.
            if (esp8266_get_fd_closed_state(fd)) {
                ret |= MP_STREAM_POLL_RD;
            }
        }
        if (flags & MP_STREAM_POLL_WR) {
            FD_SET(fd, &wfds);
        }
        if (flags & MP_STREAM_POLL_HUP) {
            FD_SET(fd, &xfds);
        }

        // call esp8266 select with minimum timeout
        //esp8266_timeval tv;
        //tv.tv_sec = 0;
        //tv.tv_usec = 1;
        //int nfds = ESP8266_EXPORT(select)(fd + 1, &rfds, &wfds, &xfds, &tv);

        // check for error
        //if (nfds == -1) {
        //    //*_errno = ESP8266_EXPORT(errno);
        //    return -1;
        //}

        // check return of select
        if (FD_ISSET(fd, &rfds)) {
            ret |= MP_STREAM_POLL_RD;
        }
        if (FD_ISSET(fd, &wfds)) {
            ret |= MP_STREAM_POLL_WR;
        }
        if (FD_ISSET(fd, &xfds)) {
            ret |= MP_STREAM_POLL_HUP;
        }
    } else {
        *_errno = MP_EINVAL;
        ret = -1;
    }
    return ret;
}

/******************************************************************************/
// MicroPython bindings; ESP8266 class

typedef struct _esp8266_obj_t {
    mp_obj_base_t base;
} esp8266_obj_t;

STATIC const esp8266_obj_t esp8266_obj = {{(mp_obj_type_t*)&mod_network_socket_nic_type_esp8266}};

// \classmethod \constructor()
STATIC mp_obj_t esp8266_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

#define AT_MAX 32
#define SDK_MAX 32
    char at_ver[AT_MAX];
    char sdk_ver[SDK_MAX];
    esp8266_driver_init();
    //if (!esp8266_driver_reset()) {
    //    nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_OSError, "failed to init ESP8266 module\n"));
    //}
    if (esp8266_AT_GMR(at_ver, sizeof(at_ver), sdk_ver, sizeof(sdk_ver))) {
        printf("AT ver=%s\n", at_ver);
        printf("SDK ver=%s\n", sdk_ver);
    } else {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_OSError, "can't get ESP vesions\n"));
    }
    esp8266_set_AT_CWMODE(3);
    // register with network module
    mod_network_register_nic((mp_obj_t)&esp8266_obj);
    return (mp_obj_t)&esp8266_obj;
}

// method connect(ssid, key=None, *, security=WPA2, bssid=None)
STATIC mp_obj_t esp8266_connect(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_ssid, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_key, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_security, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = WLAN_SEC_WPA2} },
        { MP_QSTR_bssid, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    // get ssid
    size_t ssid_len;
    const char *ssid = mp_obj_str_get_data(args[0].u_obj, &ssid_len);
    // get key and sec
    size_t key_len = 0;
    const char *key = NULL;
    // ToDo: implement security and bssid
    //mp_uint_t sec = WLAN_SEC_UNSEC;
    if (args[1].u_obj != mp_const_none) {
        key = mp_obj_str_get_data(args[1].u_obj, &key_len);
        //sec = args[2].u_int;
    }
    // get bssid
    //const char *bssid = NULL;
    //if (args[3].u_obj != mp_const_none) {
    //    bssid = mp_obj_str_get_str(args[3].u_obj);
    //}
    if (!esp8266_set_AT_CWJAP(ssid, key)) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_OSError, "could not connect to ssid=%s, key=%s\n", ssid, key));
    }
    esp8266_set_AT_CIPMUX(1);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(esp8266_connect_obj, 1, esp8266_connect);

STATIC mp_obj_t esp8266_disconnect(mp_obj_t self_in) {
    // should we check return value?
    // disconnects from the AP
    esp8266_AT_CWQAP();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(esp8266_disconnect_obj, esp8266_disconnect);

STATIC mp_obj_t esp8266_isconnected(mp_obj_t self_in) {
#if defined(DEBUG_MODNWESP8266)
    debug_printf("esp8266_isconnected\r\n");
#endif
    return mp_obj_new_bool(wlan_connected && ip_obtained);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(esp8266_isconnected_obj, esp8266_isconnected);

STATIC mp_obj_t esp8266_ifconfig(size_t n_args, const mp_obj_t *args) {
    uint8_t ip[4];
    uint8_t gw[4];
    uint8_t mask[4];
    uint8_t dns[4] = { 0, 0, 0, 0 };
    mp_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    if (n_args == 1) {
        uint8_t dhcp[4] = { 0, 0, 0, 0 };
        uint8_t mac[6] = { 0, 0, 0, 0, 0, 0 };
        esp8266_get_AT_CIPSTA(ip, gw, mask);
        //esp8266_get_AT_CIPSTAMAC_CUR((uint8_t *)mac);
        // render MAC address
        //VSTR_FIXED(mac_vstr, 18);
        //vstr_printf(&mac_vstr, "%02x:%02x:%02x:%02x:%02x:%02x", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
        //esp8266_get_AT_CWJAP_CUR(ssid, NULL, NULL, NULL);
        mp_obj_t tuple[3] = {
            netutils_format_ipv4_addr(ip, NETUTILS_BIG),
            netutils_format_ipv4_addr(gw, NETUTILS_BIG),
            netutils_format_ipv4_addr(mask, NETUTILS_BIG), };
        return mp_obj_new_tuple(MP_ARRAY_SIZE(tuple), tuple);
    } else if (n_args == 2) {
        mp_obj_t *items;
        mp_obj_get_array_fixed_n(args[1], 4, &items);
        size_t ip_len;
        size_t gw_len;
        size_t mask_len;
        size_t dns_len;
        const char *ip_str = mp_obj_str_get_data(items[0], &ip_len);
        const char *gw_str = mp_obj_str_get_data(items[1], &gw_len);
        const char *mask_str = mp_obj_str_get_data(items[2], &mask_len);
        const char *dns_str = mp_obj_str_get_data(items[3], &dns_len);
        if ((ip_len <= 0) || (gw_len <= 0) || (mask_len <= 0) || (dns_len <= 0)) {
            nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_OSError, "ip, gw, mask should be properly inputted.\n"));
        }
        bool ret = esp8266_set_AT_CIPSTA(ip_str, gw_str, mask_str);
        if (!ret) {
            nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_OSError, "could not configure ip=%s, gw=%s, mask=%s\n", ip_str, gw_str, mask_str));
        }
        ret = esp8266_set_AT_CIPDNS_CUR(dns_str, true);
        if (!ret) {
            nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_OSError, "could not configure dns=%s\n", dns_str));
        }
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(esp8266_ifconfig_obj, 1, 2, esp8266_ifconfig);

STATIC const mp_rom_map_elem_t esp8266_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_connect), MP_ROM_PTR(&esp8266_connect_obj) },
    { MP_ROM_QSTR(MP_QSTR_disconnect), MP_ROM_PTR(&esp8266_disconnect_obj) },
    { MP_ROM_QSTR(MP_QSTR_isconnected), MP_ROM_PTR(&esp8266_isconnected_obj) },
    { MP_ROM_QSTR(MP_QSTR_ifconfig), MP_ROM_PTR(&esp8266_ifconfig_obj) },

    // class constants
    { MP_ROM_QSTR(MP_QSTR_WEP), MP_ROM_INT(WLAN_SEC_WEP) },
    { MP_ROM_QSTR(MP_QSTR_WPA), MP_ROM_INT(WLAN_SEC_WPA) },
    { MP_ROM_QSTR(MP_QSTR_WPA2), MP_ROM_INT(WLAN_SEC_WPA2) },
};
STATIC MP_DEFINE_CONST_DICT(esp8266_locals_dict, esp8266_locals_dict_table);

const mod_network_socket_nic_type_t mod_network_socket_nic_type_esp8266 = {
    .base = {
        { &mp_type_type },
        .name = MP_QSTR_ESP8266,
        .make_new = esp8266_make_new,
        .locals_dict = (mp_obj_dict_t*)&esp8266_locals_dict,
    },
    .gethostbyname = mod_esp8266_gethostbyname,
    .socket = mod_esp8266_socket_socket,
    .close = mod_esp8266_socket_close,
    .bind = mod_esp8266_socket_bind,
    .listen = mod_esp8266_socket_listen,
    .accept = mod_esp8266_socket_accept,
    .connect = mod_esp8266_socket_connect,
    .send = mod_esp8266_socket_send,
    .sendall = mod_esp8266_socket_sendall,
    .recv = mod_esp8266_socket_recv,
    .sendto = mod_esp8266_socket_sendto,
    .recvfrom = mod_esp8266_socket_recvfrom,
    .setsockopt = mod_esp8266_socket_setsockopt,
    .settimeout = mod_esp8266_socket_settimeout,
    .ioctl = mod_esp8266_socket_ioctl,
};
