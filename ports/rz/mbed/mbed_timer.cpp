/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 * Copyright (c) 2020 Kentaro Sekimoto
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

#include "mbed.h"
#include "mbed_timer.h"
#include "systick.h"

static Ticker ms_tick;
static volatile uint32_t _mstick = 0;

void onMillisecondTicker(void) {
    // this code will run every millisecond
    SysTick_Handler();
    _mstick++;
}

uint32_t mbed_timer_get_ticks(void) {
    return _mstick;
}

void mbed_ticker_thread(void *thread, uint32_t us) {
    static Ticker ticker;
    ticker.attach_us((void (*)())thread, (us_timestamp_t)us);
}

void mbed_timer_init(void) {
    _mstick = 0;
    ms_tick.attach_us(onMillisecondTicker,1000);
}

void mbed_timer_deinit() {
    // turn off 1ms interrupt
    ms_tick.detach();
}
