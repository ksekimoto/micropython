/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 * Copyright (c) 2019 Kentaro Sekimoto
 *
 * Permission is hereby granted, tinyfree of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
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
/* ESP8266 Example
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
#include "shared/runtime/mpirq.h"
#include "systick.h"
#include "uart.h"

#include "common.h"
#include "tinymalloc.h"
#include "vector.h"
#include "esp8266_driver.h"

#if NEW_IMPL
pyb_uart_obj_t mp_esp_uart_obj;
mp_irq_obj_t mp_esp_irq_obj;
static uint8_t esp_rxbuf[768];
#else
#endif

#if defined(USE_DBG_PRINT)
#define DEBUG_ESP8266_ERR
#define DEBUG_ESP8266_AT_ERR
// #define DEBUG_ESP8266_SOCKET_ERR
#define DEBUG_ESP8266_AT
// #define DEBUG_ESP8266_RAW_DATA
// #define DEBUG_ESP8266_PACKET
// #define DEBUG_ESP8266_DRIVER
// #define DEBUG_ESP8266_DRIVER_PACKET_HANDLE
// #define DEBUG_ESP8266_DRIVER_PACKET_READ
// #define DEBUG_ESP8266_SOCKET
// #define DEBUG_ESP8266_SOCKET_RECV
// #define DEBUG_ESP8266_SOCKET_SEND
#endif

#if defined(RZA2M)
#define SCI_INIT_DEFAULT(CH, BAUD)  rz_sci_init(CH, 0x42, 0x41, BAUD, 8, 0, 1, 0)
#define SCI_RX_ANY          rz_sci_rx_any
#define SCI_RX_CH           rz_sci_rx_ch
#define SCI_TX_CH           rz_sci_tx_ch
#define SCI_TX_STR          rz_sci_tx_str

#else
#define SCI_INIT_DEFAULT(CH, BAUD)  rx_sci_init(CH, P23, P25, BAUD, 8, 0, 1, 0)
#define SCI_RX_ANY          rx_sci_rx_any
#define SCI_RX_CH           rx_sci_rx_ch
#define SCI_TX_CH(A,C)      rx_sci_tx_ch(A,C)
#define SCI_TX_STR          rx_sci_tx_str

#endif

char *itoa(int num, char *str, int base);
uint32_t esp8266_read(uint32_t timeout);
void esp8266_socket_handler(bool connect, int id);

#define ESP8266_READ_TIMEOUT    600
#define ESP8266_ACCEPT_TIMEOUT  600
#define ESP8266_PACKET_TIMEOUT  600
#define WIFI_SERIAL         MICROPY_HW_ESP8266_UART_CH
#define WIFI_BAUDRATE       115200
#define WIFI_WAIT_MSEC      5000
#define WIFI_TIMEOUT        10000
#define WIFI_DOMAIN_TIMEOUT 15000
#define WIFI_ATE0_TIMEOUT   10000
#define WIFI_LOGIN_TIMEOUT  10000

#define MAX_DIGITS 20
#define WIFI_DATA_MAX   1024
#define RECV_BUF_MAX    2048
#define LOCAL_BUF_MAX   10000

static uint8_t wifi_data[WIFI_DATA_MAX];
static uint8_t recv_buf[RECV_BUF_MAX];

static int esp8266_ch = WIFI_SERIAL;
static int esp8266_baud = WIFI_BAUDRATE;
int esp8266_errno = 0;

static socket_info_t socket_info[ESP8266_SOCKET_COUNT];
static driver_info_t driver_info[ESP8266_SOCKET_COUNT];
static _packet_t *_packets_top;
static _packet_t **_packets_end;
static char local_buf[LOCAL_BUF_MAX];

static bool _sock_already = false;
static int _heap_usage = 0;
static bool _cip_server = false;
static vector _accept_id;
static uint32_t _id_bits_connect = 0x00000000;
static uint32_t _id_bits_close = 0x0000001f;
static bool _ids[ESP8266_SOCKET_COUNT] = {false};
struct {
    void (*callback)(void *);
    void *data;
    int Notified;
} _cbs[ESP8266_SOCKET_COUNT];

/*******************************************************************/
/* debug routines                                                 */
/*******************************************************************/

#if defined(DEBUG_ESP8266_PACKET) || defined(DEBUG_ESP8266_RAW_DATA) || defined(DEBUG_ESP8266_SOCKET) || defined(DEBUG_ESP8266_DRIVER_PACKET_READ)
bool debug_write = true;
bool debug_read = true;
void debug_write_on() {
    debug_write = true;
}
void debug_write_off() {
    debug_write = false;
}
void debug_read_on() {
    debug_read = true;
}
void debug_read_off() {
    debug_read = false;
}

void dump_socket_info() {
    for (int i = 0; i < ESP8266_SOCKET_COUNT; i++) {
        printf("si[%d] open=%d sport=%d proto=%d\r\n", i,
            socket_info[i].open,
            socket_info[i].sport,
            socket_info[i].proto);
    }
}

/*
 *  int id;
    esp8266_protocol_t proto;
    esp8266_socket_address_t addr;
    bool connected;
    int keepalive; // TCP
    bool accept_id;
    bool tcp_server;
 */
void dump_esp8266_socket(esp8266_socket_t *s) {
    if (s) {
        printf("esp8266 socket id=%d connected=%d ka=%d accept_id=%d server=%d\r\n",
            s->id,
            s->connected,
            s->keepalive,
            s->accept_id,
            s->tcp_server);
    }
}
#else
void debug_write_on() {
}
void debug_write_off() {
}
void debug_read_on() {
}
void debug_read_off() {
}
void dump_socket_info() {
}
void dump_esp8266_socket(esp8266_socket_t *s) {
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

#define CONFIG_STR_MAX  128
#define MAC_STR_MAX     20
#define IP_STR_MAX      20
#define GW_STR_MAX      20
#define MASK_STR_MAX    20

/*
 * check if a charater can be displayed or not
 */
bool isdisplayed(uint8_t c) {
    if ((c == '\r') || (c == '\n') || (0x20 <= c && c <= 0x7e)) {
        return true;
    } else {
        return false;
    }
}

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
// #if defined(DEBUG_ESP8266_RAW_DATA)
//           char s[MAX_DIGITS];
//           itoa((int)num, (char *)s, 10);
//           DEBUG_TXSTR(s);
//           DEBUG_TXCH('\r');
//           DEBUG_TXCH('\n');
// #endif
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
static int count_ch(const char *str, char ch) {
    char c;
    int count = 0;
    while ((c = *str) != 0) {
        if (c == ch) {
            count++;
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
    char *end = strch2((const char *)start, ",:");
    if (!end) {
        return NULL;
    }
    int len = end - start;
    if (len < 0 || len > (TMPBUF_MAX - 1)) {
        return NULL;
    }
    strncpy(tmpbuf, (const char *)start, (size_t)len);
    tmpbuf[len] = 0;
// #if defined(DEBUG_ESP8266_RAW_DATA)
//    DEBUG_TXSTR(tmpbuf);
//    DEBUG_TXCH('\r');
//    DEBUG_TXCH('\n');
// #endif
    *val = atoi(tmpbuf);
    return end;
}

/*
 * parse a string between specified characters (ch1 and ch2)
 */
static char *parse_str_between(const char *str, char ch1, char ch2, char *dst, int dst_size) {
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
// #if defined(DEBUG_ESP8266_RAW_DATA)
//    DEBUG_TXSTR(dst);
//    DEBUG_TXCH('\r');
//    DEBUG_TXCH('\n');
// #endif
    return pos2;
}

/*******************************************************************/
/* serial routines                                                 */
/*******************************************************************/

static void esp8266_serial_clear_buf(void) {
    memset((void *)wifi_data, 0, (size_t)WIFI_DATA_MAX);
}

static void esp8266_serial_begin(int uart_id, int baud) {
    #if NEW_IMPL
    mp_esp_uart_obj.base.type = &pyb_uart_type;
    mp_esp_uart_obj.uart_id = uart_id;
    mp_esp_uart_obj.is_static = true;
    // We don't want to block indefinitely, but expect flow control is doing its job.
    mp_esp_uart_obj.timeout = 200;
    mp_esp_uart_obj.timeout_char = 200;
    MP_STATE_PORT(pyb_uart_obj_all)[mp_esp_uart_obj.uart_id - 1] = &mp_esp_uart_obj;
    uart_init(&mp_esp_uart_obj, baud, 8, 0, 1, 0);
    uart_set_rxbuf(&mp_esp_uart_obj, sizeof(esp_rxbuf), esp_rxbuf);
    #else
    SCI_INIT_DEFAULT(esp8266_ch, esp8266_baud);
    #endif
}

static int esp8266_serial_available(void) {
    return SCI_RX_ANY(esp8266_ch);
}

static int esp8266_serial_read(void) {
    return (int)SCI_RX_CH(esp8266_ch);
}

static void esp8266_serial_write_byte(uint8_t c) {
    SCI_TX_CH(esp8266_ch, (int)c);
    #if defined(DEBUG_ESP8266_RAW_DATA)
    if (debug_write) {
        if (isdisplayed(c)) {
            DEBUG_TXCH(c);
        } else {
            DEBUG_TXCH('*');
        }
    }
    #endif
}

static void esp8266_serial_print(const char *s) {
    SCI_TX_STR(esp8266_ch, (uint8_t *)s);
    #if defined(DEBUG_ESP8266_RAW_DATA)
    DEBUG_TXSTR(s);
    #endif
}

static void esp8266_serial_printi(int i) {
    char s[MAX_DIGITS];
    itoa(i, (char *)s, 10);
    SCI_TX_STR(esp8266_ch, (uint8_t *)s);
    #if defined(DEBUG_ESP8266_RAW_DATA)
    DEBUG_TXSTR(s);
    #endif
}

static void esp8266_serial_println(const char *s) {
    esp8266_serial_print(s);
    SCI_TX_CH(esp8266_ch, (int)'\r');
    SCI_TX_CH(esp8266_ch, (int)'\n');
    #if defined(DEBUG_ESP8266_RAW_DATA)
    DEBUG_TXCH('\r');
    DEBUG_TXCH('\n');
    #endif
}

static void esp8266_serial_printiln(int i) {
    esp8266_serial_printi(i);
    SCI_TX_CH(esp8266_ch, (int)'\r');
    SCI_TX_CH(esp8266_ch, (int)'\n');
    #if defined(DEBUG_ESP8266_RAW_DATA)
    DEBUG_TXCH('\r');
    DEBUG_TXCH('\n');
    #endif
}

#if esp8266_serial_printf
#include "vsnprintf.h"
static int esp8266_serial_printf(const void *format, ...) {
#define ESP8266_PRINTF_BUF_SIZE 1024
    char buf[ESP8266_PRINTF_BUF_SIZE];
    int len;
    va_list arg_ptr;
    va_start(arg_ptr, format);
    len = vxsnprintf(buf, (size_t)(ESP8266_PRINTF_BUF_SIZE - 1), format, arg_ptr);
    #if defined(DEBUG_ESP8266_RAW_DATA)
    DEBUG_TXSTR((uint8_t *)buf);
    #endif
    va_end(arg_ptr);
    return len;
}
#endif

static char *esp8266_serial_read_handler(const char *s1, const char *s2, uint32_t timeout) {
    uint8_t *buf = (uint8_t *)&wifi_data;
    uint8_t c;
    int i = 0;
    uint32_t start = (uint32_t)mtick();
    while ((i < (WIFI_DATA_MAX - 1)) && ((uint32_t)mtick() - start < timeout)) {
        while (esp8266_serial_available() > 0) {
            c = (uint8_t)esp8266_serial_read();
            #if defined(DEBUG_ESP8266_RAW_DATA)
            DEBUG_TXCH(c);
            #endif
            if (c == '\0') {
                continue;
            }
            buf[i] = c;
            i++;
        }
        buf[i + 1] = 0;
        if (strstr((const char *)buf, "CONNECT") != NULL) {
            if (strstr((const char *)buf, "0,CONNECT") != NULL) {
                esp8266_socket_handler(true, 0);
            }
            if (strstr((const char *)buf, "1,CONNECT") != NULL) {
                esp8266_socket_handler(true, 1);
            }
            if (strstr((const char *)buf, "2,CONNECT") != NULL) {
                esp8266_socket_handler(true, 2);
            }
            if (strstr((const char *)buf, "3,CONNECT") != NULL) {
                esp8266_socket_handler(true, 3);
            }
            if (strstr((const char *)buf, "4,CONNECT") != NULL) {
                esp8266_socket_handler(true, 4);
            }
            i = 0;
        }
        if (strstr((const char *)buf, "CLOSED") != NULL) {
            if (strstr((const char *)buf, "0,CLOSED") != NULL) {
                esp8266_socket_handler(false, 0);
            }
            if (strstr((const char *)buf, "1,CLOSED") != NULL) {
                esp8266_socket_handler(false, 1);
            }
            if (strstr((const char *)buf, "2,CLOSED") != NULL) {
                esp8266_socket_handler(false, 2);
            }
            if (strstr((const char *)buf, "3,CLOSED") != NULL) {
                esp8266_socket_handler(false, 3);
            }
            if (strstr((const char *)buf, "4,CLOSED") != NULL) {
                esp8266_socket_handler(false, 4);
            }
            i = 0;
        }
        if ((s1 != NULL) && (strstr((const char *)buf, s1) != NULL)) {
            break;
        }
        if ((s2 != NULL) && (strstr((const char *)buf, s2) != NULL)) {
            break;
        }
    }
    return (char *)buf;
}

static void esp8266_serial_empty(void) {
    #if defined(DEBUG_ESP8266_DRIVER)
    // printf("esp8266_serial_empty(s)\r\n");
    #endif
    esp8266_serial_read_handler((const char *)NULL, (const char *)NULL, 10);
    #if defined(DEBUG_ESP8266_DRIVER)
    // printf("esp8266_serial_empty(e)\r\n");
    #endif
}

static bool esp8266_serial_recv_find_tm(char *s, uint32_t tm) {
    char *buf;
    buf = esp8266_serial_read_handler(s, NULL, tm);
    if (strstr((const char *)buf, s) != NULL) {
        return true;
    }
    #if defined(DEBUG_ESP8266_ERR)
    printf("esp8266_serial_recv_find_tm: err\r\n");
    #endif
    return false;
}

static bool esp8266_serial_recv_find(char *s) {
    return esp8266_serial_recv_find_tm(s, WIFI_TIMEOUT);
}

static bool esp8266_serial_recv_find_filter(char *s, char *begin, char *end, char *val, size_t len) {
    char *buf;
    if (len <= 0) {
        return false;
    }
    buf = esp8266_serial_read_handler(s, NULL, WIFI_TIMEOUT);
    if (strstr((const char *)buf, s) != NULL) {
        char *index1 = strstr((const char *)buf, begin);
        char *index2 = strstr((const char *)buf, end);
        if (index1 != NULL && index2 != NULL) {
            len--;
            index1 += (int)strlen(begin);
            if (len > (index2 - index1)) {
                len = index2 - index1;
            }
            strncpy(val, index1, len);
            val[len] = 0;
            return true;
        }
    }
    *val = '\0';
    #if defined(DEBUG_ESP8266_ERR)
    printf("esp8266_serial_recv_find_filter: err\r\n");
    #endif
    return false;
}


/*******************************************************************/
/* packet routines                                                 */
/* copied from mbed ESP8266 driver                                 */
/*******************************************************************/

/*
 * packet_clear
 * if id == -1 then clear all of packets
 */
void packet_clear(int id) {
    #if defined(DEBUG_ESP8266_PACKET)
    printf("packet_clear(%d)\r\n", id);
    #endif
    _packet_t **p = &_packets_top;
    while (*p) {
        if ((*p)->id == id || id == ESP8266_ALL_SOCKET_IDS) {
            _packet_t *q = *p;
            int pdu_len = sizeof(_packet_t) + q->alloc_len;
            if (_packets_end == &((*p)->next)) {
                _packets_end = p; // Set last packet next field/_packets
            }
            *p = (*p)->next;
            if (q) {
                #if defined(DEBUG_ESP8266_PACKET)
                printf("packet_tinyfree: id=%d, len=%d\r\n", q->id, q->alloc_len);
                #endif
                tinyfree(q);
            }
            _heap_usage -= pdu_len;
        } else {
            // Point to last packet next field
            p = &((*p)->next);
        }
    }
    if (id == ESP8266_ALL_SOCKET_IDS) {
        for (int id = 0; id < ESP8266_SOCKET_COUNT; id++) {
            driver_info[id].tcp_data_avbl = 0;
        }
    } else {
        driver_info[id].tcp_data_avbl = 0;
    }
}

/*
 * packet_read
 */
int32_t packet_read(uint8_t *buf, int amount, uint32_t timeout) {
    uint32_t s;
    int i = 0;
    uint8_t c;
    debug_read_on();
    #if defined(DEBUG_ESP8266_PACKET)
    printf("packet_read()");
    #endif
    s = (uint32_t)mtick();
    while ((uint32_t)mtick() - s < timeout) {
        while (esp8266_serial_available() > 0) {
            c = esp8266_serial_read();
            #if defined(DEBUG_ESP8266_RAW_DATA) || defined(DEBUG_ESP8266_DRIVER_PACKET_READ)
            if (debug_read) {
                if (isdisplayed(c)) {
                    DEBUG_TXCH(c);
                } else {
                    DEBUG_TXCH('*');
                }
            }
            #endif
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
    debug_read_on();
    #if defined(DEBUG_ESP8266_PACKET)
    printf("\r\npacket_read() time=%d, req=%d, act=%d\r\n", (uint32_t)mtick() - s, amount, i);
    #endif
    return i;
}

/*
 * packet_handle
 */
int32_t packet_handle(int id, int amount, uint32_t timeout) {
    #if defined(DEBUG_ESP8266_DRIVER) || defined(DEBUG_ESP8266_DRIVER_PACKET_HANDLE)
    printf("packet_handle(id=%d, amount=%d)\r\n", id, amount);
    #endif
    int pdu_len;
    int count = 0;
    pdu_len = sizeof(_packet_t) + amount;
    if ((_heap_usage + pdu_len) > ESP8266_SOCKET_BUFSIZE) {
        #if defined(DEBUG_ESP8266_ERR)
        printf("packet_handle: heap limit. dropped\r\n");
        #endif
        count = packet_read((uint8_t *)0, amount, timeout);
        goto packet_handle_exit;
    }
    _packet_t *packet = (_packet_t *)tinymalloc(pdu_len);
    if (!packet) {
        #if defined(DEBUG_ESP8266_ERR)
        printf("packet_handler: alloc fail. dropped\r\n");
        #endif
        // just read, no store
        count = packet_read((uint8_t *)0, amount, timeout);
        goto packet_handle_exit;
    } else {
        #if defined(DEBUG_ESP8266_PACKET)
        printf("packet alloc: id=%d, len=%d\r\n", id, amount);
        #endif
    }
    _heap_usage += pdu_len;
    packet->id = id;
    packet->len = amount;
    packet->alloc_len = amount;
    packet->next = 0;
    count = packet_read((uint8_t *)(packet + 1), amount, timeout);
    if (count < amount) {
        if (packet) {
            #if defined(DEBUG_ESP8266_PACKET)
            printf("packet tinyfree: id=%d, len=%d\r\n", id, amount);
            #endif
            tinyfree(packet);
        }
        _heap_usage -= pdu_len;
    } else {
        // append to packet list
        *_packets_end = packet;
        _packets_end = (_packet_t **)&packet->next;
    }
packet_handle_exit:
    #if defined(DEBUG_ESP8266_DRIVER) || defined(DEBUG_ESP8266_DRIVER_PACKET_HANDLE)
    printf("packet_handle(timeout=%ld) count=%d heap=%d\r\n", timeout, count, _heap_usage);
    #endif
    return count;
}

/*******************************************************************/
/* AT commands                                                     */
/*******************************************************************/

static void esp8266_serial_prepare_AT(void) {
    #if defined(DEBUG_ESP8266_AT)
    printf("prepare_AT(s)\r\n");
    #endif
    esp8266_serial_empty();
    esp8266_serial_clear_buf();
    #if defined(DEBUG_ESP8266_AT)
    printf("prepare_AT(e)\r\n");
    #endif
}

/*
 * ATE
 * AT Commands Echoing
 * ATE0: Switches echo off
 * ATE1: Switches echo on
 */
bool esp8266_ATE0(void) {
    #if defined(DEBUG_ESP8266_AT)
    printf("ATE0\r\n");
    #endif
    esp8266_serial_prepare_AT();
    esp8266_serial_println("ATE0");
    bool ret = esp8266_serial_recv_find_tm("ready\r\n", WIFI_ATE0_TIMEOUT);
    #if defined(DEBUG_ESP8266_AT_ERR)
    if (!ret) {
        printf("ATE0: err\r\n");
    }
    #endif
    return ret;
}

/*
 * AT
 * Tests AT Startup
 */
bool esp8266_AT(void) {
    #if defined(DEBUG_ESP8266_AT)
    printf("AT\r\n");
    #endif
    esp8266_serial_prepare_AT();
    esp8266_serial_println("AT");
    bool ret = esp8266_serial_recv_find("OK\r\n");
    #if defined(DEBUG_ESP8266_AT_ERR)
    if (!ret) {
        printf("AT: err\r\n");
    }
    #endif
    return ret;
}

/*
 * AT+RST
 */
bool esp8266_AT_RST(void) {
    esp8266_serial_prepare_AT();
    esp8266_serial_println("AT+RST");
    bool ret = esp8266_serial_recv_find("OK\r\n");
    #if defined(DEBUG_ESP8266_AT_ERR)
    if (!ret) {
        printf("AT+RST: err\r\n");
    }
    #endif
    return ret;
}

/*
 * AT+CWAUTOCONN
 */
bool esp8266_AT_CWAUTOCONN_0(void) {
    esp8266_serial_prepare_AT();
    esp8266_serial_println("AT+CWAUTOCONN=0");
    bool ret = esp8266_serial_recv_find("OK\r\n");
    #if defined(DEBUG_ESP8266_AT_ERR)
    if (!ret) {
        printf("AT+CWAUTOCONN\r\n");
    }
    #endif
    return ret;
}

/*
 * AT+CWQAP
 * Disconnect from an AP
 */
bool esp8266_AT_CWQAP(void) {
    esp8266_serial_prepare_AT();
    esp8266_serial_println("AT+CWQAP");
    bool ret = esp8266_serial_recv_find("OK\r\n");
    #if defined(DEBUG_ESP8266_AT_ERR)
    if (!ret) {
        printf("AT+CWQAP: err\r\n");
    }
    #endif
    return ret;
}

/*
 * AT+GMR
 * Checks Version Information
 */
bool esp8266_AT_GMR(char *at_ver, size_t at_len, char *sdk_ver, size_t sdk_len) {
    const char *buf;
    const char *start;
    const char *end;
    esp8266_serial_prepare_AT();
    esp8266_serial_println("AT+GMR");
    buf = esp8266_serial_read_handler("OK\r\n", NULL, WIFI_TIMEOUT);
    if (buf) {
        start = buf;
        start = strstr(start, "AT version");
        if (!start) {
            return false;
        }
        start += 10;
        end = parse_str_between((const char *)start, ':', '\r', at_ver, at_len);
        start = end + 1;
        start = strstr(start, "SDK version");
        if (!start) {
            return false;
        }
        start += 11;
        end = parse_str_between((const char *)start, ':', '\r', sdk_ver, sdk_len);
        return true;
    } else {
        #if defined(DEBUG_ESP8266_AT_ERR)
        printf("AT+GMR: err\r\n");
        #endif
        return false;
    }
}

/*
 * AT+CIPMUX
 * Enable or Disable Multiple Connections
 */
bool esp8266_set_AT_CIPMUX(uint8_t mode) {
    esp8266_serial_prepare_AT();
    esp8266_serial_print("AT+CIPMUX=");
    esp8266_serial_printiln(mode);
    bool ret = esp8266_serial_read_handler("OK\r\n", NULL, WIFI_TIMEOUT);
    #if defined(DEBUG_ESP8266_AT_ERR)
    if (!ret) {
        printf("AT+CIPMUX: err\r\n");
    }
    #endif
    return ret;
}

/*
 * AT+CIPRECVMODE
 * passive mode
 */
bool esp8266_set_AT_CIPRECVMODE(uint8_t mode) {
    esp8266_serial_prepare_AT();
    esp8266_serial_print("AT+CIPRECVMODE=");
    esp8266_serial_printiln(mode);
    bool ret = esp8266_serial_read_handler("OK\r\n", NULL, WIFI_TIMEOUT);
    #if defined(DEBUG_ESP8266_AT_ERR)
    if (!ret) {
        printf("AT+CIPRECVMODE: err\r\n");
    }
    #endif
    return ret;
}

/*
 * AT+CWMODE
 * Sets the Wi-Fi Mode (Station/SoftAP/Station+SoftAP)
 * Query Command:
 */
bool esp8266_get_AT_CWMODE(uint8_t *mode) {
#define MODE_MAX    4
    char str_mode[MODE_MAX];
    bool ret;
    if (!mode) {
        return false;
    }
    esp8266_serial_prepare_AT();
    esp8266_serial_println("AT+CWMODE?");
    ret = esp8266_serial_recv_find_filter("OK", "+CWMODE:", "\r\n\r\nOK", str_mode, MODE_MAX);
    if (ret) {
        *mode = (uint8_t)atoi((const char *)str_mode);
        return true;
    } else {
        #if defined(DEBUG_ESP8266_AT_ERR)
        printf("AT+CWMODE: err\r\n");
        #endif
        return false;
    }
}

/*
 * AT+CWMODE
 * Sets the Wi-Fi Mode (Station/SoftAP/Station+SoftAP)
 * Set Command:
 */
bool esp8266_set_AT_CWMODE(uint8_t mode) {
    char *buf;
    esp8266_serial_prepare_AT();
    esp8266_serial_print("AT+CWMODE=");
    esp8266_serial_printiln(mode);
    buf = esp8266_serial_read_handler("OK\r\n", "no change\r\n", WIFI_TIMEOUT);
    if (strstr((const char *)buf, "OK") != NULL || strstr((const char *)buf, "no change") != NULL) {
        return true;
    }
    #if defined(DEBUG_ESP8266_AT_ERR)
    printf("AT+CWMODE: err\r\n");
    #endif
    return false;
}

/*
 * AT+CWJAP
 * Connects to an AP
 */
bool esp8266_set_AT_CWJAP(const char *ssid, const char *pwd) {
    char *buf;
    esp8266_serial_prepare_AT();
    esp8266_serial_print("AT+CWJAP=\"");
    esp8266_serial_print(ssid);
    esp8266_serial_print("\",\"");
    esp8266_serial_print(pwd);
    esp8266_serial_println("\"");

    buf = esp8266_serial_read_handler("OK\r\n", "FAIL\r\n", WIFI_LOGIN_TIMEOUT);
    if (strstr((const char *)buf, "OK") != NULL) {
        return true;
    }
    #if defined(DEBUG_ESP8266_AT_ERR)
    printf("AT+CWJAP: err\r\n");
    #endif
    return false;
}

/*
 * AT+CWJAP_CUR
 * Connects to an AP
 */
bool esp8266_get_AT_CWJAP_CUR(char *ssid, char *bssid, char *channel, char *rssi) {
    bool ret;
    char *dst[4];
    char config[CONFIG_STR_MAX];
    dst[0] = ssid;
    dst[1] = bssid;
    dst[2] = channel;
    dst[3] = rssi;
    esp8266_serial_prepare_AT();
    esp8266_serial_println("AT+CWJAP_CUR?");
    ret = esp8266_serial_recv_find_filter("OK", "+CWJAP_CUR:", "\r\n\r\nOK", config, CONFIG_STR_MAX);
    if (ret) {
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
    #if defined(DEBUG_ESP8266_AT_ERR)
    printf("AT+CWJAP_CUR?: err\r\n");
    #endif
    return false;
}

/*
 * AT+CWDHCP
 * Enables/Disables DHCP
 */
bool esp8266_set_AT_CWDHCP(uint8_t mode, bool enabled) {
    char strEn[2];
    if (enabled) {
        strEn[0] = '1';
        strEn[1] = 0;
    } else {
        strEn[0] = '0';
        strEn[1] = 0;
    }
    char *buf;
    esp8266_serial_prepare_AT();
    esp8266_serial_print("AT+CWDHCP=");
    esp8266_serial_print(strEn);
    esp8266_serial_print(",");
    esp8266_serial_printiln(mode);
    buf = esp8266_serial_read_handler("OK\r\n", "FAIL\r\n", WIFI_TIMEOUT);
    if (strstr((const char *)buf, "OK") != NULL) {
        return true;
    }
    #if defined(DEBUG_ESP8266_AT_ERR)
    printf("AT+CWDHCP: err\r\n");
    #endif
    return false;
}

/*
 * AT+CIPDOMAIN
 * Connects to an AP
 */
bool esp8266_AT_CIPDOMAIN(const char *domain, uint8_t *ip) {
#define IP_BUF_SIZE 20
    char *buf;
    char ip_buf[IP_BUF_SIZE];
    if (ip == NULL) {
        return false;
    }
    if (strlen(domain) >= 64) {
        #if defined(DEBUG_ESP8266_DRIVER)
        printf("ESP8266 ERR: domain name too long\r\n");
        #endif
        return false;
    }
    esp8266_serial_prepare_AT();
    esp8266_serial_print("AT+CIPDOMAIN=\"");
    esp8266_serial_print(domain);
    esp8266_serial_println("\"");
    buf = esp8266_serial_read_handler("OK\r\n", "ERROR\r\n", WIFI_DOMAIN_TIMEOUT);
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
    #if defined(DEBUG_ESP8266_DRIVER)
    DEBUG_TXSTR(ip_buf);
    DEBUG_TXCH('\r');
    DEBUG_TXCH('\n');
    #endif
    ip_str_to_array((const char *)ip_buf, ip);
    return true;
}

/*
 * AT+CIPSTAMAC_CUR
 * Query Command:
 */
bool esp8266_get_AT_CIPSTAMAC_CUR(uint8_t *mac) {
    char str_mac[MAC_STR_MAX];
    bool ret;
    if (!mac) {
        return false;
    }
    esp8266_serial_prepare_AT();
    esp8266_serial_println("AT+CIPSTAMAC_CUR?");
    ret = esp8266_serial_recv_find_filter("OK\r\n", "+CIPSTAMAC_CUR:\"", "\r\n\r\nOK", str_mac, MAC_STR_MAX);
    if (ret) {
        mac[0] = hex_to_u8((const char *)&str_mac[0]);
        mac[1] = hex_to_u8((const char *)&str_mac[3]);
        mac[2] = hex_to_u8((const char *)&str_mac[6]);
        mac[3] = hex_to_u8((const char *)&str_mac[9]);
        mac[4] = hex_to_u8((const char *)&str_mac[12]);
        mac[5] = hex_to_u8((const char *)&str_mac[15]);
        return true;
    } else {
        #if defined(DEBUG_ESP8266_AT_ERR)
        printf("AT+CIPSTAMAC_CUR?: err\r\n");
        #endif
        return false;
    }
}

/*
 * AT+CIPSTA
 * Query Command:
 */
bool esp8266_get_AT_CIPSTA(uint8_t *ip, uint8_t *gw, uint8_t *mask) {
    char str_config[CONFIG_STR_MAX];
    char str_ip[IP_STR_MAX];
    char str_gw[GW_STR_MAX];
    char str_mask[MASK_STR_MAX];
    bool ret;
    if (!ip || !gw || !mask) {
        return false;
    }
    esp8266_serial_prepare_AT();
    esp8266_serial_println("AT+CIPSTA?");
    ret = esp8266_serial_recv_find_filter("OK", "+CIPSTA:", "\r\n\r\nOK", str_config, CONFIG_STR_MAX);
    if (ret) {
        char *start;
        char *end;
        start = str_config;
        end = parse_str_between((const char *)start, '"', '"', str_ip, IP_STR_MAX);
        start = end + 1;
        end = parse_str_between((const char *)start, '"', '"', str_gw, GW_STR_MAX);
        start = end + 1;
        end = parse_str_between((const char *)start, '"', '"', str_mask, MASK_STR_MAX);
        ip_str_to_array(str_ip, ip);
        ip_str_to_array(str_gw, gw);
        ip_str_to_array(str_mask, mask);
        return true;
    } else {
        #if defined(DEBUG_ESP8266_AT_ERR)
        printf("AT+CIPSTA?: err\r\n");
        #endif
        return false;
    }
}

/*
 * AT+CIPSTA
 * Set Command:
 */
bool esp8266_set_AT_CIPSTA(const char *ip, const char *gw, const char *mask) {
    bool ret;
    if (!ip || !gw || !mask) {
        return false;
    }
    esp8266_serial_prepare_AT();
    esp8266_serial_print("AT+CIPSTA=\"");
    esp8266_serial_print(ip);
    esp8266_serial_print("\",\"");
    esp8266_serial_print(gw);
    esp8266_serial_print("\",\"");
    esp8266_serial_print(mask);
    esp8266_serial_println("\"");
    ret = esp8266_serial_read_handler("OK\r\n", NULL, WIFI_TIMEOUT);
    #if defined(DEBUG_ESP8266_AT_ERR)
    if (!ret) {
        printf("AT+CIPSTA: err\r\n");
    }
    #endif
    return ret;
}

/*
 * AT+CIPDNS_CUR
 * Query Command:
 */
bool esp8266_get_AT_CIPDNS_CUR(uint8_t *dns) {
    char str_config[CONFIG_STR_MAX];
    bool ret;
    if (!dns) {
        return false;
    }
    esp8266_serial_prepare_AT();
    esp8266_serial_println("AT+CIPDNS_CUR?");
    ret = esp8266_serial_recv_find_filter("OK", "+CIPDNS_CUR:", "\r\n\r\nOK", str_config, CONFIG_STR_MAX);
    if (ret) {
        ip_str_to_array(str_config, dns);
        return true;
    } else {
        #if defined(DEBUG_ESP8266_AT_ERR)
        printf("AT+CIPDNS_CUR?: err\r\n");
        #endif
        return false;
    }
}

/*
 * AT+CIPDNS_CUR
 * Set Command:
 */
bool esp8266_set_AT_CIPDNS_CUR(const char *dns, bool flag) {
    bool ret;
    if (!dns) {
        return false;
    }
    esp8266_serial_prepare_AT();
    esp8266_serial_print("AT+CIPDNS_CUR=");
    if (flag) {
        esp8266_serial_print("1,\"");
    } else {
        esp8266_serial_print("0,\"");
    }
    esp8266_serial_print(dns);
    esp8266_serial_println("\"");
    ret = esp8266_serial_read_handler("OK\r\n", NULL, WIFI_TIMEOUT);
    #if defined(DEBUG_ESP8266_AT_ERR)
    if (!ret) {
        printf("AT+CIPDNS_CUR: err\r\n");
    }
    #endif
    return ret;
}

/*
 * AT+CIPSTO
 * Sets the TCP Server Timneout
 */
bool esp8266_set_AT_CIPSTO(int timeout) {
    char *buf;
    esp8266_serial_prepare_AT();
    esp8266_serial_print("AT+CIPSTO=");
    esp8266_serial_printiln(timeout);
    buf = esp8266_serial_read_handler("OK\r\n", "FAIL\r\n", WIFI_TIMEOUT);
    if (strstr((const char *)buf, "OK") != NULL) {
        return true;
    }
    #if defined(DEBUG_ESP8266_AT_ERR)
    printf("AT+CIPSTO: err\r\n");
    #endif
    return false;
}

/*
 * AT+CIPDINFO
 * Shows the Remote IP and Port with +IPD
 */
bool esp8266_set_AT_CIPDINFO(uint8_t mode) {
    esp8266_serial_prepare_AT();
    esp8266_serial_print("AT+CIPDINFO=");
    esp8266_serial_printiln(mode);
    bool ret = esp8266_serial_read_handler("OK\r\n", NULL, WIFI_TIMEOUT);
    #if defined(DEBUG_ESP8266_AT_ERR)
    if (!ret) {
        printf("AT+CIPSTO: err\r\n");
    }
    #endif
    return ret;
}

/*
 * socket_handler
 */
void esp8266_socket_handler(bool connect, int id) {
    #if defined(DEBUG_ESP8266_DRIVER)
    printf("\r\nesp8266_socket_handler(id=%d)\r\n", id);
    #endif
    _cbs[id].Notified = 0;
    if (connect) {
        set_socket_connected(id);
        if (_cip_server) {
            int *mid = (int *)tinymalloc(sizeof(int));
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
    #if defined(DEBUG_ESP8266_DRIVER)
    printf("_cip_server=%d\r\n", _cip_server);
    #endif
}

uint32_t esp8266_read(uint32_t timeout) {
    uint32_t s;
    uint8_t *data = (uint8_t *)recv_buf;
    int recvd = 0;
    char *start;
    char *end;
    int i;
    char ip_buf[IP_BUF_SIZE];

    #if defined(DEBUG_ESP8266_DRIVER)
    // printf("esp8266_read(timeout=%d)\r\n", timeout);
    #endif
    memset((void *)data, 0, sizeof(recv_buf));
    i = 0;
    s = (uint32_t)mtick();
    while ((uint32_t)mtick() - s < timeout) {
        if (esp8266_serial_available() > 0) {
            uint8_t c = (uint8_t)esp8266_serial_read();
            #if defined(DEBUG_ESP8266_DRIVER)
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
                    esp8266_socket_handler(true, 0);
                    goto esp8266_read_exit;
                }
                if (strstr((const char *)data, "1,CONNECT") != NULL) {
                    esp8266_socket_handler(true, 1);
                    goto esp8266_read_exit;
                }
                if (strstr((const char *)data, "2,CONNECT") != NULL) {
                    esp8266_socket_handler(true, 2);
                    goto esp8266_read_exit;
                }
                if (strstr((const char *)data, "3,CONNECT") != NULL) {
                    esp8266_socket_handler(true, 3);
                    goto esp8266_read_exit;
                }
                if (strstr((const char *)data, "4,CONNECT") != NULL) {
                    esp8266_socket_handler(true, 4);
                    goto esp8266_read_exit;
                }
            }
            if (strstr((const char *)data, "CLOSED") != NULL) {
                if (strstr((const char *)data, "0,CLOSED") != NULL) {
                    esp8266_socket_handler(false, 0);
                    goto esp8266_read_exit;
                }
                if (strstr((const char *)data, "1,CLOSED") != NULL) {
                    esp8266_socket_handler(false, 1);
                    goto esp8266_read_exit;
                    ;
                }
                if (strstr((const char *)data, "2,CLOSED") != NULL) {
                    esp8266_socket_handler(false, 2);
                    goto esp8266_read_exit;
                }
                if (strstr((const char *)data, "3,CLOSED") != NULL) {
                    esp8266_socket_handler(false, 3);
                    goto esp8266_read_exit;
                }
                if (strstr((const char *)data, "4,CLOSED") != NULL) {
                    esp8266_socket_handler(false, 4);
                    goto esp8266_read_exit;
                }
            }
            if (c == ':') {
                #if defined(DEBUG_ESP8266_DRIVER)
                DEBUG_TXCH('\r');
                DEBUG_TXCH('\n');
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
        goto esp8266_read_exit;
    }
    int len = 0;
    int id = 0;
    int port = 0;
    start += 5;
    int count = count_ch((const char *)start, ',');
    if (count == 0 || count == 2) {
        end = parse_int(start, ",:", &len);
        if (end == NULL) {
            goto esp8266_read_exit;
        }
    } else if (count == 1 || count == 3 || count == 4) {
        end = parse_int(start, ",:", &id);
        if (end == NULL) {
            goto esp8266_read_exit;
        }
        start = end + 1;
        end = parse_int(start, ",:", &len);
        if (end == NULL) {
            goto esp8266_read_exit;
        }
        start = end + 1;
        end = strstr((const char *)start, ",");
        if (end == NULL) {
            goto esp8266_read_exit;
        }
        int slen = end - start;
        strncpy(ip_buf, (const char *)start, slen);
        if (slen >= IP_BUF_SIZE) {
            goto esp8266_read_exit;
        }
        ip_buf[slen] = 0;
        start = end + 1;
        end = parse_int(start, ",:", &port);
        if (end == NULL) {
            goto esp8266_read_exit;
        }
    } else {
        goto esp8266_read_exit;
    }
    #if defined(DEBUG_ESP8266_DRIVER)
    printf("len=%d\r\n", len);
    #endif
    recvd = packet_handle(id, len, ESP8266_PACKET_TIMEOUT);
esp8266_read_exit:
    #if defined(DEBUG_ESP8266_DRIVER)
    if (recvd != 0) {
        printf("\r\nesp8266_read() time=%ld, recvd=%d, addr=%s, port=%d\r\n", (uint32_t)mtick() - s, recvd, ip_buf, port);
    }
    #endif
    return recvd;
}

bool esp8266_set_AT_CIPSEND(int id, const uint8_t *buffer, uint32_t len) {
    int count = 0;
    uint32_t timeout;
    esp8266_serial_prepare_AT();
    esp8266_serial_print("AT+CIPSEND=");
    esp8266_serial_printi(id);
    esp8266_serial_print(",");
    esp8266_serial_printiln(len);
    if (esp8266_serial_recv_find(">")) {
        for (uint32_t i = 0; i < len; i++) {
            uint8_t c;
            c = buffer[i];
            debug_write_off();
            esp8266_serial_write_byte(c);
            debug_write_on();
        }
        if (esp8266_serial_recv_find("SEND OK\r\n")) {
            timeout = ESP8266_READ_TIMEOUT;
            while (true) {
                count = esp8266_read(timeout);
                if (count <= 0) {
                    break;
                }
                timeout = 200;
            }
            return true;
        }
    }
    #if defined(DEBUG_ESP8266_AT_ERR)
    printf("AT+CIPSEND: err\r\n");
    #endif
    return false;
}

bool esp8266_set_AT_CIPCLOSE(int id) {
    esp8266_serial_prepare_AT();
    esp8266_serial_print("AT+CIPCLOSE=");
    esp8266_serial_printiln(id);
    char *buf = esp8266_serial_read_handler("OK\r\n", "ERROR\r\n", WIFI_TIMEOUT);
    if (strstr((const char *)buf, "OK") != NULL) {
        return true;
    }
    #if defined(DEBUG_ESP8266_AT_ERR)
    printf("AT+CIPCLOSE: err\r\n");
    #endif
    return false;
}

bool esp8266_set_AT_CIPSERVER(int port) {
    if (_cip_server) {
        return false;
    }
    esp8266_serial_prepare_AT();
    esp8266_serial_print("AT+CIPSERVER=1,");
    esp8266_serial_printiln(port);
    char *buf = esp8266_serial_read_handler("OK\r\n", "ERROR\r\n", WIFI_TIMEOUT);
    if (strstr((const char *)buf, "OK") != NULL) {
        _cip_server = true;
        return true;
    }
    #if defined(DEBUG_ESP8266_AT_ERR)
    printf("AT+CIPSERVER=1: err\r\n");
    #endif
    return false;
}

bool esp8266_reset_AT_CIPSERVER(void) {
    esp8266_serial_prepare_AT();
    esp8266_serial_println("AT+CIPSERVER=0");
    char *buf = esp8266_serial_read_handler("OK\r\n", "ERROR\r\n", WIFI_TIMEOUT);
    if (strstr((const char *)buf, "OK") != NULL) {
        _cip_server = false;
        return true;
    }
    #if defined(DEBUG_ESP8266_AT_ERR)
    printf("AT+CIPSERVER=0: err\r\n");
    #endif
    return false;
}

uint32_t esp8266_recv(uint8_t *data, uint32_t amount, uint32_t *data_len, uint32_t timeout, int id) {
    #if defined(DEBUG_ESP8266_DRIVER)
    printf("esp8266_recv(id=%d, amount=%ld)\r\n", id, amount);
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
                    uint32_t pdu_len = sizeof(_packet_t) + q->alloc_len;
                    uint32_t len = q->len;
                    if (q) {
                        #if defined(DEBUG_ESP8266_PACKET)
                        printf("packet tinyfree: id=%d, len=%d\r\n", q->id, q->alloc_len);
                        #endif
                        tinyfree((void *)q);
                    }
                    _heap_usage -= pdu_len;
                    #if defined(DEBUG_ESP8266_DRIVER)
                    printf("esp8266_recv() len=%ld\r\n", len);
                    #endif
                    *data_len = len;
                    return len;
                } else { // return only partial packet
                    memcpy((void *)data, (const void *)(q + 1), (size_t)amount);
                    q->len -= amount;
                    memmove((void *)(q + 1), (const void *)((char *)(q + 1) + amount), (size_t)q->len);
                    *data_len = amount;
                    #if defined(DEBUG_ESP8266_DRIVER)
                    printf("esp8266_recv() amount=%ld\r\n", amount);
                    #endif
                    return amount;
                }
            }
        }
        len = esp8266_read(ESP8266_READ_TIMEOUT);
        if (len <= 0) {
            break;
        }
    }
    #if defined(DEBUG_ESP8266_DRIVER)
    printf("esp8266_recv() amount=%d\r\n", len);
    #endif
    return len;
}

/*******************************************************************/
/* ESP8266 driver                                                  */
/*******************************************************************/

/*
 * driver_init
 */
void esp8266_driver_init(uint32_t uart_id, uint32_t baud) {
    #if defined(DEBUG_ESP8266_DRIVER)
    printf("esp8266_driver_init()\r\n");
    #endif
    esp8266_ch = uart_id - 1;
    esp8266_baud = baud;
    tinymalloc_init((void *)local_buf, (size_t)LOCAL_BUF_MAX);
    _heap_usage = 0;
    _packets_top = (_packet_t *)NULL;
    _packets_end = &_packets_top;
    for (int i = 0; i < ESP8266_SOCKET_COUNT; i++) {
        socket_info[i].open = false;
        socket_info[i].sport = 0;
    }
    for (int i = 0; i < ESP8266_SOCKET_COUNT; i++) {
        driver_info[i].open = false;
        driver_info[i].proto = ESP8266_UDP;
        driver_info[i].tcp_data = NULL;
        driver_info[i].tcp_data_avbl = 0;
        driver_info[i].tcp_data_rcvd = 0;
    }
    memset(_ids, 0, sizeof(_ids));
    memset(_cbs, 0, sizeof(_cbs));
    vector_init(&_accept_id);
    esp8266_serial_begin(uart_id, baud);
    esp8266_ATE0();
    esp8266_set_AT_CIPMUX(1);
}

/*
 * driver_reset
 * Note: not working now - hang up communication between ESP8266
 */
bool esp8266_driver_reset(void) {
    #if defined(DEBUG_ESP8266_DRIVER)
    printf("esp8266_driver_reset()\r\n");
    #endif
    bool ret = true;
    // ret = esp8266_AT_RST();
    // need to wait for resetting
    mp_hal_delay_ms(3000);
    packet_clear(ESP8266_ALL_SOCKET_IDS);
    return ret;
}

/*
 * gethostbyname
 */
bool esp8266_gethostbyname(const char *name, uint8_t *ip) {
    #if defined(DEBUG_ESP8266_DRIVER)
    printf("esp8266_gethostbyname()\r\n");
    #endif
    return esp8266_AT_CIPDOMAIN(name, ip);
}

/*
 * close
 */
bool esp8266_close(int id) {
    #if defined(DEBUG_ESP8266_DRIVER)
    printf("esp8266_close(id=%d)\r\n", id);
    #endif
    if (!is_socket_closed(id)) {
        esp8266_set_AT_CIPCLOSE(id);
    }
    driver_info[id].open = false;
    driver_info[id].proto = ESP8266_UDP;
    driver_info[id].tcp_data = NULL;
    driver_info[id].tcp_data_avbl = 0;
    driver_info[id].tcp_data_rcvd = 0;
    packet_clear(id);
    #if defined(DEBUG_ESP8266_DRIVER)
    printf("esp8266_close() ret=1\r\n");
    #endif
    return true;
}

/*
 * open_tcp
 */
bool esp8266_open_tcp(int id, const char *addr, int port, int keepalive) {
    #if defined(DEBUG_ESP8266_DRIVER)
    printf("esp8266_open_tcp(id=%d, keepalive=%d)\r\n", id, keepalive);
    #endif
    char ip_str[16];
    bool done = false;
    if (id >= ESP8266_SOCKET_COUNT || driver_info[id].open) {
        esp8266_errno = ESP8266_ERROR_PARAMETER;
        #if defined(DEBUG_ESP8266_DRIVER)
        printf("esp8266_open_tcp() ret=%d\r\n", false);
        #endif
        return false;
    }
    snprintf(ip_str, 16, "%u.%u.%u.%u", addr[0], addr[1], addr[2], addr[3]);
    esp8266_serial_prepare_AT();
    for (int i = 0; i < 1; i++) {
        if (keepalive) {
            esp8266_serial_print("AT+CIPSTART=");
            esp8266_serial_printi(id);
            esp8266_serial_print(",\"TCP\",\"");
            esp8266_serial_print(ip_str);
            esp8266_serial_print("\",");
            esp8266_serial_printi(port);
            esp8266_serial_print(",");
            esp8266_serial_printiln(keepalive);
        } else {
            esp8266_serial_print("AT+CIPSTART=");
            esp8266_serial_printi(id);
            esp8266_serial_print(",\"TCP\",\"");
            esp8266_serial_print(ip_str);
            esp8266_serial_print("\",");
            esp8266_serial_printiln(port);
        }
        if (!esp8266_serial_recv_find("OK\r\n")) {
            if (_sock_already) {
                _sock_already = false;
                done = esp8266_close(id);
                if (!done) {
                    #if defined(DEBUG_ESP8266_DRIVER)
                    printf("device refused to close socket\r\n");
                    #endif
                }
            }
            continue;
        } else {
            driver_info[id].open = true;
            driver_info[id].proto = ESP8266_TCP;
            #if defined(DEBUG_ESP8266_DRIVER)
            printf("TCP socket %d opened\r\n", id);
            #endif
            done = true;
            break;
        }
    }
    packet_clear(id);
    #if defined(DEBUG_ESP8266_DRIVER)
    printf("esp8266_open_tcp() ret=%d\r\n", done);
    #endif
    return done;
}

/*
 * open_udp
 */
bool esp8266_open_udp(int id, const char *addr, int port, int local_port) {
    #if defined(DEBUG_ESP8266_DRIVER)
    printf("esp8266_open_udp(%d)\r\n", id);
    #endif
    char ip_str[16];
    bool done = false;
    if (id >= ESP8266_SOCKET_COUNT || driver_info[id].open) {
        esp8266_errno = ESP8266_ERROR_PARAMETER;
        return false;
    }
    snprintf(ip_str, 16, "%u.%u.%u.%u", addr[0], addr[1], addr[2], addr[3]);
    esp8266_serial_prepare_AT();
    for (int i = 0; i < 2; i++) {
        if (local_port) {
            esp8266_serial_print("AT+CIPSTART=");
            esp8266_serial_printi(id);
            esp8266_serial_print(",\"TCP\",\"");
            esp8266_serial_print(ip_str);
            esp8266_serial_print("\",");
            esp8266_serial_printi(port);
            esp8266_serial_print(",");
            esp8266_serial_printiln(local_port);
        } else {
            esp8266_serial_print("AT+CIPSTART=");
            esp8266_serial_printi(id);
            esp8266_serial_print(",\"TCP\",\"");
            esp8266_serial_print(ip_str);
            esp8266_serial_print("\",");
            esp8266_serial_printiln(port);
        }
        if (!esp8266_serial_recv_find("OK\r\n")) {
            if (_sock_already) {
                _sock_already = false;
                done = esp8266_close(id);
                if (!done) {
                    #if defined(DEBUG_ESP8266_DRIVER)
                    printf("device refused to close socket\r\n");
                    #endif
                }
            }
            continue;
        } else {
            driver_info[id].open = true;
            driver_info[id].proto = ESP8266_UDP;
            #if defined(DEBUG_ESP8266_DRIVER)
            printf("UDP socket %d opened", id);
            #endif
            done = true;
            break;
        }
    }
    packet_clear(id);
    return done;
}

/*
 * send
 */
int32_t esp8266_send(int id, const void *data, uint32_t amount) {
    #if defined(DEBUG_ESP8266_DRIVER)
    printf("esp8266_send(id=%d, amount=%ld)\r\n", id, amount);
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
        if (amount > CIPSEND_MAX && socket_info[id].proto == ESP8266_TCP) {
            amount = CIPSEND_MAX;
        } else if (amount > CIPSEND_MAX && socket_info[id].proto == ESP8266_UDP) {
            #if defined(DEBUG_ESP8266_DRIVER)
            printf("UDP datagram maximum size is 2048");
            #endif
            esp8266_errno = ESP8266_ERROR_PARAMETER;
            return false;
        }
        bool flag = esp8266_set_AT_CIPSEND(id, (const uint8_t *)data, amount);
        if (flag) {
            ret = (int32_t)amount;
            break;
        } else {
            error_cnt++;
        }
    }
    #if defined(DEBUG_ESP8266_DRIVER)
    printf("esp8266_send() ret=%ld\r\n", ret);
    #endif
    return ret;
}

/*
 * recv_tcp
 */
int32_t esp8266_recv_tcp(int id, const void *data, uint32_t amount) {
    #if defined(DEBUG_ESP8266_DRIVER)
    printf("esp8266_recv_tcp(id=%d, amount=%ld)\r\n", id, amount);
    #endif
    uint32_t data_len;
    int32_t count = (int32_t)esp8266_recv((uint8_t *)data, amount, &data_len, WIFI_TIMEOUT, id);
    #if defined(DEBUG_ESP8266_DRIVER)
    printf("esp8266_recv_tcp() ret=%ld\r\n", count);
    #endif
    return count;
}

/*
 * recv_udp
 */
int32_t esp8266_recv_udp(int id, const void *data, uint32_t amount) {
    #if defined(DEBUG_ESP8266_DRIVER)
    printf("esp8266_recv_udp(%d)\r\n", id);
    #endif
    uint32_t data_len;
    int32_t count = (int32_t)esp8266_recv((uint8_t *)data, amount, &data_len, WIFI_TIMEOUT, id);
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
bool esp8266_accept(int *p_id) {
    #if defined(DEBUG_ESP8266_DRIVER)
    printf("esp8266_accept(%08ld)\r\n", (uint32_t)p_id);
    #endif
    bool ret = false;
    *p_id = -1;
    while (!ret) {
        if (!_cip_server) {
            break;
        }
        esp8266_serial_prepare_AT();
        if (!is_accept_id_empty()) {
            ret = true;
        } else {
            // _parser.process_oob(); // Poll for inbound packets
            esp8266_read(ESP8266_ACCEPT_TIMEOUT);
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
    #if defined(DEBUG_ESP8266_DRIVER)
    printf("esp8266_accept() ret=%d\r\n", ret);
    #endif
    return ret;
}

/*******************************************************************/
/* ESP8266 socket interface                                        */
/*******************************************************************/

/*
 * socket_open
 */
int esp8266_socket_open(void **handle, esp8266_protocol_t proto) {
    #if defined(DEBUG_ESP8266_SOCKET)
    printf("esp8266_socket_open()\r\n");
    #endif
    int err = 0;
    int id = -1;
    for (int i = 0; i < ESP8266_SOCKET_COUNT; i++) {
        if (!socket_info[i].open) {
            id = i;
            socket_info[i].open = true;
            break;
        }
    }
    if (id == -1) {
        err = -1;
    } else {
        struct esp8266_socket *socket = (struct esp8266_socket *)tinymalloc(sizeof(struct esp8266_socket));
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
            #if defined(DEBUG_ESP8266_SOCKET)
            dump_esp8266_socket(socket);
            printf("socket alloc id=%d\r\n", id);
            #endif
        }
    }
    #if defined(DEBUG_ESP8266_SOCKET_ERR)
    if (err < 0) {
        printf("esp8266_socket_open(): err\r\n");
    }
    #endif
    return err;
}

/*
 * socket_close
 */
int esp8266_socket_close(void *handle) {
    #if defined(DEBUG_ESP8266_SOCKET)
    printf("esp8266_socket_close(handle=%08x)\r\n", handle);
    #endif
    struct esp8266_socket *socket = (struct esp8266_socket *)handle;
    int err = 0;
    if (!socket) {
        err = -1;
    } else {
        esp8266_close(socket->id);
        socket->connected = false;
        socket_info[socket->id].open = false;
        socket_info[socket->id].sport = 0;
        if (socket) {
            #if defined(DEBUG_ESP8266_SOCKET)
            printf("socket tinyfree id=%d\r\n", socket->id);
            #endif
            tinyfree(socket);
        }
    }
    #if defined(DEBUG_ESP8266_SOCKET)
    printf("esp8266_socket_close() ret=%d\r\n", err);
    #endif
    #if defined(DEBUG_ESP8266_SOCKET_ERR)
    if (err < 0) {
        printf("esp8266_socket_close(): err\r\n");
    }
    #endif
    return err;
}

/*
 * socket_bind
 */
int esp8266_socket_bind(void *handle, const esp8266_socket_address_t *address) {
    #if defined(DEBUG_ESP8266_SOCKET)
    printf("esp8266_socket_bind(handle=%08x, addr=%08x)\r\n", handle, address);
    #endif
    int err = 0;
    struct esp8266_socket *socket = (struct esp8266_socket *)handle;
    if (!socket) {
        err = -1;
    } else {
        if (socket->proto == ESP8266_UDP) {
            for (int id = 0; id < ESP8266_SOCKET_COUNT; id++) {
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
            socket->addr = *(esp8266_socket_address_t *)address;
        }
    }
    #if defined(DEBUG_ESP8266_SOCKET)
    dump_socket_info();
    printf("esp8266_socket_bind() ret=%d\r\n", err);
    #endif
    #if defined(DEBUG_ESP8266_SOCKET_ERR)
    if (err < 0) {
        printf("esp8266_socket_bind(): err\r\n");
    }
    #endif
    return err;
}

/*
 * socket_listen
 */
bool esp8266_socket_listen(void *handle, int backlog) {
    #if defined(DEBUG_ESP8266_SOCKET)
    printf("esp8266_socket_listen(handle=%08x,backlog=%d)\r\n", handle, backlog);
    #endif
    int err = 0;
    struct esp8266_socket *socket = (struct esp8266_socket *)handle;
    if (!socket) {
        err = -1;
    } else {
        if (socket->proto == ESP8266_UDP) {
            err = -1;
        } else {
            if (!esp8266_set_AT_CIPSERVER(socket->addr._port)) {
                err = -1;
            }
        }
    }
    #if defined(DEBUG_ESP8266_SOCKET)
    if (!socket) {
        dump_esp8266_socket(socket);
    }
    printf("esp8266_socket_listen() ret=%d\r\n", err);
    #endif
    #if defined(DEBUG_ESP8266_SOCKET_ERR)
    if (err < 0) {
        printf("esp8266_socket_listen(): err\r\n");
    }
    #endif
    return err == 0;
}

/*
 * socket_connect
 */
bool esp8266_socket_connect(void *handle, const esp8266_socket_address_t *addr) {
    #if defined(DEBUG_ESP8266_SOCKET)
    printf("esp8266_socket_connect(handle=%08x)\r\n", handle);
    #endif
    struct esp8266_socket *socket = (struct esp8266_socket *)handle;
    int err = 0;
    if (!socket) {
        err = -1;
    } else {
        if (socket->proto == ESP8266_UDP) {
            err = esp8266_open_udp(socket->id, (const char *)&addr->_addr, addr->_port, socket_info[socket->id].sport);
        } else {
            err = esp8266_open_tcp(socket->id, (const char *)&addr->_addr, addr->_port, 1);
        }
        socket->connected = (err == 0);
    }
    #if defined(DEBUG_ESP8266_SOCKET)
    if (!socket) {
        dump_esp8266_socket(socket);
    }
    printf("esp8266_socket_connect() ret=%d\r\n", err);
    #endif
    #if defined(DEBUG_ESP8266_SOCKET_ERR)
    if (err < 0) {
        printf("esp8266_socket_connect(): err\r\n");
    }
    #endif
    return err;
}

/*
 * socket_accept
 */
bool esp8266_socket_accept(void *server, void **socket, esp8266_socket_address_t *addr) {
    #if defined(DEBUG_ESP8266_SOCKET)
    printf("esp8266_socket_accept(server=%08x,socket=%08x,addr=%08x)\r\n", server, socket, addr);
    #endif
    int id = -1;
    ;
    int err = -1;
    struct esp8266_socket *socket_new = (struct esp8266_socket *)tinymalloc(sizeof(struct esp8266_socket));
    if (!socket_new) {
        err = false;
    } else {
        if (!esp8266_accept(&id)) {
            tinyfree(socket_new);
            err = -1;
        } else {
            socket_new->id = id;
            socket_new->proto = ESP8266_TCP;
            socket_new->connected = true;
            socket_new->accept_id = true;
            socket_new->tcp_server = false;
            *socket = socket_new;
        }
        // dummy values since ESP8266 doesn't return ip address and port when connecting
        addr->_addr.bytes[0] = 0;
        addr->_addr.bytes[1] = 0;
        addr->_addr.bytes[2] = 0;
        addr->_addr.bytes[3] = 0;
        addr->_port = 0;
    }
    #if defined(DEBUG_ESP8266_SOCKET)
    dump_esp8266_socket(socket_new);
    printf("esp8266_socket_accept() ret=%d id=%d\r\n", err, id);
    #endif
    #if defined(DEBUG_ESP8266_SOCKET_ERR)
    if (err < 0) {
        printf("esp8266_socket_accept(): err\r\n");
    }
    #endif
    return err;
}

/*
 * socket_send
 */
int esp8266_socket_send(void *handle, const void *data, unsigned size) {
    #if defined(DEBUG_ESP8266_SOCKET) || defined(DEBUG_ESP8266_SOCKET_SEND)
    printf("esp8266_socket_send(handle=%08x, size=%d)\r\n", handle, size);
    #endif
    int err;
    struct esp8266_socket *socket = (struct esp8266_socket *)handle;
    if (!socket) {
        err = -1;
    } else {
        uint32_t start = (uint32_t)mtick();
        do {
            err = (int)esp8266_send(socket->id, data, size);
        } while ((start - (uint32_t)mtick() < 50) && (err != 0));
    }
    #if defined(DEBUG_ESP8266_SOCKET) || defined(DEBUG_ESP8266_SOCKET_SEND)
    printf("esp8266_socket_send() ret=%d\r\n", err);
    #endif
    #if defined(DEBUG_ESP8266_SOCKET_ERR)
    if (err < 0) {
        printf("esp8266_socket_send(): err\r\n");
    }
    #endif
    return err;
}

/*
 * socket_recv
 */
int esp8266_socket_recv(void *handle, void *data, unsigned size) {
    #if defined(DEBUG_ESP8266_SOCKET) || defined(DEBUG_ESP8266_SOCKET_RECV)
    printf("esp8266_socket_recv(handle=%08x, size=%d)\r\n", handle, size);
    #endif
    int err = 0;
    struct esp8266_socket *socket = (struct esp8266_socket *)handle;
    if (!socket) {
        err = -1;
    } else {
        if (socket->proto == ESP8266_TCP) {
            err = (int)esp8266_recv_tcp(socket->id, data, size);
            // if (recv <= 0) {
            //    socket->connected = false;
            // }
            // if (recv == 0) {
            //    recv = -1;
            // }
        } else {
            err = (int)esp8266_recv_udp(socket->id, data, size);
        }
        #if defined(DEBUG_ESP8266_SOCKET) || defined(DEBUG_ESP8266_SOCKET_RECV)
        printf("esp8266_socket_recv() ret=%d\r\n", err);
        #endif
    }
    #if defined(DEBUG_ESP8266_SOCKET_ERR)
    if (err < 0) {
        printf("esp8266_socket_recv(): err\r\n");
    }
    #endif
    return err;
}

/*
 * socket_sendto
 */
int esp8266_socket_sendto(void *handle, const esp8266_socket_address_t *addr, const void *data, unsigned size) {
    #if defined(DEBUG_ESP8266_SOCKET)
    printf("esp8266_socket_sendto()\r\n");
    #endif
    int err = 0;
    struct esp8266_socket *socket = (struct esp8266_socket *)handle;
    if (!socket) {
        err = -1;
    } else if ((strcmp((void *)&addr->_addr, "0.0.0.0") == 0) || !addr->_port) {
        err = -1;
    } else {
        // ToDo: implement compare address
        if (socket->connected && (&socket->addr != (esp8266_socket_address_t *)addr)) {
            if (!esp8266_close(socket->id)) {
                err = -1;
            }
            socket->connected = false;
        }
        if (!socket->connected) {
            err = esp8266_socket_connect(socket, addr);
            if (err >= 0) {
                socket->addr = (esp8266_socket_address_t)*addr;
            }
        }
        err = esp8266_socket_send(socket, data, size);
    }
    #if defined(DEBUG_ESP8266_SOCKET_ERR)
    if (err < 0) {
        printf("esp8266_socket_sendto(): err\r\n");
    }
    #endif
    return err;
}

/*
 * socket_recvfrom
 */
int esp8266_socket_recvfrom(void *handle, esp8266_socket_address_t *addr, void *data, unsigned size) {
    #if defined(DEBUG_ESP8266_SOCKET)
    printf("esp8266_socket_recvfrom()\r\n");
    #endif
    int err = 0;
    struct esp8266_socket *socket = (struct esp8266_socket *)handle;
    if (!socket) {
        err = -1;
    } else {
        err = esp8266_socket_recv(socket, data, size);
        if (err >= 0 && addr) {
            *addr = socket->addr;
        }
    }
    #if defined(DEBUG_ESP8266_SOCKET_ERR)
    if (err != 0) {
        printf("esp8266_socket_recvfrom(): err\r\n");
    }
    #endif
    return err;
}

/*
 * setsockopt
 */
int esp8266_setsockopt(int handle, int level, int optname, const void *optval, unsigned optlen) {
    #if defined(DEBUG_ESP8266_SOCKET)
    printf("esp8266_setsockopt(handle=%08x, level=%d, optname=%d, optlen=%d)\r\n", handle, level, optname, optlen);
    #endif
    int err = 0;
    struct esp8266_socket *socket = (struct esp8266_socket *)handle;
    if (!optlen) {
        err = -1;
    } else if (!socket) {
        err = -1;
    } else {
        if (level == ESP8266_SOCKET && socket->proto == ESP8266_TCP) {
            switch (optname) {
                case ESP8266_KEEPALIVE: {
                    if (socket->connected) { // ESP8266 limitation, keepalive needs to be given before connecting
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
    #if defined(DEBUG_ESP8266_SOCKET)
    printf("esp8266_setsockopt() ret=%d\r\n", err);
    #endif
    #if defined(DEBUG_ESP8266_SOCKET_ERR)
    if (err != 0) {
        printf("esp8266_getsockopt(): err\r\n");
    }
    #endif
    return err;
}

/*
 * getsockopt
 */
int esp8266_getsockopt(int handle, int level, int optname, void *optval, unsigned *optlen) {
    #if defined(DEBUG_ESP8266_SOCKET)
    printf("esp8266_getsockopt()\r\n");
    #endif
    int err = 0;
    struct esp8266_socket *socket = (struct esp8266_socket *)handle;
    if (!optval || !optlen) {
        err = -1;
    } else if (!socket) {
        err = -1;
    } else {
        if (level == ESP8266_SOCKET && socket->proto == ESP8266_TCP) {
            switch (optname) {
                case ESP8266_KEEPALIVE: {
                    if (*optlen > sizeof(int)) {
                        *optlen = sizeof(int);
                    }
                    memcpy(optval, &(socket->keepalive), *optlen);
                    err = -1;
                }
            }
        }
    }
    #if defined(DEBUG_ESP8266_SOCKET_ERR)
    if (err != 0) {
        printf("esp8266_getsockopt(): err\r\n");
    }
    #endif
    return err;
}
