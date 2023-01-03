/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Kentaro Sekimoto
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

/* ESP Example
 * Copyright (c) 2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "py/runtime.h"
#include "py/stream.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "vector.h"
#include "modmachine.h"
#include "esp_driver.h"

#include "mpy_debug.h"
#include "mpy_uart.h"

#define USE_MPY_MALLOC
#if !defined(USE_MPY_MALLOC)
#define USE_TINYMALLOC
#endif

#if defined(USE_TINYMALLOC)
#include "tinymalloc.h"
#endif

#if defined(USE_MPY_DEBUG) | defined(USE_SEGGER_RTT)
// #define DEBUG_ESP_MALLOC
#define DEBUG_ESP_ERR
// #define DEBUG_ESP_AT_ERR
// #define DEBUG_ESP_SOCKET_ERR
// #define DEBUG_ESP_RAW_DATA_RD
// #define DEBUG_ESP_RAW_DATA_WR
// #define DEBUG_ESP_PACKET
// #define DEBUG_ESP_DRIVER
// #define DEBUG_ESP_DRIVER_PACKET_HANDLE
// #define DEBUG_ESP_DRIVER_PACKET_READ
// #define DEBUG_ESP_SOCKET
// #define DEBUG_ESP_SOCKET_DUMP
// #define DEBUG_ESP_SOCKET_RECV
// #define DEBUG_ESP_SOCKET_SEND
// #define DEBUG_ESP_DUMP_HEX
#endif

uint32_t esp_read(uint32_t timeout);
void esp_socket_handler(bool connect, int id);

#define ESP_READ_TIMEOUT    600
#define ESP_ACCEPT_TIMEOUT  600
#define ESP_PACKET_TIMEOUT  600
#define WIFI_TIMEOUT        10000
#define WIFI_DOMAIN_TIMEOUT 15000
#define WIFI_ATE0_TIMEOUT   10000
#define WIFI_LOGIN_TIMEOUT  10000

#define CONFIG_STR_MAX  128
#define MAC_STR_MAX     20
#define IP_STR_MAX      20
#define GW_STR_MAX      20
#define MASK_STR_MAX    20
#define WIFI_DATA_MAX   1024
#define RECV_BUF_MAX  3072
#define LOCAL_BUF_MAX ESP_SOCKET_BUFSIZE

#if defined(USE_TINYMALLOC)
static uint8_t local_buf[LOCAL_BUF_MAX];
#endif

static bool at_debug = false;
static uint8_t wifi_data[WIFI_DATA_MAX];
static uint8_t recv_buf[RECV_BUF_MAX];

static socket_info_t socket_info[ESP_SOCKET_COUNT];
static driver_info_t driver_info[ESP_SOCKET_COUNT];
static _packet_t *_packets_top;
static _packet_t **_packets_end;

static bool _sock_already = false;
static int _heap_usage = 0;
static bool _cip_server = false;
static vector _accept_id;
static uint32_t _id_bits_connect = 0x00000000;
static uint32_t _id_bits_close = 0x0000001f;
static bool _ids[ESP_SOCKET_COUNT] = {false};

void esp_malloc_init(void) {
    _heap_usage = 0;
    #if defined(USE_TINYMALLOC)
    tinymalloc_init((void *)local_buf, (size_t)LOCAL_BUF_MAX);
    #endif
}

void *esp_malloc(size_t n) {
    #if defined(USE_TINYMALLOC)
    void *p = (void *)tinymalloc(n);
    #else
    void *p = (void *)m_malloc(n);
    #endif
    if (p != NULL) {
        _heap_usage += (int)n;
    }
    #if defined(DEBUG_ESP_MALLOC)
    MPY_DEBUG_PRINTF("esp_malloc(): size=%d p=%x\r\n", n, p);
    #endif
    return p;
}

void esp_free(void *p, size_t n) {
    #if defined(USE_TINYMALLOC)
    (void)n;
    tinyfree(p);
    #else
    m_free(p);
    _heap_usage -= (int)n;
    #endif
    #if defined(DEBUG_ESP_MALLOC)
    MPY_DEBUG_PRINTF("esp_free(): p=%x\r\n", p);
    #endif
}

void dump_socket_info() {
}
void dump_esp_socket(esp_socket_t *s) {
}

#if defined(DEBUG_ESP_SOCKET_DUMP)
void dump_socket_info() {
    for (int i = 0; i < ESP_SOCKET_COUNT; i++) {
        MPY_DEBUG_PRINTF("si[%d] open=%d sport=%d proto=%d\r\n", i,
            socket_info[i].open,
            socket_info[i].sport,
            socket_info[i].proto);
    }
}

/*
 *  int id;
    esp_protocol_t proto;
    esp_socket_address_t addr;
    bool connected;
    int keepalive; // TCP
    bool accept_id;
    bool tcp_server;
 */
void dump_esp_socket(esp_socket_t *s) {
    if (s) {
        MPY_DEBUG_PRINTF("esp socket id=%d connected=%d ka=%d accept_id=%d server=%d\r\n",
            s->id,
            s->connected,
            s->keepalive,
            s->accept_id,
            s->tcp_server);
    }
}
#endif

/*******************************************************************/
/* socket routines                                                 */
/*******************************************************************/

static bool is_socket_connected(uint32_t id) {
    return _id_bits_connect & (1 << id);
}

static bool is_socket_closed(uint32_t id) {
    return _id_bits_close & (1 << id);
}

static void set_socket_closed(uint32_t id) {
    _id_bits_close |= (1 << id);
    _id_bits_connect &= ~(1 << id);
}

static void set_socket_connected(uint32_t id) {
    _id_bits_connect |= (1 << id);
    _id_bits_close &= ~(1 << id);
}

/*******************************************************************/
/* string routines                                                 */
/*******************************************************************/

/*
 * hex char to digit
 */
static char hex_to_digit(char ch) {
    if (('0' <= ch) && (ch <= '9')) {
        return ch - '0';
    } else if (('A' <= ch) && (ch <= 'F')) {
        return ch - 'A' + 10;
    } else if (('a' <= ch) && (ch <= 'f')) {
        return ch - 'a' + 10;
    }
    return ch;
}

/*
 * hex chars to uint8_t
 */
static uint8_t hex_to_u8(const char *hex) {
    uint8_t val = 0;
    val += (uint8_t)hex_to_digit(hex[0]);
    val <<= 8;
    val += (uint8_t)hex_to_digit(hex[1]);
    return 0;
}

/*
 * ip string to uint8_t array
 * big endian
 * 192.168.0.1
 * -> ip[0] = 192
 * -> ip[1] = 168
 * -> ip[2] = 0
 * -> ip[3] = 1
 */
static void ip_str_to_array(const char *ip_str, uint8_t *ip) {
    char *p = (char *)ip_str;
    int i = 0;
    uint8_t num = 0;
    uint8_t c, n;
    while ((c = *p) != 0) {
        if ((c >= '0') && (c <= '9')) {
            num = num * 10 + (c - '0');
        }
        p++;
        n = *p;
        if (n < '0' || n > '9') {
            ip[i] = num;
            i++;
            num = 0;
        }
        if ((i > 3) || !((n >= '0' && n <= '9') || n == '.')) {
            break;
        }
    }
}

/*
 * count number of 'ch' in str
 */
static int count_ch(const char *str, char ch, char stop) {
    char c;
    int count = 0;
    while ((c = *str) != 0) {
        if (c == ch) {
            count++;
        }
        if (c == stop) {
            break;
        }
        str++;
    }
    return count;
}

/*
 * search characters in str
 * return the first position
 */
static char *strch2(const char *str, const char *ch) {
    char c1;
    char *s1 = (char *)str;
    while ((c1 = *s1) != 0) {
        char c2;
        char *s2 = (char *)ch;
        while ((c2 = *s2) != 0) {
            if (c1 == c2) {
                return s1;
            }
            s2++;
        }
        s1++;
    }
    return NULL;
}

/*
 * parse integer value between tokens
 */
static char *parse_int(const char *str, char *token, int *val) {
#define TMPBUF_MAX 10
    char tmpbuf[TMPBUF_MAX];
    char *start = (char *)str;
    char *end = strch2((const char *)start, token);
    if (!end) {
        return NULL;
    }
    int len = end - start;
    if (len < 0 || len > (TMPBUF_MAX - 1)) {
        return NULL;
    }
    strncpy(tmpbuf, (const char *)start, (size_t)len);
    tmpbuf[len] = 0;
    *val = atoi(tmpbuf);
    return end;
}

/*
 * parse a string between specified characters (ch1 and ch2)
 */
static char *parse_str_between_chars(const char *str, char ch1, char ch2, char *dst, int dst_size) {
    const char *start = str;
    char *pos1 = strchr(start, ch1);
    if (!pos1) {
        return NULL;
    }
    start = pos1 + 1;
    char *pos2 = strchr(start, ch2);
    if (!pos2) {
        return NULL;
    }
    int len = pos2 - pos1 - 1;
    int size = (len > (dst_size - 1))? dst_size - 1 : len;
    strncpy(dst, (const char *)start, size);
    dst[size] = 0;
    return pos2;
}

/*
 * parse a string between specified strings (str1 and str2)
 */
static char *parse_str_between_strings(const char *str, const char *str1, const char *str2, char *dst, int dst_size) {
    char *index1 = strstr((const char *)str, str1);
    char *index2 = strstr((const char *)str, str2);
    int len = dst_size;
    if (index1 != NULL && index2 != NULL) {
        len--;
        index1 += (int)strlen(str1);
        if (len > (index2 - index1)) {
            len = index2 - index1;
        }
        strncpy(dst, index1, len);
        dst[len] = 0;
        return index2;
    } else {
        return NULL;
    }
}

/*******************************************************************/
/* serial routines                                                 */
/*******************************************************************/

#if defined(DEBUG_ESP_DUMP_HEX)
static void dump_hex(uint8_t *buf, size_t size) {
    for (int i = 0; i < size; i++) {
        if ((i != 0) && ((i % 16) == 0)) {
            MPY_DEBUG_PRINTF("\r\n");
        }
        MPY_DEBUG_PRINTF("%02x ", buf[i]);
    }
    MPY_DEBUG_PRINTF("\r\n");
}
#endif
static void esp_serial_check_connect(const char *buf) {
    if (strstr((const char *)buf, "CONNECT") != NULL) {
        if (strstr((const char *)buf, "0,CONNECT") != NULL) {
            esp_socket_handler(true, 0);
        }
        if (strstr((const char *)buf, "1,CONNECT") != NULL) {
            esp_socket_handler(true, 1);
        }
        if (strstr((const char *)buf, "2,CONNECT") != NULL) {
            esp_socket_handler(true, 2);
        }
        if (strstr((const char *)buf, "3,CONNECT") != NULL) {
            esp_socket_handler(true, 3);
        }
        if (strstr((const char *)buf, "4,CONNECT") != NULL) {
            esp_socket_handler(true, 4);
        }
        // i = 0;
    }
    if (strstr((const char *)buf, "CLOSED") != NULL) {
        if (strstr((const char *)buf, "0,CLOSED") != NULL) {
            esp_socket_handler(false, 0);
        }
        if (strstr((const char *)buf, "1,CLOSED") != NULL) {
            esp_socket_handler(false, 1);
        }
        if (strstr((const char *)buf, "2,CLOSED") != NULL) {
            esp_socket_handler(false, 2);
        }
        if (strstr((const char *)buf, "3,CLOSED") != NULL) {
            esp_socket_handler(false, 3);
        }
        if (strstr((const char *)buf, "4,CLOSED") != NULL) {
            esp_socket_handler(false, 4);
        }
        // i = 0;
    }
}

static char *esp_serial_read_handler(const char *s1, const char *s2, const char *s3, uint32_t timeout) {
    uint8_t *buf = (uint8_t *)&wifi_data;
    uint32_t readed = 0;
    int i = 0;
    memset((void *)buf, 0, (size_t)WIFI_DATA_MAX);
    uint32_t start = (uint32_t)mp_hal_ticks_ms();
    while (true) {
        #if defined(RZA2M) || defined(RX63N) || defined(RX65N)
        while (esp_serial_available() > 0) {
            #if defined(DEBUG_ESP_RAW_DATA_RD)
            mpy_uart_debug_read(true);
            #endif
            uint8_t c = (uint8_t)esp_serial_read_ch();
            if (c == '\0') {
                continue;
            }
            buf[i] = c;
            i++;
            i = (i % WIFI_DATA_MAX);
            readed++;
        }
        #elif defined(PICO_BOARD)
        if (esp_serial_available()) {
            readed = esp_serial_read_str((uint8_t *)&buf[i], (size_t)(WIFI_DATA_MAX - i));
            #if defined(DEBUG_ESP_RAW_DATA_RD)
            if (readed != 0) {
                MPY_DEBUG_PRINTF("-- RD1(%d)\r\n", readed);
                MPY_DEBUG_TXSTRN((const char *)&buf[i], readed);
                MPY_DEBUG_PRINTF("\r\n", readed);
            }
            #endif
            i += readed;
            i = (i % WIFI_DATA_MAX);
        }
        #else
        #error ("MCU is not defined.")
        #endif
        buf[i] = 0;
        if (i >= 8) {
            esp_serial_check_connect((const char *)buf);
        }
        if ((s1 != NULL) && (strstr((const char *)buf, s1) != NULL)) {
            break;
        }
        if ((s2 != NULL) && (strstr((const char *)buf, s2) != NULL)) {
            break;
        }
        if ((s3 != NULL) && (strstr((const char *)buf, s3) != NULL)) {
            break;
        }
        if ((uint32_t)mp_hal_ticks_ms() - start > timeout) {
            #if defined(DEBUG_ESP_PRINT_TIMEOUT)
            MPY_DEBUG_PRINTF("-----\r\n");
            MPY_DEBUG_PRINTF("timeout readed:%d buf: %s\r\n", i, buf);
            MPY_DEBUG_PRINTF("-----\r\n");
            #endif
            break;
        }
    }
    #if defined(DEBUG_ESP_PRINT_DURATION)
    DEBUG_PRINTF("duration=%d\r\n", (uint32_t)mp_hal_ticks_ms() - start);
    #endif
    return (char *)buf;
}

static bool esp_serial_recv_find_tm(char *s1, char *s2, char *term, uint32_t tm) {
    char *buf;
    buf = esp_serial_read_handler((const char *)s1, (const char *)s2, (const char *)term, tm);
    if (strstr((const char *)buf, s1) != NULL) {
        return true;
    }
    if (strstr((const char *)buf, s2) != NULL) {
        return true;
    }
    #if defined(DEBUG_ESP_ERR)
    MPY_DEBUG_PRINTF("esp_serial_recv_find_tm(): ERR\r\n");
    #endif
    return false;
}

/*******************************************************************/
/* packet routines                                                 */
/* copied from mbed ESP driver                                 */
/*******************************************************************/

/*
 * packet_clear
 * if id == -1 then clear all of packets
 */
void packet_clear(int id) {
    #if defined(DEBUG_ESP_PACKET)
    MPY_DEBUG_PRINTF("packet_clear(): id=%d\r\n", id);
    #endif
    _packet_t **p = &_packets_top;
    while (*p) {
        if ((*p)->id == id || id == ESP_ALL_SOCKET_IDS) {
            _packet_t *q = *p;
            if (_packets_end == &((*p)->next)) {
                _packets_end = p; // Set last packet next field/_packets
            }
            *p = (*p)->next;
            if (q) {
                #if defined(DEBUG_ESP_PACKET)
                MPY_DEBUG_PRINTF("packet free(): id=%d, len=%d\r\n", q->id, q->alloc_len);
                #endif
                esp_free(q, q->alloc_len);
            }
        } else {
            // Point to last packet next field
            p = &((*p)->next);
        }
    }
    if (id == ESP_ALL_SOCKET_IDS) {
        for (int id = 0; id < ESP_SOCKET_COUNT; id++) {
            driver_info[id].tcp_data_avbl = 0;
        }
    } else {
        driver_info[id].tcp_data_avbl = 0;
    }
}

/*
 * packet_read
 */
#if defined(RZA2M) || defined(RX63N) || defined(RX65N)
int32_t packet_read(uint8_t *buf, int amount, uint32_t timeout) {
    uint32_t s;
    int i = 0;
    uint8_t c;
    #if defined(DEBUG_ESP_PACKET)
    MPY_DEBUG_PRINTF("packet_read(): buf=%x, amount=%d\r\n", buf, amount);
    #endif
    s = (uint32_t)mp_hal_ticks_ms();
    while ((uint32_t)mp_hal_ticks_ms() - s < timeout) {
        while (esp_serial_available() > 0) {
            #if defined(DEBUG_ESP_RAW_DATA_RD) || defined(DEBUG_ESP_DRIVER_PACKET_READ)
            mpy_uart_debug_read(true);
            #endif
            c = esp_serial_read_ch();
            if (buf) {
                buf[i] = c;
            }
            i++;
            if (i == amount) {
                goto packet_read_exit;
            }
        }
    }
packet_read_exit:
    #if defined(DEBUG_ESP_PACKET)
    MPY_DEBUG_PRINTF("packet_read(): duration=%d, req_amount=%d, act_amount=%d\r\n", (uint32_t)mp_hal_ticks_ms() - start, amount, i);
    #endif
    return i;
}
#elif defined(PICO_BOARD)
int32_t packet_read(uint8_t *buf, int amount, uint32_t timeout) {
    int i = 0;
    #if defined(DEBUG_ESP_PACKET)
    MPY_DEBUG_PRINTF("packet_read(): buf=%x, amount=%d\r\n", buf, amount);
    #endif
    uint32_t start = (uint32_t)mp_hal_ticks_ms();
    uint8_t c = 0;
    uint32_t readed = 0;
    while (true) {
        if (buf != 0) {
            if (esp_serial_available()) {
                readed = esp_serial_read_str(buf + i, amount - i);
                i += readed;
                if (i == amount) {
                    break;
                }
            }
        } else {
            if (esp_serial_available()) {
                c = esp_serial_read_ch();
                (void)c;
                i++;
                if (i == amount) {
                    break;
                }
            }
        }
        if ((uint32_t)mp_hal_ticks_ms() - start > timeout) {
            break;
        }
    }
    #if defined(DEBUG_ESP_RAW_DATA_RD) || defined(DEBUG_ESP_DRIVER_PACKET_READ)
    if (i != 0) {
        MPY_DEBUG_PRINTF("-- RD2(%d) buf:%x\r\n", i, buf);
        if (buf != 0) {
            // DEBUG_TXSTRN((const char *)&buf[0], i);
        }
        MPY_DEBUG_PRINTF("\r\n--\r\n");
    }
    #endif
    #if defined(DEBUG_ESP_PACKET)
    MPY_DEBUG_PRINTF("packet_read(): duration=%d, req_amount=%d, act_amount=%d\r\n", (uint32_t)mp_hal_ticks_ms() - start, amount, i);
    #endif
    return i;
}
#else
#endif

/*
 * packet_handle
 */
int32_t packet_handle(int id, int amount, uint32_t timeout) {
    #if defined(DEBUG_ESP_DRIVER) || defined(DEBUG_ESP_DRIVER_PACKET_HANDLE)
    MPY_DEBUG_PRINTF("packet_handle(): id=%d, amount=%d\r\n", id, amount);
    #endif
    int pdu_len;
    int count = 0;
    pdu_len = sizeof(_packet_t) + amount;
    if ((_heap_usage + pdu_len) > ESP_SOCKET_BUFSIZE) {
        #if defined(DEBUG_ESP_ERR)
        MPY_DEBUG_PRINTF("packet_handle(): heap limit. dropped\r\n");
        #endif
        count = packet_read((uint8_t *)0, amount, timeout);
        goto packet_handle_exit;
    }
    _packet_t *packet = (_packet_t *)esp_malloc(pdu_len);
    if (!packet) {
        #if defined(DEBUG_ESP_ERR)
        MPY_DEBUG_PRINTF("packet_handle(): alloc FAIL. dropped\r\n");
        #endif
        // just read, no store
        count = packet_read((uint8_t *)0, amount, timeout);
        goto packet_handle_exit;
    } else {
        #if defined(DEBUG_ESP_PACKET)
        MPY_DEBUG_PRINTF("packet_handle(): alloc OK. id=%d, amount=%d\r\n", id, amount);
        #endif
    }
    packet->id = id;
    packet->len = amount;
    packet->alloc_len = amount;
    packet->next = 0;
    count = packet_read((uint8_t *)(packet + 1), amount, timeout);
    if (count < amount) {
        if (packet) {
            #if defined(DEBUG_ESP_PACKET)
            MPY_DEBUG_PRINTF("packet free(): id=%d, amount=%d\r\n", id, amount);
            #endif
            esp_free(packet, packet->alloc_len);
        }
    } else {
        // append to packet list
        *_packets_end = packet;
        _packets_end = (_packet_t **)&packet->next;
    }
packet_handle_exit:
    #if defined(DEBUG_ESP_DRIVER) || defined(DEBUG_ESP_DRIVER_PACKET_HANDLE)
    MPY_DEBUG_PRINTF("packet_handle(): timeout=%d count=%d heap=%d\r\n", timeout, count, _heap_usage);
    #endif
    return count;
}

/*******************************************************************/
/* AT commands                                                     */
/*******************************************************************/

static char *res_ok = "OK\r\n";
static char *res_err = "ERROR\r\n";
static char *res_ready = "ready\r\n";

/*
 * AT command send
 */
static bool esp_AT_send_cmd(const char *fmt, ...) {
#define ESP_PRINTF_BUF_SIZE 1024
    char buf[ESP_PRINTF_BUF_SIZE];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf((char *)&buf, (size_t)ESP_PRINTF_BUF_SIZE, fmt, ap);
    va_end(ap);
    esp_serial_strln(buf);
    bool ret = esp_serial_recv_find_tm((char *)res_ok, (char *)res_ready, (char *)res_err, WIFI_TIMEOUT);
    #if defined(DEBUG_ESP_AT_OK)
    if (ret) {
        MPY_DEBUG_PRINTF("%s: OK\r\n", (char *)&buf);
    }
    #else
    if (ret & at_debug) {
        MPY_DEBUG_PRINTF("%s: OK\r\n", (char *)&buf);
    }
    #endif
    #if defined(DEBUG_ESP_AT_ERR)
    if (!ret) {
        MPY_DEBUG_PRINTF("%s: ERR\r\n", (char *)&buf);
    }
    #else
    if ((!ret) & at_debug) {
        MPY_DEBUG_PRINTF("%s: ERR\r\n", (char *)&buf);
    }
    #endif
    return ret;
}

/*
 * AT
 * Tests AT Startup
 */
bool esp_AT(void) {
    return esp_AT_send_cmd("AT");
}

/*
 * ATEx
 */
bool esp_ATE(int enable) {
    return esp_AT_send_cmd("ATE%d", enable);
}

/*
 * AT+RST
 * Restarts the module
 */
bool esp_AT_RST(void) {
    return esp_AT_send_cmd("AT+RST");
}

/*
 * AT+CWAUTOCONN
 */
bool esp_AT_CWAUTOCONN(int enable) {
    return esp_AT_send_cmd("AT+CWAUTOCONN=%d", enable);
}

/*
 * AT+CWQAP
 * Disconnect from an AP
 */
bool esp_AT_CWQAP(void) {
    return esp_AT_send_cmd("AT+CWQAP");
}

/*
 * AT+GMR
 * Checks Version Information
 */
bool esp_AT_GMR(char *at_ver, size_t at_len, char *sdk_ver, size_t sdk_len) {
    const char *start;
    const char *end;
    const char *buf = (const char *)&wifi_data;
    if (esp_AT_send_cmd("AT+GMR")) {
        start = buf;
        start = strstr(start, "AT version");
        if (!start) {
            return false;
        }
        start += 10;
        end = parse_str_between_chars((const char *)start, ':', '\r', at_ver, at_len);
        start = end + 1;
        start = strstr(start, "SDK version");
        if (!start) {
            return false;
        }
        start += 11;
        end = parse_str_between_chars((const char *)start, ':', '\r', sdk_ver, sdk_len);
        return true;
    } else {
        return false;
    }
}

/*
 * AT+CIPMUX
 * Enable or Disable Multiple Connections
 */
bool esp_set_AT_CIPMUX(int mode) {
    return esp_AT_send_cmd("AT+CIPMUX=%d", mode);
}

/*
 * AT+CWMODE
 * Sets the Wi-Fi Mode (Station/SoftAP/Station+SoftAP)
 * Query Command:
 */
bool esp_get_AT_CWMODE(uint8_t *mode) {
    if (!mode) {
        return false;
    }
    if (esp_AT_send_cmd("AT+CWMODE?")) {
        char *p = (char *)&wifi_data;
        if ((p = strstr((const char *)p, "+CWMODE:")) != NULL) {
            *mode = (uint8_t)atoi((const char *)&p[8]);
            return true;
        }
    }
    return false;
}

/*
 * AT+CWMODE
 * Sets the Wi-Fi Mode (Station/SoftAP/Station+SoftAP)
 * Set Command:
 */
bool esp_set_AT_CWMODE(int mode) {
    return esp_AT_send_cmd("AT+CWMODE=%d", mode);
}

/*
 * AT+CWJAP
 * Connects to an AP
 */
bool esp_set_AT_CWJAP(const char *ssid, const char *pwd) {
    return esp_AT_send_cmd("AT+CWJAP=\"%s\",\"%s\"", ssid, pwd);
}

/*
 * AT+CWJAP_CUR
 * Connects to an AP
 */
bool esp_get_AT_CWJAP_CUR(char *ssid, char *bssid, char *channel, char *rssi) {
    char config[CONFIG_STR_MAX];
    char *dst[4] = {ssid, bssid, channel, rssi};
    if (esp_AT_send_cmd("AT+CWJAP_CUR?")) {
        const char *buf = (const char *)&wifi_data;
        parse_str_between_strings((const char *)buf, "+CWJAP_CUR:", "\r\n\r\nOK", config, CONFIG_STR_MAX);
        char *start;
        char *end;
        int len;
        start = (char *)config;
        for (int i = 0; i < 4; i++) {
            if (i == 3) {
                end = strchr((const char *)start, (int)0);
            } else {
                end = strchr((const char *)start, (int)',');
            }
            if (end == NULL) {
                return false;
            }
            len = end - start;
            if (len <= 0) {
                return false;
            }
            if (dst[i]) {
                strncpy(dst[i], start, len);
                (dst[i])[len] = 0;
            }
            start = end + 1;
        }
        return true;
    }
    return false;
}

/*
 * AT+CWDHCP
 * Enables/Disables DHCP
 */
bool esp_set_AT_CWDHCP(uint8_t mode, bool enabled) {
    bool flag;
    if (enabled) {
        flag = esp_AT_send_cmd("AT+CWDHCP=1,%d", mode);
    } else {
        flag = esp_AT_send_cmd("AT+CWDHCP=0,%d", mode);
    }
    return flag;
}

/*
 * AT+CIPDOMAIN
 * Connects to an AP
 */
bool esp_AT_CIPDOMAIN(const char *domain, uint8_t *ip) {
#define IP_BUF_SIZE 20
    const char *buf = (const char *)&wifi_data;
    char ip_buf[IP_BUF_SIZE];
    if ((ip == NULL) || (domain == NULL)) {
        return false;
    }
    if (strlen(domain) >= 64) {
        #if defined(DEBUG_ESP_DRIVER)
        DEBUG_PRINTF("ESP ERR: domain name too long\r\n");
        #endif
        return false;
    }
    if (esp_AT_send_cmd("AT+CIPDOMAIN=\"%s\"", domain)) {
        char *start = strstr((const char *)buf, ":");
        if (start == NULL) {
            return false;
        }
        char *end = strstr((const char *)start, "\r");
        if (end == NULL) {
            return false;
        }
        int len = end - start - 1;
        if (len >= IP_BUF_SIZE) {
            return false;
        }
        strncpy(ip_buf, (const char *)(start + 1), len);
        ip_buf[len] = 0;
        #if defined(DEBUG_ESP_DRIVER)
        DEBUG_TXSTR(ip_buf);
        DEBUG_TXCH('\r');
        DEBUG_TXCH('\n');
        #endif
        ip_str_to_array((const char *)ip_buf, ip);
        return true;
    } else {
        return false;
    }
}

/*
 * AT+CIPSTAMAC_CUR
 * Query Command:
 */
bool esp_get_AT_CIPSTAMAC_CUR(uint8_t *mac) {
    const char *buf = (const char *)&wifi_data;
    char str_mac[MAC_STR_MAX];
    if (!mac) {
        return false;
    }
    if (esp_AT_send_cmd("AT+CIPSTAMAC_CUR?")) {
        char *find = parse_str_between_strings(buf, "+CIPSTAMAC_CUR:\"", "\r\n\r\nOK", str_mac, MAC_STR_MAX);
        if (find != NULL) {
            mac[0] = hex_to_u8((const char *)&str_mac[0]);
            mac[1] = hex_to_u8((const char *)&str_mac[3]);
            mac[2] = hex_to_u8((const char *)&str_mac[6]);
            mac[3] = hex_to_u8((const char *)&str_mac[9]);
            mac[4] = hex_to_u8((const char *)&str_mac[12]);
            mac[5] = hex_to_u8((const char *)&str_mac[15]);
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

/*
 * AT+CIPSTA
 * Query Command:
 */
bool esp_get_AT_CIPSTA(uint8_t *ip, uint8_t *gw, uint8_t *mask) {
    const char *buf = (const char *)&wifi_data;
    char str_config[CONFIG_STR_MAX];
    char str_ip[IP_STR_MAX];
    char str_gw[GW_STR_MAX];
    char str_mask[MASK_STR_MAX];
    if (!ip || !gw || !mask) {
        return false;
    }
    if (esp_AT_send_cmd("AT+CIPSTA?")) {
        char *find = parse_str_between_strings(buf, "+CIPSTA:", "\r\n\r\nOK", str_config, CONFIG_STR_MAX);
        if (find != NULL) {
            char *start;
            char *end;
            start = str_config;
            end = parse_str_between_chars((const char *)start, '"', '"', str_ip, IP_STR_MAX);
            start = end + 1;
            end = parse_str_between_chars((const char *)start, '"', '"', str_gw, GW_STR_MAX);
            start = end + 1;
            end = parse_str_between_chars((const char *)start, '"', '"', str_mask, MASK_STR_MAX);
            ip_str_to_array(str_ip, ip);
            ip_str_to_array(str_gw, gw);
            ip_str_to_array(str_mask, mask);
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

/*
 * AT+CIPSTA
 * Set Command:
 */
bool esp_set_AT_CIPSTA(const char *ip, const char *gw, const char *mask) {
    if (!ip || !gw || !mask) {
        return false;
    }
    return esp_AT_send_cmd("AT+CIPSTA=\"%s\",\"%s\",\"%s\"", ip, gw, mask);
}

/*
 * AT+CIPDNS_CUR
 * Query Command:
 */
bool esp_get_AT_CIPDNS_CUR(uint8_t *dns) {
    const char *buf = (const char *)&wifi_data;
    char str_config[CONFIG_STR_MAX];
    if (!dns) {
        return false;
    }
    if (esp_AT_send_cmd("AT+CIPDNS_CUR?")) {
        char *find = parse_str_between_strings(buf, "+CIPDNS_CUR:", "\r\n\r\nOK", str_config, CONFIG_STR_MAX);
        if (find != NULL) {
            ip_str_to_array(str_config, dns);
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

/*
 * AT+CIPDNS_CUR
 * Set Command:
 */
bool esp_set_AT_CIPDNS_CUR(const char *dns, bool flag) {
    bool ret;
    if (!dns) {
        return false;
    }
    if (flag) {
        ret = esp_AT_send_cmd("AT+CIPDNS_CUR=1,\"%s\"", dns);
    } else {
        ret = esp_AT_send_cmd("AT+CIPDNS_CUR=0,\"%s\"", dns);
    }
    return ret;
}

/*
 * AT+CIPSTO
 * Sets the TCP Server Timneout
 */
bool esp_set_AT_CIPSTO(int timeout) {
    return esp_AT_send_cmd("AT+CIPSTO=%d", timeout);
}

/*
 * AT+CIPDINFO
 * Shows the Remote IP and Port with +IPD
 */
bool esp_set_AT_CIPDINFO(int mode) {
    return esp_AT_send_cmd("AT+CIPDINFO=%d", mode);
}

/*
 * socket_handler
 */
void esp_socket_handler(bool connect, int id) {
    #if defined(DEBUG_ESP_DRIVER)
    DEBUG_PRINTF("\r\nesp_socket_handler(): connect=%d, id=%d\r\n", connect, id);
    #endif
    if (connect) {
        set_socket_connected(id);
        if (_cip_server) {
            int *mid = (int *)esp_malloc(sizeof(int));
            if (mid) {
                *mid = id;
                vector_add(&_accept_id, (void *)mid);
            }
        }
    } else {
        set_socket_closed(id);
        if (_cip_server) {
            for (size_t i = 0; i < vector_count(&_accept_id); i++) {
                int *mid = vector_get(&_accept_id, i);
                if (id == *mid) {
                    vector_delete(&_accept_id, i);
                }
            }
        }
    }
    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("esp_socket_handler(): _cip_server=%d\r\n", _cip_server);
    #endif
}

#if defined(RZA2M) || defined(RX63N) || defined(RX65N)
uint32_t esp_read(uint32_t timeout) {
    uint32_t s;
    uint8_t *data = (uint8_t *)recv_buf;
    int recvd = 0;
    char *start;
    char *end;
    int i;
    char ip_buf[IP_BUF_SIZE];

    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("esp_read(): timeout=%d\r\n", timeout);
    #endif
    memset((void *)data, 0, sizeof(recv_buf));
    i = 0;
    s = (uint32_t)mp_hal_ticks_ms();
    while ((uint32_t)mp_hal_ticks_ms() - s < timeout) {
        if (esp_serial_available() > 0) {
            uint8_t c = (uint8_t)esp_serial_read_ch();
            #if defined(DEBUG_ESP_DRIVER)
            if (debug_read) {
                if (isdisplayed(c)) {
                    DEBUG_TXCH(c);
                } else {
                    DEBUG_TXCH('*');
                }
            }
            #endif
            data[i++] = c;
            data[i] = 0;
            if (strstr((const char *)data, "CONNECT") != NULL) {
                recvd = 0;
                if (strstr((const char *)data, "0,CONNECT") != NULL) {
                    esp_socket_handler(true, 0);
                    goto esp_read_exit;
                }
                if (strstr((const char *)data, "1,CONNECT") != NULL) {
                    esp_socket_handler(true, 1);
                    goto esp_read_exit;
                }
                if (strstr((const char *)data, "2,CONNECT") != NULL) {
                    esp_socket_handler(true, 2);
                    goto esp_read_exit;
                }
                if (strstr((const char *)data, "3,CONNECT") != NULL) {
                    esp_socket_handler(true, 3);
                    goto esp_read_exit;
                }
                if (strstr((const char *)data, "4,CONNECT") != NULL) {
                    esp_socket_handler(true, 4);
                    goto esp_read_exit;
                }
            }
            if (strstr((const char *)data, "CLOSED") != NULL) {
                if (strstr((const char *)data, "0,CLOSED") != NULL) {
                    esp_socket_handler(false, 0);
                    goto esp_read_exit;
                }
                if (strstr((const char *)data, "1,CLOSED") != NULL) {
                    esp_socket_handler(false, 1);
                    goto esp_read_exit;
                    ;
                }
                if (strstr((const char *)data, "2,CLOSED") != NULL) {
                    esp_socket_handler(false, 2);
                    goto esp_read_exit;
                }
                if (strstr((const char *)data, "3,CLOSED") != NULL) {
                    esp_socket_handler(false, 3);
                    goto esp_read_exit;
                }
                if (strstr((const char *)data, "4,CLOSED") != NULL) {
                    esp_socket_handler(false, 4);
                    goto esp_read_exit;
                }
            }
            if (c == ':') {
                #if defined(DEBUG_ESP_DRIVER)
                // DEBUG_TXCH('\r');
                // DEBUG_TXCH('\n');
                #endif
                break;
            }
        }
        if (i == (RECV_BUF_MAX - 1)) {
            break;
        }
    }
    start = strstr((const char *)data, "+IPD,");
    if (start == NULL) {
        goto esp_read_exit;
    }
    int len = 0;
    int id = 0;
    int port = 0;
    start += 5;
    int count = count_ch((const char *)start, ',', ':');
    if (count == 0 || count == 2) {
        end = parse_int(start, ",:", &len);
        if (end == NULL) {
            goto esp_read_exit;
        }

    } else if (count == 1 || count == 3 || count == 4) {
        end = parse_int(start, ",:", &id);
        if (end == NULL) {
            goto esp_read_exit;
        }
        start = end + 1;
        end = parse_int(start, ",:", &len);
        if (end == NULL) {
            goto esp_read_exit;
        }
        start = end + 1;
        end = strstr((const char *)start, ",");
        if (end == NULL) {
            goto esp_read_exit;
        }
        int slen = end - start;
        strncpy(ip_buf, (const char *)start, slen);
        if (slen >= IP_BUF_SIZE) {
            goto esp_read_exit;
        }
        ip_buf[slen] = 0;
        start = end + 1;
        end = parse_int(start, ",:", &port);
        if (end == NULL) {
            goto esp_read_exit;
        }
        #if defined(DEBUG_ESP_DRIVER)
        MPY_DEBUG_PRINTF("id=%d, len=%d ip=%s port=%d\r\n", id, len, ip_buf, port);
        #endif
    } else {
        goto esp_read_exit;
    }
    #if defined(DEBUG_ESP_DRIVER)
    printf("len=%d\r\n", len);
    #endif
    recvd = packet_handle(id, len, ESP_PACKET_TIMEOUT);
esp_read_exit:
    #if defined(DEBUG_ESP_DRIVER)
    if (recvd != 0) {
        MPY_DEBUG_PRINTF("\r\nesp_read(): duration=%d, readed=%d, addr=%s, port=%d\r\n", (uint32_t)mp_hal_ticks_ms() - s, readed, ip_buf, port);
    }
    #endif
    return recvd;
}
#elif defined(PICO_BOARD)
uint32_t esp_read(uint32_t timeout) {
    uint8_t *data = (uint8_t *)recv_buf;
    uint32_t readed = 0;
    char *start;
    char *end;
    char ip_buf[IP_BUF_SIZE];
    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("esp_read(): timeout=%d\r\n", timeout);
    #endif
    memset((void *)data, 0, sizeof(recv_buf));
    int i = 0;
    uint32_t s = (uint32_t)mp_hal_ticks_ms();
    uint8_t c;
    while (true) {
        if (esp_serial_available()) {
            c = esp_serial_read_ch();
            data[i++] = c;
            if (c == ':') {
                data[i] = 0;
                break;
            }
            if (i == RECV_BUF_MAX) {
                break;
            }
        }
        if ((uint32_t)mp_hal_ticks_ms() - s > timeout) {
            break;
        }
    }
    #if defined(DEBUG_ESP_RAW_DATA_RD)
    MPY_DEBUG_PRINTF("-- RD3(%d)\r\n", i);
    MPY_DEBUG_TXSTRN((const char *)data, i);
    MPY_DEBUG_PRINTF("\r\n--\r\n");
    #endif
    if (i >= 8) {
        esp_serial_check_connect((const char *)data);
    }
    start = strstr((const char *)data, "+IPD,");
    if (start == NULL) {
        goto esp_read_exit;
    }
    int len = 0;
    int id = 0;
    int port = 0;
    start += 5;
    int count = count_ch((const char *)start, ',', ':');
    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("count=%d\r\n", count);
    #endif
    if (count == 0) {
        end = parse_int(start, ",:", &len);
        if (end == NULL) {
            goto esp_read_exit;
        }
        #if defined(DEBUG_ESP_DRIVER)
        MPY_DEBUG_PRINTF("len=%d\r\n", len);
        #endif
    } else if (count == 2) {
        end = parse_int(start, ",:", &len);
        if (end == NULL) {
            goto esp_read_exit;
        }
        start = end + 1;
        end = strstr((const char *)start, ",");
        if (end == NULL) {
            goto esp_read_exit;
        }
        int slen = end - start;
        strncpy(ip_buf, (const char *)start, slen);
        if (slen >= IP_BUF_SIZE) {
            goto esp_read_exit;
        }
        ip_buf[slen] = 0;
        start = end + 1;
        end = parse_int(start, ",:", &port);
        if (end == NULL) {
            goto esp_read_exit;
        }
        #if defined(DEBUG_ESP_DRIVER)
        MPY_DEBUG_PRINTF("len=%d ip=%s port=%d\r\n", len, ip_buf, port);
        #endif
    } else if (count == 1) {
        end = parse_int(start, ",:", &id);
        if (end == NULL) {
            goto esp_read_exit;
        }
        start = end + 1;
        end = parse_int(start, ",", &len);
        if (end == NULL) {
            goto esp_read_exit;
        }
        #if defined(DEBUG_ESP_DRIVER)
        MPY_DEBUG_PRINTF("id=%d len=%d\r\n", id, len);
        #endif
    } else if (count == 3) {
        end = parse_int(start, ",:", &id);
        if (end == NULL) {
            goto esp_read_exit;
        }
        start = end + 1;
        end = parse_int(start, ",", &len);
        if (end == NULL) {
            goto esp_read_exit;
        }
        start = end + 1;
        end = strstr((const char *)start, ",");
        if (end == NULL) {
            goto esp_read_exit;
        }
        int slen = end - start;
        strncpy(ip_buf, (const char *)start, slen);
        if (slen >= IP_BUF_SIZE) {
            goto esp_read_exit;
        }
        ip_buf[slen] = 0;
        start = end + 1;
        end = parse_int(start, ",:", &port);
        if (end == NULL) {
            goto esp_read_exit;
        }
        #if defined(DEBUG_ESP_DRIVER)
        MPY_DEBUG_PRINTF("id=%d, len=%d ip=%s port=%d\r\n", id, len, ip_buf, port);
        #endif
    } else {
        goto esp_read_exit;
    }
    readed = packet_handle(id, len, ESP_PACKET_TIMEOUT);
esp_read_exit:
    #if defined(DEBUG_ESP_DRIVER)
    if (readed != 0) {
        MPY_DEBUG_PRINTF("\r\nesp_read(): duration=%d, readed=%d, addr=%s, port=%d\r\n", (uint32_t)mp_hal_ticks_ms() - s, readed, ip_buf, port);
    }
    #endif
    return readed;
}
#else
#endif

bool esp_set_AT_CIPSEND(int id, const uint8_t *buffer, uint32_t len) {
    int count = 0;
    uint32_t timeout;
    if (esp_AT_send_cmd("AT+CIPSEND=%d,%d", id, len)) {
        esp_serial_write_bytes((uint8_t *)buffer, (size_t)len);
        timeout = ESP_READ_TIMEOUT;
        while (true) {
            count = esp_read(timeout);
            if (count <= 0) {
                break;
            }
            timeout = 200;
        }
        return true;
    }
    return false;
}

bool esp_set_AT_CIPCLOSE(int id) {
    return esp_AT_send_cmd("AT+CIPCLOSE=%d", id);
}

bool esp_set_AT_CIPSERVER(int port) {
    if (_cip_server) {
        return false;
    }
    if (esp_AT_send_cmd("AT+CIPSERVER=1,%d", port)) {
        _cip_server = true;
        return true;
    }
    return false;
}

bool esp_reset_AT_CIPSERVER(void) {
    if (esp_AT_send_cmd("AT+CIPSERVER=0")) {
        _cip_server = false;
        return true;
    }
    return false;
}

uint32_t esp_recv(uint8_t *data, uint32_t amount, uint32_t *data_len, uint32_t timeout, int id) {
    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("esp_recv(): id=%d, amount=%d\r\n", id, amount);
    #endif
    int len = 0;
    if (data == NULL || data_len == NULL) {
        return 0;
    }
    *data_len = 0;
    while (true) {
        for (_packet_t **p = &_packets_top; *p; p = &((*p)->next)) {
            if ((*p)->id == id) {
                _packet_t *q = *p;
                if (q->len <= amount) { // Return and remove full packet
                    memcpy((void *)data, (const void *)(q + 1), (size_t)(q->len));
                    if (_packets_end == &((*p)->next)) {
                        _packets_end = p;
                    }
                    *p = (*p)->next;
                    uint32_t len = q->len;
                    if (q) {
                        #if defined(DEBUG_ESP_PACKET)
                        MPY_DEBUG_PRINTF("packet free(): id=%d, len=%d\r\n", q->id, q->alloc_len);
                        #endif
                        esp_free((void *)q, q->alloc_len);
                    }
                    #if defined(DEBUG_ESP_DRIVER)
                    MPY_DEBUG_PRINTF("esp_recv(): len=%d\r\n", len);
                    #endif
                    *data_len = len;
                    return len;
                } else { // return only partial packet
                    memcpy((void *)data, (const void *)(q + 1), (size_t)amount);
                    q->len -= amount;
                    memmove((void *)(q + 1), (const void *)((char *)(q + 1) + amount), (size_t)q->len);
                    *data_len = amount;
                    #if defined(DEBUG_ESP_DRIVER)
                    MPY_DEBUG_PRINTF("esp_recv(): amount=%d\r\n", amount);
                    #endif
                    return amount;
                }
            }
        }
        len = esp_read(ESP_READ_TIMEOUT);
        if (len <= 0) {
            break;
        }
    }
    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("esp_recv(): act_recv=%d\r\n", len);
    #endif
    return len;
}

/*******************************************************************/
/* ESP driver                                                  */
/*******************************************************************/

/*
 * driver_init
 */
void esp_driver_init(uint32_t port, uint32_t tx, uint32_t rx, uint32_t baudrate, bool debug) {
    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("esp_driver_init():\r\n");
    #endif
    (void)tx;
    (void)rx;
    (void)debug;
    esp_malloc_init();
    _packets_top = (_packet_t *)NULL;
    _packets_end = &_packets_top;
    for (int i = 0; i < ESP_SOCKET_COUNT; i++) {
        socket_info[i].open = false;
        socket_info[i].sport = 0;
    }
    for (int i = 0; i < ESP_SOCKET_COUNT; i++) {
        driver_info[i].open = false;
        driver_info[i].proto = ESP_UDP;
        driver_info[i].tcp_data = NULL;
        driver_info[i].tcp_data_avbl = 0;
        driver_info[i].tcp_data_rcvd = 0;
    }
    memset(_ids, 0, sizeof(_ids));
    vector_init(&_accept_id);
    #if defined(DEBUG_ESP_RAW_DATA_WR)
    mpy_uart_debug_write(true);
    #endif
    esp_serial_begin(port, baudrate);
    esp_ATE(0);
    esp_set_AT_CIPMUX(1);
}

/*
 * driver_reset
 * Note: not working now - hang up communication between ESP
 */
bool esp_driver_reset(void) {
    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("esp_driver_reset():\r\n");
    #endif
    bool ret = true;
    // ret = esp_AT_RST();
    // need to wait for resetting
    mp_hal_delay_ms(3000);
    packet_clear(ESP_ALL_SOCKET_IDS);
    return ret;
}

/*
 * gethostbyname
 */
bool esp_gethostbyname(const char *name, uint8_t *ip) {
    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("esp_gethostbyname(): name=%s\r\n", name);
    #endif
    return esp_AT_CIPDOMAIN(name, ip);
}

/*
 * close
 */
bool esp_close(int id) {
    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("esp_close(): id=%d\r\n", id);
    #endif
    if (!is_socket_closed(id)) {
        esp_set_AT_CIPCLOSE(id);
    }
    driver_info[id].open = false;
    driver_info[id].proto = ESP_UDP;
    driver_info[id].tcp_data = NULL;
    driver_info[id].tcp_data_avbl = 0;
    driver_info[id].tcp_data_rcvd = 0;
    packet_clear(id);
    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("esp_close(): ret=1\r\n");
    #endif
    return true;
}

/*
 * open_tcp
 */
bool esp_open_tcp(int id, const char *addr, int port, int keepalive) {
    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("esp_open_tcp(): id=%d, addr=%s, port=%d, keepalive=%d\r\n", id, addr, port, keepalive);
    #endif
    char ip_str[16];
    bool flag = false;
    if (id >= ESP_SOCKET_COUNT || driver_info[id].open) {
        #if defined(DEBUG_ESP_DRIVER)
        MPY_DEBUG_PRINTF("esp_open_tcp(): ret=%d\r\n", flag);
        #endif
        return flag;
    }
    snprintf(ip_str, 16, "%u.%u.%u.%u", addr[0], addr[1], addr[2], addr[3]);
    for (int i = 0; i < 1; i++) {
        if (keepalive) {
            flag = esp_AT_send_cmd("AT+CIPSTART=%d,\"TCP\",\"%s\",%d,%d", id, ip_str, port, keepalive);
        } else {
            flag = esp_AT_send_cmd("AT+CIPSTART=%d,\"TCP\",\"%s\",%d", id, ip_str, port);
        }
        if (!flag) {
            if (_sock_already) {
                _sock_already = false;
                flag = esp_close(id);
                if (!flag) {
                    #if defined(DEBUG_ESP_DRIVER)
                    MPY_DEBUG_PRINTF("device refused to close socket\r\n");
                    #endif
                }
            }
            continue;
        } else {
            driver_info[id].open = true;
            driver_info[id].proto = ESP_TCP;
            #if defined(DEBUG_ESP_DRIVER)
            MPY_DEBUG_PRINTF("TCP socket id=%d opened\r\n", id);
            #endif
            flag = true;
            break;
        }
    }
    packet_clear(id);
    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("esp_open_tcp(): ret=%d\r\n", flag);
    #endif
    return flag;
}

/*
 * open_udp
 */
bool esp_open_udp(int id, const char *addr, int port, int local_port) {
    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("esp_open_udp(): id=%d, addr=%s, port=%d, local_port=%d\r\n", id, addr, port, local_port);
    #endif
    char ip_str[16];
    bool flag = false;
    if (id >= ESP_SOCKET_COUNT || driver_info[id].open) {
        return flag;
    }
    snprintf(ip_str, 16, "%u.%u.%u.%u", addr[0], addr[1], addr[2], addr[3]);
    for (int i = 0; i < 2; i++) {
        if (local_port) {
            flag = esp_AT_send_cmd("AT+CIPSTART=%d,\"TCP\",\"%s\",%d,%d", id, ip_str, port, local_port);
        } else {
            flag = esp_AT_send_cmd("AT+CIPSTART=%d,\"TCP\",\"%s\",%d", id, ip_str, port);
        }
        if (!flag) {
            if (_sock_already) {
                _sock_already = false;
                flag = esp_close(id);
                if (!flag) {
                    #if defined(DEBUG_ESP_DRIVER)
                    MPY_DEBUG_PRINTF("device refused to close socket\r\n");
                    #endif
                }
            }
            continue;
        } else {
            driver_info[id].open = true;
            driver_info[id].proto = ESP_UDP;
            #if defined(DEBUG_ESP_DRIVER)
            MPY_DEBUG_PRINTF("UDP socket %d opened", id);
            #endif
            flag = true;
            break;
        }
    }
    packet_clear(id);
    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("esp_open_udp(): ret=%d\r\n", flag);
    #endif
    return flag;
}

/*
 * send
 */
int32_t esp_send(int id, const void *data, uint32_t amount) {
    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("esp_send(): id=%d, data=%x amount=%d\r\n", id, data, amount);
    #endif
    int32_t ret = 0;
    int error_cnt = 0;
    while (error_cnt < 2) {
        if (!is_socket_connected(id) || is_socket_closed(id)) {
            ret = -1;
            break;
        }
        // +CIPSEND supports up to 2048 bytes at a time
        // Data stream can be truncated
        if (amount > CIPSEND_MAX && socket_info[id].proto == ESP_TCP) {
            amount = CIPSEND_MAX;
        } else if (amount > CIPSEND_MAX && socket_info[id].proto == ESP_UDP) {
            #if defined(DEBUG_ESP_DRIVER)
            MPY_DEBUG_PRINTF("UDP datagram maximum size is 2048");
            #endif
            return false;
        }
        bool flag = esp_set_AT_CIPSEND(id, (const uint8_t *)data, amount);
        if (flag) {
            ret = (int32_t)amount;
            break;
        } else {
            error_cnt++;
        }
    }
    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("esp_send(): ret=%d\r\n", ret);
    #endif
    return ret;
}

/*
 * recv_tcp
 */
int32_t esp_recv_tcp(int id, const void *data, uint32_t amount) {
    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("esp_recv_tcp(): id=%d, data=%x, amount=%d\r\n", id, data, amount);
    #endif
    uint32_t data_len;
    int32_t count = (int32_t)esp_recv((uint8_t *)data, amount, &data_len, WIFI_TIMEOUT, id);
    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("esp_recv_tcp(): ret=%d\r\n", count);
    #endif
    return count;
}

/*
 * recv_udp
 */
int32_t esp_recv_udp(int id, const void *data, uint32_t amount) {
    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("esp_recv_udp(): id=%d, data=%x, amount=%d\r\n", id, data, amount);
    #endif
    uint32_t data_len;
    int32_t count = (int32_t)esp_recv((uint8_t *)data, amount, &data_len, WIFI_TIMEOUT, id);
    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("esp_recv_udp(): ret=%d\r\n", count);
    #endif
    return count;
}

/*
 * check if accept_id vector is empty?
 */
bool is_accept_id_empty(void) {
    return vector_count(&_accept_id) == 0;
}

/*
 * accept
 */
bool esp_accept(int *p_id) {
    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("esp_accept(): pid=%08x\r\n", p_id);
    #endif
    bool ret = false;
    *p_id = -1;
    while (!ret) {
        if (!_cip_server) {
            break;
        }
        if (!is_accept_id_empty()) {
            ret = true;
        } else {
            // _parser.process_oob(); // Poll for inbound packets
            esp_read(ESP_ACCEPT_TIMEOUT);
            if (!is_accept_id_empty()) {
                ret = true;
            }
        }
        if (ret) {
            /* get the first item and delete from the _accept_id vector */
            int *p = (int *)(vector_get(&_accept_id, 0));
            if (p) {
                *p_id = *p;
                vector_delete(&_accept_id, 0);
            } else {
                *p_id = 0;
            }
        }
        if (!ret) {
            mp_hal_delay_ms(5);
        }
    }
    if (ret) {
        for (int i = 0; i < 50; i++) {
            if (is_socket_closed(*p_id)) {
                break;
            }
            mp_hal_delay_ms(10);
        }
    }
    #if defined(DEBUG_ESP_DRIVER)
    MPY_DEBUG_PRINTF("esp_accept(): ret=%d\r\n", ret);
    #endif
    return ret;
}

/*******************************************************************/
/* ESP socket interface                                        */
/*******************************************************************/

/*
 * socket_open
 */
int esp_socket_open(void **handle, esp_protocol_t proto) {
    #if defined(DEBUG_ESP_SOCKET)
    MPY_DEBUG_PRINTF("esp_socket_open(): **handle=%x\r\n", handle);
    #endif
    int err = 0;
    int id = -1;
    for (int i = 0; i < ESP_SOCKET_COUNT; i++) {
        if (!socket_info[i].open) {
            id = i;
            socket_info[i].open = true;
            break;
        }
    }
    if (id == -1) {
        err = -1;
    } else {
        struct esp_socket *socket = (struct esp_socket *)esp_malloc(sizeof(struct esp_socket));
        if (!socket) {
            err = -1;
        } else {
            socket->id = id;
            socket->proto = proto;
            socket->connected = false;
            socket->keepalive = 0;
            socket->accept_id = false;
            socket->tcp_server = false;
            *handle = socket;
            #if defined(DEBUG_ESP_SOCKET)
            #if defined(DEBUG_ESP_SOCKET_DUMP)
            dump_esp_socket(socket);
            #endif
            MPY_DEBUG_PRINTF("socket alloc id=%d\r\n", id);
            #endif
        }
    }
    #if defined(DEBUG_ESP_SOCKET_ERR)
    if (err < 0) {
        MPY_DEBUG_PRINTF("esp_socket_open(): ERR\r\n");
    }
    #endif
    return err;
}

/*
 * socket_close
 */
int esp_socket_close(void *handle) {
    #if defined(DEBUG_ESP_SOCKET)
    MPY_DEBUG_PRINTF("esp_socket_close(): *handle=%08x\r\n", handle);
    #endif
    struct esp_socket *socket = (struct esp_socket *)handle;
    int err = 0;
    if (!socket) {
        err = -1;
    } else {
        esp_close(socket->id);
        socket->connected = false;
        socket_info[socket->id].open = false;
        socket_info[socket->id].sport = 0;
        if (socket) {
            #if defined(DEBUG_ESP_SOCKET)
            MPY_DEBUG_PRINTF("socket free id=%d\r\n", socket->id);
            #endif
            esp_free(socket, sizeof(struct esp_socket));
        }
    }
    #if defined(DEBUG_ESP_SOCKET)
    MPY_DEBUG_PRINTF("esp_socket_close(): ret=%d\r\n", err);
    #endif
    #if defined(DEBUG_ESP_SOCKET_ERR)
    if (err < 0) {
        MPY_DEBUG_PRINTF("esp_socket_close(): ERR\r\n");
    }
    #endif
    return err;
}

/*
 * socket_bind
 */
int esp_socket_bind(void *handle, const esp_socket_address_t *address) {
    #if defined(DEBUG_ESP_SOCKET)
    MPY_DEBUG_PRINTF("esp_socket_bind(): handle=%08x, addr=%08x)r\n", handle, address);
    #endif
    int err = 0;
    struct esp_socket *socket = (struct esp_socket *)handle;
    if (!socket) {
        err = -1;
    } else {
        if (socket->proto == ESP_UDP) {
            for (int id = 0; id < ESP_SOCKET_COUNT; id++) {
                if (socket_info[id].sport == address->_port && id != socket->id) {
                    // Port already reserved by another socket
                    err = -1;
                    break;
                } else if (id == socket->id && socket->connected) {
                    err = -1;
                    break;
                }
            }
            if (err == 0) {
                socket_info[socket->id].sport = address->_port;
            }
        } else {
            socket->addr = *(esp_socket_address_t *)address;
        }
    }
    #if defined(DEBUG_ESP_SOCKET)
    #if defined(DEBUG_ESP_SOCKET_DUMP)
    dump_socket_info();
    #endif
    MPY_DEBUG_PRINTF("esp_socket_bind(): ret=%d\r\n", err);
    #endif
    #if defined(DEBUG_ESP_SOCKET_ERR)
    if (err < 0) {
        MPY_DEBUG_PRINTF("esp_socket_bind(): ERR\r\n");
    }
    #endif
    return err;
}

/*
 * socket_listen
 */
bool esp_socket_listen(void *handle, int backlog) {
    #if defined(DEBUG_ESP_SOCKET)
    MPY_DEBUG_PRINTF("esp_socket_listen(): handle=%08x,backlog=%d\r\n", handle, backlog);
    #endif
    int err = 0;
    struct esp_socket *socket = (struct esp_socket *)handle;
    if (!socket) {
        err = -1;
    } else {
        if (socket->proto == ESP_UDP) {
            err = -1;
        } else {
            if (!esp_set_AT_CIPSERVER(socket->addr._port)) {
                err = -1;
            }
        }
    }
    #if defined(DEBUG_ESP_SOCKET)
    #if defined(DEBUG_ESP_SOCKET_DUMP)
    if (!socket) {
        dump_esp_socket(socket);
    }
    #endif
    MPY_DEBUG_PRINTF("esp_socket_listen(): ret=%d\r\n", err);
    #endif
    #if defined(DEBUG_ESP_SOCKET_ERR)
    if (err < 0) {
        MPY_DEBUG_PRINTF("esp_socket_listen(): ERR\r\n");
    }
    #endif
    return err == 0;
}

/*
 * socket_connect
 */
bool esp_socket_connect(void *handle, const esp_socket_address_t *addr) {
    #if defined(DEBUG_ESP_SOCKET)
    MPY_DEBUG_PRINTF("esp_socket_connect(): handle=%08x\r\n", handle);
    #endif
    struct esp_socket *socket = (struct esp_socket *)handle;
    int err = 0;
    if (!socket) {
        err = -1;
    } else {
        if (socket->proto == ESP_UDP) {
            err = esp_open_udp(socket->id, (const char *)&addr->_addr, addr->_port, socket_info[socket->id].sport);
        } else {
            err = esp_open_tcp(socket->id, (const char *)&addr->_addr, addr->_port, 1);
        }
        socket->connected = (err == 0);
    }
    #if defined(DEBUG_ESP_SOCKET)
    #if defined(DEBUG_ESP_SOCKET_DUMP)
    if (!socket) {
        dump_esp_socket(socket);
    }
    #endif
    MPY_DEBUG_PRINTF("esp_socket_connect(): ret=%d\r\n", err);
    #endif
    #if defined(DEBUG_ESP_SOCKET_ERR)
    if (err < 0) {
        MPY_DEBUG_PRINTF("esp_socket_connect(): ERR\r\n");
    }
    #endif
    return err;
}

/*
 * socket_accept
 */
bool esp_socket_accept(void *server, void **socket, esp_socket_address_t *addr) {
    #if defined(DEBUG_ESP_SOCKET)
    MPY_DEBUG_PRINTF("esp_socket_accept(): server=%08x, socket=%08x, addr=%08x\r\n", server, socket, addr);
    #endif
    int id = -1;
    ;
    int err = -1;
    #if defined(DEBUG_ESP_PACKET)
    MPY_DEBUG_PRINTF("malloc(): len=%d\r\n", sizeof(struct esp_socket));
    #endif
    struct esp_socket *socket_new = (struct esp_socket *)esp_malloc(sizeof(struct esp_socket));
    if (!socket_new) {
        err = false;
    } else {
        if (!esp_accept(&id)) {
            #if defined(DEBUG_ESP_PACKET)
            MPY_DEBUG_PRINTF("free(): socket_new=%x\r\n", socket_new);
            #endif
            esp_free(socket_new, sizeof(struct esp_socket));
            err = -1;
        } else {
            socket_new->id = id;
            socket_new->proto = ESP_TCP;
            socket_new->connected = true;
            socket_new->accept_id = true;
            socket_new->tcp_server = false;
            *socket = socket_new;
        }
        // dummy values since ESP doesn't return ip address and port when connecting
        addr->_addr.bytes[0] = 0;
        addr->_addr.bytes[1] = 0;
        addr->_addr.bytes[2] = 0;
        addr->_addr.bytes[3] = 0;
        addr->_port = 0;
    }
    #if defined(DEBUG_ESP_SOCKET)
    #if defined(DEBUG_ESP_SOCKET_DUMP)
    dump_esp_socket(socket_new);
    #endif
    MPY_DEBUG_PRINTF("esp_socket_accept(): ret=%d, id=%d\r\n", err, id);
    #endif
    #if defined(DEBUG_ESP_SOCKET_ERR)
    if (err < 0) {
        MPY_DEBUG_PRINTF("esp_socket_accept(): ERR\r\n");
    }
    #endif
    return err;
}

/*
 * socket_send
 */
int esp_socket_send(void *handle, const void *data, unsigned size) {
    #if defined(DEBUG_ESP_SOCKET) || defined(DEBUG_ESP_SOCKET_SEND)
    MPY_DEBUG_PRINTF("esp_socket_send(): handle=%08x, size=%d\r\n", handle, size);
    #endif
    int err;
    struct esp_socket *socket = (struct esp_socket *)handle;
    if (!socket) {
        err = -1;
    } else {
        uint32_t start = (uint32_t)mp_hal_ticks_ms();
        do {
            err = (int)esp_send(socket->id, data, size);
        } while ((start - (uint32_t)mp_hal_ticks_ms() < 50) && (err != 0));
    }
    #if defined(DEBUG_ESP_SOCKET) || defined(DEBUG_ESP_SOCKET_SEND)
    MPY_DEBUG_PRINTF("esp_socket_send(): ret=%d\r\n", err);
    #endif
    #if defined(DEBUG_ESP_SOCKET_ERR)
    if (err < 0) {
        MPY_DEBUG_PRINTF("esp_socket_send(): ERR\r\n");
    }
    #endif
    return err;
}

/*
 * socket_recv
 */
int esp_socket_recv(void *handle, void *data, unsigned size) {
    #if defined(DEBUG_ESP_SOCKET) || defined(DEBUG_ESP_SOCKET_RECV)
    MPY_DEBUG_PRINTF("esp_socket_recv(): handle=%08x, size=%d\r\n", handle, size);
    #endif
    int err = 0;
    struct esp_socket *socket = (struct esp_socket *)handle;
    if (!socket) {
        err = -1;
    } else {
        if (socket->proto == ESP_TCP) {
            err = (int)esp_recv_tcp(socket->id, data, size);
            // if (recv <= 0) {
            //    socket->connected = false;
            // }
            // if (recv == 0) {
            //    recv = -1;
            // }
        } else {
            err = (int)esp_recv_udp(socket->id, data, size);
        }
        #if defined(DEBUG_ESP_SOCKET) || defined(DEBUG_ESP_SOCKET_RECV)
        MPY_DEBUG_PRINTF("esp_socket_recv(): ret=%d\r\n", err);
        #endif
    }
    #if defined(DEBUG_ESP_SOCKET_ERR)
    if (err < 0) {
        MPY_DEBUG_PRINTF("esp_socket_recv(): ERR\r\n");
    }
    #endif
    return err;
}

/*
 * socket_sendto
 */
int esp_socket_sendto(void *handle, const esp_socket_address_t *addr, const void *data, unsigned size) {
    #if defined(DEBUG_ESP_SOCKET)
    MPY_DEBUG_PRINTF("esp_socket_sendto(): *handle=%x, data=%x, size=%d\r\n", handle, data, size);
    #endif
    int err = 0;
    struct esp_socket *socket = (struct esp_socket *)handle;
    if (!socket) {
        err = -1;
    } else if ((strcmp((void *)&addr->_addr, "0.0.0.0") == 0) || !addr->_port) {
        err = -1;
    } else {
        // ToDo: implement compare address
        if (socket->connected && (&socket->addr != (esp_socket_address_t *)addr)) {
            if (!esp_close(socket->id)) {
                err = -1;
            }
            socket->connected = false;
        }
        if (!socket->connected) {
            err = esp_socket_connect(socket, addr);
            if (err >= 0) {
                socket->addr = (esp_socket_address_t)*addr;
            }
        }
        err = esp_socket_send(socket, data, size);
    }
    #if defined(DEBUG_ESP_SOCKET_ERR)
    if (err < 0) {
        MPY_DEBUG_PRINTF("esp_socket_sendto(): ERR\r\n");
    }
    #endif
    return err;
}

/*
 * socket_recvfrom
 */
int esp_socket_recvfrom(void *handle, esp_socket_address_t *addr, void *data, unsigned size) {
    #if defined(DEBUG_ESP_SOCKET)
    MPY_DEBUG_PRINTF("esp_socket_recvfrom(): *handle=%x, data=%x, size=%d\r\n", handle, data, size);
    #endif
    int err = 0;
    struct esp_socket *socket = (struct esp_socket *)handle;
    if (!socket) {
        err = -1;
    } else {
        err = esp_socket_recv(socket, data, size);
        if (err >= 0 && addr) {
            *addr = socket->addr;
        }
    }
    #if defined(DEBUG_ESP_SOCKET_ERR)
    if (err != 0) {
        MPY_DEBUG_PRINTF("esp_socket_recvfrom(): ERR\r\n");
    }
    #endif
    return err;
}

/*
 * setsockopt
 */
int esp_setsockopt(int handle, int level, int optname, const void *optval, unsigned optlen) {
    #if defined(DEBUG_ESP_SOCKET)
    MPY_DEBUG_PRINTF("esp_setsockopt() *handle=%08x, level=%d, optname=%d, optlen=%d)\r\n", handle, level, optname, optlen);
    #endif
    int err = 0;
    struct esp_socket *socket = (struct esp_socket *)handle;
    if (!optlen) {
        err = -1;
    } else if (!socket) {
        err = -1;
    } else {
        if (level == ESP_SOCKET && socket->proto == ESP_TCP) {
            switch (optname) {
                case ESP_KEEPALIVE: {
                    if (socket->connected) { // ESP limitation, keepalive needs to be given before connecting
                        break;
                    }
                    if (optlen == sizeof(int)) {
                        int secs = *(int *)optval;
                        if (secs >= 0 && secs <= 7200) {
                            socket->keepalive = secs;
                            err = 0;
                            break;
                        }
                    } else {
                        err = -1;
                    }
                }
                break;
                default:
                    break;
            }
        }
    }
    #if defined(DEBUG_ESP_SOCKET)
    MPY_DEBUG_PRINTF("esp_setsockopt(): ret=%d\r\n", err);
    #endif
    #if defined(DEBUG_ESP_SOCKET_ERR)
    if (err != 0) {
        MPY_DEBUG_PRINTF("esp_getsockopt(): ERR\r\n");
    }
    #endif
    return err;
}

/*
 * getsockopt
 */
int esp_getsockopt(int handle, int level, int optname, void *optval, unsigned *optlen) {
    #if defined(DEBUG_ESP_SOCKET)
    MPY_DEBUG_PRINTF("esp_getsockopt(): handle=%d, level=%d, optname=%d\r\n", handle, level, optname);
    #endif
    int err = 0;
    struct esp_socket *socket = (struct esp_socket *)handle;
    if (!optval || !optlen) {
        err = -1;
    } else if (!socket) {
        err = -1;
    } else {
        if (level == ESP_SOCKET && socket->proto == ESP_TCP) {
            switch (optname) {
                case ESP_KEEPALIVE: {
                    if (*optlen > sizeof(int)) {
                        *optlen = sizeof(int);
                    }
                    memcpy(optval, &(socket->keepalive), *optlen);
                    err = -1;
                }
            }
        }
    }
    #if defined(DEBUG_ESP_SOCKET_ERR)
    if (err != 0) {
        MPY_DEBUG_PRINTF("esp_getsockopt(): ERR\r\n");
    }
    #endif
    return err;
}
