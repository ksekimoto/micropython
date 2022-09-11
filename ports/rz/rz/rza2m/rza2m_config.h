/*
 * Copyright (c) 2020, Kentaro Sekimoto
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  -Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *  -Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PORTS_RZ_RZA2M_RZA2M_CONFIG_H_
#define PORTS_RZ_RZA2M_RZA2M_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SCI_CH  4
#define SCI_BAUD 115200
#define UART_CH SCI_CH
#define UART_TxStr sci_tx_str
#define PCLK    66000000
#define ETH_CH  1

// #define PRI_ETH     (14)
// #define PRI_SCI     (3)
// #define PRI_TIM0    (15)    // 10us
// #define PRI_TIM1    (5)     // 1ms
// #define PRI_TIM2    (12)
// #define PRI_TIM3    (12)
// #define PRI_USB     (14)  defined in usb_hal.h

#if defined(USE_DBG_PRINT)
#if !defined(DEBUG_CH)
#define DEBUG_CH SCI_CH
#endif
#define DEBUG_TXSTR(s)  sci_tx_str(DEBUG_CH, (unsigned char *)s)
#define DEBUG_TXCH(c)   sci_tx_ch(DEBUG_CH, c)
#else
#define DEBUG_TXSTR(s)
#define DEBUG_TXCH(c)
#endif

void rza2m_init(void);
void bootstrap(void);

#ifdef __cplusplus
}
#endif

#endif /* PORTS_RZ_RZA2M_RZA2M_CONFIG_H_ */
