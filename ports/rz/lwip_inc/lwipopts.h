#ifndef MICROPY_INCLUDED_RZ_LWIP_LWIPOPTS_H
#define MICROPY_INCLUDED_RZ_LWIP_LWIPOPTS_H

#define SSIZE_MAX INT_MAX

#include <stdio.h>
#include <stdint.h>

// This protection is not needed, instead we execute all lwIP code at PendSV priority
#define SYS_ARCH_DECL_PROTECT(lev) do { } while (0)
#define SYS_ARCH_PROTECT(lev) do { } while (0)
#define SYS_ARCH_UNPROTECT(lev) do { } while (0)

#define NO_SYS                          1
#define SYS_LIGHTWEIGHT_PROT            1
#define MEM_ALIGNMENT                   4

#define LWIP_CHKSUM_ALGORITHM           3
#define LWIP_CHECKSUM_CTRL_PER_NETIF    1

#define LWIP_ARP                        1
#define LWIP_ETHERNET                   1
#define LWIP_RAW                        1
#define LWIP_NETCONN                    0
#define LWIP_SOCKET                     0
#define LWIP_STATS                      0
#define LWIP_NETIF_HOSTNAME             1
#define LWIP_NETIF_EXT_STATUS_CALLBACK  1

#define LWIP_IPV6                       0
#define LWIP_DHCP                       1
#define LWIP_DHCP_CHECK_LINK_UP         1
#define DHCP_DOES_ARP_CHECK             0 // to speed DHCP up
#define LWIP_DNS                        1
#define LWIP_DNS_SUPPORT_MDNS_QUERIES   1
#define LWIP_MDNS_RESPONDER             1
#define LWIP_IGMP                       1

#define LWIP_NUM_NETIF_CLIENT_DATA      LWIP_MDNS_RESPONDER
#define MEMP_NUM_UDP_PCB                (4 + LWIP_MDNS_RESPONDER)
#define MEMP_NUM_SYS_TIMEOUT            (LWIP_NUM_SYS_TIMEOUT_INTERNAL + LWIP_MDNS_RESPONDER)

#define SO_REUSE                        1
#define TCP_LISTEN_BACKLOG              1

//#define SNTP_STARTUP_DELAY              1       // when setting 1, never worked
#define SNTP_SERVER_DNS                 1       // necessary for sntp_setservername()
#define SNTP_CHECK_RESPONSE             1
//#define SNTP_STARTUP_DELAY_FUNC         ((LWIP_RAND() % 5000) + 500)
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

#if 0
// lwip takes 19159 bytes; TCP d/l and u/l are around 320k/s on local network
#define MEM_SIZE (5000)
#define TCP_WND (4 * TCP_MSS)
#define TCP_SND_BUF (4 * TCP_MSS)
#endif

#if 1
// lwip takes 26700 bytes; TCP dl/ul are around 750/600 k/s on local network
#define MEM_SIZE (8000)
#define TCP_MSS (800)
#define TCP_WND (8 * TCP_MSS)
#define TCP_SND_BUF (8 * TCP_MSS)
#define MEMP_NUM_TCP_SEG (32)

#else

// lwip takes 45600 bytes; TCP dl/ul are around 1200/1000 k/s on local network
#define MEM_SIZE (16000)
#define TCP_MSS (1460)
#define TCP_WND (8 * TCP_MSS)
#define TCP_SND_BUF (8 * TCP_MSS)
#define MEMP_NUM_TCP_SEG (32)
#endif

typedef uint32_t sys_prot_t;

#include "lwip/arch.h"
#include "sntp_client.h"

#endif // MICROPY_INCLUDED_RZ_LWIP_LWIPOPTS_H
