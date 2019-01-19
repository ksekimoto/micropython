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
 * Author: Dirk Ziegelmeier <dziegel@gmx.de>
 *
 */

#include <time.h>

#include "lwip/opt.h"
#include "lwip/apps/sntp.h"
#include "lwip/netif.h"
#include "sntp_client.h"

//#define DEBUG_SNTP_CLIENT

void set_time(time_t *t);

void sntp_set_system_time(u32_t sec, u32_t frac) {
#if defined(DEBUG_SNTP_CLIENT)
    char buf[32];
#endif
    struct tm current_time_val;
    time_t current_time = (time_t)sec;

    set_time(&current_time);
#ifdef _MSC_VER
    localtime_s(&current_time_val, &current_time);
#else
    localtime_r(&current_time, &current_time_val);
#endif
#if defined(DEBUG_SNTP_CLIENT)
    strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M:%S", &current_time_val);
    printf("SNTP time: %s\n", buf);
#endif
}

void sntp_client_init(void) {
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 5;
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        sntp_init();
        mdelay(200);
        time(&now);
        localtime_r(&now, &timeinfo);
        sntp_stop();
    }
}

#if 0
void system_time_update(void) {
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 10;
    while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        mdelay(200);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
}

void system_time_init(void) {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    if (timeinfo.tm_year < (2016 - 1900)) {
        system_time_update();
        time(&now);
    }
}
#endif
