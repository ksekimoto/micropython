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

#ifndef RX63N_TIMER_H_
#define RX63N_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*CMT_TIMER_FUNC)(uint32_t);

void cmt_timer_init(unsigned int ch);
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

#endif /* RX63N_TIMER_H_ */
