/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
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

/**
 * @file
 * SNTP client module
 *
 */

/*
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 */

#include <string.h>
#include <stdio.h>

#include "py/objlist.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "py/mperrno.h"
#include "py/mphal.h"

#if MICROPY_PY_LWIP

#include "lwipopts.h"

#include "lib/netutils/netutils.h"

#include "lwip/sys.h"
#include "lwip/sockets.h"

#include <string.h>
#include <time.h>

#include "lwip_utils.h"

time_t get_sntp_time(void) {
    int sock;
    struct sockaddr_in local;
    struct sockaddr_in to;
    int tolen;
    int size;
    int timeout;
    u8_t sntp_request[SNTP_MAX_DATA_LEN];
    u8_t sntp_response[SNTP_MAX_DATA_LEN];
    u32_t sntp_server_address;
    u32_t timestamp;
    time_t t;

    /* initialize SNTP server address */
    sntp_server_address = SNTP_SERVER_ADDRESS;

    /* if we got a valid SNTP server address... */
    if (sntp_server_address != 0) {
        /* create new socket */
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock >= 0) {
            /* prepare local address */
            memset(&local, 0, sizeof(local));
            local.sin_family = AF_INET;
            local.sin_port = htons(INADDR_ANY);
            local.sin_addr.s_addr = htonl(INADDR_ANY);

            /* bind to local address */
            if (bind(sock, (struct sockaddr *)&local, sizeof(local)) == 0) {
                /* set recv timeout */
                timeout = SNTP_RECV_TIMEOUT;
                setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

                /* prepare SNTP request */
                memset(sntp_request, 0, sizeof(sntp_request));
                sntp_request[0] = SNTP_LI_NO_WARNING | SNTP_VERSION | SNTP_MODE_CLIENT;

                /* prepare SNTP server address */
                memset(&to, 0, sizeof(to));
                to.sin_family = AF_INET;
                to.sin_port = htons(SNTP_PORT);
                to.sin_addr.s_addr = sntp_server_address;

                /* send SNTP request to server */
                if (sendto(sock, sntp_request, sizeof(sntp_request), 0, (struct sockaddr *)&to, sizeof(to)) >= 0) {
                    /* receive SNTP server response */
                    tolen = sizeof(to);
                    size = recvfrom(sock, sntp_response, sizeof(sntp_response), 0, (struct sockaddr *)&to, (socklen_t *)&tolen);

                    /* if the response size is good */
                    if (size == SNTP_MAX_DATA_LEN) {
                        /* if this is a SNTP response... */
                        if (((sntp_response[0] & SNTP_MODE_MASK) == SNTP_MODE_SERVER) || ((sntp_response[0] & SNTP_MODE_MASK) == SNTP_MODE_BROADCAST)) {
                            /* extract GMT time from response */
                            SMEMCPY(&timestamp, (sntp_response+SNTP_RCV_TIME_OFS), sizeof(timestamp));
                            t = (ntohl(timestamp) - DIFF_SEC_1900_1970);

                        } else {
                            LWIP_DEBUGF( SNTP_DEBUG, ("sntp_request: not response frame code\n"));
                        }
                    } else {
                        LWIP_DEBUGF( SNTP_DEBUG, ("sntp_request: not recvfrom==%i\n", errno));
                    }
                } else {
                    LWIP_DEBUGF( SNTP_DEBUG, ("sntp_request: not sendto==%i\n", errno));
                }
            }
            /* close the socket */
            closesocket(sock);
        }
    }
    return t;
}

#endif
