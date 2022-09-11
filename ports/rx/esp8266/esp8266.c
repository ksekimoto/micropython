/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Kentaro Sekimoto
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
/*
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "py/nlr.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/binary.h"
#include "portmodules.h"

#include "esp8266.h"

#include "common.h"
#include "lib/oofatfs/ff.h"

#if MICROPY_HW_HAS_ESP8266

char *itoa(int num, char *str, int base);

#if defined(USE_DBG_PRINT)
// #define DEBUG_ESP8266
// #define DEBUG_ESP8266_GET_DATA
// #define DEBUG_ESP8266_POST
// #define DEBUG_ESP8266_POST_HEADER
// #define DEBUG_ESP8266_POST_DATA
// #define DEBUG_ESP8266_CHKOK
#endif

#ifdef DEBUG_ESP8266
#define DEBUG_PRINT(m,v)     { debug_printf("%s:%d\r\n", m, v); }
#define DEBUG_PRINT1(a)      { debug_printf("%s", a); }
#define DEBUG_PRINTLN1(a)    { debug_printf("%s\r\n", a); }
#else
#define DEBUG_PRINT(m,v)        // do nothing
#define DEBUG_PRINT1(s)         // do nothing
#define DEBUG_PRINTLN1(s)       // do nothing
#endif

#if MICROPY_HW_ENABLE_SDCARD
#include "sd.h"
#endif

#define MAX_DIGITS 20
#define WIFI_DATA_MAX   1024

static unsigned char wifi_data[WIFI_DATA_MAX];
static int WiFiRecvOutlNum = -1; /* serial number from ESP8266 */
static int esp8266_at_ch = MICROPY_HW_ESP8266_UART_CH;
static int esp8266_at_baud = MICROPY_HW_ESP8266_UART_BAUD;

unsigned long millis() {
    return mtick();
}

static void esp8266_at_serial_begin(void) {
    rx_sci_init(esp8266_at_ch, (MICROPY_HW_ESP8266_TX)->id, (MICROPY_HW_ESP8266_RX)->id, esp8266_at_baud, 8, 0, 1, 0);
}

static int esp8266_at_serial_available(void) {
    return rx_sci_rx_any(esp8266_at_ch);
}

static int esp8266_at_serial_read(void) {
    return (int)rx_sci_rx_ch(esp8266_at_ch);
}

static void esp8266_at_serial_write_byte(unsigned char c) {
    rx_sci_tx_ch(esp8266_at_ch, (unsigned char)c);
}

#if 0
static void esp8266_at_serial_write(unsigned char *s, int len) {
    for (int i = 0; i < len; i++) {
        sci_tx_ch(esp8266_at_ch, (unsigned char)s[i]);
    }
}
#endif

static void esp8266_at_serial_print(const char *s) {
    rx_sci_tx_str(esp8266_at_ch, (uint8_t *)s);
}

static void esp8266_at_serial_printi(int i) {
    char s[MAX_DIGITS];
    itoa(i, (char *)s, 10);
    rx_sci_tx_str(esp8266_at_ch, (uint8_t *)s);
}

static void esp8266_at_serial_println(const char *s) {
    rx_sci_tx_str(esp8266_at_ch, (uint8_t *)s);
    rx_sci_tx_ch(esp8266_at_ch, '\r');
    rx_sci_tx_ch(esp8266_at_ch, '\n');
}

static void esp8266_at_serial_printiln(int i) {
    esp8266_at_serial_printi(i);
    rx_sci_tx_ch(esp8266_at_ch, '\r');
    rx_sci_tx_ch(esp8266_at_ch, '\n');
}

#if 0
/* ToDo */
static void usb_serial_begin(void) {
}

static int usb_serial_available(void) {
    return (int)0;
}

static int usb_serial_read(void) {
    return (int)0;
}

static void usb_serial_write_byte(int c) {
}

static void usb_serial_write(unsigned char *s, int len) {
}

static void usb_serial_print(char *s) {
}

static void usb_serial_println(char *s) {
}
#endif

inline char *esp8266_at_data_ptr(void) {
    return (char *)&wifi_data[0];
}

inline int esp8266_at_data_len(void) {
    return (int)strlen((const char *)&wifi_data[0]);
}

// **************************************************
// read data in wifi_data or output to serial until OK 0d0a or ERROR 0d0a
// 1:received, 0:not received 2:overflowed
// **************************************************
static int get_data(unsigned int wait_msec) {
    unsigned long times;
    int c;
    int okt = 0;
    int ert = 0;
    int len = 0;
    int n = 0;

    memset((void *)wifi_data, 0, sizeof(wifi_data));
    wifi_data[0] = 0;
    times = millis();
    while (n < WIFI_DATA_MAX) {
        if (millis() - times > wait_msec) {
            #if defined(DEBUG_ESP8266_GET_DATA)
            debug_printf("get_data:TIMEOUT\r\n");
            #endif
            wifi_data[n] = 0;
            return 0;
        }
        while ((len = esp8266_at_serial_available()) != 0) {
            // DBG_PRINT1("len=",len);
            // DBG_PRINT1("n=",n);
            for (int i = 0; i < len; i++) {
                c = esp8266_at_serial_read();
                // output received data to specified serial port
                if (WiFiRecvOutlNum >= 0) {
                    esp8266_at_serial_write_byte((unsigned char)c);
                }
                // DBG_PRINT2("c=",c);
                wifi_data[n] = c;
                n++;
                if (c == 'O') {
                    okt++;
                    ert++;
                } else if (c == 'K') {
                    okt++;
                } else if (c == 0x0d) {
                    ert++;
                    okt++;
                } else if (c == 0x0a) {
                    ert++;
                    okt++;
                    if (okt == 4 || ert == 7) {
                        // OK 0d0a || ERROR 0d0a
                        wifi_data[n] = 0;
                        #if defined(DEBUG_ESP8266_GET_DATA)
                        debug_printf("wifi_data:%s\r\n", (const char *)wifi_data);
                        #endif
                        return 1;
                    } else {
                        ert = 0;
                        okt = 0;
                    }
                } else if (c == 'E' || c == 'R') {
                    ert++;
                } else {
                    okt = 0;
                    ert = 0;
                }
            }
            times = millis();
        }
    }
    #if defined(DEBUG_ESP8266_GET_DATA)
    debug_printf("get_data:OVERFLOW\r\n");
    #endif
    return 2;
}

// **************************************************
// set station mode: WiFi.cwmode
//  mode: 1:Station, 2:SoftAP, 3:Station + SoftAP
// **************************************************
void esp8266_at_cwmode(int mode) {
    esp8266_at_serial_print((const char *)"AT+CWMODE=");
    esp8266_at_serial_printiln(mode);
    get_data(WIFI_WAIT_MSEC);
}

// **************************************************
// send command response to serial: WiFi.serialOut
//  WiFi.serialOut( mode[,serialNumber] )
//  mode: 0: not send, 1: send
//  serialNumber: serial number for sending
// **************************************************
void esp8266_at_serialout(int mode, int num) {
    // ToDo: check the original implementation
    int n = 0;
    if (mode == 0) {
        WiFiRecvOutlNum = -1;
    } else {
        if (n >= 2) {
            if (num >= 0) {
                WiFiRecvOutlNum = num;
            }
        }
    }
}

// **************************************************
// send AT command: WiFi.at
//  WiFi.at( command[, mode] )
//  command: AT command
//  mode: 0: add 'AT+' 1: not add 'AT+'
// **************************************************
void esp8266_at_at(int n, int mode, const char *s) {
    int len = (int)strlen(s);
    if (n <= 1 || mode == 0) {
        esp8266_at_serial_print((const char *)"AT+");
    }
    for (int i = 0; i < 254; i++) {
        if (i >= len) {
            break;
        }
        wifi_data[i] = s[i];
    }
    wifi_data[len] = 0;

    esp8266_at_serial_println((const char *)wifi_data);
    // DBG_PRINT2("WiFi.at",(const char *)wifi_data);
    get_data(WIFI_WAIT_MSEC);
}

// **************************************************
// connect WiFi: WiFi.connect
//  WiFi.connect(SSID,Passwd)
//  WiFi.cwjap(SSID,Passwd)
//  SSID: WiFi SSID
//  Passwd: password
// **************************************************
void esp8266_at_cwjap(const char *ssid, const char *pass) {
    int slen = (int)strlen(ssid);
    int plen = (int)strlen(pass);

    esp8266_at_serial_print((const char *)"AT+CWJAP=");
    wifi_data[0] = 0x22;     // -> "
    wifi_data[1] = 0;
    esp8266_at_serial_print((const char *)wifi_data);
    for (int i = 0; i < 254; i++) {
        if (i >= slen) {
            break;
        }
        wifi_data[i] = ssid[i];
    }
    wifi_data[slen] = 0;
    esp8266_at_serial_print((const char *)wifi_data);
    wifi_data[0] = 0x22;     // -> "
    wifi_data[1] = 0x2C;     // -> ,
    wifi_data[2] = 0x22;     // -> "
    wifi_data[3] = 0;
    esp8266_at_serial_print((const char *)wifi_data);
    for (int i = 0; i < 254; i++) {
        if (i >= plen) {
            break;
        }
        wifi_data[i] = pass[i];
    }
    wifi_data[plen] = 0;
    esp8266_at_serial_print((const char *)wifi_data);
    wifi_data[0] = 0x22;     // -> "
    wifi_data[1] = 0;
    esp8266_at_serial_println((const char *)wifi_data);
    get_data(WIFI_WAIT_MSEC);
}

// **************************************************
// WiFi access point: WiFi.softAP
//  WiFi.softAP(SSID,Passwd,Channel,Encrypt)
//  SSID: WiFi SSID
//  Passwd: password
//  Channel: channel
//  Encrypt: encryption type 0:Open, 1:WEP, 2:WPA_PSK, 3:WPA2_PSK, 4:WPA_WPA2_PSK
// **************************************************
void esp8266_at_softap(const char *ssid, const char *pass, int ch, int enc) {
    int slen = strlen(ssid);
    int plen = strlen(pass);

    if (enc < 0 || enc > 4) {
        enc = 0;
    }
    esp8266_at_serial_print((const char *)"AT+CWSAP=");
    wifi_data[0] = 0x22;     // -> "
    wifi_data[1] = 0;
    esp8266_at_serial_print((const char *)wifi_data);
    for (int i = 0; i < 254; i++) {
        if (i >= slen) {
            break;
        }
        wifi_data[i] = ssid[i];
    }
    wifi_data[slen] = 0;
    esp8266_at_serial_print((const char *)wifi_data);
    wifi_data[0] = 0x22;     // -> "
    wifi_data[1] = 0x2C;     // -> ,
    wifi_data[2] = 0x22;     // -> "
    wifi_data[3] = 0;
    esp8266_at_serial_print((const char *)wifi_data);
    for (int i = 0; i < 254; i++) {
        if (i >= plen) {
            break;
        }
        wifi_data[i] = pass[i];
    }
    wifi_data[plen] = 0;
    esp8266_at_serial_print((const char *)wifi_data);
    wifi_data[0] = 0x22;     // -> "
    wifi_data[1] = 0x2C;     // -> ,
    wifi_data[2] = 0;
    esp8266_at_serial_print((const char *)wifi_data);
    esp8266_at_serial_printi(ch);
    esp8266_at_serial_print((const char *)",");
    esp8266_at_serial_println((const char *)enc);
    get_data(WIFI_WAIT_MSEC);
}

// **************************************************
// get ip address for access point: WiFi.connetedIP
//  WiFi.connectedIP()
// **************************************************
void esp8266_at_connectedip(void) {
    esp8266_at_serial_println((const char *)"AT+CWLIF");
    get_data(WIFI_WAIT_MSEC);
}

// **************************************************
// DHCP on/off: WiFi.dhcp
//  WiFi.dhcp(mode, bool)
//  mode: 0:SoftAP, 1:Station, 2:Both softAP + Station
//  bool: 0:disable , 1:enable
// **************************************************
void esp8266_at_dhcp(int mode, int bl) {
    esp8266_at_serial_print((const char *)"AT+CWDHCP=");
    esp8266_at_serial_printi(mode);
    esp8266_at_serial_print((const char *)",");
    esp8266_at_serial_println((const char *)bl);
    get_data(WIFI_WAIT_MSEC);
}

// **************************************************
// IP address and MAC address: WiFi.ipconfig
//  WiFi.ipconfig()
//  WiFi.cifsr()
// **************************************************
void esp8266_at_cifsr(void) {
    esp8266_at_serial_println((const char *)"AT+CIFSR");
    get_data(WIFI_WAIT_MSEC);
}

#if 0
// **************************************************
// connect USB to ESP8266: WiFi.bypass
//  WiFi.bypass()
// reset for stop
// **************************************************
void esp8266_at_bypass(void) {
    int len0, len1, len;
    while (true) {
        len0 = usb_serial_available();
        len1 = esp8266_at_serial_available();
        if (len0 > 0) {
            len = len0 < 256 ? len0 : 256;
            for (int i = 0; i < len; i++) {
                // ToDo
                wifi_data[i] = (unsigned char)usb_serial_read();
            }
            esp8266_at_serial_write(wifi_data, len);
        }
        if (len1 > 0) {
            len = len1 < 256 ? len1 : 256;
            for (int i = 0; i < len; i++) {
                // ToDo
                wifi_data[i] = (unsigned char)esp8266_at_serial_read();
            }
            // ToDo
            usb_serial_write(wifi_data, len);
        }
    }
}
#endif

void esp8266_at_version(void) {
    esp8266_at_serial_println((const char *)"AT+GMR");
    get_data(WIFI_WAIT_MSEC);
}

void esp8266_at_disconnect(void) {
    esp8266_at_serial_println((const char *)"AT+CWQAP");
    get_data(WIFI_WAIT_MSEC);
}

void esp8266_at_multiconnect(int mode) {
    esp8266_at_serial_print((const char *)"AT+CIPMUX=");
    esp8266_at_serial_printiln(mode);
    get_data(WIFI_WAIT_MSEC);
}

#if MICROPY_HW_ENABLE_SDCARD
// **************************************************
// remove +IPD data contained in the file
// ipd: ipd data
// strFname1: source file
// strFname2: destination file
// **************************************************
int cut_garbage_data(const char *ipd, const char *strFname1, const char *strFname2) {
    FIL fp, fd;

    if (sd_exists(strFname2)) {
        sd_remove(strFname2);
    }
    if (!sd_open(&fp, strFname1, FA_READ)) {
        return 2;
    }
    if (!sd_open(&fd, strFname2, FA_WRITE | FA_CREATE_NEW)) {
        return 3;
    }
    int ipdLen = strlen(ipd);
    int cnt;
    unsigned char c;
    int rc;
    bool findFlg = true;
    unsigned char str[16];
    int dLen;
    unsigned long seekCnt = 0;
    while (true) {
        // finding +IPD string
        cnt = 0;
        while (true) {
            rc = sd_read_byte(&fp);
            if (rc < 0) {
                findFlg = false;
                break;
            }
            c = (unsigned char)rc;
            if (ipd[cnt] == c) {
                cnt++;
                if (cnt == ipdLen) {
                    seekCnt += cnt;
                    break;
                }
            } else if (c == 0x0D) {
                cnt = 1;
            } else {
                cnt = 0;
            }
        }
        // Serial.print("findFlg= ");
        // Serial.println(findFlg);
        if (findFlg == false) {
            break;
        }
        // get number of bytes
        cnt = 0;
        while (true) {
            rc = sd_read_byte(&fp);
            if (rc < 0) {
                findFlg = false;
                break;
            }
            c = (unsigned char)rc;
            str[cnt] = c;
            if (c == ':') {
                str[cnt] = 0;
                seekCnt += cnt + 1;
                break;
            } else if (cnt >= 15) {
                str[15] = 0;
                findFlg = false;
                break;
            }
            cnt++;
        }
        if (findFlg == false) {
            break;
        }
        // get bytes for reading
        dLen = atoi((const char *)str);
        seekCnt += dLen;
        // Serial.print("dLen= ");
        // Serial.println((const char *)str);
        while (dLen > 0) {
            if (dLen >= 256) {
                sd_read(&fp, wifi_data, 256);
                sd_write(&fd, wifi_data, 256);
                dLen -= 256;
            } else {
                sd_read(&fp, wifi_data, dLen);
                sd_write(&fd, wifi_data, dLen);
                dLen = 0;
            }
        }
    }
    if (findFlg == false) {
        // just output data without processing
        sd_seek(&fp, seekCnt);

        while (true) {
            dLen = sd_read(&fp, wifi_data, 256);
            sd_write(&fd, wifi_data, dLen);
            if (dLen < 256) {
                break;
            }
        }
    }
    sd_flush(&fd);
    sd_close(&fd);
    sd_close(&fp);
    return 1;
}

// **************************************************
// store http GET data to sd card: WiFi.httpGetSD
//  WiFi.httpGetSD( Filename, URL[,Headers] )
//  Filename: file name
//  URL: URL
//  Headers: header array to be written
//
//      0: fail
//      1: success
//      2: SD card unavailable
//      ...other errors
// **************************************************
int esp8266_at_get_sd(const char *strURL, const char *strFname, int n, char **head, int ssl) {
    const char *tmpFilename = "wifitmp.tmp";
    const char *hedFilename = "hedrfile.tmp";
    int len = 0;
    FIL fp;
    int sla, koron;
    // check if the file exists or not
    // remove the file if it exists
    if (sd_exists(hedFilename)) {
        sd_remove(hedFilename);
    }
    // open file
    if (!sd_open(&fp, hedFilename, FA_WRITE | FA_CREATE_NEW)) {
        return 3;
    }
    // write the first line
    {
        sd_write(&fp, (unsigned char *)"GET /", 5);
        // check https
        if (ssl) {
            // DBG_PRINT1("AT+CIPSSLSIZE=4096");
            esp8266_at_serial_println((const char *)"AT+CIPSSLSIZE=4096");
            get_data(WIFI_WAIT_MSEC);
        }
        // get domain from url
        len = strlen(strURL);
        sla = len;
        koron = 0;
        for (int i = 0; i < len; i++) {
            if (strURL[i] == '/') {
                sla = i;
                break;
            }
            if (strURL[i] == ':') {
                koron = i;
            }
        }
        for (int i = sla + 1; i < len; i++) {
            sd_write(&fp, &strURL[i], 1);
        }
        sd_write(&fp, (unsigned char *)" HTTP/1.1", 9);
        sd_write_byte(&fp, 0x0D);
        sd_write_byte(&fp, 0x0A);
    }
    // create host header
    {
        sd_write(&fp, (unsigned char *)"Host: ", 6);
        if (koron == 0) {
            koron = sla;
        }
        for (int i = 0; i < koron; i++) {
            sd_write_byte(&fp, strURL[i]);
        }
        sd_write_byte(&fp, 0x0D);
        sd_write_byte(&fp, 0x0A);
    }
    // when there is header information
    for (int i = 0; i < n; i++) {
        len = strlen(*head);
        // adding headers
        sd_write(&fp, (const unsigned char *)*head++, len);
        sd_write_byte(&fp, 0x0D);
        sd_write_byte(&fp, 0x0A);
    }
    // adding CRLF
    sd_write_byte(&fp, 0x0D);
    sd_write_byte(&fp, 0x0A);
    sd_flush(&fp);
    sd_close(&fp);
    // ****** AT+CIPSTART command ******
    for (int i = 0; i < sla; i++) {
        wifi_data[i] = strURL[i];
        if (i == koron) {
            wifi_data[i] = 0;
        }
    }
    wifi_data[sla] = 0;
    if (ssl) {
        esp8266_at_serial_print((const char *)"AT+CIPSTART=4,\"SSL\",\"");
    } else {
        esp8266_at_serial_print((const char *)"AT+CIPSTART=4,\"TCP\",\"");
    }
    esp8266_at_serial_print((const char *)wifi_data);
    esp8266_at_serial_print("\",");
    if (koron < sla) {
        esp8266_at_serial_println((const char *)&wifi_data[koron + 1]);
    } else {
        if (ssl) {
            esp8266_at_serial_println("443");
        } else {
            esp8266_at_serial_println("80");
        }
    }
    get_data(WIFI_WAIT_MSEC);
    if (!(wifi_data[strlen((const char *)wifi_data) - 2] == 'K'
          || wifi_data[strlen((const char *)wifi_data) - 3] == 'K')) {
        // DBG_PRINT1("WIFI ERR");
        return 0;
    }
    // Serial.print("httpServer Connect: ");
    // Serial.print((const char *)wifi_data);
    // ****** AT+CIPSEND command ******
    if (!sd_open(&fp, hedFilename, FA_READ)) {
        return 4;
    }
    // get file size
    int sByte = (int)sd_size(&fp);
    sd_close(&fp);
    // Serial.print("AT+CIPSEND=4,");
    esp8266_at_serial_print((const char *)"AT+CIPSEND=4,");
    itoa(sByte, (char *)wifi_data, 10);
    // Serial.println((const char *)wifi_data);
    esp8266_at_serial_println((const char *)wifi_data);
    get_data(WIFI_WAIT_MSEC);
    if (!(wifi_data[strlen((const char *)wifi_data) - 2] == 'K'
          || wifi_data[strlen((const char *)wifi_data) - 3] == 'K')) {
        // DBG_PRINT1("WIFI ERR");
        return 0;
    }
    // Serial.print("> Waiting: ");
    // Serial.print((const char *)wifi_data);
    // sending http GET data since mode is changed for accepting data
    {
        if (!sd_open(&fp, hedFilename, FA_READ)) {
            return 5;
        }
        wifi_data[1] = 0;
        for (int i = 0; i < sByte; i++) {
            wifi_data[0] = (unsigned char)sd_read_byte(&fp);
            // Serial.print((const char *)wifi_data);
            esp8266_at_serial_print((const char *)wifi_data);
        }
        sd_close(&fp);
        sd_remove(tmpFilename);
        get_data(WIFI_WAIT_MSEC);
        if (!(wifi_data[strlen((const char *)wifi_data) - 2] == 'K'
              || wifi_data[strlen((const char *)wifi_data) - 3] == 'K')) {
            // DBG_PRINT1("WIFI ERR");
            return 0;
        }
        // Serial.print("Send Finish: ");
        // Serial.print((const char *)wifi_data);
    }
    // ****** sending done ******
    // ****** receiving start ******
    if (!sd_open(&fp, tmpFilename, FA_WRITE | FA_CREATE_NEW)) {
        return 6;
    }
    unsigned long times;
    unsigned int wait_msec = WIFI_WAIT_MSEC;
    unsigned char recv[2];
    times = millis();
    while (true) {
        if (millis() - times > wait_msec) {
            break;
        }
        while ((len = esp8266_at_serial_available()) != 0) {
            for (int i = 0; i < len; i++) {
                recv[0] = (unsigned char)esp8266_at_serial_read();
                sd_write(&fp, (unsigned char *)recv, 1);
            }
            times = millis();
            wait_msec = 1000;
        }
    }
    sd_flush(&fp);
    sd_close(&fp);
    // ****** receiving done ******
    // Serial.println("Recv Finish");
    // removing '\r\n+\r\n+IPD,4,****:'
    int ret = cut_garbage_data("\r\n+IPD,4,", tmpFilename, strFname);
    if (ret != 1) {
        return 7;
    }
    // ****** AT+CIPCLOSE command ******
    // DBG_PRINT1("AT+CIPCLOSE=4");
    esp8266_at_serial_println((const char *)"AT+CIPCLOSE=4");
    get_data(WIFI_WAIT_MSEC);
    // Serial.println((const char *)wifi_data);
    return 1;
}
#endif

// **************************************************
// send http GET: WiFi.httpGet
//  WiFi.httpGet( URL[,Headers] )
//  sending only
//  URL: URL
//  Headers: string to be added to header
//
//      0: fail
//      1: success
// **************************************************
int esp8266_at_get(const char *strURL, char **hes, int n, int ssl) {
    int len = 0;
    int sla, cnt;
    int koron = 0;
    char sData[1024];

    // check https
    if (ssl) {
        // DBG_PRINT1("AT+CIPSSLSIZE=4096");
        esp8266_at_serial_println((const char *)"AT+CIPSSLSIZE=4096");
        get_data(WIFI_WAIT_MSEC);
    }
    // get domain from URL
    len = (int)strlen((const char *)strURL);
    sla = len;
    for (int i = 0; i < len; i++) {
        if (strURL[i] == '/') {
            sla = i;
            break;
        }
        if (strURL[i] == ':') {
            koron = i;
        }
    }
    if (koron == 0) {
        koron = sla;
    }
    // ****** AT+CIPSTART command ******
    // get domain and port number in wifi_data[]
    for (int i = 0; i < sla; i++) {
        wifi_data[i] = strURL[i];
        if (i == koron) {
            wifi_data[i] = 0;
        }
    }
    wifi_data[sla] = 0;
    if (ssl) {
        esp8266_at_serial_print((const char *)"AT+CIPSTART=4,\"SSL\",\"");
    } else {
        esp8266_at_serial_print((const char *)"AT+CIPSTART=4,\"TCP\",\"");
    }
    esp8266_at_serial_print((const char *)wifi_data);
    esp8266_at_serial_print("\",");
    if (koron < sla) {
        esp8266_at_serial_println((const char *)&wifi_data[koron + 1]);
    } else {
        if (ssl) {
            esp8266_at_serial_println("443");
        } else {
            esp8266_at_serial_println("80");
        }
    }
    get_data(WIFI_WAIT_MSEC);
    if (!(wifi_data[strlen((const char *)wifi_data) - 2] == 'K'
          || wifi_data[strlen((const char *)wifi_data) - 3] == 'K')) {
        // DBG_PRINT1("WIFI ERR");
        return 0;
    }
    // ****** AT+CIPSEND command ******
    {
        strcpy(sData, "GET /");
        cnt = 5;
        for (int i = sla + 1; i < len; i++) {
            sData[cnt] = strURL[i];
            cnt++;
        }
        sData[cnt] = 0;
        strcat(sData, " HTTP/1.1\r\n");
    }
    // create host header
    {
        strcat(sData, "Host: ");
        cnt = strlen(sData);
        for (int i = 0; i < koron; i++) {
            sData[cnt] = strURL[i];
            cnt++;
        }
        sData[cnt] = 0;
        strcat(sData, "\r\n");
    }
    // when there is header information
    for (int i = 0; i < n; i++) {
        len = (int)strlen((const char *)*hes);
        // adding header
        strcat(sData, (const char *)*hes++);
        strcat(sData, "\r\n");
    }
    // adding CRLF
    strcat(sData, "\r\n");
    // get data size
    len = strlen(sData);
    // DBG_PRINT1((const char *)sData);
    esp8266_at_serial_print("AT+CIPSEND=4,");
    esp8266_at_serial_printiln(len);
    get_data(WIFI_WAIT_MSEC);
    if (!(wifi_data[strlen((const char *)wifi_data) - 2] == 'K'
          || wifi_data[strlen((const char *)wifi_data) - 3] == 'K')) {
        // DBG_PRINT1("WIFI ERR");
        return 0;
    }
    // sending http GET data since mode is changed for accepting data
    {
        esp8266_at_serial_print((const char *)sData);
        get_data(WIFI_WAIT_MSEC);
        if (!(wifi_data[strlen((const char *)wifi_data) - 2] == 'K'
              || wifi_data[strlen((const char *)wifi_data) - 3] == 'K')) {
            // DBG_PRINT1("WIFI ERR");
            return 0;
        }
    }
    // ****** sending done ******
    // ****** receiving start ******
    unsigned long times;
    unsigned int wait_msec = WIFI_WAIT_MSEC;
    times = millis();
    while (true) {
        if (millis() - times > wait_msec) {
            break;
        }
        while ((len = esp8266_at_serial_available()) != 0) {
            for (int i = 0; i < len; i++) {
                esp8266_at_serial_read();
            }
            times = millis();
            wait_msec = 100;
        }
    }
    // ****** receiving done ******
    // ****** AT+CIPCLOSE command ******
    esp8266_at_serial_println((const char *)"AT+CIPCLOSE=4");
    get_data(WIFI_WAIT_MSEC);
    return 1;
}

// **************************************************
// close TCP/UDP connection: WiFi.cClose
//  WiFi.cClose(number)
//  number: connection number(1-4)
// **************************************************
void esp8266_at_cclose(int num) {
    esp8266_at_serial_print((const char *)"AT+CIPCLOSE=");
    esp8266_at_serial_printiln(num);
    get_data(WIFI_WAIT_MSEC);
    return;
}

// **************************************************
// start UDP connection: WiFi.udpOpen
//  WiFi.udpOpen( number, IP_Address, SendPort, ReceivePort )
//  number: connection number(1-4)
//  IP_Address: IP address
//  SendPort: sending port
//  ReceivePort: receiving port
// **************************************************
static int chk_OK() {
    char *p = (char *)wifi_data;
    int n = strlen((const char *)wifi_data);
    #if defined(DEBUG_ESP8266_CHKOK)
    DBG_PRINT1((const char *)wifi_data);
    #endif
    if (n >= 4) {
        if ((p[n - 4] == 'O') && (p[n - 3] == 'K')) {
            return 1;
        }
    }
    return 0;
}

int esp8266_at_udpopen(int num, const char *strIpAdd, int sport, int rport) {
    // ****** AT+CIPSTART command ******
    esp8266_at_serial_print((const char *)"AT+CIPSTART=");
    esp8266_at_serial_printi(num);
    esp8266_at_serial_print((const char *)",\"UDP\",\"");
    esp8266_at_serial_print((const char *)strIpAdd);
    esp8266_at_serial_print((const char *)"\",");
    esp8266_at_serial_printi(sport);
    esp8266_at_serial_print((const char *)",");
    esp8266_at_serial_printiln(rport);
    get_data(WIFI_WAIT_MSEC);
    return chk_OK();
}

// **************************************************
// send data to specified connection WiFi.send
//  WiFi.send( number, Data[, length] )
//  number: connection number(0-3)
//  Data: sending data
//  length: sending data size
//
//    data size for sending
// **************************************************
int esp8266_at_send(int num, char *strdata, int len) {
    // ****** AT+CIPSTART command ******
    esp8266_at_serial_print((const char *)"AT+CIPSEND=");
    esp8266_at_serial_printi(num);
    esp8266_at_serial_print((const char *)",");
    esp8266_at_serial_printiln(len);
    get_data(WIFI_WAIT_MSEC);
    if (!(wifi_data[strlen((const char *)wifi_data) - 2] == 'K'
          || wifi_data[strlen((const char *)wifi_data) - 3] == 'K')) {
        return 0;
    }
    // esp8266_at_serial_print((const char *)strdata);
    for (int i = 0; i < len; i++) {
        esp8266_at_serial_write_byte((char)strdata[i]);
    }
    get_data(WIFI_WAIT_MSEC);
    if (!(wifi_data[strlen((const char *)wifi_data) - 2] == 'K'
          || wifi_data[strlen((const char *)wifi_data) - 3] == 'K')) {
        // forcely sending CRLF since timeout might happen
        for (int i = 0; i < len - (int)strlen(strdata); i++) {
            esp8266_at_serial_write_byte('\r');
        }
        return 0;
    }
    return len;
}

// **************************************************
// receive data from specified connection number: WiFi.recv
//  WiFi.recv( number )
//  number: connection number(0-3)
//
//    array of received data (less than 256)
// **************************************************
// static char *esp8266_at_recv_buf[256];
int esp8266_at_recv(int num, char *recv_buf, int *recv_cnt) {
    char str[16];
    strcpy((char *)str, "\r\n+IPD,");
    itoa(num, (char *)&str[7], 10);
    strcat(str, ",");
    // sprintf((char*)str, "\r\n+IPD,%d,", num);
    // if(esp8266_at_serial_available() == 0){
    //  return -1;
    // }
    // ****** receiving start ******
    unsigned long times;
    unsigned int wait_msec = WIFI_WAIT_MSEC;
    times = millis();
    int len = strlen((char *)str);
    int cnt = 0;
    unsigned char c;
    while (true) {
        if (millis() - times > wait_msec) {
            break;
        }
        if (esp8266_at_serial_available()) {
            c = (unsigned char)esp8266_at_serial_read();
            if (str[cnt] == c) {
                cnt++;
                if (cnt == len) {
                    break;
                }
            } else if (c == 0x0D) {
                cnt = 1;
            } else {
                cnt = 0;
            }
            times = millis();
            wait_msec = 100;
        }
    }
    // get number of bytes
    times = millis();
    cnt = 0;
    while (true) {
        if (millis() - times > wait_msec) {
            str[cnt] = 0;
            break;
        }
        if (esp8266_at_serial_available()) {
            c = (unsigned char)esp8266_at_serial_read();
            str[cnt] = c;
            if (c == ':') {
                str[cnt] = 0;
                break;
            } else if (cnt >= 15) {
                str[15] = 0;
                break;
            }
            cnt++;
            times = millis();
        }
    }
    len = atoi((const char *)str);
    // Serial.print("len= ");
    // Serial.println(len);
    // receiving data
    times = millis();
    *recv_cnt = 0;
    while (true) {
        if (millis() - times > wait_msec) {
            break;
        }
        if (esp8266_at_serial_available()) {
            recv_buf[*recv_cnt] = (char)esp8266_at_serial_read();
            (*recv_cnt)++;
            if (*recv_cnt >= len) {
                break;
            }
            times = millis();
        }
    }
    // ****** receiving done ******
    return 1;
}

#if MICROPY_HW_ENABLE_SDCARD
// **************************************************
// http POST to SD card: WiFi.httpPostSD
//  WiFi.httpPostSD( URL, Headers, Filename, Filename )
//  URL: URL
//  Headers: array of header
//  Filename: POST file name
//
//      0: fail
//      1: success
//      2: unavailable SD card
//      ... other errors
// **************************************************
int esp8266_at_post_sd(const char *strURL, const char *strSFname, const char *strDFname, int n,
    char **head, int ssl) {
    const char *tmpFilename = "wifitmp.tmp";
    const char *headFilename = "headfile.tmp";
    int len = 0;
    FIL fp;
    int sla, koron;
    int sBody, sHeader;
    // get file size
    if (!sd_open(&fp, strSFname, FA_READ)) {
        return 3;
    }
    // get file size
    sBody = (int)sd_size(&fp);
    sd_close(&fp);
    if (sd_exists(headFilename)) {
        sd_remove(headFilename);
    }
    // open file
    if (!sd_open(&fp, headFilename, FA_WRITE | FA_CREATE_NEW)) {
        return 4;
    }
    esp8266_at_serial_println((const char *)"AT+CIPMUX=1");
    get_data(WIFI_WAIT_MSEC);
    // create the first line
    {
        sd_write(&fp, (unsigned char *)"POST /", 6);
        // check https
        if (ssl) {
            // DBG_PRINT1("AT+CIPSSLSIZE=4096");
            esp8266_at_serial_println("AT+CIPSSLSIZE=4096");
            get_data(WIFI_WAIT_MSEC);
        }
        // get domain from URL
        len = strlen(strURL);
        sla = len;
        koron = 0;
        for (int i = 0; i < len; i++) {
            if (strURL[i] == '/') {
                sla = i;
                break;
            }
            if (strURL[i] == ':') {
                koron = i;
            }
        }
        for (int i = sla + 1; i < len; i++) {
            sd_write_byte(&fp, strURL[i]);
        }
        sd_write(&fp, (unsigned char *)" HTTP/1.1", 9);
        sd_write_byte(&fp, 0x0D);
        sd_write_byte(&fp, 0x0A);
    }
    // adding Host header
    {
        sd_write(&fp, (unsigned char *)"Host: ", 6);
        if (koron == 0) {
            koron = sla;
        }
        for (int i = 0; i < koron; i++) {
            sd_write_byte(&fp, strURL[i]);
        }
        sd_write_byte(&fp, 0x0D);
        sd_write_byte(&fp, 0x0A);
    }
    // adding Content-Length
    {
        sd_write(&fp, (unsigned char *)"Content-Length: ", 16);
        itoa(sBody, (char *)wifi_data, 10);
        // sprintf((char*)wifi_data, "%u", sBody);
        sd_write(&fp, wifi_data, strlen((char *)wifi_data));
        sd_write_byte(&fp, 0x0D);
        sd_write_byte(&fp, 0x0A);
    }
    // when there is header information
    for (int i = 0; i < n; i++) {
        len = strlen(head[i]);
        // adding header
        sd_write(&fp, head[i], len);
        sd_write_byte(&fp, 0x0D);
        sd_write_byte(&fp, 0x0A);
    }
    // adding CRLF
    sd_write_byte(&fp, 0x0D);
    sd_write_byte(&fp, 0x0A);
    sd_flush(&fp);
    sd_close(&fp);
    // ****** AT+CIPSTART command ******
    // get domain and port number in wifi_data[]
    for (int i = 0; i < sla; i++) {
        wifi_data[i] = strURL[i];
        if (i == koron) {
            wifi_data[i] = 0;
        }
    }
    wifi_data[sla] = 0;
    if (ssl) {
        esp8266_at_serial_print((const char *)"AT+CIPSTART=4,\"SSL\",\"");
    } else {
        esp8266_at_serial_print((const char *)"AT+CIPSTART=4,\"TCP\",\"");
    }
    esp8266_at_serial_print((const char *)wifi_data);
    esp8266_at_serial_print("\",");
    if (koron < sla) {
        esp8266_at_serial_println((const char *)&wifi_data[koron + 1]);
    } else {
        if (ssl) {
            esp8266_at_serial_println((const char *)"443");
        } else {
            esp8266_at_serial_println((const char *)"80");
        }
    }
    get_data(WIFI_WAIT_MSEC);
    if (!(wifi_data[strlen((const char *)wifi_data) - 2] == 'K'
          || wifi_data[strlen((const char *)wifi_data) - 3] == 'K')) {
        // DBG_PRINT1("WIFI ERR");
        return 0;
    }
    // Serial.print("httpServer Connect: ");
    // Serial.print((const char *)wifi_data);
    // ****** AT+CIPSEND command ******
    // get header size
    if (!sd_open(&fp, headFilename, FA_READ)) {
        return 5;
    }
    // get file size
    sHeader = (int)sd_size(&fp);
    sd_close(&fp);
    // Serial.print("AT+CIPSEND=4,");
    esp8266_at_serial_print((const char *)"AT+CIPSEND=4,");
    itoa(sHeader + sBody, (char *)wifi_data, 10);
    // sprintf((char*)wifi_data, "%u", sHeader + sBody);
    // Serial.println((const char *)wifi_data);
    esp8266_at_serial_println((const char *)wifi_data);
    get_data(WIFI_WAIT_MSEC);
    if (!(wifi_data[strlen((const char *)wifi_data) - 2] == 'K'
          || wifi_data[strlen((const char *)wifi_data) - 3] == 'K')) {
        // DBG_PRINT1("WIFI ERR");
        return 0;
    }
    // Serial.print("> Waiting: ");
    // Serial.print((const char *)wifi_data);
    // sending http POST data since mode is changed for accepting data
    {
        // sending header first
        if (!sd_open(&fp, headFilename, FA_READ)) {
            return 6;
        }
        wifi_data[1] = 0;
        for (int i = 0; i < sHeader; i++) {
            wifi_data[0] = (unsigned char)sd_read_byte(&fp);
            // Serial.print((const char *)wifi_data);
            esp8266_at_serial_print((const char *)wifi_data);
        }
        sd_close(&fp);
        // sending body first
        if (!sd_open(&fp, strSFname, FA_READ)) {
            return 7;
        }
        wifi_data[1] = 0;
        for (int i = 0; i < sBody; i++) {
            wifi_data[0] = (unsigned char)sd_read_byte(&fp);
            // Serial.print((const char *)wifi_data);
            esp8266_at_serial_print((const char *)wifi_data);
        }
        sd_close(&fp);
        get_data(WIFI_WAIT_MSEC);
        if (!(wifi_data[strlen((const char *)wifi_data) - 2] == 'K'
              || wifi_data[strlen((const char *)wifi_data) - 3] == 'K')) {
            // DBG_PRINT1("WIFI ERR");
            return 0;
        }
        // Serial.print("Send Finish: ");
        // Serial.print((const char *)wifi_data);
    }
    // ****** sending done ******
    // ****** receiving start ******
    if (sd_exists(tmpFilename)) {
        sd_remove(tmpFilename);
    }
    if (!sd_open(&fp, tmpFilename, FA_WRITE | FA_CREATE_NEW)) {
        return 6;
    }
    unsigned long times;
    unsigned int wait_msec = WIFI_WAIT_MSEC;
    unsigned char recv[2];
    times = millis();
    while (true) {
        if (millis() - times > wait_msec) {
            break;
        }
        while ((len = esp8266_at_serial_available()) != 0) {
            for (int i = 0; i < len; i++) {
                // esp8266_at_serial_read();
                recv[0] = (unsigned char)esp8266_at_serial_read();
                if (n >= 4) {
                    sd_write(&fp, (unsigned char *)recv, 1);
                }
            }
            times = millis();
            wait_msec = 100;
        }
    }
    sd_flush(&fp);
    sd_close(&fp);
    // ****** receiving done ******
    // Serial.println("Recv Finish");
    if (strDFname) {
        // removing '\r\n+\r\n+IPD,4,****:'
        int ret = cut_garbage_data("\r\n+IPD,4,", tmpFilename,
            (const char *)strDFname);
        if (ret != 1) {
            return 7;
        }
    }
    // ****** AT+CIPCLOSE command ******
    esp8266_at_serial_println((const char *)"AT+CIPCLOSE=4");
    get_data(WIFI_WAIT_MSEC);
    return 1;
}
#endif

// **************************************************
// http POST: WiFi.httpPost
//  WiFi.httpPost( URL, Headers, data )
//  only sending, no receiving response
//  URL: URL
//  Headers: array of header data
//  Data: POST data
//
//      0: fail
//      1: success
// **************************************************
static char sData[1024];
int esp8266_at_post(const char *strURL, char *strData, const char *strDFname, int n, char **head,
    int ssl) {
    int sBody, sHeader;
    int sla, cnt;
    int koron = 0;
    int len;
    #if MICROPY_HW_ENABLE_SDCARD
    FIL fp;
    const char *tmpFilename = (const char *)"wifitmp.tmp";
    #endif
    sBody = strlen(strData);
    esp8266_at_serial_println((const char *)"AT+CIPMUX=1");
    get_data(WIFI_WAIT_MSEC);
    // check https
    if (ssl) {
        #if defined(DEBUG_ESP8266_POST)
        debug_printf("AT+CIPSSLSIZE=4096\r\n");
        #endif
        esp8266_at_serial_println((const char *)"AT+CIPSSLSIZE=4096");
        get_data(WIFI_WAIT_MSEC);
    }
    // get domain from URL
    len = strlen(strURL);
    sla = len;
    for (int i = 0; i < len; i++) {
        if (strURL[i] == '/') {
            sla = i;
            break;
        }
        if (strURL[i] == ':') {
            koron = i;
        }
    }
    if (koron == 0) {
        koron = sla;
    }
    // ****** AT+CIPSTART command ******
    // get domain and port number in wifi_data[]
    for (int i = 0; i < sla; i++) {
        wifi_data[i] = strURL[i];
        if (i == koron) {
            wifi_data[i] = 0;
        }
    }
    wifi_data[sla] = 0;
    if (ssl) {
        esp8266_at_serial_print((const char *)"AT+CIPSTART=4,\"SSL\",\"");
    } else {
        esp8266_at_serial_print((const char *)"AT+CIPSTART=4,\"TCP\",\"");
    }
    esp8266_at_serial_print((const char *)wifi_data);
    esp8266_at_serial_print((const char *)"\",");
    if (koron < sla) {
        esp8266_at_serial_println((const char *)&wifi_data[koron + 1]);
    } else {
        if (ssl) {
            esp8266_at_serial_println((const char *)"443");
        } else {
            esp8266_at_serial_println((const char *)"80");
        }
    }
    get_data(WIFI_WAIT_MSEC);
    if (!(wifi_data[strlen((const char *)wifi_data) - 2] == 'K'
          || wifi_data[strlen((const char *)wifi_data) - 3] == 'K')) {
        #ifdef DEBUG_ESP8266
        debug_printf("WIFI ERR 1\r\n");
        #endif
        return 0;
    }
    // ****** AT+CIPSEND command ******
    {
        strcpy(sData, "POST /");
        cnt = 6;
        for (int i = sla + 1; i < len; i++) {
            sData[cnt] = strURL[i];
            cnt++;
        }
        sData[cnt] = 0;
        strcat(sData, " HTTP/1.1\r\n");
    }
    // Host header
    {
        strcat(sData, "Host: ");
        cnt = strlen(sData);
        for (int i = 0; i < koron; i++) {
            sData[cnt] = strURL[i];
            cnt++;
        }
        sData[cnt] = 0;
        strcat(sData, "\r\n");
    }
    // adding Content-Length
    {
        strcat(sData, "Content-Length: ");
        itoa(sBody, (char *)wifi_data, 10);
        // sprintf((char*)wifi_data, "%d", sBody);
        strcat(sData, (char *)wifi_data);
        strcat(sData, "\r\n");
    }
    // adding header information
    for (int i = 0; i < n; i++) {
        strcat(sData, head[i]);
        strcat(sData, "\r\n");
    }
    // adding CRLF
    strcat(sData, "\r\n");
    // get data size for sending
    sHeader = strlen(sData);
    len = sHeader + sBody;
    esp8266_at_serial_print((const char *)"AT+CIPSEND=4,");
    esp8266_at_serial_printiln(len);
    get_data(WIFI_WAIT_MSEC);
    if (!(wifi_data[strlen((const char *)wifi_data) - 2] == 'K'
          || wifi_data[strlen((const char *)wifi_data) - 3] == 'K')) {
        #ifdef DEBUG_ESP8266
        debug_printf("WIFI ERR 2\r\n");
        #endif
        return 0;
    }
    // sending http POST data since mode is changed for accepting data
    {
        // sending header
        esp8266_at_serial_print((const char *)sData);
        #if defined(DEBUG_ESP8266_POST_HEADER)
        debug_printf("Header(%d)\r\n%s\r\n", strlen(sData), sData);
        #endif
        // sending body
        esp8266_at_serial_print((const char *)strData);
        #if defined(DEBUG_ESP8266_POST_DATA)
        debug_printf("Data(%d)\r\n%s\r\n", strlen(strData), strData);
        #endif
        get_data(WIFI_WAIT_MSEC);
        if (!(wifi_data[strlen((const char *)wifi_data) - 2] == 'K'
              || wifi_data[strlen((const char *)wifi_data) - 3] == 'K')) {
            #ifdef DEBUG_ESP8266
            debug_printf("WIFI ERR 3\r\n");
            #endif
            return 0;
        }
    }
    // ****** sending done ******
    // ****** receiving start ******
    #if MICROPY_HW_ENABLE_SDCARD
    if (sd_exists(tmpFilename)) {
        sd_remove(tmpFilename);
    }
    if (!sd_open(&fp, tmpFilename, FA_WRITE | FA_CREATE_NEW)) {
        return 6;
    }
    #endif
    unsigned long times;
    unsigned int wait_msec = WIFI_WAIT_MSEC;
    #if MICROPY_HW_ENABLE_SDCARD
    unsigned char recv[2];
    #endif
    times = millis();
    while (true) {
        if (millis() - times > wait_msec) {
            break;
        }
        while ((len = esp8266_at_serial_available()) != 0) {
            for (int i = 0; i < len; i++) {
                #if MICROPY_HW_ENABLE_SDCARD
                recv[0] = (unsigned char)esp8266_at_serial_read();
                sd_write(&fp, (unsigned char *)recv, 1);
                #else
                esp8266_at_serial_read();
                #endif
            }
            times = millis();
            wait_msec = 100;
        }
    }
    #if MICROPY_HW_ENABLE_SDCARD
    sd_flush(&fp);
    sd_close(&fp);
    #endif
    // ****** receiving done ******
    #if MICROPY_HW_ENABLE_SDCARD
    if (strDFname) {
        // removing '\r\n+\r\n+IPD,4,****:'
        int ret = cut_garbage_data("\r\n+IPD,4,", tmpFilename,
            (const char *)strDFname);
        if (ret != 1) {
            return 7;
        }
    }
    #endif
    // ****** AT+CIPCLOSE command ******
    esp8266_at_serial_println((const char *)"AT+CIPCLOSE=4");
    get_data(WIFI_WAIT_MSEC);
    return 1;
}

int esp8266_at_init(void) {
    // not output data received from ESP8266
    WiFiRecvOutlNum = -1;
    // set OUTPUT pin15 for CTS
    // pinMode(wrb2sakura(WIFI_CTS), 1);
    // digitalWrite(wrb2sakura(WIFI_CTS), 1);
    // set serial3 for WiFi
    // initialize serial
    esp8266_at_serial_begin();
    int len;
    int ret;
    int cnt = 0;

    while (true) {
        // clear receive buffer
        while ((len = esp8266_at_serial_available()) > 0) {
            // RbSerial[0]->print(len);
            for (int i = 0; i < len; i++) {
                esp8266_at_serial_read();
            }
        }
        // send ECHO OFF command
        esp8266_at_serial_println((const char *)"ATE0");
        ret = get_data(500);
        if (ret == 1) {
            // 1: wifi available
            break;
        } else if (ret == 0) {
            // wifi unavailable when timeout
            if (cnt >= 3) {
                return 0;
            }
        }
        // more than 256 bytes when not 0,1
        cnt++;
        if (cnt >= 3) {
            // giving up when ATE0 3 times
            return 0;
        }
    }
    return 1;
}

#endif
