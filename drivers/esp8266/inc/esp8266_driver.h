/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
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

#ifndef ESP8266_DRIVER_H
#define ESP8266_DRIVER_H

#ifdef  __cplusplus
extern "C" {
#endif

//#define MOD_NETWORK_IPADDR_BUF_SIZE (4)
//#define MOD_NETWORK_AF_INET (2)
//#define MOD_NETWORK_AF_INET6 (10)
//#define MOD_NETWORK_SOCK_STREAM (1)
//#define MOD_NETWORK_SOCK_DGRAM (2)
//#define MOD_NETWORK_SOCK_RAW (3)

#define WLAN_SEC_UNSEC (0)
#define WLAN_SEC_WEP   (1)
#define WLAN_SEC_WPA   (2)
#define WLAN_SEC_WPA2  (3)

#define IPADDR_BUF_SIZE 4
#define AF_INET     2
#define AF_INET6    10
#define SOCK_STREAM 1
#define SOCK_DGRAM  2
#define SOCK_RAW    3

#define SOCK_ON     0
#define SOCK_OFF    1

#define ESP8266_SOCKET              0xffff
#define ESP8266_RECV_NONBLOCK       0   // recv non block mode, set SOCK_ON or SOCK_OFF (default block mode)
#define ESP8266_RECV_TIMEOUT        1   // optname to configure recv and recvfromtimeout
#define ESP8266_ACCEPT_NONBLOCK     2   // accept non block mode, set SOCK_ON or SOCK_OFF (default block mode)
#define ESP8266_KEEPALIVE           3

#define ESP8266_AT_VERSION_TCP_PASSIVE_MODE 1070000
#define ESP8266_SOCKET_COUNT    5
#define ESP8266_CONNECT_TIMEOUT 15000
#define ESP8266_SOCKET_BUFSIZE  16384

#define CIPSEND_MAX 2048

#define ESP8266_ALL_SOCKET_IDS      -1
#define ESP8266_ERROR_PARAMETER     -1

typedef enum esp8266_protocol {
    ESP8266_TCP,
    ESP8266_UDP,
} esp8266_protocol_t;

typedef struct esp8266_addr {
    uint8_t bytes[IPADDR_BUF_SIZE];
} esp8266_addr_t;

typedef struct esp8266_socket_address {
    char *_ip_address;
    esp8266_addr_t _addr;
    uint16_t _port;
} esp8266_socket_address_t;

typedef struct esp8266_socket {
    int id;
    esp8266_protocol_t proto;
    esp8266_socket_address_t addr;
    bool connected;
    int keepalive; // TCP
    bool accept_id;
    bool tcp_server;
} esp8266_socket_t;

typedef struct socket_info {
    bool open;
    uint16_t sport;
    int proto;
} socket_info_t;

typedef struct driver_info {
    bool open;
    int  proto;
    char *tcp_data;
    int32_t tcp_data_avbl;
    int32_t tcp_data_rcvd;
} driver_info_t;

typedef struct _packet {
    struct _packet *next;
    int id;
    uint32_t len;       // Remaining length
    uint32_t alloc_len; // Original length
    // data follows
} _packet_t;

void esp8266_driver_init(void);
bool esp8266_driver_reset(void);
bool esp8266_AT(void);
bool esp8266_AT_RST(void);
bool esp8266_AT_CWQAP(void);
bool esp8266_AT_GMR(char *at_ver, size_t at_len, char *sdk_ver, size_t sdk_len);
bool esp8266_set_AT_CIPMUX(uint8_t mode);
bool esp8266_get_AT_CWMODE(uint8_t *mode);
bool esp8266_set_AT_CWMODE(uint8_t mode);
bool esp8266_set_AT_CWJAP(const char *ssid, const char *pwd);
bool esp8266_get_AT_CWJAP_CUR(char *ssid, char *bssid, char *channel, char *rssi);
bool esp8266_set_AT_CWDHCP(uint8_t mode, bool enabled);
bool esp8266_get_AT_CIPSTAMAC_CUR(uint8_t *mac);
bool esp8266_get_AT_CIPSTA(uint8_t *ip, uint8_t *gw, uint8_t *mask);
bool esp8266_set_AT_CIPSTA(const char *ip, const char *gw, const char *mask);
bool esp8266_set_AT_CIPDNS_CUR(const char *dns, bool flag);
bool esp8266_set_AT_CIPSERVER(int port);
bool esp8266_reset_AT_CIPSERVER(void);

bool esp8266_gethostbyname(const char *name, unsigned char *ip);
int esp8266_socket_open(void **handle, esp8266_protocol_t proto);
int esp8266_socket_close(void *handle);
int esp8266_socket_bind(void *handle, const esp8266_socket_address_t *address);
bool esp8266_socket_listen(void *handle, int backlog);
bool esp8266_socket_connect(void *handle, const esp8266_socket_address_t *addr);
bool esp8266_socket_accept(void *server, void **socket, esp8266_socket_address_t *addr);
int esp8266_socket_send(void *handle, const void *data, unsigned size);
int esp8266_socket_recv(void *handle, void *data, unsigned size);
int esp8266_socket_sendto(void *handle, const esp8266_socket_address_t *addr, const void *data, unsigned size);
int esp8266_socket_recvfrom(void *handle, esp8266_socket_address_t *addr, void *data, unsigned size);
int esp8266_setsockopt(int handle, int level, int optname, const void *optval, unsigned optlen);

//bool esp8266_open_tcp(int id, const char *addr, int port, int keepalive);
//bool esp8266_open_udp(int id, const char *addr, int port, int local_port);
//bool esp8266_send(int id, const void *data, uint32_t amount);
//int32_t esp8266_recv_tcp(int id, const void *data, uint32_t amount);
//int32_t esp8266_recv_udp(int id, const void *data, uint32_t amount);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif // ESP8266_DRIVER_H
