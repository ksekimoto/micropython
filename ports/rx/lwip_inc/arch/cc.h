#ifndef MICROPY_INCLUDED_RX_LWIP_ARCH_CC_H
#define MICROPY_INCLUDED_RX_LWIP_ARCH_CC_H

#include <assert.h>

#if defined(USE_DBG_PRINT)
#include "debug_printf.h"
#define LWIP_PLATFORM_DIAG(x)   do {debug_printf x;} while(0)
#else
#define LWIP_PLATFORM_DIAG(x)
#endif
#define LWIP_PLATFORM_ASSERT(x)  { assert(1); }

#define LWIP_NO_CTYPE_H 1

#endif // MICROPY_INCLUDED_RX_LWIP_ARCH_CC_H
