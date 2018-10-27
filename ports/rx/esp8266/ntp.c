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

#include <stdint.h>
#include "../esp8266/ntp.h"
#include "../esp8266/esp8266.h"
#include "common.h"

static unsigned char ntp_send[NTP_PACKT_SIZE] = { 0xe3, 0x00, 0x06, 0xec, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static unsigned char ntp_recv[60];

uint32_t ntp(char *ipaddr, int tf) {
    int cnt;
    int ret;
    int num = 1;
    uint32_t time = 0;

    ret = esp8266_udpopen(num, ipaddr, NTP_SEND_PORT, NTP_LOCAL_PORT);
    if (!ret) {
        DBG_PRINT1("esp8266_udpOpen ERR");
        return 0xffffffff;
    }
    ret = esp8266_send(num, (char *) ntp_send, 48);
    if (ret == 0) {
        DBG_PRINT1("esp8266_send ERR");
        return 0xffffffff;
    }
    ret = esp8266_recv(num, (char *) ntp_recv, &cnt);
    if (ret != 1) {
        DBG_PRINT1("esp8266_recv ERR");
        return 0xffffffff;
    }
    esp8266_cclose(num);
    time = ((uint32_t) ntp_recv[40] << 24) + ((uint32_t) ntp_recv[41] << 16)
            + ((uint32_t) ntp_recv[42] << 8) + ((uint32_t) ntp_recv[43] << 0);
    if (tf == 1) {
        time -= 2208988800; // conversion to Unixtime
    }
    return time;
}



