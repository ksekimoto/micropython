#pragma once

#ifndef JPEG_DEBUG_H_
#define JPEG_DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#if defined(USE_DBG_PRINT)
#define DEBUG_LCDSPI
#define DEBUG_JPEG
#define DEBUG_JPEG_SRC
#endif

#if defined(_MSC_VER)
#define debug_printf    printf
#endif

#ifdef __cplusplus
}
#endif

#endif
