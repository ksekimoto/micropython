#ifndef MICROPY_INCLUDED_RX_LWIP_LWIPOPTS_H
#define MICROPY_INCLUDED_RX_LWIP_LWIPOPTS_H

#include <stdio.h>
#include <stdint.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "common.h"

#define NO_SYS                          1
#define SYS_LIGHTWEIGHT_PROT            1
#define MEM_ALIGNMENT                   4

#define LWIP_CHKSUM_ALGORITHM           3

#define LWIP_ARP                        1
#define LWIP_ETHERNET                   1
#define LWIP_NETCONN                    0
#define LWIP_SOCKET                     0
#define LWIP_STATS                      0
#define LWIP_NETIF_HOSTNAME             1

#define LWIP_IPV4                       1
#define LWIP_IPV6                       0
#define LWIP_DHCP                       1
#define LWIP_DHCP_CHECK_LINK_UP         1
#define LWIP_DNS                        1
#define LWIP_IGMP                       1

#define SO_REUSE                        1

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

#endif // MICROPY_INCLUDED_RX_LWIP_LWIPOPTS_H
