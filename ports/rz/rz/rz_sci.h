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
#ifndef RZ_RZ_SCI_H_
#define RZ_RZ_SCI_H_

#ifdef __cplusplus
extern "C" {
#endif

void rz_sci_init(int ch, int baud, int bits, int parity, int stop, int flow);
void rz_sci_init_with_pins(int ch, int tx_pin, int rx_pin, int baud, int bits, int parity, int stop, int flow);
void rz_sci_init_default(int ch, int baud);
void rz_sci_set_baud(int ch, int baud);
uint8_t rz_sci_rx_ch(int ch);
int rz_sci_rx_any(int ch);
int rz_sci_tx_wait(int ch);
void rz_sci_tx_ch(int ch, unsigned char c);
void rz_sci_tx_str(int ch, unsigned char *p);
void rz_sci_deinit(int ch);
typedef int (*SCI_CALLBACK)(int d);
void rz_sci_rx_set_callback(int ch, SCI_CALLBACK callback);
void rz_sci_isr_te(int ch);
void rz_sci_isr_er(int ch);

#ifdef __cplusplus
}
#endif

#endif /* RZ_RZ_SCI_H_ */
