/*
 * Copyright (c) 2018, Kentaro Sekimoto
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

#ifndef RX63N_CONFIG_H_
#define RX63N_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SCI_CH  0
#define SCI_BAUD 115200
#define UART_CH SCI_CH
#define UART_TxStr rx_sci_tx_str
#define PCLK    48000000

#if defined(USE_DBG_PRINT)
#if !defined(DEBUG_CH)
#define DEBUG_CH SCI_CH
#endif
#if !defined(DEBUG_CH_TX)
#define DEBUG_CH_TX P20
#endif
#if !defined(DEBUG_CH_RX)
#define DEBUG_CH_RX P21
#endif
#define DEBUG_TXSTR(s)  rx_sci_tx_str(DEBUG_CH, (unsigned char *)s)
#define DEBUG_TXCH(c)   rx_sci_tx_ch(DEBUG_CH, c)
// #define DEBUG_TXSTR(S)  printf("%s", S)
// #define DEBUG_TXCH(C)   printf("%c", C)
#else
#define DEBUG_TXSTR(s)
#define DEBUG_TXCH(c)
#endif

void rx_init(void);
void bootstrap(void);

#if !defined(RX_PRI_UART)
#define RX_PRI_UART (6)
#endif
#if !defined(RX_PRI_I2C)
#define RX_PRI_I2C (8)
#endif
#if !defined(RX_PRI_TIM0)
#define RX_PRI_TIM0 (15)
#endif
#if !defined(RX_PRI_TIM1)
#define RX_PRI_TIM1 (5)
#endif
#if !defined(RX_PRI_TIM2)
#define RX_PRI_TIM2 (12)
#endif
#if !defined(RX_PRI_TIM3)
#define RX_PRI_TIM3 (12)
#endif
#if !defined(RX_PRI_EXTINT)
#define RX_PRI_EXTINT (5)
#endif
#if !defined(RX_PRI_ETH)
#define RX_PRI_ETH (14)
#endif
#if !defined(RX_PRI_SYSTICK)
#define RX_PRI_SYSTICK (0)
#endif
#if !defined(RX_PRI_PENDSV)
#define RX_PRI_PENDSV (15)
#endif
// #if !defined(RX_PRI_SDIO)
// #define RX_PRI_SDIO (4)
// #endif
// #if !defined(RX_PRI_DMA)
// #define RX_PRI_DMA (5)
// #endif
// #if !defined(RX_PRI_FLASH)
// #define RX_PRI_FLASH (6)
// #endif
// #if !defined(RX_PRI_OTG_FS)
// #define RX_PRI_OTG_FS (6)
// #endif
// #if !defined(RX_PRI_OTG_HS)
// #define RX_PRI_OTG_HS (6)
// #endif
// #if !defined(RX_PRI_CAN)
// #define RX_PRI_CAN (7)
// #endif
// #if !defined(RX_PRI_SPI)
// #define RX_PRI_SPI (8)
// #endif
// #if !defined(RX_PRI_TIMX)
// #define RX_PRI_TIMX (13)
// #endif
// #if !defined(RX_PRI_RTC_WKUP)
// #define RX_PRI_RTC_WKUP (15)
// #endif

#if !defined(__WEAK)
#define __WEAK __attribute__((weak))
#endif

#ifdef __cplusplus
}
#endif

#endif /* RX63N_CONFIG_H_ */
