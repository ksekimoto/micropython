#ifndef MICROPY_INCLUDED_RX_LWIP_LWIPOPTS_H
#define MICROPY_INCLUDED_RX_LWIP_LWIPOPTS_H

#define SSIZE_MAX INT_MAX

#include <stdio.h>
#include <stdint.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "common.h"

//#define LWIP_DEBUG                      LWIP_DBG_ON
//#define LWIP_DBG_MIN_LEVEL              LWIP_DBG_LEVEL_ALL
//#define DHCP_DEBUG                      LWIP_DBG_ON
//#define DNS_DEBUG                       LWIP_DBG_ON
//#define SNTP_DEBUG                      LWIP_DBG_ON
//#define TCP_DEBUG                       LWIP_DBG_ON
//#define IP_DEBUG                        LWIP_DBG_ON

#define NO_SYS                          1
#define SYS_LIGHTWEIGHT_PROT            1

#define LWIP_ARP                        1
#define LWIP_ETHERNET                   1
#define LWIP_NETCONN                    0
#define LWIP_SOCKET                     0
#define LWIP_STATS                      0

#define LWIP_DHCP                       1
#define LWIP_DNS                        1
#define LWIP_IGMP                       1

#define SO_REUSE                        1

//#define SNTP_STARTUP_DELAY              1       // when setting 1, never worked
#define SNTP_SERVER_DNS                 1       // necessary for sntp_setservername()
//#define SNTP_GET_SERVERS_FROM_DHCP      0
//#define SNTP_MAX_SERVERS                3
//#define SNTP_SUPPORT_MULTIPLE_SERVERS   1

#define SNTP_SET_SYSTEM_TIME_US(a, b)   sntp_set_system_time(a, b)

//#define TCP_OVERSIZE_DBGCHECK           1
//#define LWIP_CHECKSUM_ON_COPY           1
//#define CHECKSUM_GEN_TCP                1
//#define IP_REASSEMBLY                   0

extern uint32_t rng_get(void);
#define LWIP_RAND() rng_get()

// default
// lwip takes 15800 bytes; TCP d/l: 380k/s local, 7.2k/s remote
// TCP u/l is very slow

#if defined(RX63N)
// lwip takes 19159 bytes; TCP d/l and u/l are around 320k/s on local network
#define MEM_SIZE (5000)
#define TCP_WND (4 * TCP_MSS)
#define TCP_SND_BUF (4 * TCP_MSS)
#endif

#if defined(RX64M) || defined(RX65N)
// lwip takes 26700 bytes; TCP dl/ul are around 750/600 k/s on local network
#define MEM_SIZE (8000)
#define TCP_MSS (800)
#define TCP_WND (8 * TCP_MSS)
#define TCP_SND_BUF (8 * TCP_MSS)
#define MEMP_NUM_TCP_SEG (32)
#endif

#if 0
// lwip takes 45600 bytes; TCP dl/ul are around 1200/1000 k/s on local network
#define MEM_SIZE (16000)
#define TCP_MSS (1460)
#define TCP_WND (8 * TCP_MSS)
#define TCP_SND_BUF (8 * TCP_MSS)
#define MEMP_NUM_TCP_SEG (32)
#endif

typedef uint32_t sys_prot_t;

#if !defined(MICROPY_PY_LWIP)
// For now, we can simply define this as a macro for the timer code. But this function isn't
// universal and other ports will need to do something else. It may be necessary to move
// things like this into a port-provided header file.
#define sys_now mp_hal_ticks_ms
#endif

#include "lwip/arch.h"
#if !(LWIP_VER == 1)
#include "sntp_client.h"
#endif

#endif // MICROPY_INCLUDED_RX_LWIP_LWIPOPTS_H
