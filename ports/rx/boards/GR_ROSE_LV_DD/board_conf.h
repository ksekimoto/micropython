//
// Copyright (c) 2022, Kentaro Sekimoto
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  -Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
//  -Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#ifndef PORTS_RX_BOARDS_GR_ROSE_BOARD_CONF_H_
#define PORTS_RX_BOARDS_GR_ROSE_BOARD_CONF_H_

#define DEBUG_BAUD 115200
#define PCLK    60000000

#if defined(USE_DBG_PRINT)

#include "rx_gpio.h"
#include "rx_sci.h"
#include "debug_printf.h"

#if !defined(DEBUG_CH)
#define DEBUG_CH 1
#endif
#if !defined(DEBUG_CH_TX)
#define DEBUG_CH_TX P26
#endif
#if !defined(DEBUG_CH_RX)
#define DEBUG_CH_RX P30
#endif
#define DEBUG_TXSTR(s)  rx_sci_tx_str(DEBUG_CH, (unsigned char *)s)
#define DEBUG_TXCH(c)   rx_sci_tx_ch(DEBUG_CH, c)
// #define DEBUG_TXSTR(S)  printf("%s", S)
// #define DEBUG_TXCH(C)   printf("%c", C)
#else
#define DEBUG_TXSTR(s)
#define DEBUG_TXCH(c)
#endif

#endif /* PORTS_RX_BOARDS_GR_ROSE_BOARD_CONF_H_ */
