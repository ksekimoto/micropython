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

#ifndef _ESP8266_H_
#define _ESP8266_H_  1

#ifdef __cplusplus
extern "C" {
#endif

#define  WIFI_SERIAL    6   /* sci6 */
#define  WIFI_BAUDRATE  115200
//#define  WIFI_CTS     15
#define  WIFI_WAIT_MSEC 10000

void esp8266_version(void);
void esp8266_cwmode(int mode);
void esp8266_serialout(int mode, int num);
void esp8266_at(int n, int mode, const char *s);
void esp8266_cwjap(const char *ssid, const char *pass);
void esp8266_softap(const char *ssid, const char *pass, int ch, int enc);
void esp8266_connectedip();
void esp8266_dhcp(int mode, int bl);
void esp8266_cifsr(void);
void esp8266_bypass(void);
void esp8266_disconnect(void);
void esp8266_multiconnect(int mode);
int esp8266_get_sd(const char *strURL, const char *strFname, int n, char **head, int ssl);
int esp8266_get(const char *strURL, char **hes, int n, int ssl);
int esp8266_udpopen(int num, const char *ipaddr, int send_port, int recv_port);
int esp8266_send(int num, char *data, int size);
int esp8266_recv(int num, char *data, int *cnt);
int esp8266_post_sd(const char *strURL, const char *strSFname, const char *strDFname, int n, char **head, int ssl);
void esp8266_cclose(int num);
int esp8266_post(const char *strURL, char *strData, const char *strDFname, int n, char **head, int ssl);
int esp8266_init(void);
char *esp8266_data_ptr(void);
int esp8266_data_len(void);

#ifdef __cplusplus
}
#endif

#endif /* _ESP8266_H */
