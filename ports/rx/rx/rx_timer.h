/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Kentaro Sekimoto
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef RX_RX_TIMER_H_
#define RX_RX_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define DEF_CLKDEV  8
#define TENUSEC_COUNT   (MICROPY_HW_MCU_PCLK / DEF_CLKDEV / 100000)
#define MSEC_COUNT      (MICROPY_HW_MCU_PCLK / DEF_CLKDEV / 100)

#define CLKDEV  DEF_CLKDEV
#if CLKDEV == 8
#define CMT_VAL 0x0040;     // CMIE is Enable,CKS is PCLK/8     clk=1/6(us)
#elif CLKDEV == 32
#define CMT_VAL 0x0041;     // CMIE is Enable,CKS is PCLK/32    clk=1/1.5(us)
#elif CLKDEV == 128
#define CMT_VAL 0x0042;     // CMIE is Enable,CKS is PCLK/128   clk=32/12(us)
#else
#define CMT_VAL 0x0043;     // CMIE is Enable,CKS is PCLK/512   clk=128/3(us)
#endif

/*
 * RX63N
 * Prescale: 8
 * freq = 1000(1/s) = 1000(us) -> 6000
 * freq = 10000(1/s) = 100(us) -> 600
 * freq = 100000(1/s) = 10(us) -> 60
 */

/*
 * RX65N
 * Prescale: 8
 * freq = 1000(1/s) = 1000(us) -> 7500
 * freq = 10000(1/s) = 100(us) -> 750
 * freq = 100000(1/s) = 10(us) -> 75
 */

typedef void (*CMT_TIMER_FUNC)(void *);

void cmt_timer_init(unsigned int ch, unsigned int prescale);
void cmt_timer_deinit(unsigned int ch);
void cmt_timer_disable_clk(unsigned int ch);
void cmt_timer_eable_clk(unsigned int ch);
unsigned int cmt_timer_get_prescale(unsigned int ch);
void cmt_timer_set_prescale(unsigned int ch, unsigned int prescale);
unsigned int cmt_timer_get_counter(unsigned int ch);
void cmt_timer_set_counter(unsigned int ch, unsigned int count);
void cmt_timer_set_callback(unsigned int ch, CMT_TIMER_FUNC func, void *param);
void cmt_timer_callback(unsigned int ch);
unsigned long cmt_timer_get_cnt(unsigned int ch);
void cmt_timer_set_cnt(unsigned int ch, unsigned long cnt);
unsigned long cmt_timer_get_period(unsigned int ch);
void cmt_timer_set_period(unsigned int ch, unsigned long period);

void udelay_init(void);
void udelay(int m);
void mdelay(int m);
unsigned long utick(void);
unsigned long mtick(void);

#ifdef __cplusplus
}
#endif

#endif /* RX_RX_TIMER_H_ */
