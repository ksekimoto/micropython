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
#include "../esp8266/urlencode.h"

static char hex_to_num(char ch) {
    if (('0' <= ch) && (ch <= '9')) {
        return (ch - '0');
    } else if (('A' <= ch) && (ch <= 'F')) {
        return (ch - 'A' + 10);
    } else if (('a' <= ch) && (ch <= 'f')) {
        return (ch - 'a' + 10);
    }
    return ch;
}

static char hex[] = "0123456789ABCDEF";
static char num_to_hex(char num) {
    return hex[num & 0xf];
}

static int _isalnum(char ch) {
    if ((('0' <= ch) && (ch <= '9')) || (('A' <= ch) && (ch <= 'Z'))
            || (('a' <= ch) && (ch <= 'z')))
        return 1;
    else
        return 0;
}

char *_url_encode(char *str, char *dst, int size) {
    char *pstr = str;
    char *pbuf = dst;
    if (size == 0)
        return dst;
    size--;
    while (size && (*pstr)) {
        if (_isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.'
                || *pstr == '~') {
            *pbuf++ = *pstr;
            size--;
        } else if (*pstr == ' ') {
            *pbuf++ = '+';
            size--;
        } else {
            *pbuf++ = '%';
            size--;
            if (size == 0)
                break;
            *pbuf++ = num_to_hex(*pstr >> 4);
            size--;
            if (size == 0)
                break;
            *pbuf++ = num_to_hex(*pstr & 0xf);
        }
        pstr++;
    }
    *pbuf = '\0';
    return dst;
}

char *_url_decode(char *str, char *dst, int size) {
    char *pstr = str;
    char *pbuf = dst;
    if (size == 0)
        return dst;
    size--;
    while (size && (*pstr)) {
        if (*pstr == '%') {
            if (pstr[1] && pstr[2]) {
                *pbuf++ = hex_to_num(pstr[1]) << 4 | hex_to_num(pstr[2]);
                pstr += 2;
            }
        } else if (*pstr == '+') {
            *pbuf++ = ' ';
        } else {
            *pbuf++ = *pstr;
        }
        pstr++;
        size--;
    }
    *pbuf = '\0';
    return dst;
}




