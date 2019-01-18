#ifndef MICROPY_INCLUDED_RX_LWIP_ARCH_CC_H
#define MICROPY_INCLUDED_RX_LWIP_ARCH_CC_H

#include <assert.h>

#if (LWIP_VER == 1)

// ====================
// LWIP version 1.x
// ====================
#include <stdint.h>

// Generate lwip's internal types from stdint

typedef uint8_t u8_t;
typedef int8_t s8_t;
typedef uint16_t u16_t;
typedef int16_t s16_t;
typedef uint32_t u32_t;
typedef int32_t s32_t;

typedef u32_t mem_ptr_t;

#define U16_F "hu"
#define S16_F "hd"
#define X16_F "hx"
#define U32_F "u"
#define S32_F "d"
#define X32_F "x"

#define X8_F  "02x"
#define SZT_F "u"

// XXX FIXME shouldn't undef it, it's a symbol already defined in toolchain...
#undef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN

#define LWIP_CHKSUM_ALGORITHM 3

#include <assert.h>
#define LWIP_PLATFORM_DIAG(x)
#define LWIP_PLATFORM_ASSERT(x)  { assert(1); }

//#define PACK_STRUCT_FIELD(x) x __attribute__((packed))
#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_STRUCT __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

#else
// ====================
// LWIP version 2.x
// ====================
#if defined(USE_DBG_PRINT)
#include "debug_printf.h"
#define LWIP_PLATFORM_DIAG(x)   do {debug_printf x;} while(0)
#else
#define LWIP_PLATFORM_DIAG(x)
#endif
#define LWIP_PLATFORM_ASSERT(x)  { assert(1); }

#endif

#endif // MICROPY_INCLUDED_RX_LWIP_ARCH_CC_H
