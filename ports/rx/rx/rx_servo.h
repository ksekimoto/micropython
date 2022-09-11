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

#ifndef RX_RX_SERVO_H_
#define RX_RX_SERVO_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*rx_servo_cb_t)();

void rx_servo_set_callback(rx_servo_cb_t cb);
void rx_servo_enable_it(void);
void rx_servo_disable_it(void);
void rx_servo_set_pulse(uint8_t pin_idx, uint32_t pulse);
void rx_servo_start(uint8_t pin_idx);
void rx_servo_stop(uint8_t pin_idx);
void rx_servo_init(void);
void rx_servo_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* RX_RX_SERVO_H_ */
