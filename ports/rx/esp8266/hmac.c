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

//**************************************************
// hmac-sha1
// ref: http://www.deadhat.com/wlancrypto/hmac_sha1.c
//**************************************************

/****************************************************************/
/* 802.11i HMAC-SHA-1 Test Code                                 */
/* Copyright (c) 2002, David Johnston                           */
/* Author: David Johnston                                       */
/* Email (home): dj@deadhat.com                                 */
/* Email (general): david.johnston@ieee.org                     */
/* Version 0.1                                                  */
/*
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*                                                              */
/* This code implements the NIST HMAC-SHA-1 algorithm as used   */
/* the IEEE 802.11i security spec.                              */
/*                                                              */
/* Supported message length is limited to 4096 characters       */
/* ToDo:                                                        */
/*   Sort out endian tolerance. Currently little endian.        */
/****************************************************************/

#include "../esp8266/hmac.h"

#define MAX_MESSAGE_LENGTH 2048

/****************************************/
/* sha1()                               */
/* Performs the NIST SHA-1 algorithm    */
/****************************************/
static unsigned long int ft(int t, unsigned long int x, unsigned long int y,
        unsigned long int z) {
    unsigned long int a, b, c;
    if (t < 20) {
        a = x & y;
        b = (~x) & z;
        c = a ^ b;
    } else if (t < 40) {
        c = x ^ y ^ z;
    } else if (t < 60) {
        a = x & y;
        b = a ^ (x & z);
        c = b ^ (y & z);
    } else if (t < 80) {
        c = (x ^ y) ^ z;
    }
    return c;
}

static unsigned long int k(int t) {
    unsigned long int c;
    if (t < 20) {
        c = 0x5a827999;
    } else if (t < 40) {
        c = 0x6ed9eba1;
    } else if (t < 60) {
        c = 0x8f1bbcdc;
    } else if (t < 80) {
        c = 0xca62c1d6;
    }
    return c;
}

//static unsigned long int rotr(int bits, unsigned long int a)
//{
//  unsigned long int c,d,e,f,g;
//  c = (0x0001 << bits)-1;
//  d = ~c;
//  e = (a & d) >> bits;
//  f = (a & c) << (32 - bits);
//  g = e | f;
//  return (g & 0xffffffff );
//}

static unsigned long int rotl(int bits, unsigned long int a) {
    unsigned long int c, d, e, f, g;
    c = (0x0001 << (32 - bits)) - 1;
    d = ~c;
    e = (a & c) << bits;
    f = (a & d) >> (32 - bits);
    g = e | f;
    return (g & 0xffffffff);
}

static void sha1(unsigned char *message, int message_length,
        unsigned char *digest) {
    int i;
    int num_blocks;
    int block_remainder;
    int padded_length;

    unsigned long int l;
    unsigned long int t;
    unsigned long int h[5];
    unsigned long int a, b, c, d, e;
    unsigned long int w[80];
    unsigned long int temp;

    /* Calculate the number of 512 bit blocks */
    padded_length = message_length + 8; /* Add length for l */
    padded_length = padded_length + 1; /* Add the 0x01 bit postfix */

    l = message_length * 8;

    num_blocks = padded_length / 64;
    block_remainder = padded_length % 64;

    if (block_remainder > 0) {
        num_blocks++;
    }

    padded_length = padded_length + (64 - block_remainder);

    /* clear the padding field */
    for (i = message_length; i < (num_blocks * 64); i++) {
        message[i] = 0x00;
    }

    /* insert b1 padding bit */
    message[message_length] = 0x80;

    /* Insert l */
    message[(num_blocks * 64) - 1] = (unsigned char) (l & 0xff);
    message[(num_blocks * 64) - 2] = (unsigned char) ((l >> 8) & 0xff);
    message[(num_blocks * 64) - 3] = (unsigned char) ((l >> 16) & 0xff);
    message[(num_blocks * 64) - 4] = (unsigned char) ((l >> 24) & 0xff);

    /* Set initial hash state */
    h[0] = 0x67452301;
    h[1] = 0xefcdab89;
    h[2] = 0x98badcfe;
    h[3] = 0x10325476;
    h[4] = 0xc3d2e1f0;

    for (i = 0; i < num_blocks; i++) {
        /* Prepare the message schedule */
        for (t = 0; t < 80; t++) {
            if (t < 16) {
                w[t] = (256 * 256 * 256) * message[(i * 64) + (t * 4)];
                w[t] += (256 * 256) * message[(i * 64) + (t * 4) + 1];
                w[t] += (256) * message[(i * 64) + (t * 4) + 2];
                w[t] += message[(i * 64) + (t * 4) + 3];
            } else if (t < 80) {
                w[t] = rotl(1, (w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16]));
            }
        }
        /* Initialize the five working variables */
        a = h[0];
        b = h[1];
        c = h[2];
        d = h[3];
        e = h[4];

        /* iterate a-e 80 times */
        for (t = 0; t < 80; t++) {
            temp = (rotl(5, a) + ft(t, b, c, d)) & 0xffffffff;
            temp = (temp + e) & 0xffffffff;
            temp = (temp + k(t)) & 0xffffffff;
            temp = (temp + w[t]) & 0xffffffff;
            e = d;
            d = c;
            c = rotl(30, b);
            b = a;
            a = temp;
        }

        /* compute the ith intermediate hash value */
        h[0] = (a + h[0]) & 0xffffffff;
        h[1] = (b + h[1]) & 0xffffffff;
        h[2] = (c + h[2]) & 0xffffffff;
        h[3] = (d + h[3]) & 0xffffffff;
        h[4] = (e + h[4]) & 0xffffffff;
    }

    digest[3] = (unsigned char) (h[0] & 0xff);
    digest[2] = (unsigned char) ((h[0] >> 8) & 0xff);
    digest[1] = (unsigned char) ((h[0] >> 16) & 0xff);
    digest[0] = (unsigned char) ((h[0] >> 24) & 0xff);
    digest[7] = (unsigned char) (h[1] & 0xff);
    digest[6] = (unsigned char) ((h[1] >> 8) & 0xff);
    digest[5] = (unsigned char) ((h[1] >> 16) & 0xff);
    digest[4] = (unsigned char) ((h[1] >> 24) & 0xff);
    digest[11] = (unsigned char) (h[2] & 0xff);
    digest[10] = (unsigned char) ((h[2] >> 8) & 0xff);
    digest[9] = (unsigned char) ((h[2] >> 16) & 0xff);
    digest[8] = (unsigned char) ((h[2] >> 24) & 0xff);
    digest[15] = (unsigned char) (h[3] & 0xff);
    digest[14] = (unsigned char) ((h[3] >> 8) & 0xff);
    digest[13] = (unsigned char) ((h[3] >> 16) & 0xff);
    digest[12] = (unsigned char) ((h[3] >> 24) & 0xff);
    digest[19] = (unsigned char) (h[4] & 0xff);
    digest[18] = (unsigned char) ((h[4] >> 8) & 0xff);
    digest[17] = (unsigned char) ((h[4] >> 16) & 0xff);
    digest[16] = (unsigned char) ((h[4] >> 24) & 0xff);
}

/******************************************************/
/* hmac-sha1()                                        */
/* Performs the hmac-sha1 keyed secure hash algorithm */
/******************************************************/
/* Moving local variables to static variables for not using stack  */
static unsigned char k0[64];
static unsigned char k0xorIpad[64];
static unsigned char step7data[64];
static unsigned char step8data[64 + 20];
static unsigned char step5data[MAX_MESSAGE_LENGTH + 128];

void hmac_sha1(unsigned char *key, int key_length, unsigned char *data,
        int data_length, unsigned char *digest) {
    int b = 64; /* blocksize */
    unsigned char ipad = 0x36;
    unsigned char opad = 0x5c;
    int i;

    for (i = 0; i < 64; i++) {
        k0[i] = 0x00;
    }
    if (key_length != b) /* Step 1 */
    {
        /* Step 2 */
        if (key_length > b) {
            sha1(key, key_length, digest);
            for (i = 0; i < 20; i++) {
                k0[i] = digest[i];
            }
        } else if (key_length < b) /* Step 3 */
        {
            for (i = 0; i < key_length; i++) {
                k0[i] = key[i];
            }
        }
    } else {
        for (i = 0; i < b; i++) {
            k0[i] = key[i];
        }
    }
    /* Step 4 */
    for (i = 0; i < 64; i++) {
        k0xorIpad[i] = k0[i] ^ ipad;
    }
    /* Step 5 */
    for (i = 0; i < 64; i++) {
        step5data[i] = k0xorIpad[i];
    }
    for (i = 0; i < data_length; i++) {
        step5data[i + 64] = data[i];
    }
    /* Step 6 */
    sha1(step5data, data_length + b, digest);
    /* Step 7 */
    for (i = 0; i < 64; i++) {
        step7data[i] = k0[i] ^ opad;
    }
    /* Step 8 */
    for (i = 0; i < 64; i++) {
        step8data[i] = step7data[i];
    }
    for (i = 0; i < 20; i++) {
        step8data[i + 64] = digest[i];
    }
    /* Step 9 */
    sha1(step8data, b + 20, digest);
}





